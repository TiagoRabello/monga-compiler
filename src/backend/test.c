#include "ia32_assembly.h"

#include "../ast/ast.h"
#include "../ast/ast_printer.h"
#include "../parser/monga_parser.h"
#include "../common/diagnostics.h"

#include <assert.h>
#include <stdio.h>

int main()
{
  int status = yyparse();

  assert(status == 0);

  if (ast)
  {
    resolve_ids(ast);
    resolve_types(ast);
    gen_ia32(stdout, ast);
    /*PRINTF_HEADER("Program is " GREEN_LABEL "OK" NORMAL_LABEL ".\n");*/
  }

  return status;
}