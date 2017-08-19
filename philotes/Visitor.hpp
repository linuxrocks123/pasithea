#pragma once

class Decl_List;
class Include_Decl;
class Func_Decl;
class Import_Decl;
class Class_Decl;
class Class_Decl_Instantiated;

class Expression_List;
class Function_Call;
class Aggregate_Access;
class Array_Access;
class Ternary_Conditional;
class Parenthetical;
class Atomic_Expression;

class Statement_List;
class Label;
class Var_Decl_Statement;
class Sentinel;
class Comment;
class If_Statement;
class While_Statement;
class For_Statement;
class Switch_Statement;
class Anonymous_Block;
class Interruptor;
     
class Symbol_Modifiers;
class Token;
class Type;

class Visitor
{
public:
     virtual void visit(Decl_List& decl_list) = 0;
     virtual void visit(Include_Decl& include_decl) = 0;
     virtual void visit(Func_Decl& func_decl) = 0;
     virtual void visit(Import_Decl& import_decl) = 0;
     virtual void visit(Class_Decl& class_decl) = 0;
     virtual void visit(Class_Decl_Instantiated& class_decl) = 0;

     virtual void visit(Function_Call& func_call) = 0;
     virtual void visit(Aggregate_Access& ag_access) = 0;
     virtual void visit(Array_Access& ary_access) = 0;
     virtual void visit(Ternary_Conditional& ternary_expr) = 0;
     virtual void visit(Parenthetical& parenthetical) = 0;
     virtual void visit(Atomic_Expression& atomic_expr) = 0;

     virtual void visit(Statement_List& stmt_list) = 0;
     virtual void visit(Label& label) = 0;
     virtual void visit(Var_Decl_Statement& vdecl_stmt) = 0;
     virtual void visit(Sentinel& cursor) = 0;
     virtual void visit(Comment& comment) = 0;
     virtual void visit(If_Statement& if_stmt) = 0;
     virtual void visit(While_Statement& while_stmt) = 0;
     virtual void visit(For_Statement& for_stmt) = 0;
     virtual void visit(Switch_Statement& switch_stmt) = 0;
     virtual void visit(Anonymous_Block& anon_blk) = 0;
     virtual void visit(Interruptor& interrupt) = 0;
};
