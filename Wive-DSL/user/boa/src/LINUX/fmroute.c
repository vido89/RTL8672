/*
 *      Web server handler routines for Routing stuffs
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


///////////////////////////////////////////////////////////////////
void formRoute(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str, *submitUrl;
	char tmpBuf[100];
	//struct rtentry rt;
	struct in_addr *addr;
	MIB_CE_IP_ROUTE_T entry;
	int xflag, isnet;
	int skfd;
	int intVal;
	//char ifname[16];
#ifndef NO_ACTION
	int pid;
#endif
	
	memset( &entry, 0, sizeof(MIB_CE_IP_ROUTE_T));

#ifdef DEFAULT_GATEWAY_V2
	// Jenny, Default Gateway setting
	str = websGetVar(wp, T("dgwSet"), T(""));
	if (str[0]) {
		unsigned char dgw;
		
		str = websGetVar(wp, T("droute"), T(""));
		dgw = (unsigned char)atoi(str);
		if (!mib_set(MIB_ADSL_WAN_DGW_ITF, (void *)&dgw)) {
			strcpy(tmpBuf, T("Set Default Gateway error!"));
			goto setErr_route;
		}
		goto setOk_route;
	}
#endif

	// Delete
	str = websGetVar(wp, T("delRoute"), T(""));
	if (str[0]) {
		unsigned int i;
		unsigned int idx;
		MIB_CE_IP_ROUTE_T Entry;
		unsigned int totalEntry = mib_chain_total(MIB_IP_ROUTE_TBL); /* get chain record size */
		str = websGetVar(wp, T("select"), T(""));
		
		if (str[0]) {
			for (i=0; i<totalEntry; i++) {
				idx = totalEntry-i-1;			
				snprintf(tmpBuf, 4, "s%d", idx);
				
				if ( !gstrcmp(str, T(tmpBuf)) ) {
					//struct sockaddr_in *s_in;
					/* get the specified chain record */
					if (!mib_chain_get(MIB_IP_ROUTE_TBL, idx, (void *)&Entry)) {
						strcpy(tmpBuf, errGetEntry);
						goto setErr_route;
					}
					
					// delete from chain record
					if(mib_chain_delete(MIB_IP_ROUTE_TBL, idx) != 1) {
						strcpy(tmpBuf, T("Delete chain record error!"));
						goto setErr_route;
					}
	
					route_cfg_modify(&Entry, 1);
					
					goto setOk_route;
				}
			} // end of for
		}
		else {
			strcpy(tmpBuf, T("There is no item selected to delete!"));
			goto setErr_route;
		}
		
		goto setOk_route;
	}

	// parse input
	str = websGetVar(wp, T("destNet"), T(""));
	if (!inet_aton(str, (struct in_addr *)&entry.destID)) {
		snprintf(tmpBuf, 100, "Error: can't resolve dest %s", str);
		goto setErr_route;
	}
		
	str = websGetVar(wp, T("subMask"), T(""));
	//netMask = ntohl(inet_addr(str));
	if (str[0]) {
		if (!isValidNetmask(str, 1)) {
			snprintf(tmpBuf, 100, "Error: Invalid subnet mask %s", str);
			goto setErr_route;
		}
		if (!inet_aton(str, (struct in_addr *)&entry.netMask)) {
			snprintf(tmpBuf, 100, "Error: can't resolve mask %s", str);
			goto setErr_route;
		}
	}
	#if 0
	// Jenny, for checking duplicated destination address
	destNet[0] = entry.destID[0] & entry.netMask[0];
	destNet[1] = entry.destID[1] & entry.netMask[1];
	destNet[2] = entry.destID[2] & entry.netMask[2];
	destNet[3] = entry.destID[3] & entry.netMask[3];
	str = inet_ntoa(*((struct in_addr *)destNet));
	destID = ntohl(inet_addr(str));
	#endif

	str = websGetVar(wp, T("metric"), T(""));
	if ( str[0] ) {
		if (!string_to_dec(str, &intVal)) {
			snprintf(tmpBuf, 100, "Error: Metric");
			goto setErr_route;	
		}

		if ((intVal < 0) || (intVal > 16)) {
			snprintf(tmpBuf, 100, "Error: Metric must be 0 to 16");
			goto setErr_route;
		}
		entry.FWMetric = intVal;
	}

	entry.ifIndex = 0xff;
	str = websGetVar(wp, T("interface"), T(""));
	if ( str ) {
		if (!string_to_dec(str, &intVal)) {
			snprintf(tmpBuf, 100, "Error: ifname error %s", str);
			goto setErr_route;	
		}
		entry.ifIndex = intVal;	
	}

	str = websGetVar(wp, T("nextHop"), T(""));
	//nextHop = ntohl(inet_addr(str));	// Jenny, for checking duplicated destination address
	if (!str && (entry.ifIndex == 0xff)) {
		snprintf(tmpBuf, 100, "Error: can't resolve next tHop %s", str);
		goto setErr_route;
	} else if (str[0]) {
		if (!inet_aton(str, (struct in_addr *)&entry.nextHop)) {
			snprintf(tmpBuf, 100, "Error: can't resolve next tHop %s", str);
			goto setErr_route;
		}
	}

	str = websGetVar(wp, T("enable"), T(""));
	if ( str && str[0] ) {
		entry.Enable = 1;
	}
		

	// Update
	str = websGetVar(wp, T("updateRoute"), T(""));
	if (str && str[0]) {		
		//char *select, tmpBuf[8]; 
		char *select, strBuf[8]; 
		int i, idx;
		MIB_CE_IP_ROUTE_T tmp;
		unsigned int totalEntry = mib_chain_total(MIB_IP_ROUTE_TBL); /* get chain record size */

		select = websGetVar(wp, T("select"), T(""));
		if (!select ) 
			goto setOk_route;

		for (i=0; i<totalEntry; i++) {
			idx = totalEntry-i-1;			
			snprintf(strBuf, 4, "s%d", idx);			
			//snprintf(tmpBuf, 4, "s%d", idx);			

			//if ( !gstrcmp(select, T(tmpBuf)) ) {																
			if (!gstrcmp(select, T(strBuf))) {
				if (mib_chain_get(MIB_IP_ROUTE_TBL, idx, (void *)&tmp)) {
					route_cfg_modify(&tmp, 1);
					entry.InstanceNum = tmp.InstanceNum; /*keep old instancenum, jiunming*/
				}

#ifdef ZHONE_DEFAULT_CFG
				if (route_cfg_modify(&entry, 0) == -1) {
					route_cfg_modify(&tmp, 0);
					strcpy(tmpBuf, T("Update route error!"));
					goto setErr_route;
				}
#else
				if (!checkRoute(entry, idx)) {	// Jenny
					route_cfg_modify(&tmp, 0);
					strcpy(tmpBuf, T(Tinvalid_rule));
					goto setErr_route;
				}
				route_cfg_modify(&entry, 0);
#endif
				mib_chain_update(MIB_IP_ROUTE_TBL, &entry, idx);
				
				goto setOk_route;
			}
		} // end of for
		
		goto setOk_route;
	}

	// Add
	str = websGetVar(wp, T("addRoute"), T(""));
	if (str && str[0]) {	
#if 0
		int i;
		unsigned int totalEntry;
		MIB_CE_IP_ROUTE_T Entry;
		// Jenny, check if route exists
		totalEntry = mib_chain_total(MIB_IP_ROUTE_TBL);
		for (i=0; i<totalEntry; i++) {
			long pdestID, pnetMask, pnextHop;
			unsigned char pdID[4];
			char *temp;
			if (!mib_chain_get(MIB_IP_ROUTE_TBL, i, (void *)&Entry)) {
				strcpy(tmpBuf, T(strGetChainerror));
				goto setErr_route;
			}

			pdID[0] = Entry.destID[0] & Entry.netMask[0];
			pdID[1] = Entry.destID[1] & Entry.netMask[1];
			pdID[2] = Entry.destID[2] & Entry.netMask[2];
			pdID[3] = Entry.destID[3] & Entry.netMask[3];
			temp = inet_ntoa(*((struct in_addr *)pdID));
			pdestID = ntohl(inet_addr(temp));
			temp = inet_ntoa(*((struct in_addr *)Entry.netMask));
			pnetMask = ntohl(inet_addr(temp));
			temp = inet_ntoa(*((struct in_addr *)Entry.nextHop));
			pnextHop = ntohl(inet_addr(temp));
			if (pdestID == destID && pnetMask == netMask && pnextHop == nextHop) {
				strcpy(tmpBuf, "Error! Routing rule already exists!");
				goto setErr_route;
			}
		}
#endif
#ifndef ZHONE_DEFAULT_CFG
		if (!checkRoute(entry, -1)) {	// Jenny
			strcpy(tmpBuf, T(Tinvalid_rule));
			goto setErr_route;
		}
#endif
		printf("add route\n");
		/* Clean out the RTREQ structure. */
		intVal = mib_chain_add(MIB_IP_ROUTE_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			//websWrite(wp, T("%s"), Tadd_chain_error);
			//return;
			strcpy(tmpBuf, T(Tadd_chain_error));
			goto setErr_route;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, T(strTableFull));
			goto setErr_route;
		}

#ifdef ZHONE_DEFAULT_CFG
		if (route_cfg_modify(&entry, 0) == -1) {
			unsigned int totalEntry = mib_chain_total(MIB_IP_ROUTE_TBL);
			mib_chain_delete(MIB_IP_ROUTE_TBL, totalEntry-1);
			strcpy(tmpBuf, T("Add route error!"));
			goto setErr_route;
		}
#else
		route_cfg_modify(&entry, 0);
#endif
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

	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
  	return;

setErr_route:
	ERR_MSG(tmpBuf);
}

/*void ShowDefaultGateway(int eid, webs_t wp, int argc, char_t **argv)
{
#ifdef DEFAULT_GATEWAY_V2
	websWrite(wp, T("<tr><hr size=1 noshade align=top></tr>\n"));
	websWrite(wp, T("<table border=0 width=600 cellspacing=4 cellpadding=0>\n"
					"	<tr>\n"
					"		<th align=left><font size=2><b>Default Gateway:</b></th>\n"
					"		<td><select name=\"droute\">\n"
					"			<option value=255>None</option>\n"));
	checkWrite(eid, wp, argc, argv);
#else
	websWrite(wp, T("<input type=\"hidden\"  name=\"droute\">\n"));
#endif
}*/

void ShowRouteInf(int eid, webs_t wp, int argc, char_t **argv)
{
#ifdef DEFAULT_GATEWAY_V2
	ifwanList(eid, wp, argc, argv);
	websWrite(wp, T("		</select></td>\n"
					"	</tr>\n"
					"</table>\n"
					"<input type=\"submit\" value=\"Apply Changes\" name=\"dgwSet\"><br><br>"));
#endif
}

void GetDefaultGateway(int eid, webs_t wp, int argc, char_t **argv)
{
#ifdef DEFAULT_GATEWAY_V2
	unsigned char dgw;
	mib_get(MIB_ADSL_WAN_DGW_ITF, (void *)&dgw);
	//websWrite(wp, T("<script>\n"
	//				"	document.route.droute.value = %u;\n"
	//				"</script>"), dgw);
#ifdef AUTO_PPPOE_ROUTE
	if (dgw == DGW_AUTO)
		websWrite(wp, T("	document.forms[0].droute[0].checked = true;\n"));
	else
#endif
		websWrite(wp, T("	document.forms[0].droute[1].checked = true;\n"));
#endif
}
#if defined(CONFIG_USER_ROUTED_ROUTED) && !defined(CONFIG_USER_ZEBRA_OSPFD_OSPFD)
void formRip(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str, *submitUrl, *strVal;
	char tmpBuf[100];
	unsigned char rip_if;
	unsigned int entryNum, i;
	MIB_CE_RIP_T Entry;
#ifndef NO_ACTION
	int pid;
#endif

	// RIP Add
	str = websGetVar(wp, T("ripAdd"), T(""));
	if (str[0]) {
		int intVal;
		str = websGetVar(wp, T("rip_if"), T(""));
		rip_if = (unsigned char)atoi(str);
		
		// Check RIP table
		entryNum = mib_chain_total(MIB_RIP_TBL);
		for (i=0; i<entryNum; i++) {
			mib_chain_get(MIB_RIP_TBL, i, (void *)&Entry);
			if (Entry.ifIndex == rip_if) {
				strcpy(tmpBuf, T("Entry already exists!"));
				goto setErr_rip;		
			}
		}
		
		memset(&Entry, '\0', sizeof(MIB_CE_RIP_T));
		Entry.ifIndex = rip_if;
		str = websGetVar(wp, T("receive_mode"), T(""));
		if ( str[0]=='0' ) {		
			Entry.receiveMode = RIP_NONE;    // None
		} else if ( str[0]=='1') {		
			Entry.receiveMode = RIP_V1;      // RIPV1
		} else if ( str[0]=='2') {		
			Entry.receiveMode = RIP_V2;      // RIPV2
		} else if ( str[0]=='3') {		
			Entry.receiveMode = RIP_V1_V2;   // RIPV1 and RIPV2
		} else {
			strcpy(tmpBuf, T("Set RIP receive mode error!"));
			goto setErr_rip;		
		}
		
		str = websGetVar(wp, T("send_mode"), T(""));
		if ( str[0]=='0' ) {		
			Entry.sendMode = RIP_NONE;    		// None
		} else if ( str[0]=='1') {		
			Entry.sendMode = RIP_V1;      		// RIPV1
		} else if ( str[0]=='2') {		
			Entry.sendMode = RIP_V2;      		// RIPV2
		} else if ( str[0]=='4') {		
			Entry.sendMode = RIP_V1_COMPAT;      	// RIPV1COMPAT
		} else {
			strcpy(tmpBuf, T("Set RIP send mode error!"));
			goto setErr_rip;		
		}
		
		intVal = mib_chain_add(MIB_RIP_TBL, (unsigned char*)&Entry);
		if (intVal == 0) {
			//websWrite(wp, T("%s"), "Error: Add MIB_RIP_TBL chain record.");
			//return;
			strcpy(tmpBuf, T("Error: Add MIB_RIP_TBL chain record."));
			goto setErr_rip;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, T(strTableFull));
			goto setErr_rip;
		}
		goto setRefresh_rip;
	}
	
	// Delete all
	str = websGetVar(wp, T("ripDelAll"), T(""));
	if (str[0]) {
		mib_chain_clear(MIB_RIP_TBL); /* clear chain record */
		goto setRefresh_rip;
	}

	/* Delete selected */
	str = websGetVar(wp, T("ripDel"), T(""));
	if (str[0]) {
		unsigned int i;
		unsigned int idx;
		unsigned int deleted = 0;
		unsigned int totalEntry = mib_chain_total(MIB_RIP_TBL); /* get chain record size */

		for (i=0; i<totalEntry; i++) {

			idx = totalEntry-i-1;			
			snprintf(tmpBuf, 20, "select%d", idx);
			strVal = websGetVar(wp, tmpBuf, T(""));

			if ( !gstrcmp(strVal, T("ON")) ) {						
				deleted ++;
				if(mib_chain_delete(MIB_RIP_TBL, idx) != 1) {
					strcpy(tmpBuf, T("Delete MIB_RIP_TBL chain record error!"));
					goto setErr_rip;
				}
			}
		}		
		if (deleted <= 0) {
			strcpy(tmpBuf, T("There is no item selected to delete!"));
			goto setErr_rip;
		}

		goto setRefresh_rip;
	}
#if 0
	// Delete
	str = websGetVar(wp, T("ripDel"), T(""));
	if (str[0]) {
		unsigned int i;
		unsigned int idx;
		MIB_CE_RIP_T Entry;
		unsigned int totalEntry = mib_chain_total(MIB_RIP_TBL); /* get chain record size */		
		
		str = websGetVar(wp, T("select"), T(""));		
		
		if (str[0]) {
			for (i=0; i<totalEntry; i++) {
				idx = totalEntry-i-1;			
				snprintf(tmpBuf, 4, "s%d", idx);
				
				if ( !gstrcmp(str, T(tmpBuf)) ) {					
					
					// delete from chain record
					if(mib_chain_delete(MIB_RIP_TBL, idx) != 1) {
						strcpy(tmpBuf, T("Delete MIB_RIP_TBL chain record error!"));
						goto setErr_rip;
					}					
				}
			} // end of for
		}		
		goto setRefresh_rip;
	}
#endif
	
	// RIP setting
	str = websGetVar(wp, T("ripSet"), T(""));
	if (str[0]) {
		unsigned char ripVal;
		
		str = websGetVar(wp, T("rip_on"), T(""));
		if (str[0] == '1')
			ripVal = 1;
		else
			ripVal = 0;	// default "off"
		if (!mib_set(MIB_RIP_ENABLE, (void *)&ripVal)) {
			strcpy(tmpBuf, T("Set RIP error!"));
			goto setErr_rip;
		}
		
		// Commented by Mason Yu
		/*
		str = websGetVar(wp, T("rip_ver"), T(""));
		if (str[0] == '0')
			ripVal = 0;
		else
			ripVal = 1;	// default "v2"
		if (!mib_set(MIB_RIP_VERSION, (void *)&ripVal)) {
			strcpy(tmpBuf, T("Set RIP error!"));
			goto setErr_rip;
		}
		*/
	}

setOk_rip:
	startRip();
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

	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	OK_MSG(submitUrl);
	return;

setRefresh_rip:
	startRip();
	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
	return;
	
setErr_rip:
	ERR_MSG(tmpBuf);
}
#else
void formRip(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str, *submitUrl;
	char_t *strVal;
	char tmpBuf[100];
	unsigned char igpEnable;
#ifndef NO_ACTION
	int pid;
#endif

	//check if it is RIP
	strVal = websGetVar(wp, T("igp"), T(""));
	if (strVal[0] == '0') {//RIP
		// RIP Add
		str = websGetVar(wp, T("ripAdd"), T(""));
		if (str[0]) {
			unsigned char rip_if;
			unsigned int i;
			MIB_CE_RIP_T Entry;
			int intVal;
			
			memset(&Entry, '\0', sizeof(MIB_CE_RIP_T));
			
			str = websGetVar(wp, T("rip_if"), T(""));
			rip_if = (unsigned char)atoi(str);
			Entry.ifIndex = rip_if;
			
			str = websGetVar(wp, T("receive_mode"), T(""));
			if ( str[0]=='0' ) {		
				Entry.receiveMode = RIP_NONE;    // None
			} else if ( str[0]=='1') {		
				Entry.receiveMode = RIP_V1;      // RIPV1
			} else if ( str[0]=='2') {		
				Entry.receiveMode = RIP_V2;      // RIPV2
			} else if ( str[0]=='3') {		
				Entry.receiveMode = RIP_V1_V2;   // RIPV1 and RIPV2
			} else {
				strcpy(tmpBuf, T("Set RIP receive mode error!"));
				goto setErr_rip;		
			}
			
			str = websGetVar(wp, T("send_mode"), T(""));
			if ( str[0]=='0' ) {		
				Entry.sendMode = RIP_NONE;    		// None
			} else if ( str[0]=='1') {		
				Entry.sendMode = RIP_V1;      		// RIPV1
			} else if ( str[0]=='2') {		
				Entry.sendMode = RIP_V2;      		// RIPV2
			} else if ( str[0]=='4') {		
				Entry.sendMode = RIP_V1_COMPAT;      	// RIPV1COMPAT
			} else {
				strcpy(tmpBuf, T("Set RIP send mode error!"));
				goto setErr_rip;		
			}
			
			intVal = mib_chain_add(MIB_RIP_TBL, (unsigned char*)&Entry);
			if (intVal == 0) {
				//websWrite(wp, T("%s"), "Error: Add MIB_RIP_TBL chain record.");
				//return;
				strcpy(tmpBuf, T("Error: Add MIB_RIP_TBL chain record."));
				goto setErr_rip;
			}
			else if (intVal == -1) {
				strcpy(tmpBuf, T(strTableFull));
				goto setErr_rip;
			}
			goto setRefresh_rip;
		}
		
		// Delete all
		str = websGetVar(wp, T("ripDelAll"), T(""));
		if (str[0]) {
			mib_chain_clear(MIB_RIP_TBL); /* clear chain record */
			goto setRefresh_rip;
		}

		/* Delete selected */
		str = websGetVar(wp, T("ripDel"), T(""));
		if (str[0]) {
			unsigned int i;
			unsigned int idx;
			unsigned int deleted = 0;
			unsigned int totalEntry = mib_chain_total(MIB_RIP_TBL); /* get chain record size */

			for (i=0; i<totalEntry; i++) {

				idx = totalEntry-i-1;			
				snprintf(tmpBuf, 20, "select%d", idx);
				strVal = websGetVar(wp, tmpBuf, T(""));

				if ( !gstrcmp(strVal, T("ON")) ) {						
					deleted ++;
					if(mib_chain_delete(MIB_RIP_TBL, idx) != 1) {
						strcpy(tmpBuf, T("Delete MIB_RIP_TBL chain record error!"));
						goto setErr_rip;
					}
				}
			}		
			if (deleted <= 0) {
				strcpy(tmpBuf, T("There is no item selected to delete!"));
				goto setErr_rip;
			}

			goto setRefresh_rip;
		}
#if 0
		// Delete
		str = websGetVar(wp, T("ripDel"), T(""));
		if (str[0]) {
			unsigned int i;
			unsigned int idx;
			MIB_CE_RIP_T Entry;
			unsigned int totalEntry = mib_chain_total(MIB_RIP_TBL); /* get chain record size */		
			
			str = websGetVar(wp, T("select"), T(""));		
			
			if (str[0]) {
				for (i=0; i<totalEntry; i++) {
					idx = totalEntry-i-1;			
					snprintf(tmpBuf, 4, "s%d", idx);
					
					if ( !gstrcmp(str, T(tmpBuf)) ) {					
						
						// delete from chain record
						if(mib_chain_delete(MIB_RIP_TBL, idx) != 1) {
							strcpy(tmpBuf, T("Delete MIB_RIP_TBL chain record error!"));
							goto setErr_rip;
						}					
					}
				} // end of for
			}		
			goto setRefresh_rip;
		}
#endif
		
		// RIP setting
		str = websGetVar(wp, T("ripSet"), T(""));
		if (str[0]) {
			unsigned char ripVal;
			
			str = websGetVar(wp, T("rip_on"), T(""));
			if (str[0] == '1')
				ripVal = 1;
			else
				ripVal = 0;	// default "off"
			if (!mib_set(MIB_RIP_ENABLE, (void *)&ripVal)) {
				strcpy(tmpBuf, T("Set RIP error!"));
				goto setErr_rip;
			}
			
			// Commented by Mason Yu
			/*
			str = websGetVar(wp, T("rip_ver"), T(""));
			if (str[0] == '0')
				ripVal = 0;
			else
				ripVal = 1;	// default "v2"
			if (!mib_set(MIB_RIP_VERSION, (void *)&ripVal)) {
				strcpy(tmpBuf, T("Set RIP error!"));
				goto setErr_rip;
			}
			*/
		}

		mib_get(MIB_RIP_ENABLE, (void *)&igpEnable);
		if (igpEnable == 1) {//if rip enabled, close ospf; else dont change any state.
			igpEnable = 0;
			mib_set(MIB_OSPF_ENABLE, (void *)&igpEnable);
		}
	}
	else if (strVal[0] == '1') {
		//ospf add
		str = websGetVar(wp, T("ripAdd"), T(""));
		if (str[0]) {
			MIB_CE_OSPF_T Entry;
			int intVal;
			
			str = websGetVar(wp, T("ip"), T(""));
			if (str[0]) {
				if ( !inet_aton(str, (struct in_addr *)&Entry.ipAddr) ) {
					strcpy(tmpBuf, T(Tinvalid_if_ip));
					goto setErr_rip;
				}
			}
			str = websGetVar(wp, T("mask"), T(""));
			if (str[0]) {
				if (!isValidNetmask(str, 1)) {
					strcpy(tmpBuf, T(Tinvalid_if_mask));
					goto setErr_rip;
				}
				if ( !inet_aton(str, (struct in_addr *)&Entry.netMask) ) {
					strcpy(tmpBuf, T(Tinvalid_if_mask));
					goto setErr_rip;
				}
			}
			intVal = mib_chain_add(MIB_OSPF_TBL, (unsigned char*)&Entry);
			if (intVal == 0) {
				//websWrite(wp, T("%s"), "Error: Add MIB_OSPF_TBL chain record.");
				//return;
				strcpy(tmpBuf, T("Error: Add MIB_OSPF_TBL chain record."));
				goto setErr_rip;
			}
			else if (intVal == -1) {
				strcpy(tmpBuf, T(strTableFull));
				goto setErr_rip;
			}
			goto setRefresh_rip;
		}

		// Delete
		str = websGetVar(wp, T("ripDel"), T(""));
		if (str[0]) {
			unsigned int i;
			unsigned int idx;
			MIB_CE_OSPF_T Entry;
			unsigned int totalEntry = mib_chain_total(MIB_OSPF_TBL); /* get chain record size */		
			
			str = websGetVar(wp, T("select"), T(""));		
			
			if (str[0]) {
				for (i=0; i<totalEntry; i++) {
					idx = totalEntry-i-1;			
					snprintf(tmpBuf, 4, "s%d", idx);
					
					if ( !gstrcmp(str, T(tmpBuf)) ) {					
						
						// delete from chain record
						if(mib_chain_delete(MIB_OSPF_TBL, idx) != 1) {
							strcpy(tmpBuf, T("Delete MIB_OSPF_TBL chain record error!"));
							goto setErr_rip;
						}					
					}
				} // end of for
			}		
			goto setRefresh_rip;
		}

		// OSPF setting
		str = websGetVar(wp, T("ripSet"), T(""));
		if (str[0]) {
			unsigned char ripVal;
			
			str = websGetVar(wp, T("rip_on"), T(""));
			if (str[0] == '1')
				ripVal = 1;
			else
				ripVal = 0;	// default "off"
			if (!mib_set(MIB_OSPF_ENABLE, (void *)&ripVal)) {
				strcpy(tmpBuf, T("Set OSPF error!"));
				goto setErr_rip;
			}
		}

		mib_get(MIB_OSPF_ENABLE, (void *)&igpEnable);
#ifdef CONFIG_USER_ROUTED_ROUTED
		if (igpEnable == 1) {//if ospf enabled, close rip; else dont change any state.
			igpEnable = 0;
			mib_set(MIB_RIP_ENABLE, (void *)&igpEnable);
		}
#endif
	}

setRefresh_rip:
#ifdef CONFIG_USER_ROUTED_ROUTED
	startRip();
#endif
	startOspf();
	
	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
	return;
	
setErr_rip:
	ERR_MSG(tmpBuf);
}
#endif

#ifdef CONFIG_USER_ROUTED_ROUTED
// List all the rip interface at web page.
// return: number of rip interface listed.
int showRipIf(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;	
	unsigned int entryNum, i;
	MIB_CE_RIP_T Entry;
	char ifname[6], receive_mode[5], send_mode[5];	

	entryNum = mib_chain_total(MIB_RIP_TBL);
	#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
	nBytesSent += websWrite(wp, T("<tr><font size=1>"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\">选择</td>\n"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\">端口名称</td>"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\">可接收RIP模式</td>"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\">发送RIP模式</td></font></tr>\n"));
	#else
	nBytesSent += websWrite(wp, T("<tr><font size=1>"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\">Select</td>\n"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\">Interface</td>"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\">Receive Mode</td>"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\">Send Mode</td></font></tr>\n"));
         #endif

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_RIP_TBL, i, (void *)&Entry))
		{
  			websError(wp, 400, T("Get MIB_RIP_TBL chain record error!\n"));
			return;
		}		
		
		if( Entry.ifIndex == 0xff) {
			strncpy(ifname, "br0", strlen("br0"));
			ifname[strlen("br0")] = '\0';
		} else {	
			if (PPP_INDEX(Entry.ifIndex) != 0x0f)
			{	// PPP interface
				snprintf(ifname, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));
			}
			else
			{	// vc interface
				snprintf(ifname, 6, "vc%u", VC_INDEX(Entry.ifIndex));
			}
		}
		
		if ( Entry.receiveMode == RIP_NONE ) {
			strncpy(receive_mode, "None", strlen("None"));
			receive_mode[strlen("None")] = '\0';
		} else if ( Entry.receiveMode == RIP_V1 ) {
			strncpy(receive_mode, "RIP1", strlen("RIP1"));
			receive_mode[strlen("RIP1")] = '\0';
		} else if ( Entry.receiveMode == RIP_V2 ) {
			strncpy(receive_mode, "RIP2", strlen("RIP2"));
			receive_mode[strlen("RIP2")] = '\0';
		} else if ( Entry.receiveMode == RIP_V1_V2 ) {
			strncpy(receive_mode, "Both", strlen("Both"));
			receive_mode[strlen("Both")] = '\0';
		} else {
			websError(wp, 400, T("Get RIP Receive Mode error!\n"));
			return;
		}	
		
		if ( Entry.sendMode == RIP_NONE ) {
			strncpy(send_mode, "None", strlen("None"));
			send_mode[strlen("None")] = '\0';
		} else if ( Entry.sendMode == RIP_V1 ) {
			strncpy(send_mode, "RIP1", strlen("RIP1"));
			send_mode[strlen("RIP1")] = '\0';
		} else if ( Entry.sendMode == RIP_V2 ) {
			strncpy(send_mode, "RIP2", strlen("RIP2"));
			send_mode[strlen("RIP2")] = '\0';
		} else if ( Entry.sendMode == RIP_V1_COMPAT ) {
			strncpy(send_mode, "RIP1COMPAT", strlen("RIP1COMPAT"));
			send_mode[strlen("RIP1COMPAT")] = '\0';
		} else {
			websError(wp, 400, T("Get RIP Send Mode error!\n"));
			return;
		}
		
		nBytesSent += websWrite(wp, T("<tr>"
//		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"select\""
//		" value=\"s%d\""		
//		"></td>\n"
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td>\n"
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>"
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>"
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>"
		"</tr>\n"),
		i, 		
		ifname, receive_mode, send_mode);
	}
	return 0;
}

int ifRipNum()
{	
	int ifnum=0;
	
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;	
	char_t  buffer[3];
		

	// check LAN
	if (mib_get(MIB_ADSL_LAN_RIP, (void *)buffer) != 0) {
		if (buffer[0] == 1) {			
			ifnum++;
		}
	}
	
	// check WAN
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{  			
			return -1;
		}
		
		if (Entry.enable == 0)
			continue;
		
		if (Entry.cmode != ADSL_BR1483 && Entry.rip)
		{			
			ifnum++;
		}
	}
	
	return ifnum;
}
#endif	// of CONFIG_USER_ROUTED_ROUTED

int showStaticRoute(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;
	
	unsigned int entryNum, i;
	MIB_CE_IP_ROUTE_T Entry;
	unsigned long int d,g,m;
	struct in_addr dest;
	struct in_addr gw;
	struct in_addr mask;
	char sdest[16], sgw[16];
	char interface_name[30];
	MIB_CE_ATM_VC_T vcEntry;
	int j;
	int mibTotal = mib_chain_total(MIB_ATM_VC_TBL);
	
	
	entryNum = mib_chain_total(MIB_IP_ROUTE_TBL);
#ifdef ZTE_GENERAL_ROUTER_SC
nBytesSent += websWrite(wp, T("<tr><font size=1>"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\">选择</td>\n"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\">状态</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">目的地址</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">子网掩码</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">下一跳</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">跃点数</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">端口名称</td>\n"
	"</font></tr>\n"));
#else
	nBytesSent += websWrite(wp, T("<tr><font size=1>"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\">Select</td>\n"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\">State</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">Destination</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">Subnet Mask</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">NextHop</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">Metric</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">IF</td>\n"
	"</font></tr>\n"));
#endif

	for (i=0; i<entryNum; i++) {

		char destNet[16], subMask[16], nextHop[16];

		if (!mib_chain_get(MIB_IP_ROUTE_TBL, i, (void *)&Entry))
		{
  			websError(wp, 400, T("Get chain record error!\n"));
			return;
		}
			
		dest.s_addr = *(unsigned long *)Entry.destID;
		gw.s_addr   = *(unsigned long *)Entry.nextHop;
		mask.s_addr = *(unsigned long *)Entry.netMask;
		// inet_ntoa is not reentrant, we have to
		// copy the static memory before reuse it
		strcpy(sdest, inet_ntoa(dest));
		strcpy(sgw, inet_ntoa(gw));

		if ( Entry.ifIndex == 0xff )
		{
			strcpy( interface_name, "---" );
		}
#if !defined(ZTE_531B_BRIDGE_SC) && !defined(ZTE_GENERAL_ROUTER_SC)
		else if(  (Entry.ifIndex & 0xf0)==0xf0  )
		{
			sprintf( interface_name, "vc%u", VC_INDEX(Entry.ifIndex) );
		}else{
			sprintf( interface_name, "ppp%u", PPP_INDEX(Entry.ifIndex) );
		}
#else
		else {
			for (j=0; j<mibTotal; j++) {
				if (!mib_chain_get(MIB_ATM_VC_TBL, j, (void *)&vcEntry)) {
					printf("get mib_atm_vc_tbl error!\n");
					return -1;
				}

				printf("vc ifIndex: %d	  route ifIndex: %d\n", vcEntry.ifIndex, Entry.ifIndex);
				if (vcEntry.ifIndex == Entry.ifIndex) {
					//get name
					getWanName(&vcEntry, interface_name);
					break;
				}
			}
		}
#endif
		strcpy(destNet, inet_ntoa(*((struct in_addr *)Entry.destID)) );
		strcpy(nextHop, inet_ntoa(*((struct in_addr *)Entry.nextHop)) );
		strcpy(subMask, inet_ntoa(*((struct in_addr *)Entry.netMask)) );
		
	
		nBytesSent += websWrite(wp, T("<tr>"
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"select\""
		" value=\"s%d\" "
		"onClick=\"postGW(%d,  '%s','%s','%s',%d,%d,'select%d' )\""

		"></td>\n"
		"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>"
		"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%d</b></font></td>"
		"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>"
		"</tr>\n"),
		i, 
		Entry.Enable, destNet, subMask, nextHop, Entry.FWMetric, Entry.ifIndex, i, 
		Entry.Enable ? "Enable" : "Disable", sdest, inet_ntoa(mask), sgw, Entry.FWMetric, interface_name);
	}
	
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

int routeList(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;
	char buff[256];
	int flgs, metric;
	unsigned long int d,g,m;
	struct in_addr dest;
	struct in_addr gw;
	struct in_addr mask;
	char sdest[16], sgw[16], iface[30];
	FILE *fp;
	//ql--modify interface name
#ifdef ZTE_GENERAL_ROUTER_SC
	unsigned int index;
	MIB_CE_ATM_VC_T Entry;
	int i;
	int mibTotal = mib_chain_total(MIB_ATM_VC_TBL);
#endif

	if (!(fp=fopen("/proc/net/route", "r"))) {
		fclose(fp);
		printf("Error: cannot open /proc/net/route - continuing...\n");
		websWrite(wp, T("%s"), "Error: cannot open /proc/net/route !!");
		return -1;
	}
#ifdef ZTE_GENERAL_ROUTER_SC
nBytesSent += websWrite(wp, T("<tr><font size=1>"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">目的地址</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">子网掩码</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">下一跳</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">跃点数</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">端口名称</td></font></tr>\n"));
#else
	nBytesSent += websWrite(wp, T("<tr><font size=1>"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">Destination</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">Subnet Mask</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">NextHop</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">Metric</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">Iface</td></font></tr>\n"));
#endif
	fgets(buff, sizeof(buff), fp);
	
	while( fgets(buff, sizeof(buff), fp) != NULL ) {
		if(sscanf(buff, "%s%lx%lx%X%*d%*d%d%lx",
		   iface, &d, &g, &flgs, &metric, &m)!=6) {
			printf("Unsuported kernel route format\n");
			websWrite(wp, T("%s"), "Error: Unsuported kernel route format !!");
			return -1;
		}

#ifdef ZTE_GENERAL_ROUTER_SC
		if (!strncmp(iface, "vc", 2) || !strncmp(iface, "ppp", 3)) {
			if (iface[0] == 'v') {
				sscanf(iface, "vc%d", &index);
				index &= 0x0f;
			} else if (iface[0] == 'p') {
				sscanf(iface, "ppp%d", &index);
				index &= 0x0f;
				index <<= 4;
			}
			printf("%d\n", index);
			for (i=0; i<mibTotal; i++) {
				if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) {
					printf("get mib_atm_vc_tbl error!\n");
					return -1;
				}

				if ((((Entry.ifIndex & 0x0f) == index) && (iface[0] == 'v')) ||
					(((Entry.ifIndex & 0xf0) == index)) && (iface[0] == 'p')) {
					//get name
					getWanName(&Entry, iface);
					break;
				}
			}
		}else if (!strcmp(iface, "br0")) {
			memset(iface, 0, sizeof(iface));
			strcpy(iface, "本地接口");
		}
#endif
		if(flgs & RTF_UP) {
			dest.s_addr = d;
			gw.s_addr   = g;
			mask.s_addr = m;
			// inet_ntoa is not reentrant, we have to
			// copy the static memory before reuse it
			strcpy(sdest, inet_ntoa(dest));
			strcpy(sgw,  (gw.s_addr==0   ? "*" : inet_ntoa(gw)));
		
			nBytesSent += websWrite(wp, T("<tr>"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%d</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td></tr>\n"),
			sdest, inet_ntoa(mask), sgw, metric, iface);
		}
	}
	
	fclose(fp);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
void formRefleshRouteTbl(webs_t wp, char_t *path, char_t *query)
{
	char_t *submitUrl;

	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
}

//ql_xu
#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
int showOspfIf(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;	
	unsigned int entryNum, i, j;
	MIB_CE_OSPF_T Entry;
	char net[20];
	unsigned int uMask;
	unsigned int uIp;

	entryNum = mib_chain_total(MIB_OSPF_TBL);
	nBytesSent = websWrite(wp, T("<tr><font size=1>"
		"<td align=center width=\"5%%\" bgcolor=\"#808080\">选择</td>\n"
		"<td align=center width=\"20%%\" bgcolor=\"#808080\">OSPF广播网络</td></font></tr>\n"));

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_OSPF_TBL, i, (void *)&Entry))
		{
  			websError(wp, 400, T("Get MIB_OSPF_TBL chain record error!\n"));
			return;
		}

		uIp = *(unsigned int *)Entry.ipAddr;
		uMask = *(unsigned int *)Entry.netMask;
		uIp = uIp & uMask;
		sprintf(net, "%s", inet_ntoa(*((struct in_addr *)&uIp)));
		for (j=0; j<32; j++)
			if ((uMask>>j) & 0x01)
				break;
		uMask = 32 - j;
		snprintf(net, 20, "%s/%d", net, uMask);

		nBytesSent += websWrite(wp, T("<tr>\n"
			"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"select\""
			" value=\"s%d\"></td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
			"</tr>\n"),
			i, net);
	}
	return 0;
}
#endif
