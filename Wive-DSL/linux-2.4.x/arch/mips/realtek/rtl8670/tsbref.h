/* $Id: tsbref.h,v 1.1 2005/12/07 12:30:11 kaohj Exp $
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1999  TOSHIBA Corporation
 *
 *   Rev 1.0  : Initial Release                     Nov 04, 1999
 *   Rev 1.01 : Change the management of            Dec 13, 1999
 *               the BOOT parameter
 *   Rev 1.02 : Add dbg_gets prototype              Dec 16, 1999
 */

#ifndef _LANGUAGE_ASSEMBLY
#include <linux/console.h>
#endif

//#include <asm/pc87338.h>
//#include <asm/tx39.h>

#ifndef __ASM_MIPS_TSBREF_H
#define __ASM_MIPS_TSBREF_H

#define TSBDBG                  1  /* XXX */

#define TSBREF_DRAM_SIZE        0x02000000

#define TSBREF_ROMKRN_START     0x9f000000
#define TSBREF_RAMKRN_START     0x80080000

#define TX39_REGS_PHYSBASE      0x10c00000

#define TSBREF_INTR_MAXIRQ      16
#define GVPDA_INTR_MAXIRQ      16
#define TX39_MAXINTR_REG        6
#define TX3912_MAXINTR_REG      4
#define TX3911_MAXINTR_REG      6

/*
 *  TSBREF IRQ
 */
#define TSBREF_TIMER_IRQ        0     /* timer */
#define TSBREF_PWR_IRQ          1     /* Power */
#define TSBREF_AFC_IRQ		2    /* Analog Front End IRQ */
#define TSBREF_USB_DETEST_IRQ	3    /* USB Plugin detect IRQ  */   // colin : 20011009
#define TSBREF_SND_IRQ		4     /* Sound Codec DMA IRQ */
#define TSBREF_SER_IRQ          5     /* Serial */
#define TSBREF_KBD_IRQ          6     /* Keyboard */
#define TSBREF_MOS_IRQ          7     /* Mouse */
#define TSBREF_PCMCIA1_IRQ      8     /* PCMCIA Slot 1 */
#define TSBREF_PCMCIA2_IRQ      9     /* PCMCIA Slot 2 */
#define TSBREF_SFMDP_IRQ	10    /* SoftModem Data Pump (Codec DMA) IRQ */
#define TSBREF_SIB_IRQ		11    /* Sib SubFrame 0/1 IRQ */
#define TSBREF_RTC_IRQ		12    /* RTC and ALARM IRQ */   // colin : 20010411
#define TSBREF_BUTTON_IRQ	13    /* Gpio Button  */   // colin : 20010423
#define TSBREF_IRDA_IRQ		14    /* IRDA IRQ  */   // colin : 20010423
#define TSBREF_SFMRI_IRQ	15     /* SoftModem Ring Indicator IRQ */
/*
 *  Debug Board Function
 */

#define DEBUG_SUPERIO_BASE      0xb0130000
#define DEBUG_SUPERIO_PHYSBASE  0x10130000
#define DEBUG_ETHER_BASE        0xb0120000
#define DEBUG_ETHER_PHYSBASE    0x10120000
#define DEBUG_HDSP_BASE         0xb0110000
#define DEBUG_DIPSW             0xb0100000
#define DEBUG_LED_BASE          0xb0100000

#define SMC_HARD_RESET_REG      (DEBUG_ETHER_BASE + 32)

#define OFF_SUPERIO_INDEX 0x398
#define OFF_SUPERIO_DATA 0x399
#define OFF_PARALLEL_BASE 0x3bc
#define OFF_SERIAL_BASE 0x3f8
#define SUPERIO_BAUD SERIAL_DL_B38400

#define DEBUG_SUPERIO_INDEX (DEBUG_SUPERIO_BASE + OFF_SUPERIO_INDEX)
#define DEBUG_SUPERIO_DATA  (DEBUG_SUPERIO_BASE + OFF_SUPERIO_DATA)

#define DEBUG_PARALLEL_BASE (DEBUG_SUPERIO_BASE + OFF_PARALLEL_BASE)
#define DEBUG_PARALLEL_DTR  (DEBUG_PARALLEL_BASE + OFF_PARALLEL_DTR)
#define DEBUG_PARALLEL_STR  (DEBUG_PARALLEL_BASE + OFF_PARALLEL_STR)
#define DEBUG_PARALLEL_CTR  (DEBUG_PARALLEL_BASE + OFF_PARALLEL_CTR)

#define DEBUG_PARALLEL_DATAR (DEBUG_PARALLEL_BASE + OFF_PARALLEL_DATAR)
#define DEBUG_PARALLEL_AFIFO (DEBUG_PARALLEL_BASE + OFF_PARALLEL_AFIFO)
#define DEBUG_PARALLEL_DSR   (DEBUG_PARALLEL_BASE + OFF_PARALLEL_DSR)
#define DEBUG_PARALLEL_DCR   (DEBUG_PARALLEL_BASE + OFF_PARALLEL_DCR)
#define DEBUG_PARALLEL_CFIFO (DEBUG_PARALLEL_BASE + OFF_PARALLEL_CFIFO)
#define DEBUG_PARALLEL_DFIFO (DEBUG_PARALLEL_BASE + OFF_PARALLEL_DFIFO)
#define DEBUG_PARALLEL_TFIFO (DEBUG_PARALLEL_BASE + OFF_PARALLEL_TFIFO)
#define DEBUG_PARALLEL_CNFGA (DEBUG_PARALLEL_BASE + OFF_PARALLEL_CNFGA)
#define DEBUG_PARALLEL_CNFGB (DEBUG_PARALLEL_BASE + OFF_PARALLEL_CNFGB)
#define DEBUG_PARALLEL_ECR   (DEBUG_PARALLEL_BASE + OFF_PARALLEL_ECR)

#define DEBUG_SERIAL_BASE    (DEBUG_SUPERIO_BASE + OFF_SERIAL_BASE)
#define DEBUG_SERIAL_RBR     (DEBUG_SERIAL_BASE + OFF_SERIAL_RBR)
#define DEBUG_SERIAL_THR     (DEBUG_SERIAL_BASE + OFF_SERIAL_THR)
#define DEBUG_SERIAL_IER     (DEBUG_SERIAL_BASE + OFF_SERIAL_IER)
#define DEBUG_SERIAL_IIR     (DEBUG_SERIAL_BASE + OFF_SERIAL_IIR)
#define DEBUG_SERIAL_FCR     (DEBUG_SERIAL_BASE + OFF_SERIAL_FCR)
#define DEBUG_SERIAL_LCR     (DEBUG_SERIAL_BASE + OFF_SERIAL_LCR)
#define DEBUG_SERIAL_MCR     (DEBUG_SERIAL_BASE + OFF_SERIAL_MCR)
#define DEBUG_SERIAL_LSR     (DEBUG_SERIAL_BASE + OFF_SERIAL_LSR)
#define DEBUG_SERIAL_MSR     (DEBUG_SERIAL_BASE + OFF_SERIAL_MSR)
#define DEBUG_SERIAL_SCR     (DEBUG_SERIAL_BASE + OFF_SERIAL_SCR)
                                
#define DEBUG_SERIAL_DLL     (DEBUG_SERIAL_BASE + OFF_SERIAL_DLL)
#define DEBUG_SERIAL_DLM     (DEBUG_SERIAL_BASE + OFF_SERIAL_DLM)

/*
 *  KeyBoard
 */
#define KBD_DATA_PHYSBASE    (0x10000003)
#define KBD_STATUS_PHYSBASE  (0x10000007) 

/*
 * Functions
 */
//#ifndef _LANGUAGE_ASSEMBLY

void   init_dbgbd(void);
void    dbg_putchar(unsigned char);
int     dbg_getchar(void);
char *  dbg_gets(char *, int);
void    dbg_printf(char *fmt, ...);

/*
 *  RTC definitions
 */
#ifdef	CONFIG_GVPDA_TX3911
#define MstoRTC(ms)             ((ms)*32764)
#else
#define MstoRTC(ms)             ((ms)*33)
#endif

/*
 *  Other Macro
 */
#define IS_TX3922(cputype)   (cputype == CPU_TX3922)
#define IS_TX3911(cputype)   (cputype == CPU_TX3911)
#define IS_TX3912(cputype)   (cputype == CPU_TX3912)
#define PERTIME2USEC(a)      ((a*100/144))


#if	0	/* DJH */
#define READ_TX39REG(off)       *(int *)(KSEG1ADDR(TX39_REGS_PHYSBASE) + off)
#define WRITE_TX39REG(val, off) *(int *)(KSEG1ADDR(TX39_REGS_PHYSBASE + off))=val
#define OR_TX39REG(val, off)    WRITE_TX39REG(READ_TX39REG(off) | (val), off)
#define AND_TX39REG(val, off)   WRITE_TX39REG(READ_TX39REG(off) & (val), off)
#else
#define READ_TX39REG(off)       inl(TX39_REGS_PHYSBASE + off)
#define WRITE_TX39REG(val, off) outl(val, TX39_REGS_PHYSBASE + off)
#define OR_TX39REG(val, off)    WRITE_TX39REG(READ_TX39REG(off) | (val), off)
#define AND_TX39REG(val, off)   WRITE_TX39REG(READ_TX39REG(off) & (val), off)
#endif
//#endif


/*
 *  Other Definitions
 */
#define TIMER_BLINK 
#define LED8  0
#define HDSP  1


/*
 *  Define GVPDA device driver major number 	colin : 20010411
 */

#define GVPDA_VOLTAGE_MAJOR 		254
#define GVPDA_GVMISC_MAJOR 		253
#define GVPDA_SPP_MAJOR                 252
#define GVPDA_USB_MAJOR                 251
#define GVPDA_AT88SC153_MAJOR           250
#define GVPDA_DEBUG_MAJOR 		200

#endif /* __ASM_MIPS_TSBREF_H */
