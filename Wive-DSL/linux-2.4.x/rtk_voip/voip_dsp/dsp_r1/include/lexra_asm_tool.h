
#ifndef	_LEXRA_ASM_TOOL_H_
#define _LEXRA_ASM_TOOL_H_

#define __asm__ asm
#define ASM __asm__ volatile


long top_bit_asm(unsigned int bits);

#define _top_bit_asm(bits)\
({ \
	long __result; \
	ASM ("cls %0, %1" : "=d" (__result) : "d" ((long)(bits))); \
	if(((long)(bits)) <0) \
		__result = 31;\
	else \
		__result = 30- __result; \
	__result;\
})

#define top_bit_asm _top_bit_asm

/* count leading sign bits */
long leadsign(long a);
#define _leadsign(a) \
({ \
        long __result; \
        ASM ("cls %0, %1" : "=d" (__result) : "d" ((long)(a))); \
        __result; \
})

#define leadsign _leadsign

//#ifndef zero
//#define zero	$0
//#endif

#ifndef mmd
	#define mmd	$24		/* md */
#endif

#ifndef SET_MMD_MT
	#define SET_MMD_MT	0x1		/* MAC 32*32 truncate mode */
#endif
#ifndef SET_MMD_MS
	#define SET_MMD_MS	0x2		/* MAC 32 bit saturate mode */
#endif
#ifndef SET_MMD_MF
	#define SET_MMD_MF	0x4		/* MAC fractional mode */
#endif

#ifndef m0l
	#define m0l	$1		/* ma0l */
#endif

#ifndef m0h
	#define m0h	$2		/* ma0h */
#endif

#ifndef m0
	#define m0	$3		/* ma0  */
#endif

#ifndef m1l
	#define m1l	$5		/* ma1l */
#endif

#ifndef m1h
	#define m1h	$6		/* ma1h */
#endif

#ifndef m1
	#define m1	$7		/* ma1  */
#endif

#ifndef m2l
	#define m2l	$9		/* ma2l */
#endif

#ifndef m2h
	#define m2h	$10		/* ma2h */
#endif

#ifndef m2
	#define m2	$11		/* ma2  */
#endif


#if 1
long set_save_mmd(long set);
static inline long _set_save_mmd(long set)
{ 
	long __save; 
	ASM ("mfru %0, mmd \n\t" 
	     "mtru %1, mmd" 
	     : "=&d" (__save) 
	     : "d" ((long)(set)) 
	     ); 
	return __save; 
}

void restore_mmd(long save);
static inline void _restore_mmd(long save) 
{ 
	ASM ("mtru %0, mmd": /* no outputs */ : "d" ((long)(save))); 
}
/* 32x32 fractional multiply */
long fra_fmul32(long a, long b);
static inline long _fra_fmul32(long a, long b) 
{ 
    long __result; 
    ASM ("multa	m0, %0, %1" : /* no outputs */ : "d" ((long)(a)), "d" ((long)(b))); 
    ASM ("mfa	%0, m0h" : "=d" (__result) : /* no inputs */); 
    return __result; 
}

/* 16x16 fractional multiply; mmd=fract mode */
short fra_fmul16(short a, short b);
static inline short _fra_fmul16(short a, short b)
{
	short __result; 
	ASM ("multa2	m1l, %0, %1" : /* no outputs */ : "d" ((short)(a)), "d" ((short)(b))); \
	ASM ("mfa	$15, m1l" :/* no outputs */:/* no inputs */:"$15");	\
	ASM ("sra	%0, $15, 16" : "=d" ((short)__result) : /* no inputs */:"$15");	\
	return __result; 
}

/* clear ma0 */
void clear_m0(void);
static inline void _clear_m0(void)
{
	ASM ("mta2	$0, m0" : /* no outputs */ : /* no inputs */); 
}

/* clear ma1 */
void clear_m1(void);
static inline void _clear_m1(void)
{
	ASM ("mta2	$0, m1" : /* no outputs */ : /* no inputs */); 
}

/* clear ma2 */
void clear_m2(void);
static inline void _clear_m2(void)
{
	ASM ("mta2	$0, m2" : /* no outputs */ : /* no inputs */);
}

/* multiply-add m0 */
void mult_add_m0(long a);
static inline void _mult_add_m0(long a)
{
	ASM ("madda	m0, %0,%0" : /* no outputs */ : "d" ((long)(a)));
}

/* multiply-add m1 */
void mult_add_m1(long a);
static inline void _mult_add_m1(long a)
{
	ASM ("madda	m1, %0,%0" : /* no outputs */ : "d" ((long)(a)));
}

/* multiply-add m2 */
void mult_add_m2(long a);
static inline void _mult_add_m2(long a)
{
	ASM ("madda	m2, %0,%0" : /* no outputs */ : "d" ((long)(a)));
}

/* move from m0l */
long mov_from_m0l(void);
static inline long _mov_from_m0l(void) 
{ 
	long __result; 
	ASM ("mfa	%0, m0l" : "=d" (__result) : /* no inputs */); 
	return __result; 
}

/* move from m0h */
long mov_from_m0h(void);
static inline long _mov_from_m0h(void) 
{ 
	long __result; 
	ASM ("mfa	%0, m0h" : "=d" (__result) : /* no inputs */); 
	return __result; 
}

/* move from m1l */
long mov_from_m1l(void);
static inline long _mov_from_m1l(void) 
{ 
	long __result; 
	ASM ("mfa	%0, m1l" : "=d" (__result) : /* no inputs */); 
	return __result; 
}

/* move from m0l */
long mov_from_m1h(void);
static inline long _mov_from_m1h(void) 
{ 
	long __result; 
	ASM ("mfa	%0, m1h" : "=d" (__result) : /* no inputs */); 
	return __result; 
}

/* move from m2l */
long mov_from_m2l(void);
static inline long _mov_from_m2l(void) 
{ 
	long __result; 
	ASM ("mfa	%0, m2l" : "=d" (__result) : /* no inputs */); 
	return __result; 
}

/* move from m2h */
long mov_from_m2h(void);
static inline long _mov_from_m2h(void) 
{ 
	long __result; 
	ASM ("mfa	%0, m2h" : "=d" (__result) : /* no inputs */); 
	return __result; 
}

#else
long set_save_mmd(long set);
#define _set_save_mmd(set) \
({ \
	long __save; \
	ASM ("mfru %0, mmd \n\t" \
	     "mtru %1, mmd" \
	     : "=&d" (__save) \
	     : "d" ((long)(set)) \
	     ); \
	__save; \
})

void restore_mmd(long save);
#define _restore_mmd(save) \
({ \
	ASM ("mtru %0, mmd": /* no outputs */ : "d" ((long)(save))); \
})
/* 32x32 fractional multiply */
long fra_fmul32(long a, long b);
#define _fra_fmul32(a,b) \
({ \
    long __result; \
    ASM ("multa	m0, %0, %1" : /* no outputs */ : "d" ((long)(a)), "d" ((long)(b))); \
    ASM ("mfa	%0, m0h" : "=d" (__result) : /* no inputs */); \
    __result; \
})
#endif



#define set_save_mmd _set_save_mmd
#define restore_mmd _restore_mmd
#define fra_fmul32 _fra_fmul32
#define fra_fmul16 _fra_fmul16

#define clear_m0 _clear_m0
#define clear_m1 _clear_m1
#define clear_m2 _clear_m2

#define mult_add_m0 _mult_add_m0
#define mult_add_m1 _mult_add_m1
#define mult_add_m2 _mult_add_m2

#define mov_from_m0l _mov_from_m0l
#define mov_from_m0h _mov_from_m0h
#define mov_from_m1l _mov_from_m1l
#define mov_from_m1h _mov_from_m1h
#define mov_from_m2l _mov_from_m2l
#define mov_from_m2h _mov_from_m2h


#endif	/* _LEXRA_ASM_TOOL_H_ */
