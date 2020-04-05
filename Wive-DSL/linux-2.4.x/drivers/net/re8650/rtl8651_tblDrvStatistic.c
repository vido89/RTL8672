/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Switch core table driver statistic
* Abstract : 
* Author : chih-hua huang (chhuang@realtek.com.tw)               
* $Id: rtl8651_tblDrvStatistic.c,v 1.1.1.1 2003/08/18 05:43:22 kaohj Exp $
*/

#include "rtl8651_tblDrvLocal.h"
#include "rtl8651_tblDrvStatistic.h"


extern rtl8651_tblDrv_naptTcpUdpFlowEntry_t ** rtl8651NaptTcpUdpFlowTable;
rtl8651_tblDrv_naptIcmpFlowEntry_t ** rtl8651NaptIcmpFlowTable;
extern uint32 rtl8651ExistedNaptTcpUdpFlows;
uint32 rtl8651ExistedNaptIcmpFlows;

rtl8651_tblDrvNaptCounter_t	rtl8651_tblDrvNaptTcpUdpCounter;
rtl8651_tblDrvNaptCounter_t	rtl8651_tblDrvNaptIcmpCounter;




int32 _rtl8651_dumpNaptTcpUdpTable() {
	rtl8651_tblDrv_naptTcpUdpFlowEntry_t *trackEntry;
	uint32 flowTblIdx;

	for(flowTblIdx=0; flowTblIdx<RTL8651_TCPUDPTBL_SIZE; flowTblIdx++) {
		trackEntry=rtl8651NaptTcpUdpFlowTable[flowTblIdx];
		if (trackEntry != NULL)
			printf("[%d]: \n", flowTblIdx);
		for(; trackEntry; trackEntry=trackEntry->next) {
			printf("   %s (%d.%d.%d.%d %d %d.%d.%d.%d %d) age(%d) Asic(%d) [%d %d %d)\n", (trackEntry->tcp==1? "TCP": "UDP"),
			trackEntry->insideLocalIpAddr>>24, (trackEntry->insideLocalIpAddr&0x00ff0000)>>16,
			(trackEntry->insideLocalIpAddr&0x0000ff00)>>8, trackEntry->insideLocalIpAddr&0xff,
			trackEntry->insideLocalPort, trackEntry->dstIpAddr>>24, 
			(trackEntry->dstIpAddr&0x00ff0000)>>16, (trackEntry->dstIpAddr&0x0000ff00)>>8,
			trackEntry->dstIpAddr&0xff, trackEntry->dstPort, trackEntry->age, trackEntry->toAsic, 
			trackEntry->canAsic, trackEntry->fromAsic, trackEntry->fromDrv);
		}
	}
	printf("Napt TCP/UDP total entry: %d\n", rtl8651ExistedNaptTcpUdpFlows);
	return SUCCESS;
}


int32 _rtl8651_dumpNaptIcmpTable() {
	rtl8651_tblDrv_naptIcmpFlowEntry_t *trackEntry;
	uint32 flowTblIdx;

	for(flowTblIdx=0; flowTblIdx<RTL8651_ICMPTBL_SIZE; flowTblIdx++) {
		trackEntry=rtl8651NaptIcmpFlowTable[flowTblIdx];
		if (trackEntry != NULL)
			printf("[%d]: \n", flowTblIdx);
		for(; trackEntry; trackEntry=trackEntry->next) {
			printf("   %s (%d.%d.%d.%d %d %d.%d.%d.%d %d) age(%d) Asic(%d) [%d %d %d]\n", "ICMP",
			trackEntry->insideLocalIpAddr>>24, (trackEntry->insideLocalIpAddr&0x00ff0000)>>16,
			(trackEntry->insideLocalIpAddr&0x0000ff00)>>8, trackEntry->insideLocalIpAddr&0xff,
			trackEntry->insideLocalId, trackEntry->dstIpAddr>>24, 
			(trackEntry->dstIpAddr&0x00ff0000)>>16, (trackEntry->dstIpAddr&0x0000ff00)>>8,
			trackEntry->dstIpAddr&0xff, trackEntry->insideGlobalId, trackEntry->age, trackEntry->toAsic,
			trackEntry->canAsic, trackEntry->fromAsic, trackEntry->fromDrv);
		}
	}
	printf("Napt TCP/UDP total entry: %d\n", rtl8651ExistedNaptIcmpFlows);
	return SUCCESS;
}



int32 rtl8651_getTblDrvNaptIcmpCounter(rtl8651_tblDrvNaptCounter_t *counter) {
	rtl8651_tblDrv_naptTcpUdpFlowEntry_t *trackEntry;
	uint32 flowTblIdx;

	if (counter == (rtl8651_tblDrvNaptCounter_t *)0)
		return FAILED;
	rtl8651_tblDrvNaptIcmpCounter.curActiveCount		= 0;
	rtl8651_tblDrvNaptIcmpCounter.curInactiveCount	 	= 0;
	rtl8651_tblDrvNaptIcmpCounter.curInDriverCount	    = 0;
	rtl8651_tblDrvNaptIcmpCounter.curInAsicCount		= 0;
	rtl8651_tblDrvNaptIcmpCounter.curDiffHashCount	    = 0;
	rtl8651_tblDrvNaptIcmpCounter.curFromAsicCount	    = 0;

	for(flowTblIdx=0; flowTblIdx<RTL8651_ICMPTBL_SIZE; flowTblIdx++) {
		trackEntry = rtl8651NaptIcmpFlowTable[flowTblIdx];
		for(; trackEntry; trackEntry=trackEntry->next) {
			if (trackEntry->age > 0)
				rtl8651_tblDrvNaptIcmpCounter.curActiveCount++;
			else rtl8651_tblDrvNaptIcmpCounter.curInactiveCount++;
			if (trackEntry->toAsic == 1)
				rtl8651_tblDrvNaptIcmpCounter.curInAsicCount++;
			else rtl8651_tblDrvNaptIcmpCounter.curInDriverCount++;
			if (trackEntry->canAsic == 0)
				rtl8651_tblDrvNaptIcmpCounter.curDiffHashCount++;
			if (trackEntry->fromAsic == 1)
				rtl8651_tblDrvNaptIcmpCounter.curFromAsicCount++;

		}
	}
	*counter = rtl8651_tblDrvNaptIcmpCounter;
	return SUCCESS;
}


int32 rtl8651_getTblDrvNaptTcpUdpCounter(rtl8651_tblDrvNaptCounter_t *counter) {
	rtl8651_tblDrv_naptTcpUdpFlowEntry_t *trackEntry;
	uint32 flowTblIdx;
	
	if (counter == (rtl8651_tblDrvNaptCounter_t *)0)
		return FAILED;
	rtl8651_tblDrvNaptTcpUdpCounter.curActiveCount		 = 0;
	rtl8651_tblDrvNaptTcpUdpCounter.curInactiveCount	 = 0;
	rtl8651_tblDrvNaptTcpUdpCounter.curInDriverCount	 = 0;
	rtl8651_tblDrvNaptTcpUdpCounter.curInAsicCount		 = 0;
	rtl8651_tblDrvNaptTcpUdpCounter.curDiffHashCount	 = 0;
	rtl8651_tblDrvNaptTcpUdpCounter.curFromAsicCount	 = 0;
	
	for(flowTblIdx=0; flowTblIdx<RTL8651_TCPUDPTBL_SIZE; flowTblIdx++) {
		trackEntry = rtl8651NaptTcpUdpFlowTable[flowTblIdx];
		for(; trackEntry; trackEntry=trackEntry->next) {
			if (trackEntry->age > 0)
				rtl8651_tblDrvNaptTcpUdpCounter.curActiveCount++;
			else rtl8651_tblDrvNaptTcpUdpCounter.curInactiveCount++;
			if (trackEntry->toAsic == 1)
				rtl8651_tblDrvNaptTcpUdpCounter.curInAsicCount++;
			else rtl8651_tblDrvNaptTcpUdpCounter.curInDriverCount++;
			if (trackEntry->canAsic == 0)
				rtl8651_tblDrvNaptTcpUdpCounter.curDiffHashCount++;
			if (trackEntry->fromAsic == 1)
				rtl8651_tblDrvNaptTcpUdpCounter.curFromAsicCount++;
		}
	}
	*counter = rtl8651_tblDrvNaptTcpUdpCounter;
	return SUCCESS;
}


 

int32 _rtl8651_tblDrvStatisticReset() {

	rtl8651_tblDrvNaptTcpUdpCounter.protoAddCount   	 = 0;
	rtl8651_tblDrvNaptTcpUdpCounter.protoDelCount   	 = 0;
	rtl8651_tblDrvNaptTcpUdpCounter.drvAddCount     	 = 0;
	rtl8651_tblDrvNaptTcpUdpCounter.drvDelCount      	 = 0;
	rtl8651_tblDrvNaptTcpUdpCounter.threadWriteBackCount = 0;
	rtl8651_tblDrvNaptTcpUdpCounter.syncFailure     	 = 0;
	rtl8651_tblDrvNaptTcpUdpCounter.diffHashCount		 = 0;
	rtl8651_tblDrvNaptTcpUdpCounter.addFailure			 = 0;

	rtl8651_tblDrvNaptIcmpCounter.protoAddCount   		= 0;
	rtl8651_tblDrvNaptIcmpCounter.protoDelCount   	 	= 0;
	rtl8651_tblDrvNaptIcmpCounter.drvAddCount     	 	= 0;
	rtl8651_tblDrvNaptIcmpCounter.drvDelCount      	 	= 0;
	rtl8651_tblDrvNaptIcmpCounter.threadWriteBackCount 	= 0;
	rtl8651_tblDrvNaptIcmpCounter.syncFailure     	 	= 0;
	rtl8651_tblDrvNaptIcmpCounter.diffHashCount		 	= 0;
	rtl8651_tblDrvNaptIcmpCounter.addFailure			= 0;
	return SUCCESS;
}





