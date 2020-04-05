/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.                                                    
* 
* Program : The header file of realtek type definition
* Abstract :                                                           
* Author :              
* $Id: rtl_types.h,v 1.1.1.1 2003/08/18 05:43:22 kaohj Exp $
* $Log: rtl_types.h,v $
* Revision 1.1.1.1  2003/08/18 05:43:22  kaohj
* initial import into CVS
*
* Revision 1.2  2003/05/23 06:11:23  elvis
* regular update
*
* Revision 1.1  2003/05/12 10:03:41  kckao
* added from csp
*
* Revision 1.4  2003/05/05 03:28:52  orlando
* add OK/NOT_OK, and INIT() macro.
*
* Revision 1.3  2003/05/05 02:09:55  kckao
* Modify for OS independent
*
* Revision 1.1.1.2  2003/03/31 06:29:37  elvis
* no message
*
* Revision 1.1.1.1  2003/03/31 06:09:26  elvis
* no message
*
* Revision 1.2  2003/03/31 05:38:27  elvis
* add ether_addr_t
*
* Revision 1.1.1.1  2003/03/13 05:52:03  elvis
* no message
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
typedef unsigned	int	UINT32;
typedef unsigned	char	UCHAR;

typedef int	STATUS;
#define LOCAL 
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

#if 0
#ifndef NULLCHAR
#define NULLCHAR 0
#endif
#ifndef NUPTR
#define NUPTR (uint32*)0
#endif

typedef enum RESULT_E
{
   RESULT_FAIL = 0, /* for post diagnosis return value, orlando 4/27/199 */
   RESULT_PASS      /* for post diagnosis return value, orlando 4/27/199 */
}RESULT_T;

#define	__P(protos)	protos		/* full-blown ANSI C */
#endif
#define ASSERT_CSP(x) if (!(x)) {printk("\nAssert Fail: %s %d", __FILE__, __LINE__); while(1);}
#define ASSERT_ISR(x) if(!(x)) {while(1);}

#define ABORT	1
 
#define PASS 	0
#define INIT(expr, module, abort) do {\
	if(expr){\
		printk("Error >>> %s initialize Failed!!!\n", module);\
		if(abort)\
			return;\
	}else{\
		printk("%s initialized\n", module);\
	}\
}while(0)
 
#define RTL_STATIC_INLINE   static __inline__

#ifndef CLEARBITS
#define CLEARBITS(a,b)	((a) &= ~(b))
#endif

#ifndef SETBITS
#define SETBITS(a,b)		((a) |= (b))
#endif

#ifndef ISSET
#define ISSET(a,b)		(((a) & (b))!=0)
#endif

#ifndef ISCLEARED
#define ISCLEARED(a,b)	(((a) & (b))==0)
#endif

#ifndef max
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif			   /* max */

#ifndef min
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif			   /* min */

//round down x to multiple of y.  Ex: ROUNDDOWN(20, 7)=14
#ifndef ROUNDDOWN
#define	ROUNDDOWN(x, y)	(((x)/(y))*(y))
#endif

//round up x to multiple of y. Ex: ROUNDUP(11, 7) = 14
#ifndef ROUNDUP
#define	ROUNDUP(x, y)	((((x)+((y)-1))/(y))*(y))  /* to any y */
#endif

#ifndef ROUNDUP2
#define	ROUNDUP2(x, y)	(((x)+((y)-1))&(~((y)-1))) /* if y is powers of two */
#endif

#ifndef ROUNDUP4
#define	ROUNDUP4(x)		((1+(((x)-1)>>2))<<2)
#endif

#ifndef IS4BYTEALIGNED
#define IS4BYTEALIGNED(x)	 ((((x) & 0x3)==0)? 1 : 0)
#endif

#define swapl32(x)\
        ((((x) & 0xff000000U) >> 24) | \
         (((x) & 0x00ff0000U) >>  8) | \
         (((x) & 0x0000ff00U) <<  8) | \
         (((x) & 0x000000ffU) << 24))
#define swaps16(x)        \
        ((((x) & 0xff00) >> 8) | \
         (((x) & 0x00ff) << 8))


//Get/Set host-order value from/to little endian buffer. 
//first byte ptr points to Little endian system's LSB.
#define GET_UINT16_LITTLE_ENDIAN_UNALIGNED( ptr )  ((uint16)  (*(((uint8*)(ptr))+1)<<8) | *((uint8*)(ptr)))
#define SET_UINT16_LITTLE_ENDIAN_UNALIGNED(  u16value, ptr )	do{\
		*((uint8*)(ptr)) =  (uint8)((u16value));/* get and set LSB */\
		*(((uint8*)(ptr))+1) =  (uint8)((u16value)>>8);/*get and set MSB*/\
}while(0)
#define GET_UINT32_LITTLE_ENDIAN_UNALIGNED( ptr )  ((uint32)  (*(((uint8*)(ptr))+3)<<24)|(*(((uint8*)(ptr))+2)<<16)|(*(((uint8*)(ptr))+1)<<8) | *((uint8*)(ptr)))
#define SET_UINT32_LITTLE_ENDIAN_UNALIGNED(  u32value, ptr )	do{\
		*((uint8*)(ptr)) =  (uint8)((u32value));/* get and set LSB */\
		*(((uint8*)(ptr))+1) =  (uint8)((u32value)>>8);\
		*(((uint8*)(ptr))+2) =  (uint8)((u32value)>>16);\
		*(((uint8*)(ptr))+3) =  (uint8)((u32value)>>24);/*get and set MSB*/\
}while(0)


//Get/Set host-order value from/to big endian buffer. 
//first byte ptr points to Big endian system's the MSB
#define GET_UINT16_BIG_ENDIAN_UNALIGNED( ptr )  ((uint16)  (*((uint8*)(ptr))<<8) | *(((uint8*)(ptr))+1))
#define SET_UINT16_BIG_ENDIAN_UNALIGNED(  u16value, ptr )	do{\
	*((uint8*)(ptr)) = (uint8)(((uint16)u16value)>>8);  /*get and set MSB*/\
	*(((uint8*)(ptr))+1) = (uint8)(u16value); /* get and set LSB */\
}while(0)
#define GET_UINT32_BIG_ENDIAN_UNALIGNED( ptr )  ((uint32)  (*(((uint8*)(ptr))+3)<<24)|(*(((uint8*)(ptr))+2)<<16)|(*(((uint8*)(ptr))+1)<<8) | *((uint8*)(ptr)))
#define SET_UINT32_BIG_ENDIAN_UNALIGNED( u32value, ptr )	do{\
	*((uint8*)(ptr)) =  (uint8)((u32value)>>24);/* get and set MSB */\
	*(((uint8*)(ptr))+1) =  (uint8)((u32value)>>16);\
	*(((uint8*)(ptr))+2) =  (uint8)((u32value)>>8);\
	*(((uint8*)(ptr))+3) =  (uint8)((u32value));/*get and set LSB*/\
}while(0)


#if 0 /* _LITTLE_ENDIAN */
	#ifndef ntohs
	#define ntohs(x)   swaps16(x)
	#endif
	#ifndef ntohl
	#define ntohl(x)   swapl32(x)
	#endif
	#ifndef htons
	#define htons(x)   swaps16(x)
	#endif
	#ifndef htonl
	#define htonl(x)   swapl32(x)
	#endif
	#ifndef NTOHL
	#define NTOHL(d) ((d) = ntohl((d)))
	#endif
	#ifndef NTOHS
	#define NTOHS(d) ((d) = ntohs((uint16)(d)))
	#endif
	#ifndef HTONL
	#define HTONL(d) ((d) = htonl((d)))
	#endif
	#ifndef HTONS
	#define HTONS(d) ((d) = htons((uint16)(d)))
	#endif

	#define GET_UINT16_UNALIGNED( ptr )  				GET_UINT16_LITTLE_ENDIAN_UNALIGNED(ptr)
	#define SET_UINT16_UNALIGNED(  u16value, ptr )		SET_UINT16_LITTLE_ENDIAN_UNALIGNED(  u16value, ptr )
	#define GET_UINT32_UNALIGNED( ptr )  				GET_UINT32_LITTLE_ENDIAN_UNALIGNED(ptr)
	#define SET_UINT32_UNALIGNED(  u32value, ptr )		SET_UINT32_LITTLE_ENDIAN_UNALIGNED(  u32value,ptr)
	#define PKTGET_UINT16_UNALIGNED( ptr )  				GET_UINT16_BIG_ENDIAN_UNALIGNED(ptr)
	#define PKTSET_UINT16_UNALIGNED(  u16value, ptr 	)	SET_UINT16_BIG_ENDIAN_UNALIGNED(  u16value, ptr )
	#define PKTGET_UINT32_UNALIGNED( ptr )  				GET_UINT32_BIG_ENDIAN_UNALIGNED(ptr)
	#define PKTSET_UINT32_UNALIGNED(  u32value, ptr )	SET_UINT32_BIG_ENDIAN_UNALIGNED(  u32value,ptr)

#else /*Big endian system */

	#ifndef ntohs
	#define ntohs(x)   (x)
	#endif
	#ifndef ntohl
	#define ntohl(x)   (x)
	#endif
	#ifndef htons
	#define htons(x)   (x)
	#endif
	#ifndef htonl
	#define htonl(x)   (x)
	#endif
	#ifndef NTOHL
	#define NTOHL(d)
	#endif
	#ifndef NTOHS
	#define NTOHS(d)
	#endif
	#ifndef HTONL
	#define HTONL(d) 
	#endif
	#ifndef HTONS
	#define HTONS(d) 
	#endif


	
	#define GET_UINT16_UNALIGNED( ptr )  			GET_UINT16_BIG_ENDIAN_UNALIGNED(ptr)
	#define SET_UINT16_UNALIGNED(  u16value, ptr )	SET_UINT16_BIG_ENDIAN_UNALIGNED(  u16value, ptr )
	#define GET_UINT32_UNALIGNED( ptr )  			GET_UINT32_BIG_ENDIAN_UNALIGNED(ptr)
	#define SET_UINT32_UNALIGNED(  u32value, ptr )	SET_UINT32_BIG_ENDIAN_UNALIGNED(  u32value,ptr)
	#define PKTGET_UINT16_UNALIGNED( ptr )  			GET_UINT16_BIG_ENDIAN_UNALIGNED(ptr)
	#define PKTSET_UINT16_UNALIGNED(  u16value, ptr)	SET_UINT16_BIG_ENDIAN_UNALIGNED(  u16value, ptr )
	#define PKTGET_UINT32_UNALIGNED( ptr )  			GET_UINT32_BIG_ENDIAN_UNALIGNED(ptr)
	#define PKTSET_UINT32_UNALIGNED(  u32value, ptr)	SET_UINT32_BIG_ENDIAN_UNALIGNED(  u32value,ptr)	


#endif


#ifndef __offsetof
#define __offsetof(type, field) ((size_t)(&((type *)0)->field))
#endif

#ifndef offsetof
#define offsetof(type, field) __offsetof(type, field)
#endif

#endif
