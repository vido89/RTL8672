/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Switch core table driver statistic
* Abstract : 
* Author : chih-hua huang (chhuang@realtek.com.tw)               
* $Id: rtl8651_tblDrvStatistic.h,v 1.1.1.1 2003/08/18 05:43:22 kaohj Exp $
*/

#ifndef RTL8651_TBLDRV_STATISTIC_H
#define RTL8651_TBLDRV_STATISTIC_H


typedef struct rtl8651_tblDrvNaptCounter_s {
	uint32 protoAddCount,	/* The totalnumber of entries added by protocol stack */
		   protoDelCount,	/* The totalnumber of entries deleted by protocol */
		   drvAddCount, 	/* The total number of entries added by table driver */
		   drvDelCount,		/* The total number of entries deleted by table driver */
		   diffHashCount,	/* The total difference hash algo. count */
		   threadWriteBackCount, /* How many entries the maintenance thread write back to ASIC */
		   syncFailure,		/* How many entries sync fail */
		   addFailure;		/* add fail */

	uint16 curActiveCount,		/* Current active entries */
		   curInactiveCount,	/* Current in-active entries */
		   curInDriverCount,	/* Current entries in the driver table */
		   curInAsicCount,		/* Current entries in the ASIC table */
		   curDiffHashCount,	/* Current differ hash algo. counter */
		   curFromAsicCount; 	/* Current entries read from ASIC */
} rtl8651_tblDrvNaptCounter_t;



int32 _rtl8651_tblDrvStatisticReset();
int32 rtl8651_getTblDrvNaptTcpUdpCounter(rtl8651_tblDrvNaptCounter_t *counter);
int32 rtl8651_getTblDrvNaptIcmpCounter(rtl8651_tblDrvNaptCounter_t *counter);

#endif RTL8651_TBLDRV_STATISTIC_H





