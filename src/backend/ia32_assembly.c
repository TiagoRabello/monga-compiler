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

static void gen_unique_label      (char *label, const char *prefix);
static int  calc_offset_local_vars(ast_decl_node *node, int initial_offset);
static int  size_of               (ast_basic_type type);
static int  size_of_power2        (ast_basic_type type);

static void gen_unique_label(char *label, const char *prefix)
{
  static int id = 0;
  sprintf(label, "%s%d", prefix, id++);
}

static int calc_offset_local_vars(ast_decl_node *node, int initial_offset)
{
  for (; node; node = node->next)
  {
    node->value.decl_var.asm_offset = -initial_offset - 4;
    initial_offset += 4;
  }
  return initial_offset;
}

static int size_of(ast_basic_type type)
{
  if (type == ast_char_type) { return 1; }
  return 4;
}

static int size_of_power2(ast_basic_type type)
{
  if (type == ast_char_type) { return 0; }
  return 2;
}

static void gen_ia32_decl     (FILE *output, ast_decl_node      *node);
static void gen_ia32_decl_var (FILE *output, ast_decl_var_node  *node);
static void gen_ia32_decl_func(FILE *output, ast_decl_func_node *node);

static void gen_ia32_exp      (FILE *output, ast_exp_node       *node);
static void gen_ia32_func_call(FILE *output, ast_func_call_node *node);
static void gen_ia32_var      (FILE *output, ast_var_node       *node);
static void gen_ia32_binop    (FILE *output, ast_exp_binop_node *node);
static void gen_ia32_unop     (FILE *output, ast_exp_unop_node  *node);

static void gen_ia32_statement(FILE *output, ast_statement_node         *node, int initial_offset);
static void gen_ia32_if_else  (FILE *output, ast_statement_if_else_node *node, int initial_offset);
static void gen_ia32_while    (FILE *output, ast_statement_while_node   *node, int initial_offset);
static void gen_ia32_assign   (FILE *output, ast_statement_assign_node  *node);
static void gen_ia32_return   (FILE *output, ast_statement_return_node  *node);
static void gen_ia32_block    (FILE *output, ast_statement_block_node   *node, int initial_offset);

static int  gen_ia32_func_call_params(FILE *output, exp_list_node *node);
static void gen_ia32_return_noexp    (FILE *output);
static void gen_ia32_push_eax        (FILE *output);
static void gen_ia32_pop_reg         (FILE *output, const char *reg);

static int gen_ia32_func_call_params(FILE *output, exp_list_node *node)
{
  if (node == NULL) { return 0; }

  const int num_params = gen_ia32_func_call_params(output, node->next);
  gen_ia32_exp(output, node->exp);
  gen_ia32_push_eax(output);

  return num_params + 1;
}

static void gen_ia32_return_noexp(FILE *output)
{
  fprintf(output, "    movl  %%ebp, %%esp\n"
                  "    popl  %%ebp\n"
                  "    ret\n");
}

static void gen_ia32_push_eax(FILE *output)
{
  fprintf(output, "    pushl %%eax\n");
}

static void gen_ia32_pop_reg(FILE *output, const char *reg)
{
  fprintf(output, "    popl  %%%s\n", reg);
}

void gen_ia32(FILE *output, ast_program_node *ast)
{
  assert(ast);
  fprintf(output, "\n.text\n");

  ast_decl_node *node = ast;
  for (; node; node = node->next)
  {
    gen_ia32_decl(output, node);
    fprintf(output, "\n");
  }
}

static void gen_ia32_decl(FILE *output, ast_decl_node *node)
{
  switch (node->tag)
  {
    case decl_var_tag:
      gen_ia32_decl_var(output, &(node->value.decl_var));
      break;
    case decl_func_tag:
      gen_ia32_decl_func(output, &(node->value.decl_func));
      break;
  }
}

static void gen_ia32_decl_var(FILE *output, ast_decl_var_node *node)
{
}

static void gen_ia32_decl_func(FILE *output, ast_decl_func_node *node)
{
  int offset = 8;
  ast_decl_node *param;

  fprintf(output, ".globl %s\n", node->name);
  fprintf(output, "%s:\n", node->name);
  fprintf(output, "    pushl %%ebp\n"
                  "    movl  %%esp, %%ebp\n");

  for (param = node->params; param; param = param->next)
  {
    param->value.decl_var.asm_offset = offset;
    offset += 4;
  }

  gen_ia32_statement(output, node->body, 0);

  if (node->body->value.block_node.ret_status != returned)
  {
    gen_ia32_return_noexp(output);
  }
}

static void gen_ia32_exp(FILE *output, ast_exp_node *node)
{
  switch (node->tag)
  {
    case binop_tag:
      gen_ia32_binop(output, &(node->value.binop_node));
      break;
    case unop_tag:
      gen_ia32_unop(output, &(node->value.unop_node));
      break;
    case float_literal_tag:
      {
        fprintf(output, "    pushl $0x%x\n", *(unsigned int*)&(node->value.float_literal));
        fprintf(output, "    flds  (%%esp)\n");
        fprintf(output, "    addl  $4, %%esp\n"); 
      }
      break;
    case int_literal_tag:
      fprintf(output, "    movl  $%d, %%eax\n", node->value.int_literal);
      break;
    case string_literal_tag:
      /* TODO */
      break;
    case var_tag:
      gen_ia32_var(output, &(node->value.var));
      fprintf(output, "    movl  (%%eax), %%eax\n");
      break;
    case func_call_tag:
      gen_ia32_func_call(output, &(node->value.func_call));
      break;
    case operator_new_tag:
      gen_ia32_exp(output, node->value.operator_new.exp);
      fprintf(output, "    shl   $%d, %%eax\n", size_of_power2(node->value.operator_new.type.type));
      gen_ia32_push_eax(output);
      fprintf(output, "    call  malloc\n");
      fprintf(output, "    addl  $4, %%esp\n");
      break;
    case type_cast_tag:
      /* TODO */
      switch (node->value.type_cast.type.type)
      {
        case ast_int_type:
          switch (node->value.type_cast.exp->type.type)
          {
            case ast_int_type:
              /* NO OP */
              break;
            case ast_char_type:
              fprintf(output, "    movzbl %%al, %%eax\n");
              break;
            case ast_float_type:
              break;
            case ast_void_type:
              assert(FALSE);
              break;
          }
          break;
        case ast_char_type:
          switch (node->value.type_cast.exp->type.type)
          {
            case ast_int_type:
            case ast_char_type:
              /* NO OP */
              break;
            case ast_float_type:
              break;
            case ast_void_type:
              assert(FALSE);
              break;
          }
          break;
        case ast_float_type:
          switch (node->value.type_cast.exp->type.type)
          {
            case ast_int_type:
              break;
            case ast_char_type:
              break;
            case ast_float_type:
              /* NO OP */
              break;
            case ast_void_type:
              assert(FALSE);
              break;
          }
          break;
        case ast_void_type:
          assert(FALSE);
          break;
      }
      break;
  }
}

static void gen_ia32_func_call(FILE *output, ast_func_call_node *node)
{
  int num_params = gen_ia32_func_call_params(output, node->params);

  fprintf(output, "    call  %s\n", node->value.decl->value.decl_func.name);
  fprintf(output, "    addl  $%d, %%esp\n", num_params * 4);
}

static void gen_ia32_var(FILE *output, ast_var_node *node)
{
  switch (node->tag)
  {
    case indexed:
      gen_ia32_exp(output, node->value.indexed.index);
      gen_ia32_push_eax(output);
      gen_ia32_exp(output, node->value.indexed.base);
      fprintf(output, "    popl  %%ecx\n");
      fprintf(output, "    lea   (%%eax, %%ecx, 4), %%eax\n");
      break;
    case non_indexed:
      fprintf(output, "    lea   %d(%%ebp), %%eax\n", 
              node->value.non_indexed.value.decl->value.decl_var.asm_offset);
      break;
  }
}

static void gen_ia32_binop(FILE *output, ast_exp_binop_node *node)
{
  /* Avalia na ordem inversa para facilitar algumas operacoes, como subtracao */
  gen_ia32_exp(output, node->exp2);
  gen_ia32_push_eax(output);
  gen_ia32_exp(output, node->exp1);
  gen_ia32_pop_reg(output, "ecx");

  if (node->tag == equal_tag ||
      node->tag == less_equal_tag ||
      node->tag == greater_equal_tag ||
      node->tag == greater_tag ||
      node->tag == less_tag)
  {
    fprintf(output, "    cmp   %%ecx, %%eax\n");
    switch (node->tag)
    {
      case equal_tag:
        fprintf(output, "    sete  %%al\n");
        break;
      case less_equal_tag:
        fprintf(output, "    setle %%al\n");
        break;
      case greater_equal_tag:
        fprintf(output, "    setge %%al\n");
        break;
      case greater_tag:
        fprintf(output, "    setg  %%al\n");
        break;
      case less_tag:
        fprintf(output, "    setl  %%al\n");
        break;
    }
    fprintf(output, "    movzbl %%al, %%eax\n");
  }
  else if (node->tag == logical_and_tag ||
           node->tag == logical_or_tag)
  {
    const char *cmd = NULL;
    switch (node->tag)
    {
      /* TODO: Short Circuit and FIX */
      case logical_and_tag:
        cmd = "andl ";
        break;
      case logical_or_tag:
        cmd = "orl  ";
        break;
    }
    fprintf(output, "    %s %%ecx, %%eax\n", cmd);
  }
  else
  {
    const char *cmd = NULL;
    switch (node->tag)
    {
      case add_tag:
        cmd = "addl ";
        break;
      case subtract_tag:
        cmd = "subl ";
        break;
      case multiply_tag:
        cmd = "imull";
        break;

      case divide_tag:
        fprintf(output, "    idivl %%ecx\n", cmd);
        return;
    }
    fprintf(output, "    %s %%ecx, %%eax\n", cmd);
  }
}

static void gen_ia32_unop(FILE *output, ast_exp_unop_node *node)
{
  gen_ia32_exp(output, node->exp);

  switch (node->tag)
  {
    case minus_tag:
      fprintf(output, "    negl  %%eax\n");
      break;
    case logical_not_tag:
      fprintf(output, "    notl  %%eax\n");
      break;
  }
}

static void gen_ia32_if_else(FILE *output, ast_statement_if_else_node *node, int initial_offset)
{
  char else_end_label[512];
  char test_fail_label[512];
  gen_unique_label(test_fail_label, "if_end");

  gen_ia32_exp(output, node->exp);
  fprintf(output, "    test  %%eax, %%eax\n");
  fprintf(output, "    jz    %s\n", test_fail_label);

  gen_ia32_statement(output, node->if_body, initial_offset);

  if (node->else_body)
  {
    gen_unique_label(else_end_label, "else_end");
    fprintf(output, "    jmp   %s\n", else_end_label);
  }

  fprintf(output, "%s:\n", test_fail_label);

  if (node->else_body)
  {
    gen_ia32_statement(output, node->else_body, initial_offset);
    fprintf(output, "%s:\n", else_end_label);
  }
}

static void gen_ia32_while(FILE *output, ast_statement_while_node *node, int initial_offset)
{
  char while_begin_label[512];
  char while_end_label[512];

  gen_unique_label(while_begin_label, "while_begin");
  gen_unique_label(while_end_label, "while_end");

  fprintf(output, "%s:\n", while_begin_label);
  gen_ia32_exp(output, node->exp);
  fprintf(output, "    test  %%eax, %%eax\n");
  fprintf(output, "    jz    %s\n", while_end_label);
  gen_ia32_statement(output, node->statement, initial_offset);
  fprintf(output, "    jmp   %s\n", while_begin_label);
  fprintf(output, "%s:\n", while_end_label);
}

static void gen_ia32_assign(FILE *output, ast_statement_assign_node *node)
{
  fprintf(output, "# ");
  print_ast_statement_assign_node(output, node, 0);
  gen_ia32_var(output, &(node->var));
  gen_ia32_push_eax(output);
  gen_ia32_exp(output, node->exp);
  gen_ia32_pop_reg(output, "ecx");
  fprintf(output, "    movl  %%eax, (%%ecx)\n");
}

static void gen_ia32_return(FILE *output, ast_statement_return_node *node)
{
  if (node->exp)
  {
    gen_ia32_exp(output, node->exp);
  }

  gen_ia32_return_noexp(output);
}

static void gen_ia32_block(FILE *output, ast_statement_block_node *node, int initial_offset)
{
  int var_space = calc_offset_local_vars(node->decl_vars, initial_offset);
  if (var_space > 0)
  {
    fprintf(output, "    subl  $%d, %%esp\n", var_space); 
  }

  ast_statement_node *statement_node = node->statements;
  for (; statement_node; statement_node = statement_node->next)
  {
    gen_ia32_statement(output, statement_node, var_space);
  }

  if (node->ret_status != returned)
  {
    fprintf(output, "    addl  $%d, %%esp\n", var_space); 
  }
}

static void gen_ia32_statement(FILE *output, ast_statement_node *node, int initial_offset)
{
  assert(node);

  switch (node->tag)
  {
    case ast_statement_if_else_tag:
      gen_ia32_if_else(output, &(node->value.if_else_node), initial_offset);
      break;
    case ast_statement_while_tag:
      gen_ia32_while(output, &(node->value.while_node), initial_offset);
      break;
    case ast_statement_assign_tag:
      gen_ia32_assign(output, &(node->value.assign_node));
      break;
    case ast_statement_return_tag:
      gen_ia32_return(output, &(node->value.return_node));
      break;
    case ast_statement_func_call_tag:
      gen_ia32_func_call(output, &(node->value.func_call_node));
      break;
    case ast_statement_block_tag:
      gen_ia32_block(output, &(node->value.block_node), initial_offset);
      break;
  }
}
