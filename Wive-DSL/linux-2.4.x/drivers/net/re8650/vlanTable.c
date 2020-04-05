/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /usr/local/dslrepos/uClinux-dist/linux-2.4.x/drivers/net/re8650/vlanTable.c,v 1.1.1.1 2003/08/18 05:43:22 kaohj Exp $
*
* Abstract: Switch core vlan table access driver source code.
*
* $Author: kaohj $
*
* $Log: vlanTable.c,v $
* Revision 1.1.1.1  2003/08/18 05:43:22  kaohj
* initial import into CVS
*
* Revision 1.1  2003/05/12 09:55:49  kckao
* moved from csp
*
* Revision 1.3  2003/05/09 12:15:10  kckao
* remove L2/L3 register
*
* Revision 1.2  2003/05/05 02:09:31  kckao
* Modify for OS independent
*
* Revision 1.2  2003/03/13 10:56:17  cfliu
* Move ASSERT_CSP to core/types.h
*
* Revision 1.1  2002/11/22 10:42:37  danwu
* init.
*
* ---------------------------------------------------------------
*/

#include "rtl_types.h"
#include "rtl_errno.h"
#include "asicRegs.h"
#include "swTable.h"
#include "swCore.h"
#include "vlanTable.h"


/* STATIC VARIABLE DECLARATIONS
 */



/* LOCAL SUBPROGRAM SPECIFICATIONS
 */



int32 vlanTable_create(uint32 vid, rtl_vlan_param_t * param)
{
    vlan_table_t    entryContent;
    
    ASSERT_CSP(param);
    
    swTable_readEntry(TYPE_VLAN_TABLE, vid & 7, &entryContent);
    
    if ( entryContent.valid )
    {
        if ( entryContent.vhid == (vid >> 3) )
            /* Specified vlan id already exists */
            return EEXIST;
        else
            return ENFILE;
    }

    entryContent.mac47_32 = param->gMac.mac47_32;
    entryContent.mac31_16 = param->gMac.mac31_16;
    entryContent.mac15_0 = param->gMac.mac15_0;
    entryContent.valid = 1;
    entryContent.memberPort = param->memberPort & ALL_PORT_MASK;
    entryContent.vhid = vid >> 3;
    entryContent.promiscuous = param->promiscuous;
    entryContent.bcastToCPU = param->bcastToCPU;
    entryContent.egressUntag = param->egressUntag;
    entryContent.mtuH = param->mtu >> 8;
    entryContent.mtuL = param->mtu & 0xff;
    
    /* Write into hardware */
    if ( swTable_addEntry(TYPE_VLAN_TABLE, vid & 7, &entryContent) == 0 )
        return 0;
    else
        /* There might be something wrong */
        ASSERT_CSP( 0 );
}

int32 vlanTable_destroy(uint32 vid)
{
    vlan_table_t    entryContent;
    
    swTable_readEntry(TYPE_VLAN_TABLE, vid & 7, &entryContent);
    
    if ( !entryContent.valid || 
            (entryContent.vhid != (vid >> 3)) )
        /* Specified vlan id does not exist */
        return ENOENT;
    
    bzero(&entryContent, sizeof(vlan_table_t));
    
    /* Write into hardware */
    if ( swTable_modifyEntry(TYPE_VLAN_TABLE, vid & 7, &entryContent) == 0 )
        return 0;
    else
        /* There might be something wrong */
        ASSERT_CSP( 0 );
}

int32 vlanTable_setPortStpStatus(uint32 vid, uint32 portNum, uint32 STPStatus)
{
    vlan_table_t    entryContent;
    
    ASSERT_CSP( portNum < MAX_PORT_NUMBER );
    ASSERT_CSP( STPStatus <= STP_FORWARD );
    
    swTable_readEntry(TYPE_VLAN_TABLE, vid & 7, &entryContent);
    
    if ( !entryContent.valid || 
            (entryContent.vhid != (vid >> 3)) )
        /* Specified vlan id does not exist */
        return ENOENT;
    
    entryContent.STPStatus &= ~3 << (2 * portNum);
    entryContent.STPStatus |= STPStatus << (2 * portNum);
    
    /* Write into hardware */
    if ( swTable_modifyEntry(TYPE_VLAN_TABLE, vid & 7, &entryContent) == 0 )
        return 0;
    else
        /* There might be something wrong */
        ASSERT_CSP( 0 );
}

int32 vlanTable_setStpStatusOfAllPorts(uint32 vid, uint32 STPStatus)
{
    vlan_table_t    entryContent;
    uint32          portNum;
    uint32          val = STPStatus;
    
    ASSERT_CSP( STPStatus <= STP_FORWARD );
    
    swTable_readEntry(TYPE_VLAN_TABLE, vid & 7, &entryContent);
    
    if ( !entryContent.valid || 
            (entryContent.vhid != (vid >> 3)) )
        /* Specified vlan id does not exist */
        return ENOENT;
    
    for (portNum=1; portNum < MAX_PORT_NUMBER; portNum++)
        val |= (val << 2);
    entryContent.STPStatus = val;
    
    /* Write into hardware */
    if ( swTable_modifyEntry(TYPE_VLAN_TABLE, vid & 7, &entryContent) == 0 )
        return 0;
    else
        /* There might be something wrong */
        ASSERT_CSP( 0 );
}

int32 vlanTable_getPortStpStatus(uint32 vid, uint32 portNum, uint32 *STPStatus_P)
{
    vlan_table_t    entryContent;
    
    ASSERT_CSP( portNum < MAX_PORT_NUMBER );
    ASSERT_CSP( STPStatus_P );
    
    swTable_readEntry(TYPE_VLAN_TABLE, vid & 7, &entryContent);
    
    if ( !entryContent.valid || 
            (entryContent.vhid != (vid >> 3)) )
        /* Specified vlan id does not exist */
        return ENOENT;
    
    /* Read from snapshot */
    *STPStatus_P = ( entryContent.STPStatus >> (2 * portNum) ) & 3;
    
    return 0;
}

int32 vlanTable_getInformation(uint32 vid, rtl_vlan_param_t * param_P)
{
    vlan_table_t    entryContent;
    
    ASSERT_CSP(param_P);
    
    swTable_readEntry(TYPE_VLAN_TABLE, vid & 7, &entryContent);
    
    if ( !entryContent.valid || 
            (entryContent.vhid != (vid >> 3)) )
        /* Specified vlan id does not exist */
        return ENOENT;
    
    param_P->gMac.mac47_32 = entryContent.mac47_32;
    param_P->gMac.mac31_16 = entryContent.mac31_16;
    param_P->gMac.mac15_0 = entryContent.mac15_0;
    param_P->memberPort = entryContent.memberPort;
    param_P->promiscuous = entryContent.promiscuous;
    param_P->bcastToCPU = entryContent.bcastToCPU;
    param_P->egressUntag = entryContent.egressUntag;
    param_P->mtu = (entryContent.mtuH << 8) | entryContent.mtuL;
    
    return 0;
}

int32 vlanTable_getVidByIndex(uint32 eidx, uint32 * vid_P)
{
    vlan_table_t    entryContent;
    
    ASSERT_CSP(vid_P);
    
    if ( eidx >= NUMBER_OF_VLAN_TABLE_ENTRY )
        return ENOENT;
    
    swTable_readEntry(TYPE_VLAN_TABLE, eidx, &entryContent);
    
    if ( !entryContent.valid )
        return EEMPTY;
        
    return (eidx | (entryContent.vhid << 3));
}
