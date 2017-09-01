%{
#include "Token.hpp"
#include "philotes.h"
#include "philotes_config.hpp"
#include "symtab.hpp"
#include "StringFunctions.h"

static void mkyylv();
static string process_str_literal(char terminator = '"');
int yyerror(const char* e_str);

static bool statement_mode;
static bool prev_token_semicolon;
static int brace_level;

%}

delim    [ \t]
ws       {delim}+
letter   [A-Za-z_]
digit    [0-9]
hexdigit [0-9A-Fa-f]
id       {letter}({letter}|{digit})*
integer  (((0b|0)?{digit}+)|(0x{hexdigit}+))[Ll]?
fpoint   ({integer}\.{integer}*(e(-|\+)?{integer}+)?|{integer}+e(-|\+)?{integer}+|\.{integer}+e(-|\+)?{integer}+)

%%

%{
  if(inject_herald)
  {
       int to_inject = inject_herald;
       statement_mode = inject_herald == STATEMENT_HERALD;
       inject_herald = 0;
       return to_inject;
  }
%}

 /*Whitespace*/
{ws}     { prev_token_semicolon = false; }
\n       { current_line++;
           int o_p_tok_sem = prev_token_semicolon;
           prev_token_semicolon = false;
           if(o_p_tok_sem && statement_mode && !brace_level)
                return EOF;
         }

 /*Preprocessor*/
import { mkyylv();
         string imported_name;
         char x = yyinput();
         while(x==' ' || x=='\t')
             x = yyinput();
         while(x!='\n' && x!=';')
         {
             imported_name+=x;
             x = yyinput();
         }
         if(x!='\n')
         {
             yylval.token->value.ary.length = imported_name.length()+1;
             yylval.token->value.ary.ptr = strdup(imported_name.c_str());
             return IMPORT;
         }
         else
         {
             yyerror("Lexer: invalid import!");
             current_line++;
         } }

#include { mkyylv();
           string included_name;
           char delim, x;
           while((x = yyinput())!='\n')
           {
               if(x=='"' || x=='<')
               {
                   delim = x;
                   break;
               }
           }
           while((x = yyinput())!='\n')
           {
               if(x==delim)
                   break;
               included_name+=x;
           }
           if(x!='\n')
           {
               yylval.token->value.ary.length = included_name.length()+1;
               yylval.token->value.ary.ptr = strdup(included_name.c_str());
               return INCLUDE;
           }
           else
           {
               yyerror("Lexer: invalid #include!");
               current_line++;
           } }

#define  { yyerror("Lexer: #define not supported!");
           while(yyinput()!='\n');
           current_line++; }

#cursor  { mkyylv(); prev_token_semicolon = true; return CURSOR; }
#signpost { mkyylv(); return SIGNPOST; }
#prayer  { mkyylv(); return PRAYER; }

 /*Operators*/

 /*Computation*/
\+       { mkyylv(); return '+'; }
-        { mkyylv(); return '-'; } /*also unary*/
\*       { mkyylv(); return '*'; }
\/       { mkyylv(); return '/'; }
%        { mkyylv(); return '%'; }
\<\<     { mkyylv(); return LEFT_SHIFT; }
\>\>     { mkyylv(); return RIGHT_SHIFT; }
\>\>\>   { mkyylv(); return UNSIGNED_RIGHT_SHIFT; }
&        { mkyylv(); return '&'; }
\|       { mkyylv(); return '|'; }
~        { mkyylv(); return '~'; } /*unary*/
,        { mkyylv(); return ','; }
\?       { mkyylv(); return '?'; }
\^       { mkyylv(); return '^'; }

 /*Boolean logic*/
\|\|     { mkyylv(); return OR; }
&&       { mkyylv(); return AND; }
!        { mkyylv(); return '!'; } /*unary*/

 /*Logical Comparison*/
==       { mkyylv(); return EQUALS; }
!=       { mkyylv(); return NOT_EQUALS; }
\<=      { mkyylv(); return LT_EQUALS; }
\>=      { mkyylv(); return GT_EQUALS; }

 /*Assignment*/
=        { mkyylv(); return '='; }
\+=      { mkyylv(); return PLUS_ASSIGN; }
-=       { mkyylv(); return MINUS_ASSIGN; }
\*=      { mkyylv(); return TIMES_ASSIGN; }
\/=      { mkyylv(); return DIV_ASSIGN; }
%=       { mkyylv(); return MOD_ASSIGN; }
&=       { mkyylv(); return AND_ASSIGN; }
\|=      { mkyylv(); return OR_ASSIGN; }
\^=      { mkyylv(); return XOR_ASSIGN; }
\<\<=    { mkyylv(); return LEFT_SHIFT_ASSIGN; }
\>\>=    { mkyylv(); return RIGHT_SHIFT_ASSIGN; }

 /*Pre/Post Increment/Decrement*/
\+\+     { mkyylv(); return PP_INC; }
--       { mkyylv(); return MM_DEC; }

 /*Member Access*/
\.       { mkyylv(); return DOT; }
-\>      { mkyylv(); return ARROW; }
::       { mkyylv(); return SCOPE; }

 /*Separators*/
:        { mkyylv(); return ':'; }
;        { mkyylv(); prev_token_semicolon = true; return ';'; }

 /*Operator keywords*/
sizeof   { mkyylv(); return SIZEOF; }
new      { mkyylv(); return NEW; }
delete   { mkyylv(); return DELETE; }

 /*Grouping characters*/
\{       { mkyylv();
           if(statement_mode)
                brace_level++;
           return '{'; }
\(       { mkyylv(); return '('; }
\[       { mkyylv(); return '['; }
\<       { mkyylv(); return '<'; }
\>       { mkyylv(); return '>'; }
\]       { mkyylv(); return ']'; }
\)       { mkyylv(); return ')'; }
\}       { mkyylv();
           if(statement_mode)
                prev_token_semicolon = !--brace_level;
           return '}'; }

 /*Statement-affinity keywords*/
if       { mkyylv(); return IF; }
else     { mkyylv(); return ELSE; }
switch   { mkyylv(); return SWITCH; }
while    { mkyylv(); return WHILE; }
for      { mkyylv(); return FOR; }
do       { mkyylv(); return DO; }
break    { mkyylv(); return BREAK; }
return   { mkyylv(); return RETURN; }

 /*Modifier-affinity keywords*/
const    { mkyylv(); return CONST; }
final    { mkyylv(); return FINAL; }
static   { mkyylv(); return STATIC; }
register { /*ignored*/ }
inline   { mkyylv(); return INLINE; }
public   { mkyylv(); return PUBLIC; }
protected { mkyylv(); return PROTECTED; }
private  { mkyylv(); return PRIVATE; }
case     { mkyylv(); return CASE; }

 /*Type Modifier keywords*/
unsigned { mkyylv(); return UNSIGNED; }
signed   { mkyylv(); return SIGNED; }

 /*Type-affinity keywords*/
byte     { mkyylv(); yylval.token->value.ary.ptr = strdup("byte"); return BYTE; }
char     { mkyylv(); yylval.token->value.ary.ptr = strdup("char"); return CHAR; }
short    { mkyylv(); yylval.token->value.ary.ptr = strdup("short"); return SHORT; }
int      { mkyylv(); yylval.token->value.ary.ptr = strdup("int"); return INT; }
long     { mkyylv(); yylval.token->value.ary.ptr = strdup("long"); return LONG; }
float    { mkyylv(); yylval.token->value.ary.ptr = strdup("float"); return FLOAT; }
double   { mkyylv(); yylval.token->value.ary.ptr = strdup("double"); return DOUBLE; }
auto     { mkyylv(); yylval.token->value.ary.ptr = strdup("auto"); return AUTO; }
void     { mkyylv(); yylval.token->value.ary.ptr = strdup("void"); return VOID; }

 /*Declarative-affinity keywords*/
class    { mkyylv(); yylval.token->value.ary.ptr = strdup("class"); return CLASS; }
struct   { mkyylv(); yylval.token->value.ary.ptr = strdup("struct"); return STRUCT; }
union    { mkyylv(); yylval.token->value.ary.ptr = strdup("union"); return UNION; }

 /*Literal constant rules*/
true     { mkyylv(); yylval.token->value.numeric_val = 1; return BOOLEAN_LITERAL; }
false    { mkyylv(); yylval.token->value.numeric_val = 0; return BOOLEAN_LITERAL; }

\"       { mkyylv();
           string literal = process_str_literal();
           yylval.token->value.ary.length = literal.length()+1;
           yylval.token->value.ary.ptr = new char[literal.length()+1];
           memcpy(yylval.token->value.ary.ptr,literal.c_str(),literal.length()+1);
           return STR_LITERAL; }

\' { mkyylv();
     string retval = process_str_literal('\'');
     if(retval.length()!=1)
          yyerror("Lexer: invalid char literal!");
     yylval.token->value.numeric_val = retval[0];
     return CHAR_LITERAL; }

{integer} { mkyylv(); yylval.token->value.numeric_val = strtol(yytext,NULL,0); return yytext[yyleng-1]!='L' && yytext[yyleng-1]!='l' ? INT_LITERAL : LONG_LITERAL; }
{fpoint}f? { mkyylv(); yylval.token->value.float_val = atof(yytext); return yytext[yyleng-1]=='f' || yytext[yyleng-1]=='F' ? FLOAT_LITERAL : DOUBLE_LITERAL; }

 /*Identifier rule*/
{id} { mkyylv();
       yylval.token->value.ary.length = yyleng;
       yylval.token->value.ary.ptr = strdup(yytext);
       return IDENTIFIER; }

 /*Comments*/
\/\* { mkyylv();
       string comment = "/*";
       char next_char;
       while(next_char = yyinput())
       {
            if(next_char=='*')
                 if((next_char = yyinput())=='/')
                      break;
                 else
                      comment = comment+'*'+next_char;
            else
                 comment+=next_char;
            
            if(next_char=='\n')
                 current_line++;
       }
       comment+="*/";
       yylval.token->value.ary.length = comment.size()+1;
       yylval.token->value.ary.ptr = strdup(comment.c_str());
       return COMMENT; }
\/\/ { mkyylv();
       string comment = "//";
       char next_char;
       while((next_char = yyinput()) && next_char!='\n')
            comment+=next_char;
       comment+='\n';
       current_line++;
       yylval.token->value.ary.length = comment.size()+1;
       yylval.token->value.ary.ptr = strdup(comment.c_str());
       return COMMENT; }

%%

static void mkyylv()
{
    yylval.token = new Token();
    yylval.token->line = current_line;
    yylval.token->file = current_file;
}

static string process_str_literal(char terminator)
{
    string literal;
    bool escaped = false;
    char next_char;
    while(((next_char = yyinput())!=terminator || escaped) && next_char)
    {
         if(!escaped && next_char=='\\')
         {
              escaped = true;
              continue;
         }
         if(escaped)
         {
              switch(next_char)
              {
              case 'n': literal+='\n';
                   break;
              case '0': literal+='\0';
                   break;
              case 'r': literal+='\r';
                   break;
              case '\\': literal+='\\';
                   break;
              case 't': literal+='\t';
                   break;
              case '"': literal+='"';
                   break;
              case '\'': literal+='\'';
                   break;
              }
              escaped = false;
              continue;
         }
         literal+=next_char;
    }
    return literal;
}

int yyerror(const char* e_str)
{
    error_msg.push_back(e_str);
    return 0;
}
