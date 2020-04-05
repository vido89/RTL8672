/*
 *      Web server handler routines for DHCP Server stuffs
 *      Authors: Kaohj	<kaohj@realtek.com.tw>
 *
 */


/*-- System inlcude files --*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <time.h>
#include <net/route.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <net/if.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"


#ifdef __i386__
#define _LITTLE_ENDIAN_
#endif

#define _DHCPD_PID_FILE			T("/var/run/udhcpd.pid")
#define _DHCPD_LEASES_FILE		T("/var/udhcpd/udhcpd.leases")

extern int read_pid(char *filename);

/*-- Macro declarations --*/
#ifdef _LITTLE_ENDIAN_
#define ntohdw(v) ( ((v&0xff)<<24) | (((v>>8)&0xff)<<16) | (((v>>16)&0xff)<<8) | ((v>>24)&0xff) )

#else
#define ntohdw(v) (v)
#endif

///////////////////////////////////////////////////////////////////
#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
int store_day,store_hour,store_minute;
#endif
void formDhcpd(webs_t wp, char_t *path, char_t *query)
{
	char_t	*strDhcp, *submitUrl, *strIp;
	struct in_addr inIp, inMask, inGatewayIp;
#ifdef DHCPS_POOL_COMPLETE_IP
	struct in_addr inPoolStart, inPoolEnd, dhcpmask, ori_dhcpmask;
	char_t *str_dhcpmask;
#endif
	DHCP_T dhcp, curDhcp;
	char tmpBuf[100];
#ifndef NO_ACTION
	int pid;
#endif
	unsigned char vChar;
//star: for dhcp change
	unsigned char origvChar;
	unsigned int origInt;
	char origstr[30];
	char_t *origstrDomain=origstr;
	struct in_addr origGatewayIp;
	int dhcpd_changed_flag=0;
	char_t *strdhcpenable;
	unsigned char mode;

	char_t	*strdhcpRangeStart, *strdhcpRangeEnd, *strLTime, *strDomain;

//#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
	strdhcpenable = websGetVar(wp, T("dhcpdenable"), T(""));
	mib_get( MIB_DHCP_MODE, (void *)&origvChar);
#ifdef ZTE_531B_BRIDGE_SC
	if(strdhcpenable[0] == '0')
	{
		if(origvChar != DHCP_LAN_SERVER)
		{
			dhcpd_changed_flag = 1;
			mode = DHCP_LAN_SERVER;
			if ( !mib_set(MIB_DHCP_MODE, (void *)&mode)) {
  			strcpy(tmpBuf, T(strSetDhcpModeerror));
			goto setErr_dhcpd;
			}
		}
	}
	else
	{
		if(origvChar == DHCP_LAN_SERVER)
		{
			dhcpd_changed_flag = 1;
			mode = DHCP_LAN_NONE;
			if ( !mib_set(MIB_DHCP_MODE, (void *)&mode)) {
  			strcpy(tmpBuf, T(strSetDhcpModeerror));
			goto setErr_dhcpd;
			}
		}
	}
#else

	if(strdhcpenable[0])
	{
		sscanf(strdhcpenable, "%u", &origInt);
		mode = (unsigned char)origInt;
		if(mode!=origvChar)
			dhcpd_changed_flag = 1;
		if ( !mib_set(MIB_DHCP_MODE, (void *)&mode)) {
  			strcpy(tmpBuf, T(strSetDhcpModeerror));
			goto setErr_dhcpd;
		}
	}
#endif

//#endif

	// Read current DHCP setting for reference later
	// Modified by Mason Yu for dhcpmode
	//if ( !mib_get( MIB_ADSL_LAN_DHCP, (void *)&vChar) ) {
	if ( !mib_get( MIB_DHCP_MODE, (void *)&vChar) ) {
		strcpy(tmpBuf, T(strGetDhcpModeerror));
		goto setErr_dhcpd;
	}
	curDhcp = (DHCP_T) vChar;

// Commented by Mason Yu for dhcpmode
#if 0
	strDhcp = websGetVar(wp, T("dhcp"), T(""));
	if ( strDhcp[0] ) {
		if ( strDhcp[0]!='0' && strDhcp[0]!='1' && strDhcp[0]!='2') {
			strcpy(tmpBuf, T("Invalid DHCP value!"));
			goto setErr_dhcpd;
		}
		// set to MIB
		if ( strDhcp[0] == '0' )
			dhcp = DHCP_DISABLED;
		else if (strDhcp[0] == '1')
			dhcp = DHCP_CLIENT;
		else if (strDhcp[0] == '2')
			dhcp = DHCP_SERVER;
	}
	else
#endif	
		dhcp = curDhcp;


	if ( dhcp == DHCP_LAN_SERVER ) {
		// Get/Set DHCP client range
		unsigned int uVal, uLTime;
#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
		unsigned int day,hour,minute;
#endif
		unsigned char uStart, uEnd;
		
		// Kaohj
		#ifndef DHCPS_POOL_COMPLETE_IP
		strdhcpRangeStart = websGetVar(wp, T("dhcpRangeStart"), T(""));
		if ( strdhcpRangeStart[0] ) {
			sscanf(strdhcpRangeStart, "%u", &uVal);
			uStart = (unsigned char)uVal;
		}
		strdhcpRangeEnd = websGetVar(wp, T("dhcpRangeEnd"), T(""));
		if ( strdhcpRangeEnd[0] ) {
			sscanf(strdhcpRangeEnd, "%u", &uVal);
			uEnd = (unsigned char)uVal;
		}
		#else
		strdhcpRangeStart = websGetVar(wp, T("dhcpRangeStart"), T(""));
		if ( strdhcpRangeStart[0] ) {
			if ( !inet_aton(strdhcpRangeStart, &inPoolStart) ) {
				strcpy(tmpBuf, T(strSetStarIperror));
				goto setErr_dhcpd;
			}
		}
		strdhcpRangeEnd = websGetVar(wp, T("dhcpRangeEnd"), T(""));
		if ( strdhcpRangeEnd[0] ) {
			if ( !inet_aton(strdhcpRangeEnd, &inPoolEnd) ) {
				strcpy(tmpBuf, T(strSetEndIperror));
				goto setErr_dhcpd;
			}
		}
		#endif

#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
		strLTime = websGetVar(wp, T("time_dd"), T(""));		
		if ( strLTime[0] ) {
			sscanf(strLTime, "%u", &day);
		}
		strLTime = websGetVar(wp, T("time_hh"), T(""));		
		if ( strLTime[0] ) {
			sscanf(strLTime, "%u", &hour);
		}
		strLTime = websGetVar(wp, T("time_mm"), T(""));		
		if ( strLTime[0] ) {
			sscanf(strLTime, "%u", &minute);
		}
#else
		strLTime = websGetVar(wp, T("ltime"), T(""));		
		if ( strLTime[0] ) {
			sscanf(strLTime, "%u", &uLTime);
		}
#endif

		strDomain = websGetVar(wp, T("dname"), T(""));
		
		if(!mib_get( MIB_ADSL_LAN_IP,  (void *)&inIp)) {
			strcpy(tmpBuf, T(strGetIperror));
			goto setErr_dhcpd;
		}
		
		if(!mib_get( MIB_ADSL_LAN_SUBNET,  (void *)&inMask)) {
			strcpy(tmpBuf, T(strGetMaskerror));
			goto setErr_dhcpd;
		}
		
		// Kaohj
		#ifndef DHCPS_POOL_COMPLETE_IP
		// update DHCP server config file
		if ( strdhcpRangeStart[0] && strdhcpRangeEnd[0] ) {
			unsigned char *ip, *mask;
			int diff;

			diff = (int) ( uEnd - uStart );
			ip = (unsigned char *)&inIp;
			mask = (unsigned char *)&inMask;
			if (diff <= 0 ||
				(ip[3]&mask[3]) != (uStart&mask[3]) ||
				(ip[3]&mask[3]) != (uEnd&mask[3]) ) {
				strcpy(tmpBuf, T(strInvalidRange));
				goto setErr_dhcpd;
			}
		}
		#else
		// check the pool range
		if ( strdhcpRangeStart[0] && strdhcpRangeEnd[0] ) {
			if (inPoolStart.s_addr >= inPoolEnd.s_addr) {
				strcpy(tmpBuf, T(strInvalidRange));
				goto setErr_dhcpd;
			}
		}

		// Magician: Subnet mask for DHCP.
		str_dhcpmask = websGetVar(wp, T("dhcpSubnetMask"), T(""));

		if(!inet_aton(str_dhcpmask, &dhcpmask))
		{
			strcpy(tmpBuf, T("Invalid value of subnet mask!"));
			goto setErr_dhcpd;
		}

		if((inPoolStart.s_addr & dhcpmask.s_addr) != (inPoolEnd.s_addr & dhcpmask.s_addr))
		{
			strcpy(tmpBuf, T("DHCP IP range does not match under the  subnet mask!"));
			goto setErr_dhcpd;
		}
		#endif

#ifdef IP_BASED_CLIENT_TYPE
		unsigned char pcstart,pcend,cmrstart,cmrend,stbstart,stbend,phnstart,phnend;
		
		//PC
		strdhcpRangeStart = websGetVar(wp, T("dhcppcRangeStart"), T(""));
		if ( strdhcpRangeStart[0] ) {
			sscanf(strdhcpRangeStart, "%u", &uVal);
			pcstart = (unsigned char)uVal;
		}
		strdhcpRangeEnd = websGetVar(wp, T("dhcppcRangeEnd"), T(""));
		if ( strdhcpRangeEnd[0] ) {
			sscanf(strdhcpRangeEnd, "%u", &uVal);
			pcend = (unsigned char)uVal;
		}
		if ( strdhcpRangeStart[0] && strdhcpRangeEnd[0] ) {
			unsigned char *ip, *mask;
			int diff;

			diff = (int) ( pcend - pcstart );
			ip = (unsigned char *)&inIp;
			mask = (unsigned char *)&inMask;
			if (diff <= 0 ||
				(ip[3]&mask[3]) != (pcstart&mask[3]) ||
				(ip[3]&mask[3]) != (pcend&mask[3]) ) {
				strcpy(tmpBuf, T(strInvalidRangepc));
				goto setErr_dhcpd;
			}
		}
		//CMR
		strdhcpRangeStart = websGetVar(wp, T("dhcpcmrRangeStart"), T(""));
		if ( strdhcpRangeStart[0] ) {
			sscanf(strdhcpRangeStart, "%u", &uVal);
			cmrstart = (unsigned char)uVal;
		}
		strdhcpRangeEnd = websGetVar(wp, T("dhcpcmrRangeEnd"), T(""));
		if ( strdhcpRangeEnd[0] ) {
			sscanf(strdhcpRangeEnd, "%u", &uVal);
			cmrend = (unsigned char)uVal;
		}
		if ( strdhcpRangeStart[0] && strdhcpRangeEnd[0] ) {
			unsigned char *ip, *mask;
			int diff;

			diff = (int) ( cmrend - cmrstart );
			ip = (unsigned char *)&inIp;
			mask = (unsigned char *)&inMask;
			if (diff <= 0 ||
				(ip[3]&mask[3]) != (cmrstart&mask[3]) ||
				(ip[3]&mask[3]) != (cmrend&mask[3]) ) {
				strcpy(tmpBuf, T(strInvalidRangecmr));
				goto setErr_dhcpd;
			}
		}
		//STB
		strdhcpRangeStart = websGetVar(wp, T("dhcpstbRangeStart"), T(""));
		if ( strdhcpRangeStart[0] ) {
			sscanf(strdhcpRangeStart, "%u", &uVal);
			stbstart = (unsigned char)uVal;
		}
		strdhcpRangeEnd = websGetVar(wp, T("dhcpstbRangeEnd"), T(""));
		if ( strdhcpRangeEnd[0] ) {
			sscanf(strdhcpRangeEnd, "%u", &uVal);
			stbend = (unsigned char)uVal;
		}
		if ( strdhcpRangeStart[0] && strdhcpRangeEnd[0] ) {
			unsigned char *ip, *mask;
			int diff;

			diff = (int) ( stbend - stbstart );
			ip = (unsigned char *)&inIp;
			mask = (unsigned char *)&inMask;
			if (diff <= 0 ||
				(ip[3]&mask[3]) != (stbstart&mask[3]) ||
				(ip[3]&mask[3]) != (stbend&mask[3]) ) {
				strcpy(tmpBuf, T(strInvalidRangestb));
				goto setErr_dhcpd;
			}
		}
		//PHN
		strdhcpRangeStart = websGetVar(wp, T("dhcpphnRangeStart"), T(""));
		if ( strdhcpRangeStart[0] ) {
			sscanf(strdhcpRangeStart, "%u", &uVal);
			phnstart = (unsigned char)uVal;
		}
		strdhcpRangeEnd = websGetVar(wp, T("dhcpphnRangeEnd"), T(""));
		if ( strdhcpRangeEnd[0] ) {
			sscanf(strdhcpRangeEnd, "%u", &uVal);
			phnend = (unsigned char)uVal;
		}
		if ( strdhcpRangeStart[0] && strdhcpRangeEnd[0] ) {
			unsigned char *ip, *mask;
			int diff;

			diff = (int) ( phnend - phnstart );
			ip = (unsigned char *)&inIp;
			mask = (unsigned char *)&inMask;
			if (diff <= 0 ||
				(ip[3]&mask[3]) != (phnstart&mask[3]) ||
				(ip[3]&mask[3]) != (phnend&mask[3]) ) {
				strcpy(tmpBuf, T(strInvalidRangephn));
				goto setErr_dhcpd;
			}
		}
		//check if the type ip pool out of ip pool range
		if((pcstart<uStart)||(cmrstart<uStart)||(stbstart<uStart)||(phnstart<uStart)
			||(pcend>uEnd)||(cmrend>uEnd)||(stbend>uEnd)||(phnend>uEnd)){
				strcpy(tmpBuf, T(strInvalidTypeRange));
				goto setErr_dhcpd;
		}
		//check if the type ip pool overlap
		unsigned char ippool[4][2]={{pcstart,pcend},{cmrstart,cmrend},{stbstart,stbend},{phnstart,phnend}};
		unsigned char tmp1,tmp2;
		int i,j,min;
		for(i=0;i<4;i++)
		{
			min = i;
			for(j=i;j<4;j++)
			{
				if(ippool[j][0] < ippool[min][0])
					min = j;
			}
			if(min!=i){
				tmp1=ippool[i][0];
				tmp2=ippool[i][1];
				ippool[i][0]=ippool[min][0];
				ippool[i][1]=ippool[min][1];
				ippool[min][0]=tmp1;
				ippool[min][1]=tmp2;
			}
		}
	       for(i=0;i<3;i++)
	      	 {
			if(ippool[i][1]>=ippool[i+1][0]){
				strcpy(tmpBuf, T(strOverlapRange));
				goto setErr_dhcpd;
			}
	      	 }

		//set the type ip pool
		mib_get(CWMP_CT_PC_MINADDR, (void *)&origvChar);
		if(origvChar != pcstart)
			dhcpd_changed_flag = 1;
		if ( !mib_set(CWMP_CT_PC_MINADDR, (void *)&pcstart)) {
			strcpy(tmpBuf, T(strSetPcStartIperror));
			goto setErr_dhcpd;
		}

		mib_get(CWMP_CT_PC_MAXADDR, (void *)&origvChar);
		if(origvChar != pcend)
			dhcpd_changed_flag = 1;
		if ( !mib_set(CWMP_CT_PC_MAXADDR, (void *)&pcend)) {
			strcpy(tmpBuf, T(strSetPcEndIperror));
			goto setErr_dhcpd;
		}

		mib_get(CWMP_CT_CMR_MINADDR, (void *)&origvChar);
		if(origvChar != cmrstart)
			dhcpd_changed_flag = 1;
		if ( !mib_set(CWMP_CT_CMR_MINADDR, (void *)&cmrstart)) {
			strcpy(tmpBuf, T(strSetCmrStartIperror));
			goto setErr_dhcpd;
		}

		mib_get(CWMP_CT_CMR_MAXADDR, (void *)&origvChar);
		if(origvChar != cmrend)
			dhcpd_changed_flag = 1;
		if ( !mib_set(CWMP_CT_CMR_MAXADDR, (void *)&cmrend)) {
			strcpy(tmpBuf, T(strSetCmrEndIperror));
			goto setErr_dhcpd;
		}

		mib_get(CWMP_CT_STB_MINADDR, (void *)&origvChar);
		if(origvChar != stbstart)
			dhcpd_changed_flag = 1;
		if ( !mib_set(CWMP_CT_STB_MINADDR, (void *)&stbstart)) {
			strcpy(tmpBuf, T(strSetStbStartIperror));
			goto setErr_dhcpd;
		}

		mib_get(CWMP_CT_STB_MAXADDR, (void *)&origvChar);
		if(origvChar != stbend)
			dhcpd_changed_flag = 1;
		if ( !mib_set(CWMP_CT_STB_MAXADDR, (void *)&stbend)) {
			strcpy(tmpBuf, T(strSetStbEndIperror));
			goto setErr_dhcpd;
		}

		mib_get(CWMP_CT_PHN_MINADDR, (void *)&origvChar);
		if(origvChar != phnstart)
			dhcpd_changed_flag = 1;
		if ( !mib_set(CWMP_CT_PHN_MINADDR, (void *)&phnstart)) {
			strcpy(tmpBuf, T(strSetPhnStartIperror));
			goto setErr_dhcpd;
		}

		mib_get(CWMP_CT_PHN_MAXADDR, (void *)&origvChar);
		if(origvChar != phnend)
			dhcpd_changed_flag = 1;
		if ( !mib_set(CWMP_CT_PHN_MAXADDR, (void *)&phnend)) {
			strcpy(tmpBuf, T(strSetPhnEndIperror));
			goto setErr_dhcpd;
		}
		  
#endif

		// Kaohj
		#ifndef DHCPS_POOL_COMPLETE_IP
		mib_get(MIB_ADSL_LAN_CLIENT_START, (void *)&origvChar);
		if(origvChar != uStart)
			dhcpd_changed_flag = 1;
		if ( !mib_set(MIB_ADSL_LAN_CLIENT_START, (void *)&uStart)) {
			strcpy(tmpBuf, T(strSetStarIperror));
			goto setErr_dhcpd;
		}

		mib_get(MIB_ADSL_LAN_CLIENT_END, (void *)&origvChar);
		if(origvChar != uEnd)
			dhcpd_changed_flag = 1;
		if ( !mib_set(MIB_ADSL_LAN_CLIENT_END, (void *)&uEnd)) {
			strcpy(tmpBuf, T(strSetEndIperror));
			goto setErr_dhcpd;
		}		
		#else
		mib_get(MIB_DHCP_POOL_START, (void *)&inIp);
		if(inIp.s_addr != inPoolStart.s_addr)
			dhcpd_changed_flag = 1;
		if ( !mib_set( MIB_DHCP_POOL_START, (void *)&inPoolStart)) {
			strcpy(tmpBuf, T(strSetStarIperror));
			goto setErr_dhcpd;
		}
		mib_get(MIB_DHCP_POOL_END, (void *)&inIp);
		if(inIp.s_addr != inPoolEnd.s_addr)
			dhcpd_changed_flag = 1;
		if ( !mib_set( MIB_DHCP_POOL_END, (void *)&inPoolEnd)) {
			strcpy(tmpBuf, T(strSetEndIperror));
			goto setErr_dhcpd;
		}

		// Magician: Subnet mask for DHCP.
		mib_get(MIB_DHCP_SUBNET_MASK, (void *)&ori_dhcpmask);
		if( ori_dhcpmask.s_addr != dhcpmask.s_addr )
			dhcpd_changed_flag = 1;

		if( !mib_set(MIB_DHCP_SUBNET_MASK, (void *)&dhcpmask))
		{
			strcpy(tmpBuf, T("Set DHCP subnetmask failed!"));
			goto setErr_dhcpd;
		}
		#endif

		mib_get(MIB_ADSL_LAN_DHCP_LEASE, (void *)&origInt);
#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
		if(day==-1&&hour==-1&&minute==-1)
			uLTime=-1;
		else
			uLTime = 86400*day+3600*hour+60*minute;
		if(day!=store_day||hour!=store_hour||minute!=store_minute||origInt!=uLTime)
			dhcpd_changed_flag = 1;
#else
		if(origInt != uLTime)
			dhcpd_changed_flag = 1;
#endif
		if ( !mib_set(MIB_ADSL_LAN_DHCP_LEASE, (void *)&uLTime)) {
			strcpy(tmpBuf, T(strSetLeaseTimeerror));
			goto setErr_dhcpd;
		}

		mib_get(MIB_ADSL_LAN_DHCP_DOMAIN, (void *)origstrDomain);
		if(strcmp(origstrDomain, strDomain)!=0)
			dhcpd_changed_flag = 1;
		if ( !mib_set(MIB_ADSL_LAN_DHCP_DOMAIN, (void *)strDomain)) {
			strcpy(tmpBuf, T(strSetDomainNameerror));
			goto setErr_dhcpd;
		}
		
		// Added by Mason Yu for DHCP Server Gateway Address
		// Set Gateway address
		strIp = websGetVar(wp, T("ip"), T(""));
		if ( strIp[0] ) {
			if ( !inet_aton(strIp, &inGatewayIp) ) {
				strcpy(tmpBuf, T(strInvalidGatewayerror));
				goto setErr_dhcpd;
			}
			mib_get(MIB_ADSL_LAN_DHCP_GATEWAY,(void*)&origGatewayIp);
			if(origGatewayIp.s_addr != inGatewayIp.s_addr)
				dhcpd_changed_flag = 1;
			if ( !mib_set( MIB_ADSL_LAN_DHCP_GATEWAY, (void *)&inGatewayIp)) {
				strcpy(tmpBuf, T(strSetGatewayerror));
				goto setErr_dhcpd;
			}
		}		
		// Kaohj
#ifdef DHCPS_DNS_OPTIONS
		strDhcp = websGetVar(wp, T("dhcpdns"), T(""));
		strDhcp[0] -= '0';
		mib_get(MIB_DHCP_DNS_OPTION, (void *)&origvChar);
		if (origvChar != strDhcp[0])
			dhcpd_changed_flag = 1;
		mib_set(MIB_DHCP_DNS_OPTION, (void *)strDhcp);
		if (strDhcp[0] == 1) { // set manually
			strIp = websGetVar(wp, T("dns1"), T(""));
			if ( !inet_aton(strIp, &inIp) ) {
				strcpy(tmpBuf, T(Tinvalid_dns));
				goto setErr_dhcpd;
			}
			mib_get(MIB_DHCPS_DNS1, (void *)&origGatewayIp);
			if (origGatewayIp.s_addr != inIp.s_addr)
				dhcpd_changed_flag = 1;
			mib_set(MIB_DHCPS_DNS1, (void *)&inIp);
			inIp.s_addr = 0xffffffff;
			strIp = websGetVar(wp, T("dns2"), T(""));
			if (strIp[0]) {
				if ( !inet_aton(strIp, &inIp) ) {
					strcpy(tmpBuf, T(Tinvalid_dns));
					goto setErr_dhcpd;
				}
				mib_get(MIB_DHCPS_DNS2, (void *)&origGatewayIp);
				if (origGatewayIp.s_addr != inIp.s_addr)
					dhcpd_changed_flag = 1;
				mib_set(MIB_DHCPS_DNS2, (void *)&inIp);
				inIp.s_addr = 0xffffffff;
				strIp = websGetVar(wp, T("dns3"), T(""));
				if (strIp[0]) {
					if ( !inet_aton(strIp, &inIp) ) {
						strcpy(tmpBuf, T(Tinvalid_dns));
						goto setErr_dhcpd;
					}
				}
				mib_get(MIB_DHCPS_DNS3, (void *)&origGatewayIp);
				if (origGatewayIp.s_addr != inIp.s_addr)
					dhcpd_changed_flag = 1;
				mib_set(MIB_DHCPS_DNS3, (void *)&inIp);
			}
			else {
				mib_get(MIB_DHCPS_DNS2, (void *)&origGatewayIp);
				if (origGatewayIp.s_addr != inIp.s_addr)
					dhcpd_changed_flag = 1;
				mib_set(MIB_DHCPS_DNS2, (void *)&inIp);
				mib_set(MIB_DHCPS_DNS3, (void *)&inIp);
			}
		}
#endif // of DHCPS_DNS_OPTIONS
	}
//#ifdef ZTE_GENERAL_ROUTER_SC
	else if( dhcp == DHCP_LAN_RELAY ){
		struct in_addr dhcps,origdhcps;
		char_t *str;
		
		str = websGetVar(wp, T("dhcps"), T(""));
		if ( str[0] ) {
			if ( !inet_aton(str, &dhcps) ) {
				strcpy(tmpBuf, T(strInvalDhcpsAddress));
				goto setErr_dhcpd;
			}
			mib_get(MIB_ADSL_WAN_DHCPS, (void*)&origdhcps);
			if(origdhcps.s_addr != dhcps.s_addr)
				dhcpd_changed_flag = 1;
			if ( !mib_set(MIB_ADSL_WAN_DHCPS, (void *)&dhcps)) {
	  			strcpy(tmpBuf, T(strSetDhcpserror));
				goto setErr_dhcpd;
			}
		}
	}
//#endif	
	vChar = (unsigned char) dhcp;
	// Modify by Mason Yu for dhcpmode
	//if ( !mib_set(MIB_ADSL_LAN_DHCP, (void *)&vChar)) {
	if ( !mib_set(MIB_DHCP_MODE, (void *)&vChar)) {
  		strcpy(tmpBuf, T(strSetDhcpModeerror));
		goto setErr_dhcpd;
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

	if(dhcpd_changed_flag == 1)
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
#if !(defined (ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC))
		mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
#endif

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
	}
	else
	{
		submitUrl = websGetVar(wp, T("submit-url"), T(""));
		if (submitUrl[0])
			websRedirect(wp, submitUrl);
		else
			websDone(wp, 200);
	}
	
	return;

setErr_dhcpd:
	ERR_MSG(tmpBuf);
}


/////////////////////////////////////////////////////////////////////////////
#if 0
//#ifdef EMBED
static int getOneDhcpClient(char **ppStart, unsigned long *size, char *ip, char *mac, char *liveTime)
{
	struct dhcpOfferedAddr {
        	u_int8_t chaddr[16];
        	u_int32_t yiaddr;       /* network order */        	
        	u_int32_t expires;      /* host order */        	
	};

	struct dhcpOfferedAddr entry;
#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
	u_int32_t day,hour,minute,second,time,len=0;
#endif

	if ( *size < sizeof(entry) )
		return -1;

	entry = *((struct dhcpOfferedAddr *)*ppStart);
	*ppStart = *ppStart + sizeof(entry);
	*size = *size - sizeof(entry);

	if (entry.expires == 0) 		
		return 0;
//star: conflict ip addr will not be displayed on web
	if(entry.chaddr[0]==0&&entry.chaddr[1]==0&&entry.chaddr[2]==0&&entry.chaddr[3]==0&&entry.chaddr[4]==0&&entry.chaddr[5]==0)
		return 0;
	
	strcpy(ip, inet_ntoa(*((struct in_addr *)&entry.yiaddr)) );
	#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
	snprintf(mac, 20, "%02x-%02x-%02x-%02x-%02x-%02x",
			entry.chaddr[0],entry.chaddr[1],entry.chaddr[2],entry.chaddr[3],
			entry.chaddr[4], entry.chaddr[5]);
	#else
	snprintf(mac, 20, "%02x:%02x:%02x:%02x:%02x:%02x",
			entry.chaddr[0],entry.chaddr[1],entry.chaddr[2],entry.chaddr[3],
			entry.chaddr[4], entry.chaddr[5]);
	#endif

#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
	time = ntohl(entry.expires);
	if(time == -1){
		strcpy(liveTime, "永不过期");
		return 1;
	}
	day = time/86400;
	hour = (time%86400)/3600;
	minute = ((time%86400)%3600)/60;
	second = ((time%86400)%3600)%60;

	if(day!=0)
		len=snprintf(liveTime, 20, "%lu天", day);
	if(hour!=0)
		len+=snprintf(liveTime+len, 20, "%lu小时", hour);
	if(minute!=0)
		len+=snprintf(liveTime+len, 20, "%lu分", minute);
	if(second!=0)
		snprintf(liveTime+len, 20, "%lu秒", second);
	if(day == 0 && hour == 0 && minute == 0 && second == 0)
		snprintf(liveTime+len, 20, "0秒");
#else
	snprintf(liveTime, 10, "%lu", (unsigned long)ntohl(entry.expires));
#endif

	return 1;
}
#endif

/////////////////////////////////////////////////////////////////////////////
int dhcpClientList(int eid, webs_t wp, int argc, char_t **argv)
{
#ifdef EMBED
	int pid;
	struct stat status;
	int nBytesSent=0;
	int element=0, ret;
	char ipAddr[40], macAddr[40], liveTime[80], *buf=NULL, *ptr;
	FILE *fp;

	// siganl DHCP server to update lease file
	pid = read_pid(_DHCPD_PID_FILE);
	if ( pid > 0)
		kill(pid, SIGUSR1);
	usleep(1000);

	if ( stat(_DHCPD_LEASES_FILE, &status) < 0 ) 		
		goto err;	
	
	// read DHCP server lease file
	buf = malloc(status.st_size);
	if ( buf == NULL ) goto err;
	fp = fopen(_DHCPD_LEASES_FILE, "r");
	if ( fp == NULL )goto err;
	fread(buf, 1, status.st_size, fp);
	fclose(fp);

	ptr = buf;
	while (1) {
		ret = getOneDhcpClient(&ptr, &status.st_size, ipAddr, macAddr, liveTime);		
		
		if (ret < 0)
			break;
		if (ret == 0)
			continue;
		nBytesSent += websWrite(wp,
			T("<tr bgcolor=#b7b7b7><td><font size=2>%s</td><td><font size=2>%s</td><td><font size=2>%s</td></tr>"),
			ipAddr, macAddr, liveTime);
		element++;
	}

err:
	if (element == 0) {
#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
		nBytesSent += websWrite(wp,
			T("<tr bgcolor=#b7b7b7><td><font size=2>无</td><td><font size=2>----</td><td><font size=2>----</td></tr>"));
#else
		nBytesSent += websWrite(wp,
			T("<tr bgcolor=#b7b7b7><td><font size=2>None</td><td><font size=2>----</td><td><font size=2>----</td></tr>"));
#endif
	}
	if (buf)
		free(buf);

	return nBytesSent;
#else
	return 0;
#endif
}

/////////////////////////////////////////////////////////////////////////////
void formReflashClientTbl(webs_t wp, char_t *path, char_t *query)
{
	char_t *submitUrl;

	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
}

//////////////////////////////////////////////////////////////////////////////
int isDhcpClientExist(char *name)
{
/*
	char tmpBuf[100];
	struct in_addr intaddr;
	
	if ( getInAddr(name, IP_ADDR, (void *)&intaddr ) ) {
		snprintf(tmpBuf, 100, "%s/%s-%s.pid", _DHCPC_PID_PATH, _DHCPC_PROG_NAME, name);
		if ( getPid(tmpBuf) > 0)
			return 1;
	}
*/	
	return 0;
}

#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
int show_IP_range(int eid, webs_t wp, int argc, char_t **argv)
{

	int nBytesSent=0;
#ifdef IP_BASED_CLIENT_TYPE
	struct in_addr curIpAddr,curSubnet;
	unsigned char curstart,curend;
	unsigned long v1,v4;
	unsigned char saddr[16];
	unsigned char sstart[6],send[6];

	mib_get( MIB_ADSL_LAN_IP,  (void *)&curIpAddr );
	mib_get( MIB_ADSL_LAN_SUBNET,  (void *)&curSubnet );
	v1 = *((unsigned long *)&curSubnet);
	v4 = *((unsigned long *)&curIpAddr);

	sprintf(saddr,"%d.%d.%d.",(v4&0xff000000)>>24,(v4&0xff0000)>>16,(v4&0xff00)>>8);

// for PC
	mib_get(CWMP_CT_PC_MINADDR, (void*)&curstart);
	mib_get(CWMP_CT_PC_MAXADDR, (void*)&curend);

	sprintf(sstart,"%d",curstart);
	sprintf(send,"%d",curend);

#ifdef ZTE_GENERAL_ROUTER_SC
	nBytesSent += websWrite(wp, T("'<tr>"
	"<td width=\"20%%\"><font size=2><b>计算机IP地址池:</b></font></td>'+\n"
	"'<td width=\"70%%\"><font size=2><b>%s<input type=\"text\" name=\"dhcppcRangeStart\" size=\"3\" maxlength=\"3\" value=%s>"
	"<font face=\"Arial\" size=\"5\">- </font>"
	"%s<input type=\"text\" name=\"dhcppcRangeEnd\" size=\"3\" maxlength=\"3\" value=%s></font>"
	"</td></tr>'+\n"),saddr,sstart,saddr,send);
#else
	nBytesSent += websWrite(wp, T("<tr>"
	"<td width=\"20%%\"><font size=2><b>计算机IP地址池:</b></font></td>\n"
	"<td width=\"70%%\"><font size=2><b>%s<input type=\"text\" name=\"dhcppcRangeStart\" size=\"3\" maxlength=\"3\" value=%s>"
	"<font face=\"Arial\" size=\"5\">- </font>"
	"%s<input type=\"text\" name=\"dhcppcRangeEnd\" size=\"3\" maxlength=\"3\" value=%s></font>"
	"</td></tr>\n"),saddr,sstart,saddr,send);
#endif

//for CMR
	mib_get(CWMP_CT_CMR_MINADDR, (void*)&curstart);
	mib_get(CWMP_CT_CMR_MAXADDR, (void*)&curend);

	sprintf(sstart,"%d",curstart);
	sprintf(send,"%d",curend);

#ifdef ZTE_GENERAL_ROUTER_SC
	nBytesSent += websWrite(wp, T("'<tr>"
	"<td width=\"20%%\"><font size=2><b>摄像头IP地址池:</b></font></td>'+\n"
	"'<td width=\"70%%\"><font size=2><b>%s<input type=\"text\" name=\"dhcpcmrRangeStart\" size=\"3\" maxlength=\"3\" value=%s>"
	"<font face=\"Arial\" size=\"5\">- </font>"
	"%s<input type=\"text\" name=\"dhcpcmrRangeEnd\" size=\"3\" maxlength=\"3\" value=%s></font>"
	"</td></tr>'+\n"),saddr,sstart,saddr,send);
#else
	nBytesSent += websWrite(wp, T("<tr>"
	"<td width=\"20%%\"><font size=2><b>摄像头IP地址池:</b></font></td>\n"
	"<td width=\"70%%\"><font size=2><b>%s<input type=\"text\" name=\"dhcpcmrRangeStart\" size=\"3\" maxlength=\"3\" value=%s>"
	"<font face=\"Arial\" size=\"5\">- </font>"
	"%s<input type=\"text\" name=\"dhcpcmrRangeEnd\" size=\"3\" maxlength=\"3\" value=%s></font>"
	"</td></tr>\n"),saddr,sstart,saddr,send);
#endif

//for STB
	mib_get(CWMP_CT_STB_MINADDR, (void*)&curstart);
	mib_get(CWMP_CT_STB_MAXADDR, (void*)&curend);

	sprintf(sstart,"%d",curstart);
	sprintf(send,"%d",curend);

#ifdef ZTE_GENERAL_ROUTER_SC
	nBytesSent += websWrite(wp, T("'<tr>"
	"<td width=\"20%%\"><font size=2><b>机顶盒IP地址池:</b></font></td>'+\n"
	"'<td width=\"70%%\"><font size=2><b>%s<input type=\"text\" name=\"dhcpstbRangeStart\" size=\"3\" maxlength=\"3\" value=%s>"
	"<font face=\"Arial\" size=\"5\">- </font>"
	"%s<input type=\"text\" name=\"dhcpstbRangeEnd\" size=\"3\" maxlength=\"3\" value=%s></font>"
	"</td></tr>'+\n"),saddr,sstart,saddr,send);
#else
	nBytesSent += websWrite(wp, T("<tr>"
	"<td width=\"20%%\"><font size=2><b>机顶盒IP地址池:</b></font></td>\n"
	"<td width=\"70%%\"><font size=2><b>%s<input type=\"text\" name=\"dhcpstbRangeStart\" size=3\"3\"gth=\"3\" value=%s>"
	"<font face=\"Arial\" size=\"5\">- </font>"
	"%s<input type=\"text\" name=\"dhcpstbRangeEnd\" size=\"3\" maxlength=\"3\" value=%s></font>"
	"</td></tr>\n"),saddr,sstart,saddr,send);
#endif

//for PHN
	mib_get(CWMP_CT_PHN_MINADDR, (void*)&curstart);
	mib_get(CWMP_CT_PHN_MAXADDR, (void*)&curend);

	sprintf(sstart,"%d",curstart);
	sprintf(send,"%d",curend);

#ifdef ZTE_GENERAL_ROUTER_SC
	nBytesSent += websWrite(wp, T("'<tr>"
	"<td width=\"20%%\"><font size=2><b>电话机IP地址池:</b></font></td>'+\n"
	"'<td width=\"70%%\"><font size=2><b>%s<input type=\"text\" name=\"dhcpphnRangeStart\" size=\"3\" maxlength=\"3\" value=%s>"
	"<font face=\"Arial\" size=\"5\">- </font>"
	"%s<input type=\"text\" name=\"dhcpphnRangeEnd\" size=\"3\" maxlength=\"3\" value=%s></font>"
	"</td></tr>'+\n"),saddr,sstart,saddr,send);
#else
	nBytesSent += websWrite(wp, T("<tr>"
	"<td width=\"20%%\"><font size=2><b>电话机IP地址池:</b></font></td>\n"
	"<td width=\"70%%\"><font size=2><b>%s<input type=\"text\" name=\"dhcpphnRangeStart\" size=\"3\" maxlength=\"3\" value=%s>"
	"<font face=\"Arial\" size=\"5\">- </font>"
	"%s<input type=\"text\" name=\"dhcpphnRangeEnd\" size=\"3\" maxlength=\"3\" value=%s></font>"
	"</td></tr>\n"),saddr,sstart,saddr,send);
#endif
#else
#ifdef ZTE_GENERAL_ROUTER_SC
	nBytesSent += websWrite(wp, T("' '+"));
#endif
#endif

	return 0;

}
#endif


