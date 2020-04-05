/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1996, 1999, 2000 by Ralf Baechle
 * Copyright (C) 1999, 2000 Silicon Graphics, Inc.
 */
#ifndef _ASM_UNALIGNED_H
#define _ASM_UNALIGNED_H

extern void __get_unaligned_bad_length(void);
extern void __put_unaligned_bad_length(void);

/*
 * Load double unaligned.
 *
 * This could have been implemented in plain C like IA64 but egcs 1.0.3a
 * inflates this to 23 instructions ...
 */
static inline unsigned long long __ldq_u(const unsigned long long * __addr)
{
	unsigned long long __res;

	__asm__("ulw\t%0, %1\n\t"
		"ulw\t%D0, 4+%1"
		: "=&r" (__res)
		: "m" (*__addr));

	return __res;
}

/*
 * Load word unaligned.
 */
static inline unsigned long __ldl_u(const unsigned int * __addr)
{
	unsigned long __res;

	__asm__("ulw\t%0,%1"
		: "=&r" (__res)
		: "m" (*__addr));

	return __res;
}

/*
 * Load halfword unaligned.
 */
static inline unsigned long __ldw_u(const unsigned short * __addr)
{
	unsigned long __res;

	__asm__("ulh\t%0,%1"
		: "=&r" (__res)
		: "m" (*__addr));

	return __res;
}

/*
 * Store doubleword ununaligned.
 */
static inline void __stq_u(unsigned long __val, unsigned long long * __addr)
{
	__asm__("usw\t%1, %0\n\t"
		"usw\t%D1, 4+%0"
		: "=m" (*__addr)
		: "r" (__val));
}

/*
 * Store long ununaligned.
 */
static inline void __stl_u(unsigned long __val, unsigned int * __addr)
{
	__asm__("usw\t%1, %0"
		: "=m" (*__addr)
		: "r" (__val));
}

/*
 * Store word ununaligned.
 */
static inline void __stw_u(unsigned long __val, unsigned short * __addr)
{
	__asm__("ush\t%1, %0"
		: "=m" (*__addr)
		: "r" (__val));
}

/*
 * get_unaligned - get value from possibly mis-aligned location
 * @ptr: pointer to value
 *
 * This macro should be used for accessing values larger in size than
 * single bytes at locations that are expected to be improperly aligned,
 * e.g. retrieving a u16 value from a location not u16-aligned.
 *
 * Note that unaligned accesses can be very expensive on some architectures.
 */
#if 0
#define get_unaligned(ptr)						\
({									\
	__typeof__(*(ptr)) __val;					\
									\
	switch (sizeof(*(ptr))) {					\
	case 1:								\
		__val = *(const unsigned char *)ptr;			\
		break;							\
	case 2:								\
		__val = __ldw_u((const unsigned short *)ptr);		\
		break;							\
	case 4:								\
		__val = __ldl_u((const unsigned int *)ptr);		\
		break;							\
	case 8:								\
		__val = __ldq_u((const unsigned long long *)ptr);	\
		break;							\
	default:							\
		__get_unaligned_bad_length();				\
		break;							\
	}								\
									\
	__val;								\
})

/*
 * put_unaligned - put value to a possibly mis-aligned location
 * @val: value to place
 * @ptr: pointer to location
 *
 * This macro should be used for placing values larger in size than
 * single bytes at locations that are expected to be improperly aligned,
 * e.g. writing a u16 value to a location not u16-aligned.
 *
 * Note that unaligned accesses can be very expensive on some architectures.
 */
#define put_unaligned(val,ptr)						\
do {									\
	switch (sizeof(*(ptr))) {					\
	case 1:								\
		*(unsigned char *)(ptr) = (val);			\
		break;							\
	case 2:								\
		__stw_u(val, (unsigned short *)(ptr));			\
		break;							\
	case 4:								\
		__stl_u(val, (unsigned int *)(ptr));			\
		break;							\
	case 8:								\
		__stq_u(val, (unsigned long long *)(ptr));		\
		break;							\
	default:							\
		__put_unaligned_bad_length();				\
		break;							\
	}								\
} while(0)
#else
//int __bug_unaligned_x(void *ptr){}
/*
 * What is the most efficient way of loading/storing an unaligned value?
 *
 * That is the subject of this file.  Efficiency here is defined as
 * minimum code size with minimum register usage for the common cases.
 * It is currently not believed that long longs are common, so we
 * trade efficiency for the chars, shorts and longs against the long
 * longs.
 *
 * Current stats with gcc 2.7.2.2 for these functions:
 *
 *	ptrsize	get:	code	regs	put:	code	regs
 *	1		1	1		1	2
 *	2		3	2		3	2
 *	4		7	3		7	3
 *	8		20	6		16	6
 *
 * gcc 2.95.1 seems to code differently:
 *
 *	ptrsize	get:	code	regs	put:	code	regs
 *	1		1	1		1	2
 *	2		3	2		3	2
 *	4		7	4		7	4
 *	8		19	8		15	6
 *
 * which may or may not be more efficient (depending upon whether
 * you can afford the extra registers).  Hopefully the gcc 2.95
 * is inteligent enough to decide if it is better to use the
 * extra register, but evidence so far seems to suggest otherwise.
 *
 * Unfortunately, gcc is not able to optimise the high word
 * out of long long >> 32, or the low word from long long << 32
 */

#define __get_unaligned_2(__p)					\
	(__p[0] << 8 | __p[1] )

#define __get_unaligned_4(__p)					\
	(__p[0] <<24 | __p[1] <<16 | __p[2] << 8 | __p[3] )

#define get_unaligned(ptr)					\
	({							\
		__typeof__(*(ptr)) __v;				\
		__u8 *__p = (__u8 *)(ptr);			\
		switch (sizeof(*(ptr))) {			\
		case 1:	__v = *(ptr);			break;	\
		case 2: __v = __get_unaligned_2(__p);	break;	\
		case 4: __v = __get_unaligned_4(__p);	break;	\
		case 8: {					\
				unsigned int __v1, __v2;	\
				__v2 = __get_unaligned_4((__p)); \
				__v1 = __get_unaligned_4((__p+4));	\
				__v = ((unsigned long long)__v2 << 32 | __v1);	\
			}					\
			break;					\
		default:					\
			__get_unaligned_bad_length();		\
			break;					\
		}						\
		__v;						\
	})


static inline void __put_unaligned_2(__u32 __v, register __u8 *__p)
{
	*__p++ = __v>>8;
	*__p++ = __v;
}

static inline void __put_unaligned_4(__u32 __v, register __u8 *__p)
{
	__put_unaligned_2(__v >>16, __p );
	__put_unaligned_2(__v , __p +2);
}

static inline void __put_unaligned_8(const unsigned long long __v, register __u8 *__p)
{
	/*
	 * tradeoff: 8 bytes of stack for all unaligned puts (2
	 * instructions), or an extra register in the long long
	 * case - go for the extra register.
	 */
	__put_unaligned_4(__v >> 32, __p);
	__put_unaligned_4(__v, __p+4);
}

/*
 * Try to store an unaligned value as efficiently as possible.
 */
#define put_unaligned(val,ptr)					\
	({							\
		switch (sizeof(*(ptr))) {			\
		case 1:						\
			*(ptr) = (val);				\
			break;					\
		case 2: __put_unaligned_2((val),(__u8 *)(ptr));	\
			break;					\
		case 4:	__put_unaligned_4((val),(__u8 *)(ptr));	\
			break;					\
		case 8:	__put_unaligned_8((val),(__u8 *)(ptr)); \
			break;					\
		default:						\
			 __put_unaligned_bad_length();		\
			break;					\
		}						\
		(void) 0;					\
	})


#endif
#endif /* _ASM_UNALIGNED_H */
