/*iijc.y -- Yacc grammar file for (simplified) C++/Java interpreter*/
%glr-parser
%define parse.error verbose

%{

#include "Decl_List.hpp"
#include "Expression.hpp"
#include "Expression_List.hpp"
#include "Statement.hpp"
#include "Statement_List.hpp"
#include "Type.hpp"

#include "Token.hpp"
#include "symtab.hpp"
#include <algorithm>
#include <utility>

#define YYMAXDEPTH 1000000000

using std::find_if;
using std::make_pair;
using std::to_string;

int yylex();

Class_Decl* handle_aggregate_declaration(Token* keyword, Token* class_ident_ast, Decl_List* body, Token* instantiation);
Func_Decl* handle_function_declaration(AST_Node* pre_modifiers_, AST_Node* post_modifiers_, AST_Node* name_, AST_Node* parameters_, AST_Node* body_);
Function_Call* handle_binary_operator_expression(Expression* left, string op_name, Expression* right);
Atomic_Expression* handle_literal(int tok_id, AST_Node* literal);
void array_suffix_hack(Var_Decl_Statement* vds);

void push_scope(AST_Node* keyword);
string get_unique_anonymous_placeholder_str();

%}

%union
 {
      AST_Node* ast_node;
      Decl_List* decl_list;
      Include_Decl* include_decl;
      Import_Decl* import_decl;
      Class_Decl* class_decl;
      Func_Decl* func_decl;
      Statement_List* statement_list;
      Var_Decl_Statement* var_decl_statement;
      If_Statement* if_statement;
      Switch_Statement* switch_statement;
      While_Statement* while_statement;
      For_Statement* for_statement;
      Symbol_Modifiers* symbol_modifiers;
      Expression_List* expression_list;
      Expression* expression;
      Atomic_Expression* atomic_expression;
      Type* type;
      Label* label;
      Comment* comment;
      Token* token;
}

%left <token> ','
%right <token> '=' PLUS_ASSIGN MINUS_ASSIGN TIMES_ASSIGN DIV_ASSIGN MOD_ASSIGN LEFT_SHIFT_ASSIGN RIGHT_SHIFT_ASSIGN AND_ASSIGN OR_ASSIGN XOR_ASSIGN '?' ':'
%left <token> OR
%left <token> AND
%left <token> '|'
%left <token> '^'
%left <token> '&'
%left <token> EQUALS NOT_EQUALS
%left <token> '<' LT_EQUALS '>' GT_EQUALS
%left <token> LEFT_SHIFT RIGHT_SHIFT UNSIGNED_RIGHT_SHIFT
%left <token> '+' '-'
%left <token> '*' '/' '%'
%left <token> DOT_STAR ARROW_STAR
%right <token> PP_INC MM_DEC UPLUS_RULE UMINUS_RULE '!' '~' CAST_RULE DEREF_RULE ADDROF_RULE SIZEOF NEW DELETE
%left <token> PP_INC_SUFFIX_RULE MM_DEC_SUFFIX_RULE ')' ']' DOT ARROW
%precedence <token> ELSE
%left <token> SCOPE

%precedence <token> STR_LITERAL BOOLEAN_LITERAL CHAR_LITERAL INT_LITERAL FLOAT_LITERAL DOUBLE_LITERAL IDENTIFIER

%token <token> IMPORT
%token <token> INCLUDE
%token <token> CURSOR
%token <token> SIGNPOST
%token <token> PRAYER

%token <token> IF
%token <token> SWITCH
%token <token> WHILE
%token <token> FOR
%token <token> DO
%token <token> RETURN
%token <token> BREAK
%token <token> CONTINUE

%token <token> UNSIGNED
%token <token> SIGNED
%token <token> CONST
%token <token> FINAL
%token <token> STATIC
%token <token> INLINE
%token <token> PUBLIC
%token <token> PROTECTED
%token <token> PRIVATE
%token <token> CASE

%token <token> BOOLEAN
%token <token> BYTE
%token <token> CHAR
%token <token> SHORT
%token <token> INT
%token <token> LONG
%token <token> FLOAT
%token <token> DOUBLE
%token <token> AUTO
%token <token> VOID

%token <token> CLASS
%token <token> STRUCT
%token <token> UNION

%token <token> COMMENT

%type <ast_node> Global_Scope_Construct Class_Scope_Declaration Statement Substantive_Statement Var_Decl_Statement Decl_Or_Expression
%type <decl_list> Global_Scope_List Class_Scope_List Parameter_List Var_Dec_List
%type <include_decl> Include_Decl
%type <import_decl> Import_Decl
%type <class_decl> Class_Decl
%type <func_decl> Func_Decl
%type <statement_list> Statement_List Anonymous_Block Block_Or_Statement
%type <var_decl_statement> Single_Var_Decl Var_Dec_Phrase
%type <if_statement> If_Statement
%type <switch_statement> Switch_Statement
%type <while_statement> While_Statement
%type <for_statement> For_Statement
%type <symbol_modifiers> Type_Phrase Base_Type_Phrase Modifier_List Modifier
%type <expression_list> Argument_List
%type <expression> Expression Optional_Initialization
%type <atomic_expression> Literal
%type <type> Base_Type Primitive_Type_Phrase User_Defined_Type
%type <label> Label Label_Internal
%type <comment> Comment
%type <token> Aggregate Num_Array_Specifiers Optional_Reference Optional_Pointers Counting_Stars Const_Specifier Primitive_Type_Keyword Multipart_Identifier ';'

%token <token> PROGRAM_HERALD STATEMENT_HERALD
%start Herald

%%

//IMPORTANT: DURING PARSING, WE _ONLY_ PUT TYPES IN THE SYMBOL TABLE.
//GLOBAL VARIABLES COME DURING INTERPRETATION.
//The symbol table should already have all primitive types included (including void).
//It will not have things like int& or void*: there are infinitely many.

Herald: PROGRAM_HERALD Global_Scope_List { root = $2; };
        |       STATEMENT_HERALD Substantive_Statement { root = $2; };

Global_Scope_List: Global_Scope_List Global_Scope_Construct
                {
                    Decl_List* ptr = $1;
                    Decl_List* append_list;
                    if(append_list = dynamic_cast<Decl_List*>($2))
                         ptr->data.splice(ptr->data.end(),append_list->data);
                    else
                         ptr->data.push_back(dynamic_cast<Declaration*>($2));
                    $$ = $1;
                }
        |       Global_Scope_Construct
                {
                    if(dynamic_cast<Declaration*>($1))
                    {
                        Decl_List* ptr = new Decl_List;
                        ptr->copy_from(*$1);
                        ptr->data.push_back(dynamic_cast<Declaration*>($1));
                        $$ = ptr;
                    }
                    else
                    {
                        assert(dynamic_cast<Decl_List*>($1));
                        $$ = dynamic_cast<Decl_List*>($1);
                    }
                };

Global_Scope_Construct: Include_Decl { $$ = $1; }
        |       Import_Decl { $$ = $1; }
        |       Class_Decl { $$ = $1; }
        |       Func_Decl { $$ = $1; }
        |       Var_Decl_Statement //global variable
                {
                    if(language_mode==JAVA_MODE)
                        report_error("Global variables illegal in Java",$1);
                    $$ = $1;
                }
        |       Comment { $$ = $1; };
        /*TODO: C++ namespaces*/

Include_Decl: INCLUDE
                {
                    Include_Decl* d = new Include_Decl;
                    d->copy_from(*$1);
                    d->included_file = $1->value.as_string();
                    $$ = d;
                };

Import_Decl: IMPORT
                {
                    Import_Decl* d = new Import_Decl;
                    d->copy_from(*$1);
                    d->imported_class = $1->value.as_string();
                    $$ = d;
                };

//TODO: handle C++ forward declarations
Class_Decl: Aggregate IDENTIFIER '{' { push_scope($2); } Class_Scope_List { static_scopes.pop_back(); } '}' ';'
                {
                    if(language_mode!=CPP_MODE)
                        report_error("Erroneous C++-style semicolon", $8);
                    $$ = handle_aggregate_declaration($1, $2, $5, NULL);
                }
        |       Aggregate IDENTIFIER '{' { push_scope($2); } Class_Scope_List { static_scopes.pop_back(); } '}' IDENTIFIER ';'
                {
                    if(language_mode!=CPP_MODE)
                        report_error("Erroneous C++-style instantiation", $8);
                    $$ = handle_aggregate_declaration($1, $2, $5, $8);
                }
        |       Aggregate '{' {
            string placeholder = get_unique_anonymous_placeholder_str();
            Token* tok = new Token;
            tok->value.ary.ptr = strdup(placeholder.c_str());
            tok->value.ary.length = placeholder.length()+1;
            push_scope(tok);
                $<token>$ = tok; } Class_Scope_List { static_scopes.pop_back(); } '}' IDENTIFIER ';'
                {
                    if(language_mode!=CPP_MODE)
                        report_error("C/C++-style anonymous aggregate not valid in this language",$1);
                    $$ = handle_aggregate_declaration($1, $<token>3, $4, $7);
                }
        |       Aggregate IDENTIFIER '{' { push_scope($2); } Class_Scope_List { static_scopes.pop_back(); } '}'
                {
                    if(language_mode!=JAVA_MODE)
                        report_error("Erroneous Java-style lack of semicolon", $5);
                    $$ = handle_aggregate_declaration($1, $2, $5, NULL);
                };
//TODO: handle list of instantiations instead of just one, support inheritance

Aggregate: CLASS { $$ = $1; }
        |       STRUCT { $$ = $1; }
        |       UNION { $$ = $1; }
        |       PUBLIC CLASS
        {
             if(language_mode!=JAVA_MODE)
                  report_error("\"public class\" not legal in C++.",$1);
             $$ = $2;
        };

Class_Scope_List: Class_Scope_List Class_Scope_Declaration
                {
                    Decl_List* append_list;
                    if(append_list = dynamic_cast<Decl_List*>($2))
                         $1->data.splice($1->data.end(),append_list->data);
                    else
                         $1->data.push_back(dynamic_cast<Declaration*>($2));
                    $$ = $1;
                }
        |       Class_Scope_Declaration
                {
                    Decl_List* decl_list = new Decl_List;
                    decl_list->copy_from(*$1);
                    decl_list->data.push_back(dynamic_cast<Declaration*>($1));
                    $$ = decl_list;
                };

Class_Scope_Declaration: Class_Decl { $$ = $1; }
        |       Func_Decl { $$ = $1; }
        |       Var_Decl_Statement { $$ = $1; }
        |       Label { $$ = $1; }
        |       Comment { $$ = $1; };

Statement_List: Statement_List Statement
                {
                    if(dynamic_cast<Decl_List*>($2))
                         for(Declaration* decl : static_cast<Decl_List*>($2)->data)
                              $1->data.push_back(dynamic_cast<Var_Decl_Statement*>(decl));
                    else if(dynamic_cast<Statement_List*>($2))
                         $1->data.splice($1->data.end(),static_cast<Statement_List*>($2)->data);
                    else
                         $1->data.push_back(dynamic_cast<Statement*>($2));
                    $$ = $1;
                }
        |       Statement
                {
                    if(dynamic_cast<Statement*>($1))
                    {
                        Statement_List* ptr = new Statement_List;
                        ptr->copy_from(*$1);
                        ptr->data.push_back(dynamic_cast<Statement*>($1));
                        $$ = ptr;
                    }
                    else
                    {
                        assert(dynamic_cast<Decl_List*>($1));
                        Statement_List* ptr = new Statement_List;
                        for(Declaration* decl : static_cast<Decl_List*>($1)->data)
                            ptr->data.push_back(dynamic_cast<Var_Decl_Statement*>(decl));
                        $$ = ptr;
                    }
                };

//Note: class declarations inside functions not supported
Func_Decl: Type_Phrase IDENTIFIER '(' Parameter_List ')' '{' Statement_List '}'
                {
                    $$ = handle_function_declaration($1,NULL,$2,$4,$7);
                }
        |       Type_Phrase IDENTIFIER '(' Parameter_List ')' ';'
                {
                    $$ = handle_function_declaration($1,NULL,$2,$4,NULL);
                } //TODO: constructors, C++ post modifiers (i.e. void f() const)
        |       Modifier IDENTIFIER '(' Parameter_List ')' '{' Statement_List '}'
                {
                     Symbol_Modifiers* syms = assert_cast<Symbol_Modifiers*>($1);
                     if(syms->const_m || syms->static_m || syms->final_m)
                          report_error("Illegal modifier in constructor header",$1);
                     if(assert_cast<Token*>($2)->value.as_string()!=static_scopes.back())
                          report_error("Incorrectly named constructor declared",$2);
                     $$ = handle_function_declaration($1,NULL,$2,$4,$7);
                }
        |       IDENTIFIER '(' Parameter_List ')' '{' Statement_List '}'
                {
                     $$ = handle_function_declaration(new Symbol_Modifiers{},NULL,$1,$3,$6);
                };

Parameter_List: Parameter_List ',' Single_Var_Decl
                {
                    $1->data.push_back($3);
                    $$ = $1;
                }
        |       Single_Var_Decl
                {
                    Decl_List* decl_list = new Decl_List;
                    decl_list->copy_from(*$1);
                    decl_list->data.push_back(dynamic_cast<Declaration*>($1));
                    $$ = decl_list;
                }
        |       /*empty*/ { $$ = new Decl_List; };

//Misleading as fuck C++ statements like "int* x, y;" intentionally not supported
Var_Decl_Statement: Base_Type_Phrase Var_Dec_List ';' //split into multiple AST nodes, one per variable declared
                {
                    for(Declaration* i_ : assert_cast<Decl_List*>($2)->data)
                    {
                        Var_Decl_Statement* i = dynamic_cast<Var_Decl_Statement*>(i_);
                        i->modifiers = *assert_cast<Symbol_Modifiers*>($1);
                        array_suffix_hack(i);
                    }
                    $$ = $2;
                }
        |       Single_Var_Decl ';' { $$ = $1; };

Single_Var_Decl: Type_Phrase Var_Dec_Phrase
                {
                    $2->modifiers = *$1;
                    array_suffix_hack($2);
                    $$ = $2;
                };

Var_Dec_List: Var_Dec_List ',' Var_Dec_Phrase
                {
                    $1->data.push_back($3);
                    $$ = $1;
                }
        |       Var_Dec_Phrase ',' Var_Dec_Phrase
                {
                    Decl_List* decl_list = new Decl_List;
                    decl_list->copy_from(*$1);
                    decl_list->data.push_back($1);
                    decl_list->data.push_back($3);
                    $$ = decl_list;
                };

Var_Dec_Phrase: IDENTIFIER Optional_Initialization
                {
                    Var_Decl_Statement* vardec_phrase = new Var_Decl_Statement;
                    vardec_phrase->copy_from(*$1);
                    vardec_phrase->name = $1->value.as_string();
                    vardec_phrase->init_expr = $2;
                    $$ = vardec_phrase;
                }
        |       IDENTIFIER Num_Array_Specifiers Optional_Initialization
                {
                    Var_Decl_Statement* vardec_phrase = new Var_Decl_Statement;
                    vardec_phrase->copy_from(*$1);
                    vardec_phrase->name = $1->value.as_string();

                    for(int i=0; i<$2->value.numeric_val; i++)
                        vardec_phrase->name+="[]";

                    vardec_phrase->init_expr = $3;
                    $$ = vardec_phrase;
                };

Optional_Reference: '&'
                {
                    if(language_mode==JAVA_MODE)
                        report_error("C++ references not valid in Java",$1);
                    $1->value.numeric_val = 1;
                    $$ = $1;
                }
        |       /*empty*/ { $$ = NULL; };

Optional_Pointers: Counting_Stars { $$ = $1; }
        |       /*empty*/ { $$ = NULL; };

Counting_Stars: Counting_Stars '*'
                {
                    $1->value.numeric_val++;
                    $$ = $1;
                }
        |       '*'
                {
                    if(language_mode==JAVA_MODE)
                        report_error("Pointers not valid in Java",$1);
                    $1->value.numeric_val = 1;
                    $$ = $1;
                };

Num_Array_Specifiers: '[' ']' { Token* t = new Token; t->value.numeric_val = 1; $$ = t; }
        |       Num_Array_Specifiers '[' ']'
                {
                    Token* t = $1;
                    t->value.numeric_val++;
                    $$ = t;
                };

Optional_Initialization:  '=' Expression { $$ = $2; }
        |       /*empty*/ { $$ = NULL; };

Type_Phrase: Base_Type_Phrase Optional_Reference Optional_Pointers
                {
                    if($2 || $3)
                    {
                        string type_name = $1->static_type->name;
                        if($2)
                            type_name+="&";
                        if($3)
                            for(int i=0; i<$3->value.numeric_val; i++)
                                type_name+="*";

                        //Deal with creating new types
                        int suffix = type_name.length();
                        while(!typetab.count(type_name.substr(0,suffix)))
                            suffix--;
                        for(; suffix<type_name.length(); suffix++)
                        {
                             assert(type_name[suffix]=='*' || type_name[suffix]=='&');
                             Type ptr_type{};
                             ptr_type.name = type_name.substr(0,suffix+1);
                             ptr_type.is_reference = type_name[suffix]=='&';
                             ptr_type.is_pointer = type_name[suffix]=='*';
                             ptr_type.base_or_parent_types.push_back(&typetab[type_name.substr(0,suffix)]);
                             typetab[type_name.substr(0,suffix+1)] = ptr_type;
                        }
                        $1->static_type = &(typetab[type_name]);
                    }
                    $$ = $1;
                }
        |       Base_Type_Phrase Num_Array_Specifiers
                {
                    string type_name = $1->static_type->name;
                    for(int i=0; i<$2->value.numeric_val; i++)
                        type_name+="[]";
                    int suffix = type_name.length();
                    while(!typetab.count(type_name.substr(0,suffix)) && type_name[suffix-2]=='[')
                        suffix-=2;
                    for(; suffix<type_name.length(); suffix+=2)
                    {
                        assert(type_name[suffix]=='[' && type_name[suffix+1]==']');
                        Type ary_type{};
                        ary_type.name = type_name.substr(0,suffix+2);
                        ary_type.is_array = true;
                        ary_type.base_or_parent_types.push_back(&typetab[type_name.substr(0,suffix)]);
                        typetab[type_name.substr(0,suffix+2)] = ary_type;
                    }
                    $1->static_type = &(typetab[type_name]);
                    $$ = $1;
                };

Base_Type_Phrase: Modifier_List Base_Type
                {
                    $1->static_type = $2;
                    $$ = $1;
                }
        |       Base_Type
                {
                     Symbol_Modifiers* modifiers = new Symbol_Modifiers{};
                     modifiers->static_type = $1;
                     $$ = modifiers;
                };

Modifier_List: Modifier_List Modifier
                {
                    Symbol_Modifiers* old_modifiers = $1;
                    Symbol_Modifiers* new_modifier = $2;
                    
                    old_modifiers->const_m |= new_modifier->const_m;
                    old_modifiers->static_m |= new_modifier->static_m;
                    old_modifiers->final_m |= new_modifier->final_m;
                    if(!old_modifiers->visibility)
                        old_modifiers->visibility = new_modifier->visibility;
                    $$ = $1;
                }
        |       Modifier { $$ = $1; };

Modifier: Const_Specifier
                {
                    Symbol_Modifiers* modifiers = new Symbol_Modifiers();
                    modifiers->copy_from(*$1);
                    modifiers->const_m = true;
                    $$ = modifiers;
                }
        |       STATIC
                {
                    Symbol_Modifiers* modifiers = new Symbol_Modifiers();
                    modifiers->copy_from(*$1);
                    modifiers->static_m = true;
                    $$ = modifiers;
                }
        |       INLINE
                {
                    Symbol_Modifiers* modifiers = new Symbol_Modifiers();
                    modifiers->copy_from(*$1);
                    $$ = modifiers;
                }
        |       PUBLIC
                {
                    Symbol_Modifiers* modifiers = new Symbol_Modifiers();
                    modifiers->copy_from(*$1);
                    modifiers->visibility = PUBLIC;
                    $$ = modifiers;
                }
        |       PROTECTED
                {
                    Symbol_Modifiers* modifiers = new Symbol_Modifiers();
                    modifiers->copy_from(*$1);
                    modifiers->visibility = PROTECTED;
                    $$ = modifiers;
                }
        |       PRIVATE
                {
                    Symbol_Modifiers* modifiers = new Symbol_Modifiers();
                    modifiers->copy_from(*$1);
                    modifiers->visibility = PRIVATE;
                    $$ = modifiers;
                };

Const_Specifier: CONST
                {
                    if(language_mode==JAVA_MODE)
                        report_error("There is no const in Java",$1);
                    $$ = $1;
                }
        |       FINAL
                {
                    if(language_mode==CPP_MODE)
                        report_error("There is no final in C++",$1);
                    $$ = $1;
                };

Base_Type: Primitive_Type_Phrase { $$ = $1; }
        |       User_Defined_Type { $$ = $1; };

Primitive_Type_Phrase: UNSIGNED Primitive_Type_Keyword
                {
                    $$ = &typetab[string("unsigned ")+$2->value.as_string()];
                }
        |       SIGNED Primitive_Type_Keyword
                {
                    $$ = &typetab[$2->value.as_string()];
                }
        |       Primitive_Type_Keyword
                {
                    $$ = &typetab[$1->value.as_string()];
                };

Primitive_Type_Keyword: BYTE
                {
                    if(language_mode==CPP_MODE)
                        report_error("byte is not a type in C++",$1);
                    $$ = $1;
                }
        |       CHAR { $$ = $1; } | SHORT { $$ = $1; }
        |       INT { $$ = $1; } | LONG { $$ = $1; }
        |       FLOAT { $$ = $1; } | DOUBLE { $$ = $1; }
                /*TODO: C++11 auto*/ | VOID { $$ = $1; };

User_Defined_Type: IDENTIFIER
                {
                    string type_name = $1->value.as_string();
                    string scoped_name = qualify_unscoped_reference(type_name,false);
                    if(!typetab.count(scoped_name)) //insert as incomplete type
                        typetab[scoped_name] = Type{};
                    $$ = &(typetab[scoped_name]);
                };

Statement: Label { $$ = $1; }
        |       Var_Decl_Statement { $$ = $1; }
        |       CURSOR
                {
                    Sentinel* to_return = new Sentinel(CURSOR);
                    to_return->copy_from(*$1);
                    $$ = to_return;
                }
        |       SIGNPOST
                {
                    Sentinel* to_return = new Sentinel(SIGNPOST);
                    to_return->copy_from(*$1);
                    $$ = to_return;
                }
        |       PRAYER
                {
                    Sentinel* to_return = new Sentinel(PRAYER);
                    to_return->copy_from(*$1);
                    $$ = to_return;
                }
        |       Comment { $$ = $1; }
        |       If_Statement { $$ = $1; }
        |       While_Statement { $$ = $1; }
        |       For_Statement { $$ = $1; }
        |       Switch_Statement { $$ = $1; }
        |       Anonymous_Block { $$ = $1; }
        |       Expression ';' { $$ = $1; }
        |       RETURN Expression ';'
                {
                     Interruptor* to_return = new Interruptor;
                     to_return->copy_from(*$1);
                     to_return->level = RETURN;
                     to_return->returned_expression = $2;
                     $$ = to_return;
                }
        |       RETURN ';'
                {
                     Interruptor* to_return = new Interruptor;
                     to_return->copy_from(*$1);
                     to_return->level = RETURN;
                     $$ = to_return;
                }
        |       BREAK ';'
                {
                     Interruptor* to_return = new Interruptor;
                     to_return->copy_from(*$1);
                     to_return->level = BREAK;
                     $$ = to_return;
                }
        |       CONTINUE ';'
                {
                     Interruptor* to_return = new Interruptor;
                     to_return->copy_from(*$1);
                     to_return->level = CONTINUE;
                     $$ = to_return;
                };

Substantive_Statement: Var_Decl_Statement { $$ = $1; }
        |       CURSOR
                {
                    Sentinel* to_return = new Sentinel(CURSOR);
                    to_return->copy_from(*$1);
                    $$ = to_return;
                }
        |       SIGNPOST
                {
                    Sentinel* to_return = new Sentinel(SIGNPOST);
                    to_return->copy_from(*$1);
                    $$ = to_return;
                }
        |       PRAYER
                {
                    Sentinel* to_return = new Sentinel(PRAYER);
                    to_return->copy_from(*$1);
                    $$ = to_return;
                }
        |       If_Statement { $$ = $1; }
        |       While_Statement { $$ = $1; }
        |       For_Statement { $$ = $1; }
        |       Switch_Statement { $$ = $1; }
        |       Anonymous_Block { $$ = $1; }
        |       Expression ';' { $$ = $1; }
        |       RETURN Expression ';'
                {
                     Interruptor* to_return = new Interruptor;
                     to_return->copy_from(*$1);
                     to_return->level = RETURN;
                     to_return->returned_expression = $2;
                     $$ = to_return;
                }
        |       RETURN ';'
                {
                     Interruptor* to_return = new Interruptor;
                     to_return->copy_from(*$1);
                     to_return->level = RETURN;
                     $$ = to_return;
                }
        |       BREAK ';'
                {
                     Interruptor* to_return = new Interruptor;
                     to_return->copy_from(*$1);
                     to_return->level = BREAK;
                     $$ = to_return;
                }
        |       CONTINUE ';'
                {
                     Interruptor* to_return = new Interruptor;
                     to_return->copy_from(*$1);
                     to_return->level = CONTINUE;
                     $$ = to_return;
                };

Label: Label_Internal ':' { $$ = $1; };

Label_Internal: CASE Literal
                {
                    Label* to_return = new Label;
                    to_return->copy_from(*$1);
                    to_return->case_mod = true;
                    to_return->expr = $2;
                    $$ = to_return;
                }
        |       IDENTIFIER
                {
                    Label* to_return = new Label;
                    to_return->copy_from(*$1);
                    to_return->name = $1->value.as_string();
                    $$ = to_return;
                };

Comment: COMMENT
                {
                    Comment* comment = new Comment;
                    comment->copy_from(*$1);
                    comment->text = assert_cast<Token*>($1)->value.as_string();
                    $$ = comment;
                };

Anonymous_Block: '{' Statement_List '}' { $$ = $2; };

Block_Or_Statement: Substantive_Statement
                {
                     if(dynamic_cast<Statement*>($1))
                     {
                          Statement_List* blockified = new Statement_List;
                          blockified->data.push_back(dynamic_cast<Statement*>($1));
                          $$ = blockified;
                     }
                     else
                     {
                          assert(dynamic_cast<Statement_List*>($1));
                          $$ = dynamic_cast<Statement_List*>($1);
                     }
                }
        |       Comment Substantive_Statement
                {
                     Statement_List* blockified;
                     if(dynamic_cast<Statement*>($2))
                     {
                          blockified = new Statement_List;
                          blockified->data.push_back(dynamic_cast<Statement*>($2));
                     }
                     else
                          blockified = dynamic_cast<Statement_List*>($2);
                     blockified->adhesive = $1;
                     $$ = blockified;
                };

If_Statement: IF '(' Decl_Or_Expression ')' Block_Or_Statement
                {
                    If_Statement* to_return = new If_Statement;
                    to_return->copy_from(*$1);
                    //TODO: decl in if not yet supported
                    to_return->condition = dynamic_cast<Expression*>($3);
                    to_return->if_branch.data = std::move($5->data);
                    to_return->if_comment = $5->adhesive;
                    to_return->else_comment = NULL;
                    $$ = to_return;
                }
        |       IF '(' Expression ')' Block_Or_Statement ELSE Block_Or_Statement
                {
                    If_Statement* to_return = new If_Statement;
                    to_return->copy_from(*$1);
                    //TODO: decl in if not yet supported
                    to_return->condition = dynamic_cast<Expression*>($3);
                    to_return->if_branch.data = std::move($5->data);
                    to_return->else_branch.data = std::move($7->data);
                    to_return->if_comment = $5->adhesive;
                    to_return->else_comment = $7->adhesive;
                    $$ = to_return;
                };

While_Statement: WHILE '(' Decl_Or_Expression ')' Block_Or_Statement
                {
                    While_Statement* to_return = new While_Statement;
                    to_return->copy_from(*$1);
                    to_return->condition = dynamic_cast<Expression*>($3);
                    assert(to_return->condition); //TODO: decl in while not supported
                    to_return->statements.data = std::move($5->data);
                    to_return->embedded = $5->adhesive;
                    $$ = to_return;
                }
        |       DO Anonymous_Block WHILE '(' Expression ')' ';'
                {
                    While_Statement* to_return = new While_Statement;
                    to_return->copy_from(*$1);
                    to_return->do_while = true;
                    to_return->condition = $5;
                    to_return->statements.data = std::move($2->data);
                    to_return->embedded = NULL;
                    $$ = to_return;
                };

For_Statement: FOR '(' Statement Expression ';' Expression ')' Block_Or_Statement
                {
                    For_Statement* to_return = new For_Statement;
                    to_return->copy_from(*$1);
                    to_return->init_stmt = dynamic_cast<Statement*>($3);
                    to_return->condition = $4;
                    to_return->post = $6;
                    to_return->statements.data = std::move($8->data);
                    to_return->embedded = $8->adhesive;
                    $$ = to_return;
                };

Switch_Statement: SWITCH '(' Expression ')' Anonymous_Block
                {
                    Switch_Statement* to_return = new Switch_Statement;
                    to_return->copy_from(*$1);
                    to_return->condition = $3;
                    to_return->statements.data = std::move($5->data);
                    to_return->embedded = NULL;
                    $$ = to_return;
                }
        |       SWITCH '(' Expression ')' Comment Anonymous_Block
                {
                    Switch_Statement* to_return = new Switch_Statement;
                    to_return->copy_from(*$1);
                    to_return->condition = $3;
                    to_return->statements.data = std::move($6->data);
                    to_return->embedded = $5;
                    $$ = to_return;
                };

Decl_Or_Expression: Single_Var_Decl { $$ = $1; }
        | Expression { $$ = $1; };

Expression: '(' Expression ')' { $$ = new Parenthetical($2); }
        |       Expression '+' Expression { $$ = handle_binary_operator_expression($1,"+",$3); }
        |       Expression '-' Expression { $$ = handle_binary_operator_expression($1,"-",$3); }
        |       Expression '*' Expression { $$ = handle_binary_operator_expression($1,"*",$3); }
        |       Expression '/' Expression { $$ = handle_binary_operator_expression($1,"/",$3); }
        |       Expression '%' Expression { $$ = handle_binary_operator_expression($1,"%",$3); }
        |       Expression LEFT_SHIFT Expression { $$ = handle_binary_operator_expression($1,"<<",$3); }
        |       Expression RIGHT_SHIFT Expression { $$ = handle_binary_operator_expression($1,">>",$3); }
        |       Expression UNSIGNED_RIGHT_SHIFT Expression { $$ = handle_binary_operator_expression($1,">>>",$3); }
        |       Expression '&' Expression { $$ = handle_binary_operator_expression($1,"&",$3); }
        |       Expression '|' Expression { $$ = handle_binary_operator_expression($1,"|",$3); }
        |       Expression ',' Expression { $$ = handle_binary_operator_expression($1,",",$3); }
        |       Expression '^' Expression { $$ = handle_binary_operator_expression($1,"^",$3); }
        |       Expression OR Expression { $$ = handle_binary_operator_expression($1,"||",$3); }
        |       Expression AND Expression { $$ = handle_binary_operator_expression($1,"&&",$3); }
        |       Expression '<' Expression { $$ = handle_binary_operator_expression($1,"<",$3); }
        |       Expression LT_EQUALS Expression { $$ = handle_binary_operator_expression($1,"<=",$3); }
        |       Expression EQUALS Expression { $$ = handle_binary_operator_expression($1,"==",$3); }
        |       Expression GT_EQUALS Expression { $$ = handle_binary_operator_expression($1,">=",$3); }
        |       Expression '>' Expression { $$ = handle_binary_operator_expression($1,">",$3); }
        |       Expression NOT_EQUALS Expression { $$ = handle_binary_operator_expression($1,"!=",$3); }
        |       Expression '=' Expression { $$ = handle_binary_operator_expression($1,"=",$3); }
        |       Expression PLUS_ASSIGN Expression { $$ = handle_binary_operator_expression($1,"+=",$3); }
        |       Expression MINUS_ASSIGN Expression { $$ = handle_binary_operator_expression($1,"-=",$3); }
        |       Expression TIMES_ASSIGN Expression { $$ = handle_binary_operator_expression($1,"*=",$3); }
        |       Expression DIV_ASSIGN Expression { $$ = handle_binary_operator_expression($1,"/=",$3); }
        |       Expression MOD_ASSIGN Expression { $$ = handle_binary_operator_expression($1,"%=",$3); }
        |       Expression AND_ASSIGN Expression { $$ = handle_binary_operator_expression($1,"&=",$3); }
        |       Expression OR_ASSIGN Expression { $$ = handle_binary_operator_expression($1,"|=",$3); }
        |       Expression XOR_ASSIGN Expression { $$ = handle_binary_operator_expression($1,"^=",$3); }
        |       Expression LEFT_SHIFT_ASSIGN Expression { $$ = handle_binary_operator_expression($1,"<<=",$3); }
        |       Expression RIGHT_SHIFT_ASSIGN Expression { $$ = handle_binary_operator_expression($1,">>=",$3); }
        |       Expression '?' Expression ':' Expression { $$ = new Ternary_Conditional($1,$3,$5); }
        |       '*' Expression %prec DEREF_RULE
                {
                    if(language_mode!=CPP_MODE)
                        report_error("Explicit pointers do not exist in Java.",$2);
                    assert(false); //not implemented
                }
        | '&'   Expression %prec ADDROF_RULE
                {
                    if(language_mode!=CPP_MODE)
                        report_error("Explicit pointers do not exist in Java.",$2);
                    assert(false); //not implemented
                }
        | '!'   Expression
                {
                    Function_Call* func_call = new Function_Call;
                    func_call->copy_from(*$2);                    
                    func_call->called_func_name = "!_";
                    func_call->parameters = {$2};
                    $$ = func_call;                     
                }
        |       PP_INC IDENTIFIER %prec PP_INC
                {
                    Function_Call* func_call = new Function_Call;
                    func_call->copy_from(*$2);                    
                    func_call->called_func_name = "++_";
                    func_call->parameters = {new Atomic_Expression{0,$2->value}};
                    $$ = func_call;
                }
        |       MM_DEC IDENTIFIER %prec MM_DEC
                {
                    Function_Call* func_call = new Function_Call;
                    func_call->copy_from(*$2);                    
                    func_call->called_func_name = "--_";
                    func_call->parameters = {new Atomic_Expression{0,$2->value}};
                    $$ = func_call;
                }
        |       IDENTIFIER PP_INC %prec PP_INC_SUFFIX_RULE
                {
                    Function_Call* func_call = new Function_Call;
                    func_call->copy_from(*$1);                    
                    func_call->called_func_name = "_++";
                    func_call->parameters = {new Atomic_Expression{0,$1->value}};
                    $$ = func_call;
                }
        |       IDENTIFIER MM_DEC %prec MM_DEC_SUFFIX_RULE
                {
                    Function_Call* func_call = new Function_Call;
                    func_call->copy_from(*$1);                    
                    func_call->called_func_name = "_--";
                    func_call->parameters = {new Atomic_Expression{0,$1->value}};
                    $$ = func_call;
                }
        | '-'   Expression %prec UMINUS_RULE
                {
                    Function_Call* func_call = new Function_Call;
                    func_call->copy_from(*$2);                    
                    func_call->called_func_name = "-_";
                    func_call->parameters = {$2};
                    $$ = func_call;
                }
        | '+'   Expression %prec UPLUS_RULE { $$ = $2; }
        | '('   Primitive_Type_Keyword ')' Expression %prec CAST_RULE
                {
                     Function_Call* cast_call = new Function_Call;
                     cast_call->called_func_name = string("(")+$2->value.as_string()+")";
                     cast_call->parameters.push_back($4);
                     $$ = cast_call;
                }
        | '('   IDENTIFIER ')' Expression %prec CAST_RULE
                {
                     Function_Call* cast_call = new Function_Call;
                     cast_call->called_func_name = string("(")+$2->value.as_string()+")";
                     cast_call->parameters.push_back($4);
                     $$ = cast_call;
                }
        |       Multipart_Identifier '(' Argument_List ')' %prec DOT
                {
                    Function_Call* func_call = new Function_Call;
                    func_call->copy_from(*$1);
                    func_call->called_func_name = $1->value.as_string();
                    func_call->parameters = std::move($3->data);
                    $$ = func_call;
                } //This really needs to be completely redone to correctly support objects' method calls
        |       NEW IDENTIFIER '(' Argument_List ')' %prec NEW //TODO: C++
                {
                    Function_Call* func_call = new Function_Call;
                    func_call->copy_from(*$2);
                    func_call->called_func_name = $2->value.as_string();
                    func_call->parameters = std::move($4->data);
                    func_call->new_call = true;
                    $$ = func_call;
                }
        |       NEW Type_Phrase '[' Expression ']' %prec NEW
                {
                    Function_Call* func_call = new Function_Call;
                    Symbol_Modifiers* base_type_modifiers = $2;
                    string ret_type_name = base_type_modifiers->static_type->name + "[]";
                    func_call->copy_from(*$2);
                    func_call->called_func_name = ret_type_name;
                    func_call->parameters.push_back($4);
                    func_call->new_call = true;

                    //Need to insert the type of this expression into typetab if it's not there already
                    if(!typetab.count(ret_type_name))
                    {
                         Type rtype{};
                         rtype.name = ret_type_name;
                         rtype.is_array = true;
                         rtype.base_or_parent_types.push_back(base_type_modifiers->static_type);
                         typetab[ret_type_name] = rtype;
                    }

                    $$ = func_call;
                }
        |       Expression DOT IDENTIFIER %prec DOT
                {
                    Aggregate_Access* agac = new Aggregate_Access{$1,DOT,$3->value.as_string()};
                    agac->copy_from(*$2);
                    $$ = agac;
                }
        |       Expression ARROW IDENTIFIER %prec ARROW
                {
                    if(language_mode!=CPP_MODE)
                        report_error("Arrow symbol does not exist in Java.",$2);
                    Aggregate_Access* agac = new Aggregate_Access{$1,ARROW,$3->value.as_string()};
                    agac->copy_from(*$2);
                    $$ = agac;
                }
        |       Expression SCOPE IDENTIFIER %prec SCOPE
                {
                    if(language_mode!=CPP_MODE)
                        report_error("Double colon does not exist in Java.",$2);
                    Aggregate_Access* agac = new Aggregate_Access{$1,SCOPE,$3->value.as_string()};
                    agac->copy_from(*$2);
                    $$ = agac;
                }
        |       Expression '[' Expression ']'
                {
                    Array_Access* aa = new Array_Access{$1,$3};
                    aa->copy_from(*$3);
                    $$ = aa;
                }
        |       IDENTIFIER
                {
                    Atomic_Expression* ae = new Atomic_Expression{0,$1->value};
                    ae->copy_from(*$1);
                    $$ = ae;
                }
        |       Literal %prec IDENTIFIER { $$ = $1; };

Multipart_Identifier: Multipart_Identifier DOT IDENTIFIER 
                {
                     Token* tok = $1;
                     tok->value.ary.ptr = strdup((string(tok->value.as_string())+"."+$3->value.as_string()).c_str());
                     $$ = $1;
                } //TODO: C++
        |       IDENTIFIER { $$ = $1; };

Argument_List: Argument_List ',' Expression
                {
                    Expression_List* elist = $1;
                    Expression* append_exp = $3;
                    elist->data.push_back(append_exp);
                    $$ = $1;
                }
        |       Expression %prec SCOPE
                {
                    Expression_List* elist = new Expression_List;
                    elist->copy_from(*$1);
                    elist->data.push_back($1);
                    $$ = elist;
                }
        |       /*empty*/ { $$ = new Expression_List; };

Literal: STR_LITERAL { $$ = handle_literal(STR_LITERAL,$1); }
        |       BOOLEAN_LITERAL { $$ = handle_literal(BOOLEAN_LITERAL,$1); }
        |       CHAR_LITERAL { $$ = handle_literal(CHAR_LITERAL,$1); }
        |       INT_LITERAL { $$ = handle_literal(INT_LITERAL,$1); }
        |       FLOAT_LITERAL { $$ = handle_literal(FLOAT_LITERAL,$1); }
        |       DOUBLE_LITERAL { $$ = handle_literal(DOUBLE_LITERAL,$1); };

%%

Class_Decl* handle_aggregate_declaration(Token* keyword, Token* class_ident_ast, Decl_List* body, Token* instantiation)
{
    Class_Decl* to_return = instantiation ? new Class_Decl_Instantiated{} : new Class_Decl;
    to_return->copy_from(*keyword);

    string atype_str = keyword->value.as_string();
    string class_ident_str = class_ident_ast->value.as_string();
    string type_name = get_scope_qualifier_prefix()+class_ident_str;

    Type type{};
    type.name = type_name;
    if(atype_str=="class")
        type.aggregate_type = CLASS;
    else if(atype_str=="struct")
    {
        if(language_mode==JAVA_MODE)
            report_error("Java doesn't have structs",keyword);
        type.aggregate_type = STRUCT;
    }
    else if(atype_str=="union")
    {
        if(language_mode==JAVA_MODE)
            report_error("Java doesn't have unions",keyword);
        type.aggregate_type = UNION;
        type.is_union = true;
    }
    else
        assert(false);
    /*if(...class declared with inheritance clause...)*/
         //TODO: handle inheritance
    /*else*/ if(language_mode==JAVA_MODE)
         type.base_or_parent_types.push_back(&typetab["Object"]);
    

    //Okay, now, walk the declaration list to fill out the contents
    for(Declaration* cur_decl : body->data)
        if(Var_Decl_Statement* var_decl = dynamic_cast<Var_Decl_Statement*>(cur_decl))
            type.contents.push_back(var_decl);
    typetab[type_name] = type;

    //TODO: can't accurately detect duplicate declarations with current design
    /*if(typetab.count(type_name))
          report_error("Duplicate declaration of aggregate "+type_name,class_ident_ast);*/
        
    //The new type goes in level 0 of symtab, too.
    Symbol symbol;
    symbol.name = type_name;
    symbol.type = NULL;
    symbol.value.ary.length = -1;
    symbol.value.ary.ptr = to_return;

    //Report error if we're already in the symtab, then overwrite previous definition.
    if(symtab.front().count(type_name))
        report_error("Duplicate definition of aggregate "+type_name,class_ident_ast);
    symtab.front().erase(type_name);
    symtab.front().insert(make_pair(type_name,symbol));

    //Is there an instantiation in Java mode?  Report error.
    if(instantiation && language_mode==JAVA_MODE)
        report_error("C++-style immediate instantiation in Java",instantiation);
    else if(instantiation)
    {
        Var_Decl_Statement* vdec = dynamic_cast<Var_Decl_Statement*>(to_return);
        vdec->modifiers.visibility = current_visibility_level;
        vdec->modifiers.static_type = &(typetab[type_name]);
        vdec->name = assert_cast<Token*>(instantiation)->value.as_string();
    }
    
    //It remains to finish filling out the to_return Class_Decl
    to_return->type = &(typetab[type_name]);
    to_return->body = std::move(*body);

    //If no constructor was defined, we need to create a default one
    if(find_if(symtab.front().begin(),symtab.front().end(),[&](const auto& keyval) { return keyval.first.find(type_name+"::"+class_ident_str+"(")==0; })==symtab.front().end())
    {
         if(!typetab.count("()"))
         {
              Type type{};
              type.name = "()";
              type.parameters.push_back(NULL);
              typetab["()"] = type;
         }
         Symbol to_insert;
         to_insert.name = type_name+"::"+class_ident_str+"()";
         to_insert.type = &typetab["()"];

         Func_Decl* empty_constructor = new Func_Decl{};
         empty_constructor->modifiers.visibility = PUBLIC;
         empty_constructor->modifiers.static_type = &typetab["()"];
         to_insert.value.ary.ptr = empty_constructor;
         symtab.front()[to_insert.name] = to_insert;
    }
    
    return to_return;
}


Func_Decl* handle_function_declaration(AST_Node* pre_modifiers_, AST_Node* post_modifiers_, AST_Node* name_, AST_Node* parameters_, AST_Node* body_)
{
    Symbol_Modifiers* pre_modifiers = assert_cast<Symbol_Modifiers*>(pre_modifiers_);
    Symbol_Modifiers* post_modifiers = assert_cast<Symbol_Modifiers*>(post_modifiers_);
    string name = assert_cast<Token*>(name_)->value.as_string();
    Decl_List* parameters = assert_cast<Decl_List*>(parameters_);
    Statement_List* body = assert_cast<Statement_List*>(body_);

    //Find type for function
    string param_suffix;
    param_suffix+="(";
    if(parameters->data.size())
    {
        bool pre_comma = false;
        for(Declaration* param : parameters->data)
        {
            if(pre_comma)
                param_suffix+=",";
            else
                pre_comma = true;

            Type* paramtype_ptr = dynamic_cast<Var_Decl_Statement*>(param)->modifiers.static_type;
            string paramtype_str = paramtype_ptr->name;
            if(paramtype_str=="") //ugh, incomplete type
            {
                 auto it = find_if(typetab.begin(),typetab.end(),[&](const auto& x) { return &x.second==paramtype_ptr; });
                 assert(it!=typetab.end());
                 paramtype_str = it->first;
            }
            param_suffix+=paramtype_str;
        }
    }
    param_suffix+=")";
    
    string type_name = (pre_modifiers->static_type ? pre_modifiers->static_type->name : name)+param_suffix;
    string scoped_name = get_scope_qualifier_prefix()+name+param_suffix;

    if(!typetab.count(type_name))
    {
        //Construct type for typetab
        Type type{};
        type.name = type_name;
        type.parameters.push_back(pre_modifiers->static_type ? pre_modifiers->static_type : &(typetab[qualify_unscoped_reference(name)])); //retval = first param
        for(Declaration* param : parameters->data)
            type.parameters.push_back(dynamic_cast<Var_Decl_Statement*>(param)->modifiers.static_type);
        typetab[type_name] = type;
    }

    //Symbol modifiers
    Symbol_Modifiers modifiers = *pre_modifiers;
    modifiers.const_m = post_modifiers ? post_modifiers->const_m : false;    
    modifiers.static_type = &(typetab[type_name]);
    for(Declaration* param : parameters->data)
        modifiers.parameters.push_back(dynamic_cast<Var_Decl_Statement*>(param));
    
    //Set modifiers visibility
    if(modifiers.visibility==0)
        if(language_mode==CPP_MODE)
            modifiers.visibility = current_visibility_level;
        else
        {
            report_error("Warning: no visibility level declared for "+name,name_);
            modifiers.visibility = PUBLIC;
         }
    
    //Put it together
    Func_Decl* to_return = new Func_Decl;
    to_return->copy_from(*name_);
    to_return->modifiers = modifiers;
    to_return->return_value_is_const = pre_modifiers->const_m;
    to_return->name = name;
    if(body)
    {
        to_return->body = std::move(*body);
        
        Symbol symbol;
        symbol.name = scoped_name;
        symbol.type = &typetab[type_name];
        symbol.value.ary.length = -1;
        symbol.value.ary.ptr = to_return;
        symtab.front().insert(make_pair(scoped_name,symbol));
    }
    
    return to_return;
}

Function_Call* handle_binary_operator_expression(Expression* left, string op_name, Expression* right)
{
     Function_Call* func_call = new Function_Call;
     func_call->copy_from(*left);
     func_call->called_func_name = op_name;
     func_call->parameters = {left,right};
     return func_call;
};

Atomic_Expression* handle_literal(int tok_id, AST_Node* literal)
{
    return new Atomic_Expression{tok_id,assert_cast<Token*>(literal)->value};
}

/*Deal with arrays where the []s follow the identifier rather than type
  Discouraging this syntax is the one thing I like about Java over C++.*/
void array_suffix_hack(Var_Decl_Statement* vds)
{
    int first_bracket = vds->name.find('[');
    int num_suffixes = first_bracket==-1 ? 0 : (vds->name.length() - vds->name.find('['))/2;
    for(int i=0; i<num_suffixes; i++)
    {
         Type* bt_ptr = vds->modifiers.static_type;
         Type ary_type{};
         ary_type.name = bt_ptr->name + "[]";
         ary_type.is_array = true;
         ary_type.base_or_parent_types.push_back(bt_ptr);
         typetab[ary_type.name] = ary_type;
         vds->modifiers.static_type = &typetab[ary_type.name];
    }

    vds->name = vds->name.substr(0,first_bracket);
}

string get_unique_anonymous_placeholder_str()
{
    static int anonymous_type_counter;
    return "<Anonymous aggregate type #"+to_string(anonymous_type_counter++)+">";
}

void push_scope(AST_Node* keyword_)
{
    string keyword = assert_cast<Token*>(keyword_)->value.as_string();
    typetab[get_scope_qualifier_prefix()+keyword] = Type{}; //incomplete type
    static_scopes.push_back(keyword);
}

void report_error(string message, const AST_Node* node)
{
    yyerror((message+" on line "+to_string(node->line)+" in file "+node->file+".").c_str());
}
