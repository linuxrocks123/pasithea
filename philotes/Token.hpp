#pragma once

#include "AST_Node.hpp"
#include "Expression.hpp"
#include "Value.hpp"

struct Token : public AST_Node
{
     Value value;
     void visit_with(Visitor& v) { assert(false); }
};
