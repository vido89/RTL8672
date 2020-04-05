/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.                                                    
* 
* Program : The header file of realtek type definition
* Abstract :                                                           
* Author :              
* $Id: rtl_types.h,v 1.1 2005/08/29 08:16:20 kaohj Exp $
* $Log: rtl_types.h,v $
* Revision 1.1  2005/08/29 08:16:20  kaohj
* port from goahead to boa
*
* Revision 1.1.1.1  2005/08/15 13:56:31  hjen
* initial import int CVS
*
* Revision 1.1  2003/10/01 09:08:42  dicktam
* *** empty log message ***
*
* Revision 1.1  2003/06/20 16:05:32  elvis
* Initial Version
*
* Revision 1.29  2003/04/30 15:32:30  cfliu
* move macros to types.h
*
* Revision 1.28  2003/03/13 10:29:22  cfliu
* Remove unused symbols
*
* Revision 1.27  2003/03/06 05:00:04  cfliu
* Move '#pragma ghs inlineprologue' to rtl_depend.h since it is compiler dependent
*
* Revision 1.26  2003/03/06 03:41:46  danwu
* Prevent compiler from generating internal sub-routine call code at the
*  function prologue and epilogue automatically
*
* Revision 1.25  2003/03/03 09:16:35  hiwu
* remove ip4a
*
* Revision 1.24  2003/02/18 10:04:06  jzchen
* Add ether_addr_t to compatable with protocol stack's ether_addr
*
* Revision 1.23  2003/01/21 05:59:51  cfliu
* add min, max, SETBITS, CLEARBITS, etc.
*
* Revision 1.22  2002/11/25 07:31:30  cfliu
* Remove _POSIX_SOURCE since it is cygwin specific
*
* Revision 1.21  2002/09/30 11:51:49  jzchen
* Add ASSERT_ISR for not print inside ISR
*
* Revision 1.20  2002/09/18 01:43:24  jzchen
* Add type limit definition
*
* Revision 1.19  2002/09/16 00:14:34  elvis
* remove struct posix_handle_t (change the handle type from
*  structure to uint32)
*
* Revision 1.18  2002/08/20 01:40:40  danwu
* Add definitions of ipaddr_t & macaddr_t.
*
* Revision 1.17  2002/07/30 04:36:30  danwu
* Add ASSERT_CSP.
*
* Revision 1.16  2002/07/19 06:47:30  cfliu
* Add _POSIX_SOURCE symbol
*
* Revision 1.15  2002/07/05 02:10:39  elvis
* Add new types for OSK
*
* Revision 1.14  2002/07/03 12:36:21  orlando
* <rtl_depend.h> will use type definitions. Has to be moved to
* be after the type declaration lines.
*
* Revision 1.13  2002/07/03 09:19:00  cfliu
* Removed all standard header files from source code. They would be included by <core/types.h>-><rtl_depend.h>
*
* Revision 1.12  2002/07/03 09:16:48  cfliu
* Removed all standard header files from source code. They would be included by <core/types.h>-><rtl_depend.h>
*
* Revision 1.11  2002/07/03 07:14:47  orlando
* Add "struct posix_handle_t_", used by POSIX module.
*
* Revision 1.9  2002/06/21 03:15:36  cfliu
* Add time.h for struct timeval
*
* Revision 1.8  2002/06/14 01:58:03  cfliu
* Move sa_family_t to socket
*
* Revision 1.7  2002/06/13 09:37:42  cfliu
* Move byte order conversion routines to socket
*
* Revision 1.6  2002/05/23 04:24:37  hiwu
* change memaddr_t to calladdr_t
*
* Revision 1.5  2002/05/13 10:15:16  hiwu
* add new type definition
*
* Revision 1.4  2002/05/09 05:21:51  cfliu
* Add parenthesis around swaps16, swapl32
*
* Revision 1.3  2002/04/30 03:07:34  orlando
* Remove UIxx_T definitions to conform with new
* naming conventions.
*
* Revision 1.2  2002/04/29 10:10:32  hiwu
* add NTOHS macro
*
* Revision 1.1.1.1  2002/04/26 08:53:53  orlando
* Initial source tree creation.
*
* Revision 1.9  2002/04/25 03:59:05  cfliu
* no message
*
* Revision 1.8  2002/04/08 08:08:04  hiwu
* initial version
*
*/


#ifndef _RTL_TYPES_H
#define _RTL_TYPES_H

/*
 * Internal names for basic integral types.  Omit the typedef if
 * not possible for a machine/compiler combination.
 */

typedef unsigned long long	uint64;
typedef long long		int64;
typedef unsigned int	uint32;
typedef int			int32;
typedef unsigned short	uint16;
typedef short			int16;
typedef unsigned char	uint8;
typedef char			int8;

#define UINT32_MAX	UINT_MAX
#define INT32_MIN	INT_MIN
#define INT32_MAX	INT_MAX
#define UINT16_MAX	USHRT_MAX
#define INT16_MIN	SHRT_MIN
#define INT16_MAX	SHRT_MAX
#define UINT8_MAX	UCHAR_MAX
#define INT8_MIN		SCHAR_MIN
#define INT8_MAX	SCHAR_MAX

typedef uint32		memaddr;	
typedef uint32          ipaddr_t;
typedef struct {
    uint16      mac47_32;
    uint16      mac31_16;
    uint16      mac15_0;
} macaddr_t;
typedef int8*			calladdr_t;

typedef struct ether_addr_s {
	uint8 octet[6];
} ether_addr_t;


#ifndef NULL
#define NULL 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef SUCCESS
#define SUCCESS 	0
#endif
#ifndef FAILED
#define FAILED -1
#endif

#ifndef OK
#define OK		0
#endif
#ifndef NOT_OK
#define NOT_OK  1
#endif

typedef enum RESULT_E
{
   RESULT_FAIL = 0, /* for post diagnosis return value, orlando 4/27/199 */
   RESULT_PASS      /* for post diagnosis return value, orlando 4/27/199 */
}RESULT_T;

#define ASSERT_CSP(x) if (!(x)) {printf("\nAssert Fail: %s %d", __FILE__, __LINE__); while(1);}
#define ASSERT_ISR(x) if(!(x)) {while(1);}

#define ABORT	1
#define DEBUG_P(fmt,args...) printf(fmt,## args) 
#define PASS 	0
#define INIT(expr, module, abort) do {\
	if(expr){\
		printf("Error >>> %s initialize Failed!!!\n", module);\
		if(abort)\
			return;\
	}else{\
		printf("%s initialized\n", module);\
	}\
}while(0)
 
#define RTL_STATIC_INLINE   static __inline__
#endif
