#include "Pasithean_Echo_Visitor.hpp"

#include "philotes/Decl_List.hpp"
#include "philotes/Statement_List.hpp"
#include "philotes/Statement.hpp"
#include "philotes/Expression.hpp"

#include "philotes/philotes.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <unordered_map>

using std::find_if;
using std::endl;
using std::strlen;
using std::to_string;
using std::unordered_map;

const int STYLENUM = 11;
const Fl_Text_Display::Style_Table_Entry styletable[] = { // Style table
     { FL_BLACK, FL_COURIER, FL_NORMAL_SIZE }, // A - Plain
     { FL_BLACK, FL_COURIER_ITALIC, FL_NORMAL_SIZE }, // B - Comments
     { FL_BLACK, FL_COURIER, FL_NORMAL_SIZE }, // C - non-type keywords
     
     { FL_DARK_CYAN, FL_COURIER_BOLD, FL_NORMAL_SIZE }, // D - type keywords
     { FL_DARK_CYAN, FL_COURIER_ITALIC, FL_NORMAL_SIZE }, // E - type modifiers
     { FL_DARK_BLUE, FL_COURIER_BOLD, FL_NORMAL_SIZE }, //F - Variable/parameter declarations
     { FL_DARK_BLUE, FL_COURIER, FL_NORMAL_SIZE }, // G - Bound variable references
     
     { FL_RED, FL_COURIER, FL_NORMAL_SIZE }, // H(ell) - Recent autocorrections; syntactically incorrect literals (like multiple chars in single quotes)

     { FL_DARK_GREEN, FL_COURIER, FL_NORMAL_SIZE }, // I - Literals

     { FL_DARK_MAGENTA, FL_COURIER_BOLD_ITALIC, FL_NORMAL_SIZE }, // J - Function declarations
     { FL_DARK_MAGENTA, FL_COURIER_ITALIC, FL_NORMAL_SIZE }, // K - Bound function calls
     };

void Pasithean_Echo_Visitor::do_whitespace(const AST_Node& node)
{
     if(current_line > node.line)
     {
          current_line = node.line;
          last_indented_line = -1;
     }
     else if(current_line < node.line)
     {
          tout << endl;
          hout << endl;
          current_line = node.line;
     }

     do_indent();
}

void Pasithean_Echo_Visitor::do_indent()
{
     if(last_indented_line < current_line)
     {
          for(int i=0; i<scope_level; i++)
          {
               tout << "    ";
               hout << "AAAA";
          }
          last_indented_line = current_line;
     }
     else if(!disable_all_indentation)
     {
          tout << ' ';
          hout << 'A';
     }
     else
          disable_all_indentation = false;
}

void Pasithean_Echo_Visitor::handle_modifiers(const Symbol_Modifiers& modifiers)
{
     switch(modifiers.visibility)
     {
     case PUBLIC: tout << "public "; hout << "EEEEEEA"; break;
     case PROTECTED: tout << "protected "; hout << "EEEEEEEEEA"; break;
     case PRIVATE: tout << "private "; hout << "EEEEEEEA"; break;
     }
     if(modifiers.const_m)
     {
          tout << "const ";
          hout << "EEEEEA";
     }
     if(modifiers.static_m)
     {
          tout << "static ";
          hout << "EEEEEEA";
     }
     if(modifiers.final_m)
     {
          tout << "final ";
          hout << "EEEEEA";
     }
}

void Pasithean_Echo_Visitor::hout_style_for_length_of(string s, char c)
{
     for(int i=0; i<s.length(); i++)
          hout << c;
}

void Pasithean_Echo_Visitor::visit(Decl_List& decl_list)
{
     for(Declaration* decl : decl_list.data)
          decl->visit_with(*this);
}

void Pasithean_Echo_Visitor::visit(Include_Decl& include_decl)
{
     do_whitespace(include_decl);
     tout << "#include <" << include_decl.included_file << ">"; //TODO: non-system includes
     hout << "CCCCCCCCAC";
     hout_style_for_length_of(include_decl.included_file,'I');
     hout << 'A';
}

void Pasithean_Echo_Visitor::visit(Func_Decl& func_decl)
{
     do_whitespace(func_decl);
     Symbol_Modifiers modded_modifiers = func_decl.modifiers;
     modded_modifiers.const_m = false;
     if(func_decl.return_value_is_const)
     {
          tout << "const ";
          hout << "EEEEEA";
     }
     handle_modifiers(modded_modifiers);
     Type* ret_type = modded_modifiers.static_type->parameters.front();
     if(ret_type)
     {
          tout << ret_type->name << ' ';
          hout_style_for_length_of(ret_type->name,'D'); hout << 'A';
     }
     tout << func_decl.name << '(';
     hout_style_for_length_of(func_decl.name,'J'); hout << 'A';
     bool first_param = true;
     for(Var_Decl_Statement* param : func_decl.modifiers.parameters)
     {
          if(!first_param)
          {
               tout << ", ";
               hout << "AA";
          }
          else
               first_param = false;

          string unscoped = unscope(param->modifiers.static_type->name);
          tout << unscoped << ' ' << param->name;
          hout_style_for_length_of(unscoped,'D');
          hout << 'A';
          hout_style_for_length_of(param->name,'F');
     }
     tout << ")\n";
     hout << "A\n";
     current_line++;
     do_indent();
     tout << "{\n";
     hout << "A\n";
     current_line++; scope_level++;
     func_decl.body.visit_with(*this);
     scope_level--;
     do_indent();
     tout << "}\n";
     hout << "A\n";
     current_line++;
}

void Pasithean_Echo_Visitor::visit(Import_Decl& import_decl)
{
     do_whitespace(import_decl);
     tout << "import " << import_decl.imported_class << ";\n";
     hout << "CCCCCCA";
     hout_style_for_length_of(import_decl.imported_class,'D');
     hout << "A\n";
     current_line++;
}

void Pasithean_Echo_Visitor::visit(Class_Decl& class_decl)
{
     do_whitespace(class_decl);
     if(language_mode==JAVA_MODE)
     {
          tout << "public ";
          hout << "EEEEEEA";
     }
     switch(class_decl.type->aggregate_type)
     {
     case CLASS: tout << "class "; hout << "CCCCCA"; break;
     case STRUCT: tout << "struct "; hout << "CCCCCCA"; break;
     case UNION: tout << "union "; hout << "CCCCCA"; break;
     }
     string unscoped = unscope(class_decl.type->name);
     tout << unscoped;
     hout_style_for_length_of(unscoped,'D');
     if(class_decl.type->base_or_parent_types.size())
     {
          tout << ' ';
          hout << 'A';
          bool first_parent = true;
          for(Type* parent : class_decl.type->base_or_parent_types)
               if(language_mode==JAVA_MODE)
               {
                    if(parent->name!="" && parent->name!="Object")
                    {
                         tout << "extends " << parent->name;
                         hout << "CCCCCCC ";
                         hout_style_for_length_of(parent->name,'D');
                    }
               }
               else
                    if(!first_parent)
                    {
                         tout << ", public " << parent->name;
                         hout << "AAEEEEEEA";
                         hout_style_for_length_of(parent->name,'D');
                    }
                    else
                    {
                         tout << ": public " << parent->name;
                         hout << "AAEEEEEEA";
                         hout_style_for_length_of(parent->name,'D');
                         first_parent = false;
                    }
     }
     tout << endl;
     hout << endl;
     current_line++;
     do_indent();
     tout << "{\n";
     hout << "A\n";
     current_line++;
     scope_level++;
     class_decl.body.visit_with(*this);
     scope_level--;
     do_indent();
     tout << '}' << (language_mode==CPP_MODE ? ";\n" : "\n");
     hout << 'A' << (language_mode==CPP_MODE ? "A\n" : "\n");
     current_line++;
}

void Pasithean_Echo_Visitor::visit(Class_Decl_Instantiated& class_decl)
{
     assert(false); //TODO
}


void Pasithean_Echo_Visitor::visit(Function_Call& func_call)
{
     const static auto BINARY_OPERATORS = {"+","-","*","/","%","<<",">>","&","|",",","^","||","&&","<","<=","==",">=",">","!=","=","+=","-=","*=","/=","%=","&=","|=","^=","<<=",">>="};
     auto x = find_if(BINARY_OPERATORS.begin(),BINARY_OPERATORS.end(),[&](auto& c) { return func_call.called_func_name == c; });
     if(x!=BINARY_OPERATORS.end() && func_call.parameters.size()==2)
     {
          func_call.parameters.front()->visit_with(*this);
          tout << ' ' << func_call.called_func_name << ' ';
          hout << 'A';
          hout_style_for_length_of(func_call.called_func_name,'K');
          hout << 'A';
          func_call.parameters.back()->visit_with(*this);
          return;
     }

     const static auto NORMAL_UNARY_OPERATORS = {"*","&","-","+"};
     x = find_if(NORMAL_UNARY_OPERATORS.begin(),NORMAL_UNARY_OPERATORS.end(),[&](auto& c) { return func_call.called_func_name == c; });
     if(x!=NORMAL_UNARY_OPERATORS.end() && func_call.parameters.size()==1)
     {
          tout << func_call.called_func_name;
          hout_style_for_length_of(func_call.called_func_name,'K');
          func_call.parameters.front()->visit_with(*this);
          return;
     }

     if(func_call.called_func_name=="++_" || func_call.called_func_name=="--_")
     {
          tout << func_call.called_func_name.substr(0,2);
          hout_style_for_length_of(func_call.called_func_name.substr(0,2),'K');
          func_call.parameters.front()->visit_with(*this);
          return;
     }
     if(func_call.called_func_name=="_++" || func_call.called_func_name=="_--")
     {
          func_call.parameters.front()->visit_with(*this);
          tout << func_call.called_func_name.substr(1);
          hout_style_for_length_of(func_call.called_func_name.substr(1),'K');
          return;
     }
     if(func_call.called_func_name[0]=='(')
     {
          tout << func_call.called_func_name;
          hout_style_for_length_of(func_call.called_func_name,'K');
          func_call.parameters.front()->visit_with(*this);
          return;
     }
     if(func_call.called_func_name=="!_")
     {
          tout << func_call.called_func_name[0];
          hout << 'K';
          func_call.parameters.front()->visit_with(*this);
          return;
     }

     if(func_call.new_call)
     {
          tout << "new ";
          hout << "KKKA";
     }

     if(func_call.called_func_name.find('[')!=-1)
     {
          tout << func_call.called_func_name.substr(0,func_call.called_func_name.length()-2) << '[';
          hout_style_for_length_of(func_call.called_func_name.substr(0,func_call.called_func_name.length()-2),'D');
          hout << 'A';
     }
     else
     {
          tout << func_call.called_func_name << '(';
          hout_style_for_length_of(func_call.called_func_name,'K');
          hout << 'A';
     }

     bool first_param = true;
     for(Expression* parameter : func_call.parameters)
     {
          if(!first_param)
          {
               tout << ',';
               hout << 'A';
          }
          else
               first_param = false;
          parameter->visit_with(*this);
     }

     if(func_call.called_func_name.find('[')!=-1)
     {
          tout << ']';
          hout << 'A';
     }
     else
     {
          tout << ')';
          hout << 'A';
     }
}

void Pasithean_Echo_Visitor::visit(Aggregate_Access& ag_access)
{
     ag_access.base_expression->visit_with(*this);
     switch(ag_access.access_form)
     {
     case DOT: tout << '.'; hout << 'A'; break;
     case ARROW: tout << "->"; hout << "AA"; break;
     case SCOPE: tout << "::"; hout << "AA"; break;
     }
     tout << ag_access.component_name;
     hout_style_for_length_of(ag_access.component_name,'G');
}

void Pasithean_Echo_Visitor::visit(Array_Access& ary_access)
{
     ary_access.base_expression->visit_with(*this);
     tout << '[';
     hout << 'A';
     ary_access.subscript_expression->visit_with(*this);
     tout << ']';
     hout << 'A';
}

void Pasithean_Echo_Visitor::visit(Ternary_Conditional& ternary_expr)
{
     ternary_expr.conditional->visit_with(*this);
     tout << " ? ";
     hout << "AAA";
     ternary_expr.val_if_cond_true->visit_with(*this);
     tout << " : ";
     hout << "AAA";
     ternary_expr.val_if_cond_false->visit_with(*this);
}

void Pasithean_Echo_Visitor::visit(Parenthetical& parenthetical)
{
     tout << "(";
     hout << "A";
     parenthetical.nested->visit_with(*this);
     tout << ")";
     hout << "A";
}

void Pasithean_Echo_Visitor::visit(Atomic_Expression& atomic_expr)
{
     const static unordered_map<char,char> escape_map = {{'\n','n'},{'\'','\''},{'\t','t'},{'\0','0'}};
     
     char char_literal;
     const char* cstr_literal;
     string str_literal;
     switch(atomic_expr.literal_status)
     {
     case 0: tout << atomic_expr.value.as_string();
          hout_style_for_length_of(atomic_expr.value.as_string(),'G');
          break;
     case STR_LITERAL:
          tout << '"';
          hout << 'I';
          cstr_literal = atomic_expr.value.as_string();
          for(int i=0; i<strlen(cstr_literal); i++)
               if(escape_map.count(cstr_literal[i]))
               {
                    str_literal+='\\';
                    str_literal+=escape_map.at(cstr_literal[i]);
               }
               else
                    str_literal+=cstr_literal[i];
          tout << str_literal;
          hout_style_for_length_of(str_literal,'I');
          tout << '"';
          hout << 'I';
          break;
     case BOOLEAN_LITERAL:
          tout << (atomic_expr.value.numeric_val ? "true" : "false");
          hout_style_for_length_of((atomic_expr.value.numeric_val ? "true" : "false"),'I');
          break;
     case CHAR_LITERAL:
          char_literal = static_cast<char>(atomic_expr.value.numeric_val);
          tout << '\'';
          hout << 'I';
          if(escape_map.count(char_literal))
          {
               tout << '\\'; hout << 'I';
               tout << escape_map.at(char_literal);
          }
          else
               tout << char_literal;
          hout << 'I';
          tout << '\'';
          hout << 'I';
          break;
     case INT_LITERAL: tout << atomic_expr.value.numeric_val;
          hout_style_for_length_of(to_string(atomic_expr.value.numeric_val),'I');
          break;
     case FLOAT_LITERAL: tout << to_string(atomic_expr.value.float_val) << 'f';
          hout_style_for_length_of(to_string(atomic_expr.value.float_val),'I');
          hout << 'I';
          break;
     case DOUBLE_LITERAL: tout << to_string(atomic_expr.value.float_val);
          hout_style_for_length_of(to_string(atomic_expr.value.float_val),'I');
          break;
     }
}

void Pasithean_Echo_Visitor::visit(Statement_List& stmt_list)
{
     recurse_level++;
     last_stmtlist_singleton = stmt_list.data.size()==1;
     for(Statement* stmt : stmt_list.data)
     {
          if(dynamic_cast<Expression*>(stmt))
               do_whitespace(*stmt);
          stmt->visit_with(*this);
          if(dynamic_cast<Expression*>(stmt))
          {
               tout << ';';
               hout << 'A';
          }
          if(!last_stmtlist_singleton || !recurse_level)
          {
               tout << endl;
               hout << endl;
               current_line++;
          }
          last_stmtlist_singleton = stmt_list.data.size()==1;
     }
     recurse_level--;
}

void Pasithean_Echo_Visitor::visit(Label& label)
{
     do_whitespace(label);
     if(label.case_mod)
     {
          tout << "case ";
          hout << "CCCCA";
          label.expr->visit_with(*this);
     }
     else
     {
          tout << label.name;
          hout_style_for_length_of(label.name,'I');
     }
     tout << ':';
     hout << 'A';
}

void Pasithean_Echo_Visitor::visit(Var_Decl_Statement& vdecl_stmt)
{
     do_whitespace(vdecl_stmt);
     handle_modifiers(vdecl_stmt.modifiers);
     tout << unscope(vdecl_stmt.modifiers.static_type->name) << ' ';
     hout_style_for_length_of(unscope(vdecl_stmt.modifiers.static_type->name),'D');
     hout << 'A';
     tout << vdecl_stmt.name;
     hout_style_for_length_of(vdecl_stmt.name,'F');
     if(vdecl_stmt.init_expr)
     {
          tout << " = ";
          hout << "ACA";
          vdecl_stmt.init_expr->visit_with(*this);
     }
     tout << ';';
     hout << 'A';
}

void Pasithean_Echo_Visitor::visit(Sentinel& cursor)
{
     do_whitespace(cursor);
     tout << "#cursor";
     hout << "CCCCCCC";
}

void Pasithean_Echo_Visitor::visit(Comment& comment)
{
     do_whitespace(comment);
     if(comment.text[comment.text.length()-1]=='\n')
     {
          tout << comment.text.substr(0,comment.text.length()-1);
          hout_style_for_length_of(comment.text.substr(0,comment.text.length()-1),'B');
     }
     else
     {
          tout << comment.text;
          hout_style_for_length_of(comment.text,'B');
          current_line += count(comment.text.begin(),comment.text.end(),'\n');
     }
}

void Pasithean_Echo_Visitor::visit(If_Statement& if_stmt)
{
     do_whitespace(if_stmt);
     tout << "if(";
     hout << "CCA";
     if_stmt.condition->visit_with(*this);
     tout << ")\n";
     hout << "A\n";
     current_line++;
     if(false /*!!!*/ && if_stmt.if_branch.data.size()==1 && !dynamic_cast<Sentinel*>(if_stmt.if_branch.data.front()) && !dynamic_cast<If_Statement*>(if_stmt.if_branch.data.front()))
     {
          scope_level++;
          if_stmt.if_branch.visit_with(*this);
          scope_level--;
     }
     else
     {
          do_indent();
          tout << "{\n";
          hout << "A\n";
          current_line++; scope_level++;
          int old_recurse = recurse_level;
          recurse_level = -1;
          if_stmt.if_branch.visit_with(*this);
          recurse_level = old_recurse;
          scope_level--; current_line++;
          do_indent();
          tout << '}';
          hout << 'A';
          tout << "\n";
          hout << "\n";
          current_line++;
     }

     if(if_stmt.else_branch.data.size())
     {
          do_indent();
          tout << "else";
          hout << "CCCC";

          //Newline unless if-else-if...
          if(dynamic_cast<If_Statement*>(if_stmt.else_branch.data.front()) && if_stmt.else_branch.data.size()==1)
               scope_level--; //...and hack scope level for if-else-if
          else
          {
               tout << "\n";
               hout << "\n";
               current_line++;
          }
     
          /*!!!*/ //if(if_stmt.else_branch.data.size()==1 && !dynamic_cast<Sentinel*>(if_stmt.else_branch.data.front()))
          if(if_stmt.else_branch.data.size()==1 && dynamic_cast<If_Statement*>(if_stmt.else_branch.data.front()))
          {
               current_line = last_indented_line = if_stmt.else_branch.data.front()->line;
               scope_level++;
               if_stmt.else_branch.visit_with(*this);
               scope_level--;
          }
          else
          {
               do_indent();
               tout << "{\n";
               hout << "A\n";
               current_line++; scope_level++;
               int old_recurse = recurse_level;
               recurse_level = -1;
               if_stmt.else_branch.visit_with(*this);
               recurse_level = old_recurse;
               scope_level--; current_line++;
               do_indent();
               tout << "}\n";
               hout << "A\n";
               current_line++;
          }

          //If we were if-else-if, unhack scope level
          if(dynamic_cast<If_Statement*>(if_stmt.else_branch.data.front()) && if_stmt.else_branch.data.size()==1)
               scope_level++;
     }
}

void Pasithean_Echo_Visitor::visit(While_Statement& while_stmt)
{
     do_whitespace(while_stmt);
     if(!while_stmt.do_while)
     {
          tout << "while(";
          hout << "CCCCCA";
          while_stmt.condition->visit_with(*this);
          tout << ")\n";
          hout << "A\n";
          current_line++;
          if(false /*!!!*/ && while_stmt.statements.data.size()==1 && !dynamic_cast<Sentinel*>(while_stmt.statements.data.front()))
          {
               scope_level++;
               while_stmt.statements.visit_with(*this);
               scope_level--;
          }
          else
          {
               do_indent();
               tout << "{\n";
               hout << "A\n";
               current_line++; scope_level++;
               int old_recurse = recurse_level;
               recurse_level = -1;
               while_stmt.statements.visit_with(*this);
               recurse_level = old_recurse;
               scope_level--;
               do_indent();
               tout << "}\n";
               hout << "A\n";
               current_line++;
          }
     }
     else
     {
          tout << "do\n";
          hout << "CC\n";
          current_line++;
          do_indent();
          tout << "{\n";
          hout << "A\n";
          current_line++; scope_level++;
          while_stmt.statements.visit_with(*this);
          scope_level--;
          do_indent();
          tout << "} while(";
          hout << "AACCCCCA";
          while_stmt.condition->visit_with(*this);
          tout << ");";
          hout << "AA";
     }
}

void Pasithean_Echo_Visitor::visit(For_Statement& for_stmt)
{
     do_whitespace(for_stmt);
     tout << "for(";
     hout << "CCCA";
     if(for_stmt.init_stmt)
     {
          disable_all_indentation = true;
          for_stmt.init_stmt->visit_with(*this);
          tout << ' ';
          hout << 'A';
          disable_all_indentation = false;
     }
     if(!for_stmt.init_stmt || dynamic_cast<Expression*>(for_stmt.init_stmt))
     {
          tout << "; ";
          hout << "AA";
     }
     for_stmt.condition->visit_with(*this);
     tout << "; ";
     hout << "AA";
     if(for_stmt.post)
          for_stmt.post->visit_with(*this);
     tout << ")\n";
     hout << "A\n";
     current_line++;
     
     if(false /*!!!*/ && for_stmt.statements.data.size()==1)
     {
          scope_level++;
          for_stmt.statements.visit_with(*this);
          scope_level--;
     }
     else
     {
          do_indent();
          scope_level++;
          tout << "{\n";
          hout << "A\n";
          current_line++;
          int old_recurse = recurse_level;
          recurse_level = -1;
          for_stmt.statements.visit_with(*this);
          recurse_level = old_recurse;
          scope_level--;
          current_line++;
          do_indent();
          tout << "}\n";
          hout << "A\n";
          current_line++;
     }
}

void Pasithean_Echo_Visitor::visit(Switch_Statement& switch_stmt)
{
     do_whitespace(switch_stmt);
     tout << "switch(";
     hout << "CCCCCCA";
     switch_stmt.condition->visit_with(*this);
     tout << ")\n";
     hout << "A\n";
     current_line++;
     do_indent();
     tout << "{\n";
     hout << "A\n";
     current_line++; scope_level++;
     switch_stmt.statements.visit_with(*this);
     scope_level--;
     do_indent();
     tout << '}';
     hout << 'A';
}

void Pasithean_Echo_Visitor::visit(Anonymous_Block& anon_blk)
{
     do_whitespace(anon_blk);
     tout << "{\n";
     hout << "A\n";
     current_line++; scope_level++;
     anon_blk.body.visit_with(*this);
     scope_level--;
     do_indent();
     tout << '}';
     hout << 'A';
}

void Pasithean_Echo_Visitor::visit(Interruptor& interrupt)
{
     do_whitespace(interrupt);
     switch(interrupt.level)
     {
     case RETURN: tout << "return";
          hout << "CCCCCC";
          break;
     case BREAK: tout << "break";
          hout << "CCCCC";
          break;
     case CONTINUE: tout << "continue";
          hout << "CCCCCCCC";
          break;
     }
     if(interrupt.returned_expression)
     {
          tout << ' ';
          hout << 'A';
          interrupt.returned_expression->visit_with(*this);
     }
     tout << ';';
     hout << 'A';
}
