#include "Hypnos_Visitor.hpp"

#include "Declaration.hpp"
#include "Expression.hpp"

#include "philotes.h"
#include "symtab.hpp"

#include "StringFunctions.h"

#include <cmath>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <sys/time.h>

using std::fmod;
using std::istream;
using std::ifstream;
using std::endl;
using std::string;
using std::to_string;
using std::vector;

void Hypnos_Visitor::visit(Decl_List& decl_list)
{
     Func_Decl* func;
     Var_Decl_Statement* vdecl;
     for(Declaration* decl : decl_list.data)
          if(func = dynamic_cast<Func_Decl*>(decl))
          {
               if(func->name=="main")
               {
                    symtab.emplace_back();
                    this_stack.push(NULL);
                    //TODO: args array
                    func->visit_with(*this);
                    this_stack.pop();
                    symtab.pop_back();
               }
          }
          else if(static_scopes.size() && (vdecl = dynamic_cast<Var_Decl_Statement*>(decl)))
          {
               if(vdecl->modifiers.static_m)
                    vdecl->visit_with(*this);
          }
          else
               decl->visit_with(*this);
}

void Hypnos_Visitor::visit(Include_Decl& include_decl)
{
     //No code needed (handle in Semantic_Visitor)
}

void Hypnos_Visitor::visit(Func_Decl& func_decl)
{
     interrupt_level = 0;
     func_decl.body.visit_with(*this);
     interrupt_level = 0;
}

void Hypnos_Visitor::visit(Import_Decl& import_decl)
{
     //No code needed (handle in Semantic_Visitor)
}

void Hypnos_Visitor::visit(Class_Decl& class_decl)
{
     static_scopes.push_back(class_decl.type->name);
     class_decl.body.visit_with(*this);
     static_scopes.pop_back();
}

void Hypnos_Visitor::visit(Class_Decl_Instantiated& class_decl)
{
     //TODO: C++
}

void Hypnos_Visitor::visit(Function_Call& func_call)
{
     map<string,Symbol> new_local_syms;
     Func_Decl* fd_ast = static_cast<Func_Decl*>(func_call.called_func_sym->value.ary.ptr);

     //Special case array "constructor"
     if(!fd_ast)
     {
          Type* expr_type = &typetab[func_call.called_func_name];
          Type* element_type = expr_type->base_or_parent_types.front();

          func_call.parameters.front()->visit_with(*this);
          int ary_length = rvalue_of(*last_expr_eval).numeric_val;
          Symbol* ary_symbol = new Symbol{};
          for(int i=0; i<ary_length; i++)
          {
               string key = to_string(i);
               Symbol to_insert{};
               to_insert.type = element_type;
               ary_symbol->components[key] = to_insert;
          }

          ary_symbol->type = expr_type;
          last_expr_eval = mk_lvalue(ary_symbol);
          return;
     }

     list<Expression*>::iterator i;
     list<Var_Decl_Statement*>::iterator j;
     list<Symbol*> local_params_for_prayer;
     for(i=func_call.parameters.begin(),j=fd_ast->modifiers.parameters.begin(); i!=func_call.parameters.end(); ++i,++j)
     {
          (*i)->visit_with(*this);
          new_local_syms[(*j)->name] = rvalue_sym(*last_expr_eval,(*i)->static_type);
          if(!last_expr_eval->literal_status)
               local_params_for_prayer.push_back(lvalue_of(*last_expr_eval));
          else
               local_params_for_prayer.push_back(NULL);
     }
     while(j!=fd_ast->modifiers.parameters.end())
     {
          (*j)->init_expr->visit_with(*this);
          new_local_syms[(*j)->name] = rvalue_sym(*last_expr_eval,(*i)->static_type);
          ++j;
     }
     
     //Componentize func_call...
     vector<string> components;
     StringFunctions::tokenize(components,func_call.called_func_name,".");
     
     //Handle this_stack
     Symbol* new_this;
     if(func_call.new_call) //We are a constructor -- need to construct a brand new this; TODO C++
     {
          new_this = new Symbol;
          Symbol* type_symbol = resolve_atomic_symbol(func_call.called_func_name);
          assert(type_symbol);
          new_this->type = &typetab[type_symbol->name];
          symtab.emplace_back();
          for(Var_Decl_Statement* dclstmt : new_this->type->contents)
               if(!dclstmt->modifiers.static_m)
                    dclstmt->visit_with(*this);
          new_this->value.ary.ptr = NULL;
          new_this->components = symtab.back();
          symtab.pop_back();
     }
     else if(components.size()==1) //then object doesn't change
          new_this = this_stack.top();
     else if(!resolve_atomic_symbol(components[0]) || !resolve_atomic_symbol(components[0])->type) //static
          new_this = NULL;
     else
     {
          auto i = components.begin();
          new_this = static_cast<Symbol*>(resolve_atomic_symbol(*i)->value.ary.ptr);
          ++i;
          while(i+1!=components.end())
               new_this = &new_this->components[*i++];
     }

     //Create environment for function call
     symtab.push_back(new_local_syms);
     this_stack.push(new_this);
     call_stack.push(&func_call);
     sever_visibility();
     ref_params_for_prayer = local_params_for_prayer;

     //Reset static_scopes
     list<string> saved_static_scopes = static_scopes;
     string root_prefix;
     for(auto i = components.begin(); i+1!=components.end() && (root_prefix=="" || symtab.front().count(root_prefix)); ++i)
          root_prefix += "::"+*i;
     if(root_prefix!="")
     {
          static_scopes.clear();
          vector<string> ssfc; //static_scopes for call
          StringFunctions::strsplit(ssfc,root_prefix,"::");
          auto i = ssfc.begin();
          static_scopes.push_back("::"+*(i++));
          for(; i!=ssfc.end(); ++i)
               static_scopes.push_back(*i);
     }

     //Do the function call
     fd_ast->visit_with(*this);

     //Restore environment
     static_scopes = saved_static_scopes;
     ref_params_for_prayer.clear();
     severances.pop();
     call_stack.pop();
     this_stack.pop();
     symtab.pop_back();

     //If we're a constructor, our expression value is our newly constructed this
     if(func_call.new_call)
     {
          Symbol* indirection = new Symbol;
          indirection->type = new_this->type;
          indirection->value.ary.ptr = new_this;
          Value to_insert;
          to_insert.ary.ptr = indirection;
          last_expr_eval = new Atomic_Expression(0,to_insert);
     }
}

void Hypnos_Visitor::visit(Aggregate_Access& ag_access)
{
     //TODO: handle C++ value-based objects
     ag_access.base_expression->visit_with(*this);
     Symbol* indirect_sym = lvalue_of(*last_expr_eval);
     Symbol* base_sym = static_cast<Symbol*>(indirect_sym->value.ary.ptr);

     if(!indirect_sym->type) //lhs is the name of a type, not variable
     {
          string full_name = indirect_sym->name+"::"+ag_access.component_name;
          if(symtab.front().count(full_name))
          {
               last_expr_eval = mk_lvalue(&symtab.front()[full_name]);
               return;
          }
     }
     else if(base_sym && base_sym->components.count(ag_access.component_name))
     {
          last_expr_eval = mk_lvalue(&base_sym->components[ag_access.component_name]);
          return;
     }
     
     report_error("Invalid component access",&ag_access);
     throw "Runtime error";
}

void Hypnos_Visitor::visit(Array_Access& ary_access)
{
     ary_access.subscript_expression->visit_with(*this);
     string key = to_string(rvalue_of(*last_expr_eval).numeric_val);
     ary_access.base_expression->visit_with(*this);
     Symbol* base = lvalue_of(*last_expr_eval);
     if(!base->components.count(key))
     {
          report_error("Invalid array access",&ary_access);
          throw "Runtime error";
     }
     last_expr_eval = mk_lvalue(&base->components[key]);
}

void Hypnos_Visitor::visit(Ternary_Conditional& ternary_expr)
{
     ternary_expr.conditional->visit_with(*this);
     if(rvalue_of(*last_expr_eval).numeric_val)
          ternary_expr.val_if_cond_true->visit_with(*this);
     else
          ternary_expr.val_if_cond_false->visit_with(*this);
}

void Hypnos_Visitor::visit(Parenthetical& parenthetical)
{
     parenthetical.nested->visit_with(*this);
}

void Hypnos_Visitor::visit(Atomic_Expression& atomic_expr)
{
     if(atomic_expr.literal_status)
          last_expr_eval = &atomic_expr;
     else
     {
          last_expr_eval = new Atomic_Expression{0,Value{}};
          last_expr_eval->value.ary.ptr = resolve_atomic_symbol(atomic_expr.value.as_string());
     }
}

void Hypnos_Visitor::visit(Statement_List& stmt_list)
{
     interrupt_level = 0;
     for(Statement* stmt : stmt_list.data)
     {
          stmt->visit_with(*this);
          if(interrupt_level)
               break;
     }
}

void Hypnos_Visitor::visit(Label& label)
{
     //No code needed
}

void Hypnos_Visitor::visit(Var_Decl_Statement& vdecl_stmt)
{
     map<string,Symbol>& local_syms = symtab.back();
     Symbol to_insert{};
     to_insert.name = vdecl_stmt.name;
     to_insert.type = vdecl_stmt.modifiers.static_type;
     if(vdecl_stmt.init_expr)
     {
          vdecl_stmt.init_expr->visit_with(*this);
          if(last_expr_eval->literal_status || to_insert.type->name!="Object" && to_insert.type->base_or_parent_types.empty()) //TODO: handle C++ value-based objects
               to_insert = rvalue_sym(*last_expr_eval,to_insert.type);
          else
               to_insert = *lvalue_of(*last_expr_eval);
          to_insert.type = vdecl_stmt.modifiers.static_type;
     }
     else if(to_insert.type==&typetab["String"])
          to_insert.value.ary.ptr = calloc(1,1);
     if(!vdecl_stmt.modifiers.static_m)
          local_syms[vdecl_stmt.name] = to_insert;
     else
          local_syms[get_scope_qualifier_prefix()+vdecl_stmt.name] = to_insert;
}

void Hypnos_Visitor::visit(Sentinel& cursor)
{
     switch(cursor.cursor_type)
     {
     case CURSOR: assert(false);
          break;
     case SIGNPOST: assert(false);
          break;
     case PRAYER: handle_prayer();
          break;
     default: assert(false);
          break;
     }
}

void Hypnos_Visitor::visit(Comment& comment)
{
     //No code needed
}

void Hypnos_Visitor::visit(If_Statement& if_stmt)
{
     if_stmt.condition->visit_with(*this);
     if(rvalue_of(*last_expr_eval).numeric_val)
     {
          symtab.emplace_back();
          if_stmt.if_branch.visit_with(*this);
          symtab.pop_back();
     }
     else
     {
          symtab.emplace_back();
          if_stmt.else_branch.visit_with(*this);
          symtab.pop_back();
     }
}

void Hypnos_Visitor::visit(While_Statement& while_stmt)
{
     if(while_stmt.do_while)
     {
          symtab.emplace_back();
          while_stmt.statements.visit_with(*this);
          symtab.pop_back();
     }
     while_stmt.condition->visit_with(*this);
     while(rvalue_of(*last_expr_eval).numeric_val)
     {
          symtab.emplace_back();     
          while_stmt.statements.visit_with(*this);
          symtab.pop_back();
          switch(interrupt_level)
          {
          case CONTINUE: interrupt_level = 0;
          case 0: while_stmt.condition->visit_with(*this);
               break;
          case BREAK: interrupt_level = 0;
          case RETURN: return;
          default: assert(false);
          }
     }
}

void Hypnos_Visitor::visit(For_Statement& for_stmt)
{
     symtab.emplace_back();
     for_stmt.init_stmt->visit_with(*this);
     for_stmt.condition->visit_with(*this);
     while(rvalue_of(*last_expr_eval).numeric_val)
     {
          for_stmt.statements.visit_with(*this);
          switch(interrupt_level)
          {
          case CONTINUE: interrupt_level = 0;
          case 0: for_stmt.post->visit_with(*this);
               for_stmt.condition->visit_with(*this);
               break;
          case BREAK: interrupt_level = 0;
          case RETURN: return;
          default: assert(false);
          }
     }
     symtab.pop_back();
}

void Hypnos_Visitor::visit(Switch_Statement& switch_stmt)
{
     //TODO
     assert(false);
}

void Hypnos_Visitor::visit(Anonymous_Block& anon_blk)
{
     symtab.emplace_back();
     anon_blk.body.visit_with(*this);
     symtab.pop_back();
}

void Hypnos_Visitor::visit(Interruptor& interrupt)
{
     if(interrupt.returned_expression)
          interrupt.returned_expression->visit_with(*this);
     interrupt_level = interrupt.level;
}

void Hypnos_Visitor::handle_prayer()
{
     Function_Call* called_func = call_stack.top();
     string n = called_func->called_func_name;
     if(n!="System.out.println" && n!="System.out.print" && n!="Double.doubleToLongBits" && n!="Double.longBitsToDouble")
          n = n.substr(n.find(".")+1);
     map<string,Symbol>& parameters = symtab.back();
     map<string,Symbol>& caller_symtab = *++symtab.rbegin();
     Atomic_Expression* rvalue_exp = new Atomic_Expression{0,Value{}};
     Symbol* tthis = this_stack.top();

     //For operators with side effects
     Symbol* lvca = ref_params_for_prayer.size() ? ref_params_for_prayer.front() : NULL;
     Symbol* lvcb = ref_params_for_prayer.size() ? *++ref_params_for_prayer.begin() : NULL;
     bool a_float = parameters["a"].type==&typetab["float"] || parameters["a"].type==&typetab["double"];
     bool b_float = parameters.count("b") ? parameters["b"].type==&typetab["float"] || parameters["b"].type==&typetab["double"] : false;
     bool using_float = a_float || b_float;

     bool a_long = parameters["a"].type==&typetab["long"];
     bool b_long = parameters.count("b") ? parameters["b"].type==&typetab["long"] : false;
     bool using_long = a_long || b_long;
     
     int64_t a_val_int;
     int64_t b_val_int;
     double a_val_float;
     double b_val_float;
     if(a_float)
          a_val_float = parameters["a"].value.float_val;
     else if(b_float)
          a_val_float = parameters["a"].value.numeric_val;
     else
          a_val_int = parameters["a"].value.numeric_val;
     if(b_float)
          b_val_float = parameters["b"].value.float_val;
     else if(a_float && parameters.count("b"))
          b_val_float = parameters["b"].value.numeric_val;
     else if(parameters.count("b"))
          b_val_int = parameters["b"].value.numeric_val;

     if(n=="=" && !lvca->type->aggregate_type)
     {
          *lvca = rvalue_sym(*last_expr_eval,lvca->type);
          rvalue_exp = last_expr_eval;
     }
     else if(n=="=")
     {
          lvca->value = last_expr_eval->value;
          rvalue_exp = last_expr_eval;
     }
     else if(n=="++_")
     {
          if(a_float)
               lvca->value.float_val++;
          else
               lvca->value.numeric_val++;
          rvalue_exp->literal_status = 0;
          rvalue_exp->value.ary.ptr = lvca;
     }
     else if(n=="--_")
     {
          if(a_float)
               lvca->value.float_val--;
          else
               lvca->value.numeric_val--;
          rvalue_exp->literal_status = 0;
          rvalue_exp->value.ary.ptr = lvca;
     }
     else if(n=="_++")
     {
          rvalue_exp->literal_status = 0;
          rvalue_exp->value.ary.ptr = new Symbol(*lvca);
          if(a_float)
               lvca->value.float_val++;
          else
               lvca->value.numeric_val++;
     }
     else if(n=="_--")
     {
          rvalue_exp->literal_status = 0;
          rvalue_exp->value.ary.ptr = new Symbol(*lvca);
          if(a_float)
               lvca->value.float_val--;
          else
               lvca->value.numeric_val--;
     }
     else if(n=="!_")
     {
          rvalue_exp->literal_status = BOOLEAN;
          assert(!using_float);
          rvalue_exp->value.numeric_val = a_val_int ? 0 : 1;
     }
     else if(n=="&&")
     {
          rvalue_exp->literal_status = BOOLEAN;
          assert(!using_float);
          rvalue_exp->value.numeric_val = a_val_int && b_val_int;
     }
     else if(n=="||")
     {
          rvalue_exp->literal_status = BOOLEAN;
          assert(!using_float);
          rvalue_exp->value.numeric_val = a_val_int || b_val_int;
     }
     else if(n=="==" && (!lvca || !lvca->type->aggregate_type))
     {
          rvalue_exp->literal_status = BOOLEAN;
          if(using_float)
               rvalue_exp->value.numeric_val = a_val_float==b_val_float;
          else
               rvalue_exp->value.numeric_val = a_val_int==b_val_int;
     }
     else if(n=="==")
     {
          rvalue_exp->literal_status = BOOLEAN;
          rvalue_exp->value.numeric_val = lvca->value.ary.ptr==lvcb->value.ary.ptr;
     }
     else if(n=="!=" && (!lvca || !lvca->type->aggregate_type))
     {
          rvalue_exp->literal_status = BOOLEAN;
          if(using_float)
               rvalue_exp->value.numeric_val = a_val_float!=b_val_float;
          else
               rvalue_exp->value.numeric_val = a_val_int!=b_val_int;
     }
     else if(n=="!=")
     {
          rvalue_exp->literal_status = BOOLEAN;
          rvalue_exp->value.numeric_val = lvca->value.ary.ptr==lvcb->value.ary.ptr;
     }
     else if(n=="<")
     {
          rvalue_exp->literal_status = BOOLEAN;
          if(using_float)
               rvalue_exp->value.numeric_val = a_val_float < b_val_float;
          else
               rvalue_exp->value.numeric_val = a_val_int < b_val_int;
     }
     else if(n=="<=")
     {
          rvalue_exp->literal_status = BOOLEAN;
          if(using_float)
               rvalue_exp->value.numeric_val = a_val_float<=b_val_float;
          else
               rvalue_exp->value.numeric_val = a_val_int<=b_val_int;
     }
     else if(n==">")
     {
          rvalue_exp->literal_status = BOOLEAN;
          if(using_float)
               rvalue_exp->value.numeric_val = a_val_float>b_val_float;
          else
               rvalue_exp->value.numeric_val = a_val_int>b_val_int;
     }
     else if(n==">=")
     {
          rvalue_exp->literal_status = BOOLEAN;
          if(using_float)
               rvalue_exp->value.numeric_val = a_val_float>=b_val_float;
          else
               rvalue_exp->value.numeric_val = a_val_int>=b_val_int;
     }
     else if((n=="+" || n=="+=") && parameters["a"].type==&typetab["String"])
     {
          rvalue_exp->literal_status = STR_LITERAL;
          string to_append;
          if(parameters["b"].type==&typetab["String"])
               to_append = parameters["b"].value.as_string();
          else if(b_float)
               to_append = to_string(parameters["b"].value.float_val);
          else if(parameters["b"].type==&typetab["char"])
               to_append = (char)(parameters["b"].value.numeric_val);
          else if(parameters["b"].type==&typetab["byte"] || parameters["b"].type==&typetab["short"] || parameters["b"].type==&typetab["int"] || parameters["b"].type==&typetab["long"])
               to_append = to_string(parameters["b"].value.numeric_val);
          else if(parameters["b"].type==&typetab["boolean"])
               to_append = parameters["b"].value.numeric_val ? "true" : "false";
          else
               assert(false); //call toString method on Object; not implemented
          rvalue_exp->value.ary.ptr = strdup((parameters["a"].value.as_string()+to_append).c_str());

          if(n=="+=")
               *lvca = rvalue_sym(*rvalue_exp,lvca->type);
     }
     else if(n=="+" || n=="+=")
     {
          rvalue_exp->literal_status = using_float ? DOUBLE_LITERAL : using_long ? LONG_LITERAL : INT_LITERAL;
          if(using_float)
               rvalue_exp->value.float_val = a_val_float + b_val_float;
          else
               rvalue_exp->value.numeric_val = a_val_int + b_val_int;

          if(n=="+=")
               *lvca = rvalue_sym(*rvalue_exp,lvca->type);
     }
     else if(n=="-" || n=="-=")
     {
          rvalue_exp->literal_status = using_float ? DOUBLE_LITERAL : using_long ? LONG_LITERAL : INT_LITERAL;
          if(using_float)
               rvalue_exp->value.float_val = a_val_float - b_val_float;
          else
               rvalue_exp->value.numeric_val = a_val_int - b_val_int;

          if(n=="-=")
               *lvca = rvalue_sym(*rvalue_exp,lvca->type);
     }
     else if(n=="<<" || n=="<<=")
     {
          rvalue_exp->literal_status = using_float ? DOUBLE_LITERAL : using_long ? LONG_LITERAL : INT_LITERAL;
          assert(!using_float);
          rvalue_exp->value.numeric_val = a_val_int << b_val_int;

          if(n=="<<=")
               *lvca = rvalue_sym(*rvalue_exp,lvca->type);
     }
     else if(n==">>" || n==">>=")
     {
          rvalue_exp->literal_status = using_float ? DOUBLE_LITERAL : using_long ? LONG_LITERAL : INT_LITERAL;
          assert(!using_float);
          rvalue_exp->value.numeric_val = a_val_int >> b_val_int;

          if(n==">>=")
               *lvca = rvalue_sym(*rvalue_exp,lvca->type);
     }
     else if(n==">>>" || n==">>>=")
     {
          rvalue_exp->literal_status = using_float ? DOUBLE_LITERAL : using_long ? LONG_LITERAL : INT_LITERAL;
          assert(!using_float);
          uint64_t raw = a_val_int;
          raw>>=b_val_int;
          rvalue_exp->value.numeric_val = raw;

          if(n==">>>=")
               *lvca = rvalue_sym(*rvalue_exp,lvca->type);
     }
     else if(n=="&" || n=="&=")
     {
          rvalue_exp->literal_status = using_float ? DOUBLE_LITERAL : using_long ? LONG_LITERAL : INT_LITERAL;
          assert(!using_float);
          rvalue_exp->value.numeric_val = a_val_int & b_val_int;

          if(n=="&=")
               *lvca = rvalue_sym(*rvalue_exp,lvca->type);
     }
     else if(n=="|" || n=="|=")
     {
          rvalue_exp->literal_status = using_float ? DOUBLE_LITERAL : using_long ? LONG_LITERAL : INT_LITERAL;
          assert(!using_float);
          rvalue_exp->value.numeric_val = a_val_int | b_val_int;

          if(n=="|=")
               *lvca = rvalue_sym(*rvalue_exp,lvca->type);
     }
     else if(n=="^" || n=="^=")
     {
          rvalue_exp->literal_status = parameters["a"].type==&typetab["boolean"] ? BOOLEAN_LITERAL : INT_LITERAL;
          assert(!using_float);
          rvalue_exp->value.numeric_val = a_val_int ^ b_val_int;

          if(n=="^=")
               *lvca = rvalue_sym(*rvalue_exp,lvca->type);
     }
     else if(n=="-_")
     {
          rvalue_exp->literal_status = a_float ? DOUBLE_LITERAL : using_long ? LONG_LITERAL : INT_LITERAL;
          if(a_float)
               rvalue_exp->value.float_val = -a_val_float;
          else
               rvalue_exp->value.numeric_val = -a_val_int;
     }
     else if(n[0]=='(') //cast
     {
          static const map<char,int> CAST_AWAY = {{'b',BYTE},{'s',SHORT},{'c',CHAR_LITERAL},{'i',INT_LITERAL},{'l',LONG},{'f',FLOAT_LITERAL},{'d',DOUBLE_LITERAL}};
          rvalue_exp->literal_status = CAST_AWAY.at(n[1]);
          auto java_integral_downcast = [&](uint64_t mask)
          {
               int64_t raw_bits = using_float ? a_val_float : a_val_int;
               return raw_bits & mask;
          };
          switch(n[1])
          {
          case 'b':
               rvalue_exp->value.numeric_val = java_integral_downcast(0xFF);
               break;
          case 'c':
               rvalue_exp->value.numeric_val = java_integral_downcast(0xFFFF);
               break;
          case 's':
               rvalue_exp->value.numeric_val = java_integral_downcast(0xFFFF);
               break;
          case 'i':
               rvalue_exp->value.numeric_val = java_integral_downcast(0xFFFFFFFF);
               break;
          case 'l':
               rvalue_exp->value.numeric_val = java_integral_downcast(-1);
               break;
          default:
               rvalue_exp->value.float_val = using_float ? a_val_float : a_val_int;
          }
     }
     else if(n=="*" || n=="*=")
     {
          rvalue_exp->literal_status = using_float ? DOUBLE_LITERAL : using_long ? LONG_LITERAL : INT_LITERAL;
          if(using_float)
               rvalue_exp->value.float_val = a_val_float * b_val_float;
          else
               rvalue_exp->value.numeric_val = a_val_int * b_val_int;

          if(n=="*=")
               *lvca = rvalue_sym(*rvalue_exp,lvca->type);
     }
     else if(n=="/" || n=="/=")
     {
          rvalue_exp->literal_status = using_float ? DOUBLE_LITERAL : using_long ? LONG_LITERAL : INT_LITERAL;
          if(using_float)
               rvalue_exp->value.float_val = a_val_float / b_val_float;
          else
               rvalue_exp->value.numeric_val = a_val_int / b_val_int;

          if(n=="/=")
               *lvca = rvalue_sym(*rvalue_exp,lvca->type);
     }
     else if(n=="%" || n=="%=")
     {
          rvalue_exp->literal_status = using_float ? DOUBLE_LITERAL : using_long ? LONG_LITERAL : INT_LITERAL;
          if(using_float)
               rvalue_exp->value.float_val = fmod(a_val_float,b_val_float);
          else
               rvalue_exp->value.numeric_val = a_val_int % b_val_int;

          if(n=="%=")
               *lvca = rvalue_sym(*rvalue_exp,lvca->type);
     }
     else if(n=="System.out.println" || n=="System.out.print")
     {
          if(!parameters["a"].type)
               ;
          else if(parameters["a"].type==&typetab["boolean"])
               sout << (parameters["a"].value.numeric_val ? "true" : "false");
          else if(parameters["a"].type==&typetab["byte"])
               sout << static_cast<int>(static_cast<int8_t>(parameters["a"].value.numeric_val));
          else if(parameters["a"].type==&typetab["short"])
               sout << static_cast<int16_t>(parameters["a"].value.numeric_val);
          else if(parameters["a"].type==&typetab["char"])
               sout << static_cast<char>(parameters["a"].value.numeric_val);
          else if(parameters["a"].type==&typetab["int"])
               sout << static_cast<int32_t>(parameters["a"].value.numeric_val);
          else if(parameters["a"].type==&typetab["long"])
               sout << parameters["a"].value.numeric_val;
          else if(parameters["a"].type==&typetab["float"] || parameters["a"].type==&typetab["double"])
               sout << parameters["a"].value.float_val;
          else if(parameters["a"].type==&typetab["String"])
               sout << static_cast<char*>(parameters["a"].value.ary.ptr);
          else
          {
               yyerror("System.out.println called with object type; not implemented");
               throw "Runtime error";
          }
          if(n=="System.out.println")
               sout << endl;
     }
     else if(n=="Double.longBitsToDouble")
     {
          rvalue_exp->literal_status = DOUBLE_LITERAL;
          rvalue_exp->value.numeric_val = a_val_int;
     }
     else if(n=="Double.doubleToLongBits")
     {
          rvalue_exp->literal_status = LONG_LITERAL;
          rvalue_exp->value.float_val = a_val_float;
     }
     else if(n=="Scanner")
     {
          Symbol sym{};
          istream* pfin;
          if(!parameters["a"].components.count("input"))
               pfin = &sin;
          else
               pfin = new ifstream(parameters["a"].components["input"].value.as_string());
          if(!pfin)
          {
               yyerror("Error: Scanner attempted to open non-existent file.");
               throw "Runtime error";
          }
          sym.name = "#Scanner";
          sym.value.ary.ptr = pfin;
          tthis->components[".ifstream"] = sym;
     }
     else if(n=="nextInt")
     {
          rvalue_exp->literal_status = INT_LITERAL;
          (*static_cast<istream*>(tthis->components[".ifstream"].value.ary.ptr)) >> rvalue_exp->value.numeric_val;
     }
     else if(n=="nextFloat" || n=="nextDouble")
     {
          rvalue_exp->literal_status = n=="nextFloat" ? FLOAT_LITERAL : DOUBLE_LITERAL;
          (*static_cast<istream*>(tthis->components[".ifstream"].value.ary.ptr)) >> rvalue_exp->value.float_val;
     }
     else if(n=="next")
     {
          rvalue_exp->literal_status = STR_LITERAL;
          string input;
          (*static_cast<istream*>(tthis->components[".ifstream"].value.ary.ptr)) >> input;
          rvalue_exp->value.ary.ptr = strdup(input.c_str());
     }
     else if(n=="nextLine")
     {
          rvalue_exp->literal_status = STR_LITERAL;
          string input;
          getline(*static_cast<istream*>(tthis->components[".ifstream"].value.ary.ptr),input);
          rvalue_exp->value.ary.ptr = strdup(input.c_str());
     }
     else if(n=="currentTimeMillis")
     {
          rvalue_exp->literal_status = LONG;
          timeval retval;
          gettimeofday(&retval,NULL);
          rvalue_exp->value.numeric_val = retval.tv_sec*1000 + retval.tv_usec/1000;
     }
     else if(n=="length")
     {
          if(!tthis)
               throw "Runtime error: null String";
          rvalue_exp->literal_status = INT_LITERAL;
          rvalue_exp->value.numeric_val = strlen(reinterpret_cast<char*>(tthis));
     }
     else if(n=="charAt")
     {
          if(!tthis)
               throw "Runtime error: null String";
          if(a_val_int >= strlen(reinterpret_cast<char*>(tthis)))
               throw "Runtime error: index out of bounds for charAt";
          rvalue_exp->literal_status = CHAR_LITERAL;
          rvalue_exp->value.numeric_val = reinterpret_cast<char*>(tthis)[a_val_int];
     }
     else if(n=="equals" || n=="equalsIgnoreCase" || n=="compareTo")
     {
          //Function pointer for strcmp or strncmp
          int (*cmp_func)(const char* a, const char* b) = n=="equalsIgnoreCase" ? strcasecmp : strcmp;
          
          if(!tthis)
               throw "Runtime error: null String";
          if(!parameters["a"].value.ary.ptr)
               throw "Runtime error: null Parameter";
          rvalue_exp->literal_status = n=="equals" ? BOOLEAN_LITERAL : INT_LITERAL;
          rvalue_exp->value.numeric_val = cmp_func(reinterpret_cast<char*>(tthis),reinterpret_cast<char*>(parameters["a"].value.ary.ptr));
          if(n=="equals" || n=="equalsIgnoreCase")
               rvalue_exp->value.numeric_val = !rvalue_exp->value.numeric_val;
     }
     else
          assert(false);

     last_expr_eval = rvalue_exp;
}

Symbol* Hypnos_Visitor::lvalue_of(const Atomic_Expression& exp)
{
     assert(!exp.literal_status);
     return static_cast<Symbol*>(exp.value.ary.ptr);
}

Symbol Hypnos_Visitor::rvalue_sym(const Atomic_Expression& exp, Type* min_type)
{
     Type* value_type;
     switch(exp.literal_status)
     {
     case 0: if(!exp.value.ary.ptr)
          {
               report_error("Null object dereference",&exp);
               throw "Runtime error";
          }
          value_type = static_cast<Symbol*>(exp.value.ary.ptr)->type;
          break;
     case BOOLEAN_LITERAL: case BOOLEAN: value_type = &typetab["boolean"]; //TODO: C++
          break;
     case BYTE: value_type = &typetab["byte"];
          break;
     case SHORT: value_type = &typetab["short"];
          break;
     case CHAR_LITERAL: case CHAR: value_type = &typetab["char"];
          break;
     case INT_LITERAL: case INT: value_type = &typetab["int"];
          break;
     case LONG_LITERAL: case LONG: value_type = &typetab["long"];
          break;
     case FLOAT_LITERAL: case FLOAT: value_type = &typetab["float"];
          break;
     case DOUBLE_LITERAL: case DOUBLE: value_type = &typetab["double"];
          break;
     case STR_LITERAL: value_type = &typetab["String"]; //TODO: C++
     }

     Symbol to_return;
     to_return.type = value_type;
     if(exp.literal_status)
          to_return.value = exp.value;
     else if(value_type==&typetab["String"])
          to_return.value = static_cast<Symbol*>(exp.value.ary.ptr)->value;
     else
          to_return = *static_cast<Symbol*>(exp.value.ary.ptr);
     if(min_type && to_return.type->name!="float" && to_return.type->name!="double" && (min_type->name=="float" || min_type->name=="double"))
          to_return.value.float_val = to_return.value.numeric_val;
     if(min_type)
          to_return.type = min_type;
     return to_return;
}

Value Hypnos_Visitor::rvalue_of(const Atomic_Expression& exp)
{
     if(exp.literal_status)
          return exp.value;
     else
          return static_cast<Symbol*>(exp.value.ary.ptr)->value;
}

Atomic_Expression* Hypnos_Visitor::mk_lvalue(Symbol* lvalue)
{
     Atomic_Expression* to_return = new Atomic_Expression{0,Value{}};
     to_return->value.ary.ptr = lvalue;
     return to_return;
}

Symbol Hypnos_Visitor::lee_sym()
{
     if(!last_expr_eval || !last_expr_eval->literal_status && !last_expr_eval->value.ary.ptr)
          return Symbol{};
     else
          return rvalue_sym(*last_expr_eval);
}
