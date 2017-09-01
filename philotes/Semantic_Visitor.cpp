#include "Semantic_Visitor.hpp"
#include "Declaration.hpp"
#include "Expression.hpp"
#include "Statement.hpp"

#include "philotes.h"
#include "symtab.hpp"

#include <algorithm>
#include <string>
#include <vector>

#include "StringFunctions.h"

using std::find_if;
using std::for_each;
using std::string;
using std::vector;

void Semantic_Visitor::visit(Decl_List& decl_list)
{
     for(Declaration* decl : decl_list.data)
          decl->visit_with(*this);
}

void Semantic_Visitor::visit(Include_Decl& include_decl)
{
     //TODO: handle C++ includes
}

void Semantic_Visitor::visit(Func_Decl& func_decl)
{
     symtab.emplace_back();
     if(func_decl.modifiers.static_m)
          sever_visibility();     
     for(Var_Decl_Statement* vds : func_decl.modifiers.parameters)
          vds->visit_with(*this);
     func_decl.body.visit_with(*this);
     if(func_decl.modifiers.static_m)
          severances.pop();
     symtab.pop_back();
}

void Semantic_Visitor::visit(Import_Decl& import_decl)
{
     //TODO: add Java standard library imports to global scope
}

void Semantic_Visitor::visit(Class_Decl& class_decl)
{
     static_scopes.push_back(class_decl.type->name);
     for(Var_Decl_Statement* vds : class_decl.type->contents)
          if(vds->modifiers.static_m)
               vds->visit_with(*this);
     symtab.emplace_back();
     for(Var_Decl_Statement* vds : class_decl.type->contents)
          if(!vds->modifiers.static_m)
               vds->visit_with(*this);
     for(Declaration* decl : class_decl.body.data)
          if(!dynamic_cast<Var_Decl_Statement*>(decl))
               decl->visit_with(*this);
     symtab.pop_back();
     static_scopes.pop_back();
}

void Semantic_Visitor::visit(Class_Decl_Instantiated& class_decl)
{
     assert(false); //TODO C++
}

void Semantic_Visitor::visit(Function_Call& func_call)
{
     //Special case array "constructors"
     if(func_call.called_func_name.find('[')!=-1)
     {
          func_call.called_func_sym = &symtab.front()["[]"];
          last_expr_eval = typetab.count(func_call.called_func_name) ? &typetab[func_call.called_func_name] : NULL;
          return;
     }

     //First, must visit and resolve types of expressions
     for(Expression* exp : func_call.parameters)
          exp->visit_with(*this);
     list<Type*> param_types;
     for(Expression* exp : func_call.parameters)
          if(exp->static_type)
               param_types.push_back(exp->static_type);
          else
          {
               last_expr_eval = NULL;
               report_error("Cascading error due to invalid expression",&func_call);
               return;
          }
     
     Symbol* called_sym;
     if(param_types.size() != func_call.parameters.size())
          ; //TODO: C++ def-params
     if(called_sym = resolve_function_symbol(qualify_unscoped_reference(func_call.new_call ? func_call.called_func_name+"::"+func_call.called_func_name : func_call.called_func_name),param_types))
          ;
     else if(func_call.called_func_name.find(".")!=string::npos)
     {
          vector<string> result;
          StringFunctions::tokenize(result,func_call.called_func_name,".");
          auto i = result.begin();
          called_sym = resolve_atomic_symbol(*i);
          ++i;
          while(i!=result.end() && called_sym && called_sym->components.count(*i))
               called_sym = &called_sym->components[*i++];
          if(called_sym && i+1==result.end()) //(incompletely) handle object method calls
          {
               string type_prefix = called_sym->type ? called_sym->type->name : called_sym->name;
               called_sym = resolve_function_symbol(qualify_unscoped_reference(type_prefix+"::"+*i),param_types);
          }
          else
               called_sym = NULL;
     }
     else
          called_sym = NULL;
     if(!called_sym)
     {
          assert(param_types.size()==func_call.parameters.size());
          report_error("No function with signature "+func_call.called_func_name+"("+[&]()
                       {
                            string params="";
                            bool first = true;
                            for(Expression* exp : func_call.parameters)
                            {
                                 if(!first)
                                      params+=",";
                                 else
                                      first = false;
                                 params+=exp->static_type->name;
                            }
                            return params;
                       }()+")",&func_call);
          error_nodes.insert(&func_call);
     }

     func_call.called_func_sym = called_sym;
     if(called_sym && called_sym->type && called_sym->type->parameters.size())
          last_expr_eval = func_call.static_type = called_sym->type->parameters.front();
     else
          last_expr_eval = func_call.static_type = NULL;
}

void Semantic_Visitor::visit(Aggregate_Access& ag_access)
{
     ag_access.base_expression->visit_with(*this);
     if(!last_expr_eval)
     {
          error_nodes.insert(&ag_access);
          return;
     }

     //Visit components
     symtab.emplace_back();
     for(Var_Decl_Statement* component : last_expr_eval->contents)
          component->visit_with(*this);

     //TODO: handle access_form for C++ ->, ::
     assert(ag_access.access_form==DOT);

     //Do the actual lookup
     map<string,Symbol>& ag_symtab = symtab.back();
     if(!ag_symtab.count(ag_access.component_name))
     {
          report_error("Unknown component "+ag_access.component_name+" in aggregate access",&ag_access);
          error_nodes.insert(&ag_access);
          last_expr_eval = ag_access.static_type = NULL;
     }
     else
          last_expr_eval = ag_access.static_type = ag_symtab[ag_access.component_name].type;
     symtab.pop_back();
}

void Semantic_Visitor::visit(Array_Access& ary_access)
{
     Type* to_return = NULL;
     ary_access.base_expression->visit_with(*this);
     if(!last_expr_eval)
          error_nodes.insert(ary_access.base_expression);
     else if(!last_expr_eval->is_array)
     {
          error_nodes.insert(ary_access.base_expression);
          report_error("Non-array accessed as one",ary_access.base_expression);
     }
     else
     {
          assert(last_expr_eval->base_or_parent_types.size()==1);
          to_return = last_expr_eval->base_or_parent_types.front();
     }
     
     const static auto INTEGRAL_TYPES = {"byte","char","short","int","long"};
     ary_access.subscript_expression->visit_with(*this);
     if(!last_expr_eval)
          error_nodes.insert(ary_access.subscript_expression);
     else if(find_if(INTEGRAL_TYPES.begin(),INTEGRAL_TYPES.end(),[&](auto& c) { return last_expr_eval->name==c; })==INTEGRAL_TYPES.end())
     {
          error_nodes.insert(ary_access.subscript_expression);
          report_error("Array subscript expression of non-integral type",ary_access.subscript_expression);
     }

     last_expr_eval = ary_access.static_type = to_return;
}

void Semantic_Visitor::visit(Ternary_Conditional& ternary_expr)
{
     ternary_expr.conditional->visit_with(*this);
     if(!last_expr_eval)
          error_nodes.insert(ternary_expr.conditional);
     else if(last_expr_eval->name!="bool" && last_expr_eval->name!="boolean") //TODO: C++
     {
          error_nodes.insert(ternary_expr.conditional);
          report_error("Ternary operator conditional non-boolean",ternary_expr.conditional);
     }

     //TODO: check that the true and false branch types are compatible
     ternary_expr.val_if_cond_true->visit_with(*this);
     ternary_expr.val_if_cond_false->visit_with(*this);
     
     ternary_expr.static_type = last_expr_eval;
}

void Semantic_Visitor::visit(Parenthetical& parenthetical)
{
     parenthetical.nested->visit_with(*this);
     parenthetical.static_type = parenthetical.nested->static_type;
}

void Semantic_Visitor::visit(Atomic_Expression& atomic_expr)
{
     switch(atomic_expr.literal_status)
     {
     case STR_LITERAL: last_expr_eval = atomic_expr.static_type = &typetab[language_mode==JAVA_MODE ? "String" : "string"];
          return;
     case BOOLEAN_LITERAL: last_expr_eval = atomic_expr.static_type = &typetab["boolean"];
          return;
     case CHAR_LITERAL: last_expr_eval = atomic_expr.static_type = &typetab["char"];
          return;
     case INT_LITERAL: last_expr_eval = atomic_expr.static_type = &typetab["int"];
          return;
     case LONG_LITERAL: last_expr_eval = atomic_expr.static_type = &typetab["long"];
          return;
     case FLOAT_LITERAL: last_expr_eval = atomic_expr.static_type = &typetab["float"];
          return;
     case DOUBLE_LITERAL: last_expr_eval = atomic_expr.static_type = &typetab["double"];
          return;
     }

     Symbol* referenced = resolve_atomic_symbol(atomic_expr.value.as_string());
     if(referenced)
     {
          last_expr_eval = atomic_expr.static_type = referenced->type ? referenced->type : &typetab[referenced->name];
          return;
     }
     
     error_nodes.insert(&atomic_expr);
     report_error(string("Undefined variable ")+atomic_expr.value.as_string(),&atomic_expr);
     last_expr_eval = NULL;
}

void Semantic_Visitor::visit(Statement_List& stmt_list)
{
     symtab.emplace_back();
     for(Statement* stmt : stmt_list.data)
          stmt->visit_with(*this);
     symtab.pop_back();
}

void Semantic_Visitor::visit(Label& label)
{
     //no code needed
}

void Semantic_Visitor::visit(Var_Decl_Statement& vdecl_stmt)
{
     if(vdecl_stmt.init_expr)
          vdecl_stmt.init_expr->visit_with(*this);
     
     Symbol to_insert;
     to_insert.name = vdecl_stmt.name;
     to_insert.type = vdecl_stmt.modifiers.static_type;

     string insert_name = symtab.size() > 1 ? vdecl_stmt.name : get_scope_qualifier_prefix()+vdecl_stmt.name;

     if(vdecl_stmt.init_expr && vdecl_stmt.init_expr->static_type)
     {
          list<Type*> assign_state_params{vdecl_stmt.modifiers.static_type,vdecl_stmt.init_expr->static_type};
          if(!resolve_function_symbol("::=",assign_state_params))
               report_error("Invalid initialization of variable "+insert_name,&vdecl_stmt);
     }
     if(symtab.back().count(insert_name))
               report_error("Variable "+insert_name+" redefined",&vdecl_stmt);
     symtab.back()[insert_name]=to_insert;
}

void Semantic_Visitor::visit(Sentinel& cursor)
{
     //no code needed
}

void Semantic_Visitor::visit(Comment& comment)
{
     //no code needed
}

void Semantic_Visitor::visit(If_Statement& if_stmt)
{
     if_stmt.condition->visit_with(*this);
     if_stmt.if_branch.visit_with(*this);
     if_stmt.else_branch.visit_with(*this);
}

void Semantic_Visitor::visit(While_Statement& while_stmt)
{
     bool old = in_loop;
     in_loop = true;
     while_stmt.condition->visit_with(*this);
     while_stmt.statements.visit_with(*this);
     in_loop = old;
}

void Semantic_Visitor::visit(For_Statement& for_stmt)
{
     symtab.emplace_back();
     bool old = in_loop;
     in_loop = true;
     for_stmt.init_stmt->visit_with(*this);
     for_stmt.condition->visit_with(*this);
     for_stmt.statements.visit_with(*this);
     for_stmt.post->visit_with(*this);
     in_loop = old;
     symtab.pop_back();
}

void Semantic_Visitor::visit(Switch_Statement& switch_stmt)
{
     bool old = in_switch;
     in_switch = true;
     switch_stmt.condition->visit_with(*this);
     switch_stmt.statements.visit_with(*this);
     in_switch = old;
}

void Semantic_Visitor::visit(Anonymous_Block& anon_blk)
{
     anon_blk.body.visit_with(*this);
}

void Semantic_Visitor::visit(Interruptor& interruptor)
{
     if(interruptor.returned_expression)
          interruptor.returned_expression->visit_with(*this);
     
     if(interruptor.level == CONTINUE && !in_loop)
          report_error("Continue outside loop",&interruptor);
     if(interruptor.level == BREAK && !in_loop && !in_switch)
          report_error("Break outside loop or switch",&interruptor);
}
