/*
 * Copyright (c) 2006, Realtek Semiconductor Corp.
 *
 * cov.c:
 *   RLXCOV example program
 *
 * Tony Wu (tonywu@realtek.com.tw)
 * Jul. 26, 2006
 */

#include <stdio.h>
#include "rlx_library.h"


int func1(void)
{
  printf("This is function 1");
  return 0;
}

int func2(void)
{
  printf("This is function 2");
  return 0;
}

int
main(int argc, char **argv)
{
  int n;

  printf("We should do something here\n");

  n = rand();
  if (n%2 == 0) {
    func1();
  } else {
    func2();
  }

  printf("Nah.\n");
  return 0;
}
