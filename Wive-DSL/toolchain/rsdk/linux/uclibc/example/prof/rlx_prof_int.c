/*
 * Copyright (c) 2006, Realtek Semiconductor Corp.
 *
 * rlx_prof_int.c:
 *    Enable/Disable interrupts 
 *
 * Zhe Jiang (zhe_jiang@realsil.com.cn)
 * Tony Wu (tonywu@realtek.com.tw)
 *
 * Jul. 19, 2006
 */

#include <rlx_profiler.h>

#undef printf
int printf(const char *format, ...) __attribute__((far_call));

void
rlx_prof_disable_int(void)
{
  printf("to be implemented\n");
  /* disable interrupt here */
}

void
rlx_prof_enable_int(void)
{
  printf("to be implemented\n");
  /* enable interrupt here */
}
