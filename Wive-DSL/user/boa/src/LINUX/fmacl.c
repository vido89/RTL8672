
/*
 *      Web server handler routines for ACL stuffs
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

#ifdef MAC_ACL
int obtainMacAclEntry(webs_t wp, MIB_CE_ACL_MAC_Tp pEntry)
{
	char_t	*str;
	char macAddr[18];
	char tmpBuf[100];
	
	str = websGetVar(wp, T("aclMac"), T(""));
	if (str[0]) {
		int i;
		memcpy(macAddr, str, 17);
		for(i=0;i<17;i++){
			if((i+1)%3 != 0)
				macAddr[i-(i+1)/3] = macAddr[i];
		}
		macAddr[12] = '\0';
		if ( !string_to_hex(macAddr, pEntry->macAddr, 12)) {
			strcpy(tmpBuf, T(Tinvalid_source_mac));
			goto setErr_route;
		}
	}
	str = websGetVar(wp, T("enablemac"), T(""));
	if ( str[0] == '1' ) {
		pEntry->Enabled = 1;
	}else
		pEntry->Enabled = 0;
	
	str = websGetVar(wp, T("macinterface"), T(""));			
	if ( str[0]=='0' ) {
		pEntry->Interface = IF_DOMAIN_LAN;   // LAN
	} else if ( str[0]=='1') {
		pEntry->Interface = IF_DOMAIN_WAN;   // WAN
	} else {
		strcpy(tmpBuf, T(strSetInterfaceerror));
		goto setErr_route;		
	}
	return 1;

setErr_route:
	ERR_MSG(tmpBuf);
	return 0;
}
#endif

int obtainIpAclEntry(webs_t wp, MIB_CE_ACL_IP_Tp pEntry)
{
	char_t	*str;
	char tmpBuf[100];
	int isnet;
	struct sockaddr acl_ip;
	unsigned long mask, mbit;
	struct in_addr *addr;
	unsigned char aclmask[4], aclip[4];
	int totalEntry, i;
	MIB_CE_ACL_IP_T Entry;
	char *temp;
	long nAclip;
	
#ifdef ACL_IP_RANGE
	struct in_addr curIpAddr,curSubnet,secondIpAddr,secondSubnet;
	char enable;
	unsigned long vstart,vend,v2,v3,v4,v5;
	
	mib_get( MIB_ADSL_LAN_IP,  (void *)&curIpAddr );
	mib_get( MIB_ADSL_LAN_SUBNET,  (void *)&curSubnet );
	mib_get( MIB_ADSL_LAN_ENABLE_IP2, (void *)&enable );
	mib_get( MIB_ADSL_LAN_IP2, (void *)&secondIpAddr );
	mib_get( MIB_ADSL_LAN_SUBNET2, (void *)&secondSubnet);

	v2 = *((unsigned long *)&curIpAddr);
	v3 = *((unsigned long *)&curSubnet);
	v4 = *((unsigned long *)&secondIpAddr);
	v5 = *((unsigned long *)&secondSubnet);

	str = websGetVar(wp, T("aclstartIP"), T(""));	
	if (str[0]){
		if ((isnet = INET_resolve(str, &acl_ip)) < 0) {
			snprintf(tmpBuf, 100, "Error: can't resolve %s", str);
			goto setErr_route;
		}

		vstart = *((unsigned long *)&(((struct sockaddr_in *)&acl_ip)->sin_addr));
		if ((vstart & v3) != (v2 & v3)) {
			if (enable) {
				if ((vstart & v5) == (v4 & v5)) {
					goto startIP_OK;
				}
			}
			strcpy(tmpBuf, T(Tinvalid_ip_net));
			goto setErr_route;
		}
startIP_OK:
		// add into configuration (chain record)
		addr = (struct in_addr *)&(pEntry->startipAddr);
		*addr = ((struct sockaddr_in *)&acl_ip)->sin_addr;
	}

	str = websGetVar(wp, T("aclendIP"), T(""));	
	if (str[0]){
		if ((isnet = INET_resolve(str, &acl_ip)) < 0) {
			snprintf(tmpBuf, 100, "Error: can't resolve %s", str);
			goto setErr_route;
		}

		vend = *((unsigned long *)&(((struct sockaddr_in *)&acl_ip)->sin_addr));
		if ((vend & v3) != (v2 & v3)) {
			if (enable) {
				if ((vend & v5) == (v4 & v5)) {
					printf("\nenter 1!\n");
					if((vend & v5) != (vstart & v5)){
						printf("\nenter 2!\n");
						strcpy(tmpBuf, T(strWrongRangeDifflan));
						goto setErr_route;
					}
					goto endIP_OK;
				}
			}
			strcpy(tmpBuf, T(Tinvalid_ip_net));
			goto setErr_route;
		}
		if((vend & v3) != (vstart & v3)){
			strcpy(tmpBuf, T(strWrongRangeDifflan));
			goto setErr_route;
		}

endIP_OK:
		// add into configuration (chain record)
		addr = (struct in_addr *)&(pEntry->endipAddr);
		*addr = ((struct sockaddr_in *)&acl_ip)->sin_addr;
	}

	if((int)(vstart-vend )> 0){
		strcpy(tmpBuf, T(strWrongRange));
		goto setErr_route;
	}

#else
	str = websGetVar(wp, T("aclIP"), T(""));	
	if (str[0]){
		if ((isnet = INET_resolve(str, &acl_ip)) < 0) {
			snprintf(tmpBuf, 100, "Error: can't resolve %s", str);
			goto setErr_route;
		}

		// add into configuration (chain record)
		addr = (struct in_addr *)&(pEntry->ipAddr);
		*addr = ((struct sockaddr_in *)&acl_ip)->sin_addr;
	}

	str = websGetVar(wp, T("aclMask"), T(""));	
	if (str[0]) {
		if (!isValidNetmask(str, 0)) {
			strcpy(tmpBuf, T(strWrongMask));
			goto setErr_route;
		}
		inet_aton(str, (struct in_addr *)aclmask);
		inet_aton(str, (struct in_addr *)&mask);
		mbit=0;
		while (1) {
			if (mask&0x80000000) {
				mbit++;
				mask <<= 1;
			}
			else
				break;
		}
		pEntry->maskbit = mbit;
	}
	// Jenny, for checking duplicated acl IP address
	aclip[0] = pEntry->ipAddr[0] & aclmask[0];
	aclip[1] = pEntry->ipAddr[1] & aclmask[1];
	aclip[2] = pEntry->ipAddr[2] & aclmask[2];
	aclip[3] = pEntry->ipAddr[3] & aclmask[3];

	totalEntry = mib_chain_total(MIB_ACL_IP_TBL);
	temp = inet_ntoa(*((struct in_addr *)aclip));
	nAclip = ntohl(inet_addr(temp));
	for (i=0; i<totalEntry; i++) {
		unsigned long v1, v2, pAclip;
		int m;
		if (!mib_chain_get(MIB_ACL_IP_TBL, i, (void *)&Entry)) {
			strcpy(tmpBuf, T(strGetChainerror));
			goto setErr_route;
		}		
		temp[0] = '\0';
		temp = inet_ntoa(*((struct in_addr *)Entry.ipAddr));
		v1 = ntohl(inet_addr(temp));
		v2 = 0xFFFFFFFFL;
		for (m=32; m>Entry.maskbit; m--) {
			v2 <<= 1;
			v2 |= 0x80000000;
		}
		pAclip = v1&v2;
		// If all parameters of Entry are all the same as mew rule, drop this new rule.
		if (nAclip == pAclip && Entry.Interface == pEntry->Interface) {			
			strcpy(tmpBuf, T(Tinvalid_rule));
			goto setErr_route;
		}
	}
#endif

	str = websGetVar(wp, T("enable"), T(""));
	if ( str && str[0] ) {
		pEntry->Enabled = 1;
	}
	
	str = websGetVar(wp, T("interface"), T(""));			
	if ( str[0]=='0' ) {		
		pEntry->Interface = IF_DOMAIN_LAN;   // LAN
	} else if ( str[0]=='1') {		
		pEntry->Interface = IF_DOMAIN_WAN;   // WAN
	} else {
		strcpy(tmpBuf, T(strSetInterfaceerror));
		goto setErr_route;		
	}
	return 1;
	
setErr_route:
	ERR_MSG(tmpBuf);
	return 0;

}

#ifdef IP_ACL
///////////////////////////////////////////////////////////////////
 #if defined( ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
int aclapplyflag=0;
#endif
void formACL(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str, *submitUrl, *strVal;
	char tmpBuf[100];
	struct rtentry rt;
	int xflag, isnet;
	int skfd;
	struct sockaddr acl_ip;
	unsigned char aclcap;
	struct in_addr *addr;
	MIB_CE_ACL_IP_T entry;
#ifdef MAC_ACL
	MIB_CE_ACL_MAC_T macEntry;	
#endif
#ifndef NO_ACTION
	int pid;
#endif
	unsigned char vChar;

	// Set ACL Capability
	str = websGetVar(wp, T("apply"), T(""));
	if (str[0]) {
		str = websGetVar(wp, T("aclcap"), T(""));
		if (str[0]) {
			if (str[0] == '0')
				aclcap = 0;
			else
				aclcap = 1;
			if ( !mib_set(MIB_ACL_CAPABILITY, (void *)&aclcap)) {
				strcpy(tmpBuf, T(strSetACLCAPerror));
				goto setErr_route;
			}
		}
		goto setOk_route;
 	}
 #if defined( ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
	str = websGetVar(wp, T("effect"), T(""));
	if(str[0])
	{
		aclapplyflag = 0;
		goto setEffect_route;
	}
 #endif	
	// Delete all IP
	str = websGetVar(wp, T("delAllIP"), T(""));
	if (str[0]) {
		mib_chain_clear(MIB_ACL_IP_TBL); /* clear chain record */
		goto setOk_route;
	}

	/* Delete selected IP */
	str = websGetVar(wp, T("delIP"), T(""));
	if (str[0]) {
		unsigned int i;
		unsigned int idx;
		unsigned int totalEntry = mib_chain_total(MIB_ACL_IP_TBL); /* get chain record size */
		unsigned int deleted = 0;

		for (i=0; i<totalEntry; i++) {

			idx = totalEntry-i-1;			
			snprintf(tmpBuf, 20, "select%d", idx);
			strVal = websGetVar(wp, tmpBuf, T(""));
			
			if ( !gstrcmp(strVal, T("ON")) ) {						
				deleted ++;
				if(mib_chain_delete(MIB_ACL_IP_TBL, idx) != 1) {
					strcpy(tmpBuf, T("Delete chain record error!"));
					goto setErr_route;
				}
			}
		}		
		if (deleted <= 0) {
			strcpy(tmpBuf, T("There is no item selected to delete!"));
			goto setErr_route;
		}
		
		goto setOk_route;
	}

#if 0
	// Delete IP
	str = websGetVar(wp, T("delIP"), T(""));
	if (str[0]) {
		unsigned int i;
		unsigned int idx;
		MIB_CE_ACL_IP_T Entry;
		unsigned int totalEntry = mib_chain_total(MIB_ACL_IP_TBL); /* get chain record size */
		struct in_addr dest;		
		char sdest[16];
		
		str = websGetVar(wp, T("select"), T(""));		
		
		if (str[0]) {
			for (i=0; i<totalEntry; i++) {
				idx = totalEntry-i-1;			
				snprintf(tmpBuf, 4, "s%d", idx);
				
				if ( !gstrcmp(str, T(tmpBuf)) ) {
					struct sockaddr_in *s_in;
					/* get the specified chain record */
					if (!mib_chain_get(MIB_ACL_IP_TBL, idx, (void *)&Entry)) {
						strcpy(tmpBuf, errGetEntry);
						goto setErr_route;
					}
					
					//printf("Entry.ipAddr = %s\n", Entry.ipAddr);
					//dest.s_addr = *(unsigned long *)Entry.ipAddr;
		
					// inet_ntoa is not reentrant, we have to
					// copy the static memory before reuse it
					//strcpy(sdest, inet_ntoa(dest));
					
					// iptables -D INPUT -s xxx.xxx.xxx.xxx		
					//va_cmd(IPTABLES, 6, 1, "-D", "INPUT", "-s", sdest, "-j", "ACCEPT");	
					
					// delete from chain record
					if(mib_chain_delete(MIB_ACL_IP_TBL, idx) != 1) {
						strcpy(tmpBuf, T(strDelChainerror));
						goto setErr_route;
					}					
				}
			} // end of for
		}		
		goto setOk_route;
	}
#endif	

	//Delete MAC
#ifdef MAC_ACL
	str = websGetVar(wp, T("delMac"), T(""));
	if (str[0]) {
		unsigned int i;
		unsigned int idx;
		MIB_CE_ACL_MAC_T Entry;
		unsigned int totalEntry = mib_chain_total(MIB_ACL_MAC_TBL); /* get chain record size */
		
		str = websGetVar(wp, T("selectMac"), T(""));		
		
		if (str[0]) {
			for (i=0; i<totalEntry; i++) {
				idx = totalEntry-i-1;
				snprintf(tmpBuf, 4, "ms%d", idx);
				
				if ( !gstrcmp(str, T(tmpBuf)) ) {
					// delete from chain record
					if(mib_chain_delete(MIB_ACL_MAC_TBL, idx) != 1) {
						strcpy(tmpBuf, T(strDelChainerror));
						goto setErr_route;
					}
				}
			} // end of for
		}
		goto setOk_route;
	}
#endif	

	memset(&entry, '\0', sizeof(MIB_CE_ACL_IP_T));
			
#ifdef MAC_ACL
	// Update
	str = websGetVar(wp, T("updateMacACL"), T(""));
	if (str[0]) {
		unsigned int i, j;
		unsigned int idx;
		MIB_CE_ACL_MAC_T Entry, tmpEntry;
		unsigned int totalEntry = mib_chain_total(MIB_ACL_MAC_TBL); /* get chain record size */

		memset(&macEntry, 0, sizeof(MIB_CE_ACL_MAC_T));
		if ( !obtainMacAclEntry(wp, &macEntry))
			return;
		
		str = websGetVar(wp, T("selectMac"), T(""));
		if (str[0]) {
			for (i=0; i<totalEntry; i++) {
				idx = totalEntry-i-1;			
				snprintf(tmpBuf, 4, "ms%d", idx);
				
				if ( !gstrcmp(str, T(tmpBuf)) ) {
					//check if the mac existed.
					for (j=0; j<totalEntry; j++) {
						if (j==idx)
							continue;
						if(!mib_chain_get(MIB_ACL_MAC_TBL, j, (void*)&tmpEntry))
							continue;
						if ( (tmpEntry.macAddr[0]==macEntry.macAddr[0]) && 
							(tmpEntry.macAddr[1]==macEntry.macAddr[1]) &&
							(tmpEntry.macAddr[2]==macEntry.macAddr[2]) &&
							(tmpEntry.macAddr[3]==macEntry.macAddr[3]) &&
							(tmpEntry.macAddr[4]==macEntry.macAddr[4]) &&
							(tmpEntry.macAddr[5]==macEntry.macAddr[5])) {
							snprintf(tmpBuf, 100, strMacExist, str);
							goto setErr_route;
						}
					}
					
					mib_chain_update(MIB_ACL_MAC_TBL, &macEntry, idx);
					break;
				}
			} // end of for
		}
		goto setOk_route;
	}
	
	// Add
	str = websGetVar(wp, T("addMac"), T(""));
	if (str[0]) {
		int mibtotal,i, intVal;
		MIB_CE_ACL_MAC_T tmpentry;

		memset(&macEntry, 0, sizeof(MIB_CE_ACL_MAC_T));
		if ( !obtainMacAclEntry(wp, &macEntry))
			return;
		
		mibtotal = mib_chain_total(MIB_ACL_MAC_TBL);
		for(i=0;i<mibtotal;i++)
		{
			if(!mib_chain_get(MIB_ACL_MAC_TBL,i,(void*)&tmpentry))
				continue;
			if( (tmpentry.macAddr[0]==macEntry.macAddr[0]) &&
				(tmpentry.macAddr[1]==macEntry.macAddr[1]) &&
				(tmpentry.macAddr[2]==macEntry.macAddr[2]) &&
				(tmpentry.macAddr[3]==macEntry.macAddr[3]) &&
				(tmpentry.macAddr[4]==macEntry.macAddr[4]) &&
				(tmpentry.macAddr[5]==macEntry.macAddr[5]) )
			{
				snprintf(tmpBuf, 100, strMacExist, str);
				goto setErr_route;
			}
		}
			
		intVal = mib_chain_add(MIB_ACL_MAC_TBL, (unsigned char*)&macEntry);
		if (intVal == 0) {
			//websWrite(wp, T("%s"), strAddChainerror);
			//return;
			strcpy(tmpBuf, T(strAddChainerror));
			goto setErr_route;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, T(strTableFull));
			goto setErr_route;
		}
		
		goto setOk_route;
	}
#endif
	
	// Update
	str = websGetVar(wp, T("updateACL"), T(""));
	if (str[0]) {
		unsigned int i;
		unsigned int idx;
		MIB_CE_ACL_IP_T Entry;
		unsigned int totalEntry = mib_chain_total(MIB_ACL_IP_TBL); /* get chain record size */
		struct in_addr dest;		
		char sdest[16];

		memset(&entry, 0, sizeof(MIB_CE_ACL_IP_T));
		if ( !obtainIpAclEntry(wp, &entry))
			return;
		
		str = websGetVar(wp, T("select"), T(""));		
		
		if (str[0]) {
			for (i=0; i<totalEntry; i++) {
				idx = totalEntry-i-1;			
				snprintf(tmpBuf, 4, "s%d", idx);
				
				if ( !gstrcmp(str, T(tmpBuf)) ) {					
					mib_chain_update(MIB_ACL_IP_TBL, &entry, idx);
					//ql_xu add
					break;
				}
			} // end of for
		}
		
		goto setOk_route;
	}
	
	// Add
	str = websGetVar(wp, T("addIP"), T(""));
	if (str[0]) {
		int mibtotal,i, intVal;
		MIB_CE_ACL_IP_T tmpentry;

		memset(&entry, 0, sizeof(MIB_CE_ACL_IP_T));
		if ( !obtainIpAclEntry(wp, &entry))
			return;

		mibtotal = mib_chain_total(MIB_ACL_IP_TBL);
		for(i=0;i<mibtotal;i++)
		{
			if(!mib_chain_get(MIB_ACL_IP_TBL,i,(void*)&tmpentry))
				continue;
#ifdef ACL_IP_RANGE
			if(((*(int*)(entry.startipAddr))==(*(int*)(tmpentry.startipAddr)))&&((*(int*)(entry.endipAddr))==(*(int*)(tmpentry.endipAddr))))
#else
			if((*(int*)(entry.ipAddr))==(*(int*)(tmpentry.ipAddr)))
#endif
			{
				snprintf(tmpBuf, 100, strIpExist, str);
				goto setErr_route;
			}
		}
			
		intVal = mib_chain_add(MIB_ACL_IP_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			//websWrite(wp, T("%s"), strAddChainerror);
			//return;
			strcpy(tmpBuf, T(strAddChainerror));
			goto setErr_route;
		}		
		else if (intVal == -1) {
			strcpy(tmpBuf, T(strTableFull));
			goto setErr_route;
		}		
	}


setOk_route:
	restart_acl();
	
	/* upgdate to flash */
//	mib_update(CURRENT_SETTING);
 #if defined( ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
	aclapplyflag = 1;
#endif

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

	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
  	return;
 #if defined( ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
setEffect_route:
 #if 0
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
//		RECONNECT_MSG(strIp);
//		req_flush(wp);
#endif
	restart_acl();
	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	OK_MSG(submitUrl);
	return;
#endif
setErr_route:
	ERR_MSG(tmpBuf);
}

int showACLTable(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;
	
	unsigned int entryNum, i;
	MIB_CE_ACL_IP_T Entry;
	unsigned long int d,g,m;
	struct in_addr dest;
	struct in_addr gw;
	struct in_addr mask;
	char sdest[35], sgw[16];
	
	entryNum = mib_chain_total(MIB_ACL_IP_TBL);
 #if defined( ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
      unsigned char validAclListNum=0;
#endif	
	nBytesSent += websWrite(wp, T("<tr><font size=1>")
 #if defined( ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
 #ifdef ACL_IP_RANGE
	T("<td align=center width=\"5%%\" bgcolor=\"#808080\">选择</td>\n"
	"<td align=center width=\"10%%\" bgcolor=\"#808080\">状态</td>"
	"<td align=center width=\"10%%\" bgcolor=\"#808080\">接口</td>"
	"<td align=center width=\"40%%\" bgcolor=\"#808080\">IP地址或地址范围</td></font></tr>\n"));
 #else
	T("<td align=center width=\"5%%\" bgcolor=\"#808080\">选择</td>\n"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\">状态</td>\n"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\">接口</td>\n"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\">IP地址</td></font></tr>\n"));
 #endif
#else
	T("<td align=center width=\"5%%\" bgcolor=\"#808080\">Select</td>\n"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\">state</td>\n"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\">Interface</td>\n"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\">IP Address</td></font></tr>\n"));
#endif

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_ACL_IP_TBL, i, (void *)&Entry))
		{
  			websError(wp, 400, T("Get chain record error!\n"));
			return;
		}

#ifdef ACL_IP_RANGE
		struct in_addr startip,endip;
		char sstart[16],send[16];

		startip.s_addr = *(unsigned long *)Entry.startipAddr;
		endip.s_addr = *(unsigned long *)Entry.endipAddr;

		strcpy(sstart, inet_ntoa(startip));
		strcpy(send, inet_ntoa(endip));
		strcpy(sdest, sstart);
		if(strcmp(sstart,send)){
			strcat(sdest, "-");
			strcat(sdest, send);
		}

#else
		dest.s_addr = *(unsigned long *)Entry.ipAddr;
		
		// inet_ntoa is not reentrant, we have to
		// copy the static memory before reuse it
		strcpy(sdest, inet_ntoa(dest));
		snprintf(sdest, 20, "%s/%d", sdest, Entry.maskbit);
#endif
	
		nBytesSent += websWrite(wp, T("<tr>"
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td>\n")
#if 0
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"select\""
		" value=\"s%d\""
#ifdef ACL_IP_RANGE
		T("onClick=\"postACLrange(%d, %d, '%s', '%s')\"")
#else
		T("onClick=\"postACL(%d, %d, '%s')\"")
#endif
		T("></td>\n")
#endif
#if defined( ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
		T("<input type=\"hidden\" id=\"aclItemEnable%d\" value=%d>")
#endif
		T("<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\" ><font size=\"2\"><b>%s</b></font></td>"
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>"
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>"
		"</tr>\n"),
		i, 
#if 0
#ifdef ACL_IP_RANGE
		Entry.Enabled, (Entry.Interface == IF_DOMAIN_LAN)?0:1, sstart, send,
#else
		Entry.Enabled, (Entry.Interface == IF_DOMAIN_LAN)?0:1, sdest,
#endif
#endif
#if defined( ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
		i, Entry.Enabled,Entry.Enabled ? "生效" : "无效", (Entry.Interface == IF_DOMAIN_LAN)? "LAN" : "WAN", sdest);
#else		
		Entry.Enabled ? "Enable" : "Disable", (Entry.Interface == IF_DOMAIN_LAN)? "LAN" : "WAN", sdest);
#endif
#if defined( ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
		if(Entry.Enabled) validAclListNum++;
#endif
	}
#if defined( ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
	nBytesSent += websWrite(wp, T("<input type=\"hidden\" value=%d name=\"listnumber\">\n"), entryNum);
	 nBytesSent += websWrite(wp, T("<input type=\"hidden\" value=%d name=\"validAclListNum\">\n"), validAclListNum);
#endif
	
	return 0;
}

#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
int showACLIpAddr(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;
#ifdef ACL_IP_RANGE
	nBytesSent += websWrite(wp, T("<table border=0 width=\"520\" cellspacing=4 cellpadding=0>"));
	nBytesSent += websWrite(wp, T("<tr>"
	"<td width=\"30%%\"><font size=2><b>IP地址范围:</b></td>\n"
	"<td width=\"70%%\"><input type=\"text\" name=\"aclstartIP\" value=%s size=\"15\" maxlength=\"15\">－"
	"<input type=\"text\" name=\"aclendIP\" value=%s size=\"15\" maxlength=\"15\"></td>"
	"</tr></table>\n"),wp->remote_ip_addr,wp->remote_ip_addr);
	nBytesSent += websWrite(wp, T("<table border=0 width=\"520\" cellspacing=4 cellpadding=0>"));
	nBytesSent += websWrite(wp, T(" <tr><td width=\"70%%\"><font size=2>(当起始IP和结束IP相同时，则设置单一IP。)</font></td></tr></table>"));
#else
	nBytesSent += websWrite(wp, T("<table border=0 width=\"520\" cellspacing=4 cellpadding=0>"));
	nBytesSent += websWrite(wp, T(" <tr><td width=\"30%%\"><font size=2><b>IP地址:</b></td>"));
	nBytesSent += websWrite(wp, T(" <td width=\"70%%\"><input type=\"text\" name=\"aclIP\""
		"value=%s size=\"15\" maxlength=\"15\"></td></tr></table>"),wp->remote_ip_addr);
#endif
	return 0;
}
#endif
#endif

#ifdef MAC_ACL
int showACLMacTable(int eid, webs_t wp, int argc, char_t * * argv)
{
	int nBytesSent=0;
	unsigned int entryNum, i;
	MIB_CE_ACL_MAC_T Entry;
	char tmpBuf[100];
 #if defined( ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
	unsigned char validMacAclListNum=0;
#endif	
	
	entryNum = mib_chain_total(MIB_ACL_MAC_TBL);

	nBytesSent += websWrite(wp, T("<tr><font size=1>"
#if defined( ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
		"<td align=center width=\"5%%\" bgcolor=\"#808080\">选择</td>\n"
		"<td align=center width=\"20%%\" bgcolor=\"#808080\">状态</td>\n"
		"<td align=center width=\"20%%\" bgcolor=\"#808080\">接口</td>\n"
		"<td align=center width=\"20%%\" bgcolor=\"#808080\">MAC 地址</td></font></tr>\n"));
#else
		"<td align=center width=\"5%%\" bgcolor=\"#808080\">Select</td>\n"
		"<td align=center width=\"20%%\" bgcolor=\"#808080\">state</td>\n"
		"<td align=center width=\"20%%\" bgcolor=\"#808080\">Interface</td>\n"
		"<td align=center width=\"20%%\" bgcolor=\"#808080\">MAC Address</td></font></tr>\n"));
#endif

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_ACL_MAC_TBL, i, (void *)&Entry))
		{
  			websError(wp, 400, T(Tget_chain_err));
			return;
		}

		snprintf(tmpBuf, 100, T("%02x-%02x-%02x-%02x-%02x-%02x"),
			Entry.macAddr[0], Entry.macAddr[1], Entry.macAddr[2],
			Entry.macAddr[3], Entry.macAddr[4], Entry.macAddr[5]);
		
		nBytesSent += websWrite(wp, T("<tr>"
			"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"selectMac\""
			" value=\"ms%d\""
			" onClick=\"postACL2(%d, %d, '%s')\""
			"></td>\n"
#if defined( ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
			"<input type=\"hidden\" id=\"aclMacItemEnable%d\" value=%d>\n"
#endif
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\" ><font size=\"2\"><b>%s</b></font></td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>"
			"</tr>\n"),
			i, 
			Entry.Enabled, (Entry.Interface == IF_DOMAIN_LAN)?0:1, tmpBuf,
#if defined( ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
			i, Entry.Enabled,
			Entry.Enabled?"生效":"无效", (Entry.Interface==IF_DOMAIN_LAN)?"LAN":"WAN", tmpBuf);
#else		
			Entry.Enabled?"Enable":"Disable", (Entry.Interface==IF_DOMAIN_LAN)?"LAN":"WAN", tmpBuf);
#endif
#if defined( ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
		if(Entry.Enabled) validMacAclListNum++;
#endif
	}
#if defined( ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
	nBytesSent += websWrite(wp, T("<input type=\"hidden\" value=%d name=\"Maclistnumber\">\n"), entryNum);
	nBytesSent += websWrite(wp, T("<input type=\"hidden\" value=%d name=\"validMacAclListNum\">\n"), validMacAclListNum);
#endif
	
	return 0;
}

#endif

#ifdef NAT_CONN_LIMIT
int showConnLimitTable(int eid, webs_t wp, int argc, char_t * * argv)
{
	int nBytesSent=0;
		
	unsigned int entryNum, i;
	MIB_CE_CONN_LIMIT_T Entry;
	struct in_addr dest;
	char sdest[16];
		
	entryNum = mib_chain_total(MIB_CONN_LIMIT_TBL);

	nBytesSent += websWrite(wp, T("<tr><font size=1>"
		"<td align=center width=\"5%%\" bgcolor=\"#808080\">选择</td>\n"
		"<td align=center width=\"20%%\" bgcolor=\"#808080\">状态</td>\n"
		"<td align=center width=\"20%%\" bgcolor=\"#808080\">连接数</td>\n"
		"<td align=center width=\"20%%\" bgcolor=\"#808080\">IP地址</td></font></tr>\n"));
	
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_CONN_LIMIT_TBL, i, (void *)&Entry))
		{
			websError(wp, 400, T("Get chain record error!\n"));
			return;
		}
				
		dest.s_addr = *(unsigned long *)Entry.ipAddr;
		strcpy(sdest, inet_ntoa(dest));
		
		nBytesSent += websWrite(wp, T("<tr>"
			"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"select\""
			" value=\"s%d\""
			" onClick=\"postConn(%d, %d, '%s')\""
			"></td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\" ><font size=\"2\"><b>%s</b></font></td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%d</b></font></td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>"
			"</tr>\n"),
			i, 
			Entry.Enabled, Entry.connNum, sdest,
			Entry.Enabled ? "生效" : "无效", Entry.connNum, sdest);
	}
#if defined( ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
	nBytesSent += websWrite(wp, T("<input type=\"hidden\" value=%d name=\"listnumber\">\n"), entryNum);
#endif
	
	return 0;
}

int connlimitflag=0;
void formConnlimit(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str, *submitUrl;
	char tmpBuf[100];
	struct sockaddr conn_ip;
	unsigned char connlimitEn;
	MIB_CE_CONN_LIMIT_T entry;
#ifndef NO_ACTION
	int pid;
#endif
	unsigned char vChar;
	
	// Set nat session num limit Capability
	str = websGetVar(wp, T("apply"), T(""));
	if (str[0]) {
		str = websGetVar(wp, T("enableConnlimit"), T(""));
		if (str[0] == '1')
			connlimitEn = 1;
		else
			connlimitEn = 0;
		if ( !mib_set(MIB_NAT_CONN_LIMIT, (void *)&connlimitEn)) {
			strcpy(tmpBuf, T(strSetNatSessionError));
			goto setErr_route;
		}
		goto setOk_route;
	}
 #if defined( ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
	str = websGetVar(wp, T("effect"), T(""));
	if(str[0])
	{
		connlimitflag= 0;
		goto setEffect_route;
	}
 #endif	

 	// Delete IP
	str = websGetVar(wp, T("del"), T(""));
	if (str[0]) {
		unsigned int i;
		unsigned int idx;
		unsigned int totalEntry = mib_chain_total(MIB_CONN_LIMIT_TBL); /* get chain record size */
			
		str = websGetVar(wp, T("select"), T(""));		
		if (str[0]) {
			for (i=0; i<totalEntry; i++) {
				idx = totalEntry-i-1;			
				snprintf(tmpBuf, 4, "s%d", idx);
				
				if ( !gstrcmp(str, T(tmpBuf)) ) {
					// delete from chain record
					if(mib_chain_delete(MIB_CONN_LIMIT_TBL, idx) != 1) {
						strcpy(tmpBuf, T(strDelChainerror));
						goto setErr_route;
					}					
				}
			} // end of for
		}		
		goto setOk_route;
	}		
		
	// Update
	str = websGetVar(wp, T("update"), T(""));
	if (str[0]) {
		unsigned int i, j;
		unsigned int idx;
		MIB_CE_CONN_LIMIT_T Entry;
		unsigned int totalEntry = mib_chain_total(MIB_CONN_LIMIT_TBL); /* get chain record size */
		struct in_addr dest;
		char sdest[16];
		
		memset(&entry, 0, sizeof(MIB_CE_CONN_LIMIT_T));
		str = websGetVar(wp, T("IP"), T(""));	
		if (str[0]){
			if ( !inet_aton(str, (struct in_addr *)&entry.ipAddr) ) {
				strcpy(tmpBuf, T(strIPAddresserror));
				goto setErr_route;
			}
		}

		str = websGetVar(wp, T("enable"), T(""));
		if ( str && str[0] ) {
			entry.Enabled = 1;
		}
	
		str = websGetVar(wp, T("connNum"), T(""));			
		if ( str[0] ) {
			unsigned int vUInt;
			sscanf(str, "%u", &vUInt);
			entry.connNum = vUInt;
		}

		str = websGetVar(wp, T("select"), T(""));		
		if (str[0]) {
			for (i=0; i<totalEntry; i++) {
				idx = totalEntry-i-1;			
				snprintf(tmpBuf, 4, "s%d", idx);
				
				if ( !gstrcmp(str, T(tmpBuf)) ) {
					//check if the ip has been existed
					for(j=0; j<totalEntry; j++)
					{
						if (j == idx)
							continue;
						if(!mib_chain_get(MIB_CONN_LIMIT_TBL, j, (void*)&Entry))
							continue;
						
						if((*(int*)(entry.ipAddr))==(*(int*)(Entry.ipAddr)))
						{
							snprintf(tmpBuf, 100, strIpExist, str);
							goto setErr_route;
						}
					}
					
					mib_chain_update(MIB_CONN_LIMIT_TBL, &entry, idx);
					break;
				}
			} // end of for
		}
		
		goto setOk_route;
	}
	
	// Add
	str = websGetVar(wp, T("add"), T(""));
	if (str[0]) {
		int mibtotal,i, intVal;
		MIB_CE_CONN_LIMIT_T tmpentry;
		
		memset(&entry, 0, sizeof(MIB_CE_CONN_LIMIT_T));
		str = websGetVar(wp, T("IP"), T(""));	
		if (str[0]){
			if ( !inet_aton(str, (struct in_addr *)&entry.ipAddr) ) {
				strcpy(tmpBuf, T(strIPAddresserror));
				goto setErr_route;
			}
		}

		str = websGetVar(wp, T("enable"), T(""));
		if ( str && str[0] ) {
			entry.Enabled = 1;
		}
	
		str = websGetVar(wp, T("connNum"), T(""));			
		if ( str[0] ) {
			unsigned int vUInt;
			sscanf(str, "%u", &vUInt);
			entry.connNum = vUInt;
		}
		
		mibtotal = mib_chain_total(MIB_CONN_LIMIT_TBL);
		for(i=0;i<mibtotal;i++)
		{
			if(!mib_chain_get(MIB_CONN_LIMIT_TBL,i,(void*)&tmpentry))
				continue;
			if((*(int*)(entry.ipAddr))==(*(int*)(tmpentry.ipAddr)))
			{
				snprintf(tmpBuf, 100, strIpExist, str);
				goto setErr_route;
			}
		}
		
		intVal = mib_chain_add(MIB_CONN_LIMIT_TBL, (void *)&entry);
		if (intVal == 0) {
			//websWrite(wp, T("%s"), strAddChainerror);
			//return;
			strcpy(tmpBuf, T(strAddChainerror));
			goto setErr_route;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, T(strTableFull));
			goto setErr_route;
		}
	}
	
setOk_route:
	/* upgdate to flash */
	//	mib_update(CURRENT_SETTING);
	connlimitflag= 1;
	
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
	
	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
	return;

setEffect_route:
	restart_connlimit();
	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	OK_MSG(submitUrl);
	return;

setErr_route:
	ERR_MSG(tmpBuf);
}

#endif

#ifdef TCP_UDP_CONN_LIMIT
int showConnLimitTable(int eid, webs_t wp, int argc, char_t * * argv)
{
	int 						nBytesSent=0;
	unsigned int 				entryNum, i;
	MIB_CE_TCP_UDP_CONN_LIMIT_T 	Entry;
	struct in_addr 			dest;
	char 					sdest[16];
		
	entryNum = mib_chain_total(MIB_TCP_UDP_CONN_LIMIT_TBL);

	nBytesSent += websWrite(wp, 
		T("<tr><td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td>"
		"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Local IP Address</b></font></td>"
		"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Protocol</b></font></td>"
		"<td align=center width=\"25%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Max ports</b></font></td></tr>"));
	
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_TCP_UDP_CONN_LIMIT_TBL, i, (void *)&Entry))
		{
			websError(wp, 400, T("Get chain record error!\n"));
			return;
		}
				
		dest.s_addr = *(unsigned long *)Entry.ipAddr;
		strcpy(sdest, inet_ntoa(dest));

		nBytesSent += websWrite(wp, T("<tr>"
			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td>\n"
			"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\" ><font size=\"2\"><b>%s</b></font></td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
			"<td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%d</b></font></td>"
			"</tr>\n"),
			i, 
			sdest, Entry.protocol? "UDP":"TCP", 
			Entry.connNum );

	}	
	return 0;
}

int connlimitflag=0;
#define debug
void formConnlimit(webs_t wp, char_t *path, char_t *query)
{
	char							*str, *submitUrl;
	char 						tmpBuf[100];
	struct sockaddr 				conn_ip;
	unsigned char 				connlimitEn;
	MIB_CE_TCP_UDP_CONN_LIMIT_T	entry;
	int							tmpi;
	unsigned char 				vChar;
	
	// Set nat session num limit Capability
	str = websGetVar(wp, T("apply"), T(""));

	if (str[0]) {
		str = websGetVar(wp, T("connLimitcap"), T(""));

		if (str[0] == '1')
			connlimitEn = 1;
		else
			connlimitEn = 0;
		if ( !mib_set(MIB_CONNLIMIT_ENABLE, (void *)&connlimitEn)) {
			strcpy(tmpBuf, T(strSetNatSessionError));
			goto setErr_route;
		}
		str = websGetVar(wp, T("tcpconnlimit"), T(""));
				
		sscanf(str, "%d",&tmpi);		
		if (tmpi  < 10000){
			if ( !mib_set(MIB_CONNLIMIT_TCP, (void *)&tmpi)) {
				strcpy(tmpBuf, T(strSetNatSessionError));
				goto setErr_route;
			}
		}
		str = websGetVar(wp, T("udpconnlimit"), T(""));
		sscanf(str, "%d",&tmpi);
			printf("tmpi: %d", tmpi);
		if (tmpi  < 10000){
			if ( !mib_set(MIB_CONNLIMIT_UDP, (void *)&tmpi)) {
				strcpy(tmpBuf, T(strSetNatSessionError));
				goto setErr_route;
			}
		}		

		goto setEffect_route;
	}

/* Delete entry */
		str = websGetVar(wp, T("deleteSelconnLimit"), T(""));
	if (str[0])
	{
		unsigned int i;
		unsigned int idx;
		unsigned int totalEntry = mib_chain_total(MIB_TCP_UDP_CONN_LIMIT_TBL); /* get chain record size */
		unsigned char *strVal;

		for (i=0; i<totalEntry; i++) {

			idx = totalEntry-i-1;			
			snprintf(tmpBuf, 20, "select%d", idx);

			strVal = websGetVar(wp, tmpBuf, T(""));
			if ( !gstrcmp(strVal, T("ON")) ) {						
		
				if(mib_chain_delete(MIB_TCP_UDP_CONN_LIMIT_TBL, idx) != 1) {
					strcpy(tmpBuf, T("Delete chain record error!"));
					goto setErr_route;
				}
			}
		}		

		goto setEffect_route;
	}
/*delete all entries*/
	str = websGetVar(wp, T("deleteAllconnLimit"), T(""));
	if (str[0])
	{
		mib_chain_clear(MIB_TCP_UDP_CONN_LIMIT_TBL); /* clear chain record */
		goto setEffect_route;
	}
		
// Add
	str = websGetVar(wp, T("addconnlimit"), T(""));
	if (str[0]) {
		int mibtotal,i, intVal;
		MIB_CE_TCP_UDP_CONN_LIMIT_T tmpentry;
		
		memset(&entry, 0, sizeof(MIB_CE_TCP_UDP_CONN_LIMIT_T));
		//protocol
		str = websGetVar(wp, T("protocol"), T(""));			
		entry.protocol = str[0]- '0';
		//ip
		str = websGetVar(wp, T("ip"), T(""));	
		if (str[0]){
			struct in_addr curIpAddr, curSubnet;
			unsigned long v1, v2, v3;
			if ( !inet_aton(str, (struct in_addr *)&entry.ipAddr) ) {
				strcpy(tmpBuf, T(strIPAddresserror));
				goto setErr_route;
			}
			
			mib_get( MIB_ADSL_LAN_IP,  (void *)&curIpAddr);
			mib_get( MIB_ADSL_LAN_SUBNET,  (void *)&curSubnet);

			v1 = *((unsigned long *)entry.ipAddr);
			v2 = *((unsigned long *)&curIpAddr);
			v3 = *((unsigned long *)&curSubnet);

			if ( (v1 & v3) != (v2 & v3) ) {
				strcpy(tmpBuf, T("Invalid IP address! It should be set within the current subnet."));
				goto setErr_route;
			}
				
		}
		else 
				goto setOk_route;
/*		str = websGetVar(wp, T("cnlm_enable"), T(""));

		if ( str && str[0] ) {
			entry.Enabled = 1;
		}
*/
		entry.Enabled = 1;
		str = websGetVar(wp, T("connnum"), T(""));			
		if ( str[0] ) {
			unsigned int vUInt;
			sscanf(str, "%u", &vUInt);
			entry.connNum = vUInt;
		}
		else 
				goto setOk_route;
		mibtotal = mib_chain_total(MIB_TCP_UDP_CONN_LIMIT_TBL);
		for(i=0;i<mibtotal;i++){
			if(!mib_chain_get(MIB_TCP_UDP_CONN_LIMIT_TBL,i,(void*)&tmpentry))
				continue;
			if(((*(int*)(entry.ipAddr))==(*(int*)(tmpentry.ipAddr))) &&
				(entry.protocol == tmpentry.protocol))
			{
				snprintf(tmpBuf, 100, strIpExist, str);
				goto setErr_route;
			}
		}
		
		intVal = mib_chain_add(MIB_TCP_UDP_CONN_LIMIT_TBL, (void *)&entry);
		if (intVal == 0) {
			//websWrite(wp, T("%s"), strAddChainerror);
			//return;
			strcpy(tmpBuf, T(strAddChainerror));
			goto setErr_route;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, T(strTableFull));
			goto setErr_route;
		}
		else 
			goto setEffect_route;
	}
setOk_route:
	/* upgdate to flash */
	//	mib_update(CURRENT_SETTING);
	//connlimitflag= 1;
	
	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
	return;

setEffect_route:
	restart_connlimit();// iptable control goes here
	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	OK_MSG(submitUrl);
	return;

setErr_route:
	ERR_MSG(tmpBuf);	

} 
 #endif //TCP_UDP_CONN_LIMIT


