#include "Pasithean_Interpreter.hpp"
#include "philotes/Expression.hpp"
#include "philotes/Statement.hpp"
#include "philotes/philotes.h"
#include "philotes/symtab.hpp"

#include <sys/time.h>
#include <iostream>
#include <sstream>

#include <FL/fl_ask.H>

using std::endl;
using std::istream;
using std::ostringstream;
using std::stoi;
using std::to_string;

void Pasithean_Interpreter::visit(Decl_List& decl_list)
{
     /*Reset pi_sout and diagnostics_map iff we're being called directly 
       by the user; equivalently, iff we're not a recursive call.*/
     if(!depth)
     {
          pi_sin.seekg(0);
          pi_sout.str("");
          infinite_loop_detected = false;
          diagnostics_map.clear();
     }

     depth++;
     try
     {
          Hypnos_Visitor::visit(decl_list);
     }
     catch(const char* msg)
     {
          last_expr_eval = NULL;
          while(!call_stack.empty())
               call_stack.pop();
          ref_params_for_prayer.clear();
          depth = 0;
          throw;
     }
     depth--;
}

void Pasithean_Interpreter::handle_prayer()
{
     Function_Call* called_func = call_stack.top();
     Symbol* tthis = this_stack.top();
     
     string n = called_func->called_func_name;
     if(n!="System.out.println" && n!="System.out.print")
          n = n.substr(n.find(".")+1);

     if(n!="nextInt" && n!="nextFloat" && n!="nextDouble" && n!="next" && n!="nextLine" && n!="currentTimeMillis")
     {
          Hypnos_Visitor::handle_prayer();
          return;
     }

     if(n=="currentTimeMillis")
     {
          static bool seeded = false;
          static timeval retval;
          if(!seeded)
          {
               gettimeofday(&retval,NULL);
               seeded = true;
          }
          Atomic_Expression* rvalue_exp = new Atomic_Expression{0,Value{}};
          rvalue_exp->value.numeric_val = retval.tv_sec*1000 + retval.tv_usec/1000;
          last_expr_eval = rvalue_exp;
          return;
     }

     if(static_cast<istream*>(tthis->components[".ifstream"].value.ary.ptr)!=&pi_sin)
     {
          handle_prayer();
          return;
     }

     if(n=="nextInt" || n=="nextFloat" || n=="nextDouble")
     {
          string remaining = pi_sin.str().substr(pi_sin.tellg());
          int first_invalid_char_pos = remaining.find_first_not_of(" \t\n.0123456789");
          int first_num_char_pos = remaining.find_first_of("0123456789");
          int relevant_newline = remaining.find("\n",first_num_char_pos!=string::npos ? first_num_char_pos : 0);
          if(first_invalid_char_pos!=string::npos && first_invalid_char_pos < relevant_newline)
               throw "Runtime error: invalid input";
          if(first_num_char_pos==string::npos)
          {
               pi_sin << fl_input("Your program requested user input: ","") << endl;
               handle_prayer();
               return;
          }
     }

     if(n=="next")
     {
          string remaining = pi_sin.str().substr(pi_sin.tellg());
          if(remaining.find_first_not_of(" \t\n")==string::npos)
          {
               pi_sin << fl_input("Your program requested user input: ","") << endl;
               handle_prayer();
               return;
          }
     }

     if(n=="nextLine")
          if(pi_sin.tellg()==pi_sin.str().length())
          {
               pi_sin << fl_input("Your program requested user input: ","") << endl;
               handle_prayer();
               return;
          }

     Hypnos_Visitor::handle_prayer();
     switch(last_expr_eval->literal_status)
     {
     case INT_LITERAL: pi_sout << last_expr_eval->value.numeric_val << endl;
          break;
     case FLOAT_LITERAL: case DOUBLE_LITERAL:
          pi_sout << last_expr_eval->value.float_val << endl;
          break;
     case STR_LITERAL:
          pi_sout << static_cast<char*>(last_expr_eval->value.ary.ptr) << endl;
          break;
     default: assert(false);
          break;
     }
}

static string sym_as_str(const Symbol& symbol)
{
     const Type& type = *symbol.type;
     const Value& value = symbol.value;

     const string& tn = type.name;
     if(tn=="byte" || tn=="short" || tn=="int" || tn=="long")
          return to_string(value.numeric_val);
     if(tn=="char")
          return string(1,static_cast<char>(value.numeric_val));
     if(tn=="float" || tn=="double")
          return to_string(value.float_val);
     if(tn=="String")
          return static_cast<char*>(value.ary.ptr);
     return "...";
}

static void update_diagnostics_map(map<string,list<string>>& to_update)
{
     //Deal with severances stack
     while(severances.size() && severances.top() >= symtab.size())
          severances.pop();

     int lowest_visible_index = severances.empty() ? 1 : severances.top();
     auto loc_sym_it = symtab.rbegin();
     for(int i=symtab.size()-1; i>=lowest_visible_index; i--,++loc_sym_it)
          for(const auto& sym_entry : *loc_sym_it)
               if(sym_entry.second.type)
                    to_update[sym_entry.first].push_back(sym_as_str(sym_entry.second));

     if(this_stack.top())
          for(const auto& component : this_stack.top()->components)
               if(component.second.type)
                    to_update["this."+component.first].push_back(sym_as_str(component.second));
}

void Pasithean_Interpreter::visit(Statement_List& stmt_list)
{
     interrupt_level = 0;
     for(Statement* stmt : stmt_list.data)
     {
          stmt->visit_with(*this);

          //Fill in diagnostics_map for this line
          if(stmt->line)
          {
               update_diagnostics_map(diagnostics_map[stmt->line]);
               if(infinite_loop_detected)
                    diagnostics_map[stmt->line]["!WARNING!"].push_front("Information for this line incomplete due to possible infinite loop.");
          }

          if(interrupt_level)
               break;
     }
}

void Pasithean_Interpreter::visit(While_Statement& while_stmt)
{
     if(while_stmt.do_while)
     {
          symtab.emplace_back();
          while_stmt.statements.visit_with(*this);
          symtab.pop_back();
     }
     
     int iterations = 0;
     while_stmt.condition->visit_with(*this);
     while(rvalue_of(*last_expr_eval).numeric_val)
     {
          if(++iterations > max_iterations)
               infinite_loop_detected = true;
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
          if(iterations > max_iterations)
               break;
     }
}

void Pasithean_Interpreter::visit(For_Statement& for_stmt)
{
     symtab.emplace_back();
     for_stmt.init_stmt->visit_with(*this);
     for_stmt.condition->visit_with(*this);
     int iterations = 0;
     while(rvalue_of(*last_expr_eval).numeric_val)
     {
          if(++iterations >= max_iterations)
               infinite_loop_detected = true;
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
          if(iterations > max_iterations)
               break;
     }
     symtab.pop_back();
}

string Pasithean_Interpreter::get_diagnostics_for_line(int line)
{
     ostringstream sout;
     for(const auto& entry : diagnostics_map[line])
     {
          sout << entry.first << ": ";
          assert(entry.second.size());
          auto it = entry.second.begin();
          auto last = entry.second.end();
          last--;
          while(it!=last)
               sout << *it++ << ",";
          sout << *it << endl;
     }

     return sout.str();
}
