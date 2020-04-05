/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /usr/local/dslrepos/uClinux-dist/linux-2.4.x/drivers/net/re8650/swCore.h,v 1.1.1.1 2003/08/18 05:43:22 kaohj Exp $
*
* Abstract: Switch core header file.
*
* $Author: kaohj $
*
* $Log: swCore.h,v $
* Revision 1.1.1.1  2003/08/18 05:43:22  kaohj
* initial import into CVS
*
* Revision 1.1  2003/05/12 09:56:13  kckao
* moved from csp
*
* Revision 1.4  2003/05/12 02:49:32  kckao
* add swCore_layer2TableGetPortByMac for select output port by desitination MAC
*
* Revision 1.3  2003/05/12 02:23:02  orlando
* correct typo from "negociation" to "negotiation".
*
* Revision 1.2  2003/05/05 02:09:54  kckao
* Modify for OS independent
*
* Revision 1.12  2003/02/20 03:08:12  jzchen
* Change counter api to macro
*
* Revision 1.11  2002/12/10 08:46:38  elvis
* add swCore_enableARPTrapToCPU(), swCore_disableARPTrapToCPU(),
* RARP, PPPOE, IGMP, DHCP1, DHCP2, OSPF & RIP
*
* Revision 1.10  2002/11/22 10:44:05  danwu
* Reconstruct table drivers.
*
* Revision 1.9  2002/09/17 09:32:59  danwu
* Change ip table isN2One to isOne2One.
*
* Revision 1.8  2002/09/04 02:31:20  danwu
* Add swCore_setMulticastPortExternalMode().
*
* Revision 1.7  2002/09/03 12:54:57  danwu
* Add ifsel rule of acl table.
*
* Revision 1.6  2002/08/28 11:22:53  danwu
* Add napt offset control functions.
*
* Revision 1.5  2002/08/23 09:00:06  danwu
* no message
*
* Revision 1.4  2002/08/23 06:02:21  danwu
* no message
*
* Revision 1.3  2002/08/22 08:48:46  delta
* no message
*
* Revision 1.2  2002/08/21 06:40:41  danwu
* Add functions.
*
* Revision 1.1  2002/08/20 05:58:32  danwu
* Create.
*
* ---------------------------------------------------------------
*/

#ifndef _SWCORE_H
#define _SWCORE_H



#define SWNIC_DEBUG
#define SWTABLE_DEBUG
#define SWCORE_DEBUG



/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_init
 * --------------------------------------------------------------------
 * FUNCTION: This service initializes the switch core.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENFILE: Destination slot of vlan table is occupied.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_init(void);


/* VLAN service
*/

#define RTL_STP_DISABLE 0
#define RTL_STP_BLOCK   1
#define RTL_STP_LEARN   2
#define RTL_STP_FORWARD 3

/* VLAN access parameters */
typedef struct {
    uint32          memberPort;
    uint32          egressUntag;
    macaddr_t       gMac;
    uint16          bcastToCPU  : 1;
    uint16          promiscuous : 1;
    uint16          reserv0     : 14;
    uint32          mtu;
} rtl_vlan_param_t;


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanCreate
 * --------------------------------------------------------------------
 * FUNCTION: This service creates a vlan.
 * INPUT   :
		param_P: Pointer to the parameters.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
    EEXIST: Speicified vlan already exists.
		ENFILE: Destination slot of vlan table is occupied.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanCreate(uint32 vid, rtl_vlan_param_t * param_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanDestroy
 * --------------------------------------------------------------------
 * FUNCTION: This service destroys a vlan.
 * INPUT   :
		vid: Vlan ID.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified vlan does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanDestroy(uint32 vid);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanSetPVid
 * --------------------------------------------------------------------
 * FUNCTION: This service sets port based vlan id.
 * INPUT   :
		portNum: Port number.
		pvid: Vlan ID.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified vlan does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanSetPVid(uint32 portNum, uint32 pvid);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanGetPVid
 * --------------------------------------------------------------------
 * FUNCTION: This service gets port based vlan id.
 * INPUT   :
		portNum: Port number.
 * OUTPUT  : 
		pvid_P: Pointer to a variable to hold the PVid.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified vlan does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanGetPVid(uint32 portNum, uint32 *pvid_P);

/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanSetPortSTPStatus
 * --------------------------------------------------------------------
 * FUNCTION: This service sets the spanning tree status of the 
        specified port.
 * INPUT   :
		vid: Vlan ID.
		portNum: Port number.
		STPStatus: Spanning tree status. Valid values are RTL_STP_DISABLE, 
		        RTL_STP_BLOCK, RTL_STP_LEARN and RTL_STP_FORWARD.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified vlan does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanSetPortSTPStatus(uint32 vid, uint32 portNumber, uint32 STPStatus);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanSetSTPStatusOfAllPorts
 * --------------------------------------------------------------------
 * FUNCTION: This service sets the spanning tree status.
 * INPUT   :
		vid: Vlan ID.
		STPStatus: Spanning tree status. Valid values are RTL_STP_DISABLE, 
		        RTL_STP_BLOCK, RTL_STP_LEARN and RTL_STP_FORWARD.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified vlan does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanSetSTPStatusOfAllPorts(uint32 vid, uint32 STPStatus);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanGetPortSTPStatus
 * --------------------------------------------------------------------
 * FUNCTION: This service gets the spanning tree status of the 
        specified port.
 * INPUT   :
		vid: Vlan ID.
		portNum: Port number.
 * OUTPUT  : 
		STPStatus_P: Pointer to a variable to hold the spanning tree 
		        status of the specified port.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified vlan does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanGetPortSTPStatus(uint32 vid, uint32 portNumber, uint32 *STPStatus_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanGetInformation
 * --------------------------------------------------------------------
 * FUNCTION: This service gets information of the specified vlan.
 * INPUT   :
		vid: Vlan ID.
 * OUTPUT  : 
		param_P: Pointer to an area to hold the parameters.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified vlan does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanGetInformation(uint32 vid, rtl_vlan_param_t * param_P);


/* Layer 2 service
*/

/* L2 forwarding table access parameters 
*/
typedef struct {
    macaddr_t       mac;
    uint16          isStatic    : 1;
    uint16          hPriority   : 1;
    uint16          toCPU       : 1;
    uint16          srcBlock    : 1;
    uint16          nxtHostFlag : 1;
    uint16          reserv0     : 11;
    uint32          memberPort;
    uint32          agingTime;
} rtl_l2_param_t;


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_staticMacAddrAdd
 * --------------------------------------------------------------------
 * FUNCTION: This service adds the static MAC address.
 * INPUT   :
		param_P: Pointer to the parameters.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		ENFILE: Cannot allocate slot.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_staticMacAddrAdd(rtl_l2_param_t * param_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_staticMacAddrRemove
 * --------------------------------------------------------------------
 * FUNCTION: This service removes the specified static MAC address.
 * INPUT   :
		param_P: Pointer to the parameters.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified MAC address does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_staticMacAddrRemove(rtl_l2_param_t * param_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_layer2TableGetInformation
 * --------------------------------------------------------------------
 * FUNCTION: This service gets information of specified L2 switch table 
        entry.
 * INPUT   :
        entryIndex: Index of entry.
 * OUTPUT  : 
		param_P: Pointer to an area to hold the parameters.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
        EEMPTY: Specified entry is empty.
		ENOENT: Specified entry does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_layer2TableGetInformation(uint32 entryIndex, rtl_l2_param_t * param_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_layer2TableGetInformationByMac
 * --------------------------------------------------------------------
 * FUNCTION: This service gets information of specified L2 switch table 
        entry.
 * INPUT   : None.
 * OUTPUT  : 
		param_P: Pointer to an area to hold the parameters.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		ENOENT: Specified entry does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_layer2TableGetInformationByMac(rtl_l2_param_t * param_P);


/* Counter service 
*/

typedef struct {
    uint32  etherStatsOctets;
    uint32  etherStatsDropEvents;
    uint32  etherStatsCRCAlignErrors;
    uint32  etherStatsFragments;
    uint32  etherStatsJabbers;
    uint32  ifInUcastPkts;
    uint32  etherStatsMulticastPkts;
    uint32  etherStatsBroadcastPkts;
    uint32  etherStatsUndersizePkts;
    uint32  etherStatsPkts64Octets;
    uint32  etherStatsPkts65to127Octets;
    uint32  etherStatsPkts128to255Octets;
    uint32  etherStatsPkts256to511Octets;
    uint32  etherStatsPkts512to1023Octets;
    uint32  etherStatsPkts1024to1518Octets;
    uint32  etherStatsOversizepkts;
    uint32  dot3ControlInUnknownOpcodes;
    uint32  dot3InPauseFrames;
} rtl_ingress_counter_t;
typedef struct {
    uint32  ifOutOctets;
    uint32  ifOutUcastPkts;
    uint32  ifOutMulticastPkts;
    uint32  ifOutBroadcastPkts;
    uint32  dot3StatsLateCollisions;
    uint32  dot3StatsDeferredTransmissions;
    uint32  etherStatsCollisions;
    uint32  dot3StatsMultipleCollisionFrames;
    uint32  dot3OutPauseFrames;
} rtl_egress_counter_t;

/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_counterGetMemberPort
 * --------------------------------------------------------------------
 * FUNCTION: This service gets all the member for counting.
 * INPUT   : None.
 * OUTPUT  : 
        portList_P: List of member ports.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_counterGetMemberPort(uint32 *portList_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_counterSetMemberPort
 * --------------------------------------------------------------------
 * FUNCTION: This service gets all the member for counting.
 * INPUT   : 
        portList: List of member ports.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_counterSetMemberPort(uint32 portList);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_counterGetIngress
 * --------------------------------------------------------------------
 * FUNCTION: This service gets all the ingress counters.
 * INPUT   : None.
 * OUTPUT  : 
        counters_P: Pointer to an area to hold the ingress counters.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_counterGetIngress(rtl_ingress_counter_t *counters_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_counterGetEgress
 * --------------------------------------------------------------------
 * FUNCTION: This service gets all the egress counters.
 * INPUT   : None.
 * OUTPUT  : 
        counters_P: Pointer to an area to hold the egress counters.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_counterGetEgress(rtl_egress_counter_t *counters_P);


/* Port service 
*/

#define RTL_PORT_100M_FD        (1 << 0)
#define RTL_PORT_100M_HD        (1 << 1)
#define RTL_PORT_10M_FD         (1 << 2)
#define RTL_PORT_10M_HD         (1 << 3)

typedef struct {
    uint8   capableFlowCtrl : 1;
    uint8   capable100MFull : 1;
    uint8   capable100MHalf : 1;
    uint8   capable10MFull  : 1;
    uint8   capable10MHalf  : 1;
    uint8   reserv0         : 3;
} rtl_auto_nego_ability_t;
    
typedef struct {
    uint8   enAutoNego          : 1;
    uint8   enSpeed100M         : 1;
    uint8   enFullDuplex        : 1;
    uint8   enLoopback          : 1;
    uint8   linkEstablished     : 1;
    uint8   autoNegoCompleted   : 1;
    uint8   remoteFault         : 1;
    uint8   reserv0             : 1;
    rtl_auto_nego_ability_t   autoNegoAbility;
    rtl_auto_nego_ability_t   linkPartnerAutoNegoAbility;
    uint32  speedDuplex;
} rtl_port_status_t;

/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portSetSpeedDuplex
 * --------------------------------------------------------------------
 * FUNCTION: This service sets speed and duplex mode of specified port.
 * INPUT   :
		portNum: Port number.
		speedDuplex: Speed and duplex mode. Valid values are 
		    RTL_PORT_100M_FD, RTL_PORT_100M_HD, RTL_PORT_10M_FD and 
		    RTL_PORT_10M_HD.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portSetSpeedDuplex(uint32 portNum, uint32 speedDuplex);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portSetAutoNegotiationAbility
 * --------------------------------------------------------------------
 * FUNCTION: This service sets auto Negotiation pause, speed and duplex 
        mode capability of specified port.
 * INPUT   :
		portNum: Port number.
		anAbility_P: Pointer to the data structure which specifies the auto 
		    Negotiation abilities.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portSetAutoNegotiationAbility(uint32 portNum, rtl_auto_nego_ability_t *anAbility_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portEnableAutoNegotiation
 * --------------------------------------------------------------------
 * FUNCTION: This service enables auto Negotiation of specified port.
 * INPUT   :
		portNum: Port number.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portEnableAutoNegotiation(uint32 portNum);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portDisableAutoNegotiation
 * --------------------------------------------------------------------
 * FUNCTION: This service disables auto Negotiation of specified port.
 * INPUT   :
		portNum: Port number.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portDisableAutoNegotiation(uint32 portNum);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portRestartAutoNegotiation
 * --------------------------------------------------------------------
 * FUNCTION: This service restarts auto Negotiation of specified port.
 * INPUT   :
		portNum: Port number.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portRestartAutoNegotiation(uint32 portNum);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portSetLoopback
 * --------------------------------------------------------------------
 * FUNCTION: This service sets specified port to loopback mode.
 * INPUT   :
		portNum: Port number.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portSetLoopback(uint32 portNum);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portResetLoopback
 * --------------------------------------------------------------------
 * FUNCTION: This service sets specified port to normal mode.
 * INPUT   :
		portNum: Port number.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portResetLoopback(uint32 portNum);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portGetStatus
 * --------------------------------------------------------------------
 * FUNCTION: This service gets port status of specified port.
 * INPUT   : 
		portNum: Port number.
 * OUTPUT  : 
    portStatus_P: Pointer to an area to hold the port status.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portGetStatus(uint32 portNum, rtl_port_status_t *portStatus_P);



#define swCore_vlanCreate vlanTable_create
#define swCore_vlanDestroy vlanTable_destroy
#define swCore_vlanSetPortSTPStatus vlanTable_setPortStpStatus
#define swCore_vlanSetSTPStatusOfAllPorts vlanTable_setStpStatusOfAllPorts
#define swCore_vlanGetPortSTPStatus vlanTable_getSTPStatus
#define swCore_vlanGetInformation vlanTable_getInformation
#define swCore_layer2TableGetPortByMac	l2SwitchTable_getPortByMac

#endif /* _SWCORE_H */


