/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Switch asic table driver rtl8651_tblAsicDrv.c
* Abstract : 
* Author : Edward Jin-Ru Chen (jzchen@realtek.com.tw)               
* 
* $Id: rtl8651_tblAsicDrv.c,v 1.1.1.1 2003/08/18 05:43:22 kaohj Exp $
* $Log: rtl8651_tblAsicDrv.c,v $
* Revision 1.1.1.1  2003/08/18 05:43:22  kaohj
* initial import into CVS
*
* Revision 1.1  2003/05/19 09:22:01  elvis
* Initial version to BSP
*
* Revision 1.7  2003/05/02 09:03:43  orlando
* rtl8651_getMacAddress with bdinfo.
*
* Revision 1.6  2003/04/30 06:38:25  elvis
* remove linkChangePendingCount
*
* Revision 1.5  2003/04/28 15:24:30  elvis
* Synchronized with v1.54
*
* Revision 1.4  2003/04/24 09:56:56  elvis
* Synchronized with v 1.54 on main trunk
*
* Revision 1.3  2003/04/08 15:28:33  elvis
* base mac-address is read from board.c
*
* Revision 1.2  2003/04/02 01:02:23  elvis
* comment out some forwarding-cache codes
*
* Revision 1.1.1.2  2003/03/31 06:29:37  elvis
* no message
*
* Revision 1.1.1.1  2003/03/31 06:09:26  elvis
* no message
*
*
* Last: rtl8651_tblAsicDrv.c,v 1.39 2003/03/26 06:24:13 jzchen
*/
#include "assert.h"
#include "rtl_types.h"
#include "rtl8651_tblAsicDrv.h"
#include "rtl8651_asicRegs.h"
#include "phy.h"
//#include "board.h"
#define RTL_STATIC_INLINE   static __inline__
//For data structure defined used int swTable


#define NUMBER_OF_VLAN_TABLE_ENTRY             		8
#define NUMBER_OF_PROTOCOL_TRAP_TABLE_ENTRY	8
#define NUMBER_OF_EXT_INTERNAL_IP_TABLE_ENTRY	8
#define NUMBER_OF_SERVER_PORT_TABLE_ENTRY		8
#define NUMBER_OF_ALG_TABLE_ENTRY				128
#define NUMBER_OF_L3_ROUTING_TABLE_ENTRY		8
#define NUMBER_OF_ARP_TABLE_ENTRY				512
#define NUMBER_OF_PPPOE_TABLE_ENTRY				8
#define NUMBER_OF_L2_SWITCH_TABLE_ENTRY			1024
#define NUMBER_OF_MULTICAST_TABLE_ENTRY			64
#define NUMBER_OF_L4_TCP_UDP_TABLE_ENTRY		1024
#define NUMBER_OF_L4_ICMP_TABLE_ENTRY			32
#define NUMBER_OF_ACL_TABLE_ENTRY				128

typedef struct {
    /* word 0 */
    uint16          mac31_16;
    uint16          mac15_0;
    
    /* word 1 */
    uint16          vhid        : 9;
    uint16          memberPort  : 6;
    uint16          valid       : 1;
    uint16          mac47_32;
    /* word 2 */
    uint8           reserv5     : 1;
    uint8           outACLEnd   : 7;
    uint8           reserv4     : 1;
    uint8           outACLStart : 7;
    uint8           reserv3     : 1;
    uint8           inACLEnd    : 7;
    uint8           reserv2     : 1;
    uint8           inACLStart  : 7;
    /* word 3 */
    uint32          mtuL        : 8;
    uint32          macMask     : 2;
    uint32          egressUntag : 6;
    uint32          promiscuous : 1;
    uint32          bcastToCPU  : 1;
    uint32          STPStatus   : 12;
    uint32          enHWRoute   : 1;
    uint32          isInternal  : 1;
    /* word 4 */
#ifdef RTL8651B
    uint32          	reserv7     : 17,
	    			extSTPStatus:6,
	    			extEgressUntag:3,
	    			extMemberPort:3,
#else
	uint32		reserv7:29,
#endif
              		mtuH        : 3;
} rtl8651_tblAsic_vlanTable_t;

typedef struct {
    uint16          reserv5     : 12;
    uint16          valid       : 1;
    uint16          trapProtocol: 3;
    uint16          trapContent;
} rtl8651_tblAsic_protoTrapTable_t;

typedef struct {
    /* word 0 */
    ipaddr_t        internalIP;
    /* word 1 */
    ipaddr_t        externalIP;
    /* word 2 */
    uint32          reserv0     : 29;
    uint16          isLocalPublic   : 1;
    uint16          isOne2One       : 1;
    uint16          valid       : 1;
} rtl8651_tblAsic_extIpTable_t;

typedef struct {
    /* word 0 */
    ipaddr_t        internalIP;
    /* word 1 */
    ipaddr_t        externalIP;
    /* word 2 */
    uint16          externalPort;
    uint16          internalPort;
    /* word 3 */
    uint32          reserv0     : 31;
    uint32          valid       : 1;
} rtl8651_tblAsic_srvPortTable_t;

typedef struct {
    uint16          reserv      : 15;
    uint16          valid       : 1;
    uint16          L4Port;
} rtl8651_tblAsic_algTable_t;

typedef struct {
    /* word 0 */
    ipaddr_t        IPAddr;
    /* word 1 */
    union {
        struct {
            uint32          reserv0     : 7;
            uint32          ARPEnd      : 6;
            uint32          ARPStart    : 6;
            uint32          IPMask      : 5;
            uint32          vid         : 3;
            uint32          hPriority   : 1;
            uint32          process     : 3;
            uint32          valid       : 1;
        } ARPEntry;
        struct {
            uint32          reserv0     : 9;
            uint32          nextHop     : 10;
            uint32          IPMask      : 5;
            uint32          vid         : 3;
            uint32          hPriority   : 1;
            uint32          process     : 3;
            uint32          valid       : 1;
        } L2Entry;
        struct {
            uint32          reserv0     : 6;
            uint32          PPPoEIndex  : 3;
            uint32          nextHop     : 10;
            uint32          IPMask      : 5;
            uint32          vid         : 3;
            uint32          hPriority   : 1;
            uint32          process     : 3;
            uint32          valid       : 1;        
        } PPPoEEntry;
    } linkTo;
    /* word 2 added for driver management */
    uint32          idx;
} rtl8651_tblAsic_l3RouteTable_t;

typedef struct {
    uint32          reserv0     : 21;
    uint32          nextHop     : 10;
    uint32          valid       : 1;
} rtl8651_tblAsic_arpTable_t;

typedef struct {
    uint16          reserv0;
    uint16          sessionID;
} rtl8651_tblAsic_pppoeTable_t;

typedef struct {
    /* word 0 */
    uint16          mac39_24;
    uint16          mac23_8;
    /* word 1 */
#ifdef RTL8651B
    uint32          reserv0     : 5,
		 	   extMemberPortMask: 3,
#else
    uint32          reserv0     : 11,
#endif
	              nxtHostFlag : 1,
	              srcBlock    : 1,
	              agingTime   : 2,
	              isStatic    : 1,
		
	              toCPU       : 1,
	              hPriority   : 1,
	              memberPort  : 6,
	              mac47_40    : 8;
} rtl8651_tblAsic_l2Table_t;


typedef struct {
    /* word 0 */
    ipaddr_t        intIPAddr;

    /* word 1 */

#ifdef RTL8651B
	uint32	reserv0     : 2,
		selEntryIdx:10,
		selIPidx:3,
#else
	uint32	reserv0     : 15,
#endif



#ifdef RTL8651B		
		isStatic    : 1,
	#define RTL8651_LIBERAL_NAPT_ENTRY			2

		dedicate:1,
		reserv1		: 1,

	//In RTL8651B, all 3 bits in TCPFlag field is reused. Don't change these values!!!
	#define RTL8651_OUTBOUND_NAPT_ENTRY			3	//exact value in ASIC
	#define RTL8651_INBOUND_NAPT_ENTRY			2	//exact value in ASIC
	#define RTL8651_UNIDIRECTIONAL_NAPT_ENTRY		2	//exact value in ASIC
	#define RTL8651_QUIET_NAPT_ENTRY				4	//exact value in ASIC 
	
#else	
		isStatic    : 1,
		reserve1:2,
#endif		

		offset      : 6,
		agingTime   : 6,
		collision   : 1,
		valid       : 1;

		/* word 2 */
	uint32	reserv2     : 12,
		isTCP       : 1,
		TCPFlag     : 3,
		intPort:16;
} rtl8651_tblAsic_naptTcpUdpTable_t;

typedef struct {
    /* word 0 */
    ipaddr_t        intIPAddr;
    /* word 1 */
    uint32          ICMPIDL     : 15;
    uint32          isStatic    : 1;
    uint32          reserv1 : 2;
    uint32          offset      : 6;
    uint32          agingTime   : 6;
    uint32          collision   : 1;
    uint32          valid       : 1;
    /* word 2 */
    uint32          reserv2     : 15;
    uint32          count       : 16;
    uint32          ICMPIDH     : 1;
} rtl8651_tblAsic_naptIcmpTable_t;

typedef struct {
    union {
        struct {
            /* word 0 */
            uint16          dMacP31_16;
            uint16          dMacP15_0;
            /* word 1 */
            uint16          dMacM15_0;
            uint16          dMacP47_32;
            /* word 2 */
            uint16          dMacM47_32;
            uint16          dMacM31_16;
            /* word 3 */
            uint16          sMacP31_16;
            uint16          sMacP15_0;
            /* word 4 */
            uint16          sMacM15_0;
            uint16          sMacP47_32;
            /* word 5 */
            uint16          sMacM47_32;
            uint16          sMacM31_16;
            /* word 6 */
            uint16          ethTypeM;
            uint16          ethTypeP;
        } ETHERNET;
        struct {
            /* word 0 */
            uint32          reserv1     : 24;
            uint32          gidxSel     : 8;
            /* word 1~6 */
            uint32          reserv2[6];
        } IFSEL;
        struct {
            /* word 0 */
            ipaddr_t        sIPP;
            /* word 1 */
            ipaddr_t        sIPM;
            /* word 2 */
            ipaddr_t        dIPP;
            /* word 3 */
            ipaddr_t        dIPM;
            union {
                struct {
                    /* word 4 */
                    uint8           IPProtoM;
                    uint8           IPProtoP;
                    uint8           IPTOSM;
                    uint8           IPTOSP;
                    /* word 5 */
                    uint32          reserv0     : 26;
                    uint32          IPFlagM     : 3;
                    uint32          IPFlagP     : 3;
                    /* word 6 */
                    uint32          reserv1;
                } IP;
                struct {
                    /* word 4 */
                    uint8           ICMPTypeM;
                    uint8           ICMPTypeP;
                    uint8           IPTOSM;
                    uint8           IPTOSP;
                    /* word 5 */
                    uint16          reserv0;
                    uint8           ICMPCodeM;
                    uint8           ICMPCodeP;
                    /* word 6 */
                    uint32          reserv1;
                } ICMP;
                struct {
                    /* word 4 */
                    uint8           IGMPTypeM;
                    uint8           IGMPTypeP;
                    uint8           IPTOSM;
                    uint8           IPTOSP;
                    /* word 5,6 */
                    uint32          reserv0[2];
                } IGMP;
                struct {
                    /* word 4 */
                    uint8           TCPFlagM;
                    uint8           TCPFlagP;
                    uint8           IPTOSM;
                    uint8           IPTOSP;
                    /* word 5 */
                    uint16          TCPSPLB;
                    uint16          TCPSPUB;
                    /* word 6 */
                    uint16          TCPDPLB;
                    uint16          TCPDPUB;
                } TCP;
                struct {
                    /* word 4 */
                    uint16          reserv0;
                    uint8           IPTOSM;
                    uint8           IPTOSP;
                    /* word 5 */
                    uint16          UDPSPLB;
                    uint16          UDPSPUB;
                    /* word 6 */
                    uint16          UDPDPLB;
                    uint16          UDPDPUB;
                } UDP;
            } is;
        } L3L4;
    } is;
    /* word 7 */
    uint32          reserv0     : 7;
    uint32          PPPoEIndex  : 3;
    uint32          vid         : 3;
    uint32          hPriority   : 1;
    uint32          nextHop     : 10;
    uint32          actionType  : 4;
    uint32          ruleType    : 4;
} rtl8651_tblAsic_aclTable_t;

typedef struct {
    /* word 0 */
    ipaddr_t        srcIPAddr;
    /* word 1 */
    uint32          srcPortL    : 1;
    uint32          srcVid      : 3;
    uint32          destIPAddrLsbs : 28;
    /* word 2*/
    /* word 2*/
#ifdef RTL8651B
    uint32          reserv0     : 15;
	uint32	extPortList:3;	//extension port mask
	uint32   	extSrcPort:1;	//{ extSrcPort, srcPortH, srcPortL } determines source port number
#else
    uint32          reserv0     : 19;
#endif
    uint32          toCPU       : 1;
    uint32          valid       : 1;
    uint32          extIPIndex  : 3;
    uint32          portList    : 6;
    uint32          srcPortH    : 2;
} rtl8651_tblAsic_ipMulticastTable_t;

#define HSB_BASE                 0xBC0F0000
#define HSA_BASE                 0xBC0F0040

/* HSB access parameters
*/
typedef struct {

    uint32 sip2_0       : 3;  // w0[31:29]
    uint32 pppoe        : 1;  // w0[28]    
    uint32 vlan         : 1;  // w0[27]    
    uint32 vid          : 12; // w0[26:15]    
    uint32 bc           : 11; // w0[14:4]    
    uint32 spa          : 4;  // w0[3:0]
                       
	uint32 sprt2_0      : 3;  // w1[31:29]
	uint32 sip31_3      : 29; // w1[28:0]	
                       
    uint32 dip18_0      : 19; // w2[31:13]	
	uint32 sprt15_3     : 13; // w2[12:0]
                       
    uint32 ipptl2_0     : 3;  // w3[31:29]
    uint32 dprt         : 16; // w3[28:13]    
    uint32 dip31_19     : 13; // w3[12:0]

	uint8  ethrtype3_0  : 4;  // w4[31:28]
    uint8  prtnmat      : 1;  // w4[27]
    uint8  type         : 3;  // w4[26:24]
    uint8  tcpflg          ;  // w4[23:16]
    uint8  iptos           ;  // w4[15:8]
    uint8  ipflg        : 3;  // w4[7:5]
    uint8  ipptl7_3     : 5;  // w4[4:0]

    uint32 da19_0       : 20; // w5[31:12]    
    uint32 ethrtype15_4 : 12; // w5[11:0]

    uint32 sa3_0        : 4;  // w6[31:28]
    uint32 da47_20      : 28; // w6[27:0]

    uint32 sa35_4           ; // w7[31:0]
    
    uint32 reserved1    : 1;  // w8[31]
    uint32 ipfragif     : 1;  // w8[30]
    uint32 l3crcok      : 1;  // w8[29]
    uint32 l4crcok      : 1;  // w8[28]
    uint32 dirtx        : 1;  // w8[27]
    uint32 pktend       : 10; // w8[26:17]
    uint32 ttlst        : 2;  // w8[16:15]
    uint32 udp_nocs     : 1;  // w8[14]
    uint32 llc          : 1;  // w8[13]
    uint32 hp           : 1;  // w8[12]
    uint32 sa47_36      : 12; // w8[11:0]

    uint32 reserved3    : 12; // w9[31:20]
    uint32 hp2          : 1;  // w9[19]
    uint32 dp           : 7;  // w9[18:12]
    uint32 reserved2    : 12; // w9[11:0]

#ifdef RTL8651B
	uint32 	reserved4:13,	//w10
			ipLen:16,	//cfliu:maybe we don't need this?
			L2only:1,
			extSrcPortNum:2;
#endif

} rtl8651_tblAsic_hsb_param_t;

/* HSA access parameters
*/
typedef struct {

	uint32 mac31_0          ; // w0[31:0]

	uint16 ip15_0           ; // w1[31:16]	
	uint16 mac47_32         ; // w1[15:0]

	uint16 prt              ; // w2[31:16]
	uint16 ip31_16          ; // w2[15:0]

	uint16 l4cs             ; // w3[31:16]
	uint16 l3cs             ; // w3[15:0]
	
	uint32 bc4_0        : 5;  // w4[31:27]
	uint32 dpc          : 3;  // w4[26:24]
	uint32 ttl_1        : 6;  // w4[23:18]
	uint32 pppid        : 3;  // w4[17:15]
	uint32 pppoe        : 2;  // w4[14:13]
	uint32 dvid         : 3;  // w4[12:10]
	uint32 vlan         : 2;  // w4[9:8]
	uint32 llc          : 1;  // w4[7]
	uint32 type         : 3;  // w4[6:4]
	uint32 dirtx        : 1;  // w4[3]
	uint32 l34act       : 1;  // w4[2]
	uint32 l2act        : 1;  // w4[1]	
	uint32 egress       : 1;  // w4[0]

	
	uint32 spa          : 3;  // w5[31:29]
	uint32 cpursn       : 9;  // w5[28:20]
	uint32 svid         : 3;  // w5[19:17]
	uint32 mulcst       : 1;  // w5[16]
	uint32 pktend       : 10; // w5[15:6]
	uint32 bc10_5        : 6;  // w5[5:0]

#ifdef RTL8651B
	uint32 	frag2cksumDiff:16,
			ACLtrapFlag:2, //cfliu: why 2 bits here???
			extSrcPortNum:2,
			extDstPortMask:4,
			extTTL_1:3;

#endif
} rtl8651_tblAsic_hsa_param_t;


rtl8651_tblAsic_ethernet_t rtl8651AsicEthernetTable[RTL8651_PORT_NUMBER];
void (*rtl8651_asicLinkStatusNotifier)(uint32 port, int8 linkUp);
extern uint32 linkChangePendingCount;

static uint32 _rtl8651_NaptAgingToSec(uint32 value);
static uint32 _rtl8651_NaptAgingToUnit(uint32 sec);


//Hardware dependent API, with same API but implementation is setting emulation table instead of ASIC table
uint32 rtl8651_filterDbIndex(ether_addr_t * macAddr) {
    return ( macAddr->octet[0] ^ macAddr->octet[1] ^
                    macAddr->octet[2] ^ macAddr->octet[3] ^
                    macAddr->octet[4] ^ macAddr->octet[5]) & 0xFF;
}

uint32 rtl8651_vlanTableIndex(uint16 vid) {
	return (vid&7);
}

uint32 rtl8651_naptTcpUdpTableIndex(int8 isTCP, ipaddr_t srcAddr, uint16 srcPort, ipaddr_t destAddr, uint16 destPort) {
	uint32 eidx;
	
	eidx = srcAddr ^ (srcAddr >> 10) ^ (srcAddr >> 20) ^ (srcAddr >> 30) ^ (srcPort << 2) ^ (srcPort >> 8) ^ 
		destAddr ^ (destAddr >> 10) ^ (destAddr >> 20) ^ (destAddr >> 30) ^ (destPort << 2) ^ (destPort >> 8);
	if(destPort & 0x8000 )
		eidx ^= 0x300;

	if(isTCP)
		eidx = (eidx ^ 0x300) & (RTL8651_TCPUDPTBL_SIZE-1);
	else
		eidx &= (RTL8651_TCPUDPTBL_SIZE-1);;
	assert(eidx < RTL8651_TCPUDPTBL_SIZE);
	return eidx;
}

uint32 rtl8651_naptIcmpTableIndex(ipaddr_t srcAddr, uint16 icmpId, ipaddr_t destAddr, uint32 * tblIdx) {
	uint32 eidx;
	
	eidx = srcAddr ^ (srcAddr >> 10) ^ (srcAddr >> 20) ^ (srcAddr >> 30) ^ (icmpId << 2) ^ (icmpId >> 8) ^ 
		destAddr ^ (destAddr >> 10) ^ (destAddr >> 20) ^ (destAddr >> 30);
	*tblIdx = eidx &(RTL8651_ICMPTBL_SIZE-1);
	eidx &= (RTL8651_TCPUDPTBL_SIZE-1);

	return eidx;
}

uint32 rtl8651_ipMulticastTableIndex(ipaddr_t srcAddr, ipaddr_t dstAddr) {
	uint32 idx;

	idx = srcAddr ^ (srcAddr>>8) ^ (srcAddr>>16) ^ (srcAddr>>24) ^ dstAddr ^ (dstAddr>>8) ^ (dstAddr>>16) ^ (dstAddr>>24);
	idx = ((idx >> 2) ^ (idx & 0x3)) & (RTL8651_IPMULTICASTTBL_SIZE-1);

	return idx;
}

static uint32 _rtl8651_NaptAgingToSec(uint32 value) {
	//convert differentiated timer value to actual second value
	uint32 unit = 1;
	while(value > 8) {
		value -= 7;
		unit *= 8;
	}
	return value * unit;
}

static uint32 _rtl8651_NaptAgingToUnit(uint32 sec) {
	//convert actual second value to differentiated timer value
	uint32 acc = 0;
	while(sec > 8) {
		sec /= 8;
		acc += 7;
	}
	return  acc + sec;
}
int32 rtl8651_installAsicEtherrnetLinkStatusNotifier(void (*notifier)(uint32 port, int8 linkUp)) {
	rtl8651_asicLinkStatusNotifier = notifier;
	return SUCCESS;
}

int32 rtl8651_setAsicEthernetLinkStatus(uint32 port, int8 linkUp) {
	int8 notify;
	if(port >= RTL8651_PORT_NUMBER)
		return FAILED;
	if(rtl8651_asicLinkStatusNotifier && rtl8651AsicEthernetTable[port].linkUp != (linkUp==TRUE? 1: 0)) 
		notify = TRUE;
	else
		notify = FALSE;
	rtl8651AsicEthernetTable[port].linkUp = linkUp == TRUE? 1: 0;
	rtl8651_updateAsicLinkAggregatorLMPR();//Since link aggregation port mapping is referenced to link status
	if(notify==TRUE)
		rtl8651_asicLinkStatusNotifier(port, linkUp);
	return SUCCESS;
}

int32 rtl8651_getAsicEthernetLinkStatus(uint32 port, int8 *linkUp) {
	if(port >= RTL8651_PORT_NUMBER || linkUp == NULL)
		return FAILED;
	*linkUp = rtl8651AsicEthernetTable[port].linkUp == 1? TRUE: FALSE;
	return SUCCESS;
}


#define RTL8651_ASICTABLE_BASE_OF_ALL_TABLES      0xbc000000

static const void * rtl8651_asicTableAccessAddrBase[] = {
    (void *) RTL8651_ASICTABLE_BASE_OF_ALL_TABLES + 0x00000000,   /*TYPE_L2_SWITCH_TABLE*/
    (void *) RTL8651_ASICTABLE_BASE_OF_ALL_TABLES + 0x00010000,   /*TYPE_ARP_TABLE*/
    (void *) RTL8651_ASICTABLE_BASE_OF_ALL_TABLES + 0x00020000,   /*TYPE_L3_ROUTING_TABLE*/
    (void *) RTL8651_ASICTABLE_BASE_OF_ALL_TABLES + 0x00030000,   /*TYPE_MULTICAST_TABLE*/
    (void *) RTL8651_ASICTABLE_BASE_OF_ALL_TABLES + 0x00040000,   /*TYPE_PROTOCOL_TRAP_TABLE*/
    (void *) RTL8651_ASICTABLE_BASE_OF_ALL_TABLES + 0x00050000,   /*TYPE_VLAN_TABLE*/
    (void *) RTL8651_ASICTABLE_BASE_OF_ALL_TABLES + 0x00060000,   /*TYPE_EXT_INT_IP_TABLE*/
    (void *) RTL8651_ASICTABLE_BASE_OF_ALL_TABLES + 0x00070000,   /*TYPE_ALG_TABLE*/
    (void *) RTL8651_ASICTABLE_BASE_OF_ALL_TABLES + 0x00080000,   /*TYPE_SERVER_PORT_TABLE*/
    (void *) RTL8651_ASICTABLE_BASE_OF_ALL_TABLES + 0x00090000,   /*TYPE_L4_TCP_UDP_TABLE*/
    (void *) RTL8651_ASICTABLE_BASE_OF_ALL_TABLES + 0x000a0000,   /*TYPE_L4_ICMP_TABLE*/
    (void *) RTL8651_ASICTABLE_BASE_OF_ALL_TABLES + 0x000b0000,   /*TYPE_PPPOE_TABLE*/
    (void *) RTL8651_ASICTABLE_BASE_OF_ALL_TABLES + 0x000c0000    /*TYPE_ACL_TABLE*/
};
#define RTL8651_ASICTABLE_ENTRY_LENGTH          (8 * sizeof(uint32))

RTL_STATIC_INLINE void _rtl8651_asicTableAccessForward(uint32, uint32, void *);

static void _rtl8651_clearSpecifiedAsicTable(uint32 type, uint32 count);
static int32 _rtl8651_addAsicEntry(uint32 tableType, uint32 eidx, void *entryContent_P);
//static int32 _rtl8651_modifyAsicEntry(uint32 tableType, uint32 eidx, void *entryContent_P);
static int32 _rtl8651_forceAddAsicEntry(uint32 tableType, uint32 eidx, void *entryContent_P);
static int32 _rtl8651_readAsicEntry(uint32 tableType, uint32 eidx, void *entryContent_P);

static int32 _rtl8651_setPhyEntry(uint32 port, uint32 regId, uint32 value) {
	REG32(SWTAA) = PHY_BASE + (port<<5) + (regId<<2);
	REG32(TCR0) = value;
 	REG32(SWTACR) = ACTION_START | CMD_FORCE;//Activate add command
	while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done

	if ( (REG32(SWTASR) & TABSTS_MASK) == TABSTS_SUCCESS )//Check status
		return SUCCESS;
    return FAILED;
}

static int32 _rtl8651_addAsicEntry(uint32 tableType, uint32 eidx, void *entryContent_P) {
	_rtl8651_asicTableAccessForward(tableType, eidx, entryContent_P);
        
	REG32(SWTACR) = ACTION_START | CMD_ADD;//Activate add command

	while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done
    
	if ( (REG32(SWTASR) & TABSTS_MASK) != TABSTS_SUCCESS )//Check status
		return FAILED;
	return SUCCESS;
}

/*
static int32 _rtl8651_modifyAsicEntry(uint32 tableType, uint32 eidx, void *entryContent_P) {
	_rtl8651_asicTableAccessForward(tableType, eidx, entryContent_P);

	REG32(SWTACR) = ACTION_START | CMD_MODIFY;// Activate add command 

	while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done

	if ( (REG32(SWTASR) & TABSTS_MASK) != TABSTS_SUCCESS )//Check status
		return FAILED;
	else
		return SUCCESS;
}
*/

static int32 _rtl8651_forceAddAsicEntry(uint32 tableType, uint32 eidx, void *entryContent_P) {
	_rtl8651_asicTableAccessForward(tableType, eidx, entryContent_P);
        
 	REG32(SWTACR) = ACTION_START | CMD_FORCE;//Activate add command
    
	while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done
    
	if ( (REG32(SWTASR) & TABSTS_MASK) == TABSTS_SUCCESS )//Check status
		return SUCCESS;
	return FAILED;
}

static int32 _rtl8651_readAsicEntry(uint32 tableType, uint32 eidx, void *entryContent_P) {
	uint32 *    entryAddr;
    
	ASSERT_CSP(entryContent_P);
    
	entryAddr = (uint32 *) (
		(uint32) rtl8651_asicTableAccessAddrBase[tableType] + eidx * RTL8651_ASICTABLE_ENTRY_LENGTH);
    
	while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command ready
    
	/* Read registers according to entry width of each table */
	switch (tableType) {
		case TYPE_ACL_RULE_TABLE:
			*((uint32 *)entryContent_P + 7) = *(entryAddr + 7);
			*((uint32 *)entryContent_P + 6) = *(entryAddr + 6);
			*((uint32 *)entryContent_P + 5) = *(entryAddr + 5);
            		/* continue executing following lines */
		case TYPE_VLAN_TABLE:
			*((uint32 *)entryContent_P + 4) = *(entryAddr + 4);
			/* continue executing following lines */
		case TYPE_SERVER_PORT_TABLE:
			*((uint32 *)entryContent_P + 3) = *(entryAddr + 3);
			/* continue executing following lines */
		case TYPE_MULTICAST_TABLE:
		case TYPE_EXT_INT_IP_TABLE:
		case TYPE_L4_TCP_UDP_TABLE:
		case TYPE_L4_ICMP_TABLE:
			*((uint32 *)entryContent_P + 2) = *(entryAddr + 2);
			/* continue executing following lines */
		case TYPE_L2_SWITCH_TABLE:
		case TYPE_L3_ROUTING_TABLE:
			*((uint32 *)entryContent_P + 1) = *(entryAddr + 1);
			/* continue executing following lines */
		case TYPE_ARP_TABLE:
		case TYPE_PROTOCOL_TRAP_TABLE:
		case TYPE_ALG_TABLE:
		case TYPE_PPPOE_TABLE:
			*(uint32 *)entryContent_P = *entryAddr;
			break;
		default:
			ASSERT_CSP( 0 );
	}
	return 0;
}

RTL_STATIC_INLINE void _rtl8651_asicTableAccessForward(uint32 tableType, uint32 eidx, void *entryContent_P) {
	ASSERT_CSP(entryContent_P);

	while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done

	switch (tableType) {//Write registers according to entry width of each table
		case TYPE_ACL_RULE_TABLE:
			REG32(TCR7) = *((uint32 *)entryContent_P + 7);
			REG32(TCR6) = *((uint32 *)entryContent_P + 6);
			REG32(TCR5) = *((uint32 *)entryContent_P + 5);
			/* continue executing following lines */
		case TYPE_VLAN_TABLE:
			REG32(TCR4) = *((uint32 *)entryContent_P + 4);
			/* continue executing following lines */
 		case TYPE_SERVER_PORT_TABLE:
			REG32(TCR3) = *((uint32 *)entryContent_P + 3);
			/* continue executing following lines */
		case TYPE_MULTICAST_TABLE:
		case TYPE_EXT_INT_IP_TABLE:
		case TYPE_L4_TCP_UDP_TABLE:
		case TYPE_L4_ICMP_TABLE:
			REG32(TCR2) = *((uint32 *)entryContent_P + 2);
			/* continue executing following lines */
		case TYPE_L2_SWITCH_TABLE:
		case TYPE_L3_ROUTING_TABLE:
			REG32(TCR1) = *((uint32 *)entryContent_P + 1);
			/* continue executing following lines */
		case TYPE_ARP_TABLE:
		case TYPE_PROTOCOL_TRAP_TABLE:
		case TYPE_ALG_TABLE:
		case TYPE_PPPOE_TABLE:
			REG32(TCR0) = *(uint32 *)entryContent_P;
			break;
		default:
			ASSERT_CSP( 0 );
		}
	REG32(SWTAA) = (uint32) rtl8651_asicTableAccessAddrBase[tableType] + eidx * RTL8651_ASICTABLE_ENTRY_LENGTH;//Fill address
}

//This is a special function which is implemented based on board MAC address provision
void rtl8651_getMacAddress(ether_addr_t * macAddress, uint32 * number) {
	macaddr_t m;
	uint32 n;
	if (bdinfo_getMac(&m) || bdinfo_getMacNbr(&n) || (m.mac15_0 & 0x7) ||
		(m.mac47_32==0xffff && m.mac31_16==0xffff && m.mac15_0==0xffff) || n==0xff ) {
		macAddress->octet[0] = 0x00;
		macAddress->octet[1] = 0x03;
		macAddress->octet[2] = 0x47;
		macAddress->octet[3] = 0x74;
		macAddress->octet[4] = 0x4c;
		macAddress->octet[5] = 0x00;
		*number = 16;
		m.mac47_32 = 0x0003;
		m.mac31_16 = 0x4774;
		m.mac15_0 = 0x4c00;
		n = 16;
		bdinfo_setMac(&m);
	}		
	else {
		memcpy((void*)(macAddress->octet),(void*)&m,6);			
		*number = n;
	}
}

int32 rtl8651_setAsicOperationLayer(uint32 layer) {
	if(layer<1 || layer>4)
		return FAILED;
	
	if(layer == 1) {
		REG32(MSCR)&=~(EN_L2|EN_L3|EN_L4);
		REG32(MSCR)&=~(EN_IN_ACL);
		REG32(MSCR)&=~(EN_OUT_ACL);
	}else{
		REG32(MSCR)|=(EN_IN_ACL);
		REG32(MSCR)|=(EN_OUT_ACL);

		if(layer == 2) {
			REG32(MSCR)|=(EN_L2);
			REG32(MSCR)&=~(EN_L3|EN_L4);
		}
		else if(layer == 3) {
			REG32(MSCR)|=(EN_L2|EN_L3);
			REG32(MSCR)&=~(EN_L4);
		}
		else {
			REG32(MSCR)|=(EN_L2|EN_L3|EN_L4);
		}
	}
	if(layer == 1)
		rtl8651_setAsicAgingFunction(FALSE, FALSE);
	else if (layer == 2 || layer == 3)
		rtl8651_setAsicAgingFunction(TRUE, FALSE);
	else
		rtl8651_setAsicAgingFunction(TRUE, TRUE);
		
	return SUCCESS;
}

int32 rtl8651_getAsicOperationLayer(uint32 * layer) {
	uint32 regValue;
	
	if(layer == NULL)
		return FAILED;

	regValue = REG32(MSCR);
	switch(regValue & (EN_L2|EN_L3|EN_L4)) {
		case 0:
			*layer = 1;
		break;
		case EN_L2:
			*layer = 2;
		break;
		case (EN_L2|EN_L3):
			*layer = 3;
		break;
		case (EN_L2|EN_L3|EN_L4):
			*layer = 4;
		break;
		default:
			assert(0);//ASIC should not have such value
	}
	return SUCCESS;
}

int32 rtl8651_setAsicSpanningEnable(int8 spanningTreeEnabled) {
	if(spanningTreeEnabled == TRUE)
		REG32(MSCR)|=(EN_STP);
	else
		REG32(MSCR)&=~(EN_STP);
	return SUCCESS;
}

int32 rtl8651_getAsicSpanningEnable(int8 *spanningTreeEnabled) {
	if(spanningTreeEnabled == NULL)
		return FAILED;
	*spanningTreeEnabled = (REG32(MSCR)&(EN_STP)) == (EN_STP)? TRUE: FALSE;
	return SUCCESS;
}
static void _rtl8651_clearSpecifiedAsicTable(uint32 type, uint32 count) {
	rtl8651_tblAsic_aclTable_t entry;
	uint32 idx;
	
	bzero(&entry, sizeof(rtl8651_tblAsic_aclTable_t));
	for (idx=0; idx<count; idx++)// Write into hardware
		_rtl8651_forceAddAsicEntry(type, idx, &entry);

}

int32 rtl8651_clearAsicAllTable(void) {
	uint32 i;
    
	for(i=0; i<RTL8651_PORT_NUMBER; i++)
		rtl8651AsicEthernetTable[i].linkUp = 0;
	_rtl8651_clearSpecifiedAsicTable(TYPE_VLAN_TABLE, NUMBER_OF_VLAN_TABLE_ENTRY);
	_rtl8651_clearSpecifiedAsicTable(TYPE_L2_SWITCH_TABLE, NUMBER_OF_L2_SWITCH_TABLE_ENTRY);
	_rtl8651_clearSpecifiedAsicTable(TYPE_L3_ROUTING_TABLE, NUMBER_OF_L3_ROUTING_TABLE_ENTRY);
	_rtl8651_clearSpecifiedAsicTable(TYPE_ACL_RULE_TABLE, NUMBER_OF_ACL_TABLE_ENTRY);
	_rtl8651_clearSpecifiedAsicTable(TYPE_ALG_TABLE, NUMBER_OF_ALG_TABLE_ENTRY);
	_rtl8651_clearSpecifiedAsicTable(TYPE_ARP_TABLE, NUMBER_OF_ARP_TABLE_ENTRY);
	_rtl8651_clearSpecifiedAsicTable(TYPE_PROTOCOL_TRAP_TABLE, NUMBER_OF_PROTOCOL_TRAP_TABLE_ENTRY);
	_rtl8651_clearSpecifiedAsicTable(TYPE_PPPOE_TABLE, NUMBER_OF_PPPOE_TABLE_ENTRY);	
	_rtl8651_clearSpecifiedAsicTable(TYPE_MULTICAST_TABLE, NUMBER_OF_MULTICAST_TABLE_ENTRY);
	_rtl8651_clearSpecifiedAsicTable(TYPE_SERVER_PORT_TABLE, NUMBER_OF_SERVER_PORT_TABLE_ENTRY);
	_rtl8651_clearSpecifiedAsicTable(TYPE_EXT_INT_IP_TABLE, NUMBER_OF_EXT_INTERNAL_IP_TABLE_ENTRY);
	_rtl8651_clearSpecifiedAsicTable(TYPE_L4_TCP_UDP_TABLE, NUMBER_OF_L4_TCP_UDP_TABLE_ENTRY);
	_rtl8651_clearSpecifiedAsicTable(TYPE_L4_ICMP_TABLE, NUMBER_OF_L4_ICMP_TABLE_ENTRY);
	_rtl8651_clearSpecifiedAsicTable(TYPE_MULTICAST_TABLE, NUMBER_OF_MULTICAST_TABLE_ENTRY);
	
	return SUCCESS;
}

static void _rtl8651_initialRead(void) {//RTL8651 read counter for the first time will get value -1 and this is meaningless
	uint32 i;
	for(i=0; i<=0x6c; i+=0x4)
		rtl8651_returnAsicCounter(i);
}


int32 rtl8651_initAsic(void) {
	/* Reset NIC */
	swNic_resetDescriptors();
	/* Perform full-reset for sw-core */ 
	REG32(MACCR) |=  FULL_RST;
	/* Check BIST until all packet buffers are reset */
	while (REG32(BISTCR) != BIST_READY_PATTERN) // 16.2 The bist control register
		;//tick_Delay10ms(1);
	REG32(BISTCR) |= 2;	// 16.2 The bist control register,  BISTCR.TRXRDY Set 1 to start normal Tx and Rx

	rtl8651_setAsicOperationLayer(1);//Disable layer2, layer3 and layer4 function
	rtl8651_clearAsicAllTable();
	rtl8651_setAsicSpanningEnable(FALSE);
	
	rtl8651_testAsicDrv();//Can do basic test and 

	rtl8651_setAsicOperationLayer(1);//Disable layer2, layer3 and layer4 function
	rtl8651_clearAsicAllTable();
	rtl8651_setAsicSpanningEnable(FALSE);

	rtl8651_clearAsicCounter();
	REG32(0xbd012064) = 0xf0000000;//Enable Lexra bus timeout interrupt
	//MAC Control (0xBC803000)
	REG32(MACCR)&=~DIS_IPG;//Set IFG range as 96+-4bit time
	REG32(MACCR)&=~EN_INT_CAM;//Disable Internal CAM
	REG32(MACCR)|=NORMAL_BACKOFF;//Normal backoff
	REG32(MACCR)&=~BACKOFF_EXPONENTIAL_3_9;//Exponential parameter is 9
	REG32(MACCR)&=~ACPT_MAXLEN_1552_1536;//Max length 1536
	REG32(MACCR) |= (EN_PHY_P4 | EN_PHY_P3 | EN_PHY_P2 | EN_PHY_P1 | EN_PHY_P0);//Enable PHY
	{//RTL8651 patch for version before 2003/2/28
		uint32  val;
		val = REG32(PORT0_PHY_CONTROL);
		val = REG32(PORT0_PHY_STATUS);
		val = REG32(PORT0_PHY_IDENTIFIER_1);
		val = REG32(PORT0_PHY_IDENTIFIER_2);
		val = REG32(PORT0_PHY_AUTONEGO_ADVERTISEMENT);
		val = REG32(PORT0_PHY_AUTONEGO_LINK_PARTNER_ABILITY);
		val = REG32(PORT1_PHY_CONTROL);
		val = REG32(PORT1_PHY_STATUS);
		val = REG32(PORT1_PHY_IDENTIFIER_1);
		val = REG32(PORT1_PHY_IDENTIFIER_2);
		val = REG32(PORT1_PHY_AUTONEGO_ADVERTISEMENT);
		val = REG32(PORT1_PHY_AUTONEGO_LINK_PARTNER_ABILITY);
		val = REG32(PORT2_PHY_CONTROL);
		val = REG32(PORT2_PHY_STATUS);
		val = REG32(PORT2_PHY_IDENTIFIER_1);
		val = REG32(PORT2_PHY_IDENTIFIER_2);
		val = REG32(PORT2_PHY_AUTONEGO_ADVERTISEMENT);
		val = REG32(PORT2_PHY_AUTONEGO_LINK_PARTNER_ABILITY);
		val = REG32(PORT3_PHY_CONTROL);
		val = REG32(PORT3_PHY_STATUS);
		val = REG32(PORT3_PHY_IDENTIFIER_1);
		val = REG32(PORT3_PHY_IDENTIFIER_2);
		val = REG32(PORT3_PHY_AUTONEGO_ADVERTISEMENT);
		val = REG32(PORT3_PHY_AUTONEGO_LINK_PARTNER_ABILITY);
		val = REG32(PORT4_PHY_CONTROL);
		val = REG32(PORT4_PHY_STATUS);
		val = REG32(PORT4_PHY_IDENTIFIER_1);
		val = REG32(PORT4_PHY_IDENTIFIER_2);
		val = REG32(PORT4_PHY_AUTONEGO_ADVERTISEMENT);
	}
	REG32(MACMR) = 0xc0000001;	//Test mode enable
	REG32(MACCR) &= ~(EN_FX_P4 | EN_FX_P3 | EN_FX_P2 | EN_FX_P1 | EN_FX_P0);//Disable FX mode (UTP mode)
	REG32(VLANTCR)&=~VLAN_TAG_ONLY;
	REG32(MISCCR)|=L4_HASH;
	REG32(MISCCR)&=~FRAG2CPU;//Only when multilayer enabled, IP fragment packet does not need to send to CPU
	REG32(MISCCR)&= ~MULTICAST_L2_MTU_MASK;
	REG32(MISCCR)|= (1522&MULTICAST_L2_MTU_MASK);//Multicast packet layer2 size 1522 at most
	REG32(SWTMCR)&=~NAPTR_NOT_FOUND_DROP;//When reverse NAPT entry not found, CPU process it.
	rtl8651_setAsicNaptAutoAddDelete(TRUE, TRUE);
	REG32(SWTMCR)|=EN_VLAN_INGRESS_FILTER;
	REG32(SWTMCR)&=~WAN_ROUTE_MASK;//Set WAN route toEnable (Allow traffic from WAN port to WAN port)
	REG32(SWTMCR)|=NAPTF2CPU;//When packet destination to switch. Just send to CPU
	REG32(SWTMCR)|=EN_MCAST; //Enable multicast table
	REG32(SWTMCR)&=~BRIDGE_PKT_TO_CPU;//Current no bridge protocol is supported
	REG32(SWTMCR)&=~BCAST_TO_CPU;//Using protocol trapping only
	REG32(SWTMCR)&=~MCAST_TO_CPU;//Using IP multicast table
	REG32(SWTMCR)|=EN_BCAST;//Disable special broadcast handling
	REG32(SWTMCR)|=(uint32)EN_PPPOE;
	REG32(CSCR) &= ~ALLOW_L2_CHKSUM_ERR;
	REG32(CSCR) &= ~ALLOW_L3_CHKSUM_ERR;
	REG32(CSCR) &= ~ALLOW_L4_CHKSUM_ERR;
	REG32(CSCR) |= EN_ETHER_L3_CHKSUM_REC; //Enable L3 checksum Re-calculation
	REG32(CSCR) |= EN_ETHER_L4_CHKSUM_REC; //Enable L4 checksum Re-calculation
//	REG32(FCRTH)=0x180c180c;	/* Flow-control */
	REG32(FCRTH)=0x20142014;	/* Flow-control */
	REG32(FCPTR)=0x006c006c;	/* Flow-control prime threshold register*/
	//REG32(FCREN)=0xfffc05f0;	/* Enable flow-control */
	REG32(FCREN)=0x810005f0;	/* Enable flow-control */

	//Protocol trap
	REG32(PTRAPCR)|=EN_ARP_TRAP;
	REG32(PTRAPCR)&=~EN_RARP_TRAP;
	REG32(PTRAPCR)|=EN_PPPOE_TRAP;
	REG32(PTRAPCR)&=~EN_IGMP_TRAP;
	REG32(PTRAPCR)&=~EN_DHCP_TRAP1;
	REG32(PTRAPCR)&=~EN_DHCP_TRAP2;
	REG32(PTRAPCR)&=~EN_OSPF_TRAP;
	REG32(PTRAPCR)&=~EN_RIP_TRAP;
	//Enable TTL-1
	REG32(TTLCR) |= (uint32)DIS_TTL1;
    
	_rtl8651_setPhyEntry(4, 0, 0x00000100);//Patch??
	//Enable link up/down interrupt
	linkChangePendingCount = 0;
	REG32(CPUIIMR) |= (uint32)LINK_CHANG_IE;//Enable link change interrupt
	REG32(CPUIIMR) &= ~L4_COL_REMOVAL_IE;//Disable L4 collision removal interrupt
	//Read all counter value once to get rid of meaningless counter values returned
	_rtl8651_initialRead();
	return SUCCESS;
}

int32 rtl8651_updateLinkStatus(void) {
	uint32 i, patchLoop, patchReg;

	if(linkChangePendingCount>0) {
		for(patchLoop=0; patchLoop<100; patchLoop++)
			for(i=0; i<5; i++) //rtl8651 only have 5 phy
				patchReg = REG32(PHY_BASE + (i<<5) + 4);
			
			if(REG32(PHY_BASE+(i<<5) + 0x4) & 0x4) {
				rtl8651_setAsicEthernetLinkStatus(i, TRUE);
				if((REG32(PHY_BASE + (i<<5)) & 0x1000) && (REG32(PHY_BASE+(i<<5) + 0x14) & 0x400)) //Auto-negotiation enabled and link partner can enable flow control
					REG32(FCREN) |= (0x81<<(18+i));
				else
					REG32(FCREN) &= ~(0x81<<(18+i));
			}
			else {
				rtl8651_setAsicEthernetLinkStatus(i, FALSE);
				REG32(FCREN) &= ~(0x81<<(18+i));
			}
		linkChangePendingCount = 0;
		return SUCCESS;
	}
	return FAILED;//No link change
}

int32 rtl8651_setAsicEthernetPHY(uint32 port, int8 autoNegotiation, uint32 advCapability, uint32 speed, int8 fullDuplex) {
	uint32 ctrlReg;
	
	if(port >= RTL8651_PORT_NUMBER)
		return FAILED;

	REG32(SWTAA) = PHY_BASE + (port<<5) + 16;
	if(advCapability == RTL8651_ETHER_AUTO_100FULL)
		REG32(TCR0) = CAPABLE_PAUSE | CAPABLE_100BASE_TX_FD | CAPABLE_100BASE_TX_HD| CAPABLE_10BASE_TX_FD | CAPABLE_10BASE_TX_HD | 0x1;
	else if(advCapability == RTL8651_ETHER_AUTO_100HALF)
		REG32(TCR0) = CAPABLE_PAUSE | CAPABLE_100BASE_TX_HD| CAPABLE_10BASE_TX_FD | CAPABLE_10BASE_TX_HD | 0x1;
	else	if(advCapability == RTL8651_ETHER_AUTO_10FULL)
		REG32(TCR0) = CAPABLE_PAUSE | CAPABLE_10BASE_TX_FD | CAPABLE_10BASE_TX_HD | 0x1;
	else if(advCapability == RTL8651_ETHER_AUTO_10HALF)
		REG32(TCR0) = CAPABLE_PAUSE | CAPABLE_10BASE_TX_HD | 0x1;
	else
		return FAILED;//Parameter of RTL8651_ETHER_AUTO_ not matched
 	REG32(SWTACR) = ACTION_START | CMD_FORCE;//Activate add command
	while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done

	if(autoNegotiation == TRUE) 
		ctrlReg = ENABLE_AUTONEGO;//Henry suggestion not to restart it....So, remark this following parts | RESTART_AUTONEGO;
	else
		ctrlReg = 0;
	if(speed == 1) // 100Mbps, default assume 10Mbps
		ctrlReg |= SPEED_SELECT_100M;
	if(fullDuplex == TRUE)
		ctrlReg |= SELECT_FULL_DUPLEX;
	REG32(SWTAA) = PHY_BASE + (port<<5);
	REG32(TCR0) = ctrlReg;
 	REG32(SWTACR) = ACTION_START | CMD_FORCE;//Activate add command
	while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done

	if ( (REG32(SWTASR) & TABSTS_MASK) == TABSTS_SUCCESS )//Check status
		return SUCCESS;

	return SUCCESS;
}

int32 rtl8651_getAsicEthernetPHY(uint32 port, int8 *autoNegotiation, uint32 *advCapability, uint32 *speed, int8 *fullDuplex) {
	if(port >= RTL8651_PORT_NUMBER || autoNegotiation == NULL || advCapability == NULL || speed == NULL || fullDuplex == NULL)
		return FAILED;
	*autoNegotiation = (REG32(PHY_BASE + (port<<5)) & ENABLE_AUTONEGO)? TRUE: FALSE;
	if(REG32(TCR0) & CAPABLE_100BASE_TX_FD)
		*advCapability = RTL8651_ETHER_AUTO_100FULL;
	else if(REG32(TCR0) & CAPABLE_100BASE_TX_HD)
		*advCapability = RTL8651_ETHER_AUTO_100HALF;
	else if(REG32(TCR0) & CAPABLE_10BASE_TX_FD)
		*advCapability = RTL8651_ETHER_AUTO_10FULL;
	else if(REG32(TCR0) & CAPABLE_10BASE_TX_HD)
		*advCapability = RTL8651_ETHER_AUTO_10HALF;
	*speed = (REG32(PHY_BASE + (port<<5)) & SPEED_SELECT_100M)? 1: 0;
	*fullDuplex = (REG32(PHY_BASE + (port<<5)) & SELECT_FULL_DUPLEX)? TRUE: FALSE;
	
	return SUCCESS;
}

int32 rtl8651_setAsicEthernetBandwidthControl(uint32 port, int8 input, uint32 rate) {//RTL8651_ASICBC_xxx
	if(port >= RTL8651_PORT_NUMBER || rate > BW_8M)
		return FAILED;
	if(port<4) {
		if(input == TRUE)
			REG32(BCR1) = (REG32(BCR1) & ~(0xf0 << (port<<3)) ) | (rate<< (port<<3)+4);
		else
			REG32(BCR1) = (REG32(BCR1) & ~(0xf << (port<<3)) ) | (rate<< (port<<3));
	}
	else {
		if(input == TRUE)
			REG32(BCR0) = (REG32(BCR0) & ~(0xf0 << (port-4<<3)) ) | (rate<< (port-4<<3)+4);
		else
			REG32(BCR0) = (REG32(BCR0) & ~(0xf << (port-4<<3)) ) | (rate<< (port-4<<3));
	}
	return SUCCESS;
}

int32 rtl8651_getAsicEthernetBandwidthControl(uint32 port, int8 input, uint32 *rate) {//RTL8651_ASICBC_xxx
	if(port >= RTL8651_PORT_NUMBER || rate == NULL)
		return FAILED;

	if(port<4) {
		if(input == TRUE)
			*rate = (REG32(BCR1) >> (port<<3)+4) & 0xf;
		else
			*rate = (REG32(BCR1) >> (port<<3)) & 0xf;
	}
	else {
		if(input == TRUE)
			*rate = (REG32(BCR0) >> (port-4<<3)+4) & 0xf;
		else
			*rate = (REG32(BCR0) >> (port-4<<3)) & 0xf;
	}
	return SUCCESS;
}

//portState reference to RTL8651_PORTSTA_xxx
int32 rtl8651_setAsicMulticastSpanningTreePortState(uint32 port, uint32 portState) {
	if(port >= RTL8651_PORT_NUMBER || portState > RTL8651_PORTSTA_FORWARDING)
		return FAILED;
	REG32(SWTMCR) &= ~(0x3 << (7+(port<<1)));
	switch(portState) {
		case RTL8651_PORTSTA_DISABLED://0
			break;
		case RTL8651_PORTSTA_BLOCKING:
		case RTL8651_PORTSTA_LISTENING:
			REG32(SWTMCR) |= (0x1 << (7+(port<<1)));
			break;
		case RTL8651_PORTSTA_LEARNING:
			REG32(SWTMCR) |= (0x2 << (7+(port<<1)));
			break;
		case RTL8651_PORTSTA_FORWARDING:
			REG32(SWTMCR) |= (0x3 << (7+(port<<1)));
			break;
		default:
			assert(0);
	}
	return SUCCESS;
}

//portState reference to RTL8651_PORTSTA_xxx
int32 rtl8651_getAsicMulticastSpanningTreePortState(uint32 port, uint32 *portState) {
	if(port >= RTL8651_PORT_NUMBER || portState == NULL)
		return FAILED;

	switch((REG32(SWTMCR) >> (7+(port<<1)) )&0x3) {
		case 0:
			*portState = RTL8651_PORTSTA_DISABLED;
			break;
		case 1:
			*portState = RTL8651_PORTSTA_BLOCKING;
			break;
		case 2:
			*portState = RTL8651_PORTSTA_LEARNING;
			break;
		case 3:
			*portState = RTL8651_PORTSTA_FORWARDING;
			break;
		default:
			assert(0);
	}
	return SUCCESS;
}

int32 rtl8651_setAsicMulticastPortInternal(uint32 port, int8 isInternal) {
	if(port >= RTL8651_PORT_NUMBER)
		return FAILED;
	REG32(SWTMCR) &= ~(0x1 << (19+port));
	if(isInternal == TRUE)
		REG32(SWTMCR) |= (0x1 << (19+port));
	return SUCCESS;
}

int32 rtl8651_getAsicMulticastPortInternal(uint32 port, int8 *isInternal) {
	if(port >= RTL8651_PORT_NUMBER || isInternal == NULL)
		return FAILED;
	if((REG32(SWTMCR)>>(19+port)) & 0x1)
		*isInternal = TRUE;
	else
		*isInternal = FALSE;
	return SUCCESS;
}

int32 rtl8651_setAsicL2Table(uint32 row, uint32 column, ether_addr_t * mac, int8 cpu, int8 srcBlk, uint32 mbr, uint32 ageSec, int8 isStatic, int8 nhFlag) {
	rtl8651_tblAsic_l2Table_t entry;
	
	if(row >= RTL8651_L2TBL_ROW || column >= RTL8651_L2TBL_COLUMN)
		return FAILED;
	if(mac->octet[5] != ((row ^ mac->octet[0] ^ mac->octet[1] ^ mac->octet[2] ^ mac->octet[3] ^ mac->octet[4]) & 0xff))
		return FAILED;

	entry.mac47_40 = mac->octet[0];
	entry.mac39_24 = (mac->octet[1] << 8) | mac->octet[2];
	entry.mac23_8 = (mac->octet[3] << 8) | mac->octet[4];
#ifdef RTL8651B
	if(mbr > RTL8651_ALLPORTMASK){ //this MAC is on extension port
		entry.extMemberPortMask = (mbr >>RTL8651_PORT_NUMBER);
		mbr &= RTL8651_ALLPORTMASK;
	}
#endif
	entry.memberPort = mbr;
	entry.hPriority = 0;
	entry.toCPU = cpu==TRUE? 1: 0;
	entry.isStatic = (isStatic==TRUE? 1: 0);
	entry.nxtHostFlag = (nhFlag==TRUE? 1: 0);
	entry.agingTime = (ageSec>=300)? 3 : (ageSec<300 && ageSec>=200)? 2: (ageSec<200 && ageSec>=100)? 1: 0;
	entry.srcBlock = srcBlk==TRUE? 1: 0;
    
	_rtl8651_forceAddAsicEntry(TYPE_L2_SWITCH_TABLE, row<<2 | column, &entry);
	return SUCCESS;
}

int32 rtl8651_delAsicL2Table(uint32 row, uint32 column) {
	rtl8651_tblAsic_l2Table_t entry;

	if(row >= RTL8651_L2TBL_ROW || column >= RTL8651_L2TBL_COLUMN)
		return FAILED;

	entry.isStatic = 0;
	entry.agingTime = 0;
	entry.nxtHostFlag = 0;
    
	_rtl8651_forceAddAsicEntry(TYPE_L2_SWITCH_TABLE, row<<2 | column, &entry);
	return SUCCESS;
}

int32 rtl8651_getAsicL2Table(uint32 row, uint32 column, ether_addr_t * mac, int8 * cpu, int8 * srcBlk, int8 * isStatic, uint32 * mbr, uint32 * ageSec, int8 *nhFlag) {
	rtl8651_tblAsic_l2Table_t   entry;

	if(row >= RTL8651_L2TBL_ROW || column >= RTL8651_L2TBL_COLUMN)
		return FAILED;

	_rtl8651_readAsicEntry(TYPE_L2_SWITCH_TABLE, row<<2 | column, &entry);
	if(entry.srcBlock == 0 && entry.nxtHostFlag == 0 && entry.agingTime == 0)
		return FAILED;
	if (mac != NULL) {
		mac->octet[0] = entry.mac47_40;
		mac->octet[1] = entry.mac39_24 >> 8;
		mac->octet[2] = entry.mac39_24 & 0xff;
		mac->octet[3] = entry.mac23_8 >> 8;
		mac->octet[4] = entry.mac23_8 & 0xff;
		mac->octet[5] = row ^ mac->octet[0] ^ mac->octet[1] ^ mac->octet[2] ^ mac->octet[3] ^ mac->octet[4];
	}		
	if (cpu != NULL) *cpu = entry.toCPU==1? TRUE: FALSE;
	if (srcBlk != NULL) *srcBlk = entry.srcBlock==1? TRUE: FALSE;
	if (nhFlag != NULL) *nhFlag = entry.nxtHostFlag==1? TRUE: FALSE;
	if (isStatic != NULL) *isStatic = entry.isStatic==1? TRUE: FALSE;
	if (mbr != NULL){
#ifdef RTL8651B		
		*mbr = (entry.extMemberPortMask<<RTL8651_PORT_NUMBER) | entry.memberPort;
#else
		*mbr = entry.memberPort;
#endif
	}
	if (ageSec != NULL) *ageSec = entry.agingTime * 100;//Maximum to be 5 min
	return SUCCESS;
}

void rtl8651_updateAsicLinkAggregatorLMPR(void) {
	uint32 i, idx;
	int8 candidate[RTL8651_PORT_NUMBER], trueConfig;
	uint32 trunk1PortMask, lmpr;

	for(i=0; i<RTL8651_PORT_NUMBER; i++)
		candidate[i] = FALSE;
	trunk1PortMask = REG32(PTCR)&0x3f;
	for(i=0, trueConfig = FALSE; i<RTL8651_PORT_NUMBER; i++) {
		if((trunk1PortMask & (1<<i)) && rtl8651AsicEthernetTable[i].linkUp == 1) {
			candidate[i] = TRUE;
			trueConfig = TRUE;
		}
		else
			candidate[i] = FALSE;
	}

	lmpr = 0;
	if(trueConfig == TRUE) 
		for(i=0, idx = 0; i<8; i++) {
			while(candidate[idx] == FALSE)
				idx = (idx+1)%RTL8651_PORT_NUMBER;
			lmpr |= idx<<i*3;
			idx = (idx+1)%RTL8651_PORT_NUMBER;
		}
	REG32(PTCR) = (lmpr << 6) | trunk1PortMask;
}

int32 rtl8651_setAsicLinkAggregator(uint32 portMask) {
	if(portMask >= 1<<RTL8651_PORT_NUMBER)
		return FAILED;

	REG32(PTCR) = portMask;
	rtl8651_updateAsicLinkAggregatorLMPR();
	return SUCCESS;
}

int32 rtl8651_getAsicLinkAggregator(uint32 * portMask, uint32 *mapping) {

	if(portMask == NULL || mapping == NULL)
		return FAILED;
	
	*mapping = REG32(PTCR)>>6;
	*portMask = REG32(PTCR) & 0x3f;

	return SUCCESS;
}

int32 rtl8651_setAsicPvid(uint32 port, uint32 pvidx) {
	uint32 regValue;
	rtl8651_tblAsic_vlanTable_t entry;
	
	if(port>=RTL8651_PORT_NUMBER || pvidx>=RTL8651_VLAN_NUMBER)
		return FAILED;;
	_rtl8651_readAsicEntry(TYPE_VLAN_TABLE, pvidx, &entry);
	if(entry.valid == 0)
		return FAILED;

	regValue = REG32(PVCR);
	regValue = (regValue & ~(0x7<<port*3)) | (pvidx<<port*3);
	REG32(PVCR) = regValue;
	return SUCCESS;
}

int32 rtl8651_getAsicPvid(uint32 port, uint32 *pvidx) {

	if(port>=RTL8651_PORT_NUMBER || pvidx == NULL)
		return FAILED;
	*pvidx = (REG32(PVCR) >> port*3)&0x7;
	return SUCCESS;
}

int32 rtl8651_setAsicVlan(uint16 vid, ether_addr_t * mac, uint32 mbr, 
	uint32 inAclStart, uint32 inAclEnd, uint32 outAclStart, uint32 outAclEnd,
	int8 internalIntf, int8 enableRoute, int8 *portState, int8 broadcastToCpu,
	int8 promiscuous, uint32 untagPortMask, uint32 macNumber, uint32 mtu) {
	uint32 i, vidx = rtl8651_vlanTableIndex(vid);
	rtl8651_tblAsic_vlanTable_t entry;

	if(portState == NULL)
		return FAILED;
	
	entry.vhid = vid>>3;
	entry.mac47_32 = (mac->octet[0]<<8) | mac->octet[1];
	entry.mac31_16 = (mac->octet[2]<<8) | mac->octet[3];
	entry.mac15_0 = (mac->octet[4]<<8) | mac->octet[5];

#ifdef RTL8651B
	if(mbr > RTL8651_ALLPORTMASK ){
		entry.extMemberPort=mbr>>RTL8651_PORT_NUMBER;
		entry.memberPort &= RTL8651_ALLPORTMASK;
	}
	if(untagPortMask > RTL8651_ALLPORTMASK ){
		entry.extEgressUntag=mbr>>RTL8651_PORT_NUMBER;
		entry.egressUntag &= RTL8651_ALLPORTMASK;
	}	
#endif

	entry.memberPort = mbr;
	entry.inACLStart = inAclStart;
	entry.inACLEnd = inAclEnd;
	entry.outACLStart = outAclStart;
	entry.outACLEnd = outAclEnd;
	entry.isInternal = internalIntf==TRUE? 1: 0;
	entry.enHWRoute = enableRoute==TRUE? 1: 0;
	entry.STPStatus = 0;
	for(i=0; i<RTL8651_PORT_NUMBER; i++) {
		switch(portState[i]) {
			case RTL8651_PORTSTA_DISABLED:
				break;
			case RTL8651_PORTSTA_BLOCKING:
			case RTL8651_PORTSTA_LISTENING:
				entry.STPStatus |= (0x1<<(i*2));
				break;
			case RTL8651_PORTSTA_LEARNING:
				entry.STPStatus |= (0x2<<(i*2));
				break;
			case RTL8651_PORTSTA_FORWARDING:
				entry.STPStatus |= (0x3<<(i*2));
				break;
		}
	}
#ifdef RTL8651B
	for(i=0; i<RTL8651_EXTPORT_NUMBER; i++) {
		switch(portState[RTL8651_PORT_NUMBER+i]) {
			case RTL8651_PORTSTA_DISABLED:
				break;
			case RTL8651_PORTSTA_BLOCKING:
			case RTL8651_PORTSTA_LISTENING:
				entry.extSTPStatus |= (0x1<<(i*2));
				break;
			case RTL8651_PORTSTA_LEARNING:
				entry.extSTPStatus |= (0x2<<(i*2));
				break;
			case RTL8651_PORTSTA_FORWARDING:
				entry.extSTPStatus |= (0x3<<(i*2));
				break;
		}
	}
#endif

	entry.bcastToCPU = broadcastToCpu==TRUE? 1: 0;
	entry.promiscuous = promiscuous == TRUE? 1: 0;
	entry.egressUntag = untagPortMask;
	switch(macNumber) {
		case 0:
		case 1:
			entry.macMask = 0;
		break;
		case 2:
			entry.macMask = 1;
		break;
		case 4:
			entry.macMask = 2;
		break;
		case 8:
			entry.macMask = 3;
		break;
		default:
			return FAILED;//Not permitted macNumber value
	}
	entry.mtuH = mtu >> 8;
	entry.mtuL = mtu & 0xff;
	entry.valid = 1;
	_rtl8651_forceAddAsicEntry(TYPE_VLAN_TABLE, vidx, &entry);
	return SUCCESS;
}

int32 rtl8651_delAsicVlan(uint16 vid) {
	uint32 vidx = rtl8651_vlanTableIndex(vid);
	rtl8651_tblAsic_vlanTable_t entry;

	entry.valid = 0;
	_rtl8651_forceAddAsicEntry(TYPE_VLAN_TABLE, vidx, &entry);
	return SUCCESS;
}

int32 rtl8651_getAsicVlan(uint16 vid, ether_addr_t * mac, uint32 * mbr, 
	uint32 * inAclStart, uint32 * inAclEnd, uint32 * outAclStart, uint32 * outAclEnd,
	int8 * internalIntf, int8 * enableRoute, int8 *portState, int8 * broadcastToCpu,
	int8 * promiscuous, uint32 * untagPortMask, uint32 * macNumber, uint32 * mtu) {
	uint32 i, vidx = rtl8651_vlanTableIndex(vid);
	rtl8651_tblAsic_vlanTable_t entry;

	if(mac == NULL || mbr == NULL || inAclStart == NULL || inAclEnd == NULL || outAclStart == NULL || 
		outAclEnd == NULL || internalIntf == NULL || enableRoute == NULL || portState == NULL || mtu == NULL ||
		broadcastToCpu == NULL || promiscuous == NULL || untagPortMask == NULL || macNumber == NULL)
		return FAILED;
		
	_rtl8651_readAsicEntry(TYPE_VLAN_TABLE, vidx, &entry);
	if(entry.valid == 0 | entry.vhid != (vid>>3))
		return FAILED;
	mac->octet[0] = entry.mac47_32 >> 8;
	mac->octet[1] = entry.mac47_32 & 0xff;
	mac->octet[2] = entry.mac31_16 >> 8;
	mac->octet[3] = entry.mac31_16 & 0xff;
	mac->octet[4] = entry.mac15_0 >> 8;
	mac->octet[5] = entry.mac15_0 & 0xff;
#ifdef RTL8651B
	*mbr = (entry.extMemberPort<<RTL8651_PORT_NUMBER) | entry.memberPort;
	*untagPortMask = (entry.extEgressUntag<<RTL8651_PORT_NUMBER) |entry.egressUntag;
#else
	*mbr = entry.memberPort;
	*untagPortMask = entry.egressUntag;
#endif
	*inAclStart = entry.inACLStart;
	*inAclEnd = entry.inACLEnd;
	*outAclStart = entry.outACLStart;
	*outAclEnd = entry.outACLEnd;
	*internalIntf = entry.isInternal==1? TRUE: FALSE;
	*enableRoute = entry.enHWRoute==1? TRUE: FALSE;
	for(i=0; i<RTL8651_PORT_NUMBER; i++) {
		switch((entry.STPStatus>>(i*2))&0x3) {
			case 0x0:
				portState[i] = RTL8651_PORTSTA_DISABLED;
				break;
			case 0x1:
				portState[i] = RTL8651_PORTSTA_BLOCKING;
				break;
			case 0x2:
				portState[i] = RTL8651_PORTSTA_LEARNING;
				break;
			case 0x3:
				portState[i] = RTL8651_PORTSTA_FORWARDING;
				break;
		}
	}
#ifdef RTL8651B
	for(i=0; i<RTL8651_EXTPORT_NUMBER; i++) {
		switch((entry.extSTPStatus>>(i*2))&0x3) {
			uint32 portNum=RTL8651_PORT_NUMBER+i;
			case 0x0:
				portState[portNum] = RTL8651_PORTSTA_DISABLED;
				break;
			case 0x1:
				portState[portNum] = RTL8651_PORTSTA_BLOCKING;
				break;
			case 0x2:
				portState[portNum] = RTL8651_PORTSTA_LEARNING;
				break;
			case 0x3:
				portState[portNum] = RTL8651_PORTSTA_FORWARDING;
				break;
		}
	}
#endif
	
	*broadcastToCpu = entry.bcastToCPU==1? TRUE: FALSE;
	*promiscuous = entry.promiscuous==1? TRUE: FALSE;
	*macNumber = (1<<entry.macMask);
	*mtu = (entry.mtuH<<8) | entry.mtuL;
	return SUCCESS;
}

int32 rtl8651_setAsicPppoe(uint32 index, uint16 sessionId) {
	rtl8651_tblAsic_pppoeTable_t entry;
	
 	if(index >= RTL8651_PPPOETBL_SIZE || sessionId == 0xffff)
		return FAILED;

	entry.sessionID = sessionId;
	
	_rtl8651_forceAddAsicEntry(TYPE_PPPOE_TABLE, index, &entry);
	return SUCCESS;
}

int32 rtl8651_getAsicPppoe(uint32 index, uint16 *sessionId) {
	rtl8651_tblAsic_pppoeTable_t entry;

	if(sessionId == NULL)
		return FAILED;
	_rtl8651_readAsicEntry(TYPE_PPPOE_TABLE, index, &entry);
	*sessionId = entry.sessionID;
	return SUCCESS;
}

//Allow overwirte existed entry
int32 rtl8651_setAsicRouting(uint32 index, ipaddr_t ipAddr, ipaddr_t ipMask, uint32 process, //0: pppoe, 1:direct, 2:indirect, 4:Strong CPU, 
							uint32 vidx, uint32 arpStart, uint32 arpEnd, 
							uint32 nextHopRow, uint32 nextHopColumn, uint32 pppoeIdx) {
	uint32 i, asicMask;
	rtl8651_tblAsic_l3RouteTable_t entry;
	
	if(index >= RTL8651_ROUTINGTBL_SIZE)
		return FAILED;

	if(ipMask) {
		for(i=0; i<32; i++)
			if(ipMask & (1<<i)) break;
		asicMask = 31 - i;
	}
	else
		asicMask = 0;//Meaningless
	entry.IPAddr = ipAddr;
	switch(process) {
		case 0://PPPoE
			entry.linkTo.PPPoEEntry.PPPoEIndex = pppoeIdx;
			entry.linkTo.PPPoEEntry.nextHop = (nextHopRow <<2) | nextHopColumn;
			entry.linkTo.PPPoEEntry.IPMask = asicMask;
			entry.linkTo.PPPoEEntry.vid = vidx;
			entry.linkTo.PPPoEEntry.hPriority = 0;
			entry.linkTo.PPPoEEntry.process = process;
			entry.linkTo.PPPoEEntry.valid = 1;
		break;
		case 1://Direct
			entry.linkTo.L2Entry.nextHop = (nextHopRow <<2) | nextHopColumn;
			entry.linkTo.L2Entry.IPMask = asicMask;
			entry.linkTo.L2Entry.vid = vidx;
			entry.linkTo.L2Entry.hPriority = 0;
			entry.linkTo.L2Entry.process = process;
			entry.linkTo.L2Entry.valid = 1;
		break;
		case 2://Indirect
			entry.linkTo.ARPEntry.ARPEnd = arpEnd;
			entry.linkTo.ARPEntry.ARPStart = arpStart;
			entry.linkTo.ARPEntry.IPMask = asicMask;
			entry.linkTo.ARPEntry.vid = vidx;
			entry.linkTo.ARPEntry.hPriority = 0;
			entry.linkTo.ARPEntry.process = process;
			entry.linkTo.ARPEntry.valid = 1;
		break;
		case 4:
			entry.linkTo.ARPEntry.IPMask = asicMask;
			entry.linkTo.ARPEntry.process = process;
			entry.linkTo.ARPEntry.valid = 1;
		break;
	}
	_rtl8651_forceAddAsicEntry(TYPE_L3_ROUTING_TABLE, index, &entry);
	return SUCCESS;
}

int32 rtl8651_delAsicRouting(uint32 index) {
	rtl8651_tblAsic_l3RouteTable_t entry;
	if(index >= RTL8651_ROUTINGTBL_SIZE)
		return FAILED;
	entry.linkTo.ARPEntry.valid = 0;
	_rtl8651_forceAddAsicEntry(TYPE_L3_ROUTING_TABLE, index, &entry);
	return SUCCESS;
}

int32 rtl8651_getAsicRouting(uint32 index, ipaddr_t * ipAddr, ipaddr_t * ipMask, uint32 * process, //0: pppoe, 1:direct, 2:indirect, 4:Strong CPU, 
							uint32 * vidx, uint32 * arpStart, uint32 * arpEnd, 
							uint32 * nextHopRow, uint32 * nextHopColumn, uint32 * pppoeIdx) {
	uint32 i;
	rtl8651_tblAsic_l3RouteTable_t entry;
	
	if(index >= RTL8651_ROUTINGTBL_SIZE || ipAddr == NULL || ipMask == NULL || process == NULL ||	vidx == NULL || 
		arpStart == NULL || arpEnd == NULL || nextHopRow == NULL || nextHopColumn == NULL || pppoeIdx == NULL)
		return FAILED;

	_rtl8651_readAsicEntry(TYPE_L3_ROUTING_TABLE, index, &entry);
	if(entry.linkTo.ARPEntry.valid == 0)
		return FAILED;

	*ipAddr = entry.IPAddr;
	*process = entry.linkTo.ARPEntry.process;
	for(i=0, *ipMask = 0; i<=entry.linkTo.ARPEntry.IPMask; i++)
		*ipMask |= 1<<(31-i);
	
	*vidx = entry.linkTo.ARPEntry.vid;
	switch(*process) {
		case 0://PPPoE
			*arpStart = 0;
			*arpEnd = 0;
			*pppoeIdx = entry.linkTo.PPPoEEntry.PPPoEIndex;
			*nextHopRow = entry.linkTo.PPPoEEntry.nextHop>>2;
			*nextHopColumn = entry.linkTo.PPPoEEntry.nextHop & 0x3;
		break;
		case 1://Direct
			*arpStart = 0;
			*arpEnd = 0;
			*pppoeIdx = 0;
			*nextHopRow = entry.linkTo.L2Entry.nextHop>>2;
			*nextHopColumn = entry.linkTo.L2Entry.nextHop&0x3;
		break;
		case 2://Indirect
			*arpEnd = entry.linkTo.ARPEntry.ARPEnd;
			*arpStart = entry.linkTo.ARPEntry.ARPStart;
			*pppoeIdx = 0;
			*nextHopRow = 0;
			*nextHopColumn = 0;
		break;
		case 4:
			*arpStart = 0;
			*arpEnd = 0;
			*pppoeIdx = 0;
			*nextHopRow = 0;
			*nextHopColumn = 0;
		break;
	}
	return SUCCESS;
}

int32 rtl8651_setAsicArp(uint32 index, uint32 nextHopRow, uint32 nextHopColumn) {
	rtl8651_tblAsic_arpTable_t   entry;
	if(index >= RTL8651_ARPTBL_SIZE)
		return FAILED;
	entry.nextHop = (nextHopRow<<2) | (nextHopColumn&0x3);
	entry.valid = 1;
	_rtl8651_forceAddAsicEntry(TYPE_ARP_TABLE, index, &entry);
	return SUCCESS;
}

int32 rtl8651_delAsicArp(uint32 index) {
	rtl8651_tblAsic_arpTable_t   entry;
	if(index >= RTL8651_ARPTBL_SIZE)
		return FAILED;
	entry.valid = 0;
	_rtl8651_forceAddAsicEntry(TYPE_ARP_TABLE, index, &entry);
	return SUCCESS;
}

int32 rtl8651_getAsicArp(uint32 index, uint32 *nextHopRow, uint32 *nextHopColumn) {
	rtl8651_tblAsic_arpTable_t   entry;
	if(index>=RTL8651_ARPTBL_SIZE || nextHopRow == NULL || nextHopColumn == NULL)
		return FAILED;
	_rtl8651_readAsicEntry(TYPE_ARP_TABLE, index, &entry);
	if(entry.valid == 0)
		return FAILED;
	*nextHopRow = entry.nextHop>>2;
	*nextHopColumn = entry.nextHop&0x3;
	return SUCCESS;
}

int32 rtl8651_setAsicGidxRegister(uint32 regValue) {

	REG32(GIDXMCR) = regValue;
	return SUCCESS;
}

int32 rtl8651_getAsicGidxRegister(uint32 * regValue) {
	if(regValue == NULL)
		return FAILED;

	*regValue = REG32(GIDXMCR);
	
	return SUCCESS;
}

int32 rtl8651_setAsicExtIntIpTable(uint32 index, ipaddr_t extIpAddr, ipaddr_t intIpAddr, int8 localPublic, int8 nat) {
	rtl8651_tblAsic_extIpTable_t   entry;
	int32 retval;
	
 	if(index >= RTL8651_IPTABLE_SIZE || (localPublic == TRUE && nat == TRUE)) //Local public IP and NAT property cannot co-exist
		return FAILED;

	entry.externalIP = extIpAddr;
	entry.internalIP = intIpAddr;
	entry.isLocalPublic = localPublic==TRUE? 1: 0;
	entry.isOne2One = nat==TRUE? 1: 0;
	entry.valid = 1;
	retval=_rtl8651_forceAddAsicEntry(TYPE_EXT_INT_IP_TABLE, index, &entry);
	return SUCCESS;
}

int32 rtl8651_delAsicExtIntIpTable(uint32 index) {
	rtl8651_tblAsic_extIpTable_t   entry;
	int32 retval;
	if(index >= RTL8651_IPTABLE_SIZE)
		return FAILED;
	entry.valid = 0;
	retval = _rtl8651_forceAddAsicEntry(TYPE_EXT_INT_IP_TABLE, index, &entry);
	assert(retval==SUCCESS);
	return SUCCESS;
}

int32 rtl8651_getAsicExtIntIpTable(uint32 index, ipaddr_t *extIpAddr, ipaddr_t *intIpAddr, int8 *localPublic, int8 *nat) {
	rtl8651_tblAsic_extIpTable_t   entry;
	if(index>=RTL8651_IPTABLE_SIZE || extIpAddr == NULL || intIpAddr == NULL || localPublic == NULL || nat == NULL)
		return FAILED;
	_rtl8651_readAsicEntry(TYPE_EXT_INT_IP_TABLE, index, &entry);
	if(entry.valid == 0)
		return FAILED;//Entry not found
	*extIpAddr = entry.externalIP;
	*intIpAddr = entry.internalIP;
	*localPublic = entry.isLocalPublic==1? TRUE: FALSE;
	*nat = entry.isOne2One==1? TRUE: FALSE;
	return SUCCESS;
}

int32 rtl8651_setAsicServerPortTable(uint32 index, ipaddr_t extIpAddr, uint16 extPort, ipaddr_t intIpAddr, uint16 intPort) {
	rtl8651_tblAsic_srvPortTable_t   entry;
	if(index>=RTL8651_SERVERPORTTBL_SIZE)
		return FAILED;
	entry.externalIP = extIpAddr;
	entry.externalPort = extPort;
	entry.internalIP = intIpAddr;
	entry.internalPort = intPort;
	entry.valid = 1;
	_rtl8651_forceAddAsicEntry(TYPE_SERVER_PORT_TABLE, index, &entry);
	return SUCCESS;
}

int32 rtl8651_delAsicServerPortTable(uint32 index) {
	rtl8651_tblAsic_srvPortTable_t   entry;
	if(index>=RTL8651_SERVERPORTTBL_SIZE)
		return FAILED;
	entry.valid = 0;
	_rtl8651_forceAddAsicEntry(TYPE_SERVER_PORT_TABLE, index, &entry);
	return SUCCESS;
}

int32 rtl8651_getAsicServerPortTable(uint32 index, ipaddr_t *extIpAddr, uint16 *extPort, ipaddr_t *intIpAddr, uint16 *intPort) {
	rtl8651_tblAsic_srvPortTable_t   entry;
	
	if(index>=RTL8651_SERVERPORTTBL_SIZE || extIpAddr == NULL || extPort == NULL || intIpAddr == NULL || intPort == NULL)
		return FAILED;
	_rtl8651_readAsicEntry(TYPE_SERVER_PORT_TABLE, index, &entry);
	if(entry.valid == 0)
		return FAILED;
	*extIpAddr = entry.externalIP;
	*extPort = entry.externalPort;
	*intIpAddr = entry.internalIP;
	*intPort = entry.internalPort;
	return SUCCESS;
}

int32 rtl8651_setAsicAgingFunction(int8 l2Enable, int8 l4Enable) {
	REG32(TEATCR) = (REG32(TEATCR) & ~0x3) | (l2Enable == TRUE? 0x0: 0x1) | (l4Enable == TRUE? 0x0 : 0x2);
	return SUCCESS;
}

int32 rtl8651_getAsicAgingFunction(int8 * l2Enable, int8 * l4Enable) {
	if(l2Enable == NULL || l4Enable == NULL)
		return FAILED;

	*l2Enable = (REG32(TEATCR) & 0x1)? FALSE: TRUE;
	*l4Enable = (REG32(TEATCR) & 0x2)? FALSE: TRUE;
	return SUCCESS;
}

int32 rtl8651_setAsicNaptAutoAddDelete(int8 autoAdd, int8 autoDelete) {
	if(autoAdd == TRUE)
		REG32(SWTMCR)|=EN_NAPT_AUTO_LEARN;
	else
		REG32(SWTMCR)&=~EN_NAPT_AUTO_LEARN;
	if(autoDelete == TRUE)
		REG32(SWTMCR)|=EN_NAPT_AUTO_DELETE;
	else
		REG32(SWTMCR)&=~EN_NAPT_AUTO_DELETE;
	return SUCCESS;
}

int32 rtl8651_getAsicNaptAutoAddDelete(int8 *autoAdd, int8 *autoDelete) {
	if(autoAdd == NULL || autoDelete == NULL)
		return FAILED;
	if(REG32(SWTMCR)&EN_NAPT_AUTO_LEARN)
		*autoAdd = TRUE;
	else
		*autoAdd = FALSE;
	if(REG32(SWTMCR)&EN_NAPT_AUTO_DELETE)
		*autoDelete = TRUE;
	else
		*autoDelete = FALSE;
	return SUCCESS;
}

int32 rtl8651_setAsicNaptIcmpTimeout(uint32 timeout) {
	uint32 timeUnit = _rtl8651_NaptAgingToUnit(timeout), timeMask;

	if(timeUnit >= 0x400)
		return FAILED;
	timeMask = (uint32)0x3f<< 26;
	REG32(TEATCR) = (REG32(TEATCR) & ~timeMask) | (timeUnit<<26);
	return SUCCESS;
}

int32 rtl8651_getAsicNaptIcmpTimeout(uint32 *timeout) {
	if(timeout == NULL)
		return FAILED;
	*timeout = _rtl8651_NaptAgingToSec((REG32(TEATCR)>>26) & 0x3f);
	return SUCCESS;
}

int32 rtl8651_setAsicNaptUdpTimeout(uint32 timeout) {
	uint32 timeUnit = _rtl8651_NaptAgingToUnit(timeout), timeMask;

	if(timeUnit >= 0x400)
		return FAILED;
	timeMask = 0x3f << 20;
	REG32(TEATCR) = (REG32(TEATCR) & ~timeMask) | (timeUnit<<20);

	return SUCCESS;
}

int32 rtl8651_getAsicNaptUdpTimeout(uint32 *timeout) {
	if(timeout == NULL)
		return FAILED;
	*timeout = _rtl8651_NaptAgingToSec((REG32(TEATCR)>>20) & 0x3f);
	return SUCCESS;
}

int32 rtl8651_setAsicNaptTcpLongTimeout(uint32 timeout) {
	uint32 timeUnit = _rtl8651_NaptAgingToUnit(timeout), timeMask;

	if(timeUnit >= 0x400)
		return FAILED;
	timeMask = 0x3f << 14;
	REG32(TEATCR) = (REG32(TEATCR) & ~timeMask) | (timeUnit<<14);

	return SUCCESS;
}

int32 rtl8651_getAsicNaptTcpLongTimeout(uint32 *timeout) {
	if(timeout == NULL)
		return FAILED;
	*timeout = _rtl8651_NaptAgingToSec((REG32(TEATCR)>>14) & 0x3f);
	return SUCCESS;
}

int32 rtl8651_setAsicNaptTcpMediumTimeout(uint32 timeout) {
	uint32 timeUnit = _rtl8651_NaptAgingToUnit(timeout), timeMask;

	if(timeUnit >= 0x400)
		return FAILED;
	timeMask = 0x3f << 8;
	REG32(TEATCR) = (REG32(TEATCR) & ~timeMask) | (timeUnit<<8);

	return SUCCESS;
}

int32 rtl8651_getAsicNaptTcpMediumTimeout(uint32 *timeout) {
	if(timeout == NULL)
		return FAILED;
	*timeout = _rtl8651_NaptAgingToSec((REG32(TEATCR)>>8) & 0x3f);
	return SUCCESS;
}

int32 rtl8651_setAsicNaptTcpFastTimeout(uint32 timeout) {
	uint32 timeUnit = _rtl8651_NaptAgingToUnit(timeout), timeMask;

	if(timeUnit >= 0x400)
		return FAILED;
	timeMask = 0x3f << 2;
	REG32(TEATCR) = (REG32(TEATCR) & ~timeMask) | (timeUnit<<2);

	return SUCCESS;
}

int32 rtl8651_getAsicNaptTcpFastTimeout(uint32 *timeout) {
	if(timeout == NULL)
		return FAILED;
	*timeout = _rtl8651_NaptAgingToSec((REG32(TEATCR)>>2) & 0x3f);
	return SUCCESS;
}


#ifdef RTL8651B
//liberal entries, if directional, must be written separately by direction.
int32 rtl8651_setAsicNaptTcpUdpTableLiberal(int8 forced,
	ipaddr_t intIPAddr, uint16 intPort, uint32 extIpIdx, uint16 extPort,
	uint16 ageSec, int8 tcpQuiet, int8 isTcp, int8 direction,int8 collision, int8 valid)
{
	uint16 index = extPort & (RTL8651_TCPUDPTBL_SIZE-1);
	rtl8651_tblAsic_naptTcpUdpTable_t  entry;
	assert(index<RTL8651_TCPUDPTBL_SIZE);
	entry.valid = valid==TRUE? 1: 0;
	entry.collision = collision==TRUE? 1: 0;
	entry.intIPAddr = intIPAddr;
	entry.intPort = intPort;
	entry.offset = extPort>>RTL8651_TCPUDPTBL_BITS;
	entry.agingTime = _rtl8651_NaptAgingToUnit(ageSec);
	entry.isStatic = entry.dedicate = 1;
	entry.isTCP = isTcp==TRUE? 1: 0;
	//TCPFlag is reused in RTL8651B when dedicate bit is set. 
	//TCPFlag[2]= reserved, must be zero
	//TCPFlag[1]= 1: unidirectional,   0: bidirectional
	//TCPFlag[0]= 1: outbound flow,   0: inbound flow
	if(isTcp)
		entry.TCPFlag =  ((tcpQuiet?1:0)<<2)|((direction?1:0)<<1) | (((direction==1)?1:0));
	else
		entry.TCPFlag = 0;
	if(forced == TRUE)
		_rtl8651_forceAddAsicEntry(TYPE_L4_TCP_UDP_TABLE, index, &entry);
	else
		if(_rtl8651_addAsicEntry(TYPE_L4_TCP_UDP_TABLE, index, &entry))
			return FAILED;
	return SUCCESS;
}
#endif


int32 rtl8651_setAsicNaptTcpUdpTable(int8 forced, 
		ipaddr_t insideLocalIpAddr, uint16 insideLocalPort, uint16 insideGlobalPort, 
		uint16 ageSec, int8 isStatic, int8 isTcp, int8 tcpFlag, int8 isCollision, int8 isValid) {
	uint16 index = insideGlobalPort & 0x3ff;
	rtl8651_tblAsic_naptTcpUdpTable_t  entry;

	entry.intIPAddr = insideLocalIpAddr;
	entry.intPort = insideLocalPort;
	entry.offset = insideGlobalPort>>10;
	entry.agingTime = _rtl8651_NaptAgingToUnit(ageSec);
	entry.isStatic = isStatic==TRUE? 1: 0;
	entry.isTCP = isTcp==TRUE? 1: 0;
	entry.collision = isCollision==TRUE? 1: 0;
	if(isTcp==TRUE)
		entry.TCPFlag = tcpFlag; //0x4;
	else
		entry.TCPFlag = 0;
	entry.valid = isValid==TRUE? 1: 0;
	if(forced == TRUE)
		_rtl8651_forceAddAsicEntry(TYPE_L4_TCP_UDP_TABLE, index, &entry);
	else
		if(_rtl8651_addAsicEntry(TYPE_L4_TCP_UDP_TABLE, index, &entry))
			return FAILED;
	return SUCCESS;
}


#ifdef RTL8651B
int32 rtl8651_getAsicNaptTcpUdpTable2(uint8 *extIpIdx, uint16 index, 
		ipaddr_t *intIPAddr, uint16 *intPort, uint16 *extPort, 
		 uint16 *ageSec,  int8 * isStatic, int8 *isTcp, int8 *flags, int8 *isCollision, int8 *isValid) {
	rtl8651_tblAsic_naptTcpUdpTable_t  entry;
	
	assert(index<RTL8651_TCPUDPTBL_SIZE);

	_rtl8651_readAsicEntry(TYPE_L4_TCP_UDP_TABLE, index, &entry);


	if (intIPAddr != NULL) *intIPAddr = entry.intIPAddr;
	if (intPort != NULL) *intPort = entry.intPort;

	if(ISCLEARED(entry.TCPFlag, RTL8651_UNIDIRECTIONAL_NAPT_ENTRY) || /*if this entry is bi-directional */
		ISSET(entry.TCPFlag, RTL8651_OUTBOUND_NAPT_ENTRY)){ /*if this entry is unidirectional and is outbound */
		if (extIpIdx != NULL) *extIpIdx = entry.selIPidx;
		if (extPort != NULL) *extPort = (entry.selEntryIdx << RTL8651_TCPUDPTBL_BITS) | (index & (RTL8651_TCPUDPTBL_SIZE-1));
	}else{
		//this entry is for inbound flow, these two fields are don't care fields...
		if (extIpIdx != NULL) *extIpIdx = 0;
		if (extPort != NULL) *extPort = 0;
	}
	
	if (ageSec != NULL) *ageSec = _rtl8651_NaptAgingToSec(entry.agingTime);
	if (isStatic != NULL){
		if(entry.isStatic==1){
			if(entry.dedicate)
				*isStatic = RTL8651_LIBERAL_NAPT_ENTRY;
			else
				*isStatic = TRUE;
		}else
			*isStatic = FALSE;
	}
		
	if (isTcp != NULL) *isTcp = entry.isTCP==1? TRUE: FALSE;
	if (flags != NULL) *flags = entry.TCPFlag; 
	if (isCollision != NULL) *isCollision = entry.collision==1? TRUE: FALSE;
	if (isValid) *isValid = entry.valid;

	return SUCCESS;

}

#endif

int32 rtl8651_getAsicNaptTcpUdpTable(int8 precisePort, uint16 targetGlobalPort, 
		ipaddr_t *insideLocalIpAddr, uint16 *insideLocalPort, uint16 *insideGlobalPort, 
		uint16 *ageSec, int8 * isStatic, int8 *isTcp, int8 *tcpFlag, int8 *isCollision, int8 *isValid) {
	uint16 index = targetGlobalPort & 0x3ff;
	rtl8651_tblAsic_naptTcpUdpTable_t  entry;

	_rtl8651_readAsicEntry(TYPE_L4_TCP_UDP_TABLE, index, &entry);
	if (insideLocalIpAddr != NULL) *insideLocalIpAddr = entry.intIPAddr;
	if (insideLocalPort != NULL) *insideLocalPort = entry.intPort;
	if (insideGlobalPort != NULL) *insideGlobalPort = (entry.offset << 10) | (index & 0x3ff);
	if (ageSec != NULL) *ageSec = _rtl8651_NaptAgingToSec(entry.agingTime);
	if (isStatic != NULL) *isStatic = entry.isStatic==1? TRUE: FALSE;
	if (isTcp != NULL) *isTcp = entry.isTCP==1? TRUE: FALSE;
	if (tcpFlag != NULL) *tcpFlag = entry.TCPFlag;
	if (isCollision != NULL) *isCollision = entry.collision==1? TRUE: FALSE;
	if (isValid == NULL)
		return SUCCESS;

	if (entry.valid == 1) {
		if(precisePort == TRUE && ((entry.offset << 10) | (index & 0x3ff)) != targetGlobalPort)
			*isValid = FALSE;
		else *isValid = TRUE;
	} else *isValid = FALSE;

#if 0
	if(entry.valid == 1) {
		if(precisePort == TRUE && ((entry.offset << 10) | (index & 0x3ff)) != targetGlobalPort) {
			if (isValid != NULL) *isValid = FALSE;
		} else if (isValid != NULL) *isValid = TRUE;
	} else if (isValid != NULL)	*isValid = FALSE;
#endif
	return SUCCESS;
}


int32 rtl8651_setAsicNaptIcmpTable(int8 forced, 
		ipaddr_t insideLocalIpAddr, uint16 insideLocalId, uint16 insideGlobalId, 
		uint16 ageSec, int8 isStatic, int16 count, int8 isCollision, int8 isValid) {
	uint16 index = insideGlobalId & (RTL8651_ICMPTBL_SIZE-1);
	rtl8651_tblAsic_naptIcmpTable_t  entry;

	entry.intIPAddr = insideLocalIpAddr;
	entry.ICMPIDH = insideLocalId>>15;
	entry.ICMPIDL = insideLocalId&0x7fff;
	entry.offset = insideGlobalId>>10;
	entry.agingTime = _rtl8651_NaptAgingToUnit(ageSec);
	entry.isStatic = isStatic==TRUE? 1: 0;
	entry.count = count;
	entry.collision = isCollision==TRUE? 1: 0;
	entry.valid = isValid==TRUE? 1: 0;
	if(forced == TRUE)
		_rtl8651_forceAddAsicEntry(TYPE_L4_ICMP_TABLE, index, &entry);
	else
		if(_rtl8651_addAsicEntry(TYPE_L4_ICMP_TABLE, index, &entry))
			return FAILED;
	return SUCCESS;
}

int32 rtl8651_getAsicNaptIcmpTable(int8 preciseId, uint16 targetGlobalId, 
		ipaddr_t *insideLocalIpAddr, uint16 *insideLocalId, uint16 *insideGlobalId, //insideGlobalId is meaningful in first 6-bit and last 5-bit (Medium 5-bit is lost)
		uint16 *ageSec, int8 *isStatic, uint16 *count, int8 *isCollision, int8 *isValid) {
	uint16 index = targetGlobalId & 0x1f;
	rtl8651_tblAsic_naptIcmpTable_t  entry;
	
	if(insideLocalIpAddr == NULL || insideLocalId == NULL || insideGlobalId == NULL || ageSec == NULL || 
		isStatic == NULL || isCollision == NULL || isValid == NULL)
		return FAILED;

	_rtl8651_readAsicEntry(TYPE_L4_ICMP_TABLE, index, &entry);
	if (insideLocalIpAddr != NULL) *insideLocalIpAddr = entry.intIPAddr;
	if (insideLocalId != NULL) *insideLocalId = (entry.ICMPIDH << 15) | entry.ICMPIDL;
	if (insideGlobalId != NULL) *insideGlobalId = entry.offset<<10 | index;
	if (ageSec != NULL) *ageSec = _rtl8651_NaptAgingToSec(entry.agingTime);
	if (isStatic != NULL) *isStatic = entry.isStatic==1? TRUE: FALSE;
	if (count != NULL) *count = entry.count;
	if (isCollision != NULL) *isCollision = entry.collision==1? TRUE: FALSE;
	if(entry.valid == 1) {
		if(preciseId == TRUE && ((entry.offset << 10) | index) != (targetGlobalId & 0xfc1f)) {
			if (isValid != NULL) *isValid = FALSE;
		} else if (isValid != NULL) *isValid = TRUE;
	} else if (isValid != NULL)	*isValid = FALSE;
		
	return SUCCESS;
}

int32 rtl8651_setAsicL4Offset(uint16 start, uint16 end) {
	if(start > 0x3f || end > 0x3f || start > end)
		return FAILED;

	REG32(OCR) = start<<26 | end<<20;	
	return SUCCESS;
}

int32 rtl8651_getAsicL4Offset(uint16 *start, uint16 *end) {
	if(start == NULL || end == NULL)
		return FAILED;
	*start = (REG32(OCR)>>26) & 0x3f;
	*end = (REG32(OCR)>>20) & 0x3f;
	return SUCCESS;
}

int32 rtl8651_setAsicAlg(uint32 index, uint16 port) {
	rtl8651_tblAsic_algTable_t entry;
	
	if(index >= RTL8651_ALGTBL_SIZE)
		return FAILED;

	entry.L4Port = port;
	entry.valid = 1;
	_rtl8651_forceAddAsicEntry(TYPE_ALG_TABLE, index, &entry);
	return SUCCESS;
}

int32 rtl8651_delAsicAlg(uint32 index) {
	rtl8651_tblAsic_algTable_t entry;
	
	if(index >= RTL8651_ALGTBL_SIZE)
		return FAILED;

	entry.valid = 0;
	_rtl8651_forceAddAsicEntry(TYPE_ALG_TABLE, index, &entry);
	return SUCCESS;
}

int32 rtl8651_getAsicAlg(uint32 index, uint16 *port) {
	rtl8651_tblAsic_algTable_t entry;
	
	if(index >= RTL8651_ALGTBL_SIZE || port == NULL)
		return FAILED;

	_rtl8651_readAsicEntry(TYPE_ALG_TABLE, index, &entry);

	if(entry.valid == 0)
		return FAILED;
	*port = entry.L4Port;
	return SUCCESS;
}

int32 rtl8651_setAsicMacAcl(uint32 index, rtl8651_tblDrvMacAclRule_t * rule) {
	rtl8651_tblAsic_aclTable_t    entry;
	
	if(index >= RTL8651_ACLTBL_SIZE+RTL8651_ACLTBL_RESERV_SIZE || rule == NULL)
		return FAILED;
	entry.is.ETHERNET.dMacP47_32 = rule->dstMac.octet[0]<<8 | rule->dstMac.octet[1];
	entry.is.ETHERNET.dMacP31_16 = rule->dstMac.octet[2]<<8 | rule->dstMac.octet[3];
	entry.is.ETHERNET.dMacP15_0 = rule->dstMac.octet[4]<<8 | rule->dstMac.octet[5];
	entry.is.ETHERNET.dMacM47_32 = rule->dstMacMask.octet[0]<<8 | rule->dstMacMask.octet[1];
	entry.is.ETHERNET.dMacM31_16 = rule->dstMacMask.octet[2]<<8 | rule->dstMacMask.octet[3];
	entry.is.ETHERNET.dMacM15_0 = rule->dstMacMask.octet[4]<<8 | rule->dstMacMask.octet[5];
	entry.is.ETHERNET.sMacP47_32 = rule->srcMac.octet[0]<<8 | rule->srcMac.octet[1];
	entry.is.ETHERNET.sMacP31_16 = rule->srcMac.octet[2]<<8 | rule->srcMac.octet[3];
	entry.is.ETHERNET.sMacP15_0 = rule->srcMac.octet[4]<<8 | rule->srcMac.octet[5];
	entry.is.ETHERNET.sMacM47_32 = rule->srcMacMask.octet[0]<<8 | rule->srcMacMask.octet[1];
	entry.is.ETHERNET.sMacM31_16 = rule->srcMacMask.octet[2]<<8 | rule->srcMacMask.octet[3];
	entry.is.ETHERNET.sMacM15_0 = rule->srcMacMask.octet[4]<<8 | rule->srcMacMask.octet[5];
	entry.is.ETHERNET.ethTypeP = rule->typeLen;
	entry.is.ETHERNET.ethTypeM = rule->typeLenMask;
	entry.ruleType = 0x0;
	switch(rule->actionType) {
		case RTL8651_ACL_PERMIT:
			entry.actionType = 0;
		break;
		case RTL8651_ACL_DROP:
			entry.actionType = 4;
		break;
		case RTL8651_ACL_CPU:
			entry.actionType = 5;
		break;
	}
	_rtl8651_forceAddAsicEntry(TYPE_ACL_RULE_TABLE, index, &entry);
	return SUCCESS;
}

int32 rtl8651_getAsicMacAcl(uint32 index, rtl8651_tblDrvMacAclRule_t * rule) {
	rtl8651_tblAsic_aclTable_t    entry;
	
	if(index >= RTL8651_ACLTBL_SIZE+RTL8651_ACLTBL_RESERV_SIZE || rule == NULL)
		return FAILED;
	_rtl8651_readAsicEntry(TYPE_ACL_RULE_TABLE, index, &entry);
	if(entry.ruleType != 0)
		return FAILED;
	rule->dstMac.octet[0] = entry.is.ETHERNET.dMacP47_32 >> 8;
	rule->dstMac.octet[1] = entry.is.ETHERNET.dMacP47_32 & 0xff;
	rule->dstMac.octet[2] = entry.is.ETHERNET.dMacP31_16 >> 8;
	rule->dstMac.octet[3] = entry.is.ETHERNET.dMacP31_16 & 0xff;
	rule->dstMac.octet[4] = entry.is.ETHERNET.dMacP15_0 >> 8;
	rule->dstMac.octet[5] = entry.is.ETHERNET.dMacP15_0 & 0xff;
	rule->dstMacMask.octet[0] = entry.is.ETHERNET.dMacM47_32 >> 8;
	rule->dstMacMask.octet[1] = entry.is.ETHERNET.dMacM47_32 & 0xff;
	rule->dstMacMask.octet[2] = entry.is.ETHERNET.dMacM31_16 >> 8;
	rule->dstMacMask.octet[3] = entry.is.ETHERNET.dMacM31_16 & 0xff;
	rule->dstMacMask.octet[4] = entry.is.ETHERNET.dMacM15_0 >> 8;
	rule->dstMacMask.octet[5] = entry.is.ETHERNET.dMacM15_0 & 0xff;
	rule->srcMac.octet[0] = entry.is.ETHERNET.sMacP47_32 >> 8;
	rule->srcMac.octet[1] = entry.is.ETHERNET.sMacP47_32 & 0xff;
	rule->srcMac.octet[2] = entry.is.ETHERNET.sMacP31_16 >> 8;
	rule->srcMac.octet[3] = entry.is.ETHERNET.sMacP31_16 & 0xff;
	rule->srcMac.octet[4] = entry.is.ETHERNET.sMacP15_0 >> 8;
	rule->srcMac.octet[5] = entry.is.ETHERNET.sMacP15_0 & 0xff;
	rule->srcMacMask.octet[0] = entry.is.ETHERNET.sMacM47_32 >> 8;
	rule->srcMacMask.octet[1] = entry.is.ETHERNET.sMacM47_32 & 0xff;
	rule->srcMacMask.octet[2] = entry.is.ETHERNET.sMacM31_16 >> 8;
	rule->srcMacMask.octet[3] = entry.is.ETHERNET.sMacM31_16 & 0xff;
	rule->srcMacMask.octet[4] = entry.is.ETHERNET.sMacM15_0 >> 8;
	rule->srcMacMask.octet[5] = entry.is.ETHERNET.sMacM15_0 & 0xff;
	rule->typeLen = entry.is.ETHERNET.ethTypeP;
	rule->typeLenMask = entry.is.ETHERNET.ethTypeM;
	switch(entry.actionType) {
		case 0:
			rule->actionType = RTL8651_ACL_PERMIT;
		break;
		case 4:
			rule->actionType = RTL8651_ACL_DROP;
		break;
		case 5:
			rule->actionType = RTL8651_ACL_CPU;
		break;
	}
	return SUCCESS;
}

int32 rtl8651_setAsicIpAcl(uint32 index, rtl8651_tblDrvIpAclRule_t * rule) {
	rtl8651_tblAsic_aclTable_t    entry;
	
	if(index >= RTL8651_ACLTBL_SIZE+RTL8651_ACLTBL_RESERV_SIZE || rule == NULL)
		return FAILED;

	switch(rule->ruleType) {
		case RTL8651_ACL_IP:
		entry.is.L3L4.is.IP.IPTOSP = rule->tos;
		entry.is.L3L4.is.IP.IPTOSM = rule->tosMask;
		entry.is.L3L4.is.IP.IPProtoP = rule->is.ip.proto;
		entry.is.L3L4.is.IP.IPProtoM = rule->is.ip.protoMask;
		entry.is.L3L4.is.IP.IPFlagP = rule->is.ip.ipFlag;
		entry.is.L3L4.is.IP.IPFlagM = rule->is.ip.flagMask;
		entry.ruleType = 0x2;
		goto l3l4_shared;
            
		case RTL8651_ACL_ICMP:
		entry.is.L3L4.is.ICMP.IPTOSP = rule->tos;
		entry.is.L3L4.is.ICMP.IPTOSM = rule->tosMask;
		entry.is.L3L4.is.ICMP.ICMPTypeP = rule->is.icmp.type;
		entry.is.L3L4.is.ICMP.ICMPTypeM = rule->is.icmp.typeMask;
		entry.is.L3L4.is.ICMP.ICMPCodeP = rule->is.icmp.code;
		entry.is.L3L4.is.ICMP.ICMPCodeM = rule->is.icmp.codeMask;
		entry.ruleType = 0x4;
		goto l3l4_shared;
            
		case RTL8651_ACL_IGMP:
		entry.is.L3L4.is.IGMP.IPTOSP = rule->tos;
		entry.is.L3L4.is.IGMP.IPTOSM = rule->tosMask;
		entry.is.L3L4.is.IGMP.IGMPTypeP = rule->is.igmp.type;
		entry.is.L3L4.is.IGMP.IGMPTypeM = rule->is.igmp.typeMask;
		entry.ruleType = 0x5;
		goto l3l4_shared;
            
		case RTL8651_ACL_TCP:
		entry.is.L3L4.is.TCP.IPTOSP = rule->tos;
		entry.is.L3L4.is.TCP.IPTOSM = rule->tosMask;
		entry.is.L3L4.is.TCP.TCPFlagP = rule->is.tcp.tcpflag;
		entry.is.L3L4.is.TCP.TCPFlagM = rule->is.tcp.flagMask;
		entry.is.L3L4.is.TCP.TCPSPUB = rule->is.tcp.srcPortUpperBound;
		entry.is.L3L4.is.TCP.TCPSPLB = rule->is.tcp.srcPortLowerBound;
		entry.is.L3L4.is.TCP.TCPDPUB = rule->is.tcp.dstPortUpperBound;
		entry.is.L3L4.is.TCP.TCPDPLB = rule->is.tcp.dstPortLowerBound;
		entry.ruleType = 0x6;
            goto l3l4_shared;
            
		case RTL8651_ACL_UDP:
		entry.is.L3L4.is.UDP.IPTOSP = rule->tos;
		entry.is.L3L4.is.UDP.IPTOSM = rule->tosMask;
		entry.is.L3L4.is.UDP.UDPSPUB = rule->is.udp.srcPortUpperBound;
		entry.is.L3L4.is.UDP.UDPSPLB = rule->is.udp.srcPortLowerBound;
		entry.is.L3L4.is.UDP.UDPDPUB = rule->is.udp.dstPortUpperBound;
		entry.is.L3L4.is.UDP.UDPDPLB = rule->is.udp.dstPortLowerBound;
		entry.ruleType = 0x7;
            
l3l4_shared:
		entry.is.L3L4.sIPP = rule->srcIpAddr;
		entry.is.L3L4.sIPM = rule->srcIpAddrMask;
		entry.is.L3L4.dIPP = rule->dstIpAddr;
		entry.is.L3L4.dIPM = rule->dstIpAddrMask;
		break;
		default:
			return FAILED;//Unknown types
	}
	switch(rule->actionType) {
		case RTL8651_ACL_PERMIT:
			entry.actionType = 0;
		break;
		case RTL8651_ACL_DROP:
			entry.actionType = 4;
		break;
		case RTL8651_ACL_CPU:
			entry.actionType = 5;
		break;
	}
	_rtl8651_forceAddAsicEntry(TYPE_ACL_RULE_TABLE, index, &entry);
	return SUCCESS;
}

int32 rtl8651_getAsicIpAcl(uint32 index, rtl8651_tblDrvIpAclRule_t * rule) {
	rtl8651_tblAsic_aclTable_t    entry;
	
	if(index >= RTL8651_ACLTBL_SIZE+RTL8651_ACLTBL_RESERV_SIZE || rule == NULL)
		return FAILED;

	_rtl8651_readAsicEntry(TYPE_ACL_RULE_TABLE, index, &entry);
	switch(entry.ruleType) {
		case 0x2:
		rule->tos = entry.is.L3L4.is.IP.IPTOSP;
		rule->tosMask = entry.is.L3L4.is.IP.IPTOSM;
		rule->is.ip.proto = entry.is.L3L4.is.IP.IPProtoP;
		rule->is.ip.protoMask = entry.is.L3L4.is.IP.IPProtoM;
		rule->is.ip.ipFlag = entry.is.L3L4.is.IP.IPFlagP;
		rule->is.ip.flagMask = entry.is.L3L4.is.IP.IPFlagM;
		rule->ruleType = RTL8651_ACL_IP;
		goto l3l4_shared;
            
		case 0x4:
		rule->tos = entry.is.L3L4.is.ICMP.IPTOSP;
		rule->tosMask = entry.is.L3L4.is.ICMP.IPTOSM;
		rule->is.icmp.type = entry.is.L3L4.is.ICMP.ICMPTypeP;
		rule->is.icmp.typeMask = entry.is.L3L4.is.ICMP.ICMPTypeM;
		rule->is.icmp.code = entry.is.L3L4.is.ICMP.ICMPCodeP;
		rule->is.icmp.codeMask = entry.is.L3L4.is.ICMP.ICMPCodeM;
		rule->ruleType = RTL8651_ACL_ICMP;
		goto l3l4_shared;
            
		case 0x5:
		rule->tos = entry.is.L3L4.is.IGMP.IPTOSP;
		rule->tosMask = entry.is.L3L4.is.IGMP.IPTOSM;
		rule->is.igmp.type = entry.is.L3L4.is.IGMP.IGMPTypeP;
		rule->is.igmp.typeMask = entry.is.L3L4.is.IGMP.IGMPTypeM;
		rule->ruleType = RTL8651_ACL_IGMP;
		goto l3l4_shared;
            
		case 0x6:
		rule->tos = entry.is.L3L4.is.TCP.IPTOSP;
		rule->tosMask = entry.is.L3L4.is.TCP.IPTOSM;
		rule->is.tcp.tcpflag = entry.is.L3L4.is.TCP.TCPFlagP;
		rule->is.tcp.flagMask = entry.is.L3L4.is.TCP.TCPFlagM;
		rule->is.tcp.srcPortUpperBound = entry.is.L3L4.is.TCP.TCPSPUB;
		rule->is.tcp.srcPortLowerBound = entry.is.L3L4.is.TCP.TCPSPLB;
		rule->is.tcp.dstPortUpperBound = entry.is.L3L4.is.TCP.TCPDPUB;
		rule->is.tcp.dstPortLowerBound = entry.is.L3L4.is.TCP.TCPDPLB;
		rule->ruleType = RTL8651_ACL_TCP;
            goto l3l4_shared;
            
		case 0x7:
		rule->tos = entry.is.L3L4.is.UDP.IPTOSP;
		rule->tosMask = entry.is.L3L4.is.UDP.IPTOSM;
		rule->is.udp.srcPortUpperBound = entry.is.L3L4.is.UDP.UDPSPUB;
		rule->is.udp.srcPortLowerBound = entry.is.L3L4.is.UDP.UDPSPLB;
		rule->is.udp.dstPortUpperBound = entry.is.L3L4.is.UDP.UDPDPUB;
		rule->is.udp.dstPortLowerBound = entry.is.L3L4.is.UDP.UDPDPLB;
		rule->ruleType = RTL8651_ACL_UDP;
            
l3l4_shared:
		rule->srcIpAddr = entry.is.L3L4.sIPP;
		rule->srcIpAddrMask = entry.is.L3L4.sIPM;
		rule->dstIpAddr = entry.is.L3L4.dIPP;
		rule->dstIpAddrMask = entry.is.L3L4.dIPM;
		break;
		default:
			return FAILED;//Type unknown
	}
	switch(entry.actionType) {
		case 0:
			rule->actionType = RTL8651_ACL_PERMIT;
		break;
		case 4:
			rule->actionType = RTL8651_ACL_DROP;
		break;
		case 5:
			rule->actionType = RTL8651_ACL_CPU;
		break;
	}
	
	return SUCCESS;
}

int32 rtl8651_getAsicNaptTcpUdpOffset(uint16 index, uint16 * offset, int8 * isValid) {
	rtl8651_tblAsic_naptTcpUdpTable_t  entry;

	if(offset == NULL || isValid == NULL || index >= RTL8651_TCPUDPTBL_SIZE)
		return FAILED;

	_rtl8651_readAsicEntry(TYPE_L4_TCP_UDP_TABLE, index, &entry);

	*offset = entry.offset;
	*isValid = entry.valid == 1? TRUE: FALSE;
	
	return SUCCESS;
}

int32 rtl8651_getAsicNaptIcmpOffset(uint16 index, uint16 * offset, int8 * isValid) {
	rtl8651_tblAsic_naptIcmpTable_t  entry;

	if(offset == NULL || isValid == NULL || index>= RTL8651_ICMPTBL_SIZE)
		return FAILED;
	
	_rtl8651_readAsicEntry(TYPE_L4_ICMP_TABLE, index, &entry);

	*offset = entry.offset;
	*isValid = entry.valid==1? TRUE: FALSE;
	
	return SUCCESS;
}

int32 rtl8651_setAsicIpMulticastTable(ipaddr_t srcAddr, ipaddr_t dstAddr, uint16 svid, uint32 srcPort, uint32 dstPortMask) {
	uint32 idx = rtl8651_ipMulticastTableIndex(srcAddr, dstAddr);
	rtl8651_tblAsic_ipMulticastTable_t entry;
#ifdef RTL8651B
	if(dstAddr >>28 != 0xe || srcPort >= RTL8651_PORT_NUMBER+RTL8651_EXTPORT_NUMBER)
		return FAILED;//Non-IP multicast destination address
#else
	if(dstAddr >>28 != 0xe || srcPort >= RTL8651_PORT_NUMBER)
		return FAILED;//Non-IP multicast destination address
#endif

	entry.srcIPAddr = srcAddr;
	entry.destIPAddrLsbs = dstAddr & 0xfffffff;
	entry.srcPortL = srcPort & 0x1; /* 1st bit */
	entry.srcPortH = srcPort>>1;   /* 2,3rd bits */

#ifdef RTL8651B
	entry.extSrcPort = srcPort >>3; /*MSB 4th bit*/
	entry.extPortList = dstPortMask >> RTL8651_PORT_NUMBER;
#endif
	entry.srcVid = rtl8651_vlanTableIndex(svid);
	entry.toCPU = 0;
	entry.valid = 1;
	entry.extIPIndex = 0;
	entry.portList = dstPortMask & (RTL8651_ALLPORTMASK);
	
	_rtl8651_forceAddAsicEntry(TYPE_MULTICAST_TABLE, idx, &entry);
	return SUCCESS;
}

int32 rtl8651_delAsicIpMulticastTable(ipaddr_t srcAddr, ipaddr_t dstAddr) {
	uint32 idx = rtl8651_ipMulticastTableIndex(srcAddr, dstAddr);
	rtl8651_tblAsic_ipMulticastTable_t entry;

	if(dstAddr >>28 != 0xe)
		return FAILED;//Non-IP multicast destination address
	entry.valid = 0;

	_rtl8651_forceAddAsicEntry(TYPE_MULTICAST_TABLE, idx, &entry);
	return SUCCESS;
}

int32 rtl8651_getAsicIpMulticastTable(ipaddr_t srcAddr, ipaddr_t dstAddr, uint16 *svid, uint32 *srcPort, uint32 *dstPortMask) {
	uint32 idx = rtl8651_ipMulticastTableIndex(srcAddr, dstAddr);
	rtl8651_tblAsic_vlanTable_t vlanEntry;
	rtl8651_tblAsic_ipMulticastTable_t entry;

	if(dstAddr >>28 != 0xe || svid == NULL || srcPort == NULL || dstPortMask == NULL)
		return FAILED;//Non-IP multicast destination address
	_rtl8651_readAsicEntry(TYPE_MULTICAST_TABLE, idx, &entry);
	if(entry.valid == 0)
		return FAILED;//Multicast table entry invalid
	_rtl8651_readAsicEntry(TYPE_VLAN_TABLE, entry.srcVid, &vlanEntry);
 	if(vlanEntry.valid == 0)
 		return FAILED;//VLAN not exist
	*svid = vlanEntry.vhid<<3 | entry.srcVid;
#ifdef RTL8651B
	*srcPort = entry.extSrcPort<<3 | entry.srcPortH<<1 | entry.srcPortL;
	*dstPortMask = entry.extPortList<<RTL8651_PORT_NUMBER | entry.portList;
#else
	*srcPort = entry.srcPortH<<1 | entry.srcPortL;
	*dstPortMask = entry.portList;
#endif
	return SUCCESS;
}

uint32 rtl8651_returnAsicCounter(uint32 offset) {
	if(offset & 0x3)
		return 0;
	return  REG32(MIB_COUNTER_BASE + offset);
}

uint32 rtl8651_clearAsicCounter(void) {
	//Clear mib counter and enable MIB counter as system counter
	REG32(MIB_CONTROL) = IN_COUNTER_RESTART | OUT_COUNTER_RESTART | PORT_FOR_COUNTING_MASK;
	_rtl8651_initialRead();
	return SUCCESS;
}


void rtl8651_intHandler(void) {
int32  intPending;

	/* Read interrupt control register */
	intPending = REG32(CPUIISR);

	/* Filter masked interrupt */
	intPending &= REG32(CPUIIMR);

	/* Check and handle NIC interrupts */
	if (intPending & INTPENDING_NIC_MASK)
		//swNic_intHandler(intPending);
		;
	/* Check and handle link change interrupt */
	if (intPending & LINK_CHANG_IP) {
		linkChangePendingCount++;
		REG32(CPUIISR) = (uint32)LINK_CHANG_IP;
	}

	/* Check and handle L4 collision removal interrupt */
	if (intPending & L4_COL_REMOVAL_IP)
		/* Handle L4 collision removal here */;
    
	/* Check and handle software interrupt */
	if (intPending & SW_INT_IP)
		/* Handle software interrupt here */;

}

int32 rtl8651_getAsicHsB(rtl8651_tblAsic_hsb_param_watch_t * hsbWatch) {
	uint32 v32;
	uint32 m[6]; // for mac address
	uint32 a[4]; // for ip address
	uint32 i=0;
	rtl8651_tblAsic_hsb_param_t hsbParam;

	/* clear data structure */	
	memset((void *)&hsbParam, 0, sizeof(rtl8651_tblAsic_hsb_param_t));
	memset((void *)hsbWatch, 0, sizeof(rtl8651_tblAsic_hsb_param_watch_t));
	
	/* read HSB */
	memcpy((void *)&hsbParam,(void *)HSB_BASE, sizeof(rtl8651_tblAsic_hsb_param_t));
	
	hsbWatch->type = (uint32)hsbParam.type;
#ifdef RTL8651B	
	hsbWatch->spa = (uint32)(hsbParam.extSrcPortNum<<4) |hsbParam.spa;
	hsbWatch->ipLen = hsbParam.ipLen;
	hsbWatch->L2only=hsbParam.L2only;
#else
	hsbWatch->spa = (uint32)hsbParam.spa;
#endif
	hsbWatch->bc = (uint32)hsbParam.bc;
	hsbWatch->vlan = (uint32)hsbParam.vlan;
	hsbWatch->pppoe = (uint32)hsbParam.pppoe;
	hsbWatch->llc = (uint32)hsbParam.llc;
	
	// Protocol contents
	hsbWatch->vid = (uint32)hsbParam.vid;
	memset((void *)m, 0, sizeof(m));
	m[0] =  (hsbParam.da47_20 & 0x0ff00000) >> 20;
	m[1] =  (hsbParam.da47_20 & 0x000ff000) >> 12;
	m[2] =  (hsbParam.da47_20 & 0x00000ff0) >> 4 ;
	m[3] =  (hsbParam.da47_20 & 0x0000000f) << 4 ;
	m[3] |= (hsbParam.da19_0  & 0x000f0000) >> 16;
	m[4] =  (hsbParam.da19_0  & 0x0000ff00) >> 8 ;
	m[5] =  (hsbParam.da19_0  & 0x000000ff)      ;
	for (i=0; i<6; i++)
		hsbWatch->da[i] = m[i];
	
	memset((void *)m, 0, sizeof(m));
	m[0] =  (hsbParam.sa47_36 & 0x00000ff0) >> 4;
	m[1] =  (hsbParam.sa47_36 & 0x0000000f) << 4;
	m[1] |= (hsbParam.sa35_4  & 0xf0000000) >> 28;
	m[2] =  (hsbParam.sa35_4  & 0x0ff00000) >> 20;
	m[3] =  (hsbParam.sa35_4  & 0x000ff000) >> 12;
	m[4] =  (hsbParam.sa35_4  & 0x00000ff0) >>  4;
	m[5] =  (hsbParam.sa35_4  & 0x0000000f) <<  4;
	m[5] |= (hsbParam.sa3_0   & 0x0000000f);
	for (i=0; i<6; i++)
		hsbWatch->sa[i] = m[i];

	v32 = (hsbParam.ethrtype15_4 << 4) | hsbParam.ethrtype3_0;
	hsbWatch->ethrtype = v32;

	memset((void *)a, 0, sizeof(a));
	a[0] =  (uint32)((hsbParam.sip31_3 & 0x1fE00000) >> 21);
	a[1] =  (uint32)((hsbParam.sip31_3 & 0x001fE000) >> 13);
	a[2] =  (uint32)((hsbParam.sip31_3 & 0x00001fE0) >> 5 );
	a[3] =  (uint32)((hsbParam.sip31_3 & 0x0000001f) << 3 );
	a[3] |= (uint32)((hsbParam.sip2_0)                  );
	for (i=0; i<4; i++)
		hsbWatch->sip[i] = a[i];
	
	v32 = (hsbParam.sprt15_3 << 3) | hsbParam.sprt2_0;
	hsbWatch->sprt = (uint32)v32;
	
	memset((void *)a, 0, sizeof(a));
	a[0] =  (hsbParam.dip31_19 & 0x00001fe0) >>  5;
	a[1] =  (hsbParam.dip31_19 & 0x0000001f) <<  3;
	a[1] |= (hsbParam.dip18_0  & 0x00070000) >> 16;
	a[2] =  (hsbParam.dip18_0  & 0x0000ff00) >> 8 ;
	a[3] =  (hsbParam.dip18_0  & 0x000000ff)      ;
	for (i=0; i<4; i++)
		hsbWatch->dip[i] = a[i];
		
	hsbWatch->dprt = (uint32)hsbParam.dprt;
	
	v32 = (hsbParam.ipptl7_3 << 3) | hsbParam.ipptl2_0;
	hsbWatch->ipptl = (uint32)v32;
	hsbWatch->ipflg = (uint32)hsbParam.ipflg;
	hsbWatch->iptos = (uint32)hsbParam.iptos;
	hsbWatch->tcpflg = (uint32)hsbParam.tcpflg;
	
	hsbWatch->dirtx = (uint32)hsbParam.dirtx;
	hsbWatch->prtnmat = (uint32)hsbParam.prtnmat;
       
	hsbWatch->udp_nocs = (uint32)hsbParam.udp_nocs;
	hsbWatch->ttlst = (uint32)hsbParam.ttlst;
	hsbWatch->pktend = (uint32)hsbParam.pktend;
	
	hsbWatch->hp = (uint32)hsbParam.hp;
	hsbWatch->hp2 = (uint32)hsbParam.hp2;
	hsbWatch->dp = (uint32)hsbParam.dp;
	
	hsbWatch->l4crcok = (uint32)hsbParam.l4crcok;
	hsbWatch->l3crcok = (uint32)hsbParam.l3crcok;
	hsbWatch->ipfragif = (uint32)hsbParam.ipfragif;

	return SUCCESS;
} /* end hs_readHsb */

/* read header-stamp-after */
int32 rtl8651_getAsicHsA(rtl8651_tblAsic_hsa_param_watch_t * hsaWatch) {
	//uint32 v32;
	uint16 v16;
	uint32 m[6]; // for mac address
	uint32 a[4]; // for ip address	
	uint32 i=0;
	rtl8651_tblAsic_hsa_param_t hsaParam;

	/* clear data structure */	
	memset((void *)&hsaParam, 0, sizeof(rtl8651_tblAsic_hsa_param_t));
	memset((void *)hsaWatch, 0, sizeof(rtl8651_tblAsic_hsa_param_watch_t));
	
	/* read HSB */
	memcpy((void *)&hsaParam, (void*)HSA_BASE, sizeof(rtl8651_tblAsic_hsa_param_t));
	
	memset((void *)m, 0, sizeof(m));
	m[0] =  (hsaParam.mac47_32 & 0x0000ff00) >> 8 ;
	m[1] =  (hsaParam.mac47_32 & 0x000000ff)      ;
	m[2] =  (hsaParam.mac31_0  & 0xff000000) >> 24;
	m[3] =  (hsaParam.mac31_0  & 0x00ff0000) >> 16;
	m[4] =  (hsaParam.mac31_0  & 0x0000ff00) >> 8 ;
	m[5] =   hsaParam.mac31_0  & 0x000000ff       ;
	for (i=0; i<6; i++)
		hsaWatch->mac[i] = m[i];

	memset((void *)a, 0, sizeof(a));
	a[0] =  hsaParam.ip31_16 >> 8;
	a[1] =  hsaParam.ip31_16 & 0x00ff; 
	a[2] =  hsaParam.ip15_0  >> 8;
	a[3] =  hsaParam.ip15_0  & 0x00ff;
	for (i=0; i<4; i++)
	{
		hsaWatch->ip[i] = a[i];
	}

#ifdef RTL8651B
//cfliu:pending, don't know how JCWang allocate these bits.
/*
	hsaWatch->prt = (uint32)hsaParam.prt;
	uint32 	frag2cksumDiff:16,
			ACLtrapFlag:2, //cfliu: why 2 bits here???
			extSrcPortNum:2,
			extDstPortMask:4,
			extTTL_1:3;
*/
#endif

	
	hsaWatch->prt = (uint32)hsaParam.prt;
	hsaWatch->l3cs = (uint32)hsaParam.l3cs;
	hsaWatch->l4cs = (uint32)hsaParam.l4cs;
	hsaWatch->egress = (uint32)hsaParam.egress;
	hsaWatch->l2act = (uint32)hsaParam.l2act;
	hsaWatch->l34act = (uint32)hsaParam.l34act;
	hsaWatch->dirtx = (uint32)hsaParam.dirtx;
	hsaWatch->type = (uint32)hsaParam.type;
	hsaWatch->llc = (uint32)hsaParam.llc;
	hsaWatch->vlan = (uint32)hsaParam.vlan;
	hsaWatch->dvid = (uint32)hsaParam.dvid;
	hsaWatch->pppoe = (uint32)hsaParam.pppoe;
	hsaWatch->pppid = (uint32)hsaParam.pppid;
	hsaWatch->ttl_1 = (uint32)hsaParam.ttl_1;
	hsaWatch->dpc = (uint32)hsaParam.dpc;

	v16 = 0;
	v16 =  hsaParam.bc10_5 << 5;
	v16 |= hsaParam.bc4_0;
	hsaWatch->bc = (uint32)v16;

	hsaWatch->pktend = (uint32)hsaParam.pktend;
	hsaWatch->mulcst = (uint32)hsaParam.mulcst;
	hsaWatch->svid = (uint32)hsaParam.svid;
	hsaWatch->cpursn = (uint32)hsaParam.cpursn;
	hsaWatch->spa = (uint32)hsaParam.spa;

	return SUCCESS;
} /* end hs_readHsa */


int32 rtl8651_testAsicDrv(void) {
	uint32 i, j;

	for(i=0; i<14; i++) {//Operation layer test
		uint32 setOpLayer, getOpLayer;
		int8 l2Enable, l4Enable;
		setOpLayer = (i%7)<4? (i%7)+1: 7-(i%7);
		rtl8651_setAsicOperationLayer(setOpLayer);
		rtl8651_getAsicOperationLayer(&getOpLayer);
		if(setOpLayer != getOpLayer) {
			printf("Error! Set operation mode at layer %d but get information to be layer %d\n", setOpLayer, getOpLayer);
			return FAILED;
		}
		rtl8651_getAsicAgingFunction(&l2Enable, &l4Enable);
		if((setOpLayer == 1 && (l2Enable != FALSE || l4Enable != FALSE)) || ((setOpLayer == 2 || setOpLayer == 3) && (l2Enable != TRUE || l4Enable != FALSE)) || (setOpLayer == 4 && (l2Enable != TRUE || l4Enable != TRUE)) ) {
				printf("Error! Operation at layer %d with error aging funciton enabled (L2 Enable: %s L4 Enable: %s)\n", setOpLayer, l2Enable==TRUE? "TRUE": "FALSE", l4Enable==TRUE? "TRUE": "FALSE");
				return FAILED;
		}
	}
	for(i=0; i<4; i++) {//Spanning tree status check
		int8 setSpanningTreeEnabled, getSpanningTreeEnabled;
		setSpanningTreeEnabled = i%2? TRUE: FALSE;
		rtl8651_setAsicSpanningEnable(setSpanningTreeEnabled);
		rtl8651_getAsicSpanningEnable(&getSpanningTreeEnabled);
		if(setSpanningTreeEnabled != getSpanningTreeEnabled) {
			printf("Error! Configure spanning tree status %s but get status to be %s\n", setSpanningTreeEnabled==TRUE? "TRUE": "FALSE", getSpanningTreeEnabled==TRUE? "TRUE": "FALSE");
			return FAILED;
		}
	}
//If phy read have some trouble, this may failed
#if 0 	
	for(i=0; i<RTL8651_PORT_NUMBER<<2; i++) {
		int8 getTruth, getDuplex;
		uint32 getValue, getSpeed;
		
		rtl8651_setAsicEthernetLinkStatus(i%RTL8651_PORT_NUMBER, i%2==1 ? TRUE: FALSE);
		rtl8651_setAsicEthernetPHY(i%RTL8651_PORT_NUMBER, i%2==1 ? TRUE: FALSE, i%4,  i%2, i%2==1? TRUE: FALSE);
	
		rtl8651_getAsicEthernetLinkStatus(i%RTL8651_PORT_NUMBER, &getTruth);
		if(getTruth != (i%2==1 ? TRUE: FALSE)) {
			printf("Error! Configure port %d link status %s but get to be %s\n", i%RTL8651_PORT_NUMBER, i%2==1 ? "TRUE": "FALSE", getTruth==TRUE? "TRUE": "FALSE");
			return FAILED;
		}
		rtl8651_getAsicEthernetPHY(i%RTL8651_PORT_NUMBER, &getTruth, &getValue, &getSpeed, &getDuplex);
		if(getDuplex != (i%2==1 ? TRUE: FALSE)) {
			printf("Error! Configure port %d duplex mode %s but get to be %s\n", i%RTL8651_PORT_NUMBER, i%2==1 ? "TRUE": "FALSE", getDuplex==TRUE? "TRUE": "FALSE");
			return FAILED;
		}
		if(getSpeed != i%2) {
			printf("Error! Configure port %d link speed %d but get to be %d\n", i%RTL8651_PORT_NUMBER, i%2, getSpeed);
			return FAILED;
		}
		if(getTruth != (i%2==1 ? TRUE: FALSE)) {
			printf("Error! Configure port %d auto negotiation %s but get to be %s\n", i%RTL8651_PORT_NUMBER, i%2==1 ? "TRUE": "FALSE", getTruth==TRUE? "TRUE": "FALSE");
			return FAILED;
		}
		if(getValue != i%4) {
			printf("Error! Configure port advertised capability %d but get to be %d\n", i%4, getValue);
			return FAILED;
		}
	}
#endif
	for(i=0; i<RTL8651_PORT_NUMBER<<4; i++) {
		uint32 setValue, getValue;
		setValue = i%8;
		rtl8651_setAsicEthernetBandwidthControl(i%RTL8651_PORT_NUMBER, (i/RTL8651_PORT_NUMBER)%2==1? TRUE: FALSE, setValue);//RTL8651_ASICBC_xxx
		rtl8651_getAsicEthernetBandwidthControl(i%RTL8651_PORT_NUMBER, (i/RTL8651_PORT_NUMBER)%2==1? TRUE: FALSE, &getValue);//RTL8651_ASICBC_xxx
		if(setValue != getValue) {
			printf("Error! Configure port %d %s with bandwidth parameter %d but get to be %d\n", i%RTL8651_PORT_NUMBER, (i/RTL8651_PORT_NUMBER)%2==1? "INPUT": "OUTPUT", setValue, getValue);
			return FAILED;
		}
	}
	for(i=0; i<RTL8651_L2TBL_ROW; i++)
		for(j=0; j<RTL8651_L2TBL_COLUMN; j++) {
			ether_addr_t macAddr;
			macAddr.octet[0] = j<<6;
			macAddr.octet[1] = j<<4;
			macAddr.octet[2] = j<<2;
			macAddr.octet[3] = j;
			macAddr.octet[4] = i;
			macAddr.octet[5] = (j<<6) ^ (j<<4) ^ (j<<2) ^ j;
			if(rtl8651_setAsicL2Table(i, j, &macAddr, (i%2)==1? TRUE: FALSE, ((i+1)%2)==1? TRUE: FALSE, 1<<(i%6), 300, TRUE, 0) == FAILED) {
				printf("Unable to configure Row %d Column %d\n", i, j);
				return FAILED;
			}
		}
	for(i=0; i<RTL8651_L2TBL_ROW; i++)
		for(j=0; j<RTL8651_L2TBL_COLUMN; j++) {
			ether_addr_t macAddr;
			int8 isCpu, isSrcBlk, isStatic; 
			uint32 mbr, ageSec;
			if(rtl8651_getAsicL2Table(i, j, &macAddr, &isCpu, &isSrcBlk, &isStatic, &mbr, &ageSec, NULL) == FAILED) {
				printf("Error!Unable to get row %d column %d\n", i, j);
				return FAILED;
			}
 			if(macAddr.octet[0] != j<<6 || macAddr.octet[1] != j<<4 || macAddr.octet[2] != j<<2 || macAddr.octet[3] != j || macAddr.octet[4] != i || macAddr.octet[5] != ((j<<6) ^ (j<<4) ^ (j<<2) ^ j)) {
				printf("Error! MAC address configure %02x:%02x:%02x:%02x:%02x:%02x but get to be %02x:%02x:%02x:%02x:%02x:%02x \n",  
					j<<6, j<<4, j<<2, j, i, (j<<6) ^(j<<4)^(j<<2)^ j, macAddr.octet[0], macAddr.octet[1], macAddr.octet[2], macAddr.octet[3], macAddr.octet[4], macAddr.octet[5]);
				return FAILED;
 			}
			if(isCpu != ((i%2)==1? TRUE: FALSE) || isSrcBlk != (((i+1)%2)==1? TRUE: FALSE) || mbr != (1<<(i%6))) {
				printf("Error! Layer2 table content mismatch at row %d column %d. Configure to CPU %s source block %s Static and member port %x but get to be CPU %s source block %s and member port %x\n", 
					i, j, (i%2)==1? "TRUE": "FALSE", ((i+1)%2)==1? "TRUE": "FALSE", 1<<(i%6), isCpu==TRUE? "TRUE": "FALSE", isSrcBlk==TRUE? "TRUE": "FALSE", mbr);
				return FAILED;
			}
		}
	for(i=0; i<RTL8651_L2TBL_ROW; i++)
		for(j=0; j<RTL8651_L2TBL_COLUMN; j++) {
			ether_addr_t macAddr;
			int8 isCpu, isSrcBlk, isStatic; 
			uint32 mbr, ageSec;
			if(rtl8651_delAsicL2Table(i, j) != SUCCESS) {
				printf("Error! Previous installed entry row %d column %d cannot delete\n", i, j);
				return FAILED;
			}
			if(rtl8651_getAsicL2Table(i, j, &macAddr, &isCpu, &isSrcBlk, &isStatic, &mbr, &ageSec, NULL) == SUCCESS && isStatic == TRUE) {
				printf("Error!Should not to get row %d column %d because entry has deleted\n", i, j);
				return FAILED;
			}
		}
	for(i=0; i<(1<<(RTL8651_PORT_NUMBER+1))-1; i++) {
		uint32 setPortMask, getPortMask, mapping;
		int8 selectPort[RTL8651_PORT_NUMBER];

		setPortMask = i < 1<<RTL8651_PORT_NUMBER? i: (1<<(RTL8651_PORT_NUMBER+1)) -i -2;
		rtl8651_setAsicLinkAggregator(setPortMask);
		for(j=0; j<RTL8651_PORT_NUMBER; j++)
			rtl8651_setAsicEthernetLinkStatus(j, TRUE);
		rtl8651_getAsicLinkAggregator(&getPortMask, &mapping);
		if(setPortMask != getPortMask) {
			printf("Error! Configured link aggregation port mask %x but get to be %x\n", setPortMask, getPortMask);
			return FAILED;
		}
		if(setPortMask != 0) {
			uint32 maxUsed, minUsed;
			for(j=0; j<RTL8651_PORT_NUMBER; j++)
				selectPort[j] = 0;
			for(j=0; j<8; j++)
				selectPort[(mapping>>(j*3))&0x7]++;
			for(j=0; j<RTL8651_PORT_NUMBER; j++)
				if(((setPortMask & (1<<j)) && selectPort[j]== 0) || ( (setPortMask&(1<<j)) == 0 && selectPort[j] > 0)) {
					printf("Error! Configure link aggregator %s port %d but %s in mapping list\n", (setPortMask & (1<<j))? "with": "without", j, selectPort[j]>0? "appear": "not");
					return FAILED;
				}
			for(j=0, maxUsed = 0, minUsed = 8; j<RTL8651_PORT_NUMBER; j++)
				if(setPortMask & (1<<j)) {//Member port
					if(selectPort[j]>maxUsed)
						maxUsed = selectPort[j];
					if(selectPort[j]<minUsed)
						minUsed = selectPort[j];
				}
			assert(minUsed<=maxUsed);
			if(maxUsed - minUsed > 1) {
				printf("Arrangement not proper at port mask %x Maximum used %d Minimum used %d\n", setPortMask, maxUsed, minUsed);
				return FAILED;
			}
		}
		else if (mapping != 0) {
			printf("No port in link aggregation should have zero mapping but %x\n", mapping);
			return FAILED;
		}
		for(j=0; j<RTL8651_PORT_NUMBER; j++)
			rtl8651_setAsicEthernetLinkStatus(j, FALSE);
		rtl8651_getAsicLinkAggregator(&getPortMask, &mapping);
		if(setPortMask != getPortMask) {
			printf("Error! Configured link aggregation port mask %x but get to be %x\n", setPortMask, getPortMask);
			return FAILED;
		}
		if(mapping != 0) {
			printf("Error! All link down should cause mapping register to be zero but get to be %x\n", mapping);
			return FAILED;
		}

	}
	for(i=0; i<RTL8651_VLAN_NUMBER; i++) {
		ether_addr_t macAddr;
		int8 portState[RTL8651_PORT_NUMBER];
		for(j=0;j<6;j++)
			macAddr.octet[j] = i<<(j%4);
		for(j=0;j<RTL8651_PORT_NUMBER;j++)
			portState[j] = (i+j)%2==1? TRUE: FALSE;
		if(rtl8651_setAsicVlan(i, &macAddr, 1<<(i%RTL8651_PORT_NUMBER), i<<2, (i<<2) + 7, (i<<2) + 8, (i<<2) + 15, i%2==1? TRUE: FALSE, i%2==1? FALSE: TRUE, portState, i%2==1? TRUE: FALSE, i%2==1? FALSE: TRUE, 1<<(i%RTL8651_PORT_NUMBER), 1<<(i%4), (i<<7)+512) == FAILED) {
			printf("Error! Set vlan %d failed\n", i);
			return FAILED;
		}
	}
	for(i=0; i<RTL8651_VLAN_NUMBER; i++) {
		ether_addr_t macAddr;
		int8 portState[RTL8651_PORT_NUMBER], internalIntf, enableRoute, broadcastToCpu, promiscuous;
		uint32 mbr, inAclStart, inAclEnd, outAclStart, outAclEnd, untagPortMask, macNumber, mtu;
		if(rtl8651_getAsicVlan(i, &macAddr, &mbr, &inAclStart, &inAclEnd, &outAclStart, &outAclEnd, &internalIntf, &enableRoute, portState, &broadcastToCpu, &promiscuous, &untagPortMask, &macNumber, &mtu) == FAILED) {
			printf("Error get installed vlan %d failed\n", i);
			return FAILED;
		}
		for(j=0;j<6;j++)
			if(macAddr.octet[j] != (i<<(j%4))) {
				printf("Error! VLAN %d %dth MAC octet set to be %x but get to be %x\n", i, j, i<<(j%4), macAddr.octet[j]);
				return FAILED;
			}
		for(j=0;j<RTL8651_PORT_NUMBER;j++)
			if(portState[j] != ((i+j)%2==1? TRUE: FALSE)) {
				printf("ERror! VLAN %d port %d state configure to be %s but get %s\n", i, j, (i+j)%2==1? "TRUE": "FALSE", portState[j]==TRUE? "TRUE": "FALSE");
				return FAILED;
			}
		if(mbr != 1<<(i%RTL8651_PORT_NUMBER) || inAclStart!=i<<2 || inAclEnd!=(i<<2) +7||outAclStart!=(i<<2) + 8||outAclEnd!=(i<<2) + 15||internalIntf != (i%2==1? TRUE: FALSE)||enableRoute!=(i%2==1? FALSE: TRUE)) {
			printf("Error! Configure VLAN %d member port %x inAclStart %u inAclEnd %u outAclStart %u outAclEnd %u Internal interface %s enable route %s but get to be member port %x inAclStart %u inAclEnd %u outAclStart %u outAclEnd %u Internal interface %s enable route %s\n", 
				i, 1<<(i%RTL8651_PORT_NUMBER), i<<2, (i<<2) +7, (i<<2) + 8, (i<<2) + 15, i%2==1? "TRUE": "FALSE", i%2==1? "FALSE": "TRUE", mbr, inAclStart, inAclEnd, outAclStart, outAclEnd, internalIntf==TRUE? "TRUE": "FALSE", enableRoute==TRUE? "TRUE": "FALSE");
			return FAILED;
		}
		if(broadcastToCpu!=(i%2==1? TRUE: FALSE)||promiscuous!=(i%2==1? FALSE: TRUE)||untagPortMask!=1<<(i%RTL8651_PORT_NUMBER)||macNumber!=1<<(i%4)||mtu!= (i<<7)+512) {
			printf("Error! Configure VLAN %d broadcast to cpu %s promiscuous mode %s untag port %x mac number %d mtu %d but get value is broadcast to cpu %s promiscuous mode %s untag port %x mac number %d mtu %d\n",
				i, i%2==1? "TRUE": "FALSE", i%2==1? "FALSE": "TRUE", 1<<(i%RTL8651_PORT_NUMBER), 1<<(i%4), (i<<7)+512, broadcastToCpu==TRUE? "TRUE": "FALSE", promiscuous==TRUE? "TRUE": "FALSE", untagPortMask, macNumber, mtu);
			return FAILED;
		}			
	}
	for(i=0; i<RTL8651_VLAN_NUMBER; i++) {
		for(j=0; j<RTL8651_PORT_NUMBER; j++) {
			if(rtl8651_setAsicPvid(j, (j+i)%RTL8651_VLAN_NUMBER) == FAILED) {
				printf("Error! Set port %d pvid %d failed\n", j,  (j+i)%RTL8651_VLAN_NUMBER);
				return FAILED;
			}
		}
		for(j=0; j<RTL8651_PORT_NUMBER; j++) {
			uint32 pvidx;
			rtl8651_getAsicPvid(j, &pvidx);
			if(pvidx != (j+i)%RTL8651_VLAN_NUMBER) {
				printf("Error! Configured port %d pvid %d but get to be %d\n", j, (j+i)%RTL8651_VLAN_NUMBER, pvidx);
				return FAILED;
			}
		}
	}
	for(i=0; i<RTL8651_VLAN_NUMBER; i++) {
		ether_addr_t macAddr;
		int8 portState[RTL8651_PORT_NUMBER], internalIntf, enableRoute, broadcastToCpu, promiscuous;
		uint32 mbr, inAclStart, inAclEnd, outAclStart, outAclEnd, untagPortMask, macNumber, mtu;
		if(rtl8651_delAsicVlan(i) !=SUCCESS) {
			printf("Error!(%d) Delete vlan %d failed\n", __LINE__, i);
			return FAILED;
		}
		if(rtl8651_getAsicVlan(i, &macAddr, &mbr, &inAclStart, &inAclEnd, &outAclStart, &outAclEnd, &internalIntf, &enableRoute, portState, &broadcastToCpu, &promiscuous, &untagPortMask, &macNumber, &mtu) == SUCCESS) {
			printf("Error!(%d) Get deleted vlan %d success\n", __LINE__, i);
			return FAILED;
		}
	}
	for(i=0; i<RTL8651_PPPOE_NUMBER; i++) {
		for(j=0; j<RTL8651_PPPOE_NUMBER; j++)
			rtl8651_setAsicPppoe(j, i+j);
		for(j=0; j<RTL8651_PPPOE_NUMBER; j++) {
			uint16 sessionId;
			rtl8651_getAsicPppoe(j, &sessionId);
			if(sessionId != i+j) {
				printf("Error! Configure pppoe session %d session ID %d but get value is %d\n", j, i+j, sessionId);
				return FAILED;
			}
		}
	}
	for(i=0; i<RTL8651_ROUTINGTBL_SIZE; i++) {//Test routing table pppoe format
		ipaddr_t ipAddr, ipMask;
		uint32 process, vidx, arpStart, arpEnd, nextHopRow, nextHopColumn, pppoeIdx;
		for(j=0; j<RTL8651_ROUTINGTBL_SIZE; j++)
			if(rtl8651_setAsicRouting(j, i+j, 0xffffffff<<i, 0, (i+j)%RTL8651_VLAN_NUMBER, 0, 0, (i+j)%RTL8651_L2TBL_ROW, (i+j)%RTL8651_L2TBL_COLUMN, (i*j)%RTL8651_PPPOETBL_SIZE) == FAILED) {
				printf("Error! Add routing entry %d failed\n", j);
				return FAILED;
			}
		for(j=0; j<RTL8651_ROUTINGTBL_SIZE; j++) {
			if(rtl8651_getAsicRouting(j, &ipAddr, &ipMask, &process, &vidx, &arpStart, &arpEnd, &nextHopRow, &nextHopColumn, &pppoeIdx) == FAILED) {
				printf("Error! Get routing entry %d failed\n", j);
				return FAILED;
			}
			if(ipAddr != i+j || ipMask != 0xffffffff<<i || process != 0 || vidx != (i+j)%RTL8651_VLAN_NUMBER || nextHopRow != (i+j)%RTL8651_L2TBL_ROW || nextHopColumn != (i+j)%RTL8651_L2TBL_COLUMN || pppoeIdx != (i*j)%RTL8651_PPPOETBL_SIZE) {
				printf("Error! Configure routing entry %d content mismatch (%x, %x, %d, %d, %d, %d, %d) <> (%x, %x, %d, %d, %d, %d, %d)\n", 
					j, i+j, 0xffffffff<<i, 0, (i+j)%RTL8651_VLAN_NUMBER, (i+j)%RTL8651_L2TBL_ROW, (i+j)%RTL8651_L2TBL_COLUMN, (i*j)%RTL8651_PPPOETBL_SIZE, 
					ipAddr, ipMask, process, vidx, nextHopRow, nextHopColumn, pppoeIdx);
				return FAILED;
			}
			if(rtl8651_delAsicRouting(j) !=SUCCESS) {
				printf("Error! Unable to delete routing entry %d\n", j);
				return FAILED;
			}
			if(rtl8651_getAsicRouting(j, &ipAddr, &ipMask, &process, &vidx, &arpStart, &arpEnd, &nextHopRow, &nextHopColumn, &pppoeIdx) == SUCCESS) {
				printf("Error! Get deleted routing entry %d success\n", j);
				return FAILED;
			}
		}

	}
	for(i=0; i<RTL8651_ROUTINGTBL_SIZE; i++) {//Test routing table direct format
		ipaddr_t ipAddr, ipMask;
		uint32 process, vidx, arpStart, arpEnd, nextHopRow, nextHopColumn, pppoeIdx;
		for(j=0; j<RTL8651_ROUTINGTBL_SIZE; j++)
			if(rtl8651_setAsicRouting(j, i+j, 0xffffffff<<i, 1, (i+j)%RTL8651_VLAN_NUMBER, 0, 0, (i+j)%RTL8651_L2TBL_ROW, (i+j)%RTL8651_L2TBL_COLUMN, 0) == FAILED) {
				printf("Error!(%d) Add routing entry %d failed\n", __LINE__, j);
				return FAILED;
			}
		for(j=0; j<RTL8651_ROUTINGTBL_SIZE; j++) {
			if(rtl8651_getAsicRouting(j, &ipAddr, &ipMask, &process, &vidx, &arpStart, &arpEnd, &nextHopRow, &nextHopColumn, &pppoeIdx) == FAILED) {
				printf("Error!(%d) Get routing entry %d failed\n", __LINE__, j);
				return FAILED;
			}
			if(ipAddr != i+j || ipMask != 0xffffffff<<i || process != 1 || vidx != (i+j)%RTL8651_VLAN_NUMBER || nextHopRow != (i+j)%RTL8651_L2TBL_ROW || nextHopColumn != (i+j)%RTL8651_L2TBL_COLUMN) {
				printf("Error!(%d) Configure routing entry %d content mismatch (%x, %x, %d, %d, %d, %d) <> (%x, %x, %d, %d, %d, %d)\n", 
					__LINE__, j, i+j, 0xffffffff<<i, 1, (i+j)%RTL8651_VLAN_NUMBER, (i+j)%RTL8651_L2TBL_ROW, (i+j)%RTL8651_L2TBL_COLUMN, 
					ipAddr, ipMask, process, vidx, nextHopRow, nextHopColumn);
				return FAILED;
			}
			if(rtl8651_delAsicRouting(j) !=SUCCESS) {
				printf("Error!(%d) Unable to delete routing entry %d\n", __LINE__, j);
				return FAILED;
			}
			if(rtl8651_getAsicRouting(j, &ipAddr, &ipMask, &process, &vidx, &arpStart, &arpEnd, &nextHopRow, &nextHopColumn, &pppoeIdx) == SUCCESS) {
				printf("Error!(%d) Get deleted routing entry %d success\n", __LINE__, j);
				return FAILED;
			}
		}
	}
	for(i=0; i<RTL8651_ROUTINGTBL_SIZE; i++) {//Test routing table indirect format
		ipaddr_t ipAddr, ipMask;
		uint32 process, vidx, arpStart, arpEnd, nextHopRow, nextHopColumn, pppoeIdx;
		for(j=0; j<RTL8651_ROUTINGTBL_SIZE; j++)
			if(rtl8651_setAsicRouting(j, i+j, 0xffffffff<<i, 2, (i+j)%RTL8651_VLAN_NUMBER, (i+j)%RTL8651_ARPTBL_SIZE, (i*j)%RTL8651_ARPTBL_SIZE, 0, 0, 0) == FAILED) {
				printf("Error!(%d) Add routing entry %d failed\n", __LINE__, j);
				return FAILED;
			}
		for(j=0; j<RTL8651_ROUTINGTBL_SIZE; j++) {
			if(rtl8651_getAsicRouting(j, &ipAddr, &ipMask, &process, &vidx, &arpStart, &arpEnd, &nextHopRow, &nextHopColumn, &pppoeIdx) == FAILED) {
				printf("Error!(%d) Get routing entry %d failed\n", __LINE__, j);
				return FAILED;
			}
			if(ipAddr != i+j || ipMask != 0xffffffff<<i || process != 2 || vidx != (i+j)%RTL8651_VLAN_NUMBER || arpStart != (i+j)%RTL8651_ARPTBL_SIZE || arpEnd != (i*j)%RTL8651_ARPTBL_SIZE) {
				printf("Error!(%d) Configure routing entry %d content mismatch (%x, %x, %d, %d, %d, %d) <> (%x, %x, %d, %d, %d, %d)\n", 
					__LINE__, j, i+j, 0xffffffff<<i, 2, (i+j)%RTL8651_VLAN_NUMBER, arpStart != (i+j)%RTL8651_ARPTBL_SIZE, (i*j)%RTL8651_ARPTBL_SIZE, 
					ipAddr, ipMask, process, vidx, arpStart, arpEnd);
				return FAILED;
			}
			if(rtl8651_delAsicRouting(j) !=SUCCESS) {
				printf("Error!(%d) Unable to delete routing entry %d\n", __LINE__, j);
				return FAILED;
			}
			if(rtl8651_getAsicRouting(j, &ipAddr, &ipMask, &process, &vidx, &arpStart, &arpEnd, &nextHopRow, &nextHopColumn, &pppoeIdx) == SUCCESS) {
				printf("Error!(%d) Get deleted routing entry %d success\n", __LINE__, j);
				return FAILED;
			}
		}
	}
	for(i=0; i<RTL8651_ROUTINGTBL_SIZE; i++) {//Test routing table CPU format
		ipaddr_t ipAddr, ipMask;
		uint32 process, vidx, arpStart, arpEnd, nextHopRow, nextHopColumn, pppoeIdx;
		for(j=0; j<RTL8651_ROUTINGTBL_SIZE; j++)
			if(rtl8651_setAsicRouting(j, i+j, 0xffffffff<<i, 4, 0, 0, 0, 0, 0, 0) == FAILED) {
				printf("Error!(%d) Add routing entry %d failed\n", __LINE__, j);
				return FAILED;
			}
		for(j=0; j<RTL8651_ROUTINGTBL_SIZE; j++) {
			if(rtl8651_getAsicRouting(j, &ipAddr, &ipMask, &process, &vidx, &arpStart, &arpEnd, &nextHopRow, &nextHopColumn, &pppoeIdx) == FAILED) {
				printf("Error!(%d) Get routing entry %d failed\n", __LINE__, j);
				return FAILED;
			}
			if(ipAddr != i+j || ipMask != 0xffffffff<<i || process != 4) {
				printf("Error!(%d) Configure routing entry %d content mismatch (%x, %x, %d) <> (%x, %x, %d)\n", 
					__LINE__, j, i+j, 0xffffffff<<i, 4, ipAddr, ipMask, process);
				return FAILED;
			}
			if(rtl8651_delAsicRouting(j) !=SUCCESS) {
				printf("Error!(%d) Unable to delete routing entry %d\n", __LINE__, j);
				return FAILED;
			}
			if(rtl8651_getAsicRouting(j, &ipAddr, &ipMask, &process, &vidx, &arpStart, &arpEnd, &nextHopRow, &nextHopColumn, &pppoeIdx) == SUCCESS) {
				printf("Error!(%d) Get deleted routing entry %d success\n", __LINE__, j);
				return FAILED;
			}
		}
	}
	for(i=0; i<4; i++) {
		uint32 nextHopRow, nextHopColumn;
		for(j=0; j<RTL8651_ARPTBL_SIZE; j++)
			if(rtl8651_setAsicArp(j, (i+j)%RTL8651_L2TBL_ROW, (i+j)%RTL8651_L2TBL_COLUMN)== FAILED) {
				printf("Error!(%d) Unable to write %dth arp entry\n", __LINE__, j);
				return FAILED;
			}
		for(j=0; j<RTL8651_ARPTBL_SIZE; j++) {
			if(rtl8651_getAsicArp(j, &nextHopColumn, &nextHopRow) == FAILED) {
				printf("Error!(%d) Unable to get %dth arp entry\n", __LINE__, j);
				return FAILED;
			}
			if(nextHopColumn != (i+j)%RTL8651_L2TBL_ROW || nextHopRow != (i+j)%RTL8651_L2TBL_COLUMN) {
				printf("Error!(%d) Configure arp entry %d with row %d column %d but stored value is row %d column %d\n", __LINE__, j, (i+j)%RTL8651_L2TBL_ROW, (i+j)%RTL8651_L2TBL_COLUMN, nextHopRow, nextHopColumn);
				return FAILED;
			}
			if(rtl8651_delAsicArp(j)  !=SUCCESS) {
				printf("Error!(%d) Installed ARP entry %d unable to delete\n", __LINE__, j);
				return FAILED;
			}
			if(rtl8651_getAsicArp(j, &nextHopColumn, &nextHopRow) == SUCCESS) {
				printf("Error!(%d) Deleted entry %dth arp entry able to get\n", __LINE__, j);
				return FAILED;
			}
		}
	}
	for(i=0; i<8; i++) {
		uint32 setRegValue, getRegValue;
		setRegValue = 0;
		for(j=0; j<8; j++)
			setRegValue |= ((i+j)%8)<<(j*3);
		if(rtl8651_setAsicGidxRegister(setRegValue) !=SUCCESS || rtl8651_getAsicGidxRegister(&getRegValue)  !=SUCCESS) {
			printf("Error!(%d) Configure or get gidx register failed\n", __LINE__);
			return FAILED;
		}
		if(setRegValue != getRegValue) {
			printf("Error!(%d) Set gidx %x but stored value is %x\n", __LINE__, setRegValue, getRegValue);
			return FAILED;
		}
	}
	for(i=0; i<RTL8651_IPTABLE_SIZE; i++) {
		ipaddr_t extIpAddr, intIpAddr;
		int8 localPublic, nat;
		for(j=0; j<RTL8651_IPTABLE_SIZE; j++)
			if(rtl8651_setAsicExtIntIpTable(j, i+j+1, (i+j)&0x1? i+j+5: 0, (i+j)&0x2? TRUE: FALSE, (i+j)&0x1? TRUE: FALSE) == (((i+j)&0x3)==0x3? SUCCESS: FAILED)) {
				printf("Error!(%d) Unable to set external IP table entry %d\n", __LINE__, j);
				return FAILED;
			}
		
		for(j=0; j<RTL8651_IPTABLE_SIZE; j++) {	
			if(((i+j)&0x3)!=0x3) {
				if(rtl8651_getAsicExtIntIpTable(j, &extIpAddr, &intIpAddr, &localPublic, &nat) == FAILED) {
					printf("Error!(%d) Unable to get configured external IP table entry %d\n", __LINE__, j);
					return FAILED;
				}
				if(extIpAddr != i+j+1 || intIpAddr != ((i+j)&0x1? i+j+5: 0) || localPublic != ((i+j)&0x2? TRUE: FALSE) || nat != ((i+j)&0x1? TRUE: FALSE)) {
					printf("Error!(%d) Configured entry %d parameter different from stored value (%d %d %s %s) <> (%d %d %s %s)\n", __LINE__, j, i+j+1, (i+j)&0x1? i+j+5: 0, (i+j)&0x2? "TRUE": "FALSE",  (i+j)&0x1? "TRUE": "FALSE", extIpAddr, intIpAddr, localPublic==TRUE? "TRUE": "FALSE", nat==TRUE? "TRUE": "FALSE");
					return FAILED;
				}
				if(rtl8651_delAsicExtIntIpTable(j)  !=SUCCESS) {
					printf("Error!(%d) Unable to delete configured entry %d\n", __LINE__, j);
					return FAILED;
				}
			}
			if(rtl8651_getAsicExtIntIpTable(j, &extIpAddr, &intIpAddr, &localPublic, &nat) == SUCCESS) {
				printf("Error!(%d) Able to get deleted configured external IP table entry %d\n", __LINE__, j);
				return FAILED;
			}
		}
	}
	for(i=0; i< RTL8651_SERVERPORTTBL_SIZE; i++) {
		ipaddr_t extIpAddr, intIpAddr;
		uint16 extPort, intPort;
		for(j=0; j< RTL8651_SERVERPORTTBL_SIZE; j++)
			if(rtl8651_setAsicServerPortTable(j, i+j+1, i+j+2, i+j+3, i+j+4) == FAILED) {
				printf("Error!(%d) Unable to add server port table entry %d\n", __LINE__, j);
				return FAILED;
			}
		for(j=0; j<RTL8651_SERVERPORTTBL_SIZE; j++) {
			if(rtl8651_getAsicServerPortTable(j, &extIpAddr, &extPort, &intIpAddr, &intPort) == FAILED) {
				printf("Error!(%d) Unable to get configured server port entry %d\n", __LINE__, j);
				return FAILED;
			}
			if(extIpAddr != i+j+1 || extPort != i+j+2 || intIpAddr != i+j+3 || intPort != i+j+4) {
				printf("Error!(%d) Server port entry %d configured value different from stored value (%d %d %d %d) <> (%d %d %d %d)\n", __LINE__, j, i+j+1, i+j+2, i+j+3, i+j+4, extIpAddr, extPort, intIpAddr, intPort);
				return FAILED;
			}
			if(rtl8651_delAsicServerPortTable(j) !=SUCCESS) {
				printf("Error!(%d) Server port entry %d unable to delete\n", __LINE__, j);
				return FAILED;
			}
			if(rtl8651_getAsicServerPortTable(j, &extIpAddr, &extPort, &intIpAddr, &intPort) == SUCCESS) {
				printf("Error!(%d) Able to get deleted server port entry %d\n", __LINE__, j);
				return FAILED;
			}
		}
	}
	for(i=0; i<4; i++) {
		int8 l2Enable, l4Enable;
		if(rtl8651_setAsicAgingFunction(i&0x1? TRUE: FALSE, i%0x2? TRUE: FALSE) == FAILED || rtl8651_getAsicAgingFunction(&l2Enable, &l4Enable) == FAILED) {
			printf("Error!(%d) aging function set or get failed\n", __LINE__);
			return FAILED;
		}
		if(l2Enable != (i&0x1? TRUE: FALSE) || l4Enable != (i%0x2? TRUE: FALSE)) {
			printf("Error!(%d) Configured and get value mismatch. Configure L2Enable %s L4Enable %s Get L2Enable %s L4Enable %s\n", __LINE__, i&0x1? "TRUE": "FALSE", i%0x2? "TRUE": "FALSE", l2Enable==TRUE? "TRUE": "FALSE", l4Enable==TRUE? "TRUE": "FALSE");
			return FAILED;
		}
	}
	for(i=0; i<64; i++) {
		uint32 timeout;
		rtl8651_setAsicNaptIcmpTimeout(_rtl8651_NaptAgingToSec(i));
		rtl8651_setAsicNaptUdpTimeout(_rtl8651_NaptAgingToSec((i+1)%64));
		rtl8651_setAsicNaptTcpLongTimeout(_rtl8651_NaptAgingToSec((i+2)%64));
		rtl8651_setAsicNaptTcpMediumTimeout(_rtl8651_NaptAgingToSec((i+3)%64));
		rtl8651_setAsicNaptTcpFastTimeout(_rtl8651_NaptAgingToSec((i+4)%64));
		timeout = (i+5)%64;
		rtl8651_getAsicNaptIcmpTimeout(&timeout);
		if(timeout != _rtl8651_NaptAgingToSec(i)) {
			printf("Error!(%d) Get ICMP timeout value %d different from configured %d\n", __LINE__, timeout, _rtl8651_NaptAgingToSec(i));
			return FAILED;
		}
		rtl8651_getAsicNaptUdpTimeout(&timeout);
		if(timeout != _rtl8651_NaptAgingToSec((i+1)%64)) {
			printf("Error!(%d) Get UDP timeout value %d different from configured %d\n", __LINE__, timeout, _rtl8651_NaptAgingToSec((i+1)%64));
			return FAILED;
		}
		rtl8651_getAsicNaptTcpLongTimeout(&timeout);
		if(timeout != _rtl8651_NaptAgingToSec((i+2)%64)) {
			printf("Error!(%d) Get TCP long timeout value %d different from configured %d\n", __LINE__, timeout, _rtl8651_NaptAgingToSec((i+2)%64));
			return FAILED;
		}
		rtl8651_getAsicNaptTcpMediumTimeout(&timeout);
		if(timeout != _rtl8651_NaptAgingToSec((i+3)%64)) {
			printf("Error!(%d) Get TCP medium timeout value %d different from configured %d\n", __LINE__, timeout, _rtl8651_NaptAgingToSec((i+3)%64));
			return FAILED;
		}
		rtl8651_getAsicNaptTcpFastTimeout(&timeout);
		if(timeout != _rtl8651_NaptAgingToSec((i+4)%64)) {
			printf("Error!(%d) Get TCP fast timeout value %d different from configured %d\n", __LINE__, timeout, _rtl8651_NaptAgingToSec((i+4)%64));
			return FAILED;
		}
	}
	for(i=0; i<4; i++) {
		for(j=0; j<1024; j++)
			if(rtl8651_setAsicNaptTcpUdpTable(FALSE, j, j<<2, i<<10 | j, _rtl8651_NaptAgingToSec((i+j)%32), (i+j)%2==0? TRUE: FALSE, (i+j+1)%2==0? TRUE: FALSE, i, (i+j)%2==0? TRUE: FALSE, ((i+j)>>1)%2==0? TRUE: FALSE) == FAILED) {
				printf("Error!(%d) Unable to configure global port %d\n", __LINE__, i<<10 | j);
				return FAILED;
			}
		for(j=0; j<1024; j++) {
			ipaddr_t insideLocalIpAddr;
			uint16 insideLocalPort, insideGlobalPort, ageSec;
			int8 isStatic, isTcp, isCollision, isValid, tcpFlag;
			if(rtl8651_getAsicNaptTcpUdpTable(TRUE, i<<10 | j, &insideLocalIpAddr, &insideLocalPort, &insideGlobalPort, &ageSec, &isStatic, &isTcp, &tcpFlag, &isCollision, &isValid) == FAILED) {
				printf("Error!(%d) Unable to get global port %d\n", __LINE__, i<<10 | j);
				return FAILED;
			}
			if(insideLocalIpAddr != j || insideLocalPort != j<<2 || ageSec != _rtl8651_NaptAgingToSec((i+j)%32) || isStatic != ((i+j)%2==0? TRUE: FALSE) || isTcp != ((i+j+1)%2==0? TRUE: FALSE) || isCollision != ((i+j)%2==0? TRUE: FALSE) || isValid != (((i+j)>>1)%2==0? TRUE: FALSE)) {
				printf("Error!(%d) Configured ang stored parameter mismatch (%d %d %d %s %s %s %s) <> (%d %d %d %s %s %s %s)\n", __LINE__, j, j<<2, _rtl8651_NaptAgingToSec((i+j)%64), (i+j)%2==0? "TRUE": "FALSE", (i+j+1)%2==0? "TRUE": "FALSE", (i+j)%2==0? "TRUE": "FALSE", "TRUE", insideLocalIpAddr, insideLocalPort, ageSec, isStatic==TRUE? "TRUE": "FALSE", isTcp==TRUE ? "TRUE": "FALSE", isCollision==TRUE? "TRUE": "FALSE", isValid==TRUE? "TRUE": "FALSE");
				return FAILED;
			}
		}
		for(j=0; j<1024; j++) {
			uint16 offset;
			int8 isValid;
			rtl8651_getAsicNaptTcpUdpOffset(j, &offset, &isValid);
			if(offset != i || isValid != (((i+j)>>1)%2==0? TRUE: FALSE)) {
				printf("Error!(%d) Get offset get information (Offset:%d entry %s) different from configured  (Offset:%d entry Valid)\n", __LINE__, offset, isValid==TRUE?"Valid": "Invalid", i);
				return FAILED;
			}
		}
		for(j=0; j<1024; j++)
			if(rtl8651_setAsicNaptTcpUdpTable(FALSE, j, j<<2, i<<10 | j, _rtl8651_NaptAgingToSec((i+j)%32), (i+j)%2==0? TRUE: FALSE, (i+j+1)%2==0? TRUE: FALSE, i, (i+j)%2==0? TRUE: FALSE, TRUE) == (((i+j)>>1)%2==0? SUCCESS: FAILED)) {
				printf("Error!(%d) Occupied entry %d can normal write\n", __LINE__, j);
				return FAILED;
			}
		for(j=0; j<1024; j++)
			if(rtl8651_setAsicNaptTcpUdpTable(TRUE, 0, 0,  i<<10 | j, 0, FALSE, FALSE, 0, FALSE, FALSE) == FAILED) {
				printf("Error!(%d) Occupied entry %d cannot force write\n", __LINE__,  i<<10 | j);
				return FAILED;
			}
		for(j=0; j<1024; j++) {
			ipaddr_t insideLocalIpAddr;
			uint16 insideLocalPort, insideGlobalPort, ageSec;
			int8 isStatic, isTcp, isCollision, isValid, tcpFlag;
			if(rtl8651_getAsicNaptTcpUdpTable(TRUE, i<<10 | j, &insideLocalIpAddr, &insideLocalPort, &insideGlobalPort, &ageSec, &isStatic, &isTcp, &tcpFlag, &isCollision, &isValid) == FAILED) {
				printf("Error!(%d) Unable to get global port %d\n", __LINE__, i<<10 | j);
				return FAILED;
			}
			if(isValid == TRUE) {
				printf("Error!(%d) Empty entry %d but get valid data\n",  __LINE__, i<<10 | j);
				return FAILED;
			}
		}
	}
	for(i=0; i<4; i++) {
		for(j=0; j<32; j++)
			if(rtl8651_setAsicNaptIcmpTable(FALSE, j, j<<2, i<<5 | j, _rtl8651_NaptAgingToSec((i+j)%32), (i+j)%2==0? TRUE: FALSE, i+j, (i+j)%2==0? TRUE: FALSE, TRUE) == FAILED) {
				printf("Error!(%d) Unable to configure global port %d\n", __LINE__, i<<10 | j);
				return FAILED;
			}
		for(j=0; j<32; j++) {
			ipaddr_t insideLocalIpAddr;
			uint16 insideLocalId, insideGlobalId, ageSec, count;
			int8 isStatic, isCollision, isValid;
			if(rtl8651_getAsicNaptIcmpTable(TRUE, i<<5 | j, &insideLocalIpAddr, &insideLocalId, &insideGlobalId, &ageSec, &isStatic, &count, &isCollision, &isValid) == FAILED) {
				printf("Error!(%d) Unable to get global port %d\n", __LINE__, i<<5 | j);
				return FAILED;
			}
			if(insideLocalIpAddr != j || insideLocalId != j<<2 || ageSec != _rtl8651_NaptAgingToSec((i+j)%32) || isStatic != ((i+j)%2==0? TRUE: FALSE) || count != i+j || isCollision != ((i+j)%2==0? TRUE: FALSE) || isValid != TRUE) {
				printf("Error!(%d) Configured ang stored parameter mismatch (%d %d %d %s %d %s %s) <> (%d %d %d %s %d %s %s)\n", __LINE__, j, j<<2, _rtl8651_NaptAgingToSec((i+j)%64), (i+j)%2==0? "TRUE": "FALSE", i+j, (i+j)%2==0? "TRUE": "FALSE", "TRUE", insideLocalIpAddr, insideLocalId, ageSec, isStatic==TRUE? "TRUE": "FALSE", count, isCollision==TRUE? "TRUE": "FALSE", isValid==TRUE? "TRUE": "FALSE");
				return FAILED;
			}
		}
		for(j=0; j<32; j++) {
			uint16 offset;
			int8 isValid;
			rtl8651_getAsicNaptIcmpOffset(j, &offset, &isValid);
			if(offset != i>>5 || isValid != TRUE) {
				printf("Error!(%d) Configured offset %d entry valid but get informaiton is offset %d entry %s\n", __LINE__, i>>5, offset, isValid==TRUE?"Valid" : "Invalid");
				return FAILED;
			}
		}
		for(j=0; j<32; j++)
			if(rtl8651_setAsicNaptIcmpTable(FALSE, j, j<<2, i<<5 | j, _rtl8651_NaptAgingToSec((i+j)%32), (i+j)%2==0? TRUE: FALSE, (i+j+1)%2==0? TRUE: FALSE, (i+j)%2==0? TRUE: FALSE, TRUE) == SUCCESS) {
				printf("Error!(%d) Occupied entry %d can normal write\n", __LINE__, j);
				return FAILED;
			}
		for(j=0; j<32; j++)
			if(rtl8651_setAsicNaptIcmpTable(TRUE, 0, 0,  i<<5 | j, 0, FALSE, FALSE, FALSE, FALSE) == FAILED) {
				printf("Error!(%d) Occupied entry %d cannot force write\n", __LINE__,  i<<5 | j);
				return FAILED;
			}
		for(j=0; j<32; j++) {
			ipaddr_t insideLocalIpAddr;
			uint16 insideLocalId, insideGlobalId, ageSec, count;
			int8 isStatic, isCollision, isValid;
			if(rtl8651_getAsicNaptIcmpTable(TRUE, i<<10 | j, &insideLocalIpAddr, &insideLocalId, &insideGlobalId, &ageSec, &isStatic, &count, &isCollision, &isValid) == FAILED) {
				printf("Error!(%d) Unable to get global port %d\n", __LINE__, i<<5 | j);
				return FAILED;
			}
			if(isValid == TRUE) {
				printf("Error!(%d) Empty entry %d but get valid data\n",  __LINE__, i<<10 | j);
				return FAILED;
			}
		}
	}

	for(i=0; i<64; i++)
		for(j=0; j<64; j++) {
			uint16 start, end;
			if(rtl8651_setAsicL4Offset(i, j) != (i<=j? SUCCESS: FAILED)) {
				printf("Error!(%d) Start offset %d End offset %d configuration execution result error\n", __LINE__, i, j);
				return FAILED;
			}
			if(i<=j) {
				rtl8651_getAsicL4Offset(&start, &end);
				if(start != i || end != j) {
					printf("Error!(%d) Configured start end offset not match get result (%d %d) <> (%d %d)\n", __LINE__, i, j, start, end);
					return FAILED;
				}
			}

		}
	for(i=0; i<4; i++) {
		for(j=0; j< RTL8651_ALGTBL_SIZE; j++)
			rtl8651_setAsicAlg(j, i<<7 | j);
		for(j=0; j< RTL8651_ALGTBL_SIZE; j++) {
			uint16 port; 
			rtl8651_getAsicAlg(j, &port);
			if(port != ( i<<7 | j)) {
				printf("Error!(%d) ALG entry %d configure %d but get to be %d\n", __LINE__, j, i<<7 | j, port);
				return FAILED;
			}
		}
		for(j=0; j< RTL8651_ALGTBL_SIZE; j++)
			if(rtl8651_delAsicAlg(j) !=SUCCESS) {
				printf("Error!(%d) ALG entry %d unable to delete\n", __LINE__, j);
				return FAILED;
			}
		for(j=0; j< RTL8651_ALGTBL_SIZE; j++) {
			uint16 port; 
			if(rtl8651_getAsicAlg(j, &port) == SUCCESS) {
				printf("Error!(%d) Deleted entry %d can get success\n", __LINE__, j);
				return FAILED;
			}
		}
	}
	for(i=0; i<4; i++) {
		uint32 k;
		rtl8651_tblDrvMacAclRule_t rule;
		for(j=0; j<RTL8651_ACLTBL_SIZE+RTL8651_ACLTBL_RESERV_SIZE; j++) {
			for(k=0; k<6; k++) {
				rule.dstMac.octet[k] = (i+j+k)%256;
				rule.dstMacMask.octet[k] =k<i?0xff:0;
				rule.srcMac.octet[k] = (i+j+k+1)%256;
				rule.srcMacMask.octet[k] =k<=i?0xff:0;
			}
			rule.typeLen = i+j;
			rule.typeLenMask = (0xffff<<(i+j)%17)&0xffff;
			rule.actionType = (i+j)%3==0? RTL8651_ACL_CPU: (i+j)%3==1? RTL8651_ACL_DROP: RTL8651_ACL_PERMIT;
			rtl8651_setAsicMacAcl(j, &rule);
		}
		for(j=0; j<RTL8651_ACLTBL_SIZE+RTL8651_ACLTBL_RESERV_SIZE; j++) {
			rtl8651_getAsicMacAcl(j, &rule);
			for(k=0; k<6; k++) {
				if(rule.dstMac.octet[k] != (i+j+k)%256 || rule.dstMacMask.octet[k] != (k<i?0xff:0) || rule.srcMac.octet[k] != (i+j+k+1)%256 || rule.srcMacMask.octet[k] != (k<=i?0xff:0)) {
					printf("Error!(%d) MAC rule %d mac parameter (%d) mismatch! Configure (%d %x %d %x) <> (%d %x %d %x)\n", __LINE__, j, k, (i+j+k)%256, k<i?0xff:0, (i+j+k+1)%256, k<=i?0xff:0, rule.dstMac.octet[k], rule.dstMacMask.octet[k], rule.srcMac.octet[k], rule.srcMacMask.octet[k]);
					return FAILED;
				}
			}
			if(rule.typeLen != i+j || rule.typeLenMask != ((0xffff<<(i+j)%17)&0xffff) || rule.actionType != ((i+j)%3==0? RTL8651_ACL_CPU: (i+j)%3==1? RTL8651_ACL_DROP: RTL8651_ACL_PERMIT)) {
				printf("Error!(%d) MAC rule %d other parameter mismatch! Configure (%d %x %d) <> (%d %x %d)\n", __LINE__, j, i+j, (0xffff<<(i+j)%17)&0xffff, (i+j)%3==0? RTL8651_ACL_CPU: (i+j)%3==1? RTL8651_ACL_DROP: RTL8651_ACL_PERMIT, rule.typeLen, rule.typeLenMask, rule.actionType);
				return FAILED;
			}
		}
	}
	for(i=0; i<4; i++) {
		rtl8651_tblDrvIpAclRule_t rule;
		for(j=0; j<RTL8651_ACLTBL_SIZE+RTL8651_ACLTBL_RESERV_SIZE; j++) {
			rule.srcIpAddr = i+j;
			rule.srcIpAddrMask = 0xffffffff<<((i+j+1)%33);
			rule.dstIpAddr = i+j+2;
			rule.dstIpAddrMask = 0xffffffff<<((i+j+3)%33);
			rule.tos = i+j+4;
			rule.tosMask = (0xff<<((i+j+5)%9))&0xff;
			rule.actionType = (i+j+6)%3==0? RTL8651_ACL_CPU: (i+j)%3==1? RTL8651_ACL_DROP: RTL8651_ACL_PERMIT;
			switch((i+j)%5) {
				case 0://IP Rule
				rule.ruleType = RTL8651_ACL_IP;
				rule.is.ip.proto = i+j+7;
				rule.is.ip.protoMask = (0xff<<((i+j+8)%9))&0xff;
				rule.is.ip.ipFlag = (i+j+9)%8;
				rule.is.ip.flagMask = (0xff<<((i+j+10)%9))&0x3;
				break;
				case 1://ICMP Rule
				rule.ruleType = RTL8651_ACL_ICMP;
				rule.is.icmp.type = i+j+7;
				rule.is.icmp.typeMask = (0xff<<((i+j+8)%9))&0xff;
				rule.is.icmp.code = i+j+9;
				rule.is.icmp.codeMask = (0xff<<((i+j+10)%9))&0xff;
				break;
				case 2://IGMP Rule
				rule.ruleType = RTL8651_ACL_IGMP;
				rule.is.igmp.type = i+j+7;
				rule.is.igmp.typeMask = (0xff<<((i+j+8)%9))&0xff;
				break;
				case 3://TCP Rule
				rule.ruleType = RTL8651_ACL_TCP;
				rule.is.tcp.tcpflag = i+j+7;
				rule.is.tcp.flagMask =  (0xff<<((i+j+8)%9))&0xff;
				rule.is.tcp.srcPortLowerBound = i+j+9;
				rule.is.tcp.srcPortUpperBound = i+j+10;
				rule.is.tcp.dstPortLowerBound = i+j+11;
				rule.is.tcp.dstPortUpperBound = i+j+12;
				break;
				case 4://UDP Rule
				rule.ruleType = RTL8651_ACL_UDP;
				rule.is.udp.srcPortLowerBound = i+j+7;
				rule.is.udp.srcPortUpperBound = i+j+8;
				rule.is.udp.dstPortLowerBound = i+j+9;
				rule.is.udp.dstPortUpperBound = i+j+10;
				break;
			}
			rtl8651_setAsicIpAcl(j, &rule);
		}
		for(j=0; j<RTL8651_ACLTBL_SIZE+RTL8651_ACLTBL_RESERV_SIZE; j++) {
			rtl8651_getAsicIpAcl(j, &rule);
			if(rule.srcIpAddr != i+j || rule.srcIpAddrMask != (0xffffffff<<((i+j+1)%33)) || rule.dstIpAddr != i+j+2 || rule.dstIpAddrMask != (0xffffffff<<((i+j+3)%33)) || rule.tos != i+j+4 || rule.tosMask != ((0xff<<((i+j+5)%9))&0xff)) {
				printf("Error!(%d) ACL IP rule %d common part mismatch (%d %x %d %x %d %x) <> (%d %x %d %x %d %x)\n", __LINE__, j, i+j, 0xffffffff<<((i+j+1)%33), i+j+2, 0xffffffff<<((i+j+3)%33), i+j+4, (0xff<<((i+j+5)%9))&0xff, rule.srcIpAddr, rule.srcIpAddrMask, rule.dstIpAddr, rule.dstIpAddrMask, rule.tos, rule.tosMask);
				return FAILED;
			}
			if(rule.actionType != ((i+j+6)%3==0? RTL8651_ACL_CPU: (i+j)%3==1? RTL8651_ACL_DROP: RTL8651_ACL_PERMIT)) {
				printf("Error!(%d) ACL IP rule %d action type mismatch! %d <> %d\n", __LINE__, j, (i+j+6)%3==0? RTL8651_ACL_CPU: (i+j)%3==1? RTL8651_ACL_DROP: RTL8651_ACL_PERMIT, rule.actionType);
				return FAILED;
			}
			switch((i+j)%5) {
				case 0://IP Rule
				if(rule.ruleType != RTL8651_ACL_IP) {
					printf("Error!(%d) ACL Pure IP rule %d type error (%d)\n", __LINE__, j, rule.ruleType);
					return FAILED;
				}
				if(rule.is.ip.proto != i+j+7 || rule.is.ip.protoMask != ((0xff<<((i+j+8)%9))&0xff) || rule.is.ip.ipFlag != (i+j+9)%8 || rule.is.ip.flagMask != ((0xff<<((i+j+10)%9))&0x3)) {
					printf("Error!(%d) ACL Pure IP rule %d field mismatch (%d %x %d %x) <> (%d %x %d %x)\n", __LINE__, j, i+j+7, (0xff<<((i+j+8)%9))&0xff, (i+j+9)%8, (0xff<<((i+j+10)%9))&0x3, rule.is.ip.proto, rule.is.ip.protoMask, rule.is.ip.ipFlag, rule.is.ip.flagMask);
					return FAILED;
				}
				break;
				case 1://ICMP Rule
				if(rule.ruleType != RTL8651_ACL_ICMP) {
					printf("Error!(%d) ACL ICMP rule %d type error (%d)\n", __LINE__, j, rule.ruleType);
					return FAILED;
				}
				if(rule.is.icmp.type != i+j+7 || rule.is.icmp.typeMask != ((0xff<<((i+j+8)%9))&0xff) || rule.is.icmp.code != i+j+9 || rule.is.icmp.codeMask != ((0xff<<((i+j+10)%9))&0xff)) {
					printf("Error!(%d) ACL ICMP rule %d field mismatch (%d %x %d %x) <> (%d %x %d %x)\n", __LINE__, j, i+j+7, (0xff<<((i+j+8)%9))&0xff, i+j+9, 0xff<<((i+j+10)%9)&0xff, rule.is.icmp.type, rule.is.icmp.typeMask, rule.is.icmp.code, rule.is.icmp.codeMask);
					return FAILED;
				}
				break;
				case 2://IGMP Rule
				if(rule.ruleType != RTL8651_ACL_IGMP) {
					printf("Error!(%d) ACL IGMP rule %d type error (%d)\n", __LINE__, j, rule.ruleType);
					return FAILED;
				}
				if(rule.is.igmp.type != i+j+7 || rule.is.igmp.typeMask != ((0xff<<((i+j+8)%9))&0xff)) {
					printf("Error!(%d) ACL IGMP rule %d field mismatch (%d %x) <> (%d %x)\n", __LINE__, j, i+j+7, (0xff<<((i+j+8)%9))&0xff, rule.is.igmp.type, rule.is.igmp.typeMask);
					return FAILED;
				}
				break;
				case 3://TCP Rule
				if(rule.ruleType != RTL8651_ACL_TCP) {
					printf("Error!(%d) ACL TCP rule %d type error (%d)\n", __LINE__, j, rule.ruleType);
					return FAILED;
				}
				if(rule.is.tcp.tcpflag != i+j+7 || rule.is.tcp.flagMask != ((0xff<<((i+j+8)%9))&0xff) || rule.is.tcp.srcPortLowerBound != i+j+9 || rule.is.tcp.srcPortUpperBound != i+j+10 || rule.is.tcp.dstPortLowerBound != i+j+11 || rule.is.tcp.dstPortUpperBound != i+j+12) {
					printf("Error!(%d) ACL TCP rule %d field mismatch (%d %x %d %d %d %d) <> (%d %x %d %d %d %d)\n", __LINE__, j, i+j+7, (0xff<<((i+j+8)%9))&0xff, i+j+9, i+j+10, i+j+11, i+j+12, rule.is.tcp.tcpflag, rule.is.tcp.flagMask, rule.is.tcp.srcPortLowerBound, rule.is.tcp.srcPortUpperBound, rule.is.tcp.dstPortLowerBound, rule.is.tcp.dstPortUpperBound);
					return FAILED;
				}
				break;
				case 4://UDP Rule
				if(rule.ruleType != RTL8651_ACL_UDP) {
					printf("Error!(%d) ACL UDP rule %d type error (%d)\n", __LINE__, j, rule.ruleType);
					return FAILED;
				}
				if(rule.is.udp.srcPortLowerBound != i+j+7 || rule.is.udp.srcPortUpperBound != i+j+8 || rule.is.udp.dstPortLowerBound != i+j+9 || rule.is.udp.dstPortUpperBound != i+j+10) {
					printf("Error!(%d) ACL UDP rule %d field mismatch (%d %d %d %d) <> (%d %d %d %d)\n", __LINE__, j, i+j+7, i+j+8, i+j+9, i+j+10, rule.is.udp.srcPortLowerBound, rule.is.udp.srcPortUpperBound, rule.is.udp.dstPortLowerBound, rule.is.udp.dstPortUpperBound);
					return FAILED;
				}
				break;
			}
		}
	}
	for(i=0; i<RTL8651_VLAN_NUMBER; i++) {
		ether_addr_t macAddr;
		int8 portState[RTL8651_PORT_NUMBER];
		for(j=0;j<6;j++)
			macAddr.octet[j] = i<<(j%4);
		for(j=0;j<RTL8651_PORT_NUMBER;j++)
			portState[j] = (i+j)%2==1? TRUE: FALSE;
		if(rtl8651_setAsicVlan(i, &macAddr, 1<<(i%RTL8651_PORT_NUMBER), i<<2, (i<<2) + 7, (i<<2) + 8, (i<<2) + 15, i%2==1? TRUE: FALSE, i%2==1? FALSE: TRUE, portState, i%2==1? TRUE: FALSE, i%2==1? FALSE: TRUE, 1<<(i%RTL8651_PORT_NUMBER), 1<<(i%4), (i<<7)+512) == FAILED) {
			printf("Error! Set vlan %d failed\n", i);
			return FAILED;
		}
	}
	for(i=0; i<4; i++) {
		uint16 svid;
		uint32 srcPort, dstPortMask;
		for(j=0; j<0x3f; j++) 
			rtl8651_setAsicIpMulticastTable(j<<2, 0xe0000001, (i+j)%8, (i+j+1)%RTL8651_PORT_NUMBER, 1<<((i+j+2)%RTL8651_PORT_NUMBER));
		for(j=0; j<0x3f; j++) {
			if(rtl8651_getAsicIpMulticastTable(j<<2, 0xe0000001, &svid, &srcPort, &dstPortMask) == FAILED) {
				printf("Error!(%d) IP multicast table entry %u not exist\n", __LINE__, rtl8651_ipMulticastTableIndex(j<<2, 0xe0000001));
				return FAILED;
			}
			if(svid != ((i+j)%8) || srcPort != ((i+j+1)%RTL8651_PORT_NUMBER) || dstPortMask != (1<<((i+j+2)%RTL8651_PORT_NUMBER))) {
				printf("Error! (%d) IP multicast table entry %u field mismatch (%u, %u, %u) <> (%u, %u, %u)\n", __LINE__, rtl8651_ipMulticastTableIndex(j<<2, 0xe0000001), svid, srcPort, dstPortMask, (i+j)%8, (i+j+1)%RTL8651_PORT_NUMBER, 1<<((i+j+2)%RTL8651_PORT_NUMBER));
				return FAILED;
			}
		}
		for(j=0; j<0x3f; j++)
			rtl8651_delAsicIpMulticastTable(j<<2, 0xe0000001);
		for(j=0; j<0x3f; j++)
			if(rtl8651_getAsicIpMulticastTable(j<<2, 0xe0000001, &svid, &srcPort, &dstPortMask) != FAILED) {
				printf("Error!(%d) IP multicast table entry %u not successfully deleted\n", __LINE__, rtl8651_ipMulticastTableIndex(j<<2, 0xe0000001));
				return FAILED;
			}
	}
	for(i=0; i<RTL8651_VLAN_NUMBER; i++) {
		ether_addr_t macAddr;
		int8 portState[RTL8651_PORT_NUMBER], internalIntf, enableRoute, broadcastToCpu, promiscuous;
		uint32 mbr, inAclStart, inAclEnd, outAclStart, outAclEnd, untagPortMask, macNumber, mtu;
		if(rtl8651_delAsicVlan(i)  !=SUCCESS) {
			printf("Error!(%d) Delete vlan %d failed\n", __LINE__, i);
			return FAILED;
		}
		if(rtl8651_getAsicVlan(i, &macAddr, &mbr, &inAclStart, &inAclEnd, &outAclStart, &outAclEnd, &internalIntf, &enableRoute, portState, &broadcastToCpu, &promiscuous, &untagPortMask, &macNumber, &mtu) == SUCCESS) {
			printf("Error!(%d) Get deleted vlan %d success\n", __LINE__, i);
			return FAILED;
		}
	}
	printf("ASIC test complete start to initialize ASIC\n");
//Return to initial state
	rtl8651_setAsicOperationLayer(1);//Disable layer2, layer3 and layer4 function
	rtl8651_clearAsicAllTable();
	rtl8651_setAsicSpanningEnable(FALSE);

	return SUCCESS;	
}
