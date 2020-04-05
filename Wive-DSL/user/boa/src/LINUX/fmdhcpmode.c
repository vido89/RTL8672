
/*
 *      Web server handler routines for DHCP Mode stuffs
 *
 */


/*-- System inlcude files --*/
/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"



///////////////////////////////////////////////////////////////////
void formDhcpMode(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str, *submitUrl;
	char tmpBuf[100];
	DHCP_TYPE_T dtmode,olddtmode;
	unsigned char vChar;
#ifndef NO_ACTION
	int pid;
#endif
	int dhcpmodeflag=0;
	
	// Commit
	str = websGetVar(wp, T("save"), T(""));
	if (str[0]) {
		int time;		
				
		str = websGetVar(wp, T("dhcpMode"), T(""));
		
		if (str[0]) {
			if ( str[0] == '0' )
				dtmode = DHCP_LAN_NONE;
			else if ( str[0] == '1' )
				dtmode = DHCP_LAN_RELAY;
			else if ( str[0] == '2' )
				dtmode = DHCP_LAN_SERVER;
			else {
				strcpy(tmpBuf, T("Invalid dhcp mode value!"));
				goto setErr_reboot;
			}
			if ( mib_get(MIB_DHCP_MODE, (void *)&vChar) == 0) {
				strcpy(tmpBuf, T("Set dhcp mode MIB error!"));
				goto setErr_reboot;
			}
			olddtmode = vChar;
			if(olddtmode != dtmode)
			{
				dhcpmodeflag = 1;
				vChar = (unsigned char) dtmode;
				if ( mib_set(MIB_DHCP_MODE, (void *)&vChar) == 0) {
					strcpy(tmpBuf, T("Set dhcp mode MIB error!"));
					goto setErr_reboot;
				}
			}
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
//star: for take effect
	if(dhcpmodeflag == 1)
	{	
		
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

		restart_dhcp();
		submitUrl = websGetVar(wp, T("submit-url"), T(""));
		OK_MSG(submitUrl);
	}else{
		submitUrl = websGetVar(wp, T("submit-url"), T(""));
		if (submitUrl[0])
			websRedirect(wp, submitUrl);
		else
			websDone(wp, 200);
	}
	return;
  	return;

setErr_reboot:
	ERR_MSG(tmpBuf);
}
