#ifndef MONGA_PARSER_H
#define MONGA_PARSER_H

#include "../ast/ast.h"

int yylex(void);
int yyparse(void);

extern int               yylineno;
extern ast_program_node *ast;

#endif