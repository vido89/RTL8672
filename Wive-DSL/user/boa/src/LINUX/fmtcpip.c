/*
 *      Web server handler routines for TCP/IP stuffs
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *      Authors: Dick Tam	<dicktam@realtek.com.tw>
 *
 */


/*-- System inlcude files --*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <time.h>
#include <net/route.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <net/if.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "../defs.h"

#ifdef __i386__
#define _LITTLE_ENDIAN_
#endif

/*-- Macro declarations --*/
#ifdef _LITTLE_ENDIAN_
#define ntohdw(v) ( ((v&0xff)<<24) | (((v>>8)&0xff)<<16) | (((v>>16)&0xff)<<8) | ((v>>24)&0xff) )

#else
#define ntohdw(v) (v)
#endif

#ifdef CONFIG_GUI_WEB
#include "utility.h"
#include <semaphore.h>
extern sem_t semSave;
#endif
///////////////////////////////////////////////////////////////////
void formTcpipLanSetup(webs_t wp, char_t *path, char_t *query)
{
	char_t	*pStr, *strIp, *strMask, *strSnoop, *submitUrl, *strBlock;
	struct in_addr inIp, inMask;
	char tmpBuf[100], mode;
#if defined(SECONDARY_IP) && !defined(DHCPS_POOL_COMPLETE_IP)
	char dhcp_pool;
#endif
#ifndef NO_ACTION
	int pid;
#endif
#ifdef CONFIG_IGMP_FORBID
     char_t *str_igmpforbid;
     
#endif
	unsigned char vChar;
#ifdef CONFIG_GUI_WEB
	unsigned char buf[16];
#endif

//star:for ip change
	struct in_addr origIp,origMask;
	int ip_mask_changed_flag = 0;
	int igmp_changed_flag = 0;
	int ip2_changed_flag = 0;

	
	// Set clone MAC address
	strIp = websGetVar(wp, T("ip"), T(""));
	strMask = websGetVar(wp, T("mask"), T(""));
	if (!isValidHostID(strIp, strMask)) {
		strcpy(tmpBuf, T("Invalid IP/Subnet Mask combination!!"));
		goto setErr_tcpip;
	}
	if ( strIp[0] ) {
		if ( !inet_aton(strIp, &inIp) ) {
			strcpy(tmpBuf, T(strWrongIP));
			goto setErr_tcpip;
		}
		mib_get(MIB_ADSL_LAN_IP, (void *)&origIp);
		if(origIp.s_addr != inIp.s_addr)
		{
			ip_mask_changed_flag = 1;
		/*lan ip & dhcp gateway setting should be set independently*/
		#if 1
			{	
			struct in_addr dhcp_gw;
			mib_get(MIB_ADSL_LAN_DHCP_GATEWAY, (void *)&dhcp_gw);
			if(dhcp_gw.s_addr==origIp.s_addr)
				mib_set(MIB_ADSL_LAN_DHCP_GATEWAY, (void *)&inIp);
			}
		#else
			mib_set(MIB_ADSL_LAN_DHCP_GATEWAY, (void *)&inIp);
		#endif
		}
		if ( !mib_set( MIB_ADSL_LAN_IP, (void *)&inIp)) {
			strcpy(tmpBuf, T(strSetIPerror));
			goto setErr_tcpip;
		}
	}
	else { // get current used IP
//		if ( !getInAddr(BRIDGE_IF, IP_ADDR, (void *)&inIp) ) {
//			strcpy(tmpBuf, T("Get IP-address error!"));
//			goto setErr_tcpip;
//		}
	}

//	strMask = websGetVar(wp, T("mask"), T(""));
	if ( strMask[0] ) {
		if (!isValidNetmask(strMask, 1)) {
			strcpy(tmpBuf, T(strWrongMask));
			goto setErr_tcpip;
		}
		if ( !inet_aton(strMask, &inMask) ) {
			strcpy(tmpBuf, T(strWrongMask));
			goto setErr_tcpip;
		}
		mib_get(MIB_ADSL_LAN_SUBNET, (void *)&origMask);
		if(origMask.s_addr!= inMask.s_addr)
			ip_mask_changed_flag = 1;
		if ( !mib_set(MIB_ADSL_LAN_SUBNET, (void *)&inMask)) {
			strcpy(tmpBuf, T(strSetMaskerror));
			goto setErr_tcpip;
		}
	}
	else { // get current used netmask
//		if ( !getInAddr(BRIDGE_IF, SUBNET_MASK, (void *)&inMask )) {
//			strcpy(tmpBuf, T("Get subnet-mask error!"));
//			goto setErr_tcpip;
//		}
	}
#ifdef WLAN_SUPPORT
	// set eth to wireless blocking
	strBlock = websGetVar(wp, T("BlockEth2Wir"), T(""));
	if (strBlock[0]) {
		if (strBlock[0] == '0')
			vChar = 0;
		else // '1'
			vChar = 1;
		if ( mib_set(MIB_WLAN_BLOCK_ETH2WIR, (void *)&vChar) == 0) {
			strcpy(tmpBuf, T(strSetLanWlanBlokErr));
			goto setErr_tcpip;
		}
		va_cmd("/bin/brctl", 3, 1, "wlanblock", "br0", vChar==0?"0":"2");
	}
#endif

#ifdef SECONDARY_IP
	char ip2mode;
	pStr = websGetVar(wp, T("enable_ip2"), T(""));
	if (pStr[0]) {
		if (pStr[0] == '1') {
			mode = 1;
			strIp = websGetVar(wp, T("ip2"), T(""));
			if ( strIp[0] ) {
				if ( !inet_aton(strIp, &inIp) ) {
					strcpy(tmpBuf, T(strWrongIP));
					goto setErr_tcpip;
				}
				mib_get(MIB_ADSL_LAN_IP2, (void *)&origIp);
				if(origIp.s_addr != inIp.s_addr)
				{
					ip2_changed_flag = 1;
					ip_mask_changed_flag = 1;
				}
				if ( !mib_set( MIB_ADSL_LAN_IP2, (void *)&inIp)) {
					strcpy(tmpBuf, T(strSetIPerror));
					goto setErr_tcpip;
				}
			}
			strMask = websGetVar(wp, T("mask2"), T(""));
			if ( strMask[0] ) {
				if ( !inet_aton(strMask, &inMask) ) {
					strcpy(tmpBuf, T(strWrongMask));
					goto setErr_tcpip;
				}
				mib_get(MIB_ADSL_LAN_SUBNET2, (void *)&origMask);
				if(origMask.s_addr!= inMask.s_addr)
				{
					ip2_changed_flag = 1;
					ip_mask_changed_flag = 1;
				}
				if ( !mib_set(MIB_ADSL_LAN_SUBNET2, (void *)&inMask)) {
					strcpy(tmpBuf, T(strSetMaskerror));
					goto setErr_tcpip;
				}
			}
#ifndef DHCPS_POOL_COMPLETE_IP
			// DHCP Server pool is on ...
			pStr = websGetVar(wp, T("dhcpuse"), T(""));
			if ( pStr[0] == '0' ) { // Primary LAN
				dhcp_pool = 0;
			}
			else { // Secondary LAN
				dhcp_pool = 1;
			}
			mib_set(MIB_ADSL_LAN_DHCP_POOLUSE, (void *)&dhcp_pool);
#endif
		}
		else {
			mode = 0;
		}
	}
	else {
		mode = 0;
	}
	mib_get(MIB_ADSL_LAN_ENABLE_IP2, (void *)&ip2mode);
	if(ip2mode != mode)
	{
		ip2_changed_flag = 1;
		ip_mask_changed_flag = 1;
	}
	mib_set(MIB_ADSL_LAN_ENABLE_IP2, (void *)&mode);
#endif	// of SECONDARY_IP

#ifdef CONFIG_EXT_SWITCH
	char origmode = 0;
	strSnoop = websGetVar(wp, T("snoop"), T(""));
	if ( strSnoop[0] ) {
		// bitmap for virtual lan port function
		// Port Mapping: bit-0
		// QoS : bit-1
		// IGMP snooping: bit-2
		mib_get(MIB_MPMODE, (void *)&mode);
		origmode = mode;
		strSnoop = websGetVar(wp, T("snoop"), T(""));
		if ( strSnoop[0] == '1' ) {
			mode |= 0x04;
			if(origmode != mode)
				igmp_changed_flag = 1;
			// take effect immediately
            // below action is maybe valid for all vendors... jim
#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)			
			__dev_register_swport();
#endif
			__dev_setupIGMPSnoop(1);
		}
		else {
			mode &= 0xfb;
			if(origmode != mode)
				igmp_changed_flag = 1;
			__dev_setupIGMPSnoop(0);
			if (mode==0)
				__dev_setupVirtualPort(0); // disable virtual port
		}
		mib_set(MIB_MPMODE, (void *)&mode);
	}

#ifdef CONFIG_IGMP_FORBID
 str_igmpforbid = websGetVar(wp, T("igmpforbid"), T(""));
 if( str_igmpforbid[0] )
 	{
 	   if(str_igmpforbid[0]=='0')
 	   	{
		  vChar =0;	  
		  __dev_igmp_forbid(0);
 	   	}
	   else if (str_igmpforbid[0]=='1')
	   	{
	   	  vChar =1;
		   __dev_igmp_forbid(1);
	   	} 
	   mib_set(MIB_IGMP_FORBID_ENABLE, (void *)&vChar);
 	}
#endif
    
#endif

	if(ip_mask_changed_flag == 1)
	{
#ifndef CONFIG_GUI_WEB
		/* upgdate to flash */
#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
			/*to avoid the dead when wrriting the flash*/
		itfcfg("sar", 0);
		itfcfg("eth0", 0);
#ifdef WLAN_SUPPORT
		itfcfg("wlan0", 0);
#endif
#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
		//mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);

#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		itfcfg("sar", 1);
		itfcfg("eth0", 1);
#ifdef WLAN_SUPPORT
		vChar=0;
		mib_get( MIB_WLAN_DISABLED, (void *)&vChar);
		if( vChar==0 )	itfcfg("wlan0", 1);
#endif //WLAN_SUPPORT
#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
		//RECONNECT_MSG(strIp);		
		//req_flush(wp);
		
		if(ip2_changed_flag == 1)
		{
			submitUrl = websGetVar(wp, T("submit-url"), T(""));
			OK_MSG(submitUrl);
		}
		else
		{
			submitUrl = websGetVar(wp, T("submit-url"), T(""));
			OK_MSG(submitUrl);
		}
		
		restart_lanip();	
#else
		/* Save and reboot the system */
		buf[0] = '\0';
		getMIB2Str(MIB_ADSL_LAN_IP, buf);
		websWrite(wp, T("<html><head>\n" \
						"<script language=javascript>\n" \
						"var h=60;\n" \
						"var timer;\n" \
						"function stop() { clearTimeout(id); }\n"\
						"function start() { h--;\n" \
						"if (h >= 0) { timer=setTimeout(\"start()\",1000); }\n" \
						"if (h == 0) { top.location.replace(\"http://%s/\"); }}\n" \
						"</script></head><body onLoad=\"start();\" onUnload=\"stop();\">" \
						"<blockquote><h4>Change setting successfully!</h4>\n" \
						"Please wait a minute to let the new settings take effect!!<br><br>\n" \
						"The web browser would reopen automatically.<br><br>\n" \
						"If necessary, reconfigure your PC's IP address to match your new configuration.<br><br>\n" \
						"</blockquote></body></html>"), buf); 
#ifdef EMBED
		sem_post(&semSave);
#endif
	  	return;
#endif
	}
	else
	{
		if(igmp_changed_flag == 1)
		{
#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
			
#else			
// Kaohj
#if 0
			/* upgdate to flash */
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
			submitUrl = websGetVar(wp, T("submit-url"), T(""));
			OK_MSG(submitUrl);
		}
		else
		{
			submitUrl = websGetVar(wp, T("submit-url"), T(""));
			if (submitUrl[0])
				websRedirect(wp, submitUrl);
			else
				websDone(wp, 200);
		}
	}

#ifndef NO_ACTION
	pid = fork();
        if (pid)
                waitpid(pid, NULL, 0);
        else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _CONFIG_SCRIPT_PROG);
#ifdef HOME_GATEWAY
		execl( tmpBuf, _CONFIG_SCRIPT_PROG, "gw", "bridge", NULL);
#else
		execl( tmpBuf, _CONFIG_SCRIPT_PROG, "ap", "bridge", NULL);
#endif
                exit(1);
        }
#endif

	    


	return;

setErr_tcpip:
	ERR_MSG(tmpBuf);
}

#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
/////////////////////////////////////////////////////////////////////////////
void formFastConf3(webs_t wp, char_t *path, char_t *query)
{
	char_t *strVal, *submitUrl;
	struct in_addr inIp, inMask;
	struct in_addr inIp2, inMask2;
	struct in_addr startIp, endIp;
	char_t *strSubmit;
	char tmpBuf[100];
	unsigned int day, hour, min;
	unsigned int vUInt;

	strSubmit = websGetVar(wp, T("laststep"), T(""));
	if (strSubmit[0]) {
		goto jumpLastPage;
	}
	
	strVal = websGetVar(wp, T("ip"), T(""));
	if (strVal[0]) {
		if (!inet_aton(strVal, &inIp)) {
			strcpy(tmpBuf, T(Tinvalid_ip));
			goto setErr_tcpip;
		}
	}
	
	strVal = websGetVar(wp, T("mask"), T(""));
	if (strVal[0]) {
		if (!inet_aton(strVal, &inMask)) {
			strcpy(tmpBuf, T(Tinvalid_mask));
			goto setErr_tcpip;
		}
	}

	strVal = websGetVar(wp, T("bakIp"), T(""));
	if (strVal[0])
		fcEntry2.enable_ip2 = 1;
	else
		fcEntry2.enable_ip2 = 0;

	strVal = websGetVar(wp, T("DhcpS"), T(""));
	if (strVal[0])
		fcEntry2.enable_dhcp = 2;
	else
		fcEntry2.enable_dhcp = 0;

	if (fcEntry2.enable_ip2) {
		///reserve second IP/mask
		strVal = websGetVar(wp, T("lan2IpAddress"), T(""));
		if (strVal[0]) {
			if (!inet_aton(strVal, &inIp2)) {
				strcpy(tmpBuf, T(Tinvalid_ip2));
				goto setErr_tcpip;
			}
			memcpy(fcEntry2.lanIp2, (void *)&inIp2, 4);
		}

		strVal = websGetVar(wp, T("lan2SubnetMask"), T(""));
		if (strVal[0]) {
			if (!inet_aton(strVal, &inMask2)) {
				strcpy(tmpBuf, T(Tinvalid_mask2));
				goto setErr_tcpip;
			}
			memcpy(fcEntry2.subnetMask2, (void *)&inMask2, 4);
		}
	}

	memcpy(fcEntry2.ipAddr, (void *)&inIp, 4);
	memcpy(fcEntry2.netMask, (void *)&inMask, 4);

	if (fcEntry2.enable_dhcp == 2) {
		///reserve dhcp parameter
		strVal = websGetVar(wp, T("startIp"), T(""));
		if (strVal[0]) {
			if (!inet_aton(strVal, &startIp)) {
				strcpy(tmpBuf, T(Tinvalid_start_ip));
				goto setErr_tcpip;
			}
			memcpy(fcEntry2.dhcpStartIp, (void *)&startIp, 4);
		}

		strVal = websGetVar(wp, T("endIp"), T(""));
		if (strVal[0]) {
			if (!inet_aton(strVal, &endIp)) {
				strcpy(tmpBuf, T(Tinvalid_end_ip));
				goto setErr_tcpip;
			}
			memcpy(fcEntry2.dhcpEndIp, (void *)&endIp, 4);
		}

		strVal = websGetVar(wp, T("time_dd"), T(""));
		if (!strVal[0])
			day = 0;
		else {
			sscanf(strVal, "%u", &vUInt);
			day = vUInt;
		}
		strVal = websGetVar(wp, T("time_hh"), T(""));
		if (!strVal[0])
			hour = 0;
		else {
			sscanf(strVal, "%u", &vUInt);
			hour = vUInt;
		}
		strVal = websGetVar(wp, T("time_mm"), T(""));
		if (!strVal[0])
			min = 0;
		else {
			sscanf(strVal, "%u", &vUInt);
			min = vUInt;
		}
		if (day != -1) {//if day==-1, lease time infinite
			hour += min/60;
			min = min%60;
			day += hour/24;
			hour = hour%24;
		}
		fcEntry2.day = day;
		fcEntry2.hour = hour;
		fcEntry2.min = min;
	}

	submitUrl = websGetVar(wp, T("submit-url-n"), T(""));   // hidden page
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
  	return;
jumpLastPage:
	if (fcEntry.cmode == ADSL_BR1483)
  		submitUrl = websGetVar(wp, T("submit-url-bridge"), T(""));
	else if (fcEntry.cmode == ADSL_PPPoE ||fcEntry.cmode == ADSL_PPPoA)
		submitUrl = websGetVar(wp, T("submit-url-ppp"), T(""));
	else if (fcEntry.cmode == ADSL_MER1483)
		submitUrl = websGetVar(wp, T("submit-url-mer"), T(""));
	else if (fcEntry.cmode == ADSL_RT1483)
		submitUrl = websGetVar(wp, T("submit-url-ipoa"), T(""));
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
	return;
setErr_tcpip:
	ERR_MSG(tmpBuf);
}
#endif

#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
int lan_setting(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;
#ifdef SECONDARY_IP
	char buf[32];
	
	nBytesSent += websWrite(wp, T("<tr>\n<td><font size=2>\n"));
	nBytesSent += websWrite(wp, T("<input type=checkbox name=enable_ip2 value=1 "));
	nBytesSent += websWrite(wp, T("onClick=updateInput()>&nbsp;&nbsp;"));
	nBytesSent += websWrite(wp, T("<b>第二IP</b>\n</td></tr>\n"));
	
	getMIB2Str(MIB_ADSL_LAN_IP2, buf);
	nBytesSent += websWrite(wp, T("<div ID=\"secondIP\" style=\"display:none\">\n"));
	nBytesSent += websWrite(wp, T("<table border=0 width=\"500\" cellspacing=4 cellpadding=0>\n"));
	nBytesSent += websWrite(wp, T("<tr><td width=150><font size=2><b>IP地址:</b></td>\n"));
	nBytesSent += websWrite(wp, T("<td width=350><input type=text name=ip2 size=15 maxlength=15 "));
	nBytesSent += websWrite(wp, T("value=%s></td>\n</tr>\n"), buf);
	
	getMIB2Str(MIB_ADSL_LAN_SUBNET2, buf);
	nBytesSent += websWrite(wp, T("<tr><td><font size=2><b>子网掩码:</b></td>\n"));
	nBytesSent += websWrite(wp, T("<td><input type=text name=mask2 size=15 maxlength=15 "));
	nBytesSent += websWrite(wp, T("value=%s></td>\n</tr></table></div><p>\n"), buf);
#endif
	
#ifdef CONFIG_EXT_SWITCH
	nBytesSent += websWrite(wp, T("<table border=0 width=\"500\" cellspacing=4 cellpadding=0>\n"));
	nBytesSent += websWrite(wp, T("<tr><td width=150><font size=2><b>IGMP Snooping:</b></td>\n"));
	nBytesSent += websWrite(wp, T("<td width=350><font size=2>\n<input type=\"radio\""));
	nBytesSent += websWrite(wp, T(" name=snoop value=0>禁止&nbsp;&nbsp;\n"));
	nBytesSent += websWrite(wp, T("<input type=\"radio\" name=snoop value=1>允许</td>\n</tr></table>\n"));
#ifdef CONFIG_IGMP_FORBID
       nBytesSent += websWrite(wp, T("<table border=0 width=\"500\" cellspacing=4 cellpadding=0>\n"));
	nBytesSent += websWrite(wp, T("<tr><td width=150><font size=2><b>IGMP 透传:</b></td>\n"));
	nBytesSent += websWrite(wp, T("<td width=350><font size=2>\n<input type=\"radio\""));
	nBytesSent += websWrite(wp, T(" name=igmpforbid value=0>禁止&nbsp;&nbsp;\n"));
	nBytesSent += websWrite(wp, T("<input type=\"radio\" name=igmpforbid value=1>允许</td>\n</tr></table>\n"));
#endif
#endif
	
	return nBytesSent;
}
#else
int lan_setting(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;
#ifdef SECONDARY_IP
	char buf[32];
	
	nBytesSent += websWrite(wp, T("<tr>\n<td><font size=2>\n"));
	nBytesSent += websWrite(wp, T("<input type=checkbox name=enable_ip2 value=1 "));
	nBytesSent += websWrite(wp, T("onClick=updateInput()>&nbsp;&nbsp;"));
	nBytesSent += websWrite(wp, T("<b>Secondary IP</b>\n</td></tr>\n"));
	
	getMIB2Str(MIB_ADSL_LAN_IP2, buf);
	nBytesSent += websWrite(wp, T("<div ID=\"secondIP\" style=\"display:none\">\n"));
	nBytesSent += websWrite(wp, T("<table border=0 width=\"500\" cellspacing=4 cellpadding=0>\n"));
	nBytesSent += websWrite(wp, T("<tr><td width=150><font size=2><b>IP Address:</b></td>\n"));
	nBytesSent += websWrite(wp, T("<td width=350><input type=text name=ip2 size=15 maxlength=15 "));
	nBytesSent += websWrite(wp, T("value=%s></td>\n</tr>\n"), buf);
	
	getMIB2Str(MIB_ADSL_LAN_SUBNET2, buf);
	nBytesSent += websWrite(wp, T("<tr><td><font size=2><b>Subnet Mask:</b></td>\n"));
	nBytesSent += websWrite(wp, T("<td><input type=text name=mask2 size=15 maxlength=15 "));
	nBytesSent += websWrite(wp, T("value=%s></td>\n</tr>\n"), buf);
	
#ifndef DHCPS_POOL_COMPLETE_IP
	nBytesSent += websWrite(wp, T("<tr></tr><tr><td><font size=2><b>DHCP pool:</b></td>\n"));
	nBytesSent += websWrite(wp, T("<td><font size=2>\n<input type=\"radio\""));
	nBytesSent += websWrite(wp, T(" name=dhcpuse value=0>Primary LAN&nbsp;&nbsp;\n"));
	nBytesSent += websWrite(wp, T("<input type=\"radio\" name=dhcpuse value=1>Secondary LAN</td>\n</tr>"));
#endif
	nBytesSent += websWrite(wp, T("</table></div>\n"));
#endif
	
#ifdef CONFIG_EXT_SWITCH
	nBytesSent += websWrite(wp, T("<table border=0 width=\"500\" cellspacing=4 cellpadding=0>\n"));
	nBytesSent += websWrite(wp, T("<tr><td width=150><font size=2><b>IGMP Snooping:</b></td>\n"));
	nBytesSent += websWrite(wp, T("<td width=350><font size=2>\n<input type=\"radio\""));
	nBytesSent += websWrite(wp, T(" name=snoop value=0>Disabled&nbsp;&nbsp;\n"));
	nBytesSent += websWrite(wp, T("<input type=\"radio\" name=snoop value=1>Enabled</td>\n</tr></table>\n"));
#endif
	
#ifdef WLAN_SUPPORT
	nBytesSent += websWrite(wp, T("<table border=0 width=\"500\" cellspacing=4 cellpadding=0>\n"));
  	nBytesSent += websWrite(wp, T("<tr><td width=150><font size=2><b>Ethernet to Wireless Blocking:</b></td>\n"));
  	nBytesSent += websWrite(wp, T("<td width=350><font size=2>\n"));
  	nBytesSent += websWrite(wp, T("<input type=\"radio\" name=BlockEth2Wir value=0>Disabled&nbsp;&nbsp;\n"));
  	nBytesSent += websWrite(wp, T("<input type=\"radio\" name=BlockEth2Wir value=1>Enabled</td></tr></table>\n"));
  
#endif
	return nBytesSent;
}
#endif

int checkIP2(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;
	
#ifdef SECONDARY_IP
	nBytesSent += websWrite(wp, T("if (document.tcpip.enable_ip2.checked) {\n"));
	nBytesSent += websWrite(wp, T("\tif (!checkIP(document.tcpip.ip2))\n"));
	nBytesSent += websWrite(wp, T("\t\treturn false;\n"));
	nBytesSent += websWrite(wp, T("\tif (!checkMask(document.tcpip.mask2))\n"));
	nBytesSent += websWrite(wp, T("\t\treturn false;\n"));
	nBytesSent += websWrite(wp, T("\t\tif (checkLan1andLan2(document.tcpip.ip, document.tcpip.mask, document.tcpip.ip2, document.tcpip.mask2) == false) {\n"));
#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
	nBytesSent += websWrite(wp, T("\t\talert(\"第一组 IP 地址和第二组 IP 地址必须属于不同的子网。\");\n"));
#else
	nBytesSent += websWrite(wp, T("\t\talert(\"Network Address Conflict !\");\n"));
#endif
	nBytesSent += websWrite(wp, T("\t\tdocument.tcpip.ip2.value=document.tcpip.ip2.defaultValue;\n"));
	nBytesSent += websWrite(wp, T("\t\tdocument.tcpip.ip2.focus();\n"));
	nBytesSent += websWrite(wp, T("\t\treturn false;}}\n"));
#endif
	return nBytesSent;
}

int lan_script(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;
	
#ifdef SECONDARY_IP
	nBytesSent += websWrite(wp, T("function updateInput()\n"));
	nBytesSent += websWrite(wp, T("{\n\tif (document.tcpip.enable_ip2.checked == true) {\n"));
	nBytesSent += websWrite(wp, T("\t\tif (document.getElementById)  // DOM3 = IE5, NS6\n"));
	nBytesSent += websWrite(wp, T("\t\t\tdocument.getElementById('secondIP').style.display = 'block';\n"));
	nBytesSent += websWrite(wp, T("\t\t\telse {\n"));
	nBytesSent += websWrite(wp, T("\t\t\tif (document.layers == false) // IE4\n"));
	nBytesSent += websWrite(wp, T("\t\t\t\tdocument.all.secondIP.style.display = 'block';\n"));
	nBytesSent += websWrite(wp, T("\t\t}\n"));
	nBytesSent += websWrite(wp, T("\t} else {\n"));
	nBytesSent += websWrite(wp, T("\t\tif (document.getElementById)  // DOM3 = IE5, NS6\n"));
	nBytesSent += websWrite(wp, T("\t\t\tdocument.getElementById('secondIP').style.display = 'none';\n"));
	nBytesSent += websWrite(wp, T("\t\telse {\n"));
	nBytesSent += websWrite(wp, T("\t\t\tif (document.layers == false) // IE4\n"));
	nBytesSent += websWrite(wp, T("\t\t\t\tdocument.all.secondIP.style.display = 'none';\n"));
	nBytesSent += websWrite(wp, T("\t\t}\n"));
	nBytesSent += websWrite(wp, T("\t}\n"));
	nBytesSent += websWrite(wp, T("}"));
#endif
	return nBytesSent;
}

