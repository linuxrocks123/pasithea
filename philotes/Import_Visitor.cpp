#include "Import_Visitor.hpp"
#include "Declaration.hpp"
#include "symtab.hpp"

#include <map>
#include <set>
#include <string>

using std::map;
using std::set;
using std::string;

void Import_Visitor::visit(Decl_List& decl_list)
{
     for(Declaration* decl : decl_list.data)
          decl->visit_with(*this);
}

void Import_Visitor::visit(Include_Decl& include_decl)
{
}

void Import_Visitor::visit(Func_Decl& func_decl)
{
}

void Import_Visitor::visit(Import_Decl& import_decl)
{
     static map<string,set<string> > valid_imports = {{"java.util.Scanner",{"#Scanner.java"}},{"java.io.*",{"#File.java"}},{"java.util.*",{"#Random.java","#Scanner.java"}},{"java.util.Random",{"#Random.java"}}};
     if(valid_imports.count(import_decl.imported_class))
          for(const auto& x : valid_imports[import_decl.imported_class])
               remaining_sources.push(x);
     else
          report_error("Invalid import: "+import_decl.imported_class,&import_decl);
}

void Import_Visitor::visit(Class_Decl& class_decl)
{
}

void Import_Visitor::visit(Class_Decl_Instantiated& class_decl)
{
}

void Import_Visitor::visit(Function_Call& func_call)
{
}

void Import_Visitor::visit(Aggregate_Access& ag_access)
{
}

void Import_Visitor::visit(Array_Access& ary_access)
{
}

void Import_Visitor::visit(Ternary_Conditional& ternary_expr)
{
}

void Import_Visitor::visit(Parenthetical& parenthetical)
{
}

void Import_Visitor::visit(Atomic_Expression& atomic_expr)
{
}

void Import_Visitor::visit(Statement_List& stmt_list)
{
}

void Import_Visitor::visit(Label& label)
{
}

void Import_Visitor::visit(Var_Decl_Statement& vdecl_stmt)
{
}

void Import_Visitor::visit(Sentinel& cursor)
{
}

void Import_Visitor::visit(Comment& comment)
{
}

void Import_Visitor::visit(If_Statement& if_stmt)
{
}

void Import_Visitor::visit(While_Statement& while_stmt)
{
}

void Import_Visitor::visit(For_Statement& for_stmt)
{
}

void Import_Visitor::visit(Switch_Statement& switch_stmt)
{
}

void Import_Visitor::visit(Anonymous_Block& anon_blk)
{
}

void Import_Visitor::visit(Interruptor& interrupt)
{
}
