#include "Symbol.hpp"
#include "Visitor.hpp"
#include <unordered_set>

using std::unordered_set;

class Semantic_Visitor : public Visitor
{
public:
     void visit(Decl_List& decl_list);
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

     void visit(Statement_List& stmt_list);
     void visit(Label& label);
     void visit(Var_Decl_Statement& vdecl_stmt);
     void visit(Sentinel& cursor);
     void visit(Comment& comment);
     void visit(If_Statement& if_stmt);
     void visit(While_Statement& while_stmt);
     void visit(For_Statement& for_stmt);
     void visit(Switch_Statement& switch_stmt);
     void visit(Anonymous_Block& anon_blk);
     void visit(Interruptor& ret_stmt);
     
     unordered_set<AST_Node*> error_nodes;

private:
     Type* last_expr_eval = NULL; //TODO: will be a Symbol* in real execution
     bool in_loop = false;
     bool in_switch = false;
};
