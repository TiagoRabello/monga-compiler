#ifndef AST_H
#define AST_H

// Forward declaration of Declaration Nodes.
typedef struct _ast_decl_node      ast_decl_node;
typedef struct _ast_decl_var_node  ast_decl_var_node;  /* i.e. int a;       */
typedef struct _ast_decl_func_node ast_decl_func_node; /* i.e. int foo() {} */

// Forward declaration of Expression Nodes.
typedef struct _ast_exp_node              ast_exp_node;
typedef struct _ast_exp_binop_node        ast_exp_binop_node;
typedef struct _ast_exp_unop_node         ast_exp_unop_node;
typedef struct _ast_exp_operator_new_node ast_exp_operator_new_node;
typedef struct _ast_exp_type_cast_node ast_exp_type_cast_node;

// Forward declaration of Statement Nodes.
typedef struct _ast_statement_node         ast_statement_node;
typedef struct _ast_statement_return_node  ast_statement_return_node;
typedef struct _ast_statement_if_else_node ast_statement_if_else_node;
typedef struct _ast_statement_while_node   ast_statement_while_node;
typedef struct _ast_statement_assign_node  ast_statement_assign_node;
typedef struct _ast_statement_block_node   ast_statement_block_node;

// Definition of a program.
typedef ast_decl_node ast_program_node;

#include "ast_declaration.h"
#include "ast_expression.h"
#include "ast_statement.h"
#include "ast_type.h"

#endif