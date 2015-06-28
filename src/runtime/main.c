#include <stdio.h>

/* Monga program entry point. */
extern int monga_main(void) asm("monga_main");

int main(void)
{
  int result = monga_main();
  printf("Monga program returned %d.\n", result);
  return result;
}