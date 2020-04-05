#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define HEAP_START _end

extern caddr_t HEAP_START;

/*
** The RAMSIZE value must be greater than HEAP_START
** for malloc to work.
*/

#  define RAMSIZE             (caddr_t)(((unsigned long)&HEAP_START + (unsigned long)0x80000) \
					& ~((unsigned long)0xf))
