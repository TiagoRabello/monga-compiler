#ifndef IA32_ASSEMBLY_H
#define IA32_ASSEMBLY_H

#include "../ast/ast.h"

#include <stdio.h>

void generate_ia32_assembly(FILE *output, ast_program_node *ast);

#endif