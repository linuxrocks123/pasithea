#pragma once

#include "Declaration.hpp"
#include "Symbol_Modifiers.hpp"
#include "Statement_List.hpp"
struct Expression;

#include <string>

using std::string;

struct Statement : public virtual AST_Node {};

struct Label : public Statement, public Declaration
{
     string name;
     bool case_mod;
     Atomic_Expression* expr;
     void visit_with(Visitor& v) { v.visit(*this); }
};

struct Var_Decl_Statement : public Statement, public virtual Declaration
{
     Symbol_Modifiers modifiers;
     string name;
     Expression* init_expr;
     void visit_with(Visitor& v) { v.visit(*this); }
};

struct Class_Decl_Instantiated : public Class_Decl, public Var_Decl_Statement
{
     void visit_with(Visitor& v) { v.visit(*this); }
};

struct Sentinel : public Statement
{
     int cursor_type; ///<CURSOR, SIGNPOST, or PRAYER
     Sentinel(int cursor_type_) : cursor_type(cursor_type_) {}
     void visit_with(Visitor& v) { v.visit(*this); }
};

struct Comment : public Statement, public Declaration
{
     string text;
     void visit_with(Visitor& v) { v.visit(*this); }
};

struct If_Statement : public Statement
{
     Expression* condition;
     Statement_List if_branch;
     Statement_List else_branch;

     Comment* if_comment;
     Comment* else_comment;
     void visit_with(Visitor& v) { v.visit(*this); }
};

struct While_Statement : public Statement
{
     bool do_while = false;
     Expression* condition;
     Statement_List statements;
     Comment* embedded;
     void visit_with(Visitor& v) { v.visit(*this); }
};

struct For_Statement : public Statement
{
     Statement* init_stmt;
     Expression* condition;
     Statement_List statements;
     Expression* post;
     Comment* embedded;
     void visit_with(Visitor& v) { v.visit(*this); }
};

struct Switch_Statement : public Statement
{
     Expression* condition;
     Statement_List statements;
     Comment* embedded;
     void visit_with(Visitor& v) { v.visit(*this); }
};

struct Anonymous_Block : public Statement
{
     Statement_List body;
     void visit_with(Visitor& v) { v.visit(*this); }
};

struct Interruptor : public Statement
{
     int level; //CONTINUE, BREAK, RETURN
     Expression* returned_expression = NULL; //NULL for void methods
     void visit_with(Visitor& v) { v.visit(*this); }
};
