/*
 * Copyright c                Realtek Semiconductor Corporation, 2002
 * All rights reserved.                                                    
 * 
 * $Header: /usr/local/dslrepos/uClinux-dist/user/boa/src/LINUX/rtl_flashdrv.h,v 1.1 2005/08/29 08:16:20 kaohj Exp $
 *
 * $Author: kaohj $
 *
 * Abstract:
 *
 *   Flash driver header file for export include.
 *
 * $Log: rtl_flashdrv.h,v $
 * Revision 1.1  2005/08/29 08:16:20  kaohj
 * port from goahead to boa
 *
 * Revision 1.1.1.1  2005/08/15 13:56:31  hjen
 * initial import int CVS
 *
 * Revision 1.2  2005/07/12 08:38:23  kaohj
 * support auto-detect flash
 *
 * Revision 1.1  2003/10/01 09:08:42  dicktam
 * *** empty log message ***
 *
 * Revision 1.3  2003/06/23 11:11:04  elvis
 * change include path of  rtl_types.h
 *
 * Revision 1.2  2003/05/20 08:52:24  elvis
 * change the include path of "rtl_types.h"
 *
 * Revision 1.1  2003/04/29 14:17:05  orlando
 * flashdrv module initial check-in (used by cfgmgr), ported from srcroot.
 *
 * Revision 1.1  2002/07/19 08:39:33  danwu
 * Create file.
 *
 *
 * 
 */

#ifndef _FLASHDRV_H_
#define _FLASHDRV_H_



#include "rtl_types.h"
#include "rtl_board.h"
#include "rtl_flashdrv_api.h"



/* Command Definitions 
*/
#define AM29LVXXX_COMMAND_ADDR1         ((volatile uint16 *) (FLASH_BASE + 0x555 * 2))
#define AM29LVXXX_COMMAND_ADDR2         ((volatile uint16 *) (FLASH_BASE + 0x2AA * 2))
#define AM29LVXXX_COMMAND1              0xAA
#define AM29LVXXX_COMMAND2              0x55
#define AM29LVXXX_SECTOR_ERASE_CMD1     0x80
#define AM29LVXXX_SECTOR_ERASE_CMD2     0x30
#define AM29LVXXX_PROGRAM_CMD           0xA0



/* Flash Device Specific Definitions
*/


// Kaohj
#define		BLOCK_NUM_1M	19
#define		BLOCK_NUM_2M	35
#define		BLOCK_NUM_4M	71

static const uint32 blockOffset_1M[BLOCK_NUM_1M+1] ={
    0x000000L ,0x004000L ,0x006000L ,0x008000L ,0x010000L ,0x020000L ,0x030000L ,0x040000L ,
    0x050000L ,0x060000L ,0x070000L ,0x080000L ,0x090000L ,0x0A0000L ,0x0B0000L ,0x0C0000L ,
    0x0D0000L ,0x0E0000L ,0x0F0000L ,0x100000L
};

static const uint32 blockOffset_2M[BLOCK_NUM_2M+1] ={
    0x000000L ,0x004000L ,0x006000L ,0x008000L ,0x010000L ,0x020000L ,0x030000L ,0x040000L ,
    0x050000L ,0x060000L ,0x070000L ,0x080000L ,0x090000L ,0x0A0000L ,0x0B0000L ,0x0C0000L ,
    0x0D0000L ,0x0E0000L ,0x0F0000L ,0x100000L ,0x110000L ,0x120000L ,0x130000L ,0x140000L ,
    0x150000L ,0x160000L ,0x170000L ,0x180000L ,0x190000L ,0x1A0000L ,0x1B0000L ,0x1C0000L ,
    0x1D0000L ,0x1E0000L ,0x1F0000L ,0x200000L
};

static const uint32 blockOffset_4M[BLOCK_NUM_4M+1] ={
    0x000000L ,0x002000L ,0x004000L ,0x006000L ,0x008000L ,0x00A000L ,0x00C000L ,0x00E000L ,
    0x010000L ,0x020000L ,0x030000L ,0x040000L ,0x050000L ,0x060000L ,0x070000L ,0x080000L ,
    0x090000L ,0x0A0000L ,0x0B0000L ,0x0C0000L ,0x0D0000L ,0x0E0000L ,0x0F0000L ,0x100000L ,
    0x110000L ,0x120000L ,0x130000L ,0x140000L ,0x150000L ,0x160000L ,0x170000L ,0x180000L ,
    0x190000L ,0x1A0000L ,0x1B0000L ,0x1C0000L ,0x1D0000L ,0x1E0000L ,0x1F0000L ,0x200000L ,
    0x210000L ,0x220000L ,0x230000L ,0x240000L ,0x250000L ,0x260000L ,0x270000L ,0x280000L ,
    0x290000L ,0x2A0000L ,0x2B0000L ,0x2C0000L ,0x2D0000L ,0x2E0000L ,0x2F0000L ,0x300000L ,
    0x310000L ,0x320000L ,0x330000L ,0x340000L ,0x350000L ,0x360000L ,0x370000L ,0x380000L ,
    0x390000L ,0x3A0000L ,0x3B0000L ,0x3C0000L ,0x3D0000L ,0x3E0000L ,0x3F0000L ,0x400000L
};

#endif  /* _FLASHDRV_H_ */

