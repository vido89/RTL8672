/*  - machine dependent definitions for the architecture
 *	The contents of this file are not copyrighted in any
 *	way, and may therefore be used without restriction.
 *
 *	This file contains definitions for stuff that is common
 *	to *all*  CPUs.
 *
 * Author: Phil Bunce (pjb@carmel.com)
 */

#ifndef __REGS_H_INCLUDED
#define __REGS_H_INCLUDED


#define K0BASE 		0x80000000
#define K0SIZE 		0x20000000
#define K1BASE 		0xa0000000
#define K1SIZE 		0x20000000
#define K2BASE 		0xc0000000
#define	IS_K0SEG(x)	((unsigned)(x)>=K0BASE && (unsigned)(x)<K1BASE)
#define	IS_K1SEG(x)	((unsigned)(x)>=K1BASE && (unsigned)(x)<K2BASE)
#define k02k1(a)        (((ulong)(a))|K1BASE)
#define log2phy(a)	(((ulong)(a))&0x1fffffff)
#define phy2k0(a)	((a)|K0BASE)
#define phy2k1(a)	((a)|K1BASE)
#define GEN_VECT 	0x80000080
#define UTLB_VECT 	0x80000000

/* these are used by the cache flushing routines */
/* flush_cache types */
#define ICACHE  0
#define DCACHE  1
#define IADDR   2
#define ICACHEI  3	/* Icache init. On 33310 clears lock bits */
#define DCACHEI  4	/* Dcache init. On 4010 no copy-back */

#ifndef LANGUAGE_ASSEMBLY

/* ================ LANGUAGE_C ======================== */

#define C0_INDEX	0 		/* TLB INDEX   */
#define C0_RANDOM  	1 		/* TLB RANDOM  */
#define C0_ENTRYLO  	2 		/* TLB ENTRYLO */
#define C0_CONTEXT  	4 		/* PTE and BadVPN */
#define C0_WIRED  	6 		/* TLB WIRED */
#define C0_ENTRYHI  	10 		/* TLB ENTRYHI */
#define C0_SR		12		/* Processor Status */
#define C0_CAUSE	13		/* Exception Cause */
#define C0_EPC		14		/* Exception PC */
#define C0_BADADDR	8		/* Bad Address */
#define C0_BADVADDR	8		/* Bad Virtual Address */
#define C0_PRID		15		/* Processor Rev Indentifier */

/* Floating-Point Control registers */
#define FPA_CSR		31		/* Control/Status register */
#define FPA_IRR		0		/* Implementation/Rev register */
#define C1_CSR		31
#define C1_FRID		0

#else 

/* ============== LANGUAGE_ASSEMBLY */

/* break instruction codes 0..1023 */
#define BRK_BPT		0		/* breakpoint */
#define BRK_OVR		6		/* overflow */
#define BRK_DZERO	7		/* divide by zero */
#define BRK_TRAP	8		/* trap on condition */

/* aliases for general registers */
#define zero		$0
#define	AT		$1		/* assembler temporaries */
#define	v0		$2		/* value holders */
#define	v1		$3
#define	a0		$4		/* arguments */
#define	a1		$5
#define	a2		$6
#define	a3		$7
#define	t0		$8		/* temporaries */
#define	t1		$9
#define	t2		$10
#define	t3		$11
#define	t4		$12
#define	t5		$13
#define	t6		$14
#define	t7		$15
#define	s0		$16		/* saved registers */
#define	s1		$17
#define	s2		$18
#define	s3		$19
#define	s4		$20
#define	s5		$21
#define	s6		$22
#define	s7		$23
#define	t8		$24		/* temporaries */
#define	t9		$25
#define	k0		$26		/* kernel registers */
#define	k1		$27
#define	gp		$28		/* global pointer */
#define	sp		$29		/* stack pointer */
#define	s8		$30		/* saved register */
#define	fp		$30		/* frame pointer (old usage) */
#define	ra		$31		/* return address */

/* 2006-07-02 tonywu: add RADIAX register aliases */
#define m0l		$1
#define m0h 		$2
#define m0		$3
#define m1l		$5
#define m1h		$6
#define m1		$7
#define m2l		$9
#define m2h		$10
#define m2		$11
#define m3l		$13
#define m3h		$14
#define m3		$15
#define estatus		$0
#define ecause		$1
#define intvec		$2
#define cbs0		$0
#define cbs1		$1
#define cbs2		$2
#define cbe0		$4
#define cbe1		$5
#define cbe2		$6
#define lps0		$16
#define lpe0		$17
#define lpc0		$18
#define mmd		$24
/* 2006-07-02 tonywu: add RADIAX register aliases */

/* you can't use the usual register names in macros */
#define r_v0    2
#define r_v1    3
#define r_a0    4
#define r_a1    5
#define r_a2    6
#define r_a3    7
#define r_t0    8
#define r_t1    9
#define r_t2    10
#define r_t3    11
#define r_t4    12
#define r_t5    13
#define r_t6    14
#define r_t7    15
#define r_s0    16
#define r_s1    17
#define r_s2    18
#define r_s3    19
#define r_s4    20
#define r_s5    21
#define r_s6    22
#define r_s7    23

/* System Control Coprocessor (CP0) registers */
#define C0_INDEX	$0 		/* TLB INDEX   */
#define C0_RANDOM  	$1 		/* TLB RANDOM  */
#define C0_ENTRYLO  	$2 		/* TLB ENTRYLO */
#define C0_CONTEXT  	$4 		/* PTE and BadVPN */
#define C0_WIRED  	$6 		/* TLB WIRED */
#define C0_ENTRYHI  	$10 		/* TLB ENTRYHI */
#define C0_SR		$12		/* Processor Status */
#define C0_CAUSE	$13		/* Exception Cause */
#define C0_EPC		$14		/* Exception PC */
#define C0_BADADDR	$8		/* Bad Address */
#define C0_BADVADDR	$8		/* Bad Virtual Address */
#define C0_PRID		$15		/* Processor Rev Indentifier */
/* EJTAG CP0 Registers */
#define C0_DREG    	$16
#define C0_DEPC    	$17
#define C0_DESAVE  	$31
/* lexra specific CP0 Registers */
#define C0_CCTL		$20		/* Cache Control */


/* Floating-Point Control registers */
#define FPA_CSR		$31		/* Control/Status register */
#define FPA_IRR		$0		/* Implementation/Rev register */
#define C1_CSR		$31
#define C1_FRID		$0

#endif /* ================ LANGUAGE_ASSEMBLY ================ */

/* Floating-Point Control register bits */
#define CSR_C		0x00800000
#define CSR_EXC		0x0003f000
#define CSR_EE		0x00020000
#define CSR_EV		0x00010000
#define CSR_EZ		0x00008000
#define CSR_EO		0x00004000
#define CSR_EU		0x00002000
#define CSR_EI		0x00001000
#define CSR_TV		0x00000800
#define CSR_TZ		0x00000400
#define CSR_TO		0x00000200
#define CSR_TU		0x00000100
#define CSR_TI		0x00000080
#define CSR_SV		0x00000040
#define CSR_SZ		0x00000020
#define CSR_SO		0x00000010
#define CSR_SU		0x00000008
#define CSR_SI		0x00000004
#define CSR_RM		0x00000003

/* Status Register */
#define SR_CUMASK	0xf0000000	/* Coprocessor usable bits */
#define	SR_CU3		0x80000000	/* Coprocessor 3 usable */
#define SR_CU2		0x40000000	/* coprocessor 2 usable */
#define SR_CU1		0x20000000	/* Coprocessor 1 usable */
#define SR_CU0		0x10000000	/* Coprocessor 0 usable */
#define SR_BEV		0x00400000	/* Bootstrap Exception Vector */
#define SR_TS		0x00200000	/* TLB shutdown */
#define SR_PE		0x00100000	/* Parity Error */
#define SR_CM		0x00080000	/* Cache Miss */
#define SR_PZ		0x00040000	/* Parity Zero */
#define SR_SWC		0x00020000	/* Swap Caches */
#define SR_ISC		0x00010000	/* Isolate Cache */

#define SR_IMASK	0x0000ff00	/* Interrupt Mask */
#define SR_IMASK8	0x00000000	/* Interrupt Mask level=8 */
#define SR_IMASK7	0x00008000	/* Interrupt Mask level=7 */
#define SR_IMASK6	0x0000c000	/* Interrupt Mask level=6 */
#define SR_IMASK5	0x0000e000	/* Interrupt Mask level=5 */
#define SR_IMASK4	0x0000f000	/* Interrupt Mask level=4 */
#define SR_IMASK3	0x0000f800	/* Interrupt Mask level=3 */
#define SR_IMASK2	0x0000fc00	/* Interrupt Mask level=2 */
#define SR_IMASK1	0x0000fe00	/* Interrupt Mask level=1 */
#define SR_IMASK0	0x0000ff00	/* Interrupt Mask level=0 */
#define SR_ISHIFT       8

#define SR_IBIT8	0x00008000	/*  (Intr5) */
#define SR_IBIT7	0x00004000	/*  (Intr4) */
#define SR_IBIT6	0x00002000	/*  (Intr3) */
#define SR_IBIT5	0x00001000	/*  (Intr2) */
#define SR_IBIT4	0x00000800	/*  (Intr1) */
#define SR_IBIT3	0x00000400	/*  (Intr0) */
#define SR_IBIT2	0x00000200	/*  (Software Interrupt 1) */
#define SR_IBIT1	0x00000100	/*  (Software Interrupt 0) */
#define SR_INT5		0x00008000	/*  (Intr5) */
#define SR_INT4		0x00004000	/*  (Intr4) */
#define SR_INT3		0x00002000	/*  (Intr3) */
#define SR_INT2		0x00001000	/*  (Intr2) */
#define SR_INT1		0x00000800	/*  (Intr1) */
#define SR_INT0		0x00000400	/*  (Intr0) */

#define SR_KUO		0x00000020	/* Kernel/User mode, old */
#define SR_IEO		0x00000010	/* Interrupt Enable, old */
#define SR_KUP		0x00000008	/* Kernel/User mode, previous */
#define SR_IEP		0x00000004	/* Interrupt Enable, previous */
#define SR_KUC		0x00000002	/* Kernel/User mode, current */
#define SR_IEC		0x00000001	/* Interrupt Enable, current */

/* Cause Register */
#define CAUSE_BD		0x80000000	/* Branch Delay */
#define CAUSE_CEMASK		0x30000000	/* Coprocessor Error */
#define CAUSE_CESHIFT		28		/* Right justify CE  */
#define CAUSE_IPMASK		0x0000ff00	/* Interrupt Pending */
#define CAUSE_IPSHIFT		8		/* Right justify IP  */
#define CAUSE_IP8		0x00008000	/*  (Intr5) */
#define CAUSE_IP7		0x00004000	/*  (Intr4) */
#define CAUSE_IP6		0x00002000	/*  (Intr3) */
#define CAUSE_IP5		0x00001000	/*  (Intr2) */
#define CAUSE_IP4		0x00000800	/*  (Intr1) */
#define CAUSE_IP3		0x00000400	/*  (Intr0) */
#define CAUSE_INT5		0x00008000	/*  (Intr5) */
#define CAUSE_INT4		0x00004000	/*  (Intr4) */
#define CAUSE_INT3		0x00002000	/*  (Intr3) */
#define CAUSE_INT2		0x00001000	/*  (Intr2) */
#define CAUSE_INT1		0x00000800	/*  (Intr1) */
#define CAUSE_INT0		0x00000400	/*  (Intr0) */
#define CAUSE_SW2		0x00000200	/*  (Software Int 1) */
#define CAUSE_SW1		0x00000100	/*  (Software Int 0) */
#define CAUSE_EXCMASK		0x0000003c	/* Exception Code */
#define CAUSE_EXCSHIFT		2		/* Right justify EXC */

/* Exception Code */
#define EXC_INT		(0 << 2)	/* External interrupt */
#define EXC_MOD		(1 << 2)	/* TLB modification */
#define EXC_TLBL	(2 << 2)    	/* TLB miss (Load or Ifetch) */
#define EXC_TLBS	(3 << 2)	/* TLB miss (Save) */
#define EXC_ADEL	(4 << 2)    	/* Addr error (Load or Ifetch) */
#define EXC_ADES	(5 << 2)	/* Address error (Save) */
#define EXC_IBE		(6 << 2)	/* Bus error (Ifetch) */
#define EXC_DBE		(7 << 2)	/* Bus error (data load/store) */
#define EXC_SYS		(8 << 2)	/* System call */
#define EXC_BP		(9 << 2)	/* Break point */
#define EXC_RI		(10 << 2)	/* Reserved instruction */
#define EXC_CPU		(11 << 2)	/* Coprocessor unusable */
#define EXC_OVF		(12 << 2)	/* Arithmetic overflow */
#define EXC_TR          (13 << 2)       /* Trap instruction */

/* FPU stuff */
#define CSR_EMASK	(0x3f<<12)
#define CSR_TMASK	(0x1f<<7)
#define CSR_SMASK	(0x1f<<2)

#endif /* __REGS_H_INCLUDED */

