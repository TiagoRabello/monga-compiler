#include "monga_parser.h"

#include "../ast/ast.h"
#include "../ast/ast_printer.h"
#include "../common/diagnostics.h"

#include <stdio.h>

int main()
{
  int status = yyparse();

  if (status == 0)
  {
    PRINTF_HEADER("Program is " GREEN_LABEL "OK" NORMAL_LABEL ".\nPrinting a reconstruction of your program from AST...");
    print_full_ast(stdout, ast, 2);
  }

  return status;
}