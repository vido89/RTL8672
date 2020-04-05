/* v96xpbc.h,v 1.2 2000/09/27 02:30:58 chris Exp
 *
 * v96xpbc.h
 *
 * Kevin D. Kissell, kevink@mips.com and Carsten Langgaard, carstenl@mips.com
 * Copyright (C) 1999 MIPS Technologies, Inc.  All rights reserved.
 * Chris Dearman, chris@algor.co.uk
 * Copyright (C) 2001 Algorithmics Ltd.  All rights reserved.
 *
 * ########################################################################
 *
 *  This program is free software; you can distribute it and/or modify it
 *  under the terms of the GNU General Public License (Version 2) as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * ########################################################################
 *
 * i960 to PCI bridge controller
 *
 */

#ifndef _ALGOR_V96XPBC_H
#define _ALGOR_V96XPBC_H

#ifdef __ASSEMBLER__

/* offsets from base register */
#if defined(__MIPSEL__)
#define V96XW(x)		(x)
#define V96XH(x)		(x)
#define V96XB(x)		(x)
#elif defined(__MIPSEB__)
#define V96XW(x)		(x)
#define V96XH(x)		((x)^2)
#define V96XB(x)		((x)^3)
#else
#error "MIPS but neither __MIPSEL__ nor __MIPSEB__ defined."
#endif

#else /* !__ASSEMBLER */

/* offsets from base pointer, this construct allows optimisation */
/* static char * const _v96xp = PA_TO_KVA1(V96XPBC_BASE); */

#if defined(__MIPSEL__)
#define V96XW(x)		*(volatile unsigned long *)(_v96xp + (x))
#define V96XH(x)		*(volatile unsigned short *)(_v96xp + (x))
#define V96XB(x)		*(volatile unsigned char *)(_v96xp + (x))
#elif defined(__MIPSEB__)
#define V96XW(x)		*(volatile unsigned long *)(_v96xp + (x))
#define V96XH(x)		*(volatile unsigned short *)(_v96xp + ((x)^2))
#define V96XB(x)		*(volatile unsigned char *)(_v96xp + ((x)^3))
#else
#error "MIPS but neither __MIPSEL__ nor __MIPSEB__ defined."
#endif

#endif /* __ASSEMBLER__ */

#define V96X_PCI_VENDOR		V96XH(0x00)
#define V96X_PCI_DEVICE		V96XH(0x02)
#define V96X_PCI_CMD		V96XH(0x04)
#define V96X_PCI_STAT		V96XH(0x06)
#define V96X_PCI_CC_REV		V96XW(0x08)
#define V96X_PCI_I2O_BASE	V96XW(0x10) /* B.2 only */
#define V96X_PCI_HDR_CFG	V96XW(0x0c)	
#define V96X_PCI_IO_BASE	V96XW(0x10)
#define V96X_PCI_BASE0		V96XW(0x14)
#define V96X_PCI_BASE1		V96XW(0x18)
#define V96X_PCI_BPARAM		V96XW(0x3c)
#define V96X_PCI_MAP0		V96XW(0x40)
#define V96X_PCI_MAP1		V96XW(0x44)
#define V96X_PCI_INT_STAT	V96XW(0x48)
#define V96X_PCI_INT_CFG	V96XW(0x4c)
#define V96X_LB_BASE0		V96XW(0x54)
#define V96X_LB_BASE1		V96XW(0x58)
#define V96X_LB_MAP0		V96XH(0x5e)
#define V96X_LB_MAP1		V96XH(0x62)
#define V96X_LB_BASE2		V96XH(0x64) /* B.2 only */
#define V96X_LB_MAP2		V96XH(0x66) /* B.2 only */
#define V96X_LB_SIZE		V96XW(0x68) /* B.2 only */
#define V96X_LB_IO_BASE		V96XW(0x6c)
#define V96X_FIFO_CFG		V96XH(0x70)
#define V96X_FIFO_PRIORITY	V96XH(0x72)
#define V96X_FIFO_STAT		V96XH(0x74)
#define V96X_LB_ISTAT		V96XB(0x76)
#define V96X_LB_IMASK		V96XB(0x77)
#define V96X_SYSTEM		V96XH(0x78)
#define V96X_LB_CFGL		V96XB(0x7a)
#define V96X_LB_CFG		V96XB(0x7b)
#define V96X_PCI_CFG		V96XH(0x7c) /* B.2 only */
#define V96X_DMA_PCI_ADDR0	V96XW(0x80)
#define V96X_DMA_LOCAL_ADDR0	V96XW(0x84)
#define V96X_DMA_LENGTH0	V96XW(0x88)
#define V96X_DMA_CTLB_ADR0	V96XW(0x8c)
#define V96X_DMA_PCI_ADDR1	V96XW(0x90)
#define V96X_DMA_LOCAL_ADDR1	V96XW(0x94)
#define V96X_DMA_LENGTH1	V96XW(0x98)
#define V96X_DMA_CTLB_ADR1	V96XW(0x9c)
#define V96X_MAIL_DATA(n)	V96XB(0xc0+(n))
#define V96X_LB_MAIL_IEWR	V96XH(0xd0)
#define V96X_LB_MAIL_IERD	V96XH(0xd2)
#define V96X_PCI_MAIL_IEWR	V96XH(0xd4)
#define V96X_PCI_MAIL_IERD	V96XH(0xd6)
#define V96X_MAIL_WR_STAT	V96XH(0xd8)
#define V96X_MAIL_RD_STAT	V96XH(0xdc)

#define V96X_PCI_CMD_FBB_EN		0x0200
#define V96X_PCI_CMD_SERR_EN		0x0100
#define V96X_PCI_CMD_PAR_EN		0x0040
#define V96X_PCI_CMD_MASTER_EN		0x0004
#define V96X_PCI_CMD_MEM_EN		0x0002
#define V96X_PCI_CMD_IO_EN		0x0001

#define V96X_PCI_STAT_PAR_ERR		0x8000
#define V96X_PCI_STAT_SYS_ERR		0x4000
#define V96X_PCI_STAT_M_ABORT		0x2000
#define V96X_PCI_STAT_T_ABORT		0x1000
#define V96X_PCI_STAT_DEVSEL		0x0600
#define V96X_PCI_STAT_PAR_REP		0x0100
#define V96X_PCI_STAT_FAST_BACK		0x0080

#define V96X_PCI_CC_REV_BASE_CLASS	0xff000000
#define V96X_PCI_CC_REV_SUB_CLASS	0x00ff0000
#define V96X_PCI_CC_REV_PROG_IF		0x0000ff00
#define V96X_PCI_CC_REV_UREV		0x000000f0
#define V96X_PCI_CC_REV_VREV		0x0000000f

#define V96X_VREV_A	0x0
#define V96X_VREV_B0	0x1
#define V96X_VREV_B1	0x2
#define V96X_VREV_B2	0x3
#define V96X_VREV_C0	0x4

#define V96X_PCI_HDR_CFG_LT		0x0000ff00
#define V96X_PCI_HDR_CFG_LT_SHIFT	8
#define V96X_PCI_HDR_CFG_CLS		0x000000ff
#define V96X_PCI_HDR_CFG_CLS_SHIFT	0

/* pci access to internal v96xpbc registers */
#define V96X_PCI_IO_BASE_ADR_BASE	0xfffffff0
#define V96X_PCI_IO_BASE_PREFETCH	0x00000008
#define V96X_PCI_IO_BASE_TYPE		0x00000006
#define V96X_PCI_IO_BASE_IO		0x00000001
#define V96X_PCI_IO_BASE_MEM		0x00000000

/* pci to local bus aperture 0 base address */
#define V96X_PCI_BASE0_ADR_BASE		0xfff00000
#define V96X_PCI_BASE0_ADR_BASEL	0x000fff00

/* pci to local bus aperture 1 base address */
#define V96X_PCI_BASE1_ADR_BASE		0xfff00000
#define V96X_PCI_BASE1_ADR_BASEL	0x000fc000
#define V96X_PCI_BASE1_ADR_DOS_MEM	0x00000700

#define V96X_PCI_BASEx_PREFETCH		0x00000008
#define V96X_PCI_BASEx_IO		0x00000001
#define V96X_PCI_BASEx_MEM		0x00000000

/* pci bus parameter register */
#define V96X_PCI_BPARAM_MAX_LAT		0xff000000
#define V96X_PCI_BPARAM_MIN_GNT		0x00ff0000
#define V96X_PCI_BPARAM_INT_PIN		0x00000700
#define V96X_PCI_BPARAM_INT_LINE	0x0000000f

/* pci bus to local bus address map 0 */
#define V96X_PCI_MAPx_MAP_ADR		0xfff00000
#define V96X_PCI_MAPx_RD_POST_INH	0x00008000
#define V96X_PCI_MAP0_ROM_SIZE		0x00000c00
#define V96X_PCI_MAPx_SWAP		0x00000300
#define V96X_PCI_MAPx_ADR_SIZE		0x000000f0
#define V96X_PCI_MAPx_REG_EN		0x00000002
#define V96X_PCI_MAPx_ENABLE		0x00000001

#define V96X_ADR_SIZE_1MB		(0x0<<4)
#define V96X_ADR_SIZE_2MB		(0x1<<4)
#define V96X_ADR_SIZE_4MB		(0x2<<4)
#define V96X_ADR_SIZE_8MB		(0x3<<4)
#define V96X_ADR_SIZE_16MB		(0x4<<4)
#define V96X_ADR_SIZE_32MB		(0x5<<4)
#define V96X_ADR_SIZE_64MB		(0x6<<4)
#define V96X_ADR_SIZE_128MB		(0x7<<4)
#define V96X_ADR_SIZE_256MB		(0x8<<4)
#define V96X_ADR_SIZE_DOSMODE		(0xc<<4)

#define V96X_SWAP_NONE			(0x0<<8)
#define V96X_SWAP_16BIT			(0x1<<8)
#define V96X_SWAP_8BIT			(0x2<<8)
#define V96X_SWAP_AUTO			(0x3<<8)

/* pci interrupt status register */
#define V96X_PCI_INT_STAT_MAILBOX	0x80000000
#define V96X_PCI_INT_STAT_LOCAL		0x40000000
#define V96X_PCI_INT_STAT_DMA1		0x02000000
#define V96X_PCI_INT_STAT_DMA0		0x01000000
#define V96X_PCI_INT_STAT_INTC_TO_D	0x00004000
#define V96X_PCI_INT_STAT_INTB_TO_D	0x00002000
#define V96X_PCI_INT_STAT_INTA_TO_D	0x00001000
#define V96X_PCI_INT_STAT_INTD_TO_C	0x00000800
#define V96X_PCI_INT_STAT_INTB_TO_C	0x00000200
#define V96X_PCI_INT_STAT_INTA_TO_C	0x00000100
#define V96X_PCI_INT_STAT_INTD_TO_B	0x00000080
#define V96X_PCI_INT_STAT_INTC_TO_B	0x00000040
#define V96X_PCI_INT_STAT_INTA_TO_B	0x00000010
#define V96X_PCI_INT_STAT_INTD_TO_A	0x00000008
#define V96X_PCI_INT_STAT_INTC_TO_A	0x00000004
#define V96X_PCI_INT_STAT_INTB_TO_A	0x00000002

/* pci interrupt config register */
#define V96X_PCI_INT_CFG_MAILBOX	0x80000000
#define V96X_PCI_INT_CFG_LOCAL		0x40000000
#define V96X_PCI_INT_CFG_DMA1		0x02000000
#define V96X_PCI_INT_CFG_DMA0		0x01000000
#define V96X_PCI_INT_CFG_MODE_D		0x00c00000
#define V96X_PCI_INT_CFG_MODE_D_SHIFT	22
#define V96X_PCI_INT_CFG_MODE_C		0x00300000
#define V96X_PCI_INT_CFG_MODE_C_SHIFT	20
#define V96X_PCI_INT_CFG_MODE_B		0x000c0000
#define V96X_PCI_INT_CFG_MODE_B_SHIFT	18
#define V96X_PCI_INT_CFG_MODE_A		0x00030000
#define V96X_PCI_INT_CFG_MODE_A_SHIFT	16
#define  V96X_PCI_INT_CFG_MODE_LEVEL	 0x0
#define  V96X_PCI_INT_CFG_MODE_EDGE	 0x1
#define  V96X_PCI_INT_CFG_MODE_SWCLR	 0x2
#define  V96X_PCI_INT_CFG_MODE_HWCLR	 0x3
#define V96X_PCI_INT_CFG_INTD_TO_LB	0x00008000
#define V96X_PCI_INT_CFG_INTC_TO_D	0x00004000
#define V96X_PCI_INT_CFG_INTB_TO_D	0x00002000
#define V96X_PCI_INT_CFG_INTA_TO_D	0x00001000
#define V96X_PCI_INT_CFG_INTD_TO_C	0x00000800
#define V96X_PCI_INT_CFG_INTC_TO_LB	0x00000400
#define V96X_PCI_INT_CFG_INTB_TO_C	0x00000200
#define V96X_PCI_INT_CFG_INTA_TO_C	0x00000100
#define V96X_PCI_INT_CFG_INTD_TO_B	0x00000080
#define V96X_PCI_INT_CFG_INTC_TO_B	0x00000040
#define V96X_PCI_INT_CFG_INTB_TO_LB	0x00000020
#define V96X_PCI_INT_CFG_INTA_TO_B	0x00000010
#define V96X_PCI_INT_CFG_INTD_TO_A	0x00000008
#define V96X_PCI_INT_CFG_INTC_TO_A	0x00000004
#define V96X_PCI_INT_CFG_INTB_TO_A	0x00000002
#define V96X_PCI_INT_CFG_INTA_TO_LB	0x00000001

/* local bus to pci bus aperture 0,1 */
#define V96X_LB_BASEx_ADR_BASE		0xfff00000
#define V96X_LB_BASEx_SWAP		0x00000300
#define V96X_LB_BASEx_ADR_SIZE		0x000000f0
#define V96X_LB_BASEx_PREFETCH		0x00000008
#define V96X_LB_BASEx_ENABLE		0x00000001

/* local bus to pci bus address map 0,1 */
#define V96X_LB_MAPx_MAP_ADR		0xfff0
#define V96X_LB_MAPx_TYPE		0x0007
#define  V96X_LB_TYPE_IACK		 (0x0<<1)
#define  V96X_LB_TYPE_IO		 (0x1<<1)
#define  V96X_LB_TYPE_MEM		 (0x3<<1)
#define  V96X_LB_TYPE_CONF		 (0x5<<1)
#define V96X_LB_MAPx_AD_LOW_EN		0x0001 /* C.0 only */

/* local bus interrupt control, status and masks */
#define V96X_LB_INTR_MAILBOX		0x80
#define V96X_LB_INTR_PCI_RD		0x40
#define V96X_LB_INTR_PCI_WR		0x20
#define V96X_LB_INTR_PCI_INT		0x10
#define V96X_LB_INTR_DMA1		0x02
#define V96X_LB_INTR_DMA0		0x01

/* local bus configuration */
#define V96X_LB_CFG_TO_MASK		0x60	/* Rev C.0 */
#define V96X_LB_CFG_TO_1024		0x60	/* Rev C.0 */
#define V96X_LB_CFG_TO_512		0x40	/* Rev C.0 */
#define V96X_LB_CFG_TO_256		0x20
#define V96X_LB_CFG_TO_64		0x00
#define V96X_LB_CFG_LB_INT		0x04
#define V96X_LB_CFG_ERR_EN		0x02
#define V96X_LB_CFG_RDY_EN		0x01

/* PCI bus configuration */
#define V96X_PCI_CFG_I2O_EN		0x8000
#define V96X_PCI_CFG_IO_REG_DIS		0x4000
#define V96X_PCI_CFG_IO_DIS		0x2000
#define V96X_PCI_CFG_EN3V		0x1000
#define V96X_PCI_CFG_AD_LOW		0x0300
#define V96X_PCI_CFG_AD_LOW_SHIFT	8
#define V96X_PCI_CFG_DMA_RTYPE		0x00e0
#define V96X_PCI_CFG_DMA_WTYPE		0x000e

/* fifo configuration register */ 
#define V96X_FIFO_CFG_PBRST_MAX		0xc000
#define V96X_FIFO_CFG_PBRST_MAX_SHIFT	14
#define V96X_FIFO_CFG_WR_LB		0x3000
#define V96X_FIFO_CFG_WR_LB_SHIFT	12
#define V96X_FIFO_CFG_RD_LB1		0x0c00
#define V96X_FIFO_CFG_RD_LB1_SHIFT	10
#define V96X_FIFO_CFG_RD_LB0		0x0300
#define V96X_FIFO_CFG_RD_LB0_SHIFT	8
#define V96X_FIFO_CFG_LBRST_MAX		0x00c0
#define V96X_FIFO_CFG_LBRST_MAX_SHIFT	6
#define V96X_FIFO_CFG_WR_PCI		0x0030
#define V96X_FIFO_CFG_WR_PCI_SHIFT	4
#define V96X_FIFO_CFG_RD_PCI1		0x000c
#define V96X_FIFO_CFG_RD_PCI1_SHIFT	2
#define V96X_FIFO_CFG_RD_PCI0		0x0003
#define V96X_FIFO_CFG_RD_PCI0_SHIFT	0

/* meaning of above bitfields */

/* max burst length */
#define V96X_FIFO_CFG_BRST_4			0x0
#define V96X_FIFO_CFG_BRST_8			0x1
#define V96X_FIFO_CFG_BRST_16			0x2
#define V96X_FIFO_CFG_BRST_256			0x3

/* when to start refilling read fifo */
#define V96X_FIFO_CFG_RD_NOTFULL		0x0
#define V96X_FIFO_CFG_RD_HALF			0x1
#define V96X_FIFO_CFG_RD_EMPTY			0x2

/* when to start emptying write fifo */
#define V96X_FIFO_CFG_WR_NOTEMPTY		0x0
#define V96X_FIFO_CFG_WR_3WORDS			0x2
#define V96X_FIFO_CFG_WR_ENDBRST		0x3

/* fifo priority control */
#define V96X_FIFO_PRIORITY_LOCAL_RD	0x1000
#define V96X_FIFO_PRIORITY_LOCAL_WR	0x0000
#define V96X_FIFO_PRIORITY_LB_RD1	0x0c00
#define V96X_FIFO_PRIORITY_LB_RD1_SHIFT	10
#define V96X_FIFO_PRIORITY_LB_RD0	0x0300
#define V96X_FIFO_PRIORITY_LB_RD0_SHIFT 8
#define V96X_FIFO_PRIORITY_PCI_RD	0x0010
#define V96X_FIFO_PRIORITY_PCI_WR	0x0000
#define V96X_FIFO_PRIORITY_PCI_RD1	0x000c
#define V96X_FIFO_PRIORITY_PCI_RD1_SHIFT 2
#define V96X_FIFO_PRIORITY_PCI_RD0	0x0003
#define V96X_FIFO_PRIORITY_PCI_RD0_SHIFT 0

/* meaning of above bitfields */
#define V96X_FIFO_PRI_NOFLUSH			0x0
#define V96X_FIFO_PRI_FLUSHBURST		0x1 /* Rev C.0 */
#define V96X_FIFO_PRI_FLUSHME			0x2
#define V96X_FIFO_PRI_FLUSHALL			0x3

/* fifo status */
#define V96X_FIFO_STAT_L2P_WR		0x3000
#define V96X_FIFO_STAT_L2P_RD1		0x0c00
#define V96X_FIFO_STAT_L2P_RD0		0x0300
#define V96X_FIFO_STAT_P2L_WR		0x0030
#define V96X_FIFO_STAT_P2L_RD1		0x000c
#define V96X_FIFO_STAT_P2L_RD0		0x0003

#define V96X_DMA_COUNT_CHAIN		0x80000000
#define V96X_DMA_COUNT_PRIORITY		0x20000000
#define V96X_DMA_COUNT_P2L		0x10000000
#define V96X_DMA_COUNT_SWAP		0x0c000000
#define V96X_DMA_COUNT_ABORT		0x02000000
#define V96X_DMA_COUNT_DMA_IPR		0x01000000

#define V96X_SYSTEM_RST_OUT		0x8000
#define V96X_SYSTEM_LOCK		0x4000
#define V96X_SYSTEM_SPROM_EN		0x2000
#define V96X_SYSTEM_SCL			0x1000
#define V96X_SYSTEM_SDA_OUT		0x0800
#define V96X_SYSTEM_SDA_IN		0x0400
#define V96X_SYSTEM_POE			0x0200
#define V96X_SYSTEM_LB_RD_PCI1		0x0040
#define V96X_SYSTEM_LB_RD_PCI0		0x0020
#define V96X_SYSTEM_LB_WR_PCI		0x0010
#define V96X_SYSTEM_PCI_RD_LB1		0x0004
#define V96X_SYSTEM_PCI_RD_LB0		0x0002
#define V96X_SYSTEM_PC_WR_LBI		0x0001

#endif /* #ifndef _ALGOR_V96XPBC_H */
