/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Switch core table driver local header rtl8651_tblDrvLocal.h
* Abstract : 
* Creator : Edward Jin-Ru Chen (jzchen@realtek.com.tw)
* Author :  
* $Id: rtl8651_tblDrvLocal.h,v 1.1.1.1 2003/08/18 05:43:22 kaohj Exp $
* $Log: rtl8651_tblDrvLocal.h,v $
* Revision 1.1.1.1  2003/08/18 05:43:22  kaohj
* initial import into CVS
*
* Revision 1.1  2003/05/19 09:22:00  elvis
* Initial version to BSP
*
* Revision 1.2  2003/04/28 15:21:46  elvis
* Synchronized with v1.39
*
* Revision 1.1  2003/04/24 10:12:23  elvis
* Initial version: synchronized with v1.27
*
* Revision 1.27  2003/04/11 10:35:34  chhuang
* add query APIs:
* _rtl8651_getRoutingEntry(), _rtl8651_getAlgEntry(), _rtl8651_getNaptTcpUdpEntry(),
* _rtl8651_getNaptIcmpEntry(), _rtl8651_getMacAclEntry(), _rtl8651_getIpVlanAclEntry()
* _rtl8651_getIpAclEntry(), _rtl8651_reserveNaptTcpUdpExtPort()
*
* Revision 1.26  2003/04/09 13:10:55  jzchen
* Split table driver into table driver only and forwarding engine
*
*
*/

#ifndef RTL8651_TBLDRV_LOCAL_H
#define RTL8651_TBLDRV_LOCAL_H

#include "rtl_types.h"
#include "rtl8651_tblDrv.h"
#include "rtl8651_tblAsicDrv.h"

//Internal data structure

//Front part same as header defined rtl8651_tblDrvMacAclRule_t

typedef struct rtl8651_tblDrv_global_s {
	uint32 icmpTimeout;
	uint32 udpTimeout;
	uint32 tcpLongTimeout;
	uint32 tcpMediumTimeout;
	uint32 tcpFastTimeout;
	uint16 l4PortRangeStart, l4PortRangeEnd;
} rtl8651_tblDrv_global_t;

typedef struct rtl8651_tblDrv_ethernet_s {
	uint32 	speed:2, //0: 10Mbps, 1:100Mbps, 2:1000Mbps
			duplex:1, //0: half-duplex 1:full-duplex
			autoNegotiation:1, //0:disable 1:Enable
			autoAdvCapability:2, // RTL8651_ETHER_AUTO_xxx
			inputBandwidthControl:4, // RTL8651_BC_xxx
			outputBandwidthControl:4, // RTL8651_BC_xxx
			linkUp:1; //0: Down 1:Up
	uint16	aggregatorIndex;//The corresponding entry index in link aggregation table
} rtl8651_tblDrv_ethernet_t;
 
typedef struct rtl8651_tblDrv_linkAggregation_s {
	uint16 pvid;//Port vlan ID
	uint16 	individual:1, // Whether this aggregator is indivual aggregator or not
			aggregated:1;//Whether this aggregator aggregate some links
	uint32 ethernetPortMask; //port i at 1<<i position, from 0 to 31
	uint32 ethernetPortUpStatus; //port i at 1<<i position, from 0 to 31
} rtl8651_tblDrv_linkAggregation_t;


/*
Note:
1. Entry property toCpu, srcBlocking destination blocking (memberPortMask) and normal forwarding (Neither toCpu nor srcBlocking) are 
    mutually exclusive. Therefore, only 2-bit to represent this situation. 
2. All entries are static configuration
3. Unable to process conflict macAddress entry process, return fail when following operation are conflict with current one.
*/
typedef struct rtl8651_tblDrv_filterDbTableEntry_s {
	ether_addr_t	macAddr;
	uint16	process:2,		//0: Normal forwarding, 1: destination blocking 2: source blocking 3: toCpu
			//Management flag
			userConfig:1,	//Manual configure this static entry
			refCount:8,		//Referenced by other table, such as 
			configToAsic:1,	//This entry is configured to ASIC
			asicPos:2;		//The entry position of the ASIC. Since rtl8651 only provides 4-entry. Only 2-bit is required
	uint32	memberPortMask; //port i at 1<<i position, from 0 to 31
	struct rtl8651_tblDrv_filterDbTableEntry_s * next;
} rtl8651_tblDrv_filterDbTableEntry_t;

typedef struct rtl8651_tblDrv_filterDbTable_s {
	uint16 sid;// Spanning tree ID, 0: CIST id, 1-4096 MST ID
	uint32 valid:1;	//Whether this filter database is valid
	rtl8651_tblDrv_filterDbTableEntry_t *database[RTL8651_L2TBL_ROW];
	uint32 freeNum;
} rtl8651_tblDrv_filterDbTable_t;

typedef struct rtl8651_tblDrv_spanningTreeTable_s {
	uint32	protocolWorking:1,	//Whether protocol is working to configure this instance
			valid:1;	//Whether this spanning tree is valid
	int8	portState[RTL8651_PORT_NUMBER];// 0: disabled, 1: blocking, 2: listening, 3: Learning, 4: Forwarding
} rtl8651_tblDrv_spanningTreeTable_t;

typedef struct rtl8651_tblDrv_pppoeTable_s {
	uint32	objectId;
	uint16	sessionId;
	uint16	vid;			//VLAN that execute pppoe protocol
	rtl8651_tblDrv_filterDbTableEntry_t * macInfo;	//Server mac information
	uint16	valid:1,		//Whether entry is valid
			initiated:1,	//Whether sessionId is valid. This bit also represent whether ASIC configured
			vlanMacSelect:3;
} rtl8651_tblDrv_pppoeTable_t;

typedef struct rtl8651_tblDrv_aclMacEntry_s {
	rtl8651_tblDrvMacAclRule_t rule;
	struct rtl8651_tblDrv_aclMacEntry_s	* next, *prev;
} rtl8651_tblDrv_aclMacEntry_t;

//Front part same as header defined rtl8651_tblDrvIpAclPara_t
typedef struct rtl8651_tblDrv_aclIpEntry_s {
	rtl8651_tblDrvIpAclRule_t rule;
	struct rtl8651_tblDrv_aclIpEntry_s * next, *prev;
} rtl8651_tblDrv_aclIpEntry_t;

typedef struct rtl8651_tblDrv_vlanTable_s {
	ether_addr_t macAddr;
	uint32 memberPortMask;	//port i at 1<<i position, from 0 to 31. The port means link aggregator instead of actual port
	uint32 memberPortUpStatus; //Whether member aggregator is up
	uint32 untagPortMask;	//port i at 1<<i position, 1 means enable untag 0 means disable untag.
	uint16 macAddrNumber;	// The number of consecutive mac address, 0 will disable ENRTR (Enable Routing)
	uint16 vid;				// VLAN ID
	uint16 fid;				//Filtering Database Index
	rtl8651_tblDrv_aclMacEntry_t * inAclHead, * inAclTail, * outAclHead, * outAclTail;
	rtl8651_tblDrv_aclIpEntry_t * inIpAclHead, * inIpAclTail, * outIpAclHead, * outIpAclTail;
	uint32 	aclAsicInputPos, aclAsicInputUsed, aclAsicInputMaxNumber, 
			aclAsicOutputPos, aclAsicOutputUsed, aclAsicOutputMaxNumber;
	uint16 	internal:1,		//Whether vlan is internal VLAN. This is configured by NAT control APIs
			DMZFlag:1, //1: DMZ external network interface, 0: external netwrok interface
			ipAttached:1,	//IP interface attached over it
			manualAssign:1,	//Whether MAC address is manually assigned
			promiscuous:1,
			aclDefPermit:1,	//0: when all rules mismatch DRPOP, 1: when rule mismatch PERMIT
			valid:1;			//Whether entry is valid
} rtl8651_tblDrv_vlanTable_t;

typedef struct rtl8651_tblDrv_arpEntry_s {
	ipaddr_t	ipAddr;
	rtl8651_tblDrv_filterDbTableEntry_t * macInfo;
	uint32	routingTablePos;//Meaningful when allocType == 2
	uint16	vid;
	uint32	allocType:2,	//Allocation type, 1: Local arp table 2: Routing table
			fromDrv:1,	//Whether entry is recorded by driver arp engine. If fromDrv == 1, allocType == 1
			fromApp:1,	//Whether entry is recorded by applications
			dmzIpPending:1, //Arp pending due to out of IP Table
			routeAllocated:1;//ASIC routing table allocated
	uint16 age;//Seconds as unit
	struct rtl8651_tblDrv_arpEntry_s * next; 
} rtl8651_tblDrv_arpEntry_t;

typedef struct rtl8651_tblDrv_ipIntfIpAddrEntry_s {
	ipaddr_t ipAddr;
	struct rtl8651_tblDrv_ipIntfIpAddrEntry_s * next;
}rtl8651_tblDrv_ipIntfIpAddrEntry_t;

typedef struct rtl8651_tblDrv_ipIntfEntry_s {
	rtl8651_tblDrv_ipIntfIpAddrEntry_t *	ipAddr;
	rtl8651_tblDrv_ipIntfIpAddrEntry_t *	localServer;
	ipaddr_t	ipMask;
	uint32	arpAllocated:1,		//ASIC arp table allocated
			routeAllocated:1,//ASIC routing table allocated
			networkType:2;	//Which type of network it is. 1:VLAN netowrk, 3.PPPoE network
	uint32 arpStartPos, arpEndPos;
	uint32 routingTablePos;
	struct rtl8651_tblDrv_ipIntfEntry_s * nextIp;
	rtl8651_tblDrv_arpEntry_t * nextArp;
} rtl8651_tblDrv_ipIntfEntry_t;

typedef struct rtl8651_tblDrv_natEntry_s {
	ipaddr_t localIp;
	ipaddr_t globalIp;
	struct rtl8651_tblDrv_natEntry_s * next;
} rtl8651_tblDrv_natEntry_t;

typedef struct rtl8651_tblDrv_naptServerPortEntry_s {
	ipaddr_t localIp;
	ipaddr_t globalIp;
	uint16 localPort, globalPort;
	uint32 asicTablePos;
	uint32 	tcp:1, //Entry for TCP server
			udp:1, //Entry for UDP server
			toAsic:1; //Entry configured to ASIC and asicTablePos is valid when toAsic==1
	struct rtl8651_tblDrv_naptServerPortEntry_s * next;
} rtl8651_tblDrv_naptServerPortEntry_t;

typedef struct rtl8651_tblDrv_networkIntfTable_s {
	int8	ifName[RTL8651_IPINTF_NAME_LEN];
	uint16 linkLayerIndex;	//Indicate to VLAN table or pppoe table object ID
	uint32 	valid:1,//Whether this interface is valid
			aclDefPermit:1, //0: when rule mismatch DRPOP, 1: when rule mismatch PERMIT
			//DMZFlag:1, //1: DMZ external network interface, 0: external netwrok interface
			linkLayerType:2; //Which kind of link layer. 0: Not attached, 1: VLAN 2: pppoe
	rtl8651_tblDrv_ipIntfEntry_t * IpHead;	//Network belong to this IP interface
	rtl8651_tblDrv_natEntry_t * nextNat;
	rtl8651_tblDrv_naptServerPortEntry_t * nextNaptServerPort;
	rtl8651_tblDrv_aclIpEntry_t * inAclHead, * inAclTail, * outAclHead, * outAclTail;
} rtl8651_tblDrv_networkIntfTable_t;

typedef struct rtl8651_tblDrv_routeTable_s {
	ipaddr_t ipAddr;
	ipaddr_t ipMask;
	ipaddr_t nextHop;
	uint32	networkIntfIndex;
	uint32	routingTablePos;//Meaningful when routeAllocated == 1
	uint16	vid;
	uint32	valid:1,//Entry valid
			intfNet:1, //Routing entry added due to interface creased. This means no further process will done
			macPending:1, //Route forward to CPU due to required MAC table entry not exist
			arpPending:1,//Route forward to CPU due to required ARP not exist
			pppoePending:1, //Route forward to CPU due to required pppoe not exist
			nextHopType:4, //Next Hop Vlan type=> 0:RTL8651_LL_NONE, 1: RTL8651_LL_VLAN, 2:RTL8651_LL_PPPOE
			routeAllocated:1;//ASIC routing table allocated
	rtl8651_tblDrv_arpEntry_t	*arpPtr; //point to next hop's arp if next hop is ether mac
	rtl8651_tblDrv_pppoeTable_t	*pppoePtr; //point to pppoe entry if next hop is pppoe
	rtl8651_tblDrv_ipIntfEntry_t * ipIntfPtr;//When intfNet == 1, this entry point to corresponding ip interface
} rtl8651_tblDrv_routeTable_t;



/* software use */
 /* used to save changes to ACK/sequence numbers */
struct l4ack_data_record				  
{
	uint32    ack_old;
	uint32    ack_new;
	int32     delta;
	int32     active;
};
#define L4_N_LINK_TCP_DATA 2


typedef struct rtl8651_tblDrv_naptTcpUdpFlowEntry_s {
	ipaddr_t insideLocalIpAddr;
	ipaddr_t insideGlobalIpAddr;
	ipaddr_t dstIpAddr;
	uint16 insideLocalPort;
	uint16 insideGlobalPort;
	uint16 dstPort;
	uint16 age;//Seconds as unit
	//When canAsic is 0 (Unable to forward by ASIC), this index point to the entry position that fill collsion bit for 
	// backward traffic trap to CPU.
	uint32 	tcp:1,
			tcpFlag:3, //TCP flag to achieve aging time out
			staticEntry:1,
			protoDel:1, //Whether protocol stack deletes this entry
			alive:1,	//Whether this entry alive or not
			forAlg:1, //If this bit is turned on, the entry will not be written to ASIC
			toAsic:1,
			fromAsic:1,//Whether this entry is generated by ASIC, means insideGlobalIpAddr, dstIpAddr and dstPort unknown. When configure this entry to ASIC, this is dynamic entry (Aging out by ASIC)
			fromDrv:1,//Whether this entry is generated by driver. Aging by driver (To asic as static)
			canAsic:1,//Whether this translation entry feasible to fill ASIC entry. If not, 				
			// 12
	// software use 		
			softAlg:1, // software alg 
			tcpTT:1, // tcp type for state machine, synch type or fin type
			tcpET:1, // tcp external to internal
			tcpIT:1, // tcp internal to external
			tcpConnected: 1, // tcp 3 way handshaking funished
			tcpAckModified:1, // tcp ack number to modify
			tcpFtpLastLineCRLFTermed:1, // for ftp alg use
			tcpStateINdex:2; // tcp ack array index			
			// 9
			
	 /* used to save changes to ACK/sequence numbers */
	struct l4ack_data_record ack[L4_N_LINK_TCP_DATA]; /* ack number */
				
	struct rtl8651_tblDrv_naptTcpUdpFlowEntry_s * next;
} rtl8651_tblDrv_naptTcpUdpFlowEntry_t;

/* TCP Flag value for tcpFlag */
#define 	RTL8651_TCPFLAGS_CLOSED			0x0		
#define 	RTL8651_TCPFLAGS_ESTABLISHED	0x4		
#define 	RTL8651_TCPFLAGS_2MSL			0x7


typedef struct rtl8651_tblDrv_naptIcmpFlowEntry_s {
	ipaddr_t insideLocalIpAddr;
	ipaddr_t insideGlobalIpAddr;
	ipaddr_t dstIpAddr;
	uint16 insideLocalId;
	uint16 insideGlobalId;
	uint16 age;//Seconds as unit
	uint16 count;//Number of unanswered query using this translation entry
	//When canAsic is 0 (Unable to forward by ASIC), this index point to the entry position that fill collsion bit for 
	// backward traffic trap to CPU.
	uint32 	staticEntry:1,
			protoDel:1, //Whether protocol stack deletes this entry
			alive:1, //Whether entry alive or not
			toAsic:1,
			fromAsic:1,//Whether this entry is read from ASIC, means insideGlobalIpAddr, dstIpAddr and dstPort unknown
			fromDrv:1,//Whether this entry is generated by driver. Aging by driver (To asic as static)
			canAsic:1;//Whether this translation entry feasible to fill ASIC entry. If not, 
	struct rtl8651_tblDrv_naptIcmpFlowEntry_s * next;
} rtl8651_tblDrv_naptIcmpFlowEntry_t;

typedef struct rtl8651_tblDrv_algSlot_s {
	uint16 algSlot;
	struct rtl8651_tblDrv_algSlot_s * next;
} rtl8651_tblDrv_algSlot_t;

typedef struct rtl8651_tblDrv_algEntry_s {
	uint16	startPort, endPort;
	rtl8651_tblDrv_algSlot_t * nextAlgSlot; //ALG trapping assume ordered allocation (From small port to large port)
	uint32	tcpServer:1,	//The ALG service is providing TCP server service. Trapping internal to external source port and external to internal destination port
			tcpClient:1,	//The ALG service is providing TCP client service. Trapping internal to external destination port and external to internal source port
			udpServer:1,	//The ALG service is providing UDP server service. Trapping internal to external source port and external to internal destination port
			udpClient:1,	//The ALG service is providing UDP client service. Trapping internal to external destination port and external to internal source port

	
			fwEngAdd:1; // entry is add by forwarding engine 
			
			
 	int32 	(*alg_outcb)(void* , void* ip, void *);	/* software use internal to external callback */
	int32 	(*alg_incb)(void* , void* ip, void *);	/* software use exnternal to internal callback */
			
	struct rtl8651_tblDrv_algEntry_s * next;
} rtl8651_tblDrv_algEntry_t;

//From rtl8651_tblDrv.c
extern rtl8651_tblDrv_vlanTable_t * rtl8651VlanTable;

rtl8651_tblDrv_arpEntry_t * _rtl8651_getArpEntry(ipaddr_t ipAddr, int8 isRefresh);
rtl8651_tblDrv_networkIntfTable_t * _rtl8651_getNetworkIntfEntryByAddr(ipaddr_t ipAddr);
rtl8651_tblDrv_networkIntfTable_t * _rtl8651_getNetworkIntfEntryByVid(uint16 vid);
rtl8651_tblDrv_ipIntfEntry_t * _rtl8651_getIpIntfEntryByAddr(ipaddr_t ipAddr);
int8 _rtl8651_addLocalArpSpecifyNetworkInterface(int8 fromDrv, ipaddr_t ipAddr, ether_addr_t * macAddr, rtl8651_tblDrv_networkIntfTable_t * netIntfPtr, uint32 port);
int8 _rtl8651_getNetworkIntfLinkLayerType(int8 *ifName, uint32 *llType, uint32 *llIndex);
int8 _rtl8651_extIpAddr(ipaddr_t ipAddr, uint32 *netIdx);
int8 _rtl8651_extNetIpAddr(ipaddr_t ipAddr, uint32 *natIdx);
int8 _rtl8651_localServerIpAddr(ipaddr_t ipAddr);
int8 _rtl8651_intIpAddr(ipaddr_t ipAddr);
int8 _rtl8651_intNetIpAddr(ipaddr_t ipAddr);
//int8 _rtl8651_reserveNaptTcpUdpExtPort(uint16 extPort);
int8 _rtl8651_freeReservedNaptTcpUdpExtPort(uint16 extPort);
int8 _rtl8651_getPppoeTblIdx(uint32 pppoeId, uint32 *tblIdx);
int8 _rtl8651_getAllNatMapping(int32 *entry, rtl8651_tblDrv_natEntry_t *natMap);
int8 _rtl8651_getAllServerPort(int32 *entry, rtl8651_tblDrv_naptServerPortEntry_t *serverPort);
int8 _rtl8651_isAclMismatchDrop(uint32 netIdx, int8 *isDrop);
int8 _rtl8651_getUsableExtIpAndPort(int8 isTcp, ipaddr_t sip, uint16 sport, ipaddr_t dip, uint16 dport, ipaddr_t *extip, uint16 *extport);
int8 _rtl8651_getVIDByGWIpAddr(ipaddr_t gwIpAddr, uint32 *vid);
int8 _rtl8651_addAlgNaptTcpUdpFlow(int8 flowType, ipaddr_t insideLocalIpAddr, uint16 insideLocalPort, 
		ipaddr_t *insideGlobalIpAddr, uint16 *insideGlobalPort, ipaddr_t dstIpAddr, uint16 dstPort);
int8 _rtl8651_delAlgNaptTcpUdpFlow(int8 isTcp, ipaddr_t insideLocalIpAddr, 
		uint16 insideLocalPort, ipaddr_t dstIpAddr, uint16 dstPort);
int8 _rtl8651_addNaptTcpUdpCandidateFlow(int8 fromDrv, int8 flowType, ipaddr_t insideLocalIpAddr, uint16 insideLocalPort, 
		ipaddr_t insideGlobalIpAddr, uint16 insideGlobalPort, ipaddr_t dstIpAddr, uint16 dstPort);
rtl8651_tblDrv_natEntry_t * _rtl8651_getNatExternalMappingEntry(ipaddr_t extIpAddr);
rtl8651_tblDrv_natEntry_t * _rtl8651_getNatInternalMappingEntry(ipaddr_t intIpAddr);
rtl8651_tblDrv_naptServerPortEntry_t *  _rtl8651_getOutsideServerPortMapping(int8 isTcp, ipaddr_t extIpAddr, uint16 extPort);
rtl8651_tblDrv_naptServerPortEntry_t *  _rtl8651_getInsideNaptServerPortMapping(int8 isTcp, ipaddr_t intIpAddr, uint16 intPort);
//rtl8651_tblDrv_naptTcpUdpFlowEntry_t * _rtl8651_getAsicNaptTcpUdpDynamicFlowToTable(uint32 flowTblIdx);
rtl8651_tblDrv_naptIcmpFlowEntry_t * _rtl8651_getAsicNaptIcmpDynamicFlowToTable(uint32 flowTblIdx);
rtl8651_tblDrv_algEntry_t *_rtl8651_getAlgEntry(int8 isTcp, int8 *isServer, int8 *isClient, uint16 sport, uint16 eport);
rtl8651_tblDrv_routeTable_t * _rtl8651_getRoutingEntry(ipaddr_t dstIpAddr);
rtl8651_tblDrv_naptTcpUdpFlowEntry_t * _rtl8651_getInsideNaptTcpUdpEntry(int8 isTcp, ipaddr_t sip, uint16 sport, ipaddr_t dip, uint16 dport, int8 isRefresh);
rtl8651_tblDrv_naptTcpUdpFlowEntry_t * _rtl8651_getOutsideNaptTcpUdpEntry(int8 isTcp, ipaddr_t gip, uint16 gport, ipaddr_t dip, uint16 dport, int8 isRefresh);
rtl8651_tblDrv_naptIcmpFlowEntry_t * _rtl8651_getInsideNaptIcmpEntry(ipaddr_t sip, uint16 sID, ipaddr_t dip, int8 isRefresh);
rtl8651_tblDrv_naptIcmpFlowEntry_t * _rtl8651_getOutsideNaptIcmpEntry(ipaddr_t gip, uint16 gID, ipaddr_t dip, int8 isRefresh);
rtl8651_tblDrvMacAclRule_t * _rtl8651_getMacAclEntry(int8 isIngress, uint16 vid, int8 *ether_hdr);
rtl8651_tblDrvIpAclRule_t * _rtl8651_getIpVlanAclEntry(int8 isIngress, uint16 vid, int8 *ip_hdr );
rtl8651_tblDrvIpAclRule_t * _rtl8651_getIpAclEntry(int8 isIngress, uint32 netIdx, int8 *ip_hdr);



//From rtl8651_tblDrvFwd.c
extern int8 rtl8651_drvArpProcessEnable;
extern rtl8651_tblDrvFwdEngineCounts_t rtl8651_fwdEngineCounter;
extern void (* rtl8651_tblDrvFwdSend)(void * data);//void * assumed to be packet header
extern int32 _rtl8651_drvArpGeneration(ipaddr_t ipAddr);

extern void (*rtl8651_resourceLock)(void);
extern void (*rtl8651_resourceUnlock)(void);
#define RTL8651_LOCK_RESOURCE()		if (rtl8651_resourceLock) rtl8651_resourceLock();
#define RTL8651_UNLOCK_RESOURCE()	if (rtl8651_resourceUnlock) rtl8651_resourceUnlock();

int8 _rtl8651_getUsableExtIpAndPort(int8 , ipaddr_t , uint16 , ipaddr_t , 
						uint16 , ipaddr_t *, uint16 *);


extern rtl8651_tblDrv_global_t	rtl8651GlobalControl;

#endif /* RTL8651_TBLDRV_LOCAL_H */

