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

#define	DDNS_ADD	0
#define DDNS_MODIFY	1

#ifdef CONFIG_USER_DDNS
static void getEntry(webs_t wp, MIB_CE_DDNS_Tp pEntry)
{
	char_t	*str;
	
	memset(pEntry, 0, sizeof(MIB_CE_DDNS_T));	
	
	str = websGetVar(wp, T("ddnsProv"), T(""));			
	if ( str[0]=='0' ) {	
		strcpy(pEntry->provider, "dyndns");
	} else if ( str[0]=='1') {
		strcpy(pEntry->provider, "tzo");
	} else
		printf("Updatedd not support this provider!!\n");
	//printf("pEntry->provider = %s\n",pEntry->provider);
	
	str = websGetVar(wp, T("hostname"), T(""));
	if (str[0]) {							
		strcpy(pEntry->hostname, str);		
	}
	//printf("pEntry->hostname = %s\n", pEntry->hostname);
	
	str = websGetVar(wp, T("interface"), T(""));
	if (str[0]) {			
/*		ddns_if = (unsigned char)atoi(str);			
		
		if ( ddns_if == 100 ) {
			strcpy(pEntry->interface, "br0");
		} else {
			if_num = PPP_INDEX(ddns_if);
			if (if_num != 0x0f) {
				snprintf(ifname, 6, "ppp%u", if_num);					
			}else {
				snprintf(ifname, 5, "vc%u", VC_INDEX(ddns_if));					
			}
			strcpy(pEntry->interface, ifname);
		}
*/
		strcpy(pEntry->interface, str);
		//printf("pEntry->interface= %s\n", pEntry->interface);	
	}			
	
	if ( strcmp(pEntry->provider, "dyndns") == 0 ) {
		str = websGetVar(wp, T("username"), T(""));
		if (str[0]) {							
			strcpy(pEntry->username, str);		
		}
		//printf("pEntry->username = %s\n", pEntry->username);
	
		str = websGetVar(wp, T("password"), T(""));
		if (str[0]) {							
			strcpy(pEntry->password, str);		
		}
		//printf("pEntry->password = %s\n", pEntry->password);			
		
	} else if ( strcmp(pEntry->provider, "tzo") == 0 ) {
		str = websGetVar(wp, T("email"), T(""));
		if (str[0]) {							
			//strcpy(pEntry->email, str);
			strcpy(pEntry->username, str);		
		}
		//printf("email = %s\n", pEntry->username);
	
		str = websGetVar(wp, T("key"), T(""));
		if (str[0]) {							
			//strcpy(pEntry->key, str);	
			strcpy(pEntry->password, str);	
		}
		//printf("key = %s\n", pEntry->password);			
		
	} else
		printf("Please choose the correct provider!!!\n");
	
	str = websGetVar(wp, T("enable"), T(""));
	if ( str && str[0] ) {
		pEntry->Enabled = 1;
	}
}

/*
 *	type:
 *		DDNS_ADD(0):	entry to add
 *		DDNS_MODIFY(1):	entry to modify
 *	Return value:
 *	-1	: fail
 *	0	: successful
 *	pMsg	: error message
 */
static int checkEntry(MIB_CE_DDNS_Tp pEntry, int type, char *pMsg)
{
	MIB_CE_DDNS_T tmpEntry;
	int num, i;
	
	num = mib_chain_total(MIB_DDNS_TBL); /* get chain record size */
	// check duplication
	for (i=0; i<num; i++) {
		mib_chain_get(MIB_DDNS_TBL, i, (void *)&tmpEntry);
		if (type == DDNS_MODIFY) { // modify
			if (pEntry->Enabled != tmpEntry.Enabled)
				continue;
		}
		if (gstrcmp(pEntry->provider, tmpEntry.provider))
			continue;
		if (gstrcmp(pEntry->hostname, tmpEntry.hostname))
			continue;
		if (gstrcmp(pEntry->interface, tmpEntry.interface))
			continue;
		if (gstrcmp(pEntry->username, tmpEntry.username))
			continue;
		if (gstrcmp(pEntry->password, tmpEntry.password))
			continue;
		if (pEntry->ServicePort != tmpEntry.ServicePort)
			continue;
		// entry duplication
		strcpy(pMsg, T("Entry already exists!"));
		return -1;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////
void formDDNS(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str, *submitUrl;
	char tmpBuf[100], ifname[6];
	unsigned int totalEntry, selected, i, idx;
	MIB_CE_DDNS_T entry;
//	unsigned char ddns_if, if_num;	
#ifndef NO_ACTION
	int pid;
#endif	
	
	totalEntry = mib_chain_total(MIB_DDNS_TBL); /* get chain record size */
	
	// Remove
	str = websGetVar(wp, T("delacc"), T(""));
	if (str[0]) {
		str = websGetVar(wp, T("select"), T(""));		
		if (str[0]) {
			for (i=0; i<totalEntry; i++) {
				idx = totalEntry-i-1;			
				snprintf(tmpBuf, 4, "s%d", idx);
				//printf("tmpBuf(select) = %s idx=%d\n", tmpBuf, idx);
				
				if ( !gstrcmp(str, T(tmpBuf)) ) {					
					// delete from chain record
					if(mib_chain_delete(MIB_DDNS_TBL, idx) != 1) {
						strcpy(tmpBuf, T("Delete chain record error!"));
						goto setErr_route;
					}

					
				}
			} // end of for
		}		
		goto setOk_route;
	}
	
	// Add
	str = websGetVar(wp, T("addacc"), T(""));		
	if (str[0]) {
		MIB_CE_DDNS_T tmpEntry;
		int intVal;
		
		getEntry(wp, &entry);
		if (checkEntry(&entry, DDNS_ADD, &tmpBuf[0]) == -1)
			goto setErr_route;
		
		intVal = mib_chain_add(MIB_DDNS_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			//websWrite(wp, T("%s"), "Error: Add chain(MIB_DDNS_TBL) record.");
			//return;
			strcpy(tmpBuf, T("Error: Add chain(MIB_DDNS_TBL) record."));
			goto setErr_route;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, T(strTableFull));
			goto setErr_route;
		}

	}

	// Modify
	str = websGetVar(wp, T("modify"), T(""));
	if (str[0]) {
		selected = -1;
		str = websGetVar(wp, T("select"), T(""));		
		if (str[0]) {
			for (i=0; i<totalEntry; i++) {
				idx = totalEntry-i-1;			
				snprintf(tmpBuf, 4, "s%d", idx);
				
				if ( !gstrcmp(str, T(tmpBuf)) ) {
					selected = idx;
					break;
				}
			}
			if (selected >= 0) {
				getEntry(wp, &entry);
				if (checkEntry(&entry, DDNS_MODIFY, &tmpBuf[0]) == -1)
					goto setErr_route;
				
				mib_chain_update(MIB_DDNS_TBL, (void *)&entry, selected);
			}
		}
	}

setOk_route:
	/* upgdate to flash */
//	mib_update(CURRENT_SETTING);
	
	restart_ddns();
	
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

int showDNSTable(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;	
	unsigned int entryNum, i;
	MIB_CE_DDNS_T Entry;
	unsigned long int d,g,m;
	struct in_addr dest;
	struct in_addr gw;
	struct in_addr mask;
	char sdest[16], sgw[16];
	
	entryNum = mib_chain_total(MIB_DDNS_TBL);
	
	nBytesSent += websWrite(wp, T("<tr><font size=1>"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\">Select</td>\n"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\">state</td>"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\">Hostname</td>"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\">Username</td>"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\">Service</td>"
//	"<td align=center width=\"20%%\" bgcolor=\"#808080\">Interface</td>"
	"</font></tr>\n"));

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_DDNS_TBL, i, (void *)&Entry))
		{
  			websError(wp, 400, T("Get chain record error!\n"));
			return;
		}		
	
		nBytesSent += websWrite(wp, T("<tr>"
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"select\""
		" value=\"s%d\" onClick=postEntry(%d,'%s','%s','%s','%s')></td>\n"),
		i, Entry.Enabled, Entry.provider, Entry.hostname, Entry.username, Entry.password);
		nBytesSent += websWrite(wp, T(
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>"
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>"
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>"
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>"
//		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>"
		"</tr>\n"),
		//Entry.hostname, strcmp(Entry.username, "")==0?Entry.email:Entry.username, Entry.provider, Entry.interface);
//		Entry.Enabled ? "Enable" : "Disable", Entry.hostname, Entry.username, Entry.provider, Entry.interface);
		Entry.Enabled ? "Enable" : "Disable", Entry.hostname, Entry.username, Entry.provider);
		
	}	
	return 0;
}
#endif
