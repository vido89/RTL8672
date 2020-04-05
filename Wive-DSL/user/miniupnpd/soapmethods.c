/* $Id: soapmethods.c,v 1.9 2008/10/17 06:53:46 adsmt Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006-2008 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <netdb.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "config.h"
#include "upnpglobalvars.h"
#ifdef CONFIG_USE_SHARED_LIB
#include "upnphttp.h"
#include "upnpsoap.h"
#include "upnpreplyparse.h"
#else
#include "localupnphttp.h"
#include "localupnpsoap.h"
#include "localupnpreplyparse.h"
#endif
#include "upnpredirect.h"
#include "getifaddr.h"
#include "getifstats.h"
#define	T(s)	s

#ifdef TR_064
#include "../boa/src/LINUX/adsl_drv.h"
#include "../boa/src/defs.h"

bool IsTimeZoneNameExist(char *time_zone_name, char *zone)
{
	if( time_zone_name == NULL || zone == NULL )
		return false;

	char del[] = ",()";
	char *tok, *tzn, *zn;
	int i;

	tzn = (char *)malloc(sizeof(char)*strlen(time_zone_name)+1);
	zn = (char *)malloc(sizeof(char)*strlen(zone)+1);

	int len = strlen(time_zone_name);
	for( i = 0; i < len; i++ )
		tzn[i] = tolower(time_zone_name[i]);

	tzn[i] = '\0';

	len = strlen(zone);
	for( i = 0; i < len; i++ )
		zn[i] = tolower(zone[i]);

	zn[i] = '\0';

	strtok(tzn, del);  // Discard first token. (GMT-xx:xx)
	for( tok = strtok(NULL, del); tok != NULL; tok = strtok(NULL, del) )
	{
		len = strlen(tok);
		for( i = 0; i < len; i++ )  // Trim leading spaces.
		{
			if( *tok == ' ' )
				tok++;
			else
				break;
		}

		len = strlen(tok);
		for( i = len - 1; i >= 0; i-- )  // Trim trailing spaces.
    {
      if( tok[i] == ' ' )
        tok[i] = '\0';
      else
        break;
    }

		if(!strcmp(tok, zn))
		{
			free(tzn);
			free(zn);
			return true;
		}
	}

	free(tzn);
	free(zn);

	return false;

}

bool IsValidDateTimeValue(char *datetime)  // MM-dd-yyyy hh:mm:ss
{
	if( !datetime )
		return false;

	char *tok, *dt_str = strdup(datetime);
  char del[] = "/ :";
  int i, mon, day, year, hr, min, sec;
  int chk = 0;

  for( i = 0, tok = strtok(dt_str, del); tok != NULL; i++, tok = strtok(NULL, del) )
  {
  	switch(i)
  	{
  		case 0:
  			mon = atoi(tok);
  			if( strlen(tok) > 2 || mon < 1 || mon > 12 )
  				return false;
  			else
  				chk++;
  			break;
  		case 1:
  			day = atoi(tok);
  			if( strlen(tok) > 2 || day < 1 || day > 31 )
  				return false;
  			else
  				chk++;
  			break;
  		case 2:
  			if( strlen(tok) > 4 )
  				return false;
  			else
  			{
  				year = atoi(tok);
 					chk++;
  			}
  			break;
  		case 3:
  			hr = atoi(tok);
  			if( strlen(tok) > 2 || hr < 0 || hr > 23 )
  				return false;
  			else
  				chk++;
  			break;
  		case 4:
  			min = atoi(tok);
  			if( strlen(tok) > 2 || min < 0 || min > 59 )
  				return false;
  			else
  				chk++;
  			break;
  		case 5:
  			sec = atoi(tok);
  			if( strlen(tok) > 2 || sec < 0 || sec > 59 )
  				return false;
  			else
  				chk++;
  			break;
  		default:
  			return false;
  	}
  }

	if( chk == 6 )
		return true;
	else
		return false;
}

int transfer2PathName( unsigned char ifindex, char *name )
{
	int total,i;
	MIB_CE_ATM_VC_T pEntry;
	int pppc = 0, ipc = 0;

	char strfmt[]="InternetGatewayDevice.WANDevice.1.WANConnectionDevice.%d.%s.%d"; //wt-121v8-3.33, no trailing dot
	char ipstr[]="WANIPConnection";
	char pppstr[]="WANPPPConnection";
	char *pconn=NULL;
	unsigned int instnum = 0;

	if( ifindex==0xff ) return -1;
	if( name==NULL ) return -1;
	name[0]=0;

	total = mib_chain_total(MIB_ATM_VC_TBL);
	for( i=0; i<total; i++ )
	{
		if( !mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&pEntry ) )
			continue;

		if( (pEntry.cmode==ADSL_PPPoE) ||
#ifdef PPPOE_PASSTHROUGH
		    ((pEntry.cmode==ADSL_BR1483) && (pEntry.cmode==BRIDGE_PPPOE)) ||
#endif
		    (pEntry.cmode==ADSL_PPPoA) )
		{
			if(pEntry.ifIndex == ifindex)
			{
				instnum = pppc;
				pconn = pppstr;
				break;
			}
			pppc++;
		}
		else
		{
			if(pEntry.ifIndex == ifindex)
			{
				instnum = pppc;
				pconn = ipstr;
				break;
			}
			ipc++;
		}
	}

	if( i < total )
	{
		if(pEntry.enable)
			sprintf( name, strfmt, i, pconn, instnum);
		else
			return -1;
	}
	else  //name = InternetGatewayDevice.WANDevice.1.WANConnection.2.WANPPPConnection.1.
		return 0;

	return 1;
}

int GetDefaultRoute( char *name )
{
	int total, i;
	MIB_CE_ATM_VC_T pEntry;
	int pppc = 0, ipc = 0;

	char strfmt[]="InternetGatewayDevice.WANDevice.1.WANConnectionDevice.%d.%s.%d"; //wt-121v8-3.33, no trailing dot
	char ipstr[]="WANIPConnection";
	char pppstr[]="WANPPPConnection";
	char *pconn=NULL;
	unsigned int instnum=0;

	if( name==NULL ) return -1;
	name[0]=0;

#ifdef DEFAULT_GATEWAY_V2
	{
		unsigned char dgw;
		if (mib_get(MIB_ADSL_WAN_DGW_ITF, (void *)&dgw) != 0)
			transfer2PathName(dgw, name);
	}
#else
	total = mib_chain_total(MIB_ATM_VC_TBL);
	for( i=0; i<total; i++ )
	{
		if( !mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&pEntry ) )
			continue;

		if( (pEntry.cmode==ADSL_PPPoE) ||
#ifdef PPPOE_PASSTHROUGH
		    ((pEntry.cmode==ADSL_BR1483)&&(pEntry.cmode==BRIDGE_PPPOE)) ||
#endif
		    (pEntry.cmode==ADSL_PPPoA) )
			{
				if(pEntry.dgw==1)
				{
					instnum = pppc;
					pconn = pppstr;
					break;
				}
				pppc++;
			}
			else
			{
				if(pEntry.dgw==1)
				{
					instnum = ipc;
					pconn = ipstr;
					break;
				}
				ipc++;
			}
		}
#endif
	//name = InternetGatewayDevice.WANDevice.1.WANConnection.2.WANPPPConnection.1.
	if( i < total )
	{
		if( pEntry.enable )
			sprintf(name, strfmt, i, pconn, instnum);
		else
			return -1;
	}
	else
		return 0;

	return 1;
}

bool ChkInputValue(char *str, unsigned int *idx, unsigned int total)
{
	if(!str)
		return false;

	int len = strlen(str);
	int i;

	*idx = UINT_MAX;

	for( i = 0; i < len; i++ )
		if(!isdigit(str[i]))
			return false;

	*idx = atoi(str);

	if( *idx < total )
		return true;
	else
		return false;
}

// map: bit map of used interface, ppp index (0~15) is mapped into high 16 bits,
// while vc index (0~15) is mapped into low 16 bits.
// return: interface index, high nibble for PPP index and low nibble for vc index.
//		0xef: PPP not available
//		0xff: vc not available
static unsigned char if_find_index(int cmode, unsigned int map)
{
	int i;
	unsigned char index;

	// find the first available vc index (mpoa interface)
	i = 0;
	for (i=0; i<MAX_VC_NUM; i++)
	{
		if (!((map>>i) & 1))
			break;
	}

	if (i != MAX_VC_NUM)
		index = i;
	else
		return 0xff;

	if (cmode == ADSL_PPPoE || cmode == ADSL_PPPoA)
	{
		// find an available PPP index
		map >>= 16;
		i = 0;
		while (map & 1)
		{
			map >>= 1;
			i++;
		}
		if (i<=(MAX_PPP_NUM-1))
			index |= i << 4;	// high nibble for PPP index
		else
			return 0xef;

		if (cmode == ADSL_PPPoA)
			index |= 0x0f;	// PPPoA doesn't use mpoa interface, set to 0x0f (don't care)
	}
	else
	{
		// don't care the PPP index
		index |= 0xf0;
	}
	return index;
}

static void resolveServiceDependency(idx)
{
	//MIB_CE_ATM_VC_Tp pEntry;
	MIB_CE_ATM_VC_T Entry;
#ifdef CONFIG_USER_IGMPPROXY
	unsigned char igmp_proxy;
#endif
#ifdef IP_PASSTHROUGH
	unsigned char ippt_itf;
	unsigned int ippt_lease;
#endif
//	struct data_to_pass_st msg;

	/* get the specified chain record */
	if (!mib_chain_get(MIB_ATM_VC_TBL, idx, (void *)&Entry))
	{
		return;
	}

#ifdef APPLY_CHANGE
	stopConnection(&Entry);
#endif

#ifdef CONFIG_USER_IGMPPROXY
	// resolve IGMP proxy dependency
	if(!mib_get( MIB_IGMP_PROXY_ITF,  (void *)&igmp_proxy))
		return;
	if (Entry.ifIndex == igmp_proxy)
	{ // This interface is IGMP proxy interface
		igmp_proxy = 0xff;	// set to default
		mib_set(MIB_IGMP_PROXY_ITF, (void *)&igmp_proxy);
		igmp_proxy = 0;	// disable IGMP proxy
		mib_set(MIB_IGMP_PROXY, (void *)&igmp_proxy);
	}
#endif

#ifdef IP_PASSTHROUGH
	// resolve IP passthrough dependency
	if(!mib_get( MIB_IPPT_ITF,  (void *)&ippt_itf))
		return;
	if (Entry.ifIndex == ippt_itf)
	{ // This interface is IP passthrough interface
		ippt_itf = 0xff;	// set to default
		mib_set(MIB_IPPT_ITF, (void *)&ippt_itf);
		ippt_lease = 600;	// default to 10 min.
		mib_set(MIB_IPPT_LEASE, (void *)&ippt_lease);
	}
#endif

#ifdef CONFIG_EXT_SWITCH
#ifdef ITF_GROUP
	int k;
	// resolve port mapping
	for (k=0; k<SW_PORT_NUM; k++) {
		//MIB_CE_SW_PORT_Tp pSWEntry;
		MIB_CE_SW_PORT_T SWEntry;
		if (!mib_chain_get(MIB_SW_PORT_TBL, k, (void *)&SWEntry))
			return;
		if (SWEntry.pvcItf == Entry.ifIndex) {
			// This interface has been mapped by lan port
			// Detach it!
			SWEntry.pvcItf = 0xff; // set to default
			// log message
			mib_chain_update(MIB_SW_PORT_TBL, (void *)&SWEntry, k);
		}
	}
#endif	// of ITF_GROUP
#endif	// of CONFIG_EXT_SWITCH
}

void Commit(struct upnphttp *h, int dslidx)
{
	int totalEntry, i, drflag = 0;
	MIB_CE_ATM_VC_T entry, Entry;
	bool commit = true;
	unsigned int ifMap;

 	totalEntry = mib_chain_total(MIB_ATM_VC_TBL);

	MIB_CE_ATM_VC_T myEntry;
	int cnt = 0, pIdx = -1, selected;

	if(commit)
	{
		// Mason Yu
		deleteAllConnection();
		selected = -1;
		ifMap = 0;

		if(!mib_chain_get(MIB_ATM_VC_TBL, dslidx, (void *)&entry))
			commit = false;
	}

	if(commit)
	{
		for( i = 0; i < totalEntry; i++ )
		{
			if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			{
				commit = false;
				break;
			}

			if(Entry.vpi == entry.vpi && Entry.vci == entry.vci )
				cnt++;

			if( (selected == -1) && dslidx == i )
				selected = i;
			else
			{
				ifMap |= 1 << (Entry.ifIndex & 0x0f);	 // vc map
				ifMap |= (1 << 16) << ((Entry.ifIndex >> 4) & 0x0f);	 // PPP map
			}

#ifdef DEFAULT_GATEWAY_V1
			if(commit && Entry.cmode != ADSL_BR1483)
				if(Entry.dgw)
					drflag = 1;
#endif
		}

		if (selected == -1)
			commit = false;
	}

	if(commit)
	{
		if(cnt > 0)
		{
			//Make sure there is no mismatch mode
			for( i = 0, cnt = 0; i < totalEntry; i++ )
			{
				if(i == selected)
					continue;

				if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
				{
	  			commit = false;
					break;
				}

				if(Entry.vpi == entry.vpi && Entry.vci == entry.vci)
				{
					cnt++;

					if(Entry.cmode != entry.cmode)
					{
						commit = false;
						break;
					}

					if (entry.cmode == ADSL_PPPoE)		// Jenny, for multisession PPPoE support
						pIdx = i;
				}
			}

			if(commit)
			{
				if( entry.cmode == ADSL_PPPoE && cnt == MAX_POE_PER_VC ) // Jenny, multisession PPPoE support
					commit = false; //Max. 1 connection except PPPoE
				else if( entry.cmode != ADSL_PPPoE && cnt > 0 )
					commit = false;

				if( commit && entry.cmode == ADSL_PPPoE && cnt > 0 )		// Jenny, for multisession PPPoE, get existed PPPoE config for further ifindex use
					if (!mib_chain_get(MIB_ATM_VC_TBL, pIdx, (void *)&myEntry))
						commit = false;
			}
		}
	}

	if(commit)
		if(!mib_chain_get(MIB_ATM_VC_TBL, selected, (void *)&Entry))
			commit = false;

// Added by Mason Yu
#if 0
		if ( (Entry.vpi != entry.vpi) || (Entry.vci != entry.vci) ) {
			strcpy(tmpBuf, T("Can not modify VPI and VCI value!"));
			goto setErr_filter;
		}
#endif
	if(commit)
	{
		// restore stuff not posted in this form
		if(entry.cmode == ADSL_PPPoE)
			if(cnt > 0)
			{		// Jenny, for multisession PPPoE, ifIndex(VC device) must refer to existed PPPoE connection
				ifMap &= 0xffff0000; // don't care the vc part
				entry.ifIndex = if_find_index(entry.cmode, ifMap);
				entry.ifIndex &= 0xf0;
				entry.ifIndex |= (myEntry.ifIndex&0x0f);
			}
			else
			{
				entry.ifIndex = if_find_index(entry.cmode, ifMap);
#ifdef PPPOE_PASSTHROUGH
				if(entry.cmode == Entry.cmode)
					entry.brmode = Entry.brmode;
#endif
			}
		else
			entry.ifIndex = Entry.ifIndex;

		entry.qos = Entry.qos;
		entry.pcr = Entry.pcr;
		entry.scr = Entry.scr;
		entry.mbs = Entry.mbs;
		entry.cdvt = Entry.cdvt;
		entry.pppAuth = Entry.pppAuth;
		entry.rip = Entry.rip;
//		entry.dgw = Entry.dgw;
		entry.mtu = Entry.mtu;
#ifdef CONFIG_EXT_SWITCH
		//ql: when pvc is modified, interface group don't changed???
		entry.itfGroup = Entry.itfGroup;
#endif

#ifdef CONFIG_SPPPD_STATICIP
		if(entry.cmode == ADSL_PPPoE)
		{
			entry.pppIp = Entry.pppIp;
			strcpy( entry.ipAddr, Entry.ipAddr);
		}
#endif
#ifdef PPPOE_PASSTHROUGH
		if (entry.cmode != ADSL_PPPoE)
			if (entry.cmode == Entry.cmode)
				entry.brmode = Entry.brmode;
#endif
#ifdef CONFIG_EXT_SWITCH
		// VLAN
		entry.vlan = Entry.vlan;
		entry.vid = Entry.vid;
		entry.vprio = Entry.vprio;
		entry.vpass = Entry.vpass;
#endif
#ifdef _CWMP_MIB_ //jiunming, for cwmp-tr069
		entry.connDisable = 0;
		if (Entry.vpi == entry.vpi && Entry.vci == entry.vci)
		{
			entry.ConDevInstNum = Entry.ConDevInstNum;
			if( (entry.cmode==ADSL_PPPoE) ||
#ifdef PPPOE_PASSTHROUGH
			    ((entry.cmode==ADSL_BR1483)&&(entry.brmode==BRIDGE_PPPOE)) ||
#endif
			    (entry.cmode==ADSL_PPPoA) )
			{
				if( Entry.ConPPPInstNum!=0 )
					entry.ConPPPInstNum = Entry.ConPPPInstNum;
				else
					entry.ConPPPInstNum = 1 + findMaxPPPConInstNum( entry.ConDevInstNum );
				entry.ConIPInstNum = 0;
			}
			else
			{
				entry.ConPPPInstNum = 0;

				if(Entry.ConIPInstNum!=0)
					entry.ConIPInstNum = Entry.ConIPInstNum;
				else
					entry.ConIPInstNum = 1 + findMaxIPConInstNum( entry.ConDevInstNum );
			}
		}else{
			unsigned int  instnum=0;
			instnum = findConDevInstNumByPVC( entry.vpi, entry.vci );
			if(instnum==0)
				instnum = 1 + findMaxConDevInstNum();
			entry.ConDevInstNum = instnum;
			if( (entry.cmode==ADSL_PPPoE) ||
#ifdef PPPOE_PASSTHROUGH
			    ((entry.cmode==ADSL_BR1483)&&(entry.brmode==BRIDGE_PPPOE)) ||
#endif
			    (entry.cmode==ADSL_PPPoA) )
			{
				entry.ConPPPInstNum = 1 + findMaxPPPConInstNum( entry.ConDevInstNum );
				entry.ConIPInstNum = 0;
			}
			else
			{
				entry.ConPPPInstNum = 0;
				entry.ConIPInstNum = 1 + findMaxIPConInstNum( entry.ConDevInstNum );
			}
		}
		//fprintf( stderr, "<%s:%d>NewInstNum=>ConDev:%u, PPPCon:%u, IPCon:%u\n", __FILE__, __LINE__, entry.ConDevInstNum, entry.ConPPPInstNum, entry.ConIPInstNum );

		entry.autoDisTime = Entry.autoDisTime;
		entry.warnDisDelay = Entry.warnDisDelay;
		strcpy( entry.pppServiceName, Entry.pppServiceName );
		strcpy( entry.WanName, Entry.WanName );
#ifdef _PRMT_X_CT_COM_PPPOEv2_
		entry.PPPoEProxyEnable = Entry.PPPoEProxyEnable;
		entry.PPPoEProxyMaxUser = Entry.PPPoEProxyMaxUser;
#endif //_PRMT_X_CT_COM_PPPOEv2_
#ifdef _PRMT_X_CT_COM_WANEXT_
		entry.ServiceList = Entry.ServiceList;
#endif //_PRMT_X_CT_COM_WANEXT_
#endif //_CWMP_MIB_
		strcpy(entry.pppACName, Entry.pppACName);

		// disable this interface
		if (entry.enable == 0 && Entry.enable == 1)
			resolveServiceDependency(selected);

#ifdef DEFAULT_GATEWAY_V1
		// set default route flag
		if (entry.cmode != ADSL_BR1483)
		{
			if (!entry.dgw && Entry.dgw)
				drflag =0;
		}
		else
		{
			if (Entry.dgw)
				drflag = 0;
		}
#endif

		// find the ifIndex
		if (entry.cmode != Entry.cmode)
		{
			if (!(entry.cmode == ADSL_PPPoE && cnt>0))	// Jenny, entries except multisession PPPoE
				entry.ifIndex = if_find_index(entry.cmode, ifMap);

			if (entry.ifIndex == 0xff)
				commit = false;
			else if (entry.ifIndex == 0xef)
				commit = false;

			// mode changed, restore to default
			if(commit)
			{
				if(entry.cmode == ADSL_PPPoE)
				{
					entry.mtu = 1492;
#ifdef CONFIG_USER_PPPOE_PROXY
					entry.PPPoEProxyMaxUser = 4;
#endif
					entry.pppAuth = 0;
				}
				else
				{
#ifdef CONFIG_USER_PPPOE_PROXY
					entry.PPPoEProxyMaxUser = 0;
#endif
					entry.mtu = 1500;
				}
#ifdef CONFIG_EXT_SWITCH
				// VLAN
				entry.vlan = 0;	// disable
				entry.vid = 0; // VLAN tag
				entry.vprio = 0; // priority bits (0 ~ 7)
				entry.vpass = 0; // no pass-through
#endif
			}
		}
	}

#ifdef DEFAULT_GATEWAY_V1
	if(commit)
	{
		if(entry.cmode != ADSL_BR1483)
		{
			if(drflag && entry.dgw && !Entry.dgw)
				commit = false;

			if(commit && entry.dgw && !Entry.dgw)
				drflag = 1;
		}
	}
#endif
#ifdef ZTE_GENERAL_ROUTER_SC
	MIB_CE_ATM_VC_T accEntry;
#endif

	if(commit)
	{
#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
		if( entry.ifIndex!=Entry.ifIndex )
		{
			updatePortForwarding( Entry.ifIndex, entry.ifIndex );
			updateRoutingTable( Entry.ifIndex, entry.ifIndex );
		}
#endif

#ifdef CONFIG_ATM_CLIP
		if (entry.cmode == ADSL_RT1577)
			entry.encap = 1;	// LLC
#endif

#ifdef APPLY_CHANGE
		stopConnection(&Entry);
		startConnection(&entry);
#endif
		// log message
//add by ramen for ZTE ROTUER rmt acc on pvc
#ifdef ZTE_GENERAL_ROUTER_SC
		if(Entry.cmode!=entry.cmode)
			filter_set_remote_access(0);

		if(!mib_chain_get(MIB_ACC_TBL, selected, &accEntry))
			commit = false;
	}

	MIB_CE_ATM_VC_T dnsPvcEntry;

	if(commit)
	{
		copyPvcEntryToAccEntry(&accEntry, &entry);

		if( Entry.cmode != entry.cmode )
	 	{
			assignDefaultValue(&accEntry);
			mib_chain_update(MIB_ACC_TBL,&accEntry,selected);
			filter_set_remote_access(1);
	 	}
		else
			mib_chain_update(MIB_ACC_TBL,&accEntry,selected);
#endif
//add by ramen for DNS bind pvc
#ifdef DNS_BIND_PVC_SUPPORT

		if(mib_chain_get(MIB_ATM_VC_TBL,selected,&dnsPvcEntry)&&(dnsPvcEntry.cmode!=ADSL_BR1483))
		{
			int tempi=0;
			unsigned char pvcifIdx=0;

			for(tempi=0;tempi<3;tempi++)
			{
				mib_get(MIB_DNS_BIND_PVC1+tempi,(void*)&pvcifIdx);

				if(pvcifIdx==dnsPvcEntry.ifIndex)//I get it
				{
					if(entry.cmode==ADSL_BR1483)
						pvcifIdx = 255;
					else
						pvcifIdx=entry.ifIndex;

					mib_set(MIB_DNS_BIND_PVC1+tempi,(void*)&pvcifIdx);
				}
			}
		}
#endif
        //jim garbage action...
		//memcpy(&Entry, &entry, sizeof(entry));
		// log message
		mib_chain_update(MIB_ATM_VC_TBL, (void *)&entry, selected);
#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
		restart_dnsrelay(); //star
#endif
        //jim moved here from line 2055 for do connection after mib updated...
#ifdef ZTE_531B_BRIDGE_SC
		unsigned char vChar;
#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		//to avoid the dead when wrriting the flash
		itfcfg("sar", 0);
		itfcfg("eth0", 0);
#ifdef WLAN_SUPPORT
		itfcfg("wlan0", 0);
#endif
#endif //__CLOSE_INTERFACE_BEFORE_WRITE_
		mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);

#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		itfcfg("sar", 1);
		itfcfg("eth0", 1);
#ifdef WLAN_SUPPORT
		vChar=0;
		mib_get( MIB_WLAN_DISABLED, (void *)&vChar);

		if( vChar==0 )
			itfcfg("wlan0", 1);
#endif //WLAN_SUPPORT
#endif //__CLOSE_INTERFACE_BEFORE_WRITE_
#endif
//#ifdef CONFIG_GUI_WEB
		writeFlash();
//#endif
	// Added by Mason Yu. for take effect in real time
		RegisterPVCnumber();
		cleanAllFirewallRule();
		restart_dnsrelay(); //Jenny
		startWan(BOOT_LAST);
	}
}

static const TimeZone timeZoneTable[] =
{
	{"(GMT-12:00)Eniwetok, Kwajalein", 12, 1},
	{"(GMT-11:00)Midway Island, Samoa", 11, 1},
	{"(GMT-10:00)Hawaii", 10, 1},
	{"(GMT-09:00)Alaska", 9, 1},
	{"(GMT-08:00)Pacific Time (US and Canada), Tijuana", 8, 1},
	{"(GMT-07:00)Arizona", 7, 1},
	{"(GMT-07:00)Mountain Time (US and Canada)", 7, 2},
	{"(GMT-06:00)Central Time (US and Canada)", 6, 1},
	{"(GMT-06:00)Mexico City, Tegucigalpa", 6, 2},
	{"(GMT-06:00)Saskatchewan", 6, 3},
	{"(GMT-05:00)Bogota, Lima, Quito", 5, 1},
	{"(GMT-05:00)Eastern Time (US and Canada)", 5, 2},
	{"(GMT-05:00)Indiana (East)", 5, 3},
	{"(GMT-04:00)Atlantic Time (Canada)", 4, 1},
	{"(GMT-04:00)Caracas, La Paz", 4, 2},
	{"(GMT-04:00)Santiago", 4, 3},
	{"(GMT-03:30)Newfoundland", 3, 1},
	{"(GMT-03:00)Brasilia", 3, 2},
	{"(GMT-03:00)Buenos Aires, Georgetown", 3, 3},
	{"(GMT-02:00)Mid-Atlantic", 2, 1},
	{"(GMT-01:00)Azores, Cape Verde Is.", 1, 1},
	{"(GMT)Casablanca, Monrovia", 0, 1},
	{"(GMT)Greenwich Mean Time, Dublin, Edinburgh, Lisbon, London", 0, 2},
	{"(GMT+01:00)Amsterdam, Berlin, Bern, Rome, Stockholm, Vienna", -1, 1},
	{"(GMT+01:00)Belgrade, Bratislava, Budapest, Ljubljana, Prague", -1, 2},
	{"(GMT+01:00)Barcelona, Madrid", -1, 3},
	{"(GMT+01:00)Brussels, Copenhagen, Madrid, Paris, Vilnius", -1, 4},
	{"(GMT+01:00)Paris", -1, 5},
	{"(GMT+01:00)Sarajevo, Skopje, Sofija, Warsaw, Zagreb", -1, 6},
	{"(GMT+02:00)Athens, Istanbul, Minsk", -2, 1},
	{"(GMT+02:00)Bucharest", -2, 2},
	{"(GMT+02:00)Cairo", -2, 3},
	{"(GMT+02:00)Harare, Pretoria", -2, 4},
	{"(GMT+02:00)Helsinki, Riga, Tallinn", -2, 5},
	{"(GMT+02:00)Jerusalem", -2, 6},
	{"(GMT+03:00)Baghdad, Kuwait, Riyadh", -3, 1},
	{"(GMT+03:00)Moscow, St. Petersburg, Volgograd", -3, 2},
	{"(GMT+03:00)Mairobi", -3, 3},
	{"(GMT+03:30)Tehran", -3, 4},
	{"(GMT+04:00)Abu Dhabi, Muscat", -4, 1},
	{"(GMT+04:00)Baku, Tbilisi", -4, 2},
	{"(GMT+04:30)Kabul", -4, 3},
	{"(GMT+05:00)Ekaterinburg", -5, 1},
	{"(GMT+05:00)Islamabad, Karachi, Tashkent", -5, 2},
	{"(GMT+05:30)Bombay, Calcutta, Madras, New Delhi", -5, 3},
	{"(GMT+06:00)Astana, Almaty, Dhaka", -6, 1},
	{"(GMT+06:00)Colombo", -6, 2},
	{"(GMT+07:00)Bangkok, Hanoi, Jakarta", -7, 1},
	{"(GMT+08:00)Beijing, Chongqing, Hong Kong, Urumqi", -8, 1},
	{"(GMT+08:00)Perth", -8, 2},
	{"(GMT+08:00)Singapore", -8, 3},
	{"(GMT+08:00)Taipei", -8, 4},
	{"(GMT+09:00)Osaka, Sapporo, Tokyo", -9, 1},
	{"(GMT+09:00)Seoul", -9, 2},
	{"(GMT+09:00)Yakutsk", -9, 3},
	{"(GMT+09:30)Adelaide", -9, 4},
	{"(GMT+09:30)Darwin", -9, 5},
	{"(GMT+10:00)Brisbane", -10, 1},
	{"(GMT+10:00)Canberra, Melbourne, Sydney", -10, 2},
	{"(GMT+10:00)Guam, Port Moresby", -10, 3},
	{"(GMT+10:00)Hobart", -10, 4},
	{"(GMT+10:00)Vladivostok", -10, 5},
	{"(GMT+11:00)Magadan, Solomon Is., New Caledonia", -11, 1},
	{"(GMT+12:00)Auckland, Wllington", -12, 1},
	{"(GMT+12:00)Fiji, Kamchatka, Marshall Is.", -12, 2},
	{0, 0, 0},
};
#endif

#ifdef TR_064
static void DeviceConfigReboot(struct upnphttp * h)
{
	static const char resp[] =
		"<u:RebootResponse "
		"xmlns:u=\"urn:dslforum-org:service:DeviceConfig:1\">"
		"</u:RootResponse>";
	BuildSendAndCloseSoapResp(h, resp, sizeof(resp)-1);

	cmd_reboot();
}

static void DeviceConfigFactoryReset(struct upnphttp * h)
{
	static const char resp[] =
	"<u:FactoryResetResponse "
	"xmlns:u=\"urn:dslforum-org:service:DeviceConfig:1\">"
	"</u:RootResponse>";
	BuildSendAndCloseSoapResp(h, resp, sizeof(resp)-1);

	strcpy(A_ARG_TYPE_Status, "RebootRequired");

	va_cmd("/bin/flash", 2, 1, "default", "cs");
	cmd_reboot();
}

static void DeviceConfigConfigurationStarted(struct upnphttp *h)
{
	struct NameValueParserData data;
	char *uuid;

	static const char resp[] =
	"<u:ConfigurationStartedResponse "
	"xmlns:u=\"urn:dslforum-org:service:DeviceConfig:1\">"
	"</u:ConfigurationStartedResponse>";

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	uuid = GetValueFromNameValueList(&data, "NewSessionID");

	if( uuid && strlen(uuid) < 37 )
		strcpy(A_ARG_TYPE_UUID, uuid);
	else
	{
		strcpy(A_ARG_TYPE_UUID, "Invalid UUID format!");
		syslog(LOG_ERR, "UPNP ERR: Invalid UUID format!");
		SoapError(h, 402, "Invalid Args");
	}

	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void DeviceConfigConfigurationFinished(struct upnphttp *h)
{
	static char resp[192];

	strcpy(resp, "<u:ConfigurationFinishedResponse xmlns:u=\"urn:dslforum-org:service:DeviceConfig:1\"><NewStatus>");
	strcat(resp, A_ARG_TYPE_Status);
	strcat(resp, "</NewStatus>\n</u:ConfigurationFinishedResponse>");

	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void DeviceInfoGetInfo(struct upnphttp *h)
{
	char resp[1024];
	char model_name[50], sw_version[50], fw_version[20], e_time[12], pcode[64], serial[32];
	struct sysinfo info;
	FILE *fp;

	strcpy(resp, "<u:GetInfoResponse xmlns:u=\"urn:dslforum-org:service:DeviceInfo:1\">\n");
	strcat(resp, "<NewManufacturerName>");
	strcat(resp, MANUFACTURER_NAME);
	strcat(resp, "</NewManufacturerName>\n");
	strcat(resp, "<NewManufacturerOUI>");
	strcat(resp, MANUFACTURER_OUI);
	strcat(resp, "</NewManufacturerOUI>\n");

  if(!mib_get(MIB_SNMP_SYS_NAME, model_name))
	{
  	strcpy(model_name, "Get model name failed!");
  	syslog(LOG_ERR, "UPNP ERR: Get model name failed!\n");
  	SoapError(h, 501, "Action Failed: Get model name failed!");
	}

  strcat(resp, "<NewModelName>");
	strcat(resp, model_name);
	strcat(resp, "</NewModelName>\n");
	strcat(resp, "<NewDescription>");
	strcat(resp, DESCRIPTION);
	strcat(resp, "</NewDescription>\n");
	strcat(resp, "<NewProductClass>");
	strcat(resp, PRODUCT_CLASS);
	strcat(resp, "</NewProductClass>\n");

	if(!mib_get(MIB_HW_SERIAL_NUMBER, (void *)serial))
	{
  	strcpy(model_name, "Get serial number failed!");
  	syslog(LOG_ERR, "UPNP ERR: Get serial number failed!\n");
  	SoapError(h, 501, "Action Failed: Get serial number failed!");
	}

	strcat(resp, "<NewSerialNumber>");
	strcat(resp, serial);
	strcat(resp, "</NewSerialNumber>\n");

	fp = fopen("/etc/version", "r");
	if(fp != NULL)
	{
		char *tmp;

		fgets(sw_version, sizeof(sw_version), fp);  //main version
		fclose(fp);
		tmp = strchr(sw_version, ' ');
		*tmp = 0;
	}
	else
	{
		strcpy(sw_version, "Get software version failed!");
		syslog(LOG_ERR, "Get software version failed!");
		SoapError(h, 501, "Action Failed");
	}

	strcat(resp, "<NewSoftwareVersion>");
	strcat(resp, sw_version);
	strcat(resp, "</NewSoftwareVersion>\n");

	getAdslDrvInfo("version", fw_version, sizeof(fw_version));

	strcat(resp, "<NewModemFirmwareVersion>");
	strcat(resp, fw_version);
	strcat(resp, "</NewModemFirmwareVersion>\n");
	strcat(resp, "<NewHardwareVersion>");
	strcat(resp, HW_VERSION_STR);
	strcat(resp, "</NewHardwareVersion>\n");
	strcat(resp, "<NewSpecVersion>");
	strcat(resp, SPEC_VERSION);
	strcat(resp, "</NewSpecVersion>\n");

#ifdef CONFIG_USER_CWMP_TR069
	if(!mib_get(CWMP_PROVISIONINGCODE, pcode))  // Refered from TR-069
	{
		strcpy(pcode, "Get ProvisioningCode failed!");
		syslog(LOG_ERR, "UPNP ERR: Get ProvisioningCode failed!");
		SoapError(h, 501, "Action Failed");
	}
#else
	strcpy(pcode, "TR-069 must enable!");
	syslog(LOG_ERR, "UPNP ERR: TR-069 must enable!");
#endif

	strcat(resp, "<NewProvisioningCode>");
	strcat(resp, pcode);
	strcat(resp, "</NewProvisioningCode>\n");

	sysinfo(&info);
	sprintf(e_time, "%d", (int)info.uptime);

	strcat(resp, "<NewUpTime>");
	strcat(resp, e_time);
	strcat(resp, "</NewUpTime>\n");
	strcat(resp, "</u:GetInfoResponse>");

	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void DeviceInfoSetProvisioningCode(struct upnphttp *h)
{
	struct NameValueParserData data;

	static const char resp[] =
	"<u:SetProvisioningCodeResponse "
	"xmlns:u=\"urn:dslforum-org:service:DeviceInfo:1\">"
	"</u:SetProvisioningCodeResponse>";

#ifdef CONFIG_USER_CWMP_TR069
	char *pro_code;
	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	pro_code = GetValueFromNameValueList(&data, "NewProvisioningCode");

	if(pro_code != NULL && strlen(pro_code) <= 64)
	{
		if(!mib_set(CWMP_PROVISIONINGCODE, pro_code))  // Refered from TR-069
			syslog(LOG_ERR, "Set ProvisioningCode failed!");
	}
	else
	{
		syslog(LOG_ERR, "The length of ProvisioningCode is too long or zero!");
		SoapError(h, 402, "Invalid Args");
	}
#else
	syslog(LOG_ERR, "UPNP ERR: TR-069 must enable!");
#endif

	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void DeviceInfoGetDeviceLog(struct upnphttp *h)
{
	const int log_len = 32768;
	char *resp;
	char *sys_log;
	FILE *fp;
	int i;
	unsigned int filesize = 0;
	struct stat fileinfo;

	fp = fopen("/var/log/messages", "r");

	if( fp == NULL)
	{
		sys_log = strdup("Get system log failed!");
		syslog(LOG_ERR, "UPNP ERR: Get system log failed!");
		SoapError(h, 402, "Invalid Args");
	}
	else
	{
		//Get file size
		if(!stat("/var/log/messages", &fileinfo))
			filesize = fileinfo.st_size;

		if( filesize > log_len )  // Log size must smaller than 32K.
		{
			sys_log = strdup("Log length is too long!");
			syslog(LOG_ERR, "UPNP ERR: Log length is too long!");
			SoapError(h, 501, "Action Failed");
		}
		else
		{
			sys_log = (char *)malloc((filesize+1) * sizeof(char));
			fread((void *)sys_log, 1, filesize, fp);
		}
		fclose(fp);
	}

	resp = (char *)malloc((filesize + 256) * sizeof(char));

	int loglen = strlen(sys_log);
	for( i = 0; i < loglen; i++ )  // Replace character <, > to [, ] respectively, because of preventing ambiguous of xml format.
	{
		if(sys_log[i] == '<')
			sys_log[i] = '[';
		else if(sys_log[i] == '>')
			sys_log[i] = ']';
	}

	strcpy(resp, "<u:GetDeviceLogResponse xmlns:u=\"urn:dslforum-org:service:DeviceInfo:1\">\n<NewDeviceLog>");
	strcat(resp, sys_log);
	strcat(resp, "</NewDeviceLog>\n</u:GetDeviceLogResponse>");

	BuildSendAndCloseSoapResp(h, resp, strlen(resp));

	free(sys_log);
	free(resp);
}

static void Layer3ForwardingGetDefaultConnectionService(struct upnphttp *h)
{
	char resp[512];
	char def_cs[128];

	strcpy(resp, "<u:GetDefaultConnectionServiceResponse xmlns:u=\"urn:dslforum-org:service:Layer3Forwarding:1\">\n");

	int dfrt = GetDefaultRoute(def_cs);

	if( dfrt == -1 )
	{
		strcpy(def_cs, "Get DefaultConnectionService failed!");
		syslog(LOG_ERR, "UPNP ERR: Get DefaultConnectionService failed!");
		SoapError(h, 501, "Action Failed: Default route is disabled!");
	}
	else	if( dfrt == 0 )
	{
		strcpy(def_cs, "Default route not found!");
		syslog(LOG_ERR, "UPNP ERR: Default route not found!");
		SoapError(h, 501, "Action Failed: Default route not found!");
	}

	strcat(resp, "<NewDefaultConnectionService>");
	strcat(resp, def_cs);
	strcat(resp, "</NewDefaultConnectionService>\n");
	strcat(resp, "</u:GetDefaultConnectionServiceResponse>");
	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void Layer3ForwardingSetDefaultConnectionService(struct upnphttp *h)
{
	static const char resp[] =
		"<u:SetDefaultConnectionServiceResponse "
		"xmlns:u=\"urn:dslforum-org:service:Layer3Forwarding:1\">\n"
		"</u:SetDefaultConnectionServiceResponse>\n";
	struct NameValueParserData data;
	char * p;
	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	p = GetValueFromNameValueList(&data, "NewDefaultConnectionService");
	if(p) {
		syslog(LOG_INFO, "SetDefaultConnectionService(%s) : Ignored", p);
	}
	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, sizeof(resp)-1);
}

static void Layer3ForwardingGetForwardNumberOfEntries(struct upnphttp *h)
{
	char resp[256];
	char fw_num_enty[16];
	unsigned int total = 0;

	strcpy(resp, "<u:GetForwardNumberOfEntriesResponse xmlns:u=\"urn:dslforum-org:service:Layer3Forwarding:1\">\n");

	total = mib_chain_total(MIB_IP_ROUTE_TBL);
	sprintf(fw_num_enty, "%d", total);

	syslog(LOG_ERR, "UPNP ERR: Layer3Forwarding: %s", fw_num_enty);

	strcat(resp, "<NewForwardNumberOfEntries>");
	strcat(resp, fw_num_enty);
	strcat(resp, "</NewForwardNumberOfEntries>\n");
	strcat(resp, "</u:GetForwardNumberOfEntriesResponse>");
	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void Layer3ForwardingAddForwardingEntry(struct upnphttp *h)
{
	struct NameValueParserData data;
	MIB_CE_IP_ROUTE_T *fwen, fwtmp;
	char *str;
	int chk = 0;
	struct in_addr ip;

	static const char resp[] = "<u:AddForwardingEntryResponse xmlns:u=\"urn:dslforum-org:service:Layer3Forwarding:1\"></u:AddForwardingEntryResponse>";
	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	memset(&fwtmp, 0, sizeof(MIB_CE_IP_ROUTE_T));
	fwen = &fwtmp;

	if((str = GetValueFromNameValueList(&data, "NewType")))
	{
		if(!strcasecmp(str, "Network"))
		{
			fwen->Type = 0;
			chk++;
		}
		else if(!strcasecmp(str, "Host"))
		{
			fwen->Type = 1;
			chk++;
		}
		else if(!strcasecmp(str, "Default"))
		{
			fwen->Type = 2;
			chk++;
		}
		else
		{
			syslog(LOG_ERR, "UPNP ERR: Invalid Type value!");
			SoapError(h, 402, "Invalid Args");
		}
	}
	else
	{
		syslog(LOG_ERR, "UPNP ERR: Invalid NewType value!");
		SoapError(h, 402, "Invalid Args");
	}

	if((str = GetValueFromNameValueList(&data, "NewDestIPAddress")))
	{
		if(inet_aton(str, &ip))
		{
			memcpy(fwen->destID, &ip, IP_ADDR_LEN);
			chk++;
		}
		else
		{
			syslog(LOG_ERR, "UPNP ERR: Invalid DestIPAddress value!");
			SoapError(h, 402, "Invalid Args");
		}
	}
	else
	{
		syslog(LOG_ERR, "UPNP ERR: Invalid DestIPAddress value!");
		SoapError(h, 402, "Invalid Args");
	}

	if((str = GetValueFromNameValueList(&data, "NewDestSubnetMask")))
	{
		if(inet_aton(str, &ip))
		{
			memcpy(fwen->netMask, &ip, IP_ADDR_LEN);
			chk++;
		}
		else
		{
			syslog(LOG_ERR, "UPNP ERR: Invalid DestSubnetMask value!");
			SoapError(h, 402, "Invalid Args");
		}
	}
	else
	{
		syslog(LOG_ERR, "UPNP ERR: Invalid DestSubnetMask value!");
		SoapError(h, 402, "Invalid Args");
	}

	if((str = GetValueFromNameValueList(&data, "NewSourceIPAddress")))
	{
		if(inet_aton(str, &ip))
		{
			memcpy(fwen->SourceIP, &ip, IP_ADDR_LEN);
			chk++;
		}
		else
		{
			syslog(LOG_ERR, "UPNP ERR: Invalid SourceIPAddress value!");
			SoapError(h, 402, "Invalid Args");
		}
	}
	else
	{
		syslog(LOG_ERR, "UPNP ERR: Invalid SourceIPAddress value!");
		SoapError(h, 402, "Invalid Args");
	}

	if((str = GetValueFromNameValueList(&data, "NewSourceSubnetMask")))
	{
		if(inet_aton(str, &ip))
		{
			memcpy(fwen->SourceMask, &ip, IP_ADDR_LEN);
			chk++;
		}
		else
		{
			syslog(LOG_ERR, "UPNP ERR: Invalid SourceSubnetMask value!");
			SoapError(h, 402, "Invalid Args");
		}
	}
	else
	{
		syslog(LOG_ERR, "UPNP ERR: Invalid SourceSubnetMask value!");
		SoapError(h, 402, "Invalid Args");
	}

	if((str = GetValueFromNameValueList(&data, "NewGatewayIPAddress")))
	{
		if(inet_aton(str, &ip))
		{
			memcpy(fwen->nextHop, &ip, IP_ADDR_LEN);
			chk++;
		}
		else
		{
			syslog(LOG_ERR, "UPNP ERR: Invalid GatewayIPAddress value!");
			SoapError(h, 402, "Invalid Args");
		}
	}
	else
	{
		syslog(LOG_ERR, "UPNP ERR: Invalid GatewayIPAddress value!");
		SoapError(h, 402, "Invalid Args");
	}

	if((str = GetValueFromNameValueList(&data, "NewInterface")) && strlen(str) < 256)
	{
		fwen->ifIndex = atoi(str);
		chk++;
	}
	else
	{
		syslog(LOG_ERR, "UPNP ERR: Invalid Interface value!");
		SoapError(h, 402, "Invalid Args");
	}

	if((str = GetValueFromNameValueList(&data, "NewForwardingMetric")))
	{
		fwen->FWMetric = atoi(str);
		chk++;
	}
	else
	{
		syslog(LOG_ERR, "UPNP ERR: Invalid ForwardingMetric value!");
		SoapError(h, 402, "Invalid Args");
	}

	if (!checkRoute(*fwen, -1))	// Jenny
	{
		syslog(LOG_ERR, "UPNP ERR: Route is existed!");
		SoapError(h, 402, "Invalid Args: Route is existed!");
	}

	if( chk == 8 )   // All input value are valid.
	{
		fwen->Enable = true;  // Enable = true by default
		mib_chain_add(MIB_IP_ROUTE_TBL, (unsigned char *)fwen);

		route_cfg_modify(fwen, 0);
	}

	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void Layer3ForwardingDeleteForwardingEntry(struct upnphttp *h)
{
	struct NameValueParserData data;
	MIB_CE_IP_ROUTE_T fwen;
	struct in_addr ip, r_ip;
	char *str;
	int total, i, chk = 0;

	static const char resp[] = "<u:DeleteForwardingEntryResponse xmlns:u=\"urn:dslforum-org:service:Layer3Forwarding:1\"></u:DeleteForwardingEntryResponse>";
	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	total = mib_chain_total(MIB_IP_ROUTE_TBL);

	for( i = 0; i < total; i++ )
	{
		chk = 0;

		if(mib_chain_get(MIB_IP_ROUTE_TBL, i, (void *)&fwen))
		{
			if((str = GetValueFromNameValueList(&data, "NewDestIPAddress")) && inet_aton(str, &ip))
			{
				memcpy(&r_ip, fwen.destID, IP_ADDR_LEN);
				r_ip.s_addr == ip.s_addr ? chk++: chk;
			}

			if((str = GetValueFromNameValueList(&data, "NewDestSubnetMask")) && inet_aton(str, &ip))
			{
				memcpy(&r_ip, fwen.netMask, IP_ADDR_LEN);
				r_ip.s_addr == ip.s_addr ? chk++: chk;
			}
		#if 0
			if((str = GetValueFromNameValueList(&data, "NewSourceIPAddress")) && inet_aton(str, &ip))
			{
				memcpy(&r_ip, fwen.SourceIP, IP_ADDR_LEN);
				r_ip.s_addr == ip.s_addr ? chk++: chk;
			}

			if((str = GetValueFromNameValueList(&data, "NewSourceSubnetMask")) && inet_aton(str, &ip))
			{
				memcpy(&r_ip, fwen.SourceMask, IP_ADDR_LEN);
				r_ip.s_addr == ip.s_addr ? chk++: chk;
			}
		#endif

			if( chk == 2 )
				break;
		}
		else
		{
			syslog(LOG_ERR, "UNPN ERR: Get MIB_IP_ROUTE_TBL chain record failed!");
			SoapError(h, 501, "Action Failed");
			break;
		}
	}

	if(chk == 2)
	{
		mib_chain_delete(MIB_IP_ROUTE_TBL, i);
		route_cfg_modify(&fwen, 1);
	}
	else
	{
		syslog(LOG_ERR, "UNPN ERR: Layer3Forwarding.DeleteForwardingEntry: No such entry!");
		SoapError(h, 402, "Invalid Args: No such entry!");
	}

	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void Layer3ForwardingGetSpecificForwardingEntry(struct upnphttp *h)
{
	struct NameValueParserData data;
	MIB_CE_IP_ROUTE_T *fwen, fwtmp;
	struct in_addr ip, r_ip;
	char *str, resp[512], gateway_ip[] = "No IP available!", enable[] = "0", status[] = "Error\0     ", type[] = "Default", fw_matric[] = "0\099999999999";
	char ifidx[] = "N/A";
	int total, i, chk = 0;

	fwen = &fwtmp;
	strcpy(resp, "<u:GetSpecificForwardingEntryResponse xmlns:u=\"urn:dslforum-org:service:Layer3Forwarding:1\">");

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	total = mib_chain_total(MIB_IP_ROUTE_TBL);

	for( i = 0; i < total; i++ )
	{
		chk = 0;

		if(mib_chain_get(MIB_IP_ROUTE_TBL, i, (void *)fwen))
		{
			if((str = GetValueFromNameValueList(&data, "NewDestIPAddress")) && inet_aton(str, &ip))
			{
				memcpy(&r_ip, fwen->destID, IP_ADDR_LEN);
				r_ip.s_addr == ip.s_addr ? chk++: chk;
			}

			if((str = GetValueFromNameValueList(&data, "NewDestSubnetMask")) && inet_aton(str, &ip))
			{
				memcpy(&r_ip, fwen->netMask, IP_ADDR_LEN);
				r_ip.s_addr == ip.s_addr ? chk++: chk;
			}

#if 0
			if((str = GetValueFromNameValueList(&data, "NewSourceIPAddress")) && inet_aton(str, &ip))
			{
				memcpy(&r_ip, fwen->SourceIP, IP_ADDR_LEN);
				r_ip.s_addr == ip.s_addr ? chk++: chk;
			}

			if((str = GetValueFromNameValueList(&data, "NewSourceSubnetMask")) && inet_aton(str, &ip))
			{
				memcpy(&r_ip, fwen->SourceMask, IP_ADDR_LEN);
				r_ip.s_addr == ip.s_addr ? chk++: chk;
			}
#endif

			if( chk == 2 )
			{
				memcpy(&ip, fwen->nextHop, IP_ADDR_LEN);
				strcpy(gateway_ip, inet_ntoa(ip));
				sprintf(enable, "%d", fwen->Enable);
				strcpy(status, enable? "Enabled": "Disabled");

				switch(fwen->Type)
				{
					case 0: strcpy(type, "Network");
						break;
					case 1: strcpy(type, "Host");
						break;
					case 2: strcpy(type, "Default");
						break;
				}

				sprintf(ifidx, "%d", fwen->ifIndex);
				sprintf(fw_matric, "%d", fwen->FWMetric);
				break;
			}
		}
		else
		{
			syslog(LOG_ERR, "UNPN ERR: Get MIB_IP_ROUTE_TBL chain record failed!");
			SoapError(h, 501, "Action Failed");
			break;
		}
	}

 	strcat(resp, "<NewGatewayIPAddress>");
 	strcat(resp, gateway_ip);
 	strcat(resp, "</NewGatewayIPAddress>");
 	strcat(resp, "<NewEnable>");
 	strcat(resp, enable);
 	strcat(resp, "</NewEnable>");
 	strcat(resp, "<NewStatus>");
 	strcat(resp, status);
 	strcat(resp, "</NewStatus>");
 	strcat(resp, "<NewType>");
 	strcat(resp, type);
 	strcat(resp, "</NewType>");
 	strcat(resp, "<NewInterface>");
 	strcat(resp, ifidx);
 	strcat(resp, "</NewInterface>");
 	strcat(resp, "<NewForwardingMetric>");
 	strcat(resp, fw_matric);
 	strcat(resp, "</NewForwardingMetric>");
	strcat(resp, "</u:GetSpecificForwardingEntryResponse>");

	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void Layer3ForwardingSetForwardingEntryEnable(struct upnphttp *h)
{
	struct NameValueParserData data;
	MIB_CE_IP_ROUTE_T fwen;
	struct in_addr ip, r_ip;
	char *str;
	int total, i, chk = 0;

	static const char resp[] = "<u:SetForwardingEntryEnableResponse xmlns:u=\"urn:dslforum-org:service:Layer3Forwarding:1\"></u:SetForwardingEntryEnableResponse>";
	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	total = mib_chain_total(MIB_IP_ROUTE_TBL);

	for( i = 0; i < total; i++ )
	{
		chk = 0;

		if(mib_chain_get(MIB_IP_ROUTE_TBL, i, (void *)&fwen))
		{
			if((str = GetValueFromNameValueList(&data, "NewDestIPAddress")) && inet_aton(str, &ip))
			{
				memcpy(&r_ip, fwen.destID, IP_ADDR_LEN);
				r_ip.s_addr == ip.s_addr ? chk++: chk;
			}

			if((str = GetValueFromNameValueList(&data, "NewDestSubnetMask")) && inet_aton(str, &ip))
			{
				memcpy(&r_ip, fwen.netMask, IP_ADDR_LEN);
				r_ip.s_addr == ip.s_addr ? chk++: chk;
			}

#if 0
			if((str = GetValueFromNameValueList(&data, "NewSourceIPAddress")) && inet_aton(str, &ip))
			{
				memcpy(&r_ip, fwen.SourceIP, IP_ADDR_LEN);
				r_ip.s_addr == ip.s_addr ? chk++: chk;
			}

			if((str = GetValueFromNameValueList(&data, "NewSourceSubnetMask")) && inet_aton(str, &ip))
			{
				memcpy(&r_ip, fwen.SourceMask, IP_ADDR_LEN);
				r_ip.s_addr == ip.s_addr ? chk++: chk;
			}
#endif

			if( chk == 2 && (str = GetValueFromNameValueList(&data, "NewEnable")) )
			{
				route_cfg_modify(&fwen, 1);
				fwen.Enable = atoi(str);
				route_cfg_modify(&fwen, 0);
				mib_chain_update(MIB_IP_ROUTE_TBL, (unsigned char *)&fwen, i);
				break;
			}
		}
		else
		{
			syslog(LOG_ERR, "UNPN ERR: Get MIB_IP_ROUTE_TBL chain record failed!");
			SoapError(h, 501, "Action Failed");
			break;
		}
	}

	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void Layer3ForwardingGetGenericForwardingEntry(struct upnphttp *h)
{
	struct NameValueParserData data;
	MIB_CE_IP_ROUTE_T fwen;
	struct in_addr ip;
	char *str, resp[512], gateway_ip[] = "No IP available!", enable[] = "0", status[] = "Error\0     ", type[] = "Default", fw_matric[] = "0\099999999999";
	char dest_ip[] = "No IP available!", dest_mask[] = "No IP available!", source_ip[] = "No IP available!", source_mask[] = "No IP available!", interface_idx[] = "N/A";
	unsigned int total, idx;

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	total = mib_chain_total(MIB_IP_ROUTE_TBL);

	if((str = GetValueFromNameValueList(&data, "NewForwardingIndex")) && ChkInputValue(str, &idx, total))
	{
		if(mib_chain_get(MIB_IP_ROUTE_TBL, idx, (void *)&fwen))
		{
			memcpy(&ip, fwen.destID, IP_ADDR_LEN);
			strcpy(dest_ip, inet_ntoa(ip));

			memcpy(&ip, fwen.netMask, IP_ADDR_LEN);
			strcpy(dest_mask, inet_ntoa(ip));

			memcpy(&ip, fwen.SourceIP, IP_ADDR_LEN);
			strcpy(source_ip, inet_ntoa(ip));

			memcpy(&ip, fwen.SourceMask, IP_ADDR_LEN);
			strcpy(source_mask, inet_ntoa(ip));

			memcpy(&ip, fwen.nextHop, IP_ADDR_LEN);
			strcpy(gateway_ip, inet_ntoa(ip));

			sprintf(enable, "%d", fwen.Enable);
			strcpy(status, enable? "Enabled": "Disabled");

			switch(fwen.Type)
			{
				case 0: strcpy(type, "Network");
					break;
				case 1: strcpy(type, "Host");
					break;
				case 2: strcpy(type, "Default");
					break;
			}

			sprintf(interface_idx, "%d", fwen.ifIndex);
			sprintf(fw_matric, "%d", fwen.FWMetric);
		}
	}
	else
	{
		syslog(LOG_ERR, "Invalid ForwardingIndex value!");
		SoapError(h, 402, "Invalid Args");
	}

	strcpy(resp, "<u:GetGenericForwardingEntryResponse xmlns:u=\"urn:dslforum-org:service:Layer3Forwarding:1\">");
	strcat(resp, "<NewDestIPAddress>");
	strcat(resp, dest_ip);
	strcat(resp, "</NewDestIPAddress>");
	strcat(resp, "<NewDestSubnetMask>");
	strcat(resp, dest_mask);
	strcat(resp, "</NewDestSubnetMask>");
 	strcat(resp, "<NewSourceIPAddress>");
 	strcat(resp, source_ip);
 	strcat(resp, "</NewSourceIPAddress>");
 	strcat(resp, "<NewSourceSubnetMask>");
 	strcat(resp, source_mask);
 	strcat(resp, "</NewSourceSubnetMask>");
 	strcat(resp, "<NewGatewayIPAddress>");
 	strcat(resp, gateway_ip);
 	strcat(resp, "</NewGatewayIPAddress>");
 	strcat(resp, "<NewEnable>");
 	strcat(resp, enable);
 	strcat(resp, "</NewEnable>");
 	strcat(resp, "<NewStatus>");
 	strcat(resp, status);
 	strcat(resp, "</NewStatus>");
 	strcat(resp, "<NewType>");
 	strcat(resp, type);
 	strcat(resp, "</NewType>");
 	strcat(resp, "<NewInterface>");
 	strcat(resp, interface_idx);
 	strcat(resp, "</NewInterface>");
 	strcat(resp, "<NewForwardingMetric>");
 	strcat(resp, fw_matric);
 	strcat(resp, "</NewForwardingMetric>");
	strcat(resp, "</u:GetGenericForwardingEntryResponse>");

	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}


static void LANConfigSecuritySetConfigPassword(struct upnphttp *h)
{
	struct NameValueParserData data;
	char *conf_pass;

	static const char resp[] =
	"<u:SetConfigPasswordResponse "
	"xmlns:u=\"urn:dslforum-org:service:LANConfigSecurity:1\">"
	"</u:SetConfigPasswordResponse>";

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	conf_pass = GetValueFromNameValueList(&data, "NewPassword");

#ifdef CONFIG_USER_CWMP_TR069
	if(conf_pass != NULL && strlen(conf_pass) <= 64)   // Max length = 64
	{
		if( !mib_set(CWMP_LAN_CONFIGPASSWD, conf_pass) )  // Refer from TR-069
			syslog(LOG_ERR, "UPNP ERR: Set ConfigPassword failed!");
	}
	else
	{
		syslog(LOG_ERR, "UPNP ERR: The length of ConfigPassword is too long or zero!");
		SoapError(h, 402, "Invalid Args");
	}
#else
	syslog(LOG_ERR, "UPNP ERR: TR-069 must enable!");
#endif

	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void ManagementServerGetInfo(struct upnphttp *h)
{
	char resp[1024], mibstr[256];

	strcpy(resp, "<u:GetInfoResponse xmlns:u=\"urn:dslforum-org:service:ManagementServer:1\">\n");
#ifdef CONFIG_USER_CWMP_TR069
	int mibint;
	char mibch;

	if(!mib_get(CWMP_ACS_URL, mibstr))  // Refered from TR-069
	{
		strcpy(mibstr, "Get URL failed!");
		syslog(LOG_ERR, "UPNP ERR: Get URL failed!");
		SoapError(h, 501, "Action Failed");
	}

#else
	strcpy(mibstr, "UPNP ERR: TR-069 must enable!");
	syslog(LOG_ERR, "UPNP ERR: TR-069 must enable!");
#endif

	strcat(resp, "<NewURL>");
	strcat(resp, mibstr);
	strcat(resp, "</NewURL>");

#ifdef CONFIG_USER_CWMP_TR069
	if(!mib_get(CWMP_INFORM_ENABLE, &mibch))  // Refered from TR-069
	{
		syslog(LOG_ERR, "Get PeriodicInformEnable failed!");
		SoapError(h, 501, "Action Failed");
	}
	else
		sprintf(mibstr, "%d", mibch);    // Convert byte to string.
#else
	strcpy(mibstr, "UPNP ERR: TR-069 must enable!");
	syslog(LOG_ERR, "UPNP ERR: TR-069 must enable!");
#endif

	strcat(resp, "<NewPeriodicInformEnable>");
	strcat(resp, mibstr);
	strcat(resp, "</NewPeriodicInformEnable>");

#ifdef CONFIG_USER_CWMP_TR069
	if(!mib_get(CWMP_INFORM_INTERVAL, &mibint))  // Refered from TR-069
	{
		syslog(LOG_ERR, "Get PeriodicInformInterval failed!");
		SoapError(h, 501, "Action Failed");
	}
	else
		sprintf(mibstr, "%d", mibint);
#else
	strcpy(mibstr, "UPNP ERR: TR-069 must enable!");
	syslog(LOG_ERR, "UPNP ERR: TR-069 must enable!");
#endif

	strcat(resp, "<NewPeriodicInformInterval>");
	strcat(resp, mibstr);
	strcat(resp, "</NewPeriodicInformInterval>");

#ifdef CONFIG_USER_CWMP_TR069
	if(!mib_get(CWMP_INFORM_TIME, &mibint))  // Refered from TR-069
	{
		syslog(LOG_ERR, "Get PeriodicInformTime failed!");
		SoapError(h, 501, "Action Failed");
	}
	else
		sprintf(mibstr, "%d", mibint);
#else
	strcpy(mibstr, "UPNP ERR: TR-069 must enable!");
	syslog(LOG_ERR, "UPNP ERR: TR-069 must enable!");
#endif

	strcat(resp, "<NewPeriodicInformTime>");
	strcat(resp, mibstr);
	strcat(resp, "</NewPeriodicInformTime>");
	strcat(resp, "<NewParameterKey>");
	strcat(resp, "");
	strcat(resp, "</NewParameterKey>");
	strcat(resp, "<NewParameterHash>");
	strcat(resp, "");
	strcat(resp, "</NewParameterHash>");
	strcat(resp, "<NewConnectionRequestURL>");
	strcat(resp, "");
	strcat(resp, "</NewConnectionRequestURL>");

#ifdef CONFIG_USER_CWMP_TR069
	if(!mib_get(CWMP_CONREQ_USERNAME, mibstr))  // Refered from TR-069
	{
		strcpy(mibstr, "Get ConnectionRequestUsername failed!");
		syslog(LOG_ERR, "Get ConnectionRequestUsername failed!");
		SoapError(h, 501, "Action Failed");
	}
#else
	strcpy(mibstr, "UPNP ERR: TR-069 must enable!");
	syslog(LOG_ERR, "UPNP ERR: TR-069 must enable!");
#endif

	strcat(resp, "<NewConnectionRequestUsername>");
	strcat(resp, mibstr);
	strcat(resp, "</NewConnectionRequestUsername>");

#ifdef CONFIG_USER_CWMP_TR069
	if(!mib_get(CWMP_ACS_UPGRADESMANAGED, &mibch))  // Refered from TR-069
	{
		syslog(LOG_ERR, "Get UpgradesManaged failed!");
		SoapError(h, 501, "Action Failed");
	}
	else
		sprintf(mibstr, "%d", mibch);   // Convert byte to string.
#else
	strcpy(mibstr, "UPNP ERR: TR-069 must enable!");
	syslog(LOG_ERR, "UPNP ERR: TR-069 must enable!");
#endif

	strcat(resp, "<NewUpgradesManaged>");
	strcat(resp, mibstr);
	strcat(resp, "</NewUpgradesManaged>");
	strcat(resp, "</u:GetInfoResponse>");

	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void ManagementServerSetManagementServerURL(struct upnphttp *h)
{
	struct NameValueParserData data;
	char *url, *cp1, *cp2;

	static const char resp[] =
	"<u:SetManagementServerURLResponse "
	"xmlns:u=\"urn:dslforum-org:service:ManagementServer:1\">"
	"</u:SetManagementServerURLResponse>";

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	url = GetValueFromNameValueList(&data, "NewURL");

	if( url != NULL )
	{
		cp1 = strstr(url, "http://");
		cp2 = strstr(url, "https://");
	}

#ifdef CONFIG_USER_CWMP_TR069
	if( url != NULL && (url == cp1 || url == cp2) )  // Verify if the input url valided or not.
	{
		if(!mib_set(CWMP_ACS_URL, url))  // Refered from TR-069
		{
			syslog(LOG_ERR, "Set URL failed!");
			SoapError(h, 501, "Action Failed");
		}
	}
	else
	{
		syslog(LOG_ERR, "UPNP ERR: Invalid url format!");
		SoapError(h, 402, "Invalid Args");
	}
#else
	syslog(LOG_ERR, "UPNP ERR: TR-069 must enable!");
#endif

	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void ManagementServerSetManagementServerPassword(struct upnphttp *h)
{
	struct NameValueParserData data;
	char *pswd;

	static const char resp[] =
	"<u:SetManagementServerPasswordResponse "
	"xmlns:u=\"urn:dslforum-org:service:ManagementServer:1\">"
	"</u:SetManagementServerPasswordResponse>";

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	pswd = GetValueFromNameValueList(&data, "NewPassword");

#ifdef CONFIG_USER_CWMP_TR069
	if( pswd != NULL && strlen(pswd) < 256 )  // Max length = 256.
	{
		if(!mib_set(CWMP_ACS_PASSWORD, pswd))  // Refered from TR-069
		{
			syslog(LOG_ERR, "Set password failed!");
			SoapError(h, 501, "Action Failed");
		}
	}
	else
	{
		syslog(LOG_ERR, "UPNP ERR: Password is too long!");
		SoapError(h, 402, "Invalid Args");
	}
#else
	syslog(LOG_ERR, "UPNP ERR: TR-069 must enable!");
#endif

	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void ManagementServerSetPeriodicInform(struct upnphttp *h)
{
	struct NameValueParserData data;

	static const char resp[] =
	"<u:SetPeriodicInformResponse "
	"xmlns:u=\"urn:dslforum-org:service:ManagementServer:1\">"
	"</u:SetPeriodicInformResponse>";

#ifdef CONFIG_USER_CWMP_TR069
	char *str, byte;
	int value;

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	str = GetValueFromNameValueList(&data, "NewPeriodicInformEnable");

	if( str && (str[0] == '1' || str[0] == '0') )
	{
		byte = (char)atoi(str);
		if(!mib_set(CWMP_INFORM_ENABLE, &byte))  // Refered from TR-069
		{
			syslog(LOG_ERR, "UPNP ERR: Set PeriodicInformEnable failed!");
			SoapError(h, 501, "Action Failed");
		}
	}
	else
	{
		syslog(LOG_ERR, "Invalid value!");
		SoapError(h, 402, "Invalid Args");
	}

	str = GetValueFromNameValueList(&data, "NewPeriodicInformInterval");

	if(str)
	{
		value = atoi(str);
		if(!mib_set(CWMP_INFORM_INTERVAL, &value))  // Refered from TR-069
		{
			syslog(LOG_ERR, "UPNP ERR: Set PeriodicInformEnable failed!");
			SoapError(h, 501, "Action Failed");
		}
	}
	else
	{
		syslog(LOG_ERR, "UPNP ERR: Invalid PeriodicInformEnable value!");
		SoapError(h, 402, "Invalid Args");
	}

	str = GetValueFromNameValueList(&data, "NewPeriodicInformTime");

	if(str)
	{
		value = atoi(str);
		if(!mib_set(CWMP_INFORM_TIME, &value))  // Refered from TR-069
		{
			syslog(LOG_ERR, "UPNP ERR: Set NewPeriodicInformTime failed!");
			SoapError(h, 501, "Action Failed");
		}
	}
#else
	syslog(LOG_ERR, "UPNP ERR: TR-069 must enable!");
#endif

	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void ManagementServerSetUpgradeManagement(struct upnphttp *h)
{
	struct NameValueParserData data;
	char *str;

	static const char resp[] =
	"<u:SetUpgradeManagementResponse "
	"xmlns:u=\"urn:dslforum-org:service:ManagementServer:1\">"
	"</u:SetUpgradeManagementResponse>";

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	str = GetValueFromNameValueList(&data, "NewUpgradesManaged");

#ifdef CONFIG_USER_CWMP_TR069
	char byte;

	if( str != NULL && (str[0] == '1' || str[0] == '0') )
	{
		byte = (char)atoi(str);
		if(!mib_set(CWMP_ACS_UPGRADESMANAGED, &byte))  // Refered from TR-069
		{
			syslog(LOG_ERR, "UPNP ERR: Set UpgradeManagement failed!");
			SoapError(h, 501, "Action Failed");
		}
	}
	else
	{
		syslog(LOG_ERR, "UPNP ERR: Invalid value!");
		SoapError(h, 402, "Invalid Args");
	}
#else
	syslog(LOG_ERR, "UPNP ERR: TR-069 must enable!");
#endif

	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void ManagementServerSetConnectionRequestAuthentication(struct upnphttp * h)
{
	struct NameValueParserData data;

	static const char resp[] =	"<u:SetConnectionRequestAuthenticationResponse xmlns:u=\"urn:dslforum-org:service:ManagementServer:1\"></u:SetConnectionRequestAuthenticationResponse>";

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

#ifdef CONFIG_USER_CWMP_TR069
	char *str;
	if((str = GetValueFromNameValueList(&data, "NewConnectionRequestUsername")) && strlen(str) < 256 )
	{
		if(!mib_set(CWMP_CONREQ_USERNAME, str))  // Refered from TR-069
		{
			syslog(LOG_ERR, "UPNP ERR: Set Connection Request Username failed!");
			SoapError(h, 501, "Action Failed");
		}
	}
	else
	{
		syslog(LOG_ERR, "UPNP ERR: Invalid ConnectionRequestUsername value!");
		SoapError(h, 402, "Invalid Args");
	}

	if((str = GetValueFromNameValueList(&data, "NewConnectionRequestPassword")) && strlen(str) < 256 )
	{
		if(!mib_set(CWMP_CONREQ_PASSWORD, str))  // Refered from TR-069
		{
			syslog(LOG_ERR, "UPNP ERR: Set Connection Request Password failed!");
			SoapError(h, 501, "Action Failed");
		}
	}
	else
	{
		syslog(LOG_ERR, "UPNP ERR: Invalid ConnectionRequestPassword value!");
		SoapError(h, 402, "Invalid Args");
	}
#endif

	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void TimeGetInfo(struct upnphttp * h)
{
	char resp[512], str[128], ltz[8];
	struct in_addr ip;
	time_t local_time;

	strcpy(resp, "<u:GetInfoResponse xmlns:u=\"urn:dslforum-org:service:Time:1\">\n");

	if(!mib_get(MIB_NTP_SERVER_IP1, (void *)&ip))
	{
		strcpy(str, "Get NTPServer1 failed!");
		syslog(LOG_ERR, "UPNP ERR: Get NTPServer1 failed!");
		SoapError(h, 501, "Action Failed");
	}
	else
		strcpy(str, inet_ntoa(ip));

	strcat(resp, "<NewNTPServer1>");
	strcat(resp, str);
	strcat(resp, "</NewNTPServer1>\n");

	if(!mib_get(MIB_NTP_SERVER_IP2, (void *)&ip))
	{
		strcpy(str, "Get NTPServer2 failed!");
		syslog(LOG_ERR, "UPNP ERR: Get NTPServer2 failed!");
		SoapError(h, 501, "Action Failed");
	}
	else
		strcpy(str, inet_ntoa(ip));

	strcat(resp, "<NewNTPServer2>");
	strcat(resp, str);
	strcat(resp, "</NewNTPServer2>\n");

	local_time = time(NULL);

	strcat(resp, "<NewCurrentLocalTime>");
	strcat(resp, ctime(&local_time));
	strcat(resp, "</NewCurrentLocalTime>\n");

	if(mib_get(MIB_NTP_TIMEZONE, str))
	{
		int i = 0;
		int t_shift, t_index;
		char *tok[2];

		tok[0] = strtok(str, " ");
		tok[1] = strtok(NULL, " ");
		t_shift = atoi(tok[0]);
		t_index = atoi(tok[1]);

		do
		{
			if( timeZoneTable[i].time_shift == t_shift && timeZoneTable[i].index == t_index )
			{
				if( t_shift == 0 )
					strcpy(ltz, "+00:00");
				else
				{
					strncpy(ltz, timeZoneTable[i].time_zone_name+4, 6);
					ltz[6] = 0;
				}

				strcpy(str, timeZoneTable[i].time_zone_name);

				break;
			}
		} while( timeZoneTable[++i].time_zone_name );

		if( !timeZoneTable[i].time_zone_name )  // No matches of time shift and index
		{
			strcpy(ltz, "Error!");
			strcpy(str, "Get TimeZone failed!");
			syslog(LOG_ERR, "UPNP ERR: Get TimeZone failed!");
		}
	}
	else
	{
		strcpy(ltz, "Error!");
		strcpy(str, "Get TimeZone failed!");
		syslog(LOG_ERR, "UPNP ERR: Get TimeZone failed!");
		SoapError(h, 501, "Action Failed");
	}

	strcat(resp, "<NewLocalTimeZone>");
	strcat(resp, ltz);
	strcat(resp, "</NewLocalTimeZone>\n");
	strcat(resp, "<NewLocalTimeZoneName>");
	strcat(resp, str);
	strcat(resp, "</NewLocalTimeZoneName>\n");

	sprintf(str, "%d", DaylightSavingsUsed);

	strcat(resp, "<NewDaylightSavingsUsed>");
	strcat(resp, str);
	strcat(resp, "</NewDaylightSavingsUsed>\n");
	strcat(resp, "<NewDaylightSavingsStart>");
	strcat(resp, DaylightSavingsStart);
	strcat(resp, "</NewDaylightSavingsStart>\n");
	strcat(resp, "<NewDaylightSavingsEnd>");
	strcat(resp, DaylightSavingsEnd);
	strcat(resp, "</NewDaylightSavingsEnd>\n");
	strcat(resp, "</u:GetInfoResponse>");

	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void TimeSetNTPServers(struct upnphttp *h)
{
	struct NameValueParserData data;
	char *str;
	struct in_addr inp;

	static const char resp[] =
		"<u:SetNTPServersResponse "
		"xmlns:u=\"urn:dslforum-org:service:Time:1\">"
		"</u:SetNTPServersResponse>";

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	str = GetValueFromNameValueList(&data, "NewNTPServer1");

	if(str == NULL || !inet_aton(str, &inp))
	{
		syslog(LOG_ERR, "UPNP ERR: Invalid ip address of NTPServer1!");
		SoapError(h, 402, "Invalid Args: Invalid ip address of NTPServer1!");
	}
	else if(!mib_set(MIB_NTP_SERVER_IP1, &inp))
	{
		syslog(LOG_ERR, "UPNP ERR: Set NTPServer1 failed!");
		SoapError(h, 501, "Action Failed: Set NTPServer1 failed!");
	}

	str = GetValueFromNameValueList(&data, "NewNTPServer2");

	if( str == NULL || !inet_aton(str, &inp))
	{
		syslog(LOG_ERR, "UPNP ERR: Invalid ip address of NTPServer2!");
		SoapError(h, 402, "Invalid Args: Invalid ip address of NTPServer2!");
	}
	else if(!mib_set(MIB_NTP_SERVER_IP2, &inp))
	{
		syslog(LOG_ERR, "UPNP ERR: Set NTPServer2 failed!");
		SoapError(h, 501, "Action Failed: Set NTPServer2 failed!");
	}

#ifdef APPLY_CHANGE
	stopNTP();
	startNTP();
#endif

	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void TimeSetLocalTimeZone(struct upnphttp *h)
{
	struct NameValueParserData data;
	char *ltz, *ltz_name, ntp_tzone[8];

	static const char resp[] =
		"<u:SetSetLocalTimeZoneResponse "
		"xmlns:u=\"urn:dslforum-org:service:Time:1\">"
		"</u:SetSetLocalTimeZoneResponse>";

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	ltz = GetValueFromNameValueList(&data, "NewLocalTimeZone");
	ltz_name = GetValueFromNameValueList(&data, "NewLocalTimeZoneName");

	int i = 0, tzi = -1;

	do
	{// Search LocalTimeZone first. If LocalTimeZone is not found, ignore it. And Set TimeZone value by LocalTimeZoneName.
		if( ltz && strstr(timeZoneTable[i].time_zone_name, ltz) )
			tzi = i;

		if(ltz_name && IsTimeZoneNameExist(timeZoneTable[i].time_zone_name, ltz_name))
			break;
	} while(timeZoneTable[++i].time_zone_name);

	if(timeZoneTable[i].time_zone_name) // Identify TimeZone value with LocalTimeZoneName first. If not found, use LocalTimeZone instead.
	{
		sprintf(ntp_tzone, "%+d %d", timeZoneTable[i].time_shift, timeZoneTable[i].index);

		if(!mib_set(MIB_NTP_TIMEZONE, ntp_tzone))
		{
			syslog(LOG_ERR, "UPNP ERR: Set LocalTimeZone failed!");
			SoapError(h, 501, "Action Failed: Set LocalTimeZone failed!");
		}
	}  // LocalTimeZoneName is not found but LocalTimeZone found.     // Check the format if is valid or not.
	else if( tzi != -1 && (ltz[0] == '+' || ltz[0] == '-' ) && isdigit(ltz[1]) && isdigit(ltz[2]) && ltz[3] == ':' && isdigit(ltz[4]) && isdigit(ltz[5]) )
	{
		sprintf(ntp_tzone, "%+d %d", timeZoneTable[tzi].time_shift, timeZoneTable[tzi].index);

		if(!mib_set(MIB_NTP_TIMEZONE, ntp_tzone))
		{
			syslog(LOG_ERR, "UPNP ERR: Set LocalTimeZone failed!");
			SoapError(h, 501, "Action Failed: Set LocalTimeZone failed!");
		}
	}
	else if(ltz && strstr(ltz, "00:00")) // GMT +00:00, defalut to London area.
	{
		strcpy(ntp_tzone, "+0 2");

		if(!mib_set(MIB_NTP_TIMEZONE, ntp_tzone))
		{
			syslog(LOG_ERR, "UPNP ERR: Set LocalTimeZone failed!");
			SoapError(h, 501, "Action Failed: Set LocalTimeZone failed!");
		}
	}
	else
	{
		syslog(LOG_ERR, "UPNP ERR: Invalid LocalTimeZone value!");
		SoapError(h, 402, "Invalid Args: Invalid LocalTimeZone value!");
	}


	char *str;

	if((str = GetValueFromNameValueList(&data, "NewDaylightSavingsUsed")))
		DaylightSavingsUsed = (bool)atoi(str);
	else
	{
		syslog(LOG_ERR, "UPNP ERR: Invalid DaylightSavingsUsed value!");
		SoapError(h, 402, "Invalid Args: Invalid DaylightSavingsUsed value!");
	}

	if( (str = GetValueFromNameValueList(&data, "NewDaylightSavingsStart")) && strlen(str) < DATETIME_LENGTH )
		{
			if(IsValidDateTimeValue(str))
				strcpy(DaylightSavingsStart, str);
		}
	else
	{
		syslog(LOG_ERR, "UPNP ERR: Invalid DaylightSavingsStart value!");
		SoapError(h, 402, "Invalid Args: Invalid DaylightSavingsStart value!");
	}

	if( (str = GetValueFromNameValueList(&data, "NewDaylightSavingsEnd")) && strlen(str) < DATETIME_LENGTH )
	{
		if(IsValidDateTimeValue(str))
			strcpy(DaylightSavingsEnd, str);
	}
	else
	{
		syslog(LOG_ERR, "UPNP ERR: Invalid DaylightSavingsEnd value!");
		SoapError(h, 402, "Invalid Args: Invalid DaylightSavingsEnd value!");
	}

#ifdef APPLY_CHANGE
	stopNTP();
	startNTP();
#endif

	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void LANHCMGetInfo(struct upnphttp * h)
{
	struct in_addr inIP, inMask;
	unsigned char strLeaseTime[7], gateway[16], subnetmask[16], dns[4][16], dnsAll[52], strDomain[65], strLanIP[16], strLanStartIP[16], strStart[5], strLanEndIP[16], strEnd[5];
	int bodylen, i;
	char * p;
	unsigned char buffer[64], DhcpS, DhcpC;
	char body[2048];

	// Get SubnetMask
	if(!mib_get(MIB_ADSL_LAN_SUBNET,  (void *)&inMask))
		return;
	sprintf(subnetmask, "%s", inet_ntoa(inMask));

	// Get DNSServers
	dnsAll[0] = '\0';
	for ( i=MIB_ADSL_WAN_DNS1; i<=MIB_ADSL_WAN_DNS3; i++)
	{
		if(!mib_get( i,  (void *)&inIP))
			return;
		if ( inIP.s_addr == INADDR_NONE )
		{
			sprintf(dns[i], "%s", "");
			strcat(dnsAll, dns[i]);
		} else {
			//if ( i != MIB_ADSL_WAN_DNS3 ){
				sprintf(dns[i], "%s,", inet_ntoa(inIP));
				strcat(	dnsAll, dns[i]);
			//}
		}
	}
	dnsAll[strlen(dnsAll)-1] = '\0';

	// Get Domain Name
	if ( !mib_get(MIB_ADSL_LAN_DHCP_DOMAIN, (void *)strDomain))
		return;

	// Get LANIP
	if ( !mib_get(MIB_ADSL_LAN_IP, (void *)&inIP))
		return;
	sprintf(strLanIP, "%s", inet_ntoa(inIP));
	p = strrchr(strLanIP, '.');
	*(++p) = '\0';

	// Get startIP for DHCP IP Pool
	if ( !mib_get(MIB_ADSL_LAN_CLIENT_START, (unsigned char *)buffer))
		return;
	sprintf(strStart, "%u", *(unsigned char *)buffer);

	// Generate LANStartIP
	strLanStartIP[0] = '\0';
	strcat(strLanStartIP, strLanIP);
	strcat(strLanStartIP, strStart);

	// Get EndIP for DHCP IP Pool
	if ( !mib_get(MIB_ADSL_LAN_CLIENT_END, (unsigned char *)buffer))
		return;
	sprintf(strEnd, "%u", *(unsigned char *)buffer);

	// Generate LANEndIP
	strLanEndIP[0] = '\0';
	strcat(strLanEndIP, strLanIP);
	strcat(strLanEndIP, strEnd);

   	// Get Gateway Address
	if(!mib_get( MIB_ADSL_LAN_DHCP_GATEWAY,  (void *)&inIP))
		return;
	sprintf(gateway, "%s", inet_ntoa(inIP));

	// Get DHCP LeaseTime
	if(!mib_get( MIB_ADSL_LAN_DHCP_LEASE,  (void *)buffer))
		return;
	// if MIB_ADSL_LAN_DHCP_LEASE=0xffffffff, it indicate an infinate lease
	if ( *(unsigned long *)buffer == 0xffffffff )
		sprintf(strLeaseTime, "-1");
	else
		sprintf(strLeaseTime, "%u", *(unsigned int *)buffer);

	// Get DHCPMode
	if(!mib_get( MIB_DHCP_MODE, (void *)buffer) )
		return;

	if ( (DHCP_TYPE_T)buffer[0] == DHCP_LAN_NONE ) {
		DhcpS = '0';
		DhcpC = '0';
	} else if ( (DHCP_TYPE_T)buffer[0] == DHCP_LAN_RELAY ) {
		DhcpS = '0';
		DhcpC = '1';
	} else {
		DhcpS = '1';
		DhcpC = '0';
	}

	static const char resp[] =
		"<u:GetInfoResponse "
		"xmlns:u=\"urn:dslforum-org:service:LANHostConfigManagement:1\">"
		"<NewDHCPServerConfigurable>1</NewDHCPServerConfigurable>"
		"<NewDHCPRelay>%c</NewDHCPRelay>"
		"<NewSubnetMask>%s</NewSubnetMask>"
		"<NewDNSServers>%s</NewDNSServers>"
		"<NewDomainName>%s</NewDomainName>"
		"<NewMinAddress>%s</NewMinAddress>"
		"<NewMaxAddress>%s</NewMaxAddress>"
		"<NewIPRouters>%s</NewIPRouters>"
		"<NewDHCPLeaseTime>%s</NewDHCPLeaseTime>"
		"<NewDHCPServerEnable>%c</NewDHCPServerEnable>"
		"<NewUseAllocatedWAN>Normal</NewUseAllocatedWAN>"
		"</u:GetInfoResponse>";
	bodylen = snprintf(body, sizeof(body), resp, DhcpC, subnetmask, dnsAll, strDomain, strLanStartIP, strLanEndIP, gateway, strLeaseTime, DhcpS);
	BuildSendAndCloseSoapResp(h, body, bodylen);
	//BuildSendAndCloseSoapResp(h, resp, sizeof(resp)-1);
}

static void LANHCMGetDHCPRelay(struct upnphttp * h)
{
	int bodylen;
	unsigned char buffer[64], DhcpC;
	char body[2048];

	// Get DHCPMode
	if(!mib_get( MIB_DHCP_MODE, (void *)buffer) )
		return;

	if ( (DHCP_TYPE_T)buffer[0] == DHCP_LAN_NONE ) {
		DhcpC = '0';
	} else if ( (DHCP_TYPE_T)buffer[0] == DHCP_LAN_RELAY ) {
		DhcpC = '1';
	} else {
		DhcpC = '0';
	}

	static const char resp[] =
		"<u:GetInfoResponse "
		"xmlns:u=\"urn:dslforum-org:service:LANHostConfigManagement:1\">"
		"<NewDHCPRelay>%c</NewDHCPRelay>"
		"</u:GetInfoResponse>";
	bodylen = snprintf(body, sizeof(body), resp, DhcpC);
	BuildSendAndCloseSoapResp(h, body, bodylen);
}

static void LANHCMSetSubnetMask(struct upnphttp * h)
{
	static const char resp[] =
		"<u:SetSubnetMaskResponse "
		"xmlns:u=\"urn:dslforum-org:service:LANHostConfigManagement:1\">"
		"</u:SetSubnetMaskResponse>";
	struct NameValueParserData data;
	char * int_str_ip;
	struct in_addr inIP;

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	int_str_ip = GetValueFromNameValueList(&data, "NewSubnetMask");

	if(int_str_ip)
	{
		if ( !inet_aton(int_str_ip, &inIP) )
			return;

		if ( !mib_set( MIB_ADSL_LAN_SUBNET, (void *)&inIP))
			return;
	}

	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, sizeof(resp)-1);
}

static void LANHCMGetSubnetMask(struct upnphttp * h)
{
	struct in_addr inMask;
	unsigned char subnetMask[16];

	int bodylen;
	char body[2048];

	// Get SubnetMask
	if(!mib_get( MIB_ADSL_LAN_SUBNET,  (void *)&inMask))
		return;
	sprintf(subnetMask, "%s", inet_ntoa(inMask));

	static const char resp[] =
		"<u:GetSubnetMaskResponse "
		"xmlns:u=\"urn:dslforum-org:service:LANHostConfigManagement:1\">"
		"<NewSubnetMask>%s</NewSubnetMask>"
		"</u:GetSubnetMaskResponse>";
	bodylen = snprintf(body, sizeof(body), resp, subnetMask);
	BuildSendAndCloseSoapResp(h, body, bodylen);
}

static void LANHCMSetIPRouter(struct upnphttp * h)
{
	static const char resp[] =
		"<u:SetIPRouterResponse "
		"xmlns:u=\"urn:dslforum-org:service:LANHostConfigManagement:1\">"
		"</u:SetIPRouterResponse>";
	struct NameValueParserData data;
	char * int_str_ip;
	struct in_addr inIP;

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	int_str_ip = GetValueFromNameValueList(&data, "NewIPRouters");

	if ( int_str_ip ) {
		if ( !inet_aton(int_str_ip, &inIP) ) {
			return;
		}

		if ( !mib_set( MIB_ADSL_LAN_DHCP_GATEWAY, (void *)&inIP)) {
			return;
		}
	}

	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, sizeof(resp)-1);
}

static void LANHCMGetIPRoutersList(struct upnphttp * h)
{
	struct in_addr inIP;
	unsigned char gateway[16];

	int bodylen;
	char body[2048];

	// GetIPRoutersList
	if(!mib_get( MIB_ADSL_LAN_DHCP_GATEWAY,  (void *)&inIP))
		return;
	sprintf(gateway, "%s", inet_ntoa(inIP));

	static const char resp[] =
		"<u:GetIPRoutersListResponse "
		"xmlns:u=\"urn:dslforum-org:service:LANHostConfigManagement:1\">"
		"<NewIPRouters>%s</NewIPRouters>"
		"</u:GetIPRoutersListResponse>";

	bodylen = snprintf(body, sizeof(body), resp, gateway);
	BuildSendAndCloseSoapResp(h, body, bodylen);
}

static void LANHCMSetDomainName(struct upnphttp * h)
{
	static const char resp[] =
		"<u:SetDomainNameResponse "
		"xmlns:u=\"urn:dslforum-org:service:LANHostConfigManagement:1\">"
		"</u:SetDomainNameResponse>";
	struct NameValueParserData data;
	unsigned char * int_str;

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	int_str = GetValueFromNameValueList(&data, "NewDomainName");

	if ( int_str != NULL ) {
		if ( !mib_set( MIB_ADSL_LAN_DHCP_DOMAIN, (void *)int_str)) {
			return;
		}
	}

	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, sizeof(resp)-1);
}

static void LANHCMGetDomainName(struct upnphttp * h)
{
	unsigned char strDomain[65];

	int bodylen;
	char body[2048];

	// GetDomainName
	if(!mib_get( MIB_ADSL_LAN_DHCP_DOMAIN,  (void *)strDomain))
		return;

	static const char resp[] =
		"<u:GetDomainNameResponse "
		"xmlns:u=\"urn:dslforum-org:service:LANHostConfigManagement:1\">"
		"<NewDomainName>%s</NewDomainName>"
		"</u:GetDomainNameResponse>";
	bodylen = snprintf(body, sizeof(body), resp, strDomain);
	BuildSendAndCloseSoapResp(h, body, bodylen);
}

static void LANHCMGetAddressRange(struct upnphttp * h)
{
	unsigned char strLanIP[16], strLanStartIP[16], strStart[5], strLanEndIP[16], strEnd[5];
	char * p;
	unsigned char buffer[64];
	struct in_addr inIP;
	int bodylen;
	char body[2048];

	// Get LANIP
	if ( !mib_get(MIB_ADSL_LAN_IP, (void *)&inIP))
		return;
	sprintf(strLanIP, "%s", inet_ntoa(inIP));
	p = strrchr(strLanIP, '.');
	*(++p) = '\0';

	// Get startIP for DHCP IP Pool
	if ( !mib_get(MIB_ADSL_LAN_CLIENT_START, (unsigned char *)buffer))
		return;
	sprintf(strStart, "%u", *(unsigned char *)buffer);

	// Generate LANStartIP
	strLanStartIP[0] = '\0';
	strcat(strLanStartIP, strLanIP);
	strcat(strLanStartIP, strStart);

	// Get EndIP for DHCP IP Pool
	if ( !mib_get(MIB_ADSL_LAN_CLIENT_END, (unsigned char *)buffer))
		return;
	sprintf(strEnd, "%u", *(unsigned char *)buffer);

	// Generate LANEndIP
	strLanEndIP[0] = '\0';
	strcat(strLanEndIP, strLanIP);
	strcat(strLanEndIP, strEnd);

	static const char resp[] =
		"<u:GetAddressRangeResponse "
		"xmlns:u=\"urn:dslforum-org:service:LANHostConfigManagement:1\">"
		"<NewMinAddress>%s</NewMinAddress>"
		"<NewMaxAddress>%s</NewMaxAddress>"
		"</u:GetAddressRangeResponse>";
	bodylen = snprintf(body, sizeof(body), resp, strLanStartIP, strLanEndIP);
	BuildSendAndCloseSoapResp(h, body, bodylen);
}

static void LANHCMGetDNSServers(struct upnphttp * h)
{
	unsigned char dns[4][16], dnsAll[52];
	struct in_addr inIP;
	int bodylen, i;
	char body[2048];

	// Get DNSServers
	dnsAll[0] = '\0';
	for ( i=MIB_ADSL_WAN_DNS1; i<=MIB_ADSL_WAN_DNS3; i++) {
		if(!mib_get( i,  (void *)&inIP))
			return;
		if ( inIP.s_addr == INADDR_NONE ) {
			sprintf(dns[i], "%s", "");
			strcat(dnsAll, dns[i]);
		} else {
			//if ( i != MIB_ADSL_WAN_DNS3 ){
				sprintf(dns[i], "%s,", inet_ntoa(inIP));
				strcat(	dnsAll, dns[i]);
			//}
		}
	}
	dnsAll[strlen(dnsAll)-1] = '\0';

	static const char resp[] =
		"<u:GetDNSServersResponse "
		"xmlns:u=\"urn:dslforum-org:service:LANHostConfigManagement:1\">"
		"<NewDNSServers>%s</NewDNSServers>"
		"</u:GetDNSServersResponse>";
	bodylen = snprintf(body, sizeof(body), resp, dnsAll);
	BuildSendAndCloseSoapResp(h, body, bodylen);
}

static void LANHCMSetDNSServer(struct upnphttp * h)
{
	static const char resp[] =
		"<u:SetDNSServerResponse "
		"xmlns:u=\"urn:dslforum-org:service:LANHostConfigManagement:1\">"
		"</u:SetDNSServerResponse>";
	struct NameValueParserData data;
	unsigned char *int_str, *str_start, dns[4][16];
	char * p;
	int i;
	int dns_count = 0;
	struct in_addr u4_dns[4];

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	int_str = GetValueFromNameValueList(&data, "NewDNSServers");

	if ( int_str != NULL ) {
		str_start = int_str;
		while ( ( p = strchr(str_start, ',') ) != NULL ) {
			*p = '\0';
			strcpy(dns[dns_count], str_start);

			dns_count++;
			str_start = ++p;
		}

		strcpy(dns[dns_count], str_start);
		dns_count++;
	}

	for ( i=0; i<dns_count && i<3 ; i++) {
		if ( !inet_aton(dns[i], &u4_dns[i]) ) {
			return;
		}
		if ( !mib_set(MIB_ADSL_WAN_DNS1+i, (void *)&u4_dns[i])) {
	  		return;
		}
	}

	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, sizeof(resp)-1);
}

static void LANHCMDeleteDNSServer(struct upnphttp * h)
{
	static const char resp[] =
		"<u:DeleteDNSServerResponse "
		"xmlns:u=\"urn:dslforum-org:service:LANHostConfigManagement:1\">"
		"</u:DeleteDNSServerResponse>";
	struct NameValueParserData data;
	unsigned char *int_str, *str_start, dns[4][16];
	char * p;
	int i, j;
	int dns_count = 0;
	struct in_addr u4_dns[4], old_u4_dns[4];
	//FILE *f;

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	int_str = GetValueFromNameValueList(&data, "NewDNSServers");

	if ( int_str != NULL ) {
		str_start = int_str;
		while ( ( p = strchr(str_start, ',') ) != NULL ) {
			*p = '\0';
			strcpy(dns[dns_count], str_start);

			dns_count++;
			str_start = ++p;
		}

		strcpy(dns[dns_count], str_start);
		dns_count++;

	}

	//f=fopen("/var/run/upnp.log","a");
	//fprintf(f,"dns_count=%d\n", dns_count);
   	//fclose(f);

	for ( i=0; i<dns_count && i<3 ; i++) {
		//f=fopen("/var/run/upnp.log","a");
   		//fprintf(f,"dns[%d]=%s\n", i, dns[i]);
   		//fclose(f);

		if ( !inet_aton(dns[i], &u4_dns[i]) ) {
			return;
		}

		for ( j=0; j<3 ; j++ ) {
			if ( !mib_get(MIB_ADSL_WAN_DNS1+j, (void *)&old_u4_dns[j])) {
	  			return;
			}

			if( old_u4_dns[j].s_addr == u4_dns[i].s_addr ) {
				old_u4_dns[j].s_addr = INADDR_NONE;
				if ( !mib_set(MIB_ADSL_WAN_DNS1+j, (void *)&old_u4_dns[j])) {
	  				return;
				}
			}
		}
	}

	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, sizeof(resp)-1);
}

static void LANHCMSetDHCPServerEnable(struct upnphttp * h)
{
	static const char resp[] =
		"<u:SetDHCPServerEnableResponse "
		"xmlns:u=\"urn:dslforum-org:service:LANHostConfigManagement:1\">"
		"</u:SetDHCPServerEnableResponse>";
	struct NameValueParserData data;
	unsigned char * int_str, vChar;
	DHCP_TYPE_T dtmode;

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	int_str = GetValueFromNameValueList(&data, "NewDHCPServerEnable");

	if ( int_str != NULL ) {
		if ( int_str[0] == '1' ) {
			dtmode = DHCP_LAN_SERVER;
		} else {
			dtmode = DHCP_LAN_NONE;
		}

		vChar = (unsigned char) dtmode;
		if ( !mib_set( MIB_DHCP_MODE, (void *)&vChar)) {
			return;
		}
	}

	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, sizeof(resp)-1);
}

static void LANHCMSetDHCPLeaseTime(struct upnphttp * h)
{
	static const char resp[] =
		"<u:SetDHCPLeaseTimeResponse "
		"xmlns:u=\"urn:dslforum-org:service:LANHostConfigManagement:1\">"
		"</u:SetDHCPLeaseTimeResponse>";
	struct NameValueParserData data;
	unsigned char * int_str;
	unsigned int uLTime;

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	int_str = GetValueFromNameValueList(&data, "NewDHCPLeaseTime");

	if ( int_str != NULL ) {
		sscanf(int_str, "%u", &uLTime);

		if ( !mib_set(MIB_ADSL_LAN_DHCP_LEASE, (void *)&uLTime)) {
			return;
		}
	}

	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, sizeof(resp)-1);
}

static void WANCommonInterfaceConfigGetTotalBytesSent(struct upnphttp *h)
{
	char resp[256], str[32];
	struct ifdata data;
	int r;

	r = getifstats(ext_if_name, &data);
	sprintf(str, "%lu", r < 0 ? 0: data.obytes);

	strcpy(resp, "<u:GetTotalBytesSentResponse xmlns:u=\"urn:dslforum-org:service:WANCommonInterfaceConfig:1\">");
	strcat(resp, "<NewTotalBytesSent>");
	strcat(resp, str);
	strcat(resp, "</NewTotalBytesSent>");
	strcat(resp, "</u:GetTotalBytesSentResponse>");

	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANCommonInterfaceConfigGetTotalBytesReceived(struct upnphttp *h)
{
	char resp[256], str[32];
	struct ifdata data;
	int r;

	r = getifstats(ext_if_name, &data);
	sprintf(str, "%lu", r < 0 ? 0: data.ibytes);

	strcpy(resp, "<u:GetTotalBytesReceivedResponse xmlns:u=\"urn:dslforum-org:service:WANCommonInterfaceConfig:1\">");
	strcat(resp, "<NewTotalBytesReceived>");
	strcat(resp, str);
	strcat(resp, "</NewTotalBytesReceived>");
	strcat(resp, "</u:GetTotalBytesReceivedResponse>");

	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

 static void WANCommonInterfaceConfigGetTotalPacketsSent(struct upnphttp *h)
 {
  char resp[256], str[32];
  struct ifdata data;
  int r;

  r = getifstats(ext_if_name, &data);
  sprintf(str, "%lu", r < 0 ? 0: data.opackets);

  strcpy(resp, "<u:GetTotalPacketsSentResponse xmlns:u=\"urn:dslforum-org:service:WANCommonInterfaceConfig:1\">");
  strcat(resp, "<NewTotalPacketsSent>");
  strcat(resp, str);
  strcat(resp, "</NewTotalPacketsSent>");
  strcat(resp, "</u:GetTotalPacketsSentResponse>");

  BuildSendAndCloseSoapResp(h, resp, strlen(resp));
 }

static void WANCommonInterfaceConfigGetTotalPacketsReceived(struct upnphttp * h)
{
 char resp[256], str[32];
 struct ifdata data;
 int r;

 r = getifstats(ext_if_name, &data);
 sprintf(str, "%lu", r < 0 ? 0: data.ipackets);

 strcpy(resp, "<u:GetTotalPacketsReceivedResponse xmlns:u=\"urn:dslforum-org:service:WANCommonInterfaceConfig:1\">");
 strcat(resp, "<NewTotalPacketsReceived>");
 strcat(resp, str);
 strcat(resp, "</NewTotalPacketsReceived>");
 strcat(resp, "</u:GetTotalPacketsReceivedResponse>");

 BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANCommonInterfaceConfigGetCommonLinkProperties(struct upnphttp *h)
{
	char resp[512], wan_acc_type[] = "Cable", phy_link_st[] = "Up", up_max[24], down_max[24];
	int r;
	struct ifdata data;

	if((downstream_bitrate == 0) || (upstream_bitrate == 0))
	{
		r = getifstats(ext_if_name, &data);
		if(r>=0)
		{
			if(downstream_bitrate == 0)
				downstream_bitrate = data.baudrate;
			if(upstream_bitrate == 0)
				upstream_bitrate = data.baudrate;
		}
	}

	sprintf(up_max, "%lu", upstream_bitrate);
	sprintf(down_max, "%lu", downstream_bitrate);

	strcpy(resp, "<u:GetCommonLinkPropertiesResponse xmlns:u=\"urn:dslforum-org:service:WANCommonInterfaceConfig:1\">");
	strcat(resp, "<NewWANAccessType>");
	strcat(resp, wan_acc_type);
	strcat(resp, "</NewWANAccessType>");
	strcat(resp, "<NewLayer1UpstreamMaxBitRate>");
	strcat(resp, up_max);
	strcat(resp, "</NewLayer1UpstreamMaxBitRate>");
	strcat(resp, "<NewLayer1DownstreamMaxBitRate>");
	strcat(resp, down_max);
	strcat(resp, "</NewLayer1DownstreamMaxBitRate>");
	strcat(resp, "<NewPhysicalLinkStatus>");
	strcat(resp, phy_link_st);
	strcat(resp, "</NewPhysicalLinkStatus>");
	strcat(resp, "</u:GetCommonLinkPropertiesResponse>");

	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANCommonInterfaceConfigSetEnabledForInternet(struct upnphttp *h)
{
	struct NameValueParserData data;
	char *enabled;

	static const char resp[] = "<u:SetEnabledForInternetResponse xmlns:u=\"urn:dslforum-org:service:WANCommonInterfaceConfig:1\"></u:SetEnabledForInternetResponse>";
	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	if((enabled = GetValueFromNameValueList(&data, "NewEnabledForInternet")))
		EnabledForInternet = atoi(enabled)? true: false;

	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANCommonInterfaceConfigGetEnabledForInternet(struct upnphttp *h)
{
	char resp[256];

	strcpy(resp, "<u:GetEnabledForInternetResponse xmlns:u=\"urn:dslforum-org:service:WANCommonInterfaceConfig:1\">");
	strcat(resp, "<NewEnabledForInternet>");
	strcat(resp, EnabledForInternet? "1": "0");
	strcat(resp, "</NewEnabledForInternet>");
	strcat(resp, "</u:GetEnabledForInternetResponse>");

	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANDSLInterfaceConfigSetEnable(struct upnphttp *h)
{
	struct NameValueParserData data;
	char *enabled;

	static const char resp[] = "<u:SetEnableResponse xmlns:u=\"urn:dslforum-org:service:WANDSLInterfaceConfig:1\"></u:SetEnableResponse>";
	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	if(!(enabled = GetValueFromNameValueList(&data, "NewEnable")))
		SoapError(h, 402, "Invalid Args");

	if( strlen(enabled) > 1 || (enabled[0] != '0' && enabled[0] != '1') )
		SoapError(h, 402, "Invalid Args");

	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANDSLInterfaceConfigGetInfo(struct upnphttp *h)
{
	char resp[2048], str[256];
	double dval;
	Modem_Identification vMId;

	strcpy(resp, "<u:GetInfoResponse xmlns:u=\"urn:dslforum-org:service:WANDSLInterfaceConfig:1\">");
	strcat(resp, "<NewEnable>");
	strcat(resp, "1");
	strcat(resp, "</NewEnable>");

	if( getAdslInfo(ADSL_GET_STATE, str, 256) > 0 )
	{
		if(!strncmp(str, "HANDSHAKING", 11))
			strcpy(str, "Initializing");
		else if(!strncmp(str, "SHOWTIME", 8))
			strcpy(str, "Up");
		else if( !strncmp(str, "ACTIVATING", 10) || !strncmp(str, "IDLE", 4) )
			strcpy(str, "EstablishingLink");
		else
			strcpy(str, "NoSignal"); //or Error, Disabled
	}
	else
		strcpy(str, "Get Status failed!");

	strcat(resp, "<NewStatus>");
	strcat(resp, str);
	strcat(resp, "</NewStatus>");

	if( getAdslInfo(ADSL_GET_MODE, str, 256) > 0 )
	{
		if(!strncmp(str, "T1.413", 6))
			strcpy(str, "ADSL_ANSI_T1.413");
		else if(!strncmp(str, "G.dmt", 5))
			strcpy(str, "ADSL_G.dmt");
		else if(!strncmp(str, "G.Lite", 6))
			strcpy(str, "ADSL_G.lite");
		else if(!strncmp(str, "ADSL2", 5))
			strcpy(str, "ADSL_G.dmt.bis");
		else if(!strncmp(str, "ADSL2+", 6))
			strcpy(str, "ADSL_2plus");
		else
			strcpy(str, "Get ModulationType failed!");
	}
	else
		strcpy(str, "Get ModulationType failed!");

	strcat(resp, "<NewModulationType>");
	strcat(resp, str);
	strcat(resp, "</NewModulationType>");
	strcat(resp, "<NewLineEncoding>");
	strcat(resp, "DMT");
	strcat(resp, "</NewLineEncoding>");
	strcat(resp, "<NewLineNumber>");
	strcat(resp, "0");
	strcat(resp, "</NewLineNumber>");

	if( getAdslInfo(ADSL_GET_RATE_US, str, 256) <= 0)
		strcpy(str, "Action Failed: Get UpstreamCurrRate failed!");

	strcat(resp, "<NewUpstreamCurrRate>");
	strcat(resp, str);
	strcat(resp, "</NewUpstreamCurrRate>");

	if( getAdslInfo(ADSL_GET_RATE_DS, str, 256) <= 0 )
		strcpy(str, "Action Failed: Get DownstreamCurrRate failed!");

	strcat(resp, "<NewDownstreamCurrRate>");
	strcat(resp, str);
	strcat(resp, "</NewDownstreamCurrRate>");

	if( getAdslInfo(ADSL_GET_ATTRATE_US, str, 256) <= 0 )
		strcpy(str, "Action Failed: Get UpstreamMaxRate failed!");

	strcat(resp, "<NewUpstreamMaxRate>");
	strcat(resp, str);
	strcat(resp, "</NewUpstreamMaxRate>");

	if( getAdslInfo(ADSL_GET_ATTRATE_DS, str, 256) <= 0 )
		strcpy(str, "Action Failed: Get DownstreamMaxRate failed!");

	strcat(resp, "<NewDownstreamMaxRate>");
	strcat(resp, str);
	strcat(resp, "</NewDownstreamMaxRate>");

	if( getAdslInfo(ADSL_GET_SNR_US, str, 256) > 0 )
	{
		dval = atof(str);
		sprintf(str, "%f", dval);
	}
	else
		strcpy(str, "Action Failed: Get UpstreamNoiseMargin failed!");

	strcat(resp, "<NewUpstreamNoiseMargin>");
	strcat(resp, str);
	strcat(resp, "</NewUpstreamNoiseMargin>");


	if( getAdslInfo(ADSL_GET_SNR_DS, str, 256) > 0 )
	{
		dval = atof(str);
		sprintf(str, "%f", dval);
	}
	else
		strcpy(str, "Action Failed: Get DownstreamNoiseMargin failed!");

	strcat(resp, "<NewDownstreamNoiseMargin>");
	strcat(resp, str);
	strcat(resp, "</NewDownstreamNoiseMargin>");


	if( getAdslInfo(ADSL_GET_LPATT_US, str, 256) > 0 )
	{
		dval = atof(str);
		sprintf(str, "%f", dval);
	}
	else
		strcpy(str, "Action Failed: Get UpstreamAttenuation failed!");

	strcat(resp, "<NewUpstreamAttenuation>");
	strcat(resp, str);
	strcat(resp, "</NewUpstreamAttenuation>");

	if( getAdslInfo(ADSL_GET_LPATT_DS, str, 256) > 0 )
	{
		dval = atof(str);
		sprintf(str, "%f", dval);
	}
	else
		strcpy(str, "Action Failed: Get DownstreamAttenuation failed!");

	strcat(resp, "<NewDownstreamAttenuation>");
	strcat(resp, str);
	strcat(resp, "</NewDownstreamAttenuation>");

	if( getAdslInfo(ADSL_GET_POWER_US, str, 256) > 0 )
	{
		dval = atof(str);
		sprintf(str, "%f", dval);
	}
	else
   strcpy(str, "Action Failed: Get UpstreamPower failed!");

	strcat(resp, "<NewUpstreamPower>");
	strcat(resp, str);
	strcat(resp, "</NewUpstreamPower>");

	if( getAdslInfo(ADSL_GET_POWER_DS, str, 256) > 0 )
	{
		dval = atof(str);
		sprintf(str, "%f", dval);
	}
	else
   strcpy(str, "Action Failed: Get DownstreamPower failed!");

	strcat(resp, "<NewDownstreamPower>");
	strcat(resp, str);
	strcat(resp, "</NewDownstreamPower>");

	if( getAdslInfo(ADSL_GET_LATENCY, str, 256) > 0 )
	{
		if(!strncmp(str, "Fast", 4))
			strcpy(str, "Fast");
		else if(!strncmp(str, "Interleave", 10))
			strcpy(str, "Interleaved");
		else
			strcpy(str, "Get DataPath failed!");
	}
	else
		strcpy(str, "Get DataPath failed!");

	strcat(resp, "<NewDataPath>");
	strcat(resp, str);
	strcat(resp, "</NewDataPath>");

	if( getAdslInfo(ADSL_GET_D_DS, str, 256) <= 0 )
		strcpy(str, "Get InterleavedDepth failed!");

	strcat(resp, "<NewInterleavedDepth>");
	strcat(resp, str);
	strcat(resp, "</NewInterleavedDepth>");

	if(adsl_drv_get(RLCM_MODEM_NEAR_END_ID_REQ, (void *)&vMId, RLCM_MODEM_ID_REQ_SIZE))
	{
		snprintf(str, 256, "%c%c%c%c",
    (char)((vMId.ITU_VendorId.vendorCode>>24) &0xff),
    (char)((vMId.ITU_VendorId.vendorCode>>16) &0xff),
    (char)((vMId.ITU_VendorId.vendorCode>>8) &0xff),
    (char)((vMId.ITU_VendorId.vendorCode) &0xff));
	}
	else
		strcpy(str, "Action Failed: Get ATURVendor failed!");

	strcat(resp, "<NewATURVendor>");
	strcat(resp, str);
	strcat(resp, "</NewATURVendor>");

	if(adsl_drv_get(RLCM_MODEM_NEAR_END_ID_REQ, (void *)&vMId, RLCM_MODEM_ID_REQ_SIZE))
		sprintf(str, "%d", vMId.ITU_VendorId.countryCode);
	else
		strcpy(str, "Action Failed: Get ATURCountry failed!");

	strcat(resp, "<NewATURCountry>");
	strcat(resp, str);
	strcat(resp, "</NewATURCountry>");
	strcat(resp, "<NewATURANSIStd>");
	strcat(resp, "0");
	strcat(resp, "</NewATURANSIStd>");
	strcat(resp, "<NewATURANSIRev>");
	strcat(resp, "0");
	strcat(resp, "</NewATURANSIRev>");

	if(adsl_drv_get(RLCM_MODEM_FAR_END_ID_REQ, (void *)&vMId, RLCM_MODEM_ID_REQ_SIZE))
	{
		snprintf(str, 256, "%c%c%c%c",
		(char)((vMId.ITU_VendorId.vendorCode>>24) &0xff),
		(char)((vMId.ITU_VendorId.vendorCode>>16) &0xff),
		(char)((vMId.ITU_VendorId.vendorCode>>8) &0xff),
		(char)((vMId.ITU_VendorId.vendorCode) &0xff));
	}
	else
		strcpy(str, "Action Failed: Get ATUCVendor failed!");

	strcat(resp, "<NewATUCVendor>");
	strcat(resp, str);
	strcat(resp, "</NewATUCVendor>");

	if(adsl_drv_get(RLCM_MODEM_FAR_END_ID_REQ, (void *)&vMId, RLCM_MODEM_ID_REQ_SIZE))
		sprintf(str, "%d", vMId.ITU_VendorId.countryCode);
	else
		strcpy(str, "Action Failed: Get ATUCCountry failed!");

	strcat(resp, "<NewATUCCountry>");
	strcat(resp, str);
	strcat(resp, "</NewATUCCountry>");
	strcat(resp, "<NewATUCANSIStd>");
	strcat(resp, "0");
	strcat(resp, "</NewATUCANSIStd>");
	strcat(resp, "<NewATUCANSIRev>");
	strcat(resp, "0");
	strcat(resp, "</NewATUCANSIRev>");

 struct sysinfo info;
 sysinfo(&info);
 sprintf(str, "%ld", info.uptime);

	strcat(resp, "<NewTotalStart>");
	strcat(resp, str);
	strcat(resp, "</NewTotalStart>");

	unsigned int vUint[3];
	if(adsl_drv_get(RLCM_GET_DSL_ORHERS, (void *)vUint, TR069_DSL_OTHER_SIZE))
		sprintf(str, "%d", vUint[0]);
	else
		strcpy(str, "Action Failed: Get ATUCCountry failed!");

	strcat(resp, "<NewShowtimeStart>");
	strcat(resp, str);
	strcat(resp, "</NewShowtimeStart>");
	strcat(resp, "<NewLastShowtimeStart>");
	strcat(resp, "0");
	strcat(resp, "</NewLastShowtimeStart>");
	strcat(resp, "<NewCurrentDayStart>");
	strcat(resp, "0");
	strcat(resp, "</NewCurrentDayStart>");
	strcat(resp, "<NewQuarterHourStart>");
	strcat(resp, "0");
	strcat(resp, "</NewQuarterHourStart>");
	strcat(resp, "</u:GetInfoResponse>");

	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANDSLInterfaceConfigGetStatisticsTotal(struct upnphttp *h)
{
	char resp[1024], str[16];
	Modem_DSLConfigStatus MDS;

	if(!adsl_drv_get(RLCM_GET_DSL_STAT_TOTAL, (void *)&MDS, TR069_STAT_SIZE))
		return;

	strcpy(resp, "<u:GetStatisticsTotalResponse xmlns:u=\"urn:dslforum-org:service:WANDSLInterfaceConfig:1\">");

	sprintf(str, "%d", MDS.ReceiveBlocks);
	strcat(resp, "<NewReceiveBlocks>");
	strcat(resp, str);
	strcat(resp, "</NewReceiveBlocks>");
	sprintf(str, "%d", MDS.TransmitBlocks);
	strcat(resp, "<NewTransmitBlocks>");
	strcat(resp, str);
	strcat(resp, "</NewTransmitBlocks>");
	sprintf(str, "%d", MDS.CellDelin);
	strcat(resp, "<NewCellDelin>");
	strcat(resp, str);
	strcat(resp, "</NewCellDelin>");
	sprintf(str, "%d", MDS.LinkRetain);
	strcat(resp, "<NewLinkRetrain>");
	strcat(resp, str);
	strcat(resp, "</NewLinkRetrain>");
	sprintf(str, "%d", MDS.InitErrors);
	strcat(resp, "<NewInitErrors>");
	strcat(resp, str);
	strcat(resp, "</NewInitErrors>");
	sprintf(str, "%d", MDS.InitTimeouts);
	strcat(resp, "<NewInitTimeouts>");
	strcat(resp, str);
	strcat(resp, "</NewInitTimeouts>");
	sprintf(str, "%d", MDS.LOF);
	strcat(resp, "<NewLossOfFraming>");
	strcat(resp, str);
	strcat(resp, "</NewLossOfFraming>");
	sprintf(str, "%d", MDS.ES);
	strcat(resp, "<NewErroredSecs>");
	strcat(resp, str);
	strcat(resp, "</NewErroredSecs>");
	sprintf(str, "%d", MDS.SES);
	strcat(resp, "<NewSeverelyErroredSecs>");
	strcat(resp, str);
	strcat(resp, "</NewSeverelyErroredSecs>");
	sprintf(str, "%d", MDS.FEC);
	strcat(resp, "<NewFECErrors>");
	strcat(resp, str);
	strcat(resp, "</NewFECErrors>");
	sprintf(str, "%d", MDS.AtucFEC);
	strcat(resp, "<NewATUCFECErrors>");
	strcat(resp, str);
	strcat(resp, "</NewATUCFECErrors>");
	sprintf(str, "%d", MDS.HEC);
	strcat(resp, "<NewHECErrors>");
	strcat(resp, str);
	strcat(resp, "</NewHECErrors>");
	sprintf(str, "%d", MDS.AtucHEC);
	strcat(resp, "<NewATUCHECErrors>");
	strcat(resp, str);
	strcat(resp, "</NewATUCHECErrors>");
	sprintf(str, "%d", MDS.CRC);
	strcat(resp, "<NewCRCErrors>");
	strcat(resp, str);
	strcat(resp, "</NewCRCErrors>");
	sprintf(str, "%d", MDS.AtucCRC);
	strcat(resp, "<NewATUCCRCErrors>");
	strcat(resp, str);
	strcat(resp, "</NewATUCCRCErrors>");
	strcat(resp, "</u:GetStatisticsTotalResponse>");

	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANDSLConnectionManagementGetWANConnectionServiceNumberOfEntries(struct upnphttp *h)
{
	char resp[260], str[12];
	int totalEntry;

	totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
	sprintf(str, "%d", totalEntry);

	strcpy(resp, "<u:GetWANConnectionServiceNumberOfEntriesResponse xmlns:u=\"urn:dslforum-org:service:WANDSLConnectionManagement:1\">");
	strcat(resp, "<NewWANConnectionServiceNumberOfEntries>");
	strcat(resp, str);
	strcat(resp, "</NewWANConnectionServiceNumberOfEntries>");
	strcat(resp, "</u:GetWANConnectionServiceNumberOfEntriesResponse>");

	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANDSLConnectionManagementGetGenericConnectionServiceEntry(struct upnphttp *h)
{
	char resp[512], wan_cndv[256], wan_cnsr[256], name[32];
	char *str;
	struct NameValueParserData data;
	int cs_idx = -1, totalEntry = 0;
	MIB_CE_ATM_VC_T entry;

 	totalEntry = mib_chain_total(MIB_ATM_VC_TBL);

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	if((str = GetValueFromNameValueList(&data, "NewConnectionServiceIndex")))
		cs_idx = atoi(str);

	if( cs_idx >= 0 && cs_idx < totalEntry )
	{
		sprintf(wan_cndv, "WANConnectionDevice:%d", cs_idx);

		int i, ipc = -1, pppc = -1, st = 0;
		for( i = 0; i <= cs_idx; i++ )
		{
			mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entry);
			if ( (entry.cmode == ADSL_PPPoE) || (entry.cmode == ADSL_PPPoA) )
			{
				pppc++;
				st = 'p';
			}
			else
			{
				ipc++;
				st = 'i';
			}
		}

		if( st == 'p' )
			sprintf(wan_cnsr, "%s.WANPPPConnection:%d", wan_cndv, pppc);
		else if ( st == 'i' )
			sprintf(wan_cnsr, "%s.WANIPConnection:%d", wan_cndv, ipc);
		else
			strcpy(wan_cnsr, "Get WANConnectionService failed!");

		switch(entry.cmode)
		{
			case ADSL_PPPoE:
				strcpy(name, "PPPoE");
				break;
			case ADSL_PPPoA:
				strcpy(name, "PPPoA");
				break;
			case ADSL_BR1483:
				strcpy(name, "1483 Bridged");
				break;
			case ADSL_MER1483:
				strcpy(name, "1483 MER");
				break;
			case ADSL_RT1483:
				strcpy(name, "1483 Routed");
				break;
#ifdef CONFIG_ATM_CLIP
			case ADSL_RT1577:
				strcpy(name, "1577 Routed");
				break;
#endif
			default:
				sprintf(name, "Get Name failed!");
				break;
		}

	}
	else
	{
		strcpy(wan_cndv, "Get WANConnectionDevice failed!");
		strcpy(wan_cnsr, "Get WANConnectionService failed!");
		strcpy(name, "Get Name failed!");
	}

	strcpy(resp, "<u:GetGenericConnectionServiceEntryResponse xmlns:u=\"urn:dslforum-org:service:WANDSLConnectionManagement:1\">");
	strcat(resp, "<NewWANConnectionDevice>");
	strcat(resp, wan_cndv);
	strcat(resp, "</NewWANConnectionDevice>");
	strcat(resp, "<NewWANConnectionService>");
	strcat(resp, wan_cnsr);
	strcat(resp, "</NewWANConnectionService>");
	strcat(resp, "<NewName>");
	strcat(resp, name);
	strcat(resp, "</NewName>");
	strcat(resp, "</u:GetGenericConnectionServiceEntryResponse>");

	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANDSLConnectionManagementGetSpecificConnectionServiceEntry(struct upnphttp *h)
{
	struct NameValueParserData data;
	char resp[378], wan_cndv[256], name[32];
	char *str;
	int totalEntry, idx;
	MIB_CE_ATM_VC_T entry;
	bool isfound = false;

 	totalEntry = mib_chain_total(MIB_ATM_VC_TBL);

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	if((str = GetValueFromNameValueList(&data, "NewWANConnectionService")))
	{
		char *tok1, *tok2 = NULL;
		char del[] = ":";

		if((tok1 = strtok(str, del)))
			tok2 = strtok(NULL, del);

		if( !strcasecmp(tok1, "WANPPPConnection") && tok2 != NULL )
		{
			idx = atoi(tok2);

			int i, pppc = -1;
			for( i = 0; i < totalEntry; i++ )
			{
				mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entry);

				if( (entry.cmode == ADSL_PPPoE) || (entry.cmode == ADSL_PPPoA) )
					pppc++;

				if( pppc == idx )
				{
					isfound = true;
					break;
				}
			}

			if(isfound)
				sprintf(wan_cndv, "WANConnectionDevice:%d", i);
			else
				sprintf(wan_cndv, "WANConnectionDevice not found!");

		}
		else if( !strcasecmp(tok1, "WANIPConnection") && tok2 != NULL )
		{
			idx = atoi(tok2);

			int i, ipc = -1;
			for( i = 0; i < totalEntry; i++ )
			{
				mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entry);

				if( (entry.cmode != ADSL_PPPoE) && (entry.cmode != ADSL_PPPoA) )
					ipc++;

				if( ipc == idx )
				{
					isfound = true;
					break;
				}
			}

			if(isfound)
				sprintf(wan_cndv, "WANConnectionDevice:%d", i);
			else
				sprintf(wan_cndv, "WANConnectionDevice not found!");

		}

		if( isfound && totalEntry > 0 )
		{
			switch(entry.cmode)
			{
				case ADSL_PPPoE:
					strcpy(name, "PPPoE");
					break;
				case ADSL_PPPoA:
					strcpy(name, "PPPoA");
					break;
				case ADSL_BR1483:
					strcpy(name, "1483 Bridged");
					break;
				case ADSL_MER1483:
					strcpy(name, "1483 MER");
					break;
				case ADSL_RT1483:
					strcpy(name, "1483 Routed");
					break;
#ifdef CONFIG_ATM_CLIP
				case ADSL_RT1577:
					strcpy(name, "1577 Routed");
					break;
#endif
				default:
					sprintf(name, "Get Name failed!");
					break;
			}
		}
		else
			strcpy(name, "Name not found!");

	}
	else
	{
		strcpy(wan_cndv, "Get WANConnectionService failed!");
		strcpy(name, "Get Name failed!");
	}

	strcpy(resp, "<u:GetSpecificConnectionServiceEntryResponse xmlns:u=\"urn:dslforum-org:service:WANDSLConnectionManagement:1\">");
	strcat(resp, "<NewWANConnectionDevice>");
	strcat(resp, wan_cndv);
	strcat(resp, "</NewWANConnectionDevice>");
	strcat(resp, "<NewName>");
	strcat(resp, name);
	strcat(resp, "</NewName>");
	strcat(resp, "</u:GetSpecificConnectionServiceEntryResponse>");

	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANDSLConnectionManagementAddConnectionDeviceAndService(struct upnphttp *h)
{
	struct NameValueParserData data;
	char resp[384], wan_cndv[256], wan_cnsr[256], name[32], linktype[20], cnntype[20];
	char *str;
	int totalEntry, vpi = 0, vci = 0, intVal;
	MIB_CE_ATM_VC_T entry;
	bool sflag= true;  // Use to identify action failure occured or not. if it occurs, stop remainder actions.

	int ifMap = 0, cnt = 0, i, pIdx = -1, drflag = 0;
	MIB_CE_ATM_VC_T tmpentry;

  strcpy(wan_cndv, "WAN channel add successfully!");
 	strcpy(wan_cnsr, "WAN channel add successfully!");

 	if( (totalEntry = mib_chain_total(MIB_ATM_VC_TBL)) > MAX_VC_NUM )
 	{
 		strcpy(wan_cndv, "VC channel is full!");
 		strcpy(wan_cnsr, "VC channel is full!");
 	}

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	if((str = GetValueFromNameValueList(&data, "NewDestinationAddress")))
	{
		char del[] = "/";
		char *tmp;

		tmp = strtok(str, del);

		if( (vpi = atoi(tmp)) > 255 )
		{
	 		strcpy(wan_cndv, "Invalid VPI value!");
	 		strcpy(wan_cnsr, "Invalid VPI value!");
	 		sflag = false;
		}

		if(!(tmp = strtok(NULL, del)) || (vci = atoi(tmp)) > 65535 )
		{
	  	strcpy(wan_cndv, "Invalid VCI value!");
  		strcpy(wan_cnsr, "Invalid VCI value!");
  		sflag = false;
		}
	}
	else
	{
 		strcpy(wan_cndv, "Invalid VPI/VCI value!");
 		strcpy(wan_cnsr, "Invalid VPI/VCI value!");
 		sflag = false;
	}

	if(sflag)
	{
		if((str = GetValueFromNameValueList(&data, "NewLinkType")))
			strcpy(linktype, str);
		else
		{
	 		strcpy(wan_cndv, "Invalid LinkType!");
	 		strcpy(wan_cnsr, "Invalid LinkType!");
	 		sflag = false;
		}
	}

	if(sflag)
	{
		if((str = GetValueFromNameValueList(&data, "NewConnectionType")))
			strcpy(cnntype, str);
		else
		{
	 		strcpy(wan_cndv, "Invalid ConnectionType!");
	 		strcpy(wan_cnsr, "Invalid ConnectionType!");
	 		sflag = false;
		}
	}

	if(sflag)
	{
		if( (str = GetValueFromNameValueList(&data, "NewName")) && strlen(str) <= 32 )
			strcpy(name, str);
		else
		{
 			strcpy(wan_cndv, "Invalid Name!");
 			strcpy(wan_cnsr, "Invalid Name!");
 			sflag = false;
		}
	}

	if(sflag)
	{
		deleteAllConnection();
		memset(&entry, 0x00, sizeof(entry));

 		entry.vpi = vpi;
		entry.vci = vci;

		// set default Qos
		entry.qos = 0;
		entry.pcr = ATM_MAX_US_PCR;
		entry.encap = 1;
		entry.enable = 1;

#ifdef PPPOE_PASSTHROUGH
		entry.brmode = BRIDGE_DISABLE;

		// 1483 bridged
		if (!strcasecmp(cnntype, "IP_Bridged"))
			entry.brmode = 0;
#endif

		if(!strcasecmp(linktype, "EoA"))
	  {
	  	if(!strcasecmp(cnntype, "IP_Routed"))
	  	{
	  		entry.napt = 1;
	  		entry.cmode = ADSL_MER1483;
	  		entry.dgw = 0;
	  		entry.ipunnumbered = 0;
	  		entry.ipDhcp = (char)DHCP_DISABLED;
	  		inet_aton("192.168.8.8", (struct in_addr *)&entry.ipAddr);
	  		inet_aton("192.168.8.1", (struct in_addr *)&entry.remoteIpAddr);
	  		inet_aton("255.255.255.0", (struct in_addr *)&entry.netMask);
	  	}
	  	else if(!strcasecmp(cnntype, "IP_Bridged"))
	  	{
	  		entry.cmode = ADSL_BR1483;
	  	}
	  	else
	  	{
	  		strcpy(wan_cndv, "Unsupported ConnectionType!");
    		strcpy(wan_cnsr, "Unsupported ConnectionType!");
    		sflag = false;
	  	}
  	}
  	else if(!strcasecmp(linktype, "IPoA"))
  	{
  		entry.napt = 1;
  		entry.cmode = ADSL_RT1483;
  		entry.dgw = 0;
  		entry.ipDhcp = (char)DHCP_DISABLED;
  		inet_aton("192.168.4.4", (struct in_addr *)&entry.ipAddr);
	 		inet_aton("192.168.4.1", (struct in_addr *)&entry.remoteIpAddr);
	 		inet_aton("255.255.255.0", (struct in_addr *)&entry.netMask);
  	}
  	else if(!strcasecmp(linktype, "PPPoA"))
  	{
  		entry.napt = 1;
  		entry.cmode = ADSL_PPPoA;
  		entry.dgw = 0;
  		strcpy(entry.pppUsername, "testpoa@local");
  		strcpy(entry.pppPassword, "testppp");
  		entry.pppCtype = CONTINUOUS;
  	}
  	else if(!strcasecmp(linktype, "CIP"))
  	{
    	strcpy(wan_cndv, "Unsupported LinkType!");
    	strcpy(wan_cnsr, "Unsupported LinkType!");
    	sflag = false;
  	}
  	else if(!strcasecmp(linktype, "PPPoE"))
  	{
  		entry.napt = 1;
  		entry.cmode = ADSL_PPPoE;
  		entry.dgw = 0;
  		strcpy(entry.pppUsername, "testpoe@local");
  		strcpy(entry.pppPassword, "testppp");
  		entry.pppCtype = CONTINUOUS;
#ifdef CONFIG_USER_PPPOE_PROXY
			entry.PPPoEProxyMaxUser=4;
			entry.PPPoEProxyEnable=0;
#endif
  	}
  	else if(!strcasecmp(linktype, "Unconfigured"))
  	{
  		strcpy(wan_cndv, "Unsupported LinkType!");
    	strcpy(wan_cnsr, "Unsupported LinkType!");
    	sflag = false;
  	}
  	else
  	{
    	strcpy(wan_cndv, "Invalid LinkType!");
    	strcpy(wan_cnsr, "Invalid LinkType!");
    	sflag = false;
  	}
	}

	if(sflag)
	{
		for (i = 0; i < totalEntry; i++)
		{
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&tmpentry))
			{
				strcpy(wan_cndv, "Get MIB_ATM_VC_TBL failed!");
				strcpy(wan_cnsr, "Get MIB_ATM_VC_TBL failed!");
				sflag = false;
				break;
			}

			if (tmpentry.vpi == entry.vpi && tmpentry.vci == entry.vci)
			{
				cnt++;
				pIdx = i;	// Jenny, for multisession PPPoE, record entry index instead of atmvc entry pointer
			}

#ifdef DEFAULT_GATEWAY_V1
			if (tmpentry.cmode != ADSL_BR1483)
				if (tmpentry.dgw)
					drflag = 1;
#endif

			ifMap |= 1 << (tmpentry.ifIndex & 0x0f);	// vc map
			ifMap |= (1 << 16) << ((tmpentry.ifIndex >> 4) & 0x0f);	// PPP map
		}
	}

	if(sflag)
	{
    if (cnt == 0) // pvc not exists
    {
			entry.ifIndex = if_find_index(entry.cmode, ifMap);
			if (entry.ifIndex == 0xff || entry.ifIndex == 0xef )
     		sflag = false;

#ifdef _CWMP_MIB_  // jiunming, for cwmp-tr069
     entry.ConDevInstNum = 1 + findMaxConDevInstNum();
     if( (entry.cmode==ADSL_PPPoE) ||
#ifdef PPPOE_PASSTHROUGH
         ((entry.cmode==ADSL_BR1483)&&(entry.brmode==BRIDGE_PPPOE)) ||
#endif
         (entry.cmode==ADSL_PPPoA) )
      entry.ConPPPInstNum = 1;
     else
      entry.ConIPInstNum = 1;
     //fprintf( stderr, "<%s:%d>NewInstNum=>ConDev:%u, PPPCon:%u, IPCon:%u\n", __FILE__, __LINE__, entry.ConDevInstNum, entry.ConPPPInstNum, entry.ConIPInstNum );
#endif
    }
    else
    {
    	if (!mib_chain_get(MIB_ATM_VC_TBL, pIdx, (void *)&tmpentry))
    	{	// Jenny, for multisession PPPoE, get existed pvc config
	      strcpy(wan_cndv, "Get MIB_ATM_VC_TBL failed!");
  	    strcpy(wan_cnsr, "Get MIB_ATM_VC_TBL failed!");
    	  sflag = false;
			}

			if(tmpentry.cmode == ADSL_PPPoE && entry.cmode == ADSL_PPPoE)
			{
				if( cnt < MAX_POE_PER_VC )
				{	// get the pvc info.
					entry.qos = tmpentry.qos;
					entry.pcr = tmpentry.pcr;
					entry.encap = tmpentry.encap;
					ifMap &= 0xffff0000; // don't care the vc part
					entry.ifIndex = if_find_index(entry.cmode, ifMap);

					if(entry.ifIndex == 0xef)
					{
		      	strcpy(wan_cndv, "Get MIB_ATM_VC_TBL failed!");
	  	    	strcpy(wan_cnsr, "Get MIB_ATM_VC_TBL failed!");
						sflag = false;
					}
					else
					{
						entry.ifIndex &= 0xf0;
						entry.ifIndex |= (tmpentry.ifIndex&0x0f);
#ifdef PPPOE_PASSTHROUGH
						entry.brmode = tmpentry.brmode;	// Jenny, for multisession PPPoE
#endif

#ifdef _CWMP_MIB_ // jiunming, for cwmp-tr069
						entry.ConDevInstNum = tmpentry.ConDevInstNum;
						entry.ConPPPInstNum = 1 + findMaxPPPConInstNum(tmpentry.ConDevInstNum);
#endif
					}
				}
				else
				{
					strcpy(wan_cndv, "Maximum PPPoEs per VC reached!");
					strcpy(wan_cnsr, "Maximum PPPoEs per VC reached!");
					sflag = false;
				}
			}
			else
			{
				strcpy(wan_cndv, "Connection existed!");
				strcpy(wan_cnsr, "Connection existed!");
				sflag = false;
			}
    }
	}

	if(sflag)
	{
#ifdef DEFAULT_GATEWAY_V1
		if (entry.cmode != ADSL_BR1483)
			if (drflag && entry.dgw)
			{
				strcpy(wan_cndv, "Default Route existed!");
				strcpy(wan_cnsr, "Default Route existed!");
				sflag = false;
			}
#endif
	}

	if(sflag)
	{
#ifdef CONFIG_ATM_CLIP
		if (entry.cmode == ADSL_RT1577)
			entry.encap = 1;	// LLC
#endif

		// set default
//		entry.dgw = 1;
		if (entry.cmode == ADSL_PPPoE)
			{
			entry.mtu = 1492;
#ifdef CONFIG_USER_PPPOE_PROXY
			entry.PPPoEProxyMaxUser=4;
			entry.PPPoEProxyEnable=0;
#endif
			}
		else
			entry.mtu = 1500;

#ifdef CONFIG_EXT_SWITCH
		// VLAN
		entry.vlan = 0;	// disable
		entry.vid = 0; // VLAN tag
		entry.vprio = 0; // priority bits (0 ~ 7)
		entry.vpass = 0; // no pass-through
#endif
//add by ramen for ZTE Router rmt acc on pvc
#ifdef ZTE_GENERAL_ROUTER_SC
		MIB_CE_ACC_T accEntry;
		assignDefaultValue(&accEntry);
		copyPvcEntryToAccEntry(&accEntry,&entry);

		intVal = mib_chain_add(MIB_ACC_TBL, (unsigned char*)&accEntry);
		if (intVal == 0)
		{
			strcpy(wan_cndv, "Add chain record falied!");
			strcpy(wan_cnsr, "Add chain record falied!");
			sflag = false;
		}
		else if (intVal == -1) {
			strcpy(wan_cndv, T(strTableFull));
			strcpy(wan_cnsr, T(strTableFull));
			sflag = false;
		}
#endif
		intVal = mib_chain_add(MIB_ATM_VC_TBL, (unsigned char*)&entry);
		if (intVal == 0)
		{
			strcpy(wan_cndv, "Add chain record falied!");
			strcpy(wan_cnsr, "Add chain record falied!");
			sflag = false;
		}
		else if (intVal == -1) {
			strcpy(wan_cndv, T(strTableFull));
			strcpy(wan_cnsr, T(strTableFull));
			sflag = false;
		}
	}

	if(sflag)
	{
#ifdef CONFIG_GUI_WEB
	  writeFlash();
#endif
  	// Added by Mason Yu. for take effect in real time
	  restartWAN();

		totalEntry = mib_chain_total(MIB_ATM_VC_TBL);

		int i, ipc = -1, pppc = -1, st = 0;
		for( i = 0; i < totalEntry; i++ )
		{
			mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&tmpentry);
			if ( (tmpentry.cmode == ADSL_PPPoE) || (tmpentry.cmode == ADSL_PPPoA) )
			{
				pppc++;
				st = 'p';
			}
			else
			{
				ipc++;
				st = 'i';
			}
		}

		sprintf(wan_cndv, "WANConnectionDevice:%d", i-1);

		if( st == 'p' )
			sprintf(wan_cnsr, "%s.WANPPPConnection:%d", wan_cndv, pppc);
		else if ( st == 'i' )
			sprintf(wan_cnsr, "%s.WANIPConnection:%d", wan_cndv, ipc);
		else
			strcpy(wan_cnsr, "Get WANConnectionService failed!");
	}

	strcpy(resp, "<u:AddConnectionDeviceAndServiceResponse xmlns:u=\"urn:dslforum-org:service:WANDSLConnectionManagement:1\">");
	strcat(resp, "<NewWANConnectionDevice>");
	strcat(resp, wan_cndv);
	strcat(resp, "</NewWANConnectionDevice>");
	strcat(resp, "<NewWANConnectionService>");
	strcat(resp, wan_cnsr);
	strcat(resp, "</NewWANConnectionService>");
	strcat(resp, "</u:AddConnectionDeviceAndServiceResponse>");

	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANDSLConnectionManagementAddConnectionService(struct upnphttp *h)
{
  struct NameValueParserData data;
  char resp[256], wan_cndv[256], wan_cnsr[256], name[32], cnntype[20];
  char *str;
  int totalEntry, intVal;
  MIB_CE_ATM_VC_T entry;
  bool sflag= true;  // Use to identify action failure occured or not. if it occurs, stop remainder actions.

  int ifMap = 0, cnt = 0, i, pIdx = -1, drflag = 0;
  MIB_CE_ATM_VC_T tmpentry;

	strcpy(wan_cnsr, "WAN channel add successfully!");

	if( (totalEntry = mib_chain_total(MIB_ATM_VC_TBL)) > MAX_VC_NUM )
		strcpy(wan_cnsr, "VC channel is full!");

  ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	if(sflag)
  {
		if((str = GetValueFromNameValueList(&data, "NewWANConnectionDevice")))
			strcpy(wan_cndv, str);
		else
		{
			strcpy(wan_cnsr, "Invalid WANConnectionDevice!");
			sflag = false;
		}
	}

  if(sflag)
  {
		if((str = GetValueFromNameValueList(&data, "NewConnectionType")))
			strcpy(cnntype, str);
		else
		{
			strcpy(wan_cnsr, "Invalid ConnectionType!");
			sflag = false;
		}
	}

  if(sflag)
  {
		if( (str = GetValueFromNameValueList(&data, "NewName")) && strlen(str) <= 32 )
		{
			int len = strlen(str);

			for( i = 0; i < len; i++ )
				name[i] = tolower(str[i]);

			name[len] = '\0';
		}
		else
		{
			strcpy(wan_cnsr, "Invalid Name!");
			sflag = false;
		}
	}

  if(sflag)
  {
		deleteAllConnection();
		memset(&entry, 0x00, sizeof(entry));

   // set default Qos
		entry.qos = 0;
		entry.pcr = ATM_MAX_US_PCR;
		entry.encap = 1;
		entry.enable = 1;

#ifdef PPPOE_PASSTHROUGH
		entry.brmode = BRIDGE_DISABLE;

		// 1483 bridged
		if (!strcasecmp(cnntype, "IP_Bridged"))
			entry.brmode = 0;
#endif
		if(!strcasecmp(name, "1483 mer"))
		{
			entry.vpi = 0;
			entry.vci = 35;
			entry.napt = 1;
			entry.cmode = ADSL_MER1483;
			entry.dgw = 0;
			entry.ipunnumbered = 0;
			entry.ipDhcp = (char)DHCP_DISABLED;
			inet_aton("192.168.8.8", (struct in_addr *)&entry.ipAddr);
			inet_aton("192.168.8.1", (struct in_addr *)&entry.remoteIpAddr);
			inet_aton("255.255.255.0", (struct in_addr *)&entry.netMask);
		}
		else if(!strcasecmp(name, "1483 bridged"))
		{
			entry.vpi = 0;
			entry.vci = 35;
			entry.cmode = ADSL_BR1483;
		}
    else if(!strcasecmp(name, "1483 routed"))
    {
    	entry.vpi = 8;
    	entry.vci = 35;
			entry.napt = 1;
			entry.cmode = ADSL_RT1483;
			entry.dgw = 0;
     entry.ipDhcp = (char)DHCP_DISABLED;
     inet_aton("192.168.4.4", (struct in_addr *)&entry.ipAddr);
     inet_aton("192.168.4.1", (struct in_addr *)&entry.remoteIpAddr);
     inet_aton("255.255.255.0", (struct in_addr *)&entry.netMask);
    }
    else if(!strcasecmp(name, "pppoa"))
    {
    	entry.vpi = 4;
    	entry.vci = 35;
     entry.napt = 1;
     entry.cmode = ADSL_PPPoA;
     entry.dgw = 0;
     strcpy(entry.pppUsername, "testpoa@local");
     strcpy(entry.pppPassword, "testppp");
     entry.pppCtype = CONTINUOUS;
    }
    else if(!strcasecmp(name, "pppoe"))
    {
    	entry.vpi = 5;
    	entry.vci = 35;
     entry.napt = 1;
     entry.cmode = ADSL_PPPoE;
     entry.dgw = 0;
     strcpy(entry.pppUsername, "testpoe@local");
     strcpy(entry.pppPassword, "testppp");
     entry.pppCtype = CONTINUOUS;
#ifdef CONFIG_USER_PPPOE_PROXY
			entry.PPPoEProxyMaxUser=4;
			entry.PPPoEProxyEnable=0;
#endif
    }
    else
    {
      strcpy(wan_cnsr, "Unsupported LinkType!");
      sflag = false;
    }
  }

	if(sflag)
	{
		for (i = 0; i < totalEntry; i++)
		{
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&tmpentry))
			{
				strcpy(wan_cnsr, "Get MIB_ATM_VC_TBL failed!");
				sflag = false;
				break;
			}

			if (tmpentry.vpi == entry.vpi && tmpentry.vci == entry.vci)
			{
				cnt++;
				pIdx = i; // Jenny, for multisession PPPoE, record entry index instead of atmvc entry pointer
			}

#ifdef DEFAULT_GATEWAY_V1
			if (tmpentry.cmode != ADSL_BR1483)
				if (tmpentry.dgw)
					drflag = 1;
#endif

			ifMap |= 1 << (tmpentry.ifIndex & 0x0f); // vc map
			ifMap |= (1 << 16) << ((tmpentry.ifIndex >> 4) & 0x0f); // PPP map
		}
	}

  if(sflag)
  {
		if (cnt == 0) // pvc not exists
		{
			entry.ifIndex = if_find_index(entry.cmode, ifMap);
			if (entry.ifIndex == 0xff || entry.ifIndex == 0xef )
				sflag = false;

#ifdef _CWMP_MIB_ //jiunming, for cwmp-tr069
				entry.ConDevInstNum = 1 + findMaxConDevInstNum();
				if( (entry.cmode==ADSL_PPPoE) ||
#ifdef PPPOE_PASSTHROUGH
					((entry.cmode==ADSL_BR1483)&&(entry.brmode==BRIDGE_PPPOE)) ||
#endif
					(entry.cmode==ADSL_PPPoA) )
					entry.ConPPPInstNum = 1;
				else
					entry.ConIPInstNum = 1;
#endif
			}
			else
			{
				if (!mib_chain_get(MIB_ATM_VC_TBL, pIdx, (void *)&tmpentry))
				{ // Jenny, for multisession PPPoE, get existed pvc config
					strcpy(wan_cnsr, "Get MIB_ATM_VC_TBL failed!");
					sflag = false;
				}

				if(tmpentry.cmode == ADSL_PPPoE && entry.cmode == ADSL_PPPoE)
				{
					if( cnt < MAX_POE_PER_VC )
					{ // get the pvc info.
						entry.qos = tmpentry.qos;
						entry.pcr = tmpentry.pcr;
						entry.encap = tmpentry.encap;
						ifMap &= 0xffff0000; // don't care the vc part
						entry.ifIndex = if_find_index(entry.cmode, ifMap);

						if(entry.ifIndex == 0xef)
						{
							strcpy(wan_cnsr, "Get MIB_ATM_VC_TBL failed!");
							sflag = false;
						}
						else
						{
							entry.ifIndex &= 0xf0;
							entry.ifIndex |= (tmpentry.ifIndex&0x0f);
#ifdef PPPOE_PASSTHROUGH
							entry.brmode = tmpentry.brmode; // Jenny, for multisession PPPoE
#endif

#ifdef _CWMP_MIB_ // jiunming, for cwmp-tr069
							entry.ConDevInstNum = tmpentry.ConDevInstNum;
							entry.ConPPPInstNum = 1 + findMaxPPPConInstNum(tmpentry.ConDevInstNum);
#endif
						}
					}
					else
					{
						strcpy(wan_cnsr, "Maximum PPPoEs per VC reached!");
						sflag = false;
					}
				}
				else
				{
					strcpy(wan_cnsr, "Connection existed!");
					sflag = false;
				}
			}
	}

	if(sflag)
	{
#ifdef DEFAULT_GATEWAY_V1
   if (entry.cmode != ADSL_BR1483)
    if (drflag && entry.dgw)
    {
			strcpy(wan_cnsr, "Default route existed!");
			sflag = false;
    }
#endif
	}

	if(sflag)
	{
#ifdef CONFIG_ATM_CLIP
		if (entry.cmode == ADSL_RT1577)
			entry.encap = 1; // LLC
#endif

		//set default
		//entry.dgw = 1;
		if (entry.cmode == ADSL_PPPoE)
		{
			entry.mtu = 1492;
#ifdef CONFIG_USER_PPPOE_PROXY
			entry.PPPoEProxyMaxUser=4;
			entry.PPPoEProxyEnable=0;
#endif
		}
		else
			entry.mtu = 1500;

#ifdef CONFIG_EXT_SWITCH
   // VLAN
		entry.vlan = 0; // disable
		entry.vid = 0; // VLAN tag
		entry.vprio = 0; // priority bits (0 ~ 7)
		entry.vpass = 0; // no pass-through
#endif
 //add by ramen for ZTE Router rmt acc on pvc
#ifdef ZTE_GENERAL_ROUTER_SC
		MIB_CE_ACC_T accEntry;
		assignDefaultValue(&accEntry);
		copyPvcEntryToAccEntry(&accEntry,&entry);

		intVal = mib_chain_add(MIB_ACC_TBL, (unsigned char*)&accEntry);
		if (intVal == 0)
		{
			strcpy(wan_cnsr, "Add chain record falied!");
			sflag = false;
		}
		else if (intVal == -1) {
			strcpy(wan_cnsr, T(strTableFull));
			sflag = false;
		}
#endif
		intVal = mib_chain_add(MIB_ATM_VC_TBL, (unsigned char*)&entry);
		if (intVal == 0)
		{
			strcpy(wan_cnsr, "Add chain record falied!");
			sflag = false;
		}
		else if (intVal == -1) {
			strcpy(wan_cnsr, T(strTableFull));
			sflag = false;
		}
	}

	if(sflag)
	{
#ifdef CONFIG_GUI_WEB
		writeFlash();
#endif
    // Added by Mason Yu. for take effect in real time
		restartWAN();

		totalEntry = mib_chain_total(MIB_ATM_VC_TBL);

		int i, ipc = -1, pppc = -1, st = 0;
		for( i = 0; i < totalEntry; i++ )
		{
			mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&tmpentry);
			if ( (tmpentry.cmode == ADSL_PPPoE) || (tmpentry.cmode == ADSL_PPPoA) )
			{
				pppc++;
				st = 'p';
			}
			else
			{
				ipc++;
				st = 'i';
			}
		}

   sprintf(wan_cndv, "WANConnectionDevice:%d", i-1);

   if( st == 'p' )
    sprintf(wan_cnsr, "%s.WANPPPConnection:%d", wan_cndv, pppc);
   else if ( st == 'i' )
    sprintf(wan_cnsr, "%s.WANIPConnection:%d", wan_cndv, ipc);
   else
    strcpy(wan_cnsr, "Get WANConnectionService failed!");
  }

  strcpy(resp, "<u:AddConnectionServiceResponse xmlns:u=\"urn:dslforum-org:service:WANDSLConnectionManagement:1\">");
  strcat(resp, "<NewWANConnectionService>");
  strcat(resp, wan_cnsr);
  strcat(resp, "</NewWANConnectionService>");
  strcat(resp, "</u:AddConnectionServiceResponse>");

  ClearNameValueList(&data);
  BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANDSLConnectionManagementDeleteConnectionService(struct upnphttp *h)
{
	struct NameValueParserData data;
	char resp[256];
	char *str;
	int totalEntry, idx, delidx = -1;
	MIB_CE_ATM_VC_T entry;
	bool isfound = false;

 	totalEntry = mib_chain_total(MIB_ATM_VC_TBL);

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	if((str = GetValueFromNameValueList(&data, "NewWANConnectionService")))
	{
		char *tok1, *tok2 = NULL;
		char del[] = ":";

		if((tok1 = strtok(str, del)))
			tok2 = strtok(NULL, del);

		if( !strcasecmp(tok1, "WANPPPConnection") && tok2 != NULL )
		{
			idx = atoi(tok2);

			int i, pppc = -1;
			for( i = 0; i < totalEntry; i++ )
			{
				mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entry);

				if( (entry.cmode == ADSL_PPPoE) || (entry.cmode == ADSL_PPPoA) )
					pppc++;

				if( pppc == idx )
				{
					isfound = true;
					break;
				}
			}

			if(isfound)
				delidx = i;

		}
		else if( !strcasecmp(tok1, "WANIPConnection") && tok2 != NULL )
		{
			idx = atoi(tok2);

			int i, ipc = -1;
			for( i = 0; i < totalEntry; i++ )
			{
				mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entry);

				if( (entry.cmode != ADSL_PPPoE) && (entry.cmode != ADSL_PPPoA) )
					ipc++;

				if( ipc == idx )
				{
					isfound = true;
					break;
				}
			}

			if(isfound)
				delidx = i;

		}
	}

	if(isfound)
	{
		unsigned int i;
		unsigned int idx;

		// Mason Yu
		deleteAllConnection();

		for( i = 0; i < totalEntry; i++ )
		{
			idx = totalEntry - i - 1;

			if ( idx == delidx )
			{
				resolveServiceDependency(idx);
//add by ramen to delete the services access control of each pvc
#ifdef ZTE_GENERAL_ROUTER_SC
				MIB_CE_ATM_VC_T tempPvcEntry;

				if(mib_chain_get(MIB_ATM_VC_TBL,idx,&tempPvcEntry)&&(tempPvcEntry.cmode!=ADSL_BR1483))
				{
					filter_set_remote_access(0);
					mib_chain_delete(MIB_ACC_TBL,idx);
					filter_set_remote_access(1);
				}
				else
					mib_chain_delete(MIB_ACC_TBL,idx);
#endif
//add by ramen to check whether the deleted pvc bind a dns!
#ifdef DNS_BIND_PVC_SUPPORT
				MIB_CE_ATM_VC_T dnsPvcEntry;

				if(mib_chain_get(MIB_ATM_VC_TBL,idx,&dnsPvcEntry) && (dnsPvcEntry.cmode != ADSL_BR1483))
				{
					int tempi=0;
					unsigned char pvcifIdx=0;

					for(tempi=0;tempi<3;tempi++)
					{
						mib_get(MIB_DNS_BIND_PVC1+tempi,(void*)&pvcifIdx);

						if(pvcifIdx==dnsPvcEntry.ifIndex)//I get it
						{
							pvcifIdx=255;
							mib_set(MIB_DNS_BIND_PVC1+tempi,(void*)&pvcifIdx);
						}
					}
				}
#endif
#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
				{
					MIB_CE_ATM_VC_T cwmp_entry;
					if (mib_chain_get(MIB_ATM_VC_TBL, idx, (void *)&cwmp_entry))
					{
						delPortForwarding( cwmp_entry.ifIndex );
						delRoutingTable( cwmp_entry.ifIndex );
					}
				}
#endif
				if(mib_chain_delete(MIB_ATM_VC_TBL, idx) != 1)
					isfound = false;

				break;
			}
		}
		//ql add: check if it is necessary to delete a group of interface
#ifdef CONFIG_EXT_SWITCH
#ifdef ITF_GROUP
		if(isfound)
		{
			int wanPortNum;
			unsigned int swNum, vcNum;
			MIB_CE_SW_PORT_T Entry;
			MIB_CE_ATM_VC_T pvcEntry;
			int j, grpnum;
			char mygroup;

			vcNum = mib_chain_total(MIB_ATM_VC_TBL);
			for (grpnum=1; grpnum<=4; grpnum++)
			{
				wanPortNum = 0;

				for ( j = 0; j < vcNum; j++ )
				{
					if (!mib_chain_get(MIB_ATM_VC_TBL, j, (void *)&pvcEntry))
					{
			  		//websError(wp, 400, T("Get chain record error!\n"));
			  		printf("Get chain record error!\n");
						return;
					}

					if (pvcEntry.enable == 0 || pvcEntry.itfGroup!=grpnum)
						continue;

					wanPortNum++;
				}

				if (0 == wanPortNum)
				{
					printf("delete port mapping group %d\n", grpnum);
					//release LAN ports
					swNum = mib_chain_total(MIB_SW_PORT_TBL);
					for (j=swNum; j>0; j--)
					{
						if (!mib_chain_get(MIB_SW_PORT_TBL, j-1, (void *)&Entry))
							return;

						if (Entry.itfGroup == grpnum)
						{
							Entry.itfGroup = 0;
							mib_chain_update(MIB_SW_PORT_TBL, (void *)&Entry, j-1);
						}
					}
#ifdef WLAN_SUPPORT
					//release wlan0
					mib_get(MIB_WLAN_ITF_GROUP, (void *)&mygroup);

					if (mygroup == grpnum)
					{
						mygroup = 0;
						mib_set(MIB_WLAN_ITF_GROUP, (void *)&mygroup);
					}
#endif
#ifdef ZTE_GENERAL_ROUTER_SC
#ifdef WLAN_MBSSID
					//release MBSSID
					for ( j = 1; j < 5; j++)
					{
						mib_get(MIB_WLAN_VAP0_ITF_GROUP+j-1, (void *)&mygroup);
						if (mygroup == grpnum)
						{
							mygroup = 0;
							mib_set(MIB_WLAN_VAP0_ITF_GROUP+j-1, (void *)&mygroup);
						}
					}
#endif
#endif
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
					setgroup("", grpnum, lowPrio);
#else
					setgroup("", grpnum);
#endif
					break;
				}
			}
		}//end
#endif	// ITF_GROUP
#endif // CONFIG_EXT_SWITCH
#ifdef CONFIG_GUI_WEB
		writeFlash();
#endif
		// Added by Mason Yu. for take effect in real time
		restartWAN();
	}

	strcpy(resp, "<u:DeleteConnectionServiceResponse xmlns:u=\"urn:dslforum-org:service:WANDSLConnectionManagement:1\">");
	strcat(resp, "</u:DeleteConnectionServiceResponse>");

  ClearNameValueList(&data);
  BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANDSLLinkConfigSetEnable(struct upnphttp *h)
{
	char *str, resp[128];
	int dslidx;

	str = strstr(h->req_buf + h->req_contentoff, "WANDSLLinkConfig") + strlen("WANDSLLinkConfig:");
	dslidx = atoi(str);

	Commit(h, dslidx);

	sprintf(resp, "<u:SetEnableResponse xmlns:u=\"urn:dslforum-org:service:WANDSLLinkConfig:%d\"></u:SetEnableResponse>", dslidx);

  BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANDSLLinkConfigSetDestinationAddress(struct upnphttp *h)
{
	struct NameValueParserData data;
	char *str, resp[128];
	int vpi, vci, dslidx;
	MIB_CE_ATM_VC_T entry;

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	str = strstr(h->req_buf + h->req_contentoff, "WANDSLLinkConfig") + strlen("WANDSLLinkConfig:");
	dslidx = atoi(str);

	if((str = GetValueFromNameValueList(&data, "NewDestinationAddress")))
	{
		char del[] = "/";
		char *tok1, *tok2;

		if((tok1 = strtok(str, del)) && (tok2 = strtok(NULL, del)))
		{
			vpi = atoi(tok1);
			vci = atoi(tok2);

			if(mib_chain_get(MIB_ATM_VC_TBL, dslidx, (void *)&entry))
			{
				entry.vpi = vpi;
				entry.vci = vci;

				mib_chain_update(MIB_ATM_VC_TBL, (void *)&entry, dslidx);
			}
		}
	}

	sprintf(resp, "<u:SetDestinationAddressResponse xmlns:u=\"urn:dslforum-org:service:WANDSLLinkConfig:%d\"></u:SetDestinationAddressResponse>", dslidx);

  ClearNameValueList(&data);
  BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANDSLLinkConfigGetDestinationAddress(struct upnphttp *h)
{
	char resp[256], dest_addr[32], *idxstr;
	int dslidx;
	MIB_CE_ATM_VC_T entry;

	idxstr = strstr(h->req_buf + h->req_contentoff, "WANDSLLinkConfig") + strlen("WANDSLLinkConfig:");
	dslidx = atoi(idxstr);

	if(mib_chain_get(MIB_ATM_VC_TBL, dslidx, (void *)&entry))
	{
		sprintf(dest_addr, "%d/%d", entry.vpi,	entry.vci);

		if(!mib_chain_update(MIB_ATM_VC_TBL, (void *)&entry, dslidx))
			strcpy(dest_addr, "Update chain record failed!");
	}
	else
		strcpy(dest_addr, "Get chain record failed!");

	sprintf(resp, "<u:GetDestinationAddressResponse xmlns:u=\"urn:dslforum-org:service:WANDSLLinkConfig:%d\">", dslidx);
	strcat(resp, "<NewDestinationAddress>");
	strcat(resp, dest_addr);
	strcat(resp, "</NewDestinationAddress>");
	strcat(resp, "</u:GetDestinationAddressResponse>");

	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANDSLLinkConfigSetDSLLinkType(struct upnphttp *h)
{
	struct NameValueParserData data;
	char *str, resp[128];
	int dslidx;
	MIB_CE_ATM_VC_T entry;

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	str = strstr(h->req_buf + h->req_contentoff, "WANDSLLinkConfig") + strlen("WANDSLLinkConfig:");
	dslidx = atoi(str);

	if((str = GetValueFromNameValueList(&data, "NewLinkType")))
	{
		if(mib_chain_get(MIB_ATM_VC_TBL, dslidx, (void *)&entry))
		{
			if( !strcasecmp(str, "IPoA") )
				entry.cmode = ADSL_RT1483;
			else if( !strcasecmp(str, "PPPoA") )
			{
				entry.cmode = ADSL_PPPoA;
				strcpy(entry.pppUsername, "");
				strcpy(entry.pppPassword, "");
			}
			else if( !strcasecmp(str, "PPPoE") )
			{
				entry.cmode = ADSL_PPPoE;
				strcpy(entry.pppUsername, "");
				strcpy(entry.pppPassword, "");
			}

			mib_chain_update(MIB_ATM_VC_TBL, (void *)&entry, dslidx);
		}
	}

	sprintf(resp, "<u:SetDSLLinkTypeResponse xmlns:u=\"urn:dslforum-org:service:WANDSLLinkConfig:%d\"></u:SetDSLLinkTypeResponse>", dslidx);

	ClearNameValueList(&data);
  BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANDSLLinkConfigGetDSLLinkInfo(struct upnphttp *h)
{
	char *str, resp[160], *linktype;
	int dslidx;
	MIB_CE_ATM_VC_T entry;

	str = strstr(h->req_buf + h->req_contentoff, "WANDSLLinkConfig") + strlen("WANDSLLinkConfig:");
	dslidx = atoi(str);

	if(mib_chain_get(MIB_ATM_VC_TBL, dslidx, (void *)&entry))
	{
		switch(entry.cmode)
		{
			case ADSL_BR1483:
			case ADSL_MER1483:
				linktype = strdup("EoA");
				break;
			case ADSL_RT1483:
				linktype = strdup("IPoA");
				break;
			case ADSL_PPPoA:
				linktype = strdup("PPPoA");
				break;
			case ADSL_PPPoE:
				linktype = strdup("PPPoE");
				break;
			default:
				linktype = strdup("Unconfigured");
				break;
		}
	}
	else
		linktype = strdup("Get chain record failed!");

	sprintf(resp, "<u:GetDSLLinkInfoResponse xmlns:u=\"urn:dslforum-org:service:WANDSLLinkConfig:%d\">", dslidx);
	strcat(resp, "<NewLinkType>");
	strcat(resp, linktype);
	strcat(resp, "</NewLinkType>");
	strcat(resp, "</u:GetDSLLinkInfoResponse>");

	free(linktype);

	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANDSLLinkConfigSetATMEncapsulation(struct upnphttp *h)
{
	struct NameValueParserData data;
	char *str, resp[128];
	int dslidx;
	MIB_CE_ATM_VC_T entry;

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	str = strstr(h->req_buf + h->req_contentoff, "WANDSLLinkConfig") + strlen("WANDSLLinkConfig:");
	dslidx = atoi(str);

	if((str = GetValueFromNameValueList(&data, "NewATMEncapsulation")))
	{
		if(mib_chain_get(MIB_ATM_VC_TBL, dslidx, (void *)&entry))
		{
			if(!strcasecmp(str, "VCMUX"))
				entry.encap = 0;
			else if(!strcasecmp(str, "LLC"))
				entry.encap = 1;

			mib_chain_update(MIB_ATM_VC_TBL, (void *)&entry, dslidx);
		}
	}

	sprintf(resp, "<u:SetATMEncapsulationResponse xmlns:u=\"urn:dslforum-org:service:WANDSLLinkConfig:%d\"></u:SetATMEncapsulationResponse>", dslidx);

	syslog(LOG_ERR, "UPNP ERR: Set ATMEncapsulation Length = %d", strlen(resp));

	ClearNameValueList(&data);
  BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANDSLLinkConfigGetATMEncapsulation(struct upnphttp *h)
{
	char *str, *atmencap, resp[192];
	int dslidx;
	MIB_CE_ATM_VC_T entry;

	str = strstr(h->req_buf + h->req_contentoff, "WANDSLLinkConfig") + strlen("WANDSLLinkConfig:");
	dslidx = atoi(str);

	if(mib_chain_get(MIB_ATM_VC_TBL, dslidx, (void *)&entry))
	{
		switch(entry.encap)
		{
			case 0:
				atmencap = strdup("VCMUX");
				break;
			case 1:
				atmencap = strdup("LLC");
				break;
			default:
				atmencap = strdup("Unknown type");
				break;
		}
	}
	else
		atmencap = strdup("Get chain record failed!");

	sprintf(resp, "<u:GetATMEncapsulationResponse xmlns:u=\"urn:dslforum-org:service:WANDSLLinkConfig:%d\">", dslidx);
	strcat(resp, "<NewATMEncapsulation>");
	strcat(resp, atmencap);
	strcat(resp, "</NewATMEncapsulation>");
	strcat(resp, "</u:GetATMEncapsulationResponse>");

	free(atmencap);
  BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANIPConnectionGetStatusInfo(struct upnphttp * h)
{
	static const char resp[] =
		"<u:GetStatusInfoResponse "
		"xmlns:u=\"urn:dslforum-org:service:WANIPConnection:1\">\n"
		"<NewConnectionStatus>Connected</NewConnectionStatus>\n"
		"<NewLastConnectionError>ERROR_NONE</NewLastConnectionError>\n"
		"<NewUptime>%ld</NewUptime>\n"
		"</u:GetStatusInfoResponse>\n";

	char body[512];
	int bodylen;
	time_t uptime;

	uptime = (time(NULL) - startup_time);
	bodylen = snprintf(body, sizeof(body), resp, (long)uptime);
	BuildSendAndCloseSoapResp(h, body, bodylen);
}

static void WANIPConnectionGetConnectionTypeInfo(struct upnphttp *h)
{
		static const char resp[] =
		"<u:GetConnectionTypeInfoResponse "
		"xmlns:u=\"urn:dslforum-org:service:WANIPConnection:1\">\n"
		"<NewConnectionType>IP_Routed</NewConnectionType>\n"
		"<NewPossibleConnectionTypes>IP_Routed</NewPossibleConnectionTypes>\n"
		"</u:GetConnectionTypeInfoResponse>\n";
	BuildSendAndCloseSoapResp(h, resp, sizeof(resp)-1);
#if 0
	char *str, *cntype, resp[256];
	int dslidx, i, totalentry, ipc = -1;
	MIB_CE_ATM_VC_T entry;
	bool isfound = false;

	str = strstr(h->req_buf + h->req_contentoff, "WANIPConnection") + strlen("WANIPConnection:");
	dslidx = atoi(str);

	totalentry = mib_chain_total(MIB_ATM_VC_TBL);

	for( i = 0; i < totalentry; i++ )
	{
		if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entry))
			break;

		if( entry.cmode != ADSL_PPPoE && entry.cmode != ADSL_PPPoA )
			ipc++;

		if( ipc == dslidx )
		{
			isfound = true;
			break;
		}
	}

	if( isfound && mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entry) )
	{
		switch(entry.cmode)
		{
			case ADSL_BR1483:
				cntype = strdup("IP_Bridged");
				break;
			case ADSL_MER1483:
				cntype = strdup("IP_Routed");
				break;
			case ADSL_RT1483:
				cntype = strdup("IP_Routed");
				break;
			default:
				cntype = strdup("Unconfigured");
				break;
		}
	}
	else
		cntype = strdup("Get mib chain record failed!");

	sprintf(resp, "<u:GetConnectionTypeInfoResponse xmlns:u=\"urn:dslforum-org:service:WANIPConnection:%d\">", dslidx);
	strcat(resp, "<NewConnectionType>");
	strcat(resp, cntype);
	strcat(resp, "</NewConnectionType>");
	strcat(resp, "<NewPossibleConnectionTypes>");
	strcat(resp, cntype);
	strcat(resp, "</NewPossibleConnectionTypes>");
	strcat(resp, "</u:GetConnectionTypeInfoResponse>");

	free(cntype);

	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
#endif
}

static void WANIPConnectionGetNATRSIPStatus(struct upnphttp *h)
{
	static const char resp[] =
		"<u:GetNATRSIPStatusResponse "
		"xmlns:u=\"urn:dslforum-org:service:WANIPConnection:1\">\n"
		"<NewRSIPAvailable>0</NewRSIPAvailable>\n"
		"<NewNATEnabled>1</NewNATEnabled>\n"
		"</u:GetNATRSIPStatusResponse>\n";

	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANIPConnectionGetExternalIPAddress(struct upnphttp *h)
{
	static const char resp[] =
		"<u:GetExternalIPAddressResponse "
		"xmlns:u=\"urn:dslforum-org:service:WANIPConnection:1\">\n"
		"<NewExternalIPAddress>%s</NewExternalIPAddress>\n"
		"</u:GetExternalIPAddressResponse>\n";

	char body[512];
	int bodylen;
	char ext_ip_addr[INET_ADDRSTRLEN];

#ifndef MULTIPLE_EXTERNAL_IP
	if(use_ext_ip_addr[0])
	{
		strncpy(ext_ip_addr, use_ext_ip_addr, INET_ADDRSTRLEN);
	}
	else if(getifaddr(ext_if_name, ext_ip_addr, INET_ADDRSTRLEN) < 0)
	{
		syslog(LOG_ERR, "Failed to get ip address for interface %s",
			ext_if_name);
		strncpy(ext_ip_addr, "0.0.0.0", INET_ADDRSTRLEN);
	}
#else
	int i;
	strncpy(ext_ip_addr, "0.0.0.0", INET_ADDRSTRLEN);
	for(i = 0; i<n_lan_addr; i++)
	{
		if( (h->clientaddr.s_addr & lan_addr[i].mask.s_addr)
		   == (lan_addr[i].addr.s_addr & lan_addr[i].mask.s_addr))
		{
			strncpy(ext_ip_addr, lan_addr[i].ext_ip_str, INET_ADDRSTRLEN);
			break;
		}
	}
#endif
	bodylen = snprintf(body, sizeof(body), resp, ext_ip_addr);
	BuildSendAndCloseSoapResp(h, body, bodylen);
}

static void WANIPConnectionAddPortMapping(struct upnphttp *h)
{
	int r;

	static const char resp[] =
		"<u:AddPortMappingResponse "
		"xmlns:u=\"urn:dslforum-org:service:WANIPConnection:1\"/>\n";

	struct NameValueParserData data;
	char * int_ip, * int_port, * ext_port, * protocol, * desc;
	char * leaseduration;
	unsigned short iport, eport;

	struct hostent *hp; /* getbyhostname() */
	char ** ptr; /* getbyhostname() */
	struct in_addr result_ip;/*unsigned char result_ip[16];*/ /* inet_pton() */

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	int_ip = GetValueFromNameValueList(&data, "NewInternalClient");

	if (!int_ip)
	{
		ClearNameValueList(&data);
		SoapError(h, 402, "Invalid Args");
		return;
	}

	/* if ip not valid assume hostname and convert */
	if (inet_pton(AF_INET, int_ip, &result_ip) <= 0)
	{
		hp = gethostbyname(int_ip);
		if(hp && hp->h_addrtype == AF_INET)
		{
			for(ptr = hp->h_addr_list; ptr && *ptr; ptr++)
			{
				int_ip = inet_ntoa(*((struct in_addr *) *ptr));
				result_ip = *((struct in_addr *) *ptr);
				/* TODO : deal with more than one ip per hostname */
				break;
			}
		}
		else
		{
			syslog(LOG_ERR, "Failed to convert hostname '%s' to ip address", int_ip);
			ClearNameValueList(&data);
			SoapError(h, 402, "Invalid Args");
			return;
		}
	}

	/* check if NewInternalAddress is the client address */
	if(GETFLAG(SECUREMODEMASK))
	{
		if(h->clientaddr.s_addr != result_ip.s_addr)
		{
			syslog(LOG_INFO, "Client %s tried to redirect port to %s",
			       inet_ntoa(h->clientaddr), int_ip);
			ClearNameValueList(&data);
			SoapError(h, 718, "ConflictInMappingEntry");
			return;
		}
	}

	int_port = GetValueFromNameValueList(&data, "NewInternalPort");
	ext_port = GetValueFromNameValueList(&data, "NewExternalPort");
	protocol = GetValueFromNameValueList(&data, "NewProtocol");
	desc = GetValueFromNameValueList(&data, "NewPortMappingDescription");
	leaseduration = GetValueFromNameValueList(&data, "NewLeaseDuration");

	if (!int_port || !ext_port || !protocol)
	{
		ClearNameValueList(&data);
		SoapError(h, 402, "Invalid Args");
		return;
	}

	eport = (unsigned short)atoi(ext_port);
	iport = (unsigned short)atoi(int_port);

	if(leaseduration && atoi(leaseduration)) {
		syslog(LOG_WARNING, "NewLeaseDuration=%s", leaseduration);
	}

	syslog(LOG_INFO, "AddPortMapping: external port %hu to %s:%hu protocol %s for: %s",
			eport, int_ip, iport, protocol, desc);

	r = upnp_redirect(eport, int_ip, iport, protocol, desc);

	ClearNameValueList(&data);

	/* possible error codes for AddPortMapping :
	 * 402 - Invalid Args
	 * 501 - Action Failed
	 * 715 - Wildcard not permited in SrcAddr
	 * 716 - Wildcard not permited in ExtPort
	 * 718 - ConflictInMappingEntry
	 * 724 - SamePortValuesRequired
     * 725 - OnlyPermanentLeasesSupported
             The NAT implementation only supports permanent lease times on
             port mappings
     * 726 - RemoteHostOnlySupportsWildcard
             RemoteHost must be a wildcard and cannot be a specific IP
             address or DNS name
     * 727 - ExternalPortOnlySupportsWildcard
             ExternalPort must be a wildcard and cannot be a specific port value */

	switch(r)
	{
	case 0:	/* success */
	BuildSendAndCloseSoapResp(h, resp, sizeof(resp)-1);
		break;
	case -2:	/* already redirected */
	case -3:	/* not permitted */
		SoapError(h, 718, "ConflictInMappingEntry");
		break;
	default:
		SoapError(h, 501, "ActionFailed");
	}
}

static void WANIPConnectionDeletePortMapping(struct upnphttp *h)
{
  int r;

  static const char resp[] =
   "<u:DeletePortMappingResponse "
		"xmlns:u=\"urn:dslforum-org:service:WANIPConnection:1\">\n"
		"</u:DeletePortMappingResponse>\n";

  struct NameValueParserData data;
	const char * r_host, * ext_port, * protocol;
  unsigned short eport;

  ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
  r_host = GetValueFromNameValueList(&data, "NewRemoteHost");
  ext_port = GetValueFromNameValueList(&data, "NewExternalPort");
  protocol = GetValueFromNameValueList(&data, "NewProtocol");

	if(!ext_port || !protocol)
	{
		ClearNameValueList(&data);
		SoapError(h, 402, "Invalid Args");
		return;
	}

  eport = (unsigned short)atoi(ext_port);

	/* TODO : if in secure mode, check the IP */

	syslog(LOG_INFO, "DeletePortMapping: external port: %hu, protocol: %s",
		eport, protocol);

  r = upnp_delete_redirection(eport, protocol);

  if(r<0)
  {
   SoapError(h, 714, "NoSuchEntryInArray");
  }
  else
  {
   BuildSendAndCloseSoapResp(h, resp, sizeof(resp)-1);
  }

	ClearNameValueList(&data);
}

static void WANIPConnectionGetGenericPortMappingEntry(struct upnphttp *h)
{
	int r;

	static const char resp[] =
		"<u:GetGenericPortMappingEntryResponse "
		"xmlns:u=\"urn:dslforum-org:service:WANIPConnection:1\">\n"
		"<NewRemoteHost></NewRemoteHost>\n"
		"<NewExternalPort>%u</NewExternalPort>\n"
		"<NewProtocol>%s</NewProtocol>\n"
		"<NewInternalPort>%u</NewInternalPort>\n"
		"<NewInternalClient>%s</NewInternalClient>\n"
		"<NewEnabled>1</NewEnabled>\n"
		"<NewPortMappingDescription>%s</NewPortMappingDescription>\n"
		"<NewLeaseDuration>0</NewLeaseDuration>\n"
		"</u:GetGenericPortMappingEntryResponse>\n";

	int index = 0;
	unsigned short eport, iport;
	const char * m_index;
	char protocol[4], iaddr[32];
	char desc[64];
	struct NameValueParserData data;

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	m_index = GetValueFromNameValueList(&data, "NewPortMappingIndex");

	if(!m_index)
	{
		ClearNameValueList(&data);
		SoapError(h, 402, "Invalid Args");
		return;
	}

	index = (int)atoi(m_index);

	syslog(LOG_INFO, "GetGenericPortMappingEntry: index=%d", index);

	r = upnp_get_redirection_infos_by_index(index, &eport, protocol, &iport,
                                            iaddr, sizeof(iaddr),
	                                        desc, sizeof(desc));

	if(r < 0)
	{
		SoapError(h, 713, "SpecifiedArrayIndexInvalid");
	}
	else
	{
		int bodylen;
		char body[2048];
		bodylen = snprintf(body, sizeof(body), resp, (unsigned int)eport,
		                   protocol, (unsigned int)iport, iaddr, desc);
		BuildSendAndCloseSoapResp(h, body, bodylen);
	}

	ClearNameValueList(&data);

}

static void WANIPConnectionGetSpecificPortMappingEntry(struct upnphttp *h)
{
	int r;

	static const char resp[] =
		"<u:GetSpecificPortMappingEntryResponse "
		"xmlns:u=\"urn:dslforum-org:service:WANIPConnection:1\">\n"
		"<NewInternalPort>%u</NewInternalPort>\n"
		"<NewInternalClient>%s</NewInternalClient>\n"
		"<NewEnabled>1</NewEnabled>\n"
		"<NewPortMappingDescription>%s</NewPortMappingDescription>\n"
		"<NewLeaseDuration>0</NewLeaseDuration>\n"
		"</u:GetSpecificPortMappingEntryResponse>\n";

	char body[2048];
	int bodylen;
	struct NameValueParserData data;
	const char * r_host, * ext_port, * protocol;
	unsigned short eport, iport;
	char int_ip[32];
	char desc[64];

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	r_host = GetValueFromNameValueList(&data, "NewRemoteHost");
	ext_port = GetValueFromNameValueList(&data, "NewExternalPort");
	protocol = GetValueFromNameValueList(&data, "NewProtocol");

	if(!ext_port || !protocol)
	{
		ClearNameValueList(&data);
		SoapError(h, 402, "Invalid Args");
		return;
	}

	eport = (unsigned short)atoi(ext_port);

	r = upnp_get_redirection_infos(eport, protocol, &iport,
	                               int_ip, sizeof(int_ip),
	                               desc, sizeof(desc));

	if(r < 0)
	{
		SoapError(h, 714, "NoSuchEntryInArray");
	}
	else
	{
		syslog(LOG_INFO, "GetSpecificPortMappingEntry: rhost='%s' %s %s found => %s:%u desc='%s'",
		       r_host, ext_port, protocol, int_ip, (unsigned int)iport, desc);
		bodylen = snprintf(body, sizeof(body), resp, (unsigned int)iport, int_ip, desc);
		BuildSendAndCloseSoapResp(h, body, bodylen);
	}

	ClearNameValueList(&data);

}

static void WANIPConnectionQueryStateVariable(struct upnphttp *h)
{
	static const char resp[] =
		"<u:QueryStateVariableResponse "
		"xmlns:u=\"urn:dslforum-org:control-1-0\">\n"
		"<return>%s</return>\n"
		"</u:QueryStateVariableResponse>\n";

	char body[2048];
	int bodylen;
	struct NameValueParserData data;
	const char * var_name;

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	/*var_name = GetValueFromNameValueList(&data, "QueryStateVariable"); */
	/*var_name = GetValueFromNameValueListIgnoreNS(&data, "varName");*/
	if(!(var_name = GetValueFromNameValueList(&data, "varName")))  // For compatibility issue of Windows XP
		var_name = GetValueFromNameValueList(&data, "m:varName");

	/*syslog(LOG_INFO, "QueryStateVariable(%.40s)", var_name); */

	if(!var_name)
	{
		SoapError(h, 402, "Invalid Args");
	}
	else if(strcasecmp(var_name, "ConnectionStatus") == 0)
	{
	bodylen = snprintf(body, sizeof(body), resp, "Connected");
	BuildSendAndCloseSoapResp(h, body, bodylen);
}
#if 0
	/* not usefull */
	else if(strcasecmp(var_name, "ConnectionType") == 0)
	{
		bodylen = snprintf(body, sizeof(body), resp, "IP_Routed");
		BuildSendAndCloseSoapResp(h, body, bodylen);
	}
	else if(strcasecmp(var_name, "LastConnectionError") == 0)
	{
		bodylen = snprintf(body, sizeof(body), resp, "ERROR_NONE");
		BuildSendAndCloseSoapResp(h, body, bodylen);
	}
#endif
	else if(strcasecmp(var_name, "PortMappingNumberOfEntries") == 0)
	{
		int r = 0, index = 0;
		unsigned short eport, iport;
		char protocol[4], iaddr[32], desc[64];
		char strindex[10];

		do
		{
			protocol[0] = '\0'; iaddr[0] = '\0'; desc[0] = '\0';

			r = upnp_get_redirection_infos_by_index(index, &eport, protocol, &iport,
													iaddr, sizeof(iaddr),
													desc, sizeof(desc));
			index++;
		}
		while(r==0);

		snprintf(strindex, sizeof(strindex), "%i", index - 1);
		bodylen = snprintf(body, sizeof(body), resp, strindex);
		BuildSendAndCloseSoapResp(h, body, bodylen);
	}
	else
	{
		syslog(LOG_NOTICE, "QueryStateVariable: Unknown: %s", var_name?var_name:"");
		SoapError(h, 404, "Invalid Var");
	}

	ClearNameValueList(&data);}

static void WANIPConnectionGetInfo(struct upnphttp *h)
{
	char resp[2048], up_time[12], *idxstr, *cntype, *name;
	struct sysinfo info;
	int dslidx, totalentry, ipc = -1, i;
	MIB_CE_ATM_VC_T entry;
	bool isfound = false;

	idxstr = strstr(h->req_buf + h->req_contentoff, "WANIPConnection") + strlen("WANIPConnection:");
	dslidx = atoi(idxstr);

	totalentry = mib_chain_total(MIB_ATM_VC_TBL);

	for( i = 0; i < totalentry; i++ )
	{
		if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entry))
			break;

		if( entry.cmode != ADSL_PPPoE && entry.cmode != ADSL_PPPoA )
			ipc++;

		if( ipc == dslidx )
		{
			isfound = true;
			break;
		}
	}

	if(isfound)
	{
		switch(entry.cmode)
		{
			case ADSL_RT1483:
				cntype = strdup("IP_Routed");
				name = strdup("1483 Routed");
				break;
			case ADSL_MER1483:
				cntype = strdup("IP_Routed");
				name = strdup("1483 Mer");
				break;
			case ADSL_BR1483:
				cntype = strdup("IP_Bridged");
				name = strdup("1483 Bridged");
				break;
			case ADSL_PPPoA:
				cntype = strdup("Unconfigured");
				name = strdup("1483 PPPoA");
				break;
			case ADSL_PPPoE:
				cntype = strdup("Unconfigured");
				name = strdup("1483 PPPoE");
				break;
			default:
				cntype = strdup("Unconfigured");
				name = strdup("Unknown");
				break;
		}
	}
	else
	{
		cntype = strdup("Get mib chain record failed!");
		name = strdup("Get mib chain record failed!");
	}

	strcpy(resp, "<u:GetInfoResponse xmlns:u=\"urn:dslforum-org:service:WANIPConnection:1\">");
	strcat(resp, "<NewEnable>");
	strcat(resp, "1");
	strcat(resp, "</NewEnable>");
	strcat(resp, "<NewConnectionType>");
	strcat(resp, cntype);
	strcat(resp, "</NewConnectionType>");
	strcat(resp, "<NewPossibleConnectionTypes>");
	strcat(resp, cntype);
	strcat(resp, "</NewPossibleConnectionTypes>");
	strcat(resp, "<NewConnectionStatus>");
	strcat(resp, "Connected");
	strcat(resp, "</NewConnectionStatus>");
	strcat(resp, "<NewName>");
	strcat(resp, name);
	strcat(resp, "</NewName>");

	sysinfo(&info);
	sprintf(up_time, "%d", (int)info.uptime);

	strcat(resp, "<NewUptime>");
	strcat(resp, up_time);
	strcat(resp, "</NewUptime>");
	strcat(resp, "<NewLastConnectionError>");
	strcat(resp, "ERROR_NONE");
	strcat(resp, "</NewLastConnectionError>");
	strcat(resp, "<NewAutoDisconnectTime>");
	strcat(resp, "0");
	strcat(resp, "</NewAutoDisconnectTime>");
	strcat(resp, "<NewIdleDisconnectTime>");
	strcat(resp, "0");
	strcat(resp, "</NewIdleDisconnectTime>");
	strcat(resp, "<NewWarnDisconnectDelay>");
	strcat(resp, "0");
	strcat(resp, "</NewWarnDisconnectDelay>");
	strcat(resp, "<NewRSIPAvailable>");
	strcat(resp, "0");
	strcat(resp, "</NewRSIPAvailable>");
	strcat(resp, "<NewNATEnabled>");
	strcat(resp, "1");
	strcat(resp, "</NewNATEnabled>");
	strcat(resp, "<NewExternalIPAddress>");
	strcat(resp, use_ext_ip_addr);
	strcat(resp, "</NewExternalIPAddress>");
	strcat(resp, "<NewSubnetMask>");
	strcat(resp, "");
	strcat(resp, "</NewSubnetMask>");
	strcat(resp, "<NewAddressingType>");
	strcat(resp, "");
	strcat(resp, "</NewAddressingType>");
	strcat(resp, "<NewDefaultGateway>");
	strcat(resp, "");
	strcat(resp, "</NewDefaultGateway>");
	strcat(resp, "<NewMACAddress>");
	strcat(resp, "");
	strcat(resp, "</NewMACAddress>");
	strcat(resp, "<NewMACAddressOverride>");
	strcat(resp, "0");
	strcat(resp, "</NewMACAddressOverride>");
	strcat(resp, "<NewMaxMTUSize>");
	strcat(resp, "");
	strcat(resp, "</NewMaxMTUSize>");
	strcat(resp, "<NewDNSEnabled>");
	strcat(resp, "0");
	strcat(resp, "</NewDNSEnabled>");
  strcat(resp, "<NewDNSOverrideAllowed>");
  strcat(resp, "0");
  strcat(resp, "</NewDNSOverrideAllowed>");
	strcat(resp, "<NewDNSServers>");
	strcat(resp, "");
	strcat(resp, "</NewDNSServers>");
	strcat(resp, "<NewConnectionTrigger>");
	strcat(resp, "OnDemand");
	strcat(resp, "</NewConnectionTrigger>");
	strcat(resp, "<NewRouteProtocolRx>");
	strcat(resp, "Off");
	strcat(resp, "</NewRouteProtocolRx>");
	strcat(resp, "</u:GetInfoResponse>");

	free(name);
	free(cntype);

	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANIPConnectionSetConnectionType(struct upnphttp *h)
{
	struct NameValueParserData data;
	char *idxstr, *str, resp[160];
	int dslidx, totalentry, i, ipc = -1;
	MIB_CE_ATM_VC_T entry;
	bool isfound = false;

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	idxstr = strstr(h->req_buf + h->req_contentoff, "WANIPConnection") + strlen("WANIPConnection:");
	dslidx = atoi(idxstr);

	if((str = GetValueFromNameValueList(&data, "NewConnectionType")))
	{
		totalentry = mib_chain_total(MIB_ATM_VC_TBL);

		for( i = 0; i < totalentry; i++ )
		{
			if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entry))
				break;

			if( entry.cmode != ADSL_PPPoE && entry.cmode != ADSL_PPPoA )
				ipc++;

			if( ipc == dslidx )
			{
				isfound = true;
				break;
			}
		}

		if(isfound)
		{
			if( !strcasecmp(str, "IP_Routed") )
				entry.cmode = ADSL_MER1483;
			else if( !strcasecmp(str, "IP_Bridged") )
				entry.cmode = ADSL_BR1483;
			else
				isfound = false;
		}
		else
			syslog(LOG_ERR, "UPNP ERR: ConnectionType is invalid!");

		if(isfound)
			mib_chain_update(MIB_ATM_VC_TBL, (void *)&entry, i);
	}

	sprintf(resp, "<u:SetConnectionTypeResponse xmlns:u=\"urn:dslforum-org:service:WANPPPConnection:%d\"></u:SetConnectionTypeResponse>", dslidx);

	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANIPConnectionSetEnable(struct upnphttp *h)
{
	struct NameValueParserData data;
	char *str, resp[128];
	int dslidx, totalentry, ipc = -1, i;
	bool isfound = false;
	MIB_CE_ATM_VC_T entry;

	totalentry = mib_chain_total(MIB_ATM_VC_TBL);
	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	str = strstr(h->req_buf + h->req_contentoff, "WANIPConnection") + strlen("WANIPConnection:");
	dslidx = atoi(str);

	if((str = GetValueFromNameValueList(&data, "NewEnable")))
	{
		for( i = 0; i < totalentry; i++ )
		{
			if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entry))
				break;

			if( entry.cmode != ADSL_PPPoE && entry.cmode != ADSL_PPPoA )
				ipc++;

			if( ipc == dslidx )
			{
				isfound = true;
				break;
			}
		}
	}
	else
	{
		syslog(LOG_ERR, "UPNP ERR: Invalid Arguments!");
		SoapError(h, 402, "Invalid Args: No Args!");
	}

	if(isfound)
	{
		entry.enable = atoi(str);
		mib_chain_update(MIB_ATM_VC_TBL, (void *)&entry, i);
		restartWAN();
		Commit(h, i);
	}

	sprintf(resp, "<u:SetEnableResponse xmlns:u=\"urn:dslforum-org:service:WANIPConnection:%d\"></u:SetEnableResponse>", dslidx);

  BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANPPPConnectionSetEnable(struct upnphttp *h)
{
	struct NameValueParserData data;
	char *str, resp[128];
	int dslidx, totalentry, pppc = -1, i;
	bool isfound = false;
	MIB_CE_ATM_VC_T entry;

	totalentry = mib_chain_total(MIB_ATM_VC_TBL);
	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	str = strstr(h->req_buf + h->req_contentoff, "WANPPPConnection") + strlen("WANPPPConnection:");
	dslidx = atoi(str);

	if((str = GetValueFromNameValueList(&data, "NewEnable")))
	{
		for( i = 0; i < totalentry; i++ )
		{
			if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entry))
				break;

			if( entry.cmode == ADSL_PPPoE || entry.cmode == ADSL_PPPoA )
				pppc++;

			if( pppc == dslidx )
			{
				isfound = true;
				break;
			}
		}
	}
	else
	{
		syslog(LOG_ERR, "UPNP ERR: Invalid Arguments!");
		SoapError(h, 402, "Invalid Args: No Args!");
	}

	if(isfound)
	{
		entry.enable = atoi(str);
		mib_chain_update(MIB_ATM_VC_TBL, (void *)&entry, i);
		restartWAN();
		Commit(h, i);
	}

	sprintf(resp, "<u:SetEnableResponse xmlns:u=\"urn:dslforum-org:service:WANPPPConnection:%d\"></u:SetEnableResponse>", dslidx);

	ClearNameValueList(&data);
  BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANPPPConnectionSetUserName(struct upnphttp *h)
{
	struct NameValueParserData data;
	char *idxstr, *str, resp[128];
	int dslidx, totalentry, i, pppc = -1;
	MIB_CE_ATM_VC_T entry;
	bool isfound = false;

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	idxstr = strstr(h->req_buf + h->req_contentoff, "WANPPPConnection") + strlen("WANPPPConnection:");
	dslidx = atoi(idxstr);

	if( (str = GetValueFromNameValueList(&data, "NewUserName")) && strlen(str) < 30 )
	{
		totalentry = mib_chain_total(MIB_ATM_VC_TBL);

		for( i = 0; i < totalentry; i++ )
		{
			if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entry))
				break;

			if( entry.cmode == ADSL_PPPoE || entry.cmode == ADSL_PPPoA )
				pppc++;

			if( pppc == dslidx )
			{
				isfound = true;
				break;
			}
		}

		if(isfound)
		{
			strcpy(entry.pppUsername, str);
			mib_chain_update(MIB_ATM_VC_TBL, (void *)&entry, i);
		}
	}
	else
		syslog(LOG_ERR, "UPNP ERR: UserName is invalid!");

	sprintf(resp, "<u:SetUserNameResponse xmlns:u=\"urn:dslforum-org:service:WANPPPConnection:%d\"></u:SetUserNameResponse>", dslidx);

	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANPPPConnectionGetUserName(struct upnphttp *h)
{
	char *str, resp[192];
	int dslidx, totalentry, i, pppc = -1;
	MIB_CE_ATM_VC_T entry;
	bool isfound = false;

	str = strstr(h->req_buf + h->req_contentoff, "WANPPPConnection") + strlen("WANPPPConnection:");
	dslidx = atoi(str);

	totalentry = mib_chain_total(MIB_ATM_VC_TBL);

	for( i = 0; i < totalentry; i++ )
	{
		if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entry))
			break;

		if( entry.cmode == ADSL_PPPoE || entry.cmode == ADSL_PPPoA )
			pppc++;

		if( pppc == dslidx )
		{
			isfound = true;
			break;
		}
	}

	if(isfound)
		str = strdup(entry.pppUsername);
	else
		str = strdup("Get user name failed!");

	sprintf(resp, "<u:GetUserNameResponse xmlns:u=\"urn:dslforum-org:service:WANPPPConnection:%d\">", dslidx);
	strcat(resp, "<NewUserName>");
	strcat(resp, str);
	strcat(resp, "</NewUserName>");
	strcat(resp, "</u:GetUserNameResponse>");

	free(str);

	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANPPPConnectionSetPassword(struct upnphttp *h)
{
	struct NameValueParserData data;
	char *idxstr, *str, resp[128];
	int dslidx, totalentry, i, pppc = -1;
	MIB_CE_ATM_VC_T entry;
	bool isfound = false;

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);

	idxstr = strstr(h->req_buf + h->req_contentoff, "WANPPPConnection") + strlen("WANPPPConnection:");
	dslidx = atoi(idxstr);

	if( (str = GetValueFromNameValueList(&data, "NewPassword")) && strlen(str) < 30 )
	{
		totalentry = mib_chain_total(MIB_ATM_VC_TBL);

		for( i = 0; i < totalentry; i++ )
		{
			if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entry))
				break;

			if( entry.cmode == ADSL_PPPoE || entry.cmode == ADSL_PPPoA )
				pppc++;

			if( pppc == dslidx )
			{
				isfound = true;
				break;
			}
		}

		if(isfound)
		{
			strcpy(entry.pppPassword, str);
			mib_chain_update(MIB_ATM_VC_TBL, (void *)&entry, i);
		}
	}
	else
		syslog(LOG_ERR, "UPNP ERR: Password is invalid!");

	sprintf(resp, "<u:SetPasswordResponse xmlns:u=\"urn:dslforum-org:service:WANPPPConnection:%d\"></u:SetPasswordResponse>", dslidx);

	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANPPPConnectionGetStatusInfo(struct upnphttp *h)
{
	char *str, resp[256];
	time_t uptime;
	int dslidx, totalentry, i, pppc = -1;
	MIB_CE_ATM_VC_T entry;
	bool isfound = false;

	str = strstr(h->req_buf + h->req_contentoff, "WANPPPConnection") + strlen("WANPPPConnection:");
	dslidx = atoi(str);

	totalentry = mib_chain_total(MIB_ATM_VC_TBL);

	for( i = 0; i < totalentry; i++ )
	{
		if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entry))
			break;

		if( entry.cmode == ADSL_PPPoE || entry.cmode == ADSL_PPPoA )
			pppc++;

		if( pppc == dslidx )
		{
			isfound = true;
			break;
		}
	}

	uptime = (time(NULL) - startup_time);

	sprintf(resp, "<u:GetStatusInfo xmlns:u=\"urn:dslforum-org:service:WANPPPConnection:%d\">", dslidx);
	strcat(resp, "<NewConnectionStatus>");
	strcat(resp, "Connected");
	strcat(resp, "</NewConnectionStatus>");
	strcat(resp, "<NewLastConnectionError>");
	strcat(resp, "ERROR_NONE");
	strcat(resp, "</NewLastConnectionError>");
	strcat(resp, "<NewUptime>");
	sprintf(resp, "%s%ld", resp, uptime);
	strcat(resp, "</NewUptime>");
	strcat(resp, "</u:GetStatusInfo>");

	syslog(LOG_ERR, "UPNP ERR: GetUserName: %d", strlen(resp));
	free(str);

	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

static void WANPPPConnectionRequestConnection(struct upnphttp *h)
{
	char *str, resp[256];
	int dslidx, totalentry, i, pppc = -1;
	MIB_CE_ATM_VC_T entry;
	bool isfound = false;

	struct data_to_pass_st msg;
	char buff[256];
	unsigned int cflag[MAX_PPP_NUM]={0}, flag, inf;
	FILE *fp;

	str = strstr(h->req_buf + h->req_contentoff, "WANPPPConnection") + strlen("WANPPPConnection:");
	dslidx = atoi(str);

	totalentry = mib_chain_total(MIB_ATM_VC_TBL);

	for( i = 0; i < totalentry; i++ )
	{
		if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entry))
			break;

		if( entry.cmode == ADSL_PPPoE || entry.cmode == ADSL_PPPoA )
			pppc++;

		if( pppc == dslidx )
		{
			isfound = true;
			break;
		}
	}

	// Added by Jenny, for PPP connecting/disconnecting
	for (i=0; i<MAX_PPP_NUM; i++)
	{
		char tp[10];

		sprintf(tp, "ppp%d", i);
		if (find_ppp_from_conf(tp))
		{
			if ((fp = fopen("/tmp/ppp_up_log", "r")) != NULL)
			{
				while ( fgets(buff, sizeof(buff), fp) != NULL )
				{
					if(sscanf(buff, "%d %d", &inf, &flag) != 2)
						break;
					else
					{
						if (inf == i)
							cflag[i] = flag;
					}
				}
				fclose(fp);
			}

			if (cflag[i])
			{
				snprintf(msg.data, BUF_SIZE, "spppctl up %u", i);
				usleep(3000000);
				//printf(0x00000004, "%s\n", msg.data);
				write_to_pppd(&msg);
			}
		}
	}

	sprintf(resp, "<u:RequestConnection xmlns:u=\"urn:dslforum-org:service:WANPPPConnection:%d\"></u:RequestConnection>", dslidx);
	free(str);

	BuildSendAndCloseSoapResp(h, resp, strlen(resp));
}

#else // TR_064
static void GetConnectionTypeInfo(struct upnphttp * h)
{
	static const char resp[] =
		"<u:GetConnectionTypeInfoResponse "
		"xmlns:u=\"urn:schemas-upnp-org:service:WANIPConnection:1\">\n"
		"<NewConnectionType>IP_Routed</NewConnectionType>\n"
		"<NewPossibleConnectionTypes>IP_Routed</NewPossibleConnectionTypes>\n"
		"</u:GetConnectionTypeInfoResponse>\n";
	BuildSendAndCloseSoapResp(h, resp, sizeof(resp)-1);
}

static void GetTotalBytesSent(struct upnphttp * h)
{
	int r;

	static const char resp[] =
		"<u:GetTotalBytesSentResponse "
		"xmlns:u=\"urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1\">\n"
		"<NewTotalBytesSent>%lu</NewTotalBytesSent>\n"
		"</u:GetTotalBytesSentResponse>\n";

	char body[2048];
	int bodylen;
	struct ifdata data;

	r = getifstats(ext_if_name, &data);
	bodylen = snprintf(body, sizeof(body), resp, r<0?0:data.obytes);
	BuildSendAndCloseSoapResp(h, body, bodylen);
}

static void GetTotalBytesReceived(struct upnphttp * h)
{
	int r;

	static const char resp[] =
		"<u:GetTotalBytesReceivedResponse "
		"xmlns:u=\"urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1\">\n"
		"<NewTotalBytesReceived>%lu</NewTotalBytesReceived>\n"
		"</u:GetTotalBytesReceivedResponse>\n";

	char body[2048];
	int bodylen;
	struct ifdata data;

	r = getifstats(ext_if_name, &data);
	bodylen = snprintf(body, sizeof(body), resp, r<0?0:data.ibytes);
	BuildSendAndCloseSoapResp(h, body, bodylen);
}

static void GetTotalPacketsSent(struct upnphttp * h)
{
	int r;

	static const char resp[] =
		"<u:GetTotalPacketsSentResponse "
		"xmlns:u=\"urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1\">\n"
		"<NewTotalPacketsSent>%lu</NewTotalPacketsSent>\n"
		"</u:GetTotalPacketsSentResponse>\n";

	char body[2048];
	int bodylen;
	struct ifdata data;

	r = getifstats(ext_if_name, &data);
	bodylen = snprintf(body, sizeof(body), resp, r<0?0:data.opackets);
	BuildSendAndCloseSoapResp(h, body, bodylen);
}

static void GetTotalPacketsReceived(struct upnphttp * h)
{
	int r;

	static const char resp[] =
		"<u:GetTotalPacketsReceivedResponse "
		"xmlns:u=\"urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1\">\n"
		"<NewTotalPacketsReceived>%lu</NewTotalPacketsReceived>\n"
		"</u:GetTotalPacketsReceivedResponse>\n";

	char body[2048];
	int bodylen;
	struct ifdata data;

	r = getifstats(ext_if_name, &data);
	bodylen = snprintf(body, sizeof(body), resp, r<0?0:data.ipackets);
	BuildSendAndCloseSoapResp(h, body, bodylen);
}

static void GetCommonLinkProperties(struct upnphttp * h)
{
	static const char resp[] =
		"<u:GetCommonLinkPropertiesResponse "
		"xmlns:u=\"urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1\">\n"
		/*"<NewWANAccessType>DSL</NewWANAccessType>"*/
		"<NewWANAccessType>Cable</NewWANAccessType>\n"
		"<NewLayer1UpstreamMaxBitRate>%lu</NewLayer1UpstreamMaxBitRate>\n"
		"<NewLayer1DownstreamMaxBitRate>%lu</NewLayer1DownstreamMaxBitRate>\n"
		"<NewPhysicalLinkStatus>Up</NewPhysicalLinkStatus>\n"
		"</u:GetCommonLinkPropertiesResponse>\n";

	char body[2048];
	int bodylen;
	struct ifdata data;

	if((downstream_bitrate == 0) || (upstream_bitrate == 0))
	{
		if(getifstats(ext_if_name, &data) >= 0)
		{
			if(downstream_bitrate == 0) downstream_bitrate = data.baudrate;
			if(upstream_bitrate == 0) upstream_bitrate = data.baudrate;
		}
	}
	bodylen = snprintf(body, sizeof(body), resp,
		upstream_bitrate, downstream_bitrate);
	BuildSendAndCloseSoapResp(h, body, bodylen);
}

static void GetStatusInfo(struct upnphttp * h)
{
	static const char resp[] =
		"<u:GetStatusInfoResponse "
		"xmlns:u=\"urn:schemas-upnp-org:service:WANIPConnection:1\">\n"
		"<NewConnectionStatus>Connected</NewConnectionStatus>\n"
		"<NewLastConnectionError>ERROR_NONE</NewLastConnectionError>\n"
		"<NewUptime>%ld</NewUptime>\n"
		"</u:GetStatusInfoResponse>\n";

	char body[512];
	int bodylen;
	time_t uptime;

	uptime = (time(NULL) - startup_time);
	bodylen = snprintf(body, sizeof(body), resp, (long)uptime);
	BuildSendAndCloseSoapResp(h, body, bodylen);
}

static void GetNATRSIPStatus(struct upnphttp * h)
{
	static const char resp[] =
		"<u:GetNATRSIPStatusResponse "
		"xmlns:u=\"urn:schemas-upnp-org:service:WANIPConnection:1\">\n"
		"<NewRSIPAvailable>0</NewRSIPAvailable>\n"
		"<NewNATEnabled>1</NewNATEnabled>\n"
		"</u:GetNATRSIPStatusResponse>\n";

	BuildSendAndCloseSoapResp(h, resp, sizeof(resp)-1);
}

static void GetExternalIPAddress(struct upnphttp * h)
{
	static const char resp[] =
		"<u:GetExternalIPAddressResponse "
		"xmlns:u=\"urn:schemas-upnp-org:service:WANIPConnection:1\">\n"
		"<NewExternalIPAddress>%s</NewExternalIPAddress>\n"
		"</u:GetExternalIPAddressResponse>\n";

	char body[512];
	int bodylen;
	char ext_ip_addr[INET_ADDRSTRLEN];

#ifndef MULTIPLE_EXTERNAL_IP
	if(use_ext_ip_addr[0])
	{
		strncpy(ext_ip_addr, use_ext_ip_addr, INET_ADDRSTRLEN);
	}
	else if(getifaddr(ext_if_name, ext_ip_addr, INET_ADDRSTRLEN) < 0)
	{
		syslog(LOG_ERR, "Failed to get ip address for interface %s",
			ext_if_name);
		strncpy(ext_ip_addr, "0.0.0.0", INET_ADDRSTRLEN);
	}
#else
	int i;
	strncpy(ext_ip_addr, "0.0.0.0", INET_ADDRSTRLEN);
	for(i = 0; i<n_lan_addr; i++)
	{
		if( (h->clientaddr.s_addr & lan_addr[i].mask.s_addr)
		   == (lan_addr[i].addr.s_addr & lan_addr[i].mask.s_addr))
		{
			strncpy(ext_ip_addr, lan_addr[i].ext_ip_str, INET_ADDRSTRLEN);
			break;
		}
	}
#endif
	bodylen = snprintf(body, sizeof(body), resp, ext_ip_addr);
	BuildSendAndCloseSoapResp(h, body, bodylen);
}

static void AddPortMapping(struct upnphttp * h)
{
	int r;

	static const char resp[] =
		"<u:AddPortMappingResponse "
		"xmlns:u=\"urn:schemas-upnp-org:service:WANIPConnection:1\"/>\n";

	struct NameValueParserData data;
	char * int_ip, * int_port, * ext_port, * protocol, * desc;
	char * leaseduration;
	unsigned short iport, eport;

	struct hostent *hp; /* getbyhostname() */
	char ** ptr; /* getbyhostname() */
	struct in_addr result_ip;/*unsigned char result_ip[16];*/ /* inet_pton() */

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	int_ip = GetValueFromNameValueList(&data, "NewInternalClient");

	if (!int_ip)
	{
		ClearNameValueList(&data);
		SoapError(h, 402, "Invalid Args");
		return;
	}

	/* if ip not valid assume hostname and convert */
	if (inet_pton(AF_INET, int_ip, &result_ip) <= 0)
	{
		hp = gethostbyname(int_ip);
		if(hp && hp->h_addrtype == AF_INET)
		{
			for(ptr = hp->h_addr_list; ptr && *ptr; ptr++)
		   	{
				int_ip = inet_ntoa(*((struct in_addr *) *ptr));
				result_ip = *((struct in_addr *) *ptr);
				/* TODO : deal with more than one ip per hostname */
				break;
			}
		}
		else
		{
			syslog(LOG_ERR, "Failed to convert hostname '%s' to ip address", int_ip);
			ClearNameValueList(&data);
			SoapError(h, 402, "Invalid Args");
			return;
		}
	}

	/* check if NewInternalAddress is the client address */
	if(GETFLAG(SECUREMODEMASK))
	{
		if(h->clientaddr.s_addr != result_ip.s_addr)
		{
			syslog(LOG_INFO, "Client %s tried to redirect port to %s",
			       inet_ntoa(h->clientaddr), int_ip);
			ClearNameValueList(&data);
			SoapError(h, 718, "ConflictInMappingEntry");
			return;
		}
	}

	int_port = GetValueFromNameValueList(&data, "NewInternalPort");
	ext_port = GetValueFromNameValueList(&data, "NewExternalPort");
	protocol = GetValueFromNameValueList(&data, "NewProtocol");
	desc = GetValueFromNameValueList(&data, "NewPortMappingDescription");
	leaseduration = GetValueFromNameValueList(&data, "NewLeaseDuration");

	if (!int_port || !ext_port || !protocol)
	{
		ClearNameValueList(&data);
		SoapError(h, 402, "Invalid Args");
		return;
	}

	eport = (unsigned short)atoi(ext_port);
	iport = (unsigned short)atoi(int_port);

	if(leaseduration && atoi(leaseduration)) {
		syslog(LOG_WARNING, "NewLeaseDuration=%s", leaseduration);
	}

	syslog(LOG_INFO, "AddPortMapping: external port %hu to %s:%hu protocol %s for: %s",
			eport, int_ip, iport, protocol, desc);

	r = upnp_redirect(eport, int_ip, iport, protocol, desc);

	ClearNameValueList(&data);

	/* possible error codes for AddPortMapping :
	 * 402 - Invalid Args
	 * 501 - Action Failed
	 * 715 - Wildcard not permited in SrcAddr
	 * 716 - Wildcard not permited in ExtPort
	 * 718 - ConflictInMappingEntry
	 * 724 - SamePortValuesRequired
     * 725 - OnlyPermanentLeasesSupported
             The NAT implementation only supports permanent lease times on
             port mappings
     * 726 - RemoteHostOnlySupportsWildcard
             RemoteHost must be a wildcard and cannot be a specific IP
             address or DNS name
     * 727 - ExternalPortOnlySupportsWildcard
             ExternalPort must be a wildcard and cannot be a specific port
             value */
	switch(r)
	{
	case 0:	/* success */
		BuildSendAndCloseSoapResp(h, resp, sizeof(resp)-1);
		break;
	case -2:	/* already redirected */
	case -3:	/* not permitted */
		SoapError(h, 718, "ConflictInMappingEntry");
		break;
	default:
		SoapError(h, 501, "ActionFailed");
	}
}

static void GetSpecificPortMappingEntry(struct upnphttp * h)
{
	int r;

	static const char resp[] =
		"<u:GetSpecificPortMappingEntryResponse "
		"xmlns:u=\"urn:schemas-upnp-org:service:WANIPConnection:1\">\n"
		"<NewInternalPort>%u</NewInternalPort>\n"
		"<NewInternalClient>%s</NewInternalClient>\n"
		"<NewEnabled>1</NewEnabled>\n"
		"<NewPortMappingDescription>%s</NewPortMappingDescription>\n"
		"<NewLeaseDuration>0</NewLeaseDuration>\n"
		"</u:GetSpecificPortMappingEntryResponse>\n";

	char body[2048];
	int bodylen;
	struct NameValueParserData data;
	const char * r_host, * ext_port, * protocol;
	unsigned short eport, iport;
	char int_ip[32];
	char desc[64];

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	r_host = GetValueFromNameValueList(&data, "NewRemoteHost");
	ext_port = GetValueFromNameValueList(&data, "NewExternalPort");
	protocol = GetValueFromNameValueList(&data, "NewProtocol");

	if(!ext_port || !protocol)
	{
		ClearNameValueList(&data);
		SoapError(h, 402, "Invalid Args");
		return;
	}

	eport = (unsigned short)atoi(ext_port);

	r = upnp_get_redirection_infos(eport, protocol, &iport,
	                               int_ip, sizeof(int_ip),
	                               desc, sizeof(desc));

	if(r < 0)
	{
		SoapError(h, 714, "NoSuchEntryInArray");
	}
	else
	{
		syslog(LOG_INFO, "GetSpecificPortMappingEntry: rhost='%s' %s %s found => %s:%u desc='%s'",
		       r_host, ext_port, protocol, int_ip, (unsigned int)iport, desc);
		bodylen = snprintf(body, sizeof(body), resp, (unsigned int)iport, int_ip, desc);
		BuildSendAndCloseSoapResp(h, body, bodylen);
	}

	ClearNameValueList(&data);
}

static void DeletePortMapping(struct upnphttp * h)
{
	int r;

	static const char resp[] =
		"<u:DeletePortMappingResponse "
		"xmlns:u=\"urn:schemas-upnp-org:service:WANIPConnection:1\">\n"
		"</u:DeletePortMappingResponse>\n";

	struct NameValueParserData data;
	const char * r_host, * ext_port, * protocol;
	unsigned short eport;

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	r_host = GetValueFromNameValueList(&data, "NewRemoteHost");
	ext_port = GetValueFromNameValueList(&data, "NewExternalPort");
	protocol = GetValueFromNameValueList(&data, "NewProtocol");

	if(!ext_port || !protocol)
	{
		ClearNameValueList(&data);
		SoapError(h, 402, "Invalid Args");
		return;
	}

	eport = (unsigned short)atoi(ext_port);

	/* TODO : if in secure mode, check the IP */

	syslog(LOG_INFO, "DeletePortMapping: external port: %hu, protocol: %s",
		eport, protocol);

	r = upnp_delete_redirection(eport, protocol);

	if(r < 0)
	{
		SoapError(h, 714, "NoSuchEntryInArray");
	}
	else
	{
		BuildSendAndCloseSoapResp(h, resp, sizeof(resp)-1);
	}

	ClearNameValueList(&data);
}

static void GetGenericPortMappingEntry(struct upnphttp * h)
{
	int r;

	static const char resp[] =
		"<u:GetGenericPortMappingEntryResponse "
		"xmlns:u=\"urn:schemas-upnp-org:service:WANIPConnection:1\">\n"
		"<NewRemoteHost></NewRemoteHost>\n"
		"<NewExternalPort>%u</NewExternalPort>\n"
		"<NewProtocol>%s</NewProtocol>\n"
		"<NewInternalPort>%u</NewInternalPort>\n"
		"<NewInternalClient>%s</NewInternalClient>\n"
		"<NewEnabled>1</NewEnabled>\n"
		"<NewPortMappingDescription>%s</NewPortMappingDescription>\n"
		"<NewLeaseDuration>0</NewLeaseDuration>\n"
		"</u:GetGenericPortMappingEntryResponse>\n";

	int index = 0;
	unsigned short eport, iport;
	const char * m_index;
	char protocol[4], iaddr[32];
	char desc[64];
	struct NameValueParserData data;

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	m_index = GetValueFromNameValueList(&data, "NewPortMappingIndex");

	if(!m_index)
	{
		ClearNameValueList(&data);
		SoapError(h, 402, "Invalid Args");
		return;
	}

	index = (int)atoi(m_index);

	syslog(LOG_INFO, "GetGenericPortMappingEntry: index=%d", index);

	r = upnp_get_redirection_infos_by_index(index, &eport, protocol, &iport,
                                            iaddr, sizeof(iaddr),
	                                        desc, sizeof(desc));

	if(r < 0)
	{
		SoapError(h, 713, "SpecifiedArrayIndexInvalid");
	}
	else
	{
		int bodylen;
		char body[2048];
		bodylen = snprintf(body, sizeof(body), resp, (unsigned int)eport,
			protocol, (unsigned int)iport, iaddr, desc);
		BuildSendAndCloseSoapResp(h, body, bodylen);
	}

	ClearNameValueList(&data);
}

#if defined(ENABLE_L3F_SERVICE) && !defined(TR_064)
static void SetDefaultConnectionService(struct upnphttp * h)
{
	static const char resp[] =
		"<u:SetDefaultConnectionServiceResponse "
		"xmlns:u=\"urn:schemas-upnp-org:service:Layer3Forwarding:1\">\n"
		"</u:SetDefaultConnectionServiceResponse>\n";
	struct NameValueParserData data;
	char * p;
	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	p = GetValueFromNameValueList(&data, "NewDefaultConnectionService");
	if(p) {
		syslog(LOG_INFO, "SetDefaultConnectionService(%s) : Ignored", p);
	}
	ClearNameValueList(&data);
	BuildSendAndCloseSoapResp(h, resp, sizeof(resp)-1);
}

static void GetDefaultConnectionService(struct upnphttp * h)
{
	static const char resp[] =
		"<u:GetDefaultConnectionServiceResponse "
		"xmlns:u=\"urn:schemas-upnp-org:service:Layer3Forwarding:1\">\n"
		"<NewDefaultConnectionService>%s:WANConnectionDevice:1,"
		"urn:upnp-org:serviceId:WANIPConn1</NewDefaultConnectionService>\n"
		"</u:GetDefaultConnectionServiceResponse>\n";
	/* example from UPnP_IGD_Layer3Forwarding 1.0.pdf :
	 * uuid:44f5824f-c57d-418c-a131-f22b34e14111:WANConnectionDevice:1,
	 * urn:upnp-org:serviceId:WANPPPConn1 */
	char body[1024];
	int bodylen;

	bodylen = snprintf(body, sizeof(body), resp, uuidvalue);
	BuildSendAndCloseSoapResp(h, body, bodylen);
}
#endif

/*
If a control point calls QueryStateVariable on a state variable that is not
buffered in memory within (or otherwise available from) the service,
the service must return a SOAP fault with an errorCode of 404 Invalid Var.

QueryStateVariable remains useful as a limited test tool but may not be
part of some future versions of UPnP.
 */
static void QueryStateVariable(struct upnphttp * h)
{
	static const char resp[] =
		"<u:QueryStateVariableResponse "
		"xmlns:u=\"urn:schemas-upnp-org:control-1-0\">\n"
		"<return>%s</return>\n"
		"</u:QueryStateVariableResponse>\n";

	char body[2048];
	int bodylen;
	struct NameValueParserData data;
	const char * var_name;

	ParseNameValue(h->req_buf + h->req_contentoff, h->req_contentlen, &data);
	/*var_name = GetValueFromNameValueList(&data, "QueryStateVariable"); */
	/*var_name = GetValueFromNameValueListIgnoreNS(&data, "varName");*/
	if(!(var_name = GetValueFromNameValueList(&data, "varName")))  // For compatibility issue of Windows XP
		var_name = GetValueFromNameValueList(&data, "m:varName");

	/*syslog(LOG_INFO, "QueryStateVariable(%.40s)", var_name); */

	if(!var_name)
	{
		SoapError(h, 402, "Invalid Args");
	}
	else if(strcasecmp(var_name, "ConnectionStatus") == 0)
	{
		bodylen = snprintf(body, sizeof(body), resp, "Connected");
		BuildSendAndCloseSoapResp(h, body, bodylen);
	}
#if 0
	/* not usefull */
	else if(strcasecmp(var_name, "ConnectionType") == 0)
	{
		bodylen = snprintf(body, sizeof(body), resp, "IP_Routed");
		BuildSendAndCloseSoapResp(h, body, bodylen);
	}
	else if(strcasecmp(var_name, "LastConnectionError") == 0)
	{
		bodylen = snprintf(body, sizeof(body), resp, "ERROR_NONE");
		BuildSendAndCloseSoapResp(h, body, bodylen);
	}
#endif
	else if(strcasecmp(var_name, "PortMappingNumberOfEntries") == 0)
	{
		int r = 0, index = 0;
		unsigned short eport, iport;
		char protocol[4], iaddr[32], desc[64];
		char strindex[10];

		do
		{
			protocol[0] = '\0'; iaddr[0] = '\0'; desc[0] = '\0';

			r = upnp_get_redirection_infos_by_index(index, &eport, protocol, &iport,
													iaddr, sizeof(iaddr),
													desc, sizeof(desc));
			index++;
		}
		while(r==0);

		snprintf(strindex, sizeof(strindex), "%i", index - 1);
		bodylen = snprintf(body, sizeof(body), resp, strindex);
		BuildSendAndCloseSoapResp(h, body, bodylen);
	}
	else
	{
		syslog(LOG_NOTICE, "QueryStateVariable: Unknown: %s", var_name?var_name:"");
		SoapError(h, 404, "Invalid Var");
	}

	ClearNameValueList(&data);
}
#endif // TR_064

#ifdef TR_064
const struct _soapMethods soapMethods[] =
{
	{ "WANDSLInterfaceConfig", "SetEnable", WANDSLInterfaceConfigSetEnable},
	{ "WANDSLInterfaceConfig", "GetInfo", WANDSLInterfaceConfigGetInfo},
	{ "WANDSLInterfaceConfig", "GetStatisticsTotal", WANDSLInterfaceConfigGetStatisticsTotal},
	{ "WANDSLConnectionManagement", "GetWANConnectionServiceNumberOfEntries", WANDSLConnectionManagementGetWANConnectionServiceNumberOfEntries},
	{ "WANDSLConnectionManagement", "GetGenericConnectionServiceEntry", WANDSLConnectionManagementGetGenericConnectionServiceEntry},
	{ "WANDSLConnectionManagement", "GetSpecificConnectionServiceEntry", WANDSLConnectionManagementGetSpecificConnectionServiceEntry},
  { "WANDSLConnectionManagement", "AddConnectionDeviceAndService", WANDSLConnectionManagementAddConnectionDeviceAndService},
 	{ "WANDSLConnectionManagement", "AddConnectionService", WANDSLConnectionManagementAddConnectionService},
	{ "WANDSLConnectionManagement", "DeleteConnectionService", WANDSLConnectionManagementDeleteConnectionService},
	{ "WANDSLLinkConfig", "SetEnable", WANDSLLinkConfigSetEnable},
	{ "WANDSLLinkConfig", "SetDestinationAddress", WANDSLLinkConfigSetDestinationAddress},
	{ "WANDSLLinkConfig", "GetDestinationAddress", WANDSLLinkConfigGetDestinationAddress},
	{ "WANDSLLinkConfig", "SetDSLLinkType", WANDSLLinkConfigSetDSLLinkType},
	{ "WANDSLLinkConfig", "GetDSLLinkInfo", WANDSLLinkConfigGetDSLLinkInfo},
	{ "WANDSLLinkConfig", "SetATMEncapsulation", WANDSLLinkConfigSetATMEncapsulation},
	{ "WANDSLLinkConfig", "GetATMEncapsulation", WANDSLLinkConfigGetATMEncapsulation},
	{ "WANIPConnection", "GetConnectionTypeInfo", WANIPConnectionGetConnectionTypeInfo},
	{ "WANIPConnection", "GetNATRSIPStatus", WANIPConnectionGetNATRSIPStatus},
	{ "WANIPConnection", "GetExternalIPAddress", WANIPConnectionGetExternalIPAddress},
	{ "WANIPConnection", "AddPortMapping", WANIPConnectionAddPortMapping},
	{ "WANIPConnection", "DeletePortMapping", WANIPConnectionDeletePortMapping},
	{ "WANIPConnection", "GetGenericPortMappingEntry", WANIPConnectionGetGenericPortMappingEntry},
	{ "WANIPConnection", "GetSpecificPortMappingEntry", WANIPConnectionGetSpecificPortMappingEntry},
	{ "WANIPConnection", "QueryStateVariable", WANIPConnectionQueryStateVariable},
	{ "WANIPConnection", "GetInfo", WANIPConnectionGetInfo},
	{ "WANIPConnection", "SetConnectionType", WANIPConnectionSetConnectionType},
	{ "WANIPConnection", "SetEnable", WANIPConnectionSetEnable},
	{ "WANPPPConnection", "SetEnable", WANPPPConnectionSetEnable},
	{ "WANPPPConnection", "SetUserName", WANPPPConnectionSetUserName},
	{ "WANPPPConnection", "GetUserName", WANPPPConnectionGetUserName},
	{ "WANPPPConnection", "SetPassword", WANPPPConnectionSetPassword},
	{ "WANPPPConnection", "GetStatusInfo", WANPPPConnectionGetStatusInfo},
	{ "WANPPPConnection", "RequestConnection", WANPPPConnectionRequestConnection},
	{ "WANCommonInterfaceConfig", "SetEnabledForInternet", WANCommonInterfaceConfigSetEnabledForInternet},
  { "WANCommonInterfaceConfig", "GetEnabledForInternet", WANCommonInterfaceConfigGetEnabledForInternet},
	{ "WANCommonInterfaceConfig", "GetTotalBytesSent", WANCommonInterfaceConfigGetTotalBytesSent},
	{ "WANCommonInterfaceConfig", "GetTotalBytesReceived", WANCommonInterfaceConfigGetTotalBytesReceived},
	{ "WANCommonInterfaceConfig", "GetTotalPacketsSent", WANCommonInterfaceConfigGetTotalPacketsSent},
	{ "WANCommonInterfaceConfig", "GetTotalPacketsReceived", WANCommonInterfaceConfigGetTotalPacketsReceived},
	{ "WANCommonInterfaceConfig", "GetCommonLinkProperties", WANCommonInterfaceConfigGetCommonLinkProperties},
	{ "WANIPConnection", "GetStatusInfo", WANIPConnectionGetStatusInfo},
	{ "DeviceConfig", "Reboot", DeviceConfigReboot},
	{ "DeviceConfig", "FactoryReset", DeviceConfigFactoryReset},
	{ "DeviceConfig", "ConfigurationStarted", DeviceConfigConfigurationStarted},
	{ "DeviceConfig", "ConfigurationFinished", DeviceConfigConfigurationFinished},
	{ "DeviceInfo", "GetInfo", DeviceInfoGetInfo},
	{ "DeviceInfo", "SetProvisioningCode", DeviceInfoSetProvisioningCode},
	{ "DeviceInfo", "GetDeviceLog", DeviceInfoGetDeviceLog},
	{ "Layer3Forwarding", "GetDefaultConnectionService", Layer3ForwardingGetDefaultConnectionService},
	{ "Layer3Forwarding", "SetDefaultConnectionService", Layer3ForwardingSetDefaultConnectionService},
	{ "Layer3Forwarding", "GetForwardNumberOfEntries", Layer3ForwardingGetForwardNumberOfEntries},
	{ "Layer3Forwarding", "AddForwardingEntry", Layer3ForwardingAddForwardingEntry},
  { "Layer3Forwarding", "DeleteForwardingEntry", Layer3ForwardingDeleteForwardingEntry},
	{ "Layer3Forwarding", "GetSpecificForwardingEntry", Layer3ForwardingGetSpecificForwardingEntry},
	{ "Layer3Forwarding", "SetForwardingEntryEnable", Layer3ForwardingSetForwardingEntryEnable},
  { "Layer3Forwarding", "GetGenericForwardingEntry", Layer3ForwardingGetGenericForwardingEntry},
	{ "LANConfigSecurity", "SetConfigPassword", LANConfigSecuritySetConfigPassword},
	{ "ManagementServer", "GetInfo", ManagementServerGetInfo},
	{ "ManagementServer", "SetManagementServerURL", ManagementServerSetManagementServerURL},
	{ "ManagementServer", "SetManagementServerPassword", ManagementServerSetManagementServerPassword},
	{ "ManagementServer", "SetPeriodicInform", ManagementServerSetPeriodicInform},
	{ "ManagementServer", "SetUpgradeManagement", ManagementServerSetUpgradeManagement},
	{ "ManagementServer", "SetConnectionRequestAuthentication", ManagementServerSetConnectionRequestAuthentication},
	{ "Time", "GetInfo", TimeGetInfo},
	{ "Time", "SetNTPServers", TimeSetNTPServers},
	{ "Time", "SetLocalTimeZone", TimeSetLocalTimeZone},
	{ "LANHostConfigManagement", "GetInfo", LANHCMGetInfo},
	{ "LANHostConfigManagement", "GetDHCPRelay", LANHCMGetDHCPRelay},
	{ "LANHostConfigManagement", "SetSubnetMask", LANHCMSetSubnetMask},
	{ "LANHostConfigManagement", "GetSubnetMask", LANHCMGetSubnetMask},
	{ "LANHostConfigManagement", "SetIPRouter", LANHCMSetIPRouter},
	{ "LANHostConfigManagement", "GetIPRoutersList", LANHCMGetIPRoutersList},
	{ "LANHostConfigManagement", "SetDomainName", LANHCMSetDomainName},
	{ "LANHostConfigManagement", "GetDomainName", LANHCMGetDomainName},
	{ "LANHostConfigManagement", "GetAddressRange", LANHCMGetAddressRange},
	{ "LANHostConfigManagement", "SetDNSServer", LANHCMSetDNSServer},
	{ "LANHostConfigManagement", "DeleteDNSServer", LANHCMDeleteDNSServer},
	{ "LANHostConfigManagement", "GetDNSServers", LANHCMGetDNSServers},
	{ "LANHostConfigManagement", "SetDHCPServerEnable", LANHCMSetDHCPServerEnable},
	{ "LANHostConfigManagement", "SetDHCPLeaseTime", LANHCMSetDHCPLeaseTime},
	{ 0, 0, 0 }
};
#else // TR_064
/* Windows XP as client send the following requests :
 * GetConnectionTypeInfo
 * GetNATRSIPStatus
 * ? GetTotalBytesSent - WANCommonInterfaceConfig
 * ? GetTotalBytesReceived - idem
 * ? GetTotalPacketsSent - idem
 * ? GetTotalPacketsReceived - idem
 * GetCommonLinkProperties - idem
 * GetStatusInfo - WANIPConnection
 * GetExternalIPAddress
 * QueryStateVariable / ConnectionStatus!
 */
const struct _soapMethods soapMethods[] =
{
	{ 0, "GetConnectionTypeInfo", GetConnectionTypeInfo },
	{ 0, "GetNATRSIPStatus", GetNATRSIPStatus},
	{ 0, "GetExternalIPAddress", GetExternalIPAddress},
	{ 0, "AddPortMapping", AddPortMapping},
	{ 0, "DeletePortMapping", DeletePortMapping},
	{ 0, "GetGenericPortMappingEntry", GetGenericPortMappingEntry},
	{ 0, "GetSpecificPortMappingEntry", GetSpecificPortMappingEntry},
	{ 0, "QueryStateVariable", QueryStateVariable},
	{ 0, "GetTotalBytesSent", GetTotalBytesSent},
	{ 0, "GetTotalBytesReceived", GetTotalBytesReceived},
	{ 0, "GetTotalPacketsSent", GetTotalPacketsSent},
	{ 0, "GetTotalPacketsReceived", GetTotalPacketsReceived},
	{ 0, "GetCommonLinkProperties", GetCommonLinkProperties},
	{ 0, "GetStatusInfo", GetStatusInfo},
#if defined(ENABLE_L3F_SERVICE)
	{ 0, "SetDefaultConnectionService", SetDefaultConnectionService},
	{ 0, "GetDefaultConnectionService", GetDefaultConnectionService},
#endif
	{ 0, 0, 0 }
};
#endif // TR_064

