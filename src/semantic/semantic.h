#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "../ast/ast.h"

void resolve_ids(ast_program_node *ast);
void resolve_types(ast_program_node *ast);

#endif