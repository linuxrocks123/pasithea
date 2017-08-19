#include "Expression.hpp"

struct Expression_List : public AST_Node
{
     list<Expression*> data;
     void visit_with(Visitor& v) { assert(false); }
};
