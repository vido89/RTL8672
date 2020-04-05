
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
//#define MAX_POE_PER_VC		2

#define CHAIN_ENTRY_ERROR	-1
#define CHAIN_ENTRY_EMPTY	0
#define CHAIN_ENTRY_END		1
#define CHAIN_ENTRY_OK		2


enum {
	INPUT_TYPE_UINT,
	INPUT_TYPE_STRING,
	INPUT_TYPE_IPADDR,
	INPUT_TYPE_IPMASK,
	INPUT_TYPE_ETHADDR
};
char chain_updated[chain_record_number];
char xmline[256];

// Apply Star Zhang's fast load
mib_table_entry_T *infotmp;
int infototal;

unsigned char IFsel[8];

static char *getMibInfo(int id) {
	static char strbuf[256];

	if (getMIB2Str(id, strbuf) == 0)
		return strbuf;

	return NULL;
}

//#ifdef _CWMP_MIB_
unsigned int getWanifIndex( int index  )
{
	int total=0, i;
	unsigned int ifindex=0xff;

	if(index<=0) return ifindex;

	total = showInf();
	if( total<=0 || total<index ) return ifindex;

	return IFsel[index-1];
}
//#endif

// Kaohj -- get a line of chain record, and parse to obtain its name and value strings
/*
 * <Value Name="abc" Value="123"/>
 * Return:
 * -1	:	parse error
 *  0	:	end of this chain
 *  1	:	successful
 */
static int parseChainEntryLine(FILE *fp, char **str1, char **str2)
{
	char *tmp;
	int len;

	while (1) {
		if (!fgets(xmline,sizeof(xmline),fp))
			return CHAIN_ENTRY_ERROR;
		if (xmline[0] != '\n' && xmline[0] != '\r')
			break;
	}

	if(xmline[1]=='/')
		return CHAIN_ENTRY_END;

	tmp = strtok(xmline, "\"");
	// get name string
	*str1 = strtok(NULL, "\"");
	tmp = strtok(NULL, "\n");
	// get value string
	*str2 = strchr(tmp, '\"') + 1;
	tmp = *str2;
	len = strcspn(tmp, "\"");
	tmp[len] = '\0';
	return CHAIN_ENTRY_OK;
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

	if (!inet_aton(sip, (struct in_addr *)&filterEntry.srcIp))
	{
		printf("Invalid Src IP address!\n");
		goto setErr_filter;
	}

	if(!inet_aton(smask, (struct in_addr *)&mask))
	{
		printf("Invalid Src Netmask!\n");
		goto setErr_filter;
	}
	else
	{
		mbit=0;

		while (1)
		{
			if (mask&0x80000000)
			{
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
		filterEntry.srcPortFrom = (unsigned short)sfromPort;

		if ( stoPort == 0 )
			filterEntry.srcPortTo = filterEntry.srcPortFrom;
		else
			filterEntry.srcPortTo = (unsigned short)stoPort;

		if ( filterEntry.srcPortFrom  > filterEntry.srcPortTo )
		{
			printf("Error! Invalid src port range.\n");
			goto setErr_filter;
		}
	}

	// Dst Host
	if (filterEntry.protoType != PROTO_TCP && filterEntry.protoType != PROTO_UDP)
		dfromPort = 0;

	if (!inet_aton(dip, (struct in_addr *)&filterEntry.dstIp))
	{
		printf("Invalid Dst IP address!\n");
		goto setErr_filter;
	}

	if(!inet_aton(dmask, (struct in_addr *)&mask))
	{
		printf("Invalid Dst Netmask!\n");
		goto setErr_filter;
	}
	else
	{
		mbit=0;

		while (1)
		{
			if (mask&0x80000000)
			{
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
		filterEntry.dstPortFrom = (unsigned short)dfromPort;

		if ( dtoPort == 0 )
			filterEntry.dstPortTo = filterEntry.dstPortFrom;
		else
			filterEntry.dstPortTo = (unsigned short)dtoPort;

		if ( filterEntry.dstPortFrom  > filterEntry.dstPortTo )
		{
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
	else if (intVal == -1)
		printf(strTableFull);
	return 1;


setErr_filter:
	printf("set FW error!\n");
	return error;
}
/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
int dhcpOptionAdd(int type,unsigned char ifIndex,unsigned char enable,unsigned short order,unsigned short tag,unsigned char len,unsigned char *value,unsigned char usedFor,
				unsigned char dhcpOptInstNum,unsigned char dhcpConSPInstNum)
{
	MIB_CE_DHCP_OPTION_T dhcpOptionEntry;
	int intVal;
	memset(&dhcpOptionEntry, 0x00, sizeof(dhcpOptionEntry));

	dhcpOptionEntry.ifIndex = ifIndex;
	dhcpOptionEntry.enable = enable;
	dhcpOptionEntry.order = order;
	dhcpOptionEntry.tag = tag;
	dhcpOptionEntry.len = len;
	memcpy(dhcpOptionEntry.value, value, len);
	dhcpOptionEntry.value[DHCP_OPT_VAL_LEN-1] = 0;
	dhcpOptionEntry.usedFor = usedFor;
	dhcpOptionEntry.dhcpOptInstNum = dhcpOptInstNum;
	dhcpOptionEntry.dhcpConSPInstNum = dhcpConSPInstNum;

	if (type)
		intVal = mib_chain_add(MIB_DHCP_SERVER_OPTION_TBL, (unsigned char*)&dhcpOptionEntry);
	else
		intVal = mib_chain_add(MIB_DHCP_CLIENT_OPTION_TBL, (unsigned char*)&dhcpOptionEntry);
	if (intVal == 0) {
		printf(strAddChainerror);
		goto setErr_DHCPOpt;
	}
	else if (intVal == -1)
		printf(strTableFull);
	return 1;
setErr_DHCPOpt:
	return error;
}

int addDhcpPool(int enable, int poolorder, unsigned char *poolname, int sourceinterface, unsigned char *vendorclass, 
	int vendorclassflag, unsigned char *clientid, int clientidflag, unsigned char *userclass, 
	int userclassflag, unsigned char *chaddr, unsigned char *chaddrmask, int chaddrflag, 
	int localserved, unsigned char *startaddr, unsigned char *endaddr, unsigned char *subnetmask, 
	unsigned char *iprouter, unsigned char *dnsserver1, unsigned char *dnsserver2, unsigned char *dnsserver3, 
	unsigned char *domainname, int leasetime, unsigned char *dhcprelayip, int dnsservermode
#ifdef _CWMP_MIB_
	, int instanceNum
#endif
	)
{
	DHCPS_SERVING_POOL_T entry;
	int intVal;
	memset(&entry, 0x00, sizeof(DHCPS_SERVING_POOL_T));

	entry.enable = (unsigned char)enable;
	entry.poolorder = poolorder;
	memcpy(entry.poolname, poolname, MAX_NAME_LEN);
	entry.sourceinterface = (unsigned char)sourceinterface;
	memcpy(entry.vendorclass, vendorclass, OPTION_LEN);
	entry.vendorclassflag = vendorclassflag;
	memcpy(entry.clientid, clientid, OPTION_LEN);
	entry.clientidflag = (unsigned char)clientidflag;
	memcpy(entry.userclass, userclass, OPTION_LEN);
	entry.userclassflag = (unsigned char)userclassflag;
	memcpy(entry.chaddr, chaddr, MAC_ADDR_LEN);
	memcpy(entry.chaddrmask, chaddrmask, MAC_ADDR_LEN);
	entry.chaddrflag = (unsigned char)chaddrflag;
	entry.localserved = (unsigned char)localserved;
	memcpy(entry.startaddr, startaddr, IP_ADDR_LEN);
	memcpy(entry.endaddr, endaddr, IP_ADDR_LEN);
	memcpy(entry.subnetmask, subnetmask, IP_ADDR_LEN);
	memcpy(entry.iprouter, iprouter, IP_ADDR_LEN);
	memcpy(entry.dnsserver1, dnsserver1, IP_ADDR_LEN);
	memcpy(entry.dnsserver2, dnsserver2, IP_ADDR_LEN);
	memcpy(entry.dnsserver3, dnsserver3, IP_ADDR_LEN);
	memcpy(entry.domainname, domainname, GENERAL_LEN);
	entry.leasetime = leasetime;
	memcpy(entry.dhcprelayip, dhcprelayip, IP_ADDR_LEN);
	entry.dnsservermode = (unsigned char)dnsservermode;
#ifdef _CWMP_MIB_
	entry.InstanceNum = instanceNum;
#endif
	
	intVal = mib_chain_add(MIB_DHCPS_SERVING_POOL_TBL, (void *)&entry);
	if (intVal == 0) {
		printf(strAddChainerror);
		goto setErr_DHCPOpt;
	}
	else if (intVal == -1)
		printf(strTableFull);
	return 1;
setErr_DHCPOpt:
	return error;
}
#endif
/*ping_zhang:20080919 END*/


#ifdef MAC_FILTER
int MACFilterAdd(unsigned char act,unsigned char *src_macaddr,unsigned char *dst_macaddr,unsigned char dir){
	MIB_CE_MAC_FILTER_T macEntry;
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
	else if (intVal == -1)
		printf(strTableFull);
	return 1;

setErr_filter:
	return error;

}
#endif // of MAC_FILTER

int addPortFW(unsigned char *ip, unsigned short fromPort,unsigned short toPort,
		unsigned char protocol
//#ifdef _CWMP_MIB_
//		,int OutInf, int enable, unsigned int leaseduration, char *remotehost, unsigned int externalport, unsigned int InstanceNum
#ifdef XML_TR069
		,int OutInf, int enable, unsigned int leaseduration, char *remotehost, unsigned int externalfromport, unsigned int externaltoport, unsigned int InstanceNum, char * comment
#else
		,int OutInf, int enable, char *remotehost, unsigned int externalfromport, unsigned int externaltoport, char * comment
#endif
//#endif
		){

		char_t *submitUrl, *strAddPort, *strDelPort, *strVal, *strDelAllPort;
		char_t *strIp, *strFrom, *strTo;
		int intVal;
		unsigned int entryNum, i;
		MIB_CE_PORT_FW_T entry;
		struct in_addr curIpAddr, curSubnet;
		unsigned long v1, v2, v3;
		unsigned char vChar;

		memset(&entry, 0, sizeof(entry));

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
/*		if ( intVal<1 || intVal>65535) {
			printf("Error! Invalid value of from-port.\n");
			goto setErr_portfw;
		}
		*/
		entry.fromPort = (unsigned short)intVal;

		intVal=toPort;
/*		if (  intVal<1 || intVal>65535) {
			printf("Error! Invalid value of to-port.\n");
			goto setErr_portfw;
		}
		*/
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

		strcpy(entry.comment, comment);
#if 1
//#ifdef _CWMP_MIB_
		// review conf
		//entry.ifIndex = getWanifIndex( OutInf );
		entry.ifIndex = OutInf;
		entry.enable = (enable==0)?0:1;
#ifdef XML_TR069
		entry.leaseduration = leaseduration;
#endif
		inet_aton(remotehost, (struct in_addr *)&entry.remotehost);
//		entry.externalport = externalport;
		entry.externalfromport = externalfromport;
		entry.externaltoport = externaltoport;
		if ( entry.externalfromport  > entry.externaltoport ) {
			printf("Error! Invalid external port range.\n");
			goto setErr_portfw;
		}

#ifdef XML_TR069
		entry.InstanceNum = InstanceNum;
#endif
#else
		entryNum = mib_chain_total(MIB_PORT_FW_TBL);

		// Check if there is any port overlapped
		for (i=0; i<entryNum; i++) {
			//MIB_CE_PORT_FW_Tp pCheckEntry;
			MIB_CE_PORT_FW_T CheckEntry;

			if (!mib_chain_get(MIB_PORT_FW_TBL, i, (void *)&CheckEntry)) {
				printf("Get table entry error!");
				goto setErr_portfw;
			}

			if(!(((entry.fromPort < CheckEntry.fromPort) && (entry.toPort < CheckEntry.fromPort)) ||
				((entry.fromPort > CheckEntry.toPort) && (entry.toPort > CheckEntry.toPort))) &&
			       (entry.protoType & CheckEntry.protoType) ) {
				printf("Setting port range has overlapped with used port numbers!\n");
				goto setErr_portfw;
			}
		}
#endif

		intVal = mib_chain_add(MIB_PORT_FW_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			printf(strAddChainerror);
			goto setErr_portfw;
		}
		else if (intVal == -1)
			printf(strTableFull);
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

#ifdef DEFAULT_GATEWAY_V1
	int defaultGW;
#endif
#ifdef CONFIG_SPPPD_STATICIP
	int staticPPPoE=0;
#endif
	int vpi,vci,encap,channelMode,napt,channelStatus,pppConnectType,channelAddrType,pppIdleTime;
	int br_type;
	int qos, pcr, cdvt, scr, mbs, pppAuth, mtu, ipunnum;
#ifdef CONFIG_GUI_WEB
	char str[20],pppUser[50],pppPasswd[50],LocalIPAddr[16],RemoteIPAddr[16],SubnetMask[16],pppAc[30];
#else
	char str[20],pppUser[MAX_PPP_NAME_LEN+1],pppPasswd[MAX_NAME_LEN],LocalIPAddr[16],RemoteIPAddr[16],SubnetMask[16],pppAc[30];
#endif
	pppAc[0] = '\0';
#ifdef CONFIG_EXT_SWITCH
	int vlan,vid,vprio,vpass,itfGroup;
#endif
#ifdef _PRMT_TR143_
	int TR143UDPEchoItf=0;
#endif //_PRMT_TR143_
#ifdef XML_TR069
#ifdef _CWMP_MIB_
	int connDisable=0,ConDevInstNum=0,ConIPInstNum=0,ConPPPInstNum=0, autoDisTime=0, warnDisDelay=0;
#ifdef _PRMT_X_CT_COM_PPPOEv2_
	int PPPoEProxyEnable=0, PPPoEProxyMaxUser=0;
#endif //_PRMT_X_CT_COM_PPPOEv2_
#ifdef _PRMT_X_CT_COM_WANEXT_
	int ServiceList=0;
#endif //_PRMT_X_CT_COM_WANEXT_
	char pppService[30],WanName[30];
	pppService[0] = WanName[0] = '\0';
#endif //_CWMP_MIB_
#endif // #ifdef XML_TR069


	entry->qos = 0;
	entry->pcr = ATM_MAX_US_PCR;

	//sscanf(line,"%d	%d	%d	%d	%d	%d	%d	%s	%s	%s	%s	%s	%d	%d",
	//	&vpi,&vci,&encap,&channelMode,&napt,&channelStatus,&channelAddrType,LocalIPAddr,
	//	RemoteIPAddr,SubnetMask,pppUser,pppPasswd,pppConnectType,pppIdleTime);

	sscanf(line,"%s	%d	%d	%d	%d	%d	%d	%d	%d	%d	%d	%d	%d"
#ifdef CONFIG_EXT_SWITCH
		" %d %d %d %d %d"
#endif
#ifdef _PRMT_TR143_
		" %d"
#endif //_PRMT_TR143_
#ifdef XML_TR069
#ifdef _CWMP_MIB_
		"  %d %d %d %d %s"
#ifdef _PRMT_X_CT_COM_WANEXT_
		" %d"
#endif //_PRMT_X_CT_COM_WANEXT_
#endif
#endif // #ifdef XML_TR069

		,str,&vpi,&vci,&encap,&channelMode,&br_type,&napt,&channelStatus,&qos,&pcr,&cdvt,&scr,&mbs
#ifdef CONFIG_EXT_SWITCH
		,&vlan,&vid,&vprio,&vpass,&itfGroup
#endif
#ifdef _PRMT_TR143_
		,&TR143UDPEchoItf
#endif //_PRMT_TR143_
#ifdef XML_TR069
#ifdef _CWMP_MIB_
		,&connDisable,&ConDevInstNum,&ConIPInstNum,&ConPPPInstNum,WanName
#ifdef _PRMT_X_CT_COM_WANEXT_
		,&ServiceList
#endif //_PRMT_X_CT_COM_WANEXT_
#endif
#endif // #ifdef XML_TR069
		);

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
		entry->napt = napt;
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

#ifdef CONFIG_EXT_SWITCH
	entry->vlan=vlan;
	entry->vid=vid;
	entry->vprio=vprio;
	entry->vpass=vpass;
	entry->itfGroup=itfGroup;
#endif

#ifdef _PRMT_TR143_
	entry->TR143UDPEchoItf=TR143UDPEchoItf?1:0;
#endif //_PRMT_TR143_
#ifdef XML_TR069
#ifdef _CWMP_MIB_
	entry->connDisable=connDisable;
	entry->ConDevInstNum=ConDevInstNum;
	entry->ConIPInstNum=ConIPInstNum;
	entry->ConPPPInstNum=ConPPPInstNum;
	if( strcmp(WanName, STR_NULL) )
		strcpy( entry->WanName, WanName );
#ifdef _PRMT_X_CT_COM_WANEXT_
	entry->ServiceList=(unsigned short)ServiceList;
#endif //_PRMT_X_CT_COM_WANEXT_
#endif
#endif // #ifdef XML_TR069

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
#ifdef DEFAULT_GATEWAY_V1
		entry->dgw=0;
#endif
//		entry->cdvt=0;

	} else // PPP connection
	if (entry->cmode == ADSL_PPPoE || entry->cmode == ADSL_PPPoA)
	{
		//printf("PPP Settings\n");
		sscanf(line,"%s	%d	%d	%d	%d	%d	%d	%d	%d	%d	%d	%d	%d"
#ifdef CONFIG_EXT_SWITCH
		" %d %d %d %d %d"
#endif
#ifdef _PRMT_TR143_
		" %d"
#endif //_PRMT_TR143_
#ifdef XML_TR069
#ifdef _CWMP_MIB_
			"  %d %d %d %d %s"
#ifdef _PRMT_X_CT_COM_WANEXT_
			" %d"
#endif //_PRMT_X_CT_COM_WANEXT_
			"  %d %d %s"
#ifdef _PRMT_X_CT_COM_PPPOEv2_
			" %d %d"
#endif //_PRMT_X_CT_COM_PPPOEv2_
#endif //_CWMP_MIB_
#endif // #ifdef XML_TR069

#ifdef DEFAULT_GATEWAY_V1
			" %d	%d	%d	%d	%d	%s	%s	%s"
#else
			" %d	%d	%d	%d	%s	%s	%s"
#endif
#ifdef  ZTE_GENERAL_ROUTER_SC
                      " %d"
#endif
#ifdef CONFIG_SPPPD_STATICIP
					" %d %s"
#endif
                      ,str,&vpi,&vci,&encap,&channelMode,&br_type,&napt,&channelStatus,&qos,&pcr,&cdvt,&scr,&mbs,
#ifdef CONFIG_EXT_SWITCH
			&vlan,&vid,&vprio,&vpass,&itfGroup,
#endif

#ifdef _PRMT_TR143_
			&TR143UDPEchoItf,
#endif //_PRMT_TR143_
#ifdef XML_TR069
#ifdef _CWMP_MIB_
			&connDisable,&ConDevInstNum,&ConIPInstNum,&ConPPPInstNum, WanName,
#ifdef _PRMT_X_CT_COM_WANEXT_
			&ServiceList,
#endif //_PRMT_X_CT_COM_WANEXT_
			&autoDisTime, &warnDisDelay, pppService,
#ifdef _PRMT_X_CT_COM_PPPOEv2_
			&PPPoEProxyEnable, &PPPoEProxyMaxUser,
#endif //_PRMT_X_CT_COM_PPPOEv2_
#endif //_CWMP_MIB_
#endif // #ifdef XML_TR069

#ifdef DEFAULT_GATEWAY_V1
			&pppConnectType,&pppIdleTime,&defaultGW,&pppAuth,&mtu,pppUser,pppPasswd,pppAc
#else
			&pppConnectType,&pppIdleTime,&pppAuth,&mtu,pppUser,pppPasswd,pppAc

#endif
#ifdef  ZTE_GENERAL_ROUTER_SC
                     ,  &ipunnum
#endif
#ifdef CONFIG_SPPPD_STATICIP
			, &staticPPPoE, LocalIPAddr
#endif
		);

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

		if (entry->cmode == ADSL_PPPoE && strcmp(pppAc, STR_NULL))

			strcpy(entry->pppACName, pppAc);
#ifdef XML_TR069
#ifdef _CWMP_MIB_
		entry->autoDisTime = autoDisTime;
		entry->warnDisDelay = warnDisDelay;
		if (entry->cmode == ADSL_PPPoE && strcmp(pppService, STR_NULL))
			strcpy(entry->pppServiceName, pppService);

#ifdef _PRMT_X_CT_COM_PPPOEv2_
		entry->PPPoEProxyEnable = PPPoEProxyEnable;
		entry->PPPoEProxyMaxUser = PPPoEProxyMaxUser;
#endif //_PRMT_X_CT_COM_PPPOEv2_
#endif
#endif // #ifdef XML_TR069

		// 0:AUTO, 1:PAP, 2:CHAP
		if (pppAuth < 0 || pppAuth > 2){
			printf("PPP Authentication error\n");
			return error;
		}
		entry->pppAuth = pppAuth;
		if (mtu > 0)
			entry->mtu = mtu;
#ifdef ZTE_GENERAL_ROUTER_SC
		entry->ipunnumbered = ipunnum;
#endif
#ifdef CONFIG_SPPPD_STATICIP
		entry->pppIp = staticPPPoE;
		if (entry->pppIp) {
			if(!inet_aton(LocalIPAddr, (void *)&entry->ipAddr)) {
				printf("Static PPPoE IP address error\n");
				return error;
			}
		}
#endif
	} else // WAN
	{
		//printf("WAN IP Settings :\n");
		sscanf(line,"%s	%d	%d	%d	%d	%d	%d	%d	%d	%d	%d	%d	%d"
#ifdef CONFIG_EXT_SWITCH
		" %d %d %d %d %d"
#endif
#ifdef _PRMT_TR143_
		" %d"
#endif //_PRMT_TR143_
#ifdef XML_TR069
#ifdef _CWMP_MIB_
		"  %d %d %d %d %s"
#ifdef _PRMT_X_CT_COM_WANEXT_
		" %d"
#endif //_PRMT_X_CT_COM_WANEXT_
#endif
#endif // #ifdef XML_TR069

#ifdef DEFAULT_GATEWAY_V1
		" %d	%s	%s	%s	%d	%d",
#else
		" %d	%s	%s	%s	%d",
#endif
		str,&vpi,&vci,&encap,&channelMode,&br_type,&napt,&channelStatus,&qos,&pcr,&cdvt,&scr,&mbs,
#ifdef CONFIG_EXT_SWITCH
		&vlan,&vid,&vprio,&vpass,&itfGroup,
#endif
#ifdef _PRMT_TR143_
		&TR143UDPEchoItf,
#endif //_PRMT_TR143_
#ifdef XML_TR069
#ifdef _CWMP_MIB_
		&connDisable,&ConDevInstNum,&ConIPInstNum,&ConPPPInstNum, WanName,
#ifdef _PRMT_X_CT_COM_WANEXT_
		&ServiceList,
#endif //_PRMT_X_CT_COM_WANEXT_
#endif
#endif // #ifdef XML_TR069

#ifdef DEFAULT_GATEWAY_V1
		&channelAddrType,LocalIPAddr,RemoteIPAddr,SubnetMask,&defaultGW,&ipunnum);
#else
		&channelAddrType,LocalIPAddr,RemoteIPAddr,SubnetMask,&ipunnum);
#endif
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
	entry->brmode = br_type;


#ifdef DEFAULT_GATEWAY_V1
	if (entry->cmode != ADSL_BR1483)
	{
		//max=2;
		if (defaultGW<0 || defaultGW>1){
			printf("defaultGW error\n");
			return error;
		}

		entry->dgw = defaultGW;
	}
#endif

	return 1;
}

int addChannelConfiguration(char *line, unsigned char ifIndex)
{
	MIB_CE_ATM_VC_T entry;


	// Added by Mason Yu for AC Name is not NULL(WAN can not get IP Address on PPPoE Mode)
	memset(&entry, 0x00, sizeof(entry));

	if (getVcInput(line,&entry, 0) == error){
		printf("getVcInput error\n");
		return error;
	}

	// Mason Yu
	entry.ifIndex = ifIndex;

	// now do the actual set.
	do {
		unsigned int ifMap;	// high half for PPP bitmap, low half for vc bitmap
#ifdef DEFAULT_GATEWAY_V1
		int drouted=-1;
#endif
		int i, cnt, intVal;
		unsigned int totalEntry;
		MIB_CE_ATM_VC_T Entry;
		MIB_CE_ATM_VC_Tp pmyEntry;

// Mason Yu
#if 0
		ifMap = 0;
		cnt=0;
		totalEntry = mib_chain_total(MIB_ATM_VC_TBL);

		for (i=0; i<totalEntry; i++) {
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			{
	  			printf("Get chain record error!\n");
				return error;
			}

			if (Entry.vpi == entry.vpi && Entry.vci == entry.vci)
			{
				cnt++;
				pmyEntry = &Entry;
			}

			ifMap |= 1 << (Entry.ifIndex & 0x0f);	// vc map
			ifMap |= (1 << 16) << ((Entry.ifIndex >> 4) & 0x0f);	// PPP map

#ifdef DEFAULT_GATEWAY_V1
			if ((drouted == -1) && Entry.dgw && (Entry.cmode != ADSL_BR1483))	// default route entry
				drouted = i;
#endif
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
#endif  // #if 0

#ifdef DEFAULT_GATEWAY_V1
		if (entry.cmode != ADSL_BR1483)
			if (entry.dgw && drouted != -1) {	// default route already existed
				if (!mib_chain_get(MIB_ATM_VC_TBL, drouted, (void *)&Entry)) {
					Entry.dgw = 0;
					mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, drouted);
				}
			}

      // set default
//		entry.dgw = 1;
#endif

		intVal = mib_chain_add(MIB_ATM_VC_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			printf(strAddChainerror);
			return error;
		}
		else if (intVal == -1)
			printf(strTableFull);

	} while (0);

	return 1;
}

int addroutes(char *destNet,char *subMask,char *nextHop
#ifdef XML_TR069
,int Enable, int Type, char *SourceIP, char *SourceMask, int OutInf, int FWMetric, unsigned int InstanceNum
#else
,int Enable, int OutInf, int FWMetric
#endif
){
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

//#ifdef _CWMP_MIB_
	entry.Enable= (Enable==0)?0:1;
#ifdef XML_TR069
	entry.Type=(unsigned char)Type;
	inet_aton(SourceIP, (struct in_addr *)&entry.SourceIP);
	inet_aton(SourceMask, (struct in_addr *)&entry.SourceMask);
	entry.ifIndex=getWanifIndex( OutInf );
#endif
	entry.ifIndex=OutInf;
	entry.FWMetric=FWMetric;
#ifdef XML_TR069
	entry.InstanceNum=InstanceNum;
#endif
//#endif



	intVal = mib_chain_add(MIB_IP_ROUTE_TBL, (unsigned char*)&entry);
	if (intVal == 0) {
		printf(strAddChainerror);
		return error;
	}
	else if (intVal == -1)
		printf(strTableFull);
	return 1;
}

#ifdef IP_ACL
#ifdef ACL_IP_RANGE
int addacl(char * startIPAddr, char * endIPAddr, char *IPAddr, char enabled, char Interface)
#else
int addacl(char *IPAddr, char *netMask, char enabled, char Interface)
#endif
{
	MIB_CE_ACL_IP_T entry;
	struct in_addr *addr;
	struct in_addr aclip;
	int intVal;
	unsigned long  mask, mbit;

	if(!inet_aton(IPAddr, (void *)&aclip)){//1:success 0:fail
			return error;
	}

	// add into configuration (chain record)
	addr = (struct in_addr *)&entry.ipAddr;
	*addr = aclip;
#ifdef ACL_IP_RANGE
	if(!inet_aton(startIPAddr, (void *)&aclip)){//1:success 0:fail
			return error;
	}

	// add into configuration (chain record)
	addr = (struct in_addr *)&entry.startipAddr;
	*addr = aclip;

	if(!inet_aton(endIPAddr, (void *)&aclip)){//1:success 0:fail
			return error;
	}

	// add into configuration (chain record)
	addr = (struct in_addr *)&entry.endipAddr;
	*addr = aclip;
#endif
	inet_aton(netMask, (struct in_addr *)&mask);
	mbit=0;
	while (1) {
		if (mask&0x80000000) {
			mbit++;
			mask <<= 1;
		}
		else
			break;
	}
	entry.maskbit = mbit;
	entry.Enabled=enabled;
	if ( Interface == 0 )
		entry.Interface=IF_DOMAIN_LAN;
	else if ( Interface == 1 )
		entry.Interface=IF_DOMAIN_WAN;
	else {
		printf("Error: The Interface is error for ACL IP XML configuration!\n");
		return error;
	}

	intVal = mib_chain_add(MIB_ACL_IP_TBL, (unsigned char*)&entry);
	if (intVal == 0) {
		printf(strAddChainerror);
		return error;
	}
	else if (intVal == -1)
		printf(strTableFull);
	return 1;
}
#endif

#ifdef MAC_ACL
//ql_xu add
int addMacAcl(char *macAddr, char enabled, char Interface){
	MIB_CE_ACL_MAC_T entry;
	int i, intVal;

	for(i=0;i<17;i++){
		if((i+1)%3 != 0)
			macAddr[i-(i+1)/3] = macAddr[i];
	}
	macAddr[12] = '\0';
	if ( !string_to_hex(macAddr, entry.macAddr, 12)) {
		printf("retrieve mac addr error\n");
		return 0;
	}

	entry.Enabled=enabled;
	if ( Interface == 0 )
		entry.Interface=IF_DOMAIN_LAN;
	else if ( Interface == 1 )
		entry.Interface=IF_DOMAIN_WAN;
	else {
		printf("Error: The Interface is error for ACL MAC XML configuration!\n");
		return error;
	}

	intVal = mib_chain_add(MIB_ACL_MAC_TBL, (unsigned char*)&entry);
	if (intVal == 0) {
		printf(strAddChainerror);
		return 0;
	}
	else if (intVal == -1)
		printf(strTableFull);
	return 1;
}
#endif

#ifdef NAT_CONN_LIMIT
int addConnLimit(char *ipAddr, char enabled, int connNum)
{
	MIB_CE_CONN_LIMIT_T Entry;
	int intVal;

	if(!inet_aton(ipAddr, (struct in_addr *)&Entry.ipAddr)) {
		printf("get IP error!\n");
		return error;
	}
	Entry.Enabled = enabled;
	Entry.connNum = connNum;

	intVal = mib_chain_add(MIB_CONN_LIMIT_TBL, (unsigned char *)&Entry);
	if (intVal == 0) {
		printf(strAddChainerror);
		return 0;
	}
	else if (intVal == -1)
		printf(strTableFull);
	return 1;
}
#endif
#ifdef TCP_UDP_CONN_LIMIT
int addConnLimit(char *ipAddr, char enabled, int connNum,unsigned char protocol)
{
	MIB_CE_TCP_UDP_CONN_LIMIT_T Entry;
	int intVal;

	if(!inet_aton(ipAddr, (struct in_addr *)&Entry.ipAddr)) {
		printf("get IP error!\n");
		return error;
	}
	Entry.Enabled = enabled;
	Entry.connNum = connNum;
	Entry.protocol = protocol;

	intVal = mib_chain_add(MIB_TCP_UDP_CONN_LIMIT_TBL, (unsigned char *)&Entry);
	if (intVal == 0) {
		printf("Error: Add connlimit chain record!\n");
		return 0;
	}
	else if (intVal == -1)
		printf(strTableFull);
	return 1;

}
#endif // end if TCP_UDP_CONN_LIMIT

#ifdef ADDRESS_MAPPING
#ifdef MULTI_ADDRESS_MAPPING
int addAddrMapping(unsigned int addrMapType, char *lsip,char *leip, char *gsip, char *geip)
{
	MIB_CE_MULTI_ADDR_MAPPING_LIMIT_T Entry;
	int intVal;

	if(!inet_aton(lsip, (struct in_addr *)&Entry.lsip)) {
		printf("get IP error!\n");
		return error;
	}
	if(!inet_aton(leip, (struct in_addr *)&Entry.leip)) {
		printf("get IP error!\n");
		return error;
	}
	if(!inet_aton(gsip, (struct in_addr *)&Entry.gsip)) {
		printf("get IP error!\n");
		return error;
	}
	if(!inet_aton(geip, (struct in_addr *)&Entry.geip)) {
		printf("get IP error!\n");
		return error;
	}


	Entry.addressMapType = addrMapType;

	intVal = mib_chain_add(MULTI_ADDRESS_MAPPING_LIMIT_TBL, (unsigned char *)&Entry);
	if (intVal == 0) {
		printf("Error: Add addr map chain record!\n");
		return 0;
	}
	else if (intVal == -1)
		printf(strTableFull);
	return 1;

}
#endif // end if MULTI_ADDRESS_MAPPING
#endif//ADDRESS_MAPPING
#ifdef WLAN_SUPPORT
#ifdef WLAN_ACL
int addWLanAC(unsigned char *mac)
{
	MIB_CE_WLAN_AC_T macEntry;
	MIB_CE_WLAN_AC_T Entry;
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
		if (!mib_chain_get(MIB_WLAN_AC_TBL, i, (void *)&Entry))
		{
			printf("Get chain record error!\n");
			return error;
		}

		if (!memcmp(macEntry.macAddr, Entry.macAddr, 6))
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
	else if (intVal == -1)
		printf(strTableFull);
	return 1;
}
#endif
#endif

#ifdef WLAN_WDS
int addWLanWDS(unsigned char *mac, unsigned char *comment)
{
	WDS_T macEntry;
	unsigned char entryNum;
	int intVal;

	if (strlen(mac)!=12 || !string_to_hex(mac, macEntry.macAddr, 12)) {
		printf("Error! Invalid MAC address.");
		return;
	}

	if (comment != NULL) {
		if (strlen(comment) > COMMENT_LEN-1) {
			printf("Error! Comment length too long.");
			return;
		}
		strcpy(macEntry.comment, comment);
	}
	else
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
	else if (intVal == -1)
		printf(strTableFull);
	entryNum++;
	if ( !mib_set(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
		printf("Set entry number error!");
		return error;
	}
	return 1;
}
#endif

int showInf(void) // show all itf
{
	int ifnum=0;

	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}

		if (Entry.enable == 0)
			continue;
		IFsel[ifnum] = Entry.ifIndex;
		ifnum++;
	}
	return ifnum;
}

//#ifdef CONFIG_EXT_SWITCH
#ifdef NEW_IP_QOS_SUPPORT
int QosTcAdd(MIB_CE_IP_TC_Tp pEntry)
{
	int totalEntry;

	totalEntry = mib_chain_total(MIB_IP_QOS_TC_TBL);
	if (totalEntry >= MAX_QOS_RULE) {
		printf("Error: Maximum number of Qos rule exceeds!\n");
		return error;
	}

	if (!mib_chain_add(MIB_IP_QOS_TC_TBL, (void *)pEntry)) {
		printf("Error: Add Qos rule fail!\n");
		return error;
	}

	return 1;
}
#endif

#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
//#ifdef _CWMP_MIB_
#ifdef XML_TR069
int Qosadd(unsigned char *sIp, unsigned char *sMask, unsigned short srcport,
			unsigned char *dIp, unsigned char *dMask, unsigned short dstport,
			unsigned char prType, unsigned char *phyport,
#ifdef NEW_IP_QOS_SUPPORT
			unsigned char *qosDscp, unsigned char *vlan1p,
#endif
			unsigned char outIf, unsigned char *pri, unsigned char *m_ipprio, unsigned char *m_iptos, unsigned char *m_1p, unsigned char *enable, unsigned char *instancenum
#ifdef QOS_SPEED_LIMIT_SUPPORT
,char* qosSpeedEnable,char *qosSpeedRank
#endif
#ifdef QOS_DSCP
			,unsigned char *m_dscp
#endif
)
#else
int Qosadd(unsigned char *sIp, unsigned char *sMask, unsigned short srcport,
			unsigned char *dIp, unsigned char *dMask, unsigned short dstport,
			//unsigned char prType, unsigned char *phyport, unsigned char outIf, unsigned char *pri, unsigned char *m_ipprio, unsigned char *m_iptos, unsigned char *m_1p
			unsigned char prType, unsigned char *phyport,
#ifdef NEW_IP_QOS_SUPPORT
			unsigned char *qosDscp, unsigned char *vlan1p,
#endif
			unsigned char outIf, unsigned char *pri, unsigned char *m_ipprio, unsigned char *m_iptos, unsigned char *m_1p, unsigned char *enable
#ifdef QOS_SPEED_LIMIT_SUPPORT
,char* qosSpeedEnable,char *qosSpeedRank
#endif
#ifdef QOS_DSCP
			,unsigned char *m_dscp
#endif
)
#endif
{

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
	if (prType > 3) {
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
	if (entry.protoType == PROTO_TCP || entry.protoType == PROTO_UDP)
		entry.sPort = (unsigned short)srcport;

	// Destination address
	if (dIp[0]) {
		if (!inet_aton(dIp, (struct in_addr *)&entry.dip)) {
			printf("Error! Destination IP.");
			goto setErr_qos;
		}
		if (dMask[0]) {  // netmask bug
			if (!inet_aton(dMask, (struct in_addr *)&mask)) {
				printf("Error! Destination Netmask.");
				goto setErr_qos;
			}
			#if 0
			if (mask==0) {
				printf("Error! Destination Netmask.");
				goto setErr_qos;
			}
			#endif
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
	if (entry.protoType == PROTO_TCP || entry.protoType == PROTO_UDP)
		entry.dPort = (unsigned short)dstport;

// Mason Yu
#ifdef CONFIG_EXT_SWITCH
	// physical port
	if (phyport[0]) {
#ifdef ZTE_GENERAL_ROUTER_SC
        //jim we should take care of wlan0 and vap1, vap2, vap3, vap4...
		if (!string_to_dec(phyport, &intVal) || intVal < 0 ||( intVal > 9&&intVal!=255)) {
			printf("Error! Invalid physical port (0~9).");
			goto setErr_qos;
		}
		for (i=0; i<SW_PORT_NUM; i++) {
			if (virt2user[i] == intVal)
				break;
		}
		if (i!=SW_PORT_NUM)
			entry.phyPort = i;
		else
        {

			entry.phyPort = intVal;
        }
#else
		if (!string_to_dec(phyport, &intVal) || intVal < 0 || (intVal > 4 && intVal !=255)) {
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
#endif
	}
	else {
		entry.phyPort = 0xff;
	}
#else
	if (!string_to_dec(phyport, &intVal) || intVal < 0 || (intVal > 4 && intVal !=255)) {
		printf("Error! Invalid physical port (0~4).");
		goto setErr_qos;
	}
	//printf("intVal=%d\n", intVal);
	entry.phyPort = intVal;
#endif  // #ifdef CONFIG_EXT_SWITCH

#ifdef NEW_IP_QOS_SUPPORT
	if (qosDscp[0]) {
		if(!string_to_dec(qosDscp, &intVal) )
       	{
       		printf("Error! invalid DSCP value\n");
			goto setErr_qos;
		}
		entry.qosDscp = (unsigned char)intVal;
	} else {
		entry.qosDscp = 0;
	}

	if (vlan1p[0]) {
		if(!string_to_dec(vlan1p, &intVal) )
       	{
       		printf("Error! invalid DSCP value\n");
			goto setErr_qos;
		}
		entry.vlan1p = (unsigned char)intVal;
	} else {
		entry.vlan1p = 0;
	}
#endif

	// priority
	if (pri[0]) {
#ifdef ZTE_GENERAL_ROUTER_SC
        //jim valid priority value is high---> low: 0, 1, 2, 3
		if ( !string_to_dec(pri, &intVal) || intVal < 0 || intVal > 3) {
			printf("Error! Invalid priority (0~3).");
			goto setErr_qos;
		}
#else
		if ( !string_to_dec(pri, &intVal) || intVal < 0 || intVal > 3) {
			printf("Error! Invalid priority (0~2).");
			goto setErr_qos;
		}
#endif
		entry.prior = (unsigned char)intVal;
	}
	else
        entry.prior = 3; // default to lowest

	// Outbound Interface
   if(1) // jim we should take care of invalid outIf value.
    {
        int total_valid_index_num;
        memset(IFsel, 0, sizeof(IFsel));
        total_valid_index_num=showInf();
	    if (outIf < 1 ) {
		    printf("Invalid out bound interface!\n");
		    goto setErr_qos;
    	}
        if(outIf <= total_valid_index_num)
	        entry.outif = (unsigned char)IFsel[outIf-1];
        else
            entry.outif = 0 ;
    }
    //jim, some elements of entry is loss, should patch here

#ifdef QOS_DSCP
	if (m_dscp[0]) {
		entry.dscp = 1;
	       if(!string_to_dec(m_dscp, &intVal) )
       	{
	           printf("Error! invalid m_dscp DSCP value\n");
       	    goto setErr_qos;
	       }
#ifndef NEW_IP_QOS_SUPPORT
		entry.m_ipprio = intVal >> 3;
		entry.m_iptos = (intVal & 0x07) >> 1;
#else
		entry.m_dscp = intVal;
#endif
	}
	else {
		entry.dscp = 0;
#endif
	//m_ipprio
    if(m_ipprio[0])
    {
       if(!string_to_dec(m_ipprio, &intVal) || intVal < 0 || intVal > 8)
       {
           printf("Error! invalid m_ip precedence priority (0-8)\n");
           goto setErr_qos;
       }
       entry.m_ipprio=intVal;
    }
    //m_iptos
    if(m_iptos[0])
    {
       if(!string_to_dec(m_iptos, &intVal) )
       {
           printf("Error! invalid m_ip precedence priority (0-8)\n");
           goto setErr_qos;
       }
       entry.m_iptos=intVal;
    }
#ifdef QOS_DSCP
	}
#endif
    //m_1p
    if(m_1p[0])
    {
       if(!string_to_dec(m_1p, &intVal) )
       {
           printf("Error! invalid m_ip precedence priority (0-8)\n");
           goto setErr_qos;
       }
       entry.m_1p=intVal;
    }

//#ifdef _CWMP_MIB_
    //enable state
    if(enable[0])
    {
       if(!string_to_dec(enable, &intVal) )
       {
           printf("Error! invalid m_ip precedence priority (0-8)\n");
           goto setErr_qos;
       }
       entry.enable=intVal;
    }
#ifdef XML_TR069
    //instancenum;
    if(instancenum[0])
    {
       if(!string_to_dec(instancenum, &intVal) )
       {
           printf("Error! invalid m_ip precedence priority (0-8)\n");
           goto setErr_qos;
       }
       entry.InstanceNum=intVal;
    }
#endif
//#endif


#ifdef QOS_SPEED_LIMIT_SUPPORT
if(qosSpeedEnable[0])
{
 if(!string_to_dec(qosSpeedEnable, &intVal) )
       {
           printf("Error! invalid qos_speed_enable  (0-1)\n");
           goto setErr_qos;
       }
       entry.limitSpeedEnabled=intVal;
}
if(qosSpeedRank[0])
{
 if(!string_to_dec(qosSpeedRank, &intVal) )
       {
           printf("Error! invalid qos_speed_rank  (0-10240)\n");
           goto setErr_qos;
       }
       entry.limitSpeedRank=intVal;
}
#endif
	intVal = mib_chain_add(MIB_IP_QOS_TBL, (unsigned char*)&entry);
	if (intVal == 0) {
		printf(strAddChainerror);
		goto setErr_qos;
	}
	else if (intVal == -1)
		printf(strTableFull);
	return 1;

setErr_qos:
	printf("Set QoS rule error!\n");
	return error;
}
#ifdef QOS_SPEED_LIMIT_SUPPORT
int addQosLimitSpeed(char* index,char* count,char* speed,char* prior)
{
int intVal;
MIB_CE_IP_QOS_SPEEDRANK_T entry;
if(index[0])
{
 if(!string_to_dec(index, &intVal) )
       {
           printf("Error! invalid qos_speed_index  (0-10240)\n");
           goto setErr_qosspeed;
       }
       entry.index=intVal;
}
if(count[0])
{
 if(!string_to_dec(count, &intVal) )
       {
           printf("Error! invalid qos_speed_count  (0-10240)\n");
           goto setErr_qosspeed;
       }
       entry.count=intVal;
}
if(speed[0])
{
 if(!string_to_dec(speed, &intVal) )
       {
           printf("Error! invalid qos_speed_speed  (0-10240)\n");
           goto setErr_qosspeed;
       }
       entry.speed=intVal;
}
if(prior[0])
{
 if(!string_to_dec(prior, &intVal) )
       {
           printf("Error! invalid qos_speed_prior (0-10240)\n");
           goto setErr_qosspeed;
       }
       entry.prior=intVal;
}
intVal = mib_chain_add(MIB_QOS_SPEED_LIMIT, (unsigned char*)&entry);
if (intVal == 0) {
		printf("Error! Add MIB_QOS_SPEED_LIMIT chain record.");
		goto setErr_qosspeed;
	}
else if (intVal == -1)
	printf(strTableFull);
return 1;
setErr_qosspeed:
	printf("Set QoS Speed rule error!\n");
	return error;
}
#endif
#endif	// of IP_QOS
//#endif	// of CONFIG_EXT_SWITCH

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
		//MIB_CE_PORT_TRG_Tp pCheckEntry;
		MIB_CE_PORT_TRG_T CheckEntry;

		if (!mib_chain_get(MIB_PORT_TRG_TBL, i, (void *)&CheckEntry)) {
			printf("Get table entry error\n");
			return error;
		}

		if (!strncmp(name, CheckEntry.name, 32)) {
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
	else if (intVal == -1)
		printf(strTableFull);
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
		//MIB_CE_IP_FW_Tp pCheckEntry;
		MIB_CE_IP_FW_T CheckEntry;

		if (!mib_chain_get(MIB_IP_FW_TBL, i, (void *)&CheckEntry))
			continue;
		v1 = *((unsigned long *)entry.local_ip);
		v2 = *((unsigned long *)entry.remote_ip);
		v3 = *((unsigned long *)CheckEntry.local_ip);
		v4 = *((unsigned long *)CheckEntry.remote_ip);

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
	else if (intVal == -1)
		printf(strTableFull);
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
		else if (intVal == -1)
			printf(strTableFull);
		return 1;
	}
	return error;
}
//star add
int addurlkeyword(char *keyword){

	if (keyword[0]) {
		MIB_CE_KEYWD_FILTER_T entry;
		int intVal;
		memset(&entry, 0, sizeof(entry));

		// add into configuration (chain record)
		strcpy(entry.keyword, keyword);

		intVal = mib_chain_add(MIB_KEYWD_FILTER_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			printf("Error: Add URL keyword chain record.\n");
			return error;
		}
		else if (intVal == -1)
			printf(strTableFull);
		return 1;
	}
	return error;
}
#endif
#ifdef  URL_ALLOWING_SUPPORT
int addurlalw(char *url){

	if (url[0]) {
		MIB_CE_URL_ALLOW_FQDN_T entry;
		int intVal;
		memset(&entry, 0, sizeof(entry));

		// add into configuration (chain record)
		strcpy(entry.fqdn, url);

		intVal = mib_chain_add(MIB_URL_ALLOW_FQDN_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			printf("Error: Add URL chain record.\n");
			return error;
		}
		else if (intVal == -1)
			printf(strTableFull);
		return 1;
	}
	return error;
}
#endif

#ifdef DOMAIN_BLOCKING_SUPPORT
int adddomain(char *domain){

	if (domain[0]) {
		MIB_CE_DOMAIN_BLOCKING_T entry;
		int intVal;
		memset(&entry, 0, sizeof(entry));

		// add into configuration (chain record)
		strcpy(entry.domain, domain);

		intVal = mib_chain_add(MIB_DOMAIN_BLOCKING_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			printf("Error: Add Domain filter chain record.\n");
			return error;
		}
		else if (intVal == -1)
			printf(strTableFull);
		return 1;
	}
	return error;
}
#endif

#if 0
int PPPoESessionAdd(unsigned char ifNo, unsigned char vpi, unsigned short vci,
			unsigned char *acMac, unsigned short session) {

	unsigned int  totalEntry, i;
	MIB_CE_PPPOE_SESSION_T entry;

	memset(&entry, 0, sizeof(entry));
	totalEntry = mib_chain_total(MIB_PPPOE_SESSION_TBL); /* get chain record size */

	// interface identifier
	// min=0; max=7;
	if (ifNo < 0 || ifNo > 7) {
		printf("Invalid interface no!\n");
		goto setErr_session;
	}
	entry.ifNo = ifNo;

	//min=0;max=255;
	if (vpi<0 || vpi>255){
		printf("vpi error\n");
		goto setErr_session;
	}
	entry.vpi = (unsigned char)vpi;

	//min=0;max=65535;
	if (vci<0 ||vci >65535){
		printf("vci error\n");
		goto setErr_session;
	}
	entry.vci = (unsigned short)vci;

	if (strlen(acMac)!=12 || !string_to_hex(acMac, entry.acMac, 12)) {
		printf("Error! Invalid MAC address.");
		goto setErr_session;
	}

	if (session<0){
		printf("session error\n");
		goto setErr_session;
	}
	entry.sessionId = (unsigned short)session;

	if(mib_chain_add(MIB_PPPOE_SESSION_TBL, (unsigned char*)&entry) != 1){
		printf("Error! Add chain record.");
		goto setErr_session;
	}
	return 1;

setErr_session:
	printf("Set PPPoE Session error!\n");
	return error;
}
#endif

//star add
int addMacBaseTbl(MIB_CE_MAC_BASE_DHCP_T entry)
{
	int intVal;
	#if 0
	if (strlen(entry.macAddr)!=17) {
		printf("Error! Invalid MAC address.");
		goto setErr_session;
	}
	if (strlen(entry.ipAddr)!=11) {
		printf("Error! Invalid Ip address.");
		goto setErr_session;
	}
	#endif
	
	intVal = mib_chain_add(MIB_MAC_BASE_DHCP_TBL, (unsigned char*)&entry);
	if (intVal == 0) {
		printf(strAddChainerror);
		goto setErr_session;
	}
	else if (intVal == -1)
		printf(strTableFull);
	return 1;

setErr_session:
	printf("Set Mac Base Table error!\n");
	return error;
}

int addRipTbl(MIB_CE_RIP_T entry)
{
	int intVal;

	if(!(entry.receiveMode == 0 || entry.receiveMode == 1 || entry.receiveMode == 2 || entry.receiveMode == 3))  {
		printf("Error! Invalid ReceiveMode.");
		goto setErr_session;
	}

	if(!(entry.sendMode == 0 || entry.sendMode == 1 || entry.sendMode == 2 || entry.sendMode == 4)) {
		printf("Error! Invalid SendMode.");
		goto setErr_session;
	}

	intVal = mib_chain_add(MIB_RIP_TBL, (unsigned char*)&entry);
	if (intVal == 0) {
		printf("Error! Add rip chain record.");
		goto setErr_session;
	}
	else if (intVal == -1)
		printf(strTableFull);
	return 1;

setErr_session:
	printf("Set Rip Table error!\n");
	return error;

}

#ifdef CONFIG_USER_DDNS
int addDdnsTbl(MIB_CE_DDNS_T entry)
{
	int intVal;
	if(!( strcmp(entry.provider, "dyndns") == 0 || strcmp(entry.provider, "tzo") == 0)){
		printf("Error! Not Supported Provider.");
		goto setErr_session;
	}

	intVal = mib_chain_add(MIB_DDNS_TBL, (unsigned char*)&entry);
	if (intVal == 0) {
		printf("Error! Add ddns chain record.");
		goto setErr_session;
	}
	else if (intVal == -1)
		printf(strTableFull);
	return 1;

setErr_session:
	printf("Set Ddns Table error!\n");
	return error;

}
#endif
#ifdef ZTE_GENERAL_ROUTER_SC
int addVirtualServerRule(MIB_CE_VTL_SVR_T * entry)
{
	printf(__FUNCTION__, "IN\n");
	int intVal = mib_chain_add(MIB_VIRTUAL_SVR_TBL, (unsigned char*)entry);
	if (intVal == 0) {
		printf(strAddChainerror);
		goto setErr_rule;
	}
	else if (intVal == -1)
		printf(strTableFull);
	return 1;

setErr_rule:
	printf("Set PPPoE Session error!\n");
	return error;
}
#endif

#ifdef ACCOUNT_CONFIG
int addAccConfig(unsigned char *user, unsigned char *pass, unsigned char priv) {

	unsigned int  totalEntry, i;
	MIB_CE_ACCOUNT_CONFIG_T entry;
	int intVal;

	memset(&entry, 0, sizeof(entry));
	totalEntry = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL); /* get chain record size */

	strcpy(entry.userName, user);
	strcpy(entry.userPassword, pass);

	// account privilege
	// min=0; max=2;
	if (priv < 0 || priv > 2) {
		printf("Invalid privilege no!\n");
		goto setErr_account;
	}
	entry.privilege = priv;

	intVal = mib_chain_add(MIB_ACCOUNT_CONFIG_TBL, (unsigned char*)&entry);
	if (intVal == 0) {
		printf(strAddChainerror);
		goto setErr_account;
	}
	else if (intVal == -1)
		printf(strTableFull);
	return 1;

setErr_account:
	printf("Set Account Configuration error!\n");
	return error;
}
#endif

#if defined(AUTO_PVC_SEARCH_PURE_OAMPING) || defined(AUTO_PVC_SEARCH_AUTOHUNT)
int addAuotPVCTBL(unsigned int vpi, unsigned int vci)
{
	MIB_AUTO_PVC_SEARCH_T entry;
	int intVal;

	entry.vpi = vpi;
	entry.vci = vci;

	intVal = mib_chain_add(MIB_AUTO_PVC_SEARCH_TBL, (unsigned char*)&entry);
	if (intVal == 0) {
		printf(strAddChainerror);
		return error;
	}
	else if (intVal == -1)
		printf(strTableFull);
	return 1;
}
#endif

#ifdef PORT_FORWARD_ADVANCE
int addPFWAdvance(unsigned char *ip, int OutInf, unsigned char gategory, int rule)
{
	MIB_CE_PORT_FW_ADVANCE_T entry;
	int intVal;

	memset(&entry, 0, sizeof(entry));

	inet_aton(ip, (struct in_addr *)&entry.ipAddr);
	entry.ifIndex = OutInf;
	entry.gategory = gategory;
	entry.rule = rule;

	intVal = mib_chain_add(MIB_PFW_ADVANCE_TBL, (unsigned char*)&entry);
	if (intVal == 0) {
		printf(strAddChainerror);
		return error;
	}
	else if (intVal == -1)
		printf(strTableFull);
	return 1;
}
#endif

int update_setting(char *line, FILE *fp){
	int len, i;
	int ret, isEmpty;
	char *str1, *str2, *tmp, ChainTestStr[20];
	mib_table_entry_T info;
	char *endptr;    // review conf

	str1=str2=NULL;
	//printf("%s",line);
	//parse the line
	sscanf(line, "%s", ChainTestStr);
//	if (!strcmp(ChainTestStr, "</Config_Information_File>")) {
	if (!strcmp(ChainTestStr, CONFIG_TRAILER)) {
//		printf("End of config file...exit.\n");
		return 1;
	}
//	if (!strcmp(ChainTestStr, "<Config_Information_File_8671>")) {
	if (!strcmp(ChainTestStr, CONFIG_HEADER)) {
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
	if(str2!=NULL){
		if(!strcmp(str2," ")){
			//printf("NULL\n");
			return 1;
		}
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
			// Kaohj
			isEmpty = 1;

			while(1){
				int flag = 0;
				// Kaohj
				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_IP_PORT_FILTER_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1,"SrcIP")){
					strcpy(sip,str2);
					flag = 1;
				}
				if(!strcmp(str1,"SrcMask")){
					strcpy(smask,str2);
					flag = 1;
				}
				if(!strcmp(str1,"SrcPortStart")){
					sfromPort=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"SrcPortEnd")){
					stoPort=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"DstIP")){
					strcpy(dip,str2);
					flag = 1;
				}
				if(!strcmp(str1,"DstMask")) {
					strcpy(dmask,str2);
					flag = 1;
				}
				if(!strcmp(str1,"DstPortStart")){
					dfromPort=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"DstPortEnd")){
					dtoPort=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"FilterMode")){
					filterMode=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"protocol")){
					prType=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"Direction")){
					dir=atoi(str2);
					flag = 1;
				}

				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}
			return filterAdd((unsigned char)filterMode,sip,smask,(unsigned short)sfromPort,(unsigned short)stoPort,
                		dip,dmask,(unsigned short)dfromPort,(unsigned short)dtoPort,(unsigned char)dir,(unsigned char)prType);
		}

/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
		if (!strcmp(str1, "DHCP_CLIENT_OPTION_TBL")){
		//MAC_FILTER_TBL
			unsigned char tmpvalue[DHCP_OPT_VAL_LEN*2+1];
			unsigned char value[DHCP_OPT_VAL_LEN];
			int enable,order,tag,usedFor,dhcpOptInstNum,dhcpConSPInstNum;
			int ifIndex, optlen;
			
			//clear orginal record
			if(chain_updated[MIB_DHCP_CLIENT_OPTION_TBL-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_DHCP_CLIENT_OPTION_TBL);//clear chain record 	
				chain_updated[MIB_DHCP_CLIENT_OPTION_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			// Kaohj
			isEmpty = 1;

			while(1){
				int flag = 0;
				
				// Kaohj
				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_DHCP_CLIENT_OPTION_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				if (!strcmp(str1, "ifIndex")){
					ifIndex = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"Enable")){
					enable = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"Order")){
					order = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"Tag")){
					tag = atoi(str2);
					flag = 1;
				}
				if (!strcmp(str1, "Len")){
					optlen = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"Value")){
					strcpy(tmpvalue,str2);
					flag = 1;
				}
				if(!strcmp(str1,"UsedFor")){
					usedFor = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"DhcpOptInstNum")){
					dhcpOptInstNum = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"DhcpConSPInstNum")){
					dhcpConSPInstNum = atoi(str2);
					flag = 1;
				}
				
				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}
			string_to_hex(tmpvalue,value,len*2);
			return dhcpOptionAdd(0, (unsigned char)ifIndex,(unsigned char)enable,(unsigned short)order,(unsigned short)tag,(unsigned char)optlen,value,(unsigned char)usedFor,
				(unsigned char)dhcpOptInstNum,(unsigned char)dhcpConSPInstNum);
		}
		
		if(!strcmp(str1,"DHCP_SERVER_OPTION_TBL")){
			unsigned char tmpvalue[DHCP_OPT_VAL_LEN*2+1];
			unsigned char value[DHCP_OPT_VAL_LEN];
			int enable,order,tag,usedFor,dhcpOptInstNum,dhcpConSPInstNum;
			int ifIndex, optlen;
			
			//clear orginal record
			if(chain_updated[MIB_DHCP_SERVER_OPTION_TBL-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_DHCP_SERVER_OPTION_TBL);//clear chain record 	
				chain_updated[MIB_DHCP_SERVER_OPTION_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			// Kaohj
			isEmpty = 1;

			while(1){
				int flag = 0;
				
				// Kaohj
				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_DHCP_SERVER_OPTION_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				if (!strcmp(str1, "ifIndex")){
					ifIndex = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"Enable")){
					enable = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"Order")){
					order = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"Tag")){
					tag = atoi(str2);
					flag = 1;
				}
				if (!strcmp(str1, "Len")){
					optlen = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"Value")){
					strcpy(tmpvalue,str2);
					flag = 1;
				}
				if(!strcmp(str1,"UsedFor")){
					usedFor = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"DhcpOptInstNum")){
					dhcpOptInstNum = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"DhcpConSPInstNum")){
					dhcpConSPInstNum = atoi(str2);
					flag = 1;
				}
				
				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}
			string_to_hex(tmpvalue,value,len*2);
			return dhcpOptionAdd(1, (unsigned char)ifIndex,(unsigned char)enable,(unsigned short)order,(unsigned short)tag,(unsigned char)optlen,value,(unsigned char)usedFor,
				(unsigned char)dhcpOptInstNum,(unsigned char)dhcpConSPInstNum);
		}

		if (!strcmp(str1, "DHCP_SERVER_POOL_TBL")){
			int enable,poolorder,sourceinterface,vendorclassflag,clientidflag,userclassflag;
			int chaddrflag, localserved, dnsservermode, leasetime;
#ifdef _CWMP_MIB_
			int InstanceNum;
#endif
			unsigned char poolname[MAX_NAME_LEN], vendorclass[OPTION_LEN];
			unsigned char clientid[OPTION_LEN], userclass[OPTION_LEN], chaddr[MAC_ADDR_LEN];
			unsigned char chaddrmask[MAC_ADDR_LEN], startaddr[IP_ADDR_LEN];
			unsigned char endaddr[IP_ADDR_LEN], subnetmask[IP_ADDR_LEN];
			unsigned char iprouter[IP_ADDR_LEN], dnsserver1[IP_ADDR_LEN];
			unsigned char dnsserver2[IP_ADDR_LEN], dnsserver3[IP_ADDR_LEN];
			unsigned char domainname[GENERAL_LEN], dhcprelayip[IP_ADDR_LEN];
			char buftmp[30];
			
			//clear orginal record
			if(chain_updated[MIB_DHCPS_SERVING_POOL_TBL-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_DHCPS_SERVING_POOL_TBL);//clear chain record 	
				chain_updated[MIB_DHCPS_SERVING_POOL_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			// Kaohj
			isEmpty = 1;

			while(1){
				int flag = 0;
				
				// Kaohj
				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_DHCPS_SERVING_POOL_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				if (!strcmp(str1, "enable")){
					enable = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"poolorder")){
					poolorder = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"sourceinterface")){
					sourceinterface = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"vendorclassflag")){
					vendorclassflag = atoi(str2);
					flag = 1;
				}
				if (!strcmp(str1, "clientidflag")){
					clientidflag = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"userclassflag")){
					userclassflag = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"chaddrflag")){
					chaddrflag = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"localserved")){
					localserved = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"dnsservermode")){
					dnsservermode = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1, "leasetime")){
					leasetime = atoi(str2);
					flag = 1;
				}
#ifdef _CWMP_MIB_
				if(!strcmp(str1,"InstanceNum")){
					InstanceNum = atoi(str2);
					flag = 1;
				}
#endif
				
				if(!strcmp(str1,"poolname")){
					strcpy(poolname,str2);
					flag = 1;
				}
				if(!strcmp(str1,"vendorclass")){
					strcpy(vendorclass,str2);
					flag = 1;
				}
				if(!strcmp(str1,"clientid")){
					strcpy(clientid,str2);
					flag = 1;
				}
				if(!strcmp(str1,"userclass")){
					strcpy(userclass,str2);
					flag = 1;
				}
				if(!strcmp(str1,"chaddr")){
					string_to_hex(str2, chaddr, 12);
					flag = 1;
				}
				if(!strcmp(str1,"chaddrmask")){
					string_to_hex(str2, chaddrmask, 12);
					flag = 1;
				}
				if(!strcmp(str1,"startaddr")){
					inet_aton(str2, (struct in_addr *)&startaddr); 
					flag = 1;
				}
				if(!strcmp(str1,"endaddr")){;
					inet_aton(str2, (struct in_addr *)&endaddr);
					flag = 1;
				}
				if(!strcmp(str1,"subnetmask")){
					inet_aton(str2, (struct in_addr *)&subnetmask); 
					flag = 1;
				}
				if(!strcmp(str1,"iprouter")){
					inet_aton(str2, (struct in_addr *)&iprouter); 
					flag = 1;
				}
				if(!strcmp(str1,"dnsserver1")){
					inet_aton(str2, (struct in_addr *)&dnsserver1); 
					flag = 1;
				}
				if(!strcmp(str1,"dnsserver2")){
					inet_aton(str2, (struct in_addr *)&dnsserver2); 
					flag = 1;
				}
				if(!strcmp(str1,"dnsserver3")){
					inet_aton(str2, (struct in_addr *)&dnsserver3); 
					flag = 1;
				}
				if(!strcmp(str1,"domainname")){
					strcpy(domainname,str2);
					flag = 1;
				}
				if(!strcmp(str1,"dhcprelayip")){
					inet_aton(str2, (struct in_addr *)&dhcprelayip); 
					flag = 1;
				}

				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}
			return addDhcpPool(enable, poolorder, poolname, sourceinterface, vendorclass, vendorclassflag,
				clientid, clientidflag, userclass, userclassflag, chaddr, chaddrmask, chaddrflag, 
				localserved, startaddr, endaddr, subnetmask, iprouter, dnsserver1, dnsserver2, dnsserver3, 
				domainname, leasetime, dhcprelayip, dnsservermode
#ifdef _CWMP_MIB_
				, InstanceNum
#endif
				);
		}
#endif
/*ping_zhang:20080919 END*/


#ifdef MAC_FILTER
		if(!strcmp(str1,"MAC_FILTER_TBL")){
		//MAC_FILTER_TBL
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
			// Kaohj
			isEmpty = 1;

			while(1){
				int flag = 0;

				// Kaohj
				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_MAC_FILTER_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1,"SrcMac")){
					strcpy(src_mac,str2);
					flag = 1;
				}
				if(!strcmp(str1,"DstMac")){
					strcpy(dst_mac,str2);
					flag = 1;
				}
				if(!strcmp(str1,"FilterMode")){
					filterMode=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"Direction")){
					dir=atoi(str2);
					flag = 1;
				}

				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}
			return MACFilterAdd((unsigned char)filterMode,src_mac,
				dst_mac,(unsigned char)dir);
		}
#endif // of MAC_FILTER

		if(!strcmp(str1,"PORT_FW_TBL")){
		//PORT_FW_TBL
			char ip[16];
			int fromPort,toPort,protocol;
			unsigned char comment[COMMENT_LEN];
//#ifdef _CWMP_MIB_
			char remotehost[16];
//			int OutInf=0,enable=0,leaseduration=0,externalport=0,InstanceNum=0;
			int OutInf=0,enable=0,leaseduration=0,externalfromport=0,externaltoport=0,InstanceNum=0;

			strcpy( remotehost, "0.0.0.0" );
//#endif

			strcpy(ip,"0.0.0.0");
			fromPort=toPort=protocol=0;

			//clear orginal record
			if(chain_updated[MIB_PORT_FW_TBL-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_PORT_FW_TBL);//clear chain record
				chain_updated[MIB_PORT_FW_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			isEmpty = 1;
			while(1){
				int flag = 0;

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_PORT_FW_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1,"IP")){
					strcpy(ip,str2);
					flag = 1;
				}
				if(!strcmp(str1,"PortStart")){
					fromPort=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"PortEnd")){
					toPort=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"Protocol")){
					protocol=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"Comment")){
					strcpy(comment,str2);
					flag = 1;
				}

//#ifdef _CWMP_MIB_
				if(!strcmp(str1,"OutInf")){
					// review conf
					//OutInf=atoi(str2);
					// The ifIndex is 0x??. Mason Yu
					OutInf = strtol(str2+2, &endptr, 16);
					flag = 1;
				}
				if(!strcmp(str1,"enable")){
					enable=atoi(str2);
					flag = 1;
				}
#ifdef XML_TR069
				if(!strcmp(str1,"leaseduration")){
					leaseduration=atoi(str2);
					flag = 1;
				}
#endif
				if(!strcmp(str1,"remotehost")){
					strcpy( remotehost, str2 );
					flag = 1;
				}
//				if(!strcmp(str1,"externalport"))
//					externalport=atoi(str2);
				if(!strcmp(str1,"externalportStart")){
					externalfromport=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"externalportEnd")){
					externaltoport=atoi(str2);
					flag = 1;
				}
#ifdef XML_TR069
				if(!strcmp(str1,"InstanceNum")){
					InstanceNum=atoi(str2);
					flag = 1;
				}
#endif
//#endif
				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}

			}
			return addPortFW(ip,(unsigned)fromPort,(unsigned short)toPort,(unsigned char)protocol
//#ifdef _CWMP_MIB_
//			,OutInf,enable,(unsigned int)leaseduration,remotehost,(unsigned int)externalport,(unsigned int)InstanceNum
#ifdef XML_TR069
			,OutInf,enable,(unsigned int)leaseduration,remotehost,(unsigned int)externalfromport,(unsigned int)externaltoport,(unsigned int)InstanceNum,comment
#else
			,OutInf,enable,remotehost,(unsigned int)externalfromport,(unsigned int)externaltoport,comment
#endif
//#endif
			);
		}

		if(!strcmp(str1,"ATM_VC_TBL")){
		//ATM_VC_TBL
#ifdef DEFAULT_GATEWAY_V1
			int defaultGW;
#endif
			// Mason Yu
			int ifIndex;
			char *endptr;
			int vpi,vci,encap,channelMode,napt,channelStatus,channelAddrType;
			int br_type;
			int pppConnectType,pppIdleTime, pppAuth, mtu, ipunnum;
			int qos, pcr, cdvt, scr, mbs;
#ifdef CONFIG_EXT_SWITCH
			int vlan,vid,vprio,vpass,itfGroup;
#endif
			char ConfigStr[256],LocalIPAddr[16], RemoteIPAddr[16], SubnetMask[16];
#ifdef CONFIG_GUI_WEB
			char pppUser[50],pppPasswd[50], pppAC[30];
#else
			char pppUser[MAX_PPP_NAME_LEN+1],pppPasswd[MAX_NAME_LEN], pppAC[30];
#endif
#ifdef CONFIG_SPPPD_STATICIP
			int staticPPPoE=0;
#endif
#ifdef _PRMT_TR143_
			int TR143UDPEchoItf=0;
#endif //_PRMT_TR143_
#ifdef XML_TR069
#ifdef _CWMP_MIB_
			int connDisable=0,ConDevInstNum=0,ConIPInstNum=0,ConPPPInstNum=0, autoDisTime=0, warnDisDelay=0;
#ifdef _PRMT_X_CT_COM_PPPOEv2_
			int PPPoEProxyEnable=0, PPPoEProxyMaxUser=0;
#endif //_PRMT_X_CT_COM_PPPOEv2_
#ifdef _PRMT_X_CT_COM_WANEXT_
			int ServiceList=0;
#endif //_PRMT_X_CT_COM_WANEXT_
			char pppService[30],WanName[30]="";
			WanName[0] = 0;
			pppService[0] = 0;
			// Mason Yu. If the string is NULL, we should copy the "null" for the string value for sscanf().
			strcpy(WanName, STR_NULL);
			strcpy(pppService, STR_NULL);
#endif
#endif  // #ifdef XML_TR069

#ifdef DEFAULT_GATEWAY_V1
			defaultGW=0;
#endif
			vpi=vci=encap=channelMode=channelStatus=channelAddrType=pppConnectType=pppIdleTime=0;
			qos = cdvt = scr = mbs = pppAuth = ipunnum = 0;
			napt=1, pcr = ATM_MAX_US_PCR;
			strcpy(LocalIPAddr,"0.0.0.0");
			strcpy(RemoteIPAddr,"0.0.0.0");
			strcpy(SubnetMask,"0.0.0.0");
			ConfigStr[0] = pppUser[0] = pppPasswd[0] = pppAC[0] = '\0';

			//clear orginal record
			if(chain_updated[MIB_ATM_VC_TBL-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_ATM_VC_TBL);//clear chain record
				chain_updated[MIB_ATM_VC_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			isEmpty = 1;

			while(1){
				int flag = 0;

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						//printf("MIB_ATM_VC_TBL: This chain is empty\n");
						mib_chain_clear(MIB_ATM_VC_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				//bridge
				if(!strcmp(str1,"ifIndex")){
					//ifIndex=atoi(str2);
					// The ifIndex is 0x??. Mason Yu
					ifIndex = strtol(str2+2, &endptr, 16);
					flag = 1;
				}
				if(!strcmp(str1,"VPI")){
					vpi=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"VCI")){
					vci=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"Encap")){
					encap=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"ChannelMode")){
					channelMode=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"BridgeType")){
					br_type=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"NAPT")){
					napt=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"ChannelStatus")) {
					channelStatus=atoi(str2);
					flag = 1;
				}
				//ppp
				if(!strcmp(str1,"pppUser")) {
					if (strlen(str2)) {
						strcpy(pppUser,str2);
						flag = 1;
					}
					else {
						printf("Error: PPP user name can't be empty!\n");
						return error;
					}
				}
				if(!strcmp(str1,"pppPasswd")) {
					if (strlen(str2)){
						strcpy(pppPasswd,str2);
						flag = 1;
					}
					else {
						printf("Error: PPP password can't be empty!\n");
						return error;
					}
				}
				if(!strcmp(str1,"pppConnectType")){
					flag = 1;
					pppConnectType=atoi(str2);
				}
				if(!strcmp(str1,"pppIdleTime")){
					pppIdleTime=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"pppACName")) {
					if (strlen(str2)){
						strcpy(pppAC,str2);
						//flag = 1;   // checkString Bug
					}
					else
						strcpy(pppAC, STR_NULL);
					flag = 1;   // checkString Bug
				}
				if(!strcmp(str1,"pppAuth")) {
					pppAuth=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"MTU")){
					mtu=atoi(str2);
					flag = 1;
				}
#ifdef CONFIG_SPPPD_STATICIP
				if(!strcmp(str1,"StaticPPPoE")){
					staticPPPoE=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"StaticIPAddr")){
					strcpy(LocalIPAddr,str2);
					flag = 1;
				}
#endif

				//dhcp
				if(!strcmp(str1,"ChannelAddrType")){
					channelAddrType=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"LocalIPAddr")){
					strcpy(LocalIPAddr,str2);
					flag = 1;
				}
				if(!strcmp(str1,"RemoteIPAddr")){
					strcpy(RemoteIPAddr,str2);
					flag = 1;
				}
				if(!strcmp(str1,"SubnetMask")){
					strcpy(SubnetMask,str2);
					flag = 1;
				}
#ifdef DEFAULT_GATEWAY_V1
				if(!strcmp(str1,"DefaultGW")){
					defaultGW=atoi(str2);
					flag = 1;
				}
#endif

				//ATM setting
				if(!strcmp(str1,"QoS")){
					qos=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"PCR")){
					pcr=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"CDVT")) {
					cdvt=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"SCR")){
					scr=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"MBS")){
					mbs=atoi(str2);
					flag = 1;
				}

				if(!strcmp(str1,"Ipunnum")){
					ipunnum=atoi(str2);
					flag = 1;
				}
#ifdef _PRMT_TR143_
				if(!strcmp(str1,"TR143UDPEchoItf")){
					TR143UDPEchoItf=atoi(str2);
					flag = 1;
				}
#endif //_PRMT_TR143_
#ifdef XML_TR069
#ifdef _CWMP_MIB_
				if(!strcmp(str1,"connDisable")) {
					connDisable=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"ConDevInstNum")){
					ConDevInstNum=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"ConIPInstNum")) {
					ConIPInstNum=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"ConPPPInstNum")){
					ConPPPInstNum=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"autoDisTime")){
					autoDisTime=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"warnDisDelay")){
					warnDisDelay=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"pppServiceName")) {
					if (strlen(str2)){
						strcpy(pppService, str2);
						// flag = 1;  // checkString Bug
					}
					else
						strcpy(pppService, STR_NULL);
					flag = 1; //checkString Bug
				}
				if(!strcmp(str1,"WanName")) {
					if (strlen(str2)){
						strcpy(WanName, str2);
						flag = 1; // checkString Bug
					}
					else
						strcpy(WanName, STR_NULL);
					flag = 1; // checkString Bug
				}
#ifdef _PRMT_X_CT_COM_PPPOEv2_
				if(!strcmp(str1,"PPPoEProxyEnable")){
					PPPoEProxyEnable=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"PPPoEProxyMaxUser")){
					PPPoEProxyMaxUser=atoi(str2);
					flag = 1;
				}
#endif //_PRMT_X_CT_COM_PPPOEv2_
#ifdef _PRMT_X_CT_COM_WANEXT_
				if(!strcmp(str1,"ServiceList")){
					ServiceList=atoi(str2);
					flag = 1;
				}
#endif //_PRMT_X_CT_COM_WANEXT_

#endif //_CWMP_MIB_
#endif // #ifdef XML_TR069

#ifdef CONFIG_EXT_SWITCH
				if(!strcmp(str1,"vlan")){
					vlan=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"vid")){
					vid=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"vprio")){
					vprio=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"vpass")){
					vpass=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"itfGroup")){
					itfGroup=atoi(str2);
					flag = 1;
				}
#endif
				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}

#ifdef XML_TR069
#ifdef _CWMP_MIB_
			// Set default value for TR069's parameters.
			if ( (ConDevInstNum == 0) || (ConIPInstNum == 0 && ConPPPInstNum == 0) ) {
				unsigned int  instnum=0;
				//instnum = findConDevInstNumByPVC( fcEntry.vpi, fcEntry.vci );
				//if(instnum==0)
					instnum = 1 + findMaxConDevInstNum();
				ConDevInstNum = instnum;
				if( (channelMode==ADSL_PPPoE+1) ||
#ifdef PPPOE_PASSTHROUGH
				    ((channelMode==ADSL_BR1483+1)&&(channelMode==BRIDGE_PPPOE+1)) ||
#endif
				    (channelMode==ADSL_PPPoA+1) )
				{
					ConPPPInstNum = 1 + findMaxPPPConInstNum( ConDevInstNum );
					ConIPInstNum = 0;
					printf("Set InstNum for PPP Mode\n");
				}else{
					ConPPPInstNum = 0;
					ConIPInstNum = 1 + findMaxIPConInstNum( ConDevInstNum );
					printf("Set InstNum for IP Mode\n");
				}
			}
#endif
#endif

			//printf("channelMode:%d\n",channelMode);
			if(channelMode==1){
				//ATM_VC_TBL VPI VCI Encap ChannelMode br_type napt ChannelStatus qos pcr cdvt scr mbs
				sprintf(ConfigStr,"ATM_VC_TBL %d %d %d %d %d %d %d %d %d %d %d %d"
#ifdef CONFIG_EXT_SWITCH
					" %d %d %d %d %d"
#endif
#ifdef _PRMT_TR143_
					" %d"
#endif //_PRMT_TR143_
#ifdef XML_TR069
#ifdef _CWMP_MIB_
					" %d %d %d %d %s"
#ifdef _PRMT_X_CT_COM_WANEXT_
					" %d"
#endif //_PRMT_X_CT_COM_WANEXT_
#endif
#endif // #ifdef XML_TR069
					,vpi,vci,encap,channelMode,br_type,napt,channelStatus,qos,pcr,cdvt,scr,mbs
#ifdef CONFIG_EXT_SWITCH
					,vlan,vid,vprio,vpass,itfGroup
#endif
#ifdef _PRMT_TR143_
					,TR143UDPEchoItf
#endif //_PRMT_TR143_
#ifdef XML_TR069
#ifdef _CWMP_MIB_
					,connDisable,ConDevInstNum,ConIPInstNum,ConPPPInstNum, WanName
#ifdef _PRMT_X_CT_COM_WANEXT_
					,ServiceList
#endif //_PRMT_X_CT_COM_WANEXT_
#endif
#endif // #ifdef XML_TR069
					);
			}
			if((channelMode==3) || (channelMode==4)){
				//ATM_VC_TBL VPI VCI Encap ChannelMode br_type napt ChannelStatus qos pcr cdvt scr mbs
				// pppConnectType pppIdleTime defaultGW pppAuth mtu pppUser pppPasswd pppAC
				sprintf(ConfigStr,"ATM_VC_TBL %d %d %d %d %d %d %d %d %d %d %d %d"
#ifdef CONFIG_EXT_SWITCH
					" %d %d %d %d %d"
#endif
#ifdef _PRMT_TR143_
					" %d"
#endif //_PRMT_TR143_
#ifdef XML_TR069
#ifdef _CWMP_MIB_
					" %d %d %d %d %s"
#ifdef _PRMT_X_CT_COM_WANEXT_
					" %d"
#endif //_PRMT_X_CT_COM_WANEXT_
					" %d %d %s"
#ifdef _PRMT_X_CT_COM_PPPOEv2_
					" %d %d"
#endif //_PRMT_X_CT_COM_PPPOEv2_
#endif //_CWMP_MIB_
#endif // #ifdef XML_TR069

#ifdef DEFAULT_GATEWAY_V1
					" %d %d %d %d %d %s %s %s"
#else
					" %d %d %d %d %s %s %s"
#endif
#ifdef  ZTE_GENERAL_ROUTER_SC
                                   " %d"
#endif
#ifdef CONFIG_SPPPD_STATICIP
					" %d %s"
#endif
					,vpi,vci,encap,channelMode,br_type,napt,channelStatus,qos,pcr,cdvt,scr,mbs,
#ifdef CONFIG_EXT_SWITCH
					vlan,vid,vprio,vpass,itfGroup,
#endif

#ifdef _PRMT_TR143_
					TR143UDPEchoItf,
#endif //_PRMT_TR143_
#ifdef XML_TR069
#ifdef _CWMP_MIB_
					connDisable,ConDevInstNum,ConIPInstNum,ConPPPInstNum, WanName,
#ifdef _PRMT_X_CT_COM_WANEXT_
					ServiceList,
#endif //_PRMT_X_CT_COM_WANEXT_
					autoDisTime, warnDisDelay, pppService,
#ifdef _PRMT_X_CT_COM_PPPOEv2_
					PPPoEProxyEnable, PPPoEProxyMaxUser,
#endif //_PRMT_X_CT_COM_PPPOEv2_
#endif //_CWMP_MIB_
#endif // #ifdef XML_TR069

#ifdef DEFAULT_GATEWAY_V1
					pppConnectType,pppIdleTime,defaultGW,pppAuth,mtu,pppUser,pppPasswd,pppAC
#else
					pppConnectType,pppIdleTime,pppAuth,mtu,pppUser,pppPasswd,pppAC
#endif
#ifdef  ZTE_GENERAL_ROUTER_SC
                                   ,ipunnum
#endif
#ifdef CONFIG_SPPPD_STATICIP
					,staticPPPoE,LocalIPAddr
#endif
				);
			}
			if((channelMode==2) || (channelMode==5)){
				//ATM_VC_TBL VPI VCI Encap ChannelMode br_type napt ChannelStatus qos pcr cdvt scr mbs
				// channelAddrType LocalIPAddr RemoteIPAddr SubnetMask defaultGW ipunnum
				sprintf(ConfigStr,"ATM_VC_TBL %d %d %d %d %d %d %d %d %d %d %d %d"
#ifdef CONFIG_EXT_SWITCH
					" %d %d %d %d %d"
#endif
#ifdef _PRMT_TR143_
					" %d"
#endif //_PRMT_TR143_
#ifdef XML_TR069
#ifdef _CWMP_MIB_
					" %d %d %d %d %s"
#ifdef _PRMT_X_CT_COM_WANEXT_
					" %d"
#endif //_PRMT_X_CT_COM_WANEXT_
#endif
#endif // #ifdef XML_TR069

#ifdef DEFAULT_GATEWAY_V1
					" %d %s %s %s %d %d",
#else
					" %d %s %s %s %d",
#endif
					vpi,vci,encap,channelMode,br_type,napt,channelStatus,qos,pcr,cdvt,scr,mbs,
#ifdef CONFIG_EXT_SWITCH
					vlan,vid,vprio,vpass,itfGroup,
#endif
#ifdef _PRMT_TR143_
					TR143UDPEchoItf,
#endif //_PRMT_TR143_
#ifdef XML_TR069
#ifdef _CWMP_MIB_
					connDisable,ConDevInstNum,ConIPInstNum,ConPPPInstNum,WanName,
#ifdef _PRMT_X_CT_COM_WANEXT_
					ServiceList,
#endif //_PRMT_X_CT_COM_WANEXT_
#endif
#endif // #ifdef XML_TR069

#ifdef DEFAULT_GATEWAY_V1
					channelAddrType,LocalIPAddr,RemoteIPAddr,SubnetMask,defaultGW,ipunnum);
#else
					channelAddrType,LocalIPAddr,RemoteIPAddr,SubnetMask,ipunnum);
#endif
			}
			return addChannelConfiguration(ConfigStr, (unsigned char)ifIndex);
		}

		if(!strcmp(str1,"IP_ROUTE_TBL")){
		//IP_ROUTE_TBL
			char DstIP[16],Mask[16],NextHop[16];
//#ifdef _CWMP_MIB_
			int Enable=0,Type=0,OutInf=0,FWMetric=0,InstanceNum=0;
			char SourceIP[16],SourceMask[16];

			strcpy(SourceIP,"0.0.0.0");
			strcpy(SourceMask,"0.0.0.0");
//#endif

			strcpy(DstIP,"0.0.0.0");
			strcpy(Mask,"0.0.0.0");
			strcpy(NextHop,"0.0.0.0");

			//clear orginal record
			if(chain_updated[MIB_IP_ROUTE_TBL-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_IP_ROUTE_TBL);//clear chain record
				chain_updated[MIB_IP_ROUTE_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			isEmpty = 1;

			while(1){
				int flag = 0;

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_IP_ROUTE_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1,"DstIP")){
					strcpy(DstIP,str2);
					flag = 1;
				}
				if(!strcmp(str1,"Mask")){
					strcpy(Mask,str2);
					flag = 1;
				}
				if(!strcmp(str1,"NextHop")){
					strcpy(NextHop,str2);
					flag = 1;
				}

//#ifdef _CWMP_MIB_
				if(!strcmp(str1,"Enable")){
					Enable=atoi( str2 );
					flag = 1;
				}
#ifdef XML_TR069
				if(!strcmp(str1,"Type")){
					Type=atoi( str2 );
					flag = 1;
				}
				if(!strcmp(str1,"SourceIP")){
					strcpy(SourceIP,str2);
					flag = 1;
				}
				if(!strcmp(str1,"SourceMask")){
					strcpy(SourceMask,str2);
					flag = 1;
				}
#endif
				if(!strcmp(str1,"OutInf")){
					//OutInf=atoi( str2 );
					// The ifIndex is 0x??. Mason Yu
					OutInf = strtol(str2+2, &endptr, 16);
					flag = 1;
				}
				if(!strcmp(str1,"FWMetric")){
					FWMetric=atoi( str2 );
					flag = 1;
				}
#ifdef XML_TR069
				if(!strcmp(str1,"InstanceNum")){
					InstanceNum=atoi( str2 );
					flag = 1;
				}
#endif
//#endif
				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}

			}
			return addroutes(DstIP,Mask,NextHop
#ifdef XML_TR069
				,Enable,Type,SourceIP,SourceMask,OutInf,FWMetric,(unsigned int)InstanceNum
#else
				,Enable,OutInf,FWMetric
#endif
			);
		}

#ifdef IP_ACL
		if(!strcmp(str1,"ACL_IP_TBL")){
		//ACL_IP_TBL
			char IPAddr[16], netMask[16];
		#ifdef ACL_IP_RANGE
			char startIPAddr[16],endIPAddr[16];
		#endif
			char enabled=0;
			char Interface=0;
			strcpy(IPAddr,"0.0.0.0");
			strcpy(netMask,"0.0.0.0");

			//clear orginal record
			if(chain_updated[MIB_ACL_IP_TBL-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_ACL_IP_TBL);//clear chain record
				chain_updated[MIB_ACL_IP_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			isEmpty = 1;

			while(1){
				int flag = 0;

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_ACL_IP_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
			#ifdef ACL_IP_RANGE
				if(!strcmp(str1,"StartIPAddr")){
					strcpy(startIPAddr,str2);
					flag = 1;
				}
				if(!strcmp(str1,"EndIPAddr")){
					strcpy(endIPAddr,str2);
					flag = 1;
				}
			#endif
				if(!strcmp(str1,"IPAddr")){
					strcpy(IPAddr,str2);
					flag = 1;
				}
				if(!strcmp(str1,"NetMask")){
					strcpy(netMask,str2);
					flag = 1;
				}
				if(!strcmp(str1,"State")){
					enabled=atoi( str2 );
					flag = 1;
				}
				if(!strcmp(str1,"Interface")){
					Interface=atoi( str2 );
					flag = 1;
				}

				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}
		#ifdef ACL_IP_RANGE
			return addacl(startIPAddr, endIPAddr, IPAddr, enabled, Interface);
		#else
			return addacl(IPAddr, netMask, enabled, Interface);
		#endif
		}
#endif

#ifdef MAC_ACL
		//ql_xu add
		if(!strcmp(str1,"ACL_MAC_TBL")){
			char macAddr[18];
			char enabled=0;
			char Interface=0;

			//clear orginal record
			if(chain_updated[MIB_ACL_MAC_TBL-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_ACL_MAC_TBL);//clear chain record
				chain_updated[MIB_ACL_MAC_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			isEmpty = 1;

			while(1){
				int flag = 0;

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_ACL_MAC_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1,"MacAddr")){
					strcpy(macAddr,str2);
					flag = 1;
				}
				if(!strcmp(str1,"State")) {
					enabled=atoi( str2 );
					flag = 1;
				}
				if(!strcmp(str1,"Interface")){
					Interface=atoi( str2 );
					flag = 1;
				}

				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}
			return addMacAcl(macAddr, enabled, Interface);
		}
#endif

#ifdef NAT_CONN_LIMIT
		//ql_xu add
		if(!strcmp(str1,"NAT_CONN_LIMIT")){
			char IPAddr[16];
			char enabled=0;
			int connnum=0;

			//clear orginal record
			if(chain_updated[MIB_CONN_LIMIT_TBL - CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_CONN_LIMIT_TBL);//clear chain record
				chain_updated[MIB_CONN_LIMIT_TBL - CHAIN_ENTRY_TBL_ID]=1;
			}
			isEmpty = 1;

			while(1){
				int flag = 0;

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_CONN_LIMIT_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1,"IPAddr")){
					strcpy(IPAddr,str2);
					flag = 1;
				}
				if(!strcmp(str1,"State")){
					enabled=atoi( str2 );
					flag = 1;
				}
				if(!strcmp(str1,"connNum")){
					connnum=atoi( str2 );
					flag = 1;
				}

				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}
			return addConnLimit(IPAddr, enabled, connnum);
		}
#endif
#ifdef ADDRESS_MAPPING
#ifdef MULTI_ADDRESS_MAPPING
	//update xml config
		if(!strcmp(str1,"ADDRESS_MAPPING")){
			char lsip[16],leip[16],gsip[16],geip[16];
			char AddrMapType=0;
			//clear orginal record
			if(chain_updated[MULTI_ADDRESS_MAPPING_LIMIT_TBL- CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MULTI_ADDRESS_MAPPING_LIMIT_TBL);//clear chain record
				chain_updated[MULTI_ADDRESS_MAPPING_LIMIT_TBL - CHAIN_ENTRY_TBL_ID]=1;
			}
			isEmpty = 1;

			while(1){
				int flag = 0;

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MULTI_ADDRESS_MAPPING_LIMIT_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;

				 if(!strcmp(str1,"AddressMappingType")){
					AddrMapType=getaddrmappingidx( str2 );
					flag = 1;
				}
				else if(!strcmp(str1,"LocalStartIP")){
					strcpy(lsip,str2);
					flag = 1;
				}
				else if(!strcmp(str1,"LocalEndIP")){
					strcpy(leip,str2);
					flag = 1;
				}
				else if(!strcmp(str1,"GlobalStartIP")){
					strcpy(gsip,str2);
					flag = 1;
				}
				else if(!strcmp(str1,"GlobalEndIP")){
					strcpy(geip,str2);
					flag = 1;
				}
				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}
			return addAddrMapping(AddrMapType,  lsip,  leip,  gsip, geip);
		}
#endif
#endif

#ifdef TCP_UDP_CONN_LIMIT
	//update xml config
		if(!strcmp(str1,"TCP_UDP_CONN_LIMIT")){
			char IPAddr[16];
			char enabled=0;
			int connnum=0;
			unsigned char protocol=0;

			//clear orginal record
			if(chain_updated[MIB_TCP_UDP_CONN_LIMIT_TBL- CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_TCP_UDP_CONN_LIMIT_TBL);//clear chain record
				chain_updated[MIB_TCP_UDP_CONN_LIMIT_TBL - CHAIN_ENTRY_TBL_ID]=1;
			}
			isEmpty = 1;

			while(1){
				int flag = 0;

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_TCP_UDP_CONN_LIMIT_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1,"IPAddr")){
					strcpy(IPAddr,str2);
					flag = 1;
				}
				if(!strcmp(str1,"State")){
					enabled=atoi( str2 );
					flag = 1;
				}
				if(!strcmp(str1,"connNum")){
					connnum=atoi( str2 );
					flag = 1;
				}
				if(!strcmp(str1,"Protocol")){
					if (!strcmp(str2,"TCP"))
						protocol = 0;
					else
						protocol = 1;
					flag = 1;
				}
				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}
			return addConnLimit(IPAddr, enabled, connnum,protocol);
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
			isEmpty = 1;

			while(1){

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_WLAN_AC_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1,"MacAddr"))
					strcpy(MacAddr,str2);
				else
					printf("The Name %s is invalid for Configuration\n", str1);
			}
			return addWLanAC(MacAddr);
		}
#endif

#ifdef WLAN_MBSSID
		if(!strcmp(str1,"MBSSIB_TBL")){
		//MIB_MBSSIB_TBL
			MIB_CE_MBSSIB_T Entry;
			int cwmp_WLAN_BasicEncry_flag = 0, intVal;

			//clear orginal record
			if(chain_updated[MIB_MBSSIB_TBL-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_MBSSIB_TBL);//clear chain record
				chain_updated[MIB_MBSSIB_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			isEmpty = 1;

			while(1){
				int flag = 0;

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_MBSSIB_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1,"idx")){
					Entry.idx = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"encrypt")){
					Entry.encrypt = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"enable1X")){
					Entry.enable1X = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"wep")){
					Entry.wep = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"wpaAuth")){
					Entry.wpaAuth = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"wpaPSKFormat")){
					Entry.wpaPSKFormat = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"wpaPSK")){
					strcpy(Entry.wpaPSK,str2);
					flag = 1;
				}
				if(!strcmp(str1,"rsPort")){
					Entry.rsPort = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"rsIpAddr")){
					inet_aton(str2, (struct in_addr *)&Entry.rsIpAddr);
					flag = 1;
				}
				if(!strcmp(str1,"rsPassword")){
					strcpy(Entry.rsPassword, str2);
					flag = 1;
				}
				if(!strcmp(str1,"wlanDisabled")){
					Entry.wlanDisabled = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"ssid")){
					strcpy(Entry.ssid,str2);
					flag = 1;
				}
				if(!strcmp(str1,"authType")){
					Entry.authType = atoi(str2);
					flag = 1;
				}
#ifdef XML_TR069
#ifdef _CWMP_MIB_
				if(!strcmp(str1,"cwmp_WLAN_BasicEncry")){
					Entry.cwmp_WLAN_BasicEncry = atoi(str2);
					flag = 1;
					cwmp_WLAN_BasicEncry_flag = 1;
				}
#endif
#endif // #ifdef XML_TR069

#ifdef ENABLE_WPAAES_WPA2TKIP
				if(!strcmp(str1,"unicastCipher")){
					Entry.unicastCipher = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"wpa2UnicastCipher")){
					Entry.wpa2UnicastCipher = atoi(str2);
					flag = 1;
				}
#endif
				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}

#ifdef XML_TR069
#ifdef _CWMP_MIB_
			// Set default value for TR069's parameters.
			if ( cwmp_WLAN_BasicEncry_flag == 0) {
				if ( Entry.encrypt == ENCRYPT_WEP ) {
					Entry.cwmp_WLAN_BasicEncry = 1;
				} else {
					Entry.cwmp_WLAN_BasicEncry = 0;
				}
			}
#endif
#endif
			intVal = mib_chain_add(MIB_MBSSIB_TBL, (void *)&Entry);
			if (intVal == 0) {
				printf(strAddChainerror);
				return error;
			}
			else if (intVal == -1)
				printf(strTableFull);
			return 1;
		}

		if(!strcmp(str1,"MBSSIB_WEP_TBL")){
		//MIB_MBSSIB_WEP_TBL
			MIB_CE_MBSSIB_WEP_T Entry;
			int intVal;
			//clear orginal record
			if(chain_updated[MIB_MBSSIB_WEP_TBL-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_MBSSIB_WEP_TBL);//clear chain record
				chain_updated[MIB_MBSSIB_WEP_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			isEmpty = 1;

			while(1){
				int flag = 0;

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_MBSSIB_WEP_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1,"wep64Key1")){
					if(strlen(str2) == 10){
						string_to_hex(str2, Entry.wep64Key1, 10);
						flag = 1;
					}
				}
				if(!strcmp(str1,"wep64Key2")){
					if(strlen(str2) == 10) {
						string_to_hex(str2, Entry.wep64Key2, 10);
						flag = 1;
					}
				}
				if(!strcmp(str1,"wep64Key3")){
					if(strlen(str2) == 10){
						string_to_hex(str2, Entry.wep64Key3, 10);
						flag = 1;
					}
				}
				if(!strcmp(str1,"wep64Key4")){
					if(strlen(str2) == 10){
						string_to_hex(str2, Entry.wep64Key4, 10);
						flag = 1;
					}
				}
				if(!strcmp(str1,"wep128Key1")){
					if(strlen(str2) == 26){
						string_to_hex(str2, Entry.wep128Key1, 26);
						flag = 1;
					}
				}
				if(!strcmp(str1,"wep128Key2")){
					if(strlen(str2) == 26){
						string_to_hex(str2, Entry.wep128Key2, 26);
						flag = 1;
					}
				}
				if(!strcmp(str1,"wep128Key3")){
					if(strlen(str2) == 26){
						string_to_hex(str2, Entry.wep128Key3, 26);
						flag = 1;
					}
				}
				if(!strcmp(str1,"wep128Key4")){
					if(strlen(str2) == 26){
						string_to_hex(str2, Entry.wep128Key4, 26);
						flag = 1;
					}
				}
				if(!strcmp(str1,"wepDefaultKey")){
					Entry.wepDefaultKey = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"wepKeyType")) {
					Entry.wepKeyType = atoi(str2);
					flag = 1;
				}

				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}

			}
			intVal = mib_chain_add(MIB_MBSSIB_WEP_TBL, (void *)&Entry);
			if (intVal == 0) {
				printf(strAddChainerror);
				return error;
			}
			else if (intVal == -1)
				printf(strTableFull);
			return 1;
		}

#endif

#ifdef XML_TR069
#ifdef _CWMP_MIB_
		if(!strcmp(str1,"CWMP_PSK_TBL")){
		//CWMP_PSK_TBL
			CWMP_PSK_T Entry;
			int intVal;
			//clear orginal record
			if(chain_updated[CWMP_PSK_TBL-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(CWMP_PSK_TBL);//clear chain record
				chain_updated[CWMP_PSK_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			isEmpty = 1;

			while(1){
				int flag = 0;

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(CWMP_PSK_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1,"index")){
					Entry.index = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"presharedkey")){
					strcpy(Entry.presharedkey,str2);
					flag = 1;
				}
				if(!strcmp(str1,"keypassphrase")){
					strcpy(Entry.keypassphrase,str2);
					flag = 1;
				}

				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}

			}
			intVal = mib_chain_add(CWMP_PSK_TBL, (void *)&Entry);
			if (intVal == 0) {
				printf(strAddChainerror);
				return error;
			}
			else if (intVal == -1)
				printf(strTableFull);
			return 1;
		}

#endif /*_CWMP_MIB_*/
#endif // #ifdef XML_TR069
#endif
		#ifndef ZTE_531B_BRIDGE_SC
//modified by ramen
		if(!strcmp(str1,"ACC_TBL")){
		//ACC_TBL
			MIB_CE_ACC_T Entry;
			//clear orginal record
			if(chain_updated[MIB_ACC_TBL-CHAIN_ENTRY_TBL_ID]==0){
				unsigned char entryNum=0;
				mib_chain_clear(MIB_ACC_TBL);//clear chain record
				chain_updated[MIB_ACC_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			memset(&Entry, '\0', sizeof(MIB_CE_ACC_T));
			isEmpty = 1;

			while(1){
				int flag = 0;

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_ACC_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				#ifdef CONFIG_USER_TELNETD_TELNETD
				#ifndef ZTE_GENERAL_ROUTER_SC
				if(!strcmp(str1,"Telnet_LAN")){
					if(atoi(str2)==1){
						Entry.telnet |= 0x02;
					}
					flag = 1;
				}
				#endif
				if(!strcmp(str1,"Telnet_WAN")){
					if(atoi(str2)==1){
						Entry.telnet |= 0x01;
					}
					flag = 1;
				}
				if(!strcmp(str1, "Telnet_WAN_Port")){
					Entry.telnet_port = atoi(str2);
					flag = 1;
				}
				#endif
				//ftp
				#ifdef CONFIG_USER_FTPD_FTPD
				#ifndef ZTE_GENERAL_ROUTER_SC
				if(!strcmp(str1,"Ftp_LAN")){
					if(atoi(str2)==1){
						Entry.ftp |= 0x02;
					}
					flag = 1;
				}
				#endif
				if(!strcmp(str1,"Ftp_WAN")){
					if(atoi(str2)==1){
						Entry.ftp |= 0x01;
					}
					flag = 1;
				}
				if(!strcmp(str1, "Ftp_WAN_Port")){
					Entry.ftp_port = atoi(str2);
					flag = 1;
				}
				#endif
				//tftp
				#ifdef CONFIG_USER_TFTPD_TFTPD
				#ifndef ZTE_GENERAL_ROUTER_SC
				if(!strcmp(str1,"Tftp_LAN")){
					if(atoi(str2)==1){
						Entry.tftp |= 0x02;
					}
					flag = 1;
				}
				#endif
				if(!strcmp(str1,"Tftp_WAN")){
					if(atoi(str2)==1){
						Entry.tftp |= 0x01;
					}
					flag = 1;
				}
				#endif
				//web
				#ifndef ZTE_GENERAL_ROUTER_SC
				if(!strcmp(str1,"Web_LAN")){
					if(atoi(str2)==1){
						Entry.web |= 0x02;
					}
					flag = 1;
				}
				#endif
				if(!strcmp(str1,"Web_WAN")){
					if(atoi(str2)==1){
						Entry.web |= 0x01;
					}
					flag = 1;
				}
				if(!strcmp(str1, "Web_WAN_Port")){
					Entry.web_port = atoi(str2);
					flag = 1;
				}
				//https
				#ifdef CONFIG_USER_BOA_WITH_SSL
				#ifndef ZTE_GENERAL_ROUTER_SC
				if(!strcmp(str1,"Https_LAN")){
					if(atoi(str2)==1){
						Entry.https |= 0x02;
					}
					flag = 1;
				}
				#endif
				if(!strcmp(str1,"Https_WAN")){
					if(atoi(str2)==1){
						Entry.https |= 0x01;
					}
					flag = 1;
				}
				if(!strcmp(str1, "Https_WAN_Port")){
					Entry.https_port = atoi(str2);
					flag = 1;
				}
				#endif // end of CONFIG_USER_BOA_WITH_SSL
				//snmp
				#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
				#ifndef ZTE_GENERAL_ROUTER_SC
				if(!strcmp(str1,"Snmp_LAN")){
					if(atoi(str2)==1){
						Entry.snmp |= 0x02;
					}
					flag = 1;
				}
				#endif
				if(!strcmp(str1,"Snmp_WAN")){
					if(atoi(str2)==1){
						Entry.snmp |= 0x01;
					}
					flag = 1;
				}

				#endif
				//ssh
				#ifdef CONFIG_USER_SSH_DROPBEAR
				#ifndef ZTE_GENERAL_ROUTER_SC
				if(!strcmp(str1,"Ssh_LAN")){
					if(atoi(str2)==1){
						Entry.ssh |= 0x02;
					}
					flag = 1;
				}
				#endif
				if(!strcmp(str1,"Ssh_WAN")){
					if(atoi(str2)==1){
						Entry.ssh |= 0x01;
					}
					flag = 1;
				}
				#endif
				//icmp
				#ifndef ZTE_GENERAL_ROUTER_SC
				if(!strcmp(str1,"Icmp_LAN")){
					if(atoi(str2)==1){
						Entry.icmp |= 0x02;
					}
					flag = 1;
				}
				#endif
				if(!strcmp(str1,"Icmp_WAN")){
					if(atoi(str2)==1){
						Entry.icmp |= 0x01;
					}
					flag = 1;
				}
				#ifdef ZTE_GENERAL_ROUTER_SC
				if(!strcmp(str1,"ifIndex")){
					Entry.ifIndex=atoi(str2);
					flag = 1;
					}
				if(!strcmp(str1,"vpi")){
					Entry.vpi=atoi(str2);
					flag = 1;
					}
				if(!strcmp(str1,"vci")){
					Entry.vci=atoi(str2);
					flag = 1;
					}
				if(!strcmp(str1,"protocol")){
					Entry.protocol=atoi(str2);
					flag = 1;
					}
				#endif

				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}
			mib_chain_add(MIB_ACC_TBL, (void *)&Entry);
			return 1;
		}
#endif

#ifdef WLAN_SUPPORT
#ifdef WLAN_WDS
		if(!strcmp(str1,"WLAN_WDS_TBL")){
		//WLAN_WDS_TBL
			char MacAddr[12], comment[COMMENT_LEN];

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
			isEmpty = 1;

			while(1){
				int flag = 0;

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_WDS_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				/*
				if(!strcmp(str1,"MacAddr"))
					strcpy(MacAddr,str2);
				else {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				*/
				if(!strcmp(str1,"MacAddr")) {
					strcpy(MacAddr,str2);
					flag = 1;
				}
				if(!strcmp(str1,"Comment")) {
					strcpy(comment, str2);
					flag = 1;
				}
				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}
			//return addWLanWDS(MacAddr);
			return addWLanWDS(MacAddr, comment);
		}
#endif	// of WLAN_WDS
#endif

#ifdef CONFIG_EXT_SWITCH

		if(!strcmp(str1,"SW_PORT_TBL")){
		//SW_PORT_TBL
   			int switchno, pvcIf;
			MIB_CE_SW_PORT_T Entry;
			MIB_CE_SW_PORT_Tp pEntry=&Entry;
			int pvcItf,itfGroup,pvid,egressTagAction,linkMode;

			isEmpty = 1;

			while(1){
				int flag = 0;

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						//mib_chain_clear(MIB_SW_PORT_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;

				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1, "Switch"))
					if (str2[0]) {
						switchno = atoi(str2);
						flag = 1;
					}

/*
				if(!strcmp(str1,"PVCInterface")) {
							pvcIf = atoi(str2);
							printf("\npvcIf=%d\n",pvcIf);
							if (pvcIf > showInf()) {
								printf("Invalid PVC interface!\n");
								return error;
							}
							if (pvcIf <= 0)
								pEntry->pvcItf = (unsigned char)0xff;
							else
								pEntry->pvcItf = (unsigned char)IFsel[pvcIf-1];
				}
*/
				pvcItf = 0xff;

				if(!strcmp(str1,"InterfaceGroup")) {
					itfGroup = (unsigned char)atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"PVID")){
					pvid = (unsigned char)(atoi(str2) /*- 1*/);
					flag = 1;
				}
				if(!strcmp(str1,"EgressTag")){
					egressTagAction = (unsigned char)(atoi(str2)/* - 1*/);
					flag = 1;
				}
				if(!strcmp(str1,"LinkMode")){
					linkMode = (unsigned char)(atoi(str2) /*- 1*/);
					flag = 1;
				}

				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}

			}

			mib_chain_get(MIB_SW_PORT_TBL, switchno-1, (void*)&Entry);
			Entry.pvcItf = pvcItf;
			Entry.itfGroup = itfGroup;
			Entry.pvid = pvid;
			Entry.egressTagAction = egressTagAction;
			Entry.linkMode = linkMode;
			mib_chain_update(MIB_SW_PORT_TBL, (void*)&Entry, switchno-1);
			return 1;
		}
#endif

#ifdef NEW_IP_QOS_SUPPORT
		if (!strcmp(str1, "QOS_TC_TBL")) {
			MIB_CE_IP_TC_T entry;
			unsigned char sip[16]={0}, smask[16]={0}, dip[16]={0}, dmask[16]={0};
			unsigned int  mask, mbit, intVal;

			//clear orginal record
			if(chain_updated[MIB_IP_QOS_TC_TBL -CHAIN_ENTRY_TBL_ID]==0){
				unsigned char entryNum=0;
				mib_chain_clear(MIB_IP_QOS_TC_TBL);//clear chain record
				chain_updated[MIB_IP_QOS_TC_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}

			isEmpty = 1;

			while(1){
				int flag = 0;

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_IP_QOS_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;

				if(!strcmp(str1, "entryid")){
					entry.entryid = str2[0];
					flag = 1;
				}
				if(!strcmp(str1, "ifIndex")){
					entry.ifIndex = str2[0];
					flag = 1;
				}
				if(!strcmp(str1, "srcip")){
					strcpy(sip, str2);
					flag = 1;
				}
				if(!strcmp(str1, "srcmask")){
					strcpy(smask, str2);
					flag = 1;
				}
				if(!strcmp(str1, "srcport")){
					entry.sport = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1, "dstip")){
					strcpy(dip, str2);
					flag = 1;
				}
				if(!strcmp(str1, "dstmask")){
					strcpy(dmask, str2);
					flag = 1;
				}
				if(!strcmp(str1,"dstport")){
					entry.dport = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"protocol")){
					entry.protoType = str2[0];
					flag = 1;
				}
				if(!strcmp(str1, "rate")){
					entry.limitSpeed = atoi(str2);
				}

				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}

			if (sip[0]) {
				if (!inet_aton(sip, (struct in_addr *)&entry.srcip)) {
					printf("Error! Source IP.");
					return error;
				}

				if (smask[0]) {
					if (!inet_aton(smask, (struct in_addr *)&mask)) {
						printf("Error! Source Netmask.");
						return error;
					}
					mask = htonl(mask);
					mbit=0; intVal=0;
					for (i=0; i<32; i++) {
						if (mask & 0x80000000) {
							if (intVal) {
								printf("Error! Source Netmask.");
								return error;
							}
							mbit++;
						}
						else
							intVal=1;
						mask <<= 1;
					}
					entry.smaskbits = mbit;
				}
				else
					entry.smaskbits = 32;
			} else {
				entry.srcip = 0;
			}

			if (dip[0]) {
				if (!inet_aton(dip, (struct in_addr *)&entry.dstip)) {
					printf("Error! Source IP.");
					return error;
				}

				if (dmask[0]) {
					if (!inet_aton(dmask, (struct in_addr *)&mask)) {
						printf("Error! Source Netmask.");
						return error;
					}
					mask = htonl(mask);
					mbit=0; intVal=0;
					for (i=0; i<32; i++) {
						if (mask & 0x80000000) {
							if (intVal) {
								printf("Error! Source Netmask.");
								return error;
							}
							mbit++;
						}
						else
							intVal=1;
						mask <<= 1;
					}
					entry.dmaskbits = mbit;
				}
				else
					entry.dmaskbits = 32;
			} else {
				entry.dstip = 0;
			}

			return QosTcAdd(&entry);
		}
#endif

#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
		if(!strcmp(str1,"IP_QOS_TBL")){
		//IP_QOS_TBL
			unsigned char sip[16], smask[16], dip[16], dmask[16], pri[2], phyPort[2];
   			int prType, outIf;
   			int srcPort, dstPort;

#ifdef NEW_IP_QOS_SUPPORT
			unsigned char qosDscp[8], vlan1p[8];
#endif
            // jim 2007-07-02 fix save/restore bug
            unsigned char  m_ipprio[8];
            unsigned char  m_iptos[8];
            unsigned char  m_1p[8];
#ifdef QOS_DSCP
            unsigned char  m_dscp[8];
#endif

//#ifdef _CWMP_MIB_
            unsigned char  qos_enable[8];
#ifdef XML_TR069
           unsigned char  InstanceNum[8];
#endif


#ifdef QOS_SPEED_LIMIT_SUPPORT
	     char qosSpeedEnable[8]={0};
	     char qosSpeedRank[8]={0};
#endif

			strcpy(sip, "0.0.0.0");
			strcpy(smask, "0.0.0.0");
			strcpy(dip, "0.0.0.0");
			strcpy(dmask, "0.0.0.0");
			pri[0] = phyPort[0] = '\0';
			prType = outIf = srcPort = dstPort = 0;
#ifdef QOS_DSCP
			m_dscp[0] = '\0';
#endif

			//clear orginal record
			if(chain_updated[MIB_IP_QOS_TBL-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_IP_QOS_TBL);//clear chain record
				chain_updated[MIB_IP_QOS_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			isEmpty = 1;

			while(1){
				int flag = 0;

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_IP_QOS_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				if(!strcmp(str1, "SrcIP")){
					strcpy(sip, str2);
					flag = 1;
				}
				if(!strcmp(str1, "SrcMask")){
					strcpy(smask, str2);
					flag = 1;
				}
				if(!strcmp(str1, "SrcPort")){
					srcPort = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1, "DstIP")){
					strcpy(dip, str2);
					flag = 1;
				}
				if(!strcmp(str1, "DstMask")){
					strcpy(dmask, str2);
					flag = 1;
				}
				if(!strcmp(str1,"DstPort")){
					dstPort = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"Protocol")){
					prType = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"PhyPort")){
					strcpy(phyPort, str2);
					flag = 1;
				}
#ifdef NEW_IP_QOS_SUPPORT
				if(!strcmp(str1, "qosDscp")){
					strcpy(qosDscp, str2);
					flag = 1;
				}
				if(!strcmp(str1, "vlan1p")){
					strcpy(vlan1p, str2);
					flag = 1;
				}
#endif
				if(!strcmp(str1,"OutInf")){
					outIf = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"Prior")){
					strcpy(pri, str2);
					flag = 1;
				}
#ifdef QOS_DSCP
                if(!strcmp(str1, "m_dscp")){
                    strcpy(m_dscp, str2);
                    flag = 1;
                }
#endif
                if(!strcmp(str1, "m_ipprio")){
                    strcpy(m_ipprio, str2);
                    flag = 1;
                }
                if(!strcmp(str1, "m_iptos")){
                    strcpy(m_iptos, str2);
                    flag = 1;
                }
                if(!strcmp(str1, "m_1p")){
                    strcpy(m_1p, str2);
                    flag = 1;
                }

//#ifdef _CWMP_MIB_
                if(!strcmp(str1, "Enable")){
                    strcpy(qos_enable, str2);
                    flag = 1;
                }
#ifdef XML_TR069
                if(!strcmp(str1, "InstanceNum")){
                    strcpy(InstanceNum, str2);
                    flag = 1;
                }
#endif
//#endif

#ifdef QOS_SPEED_LIMIT_SUPPORT

	     if(!strcmp(str1,"limitSpeedEnabled")){
	     	strcpy(qosSpeedEnable,str2);
	     }
	     if(!strcmp(str1,"limitSpeedRank")){
	     	strcpy(qosSpeedRank,str2);
	     	flag = 1;
	     }
#endif
				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}

			}
//#ifdef _CWMP_MIB_
#ifdef XML_TR069
			return Qosadd(sip, smask, (unsigned short)srcPort, dip, dmask, (unsigned short)dstPort,
				(unsigned char)prType, phyPort,
#ifdef NEW_IP_QOS_SUPPORT
				qosDscp, vlan1p,
#endif
				(unsigned char)outIf, pri, m_ipprio, m_iptos,  m_1p, qos_enable, InstanceNum
#ifdef QOS_SPEED_LIMIT_SUPPORT
,qosSpeedEnable,qosSpeedRank
#endif
#ifdef QOS_DSCP
				,m_dscp
#endif
				);
#else
			return Qosadd(sip, smask, (unsigned short)srcPort, dip, dmask, (unsigned short)dstPort,
				//(unsigned char)prType, phyPort, (unsigned char)outIf, pri, m_ipprio, m_iptos, m_1p
				(unsigned char)prType, phyPort,
#ifdef NEW_IP_QOS_SUPPORT
				qosDscp, vlan1p,
#endif
				(unsigned char)outIf, pri, m_ipprio, m_iptos,  m_1p, qos_enable
#ifdef QOS_SPEED_LIMIT_SUPPORT
,qosSpeedEnable,qosSpeedRank
#endif
#ifdef QOS_DSCP
				,m_dscp
#endif
				);
#endif
		}
#ifdef QOS_SPEED_LIMIT_SUPPORT
if(!strcmp(str1,"MBQOS_SPEED_LIMIT")){
	MIB_CE_IP_QOS_SPEEDRANK_T entry;
	 char index[8],count[8],speed[8],prior[8];
	if(chain_updated[MIB_QOS_SPEED_LIMIT-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_QOS_SPEED_LIMIT);//clear chain record
				chain_updated[MIB_QOS_SPEED_LIMIT-CHAIN_ENTRY_TBL_ID]=1;
	}
			isEmpty = 1;

	while(1){
				int flag = 0;

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_QOS_SPEED_LIMIT);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				if(!strcmp(str1,"index")){
					strcpy(index,str2);
					flag = 1;
				}
				if(!strcmp(str1,"speed")){
					strcpy(speed,str2);
					flag = 1;
				}
				if(!strcmp(str1,"count")){
					strcpy(count,str2);
					flag = 1;
				}
				if(!strcmp(str1,"prior")){
					strcpy(prior,str2);
					flag = 1;
				}

				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}

	}

	return addQosLimitSpeed(index,count,speed,prior);
}
#endif
#endif	// of IP_QOS
//#endif	// of CONFIG_EXT_SWITCH

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
			isEmpty = 1;

			while(1){
				int flag = 0;

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_PORT_TRG_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1, "Name")){
					strcpy(name, str2);
					flag = 1;
				}
				if(!strcmp(str1, "IP")){
					strcpy(tip, str2);
					flag = 1;
				}
				if(!strcmp(str1, "TcpRange")){
					strcpy(tcprange, str2);
					flag = 1;
				}
				if(!strcmp(str1,"UdpRange")){
					strcpy(udprange, str2);
					flag = 1;
				}
				if(!strcmp(str1,"Enable")){
					enable = atoi(str2);
					flag = 1;
				}

				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
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
			isEmpty = 1;

			while(1){
				int flag = 0;

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_IP_FW_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1, "LocalIP")){
					strcpy(lip, str2);
					flag = 1;
				}
				if(!strcmp(str1, "RemoteIP")){
					strcpy(rip, str2);
					flag = 1;
				}

				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}
			return addipfw(lip, rip);
		}
#endif

#ifdef URL_BLOCKING_SUPPORT
		if(!strcmp(str1,"URL_FQDN_TBL")){
		//URL_FQDN_TBL
			unsigned char url[MAX_URL_LENGTH+2];

			url[0] = '\0';

			//clear orginal record
			if(chain_updated[MIB_URL_FQDN_TBL-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_URL_FQDN_TBL);//clear chain record
				chain_updated[MIB_URL_FQDN_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			isEmpty = 1;

			while(1){

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_URL_FQDN_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1, "URL"))
					strcpy(url, str2);
				else {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}
			return addurl(url);
		}
//star add
		if(!strcmp(str1,"KEYWD_FILTER_TBL")){
		//KEYWD_FILTER_TBL
			unsigned char keyword[MAX_KEYWD_LENGTH+2];

			keyword[0] = '\0';

			//clear orginal record
			if(chain_updated[MIB_KEYWD_FILTER_TBL-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_KEYWD_FILTER_TBL);//clear chain record
				chain_updated[MIB_KEYWD_FILTER_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			isEmpty = 1;

			while(1){

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_KEYWD_FILTER_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1, "KEYWD"))
					strcpy(keyword, str2);
				else {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}
			return addurlkeyword(keyword);
		}
#endif
#ifdef URL_ALLOWING_SUPPORT
		if(!strcmp(str1,"URL_ALLOW_FQDN_TBL")){
		//URL_ALLOW_FQDN_TBL
			unsigned char url[MAX_URL_LENGTH+2];

			url[0] = '\0';

			//clear orginal record
			if(chain_updated[MIB_URL_ALLOW_FQDN_TBL-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_URL_ALLOW_FQDN_TBL);//clear chain record
				chain_updated[MIB_URL_ALLOW_FQDN_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			isEmpty = 1;

			while(1){

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_URL_ALLOW_FQDN_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1, "URLALW"))
					strcpy(url, str2);
				else {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}
			return addurlalw(url);
		}

#endif


#ifdef DOMAIN_BLOCKING_SUPPORT
		if(!strcmp(str1,"DOMAIN_BLOCKING_TBL")){
		//DOMAIN_BLOCKING_TBL
			unsigned char domain[MAX_DOMAIN_LENGTH+2];

			domain[0] = '\0';

			//clear orginal record
			if(chain_updated[MIB_DOMAIN_BLOCKING_TBL-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_DOMAIN_BLOCKING_TBL);//clear chain record
				chain_updated[MIB_DOMAIN_BLOCKING_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			isEmpty = 1;

			while(1){

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_DOMAIN_BLOCKING_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1, "DOMAIN"))
					strcpy(domain, str2);
				else {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}
			return adddomain(domain);
		}
#endif

#if 0
		if(!strcmp(str1,"PPPOE_SESSION_TBL")){
		//PPPOE_SESSION_TBL
			unsigned char acmac[12];
			int ifno, vpi, vci, session;

			strcpy(acmac, "000000000000");
			ifno = vpi = vci = session = 0;

			//clear orginal record
			if(chain_updated[MIB_PPPOE_SESSION_TBL - CHAIN_ENTRY_TBL_ID] == 0){
				mib_chain_clear(MIB_PPPOE_SESSION_TBL);//clear chain record
				chain_updated[MIB_PPPOE_SESSION_TBL - CHAIN_ENTRY_TBL_ID] = 1;
			}
			isEmpty = 1;

			while(1){
				int flag = 0;

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_PPPOE_SESSION_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1, "InterfaceNo")){
					ifno = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"VPI")){
					vpi = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"VCI")){
					vci = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1, "ACMacAddr")){
					strcpy(acmac, str2);
					flag = 1;
				}
				if(!strcmp(str1, "SessionID")){
					session = atoi(str2);
					flag = 1;
				}

				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}
			return PPPoESessionAdd((unsigned char)ifno, (unsigned char)vpi, (unsigned short)vci, acmac, (unsigned short)session);
		}
#endif

//star add
		if(!strcmp(str1,"MAC_BASE_DHCP_TBL")){
		//MAC_BASE_DHCP_TBL
			MIB_CE_MAC_BASE_DHCP_T entry;
			int i;
			
			//clear orginal record
			if(chain_updated[MIB_MAC_BASE_DHCP_TBL - CHAIN_ENTRY_TBL_ID] == 0){
				mib_chain_clear(MIB_MAC_BASE_DHCP_TBL);//clear chain record
				chain_updated[MIB_MAC_BASE_DHCP_TBL - CHAIN_ENTRY_TBL_ID] = 1;
			}
			isEmpty = 1;

			while(1){
				int flag = 0;

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_MAC_BASE_DHCP_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1, "MacAddr")){
					//strcpy(entry.macAddr, str2);
					for (i=0; i<17; i++){
						if ((i+1)%3 != 0)
							str2[i-(i+1)/3] = str2[i];
					}
					str2[12] = '\0';
					if (strlen(str2)!=12  || !string_to_hex(str2, entry.macAddr_Dhcp, 12)) {						
						return error;
					}
					if (!isValidMacAddr(entry.macAddr_Dhcp)) {
						printf("Error! Invalid MAC address.");
						return error;
					}
		
					flag = 1;
				}
				if(!strcmp(str1,"IpAddr")){
					//strcpy(entry.ipAddr, str2);
					if ( !inet_aton(str2, (struct in_addr *)&entry.ipAddr_Dhcp ) || !isValidIpAddr(str2)) {
						printf("Error! Invalid Ip address.");
						return error;
					}
			
					flag = 1;
				}

				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}
			return addMacBaseTbl(entry);
		}

		if(!strcmp(str1,"RIP_TBL")){
		//RIP_TBL
			MIB_CE_RIP_T entry;

			//clear orginal record
			if(chain_updated[MIB_RIP_TBL - CHAIN_ENTRY_TBL_ID] == 0){
				mib_chain_clear(MIB_RIP_TBL);//clear chain record
				chain_updated[MIB_RIP_TBL - CHAIN_ENTRY_TBL_ID] = 1;
			}
			isEmpty = 1;

			while(1){
				int flag = 0;

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_RIP_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1, "IfIndex")){
					//entry.ifIndex = atoi(str2);
					entry.ifIndex = strtol(str2+2, &endptr, 16);
					flag = 1;
				}
				if(!strcmp(str1,"ReceiveMode")){
					entry.receiveMode = atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"SendMode")){
					entry.sendMode = atoi(str2);
					flag = 1;
				}

				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}
			return addRipTbl(entry);
		}
#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
		if(!strcmp(str1,"OSPF_TBL")){
			//OSPF_TBL
			MIB_CE_OSPF_T entry;
			char ipStr[16];
			char maskStr[16];
			int intVal;

			//clear orginal record
			if(chain_updated[MIB_OSPF_TBL - CHAIN_ENTRY_TBL_ID] == 0){
				mib_chain_clear(MIB_OSPF_TBL);//clear chain record
				chain_updated[MIB_OSPF_TBL - CHAIN_ENTRY_TBL_ID] = 1;
			}
			isEmpty = 1;

			while(1){
				int flag = 0;

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_OSPF_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1, "IP")) {
					strcpy(ipStr, str2);
					flag = 1;
				}
				if(!strcmp(str1,"Mask")) {
					strcpy(maskStr, str2);
					flag = 1;
				}

				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}
			inet_aton(ipStr, (struct in_addr *)&entry.ipAddr);
			inet_aton(maskStr, (struct in_addr *)&entry.netMask);

			intVal = mib_chain_add(MIB_OSPF_TBL, (void *)&entry);
			if (intVal == 0) {
				printf("Error! Add ospf chain record.");
				return error;
			}
			else if (intVal == -1)
				printf(strTableFull);
			return 1;
		}
#endif
#ifdef CONFIG_USER_DDNS
		if(!strcmp(str1,"MIB_DDNS_TBL")){
		//DDNS_TBL
			MIB_CE_DDNS_T entry;

			//clear orginal record
			if(chain_updated[MIB_DDNS_TBL - CHAIN_ENTRY_TBL_ID] == 0){
				mib_chain_clear(MIB_DDNS_TBL);//clear chain record
				chain_updated[MIB_DDNS_TBL - CHAIN_ENTRY_TBL_ID] = 1;
			}
			isEmpty = 1;

			while(1){
				int flag = 0;

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_DDNS_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1, "Provider")){
					strcpy(entry.provider, str2);
					flag = 1;
				}
				if(!strcmp(str1, "Hostname")){
					strcpy(entry.hostname, str2);
					flag = 1;
				}
#if 0
				if(!strcmp(str1, "Interface")){
					strcpy(entry.interface, str2);
					flag = 1;
				}
#endif
				// When any interface is up, we start DDNS with all Rules
				strcpy(entry.interface, "all");

				if(!strcmp(str1, "Username")){
					strcpy(entry.username, str2);
					flag = 1;
				}
				if(!strcmp(str1, "Password")){
					strcpy(entry.password, str2);
					flag = 1;
				}
#if 0
				if(!strcmp(str1, "Email")){
					strcpy(entry.email, str2);
					flag = 1;
				}
				if(!strcmp(str1, "Key")){
					strcpy(entry.key, str2);
					flag = 1;
				}
#endif
				if(!strcmp(str1, "Enabled")){
					entry.Enabled = atoi(str2);
					flag = 1;
				}

				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}
			return addDdnsTbl(entry);
		}
#endif
#ifdef ZTE_GENERAL_ROUTER_SC
		if(!strcmp(str1,"VIRTUAL_SVR_TBL")){
		//PPPOE_SESSION_TBL
			MIB_CE_VTL_SVR_T  entry;
			char svrip[16];
			char svrName[16];
			unsigned short wanStartPort=0;
			unsigned short wanEndPort=0;
			unsigned short svrStartPort=0;
			unsigned short svrEndPort=0;
			unsigned char protoType=0;
			memset(svrName, 0, sizeof(svrName));
			memset(&entry, 0, sizeof(entry));
			//clear orginal record
			if(chain_updated[MIB_VIRTUAL_SVR_TBL - CHAIN_ENTRY_TBL_ID] == 0){
				mib_chain_clear(MIB_VIRTUAL_SVR_TBL);//clear chain record
				chain_updated[MIB_VIRTUAL_SVR_TBL - CHAIN_ENTRY_TBL_ID] = 1;
			}
			isEmpty = 1;

			while(1){
				int flag = 0;

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_VIRTUAL_SVR_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1, "ServerIP")){
					strcpy(svrip, str2);
					flag = 1;
				}
				if(!strcmp(str1,"ServerName")){
					strcpy(svrName, str2);
					flag = 1;
				}
				if(!strcmp(str1,"WanStartPort")){
					wanStartPort= atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1, "WanEndPort")){
					wanEndPort=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"SvrStartPort")){
					svrStartPort= atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1, "SvrEndPort")){
					svrEndPort=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1, "ProtoType")){
					protoType= atoi(str2);
					flag = 1;
				}

				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}
			inet_aton(svrip, (struct in_addr *)&entry.svrIpAddr);
			strcpy(entry.svrName, svrName);
			entry.wanStartPort=wanStartPort;
			entry.wanEndPort=wanEndPort;
			entry.svrStartPort=svrStartPort;
			entry.svrEndPort=svrEndPort;
			entry.protoType=protoType;
			return addVirtualServerRule(&entry);
			//return PPPoESessionAdd((unsigned char)ifno, (unsigned char)vpi, (unsigned short)vci, acmac, (unsigned short)session);
		}

#endif		//ZTE_GENERAL_ROUTER_SC

#ifdef LAYER7_FILTER_SUPPORT
		if(!strcmp(str1,"LAYER7_FILTER_TBL")){
		//LAYER7_TBL
			LAYER7_FILTER_T entry;
			int intVal;

			//clear orginal record
			if(chain_updated[MIB_LAYER7_FILTER_TBL - CHAIN_ENTRY_TBL_ID] == 0){
				mib_chain_clear(MIB_LAYER7_FILTER_TBL);//clear chain record
				chain_updated[MIB_LAYER7_FILTER_TBL - CHAIN_ENTRY_TBL_ID] = 1;
			}
			isEmpty = 1;

			while(1){

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_LAYER7_FILTER_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				if(!strcmp(str1, "Appname")) {
					if(strlen(str2)>=MAX_APP_NAME){
						printf("Error! The appname is too long.");
						return error;
					}
					strcpy(entry.appname, str2);
				}
				else {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}

			intVal = mib_chain_add(MIB_LAYER7_FILTER_TBL, (void *)&entry);
			if (intVal == 0) {
				printf("Error! Add layer7_filter chain record.");
				return error;
			}
			else if (intVal == -1)
				printf(strTableFull);
			return 1;
		}
#endif

#ifdef ACCOUNT_CONFIG
		if(!strcmp(str1,"ACCOUNT_CONFIG_TBL")){
		//ACCOUNT_CONFIG_TBL
			unsigned char username[MAX_NAME_LEN], passwrd[MAX_NAME_LEN];
			int priv;

			username[0] = '\0';
			passwrd[0] = '\0';
			priv = 0;

			//clear orginal record
			if(chain_updated[MIB_ACCOUNT_CONFIG_TBL - CHAIN_ENTRY_TBL_ID] == 0){
				mib_chain_clear(MIB_ACCOUNT_CONFIG_TBL);//clear chain record
				chain_updated[MIB_ACCOUNT_CONFIG_TBL - CHAIN_ENTRY_TBL_ID] = 1;
			}
			isEmpty = 1;

			while(1){
				int flag = 0;

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_ACCOUNT_CONFIG_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);
				if (!strcmp(str1, "UserName")){
					strcpy(username, str2);
					flag = 1;
				}
				if (!strcmp(str1, "Password")){
					strcpy(passwrd, str2);
					flag = 1;
				}
				if (!strcmp(str1, "Privilege")){
					priv = atoi(str2);
					flag = 1;
				}

				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}
			return addAccConfig(username, passwrd, (unsigned char)priv);
		}
#endif

#if defined(AUTO_PVC_SEARCH_PURE_OAMPING) || defined(AUTO_PVC_SEARCH_AUTOHUNT)
		if(!strcmp(str1,"AUTO_PVC_SEARCH_TBL")){
		//AUTO_PVC_SEARCH_TBL
			unsigned short vpi=0;
			unsigned int vci=0;

			//clear orginal record
			if(chain_updated[MIB_AUTO_PVC_SEARCH_TBL-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_AUTO_PVC_SEARCH_TBL);//clear chain record
				chain_updated[MIB_AUTO_PVC_SEARCH_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			isEmpty = 1;

			while(1){
				int flag = 0;

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_AUTO_PVC_SEARCH_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);

				if(!strcmp(str1,"VPI")){
					vpi=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"VCI")){
					vci=atoi(str2);
					flag = 1;
				}

				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}
			return addAuotPVCTBL(vpi, vci);
		}
#endif

#ifdef PORT_FORWARD_ADVANCE
		if(!strcmp(str1,"PORT_FW_ADVANCE_TBL")){
		//PORT_FW_ADVANCE_TBL
			char ip[16];
			unsigned char gategory=0;
			unsigned int rule=0;
			int OutInf=0;

			//clear orginal record
			if(chain_updated[MIB_PFW_ADVANCE_TBL-CHAIN_ENTRY_TBL_ID]==0){
				mib_chain_clear(MIB_PFW_ADVANCE_TBL);//clear chain record
				chain_updated[MIB_PFW_ADVANCE_TBL-CHAIN_ENTRY_TBL_ID]=1;
			}
			isEmpty = 1;

			while(1){
				int flag = 0;

				ret = parseChainEntryLine(fp, &str1, &str2);
				if (ret == CHAIN_ENTRY_ERROR)
					return error;
				if (ret == CHAIN_ENTRY_END) {
					if (isEmpty) {
						mib_chain_clear(MIB_PFW_ADVANCE_TBL);
						return 1;
					}
					else
						break;
				}
				// This entry is not empty
				isEmpty = 0;
				//printf("name:%s	value:%s\n",str1,str2);

				if(!strcmp(str1,"IP")){
					strcpy(ip,str2);
					flag = 1;
				}
				if(!strcmp(str1,"OutInf")){
					// review conf
					//OutInf=atoi(str2);
					// The ifIndex is 0x??. Mason Yu
					OutInf = strtol(str2+2, &endptr, 16);
					flag = 1;
				}
				if(!strcmp(str1,"Gategory")){
					gategory=atoi(str2);
					flag = 1;
				}
				if(!strcmp(str1,"Rule")){
					rule=atoi(str2);
					flag = 1;
				}

				if ( flag == 0 ) {
					printf("The Name %s is invalid for Configuration\n", str1);
					return error;
				}
			}
			return addPFWAdvance(ip, OutInf, gategory, rule);
		}
#endif

	}

	i = 0;
// Apply Star Zhang's fast load
	for(i=0;i<infototal;i++){
		if(!strcmp(((mib_table_entry_T*)(infotmp+i))->name,str1)){
			memcpy(&info,(mib_table_entry_T*)(infotmp+i),sizeof(mib_table_entry_T));
			break;
		}
	}
	if(i>infototal)
		return error;
// The end of fast load

	if (info.id != -1) {
		unsigned char buffer[100];
		struct in_addr ipAddr;
		unsigned char vChar;
		unsigned short vShort;
		unsigned int vUInt;
		int vInt;

		switch (info.type) {
			case IA_T:
				ipAddr.s_addr = inet_addr(str2);
				if (!mib_set(info.id, (void *)&ipAddr)) {
					printf("Set MIB[%s] error!\n", str1);
					return error;
				}
				break;
			case BYTE_T:
				if ( (strcmp("IGMP_PROXY_ITF", info.name) == 0) ||
				     (strcmp("IPPT_ITF", info.name) == 0)       ||
				     (strcmp("UPNP_EXT_ITF", info.name) == 0)     ) {
					// ifIndex = 0x??.
					//sscanf(str2+2,"%x",&vUInt);
					vUInt = strtol(str2+2, &endptr, 16);
				} else {
					sscanf(str2,"%u",&vUInt);
				}
				vChar = (unsigned char) vUInt;
				if (!mib_set(info.id, (void *)&vChar)) {
					printf("Set MIB[%s] error!\n", str1);
					return error;
				}
				break;
			case WORD_T:
				sscanf(str2,"%u",&vUInt);
				vShort = (unsigned short) vUInt;
				if (!mib_set(info.id, (void *)&vShort)) {
					printf("Set MIB[%s] error!\n", str1);
					return error;
				}
				break;
			case DWORD_T:
				sscanf(str2,"%u",&vUInt);
				if (!mib_set(info.id, (void *)&vUInt)) {
					printf("Set MIB[%s] error!\n", str1);
					return error;
				}
				break;
			case INTEGER_T:
				sscanf(str2,"%d",&vInt);
				if (!mib_set(info.id, (void *)&vInt)) {
					printf("Set MIB[%s] error!\n", str1);
					return error;
				}
				break;
			case BYTE5_T:
				string_to_hex(str2, buffer, 10);
				if (!mib_set(info.id, (void *)&buffer)) {
					printf("Set MIB[%s] error!\n", str1);
					return error;
				}
				break;
			case BYTE6_T:
				string_to_hex(str2, buffer, 12);
				if (!mib_set(info.id, (void *)&buffer)) {
					printf("Set MIB[%s] error!\n", str1);
					return error;
				}
				break;
			case BYTE13_T:
				string_to_hex(str2, buffer, 26);
				if (!mib_set(info.id, (void *)&buffer)) {
					printf("Set MIB[%s] error!\n", str1);
					return error;
				}
				break;
			case STRING_T:
				if (!mib_set(info.id, (void *)str2)) {
					printf("Set MIB[%s] error!\n", str1);
					return error;
				}
				break;
			case BYTE_ARRAY_T:
				if (info.id != MIB_ADSL_TONE) {
					string_to_hex(str2, buffer, 28);
					if (!mib_set(info.id, (void *)&buffer)) {
						printf("Set MIB[%s] error!\n", str1);
						return error;
					}
				}
				break;
		}
	}

#if 0
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
		//printf("str2=%s\n", str2);
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
		if(!strcmp(str2,""))
			return 1;
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
		unsigned short uFRate = atoi(str2);
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

#ifdef AUTO_PROVISIONING
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
#endif

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
#endif

	return 1;
}


void update_config(void)
{
	// Kaohj
	#if 0
	//close all devices
	sleep(1);
	sync();
	va_cmd(IFCONFIG, 2, 1, (char *)ELANIF, "down");
	va_cmd(IFCONFIG, 2, 1, (char *)WLANIF, "down");
	va_cmd("/bin/sarctl",1,1,"disable");
	#endif
	printf("Updating user specific configuration and restart the system......\n\n");
	//update flash
	if (mib_update(CURRENT_SETTING, CONFIG_MIB_ALL) == 0) {
		printf("Warning : Commit cs fail !\n");
	}
	if (mib_update(HW_SETTING, CONFIG_MIB_ALL) == 0) {
		printf("Warning : Commit hs fail !\n");
	}
	// Kaohj
	#if 0
	itfcfg("eth0", 1);
	itfcfg("sar", 1);
	itfcfg("wlan0", 1);
	#endif
	//reboot the system
	va_cmd("/bin/reboot", 0, 1);
}

int main(int argc, char **argv)
{
	FILE *fpin, *fp;
	char LINE[256], i, buf[10];
	char url[256], output[256], rc4_key[256];
	int len, key_len;
	long filelen;

	for(i=0;i<chain_record_number;i++){
		chain_updated[i]=0;
	}

	#if 0
	//init. mib
	if ( mib_init() == 0 ) {
		printf("[Upload Config] Initialize MIB failed!\n");
		return error;
	}
	#endif

	printf("Get user specific configuration file......\n\n");
	if (!(fpin = fopen("/tmp/config.xml", "r")))
		printf("User configuration file not exist\n");
	else {
		fseek(fpin, 0, SEEK_END);
		filelen = ftell(fpin);
		fseek(fpin, 0, SEEK_SET);
		if (filelen > 0) {
			char str1[10], *str2, *tmp, str3[64], str4[64];
			fgets(LINE, sizeof(LINE), fpin);
//			if(strcmp(LINE,CONFIG_HEADER)){
			sprintf(str3, "%s\n", CONFIG_HEADER);
			// Support Dos Format
			sprintf(str4, "%s\r\n", CONFIG_HEADER);
			if(strcmp(LINE, str3) && strcmp(LINE, str4)){
				printf("Invalid config file!\n");
				fclose(fpin);
				exit(1);
			}
//			fgets(LINE, sizeof(LINE), fpin);	// marked by Jenny

			// Apply Star Zhang's fast load
			int i;

			infototal=mib_info_total();
			printf("\ninfototal=%d\n",infototal);
			infotmp=(mib_table_entry_T *)malloc(sizeof(mib_table_entry_T)*infototal);
			for(i=0;i<infototal;i++){
				if(!mib_info_index(i,infotmp+i))
					break;
//				printf("\nid=%d,name=%s\n",((mib_table_entry_T *)(infotmp+i))->id,((mib_table_entry_T *)(infotmp+i))->name);
			}
			if(i<infototal){
				free(infotmp);
				printf("get mib info total entry error!\n");
				return error;
			}

			while(!feof(fpin)) {
				LINE[0] = 0;
				fgets(LINE, sizeof(LINE), fpin);//get one line from the file
				//test
				//printf("%s\n", LINE);

//				if(!strcmp(LINE,CONFIG_TRAILER))
				str3[0] = '\0';
				sprintf(str3, "%s\n", CONFIG_TRAILER);

				// Support Dos Format
				str4[0] = '\0';
				sprintf(str4, "%s\r\n", CONFIG_TRAILER);
				if( (!strcmp(LINE, str3)) || !strcmp(LINE, str4))
					break;
				// Support Dos Format
				if((LINE[0]!='#') && (strlen(LINE)!=0) && !(LINE[0]==13 && LINE[1]==10) && !(LINE[0]==10)) {//this line is not comment
					if(update_setting(LINE, fpin) < 0) {
						fclose(fpin);
						printf("update setting error!\n");

						free(infotmp);
						return error;
					}
				}
			}
			fclose(fpin);
//			update_config();
		}
		else
			printf("User configuration file not exist\n");
		fclose(fpin);
   		printf("Restore settings from config file successful! \n");
	}

	free(infotmp);
	return 0;
}
