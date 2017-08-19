#pragma once

#include "AST_Node.hpp"

struct Type : public AST_Node
{
     ///For primitive types, the name is present, and
     ///everything else is false or empty
     ///For incomplete types, the name is empty, and everything else is undefined
     string name;
     
     int aggregate_type; ///<CLASS, STRUCT, UNION, or 0
     bool is_array; ///<self-explanatory
     bool is_union; ///<self-explanatory

     bool is_reference;
     bool is_pointer;
     list<Type*> base_or_parent_types;
     
     list<Var_Decl_Statement*> contents; ///<statements to initialize an aggregate
     list<Type*> parameters; ///<nonempty iff function; return type first parameter
     /*Note that this data structure does not support recording C++ constness
       of parameters: doing so would require adding an is_const member to Type.
       It is instead expected the interpreter report will runtime errors.*/

     void visit_with(Visitor& v) { assert(false); }
};
