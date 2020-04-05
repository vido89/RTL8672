/*
 *      Include file of form handler
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *      Authors: Dick Tam	<dicktam@realtek.com.tw>
 *
 */


#ifndef _INCLUDE_WEBFORM_H
#define _INCLUDE_WEBFORM_H

#include <stdlib.h>
#include "../globals.h"
#if HAVE_STDBOOL_H
# include <stdbool.h>
#else
typedef enum {false = 0, true = 1} bool;
#endif

#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../uClibc/include/linux/autoconf.h"
#endif
#include "options.h"
#include "mib.h"

#include "../defs.h"

#ifdef __i386__
  #define _CONFIG_SCRIPT_PATH	T(".")
  #define _LITTLE_ENDIAN_
#else
  #define _CONFIG_SCRIPT_PATH	T("/bin")
#endif

#define char_t char
#define webs_t request *
#define	T(s)	s
//#define _UPMIBFILE_SCRIPT_PROG  T("mibfile_run.sh")
#define _CONFIG_SCRIPT_PROG	T("init.sh")
#define _WLAN_SCRIPT_PROG	T("wlan.sh")
#define _PPPOE_SCRIPT_PROG	T("pppoe.sh")
#define _FIREWALL_SCRIPT_PROG	T("firewall.sh")
#define _PPPOE_DC_SCRIPT_PROG	T("disconnect.sh")

///////////////////////////////////////////////////////////////////////////
static bool _is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}

// Validate digit
static bool _isdigit(char c)
{
    return ((c >= '0') && (c <= '9'));
}

static int __inline__ string_to_hex(char_t *string, unsigned char *key, int len)
{
	char tmpBuf[4];
	int idx, ii=0;
	for (idx=0; idx<len; idx+=2) {
		tmpBuf[0] = string[idx];
		tmpBuf[1] = string[idx+1];
		tmpBuf[2] = 0;
		if ( !_is_hex(tmpBuf[0]) || !_is_hex(tmpBuf[1]))
			return 0;

		key[ii++] = (unsigned char) strtol(tmpBuf, (char**)NULL, 16);
	}
	return 1;
}

static int __inline__ string_to_dec(char_t *string, int *val)
{
	int idx;
	int len = strlen(string);

	for (idx=0; idx<len; idx++) {
		if ( !_isdigit(string[idx]))
			return 0;
	}

	*val = strtol(string, (char**)NULL, 10);
	return 1;
}

//added by xl_yue
#ifdef USE_LOGINWEB_OF_SERVER
#define ERR_MSG2(msg) { \
	websHeader(wp); \
	websWrite(wp, T("<body><blockquote><form>\n")); \
	websWrite(wp, T("<TABLE width=\"100%%\">\n")); \
	websWrite(wp, T("<TR><TD align = middle><h4>%s</h4></TD></TR>\n"),msg); \
	websWrite(wp, T("<TR><TD align = middle><input type=\"button\" onclick=\"history.go (-1)\" value=\"&nbsp;&nbsp;OK&nbsp;&nbsp\" name=\"OK\"></TD></TR>")); \
	websWrite(wp, T("</TABLE></form></blockquote></body>\n")); \
	websFooter(wp); \
	websDone(wp, 200); \
}
#endif

#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
#define ERR_MSG(msg) { \
	websHeader(wp); \
   	websWrite(wp, T("<body><blockquote><h4>%s</h4>\n"), msg); \
	websWrite(wp, T("<form><input type=\"button\" onclick=\"history.go (-1)\" value=\"&nbsp;&nbsp;确定&nbsp;&nbsp\" name=\"OK\"></form></blockquote></body>")); \
   	websFooter(wp); \
	websDone(wp, 200); \
}

#define OK_MSG(url) { \
	websHeader(wp); \
   	websWrite(wp, T("<body><blockquote><h4>设定生效!</h4>\n")); \
	if (url[0]) websWrite(wp, T("<form><input type=button value=\"  确定  \" OnClick=window.location.replace(\"%s\")></form></blockquote></body>"), url);\
	else websWrite(wp, T("<form><input type=button value=\"  确定  \" OnClick=window.close()></form></blockquote></body>"));\
   	websFooter(wp); \
	websDone(wp, 200); \
}

#define OK_MSG1(msg, url) { \
	websHeader(wp); \
   	websWrite(wp, T("<body><blockquote><h4>%s</h4>\n"), msg); \
	if (url) websWrite(wp, T("<form><input type=button value=\"  确定  \" OnClick=window.location.replace(\"%s\")></form></blockquote></body>"), url);\
	else websWrite(wp, T("<form><input type=button value=\"  确定  \" OnClick=window.close()></form></blockquote></body>"));\
   	websFooter(wp); \
	websDone(wp, 200); \
}
#else
#define ERR_MSG(msg) { \
	websHeader(wp); \
   	websWrite(wp, T("<body><blockquote><h4>%s</h4>\n"), msg); \
	websWrite(wp, T("<form><input type=\"button\" onclick=\"history.go (-1)\" value=\"&nbsp;&nbsp;OK&nbsp;&nbsp\" name=\"OK\"></form></blockquote></body>")); \
   	websFooter(wp); \
	websDone(wp, 200); \
}

#define OK_MSG(url) { \
	websHeader(wp); \
   	websWrite(wp, T("<body><blockquote><h4>Change setting successfully!</h4>\n")); \
	if (url[0]) websWrite(wp, T("<form><input type=button value=\"  OK  \" OnClick=window.location.replace(\"%s\")></form></blockquote></body>"), url);\
	else websWrite(wp, T("<form><input type=button value=\"  OK  \" OnClick=window.close()></form></blockquote></body>"));\
   	websFooter(wp); \
	websDone(wp, 200); \
}

#define OK_MSG1(msg, url) { \
	websHeader(wp); \
   	websWrite(wp, T("<body><blockquote><h4>%s</h4>\n"), msg); \
	if (url) websWrite(wp, T("<form><input type=button value=\"  OK  \" OnClick=window.location.replace(\"%s\")></form></blockquote></body>"), url);\
	else websWrite(wp, T("<form><input type=button value=\"  OK  \" OnClick=window.close()></form></blockquote></body>"));\
   	websFooter(wp); \
	websDone(wp, 200); \
}
#endif

/* Routines exported in fmget.c */
// Kaohj
extern int checkWrite(int eid, webs_t wp, int argc, char_t **argv);
extern int initPage(int eid, webs_t wp, int argc, char_t **argv);
extern void write_wladvanced(int eid, webs_t wp, int argc, char_t **argv);	
#ifdef WLAN_QoS
extern void ShowWmm(int eid, webs_t wp, int argc, char_t **argv);
#endif
//extern int getIndex(int eid, webs_t wp, int argc, char_t **argv);
extern int getInfo(int eid, webs_t wp, int argc, char_t **argv);
extern int getNameServer(int eid, webs_t wp, int argc, char_t **argv);
extern int getDefaultGW(int eid, webs_t wp, int argc, char_t **argv);

extern int isConnectPPP(void);
extern int srvMenu(int eid, webs_t wp, int argc, char_t **argv);

/* Routines exported in fmtcpip.c */
extern void formTcpipLanSetup(webs_t wp, char_t *path, char_t *query);
extern int lan_setting(int eid, webs_t wp, int argc, char_t **argv);
extern int checkIP2(int eid, webs_t wp, int argc, char_t **argv);
extern int lan_script(int eid, webs_t wp, int argc, char_t **argv);
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
extern void formRefresh(webs_t wp, char_t *path, char_t *query);
#endif
#ifdef WEB_REDIRECT_BY_MAC
void formLanding(webs_t wp, char_t *path, char_t *query);
#endif
//#ifdef _CWMP_MIB_
extern void formTR069Config(webs_t wp, char_t *path, char_t *query);
extern void formTR069CPECert(webs_t wp, char_t *path, char_t *query);
extern void formTR069CACert(webs_t wp, char_t *path, char_t *query);
extern int TR069ConPageShow(int eid, webs_t wp, int argc, char_t **argv);
extern int portFwTR069(int eid, webs_t wp, int argc, char_t **argv);
//#endif

/* Routines exported in fmfwall.c */
extern void formPortFw(webs_t wp, char_t *path, char_t *query);
#ifdef NATIP_FORWARDING
extern void formIPFw(webs_t wp, char_t *path, char_t *query);
#endif
#ifdef PORT_TRIGGERING
extern void formGaming(webs_t wp, char_t *path, char_t *query);
#endif
extern void formFilter(webs_t wp, char_t *path, char_t *query);
#ifdef ZTE_GENERAL_ROUTER_SC
extern void formVrtsrv(webs_t wp, char_t *path, char_t *query);
extern void formAddVrtsrv(webs_t wp, char_t *path, char_t *query);
#endif
#ifdef LAYER7_FILTER_SUPPORT
extern int AppFilterList(int eid, webs_t wp, int argc, char_t **argv);
extern void formLayer7(webs_t wp, char_t *path, char_t *query);
#endif
#ifdef PARENTAL_CTRL
extern void formParentCtrl(webs_t wp, char_t *path, char_t *query);
extern int parentalCtrlList(int eid, webs_t wp, int argc, char_t **argv);
#endif
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
extern void formFastConf(webs_t wp, char_t *path, char_t *query);
extern void formFastConf2(webs_t wp, char_t *path, char_t *query);
extern void formFastConf3(webs_t wp, char_t *path, char_t *query);
extern void formFastConf4(webs_t wp, char_t *path, char_t *query);
//extern void formFastConfEnd(webs_t wp, char_t *path, char_t *query);
//extern void showFastConfP1(int eid, webs_t wp, int argc, char_t **argv);	// auto-pvc-search
extern void formFcPppWan(webs_t wp, char_t *path, char_t *query);
extern void formFcPppType(webs_t wp, char_t *path, char_t *query);
extern void formFcMerWan(webs_t wp, char_t *path, char_t *query);
extern void formFcIPoA(webs_t wp, char_t *path, char_t *query);
#endif
extern void formDMZ(webs_t wp, char_t *path, char_t *query);
extern int portFwList(int eid, webs_t wp, int argc, char_t **argv);
#ifdef NATIP_FORWARDING
extern int ipFwList(int eid, webs_t wp, int argc, char_t **argv);
#endif
extern int ipPortFilterList(int eid, webs_t wp, int argc, char_t **argv);
#ifdef MAC_FILTER
extern int macFilterList(int eid, webs_t wp, int argc, char_t **argv);
#endif

/* Routines exported in fmmgmt.c */
extern void formPasswordSetup(webs_t wp, char_t *path, char_t *query);
extern void formUserPasswordSetup(webs_t wp, char_t *path, char_t *query);
#ifdef ACCOUNT_CONFIG
extern void formAccountConfig(webs_t wp, char_t *path, char_t *query);
extern int accountList(int eid, webs_t wp, int argc, char_t **argv);
#endif
#ifdef WEB_UPGRADE
extern void formUpload(webs_t wp, char_t * path, char_t * query);
#endif
extern void formSaveConfig(webs_t wp, char_t *path, char_t *query);
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
extern void formSnmpConfig(webs_t wp, char_t *path, char_t *query);
#endif
extern void formAdslDrv(webs_t wp, char_t *path, char_t *query);
extern void formSetAdsl(webs_t wp, char_t *path, char_t *query);
extern void formDiagAdsl(webs_t wp, char_t *path, char_t *query);
extern void formSetAdslTone(webs_t wp, char_t *path, char_t *query);
extern void formStatAdsl(webs_t wp, char_t *path, char_t *query);
extern void formStats(webs_t wp, char_t *path, char_t *query);
extern void formRconfig(webs_t wp, char_t *path, char_t *query);
extern void formSysCmd(webs_t wp, char_t *path, char_t *query);
extern int sysCmdLog(int eid, webs_t wp, int argc, char_t **argv);
#ifdef SYSLOG_SUPPORT
extern void formSysLog(webs_t wp, char_t *path, char_t *query);
extern int sysLogList(int eid, webs_t wp, int argc, char_t **argv);
extern void RemoteSyslog(int eid, webs_t wp, int argc, char_t **argv);
#endif
#ifdef DOS_SUPPORT
extern void formDosCfg(webs_t wp, char_t *path, char_t *query);
#endif
#if 0
extern int adslDrvSnrTblGraph(int eid, webs_t wp, int argc, char_t **argv);
extern int adslDrvSnrTblList(int eid, webs_t wp, int argc, char_t **argv);
extern int adslDrvBitloadTblGraph(int eid, webs_t wp, int argc, char_t **argv);
extern int adslDrvBitloadTblList(int eid, webs_t wp, int argc, char_t **argv);
#endif
extern int adslToneDiagTbl(int eid, webs_t wp, int argc, char_t **argv);
extern int adslToneDiagList(int eid, webs_t wp, int argc, char_t **argv);
extern int adslToneConfDiagList(int eid, webs_t wp, int argc, char_t **argv);
extern int memDump(int eid, webs_t wp, int argc, char_t **argv);
extern int pktStatsList(int eid, webs_t wp, int argc, char_t **argv);


/* Routines exported in fmatm.c */
extern int atmVcList(int eid, webs_t wp, int argc, char_t **argv);
extern int atmVcList2(int eid, webs_t wp, int argc, char_t **argv);
extern int wanConfList(int eid, webs_t wp, int argc, char_t **argv);
extern void formAtmVc(webs_t wp, char_t *path, char_t *query);
extern void formAdsl(webs_t wp, char_t *path, char_t *query);
extern void formPPPEdit(webs_t wp, char_t *path, char_t *query);
extern void formIPEdit(webs_t wp, char_t *path, char_t *query);
extern void formBrEdit(webs_t wp, char_t *path, char_t *query);
extern void formStatus(webs_t wp, char_t *path, char_t *query);
extern void formDate(webs_t wp, char_t *path, char_t *query);
#ifdef ZTE_GENERAL_ROUTER_SC
extern void formTimezone(webs_t wp, char_t *path, char_t *query);
#endif
/* Routines exported in fmbridge.c */
extern void formBridge(webs_t wp, char_t *path, char_t *query);
extern void formRefleshFdbTbl(webs_t wp, char_t *path, char_t *query);
extern int bridgeFdbList(int eid, webs_t wp, int argc, char_t **argv);
extern int virtualSvrList(int eid, webs_t wp, int argc, char_t **argv);
extern int ARPTableList(int eid, webs_t wp, int argc, char_t **argv);

/* Routines exported in fmroute.c */
extern void formRoute(webs_t wp, char_t *path, char_t *query);
#if defined(CONFIG_USER_ROUTED_ROUTED) || defined(CONFIG_USER_ZEBRA_OSPFD_OSPFD)
extern void formRip(webs_t wp, char_t *path, char_t *query);
#endif
#ifdef CONFIG_USER_ROUTED_ROUTED
extern int showRipIf(int eid, webs_t wp, int argc, char_t **argv);
#endif
#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
extern int showOspfIf(int eid, webs_t wp, int argc, char_t **argv);
#endif
extern void ShowDefaultGateway(int eid, webs_t wp, int argc, char_t **argv);
extern void ShowRouteInf(int eid, webs_t wp, int argc, char_t **argv);
extern void GetDefaultGateway(int eid, webs_t wp, int argc, char_t **argv);
extern int showStaticRoute(int eid, webs_t wp, int argc, char_t **argv);
extern void formRefleshRouteTbl(webs_t wp, char_t *path, char_t *query);
extern int routeList(int eid, webs_t wp, int argc, char_t **argv);

/* Routines exported in fmdhcpd.c */
extern void formDhcpd(webs_t wp, char_t *path, char_t *query);
extern void formReflashClientTbl(webs_t wp, char_t *path, char_t *query);
extern int dhcpClientList(int eid, webs_t wp, int argc, char_t **argv);

/* Routines exported in fmDNS.c */
extern void formDns(webs_t wp, char_t *path, char_t *query);

/* Routines exported in fmDDNS.c */
extern void formDDNS(webs_t wp, char_t *path, char_t *query);
extern int showDNSTable(int eid, webs_t wp, int argc, char_t **argv);


/* Routines exported in fmDNS.c */
extern void formDhcrelay(webs_t wp, char_t *path, char_t *query);

#ifdef ADDRESS_MAPPING
extern void formAddressMap(webs_t wp, char_t *path, char_t *query);
#ifdef MULTI_ADDRESS_MAPPING
extern int showMultAddrMappingTable(int eid, webs_t wp, int argc, char_t **argv);
#endif // MULTI_ADDRESS_MAPPING
#endif 

/* Routines exported in fmping.c */
extern void formPing(webs_t wp, char_t *path, char_t *query);

/* Routines exported in fmoamlb.c */
extern void formOamLb(webs_t wp, char_t *path, char_t *query);

/* Routines exported in fmreboot.c */
extern void formReboot(webs_t wp, char_t *path, char_t *query);
#ifdef FINISH_MAINTENANCE_SUPPORT
//xl_yue added,inform itms that maintenance finished
extern void formFinishMaintenance(webs_t wp, char_t *path, char_t *query);
#endif

#ifdef USE_LOGINWEB_OF_SERVER
//xl_yue added
extern void formLogin(webs_t wp, char_t *path, char_t *query);
extern void formLogout(webs_t wp, char_t *path, char_t *query);
// Kaohj
extern int passwd2xmit(int eid, webs_t wp, int argc, char_t **argv);
#endif


#ifdef ACCOUNT_LOGIN_CONTROL
//xl_yue add
extern void formAdminLogout(webs_t wp, char_t *path, char_t *query);
extern void formUserLogout(webs_t wp, char_t *path, char_t *query);
#endif

/* Routines exported in fmdhcpmode.c */
extern void formDhcpMode(webs_t wp, char_t *path, char_t *query);

/* Routines exported in fmupnp.c */
extern void formUpnp(webs_t wp, char_t *path, char_t *query);
extern int ifwanList_upnp(int eid, webs_t wp, int argc, char_t **argv);


/* Routines exported in fmigmproxy.c */
#ifdef CONFIG_USER_IGMPPROXY
extern void formIgmproxy(webs_t wp, char_t *path, char_t *query);
#endif
extern int ifwanList(int eid, webs_t wp, int argc, char_t **argv);

/* Routines exported in fmothers.c */
extern void formOthers(webs_t wp, char_t *path, char_t *query);
extern int diagMenu(int eid, webs_t wp, int argc, char_t **argv);
extern int adminMenu(int eid, webs_t wp, int argc, char_t **argv);

//xl_yue
extern int userAddAdminMenu(int eid, webs_t wp, int argc, char_t **argv);

#ifdef AUTO_PROVISIONING
/* Routines exported in fmautoprovision.c */
extern void formAutoProvision(webs_t wp, char_t *path, char_t *query);
#endif

/* Routines exported in fmsyslog.c*/
//extern void formSysLog(webs_t wp, char_t *path, char_t *query);

#ifdef IP_ACL
/* Routines exported in fmacl.c */
extern void formACL(webs_t wp, char_t *path, char_t *query);
extern int showACLTable(int eid, webs_t wp, int argc, char_t **argv);
extern int showACLIpAddr(int eid, webs_t wp, int argc, char_t **argv);
#endif
#ifdef MAC_ACL
extern int showACLMacTable(int eid, webs_t wp, int argc, char_t **argv);
#endif
#ifdef NAT_CONN_LIMIT
extern int showConnLimitTable(int eid, webs_t wp, int argc, char_t **argv);
extern void formConnlimit(webs_t wp, char_t *path, char_t *query);
#endif
#ifdef TCP_UDP_CONN_LIMIT
extern int showConnLimitTable(int eid, webs_t wp, int argc, char_t **argv);
extern void formConnlimit(webs_t wp, char_t *path, char_t *query);
#endif 

extern void formmacBase(webs_t wp, char_t *path, char_t *query);
extern int showMACBaseTable(int eid, webs_t wp, int argc, char_t **argv);

#ifdef URL_BLOCKING_SUPPORT
/* Routines exported in fmurl.c */
extern void formURL(webs_t wp, char_t *path, char_t *query);
extern int showURLTable(int eid, webs_t wp, int argc, char_t **argv);
extern int showKeywdTable(int eid, webs_t wp, int argc, char_t **argv);
#endif
#ifdef URL_ALLOWING_SUPPORT
extern int showURLALLOWTable(int eid, webs_t wp, int argc, char_t **argv);
#endif



#ifdef DOMAIN_BLOCKING_SUPPORT
/* Routines exported in fmdomainblk.c */
extern void formDOMAINBLK(webs_t wp, char_t *path, char_t *query);
extern int showDOMAINBLKTable(int eid, webs_t wp, int argc, char_t **argv);
#endif

#ifdef TIME_ZONE
extern void formNtp(webs_t wp, char_t *path, char_t *query);
#endif

#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
// star: for dhcp server
extern int show_IP_range(int eid, webs_t wp, int argc, char_t **argv);
#endif

extern int vportMenu(int eid, webs_t wp, int argc, char_t **argv);
#ifdef CONFIG_RTL867X_COMBO_PORTMAPPING
extern void formEth2pvc(webs_t wp, char_t *path, char_t *query);
extern int eth2pvcPost(int eid, webs_t wp, int argc, char_t **argv);
extern void formItfGroup(webs_t wp, char_t *path, char_t *query);
extern int ifGroupList(int eid, webs_t wp, int argc, char_t **argv);
extern int eth2pvcStatus(int eid, webs_t wp, int argc, char_t **argv);
extern int showIfGroupList(int eid, webs_t wp, int argc, char_t **argv);
#endif
#ifdef CONFIG_EXT_SWITCH
//extern void formMpMode(webs_t wp, char_t *path, char_t *query);
/* Routines exported in fmeth2pvc.c */
extern void formEth2pvc(webs_t wp, char_t *path, char_t *query);
extern int eth2pvcPost(int eid, webs_t wp, int argc, char_t **argv);
extern void formItfGroup(webs_t wp, char_t *path, char_t *query);
extern int ifGroupList(int eid, webs_t wp, int argc, char_t **argv);
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
extern int eth2pvcStatus(int eid, webs_t wp, int argc, char_t **argv);
extern int showIfGroupList(int eid, webs_t wp, int argc, char_t **argv);
#endif
extern void formVlan(webs_t wp, char_t *path, char_t *query);
extern int vlanPost(int eid, webs_t wp, int argc, char_t **argv);
#ifdef ELAN_LINK_MODE
extern void formLink(webs_t wp, char_t *path, char_t *query);
extern int show_lanport(int eid, webs_t wp, int argc, char_t **argv);
#endif
// Kaohj -- vlan-grouping
extern void formVlanGroup(webs_t wp, char_t *path, char_t *query);
#else // of CONFIG_EXT_SWITCH 
#ifdef ELAN_LINK_MODE_INTRENAL_PHY
extern void formLink(webs_t wp, char_t *path, char_t *query); 
#endif
#endif	// of CONFIG_EXT_SWITCH
//#ifdef IP_QOS
#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
extern void formQos(webs_t wp, char_t *path, char_t *query);
extern int default_qos(int eid, webs_t wp, int argc, char_t **argv);
extern int qosList(int eid, webs_t wp, int argc, char_t **argv);
extern int priority_outif(int eid, webs_t wp, int argc, char_t **argv);
extern int confDscp(int eid, webs_t wp, int argc, char_t **argv);
#ifdef QOS_DIFFSERV
extern void formDiffServ(webs_t wp, char_t *path, char_t *query);
extern int diffservList(int eid, webs_t wp, int argc, char_t **argv);
#endif
#ifdef QOS_DSCP_MATCH
extern int match_dscp(int eid, webs_t wp, int argc, char_t **argv);
#endif
#endif
#ifdef NEW_IP_QOS_SUPPORT
extern void formQosShape(webs_t wp, char_t *path, char_t *query);
extern int  initTraffictlPage(int eid, webs_t wp, int argc, char_t **argv);
#endif
extern int iflanList(int eid, webs_t wp, int argc, char_t **argv);
extern int policy_route_outif(int eid, webs_t wp, int argc, char_t **argv);

#ifdef CONFIG_8021P_PRIO
extern int setting_1ppriority(int eid, webs_t wp, int argc, char_t **argv);
#ifdef NEW_IP_QOS_SUPPORT
extern int setting_predprio(int eid, webs_t wp, int argc, char_t **argv);
#endif
#endif


extern void formAcc(webs_t wp, char_t *path, char_t *query);
extern int accPost(int eid, webs_t wp, int argc, char_t **argv);
extern int accItem(int eid, webs_t wp, int argc, char_t **argv);
extern void ShowAutoPVC(int eid, webs_t wp, int argc, char_t **argv);	// auto-pvc-search

extern void ShowApplicationMode(int eid, webs_t wp, int argc, char_t **argv); // China telecom tr069/internet/others
extern void ShowChannelMode(int eid, webs_t wp, int argc, char_t **argv); // China telecom e8-a
extern void ShowPPPIPSettings(int eid, webs_t wp, int argc, char_t **argv); // China telecom e8-a
extern void ShowNAPTSetting(int eid, webs_t wp, int argc, char_t **argv); // China telecom e8-a
#ifdef CONFIG_GUI_WEB	// Jenny
extern int fwGuiMenu(int eid, webs_t wp, int argc, char_t **argv);
extern int dnsGuiMenu(int eid, webs_t wp, int argc, char_t **argv);
extern int aclGuiMenu(int eid, webs_t wp, int argc, char_t **argv);
extern int adminGuiMenu(int eid, webs_t wp, int argc, char_t **argv);
extern int diagGuiMenu(int eid, webs_t wp, int argc, char_t **argv);
extern int vportGuiMenu(int eid, webs_t wp, int argc, char_t **argv);
extern int wlanGuiMenu(int eid, webs_t wp, int argc, char_t **argv);
extern void formPPPAuth(webs_t wp, char_t *path, char_t *query);
extern int getPPPInfo(int eid, webs_t wp, int argc, char_t **argv);
extern void checkPPPStatus(int eid, webs_t wp, int argc, char_t **argv);
#else
//xl_yue
extern int createMenu(int eid, webs_t wp, int argc, char_t **argv);
extern int createMenu_user(int eid, webs_t wp, int argc, char_t **argv);
#endif

#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
extern void ZTESoftwareVersion(int eid, webs_t wp, int argc, char_t **argv); // ZTE 531B
#endif

#ifdef WLAN_SUPPORT
extern int wlanMenu(int eid, webs_t wp, int argc, char_t **argv);
extern int wlanStatus(int eid, webs_t wp, int argc, char_t **argv);
/* Routines exported in fmwlan.c */
extern void formWlanSetup(webs_t wp, char_t *path, char_t *query);
#ifdef WLAN_ACL
extern int wlAcList(int eid, webs_t wp, int argc, char_t **argv);
extern void formWlAc(webs_t wp, char_t *path, char_t *query);
extern int wlShowAcList(int eid, webs_t wp, int argc, char_t **argv);
#endif
extern void formAdvanceSetup(webs_t wp, char_t *path, char_t *query);
extern int wirelessClientList(int eid, webs_t wp, int argc, char_t **argv);
extern int wirelessClientList2(int eid, webs_t wp, int argc, char_t **argv);
extern void formWirelessTbl(webs_t wp, char_t *path, char_t *query);
extern void formWep(webs_t wp, char_t *path, char_t *query);

#ifdef WLAN_WPA
extern void formWlEncrypt(webs_t wp, char_t *path, char_t *query);
#endif
//WDS
#ifdef WLAN_WDS
extern void formWlWds(webs_t wp, char_t *path, char_t *query);
extern int wlWdsList(int eid, webs_t wp, int argc, char_t **argv);
#endif
#ifdef WLAN_CLIENT
extern void formWlSiteSurvey(webs_t wp, char_t *path, char_t *query);
extern int wlSiteSurveyTbl(int eid, webs_t wp, int argc, char_t **argv);
#endif

#ifdef CONFIG_WIFI_SIMPLE_CONFIG //WPS
extern void formWsc(webs_t wp, char_t *path, char_t *query);
#endif
#endif // of WLAN_SUPPORT
extern int oamSelectList(int eid, webs_t wp, int argc, char_t **argv);
#ifdef DIAGNOSTIC_TEST
extern void formDiagTest(webs_t wp, char_t *path, char_t *query);	// Diagnostic test
extern int lanTest(int eid, webs_t wp, int argc, char_t **argv);	// Ethernet LAN connection test
extern int adslTest(int eid, webs_t wp, int argc, char_t **argv);	// ADSL service provider connection test
extern int internetTest(int eid, webs_t wp, int argc, char_t **argv);	// Internet service provider connection test
#endif
#ifdef WLAN_MBSSID
extern int wlmbssid_asp(int eid, webs_t wp, int argc, char_t **argv);
extern void formWlanMBSSID(webs_t wp, char_t *path, char_t *query);
extern int postSSID(int eid, webs_t wp, int argc, char_t **argv);
extern int postSSIDWEP(int eid, webs_t wp, int argc, char_t **argv);
extern int checkSSID(int eid, webs_t wp, int argc, char_t **argv);
extern int SSIDStr(int eid, webs_t wp, int argc, char_t **argv);
#endif

#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
//xl_yue
extern int LinkMBSSIDPage(int eid, webs_t wp, int argc, char_t **argv);
#endif

#ifdef PORT_FORWARD_ADVANCE
extern void formPFWAdvance(webs_t wp, char_t *path, char_t *query);
extern int showPFWAdvTable(int eid, webs_t wp, int argc, char_t **argv);
#endif
extern int showPFWAdvForm(int eid, webs_t wp, int argc, char_t **argv);
#ifdef WEB_ENABLE_PPP_DEBUG
extern void ShowPPPSyslog(int eid, webs_t wp, int argc, char_t **argv);
#endif
extern void DSLuptime(int eid, webs_t wp, int argc, char_t **argv);
extern void set_user_profile(void);
// Kaohj -- been move to utility.c from fmigmproxy.c
//extern int ifWanNum(char_t *type);
#ifdef CONFIG_USER_ROUTED_ROUTED
extern int ifRipNum(); // fmroute.c
#endif

extern const char * const BRIDGE_IF;
extern const char * const ELAN_IF;
extern const char * const WLAN_IF;

extern int g_remoteConfig;
extern int g_remoteAccessPort;
extern int g_rexpire;
extern short g_rSessionStart;

/* constant string */
static const char DOCUMENT[] = "document";
extern int lanSetting(int eid, webs_t wp, int argc, char_t **argv);
extern int getPortMappingInfo(int eid, webs_t wp, int argc, char_t **argv);
#ifdef WEB_MENU_USE_NEW
typedef enum {
	MENU_FOLDER,
	MENU_URL
} MENU_T;

struct RootMenu{
	char  name[128];
	MENU_T type;
	union {
		void *addr;
		void *url;
	} u;
	char tip[128];
	int childrennums;
	int eol;	// end of layer
};
#endif
//add by ramen for ALG ON-OFF
#ifdef CONFIG_IP_NF_ALG_ONOFF
void formALGOnOff(webs_t wp, char_t *path, char_t *query);
void GetAlgTypes(webs_t wp);
void initAlgOnOff(webs_t wp);
void CreatejsAlgTypeStatus(webs_t wp);
#endif
#endif // _INCLUDE_APFORM_H
