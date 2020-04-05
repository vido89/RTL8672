/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Switch asic table driver rtl8651_tblAsicDrv.h
* Abstract : 
* Author : Edward Jin-Ru Chen (jzchen@realtek.com.tw)               
* $Id: rtl8651_tblAsicDrv.h,v 1.1.1.1 2003/08/18 05:43:22 kaohj Exp $
*
* $Log: rtl8651_tblAsicDrv.h,v $
* Revision 1.1.1.1  2003/08/18 05:43:22  kaohj
* initial import into CVS
*
* Revision 1.1  2003/05/19 09:22:01  elvis
* Initial version to BSP
*
* Revision 1.3  2003/04/28 15:22:35  elvis
* Synchronized with v1.25
*
*/

#ifndef RTL8651_TBLASICDRV_H
#define RTL8651_TBLASICDRV_H

#include "rtl_types.h"

#include "rtl8651_tblDrv.h"

// ASIC specification part

#define RTL8651_PORT_NUMBER			6
#define RTL8651_ALLPORTMASK			((1<<RTL8651_PORT_NUMBER)-1)
#ifdef RTL8651B
#define RTL8651_EXTPORT_NUMBER		3
#endif
#define RTL8651_AGGREGATOR_NUMBER	RTL8651_PORT_NUMBER
#define RTL8651_L2TBL_ROW				256
#define RTL8651_L2TBL_COLUMN			4
#define RTL8651_MAX_AGGREGATION_NUM	1
#define RTL8651_VLAN_NUMBER			8
#define RTL8651_PPPOE_NUMBER			8
#define RTL8651_FDB_NUMBER				1
#define RTL8651_STI_NUMBER				RTL8651_FDB_NUMBER
#define RTL8651_ROUTINGTBL_SIZE		8
#define RTL8651_ARPTBL_SIZE				512
#define RTL8651_PPPOETBL_SIZE			8
#define RTL8651_TCPUDPTBL_SIZE			1024
#define RTL8651_TCPUDPTBL_BITS			10
#define RTL8651_ICMPTBL_SIZE			32
#define RTL8651_IPTABLE_SIZE			8
#define RTL8651_SERVERPORTTBL_SIZE		8
#define RTL8651_ALGTBL_SIZE				128
#define RTL8651_MULTICASTTBL_SIZE		64
#define RTL8651_PROTOCOLTRAP_SIZE		8
#define RTL8651_ACLTBL_SIZE				126 //Rule 127 is used for permit all, Rule 126 is used for all to CPU
#define RTL8651_ACLTBL_RESERV_SIZE		2
#define RTL8651_ACLTBL_PERMIT_ALL		127
#define RTL8651_ACLTBL_ALL_TO_CPU		126
#define RTL8651_IPMULTICASTTBL_SIZE		64

typedef struct rtl8651_tblAsic_ethernet_s {
	uint32 linkUp:1;
} rtl8651_tblAsic_ethernet_t;

//extern rtl8651_tblAsic_ethernet_t rtl8651AsicEthernetTable[RTL8651_PORT_NUMBER];



uint32 rtl8651_filterDbIndex(ether_addr_t * macAddr);
uint32 rtl8651_vlanTableIndex(uint16 vid);
uint32 rtl8651_naptTcpUdpTableIndex(int8 isTCP, ipaddr_t srcAddr, uint16 srcPort, ipaddr_t destAddr, uint16 destPort);
uint32 rtl8651_naptIcmpTableIndex(ipaddr_t srcAddr, uint16 icmpId, ipaddr_t destAddr, uint32 * tblIdx);
uint32 rtl8651_ipMulticastTableIndex(ipaddr_t srcAddr, ipaddr_t dstAddr);

void rtl8651_getMacAddress(ether_addr_t * macAddress, uint32 * number);

int32 rtl8651_clearAsicAllTable(void);

int32 rtl8651_initAsic(void);

int32 rtl8651_setAsicOperationLayer(uint32 layer);
int32 rtl8651_getAsicOperationLayer(uint32 * layer);
int32 rtl8651_setAsicSpanningEnable(int8 spanningTreeEnabled);
int32 rtl8651_getAsicSpanningEnable(int8 *spanningTreeEnabled);

int32 rtl8651_installAsicEtherrnetLinkStatusNotifier(void (*notifier)(uint32 port, int8 linkUp));
int32 rtl8651_updateLinkStatus(void);
int32 rtl8651_setAsicEthernetLinkStatus(uint32 port, int8 linkUp);
int32 rtl8651_getAsicEthernetLinkStatus(uint32 port, int8 *linkUp);
int32 rtl8651_setAsicEthernetPHY(uint32 port, int8 autoNegotiation, uint32 advCapability, uint32 speed, int8 fullDuplex);
int32 rtl8651_getAsicEthernetPHY(uint32 port, int8 *autoNegotiation, uint32 *advCapability, uint32 *speed, int8 *fullDuplex);
int32 rtl8651_setAsicEthernetBandwidthControl(uint32 port, int8 input, uint32 rate);//RTL8651_ASICBC_xxx
int32 rtl8651_getAsicEthernetBandwidthControl(uint32 port, int8 input, uint32 *rate);//RTL8651_ASICBC_xxx

int32 rtl8651_setAsicMulticastSpanningTreePortState(uint32 port, uint32 portState);//RTL8651_PORTSTA_xxx
int32 rtl8651_getAsicMulticastSpanningTreePortState(uint32 port, uint32 *portState);//RTL8651_PORTSTA_xxx
int32 rtl8651_setAsicMulticastPortInternal(uint32 port, int8 isInternal);
int32 rtl8651_getAsicMulticastPortInternal(uint32 port, int8 *isInternal);

int32 rtl8651_setAsicL2Table(uint32 row, uint32 column, ether_addr_t * mac, int8 cpu, int8 srcBlk, uint32 mbr, uint32 ageSec, int8 isStatic, int8 nhFlag);
int32 rtl8651_delAsicL2Table(uint32 row, uint32 column);
//rtl8651_getAsicL2Table() is NULl allowed
int32 rtl8651_getAsicL2Table(uint32 row, uint32 column, ether_addr_t * mac, int8 * cpu, int8 * srcBlk, int8 * isStatic, uint32 * mbr, uint32 *ageSec, int8 *nhFlag);
void rtl8651_updateAsicLinkAggregatorLMPR(void);
int32 rtl8651_setAsicLinkAggregator(uint32 portMask);
int32 rtl8651_getAsicLinkAggregator(uint32 * portMask, uint32 *mapping);
int32 rtl8651_setAsicPvid(uint32 port, uint32 pvidx);
int32 rtl8651_getAsicPvid(uint32 port, uint32 *pvidx);
int32 rtl8651_setAsicVlan(uint16 vid, ether_addr_t * mac, uint32 mbr, 
	uint32 inAclStart, uint32 inAclEnd, uint32 outAclStart, uint32 outAclEnd,
	int8 internalIntf, int8 enableRoute, int8 *portState, int8 broadcastToCpu,
	int8 promiscuous, uint32 untagPortMask, uint32 macNumber, uint32 mtu);
int32 rtl8651_delAsicVlan(uint16 vid);
int32 rtl8651_getAsicVlan(uint16 vid, ether_addr_t * mac, uint32 * mbr, 
	uint32 * inAclStart, uint32 * inAclEnd, uint32 * outAclStart, uint32 * outAclEnd,
	int8 * internalIntf, int8 * enableRoute, int8 *portState, int8 * broadcastToCpu,
	int8 * promiscuous, uint32 * untagPortMask, uint32 * macNumber, uint32 * mtu);
int32 rtl8651_setAsicPppoe(uint32 index, uint16 sessionId);
int32 rtl8651_getAsicPppoe(uint32 index, uint16 *sessionId);
int32 rtl8651_setAsicRouting(uint32 index, ipaddr_t ipAddr, ipaddr_t ipMask, uint32 process, //0: pppoe, 1:direct, 2:indirect, 4:Strong CPU, 
							uint32 vidx, uint32 arpStart, uint32 arpEnd, 
							uint32 nextHopRow, uint32 nextHopColumn, uint32 pppoeIdx);
int32 rtl8651_delAsicRouting(uint32 index);
int32 rtl8651_getAsicRouting(uint32 index, ipaddr_t * ipAddr, ipaddr_t * ipMask, uint32 * process, //0: pppoe, 1:direct, 2:indirect, 4:Strong CPU, 
							uint32 * vidx, uint32 * arpStart, uint32 * arpEnd, 
							uint32 * nextHopRow, uint32 * nextHopColumn, uint32 * pppoeIdx);
int32 rtl8651_setAsicArp(uint32 index, uint32 nextHopRow, uint32 nextHopColumn);
int32 rtl8651_delAsicArp(uint32 index);
int32 rtl8651_getAsicArp(uint32 index, uint32 *nextHopRow, uint32 *nextHopColumn);
int32 rtl8651_setAsicGidxRegister(uint32 regValue);
int32 rtl8651_getAsicGidxRegister(uint32 * reg);
int32 rtl8651_setAsicExtIntIpTable(uint32 index, ipaddr_t extIpAddr, ipaddr_t intIpAddr, int8 localPublic, int8 nat);
int32 rtl8651_delAsicExtIntIpTable(uint32 index);
int32 rtl8651_getAsicExtIntIpTable(uint32 index, ipaddr_t *extIpAddr, ipaddr_t *intIpAddr, int8 *localPublic, int8 *nat);
int32 rtl8651_setAsicServerPortTable(uint32 index, ipaddr_t extIpAddr, uint16 extPort, ipaddr_t intIpAddr, uint16 intPort);
int32 rtl8651_delAsicServerPortTable(uint32 index);
int32 rtl8651_getAsicServerPortTable(uint32 index, ipaddr_t *extIpAddr, uint16 *extPort, ipaddr_t *intIpAddr, uint16 *intPort);
int32 rtl8651_setAsicAgingFunction(int8 l2Enable, int8 l4Enable);
int32 rtl8651_getAsicAgingFunction(int8 * l2Enable, int8 * l4Enable);
int32 rtl8651_setAsicNaptAutoAddDelete(int8 autoAdd, int8 autoDelete);
int32 rtl8651_getAsicNaptAutoAddDelete(int8 *autoAdd, int8 *autoDelete);
int32 rtl8651_setAsicNaptIcmpTimeout(uint32 timeout);
int32 rtl8651_getAsicNaptIcmpTimeout(uint32 *timeout);
int32 rtl8651_setAsicNaptUdpTimeout(uint32 timeout);
int32 rtl8651_getAsicNaptUdpTimeout(uint32 *timeout);
int32 rtl8651_setAsicNaptTcpLongTimeout(uint32 timeout);
int32 rtl8651_getAsicNaptTcpLongTimeout(uint32 *timeout);
int32 rtl8651_setAsicNaptTcpMediumTimeout(uint32 timeout);
int32 rtl8651_getAsicNaptTcpMediumTimeout(uint32 *timeout);
int32 rtl8651_setAsicNaptTcpFastTimeout(uint32 timeout);
int32 rtl8651_getAsicNaptTcpFastTimeout(uint32 *timeout);
int32 rtl8651_setAsicNaptTcpUdpTable(int8 forced, 
		ipaddr_t insideLocalIpAddr, uint16 insideLocalPort, uint16 insideGlobalPort, 
		uint16 ageSec, int8 isStatic, int8 isTcp, int8 tcpFlag, int8 isCollision, int8 isValid);
int32 rtl8651_getAsicNaptTcpUdpTable(int8 precisePort, uint16 targetGlobalPort, 
		ipaddr_t *insideLocalIpAddr, uint16 *insideLocalPort, uint16 *insideGlobalPort, 
		uint16 *ageSec, int8 * isStatic, int8 *isTcp, int8 *tcpFlag, int8 *isCollision, int8 *isValid);
int32 rtl8651_setAsicNaptIcmpTable(int8 forced, 
		ipaddr_t insideLocalIpAddr, uint16 insideLocalId, uint16 insideGlobalId, 
		uint16 ageSec, int8 isStatic, int16 count, int8 isCollision, int8 isValid);
int32 rtl8651_getAsicNaptIcmpTable(int8 precisePort, uint16 targetGlobalId, 
		ipaddr_t *insideLocalIpAddr, uint16 *insideLocalId, uint16 *insideGlobalId, //insideGlobalId is meaningful in first 6-bit and last 5-bit (Medium 5-bit is lost)
		uint16 *ageSec, int8 *isStatic, uint16 *count, int8 *isCollision, int8 *isValid);
int32 rtl8651_setAsicL4Offset(uint16 start, uint16 end);
int32 rtl8651_getAsicL4Offset(uint16 *start, uint16 *end);
int32 rtl8651_setAsicAlg(uint32 index, uint16 port);
int32 rtl8651_delAsicAlg(uint32 index);
int32 rtl8651_getAsicAlg(uint32 index, uint16 *port);
int32 rtl8651_setAsicMacAcl(uint32 index, rtl8651_tblDrvMacAclRule_t * rule);
int32 rtl8651_getAsicMacAcl(uint32 index, rtl8651_tblDrvMacAclRule_t * rule);
int32 rtl8651_setAsicIpAcl(uint32 index, rtl8651_tblDrvIpAclRule_t * rule);
int32 rtl8651_getAsicIpAcl(uint32 index, rtl8651_tblDrvIpAclRule_t * rule);
int32 rtl8651_getAsicNaptTcpUdpOffset(uint16 index, uint16 * offset, int8 * isValid);
int32 rtl8651_getAsicNaptIcmpOffset(uint16 index, uint16 * offset, int8 * isValid);
int32 rtl8651_testAsicDrv(void);

#ifndef CYGWIN

typedef struct {
    uint32 spa;
    uint32 bc;
    uint32 vid;
    uint32 vlan;
    uint32 pppoe;
    uint8  sip[4];
    uint32 sprt;
                       
    uint8  dip[4];
    uint32 dprt;
    
    uint32 ipptl;
    uint32 ipflg;
    uint32 iptos;
    uint32 tcpflg;
    uint32 type;
    uint32 prtnmat;
	uint32 ethrtype;
    uint8  da[6];
    uint8  pad1[2];
    uint8  sa[6];
    uint8  pad2[2];
    uint32 hp;
    uint32 llc;
    uint32 udp_nocs;
    uint32 ttlst;
    uint32 pktend;
    uint32 dirtx;
    uint32 l4crcok;
    uint32 l3crcok;
    uint32 ipfragif;
    uint32 dp ;
    uint32 hp2;
#ifdef RTL8651B
	uint16	ipLen;
	uint8	L2only;
#endif
	
} rtl8651_tblAsic_hsb_param_watch_t;

typedef struct {

	uint8  mac[6];
	uint8  pad1[2];
	uint8  ip[4];
	uint32 prt;
	uint32 l3cs;
	uint32 l4cs;
	uint32 egress;
	uint32 l2act;
	uint32 l34act;
	uint32 dirtx;
	uint32 type;
	uint32 llc;
	uint32 vlan;
	uint32 dvid;
	uint32 pppoe;
	uint32 pppid;
	uint32 ttl_1;
	uint32 dpc;									
	uint32 bc;
	uint32 pktend;
	uint32 mulcst;
	uint32 svid;
	uint32 cpursn;
	uint32 spa;

} rtl8651_tblAsic_hsa_param_watch_t;

uint32 rtl8651_returnAsicCounter(uint32 offset);
uint32 rtl8651_clearAsicCounter(void);
void rtl8651_intHandler(void);

int32 rtl8651_getAsicHsB(rtl8651_tblAsic_hsb_param_watch_t * hsbWatch);
int32 rtl8651_getAsicHsA(rtl8651_tblAsic_hsa_param_watch_t * hsaWatch);
#endif

#endif
