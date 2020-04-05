/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /usr/local/dslrepos/uClinux-dist/linux-2.4.x/drivers/net/re8650/swTable.c,v 1.1.1.1 2003/08/18 05:43:22 kaohj Exp $
*
* Abstract: Switch core table access driver source code.
*
* $Author: kaohj $
*
* $Log: swTable.c,v $
* Revision 1.1.1.1  2003/08/18 05:43:22  kaohj
* initial import into CVS
*
* Revision 1.1  2003/05/12 09:56:04  kckao
* moved from csp
*
* Revision 1.3  2003/05/09 12:15:17  kckao
* remove L2/L3 register
*
* Revision 1.2  2003/05/05 02:09:41  kckao
* Modify for OS independent
*
* Revision 1.25  2003/03/13 10:56:17  cfliu
* Move ASSERT_CSP to core/types.h
*
* Revision 1.24  2002/12/11 02:27:04  elvis
* Change SWTABLE_BASE_OF_ALL_TABLES
*
* Revision 1.23  2002/11/22 10:43:11  danwu
* Reconstruct table drivers.
*
* Revision 1.22  2002/09/26 05:25:24  henryCho
* no message
*
* Revision 1.21  2002/09/24 01:30:02  henryCho
* no message
*
* Revision 1.20  2002/09/18 01:07:29  danwu
* no message
*
* Revision 1.19  2002/09/17 13:58:36  henryCho
* Modify hashL4GetIndex().
*
* Revision 1.18  2002/09/17 07:02:06  danwu
* Modify l4 hash algorithm.
*
* Revision 1.17  2002/09/17 01:56:01  henryCho
* no message
*
* Revision 1.16  2002/09/17 00:57:58  danwu
* Add l4TcpUdpTable_forceAdd(), l4TcpUdpTable_removeByIndex(),l4IcmpTable_forceAdd(), l4IcmpTable_removeByIndex().
*
* Revision 1.15  2002/09/16 07:09:41  danwu
* Add swTable_cmdTabClear().
*
* Revision 1.14  2002/09/13 10:35:08  danwu
* Add swTable_cmdTabDump() for cli command.
*
* Revision 1.13  2002/09/13 06:51:19  henryCho
* ICMP table icmp id field conform to spec.
*
* Revision 1.12  2002/09/13 06:15:32  danwu
* Change ip table isN2One to isOne2One.
*
* Revision 1.11  2002/09/13 06:00:49  henryCho
* L2 table mac conforms to spec.
*
* Revision 1.10  2002/09/10 07:10:22  danwu
* Vlan table read should include 5 words.
*
* Revision 1.9  2002/09/09 05:14:18  danwu
* Debug vlan table structure declaration.
*
* Revision 1.8  2002/09/04 10:05:33  danwu
* no message
*
* Revision 1.7  2002/09/04 02:30:32  danwu
* Add ifsel rule for acl table.
*
* Revision 1.6  2002/09/03 12:54:21  danwu
* Add ifsel rule of acl table.
*
* Revision 1.5  2002/09/03 05:20:01  danwu
* Add routines for table access commands.
*
* Revision 1.4  2002/08/28 11:18:08  danwu
* Add collision control codes. (in progress)
*
* Revision 1.3  2002/08/23 08:58:54  danwu
* no message
*
* Revision 1.2  2002/08/21 06:35:03  danwu
* Modify VLAN gMac and multicast srcPort fields according to spec.
*
* Revision 1.1  2002/08/20 05:59:09  danwu
* Create.
*
* ---------------------------------------------------------------
*/

#include "rtl_types.h"
#include "rtl_errno.h"
#include "asicRegs.h"
#include "swCore.h"
#include "swTable.h"



/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
RTL_STATIC_INLINE void tableAccessForeword(uint32, uint32, void *);

int32 swTable_addEntry(uint32 tableType, uint32 eidx, void *entryContent_P)
{
    tableAccessForeword(tableType, eidx, entryContent_P);
        
    /* Activate add command */
    REG32(SWTACR) = ACTION_START | CMD_ADD;
    
    /* Wait for command done */
    while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE );
    
    /* Check status */
    if ( (REG32(SWTASR) & TABSTS_MASK) != TABSTS_SUCCESS )
        return ECOLLISION;
    else
        return 0;
}

int32 swTable_modifyEntry(uint32 tableType, uint32 eidx, void *entryContent_P)
{
    tableAccessForeword(tableType, eidx, entryContent_P);
        
    /* Activate add command */
    REG32(SWTACR) = ACTION_START | CMD_MODIFY;
    
    /* Wait for command done */
    while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE );
    
    /* Check status */
    if ( (REG32(SWTASR) & TABSTS_MASK) != TABSTS_SUCCESS )
        return EEMPTY;
    else
        return 0;
}

int32 swTable_forceAddEntry(uint32 tableType, uint32 eidx, void *entryContent_P)
{
    tableAccessForeword(tableType, eidx, entryContent_P);
        
    /* Activate add command */
    REG32(SWTACR) = ACTION_START | CMD_FORCE;
    
    /* Wait for command done */
    while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE );
    
    /* Check status */
    if ( (REG32(SWTASR) & TABSTS_MASK) == TABSTS_SUCCESS )
        return 0;
        
    /* There might be something wrong */
    ASSERT_CSP( 0 );
}

int32 swTable_readEntry(uint32 tableType, uint32 eidx, void *entryContent_P)
{
    uint32 *    entryAddr;
    
    ASSERT_CSP(entryContent_P);
    
    entryAddr = (uint32 *) (table_access_addr_base(tableType) + eidx * TABLE_ENTRY_DISTANCE);
    
    /* Wait for command ready */
    while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE );
    
    /* Read registers according to entry width of each table */
    switch (table_entry_length(tableType))
    {
        case 8:
            *((uint32 *)entryContent_P + 7) = *(entryAddr + 7);
        case 7:
            *((uint32 *)entryContent_P + 6) = *(entryAddr + 6);
        case 6:
            *((uint32 *)entryContent_P + 5) = *(entryAddr + 5);
        case 5:
            *((uint32 *)entryContent_P + 4) = *(entryAddr + 4);
        case 4:
            *((uint32 *)entryContent_P + 3) = *(entryAddr + 3);
        case 3:
            *((uint32 *)entryContent_P + 2) = *(entryAddr + 2);
        case 2:
            *((uint32 *)entryContent_P + 1) = *(entryAddr + 1);
        case 1:
            *((uint32 *)entryContent_P + 0) = *(entryAddr + 0);
            break;
            
        default:
            ASSERT_CSP( 0 );
    }
    
    return 0;
}

RTL_STATIC_INLINE void tableAccessForeword(uint32 tableType, uint32 eidx, 
                                                void *entryContent_P)
{
    ASSERT_CSP(entryContent_P);

    /* Wait for command done */
    while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE );
    
    /* Write registers according to entry width of each table */
    switch (table_entry_length(tableType))
    {
        case 8:
            REG32(TCR7) = *((uint32 *)entryContent_P + 7);
        case 7:
            REG32(TCR6) = *((uint32 *)entryContent_P + 6);
        case 6:
            REG32(TCR5) = *((uint32 *)entryContent_P + 5);
        case 5:
            REG32(TCR4) = *((uint32 *)entryContent_P + 4);
        case 4:
            REG32(TCR3) = *((uint32 *)entryContent_P + 3);
        case 3:
            REG32(TCR2) = *((uint32 *)entryContent_P + 2);
        case 2:
            REG32(TCR1) = *((uint32 *)entryContent_P + 1);
        case 1:
            REG32(TCR0) = *(uint32 *)entryContent_P;
            break;
            
        default:
            ASSERT_CSP( 0 );
    }
    
    /* Fill address */
    REG32(SWTAA) = table_access_addr_base(tableType) + eidx * TABLE_ENTRY_DISTANCE;
}
