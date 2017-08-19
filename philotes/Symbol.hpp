#pragma once

#include "Type.hpp"
#include "Value.hpp"

#include <map>
#include <string>

using std::map;
using std::string;

//Note: function type symbols (i.e. void(int)) are not included in symtab
struct Symbol : public AST_Node
{
     bool hidden = false; ///<true for hidden builtin symbols
     string name;
     Type* type; ///<NULL for type symbols
     /**For type/function symbols _only_, this will be a pointer to
       the AST Node that is the root of the type/function.*/
     Value value;
     map<string,Symbol> components;
     
     void visit_with(Visitor& v) { assert(false); }
};
