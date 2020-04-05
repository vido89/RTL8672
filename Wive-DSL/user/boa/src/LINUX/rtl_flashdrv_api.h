/*
 * Copyright c                Realtek Semiconductor Corporation, 2002
 * All rights reserved.                                                    
 * 
 * $Header: /usr/local/dslrepos/uClinux-dist/user/boa/src/LINUX/rtl_flashdrv_api.h,v 1.3 2007/05/15 12:45:40 star Exp $
 *
 * $Author: star $
 *
 * Abstract:
 *
 *   Flash driver header file for export include.
 *
 * $Log: rtl_flashdrv_api.h,v $
 * Revision 1.3  2007/05/15 12:45:40  star
 * no message
 *
 * Revision 1.2  2005/12/07 11:45:53  kaohj
 * mmu support
 *
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
 * Revision 1.3  2003/06/23 11:03:39  elvis
 * change include path of  rtl_types.h
 *
 * Revision 1.2  2003/05/20 08:45:36  elvis
 * change the include path of "rtl_types.h"
 *
 * Revision 1.1  2003/04/29 14:18:51  orlando
 * flashdrv module initial check-in (used by cfgmgr, ported from srcroot).
 *
 * Revision 1.1  2002/07/19 08:39:14  danwu
 * Create file.
 *
 *
 * 
 */

#ifndef _RTL_FLASHDRV_H_
#define _RTL_FLASHDRV_H_



#include "rtl_types.h"
#include "rtl_board.h"

typedef struct flashdriver_obj_s {
	uint32  flashSize;
	uint32  flashBaseAddress;
	uint32 *blockBaseArray_P;
	uint32  blockBaseArrayCapacity;
	uint32  blockNumber;
} flashdriver_obj_t;



/*
 * FUNCTION PROTOTYPES
 */
/*
uint32 flashdrv_init(flashdriver_obj_t * const drvObj_P);
*/
uint32 flashdrv_init();
#ifndef CONFIG_MMU
uint32 flashdrv_eraseBlock(void *startAddr_P);
#endif
uint32 flashdrv_read (void *dstAddr_P, void *srcAddr_P, uint32 size);
uint32 flashdrv_write(void *dstAddr_P, void *srcAddr_P, uint32 size);
uint32 flashdrv_updateImg(void *srcAddr_P, void *dstAddr_P, uint32 size);

#endif  /* _RTL_FLASHDRV_H_ */

