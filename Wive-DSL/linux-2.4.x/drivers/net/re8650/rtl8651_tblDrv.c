/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Switch core table driver rtl8651_tblDrv.c
* Abstract : 
* Author : Edward Jin-Ru Chen (jzchen@realtek.com.tw)               
*
* $Id: rtl8651_tblDrv.c,v 1.1.1.1 2003/08/18 05:43:22 kaohj Exp $
* $Log: rtl8651_tblDrv.c,v $
* Revision 1.1.1.1  2003/08/18 05:43:22  kaohj
* initial import into CVS
*
* Revision 1.2  2003/05/19 09:49:58  elvis
* regular update
*
* Revision 1.1  2003/05/19 09:22:00  elvis
* Initial version to BSP
*
* Revision 1.6  2003/05/02 09:19:32  orlando
* no message
*
* Revision 1.5  2003/04/28 15:23:11  elvis
* Synchronized with v1.160
*
* Revision 1.4  2003/04/24 10:13:06  elvis
* Synchronized with v1.156
*
* Revision 1.3  2003/04/08 15:27:56  elvis
* relax some restrictions
*
* Revision 1.2  2003/04/02 01:02:05  elvis
* comment out some forwarding-cache codes
*
* Revision 1.1.1.2  2003/03/31 06:29:37  elvis
* no message
*
* Revision 1.1.1.1  2003/03/31 06:09:26  elvis
* no message
*
*
* Last: rtl8651_tblDrv.c,v 1.134 2003/03/26 07:35:33 jzchen
*/
/* Implementation note
1. All information are implemented by host order
2. Expect tblAsicDrv to process the order issue
3. 
*/

#include "assert.h"

#include "rtl_types.h"
#include "rtl8651_tblDrvLocal.h"
#include "rtl8651_tblDrvproto.h"
#include "rtl8651_tblDrvPatch.h"
#include "rtl8651_tblDrvStatistic.h"
#include "rtl8651_asicRegs.h"

// Software specification part
#define RTL8651_ARPARRANGE_NUM		64
#define RTL8651_POLLING_THRESHOLD	5
#define RTL8651_ALGSLOT_THRESHOLD 	3
 

//Internal structure definition
typedef struct rtl8651_tblDrv_macAddressEntry_s {
	ether_addr_t mac;
	uint16 vid;
	uint32 	valid:1,
			allocated:1;
} rtl8651_tblDrv_macAddressEntry_t;

/*
Store the arrangement status of current ARP table. This information does not write to ASIC but for driver reference
*/
typedef struct rtl8651_tblDrv_ipTableAsicArrangementEntry_s {
	ipaddr_t internalIp;
	ipaddr_t externalIp;
	uint32	localPublic:1,	//
			nat:1,			//
			valid:1;
} rtl8651_tblDrv_ipTableAsicArrangementEntry_t;

typedef struct rtl8651_tblDrv_arpAsicArrangementEntry_s {
	ipaddr_t netAddr;
	ipaddr_t netMask;
	uint32 valid:1;
} rtl8651_tblDrv_arpAsicArrangementEntry_t;

typedef struct rtl8651_tblDrv_routeAsicArrangementEntry_s {
	uint32 type;//Direct:0 Indirect:1 PPPoE:2 CPU:3
	ipaddr_t ipAddr;
	ipaddr_t ipMask;
	uint32 valid:1;
} rtl8651_tblDrv_routeAsicArrangementEntry_t;

typedef struct rtl8651_tblDrv_serverPortAsicArrangementEntry_s {
	uint16 valid:1;
} rtl8651_tblDrv_serverPortAsicArrangementEntry_t;

typedef struct rtl8651_tblDrv_algAsicArrangementEntry_s {
	uint16 port;
	uint16 valid:1;
} rtl8651_tblDrv_algAsicArrangementEntry_t;

typedef struct rtl8651_tblDrv_aclAsicArrangementEntry_s {
	uint16 vid;
	uint16	input:1,	//Rule belong to input filter fules
			used:1,	//Rule is used by specified vlan
			valid:1;	//Rule is assigned to specified vlan for its usage
} rtl8651_tblDrv_aclAsicArrangementEntry_t;

typedef struct rtl8651_tblDrv_rsvPortNumber_s {
	struct rtl8651_tblDrv_rsvPortNumber_s *next;
	uint16 port;	//Reserved port
} rtl8651_tblDrv_rsvPortNumber_t;

// When single bit is used to represent enable/disable, true/false, 0 means disabled or false while 1 means enabled or true

extern rtl8651_tblDrvNaptCounter_t	rtl8651_tblDrvNaptTcpUdpCounter;
extern rtl8651_tblDrvNaptCounter_t  rtl8651_tblDrvNaptIcmpCounter;


//Public defined data structure data area 
static rtl8651_tblDrvInitPara_t rtl8651_tblDrvPara;

//Public Table Driver Error Number

#define DRIVER_ASIC_ARRANGEMENT_STRUCTURE

//Table usage info of each ASIC table. Table size = corresponding table size in ASIC
rtl8651_tblDrv_macAddressEntry_t * macAddressTable;
rtl8651_tblDrv_ipTableAsicArrangementEntry_t * rtl8651IpTableAsicArrangementTable;
rtl8651_tblDrv_arpAsicArrangementEntry_t * rtl8651ArpAsicArrangementTable;
rtl8651_tblDrv_routeAsicArrangementEntry_t * rtl8651RouteAsicArrangementTable;
rtl8651_tblDrv_serverPortAsicArrangementEntry_t * rtl8651ServerPortAsicArrangementTable;
rtl8651_tblDrv_algAsicArrangementEntry_t * rtl8651AlgAsicArrangementTable; 
rtl8651_tblDrv_aclAsicArrangementEntry_t * rtl8651AclAsicArrangementTable; 

#define DRIVER_STRUCTURE

uint32 naptIpCount, natIpCount, localServerCount;

/* Age out time for ARP */
uint16 arpAgingTime = 300; /* default 5 mins */


//Hardware independent structure data area
rtl8651_tblDrv_global_t	rtl8651GlobalControl;
rtl8651_tblDrv_ethernet_t * rtl8651EthernetPortTable;
rtl8651_tblDrv_linkAggregation_t * rtl8651LinkAggregationTable;
rtl8651_tblDrv_aclMacEntry_t * rtl8651FreeMacAclList;
rtl8651_tblDrv_vlanTable_t * rtl8651VlanTable;
rtl8651_tblDrv_pppoeTable_t * rtl8651PppoeTable;
rtl8651_tblDrv_filterDbTableEntry_t * rtl8651FreeFilterDbEntryList;
rtl8651_tblDrv_filterDbTable_t * rtl8651FilterDbTable;
rtl8651_tblDrv_spanningTreeTable_t * rtl8651SpanningTreeTable;
rtl8651_tblDrv_arpEntry_t * rtl8651FreeArpList; //Linked through next pointer
rtl8651_tblDrv_arpEntry_t * rtl8651NonInterfaceArpList; //Linked through next pointer
rtl8651_tblDrv_ipIntfIpAddrEntry_t *rtl8651FreeIpIntfIpAddrList;
rtl8651_tblDrv_ipIntfEntry_t * rtl8651FreeIpIntfList; // Linked through nextIp pointer
rtl8651_tblDrv_aclIpEntry_t * rtl8651FreeIpAclList;
rtl8651_tblDrv_networkIntfTable_t * rtl8651NetworkIntfTable;
rtl8651_tblDrv_routeTable_t * rtl8651RoutingTable;
uint32 pendingRoutes;
rtl8651_tblDrv_natEntry_t * rtl8651FreeNatList;
rtl8651_tblDrv_naptServerPortEntry_t * rtl8651FreeNaptServerPortList;
rtl8651_tblDrv_naptTcpUdpFlowEntry_t ** rtl8651NaptTcpUdpFlowTable;
rtl8651_tblDrv_naptTcpUdpFlowEntry_t * rtl8651FreeNaptTcpUdpFlowEntryList;
uint16 * rtl8651NaptTcpUdpBackwardCollisionCountTable;//Whether FlowTable flows are kept in software due to backward port collision 
uint16 * rtl8651NaptTcpUdpFlowNumberTable, *rtl8651NaptTcpUdpFlowNonAsicNumberTable;
uint32 rtl8651ExistedNaptTcpUdpFlows;//For avoid interface configuration while flow existed
rtl8651_tblDrv_naptIcmpFlowEntry_t ** rtl8651NaptIcmpFlowTable;
rtl8651_tblDrv_naptIcmpFlowEntry_t * rtl8651FreeNaptIcmpFlowEntryList;
uint16 * rtl8651NaptIcmpBackwardCollisionCountTable; //Whether Flow Table flows are kept in software due to backward ID collision 
uint16 * rtl8651NaptIcmpFlowNumberTable, *rtl8651NaptIcmpFlowNonAsicNumberTable;
uint32 rtl8651ExistedNaptIcmpFlows;//For avoid interface configuration while flow existed
rtl8651_tblDrv_algSlot_t * rtl8651FreeAlgSlotList;
rtl8651_tblDrv_algEntry_t * rtl8651FreeAlgList, *rtl8651AlgList;
rtl8651_tblDrv_rsvPortNumber_t *rtl8651FreeRsvPortList, *rtl8651RsvPortList;

//Function pointer installation variable space
void (*rtl8651_linkStatusNotifier)(uint32 port, int8 linkUp);
//Driver lock and unlock function is installed instead of OS directly dependent. The ASIC driver is covered by locking range due to the assumption that user will not call driver directly.
void (*rtl8651_resourceLock)(void);
void (*rtl8651_resourceUnlock)(void);

#define STATIC_API_DEFINITION
//Static function declaration
static void _rtl8651_setEthernetPortLinkStatus(uint32 port, int8 linkUp);
static int32 _rtl8651_addVlanRefFilterDatabaseEntry(uint16 vid, ether_addr_t * macAddr, uint32 portMask);
static int32 _rtl8651_delVlanRefFilterDatabaseEntry(uint16 vid, ether_addr_t * macAddr);
static rtl8651_tblDrv_filterDbTableEntry_t * _rtl8651_getVlanFilterDatabaseEntry(uint16 vid, ether_addr_t * macAddr);
static uint32 _rtl8651_returnPortMaskPortNumber(uint32 portMask);
static void _rtl8651_updateAsicAclRuleToCpu(uint32 aclIdx);
static void _rtl8651_updateAsicAclRulePermitAll(uint32 aclIdx);
static void _rtl8651_updateAsicVlan(uint16 vid);
static void _rtl8651_updateAsicAllVlan(void);
static void _rtl8651_updateAsicArpHostRoute(rtl8651_tblDrv_arpEntry_t * allocArpEntry, uint32 routePos);
static void _rtl8651_updateAsicArpAndRoute(rtl8651_tblDrv_filterDbTableEntry_t *filterDbEntry);
static int32 _rtl8651_addLocalArp(int8 fromDrv, ipaddr_t ipAddr, ether_addr_t * macAddr, uint32 ifIdx, uint32 port);
static int32 _rtl8651_addNonLocalArp(ipaddr_t ipAddr, ether_addr_t * macAddr, int8 * ifName, uint32 port);
static int32 _rtl8651_delLocalArp(int8 fromDrv, ipaddr_t ipAddr, uint32 ifIdx);
static int32 _rtl8651_delNonLocalArp(ipaddr_t ipAddr);
static void _rtl8651_removeAllDrvArp(uint32 port);
static void _rtl8651_updateGidxRegister(void);
static int32 _rtl8651_updateAsicRoute(uint32 rtIdx, uint32 rtArgIdx);
static void _rtl8651_lockDefaultRoute(void);
static void _rtl8651_unlockDefaultRoute(void);
static int8 _rtl8651_sameMacRule(rtl8651_tblDrvMacAclRule_t * rule1, rtl8651_tblDrvMacAclRule_t * rule2);
static int8 _rtl8651_sameIpRule(rtl8651_tblDrvIpAclRule_t * rule1, rtl8651_tblDrvIpAclRule_t * rule2);
static uint32 _rtl8651_countAlgSlotAllocateNumber(rtl8651_tblDrv_algEntry_t * algPtr);
static rtl8651_tblDrv_naptTcpUdpFlowEntry_t *_rtl8651_getAsicNaptTcpUdpDynamicFlowToTable(uint32 flowTblIdx);
static void _rtl8651_updateAsicAlg(void);
static int32 _rtl8651_flushNaptFlow(void);
static int32 _rtl8651_tblDrvMemAlloc(void);
static int32 _rtl8651_tblDrvListCollect(void);
static int32 _rtl8651_tblDrvArrayInit(void);


//Macros Definition
#define RTL8651_LOCK_RESOURCE()		if (rtl8651_resourceLock) rtl8651_resourceLock();
#define RTL8651_UNLOCK_RESOURCE()	if (rtl8651_resourceUnlock) rtl8651_resourceUnlock();
#define GET_VID_BY_NETIFIDX(_vid, netIdx)\
{\
	uint32 pppoeIdx;\
 	if (rtl8651NetworkIntfTable[netIdx].linkLayerType == RTL8651_LL_VLAN)\
 		(_vid) = rtl8651NetworkIntfTable[netIdx].linkLayerIndex;\
	else {\
		assert(rtl8651NetworkIntfTable[netIdx].linkLayerType == RTL8651_LL_PPPOE);\
		for(pppoeIdx = 0; pppoeIdx<RTL8651_PPPOE_NUMBER; pppoeIdx++)\
			if(rtl8651PppoeTable[pppoeIdx].valid == 1 && rtl8651PppoeTable[pppoeIdx].objectId == rtl8651NetworkIntfTable[netIdx].linkLayerIndex)\
				break;\
		assert(pppoeIdx<RTL8651_PPPOE_NUMBER);\
		(_vid) = rtl8651PppoeTable[pppoeIdx].vid;\
	}\
	assert(rtl8651VlanTable[rtl8651_vlanTableIndex(_vid)].valid == 1);\
	assert(rtl8651VlanTable[rtl8651_vlanTableIndex(_vid)].vid == _vid);\
}
#define RTL8651_SET_ASIC_IP_TABLE(ipIdx) {\
\
	rtl8651_setAsicExtIntIpTable(ipIdx, rtl8651IpTableAsicArrangementTable[ipIdx].externalIp, rtl8651IpTableAsicArrangementTable[ipIdx].internalIp, rtl8651IpTableAsicArrangementTable[ipIdx].localPublic == 1 ? TRUE: FALSE, rtl8651IpTableAsicArrangementTable[ipIdx].nat == 1 ? TRUE: FALSE);\
}
#define RTL8651_DEL_ASIC_IP_TABLE(ipIdx) {\
\
	rtl8651_delAsicExtIntIpTable(ipIdx);\
}




#define GENERIC_APIS
// Generic APIs

int32 rtl8651_installResourceLock(void (*lockFunction)(void)) {
	rtl8651_resourceLock = lockFunction;
	return SUCCESS;
}

int32 rtl8651_installResourceUnlock(void (*unlockFunction)(void)) {
	rtl8651_resourceUnlock = unlockFunction;
	return SUCCESS;
}


int32 rtl8651_installEtherrnetLinkStatusNotifier(void (*notifier)(uint32 port, int8 linkUp)){
	rtl8651_linkStatusNotifier = notifier;
	return SUCCESS;
}

static void _rtl8651_setEthernetPortLinkStatus(uint32 port, int8 linkUp) {
	int8 ethernetNotify;
	uint32 i;
	
	if(port >= RTL8651_PORT_NUMBER)
		return;

	RTL8651_LOCK_RESOURCE();//Lock resource
	
	if(rtl8651_linkStatusNotifier && rtl8651EthernetPortTable[port].linkUp != (linkUp==TRUE? 1: 0))
		ethernetNotify = TRUE;
	else
		ethernetNotify = FALSE;
	//change port status
	rtl8651EthernetPortTable[port].linkUp = linkUp==TRUE? 1: 0;

	//Update aggregator status
	for(i = 0; i<RTL8651_PORT_NUMBER; i++)//Check aggregator
		if(rtl8651LinkAggregationTable[i].ethernetPortMask & (1<<port)) {
			if(linkUp==TRUE)
				rtl8651LinkAggregationTable[i].ethernetPortUpStatus |= (1<<port);
			else
				rtl8651LinkAggregationTable[i].ethernetPortUpStatus &= ~(1<<port);
		}
	//update VLAN member port status
	for(i=0; i<RTL8651_VLAN_NUMBER; i++) {
		if(rtl8651VlanTable[i].memberPortMask & (1<<port)) {
			if(linkUp==TRUE)
				rtl8651VlanTable[i].memberPortUpStatus |= (1<<port);
			else
				rtl8651VlanTable[i].memberPortUpStatus &= ~(1<<port);
		}
	}
	//remove all arp entries learned from this port.
	if(linkUp == FALSE)
		_rtl8651_removeAllDrvArp(port);
	
	RTL8651_UNLOCK_RESOURCE();//Unlock resource

	//notify upper layer for status change.	
	if(ethernetNotify == TRUE)
		rtl8651_linkStatusNotifier(port, linkUp);
}

static void _rtl8651_updateAsicPHY(uint32 port) {
	rtl8651_setAsicEthernetPHY(port, rtl8651EthernetPortTable[port].autoNegotiation==1? TRUE: FALSE, rtl8651EthernetPortTable[port].autoAdvCapability, rtl8651EthernetPortTable[port].speed, rtl8651EthernetPortTable[port].duplex==1? TRUE: FALSE);
}

int32 rtl8651_getEthernetPortLinkStatus(uint32 port, int8 * linkUp, uint16 *speed, int8 *fullduplex, int8 *autoNeg){
	if(port >= RTL8651_PORT_NUMBER) {
		return(TBLDRV_EINVPORT); //Invalid Port Number
	}
	RTL8651_LOCK_RESOURCE();//Lock resource
	if(linkUp)
		*linkUp = rtl8651EthernetPortTable[port].linkUp==1? TRUE: FALSE;
	if(speed){
		int32 _speed[]={ 10, 100, 1000 };
		*speed = _speed[rtl8651EthernetPortTable[port].speed];
	}
	if(fullduplex)
		*fullduplex = rtl8651EthernetPortTable[port].duplex==1? TRUE: FALSE;
	if(autoNeg)
		*autoNeg = rtl8651EthernetPortTable[port].autoNegotiation==1? TRUE: FALSE;

	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

int32 rtl8651_setEthernetPortDuplexMode(uint32 port, int8 fullDuplex) {
	uint32 i;

	if(port >= RTL8651_PORT_NUMBER) {
		return(TBLDRV_EINVPORT); //Invalid Port Number
	}
	RTL8651_LOCK_RESOURCE();//Lock resource

	/*cfliu: If this port has been aggregated, forbid change to half duplex mode*/
	if(!fullDuplex){
		//Check whether this port is a member of aggregator 
		for(i=0; i<RTL8651_PORT_NUMBER; i++) {
			if(rtl8651LinkAggregationTable[i].aggregated == 1 &&
				rtl8651LinkAggregationTable[i].ethernetPortMask & ( 1 << port)) {
				return(TBLDRV_ENOTALLOWPORTAGGREGATED);
			}
		}
	}
	rtl8651EthernetPortTable[port].duplex = fullDuplex==TRUE? 1: 0;
	_rtl8651_updateAsicPHY(port);
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

int32 rtl8651_setEthernetPortSpeed(uint32 port, uint32 speed) {
	uint32 newSpeed, i;

	if (port >= RTL8651_PORT_NUMBER) {
		return(TBLDRV_EINVPORT);	//Invalid Port Number
	}
	if (speed != 10 && speed != 100) {
		return(TBLDRV_EINVVAL); //Invalid Input value
	}
	RTL8651_LOCK_RESOURCE();//Lock resource
	switch(speed) {
		case 10:
			newSpeed = 0;
			break;
		case 100:
			newSpeed = 1;
			break;
		default:
			assert(0);//Speed mismatch should return failed
	}

	/*cfliu: if this port is a member of an aggregator, forbid link speed to be changed*/
	if(rtl8651EthernetPortTable[port].speed != newSpeed){
		for(i=0; i<RTL8651_PORT_NUMBER; i++) {
			if(rtl8651LinkAggregationTable[i].aggregated == 1 &&
				rtl8651LinkAggregationTable[i].ethernetPortMask & ( 1 << port)) {
				return(TBLDRV_ENOTALLOWPORTAGGREGATED);
			}
	    }
	}
	rtl8651EthernetPortTable[port].speed = newSpeed;
	_rtl8651_updateAsicPHY(port);
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

int32 rtl8651_setEthernetPortAutoNegotiation(uint32 port, int8 autoNegotiation, uint32 advCapability) {
	int32 i;
	
	if(port >= RTL8651_PORT_NUMBER) {
		return(TBLDRV_EINVPORT); //Invalid Port Number
	}
	RTL8651_LOCK_RESOURCE();//Lock resource
	/* If the port is a member of an aggregator, forbid setting autoNegotation */
	for(i=0; i<RTL8651_PORT_NUMBER; i++) {
		if (rtl8651LinkAggregationTable[i].aggregated == 1 &&
			rtl8651LinkAggregationTable[i].ethernetPortMask & ( 1 << port)) {
				return(TBLDRV_ENOTALLOWPORTAGGREGATED);

		}
	}
	rtl8651EthernetPortTable[port].autoNegotiation = autoNegotiation==TRUE? 1: 0;
	rtl8651EthernetPortTable[port].autoAdvCapability = advCapability;
	_rtl8651_updateAsicPHY(port);
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

int32 rtl8651_setEthernetPortBandwidthControl(uint32 port, int8 input, uint32 rate) {
	uint32 asicRate;
	
	if(port >= RTL8651_PORT_NUMBER) {
		return(TBLDRV_EINVPORT); //Invalid Port Number
	}
	RTL8651_LOCK_RESOURCE();//Lock resource

	switch(rate){
		case RTL8651_BC_FULL:
			asicRate = BW_FULL_RATE;
			break;
		case RTL8651_BC_128K:
			asicRate = BW_128K;
			break;
		case RTL8651_BC_256K:
			asicRate = BW_256K;
			break;
		case RTL8651_BC_512K:
			asicRate = BW_512K;
			break;
		case RTL8651_BC_1M:
			asicRate = BW_1M;
			break;
		case RTL8651_BC_2M:
			asicRate = BW_2M;
			break;
		case RTL8651_BC_4M:
			asicRate = BW_4M;
			break;
		case RTL8651_BC_8M:
			asicRate = BW_8M;
			break;
		default:
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return(TBLDRV_EINVVAL); //Unknown parameter
	}
	if(input == TRUE)
		rtl8651EthernetPortTable[port].inputBandwidthControl = rate;
	else
		rtl8651EthernetPortTable[port].outputBandwidthControl = rate;
	rtl8651_setAsicEthernetBandwidthControl(port, input, asicRate);
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

/*
Driver assumptions

VLAN 1, Filter database 0 and Spanning tree instance 0 must exist after initialization
*/
int32 rtl8651_addSpanningTreeInstance(uint16 sid) {
	uint32 i;
	
	if (sid >= RTL8651_FDB_NUMBER)
		return(TBLDRV_EINVSID); //Invalid Spanning Tree Instance ID
	RTL8651_LOCK_RESOURCE();//Lock resource
	if (rtl8651SpanningTreeTable[sid].valid == 1) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_ESIDEXIST);  //Entry already exists
	}
	for(i = 0; i<RTL8651_FDB_NUMBER; i++) 
		if(rtl8651SpanningTreeTable[i].valid == 1)
			break;
	if(i==RTL8651_FDB_NUMBER)//First installed spanning tree instance
		rtl8651_setAsicSpanningEnable(FALSE);
	rtl8651SpanningTreeTable[sid].valid = 1;
	rtl8651SpanningTreeTable[sid].protocolWorking = 0;//Protocol does not working
	for(i=0;i<RTL8651_PORT_NUMBER; i++)
		rtl8651SpanningTreeTable[sid].portState[i] = 1;//Blocking at initial time
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

int32 rtl8651_setSpanningTreeInstanceProtocolWorking(uint16 sid, int8 working) {
	uint32 i, workingCount;
	int8 previousWorking;
	
	if(sid >= RTL8651_STI_NUMBER) 
		return(TBLDRV_EINVSID); //Spanning tree identity out of range
	RTL8651_LOCK_RESOURCE();//Lock resource
	
	if(rtl8651SpanningTreeTable[sid].valid == 0) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EINVSID); //Spanning tree not exist
	}
	previousWorking = rtl8651SpanningTreeTable[sid].protocolWorking == 1? TRUE: FALSE;
	rtl8651SpanningTreeTable[sid].protocolWorking = working == TRUE? 1: 0;
	workingCount = 0;
	for(i=0; i< RTL8651_STI_NUMBER; i++)
		if(rtl8651SpanningTreeTable[i].valid == 1 && rtl8651SpanningTreeTable[i].protocolWorking == 1)
			workingCount++;
	if(previousWorking == TRUE && working == FALSE && workingCount == 0)
		rtl8651_setAsicSpanningEnable(FALSE);
	if(previousWorking == FALSE && working == TRUE && workingCount == 1)
		rtl8651_setAsicSpanningEnable(TRUE);
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

//Configure VLAN specific port (link aggregator) port state. 1. Use vid to find fid. 2. Use fid to find MSID. 3. Configure MSID port state
int32 rtl8651_setSpanningTreeInstancePortState(uint16 stid, uint16 port, uint8 portState) {
	uint16 i, vid;
	
	if (port >= RTL8651_PORT_NUMBER)
		return(TBLDRV_EINVPORT); //Invalid Port Number
	if (stid >= RTL8651_STI_NUMBER)
		return(TBLDRV_EINVSID); //Invalid SID
	if (portState > 4)
		return(TBLDRV_EINVVAL); //Invalid parameter
	
	RTL8651_LOCK_RESOURCE();//Lock resource
	if(rtl8651LinkAggregationTable[port].ethernetPortMask == 0) {//Configure invalid aggregator
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EINVAGGREATOR); //Invalid aggregator
	}
	if(rtl8651SpanningTreeTable[stid].valid == 0) {// Specify invalid spanning tree instance id
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EINVSID); //Invalid SID
	}
	
	//Configure all link aggraged port with the same port
	for(i = 0; i< RTL8651_PORT_NUMBER; i++)
		if(rtl8651LinkAggregationTable[port].ethernetPortMask & (1<<i))
			rtl8651SpanningTreeTable[stid].portState[i] = portState; //Set member of link aggregator port state
	//Configure port state into actual VLAN table (8651 model)
	for(vid = 0; vid<RTL8651_VLAN_NUMBER; vid++)
		if(rtl8651VlanTable[vid].valid == 1) {
			assert(rtl8651VlanTable[vid].fid<RTL8651_FDB_NUMBER);
			assert(rtl8651FilterDbTable[rtl8651VlanTable[vid].fid].valid == 1);
			assert(rtl8651FilterDbTable[rtl8651VlanTable[vid].fid].sid<RTL8651_STI_NUMBER);
			assert(rtl8651SpanningTreeTable[rtl8651FilterDbTable[rtl8651VlanTable[vid].fid].sid].valid == 1);
			if(rtl8651FilterDbTable[rtl8651VlanTable[vid].fid].sid == stid)
				_rtl8651_updateAsicVlan(rtl8651VlanTable[vid].vid) ;
		}
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

int32 rtl8651_delSpanningTreeInstance(uint16 sid) {
	uint16 fid;

	if (sid >= RTL8651_FDB_NUMBER || rtl8651SpanningTreeTable[sid].valid == 0)
		return(TBLDRV_EINVSID); //Invalid SID: either out of SID or SID doesn't exist
	
	RTL8651_LOCK_RESOURCE();//Lock resource
	for(fid=0; fid<RTL8651_FDB_NUMBER; fid++)
		if(rtl8651FilterDbTable[fid].valid == 1 && rtl8651FilterDbTable[fid].sid == sid) {
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return(TBLDRV_ESIDREFERENCED); //Spanning tree instance is referenced by some filter database
		}
	rtl8651SpanningTreeTable[sid].valid = 0;//Invalidate corresponding spanning tree instance
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

int32 rtl8651_addFilterDatabase(uint16 fid) {
	uint32 i;

	if(fid >= RTL8651_FDB_NUMBER) 
		return(TBLDRV_EINVFID); //Filter database id out of range
	RTL8651_LOCK_RESOURCE();//Lock resource
	
	if(rtl8651FilterDbTable[fid].valid == 1) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EFIDEXIST); //FID exist, cannot add
	}
	rtl8651FilterDbTable[fid].valid = 1;	//Enable filter database
	assert(rtl8651SpanningTreeTable[0].valid);//Spanning tree instance 0 should exist
	rtl8651FilterDbTable[fid].sid = 0;	//Using first spanning tree database
	for(i=0; i<RTL8651_L2TBL_ROW; i++)
		rtl8651FilterDbTable[fid].database[i] = NULL;
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}


int32 rtl8651_specifyFilterDatabaseSpanningTreeInstance(uint16 fid, uint16 sid) {
	int32 retval = SUCCESS;
	
	if(fid >= RTL8651_FDB_NUMBER)
		return(TBLDRV_EINVFID); //Invalid FID
	if (sid >= RTL8651_FDB_NUMBER) {
		return(TBLDRV_EINVSID); //Invalid SID
	}
	RTL8651_LOCK_RESOURCE();//Lock resource
	
	if(rtl8651FilterDbTable[fid].valid == 0){
		retval = TBLDRV_EINVFID;
	}else if(rtl8651SpanningTreeTable[sid].valid == 0)
		retval = TBLDRV_EINVSID;
	else
		rtl8651FilterDbTable[fid].sid = sid;	//Using specified spanning tree database
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return retval;
}

int32 rtl8651_delFilterDatabase(uint16 fid) {
	uint32 i;

	if(fid >= RTL8651_FDB_NUMBER) 
		return(TBLDRV_EINVFID); //filter database out of range
	RTL8651_LOCK_RESOURCE();//Lock resource
	
	if(rtl8651FilterDbTable[fid].valid == 0) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EINVFID); //Invlaid FID
	}
#if 0
	for(i=0; i<RTL8651_VLAN_NUMBER; i++)
		if(rtl8651VlanTable[i].fid == fid) {
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return(TBLDRV_EFIDREFERENCED); //Filter database still referenced by some VLAN
		}
#endif
	//Forbid filter database delete when VLAN reference
	for(i=0; i<RTL8651_VLAN_NUMBER; i++)
		if(rtl8651VlanTable[i].valid == 1 && rtl8651VlanTable[i].fid == fid) {//VLAN using corresponding filter database
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return(TBLDRV_EFIDREFERENCED);
		}
	for(i=0; i<RTL8651_L2TBL_ROW; i++) //Forbid database deletion when database entry exist
		if(rtl8651FilterDbTable[fid].database[i]) {
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return(TBLDRV_EFIDREFERENCED);
		}
	rtl8651FilterDbTable[fid].valid = 0;	//Disable filter database
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}


int32 rtl8651_addFilterDatabaseEntry(uint16 fid, ether_addr_t * macAddr, uint32 type, uint32 portMask) {
	uint32 i;
	int8	occupied[RTL8651_L2TBL_COLUMN] ;
	rtl8651_tblDrv_filterDbTableEntry_t * tempFilterDbEntry;
	
	if((type != RTL8651_FORWARD_MAC && type != RTL8651_DSTBLOCK_MAC && 
		 type != RTL8651_SRCBLOCK_MAC && type != RTL8651_TOCPU_MAC)) {
		return(TBLDRV_EINVVAL); //Invalid parameter
	}
	if (fid >= RTL8651_FDB_NUMBER)
		return(TBLDRV_EINVFID);
	if (macAddr == NULL)
		return(TBLDRV_EINVVAL);

	RTL8651_LOCK_RESOURCE();//Lock resource
	if(rtl8651FilterDbTable[fid].valid == 0) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EINVFID);
	}
	//Check whether entry exist
	for(i=0; i< RTL8651_L2TBL_COLUMN; i++)
		occupied[i] = FALSE;
	tempFilterDbEntry = rtl8651FilterDbTable[fid].database[rtl8651_filterDbIndex(macAddr)];
	while(tempFilterDbEntry) {
		if(tempFilterDbEntry->configToAsic == 1)
			occupied[tempFilterDbEntry->asicPos] = TRUE;
		if(memcmp(&tempFilterDbEntry->macAddr, macAddr, sizeof(ether_addr_t)) == 0) {//Entry found
			assert(tempFilterDbEntry->userConfig == 0 ? tempFilterDbEntry->process == 0 : 1);
			if(tempFilterDbEntry->userConfig == 0 && //Not previously user configured
				type == RTL8651_FORWARD_MAC && //User configured to forward 
				tempFilterDbEntry->memberPortMask == portMask) {//Forwarding port map equal to dynamic installed port map 
				tempFilterDbEntry->userConfig = 1;
				RTL8651_UNLOCK_RESOURCE();//Unlock resource
				return SUCCESS;
			}
			else {
				RTL8651_UNLOCK_RESOURCE();//Unlock resource
				return (TBLDRV_EENTRYEXIST);
			}
		}
		tempFilterDbEntry = tempFilterDbEntry->next;
	}
	if(rtl8651FreeFilterDbEntryList == NULL) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_ENOFREEBUF);
	}
	
	//Allocate free entry
	tempFilterDbEntry = rtl8651FreeFilterDbEntryList;
	rtl8651FreeFilterDbEntryList = rtl8651FreeFilterDbEntryList->next;
	tempFilterDbEntry->next = NULL;
	//Initial entry content
	tempFilterDbEntry->userConfig = 1;
	tempFilterDbEntry->macAddr = *macAddr;
	tempFilterDbEntry->configToAsic = 0;
	tempFilterDbEntry->refCount = 0;
	switch(type) {
		case RTL8651_FORWARD_MAC:
			tempFilterDbEntry->process = RTL8651_FORWARD_MAC;
			tempFilterDbEntry->memberPortMask = portMask;
		break;
		case RTL8651_DSTBLOCK_MAC:
			tempFilterDbEntry->process = RTL8651_DSTBLOCK_MAC;
			tempFilterDbEntry->memberPortMask = 0;
		break;
		case RTL8651_SRCBLOCK_MAC:
			tempFilterDbEntry->process = RTL8651_SRCBLOCK_MAC;
			tempFilterDbEntry->memberPortMask = 0;
		break;
		case RTL8651_TOCPU_MAC:
			tempFilterDbEntry->process = RTL8651_TOCPU_MAC;
			tempFilterDbEntry->memberPortMask = 0;
		break;
	}
	//Find empty slot and try to occupy it
	for(i = 0; i< RTL8651_L2TBL_COLUMN; i++) {
		if(occupied[i] == FALSE) {
			tempFilterDbEntry->configToAsic = 1;
			tempFilterDbEntry->asicPos = i;
			switch(tempFilterDbEntry->process) {
				case RTL8651_FORWARD_MAC:
				case RTL8651_DSTBLOCK_MAC:
					rtl8651_setAsicL2Table_Patch(rtl8651_filterDbIndex(macAddr), tempFilterDbEntry->asicPos, &tempFilterDbEntry->macAddr, FALSE, FALSE, tempFilterDbEntry->memberPortMask, arpAgingTime, TRUE, 0);
					break;
				case RTL8651_TOCPU_MAC:
					rtl8651_setAsicL2Table_Patch(rtl8651_filterDbIndex(macAddr), tempFilterDbEntry->asicPos, &tempFilterDbEntry->macAddr, TRUE, FALSE, tempFilterDbEntry->memberPortMask, arpAgingTime, TRUE, 0);
					break;
				case RTL8651_SRCBLOCK_MAC:
					rtl8651_setAsicL2Table_Patch(rtl8651_filterDbIndex(macAddr), tempFilterDbEntry->asicPos, &tempFilterDbEntry->macAddr, FALSE, TRUE, tempFilterDbEntry->memberPortMask, arpAgingTime, TRUE, 0);
					break;
			}
			break; //for-loop
		}
	}
	//Hook to filter database
	tempFilterDbEntry->next = rtl8651FilterDbTable[fid].database[rtl8651_filterDbIndex(macAddr)];
	rtl8651FilterDbTable[fid].database[rtl8651_filterDbIndex(macAddr)] = tempFilterDbEntry;
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

int32 rtl8651_delFilterDatabaseEntry(uint16 fid, ether_addr_t * macAddr) {
	rtl8651_tblDrv_filterDbTableEntry_t * tempFilterDbEntry;
	
	if (fid >=RTL8651_FDB_NUMBER)
		return(TBLDRV_EINVFID);
	if (macAddr == NULL) 
		return (TBLDRV_EINVVAL);
	RTL8651_LOCK_RESOURCE();//Lock resource

	if(rtl8651FilterDbTable[fid].valid == 0) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EINVFID);
	}
	//Check whether entry exist
	tempFilterDbEntry = rtl8651FilterDbTable[fid].database[rtl8651_filterDbIndex(macAddr)];
	while(tempFilterDbEntry) {
		if(memcmp(&tempFilterDbEntry->macAddr, macAddr, sizeof(ether_addr_t)) == 0) {//Entry found
			assert(tempFilterDbEntry->userConfig == 0 ? tempFilterDbEntry->process == 0 : 1);
			if(tempFilterDbEntry->userConfig == 0) {
				RTL8651_UNLOCK_RESOURCE();//Unlock resource
				return(TBLDRV_EENTRYNOTFOUND); //userConfig=0 must be hidden from users
			}
			else {// Implied (tempFilterDbEntry->userConfig == 1)
				if (tempFilterDbEntry->refCount > 0) {
					tempFilterDbEntry->userConfig = 0;
					RTL8651_UNLOCK_RESOURCE();//Unlock resource
					return SUCCESS;
				}
				else {
					if(tempFilterDbEntry->configToAsic == 1) {
						rtl8651_tblDrv_filterDbTableEntry_t * trackFilterDbEntry;
						trackFilterDbEntry = rtl8651FilterDbTable[fid].database[rtl8651_filterDbIndex(macAddr)];
						while(trackFilterDbEntry && trackFilterDbEntry->configToAsic == 1) 
							trackFilterDbEntry = trackFilterDbEntry->next;
						if(trackFilterDbEntry == (rtl8651_tblDrv_filterDbTableEntry_t *)NULL) //No entry pending
							rtl8651_delAsicL2Table(rtl8651_filterDbIndex(macAddr), tempFilterDbEntry->asicPos);
						else {//Write pending entry into ASIC
							trackFilterDbEntry->configToAsic = 1;
							trackFilterDbEntry->asicPos = tempFilterDbEntry->asicPos;
							switch(trackFilterDbEntry->process) {
								case RTL8651_FORWARD_MAC:
								case RTL8651_DSTBLOCK_MAC:
									rtl8651_setAsicL2Table_Patch(rtl8651_filterDbIndex(macAddr), trackFilterDbEntry->asicPos, &trackFilterDbEntry->macAddr, FALSE, FALSE, trackFilterDbEntry->memberPortMask, arpAgingTime, TRUE, 0);
									break;
								case RTL8651_TOCPU_MAC:
									rtl8651_setAsicL2Table_Patch(rtl8651_filterDbIndex(macAddr), trackFilterDbEntry->asicPos, &trackFilterDbEntry->macAddr, TRUE, FALSE, trackFilterDbEntry->memberPortMask, arpAgingTime, TRUE, 0);
									break;
								case RTL8651_SRCBLOCK_MAC: 
									rtl8651_setAsicL2Table_Patch(rtl8651_filterDbIndex(macAddr), trackFilterDbEntry->asicPos, &trackFilterDbEntry->macAddr, FALSE, TRUE, trackFilterDbEntry->memberPortMask, arpAgingTime, TRUE, 0);
									break;
							}
							//Write Arp pending entry to ASIC, by chhuang
							_rtl8651_updateAsicArpAndRoute(trackFilterDbEntry);
						}
					}
					//Remove entry from list
					{
						rtl8651_tblDrv_filterDbTableEntry_t * trackTempFilterDbEntry;
						if(tempFilterDbEntry == rtl8651FilterDbTable[fid].database[rtl8651_filterDbIndex(macAddr)]) {
							rtl8651FilterDbTable[fid].database[rtl8651_filterDbIndex(macAddr)] = tempFilterDbEntry->next;
						}
						else {//To be deleted entry is not at the head
							trackTempFilterDbEntry = rtl8651FilterDbTable[fid].database[rtl8651_filterDbIndex(macAddr)];
							while(trackTempFilterDbEntry && trackTempFilterDbEntry->next != tempFilterDbEntry)
								trackTempFilterDbEntry = trackTempFilterDbEntry->next;
							assert(trackTempFilterDbEntry);
							trackTempFilterDbEntry->next = tempFilterDbEntry->next;
						}
						tempFilterDbEntry->next = rtl8651FreeFilterDbEntryList;
						rtl8651FreeFilterDbEntryList = tempFilterDbEntry;
					}
					RTL8651_UNLOCK_RESOURCE();//Unlock resource
					return SUCCESS;
				}
			}
		}
		tempFilterDbEntry = tempFilterDbEntry->next;
	}
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return (TBLDRV_EENTRYNOTFOUND);
}



static int32 _rtl8651_addVlanRefFilterDatabaseEntry(uint16 vid, ether_addr_t * macAddr, uint32 portMask) {
	uint32 i;
	int8	occupied[RTL8651_L2TBL_COLUMN] ;
	rtl8651_tblDrv_filterDbTableEntry_t * tempFilterDbEntry;
	
	if(vid < 1 || vid > 4094 || macAddr == NULL)
		return FAILED;
	if(rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].valid == 0 ||	rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].vid != vid) 
		return FAILED;//VLAN not exist
	assert(rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].fid < RTL8651_FDB_NUMBER);
	assert(rtl8651FilterDbTable[rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].fid].valid == 1);
	//Check whether entry exist
	for(i=0; i< RTL8651_L2TBL_COLUMN; i++)
		occupied[i] = FALSE;
	tempFilterDbEntry = rtl8651FilterDbTable[rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].fid].database[rtl8651_filterDbIndex(macAddr)];
	while(tempFilterDbEntry) {
		if(tempFilterDbEntry->configToAsic == 1)
			occupied[tempFilterDbEntry->asicPos] = TRUE;
		if(memcmp(&tempFilterDbEntry->macAddr, macAddr, sizeof(ether_addr_t)) == 0) {//Entry found
			assert(tempFilterDbEntry->process == 0);
			if(tempFilterDbEntry->userConfig == 0 || //Not previously user configured
				tempFilterDbEntry->refCount > 0 || //Already allocated for reference entry
 				(tempFilterDbEntry->userConfig == 1 && tempFilterDbEntry->process == 0 && tempFilterDbEntry->memberPortMask == portMask)) {//Forwarding port map equal to static stored port map 
				tempFilterDbEntry->refCount++;
				return SUCCESS;
			}
			else {
				return FAILED;
			}
		}
		tempFilterDbEntry = tempFilterDbEntry->next;
	}
	if(rtl8651FreeFilterDbEntryList == NULL)
		return FAILED;//No more free entry for new entry installation
	
	//Allocate free entry
	tempFilterDbEntry = rtl8651FreeFilterDbEntryList;
	rtl8651FreeFilterDbEntryList = rtl8651FreeFilterDbEntryList->next;
	tempFilterDbEntry->next = NULL;
	//Initial entry content
	tempFilterDbEntry->macAddr = *macAddr;
	tempFilterDbEntry->configToAsic = 0;
	tempFilterDbEntry->process = RTL8651_FORWARD_MAC;
	tempFilterDbEntry->memberPortMask = portMask;
	tempFilterDbEntry->userConfig = 0;
	tempFilterDbEntry->refCount = 1;
	
	//Find empty slot and try to occupy it
	for(i = 0; i< RTL8651_L2TBL_COLUMN; i++) {
		if(occupied[i] == FALSE) {
			tempFilterDbEntry->configToAsic = 1;
			tempFilterDbEntry->asicPos = i;
			switch(tempFilterDbEntry->process) {
				case RTL8651_FORWARD_MAC:
				case RTL8651_DSTBLOCK_MAC:
					rtl8651_setAsicL2Table_Patch(rtl8651_filterDbIndex(macAddr), tempFilterDbEntry->asicPos, &tempFilterDbEntry->macAddr, FALSE, FALSE, tempFilterDbEntry->memberPortMask, arpAgingTime, TRUE, 0);
					break;
				case RTL8651_SRCBLOCK_MAC:
					rtl8651_setAsicL2Table_Patch(rtl8651_filterDbIndex(macAddr), tempFilterDbEntry->asicPos, &tempFilterDbEntry->macAddr, TRUE, FALSE, tempFilterDbEntry->memberPortMask, arpAgingTime, TRUE, 0);
					break;
				case RTL8651_TOCPU_MAC:
					rtl8651_setAsicL2Table_Patch(rtl8651_filterDbIndex(macAddr), tempFilterDbEntry->asicPos, &tempFilterDbEntry->macAddr, FALSE, TRUE, tempFilterDbEntry->memberPortMask, arpAgingTime, TRUE, 0);
					break;
			}
			break;
		}
	}
	//Hook to filter database
	tempFilterDbEntry->next = rtl8651FilterDbTable[rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].fid].database[rtl8651_filterDbIndex(macAddr)];
	rtl8651FilterDbTable[rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].fid].database[rtl8651_filterDbIndex(macAddr)] = tempFilterDbEntry;
	return SUCCESS;
}

static int32 _rtl8651_delVlanRefFilterDatabaseEntry(uint16 vid, ether_addr_t * macAddr) {
	rtl8651_tblDrv_filterDbTableEntry_t * tempFilterDbEntry;

	if(vid < 1 || vid > 4094 || macAddr == NULL)
		return FAILED;
	//If no such VLAN exist, 
	if(rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].valid == 0 ||	rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].vid != vid)
		return FAILED;//vlan doest not exist
	assert(rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].fid < RTL8651_FDB_NUMBER);
	assert(rtl8651FilterDbTable[rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].fid].valid == 1);
	//Check whether entry exist
	tempFilterDbEntry = rtl8651FilterDbTable[rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].fid].database[rtl8651_filterDbIndex(macAddr)];
	while(tempFilterDbEntry) {
		if(memcmp(&tempFilterDbEntry->macAddr, macAddr, sizeof(ether_addr_t)) == 0) {//Entry found
			assert(tempFilterDbEntry->process == 0);
			if(tempFilterDbEntry->userConfig == 1 || tempFilterDbEntry->refCount > 1) {
				tempFilterDbEntry->refCount--;
				return SUCCESS;
			}
			else {
				if(tempFilterDbEntry->configToAsic == 1) {
					rtl8651_tblDrv_filterDbTableEntry_t * trackFilterDbEntry;
					trackFilterDbEntry = rtl8651FilterDbTable[rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].fid].database[rtl8651_filterDbIndex(macAddr)];
					while(trackFilterDbEntry && trackFilterDbEntry->configToAsic == 1) 
						trackFilterDbEntry = trackFilterDbEntry->next;
					if(trackFilterDbEntry == (rtl8651_tblDrv_filterDbTableEntry_t *)NULL) //No entry pending
						rtl8651_delAsicL2Table(rtl8651_filterDbIndex(macAddr), tempFilterDbEntry->asicPos);
					else {//Write pending entry into ASIC
						trackFilterDbEntry->configToAsic = 1;
						trackFilterDbEntry->asicPos = tempFilterDbEntry->asicPos;
						rtl8651_setAsicL2Table_Patch(rtl8651_filterDbIndex(&trackFilterDbEntry->macAddr), trackFilterDbEntry->asicPos, &trackFilterDbEntry->macAddr, trackFilterDbEntry->process == RTL8651_SRCBLOCK_MAC?TRUE : FALSE, trackFilterDbEntry->process == RTL8651_TOCPU_MAC ? TRUE: FALSE, trackFilterDbEntry->memberPortMask, arpAgingTime, TRUE, 0);
						//Write Arp pending entry to ASIC, by chhuang
						_rtl8651_updateAsicArpAndRoute(trackFilterDbEntry);						
					}
				}
				//Remove entry from list
				{
					rtl8651_tblDrv_filterDbTableEntry_t * trackTempFilterDbEntry;
					if(tempFilterDbEntry == rtl8651FilterDbTable[rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].fid].database[rtl8651_filterDbIndex(macAddr)]) {
						rtl8651FilterDbTable[rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].fid].database[rtl8651_filterDbIndex(macAddr)] = tempFilterDbEntry->next;
					}
					else {//To be deleted entry is not at the head
						trackTempFilterDbEntry = rtl8651FilterDbTable[rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].fid].database[rtl8651_filterDbIndex(macAddr)];
						while(trackTempFilterDbEntry && trackTempFilterDbEntry->next != tempFilterDbEntry)
							trackTempFilterDbEntry = trackTempFilterDbEntry->next;
						assert(trackTempFilterDbEntry);
						trackTempFilterDbEntry->next = tempFilterDbEntry->next;
					}
					tempFilterDbEntry->next = rtl8651FreeFilterDbEntryList;
					rtl8651FreeFilterDbEntryList = tempFilterDbEntry;
				}
				return SUCCESS;
			}
				
		}
		tempFilterDbEntry = tempFilterDbEntry->next;
	}
	return FAILED;
}

static rtl8651_tblDrv_filterDbTableEntry_t * _rtl8651_getVlanFilterDatabaseEntry(uint16 vid, ether_addr_t * macAddr) {
	rtl8651_tblDrv_filterDbTableEntry_t * tempFilterDbEntry;
	
	if(vid < 1 || vid > 4094)
		return (rtl8651_tblDrv_filterDbTableEntry_t *)NULL;
	//If no such VLAN exist, 
	if(rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].valid == 0 ||
		rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].vid != vid)
		return (rtl8651_tblDrv_filterDbTableEntry_t *)NULL;
	assert(rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].fid < RTL8651_FDB_NUMBER);
	assert(rtl8651FilterDbTable[rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].fid].valid == 1);
	//Check whether entry exist
	tempFilterDbEntry = rtl8651FilterDbTable[rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].fid].database[rtl8651_filterDbIndex(macAddr)];
	while(tempFilterDbEntry) {
		if(memcmp(&tempFilterDbEntry->macAddr, macAddr, sizeof(ether_addr_t)) == 0)
			return tempFilterDbEntry;
		tempFilterDbEntry = tempFilterDbEntry->next;
	}
	return (rtl8651_tblDrv_filterDbTableEntry_t *)NULL;
}

static uint32 _rtl8651_returnPortMaskPortNumber(uint32 portMask) {
	uint32 i, portNumber;

	for(i=0, portNumber = 0; i<RTL8651_PORT_NUMBER; i++)
		if(portMask & (1<<i))
			portNumber++;
		
	return portNumber;
}

int32 rtl8651_getAggregatorActiveStatus(uint16 aggregator, int8 * isActive) {

	if(aggregator >= RTL8651_PORT_NUMBER) 
		return (TBLDRV_EINVAGGREATOR);
	if (isActive == NULL)
		return(TBLDRV_EINVVAL);
	RTL8651_LOCK_RESOURCE();//Lock resource
	*isActive = rtl8651LinkAggregationTable[aggregator].ethernetPortUpStatus != 0? TRUE: FALSE;
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

int32 rtl8651_setAggregatorIndividual(uint16 aggregator, int8 individual) {
	
	if(aggregator >= RTL8651_PORT_NUMBER) { //Invalid aggregator ID
		return(TBLDRV_EINVAGGREATOR);
	}
	RTL8651_LOCK_RESOURCE();//Lock resource

	if((rtl8651LinkAggregationTable[aggregator].individual == 1 && individual == TRUE) ||
		(rtl8651LinkAggregationTable[aggregator].individual == 0 && individual == FALSE)) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return SUCCESS;//If original state and new configured value are equal, no need to modify
	}
	//If condition not allow individual state, forbid this configuration
	if(rtl8651LinkAggregationTable[aggregator].aggregated == 1 && individual == TRUE) {//Not allow aggregated aggregator to be individual
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EAGGREGATING);
	}
	if(rtl8651LinkAggregationTable[aggregator].ethernetPortMask == 0) {//Not allow aggregator without port member to be configured with properties
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EAGGREHASNOPORT);

	}
	rtl8651LinkAggregationTable[aggregator].individual = (individual == TRUE) ? 1: 0;
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

int32 rtl8651_setPortAggregator(uint32 port, uint16 aggregator) {
	uint32 i, j, orgAggregator, fid, aggregaedNumber;
	int8 portLinkUp;
	
	if(port >= RTL8651_PORT_NUMBER) {
		return(TBLDRV_EINVPORT);

	}
	if (aggregator >= RTL8651_PORT_NUMBER) {
		return(TBLDRV_EINVAGGREATOR);

	}
	if (aggregator > port) { //Not allow aggregator larger than port...Port should select aggregator less or equal to itself
		return(TBLDRV_EAGGREGTPORT);
	}
	RTL8651_LOCK_RESOURCE();//Lock resource
	for(orgAggregator = 0; orgAggregator<RTL8651_PORT_NUMBER; orgAggregator++)
		if(rtl8651LinkAggregationTable[orgAggregator].ethernetPortMask & (1<<port))
			break;
	assert(orgAggregator < RTL8651_PORT_NUMBER);

	if(orgAggregator == aggregator) {//Does not change aggregator
		RTL8651_UNLOCK_RESOURCE();//Unlock resource		
		//return FAILED;//No need to modify driver internal data structure
		return SUCCESS;
	}
	if(rtl8651LinkAggregationTable[orgAggregator].individual == 1 || //Original aggregator not allow to aggregate with other
		rtl8651LinkAggregationTable[aggregator].individual == 1 ) {//Target aggregator not allow to be aggregated
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EAGGREINDIVIDUAL);
	}
	//Check whether the number of aggregator not exceed the system capacity
	for(i=0, aggregaedNumber = 0; i<RTL8651_PORT_NUMBER; i++) {
		assert(rtl8651LinkAggregationTable[i].aggregated == 1 ? _rtl8651_returnPortMaskPortNumber(rtl8651LinkAggregationTable[i].ethernetPortMask)>1 : _rtl8651_returnPortMaskPortNumber(rtl8651LinkAggregationTable[i].ethernetPortMask)<= 1);
		if(rtl8651LinkAggregationTable[i].aggregated == 1) 
			aggregaedNumber++;
	}
	if(aggregaedNumber >= RTL8651_MAX_AGGREGATION_NUM && //Unable to add new aggregated aggregator
		rtl8651LinkAggregationTable[aggregator].aggregated == 0 && //Target aggregator is not already aggregated, unable to operate without increasing aggreaged aggreagtor
		rtl8651LinkAggregationTable[orgAggregator].aggregated == 0) { //Original link is not aggreaged
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EAGGREEXCEED);
	}
	if (_rtl8651_returnPortMaskPortNumber(rtl8651LinkAggregationTable[orgAggregator].ethernetPortMask)>2) {//Orignal link is aggreaged but not change to not aggregated after port removal
		return(TBLDRV_EAGGREEXCEED);
	}
	for(i=0; i<RTL8651_VLAN_NUMBER; i++)
		if(rtl8651VlanTable[i].valid == 1 && ((rtl8651VlanTable[i].memberPortMask & (1<<port)) == 0) != ((rtl8651VlanTable[i].memberPortMask & rtl8651LinkAggregationTable[aggregator].ethernetPortMask) == 0)) {
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return(TBLDRV_EDIFFERENTBCASTDOMAIN);			
		}
	//Start to modify related tables
	rtl8651LinkAggregationTable[orgAggregator].ethernetPortMask &= (~(1<<port));
	if(rtl8651LinkAggregationTable[orgAggregator].ethernetPortUpStatus & (1<<port))
		portLinkUp = TRUE;
	else
		portLinkUp = FALSE;
	rtl8651LinkAggregationTable[orgAggregator].ethernetPortUpStatus &= ~(1<<port);
	if(_rtl8651_returnPortMaskPortNumber(rtl8651LinkAggregationTable[orgAggregator].ethernetPortMask)<= 1 && rtl8651LinkAggregationTable[orgAggregator].aggregated == 1) {
		rtl8651LinkAggregationTable[orgAggregator].aggregated = 0;
		rtl8651_setAsicLinkAggregator(0);//Clear ASIC trunk assignment
	}
	rtl8651LinkAggregationTable[aggregator].ethernetPortMask |= (1<<port);
	if(portLinkUp==TRUE)
		rtl8651LinkAggregationTable[aggregator].ethernetPortUpStatus |= (1<<port);
	if(_rtl8651_returnPortMaskPortNumber(rtl8651LinkAggregationTable[aggregator].ethernetPortMask) > 1) {
		rtl8651LinkAggregationTable[aggregator].aggregated = 1;
		rtl8651_setAsicLinkAggregator(rtl8651LinkAggregationTable[aggregator].ethernetPortMask); //Assign ASIC trunk assignment
	}
	//Processes when port removal causes aggregator to become invalid. 
	// Change VLAN, filter database . The information about original aggreator is hooked to target aggregator
	if((rtl8651LinkAggregationTable[orgAggregator].ethernetPortMask & (~(1<<port))) == 0) {
		assert(rtl8651LinkAggregationTable[orgAggregator].aggregated == 0);
		for(i=0; i<RTL8651_STI_NUMBER; i++) //Store aggregator member port state equal to aggregator state
			if(rtl8651SpanningTreeTable[i].valid == 1)
				for(j=0; j< RTL8651_PORT_NUMBER; j++)
					if((rtl8651LinkAggregationTable[aggregator].ethernetPortMask & (1<<j) ) && j != aggregator)
						rtl8651SpanningTreeTable[i].portState[j] = rtl8651SpanningTreeTable[i].portState[aggregator];
		//Traverse all filter database to move orgAggregator to new aggregator
		for(fid=0; fid<RTL8651_FDB_NUMBER; fid++)
			if(rtl8651FilterDbTable[fid].valid == 1) {
				rtl8651_tblDrv_filterDbTableEntry_t * tempFilterDbEntry;
				for(i=0; i<RTL8651_L2TBL_ROW; i++) {
					tempFilterDbEntry = rtl8651FilterDbTable[fid].database[i];
					while(tempFilterDbEntry) {
						if(tempFilterDbEntry->memberPortMask & (1<<port)) {
							tempFilterDbEntry->memberPortMask &= (~(1<<port));
							tempFilterDbEntry->memberPortMask |= (1<<aggregator);
							//Write modified entry into ASIC
							rtl8651_setAsicL2Table_Patch(rtl8651_filterDbIndex(&tempFilterDbEntry->macAddr), tempFilterDbEntry->asicPos, &tempFilterDbEntry->macAddr, tempFilterDbEntry->process == RTL8651_SRCBLOCK_MAC?TRUE : FALSE, tempFilterDbEntry->process == RTL8651_TOCPU_MAC ? TRUE: FALSE, tempFilterDbEntry->memberPortMask, arpAgingTime, TRUE, 0);
						}
						tempFilterDbEntry = tempFilterDbEntry->next;
					}
				}
			}
	}
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

//Configure link aggregation VID
int32 rtl8651_setPvid(uint32 port, uint16 vid) {
	int i;
	
	if(port >= RTL8651_PORT_NUMBER) {
		return(TBLDRV_EINVPORT);
	}
	if (vid == 0 || vid > 4094) {
		return(TBLDRV_EINVVLAN);
	}
	RTL8651_LOCK_RESOURCE();//Lock resource
	
	if(rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].valid == 0 ||	rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].vid != vid) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EINVVLAN);
	}
	//Check whether aggregator valid. The way to check the validation of aggregator is whether at least one port belong to aggregator
	if(rtl8651LinkAggregationTable[port].ethernetPortMask == 0) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EAGGREHASNOPORT);
	}
	rtl8651LinkAggregationTable[port].pvid = vid;
	
	for(i = 0; i< RTL8651_PORT_NUMBER; i++)
		if(rtl8651LinkAggregationTable[port].ethernetPortMask & (1<<i))
			rtl8651_setAsicPvid(i, rtl8651_vlanTableIndex(vid));
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}


static void _rtl8651_updateAsicAclRuleToCpu(uint32 aclIdx) {
	rtl8651_tblDrvMacAclRule_t rule;

	memset(&rule.dstMac, 0, sizeof(ether_addr_t));
	memset(&rule.dstMacMask, 0, sizeof(ether_addr_t));
	memset(&rule.srcMac, 0, sizeof(ether_addr_t));
	memset(&rule.srcMacMask, 0, sizeof(ether_addr_t));
	rule.typeLen = 0;
	rule.typeLenMask = 0;
	rule.actionType = RTL8651_ACL_CPU;
	rtl8651_setAsicMacAcl(aclIdx, &rule);
}

static void _rtl8651_updateAsicAclRulePermitAll(uint32 aclIdx) {
	rtl8651_tblDrvMacAclRule_t rule;

	memset(&rule.dstMac, 0, sizeof(ether_addr_t));
	memset(&rule.dstMacMask, 0, sizeof(ether_addr_t));
	memset(&rule.srcMac, 0, sizeof(ether_addr_t));
	memset(&rule.srcMacMask, 0, sizeof(ether_addr_t));
	rule.typeLen = 0;
	rule.typeLenMask = 0;
	rule.actionType = RTL8651_ACL_PERMIT;
	rtl8651_setAsicMacAcl(aclIdx, &rule);
}

static int8 _rtl8651_orderedAlgSlotAllocation(rtl8651_tblDrv_algEntry_t *algPtr) {
	rtl8651_tblDrv_algSlot_t *trackAlgSlotPtr;
	uint16 port = 0;
	
	trackAlgSlotPtr = algPtr->nextAlgSlot;
	if(trackAlgSlotPtr && rtl8651AlgAsicArrangementTable[trackAlgSlotPtr->algSlot].port == 0)
		trackAlgSlotPtr = trackAlgSlotPtr->next;

	while(trackAlgSlotPtr) {
		if(rtl8651AlgAsicArrangementTable[trackAlgSlotPtr->algSlot].port <= port)
			return FALSE;
		port = rtl8651AlgAsicArrangementTable[trackAlgSlotPtr->algSlot].port;
		trackAlgSlotPtr = trackAlgSlotPtr->next;
	}

	return TRUE;
}

#define RTL8651_ALG_TCPSERVER		0x01
#define RTL8651_ALG_TCPCLIENT		0x02
#define RTL8651_ALG_UDPSERVER		0x04
#define RTL8651_ALG_UDPCLIENT		0x08
#define RTL8651_SERVERPORT_TCP		0x01
#define RTL8651_SERVERPORT_UDP		0x02

static void _rtl8651_updateAsicVlan(uint16 vid) {
	uint32 i, j, aclIdx, netIdx, aclInputStart, aclInputEnd, aclOutputStart, aclOutputEnd;
	uint32 serviceFlag, vidx = rtl8651_vlanTableIndex(vid), netID;
	uint16 startPort, endPort;//Cached as the serviceFlag is assigned
	rtl8651_tblDrv_algEntry_t *trackAlgPtr;
	rtl8651_tblDrv_naptServerPortEntry_t * trackNaptServerPortPtr, *ruleNaptServerPortPtr;
	rtl8651_tblDrvIpAclRule_t ipRule;
	rtl8651_tblDrv_aclIpEntry_t * ipAclPtr;
	rtl8651_tblDrv_aclMacEntry_t * macAclPtr;
	int8 inputRulePending, outputRulePending, aclMac, aclIP;
	assert(vid > 0 && vid < 4095);
	assert(rtl8651VlanTable[vidx].valid == 1);
	assert(rtl8651VlanTable[vidx].vid == vid);

	//Arrange Input ACL table (Ranged ALG, MAC ACL and IP ACL)
	for(aclIdx = 0; aclIdx<rtl8651VlanTable[vidx].aclAsicInputMaxNumber; aclIdx++)
		rtl8651AclAsicArrangementTable[rtl8651VlanTable[vidx].aclAsicInputPos + aclIdx].used = 0;
	aclIdx = 0;
	if(rtl8651AlgList ||  rtl8651VlanTable[vidx].inAclHead || rtl8651VlanTable[vidx].inIpAclHead)
		inputRulePending = TRUE;
	else
		inputRulePending = FALSE;
	if(rtl8651VlanTable[vidx].outAclHead || rtl8651VlanTable[vidx].outIpAclHead)
		outputRulePending = TRUE;
	else
		outputRulePending = FALSE;
	for(i=0; (inputRulePending == FALSE || outputRulePending == FALSE) && i< rtl8651_tblDrvPara.networkIntfTableSize; i++) 
		if(rtl8651NetworkIntfTable[i].valid == 1) {
			if(rtl8651NetworkIntfTable[i].linkLayerType == RTL8651_LL_VLAN && rtl8651NetworkIntfTable[i].linkLayerIndex == rtl8651VlanTable[vidx].vid) {
				if(rtl8651NetworkIntfTable[i].inAclHead)
					inputRulePending = TRUE;
				if(rtl8651NetworkIntfTable[i].outAclHead)
					outputRulePending = TRUE;
			}
			else if (rtl8651NetworkIntfTable[i].linkLayerType == RTL8651_LL_PPPOE) {
				for(j=0; j<RTL8651_PPPOE_NUMBER; j++)
					if(rtl8651PppoeTable[j].valid == 1 && rtl8651PppoeTable[j].objectId == rtl8651NetworkIntfTable[i].linkLayerIndex && rtl8651PppoeTable[j].vid == rtl8651VlanTable[vidx].vid) {
						if(rtl8651NetworkIntfTable[i].inAclHead)
							inputRulePending = TRUE;
						if(rtl8651NetworkIntfTable[i].outAclHead)
							outputRulePending = TRUE;
					}
			}
		}
	//Arrange Ranged ALG
	trackAlgPtr = rtl8651AlgList;
	serviceFlag = 0;
	while(aclIdx < rtl8651VlanTable[vidx].aclAsicInputMaxNumber && (trackAlgPtr || serviceFlag)) {
		if(serviceFlag == 0) {//Get flag from trackAlgPtr and move trackAlgPtr forward
			assert(_rtl8651_countAlgSlotAllocateNumber(trackAlgPtr) <= (trackAlgPtr->endPort - trackAlgPtr->startPort + 1));
			assert(_rtl8651_orderedAlgSlotAllocation(trackAlgPtr) == TRUE);
			if(_rtl8651_countAlgSlotAllocateNumber(trackAlgPtr) < (trackAlgPtr->endPort - trackAlgPtr->startPort + 1)) {
				assert(trackAlgPtr->nextAlgSlot == NULL);
				if(trackAlgPtr->tcpServer == 1)
					serviceFlag |= RTL8651_ALG_TCPSERVER;
				if(trackAlgPtr->tcpClient == 1)
					serviceFlag |= RTL8651_ALG_TCPCLIENT;
				if(trackAlgPtr->udpServer == 1)
					serviceFlag |= RTL8651_ALG_UDPSERVER;
				if(trackAlgPtr->udpClient == 1)
					serviceFlag |= RTL8651_ALG_UDPCLIENT;
				startPort = trackAlgPtr->startPort;
				endPort = trackAlgPtr->endPort;
			}
			trackAlgPtr = trackAlgPtr->next;
		}
		else { 
			ipRule.actionType = RTL8651_ACL_CPU;
			ipRule.dstIpAddr = 0;
			ipRule.dstIpAddrMask = 0;
			ipRule.srcIpAddr = 0;
			ipRule.srcIpAddrMask = 0;
			ipRule.tos = 0;
			ipRule.tosMask = 0;
			if(serviceFlag & (RTL8651_ALG_TCPSERVER | RTL8651_ALG_TCPCLIENT)) {
				ipRule.ruleType = RTL8651_ACL_TCP;
				ipRule.is.tcp.tcpflag = 0;
				ipRule.is.tcp.flagMask = 0;
				if(	((serviceFlag & RTL8651_ALG_TCPSERVER) && rtl8651VlanTable[vidx].internal == 1) || 
					((serviceFlag & RTL8651_ALG_TCPCLIENT) && rtl8651VlanTable[vidx].internal == 0)){
					ipRule.is.tcp.srcPortUpperBound = endPort;
					ipRule.is.tcp.srcPortLowerBound = startPort;
					ipRule.is.tcp.dstPortUpperBound = 0xffff;
					ipRule.is.tcp.dstPortLowerBound = 0;
				}
				else {
					ipRule.is.tcp.srcPortUpperBound = 0xffff;
					ipRule.is.tcp.srcPortLowerBound = 0;
					ipRule.is.tcp.dstPortUpperBound = endPort;
					ipRule.is.tcp.dstPortLowerBound = startPort;
				}
				if(serviceFlag & RTL8651_ALG_TCPSERVER)
					serviceFlag &= ~RTL8651_ALG_TCPSERVER;
				else
					serviceFlag &= ~RTL8651_ALG_TCPCLIENT;
			}
			else {
				assert(serviceFlag & (RTL8651_ALG_UDPSERVER | RTL8651_ALG_UDPCLIENT));
				ipRule.ruleType = RTL8651_ACL_UDP;
				if(	((serviceFlag & RTL8651_ALG_UDPSERVER) && rtl8651VlanTable[vidx].internal == 1) || 
					((serviceFlag & RTL8651_ALG_UDPCLIENT) && rtl8651VlanTable[vidx].internal == 0)){
					ipRule.is.udp.srcPortUpperBound = endPort;
					ipRule.is.udp.srcPortLowerBound = startPort;
					ipRule.is.udp.dstPortUpperBound = 0xffff;
					ipRule.is.udp.dstPortLowerBound = 0;
				}
				else {
					ipRule.is.udp.srcPortUpperBound = 0xffff;
					ipRule.is.udp.srcPortLowerBound = 0;
					ipRule.is.udp.dstPortUpperBound = endPort;
					ipRule.is.udp.dstPortLowerBound = startPort;
				}
				if(serviceFlag & RTL8651_ALG_UDPSERVER)
					serviceFlag &= ~RTL8651_ALG_UDPSERVER;
				else
					serviceFlag &= ~RTL8651_ALG_UDPCLIENT;
			}
			rtl8651AclAsicArrangementTable[rtl8651VlanTable[vidx].aclAsicInputPos + aclIdx].used = 1;
			rtl8651_setAsicIpAcl(rtl8651VlanTable[vidx].aclAsicInputPos + aclIdx, &ipRule);
			aclIdx++;
		}
	}
	if(aclIdx == rtl8651VlanTable[vidx].aclAsicInputMaxNumber && (trackAlgPtr || serviceFlag)) //Not all ALG Rule filled
		_rtl8651_updateAsicAclRuleToCpu(rtl8651VlanTable[vidx].aclAsicInputPos + aclIdx-1);
	else {//Continue fill server port rule
		for(netIdx=0, serviceFlag = 0, trackNaptServerPortPtr = NULL; serviceFlag == 0 && trackNaptServerPortPtr == NULL && netIdx<rtl8651_tblDrvPara.networkIntfTableSize; netIdx++) {
			if(rtl8651NetworkIntfTable[netIdx].valid == 1 && rtl8651NetworkIntfTable[netIdx].nextNaptServerPort) {
				trackNaptServerPortPtr = rtl8651NetworkIntfTable[netIdx].nextNaptServerPort;
				serviceFlag = 0;
				while(aclIdx < rtl8651VlanTable[vidx].aclAsicInputMaxNumber && (trackNaptServerPortPtr || serviceFlag)) {
					if(serviceFlag == 0) {
						while(trackNaptServerPortPtr && trackNaptServerPortPtr->toAsic == 1)
							trackNaptServerPortPtr = trackNaptServerPortPtr->next;
						if(trackNaptServerPortPtr) {
							assert(trackNaptServerPortPtr->toAsic == 0);
							if(trackNaptServerPortPtr->tcp == 1)
								serviceFlag |= RTL8651_SERVERPORT_TCP;
							if(trackNaptServerPortPtr->udp == 1)
								serviceFlag |= RTL8651_SERVERPORT_UDP;
							ruleNaptServerPortPtr = trackNaptServerPortPtr;
							trackNaptServerPortPtr = trackNaptServerPortPtr->next;
						}
					}
					else {
						ipRule.actionType = RTL8651_ACL_CPU;
						ipRule.tos = 0;
						ipRule.tosMask = 0;
						if(serviceFlag & RTL8651_SERVERPORT_TCP) {
							ipRule.ruleType = RTL8651_ACL_TCP;
							ipRule.is.tcp.tcpflag = 0;
							ipRule.is.tcp.flagMask = 0;
							if(rtl8651VlanTable[vidx].internal == 1) {
								ipRule.srcIpAddr = ruleNaptServerPortPtr->localIp;
								ipRule.srcIpAddrMask = 0xffffffff;
								ipRule.dstIpAddr = 0;
								ipRule.dstIpAddrMask = 0;
								ipRule.is.tcp.srcPortUpperBound = ruleNaptServerPortPtr->localPort;
								ipRule.is.tcp.srcPortLowerBound = ruleNaptServerPortPtr->localPort;
								ipRule.is.tcp.dstPortUpperBound = 0xffff;
								ipRule.is.tcp.dstPortLowerBound = 0;
							}
							else {
								ipRule.srcIpAddr = 0;
								ipRule.srcIpAddrMask = 0;
								ipRule.dstIpAddr = ruleNaptServerPortPtr->globalIp;
								ipRule.dstIpAddrMask = 0xffffffff;
								ipRule.is.tcp.srcPortUpperBound = 0xffff;
								ipRule.is.tcp.srcPortLowerBound = 0;
								ipRule.is.tcp.dstPortUpperBound = ruleNaptServerPortPtr->globalPort;
								ipRule.is.tcp.dstPortLowerBound = ruleNaptServerPortPtr->globalPort;
							}
							serviceFlag &= ~RTL8651_SERVERPORT_TCP;
						}
						else {
							assert(serviceFlag & RTL8651_SERVERPORT_UDP);
							ipRule.ruleType = RTL8651_ACL_UDP;
							if(rtl8651VlanTable[vidx].internal == 1) {
								ipRule.srcIpAddr = ruleNaptServerPortPtr->localIp;
								ipRule.srcIpAddrMask = 0xffffffff;
								ipRule.dstIpAddr = 0;
								ipRule.dstIpAddrMask = 0;
								ipRule.is.udp.srcPortUpperBound = ruleNaptServerPortPtr->localPort;
								ipRule.is.udp.srcPortLowerBound = ruleNaptServerPortPtr->localPort;
								ipRule.is.udp.dstPortUpperBound = 0xffff;
								ipRule.is.udp.dstPortLowerBound = 0;
							}
							else {
								ipRule.srcIpAddr = 0;
								ipRule.srcIpAddrMask = 0;
								ipRule.dstIpAddr = ruleNaptServerPortPtr->globalIp;
								ipRule.dstIpAddrMask = 0xffffffff;
								ipRule.is.udp.srcPortUpperBound = 0xffff;
								ipRule.is.udp.srcPortLowerBound = 0;
								ipRule.is.udp.dstPortUpperBound = ruleNaptServerPortPtr->globalPort;
								ipRule.is.udp.dstPortLowerBound = ruleNaptServerPortPtr->globalPort;
							}
							serviceFlag &= ~RTL8651_SERVERPORT_UDP;
						}
						rtl8651AclAsicArrangementTable[rtl8651VlanTable[vidx].aclAsicInputPos + aclIdx].used = 1;
						rtl8651_setAsicIpAcl(rtl8651VlanTable[vidx].aclAsicInputPos + aclIdx, &ipRule);
						aclIdx++;
					}
				}
			}
		}
		//Fill ACL Table with:
		// (1)MAC acl  (2)VLAN IP acl  (3)IP acl
		aclMac = aclIP = FALSE;
		if(aclIdx == rtl8651VlanTable[vidx].aclAsicInputMaxNumber && (trackNaptServerPortPtr || serviceFlag)) //Not all Server port are put into input ACL
			_rtl8651_updateAsicAclRuleToCpu(rtl8651VlanTable[vidx].aclAsicInputPos + aclIdx-1);
		else {//Continue fill MAC Rule when ALG Rule fill complete with free server port rule space
			macAclPtr = rtl8651VlanTable[vidx].inAclHead;
			if (macAclPtr)
				aclMac = TRUE; //at least one MAC acl rule is added
			while(aclIdx < rtl8651VlanTable[vidx].aclAsicInputMaxNumber && macAclPtr) {
				rtl8651AclAsicArrangementTable[rtl8651VlanTable[vidx].aclAsicInputPos + aclIdx].used = 1;
				rtl8651_setAsicMacAcl(rtl8651VlanTable[vidx].aclAsicInputPos + aclIdx, &macAclPtr->rule);
				aclIdx++;
				macAclPtr = macAclPtr->next;
			}
			if(aclIdx == rtl8651VlanTable[vidx].aclAsicInputMaxNumber && macAclPtr)
				_rtl8651_updateAsicAclRuleToCpu(rtl8651VlanTable[vidx].aclAsicInputPos + aclIdx-1);
			else {//Continue fill IP rule attached over VLAN when ALG Rule fill complete with free ACL rule space
				ipAclPtr = rtl8651VlanTable[vidx].inIpAclHead;
				if (ipAclPtr)
					aclMac = TRUE; //at least one VLAN IP acl rule is added
				while(aclIdx < rtl8651VlanTable[vidx].aclAsicInputMaxNumber && ipAclPtr) {
					rtl8651AclAsicArrangementTable[rtl8651VlanTable[vidx].aclAsicInputPos + aclIdx].used = 1;
					rtl8651_setAsicIpAcl(rtl8651VlanTable[vidx].aclAsicInputPos + aclIdx, &ipAclPtr->rule);
					aclIdx++;
					ipAclPtr = ipAclPtr->next;
				}
				if(aclIdx == rtl8651VlanTable[vidx].aclAsicInputMaxNumber && ipAclPtr)
					_rtl8651_updateAsicAclRuleToCpu(rtl8651VlanTable[vidx].aclAsicInputPos + aclIdx-1);
				else {//Continue fill IP rule when ALG Rule fill complete with free ACL rule space
					ipAclPtr = NULL;
					for(i=0; ipAclPtr == NULL && i< rtl8651_tblDrvPara.networkIntfTableSize; i++)
						if(rtl8651NetworkIntfTable[i].valid == 1) {
							if(rtl8651NetworkIntfTable[i].linkLayerType == RTL8651_LL_VLAN && rtl8651NetworkIntfTable[i].linkLayerIndex == rtl8651VlanTable[vidx].vid)
								ipAclPtr = rtl8651NetworkIntfTable[i].inAclHead;
							else if (rtl8651NetworkIntfTable[i].linkLayerType == RTL8651_LL_PPPOE) {
								for(j=0; ipAclPtr == NULL && j<RTL8651_PPPOE_NUMBER; j++)
									if(rtl8651PppoeTable[j].valid == 1 && rtl8651PppoeTable[j].objectId == rtl8651NetworkIntfTable[i].linkLayerIndex && rtl8651PppoeTable[j].vid == rtl8651VlanTable[vidx].vid)
										ipAclPtr = rtl8651NetworkIntfTable[i].inAclHead;
							}
						}
					netID = i-1;
					if (ipAclPtr)
						aclIP = TRUE; //at least one IP acl rule is added
					while(aclIdx < rtl8651VlanTable[vidx].aclAsicInputMaxNumber && ipAclPtr) {
						rtl8651AclAsicArrangementTable[rtl8651VlanTable[vidx].aclAsicInputPos + aclIdx].used = 1;
						rtl8651_setAsicIpAcl(rtl8651VlanTable[vidx].aclAsicInputPos + aclIdx, &ipAclPtr->rule);
						aclIdx++;
						ipAclPtr = ipAclPtr->next;
					}
					if(aclIdx == rtl8651VlanTable[vidx].aclAsicInputMaxNumber && ipAclPtr)
						_rtl8651_updateAsicAclRuleToCpu(rtl8651VlanTable[vidx].aclAsicInputPos + aclIdx-1);
				}
			}
		}
	}
	if (aclIdx!=rtl8651VlanTable[vidx].aclAsicInputMaxNumber && aclIdx > 0) {
		if (aclIP == TRUE) {
			//IP acl rule is added, check whether add PERMIT all to last rule or not
			if (rtl8651NetworkIntfTable[netID].aclDefPermit == 1) {
				_rtl8651_updateAsicAclRulePermitAll(rtl8651VlanTable[vidx].aclAsicInputPos+aclIdx);
				aclIdx++;
			}
		} else if (aclMac == TRUE) {
			//Only Mac acl or VLAN IP acl added, check whether add PERMIT all to last rule or not
			if (rtl8651VlanTable[vidx].aclDefPermit == 1) {
				_rtl8651_updateAsicAclRulePermitAll(rtl8651VlanTable[vidx].aclAsicInputPos+aclIdx);
			aclIdx++;
		}
		} else if (aclIP==FALSE && aclMac==FALSE) {
			//Only ALG and Server Port are added into Ingress ACL Table
			//Hence, add PERMIT ALL in the last rule
			_rtl8651_updateAsicAclRulePermitAll(rtl8651VlanTable[vidx].aclAsicInputPos+aclIdx);
			aclIdx++;
		}
	}
	rtl8651VlanTable[vidx].aclAsicInputUsed = aclIdx;
	
	//Egress: Arrange Output ACL table (MAC ACL and IP ACL)
	//Check Order:
	//		(1)Network Interface IP rule  (2)VLAN IP rule  (3)MAC rule
	for(aclIdx = 0; aclIdx<rtl8651VlanTable[vidx].aclAsicOutputMaxNumber; aclIdx++)
		rtl8651AclAsicArrangementTable[rtl8651VlanTable[vidx].aclAsicOutputPos + aclIdx].used = 0;
	aclIdx = 0; aclIP = aclMac = FALSE;
	
	//(1) Fill ACL Table with Network IP Interface rules 
	for(i=0, ipAclPtr=NULL; ipAclPtr == NULL && i<rtl8651_tblDrvPara.networkIntfTableSize; i++)
				if(rtl8651NetworkIntfTable[i].valid == 1) {
					if(rtl8651NetworkIntfTable[i].linkLayerType == RTL8651_LL_VLAN && rtl8651NetworkIntfTable[i].linkLayerIndex == rtl8651VlanTable[vidx].vid)
						ipAclPtr = rtl8651NetworkIntfTable[i].outAclHead;
					else if (rtl8651NetworkIntfTable[i].linkLayerType == RTL8651_LL_PPPOE) {
						for(j=0; ipAclPtr == NULL && j<RTL8651_PPPOE_NUMBER; j++)
							if(rtl8651PppoeTable[j].valid == 1 && rtl8651PppoeTable[j].objectId == rtl8651NetworkIntfTable[i].linkLayerIndex && rtl8651PppoeTable[j].vid == rtl8651VlanTable[vidx].vid)
								ipAclPtr = rtl8651NetworkIntfTable[i].outAclHead;
					}
				}
	netID = i-1;
	if (ipAclPtr)
		aclIP = TRUE;
	while(aclIdx < rtl8651VlanTable[vidx].aclAsicOutputMaxNumber && ipAclPtr) {
		rtl8651AclAsicArrangementTable[rtl8651VlanTable[vidx].aclAsicOutputPos + aclIdx].used = 1;
		rtl8651_setAsicIpAcl(rtl8651VlanTable[vidx].aclAsicOutputPos + aclIdx, &ipAclPtr->rule);
		aclIdx++;
		ipAclPtr = ipAclPtr->next;
	}
	if(aclIdx == rtl8651VlanTable[vidx].aclAsicOutputMaxNumber && ipAclPtr)
		_rtl8651_updateAsicAclRuleToCpu(rtl8651VlanTable[vidx].aclAsicOutputPos + aclIdx-1);
	else {
		//(2)Fill ACL Table with VLAN IP rules
		ipAclPtr = rtl8651VlanTable[vidx].outIpAclHead; 
		if (ipAclPtr)
			aclMac = TRUE;
			while(aclIdx < rtl8651VlanTable[vidx].aclAsicOutputMaxNumber && ipAclPtr) {
				rtl8651AclAsicArrangementTable[rtl8651VlanTable[vidx].aclAsicOutputPos + aclIdx].used = 1;
				rtl8651_setAsicIpAcl(rtl8651VlanTable[vidx].aclAsicOutputPos + aclIdx, &ipAclPtr->rule);
				aclIdx++;
				ipAclPtr = ipAclPtr->next;
			}
			if(aclIdx == rtl8651VlanTable[vidx].aclAsicOutputMaxNumber && ipAclPtr)
				_rtl8651_updateAsicAclRuleToCpu(rtl8651VlanTable[vidx].aclAsicOutputPos + aclIdx-1);
		else {
			//(3) Fill ACL Table with MAC rules
			macAclPtr = rtl8651VlanTable[vidx].outAclHead;
			if (macAclPtr)
				aclMac = TRUE;
			while(aclIdx < rtl8651VlanTable[vidx].aclAsicOutputMaxNumber && macAclPtr) {
				rtl8651AclAsicArrangementTable[rtl8651VlanTable[vidx].aclAsicOutputPos + aclIdx].used = 1;
				rtl8651_setAsicMacAcl(rtl8651VlanTable[vidx].aclAsicOutputPos + aclIdx, &macAclPtr->rule);
				aclIdx++;
				macAclPtr = macAclPtr->next;
			}
			if(aclIdx == rtl8651VlanTable[vidx].aclAsicOutputMaxNumber && macAclPtr)
			_rtl8651_updateAsicAclRuleToCpu(rtl8651VlanTable[vidx].aclAsicOutputPos + aclIdx-1);
		}
	}
	if (aclIdx!=rtl8651VlanTable[vidx].aclAsicOutputMaxNumber && aclIdx > 0) {
		if (aclIP == TRUE) {
			//IP acl rule is added, check whether add PERMIT all to last rule or not
			if (rtl8651NetworkIntfTable[netID].aclDefPermit == 1) {
				_rtl8651_updateAsicAclRulePermitAll(rtl8651VlanTable[vidx].aclAsicOutputPos+aclIdx);
				aclIdx++;
			}
		} else if (aclMac == TRUE) {
			//Only Mac acl or VLAN IP acl added, check whether add PERMIT all to last rule or not
			if (rtl8651VlanTable[vidx].aclDefPermit == 1) {
				_rtl8651_updateAsicAclRulePermitAll(rtl8651VlanTable[vidx].aclAsicOutputPos+aclIdx);
				aclIdx++;
			}
		} else if (aclIP==FALSE && aclMac==FALSE) {
			//Only ALG and Server Port are added into Ingress ACL Table
			//Hence, add PERMIT ALL in the last rule
			_rtl8651_updateAsicAclRulePermitAll(rtl8651VlanTable[vidx].aclAsicOutputPos+aclIdx);
			aclIdx++;
		}
	}
	rtl8651VlanTable[vidx].aclAsicOutputUsed = aclIdx;
	
	//Decide staring and end rule position
	if(rtl8651VlanTable[vidx].aclAsicInputMaxNumber == 0 && inputRulePending == TRUE) {
		aclInputStart = RTL8651_ACLTBL_ALL_TO_CPU;
		aclInputEnd = RTL8651_ACLTBL_ALL_TO_CPU;
	}
	else if(rtl8651VlanTable[vidx].aclAsicInputUsed == 0) {
		aclInputStart = RTL8651_ACLTBL_PERMIT_ALL;
		aclInputEnd = RTL8651_ACLTBL_PERMIT_ALL;
	}
	else {
		aclInputStart = rtl8651VlanTable[vidx].aclAsicInputPos;
		aclInputEnd = rtl8651VlanTable[vidx].aclAsicInputPos + rtl8651VlanTable[vidx].aclAsicInputUsed - 1;
	}
	if(rtl8651VlanTable[vidx].aclAsicOutputMaxNumber == 0 && outputRulePending == TRUE) {
		aclOutputStart = RTL8651_ACLTBL_ALL_TO_CPU;
		aclOutputEnd = RTL8651_ACLTBL_ALL_TO_CPU;
	}
	else if (rtl8651VlanTable[vidx].aclAsicOutputUsed == 0) {
		aclOutputStart = RTL8651_ACLTBL_PERMIT_ALL;
		aclOutputEnd = RTL8651_ACLTBL_PERMIT_ALL;
	}
	else {
		aclOutputStart = rtl8651VlanTable[vidx].aclAsicOutputPos;
		aclOutputEnd = rtl8651VlanTable[vidx].aclAsicOutputPos + rtl8651VlanTable[vidx].aclAsicOutputUsed - 1;
	}

	rtl8651_setAsicVlan(rtl8651VlanTable[vidx].vid, &rtl8651VlanTable[vidx].macAddr, rtl8651VlanTable[vidx].memberPortMask, 
			aclInputStart, aclInputEnd, aclOutputStart, aclOutputEnd, 
			rtl8651VlanTable[vidx].internal == 1 ? TRUE: FALSE, rtl8651VlanTable[vidx].ipAttached==1? TRUE: FALSE, 
			rtl8651SpanningTreeTable[rtl8651FilterDbTable[rtl8651VlanTable[vidx].fid].sid].portState, FALSE, rtl8651VlanTable[vidx].promiscuous==1? TRUE: FALSE, 
			rtl8651VlanTable[vidx].untagPortMask, rtl8651VlanTable[vidx].macAddrNumber, 1500);
}

static void _rtl8651_updateAsicAllVlan(void) {
	uint32 i;

	for(i=0; i<RTL8651_VLAN_NUMBER; i++)
		if(rtl8651VlanTable[i].valid == 1)
			_rtl8651_updateAsicVlan(rtl8651VlanTable[i].vid);
}

int32 rtl8651_setVlanAclMismatchDrop(uint16 vid, int8 isDrop) {
	uint32 vidx = rtl8651_vlanTableIndex(vid);

	if (vid < 1 || vid > 4094) { //vid should be legal vlan ID
		return(TBLDRV_EINVVLAN);
	}
	RTL8651_LOCK_RESOURCE();//Lock resource
	
 	if (rtl8651VlanTable[vidx].valid == 0 || rtl8651VlanTable[vidx].vid != vid ) {
 		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EINVVLAN);
	}
 	rtl8651VlanTable[vidx].aclDefPermit = ((isDrop==TRUE)? 0: 1);
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

int32 rtl8651_addVlan(uint16 vid) {
	uint32 vidx = rtl8651_vlanTableIndex(vid), vlanCount;
	if(vid < 1 || vid > 4094) { //vid should be legal vlan ID
		return(TBLDRV_EINVVLAN);
	}
	RTL8651_LOCK_RESOURCE();//Lock resource
	
	if(rtl8651VlanTable[vidx].valid == 1) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EVLANEXIST);
	}

	rtl8651VlanTable[vidx].valid = 1; //Occupy entry
	rtl8651VlanTable[vidx].vid = vid; // Configure vid
	assert(rtl8651FilterDbTable[0].valid == 1); //Filter database 0 should exist
	rtl8651VlanTable[vidx].fid = 0; //Using default filtering database
	rtl8651VlanTable[vidx].memberPortMask = 0; //No member inside
	rtl8651VlanTable[vidx].untagPortMask = 0; //No port exist
	rtl8651VlanTable[vidx].memberPortUpStatus = 0; //No member port cause not member port up
	rtl8651VlanTable[vidx].macAddrNumber = 0;//No mac address arranged
	rtl8651VlanTable[vidx].internal = 1;//Default set network to be internal
	rtl8651VlanTable[vidx].DMZFlag = 0; //Default set to non-DMZ
	rtl8651VlanTable[vidx].ipAttached = 0;//Default no IP interface attached over it.
	rtl8651VlanTable[vidx].promiscuous = 0;//Default not promiscuous. When enabled, no IP, PPPoE or other can attached over it.
	rtl8651VlanTable[vidx].aclDefPermit = 0; //Default Drop packet when rule mismatch
	rtl8651VlanTable[vidx].inAclHead = NULL;
	rtl8651VlanTable[vidx].inAclTail = NULL;
	rtl8651VlanTable[vidx].outAclHead = NULL;
	rtl8651VlanTable[vidx].outAclTail = NULL;
	rtl8651VlanTable[vidx].inIpAclHead = NULL;
	rtl8651VlanTable[vidx].inIpAclTail = NULL;
	rtl8651VlanTable[vidx].outIpAclHead = NULL;
	rtl8651VlanTable[vidx].outIpAclTail = NULL;
	assert(rtl8651VlanTable[vidx].fid < RTL8651_FDB_NUMBER);// Driver should store correct fid in vlan table
	assert(rtl8651FilterDbTable[rtl8651VlanTable[vidx].fid].valid == 1); // Driver should store valid fid in vlan table
	assert(rtl8651SpanningTreeTable[rtl8651FilterDbTable[rtl8651VlanTable[vidx].fid].sid].valid == 1); // Driver should store valid sid in filtering database
	//Allocate VLAN ACL storage space
	{
		uint32 i, j, maxSlotStartPos, maxSlotSize;
		//Allocate input rule in ASIC location
		rtl8651VlanTable[vidx].aclAsicInputMaxNumber = rtl8651_tblDrvPara.aclPerVlanInitInputEntrySize;
		maxSlotStartPos = maxSlotSize = 0;
		for(i=0; i<RTL8651_ACLTBL_SIZE; i++) {
			for(j=0; j<RTL8651_ACLTBL_SIZE - i; j++) {
				if(rtl8651AclAsicArrangementTable[i+j].valid == 1 || i+j == RTL8651_ACLTBL_SIZE-1) {
					if(j > maxSlotSize) {
						maxSlotSize = j;
						maxSlotStartPos = i;
					}
					break;
				}
			}
			i = i+j+1;
		}
		if(maxSlotSize >= rtl8651VlanTable[vidx].aclAsicInputMaxNumber) {
			rtl8651VlanTable[vidx].aclAsicInputPos = maxSlotStartPos;
			rtl8651VlanTable[vidx].aclAsicInputUsed = 0;
			for(i=maxSlotStartPos; i<maxSlotStartPos+rtl8651VlanTable[vidx].aclAsicInputMaxNumber; i++) {
				assert(rtl8651AclAsicArrangementTable[i].valid == 0);
				rtl8651AclAsicArrangementTable[i].vid = rtl8651VlanTable[vidx].vid;
				rtl8651AclAsicArrangementTable[i].input = 1;
				rtl8651AclAsicArrangementTable[i].used = 0;
				rtl8651AclAsicArrangementTable[i].valid = 1;
			}
		}
		else
			rtl8651VlanTable[vidx].aclAsicInputMaxNumber = 0;
		//Allocate output rule in ASIC location
		rtl8651VlanTable[vidx].aclAsicOutputMaxNumber = rtl8651_tblDrvPara.aclPerVlanInitOutputEntrySize;
		maxSlotStartPos = maxSlotSize = 0;
		for(i=0; i<RTL8651_ACLTBL_SIZE; i++) {
			for(j=0; j<RTL8651_ACLTBL_SIZE - i; j++) {
				if(rtl8651AclAsicArrangementTable[i+j].valid == 1 || i+j == RTL8651_ACLTBL_SIZE-1) {
					if(j > maxSlotSize) {
						maxSlotSize = j;
						maxSlotStartPos = i;
					}
					break;
				}
			}
			i = i+j+1;
		}
		if(maxSlotSize >= rtl8651VlanTable[vidx].aclAsicOutputMaxNumber) {
			rtl8651VlanTable[vidx].aclAsicOutputPos = maxSlotStartPos;
			rtl8651VlanTable[vidx].aclAsicOutputUsed = 0;
			for(i=maxSlotStartPos; i<maxSlotStartPos+rtl8651VlanTable[vidx].aclAsicOutputMaxNumber; i++) {
				assert(rtl8651AclAsicArrangementTable[i].valid == 0);
				rtl8651AclAsicArrangementTable[i].vid = rtl8651VlanTable[vidx].vid;
				rtl8651AclAsicArrangementTable[i].input = 0;
				rtl8651AclAsicArrangementTable[i].used = 0;
				rtl8651AclAsicArrangementTable[i].valid = 1;
			}
		}
		else
			rtl8651VlanTable[vidx].aclAsicOutputMaxNumber = 0;
	}
	_rtl8651_updateAsicVlan(vid);//Update new VLAN information into ASIC based on driver table information
	vlanCount = 0;
	for(vidx = 0; vidx<RTL8651_VLAN_NUMBER; vidx++)
		if(rtl8651VlanTable[vidx].valid == 1)
			vlanCount++;
	if(vlanCount == 1)//Add and success and final count == 1. Change from no VLAN to VLAN exist
		rtl8651_setAsicOperationLayer(2);
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

int32 rtl8651_specifyVlanFilterDatabase(uint16 vid, uint16 fid) {
	uint32 vidx = rtl8651_vlanTableIndex(vid);
	
	if(vid == 0 || vid > 4094) {
		return(TBLDRV_EINVVLAN);
	}
	if(fid >= RTL8651_FDB_NUMBER) {
		return(TBLDRV_EINVFID);
	}
	RTL8651_LOCK_RESOURCE();//Lock resource
	
	//Check whether vlan do exist
	if(rtl8651VlanTable[vidx].valid == 0 || rtl8651VlanTable[vidx].vid != vid) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EINVVLAN);
	}

	if(rtl8651VlanTable[vidx].memberPortMask) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EFIDREFERENCED);
	}
	if(rtl8651FilterDbTable[fid].valid == 0) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EINVFID);
	}
	rtl8651VlanTable[vidx].fid = fid;
	RTL8651_UNLOCK_RESOURCE();//Unlock resource

	return SUCCESS;
}

int32 rtl8651_setVlanPromiscuous(uint16 vid, int8 isPromiscuous) {
	uint32 i, vidx = rtl8651_vlanTableIndex(vid);

	if(vid == 0 || vid > 4094) {
		return(TBLDRV_EINVVLAN);
	}

	RTL8651_LOCK_RESOURCE();//Lock resource
	
	if(rtl8651VlanTable[vidx].valid == 0 || rtl8651VlanTable[vidx].vid != vid) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EINVVLAN);
	}
	if(rtl8651VlanTable[vidx].promiscuous == (isPromiscuous==TRUE? 1: 0)) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
//		return FAILED;//Configured value equal to original one
		return SUCCESS;
	}
	if(isPromiscuous == TRUE) {//Checking network interface and pppoe
		for(i=0; i< rtl8651_tblDrvPara.networkIntfTableSize; i++)
			if(rtl8651NetworkIntfTable[i].valid == 1 && rtl8651NetworkIntfTable[i].linkLayerType == RTL8651_LL_VLAN && rtl8651NetworkIntfTable[i].linkLayerIndex == vid) {
				RTL8651_UNLOCK_RESOURCE();//Unlock resource
				return(TBLDRV_EVLANREFERENCED);
			}
		for(i=0; i<RTL8651_PPPOE_NUMBER; i++)
			if(rtl8651PppoeTable[i].valid == 1 && rtl8651PppoeTable[i].vid == vid) {
				RTL8651_UNLOCK_RESOURCE();//Unlock resource
				return(TBLDRV_EVLANREFERENCED);
			}
	}
	rtl8651VlanTable[vidx].promiscuous = isPromiscuous==TRUE? 1: 0;
	_rtl8651_updateAsicVlan(vid);
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

int32 rtl8651_getVlanActiveStatus(uint16 vid, int8 * isActive) {
	uint32 vidx = rtl8651_vlanTableIndex(vid);
	
	if(vid == 0 || vid > 4094) {
		return(TBLDRV_EINVVLAN);

	}
	if (isActive==NULL) {
		return(TBLDRV_EINVVAL);
	}
	RTL8651_LOCK_RESOURCE();//Lock resource
	
	if(rtl8651VlanTable[vidx].valid == 0 || rtl8651VlanTable[vidx].vid != vid) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EINVVLAN);
	}
	*isActive = rtl8651VlanTable[vidx].memberPortUpStatus!= 0? TRUE: FALSE;
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

int32 rtl8651_addVlanPortMember(uint16 vid, uint32 port) {
	uint32 vidx = rtl8651_vlanTableIndex(vid);

	//Impossible VID or port number checking
	if(vid == 0 || vid > 4094) {
		return(TBLDRV_EINVVLAN);
	}
	if (port >= RTL8651_PORT_NUMBER) {
		return(TBLDRV_EINVPORT);

	}
	RTL8651_LOCK_RESOURCE();//Lock resource
	
	if(rtl8651VlanTable[vidx].valid == 0 || rtl8651VlanTable[vidx].vid != vid) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EINVVLAN);
	}
	//Check whether link aggregator valid
	if(rtl8651LinkAggregationTable[port].ethernetPortMask == 0) {//No port belong to aggregator
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EAGGREHASNOPORT);
	}
	if(rtl8651VlanTable[vidx].memberPortMask & rtl8651LinkAggregationTable[port].ethernetPortMask) {//Aggregator has alread belong to VLAN
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		//return FAILED;
		return SUCCESS;
	}
	//Put port into corresponding VLAN port member set
	rtl8651VlanTable[vidx].memberPortMask |= rtl8651LinkAggregationTable[port].ethernetPortMask;
	rtl8651VlanTable[vidx].untagPortMask |= rtl8651LinkAggregationTable[port].ethernetPortMask;//Default add  port to untag list
	rtl8651VlanTable[vidx].memberPortUpStatus |= rtl8651LinkAggregationTable[port].ethernetPortUpStatus;
	//Release all allocated 
	_rtl8651_updateAsicVlan(vid);//Update new VLAN information into ASIC based on driver table information
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

int32 rtl8651_setVlanPortUntag(uint16 vid, uint32 port, int8 untag) {
	uint32 vidx = rtl8651_vlanTableIndex(vid);

	//Impossible VID or port number checking
	if(vid == 0 || vid > 4094) {
		return(TBLDRV_EINVVLAN);

	}
	if (port >= RTL8651_PORT_NUMBER) {
		return(TBLDRV_EINVPORT);

	}
	RTL8651_LOCK_RESOURCE();//Lock resource
	
	if((rtl8651VlanTable[vidx].memberPortMask & rtl8651LinkAggregationTable[port].ethernetPortMask) == 0) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_ENOTVLANPORTMEMBER);
	}
	if(untag == TRUE)
		rtl8651VlanTable[vidx].untagPortMask |= rtl8651LinkAggregationTable[port].ethernetPortMask;
	else
		rtl8651VlanTable[vidx].untagPortMask &= ~rtl8651LinkAggregationTable[port].ethernetPortMask;
	_rtl8651_updateAsicVlan(vid);//Update new VLAN information into ASIC based on driver table information
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

int32 rtl8651_assignVlanMacAddress(uint16 vid, ether_addr_t * macAddress, uint16 macAddrNumber) {
	uint32 i, j;
	if(vid < 1 || vid > 4094) {
		return(TBLDRV_EINVVLAN);
	}
	if(macAddress->octet[5] & (macAddrNumber-1)) {
		return(TBLDRV_EINVVAL);
		
	}
	RTL8651_LOCK_RESOURCE();//Lock resource
	
	if(rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].valid == 0 || rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].vid != vid ) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EINVVLAN);
		
	}
	for(i=0; i<RTL8651_PPPOE_NUMBER; i++)
		if(rtl8651PppoeTable[i].valid == 1 && rtl8651PppoeTable[i].vid == vid) {
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return(TBLDRV_EVLANMACREFERENCED);
	 		
		}
	for(i=0; i< rtl8651_tblDrvPara.networkIntfTableSize; i++)
		if(rtl8651NetworkIntfTable[i].valid == 1 && rtl8651NetworkIntfTable[i].linkLayerType == RTL8651_LL_VLAN && rtl8651NetworkIntfTable[i].linkLayerIndex == vid) {
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return(TBLDRV_EVLANMACREFERENCED);
			
		}
	if(_rtl8651_returnPortMaskPortNumber(macAddrNumber) > 1) {//Not allow more than one bit setting (0, 1, 2, 4 ... are allowed) 
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EINVVLANMAC);
	}
	if(rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].manualAssign == 0 && rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].macAddrNumber > 0) {
		for(i=0; i<rtl8651_tblDrvPara.macAddressDbSize; i++)
			if(memcmp(&rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].macAddr, &macAddressTable[i].mac, sizeof(ether_addr_t)) == 0) {//Entry found
				for(j=0; j<rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].macAddrNumber; j++) {
					assert(macAddressTable[i+j].valid == 1 && macAddressTable[i+j].allocated == 1);
					macAddressTable[i+j].allocated = 0;//Release allocated mac address
				}
				break;//Stop mac address table linear search
			}
	}
	//Allocate new mac address
	rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].macAddr = *macAddress;
	rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].macAddrNumber = macAddrNumber;
	rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].manualAssign = 1;
	_rtl8651_updateAsicVlan(vid);//Update new VLAN information into ASIC based on driver table information
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

int32 rtl8651_allocateVlanMacAddress(uint16 vid, uint16 macAddrNumber) {
	uint32 i, j, slotStartPos;
	uint16 systemAllocatedMacNumber;
	int8 slotFound;
		
	if(vid < 1 || vid > 4094) {//vid should be legal vlan ID
		return(TBLDRV_EINVVLAN);
	}
	RTL8651_LOCK_RESOURCE();//Lock resource
	
	if(rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].valid == 0 || rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].vid != vid ) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EINVVLAN);
		
	}
	for(i=0; i<RTL8651_PPPOE_NUMBER; i++)
		if(rtl8651PppoeTable[i].valid == 1 && rtl8651PppoeTable[i].vid == vid) {
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return(TBLDRV_EVLANMACREFERENCED);
	 		
		}
	for(i=0; i< rtl8651_tblDrvPara.networkIntfTableSize; i++)
		if(rtl8651NetworkIntfTable[i].valid == 1 && rtl8651NetworkIntfTable[i].linkLayerType == RTL8651_LL_VLAN && rtl8651NetworkIntfTable[i].linkLayerIndex == vid) {
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return(TBLDRV_EVLANMACREFERENCED);//VLAN MAC used by IP interface, forbid re-allocation
		}
			
	if(_rtl8651_returnPortMaskPortNumber(macAddrNumber) > 1) {//Not allow more than one bit setting (0, 1, 2, 4 ... are allowed)
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EINVVLANMAC);
	}

	if(rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].manualAssign == 1)
		systemAllocatedMacNumber = 0;
	else
		systemAllocatedMacNumber = rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].macAddrNumber;
	if(systemAllocatedMacNumber < macAddrNumber) {
		//Evaluate whether remaining free mac is large enough
		slotFound = FALSE;
		for(i=0; i<rtl8651_tblDrvPara.macAddressDbSize; i += macAddrNumber) {
			if(macAddressTable[i].valid == 1 && macAddressTable[i].allocated == 0) {
				for(j=0; j<macAddrNumber; j++)
					if(macAddressTable[i+j].valid == 0 || macAddressTable[i+j].allocated == 1) 
						break;
				if(j == macAddrNumber) {
					slotStartPos = i;
					slotFound = TRUE;
					break;
				}
			}
		}
		if(slotFound == FALSE) {
			if(systemAllocatedMacNumber>0 && (rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].macAddr.octet[5] & (macAddrNumber-1)) == 0) {//Try to find (macAddrNumber - systemAllocatedMacNumber) at allocated position
				for(i=0; i<rtl8651_tblDrvPara.macAddressDbSize; i++)
					if(memcmp(&rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].macAddr, &macAddressTable[i].mac, sizeof(ether_addr_t)) == 0) {//Entry found
						for(j=systemAllocatedMacNumber; j<macAddrNumber; j++)
							if(macAddressTable[i+j].valid == 0 || macAddressTable[i+j].allocated == 1) 
							break;
						if(j==macAddrNumber) {
							rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].macAddrNumber = macAddrNumber;
							assert((macAddressTable[i].mac.octet[5] & (macAddrNumber-1)) == 0);
							rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].manualAssign = 0;
							//Occupy allocated range
							for(j=systemAllocatedMacNumber; j<macAddrNumber; j++) {
								assert(macAddressTable[i+j].valid == 1);
								assert(macAddressTable[i+j].allocated == 0);
								macAddressTable[i+j].allocated = 1;
							}
							_rtl8651_updateAsicVlan(vid);//Update new VLAN information into ASIC based on driver table information
							RTL8651_UNLOCK_RESOURCE();//Unlock resource
							return SUCCESS;
						}
						else {
							RTL8651_UNLOCK_RESOURCE();//Unlock resource
							return(TBLDRV_ENOUSABLEMAC);//Slot contian current allocated part still not found
						}
					}
			}
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return(TBLDRV_ENOUSABLEMAC);//Unable to allocate specified MAC number
		}
		//Release previous allocated mac address
		if(systemAllocatedMacNumber > 0) {
			for(i=0; i<rtl8651_tblDrvPara.macAddressDbSize; i++)
				if(memcmp(&rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].macAddr, &macAddressTable[i].mac, sizeof(ether_addr_t)) == 0) {//Entry found
					for(j=0; j<rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].macAddrNumber; j++) {
						assert(macAddressTable[i+j].valid == 1 && macAddressTable[i+j].allocated == 1);
						macAddressTable[i+j].allocated = 0;//Release allocated mac address
					}
					break;//Stop mac address table linear search
				}
		}
		//Allocate new mac address
		rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].macAddr = macAddressTable[slotStartPos].mac;
		rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].macAddrNumber = macAddrNumber;
		assert((macAddressTable[slotStartPos].mac.octet[0] & (macAddrNumber-1)) == 0);
		rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].manualAssign = 0;
		//Occupy allocated range
		for(j=0; j<macAddrNumber; j++) {
			assert(macAddressTable[slotStartPos+j].valid == 1);
			assert(macAddressTable[slotStartPos+j].allocated == 0);
			macAddressTable[slotStartPos+j].allocated = 1;
		}
		_rtl8651_updateAsicVlan(vid);//Update new VLAN information into ASIC based on driver table information
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return SUCCESS;
	}
	//Process rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].macAddrNumber == macAddrNumber	
	if(rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].macAddrNumber == macAddrNumber) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return SUCCESS;
	}
	//Process rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].macAddrNumber > macAddrNumber	
	for(i=0; i<rtl8651_tblDrvPara.macAddressDbSize; i++)
		if(memcmp(&rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].macAddr, &macAddressTable[i].mac, sizeof(ether_addr_t)) == 0) {//Entry found
			for(j=i+macAddrNumber; j< i+rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].macAddrNumber; j++) {
				assert(macAddressTable[j].allocated == 1);
				macAddressTable[j].allocated = 0;
			}
			rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].macAddrNumber = macAddrNumber;
			_rtl8651_updateAsicVlan(vid);//Update new VLAN information into ASIC based on driver table information
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return SUCCESS;
		}
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return(TBLDRV_ENOUSABLEMAC);
	
}

int32 rtl8651_getVlanMacAddress(uint16 vid, ether_addr_t * macAddr, uint16 * macAddrNumber) {
	
	if(vid < 1 || vid > 4094) { //vid should be legal vlan ID
		return(TBLDRV_EINVVLAN);
		
	}
	RTL8651_LOCK_RESOURCE();//Lock resource
	
	if(rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].valid == 0 || rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].vid != vid ) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EINVVLAN);//Vlan do not exist
		
	}
	//Release previous allocated mac address
	if(rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].macAddrNumber > 0) {
		*macAddr = rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].macAddr;
		if(macAddrNumber)
			*macAddrNumber = rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].macAddrNumber;
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return SUCCESS;
	}
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return(TBLDRV_EVLANNOMAC);

}

int32 rtl8651_delVlan(uint16 vid) {
	uint32 i, vidx = rtl8651_vlanTableIndex(vid), vlanCount;

	if(vid < 1 || vid > 4094) 
		return(TBLDRV_EINVVLAN);
	/* System assumption, not permit vlan1 to be removed */
	if(vid == 1)
		return(TBLDRV_EDEFAULTVLAN);

	RTL8651_LOCK_RESOURCE();//Lock resource
	if(rtl8651VlanTable[vidx].valid == 0 || rtl8651VlanTable[vidx].vid != vid ) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EINVVLAN);
	}
	/* When IP interface using this VLAN, forbid to delete this VLAN */
	for(i=0; i< rtl8651_tblDrvPara.networkIntfTableSize; i++) 
		if(rtl8651NetworkIntfTable[i].valid == 1 && rtl8651NetworkIntfTable[i].linkLayerType == RTL8651_LL_VLAN && rtl8651NetworkIntfTable[i].linkLayerIndex == vid) {
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return(TBLDRV_EVLANREFERENCED);
		}
	/* PPPoE session references this vlan, forbid */
	for(i=0; i<RTL8651_PPPOE_NUMBER; i++)
		if(rtl8651PppoeTable[i].valid == 1&& rtl8651PppoeTable[i].vid == vid) {
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return(TBLDRV_EVLANREFERENCED);
		}
	/* Removing vlan should not allow mac address attached */
	if(rtl8651VlanTable[vidx].macAddrNumber) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EVLANMACNOTFREED);
	}
	/* Release allocated input and output ASIC ACL rule space */
	for(i=rtl8651VlanTable[vidx].aclAsicInputPos; i<rtl8651VlanTable[vidx].aclAsicInputPos + rtl8651VlanTable[vidx].aclAsicInputMaxNumber; i++) {
		assert(rtl8651AclAsicArrangementTable[i].valid == 1);
		rtl8651AclAsicArrangementTable[i].valid = 0;
	}
	for(i=rtl8651VlanTable[vidx].aclAsicOutputPos; i<rtl8651VlanTable[vidx].aclAsicOutputPos + rtl8651VlanTable[vidx].aclAsicOutputMaxNumber; i++) {
		assert(rtl8651AclAsicArrangementTable[i].valid == 1);
		rtl8651AclAsicArrangementTable[i].valid = 0;
	}
	/* Since vlan 1 always exist and ASIC must have one valid VLAN as PVID */
	for(i=0; i<RTL8651_PORT_NUMBER; i++)
		if(rtl8651LinkAggregationTable[i].ethernetPortMask && rtl8651LinkAggregationTable[i].pvid == vid)
			rtl8651_setPvid(i, 1);
	rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].valid = 0;
	rtl8651_delAsicVlan(vid);//Write to ASIC table
	//Update operation layer
	vlanCount = 0;
	for(vidx = 0; vidx<RTL8651_VLAN_NUMBER; vidx++)
		if(rtl8651VlanTable[vidx].valid == 1)
			vlanCount++;
	if(vlanCount == 0)//Delete and success and final count == 0. Change from VLAN exist to no VLAN exist
		rtl8651_setAsicOperationLayer(1);
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

int32 rtl8651_delVlanPortMember(uint16 vid, uint32 port) {
	uint32 vidx = rtl8651_vlanTableIndex(vid);

	//Impossible VID or port number checking
	if(vid == 0 || vid > 4094) {
		return(TBLDRV_EINVVLAN);

	}
	if (port >= RTL8651_PORT_NUMBER) {
		return(TBLDRV_EINVPORT);

	}
	RTL8651_LOCK_RESOURCE();//Lock resource
	
	if(rtl8651VlanTable[vidx].valid == 0 || rtl8651VlanTable[vidx].vid != vid) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EINVVLAN);

	}
	//Check whether link aggregator valid
	if(rtl8651LinkAggregationTable[port].ethernetPortMask == 0) {//No port belong to aggregator
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EAGGREHASNOPORT);

	}
	//Check whether port in corresponding VLAN port member set
	if((rtl8651VlanTable[vidx].memberPortMask & rtl8651LinkAggregationTable[port].ethernetPortMask) == 0) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_ENOTVLANPORTMEMBER);

	}
	//Remove port from member set
	rtl8651VlanTable[vidx].memberPortMask &=  ~rtl8651LinkAggregationTable[port].ethernetPortMask;
	rtl8651VlanTable[vidx].untagPortMask &=  ~rtl8651LinkAggregationTable[port].ethernetPortMask;
	rtl8651VlanTable[vidx].memberPortUpStatus &= ~rtl8651LinkAggregationTable[port].ethernetPortUpStatus;
	_rtl8651_updateAsicVlan(vid);//Update new VLAN information into ASIC based on driver table information
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}


int8 _rtl8651_getPppoeTblIndex(uint32 pppoeId, uint32 *tblIdx) {

	for(*tblIdx=0; *tblIdx<RTL8651_PPPOE_NUMBER; (*tblIdx)++)
		if (rtl8651PppoeTable[*tblIdx].valid == 1 && 
			rtl8651PppoeTable[*tblIdx].objectId == pppoeId)
			return SUCCESS;
 	return FAILED;
}



/*
1. Guarantee each pppoe session with different MAC address or allocation will fail
2. VLAN index should not have IP interface direct attached over it (VLAN style)
3. Allow multiple pppoe session attached over same VLAN, but network interface should guarantee

*/
int32 rtl8651_addPppoeSession(uint32 pppoeId, uint16 vid) {//Session Db Index
	uint32 i, j, dupMacIdx, vidx = rtl8651_vlanTableIndex(vid);

	if(vid < 1 || vid > 4094)
		return(TBLDRV_EINVVLAN);
		
	RTL8651_LOCK_RESOURCE();//Lock resource
	
	if(rtl8651VlanTable[vidx].valid == 0 || rtl8651VlanTable[vidx].vid != vid) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EINVVLAN);

	}
	if (rtl8651VlanTable[vidx].promiscuous == 1) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return(TBLDRV_EVLANPROMIS);

	}
	for(i=0; i< rtl8651_tblDrvPara.networkIntfTableSize; i++)
		if(rtl8651NetworkIntfTable[i].valid == 1 && rtl8651NetworkIntfTable[i].linkLayerType == RTL8651_LL_VLAN && rtl8651NetworkIntfTable[i].linkLayerIndex == vid) {
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return(TBLDRV_EVLANREFERENCED);

		}
	for(i=0; i<RTL8651_PPPOE_NUMBER; i++)
		if(rtl8651PppoeTable[i].valid == 1 && rtl8651PppoeTable[i].vid == vid) {
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return(TBLDRV_EMULTIPPPOE);//Current not support multiple pppoe session over same VLAN

		}
	for(i=0; i<RTL8651_PPPOE_NUMBER; i++)
		if(rtl8651PppoeTable[i].valid == 0) {//Available slot is found
			//Guarantee different source MAC address per-pppoe session
			for(j=0, dupMacIdx = RTL8651_PPPOE_NUMBER; j<RTL8651_PPPOE_NUMBER; j++) {
				if(i != j && rtl8651PppoeTable[j].valid == 1 && rtl8651PppoeTable[j].vid == vid && 
					(i & (rtl8651VlanTable[vidx].macAddrNumber - 1)) == (j & (rtl8651VlanTable[vidx].macAddrNumber - 1))) {
					dupMacIdx = i;
					break;
				}
			}
			if(i != dupMacIdx) {
				rtl8651PppoeTable[i].objectId = pppoeId;
				rtl8651PppoeTable[i].valid = 1;
				rtl8651PppoeTable[i].vid = vid;
				rtl8651PppoeTable[i].vlanMacSelect = i & (rtl8651VlanTable[vidx].macAddrNumber - 1);
				rtl8651PppoeTable[i].initiated = 0;
				/*
				 * turn off Napt auto add
				 */
				if (i == 0)
					rtl8651_setAsicNaptAutoAddDelete(FALSE, TRUE);
				RTL8651_UNLOCK_RESOURCE();//Unlock resource
				return SUCCESS;
			}
		}
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return(TBLDRV_ENOFREEBUF);

}

int32 rtl8651_getPppoeSessionMac(uint32 pppoeId, ether_addr_t * macAddr) {
	uint32 i, mask;

	RTL8651_LOCK_RESOURCE();//Lock resource
	
	for(i=0; i< RTL8651_PPPOE_NUMBER; i++)
		if(rtl8651PppoeTable[i].valid == 1 && rtl8651PppoeTable[i].objectId == pppoeId && 
			rtl8651VlanTable[rtl8651_vlanTableIndex(rtl8651PppoeTable[i].vid)].macAddrNumber > 0) {
				*macAddr = rtl8651VlanTable[rtl8651_vlanTableIndex(rtl8651PppoeTable[i].vid)].macAddr;
				mask = rtl8651VlanTable[rtl8651_vlanTableIndex(rtl8651PppoeTable[i].vid)].macAddrNumber - 1;
				macAddr->octet[5] = (macAddr->octet[5] & ~mask) | (i & mask);
				RTL8651_UNLOCK_RESOURCE();//Unlock resource
				return SUCCESS;
			}
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return(TBLDRV_EINVPPPOE);//Specified pppoeId not found

}

int32 rtl8651_delPppoeSession(uint32 pppoeId) {//Session Db Index
	uint32 i, j;

	RTL8651_LOCK_RESOURCE();//Lock resource
	
	for(i=0; i< RTL8651_PPPOE_NUMBER; i++)
		if(rtl8651PppoeTable[i].valid == 1 && rtl8651PppoeTable[i].objectId == pppoeId) {//Specified pppoe Id found
			if(rtl8651PppoeTable[i].initiated == 1) {//Forbid not deleted session to be freed
				RTL8651_UNLOCK_RESOURCE();//Unlock resource
				return(TBLDRV_EPPPOEREFERENCED);
				
			}
			for(j=0; j< rtl8651_tblDrvPara.networkIntfTableSize; j++) //When IP interface using this pppoe session, forbid to free this pppoe session
				if(rtl8651NetworkIntfTable[j].valid == 1 && rtl8651NetworkIntfTable[j].linkLayerType == RTL8651_LL_PPPOE && rtl8651NetworkIntfTable[j].linkLayerIndex == pppoeId) {
					RTL8651_UNLOCK_RESOURCE();//Unlock resource
					return(TBLDRV_EPPPOEREFERENCED);
					
				}
			rtl8651PppoeTable[i].valid = 0;
			/*
			 * Turn on Napt auto add
			 */
			for(j=0; j<RTL8651_PPPOE_NUMBER; j++)
				if (rtl8651PppoeTable[j].valid == 1)
					break;
			if (j == RTL8651_PPPOE_NUMBER)
				rtl8651_setAsicNaptAutoAddDelete(TRUE, TRUE);
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return SUCCESS;
		}
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return(TBLDRV_EINVPPPOE);//PPPoE ID does not exist
}

int32 rtl8651_setPppoeSessionProperty(uint32 pppoeId, uint16 sid, ether_addr_t * macAddr, uint32 port) {//Session ID
	uint32 i, j;

	if(sid == 0xffff) { //RFC2516 p.4 define 0xffff MUST NOT be used
		return(TBLDRV_ESESSIONID);
		
	}
	RTL8651_LOCK_RESOURCE();//Lock resource
	
	for(i=0; i< RTL8651_PPPOE_NUMBER; i++)
		if(rtl8651PppoeTable[i].valid == 1 && rtl8651PppoeTable[i].objectId == pppoeId) {//Specified pppoe Id found
			if(rtl8651LinkAggregationTable[port].ethernetPortMask == 0) {//No port belong to aggregator
				RTL8651_UNLOCK_RESOURCE();//Unlock resource
				return(TBLDRV_EAGGREHASNOPORT);//Port invalid due to aggregator invalid
			}
			for(j=0; j<RTL8651_PPPOE_NUMBER; j++)//Check whether property already configured for other pppoe object
				if(rtl8651PppoeTable[j].valid == 1 && rtl8651PppoeTable[j].initiated == 1 && //Session information valid
					rtl8651PppoeTable[j].objectId != pppoeId && rtl8651PppoeTable[j].sessionId == sid && 
					memcmp(&rtl8651PppoeTable[j].macInfo->macAddr, macAddr, sizeof(ether_addr_t)) == 0) {
					RTL8651_UNLOCK_RESOURCE();//Unlock resource
					return(TBLDRV_EPROPERTYCONFLICT);

				}
			if(rtl8651PppoeTable[i].initiated == 1) {//Forbid initiated session overwritten
				RTL8651_UNLOCK_RESOURCE();//Unlock resource
				return(TBLDRV_EPROPERTYWASSET);

			}
			if(_rtl8651_addVlanRefFilterDatabaseEntry(rtl8651PppoeTable[i].vid, macAddr, 1<<port) == FAILED) {
				RTL8651_UNLOCK_RESOURCE();//Unlock resource
				return(TBLDRV_EADDFDFAIL);

			}
			rtl8651PppoeTable[i].initiated = 1;
			rtl8651PppoeTable[i].sessionId = sid;
			rtl8651PppoeTable[i].macInfo = _rtl8651_getVlanFilterDatabaseEntry(rtl8651PppoeTable[i].vid, macAddr);
			//Check whether pening routing entry can be write 
			for(j=0; j<rtl8651_tblDrvPara.routingTableSize;j++)
				if(rtl8651RoutingTable[j].valid == 1 && rtl8651RoutingTable[j].routeAllocated == 1 && rtl8651NetworkIntfTable[rtl8651RoutingTable[j].networkIntfIndex].linkLayerType == RTL8651_LL_PPPOE &&
					rtl8651NetworkIntfTable[rtl8651RoutingTable[j].networkIntfIndex].linkLayerIndex == rtl8651PppoeTable[i].objectId && rtl8651RoutingTable[j].pppoePending == 1) {
					assert(rtl8651RoutingTable[j].macPending == 0);
					assert(rtl8651RoutingTable[j].arpPending == 0);
					rtl8651RoutingTable[j].pppoePending = 0;
					if(rtl8651PppoeTable[i].macInfo->configToAsic == 1) {
						rtl8651RouteAsicArrangementTable[rtl8651RoutingTable[j].routingTablePos].type = 2;
						rtl8651_setAsicRouting(rtl8651RoutingTable[j].routingTablePos, rtl8651RoutingTable[j].ipAddr, rtl8651RoutingTable[j].ipMask, 0, rtl8651_vlanTableIndex(rtl8651RoutingTable[j].vid), 0, 0, rtl8651_filterDbIndex(&rtl8651PppoeTable[i].macInfo->macAddr), rtl8651PppoeTable[i].macInfo->asicPos, i);
					}
					else 
						rtl8651RoutingTable[j].macPending = 1;
				}
			/* Check Whether the PPPoE is referenced by routing already */
			for(j=0; j<rtl8651_tblDrvPara.routingTableSize;j++)
				if (rtl8651RoutingTable[j].valid == 1 &&  rtl8651NetworkIntfTable[rtl8651RoutingTable[j].networkIntfIndex].linkLayerType == RTL8651_LL_PPPOE &&
					rtl8651NetworkIntfTable[rtl8651RoutingTable[j].networkIntfIndex].linkLayerIndex == rtl8651PppoeTable[i].objectId) {
					assert( rtl8651RoutingTable[j].nextHopType == RTL8651_LL_PPPOE);
					rtl8651RoutingTable[j].pppoePtr = &rtl8651PppoeTable[i];
					break;
				}				
			rtl8651_setAsicPppoe(i, rtl8651PppoeTable[i].sessionId);//Configure to ASIC
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return SUCCESS;
		}
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return(TBLDRV_EINVPPPOE);//No specified pppoe found

}

int32 rtl8651_resetPppoeSessionProperty(uint32 pppoeId) {
	uint32 i, j;

	RTL8651_LOCK_RESOURCE();//Lock resource
	
	for(i=0; i< RTL8651_PPPOE_NUMBER; i++)
		if(rtl8651PppoeTable[i].valid == 1 && rtl8651PppoeTable[i].objectId == pppoeId) {//Specified pppoe Id found
			rtl8651PppoeTable[i].initiated = 0;
			for(j=0; j<rtl8651_tblDrvPara.routingTableSize; j++)//Remove referenced routing entry from ASIC
				if(rtl8651RoutingTable[j].valid == 1 && rtl8651RoutingTable[j].routeAllocated == 1 && rtl8651NetworkIntfTable[rtl8651RoutingTable[j].networkIntfIndex].linkLayerType == RTL8651_LL_PPPOE && rtl8651NetworkIntfTable[rtl8651RoutingTable[j].networkIntfIndex].linkLayerIndex == pppoeId) { //Nexthop is reference to the deleted arp entry
					assert(rtl8651RoutingTable[j].pppoePending == 0);
					assert(rtl8651RoutingTable[j].arpPending == 0);
					if(rtl8651RoutingTable[j].macPending == 0) {
						rtl8651RouteAsicArrangementTable[rtl8651RoutingTable[j].routingTablePos].type = 3;
						rtl8651_setAsicRouting(rtl8651RoutingTable[j].routingTablePos, rtl8651RoutingTable[j].ipAddr, rtl8651RoutingTable[j].ipMask, 4, rtl8651_vlanTableIndex(rtl8651RoutingTable[j].vid), 0, 0, 0, 0, 0);
					}
					else
						rtl8651RoutingTable[j].macPending = 0;
					rtl8651RoutingTable[j].pppoePending = 1;
				}
			/* unbind the relationship of routing entry and PPPoE */
			for(j=0; j<rtl8651_tblDrvPara.routingTableSize; j++)
				if(rtl8651RoutingTable[j].valid == 1 && rtl8651RoutingTable[j].pppoePtr == &rtl8651PppoeTable[i]) {
					rtl8651RoutingTable[j].pppoePtr = NULL;
					break;
				}
			_rtl8651_delVlanRefFilterDatabaseEntry(rtl8651PppoeTable[i].vid, &rtl8651PppoeTable[i].macInfo->macAddr);
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return SUCCESS;
		}
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return(TBLDRV_EINVPPPOE);//No specified pppoe found

}

int32 rtl8651_setNetIfAclMismatchDrop(int8 *ifName, int8 isDrop) {
	uint32 i;
	
	RTL8651_LOCK_RESOURCE();//Lock resource
	
	//Interface name check
	for(i=0; i<rtl8651_tblDrvPara.networkIntfTableSize; i++) 
		if (rtl8651NetworkIntfTable[i].valid == 1 && strncmp(rtl8651NetworkIntfTable[i].ifName, ifName, RTL8651_IPINTF_NAME_LEN) == 0) {
			rtl8651NetworkIntfTable[i].aclDefPermit = ((isDrop==TRUE)? 0: 1);
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return SUCCESS;			
		}			
	//Otherwise Specified interface not found
	RTL8651_UNLOCK_RESOURCE();//Lock resource
	return TBLDRV_EINVNETIFNAME;
}
	
int32 rtl8651_addNetworkIntf(int8 *ifName) {
	uint32 netIdx;

	RTL8651_LOCK_RESOURCE();//Lock resource
	//Duplicate name checking
	for(netIdx=0; netIdx< rtl8651_tblDrvPara.networkIntfTableSize; netIdx++)
		if(rtl8651NetworkIntfTable[netIdx].valid == 1 && //Entry valid, imply interface name is valid as well
			strncmp(rtl8651NetworkIntfTable[netIdx].ifName, ifName, RTL8651_IPINTF_NAME_LEN) == 0) {//Duplicate interface name
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return(TBLDRV_EIFNAMEEXIST);
		}
	//Search for empty Network interface slot
	for(netIdx=0; netIdx< rtl8651_tblDrvPara.networkIntfTableSize; netIdx++)
		if(rtl8651NetworkIntfTable[netIdx].valid == 0) {//Empty entry found and create Network interface
			strncpy(rtl8651NetworkIntfTable[netIdx].ifName, ifName, RTL8651_IPINTF_NAME_LEN);
			rtl8651NetworkIntfTable[netIdx].linkLayerType = 0;
			rtl8651NetworkIntfTable[netIdx].IpHead = NULL;
			rtl8651NetworkIntfTable[netIdx].inAclHead = NULL;
			rtl8651NetworkIntfTable[netIdx].inAclTail = NULL;
			rtl8651NetworkIntfTable[netIdx].outAclHead = NULL;
			rtl8651NetworkIntfTable[netIdx].outAclTail = NULL;
			rtl8651NetworkIntfTable[netIdx].nextNaptServerPort = NULL;
			rtl8651NetworkIntfTable[netIdx].nextNat = NULL;
			rtl8651NetworkIntfTable[netIdx].valid = 1;
			rtl8651NetworkIntfTable[netIdx].aclDefPermit = 0; //Default Drop packet if mismatch
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return SUCCESS;
		}
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return(TBLDRV_ENOFREEBUF);
}

rtl8651_tblDrv_networkIntfTable_t * _rtl8651_getNetworkIntfEntryByAddr(ipaddr_t ipAddr) {
	uint32 ifIndex;
	rtl8651_tblDrv_ipIntfEntry_t * ipIfPtr;
	
	//Request local network host, try to generate arp query
	for(ifIndex=0; ifIndex< rtl8651_tblDrvPara.networkIntfTableSize; ifIndex++)
		if(rtl8651NetworkIntfTable[ifIndex].valid == 1 && rtl8651NetworkIntfTable[ifIndex].linkLayerType == RTL8651_LL_VLAN ) {//Entry match
			ipIfPtr = rtl8651NetworkIntfTable[ifIndex].IpHead;
			while(ipIfPtr) {
				if((ipIfPtr->ipAddr->ipAddr & ipIfPtr->ipMask) == (ipAddr & ipIfPtr->ipMask)) 
					return &rtl8651NetworkIntfTable[ifIndex];//ipAddr belonged network interface is found
				ipIfPtr = ipIfPtr->nextIp;
			}
		}
	return NULL;//Entry not found
}

rtl8651_tblDrv_networkIntfTable_t * _rtl8651_getNetworkIntfEntryByVid(uint16 vid) {
	uint32 ifIndex;
	
	for(ifIndex=0; ifIndex< rtl8651_tblDrvPara.networkIntfTableSize; ifIndex++)
		if(rtl8651NetworkIntfTable[ifIndex].valid == 1 && rtl8651NetworkIntfTable[ifIndex].linkLayerType == RTL8651_LL_VLAN && rtl8651NetworkIntfTable[ifIndex].linkLayerIndex == vid) 
			return &rtl8651NetworkIntfTable[ifIndex];
	return NULL;//Entry not found
}

/*
Blocked by
1. External IP interface
*/
int32 rtl8651_delNetworkIntf(int8 *ifName) {
	uint32 i, j;
	uint16 vid;

	RTL8651_LOCK_RESOURCE();//Lock resource
	for(i=0; i< rtl8651_tblDrvPara.networkIntfTableSize; i++)
		if (rtl8651NetworkIntfTable[i].valid == 1 &&
			strncmp(rtl8651NetworkIntfTable[i].ifName, ifName, RTL8651_IPINTF_NAME_LEN) == 0)
			break;
	/* Check if specified network interface exists */
	if (i == rtl8651_tblDrvPara.networkIntfTableSize) {
		RTL8651_UNLOCK_RESOURCE();
		return TBLDRV_EINVNETIFNAME;
	}
	/* Not allow Network interface with IP interface to be removed */
	if(rtl8651NetworkIntfTable[i].IpHead) {
		RTL8651_UNLOCK_RESOURCE();
		return(TBLDRV_ENETIFREFBYIPIF);
	}
	/* Not allowd Network interface referenced by routing table to be removed */
	for(j=0; j<rtl8651_tblDrvPara.routingTableSize-1; j++)
		if(rtl8651RoutingTable[j].valid == 1 && rtl8651RoutingTable[j].networkIntfIndex == i) {
			RTL8651_UNLOCK_RESOURCE();
			return(TBLDRV_ENETIFREFBYROUTE);
		}
	/* The Link layer type should be reset */
	if(rtl8651NetworkIntfTable[i].linkLayerType != 0) {
		RTL8651_UNLOCK_RESOURCE();
		return TBLDRV_ELLNOTRESET;
	}
	/* Acl rule should be empty */
	if(rtl8651NetworkIntfTable[i].inAclHead || rtl8651NetworkIntfTable[i].inAclTail || rtl8651NetworkIntfTable[i].outAclHead || rtl8651NetworkIntfTable[i].outAclTail) {
		RTL8651_UNLOCK_RESOURCE();
		return TBLDRV_ENETIFREFBYACL;
	}
	/* Ok! Invalidate the Network interface */
	rtl8651NetworkIntfTable[i].valid = 0;
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}


int8 _rtl8651_getNetworkIntfLinkLayerType(int8 *ifName, uint32 *llType, uint32 *llIndex) {
	uint32 netIdx;

	for(netIdx=0; netIdx<rtl8651_tblDrvPara.networkIntfTableSize; netIdx++)
		if (rtl8651NetworkIntfTable[netIdx].valid == 1 &&
			strncmp(rtl8651NetworkIntfTable[netIdx].ifName, ifName, RTL8651_IPINTF_NAME_LEN) == 0) {
			if (llType != NULL)
				*llType = rtl8651NetworkIntfTable[netIdx].linkLayerType;
			if (llIndex != NULL)
				*llIndex = rtl8651NetworkIntfTable[netIdx].linkLayerIndex;
			return SUCCESS;
		}
	return FAILED;
}


/*
llType is either RTL8651_LL_VLAN or RTL8651_LL_PPPOE
Process:
1. Only when Network interface is not specify link layer type before, the function can specify the link layer index
2. Cannot map more than one Network interface to the same VLAN or PPPoE session
3. If llType is RTL8651_LL_VLAN, the llIndex is VLAN ID; otherwise is PPPoE Object ID
*/
int32 rtl8651_specifyNetworkIntfLinkLayerType(int8 * ifName, uint32 llType, uint32 llIndex) {
	uint32 j, netIdx;
	uint16 vid;

	if(llType != RTL8651_LL_VLAN && llType != RTL8651_LL_PPPOE)
		return TBLDRV_EINVVAL;

	RTL8651_LOCK_RESOURCE();//Lock resource
	for(netIdx=0; netIdx<rtl8651_tblDrvPara.networkIntfTableSize; netIdx++)
		if (rtl8651NetworkIntfTable[netIdx].valid == 1 &&
			strncmp(rtl8651NetworkIntfTable[netIdx].ifName, ifName, RTL8651_IPINTF_NAME_LEN) == 0)
			break;
	/* Check to see if the specified network interface exists */
	if (netIdx == rtl8651_tblDrvPara.networkIntfTableSize) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return TBLDRV_EINVNETIFNAME;//Interface not exist
	}
	/* Check to see if the link layer type has been specified */
	if (rtl8651NetworkIntfTable[netIdx].linkLayerType != 0) {
		RTL8651_UNLOCK_RESOURCE();
		return TBLDRV_ELLSPECIFIED;
	}
	if (llType == RTL8651_LL_VLAN) {
		/* VID should be a legal vlan ID */
		if(llIndex == 0 || llIndex > 4094) {
			RTL8651_UNLOCK_RESOURCE();
			return TBLDRV_EINVVLAN;
		}
		/* Check to see if correspoinding vlan exists */
		if(rtl8651VlanTable[rtl8651_vlanTableIndex(llIndex)].valid == 0 || rtl8651VlanTable[rtl8651_vlanTableIndex(llIndex)].vid != llIndex) {
			RTL8651_UNLOCK_RESOURCE();
			return TBLDRV_EINVVLAN;
		}
		/* If promiscuous mode is enabled, forbid to bind Network interface to VLAN */
		if (rtl8651VlanTable[rtl8651_vlanTableIndex(llIndex)].promiscuous == 1) {
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return TBLDRV_EVLANPROMIS;//promiscuous mode is enabled
		}
		/* Specified vlan should not be used by PPPoE Session */
		for(j=0; j<RTL8651_PPPOE_NUMBER; j++)
			if(rtl8651PppoeTable[j].valid == 1 && rtl8651PppoeTable[j].vid == llIndex) {
				RTL8651_UNLOCK_RESOURCE();
				return TBLDRV_EVLANREFERENCED;
			}
		vid = llIndex;
	} else if (llType == RTL8651_LL_PPPOE) {
		for(j=0; j<RTL8651_PPPOE_NUMBER; j++)
			if(rtl8651PppoeTable[j].valid == 1 && rtl8651PppoeTable[j].objectId == llIndex)
				break;
		if(j == RTL8651_PPPOE_NUMBER) {
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return TBLDRV_EINVPPPOE; // No specified pppoe id found
		}
		vid = rtl8651PppoeTable[j].vid;
	}
	for(j=0; j< rtl8651_tblDrvPara.networkIntfTableSize; j++)//Duplicate link layer object assignment check
		if(netIdx!=j && rtl8651NetworkIntfTable[j].valid == 1 && rtl8651NetworkIntfTable[j].linkLayerType == llType && rtl8651NetworkIntfTable[j].linkLayerIndex == llIndex) {
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return TBLDRV_ELLIDXEXIST;//The index has already configured this link layer index
		}
	rtl8651NetworkIntfTable[netIdx].linkLayerType = llType;
	rtl8651NetworkIntfTable[netIdx].linkLayerIndex = llIndex;
	/* Disable VLAN routing by default */
	rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].ipAttached = 0;
	_rtl8651_updateAsicVlan(vid);//Update new VLAN information into ASIC based on driver table information
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

/*
Forbide by
1. Network attached on IP interface
2. NAT control table reference to IP interface
*/
int32 rtl8651_removeNetworkIntfLinkLayerType(int8 * ifName) {
	uint32 i;
	uint16 vid;

	RTL8651_LOCK_RESOURCE();//Lock resource
	for(i=0; i< rtl8651_tblDrvPara.networkIntfTableSize; i++)
		if (rtl8651NetworkIntfTable[i].valid == 1 && 
			strncmp(rtl8651NetworkIntfTable[i].ifName, ifName, RTL8651_IPINTF_NAME_LEN) == 0)
			break;
	if (i == rtl8651_tblDrvPara.networkIntfTableSize) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return TBLDRV_EINVNETIFNAME;
	}
	/* Not Allow non-specified link layer type */
	if(rtl8651NetworkIntfTable[i].linkLayerType == 0) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return TBLDRV_ENOLLSPECIFIED;//No link layer object attached
	}
	/* Not allow network interface referenced by IP interface */
	if(rtl8651NetworkIntfTable[i].IpHead) {
		RTL8651_UNLOCK_RESOURCE();
		return TBLDRV_ENETIFREFBYIPIF;
	}
	GET_VID_BY_NETIFIDX(vid, i);
	/* If the removed vlan is an external interface, forbid the request */
	if (rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].internal == 0) {
		RTL8651_UNLOCK_RESOURCE();
		return TBLDRV_ENETIFREFBYNAT;
	}
	/* OK!, reset the link layer type */
	rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].ipAttached = 0;
	_rtl8651_updateAsicVlan(vid);//Update new VLAN information into ASIC based on driver table information
	rtl8651NetworkIntfTable[i].linkLayerType = 0;
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

static void _rtl8651_updateGidxRegister(void) {
	uint32 i, j, idxCount, regValue;
	uint8 ipIdx[RTL8651_IPTABLE_SIZE];

	for(i=0, idxCount=0; i<RTL8651_IPTABLE_SIZE; i++)
		if(rtl8651IpTableAsicArrangementTable[i].valid == 1 && rtl8651IpTableAsicArrangementTable[i].localPublic == 0 && rtl8651IpTableAsicArrangementTable[i].nat == 0) {//NAPT external IP
			ipIdx[idxCount] = i;
			idxCount++;
		}
	assert(idxCount <= RTL8651_IPTABLE_SIZE);
	assert(idxCount == naptIpCount);
	regValue = 0;
	if(idxCount>0) {
		for(i=0, j=0; i<8; i++) {
			regValue |= (ipIdx[j]<<(3*i));
			j = (j+1)%idxCount;
		}
	}
	rtl8651_setAsicGidxRegister(regValue);
}


//as long as there is a NAPT address config to ExtIPTable, forbid new IP interfaces being added.
int32 rtl8651_addIpIntf(int8 * ifName, ipaddr_t ipAddr, ipaddr_t ipMask) {
	uint32 netIdx, j, k, bestStartPos, bestSize, curStartPos, curSize, netSize;
	rtl8651_tblDrv_ipIntfEntry_t * newNetIntf, * ipIntfPtr;
	uint32 routePos, rtIdx, ipIntfCount, freeIpCount;
	uint16 vid;
	int8 extIf = FALSE;

	if(ipMask == 0)
		return TBLDRV_EINVVAL;//Forbid to add default route
	
	RTL8651_LOCK_RESOURCE();
	for(netIdx=0; netIdx<rtl8651_tblDrvPara.networkIntfTableSize; netIdx++)
		if (rtl8651NetworkIntfTable[netIdx].valid == 1 &&
			strncmp(rtl8651NetworkIntfTable[netIdx].ifName, ifName, RTL8651_IPINTF_NAME_LEN) == 0)
			break;
	/* Check to see if the specified network interface exists */
	if (netIdx == rtl8651_tblDrvPara.networkIntfTableSize) {
		RTL8651_UNLOCK_RESOURCE();
		return TBLDRV_EINVNETIFNAME;
	}
	/* Forbid unknown-link layer Network IP interface assignment */
	if(rtl8651NetworkIntfTable[netIdx].linkLayerType == 0) {
		RTL8651_UNLOCK_RESOURCE();
		return TBLDRV_ENOLLSPECIFIED;
	}
	/* Check to see if still have enough buffer */
	if(rtl8651FreeIpIntfIpAddrList == NULL) {
		RTL8651_UNLOCK_RESOURCE();
		return TBLDRV_ENOFREEBUF;
	}
	/* Calculate usable ASIC IP entry */
	freeIpCount = RTL8651_IPTABLE_SIZE - naptIpCount - natIpCount - localServerCount;
	/* Check whether the interface is NAT external IP interface */
	GET_VID_BY_NETIFIDX(vid, netIdx);
	if (rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].internal == 0) {
		if(freeIpCount == 0) {
			/* 
			 * Added new network interface is NAT external IP interface. 
			 * However, no more free IP table entry is available.
			 */
			RTL8651_UNLOCK_RESOURCE();
			return TBLDRV_ENOFREEBUF;
		}
		extIf = TRUE;
	}

	/*
	 * NOTE:
	 *	If the NAPT is operating and another external IP address/IP interface
	 *	is added by protocol stack, how we handle such circumstances is as follows:
	 *  	. We triverse all NAPT table (both TCP/UDP and ICMP table) to see 
	 *		  if exist any NAPT entry from protocol stack, if found we forbid adding
	 *		  the new IP address/IP interface; otherwise, flush whole NAPT 
	 *		  table(both TCP/UDP and ICMP table). This is because when this API is 
	 *		  used, the protocol stack must make sure he has empty NAPT table.
	 */
	if (extIf == TRUE) {
		rtl8651_tblDrv_naptTcpUdpFlowEntry_t *trackNaptTcpUdp;
		rtl8651_tblDrv_naptIcmpFlowEntry_t *trackNaptIcmp;

		for(k=0; k<RTL8651_TCPUDPTBL_SIZE; k++) {
			trackNaptTcpUdp = rtl8651NaptTcpUdpFlowTable[k];
			while(trackNaptTcpUdp) {
				if (trackNaptTcpUdp->protoDel == 0) {
					RTL8651_UNLOCK_RESOURCE();
					return TBLDRV_ENAPTSTART;
				}	
				trackNaptTcpUdp = trackNaptTcpUdp->next;
			}
		}
		for(k=0; k<RTL8651_ICMPTBL_SIZE; k++) {
			trackNaptIcmp = rtl8651NaptIcmpFlowTable[k];
			while(trackNaptIcmp) {
				if (trackNaptIcmp->protoDel == 0) {
					RTL8651_UNLOCK_RESOURCE();
					return TBLDRV_ENAPTSTART;
				}
				trackNaptIcmp = trackNaptIcmp->next;
			}
		}
	}

	/* Check to see if this is an exited IP interface(subnet) */
	for(ipIntfPtr=rtl8651NetworkIntfTable[netIdx].IpHead; ipIntfPtr; ipIntfPtr=ipIntfPtr->nextIp)
		if((ipIntfPtr->ipAddr->ipAddr & ipIntfPtr->ipMask) == (ipAddr & ipMask) && ipIntfPtr->ipMask == ipMask) {
			rtl8651_tblDrv_ipIntfIpAddrEntry_t *ipPtr;

			/* Check to see if duplicated IP address assignment */
	 		for(ipPtr=ipIntfPtr->ipAddr; ipPtr; ipPtr=ipPtr->next)
	 			if (ipPtr->ipAddr == ipAddr) {
					RTL8651_UNLOCK_RESOURCE();
					return TBLDRV_EIPIFEXIST; /* duplicate found */
	 			}
	 		assert(rtl8651NetworkIntfTable[netIdx].linkLayerType == RTL8651_LL_VLAN);
	 		ipPtr = rtl8651FreeIpIntfIpAddrList;
			rtl8651FreeIpIntfIpAddrList = rtl8651FreeIpIntfIpAddrList->next;
			ipPtr->next = ipIntfPtr->ipAddr;
			ipIntfPtr->ipAddr = ipPtr;
			ipPtr->ipAddr = ipAddr;
			/*
			 * Since the IP interface is an external IP interface, 
			 * put the new address into IP table.
			 */
			if(extIf == TRUE) {
				for(k=0; k<RTL8651_IPTABLE_SIZE; k++)
					if(rtl8651IpTableAsicArrangementTable[k].valid == 0)
						break;//Have free IP table slot for new address
				assert(k < RTL8651_IPTABLE_SIZE);
				rtl8651IpTableAsicArrangementTable[k].valid = 1;
				rtl8651IpTableAsicArrangementTable[k].externalIp = ipAddr;
				rtl8651IpTableAsicArrangementTable[k].internalIp = 0;
				rtl8651IpTableAsicArrangementTable[k].localPublic = 0;
				rtl8651IpTableAsicArrangementTable[k].nat = 0;//Initial to be NAPT external IP address
				naptIpCount++;
				rtl8651_setAsicExtIntIpTable(k, rtl8651IpTableAsicArrangementTable[k].externalIp, rtl8651IpTableAsicArrangementTable[k].internalIp, rtl8651IpTableAsicArrangementTable[k].localPublic == 1 ? TRUE: FALSE, rtl8651IpTableAsicArrangementTable[k].nat == 1 ? TRUE: FALSE); 
				_rtl8651_updateGidxRegister();
				/* NAPT external IP has rearranged, flush whole NAPT table */
				if(rtl8651ExistedNaptTcpUdpFlows>0 || rtl8651ExistedNaptIcmpFlows>0)
					_rtl8651_flushNaptFlow();
				if(freeIpCount == RTL8651_IPTABLE_SIZE)//Original no IP table with entry and now add one address. NAT should start to work
					rtl8651_setAsicOperationLayer(4);
			}
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return SUCCESS;			
		}
	/* Otherwise, no existed IP interface exists, Create one! */
	if(rtl8651FreeIpIntfList == NULL) {//Unable to allocate network
		RTL8651_UNLOCK_RESOURCE();
		return TBLDRV_ENOFREEBUF; 
	}
	{/* Check whether added network with arp entry in non-interface ARP list */
 		rtl8651_tblDrv_arpEntry_t * trackArpEntry;
 		
		trackArpEntry = rtl8651NonInterfaceArpList;
		while(trackArpEntry) {
			if((trackArpEntry->ipAddr & ipMask) == (ipAddr & ipMask)) {
				RTL8651_UNLOCK_RESOURCE();//Unlock resource
				return TBLDRV_EIPIFNONIFARP;
			}
			trackArpEntry = trackArpEntry->next;
		}
	}
	/* Search for available ASIC routing entry */
	for(routePos=0; routePos<RTL8651_ROUTINGTBL_SIZE-1; routePos++)
		if(rtl8651RouteAsicArrangementTable[routePos].valid == 0) //Empty routing entry found
			break;
	/* Search for available DRIVER routing entry */
	for(rtIdx=0; rtIdx<rtl8651_tblDrvPara.routingTableSize-1; rtIdx++)
		if (rtl8651RoutingTable[rtIdx].valid == 0)
			break;
	/*
	 * Because the interface route is created by IP interface creation, 
	 * no pending route is allowed. 
	 */
	if(routePos == RTL8651_ROUTINGTBL_SIZE-1 || rtIdx == rtl8651_tblDrvPara.routingTableSize-1) {//No empty routing entry found
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return TBLDRV_ENOFREEBUF;
	}
	/* Check whether routing and arp entry are enough or not */
	if(rtl8651NetworkIntfTable[netIdx].linkLayerType == RTL8651_LL_VLAN) {
		/* Calculate network size */
		for(j=0; j<32; j++)
			if(ipMask & 1<<j)	break;
		netSize = 1<<j;
		if(netSize >1) {
			/* Search for maximum free arp slots */
			bestSize = curSize = 0;
			curStartPos = 0;
			for(j = 0; j <= RTL8651_ARPARRANGE_NUM; j++) {
				if(j == RTL8651_ARPARRANGE_NUM || rtl8651ArpAsicArrangementTable[j].valid == 1) {//End of empty slot
					if(curSize > bestSize) {
						bestStartPos = curStartPos;
						bestSize = curSize;
					}
					curStartPos = j+1;
					curSize = 0;
				} else curSize++;
			}
		}
		if(netSize>1 && (bestSize<<3) < netSize) {//ARP table not enough to store new network
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return TBLDRV_EOUTOFARPSLOT;
		}
		/* Forbid 32-bits network mask secified if link layer type is RTL8651_LL_VLAN */
		if (netSize == 1) {
			RTL8651_UNLOCK_RESOURCE();
			return TBLDRV_EINVNETMASK;
		}
	}
	/* Allocate new network interface and ip address */
	newNetIntf = rtl8651FreeIpIntfList;
	rtl8651FreeIpIntfList = rtl8651FreeIpIntfList->nextIp;
	/* Add network to ip interface */
	newNetIntf->nextIp = rtl8651NetworkIntfTable[netIdx].IpHead;
	rtl8651NetworkIntfTable[netIdx].IpHead = newNetIntf;
	newNetIntf->ipAddr = rtl8651FreeIpIntfIpAddrList;
	rtl8651FreeIpIntfIpAddrList = rtl8651FreeIpIntfIpAddrList->next;
	newNetIntf->ipAddr->next = NULL;
	/* Initial network content and corresponding arp and routing entry */
	newNetIntf->ipAddr->ipAddr = ipAddr;
	newNetIntf->ipMask = ipMask;
	newNetIntf->nextArp = NULL;
	newNetIntf->localServer = NULL;
	/* Since ip interface is external IP interface, put new address into IP table */
	if(extIf == TRUE) {
		for(k=0; k<RTL8651_IPTABLE_SIZE; k++)
			if(rtl8651IpTableAsicArrangementTable[k].valid == 0)
				break;//Have free IP table slot for new address
		assert(k < RTL8651_IPTABLE_SIZE);
		rtl8651IpTableAsicArrangementTable[k].valid = 1;
		rtl8651IpTableAsicArrangementTable[k].externalIp = ipAddr;
		rtl8651IpTableAsicArrangementTable[k].internalIp = 0;
		rtl8651IpTableAsicArrangementTable[k].localPublic = 0;
		rtl8651IpTableAsicArrangementTable[k].nat = 0;//Initial to be NAPT external IP address
		naptIpCount++;
		rtl8651_setAsicExtIntIpTable(k, rtl8651IpTableAsicArrangementTable[k].externalIp, rtl8651IpTableAsicArrangementTable[k].internalIp, rtl8651IpTableAsicArrangementTable[k].localPublic == 1 ? TRUE: FALSE, rtl8651IpTableAsicArrangementTable[k].nat == 1 ? TRUE: FALSE); 
		_rtl8651_updateGidxRegister();
		/* NAPT external IP has rearranged, flush whole NAPT table */
		if(rtl8651ExistedNaptTcpUdpFlows>0 || rtl8651ExistedNaptIcmpFlows>0)
			_rtl8651_flushNaptFlow();
	}
	/* Allocate ASIC routing table */
	rtl8651RouteAsicArrangementTable[routePos].ipAddr = ipAddr & ipMask;
	rtl8651RouteAsicArrangementTable[routePos].ipMask = ipMask;
	rtl8651RouteAsicArrangementTable[routePos].valid = 1;
	/* Allocate DRIVER routing entry */
	rtl8651RoutingTable[rtIdx].ipAddr = ipAddr & ipMask;
	rtl8651RoutingTable[rtIdx].ipMask = ipMask;
	rtl8651RoutingTable[rtIdx].nextHop = 0;
	rtl8651RoutingTable[rtIdx].networkIntfIndex = netIdx;
	rtl8651RoutingTable[rtIdx].intfNet = 1;
	rtl8651RoutingTable[rtIdx].ipIntfPtr = newNetIntf;
	rtl8651RoutingTable[rtIdx].routeAllocated = 1;
	rtl8651RoutingTable[rtIdx].routingTablePos = routePos;
	rtl8651RoutingTable[rtIdx].macPending = 0;
	rtl8651RoutingTable[rtIdx].valid = 1;
	GET_VID_BY_NETIFIDX(rtl8651RoutingTable[rtIdx].vid, netIdx);
	newNetIntf->routeAllocated = 1;
	newNetIntf->routingTablePos = routePos;

	if (rtl8651NetworkIntfTable[netIdx].linkLayerType == RTL8651_LL_PPPOE) {//PPPoE, for routing table insertion reference purpose
		/* Check if PPPoE session is ready */
		for(j=0; j<RTL8651_PPPOE_NUMBER; j++)
			if (rtl8651PppoeTable[j].valid == 1 && rtl8651PppoeTable[j].initiated == 1 &&
				rtl8651PppoeTable[j].objectId == rtl8651NetworkIntfTable[netIdx].linkLayerIndex)
				break;
		if (j != RTL8651_PPPOE_NUMBER) {
			rtl8651RouteAsicArrangementTable[routePos].type = 2;
			rtl8651_setAsicRouting(routePos, ipAddr & ipMask, ipMask, 0, rtl8651_vlanTableIndex(rtl8651RoutingTable[rtIdx].vid), 0, 0, rtl8651_filterDbIndex(&rtl8651PppoeTable[j].macInfo->macAddr), rtl8651PppoeTable[j].macInfo->asicPos, j);
			rtl8651RoutingTable[rtIdx].pppoePtr = &rtl8651PppoeTable[j];
			rtl8651RoutingTable[rtIdx].pppoePending = 0;
		} else  {
			rtl8651RouteAsicArrangementTable[routePos].type = 3;
			rtl8651_setAsicRouting(routePos, ipAddr & ipMask, ipMask, 4, rtl8651_vlanTableIndex(rtl8651RoutingTable[rtIdx].vid), 0, 0, 0, 0, 0);
			rtl8651RoutingTable[rtIdx].pppoePtr = NULL;
			rtl8651RoutingTable[rtIdx].pppoePending = 1;
		}
		newNetIntf->arpAllocated = 0;
		newNetIntf->networkType = 3;
		rtl8651RoutingTable[rtIdx].nextHopType = RTL8651_LL_PPPOE;
	}
	else if(rtl8651NetworkIntfTable[netIdx].linkLayerType == RTL8651_LL_VLAN) {//Allocate ARP entry only when subnet is not single host
		rtl8651RouteAsicArrangementTable[routePos].type = 1;
		/* Allocate ARP entry */
		newNetIntf->arpAllocated = 1;
		newNetIntf->arpStartPos = bestStartPos;
		newNetIntf->arpEndPos = bestStartPos + (netSize>>3) - ((netSize&0x7)==0? 1: 0);//ASIC using 8 entry per slot, since the endPosition is included, must -1 to avoid starting poisition collision
		/* Fill the occupied arp entry with current network information */
		for(j=newNetIntf->arpStartPos; j<=newNetIntf->arpEndPos; j++) {
			rtl8651ArpAsicArrangementTable[j].netAddr = ipAddr & ipMask;
			rtl8651ArpAsicArrangementTable[j].netMask = ipMask;
			rtl8651ArpAsicArrangementTable[j].valid = 1;
		}
		newNetIntf->networkType = 1;
		rtl8651_setAsicRouting(routePos, ipAddr & ipMask, ipMask, 2, rtl8651_vlanTableIndex(rtl8651NetworkIntfTable[netIdx].linkLayerIndex), newNetIntf->arpStartPos, newNetIntf->arpEndPos, 0, 0, 0);
		rtl8651RoutingTable[rtIdx].arpPtr = NULL;
		rtl8651RoutingTable[rtIdx].arpPending = 0;
		rtl8651RoutingTable[rtIdx].nextHopType = RTL8651_LL_VLAN;
	} else assert(0);//linkLayerType unexpected

	/* Update operation layer */
	if (freeIpCount == RTL8651_IPTABLE_SIZE) {
		if (extIf == TRUE) 
			rtl8651_setAsicOperationLayer(4);
		else {
			for(j=0, ipIntfCount = 0; j<rtl8651_tblDrvPara.networkIntfTableSize; j++)
				if(rtl8651NetworkIntfTable[j].valid == 1) {
					rtl8651_tblDrv_ipIntfEntry_t * ipIntfPtr;
					ipIntfPtr = rtl8651NetworkIntfTable[j].IpHead;
					while(ipIntfPtr) {
						ipIntfCount++;
						ipIntfPtr = ipIntfPtr->nextIp;
					}
				}
			if(ipIntfCount == 1) //Add ip interface success and have one ip interface only means change from not IP interface to IP interface exist
				rtl8651_setAsicOperationLayer(3);
		}
	} 
	/* Update rouitng enable field in Vlan Table */				
	if (rtl8651VlanTable[rtl8651_vlanTableIndex(rtl8651RoutingTable[rtIdx].vid)].ipAttached == 0) {
		rtl8651VlanTable[rtl8651_vlanTableIndex(rtl8651RoutingTable[rtIdx].vid)].ipAttached = 1;
		_rtl8651_updateAsicVlan(rtl8651RoutingTable[rtIdx].vid);
	}			
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

rtl8651_tblDrv_ipIntfEntry_t * _rtl8651_getIpIntfEntryByAddr(ipaddr_t ipAddr) {
	uint32 ifIndex;
	rtl8651_tblDrv_ipIntfEntry_t * ipIfPtr;
	
	//Request local network host, try to generate arp query
	for(ifIndex=0; ifIndex< rtl8651_tblDrvPara.networkIntfTableSize; ifIndex++)
		if(rtl8651NetworkIntfTable[ifIndex].valid == 1 && rtl8651NetworkIntfTable[ifIndex].linkLayerType == RTL8651_LL_VLAN ) {//Entry match
			ipIfPtr = rtl8651NetworkIntfTable[ifIndex].IpHead;
			while(ipIfPtr) {
				if((ipIfPtr->ipAddr->ipAddr & ipIfPtr->ipMask) == (ipAddr & ipIfPtr->ipMask)) 
					return ipIfPtr;//ipAddr belonged IP interface is found
				ipIfPtr = ipIfPtr->nextIp;
			}
		}
	return NULL;
}

int32 rtl8651_delIpIntf(int8 * ifName, ipaddr_t ipAddr, ipaddr_t ipMask) {
	uint32 netIdx, j, k, ipIntfCount, freeIpCount;
	rtl8651_tblDrv_ipIntfEntry_t * ipIntfPtr;
	uint16 vid;
	
	RTL8651_LOCK_RESOURCE();//Lock resource
	for(netIdx=0; netIdx< rtl8651_tblDrvPara.networkIntfTableSize; netIdx++)
		if( rtl8651NetworkIntfTable[netIdx].valid == 1 && 
			strncmp(rtl8651NetworkIntfTable[netIdx].ifName, ifName, RTL8651_IPINTF_NAME_LEN) == 0) 
			break;
	/* Check to see if the specified network interface exists */
	if (netIdx == rtl8651_tblDrvPara.networkIntfTableSize) {
		RTL8651_UNLOCK_RESOURCE();
		return TBLDRV_EINVNETIFNAME;
	}
	/* Find the specified IP interface */
	for(ipIntfPtr=rtl8651NetworkIntfTable[netIdx].IpHead; ipIntfPtr; ipIntfPtr=ipIntfPtr->nextIp)
		if((ipIntfPtr->ipAddr->ipAddr & ipIntfPtr->ipMask) == (ipAddr & ipMask) && ipIntfPtr->ipMask == ipMask)
			break;
	if (ipIntfPtr == (rtl8651_tblDrv_ipIntfEntry_t *)NULL) {
		RTL8651_UNLOCK_RESOURCE();
		return TBLDRV_EINVIPIF;//Specified network not exist
	}
	GET_VID_BY_NETIFIDX(vid, netIdx);
	
	/*
	 * NOTE:
	 *	If the NAPT is operating and another external IP address/IP interface
	 *	is removed by protocol stack, how we handle such circumstances is as follows:
	 *  	. We triverse all NAPT table (both TCP/UDP and ICMP table) to see 
	 *		  if exist any NAPT entry from protocol stack, if found we forbid deleting
	 *		  the new IP address/IP interface; otherwise, flush whole NAPT 
	 *		  table(both TCP/UDP and ICMP table). This is because when this API is 
	 *		  used, the protocol stack must make sure he has empty NAPT table.
	 */
	if (rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].internal == 0) {
		rtl8651_tblDrv_naptTcpUdpFlowEntry_t *trackNaptTcpUdp;
		rtl8651_tblDrv_naptIcmpFlowEntry_t *trackNaptIcmp;

		for(k=0; k<RTL8651_TCPUDPTBL_SIZE; k++) {
			trackNaptTcpUdp = rtl8651NaptTcpUdpFlowTable[k];
			while(trackNaptTcpUdp) {
				if (trackNaptTcpUdp->protoDel == 0) {
					RTL8651_UNLOCK_RESOURCE();
					return TBLDRV_ENAPTSTART;
				}	
				trackNaptTcpUdp = trackNaptTcpUdp->next;
			}
		}
		for(k=0; k<RTL8651_ICMPTBL_SIZE; k++) {
			trackNaptIcmp = rtl8651NaptIcmpFlowTable[k];
			while(trackNaptIcmp) {
				if (trackNaptIcmp->protoDel == 0) {
					RTL8651_UNLOCK_RESOURCE();
					return TBLDRV_ENAPTSTART;
				}
				trackNaptIcmp = trackNaptIcmp->next;
			}
		}
	}
	
	{ /* Otherwise, the removed IP interface is found */
		rtl8651_tblDrv_ipIntfIpAddrEntry_t * delIpPtr, *tmpIpPtr;

		assert(ipIntfPtr->ipAddr);
		/*
		 * If deletion will cause network interface to be deleted, make sure
		 * no arp entries or localSrever entries attach to it.
		 */
		if (ipIntfPtr->ipAddr->next == NULL&&(ipIntfPtr->nextArp || ipIntfPtr->localServer)) {
			RTL8651_UNLOCK_RESOURCE();
			return(TBLDRV_EIPIFREFERENCED);
		}
		/* Remove the entry */
		if(ipIntfPtr->ipAddr->ipAddr == ipAddr) {
			delIpPtr = ipIntfPtr->ipAddr;
			ipIntfPtr->ipAddr = ipIntfPtr->ipAddr->next;
		}
		else {
			tmpIpPtr = ipIntfPtr->ipAddr;
			while(tmpIpPtr->next && tmpIpPtr->next->ipAddr != ipAddr) 
				tmpIpPtr = tmpIpPtr->next;
			if(tmpIpPtr->next == NULL) {//Specified IP address not found
				RTL8651_UNLOCK_RESOURCE();//Unlock resource
				return(TBLDRV_EINVIPIF);
			}
			delIpPtr = tmpIpPtr->next;
			tmpIpPtr->next = tmpIpPtr->next->next;
		}
		/* Check whetehr the interface is external NAT IP interface */
		if (rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].internal == 0) {
			for(k=0; k<RTL8651_IPTABLE_SIZE; k++)
				if(rtl8651IpTableAsicArrangementTable[k].valid == 1 && rtl8651IpTableAsicArrangementTable[k].externalIp == delIpPtr->ipAddr) {
					assert(rtl8651IpTableAsicArrangementTable[k].localPublic == 0);
					assert(rtl8651IpTableAsicArrangementTable[k].nat == 0);
					rtl8651_delAsicExtIntIpTable(k);
					rtl8651IpTableAsicArrangementTable[k].valid = 0;
					naptIpCount--;
					_rtl8651_updateGidxRegister();
					/* NAPT external IP has rearranged, flush whole NAPT table */
					if(rtl8651ExistedNaptTcpUdpFlows>0 || rtl8651ExistedNaptIcmpFlows>0)
						_rtl8651_flushNaptFlow();
					break;
				}
			assert(k < RTL8651_IPTABLE_SIZE);
			freeIpCount = RTL8651_IPTABLE_SIZE - naptIpCount - natIpCount - localServerCount;
			if(freeIpCount == RTL8651_IPTABLE_SIZE)//Entry deleted and all IP table are freed
				rtl8651_setAsicOperationLayer(3);
		} 
		/* Release the matching network interface IP address */
		delIpPtr->next = rtl8651FreeIpIntfIpAddrList;
		rtl8651FreeIpIntfIpAddrList = delIpPtr;
		
		/* If already no IP address attached on this IP interface, remove the IP interface */
		if(ipIntfPtr->ipAddr == NULL) {
			assert(ipIntfPtr->nextArp == NULL);
			//Remove arp and route arrangement
			if(ipIntfPtr->arpAllocated) {//Remove ARP allocation
				uint32 arpPos;
				for(arpPos = ipIntfPtr->arpStartPos; arpPos <= ipIntfPtr->arpEndPos; arpPos++) 
					rtl8651ArpAsicArrangementTable[arpPos].valid = 0;
			}
			/* Remove route in Driver table */
			for(k=0, vid=0; k<rtl8651_tblDrvPara.routingTableSize-1; k++)
				if ((rtl8651RoutingTable[k].ipAddr&rtl8651RoutingTable[k].ipMask)==(ipAddr&ipMask) &&
					ipMask == rtl8651RoutingTable[k].ipMask) {
					rtl8651RoutingTable[k].valid = 0;
					vid = rtl8651RoutingTable[k].vid;
					break;
				}
			//Remove route Table
			if(ipIntfPtr->routeAllocated) {
				rtl8651RouteAsicArrangementTable[ipIntfPtr->routingTablePos].valid = 0;
				rtl8651_delAsicRouting(ipIntfPtr->routingTablePos);
			}						
			//Remove ipIntfPtr from list
			if(rtl8651NetworkIntfTable[netIdx].IpHead == ipIntfPtr)
				rtl8651NetworkIntfTable[netIdx].IpHead = ipIntfPtr->nextIp;
			else {
				rtl8651_tblDrv_ipIntfEntry_t *trackNetPtr = rtl8651NetworkIntfTable[netIdx].IpHead;
				while(trackNetPtr && trackNetPtr->nextIp != ipIntfPtr)
					trackNetPtr = trackNetPtr->nextIp;
				assert(trackNetPtr);
				trackNetPtr->nextIp = ipIntfPtr->nextIp;
			}
			ipIntfPtr->nextIp = rtl8651FreeIpIntfList;
			rtl8651FreeIpIntfList = ipIntfPtr;
			/* Update rouitng enable field in Vlan Table */		
			if (rtl8651NetworkIntfTable[netIdx].IpHead == NULL) {
				assert(vid != 0);
				rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].ipAttached = 0;
				_rtl8651_updateAsicVlan(vid);
			}
		}
		/* Update operation layer */
		for(j=0, ipIntfCount = 0; j<rtl8651_tblDrvPara.networkIntfTableSize; j++)
			if(rtl8651NetworkIntfTable[j].valid == 1) {
				rtl8651_tblDrv_ipIntfEntry_t * ipIntfPtr;
				ipIntfPtr = rtl8651NetworkIntfTable[j].IpHead;
				while(ipIntfPtr) {
					ipIntfCount++;
					ipIntfPtr = ipIntfPtr->nextIp;
				}
			}
		if(ipIntfCount == 0) //Delete ip interface success and have zero ip interface means change from last IP interface to no IP interface exist
			rtl8651_setAsicOperationLayer(2);
	}
	RTL8651_UNLOCK_RESOURCE();
	return SUCCESS;
}


static void _rtl8651_updateAsicArpHostRoute(rtl8651_tblDrv_arpEntry_t * allocArpEntry, uint32 routePos) {
	rtl8651RouteAsicArrangementTable[routePos].ipAddr = allocArpEntry->ipAddr;
	rtl8651RouteAsicArrangementTable[routePos].ipMask = 0xffffffff;
	rtl8651RouteAsicArrangementTable[routePos].type = 0;
	rtl8651RouteAsicArrangementTable[routePos].valid = 1;
	allocArpEntry->routingTablePos = routePos;
	allocArpEntry->routeAllocated= 1;
	if(allocArpEntry->macInfo->configToAsic== 1) 
		rtl8651_setAsicRouting(routePos, allocArpEntry->ipAddr, 0xffffffff, 1, rtl8651_vlanTableIndex(allocArpEntry->vid), 0, 0, rtl8651_filterDbIndex(&allocArpEntry->macInfo->macAddr), allocArpEntry->macInfo->asicPos, 0);
	else
		rtl8651_setAsicRouting(routePos, allocArpEntry->ipAddr, 0xffffffff, 4, rtl8651_vlanTableIndex(allocArpEntry->vid), 0, 0, 0, 0, 0);
}

/*
To write pending Arp and Route entry to ASIC, by chhuang
*/
static void _rtl8651_updateAsicArpAndRoute(rtl8651_tblDrv_filterDbTableEntry_t *filterDbEntry) {
	rtl8651_tblDrv_arpEntry_t * trackArpEntry;
	rtl8651_tblDrv_ipIntfEntry_t * ipIntfPtr;
	int32 i, j, rtIdx;

	if (filterDbEntry == (rtl8651_tblDrv_filterDbTableEntry_t *)NULL)
		return;
	//traverse all arp entries in network interface
	for(i=0; i<rtl8651_tblDrvPara.networkIntfTableSize; i++) {
		if (rtl8651NetworkIntfTable[i].valid == 0)
			continue;
		//if PPPoE, process PPPoE 
		if (rtl8651NetworkIntfTable[i].linkLayerType == RTL8651_LL_PPPOE) {
			for(j=0; j<RTL8651_PPPOETBL_SIZE; j++) 
				if ((rtl8651PppoeTable[j].valid==1) && (rtl8651PppoeTable[j].initiated==1) &&
					(rtl8651NetworkIntfTable[i].linkLayerIndex == rtl8651PppoeTable[j].objectId) &&
					(rtl8651PppoeTable[j].macInfo == filterDbEntry)) {
					for(rtIdx=0; rtIdx<rtl8651_tblDrvPara.routingTableSize; rtIdx++)
						if ((rtl8651RoutingTable[rtIdx].routeAllocated==1) &&
							(rtl8651RoutingTable[rtIdx].networkIntfIndex==i)) {
							assert(rtl8651RoutingTable[rtIdx].macPending == 1);
								rtl8651RoutingTable[rtIdx].macPending = 0;
						 	rtl8651_setAsicRouting(rtl8651RoutingTable[rtIdx].routingTablePos, rtl8651RoutingTable[rtIdx].ipAddr, rtl8651RoutingTable[rtIdx].ipMask, 0, rtl8651_vlanTableIndex(rtl8651RoutingTable[rtIdx].vid), 0, 0, rtl8651_filterDbIndex(&rtl8651PppoeTable[j].macInfo->macAddr), rtl8651PppoeTable[j].macInfo->asicPos, j);
						 	return;
						} 	
				}
			continue;
		}

		//otherwise, process VLAN
		assert(rtl8651NetworkIntfTable[i].linkLayerType == RTL8651_LL_VLAN);
		ipIntfPtr = rtl8651NetworkIntfTable[i].IpHead;
		while(ipIntfPtr) {
			trackArpEntry = ipIntfPtr->nextArp;
			while(trackArpEntry) {
				if (trackArpEntry->macInfo == filterDbEntry) {				
					/* If arp's IP is DMZ pending, do not write back */
					if (rtl8651VlanTable[rtl8651_vlanTableIndex(rtl8651NetworkIntfTable[i].linkLayerIndex)].DMZFlag == 1 && trackArpEntry->dmzIpPending== 1)
						return;
					//Configure to ASIC ARP table
					assert(trackArpEntry->macInfo->configToAsic == 1);
					if(ipIntfPtr->arpAllocated == 1 && 
				 	 ((ipIntfPtr->arpStartPos<<3) + (trackArpEntry->ipAddr & ~ipIntfPtr->ipMask)) <= (ipIntfPtr->arpEndPos<<3)+0x7) 
			 			rtl8651_setAsicArp((ipIntfPtr->arpStartPos<<3) + (trackArpEntry->ipAddr & ~ipIntfPtr->ipMask), rtl8651_filterDbIndex(&trackArpEntry->macInfo->macAddr), trackArpEntry->macInfo->asicPos);
					//Sync Routing Table					
					for(j=0; j<rtl8651_tblDrvPara.routingTableSize&&rtl8651VlanTable[rtl8651_vlanTableIndex(rtl8651NetworkIntfTable[i].linkLayerIndex)].DMZFlag == 0;j++) {
						if(rtl8651RoutingTable[j].valid == 1 && rtl8651RoutingTable[j].routeAllocated == 1 && //ASIC table allocated
							rtl8651NetworkIntfTable[rtl8651RoutingTable[j].networkIntfIndex].linkLayerType == RTL8651_LL_VLAN &&  //VLAN as interface link layer
							rtl8651RoutingTable[j].macPending == 1 && rtl8651RoutingTable[j].nextHop == trackArpEntry->ipAddr) {
							rtl8651RoutingTable[j].macPending = 0;
							assert(rtl8651RoutingTable[j].arpPending == 0);
							rtl8651RouteAsicArrangementTable[rtl8651RoutingTable[j].routingTablePos].type = 0;
							rtl8651_setAsicRouting(rtl8651RoutingTable[j].routingTablePos, rtl8651RoutingTable[j].ipAddr, rtl8651RoutingTable[j].ipMask, 1, rtl8651_vlanTableIndex(rtl8651RoutingTable[j].vid), 0, 0, rtl8651_filterDbIndex(&trackArpEntry->macInfo->macAddr), trackArpEntry->macInfo->asicPos, 0);
						}
					}
					for(j=0; j<rtl8651_tblDrvPara.routingTableSize&&rtl8651VlanTable[rtl8651_vlanTableIndex(rtl8651NetworkIntfTable[i].linkLayerIndex)].DMZFlag == 1;j++) {
						if(rtl8651RoutingTable[j].valid == 1 && rtl8651RoutingTable[j].routeAllocated == 1 && //ASIC table allocated
							rtl8651NetworkIntfTable[rtl8651RoutingTable[j].networkIntfIndex].linkLayerType == RTL8651_LL_VLAN &&  //VLAN as interface link layer
							rtl8651RoutingTable[j].nextHop == trackArpEntry->ipAddr) {
							rtl8651RoutingTable[j].macPending = 0;
							rtl8651RoutingTable[j].arpPending = 0;
							rtl8651RouteAsicArrangementTable[rtl8651RoutingTable[j].routingTablePos].type = 0;
							rtl8651_setAsicRouting(rtl8651RoutingTable[j].routingTablePos, rtl8651RoutingTable[j].ipAddr, rtl8651RoutingTable[j].ipMask, 1, rtl8651_vlanTableIndex(rtl8651RoutingTable[j].vid), 0, 0, rtl8651_filterDbIndex(&trackArpEntry->macInfo->macAddr), trackArpEntry->macInfo->asicPos, 0);
						}
					}
					return;
				}
				trackArpEntry = trackArpEntry->next;
			}
			ipIntfPtr = ipIntfPtr->nextIp;
		}
	}
	
	//otherwise look up nonlocal arp list
	trackArpEntry = rtl8651NonInterfaceArpList;
	while(trackArpEntry) {
		if ((trackArpEntry->macInfo == filterDbEntry) && (trackArpEntry->routeAllocated==1)) {
			assert(trackArpEntry->macInfo->configToAsic == 1);
			assert(trackArpEntry->allocType == 2);
			rtl8651_setAsicRouting(trackArpEntry->routingTablePos, trackArpEntry->ipAddr, 0xffffffff, 1, rtl8651_vlanTableIndex(trackArpEntry->vid), 0, 0, rtl8651_filterDbIndex(&trackArpEntry->macInfo->macAddr), trackArpEntry->macInfo->asicPos, 0);
			return;
		}
		trackArpEntry = trackArpEntry->next;
	}
}


static void _rtl8651_writeBackDmzArpPending() {
	rtl8651_tblDrv_ipIntfEntry_t *ipIntfPtr;
	rtl8651_tblDrv_arpEntry_t *tmpArp;
	uint32 ipIdx, netIdx, i;

	for(ipIdx=0; ipIdx < RTL8651_IPTABLE_SIZE; ipIdx++)
		if (rtl8651IpTableAsicArrangementTable[ipIdx].valid == 0)
			break;
	
	/*
	 * Go though each network interface's arp entries and try to add Local Server to
	 * ASIC IP Table if the network is DMZ external interface. Only the following cases
	 * will be picked up to added to ASIC IP Table:
	 *		(1) If the arp entry is not pending (means: be written into ASIC) and is 
	 * 			referenced by ASIC routing entry.
	 *		(2) The arp entry is not pending and is not referenced by routing table
	 *
	 *		 PS: DMZ can not in the PPPoE WAN port
	 */
	for(i=0; i<rtl8651_tblDrvPara.routingTableSize && ipIdx<RTL8651_IPTABLE_SIZE; i++)
		if (rtl8651RoutingTable[i].routeAllocated == 1 && rtl8651RoutingTable[i].arpPending == 1)
			for(netIdx=0; netIdx<rtl8651_tblDrvPara.networkIntfTableSize; netIdx++) {
				/* filter out improper network interface */
				if (rtl8651NetworkIntfTable[netIdx].valid == 0 ||
					rtl8651NetworkIntfTable[netIdx].linkLayerType != RTL8651_LL_VLAN)
					continue;
				if (rtl8651VlanTable[rtl8651_vlanTableIndex(rtl8651NetworkIntfTable[netIdx].linkLayerIndex)].DMZFlag == 0)
					continue;
				if (rtl8651RoutingTable[i].arpPtr != NULL &&
					rtl8651RoutingTable[i].arpPtr->dmzIpPending== 1 &&
					rtl8651RoutingTable[i].arpPtr->macInfo->configToAsic == 1) {
					//assert(rtl8651RoutingTable[i].arpPending == 1);
					assert(rtl8651IpTableAsicArrangementTable[ipIdx].valid == 0);
					rtl8651IpTableAsicArrangementTable[ipIdx].externalIp = rtl8651RoutingTable[i].arpPtr->ipAddr;
					rtl8651IpTableAsicArrangementTable[ipIdx].internalIp = rtl8651RoutingTable[i].arpPtr->ipAddr;
					rtl8651IpTableAsicArrangementTable[ipIdx].localPublic = 1;
					rtl8651IpTableAsicArrangementTable[ipIdx].nat = 0;
					rtl8651IpTableAsicArrangementTable[ipIdx].valid = 1;
					rtl8651RoutingTable[i].arpPtr->dmzIpPending = 0; 
					rtl8651RoutingTable[i].arpPending = 0;
					localServerCount++;
					RTL8651_SET_ASIC_IP_TABLE(ipIdx);
					_rtl8651_updateGidxRegister();
					_rtl8651_updateAsicArpAndRoute(rtl8651RoutingTable[i].arpPtr->macInfo);
					while(rtl8651IpTableAsicArrangementTable[ipIdx].valid == 1 &&
						  ipIdx < RTL8651_IPTABLE_SIZE)
						ipIdx++;
				}
			}


	for(netIdx=0; netIdx<rtl8651_tblDrvPara.networkIntfTableSize; netIdx++) {
		/* filter out improper network interface */
		if (rtl8651NetworkIntfTable[netIdx].valid == 0 ||
			rtl8651NetworkIntfTable[netIdx].linkLayerType != RTL8651_LL_VLAN)
			continue;
		if (rtl8651VlanTable[rtl8651_vlanTableIndex(rtl8651NetworkIntfTable[netIdx].linkLayerIndex)].DMZFlag == 0)
			continue;
		ipIntfPtr = rtl8651NetworkIntfTable[netIdx].IpHead;
		while (ipIntfPtr) {
			tmpArp = ipIntfPtr->nextArp;
			while (tmpArp && ipIdx < RTL8651_IPTABLE_SIZE) {
				if (tmpArp->dmzIpPending== 1 && tmpArp->macInfo->configToAsic == 1) {
					assert(rtl8651IpTableAsicArrangementTable[ipIdx].valid == 0);
					rtl8651IpTableAsicArrangementTable[ipIdx].externalIp = tmpArp->ipAddr;
					rtl8651IpTableAsicArrangementTable[ipIdx].internalIp = tmpArp->ipAddr;
					rtl8651IpTableAsicArrangementTable[ipIdx].localPublic = 1;
					rtl8651IpTableAsicArrangementTable[ipIdx].nat = 0;
					rtl8651IpTableAsicArrangementTable[ipIdx].valid = 1;
					localServerCount++;			 
					RTL8651_SET_ASIC_IP_TABLE(ipIdx);
					_rtl8651_updateGidxRegister();
					tmpArp->dmzIpPending = 0;
					_rtl8651_updateAsicArpAndRoute(tmpArp->macInfo); 
					while(rtl8651IpTableAsicArrangementTable[ipIdx].valid == 1 &&
						  ipIdx < RTL8651_IPTABLE_SIZE)
						ipIdx++;
				}
				tmpArp = tmpArp->next;
			} 
			ipIntfPtr = ipIntfPtr->nextIp;
		}
	}
	if (naptIpCount + natIpCount + localServerCount > 0)
		rtl8651_setAsicOperationLayer(4);
}




/* 
if port is configured to be 0xffffffff, this means local broadcast or port unknown.
*/
int8 _rtl8651_addLocalArpSpecifyNetworkInterface(int8 fromDrv, ipaddr_t ipAddr, ether_addr_t * macAddr, rtl8651_tblDrv_networkIntfTable_t * netIntfPtr, uint32 port) {
	rtl8651_tblDrv_ipIntfEntry_t *ipIntfPtr;
	rtl8651_tblDrv_ipIntfIpAddrEntry_t *localServer;
	rtl8651_tblDrv_arpEntry_t * allocArpEntry, *trackArpEntry;
	uint32 i;

	if(rtl8651FreeArpList == (rtl8651_tblDrv_arpEntry_t *)NULL)
		return FAILED;//No free ARP entry

	ipIntfPtr = netIntfPtr->IpHead;
	while(ipIntfPtr) {
		if((ipAddr & ipIntfPtr->ipMask) == (ipIntfPtr->ipAddr->ipAddr & ipIntfPtr->ipMask))
			break;
		ipIntfPtr = ipIntfPtr->nextIp;
	}
	if(ipIntfPtr == NULL)
		return FAILED;//Specified ipIntfPtr does not belong to specified interface
		
	trackArpEntry = ipIntfPtr->nextArp;
	while(trackArpEntry) {
		if(trackArpEntry->ipAddr == ipAddr) {
			if(fromDrv == TRUE && trackArpEntry->fromDrv == 0)
				trackArpEntry->fromDrv = 1;
			else if(fromDrv == FALSE && trackArpEntry->fromApp == 0)
				trackArpEntry->fromApp = 1;
			else return FAILED;//Duplicated configuration
			return SUCCESS;//Entry exist, just configure flag
		}
		trackArpEntry = trackArpEntry->next;
	}
	//Start to allocate ARP entry
	if(_rtl8651_addVlanRefFilterDatabaseEntry(netIntfPtr->linkLayerIndex, macAddr, port==0xffffffff? port: 1<<port) == FAILED)
		return FAILED;//Unable to allocate layer2 entry
	allocArpEntry = rtl8651FreeArpList;
	rtl8651FreeArpList = rtl8651FreeArpList->next;
	allocArpEntry->ipAddr = ipAddr;
	allocArpEntry->vid = netIntfPtr->linkLayerIndex;
	allocArpEntry->allocType = 1;//Local ARP table
	allocArpEntry->fromDrv = fromDrv == TRUE? 1: 0;
	allocArpEntry->fromApp = fromDrv == TRUE? 0: 1;
	allocArpEntry->macInfo  = _rtl8651_getVlanFilterDatabaseEntry(netIntfPtr->linkLayerIndex, macAddr);
	//Hook allocated arp to network interface
	allocArpEntry->next = ipIntfPtr->nextArp;
	ipIntfPtr->nextArp = allocArpEntry;
	/* update next hop mac address */
	for(i=0; i<rtl8651_tblDrvPara.routingTableSize;i++)
		if (rtl8651RoutingTable[i].valid == 1 && rtl8651RoutingTable[i].nextHop == ipAddr) {
			assert(rtl8651RoutingTable[i].nextHopType == RTL8651_LL_VLAN);
			rtl8651RoutingTable[i].arpPtr = allocArpEntry;
			break;
		}
	/*
	 * Check if the ARP entry belongs to DMZ network interface. If it is 
	 * a DMZ external network interface, Try to add Local Server. If out
	 * of ASIC IP Table, pending ARP entry.
	 */
	assert(netIntfPtr->linkLayerType == RTL8651_LL_VLAN);
	if (rtl8651VlanTable[rtl8651_vlanTableIndex(netIntfPtr->linkLayerIndex)].DMZFlag == 1) {
		/*
		 * Check if the arp enty is already added into Local server
		 */
		localServer = netIntfPtr->IpHead->localServer;
		while(localServer) {
			if (localServer->ipAddr == ipAddr)
				goto outDmz; //found
			localServer = localServer->next;
		}
		//not found, try to find an empty slot and add one
		for(i=0; i<RTL8651_IPTABLE_SIZE; i++)
			if (rtl8651IpTableAsicArrangementTable[i].valid == 0)
				break;
		if (i == RTL8651_IPTABLE_SIZE) {
			/* out of ASIC IP Table, pending arp */
			allocArpEntry->dmzIpPending = 1;
			return SUCCESS;
		}

		/* add Local Server */
		rtl8651IpTableAsicArrangementTable[i].externalIp = ipAddr;
		rtl8651IpTableAsicArrangementTable[i].internalIp = ipAddr;
		rtl8651IpTableAsicArrangementTable[i].localPublic = 1;
		rtl8651IpTableAsicArrangementTable[i].nat = 0;
		rtl8651IpTableAsicArrangementTable[i].valid = 1;
		localServerCount++;		 
		RTL8651_SET_ASIC_IP_TABLE(i);
		_rtl8651_updateGidxRegister();
		//Set Asic Operation Layer to Layer4.
		//Whenever there exists an "external" network interface (to ASIC, should say "external" VLAN)
		//ASIC should be layer 4 mode.
		rtl8651_setAsicOperationLayer(4);
	}
	
outDmz:
	allocArpEntry->dmzIpPending = 0;
	
	//Try to make pending routing entry point to correct Next Hop, this need not wait for ARP table filled
	for(i=0; i<rtl8651_tblDrvPara.routingTableSize;i++)
		if(rtl8651RoutingTable[i].valid == 1 && rtl8651RoutingTable[i].routeAllocated == 1 && //ASIC table allocated
			rtl8651NetworkIntfTable[rtl8651RoutingTable[i].networkIntfIndex].linkLayerType == RTL8651_LL_VLAN && //VLAN as interface link layer
			rtl8651RoutingTable[i].arpPending == 1 && rtl8651RoutingTable[i].nextHop == ipAddr) {//Waiting for new apr entry and the inserted ARP entry is what pending routing table entry waiting for
			rtl8651RoutingTable[i].arpPending = 0;
			assert(rtl8651RoutingTable[i].macPending == 0);
			if(allocArpEntry->macInfo->configToAsic == 1) {
				rtl8651RouteAsicArrangementTable[rtl8651RoutingTable[i].routingTablePos].type = 0;
				rtl8651_setAsicRouting(rtl8651RoutingTable[i].routingTablePos, rtl8651RoutingTable[i].ipAddr, rtl8651RoutingTable[i].ipMask, 1, rtl8651_vlanTableIndex(rtl8651RoutingTable[i].vid), 0, 0, rtl8651_filterDbIndex(&allocArpEntry->macInfo->macAddr), allocArpEntry->macInfo->asicPos, 0);
			}
			else
				rtl8651RoutingTable[i].macPending = 1;
		}
		//Configure to ASIC ARP table
		if(allocArpEntry->macInfo->configToAsic == 1 && ipIntfPtr->arpAllocated == 1 && 
			((ipIntfPtr->arpStartPos<<3) + (ipAddr & ~ipIntfPtr->ipMask)) <= (ipIntfPtr->arpEndPos<<3)+0x7) {
			rtl8651_setAsicArp((ipIntfPtr->arpStartPos<<3) + (ipAddr & ~ipIntfPtr->ipMask), rtl8651_filterDbIndex(&allocArpEntry->macInfo->macAddr), allocArpEntry->macInfo->asicPos);
	}
	return SUCCESS;
}

/* 
if port is configured to be 0xffffffff, this means local broadcast or port unknown.
*/
static int32 _rtl8651_addLocalArp(int8 fromDrv, ipaddr_t ipAddr, ether_addr_t * macAddr, uint32 ifIdx, uint32 port) {
	rtl8651_tblDrv_ipIntfEntry_t *netPtr;
	rtl8651_tblDrv_ipIntfIpAddrEntry_t *localServer;
	rtl8651_tblDrv_arpEntry_t * allocArpEntry, *trackArpEntry;
	uint32 i;

	if(rtl8651FreeArpList == (rtl8651_tblDrv_arpEntry_t *)NULL)
		return TBLDRV_ENOFREEBUF;//No free ARP entry
	if(rtl8651NetworkIntfTable[ifIdx].linkLayerType != RTL8651_LL_VLAN)
		return TBLDRV_ENONBCASTNET;//Forbid non-broadcast network to add ARP entry
	assert(rtl8651NetworkIntfTable[ifIdx].linkLayerIndex > 1 || rtl8651NetworkIntfTable[ifIdx].linkLayerIndex<4095);
	/* Find the IP network interface to which the added arp belongs */
	netPtr = rtl8651NetworkIntfTable[ifIdx].IpHead;
	while(netPtr) {
		if((ipAddr & netPtr->ipMask) == (netPtr->ipAddr->ipAddr & netPtr->ipMask))
			break;
		netPtr = netPtr->nextIp;
	}
	if(netPtr == NULL)
		return TBLDRV_ENOIPIFFOUND;//Specified netPtr does not belong to specified interface

	/*
	 * The IP network interface to which the arp belongs is found:
	 * 		- The entry is added by protocol stack
	 *		  never aging out
	 *		- The entry is added by table driver
	 *		  start aging out
	 */
	trackArpEntry = netPtr->nextArp;
	while(trackArpEntry) {
		if(trackArpEntry->ipAddr == ipAddr) {
			if(fromDrv == TRUE && trackArpEntry->fromDrv == 0)
				trackArpEntry->fromDrv = 1;
			else if(fromDrv == FALSE && trackArpEntry->fromApp == 0)
				trackArpEntry->fromApp = 1;
			else return(TBLDRV_EENTRYEXIST);//Duplicated configuration
			/* Decide to start aging or not */
			if (trackArpEntry->fromApp == 0)
				trackArpEntry->age = arpAgingTime;
			return SUCCESS;//Entry exist, just configure flag
		}
		trackArpEntry = trackArpEntry->next;
	}
	//Start to allocate ARP entry
	if(_rtl8651_addVlanRefFilterDatabaseEntry(rtl8651NetworkIntfTable[ifIdx].linkLayerIndex, macAddr, port==0xffffffff? port: 1<<port) == FAILED) 
		return TBLDRV_EADDFDFAIL;//Unable to allocate layer2 entry

	assert(trackArpEntry == NULL);
	allocArpEntry = rtl8651FreeArpList;
	rtl8651FreeArpList = rtl8651FreeArpList->next;
	allocArpEntry->ipAddr = ipAddr;
	allocArpEntry->vid = rtl8651NetworkIntfTable[ifIdx].linkLayerIndex;
	allocArpEntry->allocType = 1;//Local ARP table
	allocArpEntry->fromDrv = fromDrv == TRUE? 1: 0;
	allocArpEntry->fromApp = fromDrv == TRUE? 0: 1;
	allocArpEntry->macInfo  = _rtl8651_getVlanFilterDatabaseEntry(rtl8651NetworkIntfTable[ifIdx].linkLayerIndex, macAddr);
	assert(allocArpEntry->macInfo);
	
	//Hook allocated arp to network interface
	allocArpEntry->next = netPtr->nextArp;
	netPtr->nextArp = allocArpEntry;
	
	/* update next hop mac address which routing entry references */
	for(i=0; i<rtl8651_tblDrvPara.routingTableSize;i++)
		if (rtl8651RoutingTable[i].valid == 1 && rtl8651RoutingTable[i].nextHop == ipAddr) {
			assert(rtl8651RoutingTable[i].nextHopType == RTL8651_LL_VLAN);
			rtl8651RoutingTable[i].arpPtr = allocArpEntry;
			break;
		}
	/*
	 * Check if the ARP entry belongs to DMZ network interface. If it is 
	 * a DMZ external network interface, Try to add Local Server. If out
	 * of ASIC IP Table, pending ARP entry.
	 */
	assert(rtl8651NetworkIntfTable[ifIdx].linkLayerType == RTL8651_LL_VLAN);
	if (rtl8651VlanTable[rtl8651_vlanTableIndex(rtl8651NetworkIntfTable[ifIdx].linkLayerIndex)].DMZFlag == 1) {
		/*
		 * Check if the arp enty is already added into Local server
		 */
		localServer = netPtr->localServer;
		while(localServer) {
			if (localServer->ipAddr == ipAddr)
				goto outDmz;
			localServer = localServer->next;
		}
		for(i=0; i<RTL8651_IPTABLE_SIZE; i++)
			if (rtl8651IpTableAsicArrangementTable[i].valid == 0)
				break;
		if (i == RTL8651_IPTABLE_SIZE) {
			/* out of ASIC IP Table, pending arp */
			allocArpEntry->dmzIpPending = 1;
			return SUCCESS;
		}
		/* add Local Server */
		rtl8651IpTableAsicArrangementTable[i].externalIp = ipAddr;
		rtl8651IpTableAsicArrangementTable[i].internalIp = ipAddr;
		rtl8651IpTableAsicArrangementTable[i].localPublic = 1;
		rtl8651IpTableAsicArrangementTable[i].nat = 0;
		rtl8651IpTableAsicArrangementTable[i].valid = 1;
		localServerCount++;		 
		RTL8651_SET_ASIC_IP_TABLE(i);
		_rtl8651_updateGidxRegister();
		//Set Asic Operation Layer
		rtl8651_setAsicOperationLayer(4);
	}
outDmz:
	allocArpEntry->dmzIpPending = 0;

	//Try to make pending routing entry point to correct Next Hop, this need not wait for ARP table filled
	for(i=0; i<rtl8651_tblDrvPara.routingTableSize;i++)
		if(rtl8651RoutingTable[i].valid == 1 && rtl8651RoutingTable[i].routeAllocated == 1 && //ASIC table allocated
			rtl8651NetworkIntfTable[rtl8651RoutingTable[i].networkIntfIndex].linkLayerType == RTL8651_LL_VLAN && //VLAN as interface link layer
			rtl8651RoutingTable[i].arpPending == 1 && rtl8651RoutingTable[i].nextHop == ipAddr) {//Waiting for new apr entry and the inserted ARP entry is what pending routing table entry waiting for
			rtl8651RoutingTable[i].arpPending = 0;
			assert(rtl8651RoutingTable[i].macPending == 0);
			if(allocArpEntry->macInfo->configToAsic == 1) {
				rtl8651RouteAsicArrangementTable[rtl8651RoutingTable[i].routingTablePos].type = 0;
				rtl8651_setAsicRouting(rtl8651RoutingTable[i].routingTablePos, rtl8651RoutingTable[i].ipAddr, rtl8651RoutingTable[i].ipMask, 1, rtl8651_vlanTableIndex(rtl8651RoutingTable[i].vid), 0, 0, rtl8651_filterDbIndex(&allocArpEntry->macInfo->macAddr), allocArpEntry->macInfo->asicPos, 0);
			}
			else
				rtl8651RoutingTable[i].macPending = 1;
		}		
	//Configure to ASIC ARP table
	if(allocArpEntry->macInfo->configToAsic == 1 && netPtr->arpAllocated == 1 && 
		((netPtr->arpStartPos<<3) + (ipAddr & ~netPtr->ipMask)) <= (netPtr->arpEndPos<<3)+0x7) {
			rtl8651_setAsicArp((netPtr->arpStartPos<<3) + (ipAddr & ~netPtr->ipMask), rtl8651_filterDbIndex(&allocArpEntry->macInfo->macAddr), allocArpEntry->macInfo->asicPos);
	}
	return SUCCESS;
}

static int32 _rtl8651_addNonLocalArp(ipaddr_t ipAddr, ether_addr_t * macAddr, int8 * ifName, uint32 port) {
	uint32 i, routePos;
	rtl8651_tblDrv_arpEntry_t * allocArpEntry, * trackArpEntry;

	if(rtl8651FreeArpList == (rtl8651_tblDrv_arpEntry_t *)NULL) {
		return TBLDRV_ENOFREEBUF;//No free ARP entry
	}
	for(routePos=0; routePos<RTL8651_ROUTINGTBL_SIZE; routePos++)
		if(rtl8651RouteAsicArrangementTable[routePos].valid == 0) //Empty routing entry found
			break;

	for(i=0; i< rtl8651_tblDrvPara.networkIntfTableSize; i++)
		if(rtl8651NetworkIntfTable[i].valid == 1 && strncmp(ifName, rtl8651NetworkIntfTable[i].ifName, RTL8651_IPINTF_NAME_LEN) == 0) {//Interface is specified
			if(rtl8651NetworkIntfTable[i].linkLayerType != RTL8651_LL_VLAN) {
				return TBLDRV_ENONBCASTNET;//Forbid non-broadcast network to add ARP entry
			}
		break;
	}
	if(i == rtl8651_tblDrvPara.networkIntfTableSize)
		return TBLDRV_EINVNETIFNAME;//Specified Network interface not exist
	
	assert(rtl8651NetworkIntfTable[i].linkLayerIndex > 1 || rtl8651NetworkIntfTable[i].linkLayerIndex<4095);
	trackArpEntry = rtl8651NonInterfaceArpList;
	while(trackArpEntry) {
		if(trackArpEntry->ipAddr == ipAddr)
			return TBLDRV_EENTRYEXIST;//Duplicated configuration
		trackArpEntry = trackArpEntry->next;
	}
	if(_rtl8651_addVlanRefFilterDatabaseEntry(rtl8651NetworkIntfTable[i].linkLayerIndex, macAddr, port==0xffffffff? port: 1<<port) == FAILED)	
		return TBLDRV_EADDFDFAIL;
	
	allocArpEntry = rtl8651FreeArpList;
	rtl8651FreeArpList = rtl8651FreeArpList->next;
	allocArpEntry->ipAddr = ipAddr;
	allocArpEntry->vid = rtl8651NetworkIntfTable[i].linkLayerIndex;
	allocArpEntry->macInfo  = _rtl8651_getVlanFilterDatabaseEntry(rtl8651NetworkIntfTable[i].linkLayerIndex, macAddr);
	allocArpEntry->next = rtl8651NonInterfaceArpList;
	rtl8651NonInterfaceArpList = allocArpEntry;
	allocArpEntry->allocType = 2;
	allocArpEntry->fromApp = 1;
	allocArpEntry->fromDrv = 0;
	//Allocate routing table
	if(routePos < RTL8651_ROUTINGTBL_SIZE-1) 
		_rtl8651_updateAsicArpHostRoute(allocArpEntry, routePos);
	else {
		allocArpEntry->routeAllocated= 0;
		_rtl8651_lockDefaultRoute();
	}
	return SUCCESS;
}


int32 rtl8651_setArpAgingTime(uint16 agTime) {

	RTL8651_LOCK_RESOURCE();//Lock resource
	arpAgingTime = agTime;
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}


int32 rtl8651_addArp(ipaddr_t ipAddr, ether_addr_t * macAddr, int8 * ifName, uint32 port) {
	rtl8651_tblDrv_ipIntfIpAddrEntry_t *tmpIpAddr;
	rtl8651_tblDrv_ipIntfEntry_t *netPtr;
	uint32 i;
	int32 result=TBLDRV_ENOIPIFFOUND;

	RTL8651_LOCK_RESOURCE();//Lock resource
	
	//Search for interface database. If this arp entry is interface local host, hook to interface.
	for(i=0; i< rtl8651_tblDrvPara.networkIntfTableSize; i++)
		if(rtl8651NetworkIntfTable[i].valid == 1 && strncmp(ifName, rtl8651NetworkIntfTable[i].ifName, RTL8651_IPINTF_NAME_LEN) == 0) {//Interface is specified
			/*
			 * Currently, We forbid adding ARP entry for interface gateway.
			 * Because, the interface gateway already has it own mac address, if 
			 * permit it, it will cause confusion.
			 */
			netPtr = rtl8651NetworkIntfTable[i].IpHead;
			while(netPtr) {
				tmpIpAddr = netPtr->ipAddr;
				if (tmpIpAddr->ipAddr == ipAddr) {
					RTL8651_UNLOCK_RESOURCE();//Unlock resource
					return FAILED;
				}
				netPtr = netPtr->nextIp;
			}		
			result = _rtl8651_addLocalArp(FALSE, ipAddr, macAddr, i, port);
			if(result == SUCCESS) {
				RTL8651_UNLOCK_RESOURCE();//Unlock resource
				return SUCCESS;
			}
		}
	//If this arp entry does not belong to interface local host, hook to rtl8651NonInterfaceArpList
	if (result == TBLDRV_ENOIPIFFOUND)
		result = _rtl8651_addNonLocalArp(ipAddr, macAddr, ifName, port);
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return result;
}

//delete ARP entry for IP addr 'ipAddr' of network interface with index 'ifIdx'
//find IP interface of 'ipAddr'. On the IP itnerface located, search in its ARP list for 'ipAddr'
static int32 _rtl8651_delLocalArp(int8 fromDrv, ipaddr_t ipAddr, uint32 ifIdx) {
	rtl8651_tblDrv_ipIntfEntry_t * netPtr;
	rtl8651_tblDrv_ipIntfIpAddrEntry_t *localServer;
	rtl8651_tblDrv_arpEntry_t * delArpEntry, * trackArpEntry;
	uint32 i;

	if(rtl8651NetworkIntfTable[ifIdx].linkLayerType != RTL8651_LL_VLAN) {
		return TBLDRV_ENONBCASTNET;//Forbid non-broadcast network to del ARP entry
	}
	assert(rtl8651NetworkIntfTable[ifIdx].linkLayerIndex > 1 || rtl8651NetworkIntfTable[ifIdx].linkLayerIndex<4095);

	//From the first IP interface of this network interface, find the subnet which 'ipAddr' belongs to.
	netPtr = rtl8651NetworkIntfTable[ifIdx].IpHead;
	while(netPtr) {
		if((ipAddr & netPtr->ipMask) == (netPtr->ipAddr->ipAddr & netPtr->ipMask))
			break;
		netPtr = netPtr->nextIp;
	}
	if(netPtr == NULL)
		return TBLDRV_ENOIPIFFOUND;//Specified netPtr does not belong to specified interface

	//IP interface found. locate the ARP entry in ARP linked list
	trackArpEntry = netPtr->nextArp;
	while(trackArpEntry) {
		if(trackArpEntry->ipAddr == ipAddr) {//Entry match
			if(fromDrv == TRUE && trackArpEntry->fromDrv == 1) {
				trackArpEntry->fromDrv = 0;
				return SUCCESS;
			} else if(fromDrv == FALSE && trackArpEntry->fromApp == 1) {
				trackArpEntry->fromApp = 0;
				trackArpEntry->age = arpAgingTime;
				return SUCCESS;
				//cfliu: In current implementation, arp entries deletion from application  is 
				//deferred until aged out. Shall we remove the entry immediately the entry that application 
				//wants to delete is indeed unused, rather than defer ALL such operations?
			}
			break;//Entry found
		}
		trackArpEntry = trackArpEntry->next;
	}

	if(trackArpEntry == NULL)
		return TBLDRV_EENTRYNOTFOUND;//Entry not exist
	assert(trackArpEntry->fromDrv == 0 && trackArpEntry->fromApp == 0);
	
	if(netPtr->nextArp->ipAddr == ipAddr) {
		//entry is the first IP. remove entry from head
		delArpEntry = netPtr->nextArp;
		netPtr->nextArp = netPtr->nextArp->next;
	}else {
		trackArpEntry = netPtr->nextArp;
		//entry is in the middle of list. 
		//find previous entry so we can link next entry with it to. 
		//cfliu: This really should be merged in last while loop
		while(trackArpEntry->next && trackArpEntry->next->ipAddr != ipAddr)
			trackArpEntry = trackArpEntry->next;
		if(trackArpEntry->next == (rtl8651_tblDrv_arpEntry_t *)NULL) {
			
			return TBLDRV_EENTRYNOTFOUND;//Network search failed
		}
		delArpEntry = trackArpEntry->next;
		trackArpEntry->next = trackArpEntry->next->next;
	}
	
	assert(delArpEntry);//delArpEntry should store the arp entry to be removed
	for(i=0; i<rtl8651_tblDrvPara.routingTableSize; i++)
		if (rtl8651RoutingTable[i].valid == 1 && rtl8651RoutingTable[i].nextHop == ipAddr) {
			assert(rtl8651RoutingTable[i].nextHopType == RTL8651_LL_VLAN);
			rtl8651RoutingTable[i].arpPtr = (void *)0;
			break;
		}

	//Is this network interface a DMZ network? If it is, some ASIC's IP table entry may be stolen, 
	assert(rtl8651NetworkIntfTable[ifIdx].linkLayerType == RTL8651_LL_VLAN);	
	if (rtl8651VlanTable[rtl8651_vlanTableIndex(rtl8651NetworkIntfTable[ifIdx].linkLayerIndex)].DMZFlag == 1) {
		if(delArpEntry->dmzIpPending == 1) //this IP not written into ASIC IP table, so just return
			return SUCCESS;
		else{
			/*IP has been written into ASIC.*/

			/*If this IP was configured as a local server via rtl8651_addLocalServer(), continue to check 
			other ASIC tables*/
			localServer = netPtr->localServer; //start from the list of local servers on this IP interface.
			while(localServer) {
				if (localServer->ipAddr == ipAddr)
					goto outDmz;//found a match. skip dmz check
				localServer = localServer->next;
			}
			assert(!localServer);
			//no such local server in driver 


			for(i=0; i<RTL8651_IPTABLE_SIZE; i++) {
				if (rtl8651IpTableAsicArrangementTable[i].valid == 1 && 
					rtl8651IpTableAsicArrangementTable[i].externalIp == delArpEntry->ipAddr) {
					rtl8651_delAsicExtIntIpTable(i); 
					rtl8651IpTableAsicArrangementTable[i].valid = 0;
					localServerCount--;
					_rtl8651_updateGidxRegister();

					if (localServerCount == 0 && naptIpCount == 0 && natIpCount == 0)
						/* if lP table is empty, fallback to layer 3 mode */
						rtl8651_setAsicOperationLayer(3);
					_rtl8651_writeBackDmzArpPending();
					break;
				}
			}
			assert(i < RTL8651_IPTABLE_SIZE);
		}
	}
	/*
	 * If the corresponding network interface is DMZ external and IP configured into IP table as local public,
	 * try to remove Local Server from ASIC IP Table.
	 * If after successful removal, we should try to write DMZ pending ARP back to ASIC IP Table.
	 */

outDmz:
	
	for(i=0; i<rtl8651_tblDrvPara.routingTableSize; i++)//If deleted arp entry is referenced by routing table, put the routing entry status into arp pending state
		if(rtl8651RoutingTable[i].valid == 1 && rtl8651NetworkIntfTable[rtl8651RoutingTable[i].networkIntfIndex].linkLayerType == RTL8651_LL_VLAN && rtl8651RoutingTable[i].nextHop == delArpEntry->ipAddr && rtl8651RoutingTable[i].routeAllocated == 1) { //Nexthop is reference to the deleted arp entry
			if(rtl8651RoutingTable[i].macPending == 0) {
				assert(rtl8651RoutingTable[i].arpPending == 0);
				rtl8651RouteAsicArrangementTable[rtl8651RoutingTable[i].routingTablePos].type = 3;
				rtl8651_setAsicRouting(rtl8651RoutingTable[i].routingTablePos, rtl8651RoutingTable[i].ipAddr, rtl8651RoutingTable[i].ipMask, 4, rtl8651_vlanTableIndex(rtl8651RoutingTable[i].vid), 0, 0, 0, 0, 0);
			}
			else
				rtl8651RoutingTable[i].macPending = 0;
			rtl8651RoutingTable[i].arpPending = 1;
		}
	_rtl8651_delVlanRefFilterDatabaseEntry(delArpEntry->vid, &delArpEntry->macInfo->macAddr);
	delArpEntry->next = rtl8651FreeArpList;
	rtl8651FreeArpList = delArpEntry;
	//Process information synchronization
	assert(delArpEntry->allocType == 1);//Deleted arp must be local arp and allocation type must be 1
	if(netPtr->arpAllocated == 1 && delArpEntry->macInfo->configToAsic == 1 &&
	((netPtr->arpStartPos<<3) + (ipAddr & ~netPtr->ipMask)) <= (netPtr->arpEndPos<<3)+0x7)
		rtl8651_delAsicArp((netPtr->arpStartPos<<3) + (ipAddr & ~netPtr->ipMask));
	return SUCCESS;	
}

static int32 _rtl8651_delNonLocalArp(ipaddr_t ipAddr) {
	rtl8651_tblDrv_arpEntry_t * delArpEntry, * trackArpEntry;
	uint32 rtArgIdx, rtIdx;

	if(rtl8651NonInterfaceArpList == NULL) {
		
		return TBLDRV_EENTRYNOTFOUND;
	}
	else if(rtl8651NonInterfaceArpList->ipAddr == ipAddr) {
		delArpEntry = rtl8651NonInterfaceArpList;
		rtl8651NonInterfaceArpList = rtl8651NonInterfaceArpList->next;
	}
	else {
		trackArpEntry = rtl8651NonInterfaceArpList;
		while(trackArpEntry->next && trackArpEntry->next->ipAddr != ipAddr)
			trackArpEntry = trackArpEntry->next;
		if(trackArpEntry->next == (rtl8651_tblDrv_arpEntry_t *)NULL) {
			
			return TBLDRV_EENTRYNOTFOUND;//Not in non-interface arp list
		}
		delArpEntry = trackArpEntry->next;
		trackArpEntry->next = trackArpEntry->next->next;
	}
	assert(delArpEntry->allocType == 2);
	assert(delArpEntry->fromDrv == 0);
	assert(delArpEntry->fromApp == 1);
	delArpEntry->next = rtl8651FreeArpList;
	rtl8651FreeArpList = delArpEntry;
	_rtl8651_delVlanRefFilterDatabaseEntry(delArpEntry->vid, &delArpEntry->macInfo->macAddr); //chhuang
	if(delArpEntry->routeAllocated == 1) {
		rtArgIdx = delArpEntry->routingTablePos;
		if(pendingRoutes>0) {
			for(rtIdx=0; rtIdx<rtl8651_tblDrvPara.routingTableSize; rtIdx++)//Check routing table
				if(rtl8651RoutingTable[rtIdx].valid == 1 && rtl8651RoutingTable[rtIdx].routeAllocated == 0) 
					if(_rtl8651_updateAsicRoute(rtIdx, rtArgIdx) == SUCCESS) {
						assert(rtl8651RoutingTable[rtIdx].routeAllocated==1);
						assert(rtl8651RoutingTable[rtIdx].routingTablePos == rtArgIdx);
						_rtl8651_unlockDefaultRoute();//Due to successful update routing entry rtIdx
						return SUCCESS;
					}
			 trackArpEntry = rtl8651NonInterfaceArpList;
			 while(trackArpEntry) { 
			 	if(trackArpEntry->allocType == 2 && trackArpEntry->routeAllocated == 0) //Pending one
			 		break;
		 	trackArpEntry = trackArpEntry->next;
			 }
			assert(trackArpEntry);//Pending route exist but unable to find one
	 		_rtl8651_updateAsicArpHostRoute(trackArpEntry, rtArgIdx);
			assert(trackArpEntry->routeAllocated == 1);
			_rtl8651_unlockDefaultRoute();//Due to successful update non-interface arp
		}
		else {
			rtl8651_delAsicRouting(rtArgIdx);
			rtl8651RouteAsicArrangementTable[rtArgIdx].valid = 0;
		}
	}
	else
		_rtl8651_unlockDefaultRoute();
	return SUCCESS;
}

int32 rtl8651_delArp(ipaddr_t ipAddr) {
	int32 result;
	uint32 i;

	RTL8651_LOCK_RESOURCE();//Lock resource
	
	//Search for interface database. If this arp entry is interface local host, hook to interface.
	for(i=0; i< rtl8651_tblDrvPara.networkIntfTableSize; i++)
		if(rtl8651NetworkIntfTable[i].valid == 1) {//Interface valid, network interface is meaningful
			result = _rtl8651_delLocalArp(FALSE, ipAddr, i);
			if(result == SUCCESS) {
				RTL8651_UNLOCK_RESOURCE();//Unlock resource
				return SUCCESS;
			}
		}
	result = _rtl8651_delNonLocalArp(ipAddr);
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return result;
}


rtl8651_tblDrv_arpEntry_t * _rtl8651_getArpEntry(ipaddr_t ipAddr, int8 isRefresh) {
	uint32 i;
	rtl8651_tblDrv_arpEntry_t * trackArpEntry;
	rtl8651_tblDrv_ipIntfEntry_t * ipIntfPtr;

	//Search for interface database. If this arp entry is interface local host, hook to interface.
	for(i=0; i< rtl8651_tblDrvPara.networkIntfTableSize; i++)
		if(rtl8651NetworkIntfTable[i].valid == 1) {//Interface valid, network interface is meaningful
			ipIntfPtr = rtl8651NetworkIntfTable[i].IpHead;
			while(ipIntfPtr) {
				assert(ipIntfPtr->ipAddr);//Network interface must have at least one address
				if((ipAddr & ipIntfPtr->ipMask) == (ipIntfPtr->ipAddr->ipAddr & ipIntfPtr->ipMask)) {//Local host
					trackArpEntry = ipIntfPtr->nextArp;
					while(trackArpEntry && trackArpEntry->ipAddr != ipAddr)
						trackArpEntry = trackArpEntry->next;
					/* If the arp is referenced, update its aging time */
					if (isRefresh == TRUE && trackArpEntry)
						trackArpEntry->age = arpAgingTime;
					return trackArpEntry;
				}
				ipIntfPtr = ipIntfPtr->nextIp;
			}
		}
	return NULL;
}


static void _rtl8651_timeUpdateArp(uint32 secPassed) {
	rtl8651_tblDrv_ipIntfEntry_t * ipIntfPtr;
	rtl8651_tblDrv_arpEntry_t *arpEntryPtr, *delArpEntry;
	uint32 ifIndex, ageSec;
	int32 result;

	for(ifIndex=0; ifIndex<rtl8651_tblDrvPara.networkIntfTableSize; ifIndex++) {
		if (rtl8651NetworkIntfTable[ifIndex].valid == 0)
			continue;
		ipIntfPtr = rtl8651NetworkIntfTable[ifIndex].IpHead;
		for(; ipIntfPtr; ipIntfPtr=ipIntfPtr->nextIp) {
			arpEntryPtr=ipIntfPtr->nextArp;
			while (arpEntryPtr) {			
				/* If the arp entry is still referenced by protocol stack */
				if (arpEntryPtr->fromApp == 1) {
					arpEntryPtr = arpEntryPtr->next;
					continue;
				}
				/* If the arp is in counting state, downcount */
				if (arpEntryPtr->age > secPassed) {
					arpEntryPtr->age -= secPassed;
					arpEntryPtr = arpEntryPtr->next;
					continue;
				} else arpEntryPtr->age = 0;
				/* Time expired, Check the L2 entry to see if the age field reaches zero */
				if (arpEntryPtr->macInfo->configToAsic == 1) {
					result = rtl8651_getAsicL2Table_Patch(rtl8651_filterDbIndex(&(arpEntryPtr->macInfo->macAddr)), arpEntryPtr->macInfo->asicPos, NULL, NULL, NULL, NULL, NULL, &ageSec, NULL);
					assert(result == SUCCESS);
					if (ageSec > 0) {
						/*
						 * NOTE:
						 *	The aging time read from the ASIC is divided by 2 is becuase
						 *	of ASIC's rough timer. 
						 */
						arpEntryPtr->age = ageSec>>2;
						arpEntryPtr = arpEntryPtr->next;
#ifndef CYGWIN
						//printf("update arp aging time to %d\n", ageSec>>2);
#endif
						continue;
					}
				}
				/*
				 * Note: Function is not complete.
				 *		If the mac address is pending, currently, we remove it immediately.
				 *	In the future, each mac entry should maintain an aging mechanism by 
				 *  driver.
				 */
#ifndef CYGWIN
				printf("(arp) %d.%d.%d.%d %x:%x:%x:%x:%x:%x timeout!\n", arpEntryPtr->ipAddr>>24, 
				(arpEntryPtr->ipAddr&0x00ff0000)>>16, (arpEntryPtr->ipAddr&0x0000ff00)>>8,
				arpEntryPtr->ipAddr&0xff, arpEntryPtr->macInfo->macAddr.octet[0],
				arpEntryPtr->macInfo->macAddr.octet[1], arpEntryPtr->macInfo->macAddr.octet[2],
				arpEntryPtr->macInfo->macAddr.octet[3], arpEntryPtr->macInfo->macAddr.octet[5],
				arpEntryPtr->macInfo->macAddr.octet[5]);
#endif
				delArpEntry = arpEntryPtr;
				arpEntryPtr = arpEntryPtr->next;
				if (delArpEntry->fromApp == 0)
					_rtl8651_delLocalArp(TRUE, delArpEntry->ipAddr, ifIndex);
			}
		}
	}
}


static void _rtl8651_removeAllDrvArp(uint32 port) {
	uint32 ifIndex;
	rtl8651_tblDrv_ipIntfEntry_t * ipIntfPtr;
	rtl8651_tblDrv_arpEntry_t * arpPtr;
	
	//if(rtl8651_drvArpProcessEnable == TRUE) {//Driver add ARP automatically. Clean them when link down
		for(ifIndex=0; ifIndex< rtl8651_tblDrvPara.networkIntfTableSize; ifIndex++)
			if(rtl8651NetworkIntfTable[ifIndex].valid == 1 && rtl8651NetworkIntfTable[ifIndex].linkLayerType == RTL8651_LL_VLAN ) {//Entry match
				ipIntfPtr = rtl8651NetworkIntfTable[ifIndex].IpHead;
				//For each ARP entry of this IP interface on this network interface,
				//if there exists an ARP entry on 'port', delete it by where we store it.
				while(ipIntfPtr) {
						arpPtr = ipIntfPtr->nextArp;
						while(arpPtr) {
							if(arpPtr->fromDrv == 1 && (arpPtr->macInfo->memberPortMask & (1<<port))) {//Such arp need to be deleted
								_rtl8651_delLocalArp(TRUE, arpPtr->ipAddr, ifIndex);
								//rtl8651_fwdEngineCounter.arpDelete++;
								arpPtr = ipIntfPtr->nextArp;
							}
							else
								arpPtr = arpPtr->next;
						}
						ipIntfPtr = ipIntfPtr->nextIp; //find in nect IP interface on this network interface
				}
			}
	//}
}



int32 rtl8651_addLocalServer(ipaddr_t ipAddr) {
	rtl8651_tblDrv_ipIntfEntry_t * netPtr;
	rtl8651_tblDrv_ipIntfIpAddrEntry_t *tempIpAddr;
	uint32 netIdx, j;
	uint16 vid;

	RTL8651_LOCK_RESOURCE();
	
	for(netIdx=0; netIdx< rtl8651_tblDrvPara.networkIntfTableSize; netIdx++)
		if(rtl8651NetworkIntfTable[netIdx].valid == 1) {
			netPtr = rtl8651NetworkIntfTable[netIdx].IpHead;
			while(netPtr) {
				assert(netPtr->ipAddr);//Network interface must have at least one address
				if((ipAddr & netPtr->ipMask) == (netPtr->ipAddr->ipAddr & netPtr->ipMask))
					goto subnet_found; /* Local Server at this subnet */
				netPtr = netPtr->nextIp;
			}
		} 
	/*
	 * The IP interface to which the added Local Server belongs
	 * to is not found.
	 */
	RTL8651_UNLOCK_RESOURCE()
	
	return TBLDRV_ENOIPIFFOUND;
	
subnet_found:
	/*
	 * Check Whether the added Local Server is the gateway interfaece
	 * IP Address. If yes, reject the request!
	 */
	tempIpAddr = netPtr->ipAddr;
	while(tempIpAddr) {
		if(tempIpAddr->ipAddr == ipAddr) {
			RTL8651_UNLOCK_RESOURCE()
			 
			return TBLDRV_ELSISGWIF;
		}
		tempIpAddr = tempIpAddr->next;
	}
	/* 
	 * Check Whether the added Local Server already exists
	 */
	tempIpAddr = netPtr->localServer;
	while(tempIpAddr) {
		if(tempIpAddr->ipAddr == ipAddr) {
			RTL8651_UNLOCK_RESOURCE()
				
				return TBLDRV_ELSEXIST;
		}
		tempIpAddr = tempIpAddr->next;
	}
	/*
	 * Check if there still has enough free slot for new added one
	 */
	if(rtl8651FreeIpIntfIpAddrList == NULL) {
		RTL8651_UNLOCK_RESOURCE()
		return(TBLDRV_ENOFREEBUF);
	}
	/*
	 * Check Whether the VLAN to which the Local Server belongs is an
	 * EXTERNAL interface.
	 */
	GET_VID_BY_NETIFIDX(vid, netIdx);	
	/*
	 * If the Interface is an external interface, Try to write the Local Server
	 * to the ASIC IP Table.
	 */
	if (rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].internal == 0) { 
		for(j=0; j<RTL8651_IPTABLE_SIZE; j++) 
			if (rtl8651IpTableAsicArrangementTable[j].valid == 0)
				break;
		if (j == RTL8651_IPTABLE_SIZE) {
			/* Not enough ASIC IP Table entry */
			RTL8651_UNLOCK_RESOURCE()
			return(TBLDRV_ENOFREEBUF);
		}
		rtl8651IpTableAsicArrangementTable[j].valid = 1;
		rtl8651IpTableAsicArrangementTable[j].externalIp = ipAddr;
		rtl8651IpTableAsicArrangementTable[j].internalIp = ipAddr;
		rtl8651IpTableAsicArrangementTable[j].localPublic = 1;
		rtl8651IpTableAsicArrangementTable[j].nat = 0;
		localServerCount++;
		rtl8651_setAsicExtIntIpTable(j, rtl8651IpTableAsicArrangementTable[j].externalIp, rtl8651IpTableAsicArrangementTable[j].internalIp, rtl8651IpTableAsicArrangementTable[j].localPublic == 1 ? TRUE: FALSE, rtl8651IpTableAsicArrangementTable[j].nat == 1 ? TRUE: FALSE); 
		_rtl8651_updateGidxRegister();
		//Set Asic Operation Layer
		rtl8651_setAsicOperationLayer(4);
	}
	/*
	 * Allocate free slot for new added Local Server and insert it into
	 * internal data structure.
	 */
	tempIpAddr = rtl8651FreeIpIntfIpAddrList;
	rtl8651FreeIpIntfIpAddrList = rtl8651FreeIpIntfIpAddrList->next;
	tempIpAddr->ipAddr = ipAddr;
	tempIpAddr->next = netPtr->localServer;
	netPtr->localServer = tempIpAddr;
	RTL8651_UNLOCK_RESOURCE()
	return SUCCESS;
}



int32 rtl8651_delLocalServer(ipaddr_t ipAddr) {
	rtl8651_tblDrv_ipIntfIpAddrEntry_t *trackIpAddr, *tmpIpAddr;
	rtl8651_tblDrv_ipIntfEntry_t * netPtr;
	uint32 netIdx, j;

	RTL8651_LOCK_RESOURCE()
	
	for(netIdx=0; netIdx<rtl8651_tblDrvPara.networkIntfTableSize; netIdx++) 
		if(rtl8651NetworkIntfTable[netIdx].valid == 1) {
			netPtr = rtl8651NetworkIntfTable[netIdx].IpHead;
			while(netPtr) {
				assert(netPtr->ipAddr);//Network interface must have at least one address
				if((ipAddr & netPtr->ipMask) == (netPtr->ipAddr->ipAddr & netPtr->ipMask))
					goto subnet_found; /* Local Server at this subnet */
				netPtr = netPtr->nextIp;
			}
		}
		
subnet_found:
	if (netIdx < rtl8651_tblDrvPara.networkIntfTableSize) {
		assert(netPtr);
		tmpIpAddr = (rtl8651_tblDrv_ipIntfIpAddrEntry_t *)NULL;
		trackIpAddr = netPtr->localServer;
		while(trackIpAddr) {	
			if (trackIpAddr->ipAddr == ipAddr) { /* entry found */				
				if (tmpIpAddr == (rtl8651_tblDrv_ipIntfIpAddrEntry_t *)NULL) 
					netPtr->localServer = netPtr->localServer->next;
				else tmpIpAddr->next = trackIpAddr->next;
				trackIpAddr->next = rtl8651FreeIpIntfIpAddrList;
				rtl8651FreeIpIntfIpAddrList = trackIpAddr;
				/* find Asic position */
				for(j=0; j<RTL8651_IPTABLE_SIZE; j++)
					if (rtl8651IpTableAsicArrangementTable[j].valid == 1 && rtl8651IpTableAsicArrangementTable[j].externalIp == ipAddr)
						break;
				/*
				 * If the Local Server was written into the ASIC IP Table, remove it;
				 * otherwise, do nothing!
				 */
				if (j < RTL8651_IPTABLE_SIZE) {
					/* Remove it from ASIC IP Table */
					assert(rtl8651IpTableAsicArrangementTable[j].localPublic == 1);
					assert(rtl8651IpTableAsicArrangementTable[j].nat == 0);
					rtl8651_delAsicExtIntIpTable(j); 
					rtl8651IpTableAsicArrangementTable[j].valid = 0;
					localServerCount--;
					_rtl8651_updateGidxRegister();
					_rtl8651_writeBackDmzArpPending();
				}
				/*
				 * Set ASIC Operation Layer
				 */
				if (localServerCount == 0 && naptIpCount == 0 && natIpCount == 0)
					rtl8651_setAsicOperationLayer(3);
				RTL8651_UNLOCK_RESOURCE()
				return SUCCESS;
			}
			tmpIpAddr = trackIpAddr;
			trackIpAddr = trackIpAddr->next;
		}
	}
	RTL8651_UNLOCK_RESOURCE()
	return(TBLDRV_ELSNOTFOUND); /* Local Server not found */
}



rtl8651_tblDrv_routeTable_t * _rtl8651_getRoutingEntry(ipaddr_t dstIpAddr) {
	rtl8651_tblDrv_routeTable_t *tmpRtEntry = NULL;
	uint32 rtIdx, mask;

	for(rtIdx=0, mask=0; rtIdx<rtl8651_tblDrvPara.routingTableSize; rtIdx++) {
		if (rtl8651RoutingTable[rtIdx].valid == 1 && 
			rtl8651RoutingTable[rtIdx].ipAddr == (rtl8651RoutingTable[rtIdx].ipMask&dstIpAddr)&&
			mask <= rtl8651RoutingTable[rtIdx].ipMask) {
			mask = rtl8651RoutingTable[rtIdx].ipMask;
			tmpRtEntry = &rtl8651RoutingTable[rtIdx];
		}
	}
	return (rtl8651_tblDrv_routeTable_t *)tmpRtEntry;
}


static int32 _rtl8651_updateAsicRoute(uint32 rtIdx, uint32 rtArgIdx) {
	uint32 i;
	rtl8651_tblDrv_ipIntfEntry_t * ipIntfPtr;

	assert(rtArgIdx <= RTL8651_ROUTINGTBL_SIZE);
	/*
	 * Two Cases need to be handled:
	 *	(1) the link-layer type of next hop is VLAN 
	 *	(2) the link-layer type of next hop is PPPoE
	 */
	if (rtl8651NetworkIntfTable[rtl8651RoutingTable[rtIdx].networkIntfIndex].linkLayerType == RTL8651_LL_VLAN) {
		rtl8651_tblDrv_arpEntry_t * arpPtr;

		ipIntfPtr = rtl8651NetworkIntfTable[rtl8651RoutingTable[rtIdx].networkIntfIndex].IpHead;
		while(ipIntfPtr) {
			if((ipIntfPtr->ipAddr->ipAddr & ipIntfPtr->ipMask) == (rtl8651RoutingTable[rtIdx].nextHop & ipIntfPtr->ipMask))
				break;
			ipIntfPtr = ipIntfPtr->nextIp;
		}
		assert(ipIntfPtr);//When send update Asic route, network contains next hop should exist
		rtl8651RoutingTable[rtIdx].vid = rtl8651NetworkIntfTable[rtl8651RoutingTable[rtIdx].networkIntfIndex].linkLayerIndex;
		//arpPtr = ipIntfPtr->nextArp;
		//while(arpPtr && arpPtr->ipAddr != rtl8651RoutingTable[rtIdx].nextHop)
		//	arpPtr = arpPtr->next;
		arpPtr = rtl8651RoutingTable[rtIdx].arpPtr;
		if(rtArgIdx == RTL8651_ROUTINGTBL_SIZE)//Unable to allocate routing entry
			return SUCCESS;
		rtl8651RoutingTable[rtIdx].routeAllocated = 1;
		rtl8651RoutingTable[rtIdx].routingTablePos = rtArgIdx;
		rtl8651RouteAsicArrangementTable[rtArgIdx].ipAddr = rtl8651RoutingTable[rtIdx].ipAddr;
		rtl8651RouteAsicArrangementTable[rtArgIdx].ipMask = rtl8651RoutingTable[rtIdx].ipMask;
		rtl8651RouteAsicArrangementTable[rtArgIdx].valid = 1;
		if(arpPtr == (rtl8651_tblDrv_arpEntry_t *)NULL) {
			/*
			 * Unable to identify arp entry, hence set the route to CPU for
			 * arp resolving.
			 */
			rtl8651RoutingTable[rtIdx].arpPending = 1;
			rtl8651RouteAsicArrangementTable[rtArgIdx].type = 3;
			rtl8651_setAsicRouting(rtArgIdx, rtl8651RoutingTable[rtIdx].ipAddr, rtl8651RoutingTable[rtIdx].ipMask, 4, rtl8651_vlanTableIndex(rtl8651RoutingTable[rtIdx].vid), 0, 0, 0, 0, 0);
		}
		else {
			/*
			 * The arp entry is found, but we need to identify whether
			 * the MAC address of arp entry is written to ASIC or not. 
			 */
			assert(arpPtr->macInfo);
			assert(arpPtr->allocType == 1);
			if(arpPtr->macInfo->configToAsic == 1) {
				if (arpPtr->dmzIpPending == 1) {
					//rtl8651RoutingTable[rtIdx].arpPending = 1;
					rtl8651RoutingTable[rtIdx].macPending = 0;
					rtl8651RouteAsicArrangementTable[rtArgIdx].type = 3;
					rtl8651_setAsicRouting(rtArgIdx, rtl8651RoutingTable[rtIdx].ipAddr, rtl8651RoutingTable[rtIdx].ipMask, 4, rtl8651_vlanTableIndex(rtl8651RoutingTable[rtIdx].vid), 0, 0, 0, 0, 0);
					return SUCCESS;
				}
				rtl8651RouteAsicArrangementTable[rtArgIdx].type = 0;
				rtl8651_setAsicRouting(rtArgIdx, rtl8651RoutingTable[rtIdx].ipAddr, rtl8651RoutingTable[rtIdx].ipMask, 1, rtl8651_vlanTableIndex(rtl8651RoutingTable[rtIdx].vid), 0, 0, rtl8651_filterDbIndex(&arpPtr->macInfo->macAddr), arpPtr->macInfo->asicPos, 0);
			}
			else {
				/*
				 * Next Hop's mac address exists, but can not write to ASIC. The
				 * routing entry shall turn on the to-CPU flag for cpu processing.
				 */
				rtl8651RoutingTable[rtIdx].macPending = 1;
				rtl8651RouteAsicArrangementTable[rtArgIdx].type = 3;
				rtl8651_setAsicRouting(rtArgIdx, rtl8651RoutingTable[rtIdx].ipAddr, rtl8651RoutingTable[rtIdx].ipMask, 4, rtl8651_vlanTableIndex(rtl8651RoutingTable[rtIdx].vid), 0, 0, 0, 0, 0);
			}
		}
		return SUCCESS;
	}
	assert(rtl8651NetworkIntfTable[rtl8651RoutingTable[rtIdx].networkIntfIndex].linkLayerType == RTL8651_LL_PPPOE);
	for(i=0; i<RTL8651_PPPOE_NUMBER; i++)
		if(rtl8651PppoeTable[i].valid == 1 && rtl8651PppoeTable[i].objectId == rtl8651NetworkIntfTable[rtl8651RoutingTable[rtIdx].networkIntfIndex].linkLayerIndex)
			break;
	assert(i < RTL8651_PPPOE_NUMBER);
	rtl8651RoutingTable[rtIdx].intfNet = 0;
	rtl8651RoutingTable[rtIdx].vid = rtl8651PppoeTable[i].vid;
	//assert(rtl8651RoutingTable[rtIdx].pppoePtr);
	//rtl8651RoutingTable[rtIdx].vid = rtl8651RoutingTable[rtIdx].pppoePtr->vid;
	
	if(rtArgIdx == RTL8651_ROUTINGTBL_SIZE) //Unable to allocate routing entry
		rtl8651RoutingTable[rtIdx].routeAllocated = 0;
	else {
		assert(rtl8651RouteAsicArrangementTable[rtArgIdx].valid == 0);
		rtl8651RoutingTable[rtIdx].routeAllocated = 1;
		rtl8651RoutingTable[rtIdx].routingTablePos = rtArgIdx;
		rtl8651RouteAsicArrangementTable[rtArgIdx].ipAddr = rtl8651RoutingTable[rtIdx].ipAddr;
		rtl8651RouteAsicArrangementTable[rtArgIdx].ipMask = rtl8651RoutingTable[rtIdx].ipMask;
		rtl8651RouteAsicArrangementTable[rtArgIdx].valid = 1;
		if(rtl8651PppoeTable[i].initiated == 0) { // pppoe is not properly configured
			/* PPPoE is not properly configured, hence turn on to-CPU flag */
			rtl8651RoutingTable[rtIdx].pppoePending = 1;
			rtl8651RouteAsicArrangementTable[rtArgIdx].type = 3;
			rtl8651_setAsicRouting(rtArgIdx, rtl8651RoutingTable[rtIdx].ipAddr, rtl8651RoutingTable[rtIdx].ipMask, 4, rtl8651_vlanTableIndex(rtl8651RoutingTable[rtIdx].vid), 0, 0, 0, 0, 0);
		}
		else if(rtl8651PppoeTable[i].macInfo->configToAsic == 1) {
			/* PPPoE configured and MAC address is in the ASIC */
			rtl8651RouteAsicArrangementTable[rtArgIdx].type = 2;
			rtl8651_setAsicRouting(rtArgIdx, rtl8651RoutingTable[rtIdx].ipAddr, rtl8651RoutingTable[rtIdx].ipMask, 0, rtl8651_vlanTableIndex(rtl8651RoutingTable[rtIdx].vid), 0, 0, rtl8651_filterDbIndex(&rtl8651PppoeTable[i].macInfo->macAddr), rtl8651PppoeTable[i].macInfo->asicPos, i);
		}
		else {
			/* PPPoE configured but lake of MAC address, hence turn on to-CPU flag */
			rtl8651RoutingTable[rtIdx].macPending = 1;
			rtl8651RouteAsicArrangementTable[rtArgIdx].type = 3;
			rtl8651_setAsicRouting(rtArgIdx, rtl8651RoutingTable[rtIdx].ipAddr, rtl8651RoutingTable[rtIdx].ipMask, 4, rtl8651_vlanTableIndex(rtl8651RoutingTable[rtIdx].vid), 0, 0, 0, 0, 0);
		}
	}
	return SUCCESS;
}

static void _rtl8651_lockDefaultRoute(void) {
	if(pendingRoutes == 0) 
		rtl8651_setAsicRouting(RTL8651_ROUTINGTBL_SIZE-1, 0, 0, 4, 0, 0, 0, 0, 0, 0);//Default route configured, change ASIC defualt route to CPU
	pendingRoutes++;//No ASIC routing table entry available for this route
}

static void _rtl8651_unlockDefaultRoute(void) {
	if(pendingRoutes==1) {
		if(rtl8651RoutingTable[rtl8651_tblDrvPara.routingTableSize-1].valid == 1)
			_rtl8651_updateAsicRoute(rtl8651_tblDrvPara.routingTableSize-1, RTL8651_ROUTINGTBL_SIZE-1);
		else
			rtl8651_delAsicRouting(RTL8651_ROUTINGTBL_SIZE-1);
	}
	pendingRoutes--;
}

/*
Note:
1. Reserve the last entry for default route purpose due to ASIC treate default route differently.
*/
int32 rtl8651_addRoute(ipaddr_t ipAddr, ipaddr_t ipMask, int8 * ifName, ipaddr_t nextHop) {
	uint32 ifIdx, rtIdx, rtArgIdx, i;
	rtl8651_tblDrv_arpEntry_t *arpEntry = NULL;
	rtl8651_tblDrv_pppoeTable_t *pppoeEntry = NULL;
	rtl8651_tblDrv_ipIntfEntry_t * ipIntfPtr, * intfNetPtr;
	int8 intfRoute;
	int32 retValue;
	
	RTL8651_LOCK_RESOURCE();
	/*
	 * Check whether the specified route is interface route:
	 *	  Whenever a subnet is created (used rtl8651_addIpIntf() to add a network subnet),
	 *	  a route to this subnet will be added into the routing table automatically. 
	 *	  Therefore, when this API is used, we shall distinguish whether the specified 
	 *    route is a interface route.
	 */
	for(ifIdx=0, intfRoute = FALSE; intfRoute == FALSE && ifIdx<rtl8651_tblDrvPara.networkIntfTableSize; ifIdx++)
		if(rtl8651NetworkIntfTable[ifIdx].valid == 1) {
			ipIntfPtr = rtl8651NetworkIntfTable[ifIdx].IpHead;
			while(intfRoute == FALSE && ipIntfPtr) {
				if((ipIntfPtr->ipAddr->ipAddr & ipIntfPtr->ipMask) == (ipAddr & ipMask) && ipIntfPtr->ipMask == ipMask) {
					intfRoute = TRUE;
					intfNetPtr =ipIntfPtr;
				}
				ipIntfPtr = ipIntfPtr->nextIp;
			}
		}
	/* Find the specified network interface (next hop belongs to)*/
	for(ifIdx=0; ifIdx<rtl8651_tblDrvPara.networkIntfTableSize; ifIdx++)
		if(rtl8651NetworkIntfTable[ifIdx].valid == 1 && strncmp(rtl8651NetworkIntfTable[ifIdx].ifName, ifName, RTL8651_IPINTF_NAME_LEN) == 0) 
			break;
	if(ifIdx == rtl8651_tblDrvPara.networkIntfTableSize) {
		RTL8651_UNLOCK_RESOURCE();
		return TBLDRV_EINVNETIFNAME;//Specified interface not exist
	}
	/* check if the network itnerface of next hop is a legal network interface */
	if(rtl8651NetworkIntfTable[ifIdx].linkLayerType == 0) {
		RTL8651_UNLOCK_RESOURCE();
		return TBLDRV_ENOLLSPECIFIED;//Floating interface should not have route
	}
	
	/* Check duplicate routing entry */
	for(rtIdx=0; rtIdx<rtl8651_tblDrvPara.routingTableSize; rtIdx++) {
		if(rtl8651RoutingTable[rtIdx].valid == 1 && (rtl8651RoutingTable[rtIdx].ipAddr & rtl8651RoutingTable[rtIdx].ipMask) == (ipAddr & ipMask) && rtl8651RoutingTable[rtIdx].ipMask == ipMask) {
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return TBLDRV_ERTEXIST;//Duplicate route insertion
		}
	}
	if(rtl8651NetworkIntfTable[ifIdx].linkLayerType == RTL8651_LL_VLAN) {
		/*
		 * The Next Hop's network interface Type is VLAN, Try to make sure the
		 * next hop belongs to a IP interface. And try to find next hop's mac
		 */
		ipIntfPtr = rtl8651NetworkIntfTable[ifIdx].IpHead;
		while(ipIntfPtr) {
			if((ipIntfPtr->ipAddr->ipAddr & ipIntfPtr->ipMask) == (nextHop & ipIntfPtr->ipMask)) {
				arpEntry = ipIntfPtr->nextArp;
				while (arpEntry) {
					if (arpEntry->ipAddr == nextHop)
						/* 
						 * We can make sure the next hop belongs to an existing 
						 * network interface and we also found its MAC address.
						 */
						break;
					arpEntry = arpEntry->next;
				}
				/*
				 * We only can make sure the next hop belongs to an
				 * existing network interface, but we can not found its mac address.
				 */
				break;
			}
			ipIntfPtr = ipIntfPtr->nextIp;
		}
		if(ipIntfPtr == (rtl8651_tblDrv_ipIntfEntry_t *)0 && intfRoute == FALSE) {
			RTL8651_UNLOCK_RESOURCE();
			return TBLDRV_ENHOPNOTNETIF;//Specified next hop not belong to interface
		}
	} else {
		/*
		 * Otherwise, the next Hop's network interface Type is PPPoE, Try to find
		 * the PPPoE entry.
		 */
		assert(rtl8651NetworkIntfTable[ifIdx].linkLayerType == RTL8651_LL_PPPOE);
		for(i=0; i<RTL8651_PPPOE_NUMBER; i++)
			if(rtl8651PppoeTable[i].valid == 1 && rtl8651PppoeTable[i].objectId == rtl8651NetworkIntfTable[ifIdx].linkLayerIndex) {
				pppoeEntry = &rtl8651PppoeTable[i];
				break;
			}
	}
	
	if(ipMask != 0) {
		/* The specified routing entry is a non-default route. */
		for(rtIdx=0; rtIdx<rtl8651_tblDrvPara.routingTableSize-1; rtIdx++)
			if(rtl8651RoutingTable[rtIdx].valid == 0)//Search for empty slot
				break;
		if(rtIdx == rtl8651_tblDrvPara.routingTableSize-1) {
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return TBLDRV_ENOFREEBUF;//Unable to get empty slot for this route
		}
	}
	else {
		/* 
		 * The specified routing entry is a default-route:
		 * NOTE:
		 *	 Default route uses the last rouitng table entry and the last ASIC
		 *	 routing table entry.
		 */		 
		assert(intfRoute == FALSE);
		rtIdx = rtl8651_tblDrvPara.routingTableSize-1;
		if(rtl8651RoutingTable[rtIdx].valid == 1) {
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return TBLDRV_EDEFRTEXIST;//Default route is already configured
		}
	}
	
	//Place common routing information into routing table
	rtl8651RoutingTable[rtIdx].valid = 1;
	rtl8651RoutingTable[rtIdx].ipAddr = ipAddr & ipMask;
	rtl8651RoutingTable[rtIdx].ipMask = ipMask;
	rtl8651RoutingTable[rtIdx].networkIntfIndex = ifIdx;
	rtl8651RoutingTable[rtIdx].nextHopType = rtl8651NetworkIntfTable[ifIdx].linkLayerType;
	rtl8651RoutingTable[rtIdx].nextHop = nextHop;
	rtl8651RoutingTable[rtIdx].intfNet = 0;
	rtl8651RoutingTable[rtIdx].ipIntfPtr = NULL;
	rtl8651RoutingTable[rtIdx].pppoePtr = NULL;
	rtl8651RoutingTable[rtIdx].arpPtr = arpEntry;
	rtl8651RoutingTable[rtIdx].pppoePtr = pppoeEntry;
	rtl8651RoutingTable[rtIdx].macPending = 0;
	rtl8651RoutingTable[rtIdx].arpPending = 0;
	rtl8651RoutingTable[rtIdx].pppoePending = 0;
	rtl8651RoutingTable[rtIdx].routeAllocated = 0;

	/*
	 * Try to allocate ASIC rouitng entry:
	 *	(1) The added route is an existing interface route
	 *	(2) The added route is a normal-route
	 */
	if(intfRoute == TRUE) {
		/* The route is an existing interface route */
		rtl8651RoutingTable[rtIdx].intfNet = 1;
		rtl8651RoutingTable[rtIdx].ipIntfPtr = intfNetPtr;
		if(rtl8651NetworkIntfTable[ifIdx].linkLayerType == RTL8651_LL_VLAN)
			rtl8651RoutingTable[rtIdx].vid = rtl8651NetworkIntfTable[ifIdx].linkLayerIndex;
		else {
			for(i=0; i<RTL8651_PPPOE_NUMBER; i++)
				if(rtl8651PppoeTable[i].valid == 1 && rtl8651PppoeTable[i].objectId == rtl8651NetworkIntfTable[ifIdx].linkLayerIndex) 
					rtl8651RoutingTable[rtIdx].vid = rtl8651PppoeTable[i].vid;
		}
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return SUCCESS;
	} else {
		/* The route is a normal-route */
		if(ipMask != 0) {
			for(rtArgIdx=0; rtArgIdx<RTL8651_ROUTINGTBL_SIZE-1; rtArgIdx++)
				if(rtl8651RouteAsicArrangementTable[rtArgIdx].valid == 0)
					break;
			if(rtArgIdx == RTL8651_ROUTINGTBL_SIZE-1) {
				_rtl8651_lockDefaultRoute();
				rtArgIdx = RTL8651_ROUTINGTBL_SIZE;
			}
		} else if(pendingRoutes == 0) {
			rtArgIdx = RTL8651_ROUTINGTBL_SIZE-1;
			assert(rtl8651RouteAsicArrangementTable[rtArgIdx].valid == 0);
		} else rtArgIdx = RTL8651_ROUTINGTBL_SIZE;//Pending route occupy default route position for CPU to route packet
	}
	retValue = _rtl8651_updateAsicRoute(rtIdx, rtArgIdx);
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return retValue;
}

int32 rtl8651_delRoute(ipaddr_t ipAddr, ipaddr_t ipMask) {
	uint32 rtIdx, rtArgIdx;

	RTL8651_LOCK_RESOURCE();//Lock resource
	 
	rtArgIdx = RTL8651_ROUTINGTBL_SIZE;
	for(rtIdx=0; rtIdx<rtl8651_tblDrvPara.routingTableSize; rtIdx++)
		if(rtl8651RoutingTable[rtIdx].valid == 1 && (rtl8651RoutingTable[rtIdx].ipAddr & rtl8651RoutingTable[rtIdx].ipMask) == (ipAddr & ipMask) && rtl8651RoutingTable[rtIdx].ipMask == ipMask) {//Entry found
			//assert(rtl8651RoutingTable[rtIdx].intfNet == 1 ? rtl8651RoutingTable[rtIdx].routeAllocated == 0: 1);
			/*
			 * If the deleted routing entry is created by interface creation, forbid 
			 * the entry to be removed. This is because when an IP netwrok interface
			 * is created, its route will be added automatically.
			 */
			if (rtl8651RoutingTable[rtIdx].intfNet == 1) {
				RTL8651_UNLOCK_RESOURCE();//Unlock resource
				return TBLDRV_ERTCREATEBYIF;
			}
			/* If the route is in ASIC rouitng table, remove it. */
			if(rtl8651RoutingTable[rtIdx].routeAllocated == 1) {
				assert(rtl8651RoutingTable[rtIdx].ipAddr == rtl8651RouteAsicArrangementTable[rtl8651RoutingTable[rtIdx].routingTablePos].ipAddr);
				assert(rtl8651RoutingTable[rtIdx].ipMask == rtl8651RouteAsicArrangementTable[rtl8651RoutingTable[rtIdx].routingTablePos].ipMask);
				assert(rtl8651RouteAsicArrangementTable[rtl8651RoutingTable[rtIdx].routingTablePos].valid == 1);
				rtArgIdx = rtl8651RoutingTable[rtIdx].routingTablePos;
				rtl8651RoutingTable[rtIdx].valid = 0;
				break; //chhuang
			}
			rtl8651RoutingTable[rtIdx].valid = 0;
			RTL8651_UNLOCK_RESOURCE(); 
			return SUCCESS;//chhuang
		}
	/*
	 * After the specified route is removed from ASIC routing table,
	 * it is needed to pick up a pending routing to ASIC table.
	 */
	if(rtArgIdx == RTL8651_ROUTINGTBL_SIZE) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return TBLDRV_EENTRYNOTFOUND;//Specified route does not exist
	}
	if(pendingRoutes>0) {
		rtl8651_tblDrv_arpEntry_t * trackArpEntry;
		for(rtIdx=0; rtIdx<rtl8651_tblDrvPara.routingTableSize; rtIdx++)//Check routing table
			/* Find pending routing in the driver routing table list */
			if(rtl8651RoutingTable[rtIdx].valid == 1 && rtl8651RoutingTable[rtIdx].routeAllocated == 0) 
				if(_rtl8651_updateAsicRoute(rtIdx, rtArgIdx) == SUCCESS) {
					assert(rtl8651RoutingTable[rtIdx].routeAllocated==1);
					assert(rtl8651RoutingTable[rtIdx].routingTablePos == rtArgIdx);
					_rtl8651_unlockDefaultRoute();//Due to successful update routing entry rtIdx
					RTL8651_UNLOCK_RESOURCE();//Unlock resource
					return SUCCESS;
				}
		/* 
		 * Find pending non-local arp entry and write it to ASIC routing table.
		 * This is because when a non-local arp entry is added, it will also be added
		 * into routing table. 
		 */
		 trackArpEntry = rtl8651NonInterfaceArpList;
		 while(trackArpEntry) { 
		 	if(trackArpEntry->allocType == 2 && trackArpEntry->routeAllocated == 0) //Pending one
		 		break;
		 	trackArpEntry = trackArpEntry->next;//
		 }
		assert(trackArpEntry);//Pending route exist but unable to find one
 		_rtl8651_updateAsicArpHostRoute(trackArpEntry, rtArgIdx);
		assert(trackArpEntry->routeAllocated == 1);
		_rtl8651_unlockDefaultRoute();//Due to successful update non-interface arp
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return SUCCESS;
	}
	//No route needs to be configured into ASIC
	rtl8651_delAsicRouting(rtArgIdx);
	rtl8651RouteAsicArrangementTable[rtArgIdx].valid = 0;
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}



int32 rtl8651_addDmzNetworkInterface(int8 *ifName) {
	rtl8651_tblDrv_ipIntfIpAddrEntry_t *localServer;
	rtl8651_tblDrv_ipIntfEntry_t *netPtr;
	uint32 netIdx, freeAsicIpSlot, lsCount = 0, ipIdx = 0;
	uint16 vid;

	/* Forbid NULL interface name */
	if (ifName == NULL) {
		return(TBLDRV_EINVNETIFNAME);
	}
	RTL8651_LOCK_RESOURCE();
	/* Find network index according to the interface name */
	for(netIdx=0; netIdx<rtl8651_tblDrvPara.networkIntfTableSize; netIdx++)
		if (rtl8651NetworkIntfTable[netIdx].valid == 1 && 
			strncmp(rtl8651NetworkIntfTable[netIdx].ifName, ifName, RTL8651_IPINTF_NAME_LEN) == 0 )
			break;
	if (netIdx == rtl8651_tblDrvPara.networkIntfTableSize) {
		/* Specified interface not found */
		RTL8651_UNLOCK_RESOURCE();
		return(TBLDRV_EINVNETIFNAME);
	}
	/* Forbid PPPoE or non-assigned link type */
	if (rtl8651NetworkIntfTable[netIdx].linkLayerType != RTL8651_LL_VLAN) {
		RTL8651_UNLOCK_RESOURCE();
		return(TBLDRV_ENEEDVLANTYPE);
	}
	
	GET_VID_BY_NETIFIDX(vid, netIdx); /* Get Vlan ID */
	/* Check Whether this network interface has been registered as an external interface */
	if (rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].DMZFlag == 0 && rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].internal == 0) {
		RTL8651_UNLOCK_RESOURCE();
		return(TBLDRV_ENETEXTERNAL);
	}
	/* Check Whether this network interface has been registered by this API */
	if (rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].DMZFlag == 1) {
		assert(rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].internal == 0);
		RTL8651_UNLOCK_RESOURCE();
		return SUCCESS;
	}
	/* 
	 * Check if this network interface has ARP entries, if yes, forbid
	 * adding this network into DMZ
	 */
	if (rtl8651NetworkIntfTable[netIdx].IpHead->nextArp != NULL) {
		RTL8651_UNLOCK_RESOURCE();
		return(TBLDRV_ENETEXTERNAL);
	}
	/* Count free ASIC IP Slot */
	freeAsicIpSlot = RTL8651_IPTABLE_SIZE - (naptIpCount + natIpCount + localServerCount);
	/* Count local server in the found network interface */
	netPtr = rtl8651NetworkIntfTable[netIdx].IpHead;
	while(netPtr) {
		localServer = netPtr->localServer;
		while(localServer) {
			lsCount ++;
			localServer = localServer->next;
		}
		netPtr = netPtr->nextIp;
	}
	/* If not enough free ASIC IP slot for Local Servers, reject the request */
	if (lsCount > freeAsicIpSlot) {
		RTL8651_UNLOCK_RESOURCE();
		return(TBLDRV_ENOFREEBUF);
	}
	/* Add Local Server(s) to Asic IP Table */
	netPtr = rtl8651NetworkIntfTable[netIdx].IpHead;
	while(netPtr) {
		localServer = netPtr->localServer;
		while(localServer) {
			for(; ipIdx<RTL8651_IPTABLE_SIZE; ipIdx++)
				if (rtl8651IpTableAsicArrangementTable[ipIdx].valid == 0)
					break;
			assert(ipIdx < RTL8651_IPTABLE_SIZE);
			rtl8651IpTableAsicArrangementTable[ipIdx].valid = 1;
			rtl8651IpTableAsicArrangementTable[ipIdx].externalIp = localServer->ipAddr;
			rtl8651IpTableAsicArrangementTable[ipIdx].internalIp = localServer->ipAddr;
			rtl8651IpTableAsicArrangementTable[ipIdx].localPublic = 1;
			rtl8651IpTableAsicArrangementTable[ipIdx].nat = 0;
			RTL8651_SET_ASIC_IP_TABLE(ipIdx); 
			_rtl8651_updateGidxRegister();
			localServer = localServer->next;
			localServerCount++;
			ipIdx++;
		}
		netPtr = netPtr->nextIp;
	}	
	/* Set DMZ Flag and turn to internal interface. */
	rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].DMZFlag = 1;
	rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].internal = 0;
	_rtl8651_updateAsicVlan(vid);
	/* Check if need to set to Layer 4 Operation mode */
	if (freeAsicIpSlot == RTL8651_IPTABLE_SIZE && ipIdx > 0)
		rtl8651_setAsicOperationLayer(4);
	RTL8651_UNLOCK_RESOURCE();
	return SUCCESS;		
}


int32 rtl8651_delDmzNetworkInterface(int8 *ifName) {
	rtl8651_tblDrv_ipIntfIpAddrEntry_t *localServer;
	rtl8651_tblDrv_ipIntfEntry_t *netPtr;
	rtl8651_tblDrv_arpEntry_t *tmpArp;
	uint32 netIdx, ipIdx;
	uint16 vid;

	/* Forbid NULL interface Name */
	if (ifName == NULL) {
		return(TBLDRV_EINVNETIFNAME);
	}
	RTL8651_LOCK_RESOURCE();
	/* Find network interface index according to interface name */
	for(netIdx=0; netIdx<rtl8651_tblDrvPara.networkIntfTableSize; netIdx++)
		if (rtl8651NetworkIntfTable[netIdx].valid == 1 && 
			strncmp(rtl8651NetworkIntfTable[netIdx].ifName, ifName, RTL8651_IPINTF_NAME_LEN) == 0)
			break;
	if (netIdx == rtl8651_tblDrvPara.networkIntfTableSize) {
		/* Specified interface not found */
		RTL8651_UNLOCK_RESOURCE();
		return(TBLDRV_EINVNETIFNAME);
	}
	/* Forbid PPPoE or non-assigned link type */
	if (rtl8651NetworkIntfTable[netIdx].linkLayerType != RTL8651_LL_VLAN) {
		RTL8651_UNLOCK_RESOURCE();
		return(TBLDRV_ENEEDVLANTYPE);
	}
	
	GET_VID_BY_NETIFIDX(vid, netIdx); /* Get Vlan ID */
	/* Check if deleted interface is a DMZ interface */
	if (rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].DMZFlag == 0) {
		RTL8651_UNLOCK_RESOURCE();
		return(TBLDRV_ENOTDMZEXT);
	}
	/* 
	 * (1) If have local server(s) attached, remove it(them) from the Asic IP Table 
	 */
	netPtr = rtl8651NetworkIntfTable[netIdx].IpHead;
	while(netPtr) {
		localServer = netPtr->localServer;
		while(localServer) {
			for(ipIdx=0; ipIdx<RTL8651_IPTABLE_SIZE; ipIdx++)
				if (rtl8651IpTableAsicArrangementTable[ipIdx].valid == 1 && rtl8651IpTableAsicArrangementTable[ipIdx].externalIp == localServer->ipAddr)
					break;
			assert(ipIdx < RTL8651_IPTABLE_SIZE);
			rtl8651IpTableAsicArrangementTable[ipIdx].valid = 0;
			localServerCount --;
			RTL8651_DEL_ASIC_IP_TABLE(ipIdx);
			_rtl8651_updateGidxRegister();
			localServer = localServer->next;
		}
		netPtr = netPtr->nextIp;
	}
	/*
	 * (2) Search all arp entries in this network to remove non-Pending Local Server
	 */
	netPtr = rtl8651NetworkIntfTable[netIdx].IpHead;
	while (netPtr) {
		tmpArp = netPtr->nextArp;
		while (tmpArp) {
			for(ipIdx=0; (ipIdx<RTL8651_IPTABLE_SIZE)&&(tmpArp->dmzIpPending==0); ipIdx++) 
				if (rtl8651IpTableAsicArrangementTable[ipIdx].valid == 1 &&
					rtl8651IpTableAsicArrangementTable[ipIdx].externalIp == tmpArp->ipAddr) {
					rtl8651IpTableAsicArrangementTable[ipIdx].valid = 0;
					localServerCount--;
					RTL8651_DEL_ASIC_IP_TABLE(ipIdx);
					_rtl8651_updateGidxRegister();
				}
			/*
			 * If the arp entry is pending due to DMZ pending, write back the arp
			 * entry into ASIC Table
			 */
			if (tmpArp->dmzIpPending == 1) {
				tmpArp->dmzIpPending = 0;
				_rtl8651_updateAsicArpAndRoute(tmpArp->macInfo);
			}
			tmpArp = tmpArp->next;
		}
		netPtr = netPtr->nextIp;
	}
	
	/* Reset DMZ Flag */
	rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].DMZFlag = 0;
	rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].internal = 1;
	_rtl8651_updateAsicVlan(vid);
	_rtl8651_writeBackDmzArpPending();
	RTL8651_UNLOCK_RESOURCE();
	return SUCCESS;	
}

int32 rtl8651_addExtNetworkInterface(int8 *ifName) {
	rtl8651_tblDrv_ipIntfEntry_t * thisIpEntry;
	rtl8651_tblDrv_ipIntfIpAddrEntry_t *	thisIpAddr;
	rtl8651_tblDrv_ipIntfIpAddrEntry_t *	localServer;
	uint32 netIdx, ipCount, freeIpCount, ipTblIdx;
	uint16 vid;

	RTL8651_LOCK_RESOURCE();
	 
	/* When NAPT start to operate, forbid new NAT configuration */
	if(rtl8651ExistedNaptTcpUdpFlows>0 || rtl8651ExistedNaptIcmpFlows>0) {
		RTL8651_UNLOCK_RESOURCE();
		return(TBLDRV_ENAPTSTART); 
	}
	/* Find network interface index according to interface name */
	for(netIdx=0, ipCount=0; netIdx<rtl8651_tblDrvPara.networkIntfTableSize; netIdx++)
		if(rtl8651NetworkIntfTable[netIdx].valid == 1 && strncmp(rtl8651NetworkIntfTable[netIdx].ifName, ifName, RTL8651_IPINTF_NAME_LEN) == 0 && rtl8651NetworkIntfTable[netIdx].linkLayerType != 0) {
			/*
			 * If Specified interface is found, try to count those IPs which
			 * will occupy IP Table.
			 */
			thisIpEntry = rtl8651NetworkIntfTable[netIdx].IpHead;
			while(thisIpEntry) {
				thisIpAddr = thisIpEntry->ipAddr;
				localServer = thisIpEntry->localServer;
				assert(thisIpAddr);
				while(thisIpAddr) {
					ipCount++;
					thisIpAddr = thisIpAddr->next;
				}
				while(localServer) {
					ipCount++;
					localServer = localServer->next;
				}
				thisIpEntry = thisIpEntry->nextIp;
			}
			break; /* Entry found */
		}
	if(netIdx == rtl8651_tblDrvPara.networkIntfTableSize) {
		RTL8651_UNLOCK_RESOURCE();//UnLock resource
		return TBLDRV_EINVNETIFNAME;/* No specified Network Interface exist */
	}
	GET_VID_BY_NETIFIDX(vid, netIdx);
	/* Check to see if the specified network interface is DMZ external */
	if (rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].DMZFlag == 1) {
		RTL8651_UNLOCK_RESOURCE();//UnLock resource
		return TBLDRV_ENOTNATEXTIF;
	}
	
	/* Check to see if the specified network interface is assigned before */
	if (rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].internal == 0) {
		RTL8651_UNLOCK_RESOURCE();
		return TBLDRV_ENETEXTERNAL;
	}
			
	/* Count free ASIC IP Slot */
	freeIpCount = RTL8651_IPTABLE_SIZE - (naptIpCount + natIpCount + localServerCount);
	/* If ASIC has not enough IP Table slot for the rquest, just reject it */
	if(freeIpCount < ipCount) {
		RTL8651_UNLOCK_RESOURCE();//UnLock resource 
		return TBLDRV_EOUTOFIPTBLENTRY;//Unable to allocate enough external IP address table slots for external IP interface
	}	
	assert(rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].valid == 1);
	rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].internal = 0;
	_rtl8651_updateAsicVlan(vid);
	/* 
	 * Put all IP address (NAT/NAPT address and Local Server) over 
	 * this external IP interface into IP table 
	 */
	thisIpEntry = rtl8651NetworkIntfTable[netIdx].IpHead;
	while(thisIpEntry) {
		thisIpAddr = thisIpEntry->ipAddr;
		localServer = thisIpEntry->localServer;
		assert(thisIpAddr); ipTblIdx = 0;
		while(thisIpAddr) {
			for(; ipTblIdx<RTL8651_IPTABLE_SIZE; ipTblIdx++) 
				if(rtl8651IpTableAsicArrangementTable[ipTblIdx].valid == 0)
					break;
			assert(ipTblIdx < RTL8651_IPTABLE_SIZE);
			assert(rtl8651IpTableAsicArrangementTable[ipTblIdx].valid == 1 ? rtl8651IpTableAsicArrangementTable[ipTblIdx].externalIp != thisIpAddr->ipAddr: 1);
			rtl8651IpTableAsicArrangementTable[ipTblIdx].valid = 1;
			rtl8651IpTableAsicArrangementTable[ipTblIdx].externalIp = thisIpAddr->ipAddr;
			rtl8651IpTableAsicArrangementTable[ipTblIdx].internalIp = 0;
			rtl8651IpTableAsicArrangementTable[ipTblIdx].localPublic = 0;
			rtl8651IpTableAsicArrangementTable[ipTblIdx].nat = 0;//Initial to be NAPT external IP address
			naptIpCount++;
			RTL8651_SET_ASIC_IP_TABLE(ipTblIdx);
			_rtl8651_updateGidxRegister();
			thisIpAddr = thisIpAddr->next;
		}
		while(localServer) {
			for(; ipTblIdx<RTL8651_IPTABLE_SIZE; ipTblIdx++)
				if (rtl8651IpTableAsicArrangementTable[ipTblIdx].valid == 0)
					break;
			assert(ipTblIdx < RTL8651_IPTABLE_SIZE);
			rtl8651IpTableAsicArrangementTable[ipTblIdx].valid = 1;
			rtl8651IpTableAsicArrangementTable[ipTblIdx].externalIp = localServer->ipAddr;
			rtl8651IpTableAsicArrangementTable[ipTblIdx].internalIp = localServer->ipAddr;
			rtl8651IpTableAsicArrangementTable[ipTblIdx].localPublic = 1;
			rtl8651IpTableAsicArrangementTable[ipTblIdx].nat = 0;
			localServerCount++;			 
			RTL8651_SET_ASIC_IP_TABLE(ipTblIdx);
			_rtl8651_updateGidxRegister();
			localServer = localServer->next;
		}
		thisIpEntry = thisIpEntry->nextIp;//March to next network for IP address counting
	}
	/* Update ASIC operation layer */
	if(freeIpCount == RTL8651_IPTABLE_SIZE && ipCount > 0)
		rtl8651_setAsicOperationLayer(4);
	RTL8651_UNLOCK_RESOURCE();
	return SUCCESS;
}


int32 rtl8651_delExtNetworkInterface(int8 * ifName) {
	rtl8651_tblDrv_ipIntfEntry_t * netPtr;
	rtl8651_tblDrv_ipIntfIpAddrEntry_t *	ipAddr;
	rtl8651_tblDrv_ipIntfIpAddrEntry_t *	localServer;
	uint32 i, netIdx;
	uint16 vid;

	RTL8651_LOCK_RESOURCE();
	 
	/* When NAPT start to operate, forbid new NAT configuration */
	if(rtl8651ExistedNaptTcpUdpFlows>0 || rtl8651ExistedNaptIcmpFlows>0) {
		RTL8651_UNLOCK_RESOURCE();
		return TBLDRV_ENAPTSTART;
	}
	/* Find out the specified network interface */
	for(netIdx=0; netIdx<rtl8651_tblDrvPara.networkIntfTableSize; netIdx++)
		if (rtl8651NetworkIntfTable[netIdx].valid == 1 &&
		    strncmp(rtl8651NetworkIntfTable[netIdx].ifName, ifName, RTL8651_IPINTF_NAME_LEN)==0)
			break;
	if (netIdx == rtl8651_tblDrvPara.networkIntfTableSize) {
		RTL8651_UNLOCK_RESOURCE();
		return(TBLDRV_EINVNETIFNAME); 
	}
	GET_VID_BY_NETIFIDX(vid, netIdx);
	/* Make sure the network interface is an external interface */
	if (rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].internal == 1) {
		RTL8651_UNLOCK_RESOURCE();
		return(TBLDRV_ESHOULDBEEXTIF); 
	}
	/* Make sure the the interface is not DMZ external */
	if (rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].DMZFlag == 1) {
		RTL8651_UNLOCK_RESOURCE();
		return(TBLDRV_ESHOULDNOTDMZEXT); 
	}
	/* When NAT list is not empty, stop external IP interface deletion */
	if (rtl8651NetworkIntfTable[netIdx].nextNat || rtl8651NetworkIntfTable[netIdx].nextNaptServerPort) {
		RTL8651_UNLOCK_RESOURCE();
		return(TBLDRV_ENETIFREFBYNAT); 
	}
	assert(rtl8651NetworkIntfTable[netIdx].linkLayerType);
	assert(rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].valid == 1);
	rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].internal = 1;
	_rtl8651_updateAsicVlan(vid);
	/* Remove IP address over this network from ASIC IP Table */
	netPtr = rtl8651NetworkIntfTable[netIdx].IpHead;
	while(netPtr) {
		ipAddr = netPtr->ipAddr;
		localServer = netPtr->localServer;
		assert(ipAddr);
		while(ipAddr) {
			for(i=0; i<RTL8651_IPTABLE_SIZE; i++)
				if(rtl8651IpTableAsicArrangementTable[i].valid == 1 &&  rtl8651IpTableAsicArrangementTable[i].externalIp == ipAddr->ipAddr)
					break;
			assert(i < RTL8651_IPTABLE_SIZE); 
			RTL8651_DEL_ASIC_IP_TABLE(i);
			rtl8651IpTableAsicArrangementTable[i].valid = 0;
			naptIpCount--;
			_rtl8651_updateGidxRegister();
			ipAddr = ipAddr->next;
		}
		while(localServer) {
			for(i=0; i<RTL8651_IPTABLE_SIZE; i++)
				if(rtl8651IpTableAsicArrangementTable[i].valid == 1)
					break;
			assert(i < RTL8651_IPTABLE_SIZE);
			RTL8651_DEL_ASIC_IP_TABLE(i);
			rtl8651IpTableAsicArrangementTable[i].valid = 0;
			localServerCount--;
			_rtl8651_updateGidxRegister();
			localServer = localServer->next;
		}
		netPtr = netPtr->nextIp;//March to next network for IP address counting
	}
	if(naptIpCount == 0 && natIpCount ==0 && localServerCount == 0)
		rtl8651_setAsicOperationLayer(3);
	RTL8651_UNLOCK_RESOURCE();
	return SUCCESS;
}


ipaddr_t rtl8651_getNaptInsideGlobalIpAddr(int8 isTcp, ipaddr_t insideLocalIpAddr, uint16 insideLocalPortOrId, ipaddr_t dstIpAddr, uint16 dstPort) {
	
	uint32 i, index;
	
	if(naptIpCount == 0 )
		return 0;

	index = (insideLocalIpAddr & 0x7)%naptIpCount;

	RTL8651_LOCK_RESOURCE();//Lock resource
	 
	if(naptIpCount == 0) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return 0;//No external IP assigned
	}
	for(i=0; i<RTL8651_IPTABLE_SIZE; i++)
		if(rtl8651IpTableAsicArrangementTable[i].valid == 1 && rtl8651IpTableAsicArrangementTable[i].localPublic == 0 && rtl8651IpTableAsicArrangementTable[i].nat == 0) {
			if(index)
				index--;
			else {
				ipaddr_t externalIp = rtl8651IpTableAsicArrangementTable[i].externalIp;
				RTL8651_UNLOCK_RESOURCE();//Unlock resource
				return externalIp;
			}
		}
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return 0;//Unable to find global IP Address
}

int8 _rtl8651_extIpAddr(ipaddr_t ipAddr, uint32 *netIdx) {
	rtl8651_tblDrv_ipIntfEntry_t * ipIntfPtr;
	rtl8651_tblDrv_ipIntfIpAddrEntry_t * ipAddrPtr;
	uint16 vid;
	uint32 i;
	
	for(i=0; i<rtl8651_tblDrvPara.networkIntfTableSize; i++)
		if (rtl8651NetworkIntfTable[i].valid == 1) {
			GET_VID_BY_NETIFIDX(vid, i);
			if (rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].internal == 0) 
				for(ipIntfPtr=rtl8651NetworkIntfTable[i].IpHead; ipIntfPtr; ipIntfPtr=ipIntfPtr->nextIp) {
					ipAddrPtr = ipIntfPtr->ipAddr;
					assert(ipAddrPtr);
					while(ipAddrPtr) {
						if (ipAddrPtr->ipAddr == ipAddr) {
							if (netIdx)
								*netIdx = i;
							return TRUE;
						}
						ipAddrPtr = ipAddrPtr->next;
					}
				}
		}
	return FALSE;//No such ip address appear in network address list
}

int8 _rtl8651_extNetIpAddr(ipaddr_t ipAddr, uint32 *natIdx) {
	rtl8651_tblDrv_ipIntfEntry_t * ipIntfPtr;
	uint32 netIdx;
	uint16 vid;

	for(netIdx=0; netIdx<rtl8651_tblDrvPara.networkIntfTableSize; netIdx++)
		if (rtl8651NetworkIntfTable[netIdx].valid == 1) {
			GET_VID_BY_NETIFIDX(vid, netIdx);
			if (rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].internal == 0) {
				ipIntfPtr = rtl8651NetworkIntfTable[netIdx].IpHead;
				while(ipIntfPtr) {
					if ((ipAddr&ipIntfPtr->ipMask) == (ipIntfPtr->ipAddr->ipAddr&ipIntfPtr->ipMask))
						return TRUE;
					ipIntfPtr = ipIntfPtr->nextIp;
				}
			}
		}
	return FALSE;//No such ip address appear in network address list
}


int8 _rtl8651_localServerIpAddr(ipaddr_t ipAddr) {
	rtl8651_tblDrv_ipIntfEntry_t * ipIntfPtr;
	rtl8651_tblDrv_ipIntfIpAddrEntry_t * ipAddrPtr;
	uint32 netIdx;
	uint16 vid;

	for(netIdx=0; netIdx<rtl8651_tblDrvPara.networkIntfTableSize; netIdx++)
		if(rtl8651NetworkIntfTable[netIdx].valid == 1) {
			ipIntfPtr = rtl8651NetworkIntfTable[netIdx].IpHead;
			GET_VID_BY_NETIFIDX(vid, netIdx);
			/* 
			 * Only the network interface is an External interface, include DMZ,
			 * the Local Server is meaningful.
			 */
			while(ipIntfPtr && ipIntfPtr->localServer && rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].internal == 0) {
				ipAddrPtr = ipIntfPtr->localServer;
				assert(ipAddrPtr);
				while(ipAddrPtr) {
					if(ipAddrPtr->ipAddr == ipAddr)
						return TRUE;
					ipAddrPtr = ipAddrPtr->next;
				}
				ipIntfPtr = ipIntfPtr->nextIp;
			}
		}
	return FALSE;//No such ip address appear in network address list
}


int8 _rtl8651_getVIDByGWIpAddr(ipaddr_t gwIpAddr, uint32 *vid) {
	rtl8651_tblDrv_ipIntfEntry_t * ipIntfPtr;
	uint32 netIdx;

	for(netIdx=0; netIdx<rtl8651_tblDrvPara.networkIntfTableSize; netIdx++) {
		if (rtl8651NetworkIntfTable[netIdx].valid == 0)
			continue;
		for(ipIntfPtr=rtl8651NetworkIntfTable[netIdx].IpHead; ipIntfPtr; ipIntfPtr=ipIntfPtr->nextIp)
			if (ipIntfPtr->ipAddr->ipAddr == gwIpAddr) {
				GET_VID_BY_NETIFIDX((*vid), netIdx);
				return SUCCESS;
			}
	}
	return FAILED;
}


int8 _rtl8651_intIpAddr(ipaddr_t ipAddr) {
	rtl8651_tblDrv_ipIntfEntry_t * ipIntfPtr;
	rtl8651_tblDrv_ipIntfIpAddrEntry_t * ipAddrPtr;
	uint32 netIdx;
	uint16 vid;

	for(netIdx=0; netIdx<rtl8651_tblDrvPara.networkIntfTableSize; netIdx++) 
		if (rtl8651NetworkIntfTable[netIdx].valid == 1) {
			GET_VID_BY_NETIFIDX(vid, netIdx);
			if (rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].internal == 1) {
				ipIntfPtr = rtl8651NetworkIntfTable[netIdx].IpHead;
				while(ipIntfPtr) {
					assert(ipIntfPtr->ipAddr);
					ipAddrPtr = ipIntfPtr->ipAddr;
					while(ipAddrPtr) {
						if (ipAddr == ipAddrPtr->ipAddr)
							return TRUE;
						ipAddrPtr = ipAddrPtr->next;
					}
					ipIntfPtr = ipIntfPtr->nextIp;
				}
			}
		}
	return FALSE;
}

int8 _rtl8651_intNetIpAddr(ipaddr_t ipAddr) {
	rtl8651_tblDrv_ipIntfEntry_t * ipIntfPtr;
	uint32 netIdx;
	uint16 vid;

	for(netIdx=0; netIdx<rtl8651_tblDrvPara.networkIntfTableSize; netIdx++) 
		if (rtl8651NetworkIntfTable[netIdx].valid == 1) {
			GET_VID_BY_NETIFIDX(vid, netIdx);
			if (rtl8651VlanTable[rtl8651_vlanTableIndex(vid)].internal == 1) {
				ipIntfPtr = rtl8651NetworkIntfTable[netIdx].IpHead;
				while(ipIntfPtr) {
					assert(ipIntfPtr->ipAddr);
					if ((ipAddr&ipIntfPtr->ipMask) == (ipIntfPtr->ipAddr->ipAddr&ipIntfPtr->ipMask))
						return TRUE;
					ipIntfPtr = ipIntfPtr->nextIp;						
				}
			}
		}
	return FALSE;
}


int32 rtl8651_addNatMapping(ipaddr_t extIpAddr, ipaddr_t intIpAddr) {
	uint32 ipTblIdx, netIdx;
	rtl8651_tblDrv_natEntry_t * tempNatMappingPtr;

	RTL8651_LOCK_RESOURCE();//Lock resource
	 
	if(_rtl8651_extIpAddr(extIpAddr, &netIdx) == FALSE) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource		
		return TBLDRV_ENOTEXTIP;//Specified external IP address is not one of external IP interface addresses
	}
	if(_rtl8651_intNetIpAddr(intIpAddr) == FALSE) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return TBLDRV_ENOTINTIP;//Specified internal IP address is not host belong to internal network interface
	}
	if(rtl8651FreeNatList == (rtl8651_tblDrv_natEntry_t *)NULL) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return TBLDRV_ENOFREEBUF;//Unable to insert new NAT mapping entry
	}
	assert(rtl8651NetworkIntfTable[netIdx].valid == 1);
	tempNatMappingPtr = rtl8651NetworkIntfTable[netIdx].nextNat;//Check whether entry has already configured
	while(tempNatMappingPtr) {
		if(tempNatMappingPtr->globalIp == extIpAddr || tempNatMappingPtr->localIp == intIpAddr) {
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return TBLDRV_EENTRYEXIST;//Defined NAT mapping with duplicated address
		}
		tempNatMappingPtr = tempNatMappingPtr->next;
	}
	tempNatMappingPtr = rtl8651FreeNatList;
	rtl8651FreeNatList = tempNatMappingPtr->next;
	tempNatMappingPtr->globalIp = extIpAddr;
	tempNatMappingPtr->localIp = intIpAddr;
	tempNatMappingPtr->next = rtl8651NetworkIntfTable[netIdx].nextNat;
	rtl8651NetworkIntfTable[netIdx].nextNat = tempNatMappingPtr;
	for(ipTblIdx =0; ipTblIdx<RTL8651_IPTABLE_SIZE; ipTblIdx++)//Search for corresponding ASIC IP table
		if(rtl8651IpTableAsicArrangementTable[ipTblIdx].valid == 1 && rtl8651IpTableAsicArrangementTable[ipTblIdx].externalIp == extIpAddr && rtl8651IpTableAsicArrangementTable[ipTblIdx].localPublic == 0 && rtl8651IpTableAsicArrangementTable[ipTblIdx].nat == 0)
			break;
	assert(ipTblIdx<RTL8651_IPTABLE_SIZE);
	rtl8651IpTableAsicArrangementTable[ipTblIdx].nat = 1;
	rtl8651IpTableAsicArrangementTable[ipTblIdx].internalIp = intIpAddr;
	naptIpCount--;
	natIpCount++;
	rtl8651_setAsicExtIntIpTable(ipTblIdx, rtl8651IpTableAsicArrangementTable[ipTblIdx].externalIp, rtl8651IpTableAsicArrangementTable[ipTblIdx].internalIp, rtl8651IpTableAsicArrangementTable[ipTblIdx].localPublic == 1 ? TRUE: FALSE, rtl8651IpTableAsicArrangementTable[ipTblIdx].nat == 1 ? TRUE: FALSE); 
	_rtl8651_updateGidxRegister();
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

int32 rtl8651_delNatMapping(ipaddr_t extIpAddr, ipaddr_t intIpAddr) {
	uint32 ipTblIdx, netIdx;
	rtl8651_tblDrv_natEntry_t * tempNatMappingPtr, *deletedNatMappingPtr;

	RTL8651_LOCK_RESOURCE();//Lock resource
	 
	if(_rtl8651_extIpAddr(extIpAddr, &netIdx) == FALSE) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return TBLDRV_ENOTEXTIP;//Specified external IP address is not one of external IP interface addresses
	}
	if(rtl8651NetworkIntfTable[netIdx].nextNat == NULL) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return TBLDRV_EENTRYNOTFOUND;//No NAT entry exist in this control block. Must be failed deletion.
	}
	if(_rtl8651_intNetIpAddr(intIpAddr) == FALSE) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return TBLDRV_ENOTINTIP;//Specified internal IP address is not host belong to internal network interface
	}
	assert(rtl8651NetworkIntfTable[netIdx].valid == 1);
	deletedNatMappingPtr = (rtl8651_tblDrv_natEntry_t *) NULL;
	if(rtl8651NetworkIntfTable[netIdx].nextNat->globalIp == extIpAddr && rtl8651NetworkIntfTable[netIdx].nextNat->localIp == intIpAddr) {
		deletedNatMappingPtr = rtl8651NetworkIntfTable[netIdx].nextNat;
		rtl8651NetworkIntfTable[netIdx].nextNat = deletedNatMappingPtr->next;
	}
	else {
		tempNatMappingPtr = rtl8651NetworkIntfTable[netIdx].nextNat;
		while(tempNatMappingPtr->next) {
			if(tempNatMappingPtr->next->globalIp == extIpAddr && tempNatMappingPtr->next->localIp == intIpAddr) {//Entry found
				deletedNatMappingPtr = tempNatMappingPtr->next;
				tempNatMappingPtr->next = deletedNatMappingPtr->next;
				break;
			}
			tempNatMappingPtr = tempNatMappingPtr->next;
		}
		if(deletedNatMappingPtr == (rtl8651_tblDrv_natEntry_t *) NULL) {
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return TBLDRV_EENTRYNOTFOUND;//Deleted entry not found
		}
	}
	for(ipTblIdx =0; ipTblIdx<RTL8651_IPTABLE_SIZE; ipTblIdx++)//Search for corresponding ASIC IP table
		if(rtl8651IpTableAsicArrangementTable[ipTblIdx].valid == 1 && rtl8651IpTableAsicArrangementTable[ipTblIdx].externalIp == extIpAddr && rtl8651IpTableAsicArrangementTable[ipTblIdx].internalIp == intIpAddr && rtl8651IpTableAsicArrangementTable[ipTblIdx].localPublic == 0 && rtl8651IpTableAsicArrangementTable[ipTblIdx].nat == 1)
			break;
	assert(ipTblIdx<RTL8651_IPTABLE_SIZE);
	rtl8651IpTableAsicArrangementTable[ipTblIdx].nat = 0;
	naptIpCount++; 
	natIpCount--;
	rtl8651_setAsicExtIntIpTable(ipTblIdx, rtl8651IpTableAsicArrangementTable[ipTblIdx].externalIp, 0 /*rtl8651IpTableAsicArrangementTable[ipTblIdx].internalIp*/, rtl8651IpTableAsicArrangementTable[ipTblIdx].localPublic == 1 ? TRUE: FALSE, rtl8651IpTableAsicArrangementTable[ipTblIdx].nat == 1 ? TRUE: FALSE); 
	_rtl8651_updateGidxRegister();
	deletedNatMappingPtr->next = rtl8651FreeNatList;
	rtl8651FreeNatList = deletedNatMappingPtr;
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}


int8 _rtl8651_getAllNatMapping(int32 *entry, rtl8651_tblDrv_natEntry_t *natMap) {
	rtl8651_tblDrv_natEntry_t *tmpNat;
	int32 _size, i, idx;

	_size = (natMap == NULL)? 0: *entry;
	for(i=0, idx=0; i<rtl8651_tblDrvPara.networkIntfTableSize; i++)
		if (rtl8651NetworkIntfTable[i].valid == 1) 
			for(tmpNat=rtl8651NetworkIntfTable[i].nextNat; tmpNat; tmpNat=tmpNat->next) {
				if (natMap != NULL) {
					if (_size <= 0)
						return SUCCESS;
					natMap[idx] = *tmpNat;
					_size--; idx++; 
				} else _size++;
			}
	if (natMap == NULL) *entry = _size;
	return SUCCESS;
}

int8 _rtl8651_getAllServerPort(int32 *entry, rtl8651_tblDrv_naptServerPortEntry_t *serverPort) {
	rtl8651_tblDrv_naptServerPortEntry_t *tmpServ;
	int32 _size, i, idx;

	_size = (serverPort == NULL)? 0: *entry;
	for(i=0, idx=0; i<rtl8651_tblDrvPara.networkIntfTableSize; i++)
		if (rtl8651NetworkIntfTable[i].valid == 1)
			for(tmpServ=rtl8651NetworkIntfTable[i].nextNaptServerPort; tmpServ; tmpServ=tmpServ->next) {
				if (serverPort != NULL) {
					if (_size <= 0)
						return SUCCESS;
					serverPort[idx] = *tmpServ;
					_size--; idx++;
				} else _size++;
			}
	if (serverPort == NULL) *entry = _size;
	return SUCCESS;
}


rtl8651_tblDrv_natEntry_t *_rtl8651_getNatInternalMappingEntry(ipaddr_t intIpAddr) {
	uint32 netIdx; 
	rtl8651_tblDrv_natEntry_t * natPtr;

	for(netIdx=0; netIdx<rtl8651_tblDrvPara.networkIntfTableSize; netIdx++)
		if (rtl8651NetworkIntfTable[netIdx].valid == 1) {
			natPtr = rtl8651NetworkIntfTable[netIdx].nextNat;
			while(natPtr) {
				if (natPtr->localIp == intIpAddr)
					return natPtr;
				natPtr = natPtr->next;
			}
		}
	return NULL;
}



rtl8651_tblDrv_natEntry_t * _rtl8651_getNatExternalMappingEntry(ipaddr_t extIpAddr) {
	uint32 netIdx;
	rtl8651_tblDrv_natEntry_t * natPtr;

	for(netIdx=0; netIdx<rtl8651_tblDrvPara.networkIntfTableSize; netIdx++)
		if (rtl8651NetworkIntfTable[netIdx].valid == 1) {
			natPtr = rtl8651NetworkIntfTable[netIdx].nextNat;
			while(natPtr) {
				if (natPtr->globalIp == extIpAddr)
					return natPtr;
				natPtr = natPtr->next;
			}
		}
	return NULL;
}




int32 rtl8651_addNaptServerPortMapping(int8 isTcp, ipaddr_t extIpAddr, uint16 extPort, ipaddr_t intIpAddr, uint16 intPort) {
	uint32 netIdx, asicPos;
	rtl8651_tblDrv_naptServerPortEntry_t * tempNaptServerPortPtr;

	RTL8651_LOCK_RESOURCE();//Lock resource
	 
	if(_rtl8651_extIpAddr(extIpAddr, &netIdx) == FALSE) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return TBLDRV_ENOTEXTIP;//Specified external IP address is not one of external IP interface addresses
	}
	if(_rtl8651_intNetIpAddr(intIpAddr) == FALSE) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return TBLDRV_ENOTINTIP;//Specified internal IP address is not host belong to internal network interface
	}
	assert(rtl8651NetworkIntfTable[netIdx].valid == 1);
	tempNaptServerPortPtr = rtl8651NetworkIntfTable[netIdx].nextNaptServerPort;
	while(tempNaptServerPortPtr) {//Check whether same ip and port pair exist
		if(tempNaptServerPortPtr->globalIp == extIpAddr && tempNaptServerPortPtr->globalPort == extPort && tempNaptServerPortPtr->localIp == intIpAddr && tempNaptServerPortPtr->localPort == intPort) {
			if((isTcp == TRUE && tempNaptServerPortPtr->tcp == 1) || (isTcp == FALSE && tempNaptServerPortPtr->udp == 1)) {
				RTL8651_UNLOCK_RESOURCE();//Unlock resource
				return TBLDRV_EENTRYEXIST;//Entry already registered
			}
			else {
				if(isTcp == TRUE)
					tempNaptServerPortPtr->tcp = 1;
				else
					tempNaptServerPortPtr->udp = 1;
				RTL8651_UNLOCK_RESOURCE();//Unlock resource
				return SUCCESS;
			}
		}
		tempNaptServerPortPtr = tempNaptServerPortPtr->next;
	}
	if(rtl8651FreeNaptServerPortList == (rtl8651_tblDrv_naptServerPortEntry_t *)NULL) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return TBLDRV_ENOFREEBUF;//Unable to insert new NAPT server port mapping entry
	}
	tempNaptServerPortPtr = rtl8651FreeNaptServerPortList;
	rtl8651FreeNaptServerPortList = tempNaptServerPortPtr->next;
	tempNaptServerPortPtr->next = rtl8651NetworkIntfTable[netIdx].nextNaptServerPort;
	rtl8651NetworkIntfTable[netIdx].nextNaptServerPort = tempNaptServerPortPtr;
	tempNaptServerPortPtr->globalIp = extIpAddr;
	tempNaptServerPortPtr->globalPort = extPort;
	tempNaptServerPortPtr->localIp = intIpAddr;
	tempNaptServerPortPtr->localPort = intPort;
	if(isTcp == TRUE) {
		tempNaptServerPortPtr->tcp = 1;
		tempNaptServerPortPtr->udp = 0;
	}
	else {
		tempNaptServerPortPtr->tcp = 0;
		tempNaptServerPortPtr->udp = 1;
	}
	for(asicPos=0; asicPos<RTL8651_SERVERPORTTBL_SIZE; asicPos++)
		if(rtl8651ServerPortAsicArrangementTable[asicPos].valid == 0)
			break;
	if(asicPos<RTL8651_SERVERPORTTBL_SIZE) {
		tempNaptServerPortPtr->asicTablePos = asicPos;
		tempNaptServerPortPtr->toAsic = 1;
		rtl8651ServerPortAsicArrangementTable[asicPos].valid = 1;
		rtl8651_setAsicServerPortTable(tempNaptServerPortPtr->asicTablePos, tempNaptServerPortPtr->globalIp, tempNaptServerPortPtr->globalPort, tempNaptServerPortPtr->localIp, tempNaptServerPortPtr->localPort);
	}
	else {
		tempNaptServerPortPtr->toAsic = 0;//Using ACL to replace this
		_rtl8651_updateAsicAllVlan();
	}
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

int32 rtl8651_delNaptServerPortMapping(int8 isTcp, ipaddr_t extIpAddr, uint16 extPort, ipaddr_t intIpAddr, uint16 intPort) {
	uint32 netIdx;
	rtl8651_tblDrv_naptServerPortEntry_t * deletedNaptServerPortPtr, * trackNaptServerPortPtr;

	RTL8651_LOCK_RESOURCE();//Lock resource
	 
	if(_rtl8651_extIpAddr(extIpAddr, &netIdx) == FALSE) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return TBLDRV_ENOTEXTIP;//Specified external IP address is not one of external IP interface addresses
	}
	if(_rtl8651_intNetIpAddr(intIpAddr) == FALSE) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return TBLDRV_ENOTINTIP;//Specified internal IP address is not host belong to internal network interface
	}
	assert(rtl8651NetworkIntfTable[netIdx].valid == 1);
	trackNaptServerPortPtr = rtl8651NetworkIntfTable[netIdx].nextNaptServerPort;
	while(trackNaptServerPortPtr) {//Check whether same ip and port pair exist
		if(trackNaptServerPortPtr->globalIp == extIpAddr && trackNaptServerPortPtr->globalPort == extPort && trackNaptServerPortPtr->localIp == intIpAddr && trackNaptServerPortPtr->localPort == intPort && ((isTcp == TRUE && trackNaptServerPortPtr->tcp == 1) || (isTcp == FALSE && trackNaptServerPortPtr->udp == 1))) {
			if(isTcp == TRUE)
				trackNaptServerPortPtr->tcp = 0;
			else
				trackNaptServerPortPtr->udp = 0;
			if(trackNaptServerPortPtr->tcp == 1 || trackNaptServerPortPtr->udp == 1) {
				RTL8651_UNLOCK_RESOURCE();//Unlock resource
				return SUCCESS;
			}
			else
				break;//Start remove process
		}
		trackNaptServerPortPtr = trackNaptServerPortPtr->next;
	}
	if(trackNaptServerPortPtr == NULL) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return TBLDRV_EENTRYNOTFOUND;//No server port able to delete
	}
	deletedNaptServerPortPtr = NULL;
	if(rtl8651NetworkIntfTable[netIdx].nextNaptServerPort->globalIp == extIpAddr && rtl8651NetworkIntfTable[netIdx].nextNaptServerPort->globalPort == extPort && rtl8651NetworkIntfTable[netIdx].nextNaptServerPort->localIp == intIpAddr && rtl8651NetworkIntfTable[netIdx].nextNaptServerPort->localPort == intPort) {
		deletedNaptServerPortPtr = rtl8651NetworkIntfTable[netIdx].nextNaptServerPort;
		rtl8651NetworkIntfTable[netIdx].nextNaptServerPort = deletedNaptServerPortPtr->next;
	}
	else {
		trackNaptServerPortPtr = rtl8651NetworkIntfTable[netIdx].nextNaptServerPort;
		while(trackNaptServerPortPtr->next) {
			if(trackNaptServerPortPtr->next->globalIp == extIpAddr && trackNaptServerPortPtr->next->globalPort == extPort && trackNaptServerPortPtr->next->localIp == intIpAddr && trackNaptServerPortPtr->next->localPort == intPort) {
				deletedNaptServerPortPtr = trackNaptServerPortPtr->next;
				trackNaptServerPortPtr->next = deletedNaptServerPortPtr->next;
				break;
			}
			trackNaptServerPortPtr = trackNaptServerPortPtr->next;
		}
	}
	assert(deletedNaptServerPortPtr);//Tracking procedure has confirm the entry do exist
	assert(deletedNaptServerPortPtr->tcp == 0 && deletedNaptServerPortPtr->udp == 0);//Tracking procedure should confirm this
	deletedNaptServerPortPtr->next = rtl8651FreeNaptServerPortList;
	rtl8651FreeNaptServerPortList = deletedNaptServerPortPtr;
	if(deletedNaptServerPortPtr->toAsic == 1) {
		rtl8651ServerPortAsicArrangementTable[deletedNaptServerPortPtr->asicTablePos].valid = 0;
		rtl8651_delAsicServerPortTable(deletedNaptServerPortPtr->asicTablePos);
	} else _rtl8651_updateAsicAllVlan();
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

rtl8651_tblDrv_naptServerPortEntry_t *  
_rtl8651_getOutsideNaptServerPortMapping(int8 isTcp, ipaddr_t extIpAddr, uint16 extPort) {
	uint32 netIdx;
	rtl8651_tblDrv_naptServerPortEntry_t * naptServerPortPtr;

	for(netIdx=0; netIdx<rtl8651_tblDrvPara.networkIntfTableSize; netIdx++)
		if (rtl8651NetworkIntfTable[netIdx].valid == 1) {
			naptServerPortPtr = rtl8651NetworkIntfTable[netIdx].nextNaptServerPort;
			while(naptServerPortPtr) {
				if(naptServerPortPtr->globalIp == extIpAddr && naptServerPortPtr->globalPort == extPort && naptServerPortPtr->tcp == (isTcp == TRUE? 1: 0))
					return naptServerPortPtr;
				naptServerPortPtr = naptServerPortPtr->next;
			}
		}
	return NULL;
}

rtl8651_tblDrv_naptServerPortEntry_t *  
_rtl8651_getInsideNaptServerPortMapping(int8 isTcp, ipaddr_t intIpAddr, uint16 intPort) {
	uint32 netIdx;
	rtl8651_tblDrv_naptServerPortEntry_t * naptServerPortPtr;

	for(netIdx=0; netIdx<rtl8651_tblDrvPara.networkIntfTableSize; netIdx++)
		if (rtl8651NetworkIntfTable[netIdx].valid == 1) {
			naptServerPortPtr = rtl8651NetworkIntfTable[netIdx].nextNaptServerPort;
			while(naptServerPortPtr) {
				if(naptServerPortPtr->localIp== intIpAddr && naptServerPortPtr->localPort== intPort && naptServerPortPtr->tcp == (isTcp == TRUE? 1: 0))
					return naptServerPortPtr;
				naptServerPortPtr = naptServerPortPtr->next;
			}
		}
	return NULL;
}


int32 rtl8651_setNaptIcmpTimeout(uint32 timeout) {
	RTL8651_LOCK_RESOURCE();//Lock resource
	 
	rtl8651GlobalControl.icmpTimeout = timeout;
	rtl8651_setAsicNaptIcmpTimeout(timeout);
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

int32 rtl8651_setNaptUdpTimeout(uint32 timeout) {
	RTL8651_LOCK_RESOURCE();//Lock resource
	 
	rtl8651GlobalControl.udpTimeout = timeout;
	rtl8651_setAsicNaptUdpTimeout(timeout);
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

int32 rtl8651_setNaptTcpLongTimeout(uint32 timeout) {
	RTL8651_LOCK_RESOURCE();//Lock resource
	 
	rtl8651GlobalControl.tcpLongTimeout = timeout;
	rtl8651_setAsicNaptTcpLongTimeout(timeout);
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

int32 rtl8651_setNaptTcpMediumTimeout(uint32 timeout) {
	RTL8651_LOCK_RESOURCE();//Lock resource
	 
	rtl8651GlobalControl.tcpMediumTimeout = timeout;
	rtl8651_setAsicNaptTcpMediumTimeout(timeout);
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

int32 rtl8651_setNaptTcpFastTimeout(uint32 timeout) {
	RTL8651_LOCK_RESOURCE();//Lock resource
	 
	rtl8651GlobalControl.tcpFastTimeout = timeout;
	rtl8651_setAsicNaptTcpFastTimeout(timeout);
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

//Clear both TCP/UDP flows and ICMP flows
static int32 _rtl8651_flushNaptFlow(void) {
	uint32 flowTblIdx;
	rtl8651_tblDrv_naptTcpUdpFlowEntry_t * tempNaptTcpUdpFlowPtr;
	rtl8651_tblDrv_naptIcmpFlowEntry_t * tempNaptIcmpFlowPtr;
 
	for(flowTblIdx=0; flowTblIdx<RTL8651_TCPUDPTBL_SIZE; flowTblIdx++)
		while(rtl8651NaptTcpUdpFlowTable[flowTblIdx]) {
			tempNaptTcpUdpFlowPtr = rtl8651NaptTcpUdpFlowTable[flowTblIdx];
			rtl8651NaptTcpUdpFlowTable[flowTblIdx] = tempNaptTcpUdpFlowPtr->next;
			tempNaptTcpUdpFlowPtr->next = rtl8651FreeNaptTcpUdpFlowEntryList;
			rtl8651FreeNaptTcpUdpFlowEntryList = tempNaptTcpUdpFlowPtr;
		}
	for(flowTblIdx=0; flowTblIdx<RTL8651_TCPUDPTBL_SIZE; flowTblIdx++) {
		rtl8651NaptTcpUdpBackwardCollisionCountTable[flowTblIdx] = 0;
		rtl8651NaptTcpUdpFlowNonAsicNumberTable[flowTblIdx] = 0;
		rtl8651NaptTcpUdpFlowNumberTable[flowTblIdx] = 0;
	}
	for(flowTblIdx=0; flowTblIdx<RTL8651_ICMPTBL_SIZE; flowTblIdx++) {
		while(rtl8651NaptIcmpFlowTable[flowTblIdx]) {
			tempNaptIcmpFlowPtr = rtl8651NaptIcmpFlowTable[flowTblIdx];
			rtl8651NaptIcmpFlowTable[flowTblIdx] = tempNaptIcmpFlowPtr->next;
			tempNaptIcmpFlowPtr->next = rtl8651FreeNaptIcmpFlowEntryList;
			rtl8651FreeNaptIcmpFlowEntryList = tempNaptIcmpFlowPtr;
		}
	}
	for(flowTblIdx=0; flowTblIdx<RTL8651_ICMPTBL_SIZE; flowTblIdx++) {
		rtl8651NaptIcmpBackwardCollisionCountTable[flowTblIdx] = 0;
		rtl8651NaptIcmpFlowNonAsicNumberTable[flowTblIdx] = 0;
		rtl8651NaptIcmpFlowNumberTable[flowTblIdx] = 0;
	}
	rtl8651ExistedNaptTcpUdpFlows = 0;
	rtl8651ExistedNaptIcmpFlows = 0;
	
	/* flush ASIC Napt TCPUDP flows */
	for(flowTblIdx=0; flowTblIdx<RTL8651_TCPUDPTBL_SIZE; flowTblIdx++)
		rtl8651_setAsicNaptTcpUdpTable_Patch(TRUE, 0, 0, flowTblIdx, 0, FALSE, FALSE, 0, FALSE, FALSE);	
	/* flush ASIC Napt ICMP flows */
	for(flowTblIdx=0; flowTblIdx<RTL8651_ICMPTBL_SIZE; flowTblIdx++)
		rtl8651_setAsicNaptIcmpTable_Patch(TRUE, 0, 0, flowTblIdx, 0, FALSE, 0, FALSE, FALSE);
	_rtl8651_tblDrvStatisticReset();
	return SUCCESS;
}


int32 rtl8651_flushNaptFlow(void) {

	int32 retval;
	
	RTL8651_LOCK_RESOURCE();//Lock resource
	retval = _rtl8651_flushNaptFlow();
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return retval;
}


int32 rtl8651_setNaptTcpUdpPortRange(uint16 start, uint16 end) {

	if ((start&0x3ff) != 0 || (end&0x3ff) != 0x3ff || start > end) {
		return(TBLDRV_EINVVAL); 
	}
	start = start >> 10; end = end >> 10;
	if (end > 0x3f || start > 0x3f) {
		return(TBLDRV_EINVVAL); 
	}
	RTL8651_LOCK_RESOURCE();//Lock resource
	 
	rtl8651GlobalControl.l4PortRangeEnd = end;
	rtl8651GlobalControl.l4PortRangeStart = start;
	rtl8651_setAsicL4Offset(rtl8651GlobalControl.l4PortRangeStart, rtl8651GlobalControl.l4PortRangeEnd);
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

int8 _rtl8651_getUsableExtIpAndPort(int8 isTcp, ipaddr_t sip, uint16 sport, ipaddr_t dip, uint16 dport, ipaddr_t *extip, uint16 *extport) {
	uint32 flowTblIdx;
	uint16 offset;
	flowTblIdx = rtl8651_naptTcpUdpTableIndex(isTcp, sip, sport, dip, dport);
	if(_rtl8651_selectNaptTcpUdpFlowOffset(flowTblIdx, &offset) == FAILED)
			return FAILED;
	*extport = offset<<10 | flowTblIdx;
	*extip = rtl8651_getNaptInsideGlobalIpAddr(isTcp, sip, sport, dip, dport);
	return SUCCESS;
}

static int32 _rtl8651_selectNaptTcpUdpFlowOffset(uint32 flowTblIdx, uint16 *offset) {
	int8 isValid;
	uint64 mask=0; //an empty bitmap. flatten all possible offset values.
	rtl8651_tblDrv_naptTcpUdpFlowEntry_t * tempNaptTcpUdpFlowPtr;
	rtl8651_tblDrv_rsvPortNumber_t *tmpRsvPort;

	/* Read offset of ASIC entry, should avoid using this value */
	rtl8651_getAsicNaptTcpUdpOffset(flowTblIdx, offset, &isValid);
	assert(*offset<64);//rtl8651 offset should ranged from 0 to 63
	if(isValid == TRUE)
		mask |= ((uint64)1)<<(*offset);

	//Do we have a pending/collision entry on hand? should avoid using those values
	tempNaptTcpUdpFlowPtr = rtl8651NaptTcpUdpFlowTable[flowTblIdx];
	while(tempNaptTcpUdpFlowPtr) {//Search whether flow exist
		if(tempNaptTcpUdpFlowPtr->canAsic == 1) {
			*offset = tempNaptTcpUdpFlowPtr->insideGlobalPort>>10;
			mask |= ((uint64)1)<<(*offset);
		}
		tempNaptTcpUdpFlowPtr = tempNaptTcpUdpFlowPtr->next;
	}
	
	/* Check reserved port. Avoid using these offset values */
	for(tmpRsvPort=rtl8651RsvPortList; tmpRsvPort; tmpRsvPort=tmpRsvPort->next)
		if (flowTblIdx == (tmpRsvPort->port&0x03ff)) {
			*offset = (tmpRsvPort->port)>>10;
			mask |= ((uint64)1)<<(*offset);
		}
		
	/* now check which offset value within configured range we can use */
	for(*offset = rtl8651GlobalControl.l4PortRangeStart; *offset <= rtl8651GlobalControl.l4PortRangeEnd; (*offset)++)
		if((mask & ((uint64)1)<<(*offset)) == 0)//entry not occupied
			break;

	if(*offset > rtl8651GlobalControl.l4PortRangeEnd) {
		return FAILED;//Available offset not found
	}

	return SUCCESS;//Valid offset is selected
}

static rtl8651_tblDrv_naptTcpUdpFlowEntry_t *  _rtl8651_getNaptTcpUdpFlowEntry(uint32 flowTblIdx, int8 isTcp, ipaddr_t insideLocalIpAddr, uint16 insideLocalPort) {
	rtl8651_tblDrv_naptTcpUdpFlowEntry_t * tempNaptTcpUdpFlowPtr;
	
	tempNaptTcpUdpFlowPtr = rtl8651NaptTcpUdpFlowTable[flowTblIdx];
	while(tempNaptTcpUdpFlowPtr) {//Search whether flow exist
		if((tempNaptTcpUdpFlowPtr->tcp ==1? TRUE: FALSE) == isTcp && 
			tempNaptTcpUdpFlowPtr->insideLocalPort == insideLocalPort && 
			tempNaptTcpUdpFlowPtr->insideLocalIpAddr == insideLocalIpAddr) {
			return tempNaptTcpUdpFlowPtr;
		}
		tempNaptTcpUdpFlowPtr = tempNaptTcpUdpFlowPtr->next;
	}
	return NULL;
}


rtl8651_tblDrv_naptTcpUdpFlowEntry_t *
_rtl8651_getOutsideNaptTcpUdpEntry(int8 isTcp, ipaddr_t gip, uint16 gport, ipaddr_t dip, uint16 dport, int8 isRefresh) {
	rtl8651_tblDrv_naptTcpUdpFlowEntry_t *tmpNaptEntry;
	uint32 flowTblIdx = gport&(RTL8651_TCPUDPTBL_SIZE-1);

	tmpNaptEntry = rtl8651NaptTcpUdpFlowTable[flowTblIdx];
	for(; tmpNaptEntry; tmpNaptEntry=tmpNaptEntry->next) 
		if ((tmpNaptEntry->tcp == ((isTcp==TRUE)?1: 0))&&(tmpNaptEntry->insideGlobalPort == gport)) {
			if (tmpNaptEntry->fromAsic == 1)
				break;
			if ((tmpNaptEntry->fromAsic == 0) && 
				(tmpNaptEntry->insideGlobalIpAddr == gip) &&
				(tmpNaptEntry->dstIpAddr == dip) &&
				(tmpNaptEntry->dstPort == dport))
				break;
		}
	if (tmpNaptEntry != (rtl8651_tblDrv_naptTcpUdpFlowEntry_t *) NULL) {
		if (isRefresh == TRUE) {
			if (tmpNaptEntry->tcp == 0)
				tmpNaptEntry->age = rtl8651GlobalControl.udpTimeout;
			else if (tmpNaptEntry->tcpFlag < RTL8651_TCPFLAGS_ESTABLISHED ||
					tmpNaptEntry->tcpFlag == RTL8651_TCPFLAGS_2MSL)
				tmpNaptEntry->age = rtl8651GlobalControl.tcpFastTimeout;
			else if (tmpNaptEntry->tcpFlag > RTL8651_TCPFLAGS_ESTABLISHED)
				tmpNaptEntry->age = rtl8651GlobalControl.tcpMediumTimeout;
			else 
				tmpNaptEntry->age = rtl8651GlobalControl.tcpLongTimeout;
			//else assert(0):
		}
		return tmpNaptEntry;
	}

	if(rtl8651FreeNaptTcpUdpFlowEntryList) {
		ipaddr_t asicInsideLocalIp;
		uint16 asicInsideLocalPort, asicInsideGlobalPort, asicAgeSec;
		int8 isAsicStatic, isAsicTcp, isAsicCollision, isAsicValid, asicTcpFlag;
		rtl8651_getAsicNaptTcpUdpTable(FALSE, flowTblIdx, &asicInsideLocalIp, &asicInsideLocalPort, &asicInsideGlobalPort,&asicAgeSec, &isAsicStatic, &isAsicTcp, &asicTcpFlag, &isAsicCollision, &isAsicValid);
		if(isAsicValid == TRUE && asicInsideGlobalPort == gport && isTcp == isAsicTcp) {
			tmpNaptEntry = rtl8651FreeNaptTcpUdpFlowEntryList;
			rtl8651FreeNaptTcpUdpFlowEntryList = tmpNaptEntry->next;
			tmpNaptEntry->next = rtl8651NaptTcpUdpFlowTable[flowTblIdx];
			rtl8651NaptTcpUdpFlowTable[flowTblIdx] = tmpNaptEntry;
			rtl8651NaptTcpUdpFlowNumberTable[flowTblIdx]++;
			rtl8651ExistedNaptTcpUdpFlows++;
			tmpNaptEntry->canAsic = 1;
			tmpNaptEntry->insideLocalIpAddr = asicInsideLocalIp;
			tmpNaptEntry->insideLocalPort = asicInsideLocalPort;
			tmpNaptEntry->insideGlobalPort = asicInsideGlobalPort;
			tmpNaptEntry->insideGlobalIpAddr = 0;
			tmpNaptEntry->age = asicAgeSec;
			tmpNaptEntry->protoDel = 1;
			tmpNaptEntry->alive = 1;
			tmpNaptEntry->forAlg = 0;
			tmpNaptEntry->tcp = isAsicTcp==TRUE? 1: 0;
			tmpNaptEntry->tcpFlag = asicTcpFlag;
			tmpNaptEntry->fromAsic = 1;
			tmpNaptEntry->toAsic = 1;
			tmpNaptEntry->staticEntry = 0; /* should be dynamic entry */
			tmpNaptEntry->fromDrv = 0;
			rtl8651_tblDrvNaptTcpUdpCounter.drvAddCount++;
			return tmpNaptEntry;
		}
	}
	return (rtl8651_tblDrv_naptTcpUdpFlowEntry_t *)NULL;
}


rtl8651_tblDrv_naptTcpUdpFlowEntry_t *
_rtl8651_getInsideNaptTcpUdpEntry(int8 isTcp, ipaddr_t sip, uint16 sport, ipaddr_t dip, uint16 dport, int8 isRefresh) {
	rtl8651_tblDrv_naptTcpUdpFlowEntry_t *tmpNaptEntry;
	uint32 flowTblIdx;

	flowTblIdx = rtl8651_naptTcpUdpTableIndex(isTcp, sip, sport, dip, dport);
	tmpNaptEntry = rtl8651NaptTcpUdpFlowTable[flowTblIdx];
	for(; tmpNaptEntry; tmpNaptEntry=tmpNaptEntry->next) {
		if ((tmpNaptEntry->tcp == ((isTcp==TRUE)? 1: 0)) &&
			(tmpNaptEntry->insideLocalIpAddr == sip) &&
			(tmpNaptEntry->insideLocalPort == sport) &&
			(tmpNaptEntry->dstIpAddr == dip) &&
			(tmpNaptEntry->dstPort == dport)) {
				if (isRefresh == TRUE) {
					if (tmpNaptEntry->tcp == 0)
						tmpNaptEntry->age = rtl8651GlobalControl.udpTimeout;
					else if (tmpNaptEntry->tcpFlag < RTL8651_TCPFLAGS_ESTABLISHED ||
							tmpNaptEntry->tcpFlag == RTL8651_TCPFLAGS_2MSL)
						tmpNaptEntry->age = rtl8651GlobalControl.tcpFastTimeout;
					else if (tmpNaptEntry->tcpFlag > RTL8651_TCPFLAGS_ESTABLISHED)
						tmpNaptEntry->age = rtl8651GlobalControl.tcpMediumTimeout;
					else 
						tmpNaptEntry->age = rtl8651GlobalControl.tcpLongTimeout;
					//else assert(0):
				}
				return tmpNaptEntry;
			}
	}
	return (rtl8651_tblDrv_naptTcpUdpFlowEntry_t *)NULL;
}
	
#if 0

int8 _rtl8651_freeReservedNaptTcpUdpExtPort(uint16 extPort) {
	rtl8651_tblDrv_rsvPortNumber_t *tmpRsvPort, *delRsvPort;

	tmpRsvPort = (rtl8651_tblDrv_rsvPortNumber_t *)0;
	for(delRsvPort=rtl8651RsvPortList; delRsvPort; 
		tmpRsvPort=delRsvPort,delRsvPort=delRsvPort->next)
		if (delRsvPort->port == extPort) {
			if (tmpRsvPort == (rtl8651_tblDrv_rsvPortNumber_t *)0) {
				rtl8651RsvPortList = delRsvPort->next;
				delRsvPort->next = rtl8651FreeRsvPortList;
				rtl8651FreeRsvPortList = delRsvPort;
			}else {
				tmpRsvPort->next = delRsvPort->next;
				delRsvPort->next = rtl8651FreeRsvPortList;
				rtl8651FreeRsvPortList = delRsvPort;
			}
			return SUCCESS;
		}
	return FAILED;
}

int8 _rtl8651_reserveNaptTcpUdpExtPort(uint16 extPort) {
	rtl8651_tblDrv_rsvPortNumber_t *tmpRsvPort;
	rtl8651_tblDrv_naptTcpUdpFlowEntry_t *tmpTcpUdpEntry;
	
	if (rtl8651FreeRsvPortList == NULL) 
		return FAILED; /* out of allowed reserved port */
	/* Check if the port has been reserved */
	for(tmpRsvPort=rtl8651RsvPortList; tmpRsvPort; tmpRsvPort=tmpRsvPort->next)
		if (tmpRsvPort->port == extPort)
			return FAILED; /* port is already reserved */
	/* Check if the port is already in use */
	tmpTcpUdpEntry = rtl8651NaptTcpUdpFlowTable[extPort&0x3ff];
	for(; tmpTcpUdpEntry; tmpTcpUdpEntry=tmpTcpUdpEntry->next)
		if (tmpTcpUdpEntry->insideGlobalPort == extPort)
			return FAILED; /* port already in use */
	/* Allocate free slot */
	tmpRsvPort = rtl8651FreeRsvPortList;
	rtl8651FreeRsvPortList = tmpRsvPort->next;
	tmpRsvPort->port = extPort;
	tmpRsvPort->next = rtl8651RsvPortList;
	rtl8651RsvPortList = tmpRsvPort;
	return SUCCESS;
}
#endif


static int32 _rtl8651_delNaptTcpUdpFlowEntry(uint32 flowTblIdx, rtl8651_tblDrv_naptTcpUdpFlowEntry_t * deletedEntry) {
	rtl8651_tblDrv_naptTcpUdpFlowEntry_t * trackEntry;

	if(deletedEntry == NULL || rtl8651NaptTcpUdpFlowTable[flowTblIdx] == NULL)
		return FAILED;//No entry to delete
	/*
	 * If the 'protoDel' field is turned on, then remove it from ASIC and driver
	 * table; otherwise, only remove the entry from ASIC table and the protocol
	 * stack should take care of entry deletion.
	 */
	if (deletedEntry->protoDel == 1) {
		if(rtl8651NaptTcpUdpFlowTable[flowTblIdx] == deletedEntry)
			rtl8651NaptTcpUdpFlowTable[flowTblIdx] = deletedEntry->next;
		else {
			trackEntry = rtl8651NaptTcpUdpFlowTable[flowTblIdx];
			while(trackEntry->next) {
				if(trackEntry->next == deletedEntry) {
					trackEntry->next = deletedEntry->next;
					break;
				}
				trackEntry = trackEntry->next;
			}
			if(trackEntry == NULL)
				return FAILED;//No matching entry found.
		}
		/* Put the deleted entry to free slot */
		deletedEntry->next = rtl8651FreeNaptTcpUdpFlowEntryList;
		rtl8651FreeNaptTcpUdpFlowEntryList = deletedEntry;
		rtl8651ExistedNaptTcpUdpFlows--;
	}
	if (deletedEntry->forAlg == 0) {
		rtl8651NaptTcpUdpFlowNumberTable[flowTblIdx]--;
		if(deletedEntry->canAsic == 0)
		rtl8651NaptTcpUdpFlowNonAsicNumberTable[flowTblIdx]--;
	}
	deletedEntry->alive = 0;
	return SUCCESS;
}

static rtl8651_tblDrv_naptTcpUdpFlowEntry_t * 
_rtl8651_getAsicNaptTcpUdpDynamicFlowToTable(uint32 flowTblIdx) {
	ipaddr_t asicInsideLocalIp;
	uint16 asicInsideLocalPort, asicInsideGlobalPort, asicAgeSec;
	int8 isAsicStatic, isAsicTcp, isAsicCollision, isAsicValid, asicTcpFlag;
	rtl8651_tblDrv_naptTcpUdpFlowEntry_t * asicNaptTcpUdpFlowPtr;
	
	rtl8651_getAsicNaptTcpUdpTable(FALSE, flowTblIdx, &asicInsideLocalIp, &asicInsideLocalPort, &asicInsideGlobalPort,&asicAgeSec, &isAsicStatic, &isAsicTcp, &asicTcpFlag, &isAsicCollision, &isAsicValid);
	if(isAsicValid == FALSE) {//Remove from ASIC entry
		asicNaptTcpUdpFlowPtr = NULL;
	}
	else {//Entry valid
		asicNaptTcpUdpFlowPtr = _rtl8651_getNaptTcpUdpFlowEntry(flowTblIdx, isAsicTcp, asicInsideLocalIp, asicInsideLocalPort);
		assert(asicNaptTcpUdpFlowPtr == NULL || asicNaptTcpUdpFlowPtr->toAsic == 1);
		if(asicNaptTcpUdpFlowPtr) {
			/*
			 * Update aging time and tcp state.
			 */
			asicNaptTcpUdpFlowPtr->age = asicAgeSec;
			asicNaptTcpUdpFlowPtr->tcpFlag = asicTcpFlag; /* Only for tcp */
			asicNaptTcpUdpFlowPtr->toAsic = 0;
			assert(asicNaptTcpUdpFlowPtr->canAsic == 1);
			return asicNaptTcpUdpFlowPtr;//Entry is duplicated no further process is required
		}
		/*
		 * Otherwise, the entry was added by ASIC auto-learn.
		 */
		assert(isAsicStatic == FALSE);//Should not get static entry but not in driver table
		if(rtl8651FreeNaptTcpUdpFlowEntryList == NULL)
			return NULL;//Entry exist but not storage to store it.
		asicNaptTcpUdpFlowPtr = rtl8651FreeNaptTcpUdpFlowEntryList;
		rtl8651FreeNaptTcpUdpFlowEntryList = asicNaptTcpUdpFlowPtr->next;
		asicNaptTcpUdpFlowPtr->next = rtl8651NaptTcpUdpFlowTable[flowTblIdx];
		rtl8651NaptTcpUdpFlowTable[flowTblIdx] = asicNaptTcpUdpFlowPtr;
		rtl8651NaptTcpUdpFlowNumberTable[flowTblIdx]++;
		rtl8651ExistedNaptTcpUdpFlows++;
		asicNaptTcpUdpFlowPtr->canAsic = 1;
		asicNaptTcpUdpFlowPtr->insideLocalIpAddr = asicInsideLocalIp;
		asicNaptTcpUdpFlowPtr->insideLocalPort = asicInsideLocalPort;
		asicNaptTcpUdpFlowPtr->insideGlobalPort = asicInsideGlobalPort;
		asicNaptTcpUdpFlowPtr->age = asicAgeSec;
		asicNaptTcpUdpFlowPtr->protoDel = 1;
		asicNaptTcpUdpFlowPtr->alive = 1;
		asicNaptTcpUdpFlowPtr->forAlg = 0;
		asicNaptTcpUdpFlowPtr->tcp = isAsicTcp==TRUE? 1: 0;
		asicNaptTcpUdpFlowPtr->tcpFlag = asicTcpFlag;
		asicNaptTcpUdpFlowPtr->fromAsic = 1;
		asicNaptTcpUdpFlowPtr->toAsic = 1;
		asicNaptTcpUdpFlowPtr->staticEntry = 0; /* should be dynamic entry */
		asicNaptTcpUdpFlowPtr->fromDrv = 0;
		rtl8651_tblDrvNaptTcpUdpCounter.drvAddCount++;
	}
	return asicNaptTcpUdpFlowPtr;
}



int8 _rtl8651_addNaptTcpUdpCandidateFlow(int8 fromDrv, int8 flowType, ipaddr_t insideLocalIpAddr, uint16 insideLocalPort, 
			ipaddr_t insideGlobalIpAddr, uint16 insideGlobalPort, ipaddr_t dstIpAddr, uint16 dstPort) {
	uint32 flowTblIdx;
	ipaddr_t asicLocalIpAddr;
	uint16 asicLocalPort, asicGlobalPort, asicAgeSec;
	int8 asicIsStatic, asicIsTcp, asicTcpFlag, asicIsCollision, asicIsValid;
	rtl8651_tblDrv_naptTcpUdpFlowEntry_t * tempNaptTcpUdpFlowPtr, 
										 * trackNaptTcpUdpFlowPtr;

	flowTblIdx = rtl8651_naptTcpUdpTableIndex((flowType==FALSE? FALSE: TRUE), insideLocalIpAddr, insideLocalPort, dstIpAddr, dstPort);
	if((tempNaptTcpUdpFlowPtr = _rtl8651_getNaptTcpUdpFlowEntry(flowTblIdx, (flowType==FALSE? FALSE: TRUE), insideLocalIpAddr, insideLocalPort))) {
		rtl8651_tblDrvNaptTcpUdpCounter.addFailure++;
printf("=>intIP(%d.%d.%d.%d) intPort(%d)\n", insideLocalIpAddr>>24, (insideLocalIpAddr&0x00ff0000)>>16,
	(insideLocalIpAddr&0x0000ff00)>>8, insideLocalIpAddr&0xff, insideLocalPort);

trackNaptTcpUdpFlowPtr=rtl8651NaptTcpUdpFlowTable[flowTblIdx];
while(trackNaptTcpUdpFlowPtr) {
	printf("->intIP(%d.%d.%d.%d) intPort(%d)\n", trackNaptTcpUdpFlowPtr->insideLocalIpAddr >>24, (trackNaptTcpUdpFlowPtr->insideLocalIpAddr&0x00ff0000)>>16,
	(trackNaptTcpUdpFlowPtr->insideLocalIpAddr&0x0000ff00)>>8, trackNaptTcpUdpFlowPtr->insideLocalIpAddr&0xff, trackNaptTcpUdpFlowPtr);
	trackNaptTcpUdpFlowPtr = trackNaptTcpUdpFlowPtr->next;
}
		return FAILED;
	}
	if(rtl8651FreeNaptTcpUdpFlowEntryList == NULL) {
		rtl8651_tblDrvNaptTcpUdpCounter.addFailure++;
		return FAILED;//No free flow entry available
	}
	tempNaptTcpUdpFlowPtr = rtl8651FreeNaptTcpUdpFlowEntryList;
	rtl8651FreeNaptTcpUdpFlowEntryList = tempNaptTcpUdpFlowPtr->next;
	tempNaptTcpUdpFlowPtr->next = rtl8651NaptTcpUdpFlowTable[flowTblIdx];
	rtl8651NaptTcpUdpFlowTable[flowTblIdx] = tempNaptTcpUdpFlowPtr;
	rtl8651NaptTcpUdpFlowNumberTable[flowTblIdx]++;
	rtl8651ExistedNaptTcpUdpFlows++;
	tempNaptTcpUdpFlowPtr->dstIpAddr = dstIpAddr;
	tempNaptTcpUdpFlowPtr->dstPort = dstPort;
	tempNaptTcpUdpFlowPtr->insideLocalIpAddr = insideLocalIpAddr;
	tempNaptTcpUdpFlowPtr->insideLocalPort = insideLocalPort;
	tempNaptTcpUdpFlowPtr->insideGlobalIpAddr = insideGlobalIpAddr;
	tempNaptTcpUdpFlowPtr->insideGlobalPort = insideGlobalPort;
	tempNaptTcpUdpFlowPtr->protoDel = fromDrv==TRUE? 1: 0;
	tempNaptTcpUdpFlowPtr->alive = 1;
	tempNaptTcpUdpFlowPtr->forAlg = 0;
	tempNaptTcpUdpFlowPtr->canAsic = 1;
	tempNaptTcpUdpFlowPtr->toAsic = 0;
	tempNaptTcpUdpFlowPtr->fromAsic = 0;
	tempNaptTcpUdpFlowPtr->fromDrv = fromDrv==TRUE? 1: 0;
	tempNaptTcpUdpFlowPtr->tcp = (flowType)? 1: 0;
	tempNaptTcpUdpFlowPtr->tcpFlag = flowType-1;
	//tempNaptTcpUdpFlowPtr->age = (isTcp==TRUE)?rtl8651GlobalControl.tcpFastTimeout: rtl8651GlobalControl.udpTimeout;
	if(flowType==TCP_ESTABLISHED_FLOW)
		tempNaptTcpUdpFlowPtr->age = rtl8651GlobalControl.tcpLongTimeout;
	else
		tempNaptTcpUdpFlowPtr->age = (flowType)?rtl8651GlobalControl.tcpFastTimeout: rtl8651GlobalControl.udpTimeout;
	tempNaptTcpUdpFlowPtr->staticEntry = 1;
	if (fromDrv == FALSE)
		rtl8651_tblDrvNaptTcpUdpCounter.protoAddCount++;
	else rtl8651_tblDrvNaptTcpUdpCounter.drvAddCount++;
	
	if(rtl8651NaptTcpUdpBackwardCollisionCountTable[flowTblIdx] > 0)
		return SUCCESS; //Entry is collided means no change to installed to ASIC
	if(rtl8651NaptTcpUdpFlowNumberTable[flowTblIdx] - rtl8651NaptTcpUdpFlowNonAsicNumberTable[flowTblIdx] == 1) {//First can forward by ASIC entry
		//chhuang: to see if collision, the following condition should be taken into consideration
		//	  (a). rtl8651NaptTcpUdpFlowNumberTable[flowTblIdx]>1  OR
		//    (b). rtl8651NaptTcpUdpFlowNonAsicNumberTable[flowTblIdx] > 0  OR
		//    (c). rtl8651NaptTcpUdpBackwardCollisionCountTable[flowTblIdx]>0
		if(rtl8651_setAsicNaptTcpUdpTable_Patch(FALSE, tempNaptTcpUdpFlowPtr->insideLocalIpAddr, tempNaptTcpUdpFlowPtr->insideLocalPort, tempNaptTcpUdpFlowPtr->insideGlobalPort, tempNaptTcpUdpFlowPtr->age, tempNaptTcpUdpFlowPtr->staticEntry==1? TRUE: FALSE, tempNaptTcpUdpFlowPtr->tcp==1? TRUE: FALSE, tempNaptTcpUdpFlowPtr->tcpFlag, rtl8651NaptTcpUdpFlowNumberTable[flowTblIdx]>1 || rtl8651NaptTcpUdpFlowNonAsicNumberTable[flowTblIdx] > 0 || rtl8651NaptTcpUdpBackwardCollisionCountTable[flowTblIdx]>0? TRUE: FALSE, TRUE) == SUCCESS) 
			tempNaptTcpUdpFlowPtr->toAsic = 1;
		else {
			//chhuang: set collison bit
			// NOTE:
			//		It may happen that the entry is immediately timeout after the first try-write. In
			//		such situation, we may consider that the entry is still valid; but in fact, it is
			//		timeout already.
			//		TO SOVE this problem, DO NOT CARE this fact, once a NULL is returned, force write!
			rtl8651_getAsicNaptTcpUdpTable(FALSE, flowTblIdx, &asicLocalIpAddr, &asicLocalPort, &asicGlobalPort, &asicAgeSec, &asicIsStatic, &asicIsTcp, &asicTcpFlag, &asicIsCollision, &asicIsValid);
			if (asicIsValid == TRUE)
				rtl8651_setAsicNaptTcpUdpTable_Patch(TRUE, asicLocalIpAddr, asicLocalPort, asicGlobalPort, asicAgeSec, asicIsStatic, asicIsTcp, asicTcpFlag, TRUE, TRUE);			
			else {
				rtl8651_setAsicNaptTcpUdpTable_Patch(TRUE, tempNaptTcpUdpFlowPtr->insideLocalIpAddr, tempNaptTcpUdpFlowPtr->insideLocalPort, tempNaptTcpUdpFlowPtr->insideGlobalPort, tempNaptTcpUdpFlowPtr->age, tempNaptTcpUdpFlowPtr->staticEntry==1? TRUE: FALSE, tempNaptTcpUdpFlowPtr->tcp==1? TRUE: FALSE, tempNaptTcpUdpFlowPtr->tcpFlag, rtl8651NaptTcpUdpFlowNumberTable[flowTblIdx]>1 || rtl8651NaptTcpUdpFlowNonAsicNumberTable[flowTblIdx] > 0 || rtl8651NaptTcpUdpBackwardCollisionCountTable[flowTblIdx]>0? TRUE: FALSE, TRUE);
				tempNaptTcpUdpFlowPtr->toAsic = 1;
			}
		}
	}
	else if(rtl8651NaptTcpUdpFlowNumberTable[flowTblIdx] == 2) {
		trackNaptTcpUdpFlowPtr = rtl8651NaptTcpUdpFlowTable[flowTblIdx];
		while(trackNaptTcpUdpFlowPtr) {
			if(trackNaptTcpUdpFlowPtr->toAsic == 1)
				break;
			trackNaptTcpUdpFlowPtr = trackNaptTcpUdpFlowPtr->next;
		}
		if(trackNaptTcpUdpFlowPtr) //Entry previous filled
			rtl8651_setAsicNaptTcpUdpTable_Patch(TRUE, trackNaptTcpUdpFlowPtr->insideLocalIpAddr, trackNaptTcpUdpFlowPtr->insideLocalPort, trackNaptTcpUdpFlowPtr->insideGlobalPort, trackNaptTcpUdpFlowPtr->age, tempNaptTcpUdpFlowPtr->staticEntry==1? TRUE: FALSE, trackNaptTcpUdpFlowPtr->tcp==1? TRUE: FALSE, trackNaptTcpUdpFlowPtr->tcpFlag , rtl8651NaptTcpUdpFlowNumberTable[flowTblIdx]>1 || rtl8651NaptTcpUdpFlowNonAsicNumberTable[flowTblIdx] > 0 || rtl8651NaptTcpUdpBackwardCollisionCountTable[flowTblIdx]>0? TRUE: FALSE, TRUE);			
	}
	return SUCCESS;//Entry either write to ASIC or update ASIC information
}

static int32 _rtl8651_addNaptTcpUdpNonCandidateFlow(int8 flowType, ipaddr_t insideLocalIpAddr, uint16 insideLocalPort, 
	ipaddr_t insideGlobalIpAddr, uint16 insideGlobalPort, ipaddr_t dstIpAddr, uint16 dstPort) {
	uint32 flowTblIdx;
	ipaddr_t asicLocalIpAddr;
	uint16 asicLocalPort, asicGlobalPort, asicAgeSec;
	int8 asicIsStatic, asicIsTcp, asicTcpFlag, asicIsCollision, asicIsValid;
	rtl8651_tblDrv_naptTcpUdpFlowEntry_t * tempNaptTcpUdpFlowPtr, * asicNaptTcpUdpFlowPtr;
	rtl8651_tblDrv_rsvPortNumber_t *tmpRsvPort;
	
	flowTblIdx = rtl8651_naptTcpUdpTableIndex((flowType==FALSE? FALSE: TRUE), insideLocalIpAddr, insideLocalPort, dstIpAddr, dstPort);
	/* Check if user specified port is reserved */
	for(tmpRsvPort=rtl8651RsvPortList; tmpRsvPort; tmpRsvPort=tmpRsvPort->next)
		if (tmpRsvPort->port == insideGlobalPort) {
			rtl8651_tblDrvNaptTcpUdpCounter.addFailure++;
			return(TBLDRV_EINVALIDEXTPORT);
		}
	/* Check if user specified port is in use */
	tempNaptTcpUdpFlowPtr = rtl8651NaptTcpUdpFlowTable[flowTblIdx];
	for(;tempNaptTcpUdpFlowPtr; tempNaptTcpUdpFlowPtr=tempNaptTcpUdpFlowPtr->next)
		if (tempNaptTcpUdpFlowPtr->insideGlobalPort == insideGlobalPort) {
			return(TBLDRV_EINVALIDEXTPORT);
		}
	if(rtl8651FreeNaptTcpUdpFlowEntryList == NULL) {
		rtl8651_tblDrvNaptTcpUdpCounter.addFailure++;
		return FAILED;//No free flow entry available
	}
	tempNaptTcpUdpFlowPtr = rtl8651FreeNaptTcpUdpFlowEntryList;
	rtl8651FreeNaptTcpUdpFlowEntryList = tempNaptTcpUdpFlowPtr->next;
	tempNaptTcpUdpFlowPtr->next = rtl8651NaptTcpUdpFlowTable[flowTblIdx];
	rtl8651NaptTcpUdpFlowTable[flowTblIdx] = tempNaptTcpUdpFlowPtr;
	rtl8651NaptTcpUdpFlowNonAsicNumberTable[flowTblIdx]++;
	rtl8651NaptTcpUdpFlowNumberTable[flowTblIdx]++;
	rtl8651ExistedNaptTcpUdpFlows++;
	tempNaptTcpUdpFlowPtr->dstIpAddr = dstIpAddr;
	tempNaptTcpUdpFlowPtr->dstPort = dstPort;
	tempNaptTcpUdpFlowPtr->insideLocalIpAddr = insideLocalIpAddr;
	tempNaptTcpUdpFlowPtr->insideLocalPort = insideLocalPort;
	tempNaptTcpUdpFlowPtr->insideGlobalIpAddr = insideGlobalIpAddr;
	tempNaptTcpUdpFlowPtr->insideGlobalPort = insideGlobalPort;
	tempNaptTcpUdpFlowPtr->protoDel = 0;
	tempNaptTcpUdpFlowPtr->alive = 1;
	tempNaptTcpUdpFlowPtr->forAlg = 0;
	tempNaptTcpUdpFlowPtr->canAsic = 0;
	tempNaptTcpUdpFlowPtr->toAsic = 0;
	tempNaptTcpUdpFlowPtr->fromDrv = 0;
	tempNaptTcpUdpFlowPtr->fromAsic = 0;
	tempNaptTcpUdpFlowPtr->tcp = (flowType)? 1: 0;
	tempNaptTcpUdpFlowPtr->tcpFlag = flowType-1;
	//tempNaptTcpUdpFlowPtr->age = (isTcp==TRUE)?rtl8651GlobalControl.tcpFastTimeout: rtl8651GlobalControl.udpTimeout;
	if(flowType==TCP_ESTABLISHED_FLOW)
		tempNaptTcpUdpFlowPtr->age = rtl8651GlobalControl.tcpLongTimeout;
	else
		tempNaptTcpUdpFlowPtr->age = (flowType)?rtl8651GlobalControl.tcpFastTimeout: rtl8651GlobalControl.udpTimeout;
	tempNaptTcpUdpFlowPtr->staticEntry = 1;
	rtl8651_tblDrvNaptTcpUdpCounter.diffHashCount++;
	rtl8651_tblDrvNaptTcpUdpCounter.protoAddCount++;
		
	if(rtl8651NaptTcpUdpBackwardCollisionCountTable[flowTblIdx] == 0) {//Only when non-backward collided need to fill collision bit
		asicNaptTcpUdpFlowPtr = rtl8651NaptTcpUdpFlowTable[flowTblIdx];
		while(asicNaptTcpUdpFlowPtr) {
			if(asicNaptTcpUdpFlowPtr->toAsic == 1)
				break;
			asicNaptTcpUdpFlowPtr = asicNaptTcpUdpFlowPtr->next;
		}
		if(asicNaptTcpUdpFlowPtr)
			rtl8651_setAsicNaptTcpUdpTable_Patch(TRUE, asicNaptTcpUdpFlowPtr->insideLocalIpAddr, asicNaptTcpUdpFlowPtr->insideLocalPort, asicNaptTcpUdpFlowPtr->insideGlobalPort, asicNaptTcpUdpFlowPtr->age, asicNaptTcpUdpFlowPtr->staticEntry==1? TRUE: FALSE, asicNaptTcpUdpFlowPtr->tcp==1? TRUE: FALSE, asicNaptTcpUdpFlowPtr->tcpFlag, rtl8651NaptTcpUdpFlowNumberTable[flowTblIdx]>1 || rtl8651NaptTcpUdpFlowNonAsicNumberTable[flowTblIdx] > 0 || rtl8651NaptTcpUdpBackwardCollisionCountTable[flowTblIdx]>0? TRUE: FALSE, TRUE);
		else {
			if(rtl8651_setAsicNaptTcpUdpTable_Patch(FALSE, 0, 0, flowTblIdx, 0, FALSE, FALSE, 0, TRUE, FALSE) == FAILED) {
				//asicNaptTcpUdpFlowPtr = _rtl8651_getAsicNaptTcpUdpDynamicFlowToTable(flowTblIdx);
				rtl8651_getAsicNaptTcpUdpTable(FALSE, flowTblIdx, &asicLocalIpAddr, &asicLocalPort, &asicGlobalPort, &asicAgeSec, &asicIsStatic, &asicIsTcp, &asicTcpFlag, &asicIsCollision, &asicIsValid);
				if(asicIsValid == TRUE)
					rtl8651_setAsicNaptTcpUdpTable_Patch(TRUE, asicLocalIpAddr, asicLocalPort, asicGlobalPort, asicAgeSec, asicIsStatic, asicIsTcp, asicTcpFlag, TRUE, TRUE);
				else //If unable to write normally and unable to get valid ASIC entry, just force write to collide the entry
					rtl8651_setAsicNaptTcpUdpTable_Patch(TRUE, 0, 0, flowTblIdx, 0, FALSE, FALSE, 0, TRUE, FALSE);
			}
		} 
	}
	flowTblIdx = insideGlobalPort & (RTL8651_TCPUDPTBL_SIZE-1);
	rtl8651NaptTcpUdpBackwardCollisionCountTable[flowTblIdx]++;
	if(rtl8651_setAsicNaptTcpUdpTable_Patch(FALSE, 0, 0, flowTblIdx, 0, FALSE, FALSE, 0, TRUE, FALSE) == FAILED) {
		asicNaptTcpUdpFlowPtr = _rtl8651_getAsicNaptTcpUdpDynamicFlowToTable(flowTblIdx);
		/* Clear the entry in the ASIC table */
		rtl8651_setAsicNaptTcpUdpTable_Patch(TRUE, 0, 0, flowTblIdx, 0, FALSE, FALSE, 0, TRUE, FALSE);
	}
	return SUCCESS;
}

int32 rtl8651_addNaptTcpUdpFlow(int8 assigned, int8 flowType, ipaddr_t insideLocalIpAddr, uint16 insideLocalPort, 
			ipaddr_t *insideGlobalIpAddr, uint16 *insideGlobalPort, ipaddr_t dstIpAddr, uint16 dstPort) {
	rtl8651_tblDrv_naptTcpUdpFlowEntry_t * tempNaptTcpUdpFlowPtr;
	uint32 flowTblIdx, result;
	uint16 offset;
	
	if(insideGlobalIpAddr == NULL || insideGlobalPort == NULL)
		return TBLDRV_EINVVAL;//Must have Global IP address and Global port for storage
	
	RTL8651_LOCK_RESOURCE();//Lock resource
	if(_rtl8651_intNetIpAddr(insideLocalIpAddr) == FALSE) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource 
		return TBLDRV_ENOTINTIP;//Inside local ip address does not belong to configured network interface
	}
	if(assigned == TRUE && _rtl8651_extIpAddr(*insideGlobalIpAddr, NULL) == FALSE) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return TBLDRV_ENOTEXTIP;//Inside global ip address does not belong to external IP interface IP addresses
	}
	if(_rtl8651_localServerIpAddr(dstIpAddr) == TRUE) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource	 
		return TBLDRV_EDSTIPISLS;//Local server should not trigger NAT operation
	}
	if(rtl8651FreeNaptTcpUdpFlowEntryList == NULL) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return TBLDRV_ENOFREEBUF;//No free flow entry available
	}
	flowTblIdx = rtl8651_naptTcpUdpTableIndex((flowType==FALSE? FALSE: TRUE), insideLocalIpAddr, insideLocalPort, dstIpAddr, dstPort);
	if(assigned == FALSE) {
		/*
		 * Protocol stack uses the ASIC's hash Algorithm, hence, the external 
		 * port and external IP address should be picked up by table driver.
		 */
		if(_rtl8651_selectNaptTcpUdpFlowOffset(flowTblIdx, &offset) == FAILED) {//Unable to get valid offset
			RTL8651_UNLOCK_RESOURCE();//Unlock resource 
			return TBLDRV_EGETOFFSETFAIL;
		}
		*insideGlobalPort = offset<<10 | flowTblIdx;
		*insideGlobalIpAddr = rtl8651_getNaptInsideGlobalIpAddr((flowType==FALSE? FALSE: TRUE), insideLocalIpAddr, insideLocalPort, dstIpAddr, dstPort);
	}
	/*
	 * NOTE:
	 *	We assume that user-added entry is not the same as the ASIC auto-learn. Because
	 *  if entry is added by ASIC auto-learn, the packet will not be trapped to CPU.
	 *  If this happened, it means something wrong!!
	 */
	if((tempNaptTcpUdpFlowPtr = _rtl8651_getNaptTcpUdpFlowEntry(flowTblIdx, (flowType==FALSE? FALSE: TRUE), insideLocalIpAddr, insideLocalPort))) {
		/*
		 * After protocol stack removing and Before entry aging out, if protocol stack
		 * adds the same entry again, two cases should be taken care:
		 *   (1) the entry is TCP flow:
		 *		 for this kind of flow, we do not modify its status and aging time
		 *	 (2) the entry is UDP flow:
		 *		 for this kind of flow, we shall refresh its aging time
		 */
		if (tempNaptTcpUdpFlowPtr->protoDel == 1 && tempNaptTcpUdpFlowPtr->age > 0) {
			if (tempNaptTcpUdpFlowPtr->tcp == 0) {
				/* UDP flow, refresh its aging time */
				tempNaptTcpUdpFlowPtr->age = rtl8651GlobalControl.udpTimeout;
				if (tempNaptTcpUdpFlowPtr->toAsic == 1)
					/* If this entry is in the ASIC table, reset it */
					rtl8651_setAsicNaptTcpUdpTable_Patch(TRUE, tempNaptTcpUdpFlowPtr->insideLocalIpAddr, tempNaptTcpUdpFlowPtr->insideLocalPort, tempNaptTcpUdpFlowPtr->insideGlobalPort, tempNaptTcpUdpFlowPtr->age, tempNaptTcpUdpFlowPtr->staticEntry==1? TRUE: FALSE, tempNaptTcpUdpFlowPtr->tcp==1? TRUE: FALSE, tempNaptTcpUdpFlowPtr->tcpFlag , rtl8651NaptTcpUdpFlowNumberTable[flowTblIdx]>1 || rtl8651NaptTcpUdpFlowNonAsicNumberTable[flowTblIdx] > 0 || rtl8651NaptTcpUdpBackwardCollisionCountTable[flowTblIdx]>0? TRUE: FALSE, TRUE);		
			}
			*insideGlobalPort = tempNaptTcpUdpFlowPtr->insideGlobalPort;
			*insideGlobalIpAddr = tempNaptTcpUdpFlowPtr->insideLocalIpAddr;
			tempNaptTcpUdpFlowPtr->protoDel = 0;
			rtl8651_tblDrvNaptTcpUdpCounter.protoAddCount++;
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return SUCCESS;
		}
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return FAILED;//Entry duplicated
	}
	
	if(flowTblIdx != (*insideGlobalPort & (RTL8651_TCPUDPTBL_SIZE-1)) || *insideGlobalIpAddr != rtl8651_getNaptInsideGlobalIpAddr((flowType==FALSE? FALSE: TRUE), insideLocalIpAddr, insideLocalPort, dstIpAddr, dstPort))
		/* The hashing result differs from that of ASIC */
		result = _rtl8651_addNaptTcpUdpNonCandidateFlow(flowType, insideLocalIpAddr, insideLocalPort, *insideGlobalIpAddr, *insideGlobalPort, dstIpAddr, dstPort);
	else
		/* The hashing result is the same as that of ASIC */
		result = _rtl8651_addNaptTcpUdpCandidateFlow(FALSE, flowType, insideLocalIpAddr, insideLocalPort, *insideGlobalIpAddr, *insideGlobalPort, dstIpAddr, dstPort);
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return result;
}


/*
 * For some NAPT flows such as ftp control channel, the forwarding engine will always 
 * take care of their forwarding. Hence it is no needed to write such flows into ASIC
 * table for ASIC processing.
 */
int8 _rtl8651_addAlgNaptTcpUdpFlow(int8 flowType, ipaddr_t insideLocalIpAddr, uint16 insideLocalPort, 
		ipaddr_t *insideGlobalIpAddr, uint16 *insideGlobalPort, ipaddr_t dstIpAddr, uint16 dstPort) {
	rtl8651_tblDrv_naptTcpUdpFlowEntry_t * tempNaptTcpUdpFlowPtr;
	uint32 flowTblIdx;
	uint16 offset;

	if(insideGlobalIpAddr == NULL || insideGlobalPort == NULL)
		return FAILED;
	if(_rtl8651_intNetIpAddr(insideLocalIpAddr) == FALSE)
		return FAILED;
	if(_rtl8651_localServerIpAddr(dstIpAddr) == TRUE) 
		return FAILED;
	if(rtl8651FreeNaptTcpUdpFlowEntryList == NULL)
		return FAILED;
	flowTblIdx = rtl8651_naptTcpUdpTableIndex((flowType==FALSE? FALSE: TRUE), insideLocalIpAddr, insideLocalPort, dstIpAddr, dstPort);
	if((tempNaptTcpUdpFlowPtr = _rtl8651_getNaptTcpUdpFlowEntry(flowTblIdx, (flowType==FALSE? FALSE: TRUE), insideLocalIpAddr, insideLocalPort)))
		return FAILED; /* Duplicate found */
	/* Select an usable external IP address and external port */
	if(_rtl8651_selectNaptTcpUdpFlowOffset(flowTblIdx, &offset) == FAILED) 
		return FAILED;
	*insideGlobalPort = offset<<10 | flowTblIdx;
	*insideGlobalIpAddr = rtl8651_getNaptInsideGlobalIpAddr((flowType==FALSE? FALSE: TRUE), insideLocalIpAddr, insideLocalPort, dstIpAddr, dstPort);
	
	tempNaptTcpUdpFlowPtr = rtl8651FreeNaptTcpUdpFlowEntryList;
	rtl8651FreeNaptTcpUdpFlowEntryList = tempNaptTcpUdpFlowPtr->next;
	tempNaptTcpUdpFlowPtr->next = rtl8651NaptTcpUdpFlowTable[flowTblIdx];
	rtl8651NaptTcpUdpFlowTable[flowTblIdx] = tempNaptTcpUdpFlowPtr;
	rtl8651ExistedNaptTcpUdpFlows++;
	tempNaptTcpUdpFlowPtr->dstIpAddr = dstIpAddr;
	tempNaptTcpUdpFlowPtr->dstPort = dstPort;
	tempNaptTcpUdpFlowPtr->insideLocalIpAddr = insideLocalIpAddr;
	tempNaptTcpUdpFlowPtr->insideLocalPort = insideLocalPort;
	tempNaptTcpUdpFlowPtr->insideGlobalIpAddr = *insideGlobalIpAddr;
	tempNaptTcpUdpFlowPtr->insideGlobalPort = *insideGlobalPort;
	tempNaptTcpUdpFlowPtr->protoDel = 1;
	tempNaptTcpUdpFlowPtr->alive = 1;
	tempNaptTcpUdpFlowPtr->forAlg = 1;
	tempNaptTcpUdpFlowPtr->canAsic = 0;
	tempNaptTcpUdpFlowPtr->toAsic = 0;
	tempNaptTcpUdpFlowPtr->fromAsic = 0;
	tempNaptTcpUdpFlowPtr->fromDrv = 1;
	tempNaptTcpUdpFlowPtr->tcp = (flowType)? 1: 0;
	tempNaptTcpUdpFlowPtr->tcpFlag = flowType-1;
	if(flowType==TCP_ESTABLISHED_FLOW)
		tempNaptTcpUdpFlowPtr->age = rtl8651GlobalControl.tcpLongTimeout;
	else
		tempNaptTcpUdpFlowPtr->age = (flowType)?rtl8651GlobalControl.tcpFastTimeout: rtl8651GlobalControl.udpTimeout;
	tempNaptTcpUdpFlowPtr->staticEntry = 1;
	rtl8651_tblDrvNaptTcpUdpCounter.drvAddCount++;
	return SUCCESS;	
}


static void _rtl8651_refillNaptTcpUdpFlowOccupiedEntry(uint32 flowTblIdx, rtl8651_tblDrv_naptTcpUdpFlowEntry_t *delEntry) {
	rtl8651_tblDrv_naptTcpUdpFlowEntry_t * trackNaptTcpUdpFlowPtr, * asicNaptTcpUdpFlowPtr;	
	ipaddr_t asicLocalIpAddr;
	uint16 asicLocalPort, asicGlobalPort, asicAgeSec ;
	int8 asicIsStatic, asicIsTcp, asicTcpFlag, asicIsCollision, asicIsValid, drvToAsic = FALSE;

	if (rtl8651NaptTcpUdpBackwardCollisionCountTable[flowTblIdx] > 0)
		return; // chhuang: bwcoll count>0=>nothing to do
	asicNaptTcpUdpFlowPtr = NULL;//Find candidate entry to fill into ASIC
	if(rtl8651NaptTcpUdpFlowNumberTable[flowTblIdx] - rtl8651NaptTcpUdpFlowNonAsicNumberTable[flowTblIdx] > 0) {
		trackNaptTcpUdpFlowPtr = rtl8651NaptTcpUdpFlowTable[flowTblIdx];
		while(trackNaptTcpUdpFlowPtr) {//Count entry number
			if(trackNaptTcpUdpFlowPtr->canAsic == 1 && trackNaptTcpUdpFlowPtr->age > 0 && trackNaptTcpUdpFlowPtr->forAlg == 0) {
				if(asicNaptTcpUdpFlowPtr == NULL)
					asicNaptTcpUdpFlowPtr = trackNaptTcpUdpFlowPtr;
				else if(trackNaptTcpUdpFlowPtr->fromAsic > asicNaptTcpUdpFlowPtr->fromAsic)
					asicNaptTcpUdpFlowPtr = trackNaptTcpUdpFlowPtr;
				else if (trackNaptTcpUdpFlowPtr->fromAsic ==  asicNaptTcpUdpFlowPtr->fromAsic && trackNaptTcpUdpFlowPtr->fromDrv > asicNaptTcpUdpFlowPtr->fromDrv)
					asicNaptTcpUdpFlowPtr = trackNaptTcpUdpFlowPtr;
			}
			if (trackNaptTcpUdpFlowPtr->toAsic == 1)
				drvToAsic = TRUE;
			trackNaptTcpUdpFlowPtr = trackNaptTcpUdpFlowPtr->next;
		}
	}
	if (delEntry != NULL) {
		rtl8651_getAsicNaptTcpUdpTable(FALSE, flowTblIdx, &asicLocalIpAddr, &asicLocalPort, &asicGlobalPort, &asicAgeSec, &asicIsStatic, &asicIsTcp, &asicTcpFlag, &asicIsCollision, &asicIsValid);
		if (asicIsValid == TRUE) {
			if (delEntry->tcp != (asicIsTcp==TRUE? 1: 0) ||
				delEntry->insideLocalIpAddr != asicLocalIpAddr ||
				delEntry->insideLocalPort != asicLocalPort ||
				delEntry->insideGlobalPort != asicGlobalPort) {
					rtl8651_setAsicNaptTcpUdpTable_Patch(TRUE, asicLocalIpAddr, asicLocalPort, asicGlobalPort, asicAgeSec, asicIsStatic, asicIsTcp, asicTcpFlag, (rtl8651NaptTcpUdpFlowNumberTable[flowTblIdx]>0&&drvToAsic==FALSE) || (rtl8651NaptTcpUdpFlowNumberTable[flowTblIdx]>1&&drvToAsic==TRUE) || rtl8651NaptTcpUdpFlowNonAsicNumberTable[flowTblIdx] > 0 || rtl8651NaptTcpUdpBackwardCollisionCountTable[flowTblIdx]>0? TRUE: FALSE, TRUE);
				return;
			}
		}
	}

	if(asicNaptTcpUdpFlowPtr) {
		assert(asicNaptTcpUdpFlowPtr->canAsic == 1);
		rtl8651_setAsicNaptTcpUdpTable_Patch(TRUE, asicNaptTcpUdpFlowPtr->insideLocalIpAddr, asicNaptTcpUdpFlowPtr->insideLocalPort, asicNaptTcpUdpFlowPtr->insideGlobalPort, asicNaptTcpUdpFlowPtr->age, asicNaptTcpUdpFlowPtr->staticEntry==1? TRUE: FALSE, asicNaptTcpUdpFlowPtr->tcp==1? TRUE: FALSE, asicNaptTcpUdpFlowPtr->tcpFlag , rtl8651NaptTcpUdpFlowNumberTable[flowTblIdx]>1 || rtl8651NaptTcpUdpFlowNonAsicNumberTable[flowTblIdx] > 0 || rtl8651NaptTcpUdpBackwardCollisionCountTable[flowTblIdx]>0? TRUE: FALSE, TRUE);
		/*
		 * If the entry is from ASIC, write it into ASIC and remove it from driver table
		 */
		if (asicNaptTcpUdpFlowPtr->fromAsic == 1)
			_rtl8651_delNaptTcpUdpFlowEntry(flowTblIdx, asicNaptTcpUdpFlowPtr);
	 	else asicNaptTcpUdpFlowPtr->toAsic = 1;
	 	rtl8651_tblDrvNaptTcpUdpCounter.threadWriteBackCount++;
		//asicNaptTcpUdpFlowPtr->toAsic = 1;
	} else rtl8651_setAsicNaptTcpUdpTable_Patch(TRUE, 0, 0, flowTblIdx, 0, FALSE, FALSE, 0, rtl8651NaptTcpUdpFlowNumberTable[flowTblIdx]>1 || rtl8651NaptTcpUdpFlowNonAsicNumberTable[flowTblIdx] > 0 || rtl8651NaptTcpUdpBackwardCollisionCountTable[flowTblIdx]>0? TRUE: FALSE, FALSE);
}

static int32 _rtl8651_delNaptTcpUdpFlow(uint32 flowTblIdx, rtl8651_tblDrv_naptTcpUdpFlowEntry_t *delEntry) {
	rtl8651_tblDrv_naptTcpUdpFlowEntry_t *tmpDrvEntry;
	ipaddr_t asicLocalIpAddr;
	uint16 asicLocalPort, asicGlobalPort, asicAgeSec;
	int8 asicIsStatic, asicIsTcp, asicTcpFlag, asicIsCollision, asicIsValid;
	
	if (delEntry == (rtl8651_tblDrv_naptTcpUdpFlowEntry_t *)0)
		return FAILED;
	
	_rtl8651_delNaptTcpUdpFlowEntry(flowTblIdx, delEntry);
	rtl8651_tblDrvNaptTcpUdpCounter.drvDelCount++;
	/*
	 * If the 'forAlg' flag was turned on, the only thing to do is to remove
	 * the entry from the driver table because this kind of entry is never 
	 * written into ASIC table.
	 */
	if (delEntry->forAlg == 1)
		return SUCCESS;
	
	if(delEntry->toAsic == 1) {
		assert(delEntry->canAsic == 1);
		assert(rtl8651NaptTcpUdpBackwardCollisionCountTable[flowTblIdx] == 0);
	} 

	if(delEntry->toAsic == 1 || delEntry->canAsic==0)
		_rtl8651_refillNaptTcpUdpFlowOccupiedEntry(flowTblIdx, delEntry);
	else {
		/*
		 * Otherwise, the deleted entry is not written to ASIC. Beside remove it from
		 * internal structure, it is also needed to update the collison bit in the ASIC 
		 * table.
		 * NOTE:
		 *		In this case, we assume there is an entry in the ASIC table. Hence try
		 *		to read ASIC table and update its collison bit. If we find that the 
		 *		ASIC table entry is invalid, just do nothing. Because the maintenance
		 *		thread will take care of writting entry in the driver back to the ASIC table.
		 */
		tmpDrvEntry = rtl8651NaptTcpUdpFlowTable[flowTblIdx];
		while(tmpDrvEntry && tmpDrvEntry->toAsic != 1)
			tmpDrvEntry = tmpDrvEntry->next;
		rtl8651_getAsicNaptTcpUdpTable(FALSE, flowTblIdx, &asicLocalIpAddr, &asicLocalPort, &asicGlobalPort, &asicAgeSec, &asicIsStatic, &asicIsTcp, &asicTcpFlag, &asicIsCollision, &asicIsValid);
		if (asicIsValid == TRUE) {
			if (tmpDrvEntry)
				rtl8651_setAsicNaptTcpUdpTable_Patch(TRUE, asicLocalIpAddr, asicLocalPort, asicGlobalPort, asicAgeSec, asicIsStatic, asicIsTcp, asicTcpFlag, rtl8651NaptTcpUdpFlowNumberTable[flowTblIdx]>1 || rtl8651NaptTcpUdpFlowNonAsicNumberTable[flowTblIdx] > 0 || rtl8651NaptTcpUdpBackwardCollisionCountTable[flowTblIdx]>0? TRUE: FALSE, TRUE);
			else 
				rtl8651_setAsicNaptTcpUdpTable_Patch(TRUE, asicLocalIpAddr, asicLocalPort, asicGlobalPort, asicAgeSec, asicIsStatic, asicIsTcp, asicTcpFlag, rtl8651NaptTcpUdpFlowNumberTable[flowTblIdx]>0 || rtl8651NaptTcpUdpFlowNonAsicNumberTable[flowTblIdx] > 0 || rtl8651NaptTcpUdpBackwardCollisionCountTable[flowTblIdx]>0? TRUE: FALSE, TRUE);	
		}			
	}		
	if(delEntry->canAsic==0) {//This flow will backward collision other entry, process the collided flow entry
		assert(delEntry->toAsic == 0);
		flowTblIdx = delEntry->insideGlobalPort & (RTL8651_TCPUDPTBL_SIZE-1);
		rtl8651NaptTcpUdpBackwardCollisionCountTable[flowTblIdx]--;
		if(rtl8651NaptTcpUdpBackwardCollisionCountTable[flowTblIdx] == 0) 
			_rtl8651_refillNaptTcpUdpFlowOccupiedEntry(flowTblIdx, NULL);
	}
	return SUCCESS;
}


int32 rtl8651_delNaptTcpUdpFlow(int8 isTcp, ipaddr_t insideLocalIpAddr, 
		uint16 insideLocalPort, ipaddr_t dstIpAddr, uint16 dstPort) {
	uint32 flowTblIdx;
	rtl8651_tblDrv_naptTcpUdpFlowEntry_t * deletedEntry, * preDelEntry;
	
	RTL8651_LOCK_RESOURCE();//Lock resource
	 
	if(_rtl8651_intNetIpAddr(insideLocalIpAddr) == FALSE) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return TBLDRV_ENOTINTIP;//Local ip address not internal IP address
	}
	if(_rtl8651_localServerIpAddr(dstIpAddr) == TRUE) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return TBLDRV_EDSTIPISLS;
	}
	
	flowTblIdx = rtl8651_naptTcpUdpTableIndex(isTcp, insideLocalIpAddr, insideLocalPort, dstIpAddr, dstPort);
	deletedEntry = rtl8651NaptTcpUdpFlowTable[flowTblIdx];
	for(preDelEntry=NULL; deletedEntry; preDelEntry=deletedEntry, deletedEntry=deletedEntry->next) {
		if (deletedEntry->fromAsic == 0 && deletedEntry->fromDrv == 0 &&
			deletedEntry->tcp ==(isTcp==TRUE? 1: 0) &&
			deletedEntry->insideLocalIpAddr == insideLocalIpAddr &&
			deletedEntry->insideLocalPort == insideLocalPort &&
			deletedEntry->dstIpAddr == dstIpAddr &&
			deletedEntry->dstPort == dstPort)
				break;
	}
	if(deletedEntry == NULL) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return TBLDRV_EENTRYNOTFOUND;//Entry to be deleted is not found
	}
	/*
	 * If the entry was already removed by protocol stack, just return FAILED
	 * to indicate no entry found.
	 */
	if (deletedEntry->protoDel == 1) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return TBLDRV_EENTRYNOTFOUND;//Entry to be deleted is not found
	}
	//tempNaptTcpUdpFlowPtr->protoDel = 1;
	//if (tempNaptTcpUdpFlowPtr->age == 0)
	//	_rtl8651_delNaptTcpUdpFlow(flowTblIdx, tempNaptTcpUdpFlowPtr);
	deletedEntry->protoDel = 1;
	if (deletedEntry->age == 0) {
		/* Only remove the entry from driver table */
		if (preDelEntry == NULL)
			rtl8651NaptTcpUdpFlowTable[flowTblIdx] = deletedEntry->next;
		else preDelEntry->next = deletedEntry->next;
		deletedEntry->next = rtl8651FreeNaptTcpUdpFlowEntryList;
		rtl8651FreeNaptTcpUdpFlowEntryList = deletedEntry;
		rtl8651ExistedNaptTcpUdpFlows--;
	}
	rtl8651_tblDrvNaptTcpUdpCounter.protoDelCount++;
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}


int8 _rtl8651_delAlgNaptTcpUdpFlow(int8 isTcp, ipaddr_t insideLocalIpAddr, 
		uint16 insideLocalPort, ipaddr_t dstIpAddr, uint16 dstPort) {
	rtl8651_tblDrv_naptTcpUdpFlowEntry_t * trackNaptPtr;
	uint32 flowTblIdx;

	if(_rtl8651_intNetIpAddr(insideLocalIpAddr) == FALSE)
		return FAILED;
	if(_rtl8651_localServerIpAddr(dstIpAddr) == TRUE)
		return FAILED;
	flowTblIdx = rtl8651_naptTcpUdpTableIndex(isTcp, insideLocalIpAddr, insideLocalPort, dstIpAddr, dstPort);
	trackNaptPtr = rtl8651NaptTcpUdpFlowTable[flowTblIdx];
	for(; trackNaptPtr; trackNaptPtr=trackNaptPtr->next)
		if (trackNaptPtr->fromDrv == 1 && trackNaptPtr->forAlg == 1 &&
			trackNaptPtr->tcp == (isTcp==TRUE? 1: 0) &&
			trackNaptPtr->insideLocalIpAddr == insideLocalIpAddr &&
			trackNaptPtr->insideLocalPort == insideLocalPort &&
			trackNaptPtr->dstIpAddr == dstIpAddr &&
			trackNaptPtr->dstPort == dstPort) {
			return (int8)_rtl8651_delNaptTcpUdpFlowEntry(flowTblIdx, trackNaptPtr);
		}
	return FAILED;
}




//This API try to take advantage of ASIC acceleration capability
void rtl8651_updateAsicNaptTcpUdpTable(void) {
	rtl8651_tblDrv_naptTcpUdpFlowEntry_t *tmpNaptPtr, *delNaptPtr;
	static uint32 flowTblIdx;
	ipaddr_t asicLocalIpAddr;
	uint16 asicLocalPort, asicGlobalPort, asicAgeSec;
	int8 asicIsStatic, asicIsTcp, asicTcpFlag;
	int8 isAsicCollision, isAsicValid;

	 
	RTL8651_LOCK_RESOURCE();//Lock resource
	flowTblIdx = ((flowTblIdx+1)&(RTL8651_TCPUDPTBL_SIZE-1));
	rtl8651_getAsicNaptTcpUdpTable(FALSE, flowTblIdx, &asicLocalIpAddr, &asicLocalPort, &asicGlobalPort, &asicAgeSec, &asicIsStatic, &asicIsTcp, &asicTcpFlag, &isAsicCollision, &isAsicValid);
	/*
	 * If the ASIC table entry is invalid, try to write an entry in the driver 
	 * table to the ASIC table.
	 * NOTE:
	 *		If the write back entry is a TCP session, the tcpFlag should be written back
	 */
	if(isAsicCollision == TRUE && isAsicValid == FALSE)
		_rtl8651_refillNaptTcpUdpFlowOccupiedEntry(flowTblIdx, NULL);
	else if (isAsicValid == TRUE) {
		/*
		 * If any entry is a dynamic entry, Sync entry in driver table and ASIC table
		 */
		tmpNaptPtr = rtl8651NaptTcpUdpFlowTable[flowTblIdx];
		while (tmpNaptPtr) {
			if (tmpNaptPtr->toAsic == 1) {
				if (tmpNaptPtr->tcp == (asicIsTcp==TRUE? 1: 0) && 
					tmpNaptPtr->insideLocalIpAddr == asicLocalIpAddr &&
					tmpNaptPtr->insideLocalPort == asicLocalPort &&
					tmpNaptPtr->insideGlobalPort == asicGlobalPort) {
					/* If entry match, SYNC. */
					tmpNaptPtr->age = asicAgeSec;
					tmpNaptPtr->tcpFlag = asicTcpFlag;
#ifndef CYGWIN 
					//if (tmpNaptPtr->age < 10)
					//	printf("update aging time to: %d\n", tmpNaptPtr->age);
#endif
				} else {
					/* Otherwise, the entry in the ASIC is aging out */
					tmpNaptPtr->age = 0;
					delNaptPtr = tmpNaptPtr;
					tmpNaptPtr = tmpNaptPtr->next;
					_rtl8651_delNaptTcpUdpFlow(flowTblIdx, delNaptPtr);
					delNaptPtr->toAsic = 0;
					rtl8651_tblDrvNaptTcpUdpCounter.syncFailure++;
					continue;
				}
			}
			tmpNaptPtr = tmpNaptPtr->next;
		}
	}
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
}


static void _rtl8651_timeUpdateNaptTcpUdpTable(uint32 secPassed) {
	uint32 flowTblIdx;
	rtl8651_tblDrv_naptTcpUdpFlowEntry_t * trackNaptTcpUdpFlowPtr, * deletedNaptTcpUdpFlowPtr;

	for(flowTblIdx = 0; flowTblIdx < RTL8651_TCPUDPTBL_SIZE; flowTblIdx++) {
		trackNaptTcpUdpFlowPtr = rtl8651NaptTcpUdpFlowTable[flowTblIdx];
		while(trackNaptTcpUdpFlowPtr) {
			assert(!(trackNaptTcpUdpFlowPtr->canAsic==0 && trackNaptTcpUdpFlowPtr->toAsic==1));
			/* If the entry is already aging out and wait for protocol removing, IGNORE ! */
			//if (trackNaptTcpUdpFlowPtr->age == 0 && trackNaptTcpUdpFlowPtr->protoDel == 0) {
			if (trackNaptTcpUdpFlowPtr->alive == 0) {
				trackNaptTcpUdpFlowPtr = trackNaptTcpUdpFlowPtr->next;
				continue;
			}	
			/* Entry is in aging state, COUNTING!  */
			if (trackNaptTcpUdpFlowPtr->age > secPassed) {
				trackNaptTcpUdpFlowPtr->age -= secPassed;
				deletedNaptTcpUdpFlowPtr = NULL;
			} else {
				trackNaptTcpUdpFlowPtr->age = 0;
				deletedNaptTcpUdpFlowPtr = trackNaptTcpUdpFlowPtr;
#if 0
#ifndef CYGWIN				
printf("(%s) sip(%d.%d.%d.%d) sport(%d), dip(%d.%d.%d.%d) dport(%d) time out...\n", trackNaptTcpUdpFlowPtr->tcp == TRUE? "TCP": "UDP",
	trackNaptTcpUdpFlowPtr->insideLocalIpAddr>>24, (trackNaptTcpUdpFlowPtr->insideLocalIpAddr&0x00ff0000)>>16,
	(trackNaptTcpUdpFlowPtr->insideLocalIpAddr&0x0000ff00)>>8, trackNaptTcpUdpFlowPtr->insideLocalIpAddr&0xff,
	trackNaptTcpUdpFlowPtr->insideLocalPort,
	trackNaptTcpUdpFlowPtr->dstIpAddr>>24, (trackNaptTcpUdpFlowPtr->dstIpAddr&0x00ff0000)>>16,
	(trackNaptTcpUdpFlowPtr->dstIpAddr&0x0000ff00)>>8, trackNaptTcpUdpFlowPtr->dstIpAddr&0xff,
	trackNaptTcpUdpFlowPtr->dstPort);
#endif
#endif
			}
			trackNaptTcpUdpFlowPtr = trackNaptTcpUdpFlowPtr->next;
			if (deletedNaptTcpUdpFlowPtr) {
				//if (deletedNaptTcpUdpFlowPtr->protoDel == 1)
				//	_rtl8651_delNaptTcpUdpFlow(flowTblIdx, deletedNaptTcpUdpFlowPtr);
				//else deletedNaptTcpUdpFlowPtr->toAsic = 0;
				_rtl8651_delNaptTcpUdpFlow(flowTblIdx, deletedNaptTcpUdpFlowPtr);
				deletedNaptTcpUdpFlowPtr->toAsic = 0;
			}
		}
	}
}

static int32 _rtl8651_selectNaptIcmpFlowOffset(uint32 flowTblIdx, uint16 *offset) {
	int8 isValid;
	uint64 mask=0; //an empty bitmap. flatten all possible offset values.
	rtl8651_tblDrv_naptIcmpFlowEntry_t * trackNaptIcmpFlowPtr;

	/* Read offset of ASIC entry, should avoid using this value */
	rtl8651_getAsicNaptIcmpOffset(flowTblIdx, offset, &isValid);

	assert(*offset<64);//rtl8651 offset should ranged from 0 to 63
	if(isValid == TRUE)
		mask|= ((uint64)1)<<(*offset);

	//Do we have a pending/collision entry on hand? should avoid using those values
	trackNaptIcmpFlowPtr = rtl8651NaptIcmpFlowTable[flowTblIdx];
	while(trackNaptIcmpFlowPtr) {
		if(trackNaptIcmpFlowPtr->canAsic == 1) {
			*offset = trackNaptIcmpFlowPtr->insideGlobalId>>10;
			mask |= ((uint64)1)<<(*offset);
		}
		trackNaptIcmpFlowPtr = trackNaptIcmpFlowPtr->next;
	}

	/* now check which offset value within configured range we can use */	
	for(*offset = 0; *offset < 64; (*offset)++)
		if((mask & ((uint64)1)<<(*offset)) == 0) //Entry not occupied
			break;
	if(*offset == 64)
		return FAILED;//Unable to get feasible offset
	return SUCCESS;
}

static rtl8651_tblDrv_naptIcmpFlowEntry_t * _rtl8651_getNaptIcmpFlowEntry(uint32 flowTblIdx, ipaddr_t insideLocalIpAddr, uint16 insideLocalId) {
	rtl8651_tblDrv_naptIcmpFlowEntry_t * trackNaptIcmpFlowPtr;
	
	trackNaptIcmpFlowPtr = rtl8651NaptIcmpFlowTable[flowTblIdx];
	while(trackNaptIcmpFlowPtr) {
		if(trackNaptIcmpFlowPtr->insideLocalIpAddr == insideLocalIpAddr && 
		   trackNaptIcmpFlowPtr->insideLocalId == insideLocalId) {
			return trackNaptIcmpFlowPtr;//Entry found
		}
		trackNaptIcmpFlowPtr = trackNaptIcmpFlowPtr->next;
	}
	return NULL;
}

static int32 _rtl8651_delNaptIcmpFlowEntry(uint32 flowTblIdx, rtl8651_tblDrv_naptIcmpFlowEntry_t * deletedEntry) {
	rtl8651_tblDrv_naptIcmpFlowEntry_t * trackEntry;

	if(deletedEntry == NULL || rtl8651NaptIcmpFlowTable[flowTblIdx] == NULL)
		return FAILED;//No entry to delete
	/*
	 * If the 'protoDel' field is turned on, then remove it from ASIC and driver
	 * table; otherwise, only remove the entry from ASIC table and the protocol
	 * stack should take care of entry deletion.
	 */	
	if (deletedEntry->protoDel == 1) {
		if(rtl8651NaptIcmpFlowTable[flowTblIdx] == deletedEntry) {
			rtl8651NaptIcmpFlowTable[flowTblIdx] = deletedEntry->next;
			assert(rtl8651NaptIcmpFlowTable[flowTblIdx] != deletedEntry);
		} else {
			trackEntry = rtl8651NaptIcmpFlowTable[flowTblIdx];
			while(trackEntry->next) {
				if(trackEntry->next == deletedEntry) {
					trackEntry->next = deletedEntry->next;
					assert(trackEntry->next != deletedEntry);
					break;
				}
				trackEntry = trackEntry->next;
			}
			if(trackEntry == NULL)
				return FAILED;//No matching entry found.
		}
		deletedEntry->next = rtl8651FreeNaptIcmpFlowEntryList;
		rtl8651FreeNaptIcmpFlowEntryList = deletedEntry;
		assert(rtl8651FreeNaptIcmpFlowEntryList != deletedEntry->next);
		rtl8651ExistedNaptIcmpFlows--;
	}
	rtl8651NaptIcmpFlowNumberTable[flowTblIdx]--;
	if(deletedEntry->canAsic == 0)
		rtl8651NaptIcmpFlowNonAsicNumberTable[flowTblIdx]--;
	return SUCCESS;
}



rtl8651_tblDrv_naptIcmpFlowEntry_t * 
_rtl8651_getOutsideNaptIcmpEntry(ipaddr_t gip, uint16 gID, ipaddr_t dip, int8 isRefresh) {
	rtl8651_tblDrv_naptIcmpFlowEntry_t *tmpIcmpEntry;
	uint32 flowTblIdx = gID&(RTL8651_ICMPTBL_SIZE-1);

	tmpIcmpEntry = rtl8651NaptIcmpFlowTable[flowTblIdx];
	for(; tmpIcmpEntry; tmpIcmpEntry=tmpIcmpEntry->next)
		if ((tmpIcmpEntry->insideGlobalId&0xfc1f) == (gID&0xfc1f)) {
			if (tmpIcmpEntry->fromAsic == 1) 
				break;
			if (tmpIcmpEntry->fromAsic == 0 && tmpIcmpEntry->dstIpAddr == dip &&
			    tmpIcmpEntry->insideGlobalIpAddr == gip)
				break;
		}
	if (tmpIcmpEntry != (rtl8651_tblDrv_naptIcmpFlowEntry_t *)NULL) {
		if (isRefresh == TRUE) {
			tmpIcmpEntry->age = rtl8651GlobalControl.icmpTimeout;
			return tmpIcmpEntry;
		}
	}

	/* Try to read ASIC entry to driver table */
	if(rtl8651FreeNaptIcmpFlowEntryList) {
		ipaddr_t asicInsideLocalIp;
		uint16 asicInsideLocalId, asicInsideGlobalId, asicAgeSec, asicCount;
		int8 isAsicStatic, isAsicCollision, isAsicValid;

		rtl8651_getAsicNaptIcmpTable(FALSE, flowTblIdx, &asicInsideLocalIp, &asicInsideLocalId, &asicInsideGlobalId, &asicAgeSec, &isAsicStatic, &asicCount, &isAsicCollision, &isAsicValid);
		if(isAsicValid == TRUE && (asicInsideGlobalId&0xfc1f) == (gID&0xfc1f)) {
			tmpIcmpEntry = rtl8651FreeNaptIcmpFlowEntryList;
			rtl8651FreeNaptIcmpFlowEntryList = tmpIcmpEntry->next;
			assert(tmpIcmpEntry != rtl8651FreeNaptIcmpFlowEntryList);
			tmpIcmpEntry->next = rtl8651NaptIcmpFlowTable[flowTblIdx];
			rtl8651NaptIcmpFlowTable[flowTblIdx] = tmpIcmpEntry;
			assert(tmpIcmpEntry->next != rtl8651NaptIcmpFlowTable[flowTblIdx]);
			rtl8651NaptIcmpFlowNumberTable[flowTblIdx]++;
			rtl8651ExistedNaptIcmpFlows++;
			tmpIcmpEntry->canAsic = 1;
			tmpIcmpEntry->insideLocalIpAddr = asicInsideLocalIp;
			tmpIcmpEntry->insideLocalId = asicInsideLocalId;
			tmpIcmpEntry->insideGlobalId= asicInsideGlobalId;
			tmpIcmpEntry->insideGlobalIpAddr = 0;
			tmpIcmpEntry->dstIpAddr = 0;
			tmpIcmpEntry->age = asicAgeSec;
			tmpIcmpEntry->count = asicCount;
			tmpIcmpEntry->fromAsic = 1;
			tmpIcmpEntry->fromDrv = 0;
			tmpIcmpEntry->toAsic = 1;
			tmpIcmpEntry->staticEntry = 0; /* should be dynamic entry */
			tmpIcmpEntry->protoDel = 1;
			tmpIcmpEntry->alive = 1;
			rtl8651_tblDrvNaptIcmpCounter.drvAddCount++;
			return tmpIcmpEntry;
		}
	}
	return (rtl8651_tblDrv_naptIcmpFlowEntry_t *)NULL;
}



rtl8651_tblDrv_naptIcmpFlowEntry_t *
_rtl8651_getInsideNaptIcmpEntry(ipaddr_t sip, uint16 sID, ipaddr_t dip, int8 isRefresh) {
	rtl8651_tblDrv_naptIcmpFlowEntry_t *tmpIcmpEntry;
	uint32 flowTblIdx;
	
	rtl8651_naptIcmpTableIndex(sip, sID, dip, &flowTblIdx);
	tmpIcmpEntry = rtl8651NaptIcmpFlowTable[flowTblIdx];
	for(; tmpIcmpEntry; tmpIcmpEntry=tmpIcmpEntry->next)
		if ((tmpIcmpEntry->insideLocalIpAddr == sip) &&
			(tmpIcmpEntry->dstIpAddr == dip) &&
			(tmpIcmpEntry->insideLocalId == sID)) {
			if (isRefresh == TRUE)
				tmpIcmpEntry->age = rtl8651GlobalControl.icmpTimeout;
			return tmpIcmpEntry;
		}
	return (rtl8651_tblDrv_naptIcmpFlowEntry_t *)NULL;
}



static rtl8651_tblDrv_naptIcmpFlowEntry_t * 
_rtl8651_getAsicNaptIcmpDynamicFlowToTable(uint32 flowTblIdx) {
	ipaddr_t asicInsideLocalIp;
	uint16 asicInsideLocalId, asicInsideGlobalId, asicAgeSec, asicCount;
	int8 isAsicStatic, isAsicCollision, isAsicValid;
	rtl8651_tblDrv_naptIcmpFlowEntry_t * asicNaptIcmpFlowPtr;
	
	rtl8651_getAsicNaptIcmpTable(FALSE, flowTblIdx, &asicInsideLocalIp, &asicInsideLocalId, &asicInsideGlobalId, &asicAgeSec, &isAsicStatic, &asicCount, &isAsicCollision, &isAsicValid);
	if(isAsicValid == FALSE) {//Remove from ASIC entry
		asicNaptIcmpFlowPtr = NULL;
	}
	else {//Entry valid
		asicNaptIcmpFlowPtr = _rtl8651_getNaptIcmpFlowEntry(flowTblIdx, asicInsideLocalIp, asicInsideLocalId);
		assert(asicNaptIcmpFlowPtr == NULL || asicNaptIcmpFlowPtr->toAsic == 1);
		if(asicNaptIcmpFlowPtr) {
			/* Update aging time */
		 	asicNaptIcmpFlowPtr->age = asicAgeSec;
			asicNaptIcmpFlowPtr->toAsic = 0;
			assert(asicNaptIcmpFlowPtr->canAsic == 1);
			return asicNaptIcmpFlowPtr;//Entry is duplicated no further process is required
		}
		/*
		 * Otherwise, the entry was added by ASIC auto-learn.
		 */
		assert(isAsicStatic == FALSE);//Should not get static entry but not in driver table
		if(rtl8651FreeNaptIcmpFlowEntryList == NULL)
			return NULL;//Entry exist but not storage to store it.
		asicNaptIcmpFlowPtr = rtl8651FreeNaptIcmpFlowEntryList;
		rtl8651FreeNaptIcmpFlowEntryList = asicNaptIcmpFlowPtr->next;
		assert(asicNaptIcmpFlowPtr != rtl8651FreeNaptIcmpFlowEntryList);
		asicNaptIcmpFlowPtr->next = rtl8651NaptIcmpFlowTable[flowTblIdx];
		rtl8651NaptIcmpFlowTable[flowTblIdx] = asicNaptIcmpFlowPtr;
		assert(asicNaptIcmpFlowPtr->next != rtl8651NaptIcmpFlowTable[flowTblIdx]);
		rtl8651NaptIcmpFlowNumberTable[flowTblIdx]++;
		rtl8651ExistedNaptIcmpFlows++;
		asicNaptIcmpFlowPtr->canAsic = 1;
		asicNaptIcmpFlowPtr->insideLocalIpAddr = asicInsideLocalIp;
		asicNaptIcmpFlowPtr->insideLocalId = asicInsideLocalId;
		asicNaptIcmpFlowPtr->insideGlobalId= asicInsideGlobalId;
		asicNaptIcmpFlowPtr->age = asicAgeSec;
		asicNaptIcmpFlowPtr->count = asicCount;
		asicNaptIcmpFlowPtr->fromAsic = 1;
		asicNaptIcmpFlowPtr->fromDrv = 0;
		asicNaptIcmpFlowPtr->toAsic = 1;
		asicNaptIcmpFlowPtr->staticEntry = 0; /* should be dynamic entry */
		asicNaptIcmpFlowPtr->protoDel = 1;
		asicNaptIcmpFlowPtr->alive = 1;
		rtl8651_tblDrvNaptIcmpCounter.drvAddCount++;
	}
	return asicNaptIcmpFlowPtr;
}

static int32 _rtl8651_addNaptIcmpCandidateFlow(int8 fromDrv, ipaddr_t insideLocalIpAddr, uint16 insideLocalId, 
			ipaddr_t insideGlobalIpAddr, uint16 insideGlobalId, ipaddr_t dstIpAddr) {
	uint32 flowTblIdx;
	ipaddr_t asicInsideLocalIp;
	uint16 asicInsideLocalId, asicInsideGlobalId, asicAgeSec, asicCount;
	int8 asicIsStatic, asicIsColl, asicIsValid;
	rtl8651_tblDrv_naptIcmpFlowEntry_t * tempNaptIcmpFlowPtr, 
									   * trackNaptIcmpFlowPtr;
	
	rtl8651_naptIcmpTableIndex(insideLocalIpAddr, insideLocalId, dstIpAddr, &flowTblIdx);
	if(rtl8651FreeNaptIcmpFlowEntryList == NULL) {
		rtl8651_tblDrvNaptIcmpCounter.addFailure++;
		return FAILED;//No free flow entry available
	}
	tempNaptIcmpFlowPtr = rtl8651FreeNaptIcmpFlowEntryList;
	rtl8651FreeNaptIcmpFlowEntryList = tempNaptIcmpFlowPtr->next;
	assert(rtl8651FreeNaptIcmpFlowEntryList != tempNaptIcmpFlowPtr);
	tempNaptIcmpFlowPtr->next = rtl8651NaptIcmpFlowTable[flowTblIdx];
	rtl8651NaptIcmpFlowTable[flowTblIdx] = tempNaptIcmpFlowPtr;
	assert(tempNaptIcmpFlowPtr->next != rtl8651NaptIcmpFlowTable[flowTblIdx]);
	rtl8651ExistedNaptIcmpFlows++;
	rtl8651NaptIcmpFlowNumberTable[flowTblIdx]++;
	tempNaptIcmpFlowPtr->insideLocalIpAddr = insideLocalIpAddr;
	tempNaptIcmpFlowPtr->insideLocalId = insideLocalId;
	tempNaptIcmpFlowPtr->insideGlobalId = insideGlobalId;
	tempNaptIcmpFlowPtr->insideGlobalIpAddr = insideGlobalIpAddr;
	tempNaptIcmpFlowPtr->dstIpAddr = dstIpAddr;
	tempNaptIcmpFlowPtr->age = rtl8651GlobalControl.icmpTimeout;
	tempNaptIcmpFlowPtr->canAsic = 1;
	tempNaptIcmpFlowPtr->fromAsic = 0;
	tempNaptIcmpFlowPtr->fromDrv = fromDrv==TRUE? 1: 0;
	tempNaptIcmpFlowPtr->staticEntry = 1;
	tempNaptIcmpFlowPtr->protoDel = 0;
	tempNaptIcmpFlowPtr->alive = 1;
	tempNaptIcmpFlowPtr->toAsic = 0;
	tempNaptIcmpFlowPtr->count = 0;
	if (fromDrv == FALSE)
		rtl8651_tblDrvNaptIcmpCounter.protoAddCount++;
	else rtl8651_tblDrvNaptIcmpCounter.drvAddCount++;
	
	if(rtl8651NaptIcmpBackwardCollisionCountTable[flowTblIdx] > 0) 
		return SUCCESS; //Entry is collided means no change to installed to ASIC
	if(rtl8651NaptIcmpFlowNumberTable[flowTblIdx] - rtl8651NaptIcmpFlowNonAsicNumberTable[flowTblIdx] == 1) {//First can forward by ASIC entry
		if(rtl8651_setAsicNaptIcmpTable_Patch(FALSE, tempNaptIcmpFlowPtr->insideLocalIpAddr, tempNaptIcmpFlowPtr->insideLocalId, tempNaptIcmpFlowPtr->insideGlobalId, tempNaptIcmpFlowPtr->age, tempNaptIcmpFlowPtr->staticEntry==1? TRUE: FALSE, tempNaptIcmpFlowPtr->count, rtl8651NaptIcmpFlowNumberTable[flowTblIdx]> 1 || rtl8651NaptIcmpFlowNonAsicNumberTable[flowTblIdx]>0 ||rtl8651NaptIcmpBackwardCollisionCountTable[flowTblIdx]>0? TRUE: FALSE, TRUE) == SUCCESS)
			tempNaptIcmpFlowPtr->toAsic = 1;
		else { 
			//chhuang: set collison bit
			// NOTE:
			//		It may happen that the entry is immediately timeout after the first try-write. In
			//		such situation, we may consider that the entry is still valid; but in fact, it is
			//		timeout already.
			//		TO SOVE this problem, DO NOT CARE this fact, once a NULL is returned, force write!
			rtl8651_getAsicNaptIcmpTable(FALSE, flowTblIdx, &asicInsideLocalIp, &asicInsideLocalId, &asicInsideGlobalId, &asicAgeSec, &asicIsStatic, &asicCount, &asicIsColl, &asicIsValid);
			if (asicIsValid == TRUE)
				rtl8651_setAsicNaptIcmpTable_Patch(TRUE, asicInsideLocalIp, asicInsideLocalId, asicInsideGlobalId, asicAgeSec, asicIsStatic, asicCount, TRUE, asicIsValid);
			else {
				rtl8651_setAsicNaptIcmpTable_Patch(TRUE, tempNaptIcmpFlowPtr->insideLocalIpAddr, tempNaptIcmpFlowPtr->insideLocalId, tempNaptIcmpFlowPtr->insideGlobalId, tempNaptIcmpFlowPtr->age, tempNaptIcmpFlowPtr->staticEntry==1? TRUE: FALSE, tempNaptIcmpFlowPtr->count, rtl8651NaptIcmpFlowNumberTable[flowTblIdx]> 1 || rtl8651NaptIcmpFlowNonAsicNumberTable[flowTblIdx]>0 ||rtl8651NaptIcmpBackwardCollisionCountTable[flowTblIdx]>0? TRUE: FALSE, TRUE);
				tempNaptIcmpFlowPtr->toAsic = 1;
			}
		}
	}
	else if(rtl8651NaptIcmpFlowNumberTable[flowTblIdx] == 2) {
		trackNaptIcmpFlowPtr = rtl8651NaptIcmpFlowTable[flowTblIdx];
		while(trackNaptIcmpFlowPtr) {
			if(trackNaptIcmpFlowPtr->toAsic == 1)
				break;
			trackNaptIcmpFlowPtr = trackNaptIcmpFlowPtr->next;
		}
		if(trackNaptIcmpFlowPtr) //Entry previous filled into ASIC
			rtl8651_setAsicNaptIcmpTable_Patch(TRUE, trackNaptIcmpFlowPtr->insideLocalIpAddr, trackNaptIcmpFlowPtr->insideLocalId, trackNaptIcmpFlowPtr->insideGlobalId, trackNaptIcmpFlowPtr->age, trackNaptIcmpFlowPtr->staticEntry==1? TRUE: FALSE, trackNaptIcmpFlowPtr->count, rtl8651NaptIcmpFlowNumberTable[flowTblIdx]> 1 || rtl8651NaptIcmpFlowNonAsicNumberTable[flowTblIdx]>0 ||rtl8651NaptIcmpBackwardCollisionCountTable[flowTblIdx]>0? TRUE: FALSE, TRUE);
	}
	return SUCCESS;//Entry either write to ASIC or update ASIC information
}

static int32 _rtl8651_addNaptIcmpNonCandidateFlow(ipaddr_t insideLocalIpAddr, uint16 insideLocalId, 
			ipaddr_t insideGlobalIpAddr, uint16 insideGlobalId, ipaddr_t dstIpAddr) {
	uint32 flowTblIdx;
	ipaddr_t asicInsideLocalIp;
	uint16 asicInsideLocalId, asicInsideGlobalId, asicAgeSec, asicCount;
	int8 asicIsStatic, asicIsColl, asicIsValid;
	rtl8651_tblDrv_naptIcmpFlowEntry_t * tempNaptIcmpFlowPtr, * asicNaptIcmpFlowPtr;
	
	rtl8651_naptIcmpTableIndex(insideLocalIpAddr, insideLocalId, dstIpAddr, &flowTblIdx);
	/* Check if user specified ID is in use */
	tempNaptIcmpFlowPtr = rtl8651NaptIcmpFlowTable[flowTblIdx];
	for(; tempNaptIcmpFlowPtr; tempNaptIcmpFlowPtr = tempNaptIcmpFlowPtr->next)
		if (tempNaptIcmpFlowPtr->insideGlobalId == insideGlobalId)
			return(TBLDRV_EEXTIDINUSE);
	if(rtl8651FreeNaptIcmpFlowEntryList == NULL) {
		rtl8651_tblDrvNaptIcmpCounter.addFailure++;
		return FAILED;//No free flow entry available
	}
	tempNaptIcmpFlowPtr = rtl8651FreeNaptIcmpFlowEntryList;
	rtl8651FreeNaptIcmpFlowEntryList = tempNaptIcmpFlowPtr->next;
	assert(tempNaptIcmpFlowPtr!= rtl8651FreeNaptIcmpFlowEntryList);
	tempNaptIcmpFlowPtr->next = rtl8651NaptIcmpFlowTable[flowTblIdx];
	rtl8651NaptIcmpFlowTable[flowTblIdx] = tempNaptIcmpFlowPtr;
	assert(tempNaptIcmpFlowPtr->next != rtl8651NaptIcmpFlowTable[flowTblIdx]);
	rtl8651ExistedNaptIcmpFlows++;
	rtl8651NaptIcmpFlowNumberTable[flowTblIdx]++;
	rtl8651NaptIcmpFlowNonAsicNumberTable[flowTblIdx]++;
	tempNaptIcmpFlowPtr->insideLocalIpAddr = insideLocalIpAddr;
	tempNaptIcmpFlowPtr->insideLocalId = insideLocalId;
	tempNaptIcmpFlowPtr->insideGlobalId = insideGlobalId;
	tempNaptIcmpFlowPtr->insideGlobalIpAddr = insideGlobalIpAddr;
	tempNaptIcmpFlowPtr->dstIpAddr = dstIpAddr;
	tempNaptIcmpFlowPtr->age = rtl8651GlobalControl.icmpTimeout;
	tempNaptIcmpFlowPtr->canAsic = 0;
	tempNaptIcmpFlowPtr->fromAsic = 0;
	tempNaptIcmpFlowPtr->fromDrv = 0;
	tempNaptIcmpFlowPtr->staticEntry = 1;
	tempNaptIcmpFlowPtr->protoDel = 0;
	tempNaptIcmpFlowPtr->alive = 1;
	tempNaptIcmpFlowPtr->count = 0;
	tempNaptIcmpFlowPtr->toAsic = 0;
	rtl8651_tblDrvNaptIcmpCounter.diffHashCount++;
	rtl8651_tblDrvNaptIcmpCounter.protoAddCount++;
	
	if(rtl8651NaptIcmpBackwardCollisionCountTable[flowTblIdx] == 0) {//Only when non-backward collided need to fill collision bit
		asicNaptIcmpFlowPtr = rtl8651NaptIcmpFlowTable[flowTblIdx];
		while(asicNaptIcmpFlowPtr) {
			if(asicNaptIcmpFlowPtr->toAsic == 1)
				break;
			asicNaptIcmpFlowPtr = asicNaptIcmpFlowPtr->next;
		}
		if(asicNaptIcmpFlowPtr)
			rtl8651_setAsicNaptIcmpTable_Patch(TRUE, asicNaptIcmpFlowPtr->insideLocalIpAddr, asicNaptIcmpFlowPtr->insideLocalId, asicNaptIcmpFlowPtr->insideGlobalId, asicNaptIcmpFlowPtr->age, asicNaptIcmpFlowPtr->staticEntry==1? TRUE: FALSE, asicNaptIcmpFlowPtr->count, rtl8651NaptIcmpFlowNumberTable[flowTblIdx]> 1 || rtl8651NaptIcmpFlowNonAsicNumberTable[flowTblIdx]>0 || rtl8651NaptIcmpBackwardCollisionCountTable[flowTblIdx]>0? TRUE: FALSE, TRUE);
		else {
			if(rtl8651_setAsicNaptIcmpTable_Patch(FALSE, 0, 0, flowTblIdx, 0, FALSE, 0, TRUE, FALSE) == FAILED) {
				rtl8651_getAsicNaptIcmpTable(FALSE, flowTblIdx, &asicInsideLocalIp, &asicInsideLocalId, &asicInsideGlobalId, &asicAgeSec, &asicIsStatic, &asicCount, &asicIsColl, &asicIsValid);
				if (asicIsValid == TRUE)
					rtl8651_setAsicNaptIcmpTable_Patch(TRUE, asicInsideLocalIp, asicInsideLocalId, asicInsideGlobalId, asicAgeSec, asicIsStatic, asicCount, TRUE, asicIsValid);
				else //If unable to write normally and unable to get valid ASIC entry, just force write to collide the entry
					rtl8651_setAsicNaptIcmpTable_Patch(TRUE, 0, 0, flowTblIdx, 0, FALSE, 0, TRUE, FALSE);
			}
		}
	}
	flowTblIdx = insideGlobalId & (RTL8651_ICMPTBL_SIZE-1);
	rtl8651NaptIcmpBackwardCollisionCountTable[flowTblIdx]++;
	if(rtl8651_setAsicNaptIcmpTable_Patch(FALSE, 0, 0, flowTblIdx, 0, FALSE, 0, TRUE, FALSE) == FAILED) {
		asicNaptIcmpFlowPtr = _rtl8651_getAsicNaptIcmpDynamicFlowToTable(flowTblIdx);
		rtl8651_setAsicNaptIcmpTable_Patch(TRUE, 0, 0, flowTblIdx, 0, FALSE, 0, TRUE, FALSE);
	}
	return SUCCESS;//Entry either write to ASIC or update ASIC information
}

int32 rtl8651_addNaptIcmpFlow(int8 assigned, ipaddr_t insideLocalIpAddr, uint16 insideLocalId, 
			ipaddr_t *insideGlobalIpAddr, uint16 *insideGlobalId, ipaddr_t dstIpAddr) {
	rtl8651_tblDrv_naptIcmpFlowEntry_t * tempNaptIcmpFlowPtr;
	uint32 flowTblIdx, flowHashingIdx, result;
	uint16 offset;

	if(insideGlobalIpAddr == NULL || insideGlobalId == NULL)	 
		return TBLDRV_EINVVAL;//No IP and ID for storage
	RTL8651_LOCK_RESOURCE();//Lock resource
	 
	if(_rtl8651_intNetIpAddr(insideLocalIpAddr) == FALSE) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource	 
		return TBLDRV_ENOTINTIP;//Inside local ip address does not belong to configured network interface
	}
	if(assigned == TRUE && _rtl8651_extIpAddr(*insideGlobalIpAddr, NULL) == FALSE) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return TBLDRV_ENOTEXTIP;//Inside global ip address does not belong to external IP interface IP addresses
	}
	if(_rtl8651_localServerIpAddr(dstIpAddr) == TRUE) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return TBLDRV_EDSTIPISLS;//Local server should not trigger NAT operation
	}
	flowHashingIdx = rtl8651_naptIcmpTableIndex(insideLocalIpAddr, insideLocalId, dstIpAddr, &flowTblIdx);
	assert(flowHashingIdx < RTL8651_TCPUDPTBL_SIZE);//Hashing result is 10-bit instead of matching table size
	if(assigned == FALSE) {
		if(_rtl8651_selectNaptIcmpFlowOffset(flowTblIdx, &offset) == FAILED) {
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return TBLDRV_EDSTIPISLS;//Available ID not found
		}
		*insideGlobalId = offset<<10 | flowHashingIdx;
		*insideGlobalIpAddr = rtl8651_getNaptInsideGlobalIpAddr(FALSE, insideLocalIpAddr, insideLocalId, dstIpAddr, 0);
	}
	if ((tempNaptIcmpFlowPtr=_rtl8651_getNaptIcmpFlowEntry(flowTblIdx, insideLocalIpAddr, insideLocalId))) {
		/*
		 * After protocol stack removing and Beofer entry aging out, if protocol stack
		 * adds the same entry again, handle it!
		 */
		if (tempNaptIcmpFlowPtr->protoDel == 1 && tempNaptIcmpFlowPtr->age > 0) {
			tempNaptIcmpFlowPtr->age = rtl8651GlobalControl.icmpTimeout;
			if (tempNaptIcmpFlowPtr->toAsic == 1)
					rtl8651_setAsicNaptIcmpTable_Patch(TRUE, tempNaptIcmpFlowPtr->insideLocalIpAddr, tempNaptIcmpFlowPtr->insideLocalId, tempNaptIcmpFlowPtr->insideGlobalId, tempNaptIcmpFlowPtr->age, (tempNaptIcmpFlowPtr->staticEntry==1?TRUE: FALSE), tempNaptIcmpFlowPtr->count, rtl8651NaptIcmpFlowNumberTable[flowTblIdx]> 1 || rtl8651NaptIcmpFlowNonAsicNumberTable[flowTblIdx]>0 || rtl8651NaptIcmpBackwardCollisionCountTable[flowTblIdx]>0? TRUE: FALSE, TRUE);
			tempNaptIcmpFlowPtr->protoDel = 0;
			*insideGlobalId = tempNaptIcmpFlowPtr->insideGlobalId;
			*insideGlobalIpAddr = tempNaptIcmpFlowPtr->insideGlobalIpAddr;
			rtl8651_tblDrvNaptIcmpCounter.protoAddCount++;
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			return SUCCESS;
		}
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return FAILED; /* Duplicate entry */
	}	
	if(flowTblIdx != (*insideGlobalId&(RTL8651_ICMPTBL_SIZE-1)) || *insideGlobalIpAddr != rtl8651_getNaptInsideGlobalIpAddr(FALSE, insideLocalIpAddr, insideLocalId, dstIpAddr, 0)) //Unable to configure into ASIC
		result = _rtl8651_addNaptIcmpNonCandidateFlow(insideLocalIpAddr, insideLocalId, *insideGlobalIpAddr, *insideGlobalId, dstIpAddr);
	else 
		result = _rtl8651_addNaptIcmpCandidateFlow(FALSE, insideLocalIpAddr, insideLocalId, *insideGlobalIpAddr, *insideGlobalId, dstIpAddr);
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return result;
}

static void _rtl8651_refillNaptIcmpFlowOccupiedEntry(uint32 flowTblIdx, rtl8651_tblDrv_naptIcmpFlowEntry_t *delEntry) {
	rtl8651_tblDrv_naptIcmpFlowEntry_t * trackNaptIcmpFlowPtr, * asicNaptIcmpFlowPtr;
	ipaddr_t asicInsideLocalIp;
	uint16 asicInsideLocalId, asicInsideGlobalId, asicAgeSec, asicCount;
	int8 asicIsStatic, asicIsColl, asicIsValid, drvToAsic = FALSE;

	if (rtl8651NaptIcmpBackwardCollisionCountTable[flowTblIdx] > 0)
		return; //chhuang: bwcoll count>0 => nothing to do
	asicNaptIcmpFlowPtr = NULL;//Find candidate entry to fill into ASIC
	if(rtl8651NaptIcmpFlowNumberTable[flowTblIdx] - rtl8651NaptIcmpFlowNonAsicNumberTable[flowTblIdx] > 0) {
		trackNaptIcmpFlowPtr = rtl8651NaptIcmpFlowTable[flowTblIdx];
		while(trackNaptIcmpFlowPtr) {//Count entry number
			if(trackNaptIcmpFlowPtr->canAsic == 1 && trackNaptIcmpFlowPtr->age > 0) {
				if(asicNaptIcmpFlowPtr == NULL)
					asicNaptIcmpFlowPtr = trackNaptIcmpFlowPtr;
				else if(trackNaptIcmpFlowPtr->fromAsic > asicNaptIcmpFlowPtr->fromAsic)
					asicNaptIcmpFlowPtr = trackNaptIcmpFlowPtr;
				else if (trackNaptIcmpFlowPtr->fromAsic ==  asicNaptIcmpFlowPtr->fromAsic && trackNaptIcmpFlowPtr->fromDrv > asicNaptIcmpFlowPtr->fromDrv)
					asicNaptIcmpFlowPtr = trackNaptIcmpFlowPtr;
			}
			if (trackNaptIcmpFlowPtr->toAsic == 1)
				drvToAsic = TRUE;
			trackNaptIcmpFlowPtr = trackNaptIcmpFlowPtr->next;
		}
	}
	if (delEntry != NULL) {
		rtl8651_getAsicNaptIcmpTable(FALSE, flowTblIdx, &asicInsideLocalIp, &asicInsideLocalId, &asicInsideGlobalId, &asicAgeSec, &asicIsStatic, &asicCount, &asicIsColl, &asicIsValid);
		if (asicIsValid == TRUE) {
			if (delEntry->insideLocalIpAddr != asicInsideLocalIp ||
				delEntry->insideLocalId != asicInsideLocalId ||
				(delEntry->insideGlobalId&0xfc1f) != asicInsideGlobalId) {
				rtl8651_setAsicNaptIcmpTable_Patch(TRUE, asicInsideLocalIp, asicInsideLocalId, asicInsideGlobalId, asicAgeSec, asicIsStatic, asicCount, (rtl8651NaptIcmpFlowNumberTable[flowTblIdx]>0&&drvToAsic==FALSE) || (rtl8651NaptIcmpFlowNumberTable[flowTblIdx]>1&&drvToAsic==TRUE) || rtl8651NaptIcmpFlowNonAsicNumberTable[flowTblIdx]>0 || rtl8651NaptIcmpBackwardCollisionCountTable[flowTblIdx]>0? TRUE: FALSE, asicIsValid);
				return;
			}
		}
	}
	if(asicNaptIcmpFlowPtr) {
		rtl8651_setAsicNaptIcmpTable_Patch(TRUE, asicNaptIcmpFlowPtr->insideLocalIpAddr, asicNaptIcmpFlowPtr->insideLocalId, asicNaptIcmpFlowPtr->insideGlobalId, asicNaptIcmpFlowPtr->age, asicNaptIcmpFlowPtr->staticEntry==1? TRUE: FALSE, asicNaptIcmpFlowPtr->count, rtl8651NaptIcmpFlowNumberTable[flowTblIdx]>1 || rtl8651NaptIcmpFlowNonAsicNumberTable[flowTblIdx] > 0 || rtl8651NaptIcmpBackwardCollisionCountTable[flowTblIdx]>0? TRUE: FALSE, TRUE);
		/*
		 * If the entry is from ASIC, write it into ASIC and remove it from driver table
		 */
		if (asicNaptIcmpFlowPtr->fromAsic == 1)
			_rtl8651_delNaptIcmpFlowEntry(flowTblIdx, asicNaptIcmpFlowPtr);
	 	else asicNaptIcmpFlowPtr->toAsic = 1;		
		//asicNaptIcmpFlowPtr->toAsic = 1;
		rtl8651_tblDrvNaptIcmpCounter.threadWriteBackCount++;
	} else rtl8651_setAsicNaptIcmpTable_Patch(TRUE, 0, 0, flowTblIdx, 0, FALSE, 0, rtl8651NaptIcmpFlowNumberTable[flowTblIdx]>1 || rtl8651NaptIcmpFlowNonAsicNumberTable[flowTblIdx] > 0 || rtl8651NaptIcmpBackwardCollisionCountTable[flowTblIdx]>0? TRUE: FALSE, FALSE);
}


static int32 _rtl8651_delNaptIcmpFlow(uint32 flowTblIdx, rtl8651_tblDrv_naptIcmpFlowEntry_t *delEntry) {
	ipaddr_t asicInsideLocalIp;
	uint16 asicInsideLocalId, asicInsideGlobalId, asicAgeSec, asicCount;
	int8 asicIsStatic, asicIsColl, asicIsValid;
	rtl8651_tblDrv_naptIcmpFlowEntry_t *tmpDrvEntry;

	if (delEntry == (rtl8651_tblDrv_naptIcmpFlowEntry_t *)0)
		return FAILED;
	
	_rtl8651_delNaptIcmpFlowEntry(flowTblIdx, delEntry);
	rtl8651_tblDrvNaptIcmpCounter.drvDelCount++;
	if(delEntry->toAsic == 1) {
		assert(delEntry->canAsic == 1);
		assert(rtl8651NaptIcmpBackwardCollisionCountTable[flowTblIdx] == 0);
	}
	if(delEntry->toAsic == 1 || delEntry->canAsic == 0)
		_rtl8651_refillNaptIcmpFlowOccupiedEntry(flowTblIdx, delEntry);
	else {
		/*
		 * Otherwise, the deleted entry is not written to ASIC. Beside remove it from
		 * internal structure, it is also needed to update the collison bit in the ASIC 
		 * table.
		 * NOTE:
		 *		In this case, we assume there is an entry in the ASIC table. Hence try
		 *		to read ASIC table and update its collison bit. If we find that the 
		 *		ASIC table entry is invalid, just do nothing. Because the maintenance
		 *		thread will take care of writting entry in the driver back to ASIC table.
		 */
		tmpDrvEntry = rtl8651NaptIcmpFlowTable[flowTblIdx];
		while(tmpDrvEntry && tmpDrvEntry->toAsic != 1)
			tmpDrvEntry = tmpDrvEntry->next;
		rtl8651_getAsicNaptIcmpTable(FALSE, flowTblIdx, &asicInsideLocalIp, &asicInsideLocalId, &asicInsideGlobalId, &asicAgeSec, &asicIsStatic, &asicCount, &asicIsColl, &asicIsValid);
		if (asicIsValid == TRUE) {
			if (tmpDrvEntry)
				rtl8651_setAsicNaptIcmpTable_Patch(TRUE, asicInsideLocalIp, asicInsideLocalId, asicInsideGlobalId, asicAgeSec, asicIsStatic, asicCount, rtl8651NaptIcmpFlowNumberTable[flowTblIdx]> 1 || rtl8651NaptIcmpFlowNonAsicNumberTable[flowTblIdx]>0 || rtl8651NaptIcmpBackwardCollisionCountTable[flowTblIdx]>0? TRUE: FALSE, asicIsValid);
			else 
				rtl8651_setAsicNaptIcmpTable_Patch(TRUE, asicInsideLocalIp, asicInsideLocalId, asicInsideGlobalId, asicAgeSec, asicIsStatic, asicCount, rtl8651NaptIcmpFlowNumberTable[flowTblIdx]> 0 || rtl8651NaptIcmpFlowNonAsicNumberTable[flowTblIdx]>0 || rtl8651NaptIcmpBackwardCollisionCountTable[flowTblIdx]>0? TRUE: FALSE, asicIsValid);
		}			
	}
	if(delEntry->canAsic==0) {//This flow will backward collision other entry, process the collided flow entry
		assert(delEntry->toAsic == 0);
		flowTblIdx = delEntry->insideGlobalId & (RTL8651_ICMPTBL_SIZE-1);
		rtl8651NaptIcmpBackwardCollisionCountTable[flowTblIdx]--;
		if(rtl8651NaptIcmpBackwardCollisionCountTable[flowTblIdx] == 0) 
			_rtl8651_refillNaptIcmpFlowOccupiedEntry(flowTblIdx, NULL);
	}
	return SUCCESS;
}


int32 rtl8651_delNaptIcmpFlow(ipaddr_t insideLocalIpAddr, uint16 insideLocalId, ipaddr_t dstIpAddr) {
	uint32 flowHashingIdx, flowTblIdx;
	rtl8651_tblDrv_naptIcmpFlowEntry_t * deletedEntry, * preDelEntry;

	RTL8651_LOCK_RESOURCE();//Lock resource
	if(_rtl8651_intNetIpAddr(insideLocalIpAddr) == FALSE) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return TBLDRV_ENOTINTIP;//Local ip address not internal IP address
	}
	if(_rtl8651_localServerIpAddr(dstIpAddr) == TRUE) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return TBLDRV_EDSTIPISLS;
	}
	flowHashingIdx = rtl8651_naptIcmpTableIndex(insideLocalIpAddr, insideLocalId, dstIpAddr, &flowTblIdx);
	assert(flowHashingIdx < RTL8651_TCPUDPTBL_SIZE && flowTblIdx < RTL8651_ICMPTBL_SIZE);
	deletedEntry = rtl8651NaptIcmpFlowTable[flowTblIdx];
	for(preDelEntry=NULL; deletedEntry; preDelEntry=deletedEntry, deletedEntry=deletedEntry->next) {
		if (deletedEntry->fromAsic == 0 && deletedEntry->fromDrv == 0 &&
			deletedEntry->insideLocalIpAddr == insideLocalIpAddr &&
			deletedEntry->insideLocalId == insideLocalId &&
			deletedEntry->dstIpAddr == dstIpAddr)
				break;
	}
	if(deletedEntry == NULL) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return TBLDRV_EENTRYNOTFOUND;//Sepcified entry not found
	}
	/*
	 * If the entry was already remove by protocol stack, just return FAILED
	 * to indicate it.
	 */
	if (deletedEntry->protoDel == 1) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return TBLDRV_EENTRYNOTFOUND;//Entry to be deleted is not found
	}
	deletedEntry->protoDel = 1;
	//if (tempNaptIcmpFlowPtr->age == 0)
	//	_rtl8651_delNaptIcmpFlow(flowTblIdx, tempNaptIcmpFlowPtr);
	if (deletedEntry->age == 0) {
		/* Only remove the entry from driver table */
		if (preDelEntry == NULL)
			rtl8651NaptIcmpFlowTable[flowTblIdx] = deletedEntry->next;
		else preDelEntry->next = deletedEntry->next;
		deletedEntry->next = rtl8651FreeNaptIcmpFlowEntryList;
		rtl8651FreeNaptIcmpFlowEntryList = deletedEntry;
		assert(deletedEntry->next != rtl8651FreeNaptIcmpFlowEntryList);
		rtl8651ExistedNaptIcmpFlows --;
	}
	rtl8651_tblDrvNaptIcmpCounter.protoDelCount++;
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

//This API try to take advantage of ASIC acceleration capability
void rtl8651_updateAsicIcmpTable(void) {
	rtl8651_tblDrv_naptIcmpFlowEntry_t *tmpNaptPtr, *delNaptPtr;
	static uint32 flowTblIdx = RTL8651_ICMPTBL_SIZE;
	ipaddr_t asicInsideLocalIp;
	uint16 asicInsideLocalId, asicInsideGlobalId, asicAgeSec, asicCount;
	int8 asicIsStatic, asicIsColl, asicIsValid;

	RTL8651_LOCK_RESOURCE();//Lock resource
	flowTblIdx = ((flowTblIdx+1)&(RTL8651_ICMPTBL_SIZE-1));
	rtl8651_getAsicNaptIcmpTable(FALSE, flowTblIdx, &asicInsideLocalIp, &asicInsideLocalId, &asicInsideGlobalId, &asicAgeSec, &asicIsStatic, &asicCount, &asicIsColl, &asicIsValid);
	/*
	 * If the ASIC table entry is invalid, try to write an entry in the driver 
	 * table to the ASIC table.
	 */
	if(asicIsColl == TRUE && asicIsValid == FALSE)
		_rtl8651_refillNaptIcmpFlowOccupiedEntry(flowTblIdx, NULL);
	else if (asicIsValid == TRUE) {
		tmpNaptPtr = rtl8651NaptIcmpFlowTable[flowTblIdx];
		while(tmpNaptPtr) {
			if (tmpNaptPtr->toAsic == 1) {
				if (tmpNaptPtr->insideLocalIpAddr == asicInsideLocalIp &&
					tmpNaptPtr->insideLocalId == asicInsideLocalId &&
					(tmpNaptPtr->insideGlobalId&0xfc1f) == (asicInsideGlobalId&0xfc1f)) {
					/* entry match! SYNC! */
					tmpNaptPtr->age = asicAgeSec;
#ifndef CYGWIN
					//if (tmpNaptPtr->age <10)
					//	printf("Napt icmp: update aging time to: %d\n", tmpNaptPtr->age);
#endif
				} else {
					/* Otherwise, the entry in the ASIC is aging out */
					tmpNaptPtr->age = 0;
					//if (tmpNaptPtr->protoDel == 1) {
					//	delNaptPtr = tmpNaptPtr;
					//	tmpNaptPtr = tmpNaptPtr->next;
					//	_rtl8651_refillNaptIcmpFlowOccupiedEntry(flowTblIdx, tmpNaptPtr);
					//	continue;
					//}
					delNaptPtr = tmpNaptPtr;
					tmpNaptPtr = tmpNaptPtr->next;
					assert(delNaptPtr != tmpNaptPtr);
					_rtl8651_delNaptIcmpFlow(flowTblIdx, delNaptPtr);
					delNaptPtr->toAsic = 0;
					rtl8651_tblDrvNaptIcmpCounter.syncFailure++;
					continue;
				}
			}
			tmpNaptPtr = tmpNaptPtr->next;
		}
	}
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
}


static void _rtl8651_timeUpdateNaptIcmpTable(uint32 secPassed) {
	uint32 flowTblIdx;
	rtl8651_tblDrv_naptIcmpFlowEntry_t * trackNaptIcmpFlowPtr, * deletedNaptIcmpFlowPtr;

	for(flowTblIdx = 0; flowTblIdx < RTL8651_ICMPTBL_SIZE; flowTblIdx++) {
		trackNaptIcmpFlowPtr = rtl8651NaptIcmpFlowTable[flowTblIdx];
		while(trackNaptIcmpFlowPtr) {
			assert(!(trackNaptIcmpFlowPtr->canAsic==0 && trackNaptIcmpFlowPtr->toAsic==1));
			/* If the entry is already aging out, IGNORE! */
			//if (trackNaptIcmpFlowPtr->age == 0 && trackNaptIcmpFlowPtr->protoDel == 0) {
			if (trackNaptIcmpFlowPtr->alive == 0) {
				trackNaptIcmpFlowPtr = trackNaptIcmpFlowPtr->next;
				continue;
			}
			/* Entry is in aging state, COUNTING! */
			if (trackNaptIcmpFlowPtr->age > secPassed) {
				trackNaptIcmpFlowPtr->age -= secPassed;
				deletedNaptIcmpFlowPtr = NULL;
			} else {
				trackNaptIcmpFlowPtr->age = 0;
				deletedNaptIcmpFlowPtr = trackNaptIcmpFlowPtr;
				assert(trackNaptIcmpFlowPtr->next != deletedNaptIcmpFlowPtr);
#if 0
#ifndef CYGWIN
printf("(ICMP) sip(%d.%d.%d.%d) sID(%d), dip(%d.%d.%d.%d) GID(%d) time out\n", trackNaptIcmpFlowPtr->insideLocalIpAddr>>24, 
	(trackNaptIcmpFlowPtr->insideLocalIpAddr&0x00ff0000)>>16,
	(trackNaptIcmpFlowPtr->insideLocalIpAddr&0x0000ff00)>>8, trackNaptIcmpFlowPtr->insideLocalIpAddr&0xff,
	trackNaptIcmpFlowPtr->insideLocalId,
	trackNaptIcmpFlowPtr->dstIpAddr>>24, (trackNaptIcmpFlowPtr->dstIpAddr&0x00ff0000)>>16,
	(trackNaptIcmpFlowPtr->dstIpAddr&0x0000ff00)>>8, trackNaptIcmpFlowPtr->dstIpAddr&0xff,
	trackNaptIcmpFlowPtr->insideGlobalId);
#endif
#endif
			}
			trackNaptIcmpFlowPtr = trackNaptIcmpFlowPtr->next;
			if (deletedNaptIcmpFlowPtr) {
				_rtl8651_delNaptIcmpFlow(flowTblIdx, deletedNaptIcmpFlowPtr);
				deletedNaptIcmpFlowPtr->toAsic = 0;
				
			}
		}
	}
}




static rtl8651_tblDrvIpAclRule_t *
_rtl8651_ipAclLookUp(rtl8651_tblDrv_aclIpEntry_t *aclList, int8 *ip_hdr) {
	struct ip *iphdr = (struct ip *)ip_hdr;
	struct icmp *icmphdr;
	struct igmp *igmphdr;
	struct tcphdr *tcphdr;
	struct udphdr *udphdr;
 
	while (aclList) {
		/* Match common component first */
		if (((iphdr->ip_src.s_addr& aclList->rule.srcIpAddrMask) != aclList->rule.srcIpAddr) ||
			((iphdr->ip_dst.s_addr & aclList->rule.dstIpAddrMask) != aclList->rule.dstIpAddr) ||
			((iphdr->ip_tos & aclList->rule.tosMask) != aclList->rule.tos)) {
				aclList = aclList->next;
				continue;
		}
	
		switch(aclList->rule.ruleType) {

		case RTL8651_ACL_IP: /* IP rule */
			if (((iphdr->ip_p & aclList->rule.is.ip.protoMask) == aclList->rule.is.ip.proto) &&
				(((iphdr->ip_off>>13) & aclList->rule.is.ip.flagMask) == aclList->rule.is.ip.ipFlag))
			 		return (rtl8651_tblDrvIpAclRule_t *)&aclList->rule;
			break;
			
		case RTL8651_ACL_ICMP: /* ICMP rule */
			/* Check if the packet is an ICMP packet */
			if (iphdr->ip_p != IPPROTO_ICMP)
				break;
			/* Start to match ICMP rule */
			icmphdr = (struct icmp *)(ip_hdr + iphdr->ip_hl*4);
			if (((icmphdr->icmp_type & aclList->rule.is.icmp.typeMask) == aclList->rule.is.icmp.type) &&
				((icmphdr->icmp_code & aclList->rule.is.icmp.codeMask) == aclList->rule.is.icmp.code))
					return (rtl8651_tblDrvIpAclRule_t *)&aclList->rule;
			break;
			
		case RTL8651_ACL_IGMP:
			/* Check if the packet is an IGMP packet */
			if (iphdr->ip_p != IPPROTO_IGMP)
				break;
			/* Start to match IGMP rule */
			igmphdr = (struct igmp *)(ip_hdr + iphdr->ip_hl*4);
			if (((igmphdr->igmp_type & aclList->rule.is.igmp.typeMask) == aclList->rule.is.igmp.type))
					return (rtl8651_tblDrvIpAclRule_t *)&aclList->rule;
			break;
			
		case RTL8651_ACL_TCP:
			/* Check if the packet is a TCP packet */
			if (iphdr->ip_p != IPPROTO_TCP)
				break;
			/* Start to match TCP rule */
			tcphdr = (struct tcphdr *)(ip_hdr + iphdr->ip_hl*4);
			if (((tcphdr->th_flags & aclList->rule.is.tcp.flagMask) == aclList->rule.is.tcp.tcpflag) &&
				(tcphdr->th_sport>=aclList->rule.is.tcp.srcPortLowerBound && tcphdr->th_sport <= aclList->rule.is.tcp.srcPortUpperBound) &&
				(tcphdr->th_dport>=aclList->rule.is.tcp.dstPortLowerBound && tcphdr->th_dport <= aclList->rule.is.tcp.dstPortUpperBound) )
					return (rtl8651_tblDrvIpAclRule_t *)&aclList->rule;
			break;
			
		case RTL8651_ACL_UDP:
			/* Check if the packet is an UDP packet */
			if (iphdr->ip_p != IPPROTO_UDP)
				break;
			/* Start to match UDP rule */
			udphdr = (struct udphdr *)(ip_hdr + iphdr->ip_hl*4);
			if ((udphdr->uh_sport>=aclList->rule.is.udp.srcPortLowerBound && udphdr->uh_sport<=aclList->rule.is.udp.srcPortUpperBound) &&
				(udphdr->uh_dport>=aclList->rule.is.udp.dstPortLowerBound && udphdr->uh_dport<=aclList->rule.is.udp.dstPortUpperBound))
					return (rtl8651_tblDrvIpAclRule_t *)&aclList->rule;
			break;
			
		default: assert(0);
		}
		aclList = aclList->next;
	}
	/* rule not match */
	return (rtl8651_tblDrvIpAclRule_t *)0;
}

rtl8651_tblDrvMacAclRule_t * 
_rtl8651_getMacAclEntry(int8 isIngress, uint16 vid, int8 *ether_hdr) {
	struct ether_header *ehdr = (struct ether_header *)ether_hdr;
	uint32 vidx = rtl8651_vlanTableIndex(vid);
	rtl8651_tblDrv_aclMacEntry_t *macRule;
	ether_addr_t srcMac, dstMac;

	/* Input paramater check */
	if (vid < 1 || vid > 4094 || ether_hdr == (int8 *)0)
		return (rtl8651_tblDrvMacAclRule_t *)0;
	
	macRule = (isIngress == TRUE) ? rtl8651VlanTable[vidx].inAclHead :
				rtl8651VlanTable[vidx].outAclHead;
	while (macRule) {
		dstMac.octet[0]= ehdr->ether_dhost[0] & macRule->rule.dstMacMask.octet[0];
		dstMac.octet[1]= ehdr->ether_dhost[1] & macRule->rule.dstMacMask.octet[1];
		dstMac.octet[2]= ehdr->ether_dhost[2] & macRule->rule.dstMacMask.octet[2];
		dstMac.octet[3]= ehdr->ether_dhost[3] & macRule->rule.dstMacMask.octet[3];
		dstMac.octet[4]= ehdr->ether_dhost[4] & macRule->rule.dstMacMask.octet[4];
		dstMac.octet[5]= ehdr->ether_dhost[5] & macRule->rule.dstMacMask.octet[5];
		srcMac.octet[0]= ehdr->ether_shost[0] & macRule->rule.srcMacMask.octet[0];
		srcMac.octet[1]= ehdr->ether_shost[1] & macRule->rule.srcMacMask.octet[1];
		srcMac.octet[2]= ehdr->ether_shost[2] & macRule->rule.srcMacMask.octet[2];
		srcMac.octet[3]= ehdr->ether_shost[3] & macRule->rule.srcMacMask.octet[3];
		srcMac.octet[4]= ehdr->ether_shost[4] & macRule->rule.srcMacMask.octet[4];
		srcMac.octet[5]= ehdr->ether_shost[5] & macRule->rule.srcMacMask.octet[5];		
		if ( ((ehdr->ether_type & macRule->rule.typeLenMask) == macRule->rule.typeLen) &&
			 (memcmp(dstMac.octet, macRule->rule.dstMac.octet, 6) == 0) &&
			 (memcmp(srcMac.octet, macRule->rule.srcMac.octet, 6) == 0) )
				return (rtl8651_tblDrvMacAclRule_t *)&macRule->rule;
		macRule = macRule->next;
	}
	return (rtl8651_tblDrvMacAclRule_t *)0;
}

rtl8651_tblDrvIpAclRule_t * 
_rtl8651_getIpVlanAclEntry(int8 isIngress, uint16 vid, int8 *ip_hdr ) {
	uint32 vidx = rtl8651_vlanTableIndex(vid);
	rtl8651_tblDrv_aclIpEntry_t *ipRule;

	/* Input paramater check */
	if (vid < 1 || vid > 4094 || ip_hdr == (int8 *)0)
		return (rtl8651_tblDrvIpAclRule_t *)0;
	
	ipRule = (isIngress == TRUE) ? rtl8651VlanTable[vidx].inIpAclHead :
			  rtl8651VlanTable[vidx].outIpAclHead;
	return (rtl8651_tblDrvIpAclRule_t *)(_rtl8651_ipAclLookUp(ipRule, ip_hdr));
}


rtl8651_tblDrvIpAclRule_t * 
_rtl8651_getIpAclEntry(int8 isIngress, uint32 netIdx, int8 *ip_hdr) {
	rtl8651_tblDrv_aclIpEntry_t *ipRule;

	/* Input paramater check */
	if (netIdx >= rtl8651_tblDrvPara.networkIntfTableSize || ip_hdr == (int8 *)0)
		return (rtl8651_tblDrvIpAclRule_t *)0;
	
	ipRule = (isIngress == TRUE) ? rtl8651NetworkIntfTable[netIdx].inAclHead :
			  rtl8651NetworkIntfTable[netIdx].outAclHead;
	return (rtl8651_tblDrvIpAclRule_t *)(_rtl8651_ipAclLookUp(ipRule, ip_hdr));
}

int8 _rtl8651_isAclMismatchDrop(uint32 netIdx, int8 *isDrop) {

	/* Input paramater check */
	if (netIdx >= rtl8651_tblDrvPara.networkIntfTableSize)
		return FAILED;
	*isDrop = (rtl8651NetworkIntfTable[netIdx].aclDefPermit==0)? TRUE: FALSE;
	return SUCCESS;
}



/*
1. Install rule to tail
2. Accept duplicated rule
*/
static int8 _rtl8651_sameMacRule(rtl8651_tblDrvMacAclRule_t * rule1, rtl8651_tblDrvMacAclRule_t * rule2) {
	if(rule1->actionType == rule2->actionType && rule1->typeLen == rule2->typeLen && rule1->typeLenMask == rule2->typeLenMask &&
		memcmp(&rule1->dstMac, &rule2->dstMac, sizeof(ether_addr_t)) == 0 && memcmp(&rule1->dstMacMask, &rule2->dstMacMask, sizeof(ether_addr_t)) == 0 &&
		memcmp(&rule1->srcMac, &rule2->srcMac, sizeof(ether_addr_t)) == 0 && memcmp(&rule1->srcMacMask, &rule2->srcMacMask, sizeof(ether_addr_t)) == 0)
		return TRUE;
	return FALSE;
}

int32 rtl8651_addMacAcl(uint16 vid, int8 inputCheck, rtl8651_tblDrvMacAclRule_t * rule) {
	uint32 vidx = rtl8651_vlanTableIndex(vid);
	rtl8651_tblDrv_aclMacEntry_t * tempMacAclPtr;

	if(vid < 1 || vid > 4094) {//vid should be legal vlan ID
		 
		return TBLDRV_EINVVLAN;//Invalid VID
	}
	if(rule == NULL) {
		 
		return TBLDRV_EINVVAL;//No rule specified
	}
	//Only PERMIT and DROP are allowed
	if (rule->actionType != RTL8651_ACL_DROP && rule->actionType != RTL8651_ACL_PERMIT && rule->actionType != RTL8651_ACL_CPU) {
		 
		return TBLDRV_EINVVAL;
	}
	RTL8651_LOCK_RESOURCE();//Lock resource		
	 
	if(rtl8651VlanTable[vidx].valid == 0 || rtl8651VlanTable[vidx].vid != vid ){
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		 
		return TBLDRV_EINVVLAN;//VLAN not exist
	}
	if(rtl8651FreeMacAclList == NULL) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		 
		return TBLDRV_ENOFREEBUF;//No mac acl entry available
	}
	//Allocate mac acl entry
	tempMacAclPtr = rtl8651FreeMacAclList;
	rtl8651FreeMacAclList = tempMacAclPtr->next;
	tempMacAclPtr->rule = *rule;
	tempMacAclPtr->next = NULL;
	if(inputCheck == TRUE) {
		tempMacAclPtr->prev = rtl8651VlanTable[vidx].inAclTail;
		if(rtl8651VlanTable[vidx].inAclHead == (rtl8651_tblDrv_aclMacEntry_t *)NULL) {
			assert( rtl8651VlanTable[vidx].inAclTail==NULL);
			rtl8651VlanTable[vidx].inAclHead = rtl8651VlanTable[vidx].inAclTail = tempMacAclPtr;
		}
		else {
			assert( rtl8651VlanTable[vidx].inAclTail);
			 rtl8651VlanTable[vidx].inAclTail->next = tempMacAclPtr;
			 rtl8651VlanTable[vidx].inAclTail = tempMacAclPtr;
		}
	}
	else {
		tempMacAclPtr->prev = rtl8651VlanTable[vidx].outAclTail;
		if(rtl8651VlanTable[vidx].outAclHead == (rtl8651_tblDrv_aclMacEntry_t *)NULL) {
			assert( rtl8651VlanTable[vidx].outAclTail==NULL);
			rtl8651VlanTable[vidx].outAclHead = rtl8651VlanTable[vidx].outAclTail = tempMacAclPtr;
		}
		else {
			assert( rtl8651VlanTable[vidx].outAclTail);
			 rtl8651VlanTable[vidx].outAclTail->next = tempMacAclPtr;
			 rtl8651VlanTable[vidx].outAclTail = tempMacAclPtr;
		}
	}
	_rtl8651_updateAsicVlan(vid);
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

/*
1. Search for matching entry from tail
2. If inputCheck==FALSE, this means rule is output check
*/
int32 rtl8651_delMacAcl(uint16 vid, int8 inputCheck, rtl8651_tblDrvMacAclRule_t * rule) {
	uint32 vidx = rtl8651_vlanTableIndex(vid);
	rtl8651_tblDrv_aclMacEntry_t * tempMacAclPtr;

	if(vid < 1 || vid > 4094) {//vid should be legal vlan ID
		 
		return TBLDRV_EINVVLAN;//Invalid VID
	}
	if(rule == NULL) {
		 
		return TBLDRV_EINVVAL;
	}
	RTL8651_LOCK_RESOURCE();//Lock resource		
	 
	if(rtl8651VlanTable[vidx].valid == 0 || rtl8651VlanTable[vidx].vid != vid ) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		 
		return TBLDRV_EINVVAL;//VLAN not exist
	}
	if(inputCheck == TRUE) {
		tempMacAclPtr = rtl8651VlanTable[vidx].inAclTail;
		while(tempMacAclPtr) {
			if(_rtl8651_sameMacRule(&tempMacAclPtr->rule, rule) == TRUE) {
				if(tempMacAclPtr->next == NULL) {
					assert(tempMacAclPtr == rtl8651VlanTable[vidx].inAclTail);
					rtl8651VlanTable[vidx].inAclTail = tempMacAclPtr->prev;
				}
				else
					tempMacAclPtr->next->prev = tempMacAclPtr->prev;
				if(tempMacAclPtr->prev == NULL) {
					assert(tempMacAclPtr == rtl8651VlanTable[vidx].inAclHead);
					rtl8651VlanTable[vidx].inAclHead = tempMacAclPtr->next;
				}
				else
					tempMacAclPtr->prev->next = tempMacAclPtr->next;
				break;
			}
			assert(tempMacAclPtr->prev || tempMacAclPtr == rtl8651VlanTable[vidx].inAclHead);
			tempMacAclPtr = tempMacAclPtr->prev;
		}
	}
	else {// inputCheck == FALSE
		tempMacAclPtr = rtl8651VlanTable[vidx].outAclTail;
		while(tempMacAclPtr) {
			if(_rtl8651_sameMacRule(&tempMacAclPtr->rule, rule) == TRUE)  {
				if(tempMacAclPtr->next == NULL) {
					assert(tempMacAclPtr == rtl8651VlanTable[vidx].outAclTail);
					rtl8651VlanTable[vidx].outAclTail = tempMacAclPtr->prev;
				}
				else
					tempMacAclPtr->next->prev = tempMacAclPtr->prev;
				if(tempMacAclPtr->prev == NULL) {
					assert(tempMacAclPtr == rtl8651VlanTable[vidx].outAclHead);
					rtl8651VlanTable[vidx].outAclHead = tempMacAclPtr->next;
				}
				else
					tempMacAclPtr->prev->next = tempMacAclPtr->next;
				break;
			}
			assert(tempMacAclPtr->prev || tempMacAclPtr == rtl8651VlanTable[vidx].outAclHead);
			tempMacAclPtr = tempMacAclPtr->prev;
		}
	}
	if(tempMacAclPtr) {//Entry deleted, release to free list and call VLAN update to update ACL setting
		tempMacAclPtr->next = rtl8651FreeMacAclList;
		rtl8651FreeMacAclList = tempMacAclPtr;
		_rtl8651_updateAsicVlan(vid);
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return SUCCESS;
	}
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	 
	return TBLDRV_EENTRYNOTFOUND;//Specified entry not found
}
/*
1.Remove all specified acl list (input filter or output filter)
*/
int32 rtl8651_flushMacAcl(uint16 vid, int8 inputCheck) { //If inputCheck==FALSE, this means rule is output check
	uint32 vidx = rtl8651_vlanTableIndex(vid);
	rtl8651_tblDrv_aclMacEntry_t * tempMacAclPtr;

	if(vid < 1 || vid > 4094) {//vid should be legal vlan ID
		 
		return TBLDRV_EINVVLAN;//Invalid VID
	}
	RTL8651_LOCK_RESOURCE();//Lock resource		
	 
	if(rtl8651VlanTable[vidx].valid == 0 || rtl8651VlanTable[vidx].vid != vid ) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		 
		return TBLDRV_EINVVLAN;//VLAN not exist
	}
	if(inputCheck == TRUE) {	
		while(rtl8651VlanTable[vidx].inAclHead) {
			tempMacAclPtr = rtl8651VlanTable[vidx].inAclHead;
			rtl8651VlanTable[vidx].inAclHead = tempMacAclPtr->next;
			tempMacAclPtr->next = rtl8651FreeMacAclList;
			rtl8651FreeMacAclList = tempMacAclPtr;
			if(rtl8651VlanTable[vidx].inAclHead == (rtl8651_tblDrv_aclMacEntry_t *)NULL)
				rtl8651VlanTable[vidx].inAclTail = (rtl8651_tblDrv_aclMacEntry_t *)NULL;
		}
		assert(rtl8651VlanTable[vidx].inAclHead == (rtl8651_tblDrv_aclMacEntry_t *)NULL);
		assert(rtl8651VlanTable[vidx].inAclTail == (rtl8651_tblDrv_aclMacEntry_t *)NULL);
	}
	else {
		while(rtl8651VlanTable[vidx].outAclHead) {
			tempMacAclPtr = rtl8651VlanTable[vidx].outAclHead;
			rtl8651VlanTable[vidx].outAclHead = tempMacAclPtr->next;
			tempMacAclPtr->next = rtl8651FreeMacAclList;
			rtl8651FreeMacAclList = tempMacAclPtr;
			if(rtl8651VlanTable[vidx].outAclHead == (rtl8651_tblDrv_aclMacEntry_t *)NULL)
				rtl8651VlanTable[vidx].outAclTail = (rtl8651_tblDrv_aclMacEntry_t *)NULL;
		}
		assert(rtl8651VlanTable[vidx].outAclHead == (rtl8651_tblDrv_aclMacEntry_t *)NULL);
		assert(rtl8651VlanTable[vidx].outAclTail == (rtl8651_tblDrv_aclMacEntry_t *)NULL);
	}
	_rtl8651_updateAsicVlan(vid);
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

static int8 _rtl8651_sameIpRule(rtl8651_tblDrvIpAclRule_t * rule1, rtl8651_tblDrvIpAclRule_t * rule2) {
	if(rule1->srcIpAddr != rule2->srcIpAddr || rule1->srcIpAddrMask != rule2->srcIpAddrMask ||
		rule1->dstIpAddr != rule2->dstIpAddr || rule1->dstIpAddrMask != rule2->dstIpAddrMask ||
		rule1->tos != rule2->tos || rule1->tosMask != rule2->tosMask || rule1->actionType != rule2->actionType)
		return FALSE;//Common part mismatch
	if(rule1->ruleType != rule2->ruleType)
		return FALSE;//Type mismatch
	switch(rule1->ruleType) {
		case RTL8651_ACL_IP:
			if(rule1->is.ip.proto == rule2->is.ip.proto && rule1->is.ip.protoMask == rule2->is.ip.protoMask &&
				rule1->is.ip.ipFlag == rule2->is.ip.ipFlag && rule1->is.ip.flagMask == rule2->is.ip.flagMask)
				return TRUE;//IP specified part equal
			break;
		case RTL8651_ACL_ICMP:
			if(rule1->is.icmp.type == rule2->is.icmp.type && rule1->is.icmp.typeMask == rule2->is.icmp.typeMask &&
				rule1->is.icmp.code == rule2->is.icmp.code && rule1->is.icmp.codeMask == rule2->is.icmp.codeMask)
				return TRUE;//ICMP specified part equal
			break;
		case RTL8651_ACL_IGMP:
			if(rule1->is.igmp.type == rule2->is.igmp.type && rule1->is.igmp.typeMask == rule2->is.igmp.typeMask)
				return TRUE;//ICMP specified part equal
			break;
		case RTL8651_ACL_TCP:
			if(rule1->is.tcp.tcpflag == rule2->is.tcp.tcpflag && rule1->is.tcp.flagMask == rule2->is.tcp.flagMask &&
				rule1->is.tcp.srcPortUpperBound == rule2->is.tcp.srcPortUpperBound && rule1->is.tcp.srcPortLowerBound == rule2->is.tcp.srcPortLowerBound &&
				rule1->is.tcp.dstPortUpperBound == rule2->is.tcp.dstPortUpperBound && rule1->is.tcp.dstPortLowerBound == rule2->is.tcp.dstPortLowerBound)
				return TRUE;//TCP specified part euqal
			break;	
		case RTL8651_ACL_UDP:
			if(rule1->is.udp.srcPortUpperBound == rule2->is.udp.srcPortUpperBound && rule1->is.udp.srcPortLowerBound == rule2->is.udp.srcPortLowerBound &&
				rule1->is.udp.dstPortUpperBound == rule2->is.udp.dstPortUpperBound && rule1->is.udp.dstPortLowerBound == rule2->is.udp.dstPortLowerBound)
				return TRUE;//UDP specified part euqal
			break;	
	}
	return FALSE;//Unknown rule type
}

/*
1. Add rule to current tail
*/
int32 rtl8651_addVlanIpAcl(uint16 vid, int8 inputCheck, rtl8651_tblDrvIpAclRule_t * rule) {
	uint32 vidx = rtl8651_vlanTableIndex(vid);
	rtl8651_tblDrv_aclIpEntry_t * tempIpAclPtr;
		
	if(vid < 1 || vid > 4094) {//vid should be legal vlan ID
		 
		return TBLDRV_EINVVLAN;//Invalid VID
	}
	if(rule == NULL) {
		 
		return TBLDRV_EINVVAL;//No rule specified
	}
	//Only PERMIT and DROP are allowed
	if (rule->actionType != RTL8651_ACL_DROP && rule->actionType != RTL8651_ACL_PERMIT && rule->actionType != RTL8651_ACL_CPU) {
		 
		return TBLDRV_EINVVAL;
	}
	RTL8651_LOCK_RESOURCE();//Lock resource		
	 
	if(rtl8651VlanTable[vidx].valid == 0 || rtl8651VlanTable[vidx].vid != vid ) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		 
		return TBLDRV_EINVVLAN;//VLAN not exist
	}
	
	if(rtl8651FreeIpAclList == NULL) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		 
		return TBLDRV_ENOFREEBUF;//No more rule space available
	}
	//Allocate rule space
	tempIpAclPtr = rtl8651FreeIpAclList;
	rtl8651FreeIpAclList = tempIpAclPtr->next;
	tempIpAclPtr->rule = *rule;
	tempIpAclPtr->next = NULL;
	if(inputCheck == TRUE) {
		tempIpAclPtr->prev = rtl8651VlanTable[vidx].inIpAclTail;
		if(rtl8651VlanTable[vidx].inIpAclHead == NULL) {
			assert(rtl8651VlanTable[vidx].inIpAclTail == NULL);
			rtl8651VlanTable[vidx].inIpAclHead = rtl8651VlanTable[vidx].inIpAclTail = tempIpAclPtr;
		}
		else {
			assert(rtl8651VlanTable[vidx].inIpAclTail);
			rtl8651VlanTable[vidx].inIpAclTail->next = tempIpAclPtr;
			rtl8651VlanTable[vidx].inIpAclTail = tempIpAclPtr;
		}
	}
	else {
		tempIpAclPtr->prev = rtl8651VlanTable[vidx].outIpAclTail;
		if(rtl8651VlanTable[vidx].outIpAclHead == NULL) {
			assert(rtl8651VlanTable[vidx].outIpAclTail == NULL);
			rtl8651VlanTable[vidx].outIpAclHead = rtl8651VlanTable[vidx].outIpAclTail = tempIpAclPtr;
		}
		else {
			assert(rtl8651VlanTable[vidx].outIpAclTail);
			rtl8651VlanTable[vidx].outIpAclTail->next = tempIpAclPtr;
			rtl8651VlanTable[vidx].outIpAclTail = tempIpAclPtr;
		}
	}
	_rtl8651_updateAsicVlan(vid);
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

/*
1. Find and remove rule from tail
*/
int32 rtl8651_delVlanIpAcl(uint16 vid, int8 inputCheck, rtl8651_tblDrvIpAclRule_t * rule) {
	uint32 vidx = rtl8651_vlanTableIndex(vid);
	rtl8651_tblDrv_aclIpEntry_t * tempIpAclPtr;
		
	if(vid < 1 || vid > 4094) {//vid should be legal vlan ID
		 
		return TBLDRV_EINVVLAN;//Invalid VID
	}
	if(rule == NULL) {
		 
		return TBLDRV_EINVVAL;//No rule specified
	}
	RTL8651_LOCK_RESOURCE();//Lock resource		
	if(rtl8651VlanTable[vidx].valid == 0 || rtl8651VlanTable[vidx].vid != vid ) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		 
		return TBLDRV_EINVVLAN;//VLAN not exist
	}
	
	if(inputCheck == TRUE) {
		tempIpAclPtr = rtl8651VlanTable[vidx].inIpAclTail;
		while(tempIpAclPtr) {
			if(_rtl8651_sameIpRule(&tempIpAclPtr->rule, rule) == TRUE) {
				if(tempIpAclPtr->next == NULL) {
					assert(tempIpAclPtr == rtl8651VlanTable[vidx].inIpAclTail);
					rtl8651VlanTable[vidx].inIpAclTail = tempIpAclPtr->prev;
				}
				else
					tempIpAclPtr->next->prev = tempIpAclPtr->prev;
				if(tempIpAclPtr->prev == NULL) {
					assert(tempIpAclPtr == rtl8651VlanTable[vidx].inIpAclHead);
					rtl8651VlanTable[vidx].inIpAclHead = tempIpAclPtr->next;
				}
				else
					tempIpAclPtr->prev->next = tempIpAclPtr->next;
				break;
			}
			assert(tempIpAclPtr->prev || tempIpAclPtr ==  rtl8651VlanTable[vidx].inIpAclHead);
			tempIpAclPtr = tempIpAclPtr->prev;
		}
	}
	else {
		tempIpAclPtr = rtl8651VlanTable[vidx].outIpAclTail;
		while(tempIpAclPtr) {
			if(_rtl8651_sameIpRule(&tempIpAclPtr->rule, rule) == TRUE) {
				if(tempIpAclPtr->next == NULL) {
					assert(tempIpAclPtr == rtl8651VlanTable[vidx].outIpAclTail);
					rtl8651VlanTable[vidx].outIpAclTail = tempIpAclPtr->prev;
				}
				else
					tempIpAclPtr->next->prev = tempIpAclPtr->prev;
				if(tempIpAclPtr->prev == NULL) {
					assert(tempIpAclPtr == rtl8651VlanTable[vidx].outIpAclHead);
					rtl8651VlanTable[vidx].outIpAclHead = tempIpAclPtr->next;
				}
				else
					tempIpAclPtr->prev->next = tempIpAclPtr->next;
				break;
			}
			assert(tempIpAclPtr->prev || tempIpAclPtr ==  rtl8651VlanTable[vidx].outIpAclHead);
			tempIpAclPtr = tempIpAclPtr->prev;
		}
	}
	if(tempIpAclPtr) {//Entry deleted, release to free list and call VLAN update to update ACL setting
		tempIpAclPtr->next = rtl8651FreeIpAclList;
		rtl8651FreeIpAclList = tempIpAclPtr;
		_rtl8651_updateAsicVlan(vid);
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return SUCCESS;
	}
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	 
	return TBLDRV_EENTRYNOTFOUND;//Specified entry not found
}

/*
1. Remove all rules attached to the specific list
*/
int32 rtl8651_flushVlanIpAcl(uint16 vid, int8 inputCheck) {
	uint32 vidx = rtl8651_vlanTableIndex(vid);
	rtl8651_tblDrv_aclIpEntry_t * tempIpAclPtr;
		
	if(vid < 1 || vid > 4094) {//vid should be legal vlan ID
		 
		return TBLDRV_EINVVLAN;//Invalid VID
	}
	RTL8651_LOCK_RESOURCE();//Lock resource
	 
	if(rtl8651VlanTable[vidx].valid == 0 || rtl8651VlanTable[vidx].vid != vid ) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		 
		return TBLDRV_EINVVLAN;//VLAN not exist
	}
	
	if(inputCheck == TRUE) {
		while(rtl8651VlanTable[vidx].inIpAclHead) {
			tempIpAclPtr = rtl8651VlanTable[vidx].inIpAclHead;
			rtl8651VlanTable[vidx].inIpAclHead = tempIpAclPtr->next;
			tempIpAclPtr->next = rtl8651FreeIpAclList;
			rtl8651FreeIpAclList = tempIpAclPtr;
			if(rtl8651VlanTable[vidx].inIpAclHead == (rtl8651_tblDrv_aclIpEntry_t *)NULL)
				rtl8651VlanTable[vidx].inIpAclTail = (rtl8651_tblDrv_aclIpEntry_t *)NULL;
		}
		assert(rtl8651VlanTable[vidx].inIpAclHead == NULL);
		assert(rtl8651VlanTable[vidx].inIpAclTail == NULL);
	}
	else {
		while(rtl8651VlanTable[vidx].outIpAclHead) {
			tempIpAclPtr = rtl8651VlanTable[vidx].outIpAclHead;
			rtl8651VlanTable[vidx].outIpAclHead = tempIpAclPtr->next;
			tempIpAclPtr->next = rtl8651FreeIpAclList;
			rtl8651FreeIpAclList = tempIpAclPtr;
			if(rtl8651VlanTable[vidx].outIpAclHead == (rtl8651_tblDrv_aclIpEntry_t *)NULL)
				rtl8651VlanTable[vidx].outIpAclTail = (rtl8651_tblDrv_aclIpEntry_t *)NULL;
		}
		assert(rtl8651VlanTable[vidx].outIpAclHead == NULL);
		assert(rtl8651VlanTable[vidx].outIpAclTail == NULL);
	}
	_rtl8651_updateAsicVlan(vid);
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}


/*
1. Add rule to current tail
*/
int32 rtl8651_addIpAcl(int8 * ifName, int8 inputCheck, rtl8651_tblDrvIpAclRule_t * rule) {
	uint32 ipIntfIdx;
	rtl8651_tblDrv_aclIpEntry_t * tempIpAclPtr;
		
	if(ifName == NULL) {
		return(TBLDRV_EINVNETIFNAME); 
	}
	if (rule == NULL) {
		return(TBLDRV_EINVVAL); 
	}
	//Only PERMIT and DROP are allowed
	if (rule->actionType != RTL8651_ACL_DROP && rule->actionType != RTL8651_ACL_PERMIT && rule->actionType != RTL8651_ACL_CPU) {
		return(TBLDRV_EINVVAL); 
	}
	RTL8651_LOCK_RESOURCE();//Lock resource
	 
	for(ipIntfIdx=0; ipIntfIdx< rtl8651_tblDrvPara.networkIntfTableSize; ipIntfIdx++)
		if(rtl8651NetworkIntfTable[ipIntfIdx].valid == 1 && strncmp(rtl8651NetworkIntfTable[ipIntfIdx].ifName, ifName, RTL8651_IPINTF_NAME_LEN) == 0) 
			break;
	if(ipIntfIdx == rtl8651_tblDrvPara.networkIntfTableSize) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		 
		return TBLDRV_EINVNETIFNAME;//Interface specified not exist
	}
	
	if(rtl8651FreeIpAclList == NULL) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		 
		return TBLDRV_ENOFREEBUF;//No more rule space available
	}
	//Allocate rule space
	tempIpAclPtr = rtl8651FreeIpAclList;
	rtl8651FreeIpAclList = tempIpAclPtr->next;
	tempIpAclPtr->rule = *rule;
	tempIpAclPtr->next = NULL;
	if(inputCheck == TRUE) {
		tempIpAclPtr->prev = rtl8651NetworkIntfTable[ipIntfIdx].inAclTail;
		if(rtl8651NetworkIntfTable[ipIntfIdx].inAclHead == NULL) {
			assert(rtl8651NetworkIntfTable[ipIntfIdx].inAclTail == NULL);
			rtl8651NetworkIntfTable[ipIntfIdx].inAclHead = rtl8651NetworkIntfTable[ipIntfIdx].inAclTail = tempIpAclPtr;
		}
		else {
			assert(rtl8651NetworkIntfTable[ipIntfIdx].inAclTail);
			rtl8651NetworkIntfTable[ipIntfIdx].inAclTail->next = tempIpAclPtr;
			rtl8651NetworkIntfTable[ipIntfIdx].inAclTail = tempIpAclPtr;
		}
	}
	else {
		tempIpAclPtr->prev = rtl8651NetworkIntfTable[ipIntfIdx].outAclTail;
		if(rtl8651NetworkIntfTable[ipIntfIdx].outAclHead == NULL) {
			assert(rtl8651NetworkIntfTable[ipIntfIdx].outAclTail == NULL);
			rtl8651NetworkIntfTable[ipIntfIdx].outAclHead = rtl8651NetworkIntfTable[ipIntfIdx].outAclTail = tempIpAclPtr;
		}
		else {
			assert(rtl8651NetworkIntfTable[ipIntfIdx].outAclTail);
			rtl8651NetworkIntfTable[ipIntfIdx].outAclTail->next = tempIpAclPtr;
			rtl8651NetworkIntfTable[ipIntfIdx].outAclTail = tempIpAclPtr;
		}
	}
	//If link layer specified, update asic vlan information
	if(rtl8651NetworkIntfTable[ipIntfIdx].linkLayerType == RTL8651_LL_VLAN)//VLAN
		_rtl8651_updateAsicVlan(rtl8651NetworkIntfTable[ipIntfIdx].linkLayerIndex);
	else if(rtl8651NetworkIntfTable[ipIntfIdx].linkLayerType == RTL8651_LL_PPPOE) {//PPPoE session
		uint32 i;
		for(i=0; i<RTL8651_PPPOE_NUMBER; i++)
			if(rtl8651PppoeTable[i].valid == 1 && rtl8651PppoeTable[i].objectId == rtl8651NetworkIntfTable[ipIntfIdx].linkLayerIndex)
				break;
		if(i < RTL8651_PPPOE_NUMBER)
			_rtl8651_updateAsicVlan(rtl8651PppoeTable[i].vid);
	}
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

/*
1. Find and remove rule from tail
*/
int32 rtl8651_delIpAcl(int8 * ifName, int8 inputCheck, rtl8651_tblDrvIpAclRule_t * rule) {
	uint32 ipIntfIdx;
	rtl8651_tblDrv_aclIpEntry_t * tempIpAclPtr;
		
	if(ifName == NULL) {
		
		return TBLDRV_EINVNETIFNAME;
	}
	if (rule == NULL) {
		 
		return TBLDRV_EINVVAL;//No rule specified
	}
	RTL8651_LOCK_RESOURCE();//Lock resource
	 
	for(ipIntfIdx=0; ipIntfIdx< rtl8651_tblDrvPara.networkIntfTableSize; ipIntfIdx++)
		if(rtl8651NetworkIntfTable[ipIntfIdx].valid == 1 && strncmp(rtl8651NetworkIntfTable[ipIntfIdx].ifName, ifName, RTL8651_IPINTF_NAME_LEN) == 0) 
			break;
	if(ipIntfIdx == rtl8651_tblDrvPara.networkIntfTableSize) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		 
		return TBLDRV_EINVNETIFNAME;//Interface specified not exist
	}
	
	if(inputCheck == TRUE) {
		tempIpAclPtr = rtl8651NetworkIntfTable[ipIntfIdx].inAclTail;
		while(tempIpAclPtr) {
			if(_rtl8651_sameIpRule(&tempIpAclPtr->rule, rule) == TRUE) {
				if(tempIpAclPtr->next == NULL) {
					assert(tempIpAclPtr == rtl8651NetworkIntfTable[ipIntfIdx].inAclTail);
					rtl8651NetworkIntfTable[ipIntfIdx].inAclTail = tempIpAclPtr->prev;
				}
				else
					tempIpAclPtr->next->prev = tempIpAclPtr->prev;
				if(tempIpAclPtr->prev == NULL) {
					assert(tempIpAclPtr == rtl8651NetworkIntfTable[ipIntfIdx].inAclHead);
					rtl8651NetworkIntfTable[ipIntfIdx].inAclHead = tempIpAclPtr->next;
				}
				else
					tempIpAclPtr->prev->next = tempIpAclPtr->next;
				break;
			}
			assert(tempIpAclPtr->prev || tempIpAclPtr ==  rtl8651NetworkIntfTable[ipIntfIdx].inAclHead);
			tempIpAclPtr = tempIpAclPtr->prev;
		}
	}
	else {
		tempIpAclPtr = rtl8651NetworkIntfTable[ipIntfIdx].outAclTail;
		while(tempIpAclPtr) {
			if(_rtl8651_sameIpRule(&tempIpAclPtr->rule, rule) == TRUE) {
				if(tempIpAclPtr->next == NULL) {
					assert(tempIpAclPtr == rtl8651NetworkIntfTable[ipIntfIdx].outAclTail);
					rtl8651NetworkIntfTable[ipIntfIdx].outAclTail = tempIpAclPtr->prev;
				}
				else
					tempIpAclPtr->next->prev = tempIpAclPtr->prev;
				if(tempIpAclPtr->prev == NULL) {
					assert(tempIpAclPtr == rtl8651NetworkIntfTable[ipIntfIdx].outAclHead);
					rtl8651NetworkIntfTable[ipIntfIdx].outAclHead = tempIpAclPtr->next;
				}
				else
					tempIpAclPtr->prev->next = tempIpAclPtr->next;
				break;
			}
			assert(tempIpAclPtr->prev || tempIpAclPtr ==  rtl8651NetworkIntfTable[ipIntfIdx].outAclHead);
			tempIpAclPtr = tempIpAclPtr->prev;
		}
	}
	if(tempIpAclPtr) {//Entry deleted, release to free list and call VLAN update to update ACL setting
		tempIpAclPtr->next = rtl8651FreeIpAclList;
		rtl8651FreeIpAclList = tempIpAclPtr;
		if(rtl8651NetworkIntfTable[ipIntfIdx].linkLayerType == RTL8651_LL_VLAN)//VLAN
			_rtl8651_updateAsicVlan(rtl8651NetworkIntfTable[ipIntfIdx].linkLayerIndex);
		else if(rtl8651NetworkIntfTable[ipIntfIdx].linkLayerType == RTL8651_LL_PPPOE) {//PPPoE session
			uint32 i;
			for(i=0; i<RTL8651_PPPOE_NUMBER; i++)
				if(rtl8651PppoeTable[i].valid == 1 && rtl8651PppoeTable[i].objectId == rtl8651NetworkIntfTable[ipIntfIdx].linkLayerIndex)
					break;
			if(i < RTL8651_PPPOE_NUMBER)
				_rtl8651_updateAsicVlan(rtl8651PppoeTable[i].vid);
		}
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		return SUCCESS;
	}
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	 
	return TBLDRV_EENTRYNOTFOUND;//Specified entry not found
}

/*
1. Remove all rules attached to the specific list
*/
int32 rtl8651_flushIpAcl(int8 * ifName, int8 inputCheck) {
	uint32 ipIntfIdx;
	rtl8651_tblDrv_aclIpEntry_t * tempIpAclPtr;
		
	if(ifName == NULL) {
		 
		return TBLDRV_EINVNETIFNAME;//No rule specified
	}
	RTL8651_LOCK_RESOURCE();//Lock resource
	 
	for(ipIntfIdx=0; ipIntfIdx< rtl8651_tblDrvPara.networkIntfTableSize; ipIntfIdx++)
		if(rtl8651NetworkIntfTable[ipIntfIdx].valid == 1 && strncmp(rtl8651NetworkIntfTable[ipIntfIdx].ifName, ifName, RTL8651_IPINTF_NAME_LEN) == 0) 
			break;
	if(ipIntfIdx == rtl8651_tblDrvPara.networkIntfTableSize) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		 
		return TBLDRV_EINVNETIFNAME;//Interface specified not exist
	}
	
	if(inputCheck == TRUE) {
		while(rtl8651NetworkIntfTable[ipIntfIdx].inAclHead) {
			tempIpAclPtr = rtl8651NetworkIntfTable[ipIntfIdx].inAclHead;
			rtl8651NetworkIntfTable[ipIntfIdx].inAclHead = tempIpAclPtr->next;
			tempIpAclPtr->next = rtl8651FreeIpAclList;
			rtl8651FreeIpAclList = tempIpAclPtr;
			if(rtl8651NetworkIntfTable[ipIntfIdx].inAclHead == (rtl8651_tblDrv_aclIpEntry_t *)NULL)
				rtl8651NetworkIntfTable[ipIntfIdx].inAclTail = (rtl8651_tblDrv_aclIpEntry_t *)NULL;
		}
		assert(rtl8651NetworkIntfTable[ipIntfIdx].inAclHead == NULL);
		assert(rtl8651NetworkIntfTable[ipIntfIdx].inAclTail == NULL);
	}
	else {
		while(rtl8651NetworkIntfTable[ipIntfIdx].outAclHead) {
			tempIpAclPtr = rtl8651NetworkIntfTable[ipIntfIdx].outAclHead;
			rtl8651NetworkIntfTable[ipIntfIdx].outAclHead = tempIpAclPtr->next;
			tempIpAclPtr->next = rtl8651FreeIpAclList;
			rtl8651FreeIpAclList = tempIpAclPtr;
			if(rtl8651NetworkIntfTable[ipIntfIdx].outAclHead == (rtl8651_tblDrv_aclIpEntry_t *)NULL)
				rtl8651NetworkIntfTable[ipIntfIdx].outAclTail = (rtl8651_tblDrv_aclIpEntry_t *)NULL;
		}
		assert(rtl8651NetworkIntfTable[ipIntfIdx].outAclHead == NULL);
		assert(rtl8651NetworkIntfTable[ipIntfIdx].outAclTail == NULL);
	}
	if(rtl8651NetworkIntfTable[ipIntfIdx].linkLayerType == RTL8651_LL_VLAN)//VLAN
		_rtl8651_updateAsicVlan(rtl8651NetworkIntfTable[ipIntfIdx].linkLayerIndex);
	else if(rtl8651NetworkIntfTable[ipIntfIdx].linkLayerType == RTL8651_LL_PPPOE) {//PPPoE session
		uint32 i;
		for(i=0; i<RTL8651_PPPOE_NUMBER; i++)
			if(rtl8651PppoeTable[i].valid == 1 && rtl8651PppoeTable[i].objectId == rtl8651NetworkIntfTable[ipIntfIdx].linkLayerIndex)
				break;
		if(i < RTL8651_PPPOE_NUMBER)
			_rtl8651_updateAsicVlan(rtl8651PppoeTable[i].vid);
	}
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}


rtl8651_tblDrv_algEntry_t *_rtl8651_getAlgEntry(int8 isTcp, int8 *isServer, int8 *isClient, uint16 sport, uint16 eport) {
	rtl8651_tblDrv_algEntry_t *tmpAlgEntry;

	/* Parameter Check */
	if (isServer == FALSE && isClient == FALSE)
		return (rtl8651_tblDrv_algEntry_t *)0;
	
	/* Go though ALG list */
	tmpAlgEntry = rtl8651AlgList;
	while (tmpAlgEntry) {
		if (tmpAlgEntry->startPort == sport && tmpAlgEntry->endPort == eport) {
			if ((tmpAlgEntry->tcpClient|tmpAlgEntry->tcpServer) == (isTcp==TRUE? 1: 0)) {
				*isServer = (isTcp == TRUE)? tmpAlgEntry->tcpServer : tmpAlgEntry->udpServer;
				*isClient = (isTcp == TRUE)? tmpAlgEntry->tcpClient : tmpAlgEntry->udpClient;
				return tmpAlgEntry;
			}
		}
		tmpAlgEntry = tmpAlgEntry->next;
	}
	return (rtl8651_tblDrv_algEntry_t *)0;
}



static uint32 _rtl8651_countAlgSlotAllocateNumber(rtl8651_tblDrv_algEntry_t * algPtr) {
	rtl8651_tblDrv_algSlot_t * tempAlgSlotPtr = algPtr->nextAlgSlot;
	uint32 count = 0;;

	while(tempAlgSlotPtr) {
		count++;
		tempAlgSlotPtr = tempAlgSlotPtr->next;
	}
	return count;
}

static void _rtl8651_updateAsicAlg(void) {
	uint32 algIdx, algSlotCount;
	uint16 algPort;
	rtl8651_tblDrv_algEntry_t * tempAlgPtr;
	rtl8651_tblDrv_algSlot_t * tempAlgSlotPtr;

	tempAlgPtr = rtl8651AlgList;
	while(tempAlgPtr) {
		if(tempAlgPtr->endPort - tempAlgPtr->startPort < RTL8651_ALGSLOT_THRESHOLD && tempAlgPtr->nextAlgSlot == NULL) {//Try to allocate ALG table entry when port number smaller than threshold
			tempAlgSlotPtr = rtl8651FreeAlgSlotList;
			algSlotCount = 0;
			while(tempAlgSlotPtr) {
				algSlotCount++;
				tempAlgSlotPtr = tempAlgSlotPtr->next;
			}
			if(tempAlgPtr->endPort - tempAlgPtr->startPort < algSlotCount) {//Can allocate ALG table entry since the slot number is enough
				algPort = tempAlgPtr->endPort;
				algIdx = 0;
				while(algPort >= tempAlgPtr->startPort) {
					while(rtl8651AlgAsicArrangementTable[algIdx].valid == 1 && algIdx < RTL8651_ALGTBL_SIZE)
						algIdx++;
					assert(algIdx < RTL8651_ALGTBL_SIZE);//Arrangement table and freeAlgSlot list should sync.
					tempAlgSlotPtr = rtl8651FreeAlgSlotList;
					rtl8651FreeAlgSlotList = tempAlgSlotPtr->next;
					tempAlgSlotPtr->next = tempAlgPtr->nextAlgSlot;
					tempAlgPtr->nextAlgSlot = tempAlgSlotPtr;
					tempAlgSlotPtr->algSlot = algIdx;
					rtl8651AlgAsicArrangementTable[algIdx].port = algPort;
					rtl8651AlgAsicArrangementTable[algIdx].valid = 1;
					rtl8651_setAsicAlg(algIdx, algPort);
					algPort--;
				}
			}
		}
		tempAlgPtr = tempAlgPtr->next;
	}
}

/*
1. Assume user will not overlap configure. If enabling configuration is overlapped, this must be error configuration.
*/
int32 rtl8651_addALGRule(uint16 startPort, uint16 endPort, int8 isTcp, int8 isServer, int8 isClient) {
	rtl8651_tblDrv_algEntry_t * tempAlgPtr;
	
	if(isServer == FALSE && isClient == FALSE) {
		 
		return TBLDRV_EINVVAL;//Meaningless parameter
	}
	if(startPort > endPort) {
		 
		return TBLDRV_ESTARTGTEND;//Range specification error
	}
	//Check whether entry exist. (Only exactly same range is allowed. If the range are overlapped but not the same, return fail.
	RTL8651_LOCK_RESOURCE();//Lock resource
	 
	tempAlgPtr = rtl8651AlgList;
	while(tempAlgPtr) {
		if(tempAlgPtr->startPort == startPort && tempAlgPtr->endPort == endPort)
			break;
		if((tempAlgPtr->startPort <= startPort && tempAlgPtr->endPort >= startPort) ||
			(tempAlgPtr->startPort <= endPort && tempAlgPtr->endPort >= endPort)) {
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			 
			return TBLDRV_ERANGEOVERLAP;//Range overlap but not exactly same
		}
		tempAlgPtr = tempAlgPtr->next;
	}
	if(tempAlgPtr == NULL) {//No existed configuration, try to allocate new one
		if(rtl8651FreeAlgList == NULL) {
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			 
			return TBLDRV_ENOFREEBUF;//Unable to allocate new alg entry
		}
		tempAlgPtr = rtl8651FreeAlgList;
		rtl8651FreeAlgList = tempAlgPtr->next;
		tempAlgPtr->next = rtl8651AlgList;
		rtl8651AlgList = tempAlgPtr;
		tempAlgPtr->startPort = startPort;
		tempAlgPtr->endPort = endPort;
		tempAlgPtr->tcpServer = 0;
		tempAlgPtr->tcpClient = 0;
		tempAlgPtr->udpServer = 0;
		tempAlgPtr->udpClient = 0;
		tempAlgPtr->nextAlgSlot = NULL;
		_rtl8651_updateAsicAlg();
	}
	//Configure trapping property
	if(isTcp == TRUE) {
		if((isServer == TRUE && tempAlgPtr->tcpServer == 1) || (isClient == TRUE && tempAlgPtr->tcpClient == 1)) {
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			//return FAILED;//Not allow enable already enabled item
			return SUCCESS;
		}
		if(isServer == TRUE)
			tempAlgPtr->tcpServer = 1;
		if(isClient == TRUE)
			tempAlgPtr->tcpClient = 1;
	}
	else {
		assert(isTcp == FALSE);
		if((isServer == TRUE && tempAlgPtr->udpServer == 1) || (isClient == TRUE && tempAlgPtr->udpClient == 1)) {
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			//return FAILED;//Not allow enable already enabled item
			return SUCCESS;
		}
		if(isServer == TRUE)
			tempAlgPtr->udpServer = 1;
		if(isClient == TRUE)
			tempAlgPtr->udpClient = 1;
	}
	_rtl8651_updateAsicAllVlan();
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}

int32 rtl8651_delALGRule(uint16 startPort, uint16 endPort, int8 isTcp, int8 isServer, int8 isClient) {
	rtl8651_tblDrv_algEntry_t * tempAlgPtr, *trackAlgPtr;
	rtl8651_tblDrv_algSlot_t * tempAlgSlotPtr;
	
	if(isServer == FALSE && isClient == FALSE) {
		 
		return TBLDRV_EINVVAL;//Meaningless parameter
	}
	if(startPort > endPort) {
		 
		return TBLDRV_ESTARTGTEND;//Range specification error
	}
	RTL8651_LOCK_RESOURCE();//Lock resource
	 
	//Check whether entry exist. (Only exactly same range is allowed. If the range are overlapped but not the same, return fail.
	tempAlgPtr = rtl8651AlgList;
	while(tempAlgPtr) {
		if(tempAlgPtr->startPort == startPort && tempAlgPtr->endPort == endPort)
			break;
		if((tempAlgPtr->startPort <= startPort && tempAlgPtr->endPort >= startPort) ||
			(tempAlgPtr->startPort <= endPort && tempAlgPtr->endPort >= endPort)) {
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			 
			return TBLDRV_ERANGEOVERLAP;//Range overlap but not exactly same
		}
		tempAlgPtr = tempAlgPtr->next;
	}
	if(tempAlgPtr == NULL) {
		RTL8651_UNLOCK_RESOURCE();//Unlock resource
		 
		return TBLDRV_EENTRYNOTFOUND;//Entry not find
	}
	if(isTcp == TRUE) {
		if((tempAlgPtr->tcpServer == 0 && isServer == TRUE) || (tempAlgPtr->tcpClient == 0 && isClient == TRUE)) {
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			 
			return TBLDRV_EENTRYNOTFOUND;//Remove not installed service
		}
		if(isServer == TRUE)
			tempAlgPtr->tcpServer = 0;
		if(isClient == TRUE)
			tempAlgPtr->tcpClient = 0;
	}
	else {
		assert(isTcp == FALSE);
		if((tempAlgPtr->udpServer == 0 && isServer == TRUE) || (tempAlgPtr->udpClient == 0 && isClient == TRUE)) {
			RTL8651_UNLOCK_RESOURCE();//Unlock resource
			 
			return TBLDRV_EENTRYNOTFOUND;//Remove not installed service
		}
		if(isServer == TRUE)
			tempAlgPtr->udpServer = 0;
		if(isClient == TRUE)
			tempAlgPtr->udpClient = 0;
	}
	if(tempAlgPtr->tcpServer == 0 && tempAlgPtr->tcpClient == 0 && tempAlgPtr->udpServer == 0 && tempAlgPtr->udpClient == 0) {//Entry no more need to exist
		while(tempAlgPtr->nextAlgSlot) {//Move alg slot back to free list
			tempAlgSlotPtr = tempAlgPtr->nextAlgSlot;
			tempAlgPtr->nextAlgSlot = tempAlgSlotPtr->next;
			assert(rtl8651AlgAsicArrangementTable[tempAlgSlotPtr->algSlot].port >= tempAlgPtr->startPort);
			assert(rtl8651AlgAsicArrangementTable[tempAlgSlotPtr->algSlot].port <= tempAlgPtr->endPort);
			rtl8651AlgAsicArrangementTable[tempAlgSlotPtr->algSlot].valid = 0;
			rtl8651_delAsicAlg(tempAlgSlotPtr->algSlot);
			tempAlgSlotPtr->next = rtl8651FreeAlgSlotList;
			rtl8651FreeAlgSlotList = tempAlgSlotPtr;
		}
		assert(tempAlgPtr->nextAlgSlot == NULL);
		//Put alg entry to free list
		if(rtl8651AlgList == tempAlgPtr)
			rtl8651AlgList = tempAlgPtr->next;
		else {
			trackAlgPtr = rtl8651AlgList;
			while(trackAlgPtr->next && trackAlgPtr->next != tempAlgPtr)
				trackAlgPtr = trackAlgPtr->next;
			assert(trackAlgPtr);
			assert(trackAlgPtr->next = tempAlgPtr);
			trackAlgPtr->next = tempAlgPtr->next;
		}
		tempAlgPtr->next = rtl8651FreeAlgList;
		rtl8651FreeAlgList = tempAlgPtr;
		_rtl8651_updateAsicAlg();
	}
	_rtl8651_updateAsicAllVlan();
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
	return SUCCESS;
}


void rtl8651_timeUpdate(uint32 secPassed) {

	RTL8651_LOCK_RESOURCE();//Lock resource
	_rtl8651_timeUpdateNaptIcmpTable(secPassed);	/* Update NAPT ICMP aging time */
	_rtl8651_timeUpdateNaptTcpUdpTable(secPassed);	/* Update NAPT TCP/UDP aging time */
	_rtl8651_timeUpdateArp(secPassed);	/* Update ARP entry aging time */
	RTL8651_UNLOCK_RESOURCE();//Unlock resource
}


//Working after rtl8651_tblDrvPara is initialized with configured values
static int32 _rtl8651_tblDrvMemAlloc(void) {
	uint32 i;
	
	macAddressTable = (rtl8651_tblDrv_macAddressEntry_t *) malloc(rtl8651_tblDrvPara.macAddressDbSize * sizeof(rtl8651_tblDrv_macAddressEntry_t));
	rtl8651ArpAsicArrangementTable = (rtl8651_tblDrv_arpAsicArrangementEntry_t *)malloc(RTL8651_ARPARRANGE_NUM * sizeof(rtl8651_tblDrv_arpAsicArrangementEntry_t));
	rtl8651RouteAsicArrangementTable = (rtl8651_tblDrv_routeAsicArrangementEntry_t *)malloc(RTL8651_ROUTINGTBL_SIZE * sizeof( rtl8651_tblDrv_routeAsicArrangementEntry_t));
	rtl8651IpTableAsicArrangementTable = (rtl8651_tblDrv_ipTableAsicArrangementEntry_t *)malloc(RTL8651_IPTABLE_SIZE * sizeof(rtl8651_tblDrv_ipTableAsicArrangementEntry_t));
	rtl8651EthernetPortTable = (rtl8651_tblDrv_ethernet_t *)malloc(RTL8651_PORT_NUMBER * sizeof(rtl8651_tblDrv_ethernet_t));
	rtl8651LinkAggregationTable = (rtl8651_tblDrv_linkAggregation_t *)malloc(RTL8651_PORT_NUMBER * sizeof(rtl8651_tblDrv_linkAggregation_t));
	rtl8651VlanTable = (rtl8651_tblDrv_vlanTable_t *)malloc(RTL8651_VLAN_NUMBER * sizeof(rtl8651_tblDrv_vlanTable_t));
	rtl8651PppoeTable = (rtl8651_tblDrv_pppoeTable_t *)malloc(RTL8651_PPPOE_NUMBER * sizeof(rtl8651_tblDrv_pppoeTable_t));
	rtl8651FilterDbTable = (rtl8651_tblDrv_filterDbTable_t *)malloc(RTL8651_FDB_NUMBER * sizeof(rtl8651_tblDrv_filterDbTable_t));
	rtl8651FilterDbTable = (rtl8651_tblDrv_filterDbTable_t *)malloc(RTL8651_FDB_NUMBER * sizeof(rtl8651_tblDrv_filterDbTable_t));
	rtl8651SpanningTreeTable = (rtl8651_tblDrv_spanningTreeTable_t *)malloc(RTL8651_FDB_NUMBER * sizeof(rtl8651_tblDrv_spanningTreeTable_t));
	{//Initial free filter database entry
		rtl8651_tblDrv_filterDbTableEntry_t * tempFilterDb = (rtl8651_tblDrv_filterDbTableEntry_t *)malloc(sizeof(rtl8651_tblDrv_filterDbTableEntry_t));
		rtl8651FreeFilterDbEntryList = tempFilterDb;
		for(i=1; i<rtl8651_tblDrvPara.filterDbSize; i++) {
			tempFilterDb->next = (rtl8651_tblDrv_filterDbTableEntry_t *)malloc(sizeof(rtl8651_tblDrv_filterDbTableEntry_t));
			tempFilterDb = tempFilterDb->next;
		}
		tempFilterDb->next = NULL;
	}
	rtl8651NetworkIntfTable = (rtl8651_tblDrv_networkIntfTable_t *) malloc (rtl8651_tblDrvPara.networkIntfTableSize * sizeof(rtl8651_tblDrv_networkIntfTable_t));
	{//Initial network interface
		rtl8651_tblDrv_ipIntfEntry_t * tempNetIntf = (rtl8651_tblDrv_ipIntfEntry_t *) malloc (sizeof(rtl8651_tblDrv_ipIntfEntry_t));
		rtl8651FreeIpIntfList = tempNetIntf;
		for(i=1; i<rtl8651_tblDrvPara.ipIntfEntryNumber; i++) {
			tempNetIntf->nextIp = (rtl8651_tblDrv_ipIntfEntry_t *) malloc (sizeof(rtl8651_tblDrv_ipIntfEntry_t));
			tempNetIntf = tempNetIntf->nextIp;
		}
		tempNetIntf->nextIp = NULL;
	}
	{//Initial network interface IP address list
		rtl8651_tblDrv_ipIntfIpAddrEntry_t * tempNetIntfIp = (rtl8651_tblDrv_ipIntfIpAddrEntry_t *) malloc(sizeof(rtl8651_tblDrv_ipIntfIpAddrEntry_t));
		rtl8651FreeIpIntfIpAddrList = tempNetIntfIp;
		for(i=1; i<rtl8651_tblDrvPara.ipIntfIpAddrNumber; i++) {
			tempNetIntfIp->next = (rtl8651_tblDrv_ipIntfIpAddrEntry_t *) malloc(sizeof(rtl8651_tblDrv_ipIntfIpAddrEntry_t));
			tempNetIntfIp = tempNetIntfIp->next;
		}
		tempNetIntfIp->next = NULL;
	}
	{//Initial arp list
		rtl8651_tblDrv_arpEntry_t * tempArp = (rtl8651_tblDrv_arpEntry_t *) malloc(sizeof(rtl8651_tblDrv_arpEntry_t));
		rtl8651FreeArpList = tempArp;
		for(i=1; i<rtl8651_tblDrvPara.arpTableSize; i++) {
			tempArp->next = (rtl8651_tblDrv_arpEntry_t *) malloc(sizeof(rtl8651_tblDrv_arpEntry_t));
			tempArp = tempArp->next;
		}
		tempArp->next = NULL;
	}
	rtl8651RoutingTable = (rtl8651_tblDrv_routeTable_t *) malloc(rtl8651_tblDrvPara.routingTableSize * sizeof(rtl8651_tblDrv_routeTable_t));
	{//Initial nat list
		rtl8651_tblDrv_natEntry_t * tempNatMappintPtr = (rtl8651_tblDrv_natEntry_t *) malloc(sizeof(rtl8651_tblDrv_natEntry_t));
		rtl8651FreeNatList = tempNatMappintPtr;
		for(i=1; i<rtl8651_tblDrvPara.natSize; i++) {
			tempNatMappintPtr->next = (rtl8651_tblDrv_natEntry_t *) malloc(sizeof(rtl8651_tblDrv_natEntry_t));
			tempNatMappintPtr = tempNatMappintPtr->next;
		}
		tempNatMappintPtr->next = NULL;
	}
	rtl8651ServerPortAsicArrangementTable = (rtl8651_tblDrv_serverPortAsicArrangementEntry_t *) malloc(RTL8651_SERVERPORTTBL_SIZE * sizeof(rtl8651_tblDrv_serverPortAsicArrangementEntry_t));
	{//Initial napt server port list
		rtl8651_tblDrv_naptServerPortEntry_t * tempNaptServerPortPtr = (rtl8651_tblDrv_naptServerPortEntry_t *) malloc(sizeof(rtl8651_tblDrv_naptServerPortEntry_t));
		rtl8651FreeNaptServerPortList = tempNaptServerPortPtr;
		for(i=1; i<rtl8651_tblDrvPara.naptServerPortEntryNumber; i++) {
			tempNaptServerPortPtr->next = (rtl8651_tblDrv_naptServerPortEntry_t *) malloc(sizeof(rtl8651_tblDrv_naptServerPortEntry_t));
			tempNaptServerPortPtr = tempNaptServerPortPtr->next;
		}
		tempNaptServerPortPtr->next = NULL;
	}
	rtl8651NaptTcpUdpFlowTable = (rtl8651_tblDrv_naptTcpUdpFlowEntry_t **)malloc(RTL8651_TCPUDPTBL_SIZE * sizeof(rtl8651_tblDrv_naptTcpUdpFlowEntry_t *));
	{//Initial napt tcp/udp flow entry list
		rtl8651_tblDrv_naptTcpUdpFlowEntry_t * tempNaptTcpUdpFlowPtr = (rtl8651_tblDrv_naptTcpUdpFlowEntry_t *) malloc(sizeof(rtl8651_tblDrv_naptTcpUdpFlowEntry_t));
		rtl8651FreeNaptTcpUdpFlowEntryList = tempNaptTcpUdpFlowPtr;
		for(i=1; i<rtl8651_tblDrvPara.naptTcpUdpFlowSize; i++) {
			tempNaptTcpUdpFlowPtr->next = (rtl8651_tblDrv_naptTcpUdpFlowEntry_t *) malloc(sizeof(rtl8651_tblDrv_naptTcpUdpFlowEntry_t));
			tempNaptTcpUdpFlowPtr = tempNaptTcpUdpFlowPtr->next;
		}
		tempNaptTcpUdpFlowPtr->next = (rtl8651_tblDrv_naptTcpUdpFlowEntry_t *) NULL;
	}
	rtl8651NaptTcpUdpBackwardCollisionCountTable = (uint16 *) malloc(RTL8651_TCPUDPTBL_SIZE * sizeof(uint16)); 
	rtl8651NaptTcpUdpFlowNumberTable = (uint16 *) malloc(RTL8651_TCPUDPTBL_SIZE * sizeof(uint16)); 
	rtl8651NaptTcpUdpFlowNonAsicNumberTable = (uint16 *) malloc(RTL8651_TCPUDPTBL_SIZE * sizeof(uint16)); 
	rtl8651NaptIcmpFlowTable = (rtl8651_tblDrv_naptIcmpFlowEntry_t **)malloc(RTL8651_ICMPTBL_SIZE * sizeof(rtl8651_tblDrv_naptIcmpFlowEntry_t *));
	{//Initial napt icmp flow entry list
		rtl8651_tblDrv_naptIcmpFlowEntry_t * tempNaptIcmpFlowPtr = (rtl8651_tblDrv_naptIcmpFlowEntry_t *) malloc(sizeof(rtl8651_tblDrv_naptIcmpFlowEntry_t));
		rtl8651FreeNaptIcmpFlowEntryList = tempNaptIcmpFlowPtr;
		for(i=1; i<rtl8651_tblDrvPara.naptIcmpFlowSize; i++) {
			tempNaptIcmpFlowPtr->next = (rtl8651_tblDrv_naptIcmpFlowEntry_t *) malloc(sizeof(rtl8651_tblDrv_naptIcmpFlowEntry_t));
			tempNaptIcmpFlowPtr = tempNaptIcmpFlowPtr->next;
		}
		tempNaptIcmpFlowPtr->next = (rtl8651_tblDrv_naptIcmpFlowEntry_t *)NULL;
	}
	rtl8651NaptIcmpBackwardCollisionCountTable = (uint16 * ) malloc(RTL8651_ICMPTBL_SIZE * sizeof(uint16)); //Whether Flow Table flows are kept in software due to backward ID collision 
	rtl8651NaptIcmpFlowNumberTable = (uint16 * ) malloc(RTL8651_ICMPTBL_SIZE * sizeof(uint16));
	rtl8651NaptIcmpFlowNonAsicNumberTable = (uint16 * ) malloc(RTL8651_ICMPTBL_SIZE * sizeof(uint16));
	rtl8651AlgAsicArrangementTable = (rtl8651_tblDrv_algAsicArrangementEntry_t *) malloc(RTL8651_ALGTBL_SIZE * sizeof(rtl8651_tblDrv_algAsicArrangementEntry_t));
	rtl8651AclAsicArrangementTable = (rtl8651_tblDrv_aclAsicArrangementEntry_t *) malloc(RTL8651_ACLTBL_SIZE * sizeof(rtl8651_tblDrv_aclAsicArrangementEntry_t));
	{
		rtl8651_tblDrv_aclMacEntry_t * tempMacAclPtr = (rtl8651_tblDrv_aclMacEntry_t *) malloc(sizeof(rtl8651_tblDrv_aclMacEntry_t));
		rtl8651FreeMacAclList = tempMacAclPtr;
		for(i=1; i<rtl8651_tblDrvPara.macAclSize; i++) {
			tempMacAclPtr->next = (rtl8651_tblDrv_aclMacEntry_t *) malloc(sizeof(rtl8651_tblDrv_aclMacEntry_t));
			tempMacAclPtr = tempMacAclPtr->next;
		}
		tempMacAclPtr->next = (rtl8651_tblDrv_aclMacEntry_t *)NULL;
	}
	{
		rtl8651_tblDrv_aclIpEntry_t * tempIpAclPtr = (rtl8651_tblDrv_aclIpEntry_t *) malloc(sizeof(rtl8651_tblDrv_aclIpEntry_t));
		rtl8651FreeIpAclList = tempIpAclPtr;
		for(i=1; i<rtl8651_tblDrvPara.ipAclSize; i++) {
			tempIpAclPtr->next = (rtl8651_tblDrv_aclIpEntry_t *) malloc(sizeof(rtl8651_tblDrv_aclIpEntry_t));
			tempIpAclPtr = tempIpAclPtr->next;
		}
		tempIpAclPtr->next = (rtl8651_tblDrv_aclIpEntry_t *)NULL;
	}
	{
		rtl8651_tblDrv_algSlot_t * tempAlgSlotPtr = (rtl8651_tblDrv_algSlot_t *) malloc(sizeof(rtl8651_tblDrv_algSlot_t));
		rtl8651FreeAlgSlotList = tempAlgSlotPtr;
		for(i=1; i<RTL8651_ALGTBL_SIZE; i++) {
			tempAlgSlotPtr->next = (rtl8651_tblDrv_algSlot_t *) malloc(sizeof(rtl8651_tblDrv_algSlot_t));
			tempAlgSlotPtr = tempAlgSlotPtr->next;
		}
		tempAlgSlotPtr->next = (rtl8651_tblDrv_algSlot_t *)NULL;
	}
	{
		rtl8651_tblDrv_algEntry_t * tempAlgPtr = (rtl8651_tblDrv_algEntry_t *) malloc(sizeof(rtl8651_tblDrv_algEntry_t));
		rtl8651FreeAlgList = tempAlgPtr;
		for(i=1; i<rtl8651_tblDrvPara.algSize; i++) {
			tempAlgPtr->next = (rtl8651_tblDrv_algEntry_t *) malloc(sizeof(rtl8651_tblDrv_algEntry_t));
			tempAlgPtr = tempAlgPtr->next;
		}
		tempAlgPtr->next = (rtl8651_tblDrv_algEntry_t *)NULL;
	}
	/* Initialize Reserved Port list */
	{
		rtl8651_tblDrv_rsvPortNumber_t *tmpRsvPort = (rtl8651_tblDrv_rsvPortNumber_t *)malloc(sizeof(rtl8651_tblDrv_rsvPortNumber_t));
		rtl8651FreeRsvPortList = tmpRsvPort;
		for(i=1; i<rtl8651_tblDrvPara.reservedPortSize; i++) {
			tmpRsvPort->next = (rtl8651_tblDrv_rsvPortNumber_t *)malloc(sizeof(rtl8651_tblDrv_rsvPortNumber_t));
			tmpRsvPort = tmpRsvPort->next;
		}
		tmpRsvPort->next = (rtl8651_tblDrv_rsvPortNumber_t *)NULL;
	}
	return SUCCESS;
}

//Assume all data structure are initialized properly and rtl8651_tblDrvPara is ready to serve
static int32 _rtl8651_tblDrvListCollect(void) {
	uint32 i, j;
	
	{
		rtl8651_tblDrv_filterDbTableEntry_t * tempFilterDbPtr;
		for(i=0; i<RTL8651_FDB_NUMBER; i++)
			for(j=0; j<RTL8651_L2TBL_ROW; j++)
				while(rtl8651FilterDbTable[i].database[j]) {
					tempFilterDbPtr = rtl8651FilterDbTable[i].database[j];
					rtl8651FilterDbTable[i].database[j] = tempFilterDbPtr->next;
					tempFilterDbPtr->next = rtl8651FreeFilterDbEntryList;
					rtl8651FreeFilterDbEntryList = tempFilterDbPtr;
				}
		i=0;
		tempFilterDbPtr = rtl8651FreeFilterDbEntryList;
		while(tempFilterDbPtr) {
			i++;
			tempFilterDbPtr = tempFilterDbPtr->next;
		}
		assert(i == rtl8651_tblDrvPara.filterDbSize);
	}
	{// Collect rtl8651FreeMacAclList and rtl8651FreeIpAclList
		rtl8651_tblDrv_aclMacEntry_t * tempMacAclPtr;
		rtl8651_tblDrv_aclIpEntry_t * tempIpAclPtr;
		for(i=0; i<RTL8651_VLAN_NUMBER; i++)
			if(rtl8651VlanTable[i].valid == 1) {
				while(rtl8651VlanTable[i].inAclHead) {
					tempMacAclPtr = rtl8651VlanTable[i].inAclHead;
					rtl8651VlanTable[i].inAclHead = tempMacAclPtr->next;
					tempMacAclPtr->next = rtl8651FreeMacAclList;
					rtl8651FreeMacAclList = tempMacAclPtr;
					if(rtl8651VlanTable[i].inAclHead == (rtl8651_tblDrv_aclMacEntry_t *)NULL)
						rtl8651VlanTable[i].inAclTail = (rtl8651_tblDrv_aclMacEntry_t *)NULL;
				}
				assert(rtl8651VlanTable[i].inAclHead == (rtl8651_tblDrv_aclMacEntry_t *)NULL);
				assert(rtl8651VlanTable[i].inAclTail == (rtl8651_tblDrv_aclMacEntry_t *)NULL);
				while(rtl8651VlanTable[i].outAclHead) {
					tempMacAclPtr = rtl8651VlanTable[i].outAclHead;
					rtl8651VlanTable[i].outAclHead = tempMacAclPtr->next;
					tempMacAclPtr->next = rtl8651FreeMacAclList;
					rtl8651FreeMacAclList = tempMacAclPtr;
					if(rtl8651VlanTable[i].outAclHead == (rtl8651_tblDrv_aclMacEntry_t *)NULL)
						rtl8651VlanTable[i].outAclTail = (rtl8651_tblDrv_aclMacEntry_t *)NULL;
				}
				assert(rtl8651VlanTable[i].outAclHead == (rtl8651_tblDrv_aclMacEntry_t *)NULL);
				assert(rtl8651VlanTable[i].outAclTail == (rtl8651_tblDrv_aclMacEntry_t *)NULL);
				while(rtl8651VlanTable[i].inIpAclHead) {
					tempIpAclPtr = rtl8651VlanTable[i].inIpAclHead;
					rtl8651VlanTable[i].inIpAclHead = tempIpAclPtr->next;
					tempIpAclPtr->next = rtl8651FreeIpAclList;
					rtl8651FreeIpAclList = tempIpAclPtr;
					if(rtl8651VlanTable[i].inIpAclHead == (rtl8651_tblDrv_aclIpEntry_t *)NULL)
						rtl8651VlanTable[i].inIpAclTail =(rtl8651_tblDrv_aclIpEntry_t *)NULL;
				}
				assert(rtl8651VlanTable[i].inIpAclHead == (rtl8651_tblDrv_aclIpEntry_t *)NULL);
				assert(rtl8651VlanTable[i].inIpAclTail == (rtl8651_tblDrv_aclIpEntry_t *)NULL);
				while(rtl8651VlanTable[i].outIpAclHead) {
					tempIpAclPtr = rtl8651VlanTable[i].outIpAclHead;
					rtl8651VlanTable[i].outIpAclHead = tempIpAclPtr->next;
					tempIpAclPtr->next = rtl8651FreeIpAclList;
					rtl8651FreeIpAclList = tempIpAclPtr;
					if(rtl8651VlanTable[i].outIpAclHead == (rtl8651_tblDrv_aclIpEntry_t *)NULL)
						rtl8651VlanTable[i].outIpAclTail =(rtl8651_tblDrv_aclIpEntry_t *)NULL;
				}
				assert(rtl8651VlanTable[i].outIpAclHead == (rtl8651_tblDrv_aclIpEntry_t *)NULL);
				assert(rtl8651VlanTable[i].outIpAclTail == (rtl8651_tblDrv_aclIpEntry_t *)NULL);
			}
		i=0;
		tempMacAclPtr = rtl8651FreeMacAclList;
		while(tempMacAclPtr) {
			i++;
			tempMacAclPtr = tempMacAclPtr->next;
		}
		assert(i == rtl8651_tblDrvPara.macAclSize);
	}
	{//Collect rtl8651FreeIpIntfIpAddrList, rtl8651FreeArpList and then rtl8651FreeIpIntfList, rtl8651FreeIpAclList
		rtl8651_tblDrv_ipIntfEntry_t * ipIntfPtr;
		rtl8651_tblDrv_ipIntfIpAddrEntry_t * ipIntfIpPtr;
		rtl8651_tblDrv_arpEntry_t * tempArpPtr;
		rtl8651_tblDrv_aclIpEntry_t * tempIpAclPtr;
		
		for(i=0; i< rtl8651_tblDrvPara.networkIntfTableSize; i++) {
			if(rtl8651NetworkIntfTable[i].valid == 1) {//Interface valid, network interface is meaningful
				while(rtl8651NetworkIntfTable[i].IpHead) {
					ipIntfPtr = rtl8651NetworkIntfTable[i].IpHead;
					rtl8651NetworkIntfTable[i].IpHead = ipIntfPtr->nextIp;
					ipIntfPtr->nextIp = rtl8651FreeIpIntfList;
					rtl8651FreeIpIntfList = ipIntfPtr;
					while(ipIntfPtr->ipAddr) {//Collect local ip address
						ipIntfIpPtr = ipIntfPtr->ipAddr;
						ipIntfPtr->ipAddr = ipIntfIpPtr->next;
						ipIntfIpPtr->next = rtl8651FreeIpIntfIpAddrList;
						rtl8651FreeIpIntfIpAddrList = ipIntfIpPtr;
					}
					while(ipIntfPtr->localServer) {//Collect local server ip address
						ipIntfIpPtr = ipIntfPtr->localServer;
						ipIntfPtr->localServer = ipIntfIpPtr->next;
						ipIntfIpPtr->next = rtl8651FreeIpIntfIpAddrList;
						rtl8651FreeIpIntfIpAddrList = ipIntfIpPtr;
					}
					while(ipIntfPtr->nextArp) {//Collect arp entries
						tempArpPtr = ipIntfPtr->nextArp;
						ipIntfPtr->nextArp = tempArpPtr->next;
						tempArpPtr->next = rtl8651FreeArpList;
						rtl8651FreeArpList = tempArpPtr;
					}
				}
				while(rtl8651NetworkIntfTable[i].inAclHead) {
					tempIpAclPtr = rtl8651NetworkIntfTable[i].inAclHead;
					rtl8651NetworkIntfTable[i].inAclHead = tempIpAclPtr->next;
					tempIpAclPtr->next = rtl8651FreeIpAclList;
					rtl8651FreeIpAclList = tempIpAclPtr;
					if(rtl8651NetworkIntfTable[i].inAclHead == (rtl8651_tblDrv_aclIpEntry_t *)NULL)
						rtl8651NetworkIntfTable[i].inAclTail = (rtl8651_tblDrv_aclIpEntry_t *)NULL;
				}
				while(rtl8651NetworkIntfTable[i].outAclHead) {
					tempIpAclPtr = rtl8651NetworkIntfTable[i].outAclHead;
					rtl8651NetworkIntfTable[i].outAclHead = tempIpAclPtr->next;
					tempIpAclPtr->next = rtl8651FreeIpAclList;
					rtl8651FreeIpAclList = tempIpAclPtr;
					if(rtl8651NetworkIntfTable[i].outAclHead == (rtl8651_tblDrv_aclIpEntry_t *)NULL)
						rtl8651NetworkIntfTable[i].outAclTail = (rtl8651_tblDrv_aclIpEntry_t *)NULL;
				}
			}
		}
		//Network interface collect complete
		i=0;
		ipIntfPtr = rtl8651FreeIpIntfList;
		while(ipIntfPtr) {
			i++;
			ipIntfPtr = ipIntfPtr->nextIp;
		}
		assert(i==rtl8651_tblDrvPara.ipIntfEntryNumber);
		//IP address collect complete
		i=0;
		ipIntfIpPtr = rtl8651FreeIpIntfIpAddrList;
		while(ipIntfIpPtr) {
			i++;
			ipIntfIpPtr = ipIntfIpPtr->next;
		}
		assert(i==rtl8651_tblDrvPara.ipIntfIpAddrNumber);
		i=0;
		tempIpAclPtr = rtl8651FreeIpAclList;
		while(tempIpAclPtr) {
			i++;
			tempIpAclPtr = tempIpAclPtr->next;
		}
		assert(i==rtl8651_tblDrvPara.ipAclSize);
	}
	{//Move arp entries from arp non-interface arp list to free arp list
		rtl8651_tblDrv_arpEntry_t * tempArpPtr;
		while(rtl8651NonInterfaceArpList) {
			tempArpPtr = rtl8651NonInterfaceArpList;
			rtl8651NonInterfaceArpList = tempArpPtr->next;
			tempArpPtr->next = rtl8651FreeArpList;
			rtl8651FreeArpList = tempArpPtr;
		}
		//ARP entries collect complete
		i=0;
		tempArpPtr = rtl8651FreeArpList;
		while(tempArpPtr) {
			i++;
			tempArpPtr = tempArpPtr->next;
		}
		assert(i==rtl8651_tblDrvPara.arpTableSize);
	}
	{//Collect data structures attached on NAT control block
		rtl8651_tblDrv_natEntry_t * tempNatPtr;
		rtl8651_tblDrv_naptServerPortEntry_t * tempNaptServerPortPtr;

		for(i=0;i<rtl8651_tblDrvPara.networkIntfTableSize;i++) 
			while(rtl8651NetworkIntfTable[i].valid == 1 && (rtl8651NetworkIntfTable[i].nextNat || rtl8651NetworkIntfTable[i].nextNaptServerPort)) {
				if(rtl8651NetworkIntfTable[i].nextNat) {
					tempNatPtr = rtl8651NetworkIntfTable[i].nextNat;
					rtl8651NetworkIntfTable[i].nextNat = tempNatPtr->next;
					tempNatPtr->next = rtl8651FreeNatList;
					rtl8651FreeNatList = tempNatPtr;
				}
				if(rtl8651NetworkIntfTable[i].nextNaptServerPort) {
					tempNaptServerPortPtr = rtl8651NetworkIntfTable[i].nextNaptServerPort;
					rtl8651NetworkIntfTable[i].nextNaptServerPort = tempNaptServerPortPtr->next;
					tempNaptServerPortPtr->next = rtl8651FreeNaptServerPortList;
					rtl8651FreeNaptServerPortList = tempNaptServerPortPtr;
				}
			}
		i=0;
		tempNatPtr = rtl8651FreeNatList;
		while(tempNatPtr) {
			i++;
			tempNatPtr = tempNatPtr->next;
		}
		assert(i==rtl8651_tblDrvPara.natSize);
		i=0;
		tempNaptServerPortPtr = rtl8651FreeNaptServerPortList;
		while(tempNaptServerPortPtr) {
			i++;
			tempNaptServerPortPtr = tempNaptServerPortPtr->next;
		}
		assert(i==rtl8651_tblDrvPara.naptServerPortEntryNumber);
	}
	{
		rtl8651_tblDrv_naptTcpUdpFlowEntry_t * tempNaptTcpUdpFlowPtr;
		rtl8651_tblDrv_naptIcmpFlowEntry_t * tempNaptIcmpFlowPtr;
		
		rtl8651_flushNaptFlow();
		i=0;
		tempNaptTcpUdpFlowPtr = rtl8651FreeNaptTcpUdpFlowEntryList;
		while(tempNaptTcpUdpFlowPtr) {
			i++;
			tempNaptTcpUdpFlowPtr = tempNaptTcpUdpFlowPtr->next;
		}
		assert(i==rtl8651_tblDrvPara.naptTcpUdpFlowSize);
		i=0;
		tempNaptIcmpFlowPtr = rtl8651FreeNaptIcmpFlowEntryList;
		while(tempNaptIcmpFlowPtr) {
			i++;
			tempNaptIcmpFlowPtr = tempNaptIcmpFlowPtr->next;
		}
		assert(i==rtl8651_tblDrvPara.naptIcmpFlowSize);
	}
	{//Collect rtl8651FreeAlgSlotList and rtl8651FreeAlgSlotList
		rtl8651_tblDrv_algSlot_t * tempAlgSlotPtr;
		rtl8651_tblDrv_algEntry_t * tempAlgPtr;

		while(rtl8651AlgList) {
			tempAlgPtr = rtl8651AlgList;
			rtl8651AlgList = tempAlgPtr->next;
			tempAlgPtr->next = rtl8651FreeAlgList;
			rtl8651FreeAlgList = tempAlgPtr;
			while(tempAlgPtr->nextAlgSlot) {
				tempAlgSlotPtr = tempAlgPtr->nextAlgSlot;
				tempAlgPtr->nextAlgSlot = tempAlgSlotPtr->next;
				tempAlgSlotPtr->next = rtl8651FreeAlgSlotList;
				rtl8651FreeAlgSlotList = tempAlgSlotPtr;
			}
		}
		i=0;
		tempAlgPtr = rtl8651FreeAlgList;
		while(tempAlgPtr) {
			i++;
			tempAlgPtr = tempAlgPtr->next;
		}
		assert(i==rtl8651_tblDrvPara.algSize);
		i=0;
		tempAlgSlotPtr = rtl8651FreeAlgSlotList;
		while(tempAlgSlotPtr) {
			i++;
			tempAlgSlotPtr = tempAlgSlotPtr->next;
		}
		assert(i == RTL8651_ALGTBL_SIZE);
	}
	/* Collect Resvrved port */
	{
		rtl8651_tblDrv_rsvPortNumber_t *tmpRsvPort;

		while (rtl8651RsvPortList) {
			tmpRsvPort = rtl8651RsvPortList;
			rtl8651RsvPortList = rtl8651RsvPortList->next;
			tmpRsvPort->next = rtl8651FreeRsvPortList;
			rtl8651FreeRsvPortList = tmpRsvPort;
		}
		tmpRsvPort = rtl8651FreeRsvPortList; i=0;
		while (tmpRsvPort) {
			i++;
			tmpRsvPort = tmpRsvPort->next;
		}
		assert(i == rtl8651_tblDrvPara.reservedPortSize);
	}
	return SUCCESS;
}

static int32 _rtl8651_tblDrvArrayInit(void) {
	uint32 i;
	//Hardware dependent data structure initialization
	for(i=0; i< RTL8651_ARPARRANGE_NUM; i++)//Arp arrangement table
		rtl8651ArpAsicArrangementTable[i].valid = 0;
	for(i=0; i<RTL8651_ROUTINGTBL_SIZE; i++)//Routing arrangment table
		rtl8651RouteAsicArrangementTable[i].valid = 0;
	for(i=0; i<RTL8651_IPTABLE_SIZE; i++)//IP table initialization
		rtl8651IpTableAsicArrangementTable[i].valid = 0;
	for(i=0; i<rtl8651_tblDrvPara.macAddressDbSize; i++)//MAC address table initialization
		macAddressTable[i].valid = 0;
	{//Get hardware stored (flash or ROM) MAC address
		ether_addr_t macAddress;
		uint32 number;

		rtl8651_getMacAddress(&macAddress, &number);
		assert((macAddress.octet[5] & 0x7) == 0);
		for(i=0; i< number && i < rtl8651_tblDrvPara.macAddressDbSize; i++) {
			macAddressTable[i].mac = macAddress;
			macAddressTable[i].mac.octet[4] += (uint8) (i>>8);
			macAddressTable[i].mac.octet[5] += (uint8) (i&0xff);
			macAddressTable[i].allocated = 0;
			macAddressTable[i].valid = 1;
		}
	}
	for(i=0; i<RTL8651_PORT_NUMBER; i++) {//Configure Ethernet port default values
		//Initial Ethernet ports
		rtl8651EthernetPortTable[i].speed = 1;	//100Mbps default
		rtl8651EthernetPortTable[i].duplex = 1; //full-duplex mode default
		rtl8651EthernetPortTable[i].autoNegotiation = 1; //Default auto-negotiation enabled
		rtl8651EthernetPortTable[i].aggregatorIndex = i;
		rtl8651EthernetPortTable[i].linkUp = 0;			
		//Initial Link Aggregator
		rtl8651LinkAggregationTable[i].pvid = 1; //Default VLAN 1 IEEE 802.1q p.69
		rtl8651LinkAggregationTable[i].individual = 0; //Initial to be not-individual, but the total number of disabling individual property is limited by RTL8651_MAX_AGGREGATOR_NUM
		rtl8651LinkAggregationTable[i].aggregated = 0; //Initial not aggregated
		rtl8651LinkAggregationTable[i].ethernetPortMask = 1<<i;//Initially, the port and link aggragator are 1-to-1 mapping
		rtl8651LinkAggregationTable[i].ethernetPortUpStatus = 0;//Initially, the port state of the link aggregator is down
	}
	//Invalidate all spanning tree table
	for(i=0; i< RTL8651_FDB_NUMBER; i++)
		rtl8651SpanningTreeTable[i].valid = 0;
	//Initial spanning tree 0
	//Invalidate all filter database
	for(i=0; i< RTL8651_FDB_NUMBER; i++)
		rtl8651FilterDbTable[i].valid = 0;
	//Initial vlan table
	for(i=0; i<RTL8651_VLAN_NUMBER; i++)
		rtl8651VlanTable[i].valid = 0;
	//Initial PPPoE table
	for(i=0; i<RTL8651_PPPOE_NUMBER; i++)
		rtl8651PppoeTable[i].valid = 0;
	//Initial IP interface
	for(i=0; i<rtl8651_tblDrvPara.networkIntfTableSize; i++)
		rtl8651NetworkIntfTable[i].valid = 0;
	rtl8651NonInterfaceArpList = NULL;
	//Initial routing table
	for(i=0; i<rtl8651_tblDrvPara.routingTableSize; i++)
		rtl8651RoutingTable[i].valid = 0;
	pendingRoutes = 0;
	naptIpCount = localServerCount = natIpCount = 0;
	for(i=0; i<RTL8651_SERVERPORTTBL_SIZE; i++)
		rtl8651ServerPortAsicArrangementTable[i].valid = 0;
	/* Initial Reserved port list */
	rtl8651RsvPortList = NULL;
	//Initial napt tcp/udp flow table
	for(i=0; i<RTL8651_TCPUDPTBL_SIZE; i++) {
		rtl8651NaptTcpUdpFlowTable[i] = (rtl8651_tblDrv_naptTcpUdpFlowEntry_t *) NULL;
		rtl8651NaptTcpUdpBackwardCollisionCountTable[i] = 0;
		rtl8651NaptTcpUdpFlowNumberTable[i] = 0;
		rtl8651NaptTcpUdpFlowNonAsicNumberTable[i] = 0;
	}
	//Initial napt icmp flow table
	for(i=0; i<RTL8651_ICMPTBL_SIZE; i++) {
		rtl8651NaptIcmpFlowTable[i] = (rtl8651_tblDrv_naptIcmpFlowEntry_t *)NULL;
		rtl8651NaptIcmpBackwardCollisionCountTable[i] = 0;
		rtl8651NaptIcmpFlowNumberTable[i] = 0;
		rtl8651NaptIcmpFlowNonAsicNumberTable[i] = 0;
	}
	//ASIC Initialization
	for(i=0; i<RTL8651_PORT_NUMBER; i++) {
		rtl8651_setAsicMulticastPortInternal(i, TRUE);
		rtl8651_setAsicMulticastSpanningTreePortState(i, RTL8651_PORTSTA_DISABLED);
		rtl8651_setAsicEthernetBandwidthControl(i, TRUE, RTL8651_BC_FULL);
		rtl8651_setAsicEthernetBandwidthControl(i, FALSE, RTL8651_BC_FULL);
	}
	//Initial napt timeout value
	rtl8651_setNaptIcmpTimeout(60);
	rtl8651_setNaptUdpTimeout(300);
	rtl8651_setNaptTcpLongTimeout(24 * 60 * 60);
	rtl8651_setNaptTcpMediumTimeout(600);
	rtl8651_setNaptTcpFastTimeout(60);
	rtl8651ExistedNaptTcpUdpFlows = 0;
	rtl8651ExistedNaptIcmpFlows = 0;
	for(i=0; i<RTL8651_ALGTBL_SIZE; i++)
		rtl8651AlgAsicArrangementTable[i].valid = 0;
	for(i=0; i<RTL8651_ACLTBL_SIZE; i++)
		rtl8651AclAsicArrangementTable[i].valid = 0;
	rtl8651AlgList = (rtl8651_tblDrv_algEntry_t *)NULL;
	_rtl8651_updateAsicAclRulePermitAll(RTL8651_ACLTBL_PERMIT_ALL);
	_rtl8651_updateAsicAclRuleToCpu(RTL8651_ACLTBL_ALL_TO_CPU);
	//Initialize function pointers
	rtl8651_linkStatusNotifier = NULL;
	rtl8651_resourceLock = NULL;
	rtl8651_resourceUnlock = NULL;
	//Install funciton to asic driver
	rtl8651_installAsicEtherrnetLinkStatusNotifier(_rtl8651_setEthernetPortLinkStatus);
	return SUCCESS;
}

int32 rtl8651_tblDrvReinit(void) {
	uint32 i;
	
	//Initial ASIC table
	rtl8651_initAsic();
	//Collect all all link list entries
	_rtl8651_tblDrvListCollect();
	//Initial array structure
	_rtl8651_tblDrvArrayInit();
	
	//Common initialization for testing
	rtl8651_addSpanningTreeInstance(0);
	rtl8651_addFilterDatabase(0);
	rtl8651_addVlan(1);
	for(i=0; i<RTL8651_PORT_NUMBER; i++) {
		rtl8651_addVlanPortMember(1, i);
		rtl8651_setPvid(i, 1);
	}
	return SUCCESS;
}

int32 rtl8651_tblDrvInit(rtl8651_tblDrvInitPara_t * configPara) {
	if(configPara == NULL) {
		rtl8651_tblDrvPara.macAddressDbSize = 8;
		rtl8651_tblDrvPara.filterDbSize = 1024;
		rtl8651_tblDrvPara.networkIntfTableSize = 8;
		rtl8651_tblDrvPara.ipIntfEntryNumber = 16;
		rtl8651_tblDrvPara.ipIntfIpAddrNumber = 24;
		rtl8651_tblDrvPara.arpTableSize = 512;
		rtl8651_tblDrvPara.routingTableSize = 64;
		rtl8651_tblDrvPara.natSize = 8;
		rtl8651_tblDrvPara.natCtlSize = 32;
		rtl8651_tblDrvPara.naptServerPortEntryNumber = 8;
		rtl8651_tblDrvPara.naptTcpUdpFlowSize = 4096;
		rtl8651_tblDrvPara.naptIcmpFlowSize = 128;
		rtl8651_tblDrvPara.macAclSize = 64;
		rtl8651_tblDrvPara.ipAclSize = 128;
		rtl8651_tblDrvPara.algSize = 128;
		rtl8651_tblDrvPara.aclPerVlanInitInputEntrySize =20;
		rtl8651_tblDrvPara.aclPerVlanInitOutputEntrySize = 10;
		rtl8651_tblDrvPara.reservedPortSize = 20;
	}
	else {
		rtl8651_tblDrvPara = *configPara;
		if(rtl8651_tblDrvPara.macAddressDbSize == 0)
			rtl8651_tblDrvPara.macAddressDbSize = 8;
		if(rtl8651_tblDrvPara.filterDbSize == 0)
			rtl8651_tblDrvPara.filterDbSize = 1024;
		if(rtl8651_tblDrvPara.networkIntfTableSize == 0)
			rtl8651_tblDrvPara.networkIntfTableSize = 8;
		if(rtl8651_tblDrvPara.ipIntfEntryNumber == 0)
			rtl8651_tblDrvPara.ipIntfEntryNumber = 16;
		if(rtl8651_tblDrvPara.ipIntfIpAddrNumber == 0)
			rtl8651_tblDrvPara.ipIntfIpAddrNumber = 24;
		if(rtl8651_tblDrvPara.arpTableSize == 0)
			rtl8651_tblDrvPara.arpTableSize = 512;
		if(rtl8651_tblDrvPara.routingTableSize == 0)
			rtl8651_tblDrvPara.routingTableSize = 64;
		if(rtl8651_tblDrvPara.natSize == 0)
			rtl8651_tblDrvPara.natSize = 8;
		if(rtl8651_tblDrvPara.natCtlSize == 0)
			rtl8651_tblDrvPara.natCtlSize = 32;
		if(rtl8651_tblDrvPara.naptServerPortEntryNumber == 0)
			rtl8651_tblDrvPara.naptServerPortEntryNumber = 8;
		if(rtl8651_tblDrvPara.naptTcpUdpFlowSize == 0)
			rtl8651_tblDrvPara.naptTcpUdpFlowSize = 4096;
		if(rtl8651_tblDrvPara.naptIcmpFlowSize == 0)
			rtl8651_tblDrvPara.naptIcmpFlowSize = 128;
		if(rtl8651_tblDrvPara.macAclSize == 0)
			rtl8651_tblDrvPara.macAclSize = 64;
		if(rtl8651_tblDrvPara.ipAclSize == 0)
			rtl8651_tblDrvPara.ipAclSize = 128;
		if(rtl8651_tblDrvPara.algSize == 0)
			rtl8651_tblDrvPara.algSize = 128;
		if(rtl8651_tblDrvPara.aclPerVlanInitInputEntrySize == 0)
			rtl8651_tblDrvPara.aclPerVlanInitInputEntrySize =20;
		if(rtl8651_tblDrvPara.aclPerVlanInitOutputEntrySize == 0)
			rtl8651_tblDrvPara.aclPerVlanInitOutputEntrySize = 10;
		if (rtl8651_tblDrvPara.reservedPortSize == 0)
			rtl8651_tblDrvPara.reservedPortSize = 20;
	}

	//Unacceptable parameter setting checking
	if(rtl8651_tblDrvPara.macAddressDbSize==0)
		return FAILED;
	if(rtl8651_tblDrvPara.filterDbSize < RTL8651_L2TBL_ROW * RTL8651_L2TBL_COLUMN)
		return FAILED;
	if(rtl8651_tblDrvPara.naptTcpUdpFlowSize < RTL8651_TCPUDPTBL_SIZE)
		return FAILED;
	if(rtl8651_tblDrvPara.naptIcmpFlowSize < RTL8651_ICMPTBL_SIZE)
		return FAILED;
	assert(RTL8651_FDB_NUMBER >= 1);	//Must have at least one filter database

	//Initial ASIC table
	printf("rtl8651_initAsic\n");
	rtl8651_initAsic();
	//Initial driver memory space and link all link list entries
	printf("_rtl8651_tblDrvMemAlloc\n");
	_rtl8651_tblDrvMemAlloc();
	//Initial array structure
	_rtl8651_tblDrvArrayInit();
	printf("_rtl8651_tblDrvArrayInit\n");
	return SUCCESS;
}










//#ifdef CYGWIN

/* Only for testing code */
int32 test_updateNaptTcpUdpFlowAgingTime(int8 isTcp, ipaddr_t sip, uint16 sport, ipaddr_t dip, uint16 dport, uint32 ageTime) {
	uint32 flowTblIdx;
	rtl8651_tblDrv_naptTcpUdpFlowEntry_t * trackNaptTcpUdpFlowPtr;
	
	flowTblIdx = rtl8651_naptTcpUdpTableIndex(isTcp, sip, sport, dip, dport);
	trackNaptTcpUdpFlowPtr = rtl8651NaptTcpUdpFlowTable[flowTblIdx];
	while(trackNaptTcpUdpFlowPtr) {
		if (trackNaptTcpUdpFlowPtr->insideLocalIpAddr == sip &&
			trackNaptTcpUdpFlowPtr->insideLocalPort == sport &&
			trackNaptTcpUdpFlowPtr->dstIpAddr == dip &&
			trackNaptTcpUdpFlowPtr->dstPort == dport) {
			trackNaptTcpUdpFlowPtr->age = ageTime;
			return SUCCESS;
		}
		trackNaptTcpUdpFlowPtr = trackNaptTcpUdpFlowPtr->next;
	}
	return FAILED;
}


int32 test_updateNaptIcmpFlowAgingTime(ipaddr_t sip, uint16 sid, ipaddr_t dip, uint32 ageTime) {
	uint32 tblIdx;
	rtl8651_tblDrv_naptIcmpFlowEntry_t * trackNaptIcmpFlowPtr;
	
	rtl8651_naptIcmpTableIndex(sip, sid, dip, &tblIdx);
	trackNaptIcmpFlowPtr = rtl8651NaptIcmpFlowTable[tblIdx];
	while(trackNaptIcmpFlowPtr) {
		if (trackNaptIcmpFlowPtr->insideLocalIpAddr == sip &&
			trackNaptIcmpFlowPtr->insideLocalId == sid &&
			trackNaptIcmpFlowPtr->dstIpAddr == dip) {
			trackNaptIcmpFlowPtr->age = ageTime;
			return SUCCESS;
		}
		trackNaptIcmpFlowPtr = trackNaptIcmpFlowPtr->next;
	}
	return FAILED;
}


void Dump_tblDrv_NaptTcpUdp(uint32 flowTblIdx) {
	rtl8651_tblDrv_naptTcpUdpFlowEntry_t * trackNaptFlowPtr;

	trackNaptFlowPtr = rtl8651NaptTcpUdpFlowTable[flowTblIdx];
		while(trackNaptFlowPtr) {
			printf("[%d] sip(%x) sport(%d) dip(%x) dport(%d) fromAsic(%d)\n", flowTblIdx, trackNaptFlowPtr->insideLocalIpAddr, trackNaptFlowPtr->insideLocalPort, trackNaptFlowPtr->dstIpAddr, trackNaptFlowPtr->dstPort, trackNaptFlowPtr->fromAsic);
			trackNaptFlowPtr = trackNaptFlowPtr->next;
		}

}


//#endif /* CYGWIN */

