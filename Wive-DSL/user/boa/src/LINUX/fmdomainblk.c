
/*
 *      Web server handler routines for Domain Blocking stuffs
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

#ifdef DOMAIN_BLOCKING_SUPPORT
///////////////////////////////////////////////////////////////////
void formDOMAINBLK(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str, *submitUrl, *strVal;
	char tmpBuf[100];		
	unsigned char domainblkcap;
#ifndef NO_ACTION
	int pid;
#endif

	// Set domain blocking Capability
	str = websGetVar(wp, T("apply"), T(""));
	if (str[0]) {
		str = websGetVar(wp, T("domainblkcap"), T(""));
		if (str[0]) {
			if (str[0] == '0')
				domainblkcap = 0;
			else
				domainblkcap = 1;
			if ( !mib_set(MIB_DOMAINBLK_CAPABILITY, (void *)&domainblkcap)) {
				strcpy(tmpBuf, T("Set Domain Blocking Capability error!"));
				goto setErr_route;
			}
		}
 	}
 
	// Delete all Domain
	str = websGetVar(wp, T("delAllDomain"), T(""));
	if (str[0]) {
		mib_chain_clear(MIB_DOMAIN_BLOCKING_TBL); /* clear chain record */
		goto setOk_route;
	}

	/* Delete selected Domain */
	str = websGetVar(wp, T("delDomain"), T(""));
	if (str[0]) {
		unsigned int i;
		unsigned int idx;
		unsigned int totalEntry = mib_chain_total(MIB_DOMAIN_BLOCKING_TBL); /* get chain record size */
		unsigned int deleted = 0;

		for (i=0; i<totalEntry; i++) {

			idx = totalEntry-i-1;			
			snprintf(tmpBuf, 20, "select%d", idx);
			strVal = websGetVar(wp, tmpBuf, T(""));
			
			if ( !gstrcmp(strVal, T("ON")) ) {						
				deleted ++;
				if(mib_chain_delete(MIB_DOMAIN_BLOCKING_TBL, idx) != 1) {
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
	// Delete
	str = websGetVar(wp, T("delDomain"), T(""));
	if (str[0]) {
		unsigned int i;
		unsigned int idx;
		MIB_CE_DOMAIN_BLOCKING_T Entry;
		unsigned int totalEntry = mib_chain_total(MIB_DOMAIN_BLOCKING_TBL); /* get chain record size */		
		
		str = websGetVar(wp, T("select"), T(""));		
		
		if (str[0]) {
			for (i=0; i<totalEntry; i++) {
				idx = totalEntry-i-1;			
				snprintf(tmpBuf, 4, "s%d", idx);
				
				if ( !gstrcmp(str, T(tmpBuf)) ) {					
					// delete from chain record
					if(mib_chain_delete(MIB_DOMAIN_BLOCKING_TBL, idx) != 1) {
						strcpy(tmpBuf, T("Delete Domain Blocking chain record error!"));
						goto setErr_route;
					}
					
				}
			} // end of for
		}
		
		goto setOk_route;
	}
#endif

	// Add
	str = websGetVar(wp, T("addDomain"), T(""));
	if (str[0]) {
		MIB_CE_DOMAIN_BLOCKING_T entry;		
		int i, intVal;
		unsigned int totalEntry = mib_chain_total(MIB_DOMAIN_BLOCKING_TBL); /* get chain record size */  
		
		str = websGetVar(wp, T("blkDomain"), T(""));
		//printf("formDOMAINBLK:(Add) str = %s\n", str);
		// Jenny, check duplicated rule
		for (i = 0; i< totalEntry; i++) {
			if (!mib_chain_get(MIB_DOMAIN_BLOCKING_TBL, i, (void *)&entry)) {
				strcpy(tmpBuf, errGetEntry);
				goto setErr_route;
			}
			if (!strcmp(entry.domain, str)) {
				strcpy(tmpBuf, strMACInList );
				goto setErr_route;
			}
		}

		// add into configuration (chain record)
		strcpy(entry.domain, str);		
		
		intVal = mib_chain_add(MIB_DOMAIN_BLOCKING_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			//websWrite(wp, T("%s"), "Error: Add Domain Blocking chain record.");
			//return;
			strcpy(tmpBuf, T("Error: Add Domain Blocking chain record."));
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

#if defined(APPLY_CHANGE)
	restart_domainBLK();
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

setErr_route:
	ERR_MSG(tmpBuf);
}

int showDOMAINBLKTable(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;
	
	unsigned int entryNum, i;
	MIB_CE_DOMAIN_BLOCKING_T Entry;
	unsigned long int d,g,m;	
	unsigned char sdest[MAX_DOMAIN_LENGTH];
	
	entryNum = mib_chain_total(MIB_DOMAIN_BLOCKING_TBL);
	
	nBytesSent += websWrite(wp, T("<tr><font size=1>"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\">Select</td>\n"
	"<td align=center width=\"35%%\" bgcolor=\"#808080\">Domain</td></font></tr>\n"));


	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_DOMAIN_BLOCKING_TBL, i, (void *)&Entry))
		{
  			websError(wp, 400, T("Get Domain Blocking chain record error!\n"));
			return;
		}
		
		strncpy(sdest, Entry.domain, strlen(Entry.domain));	
		sdest[strlen(Entry.domain)] = '\0';	
	
		nBytesSent += websWrite(wp, T("<tr>"
//		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"select\""
//		" value=\"s%d\"></td>\n"
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td>\n"
		"<td align=center width=\"35%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td></tr>\n"),
		i, sdest);
	}
	
	return 0;	
}
#endif
