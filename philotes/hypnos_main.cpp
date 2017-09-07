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
extern FILE* yyin;
extern int yydebug;

int main(int argc, char** argv)
{
     #if DEBUG
     yydebug = 1;
     #endif
     yyin = fopen(argv[1],"r");
     language_mode = JAVA_MODE;

     //Add types to typetab and symtab
     philotes_add_primitives();

     inject_herald = PROGRAM_HERALD;
     int result = yyparse();
     if(result)
          return result;

#if 0
     Echo_Visitor printer;
     printer.visit(*static_cast<Decl_List*>(root));
#endif

     remaining_sources.push("#System.java");
     remaining_sources.push("#Double.java");
     remaining_sources.push("#Random.java");
     remaining_sources.push("#Math.java");
     
     Import_Visitor importer;
     importer.visit(*static_cast<Decl_List*>(root));
     remaining_sources.push(argv[1]);

     auto backup_sources = remaining_sources;
     auto backup_errors = error_msg;
     reset();
     remaining_sources = backup_sources;
     error_msg = backup_errors;
     
     philotes_add_primitives();
     inject_herald = PROGRAM_HERALD;
     result = yyparse();
     if(result)
          return result;

     Semantic_Visitor semantics;
     semantics.visit(*static_cast<Decl_List*>(root));

     //Print error messages
     int i=0;
     for(const string& e : error_msg)
     {
          cout << i << ": " << e << endl;
          i++;
     }

     if(error_msg.size())
          return 1;

     Hypnos_Visitor executor;
     executor.visit(*static_cast<Decl_List*>(root));

     return 0;
}
