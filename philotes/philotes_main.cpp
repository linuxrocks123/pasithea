#include "Decl_List.hpp"
#include "symtab.hpp"
#include "Token.hpp"
#include "philotes.h"

#include "Echo_Visitor.hpp"
#include "Import_Visitor.hpp"
#include "Semantic_Visitor.hpp"
#include "Hypnos_Visitor.hpp"

#include <cstdio>
#include <iostream>

using std::cout;
using std::endl;

void yyrestart(FILE* new_file);
extern FILE* yyin;
extern int yydebug;

static Hypnos_Visitor executor;

static void print_last_expr_eval()
{
     Symbol lee = executor.lee_sym();
     if(lee.type==&typetab["boolean"])
          cout << (lee.value.numeric_val ? "true" : "false");
     else if(lee.type==&typetab["byte"])
          cout << static_cast<int8_t>(lee.value.numeric_val);
     else if(lee.type==&typetab["short"])
          cout << static_cast<int16_t>(lee.value.numeric_val);
     else if(lee.type==&typetab["char"])
          cout << static_cast<char>(lee.value.numeric_val);
     else if(lee.type==&typetab["int"])
          cout << static_cast<int32_t>(lee.value.numeric_val);
     else if(lee.type==&typetab["long"])
          cout << lee.value.numeric_val;
     else if(lee.type==&typetab["float"] || lee.type==&typetab["double"])
          cout << lee.value.float_val;
     else if(lee.type==&typetab["String"])
          cout << static_cast<char*>(lee.value.ary.ptr);
     else
          cout << "<Unknown type>";
}


int main()
{
     #if DEBUG
     yydebug = 1;
     #endif
     language_mode = JAVA_MODE;

     //Introductory usage information
     cout << "Welcome to the Philotes Interactive Interpreter." << endl;
     cout << "Code will be executed immediately after it is typed as if run from inside a program's main()." << endl;
     cout << "If you want to type a class or static function definition, type #cursor to temporarily enter non-interactive mode." << endl;
     cout << "Such class or function will be immediately imported into the current working environment after you type Ctrl-D." << endl;

     cout << "\n< P H I L O T E S >\n" << endl;
     
     //Add types to typetab and symtab
     philotes_add_primitives();

#if 0
     Echo_Visitor printer;
     printer.visit(*static_cast<Decl_List*>(root));
#endif

     //Import everything
     remaining_sources.push("#System.java");
     remaining_sources.push("#File.java");
     remaining_sources.push("#Double.java");
     remaining_sources.push("#Random.java");
     remaining_sources.push("#Math.java");
     remaining_sources.push("#Scanner.java");
     yywrap();

     //Visitors
     Semantic_Visitor semantics;
     //executor already declared

     //Process prelude
     inject_herald = PROGRAM_HERALD;
     int result = yyparse();
     assert(!result);
     semantics.visit(*static_cast<Decl_List*>(root));
     assert(!error_msg.size());
     executor.visit(*static_cast<Decl_List*>(root));
     
     //Push dummy method onto stack in preparation for REPL
     symtab.emplace_back();
     this_stack.push(NULL);

     yyrestart(stdin);
     executor.interrupt_level = 0; //not initialized by constructor     
     while(!executor.interrupt_level)
     {
          cout << "Philotes/Java> ";
          inject_herald = STATEMENT_HERALD;
          result = yyparse();
          if(result)
          {
               cout << "Syntax error.  Try again." << endl;
               continue;
          }

          dynamic_cast<Statement*>(root)->visit_with(semantics);

          //Semantic error?
          if(error_msg.size())
          {
               cout << "Semantic errors.  Try again.\nError messages follow:";
               //Print error messages
               int i=0;
               for(const string& e : error_msg)
               {
                    cout << i << ": " << e << endl;
                    i++;
               }
               error_msg.clear();
               continue;
          }

          dynamic_cast<Statement*>(root)->visit_with(executor);
          cout << "   = ";
          print_last_expr_eval();
          cout << endl;

          //WTF is dynamic_cast not working here?
          if(dynamic_cast<Sentinel*>(root))
          {
               cout << "Escape registered.  Go ahead and type your function or class.  Press Ctrl-D to end.\n";
               cout << "< P H I L O T E S >\n";

               auto twilight_vars = symtab.back();
               symtab.pop_back();
               this_stack.pop();
               
               inject_herald = PROGRAM_HERALD;
               result = yyparse();
               if(result)
                    cout << "Syntax error.  Try again." << endl;
               else
               {
                    static_cast<Decl_List*>(root)->visit_with(semantics);
                    if(error_msg.size())
                    {
                         cout << "Semantic errors.  Try again.\nError messages follow:";
                         //Print error messages
                         int i=0;
                         for(const string& e : error_msg)
                         {
                              cout << i << ": " << e << endl;
                              i++;
                         }
                         error_msg.clear();
                    }
                    else
                    {
                         static_cast<Decl_List*>(root)->visit_with(executor);
                         cout << "   = ";
                         print_last_expr_eval();
                         cout << endl;
                    }
               }

               symtab.push_back(twilight_vars);
               this_stack.push(NULL);
          }
     }

     return 0;
}
