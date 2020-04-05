/*
 *      Web server handler routines for auto-provisioning stuffs
 *
 */

#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#include "debug.h"
#include <net/if.h>
#include <linux/if_bridge.h>
#include <stdio.h>

/////////////////////////////////////////////////////////////////////////////
#ifdef AUTO_PROVISIONING
///////////////////////////////////////////////////////////////////
void formAutoProvision(webs_t wp, char_t *path, char_t *query)
{
	char_t *strIp, *submitUrl;
	struct in_addr inIp;
	char tmpBuf[100];
#ifndef NO_ACTION
	int pid;
#endif

	// Set http server IP
	strIp = websGetVar(wp, T("ip"), T(""));
	if ( strIp[0] ) {
		if ( !inet_aton(strIp, &inIp) ) {
			strcpy(tmpBuf, T("Invalid IP-address value!"));
			goto setErr_auto;
		}
		if ( !mib_set(MIB_HTTP_SERVER_IP, (void *)&inIp)) {
			strcpy(tmpBuf, T("Set IP-address error!"));
			goto setErr_auto;
		}
	}

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

setErr_auto:
	ERR_MSG(tmpBuf);
}
#endif

