#pragma once

#include "AST_Node.hpp"
#include "Decl_List.hpp"
#include "Statement_List.hpp"
#include "Symbol_Modifiers.hpp"

struct Statement;

struct Declaration : public virtual AST_Node {};

//Note: Label is a Declaration but defined in Statement.hpp
//Note: Var_Decl_Statement is a Declaration but defined in Statement.hpp
//Note: Comment is a Declaration but defined in Statement.hpp

struct Include_Decl : public Declaration
{
     string included_file;
     void visit_with(Visitor& v) { v.visit(*this); }
};

struct Import_Decl : public Declaration
{
     string imported_class;
     void visit_with(Visitor& v) { v.visit(*this); }
};

struct Class_Decl : public virtual Declaration
{
     Type* type;
     Decl_List body;
     void visit_with(Visitor& v) { v.visit(*this); }     
};

struct Func_Decl : public Declaration
{
     Symbol_Modifiers modifiers;
     bool return_value_is_const;
     string name;
     Statement_List body;
     void visit_with(Visitor& v) { v.visit(*this); }
};
