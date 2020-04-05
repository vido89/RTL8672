/*-- System inlcude files --*/
#include <net/if.h>
#include <signal.h>
/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"

#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)

///////////////////////////////////////////////////////////////////
void formUpnp(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str_enb, *str_extif, *submitUrl;
	char tmpBuf[100];
	FILE *fp;
	char * argv[8];
	char ifname[6];
#ifndef NO_ACTION
	int pid;
#endif
	unsigned char is_enabled, ext_if, pre_enabled, pre_ext_if;
#ifdef EMBED
	unsigned char if_num;
	int igmp_pid;
#endif

	str_enb = websGetVar(wp, T("daemon"), T(""));
	str_extif = websGetVar(wp, T("ext_if"), T(""));

	if(str_enb[0])
	{
		if (str_enb[0] == '0')
			is_enabled = 0;
		else
			is_enabled = 1;

		if(str_extif[0])
			ext_if = (unsigned char)atoi(str_extif);
		else
			ext_if = 0xFF;  // No interface selected.

		// Magician: UPnP daemon start or shutdown
		if(mib_get(MIB_UPNP_DAEMON, (void *)&pre_enabled) && mib_get(MIB_UPNP_EXT_ITF, (void *)&pre_ext_if))
		{
			if(is_enabled)  // UPnP is enabled
			{
				if(!mib_set(MIB_UPNP_DAEMON, (void *)&is_enabled))
				{
					strcpy(tmpBuf, T("Set UPNP error!"));
					goto setErr_igmp;
				}

				if(pre_enabled && (ext_if != pre_ext_if))  // UPnP was set enabled previous time, and the bound interface was changed this time.)
				{
					if( PPP_INDEX(pre_ext_if) != 0x0f )
						snprintf(ifname, 6, "ppp%u", PPP_INDEX(pre_ext_if));	 // PPP interface
					else
						snprintf(ifname, 5, "vc%u", VC_INDEX(pre_ext_if));

					va_cmd("/bin/upnpctrl", 3, 1, "down", ifname, "br0");  // Shutdown UPnP first.

					if(!mib_set(MIB_UPNP_EXT_ITF, (void *)&ext_if))
					{
						printf("Set UPNP Binded WAN interface index error(1)\n");
						strcpy(tmpBuf, T("Set UPNP Binded WAN interface index error!"));
						goto setErr_igmp;
					}

					if( PPP_INDEX(ext_if) != 0x0f )
						snprintf(ifname, 6, "ppp%u", PPP_INDEX(ext_if));	 // PPP interface
					else
						snprintf(ifname, 5, "vc%u", VC_INDEX(ext_if));

					va_cmd("/bin/upnpctrl", 3, 1, "up", ifname, "br0");  // restart UPnP daemon
				}
				else  if(!pre_enabled)  // UPnP is set disabled previous time.
				{
					if(!mib_set(MIB_UPNP_EXT_ITF, (void *)&ext_if))
					{
						printf("Set UPNP Binded WAN interface index error(1)\n");
						strcpy(tmpBuf, T("Set UPNP Binded WAN interface index error!"));
						goto setErr_igmp;
					}

					if( PPP_INDEX(ext_if) != 0x0f )
						snprintf(ifname, 6, "ppp%u", PPP_INDEX(ext_if));	 // PPP interface
					else
						snprintf(ifname, 5, "vc%u", VC_INDEX(ext_if));

					va_cmd("/bin/upnpctrl", 3, 1, "up", ifname, "br0");
				}
			}
			else if(pre_enabled)  // UPnP is set from enabled to disabled.
			{
				if( PPP_INDEX(pre_ext_if) != 0x0f )
					snprintf(ifname, 6, "ppp%u", PPP_INDEX(pre_ext_if));	 // PPP interface
				else
					snprintf(ifname, 5, "vc%u", VC_INDEX(pre_ext_if));

				va_cmd("/bin/upnpctrl", 3, 1, "down", ifname, "br0");

				if(!mib_set(MIB_UPNP_DAEMON, (void *)&is_enabled))
				{
					strcpy(tmpBuf, T("Set UPNP error!"));
					goto setErr_igmp;
				}

				if(!mib_set(MIB_UPNP_EXT_ITF, (void *)&ext_if))
				{
					printf("Set UPNP Binded WAN interface index error(1)\n");
					strcpy(tmpBuf, T("Set UPNP Binded WAN interface index error!"));
					goto setErr_igmp;
				}
			}
		}
		else
		{
			strcpy(tmpBuf, T("UPNP error: get mib table failed!"));
			goto setErr_igmp;
		}
	}

#ifndef NO_ACTION
	pid = fork();
	if (pid)
		waitpid(pid, NULL, 0);
	else if (pid == 0)
	{
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
#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
	OK_MSG1("修改成功，如果要生效，请保存配置并重启系统！", submitUrl);
#else
	OK_MSG(submitUrl);
#endif
	return;

setErr_igmp:
	ERR_MSG(tmpBuf);
}
#endif
