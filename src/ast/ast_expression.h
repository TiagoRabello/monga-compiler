#ifndef AST_EXPRESSION_H
#define AST_EXPRESSION_H

#include "ast_common.h"
#include "ast_type.h"

// Binary Operator Node
typedef enum {
  add_tag,
  subtract_tag,
  multiply_tag,
  divide_tag,
  equal_tag,
  less_equal_tag,
  greater_equal_tag,
  greater_tag,
  less_tag,
  logical_and_tag,
  logical_or_tag

} ast_exp_binop_tag;

struct _ast_exp_binop_node
{
  ast_exp_binop_tag tag;
  ast_exp_node *exp1;
  ast_exp_node *exp2;
};

// Unary Operator Node
typedef enum
{
  minus_tag,
  logical_not_tag

} ast_exp_unop_tag;

struct _ast_exp_unop_node
{
  ast_exp_unop_tag tag;
  ast_exp_node *exp;
};

// Operator new node
struct _ast_exp_operator_new_node
{
  ast_type      type;
  ast_exp_node *exp;
};

struct _ast_exp_type_cast_node
{
  ast_type      type;
  ast_exp_node *exp;
};

// General Expresion node
typedef enum
{
  binop_tag,
  unop_tag,
  float_literal_tag,
  int_literal_tag,
  string_literal_tag,
  var_tag,
  func_call_tag,
  operator_new_tag,
  type_cast_tag

} ast_exp_tag;

struct _ast_exp_node
{
  ast_exp_tag tag;
  union
  {
    double                    float_literal;
    int                       int_literal;
    const char*               string_literal;
    ast_exp_binop_node        binop_node;
    ast_exp_unop_node         unop_node;
    ast_var_node              var;
    ast_func_call_node        func_call;
    ast_exp_operator_new_node operator_new;
    ast_exp_type_cast_node    type_cast;

  } value;

};

#endif