/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /usr/local/dslrepos/uClinux-dist/linux-2.4.x/drivers/net/re8650/l2SwitchTable.c,v 1.1.1.1 2003/08/18 05:43:22 kaohj Exp $
*
* Abstract: Switch core l2 switch table access driver source code.
*
* $Author: kaohj $
*
* $Log: l2SwitchTable.c,v $
* Revision 1.1.1.1  2003/08/18 05:43:22  kaohj
* initial import into CVS
*
* Revision 1.1  2003/05/12 09:57:49  kckao
* moved from csp
*
* Revision 1.2  2003/05/09 12:16:03  kckao
* remove L2/L3 register
*
* Revision 1.1  2003/05/05 03:07:14  kckao
* Modified for OS independent
*
* Revision 1.2  2003/03/13 10:56:19  cfliu
* Move ASSERT_CSP to core/types.h
*
* Revision 1.1  2002/11/22 10:42:38  danwu
* init.
*
* ---------------------------------------------------------------
*/

#include "rtl_types.h"
#include "rtl_errno.h"
#include "asicRegs.h"
#include "swCore.h"
#include "swTable.h"
#include "l2SwitchTable.h"



/* STATIC VARIABLE DECLARATIONS
 */



/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
RTL_STATIC_INLINE int32 _cmpTableMac(l2_switch_table_t*, macaddr_t*);
RTL_STATIC_INLINE void _getTableCommon(uint32, rtl_l2_param_t*, l2_switch_table_t*);



int32 l2SwitchTable_add(uint32 * eidx_P, rtl_l2_param_t * param_P)
{
    uint32  eidx;
    uint32  i;
    uint32  indexOfWay = 4;
    uint32  dynamicAgingTime = 0xFFFFFFFF;
    l2_switch_table_t   entryContent;
    
    ASSERT_CSP( eidx_P );
    ASSERT_CSP( param_P );
    
    /* Calculate index */
    eidx = l2SwitchTable_getHashIndex(&param_P->mac);
    
    /* Return index which is regardless of 4 ways if an error occurs */
    *eidx_P = eidx;
    
    /* Walk through 4 ways to find available slot */
    for (i=0; i<4; i++)
    {
        swTable_readEntry(TYPE_L2_SWITCH_TABLE, eidx + i, &entryContent);
        
        if ( entryContent.isStatic )
        {
            if ( _cmpTableMac(&entryContent, &param_P->mac) == 0 )
                return EEXIST;
        }
        else
        {
            /* Is an auto-learnt entry */
            if ( entryContent.agingTime && 
                    (_cmpTableMac(&entryContent, &param_P->mac) == 0) )
            {
                /* Found the specified MAC address */
                indexOfWay = i;
                goto _add_entry_now;
            }
            
            /* Find the dynamic entry with smallest aging time */
            if ( entryContent.agingTime < dynamicAgingTime )
            {
                indexOfWay = i;
                dynamicAgingTime = entryContent.agingTime;
            }
        }
    }
    
    if ( indexOfWay == 4 )
        /* All of 4 ways are occupied by static entries */
        return ENFILE;

_add_entry_now:
    entryContent.mac47_40 = param_P->mac.mac47_32 >> 8;
    entryContent.mac39_24 = ((param_P->mac.mac47_32 & 0xff) << 8) | 
                            (param_P->mac.mac31_16 >> 8);
    entryContent.mac23_8 = ((param_P->mac.mac31_16 & 0xff) << 8) | 
                            (param_P->mac.mac15_0 >> 8);
    entryContent.memberPort = param_P->memberPort;
    entryContent.hPriority = param_P->hPriority;
    entryContent.toCPU = param_P->toCPU;
    entryContent.isStatic = param_P->isStatic;
    entryContent.agingTime = param_P->agingTime;
    entryContent.srcBlock = param_P->srcBlock;
    entryContent.nxtHostFlag = param_P->nxtHostFlag;
    
    /* Write into hardware */
    /* Force add because we cannot make sure if the slot is occupied by 
    an auto-learnt entry */
    if ( swTable_forceAddEntry(TYPE_L2_SWITCH_TABLE, eidx + indexOfWay, &entryContent) == 0 )
    {
        *eidx_P = eidx + indexOfWay;
        return 0;
    }
    else
        /* There might be something wrong */
        ASSERT_CSP( 0 );
}

int32 l2SwitchTable_remove(uint32 * eidx_P, macaddr_t * mac_P)
{
    uint32  eidx;
    uint32  i;
    l2_switch_table_t   entryContent;
    
    ASSERT_CSP( eidx_P );
    ASSERT_CSP( mac_P );
    
    /* Calculate index */
    eidx = l2SwitchTable_getHashIndex(mac_P);
    
    /* Look up 4 ways */
    for (i=0; i<4; i++)
    {
        swTable_readEntry(TYPE_L2_SWITCH_TABLE, eidx + i, &entryContent);
        
        /* Only remove static entry */
        if ( entryContent.isStatic && 
                (_cmpTableMac(&entryContent, mac_P) == 0) )
        {
            bzero(&entryContent, sizeof(l2_switch_table_t));
    
            /* Clear hardware */
            if ( swTable_modifyEntry(TYPE_L2_SWITCH_TABLE, eidx + i, &entryContent) == 0 )
            {
                *eidx_P = eidx + i;
                return 0;
            }
            else
                /* There might be something wrong */
                ASSERT_CSP( 0 );
        }
    }
    
    return ENOENT;    
}

int32 l2SwitchTable_removeByIndex(uint32 eidx)
{
    /* Only remove static entry */
    l2_switch_table_t   entryContent;
    
    if ( eidx >= NUMBER_OF_L2_SWITCH_TABLE_ENTRY )
        return ENOENT;
    
    swTable_readEntry(TYPE_L2_SWITCH_TABLE, eidx, &entryContent);
    
    /* Only remove static entry */
    if ( !entryContent.isStatic  )
        return EEMPTY;
    
    bzero(&entryContent, sizeof(l2_switch_table_t));

    /* Clear hardware */
    if ( swTable_modifyEntry(TYPE_L2_SWITCH_TABLE, eidx, &entryContent) == 0 )
        return 0;
    else
        /* There might be something wrong */
        ASSERT_CSP( 0 );
}

int32 l2SwitchTable_getInformation(uint32 eidx, rtl_l2_param_t * param_P)
{
    l2_switch_table_t   entryContent;
    
    ASSERT_CSP( param_P );
    
    if ( eidx >= NUMBER_OF_L2_SWITCH_TABLE_ENTRY )
        return ENOENT;
    
    swTable_readEntry(TYPE_L2_SWITCH_TABLE, eidx, &entryContent);
    
    if ( !entryContent.isStatic && 
            !entryContent.nxtHostFlag && 
            (entryContent.agingTime == 0) )
        return EEMPTY;

    _getTableCommon(eidx, param_P, &entryContent);
    
    return 0;
}

int32 l2SwitchTable_getInformationByMac(uint32 * eidx_P, rtl_l2_param_t * param_P)
{
    uint32  eidx;
    uint32  i;
    l2_switch_table_t   entryContent;
    
    ASSERT_CSP( eidx_P );
    ASSERT_CSP( param_P );
    
    /* Calculate index */
    eidx = l2SwitchTable_getHashIndex(&param_P->mac);
    
    /* Look up 4 ways */
    for (i=0; i<4; i++)
    {
        swTable_readEntry(TYPE_L2_SWITCH_TABLE, eidx + i, &entryContent);
        if ( _cmpTableMac(&entryContent, &param_P->mac) == 0 )
            break;
    }
    
    if ( i == 4 )
        return ENOENT;
    
    eidx += i;
    if ( !entryContent.isStatic && 
            !entryContent.nxtHostFlag && 
            (entryContent.agingTime == 0) )
        return ENOENT;
    
    _getTableCommon(eidx, param_P, &entryContent);
    *eidx_P = eidx;
    
    return 0;
}

int l2SwitchTable_getPortByMac(char *eth_mac)
{
	macaddr_t *mac = (macaddr_t *)eth_mac;
    uint32  eidx;
    uint32  i;
    l2_switch_table_t   entryContent;
	char	*l2mac = (char*)&entryContent;
	
    ASSERT_CSP( mac );
    
    if(eth_mac[0]==0xff)
    	return 0xFFFFFFFF;
    /* Calculate index */
    eidx = l2SwitchTable_getHashIndex(mac);
    
    /* Look up 4 ways */
    for (i=0; i<4; i++)
    {
        swTable_readEntry(TYPE_L2_SWITCH_TABLE, eidx + i, &entryContent);
		if(eth_mac[0]==l2mac[7] && \
			eth_mac[1]==l2mac[0]&& \
			eth_mac[2]==l2mac[1]&& \
			eth_mac[3]==l2mac[2]&& \
			eth_mac[4]==l2mac[3])
			break;
    }
    
    if ( i == 4 )
    {
        return 0xFFFFFFFF;
    }
    return ((int)entryContent.memberPort);
	
}

RTL_STATIC_INLINE int32 _cmpTableMac(l2_switch_table_t * entry_P, macaddr_t * mac_P)
{
    return ( (entry_P->mac47_40 == mac_P->mac47_32 >> 8) && 
            (entry_P->mac39_24 == ((mac_P->mac47_32 & 0xff) << 8) | (mac_P->mac31_16 >> 8)) && 
            (entry_P->mac23_8 == ((mac_P->mac31_16 & 0xff) << 8) | (mac_P->mac15_0 >> 8)) );
}

RTL_STATIC_INLINE void _getTableCommon(uint32 eidx, rtl_l2_param_t * param_P, l2_switch_table_t * entry_P)
{
    param_P->mac.mac47_32 = ((uint16)entry_P->mac47_40 << 8) | (entry_P->mac39_24 >> 8);
    param_P->mac.mac31_16 = (entry_P->mac39_24 << 8) | (entry_P->mac23_8 >> 8);
    param_P->mac.mac15_0 = entry_P->mac23_8 << 8;
    l2SwitchTable_restoreMacLsbBits(&param_P->mac, eidx);
    param_P->memberPort = entry_P->memberPort;
    param_P->hPriority = entry_P->hPriority;
    param_P->toCPU = entry_P->toCPU;
    param_P->isStatic = entry_P->isStatic;
    param_P->agingTime = entry_P->agingTime;
    param_P->srcBlock = entry_P->srcBlock;
    param_P->nxtHostFlag = entry_P->nxtHostFlag;
    
    return;
}

uint32 l2SwitchTable_getHashIndex(macaddr_t * macAddr_P)
{
    return ( (macAddr_P->mac15_0 ^ (macAddr_P->mac15_0 >> 8) ^
                    macAddr_P->mac31_16 ^ (macAddr_P->mac31_16 >> 8) ^
                    macAddr_P->mac47_32 ^ (macAddr_P->mac47_32 >> 8)) & 0xFF ) << 2;
}

void l2SwitchTable_restoreMacLsbBits(macaddr_t * macAddr_P, uint32 eidx)
{
    uint64          macAddrLong = 0;
    
    memcpy(&macAddrLong, macAddr_P, 5);
    
    macAddrLong |= ( (macAddrLong >> 8) ^ (macAddrLong >> 16) ^ 
                    (macAddrLong >> 24) ^ (macAddrLong >> 32) ^ 
                    (macAddrLong >> 40) ^ (((eidx >> 2) & 0xFF) << 16) ) & 0x00FF0000;
    
    memcpy(macAddr_P, &macAddrLong, sizeof(macaddr_t));
    
    return;
}
