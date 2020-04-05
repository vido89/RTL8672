#ifndef __LINUX_COMPILER_H
#define __LINUX_COMPILER_H

#if __GNUC__ >= 3
#define inline __inline__ __attribute__((always_inline))
#define __inline__ __inline__ __attribute__((always_inline))
#endif

/* Somewhere in the middle of the GCC 2.96 development cycle, we implemented
   a mechanism by which the user can annotate likely branch directions and
   expect the blocks to be reordered appropriately.  Define __builtin_expect
   to nothing for earlier compilers.  */

#if __GNUC__ == 2 && __GNUC_MINOR__ <= 96
#define __builtin_expect(x, expected_value) (x)
#endif

#define likely(x)	__builtin_expect((x),1)
#define unlikely(x)	__builtin_expect((x),0)

#if __GNUC__ > 3
#define __attribute_used__	__attribute((__used__))
#elif __GNUC__ == 3
#if  __GNUC_MINOR__ >= 3
# define __attribute_used__	__attribute__((__used__))
#else
# define __attribute_used__	__attribute__((__unused__))
#endif /* __GNUC_MINOR__ >= 3 */
#elif __GNUC__ == 2
#define __attribute_used__	__attribute__((__unused__))
#else
#define __attribute_used__	/* not implemented */
#endif /* __GNUC__ */

#ifdef CONFIG_RTL867X_KERNEL_MIPS16
//#undef __NOMIPS16
//#undef __MIPS16
#define __NOMIPS16			__attribute__((nomips16))	/* Inidcate to prevent from MIPS16 */
#define __MIPS16			__attribute__((mips16))		/* Inidcate to use MIPS16 */
#else
#define __NOMIPS16
#define __MIPS16
#endif

#endif /* __LINUX_COMPILER_H */