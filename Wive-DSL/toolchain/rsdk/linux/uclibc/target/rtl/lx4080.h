#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef GHS_TOOL 
#define HEAP_START __ghsbegin_heap
#else
#define HEAP_START _end
#endif

#pragma ghs startdata
extern caddr_t HEAP_START;                /* HEAP_START is set in the linker command file */
#pragma ghs enddata

/*
** The RAMSIZE value must be greater than HEAP_START
** for malloc to work.
*/

#  define RAMSIZE             (caddr_t)(((unsigned long)&HEAP_START + (unsigned long)0x80000) \
					& ~((unsigned long)0xf))
