/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /usr/local/dslrepos/uClinux-dist/linux-2.4.x/drivers/net/re8650/l2SwitchTable.h,v 1.1.1.1 2003/08/18 05:43:22 kaohj Exp $
*
* Abstract: Switch core l2 switch table access header file.
*
* $Author: kaohj $
*
* $Log: l2SwitchTable.h,v $
* Revision 1.1.1.1  2003/08/18 05:43:22  kaohj
* initial import into CVS
*
* Revision 1.1  2003/05/12 09:57:44  kckao
* moved from csp
*
* Revision 1.1  2003/05/05 03:07:13  kckao
* Modified for OS independent
*
* Revision 1.1  2002/11/22 10:44:24  danwu
* init.
*
* ---------------------------------------------------------------
*/

#ifndef _L2SWITCHTABLE_H_
#define _L2SWITCHTABLE_H_



/* L2 switch table access routines 
*/

/* Add a rule 
Return: EEXIST- Specified rule already exists.
        ENFILE- No slot available.*/
int32 l2SwitchTable_add(uint32 * eidx_P, rtl_l2_param_t * param_P);

/* Remove a rule (must be static or next hop) 
Return: ENOENT- Specified rule does not exist.*/
int32 l2SwitchTable_remove(uint32 * eidx_P, macaddr_t * mac_P);

/* Remove a rule by index (must be static or next hop) 
Return: EEMPTY- Destined slot is already empty or a dynamic entry.
        ENOENT- Specified slot does not exist.*/
int32 l2SwitchTable_removeByIndex(uint32 eidx);

/* Get information 
Return: EEMPTY- Destined slot is empty.
        ENOENT- Specified slot does not exist.*/
int32 l2SwitchTable_getInformation(uint32 eidx, rtl_l2_param_t * param_P);

/* Get information by mac 
Return: ENOENT- Specified mac does not exist.*/
int32 l2SwitchTable_getInformationByMac(uint32 * eidx_P, rtl_l2_param_t * param_P);

/* Get hardware information 
Return: EEMPTY- Destined slot is empty.
        ENOENT- Specified slot does not exist.*/
int32 l2SwitchTable_getHwInformation(uint32 eidx, rtl_l2_param_t * param_P);

/* Get hashed index 
Return: index.*/
uint32 l2SwitchTable_getHashIndex(macaddr_t * macAddr_P);

/* Restore mac by hashed index */
void l2SwitchTable_restoreMacLsbBits(macaddr_t * macAddr_P, uint32 eidx);



/* Hardware bit allocation of l2 table 
*/
typedef struct {
    /* word 0 */
    uint16          mac39_24;
    uint16          mac23_8;
    /* word 1 */
    uint16          reserv0     : 11;
    uint16          nxtHostFlag : 1;
    uint16          srcBlock    : 1;
    uint16          agingTime   : 2;
    uint16          isStatic    : 1;
    uint16          toCPU       : 1;
    uint16          hPriority   : 1;
    uint16          memberPort  : 6;
    uint16          mac47_40    : 8;
} l2_switch_table_t;



#endif /*_L2SWITCHTABLE_H_*/
