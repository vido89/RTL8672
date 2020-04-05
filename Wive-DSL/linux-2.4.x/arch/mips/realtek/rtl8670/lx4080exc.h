/*
 * lx4080exc.h - Reserved instruction handler exception stack
 *               frame definitions for the LX-4080 processor.
 */

/*
** Copyright 1998 Lexra, Inc.
*/

/*
DESCRIPTION
This is the header file for the Lexra-supplied "reserved instruction
exception handler" routines required for use with the Lexra LX-4080
microprocessor core.

*/

#ifndef	__lx4080exc_h_
#define	__lx4080exc_h_

#define OP_MASK 	0x3f
#define OP_SHIFT	26
#define R_MASK		0x1f
#define RS_SHIFT	21
#define RT_SHIFT	16
#define RD_SHIFT	11
#define OFFSET_MASK	0xffff

#define _OP_(x)		(OP_MASK & ((x) >> OP_SHIFT))
#define _OPS_(x)	(OP_MASK & (x))
#define _RS_(x)		(R_MASK & ((x) >> RS_SHIFT))
#define _RT_(x)		(R_MASK & ((x) >> RT_SHIFT))
#define _RD_(x)		(R_MASK & ((x) >> RD_SHIFT))
#define _OFFSET_(x)	(OFFSET_MASK & (x))

#ifndef _ASMLANGUAGE
typedef enum {
  BRANCH_T_NONE,
  BRANCH_T_OFFSET,
  BRANCH_T_TARGET,
  BRANCH_T_REGISTER
} Branch_t;
#endif /* _ASMLANGUAGE */

#ifdef IN_PMON
#include "pmon.h"

#ifndef _ASMLANGUAGE
typedef signed int LONG;
typedef signed int INT;
typedef unsigned int ULONG;
typedef unsigned int UINT;

extern ULONG emulatedHI;
extern ULONG emulatedLO;
#endif /* _ASMLANGUAGE */

#define _STKOFFSET(rno)	(*(ULONG *)(pEXCdata+((rno)*4)))
#define _GPR_STKOFFSET(x)	_STKOFFSET(x)

#define EXC_DATA_CAUSE	R_CAUSE /* cause register */
#define EXC_DATA_BADVADDR	R_BADVADDR /* bad virtual address reg */

#define EXC_DATA_SR		R_SR /* status register */
#define EXC_DATA_EPC		R_EPC /* calculated epc */

#define EXC_DATA_LO		R_LO /* mul/div low   */
#define EXC_DATA_HI		R_HI /* mul/div hi   */

#define EXC_DATA_ZERO	R_ZERO /* just for kicks, always zero */
#define EXC_DATA_AT		R_AT /* assembler temporary */
#define EXC_DATA_V0		R_V0 /* return value 1 */
#define EXC_DATA_V1		R_V1 /* return value 2 */
#define EXC_DATA_A0		R_A0 /* passed parameter 1 */
#define EXC_DATA_A1		R_A1 /* passed parameter 2 */
#define EXC_DATA_A2		R_A2 /* passed parameter 3 */
#define EXC_DATA_A3		R_A3 /* passed parameter 4 */
#define EXC_DATA_T0		R_T0 /* temp reg t0 */
#define EXC_DATA_T1		R_T1 /* temp reg t1 */
#define EXC_DATA_T2		R_T2 /* temp reg t2 */
#define EXC_DATA_T3		R_T3 /* temp reg t3 */
#define EXC_DATA_T4		R_T4 /* temp reg t4 */
#define EXC_DATA_T5		R_T5 /* temp reg t5 */
#define EXC_DATA_T6		R_T6 /* temp reg t6 */
#define EXC_DATA_T7		R_T7 /* temp reg t7 */
#define EXC_DATA_S0		R_S0 /* saved reg s0 */
#define EXC_DATA_S1		R_S1 /* saved reg s1 */
#define EXC_DATA_S2		R_S2 /* saved reg s2 */
#define EXC_DATA_S3		R_S3 /* saved reg s3 */
#define EXC_DATA_S4		R_S4 /* saved reg s4 */
#define EXC_DATA_S5		R_S5 /* saved reg s5 */
#define EXC_DATA_S6		R_S6 /* saved reg s6 */
#define EXC_DATA_S7		R_S7 /* saved reg s7 */
#define EXC_DATA_T8		R_T8 /* temp reg t8 */
#define EXC_DATA_T9		R_T9 /* temp reg t9 */
#define EXC_DATA_K0		R_K0 /* kernel reg 0 */
#define EXC_DATA_K1		R_K1 /* kernel reg 1, not saved */
#define EXC_DATA_GP		R_GP /* global pointer */
#define EXC_DATA_SP		R_SP /* stack pointer */
#define EXC_DATA_FP		R_FP /* saved reg */
#define EXC_DATA_RA		R_RA /* return addr reg */
#endif /* IN_PMON */

#ifdef IN_TORNADO			/* Apparently in Tornado BSP */
#include "vxWorks.h"
#include "config.h"
#ifndef _ASMLANGUAGE
#include "esf.h"
#include "wdb/wdbRegs.h"
#endif /* _ASMLANGUAGE */

#ifndef _ASMLANGUAGE
IMPORT ULONG emulatedHI;
IMPORT ULONG emulatedLO;
#endif /* _ASMLANGUAGE */

#define _STKOFFSET(rno)		(*(ULONG *)(pEXCdata+(rno)))
#define _GPR_STKOFFSET(x)	(*(ULONG *)(pEXCdata+(E_STK_GREG_OFFSET(x))))

#define EXC_DATA_CAUSE	E_STK_CAUSE /* cause register */
#define EXC_DATA_BADVADDR	E_STK_BADVADDR /* bad virtual address reg */

#define EXC_DATA_SR		E_STK_SR /* status register */
#define EXC_DATA_EPC		E_STK_EPC /* calculated epc */

#define EXC_DATA_LO		E_STK_LO /* mul/div low   */
#define EXC_DATA_HI		E_STK_HI /* mul/div hi   */

#define EXC_DATA_ZERO	E_STK_ZERO /* just for kicks, always zero */
#define EXC_DATA_AT		E_STK_AT /* assembler temporary */
#define EXC_DATA_V0		E_STK_V0 /* return value 1 */
#define EXC_DATA_V1		E_STK_V1 /* return value 2 */
#define EXC_DATA_A0		E_STK_A0 /* passed parameter 1 */
#define EXC_DATA_A1		E_STK_A1 /* passed parameter 2 */
#define EXC_DATA_A2		E_STK_A2 /* passed parameter 3 */
#define EXC_DATA_A3		E_STK_A3 /* passed parameter 4 */
#define EXC_DATA_T0		E_STK_T0 /* temp reg t0 */
#define EXC_DATA_T1		E_STK_T1 /* temp reg t1 */
#define EXC_DATA_T2		E_STK_T2 /* temp reg t2 */
#define EXC_DATA_T3		E_STK_T3 /* temp reg t3 */
#define EXC_DATA_T4		E_STK_T4 /* temp reg t4 */
#define EXC_DATA_T5		E_STK_T5 /* temp reg t5 */
#define EXC_DATA_T6		E_STK_T6 /* temp reg t6 */
#define EXC_DATA_T7		E_STK_T7 /* temp reg t7 */
#define EXC_DATA_S0		E_STK_S0 /* saved reg s0 */
#define EXC_DATA_S1		E_STK_S1 /* saved reg s1 */
#define EXC_DATA_S2		E_STK_S2 /* saved reg s2 */
#define EXC_DATA_S3		E_STK_S3 /* saved reg s3 */
#define EXC_DATA_S4		E_STK_S4 /* saved reg s4 */
#define EXC_DATA_S5		E_STK_S5 /* saved reg s5 */
#define EXC_DATA_S6		E_STK_S6 /* saved reg s6 */
#define EXC_DATA_S7		E_STK_S7 /* saved reg s7 */
#define EXC_DATA_T8		E_STK_T8 /* temp reg t8 */
#define EXC_DATA_T9		E_STK_T9 /* temp reg t9 */
#define EXC_DATA_K0		E_STK_K0 /* kernel reg 0 */
#define EXC_DATA_K1		E_STK_K1 /* kernel reg 1, not saved */
#define EXC_DATA_GP		E_STK_GP /* global pointer */
#define EXC_DATA_SP		E_STK_SP /* stack pointer */
#define EXC_DATA_FP		E_STK_FP /* saved reg */
#define EXC_DATA_RA		E_STK_RA /* return addr reg */

#ifndef SBD_DISPLAY
#define SBD_DISPLAY(x,y)
#endif /* SBD_DISPLAY */
#endif /* IN_TORNADO */	/* Apparently in Tornado BSP */

/* Continue common definitions here */
#ifndef _ASMLANGUAGE
#ifdef INSTRUMENT_RI_TRAPS
typedef struct {
  unsigned long lwl;
  unsigned long lwr;
  unsigned long swl;
  unsigned long swr;
  unsigned long mfhi;
  unsigned long mthi;
  unsigned long mflo;
  unsigned long mtlo;
  unsigned long mult;
  unsigned long multu;
  unsigned long div;
  unsigned long divu;
} countRI_t;
#endif

#ifdef IN_PMON
int emulatelx4080RI(char *pEXCdata);
#endif

#define	CAUSE_BD	0x80000000	/* Branch delay slot */
#define	CAUSE_CEMASK	0x30000000	/* coprocessor error */
#define	CAUSE_CESHIFT	28

int emulatelx4080RI ( int vec, 
			     char *pEXCdata, 
			     struct pt_regs *pRegs
			     );

void lx4080ExcVecInit(void);
#endif /* _ASMLANGUAGE */
#endif /* __lx4080exc_h_ */
