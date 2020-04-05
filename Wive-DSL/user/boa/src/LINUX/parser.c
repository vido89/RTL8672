#ifdef AUTO_PROVISIONING
#include<stdio.h>
#include<string.h>
#include <arpa/inet.h>
#include <net/route.h>
#include <string.h>
#include "mib.h"
#include "mibtbl.h"
#include "utility.h"
#include "webform.h"
//#include "1x_rc4.h"
#include <sys/reboot.h>

//#include "../LINUX/climenu.c"
#define error -1
#define chain_record_number	(MIB_CHAIN_TBL_END-CHAIN_ENTRY_TBL_ID+1)
#define MAX_POE_PER_VC		2

enum {
	INPUT_TYPE_UINT,
	INPUT_TYPE_STRING,
	INPUT_TYPE_IPADDR,
	INPUT_TYPE_IPMASK,
	INPUT_TYPE_ETHADDR,
	INPUT_TYPE_UNIT_DEFAULT
};
char chain_updated[chain_record_number];

static char *getMibInfo(int id) {
	static char strbuf[256];
	
	if (getMIB2Str(id, strbuf) == 0)
		return strbuf;
	
	return NULL;
}

int filterAdd(unsigned char filterMode,unsigned char *sip,unsigned char *smask,unsigned short sfromPort,unsigned short stoPort,
                unsigned char *dip,unsigned char *dmask,unsigned short dfromPort,unsigned short dtoPort,
		unsigned char ptType,unsigned char prType){
	int mibTblId;
	unsigned int totalEntry;
	char * strVal;
	MIB_CE_IP_PORT_FILTER_T filterEntry;
	unsigned char noIP=1;
	char *strFrom, *strTo;
	unsigned long v1, v2, v3;
	struct in_addr curIpAddr, curSubnet;	
	unsigned char *tmpip;
	int i, intVal;	
	
	unsigned long mask, mbit;
	
	
	mibTblId = MIB_IP_PORT_FILTER_TBL;
	totalEntry = mib_chain_total(mibTblId); /* get chain record size */

	memset(&filterEntry, 0x00, sizeof(filterEntry));
	
	// Protocol Type
	if ( prType==1 ) {
		filterEntry.protoType = PROTO_ICMP;
	}
	else if ( prType==2  ) {
		filterEntry.protoType = PROTO_TCP;
	}
	else if ( prType==3 ) {
		filterEntry.protoType = PROTO_UDP;
	}
	else {
		printf("Error! Invalid protocol type.\n");
		goto setErr_filter;
	}
	
	// Direction
	if ( ptType==1 ) {
		filterEntry.dir = DIR_OUT;
	}
	else if ( ptType==2  ) {
		filterEntry.dir = DIR_IN;
	}	
	else {
		printf("Error! Invalid direction.\n");
		goto setErr_filter;
	}
	
	
	// Src Host
	if (filterEntry.protoType != PROTO_TCP && filterEntry.protoType != PROTO_UDP)
		sfromPort = 0;
	
	if (!inet_aton(sip, (struct in_addr *)&filterEntry.srcIp)) {
		printf("Invalid Src IP address!\n");
		goto setErr_filter;
	}
	
	inet_aton(smask, (struct in_addr *)&mask);
	if (mask==0) {
		printf("Invalid Src Netmask!\n");
		goto setErr_filter;
	}else {
				
		mbit=0;
				
		while (1) {
			if (mask&0x80000000) {
				mbit++;
				mask <<= 1;
			}
			else
				break;
		}
				
		filterEntry.smaskbit = mbit;
	}
	
	if(sfromPort != 0)
	{
		int intVal;
		if ( sfromPort<1 || sfromPort>65535) {
			printf("Error! Invalid value of src from-port.\n");
			goto setErr_filter;
		}
		filterEntry.srcPortFrom = (unsigned short)sfromPort;


		if ( stoPort == 0 )
			filterEntry.srcPortTo = filterEntry.srcPortFrom;
		else {
			if ( stoPort<1 || stoPort>65535) {
				printf("Error! Invalid value of src to-port.\n");
				goto setErr_filter;
			}
			filterEntry.srcPortTo = (unsigned short)stoPort;
		}

		if ( filterEntry.srcPortFrom  > filterEntry.srcPortTo ) {
			printf("Error! Invalid src port range.\n");
			goto setErr_filter;
		}
			
	}


	// Dst Host
	if (filterEntry.protoType != PROTO_TCP && filterEntry.protoType != PROTO_UDP)
		dfromPort = 0;
	
	if (!inet_aton(dip, (struct in_addr *)&filterEntry.dstIp)) {
		printf("Invalid Dst IP address!\n");
		goto setErr_filter;
	}
	
	inet_aton(dmask, (struct in_addr *)&mask);
	if (mask==0) {
		printf("Invalid Dst Netmask!\n");
		goto setErr_filter;
	}else {
				
		mbit=0;
				
		while (1) {
			if (mask&0x80000000) {
				mbit++;
				mask <<= 1;
			}
			else
				break;
		}
				
		filterEntry.dmaskbit = mbit;
	}
	
	if(dfromPort != 0)
	{
		int intVal;
		if ( dfromPort<1 || dfromPort>65535) {
			printf("Error! Invalid value of Dst from-port.\n");
			goto setErr_filter;
		}
		filterEntry.dstPortFrom = (unsigned short)dfromPort;


		if ( dtoPort == 0 )
			filterEntry.dstPortTo = filterEntry.dstPortFrom;
		else {
			if ( dtoPort<1 || dtoPort>65535) {
				printf("Error! Invalid value of Dst to-port.\n");
				goto setErr_filter;
			}
			filterEntry.dstPortTo = (unsigned short)dtoPort;
		}

		if ( filterEntry.dstPortFrom  > filterEntry.dstPortTo ) {
			printf("Error! Invalid Dst port range.\n");
			goto setErr_filter;
		}
			
	}


	// Rule Action
	if (filterMode==1)
		filterEntry.action = 0;
	else if (filterMode==2)
		filterEntry.action = 1;
	else {
		printf("Invalid Rule Action value!\n");
		goto setErr_filter;
	}
	
	intVal = mib_chain_add(MIB_IP_PORT_FILTER_TBL, (unsigned char*)&filterEntry);
	if (intVal == 0) {
		printf(strAddChainerror);
		goto setErr_filter;
	}
	else if (intVal == -1) {
		printf(strTableFull);
		goto setErr_filter;
	}
	return 1;
	
	
setErr_filter:
	printf("set FW error!\n");
	return error;
}

#ifdef MAC_FILTER
int MACFilterAdd(unsigned char act,unsigned char *src_macaddr,unsigned char *dst_macaddr,unsigned char dir){
	MIB_CE_MAC_FILTER_T macEntry;
	MIB_CE_MAC_FILTER_Tp pEntry;
	int mibTblId,i, intVal;
	unsigned int totalEntry;
	memset(&macEntry, 0x00, sizeof(macEntry));
	mibTblId = MIB_MAC_FILTER_TBL;


	totalEntry = mib_chain_total(mibTblId); /* get chain record size */


	if (act==1)
		macEntry.action = 0;
	else if (act==2)
		macEntry.action = 1;
	else {
		printf("Invalid Rule Action value!\n");
		goto setErr_filter;
	}


	if (dir==1)
		macEntry.dir = 0;
	else if (dir==2)
		macEntry.dir = 1;
	else {
		printf("Invalid Direction value!\n");
		goto setErr_filter;
	}


	if ( !src_macaddr[0] && !dst_macaddr[0]) {
		printf("Error! No mac address to set.\n");
		goto setErr_filter;
	}
	
	if (src_macaddr[0]) {
		if (strlen(src_macaddr)!=12 || !string_to_hex(src_macaddr, macEntry.srcMac, 12)) {
			printf("Error! Invalid Src MAC address.\n");
			goto setErr_filter;
		}
	}
	
	if (dst_macaddr[0]) {
		if (strlen(dst_macaddr)!=12 || !string_to_hex(dst_macaddr, macEntry.dstMac, 12)) {
			printf("Error! Invalid Dst MAC address.\n");
			goto setErr_filter;
		}	
	}


	intVal = mib_chain_add(MIB_MAC_FILTER_TBL, (unsigned char*)&macEntry);
	if (intVal == 0) {
		printf(strAddChainerror);
		goto setErr_filter;
	}
	else if (intVal == -1) {
		printf(strTableFull);
		goto setErr_filter;
	}
	return 1;

setErr_filter:
	return error;

}
#endif

int addPortFW(unsigned char *ip, unsigned short fromPort,unsigned short toPort,
		unsigned char protocol){

		char_t *submitUrl, *strAddPort, *strDelPort, *strVal, *strDelAllPort;
		char_t *strIp, *strFrom, *strTo;
		int intVal;
		unsigned int entryNum, i;
		MIB_CE_PORT_FW_T entry;
		struct in_addr curIpAddr, curSubnet;
		unsigned long v1, v2, v3;
		unsigned char vChar;


		inet_aton(ip, (struct in_addr *)&entry.ipAddr);
		mib_get( MIB_ADSL_LAN_IP,  (void *)&curIpAddr);
		mib_get( MIB_ADSL_LAN_SUBNET,  (void *)&curSubnet);

		v1 = *((unsigned long *)entry.ipAddr);
		v2 = *((unsigned long *)&curIpAddr);
		v3 = *((unsigned long *)&curSubnet);

		if ( (v1 & v3) != (v2 & v3) ) {
			printf("Invalid IP address! It should be set within the current subnet.\n");
			goto setErr_portfw;
		}

		intVal=fromPort;
		if ( intVal<1 || intVal>65535) {
			printf("Error! Invalid value of from-port.\n");
			goto setErr_portfw;
		}
		entry.fromPort = (unsigned short)intVal;

		intVal=toPort;
		if (  intVal<1 || intVal>65535) {
			printf("Error! Invalid value of to-port.\n");
			goto setErr_portfw;
		}
		entry.toPort = (unsigned short)intVal;

		if ( entry.fromPort  > entry.toPort ) {
			printf("Error! Invalid port range.\n");
			goto setErr_portfw;
		}

		if ( protocol==1)
			entry.protoType = PROTO_TCP;
		else if ( protocol==2 )
			entry.protoType = PROTO_UDP;
		else if ( protocol==3 )
			entry.protoType = PROTO_UDPTCP;
		else {
			printf("Error! Invalid protocol type.");
			goto setErr_portfw;
		}

		entryNum = mib_chain_total(MIB_PORT_FW_TBL);

		// Check if there is any port overlapped
		for (i=0; i<entryNum; i++) {
			MIB_CE_PORT_FW_Tp pCheckEntry;

			pCheckEntry = (MIB_CE_PORT_FW_Tp) mib_chain_get(MIB_PORT_FW_TBL, i);

			if ( pCheckEntry == NULL) {
				printf("Get table entry error!");
				goto setErr_portfw;
			}

			if(!(((entry.fromPort < pCheckEntry->fromPort) && (entry.toPort < pCheckEntry->fromPort)) || 
				((entry.fromPort > pCheckEntry->toPort) && (entry.toPort > pCheckEntry->toPort))) &&
			       (entry.protoType & pCheckEntry->protoType) ) {
				printf("Setting port range has overlapped with used port numbers!\n");
				goto setErr_portfw;
			}
		}

		intVal = mib_chain_add(MIB_PORT_FW_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			printf(strAddChainerror);
			goto setErr_portfw;
		}
		else if (intVal == -1) {
			printf(strTableFull);
			goto setErr_portfw;
		}
		return 1;

setErr_portfw:
	printf("Set port forwarding error!\n");
	return error;
}

static unsigned char if_find_index(int cmode, unsigned int map)
{
	int i;
	unsigned char index;
	
	// find the first available vc index (mpoa interface)
	i = 0;
	for (i=0; i<MAX_VC_NUM; i++)
	{
		if (!((map>>i) & 1))
			break;
	}
	
	if (i != MAX_VC_NUM)
		index = i;
	else
		return 0xff;
	
	if (cmode == ADSL_PPPoE || cmode == ADSL_PPPoA)
	{
		// find an available PPP index
		map >>= 16;
		i = 0;
		while (map & 1)
		{
			map >>= 1;
			i++;
		}
		if (i<=(MAX_PPP_NUM-1))
			index |= i << 4;	// high nibble for PPP index
		else
			return 0xef;
		
		if (cmode == ADSL_PPPoA)
			index |= 0x0f;	// PPPoA doesn't use mpoa interface, set to 0x0f (don't care)
	}
	else
	{
		// don't care the PPP index
		index |= 0xf0;
	}
	return index;
}
static int getVcInput(char *line,MIB_CE_ATM_VC_Tp entry, MIB_CE_ATM_VC_Tp old) {

	int vpi,vci,encap,channelMode,napt,channelStatus,pppConnectType,channelAddrType,pppIdleTime,defaultGW;
	int qos, pcr, cdvt, scr, mbs, pppAuth, mtu, ipunnum;
	char str[20],pppUser[30],pppPasswd[30],LocalIPAddr[16],RemoteIPAddr[16],SubnetMask[16],pppAc[30];
	
	entry->qos = 0;
	entry->pcr = ATM_MAX_US_PCR;
	
	//sscanf(line,"%d	%d	%d	%d	%d	%d	%d	%s	%s	%s	%s	%s	%d	%d",
	//	&vpi,&vci,&encap,&channelMode,&napt,&channelStatus,&channelAddrType,LocalIPAddr,
	//	RemoteIPAddr,SubnetMask,pppUser,pppPasswd,pppConnectType,pppIdleTime);
		
	sscanf(line,"%s	%d	%d	%d	%d	%d	%d	%d	%d	%d	%d	%d",
		str,&vpi,&vci,&encap,&channelMode,&napt,&channelStatus,&qos,&pcr,&cdvt,&scr,&mbs);
	
	//min=0;max=255;
	if (vpi<0 || vpi>255){
		printf("vpi error\n");
		return error;
	}
	entry->vpi = (unsigned char)vpi;
	
	//min=0;max=65535;
	if (vci<0 ||vci >65535){
		printf("vci error\n");
		return error;
	}
	entry->vci = (unsigned short)vci;
	
	//min=1;max=2;
	if (encap!=1 && encap!=2){
		printf("encap error\n");
		return error;
	}
	if (encap == 1)
		entry->encap = 1;
	else
		entry->encap = 0;

	//max=5;
	if (channelMode<1 || channelMode>5){
		printf("channelMode error\n");
		return error;
	}
	entry->cmode = channelMode-1;
	if (entry->cmode == ADSL_PPPoE)
		entry->mtu = 1452;
	else
		entry->mtu = 1500;
	
	if (entry->cmode != ADSL_BR1483) {
		//max=2;
		if (napt<0 || napt>1){
			printf("napt error\n");
			return error;
		}
		if (napt == 1)
			entry->napt = 1;
		else
			entry->napt = 0;
	} else {
		entry->napt = 0;
	}
	
	//max=2;
	if (channelStatus<1 || channelStatus>2){
		printf("channelStatus error\n");
		return error;
	}
	if (channelStatus==1)
		entry->enable = 1;
	else
		entry->enable = 0;
	
	entry->brmode = 1;
	
	if (qos != 0) {
		//max=4;
		if (qos < 1 || qos > 4){
			printf("QoS error\n");
			return error;
		}
		entry->qos = qos - 1;
	}
	
	//max=65535;
	if (pcr < 0 || pcr > 65535){
		printf("PCR error\n");
		return error;
	}
	entry->pcr = pcr;
	
	//max=0xffffffff;
	if (cdvt < 0 || cdvt > 0xffffffff){
		printf("CDVT error\n");
		return error;
	}
	entry->cdvt = cdvt;
	
	if (entry->qos == 2 || entry->qos == 3)
	{   
		//max=65535;
		if (scr < 0 || scr > 65535){
			printf("SCR error\n");
			return error;
		}
		entry->scr = scr;
		entry->mbs = mbs;      
	}
	
	// 1483 bridged
	if (entry->cmode == ADSL_BR1483)
	{
		entry->brmode = 0;
		entry->ipAddr[0]=0;
		entry->ipAddr[1]=0;
		entry->ipAddr[2]=0;
		entry->ipAddr[3]=0;
		entry->remoteIpAddr[0]=0;
		entry->remoteIpAddr[1]=0;
		entry->remoteIpAddr[2]=0;
		entry->remoteIpAddr[3]=0;
		entry->netMask[0]=0;
		entry->netMask[1]=0;
		entry->netMask[2]=0;
		entry->netMask[3]=0;
		entry->dgw=0;
//		entry->cdvt=0;

	} else // PPP connection
	if (entry->cmode == ADSL_PPPoE || entry->cmode == ADSL_PPPoA)
	{
		//printf("PPP Settings\n");
		sscanf(line,"%s	%d	%d	%d	%d	%d	%d	%d	%d	%d	%d	%d	%d	%d	%d	%d	%d	%s	%s	%s",
			str,&vpi,&vci,&encap,&channelMode,&napt,&channelStatus,&qos,&pcr,&cdvt,&scr,&mbs,
			&pppConnectType,&pppIdleTime,&defaultGW,&pppAuth,&mtu,pppUser,pppPasswd,pppAc);
		
		if (strlen(pppUser)==0){
			printf("pppUser error\n");
			return error;
		}
		strcpy(entry->pppUsername,pppUser);
		
		if (strlen(pppPasswd)==0){
			printf("pppPasswd error\n");
			return error;
		}
		strcpy(entry->pppPassword,pppPasswd);

		//min=1;max=3;
		if (pppConnectType<1 || pppConnectType>3){
			printf("pppConnectType error\n");
			return error;
		}

		if (1 == pppConnectType)
			entry->pppCtype = CONTINUOUS;
		else if (2 == pppConnectType)
			entry->pppCtype = CONNECT_ON_DEMAND;
		else
			entry->pppCtype = MANUAL;

//		if (entry->pppCtype != CONTINUOUS) {
		if (entry->pppCtype == CONNECT_ON_DEMAND) {	// Jenny
			//min=1;max=65535;
			if (pppIdleTime<1 || pppIdleTime>65535){
				printf("pppIdleTime error\n");
				return error;
			}
			entry->pppIdleTime = pppIdleTime;
		}
  
		if (entry->cmode == ADSL_PPPoE)
			strcpy(entry->pppACName, pppAc);

		// 0:AUTO, 1:PAP, 2:CHAP
		if (pppAuth < 0 || pppAuth > 2){
			printf("PPP Authentication error\n");
			return error;
		}
		entry->pppAuth = pppAuth;
		if (mtu > 0)
			entry->mtu = mtu;
	} else // WAN
	{
		//printf("WAN IP Settings :\n");
		sscanf(line,"%s	%d	%d	%d	%d	%d	%d	%d	%d	%d	%d	%d	%d	%s	%s	%s	%d	%d",
		str,&vpi,&vci,&encap,&channelMode,&napt,&channelStatus,&qos,&pcr,&cdvt,&scr,&mbs,
		&channelAddrType,LocalIPAddr,RemoteIPAddr,SubnetMask,&defaultGW,&ipunnum);
		//min=1;max=2;
		if (channelAddrType<1 || channelAddrType>2){
			printf("channelAddrType error\n");
			return error;
		}
		if (entry->cmode == ADSL_RT1483 && ipunnum == 1) {
			entry->ipunnumbered = 1;
		}
		
		if (channelAddrType == 1) {
			entry->ipDhcp = DHCP_DISABLED;
			if(!inet_aton(LocalIPAddr, (void *)&entry->ipAddr)){//1:success 0:fail
				printf("LocalIPAddr error\n");
				return error;
			}
			if(!inet_aton(RemoteIPAddr, (void *)&entry->remoteIpAddr)){//1:success 0:fail
				printf("RemoteIPAddr error\n");
				return error;
			}
			if(!inet_aton(SubnetMask, (void *)&entry->netMask)){//1:success 0:fail
				printf("SubnetMask error\n");
				return error;
			}	
		} else {
			entry->ipDhcp = DHCP_CLIENT;
		}
	}


	if (entry->cmode != ADSL_BR1483)
	{
		//max=2;
		if (defaultGW<0 || defaultGW>1){
			printf("defaultGW error\n");
			return error;
		}

		if (defaultGW==1)
			entry->dgw = 1;
		else
			entry->dgw = 0;
	}

	return 1;
}

int addChannelConfiguration(char *line)
{
	MIB_CE_ATM_VC_T entry;    
	
	// Added by Mason Yu for AC Name is not NULL(WAN can not get IP Address on PPPoE Mode)
	memset(&entry, 0x00, sizeof(entry));
	
	if (getVcInput(line,&entry, 0) == error){
		printf("getVcInput error\n");
		return error;
	}
	
	// now do the actual set.
	do {
		unsigned int ifMap;	// high half for PPP bitmap, low half for vc bitmap
		int i, cnt, drouted=-1, intVal;
		unsigned int totalEntry;
		MIB_CE_ATM_VC_Tp pEntry, pmyEntry;
		
		ifMap = 0;
		cnt=0;
		totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
	
		for (i=0; i<totalEntry; i++) {
			pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, i); /* get the specified chain record */
			
			if (pEntry == NULL)
			{
	  			printf("Get chain record error!\n");
				return error;
			}
			
			if (pEntry->vpi == entry.vpi && pEntry->vci == entry.vci)
			{
				cnt++;
				pmyEntry = pEntry;
			}
			
			ifMap |= 1 << (pEntry->ifIndex & 0x0f);	// vc map
			ifMap |= (1 << 16) << ((pEntry->ifIndex >> 4) & 0x0f);	// PPP map

			if ((drouted == -1) && pEntry->dgw && (pEntry->cmode != ADSL_BR1483))	// default route entry
				drouted = i;
		}
		
		if (cnt == 0)	// pvc not exists
		{
			entry.ifIndex = if_find_index(entry.cmode, ifMap);
			if (entry.ifIndex == 0xff)
			{
				printf("Error: Maximum number of VC exceeds !");
				return error;
			}
			else if (entry.ifIndex == 0xef)
			{
				printf("Error: Maximum number of PPP exceeds !");
				return error;
			}
		}
		else	// pvc exists
		{
			if (pmyEntry->cmode == ADSL_PPPoE)
			{
				if (cnt<MAX_POE_PER_VC)
				{	// get the pvc info.
					entry.qos = pmyEntry->qos;
					entry.pcr = pmyEntry->pcr;
					entry.encap = pmyEntry->encap;
					ifMap &= 0xffff0000; // don't care the vc part
					entry.ifIndex = if_find_index(entry.cmode, ifMap);
					if (entry.ifIndex == 0xef)
					{
						printf("Error: Maximum number of PPP exceeds !");
						return error;
					}
					entry.ifIndex &= 0xf0;
					entry.ifIndex |= (pmyEntry->ifIndex&0x0f);
				}
				else
				{
					printf("Maximum number of PPPoE connections exceeds in this vc!");
					return error;
				}
			}
			else
			{
				printf("Connection already exists!");
				return error;
			}
		}
		
		if (entry.cmode != ADSL_BR1483)
			if (entry.dgw && drouted != -1) {	// default route already existed
				pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, drouted);
				pEntry->dgw = 0;
			}

		intVal = mib_chain_add(MIB_ATM_VC_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			printf(strAddChainerror);
			return error;
		}
		else if (intVal == -1) {
			printf(strTableFull);
			return error;
		}
	
	} while (0);
	
	return 1;		
}

int addroutes(char *destNet,char *subMask,char *nextHop){
	MIB_CE_IP_ROUTE_T entry;
	struct in_addr *addr;
	char *str;
	int isnet, intVal;
	struct rtentry rt;
	/* Clean out the RTREQ structure. */
	memset((char *) &rt, 0, sizeof(struct rtentry));
		
	str = destNet;
	if ((isnet = INET_resolve(str, &rt.rt_dst)) < 0) {
		printf("Error: can't resolve %s\n", str);
		return error;
	}
	
	str = subMask;
	if ((isnet = INET_resolve(str, &rt.rt_genmask)) < 0) {
		printf("Error: can't resolve netmask %s\n", str);
		return error;
	}
		
	str = nextHop;
	if ((isnet = INET_resolve(str, &rt.rt_gateway)) < 0) {
		printf("Error: can't resolve gw %s\n", str);
		return error;
	}
		
	// add into configuration (chain record)
	addr = (struct in_addr *)&entry.destID;
	*addr = ((struct sockaddr_in *)&rt.rt_dst)->sin_addr;
	addr = (struct in_addr *)&entry.netMask;
	*addr = ((struct sockaddr_in *)&rt.rt_genmask)->sin_addr;
	addr = (struct in_addr *)&entry.nextHop;
	*addr = ((struct sockaddr_in *)&rt.rt_gateway)->sin_addr;
		
	intVal = mib_chain_add(MIB_IP_ROUTE_TBL, (unsigned char*)&entry);
	if (intVal == 0) {
		printf(strAddChainerror);
		return error;
	}
	else if (intVal == -1) {
		printf(strTableFull);
		return error;
	}
	return 1;
}

#ifdef IP_ACL
int addacl(char *IPAddr){
	MIB_CE_ACL_IP_T entry;
	struct in_addr *addr;				
	struct in_addr aclip;				
	int intVal;
				
	if(!inet_aton(IPAddr, (void *)&aclip)){//1:success 0:fail
			return error;
	}								
				
	// add into configuration (chain record)
	addr = (struct in_addr *)&entry.ipAddr;
	*addr = aclip;
					
	intVal = mib_chain_add(MIB_ACL_IP_TBL, (unsigned char*)&entry);
	if (intVal == 0) {
		printf(strAddChainerror);			
		return error;		
	}	
	else if (intVal == -1) {
		printf(strTableFull);			
		return error;		
	}	
	return 1;	
}
#endif

#ifdef WLAN_SUPPORT
#ifdef WLAN_ACL
int addWLanAC(unsigned char *mac)
{
	MIB_CE_WLAN_AC_T macEntry;
	MIB_CE_WLAN_AC_Tp pEntry;
	int entryNum, i, intVal;
	
	if (strlen(mac)!=12 || !string_to_hex(mac, macEntry.macAddr, 12)) {
		printf("Error! Invalid MAC address.");
		return error;
	}
	
	entryNum = mib_chain_total(MIB_WLAN_AC_TBL);
	if ( entryNum >= MAX_WLAN_AC_NUM ) {
		printf("Cannot add new entry because table is full!");
		return error;
	}
	
	// set to MIB. Check if entry exists
	for (i=0; i<entryNum; i++) {
		pEntry = (MIB_CE_WLAN_AC_Tp) mib_chain_get(MIB_WLAN_AC_TBL, i); /* get the specified chain record */
		if(pEntry == NULL)
		{
			printf("Get chain record error!\n");
			return error;
		}

		if (!memcmp(macEntry.macAddr, pEntry->macAddr, 6))
		{
			printf("Entry already exists!");
			return error;
		}
	}
	
	intVal = mib_chain_add(MIB_WLAN_AC_TBL, (unsigned char*)&macEntry);
	if (intVal == 0) {
		printf(strAddChainerror);
		return error;
	}
	else if (intVal == -1) {
		printf(strTableFull);
		return error;
	}
	return 1;
}
#endif
#endif

#ifdef WLAN_WDS
int addWLanWDS(unsigned char *mac)
{
	WDS_T macEntry;
	unsigned char entryNum;
	int intVal;
	
	if (strlen(mac)!=12 || !string_to_hex(mac, macEntry.macAddr, 12)) {
		printf("Error! Invalid MAC address.");
		return;
	}
	
	macEntry.comment[0] = '\0';
	
	if ( !mib_get(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
		printf("Get entry number error!");
		return error;
	}
	if ( (entryNum + 1) > MAX_WDS_NUM) {
		printf("Cannot add new entry because table is full!");
		return error;
	}
	
	// set to MIB.
	intVal = mib_chain_add(MIB_WDS_TBL, (void *)&macEntry);
	if (intVal == 0) {
		printf(strAddChainerror);
		return error;
	}
	else if (intVal == -1) {
		printf(strTableFull);
		return error;
	}
	entryNum++;
	if ( !mib_set(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
		printf("Set entry number error!");
		return error;
	}
	return 1;
}
#endif

unsigned char IFsel[8];
int showInf(void) // show all itf
{
	int ifnum=0;
	
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_Tp pEntry;
	
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<entryNum; i++) {

		pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, i); /* get the specified chain record */

		if(pEntry == NULL)
		{
  			printf("Get chain record error!\n");
			return -1;
		}
		
		if (pEntry->enable == 0)
			continue;
		IFsel[ifnum] = pEntry->ifIndex;
		ifnum++;
	}
	return ifnum;
}

#ifdef CONFIG_EXT_SWITCH
#ifdef IP_QOS
int Qosadd(unsigned char *sIp, unsigned char *sMask, unsigned short srcport,
			unsigned char *dIp, unsigned char *dMask, unsigned short dstport,
			unsigned char prType, unsigned char *phyport, unsigned char outIf, unsigned char *pri) {

	unsigned int  totalEntry, i;
	MIB_CE_IP_QOS_T entry;
	int intVal;
	unsigned long  mask, mbit;
	
	memset(&entry, 0, sizeof(entry));
	totalEntry = mib_chain_total(MIB_IP_QOS_TBL); /* get chain record size */
	
	/* Add new qos entry */
	if (totalEntry >= MAX_QOS_RULE)
	{
		printf("Error: Maximum number of Qos rule exceeds !");
		goto setErr_qos;
	}
		
	// protocol type
	// min=0; max=3;
	if (prType < 0 || prType > 3) {
		printf("Invalid protocol type!\n");
		goto setErr_qos;
	}
	entry.protoType = prType;
	
	// Source address
	if (sIp[0]) {
		if (!inet_aton(sIp, (struct in_addr *)&entry.sip)) {
			printf("Error! Source IP.");
			goto setErr_qos;
		}
		if (sMask[0]) {
			if (!inet_aton(sMask, (struct in_addr *)&mask)) {
				printf("Error! Source Netmask.");
				goto setErr_qos;
			}
			mask = htonl(mask);
			mbit=0; intVal=0;
			for (i=0; i<32; i++) {
				if (mask & 0x80000000) {
					if (intVal) {
						printf("Error! Source Netmask.");
						goto setErr_qos;
					}
					mbit++;
				}
				else
					intVal=1;
				mask <<= 1;
			}
			entry.smaskbit = mbit;
		}
		else
			entry.smaskbit = 32;
	}
	
	// source port
	if (entry.protoType == PROTO_TCP || entry.protoType == PROTO_UDP) {
		if (srcport < 1 || srcport > 65535) {
			printf("Error! Invalid source port.");
			goto setErr_qos;
		}
		entry.sPort = (unsigned short)srcport;
	}
	
	// Destination address
	if (dIp[0]) {
		if (!inet_aton(dIp, (struct in_addr *)&entry.dip)) {
			printf("Error! Destination IP.");
			goto setErr_qos;
		}
		if (!inet_aton(dMask, (struct in_addr *)&mask)) {
			if (mask==0) {
				printf("Error! Destination Netmask.");
				goto setErr_qos;
			}
			mask = htonl(mask);
			mbit=0;  intVal=0;
			for (i=0; i<32; i++) {
				if (mask&0x80000000) {
					if (intVal) {
						printf("Error! Destination Netmask.");
						goto setErr_qos;
					}
					mbit++;
				}
				else
					intVal=1;
				mask <<= 1;
			}
			entry.dmaskbit = mbit;
		}
		else
			entry.dmaskbit = 32;
	}

	// destination port
	if (entry.protoType == PROTO_TCP || entry.protoType == PROTO_UDP) {
		if (dstport < 1 || dstport > 65535) {
			printf("Error! Invalid source port.");
			goto setErr_qos;
		}
		entry.dPort = (unsigned short)dstport;
	}

	// physical port
	if (phyport[0]) {
		if ( !string_to_dec(phyport, &intVal) || intVal < 0 || intVal > 4) {
			printf("Error! Invalid physical port (0~4).");
			goto setErr_qos;
		}
		for (i=0; i<SW_PORT_NUM; i++) {
			if (virt2user[i] == intVal)
				break;
		}
		if (i!=SW_PORT_NUM)
			entry.phyPort = i;
		else
			entry.phyPort = 0xff;
	}
	else
		entry.phyPort = 0xff;
		
	// priority
	if (pri[0]) {
		if ( !string_to_dec(pri, &intVal) || intVal < 0 || intVal > 2) {
			printf("Error! Invalid priority (0~2).");
			goto setErr_qos;
		}
		entry.prior = (unsigned char)intVal;
	}
	else
		entry.prior = 2; // default to Low

	// Outbound Interface
	if (outIf < 1 || outIf > showInf()) {
		printf("Invalid out bound interface!\n");
		goto setErr_qos;
	}
	entry.outif = (unsigned char)IFsel[outIf-1];

	intVal = mib_chain_add(MIB_IP_QOS_TBL, (unsigned char*)&entry);
	if (intVal == 0) {
		printf(strAddChainerror);
		goto setErr_qos;
	}
	else if (intVal == -1) {
		printf(strTableFull);
		goto setErr_qos;
	}
	return 1;

setErr_qos:
	printf("Set QoS rule error!\n");
	return error;
}
#endif	// of IP_QOS
#endif	// of CONFIG_EXT_SWITCH

#ifdef PORT_TRIGGERING
#define PARSE_START	0
#define PARSE_DIGIT	1
#define PARSE_COMMA	2
#define PARSE_DASH	3
#define PARSE_BLANK	4
#define PARSE_NULL	5
#define PARSE_OTHER	6
#define PARSE_SECOND_DIGIT	7

/*
 *	Parse the trigger port range
 *	The trigger port format should follow these rules:
 *	. Each single port number should be seperated by comma or space
 *	. The port range should be something like: start-end (eg. 1234-1240)
 *		Note that there must not be any space around the dash
 *	. Example: "345,234,2345-2350,567"
 */
int parse_triggerPort(const char *inRange, char *outRange)
{
	int j, k;
	int cur_state, pre_state;
	
	k=0;
	pre_state = PARSE_START;
	
	for(j=0;j<GAMING_MAX_RANGE;j++)
	{
		if ((inRange[j]>='0')&&(inRange[j]<='9'))
			cur_state = PARSE_DIGIT;
		else if (inRange[j]==',')
			cur_state = PARSE_COMMA;
		else if (inRange[j]=='-')
			cur_state = PARSE_DASH;
		else if (inRange[j]==' ')
			cur_state = PARSE_BLANK;
		else if (inRange[j]==0)
			cur_state = PARSE_NULL;
		else
			cur_state = PARSE_OTHER;
		
		switch (cur_state) {
			case PARSE_BLANK:
				if (pre_state == PARSE_DIGIT) {
					outRange[k++] = ',';
					pre_state = PARSE_COMMA;
				}
				break;
			case PARSE_DIGIT:
				outRange[k++] = inRange[j];
				pre_state = PARSE_DIGIT;
				break;
			case PARSE_COMMA:
				if (pre_state == PARSE_DIGIT) {
					outRange[k++] = inRange[j];
					pre_state = PARSE_COMMA;
				}
				break;
			case PARSE_DASH:
				if (pre_state == PARSE_DIGIT) {
					outRange[k++] = inRange[j];
					pre_state = PARSE_DASH;
				}
				break;
			case PARSE_NULL:
				if (pre_state != PARSE_DIGIT && pre_state != PARSE_START)
					outRange[k-1] = 0;
				else
					outRange[k] = 0;
				return 1;
			default: break;
		}
	}
	return 0;
}

int addPortTriggering(unsigned char *name, unsigned char *tip,
					unsigned char *tcprange, unsigned char *udprange, unsigned char enable) {

	unsigned int  entryNum, i;
	MIB_CE_PORT_TRG_T entry;
	struct in_addr curIpAddr, curSubnet;
	int intVal;
	
	memset(&entry, 0, sizeof(entry));
	//name
	entryNum = mib_chain_total(MIB_PORT_TRG_TBL);
	unsigned long v1, v2, v3;

	// Check for name conflict
	for (i=0; i<entryNum; i++) {
		MIB_CE_PORT_TRG_Tp pCheckEntry;

		pCheckEntry = (MIB_CE_PORT_TRG_Tp) mib_chain_get(MIB_PORT_TRG_TBL, i);

		if ( pCheckEntry == NULL) {
			printf("Get table entry error\n");
			return error;
		}

		if (!strncmp(name, pCheckEntry->name, 32)) {
			printf("Game name conflict\n");
			return error;
		}
	}

	strncpy(entry.name, name, 32);

	//ip
	if (!inet_aton(tip, (struct in_addr *)&entry.ip)) {
		printf("Error! IP Address.");
		return error;
	}
	mib_get( MIB_ADSL_LAN_IP,  (void *)&curIpAddr);
	mib_get( MIB_ADSL_LAN_SUBNET,  (void *)&curSubnet);
	v1 = *((unsigned long *)entry.ip);
	v2 = *((unsigned long *)&curIpAddr);
	v3 = *((unsigned long *)&curSubnet);
	if ( (v1 & v3) != (v2 & v3) ) {
		printf("Invalid IP address! It should be set within the current subnet.\n");
		return error;
	}

	//tcp open port
	if (!parse_triggerPort(tcprange, entry.tcpRange)) {
		printf("Invalid TCP range !\n");
		return error;
	}

	//udp open port
	if (!parse_triggerPort(udprange, entry.udpRange)) {
		printf("Invalid UDP range !\n");
		return error;
	}

	//enable
	entry.enable = (unsigned char)enable;
		
	intVal = mib_chain_add(MIB_PORT_TRG_TBL, (unsigned char*)&entry);
	if (intVal == 0) {
		printf(strAddChainerror);
		return error;
	}
	else if (intVal == -1) {
		printf(strTableFull);
		return error;
	}
	return 1;
}
#endif

#ifdef NATIP_FORWARDING
int addipfw(char *lAddr, char *rAddr){

	MIB_CE_IP_FW_T entry;
	struct in_addr curIpAddr, curSubnet;
	unsigned long v1, v2, v3, v4;
	unsigned int entryNum, i;
	int intVal;

	memset(&entry, 0, sizeof(entry));
	// add into configuration (chain record)
	if (!inet_aton(lAddr, (struct in_addr *)&entry.local_ip)) {
		printf("Error! Local IP.");
		return error;		
	}

	if (!inet_aton(rAddr, (struct in_addr *)&entry.remote_ip)) {
		printf("Error! Remote IP.");
		return error;		
	}

	mib_get( MIB_ADSL_LAN_IP,  (void *)&curIpAddr);
	mib_get( MIB_ADSL_LAN_SUBNET,  (void *)&curSubnet);
	v1 = *((unsigned long *)entry.local_ip);
	v2 = *((unsigned long *)&curIpAddr);
	v3 = *((unsigned long *)&curSubnet);
	if ( (v1 & v3) != (v2 & v3) ) {
		printf("Invalid IP address! It should be set within the current subnet.\n");
		return error;
	}

	entryNum = mib_chain_total(MIB_IP_FW_TBL);

	// Check if there is any address conflict
	for (i=0; i<entryNum; i++) {
		MIB_CE_IP_FW_Tp pCheckEntry;

		pCheckEntry = (MIB_CE_IP_FW_Tp) mib_chain_get(MIB_IP_FW_TBL, i);
		v1 = *((unsigned long *)entry.local_ip);
		v2 = *((unsigned long *)entry.remote_ip);
		v3 = *((unsigned long *)pCheckEntry->local_ip);
		v4 = *((unsigned long *)pCheckEntry->remote_ip);

		if (v1==v3 || v2==v4) {
			printf("Error: Address conflict\n");
			return error;		
		}
	}

	intVal = mib_chain_add(MIB_IP_FW_TBL, (unsigned char*)&entry);
	if (intVal == 0) {
		printf(strAddChainerror);			
		return error;		
	}	
	else if (intVal == -1) {
		printf(strTableFull);			
		return error;		
	}	
	return 1;	
}
#endif

#ifdef URL_BLOCKING_SUPPORT
int addurl(char *url){

	if (url[0]) {
		MIB_CE_URL_FQDN_T entry;		
		int intVal;
		memset(&entry, 0, sizeof(entry));
		
		// add into configuration (chain record)
		strcpy(entry.fqdn, url);
		
		intVal = mib_chain_add(MIB_URL_FQDN_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			printf("Error: Add URL chain record.\n");
			return error;
		}
		else if (intVal == -1) {
			printf(strTableFull);
			return error;
		}
		return 1;
	}
	return error;
}
#endif

#define RC4_INT unsigned int
#define MAX_MESSAGE_LENGTH 2048
unsigned char en_cipherstream[MAX_MESSAGE_LENGTH+1];

void xor_block(int length, unsigned char *a, unsigned char *b, unsigned char *out)
{
    int i;
    for (i=0;i<length; i++)
    {
        out[i] = a[i] ^ b[i];
    }
}
/****************************************/
/* rc4_encrypt()                        */
/****************************************/
static __inline__ void swap(unsigned char *a, unsigned char *b)
{
    unsigned char tmp;

    tmp = *a;
    *a = *b;
    *b = tmp;
}
void rc4(
            unsigned char *key,
            int key_length,
            int cipherstream_length,
            unsigned char *cipherstream)
{
    int i, j, x;

#ifdef _USE_DRAM_
    unsigned char *s = rc4sbox;
    unsigned char *k = rc4kbox;
#else
    unsigned char s[256];
    unsigned char k[256];
#endif

    /* Create Key Stream */
    for (i=0; i<256; i++)
        k[i] = key[i % key_length];
    
    /* Initialize SBOX */
    for (j=0; j<256; j++)
        s[j] = j;
    
    /* Seed the SBOX */
    i = 0;
    for (j=0; j<256; j++)
    {
        i = (i + s[j] + k[j]) & 255;
        swap(&s[j], &s[i]);
    }

    /* Generate the cipherstream */
    j = 0;
    i = 0;

    for (x=0; x<cipherstream_length; x++)
    {
        j = (j + 1) & 255;
        i = (i + s[j]) & 255;
        swap(&s[j], &s[i]);
        cipherstream[x] = s[(s[j] + s[i]) & 255];
    };
}

void rc4_encrypt(
            unsigned char *key,
            int key_length,
            unsigned char *data,
            int data_length,
            unsigned char *ciphertext)
{

    rc4(key, key_length, data_length, en_cipherstream);
    xor_block(data_length, en_cipherstream, data, ciphertext);
}

int update_setting(char *line, FILE *fp){
	int len;
	char *str1, *str2, *tmp, ChainTestStr[20];

	//parse the line	
	sscanf(line, "%s", ChainTestStr);
	if (!strcmp(ChainTestStr, "</Config_Information_File>")) {
//		printf("End of config file...exit.\n");
		return 1;
	}
	if((strcmp(ChainTestStr,"<chain") != 0) && (ChainTestStr[1] != '/')){
		tmp = strtok(line, "\"");
		str1 = strtok(NULL, "\"");
		tmp = strtok(NULL, "\n");
		str2 = strchr(tmp, '\"') + 1;
		len = strcspn(str2, "\"");
		str2[len] = '\0';
	}
	
	if(!strcmp(ChainTestStr,"<chain")){
		tmp = strtok(line, "\"");
		str1 = strtok(NULL, "\"");
	//process mib-chain
		if(!strcmp(str1,"IP_PORT_FILTER_TBL")){
		//IP_PORT_FILTER_TBL
			unsigned char sip[16], smask[16], dip[16], dmask[16];
   			int filterMode,dir,prType;
   			int sfromPort, stoPort, dfromPort, dtoPort;
			
			strcpy(sip,"0.0.0.0");
			strcpy(smask,"0.0.0.0");
			strcpy(dip,"0.0.0.0");
			strcpy(dmask,"0.0.0.0");
			filterMode=dir=prType=sfromPort=stoPort=dfromPort=dtoPort=0;
			
			//clear orginal record
			if(chain_updated[MIB_IP_PORT_FILTER_TBL-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_IP_PORT_FILTER_TBL);//clear chain record 	
				chain_updated[MIB_IP_PORT_FILTER_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			while(1){
				char line2[255];
				fgets(line2,sizeof(line2),fp);
				if(line2[1]=='/')
					break;
				tmp = strtok(line2, "\"");
				str1 = strtok(NULL, "\"");
				tmp = strtok(NULL, "\n");
				str2 = strchr(tmp, '\"') + 1;
				len = strcspn(str2, "\"");
				str2[len] = '\0';
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1,"SrcIP"))
					strcpy(sip,str2);
				if(!strcmp(str1,"SrcMask"))
					strcpy(smask,str2);
				if(!strcmp(str1,"SrcPortStart"))
					sfromPort=atoi(str2);
				if(!strcmp(str1,"SrcPortEnd"))
					stoPort=atoi(str2);
				if(!strcmp(str1,"DstIP"))
					strcpy(dip,str2);
				if(!strcmp(str1,"DstMask"))
					strcpy(dmask,str2);	
				if(!strcmp(str1,"DstPortStart"))
					dfromPort=atoi(str2);
				if(!strcmp(str1,"DstPortEnd"))
					dtoPort=atoi(str2);
				if(!strcmp(str1,"FilterMode"))
					filterMode=atoi(str2);
				if(!strcmp(str1,"protocol"))
					prType=atoi(str2);
				if(!strcmp(str1,"Direction"))
					dir=atoi(str2);
			}
			return filterAdd((unsigned char)filterMode,sip,smask,(unsigned short)sfromPort,(unsigned short)stoPort,
                		dip,dmask,(unsigned short)dfromPort,(unsigned short)dtoPort,(unsigned char)dir,(unsigned char)prType);
		}
		
#ifdef MAC_FILTER
		if(!strcmp(str1,"MAC_FILTER_TBL")){
		//IP_PORT_FILTER_TBL
			unsigned char src_mac[12], dst_mac[12];
			int filterMode,dir;
			
			strcpy(src_mac,"0.0.0.0");
			strcpy(dst_mac,"0.0.0.0");
			filterMode=dir=0;
			
			//clear orginal record
			if(chain_updated[MIB_MAC_FILTER_TBL-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_MAC_FILTER_TBL);//clear chain record 	
				chain_updated[MIB_MAC_FILTER_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			while(1){
				char line2[255];
				fgets(line2,sizeof(line2),fp);
				if(line2[1]=='/')
					break;
				tmp = strtok(line2, "\"");
				str1 = strtok(NULL, "\"");
				tmp = strtok(NULL, "\n");
				str2 = strchr(tmp, '\"') + 1;
				len = strcspn(str2, "\"");
				str2[len] = '\0';
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1,"SrcMac"))
					strcpy(src_mac,str2);
				if(!strcmp(str1,"DstMac"))
					strcpy(dst_mac,str2);
				if(!strcmp(str1,"FilterMode"))
					filterMode=atoi(str2);
				if(!strcmp(str1,"Direction"))
					dir=atoi(str2);
			}
			return MACFilterAdd((unsigned char)filterMode,src_mac,
				dst_mac,(unsigned char)dir);
		}
#endif
		
		if(!strcmp(str1,"PORT_FW_TBL")){
		//IP_PORT_FILTER_TBL
			char ip[16];
			int fromPort,toPort,protocol;
			
			strcpy(ip,"0.0.0.0");
			fromPort=toPort=protocol=0;
			
			//clear orginal record
			if(chain_updated[MIB_PORT_FW_TBL-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_PORT_FW_TBL);//clear chain record 	
				chain_updated[MIB_PORT_FW_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			while(1){
				char line2[255];
				fgets(line2,sizeof(line2),fp);
				if(line2[1]=='/')
					break;
				tmp = strtok(line2, "\"");
				str1 = strtok(NULL, "\"");
				tmp = strtok(NULL, "\n");
				str2 = strchr(tmp, '\"') + 1;
				len = strcspn(str2, "\"");
				str2[len] = '\0';
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1,"IP"))
					strcpy(ip,str2);
				if(!strcmp(str1,"PortStart"))
					fromPort=atoi(str2);
				if(!strcmp(str1,"PortEnd"))
					toPort=atoi(str2);
				if(!strcmp(str1,"Protocol"))
					protocol=atoi(str2);
			}
			return addPortFW(ip,(unsigned)fromPort,(unsigned short)toPort,(unsigned char)protocol);
		}
		
		if(!strcmp(str1,"ATM_VC_TBL")){
		//ATM_VC_TBL
			int vpi,vci,encap,channelMode,napt,channelStatus,channelAddrType,defaultGW;
			int pppConnectType,pppIdleTime, pppAuth, mtu, ipunnum;
			int qos, pcr, cdvt, scr, mbs;
			char ConfigStr[100],LocalIPAddr[16], RemoteIPAddr[16], SubnetMask[16];
			char pppUser[30],pppPasswd[30], pppAC[30];
			vpi=vci=encap=channelMode=channelStatus=channelAddrType=defaultGW=pppConnectType=pppIdleTime=0;
			qos = cdvt = scr = mbs = pppAuth = ipunnum = 0;
			napt=1, pcr = ATM_MAX_US_PCR;
			strcpy(LocalIPAddr,"0.0.0.0");
			strcpy(RemoteIPAddr,"0.0.0.0");
			strcpy(SubnetMask,"0.0.0.0");
			pppUser[0] = pppPasswd[0] = pppAC[0] = '\0';
			
			//clear orginal record
			if(chain_updated[MIB_ATM_VC_TBL-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_ATM_VC_TBL);//clear chain record 	
				chain_updated[MIB_ATM_VC_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			while(1){
				char line2[255];
				fgets(line2,sizeof(line2),fp);
				if(line2[1]=='/')
					break;
				tmp = strtok(line2, "\"");
				str1 = strtok(NULL, "\"");
				tmp = strtok(NULL, "\n");
				str2 = strchr(tmp, '\"') + 1;
				len = strcspn(str2, "\"");
				str2[len] = '\0';
				//printf("name:%s	value:%s\n",str1,str2);
				//bridge
				if(!strcmp(str1,"VPI"))
					vpi=atoi(str2);
				if(!strcmp(str1,"VCI"))
					vci=atoi(str2);
				if(!strcmp(str1,"Encap"))
					encap=atoi(str2);
				if(!strcmp(str1,"ChannelMode"))
					channelMode=atoi(str2);
				if(!strcmp(str1,"NAPT"))
					napt=atoi(str2);
				if(!strcmp(str1,"ChannelStatus"))
					channelStatus=atoi(str2);
				//ppp
				if(!strcmp(str1,"pppUser"))
					strcpy(pppUser,str2);
				if(!strcmp(str1,"pppPasswd"))
					strcpy(pppPasswd,str2);
				if(!strcmp(str1,"pppConnectType"))
					pppConnectType=atoi(str2);
				if(!strcmp(str1,"pppIdleTime"))
					pppIdleTime=atoi(str2);
				if(!strcmp(str1,"pppACName"))
					strcpy(pppAC,str2);
				if(!strcmp(str1,"pppAuth"))
					pppAuth=atoi(str2);
				if(!strcmp(str1,"MTU"))
					mtu=atoi(str2);
					
				//dhcp	
				if(!strcmp(str1,"ChannelAddrType"))
					channelAddrType=atoi(str2);
				if(!strcmp(str1,"LocalIPAddr"))
					strcpy(LocalIPAddr,str2);
				if(!strcmp(str1,"RemoteIPAddr"))
					strcpy(RemoteIPAddr,str2);
				if(!strcmp(str1,"SubnetMask"))
					strcpy(SubnetMask,str2);
				if(!strcmp(str1,"DefaultGW"))
					defaultGW=atoi(str2);

				//ATM setting
				if(!strcmp(str1,"QoS"))
					qos=atoi(str2);
				if(!strcmp(str1,"PCR"))
					pcr=atoi(str2);
				if(!strcmp(str1,"CDVT"))
					cdvt=atoi(str2);
				if(!strcmp(str1,"SCR"))
					scr=atoi(str2);
				if(!strcmp(str1,"MBS"))
					mbs=atoi(str2);

				if(!strcmp(str1,"IPUnnumbered"))
					ipunnum=atoi(str2);
			}
			//printf("channelMode:%d\n",channelMode);
			if(channelMode==1){
				//ATM_VC_TBL VPI VCI Encap ChannelMode napt ChannelStatus qos pcr cdvt scr mbs
				sprintf(ConfigStr,"ATM_VC_TBL %d %d %d %d %d %d %d %d %d %d %d",vpi,vci,encap,
					channelMode,napt,channelStatus,qos,pcr,cdvt,scr,mbs);
			}
			if((channelMode==3) || (channelMode==4)){
				//ATM_VC_TBL VPI VCI Encap ChannelMode napt ChannelStatus qos pcr cdvt scr mbs
				// pppConnectType pppIdleTime defaultGW pppAuth mtu pppUser pppPasswd pppAC
				sprintf(ConfigStr,"ATM_VC_TBL %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %s %s %s",
					vpi,vci,encap,channelMode,napt,channelStatus,qos,pcr,cdvt,scr,mbs,
					pppConnectType,pppIdleTime,defaultGW,pppAuth,mtu,pppUser,pppPasswd,pppAC);
			}
			if((channelMode==2) || (channelMode==5)){
				//ATM_VC_TBL VPI VCI Encap ChannelMode napt ChannelStatus qos pcr cdvt scr mbs
				// channelAddrType LocalIPAddr RemoteIPAddr SubnetMask defaultGW ipunnum
				sprintf(ConfigStr,"ATM_VC_TBL %d %d %d %d %d %d %d %d %d %d %d %d %s %s %s %d %d",
					vpi,vci,encap,channelMode,napt,channelStatus,qos,pcr,cdvt,scr,mbs,
					channelAddrType,LocalIPAddr,RemoteIPAddr,SubnetMask,defaultGW,ipunnum);
			}
			return addChannelConfiguration(ConfigStr);
		}
		
		if(!strcmp(str1,"IP_ROUTE_TBL")){
		//IP_ROUTE_TBL
			char DstIP[16],Mask[16],NextHop[16];
			
			strcpy(DstIP,"0.0.0.0");
			strcpy(Mask,"0.0.0.0");
			strcpy(NextHop,"0.0.0.0");
			
			//clear orginal record
			if(chain_updated[MIB_IP_ROUTE_TBL-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_IP_ROUTE_TBL);//clear chain record 	
				chain_updated[MIB_IP_ROUTE_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			while(1){
				char line2[255];
				fgets(line2,sizeof(line2),fp);
				if(line2[1]=='/')
					break;
				tmp = strtok(line2, "\"");
				str1 = strtok(NULL, "\"");
				tmp = strtok(NULL, "\n");
				str2 = strchr(tmp, '\"') + 1;
				len = strcspn(str2, "\"");
				str2[len] = '\0';
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1,"DstIP"))
					strcpy(DstIP,str2);
				if(!strcmp(str1,"Mask"))
					strcpy(Mask,str2);
				if(!strcmp(str1,"NextHop"))
					strcpy(NextHop,str2);
			}
			return addroutes(DstIP,Mask,NextHop);
		}
		
#ifdef IP_ACL
		if(!strcmp(str1,"ACL_IP_TBL")){
		//ACL_IP_TBL
			char IPAddr[16];
			
			strcpy(IPAddr,"0.0.0.0");

			//clear orginal record
			if(chain_updated[MIB_ACL_IP_TBL-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_ACL_IP_TBL);//clear chain record 	
				chain_updated[MIB_ACL_IP_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			while(1){
				char line2[255];
				fgets(line2,sizeof(line2),fp);
				if(line2[1]=='/')
					break;
				tmp = strtok(line2, "\"");
				str1 = strtok(NULL, "\"");
				tmp = strtok(NULL, "\n");
				str2 = strchr(tmp, '\"') + 1;
				len = strcspn(str2, "\"");
				str2[len] = '\0';
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1,"IPAddr"))
					strcpy(IPAddr,str2);
			}
			return addacl(IPAddr);
		}
#endif
		
#ifdef WLAN_SUPPORT
#ifdef WLAN_ACL
		if(!strcmp(str1,"WLAN_AC_TBL")){
		//WLAN_AC_TBL
			char MacAddr[12];
			
			strcpy(MacAddr,"000000000000");

			//clear orginal record
			if(chain_updated[MIB_WLAN_AC_TBL-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_WLAN_AC_TBL);//clear chain record 	
				chain_updated[MIB_WLAN_AC_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			while(1){
				char line2[255];
				fgets(line2,sizeof(line2),fp);
				if(line2[1]=='/')
					break;
				tmp = strtok(line2, "\"");
				str1 = strtok(NULL, "\"");
				tmp = strtok(NULL, "\n");
				str2 = strchr(tmp, '\"') + 1;
				len = strcspn(str2, "\"");
				str2[len] = '\0';
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1,"MacAddr"))
					strcpy(MacAddr,str2);
			}
			return addWLanAC(MacAddr);
		}
#endif
#endif
		if(!strcmp(str1,"ACC_TBL")){
		//ACC_TBL
			MIB_CE_ACC_Tp pEntry;
			MIB_CE_ACC_T entry;

			pEntry = (MIB_CE_ACC_Tp) mib_chain_get(MIB_ACC_TBL, 0);
			if (!pEntry) {
				memset(&entry, '\0', sizeof(MIB_CE_ACC_T));
				mib_chain_add(MIB_ACC_TBL, (unsigned char*)&entry);
				pEntry = (MIB_CE_ACC_Tp) mib_chain_get(MIB_ACC_TBL, 0);
			}

			while(1){
				char line2[255];
				fgets(line2,sizeof(line2),fp);
				if(line2[1]=='/')
					break;
				tmp = strtok(line2, "\"");
				str1 = strtok(NULL, "\"");
				tmp = strtok(NULL, "\n");
				str2 = strchr(tmp, '\"') + 1;
				len = strcspn(str2, "\"");
				str2[len] = '\0';
				//printf("name:%s	value:%s\n",str1,str2);
				//telnet
				if(!strcmp(str1,"Telnet_LAN")){
					if(atoi(str2)==1){
						pEntry->telnet |= 0x02;
					}
				}
				if(!strcmp(str1,"Telnet_WAN")){
					if(atoi(str2)==1){
						pEntry->telnet |= 0x01;	
					}
				}
				//ftp
				if(!strcmp(str1,"Ftp_LAN")){
					if(atoi(str2)==1){
						pEntry->ftp |= 0x02;	
					}
				}
				if(!strcmp(str1,"Ftp_WAN")){
					if(atoi(str2)==1){
						pEntry->ftp |= 0x01;	
					}
				}
				//tftp
				if(!strcmp(str1,"Tftp_LAN")){
					if(atoi(str2)==1){
						pEntry->tftp |= 0x02;	
					}
				}
				if(!strcmp(str1,"Tftp_WAN")){
					if(atoi(str2)==1){
						pEntry->tftp |= 0x01;	
					}
				}
				//web
				if(!strcmp(str1,"Web_LAN")){
					if(atoi(str2)==1){
						pEntry->web |= 0x02;	
					}
				}
				if(!strcmp(str1,"Web_WAN")){
					if(atoi(str2)==1){
						pEntry->web |= 0x01;	
					}
				}
				//snmp
				if(!strcmp(str1,"Snmp_LAN")){
					if(atoi(str2)==1){
						pEntry->snmp |= 0x02;	
					}
				}
				if(!strcmp(str1,"Snmp_WAN")){
					if(atoi(str2)==1){
						pEntry->snmp |= 0x01;	
					}
				}
				//ssh
				if(!strcmp(str1,"Ssh_LAN")){
					if(atoi(str2)==1){
						pEntry->ssh |= 0x02;	
					}
				}
				if(!strcmp(str1,"Ssh_WAN")){
					if(atoi(str2)==1){
						pEntry->ssh |= 0x01;	
					}
				}
				//icmp
				if(!strcmp(str1,"Icmp_LAN")){
					if(atoi(str2)==1){
						pEntry->icmp |= 0x02;	
					}
				}
				if(!strcmp(str1,"Icmp_WAN")){
					if(atoi(str2)==1){
						pEntry->icmp |= 0x01;	
					}
				}
			}
			return 1;
		}
		
#ifdef WLAN_SUPPORT
		if(!strcmp(str1,"WLAN_WDS_TBL")){
		//WLAN_WDS_TBL
			char MacAddr[12];
			
			strcpy(MacAddr,"000000000000");

			//clear orginal record
			if(chain_updated[MIB_WDS_TBL-CHAIN_ENTRY_TBL_ID]==0){
				unsigned char entryNum=0;
				mib_chain_clear(MIB_WDS_TBL);//clear chain record 	
				chain_updated[MIB_WDS_TBL-CHAIN_ENTRY_TBL_ID]=1;
				if ( !mib_set(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
					printf("Set entry number error!");
					return error;
				}
			}
			while(1){
				char line2[255];
				fgets(line2,sizeof(line2),fp);
				if(line2[1]=='/')
					break;
				tmp = strtok(line2, "\"");
				str1 = strtok(NULL, "\"");
				tmp = strtok(NULL, "\n");
				str2 = strchr(tmp, '\"') + 1;
				len = strcspn(str2, "\"");
				str2[len] = '\0';
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1,"MacAddr"))
					strcpy(MacAddr,str2);
			}
			return addWLanWDS(MacAddr);
		}
#endif

#ifdef CONFIG_EXT_SWITCH
#if 0
		if(!strcmp(str1,"SW_PORT_TBL")){
		//SW_PORT_TBL
   			int switchno, pvcIf;
			MIB_CE_SW_PORT_Tp pEntry;

			while(1){
				char line2[255];
				fgets(line2, sizeof(line2), fp);
				if(line2[1] == '/')
					break;
				tmp = strtok(line2, "\"");
				str1 = strtok(NULL, "\"");
				tmp = strtok(NULL, "\n");
				str2 = strchr(tmp, '\"') + 1;
				len = strcspn(str2, "\"");
				str2[len] = '\0';
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1, "Switch"))
					if (str2[0]) {
						switchno = atoi(str2);
						pEntry = (MIB_CE_SW_PORT_Tp) mib_chain_get(MIB_SW_PORT_TBL, switchno - 1);

						if(!strcmp(str1,"PVCInterface")) {
							pvcIf = atoi(str2);
							if (pvcIf > showInf()) {
								printf("Invalid PVC interface!\n");
								return error;
							}
							if (pvcIf <= 0)
								pEntry->pvcItf = (unsigned char)0xff;
							else
								pEntry->pvcItf = (unsigned char)IFsel[pvcIf-1];
						}
						if(!strcmp(str1,"InterfaceGroup"))
							pEntry->itfGroup = (unsigned char)atoi(str2);
						if(!strcmp(str1,"PVID"))
							pEntry->pvid = (unsigned char)(atoi(str2) - 1);
						if(!strcmp(str1,"EgressTag"))
							pEntry->egressTagAction = (unsigned char)(atoi(str2) - 1);
						if(!strcmp(str1,"LinkMode"))
							pEntry->linkMode = (unsigned char)(atoi(str2) - 1);
					}
			}
			return 1;
		}
#endif

#ifdef IP_QOS
		if(!strcmp(str1,"IP_QOS_TBL")){
		//IP_QOS_TBL
			unsigned char sip[16], smask[16], dip[16], dmask[16], pri[2], phyPort[2];
   			int prType, outIf;
   			int srcPort, dstPort;
			
			strcpy(sip, "0.0.0.0");
			strcpy(smask, "0.0.0.0");
			strcpy(dip, "0.0.0.0");
			strcpy(dmask, "0.0.0.0");
			pri[0] = phyPort[0] = '\0';
			prType = outIf = srcPort = dstPort = 0;
			
			//clear orginal record
			if(chain_updated[MIB_IP_QOS_TBL-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_IP_QOS_TBL);//clear chain record 	
				chain_updated[MIB_IP_QOS_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			while(1){
				char line2[255];
				fgets(line2, sizeof(line2), fp);
				if(line2[1] == '/')
					break;
				tmp = strtok(line2, "\"");
				str1 = strtok(NULL, "\"");
				tmp = strtok(NULL, "\n");
				str2 = strchr(tmp, '\"') + 1;
				len = strcspn(str2, "\"");
				str2[len] = '\0';
				if(!strcmp(str1, "SrcIP"))
					strcpy(sip, str2);
				if(!strcmp(str1, "SrcMask"))
					strcpy(smask, str2);
				if(!strcmp(str1, "SrcPort"))
					srcPort = atoi(str2);
				if(!strcmp(str1, "DstIP"))
					strcpy(dip, str2);
				if(!strcmp(str1, "DstMask"))
					strcpy(dmask, str2);	
				if(!strcmp(str1,"DstPort"))
					dstPort = atoi(str2);
				if(!strcmp(str1,"Protocol"))
					prType = atoi(str2);
				if(!strcmp(str1,"PhyPort"))
					strcpy(phyPort, str2);	
				if(!strcmp(str1,"OutInf"))
					outIf = atoi(str2);
				if(!strcmp(str1,"Prior"))
					strcpy(pri, str2);	
			}
			return Qosadd(sip, smask, (unsigned short)srcPort, dip, dmask, (unsigned short)dstPort,
				(unsigned char)prType, phyPort, (unsigned char)outIf, pri);
		}
#endif	// of IP_QOS
#endif	// of CONFIG_EXT_SWITCH
	
#ifdef PORT_TRIGGERING
		if(!strcmp(str1,"PORT_TRG_TBL")){
		//PORT_TRG_TBL
			unsigned char tip[16], name[35], tcprange[35], udprange[35];
   			int enable;
			
			strcpy(tip, "0.0.0.0");
			name[0] = tcprange[0] = udprange[0] = '\0';
			enable = 0;
			
			//clear orginal record
			if(chain_updated[MIB_PORT_TRG_TBL-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_PORT_TRG_TBL);//clear chain record 	
				chain_updated[MIB_PORT_TRG_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			while(1){
				char line2[255];
				fgets(line2, sizeof(line2), fp);
				if(line2[1] == '/')
					break;
				tmp = strtok(line2, "\"");
				str1 = strtok(NULL, "\"");
				tmp = strtok(NULL, "\n");
				str2 = strchr(tmp, '\"') + 1;
				len = strcspn(str2, "\"");
				str2[len] = '\0';
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1, "Name"))
					strcpy(name, str2);
				if(!strcmp(str1, "IP"))
					strcpy(tip, str2);
				if(!strcmp(str1, "TcpRange"))
					strcpy(tcprange, str2);	
				if(!strcmp(str1,"UdpRange"))
					strcpy(udprange, str2);	
				if(!strcmp(str1,"Enable"))
					enable = atoi(str2);
			}
			return addPortTriggering(name, tip, tcprange, udprange, enable);
		}
#endif

#ifdef NATIP_FORWARDING
		if(!strcmp(str1,"IP_FW_TBL")){
		//IP_FW_TBL
			unsigned char lip[16], rip[16];

			strcpy(lip, "0.0.0.0");
			strcpy(rip, "0.0.0.0");
			
			//clear orginal record
			if(chain_updated[MIB_IP_FW_TBL-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_IP_FW_TBL);//clear chain record 	
				chain_updated[MIB_IP_FW_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			while(1){
				char line2[255];
				fgets(line2, sizeof(line2), fp);
				if(line2[1] == '/')
					break;
				tmp = strtok(line2, "\"");
				str1 = strtok(NULL, "\"");
				tmp = strtok(NULL, "\n");
				str2 = strchr(tmp, '\"') + 1;
				len = strcspn(str2, "\"");
				str2[len] = '\0';
				printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1, "LocalIP"))
					strcpy(lip, str2);
				if(!strcmp(str1, "RemoteIP"))
					strcpy(rip, str2);
			}
			return addipfw(lip, rip);
		}
#endif

#ifdef URL_BLOCKING_SUPPORT
		if(!strcmp(str1,"URL_FQDN_TBL")){
		//URL_FQDN_TBL
			unsigned char url[130];
			
			url[0] = '\0';
			
			//clear orginal record
			if(chain_updated[MIB_URL_FQDN_TBL-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_URL_FQDN_TBL);//clear chain record 	
				chain_updated[MIB_URL_FQDN_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			while(1){
				char line2[255];
				fgets(line2, sizeof(line2), fp);
				if(line2[1] == '/')
					break;
				tmp = strtok(line2, "\"");
				str1 = strtok(NULL, "\"");
				tmp = strtok(NULL, "\n");
				str2 = strchr(tmp, '\"') + 1;
				len = strcspn(str2, "\"");
				str2[len] = '\0';
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1, "URL"))
					strcpy(url, str2);
			}
			return addurl(url);
		}
#endif
	}
	
	if(!strcmp(str1, "LAN_IP_ADDR")){
	//LAN_IP_ADDR IP
		struct in_addr inIp;
		if(!inet_aton(str2, (void *)&inIp)){//1:success 0:fail
			return error;
		}
		if ( !mib_set(MIB_ADSL_LAN_IP, (void *)&inIp)) {
			printf("Set IP-address error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "LAN_SUBNET")){
	//LAN_SUBNET
		struct in_addr inSubnet;
		if(!inet_aton(str2, (void *)&inSubnet)) {
			return error;
		}
		if (!mib_set(MIB_ADSL_LAN_SUBNET, (void *)&inSubnet)) {
			printf("Set subnet-mask error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "LAN_DHCP")){
	//LAN_DHCP
		unsigned char vDhcp = atoi(str2);
		if (!mib_set(MIB_ADSL_LAN_DHCP, (void *)&vDhcp)) {
			printf("Set DHCP error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "LAN_DHCP_START")){
	//LAN DHCP_CLIENT_START
		unsigned char uStart = atoi(str2);
		if (!mib_set(MIB_ADSL_LAN_CLIENT_START, (void *)&uStart)) {
			printf("Set DHCP client start range error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "LAN_DHCP_END")){
	//LAN DHCP_CLIENT_END
		unsigned char uEnd = atoi(str2);
		if (!mib_set(MIB_ADSL_LAN_CLIENT_END, (void *)&uEnd)) {
			printf("Set DHCP client end range error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "LAN_DHCP_LEASE")){
	//LAN_DHCP_LEASE
		unsigned int uLTime = atoi(str2);
		if (!mib_set(MIB_ADSL_LAN_DHCP_LEASE, (void *)&uLTime)) {
			printf("Set DHCP lease time error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "LAN_DHCP_DOMAIN")){
	//LAN_DHCP_DOMAIN
		if (!mib_set(MIB_ADSL_LAN_DHCP_DOMAIN, (void *)str2)) {
			printf("Set DHCP Domain Name error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "LAN_RIP")){
	//LAN_RIP
		unsigned char vRip = atoi(str2);
		if (!mib_set(MIB_ADSL_LAN_RIP, (void *)&vRip)) {
			printf("Set RIP error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "LAN_AUTOSEARCH")){
	//LAN_IP_AUTOSEARCH
		unsigned char vAuto = atoi(str2);
		if (!mib_set(MIB_ADSL_LAN_AUTOSEARCH, (void *)&vAuto)) {
			printf("Set LAN IP auto-search error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "DNS_MODE")){
	//WAN_DNS_MODE
		unsigned char vDns = atoi(str2);
		if (!mib_set(MIB_ADSL_WAN_DNS_MODE, (void *)&vDns)) {
			printf("Set DNS mode error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "DNS1")){
	//WAN_DNS1
		struct in_addr inDns;
		if(!inet_aton(str2, (void *)&inDns)) {
			return error;
		}
		if (!mib_set(MIB_ADSL_WAN_DNS1, (void *)&inDns)) {
			printf("Set DNS error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "DNS2")){
	//WAN_DNS2
		struct in_addr inDns;
		if(!inet_aton(str2, (void *)&inDns)) {
			return error;
		}
		if (!mib_set(MIB_ADSL_WAN_DNS2, (void *)&inDns)) {
			printf("Set DNS error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "DNS3")){
	//WAN_DNS3
		struct in_addr inDns;
		if(!inet_aton(str2, (void *)&inDns)) {
			return error;
		}
		if (!mib_set(MIB_ADSL_WAN_DNS3, (void *)&inDns)) {
			printf("Set DNS error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "DHCPS")){
	//WAN_DHCP_SERVER_IP
		struct in_addr inDhcps;
		if(!inet_aton(str2, (void *)&inDhcps)) {
			return error;
		}
		if (!mib_set(MIB_ADSL_WAN_DHCPS, (void *)&inDhcps)) {
			printf("Set DHCP Server Address error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "DHCP_MODE")){
	//DHCP_MODE
		unsigned char vDhcp = atoi(str2);
		if (!mib_set(MIB_DHCP_MODE, (void *)&vDhcp)) {
			printf("Set DHCP mode error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "ADSL_MODE")){
	//ADSL_MODE
		unsigned short uADSL = atoi(str2);
		if (!mib_set(MIB_ADSL_MODE, (void *)&uADSL)) {
			printf("Set ADSL mode error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "ADSL_OLR")){
	//ADSL_OLR
		unsigned char vOLR = atoi(str2);
		if (!mib_set(MIB_ADSL_OLR, (void *)&vOLR)) {
			printf("Set ADSL OLR error!");
			return error;
		}
	}
	
#ifdef CONFIG_USER_ROUTED_ROUTED
	if(!strcmp(str1, "RIP_ENABLE")){
	//RIP_ENABLE
		unsigned char vRip = atoi(str2);
		if (!mib_set(MIB_RIP_ENABLE, (void *)&vRip)) {
			printf("Set RIP enable flag error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "RIP_VERSION")){
	//RIP_VERSION
		unsigned char vRipV = atoi(str2);
		if (!mib_set(MIB_RIP_VERSION, (void *)&vRipV)) {
			printf("Set RIP version error!");
			return error;
		}
	}
#endif
	
	if(!strcmp(str1, "IPF_OUT_ACTION")){
	//IPF_OUT_ACTION
		unsigned char vIPF = atoi(str2);
		if (!mib_set(MIB_IPF_OUT_ACTION, (void *)&vIPF)) {
			printf("Set Outgoing Default Filter Action error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "IPF_IN_ACTION")){
	//IPF_IN_ACTION
		unsigned char vIPF = atoi(str2);
		if (!mib_set(MIB_IPF_IN_ACTION, (void *)&vIPF)) {
			printf("Set Incoming Default Filter Action error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "MACF_OUT_ACTION")){
	//MACF_OUT_ACTION
		unsigned char vMACF = atoi(str2);
		if (!mib_set(MIB_MACF_OUT_ACTION, (void *)&vMACF)) {
			printf("Set Outgoing MAC Filter Action error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "MACF_IN_ACTION")){
	//MACF_IN_ACTION
		unsigned char vMACF = atoi(str2);
		if (!mib_set(MIB_MACF_IN_ACTION, (void *)&vMACF)) {
			printf("Set Incoming MAC Filter Action error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "PORT_FW_ENABLE")){
	//PORT_FW_ENABLE
		unsigned char vPF = atoi(str2);
		if (!mib_set(MIB_PORT_FW_ENABLE, (void *)&vPF)) {
			printf("Set port forwarding enabled flag error!");
			return error;
		}
	}
	
#ifdef NATIP_FORWARDING
	if(!strcmp(str1, "IP_FW_ENABLE")){
	//IP_FW_ENABLE
		unsigned char vIPF = atoi(str2);
		if (!mib_set(MIB_IP_FW_ENABLE, (void *)&vIPF)) {
			printf("Set IP forwarding enabled flag error!");
			return error;
		}
	}
#endif
	
	if(!strcmp(str1, "DMZ_ENABLE")){
	//DMZ_ENABLE
		unsigned char vDMZ = atoi(str2);
		if (!mib_set(MIB_DMZ_ENABLE, (void *)&vDMZ)) {
			printf("Set DMZ enabled flag error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "DMZ_IP")){
	//DMZ_IP
		struct in_addr inDMZ;
		if(!inet_aton(str2, (void *)&inDMZ)) {
			return error;
		}
		if (!mib_set(MIB_DMZ_IP, (void *)&inDMZ)) {
			printf("Set DMZ IP Address error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "USER_NAME")){
	//USER_NAME
		if (!mib_set(MIB_USER_NAME, (void *)str2)) {
			printf("Set user name error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "USER_PASSWORD")){
	//USER_PASSWORD
		if (!mib_set(MIB_USER_PASSWORD, (void *)str2)) {
			printf("Set user password error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "INIT_LINE")){
	//INIT_LINE
		unsigned char vInit = atoi(str2);
		if (!mib_set(MIB_INIT_LINE, (void *)&vInit)) {
			printf("Set Init Line error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "INIT_SCRIPT")){
	//INIT_SCRIPT
		unsigned char vInit = atoi(str2);
		if (!mib_set(MIB_INIT_SCRIPT, (void *)&vInit)) {
			printf("Set Init script error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "SNMP_SYS_DESCR")){
	//SNMP_SYS_DESCR
		if (!mib_set(MIB_SNMP_SYS_DESCR, (void *)str2)) {
			printf("Set snmpSysDescr mib error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "SNMP_SYS_OID")){
	//SNMP_SYS_OBJECT_ID
		if (!mib_set(MIB_SNMP_SYS_OID, (void *)str2)) {
			printf("Set snmpSysObjectID mib error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "SNMP_SYS_CONTACT")){
	//SNMP_SYS_CONTACT
		if (!mib_set(MIB_SNMP_SYS_CONTACT, (void *)str2)) {
			printf("Set snmpSysContact mib error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "SNMP_SYS_NAME")){
	//SNMP_SYS_NAME
		if (!mib_set(MIB_SNMP_SYS_NAME, (void *)str2)) {
			printf("Set snmpSysName mib error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "SNMP_SYS_LOCATION")){
	//SNMP_SYS_LOCATION
		if (!mib_set(MIB_SNMP_SYS_LOCATION, (void *)str2)) {
			printf("Set snmpSysLocation mib error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "SNMP_TRAP_IP")){
	//SNMP_TRAP_IP
		struct in_addr inSNMP;
		if(!inet_aton(str2, (void *)&inSNMP)) {
			return error;
		}
		if (!mib_set(MIB_SNMP_TRAP_IP, (void *)&inSNMP)) {
			printf("Set snmpTrapIpAddr mib error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "SNMP_COMM_RO")){
	//SNMP_COMM_RO
		if (!mib_set(MIB_SNMP_COMM_RO, (void *)str2)) {
			printf("Set snmpCommunityRO mib error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "SNMP_COMM_RW")){
	//SNMP_COMM_RW
		if (!mib_set(MIB_SNMP_COMM_RW, (void *)str2)) {
			printf("Set snmpCommunityRW mib error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "VC_AUTOSEARCH")){
	//VC_AUTOSEARCH
		unsigned char vAuto = atoi(str2);
		if (!mib_set(MIB_ATM_VC_AUTOSEARCH, (void *)&vAuto)) {
			printf("Set VC auto-search error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "BR_AGEING_TIME")){
	//BRIDGE_AGEING_TIME
		unsigned short uATime = atoi(str2);
		if (!mib_set(MIB_BRCTL_AGEINGTIME, (void *)&uATime)) {
			printf("Set bridge ageing time error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "BR_STP_ENABLED")){
	//BRIDGE_STP_ENABLED
		unsigned char vBSTP = atoi(str2);
		if (!mib_set(MIB_BRCTL_STP, (void *)&vBSTP)) {
			printf("Set bridge STP enabled flag error!");
			return error;
		}
	}
	
#ifdef CONFIG_EXT_SWITCH
	if(!strcmp(str1, "MP_MODE")){
	//MULTI_PORT_MODE
		unsigned char vMP = atoi(str2);
		if (!mib_set(MIB_MPMODE, (void *)&vMP)) {
			printf("Set multiport mode error!");
			return error;
		}
	}
#endif

#ifdef CONFIG_USER_IGMPPROXY
	if(!strcmp(str1, "IGMP_PROXY")){
	//IGMP_PROXY
		unsigned char vIGMP = atoi(str2);
		if (!mib_set(MIB_IGMP_PROXY, (void *)&vIGMP)) {
			printf("Set IGMP proxy error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "IGMP_PROXY_ITF")){
	//IGMP_PROXY_ITF
		unsigned char vIGMP = atoi(str2);
		if (!mib_set(MIB_IGMP_PROXY_ITF, (void *)&vIGMP)) {
			printf("Set IGMP proxy interface index error!");
			return error;
		}
	}
#endif
	
#ifdef IP_PASSTHROUGH
	if(!strcmp(str1, "IPPT_ITF")){
	//IPPT_ITF
		unsigned char vIPPT = atoi(str2);
		if (!mib_set(MIB_IPPT_ITF, (void *)&vIPPT)) {
			printf("Set IP passthrough interface index error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "IPPT_LEASE")){
	//IPPT_LEASE_TIME
		unsigned int uLTime = atoi(str2);
		if (!mib_set(MIB_IPPT_LEASE, (void *)&uLTime)) {
			printf("Set IP passthrough lease time error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "IPPT_LANACC")){
	//IPPT_LAN_ACCESS
		unsigned char vIPPT = atoi(str2);
		if (!mib_set(MIB_IPPT_LANACC, (void *)&vIPPT)) {
			printf("Set IP passthrough LAN access error!");
			return error;
		}
	}
#endif
	
	if(!strcmp(str1, "SPC_ENABLED")){
	//SINGLE_PC_ENABLED
		unsigned char vSPC = atoi(str2);
		if (!mib_set(MIB_SPC_ENABLE, (void *)&vSPC)) {
			printf("Set Single PC enable flag error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "SPC_IPTYPE")){
	//SINGLE_PC_IP_TYPE
		unsigned char vSPC = atoi(str2);
		if (!mib_set(MIB_SPC_IPTYPE, (void *)&vSPC)) {
			printf("Set Single PC IP type error!");
			return error;
		}
	}
	
#ifdef IP_ACL
	if(!strcmp(str1, "ACL_CAPABILITY")){
	//ACL_CAPABILITY
		unsigned char vACL = atoi(str2);
		if (!mib_set(MIB_ACL_CAPABILITY, (void *)&vACL)) {
			printf("Set ACL Capability error!");
			return error;
		}
	}
#endif
	
#ifdef URL_BLOCKING_SUPPORT
	if(!strcmp(str1, "URL_CAPABILITY")){
	//URL_CAPABILITY
		unsigned char vURL = atoi(str2);
		if (!mib_set(MIB_URL_CAPABILITY, (void *)&vURL)) {
			printf("Set URL Capability error!");
			return error;
		}
	}
#endif

#ifdef WLAN_SUPPORT
	if(!strcmp(str1, "SSID")){
	//WLAN SSID
		if (!mib_set(MIB_WLAN_SSID, (void *)str2)) {
			printf("Set SSID error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "CHANNEL")){
	//WLAN CHANNEL_NUM
		unsigned char vChNum = atoi(str2);
		if (!mib_set(MIB_WLAN_CHAN_NUM, (void *)&vChNum)) {
			printf("Set channel number error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "WEP")){
	//WLAN WEP
		unsigned char vWEP = atoi(str2);
		if (!mib_set(MIB_WLAN_WEP, (void *)&vWEP)) {
			printf("Set WEP error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "WEP_KEY_TYPE")){
	//WLAN WEP_KEY_TYPE
		unsigned char vWEP = atoi(str2);
		if (!mib_set(MIB_WLAN_WEP_KEY_TYPE, (void *)&vWEP)) {
			printf("Set WEP key type error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "WEP64_KEY1")){
	//WLAN WEP64_KEY1
		char wepKey[15];
		if ( !string_to_hex(str2, wepKey, 10))
			return error;
		if (!mib_set(MIB_WLAN_WEP64_KEY1, (void *)&wepKey)) {
			printf("Set WEP key1 error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "WEP64_KEY2")){
	//WLAN WEP64_KEY2
		char wepKey[15];
		if ( !string_to_hex(str2, wepKey, 10))
			return error;
		if (!mib_set(MIB_WLAN_WEP64_KEY2, (void *)&wepKey)) {
			printf("Set WEP key2 error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "WEP64_KEY3")){
	//WLAN WEP64_KEY3
		char wepKey[15];
		if ( !string_to_hex(str2, wepKey, 10))
			return error;
		if (!mib_set(MIB_WLAN_WEP64_KEY3, (void *)&wepKey)) {
			printf("Set WEP key3 error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "WEP64_KEY4")){
	//WLAN WEP64_KEY4
		char wepKey[15];
		if ( !string_to_hex(str2, wepKey, 10))
			return error;
		if (!mib_set(MIB_WLAN_WEP64_KEY4, (void *)&wepKey)) {
			printf("Set WEP key4 error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "WEP128_KEY1")){
	//WLAN WEP128_KEY1
		char wepKey[30];
		if ( !string_to_hex(str2, wepKey, 26))
			return error;
		if (!mib_set(MIB_WLAN_WEP128_KEY1, (void *)&wepKey)) {
			printf("Set WEP key1 error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "WEP128_KEY2")){
	//WLAN WEP128_KEY2
		char wepKey[30];
		if ( !string_to_hex(str2, wepKey, 26))
			return error;
		if (!mib_set(MIB_WLAN_WEP128_KEY2, (void *)&wepKey)) {
			printf("Set WEP key2 error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "WEP128_KEY3")){
	//WLAN WEP128_KEY3
		char wepKey[30];
		if ( !string_to_hex(str2, wepKey, 26))
			return error;
		if (!mib_set(MIB_WLAN_WEP128_KEY3, (void *)&wepKey)) {
			printf("Set WEP key3 error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "WEP128_KEY4")){
	//WLAN WEP128_KEY4
		char wepKey[30];
		if ( !string_to_hex(str2, wepKey, 26))
			return error;
		if (!mib_set(MIB_WLAN_WEP128_KEY4, (void *)&wepKey)) {
			printf("Set WEP key4 error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "WEP_DEFAULT_KEY")){
	//WLAN WEP_DEFAULT_KEY_ID
		unsigned char vWEP = atoi(str2);
		if (!mib_set(MIB_WLAN_WEP_DEFAULT_KEY, (void *)&vWEP)) {
			printf("Set default tx key id error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "FRAG_THRESHOLD")){
	//WLAN FRAG_THRESHOLD
		unsigned short uFrag = atoi(str2);
		if (!mib_set(MIB_WLAN_FRAG_THRESHOLD, (void *)&uFrag)) {
			printf("Set fragment threshold error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "SUPPORTED_RATES")){
	//WLAN SUPPORTED_RATES
		unsigned short uSRate = atoi(str2);
		if (!mib_set(MIB_WLAN_SUPPORTED_RATE, (void *)&uSRate)) {
			printf("Set tx operation rate error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "BEACON_INTERVAL")){
	//WLAN BEACON_INTERVAL
		unsigned short uBeacon = atoi(str2);
		if (!mib_set(MIB_WLAN_BEACON_INTERVAL, (void *)&uBeacon)) {
			printf("Set beacon interval error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "PREAMBLE_TYPE")){
	//WLAN PREAMBLE_TYPE
		unsigned char vPType = atoi(str2);
		if (!mib_set(MIB_WLAN_PREAMBLE_TYPE, (void *)&vPType)) {
			printf("Set preamble type error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "BASIC_RATES")){
	//WLAN BASIC_RATES
		unsigned short uBRate = atoi(str2);
		if (!mib_set(MIB_WLAN_BASIC_RATE, (void *)&uBRate)) {
			printf("Set tx basic rate error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "RTS_THRESHOLD")){
	//WLAN RTS_THRESHOLD
		unsigned short uRTS = atoi(str2);
		if (!mib_set(MIB_WLAN_RTS_THRESHOLD, (void *)&uRTS)) {
			printf("Set RTS threshold error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "AUTH_TYPE")){
	//WLAN AUTH_TYPE
		unsigned char vAType = atoi(str2);
		if (!mib_set(MIB_WLAN_AUTH_TYPE, (void *)&vAType)) {
			printf("Set authentication type error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "HIDDEN_SSID")){
	//WLAN HIDDEN_SSID
		unsigned char vHSSID = atoi(str2);
		if (!mib_set(MIB_WLAN_HIDDEN_SSID, (void *)&vHSSID)) {
			printf("Set hidden SSID error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "WLAN_DISABLED")){
	//WLAN_DISABLED
		unsigned char vWLAN = atoi(str2);
		if (!mib_set(MIB_WLAN_DISABLED, (void *)&vWLAN)) {
			printf("Set WLAN disabled flag error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "TX_POWER")){
	//WLAN TX_POWER
		unsigned char vTxPower = atoi(str2);
		if (!mib_set(MIB_TX_POWER, (void *)&vTxPower)) {
			printf("Set WLAN tx power error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "INACTIVITY_TIME")){
	//WLAN INACTIVITY_TIME
		unsigned long uETime = atoi(str2);
		if (!mib_set(MIB_WLAN_INACTIVITY_TIME, (void *)&uETime)) {
			printf("Set WLAN expired time error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "RATE_ADAPTIVE_ENABLED")){
	//WLAN RATE_ADAPTIVE_ENABLED
		unsigned char vRate = atoi(str2);
		if (!mib_set(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&vRate)) {
			printf("Set WLAN rate adaptive enable flag error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "DTIM_PERIOD")){
	//WLAN DTIM_PERIOD
		unsigned char vDTIM = atoi(str2);
		if (!mib_set(MIB_WLAN_DTIM_PERIOD, (void *)&vDTIM)) {
			printf("Set WLAN DTIM period error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "WLAN_MODE")){
	//WLAN_MODE
		unsigned char vMode = atoi(str2);
		if (!mib_set(MIB_WLAN_MODE, (void *)&vMode)) {
			printf("Set WLAN mode error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "NETWORK_TYPE")){
	//WLAN NETWORK_TYPE
		unsigned char vNType = atoi(str2);
		if (!mib_set(MIB_WLAN_NETWORK_TYPE, (void *)&vNType)) {
			printf("Set WLAN network type error!");
			return error;
		}
	}
	
#ifdef WLAN_IAPP
	if(!strcmp(str1, "IAPP_DISABLED")){
	//WLAN IAPP_DISABLED
		unsigned char vIAPP = atoi(str2);
		if (!mib_set(MIB_WLAN_IAPP_DISABLED, (void *)&vIAPP)) {
			printf("Set WLAN IAPP disable flag error!");
			return error;
		}
	}
#endif

#ifdef WLAN_WDS
	if(!strcmp(str1, "WLAN_WDS_ENABLED")){
	//WLAN WLAN_WDS_ENABLED
		unsigned char vWDS = atoi(str2);
		if (!mib_set(MIB_WLAN_WDS_ENABLED, (void *)&vWDS)) {
			printf("Set WLAN WDS enable flag error!");
			return error;
		}
	}

	if(!strcmp(str1, "WLAN_WDS_NUM")){
	//WLAN WLAN_WDS_NUM
		unsigned char vWDS = atoi(str2);
		if (!mib_set(MIB_WLAN_WDS_NUM, (void *)&vWDS)) {
			printf("Set WLAN WDS entry number error!");
			return error;
		}
	}

	if(!strcmp(str1, "WDS_WEP")){
	//WLAN WDS_WEP
		unsigned char vWDS = atoi(str2);
		if (!mib_set(MIB_WLAN_WDS_WEP, (void *)&vWDS)) {
			printf("Set WLAN WDS WEP error!");
			return error;
		}
	}

	if(!strcmp(str1, "WDS_WEP_KEY_TYPE")){
	//WLAN WDS_WEP_KEY_TYPE
		unsigned char vWDS = atoi(str2);
		if (!mib_set(MIB_WLAN_WDS_WEP_KEY_TYPE, (void *)&vWDS)) {
			printf("Set WLAN WDS WEP key type error!");
			return error;
		}
	}

	if(!strcmp(str1, "WDS_WEP64_KEY1")){
	//WLAN WDS_WEP64_KEY1
		char wepKey[15];
		if ( !string_to_hex(str2, wepKey, 10))
			return error;
		if (!mib_set(MIB_WLAN_WDS_WEP64_KEY1, (void *)&wepKey)) {
			printf("Set WDS WEP key1 error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "WDS_WEP64_KEY2")){
	//WLAN WDS_WEP64_KEY2
		char wepKey[15];
		if ( !string_to_hex(str2, wepKey, 10))
			return error;
		if (!mib_set(MIB_WLAN_WDS_WEP64_KEY2, (void *)&wepKey)) {
			printf("Set WDS WEP key2 error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "WDS_WEP64_KEY3")){
	//WLAN WDS_WEP64_KEY3
		char wepKey[15];
		if ( !string_to_hex(str2, wepKey, 10))
			return error;
		if (!mib_set(MIB_WLAN_WDS_WEP64_KEY3, (void *)&wepKey)) {
			printf("Set WDS WEP key3 error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "WDS_WEP64_KEY4")){
	//WLAN WDS_WEP64_KEY4
		char wepKey[15];
		if ( !string_to_hex(str2, wepKey, 10))
			return error;
		if (!mib_set(MIB_WLAN_WDS_WEP64_KEY4, (void *)&wepKey)) {
			printf("Set WDS WEP key4 error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "WDS_WEP128_KEY1")){
	//WLAN WDS_WEP128_KEY1
		printf("str2=%s\n", str2);
		char wepKey[30];
		if ( !string_to_hex(str2, wepKey, 26))
			return error;
		if (!mib_set(MIB_WLAN_WDS_WEP128_KEY1, (void *)&wepKey)) {
			printf("Set WDS WEP key1 error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "WDS_WEP128_KEY2")){
	//WLAN WDS_WEP128_KEY2
		char wepKey[30];
		if ( !string_to_hex(str2, wepKey, 26))
			return error;
		if (!mib_set(MIB_WLAN_WDS_WEP128_KEY2, (void *)&wepKey)) {
			printf("Set WDS WEP key2 error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "WDS_WEP128_KEY3")){
	//WLAN WDS_WEP128_KEY3
		char wepKey[30];
		if ( !string_to_hex(str2, wepKey, 26))
			return error;
		if (!mib_set(MIB_WLAN_WDS_WEP128_KEY3, (void *)&wepKey)) {
			printf("Set WDS WEP key3 error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "WDS_WEP128_KEY4")){
	//WLAN WDS_WEP128_KEY4
		char wepKey[30];
		if ( !string_to_hex(str2, wepKey, 26))
			return error;
		if (!mib_set(MIB_WLAN_WDS_WEP128_KEY4, (void *)&wepKey)) {
			printf("Set WDS WEP key4 error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "WDS_WEP_DEFAULT_KEY")){
	//WLAN WDS_WEP_DEFAULT_KEY
		unsigned char vWDS = atoi(str2);
		if (!mib_set(MIB_WLAN_WDS_WEP_DEFAULT_KEY, (void *)&vWDS)) {
			printf("Set WLAN WDS default tx key id error!");
			return error;
		}
	}
#endif

#ifdef WLAN_WPA
	if(!strcmp(str1, "WLAN_ENCRYPT")){
	//WLAN_ENCRYPT
		unsigned char vEncrypt = atoi(str2);
		if (!mib_set(MIB_WLAN_ENCRYPT, (void *)&vEncrypt)) {
			printf("Set WLAN encrypt error!");
			return error;
		}
	}

	if(!strcmp(str1, "WLAN_ENABLE_SUPP_NONWPA")){
	//WLAN_ENABLE_SUPP_NONWPA
		unsigned char vNonWPA = atoi(str2);
		if (!mib_set(MIB_WLAN_ENABLE_SUPP_NONWPA, (void *)&vNonWPA)) {
			printf("Set WLAN nonWPA enable flag error!");
			return error;
		}
	}

	if(!strcmp(str1, "WLAN_SUPP_NONWPA")){
	//WLAN_SUPP_NONWPA
		unsigned char vNonWPA = atoi(str2);
		if (!mib_set(MIB_WLAN_SUPP_NONWPA, (void *)&vNonWPA)) {
			printf("Set WLAN nonWPA error!");
			return error;
		}
	}

	if(!strcmp(str1, "WLAN_WPA_AUTH")){
	//WLAN_WPA_AUTH
		unsigned char vWPA = atoi(str2);
		if (!mib_set(MIB_WLAN_WPA_AUTH, (void *)&vWPA)) {
			printf("Set WLAN WPA authentication error!");
			return error;
		}
	}

	if(!strcmp(str1, "WLAN_WPA_CIPHER_SUITE")){
	//WLAN_WPA_CIPHER_SUITE
		unsigned char vWPA = atoi(str2);
		if (!mib_set(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&vWPA)) {
			printf("Set WLAN WPA cipher suite error!");
			return error;
		}
	}

	if(!strcmp(str1, "WLAN_WPA_PSK")){
	//WLAN_WPA_PSK
		if (!mib_set(MIB_WLAN_WPA_PSK, (void *)str2)) {
			printf("Set WLAN WPA psk error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "WLAN_WPA_GROUP_REKEY_TIME")){
	//WLAN WLAN_WPA_GROUP_REKEY_TIME
		unsigned long uRTime = atoi(str2);
		if (!mib_set(MIB_WLAN_WPA_GROUP_REKEY_TIME, (void *)&uRTime)) {
			printf("Set WLAN WPA group key rekey time error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "MAC_AUTH_ENABLED")){
	//WLAN MAC_AUTH_ENABLED
		unsigned char vAuth = atoi(str2);
		if (!mib_set(MIB_WLAN_ENABLE_MAC_AUTH, (void *)&vAuth)) {
			printf("Set WLAN MAC authentication enable flag error!");
			return error;
		}
	}

#ifdef WLAN_1x
	if(!strcmp(str1, "RS_IP")){
	//WLAN RS_IP
		struct in_addr inRS;
		if(!inet_aton(str2, (void *)&inRS)) {
			return error;
		}
		if (!mib_set(MIB_WLAN_RS_IP, (void *)&inRS)) {
			printf("Set WLAN RADIUS server IP error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "RS_PORT")){
	//WLAN RS_PORT
		unsigned short uPort = atoi(str2);
		if (!mib_set(MIB_WLAN_RS_PORT, (void *)&uPort)) {
			printf("Set WLAN RADIUS server port error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "RS_PASSWORD")){
	//WLAN RS_PASSWORD
		if (!mib_set(MIB_WLAN_RS_PASSWORD, (void *)str2)) {
			printf("Set WLAN RADIUS server password error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "RS_MAXRETRY")){
	//WLAN RS_MAXRETRY
		unsigned char vRetry = atoi(str2);
		if (!mib_set(MIB_WLAN_RS_RETRY, (void *)&vRetry)) {
			printf("Set WLAN RADIUS server max retry error!");
			return error;
		}
	}

	if(!strcmp(str1, "RS_INTERVAL_TIME")){
	//WLAN RS_INTERVAL_TIME
		unsigned short uITime = atoi(str2);
		if (!mib_set(MIB_WLAN_RS_INTERVAL_TIME, (void *)&uITime)) {
			printf("Set WLAN RADIUS server interval time error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "ACCOUNT_RS_ENABLED")){
	//WLAN ACCOUNT_RS_ENABLED
		unsigned char vAccount = atoi(str2);
		if (!mib_set(MIB_WLAN_ACCOUNT_RS_ENABLED, (void *)&vAccount)) {
			printf("Set WLAN account RADIUS server enable flag error!");
			return error;
		}
	}

	if(!strcmp(str1, "ACCOUNT_RS_IP")){
	//WLAN ACCOUNT_RS_IP
		struct in_addr inRS;
		if(!inet_aton(str2, (void *)&inRS)) {
			return error;
		}
		if (!mib_set(MIB_WLAN_ACCOUNT_RS_IP, (void *)&inRS)) {
			printf("Set WLAN account RADIUS server IP error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "ACCOUNT_RS_PORT")){
	//WLAN ACCOUNT_RS_PORT
		unsigned short uPort = atoi(str2);
		if (!mib_set(MIB_WLAN_ACCOUNT_RS_PORT, (void *)&uPort)) {
			printf("Set WLAN account RADIUS server port error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "ACCOUNT_RS_PASSWORD")){
	//WLAN ACCOUNT_RS_PASSWORD
		if (!mib_set(MIB_WLAN_ACCOUNT_RS_PASSWORD, (void *)str2)) {
			printf("Set WLAN account RADIUS server password error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "ACCOUNT_RS_UPDATE_ENABLED")){
	//WLAN ACCOUNT_RS_UPDATE_ENABLED
		unsigned char vAccount = atoi(str2);
		if (!mib_set(MIB_WLAN_ACCOUNT_UPDATE_ENABLED, (void *)&vAccount)) {
			printf("Set WLAN account RADIUS server update enable flag error!");
			return error;
		}
	}

	if(!strcmp(str1, "ACCOUNT_RS_UPDATE_DELAY")){
	//WLAN ACCOUNT_RS_UPDATE_DELAY
		unsigned short uDelay = atoi(str2);
		if (!mib_set(MIB_WLAN_ACCOUNT_UPDATE_DELAY, (void *)&uDelay)) {
			printf("Set WLAN account RADIUS server update delay time error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "ACCOUNT_RS_MAXRETRY")){
	//WLAN ACCOUNT_RS_MAXRETRY
		unsigned char vRetry = atoi(str2);
		if (!mib_set(MIB_WLAN_ACCOUNT_RS_RETRY, (void *)&vRetry)) {
			printf("Set WLAN account RADIUS server max retry error!");
			return error;
		}
	}

	if(!strcmp(str1, "ACCOUNT_RS_INTERVAL_TIME")){
	//WLAN ACCOUNT_RS_INTERVAL_TIME
		unsigned short uITime = atoi(str2);
		if (!mib_set(MIB_WLAN_ACCOUNT_RS_INTERVAL_TIME, (void *)&uITime)) {
			printf("Set WLAN account RADIUS server interval time error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "WLAN_ENABLE_1X")){
	//WLAN_ENABLE_1X
		unsigned char v1x = atoi(str2);
		if (!mib_set(MIB_WLAN_ENABLE_1X, (void *)&v1x)) {
			printf("Set WLAN 1x enable flag error!");
			return error;
		}
	}
#endif

	if(!strcmp(str1, "WLAN_PSK_FORMAT")){
	//WLAN_PSK_FORMAT
		unsigned char vPSK = atoi(str2);
		if (!mib_set(MIB_WLAN_WPA_PSK_FORMAT, (void *)&vPSK)) {
			printf("Set WLAN WPA PSK format error!");
			return error;
		}
	}

	if(!strcmp(str1, "WPA2_CIPHER_SUITE")){
	//WLAN WPA2_CIPHER_SUITE
		unsigned char vWPA2 = atoi(str2);
		if (!mib_set(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&vWPA2)) {
			printf("Set WLAN WPA2 cipher suite error!");
			return error;
		}
	}
#endif

#ifdef WLAN_ACL
	if(!strcmp(str1, "WLAN_MACAC_ENABLED")){
	//WLAN_MACAC_ENABLED
		unsigned char vACL = atoi(str2);
		if (!mib_set(MIB_WLAN_AC_ENABLED, (void *)&vACL)) {
			printf("Set WLAN access control enable flag error!");
			return error;
		}
	}
#endif

	if(!strcmp(str1, "WLAN_BLOCK_RELAY")){
	//WLAN_BLOCK_RELAY
		unsigned char vBRelay = atoi(str2);
		if (!mib_set(MIB_WLAN_BLOCK_RELAY, (void *)&vBRelay)) {
			printf("Set WLAN block relay error!");
			return error;
		}
	}

	if(!strcmp(str1, "WLAN_BLOCK_ETH2WIR")){
	//WLAN_BLOCK_ETH2WIR
		unsigned char vBlock = atoi(str2);
		if (!mib_set(MIB_WLAN_BLOCK_ETH2WIR, (void *)&vBlock)) {
			printf("Set ethernet to wireless blocking error!");
			return error;
		}
	}

	if(!strcmp(str1, "WLAN_ITF_GROUP")){
	//WLAN_ITF_GROUP
		unsigned char vItf = atoi(str2);
		if (!mib_set(MIB_WLAN_ITF_GROUP, (void *)&vItf)) {
			printf("Set WLAN interface group error!");
			return error;
		}
	}

#ifdef WLAN_8185AG
	if(!strcmp(str1, "WLAN_BAND")){
	//WLAN_BAND
		unsigned char vWBand = atoi(str2);
		if (!mib_set(MIB_WLAN_BAND, (void *)&vWBand)) {
			printf("Set WLAN band error!");
			return error;
		}
	}

	if(!strcmp(str1, "WLAN_FIX_RATE")){
	//WLAN_FIX_RATE
#ifdef CONFIG_USB_RTL8192SU_SOFTAP
		unsigned int uFRate = atoi(str2);
#else
		unsigned short uFRate = atoi(str2);
#endif
		if (!mib_set(MIB_WLAN_FIX_RATE, (void *)&uFRate)) {
			printf("Set WLAN fix rate error!");
			return error;
		}
	}
#endif

#ifdef WLAN_WEB_REDIRECT
	if(!strcmp(str1, "WLAN_WEB_REDIR_URL")){
	//WLAN_WEB_REDIR_URL
		if (!mib_set(MIB_WLAN_WEB_REDIR_URL, (void *)str2)) {
			printf("Set WLAN web redirect url error!");
			return error;
		}
	}
#endif
#endif // of WLAN_SUPPORT

	if(!strcmp(str1, "SUSER_NAME")){
	//SUPER_USER_NAME
		if (!mib_set(MIB_SUSER_NAME, (void *)str2)) {
			printf("Set super user name error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "SUSER_PASSWORD")){
	//SUPER_USER_PASSWORD
		if (!mib_set(MIB_SUSER_PASSWORD, (void *)str2)) {
			printf("Set super user password error!");
			return error;
		}
	}
	
	if(!strcmp(str1, "ADSL_TONE")){
	//ADSL_TONE
		char adslTone[30];
		if ( !string_to_hex(str2, adslTone, 28))
			return error;
		if (!mib_set(MIB_ADSL_TONE, (void *)adslTone)) {
			printf("Set ADSL tone error!");
			return error;
		}
	}

	if(!strcmp(str1, "DIRECT_BRIDGE_MODE")){
	//DIRECT_BRIDGE_MODE
		unsigned char vDBridge = atoi(str2);
		if (!mib_set(MIB_DIRECT_BRIDGE_MODE, (void *)&vDBridge)) {
			printf("Set direct bridge mode error!");
			return error;
		}
	}

	if(!strcmp(str1, "ADSL_HIGH_INP")){
	//ADSL_HIGH_INP
		unsigned char vADSL = atoi(str2);
		if (!mib_set(MIB_ADSL_HIGH_INP, (void *)&vADSL)) {
			printf("Set ADSL high inp error!");
			return error;
		}
	}

	if(!strcmp(str1, "HTTP_SERVER_IP")){
	//HTTP_SERVER_IP
		struct in_addr inHttps;
		if(!inet_aton(str2, (void *)&inHttps)) {
			return error;
		}
		if (!mib_set(MIB_HTTP_SERVER_IP, (void *)&inHttps)) {
			printf("Set HTTP server IP error!");
			return error;
		}
	}

//HW_SETTING
	if(!strcmp(str1, "SUPER_NAME")){
	//SUPER_NAME
		if (!mib_set(MIB_SUPER_NAME, (void *)str2)) {
			printf("Set hardware super user name error!");
			return error;
		}
	}

	if(!strcmp(str1, "SUPER_PASSWORD")){
	//SUPER_PASSWORD
		if (!mib_set(MIB_SUPER_PASSWORD, (void *)str2)) {
			printf("Set hardware super user password error!");
			return error;
		}
	}

	if(!strcmp(str1, "BOOT_MODE")){
	//BOOT_MODE
		unsigned char vBoot = atoi(str2);
		if (!mib_set(MIB_BOOT_MODE, (void *)&vBoot)) {
			printf("Set hardware boot mode error!");
			return error;
		}
	}

	if(!strcmp(str1, "ELAN_MAC_ADDR")){
	//ELAN_MAC_ADDR
		char macAddr[15];
		if ( !string_to_hex(str2, macAddr, 12))
			return error;
		if (!mib_set(MIB_ELAN_MAC_ADDR, (void *)&macAddr)) {
			printf("Set hardware Ethernet LAN MAC address error!");
			return error;
		}
	}

	if(!strcmp(str1, "WLAN_MAC_ADDR")){
	//WLAN_MAC_ADDR
		char macAddr[15];
		if ( !string_to_hex(str2, macAddr, 12))
			return error;
		if (!mib_set(MIB_ELAN_MAC_ADDR, (void *)&macAddr)) {
			printf("Set hardware Ethernet LAN MAC address error!");
			return error;
		}
	}

#if WLAN_SUPPORT
	if(!strcmp(str1, "HW_REG_DOMAIN")){
	//HW_REG_DOMAIN
		unsigned char vRegDomain = atoi(str2);
		if (!mib_set(MIB_HW_REG_DOMAIN, (void *)&vRegDomain)) {
			printf("Set hardware reg domain error!");
			return error;
		}
	}

	if(!strcmp(str1, "HW_RF_TYPE")){
	//HW_RF_TYPE
		unsigned char vRFType = atoi(str2);
		if (!mib_set(MIB_HW_RF_TYPE, (void *)&vRFType)) {
			printf("Set hardware RF type error!");
			return error;
		}
	}

#ifdef WLAN_8185AG
	if(!strcmp(str1, "HW_TX_POWER_CCK")){
	//HW_TX_POWER_CCK
		char txPower[30];
		if ( !string_to_hex(str2, txPower, 28))
			return error;
		if (!mib_set(MIB_HW_TX_POWER_CCK, (void *)txPower)) {
			printf("Set hardware TX power CCK error!");
			return error;
		}
	}

	if(!strcmp(str1, "HW_TX_POWER_OFDM")){
	//HW_TX_POWER_OFDM
		char txPower[30];
		if ( !string_to_hex(str2, txPower, 28))
			return error;
		if (!mib_set(MIB_HW_TX_POWER_OFDM, (void *)txPower)) {
			printf("Set hardware TX power OFDM error!");
			return error;
		}
	}
#else
	if(!strcmp(str1, "HW_TX_POWER")){
	//HW_TX_POWER
		char txPower[30];
		if ( !string_to_hex(str2, txPower, 28))
			return error;
		if (!mib_set(MIB_HW_TX_POWER, (void *)txPower)) {
			printf("Set hardware TX power error!");
			return error;
		}
	}
#endif

	if(!strcmp(str1, "HW_ANT_DIVERSITY")){
	//HW_ANT_DIVERSITY
		unsigned char vAnt = atoi(str2);
		if (!mib_set(MIB_HW_ANT_DIVERSITY, (void *)&vAnt)) {
			printf("Set hardware ANT diversity error!");
			return error;
		}
	}

	if(!strcmp(str1, "HW_TX_ANT")){
	//HW_TX_ANT
		unsigned char vTxAnt = atoi(str2);
		if (!mib_set(MIB_HW_TX_ANT, (void *)&vTxAnt)) {
			printf("Set hardware TX ANT error!");
			return error;
		}
	}

	if(!strcmp(str1, "HW_CS_THRESHOLD")){
	//HW_CS_THRESHOLD
		unsigned char vCSThr = atoi(str2);
		if (!mib_set(MIB_HW_CS_THRESHOLD, (void *)&vCSThr)) {
			printf("Set hardware carrier sense threshold error!");
			return error;
		}
	}

	if(!strcmp(str1, "HW_CCA_MODE")){
	//HW_CCA_MODE
		unsigned char vCCAMode = atoi(str2);
		if (!mib_set(MIB_HW_CCA_MODE, (void *)&vCCAMode)) {
			printf("Set hardware CCA mode error!");
			return error;
		}
	}

	if(!strcmp(str1, "HW_PHY_TYPE")){
	//HW_PHY_TYPE
		unsigned char vPHYType = atoi(str2);
		if (!mib_set(MIB_HW_PHY_TYPE, (void *)&vPHYType)) {
			printf("Set hardware PHY type error!");
			return error;
		}
	}

	if(!strcmp(str1, "HW_WLAN_LED_TYPE")){
	//HW_WLAN_LED_TYPE
		unsigned char vLedType = atoi(str2);
		if (!mib_set(MIB_HW_LED_TYPE, (void *)&vLedType)) {
			printf("Set hardware WLAN LED type error!");
			return error;
		}
	}
#endif // of WLAN_SUPPORT

	if(!strcmp(str1, "BYTE")){
	//BYTE_TEST
		unsigned char vByte = atoi(str2);
		if (!mib_set(MIB_BYTE_TEST, (void *)&vByte)) {
			printf("Set hardware byte test error!");
			return error;
		}
	}

	if(!strcmp(str1, "WORD")){
	//WORD_TEST
		unsigned short uWord = atoi(str2);
		if (!mib_set(MIB_WORD_TEST, (void *)&uWord)) {
			printf("Set hardware word test error!");
			return error;
		}
	}

	if(!strcmp(str1, "DWORD")){
	//DWORD_TEST
		unsigned long uDWord = atoi(str2);
		if (!mib_set(MIB_DWORD_TEST, (void *)&uDWord)) {
			printf("Set hardware double word test error!");
			return error;
		}
	}

	if(!strcmp(str1, "INT1")){
	//INT1_TEST
		unsigned int uInt1 = atoi(str2);
		if (!mib_set(MIB_INTERGER_TEST1, (void *)&uInt1)) {
			printf("Set hardware integer test1 error!");
			return error;
		}
	}

	if(!strcmp(str1, "INT2")){
	//INT2_TEST
		unsigned int uInt2 = atoi(str2);
		if (!mib_set(MIB_INTERGER_TEST2, (void *)&uInt2)) {
			printf("Set hardware integer test2 error!");
			return error;
		}
	}

	return 1;
}

#ifdef EMBED
int flashdrv_filewrite(FILE *fp,int size,void  *dstP)
{
	int i;
	int nWritten; /* completed bytes */
	volatile unsigned short *dstAddr;
	unsigned char *block;
	nWritten = 0;
	dstAddr = dstP;
	
	block = malloc(FLASH_BLOCK_SIZE);
	if(!block)
	return 1;
	
	while ( nWritten < size )
	{
		int nRead;

		/* fill buffer with file */
		for( i = 0, nRead = 0; 
		     i < FLASH_BLOCK_SIZE && ( i < (size-nWritten) );
		     i++, nRead++ )
		{
			block[i] = fgetc(fp);
		}
		
		if (flash_write( (void*)block, (void*)dstAddr, nRead ))
			printf("flashWrite --> %08x (len %d)\n", dstAddr, nRead);
		else {
			printf("flash write error!\n");
			free(block);
			return 1;
		}

		dstAddr += nRead >> 1;
		nWritten += nRead;
	}
	
	free(block);
	return 0;
}
#endif

unsigned char fwFilename[256], fwDest[256], fwPath[256];
void upgradeFirmware(void)
{
	int rv;
	FILE *fp=NULL, *fp2=NULL;
	long filelen;
	char buf[10], output[256];

#ifdef CLOSE_OTHER_PROCESS
	//kill others process
    	va_cmd("/bin/killall", 2, 1, "-9", "mpoad");
        va_cmd("/bin/adslctrl", 1, 1, "disablemodemline");
        va_cmd("/bin/wdg", 1, 1, "off");
    	va_cmd("/bin/killall", 2, 1, "-9", "wdg");
    	va_cmd("/bin/spppctl", 2, 1, "down", "*");
#endif

	if ((fp2 = fopen("/var/run/udhcpc.pid", "r"))) {
		fgets(buf, sizeof(buf), fp2);
		strcpy(output, "kill -9 ");
		strcat(output, buf);
		system(output);
		fclose(fp2);
	}

	if (!(fp = fopen(fwFilename, "rb"))) {
		printf("Image file open fail\n");
		return;
	}
	fseek(fp, 0, SEEK_END);
	filelen = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if (filelen <= 0) {
		printf("Image file not exist\n");
		fclose(fp);
		return;
	}
	
	if ((fp2 = fopen("/var/run/spppd.pid", "r"))) {
		fgets(buf, sizeof(buf), fp2);
		strcpy(output, "kill -9 ");
		strcat(output, buf);
		system(output);
		fclose(fp2);
	}
	//close all devices
	sleep(1);	
	sync();
	va_cmd(IFCONFIG, 2, 1, (char *)ELANIF, "down");
	va_cmd(IFCONFIG, 2, 1, (char *)WLANIF, "down");
	va_cmd("/bin/sarctl", 1, 1, "disable");
	
	rv = flashdrv_filewrite(fp, filelen, CODE_IMAGE_OFFSET);
	if (rv) {
		printf("flash error!\n");
		fclose(fp);
		return;
	}
	fclose(fp);

	printf("The system is restarting ...\n");
	usleep(5000);
	//restart();
	reboot(RB_AUTOBOOT);
}

unsigned char serverIP[20];
unsigned int fwUpgrade = 0;
int update_firmware(char *line){
	int len;
	char *str1, *str2, *tmp;

	//parse the line	
	tmp = strtok(line, "\"");
	str1 = strtok(NULL, "\"");
	tmp = strtok(NULL, "\n");
	str2 = strchr(tmp, '\"') + 1;
	len = strcspn(str2, "\"");
	str2[len] = '\0';

	if(!strcmp(str1, "firmware-version")){
	//firmware-version
		int v1, v2, v3, v4, v5, v6;
		sscanf(str2, "%d.%d.%d", &v4, &v5, &v6);
		getSYS2Str(SYS_FWVERSION, tmp);
		sscanf(tmp, "%d.%d.%d", &v1, &v2, &v3);
//		if (strcmp(tmp, str2) < 0)
		if (v1<v4 || (v1<=v4 && v2<v5) || (v1<=v4 && v2<=v5 && v3<v6))
			fwUpgrade = 1;
	}
	
	if(!strcmp(str1, "firmware-path")){
	//firmware-path
		strcpy(fwPath, str2);
		fwPath[strlen(str2)] = '\0';
	}
	
	if(!strcmp(str1, "firmware-filename")){
	//firmware-filename
		strcpy(fwFilename, "/tmp/");
		strcat(fwFilename, str2);
		strcpy(fwDest, serverIP);
		strcat(fwDest, "/");
		strcat(fwDest, fwPath);
		strcat(fwDest, str2);
	}
	
	return 1;
}

void update_config(void)
{
	//close all devices
	sleep(1);	
	sync();
	va_cmd(IFCONFIG, 2, 1, (char *)ELANIF, "down");
	va_cmd(IFCONFIG, 2, 1, (char *)WLANIF, "down");
	va_cmd("/bin/sarctl",1,1,"disable");
	printf("Updating user specific configuration and restart the system......\n\n");
	//update flash
	if (mib_update(CURRENT_SETTING) == 0) {
		printf("Warning : Commit cs fail !\n");				
	}
	if (mib_update(HW_SETTING) == 0) {
		printf("Warning : Commit hs fail !\n");				
	}
	//reboot the system 
	va_cmd("/bin/reboot", 0, 1);
}

int main(int argc, char **argv)
{
	FILE *fpin, *fp;
	char LINE[256], i, buf[10];
	char firmwareUpdateRequest[] = "/tmp/FirmwareUpdateRequest.xml";
	char plain_file[] = "/tmp/plain_file.xml";
	char userConfigFileName[] = "_tdp_TECOM_Router.xml";
	char url[256], output[256], rc4_key[256];
	int len, key_len;
	long filelen;
	
	for(i=0;i<chain_record_number;i++){
		chain_updated[i]=0;
	}
	
	strcpy(serverIP, argv[1]);
	len = strlen(serverIP);
	serverIP[len] = '\0';

	///////////////////////// Set RC4 key temporarily
	strcpy(rc4_key, getMibInfo(MIB_ELAN_MAC_ADDR));
	len = strlen(rc4_key);
	rc4_key[len] = '\0';
	
	if (!(fpin=fopen(firmwareUpdateRequest, "r")))
		printf("FirmwareUpdateRequest file not exist\n");
	else {
		fseek(fpin, 0, SEEK_END);
		filelen = ftell(fpin);
		fseek(fpin, 0, SEEK_SET);
		if (filelen > 0) {
			fp=fopen(plain_file, "w");
			while(!feof(fpin)){
				len = fread(LINE, sizeof(char), 255, fpin);
				LINE[len] = 0;
				//decryption
				key_len = strlen(rc4_key);
				rc4_encrypt(rc4_key, key_len, LINE, len, output);
				output[len] = 0;
				fprintf(fp, "%s", output);
			}
			fclose(fpin);
			fclose(fp);
			fp=fopen(plain_file, "r");
			fgets(LINE, sizeof(LINE), fp);
			while(!feof(fp)) {
				LINE[0] = 0;
				fgets(LINE, sizeof(LINE), fp);//get one line from the file

				if((strlen(LINE)==0) || (LINE[0]==13 && LINE[1]==10)) //empty line or CR+LF
					break;
			
				if (LINE[1] == '/')	// ignore last line
					break;
					
				if(LINE[0]!='#') {//this line is not comment	
					if(update_firmware(LINE) < 0) {
						close(fp);
						return error;
					}
				}
			}
		}
		else
			printf("FirmwareUpdateRequest file not exist\n");
		close(fp);
	}

#ifdef EMBED
	if (fwUpgrade == 1) {
		printf("Get new firmware from http server %s\n\n", serverIP);
		fp=fopen("/var/run/boa.pid", "r");
		fgets(buf, sizeof(buf), fp);
		fclose(fp);
		strcpy(output, "kill -9 ");
		strcat(output, buf);
		system(output);
		if ((fp=fopen("/var/run/snmpd.pid", "r"))) {
			fgets(buf, sizeof(buf), fp);
			strcpy(output, "kill -9 ");
			strcat(output, buf);
			system(output);
			fclose(fp);
		}
		if ((fp=fopen("/var/run/dnsmasq.pid", "r"))) {
			fgets(buf, sizeof(buf), fp);
			strcpy(output, "kill -9 ");
			strcat(output, buf);
			system(output);
			fclose(fp);
		}
#ifdef CLOSE_OTHER_PROCESS
        //kill others process
        va_cmd("/bin/killall", 2, 1, "-9", "dnsmasq");
        va_cmd("/bin/killall", 2, 1, "-9", "inetd");
        va_cmd("/bin/killall", 2, 1, "-9", "mpoad");
        va_cmd("/bin/killall", 2, 1, "-9", "vsntp");
        va_cmd("/bin/spppctl", 2, 1, "down", "*");
        va_cmd("/bin/adslctrl", 1, 1, "disablemodemline");
#endif
		va_cmd("/bin/wget", 5, 1, "--http-user=admin", "--http-passwd=system", fwDest, "-O", fwFilename);
		upgradeFirmware();
	}
	else
		printf("The firmware is up to date.\n\n");
#endif
	
	strcpy(url, serverIP);
	strcat(url, "/");
	strcat(url, getMibInfo(MIB_ELAN_MAC_ADDR));
	strcat(url, userConfigFileName);
	len = strlen(url);
	url[len] = '\0';
	strcpy(output, "/tmp/");
	strcat(output, getMibInfo(MIB_ELAN_MAC_ADDR));
	strcat(output, userConfigFileName);
	len = strlen(output);
	output[len] = '\0';
	
	printf("Get user specific configuration file from http server %s......\n\n", serverIP);
	va_cmd("/bin/wget", 5, 1, "--http-user=admin", "--http-passwd=system", url, "-O", output);
	printf("Checking user specific configuration file now......\n\n");
	if (!(fpin = fopen(output, "r")))
		printf("User configuration file not exist\n");
	else {
		fseek(fpin, 0, SEEK_END);
		filelen = ftell(fpin);
		fseek(fpin, 0, SEEK_SET);
		if (filelen > 0) {
			char str1[10], *str2, *tmp;
			int v1, v2, v3, v4, v5, v6;
			fp = fopen(plain_file, "w");
			while(!feof(fpin)){
				len = fread(LINE, sizeof(char), 255, fpin);
				LINE[len] = 0;
				//decryption
				key_len = strlen(rc4_key);
				rc4_encrypt(rc4_key, key_len, LINE, len, output);
				output[len] = 0;
				fprintf(fp, "%s", output);
			}
			fclose(fpin);
			fclose(fp);
			fp = fopen(plain_file, "r");
			fgets(LINE, sizeof(LINE), fp);
			strcpy(str1, getMibInfo(MIB_CONFIG_VERSION));
			sscanf(str1, "%d.%d.%d", &v1, &v2, &v3);
			tmp = strtok(LINE, "\"");
			str2 = strtok(NULL, "\"");
			sscanf(str2, "%d.%d.%d", &v4, &v5, &v6);
			if (v1<v4 || (v1<=v4 && v2<v5) || (v1<=v4 && v2<=v5 && v3<v6)) {
				mib_set(MIB_CONFIG_VERSION, (void *)str2);
				while(!feof(fp)) {
					LINE[0] = 0;
					fgets(LINE, sizeof(LINE), fp);//get one line from the file

					if((LINE[0]!='#') && (strlen(LINE)!=0) && !(LINE[0]==13 && LINE[1]==10)) {//this line is not comment
						if(update_setting(LINE, fp) < 0) {
							fclose(fp);
							return error;
						}
					}
				}
				fclose(fp);
				update_config();
			}
			else
				printf("The user specific configuration is up to date.\n\n");
		}
		else
			printf("User configuration file not exist\n");
		fclose(fp);
	}
	
	return 0;
}
#endif
