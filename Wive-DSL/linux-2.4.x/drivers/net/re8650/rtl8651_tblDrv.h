/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Switch core table driver rtl8651_tblDrv.c
* Abstract : 
* Author : Edward Jin-Ru Chen (jzchen@realtek.com.tw)               
* $Id: rtl8651_tblDrv.h,v 1.1.1.1 2003/08/18 05:43:22 kaohj Exp $
* $Log: rtl8651_tblDrv.h,v $
* Revision 1.1.1.1  2003/08/18 05:43:22  kaohj
* initial import into CVS
*
* Revision 1.1  2003/05/19 09:22:00  elvis
* Initial version to BSP
*
* Revision 1.5  2003/04/30 06:38:03  elvis
* remove linkChangePendingCount
*
* Revision 1.4  2003/04/28 15:20:40  elvis
* Synchronized with v1.63
*
* Revision 1.3  2003/04/24 09:57:51  elvis
* Synchronized with v 1.61 on main trunk
*
* Revision 1.2  2003/04/10 15:09:09  elvis
* declare  linkChangePendingCount
*
* Revision 1.1.1.2  2003/03/31 06:29:37  elvis
* no message
*
* Revision 1.1.1.1  2003/03/31 06:09:26  elvis
* no message
*
* Revision 1.45  2003/03/24 03:46:18  jzchen
* Export spanning tree protocol working api. Add driver napt forwarding enable/disable api
*
* Revision 1.44  2003/03/19 06:03:40  jzchen
* Add napt update and time update apis
*
* Revision 1.43  2003/03/18 01:31:41  jzchen
* Add NAPT TCP/UDP flow update and time update apis
*
* Revision 1.42  2003/03/12 12:01:54  jzchen
* Add forwarding engine counter and make arp and icmp can enable/disable dynamically
*
* Revision 1.41  2003/03/10 08:09:12  jzchen
* Export configure bandwidth control api
*
* Revision 1.40  2003/03/07 12:04:41  jzchen
* Add port untag configuraiton api
*
* Revision 1.39  2003/03/07 07:19:30  cfliu
* Change rtl8651_getEthernetPortLinkStatus prototype.
*
* Revision 1.38  2003/03/07 01:54:19  jzchen
* 1. Initial parameter changed
*
* Revision 1.37  2003/03/04 05:36:15  jzchen
* Change initial function, server port function parameter
*
* Revision 1.36  2003/03/03 12:57:56  jzchen
* Add VLAN promiscuous mode configuration function
*
* Revision 1.35  2003/03/03 11:48:41  jzchen
* Initial version of link, link aggregator and vlan status processing
*
* Revision 1.34  2003/03/03 07:04:40  jzchen
* Remove link aggregator and vlan status notifier installation function and add server port APIs
*
* Revision 1.33  2003/03/03 01:30:51  jzchen
* Add link, link aggregator and vlan status notification.
*
* Revision 1.32  2003/02/26 12:01:32  jzchen
* Change add ARP parameter from vid to interface name
*
* Revision 1.31  2003/02/21 02:32:03  jzchen
* Add bandwidth control parameter definition
*
* Revision 1.30  2003/02/19 09:31:27  cfliu
* Change rtl8651_setPvid() prototype...
*
* Revision 1.29  2003/02/19 06:52:04  jzchen
* Remove global information when try to delete napt flow
*
* Revision 1.28  2003/02/19 06:03:17  jzchen
* Add asic driver shell and server port APIs
*
* Revision 1.27  2003/02/19 01:19:21  jzchen
* Add autoNegotiation api and remove linkUp api
*
* Revision 1.26  2003/02/18 12:51:22  jzchen
* Add the capability to add/del/flush IP rule per vlan
*
* Revision 1.25  2003/02/18 11:49:56  jzchen
* Remove duplicated function - get pppoe source mac address
*
* Revision 1.24  2003/02/18 10:13:54  jzchen
* Change from macaddr_t to ether_addr_t
*
* Revision 1.23  2003/02/18 06:25:47  cfliu
* Reorganize all exported APIs and add brief command syntax...
*
* Revision 1.22  2003/02/17 10:10:41  jzchen
* Provide API to hook spaning tree instance & filter database and filter database & VLAN
*
* Revision 1.21  2003/02/17 09:37:53  jzchen
* Exchange the definition of IP interface and network interface
*
* Revision 1.20  2003/02/17 07:40:34  jzchen
* Exchange the definition of IP interface and network interface
*
* Revision 1.19  2003/02/17 06:53:30  jzchen
* Code model change to tblDrv and tblAsicDrv
*
* Revision 1.18  2003/02/17 03:30:03  jzchen
* Update public api
*
* Revision 1.17  2003/02/14 12:07:03  jzchen
* Initial alg/acl add/delete/flush API
*
* Revision 1.16  2003/02/13 09:06:18  jzchen
* Add ACL & ALG initial version Definitions
*
* Revision 1.15  2003/02/13 03:11:33  jzchen
* 1. Move internal data structure into rtl8651_tblDrv.c.
* 2. Move testing api from rtl8651_tblDrv.h into rtl8651_tblDrvLocal.h
* 3. Define emulation parameters in rtl8651_tblDrvLocal.h
* 4. Remove extern variables from rtl8651_tblDrv_test.c
*
* Revision 1.14  2003/02/11 12:55:07  jzchen
* Start icmp flow table management.
*
* Revision 1.13  2003/02/10 09:23:09  jzchen
* Add ethernet APIs
*
* Revision 1.12  2003/02/06 09:47:23  jzchen
* Change get l2 table and set l4 tcp/udp flow table api parameters
*
* Revision 1.11  2003/01/30 03:02:07  jzchen
* Add re-initialization API and testing get/set APIs
*
* Revision 1.10  2003/01/28 08:15:49  jzchen
* Add naptTcpUdpFlowSize parameter
*
* Revision 1.9  2003/01/27 00:00:46  jzchen
* Remove unnecessary configuration parameter
*
* Revision 1.8  2003/01/26 09:33:52  jzchen
* Modify NAT control parameters
*
* Revision 1.7  2003/01/17 05:33:43  jzchen
* Initial ASIC emulation table
*
* Revision 1.6  2003/01/15 07:15:58  jzchen
* Link layer type definition
*
* Revision 1.5  2003/01/13 14:38:10  jzchen
* Add filter database functions
*
* Revision 1.4  2003/01/10 10:45:41  jzchen
* Add MAC address database definition
*
* Revision 1.3  2003/01/09 12:28:01  jzchen
* Add network interface api and try to arrange arp and routing entry
*
* Revision 1.2  2003/01/09 03:34:34  jzchen
* Add Public IP, NAT, Server, multiple IP in one network
*
* Revision 1.1  2003/01/08 14:50:20  jzchen
* Change to 8651 model
*
*
*/

#ifndef RTL8651_TBLDRV_H
#define RTL8651_TBLDRV_H

#include "rtl_types.h"
#define RTL8651_IPINTF_NAME_LEN		16

typedef struct rtl8651_tblDrvInitPara_s {
	uint32	macAddressDbSize;
	uint32	filterDbSize;
	uint32	networkIntfTableSize;
	uint32	ipIntfEntryNumber;
	uint32	ipIntfIpAddrNumber;
	uint32	arpTableSize;
	uint32	routingTableSize;
	uint32	natSize;
	uint32	natCtlSize;
	uint32	naptServerPortEntryNumber;
	uint32	naptTcpUdpFlowSize;
	uint32	naptIcmpFlowSize;
	uint32	macAclSize;
	uint32	ipAclSize;
	uint32	algSize;
//Arrangement parameter	
	uint32	aclPerVlanInitInputEntrySize;
	uint32	aclPerVlanInitOutputEntrySize;
	uint32  reservedPortSize;
} rtl8651_tblDrvInitPara_t;

typedef struct rtl8651_tblDrvMacAclRule_s {
	ether_addr_t dstMac, dstMacMask;
	ether_addr_t srcMac, srcMacMask;
	uint16 typeLen, typeLenMask;
	uint32	actionType:3;// RTL8651_ACL_xx (PERMIT, DROP)
} rtl8651_tblDrvMacAclRule_t;

typedef struct rtl8651_tblDrvIpAclRule_s {
	ipaddr_t srcIpAddr, srcIpAddrMask;
	ipaddr_t dstIpAddr, dstIpAddrMask;
	uint8 tos, tosMask;
	union {
		struct {
			uint8 proto, protoMask, flagMask;// flag & flagMask only last 3-bit is meaning ful
			union {
				uint8 _flag;
				struct {
					uint8 pend1:5,
						  pend2:1,
						  _DF:1,	//don't fragment flag
						  _MF:1;	//more fragments flag
				} s;
			} un;						
		} ip;
		struct {
			uint8 type, typeMask, code, codeMask;
		} icmp;
		struct {
			uint8 type, typeMask;
		} igmp;
		struct {
			uint8 flagMask;
			uint16 srcPortUpperBound, srcPortLowerBound;
			uint16 dstPortUpperBound, dstPortLowerBound;
			union {
				uint8 _flag;
				struct {
					uint8 pend:2,
						  urg:1, //urgent bit
						  ack:1, //ack bit
						  psh:1, //push bit
						  rst:1, //reset bit
						  syn:1, //sync bit
						  fin:1; //fin bit
				} s;
			} un;
		} tcp;
		struct {
			uint16 srcPortUpperBound, srcPortLowerBound;
			uint16 dstPortUpperBound, dstPortLowerBound;
		} udp;
	} is;
	uint32	ruleType:3, // RTL8651_ACL_xxx
			actionType:3; // RTL8651_ACL_xx (PERMIT, DROP)
}rtl8651_tblDrvIpAclRule_t;

#define ipFlag		un._flag
#define ipMF		un.s._MF
#define ipDF		un.s._DF
#define ipRSV 		un.s.pend2

#define tcpflag		un._flag
#define tcpURG		un.s.urg
#define tcpACK		un.s.ack
#define tcpPSH		un.s.psh
#define tcpRST		un.s.rst
#define tcpSYN		un.s.syn
#define tcpFIN		un.s.fin


typedef struct rtl8651_tblDrvFwdEngineCounts_s {
	uint32 arpReceive, arpRelay, arpAddSuccess, arpAddFail, arpDelete, arpReply, arpGenerate;
	uint32 ipReceive, ipErrorReceive;
	uint32 icmpReceive, icmpErrorReceive, icmpEchoReply, icmpDrop;
	uint32 igmpReceive, igmpErrorReceive, icmpSend;
	uint32 udpReceive, udpErrorReceive,udpSend;
	uint32 tcpReceive, tcpErrorReceive,tcpSend;
	uint32 pppoeDiscoveryReceive, pppoeDiscoveryProcessed, pppoeReply;
	uint32 pppoeSessionReceive, pppoeSessionProcessed;
	uint32 inAclDrop, inAclPermit,inAclCpu;
	uint32 outAclDrop, outAclPermit, outAclCpu;

} rtl8651_tblDrvFwdEngineCounts_t;

//Exported API categorized by usage

// system 
int32 rtl8651_installResourceLock(void (*lockFunction)(void));
int32 rtl8651_installResourceUnlock(void (*unlockFunction)(void));
int32 rtl8651_installSendFunction(void (*sendFunction)(void * data));
void  rtl8651_timeUpdate(uint32 secPassed);

// phy-port
//when auto-eng is enabled, use negotiated capability and specified max capability to determine what link speed and duplex mode to use.
//when auto-neg is off, the speed and duplex mode given is forced to write to ASIC
int32 rtl8651_installEtherrnetLinkStatusNotifier(void (*notifier)(uint32 port, int8 linkUp));
int32 rtl8651_getEthernetPortLinkStatus(uint32 port, int8 * linkUp, uint16 *speed, int8 *fullduplex, int8 *autoNeg);
int32 rtl8651_setEthernetPortDuplexMode(uint32 port, int8 fullDuplex);
int32 rtl8651_setEthernetPortSpeed(uint32 port, uint32 speed);
int32 rtl8651_setEthernetPortAutoNegotiation(uint32 port, int8 autoNegotiation, uint32 advCapability);
int32 rtl8651_setEthernetPortBandwidthControl(uint32 port, int8 input, uint32 rate);


// spanning-tree 
//not yet tested when protocol work is on. protocol working default disabled so port state won't change dynamically.
int32 rtl8651_addSpanningTreeInstance(uint16 sid);
int32 rtl8651_setSpanningTreeInstanceProtocolWorking(uint16 sid, int8 working);
int32 rtl8651_delSpanningTreeInstance(uint16 sid);
int32 rtl8651_setSpanningTreeInstancePortState(uint16 stid, uint16 port, uint8 portState);

//filter-database 
int32 rtl8651_addFilterDatabase(uint16 fid);
int32 rtl8651_delFilterDatabase(uint16 fid);
int32 rtl8651_specifyFilterDatabaseSpanningTreeInstance(uint16 fid, uint16 sid);
int32 rtl8651_addFilterDatabaseEntry(uint16 fid, ether_addr_t * macAddr, uint32 type, uint32 portMask);
int32 rtl8651_delFilterDatabaseEntry(uint16 fid, ether_addr_t * macAddr);

// port 
//this 'port' is actually a logical port(=aggregator), that is, not physical port number. 
//RTL8651 support only one aggregator group. port VID is binded on logical port
int32 rtl8651_setAggregatorIndividual(uint16 aggregator, int8 individual);
int32 rtl8651_setPortAggregator(uint32 port, uint16 aggregator);
int32 rtl8651_getAggregatorActiveStatus(uint16 aggregator, int8 * isActive);
int32 rtl8651_setPvid(uint32 aggregator, uint16 vid);

//vlan 
//allocate a VLAN, together with ingress/egress ACL rule space allocated.
int32 rtl8651_addVlan(uint16 vid);
//free ACL rules allocated and reset member port's pvid to default VID
int32 rtl8651_delVlan(uint16 vid);
//specify VLAN's default ACL action is drop or permit for this vlan
int32 rtl8651_setVlanAclMismatchDrop(uint16 vid, int8 isDrop); //chhuang
//valid mac addr num is 1,2,4,8 (0 is not allowed, all VLANs need at least one MAC)
int32 rtl8651_allocateVlanMacAddress(uint16 vid, uint16 macAddrNumber);
int32 rtl8651_getVlanMacAddress(uint16 vid, ether_addr_t * macAddr, uint16 * macAddrNumber);
//if user wants to assign MAC addresses, ( last byte of MAC address given ) must be multiple of  (macAddrNumber of this VLAN ) 
int32 rtl8651_assignVlanMacAddress(uint16 vid, ether_addr_t * macAddress, uint16 macAddrNumber);
int32 rtl8651_delVlanPortMember(uint16 vid, uint32 port);
int32 rtl8651_addVlanPortMember(uint16 vid, uint32 port);
int32 rtl8651_specifyVlanFilterDatabase(uint16 vid, uint16 fid);
//when a vlan is in promiscuous mode, no network/IP interface can be attached on this VLAN and routing to/from
//this VLAN is automatically disabled.
int32 rtl8651_setVlanPromiscuous(uint16 vid, int8 isPromiscuous);
int32 rtl8651_getVlanActiveStatus(uint16 vid, int8 * isActive);
int32 rtl8651_setVlanPortUntag(uint16 vid, uint32 port, int8 untag);

//pppoe 
//add a PPPoE object, an ASIC PPPoE  entry allocated here. Session ID should be set later.
int32 rtl8651_addPppoeSession(uint32 pppoeId, uint16 vid);
int32 rtl8651_delPppoeSession(uint32 pppoeId);
//config properties of a PPPoE interface. PPPoE asic entry not written to ASIC until this function called.
int32 rtl8651_setPppoeSessionProperty(uint32 pppoeId, uint16 sid, ether_addr_t * macAddr, uint32 port);
//linear scan routing entry to find which route refers to this PPPoE session. Both located routing entry 
//and PPPoE entry got reset.
int32 rtl8651_resetPppoeSessionProperty(uint32 pppoeId);
int32 rtl8651_getPppoeSessionMac(uint32 pppoeId, ether_addr_t * macAddr);

//interface 
//network interface: an IP broadcast domain. This is a pseudo layer between IP interface and VLAN 
//network interface is "external" if at least one NAT host, or one NAPT address, or one DMZ host refers to this network interface, 
//otherwise it is an "internal " interface by default.
int32 rtl8651_addNetworkIntf(int8 *ifName); //a route will be created  for each IP interface added
//If a routing entry/DMZ host/NAT host refers to the network interface to be deleted, delete operation is forbidden. 
int32 rtl8651_delNetworkIntf(int8 *ifName);
//specify this network interface''s default ACL action is drop or permit for this vlan
int32 rtl8651_setNetIfAclMismatchDrop(int8 *ifName, int8 isDrop); //chhuang
int32 rtl8651_addExtNetworkInterface(int8 * ifName); //make this network interface  for NAPT
//free ExtIPTable table entries occupied and makes this interfafe internal
int32 rtl8651_delExtNetworkInterface(int8 * ifName);
//Add a DMZ network makes this internal interface external.
//a DMZ host won't be written to ExtIPTable unless its ARP is learned.
int32 rtl8651_addDmzNetworkInterface(int8 *ifName);
int32 rtl8651_delDmzNetworkInterface(int8 *ifName);
int32 rtl8651_specifyNetworkIntfLinkLayerType(int8 * ifName, uint32 llType, uint32 llIndex);
int32 rtl8651_removeNetworkIntfLinkLayerType(int8 * ifName);
	//IP interface: an IP subnet. multiple IP subnets can bind on the same network interface
	int32 rtl8651_addIpIntf(int8 * ifName, ipaddr_t ipAddr, ipaddr_t ipMask);//
	int32 rtl8651_delIpIntf(int8 * ifName, ipaddr_t ipAddr, ipaddr_t ipMask);

//arp 
int32 rtl8651_addArp(ipaddr_t ipAddr, ether_addr_t * macAddr, int8 * ifName, uint32 port);
int32 rtl8651_delArp(ipaddr_t ipAddr);

//nat 
int32 rtl8651_addNatMapping(ipaddr_t extIpAddr, ipaddr_t intIpAddr); //....for NAT addresses
int32 rtl8651_delNatMapping(ipaddr_t extIpAddr, ipaddr_t intIpAddr);

//local-server 
int32 rtl8651_addLocalServer(ipaddr_t ipAddr); // No NAT between Private IP and Local Public IPs
int32 rtl8651_delLocalServer(ipaddr_t ipAddr);

//route 
int32 rtl8651_addRoute(ipaddr_t ipAddr, ipaddr_t ipMask, int8 * ifName, ipaddr_t nextHop);
int32 rtl8651_delRoute(ipaddr_t ipAddr, ipaddr_t ipMask);

//napt
int32 rtl8651_setNaptIcmpTimeout(uint32 timeout);
int32 rtl8651_setNaptUdpTimeout(uint32 timeout);
int32 rtl8651_setNaptTcpLongTimeout(uint32 timeout);
int32 rtl8651_setNaptTcpMediumTimeout(uint32 timeout);
int32 rtl8651_setNaptTcpFastTimeout(uint32 timeout);
int32 rtl8651_setNaptTcpUdpPortRange(uint16 start, uint16 end);

int32 rtl8651_addNaptServerPortMapping(int8 isTcp, ipaddr_t extIpAddr, uint16 extPort, ipaddr_t intIpAddr, uint16 intPort);
int32 rtl8651_delNaptServerPortMapping(int8 isTcp, ipaddr_t extIpAddr, uint16 extPort, ipaddr_t intIpAddr, uint16 intPort);

int32 rtl8651_flushNaptFlow(void);
int32 rtl8651_addNaptTcpUdpFlow(int8 assigned, int8 flowType, ipaddr_t insideLocalIpAddr, uint16 insideLocalPort, 
			ipaddr_t *insideGlobalIpAddr, uint16 *insideGlobalPort, ipaddr_t dstIpAddr, uint16 dstPort);
#define UDP_FLOW							0 	/* Create a UDP entry */
#define TCP_LISTEN_FLOW					1	/* Reserved entry for TCP connection in LISTEN_STATE*/
#define TCP_OUTBOUND_SYN_RCVD_FLOW		2	/* Create entry due to outbound SYN rcvd */
#define TCP_INBOUND_SYN_RCVD_FLOW		3	/* Create entry due to inbound SYN rcvd */
#define TCP_ESTABLISHED_FLOW				5	/* Create entry after 3-way handshaking completed */

int32 rtl8651_delNaptTcpUdpFlow(int8 isTcp, ipaddr_t insideLocalIpAddr,
			uint16 insideLocalPort, ipaddr_t dstIpAddr, uint16 dstPort);
void rtl8651_updateAsicNaptTcpUdpTable(void);
int32 rtl8651_addNaptIcmpFlow(int8 assigned, ipaddr_t insideLocalIpAddr, uint16 insideLocalId, 
			ipaddr_t *insideGlobalIpAddr, uint16 *insideGlobalId, ipaddr_t dstIpAddr);
int32 rtl8651_delNaptIcmpFlow(ipaddr_t insideLocalIpAddr, uint16 insideLocalId, ipaddr_t dstIpAddr);
void rtl8651_updateAsicIcmpTable(void);


//mac-acl ...
int32 rtl8651_addMacAcl(uint16 vid, int8 inputCheck, rtl8651_tblDrvMacAclRule_t * rule);
int32 rtl8651_delMacAcl(uint16 vid, int8 inputCheck, rtl8651_tblDrvMacAclRule_t * rule);
int32 rtl8651_flushMacAcl(uint16 vid, int8 inputCheck);

//ip-acl 
int32 rtl8651_addIpAcl(int8 * ifName, int8 inputCheck, rtl8651_tblDrvIpAclRule_t * rule);
int32 rtl8651_delIpAcl(int8 * ifName, int8 inputCheck, rtl8651_tblDrvIpAclRule_t * rule);
int32 rtl8651_flushIpAcl(int8 * ifName, int8 inputCheck);
int32 rtl8651_addVlanIpAcl(uint16 vid, int8 inputCheck, rtl8651_tblDrvIpAclRule_t * rule);
int32 rtl8651_delVlanIpAcl(uint16 vid, int8 inputCheck, rtl8651_tblDrvIpAclRule_t * rule);
int32 rtl8651_flushVlanIpAcl(uint16 vid, int8 inputCheck);


//alg-trap 
int32 rtl8651_addALGRule(uint16 startPort, uint16 endPort, int8 isTcp, int8 isServer, int8 isClient);
int32 rtl8651_delALGRule(uint16 startPort, uint16 endPort, int8 isTcp, int8 isServer, int8 isClient);

// Other functions
int32 rtl8651_tblDrvReinit(void);
int32 rtl8651_tblDrvInit(rtl8651_tblDrvInitPara_t * configPara);
int32 rtl8651_tblDrvFwd_init(void);
int32 rtl8651_getPppoeSessionSourceMac(uint32 pppoeId, ether_addr_t * macAddr);
ipaddr_t rtl8651_getNaptInsideGlobalIpAddr(int8 isTcp, ipaddr_t insideLocalIpAddr, uint16 insideLocalPortOrId, ipaddr_t dstIpAddr, uint16 dstPort);

//Forwarding engine
int32 rtl8651_drvNicProcessArp(int8 enable);
int32 rtl8651_drvNicProcessArpGeneration(int8 enable);
int32 rtl8651_drvNicProcessIcmp(int8 enable);
int32 rtl8651_drvNicNaptFwd(int8 naptEnable);
int32 rtl8651_drvNicEnablePPPoE(int8 enable);
int32 rtl8651_drvNicProcess(void * data);
int32 rtl8651_getFwdEngineCounter(rtl8651_tblDrvFwdEngineCounts_t * counter);

int32 rtl8651_setArpAgingTime(uint16 agTime);


//Ethernet port bandwidth control
#define RTL8651_BC_FULL		0x00
#define RTL8651_BC_128K		0x01
#define RTL8651_BC_256K		0x02
#define RTL8651_BC_512K		0x03
#define RTL8651_BC_1M		0x04
#define RTL8651_BC_2M		0x05
#define RTL8651_BC_4M		0x06
#define RTL8651_BC_8M		0x07

#define RTL8651_ETHER_AUTO_100FULL	0x00
#define RTL8651_ETHER_AUTO_100HALF	0x01
#define RTL8651_ETHER_AUTO_10FULL	0x02
#define RTL8651_ETHER_AUTO_10HALF	0x03

#define RTL8651_OP_CONFIG		0x00
#define RTL8651_OP_DELETE		0x01

#define RTL8651_FORWARD_MAC		0x00
#define RTL8651_DSTBLOCK_MAC	0x01
#define RTL8651_SRCBLOCK_MAC	0x02
#define RTL8651_TOCPU_MAC		0x03

#define RTL8651_PORTSTA_DISABLED		0x00
#define RTL8651_PORTSTA_BLOCKING		0x01
#define RTL8651_PORTSTA_LISTENING	0x02
#define RTL8651_PORTSTA_LEARNING		0x03
#define RTL8651_PORTSTA_FORWARDING	0x04

#define RTL8651_LL_NONE				0x00
#define RTL8651_LL_VLAN				0x01
#define RTL8651_LL_PPPOE			0x02

#define RTL8651_ACL_PERMIT			0x01
#define RTL8651_ACL_DROP			0x02
#define RTL8651_ACL_CPU				0x03

/* ACL Rule type Definition */
#define RTL8651_ACL_MAC				0x00
#define RTL8651_ACL_IP				0x01
#define RTL8651_ACL_ICMP			0x02
#define RTL8651_ACL_IGMP			0x03
#define RTL8651_ACL_TCP				0x04
#define RTL8651_ACL_UDP				0x05

//Table Driver ERROR NUMBER Definition
#define TBLDRV_EINVVAL					-1	//Invalid Input Value
#define TBLDRV_ENOFREEBUF				-2	//Out of Free list/buffer
#define TBLDRV_EENTRYEXIST				-3	//Specified Entry alreay exists
#define TBLDRV_EENTRYNOTFOUND			-4	//Specified Entry not found
#define TBLDRV_EINVPORT					-5	//Invalid Port Number
#define TBLDRV_EINVAGGREATOR			-6	//Invalid Aggregator ID		
#define TBLDRV_EINVVLAN					-7	//Invalid VLAN ID
#define TBLDRV_EINVPPPOE				-8	//Invalid PPPoE ID
#define TBLDRV_EINVFID					-9	//Invalid Filter DataBase ID
#define TBLDRV_EINVSID					-10	//Invalid Spanning Tree Instance ID

#define TBLDRV_ENOTALLOWPORTAGGREGATED	-11	//The action forbids specified port to 
												//be aggregated
#define TBLDRV_ESIDEXIST				-12	//SID already exists
#define TBLDRV_ESIDREFERENCED			-13	//SID is being referenced
#define TBLDRV_EFIDEXIST				-14	//Filter DataBase already exists
#define TBLDRV_EFIDREFERENCED			-15	//FID is being referenced
#define TBLDRV_EAGGREGATING				-16	//Aggregator is aggregating some ports
#define TBLDRV_EAGGREHASNOPORT			-17	//Each aggregator at least should have one port
#define TBLDRV_EAGGREGTPORT				-18 	//Forbid aggregator ID greater than port ID
#define TBLDRV_EAGGREINDIVIDUAL			-19	//Individual flag of aggregator was set:
												//aggregator can not aggregate more than one port
#define TBLDRV_EAGGREEXCEED				-20	//Aggregating exceeds system capacity:
												//Only N aggregators are allowed
#define TBLDRV_EDIFFERENTBCASTDOMAIN	-21	//Aggregator and Port are not in the same BroadCast Domain
#define TBLDRV_EVLANEXIST				-22	//Vlan already exists
#define TBLDRV_EVLANREFERENCED			-23	//Vlan is being referenced
#define TBLDRV_ENOTVLANPORTMEMBER		-24	//Specified port is not a member port 
												//of specified vlan
#define TBLDRV_EVLANMACREFERENCED		-25	//Vlan MAC is referenced by pppoe/network interface
#define TBLDRV_EINVVLANMAC				-26	//Specified VLAN MAC number should be (0, 1, 2, 4, 8)
#define TBLDRV_ENOUSABLEMAC				-27	//No usable MAC address can be allocated
#define TBLDRV_EVLANNOMAC				-28	//Vlan has no MAC Address
#define TBLDRV_EVLANMACNOTFREED			-29	//Vlan still has MAC address 
#define TBLDRV_EVLANPROMIS				-30	//Vlan enables promiscuous mode: vlan with pppoe should
												//not enable promiscuous mode
#define TBLDRV_EMULTIPPPOE				-31	//Vlan doesn't allow multiple PPPoE session.
#define TBLDRV_EPPPOEREFERENCED			-32	//PPPoE is being referenced
#define TBLDRV_EPROPERTYCONFLICT		-33	//PPPoE Property already exist (conflict)
#define TBLDRV_EPROPERTYWASSET			-34	//PPPoE Property was set
#define TBLDRV_EADDFDFAIL				-35	//Can not add entry into filter database
#define TBLDRV_EINVNETIFNAME			-36	//Invalid Network Interface name
#define TBLDRV_EIFNAMEEXIST				-37	//Interface name already exists
#define TBLDRV_ENETIFREFBYIPIF			-38	//Network Interface is referenced BY IP interface
#define TBLDRV_ENETIFREFBYROUTE			-39	//Network Interface is referenced BY Routing
#define TBLDRV_ENETIFREFBYNAT			-40	//Network Interface is referenced BY NAT/NAPT
#define TBLDRV_ENETIFREFBYACL			-41	//Network Interface is referenced BY ACL
#define TBLDRV_ELLNOTRESET				-42	//Link Layer type of Network Interface is not reset
#define TBLDRV_ELLSPECIFIED				-43	//Link Layer was specified before
#define TBLDRV_ELLIDXEXIST				-44	//Link Layer Index already exists
#define TBLDRV_ENOLLSPECIFIED			-45	//No Link layer Type specified
#define TBLDRV_ENETEXTERNAL				-46	//Network Interface already registered as external interface
#define TBLDRV_ENOTNATEXTIF				-47	//Specified Network interface is not a NAT external interface
#define TBLDRV_ESHOULDNOTDMZEXT			-48	//Should not DMZ external
#define TBLDRV_ENOPPPOEFOUND			-49	//NO Corresponding PPPoE ID found
#define TBLDRV_ENAPTSTART				-50	//When NAPT starts, no manipulation about IP interface
												//should be done
#define TBLDRV_EDSTIPISLS				-51	//NAPT DST IP is Local Server
#define TBLDRV_EGETOFFSETFAIL			-52	//NAPT get offset fail
#define TBLDRV_EOUTOFIPTBLENTRY			-53  //Not enough IP table entry for NAT/NAPT/Local-Server
#define TBLDRV_EINVALIDEXTPORT			-54	//Maybe the external port is in use or reserved
#define TBLDRV_EIPIFEXIST				-55	//IP Interface already exists
#define TBLDRV_EIPIFNONIFARP			-56	//Added IP Interface belongs to non-Local Arp 
#define TBLDRV_EIPIFREFERENCED			-57	//IP Interface is referenced
#define TBLDRV_EINVIPIF					-58	//Invalid IP Interface
#define TBLDRV_ENOIPIFFOUND				-59	//No IP Interface(subnet) found
#define TBLDRV_EOUTOFARPSLOT			-60	//Added Subnet is too large to stored in arp slot
#define TBLDRV_ENONBCASTNET				-61	//Non-BroadCast Network
#define TBLDRV_EARPCANOTFORGW			-62  //Added arp entry can not for gateway
#define TBLDRV_ELSEXIST					-63	//Local Server already exist
#define TBLDRV_ELSISGWIF				-64	//Local Server should not be gateway interface address
//#define TBLDRV_ELSNOTNATEXTIP			-65	//Local Server doesn't belong to NAT external IP interface
#define TBLDRV_ELSNOTFOUND				-66	//Local Server not found
//#define TBLDRV_ELSNETNOTFOUND			-67	//Can not find Network Interface to which the Local Server 
#define TBLDRV_ERTEXIST					-68	//Routing entry already exists
#define TBLDRV_ENHOPNOTNETIF			-69	//NextHop doesn't belong to a network interface
#define TBLDRV_EDEFRTEXIST				-70	//Default Route already exists
#define TBLDRV_ENOTDMZEXT				-71	//Not DMZ external network interface
#define TBLDRV_ENEEDVLANTYPE			-72  //The network interface should be a VLAN type
												//belongs
#define TBLDRV_ERANGEOVERLAP			-73	//ALG port range overlap
#define TBLDRV_EDEFAULTVLAN				-74	//Default VLAN should not be rmeoved
#define TBLDRV_ESESSIONID				-75  //Invalid PPPoE Session ID
#define TBLDRV_ENOTPERMIT				-76	//The action is not allowed
#define TBLDRV_ENOTEXTIP				-77	//Specified external IP address doesn't belong to external interface
#define TBLDRV_ENOTINTIP				-78	//Specified internal IP address doesn't belong to internal interface
#define TBLDRV_ESTARTGTEND				-79	//NAPT start port > NAPT end port
#define TBLDRV_EEXTIDINUSE				-80 //NAPT external ID is in use
#define TBLDRV_EINVNETMASK				-90 //Invalid network mask
#define TBLDRV_ERTCREATEBYIF			-91 //The route is added by IP interface creation
#define TBLDRV_ESHOULDBEEXTIF			-92 //The request should be external network interface

#if 0

/* errno for port */

#define TBLDRV_EPORTEXIST			0x0101	//Port already exists
#define TBLDRV_EPORTAGGREGATED		0x0102	//Specified Port already aggregated:
											//Some settings forbid port to be aggregated
/* errno for aggregator */											
									





#define TBLDRV_EDIFFBCASTDOMAIN		0x0206	//Aggregator and Port are not in the same BroadCast Domain

/* errno for vlan */

#define TBLDRV_VLANEXIST			0x0301	//Vlan already exists


#define TBLDRV_EINVVLANMACNO		0x0304	//Specified VLAN MAC number should be (0, 1, 2, 4, 8)





/* errno for PPPoE */





#endif






#endif

