%{
#include "../ast/ast.h"
#include "../common/util.h"

#include "monga_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void yyerror (char const *);

ast_program_node *ast;

%}

%union {
  double              float_val;
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

%type <string_list> name_list name_list_tail
%type <decl_node> declaration_list declaration decl_var decl_func params params_tail decl_var_list
%type <statement_node> statement statement_list block
%type <exp_node> exp
%type <var_node> var
%type <func_call_node> func_call
%type <exp_list> exp_list exp_list_tail
%type <type> type base_type

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

program : declaration_list { ast = $1; }

declaration_list : /* empty */
                    {
                      $$ = NULL;
                    }
                 | declaration declaration_list
                    {
                      $1->next = $2;
                      $$ = $1;
                    }

declaration : decl_var | decl_func
{
  $$ = $1;
}

decl_var : type name_list ';'
{
  $$ = NULL;

  string_list_node *list;
  for (list = $2; list; list = list->next)
  {
    ast_decl_node *temp = (ast_decl_node *)malloc(sizeof(ast_decl_node));

    temp->next                = $$;
    temp->tag                 = decl_var_tag;
    temp->value.decl_var.type = $1;
    temp->value.decl_var.name = list->string;
    $$                        = temp;
  }
}

name_list : TK_ID name_list_tail
{
  $$         = (string_list_node *)malloc(sizeof(string_list_node));
  $$->string = $1;
  $$->next   = $2;
}
name_list_tail : /* empty */   { $$ = NULL; }
               | ',' name_list { $$ = $2;   }

type : base_type    { $$ = $1; }
     | type '[' ']' { $$ = $1; ++($$.dimensions); }

base_type : TK_INT_T   {  $$.type = ast_int_type;   $$.dimensions = 0; }
          | TK_CHAR_T  {  $$.type = ast_char_type;  $$.dimensions = 0; }
          | TK_FLOAT_T {  $$.type = ast_float_type; $$.dimensions = 0; }
          | TK_VOID_T  {  $$.type = ast_void_type;  $$.dimensions = 0; }

decl_func : type TK_ID '(' params ')' block
{
  $$ = (ast_decl_node *)malloc(sizeof(ast_decl_node));

  $$->next                        = NULL;
  $$->tag                         = decl_func_tag;
  $$->value.decl_func.return_type = $1;
  $$->value.decl_func.name        = $2;
  $$->value.decl_func.params      = $4;
  $$->value.decl_func.body        = $6;
}
params : /* empty */             { $$ = NULL; }
       | type TK_ID params_tail
        {
          $$ = (ast_decl_node *)malloc(sizeof(ast_decl_node));

          $$->next                = $3;
          $$->tag                 = decl_var_tag;
          $$->value.decl_var.type = $1;
          $$->value.decl_var.name = $2;
        }

params_tail : /* empty */                  { $$ = NULL;}
            | ',' type TK_ID params_tail
              {
                $$ = (ast_decl_node *)malloc(sizeof(ast_decl_node));

                $$->next                = $4;
                $$->tag                 = decl_var_tag;
                $$->value.decl_var.type = $2;
                $$->value.decl_var.name = $3;
              }

block : '{' decl_var_list statement_list '}'
{
  $$ = (ast_statement_node *)malloc(sizeof(ast_statement_node));

  $$->tag                         = ast_statement_block_tag;
  $$->value.block_node.decl_vars  = $2;
  $$->value.block_node.statements = $3;
  $$->next                        = NULL;
}
statement : TK_IF '(' exp ')' statement
            {
              $$ = (ast_statement_node *)malloc(sizeof(ast_statement_node));

              $$->tag                          = ast_statement_if_else_tag;
              $$->next                         = NULL;
              $$->value.if_else_node.exp       = $3;
              $$->value.if_else_node.if_body   = $5;
              $$->value.if_else_node.else_body = NULL;
            }
          | TK_IF '(' exp ')' statement TK_ELSE statement
            {
              $$ = (ast_statement_node *)malloc(sizeof(ast_statement_node));

              $$->tag                          = ast_statement_if_else_tag;
              $$->next                         = NULL;
              $$->value.if_else_node.exp       = $3;
              $$->value.if_else_node.if_body   = $5;
              $$->value.if_else_node.else_body = $7;
            }
          | TK_WHILE '(' exp ')' statement
            {
              $$ = (ast_statement_node *)malloc(sizeof(ast_statement_node));
              
              $$->tag                        = ast_statement_while_tag;
              $$->next                       = NULL;
              $$->value.while_node.exp       = $3;
              $$->value.while_node.statement = $5;
            }
          | var '=' exp ';'
            {
              $$ = (ast_statement_node *)malloc(sizeof(ast_statement_node));
              
              $$->tag  = ast_statement_assign_tag;
              $$->next = NULL;
              $$->value.assign_node.var = $1;
              $$->value.assign_node.exp = $3;
            }
          | TK_RETURN ';'
            {
              $$ = (ast_statement_node *)malloc(sizeof(ast_statement_node));
              
              $$->tag                   = ast_statement_return_tag;
              $$->next                  = NULL;
              $$->value.return_node.exp = NULL;
            }
          | TK_RETURN exp ';'
            {
              $$ = (ast_statement_node *)malloc(sizeof(ast_statement_node));
              
              $$->tag                   = ast_statement_return_tag;
              $$->next                  = NULL;
              $$->value.return_node.exp = $2;
            }
          | func_call ';'
            {
              $$ = (ast_statement_node *)malloc(sizeof(ast_statement_node));
              
              $$->tag                  = ast_statement_func_call_tag;
              $$->next                 = NULL;
              $$->value.func_call_node = $1;
            }
          | block
            {
              $$ = $1;
            }

decl_var_list : /* empty */ { $$ = NULL; }
              | decl_var decl_var_list
                {
                  $$ = $1;
                  $$->next = $2;
                }
statement_list : /* empty */ { $$ = NULL; }
               | statement statement_list
                {
                  $$ = $1;
                  $$->next = $2;
                }

var : TK_ID
      {
        $$.tag                              = non_indexed;
        $$.value.non_indexed.value.var_name = $1;
      }
    | exp '[' exp ']'
      {
        $$.tag                 = indexed;
        $$.value.indexed.base  = $1;
        $$.value.indexed.index = $3;
      }
exp : TK_FLOAT_LITERAL
      {
        $$ = (ast_exp_node *)malloc(sizeof(ast_exp_node));
        $$->tag = float_literal_tag;
        $$->value.float_literal = $1;
      }
    | TK_INT_LITERAL
      {
        $$ = (ast_exp_node *)malloc(sizeof(ast_exp_node));
        $$->tag = int_literal_tag;
        $$->value.int_literal = $1;
      }
    | TK_STRING_LITERAL
      {
        $$ = (ast_exp_node *)malloc(sizeof(ast_exp_node));
        $$->tag = string_literal_tag;
        $$->value.string_literal = $1;
      }
    | var
      {
        $$ = (ast_exp_node *)malloc(sizeof(ast_exp_node));
        $$->tag = var_tag;
        $$->value.var = $1;
      }
    | '(' exp ')'
      {
        $$ = $2;
      }
    | func_call
      {
        $$ = (ast_exp_node *)malloc(sizeof(ast_exp_node));
        $$->tag = func_call_tag;
        $$->value.func_call = $1;
      }
    | TK_NEW type '[' exp ']'
      {
        $$ = (ast_exp_node *)malloc(sizeof(ast_exp_node));
        $$->tag                    = operator_new_tag;
        $$->value.operator_new.type = $2;
        $$->value.operator_new.exp  = $4;
      }
    | '-' exp %prec UNM
      {
        $$ = (ast_exp_node *)malloc(sizeof(ast_exp_node));
        $$->tag = unop_tag;
        $$->value.unop_node.tag = minus_tag;
        $$->value.unop_node.exp = $2;
      }
    | exp '+' exp
      {
        $$ = (ast_exp_node *)malloc(sizeof(ast_exp_node));
        $$->tag = binop_tag;
        $$->value.binop_node.tag  = add_tag;
        $$->value.binop_node.exp1 = $1;
        $$->value.binop_node.exp2 = $3;
      }
    | exp '-' exp
      {
        $$ = (ast_exp_node *)malloc(sizeof(ast_exp_node));
        $$->tag = binop_tag;
        $$->value.binop_node.tag  = subtract_tag;
        $$->value.binop_node.exp1 = $1;
        $$->value.binop_node.exp2 = $3;
      }
    | exp '*' exp
      {
        $$ = (ast_exp_node *)malloc(sizeof(ast_exp_node));
        $$->tag = binop_tag;
        $$->value.binop_node.tag  = multiply_tag;
        $$->value.binop_node.exp1 = $1;
        $$->value.binop_node.exp2 = $3;
      }
    | exp '/' exp
      {
        $$ = (ast_exp_node *)malloc(sizeof(ast_exp_node));
        $$->tag = binop_tag;
        $$->value.binop_node.tag  = divide_tag;
        $$->value.binop_node.exp1 = $1;
        $$->value.binop_node.exp2 = $3;
      }
    | exp TK_EQ exp
      {
        $$ = (ast_exp_node *)malloc(sizeof(ast_exp_node));
        $$->tag = binop_tag;
        $$->value.binop_node.tag  = equal_tag;
        $$->value.binop_node.exp1 = $1;
        $$->value.binop_node.exp2 = $3;
      }
    | exp TK_LEQ exp
      {
        $$ = (ast_exp_node *)malloc(sizeof(ast_exp_node));
        $$->tag = binop_tag;
        $$->value.binop_node.tag  = less_equal_tag;
        $$->value.binop_node.exp1 = $1;
        $$->value.binop_node.exp2 = $3;
      }
    | exp TK_GEQ exp
      {
        $$ = (ast_exp_node *)malloc(sizeof(ast_exp_node));
        $$->tag = binop_tag;
        $$->value.binop_node.tag  = greater_equal_tag;
        $$->value.binop_node.exp1 = $1;
        $$->value.binop_node.exp2 = $3;
      }
    | exp '<' exp
      {
        $$ = (ast_exp_node *)malloc(sizeof(ast_exp_node));
        $$->tag = binop_tag;
        $$->value.binop_node.tag  = less_tag;
        $$->value.binop_node.exp1 = $1;
        $$->value.binop_node.exp2 = $3;
      }
    | exp '>' exp
      {
        $$ = (ast_exp_node *)malloc(sizeof(ast_exp_node));
        $$->tag = binop_tag;
        $$->value.binop_node.tag  = greater_tag;
        $$->value.binop_node.exp1 = $1;
        $$->value.binop_node.exp2 = $3;
      }
    | '!' exp
      {
        $$ = (ast_exp_node *)malloc(sizeof(ast_exp_node));
        $$->tag = unop_tag;
        $$->value.unop_node.tag = logical_not_tag;
        $$->value.unop_node.exp = $2;
      }
    | exp TK_AND exp
      {
        $$ = (ast_exp_node *)malloc(sizeof(ast_exp_node));
        $$->tag = binop_tag;
        $$->value.binop_node.tag  = logical_and_tag;
        $$->value.binop_node.exp1 = $1;
        $$->value.binop_node.exp2 = $3;
      }
    | exp TK_OR exp
      {
        $$ = (ast_exp_node *)malloc(sizeof(ast_exp_node));
        $$->tag = binop_tag;
        $$->value.binop_node.tag  = logical_or_tag;
        $$->value.binop_node.exp1 = $1;
        $$->value.binop_node.exp2 = $3;
      }

func_call : TK_ID '(' exp_list ')'
{
  $$.value.func_name = $1;
  $$.params          = $3;
}
exp_list : /* empty */        { $$ = NULL; }
         | exp exp_list_tail
          {
            $$ = (exp_list_node *)malloc(sizeof(exp_list_node));
            $$->exp  = $1;
            $$->next = $2;
          }
exp_list_tail : /* empty */            { $$ = NULL; }
              | ',' exp exp_list_tail
                {
                  $$ = (exp_list_node *)malloc(sizeof(exp_list_node));
                  $$->exp  = $2;
                  $$->next = $3;
                }

%%

void yyerror (char const *s) {
  fprintf( stderr, "%s (line:%d)\n", s, yylineno );
}