/*
 *      Web server handler routines for interface-vlan mapping.
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

#include "options.h"

#ifdef CONFIG_EXT_SWITCH
#ifdef VLAN_GROUP
void formVlanGroup(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str, *submitUrl;
	char tmpBuf[100];
	char vc_str[]="vc0";
	int group_num;
	unsigned char mode;
	int vlan_vid, vlan_dhcps;
	
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
	// Kaohj
	str = websGetVar(wp, T("vid"), T(""));
	if(str[0])
		vlan_vid = strtol(str, (char**)NULL, 10);
	else
		vlan_vid = 0;
	str = websGetVar(wp, T("dhcps"), T(""));
	if(str[0])
		vlan_dhcps=1;
	else
		vlan_dhcps = 0;
	str = websGetVar(wp, T("select"), T(""));
	if (str[0]) {
		group_num = str[1]-'0';
		str = websGetVar(wp, T("itfsGroup"), T(""));
		setgroup(str, group_num, vlan_vid, vlan_dhcps);
		
		#if 0
		str = websGetVar(wp, T("itfsAvail"), T(""));
		if (str[0])
		{
			setgroup(str, 0, vlan_vid);
			
		}
		#endif
	}
#if defined(APPLY_CHANGE)
	vg_setup_iptables();
	if(mode&0x01)
		setupEth2pvc_vlan();
	else {				
		setupEth2pvc_vlan_disable();		
	}
#endif
	
	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	OK_MSG(submitUrl);
  	return;
}

int ifGroupList(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;
	int i, ifnum, num;
	struct itfInfo itfs[16];
	char grpitf[512], grpval[128], availitf[512], availval[128];
	char *ptr;
	MIB_CE_VLAN_T vlan_entry;
	nBytesSent += websWrite(wp, T("<tr><font size=2>"
	"<td align=center bgcolor=\"#808080\">Select</td>\n"
	"<td align=center bgcolor=\"#808080\"><font size=2>Interfaces</td></font></tr>\n"));
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
	nBytesSent += websWrite(wp, T("<tr><font size=2>"
	"<td align=center bgcolor=\"#C0C0C0\">Default</td>\n"
	"<td align=center bgcolor=\"#C0C0C0\"><font size=2>%s</td></tr>\n"), availitf);
	
	// Show the specified groups
	for (num=1; num<=4; num++) {
		ifnum = get_group_ifinfo(itfs, 16, num);
		grpitf[0]=grpval[0]=0;
		if (ifnum>=1) {
			snprintf( grpitf, 256, "%s", itfs[0].name);
			snprintf( grpval, 64, "%d", IF_ID(itfs[0].ifdomain, itfs[0].ifid));
			ifnum--;
			for (i=0; i<ifnum; i++) {
				//jim: avoid overflow....
				snprintf( grpitf, 510, "%s,%s", grpitf, itfs[i+1].name);
				snprintf( grpval, 64, "%s,%d", grpval, IF_ID(itfs[i+1].ifdomain, itfs[i+1].ifid));
			}
		}
		// group vid
		mib_chain_get(MIB_VLAN_TBL, num, (void *)&vlan_entry);
		nBytesSent += websWrite(wp, T("<tr>"
		"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\"><input type=\"radio\" name=\"select\""
		" value=\"s%d\" onClick=\"postit('%s','%s','%s','%s','%d','%d')\"</td>\n"), num, grpitf, grpval, availitf, availval, vlan_entry.tag, vlan_entry.dhcps);
		nBytesSent += websWrite(wp, T(
		"<td align=center bgcolor=\"#C0C0C0\"><font size=2>%s</td></tr>\n"), grpitf);
	}
	
	return nBytesSent;
}
#endif	//of VLAN_GROUP
#endif	// of CONFIG_EXT_SWITCH

