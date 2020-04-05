
/*
 *      Web server handler routines for Multiport Mode stuffs
 *
 */


/*-- System inlcude files --*/
/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"

#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../../uClibc/include/linux/autoconf.h"
#endif

#ifdef CONFIG_EXT_SWITCH

///////////////////////////////////////////////////////////////////
void formMpMode(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str, *submitUrl;
	char tmpBuf[100];
	MP_TYPE_T mpmode;
	unsigned char vChar;
#ifndef NO_ACTION
	int pid;
#endif
	
	// Commit
	str = websGetVar(wp, T("save"), T(""));
	if (str[0]) {
		int time;		
				
#if 0
		str = websGetVar(wp, T("mpMode"), T(""));
		
		if (str[0]) {
			if ( str[0] == '0' )
				mpmode = MP_NONE;
			else if ( str[0] == '1' )
				mpmode = MP_PORT_MAP;
			else if ( str[0] == '2' )
				mpmode = MP_VLAN;
			else if ( str[0] == '3' )
				mpmode = MP_IPQOS;
			else if ( str[0] == '4' )
				mpmode = MP_IGMPSNOOP;
			else {
				strcpy(tmpBuf, T("Invalid dhcp mode value!"));
				goto setErr_mp;
			}
			
			vChar = (unsigned char) mpmode;
			if ( mib_set(MIB_MPMODE, (void *)&vChar) == 0) {
				strcpy(tmpBuf, T("Set multiport mode MIB error!"));
				goto setErr_mp;
			}
		}		
#endif
		str = websGetVar(wp, T("pmap"), T(""));
		if ( str[0] == '1' )
			vChar = 0x01;
		else
			vChar = 0;
		
		str = websGetVar(wp, T("qos"), T(""));
		if ( str[0] == '1' )
			vChar |= 0x02;
		
		mib_set(MIB_MPMODE, (void *)&vChar);
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

	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	OK_MSG(submitUrl);
  	return;

setErr_mp:
	ERR_MSG(tmpBuf);
}
#endif // CONFIG_EXT_SWITCH
