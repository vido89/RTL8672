/* startup.c - kaohj */	

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/route.h>
#include <linux/atm.h>
#include <linux/atmdev.h>
#include <crypt.h>
#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../../uClibc/include/linux/autoconf.h"
#endif


#include "mibtbl.h"
#include "utility.h"
#ifdef WLAN_SUPPORT

#ifdef _LINUX_2_6_
#include "wireless.h"
#else
#include <linux/wireless.h>
#endif // 2.6

#endif

#include "debug.h"
// Mason Yu
#include "syslog.h"

// Added by Mason Yu for ILMI(PVC) community string
const char PVCREADSTR[] = "ADSL";
const char PVCWRITESTR[] = "ADSL";

#ifdef WLAN_MBSSID
extern const char *WLANAPIF[];
#endif

int startLANAutoSearch(const char *ipAddr, const char *subnet);
int isDuplicate(struct in_addr *ipAddr, const char *device);

#if 0
#ifdef CONFIG_EXT_SWITCH
// br to pvc
struct brmap brInfo[SW_PORT_NUM];
#endif
#endif

//--------------------------------------------------------
// xDSL startup
// return value:
// 0  : not start by configuration
// 1  : successful
// -1 : failed
static char tone[64];
int startDsl()
{
	unsigned char init_line, olr;
	unsigned short dsl_mode;
	int val;
	int ret=1;
	BOOT_TYPE_T btmode;
	
	/*
	mib_get(MIB_ADSL_OLR, (void *)&olr);
	
	val = (int)olr;
	if (val == 2) // SRA (should include bitswap)
		val = 3;
	
	adsl_drv_get(RLCM_SET_OLR_TYPE, (void *)&val, 4);
	*/
	
	btmode = mib_get_bootmode();
	
	//if (btmode != BOOT_UPGRADE)
		// check INIT_LINE
		if (mib_get(MIB_INIT_LINE, (void *)&init_line) != 0)
		{
			if (init_line == 1)
			{
				char mode[3], inp;
				int xmode,adslmode, axB, axM, axL;
				
				// start adsl
				mib_get(MIB_ADSL_MODE, (void *)&dsl_mode);
				
				adslmode=(int)(dsl_mode & (ADSL_MODE_GLITE|ADSL_MODE_T1413|ADSL_MODE_GDMT));	// T1.413 & G.dmt
				adsl_drv_get(RLCM_PHY_START_MODEM, (void *)&adslmode, 4);
				
				if (dsl_mode & ADSL_MODE_ANXB) {	// Annex B
					axB = 1;
					axL = 0;
					axM = 0;
				}
				else {	// Annex A
					axB = 0;
					if (dsl_mode & ADSL_MODE_ANXL)	// Annex L
						axL = 3; // Wide-Band & Narrow-Band Mode
					else
						axL = 0;
					if (dsl_mode & ADSL_MODE_ANXM)	// Annex M
						axM = 1;
					else
						axM = 0;
				}

				adsl_drv_get(RLCM_SET_ANNEX_B, (void *)&axB, 4);
				adsl_drv_get(RLCM_SET_ANNEX_L, (void *)&axL, 4);
				adsl_drv_get(RLCM_SET_ANNEX_M, (void *)&axM, 4);
				
				xmode=0; 
				if (dsl_mode & (ADSL_MODE_GLITE|ADSL_MODE_T1413|ADSL_MODE_GDMT))
					xmode |= 1;	// ADSL1
				if (dsl_mode & ADSL_MODE_ADSL2)
					xmode |= 2;	// ADSL2
				if (dsl_mode & ADSL_MODE_ADSL2P)
					xmode |= 4;	// ADSL2+
				adsl_drv_get(RLCM_SET_XDSL_MODE, (void *)&xmode, 4);
				
				adsl_drv_get(RLCM_SET_ADSL_MODE, (void *)&adslmode, 4);	
				
				// set OLR Type
				mib_get(MIB_ADSL_OLR, (void *)&olr);
				
				val = (int)olr;
				if (val == 2) // SRA (should include bitswap)
					val = 3;
				
				adsl_drv_get(RLCM_SET_OLR_TYPE, (void *)&val, 4);
				
				// set Tone mask
				mib_get(MIB_ADSL_TONE, (void *)tone);
				adsl_drv_get(RLCM_LOADCARRIERMASK, (void *)tone, GET_LOADCARRIERMASK_SIZE);
				
				mib_get(MIB_ADSL_HIGH_INP, (void *)&inp);
				xmode = inp;
				adsl_drv_get(RLCM_SET_HIGH_INP, (void *)&xmode, 4);
				
				/*
				if (mib_get(MIB_ADSL_MODE, (void *)&dsl_mode) != 0)
				{
					snprintf(mode, 3, "%u", (int)dsl_mode);
					mode[2] = '\0';
				}
				va_cmd(ADSLCTRL, 2, 1, "startAdsl", mode);
				*/
				// new_hibrid
				mib_get(MIB_DEVICE_TYPE, (void *)&inp);
				xmode = inp;
				if (xmode == 1) {
					xmode = 1052;
					adsl_drv_get(RLCM_SET_HYBRID, (void *)&xmode, 4);
				}
				else if (xmode == 2) {
					xmode = 2099;
					adsl_drv_get(RLCM_SET_HYBRID, (void *)&xmode, 4);
				}
				else if (xmode == 3) {
					xmode = 3099;
					adsl_drv_get(RLCM_SET_HYBRID, (void *)&xmode, 4);
				}
				else if (xmode == 4) {
					xmode = 4052;
					adsl_drv_get(RLCM_SET_HYBRID, (void *)&xmode, 4);
				}
				else if (xmode == 5) {
					xmode = 5099;
					adsl_drv_get(RLCM_SET_HYBRID, (void *)&xmode, 4);
				}
				ret = 1;
			}
			else
				ret = 0;
		}
	
	return ret;
}



//--------------------------------------------------------
// Find the minimun WLAN-side link MRU
// It is used to set the LAN-side MTU(MRU) for the
// path-mtu problem.
// RETURN: 0: if failed
//	 : others: the minimum MRU for the WLAN link
static int get_min_wan_mru()
{
	int vcTotal, i, pmtu;
	MIB_CE_ATM_VC_T Entry;
	
	vcTotal = mib_chain_total(MIB_ATM_VC_TBL);
	pmtu = 1500;
	
	for (i = 0; i < vcTotal; i++)
	{
		/* get the specified chain record */
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			return 0;
		
		if (Entry.enable == 0)
			continue;
		
		if (Entry.mtu < pmtu)
			pmtu = Entry.mtu;
	}
	
	return pmtu;
}

//--------------------------------------------------------
// Ethernet LAN startup
// return value:
// 1  : successful
// -1 : failed
int startELan()
{
	unsigned char value[6];
	int vInt;
	char macaddr[13];
	char ipaddr[16];
	char subnet[16];
	char timeOut[6];
	int status=0;
#ifdef WLAN_MBSSID
	int i;
	char para2[20];
#endif	

	// ifconfig eth0 hw ether [macaddr]
	// ifconfig wlan0 hw ether [macaddr]

	#ifdef WIFI_TEST
	//for wifi test
	mib_get(MIB_WLAN_BAND, (void *)value);
	if(value[0]==4 || value[0]==5){//wifi
		status|=va_cmd("/bin/ethctl",2,1,"wifi","1");
	}
	else
	#endif
	{
#ifdef WLAN_SUPPORT
	// to support WIFI logo test mode.....
	mib_get(MIB_WIFI_SUPPORT, (void*)value);
	if(value[0]==1)
	{
		mib_get(MIB_WLAN_BAND, (void *)value);
		if(value[0]==2 || value[0]==3)
			status|=va_cmd("/bin/ethctl",2,1,"wifi","1");
		else
			status|=va_cmd("/bin/ethctl",2,1,"wifi","0");
	}
	else
		status|=va_cmd("/bin/ethctl",2,1,"wifi","0");
#endif
	}
	
	if (mib_get(MIB_ELAN_MAC_ADDR, (void *)value) != 0)
	{
#ifndef WLAN_MBSSID
		snprintf(macaddr, 13, "%02x%02x%02x%02x%02x%02x",
			value[0], value[1], value[2], value[3], value[4], value[5]);
		status|=va_cmd(IFCONFIG, 4, 1, "eth0", "hw", "ether", macaddr);
#ifdef CONFIG_USB_ETH
		status|=va_cmd(IFCONFIG, 4, 1, USBETHIF, "hw", "ether", macaddr);
#endif //CONFIG_USB_ETH
#ifdef WLAN_SUPPORT
		status|=va_cmd(IFCONFIG, 4, 1, WLANIF, "hw", "ether", macaddr);
#endif

#else

	snprintf(macaddr, 13, "%02x%02x%02x%02x%02x%02x",
			value[0], value[1], value[2], value[3], value[4], value[5]);
		status|=va_cmd(IFCONFIG, 4, 1, "eth0", "hw", "ether", macaddr);
#ifdef CONFIG_USB_ETH
		status|=va_cmd(IFCONFIG, 4, 1, USBETHIF, "hw", "ether", macaddr);
#endif //CONFIG_USB_ETH
#ifdef WLAN_SUPPORT
		status|=va_cmd(IFCONFIG, 4, 1, WLANIF, "hw", "ether", macaddr);
#endif

	// Set macaddr for VAP
	for (i=1; i<=4; i++) {
		value[5] = value[5] + 1;
		
		snprintf(macaddr, 13, "%02x%02x%02x%02x%02x%02x",
			value[0], value[1], value[2], value[3], value[4], value[5]);
		
		sprintf(para2, "wlan0-vap%d", i-1);
		
		status|=va_cmd(IFCONFIG, 4, 1, para2, "hw", "ether", macaddr);		
	}
#endif
	}
	
	// ifconfig eth0 up
	//va_cmd(IFCONFIG, 2, 1, "eth0", "up");
	
#ifdef CONFIG_EXT_SWITCH
#ifdef ELAN_LINK_MODE
	//setupLinkMode();
#endif
#endif

	// brctl addbr br0
	status|=va_cmd(BRCTL, 2, 1, "addbr", (char*)BRIF);
	
	if (mib_get(MIB_BRCTL_STP, (void *)value) != 0)
	{
		vInt = (int)(*(unsigned char *)value);
		if (vInt == 0)	// stp off
		{
			// brctl stp br0 off
			status|=va_cmd(BRCTL, 3, 1, "stp", (char *)BRIF, "off");
			
			// brctl setfd br0 1
			//if forwarding_delay=0, fdb_get may fail in serveral seconds after booting
			status|=va_cmd(BRCTL, 3, 1, "setfd", (char *)BRIF, "1");
		}
		else		// stp on
		{
			// brctl stp br0 on
			status|=va_cmd(BRCTL, 3, 1, "stp", (char *)BRIF, "on");
		}
	}
		
	// brctl setageing br0 ageingtime
	if (mib_get(MIB_BRCTL_AGEINGTIME, (void *)value) != 0)
	{
		vInt = (int)(*(unsigned short *)value);
		snprintf(timeOut, 6, "%u", vInt);
		status|=va_cmd(BRCTL, 3, 1, "setageing", (char *)BRIF, timeOut);
	}
	
	// brctl addif br0 eth0
	status|=va_cmd(BRCTL, 3, 1, "addif", (char *)BRIF, "eth0");
#ifdef CONFIG_USB_ETH
	status|=va_cmd(BRCTL, 3, 1, "addif", (char *)BRIF, USBETHIF);
#endif //CONFIG_USB_ETH
	
	// ifconfig LANIF LAN_IP netmask LAN_SUBNET
	if (mib_get(MIB_ADSL_LAN_IP, (void *)value) != 0)
	{
		strncpy(ipaddr, inet_ntoa(*((struct in_addr *)value)), 16);
		ipaddr[15] = '\0';
	}
	if (mib_get(MIB_ADSL_LAN_SUBNET, (void *)value) != 0)
	{
		strncpy(subnet, inet_ntoa(*((struct in_addr *)value)), 16);
		subnet[15] = '\0';
	}
	
	// get the minumum MRU for all WLAN-side link
	/* marked by Jenny
	vInt = get_min_wan_mru();
	if (vInt==0) */
		vInt = 1500;
	snprintf(value, 6, "%d", vInt);
	// set LAN-side MRU
	status|=va_cmd(IFCONFIG, 6, 1, (char*)LANIF, ipaddr, "netmask", subnet, "mtu", value);
	
#ifdef SECONDARY_IP
	mib_get(MIB_ADSL_LAN_ENABLE_IP2, (void *)value);
	if (value[0] == 1) {
		// ifconfig LANIF LAN_IP netmask LAN_SUBNET
		if (mib_get(MIB_ADSL_LAN_IP2, (void *)value) != 0)
		{
			strncpy(ipaddr, inet_ntoa(*((struct in_addr *)value)), 16);
			ipaddr[15] = '\0';
		}
		if (mib_get(MIB_ADSL_LAN_SUBNET2, (void *)value) != 0)
		{
			strncpy(subnet, inet_ntoa(*((struct in_addr *)value)), 16);
			subnet[15] = '\0';
		}
		snprintf(value, 6, "%d", vInt);
		// set LAN-side MRU
		status|=va_cmd(IFCONFIG, 6, 1, (char*)"br0:0", ipaddr, "netmask", subnet, "mtu", value);
	}
#endif

	if (mib_get(MIB_ADSL_LAN_AUTOSEARCH, (void *)value) != 0)
	{
		if (value[0] == 1)	// enable LAN ip autosearch
		{
			// check if dhcp server on ? per TR-068, I-190
			// Modified by Mason Yu for dhcpmode
			// if (mib_get(MIB_ADSL_LAN_DHCP, (void *)value) != 0)
			if (mib_get(MIB_DHCP_MODE, (void *)value) != 0)
			{
				if (value[0] != 2)	// dhcp server is disabled
				{
					usleep(2000000); // wait 2 sec for br0 ready
					startLANAutoSearch(ipaddr, subnet);
				}
			}
		}
	}
	
	if (status)  //start fail
	    return -1;
	
	//start success
    return 1;
}

#if 0
int check_for_rip()
{
	unsigned int num;
	unsigned char uChar;
	MIB_CE_ATM_VC_Tp pEntry;
	
	// --- Check LAN side
	if (mib_get(MIB_ADSL_LAN_DHCP, (void *)&uChar) != 0)
	{
		if (uChar != 0)
			return 0;	// dhcp server not disabled
	}
	
	// --- Check WAN side
	if (mib_chain_total(MIB_ATM_VC_TBL) != 1)
		return 0;
	pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL,0);
	if(!pEntry)
		return 0;
	if (pEntry->cmode != ADSL_RT1483 && pEntry->cmode != ADSL_MER1483)
		return 0;
	if(pEntry->napt != 0)
		return 0;	// napt not disabled
	if(pEntry->ipDhcp != 0)
		return 0;	// not fixed ip
	
	return 1;
}
#endif


static const char DHCPD_CONF[] = "/var/udhcpd/udhcpd.conf";
static const char DHCPD_LEASE[] = "/var/udhcpd/udhcpd.leases";
#if 0
// DHCP server configuration
// return value:
// 0  : not active
// 1  : active
// -1 : setup failed
int setupDhcpd()
{
	unsigned char value[32];
	unsigned int uInt, uLTime;
	DNS_TYPE_T dnsMode;
	FILE *fp, *fp2;
	char ipstart[16], ipend[16];
	char subnet[16], ipaddr[16], ipaddr2[16];
	char dns1[16], dns2[16], dns3[16], dhcps[16];
	char domain[MAX_NAME_LEN];
	unsigned int entryNum, i, j;
	MIB_CE_MAC_BASE_DHCP_T Entry;		
#ifdef IP_PASSTHROUGH
	int ippt;
#endif
	int spc_enable, spc_ip;
	
	// Added by Mason Yu for MAC base assignment. Start			
	if ((fp2 = fopen("/var/dhcpdMacBase.txt", "w")) == NULL)
	{
		printf("***** Open file /var/dhcpdMacBase.txt failed !\n");	
		goto dhcpConf;		
	}
	
	entryNum = mib_chain_total(MIB_MAC_BASE_DHCP_TBL);
	
	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_MAC_BASE_DHCP_TBL, i, (void *)&Entry))
		{
  			printf("setupDhcpd:Get chain(MIB_MAC_BASE_DHCP_TBL) record error!\n");			
		}
		
		for (j=0; j<17; j++){
			if ( Entry.macAddr[j] != '-' ) {
				Entry.macAddr[j] = tolower(Entry.macAddr[j]);
			}	
		}	
		
		fprintf(fp2, "%s: %s\n", Entry.macAddr, Entry.ipAddr);				
	}			
	fclose(fp2);
	// Added by Mason Yu for MAC base assignment. End	
dhcpConf:
		
	// check if dhcp server on ?
	// Modified by Mason Yu for dhcpmode
	//if (mib_get(MIB_ADSL_LAN_DHCP, (void *)value) != 0)
	if (mib_get(MIB_DHCP_MODE, (void *)value) != 0)
	{
		uInt = (unsigned int)(*(unsigned char *)value);
		if (uInt != 2 )
			return 0;	// dhcp Server not on
	}		
	
	
	if (mib_get(MIB_ADSL_LAN_SUBNET, (void *)value) != 0)
	{
		strncpy(subnet, inet_ntoa(*((struct in_addr *)value)), 16);
		subnet[15] = '\0';
	}
	else
		return -1;
	
	if (mib_get(MIB_ADSL_LAN_IP, (void *)value) != 0)
	{
		strncpy(ipaddr, inet_ntoa(*((struct in_addr *)value)), 16);
		ipaddr[15] = '\0';
	}
	else
		return -1;
	
	// IP pool start address
	if (mib_get(MIB_ADSL_LAN_CLIENT_START, (void *)&value[3]) != 0)
	{
		strncpy(ipstart, inet_ntoa(*((struct in_addr *)value)), 16);
		ipstart[15] = '\0';
	}
	else
		return -1;
	
	// IP pool end address
	if (mib_get(MIB_ADSL_LAN_CLIENT_END, (void *)&value[3]) != 0)
	{
		strncpy(ipend, inet_ntoa(*((struct in_addr *)value)), 16);
		ipend[15] = '\0';
	}
	else
		return -1;
	
	// IP max lease time
	if (mib_get(MIB_ADSL_LAN_DHCP_LEASE, (void *)&uLTime) == 0)
		return -1;
	
	if (mib_get(MIB_ADSL_LAN_DHCP_DOMAIN, (void *)domain) == 0)
		return -1;
	
	// get DNS mode
	if (mib_get(MIB_ADSL_WAN_DNS_MODE, (void *)value) != 0)
	{
		dnsMode = (DNS_TYPE_T)(*(unsigned char *)value);
	}
	else
		return -1;

	// Commented by Mason Yu for LAN_IP as DNS Server
#if 0		
	if (mib_get(MIB_ADSL_WAN_DNS1, (void *)value) != 0)
	{
		if (((struct in_addr *)value)->s_addr != 0)
		{
			strncpy(dns1, inet_ntoa(*((struct in_addr *)value)), 16);
			dns1[15] = '\0';
		}
		else
			dns1[0] = '\0';
	}
	else
		return -1;
	
	if (mib_get(MIB_ADSL_WAN_DNS2, (void *)value) != 0)
	{
		if (((struct in_addr *)value)->s_addr != 0)
		{
			strncpy(dns2, inet_ntoa(*((struct in_addr *)value)), 16);
			dns2[15] = '\0';
		}
		else
			dns2[0] = '\0';
	}
	else
		return -1;
	
	if (mib_get(MIB_ADSL_WAN_DNS3, (void *)value) != 0)
	{
		if (((struct in_addr *)value)->s_addr != 0)
		{
			strncpy(dns3, inet_ntoa(*((struct in_addr *)value)), 16);
			dns3[15] = '\0';
		}
		else
			dns3[0] = '\0';
	}
	else
		return -1;
#endif
		
	
	if ((fp = fopen(DHCPD_CONF, "w")) == NULL)
	{
		printf("Open file %s failed !\n", DHCPD_CONF);
		return -1;
	}
	
	if (mib_get(MIB_SPC_ENABLE, (void *)value) != 0)
	{
		if (value[0])
		{
			spc_enable = 1;
			mib_get(MIB_SPC_IPTYPE, (void *)value);
			spc_ip = (unsigned int)(*(unsigned char *)value);
		}
		else
			spc_enable = 0;
	}
	
	fprintf(fp, "interface %s\n", LANIF);
	if (spc_enable)
	{
		if (spc_ip == 0) { // single private ip
			fprintf(fp, "start %s\n", ipstart);
			fprintf(fp, "end %s\n", ipstart);
		}
		//else { // single shared ip
	}
	else
	{
		fprintf(fp, "start %s\n", ipstart);
		fprintf(fp, "end %s\n", ipend);
	}
	
#ifdef IP_PASSTHROUGH
	ippt = 0;
	if (mib_get(MIB_IPPT_ITF, (void *)value) != 0)
		if (value[0] != 255) // IP passthrough
			ippt = 1;
	
	if (ippt)
	{
		fprintf(fp, "ippt yes\n");
		mib_get(MIB_IPPT_LEASE, (void *)&uInt);
		fprintf(fp, "ipptlt %d\n", uInt);
	}
#endif
	
	fprintf(fp, "opt subnet %s\n", subnet);
	
	// Added by Mason Yu
	if (mib_get(MIB_ADSL_LAN_DHCP_GATEWAY, (void *)value) != 0)
	{
		strncpy(ipaddr2, inet_ntoa(*((struct in_addr *)value)), 16);
		ipaddr2[15] = '\0';
	}
	else
		return -1;
	fprintf(fp, "opt router %s\n", ipaddr2);
	
	// Modified by Mason Yu for LAN_IP as DNS Server
#if 0	
	if (dnsMode == DNS_AUTO)
	{
		fprintf(fp, "opt dns %s\n", ipaddr);
	}
	else	// DNS_MANUAL
	{
		if (dns1[0])
			fprintf(fp, "opt dns %s\n", dns1);
		if (dns2[0])
			fprintf(fp, "opt dns %s\n", dns2);
		if (dns3[0])
			fprintf(fp, "opt dns %s\n", dns3);
	}
#endif
	
	fprintf(fp, "opt dns %s\n", ipaddr);
	fprintf(fp, "opt lease %u\n", uLTime);
	if (domain[0])
		fprintf(fp, "opt domain %s\n", domain);
	else
		// per TR-068, I-188
		fprintf(fp, "opt domain domain_not_set.invalid\n");

#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
{
	//format: opt venspec [enterprise-number] [sub-option code] [sub-option data] ...
	//opt vendspec: dhcp option 125 Vendor-Identifying Vendor-Specific
	//enterprise-number: 3561(ADSL Forum)
	//sub-option code: 4(GatewayManufacturerOUI)
	//sub-option code: 5(GatewaySerialNumber)
	//sub-option code: 6(GatewayProductClass)
#if 1
	char opt125_buf[160];
	char opt125_sn[65]="";
	char opt125_oui[]="00E04C";
	
	if(mib_get(CWMP_SERIALNUMBER, (void *)opt125_sn) != 0)
	{
		sprintf( opt125_buf, "opt venspec 3561 4 %s 5 %s 6 IGD\n", opt125_oui, opt125_sn );
		fprintf( fp, "%s", opt125_buf);
	}
#else
	fprintf( fp, "opt venspec 3561 4 00e0d4 5 00000001 6 IGD\n" );
#endif
}
#endif
	
	fclose(fp);
	
	if ((fp = fopen(DHCPD_LEASE, "w")) == NULL)
	{
		printf("Open file %s failed !\n", DHCPD_LEASE);
		return -1;
	}
	
	fprintf(fp, "\n");
	fclose(fp);
	return 1;
}



// Added by Mason Yu for Dhcp Relay
// return value:
// 0  : not active
// 1  : active
// -1 : setup failed
int startDhcpRelay(void)
{
	unsigned char value[32];
	unsigned int dhcpmode;	
	char dhcps[16];
	int status=0;
	
	
	if (mib_get(MIB_DHCP_MODE, (void *)value) != 0)
	{
		dhcpmode = (unsigned int)(*(unsigned char *)value);
		if (dhcpmode != 1 )
			return 0;	// dhcp Relay not on
	}
	
	// DHCP Relay is on
	if (dhcpmode == 1) {
		
		//printf("DHCP Relay is on\n");
		
		if (mib_get(MIB_ADSL_WAN_DHCPS, (void *)value) != 0)
		{
			if (((struct in_addr *)value)->s_addr != 0)
			{
				strncpy(dhcps, inet_ntoa(*((struct in_addr *)value)), 16);
				dhcps[15] = '\0';
			}
			else
				dhcps[0] = '\0';
		}
		else
			return -1;
		
		//printf("dhcps = %s\n", dhcps);
		status=va_cmd("/bin/dhcrelay", 1, 0, dhcps);
		status=(status==-1)?-1:1;
			
		return status;
		
	}
	
}
#endif

#if 0
//jim moved to utility.c
// Added by Mason Yu for write community string
static const char SNMPCOMMSTR[] = "/var/snmpComStr.conf";
// return value:
// 1  : successful
// -1 : startup failed
int startSnmp(void)
{
	unsigned char value[16];
	unsigned char trapip[16];	
	unsigned char commRW[100], commRO[100], enterOID[100];
	FILE 	      *fp;
	
	// Get SNMP Trap Host IP Address
	if(!mib_get( MIB_SNMP_TRAP_IP,  (void *)value)){
		printf("Can no read MIB_SNMP_TRAP_IP\n");
	}
	
	if (((struct in_addr *)value)->s_addr != 0)
	{
		strncpy(trapip, inet_ntoa(*((struct in_addr *)value)), 16);
		trapip[15] = '\0';
	}
	else
		trapip[0] = '\0';		
	//printf("***** trapip = %s\n", trapip);	
	
	// Get CommunityRO String
	if(!mib_get( MIB_SNMP_COMM_RO,  (void *)commRO)) {
		printf("Can no read MIB_SNMP_COMM_RO\n");	
	}		
	//printf("*****buffer = %s\n", commRO);
	
			
	// Get CommunityRW String
	if(!mib_get( MIB_SNMP_COMM_RW,  (void *)commRW)) {
		printf("Can no read MIB_SNMP_COMM_RW\n");	
	}		
	//printf("*****commRW = %s\n", commRW);   
	
	
	// Get Enterprise OID
	if(!mib_get( MIB_SNMP_SYS_OID,  (void *)enterOID)) {
		printf("Can no read MIB_SNMP_SYS_OID\n");
	}
	//printf("*****enterOID = %s\n", enterOID); 
	
	
	// Write community string to file
	if ((fp = fopen(SNMPCOMMSTR, "w")) == NULL)
	{
		printf("Open file %s failed !\n", SNMPCOMMSTR);
		return -1;
	}
	
	if (commRO[0])
		fprintf(fp, "readStr %s\n", commRO);
	if (commRW[0])
		fprintf(fp, "writeStr %s\n", commRW);	
		
	// Add ILMI(PVC) community string
	fprintf(fp, "PvcReadStr %s\n", PVCREADSTR);
	fprintf(fp, "PvcWriteStr %s\n", PVCWRITESTR);	
	fclose(fp);
	
	// Mason Yu
	// ZyXEL Remote management does not verify the comm string, so we can limit the comm string as "ADSL"
	if (va_cmd("/bin/snmpd", 8, 0, "-p", "161", "-c", PVCREADSTR, "-th", trapip, "-te", enterOID))
	    return -1;
	return 1;
	
}	
#endif

#if 0
static const char RESOLV[] = "/var/resolv.conf";
static const char MANUAL_RESOLV[] = "/var/resolv.manual.conf";
static const char DNS_RESOLV[] = "/var/udhcpc/resolv.conf";
static const char PPP_RESOLV[] = "/etc/ppp/resolv.conf";
static const char HOSTS[] = "/etc/config/hosts";

// DNS relay server configuration
// return value:
// 1  : successful
// -1 : startup failed
int startDnsRelay(BOOT_TYPE_T mode)
{
	unsigned char value[32];
	FILE *fp;
	DNS_TYPE_T dnsMode;
	char *str;
	unsigned int i, vcTotal, resolvopt;
	char dns1[16], dns2[16], dns3[16];
	char domain[MAX_NAME_LEN];
	
	if (mode != BOOT_LAST)
		return 1;
	
	if (mib_get(MIB_ADSL_WAN_DNS1, (void *)value) != 0)
	{
		if (((struct in_addr *)value)->s_addr != INADDR_NONE)
		{
			strncpy(dns1, inet_ntoa(*((struct in_addr *)value)), 16);
			dns1[15] = '\0';
		}
		else
			dns1[0] = '\0';
	}
	else
		return -1;
	
	if (mib_get(MIB_ADSL_WAN_DNS2, (void *)value) != 0)
	{
		if (((struct in_addr *)value)->s_addr != INADDR_NONE)
		{
			strncpy(dns2, inet_ntoa(*((struct in_addr *)value)), 16);
			dns2[15] = '\0';
		}
		else
			dns2[0] = '\0';
	}
	else
		return -1;
	
	if (mib_get(MIB_ADSL_WAN_DNS3, (void *)value) != 0)
	{
		if (((struct in_addr *)value)->s_addr != INADDR_NONE)
		{
			strncpy(dns3, inet_ntoa(*((struct in_addr *)value)), 16);
			dns3[15] = '\0';
		}
		else
			dns3[0] = '\0';
	}
	else
		return -1;
	
	// get DNS mode
	if (mib_get(MIB_ADSL_WAN_DNS_MODE, (void *)value) != 0)
	{
		dnsMode = (DNS_TYPE_T)(*(unsigned char *)value);
	}
	else
		return -1;
	
	if ((fp = fopen(MANUAL_RESOLV, "w")) == NULL)
	{
		printf("Open file %s failed !\n", MANUAL_RESOLV);
		return -1;
	}
	
	if (dns1[0])
		fprintf(fp, "nameserver %s\n", dns1);
	if (dns2[0])
		fprintf(fp, "nameserver %s\n", dns2);
	if (dns3[0])
		fprintf(fp, "nameserver %s\n", dns3);
	fclose(fp);
	
	if (dnsMode == DNS_MANUAL)
	{
		// dnsmasq -h -i LANIF -r MANUAL_RESOLV
		TRACE(STA_INFO, "get DNS from manual\n");
		str=(char *)MANUAL_RESOLV;
	}
	else	// DNS_AUTO
	{
		MIB_CE_ATM_VC_T Entry;
		
		resolvopt = 0;
		vcTotal = mib_chain_total(MIB_ATM_VC_TBL);
		
		for (i = 0; i < vcTotal; i++)
		{
			/* get the specified chain record */
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
				return -1;
			
			if (Entry.enable == 0)
				continue;
			
			if ((REMOTE_ACCESS_T)Entry.cmode == ACC_PPPOE ||
				(REMOTE_ACCESS_T)Entry.cmode == ACC_PPPOA)
				resolvopt = 1;
			else if ((DHCP_T)Entry.ipDhcp == DHCP_CLIENT)
				resolvopt = 2;
		}
		
		if (resolvopt == 1) // get from PPP
		{
			// dnsmasq -h -i LANIF -r PPP_RESOLV
			TRACE(STA_INFO, "get DNS from PPP\n");
			str=(char *)PPP_RESOLV;
		}
		else if (resolvopt == 2) // get from DHCP client
		{
			// dnsmasq -h -i LANIF -r DNS_RESOLV
			TRACE(STA_INFO, "get DNS from DHCP client\n");
			str=(char *)DNS_RESOLV;
		}
		else	// get from manual
		{
			// dnsmasq -h -i LANIF -r MANUAL_RESOLV
			TRACE(STA_INFO, "get DNS from manual\n");
			str=(char *)MANUAL_RESOLV;
		}
	}
	
	// create hosts file
	if ((fp = fopen(HOSTS, "w")) == NULL)
	{
		printf("Open file %s failed !\n", HOSTS);
		return -1;
	}
	
	// add DNS entry "dsldevice" for its own address
	mib_get(MIB_ADSL_LAN_IP, (void *)value);
	fprintf(fp, "%s\t", inet_ntoa(*((struct in_addr *)value)));
	// Modified by Mason Yu for dhcpmode
	//mib_get(MIB_ADSL_LAN_DHCP, (void *)value);
	mib_get(MIB_DHCP_MODE, (void *)value);
	
	if (value[0] == DHCP_SERVER)
	{	// add domain
		mib_get(MIB_ADSL_LAN_DHCP_DOMAIN, (void *)domain);
		if (domain[0])
			fprintf(fp, "dsldevice.%s ", domain);
	}
	
	fprintf(fp, "dsldevice\n");
	fclose(fp);

	#if 0
	//va_cmd(DNSRELAY, 4, 0, (char *)ARG_I, (char *)LANIF, "-r", str);
	if (va_cmd(DNSRELAY, 2, 0, "-r", str))
	    return -1;
	#endif
	if (symlink(str, RESOLV)) {
		printf("failed to link %s --> %s\n", str, RESOLV);
		return -1;
	}
	
	if (va_cmd(DNSRELAY, 2, 0, "-r", RESOLV))
	    return -1;
	
	return 1;
}

#endif

// return value:
// 0  : not active
// 1  : successful
// -1 : startup failed
int setupService(BOOT_TYPE_T mode)
{
	MIB_CE_ACC_T Entry;
	char *argv[15];
	int status=0;
	
	if (mode != BOOT_LAST)
		return 1;
	
	if (!mib_chain_get(MIB_ACC_TBL, 0, (void *)&Entry))
		return 0;
	
	/* run as console web
	if (pEntry->web !=0) {
		// start webs
		va_cmd(WEBSERVER, 0, 0);
	}
	*/
#ifndef ZTE_531B_BRIDGE_SC	
	//if (pEntry->snmp !=0) {
		// start snmpd
		// Commented by Mason Yu
		// We use new version
		//va_cmd(SNMPD, 0, 0);
		// Add by Mason Yu for start SnmpV2Trap
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
#ifdef ZTE_GENERAL_ROUTER_SC
	char vChar;
	mib_get(MIB_SNMP_AUTORUN, (void *)&vChar);
	if(vChar==1)
		status = startSnmp();
#else
	char vChar;
	mib_get(MIB_SNMPD_ENABLE, (void *)&vChar);
	if(vChar==1)
		status = startSnmp();
#endif
#endif
	//}
#endif    
	return status;
}
	
//--------------------------------------------------------
// Daemon startup
// return value:
// 1  : successful
// -1 : failed
int startDaemon(BOOT_TYPE_T mode)
{
	int pppd_fifo_fd=-1;
	int mpoad_fifo_fd=-1;
	int status=0, tmp_status;
	int k;

	
	// start mpoad
	status|=va_cmd(MPOAD, 0, 0);
	
	// check if mpoad ready to serve
	//while ((mpoad_fifo_fd = open(MPOAD_FIFO, O_WRONLY)) == -1)
	for (k=0; k<=100; k++) 
	{
		if ((mpoad_fifo_fd = open(MPOAD_FIFO, O_WRONLY))!=-1)
			break;
		usleep(30000);
	}		

	if (mpoad_fifo_fd == -1)
	{
		printf("open mpoad fifo failed !\n");
		status = -1;
	}
	else
		close(mpoad_fifo_fd);
#ifndef ZTE_531B_BRIDGE_SC	
	if (startDnsRelay(mode) == -1)
	{
		printf("start DNS relay failed !\n");
		status=-1;
	}
#endif	
	tmp_status=setupDhcpd();
	if (tmp_status == 1)
	{
		status|=va_cmd(DHCPD, 1, 0, DHCPD_CONF);
	} else if (tmp_status==-1)
	    status = -1;
	
	
	// start spppd
	status|=va_cmd(SPPPD, 0, 0);
	
	// check if spppd ready to serve
	//while ((pppd_fifo_fd = open(PPPD_FIFO, O_WRONLY)) == -1)
	for (k=0; k<=100; k++)
	{
		if ((pppd_fifo_fd = open(PPPD_FIFO, O_WRONLY))!=-1)
			break;
		usleep(30000);
	}
	
	if (pppd_fifo_fd == -1)
		status = -1;
	else
		close(pppd_fifo_fd);
	
#if defined(TIME_ZONE) || defined(ZTE_GENERAL_ROUTER_SC)
	if (mode == BOOT_LAST)
		status|=startNTP();
#endif
		
	status|=setupService(mode);

	return status;
}

#ifdef  CONFIG_USER_PPPOE_PROXY

#define   LAN_PPP_INTERFACE 4
#define   WAN_PPP_INTERFACE 8 
#endif

#if 0	// Zebra
static const char ZEBRA_CONF[] = "/var/zebra.conf";
static const char RIPD_CONF[] = "/var/ripd.conf";

// RIP server configuration
// return value:
// 0  : not enabled
// 1  : successful
// -1 : startup failed
int startRip()
{
	FILE *fp;
	unsigned char ripInf, ripVer;
	char *argv[6];
	
	if ((fp = fopen(ZEBRA_CONF, "w")) == NULL)
	{
		printf("Open file %s failed !\n", ZEBRA_CONF);
		return -1;
	}
	
	fclose(fp);
	
	if ((fp = fopen(RIPD_CONF, "w")) == NULL)
	{
		printf("Open file %s failed !\n", RIPD_CONF);
		return -1;
	}
	
	if (mib_get(MIB_RIP_INTERFACE, (void *)&ripInf) == 0)
		ripInf = 2;	// default both
	
	if (mib_get(MIB_RIP_VERSION, (void *)&ripVer) == 0)
		ripVer = 1;	// default version 2
	
	fprintf(fp, "router rip\n");
	fprintf(fp, "redistribute kernel\n");
	
	if (ripInf == 0 || ripInf == 2)	// LAN or both
	{
		fprintf(fp, "network %s\n", LANIF);
		fprintf(fp, "interface %s\n", LANIF);
		if (ripVer == 2)
			fprintf(fp, "ip rip send version 1 2\n");
		else
			fprintf(fp, "ip rip send version %u\n", ripVer+1);
		fprintf(fp, "ip rip receive version 1 2\n");
		fprintf(fp, "router rip\n");
	}
	
	if (ripInf == 1 || ripInf == 2)	// WAN or both
	{
		fprintf(fp, "network vc0\n");
		fprintf(fp, "interface vc0\n");
		if (ripVer == 2)
			fprintf(fp, "ip rip send version 1 2\n");
		else
			fprintf(fp, "ip rip send version %u\n", ripVer+1);
		fprintf(fp, "ip rip receive version 1 2\n");
	}
	
	fclose(fp);
	
	argv[1] = "-d";
	argv[2] = "-f";
	argv[3] = (char *)ZEBRA_CONF;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", ZEBRA, argv[1], argv[2], argv[3]);
	do_cmd(ZEBRA, argv, 0);
	
	argv[1] = "-d";
	argv[2] = "-f";
	argv[3] = (char *)RIPD_CONF;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", RIPD, argv[1], argv[2], argv[3]);
	do_cmd(RIPD, argv, 0);
	return 1;
}
#else	// routed

#endif

#if 0
#ifdef WLAN_SUPPORT
// Wlan configuration
#ifdef WLAN_1x
static void WRITE_WPA_FILE(int fh, unsigned char *buf)
{
	if ( write(fh, buf, strlen(buf)) != strlen(buf) ) {
		printf("Write WPA config file error!\n");
		close(fh);
		//exit(1);
	}
}

// return value:
// 0  : success
// -1 : failed
#ifdef WLAN_MBSSID
static int generateWpaConf(char *outputFile, int isWds, MIB_CE_MBSSIB_T *Entry)
#else
static int generateWpaConf(char *outputFile, int isWds)
#endif
{
	int fh, intVal;
	unsigned char chVal, wep, encrypt, enable1x;
	unsigned char buf1[1024], buf2[1024];
	unsigned short sintVal;

	fh = open(outputFile, O_RDWR|O_CREAT|O_TRUNC);
	if (fh == -1) {
		printf("Create WPA config file error!\n");
		return -1;
	}
	if (!isWds) {
#ifndef WLAN_MBSSID		
	mib_get( MIB_WLAN_ENCRYPT, (void *)&encrypt);
#else
	encrypt = Entry->encrypt;
#endif
	sprintf(buf2, "encryption = %d\n", encrypt);
	WRITE_WPA_FILE(fh, buf2);	

#ifndef WLAN_MBSSID
	mib_get( MIB_WLAN_SSID,  (void *)buf1);
#else
	if ( Entry->idx == 0 ) {
		mib_get( MIB_WLAN_SSID,  (void *)buf1);
	} else {
		strcpy(buf1, Entry->ssid);
	}
#endif
	sprintf(buf2, "ssid = \"%s\"\n", buf1);
	WRITE_WPA_FILE(fh, buf2);

#ifndef WLAN_MBSSID
	mib_get( MIB_WLAN_ENABLE_1X, (void *)&enable1x);
#else
	enable1x = Entry->enable1X;
#endif
       	sprintf(buf2, "enable1x = %d\n", enable1x);
	WRITE_WPA_FILE(fh, buf2);

	//mib_get( MIB_WLAN_ENABLE_MAC_AUTH, (void *)&intVal);
	sprintf(buf2, "enableMacAuth = %d\n", 0);
	WRITE_WPA_FILE(fh, buf2);

/*
	mib_get( MIB_WLAN_ENABLE_SUPP_NONWPA, (void *)&intVal);
	if (intVal)
		mib_get( MIB_WLAN_SUPP_NONWPA, (void *)&intVal);
*/

	sprintf(buf2, "supportNonWpaClient = %d\n", 0);
	WRITE_WPA_FILE(fh, buf2);

#ifndef WLAN_MBSSID
	mib_get( MIB_WLAN_WEP, (void *)&wep);
#else
	wep = Entry->wep;
#endif
	sprintf(buf2, "wepKey = %d\n", wep);
	WRITE_WPA_FILE(fh, buf2);

/*
	if ( encrypt==1 && enable1x ) {
		if (wep == 1) {
			mib_get( MIB_WLAN_WEP64_KEY1, (void *)buf1);
			sprintf(buf2, "wepGroupKey = \"%02x%02x%02x%02x%02x\"\n", buf1[0],buf1[1],buf1[2],buf1[3],buf1[4]);
		}
		else {
			mib_get( MIB_WLAN_WEP128_KEY1, (void *)buf1);
			sprintf(buf2, "wepGroupKey = \"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\"\n",
				buf1[0],buf1[1],buf1[2],buf1[3],buf1[4],
				buf1[5],buf1[6],buf1[7],buf1[8],buf1[9],
				buf1[10],buf1[11],buf1[12]);
		}
	}
	else
*/
	strcpy(buf2, "wepGroupKey = \"\"\n");
	WRITE_WPA_FILE(fh, buf2);

#ifndef WLAN_MBSSID
	mib_get( MIB_WLAN_WPA_AUTH, (void *)&chVal);
#else
	chVal = Entry->wpaAuth;
#endif
	// Kaohj, PSK only
	sprintf(buf2, "authentication = %d\n", chVal); //1: radius 2:PSK
	WRITE_WPA_FILE(fh, buf2);

	mib_get( MIB_WLAN_WPA_CIPHER_SUITE, (void *)&chVal);
	sprintf(buf2, "unicastCipher = %d\n", chVal);
	WRITE_WPA_FILE(fh, buf2);

	mib_get( MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&chVal);
	sprintf(buf2, "wpa2UnicastCipher = %d\n", chVal);
	WRITE_WPA_FILE(fh, buf2);

/*
	mib_get( MIB_WLAN_WPA2_PRE_AUTH, (void *)&intVal);
	sprintf(buf2, "enablePreAuth = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);
*/

#ifndef WLAN_MBSSID
	mib_get( MIB_WLAN_WPA_PSK_FORMAT, (void *)&chVal);
#else
	chVal = Entry->wpaPSKFormat;
#endif
	if (chVal==0)
		sprintf(buf2, "usePassphrase = 1\n");
	else
		sprintf(buf2, "usePassphrase = 0\n");
	WRITE_WPA_FILE(fh, buf2);

#ifndef WLAN_MBSSID
	mib_get( MIB_WLAN_WPA_PSK, (void *)buf1);
#else
	strcpy(buf1, Entry->wpaPSK);
#endif
	sprintf(buf2, "psk = \"%s\"\n", buf1);
	WRITE_WPA_FILE(fh, buf2);

	mib_get( MIB_WLAN_WPA_GROUP_REKEY_TIME, (void *)&intVal);
	sprintf(buf2, "groupRekeyTime = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);

#if 1 // not support RADIUS
#ifndef WLAN_MBSSID
	mib_get( MIB_WLAN_RS_PORT, (void *)&sintVal);
#else
	sintVal = Entry->rsPort;
#endif
	sprintf(buf2, "rsPort = %d\n", sintVal);
	WRITE_WPA_FILE(fh, buf2);

#ifndef WLAN_MBSSID
	mib_get( MIB_WLAN_RS_IP, (void *)buf1);
#else
	*((unsigned long *)buf1) = *((unsigned long *)Entry->rsIpAddr);
	//printf("*buf1=0x%x\n", *((unsigned long *)buf1));
#endif
	sprintf(buf2, "rsIP = %s\n", inet_ntoa(*((struct in_addr *)buf1)));
	WRITE_WPA_FILE(fh, buf2);

#ifndef WLAN_MBSSID
	mib_get( MIB_WLAN_RS_PASSWORD, (void *)buf1);
#else
	strcpy(buf1, Entry->rsPassword);
#endif
	sprintf(buf2, "rsPassword = \"%s\"\n", buf1);
	WRITE_WPA_FILE(fh, buf2);

	mib_get( MIB_WLAN_RS_RETRY, (void *)&chVal);
	sprintf(buf2, "rsMaxReq = %d\n", chVal);
	WRITE_WPA_FILE(fh, buf2);

	mib_get( MIB_WLAN_RS_INTERVAL_TIME, (void *)&sintVal);
	sprintf(buf2, "rsAWhile = %d\n", sintVal);
	WRITE_WPA_FILE(fh, buf2);

	mib_get( MIB_WLAN_ACCOUNT_RS_ENABLED, (void *)&chVal);
	sprintf(buf2, "accountRsEnabled = %d\n", chVal);
	WRITE_WPA_FILE(fh, buf2);

	mib_get( MIB_WLAN_ACCOUNT_RS_PORT, (void *)&sintVal);
	sprintf(buf2, "accountRsPort = %d\n", sintVal);
	WRITE_WPA_FILE(fh, buf2);

	mib_get( MIB_WLAN_ACCOUNT_RS_IP, (void *)buf1);
	sprintf(buf2, "accountRsIP = %s\n", inet_ntoa(*((struct in_addr *)buf1)));
	WRITE_WPA_FILE(fh, buf2);

	mib_get( MIB_WLAN_ACCOUNT_RS_PASSWORD, (void *)buf1);
	sprintf(buf2, "accountRsPassword = \"%s\"\n", buf1);
	WRITE_WPA_FILE(fh, buf2);

	mib_get( MIB_WLAN_ACCOUNT_UPDATE_ENABLED, (void *)&chVal);
	sprintf(buf2, "accountRsUpdateEnabled = %d\n", chVal);
	WRITE_WPA_FILE(fh, buf2);

	mib_get( MIB_WLAN_ACCOUNT_UPDATE_DELAY, (void *)&sintVal);
	sprintf(buf2, "accountRsUpdateTime = %d\n", sintVal);
	WRITE_WPA_FILE(fh, buf2);

	mib_get( MIB_WLAN_ACCOUNT_RS_RETRY, (void *)&chVal);
	sprintf(buf2, "accountRsMaxReq = %d\n", chVal);
	WRITE_WPA_FILE(fh, buf2);

	mib_get( MIB_WLAN_ACCOUNT_RS_INTERVAL_TIME, (void *)&sintVal);
	sprintf(buf2, "accountRsAWhile = %d\n", sintVal);
	WRITE_WPA_FILE(fh, buf2);
#endif
	}

	else {
#if 0 // not support WDS
		apmib_get( MIB_WLAN_WDS_ENCRYPT, (void *)&encrypt);
		if (encrypt == WDS_ENCRYPT_TKIP)		
			encrypt = ENCRYPT_WPA;
		else if (encrypt == WDS_ENCRYPT_AES)		
			encrypt = ENCRYPT_WPA2;		
		else
			encrypt = 0;
	
		sprintf(buf2, "encryption = %d\n", encrypt);
		WRITE_WPA_FILE(fh, buf2);
		WRITE_WPA_FILE(fh, "ssid = \"REALTEK\"\n");
		WRITE_WPA_FILE(fh, "enable1x = 1\n");
		WRITE_WPA_FILE(fh, "enableMacAuth = 0\n");
		WRITE_WPA_FILE(fh, "supportNonWpaClient = 0\n");
		WRITE_WPA_FILE(fh, "wepKey = 0\n");
		WRITE_WPA_FILE(fh,  "wepGroupKey = \"\"\n");
		WRITE_WPA_FILE(fh,  "authentication = 2\n");

		if (encrypt == ENCRYPT_WPA)
			intVal = WPA_CIPHER_TKIP;
		else
			intVal = WPA_CIPHER_AES;
			
		sprintf(buf2, "unicastCipher = %d\n", intVal);
		WRITE_WPA_FILE(fh, buf2);

		sprintf(buf2, "wpa2UnicastCipher = %d\n", intVal);
		WRITE_WPA_FILE(fh, buf2);

		WRITE_WPA_FILE(fh, "enablePreAuth = 0\n");

		apmib_get( MIB_WLAN_WDS_PSK_FORMAT, (void *)&intVal);
		if (intVal==0)
			sprintf(buf2, "usePassphrase = 1\n");
		else
			sprintf(buf2, "usePassphrase = 0\n");
		WRITE_WPA_FILE(fh, buf2);

		apmib_get( MIB_WLAN_WDS_PSK, (void *)buf1);
		sprintf(buf2, "psk = \"%s\"\n", buf1);
		WRITE_WPA_FILE(fh, buf2);

		WRITE_WPA_FILE(fh, "groupRekeyTime = 0\n");
		WRITE_WPA_FILE(fh, "rsPort = 1812\n");
		WRITE_WPA_FILE(fh, "rsIP = 192.168.1.1\n");
		WRITE_WPA_FILE(fh, "rsPassword = \"\"\n");
		WRITE_WPA_FILE(fh, "rsMaxReq = 3\n");
		WRITE_WPA_FILE(fh, "rsAWhile = 10\n");
		WRITE_WPA_FILE(fh, "accountRsEnabled = 0\n");
		WRITE_WPA_FILE(fh, "accountRsPort = 1813\n");
		WRITE_WPA_FILE(fh, "accountRsIP = 192.168.1.1\n");
		WRITE_WPA_FILE(fh, "accountRsPassword = \"\"\n");
		WRITE_WPA_FILE(fh, "accountRsUpdateEnabled = 0\n");
		WRITE_WPA_FILE(fh, "accountRsUpdateTime = 1000\n");
		WRITE_WPA_FILE(fh, "accountRsMaxReq = 3\n");
		WRITE_WPA_FILE(fh, "accountRsAWhile = 1\n");
#endif
	}

	close(fh);
	
	return 0;
}


// return value:
// 0  : success
// -1 : failed
#ifdef WLAN_MBSSID
	int startWLanDaemon(MIB_CE_MBSSIB_T *Entry)
#else
int startWLanDaemon()
#endif
{
	unsigned char encrypt, no_wlan;
	int auth_pid_fd=-1;
	unsigned char enable1x=0;
	int status=0;

	int useAuth = 0;
#ifdef CONFIG_WIFI_SIMPLE_CONFIG // WPS
	int enableWsc = 1;
	unsigned char wsc_disable;
	unsigned char wsc_upnp_enabled;
	unsigned char wlan_mode;
	unsigned char wlan_nettype;
	unsigned char wpa_auth;
	char *cmd_opt[16];
	int	cmd_cnt = 0; int idx;
	int wscd_pid_fd = -1;
	
	mib_get(MIB_WSC_DISABLE, (void *)&wsc_disable);
	mib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
	mib_get(MIB_WLAN_NETWORK_TYPE, (void *)&wlan_nettype);
	mib_get(MIB_WLAN_WPA_AUTH, (void *)&wpa_auth);
	mib_get(MIB_WSC_UPNP_ENABLED, (void *)&wsc_upnp_enabled);
#endif

#ifdef  WLAN_MBSSID
	char para_auth_conf[30];
	char para_itf_name[15];
	char para_auth_pid[30];	
#endif


#ifdef CONFIG_WIFI_SIMPLE_CONFIG // WPS
	fprintf(stderr, "START WPS SETUP!\n\n\n");
	cmd_opt[cmd_cnt++] = "";

	if (wsc_disable || (wlan_mode == AP_WDS_MODE))
		enableWsc = 0;
	else if (wlan_mode == CLIENT_MODE) {
		if (wlan_nettype != INFRASTRUCTURE)
			enableWsc = 0;
	} else if (wlan_mode == AP_MODE) {
		if ((encrypt < ENCRYPT_WPA) && (enable1x != 0))
			enableWsc = 0;
		if ((encrypt >= ENCRYPT_WPA) && (wpa_auth == WPA_AUTH_AUTO))
			enableWsc = 0;		
	}

	if (enableWsc) useAuth = 1;

	if (wlan_mode == CLIENT_MODE) {
		cmd_opt[cmd_cnt++] = "-mode";
		cmd_opt[cmd_cnt++] = "2";
	} else {
		cmd_opt[cmd_cnt++] = "-start";
	}
		
	cmd_opt[cmd_cnt++] = "-c";
	cmd_opt[cmd_cnt++] = "/var/wscd.conf";
	cmd_opt[cmd_cnt++] = "-w";
	cmd_opt[cmd_cnt++] = "wlan0";
	//strcat(cmd, " -c /var/wscd.conf -w wlan0");
		
	if (1) { // use iwcontrol
		cmd_opt[cmd_cnt++] = "-fi";
		cmd_opt[cmd_cnt++] = "/var/wscd-wlan0.fifo";
		//strcat(cmd, " -fi /var/wscd-wlan0.fifo");
	}

	cmd_opt[cmd_cnt++] = "-debug";
	//strcat(cmd, " -debug");
	//strcat(cmd, " &");
	#define TARGDIR "/var/wps/"
	#define SIMPLECFG "simplecfgservice.xml"
	status |= va_cmd("/bin/flash", 3, 1, "upd-wsc-conf", "/etc/wscd.conf", "/var/wscd.conf");
	status |= va_cmd("/bin/mkdir", 2, 1, "-p", TARGDIR);
	status |= va_cmd("/bin/cp", 2, 1, "/etc/" SIMPLECFG, TARGDIR);

	cmd_opt[cmd_cnt] = 0;
	printf("CMD ARGS: ");
	for (idx=0; idx<cmd_cnt;idx++) 
		printf("%s ", cmd_opt[idx]);
	printf("\n");
	
	status |= do_cmd("/bin/wscd", cmd_opt, 0);

	while ((wscd_pid_fd = open("/var/run/wscd-wlan0.pid", O_WRONLY)) == -1)
	{
		usleep(30000);
	}

#endif


#ifndef WLAN_MBSSID	
	mib_get(MIB_WLAN_ENCRYPT, (void *)&encrypt);
	mib_get(MIB_WLAN_DISABLED, (void *)&no_wlan);
#ifdef WLAN_1x
	mib_get( MIB_WLAN_ENABLE_1X, (void *)&enable1x);
#endif

	if ((encrypt >= 2 || enable1x==1) && no_wlan == 0) {// 802.1x enabled
		status|=generateWpaConf((char *)WLAN_AUTH_CONF, 0);
		status|=va_cmd(AUTH, 4, 0, (char *)WLANIF, (char *)LANIF, "auth", (char *)WLAN_AUTH_CONF);
		// fix the depency problem
		// wait for auth-xxx.pid to be created, the iwcontrol will check for it
		while ((auth_pid_fd = open(AUTH_PID, O_WRONLY)) == -1)
		{
			usleep(30000);
		}
		useAuth = 1;		
	}
	
	if (useAuth) {
		status|=va_cmd(IWCONTROL, 1, 1, (char *)WLANIF);
	}
#else	
	
	//mib_get(MIB_WLAN_ENCRYPT, (void *)&encrypt);
	encrypt = Entry->encrypt;
	//mib_get(MIB_WLAN_DISABLED, (void *)&no_wlan);
#ifdef WLAN_1x
	//mib_get( MIB_WLAN_ENABLE_1X, (void *)&enable1x);
	enable1x = Entry->enable1X;
#endif
	if ( Entry->idx !=0 ) {
		sprintf(para_auth_conf, "/var/config/wlan0-vap%d.conf", Entry->idx - 1); 
		sprintf(para_itf_name, "wlan0-vap%d", Entry->idx - 1);
		sprintf(para_auth_pid, "/var/run/auth-wlan0-vap%d.pid", Entry->idx - 1);
	} else {
		strcpy(para_auth_conf, "/var/config/wlan0.conf");
		strcpy(para_itf_name, "wlan0");
		strcpy(para_auth_pid, "/var/run/auth-wlan0.pid");
	}

	//if ((encrypt >= 2 || enable1x==1) && no_wlan == 0) {// 802.1x enabled
	if ((encrypt >= 2 || enable1x==1) ) {// 802.1x enabled	
		//status|=generateWpaConf((char *)WLAN_AUTH_CONF, 0);
		status|=generateWpaConf(para_auth_conf, 0, Entry);
		
		//status|=va_cmd(AUTH, 4, 0, (char *)WLANIF, (char *)LANIF, "auth", (char *)WLAN_AUTH_CONF);
		status|=va_cmd(AUTH, 4, 0, para_itf_name, (char *)LANIF, "auth", para_auth_conf);
		
		// fix the depency problem
		// wait for auth-xxx.pid to be created, the iwcontrol will check for it
		//while ((auth_pid_fd = open(AUTH_PID, O_WRONLY)) == -1)		
		while ((auth_pid_fd = open(para_auth_pid, O_WRONLY)) == -1)
		{
			usleep(30000);
		}
		
		//status|=va_cmd(IWCONTROL, 1, 1, (char *)WLANIF);
		//if ( Entry->idx == 4 ) {
		//	status|=va_cmd(IWCONTROL, 1, 1, "wlan0 wlan0-vap0 wlan0-vap1 wlan0-vap2 wlan0-vap3");
		//}		
		
		strcpy(para_iwctrl[wlan_num], para_itf_name);
		wlan_num++;	
		
	}	
	
#endif	

#ifdef WLAN_WEB_REDIRECT  //jiunming,web_redirect
{
	char tmpbuf[MAX_URL_LEN];
	char ipaddr[16], ip_port[32], redir_server[33];
	
	ipaddr[0]='\0'; ip_port[0]='\0';redir_server[0]='\0';
	if (mib_get(MIB_ADSL_LAN_IP, (void *)tmpbuf) != 0)
	{
		strncpy(ipaddr, inet_ntoa(*((struct in_addr *)tmpbuf)), 16);
		ipaddr[15] = '\0';
		sprintf(ip_port,"%s:%d",ipaddr,8080);
	}//else ??

	if( mib_get(MIB_WLAN_WEB_REDIR_URL, (void*)tmpbuf) )
	{
		char *p=NULL, *end=NULL;
		
		p = strstr( tmpbuf, "http://" );
		if(p)
			p = p + 7;
		else 
			p = tmpbuf;
			
		end = strstr( p, "/" );
		if(end)
			*end = '\0';
		
		snprintf( redir_server,32,"%s",p );
		redir_server[32]='\0';
	}//else ??
		
	// Enable Bridge Netfiltering					
	//status|=va_cmd("/bin/brctl", 2, 0, "brnf", "on");
	
	//iptables -t nat -N Web_Redirect
	status|=va_cmd(IPTABLES, 4, 1, "-t", "nat","-N","Web_Redirect");
	
	//iptables -t nat -A Web_Redirect -d 192.168.1.1 -j RETURN
	status|=va_cmd(IPTABLES, 8, 1, "-t", "nat","-A","Web_Redirect",
		"-d", ipaddr, "-j", (char *)FW_RETURN);
	
	//iptables -t nat -A Web_Redirect -d 192.168.2.11 -j RETURN
	status|=va_cmd(IPTABLES, 8, 1, "-t", "nat","-A","Web_Redirect",
		"-d", redir_server, "-j", (char *)FW_RETURN);	
	
	//iptables -t nat -A Web_Redirect -p tcp --dport 80 -j DNAT --to-destination 192.168.1.1:8080 
	status|=va_cmd(IPTABLES, 12, 1, "-t", "nat","-A","Web_Redirect",
		"-p", "tcp", "--dport", "80", "-j", "DNAT", 
		"--to-destination", ip_port);
	
	//iptables -t nat -A PREROUTING -p tcp --dport 80 -j Web_Redirect
	status|=va_cmd(IPTABLES, 12, 1, "-t", "nat","-A","PREROUTING",
		"-i", (char *)WLANIF,
		"-p", "tcp", "--dport", "80", "-j", "Web_Redirect");
}
#endif
	return status;
}
#endif

//--------------------------------------------------------
// Wireless LAN startup
// return value:
// 0  : not start by configuration
// 1  : successful
// -1 : failed
int startWLan()
{
	unsigned char no_wlan;
	int status=0;
#ifdef WLAN_MBSSID
	int i=0;
	char para2[15];
	MIB_CE_MBSSIB_T Entry;
#endif
	
	//1/17/06' hrchen, always start WLAN MIB, for MP test will use 
	// "ifconfig wlan0 up" to start WLAN
	// config WLAN
	status|=setupWLan();

	// Modified by Mason Yu
#ifndef WLAN_MBSSID
	mib_get(MIB_WLAN_DISABLED, (void *)&no_wlan);
	if (no_wlan == 0)	// WLAN enabled
	{
		// brctl addif br0 wlan0
		status|=va_cmd(BRCTL, 3, 1, "addif", (char *)BRIF, (char *)WLANIF);
		// ifconfig wlan0 up
		status|=va_cmd(IFCONFIG, 2, 1, (char *)WLANIF, "up");
#ifdef WLAN_1x
		status|=startWLanDaemon();
#endif
        status = (status==-1)?-1:1;
	}
	else
		return 0;
		
#else
for ( i=0; i<=4; i++) {
	if (!mib_chain_get(MIB_MBSSIB_TBL, i, (void *)&Entry)) {  		
  		printf("Error! Get MIB_MBSSIB_TBL(startWLan) error.\n");					
	}
		
	if ( i!=0 ) {		
		no_wlan = Entry.wlanDisabled;		
		sprintf(para2, "wlan0-vap%d", i-1);
		
	} else {	
		mib_get(MIB_WLAN_DISABLED, (void *)&no_wlan);
		strcpy(para2, "wlan0");		
	}
	
	
	if (no_wlan == 0)	// WLAN enabled
	{
		// brctl addif br0 wlan0
		//status|=va_cmd(BRCTL, 3, 1, "addif", (char *)BRIF, (char *)WLANIF);
		status|=va_cmd(BRCTL, 3, 1, "addif", (char *)BRIF, para2);
		
		// ifconfig wlan0 up
		//status|=va_cmd(IFCONFIG, 2, 1, (char *)WLANIF, "up");	
		sleep(2);	
		status|=va_cmd(IFCONFIG, 2, 1, para2, "up");
#ifdef WLAN_1x
		status|=startWLanDaemon(&Entry);
#endif
        status = (status==-1)?-1:1;
	}
	else
		//return 0;
		status = 0;		
}
	
	// Mason Yu
	// We have 5 AP(Root and vap0 ~ vap3., if we add a new vap, we must modified the following codes.
	if ( wlan_num == 1 ) {
		status|=va_cmd(IWCONTROL, wlan_num, 1, para_iwctrl[0]);
	} else if ( wlan_num == 2 ) {
		status|=va_cmd(IWCONTROL, wlan_num, 1, para_iwctrl[0], para_iwctrl[1]);
	} else if ( wlan_num == 3 ) {
		status|=va_cmd(IWCONTROL, wlan_num, 1, para_iwctrl[0], para_iwctrl[1], para_iwctrl[2]);
	} else if ( wlan_num == 4 ) {
		status|=va_cmd(IWCONTROL, wlan_num, 1, para_iwctrl[0], para_iwctrl[1], para_iwctrl[2], para_iwctrl[3]);
	} else if ( wlan_num == 5 ) {
		status|=va_cmd(IWCONTROL, wlan_num, 1, para_iwctrl[0], para_iwctrl[1], para_iwctrl[2], para_iwctrl[3], para_iwctrl[4]);
	} 
			
#endif
	
	return status;
}
#endif
#endif

#ifdef CONFIG_EXT_SWITCH

#if 0
void add2br()
{
	int i, k;
	MIB_CE_SW_PORT_Tp pPort;
	char str_br[]="br0";
	char str_port[]="eth0_sw0";
	
	// bring up eth0_swx
	// brctl addif brx eth0_swx
	for (i=0; i<SW_PORT_NUM; i++) {
		pPort = (MIB_CE_SW_PORT_Tp) mib_chain_get(MIB_SW_PORT_TBL,i);
		str_port[7] = '0' + i;
		// ifconfig eth0_swx up
		va_cmd(IFCONFIG, 2, 1, str_port, "up");
		
		// find its br
		for (k=0; k<SW_PORT_NUM; k++) {
			if (brInfo[k].pvcIdx == pPort->pvcItf) {
				str_br[2] = '0'+brInfo[k].brid;
				// brctl addif brx eth0_swx
				va_cmd(BRCTL, 3, 1, "addif", str_br, str_port);
				break;
			}
		}
	}
}
#endif

#if 0 /*jiunming, move to utility.c*/
// enable rtl-8305 VLAN
static void setup_8305_vlan_enable()
{
	struct ifreq ifr;
	struct ifvlan ifvl;
	
	strcpy(ifr.ifr_name, "eth0");
	ifr.ifr_data = (char *)&ifvl;
	// enable rtl-8305 VLAN
	ifvl.cmd=VLAN_ENABLE;
	ifvl.enable=1;
	setVlan(&ifr);
	TRACE(STA_SCRIPT, "RTL8305 VLAN enabled\n");
}

/*------------------------------------------------------------------------
 *	Setup RTL8305 Vlan membership.
 *	Enable port-based Vlan and set the vlan membership with
 *	the following pre-requisite:
 *
 *	port		vlanIdx		Vlan	VID
 *	0		0		A	0
 *	1		1		B	1
 *	2		2		C	2
 *	3		3		D	3
 *
 *	The mbr is bit-map of the ports (interfaces)
 *	OLD Bit-map:  bit5  |  bit4  |  bit3  |  bit2  |  bit1  |  bit0
 *	              wlan  | device |  lan3  |  lan2  |  lan1  |  lan0
 
 * New Bit-map:  bit9 | bit8 | bit7 | bit6 | bit5  |  bit4  |  bit3  |  bit2  |  bit1  |  bit0
 *               vap3 | vap2 | vap1 | vap0 | wlan  | device |  lan3  |  lan2  |  lan1  |  lan0
 */
 /*------------------------------------------------------------------------*/
static void setup_8305_vlan_member(int port, int mbr)
{
	int k;
	struct ifreq ifr;
	struct ifvlan ifvl;
	
	strcpy(ifr.ifr_name, "eth0");
	ifr.ifr_data = (char *)&ifvl;
	
	// set 8305 port-based vlan members
	// port		vlanIdx		Vlan	VID
	// 0		0		A	0
	// 1		1		B	1
	// 2		2		C	2
	// 3		3		D	3
	ifvl.cmd=VLAN_SETINFO;
	
	// This port is a member ?
	if (mbr & (1<<port)) {
		// set the membership
		ifvl.vlanIdx=port;
		// VID 0 is used to identify priority frames and VID 4095(0xFFF) is reserved
		ifvl.vid=port+1;
// Mason Yu
#ifndef WLAN_MBSSID
		ifvl.member=(mbr & 0x3f);
#else
		ifvl.member=(mbr & 0x3ff);
#endif
		setVlan(&ifr);
		TRACE(STA_SCRIPT, "RTL8305 VLAN: vlanIdx=%d, vid=%d, member=0x%.2x\n", ifvl.vlanIdx, ifvl.vid, ifvl.member);
	}
}

#ifdef WLAN_SUPPORT
/*------------------------------------------------------------------------
 *	Setup wlan0 group membership.
 *	The mbr is bit-map of the ports (interfaces)
 *	Bit-map:  bit5  |  bit4  |  bit3  |  bit2  |  bit1  |  bit0
 *	          wlan  | device |  lan3  |  lan2  |  lan1  |  lan0
 * New Bit-map:  bit9 | bit8 | bit7 | bit6 | bit5  |  bit4  |  bit3  |  bit2  |  bit1  |  bit0
 *               vap3 | vap2 | vap1 | vap0 | wlan  | device |  lan3  |  lan2  |  lan1  |  lan0
 *------------------------------------------------------------------------*/
#define SIOSIWRTLITFGROUP		0x8B90

static void setup_wlan_group_member(int itf, int mbr)
{
	int skfd;
	struct iwreq wrq;
	int k;
	
	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	
#ifndef WLAN_MBSSID	
	/* Set device name */
	strncpy(wrq.ifr_name, (char *)WLANIF, IFNAMSIZ);
	wrq.u.data.flags = 0;
#else
	strncpy(wrq.ifr_name, (char *)WLANAPIF[itf], IFNAMSIZ);		
	wrq.u.data.flags = itf;

#endif	
	wrq.u.data.pointer = (caddr_t)&mbr;
	wrq.u.data.length = sizeof(mbr);	
	
	ioctl(skfd, SIOSIWRTLITFGROUP, &wrq);
	close( skfd );
}
#endif

#ifdef ITF_GROUP
/*
 * Setup per-vc interface group member(bit mapped)
 * The bitmap for interface should follow this convention commonly shared
 * by sar driver (sar_send())
 * OLD Bit-map:  bit5  |  bit4  |  bit3  |  bit2  |  bit1  |  bit0
 *               wlan  | device |  lan3  |  lan2  |  lan1  |  lan0
 
 * New Bit-map:  bit9 | bit8 | bit7 | bit6 | bit5  |  bit4  |  bit3  |  bit2  |  bit1  |  bit0
 *               vap3 | vap2 | vap1 | vap0 | wlan  | device |  lan3  |  lan2  |  lan1  |  lan0
 */
void setupEth2pvc()
{
	//MIB_CE_SW_PORT_Tp pPort;
	MIB_CE_SW_PORT_T Port;
	//MIB_CE_ATM_VC_Tp pvcEntry;
	MIB_CE_ATM_VC_T pvcEntry;
	int i, k, total, num;
	char str_br[]="br1";
	struct data_to_pass_st msg;
	char str_vc[5], wanif[5];
#ifdef WLAN_SUPPORT
	char wlangrp;
#ifdef WLAN_MBSSID	
	char wlanAPgrp[5];
#endif
#endif
	int mbr[MAX_VC_NUM], dft_mbr, pvc_dft_mbr=0;
	
	// register 8305 switch port to system
	//__dev_register_swport();
	
	// init vc group member to zero, device is mandatory
	for (i=0; i<MAX_VC_NUM; i++)
		mbr[i] = 0;
	// init default group member
	dft_mbr=0;
	
	// setup user-specified pvc interface group members
	total = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<total; i++) {
		int vcIdx;
		
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&pvcEntry))
			continue;
		if (!pvcEntry.enable)
			continue;
		// check for LAN ports
		num = mib_chain_total(MIB_SW_PORT_TBL);
		vcIdx = VC_INDEX(pvcEntry.ifIndex);
		for (k=0; k<num; k++) {
			if (!mib_chain_get(MIB_SW_PORT_TBL, k, (void *)&Port))
				continue;
			if (Port.itfGroup == pvcEntry.itfGroup) {
				// this lan port shares the same group with pvc, add as pvc's member
				// TODO: check for pppoa (no VC_INDEX)
				if (vcIdx < MAX_VC_NUM) {
					mbr[vcIdx] |= (1<<k);
					#if 0
					// get rid of local dhcp server if mapping to bridge
					if ((REMOTE_ACCESS_T)pvcEntry.cmode == ACC_BRIDGED) {
						char phy[]="eth0_sw0";
						phy[7]='0'+k;
						// don't touch my dhcp server if mapping to bridge port
						// iptables -A inacc -i $LAN_IF -p UDP --dport 67 -j DROP
						va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_INACC,
						(char *)ARG_I, phy, "-p", (char *)ARG_UDP,
						(char *)FW_DPORT, (char *)PORT_DHCP, "-j", (char *)FW_DROP);
					}
					#else
					// get rid of local dhcp server if not belonging to default group
					// Commented by Mason Yu
					#if 0
					if (Port.itfGroup != 0) {
						char phy[]="eth0_sw0";
						phy[7]='0'+k;
						// iptables -A inacc -i $LAN_IF -p UDP --dport 67 -j DROP
						va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_INACC,
						(char *)ARG_I, phy, "-p", (char *)ARG_UDP,
						(char *)FW_DPORT, (char *)PORT_DHCP, "-j", (char *)FW_DROP);
					}
					#endif
					#endif
				}
			}
		}
#ifdef WLAN_SUPPORT
		// check for wlan0
		{
			mib_get(MIB_WLAN_ITF_GROUP, (void *)&wlangrp);
			if (wlangrp == pvcEntry.itfGroup) {
				// this lan port shares the same group with pvc, add as pvc's member
				if (vcIdx < MAX_VC_NUM) {
					mbr[vcIdx] |= (1<<IFWLAN_SHIFT);
					// get rid of local dhcp server if not belonging to default group
					// Commented by Mason Yu
					#if 0
					if (wlangrp != 0) {
						// iptables -A inacc -i $LAN_IF -p UDP --dport 67 -j DROP
						va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_INACC,
						(char *)ARG_I, (char *)WLANIF, "-p", (char *)ARG_UDP,
						(char *)FW_DPORT, (char *)PORT_DHCP, "-j", (char *)FW_DROP);
					}
					#endif
				}
			}
			
#ifdef WLAN_MBSSID
			for ( k=1; k<5; k++ ) {
				mib_get(MIB_WLAN_VAP0_ITF_GROUP+k-1, (void *)&wlangrp);
				if (wlangrp == pvcEntry.itfGroup) {
					// this lan port shares the same group with pvc, add as pvc's member
					if (vcIdx < MAX_VC_NUM) {
						mbr[vcIdx] |= (1<< (IFWLAN_SHIFT+k) );
						// get rid of local dhcp server if not belonging to default group
						// Commented by Mason Yu
						#if 0
						if (wlangrp != 0) {
							// iptables -A inacc -i $LAN_IF -p UDP --dport 67 -j DROP
							va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_INACC,
							(char *)ARG_I, (char *)WLANAPIF[k], "-p", (char *)ARG_UDP,
							(char *)FW_DPORT, (char *)PORT_DHCP, "-j", (char *)FW_DROP);
						}
						#endif
					}
				}
			}
#endif			
			
		}
#endif
		
		// device is mandatory
		mbr[vcIdx] |= 0x10;
		/* set membership into vc */
		snprintf(wanif, 5, "vc%d", vcIdx);
		// mpoactl set vc0 group 1 member 3
		snprintf(msg.data, BUF_SIZE,
			"mpoactl set %s group 1 member %d",
			wanif, mbr[vcIdx]);
		TRACE(STA_SCRIPT, "%s\n", msg.data);
		write_to_mpoad(&msg);
		/* set membership into LAN ports */
		for (k=0; k<num; k++) {
			// Is this port a member ?
			if (mbr[vcIdx]&(1<<k)) {
				setup_8305_vlan_member(k, mbr[vcIdx]);
			}
		}
#ifdef WLAN_SUPPORT
#ifndef WLAN_MBSSID
		/* set membership into wlan0 */
		if (mbr[vcIdx]&(1<<IFWLAN_SHIFT))
			setup_wlan_group_member(0, mbr[vcIdx]);
			
#else
		for (k=0; k<5; k++) {
			if (mbr[vcIdx]&(1<<(IFWLAN_SHIFT+k)))
				setup_wlan_group_member(k, mbr[vcIdx]);
		}
#endif
#endif
	}
	
	// find the group which is not associated to the WAN port
	for (i=0; i<IFGROUP_NUM; i++) {
		int found, member;
		
		found = 0;
		total = mib_chain_total(MIB_ATM_VC_TBL);
		for (k=0; k<total; k++) {
			if (!mib_chain_get(MIB_ATM_VC_TBL, k, (void *)&pvcEntry))
				continue;
			if (!pvcEntry.enable)
				continue;
			if (pvcEntry.itfGroup == i)
				found = 1;
		}
		if (found)
			continue;
		// This is the outstanding group(i), do it ...
		member = 0;
		num = mib_chain_total(MIB_SW_PORT_TBL);
		for (k=0; k<num; k++) {
			if (!mib_chain_get(MIB_SW_PORT_TBL, k, (void *)&Port))
				continue;
			if (Port.itfGroup == i)
				member |= (1<<k);
		}
		
#ifdef WLAN_SUPPORT
		mib_get(MIB_WLAN_ITF_GROUP, (void *)&wlangrp);
		if (wlangrp == i)
			member |= (1<<IFWLAN_SHIFT);
			
#ifdef WLAN_MBSSID
		for ( k=1; k<5; k++) {
			mib_get( (MIB_WLAN_VAP0_ITF_GROUP+k-1), (void *)&wlanAPgrp[k]);
			if (wlanAPgrp[k] == i)
				member |= (1<< (IFWLAN_SHIFT+k) );
		}
#endif
#endif
		
		// device is mandatory
		member |= 0x10;
		/* set membership into LAN ports */
		for (k=0; k<num; k++) {
			if (!mib_chain_get(MIB_SW_PORT_TBL, k, (void *)&Port))
				continue;
			if (Port.itfGroup == i)
				setup_8305_vlan_member(k, member);
		}
		
#ifdef WLAN_SUPPORT
#ifndef WLAN_MBSSID
		/* set membership into wlan0 */
		if (wlangrp == i)
			setup_wlan_group_member(0, member);
			
#else
		if (wlangrp == i)
			setup_wlan_group_member(0, member);
			
		for ( k=1; k<5; k++ ) {
			/* set membership into wlan0 */
			if (wlanAPgrp[k] == i)
				setup_wlan_group_member(k, member);
			
		}
#endif
#endif
	}
	// set port 4 member to all
	setup_8305_vlan_member(4, 0x1f);
	// enable Port Mapping
	__dev_setupPortMapping(1);
	// enable 8305 vlan
	setup_8305_vlan_enable();
	
	printf("Port Mapping: Ethernet to pvc mapping started\n");
}
#endif	// of ITF_GROUP
#endif /*if 0,jiunming, move to utility.c*/

#if 0
void setupEth2pvc()
{
	MIB_CE_SW_PORT_Tp pPort;
	MIB_CE_ATM_VC_Tp pvcEntry;
	int i, k, total, num;
	char str_br[]="br1";
	struct data_to_pass_st msg;
	char str_vc[5], wanif[5];
	int mbr[MAX_VC_NUM], dft_mbr, pvc_dft_mbr=0;
	
	// register 8305 switch port to system
	//__dev_register_swport();
	total = mib_chain_total(MIB_SW_PORT_TBL);
	
	// init vc group member to zero, device is mandatory
	for (i=0; i<MAX_VC_NUM; i++)
		mbr[i] = 0;
	// init default group member
	dft_mbr=0;
	
	// setup user-specified pvc interface group members
	for (i=0; i<total; i++) {
		pPort = (MIB_CE_SW_PORT_Tp) mib_chain_get(MIB_SW_PORT_TBL,i);
		// port i maps to bit(i+1)
		if (pPort->pvcItf == 0xff)	// 0xff: not mapped
			dft_mbr |= (1<<i);
		else {
			// this lan port maps to a pvc, add as pvc's member
			// TODO: check for pppoa (no VC_INDEX)
			if (VC_INDEX(pPort->pvcItf) < MAX_VC_NUM) {
				mbr[VC_INDEX(pPort->pvcItf)] |= (1<<i);
				// check to get rid of local dhcp server if mapping to bridge
				num = mib_chain_total(MIB_ATM_VC_TBL);
				for (k=0; k<num; k++) {
					char phy[]="eth0_sw0";
					pvcEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL,k);
					if (pvcEntry->ifIndex==pPort->pvcItf && (REMOTE_ACCESS_T)pvcEntry->cmode == ACC_BRIDGED) {
						phy[7]='0'+i;
						// don't touch my dhcp server if mapping to bridge port
						// iptables -A inacc -i $LAN_IF -p UDP --dport 67 -j DROP
						va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_INACC,
						(char *)ARG_I, phy, "-p", (char *)ARG_UDP,
						(char *)FW_DPORT, (char *)PORT_DHCP, "-j", (char *)FW_DROP);
					}
				}
			}
		}
	}
	
	// enable Port Mapping
	__dev_setupPortMapping(1);
	// enable 8305 vlan
	setup_8305_vlan_enable();
	// set pvc interface members into vc
	total = mib_chain_total(MIB_ATM_VC_TBL);
	pvc_dft_mbr = 0;
	for (i = 0; i < total; i++) {
		int vcIdx;
		pvcEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL,i);
		
		if (!pvcEntry->enable)
			continue;
		
		vcIdx = VC_INDEX(pvcEntry->ifIndex);
		if (mbr[vcIdx]==0) {	// It's default group
			mbr[vcIdx] = dft_mbr;
			pvc_dft_mbr = 1;
		}
		// device is mandatory
		mbr[vcIdx] |= 0x10;
		snprintf(wanif, 5, "vc%d", vcIdx);
		// mpoactl set vc0 group 1 member 3
		snprintf(msg.data, BUF_SIZE,
			"mpoactl set %s group 1 member %d",
			wanif, mbr[vcIdx]);
		TRACE(STA_SCRIPT, "%s\n", msg.data);
		write_to_mpoad(&msg);
		for (k=0; k<SW_PORT_NUM; k++) {
			// Is this port a member ?
			if (mbr[vcIdx]&(1<<k)) {
				setup_8305_vlan_member(k, mbr[vcIdx]);
			}
		}
	}
	
	// set default group member
	if (!pvc_dft_mbr) {
		// default group not associated to pvc
		// device is mandatory
		dft_mbr |= 0x10;
		for (k=0; k<SW_PORT_NUM; k++) {
			// Is this port a member ?
			if (dft_mbr&(1<<k)) {
				setup_8305_vlan_member(k, dft_mbr);
			}
		}
	}
	
	// set port 4 member to all
	setup_8305_vlan_member(4, 0x1f);
	
	printf("Port Mapping: Ethernet to pvc mapping started\n");
}

void setupVlan()
{
	MIB_CE_VLAN_Tp pVlan;
	MIB_CE_SW_PORT_Tp pPort;
	struct ifreq ifr;
	struct ifvlan ifvl;
	int i;
	short mbr;
	
	strcpy(ifr.ifr_name, "eth0");
	ifr.ifr_data = (char *)&ifvl;
	
	// setup the Vlan state
	mbr = 0;
	for (i=0; i<VLAN_NUM; i++) {
		pVlan = (MIB_CE_VLAN_Tp) mib_chain_get(MIB_VLAN_TBL,i);
		ifvl.cmd=VLAN_SETINFO;
		ifvl.vlanIdx=i;
		ifvl.vid=pVlan->tag;
		ifvl.member=(pVlan->member | 0x10); // add port 4: eth0
		setVlan(&ifr);
	}
	
	// setup the port state
	for (i=0; i<SW_PORT_NUM; i++) {
		pPort = (MIB_CE_SW_PORT_Tp) mib_chain_get(MIB_SW_PORT_TBL,i);
		ifvl.port=i;
		ifvl.vlanIdx=pPort->pvid;
		pVlan = (MIB_CE_VLAN_Tp) mib_chain_get(MIB_VLAN_TBL,pPort->pvid);
		ifvl.vid=pVlan->tag;
		ifvl.txtag=pPort->egressTagAction;
		// setup PVIDX
		ifvl.cmd=VLAN_SETPVIDX;
		setVlan(&ifr);
		
		// setup port tx tag policy
		ifvl.cmd=VLAN_SETTXTAG;
		setVlan(&ifr);

		// enable vlan 802.1p priority classification
		ifvl.cmd=VLAN_DISABLE1PPRIORITY;
		ifvl.disable_priority=0;
		setVlan(&ifr);
		
		// set port to br mapping, used by add2br()
		brInfo[i].pvcIdx = pPort->pvcItf;
		brInfo[i].brid = 0;	// all ports belong to br0
	}
	
	// setup port 4 (to 867x NIC) tx tag policy
	ifvl.cmd=VLAN_SETTXTAG;
	ifvl.port=4;
	ifvl.txtag=TAG_ADD;
	setVlan(&ifr);

	// enable port 4 vlan 802.1p priority classification
	ifvl.cmd=VLAN_DISABLE1PPRIORITY;
	ifvl.disable_priority=0;
	setVlan(&ifr);
	
	// enable rtl-8305 VLAN
	ifvl.cmd=VLAN_ENABLE;
	ifvl.enable=1;
	setVlan(&ifr);
	
	add2br();
}
#endif

// bitmap for virtual lan port function
// Port Mapping: bit-0
// QoS : bit-1
// IGMP snooping: bit-2
int setup8305()
{
	unsigned char mode;
	
	mib_get(MIB_MPMODE, (void *)&mode);
#ifdef IP_QOS_VPORT
	//if (mode&0x03) // IPQ and PortMapping need to register virtual LAN ports by only once
	if (mode&0x07) // IPQ, PortMapping and IGMP-snooping need to register virtual LAN ports by only once
#else
	//if (mode&0x01)	// PortMapping need to register virtual LAN ports
	if (mode&0x05)	// PortMapping and IGMP-snooping need to register virtual LAN ports
#endif
		__dev_register_swport();
	
	// Note: IGMP Snooping must go ahead of PortMapping since both function
	//	set the VLAN member
	if (mode&0x04)
		__dev_setupIGMPSnoop(1);
#ifdef IP_QOS
#ifdef QOS_DIFFSERV
		unsigned char qosDomain;
		mib_get(MIB_QOS_DIFFSERV, (void *)&qosDomain);
		if (qosDomain == 1)
			setupDiffServ();
		else {
#endif
	if (mode&0x02)
		setupIPQ();
#ifdef QOS_DIFFSERV
		}
#endif
#else
#ifdef NEW_IP_QOS_SUPPORT
	//ql 20081117 START for IP QoS
	setup_qos_setting();
#endif
#endif
#ifdef ITF_GROUP
	if (mode&0x01)
		setupEth2pvc();
#else
#ifdef VLAN_GROUP
	if (mode&0x01)
		setupEth2pvc_vlan();
#endif
#endif	
	
#if 0
	if (mode==1)
		setupEth2pvc();
	else if (mode==2)
		setupVlan();
	else if (mode==3)
		setupIPQ();
	else if (mode==4)
		__dev_setupIGMPSnoop(1);
#endif
	
	return 1;
}


#else	// of CONFIG_EXT_SWITCH

#ifdef ELAN_LINK_MODE_INTRENAL_PHY
// Added by Mason Yu
int setupLinkMode_internalPHY()
{
	restart_ethernet(1);
	return 1;
	
}
#endif
#endif

#if 0 //def IP_QOS, move to utility.c, jiunming
int setupIPQ()
{
	unsigned char enable;
	unsigned int num, i, k;
	//MIB_CE_ATM_VC_Tp pvcEntry;
	MIB_CE_ATM_VC_T pvcEntry;
	//MIB_CE_IP_QOS_Tp pqEntry;
	MIB_CE_IP_QOS_T qEntry;
	//MIB_CE_SW_PORT_Tp pPort;
	char *argv[24], wanif[6];
	
	num = mib_chain_total(MIB_ATM_VC_TBL);
	// Create the root "prio" qdisc on wan interface
	for (i = 0; i < num; i++) {
		char s_level[]="0", s_classid[]="1:3", s_handle[]="3:";
		char vChar;
		/* get the specified chain record */
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&pvcEntry))
			continue;
		if (!pvcEntry.enable)
			continue;
		
		if (PPP_INDEX(pvcEntry.ifIndex) != 0x0f) {
			// PPP interface
			snprintf(wanif, 6, "ppp%u", PPP_INDEX(pvcEntry.ifIndex));
		}
		else {
			// vc interface
			snprintf(wanif, 6, "vc%u", VC_INDEX(pvcEntry.ifIndex));
		}
		// tc qdisc add dev vc0 root handle 1: htb
		// tc qdisc add dev vc0 root handle 1: prio
		// By default, this command instantly create classes 1:1, 1:2
		// and 1:3 and each of them has its pfifo queue already installed.
		#if 0
		va_cmd(TC, 8, 1, "qdisc", (char *)ARG_ADD, "dev", wanif,
			"root", "handle", "1:", "prio");
		#endif
		s_level[0] += IPQOS_NUM_PRIOQ;
		va_cmd(TC, 10, 1, "qdisc", (char *)ARG_ADD, "dev", wanif,
			"root", "handle", "1:", "prio", "bands", s_level);
		// If more than 3 bands (default), the pfifo will not be installed at
		// class(es) 1:4, 1:5, and so forth automatically. We should installed
		// pfifo extrinsically.
		if (IPQOS_NUM_PRIOQ >= 4) {
			for (k=0; k<=IPQOS_NUM_PRIOQ-4; k++) {
				s_classid[2]++;
				s_handle[0]++;
				va_cmd(TC, 9, 1, "qdisc", (char *)ARG_ADD, "dev", wanif,
				"parent", s_classid, "handle", s_handle, "pfifo");
			}
		}
		
		// Set the default mapping of Precedence bit to prio classes
		// tc filter add dev vc0 parent 1:0 prio 2 protocol ip u32
		// match ip tos 0xc0 0xc0 flowid 1:1
		#if 0
		// Assign precedence 5 to high priority queue (bit 101)
		va_cmd(TC, 18, 1, "filter", (char *)ARG_ADD, "dev", wanif,
			"parent", "1:0", "prio", "2", "protocol", "ip", "u32",
			"match", "ip", "tos", "0xa0", "0xe0", "flowid", "1:1");
		// Assign precedence 6, 7 to high priority queue (bit 11x)
		va_cmd(TC, 18, 1, "filter", (char *)ARG_ADD, "dev", wanif,
			"parent", "1:0", "prio", "2", "protocol", "ip", "u32",
			"match", "ip", "tos", "0xc0", "0xc0", "flowid", "1:1");
		// Assign precedence 2, 3 to medium priority queue (bit 01x)
		va_cmd(TC, 18, 1, "filter", (char *)ARG_ADD, "dev", wanif,
			"parent", "1:0", "prio", "2", "protocol", "ip", "u32",
			"match", "ip", "tos", "0x40", "0xc0", "flowid", "1:2");
		// Assign precedence 4 to medium priority queue (bit 100)
		va_cmd(TC, 18, 1, "filter", (char *)ARG_ADD, "dev", wanif,
			"parent", "1:0", "prio", "2", "protocol", "ip", "u32",
			"match", "ip", "tos", "0x80", "0xe0", "flowid", "1:2");
		// Assign precedence 0, 1 to low priority queue (bit 00x)
		va_cmd(TC, 18, 1, "filter", (char *)ARG_ADD, "dev", wanif,
			"parent", "1:0", "prio", "2", "protocol", "ip", "u32",
			"match", "ip", "tos", "0x00", "0xc0", "flowid", "1:3");
		#endif
#ifdef IP_QOS_VPORT
		for (k=0; k<=(IPQOS_NUM_PKT_PRIO-1); k++) {
			char flow[]="1:0";
			char pattern[]="0x00";
			int prio;

			flow[2] += priomap[k];
			prio = k<<1;
			if (prio<=9)
				pattern[2] += prio; // highest 3 bits
			else
				pattern[2] = 'a'+(prio-10);
			// match ip tos PATTERN MASK
			va_cmd(TC, 18, 1, "filter", (char *)ARG_ADD, "dev", wanif,
				"parent", "1:0", "prio", "2", "protocol", "ip", "u32",
				"match", "ip", "tos", pattern, "0xe0", "flowid", flow);
		}
#else
		mib_get(MIB_QOS_DOMAIN, (void *)&vChar);
		if (vChar == (char)PRIO_IP) {
			for (k=0; k<=(IPQOS_NUM_PKT_PRIO-1); k++) {
				char flow[]="1:0";
				char pattern[]="0x00";
				int prio;
                	
				flow[2] += priomap[k];
				prio = k<<1;
				if (prio<=9)
					pattern[2] += prio; // highest 3 bits
				else
					pattern[2] = 'a'+(prio-10);
				// match ip tos PATTERN MASK
				va_cmd(TC, 18, 1, "filter", (char *)ARG_ADD, "dev", wanif,
					"parent", "1:0", "prio", "2", "protocol", "ip", "u32",
					"match", "ip", "tos", pattern, "0xe0", "flowid", flow);
			}
		}
		else { // PRIO_802_1p
			for (k=0; k<=(IPQOS_NUM_PKT_PRIO-1); k++) {
				char s_mark[]="0";
				char flow[]="1:0";
				
				s_mark[0] += (k+1);
				flow[2] += priomap[k];
				va_cmd(TC, 15, 1, "filter", (char *)ARG_ADD, "dev", wanif,
					"parent", "1:0", "prio", "2", "protocol", "ip", "handle", s_mark,
					"fw", "flowid", flow);
			}
		}
#endif
		
	}
	
	num = mib_chain_total(MIB_IP_QOS_TBL);
	// set IP QoS rule
	for (i=0; i<num; i++) {
		char phy[]="eth0_sw0";
		char saddr[20], daddr[20], sport[6], dport[6], mark[4], prio[4];
		char *psaddr, *pdaddr;
		int idx, tos, mark1p;
		int k, vcnum;
		
		if (!mib_chain_get(MIB_IP_QOS_TBL, i, (void *)&qEntry))
			continue;

#ifdef _CWMP_MIB_
		if( qEntry.enable==0 ) continue;
#endif
		
		// source ip, mask
		snprintf(saddr, 20, "%s", inet_ntoa(*((struct in_addr *)qEntry.sip)));
		if (strcmp(saddr, ARG_0x4) == 0)
			psaddr = 0;
		else {
			if (qEntry.smaskbit!=0)
				snprintf(saddr, 20, "%s/%d", saddr, qEntry.smaskbit);
			psaddr = saddr;
		}
		// destination ip, mask
		snprintf(daddr, 20, "%s", inet_ntoa(*((struct in_addr *)qEntry.dip)));
		if (strcmp(daddr, ARG_0x4) == 0)
			pdaddr = 0;
		else {
			if (qEntry.dmaskbit!=0)
				snprintf(daddr, 20, "%s/%d", daddr, qEntry.dmaskbit);
			pdaddr = daddr;
		}
		snprintf(sport, 6, "%d", qEntry.sPort);
		snprintf(dport, 6, "%d", qEntry.dPort);

		mark1p = get_classification_mark(i);
		if (mark1p == 0)
			continue;
		#if 0
		// mark the packet:  8-bits(high) |   8-bits(low)
		//                    class id    |  802.1p (if any)
		mark1p = ((i+1) << 8);	// class id
		if (qEntry.m_1p != 0)
			mark1p |= (qEntry.m_1p-1);	// 802.1p
		#endif
		
		snprintf(mark, 4, "%d", mark1p);
		
		// Rule
		argv[1] = (char *)ARG_T;
		argv[2] = "mangle";
		argv[3] = (char *)FW_ADD;
		argv[4] = (char *)FW_PREROUTING;
		idx = 5;
		
		// lan port
		if (qEntry.phyPort != 0xff) {
#if (defined(CONFIG_EXT_SWITCH)  && defined (IP_QOS_VPORT))
//#ifdef CONFIG_EXT_SWITCH
			if (qEntry.phyPort <= SW_PORT_NUM) {
				phy[7]='0'+qEntry.phyPort;
				argv[idx++] = (char *)ARG_I;
				argv[idx++] = phy;
			}
#else
			if (qEntry.phyPort == 0) {
				argv[idx++] = (char *)ARG_I;
				argv[idx++] = (char *)ELANIF;
			}
#endif
#ifdef WLAN_SUPPORT
			else {
				argv[idx++] = (char *)ARG_I;
				argv[idx++] = (char *)WLANIF;
			}
#endif
		}
		else { // all lan ports
			argv[idx++] = (char *)ARG_I;
			argv[idx++] = (char *)LANIF;
		}
		
		// protocol
		if (qEntry.protoType != PROTO_NONE) {
			argv[idx++] = "-p";
			if (qEntry.protoType == PROTO_TCP)
				argv[idx++] = (char *)ARG_TCP;
			else if (qEntry.protoType == PROTO_UDP)
				argv[idx++] = (char *)ARG_UDP;
			else //if (qEntry.protoType == PROTO_ICMP)
				argv[idx++] = (char *)ARG_ICMP;
		}
		
		// src ip
		if (psaddr != 0)
		{
			argv[idx++] = "-s";
			argv[idx++] = psaddr;
			
		}
		
		// src port
		if ((qEntry.protoType==PROTO_TCP ||
			qEntry.protoType==PROTO_UDP) &&
			qEntry.sPort != 0) {
			argv[idx++] = (char *)FW_SPORT;
			argv[idx++] = sport;
		}
		
		// dst ip
		if (pdaddr != 0)
		{
			argv[idx++] = "-d";
			argv[idx++] = pdaddr;
		}
		// dst port
		if ((qEntry.protoType==PROTO_TCP ||
			qEntry.protoType==PROTO_UDP) &&
			qEntry.dPort != 0) {
			argv[idx++] = (char *)FW_DPORT;
			argv[idx++] = dport;
		}
		
		// target/jump		
		argv[idx++] = "-j";
		
		// Mark traffic class
		argv[idx] = "MARK";
		argv[idx+1] = "--set-mark";
		argv[idx+2] = mark;
		argv[idx+3] = NULL;
		
		// Mark traffic class
		// iptables -t mangle -A PREROUTING -i eth0_sw2
		//	-s 172.21.70.4/24
		//	-d 192.168.1.10/16 -p tcp --sport 25
		//	--dport 22 -j MARK --set-mark 22
		argv[idx] = "MARK";
		argv[idx+1] = "--set-mark";
		argv[idx+2] = mark;
		argv[idx+3] = NULL;
		TRACE(STA_SCRIPT, "%s %s %s %s %s %s %s ...--set-mark %s\n", IPTABLES, argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], mark);
		do_cmd(IPTABLES, argv, 1);
		
#if 0
		snprintf(prio, 4, "%d", (int)qEntry.prior);
		// tc filter add dev vc0 parent 1:0 prio 1 protocol ip handle 22 fw flowid 1:1
		va_cmd(TC, 15, 1, "filter", (char *)ARG_ADD, "dev", wanif,
			"parent", "1:0", "prio", prio, "protocol", "ip", "handle", mark,
			"fw", "flowid", "1:1");
#endif
		
		// Mark ip tos
		if (qEntry.m_ipprio != 0 || qEntry.m_iptos != 0xff) {
			tos = 0;
			if (qEntry.m_ipprio != 0)
				tos = (qEntry.m_ipprio-1) << 5;
			if (qEntry.m_iptos != 0xff)
				tos |= qEntry.m_iptos;
			snprintf(prio, 4, "%d", tos);
			argv[idx] = "TOS";
			argv[idx+1] = "--set-tos";
			argv[idx+2] = prio;
			argv[idx+3] = NULL;
			TRACE(STA_SCRIPT, "%s %s %s %s %s %s %s ...--set-tos %s\n", IPTABLES, argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], prio);
			do_cmd(IPTABLES, argv, 1);
		}
		
		// Configure the filter to place the packets on class
		snprintf(prio, 4, "1:%d", (int)qEntry.prior+1);
		// tc filter add dev vc0 parent 1:0 prio 1 protocol ip handle 22 fw flowid 1:1
		vcnum = mib_chain_total(MIB_ATM_VC_TBL);
		// Create the root "prio" qdisc on wan interface
		for (k = 0; k < vcnum; k++) {
			/* get the specified chain record */
			if (!mib_chain_get(MIB_ATM_VC_TBL, k, (void *)&pvcEntry))
				continue;
			if (!pvcEntry.enable)
				continue;
			
			if (PPP_INDEX(pvcEntry.ifIndex) != 0x0f) {
				// PPP interface
				snprintf(wanif, 6, "ppp%u", PPP_INDEX(pvcEntry.ifIndex));
			}
			else {
				// vc interface
				snprintf(wanif, 6, "vc%u", VC_INDEX(pvcEntry.ifIndex));
			}
			va_cmd(TC, 15, 1, "filter", (char *)ARG_ADD, "dev", wanif,
				"parent", "1:0", "prio", "1", "protocol", "ip", "handle", mark,
				"fw", "flowid", prio);
		}
	}
	
	// register 8305-switch port into system
	//__dev_register_swport();
	
#if 0
	// add sw-port into br0
	for (i=0; i<SW_PORT_NUM; i++) {
		pPort = (MIB_CE_SW_PORT_Tp) mib_chain_get(MIB_SW_PORT_TBL,i);
		// set port to br mapping, used by add2br()
		brInfo[i].pvcIdx = pPort->pvcItf;
		brInfo[i].brid = 0;	// all ports belong to br0
	}
	add2br();
#endif
	// enable IPQoS
#ifdef CONFIG_EXT_SWITCH
	__dev_setupIPQoS(1);
#endif
	
	printf("IP QoS: started\n");
}
#endif	// of IP_QOS

//---------------------------------------------------------
// pEntry: original pvc entry
// bitmap: bit map of the pvc to be searched in pvcList
//----------------------------------------------------------
#if 0
void startPVCAutoSearch(MIB_CE_ATM_VC_Tp pEntry, unsigned long bitmap)
{
	int idx;
	unsigned long mask=1;
	unsigned long ovpi, ovci;
	unsigned char vChar;
	BOOT_TYPE_T btmode;
	int i;

#ifdef AUTO_PVC_SEARCH	//  read PVC table from flash
	unsigned int entryNum, Counter;
	//MIB_AUTO_PVC_SEARCH_Tp entryP;
	MIB_AUTO_PVC_SEARCH_T Entry;
	entryNum = mib_chain_total(MIB_AUTO_PVC_SEARCH_TBL);
	if(entryNum > MAX_PVC_SEARCH_PAIRS)
		entryNum = MAX_PVC_SEARCH_PAIRS;
			
#endif
	if (!pEntry || !pEntry->enable)
		return;
    for(i=0; i<16; i++) { 
        if (doOAMLookback(pEntry)!=1) 
	{
		if(i==0)					
			stopConnection(pEntry);	
#ifndef AUTO_PVC_SEARCH	
		int sok=0;
#endif						
		// start searching ...
		printf("Default pvc failed, start searching ...\n");
		ovpi = pEntry->vpi;
		ovci = pEntry->vci;
#ifndef AUTO_PVC_SEARCH 	//Retrieve PVC list from Flash	
		for (idx=0; pvcList[idx].vpi || pvcList[idx].vci; idx++) {
			if (bitmap & mask) {
				stopConnection(pEntry);
				pEntry->vpi = pvcList[idx].vpi;
				pEntry->vci = pvcList[idx].vci;
				startConnection(pEntry);
				if (doOAMLookback(pEntry)) {
					// That's it
					printf("That's it! vpi=%d, vci=%d\n", pEntry->vpi, pEntry->vci);
					sok=1;
					break;
				}
			}
			mask <<= 1;
		}
#else
		for(Counter=0; Counter< entryNum; Counter++)
		{
			if (!mib_chain_get(MIB_AUTO_PVC_SEARCH_TBL, Counter, (void *)&Entry))
				continue;
			//stopConnection(pEntry);
			pEntry->vpi = Entry.vpi;
			pEntry->vci = Entry.vci;
			//startConnection(pEntry);
			if (doOAMLookback(pEntry)) {
				// That's it
				printf("That's it! vpi=%d, vci=%d\n", pEntry->vpi, pEntry->vci);
				printf("Inform SAR driver to stop pvc search\n");
				StopSarAutoPvcSearch(pEntry->vpi, pEntry->vci);	
				sok=1;
				break;
			}
			
		}

#endif
		if (sok) { // search ok, set it
			startConnection(pEntry);				
			if (mib_get(MIB_BOOT_MODE, (void *)&vChar))
				btmode = (BOOT_TYPE_T)vChar;
			else
				btmode = BOOT_LAST;
			
			/* upgdate to flash */
			// only "boot from last" can be updated
			if (btmode == BOOT_LAST) {
				printf("set as default\n");
				mib_chain_update(MIB_ATM_VC_TBL, (void *)pEntry, 0);
				mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
			}
			break;
		}
		else { // search failed, back to original
			//stopConnection(pEntry);
			pEntry->vpi = ovpi;
			pEntry->vci = ovci;
			//startConnection(pEntry);
			printf("Auto-search failed !\n");
		}
	}
	else
	{
		printf("first-pvc oam loopback ok!\n");
		sok=1;
		StopSarAutoPvcSearch(pEntry->vpi, pEntry->vci);			
       	break;
       }
   }
}
#endif


//--------------------------------------------------------
// LAN side IP autosearch using ARP
// Input: current IP address
// return value:
// 1  : successful
// -1 : failed
int startLANAutoSearch(const char *ipAddr, const char *subnet)
{
	unsigned char netip[4];
	struct in_addr *dst;
	int k, found;
	
	TRACE(STA_INFO, "Start LAN IP autosearch\n");
	dst = (struct in_addr *)netip;
	
	if (!inet_aton(ipAddr, dst)) {
		printf("invalid or unknown target %s", ipAddr);
		return -1;
	}
	
	if (isDuplicate(dst, LANIF)) {
		TRACE(STA_INFO, "Duplicate LAN IP found !\n");
		found = 0;
		inet_aton("192.168.1.254", dst);
		if (isDuplicate(dst, LANIF)) {
			netip[3] = 63;	// 192.168.1.63
			if (isDuplicate(dst, LANIF)) {
				// start from 192.168.1.253 and descending
				for (k=253; k>=1; k--) {
					netip[3] = k;
					if (!isDuplicate(dst, LANIF)) {
						// found it
						found = 1;
						TRACE(STA_INFO, "Change LAN ip to %s\n", inet_ntoa(*dst));
						break;
					}
				}
			}
			else {
				// found 192.168.1.63
				found = 1;
				TRACE(STA_INFO, "Change LAN ip to %s\n", inet_ntoa(*dst));
			}
		}
		else {
			// found 192.168.1.254
			found = 1;
			TRACE(STA_INFO, "Change LAN ip to %s\n", inet_ntoa(*dst));
		}
		
		if (!found) {
			printf("not available LAN IP !\n");
			return -1;
		}
		
		// ifconfig LANIF LAN_IP netmask LAN_SUBNET
		va_cmd(IFCONFIG, 4, 1, (char*)LANIF, inet_ntoa(*dst), "netmask", subnet);
	}
	
	return 1;
}

#if 0
void addStaticRoute()
{
	unsigned int entryNum, i;
	MIB_CE_IP_ROUTE_T Entry;
	//struct rtentry rt;
	//struct sockaddr_in *inaddr;
	//char	ifname[17];
	
	/* Clean out the RTREQ structure. */
	//memset((char *) &rt, 0, sizeof(struct rtentry));
	entryNum = mib_chain_total(MIB_IP_ROUTE_TBL);
	
	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_IP_ROUTE_TBL, i, (void *)&Entry))
		{
			return;
		}
		route_cfg_modify(&Entry, 0);
	}
}
#endif

//--------------------------------------------------------
// Final startup
// return value:
// 1  : successful
// -1 : failed
int startRest(BOOT_TYPE_T bt_mode)
{
	int vcTotal, i;
	unsigned char autosearch, mode;
	MIB_CE_ATM_VC_T Entry;
	int status=0;
#ifdef AUTO_PROVISIONING
	unsigned char serverIP[20];
	unsigned char strbuf[16];
	int len;
#endif

	// start webs
	if (bt_mode != BOOT_UPGRADE)
		status|=va_cmd(WEBSERVER, 0, 0);
	else
		status|=va_cmd(UPGRADE_WEB, 0, 0);
/*
	// start snmpd
	va_cmd(SNMPD, 0, 0);
*/
	
	// Add static routes
	addStaticRoute();
	
	if (bt_mode == BOOT_LAST) {	
		//Added by Mason Yu for start DHCP relay	
		// We only can choice DHCP Server or Relay one.
		if (-1==startDhcpRelay()) 
		    return -1;
		
#ifdef CONFIG_EXT_SWITCH
		// rtl-8305 VLAN setting
		setup8305();
#else
#ifdef IP_QOS
		mib_get(MIB_MPMODE, (void *)&mode);
#ifdef QOS_DIFFSERV
		unsigned char qosDomain;
		mib_get(MIB_QOS_DIFFSERV, (void *)&qosDomain);
		if (qosDomain == 1)
			setupDiffServ();
		else {
#endif
		if (mode&0x02)
			setupIPQ();
#ifdef QOS_DIFFSERV
		}
#endif
#elif defined(NEW_IP_QOS_SUPPORT)
		//ql 20081117 START for IP QoS
		setup_qos_setting();
#endif
#ifdef  CONFIG_RTL867X_COMBO_PORTMAPPING
	if (mode&0x01)
		setupEth2pvc();
#endif

#endif
	}


	// ioctl for direct bridge mode, jiunming
	{
		unsigned char  drtbr_mode;
		if (mib_get(MIB_DIRECT_BRIDGE_MODE, (void *)&drtbr_mode) != 0)
		{
			__dev_setupDirectBridge( (int) drtbr_mode );
		}
	}
	

	// Jenny, auto-provisioning
#ifdef AUTO_PROVISIONING
	if (!mib_get( MIB_HTTP_SERVER_IP,  (void *)strbuf)){
		printf("Get HTTP server IP failed!\n");
	}
	if (((struct in_addr *)strbuf)->s_addr != 0)
		strncpy(serverIP, inet_ntoa(*((struct in_addr *)strbuf)), 16);
	len = strlen(serverIP);
	serverIP[len] = '\0';
	va_cmd("/bin/auto_provisioning", 1, 0, serverIP);
#endif

#ifdef DOS_SUPPORT
	// for DOS support
	setup_dos_protection();
#endif

     #ifdef CONFIG_IGMP_FORBID
	 
       unsigned char igmpforbid_mode;

	 if (!mib_get( MIB_IGMP_FORBID_ENABLE,  (void *)&igmpforbid_mode)){
		printf("igmp forbid  parameter failed!\n");
	}
	 if(1==igmpforbid_mode){
             __dev_igmp_forbid(1);
	 }
     #endif

	return 1;
}

void clear_child(int i)
{
	int status;
	wait( &status );
	return;
}

extern unsigned char cs_valid;
extern unsigned char ds_valid;
extern unsigned char hs_valid;

#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
// Added by Mason Yu
static int getSnmpConfig(void)
{
	
	//char *str1, *str2, *str3, *str4, *str5;
	unsigned char str[256];
	FILE *fp;
	
	fp=fopen("/tmp/snmp", "w+");

		
	mib_get(MIB_SNMP_SYS_DESCR, (void *)str);
	fprintf(fp, "%s\n", str);	
		
	
	mib_get( MIB_SNMP_SYS_CONTACT,  (void *)str);
	fprintf(fp, "%s\n", str);
	
	
	mib_get( MIB_SNMP_SYS_NAME,  (void *)str);
	fprintf(fp, "%s\n", str);
	
	
	mib_get( MIB_SNMP_SYS_LOCATION,  (void *)str);
	fprintf(fp, "%s\n", str);		
	
	
	mib_get( MIB_SNMP_SYS_OID,  (void *)str);
	fprintf(fp, "%s\n", str);
	
	
  	fclose(fp);
	return 0;	
}
#endif
	

/*
 * system initialization, checking, setup, etc.
 */
const char PW_HOME_DIR[] = "/tmp";
static int sys_setup()
{
	key_t key;
	int qid, vInt, activePVC, ret;
	int i;
	MIB_CE_ATM_VC_T Entry;
	unsigned char value[32];
	FILE *fp;
	char userName[MAX_NAME_LEN], userPass[MAX_NAME_LEN];
	char *xpass;
#ifdef ACCOUNT_CONFIG
	MIB_CE_ACCOUNT_CONFIG_T entry;
	unsigned int totalEntry;
#endif
	
	ret = 0;
	//----------------- check if configd is ready -----------------------
	key = ftok("/bin/init", 'k');
	for (i=0; i<=100; i++) {
		if (i==100) {
			printf("Error: configd not started !!\n");
			return 0;
		}
		if ((qid = msgget( key, 0660 )) == -1)
			usleep(30000);
		else
			break;
	}
	
	//------------------- set active pvc number to system ---------------
	//12/30/05' hrchen, for PVC desc number setting
	RegisterPVCnumber();
	
	//----------------
	// Mason Yu
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
	getSnmpConfig();
#endif
	// ftpd: /etc/passwd & /tmp (as home dir)
	fp=fopen("/var/passwd", "w+");
#ifdef ACCOUNT_CONFIG
	totalEntry = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL); /* get chain record size */
	for (i=0; i<totalEntry; i++) {
		if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, i, (void *)&entry)) {
			printf("ERROR: Get account configuration information from MIB database failed.\n");
			return;
		}
		strcpy(userName, entry.userName);
		strcpy(userPass, entry.userPassword);
		xpass = crypt(userPass, "$1$");
		//if (xpass) {
			//#ifdef CONFIG_USER_MENU_CLI
			//#ifdef CONFIG_USER_CLI
			//if (entry.privilege == (unsigned char)PRIV_ROOT)
			//	fprintf(fp, "%s:%s:0:0::%s:/bin/cli\n", userName, xpass, PW_HOME_DIR);
			//else
			//	fprintf(fp, "%s:%s:1:0::%s:/bin/cli\n", userName, xpass, PW_HOME_DIR);
			//#else
			//if (entry.privilege == (unsigned char)PRIV_ROOT)
			//	fprintf(fp, "%s:%s:0:0::%s:/bin/sh\n", userName, xpass, PW_HOME_DIR);
			//else
			//	fprintf(fp, "%s:%s:1:0::%s:/bin/sh\n", userName, xpass, PW_HOME_DIR);
			//#endif
		//}
	}
#endif
	mib_get( MIB_SUSER_NAME, (void *)userName );
	mib_get( MIB_SUSER_PASSWORD, (void *)userPass );
	xpass = crypt(userPass, "$1$");
	if (xpass)
//#ifdef CONFIG_USER_CLI
//		fprintf(fp, "%s:%s:0:0:root:/:/bin/cli\n", userName, xpass);
//#else
		fprintf(fp, "%s:%s:0:0:root:/:/bin/sh\n", userName, xpass);
//#endif
		
	// Added by Mason Yu for others user	
	mib_get( MIB_SUPER_NAME, (void *)userName );
	mib_get( MIB_SUPER_PASSWORD, (void *)userPass );
	xpass = crypt(userPass, "$1$");
//	if (xpass)
//#ifdef CONFIG_USER_CLI
//		fprintf(fp, "%s:%s:0:0::%s:/bin/cli\n", userName, xpass, PW_HOME_DIR);
//#else
//		fprintf(fp, "%s:%s:0:0:root:/:/bin/sh\n", userName, xpass);
//#endif

			
#if 0 // anonymous ftp
	// added for anonymous ftp
	fprintf(fp, "%s:%s:10:10::/tmp:/dev/null\n", "ftp", "x");
#endif
	mib_get( MIB_USER_NAME, (void *)userName );
	if (userName[0]) {
		mib_get( MIB_USER_PASSWORD, (void *)userPass );				
		xpass = crypt(userPass, "$1$");
//		if (xpass)
//#ifdef CONFIG_USER_CLI
//			fprintf(fp, "%s:%s:1:0::%s:/bin/cli\n", userName, xpass, PW_HOME_DIR);
//#else
//			fprintf(fp, "%s:%s:1:0::%s:/bin/sh\n", userName, xpass, PW_HOME_DIR);
//#endif

	}	
	fclose(fp);
	chmod(PW_HOME_DIR, 0x1fd);	// let owner and group have write access
	// Kaohj --- force kernel(linux-2.6) igmp version to 2
#ifdef FORCE_IGMP_V2
	fp = fopen((const char *)PROC_FORCE_IGMP_VERSION,"w");
	if(fp)
	{
		fprintf(fp, "%d", 2);
		fclose(fp);
	}
#endif
	
	return ret;
}

int main(void)
{
	unsigned char value[32];
	int vInt;
	BOOT_TYPE_T btmode;
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP	
	int snmpd_pid;
#endif	
	// set debug mode
	DEBUGMODE(STA_INFO|STA_SCRIPT|STA_WARNING|STA_ERR);

	
	
	if (sys_setup() == -1)
		goto startup_fail;
	
	if (-1==startELan())
		goto startup_fail;
	
	// check INIT_SCRIPT
	if (mib_get(MIB_INIT_SCRIPT, (void *)value) != 0)
	{
		vInt = (int)(*(unsigned char *)value);
	}
	else
		vInt = 1;
	
	if (vInt == 0)
	{
		// ifconfig eth0 up
		va_cmd(IFCONFIG, 2, 1, "eth0", "up");
#ifdef CONFIG_USB_ETH
		va_cmd(IFCONFIG, 2, 1, USBETHIF, "up");
#endif //CONFIG_USB_ETH

#ifdef CONFIG_EXT_SWITCH
#ifdef ELAN_LINK_MODE
		setupLinkMode();
#endif
#endif
		va_cmd(WEBSERVER, 0, 0);
		return 0;	// stop here
	}	
	
	
	btmode = mib_get_bootmode();
	
	if (btmode == BOOT_UPGRADE) {
		char bm = (char)BOOT_LAST;
		// go back to normal mode after restarting
		mib_set(MIB_BOOT_MODE, (void *)&bm);
		mib_update(HW_SETTING, CONFIG_MIB_ALL);
	}
	
	// Kaohj --- generate the XML file
	va_cmd("/bin/CreatexmlConfig", 0, 1);
	#ifdef XOR_ENCRYPT
	cmd_xml2file(CONFIG_XMLFILE, CONFIG_XMLENC);
	#endif
	
	if (-1==startDaemon(btmode))
		goto startup_fail;
	
	// start WAN interface ...
	if (-1==startWan(btmode))
		goto startup_fail;
	
	// ifconfig eth0 up
	if (va_cmd(IFCONFIG, 2, 1, "eth0", "up"))
		goto startup_fail;	
#ifdef CONFIG_USB_ETH
	if (va_cmd(IFCONFIG, 2, 1, USBETHIF, "up"))
		goto startup_fail;	
#endif //CONFIG_USB_ETH

#ifdef SYSLOG_SUPPORT
	if (btmode == BOOT_LAST)
		if(-1==startLog())
			goto startup_fail;
#endif
	
	if (-1==startDsl())
		goto startup_fail;
	
#ifdef CONFIG_EXT_SWITCH
#ifdef ELAN_LINK_MODE
	if (-1==setupLinkMode())
		goto startup_fail;
#endif
#else
#ifdef ELAN_LINK_MODE_INTRENAL_PHY	
	setupLinkMode_internalPHY();
#endif
#endif
	
#ifdef WLAN_SUPPORT
	if (btmode == BOOT_LAST)
		//if (-1==startWLan())
		//	goto startup_fail;
		startWLan();
#endif

#if (defined(CONFIG_RTL867X_NETLOG)  && defined (CONFIG_USER_NETLOGGER_SUPPORT))
//#ifndef CONFIG_8M_SDRAM
#if !defined(CONFIG_8M_SDRAM) || defined(CONFIG_GUI_WEB)
        va_cmd(NETLOGGER,0,1);
#endif
#endif
	
#ifdef _CWMP_MIB_ /*jiunming, mib for cwmp-tr069*/
	if (btmode == BOOT_LAST)
		if (-1==startCWMP())
			goto startup_fail;	
#endif
#ifdef _PRMT_TR143_
	if (btmode == BOOT_LAST)
	{
		struct TR143_UDPEchoConfig echoconfig;
		UDPEchoConfigSave( &echoconfig );
		UDPEchoConfigStart( &echoconfig );
	}
#endif //_PRMT_TR143_
	
	//ql 20081117 START init MIB_QOS_UPRATE before startup IP QoS
#ifdef NEW_IP_QOS_SUPPORT
	unsigned int up_rate=0;
	mib_set(MIB_QOS_UPRATE, (void *)&up_rate);
#endif
	
	if (-1==startRest(btmode))
		goto startup_fail;
	
#ifdef CONFIG_USER_PPPOE_PROXY
       va_cmd("/bin/pppoe-server",0,1);
#endif

//star add: for ZTE LED request
//#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
//#ifdef ZTE_LED_REQUEST
// Kaohj --- TR068 Power LED
	FILE *fp;
	unsigned char power_flag;
	fp = fopen("/proc/power_flag","w");
	if(fp)
	{
		power_flag = '0';
		fwrite(&power_flag,1,1,fp);
		fclose(fp);
	}
//#endif

//#ifdef _CWMP_MIB_ /*jiunming, mib for cwmp-tr069*/
	/*when those processes created by startup are killed,
	  they will be zombie processes,jiunming*/
	signal(  SIGCHLD, clear_child);//add by star
//#endif

	/*ql: 20081117 START startup qos here*/
#ifdef NEW_IP_QOS_SUPPORT
	printf("start monitor QoS!\n");
	while(1)
	{
		signal(SIGCHLD, SIG_DFL);
		monitor_qos_setting();
		signal(SIGCHLD, clear_child);
		usleep(5000000); // wait 5 sec 
	}
#endif
	/*ql 20081117 END*/

#if 0
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
	// Mason Yu. System init status	
	snmpd_pid = read_pid("/var/run/snmpd.pid");
	if (snmpd_pid > 0) {
		printf("Send signal to snmpd.\n");		
		kill(snmpd_pid, SIGUSR1);
	}
#endif
#endif	
	return 0;
	
startup_fail:	
	//set ALARM LED
	va_cmd("/bin/ethctl", 2, 1, "alarm", "on");
	va_cmd("/bin/boa",0,1);
	printf("ALARM LED ON\n");
	return -1;
}

