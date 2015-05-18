#ifndef AST_PRINTER_H
#define AST_PRINTER_H

#include "ast.h"

#include <stdio.h>

#ifndef INDENT_INCREMENT
#define INDENT_INCREMENT 2
#endif

void print_full_ast(FILE *out, const ast_program_node* node, int indent_level);

void print_ast_type(FILE *out, ast_type type);

void print_ast_func_call_node(FILE *out, const ast_func_call_node *node);
void print_ast_var_node      (FILE *out, const ast_var_node       *node);

void print_ast_decl_node          (FILE *out, const ast_decl_node      *node, int indent_level);
void print_ast_decl_var_node      (FILE *out, const ast_decl_var_node  *node, int indent_level);
void print_ast_decl_func_node     (FILE *out, const ast_decl_func_node *node, int indent_level);
void print_ast_decl_func_signature(FILE *out, const ast_decl_func_node *node, int indent_level);

void print_ast_statement_node          (FILE *out, const ast_statement_node         *node, int indent_level);
void print_ast_statement_if_else_node  (FILE *out, const ast_statement_if_else_node *node, int indent_level);
void print_ast_statement_func_call_node(FILE *out, const ast_func_call_node         *node, int indent_level);
void print_ast_statement_assign_node   (FILE *out, const ast_statement_assign_node  *node, int indent_level);
void print_ast_statement_while_node    (FILE *out, const ast_statement_while_node   *node, int indent_level);
void print_ast_statement_return_node   (FILE *out, const ast_statement_return_node  *node, int indent_level);
void print_ast_statement_block_node    (FILE *out, const ast_statement_block_node   *node, int indent_level);

void print_ast_exp_node             (FILE *out, const ast_exp_node              *node);
void print_ast_exp_operator_new_node(FILE *out, const ast_exp_operator_new_node *node);
void print_ast_exp_type_cast_node   (FILE *out, const ast_exp_type_cast_node    *node);
void print_ast_exp_binop_node       (FILE *out, const ast_exp_binop_node        *node);
void print_ast_exp_unop_node        (FILE *out, const ast_exp_unop_node         *node);

#endif