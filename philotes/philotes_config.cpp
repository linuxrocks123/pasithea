#include "fmemopen.h"
#include "philotes_config.hpp"
#include "symtab.hpp"
#include "java_library.hpp"
#include <cstring>
#include <unordered_map>

using std::strlen;
using std::unordered_map;

extern FILE* yyin;

int language_mode;
list<string> error_msg;
int inject_herald;
AST_Node* root;

queue<string> remaining_sources;
const char* interaction_buffer;
extern "C" int yywrap()
{
     if(remaining_sources.empty())
          return 1;
     
     current_line = 1;
     const string next_file = remaining_sources.front();
     remaining_sources.pop();
     
     static unordered_map<string,const char*> BUILTINS = {{"#File.java",file_xxd},{"#Double.java",double_xxd},{"#Math.java",math_xxd},{"#Random.java",random_xxd},{"#Scanner.java",scanner_xxd},{"#System.java",system_xxd},{"#interactive",interaction_buffer}};
     if(next_file[0]=='#')
     {
          assert(BUILTINS.count(next_file));
          yyin = fmemopen(const_cast<char*>(BUILTINS[next_file]),strlen(BUILTINS[next_file]),"r");
          return 0;
     }

     if(yyin = fopen(next_file.c_str(),"r"))
          return 0;

     yyerror(("Loader: No such file "+next_file).c_str());
     return 1;
}
