/*
 *      Web server handler routines for Ethernet-to-PVC mapping stuffs
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

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#include "vendor.h"

#include "options.h"
#ifdef __i386__
#define _LITTLE_ENDIAN_
#endif
#ifdef CONFIG_RTL867X_COMBO_PORTMAPPING
void formItfGroup(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str, *submitUrl;
	char tmpBuf[100];
	char vc_str[]="vc0";
	int group_num;
	unsigned char mode;
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	enum PortMappingPriority priority=lowPrio;   // 3 low priority, 2, medium , 1. high priority
#endif	
	// bitmap for virtual lan port function
	// Port Mapping: bit-0
	// QoS : bit-1
	// IGMP snooping: bit-2
	mib_get(MIB_MPMODE, (void *)&mode);
	str = websGetVar(wp, T("pmap"), T(""));
	if ( str[0] == '1' )
		mode |= 0x01;
	else
		mode &= 0xfe;
	mib_set(MIB_MPMODE, (void *)&mode);
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	str = websGetVar(wp, T("priority"), T(""));
	if(str[0])
	{

		priority=str[0]-'0';
	}else
		priority=lowPrio;
	printf("priority=%d\n", priority);
#endif	
	str = websGetVar(wp, T("select"), T(""));
	if (str[0]) {
		group_num = str[1]-'0';
		str = websGetVar(wp, T("itfsGroup"), T(""));
		if (str[0])
		{
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
			setgroup(str, group_num, priority);
#else
			setgroup(str, group_num);
#endif
		}

		str = websGetVar(wp, T("itfsAvail"), T(""));
		if (str[0])
		{
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
			setgroup(str, 0, lowPrio);
#else
			setgroup(str, 0);
#endif		
			
		}
	}
#if defined(APPLY_CHANGE)
	setupEth2pvc();
#endif
	
	submitUrl = websGetVar(wp, T("submit-url"), T(""));
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	//OK_MSG1("配置修改成功, 如果要设定生效, 请保存配置并重启系统!", submitUrl);
	OK_MSG1(strSetOkToCommitReboot, submitUrl);
#else
	OK_MSG(submitUrl);
#endif
  	return;
}

int ifGroupList(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;
	int i, ifnum, num;
	struct itfInfo itfs[16];
	char grpitf[512], grpval[128], availitf[512], availval[128];
	char *ptr;
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	enum PortMappingPriority priority=lowPrio;   // 3 low priority, 2, medium , 1. high priority
	nBytesSent += websWrite(wp, T("<tr><font size=2>"
	"<td width=10%% align=center bgcolor=\"#808080\"><font size=2>选择组</td>\n"
	"<td width=80%% align=center bgcolor=\"#808080\"><font size=2>接口</td>\n"
	"<td width=10%% align=center bgcolor=\"#808080\"><font size=2>优先级</td></font></tr>\n"));
#else
	nBytesSent += websWrite(wp, T("<tr><font size=2>"
	"<td align=center bgcolor=\"#808080\">Select</td>\n"
	"<td align=center bgcolor=\"#808080\"><font size=2>Interfaces</td></font></tr>\n"));
#endif	
	// Show default group
	ifnum = get_group_ifinfo(itfs, 16, 0);
	availitf[0]=availval[0]=0;
	if (ifnum>=1) {
		strncat( availitf, itfs[0].name, 64);
		snprintf( availval, 64, "%d", IF_ID(itfs[0].ifdomain, itfs[0].ifid));
		ptr = availval+strlen(availval);
		ifnum--;
		for (i=0; i<ifnum; i++) {
			strncat(availitf, ",", 64);
			strncat(availitf, itfs[i+1].name, 64);
			snprintf(ptr, 64, ",%d", IF_ID(itfs[i+1].ifdomain, itfs[i+1].ifid));
			ptr+=strlen(ptr);
		}
	}
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
{
	char PrioString[6];
	memset(PrioString, 0, sizeof(PrioString));
	//we will get the set priority
	//now default "低"
	strcpy(PrioString, "低");
	nBytesSent += websWrite(wp, T("<tr>"
	"<td align=center bgcolor=\"#C0C0C0\"><font size=2>默认</td>\n"
  "<td align=center bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=2>%s</td>\n"
	"<td align=center bgcolor=\"#C0C0C0\"><font size=2>%s</td></tr>\n"), availitf,PrioString);
}
#else
	nBytesSent += websWrite(wp, T("<tr><font size=2>"
	"<td align=center bgcolor=\"#C0C0C0\">Default</td>\n"
	"<td align=center bgcolor=\"#C0C0C0\"><font size=2>%s</td></tr>\n"), availitf);
#endif
	
	// Show the specified groups
	for (num=1; num<=4; num++) {
		ifnum = get_group_ifinfo(itfs, 16, num);
		grpitf[0]=grpval[0]=0;
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		priority=lowPrio;
#endif
		if (ifnum>=1) {
		#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
			for(i=0; i< ifnum; i++)
			{
				int phyPort;
				int entryNum;
				if(itfs[i].ifdomain== DOMAIN_WAN )
				{
					int vcNum,k;
					MIB_CE_ATM_VC_T pvcEntry;
					vcNum = mib_chain_total(MIB_ATM_VC_TBL);
					printf("ifgrouplist: vcnum=%d\n", vcNum);
					for (k=0; k<vcNum; k++) 
					{
						if (!mib_chain_get(MIB_ATM_VC_TBL, k, (void *)&pvcEntry))
						{
  							//websError(wp, 400, T("Get chain record error!\n"));
  							printf("Get chain record error!\n");
							return -1;
						}
						if(itfs[i].ifid==pvcEntry.ifIndex)
						{
							priority=QoS2Prio[pvcEntry.qos];
							break;
						}
					}
					break;
				}
			}

		#endif			
			snprintf( grpitf, 256, "%s", itfs[0].name);
			snprintf( grpval, 64, "%d", IF_ID(itfs[0].ifdomain, itfs[0].ifid));
			ifnum--;
			for (i=0; i<ifnum; i++) {
				//jim: avoid overflow....
				snprintf( grpitf, 510, "%s,%s", grpitf, itfs[i+1].name);
				snprintf( grpval, 64, "%s,%d", grpval, IF_ID(itfs[i+1].ifdomain, itfs[i+1].ifid));
			}
		}
	#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	{
		char PrioString[6];
		memset(PrioString, 0, sizeof(PrioString));
		//we will get the set priority
		//now default "低"
		switch (priority)
		{


			case HighestPrio:
				strcpy(PrioString, "最高");
				break;	
			case HighPrio:
				strcpy(PrioString, "高");
				break;	
			case MediumPrio:
				strcpy(PrioString, "中");
				break;				
			case lowPrio:
			default:
				strcpy(PrioString, "低");
				break;				

		}
		nBytesSent += websWrite(wp, T("<tr>"
		"<td align=center bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=2>组%d<input type=\"radio\" name=\"select\""
		" value=\"s%d\" onClick=\"checkpriority('%s'); postit('%s','%s','%s','%s')\"</td>\n"), num, num, PrioString, grpitf, grpval, availitf, availval);
		nBytesSent += websWrite(wp, T(
		"<td align=center bgcolor=\"#C0C0C0\"  style=\"word-break:break-all\"><font size=2>%s</td>\n"), grpitf);	
		nBytesSent += websWrite(wp, T(
		"<td align=center bgcolor=\"#C0C0C0\"><font size=2>%s</td></tr>\n"), PrioString);	
	}
	#else
		nBytesSent += websWrite(wp, T("<tr>"
		"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\"><input type=\"radio\" name=\"select\""
		" value=\"s%d\" onClick=\"postit('%s','%s','%s','%s')\"</td>\n"), num, grpitf, grpval, availitf, availval);
		nBytesSent += websWrite(wp, T(
		"<td align=center bgcolor=\"#C0C0C0\"><font size=2>%s</td></tr>\n"), grpitf);
	#endif
	}

	return nBytesSent;
}

#endif
#ifdef CONFIG_EXT_SWITCH
/*-- Macro declarations --*/
#if 0
///////////////////////////////////////////////////////////////////
void formEth2pvc(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str, *submitUrl;
	char tmpBuf[100];
	char vc_str[]="vc0";
	int k;
	unsigned char mapen, map_if, mode;
	MIB_CE_SW_PORT_Tp pEntry;
	
#ifndef NO_ACTION
	int pid;
#endif
#ifdef EMBED
	unsigned char if_num;
	int igmp_pid;
#endif
	
	str = websGetVar(wp, T("admin"), T(""));
	
	// enable/disable PortMapping
	if (str[0]) {
		// bitmap for virtual lan port function
		// Port Mapping: bit-0
		// QoS : bit-1
		// IGMP snooping: bit-2
		mib_get(MIB_MPMODE, (void *)&mode);
		str = websGetVar(wp, T("pmap"), T(""));
		if ( str[0] == '1' )
			mode |= 0x01;
		else
			mode &= 0xfe;
		mib_set(MIB_MPMODE, (void *)&mode);
	}
	
	str = websGetVar(wp, T("save"), T(""));
	
	if (str[0]) {
		for (k=0; k<SW_PORT_NUM; k++) {
			vc_str[2] = '0' + k;
			str = websGetVar(wp, T(vc_str), T(""));
			if (str[0]) {
				map_if = (unsigned char)atoi(str);
				pEntry = (MIB_CE_SW_PORT_Tp) mib_chain_get(MIB_SW_PORT_TBL, k);
				pEntry->pvcItf = map_if;
				// log message
				mib_chain_update(MIB_SW_PORT_TBL, (char *)pEntry, k);
			}
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

setErr_map:
	ERR_MSG(tmpBuf);
}

// Post the mapped pvc at web page.
// 
int eth2pvcPost(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;
	unsigned int entryNum, i;
	MIB_CE_SW_PORT_Tp pEntry;
	
	entryNum = mib_chain_total(MIB_SW_PORT_TBL);
	
	for (i=0; i<entryNum; i++) {
		pEntry = (MIB_CE_SW_PORT_Tp) mib_chain_get(MIB_SW_PORT_TBL, i); /* get the specified chain record */

		if(pEntry == NULL)
		{
  			websError(wp, 400, T("Get chain record error!\n"));
			return -1;
		}
		
		nBytesSent += websWrite(wp, T("postit(%d,%d);\n"), i, pEntry->pvcItf);
	}
	
	return nBytesSent;
}
#endif

#ifdef ITF_GROUP
void formItfGroup(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str, *submitUrl;
	char tmpBuf[100];
	char vc_str[]="vc0";
	int group_num;
	unsigned char mode;
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	enum PortMappingPriority priority=lowPrio;   // 3 low priority, 2, medium , 1. high priority
#endif	
	// bitmap for virtual lan port function
	// Port Mapping: bit-0
	// QoS : bit-1
	// IGMP snooping: bit-2
	mib_get(MIB_MPMODE, (void *)&mode);
	str = websGetVar(wp, T("pmap"), T(""));
	if ( str[0] == '1' )
		mode |= 0x01;
	else
		mode &= 0xfe;
	mib_set(MIB_MPMODE, (void *)&mode);
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	str = websGetVar(wp, T("priority"), T(""));
	if(str[0])
	{
		
		priority=str[0]-'0';
	}else
		priority=lowPrio;
	printf("priority=%d\n", priority);
#endif	
	str = websGetVar(wp, T("select"), T(""));
	if (str[0]) {
		group_num = str[1]-'0';
		str = websGetVar(wp, T("itfsGroup"), T(""));
		if (str[0])
		{
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
			setgroup(str, group_num, priority);
#else			
			setgroup(str, group_num);
#endif
		}
		
		str = websGetVar(wp, T("itfsAvail"), T(""));
		if (str[0])
		{
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
			setgroup(str, 0, lowPrio);
#else			
			setgroup(str, 0);
#endif		
			
		}
	}
#if defined(APPLY_CHANGE)
	if(mode&0x01)
		setupEth2pvc();
	else {				
		setupEth2pvc_disable();		
	}
#endif
	
	submitUrl = websGetVar(wp, T("submit-url"), T(""));
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	//OK_MSG1("配置修改成功, 如果要设定生效, 请保存配置并重启系统!", submitUrl);
	OK_MSG1(strSetOkToCommitReboot, submitUrl);
#else
	OK_MSG(submitUrl);
#endif
  	return;
}

int ifGroupList(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;
	int i, ifnum, num;
	struct itfInfo itfs[16];
	char grpitf[512], grpval[128], availitf[512], availval[128];
	char *ptr;
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	enum PortMappingPriority priority=lowPrio;   // 3 low priority, 2, medium , 1. high priority
	nBytesSent += websWrite(wp, T("<tr><font size=2>"
	"<td width=10%% align=center bgcolor=\"#808080\"><font size=2>选择组</td>\n"
	"<td width=80%% align=center bgcolor=\"#808080\"><font size=2>接口</td>\n"
	"<td width=10%% align=center bgcolor=\"#808080\"><font size=2>优先级</td></font></tr>\n"));
#else
	nBytesSent += websWrite(wp, T("<tr><font size=2>"
	"<td align=center bgcolor=\"#808080\">Select</td>\n"
	"<td align=center bgcolor=\"#808080\"><font size=2>Interfaces</td></font></tr>\n"));
#endif	
	// Show default group
	ifnum = get_group_ifinfo(itfs, 16, 0);
	availitf[0]=availval[0]=0;
	if (ifnum>=1) {
		strncat( availitf, itfs[0].name, 64);
		snprintf( availval, 64, "%d", IF_ID(itfs[0].ifdomain, itfs[0].ifid));
		ptr = availval+strlen(availval);
		ifnum--;
		for (i=0; i<ifnum; i++) {
			strncat(availitf, ",", 64);
			strncat(availitf, itfs[i+1].name, 64);
			snprintf(ptr, 64, ",%d", IF_ID(itfs[i+1].ifdomain, itfs[i+1].ifid));
			ptr+=strlen(ptr);
		}
	}
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
{
	char PrioString[6];
	memset(PrioString, 0, sizeof(PrioString));
	//we will get the set priority
	//now default "低"
	strcpy(PrioString, "低");
	nBytesSent += websWrite(wp, T("<tr>"
	"<td align=center bgcolor=\"#C0C0C0\"><font size=2>默认</td>\n"
  "<td align=center bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=2>%s</td>\n"
	"<td align=center bgcolor=\"#C0C0C0\"><font size=2>%s</td></tr>\n"), availitf,PrioString);
}
#else
	nBytesSent += websWrite(wp, T("<tr><font size=2>"
	"<td align=center bgcolor=\"#C0C0C0\">Default</td>\n"
	"<td align=center bgcolor=\"#C0C0C0\"><font size=2>%s</td></tr>\n"), availitf);
#endif
	
	// Show the specified groups
	for (num=1; num<=4; num++) {
		ifnum = get_group_ifinfo(itfs, 16, num);
		grpitf[0]=grpval[0]=0;
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		priority=lowPrio;
#endif
		if (ifnum>=1) {
		#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
			for(i=0; i< ifnum; i++)
			{
				int phyPort;
				int entryNum;
				if(itfs[i].ifdomain== DOMAIN_WAN )
				{
					int vcNum,k;
					MIB_CE_ATM_VC_T pvcEntry;
					vcNum = mib_chain_total(MIB_ATM_VC_TBL);
					printf("ifgrouplist: vcnum=%d\n", vcNum);
					for (k=0; k<vcNum; k++) 
					{
						if (!mib_chain_get(MIB_ATM_VC_TBL, k, (void *)&pvcEntry))
						{
  							//websError(wp, 400, T("Get chain record error!\n"));
  							printf("Get chain record error!\n");
							return -1;
						}
						if(itfs[i].ifid==pvcEntry.ifIndex)
						{
							priority=QoS2Prio[pvcEntry.qos];
							break;
						}
					}
					break;
				}
			}
			
		#endif			
			snprintf( grpitf, 256, "%s", itfs[0].name);
			snprintf( grpval, 64, "%d", IF_ID(itfs[0].ifdomain, itfs[0].ifid));
			ifnum--;
			for (i=0; i<ifnum; i++) {
				//jim: avoid overflow....
				snprintf( grpitf, 510, "%s,%s", grpitf, itfs[i+1].name);
				snprintf( grpval, 64, "%s,%d", grpval, IF_ID(itfs[i+1].ifdomain, itfs[i+1].ifid));
			}
		}
	#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	{
		char PrioString[6];
		memset(PrioString, 0, sizeof(PrioString));
		//we will get the set priority
		//now default "低"
		switch (priority)
		{
			

			case HighestPrio:
				strcpy(PrioString, "最高");
				break;	
			case HighPrio:
				strcpy(PrioString, "高");
				break;	
			case MediumPrio:
				strcpy(PrioString, "中");
				break;				
			case lowPrio:
			default:
				strcpy(PrioString, "低");
				break;				
				
		}
		nBytesSent += websWrite(wp, T("<tr>"
		"<td align=center bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=2>组%d<input type=\"radio\" name=\"select\""
		" value=\"s%d\" onClick=\"checkpriority('%s'); postit('%s','%s','%s','%s')\"</td>\n"), num, num, PrioString, grpitf, grpval, availitf, availval);
		nBytesSent += websWrite(wp, T(
		"<td align=center bgcolor=\"#C0C0C0\"  style=\"word-break:break-all\"><font size=2>%s</td>\n"), grpitf);	
		nBytesSent += websWrite(wp, T(
		"<td align=center bgcolor=\"#C0C0C0\"><font size=2>%s</td></tr>\n"), PrioString);	
	}
	#else
		nBytesSent += websWrite(wp, T("<tr>"
		"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\"><input type=\"radio\" name=\"select\""
		" value=\"s%d\" onClick=\"postit('%s','%s','%s','%s')\"</td>\n"), num, grpitf, grpval, availitf, availval);
		nBytesSent += websWrite(wp, T(
		"<td align=center bgcolor=\"#C0C0C0\"><font size=2>%s</td></tr>\n"), grpitf);
	#endif
	}
	
	return nBytesSent;
}

#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
int	eth2pvcStatus(int eid, webs_t wp, int argc, char_t **argv)
{
	unsigned char mode=0;
	int nBytesSent=0;
	mib_get(MIB_MPMODE, (void *)&mode);
	if(mode&0x01) //portmapping enabled 
		nBytesSent+=websWrite(wp, T("<font color=\"#0000FF\">已启用 </font>\n" ));
	else
		nBytesSent+=websWrite(wp, T("<font color=\"#FF0000\">已禁用 </font>\n" ));
	return nBytesSent;
	
}

int showIfGroupList(int eid, webs_t wp, int argc, char_t **argv)
{
{
	int nBytesSent=0;
	int i, ifnum, num;
	struct itfInfo itfs[16];
	char grpitf[512], grpval[128], availitf[512], availval[128];
	char *ptr;
	enum PortMappingPriority priority=lowPrio;   // 3 low priority, 2, medium , 1. high priority
	nBytesSent += websWrite(wp, T("<tr><font size=2>"
	"<td align=center bgcolor=\"#808080\">选择组</td>\n"
	"<td align=center bgcolor=\"#808080\"><font size=2>接口</td>\n"
	"<td align=center bgcolor=\"#808080\"><font size=2>优先级</td></font></tr>\n"));
	// Show default group
	ifnum = get_group_ifinfo(itfs, 16, 0);
	availitf[0]=availval[0]=0;
	if (ifnum>=1) {
		strncat( availitf, itfs[0].name, 64);
		snprintf( availval, 64, "%d", IF_ID(itfs[0].ifdomain, itfs[0].ifid));
		ptr = availval+strlen(availval);
		ifnum--;
		for (i=0; i<ifnum; i++) {
			strncat(availitf, ",", 64);
			strncat(availitf, itfs[i+1].name, 64);
			snprintf(ptr, 64, ",%d", IF_ID(itfs[i+1].ifdomain, itfs[i+1].ifid));
			ptr+=strlen(ptr);
		}
	}
{
	char PrioString[6];
	memset(PrioString, 0, sizeof(PrioString));
	//we will get the set priority
	//now default "低"
	strcpy(PrioString, "低");
	nBytesSent += websWrite(wp, T("<tr><font size=2>"
	"<td align=center bgcolor=\"#C0C0C0\">默认</td>\n"
	"<td align=center bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=2>%s</td>\n"
	"<td align=center bgcolor=\"#C0C0C0\"><font size=2>%s</td></tr>\n"), availitf,PrioString);
}
	
	// Show the specified groups
	for (num=1; num<=4; num++) {
		ifnum = get_group_ifinfo(itfs, 16, num);
		grpitf[0]=grpval[0]=0;
		priority=lowPrio;
		if (ifnum>=1) {
			for(i=0; i< ifnum; i++)
			{
				int phyPort;
				int entryNum;
				if(itfs[i].ifdomain== DOMAIN_WAN )
				{
					int vcNum,k;
					MIB_CE_ATM_VC_T pvcEntry;
					vcNum = mib_chain_total(MIB_ATM_VC_TBL);
	
					for (k=0; k<vcNum; k++) 
					{
						if (!mib_chain_get(MIB_ATM_VC_TBL, k, (void *)&pvcEntry))
						{
  							//websError(wp, 400, T("Get chain record error!\n"));
  							printf("Get chain record error!\n");
							return -1;
						}
						if(itfs[i].ifid==pvcEntry.ifIndex)
						{
							priority=QoS2Prio[pvcEntry.qos];
							break;
						}
					}
					break;
				}
			}		

					
			snprintf( grpitf, 64, "%s", itfs[0].name);
			snprintf( grpval, 64, "%d", IF_ID(itfs[0].ifdomain, itfs[0].ifid));
			ifnum--;
			for (i=0; i<ifnum; i++) {
				snprintf( grpitf, 512, "%s,%s", grpitf, itfs[i+1].name);
				snprintf( grpval, 64, "%s,%d", grpval, IF_ID(itfs[i+1].ifdomain, itfs[i+1].ifid));
			}
		}
	{
		char PrioString[6];
		memset(PrioString, 0, sizeof(PrioString));
		//we will get the set priority
		//now default "低"
		switch (priority)
		{
			

			case HighestPrio:
				strcpy(PrioString, "最高");
				break;	
			case HighPrio:
				strcpy(PrioString, "高");
				break;	
			case MediumPrio:
				strcpy(PrioString, "中");
				break;				
			case lowPrio:
			default:
				strcpy(PrioString, "低");
				break;				
				
		}
		nBytesSent += websWrite(wp, T("<tr>"
		"<td align=center bgcolor=\"#C0C0C0\"  style=\"word-break:break-all\">组%d<font size=\"2\">"), num);
		nBytesSent += websWrite(wp, T(
		"<td align=center bgcolor=\"#C0C0C0\"  style=\"word-break:break-all\"><font size=2>%s</td>\n"), grpitf);	
		nBytesSent += websWrite(wp, T(
		"<td align=center bgcolor=\"#C0C0C0\"><font size=2>%s</td></tr>\n"), PrioString);	
	}
	
	}
	
	return nBytesSent;
}
}
#endif
#endif	// of ITF_GROUP
#endif // CONFIG_EXT_SWITCH

//xl_yue: translocate to fmmenucreate.c
#if !defined(ZTE_531B_BRIDGE_SC) && !defined(ZTE_GENERAL_ROUTER_SC)
int vportMenu(int eid, webs_t wp, int argc, char_t **argv)
{
	char *parm;
	
	if (ejArgs(argc, argv, T("%s"), &parm) < 1) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}
	
	if ( parm[0] == '1' ) {
		websWrite(wp, T("")
#if (defined( CONFIG_EXT_SWITCH) && defined(ITF_GROUP)) || defined(CONFIG_RTL867X_COMBO_PORTMAPPING)
#ifdef BPORTMAPPING_SUPPORT
		T("advance.addItem(\"Port Mapping\", \"eth2pvc.asp\", \"\", \"Assign Ethernet to PVC\");\n")
#endif
#endif
#ifdef IP_QOS
		T("advance.addItem(\"IP QoS\", \"ipqos.asp\", \"\", \"IP QoS Configuration\");\n")
#elif defined(NEW_IP_QOS_SUPPORT)
		T("advance.addItem(\"IP QoS\", \"ipqos_sc.asp\", \"\", \"IP QoS Configuration\");\n")
#endif
#ifdef CONFIG_EXT_SWITCH
#ifdef ELAN_LINK_MODE
		T("advance.addItem(\"Link Mode\", \"lnkmode.asp\", \"\", \"Ethernet Link Mode Setting\");\n")
#endif
#endif
		);
	}
	else {
		websWrite(wp, T("")
#if (defined( CONFIG_EXT_SWITCH) && defined(ITF_GROUP)) || defined(CONFIG_RTL867X_COMBO_PORTMAPPING)
#ifdef BPORTMAPPING_SUPPORT
		T("<tr><td>&nbsp;&nbsp;<a href=\"eth2pvc.asp\" target=\"view\">Port Mapping</a></td></tr>\n")
#endif
#endif
#ifdef IP_QOS
		T("<tr><td>&nbsp;&nbsp;<a href=\"ipqos.asp\" target=\"view\">IP QoS</a></td></tr>\n")
#elif defined(NEW_IP_QOS_SUPPORT)
		T("<tr><td>&nbsp;&nbsp;<a href=\"ipqos_sc.asp\" target=\"view\">IP QoS</a></td></tr>\n")
#endif
#ifdef CONFIG_EXT_SWITCH
#ifdef ELAN_LINK_MODE
		T("<tr><td>&nbsp;&nbsp;<a href=\"lnkmode.asp\" target=\"view\">Link Mode</a></td></tr>\n")
#endif
#endif
		);
	}
	
	return 0;
}
#endif

