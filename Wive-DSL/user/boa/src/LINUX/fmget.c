/*
 *      Web server handler routines for get info and index (getinfo(), getindex())
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *      Authors: Dick Tam	<dicktam@realtek.com.tw>
 *
 */

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/sysinfo.h>
#ifdef EMBED
#include <linux/config.h>
#include <config/autoconf.h>
#else
#include "../../../../uClibc/include/linux/autoconf.h"
#include "../../../../config/autoconf.h"
#endif

#include "../webs.h"
#include "mib.h"
#include "adsl_drv.h"
#include "utility.h"
//added by xl_yue
#include "../defs.h"

// remote config status flag: 0: disabled, 1: enabled
int g_remoteConfig=0;
int g_remoteAccessPort=51003;

// Added by Mason Yu
extern char suName[MAX_NAME_LEN];
extern char usName[MAX_NAME_LEN];
// Mason Yu on True
extern unsigned char g_login_username[MAX_NAME_LEN];

#ifdef WLAN_SUPPORT
static void translate_control_code(char *buffer)
{
	char tmpBuf[200], *p1 = buffer, *p2 = tmpBuf;


	while (*p1) {
		if (*p1 == '"') {
			memcpy(p2, "&quot;", 6);
			p2 += 6;
		}
		else
			*p2++ = *p1;
		p1++;
	}
	*p2 = '\0';

	strcpy(buffer, tmpBuf);
}
#endif

// Kaohj
typedef enum {
	INFO_MIB,
	INFO_SYS
} INFO_T;

typedef struct {
	char *cmd;
	INFO_T type;
	int id;
} web_get_cmd;

typedef struct {
	char *cmd;
	int (*handler)(int , webs_t , int , char_t **, char *);
} web_custome_cmd;

web_get_cmd get_info_list[] = {
	{"lan-ip", INFO_MIB, MIB_ADSL_LAN_IP},
	{"lan-subnet", INFO_MIB, MIB_ADSL_LAN_SUBNET},
	{"lan-ip2", INFO_MIB, MIB_ADSL_LAN_IP2},
	{"lan-subnet2", INFO_MIB, MIB_ADSL_LAN_SUBNET2},
	// Kaohj
	#ifndef DHCPS_POOL_COMPLETE_IP
	{"lan-dhcpRangeStart", INFO_MIB, MIB_ADSL_LAN_CLIENT_START},
	{"lan-dhcpRangeEnd", INFO_MIB, MIB_ADSL_LAN_CLIENT_END},
	#else
	{"lan-dhcpRangeStart", INFO_MIB, MIB_DHCP_POOL_START},
	{"lan-dhcpRangeEnd", INFO_MIB, MIB_DHCP_POOL_END},
	#endif
	{"lan-dhcpSubnetMask", INFO_MIB, MIB_DHCP_SUBNET_MASK},
	{"dhcps-dns1", INFO_MIB, MIB_DHCPS_DNS1},
	{"dhcps-dns2", INFO_MIB, MIB_DHCPS_DNS2},
	{"dhcps-dns3", INFO_MIB, MIB_DHCPS_DNS3},
	{"lan-dhcpLTime", INFO_MIB, MIB_ADSL_LAN_DHCP_LEASE},
	{"lan-dhcpDName", INFO_MIB, MIB_ADSL_LAN_DHCP_DOMAIN},
	{"elan-Mac", INFO_MIB, MIB_ELAN_MAC_ADDR},
	{"wlan-Mac", INFO_MIB, MIB_WLAN_MAC_ADDR},
	{"wan-dns1", INFO_MIB, MIB_ADSL_WAN_DNS1},
	{"wan-dns2", INFO_MIB, MIB_ADSL_WAN_DNS2},
	{"wan-dns3", INFO_MIB, MIB_ADSL_WAN_DNS3},
	{"wan-dhcps", INFO_MIB, MIB_ADSL_WAN_DHCPS},
	{"dmzHost", INFO_MIB, MIB_DMZ_IP},
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
	{"snmpSysDescr", INFO_MIB, MIB_SNMP_SYS_DESCR},
	{"snmpSysContact", INFO_MIB, MIB_SNMP_SYS_CONTACT},	
	{"snmpSysLocation", INFO_MIB, MIB_SNMP_SYS_LOCATION},
	{"snmpSysObjectID", INFO_MIB, MIB_SNMP_SYS_OID},
	{"snmpTrapIpAddr", INFO_MIB, MIB_SNMP_TRAP_IP},
	{"snmpCommunityRO", INFO_MIB, MIB_SNMP_COMM_RO},
	{"snmpCommunityRW", INFO_MIB, MIB_SNMP_COMM_RW},
	{"name", INFO_MIB, MIB_SNMP_SYS_NAME},
#endif
	{"snmpSysName", INFO_MIB, MIB_SNMP_SYS_NAME},
	{"name", INFO_MIB, MIB_SNMP_SYS_NAME},
#if defined(ZTE_GENERAL_ROUTER_SC) || defined(TIME_ZONE)
	{"ntpTimeZone", INFO_MIB, MIB_NTP_TIMEZONE},
	{"ntpServerIp1", INFO_MIB, MIB_NTP_SERVER_IP1},
	{"ntpServerIp2", INFO_MIB, MIB_NTP_SERVER_IP2},
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
//#ifdef _PRMT_WT107_
	{"ntpServerHost1", INFO_MIB, MIB_NTP_SERVER_HOST1},
	{"ntpServerHost2", INFO_MIB, MIB_NTP_SERVER_HOST2},
//#endif
/*ping_zhang:20081217 END*/
#endif
#ifdef ZTE_GENERAL_ROUTER_SC
	{"ntpServerEbl", INFO_MIB, MIB_NTP_ENABLED},
	{"ntpServerId", INFO_MIB, MIB_NTP_SERVER_ID},
#endif
	{"uptime", INFO_SYS, SYS_UPTIME},
	{"date", INFO_SYS, SYS_DATE},
	{"year", INFO_SYS, SYS_YEAR},
	{"month", INFO_SYS, SYS_MONTH},
	{"day", INFO_SYS, SYS_DAY},
	{"hour", INFO_SYS, SYS_HOUR},
	{"minute", INFO_SYS, SYS_MINUTE},
	{"second", INFO_SYS, SYS_SECOND},
	{"fwVersion", INFO_SYS, SYS_FWVERSION},
        {"VendorString", INFO_SYS, SYS_VENDOR},
        {"ModelString", INFO_SYS, SYS_MODEL},
        {"VersionString", INFO_SYS, SYS_VERSION},
    	{"dhcplan-ip", INFO_SYS, SYS_DHCP_LAN_IP},
	{"dhcplan-subnet", INFO_SYS, SYS_DHCP_LAN_SUBNET},
	{"dslstate", INFO_SYS, SYS_DSL_OPSTATE},
	{"bridge-ageingTime", INFO_MIB, MIB_BRCTL_AGEINGTIME},
#ifdef CONFIG_USER_IGMPPROXY
	{"igmp-proxy-itf", INFO_MIB, MIB_IGMP_PROXY_ITF},
#endif
//#ifdef CONFIG_USER_UPNPD	
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
	{"upnp-ext-itf", INFO_MIB, MIB_UPNP_EXT_ITF},
#endif	
#ifdef AUTO_PROVISIONING
	{"http-ip", INFO_MIB, MIB_HTTP_SERVER_IP},
#endif
#ifdef IP_PASSTHROUGH
	{"ippt-itf", INFO_MIB, MIB_IPPT_ITF},
	{"ippt-lease", INFO_MIB, MIB_IPPT_LEASE},
	{"ippt-lanacc", INFO_MIB, MIB_IPPT_LANACC},
#endif
#ifdef WLAN_SUPPORT
	{"ssid", INFO_MIB, MIB_WLAN_SSID},
	{"channel", INFO_MIB, MIB_WLAN_CHAN_NUM},
	{"fragThreshold", INFO_MIB, MIB_WLAN_FRAG_THRESHOLD},
	{"rtsThreshold", INFO_MIB, MIB_WLAN_RTS_THRESHOLD},
	{"beaconInterval", INFO_MIB, MIB_WLAN_BEACON_INTERVAL},
	{"wlanDisabled",INFO_MIB,MIB_WLAN_DISABLED},

	{"pskValue", INFO_SYS, SYS_WLAN_PSKVAL},
#ifdef WLAN_1x
	{"rsPort",INFO_MIB,MIB_WLAN_RS_PORT},
	{"rsIp",INFO_MIB,MIB_WLAN_RS_IP},
	{"rsPassword",INFO_MIB,MIB_WLAN_RS_PASSWORD},
	{"enable1X", INFO_MIB, MIB_WLAN_ENABLE_1X},
#endif
	{"wlanMode",INFO_MIB,MIB_WLAN_MODE},
	{"encrypt",INFO_MIB,MIB_WLAN_ENCRYPT},
	{"wpaAuth",INFO_MIB,MIB_WLAN_WPA_AUTH},
	{"networkType",INFO_MIB,MIB_WLAN_NETWORK_TYPE},

#ifdef CONFIG_WIFI_SIMPLE_CONFIG // WPS
	{"wscDisable",INFO_MIB,MIB_WSC_DISABLE},
	//{"wscConfig",INFO_MIB,MIB_WSC_CONFIGURED},
	{"wps_auth",INFO_MIB,MIB_WSC_AUTH},
	{"wps_enc",INFO_MIB,MIB_WSC_ENC},
	{"wscLoocalPin", INFO_MIB, MIB_WSC_PIN},

#endif

#ifdef WLAN_WDS
	{"wlanWdsEnabled",INFO_MIB,MIB_WLAN_WDS_ENABLED},
#endif
#endif
	//{ "dnsServer", INFO_SYS, SYS_DNS_SERVER},
	{"maxmsglen",INFO_MIB,MIB_MAXLOGLEN},
#ifdef _CWMP_MIB_
	{"acs-url", INFO_MIB, CWMP_ACS_URL},
	{"acs-username", INFO_MIB, CWMP_ACS_USERNAME},
	{"acs-password", INFO_MIB, CWMP_ACS_PASSWORD},
	{"inform-interval", INFO_MIB, CWMP_INFORM_INTERVAL},
	{"conreq-name", INFO_MIB, CWMP_CONREQ_USERNAME},
	{"conreq-pw", INFO_MIB, CWMP_CONREQ_PASSWORD},
	{"cert-pw", INFO_MIB, CWMP_CERT_PASSWORD},
	{"conreq-path", INFO_MIB, CWMP_CONREQ_PATH},
	{"conreq-port", INFO_MIB, CWMP_CONREQ_PORT},
#endif
#ifdef DOS_SUPPORT
	{"syssynFlood", INFO_MIB, MIB_DOS_SYSSYN_FLOOD},
	{"sysfinFlood", INFO_MIB, MIB_DOS_SYSFIN_FLOOD},
	{"sysudpFlood", INFO_MIB, MIB_DOS_SYSUDP_FLOOD},
	{"sysicmpFlood", INFO_MIB, MIB_DOS_SYSICMP_FLOOD},
	{"pipsynFlood", INFO_MIB, MIB_DOS_PIPSYN_FLOOD},
	{"pipfinFlood", INFO_MIB, MIB_DOS_PIPFIN_FLOOD},
	{"pipudpFlood", INFO_MIB, MIB_DOS_PIPUDP_FLOOD},
	{"pipicmpFlood", INFO_MIB, MIB_DOS_PIPICMP_FLOOD},
	{"blockTime", INFO_MIB, MIB_DOS_BLOCK_TIME},
#endif
	{"lan-dhcp-gateway", INFO_MIB, MIB_ADSL_LAN_DHCP_GATEWAY},
#ifdef ADDRESS_MAPPING
#ifndef MULTI_ADDRESS_MAPPING
	{"local-s-ip", INFO_MIB, MIB_LOCAL_START_IP},
	{"local-e-ip", INFO_MIB, MIB_LOCAL_END_IP},
	{"global-s-ip", INFO_MIB, MIB_GLOBAL_START_IP},
	{"global-e-ip", INFO_MIB, MIB_GLOBAL_END_IP},
#endif //!MULTI_ADDRESS_MAPPING	
#endif
#ifdef SYSLOG_SUPPORT
	{"log-level", INFO_MIB, MIB_SYSLOG_LOG_LEVEL},
	{"display-level", INFO_MIB, MIB_SYSLOG_DISPLAY_LEVEL},
#ifdef SYSLOG_REMOTE_LOG
	{"syslog-mode", INFO_MIB, MIB_SYSLOG_MODE},
	{"syslog-server-ip", INFO_MIB, MIB_SYSLOG_SERVER_IP},
	{"syslog-server-port", INFO_MIB, MIB_SYSLOG_SERVER_PORT},
#endif
#ifdef SEND_LOG
	{"log-server-ip", INFO_MIB, MIB_LOG_SERVER_IP},
	{"log-server-username", INFO_MIB, MIB_LOG_SERVER_NAME},
#endif
#endif
#ifdef TCP_UDP_CONN_LIMIT
	{"connLimit-tcp", INFO_MIB, MIB_CONNLIMIT_TCP},
	{"connLimit-udp", INFO_MIB, MIB_CONNLIMIT_UDP},
#endif 
#ifdef WEB_REDIRECT_BY_MAC
	{"landing-page-time", INFO_MIB, MIB_WEB_REDIR_BY_MAC_INTERVAL},
#endif
	{"super-user", INFO_MIB, MIB_SUSER_NAME},
	{"normal-user", INFO_MIB, MIB_USER_NAME},
#ifdef DEFAULT_GATEWAY_V2
	{"wan-default-gateway", INFO_MIB, MIB_ADSL_WAN_DGW_IP},
	{"itf-default-gateway", INFO_MIB, MIB_ADSL_WAN_DGW_ITF},
#endif
	{NULL, 0, 0}
};

#ifdef WLAN_SUPPORT
#ifdef CONFIG_WIFI_SIMPLE_CONFIG//WPS
static void convert_bin_to_str(unsigned char *bin, int len, char *out)
{
	int i;
	char tmpbuf[10];

	out[0] = '\0';

	for (i=0; i<len; i++) {
		sprintf(tmpbuf, "%02x", bin[i]);
		strcat(out, tmpbuf);
	}
}


static int fnget_wpsKey(int eid, webs_t wp, int argc, char_t **argv, char *buffer) {
	unsigned char id, vChar, type;

	mib_get(MIB_WSC_ENC, (void *)&vChar);
	buffer[0]='\0';
	if (vChar == WSC_ENCRYPT_WEP) {
		unsigned char tmp[100];
		mib_get(MIB_WLAN_WEP, (void *)&vChar);
		mib_get(MIB_WLAN_WEP_KEY_TYPE, (void *)&type);
		mib_get(MIB_WLAN_WEP_DEFAULT_KEY, (void *)&id);
		if (vChar == 1) {
			if (id == 0)				
				id = MIB_WLAN_WEP64_KEY1;
			else if (id == 1)				
				id = MIB_WLAN_WEP64_KEY2;
			else if (id == 2)				
				id = MIB_WLAN_WEP64_KEY3;
			else	
				id = MIB_WLAN_WEP64_KEY4;				
			mib_get(id, (void *)tmp);
			if(type == KEY_ASCII){
				memcpy(buffer, tmp, 5);
				buffer[5] = '\0';
			}else{				
				convert_bin_to_str(tmp, 5, buffer);
				buffer[10] = '\0';
			}
		}
		else {
			if (id == 0)				
				id = MIB_WLAN_WEP128_KEY1;
			else if (id == 1)				
				id = MIB_WLAN_WEP128_KEY2;
			else if (id == 2)				
				id = MIB_WLAN_WEP128_KEY3;
			else	
				id = MIB_WLAN_WEP128_KEY4;				
			mib_get(id, (void *)tmp);
			if(type == KEY_ASCII){
				memcpy(buffer, tmp, 13);
				buffer[13] = '\0';	
			}else{				
				convert_bin_to_str(tmp, 13, buffer);
				buffer[26] = '\0';	
			}			
		}			
	}
	else {
		if (vChar ==0 || vChar == WSC_ENCRYPT_NONE)
			strcpy(buffer, "N/A");
		else
			mib_get(MIB_WSC_PSK, (void *)buffer);
	}
   	return websWrite(wp, buffer);
}
#endif
#endif

web_custome_cmd get_info_custom_list[] = {
	#ifdef WLAN_SUPPORT
	#ifdef CONFIG_WIFI_SIMPLE_CONFIG//WPS
	{ "wps_key", fnget_wpsKey },
	#endif
	#endif
	{ NULL, 0 }
};

int getInfo(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t	*name;
	unsigned char buffer[256+1];
	int idx, ret;
	
   	if (ejArgs(argc, argv, T("%s"), &name) < 1) {
   		websError(wp, 400, T("Insufficient args\n"));
   		return -1;
   	}

	memset(buffer,0x00,64);
	
 	if ( !strncmp(name, T("adsl-drv-"), 9) ) {
 		getAdslDrvInfo(&name[9], buffer, 64);
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		idx=strlen(buffer);
		if(buffer[idx-1]=='.')
			buffer[idx-1]='\0';
#endif
		return websWrite(wp, T("%s"), buffer);
	}
	/*+++++add by Jack for VoIP project 20/03/07+++++*/
#ifdef VOIP_SUPPORT
	else if(!strncmp(name, T("voip_"), 5)){
		return asp_voip_getInfo(eid, wp, argc, argv);
	}
#endif /*VOIP_SUPPORT*/
	if(!strcmp(name, T("login-user"))){
#ifdef USE_LOGINWEB_OF_SERVER
		ret = websWrite(wp, T("%s"), g_login_username);
#else
		ret = websWrite(wp, T("%s"), wp->user);
#endif
		goto NEXTSTEP;
	}
#ifdef ZTE_GENERAL_ROUTER_SC
	if ( !strcmp(name, T("dhcp-day")) ||
		!strcmp(name, T("dhcp-hour")) ||
		!strcmp(name, T("dhcp-min")) ) {
		int time;
		int day,hour,minute;

		mib_get(MIB_ADSL_LAN_DHCP_LEASE, (void*)&time);
		if(time == -1){
			ret = websWrite(wp, T("%s"), "-1");
		}
		else{
			if ( !strcmp(name, T("dhcp-day")) ) {
				day = time/86400;
				store_day = day;
				ret = websWrite(wp, T("%d"), day);
			} else if ( !strcmp(name, T("dhcp-hour")) ) {
				hour = (time%86400)/3600;
				store_hour = hour;
				ret = websWrite(wp, T("%d"), hour);
			} else {
				minute = ((time%86400)%3600)/60;
				store_minute = minute;
				ret = websWrite(wp, T("%d"), minute);
			}
		}
		goto NEXTSTEP;
	}
#endif
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	if (!strcmp(name, T("interval"))) {
		unsigned short time;
		mib_get(MIB_REFRESH_TIME, &time);
		ret = websWrite(wp, T("%d"), time);
		goto NEXTSTEP;
	}

	if ( !strcmp(name, T("pvcExist")) ) {
		unsigned int totalEntry;
		int i, cnt=0, selected;
		unsigned int ifMap=0;	// high half for PPP bitmap, low half for vc bitmap
		MIB_CE_ATM_VC_T Entry;

		totalEntry = mib_chain_total(MIB_ATM_VC_TBL);

		selected = -1;
		for (i=0; i<totalEntry; i++) {
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) {
				websError(wp, 400, T(Tget_chain_err));
				return;
			}
			if ( (selected == -1) && i==0 )
				selected = i;
			else {
				ifMap |= 1 << (Entry.ifIndex & 0x0f);	// vc map
				ifMap |= (1 << 16) << ((Entry.ifIndex >> 4) & 0x0f);	// PPP map
			}
			
			if (Entry.vpi == fcEntry.vpi && Entry.vci == fcEntry.vci) 
				cnt++;
		}
#ifdef ZTE_531B_BRIDGE_SC
		if (cnt > 0)
			ret=websWrite(wp, T("%s"), "1");
		else if (totalEntry >= 8)
			ret=websWrite(wp, T("%s"), "3");
		else
			ret=websWrite(wp, T("%s"), "0");
		goto NEXTSTEP;
#else
		if(cnt > 0) {
			//Make sure there is no mismatch mode
			for (i=0, cnt=0; i<totalEntry; i++) {
				if(i==selected)
					continue;
				if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)){
		  			websError(wp, 400, T(Tget_chain_err));
					return;
				}
				if (Entry.vpi == fcEntry.vpi && Entry.vci == fcEntry.vci){
					cnt++;
					if(Entry.cmode != fcEntry.cmode) {
						ret = websWrite(wp, T("%s"), "1");
						goto NEXTSTEP;
					}
				}
			}
			//Max. 5 PPPoE connections
			if(fcEntry.cmode == ADSL_PPPoE && cnt==MAX_POE_PER_VC) {	// Jenny, multisession PPPoE support
				ret = websWrite(wp, T("%s"), "2");
				goto NEXTSTEP;
			//Max. 1 connection except PPPoE
			} else if(fcEntry.cmode != ADSL_PPPoE && cnt>0 ) {
				ret = websWrite(wp, T("%s"), "1");
				goto NEXTSTEP;
			} else if (totalEntry >= 8) {
				ret = websWrite(wp, T("%s"), "3");
				goto NEXTSTEP;
			}
		}else if (totalEntry >= 8)
			ret=websWrite(wp, T("%s"), "3");
		else
			ret=websWrite(wp, T("%s"), "0");
		goto NEXTSTEP;
#endif
		
	}
#endif

 	for (idx=0; get_info_custom_list[idx].cmd != NULL; idx++) {
 		if (!strcmp(name, get_info_custom_list[idx].cmd)) {
 			return get_info_custom_list[idx].handler(eid, wp, argc, argv, buffer);
 		}
 	}
	
	for (idx=0; get_info_list[idx].cmd != NULL; idx++) {
		if (!strcmp(name, get_info_list[idx].cmd)) {
			if (get_info_list[idx].type == INFO_MIB) {
				if (getMIB2Str(get_info_list[idx].id, buffer)) {
					fprintf(stderr, "failed to get %s\n", name);
					return -1;
				}
			}
			else {
				if (getSYS2Str(get_info_list[idx].id, buffer))
					return -1;
			}
	#ifdef ZTE_GENERAL_ROUTER_SC
			//jim :2007-06-21  if no IP set, 0.0.0.0 will be shown on webpages to tell user how to setting ip values, 
			//				according ZTE's advise.
			if((!strncmp(name, "wan-dns", 7))&&(*buffer==0))
			{
				ret=websWrite(wp, T("0.0.0.0"));
			}
			else
			{
				ret=websWrite(wp, T("%s"), buffer);
			}
	#else
			// Kaohj
			if ((!strncmp(name, "wan-dns", 7))&& !strcmp(buffer, "0.0.0.0"))
				ret = websWrite(wp, T(""));
			else
			ret = websWrite(wp, T("%s"), buffer);
	#endif
			//fprintf(stderr, "%s = %s\n", name, buffer);
			//printf("%s = %s\n", name, buffer);
			break;
		}
	}

NEXTSTEP:
	return ret;
}

#if 0
int getInfo(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t	*name;
	unsigned short vShort;
	unsigned char get_fail_buf[] = "Unknown";	
	unsigned char buffer[255];
	int updays, uphours, upminutes;
	struct sysinfo info;
	time_t tm;
	struct tm tm_time;
	
   	if (ejArgs(argc, argv, T("%s"), &name) < 1) {
   		websError(wp, 400, T("Insufficient args\n"));
   		return -1;
   	}

	memset(buffer,0x00,255);
	
	if ( !strcmp(name, T("lan-ip"))) {
		if(!mib_get( MIB_ADSL_LAN_IP,  (void *)buffer))
			return -1;

   		return websWrite(wp, T("%s"), inet_ntoa(*((struct in_addr *)buffer)));
	}
   	else if ( !strcmp(name, T("lan-subnet"))) {
		if(!mib_get( MIB_ADSL_LAN_SUBNET,  (void *)buffer))
			return -1;
		
   		return websWrite(wp, T("%s"), inet_ntoa(*((struct in_addr *)buffer)));
	}
   	else if ( !strcmp(name, T("lan-dhcpRangeStart"))) {
		if(!mib_get( MIB_ADSL_LAN_CLIENT_START,  (void *)buffer))
			return -1;
   		return websWrite(wp, T("%u"), *(unsigned char *)buffer);
	}
   	else if ( !strcmp(name, T("lan-dhcpRangeEnd"))) {
		if(!mib_get( MIB_ADSL_LAN_CLIENT_END,  (void *)buffer))
			return -1;
   		return websWrite(wp, T("%u"), *(unsigned char *)buffer);
	}
   	else if ( !strcmp(name, T("lan-dhcpLTime"))) {
		if(!mib_get( MIB_ADSL_LAN_DHCP_LEASE,  (void *)buffer))
			return -1;
   		return websWrite(wp, T("%u"), *(unsigned int *)buffer);
	}
   	else if ( !strcmp(name, T("lan-dhcpDName"))) {
		if(!mib_get( MIB_ADSL_LAN_DHCP_DOMAIN,  (void *)buffer))
			return -1;
   		return websWrite(wp, T("%s"), buffer);
	}
	else if ( !strcmp(name, T("elan-Mac"))) {
		if(!mib_get( MIB_ELAN_MAC_ADDR,  (void *)buffer))
			return -1;
		return websWrite(wp, T("%02x%02x%02x%02x%02x%02x"), buffer[0], buffer[1],
						buffer[2], buffer[3], buffer[4], buffer[5]);
	}
	else if ( !strcmp(name, T("wlan-Mac"))) {
		if(!mib_get( MIB_WLAN_MAC_ADDR,  (void *)buffer))
			return -1;
		return websWrite(wp, T("%02x%02x%02x%02x%02x%02x"), buffer[0], buffer[1],
						buffer[2], buffer[3], buffer[4], buffer[5]);
	}
  	else if ( !strcmp(name, T("wan-dns1"))) {
		if(!mib_get( MIB_ADSL_WAN_DNS1,  (void *)buffer))
			return -1;
		if (((struct in_addr *)buffer)->s_addr != 0)
   			return websWrite(wp, T("%s"), inet_ntoa(*((struct in_addr *)buffer)));
   		else
   			return websWrite(wp, T(""));
	}
	else if ( !strcmp(name, T("wan-dhcps"))) {
		if(!mib_get( MIB_ADSL_WAN_DHCPS,  (void *)buffer))
			return -1;
		if (((struct in_addr *)buffer)->s_addr != 0)
   			return websWrite(wp, T("%s"), inet_ntoa(*((struct in_addr *)buffer)));
   		else
   			return websWrite(wp, T(""));
	}	
  	else if ( !strcmp(name, T("wan-dns2"))) {
		if(!mib_get( MIB_ADSL_WAN_DNS2,  (void *)buffer))
			return -1;
		if (((struct in_addr *)buffer)->s_addr != 0)
	   		return websWrite(wp, T("%s"), inet_ntoa(*((struct in_addr *)buffer)));
   		else
   			return websWrite(wp, T(""));
	}
  	else if ( !strcmp(name, T("wan-dns3"))) {
		if(!mib_get( MIB_ADSL_WAN_DNS3,  (void *)buffer))
			return -1;
		if (((struct in_addr *)buffer)->s_addr != 0)
	   		return websWrite(wp, T("%s"), inet_ntoa(*((struct in_addr *)buffer)));
   		else
   			return websWrite(wp, T(""));
	}
	else if ( !strcmp(name, T("dmzHost"))) {
		if(!mib_get( MIB_DMZ_IP,  (void *)buffer))
			return -1;
   		return websWrite(wp, T("%s"), inet_ntoa(*((struct in_addr *)buffer)));
	}
 	else if ( !strcmp(name, T("snmpSysDescr")) ) {
		if(!mib_get( MIB_SNMP_SYS_DESCR,  (void *)buffer))
			return -1;
   		return websWrite(wp, T("\"%s\""), buffer);
	}
 	else if ( !strcmp(name, T("snmpSysContact")) ) {
		if(!mib_get( MIB_SNMP_SYS_CONTACT,  (void *)buffer))
			return -1;
   		return websWrite(wp, T("\"%s\""), buffer);
	}
 	else if ( !strcmp(name, T("snmpSysName")) ) {
		if(!mib_get( MIB_SNMP_SYS_NAME,  (void *)buffer))
			return -1;
   		return websWrite(wp, T("\"%s\""), buffer);
	}
 	else if ( !strcmp(name, T("snmpSysLocation")) ) {
		if(!mib_get( MIB_SNMP_SYS_LOCATION,  (void *)buffer))
			return -1;
   		return websWrite(wp, T("\"%s\""), buffer);
	}
 	else if ( !strcmp(name, T("snmpSysObjectID")) ) {
		if(!mib_get( MIB_SNMP_SYS_OID,  (void *)buffer))
			return -1;
   		return websWrite(wp, T("\"%s\""), buffer);
	}
 	else if ( !strcmp(name, T("snmpTrapIpAddr")) ) {
		if(!mib_get( MIB_SNMP_TRAP_IP,  (void *)buffer))
			return -1;
   		return websWrite(wp, T("%s"), inet_ntoa(*((struct in_addr *)buffer)));
	}
 	else if ( !strcmp(name, T("snmpCommunityRO")) ) {
		if(!mib_get( MIB_SNMP_COMM_RO,  (void *)buffer))
			return -1;
   		return websWrite(wp, T("\"%s\""), buffer);
	}
 	else if ( !strcmp(name, T("snmpCommunityRW")) ) {
		if(!mib_get( MIB_SNMP_COMM_RW,  (void *)buffer))
			return -1;
   		return websWrite(wp, T("\"%s\""), buffer);
	}
 	else if ( !strcmp(name, T("wanDhcp-current")) ) {
		return websWrite(wp, T("Under Construction"));
	}
 	else if ( !strcmp(name, T("name")) ) {
		if(!mib_get( MIB_SNMP_SYS_NAME,  (void *)buffer))
			return -1;
   		return websWrite(wp, buffer);
	}
 	else if ( !strcmp(name, T("uptime")) ) {
		sysinfo(&info);
		updays = (int) info.uptime / (60*60*24);
	 	if (updays)
	 		websWrite(wp, T("%d day%s, "), updays, (updays != 1) ? "s" : "");
		upminutes = (int) info.uptime / 60;
		uphours = (upminutes / 60) % 24;
		upminutes %= 60;
		if(uphours)
			return websWrite(wp, T("%2d:%02d"), uphours, upminutes);
		else
			return websWrite(wp, T("%d min"), upminutes);
	}
 	else if ( !strcmp(name, T("date")) ) {
 		time(&tm);
		memcpy(&tm_time, localtime(&tm), sizeof(tm_time));
		strftime(buffer, 200, "%a %b %e %H:%M:%S %Z %Y", &tm_time);
   		return websWrite(wp, buffer);
	}
 	else if ( !strcmp(name, T("fwVersion")) ) {
#ifdef EMBED
 		FILE *fp;
 		char strVer[128], tmp[128], *strBld;
 		strVer[0]=0;
		fp = fopen("/etc/version", "r");
 		if (fp!=NULL) {
 		    fgets(strVer, sizeof(strVer), fp);  //main version
 		    fclose(fp);
 		    strBld = strchr(strVer, ' ');
 		    *strBld=0;
 		    fp = fopen("/proc/version", "r");  
 		    if (fp!=NULL) {  //build number
 		    	int i;
 		        fgets(tmp, sizeof(tmp), fp);
 		        fclose(fp);
 		        strBld = strchr(tmp, '#');
 		        if (strBld!=NULL) {
 		            strBld++; i=0;
 		            while (strBld[i]!=' ') i++;
 		            strBld[i]=0;
 		        } else {
  		            strBld = tmp;
  		            tmp[0]=0;
  		        };
 		    }
 		};
		return websWrite(wp, T("%s.%s"), strVer, strBld);
#else
		return websWrite(wp, T("%s.%s"), "Acorp", "1");
#endif
	}
 	else if ( !strncmp(name, T("adsl-drv-"), 9) ) {
 		getAdslDrvInfo(&name[9], buffer	, 255);
		return websWrite(wp, T("%s"), buffer);
	}
   	else if ( !strcmp(name, T("bridge-ageingTime"))) {
		if(!mib_get( MIB_BRCTL_AGEINGTIME,  (void *)&vShort))
			return -1;
		sprintf(buffer, "%u", vShort);
   		return websWrite(wp, buffer);
	}
#ifdef CONFIG_USER_IGMPPROXY
   	else if ( !strcmp(name, T("igmp-proxy-itf"))) {
		if(!mib_get( MIB_IGMP_PROXY_ITF,  (void *)buffer))
			return -1;
   		return websWrite(wp, T("%u"), *(unsigned char *)buffer);
	}
#endif
   	else if ( !strcmp(name, T("ippt-itf"))) {
		if(!mib_get( MIB_IPPT_ITF,  (void *)buffer))
			return -1;
   		return websWrite(wp, T("%u"), *(unsigned char *)buffer);
	}
   	else if ( !strcmp(name, T("ippt-lease"))) {
		if(!mib_get( MIB_IPPT_LEASE,  (void *)buffer))
			return -1;
   		return websWrite(wp, T("%u"), *(unsigned int *)buffer);
	}
#ifdef WLAN_SUPPORT
   	else if ( !strcmp(name, T("ssid"))) {
		if ( !mib_get( MIB_WLAN_SSID,  (void *)buffer) )
			return -1;

		translate_control_code(buffer);
		
		return websWrite(wp, T("%s"), buffer);
	}
   	else if ( !strcmp(name, T("channel"))) {
		if ( !mib_get( MIB_WLAN_CHAN_NUM,  (void *)buffer) )
			return -1;
   		return websWrite(wp, T("%u"), *(unsigned char *)buffer);
	}
#ifdef WLAN_WPA
        else if ( !strcmp(name, T("wep"))) {
                ENCRYPT_T encrypt;
                if ( !mib_get( MIB_WLAN_ENCRYPT,  (void *)buffer) )
                        return -1;
                encrypt = (ENCRYPT_T)*buffer;
                if (encrypt == ENCRYPT_DISABLED)
                        strcpy( buffer, T("Disabled") );
                else if (encrypt == ENCRYPT_WPA)
                        strcpy( buffer, T("WPA") );
                else {
                        WEP_T wep;
                        if ( !mib_get( MIB_WLAN_WEP,  (void *)buffer) )
                                return -1;
                        wep = (WEP_T)*buffer; 
                        if ( wep == WEP_DISABLED )
                                strcpy( buffer, T("Disabled") );
                        else if ( wep == WEP64 )
                                strcpy( buffer, T("WEP 64bits") );
                        else if ( wep == WEP128)
                                strcpy( buffer, T("WEP 128bits") );
                }
                return websWrite(wp, buffer);
        }

#else
   	else if ( !strcmp(name, T("wep"))) {
   		WEP_T wep;
		if ( !mib_get( MIB_WLAN_WEP,  (void *)buffer) )
			return -1;
		
		wep = (WEP_T)*buffer; 
		if ( wep == WEP_DISABLED )
			strcpy( buffer, T("Disabled") );
		else if ( wep == WEP64 )
			strcpy( buffer, T("64bits") );
		else if ( wep == WEP128)
			strcpy( buffer, T("128bits") );
   		return websWrite(wp, buffer);
   	}
#endif

	/* Advance setting stuffs */
	else if ( !strcmp(name, T("fragThreshold"))) {
		if ( !mib_get( MIB_WLAN_FRAG_THRESHOLD, (void *)buffer) )
			return -1;
   		return websWrite(wp, T("%u"), *(unsigned short *)buffer);
	}
	else if ( !strcmp(name, T("rtsThreshold"))) {
		if ( !mib_get( MIB_WLAN_RTS_THRESHOLD, (void *)buffer) )
			return -1;
   		return websWrite(wp, T("%u"), *(unsigned short *)buffer);
	}
	else if ( !strcmp(name, T("beaconInterval"))) {
		if ( !mib_get( MIB_WLAN_BEACON_INTERVAL, (void *)buffer) )
			return -1;
   		return websWrite(wp, T("%u"), *(unsigned short *)buffer);
	}
	else if ( !strcmp(name, T("dtimPeriod"))) {
		if ( !mib_get( MIB_WLAN_DTIM_PERIOD, (void *)buffer) )
			return -1;
   		return websWrite(wp, T("%u"), *(unsigned char *)buffer);
	}

#ifdef WLAN_WPA
#if 0	// for RADIUS server
	else if ( !strcmp(name, T("rsIp"))) {
		if ( !mib_get( MIB_WLAN_RS_IP,  (void *)buffer) )
			return -1;
		if (!memcmp(buffer, "\x0\x0\x0\x0", 4))
			return websWrite(wp, T(""));
   		return websWrite(wp, T("%s"), inet_ntoa(*((struct in_addr *)buffer)) );
	}
	else if ( !strcmp(name, T("rsPort"))) {
		if ( !mib_get( MIB_WLAN_RS_PORT, (void *)buffer) )
			return -1;
   		return websWrite(wp, T("%u"), *(unsigned short *)buffer);
	}
 	else if ( !strcmp(name, T("rsPassword"))) {
		if ( !mib_get( MIB_WLAN_RS_PASSWORD,  (void *)buffer) )
			return -1;
  		return websWrite(wp, T("%s"), buffer);
	}
#endif
 	else if ( !strcmp(name, T("pskValue"))) {
		int i;
		buffer[0]='\0';
		if ( !mib_get(MIB_WLAN_WPA_PSK,  (void *)buffer) )
			return -1;
		for (i=0; i<strlen(buffer); i++)
			buffer[i]='*';
		buffer[i]='\0';
   		return websWrite(wp, buffer);
	}
#endif

#endif // of WLAN_SUPPORT

	return websWrite(wp, T("%s"), get_fail_buf );
}
#endif

int addMenuJavaScript( webs_t wp,int nums,int maxchildrensize)
{
#ifdef WEB_MENU_USE_NEW
	websWrite(wp,T("<script >\n"));
	int i=0;
	websWrite(wp,T("scores = new Array(%d);\n"),nums);
	for(i=0;i<nums;i++ )
		websWrite(wp,T("scores[%d]='Submenu%d';\n"),i,i);
	websWrite(wp,T("btns = new Array(%d);\n"),nums);
	for(i=0;i<nums;i++ )
		websWrite(wp,T("btns[%d]='Btn%d';\n"),i,i);
	websWrite(wp,T("\nfunction initIt()\n"
		"{\n\tdivColl = document.all.tags(\"div\");\n"
		"\tfor (i=0; i<divColl.length; i++)\n "
		"\t{\n\t\twhichEl = divColl[i];\n"
		"\t\tif (whichEl.className == \"Child\")\n"
		"\t\t\twhichEl.style.display = \"none\";\n\t}\n}\n\n"));
	websWrite(wp,T("function closeMenu(el)\n"
		"{\n"
		"\tfor(i=0;i<%d;i++)\n"
		"\t{\n\t\tfor(j=0;j<%d;j++)"
		"{\n\t\t\tif(scores[i]!=el)\n"
		"\t\t\t{\n\t\t\t\tid=scores[i]+\"Child\"+j.toString();\n"
		"\t\t\t\tif(document.getElementById(id))\n"
		"\t\t\t\t{\n\t\t\t\t\tdocument.getElementById(id).style.display = \"none\";\n"
		"\t\t\t\t\twhichEl = eval(scores[i] + \"Child\");\n"
		"\t\t\t\t\twhichEl.style.display = \"none\";\n"
		"\t\t\t\t\tdocument.getElementById(btns[i]).src =\"menu-images/menu_folder_closed.gif\";\n"
		"\t\t\t\t}\n\t\t\t}\n\t\t}\n\t}\n}\n\n"),nums, maxchildrensize);
	
	websWrite(wp,T("function expandMenu(el,imgs, num)\n"
		"{\n\tcloseMenu(el);\n"));
	websWrite(wp,T("\tif (num == 0) {\n\t\twhichEl1 = eval(el + \"Child\");\n"
		"\t\tfor(i=0;i<%d;i++)\n"),nums);
	websWrite(wp,T("\t\t{\n\t\t\twhichEl = eval(scores[i] + \"Child\");\n"
		"\t\t\tif(whichEl!=whichEl1)\n "                                   
		"\t\t\t{\n\t\t\t\twhichEl.style.display = \"none\";\n"
		"\t\t\t\tdocument.getElementById(btns[i]).src =\"menu-images/menu_folder_closed.gif\";\n"                                        
		"\t\t\t}\n\t\t}\n"));
	websWrite(wp,T("\t\twhichEl1 = eval(el + \"Child\");\n"
		"\t\tif (whichEl1.style.display == \"none\")\n "
		"\t\t{\n"
		"\t\t\twhichEl1.style.display = \"\";\n"
		"\t\t\tdocument.getElementById(imgs).src =\"menu-images/menu_folder_open.gif\";\n"
		"\t\t}\n\t\telse {\n\t\t\twhichEl1.style.display =\"none\";\n"
		"\t\t\tdocument.getElementById(imgs).src =\"menu-images/menu_folder_closed.gif\";\n"
		"\t\t}\n\t}\n\telse {\n"));
	websWrite(wp,T("\t\tfor(i=0;i<num;i++) {\n"
		"\t\t\tid = el + \"Child\"+i.toString();\n"
		"\t\t\twhichEl1 = document.getElementById(id);\n"
		"\t\t\tif (whichEl1) {\n"
		"\t\t\t\tif (whichEl1.style.display == \"none\")\n"
		"\t\t\t\t{\n"
		"\t\t\t\t\twhichEl1.style.display = \"\";\n"
		"\t\t\t\t\tdocument.getElementById(imgs).src =\"menu-images/menu_folder_open.gif\";\n"
		"\t\t\t\t}\n\t\t\t\telse {\n\t\t\t\t\twhichEl1.style.display =\"none\";\n"
		"\t\t\t\t\tdocument.getElementById(imgs).src =\"menu-images/menu_folder_closed.gif\";\n"
		"\t\t\t\t}\n\t\t\t}\n\t\t}\n\t}\n}\n</script>\n"));
	
	websWrite(wp,T("<style type=\"text/css\">\n"
		"\n.link {\n"
		"\tfont-family: arial, Helvetica, sans-serif, bold;\n\tfont-size:10pt;\n\twhite-space:nowrap;\n\tcolor: #000000;\n\ttext-decoration: none;\n}\n"
		"</style>"));
#else
	websWrite(wp,T("<script type=\"text/javascript\" src=\"/admin/mtmcode.js\">\n"
	"</script>\n"
	"\n"
	"<script type=\"text/javascript\">\n"
	"    // Morten's JavaScript Tree Menu\n"
	"    // version 2.3.2-macfriendly, dated 2002-06-10\n"
	"    // http://www.treemenu.com/\n"
	"\n"
	"    // Copyright (c) 2001-2002, Morten Wang & contributors\n"
	"    // All rights reserved.\n"
	"\n"
	"    // This software is released under the BSD License which should accompany\n"));
	websWrite(wp,T("    // it in the file \"COPYING\".  If you do not have this file you can access\n"
	"    // the license through the WWW at http://www.treemenu.com/license.txt\n"
	"\n"
	"    // Nearly all user-configurable options are set to their default values.\n"
	"    // Have a look at the section \"Setting options\" in the installation guide\n"
	"    // for description of each option and their possible values.\n"));
	websWrite(wp,T("\n"
	"MTMDefaultTarget = \"view\";\n"
	"\n"
	"/******************************************************************************\n"
	" * User-configurable list of icons.                                            *\n"
	" ******************************************************************************/\n"));
	websWrite(wp,T("\n"
	"var MTMIconList = null;\n"
	"MTMIconList = new IconList();\n"
	"MTMIconList.addIcon(new MTMIcon(\"menu_link_external.gif\", \"http://\", \"pre\"));\n"
	"MTMIconList.addIcon(new MTMIcon(\"menu_link_pdf.gif\", \".pdf\", \"post\"));\n"));
	websWrite(wp,T("\n"
	"/******************************************************************************\n"
	" * User-configurable menu.                                                     *\n"
	" ******************************************************************************/\n"));
	websWrite(wp,T("\n"
	"var menu = null;\n"
	"\n"
	"menu = new MTMenu();\n"));
#endif
}

// Kaohj
int checkWrite(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t *name;
	unsigned char vChar;
	unsigned short vUShort;
	unsigned int vUInt;
	
   	if (ejArgs(argc, argv, T("%s"), &name) < 1) {
   		websError(wp, 400, T("Insufficient args\n"));
   		return -1;
   	}
	if ( !strcmp(name, T("devType")) ) {
		if ( !mib_get( MIB_DEVICE_TYPE, (void *)&vChar) )
			return -1;
#ifdef EMBED
		if (0 == vChar)
			websWrite(wp, "disableTextField(document.adsl.adslConnectionMode);");
#endif
		return 0;
	}
#ifdef CONFIG_USER_ROUTED_ROUTED
	else if ( !strcmp(name, T("rip-on-0")) ) {
		if ( !mib_get( MIB_RIP_ENABLE, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("rip-on-1")) ) {
		if ( !mib_get( MIB_RIP_ENABLE, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			websWrite(wp, "checked");
		return 0;
	}
	if ( !strcmp(name, T("rip-ver")) ) {
		if ( !mib_get( MIB_RIP_VERSION, (void *)&vChar) )
			return -1;
		if (0==vChar) {
			websWrite(wp, "<option selected value=0>v1</option>\n");
			websWrite(wp, "\t<option value=1>v2</option>");
		} else {
			websWrite(wp, "<option value=0>v1</option>\n");
			websWrite(wp, "\t<option selected value=1>v2</option>\n");
		}
		return 0;
	}
#endif
#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
	if (!strcmp(name, T("ripEn")))
	{
#ifdef CONFIG_USER_ROUTED_ROUTED
		if (!mib_get(MIB_RIP_ENABLE, (void *)&vChar))
			return -1;
#else
		vChar = 0;
#endif
		if (1 == vChar)
			websWrite(wp, "1");
		else
			websWrite(wp, "0");
		
		return 0;
	}
	if (!strcmp(name, T("ospfEn")))
	{
		if (!mib_get(MIB_OSPF_ENABLE, (void *)&vChar))
			return -1;
		if (1 == vChar)
			websWrite(wp, "1");
		else
			websWrite(wp, "0");

		return 0;
	}
#endif
if (!strcmp (name, T ("SoftwareVersion")))
    {
     #if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
       websWrite (wp, T ("<tr bgcolor=\"#DDDDDD\">\n<td width=40%%><font size=2><b>软件版本</b></td>\n"));
       websWrite (wp,T(" <td width=60%%><font size=2>%s</td>\n</tr>\n"),ZTESOFTWAREVERSION);    
     #endif
    

      return 0;
    }

  if (!strcmp (name, T ("DspVersion")))
    {
      char buffer[256];
      getAdslDrvInfo ("version", buffer, 64);
      #if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
      websWrite (wp,  T("<tr bgcolor=\"#EEEEEE\">\n<td width=40%%><font size=2><b>DSP 版本</b></td>"
                  "<td width=60%%><font size=2>%s</td>\n </tr>"),
                 buffer);
     #else
	 websWrite (wp,  T("<tr bgcolor=\"#DDDDDD\">\n<td width=40%%><font size=2><b>DSP Version</b></td>"
                  "<td width=60%%><font size=2>%s</td>\n </tr>"),
                 buffer);
     #endif
     return 0;
    }

  if (!strcmp (name, T ("DnsServer")))
    {
  #ifdef ZTE_GENERAL_ROUTER_SC
      websWrite (wp, T("<tr bgcolor=\"#EEEEEE\">\n<td width=20%%><font size=2><b>DNS 服务器</b></td>"));  
  
      websWrite (wp, T ("<td width=80%% colspan=\"7\"><font size=2>"));
#else
   websWrite (wp,
                 T
                 ("<tr bgcolor=\"#EEEEEE\">\n<td width=20%%><font size=2><b>DNS Servers</b></td>"));
  websWrite (wp, T (" <td width=80%%><font size=2>"));
	#endif
      getNameServer (0, wp, 1, 0);
      websWrite (wp, T ("</td>\n</tr>"));
      return 0;
    }

  if (!strcmp (name, T ("DefaultGw")))
    {
   #ifdef ZTE_GENERAL_ROUTER_SC
      websWrite (wp,
                 T
                 ("<tr bgcolor=\"#DDDDDD\">\n <td width=20%% ><font size=2><b>缺省路由</b></td>"));
 #else
    websWrite (wp,
                 T
                 ("<tr bgcolor=\"#DDDDDD\">\n <td width=20%% ><font size=2><b>Default Gateway</b></td>"));
   #endif
#ifdef ZTE_GENERAL_ROUTER_SC
      websWrite (wp, T ("<td width=80%% colspan=\"7\"><font size=2>"));
#else
	  websWrite (wp, T ("<td width=80%% colspan=\"6\"><font size=2>"));
#endif
      getDefaultGW (0, wp, 1, 0);
      websWrite (wp, T ("</td> </tr>"));
 
      return 0;
    }

  if (!strcmp (name, T ("getDslSnr")))
    {
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
      websWrite (wp,
                 T
                 ("<tr bgcolor=\"#EEEEEE\">\n <td width=40%%><font size=2><b>上行信噪裕度</b></td>"));
#else
websWrite (wp,
                 T
                 ("<tr bgcolor=\"#DDDDDD\">\n <td width=40%%><font size=2><b>Upstream SNR</b></td>"));
#endif
      websWrite (wp, T ("<td width=60%%><font size=2>"));
      char buf[256] = { 0 };
      getAdslInfo (ADSL_GET_SNR_US, buf, sizeof (buf) / sizeof (char));
      websWrite (wp, T ("%sdB"), buf);
      websWrite (wp, T ("</td>\n </tr>"));
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
      websWrite (wp,
                 T
                 ("<tr bgcolor=\"#DDDDDD\">\n <td width=40%%><font size=2><b>下行信噪裕度</b></td>"));
#else
  websWrite (wp,
                 T
                 ("<tr bgcolor=\"#EEEEEE\">\n <td width=40%%><font size=2><b>Downstream SNR</b></td>"));
#endif
      websWrite (wp, T (" <td width=60%%><font size=2>"));
      memset (buf, 0, sizeof (buf) / sizeof (char));
      getAdslInfo (ADSL_GET_SNR_DS, buf, sizeof (buf) / sizeof (char));
      websWrite (wp, T ("%sdB"), buf);
      websWrite (wp, T ("</td>\n </tr>"));
      return 0;
    }

  if (!strcmp (name, T ("hskCount")))
    {
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
      websWrite (wp,
                 T
                 ("<tr bgcolor=\"#EEEEEE\">\n <td width=40%%><font size=2><b>重连次数统计</b></td>"));
#else
 websWrite (wp,
                 T
                 ("<tr bgcolor=\"#DDDDDD\">\n <td width=40%%><font size=2><b>reconnection Counts</b></td>"));
#endif
      websWrite (wp, T ("<td width=60%%><font size=2>"));
      int intVal;

      if (adsl_drv_get(RLCM_GET_REHS_COUNT, &intVal, sizeof(int)))
        {
          websWrite (wp, T ("%d"), (intVal==0)?0:intVal-1);
        }

      websWrite (wp, T ("</td>\n </tr>"));
      return 0;

    }

  if (!strcmp (name, T ("dslMode")))
    {

#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
      websWrite (wp,
                 T
                 ("<tr bgcolor=\"#EEEEEE\">\n <td width=40%%><font size=2><b>DSL模式设置</b></td>"));
   #else
    websWrite (wp,
                 T
                 ("<tr bgcolor=\"#EEEEEE\">\n <td width=40%%><font size=2><b>mode</b></td>"));
   #endif
      websWrite (wp, T ("<td width=60%%><font size=2>"));
      short mode;
      char buf[100]={0};
      mib_get( MIB_ADSL_MODE, (void *)&mode);

      if (mode & ADSL_MODE_GLITE)
        strcat(buf,"G.Lite ");

      if (mode & ADSL_MODE_T1413)
        strcat(buf,"T1.413 ");

      if (mode & ADSL_MODE_GDMT)
        strcat(buf,"G.Dmt ");

      if (mode & ADSL_MODE_ADSL2)
        strcat(buf,"ADSL2 ");

      if (mode & ADSL_MODE_ADSL2P)
        strcat(buf,"ADSL2+ ");

//according ZTE's advise... show all set mode... jim
	if (mode & ADSL_MODE_ANXL)
	        strcat(buf,"AnnexL  ");
	if (mode & ADSL_MODE_ANXM)
       	 strcat(buf,"AnnexM  ");
	if (mode & ADSL_MODE_ANXB)
       	 strcat(buf,"AnnexB  ");	
	

      websWrite(wp,T("%s"),buf);

      websWrite (wp, T ("</td>\n </tr>"));
      return 0;

    }

  if (!strcmp (name, T ("showtime")))
    {
    #if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
      websWrite (wp,
                 T
                 ("<tr bgcolor=\"#EEEEEE\">\n <td width=40%%><font size=2><b>DSL建链时间</b></td>"));
    #else
	websWrite (wp,
                 T
                 ("<tr bgcolor=\"#EEEEEE\">\n <td width=40%%><font size=2><b>Showtime:</b></td>"));
    #endif
      websWrite (wp, T ("<td width=60%%><font size=2>"));
      unsigned int intVal[3];
      char buffer[128]={0};

      if (adsl_drv_get(RLCM_GET_DSL_ORHERS, &intVal, 3*sizeof(int)))
        {
        //add by ramen to adjust the DSL connect time ---68/1
        // jim according paget's advise the accuracy equation is : actual time= showtime - showtime/69;
       	 intVal[0]=intVal[0]-intVal[0]/68;
          if (intVal[0]>3600)
            {
              sprintf(buffer,"%dh:%02dm",intVal[0]/3600,(intVal[0]%3600)/60);
              websWrite (wp, T ("%s"), buffer);
            }

          else if (intVal[0]>60)
            {
              websWrite(wp,T("%d min"),intVal[0]/60);
            }

          else
            websWrite(wp,T("< 1 min"));
        }

      websWrite (wp, T ("</td>\n </tr>"));

      return 0;

    }

	if (!strcmp (name, T ("wlaninfo")))
	{
#ifdef WLAN_SUPPORT
      		const char *bgColor[]={"#EEEEEE","#DDDDDD"};
      		int col_nums=0;
      		const char *wlan_band[] ={0,"802.11b","802.11g","802.11 b+g",0};
      		unsigned char vChar;
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
      		websWrite(wp,T("<tr>\n <td width=100%% colspan=\"2\" bgcolor=\"#008000\"><font color=\"#FFFFFF\" size=2><b>无线链接配置</b></font></td>"));
	  	websWrite(wp,T("<tr bgcolor=%s> <td width=40%%><font size=2><b>无线状态</b></td>"),bgColor[col_nums++%2]);
#else
	 	websWrite(wp,T("<tr>\n <td width=100%% colspan=\"2\" bgcolor=\"#008000\"><font color=\"#FFFFFF\" size=2><b>Wireless Configuration</b></font></td>"));
	  	websWrite(wp,T("<tr bgcolor=%s> <td width=40%%><font size=2><b>Wireless</b></td>"),bgColor[col_nums++%2]);
#endif

      		//ramen--wireless enable??
      		websWrite (wp, T ("<td width=60%%><font size=2>"));

     		 mib_get (MIB_WLAN_DISABLED, (void *) &vChar);

      		if (!vChar)
        		websWrite (wp, T (INFO_ENABLED));
      		else{
          		websWrite (wp, T (INFO_DISABLED));
          		websWrite (wp, T ("\n</td>\n</tr>\n"));
          		goto wlend;
        	}

      		websWrite (wp, T ("\n</td>\n</tr>\n"));

      		//ramen--get the wireless band
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
      		websWrite (wp,T("<tr bgcolor=%s> <td width=40%%><font size=2><b>频段</b></td>"),bgColor[col_nums++%2]);
#else
	   	websWrite (wp,T("<tr bgcolor=%s> <td width=40%%><font size=2><b>band</b></td>"),bgColor[col_nums++%2]);
#endif
      		websWrite (wp, T ("<td width=60%%><font size=2>"));
      		mib_get (MIB_WLAN_BAND, (void *) &vChar);
     	 	websWrite (wp, "%s", wlan_band[(BAND_TYPE_T) vChar]);
      		websWrite (wp, T ("\n</td>\n</tr>\n"));

	  //ramen--get wireless mode
      		if (mib_get (MIB_WLAN_MODE, (void *) &vChar))
        	{
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
          		websWrite (wp,  T ("<tr bgcolor=%s> <td width=40%%><font size=2><b>工作模式</b></td>"),bgColor[col_nums++%2]);
#else
			websWrite (wp,  T ("<tr bgcolor=%s> <td width=40%%><font size=2><b>Mode</b></td>"),bgColor[col_nums++%2]);
#endif
          		websWrite (wp, T ("<td width=60%%><font size=2>"));

          		if (vChar == AP_MODE)
            			websWrite (wp, T ("AP"));
          		else if (vChar == CLIENT_MODE)
            			websWrite (wp, T ("Client"));
          		else if (vChar == AP_WDS_MODE)// jim support wds info shown.
            			websWrite (wp, T ("AP+WDS"));
          		else if (vChar == WDS_MODE)
            			websWrite (wp, T ("WDS"));
          		websWrite (wp, T ("\n</td>\n</tr>\n"));
        	}

      	//ramen---broadcast SSID
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
      		websWrite (wp,  T ("<tr bgcolor=%s> <td width=40%%><font size=2><b>广播  SSID</b></td>"),bgColor[col_nums++%2]);
#else
	    	websWrite (wp,  T ("<tr bgcolor=%s> <td width=40%%><font size=2><b>Broadcast  SSID</b></td>"),bgColor[col_nums++%2]);
#endif
     		websWrite (wp, T ("<td width=60%%><font size=2>"));
        	mib_get (MIB_WLAN_HIDDEN_SSID, (void *) &vChar);	
          	websWrite (wp, T((vChar!=0)?INFO_DISABLED:INFO_ENABLED));
          	websWrite (wp, T ("\n</td>\n</tr>\n"));
    
wlend:
      		websWrite (wp, T ("</tr>"));
#endif
      		return 0;

	}
  
 	if(!strcmp(name,T("wlanencryptioninfo")))
  	{
#ifdef WLAN_SUPPORT
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
#ifdef ENABLE_WPAAES_WPA2TKIP
		char *encryptMode[]={"None","WEP","WPA(TKIP)","WPA(AES)","WPA2(AES)","WPA2(TKIP)","WPA2 Mixed"};
#else
		char *encryptMode[]={"None","WEP","WPA(TKIP)","","WPA2(AES)","","WPA2 Mixed"};
#endif
   		char *wepEncryptLength[]={"Disabled","64bits","128bits"};
   		char *encryptFormat[]={"Ascii","Hex"};
   		char *wpaAuthMode[]={"企业(RADIUS)","个人(预共享密钥) "};
   		char *preshareKeyMode[]={"Passphrase","Hex"};
   		char *authtype[] = { "Open System", "Shared Key", "Auto" };
   		const char *bgColor[]=    {"#EEEEEE","#DDDDDD"    };
   		unsigned char tmpBuf[100];
   		unsigned char vChar,wepDefaultKey,wepKeyType,wepLen,encryptType,enable_1x,wpaAuth;
   		unsigned char bgcolorIndex=0;
		unsigned short rsPort;
#ifdef WLAN_MBSSID
		MIB_CE_MBSSIB_T Entry;
		MIB_CE_MBSSIB_WEP_T wepEntry;
		char *ssidMode[]={"SSID1(root)","SSID2","SSID3","SSID4","SSID5"};
		int i;
		unsigned char wlanDisabled;
#endif
#ifdef WLAN_MBSSID
		for(i = 0; i < 5; i++)
		{
			if (!mib_chain_get(MIB_MBSSIB_TBL, i, (void *)&Entry)) {  		
				goto read_mib_chain_error;		
			}
		 	if(i == 0){
        			MBSSID_GetRootEntry(&Entry);
 			}
			
			mib_get(MIB_WLAN_DISABLED, (void *)&wlanDisabled);	
			
   			websWrite(wp,T("<tr><td width=100%% colspan=\"2\" bgcolor=\"#008000\"><font size=2 color=\"#FFFFFF\"><b>%s</b></font></td></tr>\n"),ssidMode[Entry.idx]);
   			if(wlanDisabled || Entry.wlanDisabled) 
   			{
   		 		websWrite (wp, T("<tr bgcolor=%s> <td width=40%%><font size=2><b>状态</b></td>\n"),bgColor[bgcolorIndex++%2]);
	  			websWrite (wp, T ("<td width=60%%><font size=2>%s</font></td></tr>\n"),INFO_DISABLED);
   				continue;
   			}
   			else{
   	  			websWrite (wp, T("<tr bgcolor=%s> <td width=40%%><font size=2><b>状态</b></td>\n"),bgColor[bgcolorIndex++%2]);
	  			websWrite (wp, T ("<td width=60%%><font size=2>%s</font></td></tr>\n"),INFO_ENABLED);
   			}
   			if(i==0)//root mode
   			{
   	  			//ramen--get wireless SSID
          			char rootssid[100] = { 0 };
          			if (mib_get (MIB_WLAN_SSID, (void *) rootssid))
            			{
              				websWrite (wp, T("<tr bgcolor=%s> <td width=40%%><font size=2><b>SSID</b></td>\n"),bgColor[bgcolorIndex++%2]);
              				websWrite (wp, T ("<td width=60%%><font size=2>\n"));
              				websWrite (wp, T ("%s"), rootssid);
              				websWrite (wp, T ("\n</td>\n</tr>\n"));
	  			}  	 
   			}
  			else{
  	 			websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>SSID</b></font></td>\n"),bgColor[bgcolorIndex++%2]);
          			websWrite(wp,T("<td width=60%%><font size=2>%s</font></td></tr>\n"),Entry.ssid);
  			}
   			websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>认证方式</b></font></td>\n"),bgColor[bgcolorIndex++%2]);
   			if(i == 0) //root
   			{
   				mib_get (MIB_WLAN_AUTH_TYPE, (void *) &vChar);
				websWrite(wp,T("<td width=60%%><font size=2>%s</font></td></tr>\n"),authtype[vChar]);
   			}
   			else
   				websWrite(wp,T("<td width=60%%><font size=2>%s</font></td></tr>\n"),authtype[Entry.authType]);
  			websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>加密方式</b></font></td>\n"),bgColor[bgcolorIndex++%2]);
			vChar = Entry.encrypt;
#ifdef ENABLE_WPAAES_WPA2TKIP
			if(Entry.encrypt == ENCRYPT_WPA){
				if(Entry.unicastCipher == WPA_CIPHER_TKIP)
					vChar = 2;
				else
					vChar = 3;
			}else if(Entry.encrypt == ENCRYPT_WPA2){
				if(Entry.wpa2UnicastCipher == WPA_CIPHER_TKIP)
					vChar = 5;
				else
					vChar = 4;
			}
#endif
			websWrite(wp,T("<td width=60%%><font size=2>%s</font></td></tr>"),encryptMode[vChar] );

   			if(Entry.encrypt==ENCRYPT_WEP)
   			{
		   	    	websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>密钥长度</b></font></td>\n"),bgColor[bgcolorIndex++%2]);
			    	websWrite(wp,T("<td width=60%%><font size=2>%s</font></td></tr>\n"),wepEncryptLength[Entry.wep]);
				if (0 == i) {
                                	MBSSID_GetRootEntryWEP(&wepEntry);
				}else{
					if (!mib_chain_get(MIB_MBSSIB_WEP_TBL, i, (void *)&wepEntry)) {  		
						goto read_mib_chain_error;		
					}
				}

			    	websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>密钥格式</b></font></td>\n"),bgColor[bgcolorIndex++%2]);
			    	websWrite(wp,T("<td width=60%%><font size=2>%s</font></td></tr>\n"),encryptFormat[wepEntry.wepKeyType]);
			    	websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>密钥%d</b></font></td>\n"),bgColor[bgcolorIndex++%2],wepEntry.wepDefaultKey+1);
			    	if(Entry.wep==1)//64bits
			       {
			          	switch(wepEntry.wepDefaultKey+1)
			          	{
			             	case 1:
				      		if(wepEntry.wepKeyType ==0)//ascii	 	
			                   		websWrite(wp,T("<td width=60%%><font size=2>%c%c%c%c%c</font></td></tr>\n"),
			 					wepEntry.wep64Key1[0],wepEntry.wep64Key1[1],wepEntry.wep64Key1[2],wepEntry.wep64Key1[3],
			 					wepEntry.wep64Key1[4]);
				      		else
					  		websWrite(wp,T("<td width=60%%><font size=2>%02x%02x%02x%02x%02x</font></td></tr>\n"),
			 					wepEntry.wep64Key1[0],wepEntry.wep64Key1[1],wepEntry.wep64Key1[2],wepEntry.wep64Key1[3],
			 					wepEntry.wep64Key1[4]);
			                	break;
			             	case 2:
			                  	 if(wepEntry.wepKeyType ==0)//ascii	 	
			                   		websWrite(wp,T("<td width=60%%><font size=2>%c%c%c%c%c</font></td></tr>\n"),
			 					wepEntry.wep64Key2[0],wepEntry.wep64Key2[1],wepEntry.wep64Key2[2],wepEntry.wep64Key2[3],
			 					wepEntry.wep64Key2[4]);
				      		else
					  		websWrite(wp,T("<td width=60%%><font size=2>%02x%02x%02x%02x%02x</font></td></tr>\n"),
			 					wepEntry.wep64Key2[0],wepEntry.wep64Key2[1],wepEntry.wep64Key2[2],wepEntry.wep64Key2[3],
			 					wepEntry.wep64Key2[4]);
			                	break;
			             	case 3:
			                 	 if(wepEntry.wepKeyType ==0)//ascii	 	
			                   		websWrite(wp,T("<td width=60%%><font size=2>%c%c%c%c%c</font></td></tr>\n"),
			 					wepEntry.wep64Key3[0],wepEntry.wep64Key3[1],wepEntry.wep64Key3[2],wepEntry.wep64Key3[3],
			 					wepEntry.wep64Key3[4]);
				      		else
					  		websWrite(wp,T("<td width=60%%><font size=2>%02x%02x%02x%02x%02x</font></td></tr>\n"),
			 					wepEntry.wep64Key3[0],wepEntry.wep64Key3[1],wepEntry.wep64Key3[2],wepEntry.wep64Key3[3],
			 					wepEntry.wep64Key3[4]);
			                	break;
			             	case 4:
			                  	if(wepEntry.wepKeyType ==0)//ascii	 	
			                   		websWrite(wp,T("<td width=60%%><font size=2>%c%c%c%c%c</font></td></tr>\n"),
			 					wepEntry.wep64Key4[0],wepEntry.wep64Key4[1],wepEntry.wep64Key4[2],wepEntry.wep64Key4[3],
			 					wepEntry.wep64Key4[4]);
				      		else
					  		websWrite(wp,T("<td width=60%%><font size=2>%02x%02x%02x%02x%02x</font></td></tr>\n"),
			 					wepEntry.wep64Key4[0],wepEntry.wep64Key4[1],wepEntry.wep64Key4[2],wepEntry.wep64Key4[3],
			 					wepEntry.wep64Key4[4]);
			                	break;
			             	default:
			                	websWrite(wp,T("<td width=60%%><font size=2>&nbsp</font></td></tr>\n"));
			                	break;
			             
		            		}
		         	}
		      		else if(Entry.wep == 2)//128bits
		           	{
		           		switch(wepEntry.wepDefaultKey+1)
			          	{
			            	 case 1:
				  		 if(wepEntry.wepKeyType ==0)//ascii	 
			                		websWrite(wp,T("<td width=60%%><font size=2>%c%c%c%c%c%c%c%c%c%c%c%c%c</font></td></tr>\n"),
			 					wepEntry.wep128Key1[0],wepEntry.wep128Key1[1],wepEntry.wep128Key1[2],wepEntry.wep128Key1[3],
			 					wepEntry.wep128Key1[4],wepEntry.wep128Key1[5],wepEntry.wep128Key1[6],wepEntry.wep128Key1[7],
			 					wepEntry.wep128Key1[8],wepEntry.wep128Key1[9],wepEntry.wep128Key1[10],wepEntry.wep128Key1[11],
			 					wepEntry.wep128Key1[12]);
				   		else
				        		websWrite(wp,T("<td width=60%%><font size=2>%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x</font></td></tr>\n"),
			 					wepEntry.wep128Key1[0],wepEntry.wep128Key1[1],wepEntry.wep128Key1[2],wepEntry.wep128Key1[3],
			 					wepEntry.wep128Key1[4],wepEntry.wep128Key1[5],wepEntry.wep128Key1[6],wepEntry.wep128Key1[7],
			 					wepEntry.wep128Key1[8],wepEntry.wep128Key1[9],wepEntry.wep128Key1[10],wepEntry.wep128Key1[11],
			 					wepEntry.wep128Key1[12]);
			                	break;
			             	case 2:
			                 	if(wepEntry.wepKeyType ==0)//ascii	 
			                		websWrite(wp,T("<td width=60%%><font size=2>%c%c%c%c%c%c%c%c%c%c%c%c%c</font></td></tr>\n"),
			 					wepEntry.wep128Key2[0],wepEntry.wep128Key2[1],wepEntry.wep128Key2[2],wepEntry.wep128Key2[3],
			 					wepEntry.wep128Key2[4],wepEntry.wep128Key2[5],wepEntry.wep128Key2[6],wepEntry.wep128Key2[7],
			 					wepEntry.wep128Key2[8],wepEntry.wep128Key2[9],wepEntry.wep128Key2[10],wepEntry.wep128Key2[11],
			 					wepEntry.wep128Key2[12]);
				   		else
				        		websWrite(wp,T("<td width=60%%><font size=2>%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x</font></td></tr>\n"),
			 					wepEntry.wep128Key2[0],wepEntry.wep128Key2[1],wepEntry.wep128Key2[2],wepEntry.wep128Key2[3],
			 					wepEntry.wep128Key2[4],wepEntry.wep128Key2[5],wepEntry.wep128Key2[6],wepEntry.wep128Key2[7],
			 					wepEntry.wep128Key2[8],wepEntry.wep128Key2[9],wepEntry.wep128Key2[10],wepEntry.wep128Key2[11],
			 					wepEntry.wep128Key2[12]);
			                	break;
			             	case 3:
			               		if(wepEntry.wepKeyType ==0)//ascii	 
			                		websWrite(wp,T("<td width=60%%><font size=2>%c%c%c%c%c%c%c%c%c%c%c%c%c</font></td></tr>\n"),
			 					wepEntry.wep128Key3[0],wepEntry.wep128Key3[1],wepEntry.wep128Key3[2],wepEntry.wep128Key3[3],
			 					wepEntry.wep128Key3[4],wepEntry.wep128Key3[5],wepEntry.wep128Key3[6],wepEntry.wep128Key3[7],
			 					wepEntry.wep128Key3[8],wepEntry.wep128Key3[9],wepEntry.wep128Key3[10],wepEntry.wep128Key3[11],
			 					wepEntry.wep128Key3[12]);
				   		else
				        		websWrite(wp,T("<td width=60%%><font size=2>%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x</font></td></tr>\n"),
			 					wepEntry.wep128Key3[0],wepEntry.wep128Key3[1],wepEntry.wep128Key3[2],wepEntry.wep128Key3[3],
			 					wepEntry.wep128Key3[4],wepEntry.wep128Key3[5],wepEntry.wep128Key3[6],wepEntry.wep128Key3[7],
			 					wepEntry.wep128Key3[8],wepEntry.wep128Key3[9],wepEntry.wep128Key3[10],wepEntry.wep128Key3[11],
			 					wepEntry.wep128Key3[12]);
			                	break;
			             	case 4:
			              		if(wepEntry.wepKeyType ==0)//ascii	 
			                		websWrite(wp,T("<td width=60%%><font size=2>%c%c%c%c%c%c%c%c%c%c%c%c%c</font></td></tr>\n"),
			 					wepEntry.wep128Key4[0],wepEntry.wep128Key4[1],wepEntry.wep128Key4[2],wepEntry.wep128Key4[3],
			 					wepEntry.wep128Key4[4],wepEntry.wep128Key4[5],wepEntry.wep128Key4[6],wepEntry.wep128Key4[7],
			 					wepEntry.wep128Key4[8],wepEntry.wep128Key4[9],wepEntry.wep128Key4[10],wepEntry.wep128Key4[11],
			 					wepEntry.wep128Key4[12]);
				   		else
				        		websWrite(wp,T("<td width=60%%><font size=2>%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x</font></td></tr>\n"),
			 					wepEntry.wep128Key4[0],wepEntry.wep128Key4[1],wepEntry.wep128Key4[2],wepEntry.wep128Key4[3],
			 					wepEntry.wep128Key4[4],wepEntry.wep128Key4[5],wepEntry.wep128Key4[6],wepEntry.wep128Key4[7],
			 					wepEntry.wep128Key4[8],wepEntry.wep128Key4[9],wepEntry.wep128Key4[10],wepEntry.wep128Key4[11],
			 					wepEntry.wep128Key4[12]);
			                	break;
			             	default:
			                	websWrite(wp,T("<td width=60%%><font size=2>&nbsp</font></td></tr>\n"));
			                	break;
			             
		            		}
		      		}
//enable 802.1x??
#ifdef WLAN_1x
				if(Entry.enable1X)
				{
					websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>802.1x认证方式</b></font></td>\n"),bgColor[bgcolorIndex++%2]);
		 			websWrite(wp,T("<td width=60%%><font size=2>%s</font></td></tr>\n"),INFO_ENABLED);
		 
		     			websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>Radius 服务器Ip </b></font></td>\n"),bgColor[bgcolorIndex++%2]);
					websWrite(wp,T("<td width=60%%><font size=2>%d.%d.%d.%d</font></td></tr>\n"),Entry.rsIpAddr[0],Entry.rsIpAddr[1],Entry.rsIpAddr[2],Entry.rsIpAddr[3]);

		  			websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>Radius 服务器Ip 端口号</b></font></td>\n"),bgColor[bgcolorIndex++%2]);
		 			websWrite(wp,T("<td width=60%%><font size=2>%d</font></td></tr>\n"),Entry.rsPort);

		  			websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>Radius 服务器密码</b></font></td>\n"),bgColor[bgcolorIndex++%2]);
		 			websWrite(wp,T("<td width=60%%><font size=2>%s</font></td></tr>\n"),Entry.rsPassword);

				}else{
		 			websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>802.1x认证方式</b></font></td>\n"),bgColor[bgcolorIndex++%2]);
		 			websWrite(wp,T("<td width=60%%><font size=2>Disabled</font></td></tr>\n"));
				}	    
#endif
   			}
   			else//other enctyption mode except WEP
   			{
   	  			if(Entry.encrypt==ENCRYPT_DISABLED) continue;
	  			websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>WPA认证模式</b></font></td>\n"),bgColor[bgcolorIndex++%2]);
	  			websWrite(wp,T("<td width=60%%><font size=2>%s</font></td></tr>\n"),wpaAuthMode[Entry.wpaAuth-1]);
	 			if(Entry.wpaAuth==1)//radius server
	    			{
	    				websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>Radius 服务器Ip</b></font></td>\n"),bgColor[bgcolorIndex++%2]);
		 			websWrite(wp,T("<td width=60%%><font size=2>%d.%d.%d.%d</font></td></tr>\n"),Entry.rsIpAddr[0],Entry.rsIpAddr[1],Entry.rsIpAddr[2],Entry.rsIpAddr[3]);

		  			websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>Radius 服务器Ip 端口号</b></font></td>\n"),bgColor[bgcolorIndex++%2]);
		 			websWrite(wp,T("<td width=60%%><font size=2>%d</font></td></tr>\n"),Entry.rsPort);

		  			websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>Radius 服务器密码</b></font></td>\n"),bgColor[bgcolorIndex++%2]);
		 			websWrite(wp,T("<td width=60%%><font size=2>%s</font></td></tr>\n"),Entry.rsPassword);
	  			}
	  			else {//presharekey
	             			websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>预共享密钥格式</b></font></td>\n"),bgColor[bgcolorIndex++%2]);
		    			websWrite(wp,T("<td  width=60%%><font size=2>%s</font></td></tr>\n"),preshareKeyMode[Entry.wpaPSKFormat]);
		    			websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>预共享密钥</b></font></td>\n"),bgColor[bgcolorIndex++%2]);
		    			websWrite(wp,T("<td width=60%%><font size=2>%s</font></td></tr>\n"),Entry.wpaPSK);
	  			}
   			}
	
		}
#else

		websWrite(wp,T("<tr><td width=100%% colspan=\"2\" bgcolor=\"#008000\"><font size=2 color=\"#FFFFFF\"><b>root</b></font></td></tr>\n"));
   	  	websWrite(wp, T("<tr bgcolor=%s> <td width=40%%><font size=2><b>状态</b></td>\n"),bgColor[bgcolorIndex++%2]);
		mib_get(MIB_WLAN_DISABLED, (void *)&vChar);	
		if(vChar !=0 ) 
   		{
	  		websWrite (wp, T ("<td width=60%%><font size=2>%s</font></td></tr>\n"),INFO_DISABLED);
   			return 0;
   		}
   		else{
	  		websWrite (wp, T ("<td width=60%%><font size=2>%s</font></td></tr>\n"),INFO_ENABLED);
   		}

   	  	//ramen--get wireless SSID
          	char rootssid[100] = { 0 };
          	if (mib_get (MIB_WLAN_SSID, (void *) rootssid))
            	{
              		websWrite (wp, T("<tr bgcolor=%s> <td width=40%%><font size=2><b>SSID</b></td>\n"),bgColor[bgcolorIndex++%2]);
              		websWrite (wp, T ("<td width=60%%><font size=2>\n"));
              		websWrite (wp, T ("%s"), rootssid);
              		websWrite (wp, T ("\n</td>\n</tr>\n"));
	  	}  	 

		websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>认证方式</b></font></td>\n"),bgColor[bgcolorIndex++%2]);
		mib_get (MIB_WLAN_AUTH_TYPE, (void *) &vChar);
		websWrite(wp,T("<td width=60%%><font size=2>%s</font></td></tr>\n"),authtype[vChar]);

		websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>加密方式</b></font></td>\n"),bgColor[bgcolorIndex++%2]);
		mib_get(MIB_WLAN_ENCRYPT, (void *)&encryptType);	
		vChar = encryptType;
#ifdef ENABLE_WPAAES_WPA2TKIP
		if(encryptType == ENCRYPT_WPA){
			mib_get( MIB_WLAN_WPA_CIPHER_SUITE, &vChar);
			if(vChar == WPA_CIPHER_TKIP)
				vChar = 2;
			else
				vChar = 3;
		}else if(encryptType == ENCRYPT_WPA2){
			mib_get( MIB_WLAN_WPA2_CIPHER_SUITE, &vChar);
			if(vChar == WPA_CIPHER_TKIP)
				vChar = 5;
			else
				vChar = 4;
		}
#endif
		websWrite(wp,T("<td width=60%%><font size=2>%s</font></td></tr>"),encryptMode[vChar] );

		if(encryptType==ENCRYPT_WEP)
   		{
			websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>密钥长度</b></font></td>\n"),bgColor[bgcolorIndex++%2]);
			mib_get(MIB_WLAN_WEP, (void *)&wepLen);
			websWrite(wp,T("<td width=60%%><font size=2>%s</font></td></tr>\n"),wepEncryptLength[wepLen]);
			websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>密钥格式</b></font></td>\n"),bgColor[bgcolorIndex++%2]);
			mib_get(MIB_WLAN_WEP_KEY_TYPE, (void *)&wepKeyType);
			websWrite(wp,T("<td width=60%%><font size=2>%s</font></td></tr>\n"),encryptFormat[wepKeyType]);
			mib_get(MIB_WLAN_WEP_DEFAULT_KEY,(void *)&wepDefaultKey);
			websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>密钥%d</b></font></td>\n"),bgColor[bgcolorIndex++%2],wepDefaultKey+1);
			if(wepLen==1)//64bits
			{
			        switch(wepDefaultKey+1)
			        {
			         case 1:
					mib_get(MIB_WLAN_WEP64_KEY1, (void *)tmpBuf);
					if(wepKeyType ==0)//ascii	 	
		                   		websWrite(wp,T("<td width=60%%><font size=2>%c%c%c%c%c</font></td></tr>\n"),
		 					tmpBuf[0],tmpBuf[1],tmpBuf[2],tmpBuf[3],tmpBuf[4]);
			      		else
				  		websWrite(wp,T("<td width=60%%><font size=2>%02x%02x%02x%02x%02x</font></td></tr>\n"),
		 					tmpBuf[0],tmpBuf[1],tmpBuf[2],tmpBuf[3],tmpBuf[4]);
		                	break;
		             	case 2:
					mib_get(MIB_WLAN_WEP64_KEY2, (void *)tmpBuf);
					if(wepKeyType ==0)//ascii	 	
		                   		websWrite(wp,T("<td width=60%%><font size=2>%c%c%c%c%c</font></td></tr>\n"),
		 					tmpBuf[0],tmpBuf[1],tmpBuf[2],tmpBuf[3],tmpBuf[4]);
			      		else
				  		websWrite(wp,T("<td width=60%%><font size=2>%02x%02x%02x%02x%02x</font></td></tr>\n"),
		 					tmpBuf[0],tmpBuf[1],tmpBuf[2],tmpBuf[3],tmpBuf[4]);
		                	break;
		            	case 3:
					mib_get(MIB_WLAN_WEP64_KEY3, (void *)tmpBuf);
					if(wepKeyType ==0)//ascii	 	
		                   		websWrite(wp,T("<td width=60%%><font size=2>%c%c%c%c%c</font></td></tr>\n"),
		 					tmpBuf[0],tmpBuf[1],tmpBuf[2],tmpBuf[3],tmpBuf[4]);
			      		else
				  		websWrite(wp,T("<td width=60%%><font size=2>%02x%02x%02x%02x%02x</font></td></tr>\n"),
		 					tmpBuf[0],tmpBuf[1],tmpBuf[2],tmpBuf[3],tmpBuf[4]);
		                	break;
		             	case 4:
					mib_get(MIB_WLAN_WEP64_KEY4, (void *)tmpBuf);
					if(wepKeyType ==0)//ascii	 	
		                   		websWrite(wp,T("<td width=60%%><font size=2>%c%c%c%c%c</font></td></tr>\n"),
		 					tmpBuf[0],tmpBuf[1],tmpBuf[2],tmpBuf[3],tmpBuf[4]);
			      		else
				  		websWrite(wp,T("<td width=60%%><font size=2>%02x%02x%02x%02x%02x</font></td></tr>\n"),
		 					tmpBuf[0],tmpBuf[1],tmpBuf[2],tmpBuf[3],tmpBuf[4]);
		                	break;
		             	default:
		                	websWrite(wp,T("<td width=60%%><font size=2>&nbsp</font></td></tr>\n"));
		                	break;
	           		}
		        }
		      	else if(wepLen == 2)//128bits
		        {
	           		switch(wepDefaultKey+1)
		          	{
		            	 case 1:
					mib_get(MIB_WLAN_WEP128_KEY1,(void *)tmpBuf);
					if(wepKeyType ==0)//ascii	 
			                	websWrite(wp,T("<td width=60%%><font size=2>%c%c%c%c%c%c%c%c%c%c%c%c%c</font></td></tr>\n"),
							tmpBuf[0],tmpBuf[1],tmpBuf[2],tmpBuf[3],tmpBuf[4],tmpBuf[5],tmpBuf[6],tmpBuf[7],
		 					tmpBuf[8],tmpBuf[9],tmpBuf[10],tmpBuf[11],tmpBuf[12]);
			   		else
			        		websWrite(wp,T("<td width=60%%><font size=2>%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x</font></td></tr>\n"),
		 					tmpBuf[0],tmpBuf[1],tmpBuf[2],tmpBuf[3],tmpBuf[4],tmpBuf[5],tmpBuf[6],tmpBuf[7],
		 					tmpBuf[8],tmpBuf[9],tmpBuf[10],tmpBuf[11],tmpBuf[12]);
		                	break;
		             	case 2:
					mib_get(MIB_WLAN_WEP128_KEY2,(void *)tmpBuf);
					if(wepKeyType ==0)//ascii	 
			                	websWrite(wp,T("<td width=60%%><font size=2>%c%c%c%c%c%c%c%c%c%c%c%c%c</font></td></tr>\n"),
							tmpBuf[0],tmpBuf[1],tmpBuf[2],tmpBuf[3],tmpBuf[4],tmpBuf[5],tmpBuf[6],tmpBuf[7],
		 					tmpBuf[8],tmpBuf[9],tmpBuf[10],tmpBuf[11],tmpBuf[12]);
			   		else
			        		websWrite(wp,T("<td width=60%%><font size=2>%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x</font></td></tr>\n"),
		 					tmpBuf[0],tmpBuf[1],tmpBuf[2],tmpBuf[3],tmpBuf[4],tmpBuf[5],tmpBuf[6],tmpBuf[7],
		 					tmpBuf[8],tmpBuf[9],tmpBuf[10],tmpBuf[11],tmpBuf[12]);
		                	break;
		             	case 3:
					mib_get(MIB_WLAN_WEP128_KEY3,(void *)tmpBuf);
					if(wepKeyType ==0)//ascii	 
			                	websWrite(wp,T("<td width=60%%><font size=2>%c%c%c%c%c%c%c%c%c%c%c%c%c</font></td></tr>\n"),
							tmpBuf[0],tmpBuf[1],tmpBuf[2],tmpBuf[3],tmpBuf[4],tmpBuf[5],tmpBuf[6],tmpBuf[7],
		 					tmpBuf[8],tmpBuf[9],tmpBuf[10],tmpBuf[11],tmpBuf[12]);
			   		else
			        		websWrite(wp,T("<td width=60%%><font size=2>%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x</font></td></tr>\n"),
		 					tmpBuf[0],tmpBuf[1],tmpBuf[2],tmpBuf[3],tmpBuf[4],tmpBuf[5],tmpBuf[6],tmpBuf[7],
		 					tmpBuf[8],tmpBuf[9],tmpBuf[10],tmpBuf[11],tmpBuf[12]);
		                	break;
		             	case 4:
					mib_get(MIB_WLAN_WEP128_KEY4,(void *)tmpBuf);
					if(wepKeyType ==0)//ascii	 
			                	websWrite(wp,T("<td width=60%%><font size=2>%c%c%c%c%c%c%c%c%c%c%c%c%c</font></td></tr>\n"),
							tmpBuf[0],tmpBuf[1],tmpBuf[2],tmpBuf[3],tmpBuf[4],tmpBuf[5],tmpBuf[6],tmpBuf[7],
		 					tmpBuf[8],tmpBuf[9],tmpBuf[10],tmpBuf[11],tmpBuf[12]);
			   		else
			        		websWrite(wp,T("<td width=60%%><font size=2>%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x</font></td></tr>\n"),
		 					tmpBuf[0],tmpBuf[1],tmpBuf[2],tmpBuf[3],tmpBuf[4],tmpBuf[5],tmpBuf[6],tmpBuf[7],
		 					tmpBuf[8],tmpBuf[9],tmpBuf[10],tmpBuf[11],tmpBuf[12]);
		                	break;
		             	default:
		                	websWrite(wp,T("<td width=60%%><font size=2>&nbsp</font></td></tr>\n"));
		                	break;
			             
		       		}
		      	}
//enable 802.1x??
#ifdef WLAN_1x
			mib_get(MIB_WLAN_ENABLE_1X, (void *)&enable_1x);
			if(enable_1x)
			{
				websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>802.1x认证方式</b></font></td>\n"),bgColor[bgcolorIndex++%2]);
				websWrite(wp,T("<td width=60%%><font size=2>%s</font></td></tr>\n"),INFO_ENABLED);
		 
				websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>Radius 服务器Ip </b></font></td>\n"),bgColor[bgcolorIndex++%2]);
				mib_get(MIB_WLAN_RS_IP, (void *)tmpBuf);
				websWrite(wp,T("<td width=60%%><font size=2>%d.%d.%d.%d</font></td></tr>\n"),tmpBuf[0],tmpBuf[1],tmpBuf[2],tmpBuf[3]);

	  			websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>Radius 服务器Ip 端口号</b></font></td>\n"),bgColor[bgcolorIndex++%2]);
				mib_get(MIB_WLAN_RS_PORT, (void *)&rsPort);
				websWrite(wp,T("<td width=60%%><font size=2>%d</font></td></tr>\n"),rsPort);

				websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>Radius 服务器密码</b></font></td>\n"),bgColor[bgcolorIndex++%2]);
				mib_get(MIB_WLAN_RS_PASSWORD, (void *)tmpBuf);
				websWrite(wp,T("<td width=60%%><font size=2>%s</font></td></tr>\n"),tmpBuf);

			}else{
	 			websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>802.1x认证方式</b></font></td>\n"),bgColor[bgcolorIndex++%2]);
	 			websWrite(wp,T("<td width=60%%><font size=2>Disabled</font></td></tr>\n"));
			}	    
#endif
   		}
   		else//other enctyption mode except WEP
   		{
   			if(encryptType==ENCRYPT_DISABLED)
				return 0;
 			websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>WPA认证模式</b></font></td>\n"),bgColor[bgcolorIndex++%2]);
			mib_get(MIB_WLAN_WPA_AUTH, &wpaAuth);
			websWrite(wp,T("<td width=60%%><font size=2>%s</font></td></tr>\n"),wpaAuthMode[wpaAuth-1]);
			if(wpaAuth==1)//radius server
	  		{
	    			websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>Radius 服务器Ip</b></font></td>\n"),bgColor[bgcolorIndex++%2]);
				mib_get(MIB_WLAN_RS_IP, (void *)tmpBuf);
				websWrite(wp,T("<td width=60%%><font size=2>%d.%d.%d.%d</font></td></tr>\n"),tmpBuf[0],tmpBuf[1],tmpBuf[2],tmpBuf[3]);

	  			websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>Radius 服务器Ip 端口号</b></font></td>\n"),bgColor[bgcolorIndex++%2]);
				mib_get(MIB_WLAN_RS_PORT, (void *)&rsPort);
				websWrite(wp,T("<td width=60%%><font size=2>%d</font></td></tr>\n"),rsPort);

	  			websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>Radius 服务器密码</b></font></td>\n"),bgColor[bgcolorIndex++%2]);
				mib_get(MIB_WLAN_RS_PASSWORD, (void *)tmpBuf);
				websWrite(wp,T("<td width=60%%><font size=2>%s</font></td></tr>\n"),tmpBuf);
	  		}
	  		else {//presharekey
	       			websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>预共享密钥格式</b></font></td>\n"),bgColor[bgcolorIndex++%2]);
				mib_get(MIB_WLAN_WPA_PSK_FORMAT, (void *)&vChar);
				websWrite(wp,T("<td  width=60%%><font size=2>%s</font></td></tr>\n"),preshareKeyMode[vChar]);
	    			websWrite(wp,T("<tr bgcolor=%s><td width=40%%><font size=2><b>预共享密钥</b></font></td>\n"),bgColor[bgcolorIndex++%2]);
				mib_get(MIB_WLAN_WPA_PSK, (void *)tmpBuf);
				websWrite(wp,T("<td width=60%%><font size=2>%s</font></td></tr>\n"),tmpBuf);
			}
   		}
#endif

 read_mib_chain_error:
#endif
#endif
  		return 0;
  	}

  	if (!strcmp(name,T("wlanClient")))
    	{
#ifdef WLAN_SUPPORT
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
      		websWrite(wp,T("<P><table border=0 width=\"550\" bgcolor=\"#999999\">"
                     "<tr> <td width=100%% colspan=\"6\" bgcolor=\"#008000\"><font color=\"#FFFFFF\" size=2><b>Wireless 客户端列表</b></font></td> </tr>"
                     "<tr bgcolor=#7f7f7f><td width=\"25%%\"><font size=2><b>MAC 地址</b></td>"
                     "<td width=\"15%%\"><font size=2><b>发包个数</b></td>"
                     "<td width=\"15%%\"><font size=2><b>收包个数</b></td>"
                     "<td width=\"15%%\"><font size=2><b>发包速率 (Mbps)</b></td>"
                     "<td width=\"15%%\"><font size=2><b>Power Saving</b></td>"
                     "<td width=\"15%%\"><font size=2><b>过期时限(s)</b></td></tr>"));
#else
 		websWrite(wp,T("<P><table border=0 width=\"550\">"
                     "<tr> <td width=100%% colspan=\"6\" bgcolor=\"#008000\"><font color=\"#FFFFFF\" size=2><b>Wireless Client List</b></font></td> </tr>"
                     "<tr bgcolor=#7f7f7f><td width=\"25%%\"><font size=2><b>MAC Address</b></td>"
                     "<td width=\"15%%\"><font size=2><b>Tx Packet</b></td>"
                     "<td width=\"15%%\"><font size=2><b>Rx Packet</b></td>"
                     "<td width=\"15%%\"><font size=2><b>Tx Rate (Mbps)</b></td>"
                     "<td width=\"15%%\"><font size=2><b>Power Saving</b></td>"
                     "<td width=\"15%%\"><font size=2><b>Expired Time (s)</b></td></tr>"));
#endif
      		wirelessClientList(0,wp,1,0);
      		websWrite(wp,T("</table>"));
#endif
      		return 0;
    	}

  	if (!strcmp(name,"wlanAccessControl"))
    	{
#ifdef WLAN_SUPPORT
#ifdef WLAN_ACL
      		unsigned char vChar;
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
      		char *acType[]={"禁用","允许列表方式","禁止列表方式"};
#else
      		char *acType[]={"Disable","Allow Listed","Deny Listed"};
#endif

      		mib_get( MIB_WLAN_AC_ENABLED, (void *)&vChar);
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
      		websWrite(wp,T("<P><table border=0 width=550 bgcolor=\"#999999\">"
                     "<tr> <td width=100%%  bgcolor=\"#008000\" colspan=\"2\"><font color=\"#FFFFFF\" size=2><b>当前链路控制列表</b></font></td> </tr>"));
      		websWrite(wp,T("<tr bgcolor=\"#EEEEEE\"><td><font size=2><b>控制列表模式</b></font></td><td align=left><font size=2>%s</font></td></tr>"),acType[vChar]);
#else
   		websWrite(wp,T("<P><table border=0 width=550>"
                     "<tr> <td width=100%%  bgcolor=\"#008000\" colspan=\"2\"><font color=\"#FFFFFF\" size=2><b>Current Access Control List:</b></font></td> </tr>"));
      		websWrite(wp,T("<tr bgcolor=\"#EEEEEE\"><td><font size=2><b>Mode</b></font></td><td align=left><font size=2>%s</font></td></tr>"),acType[vChar]);
#endif

      		if (vChar){
          		wlShowAcList(0,wp,1,0);
        	}

      		websWrite(wp,T(" </table>"));
#endif
#endif
      		return 0;
    	}

  	if (!strcmp (name, T ("showpvctable0")))
    	{
#ifdef ZTE_531B_BRIDGE_SC
      		websWrite (wp,T("<table border=\"0\" width=700><tr><font size=2><b>当前ATM VC 列表</b></font></tr>"));
#else
   		websWrite (wp,T("<table border=\"0\" width=700><tr><font size=2><b>Current ATM VC Table:</b></font></tr>"));
#endif
      		atmVcList2 (0, wp, 1, 0);
      		websWrite (wp, T ("</table>"));
      		return 0;
    	}

  if (!strcmp (name, T ("showpvctable1")))
    {
#ifdef ZTE_531B_BRIDGE_SC
 websWrite (wp,
                 T
                 ("<table border=\"0\" width=700><tr><font size=2><b>当前ATM VC 列表</b></font></tr>"));
      atmVcList2 (0, wp, 1, 0);
      websWrite (wp, T ("</table>"));
#else
      websWrite (wp,
                 T
                 ("<table border=\"0\" width=700><tr><font size=2><b>Current ATM VC Table:</b></font></tr>"));
      atmVcList2 (0, wp, 1, 0);
      websWrite (wp, T ("</table>"));
#endif
      return 0;
    }
	if ( !strcmp(name, T("dhcpMode")) ) {
 		if ( !mib_get( MIB_DHCP_MODE, (void *)&vChar) )
			return -1;
/*		if (vChar == 0) {
			websWrite(wp, "<option selected value=\"0\">None</option>\n" );
			websWrite(wp, "<option value=\"1\">DHCP Relay</option>\n" );
			websWrite(wp, "<option value=\"2\">DHCP Server</option>\n" );
		}
		if (vChar == 1) {
			websWrite(wp, "<option selected value=\"1\">DHCP Relay</option>\n" );
			websWrite(wp, "<option value=\"0\">None</option>\n" );
			websWrite(wp, "<option value=\"2\">DHCP Server</option>\n" );
		}
		if (vChar == 2) {
			websWrite(wp, "<option selected value=\"2\">DHCP Server</option>\n" );
			websWrite(wp, "<option value=\"0\">None</option>\n" );
			websWrite(wp, "<option value=\"1\">DHCP Relay</option>\n" );
		}*/
		websWrite(wp, "<input type=\"radio\" name=dhcpdenable value=0 onClick=\"disabledhcpd()\">None&nbsp;&nbsp;\n" );
			websWrite(wp, "<input type=\"radio\"name=dhcpdenable value=1 onClick=\"enabledhcprelay()\">DHCP Relay&nbsp;&nbsp;\n" );
			websWrite(wp, "<input type=\"radio\"name=dhcpdenable value=2 onClick=\"enabledhcpd()\">DHCP Server&nbsp;&nbsp;\n" );
		return 0;
	}

#ifdef ADDRESS_MAPPING
#ifndef MULTI_ADDRESS_MAPPING
	if ( !strcmp(name, T("addressMapType")) ) {
 		if ( !mib_get( MIB_ADDRESS_MAP_TYPE, (void *)&vChar) )
			return -1;
		
		websWrite(wp, "<option value=0>None</option>\n" );
		websWrite(wp, "<option value=1>One-to-One</option>\n" );		
		websWrite(wp, "<option value=2>Many-to-One</option>\n" );
		websWrite(wp, "<option value=3>Many-to-Many Overload</option>\n" );			
		// Mason Yu on True
		websWrite(wp, "<option value=4>One-to-Many</option>\n" );
		return 0;
	}
#endif	// end of !MULTI_ADDRESS_MAPPING
#endif

#ifdef WLAN_SUPPORT
#ifdef WLAN_ACL
	if (!strcmp(name, T("wlanAcNum"))) {
		vUInt = mib_chain_total(MIB_WLAN_AC_TBL);
		if (0 == vUInt)
			websWrite(wp, "disableDelButton();");
		return 0;
	}
#endif
#ifdef WLAN_WDS
		if (!strcmp(name, T("wlanWDSNum"))) {
			vUInt = mib_chain_total(MIB_WDS_TBL);
			if (0 == vUInt)
				websWrite(wp, "disableDelButton();");
			return 0;
		}
#endif
	if ( !strcmp(name, T("wlmode")) ) {
 		if ( !mib_get( MIB_WLAN_MODE, (void *)&vChar) )
			return -1;
		if (vChar == AP_MODE) {
			websWrite(wp, "<option selected value=\"0\">AP</option>\n" );

#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
//xl_yue: 531B bridge only need AP
#ifdef ZTE_GENERAL_ROUTER_SC
#ifdef WLAN_CLIENT
			websWrite(wp, "<option value=\"1\">Client</option>\n" );
#endif
#ifdef WLAN_WDS
			websWrite(wp, "<option value=\"3\">AP+WDS</option>\n" );
#endif
#endif

#else

#ifdef WLAN_CLIENT
			websWrite(wp, "<option value=\"1\">Client</option>\n" );
#endif
#ifdef WLAN_WDS
			websWrite(wp, "<option value=\"3\">AP+WDS</option>\n" );
#endif

#endif
		}
#ifdef WLAN_CLIENT
		if (vChar == CLIENT_MODE) {
			websWrite(wp, "<option selected value=\"1\">Clinet</option>\n" );
			websWrite(wp, "<option value=\"0\">AP</option>\n" );
#ifdef WLAN_WDS

			websWrite(wp, "<option value=\"3\">AP+WDS</option>\n" );
#endif
		}
#endif
#ifdef WLAN_WDS
		if (vChar == AP_WDS_MODE) {
			websWrite(wp, "<option selected value=\"3\">AP+WDS</option>\n" );
			websWrite(wp, "<option value=\"0\">AP</option>\n" );
#ifdef WLAN_CLIENT
			websWrite(wp, "<option value=\"1\">Client</option>\n" );
#endif
		}
#endif
		return 0;
	}	
#ifdef WLAN_WDS
	if ( !strcmp(name, T("wlanWdsEnabled")) ) {
		if ( !mib_get( MIB_WLAN_WDS_ENABLED, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			websWrite(wp, "checked");
		return 0;
	}

#endif
	if ( !strcmp(name, T("wlband")) ) {
#ifdef WIFI_TEST
		websWrite(wp, "<option value=3>WiFi-G</option>\n" );
		websWrite(wp, "<option value=4>WiFi-BG</option>\n" );
#endif
#ifdef CONFIG_USB_RTL8192SU_SOFTAP
		websWrite(wp, "<option value=7>2.4 GHz (N)</option>\n" );   // add by yq_zhou 2.10
		websWrite(wp, "<option value=9>2.4 GHz (G+N)</option>\n" );
		websWrite(wp, "<option value=10>2.4 GHz (B+G+N)</option>\n" );
#endif
		return 0;
	}
	if ( !strcmp(name, T("wlchanwid")) ) {
#ifdef CONFIG_USB_RTL8192SU_SOFTAP
		websWrite(wp, "<option value=\"0\">20MHZ</option>\n" );
		websWrite(wp, "<option value=\"1\">40MHZ</option>\n" ); 
#endif
		return 0;
	}
	if ( !strcmp(name, T("wlctlband")) ) {
#ifdef CONFIG_USB_RTL8192SU_SOFTAP
		websWrite(wp, "<option value=\"0\">Upper</option>\n" );
		websWrite(wp, "<option value=\"1\">Lower</option>\n" );   
#endif
		return 0;
	}
	// Added by Mason Yu for TxPower	
	if ( !strcmp(name, T("txpower")) ) {
//modified by xl_yue
#ifdef WLAN_TX_POWER_DISPLAY
			websWrite(wp, "<option value=\"0\">100%%</option>\n" );
			websWrite(wp, "<option value=\"1\">80%%</option>\n" );
			websWrite(wp, "<option value=\"2\">50%%</option>\n" );
			websWrite(wp, "<option value=\"3\">25%%</option>\n" );
			websWrite(wp, "<option value=\"4\">10%%</option>\n" );
#else
 		if ( !mib_get( MIB_TX_POWER, (void *)&vChar) )
			return -1;

		if (vChar == 0) {
			websWrite(wp, "<option selected value=\"0\">15 mW</option>\n" );
			websWrite(wp, "<option value=\"1\">30 mW</option>\n" );
			websWrite(wp, "<option value=\"2\">60 mW</option>\n" );
		}
		if (vChar == 1) {
			websWrite(wp, "<option selected value=\"1\">30 mW</option>\n" );
			websWrite(wp, "<option value=\"0\">15 mW</option>\n" );
			websWrite(wp, "<option value=\"2\">60 mW</option>\n" );
		}
		if (vChar == 2) {
			websWrite(wp, "<option selected value=\"2\">60 mW</option>\n" );
			websWrite(wp, "<option value=\"0\">15 mW</option>\n" );
			websWrite(wp, "<option value=\"1\">30 mW</option>\n" );
		}
#endif
		return 0;
	}	
//added by xl_yue
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
#ifdef WLAN_MBSSID
	if ( !strcmp(name, T("wpaVAPSSID")) ) {
		MIB_CE_MBSSIB_T Entry;
		char strbuf2[20];
		char vapName[10];
		int i;
		unsigned char selEncrypt;
		
		for(i=0;i<4;i++){
			if (!mib_chain_get(MIB_MBSSIB_TBL, i+1, (void *)&Entry)) {  		
				return 0;		
			}
			
			if ( ((struct in_addr *)Entry.rsIpAddr)->s_addr == INADDR_NONE ) {
				sprintf(strbuf2, "%s", "");				
			} else {	
				sprintf(strbuf2, "%s", inet_ntoa(*((struct in_addr *)Entry.rsIpAddr)));
			}

			sprintf(vapName,"vap%d",i);

			selEncrypt = Entry.encrypt;
#ifdef ENABLE_WPAAES_WPA2TKIP
			if(Entry.encrypt == ENCRYPT_WPA){
				if(Entry.unicastCipher == WPA_CIPHER_TKIP)
					selEncrypt = ENCRY_WPA_TKIP;
				else
					selEncrypt = ENCRY_WPA_AES;
			}else if(Entry.encrypt == ENCRYPT_WPA2){
				if(Entry.wpa2UnicastCipher == WPA_CIPHER_TKIP)
					vChar = ENCRY_WPA2_TKIP;
				else
					vChar = ENCRY_WPA2_AES;
			}
#endif

			if(!Entry.wlanDisabled)
				websWrite(wp, "<input type=\"radio\" name=\"wpaSSID\" value=\"vap%d\" onClick=\"postSecurity(%d, %d, %d, %d, %d, '%s', %d, '%s', '%s', '%s')\">SSID%d&nbsp;\n",
							i,selEncrypt, Entry.enable1X, Entry.wep, Entry.wpaAuth, Entry.wpaPSKFormat, Entry.wpaPSK, Entry.rsPort, strbuf2, Entry.rsPassword,vapName, i+2 );
		}
	}
	if ( !strcmp(name, T("wepVAPSSID")) ) {
		MIB_CE_MBSSIB_T Entry;
		MIB_CE_MBSSIB_WEP_T EntryWEP;
		int i;
		for(i=0;i<4;i++){
			if (!mib_chain_get(MIB_MBSSIB_TBL, i+1, (void *)& Entry)) {  		
				return 0;		
			}
			if (!mib_chain_get(MIB_MBSSIB_WEP_TBL, i+1, (void *)&EntryWEP)) {  		
  				return 0;					
			}	
	
			if (Entry.wep == 0)
				Entry.wep = 1;

			if(! Entry.wlanDisabled)
				websWrite(wp, "<input type=\"radio\" name=\"wepSSID\" value=\"vap%d\" onClick=\"postWEP(%d, %d, %d, %d)\">SSID%d&nbsp;\n",
							i, Entry.wep, EntryWEP.wepKeyType+1, EntryWEP.wepDefaultKey+1, i+1, i+2);
		}
	}
#endif
#endif
	if ( !strcmp(name, T("wpaEncrypt")) ) {
#ifdef ENABLE_WPAAES_WPA2TKIP
		websWrite(wp, "<option value=%d>None</option>\n",ENCRY_DISABLED);
		websWrite(wp, "<option value=%d>WEP</option>\n" ,ENCRY_WEP);
		websWrite(wp, "<option value=%d>WPA(TKIP)</option>\n", ENCRY_WPA_TKIP);
		websWrite(wp, "<option value=%d>WPA(AES)</option>\n",ENCRY_WPA_AES);
		websWrite(wp, "<option value=%d>WPA2(AES)</option>\n",ENCRY_WPA2_AES);
		websWrite(wp, "<option value=%d>WPA2(TKIP)</option>\n",ENCRY_WPA2_TKIP);
		websWrite(wp, "<option value=%d>WPA2 Mixed</option>\n",ENCRY_MIXED);
#else
		websWrite(wp, "<option value=0>None</option>\n" );
		websWrite(wp, "<option value=1>WEP</option>\n" );
		websWrite(wp, "<option value=2>WPA(TKIP)</option>\n" );
		websWrite(wp, "<option value=4>WPA2(AES)</option>\n" );
		websWrite(wp, "<option value=6>WPA2 Mixed</option>\n" );
#endif
	}
#endif
	// Added by Mason Yu for 2 level web page
	if ( !strcmp(name, T("userMode")) ) {	
		#ifdef ACCOUNT_CONFIG
		MIB_CE_ACCOUNT_CONFIG_T Entry;
		int totalEntry, i;
		#else
		char suStr[100], usStr[100];
		#endif
#ifdef ZTE_531B_BRIDGE_SC
		//jim luo support ZTE 531B bridge only one user.
		sprintf(suStr, "<option selected value=\"0\">%s</option>\n", suName);
		websWrite(wp, suStr );
#else
#ifdef ACCOUNT_CONFIG
		#ifdef USE_LOGINWEB_OF_SERVER
		if (!strcmp(g_login_username, suName))
		#else
		if (!strcmp(wp->user, suName))
		#endif
		{
			websWrite(wp, "<option selected value=\"0\">%s</option>\n", suName);
			websWrite(wp, "<option value=\"1\">%s</option>\n", usName);
		}
		#ifdef USE_LOGINWEB_OF_SERVER
		else if (!strcmp(g_login_username, usName))
		#else
		else if (!strcmp(wp->user, usName))
		#endif
		{
			websWrite(wp, "<option value=\"0\">%s</option>\n", suName);
			websWrite(wp, "<option selected value=\"1\">%s</option>\n", usName);
		}
		totalEntry = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL);
		for (i=0; i<totalEntry; i++) {
			if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, i, (void *)&Entry))
				continue;
			#ifdef USE_LOGINWEB_OF_SERVER
			if (!strcmp(g_login_username, Entry.userName))
			#else
			if (strcmp(wp->user, Entry.userName) == 0)
			#endif
				websWrite(wp, "<option selected value=\"%d\">%s</option>\n", i+2, Entry.userName);
			else
				websWrite(wp, "<option value=\"%d\">%s</option>\n", i+2, Entry.userName);
		}
#else
		#ifdef USE_LOGINWEB_OF_SERVER
		if (!strcmp(g_login_username, suName))
		#else
		if(!strcmp(wp->user,suName))
		#endif
			{
			sprintf(suStr, "<option selected value=\"0\">%s</option>\n", suName);
			sprintf(usStr, "<option value=\"1\">%s</option>\n", usName);
			}
		else
			sprintf(usStr, "<option selected value=\"1\">%s</option>\n", usName);

		websWrite(wp, suStr );
		websWrite(wp, usStr );	
#endif
#endif
		return 0;
	}	
	if ( !strcmp(name, T("lan-dhcp-st")) ) {
		if ( !mib_get( MIB_DHCP_MODE, (void *)&vChar) )
			return -1;
		if (DHCP_LAN_SERVER == vChar) 
			websWrite(wp, "Enabled");
		else
			websWrite(wp, "Disabled");
		return 0;
	}
	else if ( !strcmp(name, T("br-stp-0")) ) {     
		if ( !mib_get( MIB_BRCTL_STP, (void *)&vChar) )
			return -1;
		if (0 == vChar) 
			websWrite(wp, "checked");      
		return 0;
	}
	else if ( !strcmp(name, T("br-stp-1")) ) {       
		if ( !mib_get( MIB_BRCTL_STP, (void *)&vChar) )
			return -1;
		if (1 == vChar) 
			websWrite(wp, "checked");      
		return 0;
	}
#ifdef CONFIG_USER_IGMPPROXY
	else if ( !strcmp(name, T("igmpProxy0")) ) {
		if ( !mib_get( MIB_IGMP_PROXY, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			websWrite(wp, "checked");
		if (ifWanNum("rt") ==0)
			websWrite(wp, " disabled");
		return 0;
	}
	else if ( !strcmp(name, T("igmpProxy1")) ) {
		if ( !mib_get( MIB_IGMP_PROXY, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			websWrite(wp, "checked");
		if (ifWanNum("rt") ==0)
			websWrite(wp, " disabled");
		return 0;
	}
	else if ( !strcmp(name, T("igmpProxy0d")) ) {
		if ( !mib_get( MIB_IGMP_PROXY, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			websWrite(wp, "disabled");
		return 0;
	}
#endif
//#ifdef CONFIG_USER_UPNPD	
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
	else if ( !strcmp(name, T("upnp0")) ) {
		if ( !mib_get( MIB_UPNP_DAEMON, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			websWrite(wp, "checked");
		if (ifWanNum("rt") ==0)
			websWrite(wp, " disabled");
		return 0;
	}
	else if ( !strcmp(name, T("upnp1")) ) {
		if ( !mib_get( MIB_UPNP_DAEMON, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			websWrite(wp, "checked");
		if (ifWanNum("rt") ==0)
			websWrite(wp, " disabled");
		return 0;
	}
	else if ( !strcmp(name, T("upnp0d")) ) {
		//if ( !mib_get( MIB_UPNP_DAEMON, (void *)&vChar) )
		//	return -1;
		if (ifWanNum("rt") ==0)
			websWrite(wp, "disabled");
		return 0;
	}
#endif	
#ifdef NAT_CONN_LIMIT
	else if (!strcmp(name, T("connlimit"))) {
		if (!mib_get(MIB_NAT_CONN_LIMIT, (void *)&vChar))
			return -1;
		if (1 == vChar)
			websWrite(wp, "checked");
	}
#endif
#ifdef TCP_UDP_CONN_LIMIT
	else if ( !strcmp(name, T("connLimit-cap0")) ) {
   		if ( !mib_get( MIB_CONNLIMIT_ENABLE, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("connLimit-cap1")) ) {
   		if ( !mib_get( MIB_CONNLIMIT_ENABLE, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			websWrite(wp, "checked");
		return 0;
	}

#endif

	else if ( !strcmp(name, T("acl-cap0")) ) {
   		if ( !mib_get( MIB_ACL_CAPABILITY, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("acl-cap1")) ) {
   		if ( !mib_get( MIB_ACL_CAPABILITY, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			websWrite(wp, "checked");
		return 0;
	}
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
	else if ( !strcmp(name, T("snmpd-on")) ) {
   		if ( !mib_get( MIB_SNMPD_ENABLE, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("snmpd-off")) ) {
   		if ( !mib_get( MIB_SNMPD_ENABLE, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			websWrite(wp, "checked");
		return 0;
	}
#endif
#ifdef URL_BLOCKING_SUPPORT
	else if ( !strcmp(name, T("url-cap0")) ) {
   		if ( !mib_get( MIB_URL_CAPABILITY, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("url-cap1")) ) {
   		if ( !mib_get( MIB_URL_CAPABILITY, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			websWrite(wp, "checked");
		return 0;
	}
#endif
//alex_huang 
#ifdef URL_ALLOWING_SUPPORT
       else if( !strcmp(name ,T("url-cap2")) ) {
	   	if( !mib_get (MIB_URL_CAPABILITY,(void*)&vChar) )
			return -1;
		if(2 == vChar)
			{
			    websWrite(wp, "checked");
			}
		return 0;
		
       	}
#endif


#ifdef DOMAIN_BLOCKING_SUPPORT
	else if ( !strcmp(name, T("domainblk-cap0")) ) {
   		if ( !mib_get( MIB_DOMAINBLK_CAPABILITY, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("domainblk-cap1")) ) {
   		if ( !mib_get( MIB_DOMAINBLK_CAPABILITY, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			websWrite(wp, "checked");
		return 0;
	}
#endif	
	else if ( !strcmp(name, T("dns0")) ) {
		if ( !mib_get( MIB_ADSL_WAN_DNS_MODE, (void *)&vChar) )
			return -1;
		if (0 == vChar) 
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("dns1")) ) {
		if ( !mib_get( MIB_ADSL_WAN_DNS_MODE, (void *)&vChar) )
			return -1;
		if (1 == vChar) 
			websWrite(wp, "checked");
		return 0;
	}
/*	else if ( !strcmp(name, T("portFwEn"))) {
		if ( !mib_get( MIB_PORT_FW_ENABLE, (void *)&vChar) )
			return -1;
		if (1 == vChar) 
			websWrite(wp, "checked");
		return 0;
	}*/
	else if ( !strcmp(name, T("portFw-cap0")) ) {
   		if ( !mib_get( MIB_PORT_FW_ENABLE, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("portFw-cap1")) ) {
   		if ( !mib_get( MIB_PORT_FW_ENABLE, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			websWrite(wp, "checked");
		return 0;
	}
	if ( !strcmp(name, T("portFwNum"))) {
		vUInt = mib_chain_total(MIB_PORT_FW_TBL);
		if (0 == vUInt)
			websWrite(wp, "disableDelButton();");
		return 0;
	}
#ifdef NATIP_FORWARDING
	else if ( !strcmp(name, T("ipFwEn"))) {
		if ( !mib_get( MIB_IP_FW_ENABLE, (void *)&vChar) )
			return -1;
		if (1 == vChar) 
			websWrite(wp, "checked");
		return 0;
	}
	if ( !strcmp(name, T("ipFwNum"))) {
		vUInt = mib_chain_total(MIB_IP_FW_TBL);
		if (0 == vUInt)
			websWrite(wp, "disableDelButton();");
		return 0;
	}
#endif
	else if ( !strcmp(name, T("ipf_out_act0"))) {
		if ( !mib_get( MIB_IPF_OUT_ACTION, (void *)&vChar) )
			return -1;
		if (0 == vChar) 
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("ipf_out_act1"))) {
		if ( !mib_get( MIB_IPF_OUT_ACTION, (void *)&vChar) )
			return -1;
		if (1 == vChar) 
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("ipf_in_act0"))) {
		if ( !mib_get( MIB_IPF_IN_ACTION, (void *)&vChar) )
			return -1;
		if (0 == vChar) 
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("ipf_in_act1"))) {
		if ( !mib_get( MIB_IPF_IN_ACTION, (void *)&vChar) )
			return -1;
		if (1 == vChar) 
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("macf_out_act0"))) {
		if ( !mib_get( MIB_MACF_OUT_ACTION, (void *)&vChar) )
			return -1;
		if (0 == vChar) 
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("macf_out_act1"))) {
		if ( !mib_get( MIB_MACF_OUT_ACTION, (void *)&vChar) )
			return -1;
		if (1 == vChar) 
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("macf_in_act0"))) {
		if ( !mib_get( MIB_MACF_IN_ACTION, (void *)&vChar) )
			return -1;
		if (0 == vChar) 
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("macf_in_act1"))) {
		if ( !mib_get( MIB_MACF_IN_ACTION, (void *)&vChar) )
			return -1;
		if (1 == vChar) 
			websWrite(wp, "checked");
		return 0;
	}
/*	else if ( !strcmp(name, T("dmzEn"))) {
		if ( !mib_get( MIB_DMZ_ENABLE, (void *)&vChar) )
			return -1;
		if (vChar)
			websWrite(wp, "checked");
		return 0;
	}*/
	else if ( !strcmp(name, T("dmz-cap0")) ) {
   		if ( !mib_get( MIB_DMZ_ENABLE, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("dmz-cap1")) ) {
   		if ( !mib_get( MIB_DMZ_ENABLE, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("ipFilterNum"))) {
		vUInt = mib_chain_total(MIB_IP_PORT_FILTER_TBL);
		if (0 == vUInt)
			websWrite(wp, "disableDelButton();");
		return 0;
	}
#ifdef TCP_UDP_CONN_LIMIT
	else if ( !strcmp(name, T("connLimitNum"))) {
		vUInt = mib_chain_total(MIB_TCP_UDP_CONN_LIMIT_TBL);
		if (0 == vUInt)
			websWrite(wp, "disableDelButton();");
		return 0;
	}
#endif
#ifdef MULTI_ADDRESS_MAPPING
	else if ( !strcmp(name, T("AddresMapNum"))) {
		vUInt = mib_chain_total(MULTI_ADDRESS_MAPPING_LIMIT_TBL);
		if (0 == vUInt)
			websWrite(wp, "disableDelButton();");
		return 0;
	}
#endif  // end of MULTI_ADDRESS_MAPPING
#ifdef URL_BLOCKING_SUPPORT
	else if ( !strcmp(name, T("keywdNum"))) {
		vUInt = mib_chain_total(MIB_KEYWD_FILTER_TBL);
		if (0 == vUInt)
			websWrite(wp, "disableDelKeywdButton();");
		return 0;
	}
	else if ( !strcmp(name, T("FQDNNum"))) {
		vUInt = mib_chain_total(MIB_URL_FQDN_TBL);
		if (0 == vUInt)
			websWrite(wp, "disableDelFQDNButton();");
		return 0;
	}
#endif
#ifdef DOMAIN_BLOCKING_SUPPORT
	else if ( !strcmp(name, T("domainNum"))) {
		vUInt = mib_chain_total(MIB_DOMAIN_BLOCKING_TBL);
		if (0 == vUInt)
			websWrite(wp, "disableDelButton();");
		return 0;
	}
#endif
	else if ( !strcmp(name, T("ripNum"))) {
		vUInt = mib_chain_total(MIB_RIP_TBL);
		if (0 == vUInt)
			websWrite(wp, "disableDelButton();");
		return 0;
	}
	else if ( !strcmp(name, T("aclNum"))) {
		vUInt = mib_chain_total(MIB_ACL_IP_TBL);
		if (0 == vUInt)
			websWrite(wp, "disableDelButton();");
		return 0;
	}
else if (!strcmp(name, T("fc_page2"))) {
#if defined(ZTE_531B_BRIDGE_SC) && !defined(ZTE_GENERAL_ROUTER_SC)
		websWrite(wp, "disabled");
#endif
		return 0;
	}
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
#ifdef ZTE_GENERAL_ROUTER_SC
	else if(!strcmp(name,"openedPort")) {
		MIB_CE_VTL_SVR_T vtlSvrEntry;
		int vtlSvrEntryNums=mib_chain_total(MIB_VIRTUAL_SVR_TBL);
		int vtlSvrEntryIndex = 0;

		for(vtlSvrEntryIndex=0;vtlSvrEntryIndex<vtlSvrEntryNums;vtlSvrEntryIndex++)
		{
			if(!mib_chain_get(MIB_VIRTUAL_SVR_TBL,vtlSvrEntryIndex,&vtlSvrEntry))
			{
				printf("get MIB_VIRTUAL_SVR_TBL error!\n");
				return 0;
			}
			websWrite(wp, T("openedPort[%d]=new portInfo(%d,%d,\"%s\");\n"), 
				vtlSvrEntryIndex,vtlSvrEntry.wanStartPort,vtlSvrEntry.wanEndPort,vtlSvrEntry.svrName);
		}
		return 0;
	}
	else if (!strcmp(name, "rmtAccPort")) {
		MIB_CE_ACC_T Entry;
		int totalEntry = mib_chain_total(MIB_ACC_TBL);
		int entryIdx;
		int portArray[21];
		int i, idx=0;

		memset(portArray, 0, sizeof(portArray));
		for (entryIdx=0; entryIdx<totalEntry; entryIdx++) {
			if(!mib_chain_get(MIB_ACC_TBL, entryIdx, &Entry))
			{
				printf("get MIB_ACC_TBL error!\n");
				return 0;
			}
			if (Entry.telnet==0x01)
				portArray[idx++] = Entry.telnet_port;
			if (Entry.ftp==0x01)
				portArray[idx++] = Entry.ftp_port;
			if (Entry.web==0x01)
				portArray[idx++] = Entry.web_port;
		}
		if (idx) {
			for(i=0; i<idx; i++)
				websWrite(wp,T("%d,"),portArray[i]);
			websWrite(wp, T("%d"), 0);
		}
		
		return 0;
	}
#endif
	else if (!strcmp(name, T("wanip"))) {
		unsigned char tmpBuf[20];

		memcpy(tmpBuf, fcEntry.ipAddr, IP_ADDR_LEN);
		if ( ((struct in_addr *)tmpBuf)->s_addr == 0 )
			websWrite(wp, "0.0.0.0");
		else
			websWrite(wp, T("%s"),  inet_ntoa(*((struct in_addr *)tmpBuf)));
		
		return 0;
	}
	else if (!strcmp(name, T("ip"))) {
		unsigned char tmpBuf[20];

		memcpy(tmpBuf, fcEntry2.ipAddr, IP_ADDR_LEN);
		if ( ((struct in_addr *)tmpBuf)->s_addr == 0 ) {
			getMIB2Str(MIB_ADSL_LAN_IP, tmpBuf);
			websWrite(wp, T("%s"),  tmpBuf);
		} else {
			websWrite(wp, T("%s"),  inet_ntoa(*((struct in_addr *)tmpBuf)));
		}
		return 0;
	}
	else if (!strcmp(name, T("mask"))) {
		unsigned char tmpBuf[20];

		memcpy(tmpBuf, fcEntry2.netMask, IP_ADDR_LEN);
		if ( ((struct in_addr *)tmpBuf)->s_addr == 0 ) {
			getMIB2Str(MIB_ADSL_LAN_SUBNET, tmpBuf);
			websWrite(wp, T("%s"),  tmpBuf);
		} else
			websWrite(wp, T("%s"),  inet_ntoa(*((struct in_addr *)tmpBuf)));
		return 0;
	}
	else if (!strcmp(name, T("IP2"))) {
		if (fcEntry2.enable_ip2)
			websWrite(wp, "checked");
		return 0;
	}
	else if (!strcmp(name, T("enblDhcps"))) {
		if (fcEntry2.enable_dhcp)
			websWrite(wp, "checked");
		return 0;
	}
	else if (!strcmp(name, T("vpi-vci"))) {
		char tmpBuf[10];
		
		sprintf(tmpBuf, "%d / %d", fcEntry.vpi, fcEntry.vci);

		websWrite(wp, T("%s"), tmpBuf);
		return 0;
	}
	else if (!strcmp(name, T("conn-type"))) {
		char tmpBuf[60];

		memset(tmpBuf, 0, sizeof(tmpBuf));
		/*if (fcEntry.cmode == 0)
			strcpy(tmpBuf, "bridge ");
		if (fcEntry.encap == 1)
			strcpy(tmpBuf+strlen(tmpBuf), "LLC/SNAP");
		else if (fcEntry.encap == 0)
			strcpy(tmpBuf+strlen(tmpBuf), "VC-Mux");
		websWrite(wp, T("%s"), tmpBuf);*/
		switch(fcEntry.cmode)
		{
			case ADSL_BR1483:
				strcpy(tmpBuf, "1483 Bridged ");
				break;
			case ADSL_PPPoE:
				strcpy(tmpBuf, "PPPoE ");
				break;
			case ADSL_PPPoA:
				strcpy(tmpBuf, "PPPoA ");
				break;
			case ADSL_MER1483:
				strcpy(tmpBuf, "1483 MER ");
				break;
			case ADSL_RT1483:
				strcpy(tmpBuf, "1483 Routed ");
				break;
			default:
				break;
		}
		if (fcEntry.encap == 1)
			strcpy(tmpBuf+strlen(tmpBuf), "LLC/SNAP");
		else if (fcEntry.encap == 0)
			strcpy(tmpBuf+strlen(tmpBuf), "VC-Mux");

		if (fcEntry.cmode==2 || fcEntry.cmode==3)
		{//pppoe/ppp0a
			char tmpBuf2[20];
			switch(fcEntry.pppCtype)
			{
				case 0://connect forever
					strcpy(tmpBuf+strlen(tmpBuf), Tconn_forever);
					break;
				case 1://connect on demand
					sprintf(tmpBuf2, Tconn_on_dem, fcEntry.pppIdleTime);
					strcpy(tmpBuf+strlen(tmpBuf), tmpBuf2);
					break;
				case 2:
					strcpy(tmpBuf+strlen(tmpBuf), Tconn_manual);
					break;
			}
		}
		websWrite(wp, T("%s"), tmpBuf);
				
		return 0;
	}
	else if (!strcmp(name, T("lanip"))) {
		char tmpBuf[40];
		
		strcpy(tmpBuf, inet_ntoa(*((struct in_addr *)fcEntry2.ipAddr)));
		strcat(tmpBuf, " / ");
		strcat(tmpBuf, inet_ntoa(*((struct in_addr *)fcEntry2.netMask)));

		websWrite(wp, T("%s"), tmpBuf);
		return 0;
	}
	else if (!strcmp(name, T("seclanip"))) {
		char tmpBuf[40], strIp[IP_ADDR_LEN], strMask[IP_ADDR_LEN];;

		if (fcEntry2.enable_ip2) {
			memcpy(strIp, fcEntry2.lanIp2, IP_ADDR_LEN);
			memcpy(strMask, fcEntry2.subnetMask2, IP_ADDR_LEN);
			strcpy(tmpBuf, inet_ntoa(*((struct in_addr *)strIp)));
			strcat(tmpBuf+strlen(tmpBuf), " / ");
			strcat(tmpBuf+strlen(tmpBuf), inet_ntoa(*((struct in_addr *)strMask)));
		} else {
			strcpy(tmpBuf, "0.0.0.0 / 0.0.0.0");
		}

		websWrite(wp, T("%s"), tmpBuf);
		return 0;
	}
	else if (!strcmp(name, T("dhcps"))) {
		char tmpBuf[32];

		if (fcEntry2.enable_dhcp == 2)
			websWrite(wp, T("%s"), "启用");
		else
			websWrite(wp, T("%s"), "停用");
		
		return 0;
	}
#endif
	else if ( !strcmp(name, T("macFilterNum"))) {
		vUInt = mib_chain_total(MIB_MAC_FILTER_TBL);
		if (0 == vUInt)
			websWrite(wp, "disableDelButton();");
		return 0;
	}
#ifdef PARENTAL_CTRL
	else if( !strcmp(name, T("parentCtrlNum"))) {
			return 1;  //temp
		}
/*
	else if ( !strcmp(name, T("parentCtrlNum"))) {
		vUInt = mib_chain_total(MIB_MAC_FILTER_TBL);
		if (0 == vUInt)
			websWrite(wp, "disableDelButton();");
		return 0;
	}
*/
#endif
	else if ( !strcmp(name, T("vcMax"))) {
		vUInt = mib_chain_total(MIB_ATM_VC_TBL);
		if (vUInt >= 16) {
			websWrite(wp, "alert(\"Max number of ATM VC Settings is 16!\");");
			websWrite(wp, "return false;");
		}
		return 0;
	}
	else if ( !strcmp(name, T("vcCount"))) {
		vUInt = mib_chain_total(MIB_ATM_VC_TBL);
		if (vUInt == 0) {
			websWrite(wp, "disableButton(document.adsl.delvc);");
			// Commented by Mason Yu. The "refresh" button is be disabled on wanadsl.asp
			//websWrite(wp, "disableButton(document.adsl.refresh);");
		}
		return 0;
	}
	else if ( !strcmp(name, T("pppoeStatus")) ) {
		if (0) {
			websWrite(wp, "\n<script> setPPPConnected(); </script>\n");
		}
		return 0;
	}
#ifdef CONFIG_USER_PPPOE_PROXY
  else if(!strcmp(name,T("pppoeProxy")))
  	{
  	websWrite(wp,T("<tr><td><font size=2><b>PPPoE Proxy:</b></td>"
         "<td><b><input type=\"radio\" value=1 name=\"pppEnable\" >Enable&nbsp;&nbsp;"
	"<input type=\"radio\" value=0 name=\"pppEnable\" checked>Disable</b></td></tr>"));
  	}
  else if(!strcmp(name,T("pppSettingsDisable")))
  	{
  	  websWrite(wp,T("{document.adsl.pppEnable[0].disabled=true;\n"
	  	"document.adsl.pppEnable[1].disabled=true;}"));
  	}
    else if(!strcmp(name,T("pppSettingsEnable")))
  	{
  	  websWrite(wp,T("{document.adsl.pppEnable[0].disabled=false;\n"
	  	"document.adsl.pppEnable[1].disabled=false;}else{document.adsl.pppEnable[0].disabled=true;\n"
	  	"document.adsl.pppEnable[1].disabled=true;}"
	  	"document.adsl.pppEnable[0].checked=false;"
	  	"document.adsl.pppEnable[1].checked=true;"));
  	} 

 #endif
  #ifdef CONFIG_USER_PPPOE_PROXY
     else if(!strcmp(name,T("PostVC")))
     	{
     	   websWrite(wp,T("function postVC(vpi,vci,encap,napt,mode,username,passwd,pppType,idletime,pppoeProxyEnable,ipunnum,ipmode,ipaddr,remoteip,netmask,droute,status,enable)"));
     	}
     else if(!strcmp(name,T("pppoeProxyEnable")))
     	{
	websWrite(wp,T("  if(mode==\"PPPoE\")"
		"{if(pppoeProxyEnable)"
		"{ document.adsl.pppEnable[0].checked=true;\n"
                  "document.adsl.pppEnable[1].checked=false;}\n"			
		"else {document.adsl.pppEnable[0].checked=false;"
		 " document.adsl.pppEnable[1].checked=true;}  "
		" document.adsl.pppEnable[0].disabled=false;"
			  " document.adsl.pppEnable[1].disabled=false;"));	
	websWrite(wp,T(" }else"
		"{"
		"	  document.adsl.pppEnable[0].checked=false;"
		"	   document.adsl.pppEnable[1].checked=true;"
		"	   document.adsl.pppEnable[0].disabled=true;"
		"	   document.adsl.pppEnable[1].disabled=true;}"
		));
     	}
  #else
   else if(!strcmp(name,T("PostVC")))
     	{
     	   websWrite(wp,T("function postVC(vpi,vci,encap,napt,mode,username,passwd,pppType,idletime,ipunnum,ipmode,ipaddr,remoteip,netmask,droute,status,enable)"));
     	}
     else if(!strcmp(name,T("pppoeProxyEnable")))
     	{
     	 
     	}
  #endif

	else if ( !strcmp(name, T("adsl-line-mode")) ) {
		if ( !mib_get( MIB_ADSL_MODE, (void *)&vChar) )
			return -1;
		if (1 == vChar) {
			websWrite(wp,"<option selected value=\"1\">T1.413</option>");
			websWrite(wp,"<option value=\"2\">G.dmt</option>");
			websWrite(wp,"<option value=\"3\">MultiMode</option>");
		} else if (2 == vChar) {
			websWrite(wp,"<option value=\"1\">T1.413</option>");
			websWrite(wp,"<option selected value=\"2\">G.dmt</option>");
			websWrite(wp,"<option value=\"3\">MultiMode</option>");
		} else if (3 == vChar) {
			websWrite(wp,"<option value=\"1\">T1.413</option>");
			websWrite(wp,"<option value=\"2\">G.dmt</option>");
			websWrite(wp,"<option selected value=\"3\">MultiMode</option>");
		}		
		return 0;
	}
#ifdef WLAN_SUPPORT
#ifdef WLAN_8185AG
	else if ( !strcmp(name, T("wl_txRate"))) {
		mib_get( MIB_WLAN_BAND, (void *)&vChar);
		websWrite(wp, "band=%d\n",vChar);
#ifdef CONFIG_USB_RTL8192SU_SOFTAP		
		mib_get( MIB_WLAN_FIX_RATE, (void *)&vUInt);
		websWrite(wp, "txrate=%d\n",vUInt);
#else
		mib_get( MIB_WLAN_FIX_RATE, (void *)&vUShort);
		websWrite(wp, "txrate=%d\n",vUShort);
#endif
		mib_get( MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&vChar);
		websWrite(wp, "auto=%d\n",vChar);
		websWrite(wp, "rf_num=2\n");	//cathy, for rtl8192su: "2T"2R
	}
#endif
//added by xl_yue:
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	else if ( !strcmp(name, T("wl_domain"))) {
		struct wlCountryReg * pwlCountryReg = wlCountryRegList;
		//selIndex must start from 0, it should keep accordance with wlCountryRegList's index,
		//because in formWlanSetup(), the value of select option will be used as the array's index of wlCountryRegList
		//and in initWlBasicPage(), the regDomainList[] is start from 0
		int selIndex = 0;
		while(strlen(pwlCountryReg->country)){
			websWrite(wp, "<option value=\"%d\">%s</option>\n",selIndex,pwlCountryReg->country);
			pwlCountryReg++;
			selIndex++;
		}
	}
#endif
	else if ( !strcmp(name, T("wl_chno"))) {
//xl_yue:
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		mib_get( MIB_WLAN_REG_DOMAIN, (void *)&vChar);
#else
		mib_get( MIB_HW_REG_DOMAIN, (void *)&vChar);
#endif
		websWrite(wp, "regDomain=%d\n",vChar);
		mib_get( MIB_WLAN_CHAN_NUM, (void *)&vChar);
		websWrite(wp, "defaultChan=%d\n",vChar);
	}
#endif

	//for web log
	else if ( !strcmp(name, T("log-cap0")) ) {
   		if ( !mib_get( MIB_SYSLOG, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("log-cap1")) ) {
   		if ( !mib_get( MIB_SYSLOG, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			websWrite(wp, "checked");
		return 0;
	}
	if (!strcmp(name, T("syslog-log")) || !strcmp(name, T("syslog-display"))) {
		char *SYSLOGLEVEL[] = {"Emergency", "Alert", "Critical", "Error", "Warning", "Notice", "Infomational", "Debugging"};
		int i;
		if (!strcmp(name, T("syslog-log"))) {
			if (!mib_get(MIB_SYSLOG_LOG_LEVEL, (void *)&vChar))
				return -1;
		}
		else if (!strcmp(name, T("syslog-display"))) {
			if (!mib_get(MIB_SYSLOG_DISPLAY_LEVEL, (void *)&vChar))
				return -1;
		}
		for (i=0; i<8; i++) {
			if (i == vChar)
				websWrite(wp,T("<option selected value=\"%d\">%s</option>"), i, SYSLOGLEVEL[i]);
			else
				websWrite(wp,T("<option value=\"%d\">%s</option>"), i, SYSLOGLEVEL[i]);
		}
		return 0;
	}
#ifdef SYSLOG_REMOTE_LOG
	if ( !strcmp(name, T("syslog-mode")) ) {
		char *SYSLOGMODE[] = {"Local", "Remote", "Both"};
		int i;
		if (!mib_get(MIB_SYSLOG_MODE, (void *)&vChar))
			return -1;
		for (i=0; i<3; i++) {
			if (i == vChar - 1)
				websWrite(wp,T("<option selected value=\"%d\">%s</option>"), i+1, SYSLOGMODE[i]);
			else
				websWrite(wp,T("<option value=\"%d\">%s</option>"), i+1, SYSLOGMODE[i]);
		}
	}
#endif

	//for adsl debug
	else if ( !strcmp(name, T("adsldbg-cap0")) ) {
   		if ( !mib_get( MIB_ADSL_DEBUG, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("adsldbg-cap1")) ) {
   		if ( !mib_get( MIB_ADSL_DEBUG, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			websWrite(wp, "checked");
		return 0;
	}
#ifdef _CWMP_MIB_
	else if ( !strcmp(name, T("tr069-interval")) ) {
   		if ( !mib_get( CWMP_INFORM_ENABLE, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			websWrite(wp, "disabled");
		return 0;
	}
	else if ( !strcmp(name, T("tr069-inform-0")) ) {
   		if ( !mib_get( CWMP_INFORM_ENABLE, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("tr069-inform-1")) ) {
   		if ( !mib_get( CWMP_INFORM_ENABLE, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("tr069-dbgmsg-0")) ) {
   		if ( !mib_get( CWMP_FLAG, (void *)&vChar) )
			return -1;
		if ( (vChar & CWMP_FLAG_DEBUG_MSG)==0 )
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("tr069-dbgmsg-1")) ) {
   		if ( !mib_get( CWMP_FLAG, (void *)&vChar) )
			return -1;
		if ( (vChar & CWMP_FLAG_DEBUG_MSG)!=0 )
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("tr069-certauth-0")) ) {
   		if ( !mib_get( CWMP_FLAG, (void *)&vChar) )
			return -1;
		if ( (vChar & CWMP_FLAG_CERT_AUTH)==0 )
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("tr069-certauth-1")) ) {
   		if ( !mib_get( CWMP_FLAG, (void *)&vChar) )
			return -1;
		if ( (vChar & CWMP_FLAG_CERT_AUTH)!=0 )
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("tr069-sendgetrpc-0")) ) {
   		if ( !mib_get( CWMP_FLAG, (void *)&vChar) )
			return -1;
		if ( (vChar & CWMP_FLAG_SENDGETRPC)==0 )
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("tr069-sendgetrpc-1")) ) {
   		if ( !mib_get( CWMP_FLAG, (void *)&vChar) )
			return -1;
		if ( (vChar & CWMP_FLAG_SENDGETRPC)!=0 )
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("tr069-skipmreboot-0")) ) {
   		if ( !mib_get( CWMP_FLAG, (void *)&vChar) )
			return -1;
		if ( (vChar & CWMP_FLAG_SKIPMREBOOT)==0 )
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("tr069-skipmreboot-1")) ) {
   		if ( !mib_get( CWMP_FLAG, (void *)&vChar) )
			return -1;
		if ( (vChar & CWMP_FLAG_SKIPMREBOOT)!=0 )
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("tr069-delay-0")) ) {
   		if ( !mib_get( CWMP_FLAG, (void *)&vChar) )
			return -1;
		if ( (vChar & CWMP_FLAG_DELAY)==0 )
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("tr069-delay-1")) ) {
   		if ( !mib_get( CWMP_FLAG, (void *)&vChar) )
			return -1;
		if ( (vChar & CWMP_FLAG_DELAY)!=0 )
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("tr069-autoexec-0")) ) {
   		if ( !mib_get( CWMP_FLAG, (void *)&vChar) )
			return -1;
		if ( (vChar & CWMP_FLAG_AUTORUN)==0 )
			websWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, T("tr069-autoexec-1")) ) {
   		if ( !mib_get( CWMP_FLAG, (void *)&vChar) )
			return -1;
		if ( (vChar & CWMP_FLAG_AUTORUN)!=0 )
			websWrite(wp, "checked");
		return 0;
	}

#endif
#ifdef WLAN_SUPPORT
#ifdef CONFIG_WIFI_SIMPLE_CONFIG // WPS
	else if (!strcmp(name, T("wscConfig-0")) ) {
		if (!mib_get(MIB_WSC_CONFIGURED, (void *)&vChar))
			return -1;
		if (!vChar) websWrite(wp, "checked");
		return 0;
	}
	else if (!strcmp(name, T("wscConfig-1"))) {
		if (!mib_get(MIB_WSC_CONFIGURED, (void *)&vChar))
			return -1;
		if (vChar)
			websWrite(wp, "checked");
		return 0;
	}
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	else if (!strcmp(name, T("isWscConfiged"))) {
		if (!mib_get(MIB_WSC_CONFIGURED, (void *)&vChar))
			return -1;
		if (vChar)
			websWrite(wp, "已配置加密模式");
		else
			websWrite(wp, "未配置加密模式");
		return 0;
	}
#endif
	else if (!strcmp(name, T("wscConfig-A"))) {
		if (!mib_get(MIB_WSC_CONFIGURED, (void *)&vChar))
			return -1;
		if (vChar)
			websWrite(wp, "isConfig=1;");
		else
			websWrite(wp, "isConfig=0;");
		return 0;
	}
#ifdef CONFIG_USB_RTL8192SU_SOFTAP		// add by yq_zhou 1.20
	else if (!strcmp(name,T("wscConfig"))){          
		if (!mib_get(MIB_WSC_CONFIGURED, (void *)&vChar))
			return -1;
		if (vChar)
			websWrite(wp, "enableButton(form.elements['resetUnConfiguredBtn']);");
		else
			websWrite(wp, "disableButton(form.elements['resetUnConfiguredBtn']);");
		return 0;
	}
	else if (!strcmp(name,T("protectionDisabled-0"))){ 	
		if (!mib_get(MIB_WLAN_PROTECTION_DISABLED,(void *)&vChar))
			return -1;
		if (!vChar)
			websWrite(wp,"checked");
	}
	else if (!strcmp(name,T("protectionDisabled-1"))){
		if (!mib_get(MIB_WLAN_PROTECTION_DISABLED,(void *)&vChar))
			return -1;
		if (vChar)
			websWrite(wp,"checked");
	}
	else if (!strcmp(name,T("aggregation-0"))){
		if (!mib_get(MIB_WLAN_AGGREGATION,(void *)&vChar))
			return -1;
		if (vChar)
			websWrite(wp,"checked");
	}
	else if (!strcmp(name,T("aggregation-1"))){
		if (!mib_get(MIB_WLAN_AGGREGATION,(void *)&vChar))
			return -1;
		if (vChar)
			websWrite(wp,"checked");
	}
	else if (!strcmp(name,T("shortGIEnabled-0"))){
		if (!mib_get(MIB_WLAN_SHORTGI_ENABLED,(void *)&vChar))
			return -1;
		if (vChar)
			websWrite(wp,"checked");
	}
	else if (!strcmp(name,T("shortGIEnabled-1"))){
		if (!mib_get(MIB_WLAN_SHORTGI_ENABLED,(void *)&vChar))
			return -1;
		if (vChar)
			websWrite(wp,"checked");
	}
#endif
	else if (!strcmp(name, T("wlanMode")))  {
		if (!mib_get(MIB_WLAN_MODE, (void *)&vChar))
			return -1;
		if (vChar == CLIENT_MODE)
			websWrite(wp, "isClient=1;");
		else
			websWrite(wp, "isClient=0;");
		return 0;
		
	}
	else if (!strcmp(name, T("wscDisable")))  {
		
		if (!mib_get(MIB_WSC_DISABLE, (void *)&vChar))
			return -1;
		if (vChar)
			websWrite(wp, "checked");
		return 0;		
	}
	else if (!strcmp(name, T("wps_auth")))  {
		
		if (!mib_get(MIB_WSC_AUTH, (void *)&vChar))
			return -1;
		switch(vChar) {
			case WSC_AUTH_OPEN: websWrite(wp, "Open"); break;
			case WSC_AUTH_WPAPSK: websWrite(wp, "WPA PSK"); break;
			case WSC_AUTH_SHARED: websWrite(wp, "WEP Shared"); break;
			case WSC_AUTH_WPA: websWrite(wp, "WPA Enterprise"); break;
			
			case WSC_AUTH_WPA2: websWrite(wp, "WPA2 Enterprise"); break;
			case WSC_AUTH_WPA2PSK: websWrite(wp, "WPA2 PSK"); break;
			case WSC_AUTH_WPA2PSKMIXED: websWrite(wp, "WPA2-Mixed PSK"); break;
			default:
				break;
		}
		return 0;
	}
	else if (!strcmp(name, T("wps_enc")))  {
		
		if (!mib_get(MIB_WSC_ENC, (void *)&vChar))
			return -1;
		switch(vChar) {
			case 0:
			case WSC_ENCRYPT_NONE: websWrite(wp, "None"); break;
			case WSC_ENCRYPT_WEP: websWrite(wp, "WEP"); break;
			case WSC_ENCRYPT_TKIP: websWrite(wp, "TKIP"); break;
			case WSC_ENCRYPT_AES: websWrite(wp, "AES"); break;			
			case WSC_ENCRYPT_TKIPAES: websWrite(wp, "TKIP+AES"); break;
			default:
				break;
		}
		return 0;
	}
#endif
#endif
	// Jenny, for RFC1577
	if ( !strcmp(name, T("adslcmode")) ) {
#ifdef CONFIG_ATM_CLIP
		websWrite(wp, "<option value=\"5\">1577 Routed</option>" );
#endif
		return 0;
	}
#ifndef CONFIG_GUI_WEB
//add by ramen for create new menu 
       else if(!strcmp(name,T("CreateMenu"))){

	   #ifdef WEB_MENU_USE_NEW
	   createMenu(0,wp,0,0);
	   #else
	   addMenuJavaScript( wp,0,0);
	   char *argv[]={"1",0};
	   if(!strcmp(argv[0],"1"))
	                     createMenu(0,wp,2,argv);
	   websWrite(wp,T("</script>\n</head>\n"));
	   websWrite(wp,T("<body onload=\"MTMStartMenu(true)\" bgcolor=\"#000033\" text=\"#ffffcc\" link=\"yellow\" vlink=\"lime\" alink=\"red\">\n"));
	   if(!strcmp(argv[0],"0"))
	                     {
	                      websWrite(wp,T("<table width=100%% border=0 cellpadding=0 cellspacing=0>\n<tr><td width=20%%>&nbsp&nbsp</td><td  align=left>\n"));
	                     createMenu(0,wp,2,argv);
			   websWrite(wp,T("</td></tr>\n</table>\n"));
	   	     }
	 
	   #endif	
	   websWrite(wp,T("</body>\n</html>\n"));
	   	return 0;
       	}if(!strcmp(name,T("CreateMenu_user")))
       	
       		{
       		createMenu_user(0,wp,0,0);
			return 0;
       		}

	else if(!strcmp(name,T("naptEnable")))
	{
		websWrite(wp,T("\tif ((document.adsl.adslConnectionMode.selectedIndex == 1) ||\n"
			"\t\t(document.adsl.adslConnectionMode.selectedIndex == 2))\n"
			"\t\tdocument.adsl.naptEnabled.checked = true;\n"
			"\telse\n"			
			"\t\tdocument.adsl.naptEnabled.checked = false;\n"));
		return 0;
	}
#else
	else if(!strcmp(name,T("naptEnable")))
	{
		websWrite(wp,T("\tif ((document.adsl.adslConnectionMode.selectedIndex == 1) ||\n"
			"\t\t(document.adsl.adslConnectionMode.selectedIndex == 2) || (document.adsl.adslConnectionMode.selectedIndex == 3))\n"
			"\t\tdocument.adsl.naptEnabled.checked = true;\n"
			"\telse\n"			
			"\t\tdocument.adsl.naptEnabled.checked = false;\n"));
		return 0;
	}
#endif
	//add by ramen for zte acl default ip
	else if(!strcmp(name,T("remoteClientIp")))
		{
		  websWrite(wp,T("%s"),wp->remote_ip_addr);
			return 0;
		}
	   //add by ramen shen for the remote managment for zte
	   #ifdef ZTE_GENERAL_ROUTER_SC	
	   else if(!strcmp(name,T("WANInterfaceList")))
       		{
				MIB_CE_ATM_VC_T Entry;
				int entryNum;
				int mibcnt;		
				char interfacename[MAX_NAME_LEN];
				entryNum = mib_chain_total(MIB_ATM_VC_TBL);
				unsigned char forSelect=0;
		            for(mibcnt=0;mibcnt<entryNum;mibcnt++)
		            {
		            if (!mib_chain_get(MIB_ATM_VC_TBL, mibcnt, (void *)&Entry))
						{
		  					websError(wp, 400, T("Get chain record error!\n"));
							return -1;
						}
			      if(Entry.cmode!=ADSL_BR1483)// ADSL_BR1483 ADSL_MER1483 ADSL_PPPoE ADSL_PPPoA	ADSL_RT1483	ADSL_RT1577
			      	{
			      	 #ifdef CTC_WAN_NAME
					if(Entry.enable == 0)
						continue;
					getWanName(&Entry, interfacename);
//					sprintf(interfacename,"Internet_R_%u_%u",Entry.vpi,Entry.vci);
				#else
				  	if((Entry.cmode == ADSL_PPPoE) || (Entry.cmode == ADSL_PPPoA) ) {
							 sprintf(interfacename, "PPP%u", PPP_INDEX(Entry.ifIndex));
				  	}else
				  	{
				  	                     sprintf(interfacename, "vc%u", PPP_INDEX(Entry.ifIndex));
				  	}
		             #endif
					if(forSelect==0)
						{
						  websWrite(wp,T("\n<td>选择端口</td>"
						  	"<td colspan=\"2\"><select size=\"1\" name=\"interfaceName\" onChange=\"interfaceNameSelection()\">"));
						  websWrite(wp,T("<option selected value=\"%d\">%s</option>"),mibcnt,interfacename);					
						}
					else {
						websWrite(wp,T("<option  value=\"%d\">%s</option>"),mibcnt,interfacename);						
						}
					forSelect++;
					 
		                    }
				 
		            }
		            if(forSelect==0) websWrite(wp,T("<td>您还没有配置路由模式的PVC，请点击<a href=\"wanadsl_sc.asp\">广域网接口配置</a></td>\n"));
				else websWrite(wp,T("</select></td>\n"));
				
       		return 0;
       			}else if(!strcmp(name,"interfaceNameSelection"))
       				{
       				int accIndex=0,nBytesSent=0;
				char strDocAcc[]="document.acc.";								
				MIB_CE_ACC_T Entry;
				int accTotalNums=mib_chain_total(MIB_ACC_TBL);
				for(accIndex=0;accIndex<accTotalNums;accIndex++)
				{
				if(!mib_chain_get(MIB_ACC_TBL,accIndex,&Entry))
					{
					printf("get MIB_ACC_TBL enttry failed\n");
					return 0;
					}
				if(Entry.protocol != ADSL_BR1483)
					{
					nBytesSent+=websWrite(wp,T("if(document.acc.interfaceName.value ==\"%d\")\n{\n"),accIndex);
					#ifdef CONFIG_USER_TELNETD_TELNETD
						if (Entry.telnet & 0x01)
						nBytesSent += websWrite(wp, T("%sw_telnet.checked=true;\n"), strDocAcc);
						else
							nBytesSent += websWrite(wp, T("%sw_telnet.checked=false;\n"), strDocAcc);
					#endif
					#ifdef CONFIG_USER_FTPD_FTPD
						if (Entry.ftp & 0x01)
						nBytesSent += websWrite(wp, T("%sw_ftp.checked=true;\n"), strDocAcc);
						else
							nBytesSent += websWrite(wp, T("%sw_ftp.checked=false;\n"), strDocAcc);
					#endif
					#ifdef CONFIG_USER_TFTPD_TFTPD
						if (Entry.tftp & 0x01)
						nBytesSent += websWrite(wp, T("%sw_tftp.checked=true;\n"), strDocAcc);
						else
							nBytesSent += websWrite(wp, T("%sw_tftp.checked=false;\n"), strDocAcc);
					#endif
						if (Entry.web & 0x01)
						nBytesSent += websWrite(wp, T("%sw_web.checked=true;\n"), strDocAcc);
						else nBytesSent += websWrite(wp, T("%sw_web.checked=false;\n"), strDocAcc);
					#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
						if (Entry.snmp & 0x01)
						nBytesSent += websWrite(wp, T("%sw_snmp.checked=true;\n"), strDocAcc);
						else
							nBytesSent += websWrite(wp, T("%sw_snmp.checked=false;\n"), strDocAcc);
					#endif
					#ifdef CONFIG_USER_SSH_DROPBEAR
						if (Entry.ssh & 0x01)
						nBytesSent += websWrite(wp, T("%sw_ssh.checked=true;\n"), strDocAcc);
						else nBytesSent += websWrite(wp, T("%sw_ssh.checked=false;\n"), strDocAcc);
					#endif
						if (Entry.icmp & 0x01)							
						nBytesSent += websWrite(wp, T("%sw_icmp.checked=true;\n"), strDocAcc);
						else nBytesSent += websWrite(wp, T("%sw_icmp.checked=false;\n"), strDocAcc);
					#ifdef CONFIG_USER_TELNETD_TELNETD	
						nBytesSent += websWrite(wp, T("%sw_telnet_port.value=%d;\n"), strDocAcc, Entry.telnet_port);
					#endif
					#ifdef CONFIG_USER_FTPD_FTPD
					         nBytesSent += websWrite(wp, T("%sw_ftp_port.value=%d;\n"), strDocAcc, Entry.ftp_port);	
					#endif
					         nBytesSent += websWrite(wp, T("%sw_web_port.value=%d;\n"), strDocAcc, Entry.web_port);
						nBytesSent +=websWrite(wp,T("document.getElementById(\"accSetting\").style.display=\"\";\n"));
						nBytesSent +=websWrite(wp,T("return true;\n}\n"));
					}	
				}
			       		return 0;
       			}
#endif
		else if(!strcmp(name,"pwdManagment"))
       	{
       		if(!strcmp(wp->user,suName))
       			websWrite(wp,T("<form action=/goform/formPasswordSetup method=POST name=\"password\">"));
			else
				websWrite(wp,T("<form action=/goform/admin/formPasswordSetup method=POST name=\"password\">"));
       		return 0;
       	}
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
else if(!strcmp(name,"IsACLEnabled"))
       					{
       					if ( !mib_get( MIB_ACL_CAPABILITY, (void *)&vChar) )
						return -1;					
						websWrite(wp, T(" %d "),vChar);     
				                  return 0;
						}
#endif
#ifdef DEFAULT_GATEWAY_V2
	// Jenny, for PPPoE auto route
	else if ( !strcmp(name, T("autort")) ) {
#ifdef AUTO_PPPOE_ROUTE
		websWrite(wp, "<option value=239>Auto</option>" );
#endif
		return 0;
	}
#endif
	else if ( !strcmp(name, T("pppExist")) ) {
		MIB_CE_ATM_VC_T Entry;
		unsigned int totalEntry;
		int i, isPPP=0;
		totalEntry = mib_chain_total(MIB_ATM_VC_TBL); /* get chain record size */
		for (i=0; i<totalEntry; i++)
			if (mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
				if (Entry.enable == 1 && (Entry.cmode == ADSL_PPPoE || Entry.cmode == ADSL_PPPoA)) {
					isPPP = 1;
					break;
				}
		if (isPPP == 0)
			websWrite(wp,T("document.pppauth.username.disabled = true;\n"
							"document.pppauth.password.disabled = true;\n"
							"document.all.Submit.disabled = true;\n"));
		return 0;
	}
#ifdef CONFIG_IP_NF_ALG_ONOFF
	else if(!strcmp(name,T("GetAlgType")))
		{
		GetAlgTypes(wp);
		return 0;
		}
	else if(!strcmp(name,T("AlgTypeStatus")))
		{
		CreatejsAlgTypeStatus( wp);
	 	return 0;
		}
#endif
#ifdef DNS_BIND_PVC_SUPPORT
	else if(!strcmp(name,T("DnsBindPvc")))
		{	
		unsigned char dnsBindPvcEnable=0;
		mib_get(MIB_DNS_BIND_PVC_ENABLE,(void*)&dnsBindPvcEnable);
		printf("dns bind pvc = %d\n",dnsBindPvcEnable);
		websWrite(wp,T("<font size=2>开启DNS绑定:<input type=\"checkbox\" name=\"enableDnsBind\" value=\"on\" %s onClick=\"DnsBindPvcClicked();\"></font>"),
			(dnsBindPvcEnable)?"checked":"");
		return 0;
		}
#endif
	else  if(!strcmp(name,"WanPvcRouter"))
		{
#ifdef DNS_BIND_PVC_SUPPORT
				MIB_CE_ATM_VC_T Entry;
				int entryNum;
				int mibcnt;		
				char interfacename[MAX_NAME_LEN];
				entryNum = mib_chain_total(MIB_ATM_VC_TBL);
				unsigned char forSelect=0;
		            for(mibcnt=0;mibcnt<entryNum;mibcnt++)
		            {
		            if (!mib_chain_get(MIB_ATM_VC_TBL, mibcnt, (void *)&Entry))
						{
		  					websError(wp, 400, T("Get chain record error!\n"));
							return -1;
						}
			      if(Entry.cmode!=ADSL_BR1483)// ADSL_BR1483 ADSL_MER1483 ADSL_PPPoE ADSL_PPPoA	ADSL_RT1483	ADSL_RT1577
			      	{
			      	websWrite(wp,"0");
				return 0;
		                  }
				 
		            }
		           websWrite(wp,T("1"));
#else
	 		 websWrite(wp,T("0"));
#endif
			return 0;
				
		}

#ifdef DNS_BIND_PVC_SUPPORT
	else if(!strcmp(name,T("dnsBindPvcInit")))
			{
				unsigned char dnspvc1,dnspvc2,dnspvc3;
				if(!mib_get(MIB_DNS_BIND_PVC1,(void*)&dnspvc1))
					{
					websError(wp, 400, T("Get MIB_DNS_BIND_PVC1 record error!\n"));
							return -1;
					}
				if(!mib_get(MIB_DNS_BIND_PVC2,(void*)&dnspvc2))
					{
					websError(wp, 400, T("Get MIB_DNS_BIND_PVC2 record error!\n"));
							return -1;
					}
				if(!mib_get(MIB_DNS_BIND_PVC3,(void*)&dnspvc3))
					{
					websError(wp, 400, T("Get MIB_DNS_BIND_PVC3 record error!\n"));
							return -1;
					}
			
				    websWrite(wp,T("DnsBindSelectdInit('wanlist1',%d);\n"),dnspvc1);
				    websWrite(wp,T("DnsBindSelectdInit('wanlist2',%d);\n"),dnspvc2);
				    websWrite(wp,T("DnsBindSelectdInit('wanlist3',%d);\n"),dnspvc3);
				websWrite(wp,T("DnsBindPvcClicked();"));
				return 0;
			}
#endif	
	else if(!strcmp(name,T("QosSpeedLimitWeb")))
		{
#ifdef QOS_SPEED_LIMIT_SUPPORT
	         websWrite(wp,T("<td>\n"
		"<input type=\"checkbox\" name= qosspeedenable onClick=\"qosSpeedClick(this)\"; > <font size=2>限速</font>\n"
		"</td>\n "
	         "<td>\n"
	         "<div id='speedlimit' style=\"display:none\">\n"
		"<table>\n"
		"<tr>\n<td>\n"
		"<input type=text name=speedLimitRank  size=6 maxlength=5 >\n"
		"</td>\n<td><font size=2>\nkBps< bytes/sec * 1024></td>\n</tr>\n</table>\n"
		"</div>\n"	         		
		"</td>\n"));
#endif
		return 0;
		}
#if defined(CONFIG_USER_ZEBRA_OSPFD_OSPFD) || defined(CONFIG_USER_ROUTED_ROUTED)
	else if (!strcmp(name, T("ospf"))) {
#ifdef CONFIG_USER_ROUTED_ROUTED
		websWrite(wp, T("	<option value=\"0\">RIP</option>\n"));
#endif
#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
		websWrite(wp, T("	<option value=\"1\">OSPF</option>"));
#endif
	}
#endif

#ifdef ZTE_GENERAL_ROUTER_SC
	else if(!strcmp(name, T("tr069status"))){
		FILE *fp;
		fp=fopen("/var/run/cwmp.pid","r");
		if(fp==NULL)
			websWrite(wp, T("已关闭"));
		else{
			websWrite(wp, T("运行中..."));
			fclose(fp);
		}
	}
	else if(!strcmp(name, T("snmpstatus"))){
		FILE *fp;
		fp=fopen("/var/run/snmpd.pid","r");
		if(fp==NULL)
			websWrite(wp, T("已关闭"));
		else{
			websWrite(wp, T("运行中..."));
			fclose(fp);
		}
	}
#endif
	else if(!strcmp(name, T("dgw"))){
#ifdef DEFAULT_GATEWAY_V1
		websWrite(wp, T("\tif (droute == 1)\n"));
		websWrite(wp, T("\t\tdocument.adsl.droute[1].checked = true;\n"));
		websWrite(wp, T("\telse\n"));
		websWrite(wp, T("\t\tdocument.adsl.droute[0].checked = true;\n"));
#else
		GetDefaultGateway(eid, wp, argc, argv);
		websWrite(wp, T("\tautoDGWclicked();\n"));
#endif
	}
/* add by yq_zhou 09.2.02 add sagem logo for 11n*/
	else if(!strncmp(name, T("title"), 5))	{
#ifndef CONFIG_11N_SAGEM_WEB
		websWrite(wp,T(	"<img src=\"graphics/topbar.png\" width=900 height=60 border=0>"));
#else
		websWrite(wp,T(	"<img src=\"graphics/sagemlogo1.gif\" width=1350 height=60 border=0>"));
#endif
	}
	else if(!strncmp(name, T("logobelow"), 9))	{
#ifdef CONFIG_11N_SAGEM_WEB
		websWrite(wp,T(	"<img src=\"graphics/sagemlogo2.gif\" width=180 height=60 border=0>"));
#endif
	}
	else
		return -1;
	
	return 0;
}

void write_wladvanced(int eid, webs_t wp, int argc, char_t **argv)        //add by yq_zhou 1.20
{
	websWrite(wp,T( 
     "<tr><td width=\"30%%\"><font size=2><b>Preamble Type:</b></td>\n"
     "<td width=\"70%%\"><font size=2>\n"
     "<input type=\"radio\" value=\"long\" name=\"preamble\">Long Preamble&nbsp;&nbsp;\n"
     "<input type=\"radio\" name=\"preamble\" value=\"short\">Short Preamble</td></tr>\n"));
	websWrite(wp,T( 
     "<tr><td width=\"30%%\"><font size=2><b>Broadcast SSID:</b></td>\n"
     "<td width=\"70%%\"><font size=2>\n"
     "<input type=\"radio\" name=\"hiddenSSID\" value=\"no\">Enabled&nbsp;&nbsp;\n"
     "<input type=\"radio\" name=\"hiddenSSID\" value=\"yes\">Disabled</td></tr>\n"));
	websWrite(wp,T(
     "<tr><td width=\"30%%\"><font size=2><b>Relay Blocking:</b></td>\n"
     "<td width=\"70%%\"><font size=2>\n"
     "<input type=\"radio\" name=block value=1>Enabled&nbsp;&nbsp;\n"
     "<input type=\"radio\" name=block value=0>Disabled</td></tr>\n")); 
#ifdef CONFIG_USB_RTL8192SU_SOFTAP	
	websWrite(wp,T(
     "<tr><td width=\"30%%\"><font size=2><b>Protection:</b></td>\n"
     "<td width=\"70%%\"><font size=2>\n"
     "<input type=\"radio\" name=\"protection\" value=\"yes\">Enabled&nbsp;&nbsp;\n"
     "<input type=\"radio\" name=\"protection\" value=\"no\">Disabled</td></tr>\n"));
     	websWrite(wp,T(
     "<tr><td width=\"30%%\"><font size=2><b>Aggregation:</b></td>\n"
     "<td width=\"70%%\"><font size=2>\n"
     "<input type=\"radio\" name=\"aggregation\" value=\"enable\">Enabled&nbsp;&nbsp;\n"
     "<input type=\"radio\" name=\"aggregation\" value=\"disable\">Disabled</td></tr>\n"));
       websWrite(wp,T(
     "<tr id=\"ShortGi\" style=\"display:\">\n"
     "<td width=\"30%%\"><font size=2><b>Short GI:</b></td>\n"
     "<td width=\"70%%\"><font size=2>\n"
     "<input type=\"radio\" name=\"shortGI0\" value=\"on\">Enabled&nbsp;&nbsp;\n"
     "<input type=\"radio\" name=\"shortGI0\" value=\"off\">Disabled</td></tr>\n"));
#endif
}

/* add by yq_zhou 09.2.02 add sagem logo for 11n*/
#if 0
void write_title(int eid, webs_t wp, int argc, char_t **argv)  
{
	printf("%s ...............1\n",__FUNCTION__);
#ifndef CONFIG_11N_SAGEM_WEB
	websWrite(wp,T(	"<img src=\"graphics/topbar.png\" width=900 height=60 border=0>"));
#else
	websWrite(wp,T(	"<img src=\"graphics/sagemlogo1.gif\" width=1350 height=60 border=0>"));
#endif
}

void write_logo_below(int eid, webs_t wp, int argc, char_t **argv)  
{
#ifdef CONFIG_11N_SAGEM_WEB
	printf("%s ...............1\n",__FUNCTION__);
	websWrite(wp,T(
	"<img src=\"graphics/sagemlogo2.gif\" width=160 height=80 border=0>"));
#endif
}
#endif
	
// Kaohj
#if 0
int getIndex(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t *name;
	char_t  buffer[100];

	unsigned char vChar;
	unsigned short vUShort;
	unsigned int vUInt;

   	if (ejArgs(argc, argv, T("%s"), &name) < 1) {
   		websError(wp, 400, T("Insufficient args\n"));
   		return -1;
   	}

	memset(buffer,0x00,100);
   	if ( !strcmp(name, T("device-type")) ) {
 		if ( !mib_get( MIB_DEVICE_TYPE, (void *)&vChar) )
			return -1;
#ifdef __uClinux__
		sprintf(buffer, "%u", vChar);
#else
		sprintf(buffer,"%u", 1);
#endif
		ejSetResult(eid, buffer);
		return 0;
	}
   	if ( !strcmp(name, T("boot-mode")) ) {
 		if ( !mib_get( MIB_BOOT_MODE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	if ( !strcmp(name, T("dhcp-mode")) ) {
 		if ( !mib_get( MIB_DHCP_MODE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}	
   	if ( !strcmp(name, T("adsl-line-mode")) ) {
 		if ( !mib_get( MIB_ADSL_MODE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
#ifdef CONFIG_USER_ROUTED_ROUTED
   	if ( !strcmp(name, T("rip-on")) ) {
 		if ( !mib_get( MIB_RIP_ENABLE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
   	if ( !strcmp(name, T("rip-ver")) ) {
 		if ( !mib_get( MIB_RIP_VERSION, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
#endif
// Commented by Mason Yu for dhcpmode
#if 0	
   	if ( !strcmp(name, T("lan-dhcp")) ) {
 		if ( !mib_get( MIB_ADSL_LAN_DHCP, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
#endif	
 	else if ( !strcmp(name, T("br-stp")) ) {
   		if ( !mib_get( MIB_BRCTL_STP, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
#ifdef CONFIG_EXT_SWITCH
 	else if ( !strcmp(name, T("mp-mode")) ) {
   		if ( !mib_get( MIB_MPMODE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
#endif
#ifdef CONFIG_USER_IGMPPROXY
 	else if ( !strcmp(name, T("igmp-proxy")) ) {
   		if ( !mib_get( MIB_IGMP_PROXY, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
#endif
	
	else if ( !strcmp(name, T("acl-cap")) ) {
   		if ( !mib_get( MIB_ACL_CAPABILITY, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	
   	else if ( !strcmp(name, T("wan-dns")) ) {
 		if ( !mib_get( MIB_ADSL_WAN_DNS_MODE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("portFwEnabled"))) {
		if ( !mib_get( MIB_PORT_FW_ENABLE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("defaultFilterAction"))) {
		if ( !mib_get( MIB_IPF_OUT_ACTION, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("dmzEnabled"))) {
		if ( !mib_get( MIB_DMZ_ENABLE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("vc-auto"))) {
		if ( !mib_get( MIB_ATM_VC_AUTOSEARCH, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
   	else if ( !strcmp(name, T("ippt-itf"))) {
		if( !mib_get( MIB_IPPT_ITF,  (void *)&vChar))
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
   	else if ( !strcmp(name, T("ippt-lanacc"))) {
		if( !mib_get( MIB_IPPT_LANACC,  (void *)&vChar))
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("rconf-status"))) {
		sprintf(buffer, "%d", g_remoteConfig);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("rconf-port"))) {
		sprintf(buffer, "%d", g_remoteAccessPort);
		ejSetResult(eid, buffer);
		return 0;
	}
   	else if ( !strcmp(name, T("spc-enable"))) {
		if( !mib_get( MIB_SPC_ENABLE,  (void *)&vChar))
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
   	else if ( !strcmp(name, T("spc-iptype"))) {
		if( !mib_get( MIB_SPC_IPTYPE,  (void *)&vChar))
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("ipPortFilterNum"))) {
		vUInt = mib_chain_total(MIB_IP_PORT_FILTER_TBL);
		sprintf(buffer, "%u", vUInt);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("macFilterNum"))) {
		vUInt = mib_chain_total(MIB_MAC_FILTER_TBL);
		sprintf(buffer, "%u", vUInt);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("portFwNum"))) {
		vUInt = mib_chain_total(MIB_PORT_FW_TBL);
		sprintf(buffer, "%u", vUInt);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("atmVcNum"))) {
		vUInt = mib_chain_total(MIB_ATM_VC_TBL);
		sprintf(buffer, "%u", vUInt);
		ejSetResult(eid, buffer);
		return 0;
	}
   	else if ( !strcmp(name, T("wan-pppoeConnectStatus")) ) {
////	check the pppoe status
		sprintf(buffer, "%d", 0);
		ejSetResult(eid, buffer);
		return 0;
	}
#ifdef WLAN_SUPPORT
	else if ( !strcmp(name, T("channel")) ) {
		if ( !mib_get( MIB_WLAN_CHAN_NUM, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%d", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("regDomain")) ) {
		if ( !mib_get( MIB_HW_REG_DOMAIN, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("wep")) ) {
		if ( !mib_get( MIB_WLAN_WEP, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
   	    	return 0;
	}
	else if ( !strcmp(name, T("defaultKeyId")) ) {
		if ( !mib_get( MIB_WLAN_WEP_DEFAULT_KEY, (void *)&vChar) )
			return -1;
		vChar++;
		sprintf(buffer, "%u", vChar) ;
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("keyType")) ) {
		if ( !mib_get( MIB_WLAN_WEP_KEY_TYPE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar) ;
		ejSetResult(eid, buffer);
		return 0;
	}
  	else if ( !strcmp(name, T("authType"))) {
		if ( !mib_get( MIB_WLAN_AUTH_TYPE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar) ;
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("operRate"))) {
		if ( !mib_get( MIB_WLAN_SUPPORTED_RATE, (void *)&vUShort) )
			return -1;
		sprintf(buffer, "%u", vUShort);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("basicRate"))) {
		if ( !mib_get( MIB_WLAN_BASIC_RATE, (void *)&vUShort) )
			return -1;
		sprintf(buffer, "%u", vUShort);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("preamble"))) {
		if ( !mib_get( MIB_WLAN_PREAMBLE_TYPE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("hiddenSSID"))) {
		if ( !mib_get( MIB_WLAN_HIDDEN_SSID, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("wmFilterNum"))) {
		if ( !mib_get( MIB_WLAN_AC_NUM, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("wlanDisabled"))) {
		if ( !mib_get( MIB_WLAN_DISABLED, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("wlanAcNum")) ) {
		if ( !mib_get( MIB_WLAN_AC_NUM, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("wlanAcEnabled"))) {
		if ( !mib_get( MIB_WLAN_AC_ENABLED, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("rateAdaptiveEnabled"))) {
		if ( !mib_get( MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("wlanMode"))) {
		if ( !mib_get( MIB_WLAN_MODE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("networkType"))) {
		if ( !mib_get( MIB_WLAN_NETWORK_TYPE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("iappDisabled"))) {
		if ( !mib_get( MIB_WLAN_IAPP_DISABLED, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
#ifdef WLAN_WPA
	else if ( !strcmp(name, T("encrypt"))) {
		if ( !mib_get( MIB_WLAN_ENCRYPT, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("enable1X"))) {
		if ( !mib_get( MIB_WLAN_ENABLE_1X, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("enableSuppNonWpa"))) {
		if ( !mib_get( MIB_WLAN_ENABLE_SUPP_NONWPA, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("suppNonWpa"))) {
		if ( !mib_get( MIB_WLAN_SUPP_NONWPA, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("wpaAuth"))) {
		if ( !mib_get( MIB_WLAN_WPA_AUTH, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("wpaCipher"))) {
		if ( !mib_get( MIB_WLAN_WPA_CIPHER_SUITE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("pskFormat"))) {
		if ( !mib_get( MIB_WLAN_WPA_PSK_FORMAT, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("enableMacAuth"))) {
		if ( !mib_get( MIB_WLAN_ENABLE_MAC_AUTH, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("rsRetry")) ) {
		if ( !mib_get( MIB_WLAN_RS_RETRY, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
#endif

#ifdef WLAN_WDS
	else if ( !strcmp(name, T("wlanWdsEnabled"))) {
		if ( !mib_get( MIB_WLAN_WDS_ENABLED, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("wlanWdsNum"))) {
		if ( !mib_get( MIB_WLAN_WDS_NUM, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("wdsWep"))) {
		if ( !mib_get( MIB_WLAN_WDS_WEP, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("wdsDefaultKeyId"))) {
		if ( !mib_get( MIB_WLAN_WDS_WEP_DEFAULT_KEY, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", ++vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("wdsKeyType")) ) {
		if ( !mib_get( MIB_WLAN_WDS_WEP_KEY_TYPE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar) ;
		 ejSetResult(eid, buffer);
		return 0;
	}
#endif

#ifdef WLAN_8185AG
	else if ( !strcmp(name, T("RFType")) ) {
		if ( !mib_get( MIB_HW_RF_TYPE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar) ;
		 ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("band")) ) {
		if ( !mib_get( MIB_WLAN_BAND, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", (int)vChar) ;
		 ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, T("fixTxRate")) ) {
		if ( !mib_get( MIB_WLAN_FIX_RATE, (void *)&vUShort) )
			return -1;
		sprintf(buffer, "%u", vUShort) ;
		 ejSetResult(eid, buffer);
		return 0;
	}	
#endif

#endif // of WLAN_SUPPORT

	sprintf(buffer, "%d", -1);
	ejSetResult(eid, buffer);
	return 0;

//   	return -1;
}
#endif

int isConnectPPP(void)
{
	return 0;
}

#ifdef CONFIG_EXT_SWITCH
int show_lanport(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t *name;
	int vport;
	
	if (ejArgs(argc, argv, T("%s"), &name) < 1) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}

	vport = name[0]-'0';
	websWrite(wp,"LAN%d;\n", virt2user[vport]);
	return 0;
}
#endif


int getNameServer(int eid, webs_t wp, int argc, char_t **argv) {

	FILE *fp;
	char buffer[128], tmpbuf[64];
	int count = 0;
	//fprintf(stderr, "getNameServer %x\n", gResolvFile);
	//websWrite(wp, "[]", tmpbuf);
	//if ((gResolvFile == NULL) || 
	if ( (fp = fopen("/var/resolv.conf", "r")) == NULL ) {
		//printf("Unable to open resolver file\n");
		return -1;
	}

	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		if (sscanf(buffer, "nameserver %s", tmpbuf) != 1) {
			continue;
		}

		if (count == 0)
			websWrite(wp, "%s", tmpbuf);
		else
			websWrite(wp, ", %s", tmpbuf);
		count ++;
	}
	
	fclose(fp);
	return 0;
}

#ifndef RTF_UP
/* Keep this in sync with /usr/src/linux/include/linux/route.h */
#define RTF_UP          0x0001	/* route usable                 */
#define RTF_GATEWAY     0x0002	/* destination is a gateway     */
#define RTF_HOST        0x0004	/* host entry (net otherwise)   */
#define RTF_REINSTATE   0x0008	/* reinstate route after tmout  */
#define RTF_DYNAMIC     0x0010	/* created dyn. (by redirect)   */
#define RTF_MODIFIED    0x0020	/* modified dyn. (by redirect)  */
#define RTF_MTU         0x0040	/* specific MTU for this route  */
#ifndef RTF_MSS
#define RTF_MSS         RTF_MTU	/* Compatibility :-(            */
#endif
#define RTF_WINDOW      0x0080	/* per route window clamping    */
#define RTF_IRTT        0x0100	/* Initial round trip time      */
#define RTF_REJECT      0x0200	/* Reject route                 */
#endif

// Jenny, get default gateway information
int getDefaultGW(int eid, webs_t wp, int argc, char_t **argv)
{
	char buff[256];
	int flgs;
	//unsigned long int g;
	struct in_addr gw, dest, mask, inAddr;
	char ifname[16], dgw[16];
	FILE *fp;
	
	if (!(fp=fopen("/proc/net/route", "r"))) {
		printf("Error: cannot open /proc/net/route - continuing...\n");
		return -1;
	}

	fgets(buff, sizeof(buff), fp);
	while (fgets(buff, sizeof(buff), fp) != NULL) {
		//if (sscanf(buff, "%*s%*lx%lx%X%", &g, &flgs) != 2) {
		if (sscanf(buff, "%s%x%x%x%*d%*d%*d%x", ifname, &dest, &gw, &flgs, &mask) != 5) {
			printf("Unsuported kernel route format\n");
			fclose(fp);
			return -1;
		}
		
		//printf("ifname=%s, dest=%x, gw=%x, flgs=%x, mask=%x\n", ifname, dest.s_addr, gw.s_addr, flgs, mask.s_addr);
		if(flgs & RTF_UP) {
			// default gateway
			if (getInAddr(ifname, IP_ADDR, (void *)&inAddr) == 1) {
				if (inAddr.s_addr == 0x40404040) {
					websWrite(wp, "");
					fclose(fp);
					return 0;
				}
			}
			if (dest.s_addr == 0 && mask.s_addr == 0) {
				if (gw.s_addr != 0) {
					strncpy(dgw,  inet_ntoa(gw), 16);
					websWrite(wp, "%s", dgw);
					fclose(fp);
					return 0;
				}
				else {
					#ifdef ZTE_GENERAL_ROUTER_SC
					#ifdef CTC_WAN_NAME
					MIB_CE_ATM_VC_T tempPvcEntry;
					unsigned int pvcEntryNums=mib_chain_total(MIB_ATM_VC_TBL);
					int pvcEntryIndex=0;
					for(pvcEntryIndex=0;pvcEntryIndex<pvcEntryNums;pvcEntryIndex++)
						{
						if(!mib_chain_get(MIB_ATM_VC_TBL,pvcEntryIndex,&tempPvcEntry))
							{
							printf("mib get MIB_ATM_VC_TBL error!\n");
							return 0;
							}
						char interfaceName[64]={0};
						if(tempPvcEntry.cmode==ADSL_PPPoE||tempPvcEntry.cmode==ADSL_PPPoA)
						snprintf(interfaceName,64,"ppp%d",PPP_INDEX(tempPvcEntry.ifIndex));
						else
						snprintf(interfaceName,64,"vc%d",VC_INDEX(tempPvcEntry.ifIndex));
						if(!strcmp(ifname,interfaceName))
							{
								websWrite(wp, "Internet_R_%d_%d", tempPvcEntry.vpi,tempPvcEntry.vci);
								fclose(fp);
								return 0;
							}
						   
						}
					
					#endif
					#else
					websWrite(wp, "%s", ifname);
					#endif
					fclose(fp);
					return 0;
				}
			}
					
		}
		/*
		if(flgs & RTF_UP) {
			gw.s_addr = g;
			if (gw.s_addr != 0) {
				strcpy(dgw,  inet_ntoa(gw));
				websWrite(wp, "%s", dgw);
				fclose(fp);
				return 0;
			}
		}
		*/
	}
	fclose(fp);
	return 0;
}
