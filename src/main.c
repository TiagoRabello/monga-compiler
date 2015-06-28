#include "ast/ast.h"
#include "backend/ia32_assembly.h"
#include "parser/monga_parser.h"

#include <stdio.h>

int main()
{
  int status = yyparse();

  if (status == 0 && ast)
  {
    resolve_ids(ast);
    resolve_types(ast);
    gen_ia32(stdout, ast);
  }

  return status;
}