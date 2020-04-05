
/*
 *      Web server handler routines for MAC-Base Assignment for DHCP Server stuffs
 *
 */


/*-- System inlcude files --*/
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/route.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"


#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
#define GOOD_MSG(url) { \
	websHeader(wp); \
	websWrite(wp, T("<body><blockquote><h5>设置成功!<BR>" \
                "<BR><form><input type=button value=\"  确定  \" OnClick=window.location.replace(\"%s\")></form></blockquote></body>"), url);\
	websFooter(wp); \
	websDone(wp, 200); \
}
#else
#define GOOD_MSG(url) { \
	websHeader(wp); \
	websWrite(wp, T("<body><blockquote><h5>Change setting successfully!<BR>" \
                "<BR><form><input type=button value=\"  OK  \" OnClick=window.location.replace(\"%s\")></form></blockquote></body>"), url);\
	websFooter(wp); \
	websDone(wp, 200); \
}
#endif

///////////////////////////////////////////////////////////////////
void formmacBase(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str, *submitUrl;
	char tmpBuf[100];	
#ifndef NO_ACTION
	int pid;
#endif	
	unsigned long v1, v2;
	
//star: for take effect immediately
	int change_flag = 0;

	// Delete
	str = websGetVar(wp, T("delIP"), T(""));
	if (str[0]) {
		unsigned int i;
		unsigned int idx;
		unsigned int totalEntry = mib_chain_total(MIB_MAC_BASE_DHCP_TBL); /* get chain record size */		
		
		str = websGetVar(wp, T("select"), T(""));		
		//printf("str(select) = %s\n", str);
		
		if (str[0]) {
			for (i=0; i<totalEntry; i++) {
				idx = totalEntry-i-1;			
				snprintf(tmpBuf, 4, "s%d", idx);
				//printf("tmpBuf(select) = %s idx=%d\n", tmpBuf, idx);
				
				if ( !gstrcmp(str, T(tmpBuf)) ) {
					change_flag = 1;
					// delete from chain record
					if(mib_chain_delete(MIB_MAC_BASE_DHCP_TBL, idx) != 1) {
						strcpy(tmpBuf, T(strDelChainerror));
						goto setErr_route;
					}

					
				}
			} // end of for
		}		
		goto setOk_route;
	}

	// Add
	str = websGetVar(wp, T("addIP"), T(""));
	//printf("str=%s\n", str);	
	if (str[0]) {
		MIB_CE_MAC_BASE_DHCP_T entry,Entry;
		int mibtotal,i, intVal;
		unsigned char macAddr[MAC_ADDR_LEN];
		
		str = websGetVar(wp, T("hostMac"), T(""));
		// Modified by Mason Yu. 2008/12/26 					
		//strcpy(entry.macAddr, str);		
		
		
		for (i=0; i<17; i++){
			if ((i+1)%3 != 0)
				str[i-(i+1)/3] = str[i];
		}
		str[12] = '\0';
		if (strlen(str)!=12  || !string_to_hex(str, entry.macAddr_Dhcp, 12)) {
			strcpy(tmpBuf, T(strInvdMACAddr));
			goto setErr_route;
		}
		if (!isValidMacAddr(entry.macAddr_Dhcp)) {
			strcpy(tmpBuf, T(strInvdMACAddr));
			goto setErr_route;
		}
		
		
		str = websGetVar(wp, T("hostIp"), T(""));				
		//strcpy(entry.ipAddr, str);
		
		if (str[0]){
			if ( !inet_aton(str, (struct in_addr *)&entry.ipAddr_Dhcp ) ) {
				strcpy(tmpBuf, T(strIPAddresserror));
				goto setErr_route;
			}
		}

		//printf("entry.ipAddr = 0x%x\n", ((struct in_addr *)&entry.ipAddr_Dhcp)->s_addr);		
		
		mibtotal = mib_chain_total(MIB_MAC_BASE_DHCP_TBL);
		for(i=0;i<mibtotal;i++)
		{
			mib_chain_get(MIB_MAC_BASE_DHCP_TBL, i, (void*)&Entry);
			v1 = *((unsigned long *)Entry.ipAddr_Dhcp);
			v2 = *((unsigned long *)entry.ipAddr_Dhcp);
			
			if ( (	Entry.macAddr_Dhcp[0]==entry.macAddr_Dhcp[0] && Entry.macAddr_Dhcp[1]==entry.macAddr_Dhcp[1] && Entry.macAddr_Dhcp[2]==entry.macAddr_Dhcp[2] &&
	     			Entry.macAddr_Dhcp[3]==entry.macAddr_Dhcp[3] && Entry.macAddr_Dhcp[4]==entry.macAddr_Dhcp[4] && Entry.macAddr_Dhcp[5]==entry.macAddr_Dhcp[5] ) || (v1==v2)  ) {		     	
				strcpy(tmpBuf, T(strStaticipexist));
				goto setErr_route;
			}		
				
		}
			
		change_flag = 1;
		
		intVal = mib_chain_add(MIB_MAC_BASE_DHCP_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			strcpy(tmpBuf, T(strAddChainerror));
			goto setErr_route;
		}	
		else if (intVal == -1) {
			strcpy(tmpBuf, T(strTableFull));
			goto setErr_route;
		}	
	goto setOk_route;
	}

	//Mod
	str = websGetVar(wp, T("modIP"), T(""));
	//printf("str=%s\n", str);	
	if (str[0]) {
		unsigned int i, j;
		unsigned int idx;
		unsigned int totalEntry = mib_chain_total(MIB_MAC_BASE_DHCP_TBL); /* get chain record size */		
		
		str = websGetVar(wp, T("select"), T(""));		
		//printf("str(select) = %s\n", str);
		
		if (str[0]) {
			for (i=0; i<totalEntry; i++) {
				idx = totalEntry-i-1;			
				snprintf(tmpBuf, 4, "s%d", idx);
				//printf("tmpBuf(select) = %s idx=%d\n", tmpBuf, idx);
				
				if ( !gstrcmp(str, T(tmpBuf)) ) {
					change_flag = 1;
					MIB_CE_MAC_BASE_DHCP_T entry;				
		
					str = websGetVar(wp, T("hostMac"), T(""));					
					//strcpy(entry.macAddr, str);
					for (j=0; j<17; j++){
						if ((j+1)%3 != 0)
							str[j-(j+1)/3] = str[j];
					}
					str[12] = '\0';
					if (strlen(str)!=12  || !string_to_hex(str, entry.macAddr_Dhcp, 12)) {
						strcpy(tmpBuf, T(strInvdMACAddr));
						goto setErr_route;
					}
					if (!isValidMacAddr(entry.macAddr_Dhcp)) {
						strcpy(tmpBuf, T(strInvdMACAddr));
						goto setErr_route;
					}
		
					//printf("entry.macAddr = 0x%x\n", entry.macAddr);
					
					str = websGetVar(wp, T("hostIp"), T(""));				
					//strcpy(entry.ipAddr, str);
					inet_aton(str, (struct in_addr *)&entry.ipAddr_Dhcp);
					//printf("entry.ipAddr = 0x%\n", entry.ipAddr);
					
					if(mib_chain_update(MIB_MAC_BASE_DHCP_TBL, (unsigned char*)&entry, idx) != 1){
						strcpy(tmpBuf, T(strModChainerror));
						goto setErr_route;
					}	

					
				}
			} // end of for
		}
	}


setOk_route:
	/* upgdate to flash */
//	mib_update(CURRENT_SETTING);

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
	if(change_flag == 1)
	{
		unsigned char vChar;
// Kaohj
#if 0
#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		/*to avoid the dead when wrriting the flash*/
		itfcfg("sar", 0);
		itfcfg("eth0", 0);
#ifdef WLAN_SUPPORT
		itfcfg("wlan0", 0);
#endif
#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
#if !(defined (ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC))
		mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
#endif

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

		restart_dhcp();
		submitUrl = websGetVar(wp, T("submit-url"), T(""));
		GOOD_MSG(submitUrl);
		return;
	}

	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
  	return;

setErr_route:
	ERR_MSG(tmpBuf);
	
}

#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
int showMACBaseTable(int eid, webs_t wp, int argc, char_t **argv)
{		
	int nBytesSent=0;
	unsigned int entryNum, i;
	MIB_CE_MAC_BASE_DHCP_T Entry;	
	char macaddr[20];
	
	entryNum = mib_chain_total(MIB_MAC_BASE_DHCP_TBL);	
	
	nBytesSent += websWrite(wp, T("<tr><font size=1>"
	"<td align=center width=\"15%%\" bgcolor=\"#808080\">索引</td>\n"
	"<td align=center width=\"40%%\" bgcolor=\"#808080\">主机MAC地址</td>\n"
	"<td align=center width=\"40%%\" bgcolor=\"#808080\">分配的IP地址</td></font></tr>\n"));

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_MAC_BASE_DHCP_TBL, i, (void *)&Entry))
		{
  			websError(wp, 400, T("取得相关记录失败!\n"));
			return;
		}		
		
		snprintf(macaddr, 18, "%02x-%02x-%02x-%02x-%02x-%02x",
				Entry.macAddr_Dhcp[0], Entry.macAddr_Dhcp[1],
				Entry.macAddr_Dhcp[2], Entry.macAddr_Dhcp[3],
				Entry.macAddr_Dhcp[4], Entry.macAddr_Dhcp[5]);
				
		nBytesSent += websWrite(wp, T("<tr>"
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"select\""
		" value=\"s%d\" onClick = \"onSelect('%s','%s')\"></td>\n"
		"<td align=center width=\"40%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"40%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td></tr>\n"),
		i, macaddr, inet_ntoa(*((struct in_addr *)Entry.ipAddr_Dhcp)) , macaddr, inet_ntoa(*((struct in_addr *)Entry.ipAddr_Dhcp)) );
	}
	
	return 0;
}

#else
int showMACBaseTable(int eid, webs_t wp, int argc, char_t **argv)
{		
	int nBytesSent=0;
	unsigned int entryNum, i;
	MIB_CE_MAC_BASE_DHCP_T Entry;	
	struct in_addr matchIp;
	char macaddr[20];
	
	entryNum = mib_chain_total(MIB_MAC_BASE_DHCP_TBL);	
	
	nBytesSent += websWrite(wp, T("<tr><font size=1>"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\">Select</td>\n"
	"<td align=center width=\"40%%\" bgcolor=\"#808080\">Host MAC Address</td>\n"
	"<td align=center width=\"40%%\" bgcolor=\"#808080\">Assigned IP Address</td></font></tr>\n"));	
	
	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_MAC_BASE_DHCP_TBL, i, (void *)&Entry))
		{
  			websError(wp, 400, T("Get chain(MIB_MAC_BASE_DHCP_TBL) record error!\n"));
			return;
		}	
		
		//matchIp.s_addr = Entry.ipAddr_Dhcp;
		//strcpy(ip,inet_ntoa(matchIp));
	
		snprintf(macaddr, 18, "%02x-%02x-%02x-%02x-%02x-%02x",
				Entry.macAddr_Dhcp[0], Entry.macAddr_Dhcp[1],
				Entry.macAddr_Dhcp[2], Entry.macAddr_Dhcp[3],
				Entry.macAddr_Dhcp[4], Entry.macAddr_Dhcp[5]);	
		
		nBytesSent += websWrite(wp, T("<tr>"
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"select\""
		" value=\"s%d\"></td>\n"
		"<td align=center width=\"40%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"40%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td></tr>\n"),
		i, macaddr, inet_ntoa(*((struct in_addr *)Entry.ipAddr_Dhcp))    );
	}
	
	return 0;
}
#endif
