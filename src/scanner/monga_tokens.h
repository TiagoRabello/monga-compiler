/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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

#ifndef YY_YY_SRC_SCANNER_MONGA_TOKENS_H_INCLUDED
# define YY_YY_SRC_SCANNER_MONGA_TOKENS_H_INCLUDED
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
    TK_ID = 258,
    TK_EQ = 259,
    TK_LEQ = 260,
    TK_GEQ = 261,
    TK_AND = 262,
    TK_OR = 263,
    TK_COMMENT = 264,
    TK_CHAR_T = 265,
    TK_FLOAT_T = 266,
    TK_INT_T = 267,
    TK_VOID_T = 268,
    TK_IF = 269,
    TK_ELSE = 270,
    TK_WHILE = 271,
    TK_NEW = 272,
    TK_RETURN = 273,
    TK_FLOAT_LITERAL = 274,
    TK_INT_LITERAL = 275,
    TK_STRING_LITERAL = 276,
    TK_UNKNOWN = 277,
    UNM = 278
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 17 "src/parser/monga.y" /* yacc.c:1909  */

  float               float_val;
  int                 int_val;
  char               *string_val;
  char                char_val;
  string_list_node   *string_list;
  ast_decl_node      *decl_node;
  ast_statement_node *statement_node;
  ast_exp_node       *exp_node;
  ast_var_node        var_node;
  ast_func_call_node  func_call_node;
  ast_type            type;
  exp_list_node      *exp_list;

#line 93 "src/scanner/monga_tokens.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SRC_SCANNER_MONGA_TOKENS_H_INCLUDED  */
