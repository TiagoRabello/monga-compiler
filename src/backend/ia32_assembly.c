#include "ia32_assembly.h"

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

static int generate_unique_id(void);

static int generate_unique_id(void)
{
  static int id = 0;
  return id++;
}

static void generate_ia32_assembly_decl     (FILE *output, ast_decl_node      *node);
static void generate_ia32_assembly_decl_var (FILE *output, ast_decl_var_node  *node);
static void generate_ia32_assembly_decl_func(FILE *output, ast_decl_func_node *node);

static void generate_ia32_assembly_exp      (FILE *output, ast_exp_node       *node);
static void generate_ia32_assembly_func_call(FILE *output, ast_func_call_node *node);
static void generate_ia32_assembly_var      (FILE *output, ast_var_node       *node);
static void generate_ia32_assembly_binop    (FILE *output, ast_exp_binop_node *node);
static void generate_ia32_assembly_unop     (FILE *output, ast_exp_unop_node  *node);

static void generate_ia32_assembly_statement(FILE *output, ast_statement_node         *node);
static void generate_ia32_assembly_if_else  (FILE *output, ast_statement_if_else_node *node);
static void generate_ia32_assembly_while    (FILE *output, ast_statement_while_node   *node);
static void generate_ia32_assembly_assign   (FILE *output, ast_statement_assign_node  *node);
static void generate_ia32_assembly_return   (FILE *output, ast_statement_return_node  *node);
static void generate_ia32_assembly_block    (FILE *output, ast_statement_block_node   *node);

static int  generate_ia32_assembly_func_call_params(FILE *output, exp_list_node *node);
static void generate_ia32_assembly_return_noexp    (FILE *output);
static void generate_ia32_assembly_push_eax        (FILE *output);

static int generate_ia32_assembly_func_call_params(FILE *output, exp_list_node *node)
{
  if (node == NULL) { return 0; }

  const int num_params = generate_ia32_assembly_func_call_params(output, node->next);
  generate_ia32_assembly_exp(output, node->exp);
  generate_ia32_assembly_push_eax(output);

  return num_params + 1;
}

static void generate_ia32_assembly_return_noexp(FILE *output)
{
  fprintf(output, "movl  %%ebp, %%esp\n"
                  "popl  %%ebp\n"
                  "ret\n");
}

void generate_ia32_assembly(FILE *output, ast_program_node *ast)
{
  assert(ast);
  fprintf(output, "\n");

  ast_decl_node *node = ast;
  for (; node; node = node->next)
  {
    generate_ia32_assembly_decl(output, node);
    fprintf(output, "\n");
  }
}

static void generate_ia32_assembly_push_eax(FILE *output)
{
  fprintf(output, "pushl %%eax\n");
}

static void generate_ia32_assembly_decl(FILE *output, ast_decl_node *node)
{
  switch (node->tag)
  {
    case decl_var_tag:
      generate_ia32_assembly_decl_var(output, &(node->value.decl_var));
      break;
    case decl_func_tag:
      generate_ia32_assembly_decl_func(output, &(node->value.decl_func));
      break;
  }
}

static void generate_ia32_assembly_decl_var(FILE *output, ast_decl_var_node *node)
{
}

static void generate_ia32_assembly_decl_func(FILE *output, ast_decl_func_node *node)
{
  fprintf(output, ".globl %s\n", node->name);
  fprintf(output, "%s:\n", node->name);
  fprintf(output, "pushl %%ebp\n"
                  "movl  %%esp, %%ebp\n");

  ast_decl_node *param;
  for (param = node->params; param; param = param->next)
  {
    generate_ia32_assembly_decl_var(output, &(param->value.decl_var));
  }

  generate_ia32_assembly_statement(output, node->body);
}

static void generate_ia32_assembly_exp(FILE *output, ast_exp_node *node)
{
  switch (node->tag)
  {
    case binop_tag:
      generate_ia32_assembly_binop(output, &(node->value.binop_node));
      break;
    case unop_tag:
      generate_ia32_assembly_unop(output, &(node->value.unop_node));
      break;
    case float_literal_tag:
      break;
    case int_literal_tag:
      break;
    case string_literal_tag:
      break;
    case var_tag:
      generate_ia32_assembly_var(output, &(node->value.var));
      break;
    case func_call_tag:
      generate_ia32_assembly_func_call(output, &(node->value.func_call));
      break;
    case operator_new_tag:
      break;
  }
}

static void generate_ia32_assembly_func_call(FILE *output, ast_func_call_node *node)
{
  int num_params = generate_ia32_assembly_func_call_params(output, node->params);

  fprintf(output, "call  %s\n", node->value.decl->value.decl_func.name);
  fprintf(output, "addl  $%d, %%esp\n", num_params * 4);
}

static void generate_ia32_assembly_var(FILE *output, ast_var_node *node)
{
  switch (node->tag)
  {
    case indexed:
      generate_ia32_assembly_exp(output, node->value.indexed.index);
      generate_ia32_assembly_push_eax(output);
      generate_ia32_assembly_exp(output, node->value.indexed.base);
      break;
    case non_indexed:
      break;
  }
}

static void generate_ia32_assembly_binop(FILE *output, ast_exp_binop_node *node)
{
  /* Avalia na ordem inversa para facilitar algumas operacoes, como subtracao */
  generate_ia32_assembly_exp(output, node->exp2);
  generate_ia32_assembly_push_eax(output);
  generate_ia32_assembly_exp(output, node->exp1);
  fprintf(output, "popl  %%ecx\n");

  switch (node->tag)
  {
    case add_tag:
      fprintf(output, "addl  %%ecx, %%eax\n");
      break;
    case subtract_tag:
      fprintf(output, "subl  %%ecx, %%eax\n");
      break;
    case multiply_tag:
      fprintf(output, "imull %%ecx, %%eax\n");
      break;
    case divide_tag:
      fprintf(output, "idivl %%ecx, %%eax\n");
      break;

    case equal_tag:
      break;
    case less_equal_tag:
      break;
    case greater_equal_tag:
      break;
    case greater_tag:
      break;
    case less_tag:
      break;

    case logical_and_tag:
      fprintf(output, "andl  %%ecx, %%eax\n");
      break;
    case logical_or_tag:
      fprintf(output, "orl   %%ecx, %%eax\n");
      break;
  }
}

static void generate_ia32_assembly_unop(FILE *output, ast_exp_unop_node *node)
{
  generate_ia32_assembly_exp(output, node->exp);

  switch (node->tag)
  {
    case minus_tag:
      fprintf(output, "negl  %%eax\n");
      break;
    case logical_not_tag:
      fprintf(output, "notl  %%eax\n");
      break;
  }
}

static void generate_ia32_assembly_if_else(FILE *output, ast_statement_if_else_node *node)
{
  generate_ia32_assembly_exp(output, node->exp);

  generate_ia32_assembly_statement(output, node->if_body);

  if (node->else_body)
  {
    generate_ia32_assembly_statement(output, node->else_body);
  }
}

static void generate_ia32_assembly_while(FILE *output, ast_statement_while_node *node)
{
  generate_ia32_assembly_exp(output, node->exp);
  generate_ia32_assembly_statement(output, node->statement);
}

static void generate_ia32_assembly_assign(FILE *output, ast_statement_assign_node *node)
{
  generate_ia32_assembly_var(output, &(node->var));
  generate_ia32_assembly_exp(output, node->exp);
}

static void generate_ia32_assembly_return(FILE *output, ast_statement_return_node *node)
{
  if (node->exp)
  {
    generate_ia32_assembly_exp(output, node->exp);
  }

  generate_ia32_assembly_return_noexp(output);
}

static void generate_ia32_assembly_block(FILE *output, ast_statement_block_node *node)
{
  ast_decl_node *decl_node;
  for (decl_node = node->decl_vars; decl_node; decl_node = decl_node->next)
  {
    generate_ia32_assembly_decl_var(output, &(decl_node->value.decl_var));
  }

  ast_statement_node *statement_node;
  for (statement_node = node->statements; statement_node; statement_node = statement_node->next)
  {
    generate_ia32_assembly_statement(output, statement_node);
  }
}

static void generate_ia32_assembly_statement(FILE *output, ast_statement_node *node)
{
  assert(node);

  switch (node->tag)
  {
    case ast_statement_if_else_tag:
      generate_ia32_assembly_if_else(output, &(node->value.if_else_node));
      break;
    case ast_statement_while_tag:
      generate_ia32_assembly_while(output, &(node->value.while_node));
      break;
    case ast_statement_assign_tag:
      generate_ia32_assembly_assign(output, &(node->value.assign_node));
      break;
    case ast_statement_return_tag:
      generate_ia32_assembly_return(output, &(node->value.return_node));
      break;
    case ast_statement_func_call_tag:
      generate_ia32_assembly_func_call(output, &(node->value.func_call_node));
      break;
    case ast_statement_block_tag:
      generate_ia32_assembly_block(output, &(node->value.block_node));
      break;
  }
}
