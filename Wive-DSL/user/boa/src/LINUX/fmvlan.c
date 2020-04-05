/*
 *      Web server handler routines for VLAN configuration stuffs
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

#ifdef CONFIG_EXT_SWITCH
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
void formVlan(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str, *submitUrl;
	char tmpBuf[100];
	char sw_str[]="pa0";
	char tag_str[]="tag_a";
	char pidx_str[]="pidx0";
	char act_str[]="act0";
	int i, k;
	unsigned char vlan_en;
	MIB_CE_VLAN_Tp pVlan;
	MIB_CE_SW_PORT_Tp pPort;
#ifndef NO_ACTION
	int pid;
#endif
#ifdef EMBED
	unsigned char if_num;
	int igmp_pid;
#endif

	// Set Vlan
	for (i=0; i<VLAN_NUM; i++) {
		sw_str[1] = tag_str[4] = 'a' + i;
		pVlan = (MIB_CE_VLAN_Tp) mib_chain_get(MIB_VLAN_TBL, i);
		pVlan->member = 0;
		
		for (k=0; k<SW_PORT_NUM; k++) {
			sw_str[2] = '0' + k;
			str = websGetVar(wp, T(sw_str), T(""));
			if (str[0] == '1')
				pVlan->member |= (1<<k);
		}
		
		str = websGetVar(wp, T(tag_str), T(""));
		if (str[0])
			pVlan->tag = (unsigned short)atoi(str);
		// log message
		mib_chain_update(MIB_VLAN_TBL, (char *)pVlan, i);
	}
			
	for (i=0; i<SW_PORT_NUM; i++) {
		pidx_str[4] = act_str[3] = '0' + i;
		pPort = (MIB_CE_SW_PORT_Tp) mib_chain_get(MIB_SW_PORT_TBL, i);
		str = websGetVar(wp, T(pidx_str), T(""));
		pPort->pvid = str[0] - '0';
		str = websGetVar(wp, T(act_str), T(""));
		pPort->egressTagAction = str[0] - '0';
		// log message
		mib_chain_update(MIB_SW_PORT_TBL, (char *)pPort, i);
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

setErr_vlan:
	ERR_MSG(tmpBuf);
}

// Post the vlan configuration at web page.
// 
int vlanPost(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;
	unsigned int entryNum, i, k;
	MIB_CE_VLAN_Tp pEntry;
	MIB_CE_SW_PORT_Tp pSwport;
	char strMember[]="document.vlan.pa0.checked=1";
	char strTag[]="document.vlan.tag_a.value=";
	char strPidx[]="document.vlan.pidx0.value=";
	char strAct[]="document.vlan.act0.value=";
	
	entryNum = mib_chain_total(MIB_VLAN_TBL);
	
	for (i=0; i<entryNum; i++) {
		pEntry = (MIB_CE_VLAN_Tp) mib_chain_get(MIB_VLAN_TBL, i); /* get the specified chain record */

		if(pEntry == NULL)
		{
  			websError(wp, 400, T("Get chain record error!\n"));
			return -1;
		}
		
		strMember[15] = 'a' + i; // a, b, c ...
		strTag[18] = 'a' + i; // a, b, c ...
		
		for (k=0; k<SW_PORT_NUM; k++) {
			if ((pEntry->member>>k) & 1) {
				strMember[16]='0'+k; // 0, 1, 2, ...
				nBytesSent += websWrite(wp, T("%s;\n"), strMember);
			}
		}
		
		nBytesSent += websWrite(wp, T("%s%d;\n"), strTag, pEntry->tag);
	}
	
	entryNum = mib_chain_total(MIB_SW_PORT_TBL);
	
	for (i=0; i<entryNum; i++) {
		pSwport = (MIB_CE_SW_PORT_Tp) mib_chain_get(MIB_SW_PORT_TBL, i); /* get the specified chain record */

		if(pSwport == NULL)
		{
  			websError(wp, 400, T("Get chain record error!\n"));
			return -1;
		}
		
		strPidx[18] = '0' + i; // 0, 1, 2 ...
		strAct[17] = '0' + i; // 0, 1, 2 ...
		nBytesSent += websWrite(wp, T("%s%d;\n"), strPidx, pSwport->pvid);
		nBytesSent += websWrite(wp, T("%s%d;\n"), strAct, pSwport->egressTagAction);
	}
	
	return nBytesSent;
}
#endif // CONFIG_EXT_SWITCH
