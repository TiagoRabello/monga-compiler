#include "ast_printer.h"

#include "ast.h"

#include <assert.h>
#include <stdio.h>

static const char *indent_spaces = "                                                                      ";

#define PRINT_INDENTED(file, indent, format, ...) \
  fprintf(file, "%.*s" format, indent, indent_spaces, ##__VA_ARGS__)

void print_ast_type(FILE* out, ast_type type)
{
  static const char *types[] = { "int", "char", "float", "void" };
  int i;

  fprintf(out, "%s", types[type.type]);
  for (i = 0; i < type.dimensions; ++i)
  {
    fprintf(out, "[]");
  }
}

void print_full_ast(FILE *out, const ast_program_node* node, int indent_level)
{
  for (; node; node = node->next) { print_ast_decl_node(out, node, indent_level); }
}

void print_ast_decl_node(FILE *out, const ast_decl_node* node, int indent_level)
{
  switch (node->tag)
  {
    case decl_var_tag:
      print_ast_decl_var_node(out, &node->value.decl_var, indent_level);
      break;
    case decl_func_tag:
      print_ast_decl_func_node(out, &node->value.decl_func, indent_level);
      break;
  }
}

void print_ast_decl_var_node(FILE *out, const ast_decl_var_node *node, int indent_level)
{
  PRINT_INDENTED(out, indent_level, "");
  print_ast_type(out, node->type);
  fprintf(out, " %s;\n", node->name);
}

void print_ast_decl_func_node(FILE *out, const ast_decl_func_node* node, int indent_level)
{
  print_ast_decl_func_signature(out, node, indent_level);

  assert(node->body->tag == ast_statement_block_tag);
  print_ast_statement_block_node(out, &node->body->value.block_node, indent_level);
}

void print_ast_decl_func_signature(FILE *out, const ast_decl_func_node* node, int indent_level)
{
  ast_decl_node *param;
  PRINT_INDENTED(out, indent_level, "");
  print_ast_type(out, node->return_type);
  fprintf(out, " %s(", node->name);
  for (param = node->params; param && param->next; param = param->next)
  {
    assert(param->tag == decl_var_tag);
    print_ast_type(out, param->value.decl_var.type);
    fprintf(out, " %s,", param->value.decl_var.name);
  }
  if (param)
  {
    assert(param->tag == decl_var_tag);
    print_ast_type(out, param->value.decl_var.type);
    fprintf(out, " %s", param->value.decl_var.name);
  }
  fprintf(out, ")\n");
}

void print_ast_statement_node(FILE *out, const ast_statement_node *node, int indent_level)
{
  assert(node);

  switch (node->tag)
  {
    case ast_statement_if_else_tag:
      print_ast_statement_if_else_node(out, &(node->value.if_else_node), indent_level);
      break;
    case ast_statement_while_tag:
      print_ast_statement_while_node(out, &(node->value.while_node), indent_level);
      break;
    case ast_statement_assign_tag:
      print_ast_statement_assign_node(out, &(node->value.assign_node), indent_level);
      break;
    case ast_statement_return_tag:
      print_ast_statement_return_node(out, &(node->value.return_node), indent_level);
      break;
    case ast_statement_func_call_tag:
      print_ast_statement_func_call_node(out, &(node->value.func_call_node), indent_level);
      break;
    case ast_statement_block_tag:
      print_ast_statement_block_node(out, &(node->value.block_node), indent_level);
      break;
  }
}

void print_ast_statement_if_else_node(FILE *out, const ast_statement_if_else_node *node, int indent_level)
{
  PRINT_INDENTED(out, indent_level, "if (");
  print_ast_exp_node(out, node->exp);
  fprintf(out, ")\n");
  print_ast_statement_node(out, node->if_body, indent_level);
  if (node->else_body)
  {
    fprintf(out, "else\n");
    print_ast_statement_node(out, node->else_body, indent_level);
  }
  else
  {
    fprintf(out, "\n");
  }
}

void print_ast_statement_func_call_node(FILE *out, const ast_func_call_node *node, int indent_level)
{
  PRINT_INDENTED(out, indent_level, "");
  print_ast_func_call_node(out, node);
  fprintf(out, ";\n");
}

void print_ast_statement_assign_node(FILE *out, const ast_statement_assign_node *node, int indent_level)
{
  PRINT_INDENTED(out, indent_level, "");
  print_ast_var_node(out, &(node->var));
  fprintf(out, " = ");
  print_ast_exp_node(out, node->exp);
  fprintf(out, ";\n");
}

void print_ast_statement_while_node(FILE *out, const ast_statement_while_node *node, int indent_level)
{
  PRINT_INDENTED(out, indent_level, "while (");
  print_ast_exp_node(out, node->exp);
  fprintf(out, ")\n");
  print_ast_statement_node(out, node->statement, indent_level);
}

void print_ast_statement_return_node(FILE *out, const ast_statement_return_node *node, int indent_level)
{
  if (node->exp)
  {
    PRINT_INDENTED(out, indent_level, "return ");
    print_ast_exp_node(out, node->exp);
    fprintf(out, ";\n");
  }
  else
  {
    PRINT_INDENTED(out, indent_level, "return;\n");
  }
}

void print_ast_statement_block_node(FILE *out, const ast_statement_block_node *node, int indent_level)
{
  PRINT_INDENTED(out, indent_level, "{\n");

  ast_decl_node *decl_node;
  for (decl_node = node->decl_vars; decl_node; decl_node = decl_node->next)
  {
    assert(decl_node->tag == decl_var_tag);
    print_ast_decl_var_node(out, &(decl_node->value.decl_var), indent_level + INDENT_INCREMENT); 
  }

  ast_statement_node *statement_node;
  for (statement_node = node->statements; statement_node; statement_node = statement_node->next)
  {
    print_ast_statement_node(out, statement_node, indent_level + INDENT_INCREMENT); 
  }

  PRINT_INDENTED(out, indent_level, "}\n");
}

void print_ast_exp_node(FILE *out, const ast_exp_node *node)
{
  switch (node->tag)
  {
    case binop_tag:
      print_ast_exp_binop_node(out, &(node->value.binop_node));
      break;
    case unop_tag:
      print_ast_exp_unop_node(out, &(node->value.unop_node));
      break;
    case float_literal_tag:
      fprintf(out, "%f", node->value.float_literal);
      break;
    case int_literal_tag:
      fprintf(out, "%d", node->value.int_literal);
      break;
    case string_literal_tag:
      fprintf(out, "%s", node->value.string_literal);
      break;
    case var_tag:
      print_ast_var_node(out, &(node->value.var));
      break;
    case func_call_tag:
      print_ast_func_call_node(out, &(node->value.func_call));
      break;
    case operator_new_tag:
      print_ast_exp_operator_new_node(out, &(node->value.operator_new));
      break;
    case type_cast_tag:
      print_ast_exp_type_cast_node(out, &(node->value.type_cast));
      break;
  }
}

void print_ast_exp_type_cast_node(FILE *out, const ast_exp_type_cast_node *node)
{
  print_ast_type(out, node->type);
  fprintf(out, "(");
  print_ast_exp_node(out, node->exp);
  fprintf(out, ")");
}

void print_ast_exp_operator_new_node(FILE *out, const ast_exp_operator_new_node *node)
{
  fprintf(out, "new ");
  print_ast_type(out, node->type);
  fprintf(out, "[");
  print_ast_exp_node(out, node->exp);
  fprintf(out, "]");
}

void print_ast_var_node(FILE *out, const ast_var_node *node)
{
  switch (node->tag)
  {
    case non_indexed:
      {
        const char *var_name = (node->value.non_indexed.link_status == not_linked)
                               ? node->value.non_indexed.value.var_name
                               : node->value.non_indexed.value.decl->value.decl_var.name;

        fprintf(out, "%s", var_name);
      }
      break;
    case indexed:
      print_ast_exp_node(out, node->value.indexed.base);
      fprintf(out, "[");
      print_ast_exp_node(out, node->value.indexed.index);
      fprintf(out, "]");
      break;
  }
}

void print_ast_func_call_node(FILE *out, const ast_func_call_node *node)
{
  exp_list_node *param;
  const char *func_name = (node->link_status == not_linked) ? node->value.func_name : node->value.decl->value.decl_func.name;
  fprintf(out, "%s(", func_name);
  for (param = node->params; param && param->next; param = param->next)
  {
    print_ast_exp_node(out, param->exp);
    fprintf(out, ", ");
  }
  if (param)
  {
    print_ast_exp_node(out, param->exp);
  }
  fprintf(out, ")");
}

void print_ast_exp_binop_node(FILE *out, const ast_exp_binop_node *node)
{
  static const char *binops[] = { "+", "-", "*", "/", "==", "<=", ">=", ">", "<", "&&", "||" };

  print_ast_exp_node(out, node->exp1);
  fprintf(out, " %s ", binops[node->tag]);
  print_ast_exp_node(out, node->exp2);
}

void print_ast_exp_unop_node(FILE *out, const ast_exp_unop_node *node)
{
  static const char *unops = "-!";
  
  fprintf(out, "%c", unops[node->tag]);
  print_ast_exp_node(out, node->exp);
}
