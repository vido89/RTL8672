/*
*  fmguimenu.c is used to create GUI menu
*  added by Jenny
*/

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "../webs.h"
#include "mib.h"
#include "webform.h"
#include "utility.h"
#include "debug.h"
#include "vendor.h"

int fwGuiMenu(int eid, webs_t wp, int argc, char_t **argv)
{
	char *parm;
	
	if (ejArgs(argc, argv, T("%s"), &parm) < 1) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}
	
	if ( parm[0] == '1' ) {
		websWrite(wp, T("stm_aix(\"p0i5\",\"p0i2\",[2,\"\",\"images/img-firewall-b.gif\",\"images/img-firewall-a.gif\",74,71,0,\"\",\"_self\",\"\",\"Firewall\"]);"
			"stm_bpx(\"p5\",\"p2\",[0,4,-600]);"
#ifdef WLAN_SUPPORT
#ifdef WLAN_ACL
			"stm_aix(\"p5i0\",\"p3i3\",[]);"
#endif
#endif
			"stm_aix(\"p5i1\",\"p1i1\",[0,\"IP/Port Filtering    \",\"\",\"\",-1,-1,0,\"/fw-ipportfilter.asp\",\"view\"]);"
#ifdef MAC_FILTER
			"stm_aix(\"p5i2\",\"p1i1\",[0,\"MAC Filtering    \",\"\",\"\",-1,-1,0,\"/fw-macfilter.asp\",\"view\"]);"));
#endif
#ifdef URL_BLOCKING_SUPPORT
		websWrite(wp, T("stm_aix(\"p5i4\",\"p1i1\",[0,\"URL Blocking    \",\"\",\"\",-1,-1,0,\"/url_blocking.asp\",\"view\"]);"));			
#endif	
#ifdef DOMAIN_BLOCKING_SUPPORT	
		websWrite(wp, T("stm_aix(\"p5i5\",\"p1i1\",[0,\"Domain Blocking    \",\"\",\"\",-1,-1,0,\"/domainblk.asp\",\"view\"]);"));			
#endif
#ifdef NATIP_FORWARDING
		websWrite(wp, T("stm_aix(\"p5i6\",\"p1i1\",[0,\"NAT IP Forwarding    \",\"\",\"\",-1,-1,0,\"/fw-ipfw.asp\",\"view\"]);"));			
#else
		websWrite(wp, T("stm_aix(\"p5i3\",\"p1i1\",[0,\"Port Forwarding    \",\"\",\"\",-1,-1,0,\"/fw-portfw.asp\",\"view\"]);"));

#endif
#ifdef PORT_TRIGGERING
		websWrite(wp, T("stm_aix(\"p5i7\",\"p1i1\",[0,\"Port Triggering    \",\"\",\"\",-1,-1,0,\"/gaming.asp\",\"view\"]);"));			
#endif
		websWrite(wp, T("stm_aix(\"p5i8\",\"p1i1\",[0,\"DMZ    \",\"\",\"\",-1,-1,0,\"/fw-dmz.asp\",\"view\"]);"
#ifdef CONFIG_USER_IGMPPROXY
			"stm_aix(\"p5i9\",\"p1i1\",[0,\"IGMP Proxy    \",\"\",\"\",-1,-1,0,\"/igmproxy.asp\",\"view\"]);"
#endif
//#ifdef CONFIG_USER_UPNPD
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
			"stm_aix(\"p5i10\",\"p1i1\",[0,\"UPnP    \",\"\",\"\",-1,-1,0,\"/upnp.asp\",\"view\"]);"
#endif	
#ifdef CONFIG_USER_ROUTED_ROUTED
			"stm_aix(\"p5i11\",\"p1i1\",[0,\"RIP    \",\"\",\"\",-1,-1,0,\"/rip.asp\",\"view\"]);"
#endif
			"stm_ep();"));
	}
	return 0;
}

int dnsGuiMenu(int eid, webs_t wp, int argc, char_t **argv)
{
	char *parm;
	
	if (ejArgs(argc, argv, T("%s"), &parm) < 1) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}
	
	if ( parm[0] == '1' ) {
		websWrite(wp, T("stm_aix(\"p6i9\",\"p1i1\",[0,\"DNS Server \",\"\",\"\",-1,-1,0,\"/dns.asp\",\"view\"]);"
#ifdef CONFIG_USER_DDNS			
			"stm_aix(\"p6i10\",\"p1i1\",[0,\"DDNS \",\"\",\"\",-1,-1,0,\"/ddns.asp\",\"view\"]);"
#endif
		));
	}
	return 0;
}

int aclGuiMenu(int eid, webs_t wp, int argc, char_t **argv)
{
	char *parm;
	
	if (ejArgs(argc, argv, T("%s"), &parm) < 1) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}
	
	if ( parm[0] == '1' ) {
#ifdef IP_ACL
		websWrite(wp, T("stm_aix(\"p6i11\",\"p1i1\",[0,\"ACL Config    \",\"\",\"\",-1,-1,0,\"/admin/acl.asp\",\"view\"]);"));
#endif
	}
	return 0;
}

int adminGuiMenu(int eid, webs_t wp, int argc, char_t **argv)
{
	char *parm;
	
	if (ejArgs(argc, argv, T("%s"), &parm) < 1) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}
	
	if ( parm[0] == '1' ) {
		websWrite(wp, T("stm_aix(\"p0i7\",\"p0i2\",[2,\"\",\"images/img-admin-b.gif\",\"images/img-admin-a.gif\",74,71,0,\"\",\"_self\",\"\",\"Admin\"]);"
			"stm_bpx(\"p7\",\"p2\",[0,4,-450]);"
#ifdef FINISH_MAINTENANCE_SUPPORT			
			"stm_aix(\"p7i0\",\"p1i1\",[0,\"Finish Maintenance    \",\"\",\"\",-1,-1,0,\"/finishmaintenance.asp\",\"view\"]);"
#endif			
			"stm_aix(\"p7i1\",\"p1i1\",[0,\"Save &amp; Reboot    \",\"\",\"\",-1,-1,0,\"/admin/reboot.asp\",\"view\"]);"
#ifdef CONFIG_SAVE_RESTORE			
			"stm_aix(\"p7i2\",\"p1i1\",[0,\"Backup/Restore    \",\"\",\"\",-1,-1,0,\"/admin/saveconf.asp\",\"view\"]);"
#endif	
#ifdef ACCOUNT_LOGIN_CONTROL
			"stm_aix(\"p7i3\",\"p1i1\",[0,\"Logout    \",\"\",\"\",-1,-1,0,\"/admin/adminlogout.asp\",\"view\"]);"
#endif			

#ifdef SYSLOG_SUPPORT
			"stm_aix(\"p7i4\",\"p1i1\",[0,\"System Log    \",\"\",\"\",-1,-1,0,\"/syslog.asp\",\"view\"]);"
#endif
#ifdef DOS_SUPPORT
			"stm_aix(\"p7i5\",\"p1i1\",[0,\"DOS    \",\"\",\"\",-1,-1,0,\"/dos.asp\",\"view\"]);"
#endif
#ifdef ACCOUNT_CONFIG
			"stm_aix(\"p7i6\",\"p1i1\",[0,\"User Account    \",\"\",\"\",-1,-1,0,\"/userconfig.asp\",\"view\"]);"));
#else
			"stm_aix(\"p7i6\",\"p1i1\",[0,\"Password    \",\"\",\"\",-1,-1,0,\"/password.asp\",\"view\"]);"));
#endif
		websWrite(wp, T(
#ifdef WEB_UPGRADE
#ifdef UPGRADE_V1
//			"stm_aix(\"p7i7\",\"p1i1\",[0,\"Upgrade Firmware    \",\"\",\"\",-1,-1,0,\"/admin/upload.asp\",\"view\"]);"
			"stm_aix(\"p7i7\",\"p1i1\",[0,\"Upgrade Firmware    \",\"\",\"\",-1,-1,0,\"/upload.asp\",\"view\"]);"
#else
#ifdef UPGRADE_V2
			"stm_aix(\"p7i7\",\"p1i1\",[0,\"Upgrade Firmware    \",\"\",\"\",-1,-1,0,\"/admin/upload2.asp\",\"view\"]);"
#endif // of UPGRADE_V2
#ifdef UPGRADE_V3
			"stm_aix(\"p7i7\",\"p1i1\",[0,\"Upgrade Firmware    \",\"\",\"\",-1,-1,0,\"/admin/upload3.asp\",\"view\"]);"
#endif // of UPGRADE_V3
#endif // of UPGRADE_V1
#endif // of WEB_UPGRADE
#ifdef TIME_ZONE
			"stm_aix(\"p7i8\",\"p1i1\",[0,\"Time Zone    \",\"\",\"\",-1,-1,0,\"/ntp.asp\",\"view\"]);"
#endif
#ifdef AUTO_PROVISIONING
			"stm_aix(\"p7i9\",\"p1i1\",[0,\"Auto-Provisionning    \",\"\",\"\",-1,-1,0,\"/autoprovision.asp\",\"view\"]);"
#endif
#ifdef _CWMP_MIB_
			"stm_aix(\"p7i10\",\"p1i1\",[0,\"TR-069 Config    \",\"\",\"\",-1,-1,0,\"/tr069config.asp\",\"view\"]);"
#endif
//added by xl_yue
#ifdef USE_LOGINWEB_OF_SERVER
			"stm_aix(\"p7i11\",\"p1i1\",[0,\"Logout    \",\"\",\"\",-1,-1,0,\"/admin/logout.asp\",\"view\"]);"
#endif

			));
		websWrite(wp, T("stm_ep();"));
	}
	return 0;
}

int diagGuiMenu(int eid, webs_t wp, int argc, char_t **argv)
{
	char *parm;
	
	if (ejArgs(argc, argv, T("%s"), &parm) < 1) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}
	
	if ( parm[0] == '1' ) {
#if 0
		websWrite(wp, T("stm_aix(\"p1i1\",\"p1i0\",[0,\"Ping    \",\"\",\"\",-1,-1,0,\"/ping.asp\",\"view\"]);"
			"stm_aix(\"p1i2\",\"p1i1\",[0,\"ATM Loopback    \",\"\",\"\",-1,-1,0,\"/oamloopback.asp\",\"view\"]);"
			"stm_aix(\"p1i3\",\"p1i1\",[0,\"ADSL Diagnostic    \",\"\",\"\",-1,-1,0,\"/adv/adsl-diag.asp\",\"view\"]);"
#ifdef DIAGNOSTIC_TEST
			"stm_aix(\"p1i4\",\"p1i1\",[0,\"Diagnostic Test    \",\"\",\"\",-1,-1,0,\"/diag-test.asp\",\"view\"]);"
#endif
		));
#endif
#ifdef DIAGNOSTIC_TEST
		websWrite(wp, T("stm_aix(\"p1i1\",\"p1i0\",[0,\"Diagnostic Test    \",\"\",\"\",-1,-1,0,\"/diag-test.asp\",\"view\"]);"));
#endif
	}
	return 0;
}

int vportGuiMenu(int eid, webs_t wp, int argc, char_t **argv)
{
	char *parm;
	
	if (ejArgs(argc, argv, T("%s"), &parm) < 1) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}
	
	if ( parm[0] == '1' ) {
		websWrite(wp, T(
#ifdef CONFIG_IP_NF_ALG_ONOFF
		"stm_aix(\"p6i4\",\"p1i1\",[0,\"ALG \",\"\",\"\",-1,-1,0,\"/algonoff.asp\",\"view\"]);"
#endif
#if (defined( CONFIG_EXT_SWITCH) && defined(ITF_GROUP)) || defined(CONFIG_RTL867X_COMBO_PORTMAPPING)
#ifdef BPORTMAPPING_SUPPORT
		"stm_aix(\"p6i5\",\"p1i1\",[0,\"Port Mapping \",\"\",\"\",-1,-1,0,\"/eth2pvc.asp\",\"view\"]);"
#endif
#endif

#ifndef NEW_IP_QOS_SUPPORT
#ifdef IP_QOS
		"stm_aix(\"p6i6\",\"p1i1\",[0,\"IP QoS \",\"\",\"\",-1,-1,0,\"/ipqos.asp\",\"view\"]);"
#endif
#else
		"stm_aix(\"p6i6\",\"p1i1\",[0,\"IP QoS \",\"\",\"\",-1,-1,0,\"/ipqos_sc.asp\",\"view\"]);"
#endif

#ifdef CONFIG_EXT_SWITCH
#ifdef ELAN_LINK_MODE
		"stm_aix(\"p6i7\",\"p1i1\",[0,\"Link Mode \",\"\",\"\",-1,-1,0,\"/lnkmode.asp\",\"view\"]);"
#endif
#endif
		));
	}
	
	return 0;
}

int wlanGuiMenu(int eid, webs_t wp, int argc, char_t **argv)
{
#ifdef WLAN_SUPPORT
	char *parm;
	
	if (ejArgs(argc, argv, T("%s"), &parm) < 1) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}
	
	if ( parm[0] == '1' ) {
		websWrite(wp, T("stm_aix(\"p0i3\",\"p0i2\",[2,\"\",\"images/img-wireless-b.gif\",\"images/img-wireless-a.gif\",74,71,0,\"\",\"_self\",\"\",\"Wireless\"]);"
			"stm_bpx(\"p3\",\"p2\",[0,4,-220,4]);"
			"stm_aix(\"p3i0\",\"p1i1\",[0,\"Basic Settings    \",\"\",\"\",-1,-1,0,\"/admin/wlbasic.asp\",\"view\"]);"
			"stm_aix(\"p3i1\",\"p1i1\",[0,\"Advanced Settings    \",\"\",\"\",-1,-1,0,\"/admin/wladvanced.asp\",\"view\"]);"
#ifdef WLAN_MBSSID
			"stm_aix(\"p3i2\",\"p1i1\",[0,\"Security    \",\"\",\"\",-1,-1,0,\"/admin/wlwpa_mbssid.asp\",\"view\"]);"));			
#else
			"stm_aix(\"p3i2\",\"p1i1\",[0,\"Security    \",\"\",\"\",-1,-1,0,\"/admin/wlwpa.asp\",\"view\"]);"));			
#endif

#ifdef WLAN_ACL
		websWrite(wp, T("stm_aix(\"p3i3\",\"p1i1\",[0,\"Access Control    \",\"\",\"\",-1,-1,0,\"/admin/wlactrl.asp\",\"view\"]);"));
#endif
#ifdef WLAN_WDS
		websWrite(wp, T("stm_aix(\"p3i4\",\"p1i1\",[0,\"WDS    \",\"\",\"\",-1,-1,0,\"/admin/wlwds.asp\",\"view\"]);"));
#endif
#ifdef WLAN_CLIENT
		websWrite(wp, T("stm_aix(\"p3i5\",\"p1i1\",[0,\"Site Survey    \",\"\",\"\",-1,-1,0,\"/admin/wlsurvey.asp\",\"view\"]);"));
#endif

#ifdef CONFIG_WIFI_SIMPLE_CONFIG//WPS def WLAN_CLIENT
		websWrite(wp, T("stm_aix(\"p3i6\",\"p1i1\",[0,\"WPS    \",\"\",\"\",-1,-1,0,\"/admin/wlwps.asp\",\"view\"]);"));
#endif

#ifdef WLAN_MBSSID
		websWrite(wp, T("stm_aix(\"p3i7\",\"p1i1\",[0,\"MBSSID    \",\"\",\"\",-1,-1,0,\"/admin/wlmbssid.asp\",\"view\"]);"));
#endif

		websWrite(wp, T("stm_ep();"));
	}
#endif
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int defaultPPPRecord(MIB_CE_ATM_VC_Tp pEntry)
{
	MIB_CE_ATM_VC_T Entry;
	unsigned int totalEntry;
	int i, isPPP=0;
	totalEntry = mib_chain_total(MIB_ATM_VC_TBL); /* get chain record size */

	for (i=0; i<totalEntry; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) {
  			printf(T(strGetChainerror));
			return -1;
		}
		if (Entry.enable == 1 && (Entry.cmode == ADSL_PPPoE || Entry.cmode == ADSL_PPPoA)) {
			isPPP = 1;
			break;
		}
	}
	if (isPPP == 1)
		memcpy(pEntry, &Entry, sizeof(Entry));
	else
		return -1;
	return 0;
}

int getPPPInfo(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t *name;
	MIB_CE_ATM_VC_T entry;

	if (ejArgs(argc, argv, T("%s"), &name) < 1) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}

	if (defaultPPPRecord(&entry) == -1) {
		if (!strcmp(name, T("username")) || !strcmp(name,T("password")))
			websWrite(wp, T(""));
		return 1;
	}

	if ( !strcmp(name, T("username")) ) {
		websWrite(wp, T("%s"), entry.pppUsername);
		return 0;
	}
	else if (!strcmp(name,T("password"))) {
		websWrite(wp,T("%s"), entry.pppPassword);
		return 0;
     	}
	else
		return -1;
}

void checkPPPStatus(int eid, webs_t wp, int argc, char_t **argv)
{
	MIB_CE_ATM_VC_T Entry;
	unsigned int totalEntry;
	int i, pppUp = 0, flags;

	totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<totalEntry; i++) {
		struct in_addr inAddr;
		char ifname[16]="";
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) {
  			printf(T(strGetChainerror));
			return;
		}
		if (Entry.cmode != ADSL_PPPoE)
			continue;
		if (Entry.enable == 0)
			continue;
		if (ifGetName(Entry.ifIndex, ifname, sizeof(ifname)) != NULL) {
			//printf("ifname = %s\n", ifname);
			if (getInFlags(ifname, &flags) == 1) {
				if ((flags & IFF_UP) && (getInAddr( ifname, IP_ADDR, (void *)&inAddr) == 1))
					pppUp = 1;
			}
		}
	}
	//printf("pppUp=%d\n", pppUp);
	if (!pppUp)
		websWrite(wp, T("<span class=\"f11red\">\n"
						"Username and/or Password not correct, authentication fail. Please try again.\n"
						"</span>\n"));
}

void formPPPAuth(webs_t wp, char_t *path, char_t *query)
{
	char_t *username, *passwd, *submitUrl;
	char tmpBuf[100];
	MIB_CE_ATM_VC_T Entry;
	unsigned int totalEntry;
	int i;

	// user name
	username = websGetVar(wp, T("username"), T(""));
	if ( username[0] ) {
		if (strlen(username) > P_MAX_NAME_LEN-1) {
			strcpy(tmpBuf, T(strUserNametoolong));
			goto setErr_ppp;
		}
	}
	else {
		strcpy(tmpBuf, T(strUserNameempty));
		goto setErr_ppp;
	}

	// password
	passwd = websGetVar(wp, T("password"), T(""));
	if ( passwd[0] ) {
		if (strlen(passwd) > P_MAX_NAME_LEN-1 ) {
			strcpy(tmpBuf, T(strPasstoolong));
			goto setErr_ppp;
		}
	}
	else {
		strcpy(tmpBuf, T(strPassempty));
		goto setErr_ppp;
	}

	totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<totalEntry; i++) {
		struct data_to_pass_st msg;
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) {
  			printf(T(strGetChainerror));
			return;
		}
		if(Entry.enable == 0)
			continue;
		if (Entry.cmode == ADSL_PPPoE || Entry.cmode == ADSL_PPPoA) {
			strncpy(Entry.pppUsername, username, P_MAX_NAME_LEN-1);
			Entry.pppUsername[strlen(username)]='\0';
			strncpy(Entry.pppPassword, passwd, P_MAX_NAME_LEN-1);
			Entry.pppPassword[strlen(passwd)]='\0';
			mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, i);
		}
		else
			continue;
	}

#ifdef PPP_QUICK_CONNECT
	writeFlash();
#endif

	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
  	return;
  
setErr_ppp:
	ERR_MSG(tmpBuf);
}
