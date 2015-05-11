#ifndef AST_TYPE_H
#define AST_TYPE_H

typedef struct _ast_type ast_type;

typedef enum
{
  ast_int_type,
  ast_char_type,
  ast_float_type,
  ast_void_type
  
} ast_basic_type;

struct _ast_type
{
  ast_basic_type type;
  int            dimensions;
};

#endif