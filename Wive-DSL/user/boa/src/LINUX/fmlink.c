/*
 *      Web server handler routines for Ethernet Link Mode stuffs
 *
 */


/*-- System inlcude files --*/
#include <net/if.h>
#include <signal.h>
#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../../uClibc/include/linux/autoconf.h"
#endif
#include "options.h"

#ifdef CONFIG_EXT_SWITCH
#ifdef ELAN_LINK_MODE
/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"

#ifdef __i386__
#define _LITTLE_ENDIAN_
#endif

/*-- Macro declarations --*/

///////////////////////////////////////////////////////////////////
void formLink(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str, *submitUrl;
	char tmpBuf[100];
	char p_str[]="p0";
	int k, mode;
	MIB_CE_SW_PORT_T Entry;
	
#ifndef NO_ACTION
	int pid;
#endif
#ifdef EMBED
	unsigned char if_num;
	int igmp_pid;
#endif

	for (k=0; k<SW_PORT_NUM; k++) {
		p_str[1] = '0' + k;
		str = websGetVar(wp, T(p_str), T(""));
		mode = str[0] - '0';
		if (mode < LINK_10HALF || mode > LINK_AUTO) {
			strcpy(tmpBuf, T("Invalid Link Mode value!"));
			goto setErr_link;
		}
			
		if (!mib_chain_get(MIB_SW_PORT_TBL, k, (void *)&Entry)) {
			strcpy(tmpBuf, errGetEntry);
			goto setErr_link;
		}
		Entry.linkMode = mode;
		// log message
		mib_chain_update(MIB_SW_PORT_TBL, (void *)&Entry, k);
	}
#if defined(APPLY_CHANGE)
	setupLinkMode();	
#endif
	
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

setErr_link:
	ERR_MSG(tmpBuf);
}

#endif	// of ELAN_LINK_MODE

#else  // CONFIG_EXT_SWITCH
#ifdef ELAN_LINK_MODE_INTRENAL_PHY
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
///////////////////////////////////////////////////////////////////
void formLink(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str, *submitUrl;
	char tmpBuf[100];
	char p_str[]="p0";
	unsigned char mode;
	//MIB_CE_SW_PORT_T Entry;
	
#ifndef NO_ACTION
	int pid;
#endif
#ifdef EMBED
	unsigned char if_num;
	int igmp_pid;
#endif

	//for (k=0; k<SW_PORT_NUM; k++) {
		p_str[1] = '0';
		str = websGetVar(wp, T(p_str), T(""));
		mode = str[0] - '0';
		if (mode < LINK_10HALF || mode > LINK_AUTO) {
			strcpy(tmpBuf, T("Invalid Link Mode value!"));
			goto setErr_link;
		}
			
		if (!mib_set(MIB_ETH_MODE, &mode)) {
			strcpy(tmpBuf, errGetEntry);
			goto setErr_link;
		}
		
	//}
	
	/* upgdate to flash */
//	mib_update(CURRENT_SETTING);

	restart_ethernet(1);
	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	OK_MSG(submitUrl);
  	return;

setErr_link:
	ERR_MSG(tmpBuf);
}

#endif
#endif // CONFIG_EXT_SWITCH
