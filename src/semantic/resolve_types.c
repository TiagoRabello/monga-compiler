#include "semantic.h"

#include "../ast/ast.h"
#include "../ast/ast_printer.h"
#include "../ast/ast_type.h"
#include "../common/diagnostics.h"

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

#ifndef MAX
#define MAX(A,B) (((A) > (B)) ? (A) : (B))
#endif

void resolve_types_decl     (ast_decl_node      *node);
void resolve_types_decl_var (ast_decl_var_node  *node);
void resolve_types_decl_func(ast_decl_func_node *node);

ast_type resolve_types_exp      (ast_exp_node       *node);
ast_type resolve_types_func_call(ast_func_call_node *node);
ast_type resolve_types_var      (ast_var_node       *node);
ast_type resolve_types_binop    (ast_exp_binop_node *node);
ast_type resolve_types_unop     (ast_exp_unop_node  *node);

return_status resolve_types_statement(ast_statement_node         *node, ast_type expected_return_type);
return_status resolve_types_if_else  (ast_statement_if_else_node *node, ast_type expected_return_type);
return_status resolve_types_while    (ast_statement_while_node   *node, ast_type expected_return_type);
return_status resolve_types_assign   (ast_statement_assign_node  *node, ast_type expected_return_type);
return_status resolve_types_return   (ast_statement_return_node  *node, ast_type expected_return_type);
return_status resolve_types_block    (ast_statement_block_node   *node, ast_type expected_return_type);

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

  return FALSE;
}

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
    fprintf(stderr, ERROR_LABEL"Variable cannot be of type void: ");
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

  return_status body_ret_status = resolve_types_statement(node->body, node->return_type);

  // If return type is void it doesn't matter if we returned or not.
  if (node->return_type.type == ast_void_type && node->return_type.dimensions == 0) { return; }

  switch (body_ret_status)
  {
    case didnt_return:
      fprintf(stderr, ERROR_LABEL"Control reaches end of non-void function: ");
      print_ast_decl_func_signature(stderr, node, 0);
      exit(1);
      break;
    case maybe_returned:
      fprintf(stderr, WARNING_LABEL"Control may reach end of non-void function: ");
      print_ast_decl_func_signature(stderr, node, 0);
      break;
    case returned:
      break;
  }
}

ast_type resolve_types_exp(ast_exp_node *node)
{
  switch (node->tag)
  {
    case binop_tag:
      node->type = resolve_types_binop(&(node->value.binop_node));
      break;
    case unop_tag:
      node->type = resolve_types_unop(&(node->value.unop_node));
      break;
    case float_literal_tag:
      node->type.type = ast_float_type;
      node->type.dimensions = 0;
      break;
    case int_literal_tag:
      node->type.type = ast_int_type;
      node->type.dimensions = 0;
      break;
    case string_literal_tag:
      node->type.type = ast_char_type;
      node->type.dimensions = 1;
      break;
    case var_tag:
      node->type = resolve_types_var(&(node->value.var));
      if (node->type.type == ast_char_type && node->type.dimensions == 0)
      {
        ast_type cast_type = { ast_int_type, 0 };
        add_type_cast(node, cast_type);
      }
      break;
    case func_call_tag:
      node->type = resolve_types_func_call(&(node->value.func_call));
      break;
    case operator_new_tag:
      node->type.type = ast_int_type;
      node->type.dimensions = 0;
      break;
  }

  return node->type;
}

ast_type resolve_types_func_call(ast_func_call_node *node)
{
  int num_processed_params = 0;
  exp_list_node *param;
  ast_decl_node *param_decl;
  for (param = node->params, param_decl = node->value.decl->value.decl_func.params; param && param_decl; param = param->next, param_decl = param_decl->next)
  {
    ast_type exp_type   = resolve_types_exp(param->exp);
    ast_type param_type = param_decl->value.decl_var.type;

    if (param_type.type != exp_type.type || param_type.dimensions != exp_type.dimensions)
    {
      fprintf(stderr, ERROR_LABEL"Cannot pass a value of ");
      print_ast_type(stderr, exp_type);
      fprintf(stderr, " type as argument to a function waiting on ");
      print_ast_type(stderr, param_type);
      fprintf(stderr, " type: ");
      print_ast_exp_node(stderr, param->exp);
      fprintf(stderr, "\n"INFO_LABEL"Argument at position %d of function call: ", num_processed_params);
      print_ast_func_call_node(stderr, node);
      fprintf(stderr, "\n"INFO_LABEL"Function signature: ");
      print_ast_decl_func_signature(stderr, &(node->value.decl->value.decl_func), 0);
      exit(1);
    }

    ++num_processed_params;
  }

  if (param != NULL)
  {
    int extra_params = 0;
    while (param) { param = param->next; ++extra_params; }

    fprintf(stderr, ERROR_LABEL"Too many arguments to function %s: ", node->value.decl->value.decl_func.name);
    print_ast_func_call_node(stderr, node);
    fprintf(stderr, "\n"INFO_LABEL"Function expecting %d parameter%s but called with %d: ", num_processed_params
                                                                                          , (num_processed_params != 1) ? "s" : ""
                                                                                          , num_processed_params + extra_params);
    print_ast_decl_func_signature(stderr, &(node->value.decl->value.decl_func), 0);
    exit(1);
  }

  if (param_decl != NULL)
  {
    int extra_params = 0;
    while (param_decl) { param_decl = param_decl->next; ++extra_params; }

    fprintf(stderr, ERROR_LABEL"Too few arguments to function %s: ", node->value.decl->value.decl_func.name);
    print_ast_func_call_node(stderr, node);
    fprintf(stderr, "\n"INFO_LABEL"Function expecting %d parameter%s but called with %d: ", num_processed_params + extra_params
                                                                                          , (num_processed_params + extra_params != 1) ? "s" : ""
                                                                                          , num_processed_params);
    print_ast_decl_func_signature(stderr, &(node->value.decl->value.decl_func), 0);
    exit(1);
  }

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
          fprintf(stderr, ERROR_LABEL);
          print_ast_type(stderr, index_type);
          fprintf(stderr, " is not convertible to int: ");
          print_ast_exp_node(stderr, node->value.indexed.index);
          fprintf(stderr, "\n");
          fprintf(stderr, INFO_LABEL"Conversion required by: ");
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
          fprintf(stderr, ERROR_LABEL"Variable of type ");
          print_ast_type(stderr, base_type);
          fprintf(stderr, " is not indexable: ");
          print_ast_exp_node(stderr, node->value.indexed.base);
          fprintf(stderr, "\n");
          fprintf(stderr, INFO_LABEL"Indexing required by: ");
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
    fprintf(stderr, ERROR_LABEL"Cannot operate on expressions of type void: ");
    print_ast_exp_binop_node(stderr, node);
    fprintf(stderr, "\n"INFO_LABEL"Expression of type void: ");
    print_ast_exp_node(stderr, node->exp1);
    fprintf(stderr, "\n");
    exit(1);
  }

  if (rhs_type.type == ast_void_type)
  {
    fprintf(stderr, ERROR_LABEL"Cannot operate on expressions of type void: ");
    print_ast_exp_binop_node(stderr, node);
    fprintf(stderr, "\n"INFO_LABEL"Expression of type void: ");
    print_ast_exp_node(stderr, node->exp2);
    fprintf(stderr, "\n");
    exit(1);
  }

  if (lhs_type.dimensions != 0)
  {
    fprintf(stderr, ERROR_LABEL"Cannot operate on expressions of indexable type (");
    print_ast_type(stderr, lhs_type);
    fprintf(stderr, "): ");
    print_ast_exp_binop_node(stderr, node);
    fprintf(stderr, "\n"INFO_LABEL"Expression of type ");
    print_ast_type(stderr, lhs_type);
    fprintf(stderr, ": ");
    print_ast_exp_node(stderr, node->exp1);
    fprintf(stderr, "\n");
  }

  if (rhs_type.dimensions != 0)
  {
    fprintf(stderr, ERROR_LABEL"Cannot operate on expressions of indexable type (");
    print_ast_type(stderr, rhs_type);
    fprintf(stderr, "): ");
    print_ast_exp_binop_node(stderr, node);
    fprintf(stderr, "\n"INFO_LABEL"Expression of type ");
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
        fprintf(stderr, ERROR_LABEL);
        print_ast_type(stderr, lhs_type);
        fprintf(stderr, " is not convertible to int: ");
        print_ast_exp_node(stderr, node->exp1);
        fprintf(stderr, "\n");
        fprintf(stderr, INFO_LABEL"Conversion required by: ");
        print_ast_exp_binop_node(stderr, node);
        fprintf(stderr, ")\n");
        exit(1);
      }

      if (is_convertible_to_int(rhs_type) == FALSE)
      {
        fprintf(stderr, ERROR_LABEL);
        print_ast_type(stderr, rhs_type);
        fprintf(stderr, " is not convertible to int: ");
        print_ast_exp_node(stderr, node->exp2);
        fprintf(stderr, "\n");
        fprintf(stderr, INFO_LABEL"Conversion required by: ");
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
    fprintf(stderr, ERROR_LABEL"Cannot operate on expressions of type void: ");
    print_ast_exp_unop_node(stderr, node);
    fprintf(stderr, "\n"INFO_LABEL"Expression of type void: ");
    print_ast_exp_node(stderr, node->exp);
    fprintf(stderr, "\n");
    exit(1);
  }

  if (exp_type.dimensions != 0)
  {
    fprintf(stderr, ERROR_LABEL"Cannot operate on expressions of indexable type (");
    print_ast_type(stderr, exp_type);
    fprintf(stderr, "): ");
    print_ast_exp_unop_node(stderr, node);
    fprintf(stderr, "\n"INFO_LABEL"Expression of type ");
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
        fprintf(stderr, ERROR_LABEL);
        print_ast_type(stderr, exp_type);
        fprintf(stderr, " is not convertible to int: ");
        print_ast_exp_node(stderr, node->exp);
        fprintf(stderr, "\n");
        fprintf(stderr, INFO_LABEL"Conversion required by: ");
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

return_status resolve_types_if_else(ast_statement_if_else_node *node, ast_type expected_return_type)
{
  return_status if_ret_status   = didnt_return;
  return_status else_ret_status = didnt_return;

  ast_type exp_type = resolve_types_exp(node->exp);
  if (is_convertible_to_int(exp_type) == FALSE)
  {
    fprintf(stderr, ERROR_LABEL);
    print_ast_type(stderr, exp_type);
    fprintf(stderr, " is not convertible to int: ");
    print_ast_exp_node(stderr, node->exp);
    fprintf(stderr, "\n");
    fprintf(stderr, INFO_LABEL"Conversion required by: if (");
    print_ast_exp_node(stderr, node->exp);
    fprintf(stderr, ")\n");
    exit(1);
  }

  if (exp_type.type != ast_int_type)
  {
    exp_type.type = ast_int_type;
    add_type_cast(node->exp, exp_type);
  }

  if_ret_status = resolve_types_statement(node->if_body, expected_return_type);

  if (node->else_body)
  {
    else_ret_status = resolve_types_statement(node->else_body, expected_return_type);
  }

  if (if_ret_status == returned && else_ret_status == returned) { return returned; }
  if (if_ret_status == returned || else_ret_status == returned) { return maybe_returned; }

  return didnt_return;
}

return_status resolve_types_while(ast_statement_while_node *node, ast_type expected_return_type)
{
  ast_type exp_type = resolve_types_exp(node->exp);
  if (is_convertible_to_int(exp_type) == FALSE)
  {
    fprintf(stderr, ERROR_LABEL);
    print_ast_type(stderr, exp_type);
    fprintf(stderr, " is not convertible to int: ");
    print_ast_exp_node(stderr, node->exp);
    fprintf(stderr, "\n");
    fprintf(stderr, INFO_LABEL"Conversion required by: while (");
    print_ast_exp_node(stderr, node->exp);
    fprintf(stderr, ")\n");
    exit(1);
  }

  if (exp_type.type != ast_int_type)
  {
    exp_type.type = ast_int_type;
    add_type_cast(node->exp, exp_type);
  }

  switch (resolve_types_statement(node->statement, expected_return_type))
  {
    case didnt_return:
      return didnt_return;

    case returned: /* Treat as maybe because we don't know if while condition will be met. */
    case maybe_returned:
      return maybe_returned;
  }

  return didnt_return;
}

return_status resolve_types_assign(ast_statement_assign_node *node, ast_type expected_return_type)
{
  ast_type var_type = resolve_types_var(&(node->var));
  ast_type exp_type = resolve_types_exp(node->exp);

  // Cover "operator new" case.
  if (var_type.dimensions != 0 && exp_type.type == ast_int_type) { return didnt_return; }

  if (var_type.type != exp_type.type || var_type.dimensions != exp_type.dimensions)
  {
    fprintf(stderr, ERROR_LABEL"Cannot assign a value of type ");
    print_ast_type(stderr, exp_type);
    fprintf(stderr, " to a variable of type ");
    print_ast_type(stderr, var_type);
    fprintf(stderr, ": ");
    print_ast_statement_assign_node(stderr, node, 0);
    exit(1);
  }

  return didnt_return;
}

return_status resolve_types_return(ast_statement_return_node *node, ast_type expected_return_type)
{
  ast_type return_type;

  if (node->exp)
  {
    return_type = resolve_types_exp(node->exp);
  }
  else
  {
    return_type.type       = ast_void_type;
    return_type.dimensions = 0;
  }

  if (node->exp == NULL && (expected_return_type.type != ast_void_type || expected_return_type.dimensions != 0))
  {
    fprintf(stderr, ERROR_LABEL"Return value must be of type ");
    print_ast_type(stderr, expected_return_type);
    fprintf(stderr, ": ");
    print_ast_statement_return_node(stderr, node, 0);
    exit(1); 
  }

  if (return_type.type != expected_return_type.type || return_type.dimensions != expected_return_type.dimensions)
  {
    fprintf(stderr, ERROR_LABEL"Cannot return a value of type ");
    print_ast_type(stderr, return_type);
    fprintf(stderr, " from a function with return of ");
    print_ast_type(stderr, expected_return_type);
    fprintf(stderr, " type: ");
    print_ast_statement_return_node(stderr, node, 0);
    exit(1); 
  }

  return returned;
}

return_status resolve_types_block(ast_statement_block_node *node, ast_type expected_return_type)
{
  return_status  ret_status = didnt_return;
  ast_decl_node *decl_node;
  for (decl_node = node->decl_vars; decl_node; decl_node = decl_node->next)
  {
    assert(decl_node->tag == decl_var_tag);
    resolve_types_decl_var(&(decl_node->value.decl_var));
  }

  ast_statement_node *statement_node;
  for (statement_node = node->statements; statement_node; statement_node = statement_node->next)
  {
    switch (ret_status)
    {
      case didnt_return:
      case maybe_returned:
        break;
      case returned:
        fprintf(stderr, WARNING_LABEL"Unreachable code: ");
        print_ast_statement_node(stderr, statement_node, 0);
        break;
    }
    ret_status = MAX(ret_status, resolve_types_statement(statement_node, expected_return_type));
  }

  node->ret_status = ret_status;
  return ret_status;
}

return_status resolve_types_statement(ast_statement_node *node, ast_type expected_return_type)
{
  assert(node);

  switch (node->tag)
  {
    case ast_statement_if_else_tag:
      return resolve_types_if_else(&(node->value.if_else_node), expected_return_type);
      break;
    case ast_statement_while_tag:
      return resolve_types_while(&(node->value.while_node), expected_return_type);
      break;
    case ast_statement_assign_tag:
      return resolve_types_assign(&(node->value.assign_node), expected_return_type);
      break;
    case ast_statement_return_tag:
      return resolve_types_return(&(node->value.return_node), expected_return_type);
      break;
    case ast_statement_func_call_tag:
      {
        ast_type return_type = resolve_types_func_call(&(node->value.func_call_node));
        if (return_type.type != ast_void_type)
        {
          fprintf(stderr, WARNING_LABEL"Ignoring return value: ");
          print_ast_statement_func_call_node(stderr, &(node->value.func_call_node), 0);
          fprintf(stderr, "Node: Return value is of type ");
          print_ast_type(stderr, return_type);
          fprintf(stderr, "\n");
        }

        return didnt_return;
      }
      break;
    case ast_statement_block_tag:
      return resolve_types_block(&(node->value.block_node), expected_return_type);
      break;
  }

  return didnt_return;
}
