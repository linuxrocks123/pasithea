#pragma once

#include "Visitor.hpp"
#include "Symbol.hpp"

#include <iostream>
#include <stack>

class Hypnos_Visitor : public Visitor
{
public:
     virtual void visit(Decl_List& decl_list);
     void visit(Include_Decl& include_decl);
     void visit(Func_Decl& func_decl);
     void visit(Import_Decl& import_decl);
     void visit(Class_Decl& class_decl);
     void visit(Class_Decl_Instantiated& class_decl);

     void visit(Function_Call& func_call);
     void visit(Aggregate_Access& ag_access);
     void visit(Array_Access& ary_access);
     void visit(Ternary_Conditional& ternary_expr);
     void visit(Parenthetical& parenthetical);
     void visit(Atomic_Expression& atomic_expr);

     virtual void visit(Statement_List& stmt_list);
     void visit(Label& label);
     void visit(Var_Decl_Statement& vdecl_stmt);
     virtual void visit(Sentinel& cursor);
     void visit(Comment& comment);
     void visit(If_Statement& if_stmt);
     virtual void visit(While_Statement& while_stmt);
     virtual void visit(For_Statement& for_stmt);
     void visit(Switch_Statement& switch_stmt);
     void visit(Anonymous_Block& anon_blk);
     void visit(Interruptor& interrupt);

     int interrupt_level;

     Symbol lee_sym();

     Hypnos_Visitor(std::istream& sin_ = std::cin, std::ostream& sout_ = std::cout)
          : sin(sin_), sout(sout_) { }
     
protected:
     std::istream& sin;
     std::ostream& sout;
     
     /**If literal_status==0, is pointer to lvalue Symbol.
        literal_status may be either literal tokens or type tokens to indicate rvalue.*/
     Atomic_Expression* last_expr_eval;
     std::stack<Function_Call*> call_stack;
     list<Symbol*> ref_params_for_prayer;
     virtual void handle_prayer();

     static Value rvalue_of(const Atomic_Expression& exp);
private:
     static Symbol rvalue_sym(const Atomic_Expression& exp, Type* min_type = NULL);
     static Symbol* lvalue_of(const Atomic_Expression& exp);
     static Atomic_Expression* mk_lvalue(Symbol* lvalue);
};
