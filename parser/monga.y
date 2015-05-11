%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int yylex (void);
void yyerror (char const *);
extern int yylineno;
%}

%union {
  double float_val;
  int    int_val;
  char  *string_val;
  char   char_val;
}

%token <string_val> TK_ID
%token <int_val>    TK_EQ
%token <int_val>    TK_LEQ
%token <int_val>    TK_GEQ
%token <int_val>    TK_AND
%token <int_val>    TK_OR
%token <string_val> TK_COMMENT
%token <int_val>    TK_CHAR_T
%token <int_val>    TK_FLOAT_T
%token <int_val>    TK_INT_T
%token <int_val>    TK_VOID_T
%token <int_val>    TK_IF
%token <int_val>    TK_ELSE
%token <int_val>    TK_WHILE
%token <int_val>    TK_NEW
%token <int_val>    TK_RETURN
%token <float_val>  TK_FLOAT_LITERAL
%token <int_val>    TK_INT_LITERAL
%token <string_val> TK_STRING_LITERAL
%token              TK_UNKNOWN

%left  TK_OR
%left  TK_AND
%left  TK_EQ
%left  TK_LEQ TK_GEQ '<' '>'
%left  '-' '+'
%left  '*' '/'
%right '!' UNM
%left '['

%start program

%%

program : declaration_list
declaration_list : /* empty */ | declaration declaration_list
declaration : decl_var | decl_func

decl_var : type name_list ';'

name_list : TK_ID name_list_tail
name_list_tail : /* empty */ | ',' name_list

type : base_type | type '[' ']'
base_type : TK_INT_T | TK_CHAR_T | TK_FLOAT_T | TK_VOID_T

decl_func : type TK_ID '(' params ')' block
params : /* empty */ | type TK_ID params_tail
params_tail : /* empty */ | ',' type TK_ID params_tail

block : '{' decl_var_list statement_list '}'
statement : TK_IF '(' exp ')' statement
          | TK_IF '(' exp ')' statement TK_ELSE statement
          | TK_WHILE '(' exp ')' statement
          | var '=' exp ';'
          | TK_RETURN ';'
          | TK_RETURN exp ';'
          | func_call ';'
          | block

decl_var_list : /* empty */ | decl_var decl_var_list
statement_list : /* empty */ | statement statement_list

var : TK_ID | exp '[' exp ']'
exp : TK_FLOAT_LITERAL | TK_INT_LITERAL | TK_STRING_LITERAL
    | var
    | '(' exp ')'
    | func_call
    | TK_NEW type '[' exp ']'
    | '-' exp %prec UNM
    | exp '+' exp
    | exp '-' exp
    | exp '*' exp
    | exp '/' exp
    | exp TK_EQ exp
    | exp TK_LEQ exp
    | exp TK_GEQ exp
    | exp '<' exp
    | exp '>' exp
    | '!' exp
    | exp TK_AND exp
    | exp TK_OR exp

func_call : TK_ID '(' exp_list ')'
exp_list : /* empty */ | exp exp_list_tail
exp_list_tail : /* empty */ | ',' exp exp_list_tail

%%

void yyerror (char const *s) {
  fprintf( stderr, "%s (line:%d)\n", s, yylineno );
}