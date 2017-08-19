/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Skeleton interface for Bison GLR parsers in C

   Copyright (C) 2002-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_PHILOTES_H_INCLUDED
# define YY_YY_PHILOTES_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    PLUS_ASSIGN = 258,
    MINUS_ASSIGN = 259,
    TIMES_ASSIGN = 260,
    DIV_ASSIGN = 261,
    MOD_ASSIGN = 262,
    LEFT_SHIFT_ASSIGN = 263,
    RIGHT_SHIFT_ASSIGN = 264,
    AND_ASSIGN = 265,
    OR_ASSIGN = 266,
    XOR_ASSIGN = 267,
    OR = 268,
    AND = 269,
    EQUALS = 270,
    NOT_EQUALS = 271,
    LT_EQUALS = 272,
    GT_EQUALS = 273,
    LEFT_SHIFT = 274,
    RIGHT_SHIFT = 275,
    UNSIGNED_RIGHT_SHIFT = 276,
    DOT_STAR = 277,
    ARROW_STAR = 278,
    PP_INC = 279,
    MM_DEC = 280,
    UPLUS_RULE = 281,
    UMINUS_RULE = 282,
    CAST_RULE = 283,
    DEREF_RULE = 284,
    ADDROF_RULE = 285,
    SIZEOF = 286,
    NEW = 287,
    DELETE = 288,
    PP_INC_SUFFIX_RULE = 289,
    MM_DEC_SUFFIX_RULE = 290,
    DOT = 291,
    ARROW = 292,
    ELSE = 293,
    SCOPE = 294,
    STR_LITERAL = 295,
    BOOLEAN_LITERAL = 296,
    CHAR_LITERAL = 297,
    INT_LITERAL = 298,
    FLOAT_LITERAL = 299,
    DOUBLE_LITERAL = 300,
    IDENTIFIER = 301,
    IMPORT = 302,
    INCLUDE = 303,
    CURSOR = 304,
    SIGNPOST = 305,
    PRAYER = 306,
    IF = 307,
    SWITCH = 308,
    WHILE = 309,
    FOR = 310,
    DO = 311,
    RETURN = 312,
    BREAK = 313,
    CONTINUE = 314,
    UNSIGNED = 315,
    SIGNED = 316,
    CONST = 317,
    FINAL = 318,
    STATIC = 319,
    INLINE = 320,
    PUBLIC = 321,
    PROTECTED = 322,
    PRIVATE = 323,
    CASE = 324,
    BOOLEAN = 325,
    BYTE = 326,
    CHAR = 327,
    SHORT = 328,
    INT = 329,
    LONG = 330,
    FLOAT = 331,
    DOUBLE = 332,
    AUTO = 333,
    VOID = 334,
    CLASS = 335,
    STRUCT = 336,
    UNION = 337,
    COMMENT = 338,
    PROGRAM_HERALD = 339,
    STATEMENT_HERALD = 340
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 39 "philotes.y" /* glr.c:197  */

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

#line 163 "philotes.h" /* glr.c:197  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PHILOTES_H_INCLUDED  */
