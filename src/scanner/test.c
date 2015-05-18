#include <stdio.h>

#include "../common/diagnostics.h"

extern int yylex();

int main()
{
  PRINTF_HEADER("Tokenizing and printing input...");
  while(yylex());
  printf("\n");
  PRINTF_HEADER("Program is " GREEN_LABEL "OK" NORMAL_LABEL ".");
  return 0;
}