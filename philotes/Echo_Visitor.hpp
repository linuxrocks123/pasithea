#include "AST_Node.hpp"
#include "Visitor.hpp"

class Echo_Visitor : public Visitor
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
     void visit(Interruptor& interruptor);

     Echo_Visitor() : scope_level(0),current_line(0),last_indented_line(-1),disable_all_indentation(false) {}
private:
     void do_whitespace(const AST_Node& node);
     void do_indent();
     void handle_modifiers(const Symbol_Modifiers& modifiers);
     static string unscope(const string& x) { return x.rfind("::")!=string::npos ? x.substr(x.rfind("::")+2) : x; }
     
     int scope_level;
     int current_line;
     int last_indented_line;
     bool disable_all_indentation;
};
