#include<stdio.h>
#include<string.h>
#include <arpa/inet.h>
#include <net/route.h>
#include <string.h>
#include "mib.h"
#include "mibtbl.h"
#include "utility.h"

#define error -1
static const char *emptystr = "";
char strbuf[256];//100=>256,jiunming, the max length of tr069 = 256

char *getMibInfo(int id)
{
	if((id == MIB_USER_NAME) || (id == MIB_USER_PASSWORD) || (id == MIB_SUPER_NAME) || (id == MIB_SUPER_PASSWORD) ||
#ifdef WLAN_SUPPORT
		(id == MIB_WLAN_WPA_PSK) || (id == MIB_WLAN_RS_PASSWORD) || (id == MIB_WLAN_ACCOUNT_RS_PASSWORD) || (id == MIB_WLAN_ACCOUNT_RS_IP)||
#endif
		(id == MIB_SUSER_NAME) || (id == MIB_SUSER_PASSWORD) ){
		if (!mib_get(id, (void *)strbuf)) {
			printf("get mib string error!");
			return (char *)emptystr;
		}
		return strbuf;
	}
#ifdef WLAN_SUPPORT
	if((id == MIB_WLAN_ACCOUNT_RS_IP)){
		if(!mib_get( id, (void *)strbuf)){
			printf("get mib IA error!");
			return (char *)emptystr;
		}
		sprintf(strbuf, "%s", inet_ntoa(*((struct in_addr *)strbuf)));
		return strbuf;
	}
#endif
	if (getMIB2Str(id, strbuf) == 0){
		//printf("%s\n",strbuf);
		return strbuf;
	}

	return (char *)emptystr;
}

unsigned int getMibValue(int id){
	unsigned char charVar;
	unsigned short shortVar;
	unsigned int intVar;
	mib_table_entry_T info;
	TYPE_T	dtype;
	int i;

	if (!mib_info_id(id, &info))
		return error;
	dtype = info.type;
	if(dtype == BYTE_T)	{
		if (!mib_get(id, (void *)&charVar)) {
			printf("Get MIB[%s] error!\n",info.name);
			return error;
		}
		return charVar;
	}
	if(dtype == WORD_T)	{
		if (!mib_get(id, (void *)&shortVar)) {
			printf("Get MIB[%s] error!\n",info.name);
			return error;
		}
		return shortVar;
	}
	if(dtype == INTEGER_T)	{
		if (!mib_get(id, (void *)&intVar)) {
			printf("Get MIB[%s] error!\n",info.name);
			return error;
		}
		return intVar;
	}
}

void getMask(unsigned char *mask, unsigned char bit){
	unsigned long long tmp=0xffffffff;   // netmask bug

	mask[0]=mask[1]=mask[2]=mask[3]=0xff;

	tmp = (tmp << (32-bit));
	mask[0] &= (tmp>>24);
	mask[1] &= (tmp>>16);
	mask[2] &= (tmp>>8);
	mask[3] &= (tmp);
	//printf("%x\n",tmp);
}
unsigned char IFsel[8];
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

//#ifdef _CWMP_MIB_
/* -1:error
    0:all interface
    others: specific interface (start from 1,2...to 8)
 */
int getWanIndex( unsigned char id  )
{
	int total=0, i=0;

	if(id==0xff) return 0;

	total = showInf();
	if(total<=0) return -1;


	for( i=0;i<total;i++ )
		if( IFsel[i]==id ) break;

	if(i==total) return -1;


	return i+1;
}
//#endif

int create_config_file()
{
	FILE *fp;
	unsigned char charVar;
	unsigned short shortVar;
	unsigned int intVar;
	unsigned long longVar;
	int i,idx;
	mib_table_entry_T info;
	mib_chain_record_table_entry_T chainInfo;
	unsigned char value[64];

	fp=fopen("/tmp/config.xml","w");

//	fprintf(fp,"<Config_Information_File_8671>\n");
//	fprintf(fp,"%s\n", CONFIG_HEADER);
	//MIB record
	i = 0;
	while (1) {
		if (!mib_info_index(i++, &info))
			return error;

		if (info.id == 0)
			break;

		if((info.type == IA_T) || (info.type == STRING_T)){
			fprintf(fp,"<Value Name=\"%s\" Value=\"%s\"/>\n",info.name,getMibInfo(info.id));
		}
		if(info.type == BYTE_T)	{
			if (!mib_get(info.id, (void *)&charVar)) {
				printf("Get MIB[%s] error!\n",info.name);
				return error;
			}
			if ( (strcmp("IGMP_PROXY_ITF", info.name) == 0) ||
			     (strcmp("IPPT_ITF", info.name) == 0)       ||
			     (strcmp("UPNP_EXT_ITF", info.name) == 0)     ) {
			     	// ifIndex = 0x??.
				fprintf(fp,"<Value Name=\"%s\" Value=\"0x%x\"/>\n",info.name,charVar);
			} else
				fprintf(fp,"<Value Name=\"%s\" Value=\"%d\"/>\n",info.name,charVar);
		}
		if(info.type == WORD_T)	{
			if (!mib_get(info.id, (void *)&shortVar)) {
				printf("Get MIB[%s] error!\n",info.name);
				return error;
			}
			fprintf(fp,"<Value Name=\"%s\" Value=\"%d\"/>\n",info.name,shortVar);
		}
		if(info.type == INTEGER_T)	{
			if (!mib_get(info.id, (void *)&intVar)) {
				printf("Get MIB[%s] error!\n",info.name);
				return error;
			}
			fprintf(fp,"<Value Name=\"%s\" Value=\"%d\"/>\n",info.name,intVar);
		}
		if(info.type == DWORD_T)	{
			if (!mib_get(info.id, (void *)&longVar)) {
				printf("Get MIB[%s] error!\n",info.name);
				return error;
			}
			fprintf(fp,"<Value Name=\"%s\" Value=\"%d\"/>\n",info.name,longVar);
		}
		if(info.type == BYTE5_T)	{
			if (!mib_get(info.id, (void *)value)) {
				printf("Get MIB[%s] error!\n",info.name);
				return error;
			}
			fprintf(fp,"<Value Name=\"%s\" Value=\"%02x%02x%02x%02x%02x\"/>\n",info.name, value[0],
				value[1], value[2], value[3], value[4]);
		}
		if(info.type == BYTE13_T)	{
			if (!mib_get(info.id, (void *)value)) {
				printf("Get MIB[%s] error!\n",info.name);
				return error;
			}
			fprintf(fp,"<Value Name=\"%s\" Value=\"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\"/>\n",info.name, value[0],
				value[1], value[2], value[3], value[4],value[5], value[6], value[7], value[8], value[9],value[10],value[11], value[12]);
		}
		if(info.type == BYTE6_T)	{
			if (!mib_get(info.id, (void *)value)) {
				printf("Get MIB[%s] error!\n",info.name);
				return error;
			}
			fprintf(fp,"<Value Name=\"%s\" Value=\"%02x%02x%02x%02x%02x%02x\"/>\n",info.name, value[0],
				value[1], value[2], value[3], value[4], value[5]);
		}

		if(info.type == BYTE_ARRAY_T)	{
			if (!mib_get(info.id, (void *)value)) {
				printf("Get MIB[%s] error!\n",info.name);
				return error;
			}

#ifdef WLAN_SUPPORT
#ifdef WLAN_8185AG
			if((info.id == MIB_HW_TX_POWER_CCK) || (info.id == MIB_HW_TX_POWER_OFDM))
#else
			if(info.id == MIB_HW_TX_POWER)
#endif
			{
				fprintf(fp,"<Value Name=\"%s\" Value=\"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\"/>\n",info.name, value[0],
					value[1], value[2], value[3], value[4],value[5], value[6], value[7], value[8], value[9],value[10],value[11], value[12], value[13]);
			}
#endif
			/*if(info.id == MIB_ADSL_TONE){
				fprintf(fp,"<Value Name=\"%s\" Value=\"",info.name);
				for(idx=0;idx<64;idx++){
					fprintf(fp,"%02x",value[i]);
				}
				fprintf(fp,"\"/>\n");
			}*/
		}
	}

	//MIB chain record
	i = 0;
	while (1) {
		unsigned int entryNum, index;

		if (!mib_chain_info_index(i++, &chainInfo))
			return error;

		if (chainInfo.id == 0)
			break;

		if(chainInfo.id == MIB_IP_PORT_FILTER_TBL){
			MIB_CE_IP_PORT_FILTER_T Entry;
			entryNum = mib_chain_total(MIB_IP_PORT_FILTER_TBL);
			if(entryNum>0){
				for(index=0;index<entryNum;index++){
					unsigned char mask[IP_ADDR_LEN],protocol,dir,filterMode;
					if (!mib_chain_get(MIB_IP_PORT_FILTER_TBL, index, (void *)&Entry))
						continue;
					fprintf(fp,"<chain chainName=\"IP_PORT_FILTER_TBL\">\n");
					//action
					if (Entry.action == 0)
						filterMode=1;
					else if (Entry.action == 1)
						filterMode=2;
					fprintf(fp,"<Value Name=\"FilterMode\" Value=\"%d\"/>\n",filterMode);
					fprintf(fp,"<Value Name=\"SrcIP\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)Entry.srcIp)));
					fprintf(fp,"<Value Name=\"DstIP\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)Entry.dstIp)));
					getMask(mask,Entry.smaskbit);
					fprintf(fp,"<Value Name=\"SrcMask\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)mask)));
					getMask(mask,Entry.dmaskbit);
					fprintf(fp,"<Value Name=\"DstMask\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)mask)));

					fprintf(fp,"<Value Name=\"SrcPortStart\" Value=\"%d\"/>\n",Entry.srcPortFrom);
					fprintf(fp,"<Value Name=\"SrcPortEnd\" Value=\"%d\"/>\n",Entry.srcPortTo);
					fprintf(fp,"<Value Name=\"DstPortStart\" Value=\"%d\"/>\n",Entry.dstPortFrom);
					fprintf(fp,"<Value Name=\"DstPortEnd\" Value=\"%d\"/>\n",Entry.dstPortTo);

					// Direction
					if ( Entry.dir == DIR_OUT ) {
						dir=1;
					}
					else if ( Entry.dir == DIR_IN  ) {
						dir=2;
					}
					fprintf(fp,"<Value Name=\"Direction\" Value=\"%d\"/>\n",dir);

					// Protocol Type
					if ( Entry.protoType == PROTO_ICMP) {
						protocol=1;
					}
					else if (Entry.protoType == PROTO_TCP) {
						protocol=2;
					}
					else if ( Entry.protoType == PROTO_UDP ) {
						protocol=3;
					}
					fprintf(fp,"<Value Name=\"protocol\" Value=\"%d\"/>\n",protocol);

					fprintf(fp,"</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"IP_PORT_FILTER_TBL\">\n");
				fprintf(fp,"</chain>\n");
			}


		}

/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
		if(chainInfo.id == MIB_DHCP_SERVER_OPTION_TBL){
			MIB_CE_DHCP_OPTION_T Entry;
			int i;
			char tmpbuf[10]={0};
			char buf[128]={0};
			entryNum = mib_chain_total(MIB_DHCP_SERVER_OPTION_TBL);
			if(entryNum>0){
				for(index=0;index<entryNum;index++){
					if (!mib_chain_get(MIB_DHCP_SERVER_OPTION_TBL, index, (void *)&Entry))
						continue;
					fprintf(fp,"<chain chainName=\"DHCP_SERVER_OPTION_TBL\">\n");
					fprintf(fp,"<Value Name=\"ifIndex\" Value=\"%d\"/>\n",Entry.ifIndex);
					fprintf(fp,"<Value Name=\"Enable\" Value=\"%d\"/>\n",Entry.enable);
					fprintf(fp,"<Value Name=\"Order\" Value=\"%d\"/>\n",Entry.order);
 					fprintf(fp,"<Value Name=\"Tag\" Value=\"%d\"/>\n", Entry.tag);
 					fprintf(fp,"<Value Name=\"Len\" Value=\"%d\"/>\n", Entry.len);
					for(i=0;i<Entry.len;i++){
						sprintf(tmpbuf,"%02x",Entry.value[i]);
						strcat(buf,tmpbuf);
					}
					fprintf(fp,"<Value Name=\"Value\" Value=\"%s\"/>\n", buf);	
					memset(buf,0,128);
					fprintf(fp,"<Value Name=\"UsedFor\" Value=\"%d\"/>\n",Entry.usedFor);
					fprintf(fp,"<Value Name=\"DhcpOptInstNum\" Value=\"%d\"/>\n",Entry.dhcpOptInstNum);
					fprintf(fp,"<Value Name=\"DhcpConSPInstNum\" Value=\"%d\"/>\n",Entry.dhcpConSPInstNum);
					fprintf(fp,"</chain>\n");	
				}	
			}		
		}

		if (chainInfo.id == MIB_DHCP_CLIENT_OPTION_TBL){
			MIB_CE_DHCP_OPTION_T Entry;
			int i;
			char tmpbuf[10]={0};
			char buf[128]={0};
			entryNum = mib_chain_total(MIB_DHCP_CLIENT_OPTION_TBL);
			if(entryNum>0){
				for(index=0;index<entryNum;index++){
					if (!mib_chain_get(MIB_DHCP_CLIENT_OPTION_TBL, index, (void *)&Entry))
						continue;
					fprintf(fp,"<chain chainName=\"DHCP_CLIENT_OPTION_TBL\">\n");
					fprintf(fp,"<Value Name=\"ifIndex\" Value=\"%d\"/>\n",Entry.ifIndex);
					fprintf(fp,"<Value Name=\"Enable\" Value=\"%d\"/>\n",Entry.enable);
					fprintf(fp,"<Value Name=\"Order\" Value=\"%d\"/>\n",Entry.order);
 					fprintf(fp,"<Value Name=\"Tag\" Value=\"%d\"/>\n", Entry.tag);
 					fprintf(fp,"<Value Name=\"Len\" Value=\"%d\"/>\n", Entry.len);
					for(i=0;i<Entry.len;i++){
						sprintf(tmpbuf,"%02x",Entry.value[i]);
						strcat(buf,tmpbuf);
					}
					fprintf(fp,"<Value Name=\"Value\" Value=\"%s\"/>\n", buf);
					memset(buf,0,128);
					fprintf(fp,"<Value Name=\"UsedFor\" Value=\"%d\"/>\n",Entry.usedFor);
					fprintf(fp,"<Value Name=\"DhcpOptInstNum\" Value=\"%d\"/>\n",Entry.dhcpOptInstNum);
					fprintf(fp,"<Value Name=\"DhcpConSPInstNum\" Value=\"%d\"/>\n",Entry.dhcpConSPInstNum);
					fprintf(fp,"</chain>\n");	
				}	
			}		
		}
		if (chainInfo.id == MIB_DHCPS_SERVING_POOL_TBL){
			DHCPS_SERVING_POOL_T Entry;
			char macbuf[20];
			entryNum = mib_chain_total(MIB_DHCPS_SERVING_POOL_TBL);
			if (entryNum>0){
				for (index=0;index<entryNum;index++){
					if (!mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL, index, (void *)&Entry))
						continue;
					fprintf(fp,"<chain chainName=\"DHCP_SERVER_POOL_TBL\">\n");
					fprintf(fp,"<Value Name=\"enable\" Value=\"%d\"/>\n",Entry.enable);
					fprintf(fp,"<Value Name=\"poolorder\" Value=\"%d\"/>\n",Entry.poolorder);
					fprintf(fp,"<Value Name=\"poolname\" Value=\"%s\"/>\n",Entry.poolname);
 					fprintf(fp,"<Value Name=\"sourceinterface\" Value=\"%d\"/>\n", Entry.sourceinterface);
 					fprintf(fp,"<Value Name=\"vendorclass\" Value=\"%s\"/>\n", Entry.vendorclass);
					fprintf(fp,"<Value Name=\"vendorclassflag\" Value=\"%d\"/>\n", Entry.vendorclassflag);	
					fprintf(fp,"<Value Name=\"clientid\" Value=\"%s\"/>\n",Entry.clientid);
					fprintf(fp,"<Value Name=\"clientidflag\" Value=\"%d\"/>\n",Entry.clientidflag);
					fprintf(fp,"<Value Name=\"userclass\" Value=\"%s\"/>\n",Entry.userclass);
					fprintf(fp,"<Value Name=\"userclassflag\" Value=\"%d\"/>\n",Entry.userclassflag);
					sprintf(macbuf,"%02x%02x%02x%02x%02x%02x",Entry.chaddr[0],Entry.chaddr[1],Entry.chaddr[2],Entry.chaddr[3],Entry.chaddr[4],Entry.chaddr[5]);
					fprintf(fp,"<Value Name=\"chaddr\" Value=\"%s\"/>\n",macbuf);
					sprintf(macbuf,"%02x%02x%02x%02x%02x%02x",Entry.chaddrmask[0],Entry.chaddrmask[1],Entry.chaddrmask[2],Entry.chaddrmask[3],Entry.chaddrmask[4],Entry.chaddrmask[5]);
					fprintf(fp,"<Value Name=\"chaddrmask\" Value=\"%s\"/>\n",macbuf);
					fprintf(fp,"<Value Name=\"chaddrflag\" Value=\"%d\"/>\n",Entry.chaddrflag);
					fprintf(fp,"<Value Name=\"localserved\" Value=\"%d\"/>\n",Entry.localserved);
					fprintf(fp,"<Value Name=\"startaddr\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)Entry.startaddr)));
					fprintf(fp,"<Value Name=\"endaddr\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)Entry.endaddr)));
					fprintf(fp,"<Value Name=\"subnetmask\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)Entry.subnetmask)));
					fprintf(fp,"<Value Name=\"iprouter\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)Entry.iprouter)));
					fprintf(fp,"<Value Name=\"dnsserver1\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)Entry.dnsserver1)));
					fprintf(fp,"<Value Name=\"dnsserver2\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)Entry.dnsserver2)));
					fprintf(fp,"<Value Name=\"dnsserver3\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)Entry.dnsserver3)));
					fprintf(fp,"<Value Name=\"domainname\" Value=\"%s\"/>\n",Entry.domainname);
					fprintf(fp,"<Value Name=\"leasetime\" Value=\"%d\"/>\n",Entry.leasetime);
					fprintf(fp,"<Value Name=\"dhcprelayip\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)Entry.dhcprelayip)));
					fprintf(fp,"<Value Name=\"dnsservermode\" Value=\"%d\"/>\n",Entry.dnsservermode);
#ifdef _CWMP_MIB_
					fprintf(fp,"<Value Name=\"InstanceNum\" Value=\"%d\"/>\n",Entry.InstanceNum);
#endif
					fprintf(fp,"</chain>\n");	
				}
			}
		}
#endif
/*ping_zhang:20080919 END*/

		
#ifdef MAC_FILTER
		if(chainInfo.id == MIB_MAC_FILTER_TBL){
			MIB_CE_MAC_FILTER_T Entry;
			entryNum = mib_chain_total(MIB_MAC_FILTER_TBL);
			if(entryNum>0){
				for(index=0;index<entryNum;index++){
					unsigned char mask[IP_ADDR_LEN],dir,filterMode;
					if (!mib_chain_get(MIB_MAC_FILTER_TBL, index, (void *)&Entry))
						continue;
					fprintf(fp,"<chain chainName=\"MAC_FILTER_TBL\">\n");
					//action
					if (Entry.action == 0)
						filterMode=1;
					else if (Entry.action == 1)
						filterMode=2;
					fprintf(fp,"<Value Name=\"FilterMode\" Value=\"%d\"/>\n",filterMode);

					fprintf(fp,"<Value Name=\"SrcMac\" Value=\"%02x%02x%02x%02x%02x%02x\"/>\n",
						Entry.srcMac[0],Entry.srcMac[1],Entry.srcMac[2],Entry.srcMac[3],Entry.srcMac[4],Entry.srcMac[5]);
					fprintf(fp,"<Value Name=\"DstMac\" Value=\"%02x%02x%02x%02x%02x%02x\"/>\n",
						Entry.dstMac[0],Entry.dstMac[1],Entry.dstMac[2],Entry.dstMac[3],Entry.dstMac[4],Entry.dstMac[5]);

					// Direction
					if ( Entry.dir == DIR_OUT ) {
						dir=1;
					}
					else if ( Entry.dir == DIR_IN  ) {
						dir=2;
					}
					fprintf(fp,"<Value Name=\"Direction\" Value=\"%d\"/>\n",dir);

					fprintf(fp,"</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"MAC_FILTER_TBL\">\n");
				fprintf(fp,"</chain>\n");
			}
		}
#endif // of MAC_FILTER

		if(chainInfo.id == MIB_PORT_FW_TBL){
			MIB_CE_PORT_FW_T Entry;
			entryNum = mib_chain_total(MIB_PORT_FW_TBL);
			if(entryNum>0){
				for(index=0;index<entryNum;index++){
					unsigned char mask[IP_ADDR_LEN],dir,protocol;
					if (!mib_chain_get(MIB_PORT_FW_TBL, index, (void *)&Entry))
						continue;

					fprintf(fp,"<chain chainName=\"PORT_FW_TBL\">\n");
					fprintf(fp,"<Value Name=\"IP\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)Entry.ipAddr)));
					fprintf(fp,"<Value Name=\"PortStart\" Value=\"%d\"/>\n",Entry.fromPort);
					fprintf(fp,"<Value Name=\"PortEnd\" Value=\"%d\"/>\n",Entry.toPort);
					fprintf(fp,"<Value Name=\"Comment\" Value=\"%s\"/>\n",Entry.comment);
					// Protocol Type
					if ( Entry.protoType == PROTO_UDPTCP) {
						protocol=3;
					}
					else if (Entry.protoType == PROTO_TCP) {
						protocol=1;
					}
					else if ( Entry.protoType == PROTO_UDP ) {
						protocol=2;
					}
					fprintf(fp,"<Value Name=\"Protocol\" Value=\"%d\"/>\n",protocol);

//#ifdef _CWMP_MIB_
					//fprintf(fp,"<Value Name=\"OutInf\" Value=\"%d\"/>\n",getWanIndex(Entry.ifIndex) );
					fprintf(fp,"<Value Name=\"OutInf\" Value=\"0x%x\"/>\n",Entry.ifIndex );
					fprintf(fp,"<Value Name=\"enable\" Value=\"%d\"/>\n",Entry.enable);
#ifdef XML_TR069
					fprintf(fp,"<Value Name=\"leaseduration\" Value=\"%d\"/>\n",Entry.leaseduration);
#endif
					fprintf(fp,"<Value Name=\"remotehost\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)Entry.remotehost)) );
//					fprintf(fp,"<Value Name=\"externalport\" Value=\"%d\"/>\n",Entry.externalport);
					fprintf(fp,"<Value Name=\"externalportStart\" Value=\"%d\"/>\n",Entry.externalfromport);
					fprintf(fp,"<Value Name=\"externalportEnd\" Value=\"%d\"/>\n",Entry.externaltoport);
#ifdef XML_TR069
					fprintf(fp,"<Value Name=\"InstanceNum\" Value=\"%d\"/>\n",Entry.InstanceNum);
#endif
//#endif

					fprintf(fp,"</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"PORT_FW_TBL\">\n");
				fprintf(fp,"</chain>\n");
			}
		}

		//ATM_VC_TBL
		if(chainInfo.id == MIB_ATM_VC_TBL){
			MIB_CE_ATM_VC_T Entry;
			entryNum = mib_chain_total(MIB_ATM_VC_TBL);
			if(entryNum>0){
				for(index=0;index<entryNum;index++){
					unsigned char mask[IP_ADDR_LEN],dir,status,napt,encap;
					if (!mib_chain_get(MIB_ATM_VC_TBL, index, (void *)&Entry))
						continue;
					fprintf(fp,"<chain chainName=\"ATM_VC_TBL\">\n");
					fprintf(fp,"<Value Name=\"ifIndex\" Value=\"0x%x\"/>\n",Entry.ifIndex);
					fprintf(fp,"<Value Name=\"VPI\" Value=\"%d\"/>\n",Entry.vpi);
					fprintf(fp,"<Value Name=\"VCI\" Value=\"%d\"/>\n",Entry.vci);
					if(Entry.encap==1)
						encap=1;
					else
						encap=2;
					fprintf(fp,"<Value Name=\"Encap\" Value=\"%d\"/>\n",encap);
					fprintf(fp,"<Value Name=\"ChannelMode\" Value=\"%d\"/>\n",Entry.cmode+1);
					if(Entry.enable==1)
						status=1;
					else
						status=2;
					fprintf(fp,"<Value Name=\"ChannelStatus\" Value=\"%d\"/>\n",status);

					if(Entry.napt==1)
						napt=1;
					else
						napt=0;
					fprintf(fp,"<Value Name=\"NAPT\" Value=\"%d\"/>\n",napt);

					if(Entry.cmode==2 || Entry.cmode==3){ //PPP
#ifdef DEFAULT_GATEWAY_V1
						unsigned char defaultGW;
#endif
						unsigned char ctype;
						fprintf(fp,"<Value Name=\"pppUser\" Value=\"%s\"/>\n",Entry.pppUsername);
						fprintf(fp,"<Value Name=\"pppPasswd\" Value=\"%s\"/>\n",Entry.pppPassword);
						if(Entry.pppCtype == CONTINUOUS)
							ctype=1;
						else if(Entry.pppCtype == CONNECT_ON_DEMAND)
							ctype=2;
						else ctype=3;
						fprintf(fp,"<Value Name=\"pppConnectType\" Value=\"%d\"/>\n",ctype);
						fprintf(fp,"<Value Name=\"pppIdleTime\" Value=\"%d\"/>\n",Entry.pppIdleTime);

						fprintf(fp,"<Value Name=\"pppAuth\" Value=\"%d\"/>\n",Entry.pppAuth);
						fprintf(fp,"<Value Name=\"pppACName\" Value=\"%s\"/>\n",Entry.pppACName);

#ifdef DEFAULT_GATEWAY_V1
						if(Entry.dgw == 1)
							defaultGW=1;
						else defaultGW=0;
						fprintf(fp,"<Value Name=\"DefaultGW\" Value=\"%d\"/>\n",defaultGW);
#endif
#ifdef XML_TR069
#ifdef _CWMP_MIB_
						fprintf(fp,"<Value Name=\"autoDisTime\" Value=\"%d\"/>\n",Entry.autoDisTime);    // 0
						fprintf(fp,"<Value Name=\"warnDisDelay\" Value=\"%d\"/>\n",Entry.warnDisDelay);  // 0
						fprintf(fp,"<Value Name=\"pppServiceName\" Value=\"%s\"/>\n",Entry.pppServiceName);  // NULL
#ifdef _PRMT_X_CT_COM_PPPOEv2_
						fprintf(fp,"<Value Name=\"PPPoEProxyEnable\" Value=\"%d\"/>\n",Entry.PPPoEProxyEnable);
						fprintf(fp,"<Value Name=\"PPPoEProxyMaxUser\" Value=\"%d\"/>\n",Entry.PPPoEProxyMaxUser);
#endif //_PRMT_X_CT_COM_PPPOEv2_

#endif
#endif  // #ifdef XML_TR069
#ifdef CONFIG_SPPPD_STATICIP
						fprintf(fp,"<Value Name=\"StaticPPPoE\" Value=\"%d\"/>\n", Entry.pppIp);
						if (Entry.pppIp)
							fprintf(fp,"<Value Name=\"StaticIPAddr\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)Entry.ipAddr)));
#endif
					}
					if(Entry.cmode==1 || Entry.cmode==4){ //MER or routed1483
#ifdef DEFAULT_GATEWAY_V1
						unsigned char defaultGW;
#endif
						unsigned char dhcp;
						if(Entry.ipDhcp == DHCP_DISABLED)
							dhcp=1;
						else
							dhcp=2;
						fprintf(fp,"<Value Name=\"ChannelAddrType\" Value=\"%d\"/>\n",dhcp);

						if(dhcp==1){
							fprintf(fp,"<Value Name=\"LocalIPAddr\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)Entry.ipAddr)));
							fprintf(fp,"<Value Name=\"RemoteIPAddr\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)Entry.remoteIpAddr)));
							fprintf(fp,"<Value Name=\"SubnetMask\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)Entry.netMask)));
						}
#ifdef DEFAULT_GATEWAY_V1
						if(Entry.dgw == 1)
							defaultGW=1;
						else defaultGW=0;
						fprintf(fp,"<Value Name=\"DefaultGW\" Value=\"%d\"/>\n",defaultGW);
#endif
					}
					fprintf(fp,"<Value Name=\"BridgeType\" Value=\"%d\"/>\n",Entry.brmode);
					fprintf(fp,"<Value Name=\"Ipunnum\" Value=\"%d\"/>\n",Entry.ipunnumbered);
					fprintf(fp,"<Value Name=\"MTU\" Value=\"%d\"/>\n",Entry.mtu);

					//ATM QoS setting
					fprintf(fp,"<Value Name=\"QoS\" Value=\"%d\"/>\n",(Entry.qos)+1);
					fprintf(fp,"<Value Name=\"PCR\" Value=\"%d\"/>\n",Entry.pcr);
					fprintf(fp,"<Value Name=\"CDVT\" Value=\"%d\"/>\n",Entry.cdvt);
					fprintf(fp,"<Value Name=\"SCR\" Value=\"%d\"/>\n",Entry.scr);
					fprintf(fp,"<Value Name=\"MBS\" Value=\"%d\"/>\n",Entry.mbs);
#ifdef XML_TR069
#ifdef _CWMP_MIB_
					fprintf(fp,"<Value Name=\"connDisable\" Value=\"%d\"/>\n",Entry.connDisable);
					fprintf(fp,"<Value Name=\"ConDevInstNum\" Value=\"%d\"/>\n",Entry.ConDevInstNum);
					fprintf(fp,"<Value Name=\"ConIPInstNum\" Value=\"%d\"/>\n",Entry.ConIPInstNum);
					fprintf(fp,"<Value Name=\"ConPPPInstNum\" Value=\"%d\"/>\n",Entry.ConPPPInstNum);
					fprintf(fp,"<Value Name=\"WanName\" Value=\"%s\"/>\n",Entry.WanName);  // null
#ifdef _PRMT_X_CT_COM_WANEXT_
					fprintf(fp,"<Value Name=\"ServiceList\" Value=\"%d\"/>\n",Entry.ServiceList); // null
#endif //_PRMT_X_CT_COM_WANEXT_
#endif //_CWMP_MIB_
#endif  // #ifdef XML_TR069
#ifdef _PRMT_TR143_
					fprintf(fp,"<Value Name=\"TR143UDPEchoItf\" Value=\"%d\"/>\n",Entry.TR143UDPEchoItf);
#endif //_PRMT_TR143_
#ifdef CONFIG_EXT_SWITCH
					fprintf(fp,"<Value Name=\"vlan\" Value=\"%d\"/>\n",Entry.vlan);
					fprintf(fp,"<Value Name=\"vid\" Value=\"%d\"/>\n",Entry.vid);
					fprintf(fp,"<Value Name=\"vprio\" Value=\"%d\"/>\n",Entry.vprio);
					fprintf(fp,"<Value Name=\"vpass\" Value=\"%d\"/>\n",Entry.vpass);
					fprintf(fp,"<Value Name=\"itfGroup\" Value=\"%d\"/>\n",Entry.itfGroup);
#endif

					fprintf(fp,"</chain>\n");

				}
			} else {
				fprintf(fp,"<chain chainName=\"ATM_VC_TBL\">\n");
				fprintf(fp,"</chain>\n");
			}
		}

		//IP_ROUTE_TBL
		if(chainInfo.id == MIB_IP_ROUTE_TBL){
			MIB_CE_IP_ROUTE_T Entry;
			entryNum = mib_chain_total(MIB_IP_ROUTE_TBL);
			if(entryNum>0){
				for(index=0;index<entryNum;index++){
					if (!mib_chain_get(MIB_IP_ROUTE_TBL, index, (void *)&Entry))
						continue;
					fprintf(fp,"<chain chainName=\"IP_ROUTE_TBL\">\n");
					fprintf(fp,"<Value Name=\"DstIP\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)Entry.destID)));
					fprintf(fp,"<Value Name=\"Mask\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)Entry.netMask)));
					fprintf(fp,"<Value Name=\"NextHop\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)Entry.nextHop)));

//#ifdef _CWMP_MIB_
					fprintf(fp,"<Value Name=\"Enable\" Value=\"%d\"/>\n",Entry.Enable);
#ifdef XML_TR069
					fprintf(fp,"<Value Name=\"Type\" Value=\"%d\"/>\n",Entry.Type);
					fprintf(fp,"<Value Name=\"SourceIP\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)Entry.SourceIP)));
					fprintf(fp,"<Value Name=\"SourceMask\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)Entry.SourceMask)));
#endif
//					fprintf(fp,"<Value Name=\"OutInf\" Value=\"%d\"/>\n",getWanIndex(Entry.ifIndex) );
					fprintf(fp,"<Value Name=\"OutInf\" Value=\"0x%x\"/>\n",Entry.ifIndex );
					fprintf(fp,"<Value Name=\"FWMetric\" Value=\"%d\"/>\n",Entry.FWMetric);
#ifdef XML_TR069
					fprintf(fp,"<Value Name=\"InstanceNum\" Value=\"%d\"/>\n",Entry.InstanceNum);
#endif
//#endif

					fprintf(fp,"</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"IP_ROUTE_TBL\">\n");
				fprintf(fp,"</chain>\n");
			}
		}

#ifdef IP_ACL
		//ACL_IP_TBL
		if(chainInfo.id == MIB_ACL_IP_TBL){
			MIB_CE_ACL_IP_T Entry;
			unsigned char mask[IP_ADDR_LEN];
			entryNum = mib_chain_total(MIB_ACL_IP_TBL);
			if(entryNum>0){
				for(index=0;index<entryNum;index++){
					if (!mib_chain_get(MIB_ACL_IP_TBL, index, (void *)&Entry))
						continue;
					fprintf(fp,"<chain chainName=\"ACL_IP_TBL\">\n");
				#ifdef ACL_IP_RANGE
					fprintf(fp,"<Value Name=\"StartIPAddr\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)Entry.startipAddr)));
					fprintf(fp,"<Value Name=\"EndIPAddr\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)Entry.endipAddr)));
				#endif
					fprintf(fp,"<Value Name=\"IPAddr\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)Entry.ipAddr)));
					//if (Entry.maskbit)    // netmask bug
						getMask(mask, Entry.maskbit);
					fprintf(fp,"<Value Name=\"NetMask\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)mask)));
					fprintf(fp,"<Value Name=\"State\" Value=\"%d\"/>\n", Entry.Enabled);
					if ( Entry.Interface == IF_DOMAIN_LAN )
						fprintf(fp,"<Value Name=\"Interface\" Value=\"%d\"/>\n", 0);
					else
						fprintf(fp,"<Value Name=\"Interface\" Value=\"%d\"/>\n", 1);
					fprintf(fp,"</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"ACL_IP_TBL\">\n");
				fprintf(fp,"</chain>\n");
			}
		}
#endif
#ifdef MAC_ACL
		//ql_xu add: ACL_MAC_TBL
		if (chainInfo.id == MIB_ACL_MAC_TBL) {
			MIB_CE_ACL_MAC_T Entry;
			char tmpBuf[18];

			entryNum = mib_chain_total(MIB_ACL_MAC_TBL);
			if (entryNum > 0) {
				for (index=0; index<entryNum; index++) {
					if (!mib_chain_get(MIB_ACL_MAC_TBL, index, (void *)&Entry))
						continue;

					snprintf(tmpBuf, 18, "%02x-%02x-%02x-%02x-%02x-%02x",
						Entry.macAddr[0], Entry.macAddr[1], Entry.macAddr[2],
						Entry.macAddr[3], Entry.macAddr[4], Entry.macAddr[5]);

					fprintf(fp, "<chain chainName=\"ACL_MAC_TBL\">\n");
					fprintf(fp, "<Value Name=\"MacAddr\" Value=\"%s\"/>\n", tmpBuf);
					fprintf(fp, "<Value Name=\"State\" Value=\"%d\"/>\n", Entry.Enabled);
					if ( Entry.Interface == IF_DOMAIN_LAN )
						fprintf(fp,"<Value Name=\"Interface\" Value=\"%d\"/>\n", 0);
					else
						fprintf(fp,"<Value Name=\"Interface\" Value=\"%d\"/>\n", 1);
					fprintf(fp, "</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"ACL_MAC_TBL\">\n");
				fprintf(fp,"</chain>\n");
			}
		}
#endif
#ifdef NAT_CONN_LIMIT
		//ql_xu add: NAT_CONN_LIMIT
		if (chainInfo.id == MIB_CONN_LIMIT_TBL) {
			MIB_CE_CONN_LIMIT_T Entry;

			entryNum = mib_chain_total(MIB_CONN_LIMIT_TBL);
			if (entryNum > 0) {
				for (index=0; index<entryNum; index++) {
					if (!mib_chain_get(MIB_CONN_LIMIT_TBL, index, (void *)&Entry))
						continue;
					fprintf(fp,"<chain chainName=\"NAT_CONN_LIMIT\">\n");
					fprintf(fp,"<Value Name=\"IPAddr\" Value=\"%s\"/>\n", inet_ntoa(*((struct in_addr *)Entry.ipAddr)));
					fprintf(fp,"<Value Name=\"State\" Value=\"%d\"/>\n", Entry.Enabled);
					fprintf(fp,"<Value Name=\"connNum\" Value=\"%d\"/>\n", Entry.connNum);
					fprintf(fp,"</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"NAT_CONN_LIMIT\">\n");
				fprintf(fp,"</chain>\n");
			}
		}
#endif
#ifdef TCP_UDP_CONN_LIMIT
		//eric : TCP_UDP_CONN_LIMIT xml config
		if (chainInfo.id == MIB_TCP_UDP_CONN_LIMIT_TBL) {
			MIB_CE_TCP_UDP_CONN_LIMIT_T Entry;

			entryNum = mib_chain_total(MIB_TCP_UDP_CONN_LIMIT_TBL);
			if (entryNum > 0) {
				for (index=0; index<entryNum; index++) {
					if (!mib_chain_get(MIB_TCP_UDP_CONN_LIMIT_TBL, index, (void *)&Entry))
						continue;
					fprintf(fp,"<chain chainName=\"TCP_UDP_CONN_LIMIT\">\n");
					fprintf(fp,"<Value Name=\"IPAddr\" Value=\"%s\"/>\n", inet_ntoa(*((struct in_addr *)Entry.ipAddr)));
					fprintf(fp,"<Value Name=\"Protocol\" Value=\"%s\"/>\n", Entry.protocol?"TCP":"UDP");
					fprintf(fp,"<Value Name=\"State\" Value=\"%d\"/>\n", Entry.Enabled);
					fprintf(fp,"<Value Name=\"connNum\" Value=\"%d\"/>\n", Entry.connNum);
					fprintf(fp,"</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"TCP_UDP_CONN_LIMIT\">\n");
				fprintf(fp,"</chain>\n");
			}
		}
#endif
#ifdef ADDRESS_MAPPING
#ifdef MULTI_ADDRESS_MAPPING
		//eric : MULTI_ADDRESS_MAPPING_LIMIT_TBL xml config
		if (chainInfo.id == MULTI_ADDRESS_MAPPING_LIMIT_TBL) {
			MIB_CE_MULTI_ADDR_MAPPING_LIMIT_T Entry;

			entryNum = mib_chain_total(MULTI_ADDRESS_MAPPING_LIMIT_TBL);
			if (entryNum > 0) {
				for (index=0; index<entryNum; index++) {
					if (!mib_chain_get(MULTI_ADDRESS_MAPPING_LIMIT_TBL, index, (void *)&Entry))
						continue;
					fprintf(fp,"<chain chainName=\"ADDRESS_MAPPING\">\n");
					fprintf(fp,"<Value Name=\"AddressMappingType\" Value=\"%s\"/>\n", getaddrmappingstr(Entry.addressMapType));
					fprintf(fp,"<Value Name=\"LocalStartIP\" Value=\"%s\"/>\n", inet_ntoa(*((struct in_addr *)Entry.lsip)));
					fprintf(fp,"<Value Name=\"LocalEndIP\" Value=\"%s\"/>\n", inet_ntoa(*((struct in_addr *)Entry.leip)));
					fprintf(fp,"<Value Name=\"GlobalStartIP\" Value=\"%s\"/>\n", inet_ntoa(*((struct in_addr *)Entry.gsip)));
					fprintf(fp,"<Value Name=\"GlobalEndIP\" Value=\"%s\"/>\n", inet_ntoa(*((struct in_addr *)Entry.geip)));
					fprintf(fp,"</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"ADDRESS_MAPPING\">\n");
				fprintf(fp,"</chain>\n");
			}
		}
#endif
#endif

#ifdef WLAN_SUPPORT
#ifdef WLAN_ACL
		//WLAN_AC_TBL
		if(chainInfo.id == MIB_WLAN_AC_TBL){
			MIB_CE_WLAN_AC_T Entry;
			entryNum = mib_chain_total(MIB_WLAN_AC_TBL);
			if(entryNum>0){
				for(index=0;index<entryNum;index++){
					if (!mib_chain_get(MIB_WLAN_AC_TBL, index, (void *)&Entry))
						continue;
					fprintf(fp,"<chain chainName=\"WLAN_AC_TBL\">\n");
					fprintf(fp,"<Value Name=\"MacAddr\" Value=\"%02x%02x%02x%02x%02x%02x\"/>\n",
						Entry.macAddr[0],Entry.macAddr[1],Entry.macAddr[2],Entry.macAddr[3],Entry.macAddr[4],Entry.macAddr[5]);
					fprintf(fp,"</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"WLAN_AC_TBL\">\n");
				fprintf(fp,"</chain>\n");
			}
		}
#endif

#ifdef WLAN_MBSSID
		//MIB_MBSSIB_TBL
		if(chainInfo.id == MIB_MBSSIB_TBL){
			MIB_CE_MBSSIB_T Entry;
			entryNum = mib_chain_total(MIB_MBSSIB_TBL);
			// Mason Yu Test
			printf("MIB_MBSSIB_TBL: entryNum=%d\n", entryNum);
			if(entryNum>0){
				for(index=0;index<entryNum;index++){
					if (!mib_chain_get(MIB_MBSSIB_TBL, index, (void *)&Entry))
						continue;
					fprintf(fp,"<chain chainName=\"MBSSIB_TBL\">\n");
					fprintf(fp,"<Value Name=\"idx\" Value=\"%d\"/>\n",Entry.idx);
					fprintf(fp,"<Value Name=\"encrypt\" Value=\"%d\"/>\n",Entry.encrypt);
					fprintf(fp,"<Value Name=\"enable1X\" Value=\"%d\"/>\n",Entry.enable1X);
					fprintf(fp,"<Value Name=\"wep\" Value=\"%d\"/>\n",Entry.wep);
					fprintf(fp,"<Value Name=\"wpaAuth\" Value=\"%d\"/>\n",Entry.wpaAuth);
					fprintf(fp,"<Value Name=\"wpaPSKFormat\" Value=\"%d\"/>\n",Entry.wpaPSKFormat);
					fprintf(fp,"<Value Name=\"wpaPSK\" Value=\"%s\"/>\n",Entry.wpaPSK);
					fprintf(fp,"<Value Name=\"rsPort\" Value=\"%d\"/>\n",Entry.rsPort);
					fprintf(fp,"<Value Name=\"rsIpAddr\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)Entry.rsIpAddr)));
					fprintf(fp,"<Value Name=\"rsPassword\" Value=\"%s\"/>\n",Entry.rsPassword);
					fprintf(fp,"<Value Name=\"wlanDisabled\" Value=\"%d\"/>\n",Entry.wlanDisabled);
					fprintf(fp,"<Value Name=\"ssid\" Value=\"%s\"/>\n",Entry.ssid);
					fprintf(fp,"<Value Name=\"authType\" Value=\"%d\"/>\n",Entry.authType);
#ifdef XML_TR069
#ifdef _CWMP_MIB_
					fprintf(fp,"<Value Name=\"cwmp_WLAN_BasicEncry\" Value=\"%d\"/>\n",Entry.cwmp_WLAN_BasicEncry);
#endif
#endif // #ifdef XML_TR069
#ifdef ENABLE_WPAAES_WPA2TKIP
					fprintf(fp,"<Value Name=\"unicastCipher\" Value=\"%d\"/>\n",Entry.unicastCipher);
					fprintf(fp,"<Value Name=\"wpa2UnicastCipher\" Value=\"%d\"/>\n",Entry.wpa2UnicastCipher);
#endif
					fprintf(fp,"</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"MBSSIB_TBL\">\n");
				fprintf(fp,"</chain>\n");
			}
		}

		//MIB_MBSSIB_WEP_TBL
		if(chainInfo.id == MIB_MBSSIB_WEP_TBL){
			MIB_CE_MBSSIB_WEP_T Entry;
			entryNum = mib_chain_total(MIB_MBSSIB_WEP_TBL);
			if(entryNum>0){
				for(index=0;index<entryNum;index++){
					if (!mib_chain_get(MIB_MBSSIB_WEP_TBL, index, (void *)&Entry))
						continue;
					fprintf(fp,"<chain chainName=\"MBSSIB_WEP_TBL\">\n");
					fprintf(fp,"<Value Name=\"wep64Key1\" Value=\"%02x%02x%02x%02x%02x\"/>\n",Entry.wep64Key1[0],Entry.wep64Key1[1],
						Entry.wep64Key1[2],Entry.wep64Key1[3],Entry.wep64Key1[4]);
					fprintf(fp,"<Value Name=\"wep64Key2\" Value=\"%02x%02x%02x%02x%02x\"/>\n",Entry.wep64Key2[0],Entry.wep64Key2[1],
						Entry.wep64Key2[2],Entry.wep64Key2[3],Entry.wep64Key2[4]);
					fprintf(fp,"<Value Name=\"wep64Key3\" Value=\"%02x%02x%02x%02x%02x\"/>\n",Entry.wep64Key3[0],Entry.wep64Key3[1],
						Entry.wep64Key3[2],Entry.wep64Key3[3],Entry.wep64Key3[4]);
					fprintf(fp,"<Value Name=\"wep64Key4\" Value=\"%02x%02x%02x%02x%02x\"/>\n",Entry.wep64Key4[0],Entry.wep64Key4[1],
						Entry.wep64Key4[2],Entry.wep64Key4[3],Entry.wep64Key4[4]);
					fprintf(fp,"<Value Name=\"wep128Key1\" Value=\"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\"/>\n",Entry.wep128Key1[0],
						Entry.wep128Key1[1],Entry.wep128Key1[2],Entry.wep128Key1[3],Entry.wep128Key1[4],Entry.wep128Key1[5],Entry.wep128Key1[6],
						Entry.wep128Key1[7],Entry.wep128Key1[8],Entry.wep128Key1[9],Entry.wep128Key1[10],Entry.wep128Key1[11],Entry.wep128Key1[12]);
					fprintf(fp,"<Value Name=\"wep128Key2\" Value=\"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\"/>\n",Entry.wep128Key2[0],
						Entry.wep128Key2[1],Entry.wep128Key2[2],Entry.wep128Key2[3],Entry.wep128Key2[4],Entry.wep128Key2[5],Entry.wep128Key2[6],
						Entry.wep128Key2[7],Entry.wep128Key2[8],Entry.wep128Key2[9],Entry.wep128Key2[10],Entry.wep128Key2[11],Entry.wep128Key2[12]);
					fprintf(fp,"<Value Name=\"wep128Key3\" Value=\"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\"/>\n",Entry.wep128Key3[0],
						Entry.wep128Key3[1],Entry.wep128Key3[2],Entry.wep128Key3[3],Entry.wep128Key3[4],Entry.wep128Key3[5],Entry.wep128Key3[6],
						Entry.wep128Key3[7],Entry.wep128Key3[8],Entry.wep128Key3[9],Entry.wep128Key3[10],Entry.wep128Key3[11],Entry.wep128Key3[12]);
					fprintf(fp,"<Value Name=\"wep128Key4\" Value=\"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\"/>\n",Entry.wep128Key4[0],
						Entry.wep128Key4[1],Entry.wep128Key4[2],Entry.wep128Key4[3],Entry.wep128Key4[4],Entry.wep128Key4[5],Entry.wep128Key4[6],
						Entry.wep128Key4[7],Entry.wep128Key4[8],Entry.wep128Key4[9],Entry.wep128Key4[10],Entry.wep128Key4[11],Entry.wep128Key4[12]);
					fprintf(fp,"<Value Name=\"wepDefaultKey\" Value=\"%d\"/>\n",Entry.wepDefaultKey);
					fprintf(fp,"<Value Name=\"wepKeyType\" Value=\"%d\"/>\n",Entry.wepKeyType);

					fprintf(fp,"</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"MBSSIB_WEP_TBL\">\n");
				fprintf(fp,"</chain>\n");
			}
		}
#endif

#ifdef XML_TR069
#ifdef _CWMP_MIB_
		//CWMP_PSK_TBL
		if(chainInfo.id == CWMP_PSK_TBL){
			CWMP_PSK_T Entry;
			entryNum = mib_chain_total(CWMP_PSK_TBL);
			if(entryNum>0){
				for(index=0;index<entryNum;index++){
					if (!mib_chain_get(CWMP_PSK_TBL, index, (void *)&Entry))
						continue;
					fprintf(fp,"<chain chainName=\"CWMP_PSK_TBL\">\n");
					fprintf(fp,"<Value Name=\"index\" Value=\"%d\"/>\n",Entry.index);
					fprintf(fp,"<Value Name=\"presharedkey\" Value=\"%s\"/>\n",Entry.presharedkey);
					fprintf(fp,"<Value Name=\"keypassphrase\" Value=\"%s\"/>\n",Entry.keypassphrase);

					fprintf(fp,"</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"CWMP_PSK_TBL\">\n");
				fprintf(fp,"</chain>\n");
			}
		}
#endif /*_CWMP_MIB_*/
#endif // #ifdef XML_TR069
#endif

#ifndef ZTE_531B_BRIDGE_SC
		//ACC_TBL
		if(chainInfo.id == MIB_ACC_TBL){
			MIB_CE_ACC_T Entry;
			entryNum = mib_chain_total(MIB_ACC_TBL);
			if(entryNum>0){
				for(index=0;index<entryNum;index++){
					if (!mib_chain_get(MIB_ACC_TBL, index, (void *)&Entry))
						continue;
					fprintf(fp,"<chain chainName=\"ACC_TBL\">\n");
					#ifdef CONFIG_USER_TELNETD_TELNETD
					#ifndef ZTE_GENERAL_ROUTER_SC
					if(Entry.telnet & 0x02)
						fprintf(fp,"<Value Name=\"Telnet_LAN\" Value=\"1\"/>\n");
					else
						fprintf(fp,"<Value Name=\"Telnet_LAN\" Value=\"0\"/>\n");
					#endif
					if(Entry.telnet & 0x01)
						fprintf(fp,"<Value Name=\"Telnet_WAN\" Value=\"1\"/>\n");
					else
						fprintf(fp,"<Value Name=\"Telnet_WAN\" Value=\"0\"/>\n");
					fprintf(fp,"<Value Name=\"Telnet_WAN_Port\" Value=\"%d\"/>\n", Entry.telnet_port);
					#endif
					#ifdef CONFIG_USER_FTPD_FTPD
					#ifndef ZTE_GENERAL_ROUTER_SC
					if(Entry.ftp & 0x02)
						fprintf(fp,"<Value Name=\"Ftp_LAN\" Value=\"1\"/>\n");
					else
						fprintf(fp,"<Value Name=\"Ftp_LAN\" Value=\"0\"/>\n");
					#endif
					if(Entry.ftp & 0x01)
						fprintf(fp,"<Value Name=\"Ftp_WAN\" Value=\"1\"/>\n");
					else
						fprintf(fp,"<Value Name=\"Ftp_WAN\" Value=\"0\"/>\n");
					fprintf(fp,"<Value Name=\"Ftp_WAN_Port\" Value=\"%d\"/>\n", Entry.ftp_port);
					#endif
					#ifdef CONFIG_USER_TFTPD_TFTPD
					#ifndef ZTE_GENERAL_ROUTER_SC
					if(Entry.tftp & 0x02)
						fprintf(fp,"<Value Name=\"Tftp_LAN\" Value=\"1\"/>\n");
					else
						fprintf(fp,"<Value Name=\"Tftp_LAN\" Value=\"0\"/>\n");
					#endif
					if(Entry.tftp & 0x01)
						fprintf(fp,"<Value Name=\"Tftp_WAN\" Value=\"1\"/>\n");
					else
						fprintf(fp,"<Value Name=\"Tftp_WAN\" Value=\"0\"/>\n");
					#endif

					#ifndef ZTE_GENERAL_ROUTER_SC
					if(Entry.web & 0x02)
						fprintf(fp,"<Value Name=\"Web_LAN\" Value=\"1\"/>\n");
					else
						fprintf(fp,"<Value Name=\"Web_LAN\" Value=\"0\"/>\n");
					#endif
					if(Entry.web & 0x01)
						fprintf(fp,"<Value Name=\"Web_WAN\" Value=\"1\"/>\n");
					else
						fprintf(fp,"<Value Name=\"Web_WAN\" Value=\"0\"/>\n");
					fprintf(fp,"<Value Name=\"Web_WAN_Port\" Value=\"%d\"/>\n", Entry.web_port);

					#ifdef CONFIG_USER_BOA_WITH_SSL
					#ifndef ZTE_GENERAL_ROUTER_SC
					if(Entry.https & 0x02)
						fprintf(fp,"<Value Name=\"Https_LAN\" Value=\"1\"/>\n");
					else
						fprintf(fp,"<Value Name=\"Https_LAN\" Value=\"0\"/>\n");
					#endif
					if(Entry.https & 0x01)
						fprintf(fp,"<Value Name=\"Https_WAN\" Value=\"1\"/>\n");
					else
						fprintf(fp,"<Value Name=\"Https_WAN\" Value=\"0\"/>\n");
					fprintf(fp,"<Value Name=\"Https_WAN_Port\" Value=\"%d\"/>\n", Entry.https_port);
					#endif //end of CONFIG_USER_BOA_WITH_SSL

					#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
					#ifndef ZTE_GENERAL_ROUTER_SC
					if(Entry.snmp & 0x02)
						fprintf(fp,"<Value Name=\"Snmp_LAN\" Value=\"1\"/>\n");
					else
						fprintf(fp,"<Value Name=\"Snmp_LAN\" Value=\"0\"/>\n");
					#endif
					if(Entry.snmp & 0x01)
						fprintf(fp,"<Value Name=\"Snmp_WAN\" Value=\"1\"/>\n");
					else
						fprintf(fp,"<Value Name=\"Snmp_WAN\" Value=\"0\"/>\n");
					#endif
					#ifdef CONFIG_USER_SSH_DROPBEAR
					#ifndef ZTE_GENERAL_ROUTER_SC
					if(Entry.ssh & 0x02)
						fprintf(fp,"<Value Name=\"Ssh_LAN\" Value=\"1\"/>\n");
					else
						fprintf(fp,"<Value Name=\"Ssh_LAN\" Value=\"0\"/>\n");
					#endif
					if(Entry.ssh & 0x01)
						fprintf(fp,"<Value Name=\"Ssh_WAN\" Value=\"1\"/>\n");
					else
						fprintf(fp,"<Value Name=\"Ssh_WAN\" Value=\"0\"/>\n");
					#endif
					#ifndef ZTE_GENERAL_ROUTER_SC
					if(Entry.icmp & 0x02)
						fprintf(fp,"<Value Name=\"Icmp_LAN\" Value=\"1\"/>\n");
					else
						fprintf(fp,"<Value Name=\"Icmp_LAN\" Value=\"0\"/>\n");
					#endif
					if(Entry.icmp & 0x01)
						fprintf(fp,"<Value Name=\"Icmp_WAN\" Value=\"1\"/>\n");
					else
						fprintf(fp,"<Value Name=\"Icmp_WAN\" Value=\"0\"/>\n");
					#ifdef ZTE_GENERAL_ROUTER_SC
						fprintf(fp,"<Value Name=\"ifIndex\" Value=\"%d\"/>\n",Entry.ifIndex);
						fprintf(fp,"<Value Name=\"vpi\" Value=\"%d\"/>\n",Entry.vpi);
						fprintf(fp,"<Value Name=\"vci\" Value=\"%d\"/>\n",Entry.vci);
						fprintf(fp,"<Value Name=\"protocol\" Value=\"%d\"/>\n",Entry.protocol);
					#endif
					fprintf(fp,"</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"ACC_TBL\">\n");
				fprintf(fp,"</chain>\n");
			}
		}
#endif
#ifdef WLAN_WDS
		//WLAN_WDS_TBL
		if(chainInfo.id == MIB_WDS_TBL){
			WDS_T Entry;
			entryNum = mib_chain_total(MIB_WDS_TBL);
			if(entryNum>0){
				for(index=0;index<entryNum;index++){
					if (!mib_chain_get(MIB_WDS_TBL, index, (void *)&Entry))
						continue;
					fprintf(fp,"<chain chainName=\"WLAN_WDS_TBL\">\n");
					fprintf(fp,"<Value Name=\"MacAddr\" Value=\"%02x%02x%02x%02x%02x%02x\"/>\n",
						Entry.macAddr[0],Entry.macAddr[1],Entry.macAddr[2],Entry.macAddr[3],Entry.macAddr[4],Entry.macAddr[5]);
					fprintf(fp,"<Value Name=\"Comment\" Value=\"%s\"/>\n", Entry.comment);
					fprintf(fp,"</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"WLAN_WDS_TBL\">\n");
				fprintf(fp,"</chain>\n");
			}
		}
#endif

#ifdef CONFIG_EXT_SWITCH
//star zhang add to support port mapping ...
#if 1
		if(chainInfo.id == MIB_SW_PORT_TBL){
			MIB_CE_SW_PORT_T Entry;

			int ifnum,i;
			entryNum = mib_chain_total(MIB_SW_PORT_TBL);
			if(entryNum>0){
				for(index=0;index<entryNum;index++){
					int entryNum_atm,atmitf;
					if (!mib_chain_get(MIB_SW_PORT_TBL, index, (void *)&Entry))
						continue;

					fprintf(fp,"<chain chainName=\"SW_PORT_TBL\">\n");

					fprintf(fp,"<Value Name=\"Switch\" Value=\"%d\"/>\n",index+1);
/*
					showInf();

					entryNum_atm = mib_chain_total(MIB_ATM_VC_TBL);
					for (atmitf=0; atmitf<entryNum_atm; atmitf++) {
						if(IFsel[atmitf]==Entry.pvcItf)
							break;
					}

					fprintf(fp,"<Value Name=\"PVCInterface\" Value=\"%d\"/>\n",Entry.pvcItf);
*/
					fprintf(fp,"<Value Name=\"InterfaceGroup\" Value=\"%d\"/>\n",Entry.itfGroup);
					fprintf(fp,"<Value Name=\"PVID\" Value=\"%d\"/>\n",Entry.pvid);
					fprintf(fp,"<Value Name=\"EgressTag\" Value=\"%d\"/>\n",Entry.egressTagAction);
					fprintf(fp,"<Value Name=\"LinkMode\" Value=\"%d\"/>\n",Entry.linkMode);


					fprintf(fp,"</chain>\n");
				}
			}
			/*
			else {
				fprintf(fp,"<chain chainName=\"SW_PORT_TBL\">\n");
				fprintf(fp,"</chain>\n");
			}
			*/
		}

#endif
#endif
		//IP_QOS_TBL
#ifdef NEW_IP_QOS_SUPPORT
		if (chainInfo.id == MIB_IP_QOS_TC_TBL){
			MIB_CE_IP_TC_T entry;
			int entryNum, i;

			entryNum = mib_chain_total(MIB_IP_QOS_TC_TBL);
			for (i=0; i<entryNum; i++) {
				unsigned char mask[IP_ADDR_LEN];

				if (!mib_chain_get(MIB_IP_QOS_TC_TBL, i, (void *)&entry))
					continue;

				fprintf(fp, "<chain chainName=\"QOS_TC_TBL\">\n");
				fprintf(fp, "<Value Name=\"entryid\" Value=\"%d\"/>\n", entry.entryid);
				fprintf(fp, "<Value Name=\"ifIndex\" Value=\"%d\"/>\n", entry.ifIndex);
				fprintf(fp, "<Value Name=\"srcip\" Value=\"%s\"/>\n", inet_ntoa(*(struct in_addr *)&entry.srcip));
				getMask(mask, entry.smaskbits);
				fprintf(fp, "<Value Name=\"srcmask\" Value=\"%s\"/>\n", inet_ntoa(*(struct in_addr *)mask));
				fprintf(fp, "<Value Name=\"dstip\" Value=\"%s\"/>\n", inet_ntoa(*(struct in_addr *)&entry.dstip));
				getMask(mask, entry.dmaskbits);
				fprintf(fp, "<Value Name=\"dstmask\" Value=\"%s\"/>\n", inet_ntoa(*(struct in_addr *)mask));
				fprintf(fp, "<Value Name=\"srcport\" Value=\"%d\"/>\n", entry.sport);
				fprintf(fp, "<Value Name=\"dstport\" Value=\"%d\"/>\n", entry.dport);
				fprintf(fp, "<Value Name=\"protocol\" Value=\"%d\"/>\n", entry.protoType);
				fprintf(fp, "<Value Name=\"rate\" Value=\"%d\"/>\n", entry.limitSpeed);
				fprintf(fp,"</chain>\n");
			}
		}
#endif

#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
		if(chainInfo.id == MIB_IP_QOS_TBL){
			MIB_CE_IP_QOS_T Entry;

			int ifnum,i;
			entryNum = mib_chain_total(MIB_IP_QOS_TBL);
			if(entryNum>0){
				for(index=0;index<entryNum;index++){
					if (!mib_chain_get(MIB_IP_QOS_TBL, index, (void *)&Entry))
						continue;
					int entryNum_atm,atmitf;
					unsigned char mask[IP_ADDR_LEN];

					fprintf(fp,"<chain chainName=\"IP_QOS_TBL\">\n");

					fprintf(fp,"<Value Name=\"SrcIP\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)Entry.sip)));
					getMask(mask,Entry.smaskbit);
					fprintf(fp,"<Value Name=\"SrcMask\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)mask)));
					fprintf(fp,"<Value Name=\"SrcPort\" Value=\"%d\"/>\n",Entry.sPort);
					fprintf(fp,"<Value Name=\"DstIP\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)Entry.dip)));
					getMask(mask,Entry.dmaskbit);
					fprintf(fp,"<Value Name=\"DstMask\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)mask)));
					fprintf(fp,"<Value Name=\"DstPort\" Value=\"%d\"/>\n",Entry.dPort);

					fprintf(fp,"<Value Name=\"Protocol\" Value=\"%d\"/>\n",Entry.protoType);
					fprintf(fp,"<Value Name=\"PhyPort\" Value=\"%d\"/>\n",Entry.phyPort);
#ifdef NEW_IP_QOS_SUPPORT
					fprintf(fp,"<Value Name=\"qosDscp\" Value=\"%d\"/>\n",Entry.qosDscp);
					fprintf(fp,"<Value Name=\"vlan1p\" Value=\"%d\"/>\n",Entry.vlan1p);
#endif

					showInf();


					entryNum_atm = mib_chain_total(MIB_ATM_VC_TBL);
					for (atmitf=0; atmitf<entryNum_atm; atmitf++) {
						if(IFsel[atmitf]==Entry.outif)
							break;
					}

					fprintf(fp,"<Value Name=\"OutInf\" Value=\"%d\"/>\n",atmitf+1);
					fprintf(fp,"<Value Name=\"Prior\" Value=\"%d\"/>\n",Entry.prior);
	        // jim 2007-07-02 below entries should be saved....
#ifdef QOS_DSCP
					if (Entry.dscp == 0) {
#endif
					fprintf(fp,"<Value Name=\"m_ipprio\" Value=\"%d\"/>\n",Entry.m_ipprio);
					fprintf(fp,"<Value Name=\"m_iptos\" Value=\"%d\"/>\n",Entry.m_iptos);
#ifdef QOS_DSCP
					}
					else if (Entry.dscp == 1) {
#ifdef NEW_IP_QOS_SUPPORT
						fprintf(fp,"<Value Name=\"m_dscp\" Value=\"%d\"/>\n", Entry.m_dscp);
#else
						int dscp;
						dscp = Entry.m_ipprio << 3;
						dscp |= Entry.m_iptos << 1;
						fprintf(fp,"<Value Name=\"m_dscp\" Value=\"%d\"/>\n", dscp);
#endif
					}
#endif
					fprintf(fp,"<Value Name=\"m_1p\" Value=\"%d\"/>\n",Entry.m_1p);

//#ifdef _CWMP_MIB_
					fprintf(fp,"<Value Name=\"Enable\" Value=\"%d\"/>\n",Entry.enable);
#ifdef XML_TR069
					fprintf(fp,"<Value Name=\"InstanceNum\" Value=\"%d\"/>\n",Entry.InstanceNum);
#endif

#ifdef QOS_SPEED_LIMIT_SUPPORT
					fprintf(fp,"<Value Name=\"limitSpeedEnabled\" Value=\"%d\"/>\n",Entry.limitSpeedEnabled);
					fprintf(fp,"<Value Name=\"limitSpeedRank\" Value=\"%d\"/>\n",Entry.limitSpeedRank);
#endif

					fprintf(fp,"</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"IP_QOS_TBL\">\n");
				fprintf(fp,"</chain>\n");
			}
		}
//add by ramen
#ifdef QOS_SPEED_LIMIT_SUPPORT
if(chainInfo.id == MIB_QOS_SPEED_LIMIT){
	MIB_CE_IP_QOS_SPEEDRANK_T qosSpeedEntry;
	entryNum = mib_chain_total(MIB_QOS_SPEED_LIMIT);
	if(entryNum>0){
		for(index=0;index<entryNum;index++){
					if (!mib_chain_get(MIB_QOS_SPEED_LIMIT, index, (void *)&qosSpeedEntry))
						continue;
					fprintf(fp,"<chain chainName=\"MBQOS_SPEED_LIMIT\">\n");
					fprintf(fp,"<Value Name=\"index\" Value=\"%d\"/>\n",qosSpeedEntry.index);
					fprintf(fp,"<Value Name=\"count\" Value=\"%d\"/>\n",qosSpeedEntry.count);
					fprintf(fp,"<Value Name=\"speed\" Value=\"%d\"/>\n",qosSpeedEntry.speed);
					fprintf(fp,"<Value Name=\"prior\" Value=\"%d\"/>\n",qosSpeedEntry.prior);
					fprintf(fp,"</chain>\n");
		}
	} else {
		fprintf(fp,"<chain chainName=\"MBQOS_SPEED_LIMIT\">\n");
		fprintf(fp,"</chain>\n");
	}
}
#endif
#endif	// of IP_QOS
//#endif

#if 0
		//PPPOE_SESSION_TBL
		if(chainInfo.id == MIB_PPPOE_SESSION_TBL){
			MIB_CE_PPPOE_SESSION_T Entry;
			entryNum = mib_chain_total(MIB_PPPOE_SESSION_TBL);
			if(entryNum>0){
				for(index=0;index<entryNum;index++){
					if (!mib_chain_get(MIB_PPPOE_SESSION_TBL, index, (void *)&Entry))
						continue;
					fprintf(fp,"<chain chainName=\"PPPOE_SESSION_TBL\">\n");
					fprintf(fp,"<Value Name=\"InterfaceNo\" Value=\"%d\"/>\n", Entry.ifNo);
					fprintf(fp,"<Value Name=\"VPI\" Value=\"%d\"/>\n", Entry.vpi);
					fprintf(fp,"<Value Name=\"VCI\" Value=\"%d\"/>\n", Entry.vci);
					fprintf(fp,"<Value Name=\"ACMacAddr\" Value=\"%02x%02x%02x%02x%02x%02x\"/>\n",
						Entry.acMac[0], Entry.acMac[1], Entry.acMac[2], Entry.acMac[3], Entry.acMac[4], Entry.acMac[5]);
					fprintf(fp,"<Value Name=\"SessionID\" Value=\"%d\"/>\n", Entry.sessionId);

					fprintf(fp,"</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"PPPOE_SESSION_TBL\">\n");
				fprintf(fp,"</chain>\n");
			}
		}
#endif
//star add
		if(chainInfo.id == MIB_MAC_BASE_DHCP_TBL){
			MIB_CE_MAC_BASE_DHCP_T Entry;
			char macaddr[20];
			
			entryNum = mib_chain_total(MIB_MAC_BASE_DHCP_TBL);
			if(entryNum>0){
				for(index=0;index<entryNum;index++){
					if (!mib_chain_get(MIB_MAC_BASE_DHCP_TBL, index, (void *)&Entry))
						continue;
					
					snprintf(macaddr, 18, "%02x-%02x-%02x-%02x-%02x-%02x",
						Entry.macAddr_Dhcp[0], Entry.macAddr_Dhcp[1],
						Entry.macAddr_Dhcp[2], Entry.macAddr_Dhcp[3],
						Entry.macAddr_Dhcp[4], Entry.macAddr_Dhcp[5]);
				
					fprintf(fp,"<chain chainName=\"MAC_BASE_DHCP_TBL\">\n");
					fprintf(fp,"<Value Name=\"MacAddr\" Value=\"%s\"/>\n",
						macaddr);
					fprintf(fp,"<Value Name=\"IpAddr\" Value=\"%s\"/>\n",
						inet_ntoa(*((struct in_addr *)Entry.ipAddr_Dhcp)) );

					fprintf(fp,"</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"MAC_BASE_DHCP_TBL\">\n");
				fprintf(fp,"</chain>\n");
			}
		}

		if(chainInfo.id == MIB_RIP_TBL){
			MIB_CE_RIP_T Entry;
			entryNum = mib_chain_total(MIB_RIP_TBL);
			if(entryNum>0){
				for(index=0;index<entryNum;index++){
					if (!mib_chain_get(MIB_RIP_TBL, index, (void *)&Entry))
						continue;
					fprintf(fp,"<chain chainName=\"RIP_TBL\">\n");
					//fprintf(fp,"<Value Name=\"IfIndex\" Value=\"%d\"/>\n", Entry.ifIndex);
					fprintf(fp,"<Value Name=\"IfIndex\" Value=\"0x%x\"/>\n", Entry.ifIndex);
					fprintf(fp,"<Value Name=\"ReceiveMode\" Value=\"%d\"/>\n",
						Entry.receiveMode);
					fprintf(fp,"<Value Name=\"SendMode\" Value=\"%d\"/>\n",
						Entry.sendMode);

					fprintf(fp,"</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"RIP_TBL\">\n");
				fprintf(fp,"</chain>\n");
			}
		}
#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
		if(chainInfo.id == MIB_OSPF_TBL){
			MIB_CE_OSPF_T Entry;
			entryNum = mib_chain_total(MIB_OSPF_TBL);
			if(entryNum>0){
				for(index=0;index<entryNum;index++){
					if (!mib_chain_get(MIB_OSPF_TBL, index, (void *)&Entry)) {
						printf("get MIB_OSPF_TBL error\n");
						continue;
					}
					fprintf(fp,"<chain chainName=\"OSPF_TBL\">\n");
					fprintf(fp,"<Value Name=\"IP\" Value=\"%s\"/>\n",
						inet_ntoa(*((struct in_addr *)Entry.ipAddr)));
					fprintf(fp,"<Value Name=\"Mask\" Value=\"%s\"/>\n",
						inet_ntoa(*((struct in_addr *)Entry.netMask)));

					fprintf(fp,"</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"OSPF_TBL\">\n");
				fprintf(fp,"</chain>\n");
			}
		}
#endif
#ifdef URL_BLOCKING_SUPPORT
		if(chainInfo.id == MIB_URL_FQDN_TBL){
			MIB_CE_URL_FQDN_T Entry;
			entryNum = mib_chain_total(MIB_URL_FQDN_TBL);
			if(entryNum>0){
				for(index=0;index<entryNum;index++){
					if (!mib_chain_get(MIB_URL_FQDN_TBL, index, (void *)&Entry))
						continue;
					fprintf(fp,"<chain chainName=\"URL_FQDN_TBL\">\n");
					fprintf(fp,"<Value Name=\"URL\" Value=\"%s\"/>\n",Entry.fqdn);
					fprintf(fp,"</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"URL_FQDN_TBL\">\n");
				fprintf(fp,"</chain>\n");
			}
		}

		if(chainInfo.id == MIB_KEYWD_FILTER_TBL){
			MIB_CE_KEYWD_FILTER_T Entry;
			entryNum = mib_chain_total(MIB_KEYWD_FILTER_TBL);
			if(entryNum>0){
				for(index=0;index<entryNum;index++){
					if (!mib_chain_get(MIB_KEYWD_FILTER_TBL, index, (void *)&Entry))
						continue;
					fprintf(fp,"<chain chainName=\"KEYWD_FILTER_TBL\">\n");
					fprintf(fp,"<Value Name=\"KEYWD\" Value=\"%s\"/>\n",Entry.keyword);
					fprintf(fp,"</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"KEYWD_FILTER_TBL\">\n");
				fprintf(fp,"</chain>\n");
			}
		}
#endif
		//alex_huang for url allow
#ifdef  URL_ALLOWING_SUPPORT
	        if(chainInfo.id == MIB_URL_ALLOW_FQDN_TBL){
			MIB_CE_URL_ALLOW_FQDN_T Entry;
			entryNum = mib_chain_total(MIB_URL_ALLOW_FQDN_TBL);
			if(entryNum>0){
				for(index=0;index<entryNum;index++){
					if (!mib_chain_get(MIB_URL_ALLOW_FQDN_TBL, index, (void *)&Entry))
						continue;
					fprintf(fp,"<chain chainName=\"URL_ALLOW_FQDN_TBL\">\n");
					fprintf(fp,"<Value Name=\"URLALW\" Value=\"%s\"/>\n",Entry.fqdn);
					fprintf(fp,"</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"URL_ALLOW_FQDN_TBL\">\n");
				fprintf(fp,"</chain>\n");
			}
		}
#endif

#ifdef DOMAIN_BLOCKING_SUPPORT
		if(chainInfo.id == MIB_DOMAIN_BLOCKING_TBL){
			MIB_CE_DOMAIN_BLOCKING_T Entry;
			entryNum = mib_chain_total(MIB_DOMAIN_BLOCKING_TBL);
			if(entryNum>0){
				for(index=0;index<entryNum;index++){
					if (!mib_chain_get(MIB_DOMAIN_BLOCKING_TBL, index, (void *)&Entry))
						continue;
					fprintf(fp,"<chain chainName=\"DOMAIN_BLOCKING_TBL\">\n");
					fprintf(fp,"<Value Name=\"DOMAIN\" Value=\"%s\"/>\n",Entry.domain);
					fprintf(fp,"</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"DOMAIN_BLOCKING_TBL\">\n");
				fprintf(fp,"</chain>\n");
			}
		}
#endif
#ifdef CONFIG_USER_DDNS
		if(chainInfo.id == MIB_DDNS_TBL){
			MIB_CE_DDNS_T Entry;
			entryNum = mib_chain_total(MIB_DDNS_TBL);
			if(entryNum>0){
				for(index=0;index<entryNum;index++){
					if (!mib_chain_get(MIB_DDNS_TBL, index, (void *)&Entry))
						continue;
					fprintf(fp,"<chain chainName=\"MIB_DDNS_TBL\">\n");
					fprintf(fp,"<Value Name=\"Provider\" Value=\"%s\"/>\n",Entry.provider);
					fprintf(fp,"<Value Name=\"Hostname\" Value=\"%s\"/>\n",Entry.hostname);
					//fprintf(fp,"<Value Name=\"Interface\" Value=\"%s\"/>\n",Entry.interface);
					fprintf(fp,"<Value Name=\"Username\" Value=\"%s\"/>\n",Entry.username);
					fprintf(fp,"<Value Name=\"Password\" Value=\"%s\"/>\n",Entry.password);
					fprintf(fp,"<Value Name=\"Enabled\" Value=\"%d\"/>\n",Entry.Enabled);
					fprintf(fp,"</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"MIB_DDNS_TBL\">\n");
				fprintf(fp,"</chain>\n");
			}
		}
#endif
		//JIMLUO : VIRTUAL SERVER TBL jim
		//MIB_VIRTUAL_SVR_TBL
#ifdef ZTE_GENERAL_ROUTER_SC
		if(chainInfo.id == MIB_VIRTUAL_SVR_TBL){
			MIB_CE_VTL_SVR_T Entry;
			entryNum = mib_chain_total(MIB_VIRTUAL_SVR_TBL);
			if(entryNum>0){
				for(index=0;index<entryNum;index++){
					if (!mib_chain_get(MIB_VIRTUAL_SVR_TBL, index, (void *)&Entry))
						continue;
					fprintf(fp,"<chain chainName=\"VIRTUAL_SVR_TBL\">\n");
					fprintf(fp,"<Value Name=\"ServerIP\" Value=\"%d.%d.%d.%d\"/>\n", Entry.svrIpAddr[0],Entry.svrIpAddr[1],Entry.svrIpAddr[2],Entry.svrIpAddr[3]);
					fprintf(fp,"<Value Name=\"ServerName\" Value=\"%s\"/>\n", Entry.svrName);
					fprintf(fp,"<Value Name=\"WanStartPort\" Value=\"%d\"/>\n", Entry.wanStartPort);
					fprintf(fp,"<Value Name=\"WanEndPort\" Value=\"%d\"/>\n",
						Entry.wanEndPort);
					fprintf(fp,"<Value Name=\"SvrStartPort\" Value=\"%d\"/>\n", Entry.svrStartPort);
					fprintf(fp,"<Value Name=\"SvrEndPort\" Value=\"%d\"/>\n",
						Entry.svrEndPort);
					fprintf(fp,"<Value Name=\"ProtoType\" Value=\"%d\"/>\n", Entry.protoType);

					fprintf(fp,"</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"VIRTUAL_SVR_TBL\">\n");
				fprintf(fp,"</chain>\n");
			}
		}
#endif		//ZTE_GENERAL_ROUTER_SC

#ifdef LAYER7_FILTER_SUPPORT
//star: for app software filter
		if(chainInfo.id == MIB_LAYER7_FILTER_TBL){
			LAYER7_FILTER_T Entry;
			entryNum = mib_chain_total(MIB_LAYER7_FILTER_TBL);
			if(entryNum>0){
				for(index=0;index<entryNum;index++){
					if (!mib_chain_get(MIB_LAYER7_FILTER_TBL, index, (void *)&Entry))
						continue;
					fprintf(fp,"<chain chainName=\"LAYER7_FILTER_TBL\">\n");
					fprintf(fp,"<Value Name=\"Appname\" Value=\"%s\"/>\n",Entry.appname);
					fprintf(fp,"</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"LAYER7_FILTER_TBL\">\n");
				fprintf(fp,"</chain>\n");
			}
		}
#endif

#ifdef ACCOUNT_CONFIG
		//ACCOUNT_CONFIG_TBL
		if (chainInfo.id == MIB_ACCOUNT_CONFIG_TBL) {
			MIB_CE_ACCOUNT_CONFIG_T Entry;
			entryNum = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL);
			if (entryNum > 0) {
				for (index = 0; index < entryNum; index++) {
					if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, index, (void *)&Entry))
						continue;
					fprintf(fp,"<chain chainName=\"ACCOUNT_CONFIG_TBL\">\n");
					fprintf(fp,"<Value Name=\"UserName\" Value=\"%s\"/>\n", Entry.userName);
					fprintf(fp,"<Value Name=\"Password\" Value=\"%s\"/>\n", Entry.userPassword);
					fprintf(fp,"<Value Name=\"Privilege\" Value=\"%d\"/>\n", Entry.privilege);
					fprintf(fp,"</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"ACCOUNT_CONFIG_TBL\">\n");
				fprintf(fp,"</chain>\n");
			}
		}
#endif

#if defined(AUTO_PVC_SEARCH_PURE_OAMPING) || defined(AUTO_PVC_SEARCH_AUTOHUNT)
		//AUTO_PVC_SEARCH_TBL
		if(chainInfo.id == MIB_AUTO_PVC_SEARCH_TBL){
			MIB_AUTO_PVC_SEARCH_T Entry;
			unsigned char mask[IP_ADDR_LEN];
			entryNum = mib_chain_total(MIB_AUTO_PVC_SEARCH_TBL);
			if(entryNum>0){
				for(index=0;index<entryNum;index++){
					if (!mib_chain_get(MIB_AUTO_PVC_SEARCH_TBL, index, (void *)&Entry))
						continue;
					fprintf(fp,"<chain chainName=\"AUTO_PVC_SEARCH_TBL\">\n");
					fprintf(fp,"<Value Name=\"VPI\" Value=\"%d\"/>\n",Entry.vpi);
					fprintf(fp,"<Value Name=\"VCI\" Value=\"%d\"/>\n",Entry.vci);
					fprintf(fp,"</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"AUTO_PVC_SEARCH_TBL\">\n");
				fprintf(fp,"</chain>\n");
			}
		}
#endif

#ifdef PORT_FORWARD_ADVANCE
		//PORT_FW_ADVANCE_TBL
		if(chainInfo.id == MIB_PFW_ADVANCE_TBL){
			MIB_CE_PORT_FW_ADVANCE_T Entry;
			unsigned char mask[IP_ADDR_LEN];
			entryNum = mib_chain_total(MIB_PFW_ADVANCE_TBL);
			if(entryNum>0){
				for(index=0;index<entryNum;index++){
					if (!mib_chain_get(MIB_PFW_ADVANCE_TBL, index, (void *)&Entry))
						continue;
					fprintf(fp,"<chain chainName=\"PORT_FW_ADVANCE_TBL\">\n");
					fprintf(fp,"<Value Name=\"IP\" Value=\"%s\"/>\n",inet_ntoa(*((struct in_addr *)Entry.ipAddr)));
					fprintf(fp,"<Value Name=\"OutInf\" Value=\"0x%x\"/>\n",Entry.ifIndex );
					fprintf(fp,"<Value Name=\"Gategory\" Value=\"%d\"/>\n",Entry.gategory);
					fprintf(fp,"<Value Name=\"Rule\" Value=\"%d\"/>\n",Entry.rule);
					fprintf(fp,"</chain>\n");
				}
			} else {
				fprintf(fp,"<chain chainName=\"PORT_FW_ADVANCE_TBL\">\n");
				fprintf(fp,"</chain>\n");
			}
		}
#endif

	}
//	fprintf(fp,"</Config_Information_File_8671>\n");
//	fprintf(fp,"%s\n", CONFIG_TRAILER);

	fclose(fp);
	return 1;
}

int main(int argc, char **argv){
	#if 0
	if ( mib_init() == 0 ) {
		printf("[Config] Initialize MIB failed!\n");
		return error;
	}
	#endif
	if(create_config_file()==error){
		printf("create config file error!!!!\n");
	}
	else
		return 0;

	return 1;
}
