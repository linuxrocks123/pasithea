#pragma once

#include <map>
#include <list>
#include <stack>
#include <string>

#include "Symbol.hpp"

using std::map;
using std::list;
using std::stack;
using std::string;

/**The second and subsequent levels of symtab are for DYNAMIC scopes.
  All namespace level types/functions/variables are in the first level
  of symtab, stored as "namespace_name::symbol_name".*/
extern list<map<string,Symbol> > symtab;

/**Repository for all types, even builtins
  Note: A type is incomplete iff it appears in typetab but not symtab*/
extern map<string,Type> typetab;

/**Stack has one entry per level of dynamic scope.
  If there is no this pointer for a particular level, NULL is used.*/
extern stack<Symbol*> this_stack;

///This is for convenience in searching through static scopes.
string get_scope_qualifier_prefix(); ///<includes trailing :: or .

/**Get the globally scoped string for a defined but possibly unscoped or
   partially scoped referenced symbol in the current scope.  If none exists,
   return a globally scoped undefined symbol with the same name.
   This function searches both symtab and typetab for a symbol definition.*/
string qualify_unscoped_reference(const string& symbol, bool funcs_valid = true);
extern list<string> static_scopes;

///Get correct function symbol, given name and types of arguments
///The return type is NOT the first parameter since return types do not
///participate in overload resolution.
Symbol* resolve_function_symbol(string name, list<Type*>& parameters);

///Resolve an atomic symbol
Symbol* resolve_atomic_symbol(string name);

///Mark a visibility severance, i.e. a function call, in the symbol table
void sever_visibility();
extern stack<int> severances;

///Current class visibility for C++
extern int current_visibility_level;

///Current line number and file
extern int current_line;
extern string current_file;

void reset();

///Add primitive types to symtab/typetab
void philotes_add_primitives();

///Handle interactive user inspection commands
void inspector_gadget(string command, string* buffer = NULL);

///Yacc-defined error reporting
int yyerror(const char* msg);
///Higher level error reporting
void report_error(string message, const AST_Node* node);
