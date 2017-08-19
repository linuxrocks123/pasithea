#pragma once

#include "AST_Node.hpp"

struct Declaration;
struct Decl_List : public AST_Node
{
     list<Declaration*> data;
     void visit_with(Visitor& v) { v.visit(*this); }
};
