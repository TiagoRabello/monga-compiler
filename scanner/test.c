#include <stdio.h>

extern int yylex();

int main()
{
  while(yylex());
  return 0;
}