#include "semantic.h"

#include "../ast/ast.h"
#include "../ast/ast_printer.h"
#include "../common/diagnostics.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
  const char    *symbol;
  ast_decl_node *decl;

} symbol_table_cell;

typedef struct
{
  symbol_table_cell *begin;
  symbol_table_cell *end;
  symbol_table_cell *end_capacity;

} symbol_table;

size_t symbol_table_size(const symbol_table *table)
{
  return table->end - table->begin;
}

void symbol_table_take(symbol_table *table, size_t n)
{
  assert(n <= symbol_table_size(table));
  table->end = table->begin + n;
}

void symbol_table_insert(symbol_table *table, const char *symbol, ast_decl_node *decl_node)
{
  if (table->end == table->end_capacity)
  {
    size_t old_capacity = (table->end_capacity - table->begin);
    size_t new_capacity = (old_capacity) ? (old_capacity) * 2 : 10;
    table->begin        = (symbol_table_cell *)realloc(table->begin, new_capacity * sizeof(symbol_table_cell));
    table->end          = table->begin + old_capacity;
    table->end_capacity = table->begin + new_capacity;
  }

  table->end->symbol = symbol;
  table->end->decl   = decl_node;
  ++(table->end);
}

symbol_table_cell * symbol_table_find_iter(symbol_table_cell *first, symbol_table_cell *last, const char *symbol)
{
  symbol_table_cell *end = last;
  while (first != last)
  {
    if (strcmp((last - 1)->symbol, symbol) == 0) { break; }
    --last;
  }

  return (first != last) ? (last - 1) : end;
}

symbol_table_cell * symbol_table_find(symbol_table *table, const char *symbol)
{
  return symbol_table_find_iter(table->begin, table->end, symbol);
}

void report_redeclaration_error(ast_decl_node *prev_decl, ast_decl_node *curr_decl)
{
  switch (curr_decl->tag)
  {
    case decl_var_tag:
      fprintf(stderr, ERROR_LABEL "Redeclaration of variable within same scope: ");
      break;
    case decl_func_tag:
      fprintf(stderr, ERROR_LABEL "Redeclaration of function within same scope: ");
      break;
  }
  print_ast_decl_node(stderr, curr_decl, 0);
  fprintf(stderr, INFO_LABEL "Previously declared as: ");
  print_ast_decl_node(stderr, prev_decl, 0);
  exit(1);
}

void report_shadowing_warning(ast_decl_node *prev_decl, ast_decl_node *curr_decl)
{
  switch (curr_decl->tag)
  {
    case decl_var_tag:
      fprintf(stderr, WARNING_LABEL "Variable declaration hides previous declared on an outter scope: ");
      break;
    case decl_func_tag:
      fprintf(stderr, WARNING_LABEL "Function declaration hides previous declared on an outter scope: ");
      break;
  }
  print_ast_decl_node(stderr, curr_decl, 0);
  fprintf(stderr, INFO_LABEL "Outter declaration: ");
  print_ast_decl_node(stderr, prev_decl, 0);
}

void resolve_ids_exp         (ast_exp_node              *node, symbol_table *table);
void resolve_ids_binop       (ast_exp_binop_node        *node, symbol_table *table);
void resolve_ids_unop        (ast_exp_unop_node         *node, symbol_table *table);
void resolve_ids_operator_new(ast_exp_operator_new_node *node, symbol_table *table);

void resolve_ids_statement(ast_statement_node         *node, symbol_table *table);
void resolve_ids_if_else  (ast_statement_if_else_node *node, symbol_table *table);
void resolve_ids_while    (ast_statement_while_node   *node, symbol_table *table);
void resolve_ids_assign   (ast_statement_assign_node  *node, symbol_table *table);
void resolve_ids_return   (ast_statement_return_node  *node, symbol_table *table);
void resolve_ids_func_call(ast_func_call_node         *node, symbol_table *table);
void resolve_ids_block    (ast_statement_block_node   *node, symbol_table *table);
void resolve_ids_statement(ast_statement_node         *node, symbol_table *table);

void resolve_ids_func     (ast_decl_func_node *node, symbol_table *table);
void resolve_ids_var      (ast_var_node       *node, symbol_table *table);

void resolve_ids(ast_program_node *ast)
{
  symbol_table_cell *itr;
  symbol_table       table = { NULL, NULL, NULL };
  ast_decl_node     *node  = ast;

  assert(ast);

  for (; node; node = node->next)
  {
    switch (node->tag)
    {
      case decl_var_tag:
        itr = symbol_table_find(&table, node->value.decl_var.name);
        if (itr != table.end) { report_redeclaration_error(itr->decl, node); }

        symbol_table_insert(&table, node->value.decl_var.name, node);
        break;
      case decl_func_tag:
        itr = symbol_table_find(&table, node->value.decl_func.name);
        if (itr != table.end) { report_redeclaration_error(itr->decl, node); }

        symbol_table_insert(&table, node->value.decl_func.name, node);
        resolve_ids_func(&(node->value.decl_func), &table);
        break;
    }
  }
}

void resolve_ids_binop(ast_exp_binop_node *node, symbol_table *table)
{
  resolve_ids_exp(node->exp1, table);
  resolve_ids_exp(node->exp2, table);
}

void resolve_ids_unop(ast_exp_unop_node *node, symbol_table *table)
{
  resolve_ids_exp(node->exp, table);
}

void resolve_ids_operator_new(ast_exp_operator_new_node *node, symbol_table *table)
{
  resolve_ids_exp(node->exp, table);
}

void resolve_ids_exp(ast_exp_node *node, symbol_table *table)
{
  switch (node->tag)
  {
    case binop_tag:
      resolve_ids_binop(&(node->value.binop_node), table);
      break;
    case unop_tag:
      resolve_ids_unop(&(node->value.unop_node), table);
      break;
    case float_literal_tag:
      // No linking required;
      break;
    case int_literal_tag:
      // No linking required;
      break;
    case string_literal_tag:
      // No linking required;
      break;
    case var_tag:
      resolve_ids_var(&(node->value.var), table);
      break;
    case func_call_tag:
      resolve_ids_func_call(&(node->value.func_call), table);
      break;
    case operator_new_tag:
      resolve_ids_operator_new(&(node->value.operator_new), table);
      break;
  }
}

void resolve_ids_if_else(ast_statement_if_else_node *node, symbol_table *table)
{
  resolve_ids_exp(node->exp, table);
  resolve_ids_statement(node->if_body, table);
  if (node->else_body) { resolve_ids_statement(node->else_body, table); }
}

void resolve_ids_while(ast_statement_while_node *node, symbol_table *table)
{
  resolve_ids_exp(node->exp, table);
  resolve_ids_statement(node->statement, table);
}

void resolve_ids_assign(ast_statement_assign_node *node, symbol_table *table)
{
  resolve_ids_var(&(node->var), table);
  resolve_ids_exp(node->exp, table);
}

void resolve_ids_return(ast_statement_return_node *node, symbol_table *table)
{
  if (node->exp) { resolve_ids_exp(node->exp, table); }
}

void resolve_ids_func_call(ast_func_call_node *node, symbol_table *table)
{
  symbol_table_cell *cell = symbol_table_find(table, node->value.func_name);
  if (cell != table->end)
  {
    node->link_status = linked;
    node->value.decl  = cell->decl;
  }
  else
  {
    fprintf(stderr, ERROR_LABEL"Call to undefined function: ");
    print_ast_func_call_node(stderr, node);
    fprintf(stderr, "\n");
    exit(1);
  } 

  exp_list_node *param;
  for (param = node->params; param; param = param->next)
  {
    resolve_ids_exp(param->exp, table);
  }
}

void resolve_ids_block(ast_statement_block_node *node, symbol_table *table)
{
  size_t original_size = symbol_table_size(table);

  ast_decl_node *decl_node;
  for (decl_node = node->decl_vars; decl_node; decl_node = decl_node->next)
  {
    assert(node->decl_vars->tag == decl_var_tag);

    symbol_table_cell *first = table->begin + original_size;
    symbol_table_cell *last  = table->end;
    symbol_table_cell *itr   = symbol_table_find_iter(first, last, decl_node->value.decl_var.name);
    if (itr != last) { report_redeclaration_error(itr->decl, decl_node); }

    first = table->begin;
    last  = table->begin + original_size;
    itr   = symbol_table_find_iter(first, last, decl_node->value.decl_var.name);
    if (itr != last) { report_shadowing_warning(itr->decl, decl_node); }

    symbol_table_insert(table, decl_node->value.decl_var.name, decl_node);
  }

  ast_statement_node *statement_node;
  for (statement_node = node->statements; statement_node; statement_node = statement_node->next)
  {
    resolve_ids_statement(statement_node, table);
  }

  symbol_table_take(table, original_size);
}

void resolve_ids_statement(ast_statement_node *node, symbol_table *table)
{
  switch (node->tag)
  {
    case ast_statement_if_else_tag:
      resolve_ids_if_else(&(node->value.if_else_node), table);
      break;
    case ast_statement_while_tag:
      resolve_ids_while(&(node->value.while_node), table);
      break;
    case ast_statement_assign_tag:
      resolve_ids_assign(&(node->value.assign_node), table);
      break;
    case ast_statement_return_tag:
      resolve_ids_return(&(node->value.return_node), table);
      break;
    case ast_statement_func_call_tag:
      resolve_ids_func_call(&(node->value.func_call_node), table);
      break;
    case ast_statement_block_tag:
      resolve_ids_block(&(node->value.block_node), table);
      break;
  }
}

void resolve_ids_func(ast_decl_func_node *node, symbol_table *table)
{
  size_t original_size = symbol_table_size(table);

  ast_decl_node *param;
  for (param = node->params; param; param = param->next)
  {
    assert(param->tag == decl_var_tag);

    symbol_table_cell *first = table->begin + original_size;
    symbol_table_cell *last  = table->end;
    symbol_table_cell *itr   = symbol_table_find_iter(first, last, param->value.decl_var.name);
    if (itr != last) { report_redeclaration_error(itr->decl, param); }

    first = table->begin;
    last  = table->begin + original_size;
    itr   = symbol_table_find_iter(first, last, param->value.decl_var.name);
    if (itr != last) { report_shadowing_warning(itr->decl, param); }

    symbol_table_insert(table, param->value.decl_var.name, param);
  }

  resolve_ids_block(&(node->body->value.block_node), table);

  symbol_table_take(table, original_size);
}

void resolve_ids_var(ast_var_node *node, symbol_table *table)
{
  switch (node->tag)
  {
    case non_indexed:
      {
        symbol_table_cell *cell = symbol_table_find(table, node->value.non_indexed.value.var_name);
        if (cell != table->end)
        {
          node->value.non_indexed.link_status = linked;
          node->value.non_indexed.value.decl  = cell->decl;
        }
        else
        {
          fprintf(stderr, ERROR_LABEL"Use of undefined variable: ");
          print_ast_var_node(stderr, node);
          fprintf(stderr, "\n");
          exit(1);
        } 
      }
      break;
    case indexed:
      resolve_ids_exp(node->value.indexed.base, table);
      resolve_ids_exp(node->value.indexed.index, table);
      break;
  }
}
