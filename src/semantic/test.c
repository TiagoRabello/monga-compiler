#include "semantic.h"

#include "../ast/ast.h"
#include "../parser/monga_parser.h"

#include <assert.h>

int main()
{
  int status = yyparse();

  assert(status == 0);

  if (ast)
  {
    resolve_ids(ast);
  }

  return status;
}