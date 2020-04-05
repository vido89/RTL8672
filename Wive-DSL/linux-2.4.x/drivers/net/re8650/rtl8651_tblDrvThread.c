/*
* --------------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Table driver maintenance task rtl8651_tblDrvThread.c
* Abstract :                                                           
*
* $Id: rtl8651_tblDrvThread.c,v 1.1.1.1 2003/08/18 05:43:22 kaohj Exp $
* $Log: rtl8651_tblDrvThread.c,v $
* Revision 1.1.1.1  2003/08/18 05:43:22  kaohj
* initial import into CVS
*
* Revision 1.1  2003/05/19 09:21:59  elvis
* Initial version to BSP
*
*
*/
#include "rtl_types.h"
#include "rtl8651_tblAsicDrv.h"
#include "phy.h"

static uint32 linkChange;

void	rtl8651_driverMaintenanceTask(void ) {
 	linkChange = 0;
 	while(1) {
		if(rtl8651_updateLinkStatus() == SUCCESS) {
			printf("Link status changed!\n");
			linkChange++;
		}
 		rtl8651_updateAsicNaptTcpUdpTable();
		rtl8651_updateAsicIcmpTable();
	
		taskDelay(sysClkRateGet()/1000); /* delay 1 msec */
	}
}

