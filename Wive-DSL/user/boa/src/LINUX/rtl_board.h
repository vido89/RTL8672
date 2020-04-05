/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /usr/local/dslrepos/uClinux-dist/user/boa/src/LINUX/rtl_board.h,v 1.3 2007/05/15 12:45:40 star Exp $
*
* Abstract: Board specific definitions.
*
* $Author: star $
*
* $Log: rtl_board.h,v $
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
* Revision 1.2  2004/02/20 12:33:32  kaohj
* use 2MB flash
*
* Revision 1.1  2003/10/01 09:08:42  dicktam
* *** empty log message ***
*
* Revision 1.24  2003/07/03 12:15:59  tysu
* fix some warning messages.
*
* Revision 1.23  2003/07/03 06:01:11  tysu
* change dhcpcCfgParam_s field order, for 4 bytes alignment.
*
* Revision 1.22  2003/07/01 10:25:22  orlando
* call table driver server port and acl API in board.c
*
* Revision 1.21  2003/07/01 03:09:06  tysu
* add aclGlobalCfgParam structure.
* modify aclCfgParam and serverpCfgParam structure.
*
* Revision 1.20  2003/06/30 13:27:55  tysu
* add a field in server_port table
*
* Revision 1.19  2003/06/30 07:46:12  tysu
* add ACL and Server_Port structure
*
* Revision 1.18  2003/06/27 13:53:25  orlando
* remove dhcpcCfgParam.valid field, use ifCfgParam[0].connType field
* modify rtConfigHook() for cloneMac:
* use dhcpcCfgParam.cloneMac if ifCfgParam[0].connType is
* DHCPC and dhcpcCfgParam.cmacValid is 1 during boottime.
*
* Revision 1.17  2003/06/27 05:33:06  orlando
* adjust some structure.
*
* Revision 1.16  2003/06/26 03:22:29  tysu
* remove some element in dhcpc data structure.
*
* Revision 1.15  2003/06/26 03:19:53  orlando
* add ifCfgParam_connType enumeration.
*
* Revision 1.14  2003/06/23 10:57:23  elvis
* change include path of  rtl_types.h
*
* Revision 1.13  2003/06/20 12:59:50  tysu
* add dhcp client
*
* Revision 1.12  2003/06/16 08:08:30  tysu
* add dhcps & dns function
*
* Revision 1.11  2003/06/11 11:39:29  tysu
* add WAN IP connection Type (static, PPPoE, DHCP)
*
* Revision 1.10  2003/06/06 11:57:06  orlando
* add pppoe cfg.
*
* Revision 1.9  2003/06/06 06:31:45  idayang
* add mgmt table in cfgmgr.
*
* Revision 1.8  2003/06/03 10:56:57  orlando
* add nat table in cfgmgr.
*
* Revision 1.7  2003/05/20 08:45:03  elvis
* change the include path of "rtl_types.h"
*
* Revision 1.6  2003/05/09 12:22:20  kckao
* remove RTL8650_LOAD_COUNT
*
* Revision 1.5  2003/05/02 08:51:45  orlando
* merge cfgmgr with old board.c/board.h.
*
* Revision 1.4  2003/04/29 14:20:19  orlando
* modified for cfgmgr.
*
* Revision 1.3  2003/04/08 15:29:47  elvis
* add interface ,route and static arp configurations
*
* Revision 1.2  2003/04/04 02:28:48  elvis
* add constant definition for PHY0, ..., PHY4
*
* Revision 1.1  2003/04/03 01:52:37  danwu
* init
*
* ---------------------------------------------------------------
*/
#ifndef _BOARD_H_
#define _BOARD_H_

#include "rtl_types.h"

/* Define flash device 
*/
#undef FLASH_AM29LV800BB   /* only use 1MB currently */
#define FLASH_AM29LV160BB
#ifdef CONFIG_MMU
#define FLASH_BASE          0
#else
#define FLASH_BASE          0xBFC00000
#endif

#endif /* _BOARD_H_ */
