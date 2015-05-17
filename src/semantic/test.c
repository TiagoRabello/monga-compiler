#include "semantic.h"

#include "../ast/ast.h"
#include "../ast/ast_printer.h"
#include "../parser/monga_parser.h"

#include <assert.h>

int main()
{
  int status = yyparse();

  assert(status == 0);

  if (ast)
  {
    resolve_ids(ast);
    resolve_types(ast);
    print_full_ast(stdout, ast, 0);
  }

  return status;
}