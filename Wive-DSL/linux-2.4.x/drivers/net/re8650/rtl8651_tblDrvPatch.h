/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Patching Switch core table driver 
* Abstract : 
* Author : chih-hua huang (chhuang@realtek.com.tw)               
*
* $Id: rtl8651_tblDrvPatch.h,v 1.1.1.1 2003/08/18 05:43:22 kaohj Exp $
* $Log: rtl8651_tblDrvPatch.h,v $
* Revision 1.1.1.1  2003/08/18 05:43:22  kaohj
* initial import into CVS
*
* Revision 1.1  2003/05/19 09:22:00  elvis
* Initial version to BSP
*
*
*/


#ifndef RTL8651_TBLDRV_PATCH_H
#define RTL8651_TBLDRV_PATCH_H


int32 rtl8651_setAsicNaptTcpUdpTable_Patch(int8 forced, ipaddr_t sip, uint16 sport, uint16 gport,
		uint16 ageSec, int8 isStatic, int8 isTcp, int8 tcpFlag, int8 isColl, int8 isValid) ;
int32 rtl8651_setAsicNaptIcmpTable_Patch(int8 forced, ipaddr_t sip, uint16 sID, uint16 gID, 
		uint16 ageSec, int8 isStatic, int16 count, int8 isColl, int8 isValid);


#endif /* RTL8651_TBLDRV_PATCH_H */


