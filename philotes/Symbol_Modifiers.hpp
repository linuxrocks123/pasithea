#pragma once

#include "AST_Node.hpp"
#include <cassert>

struct Var_Decl_Statement;
struct Symbol_Modifiers : public AST_Node
{
     bool const_m;
     bool static_m;
     bool final_m;
     int visibility; ///<value one of PUBLIC/PROTECTED/PRIVATE tokens
     Type* static_type; ///<pointer to structure in typetab

     list<Var_Decl_Statement*> parameters; ///<unlike with types, first parameter is not return type
     void visit_with(Visitor& v) { assert(false); }
};
