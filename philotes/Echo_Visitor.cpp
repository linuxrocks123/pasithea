#include "Echo_Visitor.hpp"

#include "Decl_List.hpp"
#include "Statement_List.hpp"
#include "Statement.hpp"
#include "Expression.hpp"
#include "philotes.h"

#include <algorithm>
#include <iostream>

using std::cout;
using std::find_if;
using std::endl;

void Echo_Visitor::do_whitespace(const AST_Node& node)
{
     if(current_line > node.line)
     {
          current_line = node.line;
          last_indented_line = -1;
     }
     while(current_line < node.line)
     {
          cout << endl;
          current_line++;
     }

     do_indent();
}

void Echo_Visitor::do_indent()
{
     if(last_indented_line < current_line)
     {
          for(int i=0; i<scope_level; i++)
          {
               cout << "    ";
          }
          last_indented_line = current_line;
     }
     else if(!disable_all_indentation)
          cout << ' ';
     else
          disable_all_indentation = false;
}

void Echo_Visitor::handle_modifiers(const Symbol_Modifiers& modifiers)
{
     switch(modifiers.visibility)
     {
     case PUBLIC: cout << "public "; break;
     case PROTECTED: cout << "protected "; break;
     case PRIVATE: cout << "private "; break;
     }
     if(modifiers.const_m)
          cout << "const ";
     if(modifiers.static_m)
          cout << "static ";
     if(modifiers.final_m)
          cout << "final ";
}

void Echo_Visitor::visit(Decl_List& decl_list)
{
     for(Declaration* decl : decl_list.data)
          decl->visit_with(*this);
}

void Echo_Visitor::visit(Include_Decl& include_decl)
{
     do_whitespace(include_decl);
     cout << "#include <" << include_decl.included_file << ">"; //TODO: non-system includes
}

void Echo_Visitor::visit(Func_Decl& func_decl)
{
     do_whitespace(func_decl);
     Symbol_Modifiers modded_modifiers = func_decl.modifiers;
     modded_modifiers.const_m = false;
     if(func_decl.return_value_is_const)
          cout << "const ";
     handle_modifiers(modded_modifiers);
     Type* ret_type = modded_modifiers.static_type->parameters.front();
     if(ret_type)
          cout << ret_type->name << ' ';
     cout << func_decl.name << '(';
     bool first_param = true;
     for(Var_Decl_Statement* param : func_decl.modifiers.parameters)
     {
          if(!first_param)
               cout << ", ";
          else
               first_param = false;
          cout << unscope(param->modifiers.static_type->name) << ' ' << param->name;
     }
     cout << ")\n";
     current_line++;
     do_indent();
     cout << "{\n";
     current_line++; scope_level++;
     func_decl.body.visit_with(*this);
     scope_level--;
     do_indent();
     cout << "}\n";
     current_line++;
}

void Echo_Visitor::visit(Import_Decl& import_decl)
{
     do_whitespace(import_decl);
     cout << "import " << import_decl.imported_class << ";\n";
     current_line++;
}

void Echo_Visitor::visit(Class_Decl& class_decl)
{
     do_whitespace(class_decl);
     if(language_mode==JAVA_MODE)
          cout << "public ";
     switch(class_decl.type->aggregate_type)
     {
     case CLASS: cout << "class "; break;
     case STRUCT: cout << "struct "; break;
     case UNION: cout << "union "; break;
     }
     cout << unscope(class_decl.type->name);
     if(class_decl.type->base_or_parent_types.size())
     {
          cout << ' ';
          bool first_parent = true;
          for(Type* parent : class_decl.type->base_or_parent_types)
               if(language_mode==JAVA_MODE)
               {
                    if(parent->name!="" && parent->name!="Object")
                         cout << "extends " << parent->name;
               }
               else
                    if(!first_parent)
                         cout << ", public " << parent->name;
                    else
                    {
                         cout << ": public " << parent->name;
                         first_parent = false;
                    }
     }
     cout << endl;
     current_line++;
     do_indent();
     cout << "{\n";
     current_line++;
     scope_level++;
     class_decl.body.visit_with(*this);
     scope_level--;
     do_indent();
     cout << '}' << (language_mode==CPP_MODE ? ";\n" : "\n");
     current_line++;
}

void Echo_Visitor::visit(Class_Decl_Instantiated& class_decl)
{
     assert(false); //TODO
}


void Echo_Visitor::visit(Function_Call& func_call)
{
     const static auto BINARY_OPERATORS = {"+","-","*","/","%","<<",">>","&","|",",","^","||","&&","<","<=","==",">=",">","!=","=","+=","-=","*=","/=","%=","&=","|=","^=","<<=",">>="};
     auto x = find_if(BINARY_OPERATORS.begin(),BINARY_OPERATORS.end(),[&](auto& c) { return func_call.called_func_name == c; });
     if(x!=BINARY_OPERATORS.end() && func_call.parameters.size()==2)
     {
          func_call.parameters.front()->visit_with(*this);
          cout <<  ' ' << func_call.called_func_name << ' ';
          func_call.parameters.back()->visit_with(*this);
          return;
     }

     const static auto NORMAL_UNARY_OPERATORS = {"*","&","-","+"};
     x = find_if(NORMAL_UNARY_OPERATORS.begin(),NORMAL_UNARY_OPERATORS.end(),[&](auto& c) { return func_call.called_func_name == c; });
     if(x!=NORMAL_UNARY_OPERATORS.end() && func_call.parameters.size()==1)
     {
          cout << func_call.called_func_name;
          func_call.parameters.front()->visit_with(*this);
          return;
     }

     if(func_call.called_func_name=="++_" || func_call.called_func_name=="--_")
     {
          cout << func_call.called_func_name.substr(0,2);
          func_call.parameters.front()->visit_with(*this);
          return;
     }
     if(func_call.called_func_name=="_++" || func_call.called_func_name=="_--")
     {
          func_call.parameters.front()->visit_with(*this);
          cout << func_call.called_func_name.substr(1);
          return;
     }
     if(func_call.called_func_name[0]=='(')
     {
          cout << func_call.called_func_name;
          func_call.parameters.front()->visit_with(*this);
          return;
     }

     if(func_call.new_call)
          cout << "new ";

     if(func_call.called_func_name.find('[')!=-1)
          cout << func_call.called_func_name.substr(0,func_call.called_func_name.length()-2) << '[';
     else
          cout << func_call.called_func_name << '(';

     bool first_param = true;
     for(Expression* parameter : func_call.parameters)
     {
          if(!first_param)
               cout << ',';
          else
               first_param = false;
          parameter->visit_with(*this);
     }

     if(func_call.called_func_name.find('[')!=-1)
          cout << ']';
     else
          cout << ')';
}

void Echo_Visitor::visit(Aggregate_Access& ag_access)
{
     ag_access.base_expression->visit_with(*this);
     switch(ag_access.access_form)
     {
     case DOT: cout << '.'; break;
     case ARROW: cout << "->"; break;
     case SCOPE: cout << "::"; break;
     }
     cout << ag_access.component_name;
}

void Echo_Visitor::visit(Array_Access& ary_access)
{
     ary_access.base_expression->visit_with(*this);
     cout << '[';
     ary_access.subscript_expression->visit_with(*this);
     cout << ']';
}

void Echo_Visitor::visit(Ternary_Conditional& ternary_expr)
{
     ternary_expr.conditional->visit_with(*this);
     cout << " ? ";
     ternary_expr.val_if_cond_true->visit_with(*this);
     cout << " : ";
     ternary_expr.val_if_cond_false->visit_with(*this);
}

void Echo_Visitor::visit(Parenthetical& parenthetical)
{
     cout << "(";
     parenthetical.nested->visit_with(*this);
     cout << ")";
}

void Echo_Visitor::visit(Atomic_Expression& atomic_expr)
{
     switch(atomic_expr.literal_status)
     {
     case 0: cout << atomic_expr.value.as_string(); break;
     case STR_LITERAL:
          cout << '"';
          cout << atomic_expr.value.as_string();
          cout << '"';
          break;
     case BOOLEAN_LITERAL:
          cout << (atomic_expr.value.numeric_val ? "true" : "false");
          break;
     case CHAR_LITERAL:
          cout << '\'';
          cout << static_cast<char>(atomic_expr.value.numeric_val);
          cout << '\'';
          break;
     case INT_LITERAL: cout << atomic_expr.value.numeric_val; break;
     case FLOAT_LITERAL: cout << atomic_expr.value.float_val << 'f'; break;
     case DOUBLE_LITERAL: cout << atomic_expr.value.float_val; break;
     }
}

void Echo_Visitor::visit(Statement_List& stmt_list)
{
     for(Statement* stmt : stmt_list.data)
     {
          if(dynamic_cast<Expression*>(stmt))
               do_whitespace(*stmt);
          stmt->visit_with(*this);
          if(dynamic_cast<Expression*>(stmt))
               cout << ';';
     }
     cout << endl;
     current_line++;
}

void Echo_Visitor::visit(Label& label)
{
     do_whitespace(label);
     if(label.case_mod)
     {
          cout << "case ";
          label.expr->visit_with(*this);
     }
     else
          cout << label.name;
     cout << ':';
}

void Echo_Visitor::visit(Var_Decl_Statement& vdecl_stmt)
{
     do_whitespace(vdecl_stmt);
     handle_modifiers(vdecl_stmt.modifiers);
     cout << vdecl_stmt.modifiers.static_type->name << ' ';
     cout << vdecl_stmt.name;
     if(vdecl_stmt.init_expr)
     {
          cout << " = ";
          vdecl_stmt.init_expr->visit_with(*this);
     }
     cout << ';';
}

void Echo_Visitor::visit(Sentinel& cursor)
{
     do_whitespace(cursor);
     cout << "#cursor";
}

void Echo_Visitor::visit(Comment& comment)
{
     do_whitespace(comment);
     cout << comment.text;
}

void Echo_Visitor::visit(If_Statement& if_stmt)
{
     do_whitespace(if_stmt);
     cout << "if(";
     if_stmt.condition->visit_with(*this);
     cout << ")\n";
     current_line++;
     if(if_stmt.if_branch.data.size()==1 && !dynamic_cast<Sentinel*>(if_stmt.if_branch.data.front()))
     {
          scope_level++;
          if_stmt.if_branch.visit_with(*this);
          scope_level--;
     }
     else
     {
          do_indent();
          cout << "{\n";
          current_line++; scope_level++;
          if_stmt.if_branch.visit_with(*this);
          scope_level--;
          do_indent();
          cout << "}\n";
          current_line++;
     }

     if(if_stmt.else_branch.data.size())
     {
          do_indent();
          cout << "else\n";
          current_line++;
     
          if(if_stmt.else_branch.data.size()==1 && !dynamic_cast<Sentinel*>(if_stmt.else_branch.data.front()))
          {
               scope_level++;
               if_stmt.else_branch.visit_with(*this);
               scope_level--;
          }
          else
          {
               do_indent();
               cout << "{\n";
               current_line++; scope_level++;
               if_stmt.else_branch.visit_with(*this);
               scope_level--;
               do_indent();
               cout << "}\n";
               current_line++;
          }
     }
}

void Echo_Visitor::visit(While_Statement& while_stmt)
{
     do_whitespace(while_stmt);
     if(!while_stmt.do_while)
     {
          cout << "while(";
          while_stmt.condition->visit_with(*this);
          cout << ")\n";
          current_line++;
          if(while_stmt.statements.data.size()==1 && !dynamic_cast<Sentinel*>(while_stmt.statements.data.front()))
          {
               scope_level++;
               while_stmt.statements.visit_with(*this);
               scope_level--;
          }
          else
          {
               do_indent();
               cout << "{\n";
               current_line++; scope_level++;
               while_stmt.statements.visit_with(*this);
               scope_level--;
               do_indent();
               cout << "}\n";
               current_line++;
          }
     }
     else
     {
          cout << "do\n";
          current_line++;
          do_indent();
          cout << "{\n";
          current_line++; scope_level++;
          while_stmt.statements.visit_with(*this);
          scope_level--;
          do_indent();
          cout << "} while(";
          while_stmt.condition->visit_with(*this);
          cout << ");";
     }
}

void Echo_Visitor::visit(For_Statement& for_stmt)
{
     do_whitespace(for_stmt);
     cout << "for(";
     if(for_stmt.init_stmt)
     {
          disable_all_indentation = true;
          for_stmt.init_stmt->visit_with(*this);
          cout << ' ';
     }
     else
          cout << "; ";
     for_stmt.condition->visit_with(*this);
     cout << "; ";
     if(for_stmt.post)
          for_stmt.post->visit_with(*this);
     cout << ")\n";
     current_line++;
     do_indent();
     cout << "{\n";
     current_line++; scope_level++;
     for_stmt.statements.visit_with(*this);
     scope_level--;
     do_indent();
     cout << "}\n";
     current_line++;
}

void Echo_Visitor::visit(Switch_Statement& switch_stmt)
{
     do_whitespace(switch_stmt);
     cout << "switch(";
     switch_stmt.condition->visit_with(*this);
     cout << ")\n";
     current_line++;
     do_indent();
     cout << "{\n";
     current_line++; scope_level++;
     switch_stmt.statements.visit_with(*this);
     scope_level--;
     do_indent();
     cout << "}\n";
}

void Echo_Visitor::visit(Anonymous_Block& anon_blk)
{
     do_whitespace(anon_blk);
     cout << "{\n";
     current_line++; scope_level++;
     anon_blk.body.visit_with(*this);
     scope_level--;
     do_indent();
     cout << "}\n";
}

void Echo_Visitor::visit(Interruptor& interrupt)
{
     do_whitespace(interrupt);
     switch(interrupt.level)
     {
     case RETURN: cout << "return";
          break;
     case BREAK: cout << "break";
          break;
     case CONTINUE: cout << "continue";
          break;
     }
     if(interrupt.returned_expression)
     {
          cout << ' ';
          interrupt.returned_expression->visit_with(*this);
     }
     cout << ";\n";
}
