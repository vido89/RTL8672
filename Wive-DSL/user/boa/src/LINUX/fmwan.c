/*
 *      Web server handler routines for ATM
 *      Authors: Dick Tam	<dicktam@realtek.com.tw>
 *
 */

/*-- System inlcude files --*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/wait.h>
#include <semaphore.h>
#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../../uClibc/include/linux/autoconf.h"
#endif

/* for ioctl */
#include <sys/ioctl.h>
#include <net/if.h>

#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#include "debug.h"
#include "../defs.h"

//#define MAX_POE_PER_VC		2	// Jenny, move definition to utiltiy.h
#if defined (ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
static const char IF_UP[] = "开启";
static const char IF_DOWN[] = "关闭";
static const char IF_NA[] = "n/a";
static const char IF_DISABLED[] = "禁止";
static const char IF_ENABLE[]="允许";
static const char IF_ON[] = "开";
static const char IF_OFF[] = "关";
#else
static const char IF_UP[] = "up";
static const char IF_DOWN[] = "down";
static const char IF_NA[] = "n/a";
static const char IF_DISABLED[] = "Disabled";
static const char IF_ENABLE[]="Enable";
static const char IF_ON[] = "On";
static const char IF_OFF[] = "Off";
#endif

typedef enum {
	CONN_DISABLED=0,
	CONN_NOT_EXIST,
	CONN_DOWN,
	CONN_UP
} CONN_T;

//  add for auto-pvc-search 
#define MAX_PVC_SEARCH_PAIRS 16

#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
#define REBOOT_MSG(url) { \
	websHeader(wp); \
	websWrite(wp, T("<body><blockquote><h5>如果您改变了配置，要使配置生效请重新启动系统!。" \
                "<BR><form><input type=button value=\"  确定  \" OnClick=window.location.replace(\"%s\")></form></blockquote></body>"), url);\
	websFooter(wp); \
	websDone(wp, 200); \
}
#endif
// for auto-pvc-search 
//int update_auto_pvc_disable(unsigned char enabled)
void update_auto_pvc_disable(unsigned int enabled)
{
	MIB_T tmp_buffer;
	unsigned char vChar=0;
	if(enabled)	vChar = 1;
	mib_set( MIB_ATM_VC_AUTOSEARCH, (void *)&vChar);
}


int autopvc_is_up(void)
{
  	MIB_T tmp_buffer;
 	unsigned char vChar;
  	int value=0;

	mib_get( MIB_ATM_VC_AUTOSEARCH, (void *)&vChar);
//	    printf("autopvc_is_up: VC_AUTOSEARCH %x\n", vChar);
	if (vChar==1) 
		value=1;
	return value;
}

// action : 0=> delete PVC in auto-pvc-search tbl
//		  1=> add a pair of PVC in 
int add_auto_pvc_search_pair(webs_t wp, int action)
{
	char_t *strValue;
	//MIB_AUTO_PVC_SEARCH_Tp entryP;
	MIB_AUTO_PVC_SEARCH_T Entry;
	MIB_AUTO_PVC_SEARCH_T entry;	

	unsigned int VPI,VCI,entryNum,i;
	char tmpBuf[100];

	entryNum = mib_chain_total(MIB_AUTO_PVC_SEARCH_TBL);
	if(entryNum > MAX_PVC_SEARCH_PAIRS ) {
		strcpy(tmpBuf, T("Auto-PVC search table was full!"));
		goto setErr_filter;
	}		
	
	memset(&entry, 0x00, sizeof(entry));
	/*
	strValue = websGetVar(wp, T("autopvcvpi"), T(""));
	if(!strValue[0]) {
		printf("add_auto_pvc_search: strValue(\"autopvcvpi\") = %s\n", strValue);
	}
	*/
	strValue = websGetVar(wp, T("addVPI"), T(""));
	if (!strValue[0]) {
		strcpy(tmpBuf, T("Enter VPI (0~255)!"));
		goto setErr_filter;
	}
	//printf("add_auto_pvc_search: strValue(VPI) = %s\n", strValue);
	sscanf(strValue,"%u",&VPI);
	if(VPI > 255 ) {
		strcpy(tmpBuf, T("Invalid VPI!"));
		goto setErr_filter;
	}

	strValue = websGetVar(wp, T("addVCI"), T(""));

	if (!strValue[0]) {
		strcpy(tmpBuf, T("Enter VCI (0~65535)!"));
		goto setErr_filter;
	}
	//printf("add_auto_pvc_search: strValue(VCI) = %s\n", strValue);	
	sscanf(strValue,"%u",&VCI);
	if ( VCI>65535) {
		strcpy(tmpBuf, T("Invalid VCI!"));
		goto setErr_filter;
	}

	if(VCI ==0 && VPI == 0) {
		strcpy(tmpBuf, T("Invalid VPI and VCI (0,0)!"));
		goto setErr_filter;
	}
	
	for(i=0; i< entryNum; i++)
	{
		if (!mib_chain_get(MIB_AUTO_PVC_SEARCH_TBL, i, (void *)&Entry))
			continue;
		if(Entry.vpi == VPI && Entry.vci == VCI) {
			if(action)
				return 0;
			else
				break;
		}				
	}
		
	entry.vpi = VPI;	entry.vci = VCI;
	
	if(action) {
		printf("add_auto_pvc_search : adding (%d %d) PVC into search table\n", VPI,VCI);
		int intVal = mib_chain_add(MIB_AUTO_PVC_SEARCH_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			strcpy(tmpBuf, T(strAddChainerror));
			goto setErr_filter;
		}	
		else if (intVal == -1) {
			strcpy(tmpBuf, T(strTableFull));
			goto setErr_filter;
		}	
	} else {
		printf("add_auto_pvc_search : deleting (%d %d) PVC from search table\n", VPI,VCI);
		if(mib_chain_delete(MIB_AUTO_PVC_SEARCH_TBL, i) != 1){
			strcpy(tmpBuf, T("Error! Delete chain record."));
			goto setErr_filter;
		}	
	}

//	mib_update(CURRENT_SETTING);
	return 0;
	
setErr_filter:
	ERR_MSG(tmpBuf);
	return -1;

}

// retrieve VC record from web form
// reture value:
// 0  : success
// -1 : failed
static int retrieveVcRecord(webs_t wp, MIB_CE_ATM_VC_Tp pEntry)
{
	char tmpBuf[100];
	char_t *strValue, *strIp, *strMask, *strGW;
	unsigned int vUInt;
	MIB_CE_ATM_VC_T entry;
	
	// VPI (0~255)
	strValue = websGetVar(wp, T("vpi"), T(""));
	if (!strValue[0]) {
		strcpy(tmpBuf, T("Enter VPI (0~255)!"));
		goto setErr_filter;
	}
	
	memset(&entry, 0x00, sizeof(entry));
	sscanf(strValue,"%u",&vUInt);
	if ( vUInt>255) {
		strcpy(tmpBuf, T("Invalid VPI!"));
		goto setErr_filter;
	}
	entry.vpi = (unsigned char)vUInt;

	// VCI (0~65535)
	strValue = websGetVar(wp, T("vci"), T(""));

	if (!strValue[0]) {
		strcpy(tmpBuf, T("Enter VCI (0~65535)!"));
		goto setErr_filter;
	}
	
	sscanf(strValue,"%u",&vUInt);
	if ( vUInt>65535) {
		strcpy(tmpBuf, T("Invalid VCI!"));
		goto setErr_filter;
	}
	entry.vci = (unsigned short)vUInt;
	
	// set default Qos
	entry.qos = 0;
	entry.pcr = ATM_MAX_US_PCR;
	
	// Encaptulation
	strValue = websGetVar(wp, T("adslEncap"), T(""));
	if (strValue[0]) {
		entry.encap = strValue[0] - '0';
	}
	
	// Enable NAPT
	strValue = websGetVar(wp, T("naptEnabled"), T(""));
	if ( !gstrcmp(strValue, T("ON")))
		entry.napt = 1;
	else
		entry.napt = 0;
	
	// Enabled
	strValue = websGetVar(wp, T("chEnable"), T(""));
	if (strValue[0]) {
		entry.enable = strValue[0] - '0';
	}
	
	// Connection mode
	strValue = websGetVar(wp, T("adslConnectionMode"), T(""));
	if (strValue[0]) {
		entry.cmode = strValue[0] - '0';
	}
#ifdef CTC_WAN_NAME
	//application mode
	strValue = websGetVar(wp, T("adslApplicationMode"), T(""));
	if (strValue[0]) {
		char vpistr[6];
		char vcistr[6];
		entry.applicationtype = strValue[0] - '0';
		/*
		memset(vpistr, 0, sizeof(vpistr));
		memset(vcistr, 0, sizeof(vcistr));		
		memset(entry.applicationname, 0, sizeof(entry.applicationname));
		switch(entry.applicationtype)
		{
			//Internet
			case 0:
				strcpy(entry.applicationname, "Internet_");
				break;
			//TR069_Internet
			case 1:
				strcpy(entry.applicationname, "TR069_Internet_");
				break;

			//TR069
			case 2:
				strcpy(entry.applicationname, "TR069_");
				break;
			//Others
			case 3:
				strcpy(entry.applicationname, "Other_");
				break;
			default:
				strcpy(entry.applicationname, "Internet_");
				break;
		}
		
		if(entry.cmode == ADSL_BR1483)
			strcat(entry.applicationname, "B_");
		else
			strcat(entry.applicationname, "R_");
		sprintf(vpistr, "%d", entry.vpi);
		sprintf(vcistr, "%d", entry.vci);
		strcat(entry.applicationname, vpistr);
		strcat(entry.applicationname, "_");
		strcat(entry.applicationname, vcistr);*/
	}	
#endif
#ifdef DEFAULT_GATEWAY_V1
	// Default Route
	strValue = websGetVar(wp, T("droute"), T(""));
	if (strValue[0]) {
		entry.dgw = strValue[0] - '0';
	}
#else
{
	char_t *strIp, *strIf;
	unsigned char vChar, strIfc;
	unsigned char dgw;
	struct in_addr inGatewayIp;

	strValue = websGetVar(wp, T("droute"), T(""));
	if (strValue[0])
		vChar = strValue[0] - '0';

	if (vChar == 0) {
		strValue = websGetVar(wp, T("gwStr"), T(""));
		if (strValue[0]) {
			strIfc = strValue[0] - '0';
			if (strIfc == 1) {
				char ifname[16];
				strIf = websGetVar(wp, T("wanIf"), T(""));
				dgw = (unsigned char)atoi(strIf);
			}
			else if (strIfc == 0) {
				dgw = DGW_NONE;
				strIp = websGetVar(wp, T("dstGtwy"), T(""));
				if (strIp[0]) {
					if (!inet_aton(strIp, &inGatewayIp)) {
						strcpy(tmpBuf, T(strInvalidGatewayerror));
						goto setErr_filter;
					}
					if (entry.cmode == ADSL_MER1483 || (entry.cmode == ADSL_RT1483 && entry.ipunnumbered == 0)) {
						if (!inet_aton(strIp, (struct in_addr *)&entry.remoteIpAddr)) {
							strcpy(tmpBuf, T(strIPAddresserror));
							goto setErr_filter;
						}
					}
					if (!mib_set(MIB_ADSL_WAN_DGW_IP, (void *)&inGatewayIp)) {
						strcpy(tmpBuf, T(strSetGatewayerror));
						goto setErr_filter;
					}
				}
			}
		}
	}
	else if (vChar == 1)
		dgw = DGW_AUTO;

	if (!mib_set(MIB_ADSL_WAN_DGW_ITF, (void *)&dgw)) {
		strcpy(tmpBuf, T(strSetGatewayerror));
		goto setErr_filter;
	}
}
#endif
#ifdef ZTE_GENERAL_ROUTER_SC
strValue = websGetVar(wp, T("ipUnnum"), T(""));
		if (!gstrcmp(strValue, T("ON")))
			entry.ipunnumbered = 1;
		else
			entry.ipunnumbered = 0;
#endif	
#ifdef PPPOE_PASSTHROUGH
	entry.brmode = BRIDGE_DISABLE;
	
	// 1483 bridged
	if (entry.cmode == ADSL_BR1483)
	{
		entry.brmode = 0;
	}
	else // PPP connection
#endif
	if (entry.cmode == ADSL_PPPoE || entry.cmode == ADSL_PPPoA)
	{
#ifdef CONFIG_GUI_WEB
		// PPP user name
		strValue = websGetVar(wp, T("pppUserName"), T(""));
		if ( strValue[0] ) {
			if ( strlen(strValue) > P_MAX_NAME_LEN-1 ) {
				strcpy(tmpBuf, T(strUserNametoolong));
				goto setErr_filter;
			}
			strncpy(entry.pppUsername, strValue, P_MAX_NAME_LEN-1);
			entry.pppUsername[P_MAX_NAME_LEN-1]='\0';
			//entry.pppUsername[P_MAX_NAME_LEN]='\0';
		}

		// PPP password
		strValue = websGetVar(wp, T("pppPassword"), T(""));
		
		if ( strValue[0] ) {
			if ( strlen(strValue) > P_MAX_NAME_LEN-1 ) {
				strcpy(tmpBuf, T(strPasstoolong));
				goto setErr_filter;
			}
			strncpy(entry.pppPassword, strValue, P_MAX_NAME_LEN-1);
			entry.pppPassword[P_MAX_NAME_LEN-1]='\0';
			//entry.pppPassword[P_MAX_NAME_LEN]='\0';
		}
#else
		// PPP user name
		strValue = websGetVar(wp, T("pppUserName"), T(""));
		if ( strValue[0] ) {
			if ( strlen(strValue) > MAX_PPP_NAME_LEN ) {
				strcpy(tmpBuf, T(strUserNametoolong));
				goto setErr_filter;
			}
			strncpy(entry.pppUsername, strValue, MAX_PPP_NAME_LEN);
			entry.pppUsername[MAX_PPP_NAME_LEN]='\0';
		}

		// PPP password
		strValue = websGetVar(wp, T("pppPassword"), T(""));
		
		if ( strValue[0] ) {
			if ( strlen(strValue) > MAX_NAME_LEN-1 ) {
				strcpy(tmpBuf, T(strPasstoolong));
				goto setErr_filter;
			}
			strncpy(entry.pppPassword, strValue, MAX_NAME_LEN-1);
			entry.pppPassword[MAX_NAME_LEN-1]='\0';
			//entry.pppPassword[MAX_NAME_LEN]='\0';
		}
#endif

		// PPP connection type
		strValue = websGetVar(wp, T("pppConnectType"), T(""));
		
		if ( strValue[0] ) {
			PPP_CONNECT_TYPE_T type;
			
			if ( strValue[0] == '0' )
				type = CONTINUOUS;
			else if ( strValue[0] == '1' )
				type = CONNECT_ON_DEMAND;
			else if ( strValue[0] == '2' )
				type = MANUAL;
			else {
				strcpy(tmpBuf, T(strInvalPPPType));
				goto setErr_filter;
			}
			
			entry.pppCtype = (unsigned char)type;
			
			if (type != CONTINUOUS) {
				// PPP idle time
				strValue = websGetVar(wp, T("pppIdleTime"), T(""));
				if ( strValue[0] ) {
					unsigned short time;
					time = (unsigned short) strtol(strValue, (char**)NULL, 10);
					entry.pppIdleTime = time;
				}
			}
		}
	}
	else // Wan IP setting
	{
#ifndef ZTE_GENERAL_ROUTER_SC
		// Jenny, IP unnumbered

		strValue = websGetVar(wp, T("ipUnnum"), T(""));
		if (!gstrcmp(strValue, T("ON")))
			entry.ipunnumbered = 1;
		else
			entry.ipunnumbered = 0;
#endif
		// IP mode
		strValue = websGetVar(wp, T("ipMode"), T(""));
		if (strValue[0]) {

			if(strValue[0] == '0')
				entry.ipDhcp = (char) DHCP_DISABLED;
			else if(strValue[0] == '1')
				entry.ipDhcp = (char) DHCP_CLIENT;
			else {
				strcpy(tmpBuf, T(strInvalDHCP));
				goto setErr_filter;
			}
		}

		// Local IP address
		strIp = websGetVar(wp, T("ip"), T(""));
		if (strIp[0]) {
			if (!inet_aton(strIp, (struct in_addr *)&entry.ipAddr)) {
				strcpy(tmpBuf, T(strInvalIP));
				goto setErr_filter;
			}
		}
		
		// Remote IP address
		strGW = websGetVar(wp, T("remoteIp"), T(""));
		if (strGW[0]) {
			if (!inet_aton(strGW, (struct in_addr *)&entry.remoteIpAddr)) {
				strcpy(tmpBuf, T(strInvalGateway));
				goto setErr_filter;
			}
		}
		
		// Subnet Mask, added by Jenny
		strMask = websGetVar(wp, T("netmask"), T(""));
		if (strMask[0]) {
			if (!isValidNetmask(strMask, 1)) {
				strcpy(tmpBuf, T(strInvalMask));
				goto setErr_filter;
			}
			if (!inet_aton(strMask, (struct in_addr *)&entry.netMask)) {
				strcpy(tmpBuf, T(strInvalMask));
				goto setErr_filter;
			}
			if (!isValidHostID(strIp, strMask)) {
				strcpy(tmpBuf, T("Invalid IP/Subnet Mask combination!!"));
				goto setErr_filter;
			}
		}

#ifdef DEFAULT_GATEWAY_V1
		//if (!isSameSubnet(strIp, strGW, strMask)) {
		if (entry.cmode == ADSL_MER1483 && !isSameSubnet(strIp, strGW, strMask)) {
			strcpy(tmpBuf, T("Invalid IP address! It should be located in the same subnet."));
			goto setErr_filter;
		}
#endif
	}
	#ifdef CONFIG_USER_PPPOE_PROXY
		if(entry.cmode == ADSL_PPPoE)
			{
			 strValue=websGetVar(wp,T("pppEnable"),T(""));
			 if(strValue[0])
			 	{
			 	   entry.PPPoEProxyEnable=strValue[0]-'0';
			 	}
			}
		else entry.PPPoEProxyEnable=0;
			
	#endif
	
	memcpy(pEntry, &entry, sizeof(entry));
	return 0;
	
setErr_filter:
	ERR_MSG(tmpBuf);
	return -1;
}

static void resolveServiceDependency(idx)
{
	//MIB_CE_ATM_VC_Tp pEntry;
	MIB_CE_ATM_VC_T Entry;
#ifdef CONFIG_USER_IGMPPROXY
	unsigned char igmp_proxy;
#endif
#ifdef IP_PASSTHROUGH
	unsigned char ippt_itf;
	unsigned int ippt_lease;
#endif
	struct data_to_pass_st msg;
	int k;
	
	/* get the specified chain record */
	if (!mib_chain_get(MIB_ATM_VC_TBL, idx, (void *)&Entry))
	{
		return;
	}
#if 0	
#ifdef APPLY_CHANGE
	stopConnection(&Entry);
#endif
#endif
	
#ifdef CONFIG_USER_IGMPPROXY
	// resolve IGMP proxy dependency
	if(!mib_get( MIB_IGMP_PROXY_ITF,  (void *)&igmp_proxy))
		return;
	if (Entry.ifIndex == igmp_proxy)
	{ // This interface is IGMP proxy interface
		igmp_proxy = 0xff;	// set to default
		mib_set(MIB_IGMP_PROXY_ITF, (void *)&igmp_proxy);
		igmp_proxy = 0;	// disable IGMP proxy
		mib_set(MIB_IGMP_PROXY, (void *)&igmp_proxy);
	}
#endif
	
#ifdef IP_PASSTHROUGH
	// resolve IP passthrough dependency
	if(!mib_get( MIB_IPPT_ITF,  (void *)&ippt_itf))
		return;
	if (Entry.ifIndex == ippt_itf)
	{ // This interface is IP passthrough interface
		ippt_itf = 0xff;	// set to default
		mib_set(MIB_IPPT_ITF, (void *)&ippt_itf);
		ippt_lease = 600;	// default to 10 min.
		mib_set(MIB_IPPT_LEASE, (void *)&ippt_lease);
	}
#endif
	
#ifdef CONFIG_EXT_SWITCH
#ifdef ITF_GROUP
	// resolve port mapping
	for (k=0; k<SW_PORT_NUM; k++) {
		//MIB_CE_SW_PORT_Tp pSWEntry;
		MIB_CE_SW_PORT_T SWEntry;
		if (!mib_chain_get(MIB_SW_PORT_TBL, k, (void *)&SWEntry))
			return;
		if (SWEntry.pvcItf == Entry.ifIndex) {
			// This interface has been mapped by lan port
			// Detach it!
			SWEntry.pvcItf = 0xff; // set to default
			// log message
			mib_chain_update(MIB_SW_PORT_TBL, (void *)&SWEntry, k);
		}
	}
#endif	// of ITF_GROUP
#endif	// of CONFIG_EXT_SWITCH
}	

// map: bit map of used interface, ppp index (0~15) is mapped into high 16 bits,
// while vc index (0~15) is mapped into low 16 bits.
// return: interface index, high nibble for PPP index and low nibble for vc index.
//		0xef: PPP not available
//		0xff: vc not available
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

#ifdef CONFIG_EXT_SWITCH
#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
static void write1q(webs_t wp, MIB_CE_ATM_VC_Tp pEntry, char *formName)
{
	// vlan mapping
	websWrite(wp,
	T("<script>\nfunction click1q()\n{\n\tif (%s.%s.vlan[0].checked)"
	"\n\t\t%s.%s.vid.disabled=true;\n\telse\n\t\t%s.%s.vid.disabled=false;"
	"\n}\n</script>\n"),
	DOCUMENT, formName, DOCUMENT, formName, DOCUMENT, formName);
	websWrite(wp,
	T("<tr><th align=left><b>802.1q:</b></th>\n<td>\n"
	"<input type=radio value=0 name=\"vlan\" %s onClick=click1q()>禁止&nbsp;&nbsp;\n"
	"<input type=radio value=1 name=\"vlan\" %s onClick=click1q()>允许\n</td></tr>\n"),
	pEntry->vlan? "":"checked", pEntry->vlan? "checked":"");
	websWrite(wp,
	T("<tr><th align=left></th>\n<td>\n"
	"VLAN ID(0-4095):&nbsp;&nbsp;\n"
	"<input type=text name=vid size=6 maxlength=4 value=%d %s></td></tr>\n")
	, pEntry->vid, pEntry->vlan?"":"disabled");
	/*
	websWrite(wp,
	T("<input type=checkbox name=vpass value=1 %s>VLAN Passthrough\n"), pEntry->vpass==1? "checked":"");
	websWrite(wp,
	T("<tr><th align=left></th>\n<td>\n"
	"Priority bits:&nbsp;&nbsp;\n"
	"<select name=prio>\n"));
	websWrite(wp,
	T("<option value=0>0</option>\n"
	"<option value=1>1</option>\n"
	"<option value=2>2</option>\n"
	"<option value=3>3</option>\n"
	"<option value=4>4</option>\n"
	"<option value=5>5</option>\n"
	"<option value=6>6</option>\n"
	"<option value=7>7</option>\n"));
	websWrite(wp,
	T("<script> document.brif.prio.value=%d;\n</script>"), pEntry->vprio);
	*/
}
#else
static void write1q(webs_t wp, MIB_CE_ATM_VC_Tp pEntry, char *formName)
{
	// vlan mapping
	websWrite(wp,
	T("<script>\nfunction click1q()\n{\n\tif (%s.%s.vlan[0].checked)"
	"\n\t\t%s.%s.vid.disabled=true;\n\telse\n\t\t%s.%s.vid.disabled=false;"
	"\n}\n</script>\n"),
	DOCUMENT, formName, DOCUMENT, formName, DOCUMENT, formName);
	websWrite(wp,
	T("<script>\nfunction check1q(str)\n{\n\tfor (var i=0; i<str.length; i++) {"
	"\n\t\tif ((str.charAt(i) >= '0' && str.charAt(i) <= '9'))"
	"\n\t\t\tcontinue;"
	"\n\t\treturn false;\n\t}"
	"\n\td = parseInt(str, 10);"
	"\n\tif (d < 0 || d > 4095)"
	"\n\t\treturn false;\n\treturn true;\n}\n"));
	websWrite(wp,
	T("\nfunction apply1q()\n{\n\tif (!check1q(%s.%s.vid.value)) {"
	"\n\t\talert(\"Invalid VLAN ID!\");"
	"\n\t\t%s.%s.vid.focus();"
	"\n\t\treturn false;\n\t}\n\treturn true;\n}\n</script>\n"),
	DOCUMENT, formName, DOCUMENT, formName);
	websWrite(wp,
	T("<tr><th align=left><b>802.1q:</b></th>\n<td>\n"
	"<input type=radio value=0 name=\"vlan\" %s onClick=click1q()>Disable&nbsp;&nbsp;\n"
	"<input type=radio value=1 name=\"vlan\" %s onClick=click1q()>Enable\n</td></tr>\n"),
	pEntry->vlan? "":"checked", pEntry->vlan? "checked":"");
	websWrite(wp,
	T("<tr><th align=left></th>\n<td>\n"
	"VLAN ID(0-4095):&nbsp;&nbsp;\n"
	"<input type=text name=vid size=6 maxlength=4 value=%d %s></td></tr>\n")
	, pEntry->vid, pEntry->vlan?"":"disabled");
	/*
	websWrite(wp,
	T("<input type=checkbox name=vpass value=1 %s>VLAN Passthrough\n"), pEntry->vpass==1? "checked":"");
	websWrite(wp,
	T("<tr><th align=left></th>\n<td>\n"
	"Priority bits:&nbsp;&nbsp;\n"
	"<select name=prio>\n"));
	websWrite(wp,
	T("<option value=0>0</option>\n"
	"<option value=1>1</option>\n"
	"<option value=2>2</option>\n"
	"<option value=3>3</option>\n"
	"<option value=4>4</option>\n"
	"<option value=5>5</option>\n"
	"<option value=6>6</option>\n"
	"<option value=7>7</option>\n"));
	websWrite(wp,
	T("<script> document.brif.prio.value=%d;\n</script>"), pEntry->vprio);
	*/
}
#endif
#endif

// Jenny, sync PPPoE Passthrough flag for multisession PPPoE
static void syncPPPoEPassthrough(MIB_CE_ATM_VC_T entry)
{
	MIB_CE_ATM_VC_T Entry;
	unsigned int totalEntry;
	int i;

	totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<totalEntry; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			return;
		if (Entry.vpi == entry.vpi && Entry.vci == entry.vci) {
			Entry.brmode = entry.brmode;
			mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, i);
		}
	}
}

#ifdef DEFAULT_GATEWAY_V1
static int dr=0, pdgw=0;
#endif
// Jenny, PPPoE static IP option
#ifdef CONFIG_SPPPD_STATICIP
static void writeStaticIP(webs_t wp, MIB_CE_ATM_VC_Tp pEntry, char *formName)
{
	char ipAddr[20];

	// static PPPoE
	websWrite(wp,
		T("<script>\nfunction clickstatic()\n{\n\tif (%s.%s.pppip[0].checked)"
		"\n\t\t%s.%s.staticip.disabled=true;\n\telse\n\t\t%s.%s.staticip.disabled=false;"
		"\n}\n</script>\n"),
		DOCUMENT, formName, DOCUMENT, formName, DOCUMENT, formName);
	websWrite(wp,
		T("<tr><th align=left><b>%s:</b></th>\n<td>\n"
		"<input type=radio value=0 name=\"pppip\" %s onClick=clickstatic()>%s&nbsp;&nbsp;\n"
		"<input type=radio value=1 name=\"pppip\" %s onClick=clickstatic()>%s&nbsp;&nbsp;\n"),
		Tip_addr, pEntry->pppIp? "":"checked", Tdynamic_ip, pEntry->pppIp? "checked":"", Tstatic_ip);
	strcpy(ipAddr, inet_ntoa(*((struct in_addr *)pEntry->ipAddr)));
	websWrite(wp,
		T("<input type=text name=staticip size=10 maxlength=15 value=%s %s></td></tr>\n")
		, ipAddr, pEntry->pppIp?"":"disabled");
}
#endif

#ifdef ZTE_GENERAL_ROUTER_SC
void writePPPEdit(webs_t wp, MIB_CE_ATM_VC_Tp pEntry, int index)
{
		char *fmName="ppp";
		char *proto;
		
		if (pEntry->cmode == ADSL_PPPoE)
			proto = "PPPoE";
		else
		if (pEntry->cmode == ADSL_PPPoA)
			proto = "PPPoA";
		
		#ifdef DEFAULT_GATEWAY_V1
		pdgw=pEntry->dgw;
		#endif
		
		// Javascript
		websWrite(wp, T("<head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=gb2312\">\n<script>\nfunction pppTypeSelection()\n{\n"
		"if ( document.%s.pppConnectType.selectedIndex == 2) {\n"
		"document.%s.pppIdleTime.value = \"\";\n"
		"document.%s.pppIdleTime.disabled = true;\n}\nelse {\n"
		"if (document.%s.pppConnectType.selectedIndex == 1) {\n"
		"document.%s.pppIdleTime.value = %d;\n"
		"document.%s.pppIdleTime.disabled = false;\n}\nelse {\n"
		"document.%s.pppIdleTime.value = \"\";\n"
		"document.%s.pppIdleTime.disabled = true;\n}\n}\n}\n")
		,fmName, fmName, fmName, fmName, fmName, pEntry->pppIdleTime, fmName, fmName, fmName);

		websWrite(wp,T("function getDigit(str, num)\n{"
"  i=1;  if ( num != 1 ) {"
"  	while (i!=num && str.length!=0) {"
"		if ( str.charAt(0) == '.' ) {"
"			i++;"
"		}"
"		str = str.substring(1);"
  	"}  	if ( i!=num )"
"  		return -1;"
  "}"
  "for (i=0; i<str.length; i++) {"
"  	if ( str.charAt(i) == '.' ) {"
"		str = str.substring(0, i);"
"		break;"
"	}"
  "}"));
		websWrite(wp,T(" if ( str.length == 0)"
"  	return -1;"
"  d = parseInt(str, 10);"
"  return d;"
"}"
"function validateKey(str)"
"{   for (var i=0; i<str.length; i++) {"
"    if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||"
"    		(str.charAt(i) == '.' ) )"
"			continue;"
"	return 0;"
" }"
  "return 1;"
"}"));
websWrite(wp,T("function postcheck()"
"{if ( validateKey( document.ppp.maxusernums.value ) == 0 ) {"
"alert(\"无效的最大用户数目!\");"
"document.ppp.maxusernums.focus();"
"return false;"
"}"
"d1 = getDigit(document.ppp.maxusernums.value, 1);"
"if (d1 > 255 || d1 < 1) {"
"alert(\"无效的最大用户数目!\");"
"document.ppp.maxusernums.focus();"
"return false;}return true;}</script></head>"));

		// body
		websWrite(wp,
		T("<body><blockquote><h2><font color=\"#0000FF\">PPP接口-修改</font></h2>\n"
		"<form action=/goform/formPPPEdit method=POST name=\"%s\">\n"
		"<table border=0 width=600 cellspacing=4 cellpadding=0>"
		"<tr>\n<th align=left><b>PPP接口:</b></th>\n<td>ppp%d</td></tr>\n"
		"<tr>\n<th align=left><b>协议:</b></th>\n<td>%s</td></tr>\n"
		"<tr>\n<th align=left><b>ATM虚连接:</b></th>\n<td>%d/%d</td></tr>\n"),
		fmName, PPP_INDEX(pEntry->ifIndex), proto, pEntry->vpi, pEntry->vci);
		
		websWrite(wp,
		T("<tr><th align=left><b>状态:</b></th>\n<td>\n"
		"<input type=radio value=0 name=\"status\" %s>禁止&nbsp;&nbsp;\n"
		"<input type=radio value=1 name=\"status\" %s>允许\n</td></tr>\n"),
		pEntry->enable? "":"checked", pEntry->enable? "checked":"");
		
		websWrite(wp,
		T("<tr><th align=left><b>登录名:</b></th>\n<td>"
		"<input type=\"text\" name=\"name\" maxlength=30 value=%s></td></tr>\n"
		"<tr><th align=left><b>密码:</b></th>\n"
		"<td><input type=\"password\" name=\"passwd\" maxlength=30 value=%s></td></tr>\n"
		"<tr><th align=left><b>验证方法:</b></th>\n<td>"
		"<select size=1 name=\"auth\">\n"
		"<option %s value=0>自适应</option>\n"
		"<option %s value=1>PAP</option>\n"
		"<option %s value=2>CHAP</option></select></td></tr>\n"),
		pEntry->pppUsername, pEntry->pppPassword,
		pEntry->pppAuth==PPP_AUTH_AUTO?"selected":"",
		pEntry->pppAuth==PPP_AUTH_PAP?"selected":"",
		pEntry->pppAuth==PPP_AUTH_CHAP?"selected":"");
		
		websWrite(wp,
		T("<tr><th align=left><b>连接类型:</b></th>\n<td>"
		"<select size=1 name=\"pppConnectType\" onChange=\"pppTypeSelection()\">\n"
		"<option %s value=0>始终在线</option>\n"
		"<option %s value=1>按需连接</option>\n"
		"<option %s value=2>手动连接</option></select></td></tr>\n"
		"<tr><th align=left><b>空闲时间:</b></th>\n<td>"
		"<input type=\"text\" name=\"pppIdleTime\" maxlength=3 value=%d %s></td></tr>"),
		pEntry->pppCtype==CONTINUOUS?"selected":"",
		pEntry->pppCtype==CONNECT_ON_DEMAND?"selected":"",
		pEntry->pppCtype==MANUAL?"selected":"",
		pEntry->pppIdleTime,
		pEntry->pppCtype==CONNECT_ON_DEMAND?"":"disabled");
		
#ifdef _CWMP_MIB_
		websWrite(wp,
		T("<tr><th align=left><b>自动断线时间:</b></th>\n<td>"
		"<input type=\"text\" name=\"disconnectTime\" value=%d maxlength=10></td></tr>\n"),
		pEntry->autoDisTime);
		websWrite(wp,
		T("<tr><th align=left><b>连接中断警告延时:</b></th>\n<td>"
		"<input type=\"text\" name=\"disconnectDelay\" value=%d maxlength=10></td></tr>\n"),
		pEntry->warnDisDelay);
#endif

#ifdef DEFAULT_GATEWAY_V1
		websWrite(wp,
		T("<tr><th align=left><b>默认路由:</b></th>\n<td>"
		"<input type=radio value=0 name=\"droute\" %s>禁止&nbsp;&nbsp;\n"
		"<input type=radio value=1 name=\"droute\" %s>允许\n</td></tr>\n"),
		pEntry->dgw? "":"checked", pEntry->dgw? "checked":"");
#endif		
		if (pEntry->cmode == ADSL_PPPoE) {
			websWrite(wp,
			T("<tr><th align=left><b>MRU(65~1492):</b></th>\n<td>"
			"<input type=\"text\" name=\"mru\" value=%d maxlength=4></td></tr>\n"),
			pEntry->mtu);

		}else{
			websWrite(wp,
			T("<tr><th align=left><b>MRU(65~1500):</b></th>\n<td>"
			"<input type=\"text\" name=\"mru\" value=%d maxlength=4></td></tr>\n"),
			pEntry->mtu);
		}
		
		if (pEntry->cmode == ADSL_PPPoE) {
#ifdef CONFIG_SPPPD_STATICIP
			writeStaticIP(wp, pEntry, fmName);
#endif
#ifdef PPPOE_PASSTHROUGH
			#if 0
			websWrite(wp,
			T("<tr><th align=left><b>PPPoE pass-through:</b></th>\n<td>"
			"<input type=radio value=0 name=\"poe\" %s>Disable&nbsp;&nbsp;\n"
			"<input type=radio value=1 name=\"poe\" %s>Enable\n</td></tr>\n"),
			//pEntry->brmode? "":"checked", pEntry->brmode? "checked":"");
			(pEntry->brmode==BRIDGE_ETHERNET)? "checked":"", (pEntry->brmode==BRIDGE_PPPOE)? "checked":"");
			#endif
			websWrite(wp,
			T("<tr><th align=left><b>网桥:</b></th>\n<td>"
			"<input type=radio value=0 name=\"mode\" %s>以太网网桥"
			" (透明网桥)&nbsp;&nbsp;\n</td></tr>\n"
			"<tr><th></th>\n<td>\n"
			"<input type=radio value=1 name=\"mode\" %s>PPPoE网桥"
			" (只有PPPoE包可以通过)\n</td></tr>\n"
			"<tr><th></th>\n<td>\n"
			"<input type=radio value=2 name=\"mode\" %s>网桥禁止(禁止该通道上的网桥功能)"
			" \n</td></tr>\n"),
			pEntry->brmode? "":"checked", (pEntry->brmode==BRIDGE_PPPOE)? "checked":"", (pEntry->brmode==BRIDGE_DISABLE)? "checked":"");
#endif
			
			websWrite(wp,
			T("<tr><th align=left><b>AC名称:</b></th>\n<td>"
			"<input type=\"text\" name=\"acName\" maxlength=30 value=%s></td></tr>\n"),
			pEntry->pppACName);

#ifdef _CWMP_MIB_
			websWrite(wp,
			T("<tr><th align=left><b>服务名称:</b></th>\n<td>"
			"<input type=\"text\" name=\"serviceName\" maxlength=30 value=%s></td></tr>\n"),
			pEntry->pppServiceName);
#endif

#ifdef CONFIG_EXT_SWITCH
			write1q(wp, pEntry, fmName);
#endif
#ifdef CONFIG_USER_PPPOE_PROXY
                          websWrite(wp,
			T("<tr><th align=left><b>PPPoE代理:</b></th>\n<td>"
			"<input type=radio value=0 name=\"pproxy\" %s>禁止&nbsp;&nbsp;\n"
			"<input type=radio value=1 name=\"pproxy\" %s>允许\n</td></tr>\n"),
			//pEntry->brmode? "":"checked", pEntry->brmode? "checked":"");
			
			(!pEntry->PPPoEProxyEnable)? "checked":"", (pEntry->PPPoEProxyEnable)? "checked":"");

                          websWrite(wp,
			T("<tr><th align=left><b>最大用户数目:</b></th>\n<td>"
			"<input type=\"text\" name=\"maxusernums\" maxlength=2 value=%d></td></tr>\n"),
			pEntry->PPPoEProxyMaxUser);
#endif			  

		}
		
		websWrite(wp,
		T("</table>\n<br><input type=submit value=\"应用\" name=\"save\">\n"
		"<input type=submit value=\"返回\" name=\"return\">\n"
		"<input type=reset value=\"清空\" name=\"reset\">\n"
		"<input type=hidden value=%d name=\"item\">\n"
		"<input type=hidden value=\"/wanadsl_sc.asp\" name=\"submit-url\">\n"
		"</form></blockquote></body>"), index);
}
#else




void writePPPEdit(webs_t wp, MIB_CE_ATM_VC_Tp pEntry, int index)
{
		char *fmName="ppp";
		char *proto;
		
		if (pEntry->cmode == ADSL_PPPoE)
			proto = "PPPoE";
		else
		if (pEntry->cmode == ADSL_PPPoA)
			proto = "PPPoA";
		
#ifdef DEFAULT_GATEWAY_V1
		pdgw=pEntry->dgw;
#endif
		
		// Javascript
		websWrite(wp, T("<head>\n<script>\nfunction pppTypeSelection()\n{\n"
		"if ( document.%s.pppConnectType.selectedIndex == 2) {\n"
		"document.%s.pppIdleTime.value = \"\";\n"
		"document.%s.pppIdleTime.disabled = true;\n}\nelse {\n"
		"if (document.%s.pppConnectType.selectedIndex == 1) {\n"
		"document.%s.pppIdleTime.value = %d;\n"
		"document.%s.pppIdleTime.disabled = false;\n}\nelse {\n"
		"document.%s.pppIdleTime.value = \"\";\n"
		"document.%s.pppIdleTime.disabled = true;\n}\n}\n}\n")
		,fmName, fmName, fmName, fmName, fmName, pEntry->pppIdleTime, fmName, fmName, fmName);

		websWrite(wp,T("function getDigit(str, num)\n{"
"  i=1;  if ( num != 1 ) {"
"  	while (i!=num && str.length!=0) {"
"		if ( str.charAt(0) == '.' ) {"
"			i++;"
"		}"
"		str = str.substring(1);"
  	"}  	if ( i!=num )"
"  		return -1;"
  "}"
  "for (i=0; i<str.length; i++) {"
"  	if ( str.charAt(i) == '.' ) {"
"		str = str.substring(0, i);"
"		break;"
"	}"
  "}"));
		websWrite(wp,T(" if ( str.length == 0)"
"  	return -1;"
"  d = parseInt(str, 10);"
"  return d;"
"}"
"function validateKey(str)"
"{   for (var i=0; i<str.length; i++) {"
"    if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||"
"    		(str.charAt(i) == '.' ) )"
"			continue;"
"	return 0;"
" }"
  "return 1;"
"}"));
		websWrite(wp,
		T("\nfunction checkString(str)\n{\n\tfor (var i=0; i<str.length; i++) {"
		"\n\t\tif ((str.charAt(i) >= '0' && str.charAt(i) <= '9') || (str.charAt(i) == '-' ) || (str.charAt(i) == '_' ) || (str.charAt(i) == '@' ) || (str.charAt(i) == ':' ) ||"
		"\n\t\t\t(str.charAt(i) == '.' ) || (str.charAt(i) == '/' ) || (str.charAt(i) >= 'A' && str.charAt(i) <= 'Z') || (str.charAt(i) >= 'a' && str.charAt(i) <= 'z'))"
		"\n\t\t\tcontinue;"
		"\n\t\treturn 0;\n\t}"
		"\n\treturn 1;\n}\n"));
		websWrite(wp,
		T("\nfunction checkDigit(str)\n{\n\tfor (var i=0; i<str.length; i++) {"
		"\n\t\tif ((str.charAt(i) >= '0' && str.charAt(i) <= '9'))"
		"\n\t\t\tcontinue;"
		"\n\t\treturn false;\n\t}"
		"\n\treturn true;\n}\n"));
		websWrite(wp,
		T("\nfunction applyPPP()\n{"
		"if (document.%s.pppConnectType.selectedIndex == 1) {"
		"\n\t\tif ((document.ppp.pppIdleTime.value <= 0) || (!checkDigit(document.ppp.pppIdleTime.value))) {"
		"\n\t\t\talert(\"Invalid idle time!\");"
		"\n\t\t\tdocument.ppp.pppIdleTime.focus();"
		"\n\t\t\treturn false;\n\t\t}\n\t}"
		"\n\tif (!checkString(document.ppp.name.value)) {"
		"\n\t\talert(\"Invalid username!\");"
		"\n\t\tdocument.ppp.name.focus();"
		"\n\t\treturn false;\n\t}"
		"\n\tif (!checkString(document.ppp.passwd.value)) {"
		"\n\t\talert(\"Invalid password!\");"
		"\n\t\tdocument.ppp.passwd.focus();"
		"\n\t\treturn false;\n\t}"), fmName);
		if (pEntry->cmode == ADSL_PPPoE) {
			websWrite(wp,
			T("\n\tif (!checkString(document.ppp.acName.value)) {"
			"\n\t\talert(\"Invalid AC name!\");"
			"\n\t\tdocument.ppp.acName.focus();"
			"\n\t\treturn false;\n\t}"));
#ifdef CONFIG_EXT_SWITCH
			websWrite(wp,
			T("\n\tif (!check1q(document.ppp.vid.value)) {"
			"\n\t\talert(\"Invalid VLAN ID!\");"
			"\n\t\tdocument.ppp.vid.focus();"
			"\n\t\treturn false;\n\t}"));
#endif
#ifdef _CWMP_MIB_
			websWrite(wp,
			T("\n\tif (!checkString(document.ppp.serviceName.value)) {"
			"\n\t\talert(\"Invalid service name!\");"
			"\n\t\tdocument.ppp.serviceName.focus();"
			"\n\t\treturn false;\n\t}"));
#endif
		}
		websWrite(wp,T("\n\treturn true;\n}\n"));
websWrite(wp,T("function postcheck()"
"{if ( validateKey( document.ppp.maxusernums.value ) == 0 ) {"
"alert(\"Invalid max user number!\");"
"document.ppp.maxusernums.focus();"
"return false;"
"}"
"d1 = getDigit(document.ppp.maxusernums.value, 1);"
"if (d1 > 255 || d1 < 1) {"
"alert(\"Invalid max user number!\");"
"document.ppp.maxusernums.focus();"
"return false;}return true;}</script></head>"));

		// body
		websWrite(wp,
		T("<body><blockquote><h2><font color=\"#0000FF\">PPP Interface - Modify</font></h2>\n"
		"<form action=/goform/admin/formPPPEdit method=POST name=\"%s\">\n"
		"<table border=0 width=600 cellspacing=4 cellpadding=0>"
		"<tr>\n<th align=left><b>PPP Interface:</b></th>\n<td>ppp%d</td></tr>\n"
		"<tr>\n<th align=left><b>Protocol:</b></th>\n<td>%s</td></tr>\n"
		"<tr>\n<th align=left><b>ATM VCC:</b></th>\n<td>%d/%d</td></tr>\n"),
		fmName, PPP_INDEX(pEntry->ifIndex), proto, pEntry->vpi, pEntry->vci);
		
		websWrite(wp,
		T("<tr><th align=left><b>Status:</b></th>\n<td>\n"
		"<input type=radio value=0 name=\"status\" %s>Disable&nbsp;&nbsp;\n"
		"<input type=radio value=1 name=\"status\" %s>Enable\n</td></tr>\n"),
		pEntry->enable? "":"checked", pEntry->enable? "checked":"");
		
		websWrite(wp,
		T("<tr><th align=left><b>Login Name:</b></th>\n<td>"
		"<input type=\"text\" name=\"name\" maxlength=%d value=%s></td></tr>\n"
		"<tr><th align=left><b>Password:</b></th>\n"
		"<td><input type=\"password\" name=\"passwd\" maxlength=%d value=%s></td></tr>\n"
		"<tr><th align=left><b>Authentication Method:</b></th>\n<td>"
		"<select size=1 name=\"auth\">\n"
		"<option %s value=0>AUTO</option>\n"
		"<option %s value=1>PAP</option>\n"
		"<option %s value=2>CHAP</option></select></td></tr>\n"),
#ifdef CONFIG_GUI_WEB
		50, pEntry->pppUsername, 50, pEntry->pppPassword,
#else
		MAX_PPP_NAME_LEN, pEntry->pppUsername, MAX_NAME_LEN-1, pEntry->pppPassword,
#endif
		pEntry->pppAuth==PPP_AUTH_AUTO?"selected":"",
		pEntry->pppAuth==PPP_AUTH_PAP?"selected":"",
		pEntry->pppAuth==PPP_AUTH_CHAP?"selected":"");
		
		websWrite(wp,
		T("<tr><th align=left><b>Connection Type:</b></th>\n<td>"
		"<select size=1 name=\"pppConnectType\" onChange=\"pppTypeSelection()\">\n"
		"<option %s value=0>Continuous</option>\n"
		"<option %s value=1>Connect on Demand</option>\n"
		"<option %s value=2>Manual</option></select></td></tr>\n"
		"<tr><th align=left><b>Idle Time:</b></th>\n<td>"
		"<input type=\"text\" name=\"pppIdleTime\" maxlength=3 value=%d %s></td></tr>"),
		pEntry->pppCtype==CONTINUOUS?"selected":"",
		pEntry->pppCtype==CONNECT_ON_DEMAND?"selected":"",
		pEntry->pppCtype==MANUAL?"selected":"",
		pEntry->pppIdleTime,
		pEntry->pppCtype==CONNECT_ON_DEMAND?"":"disabled");
		
#ifdef _CWMP_MIB_
		websWrite(wp,
		T("<tr><th align=left><b>Auto Disconnect Time:</b></th>\n<td>"
		"<input type=\"text\" name=\"disconnectTime\" value=%d maxlength=10></td></tr>\n"),
		pEntry->autoDisTime);
		websWrite(wp,
		T("<tr><th align=left><b>Warn Disconnect Delay:</b></th>\n<td>"
		"<input type=\"text\" name=\"disconnectDelay\" value=%d maxlength=10></td></tr>\n"),
		pEntry->warnDisDelay);
#endif
		
#ifdef DEFAULT_GATEWAY_V1
		websWrite(wp,
		T("<tr><th align=left><b>Default Route:</b></th>\n<td>"
		"<input type=radio value=0 name=\"droute\" %s>Disable&nbsp;&nbsp;\n"
		"<input type=radio value=1 name=\"droute\" %s>Enable\n</td></tr>\n"),
		pEntry->dgw? "":"checked", pEntry->dgw? "checked":"");
#endif
		
		websWrite(wp,
		T("<tr><th align=left><b>MTU:</b></th>\n<td>"
		"<input type=\"text\" name=\"mru\" value=%d maxlength=4></td></tr>\n"),
		pEntry->mtu);
		
		if (pEntry->cmode == ADSL_PPPoE) {
#ifdef CONFIG_SPPPD_STATICIP
			writeStaticIP(wp, pEntry, fmName);
#endif
#ifdef PPPOE_PASSTHROUGH
			#if 0
			websWrite(wp,
			T("<tr><th align=left><b>PPPoE pass-through:</b></th>\n<td>"
			"<input type=radio value=0 name=\"poe\" %s>Disable&nbsp;&nbsp;\n"
			"<input type=radio value=1 name=\"poe\" %s>Enable\n</td></tr>\n"),
			//pEntry->brmode? "":"checked", pEntry->brmode? "checked":"");
			(pEntry->brmode==BRIDGE_ETHERNET)? "checked":"", (pEntry->brmode==BRIDGE_PPPOE)? "checked":"");
			#endif
			websWrite(wp,
			T("<tr><th align=left><b>Bridge:</b></th>\n<td>"
			"<input type=radio value=0 name=\"mode\" %s>Bridged Ethernet"
			" (Transparent Bridging)&nbsp;&nbsp;\n</td></tr>\n"
			"<tr><th></th>\n<td>\n"
			"<input type=radio value=1 name=\"mode\" %s>Bridged PPPoE"
			" (implies Bridged Ethernet)\n</td></tr>\n"
			"<tr><th></th>\n<td>\n"
			"<input type=radio value=2 name=\"mode\" %s>Disable Bridge"
			" \n</td></tr>\n"),
			pEntry->brmode? "":"checked", (pEntry->brmode==BRIDGE_PPPOE)? "checked":"", (pEntry->brmode==BRIDGE_DISABLE)? "checked":"");
#endif
			
			websWrite(wp,
			T("<tr><th align=left><b>AC-Name:</b></th>\n<td>"
//			"<input type=\"text\" name=\"acName\" maxlength=30 value=%s></td></tr>\n"),
			"<input type=\"text\" name=\"acName\" maxlength=29 value=%s></td></tr>\n"),
			pEntry->pppACName);

#ifdef _CWMP_MIB_
			websWrite(wp,
			T("<tr><th align=left><b>Service-Name:</b></th>\n<td>"
//			"<input type=\"text\" name=\"serviceName\" maxlength=30 value=%s></td></tr>\n"),
			"<input type=\"text\" name=\"serviceName\" maxlength=29 value=%s></td></tr>\n"),
			pEntry->pppServiceName);
#endif

#ifdef CONFIG_EXT_SWITCH
			write1q(wp, pEntry, fmName);
#endif
#ifdef CONFIG_USER_PPPOE_PROXY
                          websWrite(wp,
			T("<tr><th align=left><b>PPPoE Proxy:</b></th>\n<td>"
			"<input type=radio value=0 name=\"pproxy\" %s>Disable&nbsp;&nbsp;\n"
			"<input type=radio value=1 name=\"pproxy\" %s>Enable\n</td></tr>\n"),
			//pEntry->brmode? "":"checked", pEntry->brmode? "checked":"");
			
			(!pEntry->PPPoEProxyEnable)? "checked":"", (pEntry->PPPoEProxyEnable)? "checked":"");

                          websWrite(wp,
			T("<tr><th align=left><b>Max User Nums:</b></th>\n<td>"
			"<input type=\"text\" name=\"maxusernums\" maxlength=2 value=%d></td></tr>\n"),
			pEntry->PPPoEProxyMaxUser);
#endif			  

		}
#ifdef WEB_ENABLE_PPP_DEBUG
		int pppdbg = pppdbg_get(PPP_INDEX(pEntry->ifIndex));
		websWrite(wp,
		T("<tr><th align=left><b>Debug:</b></th>\n<td>\n"
		"<input type=radio value=0 name=\"pppdebug\" %s>Disable&nbsp;&nbsp;\n"
		"<input type=radio value=1 name=\"pppdebug\" %s>Enable\n</td></tr>\n"),
		pppdbg? "":"checked", pppdbg? "checked":"");
#endif
		if ( strcmp(directory_index, "/admin/index_user.html") == 0) {
			websWrite(wp,
//			T("</table>\n<br><input type=submit value=\"Apply Changes\" name=\"save\">\n"
			T("</table>\n<br><input type=submit value=\"Apply Changes\" name=\"save\" onClick=\"return applyPPP()\">\n"
			"<input type=submit value=\"Return\" name=\"return\">\n"
			"<input type=reset value=\"Undo\" name=\"reset\">\n"
			"<input type=hidden value=%d name=\"item\">\n"
			"<input type=hidden value=\"/admin/wanadsl_user.asp\" name=\"submit-url\">\n"
			"</form></blockquote></body>"), index);
		} else {
		        websWrite(wp,
			 //T("</table>\n<br><input type=submit value=\"Apply Changes\" name=\"save\">\n"
		        T("</table>\n<br><input type=submit value=\"Apply Changes\" name=\"save\" onClick=\"return applyPPP()\">\n"
		        "<input type=submit value=\"Return\" name=\"return\">\n"
		        "<input type=reset value=\"Undo\" name=\"reset\">\n"
		        "<input type=hidden value=%d name=\"item\">\n"
		        "<input type=hidden value=\"/wanadsl.asp\" name=\"submit-url\">\n"
		        "</form></blockquote></body>"), index);
                }
}
#endif

MIB_CE_ATM_VC_T tmpEntry;
#ifdef ZTE_GENERAL_ROUTER_SC
void writeIPEdit(webs_t wp, MIB_CE_ATM_VC_Tp pEntry, int index)
{
		char *fmName="ip";
		char *proto;
		char ipAddr[20], remoteIp[20], netMask[20];
		
		if (pEntry->cmode == ADSL_MER1483)
			proto = "MER";
		else
		if (pEntry->cmode == ADSL_RT1483)
			proto = "1483 routed";
#ifdef CONFIG_ATM_CLIP
		else if (pEntry->cmode == ADSL_RT1577)
			proto = "1577 routed";
#endif
		
#ifdef DEFAULT_GATEWAY_V1
		pdgw=pEntry->dgw;
#endif
		memcpy(&tmpEntry, pEntry, sizeof(tmpEntry));
		
		// Javascript
		websWrite(wp, T("<head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=gb2312\">\n<script>\nfunction ipTypeSelection()\n{\n"
		"if ( document.%s.dhcp[0].checked) {\n"
		"document.%s.ipaddr.disabled = false;\n"
		"document.%s.remoteip.disabled = false;\n"
		"document.%s.netmask.disabled = false;\n}\nelse {\n"
		"document.%s.ipaddr.value = \"\";\n"
		"document.%s.remoteip.value = \"\";\n"
		"document.%s.netmask.value = \"\";\n"
		"document.%s.ipaddr.disabled = true;\n"
		"document.%s.remoteip.disabled = true;\n"
		"document.%s.netmask.disabled = true;\n}\n}"
		),  fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName);
//		"\n</script>\n</head>\n"),  fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName);

		websWrite(wp, T("\nfunction ipModeSelection()\n{\n"
		"if ( document.%s.ipunnumber[1].checked) {\n"
		"document.%s.ipaddr.disabled = true;\n"
		"document.%s.remoteip.disabled = true;\n"
		"document.%s.netmask.disabled = true;\n}\n}"
/*		"document.%s.dhcp[0].disabled = true;\n"
		"document.%s.dhcp[1].disabled = true;\n}\nelse {\n"
		"document.%s.dhcp[0].disabled = false;\n"
		"document.%s.dhcp[1].disabled = false;\n}\n}"
		"ipTypeSelection();\n}\n}"*/
		"\n</script>\n</head>\n"),  fmName, fmName, fmName, fmName);
//		"\n</script>\n</head>\n"),  fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName);
		
		// body
		websWrite(wp,
		T("<body><blockquote><h2><font color=\"#0000FF\">IP接口-修改</font></h2>\n"
		"<form action=/goform/formIPEdit method=POST name=\"%s\">\n"
		"<table border=0 width=600 cellspacing=4 cellpadding=0>"
		"<tr>\n<th align=left><b>IP接口:</b></th>\n<td>vc%d</td></tr>\n"
		"<tr>\n<th align=left><b>协议:</b></th>\n<td>%s</td></tr>\n"
		"<tr>\n<th align=left><b>ATM虚连接:</b></th>\n<td>%d/%d</td></tr>\n"),
		fmName, VC_INDEX(pEntry->ifIndex), proto, pEntry->vpi, pEntry->vci);
		
		websWrite(wp,
		T("<tr><th align=left><b>状态:</b></th>\n<td>\n"
		"<input type=radio value=0 name=\"status\" %s>禁止&nbsp;&nbsp;\n"
		"<input type=radio value=1 name=\"status\" %s>允许\n</td></tr>\n"),
		pEntry->enable? "":"checked", pEntry->enable? "checked":"");

		if (pEntry->cmode == ADSL_RT1483) {
			websWrite(wp,
			T("<tr><th align=left><b>Unnumbered:</b></th>\n<td>\n"
			"<input type=radio value=0 name=\"ipunnumber\" %s onClick=\"ipModeSelection()\">禁止&nbsp;&nbsp;\n"
			"<input type=radio value=1 name=\"ipunnumber\" %s onClick=\"ipModeSelection()\">允许\n</td></tr>\n"),
			pEntry->ipunnumbered? "":"checked", pEntry->ipunnumbered? "checked":"");
		}
		
		if (pEntry->cmode == ADSL_MER1483) {
			websWrite(wp,
			T("<tr><th align=left><b>使用DHCP:</b></th>\n<td>\n"
			"<input type=radio value=0 name=\"dhcp\" %s onClick=\"ipTypeSelection()\">禁止&nbsp;&nbsp;\n"
			"<input type=radio value=1 name=\"dhcp\" %s onClick=\"ipTypeSelection()\">允许\n</td></tr>\n"),
			pEntry->ipDhcp? "":"checked", pEntry->ipDhcp? "checked":"");
		}
		
		strcpy(ipAddr, inet_ntoa(*((struct in_addr *)pEntry->ipAddr)));
		strcpy(remoteIp, inet_ntoa(*((struct in_addr *)pEntry->remoteIpAddr)));
		strcpy(netMask, inet_ntoa(*((struct in_addr *)pEntry->netMask)));
		
		websWrite(wp,
		T("<tr><th align=left><b>本地IP地址:</b></th>\n<td>"
		"<input type=\"text\" name=\"ipaddr\" maxlength=15 value=%s %s></td></tr>\n"
		"<tr><th align=left><b>网关:</b></th>\n"
		"<td><input type=\"text\" name=\"remoteip\" maxlength=15 value=%s %s></td></tr>\n"
		"<tr><th align=left><b>子网掩码:</b></th>\n"
		"<td><input type=\"text\" name=\"netmask\" maxlength=15 value=%s %s></td></tr>\n"),
		ipAddr, (pEntry->ipDhcp || pEntry->ipunnumbered==1)?"disabled":"", 
		remoteIp, (pEntry->ipDhcp || pEntry->ipunnumbered==1)?"disabled":"", 
		netMask, (pEntry->ipDhcp || pEntry->ipunnumbered==1)?"disabled":"");
#ifdef DEFAULT_GATEWAY_V1		
		websWrite(wp,
		T("<tr><th align=left><b>默认路由:</b></th>\n<td>"
		"<input type=radio value=0 name=\"droute\" %s>禁止&nbsp;&nbsp;\n"
		"<input type=radio value=1 name=\"droute\" %s>允许\n</td></tr>\n"),
		pEntry->dgw? "":"checked", pEntry->dgw? "checked":"");
#endif		
//		websWrite(wp,
//		T("<tr><th align=left><b>MTU:</b></th>\n<td>"
//		"<input type=\"text\" name=\"mtu\" value=%d maxlength=4></td></tr>\n"),
//		pEntry->mtu);
		
		if (pEntry->cmode == ADSL_MER1483) {
#ifdef PPPOE_PASSTHROUGH
			#if 0
			websWrite(wp,
			T("<tr><th align=left><b>PPPoE pass-through:</b></th>\n<td>"
			"<input type=radio value=0 name=\"poe\" %s>Disable&nbsp;&nbsp;\n"
			"<input type=radio value=1 name=\"poe\" %s>Enable\n</td></tr>\n"),
			//pEntry->brmode? "":"checked", pEntry->brmode? "checked":"");
			(pEntry->brmode==BRIDGE_ETHERNET)? "checked":"", (pEntry->brmode==BRIDGE_PPPOE)? "checked":"");
			#endif
			websWrite(wp,
			T("<tr><th align=left><b>网桥:</b></th>\n<td>"
			"<input type=radio value=0 name=\"mode\" %s>以太网网桥"
			" (透明网桥)&nbsp;&nbsp;\n</td></tr>\n"
			"<tr><th></th>\n<td>\n"
			"<input type=radio value=1 name=\"mode\" %s>PPPoE网桥"
			" (只有PPPoE包可以通过)\n</td></tr>\n"
			"<tr><th></th>\n<td>\n"
			"<input type=radio value=2 name=\"mode\" %s>网桥禁止(禁止该通道上的网桥功能)"
			" \n</td></tr>\n"),
			pEntry->brmode? "":"checked", (pEntry->brmode==BRIDGE_PPPOE)? "checked":"", (pEntry->brmode==BRIDGE_DISABLE)? "checked":"");
#endif
#ifdef CONFIG_EXT_SWITCH
			write1q(wp, pEntry, fmName);
#endif
		}
		
		websWrite(wp,
		T("</table>\n<br><input type=submit value=\"应用\" name=\"save\">\n"
		"<input type=submit value=\"返回\" name=\"return\">\n"
		"<input type=reset value=\"清空\" name=\"reset\">\n"
		"<input type=hidden value=%d name=\"item\">\n"
		"<input type=hidden value=\"/wanadsl_sc.asp\" name=\"submit-url\">\n"), index);
//		"<input type=hidden value=\"/wanadsl.asp\" name=\"submit-url\">\n"
		if (pEntry->cmode == ADSL_RT1483)
			websWrite(wp,"<script>ipModeSelection();</script>");
		websWrite(wp,
		"</form></blockquote></body>");
//		"</form></blockquote></body>"), index);
}

#else
void writeIPEdit(webs_t wp, MIB_CE_ATM_VC_Tp pEntry, int index)
{
		char *fmName="ip";
		char *proto;
		char ipAddr[20], remoteIp[20], netMask[20];
		
		if (pEntry->cmode == ADSL_MER1483)
			proto = "MER";
		else
		if (pEntry->cmode == ADSL_RT1483)
			proto = "1483 routed";
#ifdef CONFIG_ATM_CLIP
		else if (pEntry->cmode == ADSL_RT1577)
			proto = "1577 routed";
#endif
		
#ifdef DEFAULT_GATEWAY_V1
		pdgw=pEntry->dgw;
#endif
		memcpy(&tmpEntry, pEntry, sizeof(tmpEntry));
		
		// Javascript
		websWrite(wp, T("<head>\n<script>\n"));
		websWrite(wp,
		T("\nfunction getDigit(str, num)\n{\n"
		"\ti = 1;\n"
		"\tif (num != 1) {\n"
		"\t\twhile (i!=num && str.length!=0) {\n"
		"\t\t\tif ( str.charAt(0) == '.' ) {\n"
		"\t\t\t\ti ++;\n\t\t\t}\n"
		"\t\t\tstr = str.substring(1);\n\t\t}\n"
		"\t\tif (i!=num)\n\t\t\treturn -1;\n\t}\n"
		"\tfor (i=0; i<str.length; i++) {\n"
		"\t\tif (str.charAt(i) == '.') {\n"
		"\t\t\tstr = str.substring(0, i);\n\t\t\tbreak;\n\t\t}\n\t}\n"
		"\tif (str.length == 0)\n\t\treturn -1;\n"
		"\td = parseInt(str, 10);\n"
		"\treturn d;\n}\n"));
		websWrite(wp,
		T("\nfunction validateKey(str)\n{\n"
		"\tfor (var i=0; i<str.length; i++) {\n"
		"\t\tif ((str.charAt(i) >= '0' && str.charAt(i) <= '9') || (str.charAt(i) == '.' ))\n"
		"\t\t\tcontinue;\n\t\treturn 0;\n\t}\n\treturn 1;\n}\n"));
		websWrite(wp,
		T("\nfunction IsInvalidIP(str)\n{\n"
		"\td = getDigit(str, 1);\n"
		"\tif (d == 127)\n"
		"\t\treturn 1;\n\treturn 0;\n}\n"));
		websWrite(wp,
		T("\nfunction checkDigitRange(str, num, min, max)\n{\n"
		"\td = getDigit(str,num);\n"
		"\tif (d > max || d < min)\n"
		"\t\treturn false;\n\treturn true;\n}\n"));
		websWrite(wp,
		T("\nfunction checkIP(ip)\n{\n"
		"\tif (ip.value==\"\") {\n"
		"\t\talert(\"IP address cannot be empty! It should be filled with 4 digit numbers as xxx.xxx.xxx.xxx.\");\n"
		"\t\tip.focus();\n\t\treturn false;\n\t}\n"
		"\tif (validateKey(ip.value) == 0) {\n"
		"\t\talert(\"Invalid IP address value. It should be the decimal number (0-9).\");\n"
		"\t\tip.focus();\n\t\treturn false;\n\t}\n"
		"\tif (IsInvalidIP( ip.value)==1) {\n"
		"\t\talert(\"Invalid IP address value\");\n"
		"\t\tip.focus();\n\t\treturn false;\n\t}\n"));
		websWrite(wp,
		T("\tif (!checkDigitRange(ip.value,1,1,254)) {\n"
		"\t\talert('Invalid IP address range in 1st digit. It should be 1-254.');\n"
		"\t\tip.focus();\n\t\treturn false;\n\t}\n"
		"\tif (!checkDigitRange(ip.value,2,0,255)) {\n"
		"\t\talert('Invalid IP address range in 2nd digit. It should be 0-255.');\n"
		"\t\tip.focus();\n\t\treturn false;\n\t}\n"
		"\tif (!checkDigitRange(ip.value,3,0,255)) {\n"
		"\t\talert('Invalid IP address range in 3rd digit. It should be 0-255.');\n"
		"\t\tip.focus();\n\t\treturn false;\n\t}\n"
		"\tif (!checkDigitRange(ip.value,4,0,254)) {\n"
		"\t\talert('Invalid IP address range in 4th digit. It should be 1-254.');\n"
		"\t\tip.focus();\n\t\treturn false;\n\t}\n\treturn true;\n}\n"));
		websWrite(wp,
		T("\nfunction checkMask(netmask)\n{\n"
		"\tvar i, d;\n"
		"\tif (netmask.value==\"\") {\n"
		"\t\talert(\"Subnet mask cannot be empty! It should be filled with 4 digit numbers as xxx.xxx.xxx.xxx.\");\n"
		"\t\tnetmask.focus();\n\t\treturn false;\n\t}\n"
		"\tif (validateKey(netmask.value) == 0) {\n"
		"\t\talert(\"Invalid subnet mask value. It should be the decimal number (0-9).\");\n"
		"\t\tnetmask.focus();\n\t\treturn false;\n\t}\n"
		"\tfor (i=1; i<=4; i++) {\n"
		"\t\td = getDigit(netmask.value,i);\n"
		"\t\tif( !(d==0 || d==128 || d==192 || d==224 || d==240 || d==248 || d==252 || d==254 || d==255 )) {\n"
		"\t\t\talert('Invalid subnet mask digit.It should be the number of 0, 128, 192, 224, 240, 248, 252 or 254');\n"
		"\t\t\tnetmask.focus();\n\t\t\treturn false;\n\t\t}\n\t}\n"
		"\treturn true;\n}\n"));
		if (pEntry->cmode == ADSL_MER1483)
			websWrite(wp,
			T("\nfunction ipTypeSelection()\n{\n"
			"if ( document.%s.dhcp[0].checked) {\n"
			"document.%s.ipaddr.disabled = false;\n"
#ifdef DEFAULT_GATEWAY_V1
			"document.%s.remoteip.disabled = false;\n"
#endif
			"document.%s.netmask.disabled = false;\n}\nelse {\n"
			"document.%s.ipaddr.value = \"\";\n"
#ifdef DEFAULT_GATEWAY_V1
			"document.%s.remoteip.value = \"\";\n"
#endif
			"document.%s.netmask.value = \"\";\n"
			"document.%s.ipaddr.disabled = true;\n"
#ifdef DEFAULT_GATEWAY_V1
			"document.%s.remoteip.disabled = true;\n"
#endif
			"document.%s.netmask.disabled = true;\n}\n}"
#ifdef DEFAULT_GATEWAY_V1
			),  fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName);
#else
			), fmName, fmName, fmName, fmName, fmName, fmName, fmName);
#endif
//		"\n</script>\n</head>\n"),  fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName);
		if (pEntry->cmode == ADSL_RT1483)
			websWrite(wp, T("\nfunction ipModeSelection()\n{\n"
			"if ( document.%s.ipunnumber[1].checked) {\n"
			"document.%s.ipaddr.disabled = true;\n"
			//"document.%s.remoteip.disabled = true;\n"
			//"document.%s.netmask.disabled = true;\n}\n"
#ifdef DEFAULT_GATEWAY_V1
			"document.%s.remoteip.disabled = true;\n}\n"
#else
			"}\n"
#endif
			"else if (document.%s.ipunnumber[0].checked) {\n"
			"document.%s.ipaddr.disabled = false;\n"
			//"document.%s.remoteip.disabled = false;\n"
			//"document.%s.netmask.disabled = false;\n}\n}"),  fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName);
#ifdef DEFAULT_GATEWAY_V1
			"document.%s.remoteip.disabled = false;\n}\n"
			"document.%s.netmask.disabled = true;\n}"),  fmName, fmName, fmName, fmName, fmName, fmName, fmName);
#else
			"}\n"
			"document.%s.netmask.disabled = true;\n}"),  fmName, fmName, fmName, fmName, fmName);
#endif

		websWrite(wp, T("\nfunction applyIP()\n{"));
		if (pEntry->cmode == ADSL_MER1483) {
#ifdef CONFIG_EXT_SWITCH
			websWrite(wp,
			T("\n\tif (!check1q(document.%s.vid.value)) {"
			"\n\t\talert(\"Invalid VLAN ID!\");"
			"\n\t\tdocument.%s.vid.focus();"
			"\n\t\treturn false;\n\t}"), fmName, fmName);
#endif
			websWrite(wp,
			T("\n\tif (document.%s.dhcp[0].checked) {"
			"\n\t\tif (!checkIP(document.%s.ipaddr))\n\t\treturn false;"
#ifdef DEFAULT_GATEWAY_V1
			"\n\t\tif (!checkIP(document.%s.remoteip))\n\t\treturn false;"
			"\n\t\tif (!checkMask(document.%s.netmask))\n\t\treturn false;"
			), fmName, fmName, fmName, fmName);
#else
			"\n\t\tif (!checkMask(document.%s.netmask))\n\t\treturn false;"
			), fmName, fmName, fmName);
#endif
		}
		if (pEntry->cmode == ADSL_RT1483)
			websWrite(wp,
			T("\n\tif (document.%s.ipunnumber[0].checked) {"
			"\n\t\tif (!checkIP(document.%s.ipaddr))\n\t\t\treturn false;"
#ifdef DEFAULT_GATEWAY_V1
			"\n\t\tif (!checkIP(document.%s.remoteip))\n\t\t\treturn false;"
			), fmName, fmName, fmName);
#else
			), fmName, fmName);
#endif
			//"\n\t\tif (!checkMask(document.%s.netmask))\n\t\t\treturn false;"
			//), fmName, fmName, fmName, fmName);
		websWrite(wp, T("\n\t}\n\treturn true;\n}\n"));
/*		"document.%s.dhcp[0].disabled = true;\n"
		"document.%s.dhcp[1].disabled = true;\n}\nelse {\n"
		"document.%s.dhcp[0].disabled = false;\n"
		"document.%s.dhcp[1].disabled = false;\n}\n}"
		"ipTypeSelection();\n}\n}"*/
		websWrite(wp, T("\n</script>\n</head>\n"));
//		"\n</script>\n</head>\n"),  fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName);
		
		// body
		websWrite(wp,
		T("<body><blockquote><h2><font color=\"#0000FF\">IP Interface - Modify</font></h2>\n"
		"<form action=/goform/admin/formIPEdit method=POST name=\"%s\">\n"
		"<table border=0 width=600 cellspacing=4 cellpadding=0>"
		"<tr>\n<th align=left><b>IP Interface:</b></th>\n<td>vc%d</td></tr>\n"
		"<tr>\n<th align=left><b>Protocol:</b></th>\n<td>%s</td></tr>\n"
		"<tr>\n<th align=left><b>ATM VCC:</b></th>\n<td>%d/%d</td></tr>\n"),
		fmName, VC_INDEX(pEntry->ifIndex), proto, pEntry->vpi, pEntry->vci);
		
		websWrite(wp,
		T("<tr><th align=left><b>Status:</b></th>\n<td>\n"
		"<input type=radio value=0 name=\"status\" %s>Disable&nbsp;&nbsp;\n"
		"<input type=radio value=1 name=\"status\" %s>Enable\n</td></tr>\n"),
		pEntry->enable? "":"checked", pEntry->enable? "checked":"");
		
		if (pEntry->cmode == ADSL_RT1483) {
			websWrite(wp,
			T("<tr><th align=left><b>Unnumbered:</b></th>\n<td>\n"
			"<input type=radio value=0 name=\"ipunnumber\" %s onClick=\"ipModeSelection()\">Disable&nbsp;&nbsp;\n"
			"<input type=radio value=1 name=\"ipunnumber\" %s onClick=\"ipModeSelection()\">Enable\n</td></tr>\n"),
			pEntry->ipunnumbered? "":"checked", pEntry->ipunnumbered? "checked":"");
		}
		
		if (pEntry->cmode == ADSL_MER1483) {
			websWrite(wp,
			T("<tr><th align=left><b>Use DHCP:</b></th>\n<td>\n"
			"<input type=radio value=0 name=\"dhcp\" %s onClick=\"ipTypeSelection()\">Disable&nbsp;&nbsp;\n"
			"<input type=radio value=1 name=\"dhcp\" %s onClick=\"ipTypeSelection()\">Enable\n</td></tr>\n"),
			pEntry->ipDhcp? "":"checked", pEntry->ipDhcp? "checked":"");
		}
		
		strcpy(ipAddr, inet_ntoa(*((struct in_addr *)pEntry->ipAddr)));
#ifdef DEFAULT_GATEWAY_V1
		strcpy(remoteIp, inet_ntoa(*((struct in_addr *)pEntry->remoteIpAddr)));
#endif
		if (pEntry->cmode == ADSL_MER1483)
			strcpy(netMask, inet_ntoa(*((struct in_addr *)pEntry->netMask)));
		else
			strcpy(netMask, "");
		
		websWrite(wp,
		T("<tr><th align=left><b>Local IP Address:</b></th>\n<td>"
		"<input type=\"text\" name=\"ipaddr\" maxlength=15 value=\"%s\" %s></td></tr>\n"
#ifdef DEFAULT_GATEWAY_V1
		"<tr><th align=left><b>Remote IP Address:</b></th>\n"
		"<td><input type=\"text\" name=\"remoteip\" maxlength=15 value=\"%s\" %s></td></tr>\n"
#endif
		"<tr><th align=left><b>Subnet Mask:</b></th>\n"
		"<td><input type=\"text\" name=\"netmask\" maxlength=15 value=\"%s\" %s></td></tr>\n"),
		ipAddr, (pEntry->ipDhcp || pEntry->ipunnumbered==1)?"disabled":"", 
#ifdef DEFAULT_GATEWAY_V1
		remoteIp, (pEntry->ipDhcp || pEntry->ipunnumbered==1)?"disabled":"", 
#endif
		netMask, (pEntry->ipDhcp || pEntry->cmode == ADSL_RT1483)?"disabled":"");
		//netMask, (pEntry->ipDhcp || pEntry->ipunnumbered==1)?"disabled":"");
		
#ifdef DEFAULT_GATEWAY_V1
		websWrite(wp,
		T("<tr><th align=left><b>Default Route:</b></th>\n<td>"
		"<input type=radio value=0 name=\"droute\" %s>Disable&nbsp;&nbsp;\n"
		"<input type=radio value=1 name=\"droute\" %s>Enable\n</td></tr>\n"),
		pEntry->dgw? "":"checked", pEntry->dgw? "checked":"");
#endif
		
//		websWrite(wp,
//		T("<tr><th align=left><b>MTU:</b></th>\n<td>"
//		"<input type=\"text\" name=\"mtu\" value=%d maxlength=4></td></tr>\n"),
//		pEntry->mtu);
		
		if (pEntry->cmode == ADSL_MER1483) {
#ifdef PPPOE_PASSTHROUGH
			#if 0
			websWrite(wp,
			T("<tr><th align=left><b>PPPoE pass-through:</b></th>\n<td>"
			"<input type=radio value=0 name=\"poe\" %s>Disable&nbsp;&nbsp;\n"
			"<input type=radio value=1 name=\"poe\" %s>Enable\n</td></tr>\n"),
			//pEntry->brmode? "":"checked", pEntry->brmode? "checked":"");
			(pEntry->brmode==BRIDGE_ETHERNET)? "checked":"", (pEntry->brmode==BRIDGE_PPPOE)? "checked":"");
			#endif
			websWrite(wp,
			T("<tr><th align=left><b>Bridge:</b></th>\n<td>"
			"<input type=radio value=0 name=\"mode\" %s>Bridged Ethernet"
			" (Transparent Bridging)&nbsp;&nbsp;\n</td></tr>\n"
			"<tr><th></th>\n<td>\n"
			"<input type=radio value=1 name=\"mode\" %s>Bridged PPPoE"
			" (implies Bridged Ethernet)\n</td></tr>\n"
			"<tr><th></th>\n<td>\n"
			"<input type=radio value=2 name=\"mode\" %s>Disable Bridge"
			" \n</td></tr>\n"),
			pEntry->brmode? "":"checked", (pEntry->brmode==BRIDGE_PPPOE)? "checked":"", (pEntry->brmode==BRIDGE_DISABLE)? "checked":"");
#endif
#ifdef CONFIG_EXT_SWITCH
			write1q(wp, pEntry, fmName);
#endif
		}
		
		// Mason Yu. Set MTU
		websWrite(wp,
		T("<tr><th align=left><b>MTU:</b></th>\n<td>"
		"<input type=\"text\" name=\"mru\" value=%d maxlength=4></td></tr>\n"),
		pEntry->mtu);
		
		if ( strcmp(directory_index, "/admin/index_user.html") == 0) {
			websWrite(wp,
//			T("</table>\n<br><input type=submit value=\"Apply Changes\" name=\"save\">\n"
			T("</table>\n<br><input type=submit value=\"Apply Changes\" name=\"save\" onClick=\"return applyIP()\">\n"
			"<input type=submit value=\"Return\" name=\"return\">\n"
			"<input type=reset value=\"Undo\" name=\"reset\">\n"
			"<input type=hidden value=%d name=\"item\">\n"
			"<input type=hidden value=\"/admin/wanadsl_user.asp\" name=\"submit-url\">\n"), index);
		} else {
		        websWrite(wp,
//		        T("</table>\n<br><input type=submit value=\"Apply Changes\" name=\"save\">\n"
			 T("</table>\n<br><input type=submit value=\"Apply Changes\" name=\"save\" onClick=\"return applyIP()\">\n"
		        "<input type=submit value=\"Return\" name=\"return\">\n"
		        "<input type=reset value=\"Undo\" name=\"reset\">\n"
		        "<input type=hidden value=%d name=\"item\">\n"
		        "<input type=hidden value=\"/wanadsl.asp\" name=\"submit-url\">\n"), index);
		}
//		"<input type=hidden value=\"/wanadsl.asp\" name=\"submit-url\">\n"
		if (pEntry->cmode == ADSL_RT1483)
			websWrite(wp,"<script>ipModeSelection();</script>");
		websWrite(wp,
		"</form></blockquote></body>");
//		"</form></blockquote></body>"), index);
}
#endif

#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
void writeBrEdit(webs_t wp, MIB_CE_ATM_VC_Tp pEntry, int index)
{
		char *fmName="brif";
		char *proto;
		
		proto = "ENET";
		
		// head
		websWrite(wp,T("<head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=gb2312\">\n"));
		websWrite(wp,T("<script>function saveChanges(){\n"));
		websWrite(wp,T("if(document.brif.vid){\n"));
		websWrite(wp,T("d=parseInt(document.brif.vid.value,10);\n"));
		websWrite(wp,T("if(d<0||d>4095){\n"));
		websWrite(wp,T("alert(\"VLAN VID应该在0到4095之间!\");\n"));
		websWrite(wp,T("return false;}}\n"));
		websWrite(wp,T("return true;}</script>\n"));
		websWrite(wp,T("</head>\n"));

		// body
		websWrite(wp,
		T("<body><blockquote><h2><font color=\"#0000FF\">网桥接口－修改</font></h2>\n"
		"<form action=/goform/formBrEdit method=POST name=\"%s\">\n"
		"<table border=0 width=500 cellspacing=4 cellpadding=0>"
		"<tr>\n<th align=left><b>网桥接口:</b></th>\n<td>vc%d</td></tr>\n"
		"<tr>\n<th align=left><b>协议:</b></th>\n<td>%s</td></tr>\n"
		"<tr>\n<th align=left><b>ATM虚连接:</b></th>\n<td>%d/%d</td></tr>\n"),
		fmName, VC_INDEX(pEntry->ifIndex), proto, pEntry->vpi, pEntry->vci);
		
		websWrite(wp,
		T("<tr><th align=left><b>状态:</b></th>\n<td>\n"
		"<input type=radio value=0 name=\"status\" %s>禁止&nbsp;&nbsp;\n"
		"<input type=radio value=1 name=\"status\" %s>允许\n</td></tr>\n"),
		pEntry->enable? "":"checked", pEntry->enable? "checked":"");
		
#ifdef PPPOE_PASSTHROUGH
		websWrite(wp,
		T("<tr><th align=left><b>模式:</b></th>\n<td>"
		"<input type=radio value=0 name=\"mode\" %s>以太网网桥"
		" (透明网桥)&nbsp;&nbsp;\n</td></tr>\n"
		"<tr><th></th>\n<td>\n"
		"<input type=radio value=1 name=\"mode\" %s>PPPoE网桥"
		" (只有PPPoE包可以通过)\n</td></tr>\n"
/*
		"<tr><th></th>\n<td>\n"
		"<input type=radio value=2 name=\"mode\" %s>禁止网桥"
		" \n</td></tr>\n"*/),
		pEntry->brmode? "":"checked", (pEntry->brmode==BRIDGE_PPPOE)? "checked":"", (pEntry->brmode==BRIDGE_DISABLE)? "checked":"");
#endif
		
#ifdef CONFIG_EXT_SWITCH
		write1q(wp, pEntry, fmName);
#endif
		websWrite(wp,
		T("</table>\n<br><input type=submit value=\"应用\" name=\"save\" onClick=\"return saveChanges()\" >\n"
		"<input type=submit value=\"返回\" name=\"return\">\n"
		"<input type=reset value=\"清空\" name=\"reset\">\n"
		"<input type=hidden value=%d name=\"item\">\n"
		"<input type=hidden value=\"/wanadsl_sc.asp\" name=\"submit-url\">\n"), index);

		
		websWrite(wp, T("</form></blockquote></body>"));
}
#else

void writeBrEdit(webs_t wp, MIB_CE_ATM_VC_Tp pEntry, int index)
{
		char *fmName="brif";
		char *proto;
		
		proto = "ENET";
		
		// head
		websWrite(wp,T("<head>\n</head>\n"));
		
		// body
		websWrite(wp,
		T("<body><blockquote><h2><font color=\"#0000FF\">Bridged Interface - Modify</font></h2>\n"
		"<form action=/goform/admin/formBrEdit method=POST name=\"%s\">\n"
		"<table border=0 width=500 cellspacing=4 cellpadding=0>"
		"<tr>\n<th align=left><b>Bridged Interface:</b></th>\n<td>vc%d</td></tr>\n"
		"<tr>\n<th align=left><b>Protocol:</b></th>\n<td>%s</td></tr>\n"
		"<tr>\n<th align=left><b>ATM VCC:</b></th>\n<td>%d/%d</td></tr>\n"),
		fmName, VC_INDEX(pEntry->ifIndex), proto, pEntry->vpi, pEntry->vci);
		
		websWrite(wp,
		T("<tr><th align=left><b>Status:</b></th>\n<td>\n"
		"<input type=radio value=0 name=\"status\" %s>Disable&nbsp;&nbsp;\n"
		"<input type=radio value=1 name=\"status\" %s>Enable\n</td></tr>\n"),
		pEntry->enable? "":"checked", pEntry->enable? "checked":"");
		
#ifdef PPPOE_PASSTHROUGH
		websWrite(wp,
		T("<tr><th align=left><b>Mode:</b></th>\n<td>"
		"<input type=radio value=0 name=\"mode\" %s>Bridged Ethernet"
		" (Transparent Bridging)&nbsp;&nbsp;\n</td></tr>\n"
		"<tr><th></th>\n<td>\n"
		"<input type=radio value=1 name=\"mode\" %s>Bridged PPPoE"
		" (implies Bridged Ethernet)\n</td></tr>\n"
		"<tr><th></th>\n<td>\n"
		"<input type=radio value=2 name=\"mode\" %s>Disable Bridge"
		" \n</td></tr>\n"),
		pEntry->brmode? "":"checked", (pEntry->brmode==BRIDGE_PPPOE)? "checked":"", (pEntry->brmode==BRIDGE_DISABLE)? "checked":"");
#endif
		
#ifdef CONFIG_EXT_SWITCH
		write1q(wp, pEntry, fmName);
#endif
		if ( strcmp(directory_index, "/admin/index_user.html") == 0) {
#ifdef CONFIG_EXT_SWITCH
			websWrite(wp,
			T("</table>\n<br><input type=submit value=\"Apply Changes\" name=\"save\" onClick=\"return apply1q()\">\n"
			"<input type=submit value=\"Return\" name=\"return\">\n"
			"<input type=reset value=\"Undo\" name=\"reset\">\n"
			"<input type=hidden value=%d name=\"item\">\n"
			"<input type=hidden value=\"/admin/wanadsl_user.asp\" name=\"submit-url\">\n"), index);
#else
			websWrite(wp,
			T("</table>\n<br><input type=submit value=\"Apply Changes\" name=\"save\">\n"
			"<input type=submit value=\"Return\" name=\"return\">\n"
			"<input type=reset value=\"Undo\" name=\"reset\">\n"
			"<input type=hidden value=%d name=\"item\">\n"		
			"<input type=hidden value=\"/admin/wanadsl_user.asp\" name=\"submit-url\">\n"), index);
#endif
		} else {
#ifdef CONFIG_EXT_SWITCH
			websWrite(wp,
			T("</table>\n<br><input type=submit value=\"Apply Changes\" name=\"save\" onClick=\"return apply1q()\">\n"
			"<input type=submit value=\"Return\" name=\"return\">\n"
			"<input type=reset value=\"Undo\" name=\"reset\">\n"
			"<input type=hidden value=%d name=\"item\">\n"
			"<input type=hidden value=\"/wanadsl.asp\" name=\"submit-url\">\n"), index);
#else
			websWrite(wp,
		        T("</table>\n<br><input type=submit value=\"Apply Changes\" name=\"save\">\n"
		        "<input type=submit value=\"Return\" name=\"return\">\n"
		        "<input type=reset value=\"Undo\" name=\"reset\">\n"
		        "<input type=hidden value=%d name=\"item\">\n"
		        "<input type=hidden value=\"/wanadsl.asp\" name=\"submit-url\">\n"), index);
#endif
		
		}
		websWrite(wp, T("</form></blockquote></body>"));
}

#endif

/*--------------------------------------------------------------
 *	Check if user do some action in this page
 *	Return:
 *		0 :	no action
 *		1 :	do action
 *		-1:	error with message errMsg
 *-------------------------------------------------------------*/
int checkAction(webs_t wp, char *errMsg)
{
	char tmpBuf[100];
	char_t *strSubmit, *strValue;
	//MIB_CE_ATM_VC_Tp pEntry;
	MIB_CE_ATM_VC_T Entry;
	int action, index;
	
	strSubmit = websGetVar(wp, T("action"), T(""));
	action = -1;
	
	if (strSubmit[0]) {
		action = strSubmit[0] - '0';
		strSubmit = websGetVar(wp, T("idx"), T(""));
		index = strSubmit[0] - '0';
	}
	else
		return 0;	// no action
	
	if (action == 0) {	// delete
		deleteAllConnection();
		resolveServiceDependency(index);
		if (mib_chain_get(MIB_ATM_VC_TBL, index, (void *)&Entry))
		{
			delPortForwarding(Entry.ifIndex);
			delRoutingTable(Entry.ifIndex);
		}
		if(mib_chain_delete(MIB_ATM_VC_TBL, index) != 1) {
			strcpy(errMsg, T(strDelChainerror));
			return -1;
		}

#ifdef CONFIG_EXT_SWITCH
#ifdef ITF_GROUP
		{
			int wanPortNum;
			unsigned int swNum, vcNum;
			int j, grpnum;

			vcNum = mib_chain_total(MIB_ATM_VC_TBL);
			for (grpnum=1; grpnum<=4; grpnum++) {
				wanPortNum = 0;

				for (j=0; j<vcNum; j++) {
					if (!mib_chain_get(MIB_ATM_VC_TBL, j, (void *)&Entry)) {
			  			printf("Get chain record error!\n");
						return -1;
					}
					if (Entry.enable == 0 || Entry.itfGroup!=grpnum)
						continue;
						wanPortNum++;
				}

				if (0 == wanPortNum) {
					char mygroup;
					printf("delete port mapping group %d\n", grpnum);
					//release LAN ports
					swNum = mib_chain_total(MIB_SW_PORT_TBL);
					MIB_CE_SW_PORT_T swEntry;
					for (j=swNum; j>0; j--) {
						if (!mib_chain_get(MIB_SW_PORT_TBL, j-1, (void *)&swEntry))
							return -1;
						if (swEntry.itfGroup == grpnum) {
							swEntry.itfGroup = 0;
							mib_chain_update(MIB_SW_PORT_TBL, (void *)&swEntry, j-1);
						}
					}
#ifdef WLAN_SUPPORT
					//release wlan0
					mib_get(MIB_WLAN_ITF_GROUP, (void *)&mygroup);
					if (mygroup == grpnum) {
						mygroup = 0;
						mib_set(MIB_WLAN_ITF_GROUP, (void *)&mygroup);
					}
#endif
					setgroup("", grpnum);
					break;
				}
			}
		}//end
#endif	// of ITF_GROUP
#endif //CONFIG_EXT_SWITCH
#ifdef CONFIG_GUI_WEB
		writeFlash();
#endif
		restartWAN();
		/*RegisterPVCnumber();		
		cleanAllFirewallRule();
		restart_dnsrelay();
		startWan(BOOT_LAST);*/

	}
	else	
	if (action == 1) {	// modify
		if (!mib_chain_get(MIB_ATM_VC_TBL, index, (void *)&Entry)) {
			strcpy(errMsg, errGetEntry);
			return -1;
		}
		
		websHeader(wp);
		
		if (Entry.cmode == ADSL_PPPoE || Entry.cmode == ADSL_PPPoA)
			writePPPEdit(wp, &Entry, index);
		else
#ifdef CONFIG_ATM_CLIP
		if (Entry.cmode == ADSL_MER1483 || Entry.cmode == ADSL_RT1483 || Entry.cmode == ADSL_RT1577)
#else
		if (Entry.cmode == ADSL_MER1483 || Entry.cmode == ADSL_RT1483)
#endif
			writeIPEdit(wp, &Entry, index);
		else
		if (Entry.cmode == ADSL_BR1483)
			writeBrEdit(wp, &Entry, index);
		
		websFooter(wp);
	}
	
	strSubmit = websGetVar(wp, T("submit-url"), T(""));   // hidden page
	if (strSubmit[0] && action != 1)
		websRedirect(wp, strSubmit);
	else
		websDone(wp, 200);
	
  	return 1;
}

/////////////////////////////////////////////////////////////////////////////
#ifdef ZTE_GENERAL_ROUTER_SC
void assignDefaultValue(MIB_CE_ACC_T *accEntry)
{
accEntry->telnet=0;
accEntry->ftp=0;
accEntry->tftp=0;
accEntry->web=0;
accEntry->snmp=0;
accEntry->ssh=0;
accEntry->icmp=0;
accEntry->telnet_port=ACC_TELNET_PORT;
accEntry->web_port=ACC_HTTP_PORT;
accEntry->ftp_port=ACC_FTP_PORT;
}
void copyPvcEntryToAccEntry(MIB_CE_ACC_T *accEntry,MIB_CE_ATM_VC_T *pvcEntry)
{
accEntry->vpi=pvcEntry->vpi;
accEntry->vci=pvcEntry->vci;
accEntry->protocol=pvcEntry->cmode;
accEntry->ifIndex=pvcEntry->ifIndex;
return;
}
#endif
void formAdsl(webs_t wp, char_t *path, char_t *query)
{
	char_t *strSubmit, *strValue, *strMode;
	char_t *strConn, *strDisconn;
	char_t *submitUrl;
	char tmpBuf[100];
	int dns_changed=0;
	unsigned int vUInt;
	unsigned int totalEntry;
	MIB_CE_ATM_VC_T Entry;
	int i, selected;
	int havePPP=0;
	char ifname[6];
	char buff[256];
	unsigned int ifMap;	// high half for PPP bitmap, low half for vc bitmap
	struct data_to_pass_st msg;
	char qosParms[32];
	int drflag=0;	// Jenny, check if default route exists
	char_t disabled;	// for auto-pvc-search	
	
	
	i = checkAction(wp, tmpBuf);
	if (i == 1)	// do action
		return;
	else if (i == -1) // error
		goto setErr_filter;
	// else no action (i == 0) -> continue ...
	
	totalEntry = mib_chain_total(MIB_ATM_VC_TBL); /* get chain record size */
	
	strSubmit = websGetVar(wp, T("delvc"), T(""));

	// Delete
	if (strSubmit[0]) {
		unsigned int i;
		unsigned int idx;
		
		// Mason Yu 
		deleteAllConnection();
		strValue = websGetVar(wp, T("select"), T(""));
		
		if (strValue[0]) {
			for (i=0; i<totalEntry; i++) {
				idx = totalEntry-i-1;			
				snprintf(tmpBuf, 4, "s%d", idx);
				
				if ( !gstrcmp(strValue, T(tmpBuf)) )
				{
					resolveServiceDependency(idx);
//add by ramen to delete the services access control of each pvc
#ifdef ZTE_GENERAL_ROUTER_SC
MIB_CE_ATM_VC_T tempPvcEntry;
if(mib_chain_get(MIB_ATM_VC_TBL,idx,&tempPvcEntry)&&(tempPvcEntry.cmode!=ADSL_BR1483))
	{
	filter_set_remote_access(0);
	mib_chain_delete(MIB_ACC_TBL,idx);
	filter_set_remote_access(1);
}else mib_chain_delete(MIB_ACC_TBL,idx);

#endif
//add by ramen to check whether the deleted pvc bind a dns!
#ifdef DNS_BIND_PVC_SUPPORT
MIB_CE_ATM_VC_T dnsPvcEntry;
if(mib_chain_get(MIB_ATM_VC_TBL,idx,&dnsPvcEntry)&&(dnsPvcEntry.cmode!=ADSL_BR1483))
{
      int tempi=0;
	unsigned char pvcifIdx=0;
	for(tempi=0;tempi<3;tempi++)
		{
		    mib_get(MIB_DNS_BIND_PVC1+tempi,(void*)&pvcifIdx);
		    if(pvcifIdx==dnsPvcEntry.ifIndex)//I get it
		    	{
		    		pvcifIdx=255;
				mib_set(MIB_DNS_BIND_PVC1+tempi,(void*)&pvcifIdx);
		    	}	
		}	
}
#endif
#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
					{
						MIB_CE_ATM_VC_T cwmp_entry;
						if (mib_chain_get(MIB_ATM_VC_TBL, idx, (void *)&cwmp_entry))
						{
							delPortForwarding( cwmp_entry.ifIndex );
							delRoutingTable( cwmp_entry.ifIndex );
						}
					}
#endif
/*ql:20080926 START: delete MIB_DHCP_CLIENT_OPTION_TBL entry to this pvc*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
					{
						MIB_CE_ATM_VC_T dhcp_entry;
						if (mib_chain_get(MIB_ATM_VC_TBL, idx, (void *)&dhcp_entry))
						{
							if ((dhcp_entry.cmode == ADSL_MER1483) && (dhcp_entry.ipDhcp == DHCP_CLIENT))
								delDhcpcOption(dhcp_entry.ifIndex);
						}
					}
#endif
/*ql:20080926 END*/
#ifdef NEW_IP_QOS_SUPPORT//ql 20081125
					{
						MIB_CE_ATM_VC_T vcEntry;
						if (mib_chain_get(MIB_ATM_VC_TBL, idx, (void *)&vcEntry))
						{
							delIpQosTcRule(&vcEntry);
						}
					}
#endif
					if(mib_chain_delete(MIB_ATM_VC_TBL, idx) != 1) {
						strcpy(tmpBuf, T(strDelChainerror));
						goto setErr_filter;
					}
					break;
				}
			}
			//ql add: check if it is necessary to delete a group of interface
#ifdef CONFIG_EXT_SWITCH
#ifdef ITF_GROUP
			{
				int wanPortNum;
				unsigned int swNum, vcNum;
				MIB_CE_SW_PORT_T Entry;
				MIB_CE_ATM_VC_T pvcEntry;
				int j, grpnum;
				char mygroup;

				vcNum = mib_chain_total(MIB_ATM_VC_TBL);
				for (grpnum=1; grpnum<=4; grpnum++) {
					wanPortNum = 0;
					
					for (j=0; j<vcNum; j++) {
						if (!mib_chain_get(MIB_ATM_VC_TBL, j, (void *)&pvcEntry))
						{
				  			//websError(wp, 400, T("Get chain record error!\n"));
				  			//printf("Get chain record error!\n");
							//return -1;
							strcpy(tmpBuf, T(strGetChainerror));
							goto setErr_filter;
						}
						
						if (pvcEntry.enable == 0 || pvcEntry.itfGroup!=grpnum)
							continue;

						wanPortNum++;
					}

					if (0 == wanPortNum) {
						printf("delete port mapping group %d\n", grpnum);
						//release LAN ports
						swNum = mib_chain_total(MIB_SW_PORT_TBL);
						for (j=swNum; j>0; j--) {
							if (!mib_chain_get(MIB_SW_PORT_TBL, j-1, (void *)&Entry)) {
								//return -1;
								strcpy(tmpBuf, T(strGetChainerror));
								goto setErr_filter;
							}
							if (Entry.itfGroup == grpnum) {
								Entry.itfGroup = 0;
								mib_chain_update(MIB_SW_PORT_TBL, (void *)&Entry, j-1);
							}
						}
#ifdef WLAN_SUPPORT
						//release wlan0
						mib_get(MIB_WLAN_ITF_GROUP, (void *)&mygroup);
						if (mygroup == grpnum) {
							mygroup = 0;
							mib_set(MIB_WLAN_ITF_GROUP, (void *)&mygroup);
						}
#endif
#ifdef ZTE_GENERAL_ROUTER_SC
#ifdef WLAN_MBSSID
						//release MBSSID
						for (j=1; j<5; j++) {
							mib_get(MIB_WLAN_VAP0_ITF_GROUP+j-1, (void *)&mygroup);
							if (mygroup == grpnum) {
								mygroup = 0;
								mib_set(MIB_WLAN_VAP0_ITF_GROUP+j-1, (void *)&mygroup);
							}
						}
#endif
#endif
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
						setgroup("", grpnum, lowPrio);
#else
						setgroup("", grpnum);
#endif
						break;
					}
				}
			}//end
#endif	// of ITF_GROUP
#endif
		}
		else
		{
			strcpy(tmpBuf, T(strSelectvc));
			goto setErr_filter;
		}
	
		goto setOk_filter;
	}
	
	strSubmit = websGetVar(wp, T("modify"), T(""));

	// Modify
	if (strSubmit[0]) {
		MIB_CE_ATM_VC_T entry, myEntry;		
		int cnt=0, pIdx;	
		
		// Mason Yu 
		deleteAllConnection();
		strValue = websGetVar(wp, T("select"), T(""));
		selected = -1;
		ifMap = 0;

		if (retrieveVcRecord(wp, &entry) != 0) {
			/*RegisterPVCnumber();		
			cleanAllFirewallRule();
			restart_dnsrelay();
			startWan(BOOT_LAST);*/
			restartWAN();
			return;
		}
		for (i=0; i<totalEntry; i++) {
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			{
	  			//websError(wp, 400, T(strGetChainerror));
				//return;
				strcpy(tmpBuf, T(strGetChainerror));
				goto setErr_filter;
			}
			if (Entry.vpi == entry.vpi && Entry.vci == entry.vci ) 
				cnt++;
			snprintf(tmpBuf, 4, "s%d", i);
			if ( (selected == -1) && !gstrcmp(strValue, T(tmpBuf)) )
				selected = i;
			else
			{
				ifMap |= 1 << (Entry.ifIndex & 0x0f);	// vc map
				ifMap |= (1 << 16) << ((Entry.ifIndex >> 4) & 0x0f);	// PPP map
			}

#ifdef DEFAULT_GATEWAY_V1
			if (Entry.cmode != ADSL_BR1483)
				if (Entry.dgw)
					drflag = 1;
#endif
		}
		
		if (selected == -1)
		{
			strcpy(tmpBuf, T(strSelectvc));
			goto setErr_filter;
		}

		if(cnt > 0) {
			//Make sure there is no mismatch mode
			for (i=0, cnt=0; i<totalEntry; i++) {
				if(i==selected)
					continue;
				if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)){
	  				//websError(wp, 400, T(strGetChainerror));
					//return;
					strcpy(tmpBuf, T(strGetChainerror));
					goto setErr_filter;
				}
				if (Entry.vpi == entry.vpi && Entry.vci == entry.vci){
					cnt++;
					if(Entry.cmode != entry.cmode){
						strcpy(tmpBuf, T(strConnectExist));
						goto setErr_filter;											
					}
					if (entry.cmode == ADSL_PPPoE)		// Jenny, for multisession PPPoE support
						pIdx = i;
				}
			}
			//Max. 2 PPPoE connections
//			if(entry.cmode == ADSL_PPPoE && cnt==2) {
			if(entry.cmode == ADSL_PPPoE && cnt==MAX_POE_PER_VC) {	// Jenny, multisession PPPoE support
				strcpy(tmpBuf, T(strMaxNumPPPoE));
				goto setErr_filter;		
			//Max. 1 connection except PPPoE
			} else if(entry.cmode != ADSL_PPPoE&& cnt>0 ) {
				strcpy(tmpBuf, T(strConnectExist));
				goto setErr_filter;
			}
			if (entry.cmode == ADSL_PPPoE && cnt>0)		// Jenny, for multisession PPPoE, get existed PPPoE config for further ifindex use
				if (!mib_chain_get(MIB_ATM_VC_TBL, pIdx, (void *)&myEntry)) {
					strcpy(tmpBuf, errGetEntry);
					goto setErr_filter;
				}
		}
		if (!mib_chain_get(MIB_ATM_VC_TBL, selected, (void *)&Entry)) {
			strcpy(tmpBuf, errGetEntry);
			goto setErr_filter;
		}

//		if (retrieveVcRecord(wp, &entry) != 0)
//			return;
		
		// Added by Mason Yu
#if 0		
		if ( (Entry.vpi != entry.vpi) || (Entry.vci != entry.vci) ) {
			strcpy(tmpBuf, T("Can not modify VPI and VCI value!"));
			goto setErr_filter;
		}
#endif		
		// restore stuff not posted in this form
		if (entry.cmode == ADSL_PPPoE)
			if (cnt > 0) {		// Jenny, for multisession PPPoE, ifIndex(VC device) must refer to existed PPPoE connection
				ifMap &= 0xffff0000; // don't care the vc part
				entry.ifIndex = if_find_index(entry.cmode, ifMap);
				entry.ifIndex &= 0xf0;
				entry.ifIndex |= (myEntry.ifIndex&0x0f);
			}
			else {
				entry.ifIndex = if_find_index(entry.cmode, ifMap);
#ifdef PPPOE_PASSTHROUGH
				if (entry.cmode == Entry.cmode)
					entry.brmode = Entry.brmode;
#endif
			}
		else
			entry.ifIndex = Entry.ifIndex;

		entry.qos = Entry.qos;
		entry.pcr = Entry.pcr;
		entry.scr = Entry.scr;
		entry.mbs = Entry.mbs;
		entry.cdvt = Entry.cdvt;
		entry.pppAuth = Entry.pppAuth;
		entry.rip = Entry.rip;
//		entry.dgw = Entry.dgw;
		entry.mtu = Entry.mtu;
#ifdef CONFIG_EXT_SWITCH
		//ql: when pvc is modified, interface group don't changed???
		entry.itfGroup = Entry.itfGroup;
#endif

#ifdef CONFIG_SPPPD_STATICIP
                if(entry.cmode == ADSL_PPPoE)
		  {
		    entry.pppIp = Entry.pppIp;
		    strcpy( entry.ipAddr, Entry.ipAddr);
                  }
#endif
#ifdef PPPOE_PASSTHROUGH
		if (entry.cmode != ADSL_PPPoE)
			if (entry.cmode == Entry.cmode)
				entry.brmode = Entry.brmode;
#endif
#ifdef CONFIG_EXT_SWITCH
		// VLAN
		entry.vlan = Entry.vlan;
		entry.vid = Entry.vid;
		entry.vprio = Entry.vprio;
		entry.vpass = Entry.vpass;
#endif
#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
		entry.connDisable = 0;
		if (Entry.vpi == entry.vpi && Entry.vci == entry.vci)
		{
			entry.ConDevInstNum = Entry.ConDevInstNum;
			if( (entry.cmode==ADSL_PPPoE) || 
#ifdef PPPOE_PASSTHROUGH
			    ((entry.cmode==ADSL_BR1483)&&(entry.brmode==BRIDGE_PPPOE)) ||
#endif
			    (entry.cmode==ADSL_PPPoA) )
			{
				if( Entry.ConPPPInstNum!=0 )
					entry.ConPPPInstNum = Entry.ConPPPInstNum;
				else
					entry.ConPPPInstNum = 1 + findMaxPPPConInstNum( entry.ConDevInstNum );
				entry.ConIPInstNum = 0;
			}else{
				entry.ConPPPInstNum = 0;
				if(Entry.ConIPInstNum!=0)
					entry.ConIPInstNum = Entry.ConIPInstNum;
				else
					entry.ConIPInstNum = 1 + findMaxIPConInstNum( entry.ConDevInstNum );
			}
		}else{
			unsigned int  instnum=0;
			instnum = findConDevInstNumByPVC( entry.vpi, entry.vci );
			if(instnum==0)
				instnum = 1 + findMaxConDevInstNum();
			entry.ConDevInstNum = instnum;
			if( (entry.cmode==ADSL_PPPoE) || 
#ifdef PPPOE_PASSTHROUGH
			    ((entry.cmode==ADSL_BR1483)&&(entry.brmode==BRIDGE_PPPOE)) ||
#endif
			    (entry.cmode==ADSL_PPPoA) )
			{
				entry.ConPPPInstNum = 1 + findMaxPPPConInstNum( entry.ConDevInstNum );
				entry.ConIPInstNum = 0;
			}else{
				entry.ConPPPInstNum = 0;
				entry.ConIPInstNum = 1 + findMaxIPConInstNum( entry.ConDevInstNum );
			}
		}
		//fprintf( stderr, "<%s:%d>NewInstNum=>ConDev:%u, PPPCon:%u, IPCon:%u\n", __FILE__, __LINE__, entry.ConDevInstNum, entry.ConPPPInstNum, entry.ConIPInstNum );

		entry.autoDisTime = Entry.autoDisTime;
		entry.warnDisDelay = Entry.warnDisDelay;
		strcpy( entry.pppServiceName, Entry.pppServiceName );
		strcpy( entry.WanName, Entry.WanName );
#ifdef _PRMT_TR143_
		entry.TR143UDPEchoItf = Entry.TR143UDPEchoItf;
#endif //_PRMT_TR143_
#ifdef _PRMT_X_CT_COM_PPPOEv2_
		entry.PPPoEProxyEnable = Entry.PPPoEProxyEnable;
		entry.PPPoEProxyMaxUser = Entry.PPPoEProxyMaxUser;
#endif //_PRMT_X_CT_COM_PPPOEv2_
#ifdef _PRMT_X_CT_COM_WANEXT_
		entry.ServiceList = Entry.ServiceList;
#endif //_PRMT_X_CT_COM_WANEXT_
#endif //_CWMP_MIB_
		strcpy(entry.pppACName, Entry.pppACName);
		
		// disable this interface
		if (entry.enable == 0 && Entry.enable == 1)
			resolveServiceDependency(selected);
		
#ifdef DEFAULT_GATEWAY_V1
		// set default route flag
		if (entry.cmode != ADSL_BR1483)
			if (!entry.dgw && Entry.dgw)
				drflag =0;
		else
			if (Entry.dgw)
				drflag = 0;
#endif

		// find the ifIndex
		if (entry.cmode != Entry.cmode)
		{
			if (!(entry.cmode == ADSL_PPPoE && cnt>0))	// Jenny, entries except multisession PPPoE
				entry.ifIndex = if_find_index(entry.cmode, ifMap);
			if (entry.ifIndex == 0xff)
			{
				strcpy(tmpBuf, T(strMaxVc));
				goto setErr_filter;
			}
			else if (entry.ifIndex == 0xef)
			{
				strcpy(tmpBuf, T(strMaxNumPPPoE));
				goto setErr_filter;
			}
			
			// mode changed, restore to default
			if (entry.cmode == ADSL_PPPoE) {
				entry.mtu = 1452;
#ifdef CONFIG_USER_PPPOE_PROXY
				entry.PPPoEProxyMaxUser=4;
#endif
				entry.pppAuth = 0;
			}
			else {
#ifdef CONFIG_USER_PPPOE_PROXY
				entry.PPPoEProxyMaxUser=0;
#endif
//				entry.dgw = 1;
				entry.mtu = 1500;
			}
			
//			entry.dgw = 1;
#ifdef CONFIG_EXT_SWITCH
			// VLAN
			entry.vlan = 0;	// disable
			entry.vid = 0; // VLAN tag
			entry.vprio = 0; // priority bits (0 ~ 7)
			entry.vpass = 0; // no pass-through
#endif
		}
		
#ifdef DEFAULT_GATEWAY_V1
		if (entry.cmode != ADSL_BR1483)
		{
			if (drflag && entry.dgw && !Entry.dgw)
			{
				strcpy(tmpBuf, T(strDrouteExist));
				goto setErr_filter;
			}
			if (entry.dgw && !Entry.dgw)
				drflag = 1;
		}
#endif

		/*ql:20080926 START: delete MIB_DHCP_CLIENT_OPTION_TBL entry to this pvc*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
		if ((Entry.cmode == ADSL_MER1483) && (Entry.ipDhcp == DHCP_CLIENT))
			delDhcpcOption(Entry.ifIndex);
#endif
		/*ql:20080926 END*/

		if( entry.ifIndex!=Entry.ifIndex )
		{
			updatePortForwarding( Entry.ifIndex, entry.ifIndex );
			updateRoutingTable( Entry.ifIndex, entry.ifIndex );
			//ql 20081125 add
#ifdef NEW_IP_QOS_SUPPORT
			delIpQosTcRule(&Entry);
#endif
		}
#ifdef NEW_IP_QOS_SUPPORT
		//ql 20081125 Add else
		else if (entry.cmode != Entry.cmode)
		{
			delIpQosTcRule(&Entry);
		}
#endif

#ifdef CONFIG_ATM_CLIP
		if (entry.cmode == ADSL_RT1577)
			entry.encap = 1;	// LLC
#endif

#if 0
#ifdef APPLY_CHANGE		
		stopConnection(&Entry);
		startConnection(&entry);
#endif
#endif
		// log message
//add by ramen for ZTE ROTUER rmt acc on pvc
#ifdef ZTE_GENERAL_ROUTER_SC
		if(Entry.cmode!=entry.cmode) filter_set_remote_access(0);
		MIB_CE_ACC_T accEntry;	
		if(!mib_chain_get(MIB_ACC_TBL,selected,&accEntry))
		{
			  printf("get MIB_ACC_TBL error!\n");
			  return;
		}
		copyPvcEntryToAccEntry(&accEntry,&entry);

		 if(Entry.cmode!=entry.cmode)
	 	{
	 	        assignDefaultValue(&accEntry);
		        mib_chain_update(MIB_ACC_TBL,&accEntry,selected);  
	 		  filter_set_remote_access(1);
	 	}
		else 
			mib_chain_update(MIB_ACC_TBL,&accEntry,selected);  
#endif
//add by ramen for DNS bind pvc
#ifdef DNS_BIND_PVC_SUPPORT
MIB_CE_ATM_VC_T dnsPvcEntry;
if(mib_chain_get(MIB_ATM_VC_TBL,selected,&dnsPvcEntry)&&(dnsPvcEntry.cmode!=ADSL_BR1483))
{
      int tempi=0;
	unsigned char pvcifIdx=0;
	for(tempi=0;tempi<3;tempi++)
		{
		    mib_get(MIB_DNS_BIND_PVC1+tempi,(void*)&pvcifIdx);
		    if(pvcifIdx==dnsPvcEntry.ifIndex)//I get it
		    	{
		    	  if(entry.cmode==ADSL_BR1483)	 		pvcifIdx=255;				
		    	  	else    	  	pvcifIdx=entry.ifIndex;		    	  		
				mib_set(MIB_DNS_BIND_PVC1+tempi,(void*)&pvcifIdx);
		    	}			
		}
	
}
#endif
        //jim garbage action...
		//memcpy(&Entry, &entry, sizeof(entry));
		// log message
		mib_chain_update(MIB_ATM_VC_TBL, (void *)&entry, selected);
#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
		restart_dnsrelay(); //star
#endif
        //jim moved here from line 2055 for do connection after mib updated...
#ifdef ZTE_531B_BRIDGE_SC
		unsigned char vChar;
#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		/*to avoid the dead when wrriting the flash*/
		itfcfg("sar", 0);
		itfcfg("eth0", 0);
#ifdef WLAN_SUPPORT
		itfcfg("wlan0", 0);
#endif
#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
		mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);

#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		itfcfg("sar", 1);
		 itfcfg("eth0", 1);
#ifdef WLAN_SUPPORT
		vChar=0;
		mib_get( MIB_WLAN_DISABLED, (void *)&vChar);
		if( vChar==0 )	itfcfg("wlan0", 1);
#endif //WLAN_SUPPORT
#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
#endif
		goto setOk_filter;		
		
		
	}

	strSubmit = websGetVar(wp, T("add"), T(""));

	// Add
	if (strSubmit[0]) {
		MIB_CE_ATM_VC_T entry;
//		MIB_CE_ATM_VC_Tp pmyEntry;
		int cnt, pIdx, intVal;
		unsigned char vcIdx;
		
		// Mason Yu 
		deleteAllConnection();
		if (totalEntry >= MAX_VC_NUM)
		{
			strcpy(tmpBuf, T(strMaxVc));
			goto setErr_filter;
		}
		
		if (retrieveVcRecord(wp, &entry) != 0) {
			/*RegisterPVCnumber();		
			cleanAllFirewallRule();
			restart_dnsrelay(); //Jenny
			startWan(BOOT_LAST);*/
			restartWAN();
			return;
		}
		
		// check if connection exists
		ifMap = 0;
		cnt=0;
		
		for (i=0; i<totalEntry; i++) {
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			{
	  			//websError(wp, 400, T(strGetChainerror));
				//return;
				strcpy(tmpBuf, T(strGetChainerror));
				goto setErr_filter;
			}
			
			if (Entry.vpi == entry.vpi && Entry.vci == entry.vci)
			{
				cnt++;
				pIdx = i;	// Jenny, for multisession PPPoE, record entry index instead of atmvc entry pointer
//				pmyEntry = &Entry;
			}
			
#ifdef DEFAULT_GATEWAY_V1
			if (Entry.cmode != ADSL_BR1483)
				if (Entry.dgw)
					drflag = 1;
#endif

			ifMap |= 1 << (Entry.ifIndex & 0x0f);	// vc map
			ifMap |= (1 << 16) << ((Entry.ifIndex >> 4) & 0x0f);	// PPP map
		}

		if (cnt == 0)	// pvc not exists
		{
			entry.ifIndex = if_find_index(entry.cmode, ifMap);
			if (entry.ifIndex == 0xff)
			{
				strcpy(tmpBuf, T(strMaxVc));
				goto setErr_filter;
			}
			else if (entry.ifIndex == 0xef)
			{
				strcpy(tmpBuf, T(strMaxNumPPPoE));
				goto setErr_filter;
			}

#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
			entry.ConDevInstNum = 1 + findMaxConDevInstNum();
			if( (entry.cmode==ADSL_PPPoE) || 
#ifdef PPPOE_PASSTHROUGH
			    ((entry.cmode==ADSL_BR1483)&&(entry.brmode==BRIDGE_PPPOE)) ||
#endif
			    (entry.cmode==ADSL_PPPoA) )
				entry.ConPPPInstNum = 1;
			else
				entry.ConIPInstNum = 1;
			//fprintf( stderr, "<%s:%d>NewInstNum=>ConDev:%u, PPPCon:%u, IPCon:%u\n", __FILE__, __LINE__, entry.ConDevInstNum, entry.ConPPPInstNum, entry.ConIPInstNum );
#endif
		}
		else	// pvc exists
		{
			if (!mib_chain_get(MIB_ATM_VC_TBL, pIdx, (void *)&Entry)) {	// Jenny, for multisession PPPoE, get existed pvc config
				strcpy(tmpBuf, errGetEntry);
				goto setErr_filter;
			}
			if (Entry.cmode == ADSL_PPPoE && entry.cmode == ADSL_PPPoE)
			{
				if (cnt<MAX_POE_PER_VC)
				{	// get the pvc info.
					entry.qos = Entry.qos;
					entry.pcr = Entry.pcr;
					entry.encap = Entry.encap;
					ifMap &= 0xffff0000; // don't care the vc part
					entry.ifIndex = if_find_index(entry.cmode, ifMap);
					if (entry.ifIndex == 0xef)
					{
						strcpy(tmpBuf, T(strMaxNumPPPoE));
						goto setErr_filter;
					}
					entry.ifIndex &= 0xf0;
					entry.ifIndex |= (Entry.ifIndex&0x0f);
#ifdef PPPOE_PASSTHROUGH
					entry.brmode = Entry.brmode;	// Jenny, for multisession PPPoE
#endif

#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
					entry.ConDevInstNum = Entry.ConDevInstNum;
					entry.ConPPPInstNum = 1 + findMaxPPPConInstNum( Entry.ConDevInstNum );
					//fprintf( stderr, "<%s:%d>NewInstNum=>ConDev:%u, PPPCon:%u, IPCon:%u\n", __FILE__, __LINE__, entry.ConDevInstNum, entry.ConPPPInstNum, entry.ConIPInstNum );
#endif
				}
				else
				{
					strcpy(tmpBuf, T(strMaxNumPPPoE));
					goto setErr_filter;
				}
			}
			else
			{
				strcpy(tmpBuf, T(strConnectExist));
				goto setErr_filter;
			}
		}
		
#ifdef DEFAULT_GATEWAY_V1
		if (entry.cmode != ADSL_BR1483)
			if (drflag && entry.dgw)
			{
				strcpy(tmpBuf, T(strDrouteExist));
				goto setErr_filter;
			}
#endif

#ifdef CONFIG_ATM_CLIP
		if (entry.cmode == ADSL_RT1577)
			entry.encap = 1;	// LLC
#endif

		// set default
//		entry.dgw = 1;
		if (entry.cmode == ADSL_PPPoE)
			{
			entry.mtu = 1452;
#ifdef CONFIG_USER_PPPOE_PROXY
			entry.PPPoEProxyMaxUser=4;
			entry.PPPoEProxyEnable=0;
#endif
			}
		else
			entry.mtu = 1500;
		
#ifdef CONFIG_EXT_SWITCH
		// VLAN
		entry.vlan = 0;	// disable
		entry.vid = 0; // VLAN tag
		entry.vprio = 0; // priority bits (0 ~ 7)
		entry.vpass = 0; // no pass-through
#endif
//add by ramen for ZTE Router rmt acc on pvc
#ifdef ZTE_GENERAL_ROUTER_SC
MIB_CE_ACC_T accEntry;		
assignDefaultValue(&accEntry);
copyPvcEntryToAccEntry(&accEntry,&entry);
		intVal = mib_chain_add(MIB_ACC_TBL, (unsigned char*)&accEntry);
		if (intVal == 0) {
			strcpy(tmpBuf, T(strAddChainerror));
			goto setErr_filter;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, T(strTableFull));
			goto setErr_filter;
		}

#endif
		intVal = mib_chain_add(MIB_ATM_VC_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			strcpy(tmpBuf, T(strAddChainerror));
			goto setErr_filter;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, T(strTableFull));
			goto setErr_filter;
		}
#if 0
#ifdef APPLY_CHANGE		
		startConnection(&entry);		
#endif
#endif			
		
		goto setOk_filter;
	}

#if 0
	strConn = websGetVar(wp, T("pppConnect"), T(""));
	strDisconn = websGetVar(wp, T("pppDisconnect"), T(""));
	
	if (strConn[0] || strDisconn[0]) {
		// check for PPP connection
		selected = -1;
		strValue = websGetVar(wp, T("select"), T(""));
		if (strValue[0]) {
			for (i=0; i<totalEntry; i++) {
				snprintf(tmpBuf, 4, "s%d", totalEntry-i-1);
				
				if ( !gstrcmp(strValue, T(tmpBuf)) )
				{
					selected = totalEntry-i-1;
					break;
				}
			}
		}
		if (selected == -1)
			goto setOk_filter;
		pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, selected); /* get the specified chain record */
		if (pEntry == NULL)
			goto setOk_filter;
		// check if this action allowed
		if ((pEntry->cmode != ACC_PPPOE && pEntry->cmode != ACC_PPPOA) || pEntry->pppCtype != MANUAL) {
			printf("not allowed for this connection !\n");
			goto setOk_filter;
		}
		
		snprintf(ifname, 6, "ppp%u", PPP_INDEX(pEntry->ifIndex));
		havePPP = find_ppp_from_conf(ifname);
	}
	
	// PPP Connect
	if (strConn[0]) {
		if (!havePPP) {
			// create a PPP connection
			printf("create %s here\n", ifname);
			if (pEntry->cmode == ACC_PPPOE)	// PPPoE
			{
				// spppctl add 0 pppoe vc0 username USER password PASSWORD gw 1
				snprintf(msg.data, BUF_SIZE,
					"spppctl add %u pppoe vc%u username %s password %s gw 1",
					PPP_INDEX(pEntry->ifIndex), VC_INDEX(pEntry->ifIndex),
					pEntry->pppUsername, pEntry->pppPassword);
			}
			else	// PPPoA
			{
				if ((ATM_QOS_T)pEntry->qos == ATMQOS_UBR)
				{
					snprintf(qosParms, 32, "ubr:pcr=%u", pEntry->pcr);
				}
				else if ((ATM_QOS_T)pEntry->qos == ATMQOS_UBR)
				{
					snprintf(qosParms, 32, "cbr:pcr=%u", pEntry->pcr);
				}
				else	// rt-vbr or nrt-vbr
				{
					snprintf(qosParms, 32, "vbr:pcr=%u,scr=%u,mbs=%u",
						pEntry->pcr, pEntry->scr, pEntry->mbs);
				}
				
				snprintf(msg.data, BUF_SIZE,
					"spppctl add %u pppoa %u.%u encaps %d qos %s "
					"username %s password %s gw 1",
					PPP_INDEX(pEntry->ifIndex), pEntry->vpi, pEntry->vci, pEntry->encap,
					qosParms, pEntry->pppUsername, pEntry->pppPassword);
			}
			TRACE(STA_SCRIPT, "%s\n", msg.data);
			write_to_pppd(&msg);
		}
		else
			printf("%s already exists\n", ifname);
		
		goto setOk_filter;
	}
	
	// PPP DisConnect
	if (strDisconn[0]) {
		if (havePPP) {
			// delete the PPP connection
			printf("delete %s here\n", ifname);
			// spppctl del 0
			snprintf(msg.data, BUF_SIZE,
				"spppctl del %u", PPP_INDEX(pEntry->ifIndex));
			TRACE(STA_SCRIPT, "%s\n", msg.data);
			write_to_pppd(&msg);
		}
		else
			printf("%s not exists\n", ifname);
		
		goto setOk_filter;
	}
#endif

	// auto-pvc-search Apply
	strSubmit = websGetVar(wp, T("enablepvc"), T(""));
	if ( strSubmit[0] ) {		
		//printf("formAdsl: got disabled %x %s\n", atoi(strSubmit), strSubmit);		
		update_auto_pvc_disable(atoi(strSubmit));		
	}

	//auto-pvc-search add PVC	
	strSubmit = websGetVar(wp, T("autopvcadd"), T(""));
	if ( strSubmit[0] ) {	
		//printf("formAdsl(autopvcadd): got strSubmit %s\n", strSubmit);
		if(add_auto_pvc_search_pair(wp, 1) != 0)
			return;		
	}
	//auto-pvc-search delete PVC
	strSubmit = websGetVar(wp, T("autopvcdel"), T(""));
	if ( strSubmit[0] ) {	
		//printf("formAdsl(autopvcdel): got strSubmit %s\n", strSubmit);
		if(add_auto_pvc_search_pair(wp, 0) != 0)
			return;		
	}

	strSubmit = websGetVar(wp, T("refresh"), T(""));
	// Refresh
	if (strSubmit[0]) {
		//goto setOk_filter;
		goto setOk_nofilter;
	}
	goto setOk_nofilter;
#ifdef ZTE_531B_BRIDGE_SC
	strSubmit = websGetVar(wp, T("reboot"), T(""));
	// Refresh
	if (strSubmit[0]) {
		goto setReboot_filter;
	}
#endif

setOk_filter:

#ifdef CONFIG_GUI_WEB
	writeFlash();
#endif
	// Added by Mason Yu. for take effect in real time	
	/*RegisterPVCnumber();		
	cleanAllFirewallRule();
	restart_dnsrelay(); //Jenny
	startWan(BOOT_LAST);*/
	restartWAN();
		
	//ql 20081118 START restart IP QoS
#ifdef NEW_IP_QOS_SUPPORT
	take_qos_effect();
#endif
	
//	mib_update(CURRENT_SETTING);

setOk_nofilter:

#ifndef NO_ACTION
	pid = fork();
        if (pid) {
	      	waitpid(pid, NULL, 0);
	}
        else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _FIREWALL_SCRIPT_PROG);
		execl( tmpBuf, _FIREWALL_SCRIPT_PROG, NULL);
               	exit(1);
        }
#endif

	submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page

	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);

  	return;
	
#ifdef ZTE_531B_BRIDGE_SC
setReboot_filter:
	submitUrl = websGetVar(wp, T("reboot-url"), T(""));   // hidden page

	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);

  	return;
#endif
  	
setErr_filter:
	ERR_MSG(tmpBuf);
#ifdef CONFIG_GUI_WEB
	writeFlash();
#endif
	restartWAN();
	/*RegisterPVCnumber();		
	cleanAllFirewallRule();
	restart_dnsrelay(); //Jenny
	startWan(BOOT_LAST);*/
}

/////////////////////////////////////////////////////////////////////////////
///ql_xu add for fast configure.
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
MIB_CE_ATM_VC_T fcEntry;
//unsigned char lanIp2[IP_ADDR_LEN];
//unsigned char subnetMask2[IP_ADDR_LEN];
//int encap=0;
//char enable_ip2=0;
TMP_FC_T fcEntry2;
void formFastConf(webs_t wp, char_t *path, char_t *query)
{
	char_t *strVal, *submitUrl;
	char tmpBuf[100];
	unsigned int vUInt;

	//memset(&fcEntry, 0x00, sizeof(fcEntry));
	strVal = websGetVar(wp, T("vpi"), T(""));
	if (!strVal[0]) {
		strcpy(tmpBuf, T(Tenter_vpi));
		goto setErr_filter;
	}

	sscanf(strVal, "%u", &vUInt);
	if (vUInt > 255) {
		strcpy(tmpBuf, T(Tinvalid_vpi));
		goto setErr_filter;
	}
	fcEntry.vpi = (unsigned char)vUInt;

	strVal = websGetVar(wp, T("vci"), T(""));
	if (!strVal[0]) {
		strcpy(tmpBuf, T(Tenter_vci));
		goto setErr_filter;
	}

	sscanf(strVal,"%u",&vUInt);
	if ( vUInt>65535 || vUInt<32) {
		strcpy(tmpBuf, T(Tinvalid_vci));
		goto setErr_filter;
	}
	fcEntry.vci = (unsigned short)vUInt;

	submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
  	return;
  	
setErr_filter:
	ERR_MSG(tmpBuf);
}

void formFastConf2(webs_t wp, char_t *path, char_t *query)
{
	char_t *strVal, *submitUrl;
	char_t *strSubmit;

	/////
	fcEntry2.encap_tmp = 1;
	strSubmit = websGetVar(wp, T("laststep"), T(""));
	if (strSubmit[0]) {
		goto jumpLastPage;
	}
	
	strVal = websGetVar(wp, T("wantype"), T(""));
	fcEntry.cmode = strVal[0]-'0';

	strVal = websGetVar(wp, T("adslEncap"), T(""));
	fcEntry.encap = strVal[0]-'0';

	if (fcEntry.cmode == ADSL_BR1483)
		submitUrl = websGetVar(wp, T("submit-url-bridge"), T(""));   // hidden page
	else if (fcEntry.cmode == ADSL_PPPoE ||fcEntry.cmode == ADSL_PPPoA)
		submitUrl = websGetVar(wp, T("submit-url-ppp"), T(""));   // hidden page
	else if (fcEntry.cmode == ADSL_MER1483)
		submitUrl = websGetVar(wp, T("submit-url-mer"), T(""));   // hidden page
	else if (fcEntry.cmode == ADSL_RT1483)
		submitUrl = websGetVar(wp, T("submit-url-ipoa"), T(""));   // hidden page
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
  	return;
jumpLastPage:
  	submitUrl = websGetVar(wp, T("submit-url-l"), T(""));
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
	return;
}

#define DAY2SECOND	24*60*60
#define HOUR2SECOND	60*60
#define MIN2SECOND	60
extern sem_t semSave;
void formFastConf4(webs_t wp, char_t *path, char_t *query)
{
	char_t *strSubmit, *submitUrl;
	char tmpBuf[100];
	unsigned int totalEntry;
	int i, cnt=0, pIdx=0/*, pLastIdx*/, selected;
	int pppConn=0;
	MIB_CE_ATM_VC_T Entry, myEntry;
	unsigned int ifMap=0;	// high half for PPP bitmap, low half for vc bitmap
#ifdef ZTE_GENERAL_ROUTER_SC
	int drflag=0;
#endif
	int IdxDel=0;
	int pppNum=0, intVal;

	strSubmit = websGetVar(wp, T("laststep"), T(""));
	if (strSubmit[0]) {
		goto jumpLastPage;
	}
	/*patch: if refresh the page after the system reboot successfully, fast config action will be executed again, while all params
	be cleared.*/
	if (fcEntry.vpi==0 && fcEntry.vci==0) {
		websRedirect(wp, "/admin/status_sc.asp");
		return;
	}

	//fcEntry.dgw = 1;
	// set default Qos
	fcEntry.qos = 0;
	fcEntry.pcr = ATM_MAX_US_PCR;
	fcEntry.enable = 1;
#ifdef CTC_WAN_NAME
	fcEntry.applicationtype = 0;
#endif
#ifdef PPPOE_PASSTHROUGH
	fcEntry.brmode = BRIDGE_DISABLE;
	
	// 1483 bridged
	if (fcEntry.cmode == ADSL_BR1483)
	{
		fcEntry.brmode = 0;
	}
#endif

	///
	totalEntry = mib_chain_total(MIB_ATM_VC_TBL);

	if (totalEntry >= 8)
		selected = 0;//modify vc 0
	else
		selected = -1;//add
	
	for (i=0; i<totalEntry; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) {
			websError(wp, 400, T(Tget_chain_err));
			return;
		}
		//if ( (selected == -1) && i==0 )
		//	selected = i;
		//else {
			//ifMap |= 1 << (Entry.ifIndex & 0x0f);	// vc map
			//ifMap |= (1 << 16) << ((Entry.ifIndex >> 4) & 0x0f);	// PPP map
		//}
		
		if (Entry.vpi == fcEntry.vpi && Entry.vci == fcEntry.vci/* && selected != i*/) {
			cnt++;
			IdxDel = i;

			if ((fcEntry.cmode == ADSL_PPPoE) && (fcEntry.cmode == Entry.cmode)) {
				pIdx = i;
				pppConn++;
			} else if ((fcEntry.cmode != ADSL_PPPoE) && (Entry.cmode == ADSL_PPPoE)) {
				//if only one pppoe, just modify it; if more than one, delete all.
				//and add the new pvc
				pppNum++;
				selected = -1;
			}
		}
#ifdef ZTE_GENERAL_ROUTER_SC
		if (Entry.cmode != ADSL_BR1483)
			if (Entry.dgw) {
				drflag = 1;
				//if default GW existed, then take out it
				////Entry.dgw = 0;
				//write back to the mib chain
				//mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, i);
			}

#endif
	}
#ifndef ZTE_GENERAL_ROUTER_SC
	if (cnt > 0) {
		/*//delete existed PVC
		if(mib_chain_delete(MIB_ATM_VC_TBL, IdxDel) != 1) {
			strcpy(tmpBuf, T(strDelChainerror));
			goto setErr_filter;
		}*/
		//modify existed PVC
		selected = IdxDel;
	}
#else
	if(cnt > 0) {
		if (pppConn == MAX_POE_PER_VC)
			selected = pIdx;
		
		//Make sure there is no mismatch mode
		for (i=0/*, cnt=0*/; i<totalEntry;) {
			if(i == selected) {
				i++;
				continue;
			}
			
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)){
	  			websError(wp, 400, T(Tget_chain_err));
				return;
			}

			if (Entry.vpi == fcEntry.vpi && Entry.vci == fcEntry.vci){
				//cnt++;
				if(Entry.cmode != fcEntry.cmode || fcEntry.cmode != ADSL_PPPoE) {
					//if current PVC is different from existed PPPoE PVC in channel mode, delete all existed PVC					
					if (pppNum >= 2) {
#ifdef ZTE_GENERAL_ROUTER_SC
						MIB_CE_ATM_VC_T tempPvcEntry;
						if(mib_chain_get(MIB_ATM_VC_TBL,i,&tempPvcEntry)&&(tempPvcEntry.cmode!=ADSL_BR1483))
						{
							filter_set_remote_access(0);
							mib_chain_delete(MIB_ACC_TBL,i);
							filter_set_remote_access(1);
						} else
							mib_chain_delete(MIB_ACC_TBL,i);
#endif
						{
							MIB_CE_ATM_VC_T cwmp_entry;
							if (mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&cwmp_entry))
							{
								delPortForwarding( cwmp_entry.ifIndex );
								delRoutingTable( cwmp_entry.ifIndex );
							}
						}

						if(mib_chain_delete(MIB_ATM_VC_TBL, i) != 1) {
							strcpy(tmpBuf, T(strDelChainerror));
							goto setErr_filter;
						}
						//cnt--;
						totalEntry--;
						continue;
					}
					selected = i;//modify i
				}
				else if (fcEntry.cmode == ADSL_PPPoE) {		// Jenny, for multisession PPPoE support
					//pLastIdx = pIdx;
					pIdx = i;
				}
			}
			
			ifMap |= 1 << (Entry.ifIndex & 0x0f);	// vc map
			ifMap |= (1 << 16) << ((Entry.ifIndex >> 4) & 0x0f);	// PPP map

			i++;
		}
		//Max. 5 PPPoE connections
		/*if(fcEntry.cmode == ADSL_PPPoE && pppConn==MAX_POE_PER_VC) {	// Jenny, multisession PPPoE support
#ifdef ZTE_GENERAL_ROUTER_SC
			MIB_CE_ATM_VC_T tempPvcEntry;
			if(mib_chain_get(MIB_ATM_VC_TBL,selected,&tempPvcEntry)&&(tempPvcEntry.cmode!=ADSL_BR1483))
			{
				filter_set_remote_access(0);
				mib_chain_delete(MIB_ACC_TBL,selected);
				filter_set_remote_access(1);
			}else
				mib_chain_delete(MIB_ACC_TBL,selected);
#endif
			{
				MIB_CE_ATM_VC_T cwmp_entry;
				if (mib_chain_get(MIB_ATM_VC_TBL, selected, (void *)&cwmp_entry))
				{
					delPortForwarding( cwmp_entry.ifIndex );
					delRoutingTable( cwmp_entry.ifIndex );
				}
			}*/
			/*if (!mib_chain_delete(MIB_ATM_VC_TBL, pIdx)) {
				strcpy(tmpBuf, T(strDelChainerror));
				goto setErr_filter;
			}
			pIdx = pLastIdx;*/
		//Max. 1 connection except PPPoE
		//} 
		if (fcEntry.cmode == ADSL_PPPoE && pppConn>0 && selected != -1)// Jenny, for multisession PPPoE, get existed PPPoE config for further ifindex use
			if (!mib_chain_get(MIB_ATM_VC_TBL, pIdx, (void *)&myEntry)) {
				strcpy(tmpBuf, errGetEntry);
				goto setErr_filter;
			}
	}
#endif
	
	if (selected == -1) {//no pvc exist, add
#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
		fcEntry.ConDevInstNum = 1 + findMaxConDevInstNum();
		if( (fcEntry.cmode==ADSL_PPPoE) || 
#ifdef PPPOE_PASSTHROUGH
			((fcEntry.cmode==ADSL_BR1483)&&(fcEntry.brmode==BRIDGE_PPPOE)) ||
#endif
			(fcEntry.cmode==ADSL_PPPoA) )
			fcEntry.ConPPPInstNum = 1;
		else
			fcEntry.ConIPInstNum = 1;
		//fprintf( stderr, "<%s:%d>NewInstNum=>ConDev:%u, PPPCon:%u, IPCon:%u\n", __FILE__, __LINE__, fcEntry.ConDevInstNum, fcEntry.ConPPPInstNum, fcEntry.ConIPInstNum );
#endif

		fcEntry.ifIndex = if_find_index(fcEntry.cmode, ifMap);
		
#ifndef ZTE_GENERAL_ROUTER_SC
		fcEntry.mtu = 1500;
#else
		if (fcEntry.cmode != ADSL_BR1483) {
			if (drflag && fcEntry.dgw)
			{
				strcpy(tmpBuf, T(Tdef_gw_exist));
				goto setErr_filter;
			}
		}

		// set default
		if (fcEntry.cmode == ADSL_PPPoE)
			fcEntry.mtu = 1452;
		else
			fcEntry.mtu = 1500;

#endif
#ifdef CONFIG_EXT_SWITCH
		// VLAN
		fcEntry.vlan = 0;	// disable
		fcEntry.vid = 0; // VLAN tag
		fcEntry.vprio = 0; // priority bits (0 ~ 7)
		fcEntry.vpass = 0; // no pass-through
#endif
#ifdef ZTE_GENERAL_ROUTER_SC
		MIB_CE_ACC_T accEntry;		
		assignDefaultValue(&accEntry);
		copyPvcEntryToAccEntry(&accEntry,&fcEntry);
		intVal = mib_chain_add(MIB_ACC_TBL, (unsigned char*)&accEntry);
		if (intVal == 0) {
			strcpy(tmpBuf, T(strAddChainerror));
			goto setErr_filter;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, T(strTableFull));
			goto setErr_filter;
		}
		
#endif
		intVal = mib_chain_add(MIB_ATM_VC_TBL, (unsigned char*)&fcEntry);
		if (intVal == 0) {
			strcpy(tmpBuf, T(Tadd_err));
			goto setErr_filter;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, T(strTableFull));
			goto setErr_filter;
		}
	} else {//pvc exist, then modify vc0
		if (!mib_chain_get(MIB_ATM_VC_TBL, selected, (void *)&Entry)) {
			strcpy(tmpBuf, errGetEntry);
			goto setErr_filter;
		}
#ifdef ZTE_GENERAL_ROUTER_SC
		if (fcEntry.cmode == ADSL_PPPoE) {
			if (cnt > 0) {		// Jenny, for multisession PPPoE, ifIndex(VC device) must refer to existed PPPoE connection
				ifMap &= 0xffff0000; // don't care the vc part
				fcEntry.ifIndex = if_find_index(fcEntry.cmode, ifMap);
				fcEntry.ifIndex &= 0xf0;
				fcEntry.ifIndex |= (myEntry.ifIndex&0x0f);
			}
			else {
				fcEntry.ifIndex = if_find_index(fcEntry.cmode, ifMap);
#ifdef PPPOE_PASSTHROUGH
				if (fcEntry.cmode == Entry.cmode)
					fcEntry.brmode = Entry.brmode;
#endif
			}
		} else
			fcEntry.ifIndex = Entry.ifIndex;
		
		fcEntry.qos = Entry.qos;
		fcEntry.pcr = Entry.pcr;
		fcEntry.scr = Entry.scr;
		fcEntry.mbs = Entry.mbs;
		fcEntry.cdvt = Entry.cdvt;
		fcEntry.pppAuth = Entry.pppAuth;
		fcEntry.rip = Entry.rip;
		fcEntry.mtu = Entry.mtu;
#else
		fcEntry.ifIndex = Entry.ifIndex;
#endif

#ifdef PPPOE_PASSTHROUGH
		if (fcEntry.cmode != ADSL_PPPoE)
			if (fcEntry.cmode == Entry.cmode)
				fcEntry.brmode = Entry.brmode;
#endif

#ifdef CONFIG_EXT_SWITCH
		// VLAN
		fcEntry.vlan = Entry.vlan;
		fcEntry.vid = Entry.vid;
		fcEntry.vprio = Entry.vprio;
		fcEntry.vpass = Entry.vpass;
#endif
#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
		fcEntry.connDisable = 0;
		if (Entry.vpi == fcEntry.vpi && Entry.vci == fcEntry.vci)
		{
			fcEntry.ConDevInstNum = Entry.ConDevInstNum;
			if( (fcEntry.cmode==ADSL_PPPoE) || 
#ifdef PPPOE_PASSTHROUGH
			    ((fcEntry.cmode==ADSL_BR1483)&&(fcEntry.brmode==BRIDGE_PPPOE)) ||
#endif
			    (fcEntry.cmode==ADSL_PPPoA) )
			{
				if( Entry.ConPPPInstNum!=0 )
					fcEntry.ConPPPInstNum = Entry.ConPPPInstNum;
				else
					fcEntry.ConPPPInstNum = 1 + findMaxPPPConInstNum( fcEntry.ConDevInstNum );
				fcEntry.ConIPInstNum = 0;
			}else{
				fcEntry.ConPPPInstNum = 0;
				if(Entry.ConIPInstNum!=0)
					fcEntry.ConIPInstNum = Entry.ConIPInstNum;
				else
					fcEntry.ConIPInstNum = 1 + findMaxIPConInstNum( fcEntry.ConDevInstNum );
			}
		}else{
			unsigned int  instnum=0;
			instnum = findConDevInstNumByPVC( fcEntry.vpi, fcEntry.vci );
			if(instnum==0)
				instnum = 1 + findMaxConDevInstNum();
			fcEntry.ConDevInstNum = instnum;
			if( (fcEntry.cmode==ADSL_PPPoE) || 
#ifdef PPPOE_PASSTHROUGH
			    ((fcEntry.cmode==ADSL_BR1483)&&(fcEntry.brmode==BRIDGE_PPPOE)) ||
#endif
			    (fcEntry.cmode==ADSL_PPPoA) )
			{
				fcEntry.ConPPPInstNum = 1 + findMaxPPPConInstNum( fcEntry.ConDevInstNum );
				fcEntry.ConIPInstNum = 0;
			}else{
				fcEntry.ConPPPInstNum = 0;
				fcEntry.ConIPInstNum = 1 + findMaxIPConInstNum( fcEntry.ConDevInstNum );
			}
		}
		//fprintf( stderr, "<%s:%d>NewInstNum=>ConDev:%u, PPPCon:%u, IPCon:%u\n", __FILE__, __LINE__, fcEntry.ConDevInstNum, fcEntry.ConPPPInstNum, fcEntry.ConIPInstNum );

		fcEntry.autoDisTime = Entry.autoDisTime;
		fcEntry.warnDisDelay = Entry.warnDisDelay;
		strcpy( fcEntry.pppServiceName, Entry.pppServiceName );
		strcpy( fcEntry.WanName, Entry.WanName );
#ifdef _PRMT_TR143_
		fcEntry.TR143UDPEchoItf = Entry.TR143UDPEchoItf;
#endif //_PRMT_TR143_
#ifdef _PRMT_X_CT_COM_PPPOEv2_
		fcEntry.PPPoEProxyEnable = Entry.PPPoEProxyEnable;
		fcEntry.PPPoEProxyMaxUser = Entry.PPPoEProxyMaxUser;
#endif //_PRMT_X_CT_COM_PPPOEv2_
#ifdef _PRMT_X_CT_COM_WANEXT_
		fcEntry.ServiceList = Entry.ServiceList;
#endif //_PRMT_X_CT_COM_WANEXT_
#endif //_CWMP_MIB_

#ifdef ZTE_GENERAL_ROUTER_SC
		strcpy(fcEntry.pppACName, Entry.pppACName);
		
		//set default route flag
		if (fcEntry.cmode != ADSL_BR1483)
			if (!fcEntry.dgw && Entry.dgw)
				drflag =0;
		else
			if (Entry.dgw)
				drflag = 0;

		// find the ifIndex
		if (fcEntry.cmode != Entry.cmode)
		{
			if (!(fcEntry.cmode == ADSL_PPPoE && cnt>0))	// Jenny, fcEntry is except multisession PPPoE
				fcEntry.ifIndex = if_find_index(fcEntry.cmode, ifMap);
			if (fcEntry.ifIndex == 0xff)
			{
				strcpy(tmpBuf, T(Tvc_exceed));
				goto setErr_filter;
			}
			else if (fcEntry.ifIndex == 0xef)
			{
				strcpy(tmpBuf, T(Tppp_conn_excd));
				goto setErr_filter;
			}
			
			// mode changed, restore to default
			if (fcEntry.cmode == ADSL_PPPoE) {
				fcEntry.mtu = 1452;
				fcEntry.pppAuth = 0;
			}
			else {
				fcEntry.mtu = 1500;
			}
			
#ifdef CONFIG_EXT_SWITCH
			// VLAN
			fcEntry.vlan = 0;	// disable
			fcEntry.vid = 0; // VLAN tag
			fcEntry.vprio = 0; // priority bits (0 ~ 7)
			fcEntry.vpass = 0; // no pass-through
#endif
		}
		
		if (fcEntry.cmode != ADSL_BR1483)
		{
			if (drflag && fcEntry.dgw && !Entry.dgw)
			{
				strcpy(tmpBuf, T(Tdef_gw_exist));
				goto setErr_filter;
			}
		}
#endif
	if( fcEntry.ifIndex!=Entry.ifIndex )
	{
		updatePortForwarding( Entry.ifIndex, fcEntry.ifIndex );
		updateRoutingTable( Entry.ifIndex, fcEntry.ifIndex );
	}
#ifdef ZTE_GENERAL_ROUTER_SC
		if(Entry.cmode!=fcEntry.cmode)
			filter_set_remote_access(0);
		MIB_CE_ACC_T accEntry; 
		if(!mib_chain_get(MIB_ACC_TBL,selected,&accEntry))
		{
			strcpy(tmpBuf,"获取链表记录出错!");
			goto setErr_filter;
		}
		copyPvcEntryToAccEntry(&accEntry,&fcEntry);
		
		if(Entry.cmode!=fcEntry.cmode)
		{
			assignDefaultValue(&accEntry);
			mib_chain_update(MIB_ACC_TBL,&accEntry,selected);  
			filter_set_remote_access(1);
		} else
		 	mib_chain_update(MIB_ACC_TBL,&accEntry,selected);
#endif
		memcpy(&Entry, &fcEntry, sizeof(fcEntry));
		// log message
		mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, selected);
#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		/*to avoid the dead when wrriting the flash*/
		itfcfg("sar", 0);
		itfcfg("eth0", 0);
#ifdef WLAN_SUPPORT
		itfcfg("wlan0", 0);
#endif
#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/

		//apply changes
		//stopConnection(&Entry);
		//startConnection(&fcEntry);
	}

	if (fcEntry2.enable_ip2) {
		if ( !mib_set( MIB_ADSL_LAN_IP2, (void *)fcEntry2.lanIp2)) {
			strcpy(tmpBuf, T(Tsecond_ip_err));
			goto setErr_filter;
		}
		if ( !mib_set(MIB_ADSL_LAN_SUBNET2, (void *)fcEntry2.subnetMask2)) {
			strcpy(tmpBuf, T(Tsecond_mask_err));
			goto setErr_filter;
		}	
	}
	mib_set(MIB_ADSL_LAN_ENABLE_IP2, (void *)&fcEntry2.enable_ip2);	

	//restart lan IP
	{
		if ( mib_set(MIB_DHCP_MODE, (void *)&fcEntry2.enable_dhcp) == 0) {
			strcpy(tmpBuf, T(Tdhcp_err));
			goto setErr_filter;
		}
		if (fcEntry2.enable_dhcp) {
			///configure DHCP Server
			unsigned int uLTime;
			unsigned char uStart, uEnd;

			uStart = fcEntry2.dhcpStartIp[3];
			uEnd = fcEntry2.dhcpEndIp[3];
			if (fcEntry2.day != -1)
				uLTime = fcEntry2.day * DAY2SECOND + fcEntry2.hour * HOUR2SECOND + fcEntry2.min * MIN2SECOND;
			else
				uLTime = -1;

			if ( !mib_set(MIB_ADSL_LAN_CLIENT_START, (void *)&uStart)) {
				strcpy(tmpBuf, T(Tset_start_ip_err));
				goto setErr_filter;
			}
			if ( !mib_set(MIB_ADSL_LAN_CLIENT_END, (void *)&uEnd)) {
				strcpy(tmpBuf, T(Tset_end_ip_err));
				goto setErr_filter;
			}		
			if ( !mib_set(MIB_ADSL_LAN_DHCP_LEASE, (void *)&uLTime)) {
				strcpy(tmpBuf, T(Tset_lease_err));
				goto setErr_filter;
			}
		}

#if 0
		//judge whether IP changed
		{
			unsigned char originalIp[4];
			if (!mib_get( MIB_ADSL_LAN_IP, (void *)originalIp)) {
				strcpy(tmpBuf, T(Tget_ip_err));
				goto setErr_filter;
			}
			if ( (originalIp[0]==fcEntry2.ipAddr[0]) &&
				(originalIp[1]==fcEntry2.ipAddr[1]) &&
				(originalIp[2]==fcEntry2.ipAddr[2]) &&
				(originalIp[3]==fcEntry2.ipAddr[3]) )
				fcEntry2.ip_changed = 0;
			else
				fcEntry2.ip_changed = 1;
		}
#endif
		if ( !mib_set( MIB_ADSL_LAN_IP, (void *)fcEntry2.ipAddr)) {
			strcpy(tmpBuf, T(Tset_ip_err));
			goto setErr_filter;
		}
		if ( !mib_set( MIB_ADSL_LAN_SUBNET, (void *)fcEntry2.netMask)) {
			strcpy(tmpBuf, T(Tset_mask_err));
			goto setErr_filter;
		}
		if ( !mib_set( MIB_ADSL_LAN_DHCP_GATEWAY, (void *)fcEntry2.ipAddr)) {
			strcpy(tmpBuf, T(strSetIPerror));
			goto setErr_filter;
		}
	}
	//restart_lanip();
	
setOk_filter:
	//mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
#ifndef NO_ACTION
	pid = fork();
        if (pid) {
	      	waitpid(pid, NULL, 0);
	}
        else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _FIREWALL_SCRIPT_PROG);
		execl( tmpBuf, _FIREWALL_SCRIPT_PROG, NULL);
               	exit(1);
        }
#endif

	///restart system
	websHeader(wp);
	websWrite(wp, T("<META HTTP-EQUIV=Refresh CONTENT=\"60; URL=/status_sc.asp\">\n"));
   	websWrite(wp, T("<body><blockquote><h4>\n"));
   	websWrite(wp, T("%s</h4>\n"), Treboot_wd0);
   	websWrite(wp, T("%s<br><br>\n"), Treboot_wd1);
   	websWrite(wp, T("<table><tr><td width=\"600\">%s</td></tr></table>\n"), Treboot_wd2);
   	websWrite(wp, T("</blockquote></body>"));
   	websFooter(wp);
	websDone(wp, 200);
	
	//mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
	//semSave.__sem_value=SEM_REBOOT;
	sem_post(&semSave);

  	return;
jumpLastPage:
  	submitUrl = websGetVar(wp, T("submit-url-l"), T(""));
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
	return;
setErr_filter:
	ERR_MSG(tmpBuf);
}

#if 0
void formFastConfEnd(webs_t wp, char_t *path, char_t *query)
{
	char_t  *submitUrl;

	submitUrl = websGetVar(wp, T("submit-url"), T(""));

	if (fcEntry2.ip_changed)
		restart_lanip();
	else
		websRedirect(wp, submitUrl);

	return 0;
}
#endif

void formFcPppWan(webs_t wp, char_t *path, char_t *query)
{
	char_t *strSubmit, *submitUrl;
	char_t *strValue;
	struct in_addr inIp;
	char tmpBuf[100];

	strSubmit = websGetVar(wp, T("laststep"), T(""));
	if (strSubmit[0]) {
		goto jumpLastPage;
	}
	
	strValue = websGetVar(wp, T("naptEnabled"), T(""));
	if ( !gstrcmp(strValue, T("ON")))
		fcEntry.napt = 1;
	else
		fcEntry.napt = 0;

	strValue = websGetVar(wp, T("droute"), T(""));
	if ( !gstrcmp(strValue, T("ON")) )
		fcEntry.dgw = 1;
	else
		fcEntry.dgw = 0;

#ifdef CONFIG_SPPPD_STATICIP
	strValue = websGetVar(wp, T("ipMode"), T(""));
	if (strValue[0])
		fcEntry.pppIp = strValue[0]-'0';
	
	if (strValue[0]=='1') {//get pppoe wan IP
		strValue = websGetVar(wp, T("ip"), T(""));
		if (strValue[0]) {
			if (!inet_aton(strValue, (struct in_addr *)fcEntry.ipAddr)) {
				strcpy(tmpBuf, T(Tinvalid_ip));
				goto setErr;
			}
		}
	}
#endif

	submitUrl = websGetVar(wp, T("submit-url-n"), T(""));   // hidden page
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
  	return;

jumpLastPage:
  	submitUrl = websGetVar(wp, T("submit-url-l"), T(""));
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
	return;
setErr:
	ERR_MSG(tmpBuf);
}

void formFcPppType(webs_t wp, char_t *path, char_t *query)
{
	char_t *strSubmit, *submitUrl;
	char tmpBuf[100];
	char_t *strValue;
	unsigned int vUInt;

	strSubmit = websGetVar(wp, T("laststep"), T(""));
	if (strSubmit[0]) {
		goto jumpLastPage;
	}
	
	// PPP user name
	strValue = websGetVar(wp, T("pppUserName"), T(""));
	if ( strValue[0] ) {
		if ( strlen(strValue) > MAX_NAME_LEN-1 ) {
			strcpy(tmpBuf, T(Tuser_long));
			goto setErr_filter;
		}
		strncpy(fcEntry.pppUsername, strValue, MAX_NAME_LEN-1);
		fcEntry.pppUsername[MAX_NAME_LEN-1]='\0';
		//fcEntry.pppUsername[MAX_NAME_LEN]='\0';
	}

	// PPP password
	strValue = websGetVar(wp, T("pppPassword"), T(""));
	if ( strValue[0] ) {
		if ( strlen(strValue) > MAX_NAME_LEN-1 ) {
			strcpy(tmpBuf, T(Tpassw_long));
			goto setErr_filter;
		}
		strncpy(fcEntry.pppPassword, strValue, MAX_NAME_LEN-1);
		fcEntry.pppPassword[MAX_NAME_LEN-1]='\0';
		//fcEntry.pppPassword[MAX_NAME_LEN]='\0';
	}

	// PPP connection type
	strValue = websGetVar(wp, T("pppConnectType"), T(""));
	if ( strValue[0] ) {
		PPP_CONNECT_TYPE_T type;
			
		if ( strValue[0] == '0' )
			type = CONTINUOUS;
		else if ( strValue[0] == '1' )
			type = CONNECT_ON_DEMAND;
		else if ( strValue[0] == '2' )
			type = MANUAL;
		else {
			strcpy(tmpBuf, T(Tinvalid_ppp_type));
			goto setErr_filter;
		}
			
		fcEntry.pppCtype = (unsigned char)type;
			
		if (type == CONNECT_ON_DEMAND) {
			// PPP idle time
			strValue = websGetVar(wp, T("pppIdleTime"), T(""));
			if ( strValue[0] ) {
				unsigned short time;
				time = (unsigned short) strtol(strValue, (char**)NULL, 10);
				fcEntry.pppIdleTime = time;
			}
		} else if (type == MANUAL) {
			// PPP idle time
			strValue = websGetVar(wp, T("pppIdleTime2"), T(""));
			if ( strValue[0] ) {
				unsigned short time;
				time = (unsigned short) strtol(strValue, (char**)NULL, 10);
				fcEntry.pppIdleTime = time;
			}
		}
	}

	submitUrl = websGetVar(wp, T("submit-url-n"), T(""));   // hidden page
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
  	return;
	
jumpLastPage:
  	submitUrl = websGetVar(wp, T("submit-url-l"), T(""));
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
	return;
	
setErr_filter:
	ERR_MSG(tmpBuf);
}

void formFcMerWan(webs_t wp, char_t *path, char_t *query)
{
	char_t *strSubmit, *submitUrl;
	char_t *strValue;
	char tmpBuf[100];
	
	strSubmit = websGetVar(wp, T("laststep"), T(""));
	if (strSubmit[0]) {
		goto jumpLastPage;
	}

	fcEntry.ipunnumbered = 0;
	//IP mode
	strValue = websGetVar(wp, T("ipmode"), T(""));
	if (strValue[0]) {
		if(strValue[0] == '0')
			fcEntry.ipDhcp = (char)DHCP_DISABLED;
		else if(strValue[0] == '1')
			fcEntry.ipDhcp = (char)DHCP_CLIENT;
		else {
			strcpy(tmpBuf, T(Tinvalid_wan_dhcp));
			goto setErr_filter;
		}
	}

	if (fcEntry.ipDhcp == DHCP_DISABLED) {
		// Local WAN IP address
		strValue = websGetVar(wp, T("ip"), T(""));
		if ( strValue[0] ) {
			if ( !inet_aton(strValue, (struct in_addr *)&fcEntry.ipAddr) ) {
				strcpy(tmpBuf, T(Tinvalid_ip));
				goto setErr_filter;
			}
		}
			
		// Reserved Gateway address
		strValue = websGetVar(wp, T("remoteIp"), T(""));
		if ( strValue[0] ) {
			if ( !inet_aton(strValue, (struct in_addr *)&fcEntry.remoteIpAddr) ) {
				strcpy(tmpBuf, T(Tinvalid_gw));
				goto setErr_filter;
			}
		}
			
		// Subnet Mask
		strValue = websGetVar(wp, T("netmask"), T(""));
		if ( strValue[0] ) {
			if ( !inet_aton(strValue, (struct in_addr *)&fcEntry.netMask) ) {
				strcpy(tmpBuf, T(Tinvalid_mask));
				goto setErr_filter;
			}
		}
	}

	//dns mode
	strValue = websGetVar(wp, T("dnsMode"), T(""));
	if ( strValue[0] ) {
		if (!strcmp(strValue, T("dnsAuto")))
			fcEntry2.dnsMode = DNS_AUTO;
		else if (!strcmp(strValue, T("dnsManual")))
			fcEntry2.dnsMode = DNS_MANUAL;
		else {
			strcpy(tmpBuf, T(Tinvalid_dns_mode));
			goto setErr_filter;
		}
	}
	if (fcEntry2.dnsMode == DNS_MANUAL) {
		//dns1
		strValue = websGetVar(wp, T("dns1"), T(""));
		if (strValue[0]) {
			if ( !inet_aton(strValue, (struct in_addr *)&fcEntry2.dns1) ) {
				strcpy(tmpBuf, T(Tinvalid_dns));
				goto setErr_filter;
			}
		}
		//dns2
		strValue = websGetVar(wp, T("dns2"), T(""));
		if (strValue[0]) {
			if ( !inet_aton(strValue, (struct in_addr *)&fcEntry2.dns2) ) {
				strcpy(tmpBuf, T(Tinvalid_dns));
				goto setErr_filter;
			}
		}
	}

	//napt
	strValue = websGetVar(wp, T("napt"), T(""));
	if ( strValue[0] )
		fcEntry.napt = 1;
	else
		fcEntry.napt = 0;

	strValue = websGetVar(wp, T("droute"), T(""));
	if ( strValue[0] )
		fcEntry.dgw = 1;
	else
		fcEntry.dgw = 0;
	
	submitUrl = websGetVar(wp, T("submit-url-n"), T(""));	// hidden page
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
	return;
		
jumpLastPage:
	submitUrl = websGetVar(wp, T("submit-url-l"), T(""));
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
	return;
		
setErr_filter:
	ERR_MSG(tmpBuf);
}

void formFcIPoA(webs_t wp, char_t *path, char_t *query)
{
	char_t *strSubmit, *submitUrl;
	char_t *strValue;
	char tmpBuf[100];
	
	strSubmit = websGetVar(wp, T("laststep"), T(""));
	if (strSubmit[0]) {
		goto jumpLastPage;
	}
	
	fcEntry.ipunnumbered = 0;
	//IP mode
	strValue = websGetVar(wp, T("ipmode"), T(""));
	if (strValue[0]) {
		if(strValue[0] == '2')
			fcEntry.ipDhcp = (char)DHCP_DISABLED;
		else if(strValue[0] == '1')
			fcEntry.ipDhcp = (char)DHCP_CLIENT;
		else {
			fcEntry.ipunnumbered = 1;
		}
	}

	if (fcEntry.ipunnumbered == 0){
		if (fcEntry.ipDhcp == DHCP_DISABLED) {
			// Local WAN IP address
			strValue = websGetVar(wp, T("ip"), T(""));
			if ( strValue[0] ) {
				if ( !inet_aton(strValue, (struct in_addr *)&fcEntry.ipAddr) ) {
					strcpy(tmpBuf, T(Tinvalid_ip));
					goto setErr_filter;
				}
			}
					
			// Subnet Mask
			strValue = websGetVar(wp, T("netmask"), T(""));
			if ( strValue[0] ) {
				if ( !inet_aton(strValue, (struct in_addr *)&fcEntry.netMask) ) {
					strcpy(tmpBuf, T(Tinvalid_mask));
					goto setErr_filter;
				}
			}
		}

		//dns mode
		strValue = websGetVar(wp, T("dnsMode"), T(""));
		if ( strValue[0] ) {
			if (!strcmp(strValue, T("dnsAuto")))
				fcEntry2.dnsMode = DNS_AUTO;
			else if (!strcmp(strValue, T("dnsManual")))
				fcEntry2.dnsMode = DNS_MANUAL;
			else {
				strcpy(tmpBuf, T(Tinvalid_dns_mode));
				goto setErr_filter;
			}
		}
		if (fcEntry2.dnsMode == DNS_MANUAL) {
			//dns1
			strValue = websGetVar(wp, T("dns1"), T(""));
			if (strValue[0]) {
				if ( !inet_aton(strValue, (struct in_addr *)&fcEntry2.dns1) ) {
					strcpy(tmpBuf, T(Tinvalid_dns));
					goto setErr_filter;
				}
			}
			//dns2
			strValue = websGetVar(wp, T("dns2"), T(""));
			if (strValue[0]) {
				if ( !inet_aton(strValue, (struct in_addr *)&fcEntry2.dns2) ) {
					strcpy(tmpBuf, T(Tinvalid_dns));
					goto setErr_filter;
				}
			}
		}
	}

	//napt
	strValue = websGetVar(wp, T("napt"), T(""));
	if ( strValue[0] )
		fcEntry.napt = 1;
	else
		fcEntry.napt = 0;

	//droute
	strValue = websGetVar(wp, T("droute"), T(""));
	if (strValue[0])
		fcEntry.dgw = 1;
	else
		fcEntry.dgw = 0;

	submitUrl = websGetVar(wp, T("submit-url-n"), T(""));	// hidden page
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
	return;
jumpLastPage:
	submitUrl = websGetVar(wp, T("submit-url-l"), T(""));
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
	return;
		
setErr_filter:
	ERR_MSG(tmpBuf);

}

#if 0
void showFastConfP1(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;

	nBytesSent += websWrite(wp, T("<h2><font color=\"#0000FF\">%s</font></h2>"
		"<table border=0 width=\"700\" cellspacing=4 cellpadding=0>"
		"<tr><td><font size=3>%s</font></td></tr>"
		"</table>"
		"<table border=0 width=\"700\" cellspacing=4 cellpadding=0>"
		"<tr>	<td><font size=4 color=\"#2F2640\"><b>%s </b></font>"
  		"<font size=3>%s</font></td></tr></table>"
		"<h2><font color=\"#0000FF\">%s</font></h2>"
		"<table border=0 width=\"700\" cellspacing=4 cellpadding=0>"
		"<tr><td><font size=3>%s</font></td></tr></table>"), 
		Tfast_config, Tfc_intro, Tattention, Tatt_intro, Tint_conn_conf, Tint_conn_intro);
	
	nBytesSent += websWrite(wp, T("<form action=/goform/formFastConf method=POST name=\"formFastConfig\">"
		"<tr><td><font size=2><p>VPI: "
		"<input type=\"text\" name=\"vpi\" size=\"3\" value=\"0\">(0-255)"
		"</p></font></td></tr>"
		"<tr><td><font size=2><p>VCI: "
		"<input type=\"text\" name=\"vci\" size=\"5\" value=\"35\">(32-65533)"
		"</p></font></td></tr>"));

	nBytesSent += websWrite(wp, T("<table border=0 width=\"700\" cellspacing=4 cellpadding=0 align=right>"
		"<tr><td>"
		"<input type=\"submit\" value=\"%s\" name=\"nextstep\" onClick=\"return addClick()\">&nbsp;&nbsp;"
		"<input type=\"hidden\" value=\"/fc-page2.asp\" name=\"submit-url\">"
		"</td></tr></table></form>"), Tnext_step);
}
#endif
#endif
/////////////////////////////////////////////////////////////////////////////
void formAtmVc(webs_t wp, char_t *path, char_t *query)
{
	char_t *strVal, *submitUrl;
	MIB_CE_ATM_VC_T entry;
	char tmpBuf[100];
	memset(tmpBuf,0x00,100);

	strVal = websGetVar(wp, T("changeAtmVc"), T(""));

	if (strVal[0] ) {
		unsigned int i, k;
		MIB_CE_ATM_VC_T Entry;
		unsigned int totalEntry = mib_chain_total(MIB_ATM_VC_TBL); /* get chain record size */
		unsigned int vUInt;
		
		strVal = websGetVar(wp, T("select"), T(""));
		if (!strVal[0]) {
			strcpy(tmpBuf, T(strSelectvc));
			goto setErr_filter;
		}
		
		sscanf(strVal,"s%u", &i);
			
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			websError(wp, 400, T(strGetChainerror));
			return;
		}
		
		memcpy(&entry, &Entry, sizeof(entry));
		
		strVal = websGetVar(wp, T("qos"), T(""));

		if (!strVal[0]) {
			strcpy(tmpBuf, T("Invalid QoS!"));
			goto setErr_filter;
		}
		
		entry.qos = strVal[0] - '0';
		
		strVal = websGetVar(wp, T("pcr"), T(""));
		
		if (!strVal[0]) {
			strcpy(tmpBuf, T("Invalid PCR!"));
			goto setErr_filter;
		}
		
		sscanf(strVal,"%u",&vUInt);
		if ( vUInt>65535) {
			strcpy(tmpBuf, T("Invalid PCR!"));
			goto setErr_filter;
		}
		entry.pcr = (unsigned short)vUInt;
		
		strVal = websGetVar(wp, T("cdvt"), T(""));
		
		if (!strVal[0]) {
			strcpy(tmpBuf, T("Invalid CDVT!"));
			goto setErr_filter;
		}
		
		sscanf(strVal,"%u",&entry.cdvt);
		
		if (entry.qos == 2 || entry.qos == 3)
		{
			strVal = websGetVar(wp, T("scr"), T(""));

			if (!strVal[0]) {
				strcpy(tmpBuf, T("Invalid SCR!"));
				goto setErr_filter;
			}
			
			sscanf(strVal,"%u",&vUInt);
			if ( vUInt>65535) {
				strcpy(tmpBuf, T("Invalid SCR!"));
				goto setErr_filter;
			}
			entry.scr = (unsigned short)vUInt;

			strVal = websGetVar(wp, T("mbs"), T(""));
			
			if (!strVal[0]) {
				strcpy(tmpBuf, T("Invalid MBS!"));
				goto setErr_filter;
			}
			
			sscanf(strVal,"%u",&vUInt);
			if ( vUInt>65535) {
				strcpy(tmpBuf, T("Invalid MBS!"));
				goto setErr_filter;
			}
			entry.mbs = (unsigned short)vUInt;
		}
		
#if defined(APPLY_CHANGE) || defined(ZTE_531B_BRIDGE_SC)
		stopConnection(&Entry);
#endif
		memcpy(&Entry, &entry, sizeof(entry));
		// log message
		mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, i);
		
		// synchronize this vc across all interfaces
		for (k=i+1; k<totalEntry; k++) {
			if (!mib_chain_get(MIB_ATM_VC_TBL, k, (void *)&Entry)) {
				strcpy(tmpBuf, errGetEntry);
				goto setErr_filter;
			}
			if (Entry.vpi == entry.vpi && Entry.vci == entry.vci) {
				Entry.qos = entry.qos;
				Entry.pcr = entry.pcr;
				Entry.cdvt = entry.cdvt;
				Entry.scr = entry.scr;
				Entry.mbs = entry.mbs;
				// log message
				mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, k);
			}
		}

#if defined(APPLY_CHANGE) || defined(ZTE_531B_BRIDGE_SC)
		startConnection(&entry);

// Kaohj
#if 0
		unsigned char vChar;
#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		/*to avoid the dead when wrriting the flash*/
		itfcfg("sar", 0);
		itfcfg("eth0", 0);
#ifdef WLAN_SUPPORT
		itfcfg("wlan0", 0);
#endif
#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
		mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);

#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		itfcfg("sar", 1);
		 itfcfg("eth0", 1);
#ifdef WLAN_SUPPORT
		vChar=0;
		mib_get( MIB_WLAN_DISABLED, (void *)&vChar);
		if( vChar==0 )	itfcfg("wlan0", 1);
#endif //WLAN_SUPPORT
#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
#endif // of #if 0
#endif
	}	


setOk_filter:
//	mib_update(CURRENT_SETTING);

#ifndef NO_ACTION
	pid = fork();
        if (pid) {
	      	waitpid(pid, NULL, 0);
	}
        else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _FIREWALL_SCRIPT_PROG);
		execl( tmpBuf, _FIREWALL_SCRIPT_PROG, NULL);
               	exit(1);
        }
#endif

	submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page
	OK_MSG(submitUrl);
	return;
setErr_filter:
	ERR_MSG(tmpBuf);
}

/////////////////////////////////////////////////////////////////////////////

int atmVcList(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;

	unsigned int entryNum, i, k;
	MIB_CE_ATM_VC_T Entry;
	char	vpi[6], vci[6], *qos, pcr[6], scr[6], mbs[6];
	char cdvt[12];
	char *temp;
	int vcList[MAX_VC_NUM], found;

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	memset(vcList, 0, MAX_VC_NUM*4);

#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
	nBytesSent += websWrite(wp, T("<tr><font size=2>"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\"><font size=2><b>索引</b></td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\"><font size=2><b>VPI</b></td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\"><font size=2><b>VCI</b></td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\"><font size=2><b>QoS</b></td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\"><font size=2><b>PCR</b></td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\"><font size=2><b>CDVT</b></td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\"><font size=2><b>SCR</b></td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\"><font size=2><b>MBS</b></td></font></tr>\n"));

#else
	nBytesSent += websWrite(wp, T("<tr><font size=1>"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\">Select</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">VPI</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">VCI</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">QoS</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">PCR</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">CDVT</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">SCR</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">MBS</td></font></tr>\n"));
#endif

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			websError(wp, 400, T("Get chain record error!\n"));
			return -1;
		}
		
		// skip duplicate vc display
		found = 0;
		for (k=0; k<MAX_VC_NUM && vcList[k] != 0; k++) {
			if (vcList[k] == VC_INDEX(Entry.ifIndex)) {
				found = 1;
				break;
			}
		}
		if (found)
			continue;
		else
			vcList[k] = VC_INDEX(Entry.ifIndex);
		
		snprintf(vpi, 6, "%u", Entry.vpi);
		snprintf(vci, 6, "%u", Entry.vci);
		snprintf(pcr, 6, "%u", Entry.pcr);
		snprintf(cdvt, 12, "%u", Entry.cdvt);

		if ( Entry.qos == ATMQOS_UBR )
			qos = "UBR";
		else if ( Entry.qos == ATMQOS_CBR )
			qos = "CBR";
		else if ( Entry.qos == ATMQOS_VBR_NRT )
			qos = "nrt-VBR";
		else if ( Entry.qos == ATMQOS_VBR_RT )
			qos = "rt-VBR";

		if(Entry.qos > 1) {
			snprintf(scr, 6, "%u", Entry.scr);
			snprintf(mbs, 6, "%u", Entry.mbs);
		} else {
			strcpy(scr, "---");
			strcpy(mbs, "---");
		}
		
		nBytesSent += websWrite(wp, T("<tr>"
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"select\""
		" value=\"s%d\" onClick=\"postVC(%s,%s,%d,%d,%d,%d,%d)\"></td>\n"),
		i,vpi,vci,Entry.qos,Entry.pcr,Entry.cdvt,Entry.scr,Entry.mbs);
		nBytesSent += websWrite(wp, T(
		      	"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		      	"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		      	"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td></tr>\n"),
				vpi, vci, qos, pcr, cdvt, scr, mbs);
	}

	return nBytesSent;
}

#ifdef CONFIG_EXT_SWITCH
static int fm1q(webs_t wp, MIB_CE_ATM_VC_Tp pEntry)
{
	char_t *strValue;
	
	// VLAN
	strValue = websGetVar(wp, T("vlan"), T(""));
	pEntry->vlan = strValue[0] - '0';
	
	strValue = websGetVar(wp, T("vid"), T(""));
	pEntry->vid = atoi(strValue);
	if (pEntry->vid<=0 || pEntry->vid>=4096)
		pEntry->vid=0;
	
	/*
	strValue = websGetVar(wp, T("prio"), T(""));
	pEntry->vprio = strValue[0]-'0';
	
	strValue = websGetVar(wp, T("vpass"), T(""));
	if (strValue[0]=='1')
		pEntry->vpass = 1;
	else
		pEntry->vpass = 0;
	*/
}
#endif

// Jenny, PPPoE static IP option
#ifdef CONFIG_SPPPD_STATICIP
static int fmStaticIP(webs_t wp, MIB_CE_ATM_VC_Tp pEntry)
{
	char_t *strValue;
	char tmpBuf[100];
	
	// static PPPoE
	strValue = websGetVar(wp, T("pppip"), T(""));
	pEntry->pppIp = strValue[0] - '0';
	
	strValue = websGetVar(wp, T("staticip"), T(""));
	if (strValue[0])
		if (!inet_aton(strValue, (struct in_addr *)pEntry->ipAddr)) {
			strcpy(tmpBuf, T(Tinvalid_ip));
			ERR_MSG(tmpBuf);
		}
}
#endif

/////////////////////////////////////////////////////////////////////////////
void formPPPEdit(webs_t wp, char_t *path, char_t *query)
{
	char_t *strSubmit, *strValue;
	char_t *submitUrl;
	char tmpBuf[100];
	MIB_CE_ATM_VC_T Entry,entry;
	int index,intVal;
	
	strSubmit = websGetVar(wp, T("save"), T(""));
	if (strSubmit[0]) {
		strSubmit = websGetVar(wp, T("item"), T(""));
		index = strSubmit[0] - '0';
		if (!mib_chain_get(MIB_ATM_VC_TBL, index, (void *)&Entry)) {
			strcpy(tmpBuf, errGetEntry);
			goto setErr_ppp;
		}

		memcpy(&entry,&Entry,sizeof(entry));
		
		// status
		strValue = websGetVar(wp, T("status"), T(""));
		Entry.enable = strValue[0] - '0';
		
#ifdef CONFIG_GUI_WEB
		// user name
		strValue = websGetVar(wp, T("name"), T(""));
		if ( strValue[0] ) {
			if ( strlen(strValue) > P_MAX_NAME_LEN-1 ) {
				strcpy(tmpBuf, T(strUserNametoolong));
				goto setErr_ppp;
			}
			strncpy(Entry.pppUsername, strValue, P_MAX_NAME_LEN-1);
			Entry.pppUsername[P_MAX_NAME_LEN-1]='\0';
			//Entry.pppUsername[P_MAX_NAME_LEN]='\0';
		}
		else {
			strcpy(tmpBuf, T(strUserNameempty));
			goto setErr_ppp;
		}
		

		// password
		strValue = websGetVar(wp, T("passwd"), T(""));
		
		if ( strValue[0] ) {
			if ( strlen(strValue) > P_MAX_NAME_LEN-1 ) {
				strcpy(tmpBuf, T(strPasstoolong));
				goto setErr_ppp;
			}
			strncpy(Entry.pppPassword, strValue, P_MAX_NAME_LEN-1);
			Entry.pppPassword[P_MAX_NAME_LEN-1]='\0';
			//Entry.pppPassword[P_MAX_NAME_LEN]='\0';
		}
		else {
			strcpy(tmpBuf, T(strPassempty));
			goto setErr_ppp;
		}
#else
		// user name
		strValue = websGetVar(wp, T("name"), T(""));
		if ( strValue[0] ) {
			if ( strlen(strValue) > MAX_PPP_NAME_LEN ) {
				strcpy(tmpBuf, T(strUserNametoolong));
				goto setErr_ppp;
			}
			strncpy(Entry.pppUsername, strValue, MAX_PPP_NAME_LEN);
			Entry.pppUsername[MAX_PPP_NAME_LEN]='\0';
		}
		else {
			strcpy(tmpBuf, T(strUserNameempty));
			goto setErr_ppp;
		}
		

		// password
		strValue = websGetVar(wp, T("passwd"), T(""));
		
		if ( strValue[0] ) {
			if ( strlen(strValue) > MAX_NAME_LEN-1 ) {
				strcpy(tmpBuf, T(strPasstoolong));
				goto setErr_ppp;
			}
			strncpy(Entry.pppPassword, strValue, MAX_NAME_LEN-1);
			Entry.pppPassword[MAX_NAME_LEN-1]='\0';
			//Entry.pppPassword[MAX_NAME_LEN]='\0';
		}
		else {
			strcpy(tmpBuf, T(strPassempty));
			goto setErr_ppp;
		}
#endif

		// authentication method
		strValue = websGetVar(wp, T("auth"), T(""));
		
		if ( strValue[0] ) {
			PPP_AUTH_T method;
			
			method = (PPP_AUTH_T)(strValue[0] - '0');
			Entry.pppAuth = (unsigned char)method;
		}
			
		// connection type
		strValue = websGetVar(wp, T("pppConnectType"), T(""));
		
		if ( strValue[0] ) {
			PPP_CONNECT_TYPE_T type;
			
			if ( strValue[0] == '0' )
				type = CONTINUOUS;
			else if ( strValue[0] == '1' )
				type = CONNECT_ON_DEMAND;
			else if ( strValue[0] == '2' )
				type = MANUAL;
			else {
				strcpy(tmpBuf, T(strInvalPPPType));
				goto setErr_ppp;
			}
			
			Entry.pppCtype = (unsigned char)type;
			
			if (type == CONNECT_ON_DEMAND) {
				// PPP idle time
				strValue = websGetVar(wp, T("pppIdleTime"), T(""));
				if ( strValue[0] ) {
					unsigned short time;
					time = (unsigned short) strtol(strValue, (char**)NULL, 10);
					if (time > 0)
						Entry.pppIdleTime = time;
					else
						Entry.pppIdleTime = 600;	// Jenny, default PPP idle time
				}
				else
					Entry.pppIdleTime = 600;	// Jenny, default PPP idle time
			}
		}
		
#ifdef DEFAULT_GATEWAY_V1
		// default route
		strValue = websGetVar(wp, T("droute"), T(""));
		Entry.dgw = strValue[0] - '0';
		if (dr && !pdgw && Entry.dgw)
		{
			Entry.dgw = 0;	// set to disable
			strcpy(tmpBuf, T(strDrouteExist));
			goto setErr_ppp;
		}
#endif
		
		// mtu
		strValue = websGetVar(wp, T("mru"), T(""));
		if (strValue[0]){
			intVal = strtol(strValue, (char**)NULL, 10);
			if(intVal <  65 || intVal > (Entry.cmode == ADSL_PPPoE ? 1492 : 1500)){
				strcpy(tmpBuf, T(strMruErr));
				goto setErr_ppp;
			}
			Entry.mtu = intVal;
		}
		else {
			if (Entry.cmode == ADSL_PPPoE)
				Entry.mtu = 1452;		// Jenny, default PPPoE MRU
			else
				Entry.mtu = 1500;		// Jenny, default PPPoA MRU
		}
		
#ifdef CONFIG_SPPPD_STATICIP
		fmStaticIP(wp, &Entry);
#endif

#ifdef PPPOE_PASSTHROUGH
		#if 0
		// PPPoE pass-through
		strValue = websGetVar(wp, T("poe"), T(""));
		Entry.brmode = strValue[0] - '0';
		#endif
		// bridged mode
		strValue = websGetVar(wp, T("mode"), T(""));
		Entry.brmode = strValue[0] - '0';
		syncPPPoEPassthrough(Entry);	// Jenny, for multisession PPPoE
#endif
		
		// Access concentrator name
		strValue = websGetVar(wp, T("acName"), T(""));
		if ( strValue[0] ) {
			if ( strlen(strValue) > MAX_NAME_LEN-1 ) {
				strcpy(tmpBuf, T(strACName));
				goto setErr_ppp;
			}
			strncpy(Entry.pppACName, strValue, MAX_NAME_LEN-1);
			Entry.pppACName[MAX_NAME_LEN-1]='\0';
			//Entry.pppACName[MAX_NAME_LEN]='\0';
		}
		else
			Entry.pppACName[0] = '\0';	// Jenny, AC name could be empty
		
#ifdef CONFIG_EXT_SWITCH
		fm1q(wp, &Entry);
#endif

#ifdef _CWMP_MIB_
		// auto disconnect time
		strValue = websGetVar(wp, T("disconnectTime"), T(""));
		if (strValue[0])
			Entry.autoDisTime = (unsigned short)strtol(strValue, (char**)NULL, 10);
		else
			Entry.autoDisTime = 0;
		// warn disconnect delay
		strValue = websGetVar(wp, T("disconnectDelay"), T(""));
		if (strValue[0])
			Entry.warnDisDelay = (unsigned short)strtol(strValue, (char**)NULL, 10);
		else
			Entry.warnDisDelay = 0;
		// Service name
		strValue = websGetVar(wp, T("serviceName"), T(""));
		if ( strValue[0] ) {
			if ( strlen(strValue) > MAX_NAME_LEN-1 ) {
				strcpy(tmpBuf, T(strServerName));
				goto setErr_ppp;
			}
			strncpy(Entry.pppServiceName, strValue, MAX_NAME_LEN-1);
			Entry.pppServiceName[MAX_NAME_LEN-1]='\0';
			//Entry.pppServiceName[MAX_NAME_LEN]='\0';
		}
		else
			Entry.pppServiceName[0] = '\0';	// Jenny, service name could be empty
#endif
#ifdef CONFIG_USER_PPPOE_PROXY
            //pppoe proxy
            strValue = websGetVar(wp, T("pproxy"), T(""));
	    Entry.PPPoEProxyEnable = strValue[0] - '0';
		printf("set pppoeproxy enable %d \n",Entry.PPPoEProxyEnable);
	  //max user nums
	  strValue = websGetVar(wp, T("maxusernums"), T(""));
		if ( strValue[0] ) {
			Entry.PPPoEProxyMaxUser=atoi(strValue);
		}
		else
			Entry.PPPoEProxyMaxUser = 4;	
		
		
                
#endif
#ifdef WEB_ENABLE_PPP_DEBUG
	// PPP debug
	{
		unsigned char pppdbg;
		struct data_to_pass_st msg;
		strValue = websGetVar(wp, T("pppdebug"), T(""));
		pppdbg = strValue[0] - '0';
		snprintf(msg.data, BUF_SIZE, "spppctl dbg %d debug %d", PPP_INDEX(Entry.ifIndex), pppdbg);
		write_to_pppd(&msg);
	}
#endif
		// log message
		mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, index);

#ifdef CONFIG_GUI_WEB
	writeFlash();
#endif
#ifdef APPLY_CHANGE
	restartWAN();
	/*RegisterPVCnumber();		
	cleanAllFirewallRule();
	restart_dnsrelay();
	startWan(BOOT_LAST);*/
#endif

		goto setOk_ppp;
	}
	
	strSubmit = websGetVar(wp, T("return"), T(""));
	if (strSubmit[0]) {
		goto setOk_ppp;
	}
	
setOk_ppp:
	
	submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
  	return;
  	
setErr_ppp:
	ERR_MSG(tmpBuf);

}

/////////////////////////////////////////////////////////////////////////////
void formIPEdit(webs_t wp, char_t *path, char_t *query)
{
	char_t *strSubmit, *strValue, *strIp, *strMask, *strGW;
	char_t *submitUrl;
	char tmpBuf[100];
	MIB_CE_ATM_VC_T Entry,entry;
	int index;
	
	strSubmit = websGetVar(wp, T("save"), T(""));
	if (strSubmit[0]) {
		strSubmit = websGetVar(wp, T("item"), T(""));
		index = strSubmit[0] - '0';
		if (!mib_chain_get(MIB_ATM_VC_TBL, index, (void *)&Entry)) {
			strcpy(tmpBuf, errGetEntry);
			goto setErr_ip;
		}
		memcpy(&entry,&Entry,sizeof(entry));
		// status
		strValue = websGetVar(wp, T("status"), T(""));
		Entry.enable = strValue[0] - '0';
		
		// Jenny, IP unnumbered
		if (Entry.cmode == ADSL_RT1483) {
			strValue = websGetVar(wp, T("ipunnumber"), T(""));
			Entry.ipunnumbered = strValue[0] - '0';
		  
		}

		// DHCP
		strValue = websGetVar(wp, T("dhcp"), T(""));
		if (strValue[0]) {
			if(strValue[0] == '0')
				Entry.ipDhcp = (char) DHCP_DISABLED;
			else if(strValue[0] == '1')
				Entry.ipDhcp = (char) DHCP_CLIENT;
			else {
				strcpy(tmpBuf, T(strInvalDHCP));
				goto setErr_ip;
			}
		}
		
		if (Entry.ipDhcp == (char) DHCP_DISABLED) {
			// Local IP address
			strIp = websGetVar(wp, T("ipaddr"), T(""));
			if (strIp[0]) {
				if (!inet_aton(strIp, (struct in_addr *)&Entry.ipAddr)) {
					strcpy(tmpBuf, T(strInvalIP));
					goto setErr_ip;
				}
			}
			else
				if (Entry.ipunnumbered!=1) {
					strcpy(tmpBuf, T(strIPAddresserror));
					goto setErr_ip;
				}
				
			
			// Remote IP address
			strGW = websGetVar(wp, T("remoteip"), T(""));
			if (strGW[0]) {
				if (!inet_aton(strGW, (struct in_addr *)&Entry.remoteIpAddr)) {
					strcpy(tmpBuf, T(strInvalGateway));
					goto setErr_ip;
				}
			}
			else
				if (Entry.ipunnumbered!=1) {
					strcpy(tmpBuf, T(strGatewayIpempty));
					goto setErr_ip;
				}
			
			// Subnet mask
			strMask = websGetVar(wp, T("netmask"), T(""));
			if (strMask[0]) {
				if (!isValidNetmask(strMask, 1)) {
					strcpy(tmpBuf, T(strInvalMask));
					goto setErr_ip;
				}
				if (!inet_aton(strMask, (struct in_addr *)&Entry.netMask)) {
					strcpy(tmpBuf, T(strInvalMask));
					goto setErr_ip;
				}
				if (!isValidHostID(strIp, strMask)) {
					strcpy(tmpBuf, T("Invalid IP/Subnet Mask combination!!"));
					goto setErr_ip;
				}
#ifdef DEFAULT_GATEWAY_V1
				if (!isSameSubnet(strIp, strGW, strMask)) {
					strcpy(tmpBuf, T("Invalid IP address! It should be located in the same subnet."));
					goto setErr_ip;
				}
#endif
			}
			/*else
				if (Entry.ipunnumbered!=1) {
					strcpy(tmpBuf, T(strMaskempty));
					goto setErr_ip;
				}*/
		}
			
#ifdef DEFAULT_GATEWAY_V1
		// default route
		strValue = websGetVar(wp, T("droute"), T(""));
		Entry.dgw = strValue[0] - '0';
		if (dr && !pdgw && Entry.dgw)
		{
			Entry.dgw = 0;	// set to disable
			strcpy(tmpBuf, T(strDrouteExist));
			goto setErr_ip;
		}
#endif
		
		// mtu
//		strValue = websGetVar(wp, T("mtu"), T(""));
//		Entry.mtu = strtol(strValue, (char**)NULL, 10);
		// Masob Yu. Set mtu
		strValue = websGetVar(wp, T("mru"), T(""));
		if (strValue[0]) {
			//Entry.mtu = strtol(strValue, (char**)NULL, 10);
			int intVal = strtol(strValue, (char**)NULL, 10);
			if (intVal <  65 || intVal > 1500){
				strcpy(tmpBuf, T(strMruErr));
				goto setErr_ip;
			}
			Entry.mtu = intVal;
		}
		else {			
			Entry.mtu = 1500;		// Mason Yu, default MTU
		}
		
#ifdef PPPOE_PASSTHROUGH
		#if 0
		// PPPoE pass-through
		strValue = websGetVar(wp, T("poe"), T(""));
		Entry.brmode = strValue[0] - '0';
		#endif
		// bridge mode
		strValue = websGetVar(wp, T("mode"), T(""));
		Entry.brmode = strValue[0] - '0';
#endif
#ifdef CONFIG_EXT_SWITCH
		fm1q(wp, &Entry);
#endif
		
		// log message
		mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, index);

#ifdef CONFIG_GUI_WEB
	writeFlash();
#endif
#ifdef APPLY_CHANGE
	restartWAN();
	/*RegisterPVCnumber();		
	cleanAllFirewallRule();
	restart_dnsrelay();
	startWan(BOOT_LAST);*/
#endif

		goto setOk_ip;
	}
	
	strSubmit = websGetVar(wp, T("return"), T(""));
	if (strSubmit[0]) {
		goto setOk_ip;
	}
	
setOk_ip:
	
	submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
  	return;
  	
setErr_ip:
	//memcpy(pEntry, &tmpEntry, sizeof(tmpEntry));
	ERR_MSG(tmpBuf);

}

/////////////////////////////////////////////////////////////////////////////
void formBrEdit(webs_t wp, char_t *path, char_t *query)
{
	char_t *strSubmit, *strValue;
	char_t *submitUrl;
	char tmpBuf[100];
	MIB_CE_ATM_VC_T Entry,entry;
	int index;
	
	strSubmit = websGetVar(wp, T("save"), T(""));
	if (strSubmit[0]) {
		strSubmit = websGetVar(wp, T("item"), T(""));
		index = strSubmit[0] - '0';
		if (!mib_chain_get(MIB_ATM_VC_TBL, index, (void *)&Entry)) {
			strcpy(tmpBuf, errGetEntry);
			goto setErr_br;
		}

		memcpy(&entry,&Entry,sizeof(entry));
		
		// status
		strValue = websGetVar(wp, T("status"), T(""));
		Entry.enable = strValue[0] - '0';
		
#ifdef PPPOE_PASSTHROUGH
		// bridged mode
		strValue = websGetVar(wp, T("mode"), T(""));
		Entry.brmode = strValue[0] - '0';
#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
		if(Entry.cmode==ADSL_BR1483)
		{
			if(Entry.brmode==BRIDGE_PPPOE)
			{	//wanpppconnection
				if( Entry.ConPPPInstNum==0 )
					Entry.ConPPPInstNum = 1 + findMaxPPPConInstNum( Entry.ConDevInstNum );
				Entry.ConIPInstNum = 0;
			}else{  //wanipconnection
				if( Entry.ConIPInstNum==0 )
					Entry.ConIPInstNum = 1 + findMaxIPConInstNum( Entry.ConDevInstNum );
				Entry.ConPPPInstNum = 0;
			}
		}
#endif //_CWMP_MIB_
#endif
		
#ifdef CONFIG_EXT_SWITCH
		fm1q(wp, &Entry);
#endif
#ifdef ZTE_531B_BRIDGE_SC
		stopConnection(&entry);
		startConnection(&Entry);
#endif
		// log message
		mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, index);
#ifdef ZTE_531B_BRIDGE_SC
		unsigned char vChar;
#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		/*to avoid the dead when wrriting the flash*/
		itfcfg("sar", 0);
		itfcfg("eth0", 0);
#ifdef WLAN_SUPPORT
		itfcfg("wlan0", 0);
#endif
#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
		mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);

#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		itfcfg("sar", 1);
		 itfcfg("eth0", 1);
#ifdef WLAN_SUPPORT
		vChar=0;
		mib_get( MIB_WLAN_DISABLED, (void *)&vChar);
		if( vChar==0 )	itfcfg("wlan0", 1);
#endif //WLAN_SUPPORT
#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
#endif

#ifdef CONFIG_GUI_WEB
	writeFlash();
#endif
#ifdef APPLY_CHANGE
	restartWAN();
	/*RegisterPVCnumber();		
	cleanAllFirewallRule();
	restart_dnsrelay();
	startWan(BOOT_LAST);*/
#endif
		goto setOk_br;
	}
	
	strSubmit = websGetVar(wp, T("return"), T(""));
	if (strSubmit[0]) {
		goto setOk_br;
	}
	
setOk_br:
	
	submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
  	return;
  	
setErr_br:
	ERR_MSG(tmpBuf);

}

int atmVcList2(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;

	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	char ifname[6];
	char if_display_name[16];
#ifdef CTC_WAN_NAME
	char ctcWanName[40];
#endif
	char	*mode, vpi[6], vci[6], *aal5Encap;
#ifdef ZTE_GENERAL_ROUTER_SC
	char vpi_vci[10];
#endif
	char	*strNapt, ipAddr[20], remoteIp[20], netmask[20], *strUnnum, *strDroute;
	char IpMask[20];
#ifdef CONFIG_GUI_WEB
	char	userName[P_MAX_NAME_LEN], passwd[P_MAX_NAME_LEN];
#else
	char	userName[MAX_PPP_NAME_LEN+1], passwd[MAX_NAME_LEN];
#endif
#ifdef CONFIG_USER_PPPOE_PROXY
     char pppoeProxy[10]={0};
#endif
	char	*pppType, *strStatus, *temp;
	CONN_T	conn_status;

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
#ifdef DEFAULT_GATEWAY_V1
	dr = 0;
#endif

#ifdef ZTE_531B_BRIDGE_SC
	nBytesSent += websWrite(wp, T("<tr><font size=2>"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\"><font size=2><b>索引</b></td>\n"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=2><b>接口</b></td>\n"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=2><b>模式</b></td>\n"
	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=2><b>VPI</b></td>\n"
	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=2><b>VCI</b></td>\n"
	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=2><b>封装类型</b></td>\n"
	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=2><b>状态</b></td>\n"
	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=2><b>动作</b></td></font></tr>\n"));
#else
#ifdef ZTE_GENERAL_ROUTER_SC
	nBytesSent += websWrite(wp, T("<tr><font size=2>"
	"<td align=center width=\"2%%\" bgcolor=\"#808080\" style=\"word-break:break-all\"><font size=2><b>索引</b></td>\n"
	"<td align=center width=\"14%%\" bgcolor=\"#808080\" style=\"word-break:break-all\"><font size=2><b>接口</b></td>\n"
	"<td align=center width=\"9%%\" bgcolor=\"#808080\" style=\"word-break:break-all\"><font size=2><b>模式</b></td>\n"
	"<td align=center width=\"7%%\" bgcolor=\"#808080\" style=\"word-break:break-all\"><font size=2><b>VPI/ VCI</b></td>\n"
	"<td align=center width=\"7%%\" bgcolor=\"#808080\" style=\"word-break:break-all\"><font size=2><b>封装类型</b></td>\n"
	"<td align=center width=\"6%%\" bgcolor=\"#808080\" style=\"word-break:break-all\"><font size=2><b>NAPT</b></td>\n"
	"<td align=center width=\"19%%\" bgcolor=\"#808080\" style=\"word-break:break-all\"><font size=2><b>IP地址</b></td>\n"
	"<td align=center width=\"15%%\" bgcolor=\"#808080\" style=\"word-break:break-all\"><font size=2><b>网关</b></td>\n"));
	nBytesSent += websWrite(wp, T("<td align=center width=\"9%%\" bgcolor=\"#808080\" style=\"word-break:break-all\"><font size=2><b>用户名</b></td>\n"	
	"<td align=center width=\"6%%\" bgcolor=\"#808080\" style=\"word-break:break-all\"><font size=2><b>默认路由</b></td>\n"));
	nBytesSent += websWrite(wp, T("<td align=center width=\"3%%\" bgcolor=\"#808080\" style=\"word-break:break-all\"><font size=2><b>状态</b></td>\n"
	"<td align=center width=\"3%%\" bgcolor=\"#808080\" style=\"word-break:break-all\"><font size=2><b>动作</b></td></font></tr>\n"));
#else
	nBytesSent += websWrite(wp, T("<tr><font size=2>"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\"><font size=2>Select</td>\n"
	"<td align=center width=\"4%%\" bgcolor=\"#808080\"><font size=2>Inf</td>\n"
	"<td align=center width=\"7%%\" bgcolor=\"#808080\"><font size=2>Mode</td>\n"
	"<td align=center width=\"4%%\" bgcolor=\"#808080\"><font size=2>VPI</td>\n"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\"><font size=2>VCI</td>\n"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\"><font size=2>Encap</td>\n"
	"<td align=center width=\"3%%\" bgcolor=\"#808080\"><font size=2>NAPT</td>\n"
	"<td align=center width=\"13%%\" bgcolor=\"#808080\"><font size=2>IP Addr</td>\n"
#ifdef DEFAULT_GATEWAY_V1
	"<td align=center width=\"13%%\" bgcolor=\"#808080\"><font size=2>Remote IP</td>\n"
#endif
	"<td align=center width=\"13%%\" bgcolor=\"#808080\"><font size=2>Subnet Mask</td>\n"
	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=2>User Name</td>\n"));
	
#ifdef DEFAULT_GATEWAY_V1
	nBytesSent += websWrite(wp, T("<td align=center width=\"3%%\" bgcolor=\"#808080\"><font size=2>DRoute</td>\n"));
#endif
	nBytesSent += websWrite(wp, T("<td align=center width=\"5%%\" bgcolor=\"#808080\"><font size=2>Status</td>\n"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\"><font size=2>Actions</td></font></tr>\n"));
#endif
#endif

	for (i=0; i<entryNum; i++) {
		struct in_addr inAddr;
		int flags;

		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			websError(wp, 400, T("Get chain record error!\n"));
			return -1;
		}
		
		mode = 0;
		
		if (Entry.cmode == ADSL_PPPoE)
			mode = "PPPoE";
		else if (Entry.cmode == ADSL_PPPoA)
			mode = "PPPoA";
		else if (Entry.cmode == ADSL_BR1483)
			mode = "br1483";
		else if (Entry.cmode == ADSL_MER1483)
			mode = "mer1483";
		else if (Entry.cmode == ADSL_RT1483)
			mode = "rt1483";
#ifdef CONFIG_ATM_CLIP
		else if (Entry.cmode == ADSL_RT1577)
			mode = "rt1577";
#endif
		
		snprintf(vpi, 6, "%u", Entry.vpi);
		snprintf(vci, 6, "%u", Entry.vci);
#ifdef ZTE_GENERAL_ROUTER_SC
		sprintf(vpi_vci, "%u/%u", Entry.vpi, Entry.vci);
#endif
		
		aal5Encap = 0;
		if (Entry.encap == 0)
			aal5Encap = "VCMUX";
		else
			aal5Encap = "LLC";
		
		if (Entry.napt == 0)
			strNapt = (char*)IF_OFF;
		else
			strNapt = (char*)IF_ON;
		
#ifdef DEFAULT_GATEWAY_V1
		if (Entry.dgw == 0)	// Jenny, default route
			strDroute = (char*)IF_OFF;
		else
			strDroute = (char*)IF_ON;
		if (Entry.dgw && Entry.cmode != ADSL_BR1483)
			dr = 1;
#endif

		if (Entry.cmode == ADSL_PPPoE || Entry.cmode == ADSL_PPPoA)
		{
			PPP_CONNECT_TYPE_T type;
#ifdef CONFIG_GUI_WEB
			strncpy(userName, Entry.pppUsername, P_MAX_NAME_LEN-1);
			userName[P_MAX_NAME_LEN-1] = '\0';
			//userName[P_MAX_NAME_LEN] = '\0';
			strncpy(passwd, Entry.pppPassword, P_MAX_NAME_LEN-1);
			passwd[P_MAX_NAME_LEN-1] = '\0';
			//passwd[P_MAX_NAME_LEN] = '\0';
#else
			strncpy(userName, Entry.pppUsername, MAX_PPP_NAME_LEN);
			userName[MAX_PPP_NAME_LEN] = '\0';
			//userName[MAX_NAME_LEN] = '\0';
			strncpy(passwd, Entry.pppPassword, MAX_NAME_LEN-1);
			passwd[MAX_NAME_LEN-1] = '\0';
			//passwd[MAX_NAME_LEN] = '\0';
#endif
			type = Entry.pppCtype;
			
			if (type == CONTINUOUS)
				pppType = "conti";
			else if (type == CONNECT_ON_DEMAND)
				pppType = "demand";
			else
				pppType = "manual";
			
			snprintf(ifname, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));
//			printf("interface=%s\n", ifname);
#if 0			
#ifdef EMBED
			if (getInAddr( ifname, IP_ADDR, (void *)&inAddr) == 1)
			{
				temp = inet_ntoa(inAddr);
				strcpy(ipAddr, temp);
			}
			else
#endif
#endif
#ifdef CONFIG_SPPPD_STATICIP
			if (Entry.cmode == ADSL_PPPoE && Entry.pppIp) {
				temp = inet_ntoa(*((struct in_addr *)Entry.ipAddr));
				strcpy(ipAddr, temp);
				strcpy(IpMask, temp);
	        }
			else {
				strcpy(ipAddr, "");
				strcpy(IpMask, "");
			}
#else
			strcpy(ipAddr, "");
			strcpy(IpMask, "");
#endif
#if 0			
#ifdef EMBED
			if (getInAddr( ifname, DST_IP_ADDR, (void *)&inAddr) == 1)
			{
				temp = inet_ntoa(inAddr);
				strcpy(remoteIp, temp);
			}
			else
#endif
#endif
				strcpy(remoteIp, "");
#if 0
#ifdef EMBED
			if (getInAddr( ifname, SUBNET_MASK, (void *)&inAddr) == 1)	// Jenny, subnet mask
			{
				temp = inet_ntoa(inAddr);
				strcpy(netmask, temp);
			}
			else
#endif
#endif
				strcpy(netmask, "");
			
			// set status flag
			if (Entry.enable == 0)
			{
				strStatus = (char *)IF_DISABLED;
				conn_status = CONN_DISABLED;
			}
			else
			if (getInFlags( ifname, &flags) == 1)
			{
				if (flags & IFF_UP)
				{
//					strStatus = (char *)IF_UP;
					strStatus = (char *)IF_ENABLE;
					conn_status = CONN_UP;
				}
				else
				{
					if (find_ppp_from_conf(ifname))
					{
//						strStatus = (char *)IF_DOWN;
						strStatus = (char *)IF_ENABLE;
						conn_status = CONN_DOWN;
					}
					else
					{
//						strStatus = (char *)IF_NA;
						strStatus = (char *)IF_ENABLE;
						conn_status = CONN_NOT_EXIST;
					}
				}
			}
			else
			{
//				strStatus = (char *)IF_NA;
				strStatus = (char *)IF_ENABLE;
				conn_status = CONN_NOT_EXIST;
			}
	              #ifdef CONFIG_USER_PPPOE_PROXY
	              if(Entry.cmode==ADSL_PPPoE)
	              {
	               if(Entry.PPPoEProxyEnable) 
				   	strcpy(pppoeProxy,"Enable");
	               else 
				   	strcpy(pppoeProxy,"Disabled");
	             }
	              #endif
		}
		else
		{
			snprintf(ifname, 5, "vc%u", VC_INDEX(Entry.ifIndex));
//			printf("interface=%s\n", ifname);
			
			if (Entry.ipDhcp == (char)DHCP_DISABLED)
			{
				// static IP address
				temp = inet_ntoa(*((struct in_addr *)Entry.ipAddr));
				strcpy(ipAddr, temp);

				temp = inet_ntoa(*((struct in_addr *)Entry.remoteIpAddr));
				strcpy(remoteIp, temp);

				temp = inet_ntoa(*((struct in_addr *)Entry.netMask));	// Jenny, subnet mask
				strcpy(netmask, temp);
#ifdef ZTE_GENERAL_ROUTER_SC
				{
					unsigned int uMask;
					int i;
					
					uMask = *(unsigned int *)Entry.netMask;
					for (i=0; i<32; i++)
						if ((uMask>>i) & 0x01)
							break;
					uMask = 32 - i;

					sprintf(IpMask, "%s/%d", ipAddr, uMask);
				}
#endif
			}
			else
			{
				// DHCP enabled
#if 0
#ifdef EMBED
				if (getInAddr( ifname, IP_ADDR, (void *)&inAddr) == 1)
				{
					temp = inet_ntoa(inAddr);
					strcpy(ipAddr, temp);
				}
				else
#endif
#endif
					strcpy(ipAddr, "");
					strcpy(IpMask, "");
#if 0
#ifdef EMBED
				if (getInAddr( ifname, DST_IP_ADDR, (void *)&inAddr) == 1)
				{
					temp = inet_ntoa(inAddr);
					strcpy(remoteIp, temp);
				}
				else
#endif
#endif
					strcpy(remoteIp, "");
#if 0
#ifdef EMBED
				if (getInAddr( ifname, SUBNET_MASK, (void *)&inAddr) == 1)	// Jenny, subnet mask
				{
					temp = inet_ntoa(inAddr);
					strcpy(netmask, temp);
				}
				else
#endif
#endif
					strcpy(netmask, "");
			}

			if (Entry.ipunnumbered)
			{
				strcpy(ipAddr, "");
				strcpy(IpMask, "");
				strcpy(netmask, "");
				strcpy(remoteIp, "");
			}
			
			if (Entry.cmode == ADSL_BR1483)
			{
				strcpy(ipAddr, "");
				strcpy(IpMask, "");
				strcpy(netmask, "");
				strcpy(remoteIp, "");
				strNapt = "";
				strDroute = "";
			}	
			else if (Entry.cmode == ADSL_RT1483)
				strcpy(netmask, "");
				
			// set status flag
			if (Entry.enable == 0)
			{
				strStatus = (char *)IF_DISABLED;
				conn_status = CONN_DISABLED;
			}
			else
			if (getInFlags( ifname, &flags) == 1)
			{
				if (flags & IFF_UP)
				{
//					strStatus = (char *)IF_UP;
					strStatus = (char *)IF_ENABLE;
					conn_status = CONN_UP;
				}
				else
				{
//					strStatus = (char *)IF_DOWN;
					strStatus = (char *)IF_ENABLE;
					conn_status = CONN_DOWN;
				}
			}
			else
			{
//				strStatus = (char *)IF_NA;
				strStatus = (char *)IF_ENABLE;
				conn_status = CONN_NOT_EXIST;
			}
			
			strcpy(userName, "");
			passwd[0]='\0';
			pppType = BLANK;
		}
		getDisplayWanName(&Entry, if_display_name);
             #ifdef CONFIG_USER_PPPOE_PROXY
	       if(Entry.cmode != ADSL_PPPoE)
	  	 {
	  	     strcpy(pppoeProxy,"----");
	  	  }
	       #endif

#ifdef CTC_WAN_NAME
{
			  
		memset(ctcWanName, 0, sizeof(ctcWanName));
		getWanName(&Entry, ctcWanName);
#if 0
#ifdef _CWMP_MIB_ 
		if(*(Entry.WanName))
			strcpy(ctcWanName, Entry.WanName);
		else
#endif
		{
			switch(Entry.applicationtype)
			{
				//Internet
				case 0:
					strcpy(ctcWanName, "Internet_");
					break;
				//TR069_Internet
				case 1:
					strcpy(ctcWanName, "TR069_Internet_");
					break;

				//TR069
				case 2:
					strcpy(ctcWanName, "TR069_");
					break;
				//Others
				case 3:
					strcpy(ctcWanName, "Other_");
					break;
				default:
					strcpy(ctcWanName, "Internet_");
					break;
			}
			
			if(Entry.cmode == ADSL_BR1483)
				strcat(ctcWanName, "B_");
			else
				strcat(ctcWanName, "R_");
			strcat(ctcWanName, vpi);
			strcat(ctcWanName, "_");
			strcat(ctcWanName, vci);
		//star: for multi-ppp in one pvc
			if(Entry.cmode == ADSL_PPPoE || Entry.cmode == ADSL_PPPoA)
			{	
				char pppindex[6];
				int intindex;
				intindex = getpppindex(&Entry);
				printf("\npppinex=%d\n",intindex);
				if(intindex != -1){
					snprintf(pppindex,6,"%u",intindex);
					strcat(ctcWanName, "_");
					strcat(ctcWanName, pppindex);
				}
			}
		}
#endif		
}	

#endif
#ifdef ZTE_531B_BRIDGE_SC
		nBytesSent += websWrite(wp, T("<tr>"
		"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><input type=\"radio\" name=\"select\""
		" value=\"s%d\" onClick=\"postVC(%s,%s,'%s','%s', %d, %d)\"></td>\n"),
		i,vpi,vci,aal5Encap, mode, conn_status, Entry.enable);
#else
         #ifdef CONFIG_USER_PPPOE_PROXY
	  

 nBytesSent += websWrite (wp, T ("<tr>" 
					     "<td align=center width=\"3%%\" bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=\"2\"><input type=\"radio\" name=\"select\""
					     
					     " value=\"s%d\" onClick=\"postVC(%s,%s,'%s','%s','%s','%s','%s','%s',%d,%d,%d,%d,'%s','%s', '%s', %d, %d, %d)\"></td>\n"),
				      
i, vpi, vci, aal5Encap, strNapt, mode,
				      userName, passwd, pppType,
				      
Entry.pppIdleTime, 
				  	 Entry.PPPoEProxyEnable,
				        Entry.ipunnumbered,
				      Entry.ipDhcp, ipAddr, 
remoteIp,
				      netmask, Entry.dgw, conn_status,
				      Entry.enable);
	#else
		nBytesSent += websWrite(wp, T("<tr>"
		"<td align=center width=\"2%%\" bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=\"2\"><input type=\"radio\" name=\"select\""
		" value=\"s%d\" onClick=\"postVC(%s,%s,'%s','%s','%s','%s','%s','%s',%d,%d,%d,'%s','%s', '%s', %d, %d, %d)\"></td>\n"),
		i,vpi,vci,aal5Encap,strNapt,mode,userName,passwd,pppType,
		Entry.pppIdleTime,Entry.ipunnumbered,Entry.ipDhcp,ipAddr,
		remoteIp, netmask, Entry.dgw, conn_status, Entry.enable);

#endif
#endif

#ifdef CTC_WAN_NAME
#ifdef ZTE_531B_BRIDGE_SC
		nBytesSent += websWrite(wp, T(
		"<td align=center width=\"4%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"7%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"4%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"),
		ctcWanName, mode, vpi, vci, aal5Encap);
#else
		nBytesSent += websWrite(wp, T(
		"<td align=center width=\"14%%\" bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"9%%\" bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=\"2\"><b>%s</b></font></td>\n"
#ifndef ZTE_GENERAL_ROUTER_SC
		"<td align=center width=\"4%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
#else
		"<td align=center width=\"7%%\" bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=\"2\"><b>%s</b></font></td>\n"
#endif
		"<td align=center width=\"7%%\" bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"6%%\" bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"19%%\" bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=\"2\"><b>%s</b></font></td>\n"),
		ctcWanName, mode, 
#ifndef ZTE_GENERAL_ROUTER_SC
		vpi, vci, 
#else
		vpi_vci, 
#endif
		aal5Encap, strNapt, 
#ifndef ZTE_GENERAL_ROUTER_SC
		ipAddr
#else
		IpMask
#endif
		);
#endif
#else
#ifdef ZTE_531B_BRIDGE_SC
		nBytesSent += websWrite(wp, T(
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"),
		ifname, mode, vpi, vci, aal5Encap);
#else
		nBytesSent += websWrite(wp, T(
		"<td align=center width=\"4%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"7%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"4%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"3%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"13%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"),
		if_display_name, mode, vpi, vci, aal5Encap, strNapt, ipAddr);
#endif
#endif
#ifdef ZTE_531B_BRIDGE_SC
		nBytesSent += websWrite(wp, T(
		"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\">"),
		strStatus);
#else
#ifdef DEFAULT_GATEWAY_V1
		nBytesSent += websWrite(wp, T(
		"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=\"2\"><b>%s</b></font></td>\n")
#else
		nBytesSent += websWrite(wp,
#endif
#ifndef ZTE_GENERAL_ROUTER_SC
		T("<td align=center width=\"13%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n")
#endif
		T("<td align=center width=\"9%%\" bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=\"2\"><b>%s</b></font></td>\n")
		
#ifdef DEFAULT_GATEWAY_V1
		T("<td align=center width=\"6%%\" bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=\"2\"><b>%s</b></font></td>\n")
#endif
		T("<td align=center width=\"3%%\" bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"3%%\" bgcolor=\"#C0C0C0\" style=\"word-break:break-all\">"),
#ifdef DEFAULT_GATEWAY_V1
		remoteIp, 
#endif
#ifndef ZTE_GENERAL_ROUTER_SC
		netmask,
#endif
		userName,
		
#ifdef DEFAULT_GATEWAY_V1
		strDroute,
#endif
		strStatus);
#endif
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		nBytesSent += websWrite(wp, T(
		"<a href=\"#?edit\" onClick=\"editClick(%d)\">"
		"<image border=0 src=\"graphics/edit.gif\" alt=\"Edit\" /></a></td></tr>\n"), i);
#else
		nBytesSent += websWrite(wp, T(
		"<a href=\"#?edit\" onClick=\"editClick(%d)\">"
		"<image border=0 src=\"graphics/edit.gif\" alt=\"Edit\" /></a>"), i);

		nBytesSent += websWrite(wp, T(
		"<a href=\"#?delete\" onClick=\"delClick(%d)\">"
		"<image border=0 src=\"graphics/del.gif\" alt=Delete /></td></tr>\n"), i);
#endif
	}
	
	return nBytesSent;
}
#if 0
// add for auto-pvc search
void showPVCList(int eid, webs_t wp, int argc, char_t **argv)
{
	int i, entryNum;
	MIB_AUTO_PVC_SEARCH_T	Entry;
	entryNum = mib_chain_total(MIB_AUTO_PVC_SEARCH_TBL);
	
//	websWrite(wp, T("<table border=2 width=\"500\" cellspacing=4 cellpadding=0><tr>"
	websWrite(wp, T("<tr>"
	"<td bgcolor=\"#808080\"><font size=2><b>PVC</b></td>\n"
 	"<td bgcolor=\"#808080\"><font size=2><b>VPI</b></td>\n"
	"<td bgcolor=\"#808080\"><font size=2><b>VCI</b></td>\n"
// 	"<td bgcolor=\"#808080\"><font size=2><b>QoS</b></td>\n"
//	"<td bgcolor=\"#808080\"><font size=2><b>Frame</b></td>\n"
// 	"<td bgcolor=\"#808080\"><font size=2><b>Status</b></td>\n"
//	"<td bgcolor=\"#808080\"><font size=2><b>Action</b></td></font></tr>\n"));
	"</font></tr>\n"));
	if(autopvc_is_up()) {
		
			for(i=0;i<entryNum;i++)
			{
				if (!mib_chain_get(MIB_AUTO_PVC_SEARCH_TBL, i, (void *)&Entry))
				{
  					websError(wp, 400, T("Get chain record error!\n"));
					break;
				}
		
				websWrite(wp, T("<tr>"
				"<td><b>%d</b></td>\n"
			 	"<td><font size=2><b>%d</b></td>\n"
			 	"<td><font size=2><b>%d</b></td>\n"
//			 	"<td><font size=2><b>ubr</b></td>\n"
//		 		"<td><font size=2><b>br1483</b></td>\n"
//			 	"<td><font size=2><b>active</b></td>\n"
//			 	"<td><input type=\"submit\" value=\"Delete\" name=\"deletePVC\"></td></tr>\n"),
				"</td></tr>\n"),

			 	i, Entry.vpi, Entry.vci);
			}		

	}

}
#endif

void ShowAutoPVC(int eid, webs_t wp, int argc, char_t **argv)
{
//#if AutoPvcSearch
#if defined(AUTO_PVC_SEARCH_TR068_OAMPING) || defined(AUTO_PVC_SEARCH_PURE_OAMPING) || defined(AUTO_PVC_SEARCH_AUTOHUNT)
	int i, entryNum;
	MIB_AUTO_PVC_SEARCH_T	Entry;
	entryNum = mib_chain_total(MIB_AUTO_PVC_SEARCH_TBL);
	
	websWrite(wp, T("<br>"
//	"<input type=\"hidden\" name=\"enablepvc\" value=0>"
	"<input type=\"hidden\" name=\"enablepvc\" value=%d>"
	"<input type=\"hidden\" name=\"addVPI\" value=0>  "
	"<input type=\"hidden\" name=\"addVCI\" value=0>      	"
	"<table>"
	"  <tr>"
	"<td width=\"70%%\" colspan=2><font size=2><b>       "
   	"<input type=\"checkbox\" name=\"autopvc\" value="
   	), autopvc_is_up());
	
	if (autopvc_is_up()) {
	    websWrite(wp, T("\"ON\" checked enabled"));
	    
	} else {
	    websWrite(wp, T("\"OFF\" enabled"));
	}

//#if defined(AUTO_PVC_SEARCH_PURE_OAMPING) || defined(AUTO_PVC_SEARCH_AUTOHUNT)
#ifdef AUTO_PVC_SEARCH_TABLE
	websWrite(wp, T(
	" ONCLICK=updatepvcState() >&nbsp;&nbsp;Enable Auto-PVC Search</b>"
     	"</td>")
//#endif

//#if defined(AUTO_PVC_SEARCH_TR068_OAMPING)
#else
	websWrite(wp, T(
	" ONCLICK=updatepvcState2() >&nbsp;&nbsp;Enable Auto-PVC Search</b>"
     	"</td>")
#endif
     	);     	
   	
   	websWrite(wp, T(
     	"<td width=\"30%%\" colspan=2><font size=2><b>"
   	"<input type=\"submit\" name=\"autopvcapply\" value= \"Apply\" >"
	"</td>"     
	"</tr>")
	
//#if defined(AUTO_PVC_SEARCH_PURE_OAMPING) || defined(AUTO_PVC_SEARCH_AUTOHUNT)
#ifdef AUTO_PVC_SEARCH_TABLE
	T("<tr>"
	"<td><font size=2><b>VPI: </b>"
	"<input type=\"text\" name=\"autopvcvpi\" size=\"4\" maxlength=\"3\" value=0>&nbsp;&nbsp;"
	"</td>"
	
	"<td><b>VCI: </b>"
	"<input type=\"text\" name=\"autopvcvci\" size=\"6\" maxlength=\"5\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
	"</td>"
	
	"<td>"
	"<input type=\"submit\" value=\"Add\" name=\"autopvcadd\" onClick=\"return autopvcCheckClick()\">			"
        "</td>"
        
	"<td>"
	"<input type=\"submit\" value=\"Delete\" name=\"autopvcdel\" onClick=\"return autopvcCheckClick()\">			"
        "</td>"      
	"</tr>")
#endif	
	T("</table>")
	
//#if defined(AUTO_PVC_SEARCH_PURE_OAMPING) || defined(AUTO_PVC_SEARCH_AUTOHUNT)
#ifdef AUTO_PVC_SEARCH_TABLE
	T("<br>  "
	"<table border=\"1\" width=\"30%%\">"
	"<tr><font size=2><b>Current Auto-PVC Table:</b></font></tr> ")
#endif
	);

//#if defined(AUTO_PVC_SEARCH_PURE_OAMPING) || defined(AUTO_PVC_SEARCH_AUTOHUNT)
#ifdef AUTO_PVC_SEARCH_TABLE
	websWrite(wp, T("<tr>"
	"<td bgcolor=\"#808080\"><font size=2><b>PVC</b></td>\n"
 	"<td bgcolor=\"#808080\"><font size=2><b>VPI</b></td>\n"
	"<td bgcolor=\"#808080\"><font size=2><b>VCI</b></td>\n"
	"</font></tr>\n"));
	if(autopvc_is_up()) {
		
			for(i=0;i<entryNum;i++)
			{
				if (!mib_chain_get(MIB_AUTO_PVC_SEARCH_TBL, i, (void *)&Entry))
				{
  					websError(wp, 400, T("Get chain record error!\n"));
					break;
				}
		
				websWrite(wp, T("<tr>"
				"<td><b>%d</b></td>\n"
			 	"<td><font size=2><b>%d</b></td>\n"
			 	"<td><font size=2><b>%d</b></td>\n"
				"</td></tr>\n"),

			 	i, Entry.vpi, Entry.vci);
			}		

	}
	
	websWrite(wp, T("</table>"));
#endif
#endif
}

//jim for China telecom...
#ifdef ZTE_GENERAL_ROUTER_SC
void ShowApplicationMode(int eid, webs_t wp, int argc, char_t **argv)
{
#ifdef CTC_WAN_NAME

	websWrite(wp, T("<font size=2>"
	" <b>应用模式:</b>"
	"<select size=\"1\" name=\"adslApplicationMode\" onChange=\"adslApplicationModeSelection()\">"
	"	  <option selected value=\"0\">Internet</option>"
	"</select>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
	));

#endif	
}
#else
void ShowApplicationMode(int eid, webs_t wp, int argc, char_t **argv)
{
#ifdef CTC_WAN_NAME

	websWrite(wp, T("<font size=2>"
	" <b>Application Mode:</b>"
	"<select size=\"1\" name=\"adslApplicationMode\" onChange=\"adslApplicationModeSelection()\">"
	"	  <option selected value=\"0\">Internet</option>"
	"	  <option value=\"1\">TR069_Internet</option>"
	"	  <option value=\"2\">TR069</option>"
	"	  <option value=\"3\">Other</option>"
	"</select>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
	));

#endif	
}
#endif

#ifdef ZTE_531B_BRIDGE_SC
void ShowChannelMode(int eid, webs_t wp, int argc, char_t **argv)
{
	websWrite(wp, T("<font size=2>"
	" <b>信道模式:</b> 1483 Bridged"
	"<select  size=\"1\" name=\"adslConnectionMode\" style=\"visibility:hidden\">\n"
	"	  <option selected value=\"0\"></option>\n"
	));
//	checkWrite(eid, wp, argc, argv);
	websWrite(wp, T("</select>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n"));
}
#else 
#ifdef ZTE_GENERAL_ROUTER_SC
void ShowChannelMode(int eid, webs_t wp, int argc, char_t **argv)
{
#ifdef BRIDGE_ONLY_ON_WEB

	websWrite(wp, T("<font size=2>"
	" <b>Channel Mode:</b>"
	"<select size=\"1\" name=\"adslConnectionMode\" >\n"
	"	  <option selected value=\"0\">1483 Bridged</option>\n"
	));
//	checkWrite(eid, wp, argc, argv);
	websWrite(wp, T("</select>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n"));
#else
	websWrite(wp, T("<font size=2>"
	" <b>信道模式:</b>"
	"<select size=\"1\" name=\"adslConnectionMode\" onChange=\"adslConnectionModeSelection()\">\n"
	"	  <option selected value=\"0\">1483 Bridged</option>\n"
	"	  <option value=\"1\">1483 MER</option>\n"
	"	  <option value=\"2\">PPPoE</option>\n"
	"	  <option value=\"3\">PPPoA</option>\n"	
	"	  <option value=\"4\">1483 Routed(IPOA)</option>\n"
	));
	checkWrite(eid, wp, argc, argv);
	websWrite(wp, T("</select>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n"));
#endif	
}
#else
//jim for China telecom...
void ShowChannelMode(int eid, webs_t wp, int argc, char_t **argv)
{
#ifdef BRIDGE_ONLY_ON_WEB

	websWrite(wp, T("<font size=2>"
	" <b>Channel Mode:</b>"
	"<select size=\"1\" name=\"adslConnectionMode\" >\n"
	"	  <option selected value=\"0\">1483 Bridged</option>\n"
	));
//	checkWrite(eid, wp, argc, argv);
	websWrite(wp, T("</select>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n"));
#else
	websWrite(wp, T("<font size=2>"
	" <b>Channel Mode:</b>"
	"<select size=\"1\" name=\"adslConnectionMode\" onChange=\"adslConnectionModeSelection()\">\n"
	"	  <option selected value=\"0\">1483 Bridged</option>\n"
	"	  <option value=\"1\">1483 MER</option>\n"
	"	  <option value=\"2\">PPPoE</option>\n"
	"	  <option value=\"3\">PPPoA</option>\n"	
	"	  <option value=\"4\">1483 Routed</option>\n"
	));
	checkWrite(eid, wp, argc, argv);
	websWrite(wp, T("</select>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</font>\n"));
#endif	
}
#endif
#endif

#ifdef ZTE_GENERAL_ROUTER_SC
void ShowNAPTSetting(int eid, webs_t wp, int argc, char_t **argv)
{
#ifdef BRIDGE_ONLY_ON_WEB
	websWrite(wp, T("<input type=\"hidden\"  name=\"naptEnabled\">\n"));
	return;
#else
	websWrite(wp, T("<b>允许NAPT: </b><input type=\"checkbox\" name=\"naptEnabled\"\n"
			"size=\"2\" maxlength=\"2\" value=\"ON\" onClick=naptClicked()>"));
#endif
}
#else
void ShowNAPTSetting(int eid, webs_t wp, int argc, char_t **argv)
{
#ifdef BRIDGE_ONLY_ON_WEB
	websWrite(wp, T("<input type=\"hidden\"  name=\"naptEnabled\">\n"));
	return;
#else
	websWrite(wp, T("<font size=2><b>Enable NAPT: </b><input type=\"checkbox\" name=\"naptEnabled\"\n"
			"size=\"2\" maxlength=\"2\" value=\"ON\" onClick=naptClicked()>"));
#endif
}
#endif

#ifdef ZTE_GENERAL_ROUTER_SC
void ShowPPPIPSettings(int eid, webs_t wp, int argc, char_t **argv)
{
#ifdef BRIDGE_ONLY_ON_WEB
websWrite(wp, T("<input type=\"hidden\"  name=\"pppUserName\">\n"));
websWrite(wp, T("<input type=\"hidden\"  name=\"pppPassword\">\n"));
websWrite(wp, T("<input type=\"hidden\"  name=\"pppConnectType\">\n"));
websWrite(wp, T("<input type=\"hidden\"  name=\"pppIdleTime\">\n"));

websWrite(wp, T("<input type=\"hidden\"  name=\"ipMode\">\n"));
websWrite(wp, T("<input type=\"hidden\"  name=\"ipMode\">\n"));
websWrite(wp, T("<input type=\"hidden\"  name=\"ip\">\n"));
websWrite(wp, T("<input type=\"hidden\"  name=\"remoteIp\">\n"));
websWrite(wp, T("<input type=\"hidden\"  name=\"netmask\">\n"));
websWrite(wp, T("<input type=\"hidden\"  name=\"ipUnnum\">\n"));
websWrite(wp, T("<input type=\"hidden\"  name=\"droute\">\n"));
websWrite(wp, T("<input type=\"hidden\"  name=\"droute\">\n"));
#else
	websWrite(wp, T("<table border=0 width=\"600\" cellspacing=4 cellpadding=0>\n"
"<tr><td colspan=5><hr size=2 align=top></td></tr>\n"
"<tr><th align=\"left\"><font size=2><b>PPP设置:</b></th>\n"
"	<td><font size=2><b>用户名:</b></td>\n"
"	<td><font size=2><input type=\"text\" name=\"pppUserName\" size=\"10\" maxlength=\"20\"></td>\n"
"	<td><font size=2><b>密码:</b></td>\n"
"	<td><font size=2><input type=\"password\" name=\"pppPassword\" size=\"10\" maxlength=\20\"></td>\n"
"</tr>\n"
"<tr><th></th>\n"
"	<td><font size=2><b>类型:</b></td>\n"
"	<td><font size=2><select size=\"1\" name=\"pppConnectType\" onChange=\"pppTypeSelection()\">\n"
"		<option selected value=\"0\">始终在线</option>\n"
"		<option value=\"1\">按需连接</option>\n"
"		<option value=\"2\">手动连接</option>\n"
"		</select>\n"
"	</td>\n"
"	<td><font size=2><b>空闲时间(分):</b></td>\n"
"	<td><font size=2><input type=\"text\" name=\"pppIdleTime\" size=\"10\" maxlength=\"3\"></td>\n"));
        checkWrite(eid, wp, argc, argv);
	
	websWrite(wp, T("</tr>\n"
"<tr><td colspan=5><hr size=2 align=top></td></tr>\n"
"<tr><th align=\"left\"><font size=2><b>WAN端IP设置:</b></th>\n"
"\n"
"	<td><font size=2><b>类型:</b></td>\n"
"	<td><font size=2>\n"
"	<input type=\"radio\" value=\"0\" name=\"ipMode\" checked onClick=\"ipTypeSelection()\">固定IP</td>\n"
"	<td><font size=2>\n"
"	<input type=\"radio\" value=\"1\" name=\"ipMode\" onClick=\"ipTypeSelection()\">DHCP分配</td>\n"
"</tr>\n"
"<tr><th></th>\n"
"	<td><font size=2><b>本地IP地址:</b></td>\n"
"	<td><font size=2><input type=\"text\" name=\"ip\" size=\"10\" maxlength=\"15\"></td>\n"
"	<td><font size=2><b>网关IP地址:</b></td>\n"
"	<td><font size=2><input type=\"text\" name=\"remoteIp\" size=\"10\" maxlength=\"15\"></td>\n"
"</tr>\n"
"<tr><th></th>\n"
"	<td><font size=2><b>子网掩码:</b></td>\n"
"	<td><font size=2><input type=\"text\" name=\"netmask\" size=\"10\" maxlength=\"15\"></td>\n"
"	<td><font size=2><b>Unnumbered</b>\n"
"		<input type=\"checkbox\" name=\"ipUnnum\" size=\"2\" maxlength=\"2\" value=\"ON\"  onClick=\"ipModeSelection()\"></td>\n"
"</tr>\n"
"<tr><th></th>\n"));

	websWrite(wp, T("	<td><font size=2><b>默认路由:</b></td>\n"
"	<td><font size=2><input type=radio value=0 name=\"droute\">禁止</td>\n"
"	<td><font size=2><input type=radio value=1 name=\"droute\" checked>允许&nbsp;&nbsp;</td>\n"
"</tr>\n"
"</table>\n"));
#endif
}

#else

void ShowPPPIPSettings(int eid, webs_t wp, int argc, char_t **argv)
{
#ifdef BRIDGE_ONLY_ON_WEB
websWrite(wp, T("<input type=\"hidden\"  name=\"pppUserName\">\n"));
websWrite(wp, T("<input type=\"hidden\"  name=\"pppPassword\">\n"));
websWrite(wp, T("<input type=\"hidden\"  name=\"pppConnectType\">\n"));
websWrite(wp, T("<input type=\"hidden\"  name=\"pppIdleTime\">\n"));

websWrite(wp, T("<input type=\"hidden\"  name=\"ipMode\">\n"));
websWrite(wp, T("<input type=\"hidden\"  name=\"ipMode\">\n"));
websWrite(wp, T("<input type=\"hidden\"  name=\"ip\">\n"));
websWrite(wp, T("<input type=\"hidden\"  name=\"remoteIp\">\n"));
websWrite(wp, T("<input type=\"hidden\"  name=\"netmask\">\n"));
websWrite(wp, T("<input type=\"hidden\"  name=\"ipUnnum\">\n"));
websWrite(wp, T("<input type=\"hidden\"  name=\"droute\">\n"));
websWrite(wp, T("<input type=\"hidden\"  name=\"droute\">\n"));
#else
	websWrite(wp, T("<table border=0 width=\"600\" cellspacing=4 cellpadding=0>\n"
"<tr><td colspan=5><hr size=2 align=top></td></tr>\n"
"<tr><th align=\"left\"><font size=2><b>PPP Settings:</b></th>\n"
"	<td><font size=2><b>User Name:</b></td>\n"
"	<td><font size=2><input type=\"text\" name=\"pppUserName\" size=\"16\" maxlength=\"%d\"></td>\n"
"	<td><font size=2><b>Password:</b></td>\n"
"	<td><font size=2><input type=\"password\" name=\"pppPassword\" size=\"10\" maxlength=\"%d\"></td>\n"
"</tr>\n"
"<tr><th></th>\n"
"	<td><font size=2><b>Type:</b></td>\n"
"	<td><font size=2><select size=\"1\" name=\"pppConnectType\" onChange=\"pppTypeSelection()\">\n"
"		<option selected value=\"0\">Continuous</option>\n"
"		<option value=\"1\">Connect on Demand</option>\n"
"		<option value=\"2\">Manual</option>\n"
"		</select>\n"
"	</td>\n"
"	<td><font size=2><b>Idle Time (min):</b></td>\n"
"	<td><font size=2><input type=\"text\" name=\"pppIdleTime\" size=\"10\" maxlength=\"3\"></td>\n"),
#ifdef CONFIG_GUI_WEB
	50, 50
#else
	MAX_PPP_NAME_LEN, MAX_NAME_LEN-1
#endif
	);
        checkWrite(eid, wp, argc, argv);
	
	websWrite(wp, T("</tr>\n"
"<tr><td colspan=5><hr size=2 align=top></td></tr>\n"
"<tr><th align=\"left\"><font size=2><b>WAN IP Settings:</b></th>\n"
"\n"
"	<td><font size=2><b>Type:</b></td>\n"
"	<td><font size=2>\n"
"	<input type=\"radio\" value=\"0\" name=\"ipMode\" checked onClick=\"ipTypeSelection()\">Fixed IP\n"
"	<font size=2>\n"
"	<input type=\"radio\" value=\"1\" name=\"ipMode\" onClick=\"ipTypeSelection()\">DHCP</td>\n"
#ifdef DEFAULT_GATEWAY_V2
"	<td><font size=2><b>Unnumbered</b>\n"
"		<input type=\"checkbox\" name=\"ipUnnum\" size=\"2\" maxlength=\"2\" value=\"ON\"  onClick=\"ipModeSelection()\"></td>\n"
#endif
"</tr>\n"
"<tr><th></th>\n"
"	<td><font size=2><b>Local IP Address:</b></td>\n"
"	<td><font size=2><input type=\"text\" name=\"ip\" size=\"10\" maxlength=\"15\"></td>\n"
#ifdef DEFAULT_GATEWAY_V1
"	<td><font size=2><b>Remote IP Address:</b></td>\n"
"	<td><font size=2><input type=\"text\" name=\"remoteIp\" size=\"10\" maxlength=\"15\"></td>\n"
"</tr>\n"
"<tr><th></th>\n"
#endif
"	<td><font size=2><b>Subnet Mask:</b></td>\n"
"	<td><font size=2><input type=\"text\" name=\"netmask\" size=\"10\" maxlength=\"15\"></td>\n"
#ifdef DEFAULT_GATEWAY_V1
"	<td><font size=2><b>Unnumbered</b>\n"
"		<input type=\"checkbox\" name=\"ipUnnum\" size=\"2\" maxlength=\"2\" value=\"ON\"  onClick=\"ipModeSelection()\"></td>\n"
#endif
"</tr>\n"
"<tr><th></th>\n"));
#ifdef DEFAULT_GATEWAY_V1
	websWrite(wp, T("	<td><font size=2><b>Default Route:</b></td>\n"
"	<td><font size=2><input type=radio value=0 name=\"droute\">Disable</td>\n"
"	<td><font size=2><input type=radio value=1 name=\"droute\" checked>Enable&nbsp;&nbsp;</td>\n"));
//#else
//	websWrite(wp, T("<input type=\"hidden\"  name=\"droute\">\n"));
//	websWrite(wp, T("<input type=\"hidden\"  name=\"droute\">\n"));
//#endif
	websWrite(wp, T("	</tr>\n"
"</table>\n"));
#endif
#endif
}

void ShowDefaultGateway(int eid, webs_t wp, int argc, char_t **argv)
{
#ifdef DEFAULT_GATEWAY_V2
	websWrite(wp, T("	<td colspan=4><input type=\"radio\" name=\"droute\" value=\"1\" onClick='autoDGWclicked()'>"
	"<font size=2><b>&nbsp;&nbsp;Obtain default gateway automatically</b></td>\n</tr>\n"
	"<tr><th></th>\n	<td colspan=4><input type=\"radio\" name=\"droute\" value=\"0\" onClick='autoDGWclicked()'>"
	"<font size=2><b>&nbsp;&nbsp;Use the following default gateway:</b></td>\n</tr>\n"));
	websWrite(wp, T("<tbody id='gwInfo'>\n"
	"<tr><th></th>\n	<td>&nbsp;</td>\n"
	"	<td colspan=2><font size=2><input type=\"radio\" name='gwStr' value=\"0\" onClick='gwStrClick()'><b>&nbsp;Use Remote WAN IP Address:&nbsp;&nbsp;</b></td>\n"
	"	<td><div id='id_dfltgwy'><font size=2><input type='text' name='dstGtwy' maxlength=\"15\" size=\"10\"></div></td>\n</tr>\n"
	"<tr><th></th>\n	<td>&nbsp;</td>\n"
	"	<td colspan=2><font size=2><input type=\"radio\" name='gwStr' value=\"1\" onClick='gwStrClick()'><b>&nbsp;Use WAN Interface:&nbsp;&nbsp;</b></td>\n"
	"	<td><div id='id_wanIf'><font size=2><select name='wanIf'>"));
	ifwanList(eid, wp, argc, argv);
	websWrite(wp, T("</select></div></td>\n</tr>\n</tbody>\n</table>\n"));
	websWrite(wp, T("<input type=\"hidden\"  name=\"remoteIp\">\n"));
#else
	websWrite(wp, T("<tbody id='gwInfo'>\n"));
	websWrite(wp, T("<input type=\"hidden\"  name=\"gwStr\">\n"));
	websWrite(wp, T("<div id='id_dfltgwy'>\n"));
	websWrite(wp, T("<input type=\"hidden\"  name=\"dstGtwy\"></div>\n"));
	websWrite(wp, T("<input type=\"hidden\"  name=\"gwStr\">\n"));
	websWrite(wp, T("<div id='id_wanIf'>\n"));
	websWrite(wp, T("<input type=\"hidden\"  name=\"wanIf\"></div>\n</tbody>\n"));
#endif
}

#endif
#if 0
int autopvcStatus(int eid, webs_t wp, int argc, char_t **argv)
{

	if (autopvc_is_up()) {
	    websWrite(wp, T("\"ON\" checked enabled"));
	    
	} else {
	    websWrite(wp, T("\"OFF\" enabled"));
	}
	return 0;

}
#endif

