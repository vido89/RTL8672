#ifndef _MYTYPES_H
#define _MYTYPES_H

#include "rtl_types.h"
#include "rtk_voip.h"

typedef unsigned int	RESULT;
typedef unsigned char	uchar;
typedef unsigned char UCHAR;

#ifndef __cplusplus
#ifndef __ECOS
typedef int				bool;
#endif
#endif

#define BOOL unsigned char


#ifndef NULL
#define NULL		0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif


/* unsigned operations */
#define _div32(x, y) ((x) / (y))
#define _mul32(x, y) ((x) * (y))
#define _mod32(x, y) ((x) % (y))

/* signed operations */
#define _idiv32(x, y) ((x) / (y))
#define _imul32(x, y) ((x) * (y))
#define _imod32(x, y) ((x) % (y))

#endif // _MYTYPES_H
