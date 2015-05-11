#ifndef AST_STATEMENT_H
#define AST_STATEMENT_H

#include "ast_common.h"
#include "ast_expression.h"

// Return node
struct _ast_statement_return_node
{
  ast_exp_node *exp; // Optional
};

// Block node
struct _ast_statement_block_node
{
  ast_decl_node      *decl_vars;
  ast_statement_node *statements;
};

// If else node
struct _ast_statement_if_else_node
{
  ast_exp_node       *exp;
  ast_statement_node *if_body;
  ast_statement_node *else_body; // Optional
};

// While node
struct _ast_statement_while_node
{
  ast_exp_node       *exp;
  ast_statement_node *statement;
};

// Assign node
struct _ast_statement_assign_node
{
  ast_var_node  var;
  ast_exp_node *exp;
};

// Statement node
typedef enum
{
  ast_statement_if_else_tag,
  ast_statement_while_tag,
  ast_statement_assign_tag,
  ast_statement_return_tag,
  ast_statement_func_call_tag,
  ast_statement_block_tag

} ast_statement_tag;

struct _ast_statement_node
{
  ast_statement_tag tag;

  union
  {
    ast_statement_if_else_node if_else_node;
    ast_statement_while_node   while_node;
    ast_statement_assign_node  assign_node;
    ast_statement_return_node  return_node;
    ast_statement_block_node   block_node;
    ast_func_call_node         func_call_node;
  } value;

  ast_statement_node *next;
};

#endif