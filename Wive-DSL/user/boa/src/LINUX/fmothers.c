/*
 *      Web server handler routines for other advanced stuffs
 *
 */


/*-- System inlcude files --*/
#include <net/if.h>
#include <signal.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"

///////////////////////////////////////////////////////////////////
void formOthers(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str, *submitUrl;
	char tmpBuf[100];
#ifndef NO_ACTION
	int pid;
#endif
	unsigned char ippt_itf=255, old_ippt_itf, ippt_lanacc=0, old_ippt_lanacc, flag;
	unsigned int ippt_lease=0, old_ippt_lease;
	struct ippt_para para;
	
	// Set PVC auto-search setting
	str = websGetVar(wp, T("autosearch"), T(""));
	if (str[0]) {
		if (str[0] == '0')
			flag = 0;
		else
			flag = 1;
		if ( !mib_set(MIB_ATM_VC_AUTOSEARCH, (void *)&flag)) {
			strcpy(tmpBuf, T("Set VC auto-search error!"));
			goto setErr_others;
		}
	}
	
#ifdef IP_PASSTHROUGH
	// Get old index of IPPT interface
	mib_get(MIB_IPPT_ITF, (void *)&old_ippt_itf);
	para.old_ippt_itf= old_ippt_itf;		
	
	// Set IP PassThrough setting
	str = websGetVar(wp, T("ippt"), T(""));
	if (str[0]) {
		ippt_itf = (unsigned char)atoi(str);
		if ( !mib_set(MIB_IPPT_ITF, (void *)&ippt_itf)) {
			strcpy(tmpBuf, T("Set IP passthrough interface index error!"));
			goto setErr_others;
		}
	}
	
	// Get old IPPT Lease Time
	mib_get(MIB_IPPT_LEASE, (void *)&old_ippt_lease);
	para.old_ippt_lease= old_ippt_lease;	
	
	str = websGetVar(wp, T("ltime"), T(""));
	if (str[0]) {
		ippt_lease = (unsigned int)atoi(str);
		if ( !mib_set(MIB_IPPT_LEASE, (void *)&ippt_lease)) {
			strcpy(tmpBuf, T("Set IP passthrough lease time error!"));
			goto setErr_others;
		}
	}
	
	// Get old IPPT LAN access flag
	mib_get(MIB_IPPT_LANACC, (void *)&old_ippt_lanacc);
	para.old_ippt_lanacc= old_ippt_lanacc;	
	
	str = websGetVar(wp, T("lan_acc"), T(""));
	if ( !gstrcmp(str, T("ON")))
		ippt_lanacc = 1;
	else
		ippt_lanacc = 0;
	
	if ( !mib_set(MIB_IPPT_LANACC, (void *)&ippt_lanacc)) {
		strcpy(tmpBuf, T("Set IP passthrough LAN access error!"));
		goto setErr_others;
	}
#endif
	
	str = websGetVar(wp, T("singlePC"), T(""));
	if ( !gstrcmp(str, T("ON")))
		flag = 1;
	else
		flag = 0;
	
	if ( !mib_set(MIB_SPC_ENABLE, (void *)&flag)) {
		strcpy(tmpBuf, T("Set Single PC flag error!"));
		goto setErr_others;
	}
	
	str = websGetVar(wp, T("IPtype"), T(""));
	if (str[0]) {
		if (str[0] == '0')
			flag = 0;
		else
			flag = 1;
		if ( !mib_set(MIB_SPC_IPTYPE, (void *)&flag)) {
			strcpy(tmpBuf, T("Set Single PC IP type error!"));
			goto setErr_others;
		}
	}

	// Mason Yu
#if defined(APPLY_CHANGE)
	para.new_ippt_itf= ippt_itf;
	para.new_ippt_lease= ippt_lease;
	para.new_ippt_lanacc= ippt_lanacc;
	
	restartIPPT(para);
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
	OK_MSG(submitUrl);
  	return;

setErr_others:
	ERR_MSG(tmpBuf);
}

