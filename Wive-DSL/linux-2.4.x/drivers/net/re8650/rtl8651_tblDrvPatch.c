/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Patching Switch core table driver 
* Abstract : 
* Author : chih-hua huang (chhuang@realtek.com.tw)               
*
* $Id: rtl8651_tblDrvPatch.c,v 1.1.1.1 2003/08/18 05:43:22 kaohj Exp $
* $Log: rtl8651_tblDrvPatch.c,v $
* Revision 1.1.1.1  2003/08/18 05:43:22  kaohj
* initial import into CVS
*
* Revision 1.1  2003/05/19 09:22:00  elvis
* Initial version to BSP
**
*/

#include "rtl_types.h"

/*		
 * <<RTL8651 version B Bug>>
 * Due to the rtl8651 NAPT bug(if an entry is written into ASIC as a static entry, the 			
 * aging time and flow status will not be updated by ASIC), a patch is needed for 				
 * this bug:																					
 *	     Once an entry is added as a static entry(in fact, entries added by driver table		
 *		 are always static entry), we modify it to dynamic entry unconsciously in this		
 *		 macro.																					
 */	
int32 rtl8651_setAsicNaptTcpUdpTable_Patch(int8 forced, ipaddr_t sip, uint16 sport, uint16 gport,
		uint16 ageSec, int8 isStatic, int8 isTcp, int8 tcpFlag, int8 isColl, int8 isValid) {

	return rtl8651_setAsicNaptTcpUdpTable(
			forced, 
			sip,
			sport,
			gport,
			ageSec,
			FALSE,	/* patch here!! always false(dynamic) */
			isTcp,
			tcpFlag,
			isColl,
			isValid
	);
}


/*		
 * <<RTL8651 version B Bug>>
 * Due to the rtl8651 NAPT bug(if an entry is written into ASIC as a static entry, the 			
 * aging time and flow status will not be updated by ASIC), a patch is needed for 				
 * this bug:																					
 *	     Once an entry is added as a static entry(in fact, entries added by driver table		
 *		 are always static entry), we modify it to dynamic entry unconsciously in this		
 *		 macro.																					
 */	
int32 rtl8651_setAsicNaptIcmpTable_Patch(int8 forced, ipaddr_t sip, uint16 sID, uint16 gID, 
		uint16 ageSec, int8 isStatic, int16 count, int8 isColl, int8 isValid) {

	return rtl8651_setAsicNaptIcmpTable(
			forced,
			sip,
			sID,
			gID,
			ageSec,
			FALSE,	/* patch here!! always false(dynamic) */
			count,
			isColl,
			isValid
	);
}


/*
 * <<RTL8651 version C Bug>> 
 * In this version, once the aging time of an ICMP static entry reaches zero, the ASIC
 * will reset its valid bit and never auto-learn. To patch this bug, once the 
 * static and valid fields of reading entry are 1 and 0 respectively, translate it
 * to static=1 and valid=1.
 */
int32 rtl8651_getAsicNaptIcmpTable_Patch(int8 preID, uint16 tgID, ipaddr_t *sip, uint16 *sID, 
		uint16 *gID, uint16 *ageSec, int8 *isStatic, uint16 *count, int8 *isColl, int8 *isValid) {
	int8 _valid, _static;
	uint32 retval=rtl8651_getAsicNaptIcmpTable(
					preID,
					tgID,
					sip,
					sID,
					gID,
					ageSec,
					&_static,
					count,
					isColl,
					&_valid
	);
	/* patch here, if static bit is turned on, valid bit is always true */
	if (_static == TRUE)  _valid = TRUE; 
	if (isValid != NULL)  isValid = _valid;
	if (isStatic != NULL) isStatic = _static;
	return retval;
}


/*
 * <<RTL8651 version B Bug>>
 * RTL8651 L2 entry bug:
 *		For each L2 entry added by driver table as a static entry, the aging time 
 *		will not be updated by ASIC
 * Bug fixed:
 *		To patch this bug, set the entry is a dynamic entry and turn on the 'nhFlag', 
 *		then the aging time of this entry will be updated and once aging time expired,
 *		it won't be removed by ASIC automatically.
 */
int32 rtl8651_setAsicL2Table_Patch(uint32 row, uint32 column, ether_addr_t * mac, int8 cpu, 
		int8 srcBlk, uint32 mbr, uint32 ageSec, int8 isStatic, int8 nhFlag) {
		
	return rtl8651_setAsicL2Table(
			row,
			column,
			mac,
			cpu,
			srcBlk,
			mbr,
			ageSec,
			FALSE,							/* patch here!! always dynamic entry */
			(isStatic==TRUE? TRUE: FALSE)	/* patch here!! nhFlag always turned on if static entry*/
	);
}


/*
 * <<RTL8651 version B Bug>>
 * RTL8651 L2 entry bug:
 *		For each L2 entry added by driver table as a static entry, the aging time 
 *		will not be updated by ASIC
 * Bug fixed:
 *		To patch this bug, set the entry as a dynamic entry and turn on the 'nhFlag', 
 *		then the aging time of this entry will be updated and once aging time expired,
 *		it won't be removed by ASIC automatically.
 */
int32 rtl8651_getAsicL2Table_Patch(uint32 row, uint32 column, ether_addr_t * mac, int8 * cpu, 
		int8 * srcBlk, int8 * isStatic, uint32 * mbr, uint32 * ageSec, int8 *nhFlag) {
	int32 retval = rtl8651_getAsicL2Table(
					row, 
					column,
					mac,
					cpu,
					srcBlk,
					isStatic,
					mbr,
					ageSec,
					nhFlag
	);
	if (isStatic != NULL) *isStatic = TRUE; /* patch!!, always TRUE(static entry */
	if (nhFlag != NULL) *nhFlag = FALSE;  /* always false */
	return retval;
}


