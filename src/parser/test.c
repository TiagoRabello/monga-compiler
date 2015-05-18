#include "monga_parser.h"

#include "../ast/ast.h"
#include "../ast/ast_printer.h"

int main()
{
  int status = yyparse();

  if (status == 0)
  {
    print_full_ast(stdout, ast, 0);
  }

  return status;
}