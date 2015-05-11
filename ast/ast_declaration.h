#ifndef AST_DECLARATION_H
#define AST_DECLARATION_H

#include "ast_statement.h"
#include "ast_type.h"

struct _ast_decl_var_node
{
  const char *name;
  ast_type    type;
};

struct _ast_decl_func_node
{
  ast_type            return_type;
  const char         *name;
  ast_decl_node      *params;
  ast_statement_node *body;
};

typedef enum
{
  decl_var_tag,
  decl_func_tag
  
} ast_decl_tag;

struct _ast_decl_node
{
  ast_decl_tag tag;

  union
  {
    ast_decl_var_node  decl_var;
    ast_decl_func_node decl_func;
  } value;

  ast_decl_node *next;
};

#endif