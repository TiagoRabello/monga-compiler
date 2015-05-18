#include "semantic.h"

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
    PRINTF_HEADER("Program is " GREEN_LABEL "OK" NORMAL_LABEL ".\nPrinting a reconstruction of your program from processed AST...");
    print_full_ast(stdout, ast, 2);
  }

  return status;
}