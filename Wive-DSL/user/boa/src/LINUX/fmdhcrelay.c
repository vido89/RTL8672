
/*
 *      Web server handler routines for DNS stuffs
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

void formDhcrelay(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str, *submitUrl;
	char tmpBuf[100];
	char_t vChar;
	DHCP_TYPE_T curDhcp;
	int dhcprelayflag=0;

	str = websGetVar(wp, T("save"), T(""));
	if (str[0]) {
		struct in_addr dhcps;
		struct in_addr olddhcps;

		if ( !mib_get( MIB_DHCP_MODE, (void *)&vChar) ) {
			strcpy(tmpBuf, T(strGetDhcpModeerror));
			goto setErr_dhcrelay;
		}
		curDhcp = (DHCP_TYPE_T) vChar;

		if(curDhcp == DHCP_LAN_RELAY)
		{
			str = websGetVar(wp, T("dhcps"), T(""));
			if ( str[0] ) {
				if ( !inet_aton(str, &dhcps) ) {
					strcpy(tmpBuf, T("Invalid DHCPS address value!"));
					goto setErr_dhcrelay;
				}
				if(!mib_get(MIB_ADSL_WAN_DHCPS,(void*)&olddhcps)){
					strcpy(tmpBuf, T("Set DHCPS MIB error!"));
					goto setErr_dhcrelay;
				}
				if(olddhcps.s_addr != dhcps.s_addr){
					dhcprelayflag = 1;
					if ( !mib_set(MIB_ADSL_WAN_DHCPS, (void *)&dhcps)) {
			  			strcpy(tmpBuf, T("Set DHCPS MIB error!"));
						goto setErr_dhcrelay;
					}
				}
				goto setOk_dhcrelay;
			}
			else {
				/*
				if ( !mib_get(MIB_ADSL_WAN_DNS1, (void *)&dns1) ) {
					strcpy(tmpBuf, T("Get DNS1 MIB error!"));
					goto setErr_dns;
				}
				*/
				goto setOk_dhcrelay;
			}
		}
	}
	
	
setOk_dhcrelay:
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

//star: for take effect
	if(dhcprelayflag == 1)
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

setErr_dhcrelay:
	ERR_MSG(tmpBuf);
}
