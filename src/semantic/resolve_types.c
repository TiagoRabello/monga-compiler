#include "semantic.h"

#include "../ast/ast.h"
#include "../ast/ast_printer.h"
#include "../ast/ast_type.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef TRUE
#define TRUE (0 == 0)
#endif

#ifndef FALSE
#define FALSE (0 != 0)
#endif

void resolve_types_decl     (ast_decl_node      *node);
void resolve_types_decl_var (ast_decl_var_node  *node);
void resolve_types_decl_func(ast_decl_func_node *node);

ast_type resolve_types_exp      (ast_exp_node       *node);
ast_type resolve_types_func_call(ast_func_call_node *node);
ast_type resolve_types_var      (ast_var_node       *node);
ast_type resolve_types_binop    (ast_exp_binop_node *node);
ast_type resolve_types_unop     (ast_exp_unop_node  *node);

void resolve_types_statement(ast_statement_node         *node);
void resolve_types_if_else  (ast_statement_if_else_node *node);
void resolve_types_while    (ast_statement_while_node   *node);
void resolve_types_assign   (ast_statement_assign_node  *node);
void resolve_types_return   (ast_statement_return_node  *node);
void resolve_types_block    (ast_statement_block_node   *node);

ast_basic_type common_type(ast_basic_type type1, ast_basic_type type2)
{
  if (type1 == ast_void_type  || type2 == ast_void_type)  { return ast_void_type; }
  if (type1 == ast_float_type || type2 == ast_float_type) { return ast_float_type; }
  if (type1 == ast_int_type   || type2 == ast_int_type)   { return ast_int_type; }

  return ast_char_type;
}

void add_type_cast(ast_exp_node *node, ast_type type)
{
  ast_exp_node *new_node = (ast_exp_node *)malloc(sizeof(ast_exp_node));
  memcpy(new_node, node, sizeof(ast_exp_node));
  node->tag                  = type_cast_tag;
  node->value.type_cast.type = type;
  node->value.type_cast.exp  = new_node;
}

int is_convertible_to_int(ast_type type)
{
  switch (type.type)
  {
    case ast_int_type:
    case ast_char_type:
      return (type.dimensions == 0);
    case ast_float_type:
    case ast_void_type:
      return FALSE;
  }
}

// TODO: Verify return type.
// TODO: Add type cast.
// TODO: Modify print functions to work after linking.

void resolve_types(ast_program_node *ast)
{
  assert(ast);

  ast_decl_node *node = ast;
  for (; node; node = node->next)
  {
    resolve_types_decl(node);
  }
}

void resolve_types_decl(ast_decl_node *node)
{
  switch (node->tag)
  {
    case decl_var_tag:
      resolve_types_decl_var(&(node->value.decl_var));
      break;
    case decl_func_tag:
      resolve_types_decl_func(&(node->value.decl_func));
      break;
  }
}

void resolve_types_decl_var(ast_decl_var_node *node)
{
  if (node->type.type == ast_void_type)
  {
    fprintf(stderr, "Error: Variable cannot be of type void: ");
    print_ast_decl_var_node(stderr, node, 0);
    exit(1);
  }
}

void resolve_types_decl_func(ast_decl_func_node *node)
{
  ast_decl_node *param;
  for (param = node->params; param; param = param->next)
  {
    assert(param->tag == decl_var_tag);
    resolve_types_decl_var(&(param->value.decl_var));
  }

  resolve_types_statement(node->body);
}

ast_type resolve_types_exp(ast_exp_node *node)
{
  ast_type exp_type;
  switch (node->tag)
  {
    case binop_tag:
      exp_type = resolve_types_binop(&(node->value.binop_node));
    case unop_tag:
      exp_type = resolve_types_unop(&(node->value.unop_node));
      break;
    case float_literal_tag:
      exp_type.type = ast_float_type;
      exp_type.dimensions = 0;
      break;
    case int_literal_tag:
      exp_type.type = ast_int_type;
      exp_type.dimensions = 0;
      break;
    case string_literal_tag:
      exp_type.type = ast_char_type;
      exp_type.dimensions = 1;
      break;
    case var_tag:
      exp_type = resolve_types_var(&(node->value.var));
      if (exp_type.type == ast_char_type && exp_type.dimensions == 0)
      {
        ast_type cast_type = { ast_int_type, 0 };
        add_type_cast(node, cast_type);
      }
      break;
    case func_call_tag:
      exp_type = resolve_types_func_call(&(node->value.func_call));
      break;
    case operator_new_tag:
      exp_type.type = ast_int_type;
      exp_type.dimensions = 0;
      break;
  }

  return exp_type;
}

ast_type resolve_types_func_call(ast_func_call_node *node)
{
  return node->value.decl->value.decl_func.return_type;
}

ast_type resolve_types_var(ast_var_node *node)
{
  ast_type var_type;
  switch (node->tag)
  {
    case indexed:
      {
        ast_type index_type = resolve_types_exp(node->value.indexed.index);
        ast_type base_type  = resolve_types_exp(node->value.indexed.base);

        if (is_convertible_to_int(index_type) == FALSE)
        {
          fprintf(stderr, "Error: ");
          print_ast_type(stderr, index_type);
          fprintf(stderr, " is not convertible to int: ");
          print_ast_exp_node(stderr, node->value.indexed.index);
          fprintf(stderr, "\n");
          fprintf(stderr, "Note: Conversion required by: ");
          print_ast_var_node(stderr, node);
          fprintf(stderr, "\n");
          exit(1);
        }


        if (index_type.type != ast_int_type)
        {
          index_type.type = ast_int_type;
          add_type_cast(node->value.indexed.index, index_type);
        }

        if (base_type.dimensions == 0)
        {
          fprintf(stderr, "Error: Variable of type ");
          print_ast_type(stderr, base_type);
          fprintf(stderr, " is not indexable: ");
          print_ast_exp_node(stderr, node->value.indexed.base);
          fprintf(stderr, "\n");
          fprintf(stderr, "Note: Indexing required by: ");
          print_ast_var_node(stderr, node);
          fprintf(stderr, "\n");
          exit(1); 
        }

        var_type = base_type;
        --(var_type.dimensions);
      }
      break;
    case non_indexed:
      var_type = node->value.non_indexed.value.decl->value.decl_var.type;
      break;
  }
  return var_type;
}

ast_type resolve_types_binop(ast_exp_binop_node *node)
{
  ast_type exp_type;
  ast_type lhs_type = resolve_types_exp(node->exp1);
  ast_type rhs_type = resolve_types_exp(node->exp2);

  if (lhs_type.type == ast_void_type)
  {
    fprintf(stderr, "Error: Cannot operate on expressions of type void: ");
    print_ast_exp_binop_node(stderr, node);
    fprintf(stderr, "\nNote: Expression of type void: ");
    print_ast_exp_node(stderr, node->exp1);
    fprintf(stderr, "\n");
    exit(1);
  }

  if (rhs_type.type == ast_void_type)
  {
    fprintf(stderr, "Error: Cannot operate on expressions of type void: ");
    print_ast_exp_binop_node(stderr, node);
    fprintf(stderr, "\nNote: Expression of type void: ");
    print_ast_exp_node(stderr, node->exp2);
    fprintf(stderr, "\n");
    exit(1);
  }

  if (lhs_type.dimensions != 0)
  {
    fprintf(stderr, "Error: Cannot operate on expressions of indexable type (");
    print_ast_type(stderr, lhs_type);
    fprintf(stderr, "): ");
    print_ast_exp_binop_node(stderr, node);
    fprintf(stderr, "\nNote: Expression of type ");
    print_ast_type(stderr, lhs_type);
    fprintf(stderr, ": ");
    print_ast_exp_node(stderr, node->exp1);
    fprintf(stderr, "\n");
  }

  if (rhs_type.dimensions != 0)
  {
    fprintf(stderr, "Error: Cannot operate on expressions of indexable type (");
    print_ast_type(stderr, rhs_type);
    fprintf(stderr, "): ");
    print_ast_exp_binop_node(stderr, node);
    fprintf(stderr, "\nNote: Expression of type ");
    print_ast_type(stderr, rhs_type);
    fprintf(stderr, ": ");
    print_ast_exp_node(stderr, node->exp2);
    fprintf(stderr, "\n");
  }

  switch (node->tag)
  {
    case add_tag:
    case subtract_tag:
    case multiply_tag:
    case divide_tag:
      exp_type.type = common_type(lhs_type.type, rhs_type.type);
      exp_type.dimensions = 0;

      if (lhs_type.type != exp_type.type) { add_type_cast(node->exp1, exp_type); }
      if (rhs_type.type != exp_type.type) { add_type_cast(node->exp2, exp_type); }
      break;

    case equal_tag:
    case less_equal_tag:
    case greater_equal_tag:
    case greater_tag:
    case less_tag:
      exp_type.type       = ast_int_type;
      exp_type.dimensions = 0;
      {
        ast_type common = { common_type(lhs_type.type, rhs_type.type), 0 };

        if (lhs_type.type != common.type) { add_type_cast(node->exp1, common); }
        if (rhs_type.type != common.type) { add_type_cast(node->exp2, common); }
      }
      break;

    case logical_and_tag:
    case logical_or_tag:
      if (is_convertible_to_int(lhs_type) == FALSE)
      {
        fprintf(stderr, "Error: ");
        print_ast_type(stderr, lhs_type);
        fprintf(stderr, " is not convertible to int: ");
        print_ast_exp_node(stderr, node->exp1);
        fprintf(stderr, "\n");
        fprintf(stderr, "Note: Conversion required by: ");
        print_ast_exp_binop_node(stderr, node);
        fprintf(stderr, ")\n");
        exit(1);
      }

      if (is_convertible_to_int(rhs_type) == FALSE)
      {
        fprintf(stderr, "Error: ");
        print_ast_type(stderr, rhs_type);
        fprintf(stderr, " is not convertible to int: ");
        print_ast_exp_node(stderr, node->exp2);
        fprintf(stderr, "\n");
        fprintf(stderr, "Note: Conversion required by: ");
        print_ast_exp_binop_node(stderr, node);
        fprintf(stderr, ")\n");
        exit(1);
      }

      exp_type.type       = ast_int_type;
      exp_type.dimensions = 0;

      if (lhs_type.type != ast_int_type) { add_type_cast(node->exp1, exp_type); }
      if (rhs_type.type != ast_int_type) { add_type_cast(node->exp2, exp_type); }
      break;
  }

  return exp_type;
}

ast_type resolve_types_unop(ast_exp_unop_node *node)
{
  ast_type exp_type = resolve_types_exp(node->exp);

  if (exp_type.type == ast_void_type)
  {
    fprintf(stderr, "Error: Cannot operate on expressions of type void: ");
    print_ast_exp_unop_node(stderr, node);
    fprintf(stderr, "\nNote: Expression of type void: ");
    print_ast_exp_node(stderr, node->exp);
    fprintf(stderr, "\n");
    exit(1);
  }

  if (exp_type.dimensions != 0)
  {
    fprintf(stderr, "Error: Cannot operate on expressions of indexable type (");
    print_ast_type(stderr, exp_type);
    fprintf(stderr, "): ");
    print_ast_exp_unop_node(stderr, node);
    fprintf(stderr, "\nNote: Expression of type ");
    print_ast_type(stderr, exp_type);
    fprintf(stderr, ": ");
    print_ast_exp_node(stderr, node->exp);
    fprintf(stderr, "\n");
  }

  switch (node->tag)
  {
    case minus_tag:
      // no additional restrictions.
      break;
    case logical_not_tag:
      if (is_convertible_to_int(exp_type) == FALSE)
      {
        fprintf(stderr, "Error: ");
        print_ast_type(stderr, exp_type);
        fprintf(stderr, " is not convertible to int: ");
        print_ast_exp_node(stderr, node->exp);
        fprintf(stderr, "\n");
        fprintf(stderr, "Note: Conversion required by: ");
        print_ast_exp_unop_node(stderr, node);
        fprintf(stderr, "\n");
        exit(1);
      }

      if (exp_type.type != ast_int_type)
      {
        exp_type.type = ast_int_type;
        add_type_cast(node->exp, exp_type);
      }
      break;
  }

  return exp_type;
}

void resolve_types_if_else(ast_statement_if_else_node *node)
{
  ast_type exp_type = resolve_types_exp(node->exp);
  if (is_convertible_to_int(exp_type) == FALSE)
  {
    fprintf(stderr, "Error: ");
    print_ast_type(stderr, exp_type);
    fprintf(stderr, " is not convertible to int: ");
    print_ast_exp_node(stderr, node->exp);
    fprintf(stderr, "\n");
    fprintf(stderr, "Note: Conversion required by: if (");
    print_ast_exp_node(stderr, node->exp);
    fprintf(stderr, ")\n");
    exit(1);
  }

  if (exp_type.type != ast_int_type)
  {
    exp_type.type = ast_int_type;
    add_type_cast(node->exp, exp_type);
  }

  resolve_types_statement(node->if_body);

  if (node->else_body)
  {
    resolve_types_statement(node->else_body);
  }
}

void resolve_types_while(ast_statement_while_node *node)
{
  ast_type exp_type = resolve_types_exp(node->exp);
  if (is_convertible_to_int(exp_type) == FALSE)
  {
    fprintf(stderr, "Error: ");
    print_ast_type(stderr, exp_type);
    fprintf(stderr, " is not convertible to int: ");
    print_ast_exp_node(stderr, node->exp);
    fprintf(stderr, "\n");
    fprintf(stderr, "Note: Conversion required by: while (");
    print_ast_exp_node(stderr, node->exp);
    fprintf(stderr, ")\n");
    exit(1);
  }

  if (exp_type.type != ast_int_type)
  {
    exp_type.type = ast_int_type;
    add_type_cast(node->exp, exp_type);
  }

  resolve_types_statement(node->statement);
}

void resolve_types_assign(ast_statement_assign_node *node)
{
  ast_type var_type = resolve_types_var(&(node->var));
  ast_type exp_type = resolve_types_exp(node->exp);

  // Cover "operator new" case.
  if (var_type.dimensions != 0 && exp_type.type == ast_int_type) { return; }

  if (var_type.type != exp_type.type || var_type.dimensions != exp_type.dimensions)
  {
    fprintf(stderr, "Error: Cannot assign a value of type ");
    print_ast_type(stderr, exp_type);
    fprintf(stderr, " to a variable of type ");
    print_ast_type(stderr, var_type);
    fprintf(stderr, ": ");
    print_ast_statement_assign_node(stderr, node, 0);
    exit(1);
  }
}

void resolve_types_return(ast_statement_return_node *node)
{
  if (node->exp)
  {
    resolve_types_exp(node->exp); 
  }
}

void resolve_types_block(ast_statement_block_node *node)
{
  ast_decl_node *decl_node;
  for (decl_node = node->decl_vars; decl_node; decl_node = decl_node->next)
  {
    assert(decl_node->tag == decl_var_tag);
    resolve_types_decl_var(&(decl_node->value.decl_var));
  }

  ast_statement_node *statement_node;
  for (statement_node = node->statements; statement_node; statement_node = statement_node->next)
  {
    resolve_types_statement(statement_node);
  }
}

void resolve_types_statement(ast_statement_node *node)
{
  assert(node);

  switch (node->tag)
  {
    case ast_statement_if_else_tag:
      resolve_types_if_else(&(node->value.if_else_node));
      break;
    case ast_statement_while_tag:
      resolve_types_while(&(node->value.while_node));
      break;
    case ast_statement_assign_tag:
      resolve_types_assign(&(node->value.assign_node));
      break;
    case ast_statement_return_tag:
      resolve_types_return(&(node->value.return_node));
      break;
    case ast_statement_func_call_tag:
      {
        ast_type return_type = resolve_types_func_call(&(node->value.func_call_node));
        if (return_type.type != ast_void_type)
        {
          fprintf(stderr, "Warning: Ignoring return value: ");
          print_ast_statement_func_call_node(stderr, &(node->value.func_call_node), 0);
          fprintf(stderr, "Node: Return value is of type ");
          print_ast_type(stderr, return_type);
          fprintf(stderr, "\n");
        }
      }
      break;
    case ast_statement_block_tag:
      resolve_types_block(&(node->value.block_node));
      break;
  }
}