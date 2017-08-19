#include "Visitor.hpp"
#include <string>
#include <list>

class Import_Visitor : public Visitor
{
public:
     virtual void visit(Decl_List& decl_list);
     virtual void visit(Include_Decl& include_decl);
     virtual void visit(Func_Decl& func_decl);
     virtual void visit(Import_Decl& import_decl);
     virtual void visit(Class_Decl& class_decl);
     virtual void visit(Class_Decl_Instantiated& class_decl);

     virtual void visit(Function_Call& func_call);
     virtual void visit(Aggregate_Access& ag_access);
     virtual void visit(Array_Access& ary_access);
     virtual void visit(Ternary_Conditional& ternary_expr);
     virtual void visit(Parenthetical& parenthetical);
     virtual void visit(Atomic_Expression& atomic_expr);

     virtual void visit(Statement_List& stmt_list);
     virtual void visit(Label& label);
     virtual void visit(Var_Decl_Statement& vdecl_stmt);
     virtual void visit(Sentinel& cursor);
     virtual void visit(Comment& comment);
     virtual void visit(If_Statement& if_stmt);
     virtual void visit(While_Statement& while_stmt);
     virtual void visit(For_Statement& for_stmt);
     virtual void visit(Switch_Statement& switch_stmt);
     virtual void visit(Anonymous_Block& anon_blk);
     virtual void visit(Interruptor& interrupt);
};
