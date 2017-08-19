#pragma once

#include <list>

using std::list;

struct Statement;
struct Statement_List : public AST_Node
{
     list<Statement*> data;
     Comment* adhesive = NULL;
     void visit_with(Visitor& v) { v.visit(*this); }
};
