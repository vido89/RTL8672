/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /usr/local/dslrepos/uClinux-dist/linux-2.4.x/drivers/net/re8650/swTable.h,v 1.1.1.1 2003/08/18 05:43:22 kaohj Exp $
*
* Abstract: Switch core table header file.
*
* $Author: kaohj $
*
* $Log: swTable.h,v $
* Revision 1.1.1.1  2003/08/18 05:43:22  kaohj
* initial import into CVS
*
* Revision 1.1  2003/05/12 09:55:58  kckao
* moved from csp
*
* Revision 1.2  2003/05/05 02:09:41  kckao
* Modify for OS independent
*
* Revision 1.13  2002/11/22 10:43:49  danwu
* Reconstruct table drivers.
*
* Revision 1.12  2002/09/24 01:31:11  henryCho
* no message
*
* Revision 1.11  2002/09/17 14:03:20  henryCho
* Modify hashL4GetIndex().
*
* Revision 1.10  2002/09/17 00:56:18  danwu
* no message
*
* Revision 1.9  2002/09/16 09:16:47  danwu
* Icmp table conforms to spec.
*
* Revision 1.8  2002/09/13 06:23:01  danwu
* ACL ethernet table conforms to spec.
*
* Revision 1.7  2002/09/13 06:12:07  henryCho
* L2 table mac conforms to spec.
*
* Revision 1.6  2002/09/09 05:14:01  danwu
* Debug vlan table structure declaration.
*
* Revision 1.5  2002/09/03 12:55:52  danwu
* Add ifsel rule of acl table.
*
* Revision 1.4  2002/09/03 05:21:08  danwu
* Add mtu field to vlan table.
* Add routines for table access commands.
*
* Revision 1.3  2002/08/28 11:21:46  danwu
* Add mtu field of VLAN table.
*
* Revision 1.2  2002/08/21 06:42:50  danwu
* Modify VLAN gMac and multicast srcPort fields according to spec.
*
* Revision 1.1  2002/08/20 05:57:26  danwu
* Create.
*
* ---------------------------------------------------------------
*/

#ifndef _SWTABLE_H_
#define _SWTABLE_H_

#include "rtl_types.h"
#include "swCore.h"


/* DATA STRUCTURE DECLARATIONS
*/
typedef void * routeid_t;



int32 swTable_init(void);

/* Add an entry 
Return: ECOLLISION- Specified slot is already occupied.*/
int32 swTable_addEntry(uint32 tableType, uint32 eidx, void *entryContent_P);

/* Add an entry 
Return: EEMPTY- Specified slot is empty.*/
int32 swTable_modifyEntry(uint32 tableType, uint32 eidx, void *entryContent_P);

/* Force add an entry */
int32 swTable_forceAddEntry(uint32 tableType, uint32 eidx, void *entryContent_P);

/* Read an entry */
int32 swTable_readEntry(uint32 tableType, uint32 eidx, void *entryContent_P);



/* Table entry numbers defintions 
*/
#define NUMBER_OF_VLAN_TABLE_ENTRY             8
#define NUMBER_OF_PROTOCOL_TRAP_TABLE_ENTRY    8
#define NUMBER_OF_EXT_INTERNAL_IP_TABLE_ENTRY  8
#define NUMBER_OF_SERVER_PORT_TABLE_ENTRY      8
#define NUMBER_OF_ALG_TABLE_ENTRY              128
#define NUMBER_OF_L3_ROUTING_TABLE_ENTRY       8
#define NUMBER_OF_ARP_TABLE_ENTRY              512
#define NUMBER_OF_PPPOE_TABLE_ENTRY            8
#define NUMBER_OF_L2_SWITCH_TABLE_ENTRY        1024
#define NUMBER_OF_MULTICAST_TABLE_ENTRY        64
#define NUMBER_OF_L4_TCP_UDP_TABLE_ENTRY       1024
#define NUMBER_OF_L4_ICMP_TABLE_ENTRY          32
#define NUMBER_OF_ACL_TABLE_ENTRY              128

#define L3_DEFAULT_ROUTE_ENTRY_NUMBER       7
#define NUMBER_OF_ENTRY_IN_ARP_BLOCK        8



#endif  /* _SWTABLE_H_ */
