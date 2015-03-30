#ifndef _MONGA_DECL_H
#define _MONGA_DECL_H

#include <limits.h>

enum {
  TK_ID = UCHAR_MAX + 1,
  TK_EQ,
  TK_LEQ,
  TK_GEQ,
  TK_AND,
  TK_OR,
  TK_COMMENT,
  TK_CHAR_T,
  TK_FLOAT_T,
  TK_INT_T,
  TK_VOID_T,
  TK_IF,
  TK_ELSE,
  TK_WHILE,
  TK_NEW,
  TK_RETURN,
  TK_FLOAT_LITERAL,
  TK_INT_LITERAL,
  TK_STRING_LITERAL,
  TK_UNKNOWN
};

union {
  int    int_val;
  double float_val;
  char * string_val;
  char   char_val;
} yyval;

#endif