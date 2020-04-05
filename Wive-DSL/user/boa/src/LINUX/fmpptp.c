/*
 *      Web server handler routines for PPTP stuff
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

#ifdef CONFIG_PPTP_CLIENT
/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"

#define _PATH_PROCNET_DEV      "/proc/net/dev"

typedef struct mib_fetch_t
{
	const char_t *param;
	const char *mib_sid;
	int mib_id;
	int is_switch;
} mib_fetch_t;

const mib_fetch_t pptp_args[] =
{
	{ T("pptp_server"),     "VPN_SERVER",       MIB_VPN_SERVER,         0 },
	{ T("pptp_range"),      "VPN_RANGE",        MIB_VPN_RANGE,          0 },
	{ T("pptp_user"),       "VPN_USER",         MIB_VPN_USER,           0 },
	{ T("pptp_pass"),       "VPN_PASS",         MIB_VPN_PASS,           0 },
	{ T("pptp_mtu"),        "VPN_MTU",          MIB_VPN_MTU,            0 },
	{ T("pptp_type"),       "VPN_TYPE",         MIB_VPN_TYPE,           0 },
	{ T("pptp_mppe"),       "VPN_MPPE",         MIB_VPN_MPPE,           1 },
	{ T("pptp_dgw"),        "VPN_DGW",          MIB_VPN_DGW,            1 },
	{ T("pptp_peerdns"),    "VPN_PEERDNS",      MIB_VPN_PEERDNS,        1 },
	{ T("pptp_mdgw"),       "VPN_MDGW",         MIB_VPN_MDGW,           1 },
	{ T("pptp_debug"),      "VPN_DEBUG",        MIB_VPN_DEBUG,          1 },
	{ T("pptp_nat"),        "VPN_NAT",          MIB_VPN_NAT,            1 },
	{ T("pptp_pppoe_iface"),"VPN_PPPOEIFACE",   MIB_VPN_PPPOEIFACE,     0 },
	{ NULL, 0, 0 }
};

void initPPTPSetupPage(webs_t wp)
{
}

int pptpIfaceList(int eid, webs_t wp, int argc, char_t **argv)
{
	FILE * fd = fopen(_PATH_PROCNET_DEV, "r");
	
	if (fd != NULL)
	{
		char_t iface[32], curr_if[32];
		char_t line[256];
		
		int ret = readMibToBuffer("pptp_pppoe_iface", iface);
		if (ret<0)
			strcpy(iface, "br0");

		// Read all ifaces and check match
		while (fgets(line, 255, fd)!=NULL)
		{
			if (sscanf(line, " %[a-zA-Z0-9]", curr_if)==1)
			{
				// filer only "brXX" && "vcXX" ifaces
				int found = strncmp(curr_if, "br", 2)==0;
				if (!found)
					found = strncmp(curr_if, "vc", 2)==0;
				if (found)
				{
					// Write iface to output if it was found
					websWrite(wp, T("<option value=\"%s\" %s>%s</option>\n"),
						curr_if,
						(strcmp(curr_if, iface)==0) ? "selected=\"selected\"" : "",
						curr_if
					);
				}
			}
		}
		
		fclose(fd);
	}
	else
	{
		fprintf(stderr, "Warning: cannot open %s (%s).\n",
			_PATH_PROCNET_DEV, strerror(errno));
	}
}

void formPPTPSetup(webs_t wp, char_t *path, char_t *query)
{
	char_t  *pptp_enabled, *submitUrl;

	pptp_enabled = websGetVar(wp, T("pptp_enabled"), T(""));
	if (pptp_enabled[0] == '\0')
		pptp_enabled="off";
	
	// Do not set other params if VPN is turned off
	if (strcmp(pptp_enabled, "on")==0)
	{
		char_t *str;
		
		const mib_fetch_t *fetch = pptp_args;
		while (fetch->param != NULL)
		{
			// Get variable
			str = websGetVar(wp, (char *)fetch->param, T(""));
			if (fetch->is_switch) // Check if need update a switch
			{
				if (str[0]=='\0')
					str = "off";
			}
			
			if (!mib_set(fetch->mib_id, (void *)str))
				printf("Set %s mib error!", fetch->param);
			else
				va_cmd("/bin/flash", 3, 1, "set",  fetch->mib_sid, str);
#ifdef WEB_DEBUG_MSG
			printf("%s value : %s\n", fetch->param, str);
#endif
			fetch++;
		}
	}
	
	if (!mib_set(MIB_VPN_ENABLED, (void *)pptp_enabled))
	{
		printf("Set pptp_enabled mib error!");
	}
	else
	{
		va_cmd("/bin/flash", 3, 1, "set",  "VPN_ENABLED", pptp_enabled);
#ifdef WEB_DEBUG_MSG
		printf("pptp_enabled value : %s\n", str);
#endif
		printf("Call vpn helper!\n");
		//va_cmd("/bin/killall", 3, 0, "-9", "vpnhelper.sh");
		va_cmd("/bin/service", 3, 0, "start_vpn", "restart", "&");
	}

	submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
	return;
}
#endif //PPTP_CLIENT
