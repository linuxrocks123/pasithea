#pragma once

#include "AST_Node.hpp"
#include "Statement.hpp"
#include "Symbol.hpp"

struct Expression : public Statement
{
     /**Pointer to structure in typetab
        This is usually NULL during initial parse and filled in in Phase 2.*/
     Type* static_type = NULL;
};

///Note: builtin operators considered functions (including dereference)
///Note: this handles assignments, too: T& operator=(T& l, const T& r)
///Note: and casts: operator T()(const S& s)
struct Function_Call : public Expression
{
     bool new_call = false;
     string called_func_name;
     ///Pointer to structure in Level 0 of symtab, filled in during Phase 2
     Symbol* called_func_sym = NULL;
     list<Expression*> parameters;

     void visit_with(Visitor& v) { v.visit(*this); }
};

struct Aggregate_Access : public Expression
{
     Expression* base_expression;
     int access_form; ///<DOT, ARROW, or SCOPE
     string component_name;

     Aggregate_Access(Expression* be, int af, string cn) : base_expression(be),access_form(af),component_name(cn) {}
     void visit_with(Visitor& v) { v.visit(*this); }
};

struct Array_Access : public Expression
{
     Expression* base_expression;
     Expression* subscript_expression;

     Array_Access(Expression* be, Expression* se) : base_expression(be),subscript_expression(se) {}
     void visit_with(Visitor& v) { v.visit(*this); }
};

struct Ternary_Conditional : public Expression
{
     Expression* conditional;
     Expression* val_if_cond_true;
     Expression* val_if_cond_false;

     Ternary_Conditional(Expression* cond, Expression* t_val, Expression* f_val) : conditional(cond),val_if_cond_true(t_val),val_if_cond_false(f_val) {}
     void visit_with(Visitor& v) { v.visit(*this); }
};

struct Parenthetical : public Expression
{
     Expression* nested;
     Parenthetical(Expression* nested_) : nested(nested_) {};
     void visit_with(Visitor& v) { v.visit(*this); }
};

struct Atomic_Expression : public Expression
{
     int literal_status;
     Value value;

     Atomic_Expression(int literal_status_, Value value_) : literal_status(literal_status_),value(value_) {}
     void visit_with(Visitor& v) { v.visit(*this); }
};
