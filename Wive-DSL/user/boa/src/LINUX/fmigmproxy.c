/*
 *      Web server handler routines for IGMP proxy stuffs
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

#ifdef __i386__
#define _LITTLE_ENDIAN_
#endif

/*-- Macro declarations --*/
#ifdef _LITTLE_ENDIAN_
#define ntohdw(v) ( ((v&0xff)<<24) | (((v>>8)&0xff)<<16) | (((v>>16)&0xff)<<8) | ((v>>24)&0xff) )

#else
#define ntohdw(v) (v)
#endif

extern int startIgmproxy();

#ifdef CONFIG_USER_IGMPPROXY
#define RUNFILE "/var/run/igmp_pid"

///////////////////////////////////////////////////////////////////
void formIgmproxy(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str, *submitUrl;
	char tmpBuf[100];
	FILE *fp;
	char * argv[8];
	char ifname[6];
#ifndef NO_ACTION
	int pid;
#endif
	unsigned char proxy, proxy_if;
#ifdef EMBED
	unsigned char if_num;
	int igmp_pid;
#endif

	// Set IGMP proxy
	str = websGetVar(wp, T("proxy"), T(""));
	if (str[0]) {
		if (str[0] == '0')
			proxy = 0;
		else
			proxy = 1;
		if ( !mib_set(MIB_IGMP_PROXY, (void *)&proxy)) {
			strcpy(tmpBuf, T("Set IGMP proxy error!"));
			goto setErr_igmp;
		}
	}
	
	str = websGetVar(wp, T("proxy_if"), T(""));
	if (str[0]) {
		proxy_if = (unsigned char)atoi(str);
		if ( !mib_set(MIB_IGMP_PROXY_ITF, (void *)&proxy_if)) {
			strcpy(tmpBuf, T("Set IGMP proxy interface index error!"));
			goto setErr_igmp;
		}
	}
	
#ifdef EMBED

	startIgmproxy();

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

setErr_igmp:
	ERR_MSG(tmpBuf);
}
#endif	// of CONFIG_USER_IGMPPROXY

// List all the available WAN side ip interface at web page.
// return: number of ip interface listed.
int ifwanList(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;
	int ifnum=0;
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	//char_t  buffer[3];
	char_t *name;
	int type, hasAny = 0;
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	char wanname[40];
#endif	
	if (ejArgs(argc, argv, T("%s"), &name) < 1) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}
	
	if ( !strcmp(name, T("all")) )
		type = 0;
	else if ( !strcmp(name, T("rt")) )
		type = 1;	// route interface
	else if ( !strcmp(name, T("rt-any")) )
		{
			type = 1; hasAny = 1;
		}
	else if ( !strcmp(name, T("br")) )
		type = 2;	// bridge interface
	else if ( !strcmp(name, T("p2p")) )
		type = 3;	// point-to-point interface
	else
		type = 1;	// default to route
	
	if (hasAny) {
		nBytesSent += websWrite(wp, T("<option value=255>any</option>\n")
);
	}
	
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			websError(wp, 400, T("Get chain record error!\n"));
			return -1;
		}
		
		if (Entry.enable == 0)
			continue;
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		memset(wanname, 0, sizeof(wanname));
		getWanName(&Entry, wanname);
 #endif
		if (type == 2) {
			if (Entry.cmode == ADSL_BR1483)
			{
	#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
				nBytesSent += websWrite(wp, T("<option value=%u>%s</option>\n"),
					Entry.ifIndex, wanname);
	#else
				nBytesSent += websWrite(wp, T("<option value=%u>vc%u</option>\n"),
					Entry.ifIndex, VC_INDEX(Entry.ifIndex));
	#endif
				ifnum++;
			}
		}
		else { // rt or all (1 or 0)
			if ((type == 1 || type == 3) && Entry.cmode == ADSL_BR1483)
				continue;
			
			// check for p-2-p link
//			if (type == 3 && Entry.cmode == ADSL_MER1483)
			if (type == 3 && (Entry.cmode == ADSL_MER1483 || Entry.ipunnumbered==1))	// Jenny
				continue;
			
			if (PPP_INDEX(Entry.ifIndex) != 0x0f)
			{	// PPP interface
	#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
				nBytesSent += websWrite(wp, T("<option value=%u>%s</option>\n"),
					Entry.ifIndex, wanname);
	#else			
				nBytesSent += websWrite(wp, T("<option value=%u>ppp%u</option>\n"),
					Entry.ifIndex, PPP_INDEX(Entry.ifIndex));
	#endif
			}
			else
			{	// vc interface
	#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
				nBytesSent += websWrite(wp, T("<option value=%u>%s</option>\n"),
					Entry.ifIndex, wanname);
	#else			
				nBytesSent += websWrite(wp, T("<option value=%u>vc%u</option>\n"),
					Entry.ifIndex, VC_INDEX(Entry.ifIndex));
	#endif	
			}
			ifnum++;
		}
	}
	
	//snprintf(buffer, 3, "%u", ifnum);
	//ejSetResult(eid, buffer);
	return nBytesSent;
}

