#ifndef TYPEDEF_H
#define TYPEDEF_H

#include <linux/config.h>
#include "rtk_voip.h"
#include "myTypes.h"

#ifndef NULL
#define NULL	(void*)0
#endif

#define ALIGN(x)			__attribute__((aligned(x)))
#define MEM_SECTION(sect)	__attribute__((section (sect)))


#if 0  // moved to rtl_types.h
typedef  long  int   Word32;
typedef  short int   Word16;	
typedef  short int   Flag;	
#endif

typedef unsigned int UINT;
typedef unsigned short USINT;


#endif
