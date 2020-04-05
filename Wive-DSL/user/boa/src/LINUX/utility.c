/*
 *      Utiltiy function to communicate with TCPIP stuffs
 *
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *
 *      $Id: utility.c,v 1.459 2009/02/20 09:43:38 cathy Exp $
 *
 */

/*-- System inlcude files --*/
#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/route.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/sysinfo.h>

#include <netinet/if_ether.h>
#include <net/if.h>
#include <linux/sockios.h>
#ifdef EMBED
#include <linux/config.h>
#include <config/autoconf.h>
#else
#include "../../../../uClibc/include/linux/autoconf.h"
#include "../../../../config/autoconf.h"
#endif
//add by ramen to use struct PPPOE_DRV_CTRL for save_pppoe_sessionid()
#include "../../../spppd/pppoe.h"

/*-- Local include files --*/
#include "mib.h"
#include "utility.h"
#include "adsl_drv.h"
#include "vendor.h"

/* for open(), lseek() */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "debug.h"

#include "wireless.h"
#ifdef CONFIG_ATM_CLIP
#include <linux/atmclip.h>
#include <linux/atmarp.h>
#endif

//xl_yue
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
#include "../defs.h"
#endif

#include <linux/if_bridge.h>
#include <netdb.h>
#include <netinet/ip_icmp.h>


#ifdef ELAN_LINK_MODE_INTRENAL_PHY
typedef __uint64_t u64;
typedef __uint32_t u32;
typedef __uint16_t u16;
typedef __uint8_t u8;
#include <linux/ethtool.h>
#endif

#define PR_VC_START	1
#define	PR_PPP_START	16

static int finished_OAMLB = 0;
static int finished_OAMLB_F5_EndToEnd = 0;

const char LANIF[] = "br0";
const char LAN_ALIAS[] = "br0:0";	// alias for secondary IP
const char LAN_IPPT[] = "br0:1";	// alias for IP passthrough
const char WLANIF[] = "wlan0";
const char ELANIF[] = "eth0";
const char BRIF[] = "br0";
#ifdef CONFIG_USB_ETH
const char USBETHIF[] = "usb0";
#endif //CONFIG_USB_ETH
#ifdef WLAN_WDS
const char WDSIF[]="wlan0-wds0";
#endif
const char VC_BR[] = "0";
const char LLC_BR[] = "1";
const char VC_RT[] = "3";
const char LLC_RT[] = "4";
#ifdef CONFIG_ATM_CLIP
const char LLC_1577[] = "6";
#endif
const char PORT_DNS[] = "53";
const char PORT_DHCP[] = "67";
const char BLANK[] = "";
const char ARG_ADD[] = "add";
const char ARG_DEL[] = "del";
const char ARG_ENCAPS[] = "encaps";
const char ARG_QOS[] = "qos";
const char ARG_255x4[] = "255.255.255.255";
const char ARG_0x4[] = "0.0.0.0";
const char ARG_BKG[] = "&";
const char ARG_I[] = "-i";
const char ARG_O[] = "-o";
const char ARG_T[] = "-t";
const char ARG_TCP[] = "TCP";
const char ARG_UDP[] = "UDP";
//#ifdef ZTE_GENERAL_ROUTER_SC
#if defined(ZTE_GENERAL_ROUTER_SC) || defined(NEW_IP_QOS_SUPPORT)
const char ARG_TCPUDP[] = "TCP/UDP";
#endif
const char ARG_ICMP[] = "ICMP";
const char FW_BLOCK[] = "block";
const char FW_INACC[] = "inacc";
const char FW_IPFILTER[] = "ipfilter";
const char FW_MACFILTER[] = "macfilter";
const char FW_APPFILTER[] = "appfilter";
const char FW_APP_P2PFILTER[] = "appp2pfilter";
#ifdef PORT_FORWARD_ADVANCE
const char FW_PPTP[] = "pptp";
const char FW_L2TP[] = "l2tp";
#endif
const char PORT_FW[] = "portfw";
const char IPTABLE_DMZ[] = "dmz";
const char IPTABLE_IPFW[] = "ipfw";
const char IPTABLE_IPFW2[] = "ipfw_PostRouting";
const char IPTABLES_PORTTRIGGER[] = "portTrigger";
const char FW_DROP[] = "DROP";
const char FW_ACCEPT[] = "ACCEPT";
const char FW_RETURN[] = "RETURN";
const char FW_FORWARD[] = "FORWARD";
const char FW_INPUT[] = "INPUT";
const char FW_PREROUTING[] = "PREROUTING";
const char FW_POSTROUTING[] = "POSTROUTING";
const char FW_DPORT[] = "--dport";
const char FW_SPORT[] = "--sport";
const char FW_ADD[] = "-A";
const char FW_DEL[] = "-D";
const char RMACC_MARK[] = "0x1000";
const char CONFIG_HEADER[] = "<Config_Information_File_8671>";
const char CONFIG_TRAILER[] = "</Config_Information_File_8671>";
const char CONFIG_XMLFILE[] = "/tmp/config.xml";
const char CONFIG_XMLENC[] = "/tmp/config.enc";
const char PPP_SYSLOG[] = "/tmp/ppp_syslog";
const char PPP_DEBUG_LOG[] = "/tmp/ppp_debug_log";

const char ADSLCTRL[] = "/bin/adslctrl";
const char IFCONFIG[] = "/bin/ifconfig";
const char BRCTL[] = "/bin/brctl";
const char MPOAD[] = "/bin/mpoad";
const char MPOACTL[] = "/bin/mpoactl";
const char DHCPD[] = "/bin/udhcpd";
const char DHCPC[] = "/bin/udhcpc";
const char DNSRELAY[] = "/bin/dnsmasq";
const char SPPPD[] = "/bin/spppd";
const char SPPPCTL[] = "/bin/spppctl";
const char WEBSERVER[] = "/bin/boa";
const char UPGRADE_WEB[] = "/bin/httpd";
const char SNMPD[] = "/bin/snmpd";
const char ROUTE[] = "/bin/route";
const char IPTABLES[] = "/bin/iptables";
/*ql 20081114 START need ebtables support*/
const char EBTABLES[] = "/bin/ebtables";
const char ZEBRA[] = "/bin/zebra";
const char RIPD[] = "/bin/ripd";
#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
const char OSPFD[] = "/bin/ospfd";
#endif
const char ROUTED[] = "/bin/routed";
const char IGMPROXY[] = "/bin/igmpproxy";
#if (defined(CONFIG_RTL867X_NETLOG)  && defined (CONFIG_USER_NETLOGGER_SUPPORT))
const char NETLOGGER[]="/bin/netlogger";
#endif
const char TC[] = "/bin/tc";
#ifdef WLAN_SUPPORT
const char IWPRIV[] = "/bin/iwpriv";
const char AUTH[] = "/bin/auth";
const char IWCONTROL[] = "/bin/iwcontrol";
const char AUTH_PID[] = "/var/run/auth-wlan0.pid";
#endif
#if defined(TIME_ZONE) || defined(ZTE_GENERAL_ROUTER_SC)
const char SNTPC[] = "/bin/vsntp";
const char SNTPC_PID[] = "/var/run/vsntp.pid";
#endif
const char ROUTED_PID[] = "/var/run/routed.pid";
#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
const char ZEBRA_PID[] = "/var/run/zebra.pid";
const char OSPFD_PID[] = "/var/run/ospfd.pid";
#endif
const char IGMPPROXY_PID[] = "/var/run/igmp_pid";

const char PROC_DYNADDR[] = "/proc/sys/net/ipv4/ip_dynaddr";
const char PROC_IPFORWARD[] = "/proc/sys/net/ipv4/ip_forward";
const char PROC_NET_ATM_PVC[] = "/proc/net/atm/pvc";
const char PROC_FORCE_IGMP_VERSION[] = "/proc/sys/net/ipv4/conf/default/force_igmp_version";
const char PPP_CONF[] = "/var/ppp/ppp.conf";
const char PPPD_FIFO[] = "/tmp/ppp_serv_fifo";
const char MPOAD_FIFO[] = "/tmp/serv_fifo";
const char DHCPC_SCRIPT[] = "/etc/scripts/udhcpc.sh";
const char DHCPC_SCRIPT_NAME[] = "/var/udhcpc/udhcpc";
const char DHCPC_PID[] = "/var/run/udhcpc.pid";
const char IPOA_IPINFO[] = "/tmp/IPoAHalfBridge";
const char MER_GWINFO[] = "/tmp/MERgw";
#ifdef WLAN_SUPPORT
const char WLAN_AUTH_CONF[] = "/var/config/wlan0.conf";
#define WIRELESSAUTHPID  "/var/run/auth-wlan0.pid"
#define IWCONTROLPID  "/var/run/iwcontrol.pid"
#define WSCDPID  "/var/run/wscd-wlan0.pid"
#define MINI_UPNPDPID  "/var/run/mini_upnpd.pid"	//cathy
#define MINIUPNPDPID  "/var/run/linuxigd.pid"

char *WLANAPIF[] = {
	"wlan0", "wlan0-vap0", "wlan0-vap1", "wlan0-vap2", "wlan0-vap3", 0
};

const char *WLANAUTHPID[] = {
	"/var/run/auth-wlan0.pid", "/var/run/auth-wlan0-vap0.pid", "/var/run/auth-wlan0-vap1.pid", "/var/run/auth-wlan0-vap2.pid", "/var/run/auth-wlan0-vap3.pid", 0
};

const char *WLANAUTHCONF[] = {
	"/var/config/wlan0.conf", "/var/config/wlan0-vap0.conf", "/var/config/wlan0-vap1.conf", "/var/config/wlan0-vap2.conf", "/var/config/wlan0-vap3.conf", 0
};

const char *WLANAUTHFIFO[] = {
	"/var/auth-wlan0.fifo", "/var/auth-wlan0-vap0.fifo", "/var/auth-wlan0-vap1.fifo", "/var/auth-wlan0-vap2.fifo", "/var/auth-wlan0-vap3.fifo", 0
};
#endif
const char STR_DISABLE[] = "Disabled";
const char STR_ENABLE[] = "Enabled";
const char STR_AUTO[] = "Auto";
const char STR_MANUAL[] = "Manual";
const char STR_UNNUMBERED[] = "unnumbered";
const char STR_ERR[] = "err";
const char STR_NULL[] = "null";
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
const char rebootWord0[] = "系统重启中 ...";
const char rebootWord1[] = "系统已被重新配置，正在重启中.";
const char rebootWord2[] = "关闭网页浏览器，2分钟后再重新打开，如果需要，请重新设定PC的IP地址!";


int prio2QoS[4]={1,3,2,0}; //mapping priority to ATM QoS     0---> CBR (1); 1---> rtvbr(3) 2 --->nrtvbr(2); 3---->ubr(0). 
int QoS2Prio[4]={3, 0, 2, 1};

#else
const char rebootWord0[] = "The System is Restarting ...";
const char rebootWord1[] = "The DSL Router has been configured and is rebooting.";
const char rebootWord2[] = "Close the DSL Router Configuration window and wait"
			" for a minute before reopening your web browser."
			" If necessary, reconfigure your PC's IP address to match"
			" your new configuration.";
#endif
// TR-111: ManufacturerOUI, ProductClass
const char MANUFACTURER_OUI[] = "00E04C";
const char PRODUCT_CLASS[] = "IGD";

//added by xl_yue: supporting wlcountry list
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
struct wlCountryReg wlCountryRegList[] = {
	{strDefault,1},
	{strUSA,1},
	{strCHINA,3},
	{strHONGKONG,3},
	{strMACAO,3},
	{strTAIWAN,1},
	{strJAPAN,6},
	{strEUROPE,3},
	{strANTIGUA_BARBUDA,3},
	{strARGENTINA,3},
	{strARUBA,3},
	{strAUSTRALIA,3},
	{strAUSTRIA,3},
	{strBAHAMAS,3},
	{strBANGLADESH,3},
	{strBARBADOS,3},
	{strBELGIUM,3},
	{strBERMUDA,3},
	{strBOLIVIA ,3},
	{strBOSNIA_HERZEGOVINA,3},
	{strBRAZIL,3},
	{strBULGARIA,3},
	{strCANADA,1},
	{strCAYMAN_ISLANDS,3},
	{strCHILE,3},
	{strCOLOMBIA,3},
	{strCOSTA_RICA,3},
	{strCROATIA,3},
	{strCYPRUS,3},
	{strCZECH_REPUBLIC,3},
	{strDENMARK,3},
	{strDOMINICAN_REPUBLIC,3},
	{strECUADOR,3},
	{strEGYPT,3},
	{strEL_SALVADOR,3},
	{strESTONIA,3},
	{strFINLAND,3},
	{strFRANCE,3},
	{strGERMANY,3},
	{strGREECE,3},
	{strGUAM,3},
	{strGUATEMALA,3},
	{strHAITI,3},
	{strHONDURAS,3},
	{strHUNGARY,3},
	{strICELAND,3},
	{strINDIA,3},
	{strINDONESIA,3},
	{strIRELAND,3},
	{strISRAEL,3},
	{strITALY,3},
	{strJORDAN,3},
	{strKOREA,3},
	{strLATVIA,3},
	{strLIECHTENSTEIN,3},
	{strLITHUANIA,3},
	{strLUXEMBOURG,3},
	{strMALAYSIA,3},
	{strMALTA,3},
	{strMEXICO,3},
	{strMONACO,3},
	{strNETHERLANDS,3},
	{strNETHERLANDS_ANTILLES,3},
	{strNEW_ZEALAND,3},
	{strNICARAGUA,3},
	{strNIGERIA,3},
	{strNORWAY,3},
	{strPAKISTAN,3},
	{strPANAMA,1},
	{strPARAGUAY,3},
	{strPERU,3},
	{strPHILIPPINES,3},
	{strPOLAND,3},
	{strPORTUGAL,3},
	{strROMANIA,3},
	{strRUSSIAN_FEDERATION,3},
	{strSAUDI_ARABIA,3},
	{strSINGAPORE,3},
	{strSLOVAKIA,3},
	{strSLOVENIA,3},
	{strSOUTH_AFRICA,3},
	{strSPAIN,3},
	{strSWEDEN,3},
	{strSWITZERLAND,3},
	{strTANZANIA,3},
	{strTHAILAND,3},
	{strTUNISIA,3},
	{strTURKEY,3},
	{strUKRAINE,3},
	{strUNITED_ARAB_EMIRATES,3},
	{strUNITED_KINGDOM,3},
	{strURUGUAY,3},
	{strVENEZUELA,3},
	{strVIET_NAM,3},
	{"\0",0}
};
#endif

//eason
#ifdef _PRMT_USB_ETH_
//0:ok, -1:error
int getUSBLANMacAddr( char *p )
{
	unsigned char *hwaddr;
	struct ifreq ifr;	
	strcpy(ifr.ifr_name, USBETHIF);
	do_ioctl(SIOCGIFHWADDR, &ifr);
	hwaddr = (unsigned char *)ifr.ifr_hwaddr.sa_data;
//	printf("The result of SIOCGIFHWADDR is type %d  "
//	       "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x.\n",
//	       ifr.ifr_hwaddr.sa_family, hwaddr[0], hwaddr[1],
//	       hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);
	memcpy( p, hwaddr, 6 );
	return 0;	
}

//0:high, 1:full, 2:low
int getUSBLANRate( void )
{
	struct ifreq ifr;	
	strcpy(ifr.ifr_name, USBETHIF);
	do_ioctl( SIOCUSBRATE, &ifr );
//	fprintf( stderr, "getUSBLANRate=%d\n", *(int*)(&ifr.ifr_ifindex) ); //ifr_ifru.ifru_ivalue );
	return ifr.ifr_ifindex;//ifr_ifru.ifru_ivalue;	
}

//0:up, 1:nolink
int getUSBLANStatus(void )
{
	struct ifreq ifr;	
	strcpy(ifr.ifr_name, USBETHIF);
	do_ioctl( SIOCUSBSTAT, &ifr );
//	fprintf( stderr, "getUSBLANStatus=%d\n", *(int*)(&ifr.ifr_ifindex) ); //ifr_ifru.ifru_ivalue );
	return ifr.ifr_ifindex; //ifr_ifru.ifru_ivalue;	
}
#endif


#ifndef ZTE_GENERAL_ROUTER_SC
const char errGetEntry[] = "Get table entry error!";
#else
const char errGetEntry[] = "获取MIB 链entry 出错!";
#endif
static char adslDevice[] = "/dev/adsl0";

static FILE* adslFp = NULL;

const char *ppp_auth[] = {
	"AUTO", "PAP", "CHAP"
};

const char *dhcp_mode[] = {
	"None", "DHCP Relay", "DHCP Server"
};

#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
const char *n0to7[] = {
	"n/a", "0", "1", "2", "3", "4", "5", "6", "7"
};

/*
const char *prioLevel[] = {
	"High", "Medium", "Low"
};
*/
// Note: size of prioLevel depends on the IPQOS_NUM_PKT_PRIO
const char *prioLevel[] = {
	"p0", "p1", "p2", "p3", "p4", "p5", "p6", "p7"
};

//alex_huang
#ifdef CONFIG_8021P_PRIO
const char *set1ptable[]={"set1ptbl0","set1ptbl1","set1ptbl2","set1ptbl3","set1ptbl4","set1ptbl5","set1ptbl6","set1ptbl7"};
#ifdef NEW_IP_QOS_SUPPORT
const char *setpredtable[]={"setpredtbl0", "setpredtbl1", "setpredtbl2", "setpredtbl3", "setpredtbl4", "setpredtbl5", "setpredtbl6", "setpredtbl7"};
#endif
#endif
// priority mapping of packet priority against priority queue
// ex, priomap[3] == 2 means packet priority 3 is mapping to priority queue_2 by default
//const int priomap[8] = {3, 3, 2, 2, 2, 1, 1, 1};
//const int priomap[8] = {4, 4, 3, 3, 2, 2, 1, 1};
const int priomap[8] = {3, 4, 4, 3, 2, 2, 1, 1}; //cathy
const char *ipTos[] = {
	"n/a", "Normal Service", "Minimize Cost", "Maximize Reliability",
	"Maximize Throughput", "Minimize Delay"
};
#endif

#ifdef WLAN_SUPPORT
const char *wlan_band[] = {
	0, "2.4 GHz (B)", "2.4 GHz (G)", "2.4 GHz (B+G)", 0
#ifdef CONFIG_USB_RTL8192SU_SOFTAP
	, 0, 0, 0, "2.4 GHz (N)", 0, "2.4 GHz (G+N)", "2.4 GHz (B+G+N)", 0
#endif
};

const char *wlan_mode[] = {
	//"AP", "Client", "AP+WDS"
	"AP", "Client", "WDS", "AP+WDS"
};

const char *wlan_rate[] = {
	"1M", "2M", "5.5M", "11M", "6M", "9M", "12M", "18M", "24M", "36M", "48M", "54M"
#ifdef CONFIG_USB_RTL8192SU_SOFTAP
	, "MCS0", "MCS1", "MCS2", "MCS3", "MCS4", "MCS5", "MCS6", "MCS7", "MCS8", "MCS9", "MCS10", "MCS11", "MCS12", "MCS13", "MCS14", "MCS15" 
#endif
};

const char *wlan_auth[] = {
	"Open", "Shared", "Auto"
};

//modified  by xl_yue
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
const char *wlan_preamble[] = {
	"Long", "Short", "Auto"
};
#else
const char *wlan_preamble[] = {
	"Long", "Short"
};
#endif

const char *wlan_encrypt[] = {
#ifdef ENABLE_WPAAES_WPA2TKIP
	"None", "WEP", "WPA (TKIP)",  "WPA (AES)", "WPA2 (AES)", "WPA2 (TKIP)", "WPA2 Mixed"
#else
	"None", "WEP", "WPA (TKIP)", "WPA2 (AES)", "WPA2 Mixed"
#endif
};

const char *wlan_pskfmt[] = {
	"Passphrase", "Hex"
};

const char *wlan_wepkeylen[] = {
	"Disable", "64-bit", "128-bit"
};

const char *wlan_wepkeyfmt[] = {
	"ASCII", "Hex"
};
static int useAuth_wlan0=0;
static int wlan_num=0;
static char para_iwctrl[10][20];


#ifdef WLAN_MBSSID
void MBSSID_GetRootEntry(MIB_CE_MBSSIB_T *Entry) {
	Entry->idx = 0;

	mib_get(MIB_WLAN_ENCRYPT, &Entry->encrypt);
	mib_get(MIB_WLAN_ENABLE_1X, &Entry->enable1X);
	mib_get(MIB_WLAN_WEP, &Entry->wep);
	mib_get(MIB_WLAN_WPA_AUTH, &Entry->wpaAuth);
	mib_get(MIB_WLAN_WPA_PSK_FORMAT, &Entry->wpaPSKFormat);
	mib_get(MIB_WLAN_WPA_PSK, Entry->wpaPSK);
	mib_get(MIB_WLAN_RS_PORT, &Entry->rsPort);
	mib_get(MIB_WLAN_RS_IP, Entry->rsIpAddr);

	mib_get(MIB_WLAN_RS_PASSWORD, Entry->rsPassword);
	mib_get(MIB_WLAN_DISABLED, &Entry->wlanDisabled);
	mib_get(MIB_WLAN_SSID, Entry->ssid);
	mib_get(MIB_WLAN_AUTH_TYPE, &Entry->authType);
//added by xl_yue
#ifdef ENABLE_WPAAES_WPA2TKIP
	mib_get( MIB_WLAN_WPA_CIPHER_SUITE, &Entry->unicastCipher);
	mib_get( MIB_WLAN_WPA2_CIPHER_SUITE, &Entry->wpa2UnicastCipher);
#endif

}

void MBSSID_GetRootEntryWEP(MIB_CE_MBSSIB_WEP_T *Entry) {	
	mib_get(MIB_WLAN_WEP64_KEY1, Entry->wep64Key1);
	mib_get(MIB_WLAN_WEP64_KEY2, Entry->wep64Key2);
	mib_get(MIB_WLAN_WEP64_KEY3, Entry->wep64Key3);
	mib_get(MIB_WLAN_WEP64_KEY4, Entry->wep64Key4);

	mib_get(MIB_WLAN_WEP128_KEY1, Entry->wep128Key1);
	mib_get(MIB_WLAN_WEP128_KEY2, Entry->wep128Key2);
	mib_get(MIB_WLAN_WEP128_KEY3, Entry->wep128Key3);
	mib_get(MIB_WLAN_WEP128_KEY4, Entry->wep128Key4);

	mib_get(MIB_WLAN_WEP_KEY_TYPE, &Entry->wepKeyType);
	mib_get(MIB_WLAN_WEP_DEFAULT_KEY, &Entry->wepDefaultKey);
}

#endif



#endif	// of WLAN_SUPPORT

#ifdef CONFIG_EXT_SWITCH
#ifdef CONFIG_USBCLIENT
// mapping of virtual port to the board-level port number
#if defined(ZTE_GENERAL_ROUTER_SC) || defined(ZTE_531B_BRIDGE_SC)
const int virt2user[] = {
	1, 2, 3, 4, 5
};
#else
const int virt2user[] = {
	5, 4, 3, 2, 1
};
#endif
#else
// mapping of virtual port to the board-level port number
#if defined(ZTE_GENERAL_ROUTER_SC) || defined(ZTE_531B_BRIDGE_SC)
const int virt2user[] = {
	1, 2, 3, 4
};
#else
const int virt2user[] = {
	4, 3, 2, 1
};
#endif
#endif
#endif

// Timer for auto search PVC
#if defined(AUTO_PVC_SEARCH_TR068_OAMPING) || defined(AUTO_PVC_SEARCH_PURE_OAMPING) || defined(AUTO_PVC_SEARCH_AUTOHUNT)
int autoHunt_found = 0;
int fin_AutoSearchPVC = 0;
#endif

// Mason Yu
#ifdef PORT_FORWARD_ADVANCE
const char *PFW_Gategory[] = {"VPN", "Game"};
const char *PFW_Rule[] = {"PPTP", "L2TP"};
#endif

//#ifdef _PRMT_TR143_
//keep the same order with ITF_T
//if want to convert lan device name to br0, call LANDEVNAME2BR0() macro
char *strItf[]=
{
	"",		//ITF_ALL
	"",		//ITF_WAN
	"br0",		//ITF_LAN

	"eth0",		//ITF_ETH0
	"eth0_sw0",	//ITF_ETH0_SW0
	"eth0_sw1",	//ITF_ETH0_SW1
	"eth0_sw2",	//ITF_ETH0_SW2
	"eth0_sw3",	//ITF_ETH0_SW3

	"wlan0",	//ITF_WLAN0
	"wlan0-vap0",	//ITF_WLAN0_VAP0
	"wlan0-vap1",	//ITF_WLAN0_VAP1
	"wlan0-vap2",	//ITF_WLAN0_VAP2
	"wlan0-vap3",	//ITF_WLAN0_VAP3

	"usb0",		//ITF_USB0

	""		//ITF_END
};

int IfName2ItfId( char *s )
{
	int i;
	if( !s || s[0]==0 ) return ITF_ALL;
	if( (strncmp(s, "ppp", 3)==0) || (strncmp(s, "vc", 2)==0) )
		return ITF_WAN;
	
	for( i=0;i<ITF_END;i++ )
	{
		if( strcmp( strItf[i],s )==0 ) return i;
	}
	
	return -1;
}
//#endif //_PRMT_TR143_

char *ifGetName(int ifindex, char *buffer, unsigned int len);

static int do_ioctl(unsigned int cmd, struct ifreq *ifr)
{
	int skfd, ret;
	
	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return (-1);
	}
	
	ret = ioctl(skfd, cmd, ifr);
	close(skfd);
	return ret;
}

/*
 * Get Interface Addr (MAC, IP, Mask)
 */
int getInAddr( char *interface, ADDR_T type, void *pAddr )
{
	struct ifreq ifr;
	int found=0;
	struct sockaddr_in *addr;
	
	strcpy(ifr.ifr_name, interface);
	if (do_ioctl(SIOCGIFFLAGS, &ifr) < 0)
		return (0);
	
	if (type == HW_ADDR) {
		if (do_ioctl(SIOCGIFHWADDR, &ifr) >= 0) {
			memcpy(pAddr, &ifr.ifr_hwaddr, sizeof(struct sockaddr));
			found = 1;
		}
	}
	else if (type == IP_ADDR) {
		if (do_ioctl(SIOCGIFADDR, &ifr) == 0) {
			addr = ((struct sockaddr_in *)&ifr.ifr_addr);
			*((struct in_addr *)pAddr) = *((struct in_addr *)&addr->sin_addr);
			found = 1;
		}
	}
	else if (type == DST_IP_ADDR) {
		if (do_ioctl(SIOCGIFDSTADDR, &ifr) == 0) {
			addr = ((struct sockaddr_in *)&ifr.ifr_addr);
			*((struct in_addr *)pAddr) = *((struct in_addr *)&addr->sin_addr);
			found = 1;
		}
	}
	else if (type == SUBNET_MASK) {
		if (do_ioctl(SIOCGIFNETMASK, &ifr) >= 0) {
			addr = ((struct sockaddr_in *)&ifr.ifr_addr);
			*((struct in_addr *)pAddr) = *((struct in_addr *)&addr->sin_addr);
			found = 1;
		}
	}
	return found;
}

int getInFlags( char *interface, int *flags )
{
	struct ifreq ifr;
	int found=0;

#ifdef EMBED
	strcpy(ifr.ifr_name, interface);
	
	if (do_ioctl(SIOCGIFFLAGS, &ifr) == 0) {
		if (flags)
			*flags = ifr.ifr_flags;
		found = 1;
	}
#endif
	return found;
}

int setInFlags( char *interface, int flags )
{
	struct ifreq ifr;
	int ret=0;

#ifdef EMBED
	strcpy(ifr.ifr_name, interface);
	ifr.ifr_flags = flags;
	
	if (do_ioctl(SIOCSIFFLAGS, &ifr) == 0)
		ret = 1;
#endif
	return ret;
}

int INET_resolve(char *name, struct sockaddr *sa)
{
	struct sockaddr_in *s_in = (struct sockaddr_in *)sa;

	s_in->sin_family = AF_INET;
	s_in->sin_port = 0;

	/* Default is special, meaning 0.0.0.0. */
	if (strcmp(name, "default")==0) {
		s_in->sin_addr.s_addr = INADDR_ANY;
		return 1;
	}
	/* Look to see if it's a dotted quad. */
	if (inet_aton(name, &s_in->sin_addr)) {
		return 0;
	}
	/* guess not.. */
	return -1;
}

enum { ROUTE_ADD, ROUTE_DEL };
static int route_modify(struct rtentry *rt, int action) 
{
	int skfd;
	
	if (rt==0)
		return -1;
	
	rt->rt_flags = RTF_UP;
	if (((struct sockaddr_in *)(&rt->rt_gateway))->sin_addr.s_addr)
		rt->rt_flags |= RTF_GATEWAY;
	
	/* Create a socket to the INET kernel. */
	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return -1;
	}
	
	// Added by Mason Yu. According the netmask, we input the correct dst address.
	((struct sockaddr_in *)(&rt->rt_dst))->sin_addr.s_addr = 
	(((struct sockaddr_in *)(&rt->rt_dst))->sin_addr.s_addr & ((struct sockaddr_in *)(&rt->rt_genmask))->sin_addr.s_addr );

	if (action == ROUTE_ADD) {
	/* Tell the kernel to accept this route. */
		if (ioctl(skfd, SIOCADDRT, rt) < 0) {
			perror("SIOCADDRT");
			close(skfd);
			return -1;
		}
	} else {
		if (ioctl(skfd, SIOCDELRT, rt) < 0) {
			perror("SIOCDELRT");
			close(skfd);
			return -1;
		}
	}
	
	/* Close the socket. */
	(void) close(skfd);
	return 0;

}

// update corresponding field of rtentry from MIB_CE_IP_ROUTE_T
static void updateRtEntry(MIB_CE_IP_ROUTE_T *pSrc, struct rtentry *pDst) {
	struct sockaddr_in *s_in;

	//pDst->rt_flags = RTF_UP;
	s_in = (struct sockaddr_in *)&pDst->rt_dst;
	s_in->sin_family = AF_INET;
	s_in->sin_port = 0;
	s_in->sin_addr = *(struct in_addr *)pSrc->destID;
					
	s_in = (struct sockaddr_in *)&pDst->rt_genmask;
	s_in->sin_family = AF_INET;
	s_in->sin_port = 0;
	s_in->sin_addr = *(struct in_addr *)pSrc->netMask;

	//if (pSrc->nextHop[0]&&pSrc->nextHop[1]&&pSrc->nextHop[2]&&pSrc->nextHop[3]) {				
	if (pSrc->nextHop[0] || pSrc->nextHop[1] || pSrc->nextHop[2] ||pSrc->nextHop[3]) {
		s_in = (struct sockaddr_in *)&pDst->rt_gateway;	
		s_in->sin_family = AF_INET;
		s_in->sin_port = 0;
		s_in->sin_addr = *(struct in_addr *)pSrc->nextHop;
	}
}

/*del=>  1: delete the route entry, 
         0: add the route entry(skip ppp part), 
        -1: add the route entry*/
void route_cfg_modify(MIB_CE_IP_ROUTE_T *pRoute, int del) {
//int route_cfg_modify(MIB_CE_IP_ROUTE_T *pRoute, int del) {
	struct rtentry rt;
	char ifname[16];

	if( pRoute==NULL ) return;
	if(!pRoute->Enable) return; 

	memset(&rt, 0, sizeof(rt));
	updateRtEntry(pRoute, &rt);
	rt.rt_dev = ifGetName(pRoute->ifIndex, ifname, sizeof(ifname));
	if (pRoute->FWMetric > -1)
		rt.rt_metric = pRoute->FWMetric + 1;


	if (del>0) {
		INET_delroute(&rt);
		return;
//		return -1;
	}
	
	INET_addroute(&rt);
//	return INET_addroute(&rt);

}

void route_ppp_ifup(unsigned long pppGW) {
	unsigned int entryNum, i;
	char ifname0[16];//, *ifname;
	MIB_CE_IP_ROUTE_T Entry;
	struct rtentry rt;			

	entryNum = mib_chain_total(MIB_IP_ROUTE_TBL);
	
	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_IP_ROUTE_TBL, i, (void *)&Entry))
		{
			continue;
		}
		if( !Entry.Enable ) continue;

		memset(&rt, 0, sizeof(rt));

		rt.rt_dev = ifGetName(Entry.ifIndex, ifname0, sizeof(ifname0));

		if (rt.rt_dev && !strncmp(rt.rt_dev, "ppp", 3)) {
			updateRtEntry(&Entry, &rt);
			rt.rt_metric = Entry.FWMetric + 1;		
			route_modify(&rt, ROUTE_ADD);
		}
		else if (Entry.ifIndex == 0xff) {	// Interface "any"
			struct in_addr *addr;
			addr = (struct in_addr *)&Entry.nextHop;
			if (addr->s_addr == pppGW) {
				updateRtEntry(&Entry, &rt);
				rt.rt_metric = Entry.FWMetric + 1;		
				route_modify(&rt, ROUTE_ADD);
			}
		}
	}
#ifdef DNS_BIND_PVC_SUPPORT
	DnsBindPvcRoute(DELDNSROUTE);
	DnsBindPvcRoute(ADDDNSROUTE);
#endif
  
}
/*
*	Del a route
*/
int INET_delroute(struct rtentry *rt)
{
	return route_modify(rt, ROUTE_DEL);
}
/*
 *	Add a route
 */
int INET_addroute(struct rtentry *rt)
{
	return route_modify(rt, ROUTE_ADD);
#if 0
	int skfd;
	
	if (rt==0)
		return -1;
	
	rt->rt_flags = (RTF_UP | RTF_GATEWAY);
	
	/* Create a socket to the INET kernel. */
	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return -1;
	}
	
	// Added by Mason Yu. According the netmask, we input the correct dst address.
	((struct sockaddr_in *)(&rt->rt_dst))->sin_addr.s_addr = 
	(((struct sockaddr_in *)(&rt->rt_dst))->sin_addr.s_addr & ((struct sockaddr_in *)(&rt->rt_genmask))->sin_addr.s_addr );
	
	// Mason Yu
	//printf("&rt->rt_dst=%x  &rt->rt_genmask=%x  &rt->rt_gateway)=%x\n", 
	//        ((struct sockaddr_in *)(&rt->rt_dst))->sin_addr.s_addr, 
	//        ((struct sockaddr_in *)(&rt->rt_genmask))->sin_addr.s_addr, 
	//        ((struct sockaddr_in *)(&rt->rt_gateway))->sin_addr.s_addr); 
	
	/* Tell the kernel to accept this route. */
	if (ioctl(skfd, SIOCADDRT, rt) < 0) {
		perror("SIOCADDRT");
		close(skfd);
		return -1;
	}
	
	/* Close the socket. */
	(void) close(skfd);
	return 0;
#endif
}


/*
* Convert ifIndex to system interface name, e.g. eth0,vc0...
*/
char *ifGetName(int ifindex, char *buffer, unsigned int len) {
	if ( ifindex == 0xff )
	{
		return 0;
	}else 
	if(  (ifindex & 0xf0)==0xf0  )
	{
		snprintf( buffer, len,  "vc%u", VC_INDEX(ifindex) );	

	}else{
		snprintf( buffer, len,  "ppp%u", PPP_INDEX(ifindex) );
		
	}
	return buffer;
}
#if !defined( CONFIG_EXT_SWITCH) && defined(CONFIG_RTL867X_COMBO_PORTMAPPING)
int setVlan(struct ifreq *ifr)
{
	int ret=0;
	
#ifdef EMBED
	if (do_ioctl(SIOCS8305VLAN, ifr) == 0)
		ret = 1;
	
#endif
	return ret;
}
#ifdef CONFIG_USB_ETH
static void setup_usb_group_member(int mbr)
{
	struct ifreq ifr;	
	printf( "setup_usb_group_member> mbr=0x%x\n", mbr );
	strcpy(ifr.ifr_name, USBETHIF);
	ifr.ifr_ifru.ifru_ivalue = mbr;
	do_ioctl( SIOCSITFGROUP, &ifr );
	return;
}
#endif
void setgroup(char *list, int grpnum)
{
	int itfid, itfdomain;
	int i, num;
	char *arg0, *token;
	
	arg0=list;
	while ((token=strtok(arg0,","))!=NULL) {		
		itfid = atoi(token);		
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		setitfPrio(itfid, priority );
#endif		
		itfdomain = IF_DOMAIN(itfid);
		itfid = itfid&0x0ff;		

		if (itfdomain == DOMAIN_ELAN) {
			char mygroup;
			mygroup = (char)grpnum;
			if (itfid == 0)
				mib_set(MIB_ETH_ITF_GROUP, (void *)&mygroup);
		}
#ifdef WLAN_SUPPORT
		else if (itfdomain == DOMAIN_WLAN) {
			char mygroup;
			
			mygroup = (char)grpnum;			
			if (itfid == 0)
				mib_set(MIB_WLAN_ITF_GROUP, (void *)&mygroup);
				
			// Added by Mason Yu			
#ifdef WLAN_MBSSID
			for ( i=1; i<5; i++) {
				if (itfid == i) {// vap0								
					mib_set(MIB_WLAN_VAP0_ITF_GROUP+i-1, (void *)&mygroup);
				}
			}
			
#endif
		}
#endif		

#ifdef CONFIG_USB_ETH
		else if (itfdomain == DOMAIN_ULAN) {
			char mygroup;
			mygroup = (char)grpnum;
			if (itfid == 0)
				mib_set(MIB_USBETH_ITF_GROUP, (void *)&mygroup);
		}		
#endif

		else if (itfdomain == DOMAIN_WAN) {
			MIB_CE_ATM_VC_T Entry;
			
			num = mib_chain_total(MIB_ATM_VC_TBL);
			for (i=0; i<num; i++) {
				if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
					return;
				if (Entry.enable && Entry.ifIndex==itfid) {
					Entry.itfGroup = grpnum;
					mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, i);
				}
			}
		}
		arg0=0;
	}
}

static void setup_combo_eth_member(int port, int mbr)
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
#ifdef CONFIG_RE8306
		ifvl.member=(mbr & 0x7f);	
#else
		ifvl.member=(mbr & 0x3f);
#endif               	
#else
#ifdef CONFIG_RE8306
//#ifdef CONFIG_USBCLIENT
		ifvl.member=(mbr & 0x7ff);
#else
		ifvl.member=(mbr & 0x3ff);
#endif		
#endif

#ifdef CONFIG_USB_ETH
		ifvl.member|=( mbr&(1<<IFUSBETH_SHIFT) );
#endif //CONFIG_USB_ETH

		setVlan(&ifr);
		TRACE(STA_SCRIPT, "RTL8305 VLAN: vlanIdx=%d, vid=%d, member=0x%.2x\n", ifvl.vlanIdx, ifvl.vid, ifvl.member);
	}
}

void __dev_setupPortMapping(int flag)
{
	struct ifreq ifr;
	struct ifvlan ifvl;
	
	strcpy(ifr.ifr_name, "eth0");
	ifr.ifr_data = (char *)&ifvl;
	ifvl.cmd=VLAN_SETPORTMAPPING;
	ifvl.enable=flag;
	setVlan(&ifr);
	//printf("Port Mapping: %s\n", flag?"enabled":"disabled");
}
/*
 * Setup per-vc interface group member(bit mapped)
 * The bitmap for interface should follow this convention commonly shared
 * by sar driver (sar_send())
 * OLD Bit-map:  (bit6) | bit5  |  bit4  |  bit3  |  bit2  |  bit1  |  bit0
 *               (usb0) | wlan  | device |  lan3  |  lan2  |  lan1  |  lan0
 
 * New Bit-map:  (bit10) | bit9 | bit8 | bit7 | bit6 | bit5  |  bit4  |  bit3  |  bit2  |  bit1  |  bit0
 *               (usb0)  | vap3 | vap2 | vap1 | vap0 | wlan  | device |  lan3  |  lan2  |  lan1  |  lan0
 if usb add 
    (bit11) | bit10 | bit9  |  bit8  | bit7 | bit6   | bit5   |   bit4   |  bit3  |  bit2  |  bit1  |  bit0
 *  (usb0)  | vap3  | vap2  | vap1   | vap0 | wlan   | device | lan4(usb)|  lan3  |  lan2  |  lan1  |  lan0
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
#ifdef CONFIG_USB_ETH
	char usbethgrp,ethgrp;
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
		vcIdx = VC_INDEX(pvcEntry.ifIndex);
		{
			mib_get(MIB_ETH_ITF_GROUP, (void *)&ethgrp);
			if (ethgrp == pvcEntry.itfGroup) {
				// this lan port shares the same group with pvc, add as pvc's member
				if (vcIdx < MAX_VC_NUM) {
					mbr[vcIdx] |= (1);
					// get rid of local dhcp server if not belonging to default group
					// Commented by Mason Yu
					#if 0
					if (usbethgrp != 0) {
						// iptables -A inacc -i $LAN_IF -p UDP --dport 67 -j DROP
						va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_INACC,
						(char *)ARG_I, (char *)USBETHIF, "-p", (char *)ARG_UDP,
						(char *)FW_DPORT, (char *)PORT_DHCP, "-j", (char *)FW_DROP);
					}
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
#ifdef CONFIG_USB_ETH
		{//check for usb eth
			mib_get(MIB_USBETH_ITF_GROUP, (void *)&usbethgrp);
			if (usbethgrp == pvcEntry.itfGroup) {
				// this lan port shares the same group with pvc, add as pvc's member
				if (vcIdx < MAX_VC_NUM) {
					mbr[vcIdx] |= (1<<IFUSBETH_SHIFT);
					// get rid of local dhcp server if not belonging to default group
					// Commented by Mason Yu
					#if 0
					if (usbethgrp != 0) {
						// iptables -A inacc -i $LAN_IF -p UDP --dport 67 -j DROP
						va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_INACC,
						(char *)ARG_I, (char *)USBETHIF, "-p", (char *)ARG_UDP,
						(char *)FW_DPORT, (char *)PORT_DHCP, "-j", (char *)FW_DROP);
					}
					#endif
				}
			}
		}
#endif //CONFIG_USB_ETH


		// device is mandatory
#ifdef CONFIG_USBCLIENT		
		mbr[vcIdx] |= 0x20;
#else
		mbr[vcIdx] |= 0x10;
#endif	

		/* set membership into vc */
		snprintf(wanif, 5, "vc%d", vcIdx);
		// mpoactl set vc0 group 1 member 3
		snprintf(msg.data, BUF_SIZE,
			"mpoactl set %s group 1 member %d",
			wanif, mbr[vcIdx]);
		TRACE(STA_SCRIPT, "%s\n", msg.data);
		write_to_mpoad(&msg);
		/* set membership into LAN ports */
			// Is this port a member ?
			if (mbr[vcIdx]&(1)) {
				setup_combo_eth_member(0, mbr[vcIdx]);
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
#ifdef CONFIG_USB_ETH
		/* set membership into usb0 */
		if (mbr[vcIdx]&(1<<IFUSBETH_SHIFT)){
			setup_usb_group_member( mbr[vcIdx] );
		}
#endif //CONFIG_USB_ETH
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

		mib_get(MIB_ETH_ITF_GROUP, (void *)&ethgrp);
		if(ethgrp == i)
			member |= 1;
#if 0
		num = mib_chain_total(MIB_SW_PORT_TBL);
		for (k=0; k<num; k++) {
			if (!mib_chain_get(MIB_SW_PORT_TBL, k, (void *)&Port))
				continue;
			if (Port.itfGroup == i)
				member |= (1<<k);
		}
#endif		
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

#ifdef CONFIG_USB_ETH
		mib_get(MIB_USBETH_ITF_GROUP, (void *)&usbethgrp);
		if (usbethgrp == i)
			member |= (1<<IFUSBETH_SHIFT);
#endif //CONFIG_USB_ETH
		
		// device is mandatory
#ifdef CONFIG_USBCLIENT
             member |= 0x20;
		
#else
             member |= 0x10; 
#endif
		mib_get(MIB_ETH_ITF_GROUP, (void *)&ethgrp);
		if(ethgrp == i)
			setup_combo_eth_member(0, member);
#if 0
	/* set membership into LAN ports */
		for (k=0; k<num; k++) {
			if (!mib_chain_get(MIB_SW_PORT_TBL, k, (void *)&Port))
				continue;
			if (Port.itfGroup == i)
				setup_combo_eth_member(k, member);
		}
#endif		
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
#ifdef CONFIG_USB_ETH
		/* set membership into usb0 */
		if (usbethgrp == i)
			setup_usb_group_member( member );
#endif //CONFIG_USB_ETH
	}

#if 0
	// set port 4 member to all
#ifdef CONFIG_USBCLIENT
       setup_8305_vlan_member(5, 0x3f);
#else
	setup_8305_vlan_member(4, 0x1f);
#endif
#endif
	
	// enable Port Mapping
	__dev_setupPortMapping(1);
	// enable 8305 vlan
	//setup_8305_vlan_enable();
	
	printf("Port Mapping: Ethernet to pvc mapping started\n");
}

/*------------------------------------------------------------------
 * Get a list of interface info. (itfInfo) of group.
 * where,
 * info: a list of interface info entries
 * len: max length of the info list
 * grpnum: group id
 *-----------------------------------------------------------------*/
int get_group_ifinfo(struct itfInfo *info, int len, int grpnum)
{
	unsigned int swNum, vcNum;
	int i, num=0;
	char mygroup;
	char strlan[]="LAN0", strPPP[]="ppp0", strvc[]="vc0";
	MIB_CE_SW_PORT_T Entry;
	MIB_CE_ATM_VC_T pvcEntry;
#ifdef CTC_WAN_NAME
	char wanname[100];	
#endif
	//eth0
	mib_get(MIB_ETH_ITF_GROUP, (void *)&mygroup);
	if (mygroup == grpnum) {
		info[num].ifdomain = DOMAIN_ELAN;
		info[num].ifid=0;
		strcpy(info[num].name, (char *)ELANIF);
		num++;
	}

#ifdef WLAN_SUPPORT
	// wlan0
	mib_get(MIB_WLAN_ITF_GROUP, (void *)&mygroup);
	if (mygroup == grpnum) {
		info[num].ifdomain = DOMAIN_WLAN;
		info[num].ifid=0;
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		sprintf(info[num].name, "SSID1(root)");
#else
		strncpy(info[num].name, (char *)WLANIF, 8);
#endif
		num++;
	}
#endif
	
#ifdef WLAN_MBSSID
	for (i=1; i<5; i++) {
		mib_get(MIB_WLAN_VAP0_ITF_GROUP+i-1, (void *)&mygroup);	
		if (mygroup == grpnum) {
			info[num].ifdomain = DOMAIN_WLAN;
			info[num].ifid=i;
			//strncpy(info[num].name, "vap0", 8);
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
			sprintf(info[num].name, "SSID%d", i+1);
#else
			sprintf(info[num].name, "vap%d", i-1);
#endif
			num++;
		}
	}	

#endif	

#ifdef CONFIG_USB_ETH
	// usb0
	mib_get(MIB_USBETH_ITF_GROUP, (void *)&mygroup);
	if (mygroup == grpnum) {
		info[num].ifdomain = DOMAIN_ULAN;
		info[num].ifid=0;
		strcpy(info[num].name, (char *)USBETHIF);
		num++;
	}
#endif	
	// vc
	vcNum = mib_chain_total(MIB_ATM_VC_TBL);
	
	for (i=0; i<vcNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&pvcEntry))
		{
  			//websError(wp, 400, T("Get chain record error!\n"));
  			printf("Get chain record error!\n");
			return -1;
		}
		
		if (pvcEntry.enable == 0 || pvcEntry.itfGroup!=grpnum)
			continue;

#ifdef CTC_WAN_NAME
		getWanName(&pvcEntry, wanname);
		info[num].ifdomain = DOMAIN_WAN;
		info[num].ifid=pvcEntry.ifIndex;
		strncpy(info[num].name, wanname, 30);
		num++;
#else
		if (PPP_INDEX(pvcEntry.ifIndex) != 0x0f)
		{	// PPP interface
			strPPP[3] = '0'+PPP_INDEX(pvcEntry.ifIndex);
			info[num].ifdomain = DOMAIN_WAN;
			info[num].ifid=pvcEntry.ifIndex;
			strncpy(info[num].name, strPPP, 8);
			num++;
		}
		else
		{	// vc interface
			strvc[2] = '0'+VC_INDEX(pvcEntry.ifIndex);
			info[num].ifdomain = DOMAIN_WAN;
			info[num].ifid=pvcEntry.ifIndex;
			strncpy(info[num].name, strvc, 8);
			num++;
		}
#endif		
		if (num > len)
			break;
	}
	
	return num;
}

#endif

#ifdef CONFIG_EXT_SWITCH
//-----------------------------------------------------------------
// register switch port 0 ~ 3 into system.
// This will fix the 4-ports vlan mapping, so that the user should
// not reconfig the switch vlan.
void __dev_register_swport()
{
#ifdef EMBED
	struct ifreq ifr;
	struct ifvlan ifvl;
	short mbr;
	int i;
	
	strcpy(ifr.ifr_name, "eth0");
	
	// set per-vlan id and member set
	// vlanIdx	VLAN	VID	MEMBER
	//	0	A	1	port 0,1,2,3,4
	//	1	B	2	port 0,1,2,3,4
	//	2	C	3	port 0,1,2,3,4
	//	3	D	4	port 0,1,2,3,4
	mbr = 0x10;	// set port 4 as member
	for (i=0; i<SW_PORT_NUM; i++) {
		ifvl.cmd=VLAN_SETINFO;
		ifvl.vlanIdx=i;
		// VID 0 is used to identify priority frames and VID 4095(0xFFF) is reserved
		ifvl.vid=i+1;
		//ifvl.member=mbr | (1<<i);
//		ifvl.member=0x1f; // member ports: 0,1,2,3,4
#ifdef CONFIG_USBCLIENT		
		ifvl.member=0x3f; //member ports :0,1,2,3,4,5
#else
		ifvl.member=0x1f; // member ports: 0,1,2,3,4
#endif		
		ifr.ifr_data = (char *)&ifvl;
		setVlan(&ifr);
	}
	
	// set port vlan mapping
	// port		vlanIdx
	//	0	0
	//	1	1
	//	2	2
	//	3	3
	for (i=0; i<SW_PORT_NUM; i++) {
		// set sw-port 0~3 vlan idex (vlan id) to let system tell the sw-ports
		ifvl.cmd=VLAN_SETPVIDX;
		ifvl.port=i;
		ifvl.vlanIdx=i;
		ifr.ifr_data = (char *)&ifvl;
		setVlan(&ifr);
	}
	
	// Setup port 4 (to 867x NIC) tx tag policy to be tag_replace.
	// The vlan tag would be replaced according to its source port
	// vlanIdx setting when output from port 4
	ifvl.cmd=VLAN_SETTXTAG;
#ifdef CONFIG_USBCLIENT	
	ifvl.port=5;
#else
        ifvl.port=4;
#endif	
	ifvl.txtag=TAG_REPLACE;
	ifr.ifr_data = (char *)&ifvl;
	setVlan(&ifr);
	
	__dev_setupVirtualPort(1);
#endif
}

// enable/disable IGMP snooping
void __dev_setupIGMPSnoop(int flag)
{
	struct ifreq ifr;
	struct ifvlan ifvl;
	
	strcpy(ifr.ifr_name, "eth0");
	ifr.ifr_data = (char *)&ifvl;
	ifvl.cmd=VLAN_SETIGMPSNOOP;
	ifvl.enable=flag;
	setVlan(&ifr);
	printf("IGMP Snooping: %s\n", flag?"enabled":"disabled");
}
	
// enable/disable IGMP snooping
void __dev_setupPortMapping(int flag)
{
#ifdef ITF_GROUP
	struct ifreq ifr;
	struct ifvlan ifvl;
	
	strcpy(ifr.ifr_name, "eth0");
	ifr.ifr_data = (char *)&ifvl;
	ifvl.cmd=VLAN_SETPORTMAPPING;
	ifvl.enable=flag;
	setVlan(&ifr);
	//printf("Port Mapping: %s\n", flag?"enabled":"disabled");
#endif
}
	
// enable/disable LAN virtual port
void __dev_setupVirtualPort(int flag)
{
	struct ifreq ifr;
	struct ifvlan ifvl;
	
	strcpy(ifr.ifr_name, "eth0");
	ifr.ifr_data = (char *)&ifvl;
	ifvl.cmd=VLAN_VIRTUAL_PORT;
	ifvl.enable=flag;
	setVlan(&ifr);
}
#ifdef CONFIG_IGMP_FORBID
void __dev_igmp_forbid(int flag)
{
     struct ifreq ifr;
	struct ifvlan ifvl;
	
	strcpy(ifr.ifr_name, "eth0");
	ifr.ifr_data = (char *)&ifvl;
	ifvl.cmd=IGMP_FORBID;
	ifvl.enable=flag;
	setVlan(&ifr);
}
#endif

// Kaohj
#ifdef VLAN_GROUP
// enable/disable VLAN-Grouping
void __dev_setupVlanGrouping(int flag)
{
	struct ifreq ifr;
	struct ifvlan ifvl;
	
	strcpy(ifr.ifr_name, "eth0");
	ifr.ifr_data = (char *)&ifvl;
	ifvl.cmd=VLAN_SETVLANGROUPING;
	ifvl.enable=flag;
	setVlan(&ifr);
	printf("Port/VLAN mapping: %s\n", flag?"enabled":"disabled");
}
#endif	// of VLAN_GROUP
	
int setVlan(struct ifreq *ifr)
{
	int ret=0;
	
#ifdef EMBED
	if (do_ioctl(SIOCS8305VLAN, ifr) == 0)
		ret = 1;
	
#endif
	return ret;
}

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


// disable rtl-8305 VLAN
static void setup_8305_vlan_disable()
{
	struct ifreq ifr;
	struct ifvlan ifvl;
	
	strcpy(ifr.ifr_name, "eth0");
	ifr.ifr_data = (char *)&ifvl;
	// disable rtl-8305 VLAN
	ifvl.cmd=VLAN_ENABLE;
	ifvl.enable=0;
	setVlan(&ifr);
	TRACE(STA_SCRIPT, "RTL8305 VLAN disabled\n");
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

  New Bit-map for 8306:  bit 10 | bit9  | bit8   | bit7   | bit6    | bit5   |  bit4  |  bit3  |  bit2  |  bit1  |  bit0
 *                                  vap3  | vap2 | vap1 | vap0 | wlan  | device| lan4  |  lan3  |  lan2  |  lan1  |  lan0
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
#ifdef CONFIG_RE8306
		ifvl.member=(mbr & 0x7f);	
#else
		ifvl.member=(mbr & 0x3f);
#endif               	
#else
#ifdef CONFIG_RE8306
//#ifdef CONFIG_USBCLIENT
		ifvl.member=(mbr & 0x7ff);
#else
		ifvl.member=(mbr & 0x3ff);
#endif		
#endif

#ifdef CONFIG_USB_ETH
		ifvl.member|=( mbr&(1<<IFUSBETH_SHIFT) );
#endif //CONFIG_USB_ETH		
		
		setVlan(&ifr);
		TRACE(STA_SCRIPT, "RTL8305 VLAN: vlanIdx=%d, vid=%d, member=0x%.2x\n", ifvl.vlanIdx, ifvl.vid, ifvl.member);
	} else {
		// Added by Mason Yu
		// set the membership
		ifvl.vlanIdx=port;		
		// VID 0 is used to identify priority frames and VID 4095(0xFFF) is reserved
		ifvl.vid=port+1;		
		
#ifdef CONFIG_USBCLIENT
		ifvl.member=0x3f;
#else		
		ifvl.member=0x1f;		
#endif
		setVlan(&ifr);
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

#ifdef CONFIG_USB_ETH
static void setup_usb_group_member(int mbr)
{
	struct ifreq ifr;	
	//printf( "setup_usb_group_member> mbr=0x%x\n", mbr );
	strcpy(ifr.ifr_name, USBETHIF);
	ifr.ifr_ifru.ifru_ivalue = mbr;
	do_ioctl( SIOCSITFGROUP, &ifr );
	return;
}
#endif //CONFIG_USB_ETH

#ifdef ITF_GROUP
/*
 * Setup per-vc interface group member(bit mapped)
 * The bitmap for interface should follow this convention commonly shared
 * by sar driver (sar_send())
 * OLD Bit-map:  (bit6) | bit5  |  bit4  |  bit3  |  bit2  |  bit1  |  bit0
 *               (usb0) | wlan  | device |  lan3  |  lan2  |  lan1  |  lan0
 
 * New Bit-map:  (bit10) | bit9 | bit8 | bit7 | bit6 | bit5  |  bit4  |  bit3  |  bit2  |  bit1  |  bit0
 *               (usb0)  | vap3 | vap2 | vap1 | vap0 | wlan  | device |  lan3  |  lan2  |  lan1  |  lan0
 if usb add 
    (bit11) | bit10 | bit9  |  bit8  | bit7 | bit6   | bit5   |   bit4   |  bit3  |  bit2  |  bit1  |  bit0
 *  (usb0)  | vap3  | vap2  | vap1   | vap0 | wlan   | device | lan4(usb)|  lan3  |  lan2  |  lan1  |  lan0
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
#ifdef CONFIG_USB_ETH
	char usbethgrp;
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
#ifdef CONFIG_USB_ETH
		{//check for usb eth
			mib_get(MIB_USBETH_ITF_GROUP, (void *)&usbethgrp);
			if (usbethgrp == pvcEntry.itfGroup) {
				// this lan port shares the same group with pvc, add as pvc's member
				if (vcIdx < MAX_VC_NUM) {
					mbr[vcIdx] |= (1<<IFUSBETH_SHIFT);
					// get rid of local dhcp server if not belonging to default group
					// Commented by Mason Yu
					#if 0
					if (usbethgrp != 0) {
						// iptables -A inacc -i $LAN_IF -p UDP --dport 67 -j DROP
						va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_INACC,
						(char *)ARG_I, (char *)USBETHIF, "-p", (char *)ARG_UDP,
						(char *)FW_DPORT, (char *)PORT_DHCP, "-j", (char *)FW_DROP);
					}
					#endif
				}
			}
		}
#endif //CONFIG_USB_ETH


		// device is mandatory
#ifdef CONFIG_USBCLIENT		
		mbr[vcIdx] |= 0x20;
#else
		mbr[vcIdx] |= 0x10;
#endif	
				
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
#ifdef CONFIG_USB_ETH
		/* set membership into usb0 */
		if (mbr[vcIdx]&(1<<IFUSBETH_SHIFT))
			setup_usb_group_member( mbr[vcIdx] );
#endif //CONFIG_USB_ETH
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

#ifdef CONFIG_USB_ETH
		mib_get(MIB_USBETH_ITF_GROUP, (void *)&usbethgrp);
		if (usbethgrp == i)
			member |= (1<<IFUSBETH_SHIFT);
#endif //CONFIG_USB_ETH
		
		// device is mandatory
#ifdef CONFIG_USBCLIENT
             member |= 0x20;
		
#else
             member |= 0x10; 
#endif		
		
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
#ifdef CONFIG_USB_ETH
		/* set membership into usb0 */
		if (usbethgrp == i)
			setup_usb_group_member( member );
#endif //CONFIG_USB_ETH
	}
	// set port 4 member to all
#ifdef CONFIG_USBCLIENT
       setup_8305_vlan_member(5, 0x3f);
#else
	setup_8305_vlan_member(4, 0x1f);
#endif
	
	// enable Port Mapping
	__dev_setupPortMapping(1);
	// enable 8305 vlan
	setup_8305_vlan_enable();
	
	printf("Port Mapping(TW): Ethernet to pvc mapping started\n");
}


void setupEth2pvc_disable()
{		
	MIB_CE_ATM_VC_T pvcEntry;
	int i, k, total, num, member;	
	struct data_to_pass_st msg;
	char wanif[5];
	unsigned char vChar=0;
	
	// Mason Yu
	// If we disable Port Mapping, we should set all interface's membership to '0'.
	member=0;
	
	total = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<total; i++) {
		int vcIdx;
		
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&pvcEntry))
			continue;
		if (!pvcEntry.enable)
			continue;
		
		vcIdx = VC_INDEX(pvcEntry.ifIndex);
				
		/* set membership into vc */
		snprintf(wanif, 5, "vc%d", vcIdx);
		// mpoactl set vc0 group 1 member 3
		snprintf(msg.data, BUF_SIZE,
			"mpoactl set %s group 1 member %d",
			wanif, member);
		TRACE(STA_SCRIPT, "%s\n", msg.data);
		write_to_mpoad(&msg);
	}

	/* set membership 0 into LAN ports */	
	num = mib_chain_total(MIB_SW_PORT_TBL);	
	for (k=0; k<num; k++) {		
		setup_8305_vlan_member(k, member);		
	}

#ifdef WLAN_SUPPORT
#ifndef WLAN_MBSSID
	/* set membership 0 into wlan0 */	
	setup_wlan_group_member(0, member);
			
#else
	for (k=0; k<5; k++) {		
		setup_wlan_group_member(k, member);
	}
#endif
#endif

#ifdef CONFIG_USB_ETH
	/* set membership 0 into usb0 */	
	setup_usb_group_member( member );
#endif //CONFIG_USB_ETH		
	
	// set port 4 member to all
#ifdef CONFIG_USBCLIENT
      	setup_8305_vlan_member(5, member);
#else		
	setup_8305_vlan_member(4, member);
#endif

	
	// disable Port Mapping
	__dev_setupPortMapping(0);
	
	
	// disable 8305 vlan
	mib_get( MIB_MPMODE, (void *)&vChar);
	// If one the IGMP Snooping, IPQoS and Port Mapping is enabled, we  can not disable VLAN.
	if ( vChar == 0) {
		printf("Disable 8305 VLAN.\n");
		setup_8305_vlan_disable();
	}
	
	printf("Port Mapping: Ethernet to pvc mapping stoped\n");
}
#endif	// of ITF_GROUP
// Kaohj
#ifdef VLAN_GROUP
void setupEth2pvc_vlan()
{
#ifdef EMBED
	struct ifreq ifr;
	struct ifvlan ifvl;
	int i, k, mbr, maskbit;
	MIB_CE_VLAN_T vlan_entry;
	MIB_CE_SW_PORT_T sw_entry;
	unsigned char mode;
	
	mib_get(MIB_MPMODE, (void *)&mode);
#ifdef IP_QOS_VPORT
	if (mode&0x07) // IPQ, PortMapping and IGMP-snooping need to register virtual LAN ports by only once
#else
	if (mode&0x05)	// PortMapping and IGMP-snooping need to register virtual LAN ports
#endif
	__dev_register_swport();
	
	strcpy(ifr.ifr_name, "eth0");
	// set VLAN G(group1), H(group2), I(group3), J(group4)
	// VLANF(group0) is default group
	// VLAN A, B, C, D, E are used for LAN port identification
	ifvl.cmd=VLAN_SETINFO;
	//------------ set default group ---------------
	ifvl.vlanIdx=SW_PORT_NUM+1; // don't touch sw_port and cpu port
	ifvl.vid=0;
	ifvl.member = 0xffffffff;
	ifr.ifr_data = (char *)&ifvl;
	setVlan(&ifr);
	//------------ set user-defined group ----------
	for (i=1; i<VLAN_NUM; i++) {
		mib_chain_get(MIB_VLAN_TBL, i, (void *)&vlan_entry);
		ifvl.vlanIdx=SW_PORT_NUM+1+i; // don't touch sw_port and cpu port
		// VID 0 is used to identify priority frames and VID 4095(0xFFF) is reserved
		ifvl.vid=vlan_entry.tag;
		ifvl.member = vlan_entry.member; //mbr; // member ports: 
		ifvl.member |= 0x10; // Set CPU as a member of each vlan.
		ifr.ifr_data = (char *)&ifvl;
		//printf("vlanidx: %d, vid: %d, mbr: 0x%x\n", ifvl.vlanIdx, ifvl.vid, ifvl.member);
		setVlan(&ifr);
	}
	
	//------------ Set LAN pvid ---------------
	ifvl.cmd=VLAN_SETPVIDX;
	for (i=0; i<SW_PORT_NUM; i++) {
		// set sw-port 0~3 vlan idex.
		mib_chain_get(MIB_SW_PORT_TBL, i, (void *)&sw_entry);
		ifvl.port=SW_PORT_NUM+1+i; // don't touch sw_port and cpu port
		ifvl.vlanIdx=SW_PORT_NUM+1+sw_entry.pvid;
		ifr.ifr_data = (char *)&ifvl;
		//printf("port: %d, pvid: %d\n", ifvl.port, ifvl.vlanIdx);
		setVlan(&ifr);
	}
	
	//------------ enable vlan-grouping -----------
	__dev_setupVlanGrouping(1);
	//------------ enable 8305 vlan -----------
	setup_8305_vlan_enable();
#endif	// of EMBED
}
void setupEth2pvc_vlan_disable()
{		
	unsigned char vChar=0;
	
	// disable VLAN groupping
	__dev_setupVlanGrouping(0);
	
	// disable 8305 vlan
	mib_get( MIB_MPMODE, (void *)&vChar);
	// If one the IGMP Snooping, IPQoS and Port Mapping is enabled, we  can not disable VLAN.
	if ( vChar == 0) {
		printf("Disable switch VLAN.\n");
		setup_8305_vlan_disable();
		__dev_setupVirtualPort(0);
	}
	
	printf("Port/VLAN mapping stopped\n");
}
#endif	// of VLAN_GROUP
#endif // of CONFIG_EXT_SWITCH

// ioctl for direct bridge mode, jiunming
void __dev_setupDirectBridge(int flag)
{
	struct ifreq ifr;
	int ret;
	
	strcpy(ifr.ifr_name, "eth0");
	ifr.ifr_ifru.ifru_ivalue = flag;
#ifdef EMBED
	if( do_ioctl(SIOCDIRECTBR, &ifr)==0 ) {
		ret = 1;
		//printf("Set direct bridge mode %s!\n", flag?"enable":"disable" );
	}
	else {
		ret = 0;
		//printf("Set direct bridge mode error!\n");
	}
#endif

}

// Mason Yu, For Set IPQOS
//ql 20081117 START for ip qos
#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
int setIPQos(struct ifreq *ifr)
{
	int ret=0;
	
#ifdef EMBED		
	if (do_ioctl(SIOCSIPQOS, ifr) == 0)
		ret = 1;
	
#endif
	return ret;
}
#endif

// enable/disable IPQoS
void __dev_setupIPQoS(int flag)
{
//ql 20081117 START for ip qos
#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
	struct ifreq ifr;	
	struct ifIpQos ifipq;
	
	strcpy(ifr.ifr_name, "eth0");
	ifr.ifr_data = (char *)&ifipq;
	ifipq.cmd=SETIPQOS;
	ifipq.enable=flag;	
	setIPQos(&ifr);
	//printf("IPQoS: %s\n", flag?"enabled":"disabled");
#endif
}


#ifdef QOS_DIFFSERV
static char* proto2layer2[] = {
    [0]" ",
    [1]"6",
    [2]"17",
    [3]"1",
};

static char* strPolicing[] = {
    [0]" ",
    [1]"drop",
    [2]"continue",
};

void cleanupDiffservRule(int idx)
{
	unsigned int num, i;
	MIB_CE_IP_QOS_T qEntry;
	char wanif[6];

	mib_chain_get(MIB_IP_QOS_TBL, idx, (void *)&qEntry);
	if (PPP_INDEX(qEntry.ifIndex) != 0x0f)	// PPP interface
		snprintf(wanif, 6, "ppp%u", PPP_INDEX(qEntry.ifIndex));
	else		// vc interface
		snprintf(wanif, 6, "vc%u", VC_INDEX(qEntry.ifIndex));

	// tc qdisc del dev vc0 root
	va_cmd(TC, 5, 1, "qdisc", (char *)ARG_DEL, "dev", wanif, "root");
}

int deleteDiffservEntry()
{
	unsigned int totalEntry, i;
	MIB_CE_IP_QOS_T entry;
	int chklist[MAX_QOS_RULE], countlist = 0;
	unsigned int idx, j;

	totalEntry = mib_chain_total(MIB_IP_QOS_TBL); /* get chain record size */
	// Delete all existed diffserv entry
	for (i = 0; i < totalEntry; i ++) {
		if (!mib_chain_get(MIB_IP_QOS_TBL, i, (void *)&entry)) {
			printf("%s\n", errGetEntry);
			return 1;
		}
		if (entry.enDiffserv == 1) {
			chklist[countlist] = i;
			countlist ++;
		}
	}
	for (i = 0; i < totalEntry; i ++) {
		idx = totalEntry - i - 1;
		for (j = 0; j < countlist; j ++) {
			if (chklist[j] == idx) {
				if (!mib_chain_get(MIB_IP_QOS_TBL, idx, (void *)&entry)) {
					printf("%s\n", errGetEntry);
					return 1;
				}
				// delete from chain record
				if (mib_chain_delete(MIB_IP_QOS_TBL, idx) != 1) {
					printf("Delete MIB_IP_QOS_TBLchain record error!\n");
					return 1;
				}
			}
		}
	}
	return 0;
}

static void diffserv_filter_rule(MIB_CE_IP_QOS_Tp qEntry, char *prio, char *classid)
{
	char *argv[60], wanif[6];
	char saddr[20], daddr[20], sport[6], dport[6], strdscp[6] = {0}, strUpLinkRate[10];
	char *psaddr, *pdaddr;
	int idx, i;

	if (PPP_INDEX(qEntry->ifIndex) != 0x0f)	// PPP interface
		snprintf(wanif, 6, "ppp%u", PPP_INDEX(qEntry->ifIndex));
	else		// vc interface
		snprintf(wanif, 6, "vc%u", VC_INDEX(qEntry->ifIndex));

	// source ip, mask
	snprintf(saddr, 20, "%s", inet_ntoa(*((struct in_addr *)qEntry->sip)));
	if (strcmp(saddr, ARG_0x4) == 0)
		psaddr = 0;
	else {
		if (qEntry->smaskbit!=0)
			snprintf(saddr, 20, "%s/%d", saddr, qEntry->smaskbit);
		psaddr = saddr;
	}
	// destination ip, mask
	snprintf(daddr, 20, "%s", inet_ntoa(*((struct in_addr *)qEntry->dip)));
	if (strcmp(daddr, ARG_0x4) == 0)
		pdaddr = 0;
	else {
		if (qEntry->dmaskbit!=0)
			snprintf(daddr, 20, "%s/%d", daddr, qEntry->dmaskbit);
		pdaddr = daddr;
	}
	snprintf(sport, 6, "%d", qEntry->sPort);
	snprintf(dport, 6, "%d", qEntry->dPort);

	// Classifier setup for 1:0
	// tc filter add dev vc0 parent 1:0 protocol ip prio 1 u32
	//	match ip src 192.168.1.3/32 match ip dst 192.168.8.11/32
	//	match ip tos 0x38 0xff match ip protocol 6 0xff
	//	match ip sport 1090 0xff match ip dport 21 0xff
	//	police rate 500kbit burst 10k drop classid 1:1
	argv[1] = "filter";
	argv[2] = (char *)ARG_ADD;
	argv[3] = "dev";
	argv[4] = wanif;
	argv[5] = "parent";
	argv[6] = "1:0";
	argv[7] = "protocol";
	argv[8] = "ip";
	argv[9] = "prio";
	argv[10] = prio;
	argv[11] = "u32";
	idx = 12;

	// match filter
	// src ip
	if (psaddr != 0) {
		argv[idx++] = "match";
		argv[idx++] = "ip";
		argv[idx++] = "src";
		argv[idx++] = psaddr;
	}
	// dst ip
	if (pdaddr != 0) {
		argv[idx++] = "match";
		argv[idx++] = "ip";
		argv[idx++] = "dst";
		argv[idx++] = pdaddr;
	}
	//dscp match
#ifdef QOS_DSCP_MATCH
	if (0 != qEntry->qosDscp) {
		argv[idx++] = "match";
		argv[idx++] = "ip";
		argv[idx++] = "tos";
		snprintf(strdscp, 6, "0x%x", (qEntry->qosDscp-1)&0xFF);
		argv[idx++] = strdscp;
		argv[idx++] = "0xff";
	}
#endif
	// protocol
	if (qEntry->protoType != PROTO_NONE) {
		argv[idx++] = "match";
		argv[idx++] = "ip";
		argv[idx++] = "protocol";
		argv[idx++] = proto2layer2[qEntry->protoType];
		argv[idx++] = "0xff";
	}
	// src port
	if ((qEntry->protoType==PROTO_TCP ||
		qEntry->protoType==PROTO_UDP) &&
		qEntry->sPort != 0) {
		argv[idx++] = "match";
		argv[idx++] = "ip";
		argv[idx++] = "sport";
		argv[idx++] = sport;
		argv[idx++] = "0xff";
	}
	// dst port
	if ((qEntry->protoType==PROTO_TCP ||
		qEntry->protoType==PROTO_UDP) &&
		qEntry->dPort != 0) {
		argv[idx++] = "match";
		argv[idx++] = "ip";
		argv[idx++] = "dport";
		argv[idx++] = dport;
		argv[idx++] = "0xff";
	}

	// police
	if (0 != qEntry->limitSpeed) {
		argv[idx++] = "police";
		argv[idx++] = "rate";
		snprintf(strUpLinkRate, 10, "%dKbit", qEntry->limitSpeed);
		argv[idx++] = strUpLinkRate;
		argv[idx++] = "burst";
		argv[idx++] = "10k";
		argv[idx++] = strPolicing[qEntry->policing];
	}

	argv[idx++] = "classid";
	argv[idx++] = classid;
	argv[idx++] = NULL;

	printf("%s", TC);
	for (i=1; i<idx-1; i++)
		printf(" %s", argv[i]);
	printf("\n");
	do_cmd(TC, argv, 1);
}

//static void diffserv_HTB_bw_div(MIB_CE_IP_QOS_Tp qEntry, char *classid, unsigned short rate)
static void diffserv_HTB_bw_div(MIB_CE_IP_QOS_Tp qEntry, char *classid)
{
	unsigned short ceil, rateBE;
	char wanif[6], strCeil[10], strRate[10], strRateBE[10];
	int htbrate = 0;

	if (PPP_INDEX(qEntry->ifIndex) != 0x0f)	// PPP interface
		snprintf(wanif, 6, "ppp%u", PPP_INDEX(qEntry->ifIndex));
	else		// vc interface
		snprintf(wanif, 6, "vc%u", VC_INDEX(qEntry->ifIndex));

	//patch: actual bandwidth maybe a little greater than configured limit value, so I minish 7% of the configured limit value ahead.
	//ceil = qEntry->totalBandwidth / 100 * 93;
	ceil = qEntry->totalBandwidth;
	htbrate = (qEntry->htbRate>=qEntry->totalBandwidth)?qEntry->totalBandwidth-100:qEntry->htbRate;
	rateBE = ceil - htbrate;
	snprintf(strCeil, 10, "%dKbit", ceil);
	snprintf(strRate, 10, "%dKbit", htbrate);
	snprintf(strRateBE, 10, "%dKbit", rateBE);

	// tc qdisc add dev vc0 parent 1:0 handle 2:0 htb
	va_cmd(TC, 9, 1, "qdisc", (char *)ARG_ADD, "dev", wanif, "parent", "1:0", "handle", "2:0", "htb");
	// tc class add dev vc0 parent 2:0 classid 2:1 htb rate 1Mbit ceil 1Mbit
	va_cmd(TC, 13, 1, "class", (char *)ARG_ADD, "dev", wanif, "parent", "2:0", "classid", "2:1", "htb", "rate", strCeil, "ceil", strCeil);
	// tc class add dev vc0 parent 2:1 classid 2:2 htb rate 1Mbit ceil 1Mbit
	va_cmd(TC, 13, 1, "class", (char *)ARG_ADD, "dev", wanif, "parent", "2:1", "classid", "2:2", "htb", "rate", strRate, "ceil", strCeil);
	// tc class add dev vc0 parent 2:1 classid 2:3 htb rate 1Mbit ceil 1Mbit
	va_cmd(TC, 13, 1, "class", (char *)ARG_ADD, "dev", wanif, "parent", "2:1", "classid", classid, "htb", "rate", strRateBE, "ceil", strCeil);
}

static void calculate_RED(MIB_CE_IP_QOS_Tp qEntry, char *strLimit, char *strMin, char *strMax, char *strBurst)
{
	int max, min, limit, burst;

	max = (int)(qEntry->totalBandwidth / 8 * qEntry->latency / 1000);
	min = (int)(max / 3);
	limit = (int)(8 * max);
	burst = (int)(( 2 * min * 1000 + max * 1000 ) / ( 3* 1000 ));

	snprintf(strLimit, 10, "%dKB", limit);
	snprintf(strMin, 10, "%dKB", min);
	snprintf(strMax, 10, "%dKB", max);
	snprintf(strBurst, 6, "%d", burst);
}

static void diffserv_be_queue(MIB_CE_IP_QOS_Tp qEntry, char *classid)
{
	char wanif[6], strMax[10], strMin[10], strLimit[10], strBurst[6], strBw[10];

	if (PPP_INDEX(qEntry->ifIndex) != 0x0f)	// PPP interface
		snprintf(wanif, 6, "ppp%u", PPP_INDEX(qEntry->ifIndex));
	else		// vc interface
		snprintf(wanif, 6, "vc%u", VC_INDEX(qEntry->ifIndex));
	snprintf(strBw, 10, "%dKbit", qEntry->totalBandwidth);
	calculate_RED(qEntry, strLimit, strMin, strMax, strBurst);

	// tc qdisc add dev vc0 parent 2:3 red limit 6KB min 1.5KB max 4.5KB burst 20 avpkt 1000 bandwidth 1Mbit probability 0.4
	printf("%s qdisc %s dev %s parent %s red limit %s min %s max %s burst %s avpkt 1000 bandwidth %s probability 0.4\n", TC
		, (char *)ARG_ADD, wanif, classid, strLimit, strMin, strMax, strBurst, strBw);
	va_cmd(TC, 21, 1, "qdisc", (char *)ARG_ADD, "dev", wanif, "parent", classid,
		"red", "limit", strLimit, "min", strMin, "max", strMax, "burst", strBurst,
		"avpkt", "1000", "bandwidth", strBw, "probability", "0.4");
}

static int setupEFTraffic(int efindex)
{
	MIB_CE_IP_QOS_T qEntry;
	char wanif[6], prio[2], classid[6], strdscp[6];
	int phb;

	mib_chain_get(MIB_IP_QOS_TBL, efindex, (void *)&qEntry);
	phb = qEntry.m_ipprio << 3 | qEntry.m_iptos << 1;
	if (PPP_INDEX(qEntry.ifIndex) != 0x0f)	// PPP interface
		snprintf(wanif, 6, "ppp%u", PPP_INDEX(qEntry.ifIndex));
	else		// vc interface
		snprintf(wanif, 6, "vc%u", VC_INDEX(qEntry.ifIndex));

	// Create the egress root "dsmark" qdisc on wan interface
	// tc qdisc add dev vc0 root handle 1:0 dsmark indices 64 default_index 2
	va_cmd(TC, 12, 1, "qdisc", (char *)ARG_ADD, "dev", wanif,
		"root", "handle", "1:0", "dsmark", "indices", "64", "default_index", "2");

	snprintf(prio, 2, "1");
	snprintf(classid, 6, "1:1");

	// Classifier setup for 1:0
	diffserv_filter_rule(&qEntry, prio, classid);

	// Classes to specify DSCPs
	// tc class change dev vc0 parent 1:0 classid 1:1 dsmark mask 0x3 value 0xb8
	snprintf(strdscp, 6, "0x%x", (phb << 2)&0xFF);
	va_cmd(TC, 13, 1, "class", "change", "dev", wanif,
		"parent", "1:0", "classid", classid, "dsmark", "mask", "0x3", "value", strdscp);
	// BE
	va_cmd(TC, 13, 1, "class", "change", "dev", wanif, "parent", "1:0", "classid", "1:2", "dsmark", "mask", "0x3", "value", "0x0");

	// bandwidth division with HTB
	snprintf(classid, 6, "2:3");
	diffserv_HTB_bw_div(&qEntry, classid);

	// queue setup
	// tc qdisc add dev vc0 parent 2:2 pfifo limit 5
	va_cmd(TC, 9, 1, "qdisc", (char *)ARG_ADD, "dev", wanif, "parent", "2:2", "pfifo", "limit", "5");
	diffserv_be_queue(&qEntry, classid);

	// classifier setup for 2:1
	// tc filter add dev vc0 parent 2:0 protocol ip prio 1 handle 1 tcindex classid 2:2
	va_cmd(TC, 15, 1, "filter", (char *)ARG_ADD, "dev", wanif, "parent", "2:0", "protocol", "ip",
		"prio", "1", "handle", "1", "tcindex", "classid", "2:2");
	// tc filter add dev vc0 parent 2:0 protocol ip prio 2 handle 2 tcindex classid 2:3
	va_cmd(TC, 15, 1, "filter", (char *)ARG_ADD, "dev", wanif, "parent", "2:0", "protocol", "ip",
		"prio", "2", "handle", "2", "tcindex", "classid", classid);

	return 0;
}

static int setupAFTraffic(int *afindex, int aftotal, char *wanif)
{
	MIB_CE_IP_QOS_T qEntry;
	char prio[2], classid[6], strdscp[6];
	int i, phb;
	//int htbrate = 0;
	char strMax[10], strMin[10], strLimit[10], strBurst[6], strBw[10], strDP[2], strProbability[6];

	// Create the root "dsmark" qdisc on wan interface
	// tc qdisc add dev vc0 handle 1:0 root dsmark indices 64
	va_cmd(TC, 10, 1, "qdisc", (char *)ARG_ADD, "dev", wanif,
		"handle", "1:0", "root", "dsmark", "indices", "64");

	// dsmark classes to specify DSCPs
	// tc class change dev vc0 parent 1:0 classid 1:11 dsmark mask 0x3 value 0x28
	for (i = 0; i < aftotal; i ++) {
		mib_chain_get(MIB_IP_QOS_TBL, afindex[i], (void *)&qEntry);
		phb = qEntry.m_ipprio << 3 | qEntry.m_iptos << 1;
		snprintf(strdscp, 6, "0x%x", (phb << 2)&0xFF);
		snprintf(classid, 6, "1:%d%d", qEntry.m_ipprio, qEntry.m_iptos);
		va_cmd(TC, 13, 1, "class", "change", "dev", wanif,
			"parent", "1:0", "classid", classid, "dsmark", "mask", "0x3", "value", strdscp);
	}
	// BE
	va_cmd(TC, 13, 1, "class", "change", "dev", wanif, "parent", "1:0", "classid", "1:5", "dsmark", "mask", "0x3", "value", "0x0");

	// Classifier setup for 1:0
	for (i = 0; i < aftotal; i ++) {
		mib_chain_get(MIB_IP_QOS_TBL, afindex[i], (void *)&qEntry);
		snprintf(prio, 2, "%d", i + 1);
		snprintf(classid, 6, "1:%d%d", qEntry.m_ipprio, qEntry.m_iptos);
		//htbrate += qEntry.limitSpeed;
		diffserv_filter_rule(&qEntry, prio, classid);
	}
	// BE: tc filter add dev vc0 parent 1:0 protocol ip prio 5 u32 match ip protocol 0 0 flowid 1:5
	va_cmd(TC, 18, 1, "filter", (char *)ARG_ADD, "dev", wanif,
		"parent", "1:0", "protocol", "ip", "prio", "5", "u32", "match", "ip", "protocol", "0", "0", "flowid", "1:5");

	// bandwidth division with HTB
	snprintf(classid, 6, "2:6");
	//htbrate = (htbrate>qEntry.totalBandwidth)?qEntry.totalBandwidth-100:htbrate;
	diffserv_HTB_bw_div(&qEntry, classid);

	// queue setup
	// GRED setup for AF class
	// tc qdisc add dev vc0 parent 2:2 gred setup DPs 3 default 2 grio
	va_cmd(TC, 13, 1, "qdisc", (char *)ARG_ADD, "dev", wanif,
		"parent", "2:2", "gred", "setup", "DPs", "3", "default", "2", "grio");
	// tc qdisc change dev vc0 parent 2:2 gred limit 6KB min 1.5KB max 4.5KB burst 20 avpkt 1000 bandwidth 1Mbit DP 1 probability 0.02 prio 2
	snprintf(strBw, 10, "%dKbit", qEntry.totalBandwidth);
	for (i = 0; i < 3; i ++) {
		calculate_RED(&qEntry, strLimit, strMin, strMax, strBurst);
		snprintf(strDP, 2, "%d", i + 1);
		snprintf(strProbability, 6, "0.0%d", 2 * (i + 1));
		snprintf(prio, 2, "%d", i + 2);
		va_cmd(TC, 25, 1, "qdisc", "change", "dev", wanif, "parent", "2:2",
			"gred", "limit", strLimit, "min", strMin, "max", strMax, "burst", strBurst,
			"avpkt", "1000", "bandwidth", strBw, "DP", strDP, "probability", strProbability, "prio", prio);
		printf("%s qdisc change dev %s parent 2:2 gred limit %s min %s max %s burst %s avpkt 1000 bandwidth %s DP %s probability %s prio %s\n"
			, TC, wanif, strLimit, strMin, strMax, strBurst, strBw, strDP, strProbability, prio);
	}
	// RED setup for BE
	diffserv_be_queue(&qEntry, classid);

	// classifier setup for 2:1
	// tc filter add dev vc0 parent 2:0 protocol ip prio 1 handle 17 tcindex classid 2:2
	for (i = 0; i < aftotal; i ++) {
		mib_chain_get(MIB_IP_QOS_TBL, afindex[i], (void *)&qEntry);
		snprintf(strdscp, 6, "%d", qEntry.m_ipprio * 16 + qEntry.m_iptos);
		va_cmd(TC, 15, 1, "filter", (char *)ARG_ADD, "dev", wanif, "parent", "2:0", "protocol", "ip",
			"prio", "1", "handle", strdscp, "tcindex", "classid", "2:2");
	}
	// tc filter add dev vc0 parent 2:0 protocol ip prio 1 handle 5 tcindex classid 2:3
	va_cmd(TC, 15, 1, "filter", (char *)ARG_ADD, "dev", wanif, "parent", "2:0", "protocol", "ip",
		"prio", "1", "handle", "5", "tcindex", "classid", classid);

	return 0;
}

int setupDiffServ(void)
{
	unsigned int num, i;
	MIB_CE_IP_QOS_T qEntry;
	char wanif[6];
	unsigned char phbclass;
	int efIndex, afIndex[3], afcount = 0;

	mib_get(MIB_DIFFSERV_PHBCLASS, (void *)&phbclass);
	num = mib_chain_total(MIB_IP_QOS_TBL);
	for (i = 0; i < num; i ++) {
		if (!mib_chain_get(MIB_IP_QOS_TBL, i, (void *)&qEntry))
			continue;
		if (qEntry.enDiffserv == 0) // non-Diffserv entry
			continue;
		//if (qEntry.m_ipprio == 5 && qEntry.m_iptos == 3) {	// EF
		if (phbclass == qEntry.m_ipprio) {
			if (phbclass == 5) {	// EF
				efIndex = i;
				break;
			}
			else {	// AF
				afIndex[afcount] = i;
				afcount ++;
				if (PPP_INDEX(qEntry.ifIndex) != 0x0f)	// PPP interface
					snprintf(wanif, 6, "ppp%u", PPP_INDEX(qEntry.ifIndex));
				else		// vc interface
					snprintf(wanif, 6, "vc%u", VC_INDEX(qEntry.ifIndex));
			}
		}
	}
	if (num > 0) {
		if (afcount > 0)
			setupAFTraffic(afIndex, afcount, wanif);
		else 
			setupEFTraffic(efIndex);
	}

	return 0;
}
#endif // #ifdef QOS_DIFFSERV


#ifdef CONFIG_EXT_SWITCH
/*------------------------------------------------------------------
 * Set the group id into interface records.
 * where,
 * list: a list of interface id seperated by comma
 * grpnum: group id
 *-----------------------------------------------------------------*/
#ifdef ITF_GROUP
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
void setitfPrio(int itfid,  enum PortMappingPriority prio)
{
	//interface id is  	0-255 physical lan port, 
	//				256- 1023   logic lan port, 
	//				1024.....      Wlans
	int itfdomain;
	int phyPort;
	int entryNum,i;
	ATM_QOS_T qos;
	//MIB_CE_IP_QOS_T Entry;
	//memset(&Entry, 0, sizeof(MIB_CE_IP_QOS_T));
	itfdomain = IF_DOMAIN(itfid);
	itfid = itfid&0x0ff;
	printf("setitfPrio: itfid=%08x, prio=%d\n ", itfid, prio);

	 if (itfdomain == DOMAIN_WAN) {
			MIB_CE_ATM_VC_T Entry;	
			MIB_CE_ATM_VC_T entry;	
			int k;
			int num;
			num = mib_chain_total(MIB_ATM_VC_TBL);
			for (i=0; i<num; i++) {
				if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
					return;
				if (Entry.enable && Entry.ifIndex==itfid) {
					qos=prio2QoS[prio];
					Entry.qos=qos;
					switch(qos)
					{
						case ATMQOS_UBR:
							Entry.pcr=6000;
							break;
						case ATMQOS_CBR:
							Entry.pcr=6000;
							break;
						case ATMQOS_VBR_NRT:
						case ATMQOS_VBR_RT:
							Entry.pcr=6000;
							Entry.scr=5000;
							Entry.mbs=128;
							break;
						default:
							Entry.pcr=6000;
							break;
					}
					//Entry.itfGroup = grpnum;
					mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, i);
					printf("QoS set: setitfPrio qos=%d\n", qos);
					// synchronize this vc across all interfaces
					for (k=i+1; k<num; k++) 
					{
						if (!mib_chain_get(MIB_ATM_VC_TBL, k, (void *)&entry)) 
						{
							return;
						}
						if (entry.vpi == Entry.vpi && entry.vci == Entry.vci) 
						{
							entry.qos = Entry.qos;
							entry.pcr = Entry.pcr;
							entry.cdvt = Entry.cdvt;
							entry.scr = Entry.scr;
							entry.mbs = Entry.mbs;
							// log message
							mib_chain_update(MIB_ATM_VC_TBL, (void *)&entry, k);
						}
					}
					break;
				}
			}
		}	
}
#endif



#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
void setgroup(char *list, int grpnum, enum PortMappingPriority priority)
#else
void setgroup(char *list, int grpnum)
#endif
{
	int itfid, itfdomain;
	int i, num;
	char *arg0, *token;
	
	arg0=list;
	while ((token=strtok(arg0,","))!=NULL) {		
		itfid = atoi(token);		
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		setitfPrio(itfid, priority );
#endif		
		itfdomain = IF_DOMAIN(itfid);
		itfid = itfid&0x0ff;		

		if (itfdomain == DOMAIN_ELAN) {
			MIB_CE_SW_PORT_T Entry;
			
			num = mib_chain_total(MIB_SW_PORT_TBL);
			if (itfid < num) {
				if (!mib_chain_get(MIB_SW_PORT_TBL, itfid, (void *)&Entry))
					return;
				Entry.itfGroup = grpnum;
				mib_chain_update(MIB_SW_PORT_TBL, (void *)&Entry, itfid);
			}
		}
#ifdef WLAN_SUPPORT
		else if (itfdomain == DOMAIN_WLAN) {
			char mygroup;
			
			mygroup = (char)grpnum;			
			if (itfid == 0)
				mib_set(MIB_WLAN_ITF_GROUP, (void *)&mygroup);
				
			// Added by Mason Yu			
#ifdef WLAN_MBSSID
			for ( i=1; i<5; i++) {
				if (itfid == i) {// vap0								
					mib_set(MIB_WLAN_VAP0_ITF_GROUP+i-1, (void *)&mygroup);
				}
			}
			
#endif
		}
#endif		

#ifdef CONFIG_USB_ETH
		else if (itfdomain == DOMAIN_ULAN) {
			char mygroup;
			mygroup = (char)grpnum;
			if (itfid == 0)
				mib_set(MIB_USBETH_ITF_GROUP, (void *)&mygroup);
		}		
#endif

		else if (itfdomain == DOMAIN_WAN) {
			MIB_CE_ATM_VC_T Entry;
			
			num = mib_chain_total(MIB_ATM_VC_TBL);
			for (i=0; i<num; i++) {
				if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
					return;
				if (Entry.enable && Entry.ifIndex==itfid) {
					Entry.itfGroup = grpnum;
					mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, i);
				}
			}
		}
		arg0=0;
	}
}

/*------------------------------------------------------------------
 * Get a list of interface info. (itfInfo) of group.
 * where,
 * info: a list of interface info entries
 * len: max length of the info list
 * grpnum: group id
 *-----------------------------------------------------------------*/
int get_group_ifinfo(struct itfInfo *info, int len, int grpnum)
{
	unsigned int swNum, vcNum;
	int i, num;
	char mygroup;
	char strlan[]="LAN0", strPPP[]="ppp0", strvc[]="vc0";
	MIB_CE_SW_PORT_T Entry;
	MIB_CE_ATM_VC_T pvcEntry;
#ifdef CTC_WAN_NAME
	char wanname[100];	
#endif

	// LAN ports
	swNum = mib_chain_total(MIB_SW_PORT_TBL);
	num=0;
	for (i=swNum; i>0; i--) {
		if (!mib_chain_get(MIB_SW_PORT_TBL, i-1, (void *)&Entry))
			return -1;
		if (Entry.itfGroup == grpnum) {
			strlan[3] = '0'+virt2user[i-1];
			info[num].ifdomain = DOMAIN_ELAN;
			info[num].ifid=i-1;
			strncpy(info[num].name, strlan, 8);
			num++;
		}
		if (num > len)
			break;
	}
	
#ifdef WLAN_SUPPORT
	// wlan0
	mib_get(MIB_WLAN_ITF_GROUP, (void *)&mygroup);
	if (mygroup == grpnum) {
		info[num].ifdomain = DOMAIN_WLAN;
		info[num].ifid=0;
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		sprintf(info[num].name, "SSID1(root)");
#else
		strncpy(info[num].name, (char *)WLANIF, 8);
#endif
		num++;
	}
#endif
	
#ifdef WLAN_MBSSID
	for (i=1; i<5; i++) {
		mib_get(MIB_WLAN_VAP0_ITF_GROUP+i-1, (void *)&mygroup);	
		if (mygroup == grpnum) {
			info[num].ifdomain = DOMAIN_WLAN;
			info[num].ifid=i;
			//strncpy(info[num].name, "vap0", 8);
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
			sprintf(info[num].name, "SSID%d", i+1);
#else
			sprintf(info[num].name, "vap%d", i-1);
#endif
			num++;
		}
	}	

#endif	

#ifdef CONFIG_USB_ETH
	// usb0
	mib_get(MIB_USBETH_ITF_GROUP, (void *)&mygroup);
	if (mygroup == grpnum) {
		info[num].ifdomain = DOMAIN_ULAN;
		info[num].ifid=0;
		strcpy(info[num].name, (char *)USBETHIF);
		num++;
	}
#endif	
	// vc
	vcNum = mib_chain_total(MIB_ATM_VC_TBL);
	
	for (i=0; i<vcNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&pvcEntry))
		{
  			//websError(wp, 400, T("Get chain record error!\n"));
  			printf("Get chain record error!\n");
			return -1;
		}
		
		if (pvcEntry.enable == 0 || pvcEntry.itfGroup!=grpnum)
			continue;

#ifdef CTC_WAN_NAME
		getWanName(&pvcEntry, wanname);
		info[num].ifdomain = DOMAIN_WAN;
		info[num].ifid=pvcEntry.ifIndex;
		strncpy(info[num].name, wanname, 30);
		num++;
#else
		if (PPP_INDEX(pvcEntry.ifIndex) != 0x0f)
		{	// PPP interface
			strPPP[3] = '0'+PPP_INDEX(pvcEntry.ifIndex);
			info[num].ifdomain = DOMAIN_WAN;
			info[num].ifid=pvcEntry.ifIndex;
			strncpy(info[num].name, strPPP, 8);
			num++;
		}
		else
		{	// vc interface
			strvc[2] = '0'+VC_INDEX(pvcEntry.ifIndex);
			info[num].ifdomain = DOMAIN_WAN;
			info[num].ifid=pvcEntry.ifIndex;
			strncpy(info[num].name, strvc, 8);
			num++;
		}
#endif		
		if (num > len)
			break;
	}
	
	return num;
}
#endif	// of ITF_GROUP
// Kaohj
#ifdef VLAN_GROUP
// membership bit-map:
// | resvd | vc7 vc6 vc5 vc4 | vc3 vc2 vc1 vc0 |
//  vap3 | vap2 vap1 vap0 wlan0 | resvd | LAN4 LAN3 LAN2 LAN1
void setgroup(char *list, int grpnum, int vlan_vid, int vlan_dhcps)
{
	int itfid, itfdomain;
	int i, vlan_num, vc_num;
	char *arg0, *token;
	unsigned int member, maskbit;
	MIB_CE_VLAN_T vlan_entry;
	MIB_CE_ATM_VC_T vc_entry;
	MIB_CE_SW_PORT_T sw_entry;
	
	vlan_num = mib_chain_total(MIB_VLAN_TBL);
	if (grpnum <= 0 && grpnum >=vlan_num)
		return;
	mib_chain_get(MIB_VLAN_TBL, grpnum, (void *)&vlan_entry);
	// reset pvid
	maskbit = 1;
	for (i=0; i<4; i++) {
		if (vlan_entry.member & maskbit) {
			mib_chain_get(MIB_SW_PORT_TBL, i, (void *)&sw_entry);
			sw_entry.pvid = 0;
			mib_chain_update(MIB_SW_PORT_TBL, (void *)&sw_entry, i);
		}
		maskbit<<=1;
	}
	vlan_entry.member = 0;
	vlan_entry.tag = vlan_vid;
	vlan_entry.dhcps = vlan_dhcps;
	vc_num = mib_chain_total(MIB_ATM_VC_TBL);
	
	member = 0;
	arg0=list;
	while ((token=strtok(arg0,","))!=NULL) {		
		itfid = atoi(token);
		itfdomain = IF_DOMAIN(itfid);
		itfid = itfid&0x0ff;

		if (itfdomain == DOMAIN_ELAN) {
			// VLAN membershipt
			vlan_entry.member |= 1<<itfid;
			// set pvid
			mib_chain_get(MIB_SW_PORT_TBL, itfid, (void *)&sw_entry);
			sw_entry.pvid = grpnum;
			mib_chain_update(MIB_SW_PORT_TBL, (void *)&sw_entry, itfid);
		}
		else if (itfdomain == DOMAIN_WAN) {
			for (i=0; i<vc_num; i++) {
				mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&vc_entry);
				if (vc_entry.enable && VC_INDEX(vc_entry.ifIndex)==VC_INDEX(itfid)) {
					if (vc_entry.cmode == ADSL_BR1483 ||
						vc_entry.cmode == ADSL_MER1483 ||
						vc_entry.cmode == ADSL_PPPoE)
						vlan_entry.member |= (1<<16)<<VC_INDEX(itfid);
				}
			}
		}
		arg0=0;
	}
	//printf("vlan_entry %d, member=0x%x, tag=%d\n", grpnum, vlan_entry.member, vlan_entry.tag);
	mib_chain_update(MIB_VLAN_TBL, (void *)&vlan_entry, grpnum);
}

/*------------------------------------------------------------------
 * Get a list of interface info. (itfInfo) of group.
 * where,
 * info: a list of interface info entries
 * len: max length of the info list
 * grpnum: group id
 *-----------------------------------------------------------------*/
int get_group_ifinfo(struct itfInfo *info, int len, int grpnum)
{
	unsigned int swNum, vcNum;
	int i, k, num;
	unsigned int dft_member, maskbit;
	char mygroup;
	char strlan[]="LAN0", strPPP[]="ppp0", strvc[]="vc0";
	MIB_CE_ATM_VC_T pvcEntry;
	MIB_CE_VLAN_T vlan_entry;

	if (grpnum == 0) { // default group
		dft_member = 0x0f;
		// LAN -- get default LAN members
		for (k=1; k<VLAN_NUM; k++) {
			maskbit = 1;
			mib_chain_get(MIB_VLAN_TBL, k, (void *)&vlan_entry);
			for (i=0; i<4; i++) {
				if (vlan_entry.member & maskbit) {
					dft_member &= (~maskbit);
				}
				maskbit<<=1;
			}
		}
		// assign default LAN info.
		num = 0;
		maskbit = 1;
		for (i=0; i<4; i++) {
			if (num > len)
				break;
			if (dft_member & maskbit) {
				info[num].ifdomain = DOMAIN_ELAN;
				info[num].ifid=i;
				strlan[3] = '0'+virt2user[i];
				strncpy(info[num].name, strlan, 8);
				num++;
			}
			maskbit<<=1;
		}
		
		// WAN -- get default WAN members
		vcNum = mib_chain_total(MIB_ATM_VC_TBL);
		for (i=0; i<vcNum; i++) {
			mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&pvcEntry);
			if (pvcEntry.enable) {
				if (pvcEntry.cmode == ADSL_BR1483 ||
					pvcEntry.cmode == ADSL_MER1483 ||
					pvcEntry.cmode == ADSL_PPPoE)
					dft_member |= (1<<16)<<VC_INDEX(pvcEntry.ifIndex);
			}
		}
		// assign default WAN info.
		maskbit = 0x10000;
		for (i=0; i<8; i++) {
			if (num > len)
				break;
			if (dft_member & maskbit) {
				info[num].ifdomain = DOMAIN_WAN;
				info[num].ifid=(0xf0|i);
				strvc[2] = '0'+i;
				strncpy(info[num].name, strvc, 8);
				num++;
			}
			maskbit<<=1;
		}
		return num;
	}
	if (grpnum <=0 && grpnum >=VLAN_NUM)
		return 0;
	
	mib_chain_get(MIB_VLAN_TBL, grpnum, (void *)&vlan_entry);
	num = 0;
	// LAN
	maskbit = 1;
	for (i=0; i<4; i++) {
		if (num > len)
			break;
		if (vlan_entry.member & maskbit) {
			info[num].ifdomain = DOMAIN_ELAN;
			info[num].ifid=i;
			strlan[3] = '0'+virt2user[i];
			strncpy(info[num].name, strlan, 8);
			num++;
		}
		maskbit<<=1;
	}
	// WAN
	maskbit = 0x10000;
	for (i=0; i<8; i++) {
		if (num > len)
			break;
		if (vlan_entry.member & maskbit) {
			info[num].ifdomain = DOMAIN_WAN;
			info[num].ifid=(0xf0|i);
			strvc[2] = '0'+i;
			strncpy(info[num].name, strvc, 8);
			num++;
		}
		maskbit<<=1;
	}
	
#ifdef WLAN_SUPPORT
	// wlan0
#endif
	
#ifdef WLAN_MBSSID
#endif	

	return num;
}

void vg_setup_iptables()
{
	MIB_CE_VLAN_T vlan_entry;
	int i, k;
	unsigned int maskbit;
	char ifname[] = "eth0_sw0";
	unsigned char mode;
	
	va_cmd(IPTABLES, 2, 1, "-F", "itfgroup");
	mib_get(MIB_MPMODE, (void *)&mode);
	if (!(mode&0x01))
		return;
	
	for (i=1; i<VLAN_NUM; i++) {
		mib_chain_get(MIB_VLAN_TBL, i, (void *)&vlan_entry);
		if (!vlan_entry.dhcps) {
			// disable dhcp server for LAN port which is in the group
			maskbit = 1;
			for (k=0; k<4; k++) {
				// find the virtual LAN ports in this group
				if (vlan_entry.member & maskbit) {
					ifname[7] = '0'+ k;
					// iptables -A itfgroup -i eth0_sw0 -p udp --dport 67 -j DROP		
					va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, "itfgroup", (char *)ARG_I, (char *)ifname, "-p", "udp", (char *)FW_DPORT, (char *)PORT_DHCP, "-j", (char *)FW_DROP);
				}
				maskbit<<=1;
			}
		}
	}
}
#endif	// of VLAN_GROUP
#endif // of CONFIG_EXT_SWITCH

/*------------------------------------------------------------------
 * Get a list of interface info. (itfInfo) of the specified ifdomain.
 * where,
 * info: a list of interface info entries
 * len: max length of the info list
 * ifdomain: interface domain
 *-----------------------------------------------------------------*/
int get_domain_ifinfo(struct itfInfo *info, int len, int ifdomain)
{
	unsigned int swNum, vcNum;
	int i, num;
	char mygroup;
	char strlan[]="LAN0", strPPP[]="ppp0", strvc[]="vc0";
	//MIB_CE_SW_PORT_T Entry;
	//MIB_CE_ATM_VC_Tp pvcEntry;
	MIB_CE_ATM_VC_T pvcEntry;
	num=0;
	
	if (ifdomain&DOMAIN_ELAN) {
		// LAN ports
#if (defined(CONFIG_EXT_SWITCH)  && defined (IP_QOS_VPORT))
//#ifdef CONFIG_EXT_SWITCH
		swNum = mib_chain_total(MIB_SW_PORT_TBL);
		for (i=swNum; i>0; i--) {
			//if (!mib_chain_get(MIB_SW_PORT_TBL, i-1, (void *)&Entry))
			//	return -1
			strlan[3] = '0'+virt2user[i-1];
			info[num].ifdomain = DOMAIN_ELAN;
			info[num].ifid=i-1;
			strncpy(info[num].name, strlan, 8);
			num++;
			if (num > len)
				break;
		}
#else
		info[num].ifdomain = DOMAIN_ELAN;
		info[num].ifid=0;
		strncpy(info[num].name, strlan, 8);
		num++;
#endif
	}
	
#ifdef WLAN_SUPPORT
	if (ifdomain&DOMAIN_WLAN) {
		// wlan0
		mib_get(MIB_WLAN_ITF_GROUP, (void *)&mygroup);
		info[num].ifdomain = DOMAIN_WLAN;
		info[num].ifid=0;
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		sprintf(info[num].name, "SSID1(root)");
#else
		strncpy(info[num].name, (char *)WLANIF, 8);
#endif
		num++;
	}
//jim luo add it to support QoS on Virtual AP...
#ifdef WLAN_MBSSID
	for (i=1; i<5; i++) {
		mib_get(MIB_WLAN_VAP0_ITF_GROUP+i-1, (void *)&mygroup);	
		info[num].ifdomain = DOMAIN_WLAN;
		info[num].ifid=i;
		//strncpy(info[num].name, "vap0", 8);
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		sprintf(info[num].name, "SSID%d", i+1);
#else
		sprintf(info[num].name, "vap%d", i-1);
#endif
		num++;
	}	
#endif	
#endif

#ifdef CONFIG_USB_ETH
	if (ifdomain&DOMAIN_ULAN) {
		// usb0
		mib_get(MIB_USBETH_ITF_GROUP, (void *)&mygroup);
		info[num].ifdomain = DOMAIN_ULAN;
		info[num].ifid=0;
		sprintf(info[num].name, "%s", USBETHIF );
		num++;
	}
#endif //CONFIG_USB_ETH
	
	if (ifdomain&DOMAIN_WAN) {
		// vc
		vcNum = mib_chain_total(MIB_ATM_VC_TBL);
		
		for (i=0; i<vcNum; i++) {
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&pvcEntry))
			{
  				//websError(wp, 400, T("Get chain record error!\n"));
  				printf("Get chain record error!\n");
				return -1;
			}
			
			if (pvcEntry.enable == 0)
				continue;
			
			if (PPP_INDEX(pvcEntry.ifIndex) != 0x0f)
			{	// PPP interface
				strPPP[3] = '0'+PPP_INDEX(pvcEntry.ifIndex);
				info[num].ifdomain = DOMAIN_WAN;
				info[num].ifid=pvcEntry.ifIndex;
				strncpy(info[num].name, strPPP, 8);
				num++;
			}
			else
			{	// vc interface
				strvc[2] = '0'+VC_INDEX(pvcEntry.ifIndex);
				info[num].ifdomain = DOMAIN_WAN;
				info[num].ifid=pvcEntry.ifIndex;
				strncpy(info[num].name, strvc, 8);
				num++;
			}
			
			if (num > len)
				break;
		}
	}
	
	return num;
}


short Max(short* array, int len)
{
	short rValue = array[0];
	int idx;

	for(idx=1;idx<len;idx++) {
		if(array[idx] > rValue)
			rValue = array[idx];
	}
		
	return rValue;	
}

short Min(short* array, int len)
{
	short rValue = array[0];
	int idx;

	for(idx=1;idx<len;idx++) {
		if(array[idx] < rValue)
			rValue = array[idx];
	}
		
	return rValue;	
}

int read_pid(char *filename)
{
	int fh;
	FILE *in;
	int pid;

	fh = open(filename, O_RDWR);
	if ( fh == -1 ) return -1;
	if ((in = fdopen(fh, "r")) == NULL) return -1;
	fscanf(in, "%d", &pid);
	fclose(in);
	close(fh);

	return pid;
}

// Added by Kaohj
//return 0:OK, other:fail
int do_cmd(const char *filename, char *argv [], int dowait)
{
	pid_t pid, wpid;
	int stat=0, st;
	
	if((pid = vfork()) == 0) {
		/* the child */
		char *env[3];
		
		signal(SIGINT, SIG_IGN);
		argv[0] = (char *)filename;
		env[0] = "PATH=/bin:/usr/bin:/etc:/sbin:/usr/sbin";
		env[1] = NULL;

		execve(filename, argv, env);

		printf("exec %s failed\n", filename);
		_exit(2);
	} else if(pid > 0) {
		if (!dowait)
			stat = 0;
		else {
			/* parent, wait till rc process dies before spawning */
			while ((wpid = wait(&st)) != pid)
			{
				if (wpid == -1 && errno == EINTR)
				{
				     wpid=waitpid(pid, &st, WNOHANG);
				}
				if (wpid == -1 && errno == ECHILD) { /* see wait(2) manpage */
					stat = 0;
					break;
				}
			}
		}
	} else if(pid < 0) {
		printf("fork of %s failed\n", filename);
		stat = -1;
	}
	st = WEXITSTATUS(stat);
	return st;
}

//return 0:OK, other:fail
int va_cmd(const char *cmd, int num, int dowait, ...)
{
	va_list ap;
	int k;
	char *s;
	char *argv[24];
	int status;
	
	TRACE(STA_SCRIPT, "%s ", cmd);
	va_start(ap, dowait);
	
	for (k=0; k<num; k++)
	{
		s = va_arg(ap, char *);
		argv[k+1] = s;
		TRACE(STA_SCRIPT|STA_NOTAG, "%s ", s);
	}
	
	TRACE(STA_SCRIPT|STA_NOTAG, "\n");
	argv[k+1] = NULL;
	status = do_cmd(cmd, argv, dowait);
	va_end(ap);
	
	return status;
}

//return 0:OK, other:fail
/*same function as va_cmd(). Execute silently. 
  No print out command string in console.
*/
int va_cmd_no_echo(const char *cmd, int num, int dowait, ...)
{
	va_list ap;
	int k;
	char *s;
	char *argv[24];
	int status;
	
	//TRACE(STA_SCRIPT, "%s ", cmd);
	va_start(ap, dowait);
	
	for (k=0; k<num; k++)
	{
		s = va_arg(ap, char *);
		argv[k+1] = s;
		//TRACE(STA_SCRIPT|STA_NOTAG, "%s ", s);
	}
	
	//TRACE(STA_SCRIPT|STA_NOTAG, "\n");
	argv[k+1] = NULL;
	status = do_cmd(cmd, argv, dowait);
	va_end(ap);
	
	return status;
}

//return 0:OK, other:fail
int call_cmd(const char *filename, int num, int dowait, ...)
{
	va_list ap;
	char *s;
	char *argv[24];
	int status=0, st, k;
	pid_t pid, wpid;
	
	va_start(ap, dowait);
	
	for (k=0; k<num; k++)
	{
		s = va_arg(ap, char *);
		argv[k+1] = s;
	}
	
	argv[k+1] = NULL;
	if((pid = vfork()) == 0) {
		/* the child */
		char *env[3];
		
		signal(SIGINT, SIG_IGN);
		argv[0] = (char *)filename;
		env[0] = "PATH=/bin:/usr/bin:/etc:/sbin:/usr/sbin";
		env[1] = NULL;

		execve(filename, argv, env);

		printf("exec %s failed\n", filename);
		_exit(2);
	} else if(pid > 0) {
		if (!dowait)
			status = 0;
		else {
			/* parent, wait till rc process dies before spawning */
			while ((wpid = wait(&st)) != pid)
				if (wpid == -1 && errno == ECHILD) { /* see wait(2) manpage */
					break;
				}
		}
	} else if(pid < 0) {
		printf("fork of %s failed\n", filename);
		status = -1;
	}
	if (wpid>0)
		if (WIFEXITED(st))
			status = WEXITSTATUS(st);
//			printf("Child exited with RC=%d\n",WEXITSTATUS(st));
	va_end(ap);
	
	return status;
}

void write_to_pppd(struct data_to_pass_st *pmsg)
{
	int pppd_fifo_fd=-1;
	
	pppd_fifo_fd = open(PPPD_FIFO, O_WRONLY);
	if (pppd_fifo_fd == -1)
		fprintf(stderr, "Sorry, no spppd server\n");
	else
	{
		write(pppd_fifo_fd, pmsg, sizeof(*pmsg));
		close(pppd_fifo_fd);
	}
}

// return value:
// 0  : successful
// -1 : failed
int write_to_mpoad(struct data_to_pass_st *pmsg)
{
	int mpoad_fifo_fd=-1;
	int status=0;
	int ret=0;	
	
	mpoad_fifo_fd = open(MPOAD_FIFO, O_WRONLY);
	if (mpoad_fifo_fd == -1) {
		fprintf(stderr, "Sorry, no mpoad server\n");
		status = -1;
	} else
	{
		// Modified by Mason Yu
		//write(mpoad_fifo_fd, pmsg, sizeof(*pmsg));
REWRITE:
		ret = write(mpoad_fifo_fd, pmsg, sizeof(*pmsg));
		if(ret<0 && errno==EPIPE)
			 goto REWRITE;
#ifdef _LINUX_2_6_
		//sleep more time for mpoad to wake up
		usleep(100*1000);
#else
		usleep(30*1000);
#endif
		
		close(mpoad_fifo_fd);
		// wait server to consume it
#ifdef _LINUX_2_6_
		//sleep more time for mpoad to wake up
		usleep(100*1000);
#else
		usleep(1000);
#endif
	}
	return status;
}

static void WRITE_DHCPC_FILE(int fh, unsigned char *buf)
{
	if ( write(fh, buf, strlen(buf)) != strlen(buf) ) {
		printf("Write udhcpc script file error!\n");
		close(fh);
	}
}

static void write_to_dhcpc_script(char *fname, MIB_CE_ATM_VC_Tp pEntry)
{
	int fh;
	int mark;
	char buff[64];
#ifdef DEFAULT_GATEWAY_V2
	unsigned char dgw;
#endif
#ifdef IP_POLICY_ROUTING
	int i, num, found;
	MIB_CE_IP_QOS_T qEntry;
#endif
	
	fh = open(fname, O_RDWR|O_CREAT|O_TRUNC, S_IXUSR);
	
	if (fh == -1) {
		printf("Create udhcpc script file %s error!\n", fname);
		return;
	}
	
	WRITE_DHCPC_FILE(fh, "#!/bin/sh\n");
	WRITE_DHCPC_FILE(fh, "RESOLV_CONF=\"/var/udhcpc/resolv.conf\"\n");
	WRITE_DHCPC_FILE(fh, "[ \"$broadcast\" ] && BROADCAST=\"broadcast $broadcast\"\n");
	WRITE_DHCPC_FILE(fh, "[ \"$subnet\" ] && NETMASK=\"netmask $subnet\"\n");
	WRITE_DHCPC_FILE(fh, "ifconfig $interface $ip $BROADCAST $NETMASK -pointopoint\n");
	WRITE_DHCPC_FILE(fh, "MER_GW_INFO=\"/tmp/MERgw.\"$interface\n");	// Jenny, write MER1483 gateway info
	WRITE_DHCPC_FILE(fh, "\techo $router > $MER_GW_INFO\n");
	
#ifdef DEFAULT_GATEWAY_V2
	mib_get( MIB_ADSL_WAN_DGW_ITF, (void *)&dgw);	// Jenny, check default gateway
	if ((dgw == pEntry->ifIndex)
#ifdef AUTO_PPPOE_ROUTE
		 || (dgw == DGW_AUTO && (REMOTE_ACCESS_T)pEntry->cmode == ACC_MER)
#endif
	)
#else
	if (pEntry->dgw)
#endif
	{
		WRITE_DHCPC_FILE(fh, "if [ \"$router\" ]; then\n");
#ifdef DEFAULT_GATEWAY_V2
		if (ifExistedDGW() == 1)	// Jenny, delete existed default gateway first
			WRITE_DHCPC_FILE(fh, "\troute del default\n");
#endif
		WRITE_DHCPC_FILE(fh, "\twhile route del -net default gw 0.0.0.0 dev $interface\n");
		WRITE_DHCPC_FILE(fh, "\tdo :\n");
		WRITE_DHCPC_FILE(fh, "\tdone\n\n");
		WRITE_DHCPC_FILE(fh, "\tfor i in $router\n");
		WRITE_DHCPC_FILE(fh, "\tdo\n");
		WRITE_DHCPC_FILE(fh, "\tifconfig $interface pointopoint $i\n");
		WRITE_DHCPC_FILE(fh, "\troute add -net default gw $i dev $interface\n");
		WRITE_DHCPC_FILE(fh, "\tdone\n");
		WRITE_DHCPC_FILE(fh, "\tifconfig $interface -pointopoint\n");
		WRITE_DHCPC_FILE(fh, "fi\n");
	}
#ifdef DEFAULT_GATEWAY_V2	// Jenny, assign default gateway by remote WAN IP
	else {
		unsigned char dgw, dgwip[16];
		if (mib_get(MIB_ADSL_WAN_DGW_ITF, (void *)&dgw) != 0) {
			if (dgw == DGW_NONE && getMIB2Str(MIB_ADSL_WAN_DGW_IP, dgwip) == 0) {
				if (ifExistedDGW() == 1)
					WRITE_DHCPC_FILE(fh, "\troute del default\n");
				// route add default gw remotip
				snprintf(buff, 64, "\troute add default gw %s\n", dgwip);
				WRITE_DHCPC_FILE(fh, buff);
			}
		}
	}
#endif
	
	WRITE_DHCPC_FILE(fh, "if [ \"$dns\" ]; then\n");
	WRITE_DHCPC_FILE(fh, "\trm $RESOLV_CONF\n");
	WRITE_DHCPC_FILE(fh, "\tfor i in $dns\n");
	WRITE_DHCPC_FILE(fh, "\tdo\n");
	WRITE_DHCPC_FILE(fh, "\techo 'DNS=' $i\n");
	WRITE_DHCPC_FILE(fh, "\techo nameserver $i >> $RESOLV_CONF\n");
	WRITE_DHCPC_FILE(fh, "\tdone\n");
	WRITE_DHCPC_FILE(fh, "fi\n");
#ifdef IP_POLICY_ROUTING
	num = mib_chain_total(MIB_IP_QOS_TBL);
	found = 0;
	// set advanced-routing rule
	for (i=0; i<num; i++) {
		if (!mib_chain_get(MIB_IP_QOS_TBL, i, (void *)&qEntry))
			continue;
#ifdef QOS_DIFFSERV
		if (qEntry.enDiffserv == 1) // Diffserv entry
			continue;
#endif
		if (qEntry.outif == pEntry->ifIndex) {
			
			found = 1;
			mark = get_classification_mark(i);
			if (mark != 0) {
				WRITE_DHCPC_FILE(fh, "if [ \"$router\" ]; then\n");
				snprintf(buff, 64, "\tip ru add fwmark %x table %d\n", mark, VC_INDEX(pEntry->ifIndex)+PR_VC_START);
				WRITE_DHCPC_FILE(fh, buff);
			}
		}
	}
	if (found) {
		snprintf(buff, 64, "\tip ro add default via $router dev $interface table %d\n", VC_INDEX(pEntry->ifIndex)+PR_VC_START);
		WRITE_DHCPC_FILE(fh, buff);
		WRITE_DHCPC_FILE(fh, "fi\n");
	}
#endif
	close(fh);
}

// DHCP client configuration
// return value:
// 1  : successful
int startDhcpc(char *inf, MIB_CE_ATM_VC_Tp pEntry)
{
	unsigned char value[32];
	FILE *fp;
	DNS_TYPE_T dnsMode;
	/*ql:20080926 START: initial MIB_DHCP_CLIENT_OPTION_TBL*/
#ifndef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	char * argv[9];
#else
	char * argv[11];
#endif
	/*ql:20080926 END*/
	unsigned int i, vcTotal, resolvopt;
	
	argv[1] = inf;
	argv[2] = "up";
	argv[3] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s\n", IFCONFIG, argv[1], argv[2]);
	do_cmd(IFCONFIG, argv, 1);
	
	// udhcpc -i vc0 -p pid -s script
	argv[1] = (char *)ARG_I;
	argv[2] = inf;
	argv[3] = "-p";
#ifdef ZTE_GENERAL_ROUTER_SC
	unsigned char pidfile[32];
	snprintf(pidfile, 32, "/var/run/udhcpc%s.pid", inf);
//	printf("\nstartudhcpc:pidfile=%s\n",pidfile);
	argv[4] = pidfile;
#else
	argv[4] = (char *)DHCPC_PID;
#endif
	argv[5] = "-s";
	snprintf(value, 32, "%s.%s", (char *)DHCPC_SCRIPT_NAME, inf);
	write_to_dhcpc_script(value, pEntry);
	argv[6] = (char *)DHCPC_SCRIPT;
	/*ql:20080926 START: initial MIB_DHCP_CLIENT_OPTION_TBL*/
#ifndef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	argv[7] = NULL;
	
	if (strcmp(inf, LANIF) == 0)
	{
		// LAN interface
		// enable Microsoft auto IP configuration
		argv[7] = "-a";
		argv[8] = NULL;
	}
#else
	argv[7] = "-w";
	snprintf(value, 32, "%d", pEntry->ifIndex);
	argv[8] = value;
	argv[9] = NULL;

	if (strcmp(inf, LANIF) == 0)
	{
		argv[9] = "-a";
		argv[10] = NULL;
	}
#endif
	/*ql:20080926 END*/
	
	TRACE(STA_SCRIPT, "%s %s %s %s ", DHCPC, argv[1], argv[2], argv[3]);
#ifndef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	TRACE(STA_SCRIPT, "%s %s %s\n", argv[4], argv[5], argv[6]);
#else
	TRACE(STA_SCRIPT, "%s %s %s ", argv[4], argv[5], argv[6]);
	TRACE(STA_SCRIPT, "%s %s\n", argv[7], argv[8]);
#endif
	do_cmd(DHCPC, argv, 0);
	
	return 1;
}

#define BRNF_MODULE_MAC_FILTER	(0x01 << 0)
#define BRNF_MODULE_URLBLOCK		(0x01 << 1)
/*
  * Unified Control functio for Brctl brnf on/off
  * trigger : 0-> off,  1 -> on
  * module, the module identifier, must be in format of (0x1 << x)
*/
unsigned int br_nf_flag = 0;
void brnfctrl(int trigger, int module)
{
	int total = 0;
	int	orig;

	orig = br_nf_flag;
	total = ifWanNum("br");

	if (total == 0)//no WAN in bridge mode
	{
		if (br_nf_flag!=0)
			va_cmd("/bin/brctl", 2, 0, "brnf", "off");
		br_nf_flag = 0;
		return;
	}


	if (trigger == 0){
		br_nf_flag = br_nf_flag & (~module);
		if ((orig != 0)&&(br_nf_flag == 0)) 
			va_cmd("/bin/brctl", 2, 0, "brnf", "off");
	}
	else {
		br_nf_flag = br_nf_flag | module;
		if ((orig == 0)&& (br_nf_flag != 0))
			va_cmd("/bin/brctl", 2, 0, "brnf", "on");
	}

	
}

#ifdef ADDRESS_MAPPING
#ifdef MULTI_ADDRESS_MAPPING

char* addrMap_str[5] = {"None","One_to_one","Many_to_one","Many_to_many","One_to_many"};
char* getaddrmappingstr( unsigned int idx)
{
	return addrMap_str[idx];
}
unsigned int getaddrmappingidx(char *str)
{
	unsigned int idx;
	for (idx = 0;idx<5;idx++)
	{
		if (!strcmp(str,addrMap_str[idx]))
			 return idx;
	}

	 return 0;
}


int GetIP_AddressMap(MIB_CE_MULTI_ADDR_MAPPING_LIMIT_T *entry, ADDRESSMAP_IP_T *ip_info)
{
	unsigned char value[32];		     

        // Get Local Start IP
	if (((struct in_addr *)entry->lsip)->s_addr != 0)
	{
		strncpy(ip_info->lsip, inet_ntoa(*((struct in_addr *)entry->lsip)), 16);
		ip_info->lsip[15] = '\0';
	}
	else
		ip_info->lsip[0] = '\0';

	
	// Get Local End IP
	if (((struct in_addr *)entry->leip)->s_addr != 0)
	{
		strncpy(ip_info->leip, inet_ntoa(*((struct in_addr *)entry->leip)), 16);
		ip_info->leip[15] = '\0';
	}
	else
		ip_info->leip[0] = '\0';
	
	// Get Global Start IP
	if (((struct in_addr *)entry->gsip)->s_addr != 0)
	{
		strncpy(ip_info->gsip, inet_ntoa(*((struct in_addr *)entry->gsip)), 16);
		ip_info->gsip[15] = '\0';
	}
	else
		ip_info->gsip[0] = '\0';
	
	// Get Global End IP
	if (((struct in_addr *)entry->geip)->s_addr != 0)
	{
		strncpy(ip_info->geip, inet_ntoa(*((struct in_addr *)entry->geip)), 16);
		ip_info->geip[15] = '\0';
	}
	else
		ip_info->geip[0] = '\0';	
	
	sprintf(ip_info->srcRange, "%s-%s", ip_info->lsip, ip_info->leip);
	sprintf(ip_info->globalRange, "%s-%s", ip_info->gsip, ip_info->geip);
//	printf( "\r\nGetIP_AddressMap %s-%s", ip_info->lsip, ip_info->leip);
//	printf( "\r\nGetIP_AddressMap %s-%s", ip_info->gsip, ip_info->geip);
	return 1;
}

#else //!MULTI_ADDRESS_MAPPING
int GetIP_AddressMap(ADDRESSMAP_IP_T *ip_info)
{		
	unsigned char value[32];		
        
        // Get Local Start IP
        if (mib_get(MIB_LOCAL_START_IP, (void *)value) != 0)
	{
		if (((struct in_addr *)value)->s_addr != 0)
		{
			strncpy(ip_info->lsip, inet_ntoa(*((struct in_addr *)value)), 16);
			ip_info->lsip[15] = '\0';
		}
		else
			ip_info->lsip[0] = '\0';
	}
	
	// Get Local End IP
        if (mib_get(MIB_LOCAL_END_IP, (void *)value) != 0)
	{
		if (((struct in_addr *)value)->s_addr != 0)
		{
			strncpy(ip_info->leip, inet_ntoa(*((struct in_addr *)value)), 16);
			ip_info->leip[15] = '\0';
		}
		else
			ip_info->leip[0] = '\0';
	}
	
	// Get Global Start IP
	if (mib_get(MIB_GLOBAL_START_IP, (void *)value) != 0)
	{
		if (((struct in_addr *)value)->s_addr != 0)
		{
			strncpy(ip_info->gsip, inet_ntoa(*((struct in_addr *)value)), 16);
			ip_info->gsip[15] = '\0';
		}
		else
			ip_info->gsip[0] = '\0';
	}
	
	// Get Global End IP
	if (mib_get(MIB_GLOBAL_END_IP, (void *)value) != 0)
	{
		if (((struct in_addr *)value)->s_addr != 0)
		{
			strncpy(ip_info->geip, inet_ntoa(*((struct in_addr *)value)), 16);
			ip_info->geip[15] = '\0';
		}
		else
			ip_info->geip[0] = '\0';
	}	
	
	sprintf(ip_info->srcRange, "%s-%s", ip_info->lsip, ip_info->leip);
	sprintf(ip_info->globalRange, "%s-%s", ip_info->gsip, ip_info->geip);
	
	return 1;
}
#endif //end of !MULTI_ADDRESS_MAPPING
#endif

// Setup one NAT rule for a specfic interface
int startAddressMap(MIB_CE_ATM_VC_Tp pEntry)
{
	char wanif[6];
	char myip[16];
#ifdef ADDRESS_MAPPING
	ADDRESSMAP_IP_T ip_info;
#ifdef MULTI_ADDRESS_MAPPING
	MIB_CE_MULTI_ADDR_MAPPING_LIMIT_T		entry;
	int		i,entryNum;


#else // ! MULTI_ADDRESS_MAPPING
	char vChar;
	ADSMAP_T mapType;
	
	GetIP_AddressMap(&ip_info);
	
	mib_get( MIB_ADDRESS_MAP_TYPE,  (void *)&vChar);
        mapType = (ADSMAP_T)vChar;
#endif  //end of !MULTI_ADDRESS_MAPPING		
#endif

	if ( !pEntry->enable || ((DHCP_T)pEntry->ipDhcp != DHCP_DISABLED)
		|| ((REMOTE_ACCESS_T)pEntry->cmode == ACC_BRIDGED)    
		|| ((REMOTE_ACCESS_T)pEntry->cmode == ACC_PPPOE)
		|| ((REMOTE_ACCESS_T)pEntry->cmode == ACC_PPPOA)
		|| (!pEntry->napt)
		)
		return -1;
	
	snprintf(wanif, 6, "vc%u", VC_INDEX(pEntry->ifIndex));
	strncpy(myip, inet_ntoa(*((struct in_addr *)pEntry->ipAddr)), 16);
	myip[15] = '\0';

#ifdef ADDRESS_MAPPING
#ifdef MULTI_ADDRESS_MAPPING
	entryNum = mib_chain_total(MULTI_ADDRESS_MAPPING_LIMIT_TBL);

	for (i = 0; i<entryNum; i++)
	{
		mib_chain_get(MULTI_ADDRESS_MAPPING_LIMIT_TBL, i, &entry);
		GetIP_AddressMap(&entry, &ip_info);

		// add customized mapping
		if ( entry.addressMapType == ADSMAP_ONE_TO_ONE ) {				
			va_cmd(IPTABLES, 12, 1, "-t", "nat", FW_ADD, "POSTROUTING", "-s", ip_info.lsip,
				ARG_O, wanif, "-j", "SNAT", "--to-source", ip_info.gsip);					
			
		} 
		else if ( entry.addressMapType == ADSMAP_MANY_TO_ONE ) {
			va_cmd(IPTABLES, 14, 1, "-t", "nat", FW_ADD, "POSTROUTING", "-m", "iprange", "--src-range", ip_info.srcRange,
				ARG_O, wanif, "-j", "SNAT", "--to-source", ip_info.gsip);
			
		} 
		else if ( entry.addressMapType == ADSMAP_MANY_TO_MANY ) {
			va_cmd(IPTABLES, 14, 1, "-t", "nat", FW_ADD, "POSTROUTING", "-m", "iprange", "--src-range", ip_info.srcRange,
				ARG_O, wanif, "-j", "SNAT", "--to-source", ip_info.globalRange);
			
		}
		else if ( entry.addressMapType == ADSMAP_ONE_TO_MANY ) {
			va_cmd(IPTABLES, 12, 1, "-t", "nat", FW_ADD, "POSTROUTING", "-s", ip_info.lsip,
				ARG_O, wanif, "-j", "SNAT", "--to-source", ip_info.globalRange);			
		}		
		
	}
#else //!MULTI_ADDRESS_MAPPING
	// add customized mapping
	if ( mapType == ADSMAP_ONE_TO_ONE ) {				
		va_cmd(IPTABLES, 12, 1, "-t", "nat", FW_ADD, "POSTROUTING", "-s", ip_info.lsip,
			ARG_O, wanif, "-j", "SNAT", "--to-source", ip_info.gsip);					
		
	} 
	else if ( mapType == ADSMAP_MANY_TO_ONE ) {		
		va_cmd(IPTABLES, 14, 1, "-t", "nat", FW_ADD, "POSTROUTING", "-m", "iprange", "--src-range", ip_info.srcRange,
			ARG_O, wanif, "-j", "SNAT", "--to-source", ip_info.gsip);
		
	} 
	else if ( mapType == ADSMAP_MANY_TO_MANY ) {
		va_cmd(IPTABLES, 14, 1, "-t", "nat", FW_ADD, "POSTROUTING", "-m", "iprange", "--src-range", ip_info.srcRange,
			ARG_O, wanif, "-j", "SNAT", "--to-source", ip_info.globalRange);
		
	}
	// Mason Yu on True
#if 1
	else if ( mapType == ADSMAP_ONE_TO_MANY ) {
		va_cmd(IPTABLES, 12, 1, "-t", "nat", FW_ADD, "POSTROUTING", "-s", ip_info.lsip,
			ARG_O, wanif, "-j", "SNAT", "--to-source", ip_info.globalRange);
		
	}
#endif
#endif //end of !MULTI_ADDRESS_MAPPING	
#endif
	// add default mapping
	va_cmd(IPTABLES, 10, 1, "-t", "nat", FW_ADD, "POSTROUTING",
		ARG_O, wanif, "-j", "SNAT", "--to-source", myip);


}

// Delete one NAT rule for a specfic interface
void stopAddressMap(MIB_CE_ATM_VC_Tp pEntry)
{
	char *argv[16];
	char wanif[6];
	char myip[16];

#ifdef ADDRESS_MAPPING
	ADDRESSMAP_IP_T ip_info;
#ifdef MULTI_ADDRESS_MAPPING
	MIB_CE_MULTI_ADDR_MAPPING_LIMIT_T		entry;
	int		i,entryNum;


#else //MULTI_ADDRESS_MAPPING
	char vChar;
	ADSMAP_T mapType;
	
	GetIP_AddressMap(&ip_info);
	
	mib_get( MIB_ADDRESS_MAP_TYPE,  (void *)&vChar);
        mapType = (ADSMAP_T)vChar;
#endif //!MULTI_ADDRESS_MAPPING		
#endif

	if ( !pEntry->enable || ((DHCP_T)pEntry->ipDhcp != DHCP_DISABLED)
		|| ((REMOTE_ACCESS_T)pEntry->cmode == ACC_BRIDGED)    
		|| ((REMOTE_ACCESS_T)pEntry->cmode == ACC_PPPOE)
		|| ((REMOTE_ACCESS_T)pEntry->cmode == ACC_PPPOA)
		|| (!pEntry->napt)
		)
		return -1;
	
	snprintf(wanif, 6, "vc%u", VC_INDEX(pEntry->ifIndex));
	strncpy(myip, inet_ntoa(*((struct in_addr *)pEntry->ipAddr)), 16);
	myip[15] = '\0';
	
	if (pEntry->cmode != ACC_BRIDGED && pEntry->napt == 1)
	{	// Enable NAPT
		argv[1] = "-t";
		argv[2] = "nat";
		argv[3] = (char *)FW_DEL;
		argv[4] = "POSTROUTING";
		
		// remove default mapping
		argv[5] = "-o";
		argv[6] = wanif;
		argv[7] = "-j";
		if ((DHCP_T)pEntry->ipDhcp == DHCP_DISABLED) {
			strncpy(myip, inet_ntoa(*((struct in_addr *)pEntry->ipAddr)), 16);
			myip[15] = '\0';
			argv[8] = "SNAT";
			argv[9] = "--to-source";
			argv[10] = myip;
			argv[11] = NULL;
		}
		else {
			argv[8] = "MASQUERADE";
			argv[9] = NULL;
		}
		do_cmd(IPTABLES, argv, 1);

#ifdef ADDRESS_MAPPING
#ifdef MULTI_ADDRESS_MAPPING
		entryNum = mib_chain_total(MULTI_ADDRESS_MAPPING_LIMIT_TBL);

		for (i = 0; i<entryNum; i++)
		{			
			mib_chain_get(MULTI_ADDRESS_MAPPING_LIMIT_TBL, i, &entry);
			GetIP_AddressMap(&entry, &ip_info);
			
			// remove customized mapping
			if ( entry.addressMapType  == ADSMAP_ONE_TO_ONE ) {
				argv[5] = "-s";
				argv[6] = ip_info.lsip;
				argv[7] = "-o";
				argv[8] = wanif;			
				argv[9] = "-j";
				if ((DHCP_T)pEntry->ipDhcp == DHCP_DISABLED) {				
					argv[10] = "SNAT";
					argv[11] = "--to-source";
					argv[12] = ip_info.gsip;
					argv[13] = NULL;
				}
				else {
					argv[8] = "MASQUERADE";
					argv[9] = NULL;
				}			
				
			} else if ( entry.addressMapType  == ADSMAP_MANY_TO_ONE ) {		

				argv[5] = "-m";
				argv[6] = "iprange";
				argv[7] = "--src-range";
				argv[8] = ip_info.srcRange;	
				argv[9] = "-o";
				argv[10] = wanif;
				argv[11] = "-j";

				if ((DHCP_T)pEntry->ipDhcp == DHCP_DISABLED) {				
					argv[12] = "SNAT";
					argv[13] = "--to-source";
					argv[14] = ip_info.gsip;
					argv[15] = NULL;
				}
				else {
					argv[8] = "MASQUERADE";
					argv[9] = NULL;
				}			
				
			} else if ( entry.addressMapType  == ADSMAP_MANY_TO_MANY ) {			
				argv[5] = "-m";
				argv[6] = "iprange";
				argv[7] = "--src-range";
				argv[8] = ip_info.srcRange;	
				argv[9] = "-o";
				argv[10] = wanif;
				argv[11] = "-j";
				if ((DHCP_T)pEntry->ipDhcp == DHCP_DISABLED) {				
					argv[12] = "SNAT";
					argv[13] = "--to-source";
					argv[14] = ip_info.globalRange;
					argv[15] = NULL;
				}
				else {
					argv[8] = "MASQUERADE";
					argv[9] = NULL;
				}			
				
			} 
			
			else if ( entry.addressMapType  == ADSMAP_ONE_TO_MANY ) {
				argv[5] = "-s";
				argv[6] = ip_info.lsip;
				argv[7] = "-o";
				argv[8] = wanif;			
				argv[9] = "-j";			
				if ((DHCP_T)pEntry->ipDhcp == DHCP_DISABLED) {				
					argv[10] = "SNAT";
					argv[11] = "--to-source";
					argv[12] = ip_info.globalRange;
					argv[13] = NULL;
				}
				else {
					argv[8] = "MASQUERADE";
					argv[9] = NULL;
				}			
			}

			else
				return;
			TRACE(STA_SCRIPT, "%s %s %s %s %s ", IPTABLES, argv[1], argv[2], argv[3], argv[4]);
			TRACE(STA_SCRIPT, "%s %s %s %s\n", argv[5], argv[6], argv[7], argv[8]);		
			do_cmd(IPTABLES, argv, 1);
		}
#else //!MULTI_ADDRESS_MAPPING
		// remove customized mapping
		if ( mapType == ADSMAP_ONE_TO_ONE ) {
			argv[5] = "-s";
			argv[6] = ip_info.lsip;
			argv[7] = "-o";
			argv[8] = wanif;			
			argv[9] = "-j";
			if ((DHCP_T)pEntry->ipDhcp == DHCP_DISABLED) {				
				argv[10] = "SNAT";
				argv[11] = "--to-source";
				argv[12] = ip_info.gsip;
				argv[13] = NULL;
			}
			else {
				argv[8] = "MASQUERADE";
				argv[9] = NULL;
			}			
			
		} else if ( mapType == ADSMAP_MANY_TO_ONE ) {			
			argv[5] = "-m";
			argv[6] = "iprange";
			argv[7] = "--src-range";
			argv[8] = ip_info.srcRange;	
			argv[9] = "-o";
			argv[10] = wanif;
			argv[11] = "-j";
			if ((DHCP_T)pEntry->ipDhcp == DHCP_DISABLED) {				
				argv[12] = "SNAT";
				argv[13] = "--to-source";
				argv[14] = ip_info.gsip;
				argv[15] = NULL;
			}
			else {
				argv[8] = "MASQUERADE";
				argv[9] = NULL;
			}			
			
		} else if ( mapType == ADSMAP_MANY_TO_MANY ) {			
			argv[5] = "-m";
			argv[6] = "iprange";
			argv[7] = "--src-range";
			argv[8] = ip_info.srcRange;	
			argv[9] = "-o";
			argv[10] = wanif;
			argv[11] = "-j";
			if ((DHCP_T)pEntry->ipDhcp == DHCP_DISABLED) {				
				argv[12] = "SNAT";
				argv[13] = "--to-source";
				argv[14] = ip_info.globalRange;
				argv[15] = NULL;
			}
			else {
				argv[8] = "MASQUERADE";
				argv[9] = NULL;
			}			
			
		} 
		
		// Msason Yu on True
#if 1
		else if ( mapType == ADSMAP_ONE_TO_MANY ) {
			argv[5] = "-s";
			argv[6] = ip_info.lsip;
			argv[7] = "-o";
			argv[8] = wanif;			
			argv[9] = "-j";			
			if ((DHCP_T)pEntry->ipDhcp == DHCP_DISABLED) {				
				argv[10] = "SNAT";
				argv[11] = "--to-source";
				argv[12] = ip_info.globalRange;
				argv[13] = NULL;
			}
			else {
				argv[8] = "MASQUERADE";
				argv[9] = NULL;
			}			
		}
#endif
		else
			return;
		TRACE(STA_SCRIPT, "%s %s %s %s %s ", IPTABLES, argv[1], argv[2], argv[3], argv[4]);
		TRACE(STA_SCRIPT, "%s %s %s %s\n", argv[5], argv[6], argv[7], argv[8]);		
		do_cmd(IPTABLES, argv, 1);

#endif //end of !MULTI_ADDRESS_MAPPING
#endif // of ADDRESS_MAPPING
	}
	
}


// Config all NAT rules. 
// If action= ACT_STOP, delete all NAT rules.
// If action= ACT_START, setup all NAT rules.
void config_AddressMap(int action)
{
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	char wanif[6];
	char myip[16];
	
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);			
	for (i = 0; i < entryNum; i++) {				
		/* Retrieve entry */
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) {
			printf("restartAddressMap: cannot get ATM_VC_TBL(ch=%d) entry\n", i);
			return 0;
		}
		
		if (Entry.enable == 0)
			continue;
		if ( action == ACT_STOP )
			stopAddressMap(&Entry);
		else if ( action == ACT_START) {
			startAddressMap(&Entry);
		}
	}
	
	if ( action == ACT_START) {
		va_cmd("/bin/ethctl", 2, 1, "conntrack", "killall");
	}	
	
}

// IP interface: 1483-r or MER
// return value:
// 1  : successful
int startIP(char *inf, MIB_CE_ATM_VC_Tp pEntry, REMOTE_ACCESS_T ipEncap)
{
	char myip[16], remoteip[16], netmask[16];
	unsigned char buffer[7];
#ifdef IP_PASSTHROUGH
	unsigned char ippt_itf;
	int ippt=0;
	struct in_addr net;
	char netip[16];
#endif
#ifdef DEFAULT_GATEWAY_V2
	unsigned char dgw;
	int isdgw = 0;
#endif
	FILE *fp;
	// Mason Yu
	unsigned long broadcastIpAddr;	
	char broadcast[16];
	
	// Set MTU for 1483-r or MER
	sprintf(buffer, "%u", pEntry->mtu);
	va_cmd(IFCONFIG,3,1,inf, "mtu", buffer);
	
#ifdef IP_PASSTHROUGH
	// check IP passthrough
	if (mib_get(MIB_IPPT_ITF, (void *)&ippt_itf) != 0)
	{
		if (ippt_itf == pEntry->ifIndex)
			ippt = 1; // this interface enable the IP passthrough
	}
#endif
			
#ifdef DEFAULT_GATEWAY_V2
	// Jenny, check default gateway
	if (mib_get(MIB_ADSL_WAN_DGW_ITF, (void *)&dgw) != 0)
	{
		if (dgw == pEntry->ifIndex)
			isdgw = 1; // this interface is default gateway
	}
#endif

	if ((DHCP_T)pEntry->ipDhcp == DHCP_DISABLED)
	{
		// ifconfig vc0 ipaddr
		strncpy(myip, inet_ntoa(*((struct in_addr *)pEntry->ipAddr)), 16);
		myip[15] = '\0';
		strncpy(remoteip, inet_ntoa(*((struct in_addr *)pEntry->remoteIpAddr)), 16);
		remoteip[15] = '\0';
		strncpy(netmask, inet_ntoa(*((struct in_addr *)pEntry->netMask)), 16);
		netmask[15] = '\0';
		// Mason Yu
		broadcastIpAddr = ((struct in_addr *)pEntry->ipAddr)->s_addr | ~(((struct in_addr *)pEntry->netMask)->s_addr);
		strncpy(broadcast, inet_ntoa(*((struct in_addr *)&broadcastIpAddr)), 16);
		broadcast[15] = '\0';		
#ifdef CONFIG_ATM_CLIP
		if (ipEncap == ACC_ROUTED || ipEncap == ACC_IPOA)
#else
		if (ipEncap == ACC_ROUTED)
#endif
		{
#ifdef IP_PASSTHROUGH
			if (!ippt)
			{
#endif
				if (pEntry->ipunnumbered)	// Jenny, for IP unnumbered determination temporarily
					va_cmd(IFCONFIG, 8, 1, inf, "10.0.0.1", "-arp",
						"-broadcast", "pointopoint", "10.0.0.2",	
						"netmask", ARG_255x4);
				else
					// ifconfig vc0 myip -arp -broadcast pointopoint
					//   remoteip netmask 255.255.255.255
					va_cmd(IFCONFIG, 8, 1, inf, myip, "-arp",
						"-broadcast", "pointopoint", remoteip,
//						"netmask", netmask);	// Jenny, subnet mask added
						"netmask", ARG_255x4);
#ifdef IP_PASSTHROUGH
			}
			else	// IP passthrough
			{
				// ifconfig vc0 10.0.0.1 -arp -broadcast pointopoint
				//   10.0.0.2 netmask 255.255.255.255
				va_cmd(IFCONFIG, 8, 1, inf, "10.0.0.1", "-arp",
					"-broadcast", "pointopoint", "10.0.0.2",				// Jenny, for IP passthrough determination temporarily
					"netmask", netmask);
//				va_cmd(IFCONFIG, 8, 1, inf, "10.0.0.1", "-arp",
//					"-broadcast", "pointopoint", "10.0.0.2",
//					"netmask", ARG_255x4);
				// ifconfig br0:1 remoteip
				va_cmd(IFCONFIG, 2, 1, LAN_IPPT, remoteip);
				
				// Mason Yu. Add route for Public IP				
               			net.s_addr = (((struct in_addr *)pEntry->remoteIpAddr)->s_addr) & (((struct in_addr *)pEntry->netMask)->s_addr);
               			strncpy(netip, inet_ntoa(net), 16);
				netip[15] = '\0';				
				va_cmd(ROUTE, 7, 1, ARG_DEL, "-net", netip, "netmask", netmask, "dev", LANIF);
				va_cmd(ROUTE, 5, 1, ARG_ADD, "-host", myip, "dev", LANIF);
               			
				// write ip to file for DHCP server
				if (fp = fopen(IPOA_IPINFO, "w"))
				{
					fwrite( pEntry->ipAddr, 4, 1, fp);       	  
					fwrite( pEntry->remoteIpAddr, 4, 1, fp);      	  
					fwrite( pEntry->netMask, 4, 1, fp);      	  
					fclose(fp);
				}	
			}
#endif
#ifdef DEFAULT_GATEWAY_V2
			if (isdgw)
#else
			if (pEntry->dgw)
#endif
			{
#ifdef DEFAULT_GATEWAY_V2
				if (ifExistedDGW() == 1)	// Jenny, delete existed default gateway first
					va_cmd(ROUTE, 2, 1, ARG_DEL, "default");
#endif
				// route add default vc0
				va_cmd(ROUTE, 3, 1, ARG_ADD, "default", inf);
				//va_cmd(ROUTE, 4, 1, ARG_ADD, "default", "gw", remoteip);
			}
		}
		else
		{
			// Mason Yu. Set netmask and broadcast
			// ifconfig vc0 myip
			//va_cmd(IFCONFIG, 2, 1, inf, myip);
			va_cmd(IFCONFIG, 6, 1, inf, myip, "netmask", netmask, "broadcast",  broadcast);
			
#ifdef DEFAULT_GATEWAY_V2
			if (isdgw)
#else
			if (pEntry->dgw)
#endif
			{
#ifdef DEFAULT_GATEWAY_V2
					if (ifExistedDGW() == 1)	// Jenny, delete existed default gateway first
						va_cmd(ROUTE, 2, 1, ARG_DEL, "default");
#endif
				// route add default gw remotip
				va_cmd(ROUTE, 4, 1, ARG_ADD, "default", "gw", remoteip);
			}
			if (ipEncap == ACC_MER) {
				unsigned char value[32];
				snprintf(value, 32, "%s.%s", (char *)MER_GWINFO, inf);
				if (fp = fopen(value, "w"))
				{
					fprintf(fp, "%s\n", remoteip);
					fclose(fp);
				}
			}
		}
		
		if (pEntry->napt == 1)
		{	
			// Setup one NAT Rule for the specfic interface
			startAddressMap(pEntry);
		}
	}
	else
	{
		int dhcpc_pid;
		// Enabling support for a dynamically assigned IP (ISP DHCP)...
		va_cmd(IPTABLES, 16, 1, FW_ADD, FW_INPUT, ARG_I, inf, "-p",
			ARG_UDP, FW_DPORT, "69", "-d", ARG_255x4, "-m",
			"state", "--state", "NEW", "-j", FW_ACCEPT);
		
		if (fp = fopen(PROC_DYNADDR, "w"))
		{
			fprintf(fp, "1\n");
			fclose(fp);
		}
		else
		{
			printf("Open file %s failed !\n", PROC_DYNADDR);
		}
		
		dhcpc_pid = read_pid((char*)DHCPC_PID);
		if (dhcpc_pid > 0)
			kill(dhcpc_pid, SIGTERM);
		if (startDhcpc(inf, pEntry) == -1)
		{
			printf("start DHCP client failed !\n");
		}
		
		if (pEntry->napt == 1)
		{	// Enable NAPT
			va_cmd(IPTABLES, 8, 1, "-t", "nat", FW_ADD, "POSTROUTING",
				ARG_O, inf, "-j", "MASQUERADE");
		}
	}
	
	return 1;
}

#ifdef USE_PPPD
static const char PPPD_OPTIONS[] = "/etc/ppp/options";
static const char PPPD_PAPFILE[] = "/etc/ppp/pap-secrets";
static const char PPPD_CHAPFILE[] = "/etc/ppp/chap-secrets";
#endif
#ifdef CONFIG_USER_PPPOE_PROXY

static const char PPPD_PAPFILE[] = "/var/ppp/pap-secrets";

////alex_huang
static int has_pppoe_init;
 int ppp_proxy_ioctl(pppoe_proxy *if_pp_proxy,int cmd )
{
     
        int skfd, ret;  
        struct ifreq ifr;
       
        if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
 
                printf("ppp_proxy _ioctl error \n");
 
                return (-1);
 
        }

        strcpy(ifr.ifr_name, "ppp0");
        ifr.ifr_data= (char*)if_pp_proxy;
        ret = ioctl(skfd, cmd, &ifr);
        close(skfd);
  
        return ret;
}

#endif 

// Jenny, stop PPP
void stopPPP()
{
	int i;
	char tp[10];
	struct data_to_pass_st msg;
	for (i=0; i<MAX_PPP_NUM; i++) {
		sprintf(tp, "ppp%d", i);
		if (find_ppp_from_conf(tp)) {
			snprintf(msg.data, BUF_SIZE, "spppctl del %u", i);
			TRACE(STA_SCRIPT, "%s\n", msg.data);
			write_to_pppd(&msg);
		}
	}
}

// PPP connection
// Input: inf == "vc0","vc1", ...
// pppEncap: 0 : PPPoE, 1 : PPPoA
// return value:
// 1  : successful
int startPPP(char *inf, MIB_CE_ATM_VC_Tp pEntry, char *qos, REMOTE_ACCESS_T pppEncap)
{
	char ifIdx[3], pppif[6], stimeout[7];
#ifdef IP_PASSTHROUGH
	unsigned char ippt_itf;
	int ippt=0;
#endif
#ifdef USE_PPPD
	FILE *fp_options, *fp_pap, *fp_chap;
#else
	struct data_to_pass_st msg;
#endif
#ifdef DEFAULT_GATEWAY_V2
	unsigned char dgw;
	int isdgw = 0;
#endif
	int lastArg, pppinf, pppdbg = 0;
	
#ifdef IP_PASSTHROUGH
	// check IP passthrough
	if (mib_get(MIB_IPPT_ITF, (void *)&ippt_itf) != 0)
	{		
		if (ippt_itf == pEntry->ifIndex)
			ippt = 1; // this interface enable the IP passthrough
	}
#endif

#ifdef  ZTE_GENERAL_ROUTER_SC
         if(pEntry->ipunnumbered)
		 ippt=2;       //ip unnumbered alex
#endif
			
#ifdef DEFAULT_GATEWAY_V2
	// Jenny, check default gateway
	if (mib_get(MIB_ADSL_WAN_DGW_ITF, (void *)&dgw) != 0) {
#ifdef AUTO_PPPOE_ROUTE
		if (DGW_AUTO == dgw)
			isdgw = 1;
#endif
		if (dgw == pEntry->ifIndex)
			isdgw = 1; // this interface is default gateway
	}
#endif

	snprintf(ifIdx, 3, "%u", PPP_INDEX(pEntry->ifIndex));
	snprintf(pppif, 6, "ppp%u", PPP_INDEX(pEntry->ifIndex));
	if (pppEncap == ACC_PPPOE)	// PPPoE
	{
#ifdef USE_PPPD
		if ((fp_options = fopen(PPPD_OPTIONS, "w")) == NULL)
		{
			printf("Open file %s failed !\n", PPPD_OPTIONS);
			return -1;
		}
		
		if ((fp_pap = fopen(PPPD_PAPFILE, "w")) == NULL)
		{
			printf("Open file %s failed !\n", PPPD_PAPFILE);
			return -1;
		}
		
		if ((fp_chap = fopen(PPPD_CHAPFILE, "w")) == NULL)
		{
			printf("Open file %s failed !\n", PPPD_CHAPFILE);
			return -1;
		}
		
		fprintf(fp_options, "name \"%s\"\n", pEntry->pppUsername);
		fprintf(fp_pap, "\"%s\"	*	\"%s\"\n", pEntry->pppUsername, pEntry->pppPassword);
		fprintf(fp_chap, "\"%s\"	*	\"%s\"\n", pEntry->pppUsername, pEntry->pppPassword);

		fprintf(fp_options, "plugin pppoe\n");
		fprintf(fp_options, "tty %s\n", inf);
		fprintf(fp_options, "noipdefault\n");
		fprintf(fp_options, "hide-password\n");
		fprintf(fp_options, "defaultroute\n");
		fprintf(fp_options, "persist\n");
		fprintf(fp_options, "ipcp-accept-remote\n");
		fprintf(fp_options, "ipcp-accept-local\n");
		fprintf(fp_options, "nodetach\n");
		fprintf(fp_options, "usepeerdns\n");
		fprintf(fp_options, "lcp-echo-interval 20\n");
		fprintf(fp_options, "lcp-echo-failure 3\n");
		fprintf(fp_options, "lock\n");
		
		fclose(fp_options);
		fclose(fp_pap);
		fclose(fp_chap);

#else
 #ifdef CONFIG_USER_PPPOE_PROXY
              printf("enable pppoe proxy %d \n",pEntry->PPPoEProxyEnable);
              printf("maxuser = %d ...",pEntry->PPPoEProxyMaxUser);
             if(pEntry->PPPoEProxyEnable)
	    {
                FILE  *fp_pap;
                pppoe_proxy pp_proxy;
	         if ((fp_pap = fopen(PPPD_PAPFILE, "a+")) == NULL)
		  {
			printf("Open file %s failed !\n", PPPD_PAPFILE);
			return -1;
		  }
		  fprintf(fp_pap, "%s * \"%s\"  *\n", pEntry->pppUsername, pEntry->pppPassword);
		  fclose(fp_pap);
	      
		  if(!has_pppoe_init){
				pp_proxy.cmd =PPPOE_PROXY_ENABLE;
				ppp_proxy_ioctl(&pp_proxy,SIOCPPPOEPROXY);
				has_pppoe_init =1;
		   }	
		    pp_proxy.wan_unit =PPP_INDEX(pEntry->ifIndex) ;
		    pp_proxy.cmd =PPPOE_WAN_UNIT_SET;
		    strcpy(pp_proxy.user,pEntry->pppUsername);
		    strcpy(pp_proxy.passwd,pEntry->pppPassword);
		    pp_proxy.maxShareNum = pEntry->PPPoEProxyMaxUser;
		    ppp_proxy_ioctl(&pp_proxy,SIOCPPPOEPROXY);
    
         	    if (pEntry->napt == 1)
        	   {	// Enable NAPT
		      va_cmd(IPTABLES, 8, 1, "-t", "nat", FW_ADD, "POSTROUTING",
			 "-o", pppif, "-j", "MASQUERADE");
        	    }
	       
		       
		   	return 1;
             }
#endif

		if (pEntry->pppCtype != MANUAL){	// Jenny
			// spppctl add 0 pppoe vc0 username USER password PASSWORD
			//         gw 1 mru xxxx acname xxx
			snprintf(msg.data, BUF_SIZE,
				"spppctl add %s pppoe %s username %s password %s"
				" gw %d mru %d acname %s", ifIdx,
				inf, pEntry->pppUsername, pEntry->pppPassword,
#ifdef DEFAULT_GATEWAY_V2
				isdgw, pEntry->mtu, pEntry->pppACName);
#else
				pEntry->dgw, pEntry->mtu, pEntry->pppACName);
#endif
		}		
		else
			snprintf(msg.data, BUF_SIZE,
				"spppctl new %s pppoe %s username %s password %s"
				" gw %d mru %d acname %s", ifIdx,
				inf, pEntry->pppUsername, pEntry->pppPassword,
#ifdef DEFAULT_GATEWAY_V2
				isdgw, pEntry->mtu, pEntry->pppACName);
#else
				pEntry->dgw, pEntry->mtu, pEntry->pppACName);
#endif
#ifdef CONFIG_SPPPD_STATICIP
		// Jenny, set PPPoE static IP
		if (pEntry->pppIp) {
			unsigned long addr;
			addr = *((unsigned long *)pEntry->ipAddr);
			if (addr)
				snprintf(msg.data, BUF_SIZE, "%s staticip %x", msg.data, addr);
		}
#endif
#ifdef _CWMP_MIB_
		// Set Service Name
		if (strlen(pEntry->pppServiceName))
			snprintf(msg.data, BUF_SIZE, "%s servicename %s", msg.data, pEntry->pppServiceName);
#endif
#endif
	}
	else	// PPPoA
	{
#ifdef USE_PPPD
		// ...
#else
		if (pEntry->pppCtype != MANUAL)	// Jenny
			// spppctl add 0 pppoa vpi.vci encaps ENCAP qos xxx
			//         username USER password PASSWORD gw 1 mru xxxx
			snprintf(msg.data, BUF_SIZE,
				"spppctl add %s pppoa %u.%u encaps %d qos %s "
				"username %s password %s gw %d mru %d",
				ifIdx, pEntry->vpi, pEntry->vci, pEntry->encap,	qos,
#ifdef DEFAULT_GATEWAY_V2
				pEntry->pppUsername, pEntry->pppPassword, isdgw, pEntry->mtu);
#else
				pEntry->pppUsername, pEntry->pppPassword, pEntry->dgw, pEntry->mtu);
#endif
		else
			snprintf(msg.data, BUF_SIZE,
				"spppctl new %s pppoa %u.%u encaps %d qos %s "
				"username %s password %s gw %d mru %d",
				ifIdx, pEntry->vpi, pEntry->vci, pEntry->encap,	qos,
#ifdef DEFAULT_GATEWAY_V2
				pEntry->pppUsername, pEntry->pppPassword, isdgw, pEntry->mtu);
#else
				pEntry->pppUsername, pEntry->pppPassword, pEntry->dgw, pEntry->mtu);
#endif
#endif
	}
	
	// Set Authentication Method
	if ((PPP_AUTH_T)pEntry->pppAuth >= PPP_AUTH_PAP && (PPP_AUTH_T)pEntry->pppAuth <= PPP_AUTH_CHAP)
		snprintf(msg.data, BUF_SIZE, "%s auth %s", msg.data, ppp_auth[pEntry->pppAuth]);
	
#ifdef IP_PASSTHROUGH
	// Set IP passthrough
	snprintf(msg.data, BUF_SIZE, "%s ippt %d", msg.data, ippt);
#endif

	// set PPP debug
	pppdbg = pppdbg_get(PPP_INDEX(pEntry->ifIndex));
	snprintf(msg.data, BUF_SIZE, "%s debug %d", msg.data, pppdbg);

#ifdef _CWMP_MIB_
	// Set Auto Disconnect Timer
	if (pEntry->autoDisTime > 0)
		snprintf(msg.data, BUF_SIZE, "%s disctimer %d", msg.data, pEntry->autoDisTime);
	// Set Warn Disconnect Delay
	if (pEntry->warnDisDelay > 0)
		snprintf(msg.data, BUF_SIZE, "%s discdelay %d", msg.data, pEntry->warnDisDelay);
#endif
	
	if (pEntry->pppCtype == CONTINUOUS)	// Continuous
	{
#ifdef USE_PPPD
		va_cmd("/bin/pppd", 0, 0);
#else
		TRACE(STA_SCRIPT, "%s\n", msg.data);
//printf("\ncmd=%s\n",msg.data);
		write_to_pppd(&msg);
#endif
		// set the ppp keepalive timeout
		snprintf(msg.data, BUF_SIZE,
			"spppctl katimer 100");
		TRACE(STA_SCRIPT, "%s\n", msg.data);
		
		write_to_pppd(&msg);
		printf("PPP Connection (Continuous)...\n");
	}
	else if (pEntry->pppCtype == CONNECT_ON_DEMAND)	// On-demand
	{
#ifdef USE_PPPD

#else
		snprintf(msg.data, BUF_SIZE, "%s timeout %u", msg.data, pEntry->pppIdleTime);
		TRACE(STA_SCRIPT, "%s\n", msg.data);
		write_to_pppd(&msg);
#endif
		printf("PPP Connection (On-demand)...\n");
	}
	else if (pEntry->pppCtype == MANUAL)	// Manual
	{
#ifdef USE_PPPD

#else
//		TRACE(STA_SCRIPT|STA_NOTAG, "\n");
		// Jenny, for PPP connecting/disconnecting manually
		TRACE(STA_SCRIPT, "%s\n", msg.data);
		write_to_pppd(&msg);
#endif
		printf("PPP Connection (Manual)...\n");
	}
	
	if (pEntry->napt == 1)
	{	// Enable NAPT
		va_cmd(IPTABLES, 8, 1, "-t", "nat", FW_ADD, "POSTROUTING",
			"-o", pppif, "-j", "MASQUERADE");
	}
	
	return 1;
}

// find if pppif exists in /var/ppp/ppp.conf
int find_ppp_from_conf(char *pppif)
{
	char buff[256];
	FILE *fp;
	char strif[6];
	int found=0;
	
	if (!(fp=fopen(PPP_CONF, "r"))) {
		fclose(fp);
		printf("%s not exists.\n", PPP_CONF);
	}
	else {
		fgets(buff, sizeof(buff), fp);
		while( fgets(buff, sizeof(buff), fp) != NULL ) {
			if(sscanf(buff, "%s", strif)!=1) {
				found=0;
				printf("Unsuported ppp configuration format\n");
				break;
			}
			if ( !strcmp(pppif, strif) ) {
				found = 1;
				break;
			}
		}
		fclose(fp);
	}
	return found;
}

// find if vc exists in /proc/net/atm/pvc
static int find_pvc_from_running(int vpi, int vci)
{
	char buff[256];
	FILE *fp;
	int tvpi, tvci;
	int found=0;
	
	if (!(fp=fopen(PROC_NET_ATM_PVC, "r"))) {
		fclose(fp);
		printf("%s not exists.\n", PROC_NET_ATM_PVC);
	}
	else {
		fgets(buff, sizeof(buff), fp);
		while( fgets(buff, sizeof(buff), fp) != NULL ) {
			if(sscanf(buff, "%*d%d%d", &tvpi, &tvci)!=2) {
				found=0;
				printf("Unsuported pvc configuration format\n");
				break;
			}
			if ( tvpi==vpi && tvci==vci ) {
				found = 1;
				break;
			}
		}
		fclose(fp);
	}
	return found;
}

// calculate the 15-0(bits) Cell Rate register value (PCR or SCR)
// return its corresponding register value
static int cr2reg(int pcr)
{
#ifdef CONFIG_RTL8672
	return pcr;
#else
	int k, e, m, pow2, reg;
	
	k = pcr;
	e=0;
	
	while (k>1) {
		k = k/2;
		e++;
	}
	
	//printf("pcr=%d, e=%d\n", pcr,e);
	pow2 = 1;
	for (k = 1; k <= e; k++)
		pow2*=2;
	
	//printf("pow2=%d\n", pow2);
	//m = ((pcr/pow2)-1)*512;
	k = 0;
	while (pcr >= pow2) {
		pcr -= pow2;
		k++;
	}
	m = (k-1)*512 + pcr*512/pow2;
	//printf("m=%d\n", m);
	reg = (e<<9 | m );
	//printf("reg=%d\n", reg);
	return reg;
#endif
}

/*
 *	get the mark value for a traffic classification
 */
 
int _get_classification_mark( int entryNo, MIB_CE_IP_QOS_T *p )
{
	int mark=0;
	
	if(p==NULL) return 0;

	// mark the packet:  8-bits(high) |   8-bits(low)
	//                    class id    |  802.1p (if any)
	mark = ((entryNo+1) << 8);	// class id
	#if 0
	#ifdef QOS_SPEED_LIMIT_SUPPORT
	//use the first 3 bit
	if(p->limitSpeedEnabled)
		{
			int tmpmark=p->limitSpeedRank;
			
		  	mark|=tmpmark<<12;
			printf("limitSpeedRank=%d,mark=%d\n",p->limitSpeedRank,mark);
		}
	
	#endif
	#endif
	if (p->m_1p != 0)
		mark |= (p->m_1p-1);	// 802.1p

	return mark;
}

int get_classification_mark(int entryNo)
{
	MIB_CE_IP_QOS_T qEntry;
	int i, num, mark;
	
	mark = 0;
	num = mib_chain_total(MIB_IP_QOS_TBL);
	if (entryNo >= num)
		return 0;
	// get fwmark
	if (!mib_chain_get(MIB_IP_QOS_TBL, entryNo, (void *)&qEntry))
		return 0;
	
#if 1
	mark =  _get_classification_mark( entryNo, &qEntry );
#else
	// mark the packet:  8-bits(high) |   8-bits(low)
	//                    class id    |  802.1p (if any)
	mark = ((entryNo+1) << 8);	// class id
	if (qEntry.m_1p != 0)
		mark |= (qEntry.m_1p-1);	// 802.1p
#endif
	
	return mark;
}

/*
 *	generate the ifup_ppp(vc)x script for WAN interface
 */
static int generate_ifup_script(unsigned char ifIndex)
{
	int mark, ret;
	FILE *fp;
	char wanif[6], ifup_path[32];
#ifdef IP_POLICY_ROUTING
	int i, num, found;
	MIB_CE_IP_QOS_T qEntry;
#endif
	
	ret = 0;
	
	if (PPP_INDEX(ifIndex) != 0x0f) {
		// PPP interface
		
		snprintf(wanif, 6, "ppp%u", PPP_INDEX(ifIndex));
		snprintf(ifup_path, 32, "/var/ppp/ifup_%s", wanif);
		if (fp=fopen(ifup_path, "w+") ) {
			fprintf(fp, "#!/bin/sh\n\n");
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
			// Added by Mason Yu for Start upnpd.
			fprintf(fp, "/bin/upnpctrl down %s br0\n", wanif);
#endif
			
#ifdef CONFIG_USER_DDNS
			// Added by Mason Yu for Start updatedd.
			fprintf(fp, "/bin/updateddctrl %s\n", wanif);
#endif
#ifdef IP_POLICY_ROUTING
			num = mib_chain_total(MIB_IP_QOS_TBL);
			found = 0;
			// set advanced-routing rule
			for (i=0; i<num; i++) {
				if (!mib_chain_get(MIB_IP_QOS_TBL, i, (void *)&qEntry))
					continue;
#ifdef QOS_DIFFSERV
				if (qEntry.enDiffserv == 1) // Diffserv entry
					continue;
#endif
				if (qEntry.outif == ifIndex) {
					
					found = 1;
					mark = get_classification_mark(i);
					if (mark != 0) {
						// Don't forget to point out that fwmark with
						// ipchains/iptables is a decimal number, but that
						// iproute2 uses hexadecimal number.
						fprintf(fp, "ip ru add fwmark %x table %d\n", mark, PPP_INDEX(ifIndex)+PR_PPP_START);
					}
				}
			}
			if (found) {
				fprintf(fp, "ip ro add default dev %s table %d\n", wanif, PPP_INDEX(ifIndex)+PR_PPP_START);
			}
#endif
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
			// Added by Mason Yu for Start upnpd.
			fprintf(fp, "/bin/upnpctrl up %s br0\n", wanif);
#endif

			fclose(fp);
			chmod(ifup_path, 484);
		}
		else
			ret = -1;
	}
	else {
		// not supported till now
		return -1;
		// vc interface
		snprintf(wanif, 6, "vc%u", VC_INDEX(ifIndex));
	}
	
	return ret;
}

/*
 *	setup the policy-routing for static link
 */
#ifdef IP_POLICY_ROUTING
static int setup_static_PolicyRouting(unsigned char ifIndex)
{
	int i, num, mark, ret, found;
	char str_mark[8], str_rtable[8], ipAddr[32], wanif[8];
	struct in_addr inAddr;
	MIB_CE_IP_QOS_T qEntry;
	
	ret = -1;
	
	if (ifIndex == 0xff)
		return ret;
	else {
		if (PPP_INDEX(ifIndex) != 0x0f) {
			// PPP interface
			return ret;
		}
		else {
			// vc interface
			snprintf(wanif, 8, "vc%u", VC_INDEX(ifIndex));
			snprintf(str_rtable, 8, "%d", VC_INDEX(ifIndex)+PR_VC_START);
			if (getInAddr( wanif, DST_IP_ADDR, (void *)&inAddr) == 1)
			{
				strcpy(ipAddr, inet_ntoa(inAddr));
			}
			else
				return ret;
		}
		
		if (!getInFlags(wanif, 0))
			return ret;	// interface not found
	}
	
	num = mib_chain_total(MIB_IP_QOS_TBL);
	found = 0;
	// set advanced-routing rule
	for (i=0; i<num; i++) {
		if (!mib_chain_get(MIB_IP_QOS_TBL, i, (void *)&qEntry))
			continue;
#ifdef QOS_DIFFSERV
		if (qEntry.enDiffserv == 1) // Diffserv entry
			continue;
#endif
		if (qEntry.outif == ifIndex) {
			
			found = 1;
			mark = get_classification_mark(i);
			if (mark != 0) {
				snprintf(str_mark, 8, "%x", mark);
				// Don't forget to point out that fwmark with
				// ipchains/iptables is a decimal number, but that
				// iproute2 uses hexadecimal number.
				// ip ru add fwmark xxx table 3
				va_cmd("/bin/ip", 6, 1, "ru", "add", "fwmark", str_mark, "table", str_rtable);
				// ip ro add default via ipaddr dev vc0 table 3
			}
		}
	}
	if (found) {
		// ip ro add default via ipaddr dev vc0 table 3
		va_cmd("/bin/ip", 9, 1, "ro", "add", "default", "via", ipAddr, "dev", wanif, "table", str_rtable);
		ret = 0;
	}
	
	return ret;
}
#endif

//Inform kernel the number of routing interface.
// flag = 1 to increase 1
// flag = 0 to decrease 1
void internetLed_route_if(int flag)
{
	FILE *fp;

	fp = fopen ("/proc/IntnetLED", "w");
	if (fp)
	{
		if (flag)
			fprintf(fp,"+");
		else
			fprintf(fp,"-");
		fclose(fp);
	}
}

// return value:
// 0  : successful
// -1 : failed
#ifdef ZTE_GENERAL_ROUTER_SC
unsigned char mac_compensate=0;
#endif
int startConnection(MIB_CE_ATM_VC_Tp pEntry)
{
	char *argv[5];
	struct data_to_pass_st msg;
	char *aal5Encap, qosParms[64], wanif[5];
	int brpppoe;
	int pcreg, screg;
	int status=0;
	unsigned char upnpdEnable, upnpItf;
//#ifdef CONFIG_USER_UPNPD	
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
	char ext_ifname[6];
#endif	
	
	if(pEntry == NULL)
	{
		return 0;
	}

#if defined(ZTE_531B_BRIDGE_SC)||(ZTE_GENERAL_ROUTER_SC)
//star
	if(pEntry->enable==0)
		return 0;
#endif

	if (pEntry->cmode != ADSL_BR1483)
		internetLed_route_if(1);//+1

	// get the aal5 encapsulation
	if (pEntry->encap == ENCAP_VCMUX)
	{
#ifdef CONFIG_ATM_CLIP
		if ((REMOTE_ACCESS_T)pEntry->cmode == ACC_ROUTED || (REMOTE_ACCESS_T)pEntry->cmode == ACC_IPOA)
#else
		if ((REMOTE_ACCESS_T)pEntry->cmode == ACC_ROUTED)
#endif
			aal5Encap = (char *)VC_RT;
		else
			aal5Encap = (char *)VC_BR;
	}
	else	// LLC
	{
		if ((REMOTE_ACCESS_T)pEntry->cmode == ACC_ROUTED)
			aal5Encap = (char *)LLC_RT;
#ifdef CONFIG_ATM_CLIP
		else if ((REMOTE_ACCESS_T)pEntry->cmode == ACC_IPOA)
			aal5Encap = (char *)LLC_1577;
#endif
		else
			aal5Encap = (char *)LLC_BR;
	}
	
	// calculate the 15-0(bits) PCR CTLSTS value
	pcreg = cr2reg(pEntry->pcr);
	
	if ((ATM_QOS_T)pEntry->qos == ATMQOS_CBR)
	{
		snprintf(qosParms, 64, "cbr:pcr=%u", pcreg);
	}
	else if ((ATM_QOS_T)pEntry->qos == ATMQOS_VBR_NRT)
	{
		screg = cr2reg(pEntry->scr);
		snprintf(qosParms, 64, "nrt-vbr:pcr=%u,scr=%u,mbs=%u",
			pcreg, screg, pEntry->mbs);
	}
	else if ((ATM_QOS_T)pEntry->qos == ATMQOS_VBR_RT)
	{
		screg = cr2reg(pEntry->scr);
		snprintf(qosParms, 64, "rt-vbr:pcr=%u,scr=%u,mbs=%u",
			pcreg, screg, pEntry->mbs);
	}
	else //if ((ATM_QOS_T)pEntry->qos == ATMQOS_UBR)
	{
		snprintf(qosParms, 64, "ubr:pcr=%u", pcreg);
	}
	// Mason Yu Test
	printf("qosParms=%s\n", qosParms);	
	snprintf(wanif, 5, "vc%d", VC_INDEX(pEntry->ifIndex));
	
	// start this vc
	if (pEntry->cmode != ACC_PPPOA && !find_pvc_from_running(pEntry->vpi, pEntry->vci))
	{
		unsigned char devAddr[MAC_ADDR_LEN];
		char macaddr[MAC_ADDR_LEN*2+1];
		
		// mpoactl add vc0 pvc 0.33 encaps 4 qos xxx brpppoe x
#ifdef PPPOE_PASSTHROUGH			
		snprintf(msg.data, BUF_SIZE,
			"mpoactl add %s pvc %u.%u encaps %s qos %s brpppoe %d", wanif,
			pEntry->vpi, pEntry->vci, aal5Encap, qosParms, pEntry->brmode);			
#else
#ifdef CONFIG_ATM_CLIP
		if (pEntry->cmode != ACC_IPOA)
#endif
		snprintf(msg.data, BUF_SIZE,
			"mpoactl add %s pvc %u.%u encaps %s qos %s", wanif,
			pEntry->vpi, pEntry->vci, aal5Encap, qosParms);
#ifdef CONFIG_ATM_CLIP
		else
			snprintf(msg.data, BUF_SIZE,
				"mpoactl addclip %s pvc %u.%u encaps %s qos %s", wanif,
				pEntry->vpi, pEntry->vci, aal5Encap, qosParms);
#endif
#endif
		TRACE(STA_SCRIPT, "%s\n", msg.data);
		status|=write_to_mpoad(&msg);

#ifdef _LINUX_2_6_
		{
			int repeat_i=0;
			//make sure the channel is created (ifconfig hw ether may fail because the vc# is not ready)
			while( (repeat_i<10) && (!find_pvc_from_running(pEntry->vpi, pEntry->vci)) )
			{
				usleep(50000);
				repeat_i++;
			}
		}
#endif //_LINUX_2_6_
		
#ifdef CONFIG_ATM_CLIP
		if (mib_get(MIB_ELAN_MAC_ADDR, (void *)devAddr) != 0 && pEntry->cmode != ACC_IPOA)
#else
		if (mib_get(MIB_ELAN_MAC_ADDR, (void *)devAddr) != 0)
#endif
		{
//add by ramen for ZTE 531B router
#ifdef ZTE_GENERAL_ROUTER_SC
#if 0
if(pEntry->cmode!=ADSL_BR1483)
{
	if(pEntry->cmode==ADSL_PPPoE||pEntry->cmode==ADSL_PPPoA)
	snprintf(macaddr, 13, "%1x%1x%02x%02x%02x%02x%02x",
					pEntry->cmode,((pEntry->ifIndex&0xF0)>>2), devAddr[1], devAddr[2],
					devAddr[3], devAddr[4], devAddr[5]);
	else snprintf(macaddr, 13, "%1x%1x%02x%02x%02x%02x%02x",
					pEntry->cmode,((pEntry->ifIndex&0x0f)<<2), devAddr[1], devAddr[2],
					devAddr[3], devAddr[4], devAddr[5]);
	
}else	
snprintf(macaddr, 13, "%02x%02x%02x%02x%02x%02x",
				devAddr[0], devAddr[1], devAddr[2],
	
#endif
//add by ramen to borrow the mac address from WLAN
if(pEntry->cmode!=ADSL_BR1483)
 {
      if(mac_compensate<5)
			devAddr[5]+=mac_compensate++;	
}
	snprintf(macaddr, 13, "%02x%02x%02x%02x%02x%02x",
				devAddr[0], devAddr[1], devAddr[2],
				devAddr[3], devAddr[4], devAddr[5]);

#else
	snprintf(macaddr, 13, "%02x%02x%02x%02x%02x%02x",
				devAddr[0], devAddr[1], devAddr[2],
				devAddr[3], devAddr[4], devAddr[5]);
#endif
			argv[1]=wanif;
			argv[2]="hw";
			argv[3]="ether";
			argv[4]=macaddr;
			argv[5]=NULL;
			TRACE(STA_SCRIPT, "%s %s %s %s %s\n", IFCONFIG, argv[1], argv[2],
						argv[3], argv[4]);
			status|=do_cmd(IFCONFIG, argv, 1);
		}
		
		// ifconfig vc0 txqueuelen 10
		argv[1] = wanif;
		argv[2] = "txqueuelen";
                argv[3] = "10";
		argv[4] = NULL;
		TRACE(STA_SCRIPT, "%s %s %s\n", IFCONFIG, argv[1], argv[2], argv[3]);
		status|=do_cmd(IFCONFIG, argv, 1);
		// ifconfig vc0 up
		argv[2] = "up";
		argv[3] = NULL;
		TRACE(STA_SCRIPT, "%s %s %s\n", IFCONFIG, argv[1], argv[2]);
		status|=do_cmd(IFCONFIG, argv, 1);
	}
	
#ifdef CONFIG_EXT_SWITCH
		// mpoactl set vc0 vlan 1 pvid 1
		snprintf(msg.data, BUF_SIZE,
			"mpoactl set %s vlan %d vid %d", wanif,
			pEntry->vlan, pEntry->vid);
#endif
  		
	if (pEntry->cmode == ACC_BRIDGED)
	{
  	//shlee turns on DIRECT_BRIDGE_MODE only bridge mode pvc 
  	//Avoiding pppoe passthrough will enter direct bridge
	 	unsigned char ch_no[32];	
		sprintf(ch_no,"%d",VC_INDEX(pEntry->ifIndex));
  	va_cmd("/bin/sarctl",2,1,"direct_bridge",ch_no);	// Turns on direct bridge mode

		// rfc1483-bridged
		// brctl addif br0 vc0
		printf("1483 bridged\n");
		// mpoactl set vc0 vlan 1 pvid 1
		TRACE(STA_SCRIPT, "%s\n", msg.data);
		status|=write_to_mpoad(&msg);
		
		// Mason Yu
		if ( pEntry->brmode != BRIDGE_DISABLE )    
			status|=va_cmd(BRCTL, 3, 1, "addif", BRIF, wanif);      
	}
	else if ((REMOTE_ACCESS_T)pEntry->cmode == ACC_MER)
	{
		// MAC Encapsulated Routing
		printf("1483 MER\n");
		// mpoactl set vc0 vlan 1 pvid 1
		TRACE(STA_SCRIPT, "%s\n", msg.data);
		status|=write_to_mpoad(&msg);
		
		if (startIP(wanif, pEntry, ACC_MER) == -1)
		{
			printf("start MER failed !\n");
			status=-1;
		}
#ifdef PPPOE_PASSTHROUGH
		//if (pEntry->brmode == BRIDGE_PPPOE)	// enable PPPoE pass-through
		if (pEntry->brmode != BRIDGE_DISABLE)	// enable bridge
			// brctl addif br0 vc0
			status|=va_cmd(BRCTL, 3, 1, "addif", BRIF, wanif);
#endif
		if ( pEntry->ipDhcp == DHCP_DISABLED ) {
#ifdef IP_POLICY_ROUTING
			setup_static_PolicyRouting(pEntry->ifIndex);
#endif
		}
	}
	else if ((REMOTE_ACCESS_T)pEntry->cmode == ACC_PPPOE)
	{
		// PPPoE
		printf("PPPoE\n");
		// mpoactl set vc0 vlan 1 pvid 1
		TRACE(STA_SCRIPT, "%s\n", msg.data);
		status|=write_to_mpoad(&msg);

		if (startPPP(wanif, pEntry, 0, ACC_PPPOE) == -1)
		{
			printf("start PPPoE failed !\n");
			status=-1;
		}
		
#ifdef PPPOE_PASSTHROUGH
		//if (pEntry->brmode == BRIDGE_PPPOE)	// enable PPPoE pass-through
		if (pEntry->brmode != BRIDGE_DISABLE)	// enable bridge
			// brctl addif br0 vc0
			status|=va_cmd(BRCTL, 3, 1, "addif", BRIF, wanif);
#endif
	}
	else if ((REMOTE_ACCESS_T)pEntry->cmode == ACC_PPPOA)
	{
		// PPPoA
		printf("PPPoA\n");
		if (startPPP(wanif, pEntry, qosParms, ACC_PPPOA) == -1)
		{
			printf("start PPPoA failed !\n");
			status=-1;
		}
	}
	else if ((REMOTE_ACCESS_T)pEntry->cmode == ACC_ROUTED)
	{
		// rfc1483-routed
		printf("1483 routed\n");
		if (startIP(wanif, pEntry, ACC_ROUTED) == -1)
		{
			printf("start 1483-routed failed !\n");
			status=-1;
		}
#ifdef IP_POLICY_ROUTING
		setup_static_PolicyRouting(pEntry->ifIndex);
#endif
	}
#ifdef CONFIG_ATM_CLIP
	else if ((REMOTE_ACCESS_T)pEntry->cmode == ACC_IPOA)
	{
		// rfc1577-routed
		printf("1577 routed\n");
		unsigned long addr;
		struct data_to_pass_st msg;
		if (startIP(wanif, pEntry, ACC_IPOA) == -1)
		{
			printf("start 1577-routed failed !\n");
			status=-1;
		}
		addr = *((unsigned long *)pEntry->ipAddr);
		snprintf(msg.data, BUF_SIZE, "mpoactl set %s cip %lu", wanif, addr);
		write_to_mpoad(&msg);
	}
#endif
#ifdef CONFIG_RTL8672	
	if(pEntry->cmode != ACC_BRIDGED)
	{	
		unsigned char cmode[4], vpi[4], vci[4];                
//		printf("ch %d (vc_index), cmode %d (%d. %d)\n", VC_INDEX(pEntry->ifIndex), pEntry->cmode, pEntry->vpi, pEntry->vci);
                sprintf(cmode, "%d", pEntry->cmode);
		//transfer vpi,vci for setting PPPoA correct CH index 
		sprintf(vpi, "%d", pEntry->vpi);
		sprintf(vci, "%d", pEntry->vci);
		va_cmd("/bin/sarctl",4,1,"set_header", cmode,vpi,vci);	
	}
	va_cmd("/bin/sarctl",1,1,"set_pkta");
#endif //CONFIG_RTL8672

// Magician: UPnP Daemon Start
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
	if(mib_get(MIB_UPNP_DAEMON, (void *)&upnpdEnable) && upnpdEnable)
	{
		if (mib_get(MIB_UPNP_EXT_ITF, (void *)&upnpItf) && upnpItf == pEntry->ifIndex)
		{
			if (PPP_INDEX(upnpItf) != 0x0f)
				snprintf(ext_ifname, 6, "ppp%u", PPP_INDEX(upnpItf));	 // PPP interface
			else
				snprintf(ext_ifname, 5, "vc%u", VC_INDEX(upnpItf));

			va_cmd("/bin/upnpctrl", 3, 1, "down", ext_ifname, "br0");
			va_cmd("/bin/upnpctrl", 3, 1, "up", ext_ifname, "br0");
		}
	}
#endif
// The end of UPnP Daemon Start

	if (status >= 0)
		generate_ifup_script(pEntry->ifIndex);
	
	return status;
}

#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
#define DHCPCLIENTPID  "/var/run/udhcpc.pid"
#endif

void stopConnection(MIB_CE_ATM_VC_Tp pEntry)
{
	struct data_to_pass_st msg;
	char wanif[6];
	char *argv[16];
	char myip[16];
	int i;
	
	// delete this vc
	if (pEntry->cmode == ACC_PPPOE || pEntry->cmode == ACC_PPPOA)
	{
		// spppctl del 0
		snprintf(msg.data, BUF_SIZE,
			"spppctl del %u", PPP_INDEX(pEntry->ifIndex));
		snprintf(wanif, 6, "ppp%u", PPP_INDEX(pEntry->ifIndex));
		TRACE(STA_SCRIPT, "%s\n", msg.data);
		write_to_pppd(&msg);
	}
	else
		snprintf(wanif, 6, "vc%u", VC_INDEX(pEntry->ifIndex));
	
	sleep(1);
	va_cmd(BRCTL, 3, 1, "delif", BRIF, wanif);   

	if (pEntry->cmode != ADSL_BR1483)
		internetLed_route_if(0);//-1

#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
//star: for dhcpc stop
	if(pEntry->cmode == ACC_MER && pEntry->ipDhcp != DHCP_DISABLED)
	{
		int dhcpcpid;
		unsigned char infname[6],pidfile[32];
		snprintf(infname, 6, "vc%u", VC_INDEX(pEntry->ifIndex));
		snprintf(pidfile, 32, "/var/run/udhcpc%s.pid", infname);
		dhcpcpid = read_pid(pidfile);

//		printf("\npidfile=%s,dhcpcpid=%d\n",pidfile,dhcpcpid);

		if(dhcpcpid > 0)
			kill(dhcpcpid, 15);
	}
#endif
#if 0
#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
//star: for multi-ppp
	int found=0;
	if (pEntry->cmode == ACC_PPPOE || pEntry->cmode == ACC_PPPOA)
	{
		int mibtotal,i;
		MIB_CE_ATM_VC_T tmpentry;
		mibtotal=mib_chain_total(MIB_ATM_VC_TBL);
		for(i=0;i<mibtotal;i++)
		{
					if(!mib_chain_get(MIB_ATM_VC_TBL,i,(void*)&tmpentry))
						continue;
					if(tmpentry.vpi==pEntry->vpi&&tmpentry.vci==pEntry->vci&&(tmpentry.cmode==ADSL_PPPoE||tmpentry.cmode==ADSL_PPPoA)
						&&(tmpentry.ifIndex!=pEntry->ifIndex))
					{
						found=1;
						break;
					}		
		}	
	}
#endif
#endif
	if ((pEntry->cmode != ACC_PPPOA)
#if 0
#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
	   &&found==0
#endif
#endif
	   )
	{
		// ifconfig vc0 down
		va_cmd(IFCONFIG, 2, 1, wanif, "down");
		// wait for sar to process the queueing packets
		//usleep(1000);
		for (i=0; i<10000000; i++);
		
		// mpoactl del vc0
		snprintf(msg.data, BUF_SIZE,
			"mpoactl del vc%u", VC_INDEX(pEntry->ifIndex));
		TRACE(STA_SCRIPT, "%s\n", msg.data);
		write_to_mpoad(&msg);
		// make sure this vc been deleted completely
		while (find_pvc_from_running(pEntry->vpi, pEntry->vci));
	}
	else {
		va_cmd(IFCONFIG, 2, 1, wanif, "down");
		for (i=0; i<10000000; i++);
		while (find_pvc_from_running(pEntry->vpi, pEntry->vci));
	}

	// delete one NAT rule for the specific interface
	stopAddressMap(pEntry);

}

#ifdef WLAN_SUPPORT
/////////////////////////////////////////////////////////////////////////////
static inline int
iw_get_ext(int                  skfd,           /* Socket to the kernel */
           char *               ifname,         /* Device name */
           int                  request,        /* WE ID */
           struct iwreq *       pwrq)           /* Fixed part of the request */
{
  /* Set device name */
  strncpy(pwrq->ifr_name, ifname, IFNAMSIZ);
  /* Do the request */
  return(ioctl(skfd, request, pwrq));
}

int getWlStaInfo( char *interface,  WLAN_STA_INFO_Tp pInfo )
{
    int skfd;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0)
    {
    	close( skfd );
      /* If no wireless name : no wireless extensions */
        return -1;
    }

    wrq.u.data.pointer = (caddr_t)pInfo;
    wrq.u.data.length = sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLSTAINFO, &wrq) < 0)
    {
    	close( skfd );
	return -1;
    }

    close( skfd );

    return 0;
}

#ifdef WLAN_ACL
// return value:
// 0  : successful
// -1 : failed
int set_wlan_acl()
{
	char *argv[8];
	unsigned char value[32];
	char parm[64];
	int num, i;
	MIB_CE_WLAN_AC_T Entry;
	int status=0;
	
	argv[1] = (char*)WLANIF;
	argv[2] = "set_mib";
	
	// aclnum=0
	sprintf(parm, "aclnum=0");
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
	
	// aclmode
	mib_get(MIB_WLAN_AC_ENABLED, (void *)value);
	sprintf(parm, "aclmode=%u", value[0]);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
	
	if (value[0] == 0) // ACL disabled
		return status;
	
	if ((num = mib_chain_total(MIB_WLAN_AC_TBL)) == 0)
		return status;
	
	for (i=0; i<num; i++) {
		if (!mib_chain_get(MIB_WLAN_AC_TBL, i, (void *)&Entry))
			return;
		
		// acladdr
		sprintf(parm, "acladdr=%.2x%.2x%.2x%.2x%.2x%.2x",
			Entry.macAddr[0], Entry.macAddr[1], Entry.macAddr[2],
			Entry.macAddr[3], Entry.macAddr[4], Entry.macAddr[5]);
		argv[3] = parm;
		argv[4] = NULL;
		TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
		status|=do_cmd(IWPRIV, argv, 1);
	}
	return status;
}
#endif

#ifdef WLAN_8185AG
#define RTL8185_IOCTL_READ_EEPROM	0x89f9
static int check_wlan_eeprom()
{
	int skfd,i;
	struct iwreq wrq;
	unsigned char tmp[162];
	char mode;
	char parm[64];
	char *argv[6];
	argv[1] = (char*)WLANIF;
	argv[2] = "set_mib";
	
	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	/* Set device name */
	strncpy(wrq.ifr_name, (char *)WLANIF, IFNAMSIZ);
	strcpy(tmp,"RFChipID");
	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = 10;
	ioctl(skfd, RTL8185_IOCTL_READ_EEPROM, &wrq);
	if(wrq.u.data.length>0){
		printf("read eeprom success!\n");
		//return 1;
	}
	else{
		printf("read eeprom fail!\n");
		if(skfd!=-1) close(skfd);
		return 0;
	}
	//set TxPowerCCK from eeprom
	strcpy(tmp,"TxPowerCCK");
	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = 20;
	ioctl(skfd, RTL8185_IOCTL_READ_EEPROM, &wrq);
	sprintf(parm, "TxPowerCCK=");
	if ( mib_get( MIB_TX_POWER, (void *)&mode) == 0) {
   		printf("Get MIB_TX_POWER error!\n");		
	}

//added by xl_yue: 
//#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
#ifdef WLAN_TX_POWER_DISPLAY
	if ( mode==0 ) {          // 100%
		for(i=0; i<=13; i++)
		{
	//		value[i] = 8;
			sprintf(parm, "%s%02x", parm, wrq.u.data.pointer[i]);
		}
	}else if ( mode==1 ) {    // 80%
		for(i=0; i<=13; i++)
		{
		    	wrq.u.data.pointer[i] -= 1;
			sprintf(parm, "%s%02x", parm, wrq.u.data.pointer[i]);
		}

	}else if ( mode==2 ) {    // 50%
		for(i=0; i<=13; i++)
		{
		    	wrq.u.data.pointer[i] -= 3;
			sprintf(parm, "%s%02x", parm, wrq.u.data.pointer[i]);
		}
	}else if ( mode==3 ) {    // 25%
		for(i=0; i<=13; i++)
		{
		   	wrq.u.data.pointer[i] -= 6;
			sprintf(parm, "%s%02x", parm, wrq.u.data.pointer[i]);
		}
	}else if ( mode==4 ) {    // 10%
		for(i=0; i<=13; i++)
		{
		   	wrq.u.data.pointer[i] -= 10;
			sprintf(parm, "%s%02x", parm, wrq.u.data.pointer[i]);
		}
	}	
#else
	if ( mode==2 ) {          // 60mW
		for(i=0; i<=13; i++)
		{
	//		value[i] = 8;
			sprintf(parm, "%s%02x", parm, wrq.u.data.pointer[i]);
		}	
	}else if ( mode==1 ) {    // 30mW
		for(i=0; i<=13; i++)
		{
		    	wrq.u.data.pointer[i] -= 3;
			sprintf(parm, "%s%02x", parm, wrq.u.data.pointer[i]);
		}
	}else if ( mode==0 ) {    // 15mW
		for(i=0; i<=13; i++)
		{
		   	wrq.u.data.pointer[i] -= 6;
			sprintf(parm, "%s%02x", parm, wrq.u.data.pointer[i]);
		}
	}	
#endif
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	do_cmd(IWPRIV, argv, 1);	

	strcpy(tmp,"TxPowerOFDM");
	wrq.u.data.pointer = (caddr_t)&tmp;
	wrq.u.data.length = 20;
	ioctl(skfd, RTL8185_IOCTL_READ_EEPROM, &wrq);
	sprintf(parm, "TxPowerOFDM=");
	if ( mib_get( MIB_TX_POWER, (void *)&mode) == 0) {
   		printf("Get MIB_TX_POWER error!\n");		
	}

//added by xl_yue: 
//#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
#ifdef WLAN_TX_POWER_DISPLAY
	if ( mode==0 ) {          // 100%
		for(i=0; i<=13; i++)
		{
	//		value[i] = 8;
			sprintf(parm, "%s%02x", parm, wrq.u.data.pointer[i]);
		}	
	}else if ( mode==1 ) {    // 80%
		for(i=0; i<=13; i++)
		{
		    	wrq.u.data.pointer[i] -= 1;
			sprintf(parm, "%s%02x", parm, wrq.u.data.pointer[i]);
		}
	}else if ( mode==2 ) {    // 50%
		for(i=0; i<=13; i++)
		{
		    	wrq.u.data.pointer[i] -= 3;
			sprintf(parm, "%s%02x", parm, wrq.u.data.pointer[i]);
		}
	}else if ( mode==3 ) {    // 25%
		for(i=0; i<=13; i++)
		{
		   	wrq.u.data.pointer[i] -= 6;
			sprintf(parm, "%s%02x", parm, wrq.u.data.pointer[i]);
		}
	}else if ( mode==4 ) {    // 10%
		for(i=0; i<=13; i++)
		{
		   	wrq.u.data.pointer[i] -= 10;
			sprintf(parm, "%s%02x", parm, wrq.u.data.pointer[i]);
		}
	}
#else
	if ( mode==2 ) {          // 60mW
		for(i=0; i<=13; i++)
		{
	//		value[i] = 8;
			sprintf(parm, "%s%02x", parm, wrq.u.data.pointer[i]);
		}	
	}else if ( mode==1 ) {    // 30mW
		for(i=0; i<=13; i++)
		{
		    	wrq.u.data.pointer[i] -= 3;
			sprintf(parm, "%s%02x", parm, wrq.u.data.pointer[i]);
		}
	}else if ( mode==0 ) {    // 15mW
		for(i=0; i<=13; i++)
		{
		   	wrq.u.data.pointer[i] -= 6;
			sprintf(parm, "%s%02x", parm, wrq.u.data.pointer[i]);
		}
	}
#endif
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	do_cmd(IWPRIV, argv, 1);

	close( skfd );
	return 1;
}
#endif

int setupWDS()
{
#ifdef WLAN_WDS
	char *argv[6];
	unsigned char value[16];
	char macaddr[16];
	char vChar, wds_enabled;
	char parm[32];
	char wds_num;
	WDS_T Entry;
	char wdsif[10];
	int i;
	int status = 0;
	
	argv[1] = (char*)WLANIF;
	argv[2] = "set_mib";
	
	mib_get(MIB_WLAN_MODE, (void *)&vChar);
	mib_get(MIB_WLAN_WDS_ENABLED, (void *)&wds_enabled);
	if (vChar != AP_WDS_MODE || wds_enabled == 0) {
		argv[3] = "wds_num=0";
		argv[4] = NULL;
		TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
		status|=do_cmd(IWPRIV, argv, 1);
	
		sprintf(parm, "wds_enable=0");
		argv[3] = parm;
		argv[4] = NULL;
		TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
		status|=do_cmd(IWPRIV, argv, 1);
		
		for (i=0; i<MAX_WDS_NUM; i++) {
			strcpy(wdsif,(char *)WDSIF);
			wdsif[9]+=i;
			//ifconfig wlanX-wdsX down
			status|=va_cmd(IFCONFIG, 2, 1, wdsif, "down");
			//brctl delif br0 wlanX-wdsX
			status|=va_cmd(BRCTL, 3, 1, "delif", (char *)BRIF, wdsif);
		}
		return 0;
	}
	
	// wds_pure
	vChar = 0;	// AP + WDS
	sprintf(parm, "wds_pure=%u", vChar);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
	
	// wds_enable
	mib_get(MIB_WLAN_WDS_ENABLED, (void *)&vChar);
	sprintf(parm, "wds_enable=%u", vChar);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
		
	argv[3] = "wds_num=0";
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
	
	//mib_get(MIB_WLAN_WDS_ENABLED, (void *)&vChar);
	//if(vChar==1){
		for (i=0; i<MAX_WDS_NUM; i++) {
			strcpy(wdsif,(char *)WDSIF);
			wdsif[9]+=i;
			//ifconfig wlanX-wdsX down
			status|=va_cmd(IFCONFIG, 2, 1, wdsif, "down");
			//brctl delif br0 wlanX-wdsX
			status|=va_cmd(BRCTL, 3, 1, "delif", (char *)BRIF, wdsif);
		}
	//}
		
	mib_get(MIB_WLAN_WDS_NUM, &wds_num);
	
	strcpy(wdsif,(char *)WDSIF);
	for(i=0;i<wds_num;i++){
		if (!mib_chain_get(MIB_WDS_TBL, i, (void *)&Entry))
			continue;
		sprintf(parm, "wds_add=%02x%02x%02x%02x%02x%02x", 
			Entry.macAddr[0], Entry.macAddr[1], Entry.macAddr[2], 
			Entry.macAddr[3], Entry.macAddr[4], Entry.macAddr[5]);
		argv[3] = parm;
		argv[4] = NULL;
		TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
		status|=do_cmd(IWPRIV, argv, 1);	
		//ifconfig wlanX-wdsX hw ether 00e04c867001
		mib_get(MIB_ELAN_MAC_ADDR, (void *)value);
		snprintf(macaddr, 13, "%02x%02x%02x%02x%02x%02x",
			value[0], value[1], value[2], value[3], value[4], value[5]);		
		wdsif[9]='0'+i;
		status|=va_cmd(IFCONFIG, 4, 1, wdsif, "hw", "ether", macaddr);

		//brctl delif br0 wlanX-wdsX
		//va_cmd(BRCTL, 3, 1, "delif", (char *)BRIF, wdsif);
		//brctl addif br0 wlanX-wdsX
		status|=va_cmd(BRCTL, 3, 1, "addif", (char *)BRIF, wdsif);

		//ifconfig wlanX-wdsX up
		status|=va_cmd(IFCONFIG, 2, 1, wdsif, "up");
	}
	
	// wds_encrypt
	vChar = 0;
	sprintf(parm, "wds_encrypt=%u", vChar);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);

	vChar = 1;
	sprintf(parm, "wds_priority=%u", vChar);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
	
	//for MOD multicast-filter
	mib_get(MIB_WLAN_WDS_ENABLED, (void *)&vChar);
	if (vChar) {
		//brctl clrfltrport br0
		va_cmd(BRCTL,2,1,"clrfltrport",(char *)BRIF);
		//brctl setfltrport br0 11111
		va_cmd(BRCTL,3,1,"setfltrport",(char *)BRIF,"11111");
		//brctl setfltrport br0 55555
		va_cmd(BRCTL,3,1,"setfltrport",(char *)BRIF,"55555");
		//brctl setfltrport br0 2105
		va_cmd(BRCTL,3,1,"setfltrport",(char *)BRIF,"2105");
		//brctl setfltrport br0 2105
		va_cmd(BRCTL,3,1,"setfltrport",(char *)BRIF,"2107");
	}
	
	return status;
#else
	return 0;
#endif
}

//added by xl_yue
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
int isAllZero(unsigned char * buffer, int len)
{
	int i;
	for(i=0;i<len;i++)
	{
		if(buffer[i] != 0)
			return 0;
	}

	return 1;
}
#endif

// return value:
// 0  : success
// -1 : failed
int setupWLan()
{
	char *argv[6];
	unsigned char value[34];
	char parm[64], para2[15];
	int i, vInt, autoRate;
	// Added by Mason Yu for Set TxPower
	char mode=-1;
	int status=0;
	unsigned char stringbuf[MAX_SSID_LEN + 1];
	unsigned char rootSSID[MAX_SSID_LEN + 1];
#ifdef WLAN_MBSSID
	int j=0;
	MIB_CE_MBSSIB_T Entry;
	MIB_CE_MBSSIB_WEP_T EntryWEP;
#endif
	
#ifdef WLAN_QoS
	MIB_WLAN_QOS_T QOSEntry;
#endif
	argv[1] = (char*)WLANIF;
	argv[2] = "set_mib";
	// regdomain
//added by xl_yue
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	mib_get(MIB_WLAN_REG_DOMAIN, (void *)value);
#else
	mib_get(MIB_HW_REG_DOMAIN, (void *)value);
#endif
	sprintf(parm, "regdomain=%u", value[0]);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
#ifndef CONFIG_USB_RTL8187SU_SOFTAP	
#ifndef CONFIG_USB_RTL8192SU_SOFTAP
	// led_type
	mib_get(MIB_HW_LED_TYPE, (void *)value);
	sprintf(parm, "led_type=%u", value[0]);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
#endif
#endif
	// Root AP
//added by xl_yue
#ifdef ZTE_531B_BRIDGE_SC
	if (mib_get(MIB_ELAN_MAC_ADDR, (void *)value) != 0)
	{
		memset(stringbuf, 0, sizeof(stringbuf));
		if(mib_get(MIB_WLAN_SSID, (void*)stringbuf))
		{
			printf("get root ap ssid= %s\n", stringbuf);
			if(strlen(stringbuf)==0)
			{	
				value[0] = value[0]^value[2]^value[4];
				value[1] = value[1]^value[3]^value[5];
				snprintf((char *)(stringbuf), 9, "CTC-%02x%02x",value[0]+0xaa, value[1]+0xaa);
				if ( mib_set(MIB_WLAN_SSID, (void *)stringbuf ) == 0) 
				{
					printf("set root ssid error!\n");
				}
			}
			strcpy(rootSSID,stringbuf);	
		}
	}
#endif

#ifdef ZTE_GENERAL_ROUTER_SC
	if (mib_get(MIB_ELAN_MAC_ADDR, (void *)value) != 0)
	{
		memset(stringbuf, 0, sizeof(stringbuf));
		if(mib_get(MIB_WLAN_SSID, (void*)stringbuf))
		{
			printf("get root ap ssid= %s\n", stringbuf);
			if(strlen(stringbuf)==0)
			{	
				value[3] ^= value[0];
				value[4] ^= value[1];
				value[5] ^= value[2];
				snprintf((char *)(stringbuf), 12, "531B-%02x%02x%02x",value[3],value[4], value[5]);
				if ( mib_set(MIB_WLAN_SSID, (void *)stringbuf ) == 0) 
				{
					printf("set root ssid error!\n");
				}
			}
			strcpy(rootSSID,stringbuf);	
		}
	}
#endif

	// ssid
	// Modified by Mason Yu
	// Root AP's SSID
	mib_get(MIB_WLAN_SSID, (void *)value);
	sprintf(parm, "ssid=%s", value);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);

#ifdef WLAN_MBSSID	
	// VAP's SSID
	for (j=1; j<=4; j++) {
		if (!mib_chain_get(MIB_MBSSIB_TBL, j, (void *)&Entry)) {  		
  			printf("Error! Get MIB_MBSSIB_TBL for VAP SSID error.\n");					
		}

//added by xl_yue: for createing random virtual SSID
#ifdef ZTE_531B_BRIDGE_SC
		if (mib_get(MIB_ELAN_MAC_ADDR, (void *)value) != 0)
		{
			if(strlen(Entry.ssid)==0)
			{
				printf("change virtual ssid%d from default: %s to ",j,Entry.ssid);
				value[0] = value[0]^value[2]^value[4];
				value[1] = value[1]^value[3]^value[5] ^ ((unsigned char)j);
				//snprintf((char *)(Entry.ssid), 9, "CTC-%02x%02x",value[0], value[1]);
				sprintf(Entry.ssid,"%s%d",rootSSID,j+1);
				printf("random: %s\n",Entry.ssid);
				mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, j);
			}
		}
#endif
#ifdef ZTE_GENERAL_ROUTER_SC
		if (mib_get(MIB_ELAN_MAC_ADDR, (void *)value) != 0)
		{
			if(strlen(Entry.ssid)==0)
			{
				printf("change virtual ssid%d from default: %s to ",j,Entry.ssid);
				value[5] = value[5] +((unsigned char)j);
				//snprintf((char *)(Entry.ssid), 12, "531B-%02x%02x%02x",value[3], value[4],value[5]);
				sprintf(Entry.ssid,"%s%d",rootSSID,j+1);
				printf("random: %s\n",Entry.ssid);
				mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, j);
			}
		}
#endif
 		//if ( Entry.wlanDisabled == 1 ) {
			sprintf(para2, "wlan0-vap%d", j-1);
			sprintf(parm, "ssid=%s", Entry.ssid);
			argv[1] = para2;
			argv[3] = parm;
			argv[4] = NULL;
			TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
			status|=do_cmd(IWPRIV, argv, 1);	
		//}		
	}
	argv[1] = (char*)WLANIF;
#endif	

	// opmode
	mib_get(MIB_WLAN_MODE, (void *)value);
	if (value[0] == CLIENT_MODE) 
		vInt = 8;	// client
	else	// 0(AP_MODE) or 3(AP_WDS_MODE)
		vInt = 16;	// AP

	sprintf(parm, "opmode=%u", vInt);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);

	// RFChipID
	//12/16/04' hrchen, set to 0 for driver support auto select
	mib_get(MIB_HW_RF_TYPE, (void *)value);
	sprintf(parm, "RFChipID=%u", value[0]);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
#ifndef CONFIG_USB_RTL8192SU_SOFTAP	
	// Diversity
	mib_get(MIB_HW_ANT_DIVERSITY, (void *)value);
	sprintf(parm, "Diversity=%u", value[0]);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
	
	// DefaultAnt
	mib_get(MIB_HW_TX_ANT, (void *)value);
	sprintf(parm, "DefaultAnt=%u", value[0]);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
	
	//12/16/04' hrchen, according to David, set to 4
	// initialGain
	value[0] = 4;
	sprintf(parm, "initialGain=%u", value[0]);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);

	// shortretry
	argv[3] = "shortretry=6";
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
	
	// longretry
	argv[3] = "longretry=6";
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
#endif	
#ifdef WLAN_8185AG
	//12/16/04' hrchen, only need to set once
	// Added by Mason Yu for Set TxPower	
#ifndef CONFIG_USB_RTL8192SU_SOFTAP	
#ifndef CONFIG_USB_RTL8187SU_SOFTAP
	//check if eeprom exist
	if(!check_wlan_eeprom()){	
#endif		
		//no eeprom
		if ( mib_get( MIB_TX_POWER, (void *)&mode) == 0) {
   			printf("Get MIB_TX_POWER error!\n");		
   			status=-1;
		}
		
		i=0;	
		mib_get(MIB_HW_TX_POWER_CCK, (void *)value);
		sprintf(parm, "TxPowerCCK=");

//added by xl_yue: 
//#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
#ifdef WLAN_TX_POWER_DISPLAY
		if ( mode==0 ) {          // 100%
			for(i=0; i<=13; i++)
			{
	//			    value[i] = 8;
				sprintf(parm, "%s%02x", parm, value[i]);
			}	
		}else if ( mode==1 ) {    // 80%
			for(i=0; i<=13; i++)
			{
#ifdef CONFIG_USB_RTL8187SU_SOFTAP	
				value[i] -= 2;
#else			
			    	value[i] -= 1;
#endif			    	
				sprintf(parm, "%s%02x", parm, value[i]);
			}
		}else if ( mode==2 ) {    // 50%
			for(i=0; i<=13; i++)
			{
#ifdef CONFIG_USB_RTL8187SU_SOFTAP	
				value[i] -= 6;
#else				
			    	value[i] -= 3;
#endif			    	
				sprintf(parm, "%s%02x", parm, value[i]);
			}
		}else if ( mode==3 ) {    // 25%
			for(i=0; i<=13; i++)
			{
#ifdef CONFIG_USB_RTL8187SU_SOFTAP	
				value[i] -= 12;
#else				
			   	value[i] -= 6;
#endif			   	
				sprintf(parm, "%s%02x", parm, value[i]);
			}
		}else if ( mode==4 ) {    // 10%
			for(i=0; i<=13; i++)
			{
#ifdef CONFIG_USB_RTL8187SU_SOFTAP	
				value[i] -= 20;
#else				
			   	value[i] -= 10;
#endif			   	
				sprintf(parm, "%s%02x", parm, value[i]);
			}
		}
#else

		if ( mode==2 ) {          // 60mW
			for(i=0; i<=13; i++)
			{
	//			    value[i] = 8;
				sprintf(parm, "%s%02x", parm, value[i]);
			}	
		}else if ( mode==1 ) {    // 30mW
			for(i=0; i<=13; i++)
			{
#ifdef CONFIG_USB_RTL8187SU_SOFTAP	
				value[i] -= 6;
#else				
			    	value[i] -= 3;
#endif			    	
				sprintf(parm, "%s%02x", parm, value[i]);
			}
		}else if ( mode==0 ) {    // 15mW
			for(i=0; i<=13; i++)
			{
#ifdef CONFIG_USB_RTL8187SU_SOFTAP	
				value[i] -= 12;
#else				
			   	value[i] -= 6;
#endif			   	
				sprintf(parm, "%s%02x", parm, value[i]);
			}
		}
#endif	
		argv[3] = parm;
		argv[4] = NULL;
//		printf("txpower mode = %u,set = %s\n",mode,parm);
		TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
		status|=do_cmd(IWPRIV, argv, 1);
			
		mib_get(MIB_HW_TX_POWER_OFDM, (void *)value);
		sprintf(parm, "TxPowerOFDM=");

//added by xl_yue: 
//#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
#ifdef WLAN_TX_POWER_DISPLAY
		if ( mode==0 ) {          // 100%
			for(i=0; i<=13; i++)
			{
//			    value[i] = 8;
				sprintf(parm, "%s%02x", parm, value[i]);
			}	
		}else if ( mode==1 ) {    // 80%
			for(i=0; i<=13; i++)
			{
#ifdef CONFIG_USB_RTL8187SU_SOFTAP	
				value[i] -= 2;
#else				
			    	value[i] -= 1;
#endif			    	
				sprintf(parm, "%s%02x", parm, value[i]);
			}
		}else if ( mode==2 ) {    // 50%
			for(i=0; i<=13; i++)
			{
#ifdef CONFIG_USB_RTL8187SU_SOFTAP	
				value[i] -= 6;
#else				
			    	value[i] -= 3;
#endif			    	
				sprintf(parm, "%s%02x", parm, value[i]);
			}
		}else if ( mode==3 ) {    // 25%
			for(i=0; i<=13; i++)
			{
#ifdef CONFIG_USB_RTL8187SU_SOFTAP	
				value[i] -= 12;
#else				
			   	value[i] -= 6;
#endif			   	
				sprintf(parm, "%s%02x", parm, value[i]);
			}
		}else if ( mode==4 ) {    // 10%
			for(i=0; i<=13; i++)
			{
#ifdef CONFIG_USB_RTL8187SU_SOFTAP	
				value[i] -= 20;
#else				
			   	value[i] -= 10;
#endif			   	
				sprintf(parm, "%s%02x", parm, value[i]);
			}
		}	
#else
		if ( mode==2 ) {          // 60mW
			for(i=0; i<=13; i++)
			{
//			    value[i] = 8;
				sprintf(parm, "%s%02x", parm, value[i]);
			}	
		}else if ( mode==1 ) {    // 30mW
			for(i=0; i<=13; i++)
			{
#ifdef CONFIG_USB_RTL8187SU_SOFTAP	
				value[i] -= 6;
#else				
			    	value[i] -= 3;
#endif			    	
				sprintf(parm, "%s%02x", parm, value[i]);
			}
		}else if ( mode==0 ) {    // 15mW
			for(i=0; i<=13; i++)
			{
#ifdef CONFIG_USB_RTL8187SU_SOFTAP	
				value[i] -= 12;
#else				
			   	value[i] -= 6;
#endif			   	
				sprintf(parm, "%s%02x", parm, value[i]);
			}
		}	
#endif	
		argv[3] = parm;
		argv[4] = NULL;
//		printf("txpower mode = %u,set = %s\n",mode,parm);
		TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
		status|=do_cmd(IWPRIV, argv, 1);
#ifndef CONFIG_USB_RTL8187SU_SOFTAP
	}	
#endif
#endif
#endif
	
	// bcnint
	mib_get(MIB_WLAN_BEACON_INTERVAL, (void *)value);
	vInt = (int)(*(unsigned short *)value);
	sprintf(parm, "bcnint=%u", vInt);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
	
	// channel
	mib_get(MIB_WLAN_CHAN_NUM, (void *)value);
	sprintf(parm, "channel=%u", value[0]);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
	
#ifdef WIFI_TEST
	mib_get(MIB_WLAN_BAND, (void *)value);
	if (value[0] == 4) // WiFi-G
		sprintf(parm, "basicrates=%u", 0x15f);
	else if (value[0] == 5) // WiFi-BG
		sprintf(parm, "basicrates=%u", 0x00f);
	else {
#endif
	//jim do wifi test tricky, 
	//    1 for wifi logo test, 
	//    0 for normal case...
	mib_get(MIB_WIFI_SUPPORT, (void*)value);
	if(value[0]==1){
		mib_get(MIB_WLAN_BAND, (void *)value);
		if (value[0] == 2) // WiFi-G
			sprintf(parm, "basicrates=%u", 0x15f);
		else if (value[0] == 3) // WiFi-BG
			sprintf(parm, "basicrates=%u", 0x00f);
		else {	
			mib_get(MIB_WLAN_BASIC_RATE, (void *)value);
			vInt = (int)(*(unsigned short *)value);
			sprintf(parm, "basicrates=%u", vInt);
		}
	}
	else{
		// basicrates
		mib_get(MIB_WLAN_BASIC_RATE, (void *)value);
		vInt = (int)(*(unsigned short *)value);
		sprintf(parm, "basicrates=%u", vInt);
	}
#ifdef WIFI_TEST
	}
#endif
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
	
#ifdef WIFI_TEST
	mib_get(MIB_WLAN_BAND, (void *)value);
	if (value[0] == 4) // WiFi-G
		sprintf(parm, "oprates=%u", 0xfff);
	else if (value[0] == 5) // WiFi-BG
		sprintf(parm, "oprates=%u", 0xfff);
	else {
#endif
	//jim do wifi test tricky, 
	//    1 for wifi logo test, 
	//    0 for normal case...
	mib_get(MIB_WIFI_SUPPORT, (void*)value);
	if(value[0]==1){
		mib_get(MIB_WLAN_BAND, (void *)value);
		if (value[0] == 2) // WiFi-G
			sprintf(parm, "oprates=%u", 0xfff);
		else if (value[0] == 3) // WiFi-BG
			sprintf(parm, "oprates=%u", 0xfff);
		else {
			mib_get(MIB_WLAN_SUPPORTED_RATE, (void *)value);
			vInt = (int)(*(unsigned short *)value);
			sprintf(parm, "oprates=%u", vInt);
		}
	}
	else{
		// oprates
		mib_get(MIB_WLAN_SUPPORTED_RATE, (void *)value);
		vInt = (int)(*(unsigned short *)value);
		sprintf(parm, "oprates=%u", vInt);
	}
#ifdef WIFI_TEST
	}
#endif
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
	
	// autorate
	mib_get(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)value);
	autoRate = (int)value[0];
	sprintf(parm, "autorate=%u", autoRate);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
	
	// rtsthres
	mib_get(MIB_WLAN_RTS_THRESHOLD, (void *)value);
	vInt = (int)(*(unsigned short *)value);
	sprintf(parm, "rtsthres=%u", vInt);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
	
	// fragthres
	mib_get(MIB_WLAN_FRAG_THRESHOLD, (void *)value);
	vInt = (int)(*(unsigned short *)value);
	sprintf(parm, "fragthres=%u", vInt);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
	
	// expired_time
	mib_get(MIB_WLAN_INACTIVITY_TIME, (void *)value);
	vInt = (int)(*(unsigned long *)value);
	sprintf(parm, "expired_time=%u", vInt);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
	
	// preamble
	mib_get(MIB_WLAN_PREAMBLE_TYPE, (void *)value);

//modified  by xl_yue
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	if(value[0] == AUTO_PREAMBLE)
		value[0] = SHORT_PREAMBLE;
#endif

	sprintf(parm, "preamble=%u", value[0]);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
	
	// hiddenAP
	mib_get(MIB_WLAN_HIDDEN_SSID, (void *)value);
	sprintf(parm, "hiddenAP=%u", value[0]);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
#ifndef CONFIG_USB_RTL8192SU_SOFTAP
	// beacon advertisement
	mib_get(MIB_WLAN_BEACON_ADVERTISEMENT, (void *)value);
	sprintf(parm, "bcnadv=%u", value[0]);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
#endif	
	// dtimperiod
	mib_get(MIB_WLAN_DTIM_PERIOD, (void *)value);
	sprintf(parm, "dtimperiod=%u", value[0]);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
	
#ifdef WLAN_ACL
	status|=set_wlan_acl();
#endif
	
	// authtype
	// Modified by Mason Yu
	// Root AP's authtype
	mib_get(MIB_WLAN_AUTH_TYPE, (void *)value);
	mib_get(MIB_WLAN_ENCRYPT, (void *)&value[1]);
	mib_get(MIB_WSC_DISABLE, (void *)&value[2]);
	
	if (value[0] == 1 && value[1] != 1)
		// shared-key and not WEP enabled, force to open-system
		value[0] = 0;
#ifdef CONFIG_WIFI_SIMPLE_CONFIG //cathy
	//clear wsc_enable, this parameter will be set in wsc daemon
	sprintf(parm, "wsc_enable=0");
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);

	if(value[0] == 1 && value[1] == 1 && value[2]!=1)
		value[0] = 2;	//if shared-key and wep/wps enable, force to open+shared system for wps		
#endif
	sprintf(parm, "authtype=%u", value[0]);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);


#ifdef WLAN_MBSSID	
	// VAP
	for (j=1; j<=4; j++) {
		if (!mib_chain_get(MIB_MBSSIB_TBL, j, (void *)&Entry)) {  		
  			printf("Error! Get MIB_MBSSIB_TBL for VAP SSID error.\n");					
		}
		
		//if ( Entry.wlanDisabled == 1 ) {
			sprintf(para2, "wlan0-vap%d", j-1);
			
			value[0] = Entry.authType;
			value[1] = Entry.encrypt;
			if (value[0] == 1 && value[1] != 1)
				// shared-key and not WEP enabled, force to open-system
				value[0] = 0;
#ifdef CONFIG_WIFI_SIMPLE_CONFIG  //cathy
			if(value[0] == 1 && value[1] == 1)
				value[0] = 2;	//if shared-key and wep enable, force to open+shared system for wps
#endif	
			sprintf(parm, "authtype=%u", value[0]);
			argv[1] = para2;
			argv[3] = parm;
			argv[4] = NULL;
			TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
			status|=do_cmd(IWPRIV, argv, 1);				
		//}		

		//for wifi-vap hidden AP
		value[0] = Entry.hidessid;
		sprintf(parm,"hiddenAP=%u",value[0]);
		argv[3]=parm;
		TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
		status|=do_cmd(IWPRIV, argv, 1);	

		//for wifi-vap beacon advertisement
		value[0] = Entry.bcnAdvtisement;
		sprintf(parm,"bcnadv=%u",value[0]);
		argv[3]=parm;
		TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
		status|=do_cmd(IWPRIV, argv, 1);	

		
	}
	argv[1] = (char*)WLANIF;
#endif

#ifdef WLAN_QoS
	for (j=0; j<4; j++) {
		if (!mib_chain_get(MIB_WLAN_QOS_AP_TBL, j, (void *)&QOSEntry)) {  		
  			printf("Error! Get MIB_WLAN_AP_QOS_TBL error.\n");	
  			continue;
		}
		switch(j){
			case 0://VO
				value[0] = QOSEntry.txop;
				sprintf(parm,"vo_txop=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				value[0] = QOSEntry.ecwmax;
				sprintf(parm,"vo_ecwmax=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				value[0] = QOSEntry.ecwmin;
				sprintf(parm,"vo_ecwmin=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				value[0] = QOSEntry.aifsn;
				sprintf(parm,"vo_aifsn=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				break;
			case 1://VI
				value[0] = QOSEntry.txop;
				sprintf(parm,"vi_txop=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				value[0] = QOSEntry.ecwmax;
				sprintf(parm,"vi_ecwmax=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				value[0] = QOSEntry.ecwmin;
				sprintf(parm,"vi_ecwmin=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				value[0] = QOSEntry.aifsn;
				sprintf(parm,"vi_aifsn=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				break;
			case 2://BE
				value[0] = QOSEntry.txop;
				sprintf(parm,"be_txop=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				value[0] = QOSEntry.ecwmax;
				sprintf(parm,"be_ecwmax=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				value[0] = QOSEntry.ecwmin;
				sprintf(parm,"be_ecwmin=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				value[0] = QOSEntry.aifsn;
				sprintf(parm,"be_aifsn=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				break;
			case 3://BK
				value[0] = QOSEntry.txop;
				sprintf(parm,"bk_txop=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				value[0] = QOSEntry.ecwmax;
				sprintf(parm,"bk_ecwmax=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				value[0] = QOSEntry.ecwmin;
				sprintf(parm,"bk_ecwmin=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				value[0] = QOSEntry.aifsn;
				sprintf(parm,"bk_aifsn=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				break;
			default:
				break;
		}
	}

	//sta
	for (j=0; j<4; j++) {
		if (!mib_chain_get(MIB_WLAN_QOS_STA_TBL, j, (void *)&QOSEntry)) {  		
  			printf("Error! Get MIB_WLAN_STA_QOS_TBL error.\n");	
  			continue;
		}
		switch(j){
			case 0://VO
				value[0] = QOSEntry.txop;
				sprintf(parm,"sta_vo_txop=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				value[0] = QOSEntry.ecwmax;
				sprintf(parm,"sta_vo_ecwmax=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				value[0] = QOSEntry.ecwmin;
				sprintf(parm,"sta_vo_ecwmin=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				value[0] = QOSEntry.aifsn;
				sprintf(parm,"sta_vo_aifsn=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				break;
			case 1://VI
				value[0] = QOSEntry.txop;
				sprintf(parm,"sta_vi_txop=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				value[0] = QOSEntry.ecwmax;
				sprintf(parm,"sta_vi_ecwmax=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				value[0] = QOSEntry.ecwmin;
				sprintf(parm,"sta_vi_ecwmin=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				value[0] = QOSEntry.aifsn;
				sprintf(parm,"sta_vi_aifsn=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				break;
			case 2://BE
				value[0] = QOSEntry.txop;
				sprintf(parm,"sta_be_txop=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				value[0] = QOSEntry.ecwmax;
				sprintf(parm,"sta_be_ecwmax=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				value[0] = QOSEntry.ecwmin;
				sprintf(parm,"sta_be_ecwmin=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				value[0] = QOSEntry.aifsn;
				sprintf(parm,"sta_be_aifsn=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				break;
			case 3://BK
				value[0] = QOSEntry.txop;
				sprintf(parm,"sta_bk_txop=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				value[0] = QOSEntry.ecwmax;
				sprintf(parm,"sta_bk_ecwmax=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				value[0] = QOSEntry.ecwmin;
				sprintf(parm,"sta_bk_ecwmin=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				value[0] = QOSEntry.aifsn;
				sprintf(parm,"sta_bk_aifsn=%u",value[0]);
				argv[3]=parm;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);	
				break;
			default:
				break;
		}
	}

#endif

#ifdef WLAN_WPA

	// Modified by Mason Yu
	// encmode
	// Root AP
//added by xl_yue: create random wep key for rootAP
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		if (mib_get(MIB_ELAN_MAC_ADDR, (void *)value) != 0)
		{
			value[0] = value[0]^value[2]^value[4];
			value[1] = value[1]^value[3]^value[5];
			mib_get(MIB_WLAN_WEP64_KEY1, (void *)stringbuf);
			if(isAllZero(stringbuf,5)){
				snprintf((char *)(stringbuf), 6, "%02x%02x%d",value[0], value[1], 1);
				mib_set(MIB_WLAN_WEP64_KEY1, (void *)stringbuf);
			}
			mib_get(MIB_WLAN_WEP64_KEY2, (void *)stringbuf);
			if(isAllZero(stringbuf,5)){
				snprintf((char *)(stringbuf), 6, "%02x%02x%d",value[0], value[1], 2);
				mib_set(MIB_WLAN_WEP64_KEY2, (void *)stringbuf);
			}
			mib_get(MIB_WLAN_WEP64_KEY3, (void *)stringbuf);
			if(isAllZero(stringbuf,5)){
				snprintf((char *)(stringbuf), 6, "%02x%02x%d",value[0], value[1], 3);
				mib_set(MIB_WLAN_WEP64_KEY3, (void *)stringbuf);
			}
			mib_get(MIB_WLAN_WEP64_KEY4, (void *)stringbuf);
			if(isAllZero(stringbuf,5)){
				snprintf((char *)(stringbuf), 6, "%02x%02x%d",value[0], value[1], 4);
				mib_set(MIB_WLAN_WEP64_KEY4, (void *)stringbuf);
			}
			mib_get(MIB_WLAN_WEP128_KEY1, (void *)stringbuf);
			if(isAllZero(stringbuf,13)){
				snprintf((char *)(stringbuf), 14, "%02x%02x%02x%02x%02x%02x%d",value[0], value[1], value[2], value[3], value[4], value[5], 1);
				mib_set(MIB_WLAN_WEP128_KEY1, (void *)stringbuf);
			}
			mib_get(MIB_WLAN_WEP128_KEY2, (void *)stringbuf);
			if(isAllZero(stringbuf,13)){
				snprintf((char *)(stringbuf), 14, "%02x%02x%02x%02x%02x%02x%d",value[0], value[1], value[2], value[3], value[4], value[5], 2);
				mib_set(MIB_WLAN_WEP128_KEY2, (void *)stringbuf);
			}
			mib_get(MIB_WLAN_WEP128_KEY3, (void *)stringbuf);
			if(isAllZero(stringbuf,13)){
				snprintf((char *)(stringbuf), 14, "%02x%02x%02x%02x%02x%02x%d",value[0], value[1], value[2], value[3], value[4], value[5], 3);
				mib_set(MIB_WLAN_WEP128_KEY3, (void *)stringbuf);
			}
			mib_get(MIB_WLAN_WEP128_KEY4, (void *)stringbuf);
			if(isAllZero(stringbuf,13)){
				snprintf((char *)(stringbuf), 14, "%02x%02x%02x%02x%02x%02x%d",value[0], value[1], value[2], value[3], value[4], value[5], 4);
				mib_set(MIB_WLAN_WEP128_KEY4, (void *)stringbuf);
			}
		}
#endif

	mib_get(MIB_WLAN_ENCRYPT, (void *)value);
	vInt = (int)value[0];
	
	if (vInt == 1)	// WEP mode
	{
		mib_get(MIB_WLAN_WEP, (void *)value);
		if (value[0] == 1) {
			// 64 bits
			// wepkey1
			mib_get(MIB_WLAN_WEP64_KEY1, (void *)value);
			sprintf(parm, "wepkey1=%02x%02x%02x%02x%02x", value[0],
				value[1], value[2], value[3], value[4]);
			argv[3] = parm;
			argv[4] = NULL;
			TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
			status|=do_cmd(IWPRIV, argv, 1);
			
			// wepkey2
			mib_get(MIB_WLAN_WEP64_KEY2, (void *)value);
			sprintf(parm, "wepkey2=%02x%02x%02x%02x%02x", value[0],
				value[1], value[2], value[3], value[4]);
			argv[3] = parm;
			argv[4] = NULL;
			TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
			status|=do_cmd(IWPRIV, argv, 1);
			
			// wepkey3
			mib_get(MIB_WLAN_WEP64_KEY3, (void *)value);
			sprintf(parm, "wepkey3=%02x%02x%02x%02x%02x", value[0],
				value[1], value[2], value[3], value[4]);
			argv[3] = parm;
			argv[4] = NULL;
			TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
			status|=do_cmd(IWPRIV, argv, 1);
			
			// wepkey4
			mib_get(MIB_WLAN_WEP64_KEY4, (void *)value);
			sprintf(parm, "wepkey4=%02x%02x%02x%02x%02x", value[0],
				value[1], value[2], value[3], value[4]);
			argv[3] = parm;
			argv[4] = NULL;
			TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
			status|=do_cmd(IWPRIV, argv, 1);
			
			// wepdkeyid
			mib_get(MIB_WLAN_WEP_DEFAULT_KEY, (void *)value);
			sprintf(parm, "wepdkeyid=%u", value[0]);
			argv[3] = parm;
			argv[4] = NULL;
			TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
			status|=do_cmd(IWPRIV, argv, 1);
		}
		else {
			// 128 bits
			vInt = 5; // encmode = 5
			// wepkey1
			mib_get(MIB_WLAN_WEP128_KEY1, (void *)value);
			sprintf(parm, "wepkey1=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
				value[0], value[1], value[2], value[3], value[4],
				value[5], value[6], value[7], value[8], value[9],
				value[10], value[11], value[12]);
			argv[3] = parm;
			argv[4] = NULL;
			TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
			status|=do_cmd(IWPRIV, argv, 1);
			
			// wepkey2
			mib_get(MIB_WLAN_WEP128_KEY2, (void *)value);
			sprintf(parm, "wepkey2=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
				value[0], value[1], value[2], value[3], value[4],
				value[5], value[6], value[7], value[8], value[9],
				value[10], value[11], value[12]);
			argv[3] = parm;
			argv[4] = NULL;
			TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
			status|=do_cmd(IWPRIV, argv, 1);
			
			// wepkey3
			mib_get(MIB_WLAN_WEP128_KEY3, (void *)value);
			sprintf(parm, "wepkey3=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
				value[0], value[1], value[2], value[3], value[4],
				value[5], value[6], value[7], value[8], value[9],
				value[10], value[11], value[12]);
			argv[3] = parm;
			argv[4] = NULL;
			TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
			status|=do_cmd(IWPRIV, argv, 1);
			
			// wepkey4
			mib_get(MIB_WLAN_WEP128_KEY4, (void *)value);
			sprintf(parm, "wepkey4=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
				value[0], value[1], value[2], value[3], value[4],
				value[5], value[6], value[7], value[8], value[9],
				value[10], value[11], value[12]);
			argv[3] = parm;
			argv[4] = NULL;
			TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
			status|=do_cmd(IWPRIV, argv, 1);
			
			// wepdkeyid
			mib_get(MIB_WLAN_WEP_DEFAULT_KEY, (void *)value);
			sprintf(parm, "wepdkeyid=%u", value[0]);
			argv[3] = parm;
			argv[4] = NULL;
			TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
			status|=do_cmd(IWPRIV, argv, 1);
		}
	}
	// Kaohj --- this is for driver level security.
	// ignoring it if using userland 'auth' security
	#ifdef CONFIG_USB_RTL8192SU_SOFTAP	//driver based security is configured in rtl8192 driver
	else if ((vInt >= 2) && !is8021xEnabled()) {	// WPA setup
		char buf[1024];
		// psk_enable: 1: WPA, 2: WPA2, 3: WPA+WPA2
		sprintf(parm, "psk_enable=%d", vInt/2);
		argv[3] = parm;
		argv[4] = NULL;
		TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
		status|=do_cmd(IWPRIV, argv, 1);
		
		//if (vInt == ENCRYPT_WPA) {
			// wpa_cipher: fixed at TKIP:2
			sprintf(parm, "wpa_cipher=2");
			argv[3] = parm;
			argv[4] = NULL;
			TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
			status|=do_cmd(IWPRIV, argv, 1);
		//}
		//else if (vInt == ENCRYPT_WPA2) {
			// wpa2_cipher: fixed at AES:8
			sprintf(parm, "wpa2_cipher=8");
			argv[3] = parm;
			argv[4] = NULL;
			TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
			status|=do_cmd(IWPRIV, argv, 1);
		//}
		//else {// mixed mode
		//}
		// passphrase
		mib_get( MIB_WLAN_WPA_PSK, (void *)parm);
		sprintf(buf, "passphrase=%s", parm);
		argv[3] = buf;
		argv[4] = NULL;
		TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
		status|=do_cmd(IWPRIV, argv, 1);
		
		vInt = 2;
	}
	#endif
	
	sprintf(parm, "encmode=%u", vInt);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);

#ifdef WLAN_MBSSID
	// encmode	
	for (j=1; j<=4; j++) {
		if (!mib_chain_get(MIB_MBSSIB_TBL, j, (void *)&Entry)) {  		
  			printf("Error! Get MIB_MBSSIB_TBL error.\n");					
		}
		
		if (!mib_chain_get(MIB_MBSSIB_WEP_TBL, j, (void *)&EntryWEP)) {  		
  			printf("Error! Get MIB_MBSSIB_WEP_TBL error.\n");					
		}

//added by xl_yue: create random wep key
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		if (mib_get(MIB_ELAN_MAC_ADDR, (void *)value) != 0)
		{
			value[0] = value[0]^value[2]^value[4]^j;
			value[1] = value[1]^value[3]^value[5];
//			if(strlen(EntryWEP.wep64Key1) == 0){
			if(isAllZero((EntryWEP.wep64Key1),5)){
				snprintf((char *)(stringbuf), 6, "%02x%02x%d",value[0], value[1], 1);
				memcpy(EntryWEP.wep64Key1,stringbuf,WEP64_KEY_LEN);
			}
//			if(strlen(EntryWEP.wep64Key2) == 0){
			if(isAllZero((EntryWEP.wep64Key2),5)){
				snprintf((char *)(stringbuf), 6, "%02x%02x%d",value[0], value[1], 2);
				memcpy(EntryWEP.wep64Key2,stringbuf,WEP64_KEY_LEN);
			}
//			if(strlen(EntryWEP.wep64Key3) == 0){
			if(isAllZero((EntryWEP.wep64Key3),5)){
				snprintf((char *)(stringbuf), 6, "%02x%02x%d",value[0], value[1], 3);
				memcpy(EntryWEP.wep64Key3,stringbuf,WEP64_KEY_LEN);
			}
//			if(strlen(EntryWEP.wep64Key4) == 0){
			if(isAllZero((EntryWEP.wep64Key4),5)){
				snprintf((char *)(stringbuf), 6, "%02x%02x%d",value[0], value[1], 4);
				memcpy(EntryWEP.wep64Key4,stringbuf,WEP64_KEY_LEN);
			}
//			if(strlen(EntryWEP.wep128Key1) == 0){
			if(isAllZero((EntryWEP.wep128Key1),13)){
				snprintf((char *)(stringbuf), 14, "%02x%02x%02x%02x%02x%02x%d",value[0], value[1], value[2], value[3], value[4], value[5], 1);
				memcpy(EntryWEP.wep128Key1,stringbuf,WEP128_KEY_LEN);
			}
//			if(strlen(EntryWEP.wep128Key2) == 0){
			if(isAllZero((EntryWEP.wep128Key2),13)){
				snprintf((char *)(stringbuf), 14, "%02x%02x%02x%02x%02x%02x%d",value[0], value[1], value[2], value[3], value[4], value[5], 2);
				memcpy(EntryWEP.wep128Key2,stringbuf,WEP128_KEY_LEN);
			}
//			if(strlen(EntryWEP.wep128Key3) == 0){
			if(isAllZero((EntryWEP.wep128Key3),13)){
				snprintf((char *)(stringbuf), 14, "%02x%02x%02x%02x%02x%02x%d",value[0], value[1], value[2], value[3], value[4], value[5], 3);
				memcpy(EntryWEP.wep128Key3,stringbuf,WEP128_KEY_LEN);
			}
//			if(strlen(EntryWEP.wep128Key4) == 0){
			if(isAllZero((EntryWEP.wep128Key4),13)){
				snprintf((char *)(stringbuf), 14, "%02x%02x%02x%02x%02x%02x%d",value[0], value[1], value[2], value[3], value[4], value[5], 4);
				memcpy(EntryWEP.wep128Key4,stringbuf,WEP128_KEY_LEN);
			}

			mib_chain_update(MIB_MBSSIB_WEP_TBL, (void *)&EntryWEP, j);
		}
#endif
			
		if ( j != 0 ) {
			sprintf(para2, "wlan0-vap%d", j-1);			
			argv[1] = para2;				
		}	
		
		//mib_get(MIB_WLAN_ENCRYPT, (void *)value);
		//vInt = (int)value[0];
		vInt = (int)Entry.encrypt;
		
		if (vInt == 1)	// WEP mode
		{
			//mib_get(MIB_WLAN_WEP, (void *)value);
			value[0] = Entry.wep;
			if (value[0] == 1) {
				// 64 bits
				// wepkey1
				//mib_get(MIB_WLAN_WEP64_KEY1, (void *)value);				
				memcpy(value, EntryWEP.wep64Key1, WEP64_KEY_LEN);
				sprintf(parm, "wepkey1=%02x%02x%02x%02x%02x", value[0],
					value[1], value[2], value[3], value[4]);
				argv[3] = parm;
				argv[4] = NULL;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				
				// wepkey2
				//mib_get(MIB_WLAN_WEP64_KEY2, (void *)value);				
				memcpy(value, EntryWEP.wep64Key2, WEP64_KEY_LEN);
				sprintf(parm, "wepkey2=%02x%02x%02x%02x%02x", value[0],
					value[1], value[2], value[3], value[4]);
				argv[3] = parm;
				argv[4] = NULL;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				
				// wepkey3
				//mib_get(MIB_WLAN_WEP64_KEY3, (void *)value);				
				memcpy(value, EntryWEP.wep64Key3, WEP64_KEY_LEN);
				sprintf(parm, "wepkey3=%02x%02x%02x%02x%02x", value[0],
					value[1], value[2], value[3], value[4]);
				argv[3] = parm;
				argv[4] = NULL;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				
				// wepkey4
				//mib_get(MIB_WLAN_WEP64_KEY4, (void *)value);				
				memcpy(value, EntryWEP.wep64Key4, WEP64_KEY_LEN);
				sprintf(parm, "wepkey4=%02x%02x%02x%02x%02x", value[0],
					value[1], value[2], value[3], value[4]);
				argv[3] = parm;
				argv[4] = NULL;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				
				// wepdkeyid
				//mib_get(MIB_WLAN_WEP_DEFAULT_KEY, (void *)value);
				value[0] = EntryWEP.wepDefaultKey;
				sprintf(parm, "wepdkeyid=%u", value[0]);
				argv[3] = parm;
				argv[4] = NULL;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
			}
			else {
				// 128 bits
				vInt = 5; // encmode = 5
				// wepkey1
				//mib_get(MIB_WLAN_WEP128_KEY1, (void *)value);				
				memcpy(value, EntryWEP.wep128Key1, WEP128_KEY_LEN);
				sprintf(parm, "wepkey1=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
					value[0], value[1], value[2], value[3], value[4],
					value[5], value[6], value[7], value[8], value[9],
					value[10], value[11], value[12]);
				argv[3] = parm;
				argv[4] = NULL;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				
				// wepkey2
				//mib_get(MIB_WLAN_WEP128_KEY2, (void *)value);				
				memcpy(value, EntryWEP.wep128Key2, WEP128_KEY_LEN);
				sprintf(parm, "wepkey2=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
					value[0], value[1], value[2], value[3], value[4],
					value[5], value[6], value[7], value[8], value[9],
					value[10], value[11], value[12]);
				argv[3] = parm;
				argv[4] = NULL;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				
				// wepkey3
				//mib_get(MIB_WLAN_WEP128_KEY3, (void *)value);				
				memcpy(value, EntryWEP.wep128Key3, WEP128_KEY_LEN);
				sprintf(parm, "wepkey3=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
					value[0], value[1], value[2], value[3], value[4],
					value[5], value[6], value[7], value[8], value[9],
					value[10], value[11], value[12]);
				argv[3] = parm;
				argv[4] = NULL;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				
				// wepkey4
				//mib_get(MIB_WLAN_WEP128_KEY4, (void *)value);				
				memcpy(value, EntryWEP.wep128Key4, WEP128_KEY_LEN);
				sprintf(parm, "wepkey4=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
					value[0], value[1], value[2], value[3], value[4],
					value[5], value[6], value[7], value[8], value[9],
					value[10], value[11], value[12]);
				argv[3] = parm;
				argv[4] = NULL;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
				
				// wepdkeyid
				//mib_get(MIB_WLAN_WEP_DEFAULT_KEY, (void *)value);
				value[0] = EntryWEP.wepDefaultKey;
				sprintf(parm, "wepdkeyid=%u", value[0]);
				argv[3] = parm;
				argv[4] = NULL;
				TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
				status|=do_cmd(IWPRIV, argv, 1);
			}
		}
		// Kaohj --- this is for driver level security.
		// ignoring it if using userland 'auth' security	
		
		sprintf(parm, "encmode=%u", vInt);
		argv[3] = parm;
		argv[4] = NULL;
		TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
		status|=do_cmd(IWPRIV, argv, 1);	
	}
	argv[1] = (char*)WLANIF;	
#endif


	// Modified by Mason Yu
	// Set 802.1x flag
	mib_get(MIB_WLAN_ENCRYPT, (void *)value);
	vInt = (int)value[0];
	
	if (vInt < 2)
	//else if (vInt == 2)	// WPA mode
	{
		// 802_1x
		mib_get(MIB_WLAN_ENABLE_1X, (void *)value);
		sprintf(parm, "802_1x=%u", value[0]);
		argv[3] = parm;
		argv[4] = NULL;
		TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
		status|=do_cmd(IWPRIV, argv, 1);
		//vInt = 0;
	}
	else{
		vInt = 1;
		sprintf(parm, "802_1x=%u", vInt);
		argv[3] = parm;
		argv[4] = NULL;
		TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
		status|=do_cmd(IWPRIV, argv, 1);
	}


#ifdef WLAN_MBSSID
	// Set 802.1x flag
	for (j=1; j<=4; j++) {
		if (!mib_chain_get(MIB_MBSSIB_TBL, j, (void *)&Entry)) {  		
  			printf("Error! Get MIB_MBSSIB_TBL(802.1x) error.\n");					
		}
		
		if ( j != 0 ) {
			sprintf(para2, "wlan0-vap%d", j-1);			
			argv[1] = para2;				
		}
			
		//mib_get(MIB_WLAN_ENCRYPT, (void *)value);
		//vInt = (int)value[0];
		vInt = (int)Entry.encrypt;
		
		if (vInt < 2)
		//else if (vInt == 2)	// WPA mode
		{
			// 802_1x
			//mib_get(MIB_WLAN_ENABLE_1X, (void *)value);
			value[0] = Entry.enable1X;
			sprintf(parm, "802_1x=%u", value[0]);
			argv[3] = parm;
			argv[4] = NULL;
			TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
			status|=do_cmd(IWPRIV, argv, 1);
			//vInt = 0;
		}
		else{
			vInt = 1;
			sprintf(parm, "802_1x=%u", vInt);
			argv[3] = parm;
			argv[4] = NULL;
			TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
			status|=do_cmd(IWPRIV, argv, 1);
		}
	}
	argv[1] = (char*)WLANIF;
#endif


#endif
	
#ifdef WLAN_IAPP
	// iapp_enable
	mib_get(MIB_WLAN_IAPP_DISABLED, (void *)value);
	vInt = (int)(value[0] ^ 0x01);
	sprintf(parm, "iapp_enable=%u", vInt);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
#endif
	
#ifdef WLAN_8185AG
	// band
	mib_get(MIB_WLAN_BAND, (void *)value);
#ifdef WIFI_TEST
	if (value[0] == 4) // WiFi-G
		value[0] = 3; // 2.4 GHz (B+G)
	else if (value[0] == 5) // WiFi-BG
		value[0] = 3; // 2.4 GHz (B+G)
#endif
	//jim do wifi test tricky, 
	//    1 for wifi logo test, 
	//    0 for normal case...
	unsigned char vChar;
	mib_get(MIB_WIFI_SUPPORT, (void*)&vChar);
	if(vChar==1){
		if(value[0] == 2)
			value[0] = 3;
	}
	sprintf(parm, "band=%u", value[0]);	//802.11b:1, 802.11g:2, 802.11n:8
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
	
	if (autoRate == 0)
	{
		// fixrate
		mib_get(MIB_WLAN_FIX_RATE, (void *)value);
#ifdef CONFIG_USB_RTL8192SU_SOFTAP
		vInt = (int)(*(unsigned int *)value);
#else
		vInt = (int)(*(unsigned short *)value);
#endif
		sprintf(parm, "fixrate=%u", vInt);
		argv[3] = parm;
		argv[4] = NULL;
		TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
		status|=do_cmd(IWPRIV, argv, 1);
	}
//cathy, for multicast rate
#ifdef CONFIG_USB_RTL8187SU_SOFTAP
	mib_get(MIB_WLAN_MLCSTRATE, (void *)value);
	vInt = (int)(*(unsigned short *)value);
	sprintf(parm, "lowestMlcstRate=%u", vInt);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);

#endif
	
#endif

#ifdef WLAN_WDS
	setupWDS();
#endif

//12/23/04' hrchen, these MIBs are for CLIENT mode, disable them
#if 0	
	//12/16/04' hrchen, disable nat25_disable
	// nat25_disable
	value[0] = 0;
	sprintf(parm, "nat25_disable=%u", value[0]);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
		
	//12/16/04' hrchen, disable macclone_enable
	// macclone_enable
	value[0] = 0;
	sprintf(parm, "macclone_enable=%u", value[0]);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
#endif

#if 0	
	//12/16/04' hrchen, debug flag
	// debug_err
	sprintf(parm, "debug_err=ffffffff");
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
	// debug_info
	sprintf(parm, "debug_info=0");
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
	// debug_warn
	sprintf(parm, "debug_warn=0");
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
	// debug_trace
	sprintf(parm, "debug_trace=0");
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
#endif
		
	//12/16/04' hrchen, for protection mode
	// cts2self
	sprintf(parm, "cts2self=1");
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
		
	//12/16/04' hrchen, set 11g protection mode
	// disable_protection
#ifdef CONFIG_USB_RTL8192SU_SOFTAP
	mib_get(MIB_WLAN_PROTECTION_DISABLED, (void *)value);
#else
	value[0] = 0;
#endif
	sprintf(parm, "disable_protection=%u", value[0]);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
		
	//12/16/04' hrchen, chipVersion
	// chipVersion
	/*
	sprintf(parm, "chipVersion=0");
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
	*/
		
#if 0	// not necessary for AP
	//12/16/04' hrchen, defssid
	// defssid
	sprintf(parm, "defssid=\"defaultSSID\"");
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
#endif
		
	//12/16/04' hrchen, set block relay
	// block_relay
	mib_get(MIB_WLAN_BLOCK_RELAY, (void *)value);
	sprintf(parm, "block_relay=%u", value[0]);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
	// Kaohj --- set the wireless block mode on bridging
	// defined at linux-2.4.x/net/bridge/br_private.h
	//#define	WLAN_BLOCK_DISABLE	0
	//#define	WLAN_BLOCK_ALL		1
	//#define	WLAN_BLOCK_ELAN		2
	//#define	WLAN_BLOCK_WAN		3
	mib_get(MIB_WLAN_BLOCK_ETH2WIR, (void *)value);
	va_cmd(BRCTL, 3, 1, "wlanblock", (char *)BRIF, value[0]==0?"0":"2");
#ifdef WIFI_TEST
	mib_get(MIB_WLAN_BAND, (void *)value);
	if (value[0] == 4 || value[0] == 5) {// WiFi-G or WiFi-BG
		// block_relay=0
		sprintf(parm, "block_relay=0");
		argv[3] = parm;
		argv[4] = NULL;
		TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
		status|=do_cmd(IWPRIV, argv, 1);
		// wifi_specific=1
		sprintf(parm, "wifi_specific=1");
		argv[3] = parm;
		argv[4] = NULL;
		TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
		status|=do_cmd(IWPRIV, argv, 1);
	}
#endif
	//jim do wifi test tricky, 
	//    1 for wifi logo test, 
	//    0 for normal case...
	mib_get(MIB_WIFI_SUPPORT, (void*)value);
	if(value[0]==1){
		mib_get(MIB_WLAN_BAND, (void *)value);
		if (value[0] == 2 || value[0] == 3) {// WiFi-G or WiFi-BG
			// block_relay=0
			sprintf(parm, "block_relay=0");
			argv[3] = parm;
			argv[4] = NULL;
			TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
			status|=do_cmd(IWPRIV, argv, 1);
			// wifi_specific=1
			sprintf(parm, "wifi_specific=1");
			argv[3] = parm;
			argv[4] = NULL;
			TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
			status|=do_cmd(IWPRIV, argv, 1);
		}
	}

#ifdef WLAN_QoS
	mib_get(MIB_WLAN_QoS, (void *)value);
	if(value[0]==0)
		va_cmd(IWPRIV, 3, 1, (char *)WLANIF, "set_mib", "qos_enable=0");
	else if(value[0]==1)
		va_cmd(IWPRIV, 3, 1, (char *)WLANIF, "set_mib", "qos_enable=1");		

#ifdef CONFIG_USB_RTL8192SU_SOFTAP
	//for wmm power saving 8192 only
	mib_get(MIB_WLAN_APSD_ENABLE, (void *)value);
	if(value[0]==0)
		va_cmd(IWPRIV, 3, 1, (char *)WLANIF, "set_mib", "apsd_enable=0");
	else if(value[0]==1)
		va_cmd(IWPRIV, 3, 1, (char *)WLANIF, "set_mib", "apsd_enable=1");
#endif
#endif

#ifndef CONFIG_USB_RTL8192SU_SOFTAP
	// DIG_enable --- Dynamic Initial Gain
	mib_get(MIB_WLAN_DIG, (void *)value);
	sprintf(parm, "DIG_enable=%d", value[0]);
	argv[3] = parm;
	argv[4] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s %s\n", IWPRIV, argv[1], argv[2], argv[3]);
	status|=do_cmd(IWPRIV, argv, 1);
#endif

#ifdef CONFIG_USB_RTL8192SU_SOFTAP	//11n only
	//Channel Width
	mib_get(MIB_WLAN_CHANNEL_WIDTH, (void *)value);
	if(value[0]==0) 
		va_cmd(IWPRIV, 3, 1, (char *)WLANIF, "set_mib", "use40M=0");
	else
		va_cmd(IWPRIV, 3, 1, (char *)WLANIF, "set_mib", "use40M=1");

	//Conntrol Sideband
	if(value[0]==0) {	//20M
		va_cmd(IWPRIV, 3, 1, (char *)WLANIF, "set_mib", "2ndchoffset=0");
	}
	else {	//40M
		mib_get(MIB_WLAN_CONTROL_BAND, (void *)value);
		if(value[0]==0)	//upper
			va_cmd(IWPRIV, 3, 1, (char *)WLANIF, "set_mib", "2ndchoffset=2");
		else		//lower
			va_cmd(IWPRIV, 3, 1, (char *)WLANIF, "set_mib", "2ndchoffset=1");
	}

	//short GI
	mib_get(MIB_WLAN_SHORTGI_ENABLED, (void *)value);
	if(value[0]==0) {
		va_cmd(IWPRIV, 3, 1, (char *)WLANIF, "set_mib", "shortGI20M=0");
		va_cmd(IWPRIV, 3, 1, (char *)WLANIF, "set_mib", "shortGI40M=0");
	}
	else {
		va_cmd(IWPRIV, 3, 1, (char *)WLANIF, "set_mib", "shortGI20M=1");
		va_cmd(IWPRIV, 3, 1, (char *)WLANIF, "set_mib", "shortGI40M=1");
	}

	//aggregation
	mib_get(MIB_WLAN_AGGREGATION, (void *)value);
	if(value[0]==0) {
		va_cmd(IWPRIV, 3, 1, (char *)WLANIF, "set_mib", "ampdu=0");
		va_cmd(IWPRIV, 3, 1, (char *)WLANIF, "set_mib", "amsdu=0");
	}
	else {
		va_cmd(IWPRIV, 3, 1, (char *)WLANIF, "set_mib", "ampdu=1");
		va_cmd(IWPRIV, 3, 1, (char *)WLANIF, "set_mib", "amsdu=0");
	}
#endif
	return status;
}
#if 0
void restartWlan()
{
	char vChar;
	ENCRYPT_T encrypt;
	unsigned char no_wlan;
	unsigned char enable1x=0;
#ifdef WLAN_MBSSID
	int i;
	MIB_CE_MBSSIB_T Entry;
	char para[20];
#endif
	
// Mason Yu Test
#ifndef WLAN_MBSSID	
	mib_get( MIB_WLAN_ENCRYPT,  (void *)&vChar);
	encrypt = (ENCRYPT_T)vChar;
	mib_get(MIB_WLAN_DISABLED, (void *)&no_wlan);
#ifdef WLAN_1x
	mib_get( MIB_WLAN_ENABLE_1X, (void *)&enable1x);
#endif
	if (encrypt<=ENCRYPT_WEP && !enable1x && !no_wlan) {
		itfcfg((char *)WLANIF, 0);
		setupWLan();
		itfcfg((char *)WLANIF, 1);
	}
#else

//xl_yue:down wlan interface(root/vap)
for(i=0;i<5;i++){
	if(i == 0)
		strcpy(para, "wlan0");
	else
		sprintf(para, "wlan0-vap%d", i-1);
	itfcfg(para, 0);
}

for ( i=0; i<=4; i++) {
	if (!mib_chain_get(MIB_MBSSIB_TBL, i, (void *)&Entry)) {  		
  		printf("Error! Get MIB_MBSSIB_TBL(startWLan) error.\n");					
	}
	
	//mib_get( MIB_WLAN_ENCRYPT,  (void *)&vChar);
	//encrypt = (ENCRYPT_T)VCHAR;
	encrypt = (ENCRYPT_T)Entry.encrypt;
	
	//mib_get(MIB_WLAN_DISABLED, (void *)&no_wlan);
	if ( i == 0 ) {
		mib_get(MIB_WLAN_DISABLED, (void *)&no_wlan);
//		strcpy(para, "wlan0");
	} else {
		no_wlan = Entry.wlanDisabled;		
//		sprintf(para, "wlan0-vap%d", i-1);
	}
	
#ifdef WLAN_1x
	//mib_get( MIB_WLAN_ENABLE_1X, (void *)&enable1x);
	enable1x = Entry.enable1X;
#endif

	if (encrypt<=ENCRYPT_WEP && !enable1x && !no_wlan) {
		//itfcfg(para, 0);
		setupWLan();
		goto up_wlan_itf;
		//itfcfg(para, 1);
	}
}

up_wlan_itf:
//xl_yue:up wlan interface
for(i=0;i<5;i++){
	if(i == 0)
		strcpy(para, "wlan0");
	else
		sprintf(para, "wlan0-vap%d", i-1);
	itfcfg(para, 1);
}
#endif
}
#endif

// Added by Mason Yu
int stopwlan()
{		
	int status = 0;
	int wirelessauthpid=0,iwcontrolpid=0, wscdpid=0, upnppid=0;	
	int AuthPid[MAX_WLAN_VAP+1]= { 0, 0, 0, 0, 0};
	int i, flags;
	unsigned char no_wlan;
#ifdef	WLAN_MBSSID
	MIB_CE_MBSSIB_T Entry;
#endif	
	
	// Kill iwcontrol
	iwcontrolpid = read_pid((char*)IWCONTROLPID);
	if(iwcontrolpid > 0){
		kill(iwcontrolpid, 9);
		unlink(IWCONTROLPID);			
	}
	
	// Kill Auth
	for ( i=0; i<=MAX_WLAN_VAP; i++) {
		AuthPid[i] = read_pid((char*)WLANAUTHPID[i]);
		if(AuthPid[i] > 0) {
			kill(AuthPid[i], 9);
			unlink(WLANAUTHCONF[i]);
			unlink(WLANAUTHPID[i]);
			unlink(WLANAUTHFIFO[i]);
		}				
	}

#ifdef CONFIG_WIFI_SIMPLE_CONFIG // WPS	
	// Kill wscd-wlan0.pid
	wscdpid = read_pid((char*)WSCDPID);
	if(wscdpid > 0){
		system("/bin/echo 0 > /proc/gpio");
		kill(wscdpid, 9);
		unlink(WSCDPID);			
	}
	upnppid = read_pid((char*)MINI_UPNPDPID);	//cathy
	if(upnppid > 0){
		kill(upnppid, 9);
		unlink(MINI_UPNPDPID);
		upnppid = read_pid((char*)MINIUPNPDPID);
		if(upnppid > 0){
			va_cmd("/bin/mini_upnpd", 3, 1, "-igd", "/tmp/igd_config", "&");
		}
	}
#endif


	for ( i=0; i<=MAX_WLAN_VAP; i++) {		
		if (getInFlags( WLANAPIF[i], &flags) == 1){		
			if (flags & IFF_UP){
				status |= va_cmd(IFCONFIG, 2, 1, WLANAPIF[i], "down");
				status|=va_cmd(BRCTL, 3, 1, "delif", (char *)BRIF, WLANAPIF[i]);
			}			
		}
	}
}

int config_WLAN( int action_type )
{	
	switch( action_type )
	{
	case ACT_START:
		startWLan();
		break;
		
	case ACT_RESTART:
		stopwlan();
		startWLan();
		break;
		
	case ACT_STOP:
		stopwlan();		
		break;
		
	default:
		return -1;
	}
	return 0;
}

#endif

#ifdef PORT_FORWARD_ADVANCE
int config_PFWAdvance( int action_type )
{	
	switch( action_type )
	{
	case ACT_START:
		startPFWAdvance();
		break;
		
	case ACT_RESTART:
		stopPFWAdvance();
		startPFWAdvance();
		break;
		
	case ACT_STOP:
		stopPFWAdvance();		
		break;
		
	default:
		return -1;
	}
	return 0;
}
#endif

#ifdef SYSLOG_SUPPORT
#define SLOGDPID  "/var/run/syslogd.pid"
#define KLOGDPID  "/var/run/klogd.pid"
#define SLOGDLINK1 "/var/tmp/messages"
#define SLOGDLINK2 "/var/tmp/messages.old"

int stopSlogD()
{
	int slogDid=0;
	int status=0;
	
	slogDid = read_pid((char*)SLOGDPID);
	if(slogDid > 0) {
		kill(slogDid, 9);
		unlink(SLOGDPID);
		unlink(SLOGDLINK1);
		unlink(SLOGDLINK2);
	}
	return 1;
	
}

int stopKlogD()
{
	int klogDid=0;
	int status=0;
	
	klogDid = read_pid((char*)KLOGDPID);
	if(klogDid > 0) {
		kill(klogDid, 9);
		unlink(KLOGDPID);
	}
	return 1;
	
}

int stopLog(){
	unsigned char vChar;
	unsigned int vInt;
	char buffer[100];
	
	mib_get(MIB_ADSL_DEBUG, (void *)&vChar);
	if(vChar==1){
#ifdef CONFIG_USER_BUSYBOX_KLOGD
		// Kill SlogD
		stopSlogD();
		
		// Kill KlogD
		stopKlogD();		
#endif
	}
	else{			
			// Kill SlogD
			stopSlogD();			
#ifdef CONFIG_USER_BUSYBOX_KLOGD
			// Kill KlogD
			stopKlogD();			
#endif		
	}

	return 1;
}


int startLog(){
	unsigned char vChar;
	unsigned int vInt;
	char buffer[100];
	char *argv[30], loglen[8], loglevel[2];
#ifdef SYSLOG_REMOTE_LOG
	char logmode[2], serverip[30], serverport[6];
#endif
	int idx;
	
	mib_get(MIB_MAXLOGLEN, (void *)&vInt);
	snprintf(loglen, 8, "%d", vInt);
	mib_get(MIB_SYSLOG_LOG_LEVEL, (void *)&vChar);
	snprintf(loglevel, 2, "%d", vChar);
#ifdef SYSLOG_REMOTE_LOG
	mib_get(MIB_SYSLOG_MODE, (void *)&vChar);
#endif
	argv[1] = "-n";
	argv[2] = "-s";
	argv[3] = loglen;
	argv[4] = "-l";
	argv[5] = loglevel;
#ifdef SYSLOG_REMOTE_LOG
	idx = 6;
	snprintf(logmode, 2, "%d", vChar);
	if (vChar == 2 || vChar == 3) {
		getMIB2Str(MIB_SYSLOG_SERVER_IP, serverip);
		getMIB2Str(MIB_SYSLOG_SERVER_PORT, serverport);
		snprintf(serverip, 30, "%s:%s", serverip, serverport);
		argv[idx++] = "-R";
		argv[idx++] = serverip;
		if (vChar == 3)
			argv[idx++] = "-L";
	}
	argv[idx++] = NULL;
#else
	argv[6] = NULL;
#endif

	mib_get(MIB_ADSL_DEBUG, (void *)&vChar);
	if(vChar==1){
#ifdef CONFIG_USER_BUSYBOX_KLOGD
		sprintf(buffer, "%u", 100000);
		argv[3] = loglen;
		TRACE(STA_SCRIPT, "/bin/slogd %s %s %s %s %s ...\n", argv[1], argv[2], argv[3], argv[4], argv[5]);
		printf("/bin/slogd %s %s %s %s %s ...\n", argv[1], argv[2], argv[3], argv[4], argv[5]);
		va_cmd("/bin/slogd", argv, 0);
		//va_cmd("/bin/syslogd",3,0,"-n","-s",buffer);
		//va_cmd("/bin/slogd",3,0,"-n","-s",buffer);
		va_cmd("/bin/klogd",1,0,"-n");
		va_cmd("/bin/adslctrl",2,0,"debug","10");
#endif
	}
	else{
		mib_get(MIB_SYSLOG, (void *)&vChar);
		if(vChar==1){
			mib_get(MIB_MAXLOGLEN, (void *)&vInt);
			sprintf(buffer, "%u", vInt);
			TRACE(STA_SCRIPT, "/bin/slogd %s %s %s %s %s ...\n", argv[1], argv[2], argv[3], argv[4], argv[5]);
			printf("/bin/slogd %s %s %s %s %s ...\n", argv[1], argv[2], argv[3], argv[4], argv[5]);
			do_cmd("/bin/slogd", argv, 0);
			//va_cmd("/bin/syslogd",3,0,"-n","-s",buffer);
			//va_cmd("/bin/slogd",3,0,"-n","-s",buffer);
#ifdef CONFIG_USER_BUSYBOX_KLOGD
			va_cmd("/bin/klogd",1,0,"-n");
#endif
		}
	}

	return 1;
}
#endif

#ifdef DEFAULT_GATEWAY_V2
int ifExistedDGW(void) {
	char buff[256];
	int flgs;
	struct in_addr dest, mask;
	FILE *fp;
	if (!(fp = fopen("/proc/net/route", "r"))) {
		printf("Error: cannot open /proc/net/route - continuing...\n");
		return -1;
	}
	fgets(buff, sizeof(buff), fp);
	while (fgets(buff, sizeof(buff), fp) != NULL) {
		if (sscanf(buff, "%*s%x%*x%x%*d%*d%*d%x", &dest, &flgs, &mask) != 3) {
			printf("Unsuported kernel route format\n");
			fclose(fp);
			return -1;
		}
		if ((flgs & RTF_UP) && dest.s_addr == 0 && mask.s_addr == 0) {
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}
#endif

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

#ifdef  WLAN_MBSSID
	if (Entry->idx != 0) 
		encrypt = Entry->encrypt;
	else
#endif
	{
		mib_get( MIB_WLAN_ENCRYPT, (void *)&encrypt);
	}

	sprintf(buf2, "encryption = %d\n", encrypt);
	WRITE_WPA_FILE(fh, buf2);	

#ifdef  WLAN_MBSSID
	if (Entry->idx != 0) 
		strcpy(buf1, Entry->ssid);
	else
#endif
	{
		mib_get( MIB_WLAN_SSID,  (void *)buf1);
	}

	sprintf(buf2, "ssid = \"%s\"\n", buf1);
	WRITE_WPA_FILE(fh, buf2);

#ifdef  WLAN_MBSSID
	if (Entry->idx != 0) 
		enable1x = Entry->enable1X;
	else
#endif
	{
		mib_get( MIB_WLAN_ENABLE_1X, (void *)&enable1x);
	}

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

#ifdef  WLAN_MBSSID
	if (Entry->idx != 0) 
		wep = Entry->wep;
	else
#endif
	{
		mib_get( MIB_WLAN_WEP, (void *)&wep);
	}

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

	
#ifdef  WLAN_MBSSID
	if (Entry->idx != 0) 
		chVal = Entry->wpaAuth;
	else
#endif
	mib_get( MIB_WLAN_WPA_AUTH, (void *)&chVal);

	// Kaohj, PSK only
	sprintf(buf2, "authentication = %d\n", chVal); //1: radius 2:PSK
	WRITE_WPA_FILE(fh, buf2);

//added by xl_yue
#ifdef ENABLE_WPAAES_WPA2TKIP
#ifdef  WLAN_MBSSID
	if (Entry->idx != 0) 
		chVal = Entry->unicastCipher;
	else
#endif
#endif
	mib_get( MIB_WLAN_WPA_CIPHER_SUITE, (void *)&chVal);

	sprintf(buf2, "unicastCipher = %d\n", chVal);
	WRITE_WPA_FILE(fh, buf2);

//added by xl_yue
#ifdef ENABLE_WPAAES_WPA2TKIP
#ifdef  WLAN_MBSSID
	if (Entry->idx != 0) 
		chVal = Entry->wpa2UnicastCipher;
	else
#endif
#endif
	mib_get( MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&chVal);

	sprintf(buf2, "wpa2UnicastCipher = %d\n", chVal);
	WRITE_WPA_FILE(fh, buf2);

/*
	mib_get( MIB_WLAN_WPA2_PRE_AUTH, (void *)&intVal);
	sprintf(buf2, "enablePreAuth = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);
*/

#ifdef  WLAN_MBSSID
	if (Entry->idx != 0) 
		chVal = Entry->wpaPSKFormat;
	else
#endif
	mib_get( MIB_WLAN_WPA_PSK_FORMAT, (void *)&chVal);

	if (chVal==0)
		sprintf(buf2, "usePassphrase = 1\n");
	else
		sprintf(buf2, "usePassphrase = 0\n");
	WRITE_WPA_FILE(fh, buf2);


#ifdef  WLAN_MBSSID
	if (Entry->idx != 0) 
		strcpy(buf1, Entry->wpaPSK);
	else
#endif
	mib_get( MIB_WLAN_WPA_PSK, (void *)buf1);

	sprintf(buf2, "psk = \"%s\"\n", buf1);
	WRITE_WPA_FILE(fh, buf2);

	mib_get( MIB_WLAN_WPA_GROUP_REKEY_TIME, (void *)&intVal);
	sprintf(buf2, "groupRekeyTime = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);

#if 1 // not support RADIUS

#ifdef  WLAN_MBSSID
	if (Entry->idx != 0) 
		sintVal = Entry->rsPort;
	else
#endif
	mib_get( MIB_WLAN_RS_PORT, (void *)&sintVal);

	sprintf(buf2, "rsPort = %d\n", sintVal);
	WRITE_WPA_FILE(fh, buf2);

#ifdef  WLAN_MBSSID
	if (Entry->idx != 0) 
		*((unsigned long *)buf1) = *((unsigned long *)Entry->rsIpAddr);
	else
#endif
	mib_get( MIB_WLAN_RS_IP, (void *)buf1);

	sprintf(buf2, "rsIP = %s\n", inet_ntoa(*((struct in_addr *)buf1)));
	WRITE_WPA_FILE(fh, buf2);

#ifdef  WLAN_MBSSID
	if (Entry->idx != 0) 
		strcpy(buf1, Entry->rsPassword);
	else
#endif
	mib_get( MIB_WLAN_RS_PASSWORD, (void *)buf1);

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

static int is8021xEnabled() {
#ifdef WLAN_1x
	
	unsigned char enable_1x = 0;

	
	
	mib_get( MIB_WLAN_ENABLE_1X, (void *)&enable_1x);

	if (enable_1x) {
		return 1;
	} else {
		unsigned char wpaAuth = 0;
		unsigned char encrypt = 0;
		mib_get( MIB_WLAN_ENCRYPT, (void *)&encrypt);
		//fprintf(stderr, "encrypt=%d\n", encrypt);
		if (encrypt & ENCRYPT_WPA2_MIXED) {
			
			mib_get( MIB_WLAN_WPA_AUTH, (void *)&wpaAuth);
			///fprintf(stderr, "wpaAuth=%d\n", wpaAuth);
			if (WPA_AUTH_AUTO == wpaAuth)
				return 1;

		}
	}

	
#endif
	return 0;
}

#ifdef CONFIG_WIFI_SIMPLE_CONFIG // WPS
int start_WPS()
{
	int status=0;	
	unsigned char encrypt;	
	unsigned char enable1x=0;
	
	int enableWsc = 1, retry;
	unsigned char wsc_disable;
	unsigned char wsc_upnp_enabled;
	unsigned char wlan_mode;
	unsigned char wlan_nettype;
	unsigned char wpa_auth;
	char *cmd_opt[16];
	int cmd_cnt = 0; int idx;
	int wscd_pid_fd = -1;
	
	mib_get(MIB_WSC_DISABLE, (void *)&wsc_disable);
	mib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
	mib_get(MIB_WLAN_NETWORK_TYPE, (void *)&wlan_nettype);
	mib_get(MIB_WLAN_WPA_AUTH, (void *)&wpa_auth);
	mib_get(MIB_WSC_UPNP_ENABLED, (void *)&wsc_upnp_enabled);	

	fprintf(stderr, "START WPS SETUP!\n\n\n");
	cmd_opt[cmd_cnt++] = "";

	if (wsc_disable || is8021xEnabled())
		goto WSC_DISABLE;	
	
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

	if (enableWsc) {
		useAuth_wlan0 = 1;		
	}

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

	//cmd_opt[cmd_cnt++] = "-debug";
	//strcat(cmd, " -debug");
	//strcat(cmd, " &");
	#define TARGDIR "/var/wps/"
	#define SIMPLECFG "simplecfgservice.xml"
	//status |= va_cmd("/bin/flash", 3, 1, "upd-wsc-conf", "/etc/wscd.conf", "/var/wscd.conf");
	status |= va_cmd("/bin/mkdir", 2, 1, "-p", TARGDIR);
	status |= va_cmd("/bin/cp", 2, 1, "/etc/" SIMPLECFG, TARGDIR);

	cmd_opt[cmd_cnt] = 0;
	printf("CMD ARGS: ");
	for (idx=0; idx<cmd_cnt;idx++) 
		printf("%s ", cmd_opt[idx]);
	printf("\n");
	
	status |= do_cmd("/bin/wscd", cmd_opt, 0);

	retry = 0;
	while ((wscd_pid_fd = open("/var/run/wscd-wlan0.pid", O_WRONLY)) == -1)
	{
		usleep(30000);
		retry ++;

		if (retry > 10) {
			printf("wscd invoke timeout, abort\n");
			break;
		}
	}
	
	if(wscd_pid_fd!=-1) close(wscd_pid_fd);/*jiunming, close the opened fd*/
WSC_DISABLE:

	return status;
}
#endif	


int start_Auth()
{
	unsigned char encrypt, no_wlan;
	int auth_pid_fd=-1;
	unsigned char enable1x=0;
	unsigned char wlan_mode;
	int status=0;	
	
#ifdef  WLAN_MBSSID	
	int i=0;
	char para2[15];
	MIB_CE_MBSSIB_T Entry;
	
	char para_auth_conf[30];
	char para_itf_name[15];
	char para_auth_pid[30];	
#ifdef CONFIG_USB_RTL8187SU_SOFTAP  //cathy
	#define RTL8185_IOCTL_GET_MIB	0x89f2
	int skfd;
	struct iwreq wrq;
#endif
#endif
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	unsigned char stringbuf[MAX_PSK_LEN+1];
#endif
		unsigned char value[30];
//#ifndef WLAN_MBSSID
	//added by xl_yue: create random wpa key for rootAP
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		if (mib_get(MIB_ELAN_MAC_ADDR, (void *)value) != 0)
		{
			value[0] = value[0]^value[4]^value[5];
			value[1] = value[1]^value[4]^value[5];
			value[2] = value[2]^value[4]^value[5];
			value[3] = value[3]^value[4]^value[5];
			mib_get(MIB_WLAN_WPA_PSK, (void *)stringbuf);
			if(strlen(stringbuf) < 8){
				snprintf((char *)(stringbuf), 9, "%02x%02x%02x%02x",value[0], value[1], value[2], value[3]);
				mib_set(MIB_WLAN_WPA_PSK, (void *)stringbuf);	
			}
		}
#endif
	
	mib_get(MIB_WLAN_DISABLED, (void *)&no_wlan);
	if (no_wlan == 0)	// WLAN enabled
	{
		// brctl addif br0 wlan0
		status|=va_cmd(BRCTL, 3, 1, "addif", (char *)BRIF, (char *)WLANIF);
		// ifconfig wlan0 up
		status|=va_cmd(IFCONFIG, 2, 1, (char *)WLANIF, "up");
#ifdef WLAN_1x		
		mib_get(MIB_WLAN_ENCRYPT, (void *)&encrypt);
		mib_get(MIB_WLAN_DISABLED, (void *)&no_wlan);
		mib_get(MIB_WLAN_WPA_AUTH, (void *)&wlan_mode);
		mib_get(MIB_WLAN_ENABLE_1X, (void *)&enable1x);

#ifdef CONFIG_USB_RTL8192SU_SOFTAP
		if ((enable1x==1 || WPA_AUTH_AUTO==wlan_mode) && no_wlan == 0) {// 802.1x enabled, auth is only used when 802.1x is enable since encryption is driver based in 11n driver
#else
		if ((encrypt >= 2 || enable1x==1) && no_wlan == 0) {// 802.1x enabled
#endif
			#ifndef WLAN_MBSSID
			status|=generateWpaConf((char *)WLAN_AUTH_CONF, 0);
			#else
			Entry.idx = 0;
			status|=generateWpaConf((char *)WLAN_AUTH_CONF, 0, &Entry);
			#endif
			status|=va_cmd(AUTH, 4, 0, (char *)WLANIF, (char *)LANIF, "auth", (char *)WLAN_AUTH_CONF);
			// fix the depency problem
			// wait for auth-xxx.pid to be created, the iwcontrol will check for it
			while ((auth_pid_fd = open(AUTH_PID, O_WRONLY)) == -1)
			{
				usleep(30000);
			}
			if(auth_pid_fd!=-1) close(auth_pid_fd);/*jiunming, close the opened fd*/
			useAuth_wlan0 = 1;		
		}
		
		//if (useAuth_wlan0) {
		//	status|=va_cmd(IWCONTROL, 1, 1, (char *)WLANIF);
		//}		
#endif
        	status = (status==-1)?-1:1;
	}
	else
		return 0;
	
	
//#else	//MBSSID
#ifdef WLAN_MBSSID
#if defined (CONFIG_USB_RTL8187SU_SOFTAP) || defined(CONFIG_USB_RTL8192SU_SOFTAP)  //cathy
	//CurrentChannel
	mib_get(MIB_WLAN_CHAN_NUM, (void *)value);
	if( value[0] == 0 ){	//root ap is auto channel		
		do {		//wait for selecting channel	
			sleep(1);
			skfd = socket(AF_INET, SOCK_DGRAM, 0);
			strcpy(wrq.ifr_name, "wlan0");
			strcpy(value,"opmode");
			wrq.u.data.pointer = (caddr_t)&value;
			wrq.u.data.length = 10;
			ioctl(skfd, RTL8185_IOCTL_GET_MIB, &wrq);	
			close( skfd );
		}while(value[0] == 0x04);	//WIFI_WAIT_FOR_CHANNEL_SELECT
		
	}
#endif
	// Modified by Mason Yu
	for ( i=1; i<=4; i++) {
		if (!mib_chain_get(MIB_MBSSIB_TBL, i, (void *)&Entry)) {  		
	  		printf("Error! Get MIB_MBSSIB_TBL(startWLan) error.\n");					
		}
	
	//added by xl_yue: create random wpa key
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		if (mib_get(MIB_ELAN_MAC_ADDR, (void *)value) != 0)
		{
			value[0] = value[0]^value[4]^value[5];
			value[1] = value[1]^value[4]^value[5];
			value[2] = value[2]^value[4]^value[5];
			value[3] = value[3]^value[4]^value[5]^i;
			if(strlen(Entry.wpaPSK) < 8){
				snprintf((char *)(stringbuf), 9, "%02x%02x%02x%02x",value[0], value[1], value[2], value[3]);
				memcpy(Entry.wpaPSK,stringbuf,9);
				printf("set MIB_MBSSIB_TBL entry%d wpaPSK = %s\n", i, Entry.wpaPSK);
			}
	
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, i);
		}
#endif
			
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
			//status|=startWLanDaemon(&Entry, ctrlFlags);			
			
			encrypt = Entry.encrypt;			
			enable1x = Entry.enable1X;
 	
			if ( Entry.idx !=0 ) {
				sprintf(para_auth_conf, "/var/config/wlan0-vap%d.conf", Entry.idx - 1); 
				sprintf(para_itf_name, "wlan0-vap%d", Entry.idx - 1);
				sprintf(para_auth_pid, "/var/run/auth-wlan0-vap%d.pid", Entry.idx - 1);
			} else {
				strcpy(para_auth_conf, "/var/config/wlan0.conf");
				strcpy(para_itf_name, "wlan0");
				strcpy(para_auth_pid, "/var/run/auth-wlan0.pid");
			}
	        	
			//if ((encrypt >= 2 || enable1x==1) && no_wlan == 0) {// 802.1x enabled
			if ((encrypt >= 2 || enable1x==1) ) {// 802.1x enabled	
				//status|=generateWpaConf((char *)WLAN_AUTH_CONF, 0);
				status|=generateWpaConf(para_auth_conf, 0, &Entry);				
				
				status|=va_cmd(AUTH, 4, 0, para_itf_name, (char *)LANIF, "auth", para_auth_conf);
				
				// fix the depency problem
				// wait for auth-xxx.pid to be created, the iwcontrol will check for it									
				while ((auth_pid_fd = open(para_auth_pid, O_WRONLY)) == -1)
				{			
					usleep(30000);
					
				}
				if(auth_pid_fd!=-1) close(auth_pid_fd);/*jiunming, close the opened fd*/						
				
				strcpy(para_iwctrl[wlan_num], para_itf_name);
				wlan_num++;	
				
			}		
#endif
	        	status = (status==-1)?-1:1;
		}		
	}	
#endif
	return status;
}

#ifdef WLAN_WEB_REDIRECT  //jiunming,web_redirect
int start_wlan_web_redirect(){

	int status=0;
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
		
	return status;	
}
#endif

int start_iwcontrol()
{
	int tmp, found = 0;
	int status = 0;
	
	// When (1) WPS enabled or (2) Root AP's encryption is WPA/WPA2 without MBSSID,
	// we should start iwcontrol with wlan0 interface.	
	if ( useAuth_wlan0 == 1 ) { 		
		for (tmp=0; tmp < wlan_num; tmp++) {
			if (strcmp(para_iwctrl[tmp], WLANIF)==0) {
				found = 1;
				break;
			}
		}

		if (!found) {
			strcpy(para_iwctrl[wlan_num], WLANIF);
			wlan_num++;
		}
	}	
	
	// Mason Yu
	// We have 5 AP(Root and vap0 ~ vap3., if we add a new vap, we must modified the following codes.
	printf("Total WPA/WPA2 number is %d\n", wlan_num);	
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
	
}

// return value:
// 0  : success
// -1 : failed
int startWLanDaemon()
{	
	int status=0;
		
#ifdef CONFIG_WIFI_SIMPLE_CONFIG
	status |= start_WPS();
#endif
	
	status |= start_Auth();
	
#ifdef WLAN_WEB_REDIRECT
	status |= start_wlan_web_redirect();
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
	unsigned char no_wlan, wsc_disable;
	int status=0, upnppid=0;
	
	// Check wireless interface
	if (!getInFlags((char *)WLANIF, 0)) {
		printf("Wireless Interface Not Found !\n");
		return -1;	// interface not found
	}
	
	//1/17/06' hrchen, always start WLAN MIB, for MP test will use 
	// "ifconfig wlan0 up" to start WLAN
	// config WLAN
	status|=setupWLan();

	// Modified by Mason Yu
	wlan_num = 0; /*reset to 0,jiunming*/
	useAuth_wlan0 = 0;  /*reset to 0 */

	status|=startWLanDaemon();
	
	status|=start_iwcontrol();
#ifdef CONFIG_WIFI_SIMPLE_CONFIG
	mib_get(MIB_WLAN_DISABLED, (void *)&no_wlan);
	mib_get(MIB_WSC_DISABLE, (void *)&wsc_disable);
	upnppid = read_pid((char*)MINI_UPNPDPID);	//cathy
	if( !no_wlan && !wsc_disable ) {
		if(upnppid > 0){
			kill(upnppid, 9);
			unlink(MINI_UPNPDPID);
			va_cmd("/bin/mini_upnpd", 5, 0, "-wsc", "/tmp/wscd_config", "-igd", "/tmp/igd_config", "&");
		}
		else {
			va_cmd("/bin/mini_upnpd", 3, 0, "-wsc", "/tmp/wscd_config", "&");
		}
	}
#endif	
	return status;
}

#endif

#ifdef CONFIG_USER_IGMPPROXY
// IGMP proxy configuration
// return value:
// 1  : successful
// 0  : function not enable
// -1 : startup failed
int startIgmproxy()
{
	unsigned char igmpEnable, igmpItf;
	char ifname[6];
	int igmpproxy_pid;
	
	// Kill old IGMP proxy
	igmpproxy_pid = read_pid((char *)IGMPPROXY_PID);
	if (igmpproxy_pid >= 1) {
		// kill it
		if (kill(igmpproxy_pid, SIGTERM) != 0) {
			printf("Could not kill pid '%d'", igmpproxy_pid);
		}
	}
	
	// check if IGMP proxy enabled ?
	if (mib_get(MIB_IGMP_PROXY, (void *)&igmpEnable) != 0)
	{
		if (igmpEnable != 1)
			return 0;	// IGMP proxy not enable
	}
	if (mib_get(MIB_IGMP_PROXY_ITF, (void *)&igmpItf) != 0)
	{
		if (igmpItf != 0xff)
		{
			if (PPP_INDEX(igmpItf) != 0x0f)
				snprintf(ifname, 6, "ppp%u", PPP_INDEX(igmpItf));	// PPP interface
			else
				snprintf(ifname, 5, "vc%u", VC_INDEX(igmpItf));
		}
		else
		{
			printf("Error: IGMP proxy interface not set !\n");
			return 0;
		}
	}
	
	va_cmd(IGMPROXY, 2, 0, ifname, (char *)LANIF);
	return 1;
}
#endif // of CONFIG_USER_IGMPPROXY

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

void deleteStaticRoute()
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
		route_cfg_modify(&Entry, 1);
	}
}

int RegisterPVCnumber()
{
	int i, vInt, activePVC;
	MIB_CE_ATM_VC_T Entry;
	unsigned char value[32];
	
	vInt = mib_chain_total(MIB_ATM_VC_TBL);
	activePVC = 0;
	for (i = 0; i < vInt; i++)
	{
		/* get the specified chain record */
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			return -1;
		
		if (Entry.enable == 0)
			continue;
		
		activePVC ++;
	}
	sprintf(value,"%d",activePVC);
	if (va_cmd("/bin/sarctl",2,1,"pvcnumber",value))
		return -1;
	
}

// Added by Mason Yu
int deleteAllConnection()
{
	unsigned int entryNum, i, idx;
	MIB_CE_ATM_VC_T Entry;
	
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);			
	for (i = 0; i < entryNum; i++) {				
		/* Retrieve entry */
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) {
			printf("deleteAllConnection: cannot get ATM_VC_TBL(ch=%d) entry\n", i);
			return 0;
		}
        
		/* remove connection on driver*/		
		stopConnection(&Entry);				
	}
#if 0	
	for (i = 0; i < entryNum; i++) {		
		/* remove the mib setting */
		if(mib_chain_delete(MIB_ATM_VC_TBL, i) != 1) {
			printf("deleteAllConnection:Delete MIB_ATM_VC_TBL chain(ch=%d) record error!", i);
			return 0;					
		}
	}
#endif
}

// Added by Mason Yu
int executeAllConnection()
{
	unsigned int entryNum, i, idx;
	MIB_CE_ATM_VC_T Entry;
	
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);			
	for (i = 0; i < entryNum; i++) {				
		/* Retrieve entry */
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) {
			printf("deleteAllConnection: cannot get ATM_VC_TBL(ch=%d) entry\n", i);
			return 0;
		}
        
		/* execute connection on driver*/		
		startConnection(&Entry);				
	}
#if 0	
	for (i = 0; i < entryNum; i++) {		
		/* remove the mib setting */
		if(mib_chain_delete(MIB_ATM_VC_TBL, i) != 1) {
			printf("deleteAllConnection:Delete MIB_ATM_VC_TBL chain(ch=%d) record error!", i);
			return 0;					
		}
	}
#endif
}

void cleanAllFirewallRule()
{
	// Added by Mason Yu. Clean all Firewall rules.
	va_cmd(IPTABLES, 1, 1, "-F");
	va_cmd(IPTABLES, 3, 1, "-t", "nat", "-F");
#ifdef IP_ACL
	va_cmd(IPTABLES, 2, 1, "-X", "aclblock");
#endif
#ifdef CONFIG_EXT_SWITCH
#ifdef VLAN_GROUP
	va_cmd(IPTABLES, 2, 1, "-X", "itfgroup");
#endif
#endif
#ifdef DOMAIN_BLOCKING_SUPPORT
	va_cmd(IPTABLES, 2, 1, "-X", "domainblk");
#endif				
	va_cmd(IPTABLES, 2, 1, "-X", (char *)PORT_FW);
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-X", (char *)PORT_FW);
	
	va_cmd(IPTABLES, 2, 1, "-X", (char *)IPTABLE_DMZ);
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-X", (char *)IPTABLE_DMZ);
	
#ifdef NATIP_FORWARDING	
	va_cmd(IPTABLES, 2, 1, "-X", (char *)IPTABLE_IPFW);
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-X", (char *)IPTABLE_IPFW);
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-X", (char *)IPTABLE_IPFW2);
#endif

#ifdef PORT_FORWARD_ADVANCE				
	va_cmd(IPTABLES, 2, 1, "-X", (char *)FW_PPTP);	
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-X", (char *)FW_PPTP);				
	va_cmd(IPTABLES, 2, 1, "-X", (char *)FW_L2TP);	
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-X", (char *)FW_L2TP);	
#endif

#ifdef PORT_TRIGGERING
	va_cmd(IPTABLES, 2, 1, "-X", (char *)IPTABLES_PORTTRIGGER);
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-X", (char *)IPTABLES_PORTTRIGGER);
#endif

	va_cmd(IPTABLES, 2, 1, "-X", (char *)FW_INACC);
	va_cmd(IPTABLES, 2, 1, "-X", (char *)FW_IPFILTER);
	
#ifdef MAC_FILTER
	va_cmd(IPTABLES, 2, 1, "-X", (char *)FW_MACFILTER);
#endif

#ifdef URL_BLOCKING_SUPPORT
	va_cmd(IPTABLES, 2, 1, "-X", "urlblock");
#endif

#ifdef URL_ALLOWING_SUPPORT
	va_cmd(IPTABLES, 2, 1, "-X", "urlallow");
#endif

#ifdef WEB_REDIRECT_BY_MAC
	va_cmd(IPTABLES, 2, 1, "-t", "nat", "-X", "WebRedirectByMAC");
#endif

#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
	va_cmd(IPTABLES, 2, 1, "-t", "nat", "-X", "CaptivePortal");
#endif

}

int setupIPFilter()
{
	char *argv[20];
	unsigned char value[32], byte;
	int vInt, i, total;
	MIB_CE_IP_PORT_FILTER_T IpEntry;
	char *policy, *filterSIP, *filterDIP, srcPortRange[12], dstPortRange[12];
	char  srcip[20], dstip[20];
	
	// Delete ipfilter rule
	va_cmd(IPTABLES, 2, 1, "-F", (char *)FW_IPFILTER);
	
	// packet filtering
	// ip filtering
	total = mib_chain_total(MIB_IP_PORT_FILTER_TBL);
	// Add chain for ip filtering
	// iptables -N ipfilter
	//va_cmd(IPTABLES, 2, 1, "-N", (char *)FW_IPFILTER);
	
	// accept related
	// iptables -A ipfilter -m state --state ESTABLISHED,RELATED -j RETURN
	va_cmd(IPTABLES, 8, 1, (char *)FW_ADD, (char *)FW_IPFILTER, "-m", "state",
		"--state", "ESTABLISHED,RELATED", "-j", (char *)FW_RETURN);
	// iptables -A ipfilter -d 224.0.0.0/4 -j RETURN
	va_cmd(IPTABLES, 6, 1, (char *)FW_ADD, (char *)FW_IPFILTER, "-d",
		"224.0.0.0/4", "-j", (char *)FW_RETURN);
	
	
	for (i = 0; i < total; i++)
	{
		int idx=0;
		/*
		 *	srcPortRange: src port
		 *	dstPortRange: dst port
		 */
		if (!mib_chain_get(MIB_IP_PORT_FILTER_TBL, i, (void *)&IpEntry))
			return -1;
		
		if (IpEntry.action == 0)
			policy = (char *)FW_DROP;
		else
			policy = (char *)FW_RETURN;
		
		// source port
		if (IpEntry.srcPortFrom == 0)
			srcPortRange[0]='\0';
		else if (IpEntry.srcPortFrom == IpEntry.srcPortTo)
			snprintf(srcPortRange, 12, "%u", IpEntry.srcPortFrom);
		else
			snprintf(srcPortRange, 12, "%u:%u",
				IpEntry.srcPortFrom, IpEntry.srcPortTo);
		
		// destination port
		if (IpEntry.dstPortFrom == 0)
			dstPortRange[0]='\0';
		else if (IpEntry.dstPortFrom == IpEntry.dstPortTo)
			snprintf(dstPortRange, 12, "%u", IpEntry.dstPortFrom);
		else
			snprintf(dstPortRange, 12, "%u:%u",
				IpEntry.dstPortFrom, IpEntry.dstPortTo);
		
		// source ip, mask
		strncpy(srcip, inet_ntoa(*((struct in_addr *)IpEntry.srcIp)), 16);
		if (strcmp(srcip, ARG_0x4) == 0)
			filterSIP = 0;
		else {
			if (IpEntry.smaskbit!=0)
				snprintf(srcip, 20, "%s/%d", srcip, IpEntry.smaskbit);
			filterSIP = srcip;
		}
		
		// destination ip, mask
		strncpy(dstip, inet_ntoa(*((struct in_addr *)IpEntry.dstIp)), 16);
		if (strcmp(dstip, ARG_0x4) == 0)
			filterDIP = 0;
		else {
			if (IpEntry.dmaskbit!=0)
				snprintf(dstip, 20, "%s/%d", dstip, IpEntry.dmaskbit);
			filterDIP = dstip;
		}
		
		// interface
		argv[1] = (char *)FW_ADD;
		argv[2] = (char *)FW_IPFILTER;
		argv[3] = (char *)ARG_I;
		
		if (IpEntry.dir == DIR_OUT) {
			argv[4] = (char *)LANIF;
			idx = 5;
		}
		else {
			argv[4] = "!";
			argv[5] = (char *)LANIF;
			idx = 6;
		}
		
		// protocol
		if (IpEntry.protoType != PROTO_NONE) {
			argv[idx++] = "-p";
			if (IpEntry.protoType == PROTO_TCP)
				argv[idx++] = (char *)ARG_TCP;
			else if (IpEntry.protoType == PROTO_UDP)
				argv[idx++] = (char *)ARG_UDP;
			else //if (IpEntry.protoType == PROTO_ICMP)
				argv[idx++] = (char *)ARG_ICMP;
		}
		
		// src ip
		if (filterSIP != 0)
		{
			argv[idx++] = "-s";
			argv[idx++] = filterSIP;
			
		}
		
		// src port
		if ((IpEntry.protoType==PROTO_TCP ||
			IpEntry.protoType==PROTO_UDP) &&
			srcPortRange[0] != 0) {
			argv[idx++] = (char *)FW_SPORT;
			argv[idx++] = srcPortRange;
		}
		
		// dst ip
		if (filterDIP != 0)
		{
			argv[idx++] = "-d";
			argv[idx++] = filterDIP;
		}
		// dst port
		if ((IpEntry.protoType==PROTO_TCP ||
			IpEntry.protoType==PROTO_UDP) &&
			dstPortRange[0] != 0) {
			argv[idx++] = (char *)FW_DPORT;
			argv[idx++] = dstPortRange;
		}
		
		// target/jump		
		argv[idx++] = "-j";
		argv[idx++] = policy;
		argv[idx++] = NULL;
		
		//printf("idx=%d\n", idx);
		TRACE(STA_SCRIPT, "%s %s %s %s %s %s %s ...\n", IPTABLES, argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]);
		do_cmd(IPTABLES, argv, 1);
	}
	
#ifdef  CONFIG_USER_PPPOE_PROXY
       char ppp_proxyif[6];
	char ppp_proxynum[2];
	int proxy_index;
	
	for(proxy_index = WAN_PPP_INTERFACE; proxy_index< WAN_PPP_INTERFACE+LAN_PPP_INTERFACE;proxy_index++){
		snprintf(ppp_proxyif, 6, "ppp%u", proxy_index);
		va_cmd (IPTABLES, 6, 1, (char *) FW_ADD, (char *) FW_IPFILTER, (char *)
           ARG_I, ppp_proxyif, "-j", (char *) FW_RETURN);
		}
#endif

	// allow DMZ to pass
	// Mason Yu	
	#if 0
	if ((mib_get(MIB_DMZ_ENABLE, (void *)&byte) != 0) && byte) {
		if (mib_get(MIB_DMZ_IP, (void *)value) != 0)
		{
			char *ipaddr = srcip;
			strncpy(ipaddr, inet_ntoa(*((struct in_addr *)value)), 16);
			ipaddr[15] = '\0';
		
			total = mib_chain_total (MIB_IP_PORT_FILTER_TBL) + 3;
			snprintf(value, 8, "%d", total);
			// iptables -I ipfilter 3 -i ! $LAN_IF -o $LAN_IF -d $DMZ_IP -j RETURN
			va_cmd(IPTABLES, 12, 1, "-I",
				(char *)FW_IPFILTER, value, (char *)ARG_I, "!",
				(char *)LANIF, (char *)ARG_O,
				(char *)LANIF, "-d", ipaddr, "-j", (char *)FW_RETURN);
		}	
	}
	#endif
	// iptables -A FORWARD -j ipfilter
	//va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", (char *)FW_IPFILTER);
}

int setupMacFilter()
{
	unsigned char value[32];
	int vInt, i, total, vcNum;
	char *policy;
	char srcmacaddr[18], dstmacaddr[18];
#ifdef MAC_FILTER
	MIB_CE_MAC_FILTER_T MacEntry;
	char mac_in_dft, mac_out_dft;
	char eth_mac_ctrl=0, wlan_mac_ctrl=0;
#endif
	unsigned char vChar;
	MIB_CE_ATM_VC_T Entry;
	char str_vc[] = "vc0";
	
	// Delete all Macfilter rule
	va_cmd(IPTABLES, 2, 1, "-F", (char *)FW_MACFILTER);
#ifdef PARENTAL_CTRL
	parent_ctrl_table_init();
	parent_ctrl_table_rule_update();
#endif	
#ifdef MAC_FILTER
	// MAC filtering support in bridge mode only
	total = ifWanNum("br");
	vInt = mib_chain_total(MIB_MAC_FILTER_TBL);
	mib_get(MIB_MACF_OUT_ACTION, (void *)&mac_out_dft);
	mib_get(MIB_MACF_IN_ACTION, (void *)&mac_in_dft);
	mib_get(MIB_ETH_MAC_CTRL, (void *)&eth_mac_ctrl);
	mib_get(MIB_WLAN_MAC_CTRL, (void *)&wlan_mac_ctrl);
#endif
#ifdef IP_QOS
	// Added by Nason Yu for brnf enable/disable buttom	
	// If user set some rules on MAC Filtering or enable IPQos, we should enable Bridge netfilter.
	// And we enable/disable bridge netfilter once in order to avoid double setting.
	mib_get(MIB_MPMODE, (void *)&vChar);
#endif

	
#if (defined IP_QOS) || (defined MAC_FILTER)
	if (
#ifdef MAC_FILTER
		total >= 1 && ((mac_out_dft==0 || mac_in_dft==0) || vInt >= 1)
#endif
#ifdef IP_QOS
#ifdef MAC_FILTER
		||
#endif
		(vChar&0x02)
#endif
	) {
		// Enable Bridge Netfiltering					
		//va_cmd("/bin/brctl", 2, 0, "brnf", "on");
		brnfctrl(1,BRNF_MODULE_MAC_FILTER);
	}else {
		// Disable Bridge Netfiltering
		//va_cmd("/bin/brctl", 2, 0, "brnf", "off");
		brnfctrl(0,BRNF_MODULE_MAC_FILTER);
	}
#endif
	
#ifdef MAC_FILTER
	//if (total >= 1) 
	{
		total = mib_chain_total(MIB_MAC_FILTER_TBL);
		vcNum = mib_chain_total(MIB_ATM_VC_TBL);
		
		// Commented by Mason Yu
		//if (total >= 1) {
			// Add chain for MAC filtering
			// iptables -N macfilter
			//va_cmd(IPTABLES, 2, 1, "-N", (char *)FW_MACFILTER);
		//}
		
		for (i = 0; i < total; i++)
		{
			if (!mib_chain_get(MIB_MAC_FILTER_TBL, i, (void *)&MacEntry))
				return -1;
			
			if (MacEntry.action == 0)
				policy = (char *)FW_DROP;
			else
				policy = (char *)FW_RETURN;
				
			
			snprintf(srcmacaddr, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
				MacEntry.srcMac[0], MacEntry.srcMac[1],
				MacEntry.srcMac[2], MacEntry.srcMac[3],
				MacEntry.srcMac[4], MacEntry.srcMac[5]);
			
			
				
			snprintf(dstmacaddr, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
				MacEntry.dstMac[0], MacEntry.dstMac[1],
				MacEntry.dstMac[2], MacEntry.dstMac[3],
				MacEntry.dstMac[4], MacEntry.dstMac[5]);	
			
			
			
			// Added by Mason Yu for Incoming MAC filtering
			if (MacEntry.dir == DIR_OUT) {
				if (   MacEntry.dstMac[0]==0 && MacEntry.dstMac[1]==0
			    	    && MacEntry.dstMac[2]==0 && MacEntry.dstMac[3]==0
		 	    	    && MacEntry.dstMac[4]==0 && MacEntry.dstMac[5]==0 ) {
					if(mac_out_dft || eth_mac_ctrl)
						// iptables -A macfilter -i eth0  -m mac --mac-source $MAC -j ACCEPT/DROP
						va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
							(char *)ARG_I, (char *)ELANIF, "-m", "mac",
							"--mac-source",  srcmacaddr, "-j", policy);	
#ifdef WLAN_SUPPORT
					if(mac_out_dft || wlan_mac_ctrl)
						// iptables -A macfilter -i wlan0  -m mac --mac-source $MAC -j ACCEPT/DROP
						va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
							(char *)ARG_I, (char *)WLANIF, "-m", "mac",
							"--mac-source",  srcmacaddr, "-j", policy);	
#endif
#ifdef CONFIG_USB_ETH
						// iptables -A macfilter -i usb0  -m mac --mac-source $MAC -j ACCEPT/DROP
						va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
							(char *)ARG_I, (char *)USBETHIF, "-m", "mac",
							"--mac-source",  srcmacaddr, "-j", policy);	
#endif //CONFIG_USB_ETH
					
				}else if (   MacEntry.srcMac[0]==0 && MacEntry.srcMac[1]==0
			    	  	  && MacEntry.srcMac[2]==0 && MacEntry.srcMac[3]==0
		 	                  && MacEntry.srcMac[4]==0 && MacEntry.srcMac[5]==0 ) { 
				
					// iptables -A macfilter -i eth0  -m mac --mac-dst $MAC -j ACCEPT/DROP
					va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
						(char *)ARG_I, (char *)ELANIF, "-m", "mac",
						"--mac-dst",  dstmacaddr, "-j", policy);	
#ifdef WLAN_SUPPORT
					// iptables -A macfilter -i wlan0  -m mac --mac-dst $MAC -j ACCEPT/DROP
					va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
						(char *)ARG_I, (char *)WLANIF, "-m", "mac",
						"--mac-dst",  dstmacaddr, "-j", policy);
#endif
#ifdef CONFIG_USB_ETH
					// iptables -A macfilter -i usb0  -m mac --mac-dst $MAC -j ACCEPT/DROP
					va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
						(char *)ARG_I, (char *)USBETHIF, "-m", "mac",
						"--mac-dst",  dstmacaddr, "-j", policy);
#endif //CONFIG_USB_ETH		
				}else {
					// iptables -A macfilter -i eth0  -m mac --mac-source $MAC --mac-dst $MAC -j ACCEPT/DROP
					va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
						(char *)ARG_I, (char *)ELANIF, "-m", "mac",
						"--mac-source",  srcmacaddr, "--mac-dst",  dstmacaddr, "-j", policy);	
#ifdef WLAN_SUPPORT
					// iptables -A macfilter -i wlan0  -m mac --mac-source $MAC --mac-dst $MAC -j ACCEPT/DROP
					va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
						(char *)ARG_I, (char *)WLANIF, "-m", "mac",
						"--mac-source",  srcmacaddr, "--mac-dst",  dstmacaddr, "-j", policy);
#endif
#ifdef CONFIG_USB_ETH
					// iptables -A macfilter -i usb0  -m mac --mac-source $MAC --mac-dst $MAC -j ACCEPT/DROP
					va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
						(char *)ARG_I, (char *)USBETHIF, "-m", "mac",
						"--mac-source",  srcmacaddr, "--mac-dst",  dstmacaddr, "-j", policy);
#endif //CONFIG_USB_ETH

				}
					
			}
			else {
				for (vInt=0; vInt<vcNum; vInt++) {
					if (!mib_chain_get(MIB_ATM_VC_TBL, vInt, (void *)&Entry))
						continue;
					if (Entry.enable == 0)
						continue;
					#ifdef ZTE_GENERAL_ROUTER_SC
					  char WanIfName[64]={0};
					if(Entry.cmode==ADSL_PPPoA||Entry.cmode==ADSL_PPPoE)							
						  snprintf(WanIfName,sizeof(WanIfName),"ppp%d",PPP_INDEX(Entry.ifIndex));						
					else
						snprintf(WanIfName,sizeof(WanIfName),"vc%d",VC_INDEX(Entry.ifIndex));
					if(!memcmp(MacEntry.dstMac,"\x00\x00\x00\x00\x00\x00",MAC_ADDR_LEN))
						  	{
						  	va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
								(char *)ARG_I, WanIfName, "-m", "mac",
								"--mac-source",  srcmacaddr, "-j", policy);
						  	}
						  else   if(!memcmp(MacEntry.srcMac,"\x00\x00\x00\x00\x00\x00",MAC_ADDR_LEN))
						  	{
						  	va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
								(char *)ARG_I, WanIfName, "-m", "mac",
								"--mac-dst",  dstmacaddr, "-j", policy);
						  	}else{
						  		va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
								(char *)ARG_I, WanIfName, "-m", "mac",
								"--mac-source",  srcmacaddr, "--mac-dst",  dstmacaddr, "-j", policy);
						  		}
						
					#else
					if (Entry.cmode == ADSL_BR1483) {
						// vc index should be less than 10
						str_vc[2] = '0'+VC_INDEX(Entry.ifIndex);
						
						if (   MacEntry.dstMac[0]==0 && MacEntry.dstMac[1]==0
			    	    		    && MacEntry.dstMac[2]==0 && MacEntry.dstMac[3]==0
		 	    	    		    && MacEntry.dstMac[4]==0 && MacEntry.dstMac[5]==0 ) {
						
							// iptables -A macfilter -i vc0  -m mac --mac-source $MAC -j ACCEPT/DROP
							va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
								(char *)ARG_I, str_vc, "-m", "mac",
								"--mac-source",  srcmacaddr, "-j", policy);
								
						}else if (   MacEntry.srcMac[0]==0 && MacEntry.srcMac[1]==0
			    	  	  		  && MacEntry.srcMac[2]==0 && MacEntry.srcMac[3]==0
		 	                  		  && MacEntry.srcMac[4]==0 && MacEntry.srcMac[5]==0 ) { 
		 	                  		  		
							// iptables -A macfilter -i vc0  -m mac --mac-dst $MAC -j ACCEPT/DROP
							va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
								(char *)ARG_I, str_vc, "-m", "mac",
								"--mac-dst",  dstmacaddr, "-j", policy);
								
						}else {
							// iptables -A macfilter -i vc0  -m mac --mac-source $MAC --mac-dst $MAC -j ACCEPT/DROP
							va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
								(char *)ARG_I, str_vc, "-m", "mac",
								"--mac-source",  srcmacaddr, "--mac-dst",  dstmacaddr, "-j", policy);
						}	
						
							
					}
				#endif
				}
			}
		}
		
		// Commented by Mason Yu
		//if (total >= 1) {
			// default action
			if (mib_get(MIB_MACF_OUT_ACTION, (void *)value) != 0)
			{
				vInt = (int)(*(unsigned char *)value);
				if (vInt == 0 && eth_mac_ctrl )
				{
					// iptables -A macfilter -i eth0 -j DROP					
					va_cmd(IPTABLES, 6, 1, (char *)FW_ADD,
						(char *)FW_MACFILTER, (char *)ARG_I,
						(char *)ELANIF, "-j", (char *)FW_DROP);
				}
#ifdef WLAN_SUPPORT
				if (vInt == 0 && wlan_mac_ctrl )
				{
					// iptables -A macfilter -i wlan0 -j DROP					
					va_cmd(IPTABLES, 6, 1, (char *)FW_ADD,
						(char *)FW_MACFILTER, (char *)ARG_I,
						(char *)WLANIF, "-j", (char *)FW_DROP);
				}				
#endif
#ifdef CONFIG_USB_ETH
				if (vInt == 0)
				{
					// iptables -A macfilter -i usb0 -j DROP					
					va_cmd(IPTABLES, 6, 1, (char *)FW_ADD,
						(char *)FW_MACFILTER, (char *)ARG_I,
						(char *)USBETHIF, "-j", (char *)FW_DROP);
				}
#endif //CONFIG_USB_ETH
			}
			
			if (mib_get(MIB_MACF_IN_ACTION, (void *)value) != 0)
			{
				vInt = (int)(*(unsigned char *)value);
				if (vInt == 0)	// DROP
				{
					for (vInt=0; vInt<vcNum; vInt++) {
						if (!mib_chain_get(MIB_ATM_VC_TBL, vInt, (void *)&Entry))
							continue;
						if (Entry.enable == 0)
							continue;
					#ifdef ZTE_GENERAL_ROUTER_SC
					  char WanIfName[64]={0};
					if(Entry.cmode==ADSL_PPPoA||Entry.cmode==ADSL_PPPoE)							
						  snprintf(WanIfName,sizeof(WanIfName),"ppp%d",PPP_INDEX(Entry.ifIndex));						
					else
						snprintf(WanIfName,sizeof(WanIfName),"vc%d",VC_INDEX(Entry.ifIndex));
					va_cmd(IPTABLES, 6, 1, (char *)FW_ADD,
								(char *)FW_MACFILTER, (char *)ARG_I,
								WanIfName, "-j", (char *)FW_DROP);
					#else
						if (Entry.cmode == ADSL_BR1483) {
							// vc index should be less than 10
							str_vc[2] = '0'+VC_INDEX(Entry.ifIndex);
							// iptables -A macfilter -i vc0 -j DROP							
							va_cmd(IPTABLES, 6, 1, (char *)FW_ADD,
								(char *)FW_MACFILTER, (char *)ARG_I,
								str_vc, "-j", (char *)FW_DROP);
						}
					#endif
					}
				}
			}
			// iptables -A FORWARD -j macfilter
			//va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", (char *)FW_MACFILTER);
		// Commented by Mason Yu
		//}
	}
#endif // of MAC_FILTER
	
}

#ifdef LAYER7_FILTER_SUPPORT
int setupAppFilter()
{
	int entryNum,i;
	LAYER7_FILTER_T Entry;

	// Delete all Appfilter rule
	va_cmd(IPTABLES, 2, 1, "-F", (char *)FW_APPFILTER);
	va_cmd(IPTABLES, 2, 1, "-F", (char *)FW_APP_P2PFILTER);
	va_cmd(IPTABLES, 4, 1, ARG_T, "mangle", "-F", (char *)FW_APPFILTER);

	entryNum = mib_chain_total(MIB_LAYER7_FILTER_TBL);

	for(i=0;i<entryNum;i++)
	{
		if (!mib_chain_get(MIB_LAYER7_FILTER_TBL, i, (void *)&Entry))
			return -1;

		if(!strcmp("smtp",Entry.appname)){
			//iptables -A appfilter -i br0 -p TCP --dport 25 -j DROP
			va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_APPFILTER, ARG_I, LANIF,
				"-p", ARG_TCP, FW_DPORT, "25", "-j", (char *)FW_DROP);
			continue;
		}
		else if(!strcmp("pop3",Entry.appname)){
			//iptables -A appfilter -i br0 -p TCP --dport 110 -j DROP
			va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_APPFILTER, ARG_I, LANIF,
				"-p", ARG_TCP, FW_DPORT, "110", "-j", (char *)FW_DROP);
			continue;
		}
		else if(!strcmp("bittorrent",Entry.appname)){
			//iptables -A appp2pfilter -m ipp2p --bit -j DROP
			va_cmd(IPTABLES, 7, 1, (char *)FW_ADD, (char *)FW_APP_P2PFILTER, "-m", "ipp2p",
				"--bit", "-j", (char *)FW_DROP);
			continue;
		}
		else if(!strcmp("chinagame",Entry.appname)){
			//iptables -A appfilter -i br0 -p TCP --dport 8000 -j DROP
			va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_APPFILTER, ARG_I, LANIF,
				"-p", ARG_TCP, FW_DPORT, "8000", "-j", (char *)FW_DROP);
			continue;
		}
		else if(!strcmp("gameabc",Entry.appname)){
			//iptables -A appfilter -i br0 -p TCP --dport 5100 -j DROP
			va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_APPFILTER, ARG_I, LANIF,
				"-p", ARG_TCP, FW_DPORT, "5100", "-j", (char *)FW_DROP);
			continue;
		}
		else if(!strcmp("haofang",Entry.appname)){
			//iptables -A appfilter -i br0 -p TCP --dport 1201 -j DROP
			va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_APPFILTER, ARG_I, LANIF,
				"-p", ARG_TCP, FW_DPORT, "1201", "-j", (char *)FW_DROP);
			continue;
		}
		else if(!strcmp("ourgame",Entry.appname)){
			//iptables -A appfilter -i br0 -p TCP --dport 2000 -j DROP
			va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_APPFILTER, ARG_I, LANIF,
				"-p", ARG_TCP, FW_DPORT, "2000", "-j", (char *)FW_DROP);
		}
			
		
		// iptables -t mangle -A appfilter -m layer7 --l7proto qq -j DROP
		va_cmd(IPTABLES, 10, 1, ARG_T, "mangle", (char *)FW_ADD, (char *)FW_APPFILTER, "-m", "layer7",
			"--l7proto", Entry.appname, "-j", (char *)FW_DROP);

	}

	return 0;
}
#endif
#ifdef PARENTAL_CTRL
//Uses Global variable for keep watching timeout
MIB_PARENT_CTRL_T parentctrltable[MAX_PARENTCTRL_USER_NUM] = {0};
/********************************
 *
 *	Initialization. load from flash
 *	
 ********************************/
//return if this mac should be filtered now!
static int parent_ctrl_check(MIB_PARENT_CTRL_T *entry)
{
	time_t tm;
	struct tm the_tm;
	int		tmp1,tmp2,tmp3;;
	
	time(&tm);
	memcpy(&the_tm, localtime(&tm), sizeof(the_tm));

	if (((entry->controlled_day) & (1 << the_tm.tm_wday))!=0)
	{
		tmp1 = entry->start_hr * 60 +  entry->start_min;
		tmp2 = entry->end_hr * 60 +  entry->end_min;
		tmp3 = the_tm.tm_hour *60 + the_tm.tm_min;
		if ((tmp3 >= tmp1) && (tmp3 <= tmp2) )
			return 1;	
	}

		return 0;

}
int parent_ctrl_table_init()
{
	int total,i;
	MIB_PARENT_CTRL_T	entry;

	memset(&parentctrltable[0],0, sizeof(MIB_PARENT_CTRL_T)*MAX_PARENTCTRL_USER_NUM);
	total = mib_chain_total(MIB_PARENTAL_CTRL_TBL);
	if (total >= MAX_PARENTCTRL_USER_NUM)
	{
		total = MAX_PARENTCTRL_USER_NUM -1;
		printf("ERROR, CHECK!");
	}
	 for ( i=0; i<total; i++)
	 {
		mib_chain_get(MIB_PARENTAL_CTRL_TBL, i, &parentctrltable[i]);
	 }
}
int parent_ctrl_table_add(MIB_PARENT_CTRL_T *addedEntry)
{
	int	i;

	for (i = 0; i < MAX_PARENTCTRL_USER_NUM; i++)
	{
		if (strlen(parentctrltable[i].username) == 0)
		{
			break;
		}
	}
	addedEntry->cur_state = 0;
	memcpy (&parentctrltable[i],addedEntry, sizeof(MIB_PARENT_CTRL_T));

	parent_ctrl_table_rule_update();
}
int parent_ctrl_table_del(MIB_PARENT_CTRL_T *addedEntry)
{
	int	i;
	char macaddr[20];
	
	for (i = 0; i < MAX_PARENTCTRL_USER_NUM; i++)
	{
		if ( !strcmp(parentctrltable[i].username, addedEntry->username)) 
		{
			if (parentctrltable[i].cur_state == 1)
			{
				//del the entry
					snprintf(macaddr, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
					parentctrltable[i].mac[0], parentctrltable[i].mac[1],
					parentctrltable[i].mac[2], parentctrltable[i].mac[3],
					parentctrltable[i].mac[4], parentctrltable[i].mac[5]);	
				va_cmd(IPTABLES, 10, 1, (char *)FW_DEL, (char *)FW_MACFILTER,
						(char *)ARG_I, (char *)WLANIF, "-m", "mac",
						"--mac-source",  macaddr, "-j", "DROP");	
				va_cmd(IPTABLES, 10, 1, (char *)FW_DEL, (char *)FW_MACFILTER,
						(char *)ARG_I, (char *)ELANIF, "-m", "mac",
						"--mac-source",  macaddr, "-j", "DROP");				
			}
			//remove the iptable here if it exists
			memset(&parentctrltable[i],0, sizeof(MIB_PARENT_CTRL_T));			
			break;
		}
	}
}
int parent_ctrl_table_delall()
{
	int	i;
	char macaddr[20];

	for (i = 0; i < MAX_PARENTCTRL_USER_NUM; i++)
	{
		if (parentctrltable[i].cur_state == 1)
		{
			//del the entry
			snprintf(macaddr, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
				parentctrltable[i].mac[0], parentctrltable[i].mac[1],
				parentctrltable[i].mac[2], parentctrltable[i].mac[3],
				parentctrltable[i].mac[4], parentctrltable[i].mac[5]);	
			va_cmd(IPTABLES, 10, 1, (char *)FW_DEL, (char *)FW_MACFILTER,
					(char *)ARG_I, (char *)WLANIF, "-m", "mac",
					"--mac-source",  macaddr, "-j", "DROP");	
			va_cmd(IPTABLES, 10, 1, (char *)FW_DEL, (char *)FW_MACFILTER,
					(char *)ARG_I, (char *)ELANIF, "-m", "mac",
					"--mac-source",  macaddr, "-j", "DROP");				
		}
				
	}	
	memset(&parentctrltable[0],0, sizeof(MIB_PARENT_CTRL_T)*MAX_PARENTCTRL_USER_NUM);
}

// update the rules to iptables according to current time
int parent_ctrl_table_rule_update()
{
	int i, check;
	char macaddr[20];
	
	for (i = 0; i < MAX_PARENTCTRL_USER_NUM; i++)
	{
		if (strlen(parentctrltable[i].username) > 0) 
		{
			
			check = parent_ctrl_check(&parentctrltable[i]);
			
			if (( check == 1) &&
				(parentctrltable[i].cur_state == 0))
			{
				parentctrltable[i].cur_state = 1;
			
				//add the entry
				snprintf(macaddr, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
				parentctrltable[i].mac[0], parentctrltable[i].mac[1],
				parentctrltable[i].mac[2], parentctrltable[i].mac[3],
				parentctrltable[i].mac[4], parentctrltable[i].mac[5]);	

				//for debug							
				va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
						(char *)ARG_I, (char *)WLANIF, "-m", "mac",
						"--mac-source",  macaddr, "-j", "DROP");	
				va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
						(char *)ARG_I, (char *)ELANIF, "-m", "mac",
						"--mac-source",  macaddr, "-j", "DROP");	
			}
			else if ((check == 0) &&
				(parentctrltable[i].cur_state == 1))
			{
				parentctrltable[i].cur_state = 0;
				//del the entry
				snprintf(macaddr, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
				parentctrltable[i].mac[0], parentctrltable[i].mac[1],
				parentctrltable[i].mac[2], parentctrltable[i].mac[3],
				parentctrltable[i].mac[4], parentctrltable[i].mac[5]);	

				va_cmd(IPTABLES, 10, 1, (char *)FW_DEL, (char *)FW_MACFILTER,
						(char *)ARG_I, (char *)WLANIF, "-m", "mac",
						"--mac-source",  macaddr, "-j", "DROP");	
				va_cmd(IPTABLES, 10, 1, (char *)FW_DEL, (char *)FW_MACFILTER,
						(char *)ARG_I, (char *)ELANIF, "-m", "mac",
						"--mac-source",  macaddr, "-j", "DROP");	
			}
			
		}
	}
}


#endif

#ifdef CONFIG_USER_RTK_VOIP
#include "web_voip.h"
/*----------------------------------------------------------------------------
 * Name:
 *      voip_setup_iptable
 * Descriptions:
 *      Creat an iptable rule to allow incoming VoIP calls.
 * return:              none
 *---------------------------------------------------------------------------*/
void voip_setup_iptable()
{
        char portbuff[10];
	voipCfgParam_t * pCfg;
	voipCfgPortParam_t *VoIPport;
	int i,val;

	val=voip_flash_get( &pCfg);
	if (val != 0)
	{
		for ( i=5060; i<=5061; i++)	//default value
		{
			sprintf(portbuff, "%d",i);	
  			va_cmd(IPTABLES, 11, 1, "-I", (char *)FW_INPUT,
             	   (char *)ARG_I, "!", (char *)LANIF, "-p", (char *)ARG_UDP,
	                (char *)FW_DPORT,portbuff, "-j", (char* ) "ACCEPT");
		}
	}
	else
	{
        for ( i=0; i<VOIP_PORTS; i++)
        {
                VoIPport = &pCfg->ports[i];
	        sprintf(portbuff, "%d",VoIPport->sip_port);
      		  va_cmd(IPTABLES, 11, 1, "-I", (char *)FW_INPUT,
                (char *)ARG_I, "!", (char *)LANIF, "-p", (char *)ARG_UDP,
                (char *)FW_DPORT,portbuff, "-j", (char* ) "ACCEPT");
        }		
}
}
#endif

#ifdef ZTE_GENERAL_ROUTER_SC
int setupVtlsvr()
{
	int i, total;
	MIB_CE_VTL_SVR_T Entry;
	char srcPortRange[12], dstPortRange[12], dstPort[12];
	char dstip[32];
	char ipAddr[20];
	//char *act;
	char ifname[16];
	
	total = mib_chain_total(MIB_VIRTUAL_SVR_TBL);
	// attach the vrtsvr rules to the chain for ip filtering
	
	for (i = 0; i < total; i++)
	{
		/*
		 *	srcPortRange: src port
		 *	dstPortRange: dst port
		 */
		if (!mib_chain_get(MIB_VIRTUAL_SVR_TBL, i, (void *)&Entry))
			return -1;

		// destination ip(server ip)
		strncpy(dstip, inet_ntoa(*((struct in_addr *)Entry.svrIpAddr)), 16);
		snprintf(ipAddr, 20, "%s/%d", dstip, 32);
		

		//wan port
		if (Entry.wanStartPort == 0)
			srcPortRange[0]='\0';
		else if (Entry.wanStartPort == Entry.wanEndPort)
			snprintf(srcPortRange, 12, "%u", Entry.wanStartPort);
		else
			snprintf(srcPortRange, 12, "%u:%u", 
				Entry.wanStartPort, Entry.wanEndPort);
		
		// server port
		if (Entry.svrStartPort == 0)
			dstPortRange[0]='\0';
		else {
			if (Entry.svrStartPort == Entry.svrEndPort) {
				snprintf(dstPortRange, 12, "%u", Entry.svrStartPort);
				snprintf(dstPort, 12, "%u", Entry.svrStartPort);
			} else {
				snprintf(dstPortRange, 12, "%u-%u",
					Entry.svrStartPort, Entry.svrEndPort);
				snprintf(dstPort, 12, "%u:%u", 
					Entry.svrStartPort, Entry.svrEndPort);
			}
			snprintf(dstip, 32, "%s:%s", dstip, dstPortRange);
		}
		
		//act = (char *)FW_ADD;
		// interface
		strcpy(ifname, LANIF);
			
		//printf("idx=%d\n", idx);
		if (Entry.protoType == PROTO_TCP || Entry.protoType == PROTO_UDPTCP)
		{
			// iptables -t nat -A PREROUTING -i ! $LAN_IF -p TCP --dport dstPortRange -j DNAT --to-destination ipaddr
			va_cmd(IPTABLES, 15, 1, "-t", "nat", "-I", (char *)FW_PREROUTING, (char *)ARG_I, "!", 
				(char *)ifname,	"-p", (char *)ARG_TCP, (char *)FW_DPORT, srcPortRange, "-j", "DNAT", 
				"--to-destination", dstip);
			
			// iptables -I ipfilter 3 -i ! $LAN_IF -o $LAN_IF -p TCP --dport dstPortRange -j RETURN
			va_cmd(IPTABLES, 16, 1, "-I", (char *)FW_IPFILTER, "3", (char *)ARG_I, "!", (char *)ifname, 
				(char *)ARG_O, (char *)ifname, "-p", (char *)ARG_TCP, (char *)FW_DPORT, dstPort, "-d", 
				(char *)ipAddr, "-j",(char *)FW_RETURN);
		}
		if (Entry.protoType == PROTO_UDP || Entry.protoType == PROTO_UDPTCP)
		{
			va_cmd(IPTABLES, 15, 1, "-t", "nat", "-I", (char *)FW_PREROUTING, (char *)ARG_I, "!", 
				(char *)ifname,	"-p", (char *)ARG_UDP, (char *)FW_DPORT, srcPortRange, "-j", "DNAT", 
				"--to-destination", dstip);
			// iptables -I ipfilter 3 -i ! $LAN_IF -o $LAN_IF -p UDP --dport dstPortRange -j RETURN
			va_cmd(IPTABLES, 16, 1, "-I", (char *)FW_IPFILTER, "3", (char *)ARG_I, "!", (char *)ifname, 
				(char *)ARG_O, (char *)LANIF, "-p", (char *)ARG_UDP, (char *)FW_DPORT, dstPort, "-d", 
				(char *)ipAddr, "-j",(char *)FW_RETURN);
		}
	}
		
	return 1;
}
#endif

// Mason Yu
int setupPortFW()
{
	int vInt, i, total;
	unsigned char value[32];
	MIB_CE_PORT_FW_T PfEntry;	
	
	// Clean all rules
	// iptables -F portfw			
	va_cmd(IPTABLES, 2, 1, "-F", (char *)PORT_FW);		
	// iptables -t nat -F portfw
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-F", (char *)PORT_FW);	
	
	vInt = 0;
	if (mib_get(MIB_PORT_FW_ENABLE, (void *)value) != 0)
		vInt = (int)(*(unsigned char *)value);
	
	if (vInt == 1)
	{

		int negate=0, hasRemote=0;
		char * proto = 0;
		char intPort[32], extPort[32];
		
		total = mib_chain_total(MIB_PORT_FW_TBL);

		for (i = 0; i < total; i++)
		{		
			if (!mib_chain_get(MIB_PORT_FW_TBL, i, (void *)&PfEntry))
				return -1;
			
			portfw_modify( &PfEntry, 0 );
		}
	}//if (vInt == 1)
	
	return 1;
}

#ifdef PORT_FORWARD_ADVANCE
int setupPFWAdvance()
{
	// iptables -N pptp			
	va_cmd(IPTABLES, 2, 1, "-N", (char *)FW_PPTP);	
	// iptables -A FORWARD -j pptp
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", (char *)FW_PPTP);	
	// iptables -t nat -N pptp
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-N", (char *)FW_PPTP);	
	// iptables -t nat -A PREROUTING -j pptp			
	va_cmd(IPTABLES, 6, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_PREROUTING, "-j", (char *)FW_PPTP);
	
	
	// iptables -N l2tp			
	va_cmd(IPTABLES, 2, 1, "-N", (char *)FW_L2TP);	
	// iptables -A FORWARD -j l2tp
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", (char *)FW_L2TP);	
	// iptables -t nat -N l2tp
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-N", (char *)FW_L2TP);	
	// iptables -t nat -A PREROUTING -j l2tp			
	va_cmd(IPTABLES, 6, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_PREROUTING, "-j", (char *)FW_L2TP);	
	
	config_PFWAdvance(ACT_START);
	
	return 0;
}

int stopPFWAdvance()
{
	unsigned int entryNum, i;
	MIB_CE_PORT_FW_ADVANCE_T Entry;	
	int pptp_enable=1;
	int l2tp_enable=1;
	
	entryNum = mib_chain_total(MIB_PFW_ADVANCE_TBL);
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_PFW_ADVANCE_TBL, i, (void *)&Entry))
		{  			
  			printf("stopPFWAdvance: Get chain record error!\n");
			return 1;
		}		
		
		if ( strcmp("PPTP", PFW_Rule[(PFW_RULE_T)Entry.rule]) == 0 && pptp_enable == 1) {			
			// iptables -F pptp			
			va_cmd(IPTABLES, 2, 1, "-F", (char *)FW_PPTP);			
			// iptables -t nat -F pptp
			va_cmd(IPTABLES, 4, 1, "-t", "nat", "-F", (char *)FW_PPTP);			
			pptp_enable = 0;
		}
		
		if ( strcmp("L2TP", PFW_Rule[(PFW_RULE_T)Entry.rule]) == 0 && l2tp_enable == 1) {			
			// iptables -F l2tp			
			va_cmd(IPTABLES, 2, 1, "-F", (char *)FW_L2TP);			
			// iptables -t nat -F l2tp
			va_cmd(IPTABLES, 4, 1, "-t", "nat", "-F", (char *)FW_L2TP);			
			l2tp_enable = 0;
		}
	}
	return 0;
}

int startPFWAdvance()
{
	unsigned int entryNum, i;
	MIB_CE_PORT_FW_ADVANCE_T Entry;
	char interface_name[8], lanIP[16], ip_port[32];
	struct in_addr dest;
	int pf_enable;
	unsigned char value[32];
	int pptp_enable=0;
	int l2tp_enable=0;
	
	entryNum = mib_chain_total(MIB_PFW_ADVANCE_TBL);
	
	
	pf_enable = 0;
	if (mib_get(MIB_PORT_FW_ENABLE, (void *)value) != 0)
		pf_enable = (int)(*(unsigned char *)value);
		
	if ( pf_enable != 1 ) {
		printf("Port Forwarding is disable and stop to setup Port Forwarding Advance!\n");
		return 1;
	}
		
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_PFW_ADVANCE_TBL, i, (void *)&Entry))
		{  			
  			printf("startPFWAdvance: Get chain record error!\n");
			return 1;
		}
		
		if ( strcmp("PPTP", PFW_Rule[(PFW_RULE_T)Entry.rule]) == 0 && pptp_enable == 0) {			
			// LAN IP Address
			dest.s_addr = *(unsigned long *)Entry.ipAddr;		
			// inet_ntoa is not reentrant, we have to
			// copy the static memory before reuse it
			strcpy(lanIP, inet_ntoa(dest));
			lanIP[15] = '\0';
			sprintf(ip_port,"%s:%d",lanIP, 1723);
		
			// interface
			if ( Entry.ifIndex == 0xff )
			{
				strcpy( interface_name, "---" );
			}else if(  (Entry.ifIndex & 0xf0)==0xf0  )
			{
				sprintf( interface_name, "vc%u", VC_INDEX(Entry.ifIndex) );
			}else{
				sprintf( interface_name, "ppp%u", PPP_INDEX(Entry.ifIndex) );
			}
			
			// iptables -A pptp -p tcp --destination-port 1723 --dst $LANIP -j ACCEPT
			va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_PPTP, "-p", (char *)ARG_TCP, "--destination-port", "1723", "--dst", lanIP, "-j", (char *)FW_ACCEPT);
			
			// iptables -A pptp -p 47 --dst $LANIP -j ACCEPT
			va_cmd(IPTABLES, 8, 1, (char *)FW_ADD, (char *)FW_PPTP, "-p", "47", "--dst", lanIP, "-j", (char *)FW_ACCEPT);		
			
			
			if ( Entry.ifIndex != 0xff ) {
				// iptables -t nat -A pptp -i ppp0 -p tcp --dport 1723 -j DNAT --to-destination $LANIP:1723
				va_cmd(IPTABLES, 14, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_PPTP, "-i", interface_name, "-p", (char *)ARG_TCP, "--dport", "1723", "-j", "DNAT", "--to-destination", ip_port);
			
				// iptables -t nat -A pptp -i ppp0 -p 47 -j DNAT --to-destination $LANIP
				va_cmd(IPTABLES, 12, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_PPTP, "-i", interface_name, "-p", "47", "-j", "DNAT", "--to-destination", lanIP);
			} else {
				// iptables -t nat -A pptp -p tcp --dport 1723 -j DNAT --to-destination $LANIP:1723
				va_cmd(IPTABLES, 12, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_PPTP, "-p", (char *)ARG_TCP, "--dport", "1723", "-j", "DNAT", "--to-destination", ip_port);
			
				// iptables -t nat -A pptp -p 47 -j DNAT --to-destination $LANIP
				va_cmd(IPTABLES, 10, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_PPTP, "-p", "47", "-j", "DNAT", "--to-destination", lanIP);
			}			
			pptp_enable = 1;
		}
		
		if ( strcmp("L2TP", PFW_Rule[(PFW_RULE_T)Entry.rule]) == 0 && l2tp_enable == 0) {			
			// LAN IP Address
			dest.s_addr = *(unsigned long *)Entry.ipAddr;		
			// inet_ntoa is not reentrant, we have to
			// copy the static memory before reuse it
			strcpy(lanIP, inet_ntoa(dest));
			lanIP[15] = '\0';
			sprintf(ip_port,"%s:%d",lanIP, 1701);
		
			// interface
			if ( Entry.ifIndex == 0xff )
			{
				strcpy( interface_name, "---" );
			}else if(  (Entry.ifIndex & 0xf0)==0xf0  )
			{
				sprintf( interface_name, "vc%u", VC_INDEX(Entry.ifIndex) );
			}else{
				sprintf( interface_name, "ppp%u", PPP_INDEX(Entry.ifIndex) );
			}
			
			// iptables -A l2tp -p udp --destination-port 1701 --dst $LANIP -j ACCEPT
			va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_L2TP, "-p", (char *)ARG_UDP, "--destination-port", "1701", "--dst", lanIP, "-j", (char *)FW_ACCEPT);			
			
			// iptables -t nat -A l2tp -i ppp0 -p udp --dport 1701 -j DNAT --to-destination $LANIP:1701
			if ( Entry.ifIndex == 0xff ) {
				va_cmd(IPTABLES, 12, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_L2TP, "-p", (char *)ARG_UDP, "--dport", "1701", "-j", "DNAT", "--to-destination", ip_port);
			} else {
				va_cmd(IPTABLES, 14, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_L2TP, "-i", interface_name, "-p", (char *)ARG_UDP, "--dport", "1701", "-j", "DNAT", "--to-destination", ip_port);			
			}			
			l2tp_enable = 1;
		}
	}
	return 0;
}
#endif

// Mason Yu
int setupDMZ()
{
	int vInt;
	unsigned char value[32];
	char ipaddr[32];
	char lanip[16];	
	
	// iptables -F dmz			
	va_cmd(IPTABLES, 2, 1, "-F", (char *)IPTABLE_DMZ);		
	// iptables -t nat -F dmz
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-F", (char *)IPTABLE_DMZ);
	
	vInt = 0;
	if (mib_get(MIB_DMZ_ENABLE, (void *)value) != 0)
		vInt = (int)(*(unsigned char *)value);
	
	if (vInt == 1)
	{
		struct ifreq ifr;
		int skfd;
		struct sockaddr_in *addr;		
		int entryNum;
		char s_entryNum[8];
	
		// Don't forward SNMP to DMZ
//		if (wan_snmp_enable)
		{
			// iptables -t nat -A dmz -i ! $LAN_IF -p UDP --dport 161:162 -j ACCEPT
			va_cmd(IPTABLES, 13, 1, "-t", "nat", (char *)FW_ADD,
				(char *)IPTABLE_DMZ, (char *)ARG_I, "!",
				(char *)LANIF, "-p", (char *)ARG_UDP,
				(char *)FW_DPORT, "161:162", "-j",
				(char *)FW_ACCEPT);
		}
		//add by ramen to let the packet accessing tr069 can go through.
		#ifdef _CWMP_MIB_
		va_cmd(IPTABLES, 13, 1, "-t", "nat", (char *)FW_ADD,
				(char *)IPTABLE_DMZ, (char *)ARG_I, "!",
				(char *)LANIF, "-p", (char *)ARG_TCP,
				(char *)FW_DPORT, "9999", "-j",
				(char *)FW_ACCEPT);		
		#endif
		if (mib_get(MIB_DMZ_IP, (void *)value) != 0)
		{
			strncpy(ipaddr, inet_ntoa(*((struct in_addr *)value)), 16);
			ipaddr[15] = '\0';
		}
		
		skfd = socket(AF_INET, SOCK_DGRAM, 0);
		strcpy(ifr.ifr_name, LANIF);
		
		if (ioctl(skfd, SIOCGIFADDR, &ifr) == 0) {
			addr = ((struct sockaddr_in *)&ifr.ifr_addr);
			strncpy(lanip, inet_ntoa(addr->sin_addr), 16);
			lanip[15] = '\0';;
		}
		else
		{
			printf("LAN ip address not found !\n");
			strcpy(lanip, ARG_0x4);
		}
		
		close( skfd );
		
		// iptables -t nat -A dmz -i ! $LAN_IF -j DNAT --to-destination $DMZ_IP
		va_cmd(IPTABLES, 11, 1, "-t", "nat", (char *)FW_ADD,
			(char *)IPTABLE_DMZ, (char *)ARG_I, "!",
			(char *)LANIF, "-j", "DNAT", "--to-destination", ipaddr);
		
		/*
		// iptables -t nat -A POSTROUTING -d $DMZ_IP -j SNAT --to-source $LAN_IP
		va_cmd(IPTABLES, 10, 1, "-t", "nat", (char *)FW_ADD,
			"POSTROUTING", "-d", ipaddr, "-j", "SNAT",
			"--to-source", lanip);
		*/
		
		/*
		// iptables -A FORWARD -i ! $LAN_IF -o $LAN_IF -d $DMZ_IP -j ACCEPT
		va_cmd(IPTABLES, 11, 1, (char *)FW_ADD, (char *)FW_FORWARD,
			(char *)ARG_I, "!", (char *)LANIF, "-o", (char *)LANIF,
			"-d", ipaddr, "-j", (char *)FW_ACCEPT);
		*/
		
		// Let's do IP Filtering before doing DMZ
		entryNum = mib_chain_total (MIB_IP_PORT_FILTER_TBL) + 3;
		snprintf(s_entryNum, 8, "%d", entryNum);
		// iptables -A dmz -i ! $LAN_IF -o $LAN_IF -d $DMZ_IP -j RETURN
		va_cmd(IPTABLES, 11, 1, (char *)FW_ADD,
			(char *)IPTABLE_DMZ, (char *)ARG_I, "!",
			(char *)LANIF, (char *)ARG_O,
			(char *)LANIF, "-d", ipaddr, "-j", (char *)FW_ACCEPT);		
		printf("iptables firewall enabled !\n");
	}	
}	

#ifdef NATIP_FORWARDING
static void fw_setupIPForwarding()
{
	int i, total;
	char local[16], remote[16], enable;
	MIB_CE_IP_FW_T Entry;
	
	// Add New chain on filter and nat
	// iptables -N ipfw			
	va_cmd(IPTABLES, 2, 1, "-N", (char *)IPTABLE_IPFW);	
	// iptables -A FORWARD -j ipfw
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", (char *)IPTABLE_IPFW);	
	// iptables -t nat -N ipfw
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-N", (char *)IPTABLE_IPFW);	
	// iptables -t nat -A PREROUTING -j ipfw			
	va_cmd(IPTABLES, 6, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_PREROUTING, "-j", (char *)IPTABLE_IPFW);
	// iptables -t nat -N ipfw2
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-N", (char *)IPTABLE_IPFW2);	
	// iptables -t nat -A POSTROUTING -j ipfw2		
	va_cmd(IPTABLES, 6, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_POSTROUTING, "-j", (char *)IPTABLE_IPFW2);
	
	
	mib_get(MIB_IP_FW_ENABLE, (void *)&enable);
	if (!enable)
		return;
	
	total = mib_chain_total(MIB_IP_FW_TBL);
	for (i = 0; i < total; i++)
	{
		if (!mib_chain_get(MIB_IP_FW_TBL, i, (void *)&Entry))
			continue;
		strncpy(local, inet_ntoa(*((struct in_addr *)Entry.local_ip)), 16);
		strncpy(remote, inet_ntoa(*((struct in_addr *)Entry.remote_ip)), 16);
		// iptables -t nat -A ipfw -d remoteip -i ! $LAN_IF -j DNAT --to-destination localip
		va_cmd(IPTABLES, 13, 1, "-t", "nat",
			(char *)FW_ADD,	(char *)IPTABLE_IPFW, "-d", remote,
			(char *)ARG_I, "!", (char *)LANIF, "-j",
			"DNAT", "--to-destination", local);
		// iptables -t nat -A ipfw2 -s localip -o ! br0 -j SNAT --to-source remoteip
		va_cmd(IPTABLES, 13, 1, "-t", "nat", FW_ADD, (char *)IPTABLE_IPFW2,

			(char *)ARG_O, "!", (char *)LANIF, "-s", local, "-j", "SNAT", "--to-source", remote);

		// iptables -A ipfw2 -d localip -i ! $LAN_IF -o $LAN_IF -j RETURN
		va_cmd(IPTABLES, 11, 1, (char *)FW_ADD,
			(char *)IPTABLE_IPFW, "-d", local, (char *)ARG_I,
			"!", (char *)LANIF, (char *)ARG_O,
			(char *)LANIF, "-j", (char *)FW_ACCEPT);
	}
}
#endif

int setup_default_IPFilter()
{
	// Set default action for ipfilter		
	unsigned char value[32];
	int vInt;
		
	if (mib_get(MIB_IPF_OUT_ACTION, (void *)value) != 0)
	{
		vInt = (int)(*(unsigned char *)value);
		if (vInt == 0)	// DROP
		{
			// iptables -A ipfilter -i $LAN_IF -j DROP
			
			va_cmd(IPTABLES, 6, 1, (char *)FW_ADD,
				(char *)FW_IPFILTER, (char *)ARG_I,
				(char *)LANIF, "-j", (char *)FW_DROP);			
		}
	}
	
	if (mib_get(MIB_IPF_IN_ACTION, (void *)value) != 0)
	{
		vInt = (int)(*(unsigned char *)value);
		if (vInt == 0)	// DROP
		{
			// iptables -A ipfilter -i ! $LAN_IF -j DROP
			
				va_cmd(IPTABLES, 7, 1, (char *)FW_ADD,
				(char *)FW_IPFILTER, (char *)ARG_I, "!",
				(char *)LANIF, "-j", (char *)FW_DROP);
			
		}
	}
	
	return 1;	
}

int restart_IPFilter_DMZ()
{
	setupIPFilter();	
	// iptables -A filter -j dmz
	va_cmd(IPTABLES, 4, 1, "-A", (char *)FW_IPFILTER, "-j", (char *)IPTABLE_DMZ);
	setup_default_IPFilter();
	return 1;	
}

#ifdef PORT_TRIGGERING
void parse_and_add_triggerPort(char *inRange, PROTO_TYPE_T prot, char *ip)
{
	int parseLen, j;
	char tempStr1[10]={0},tempStr2[10]={0};
	char *p, dstPortRange[32];
	int idx=0,shift=0;
	
	parseLen = strlen(inRange);
	if (prot == PROTO_TCP)
		p = (char *)ARG_TCP;
	else
		p = (char *)ARG_UDP;
	
	for(j=0;j<GAMING_MAX_RANGE;j++)
	{
		if(((inRange[j]>='0')&&(inRange[j]<='9')))
		{
			if(shift>=9) continue;
			if(idx==0)
				tempStr1[shift++]=inRange[j];
			else
				tempStr2[shift++]=inRange[j];
			
		}
		else if((inRange[j]==',')||
				(inRange[j]=='-')||
				(inRange[j]==0))
		{
			if(idx==0)
				tempStr1[shift]=0;
			else
				tempStr2[shift]=0;
		
			shift=0;
			if((inRange[j]==',')||
				(inRange[j]==0))
			{
				/*
				uint16 inStart,inFinish;
				inStart=atoi(tempStr1);
				if(idx==0)
					inFinish=inStart;
				else
					inFinish=atoi(tempStr2);
				*/
				dstPortRange[0] = '\0';
				if (idx==0) // single port number
					strncpy(dstPortRange, tempStr1, 32);
				else {
					if (strlen(tempStr1)!=0 && strlen(tempStr2)!=0)
						snprintf(dstPortRange, 32, "%s:%s", tempStr1, tempStr2);
				}

				idx=0;
				
				// iptables -t nat -A PREROUTING -i ! $LAN_IF -p TCP --dport dstPortRange -j DNAT --to-destination ipaddr
				va_cmd(IPTABLES, 15, 1, "-t", "nat",
					(char *)FW_ADD,	(char *)IPTABLES_PORTTRIGGER,
					(char *)ARG_I, "!", (char *)LANIF,
					"-p", p,
					(char *)FW_DPORT, dstPortRange, "-j",
					"DNAT", "--to-destination", ip);
				
				// iptables -I ipfilter 3 -i ! $LAN_IF -o $LAN_IF -p TCP --dport dstPortRange -j RETURN
				va_cmd(IPTABLES, 13, 1, (char *)FW_ADD,
					(char *)IPTABLES_PORTTRIGGER, (char *)ARG_I, "!",
					(char *)LANIF, (char *)ARG_O,
					(char *)LANIF, "-p", p,
					(char *)FW_DPORT, dstPortRange, "-j",
					(char *)FW_ACCEPT);
				/*
				//make inFinish always bigger than inStart
				if(inStart>inFinish)
				{
					uint16 temp;
					temp=inFinish;
					inFinish=inStart;
					inStart=temp;
				}
				
				if(!((inStart==0)||(inFinish==0)))
				{
					rtl8651a_addTriggerPortRule(
					dsid, //dsid
					inType,inStart,inFinish,
					outType,
					outStart,
					outFinish,localIp);
//					printf("inRange=%d-%d\n inType=%d\n",inStart,inFinish,inType);
				}
				*/
			}
			else
			{
				idx++;	
			}
			if(inRange[j]==0)
				break;
		}				
	}
}

void setupPortTriggering()
{
	int i, total;
	char ipaddr[16];
	MIB_CE_PORT_TRG_T Entry;
	
	total = mib_chain_total(MIB_PORT_TRG_TBL);
	for (i = 0; i < total; i++)
	{
		if (!mib_chain_get(MIB_PORT_TRG_TBL, i, (void *)&Entry))
			continue;
		if(!Entry.enable)
			continue;
		
		strncpy(ipaddr, inet_ntoa(*((struct in_addr *)Entry.ip)), 16);
		
		parse_and_add_triggerPort(Entry.tcpRange, PROTO_TCP, ipaddr);
		parse_and_add_triggerPort(Entry.udpRange, PROTO_UDP, ipaddr);
	}
}
#endif // of PORT_TRIGGERING

// Execute firewall rules
// return value:
// 1  : successful
int setupFirewall()
{
	char *argv[20];
	unsigned char value[32];
	int vInt, i, total, vcNum, min_mtu=1452;
	MIB_CE_IP_PORT_FILTER_T IpEntry;
#ifdef MAC_FILTER
	MIB_CE_MAC_FILTER_T MacEntry;
	char mac_in_dft, mac_out_dft;
	char eth_mac_ctrl=0, wlan_mac_ctrl=0;
#endif	
	MIB_CE_ATM_VC_T Entry;
	char *policy, *filterSIP, *filterDIP, srcPortRange[12], dstPortRange[12];
	char ipaddr[32], srcip[20], dstip[20], mmtu[5]="1452", mmtu_s[11];
	char ifname[16], extra[32];
	char srcmacaddr[18], dstmacaddr[18];
#ifdef IP_PASSTHROUGH
	unsigned char ippt_itf;
#endif
	int spc_enable, spc_ip;
	// Added by Mason Yu for ACL
	unsigned char aclEnable, domainEnable;	
	unsigned char dhcpvalue[32];
	unsigned char vChar;
	int dhcpmode;
	char str_vc[] = "vc0";
	// Added by Mason Yu for URL Blocking
#if defined( URL_BLOCKING_SUPPORT)||defined( URL_ALLOWING_SUPPORT)	
	unsigned char urlEnable;  
#endif		
#ifdef NAT_CONN_LIMIT
	unsigned char connlimitEn;
#endif

	
	
	//-------------------------------------------------
	// Filter table
	//-------------------------------------------------
	//--------------- set policies --------------
	// iptables -P INPUT DROP
	va_cmd(IPTABLES, 3, 1, "-P", (char *)FW_INPUT, (char *)FW_DROP);
	
	// iptables -P FORWARD ACCEPT
	va_cmd(IPTABLES, 3, 1, "-P", (char *)FW_FORWARD, (char *)FW_ACCEPT);
	
	// accept related
	// iptables -A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT
	va_cmd(IPTABLES, 8, 1, (char *)FW_ADD, (char *)FW_INPUT, "-m", "state",
		"--state", "ESTABLISHED,RELATED", "-j", (char *)FW_ACCEPT);
	
	// Allowing multicast access, ie. IGMP, RIPv2
	// iptables -A INPUT -d 224.0.0.0/4 -j ACCEPT
	va_cmd(IPTABLES, 6, 1, (char *)FW_ADD, (char *)FW_INPUT, "-d",
		"224.0.0.0/4", "-j", (char *)FW_ACCEPT);
#ifdef IP_ACL
	//  Add Chain(aclblock) for ACL
	va_cmd(IPTABLES, 2, 1, "-N", "aclblock");	
	mib_get(MIB_ACL_CAPABILITY, (void *)&aclEnable);
	if (aclEnable == 1)  // ACL Capability is enabled
		filter_set_acl(1);
	//iptables -A INPUT -j aclblock
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_INPUT, "-j", "aclblock");
#endif

#ifdef NAT_CONN_LIMIT
	//Add Chain(connlimit) for NAT conn limit
	va_cmd(IPTABLES, 2, 1, "-N", "connlimit");

	mib_get(MIB_NAT_CONN_LIMIT, (void *)&connlimitEn);
	if (connlimitEn == 1)
		set_conn_limit();

	//iptables -A FORWARD -j connlimit
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", "connlimit");
#endif
#ifdef TCP_UDP_CONN_LIMIT
	//Add Chain(connlimit) for NAT conn limit
	va_cmd(IPTABLES, 2, 1, "-N", "connlimit");

	mib_get(MIB_CONNLIMIT_ENABLE, (void *)&vChar);
	if (vChar == 1)
		set_conn_limit();
	//iptables -A FORWARD -j connlimit
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", "connlimit");
#endif

#ifdef CONFIG_USER_RTK_VOIP
	voip_setup_iptable();
#endif
#ifdef CONFIG_EXT_SWITCH
#ifdef VLAN_GROUP
	//  Add Chain(itfgroup)
	va_cmd(IPTABLES, 2, 1, "-N", "itfgroup");	
	vg_setup_iptables();
	//iptables -A INPUT -j itfgroup
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_INPUT, "-j", "itfgroup");
#endif
#endif

#ifdef DOMAIN_BLOCKING_SUPPORT
	//  Add Chain(domainblk) for ACL
	va_cmd(IPTABLES, 2, 1, "-N", "domainblk");	
	mib_get(MIB_DOMAINBLK_CAPABILITY, (void *)&domainEnable);
	if (domainEnable == 1)  // Domain blocking Capability is enabled
		filter_set_domain(1);
	//iptables -A INPUT -j domainblk
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_INPUT, "-j", "domainblk");
	
	//iptables -A FORWARD -j domainblk
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", "domainblk");
	
	//iptables -A OUTPUT -j domainblk
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, "OUTPUT", "-j", "domainblk");
#endif
	
	//paccEntry = (MIB_CE_ACC_Tp) mib_chain_get(MIB_ACC_TBL, 0);
	//if (!paccEntry)
	//	return;
	
	// Add chain for remote access
	// iptables -N inacc
	va_cmd(IPTABLES, 2, 1, "-N", (char *)FW_INACC);
	va_cmd(IPTABLES, 11, 1, "-A", (char *)FW_INACC, 
		(char *)ARG_I, "!", LANIF, "-m", "mark", "--mark", RMACC_MARK, "-j", FW_ACCEPT);
 
	filter_set_remote_access(1);
	
#ifdef IP_PASSTHROUGH
	// IP Passthrough, LAN access
	set_IPPT_LAN_access();
#endif		
	
	// Added by Mason Yu for dhcp Relay. Open DHCP Relay Port for Incoming Packets.
	if (mib_get(MIB_DHCP_MODE, (void *)dhcpvalue) != 0)
	{
		dhcpmode = (unsigned int)(*(unsigned char *)dhcpvalue);
		if (dhcpmode == 1 ){
			// iptables -A inacc -i ! br0 -p udp --dport 67 -j ACCEPT		
			va_cmd(IPTABLES, 11, 1, (char *)FW_ADD, (char *)FW_INACC, (char *)ARG_I, "!", "br0", "-p", "udp", (char *)FW_DPORT, (char *)PORT_DHCP, "-j", (char *)FW_ACCEPT);				
		}	
	}	
	
#ifdef CONFIG_USER_ROUTED_ROUTED
	// Added by Mason Yu. Open RIP Port for Incoming Packets.
	if (mib_get( MIB_RIP_ENABLE, (void *)&vChar) != 0)
	{	
		if (1 == vChar)	
		{	
			// iptables -A inacc -i ! br0 -p udp --dport 520 -j ACCEPT		
			va_cmd(IPTABLES, 11, 1, (char *)FW_ADD, (char *)FW_INACC, "-i", "!", "br0", "-p", "udp", (char *)FW_DPORT, "520", "-j", (char *)FW_ACCEPT);	
			// iptables -A INPUT -p udp --dport 520 -j ACCEPT
			va_cmd(IPTABLES, 8, 1, (char *)FW_ADD, (char *)FW_INPUT, "-p", "udp", (char *)FW_DPORT, "520", "-j", (char *)FW_ACCEPT);
		}			
	}
#endif
							
	// iptables -A INPUT -j inacc
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_INPUT, "-j", (char *)FW_INACC);
	
	// Added by Mason Yu for accept packet with ip(127.0.0.1)
	va_cmd(IPTABLES, 6, 1, (char *)FW_ADD, (char *)FW_INPUT, "-d", "127.0.0.1", "-j", (char *)FW_ACCEPT);
	
	// Single PC mode
	if (mib_get(MIB_SPC_ENABLE, (void *)value) != 0)
	{
		if (value[0]) // Single PC mode enabled
		{
			spc_enable = 1;
			mib_get(MIB_SPC_IPTYPE, (void *)value);
			if (value[0] == 0) // single private IP
			{
				spc_ip = 0;
				mib_get(MIB_ADSL_LAN_IP, (void *)value);
				
				// IP pool start address
				// Kaohj
				#ifndef DHCPS_POOL_COMPLETE_IP
				mib_get(MIB_ADSL_LAN_CLIENT_START, (void *)&value[3]);
				strncpy(ipaddr, inet_ntoa(*((struct in_addr *)value)), 16);
				#else
				mib_get(MIB_DHCP_POOL_START, (void *)value);
				strncpy(ipaddr, inet_ntoa(*((struct in_addr *)value)), 16);
				#endif
				ipaddr[15] = '\0';
				// iptables -A FORWARD -i $LANIF -s ! ipaddr -j DROP
				va_cmd(IPTABLES, 9, 1, (char *)FW_ADD,
					(char *)FW_FORWARD, (char *)ARG_I,
					(char *)LANIF, "-s", "!", ipaddr,
					"-j", (char *)FW_DROP);
			}
			else // single IP passthrough
			{
				spc_ip = 1;
			}
		}
		else
			spc_enable = 0;
	}
				
#ifdef IP_PASSTHROUGH
	// check vc
	total = mib_chain_total(MIB_ATM_VC_TBL);
	for (i = 0; i < total; i++)
	{
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			return -1;
		
		if (spc_enable && (spc_ip == 1)) // single IP passthrough (public IP)
		{
			if ((REMOTE_ACCESS_T)Entry.cmode == ACC_ROUTED &&
				ippt_itf == Entry.ifIndex)
			{	// ippt WAN interface (1483-r)
				if ((DHCP_T)Entry.ipDhcp == DHCP_DISABLED)
				{
					strncpy(ipaddr, inet_ntoa(*((struct in_addr *)Entry.ipAddr)), 16);
					ipaddr[15] = '\0';
					// iptables -A FORWARD -i $LANIF -s ! ipaddr -j DROP
					va_cmd(IPTABLES, 9, 1, (char *)FW_ADD,
						(char *)FW_FORWARD, (char *)ARG_I,
						(char *)LANIF, "-s", "!", ipaddr,
						"-j", (char *)FW_DROP);
				}
			}
		}
	}
#endif

#if defined(URL_BLOCKING_SUPPORT)||defined(URL_ALLOWING_SUPPORT)
 mib_get(MIB_URL_CAPABILITY, (void *)&urlEnable);
#endif

#ifdef URL_BLOCKING_SUPPORT		
// Added by Mason Yu for URL Blocking
//  Add Chain(urlblock) for URL	
 va_cmd(IPTABLES, 2, 1, "-N", "urlblock");	
 if (urlEnable == 1)  // URL Capability enabled
   filter_set_url(1);	
  //iptables -A FORWARD -j urlblock	
   va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", "urlblock");
 #endif
 
 #ifdef URL_ALLOWING_SUPPORT       
   va_cmd(IPTABLES,2,1 ,"-N","urlallow");
   if(urlEnable==2)
 	set_url(2); //alex	 
 	
 	//iptables -A FORWARD -j urlallow	
   va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", "urlallow");
#endif

#ifdef LAYER7_FILTER_SUPPORT
	//App Filter
	va_cmd(IPTABLES, 2, 1, "-N", (char *)FW_APPFILTER);
	va_cmd(IPTABLES, 2, 1, "-N", (char *)FW_APP_P2PFILTER);
	va_cmd(IPTABLES, 4, 1, ARG_T, "mangle", "-N", (char *)FW_APPFILTER);
	setupAppFilter();
	// iptables -A FORWARD -j appfilter
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", (char *)FW_APPFILTER);	
	// iptables -t mangle -A POSTROUTING -j appfilter
	va_cmd(IPTABLES, 6, 1, ARG_T, "mangle", (char *)FW_ADD, (char *)FW_POSTROUTING, "-j", (char *)FW_APPFILTER);	
#endif

	// port forwarding
	// Add New chain on filter and nat
	// iptables -N portfw			
	va_cmd(IPTABLES, 2, 1, "-N", (char *)PORT_FW);	
	// iptables -A FORWARD -j portfw
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", (char *)PORT_FW);	
	// iptables -t nat -N portfw
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-N", (char *)PORT_FW);	
	// iptables -t nat -A PREROUTING -j portfw			
	va_cmd(IPTABLES, 6, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_PREROUTING, "-j", (char *)PORT_FW);
	setupPortFW();

#ifdef PORT_FORWARD_ADVANCE
	setupPFWAdvance();	
#endif		

#ifdef NATIP_FORWARDING
	fw_setupIPForwarding();
#endif

#ifdef PORT_TRIGGERING
	// Add New chain on filter and nat
	// iptables -N portTrigger			
	va_cmd(IPTABLES, 2, 1, "-N", (char *)IPTABLES_PORTTRIGGER);	
	// iptables -A FORWARD -j portTrigger
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", (char *)IPTABLES_PORTTRIGGER);	
	// iptables -t nat -N portTrigger
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-N", (char *)IPTABLES_PORTTRIGGER);	
	// iptables -t nat -A PREROUTING -j portTrigger			
	va_cmd(IPTABLES, 6, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_PREROUTING, "-j", (char *)IPTABLES_PORTTRIGGER);
	setupPortTriggering();
#endif	
	// IP Filter
	va_cmd(IPTABLES, 2, 1, "-N", (char *)FW_IPFILTER);	
	setupIPFilter();
	// iptables -A FORWARD -j ipfilter
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", (char *)FW_IPFILTER);	
	
	// DMZ
	// Add New chain on filter and nat
	// iptables -N dmz			
	va_cmd(IPTABLES, 2, 1, "-N", (char *)IPTABLE_DMZ);		
	// iptables -t nat -N dmz
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-N", (char *)IPTABLE_DMZ);		
	setupDMZ();
	// iptables -A filter -j dmz
	va_cmd(IPTABLES, 4, 1, "-A", (char *)FW_IPFILTER, "-j", (char *)IPTABLE_DMZ);
	// iptables -t nat -A PREROUTING -j dmz			
	va_cmd(IPTABLES, 6, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_PREROUTING, "-j", (char *)IPTABLE_DMZ);
	
	// Set IP filter default action
	setup_default_IPFilter();	
	
	// Mac Filter	
	va_cmd(IPTABLES, 2, 1, "-N", (char *)FW_MACFILTER);
	setupMacFilter();
	// iptables -A FORWARD -j macfilter
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", (char *)FW_MACFILTER);	


#ifdef LAYER7_FILTER_SUPPORT
	// iptables -A FORWARD -j appp2pfilter
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", (char *)FW_APP_P2PFILTER);	
#endif


#ifdef ZTE_GENERAL_ROUTER_SC
	//execute virtual server rules
	setupVtlsvr();
#endif	
	
	total = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<total; i++)
	{
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			return -1;
		if (Entry.enable == 0)
			continue;
		if(min_mtu > Entry.mtu)
			min_mtu = Entry.mtu;
	}
	sprintf(mmtu, "%d", min_mtu-40);
	// Added by Mason Yu
	sprintf(mmtu_s, "%d:1536", min_mtu-40);
	
	// for PPPoE Black Hole
	// iptables -A FORWARD -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu
	{ 
	    /* Since we should NOT modify the MSS of the inbound SYN packet, we should add this 
	    rule ONLY on PPP interface. */
	    va_cmd(IPTABLES, 14, 1, "-t", "mangle", (char *)FW_ADD, (char *)FW_FORWARD,
		"-p", "tcp", "-o", "ppp+", "--tcp-flags", "SYN,RST", "SYN", 
		"-j", "TCPMSS", "--clamp-mss-to-pmtu");

	    va_cmd(IPTABLES, 14, 1, "-t", "mangle", (char *)FW_ADD, (char *)FW_FORWARD,
		"-p", "tcp", "-i", "ppp+", "--tcp-flags", "SYN,RST", "SYN", 
		"-j", "TCPMSS", "--clamp-mss-to-pmtu");

	    // When the receive interface is ppp, if the incoming packet's mss is 
	    //greater than WAN's min mss, the incoming packet's mss must be changed to WAN's min mss,
	    //is smaller than WAN's min mss, the incoming packet's mss need not be changed.
	    va_cmd(IPTABLES, 19, 1, "-t", "mangle", (char *)FW_ADD, (char *)FW_FORWARD, 
		"-p", "tcp", "-i", "ppp+", "--tcp-flags", "SYN,RST", "SYN",
		"-m" , "tcpmss", "--mss", mmtu_s, "-j", "TCPMSS", "--set-mss", mmtu);
	}
	
 	// for multicast
#if 0 // move ahead to ipfilter chain
	// iptables -A FORWARD -d 224.0.0.0/4 -j ACCEPT
	va_cmd(IPTABLES, 6, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-d",
		"224.0.0.0/4", "-j", (char *)FW_ACCEPT);
#endif
	
	// Kaohj --- only the LAN ports in the same group with IGMP proxy upstream interface
	// be able to accept IGMP messages
#if (defined(CONFIG_EXT_SWITCH) && defined(ITF_GROUP) && defined(CONFIG_USER_IGMPPROXY))
	mib_get(MIB_MPMODE, (void *)&value[0]);
	mib_get(MIB_IGMP_PROXY, (void *)&value[1]);
	if ((value[0]&0x01) && (value[1] == 1)) { // Port-mapping && igmp proxy enabled
		mib_get(MIB_IGMP_PROXY_ITF, (void *)&value[0]);
		total = mib_chain_total(MIB_ATM_VC_TBL);
		for (i=0; i<total; i++) {
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
				continue;
			if (!Entry.enable)
				continue;
			if (Entry.ifIndex == value[0])	// igmpproxy upstream interface
				break;
		}
		if (i!=total) {	// found igmpproxy upstream interface
			MIB_CE_SW_PORT_T lanPort;
			total = mib_chain_total(MIB_SW_PORT_TBL);
			for (i=0; i<total; i++) {
				if (!mib_chain_get(MIB_SW_PORT_TBL, i, (void *)&lanPort))
					continue;
				if (lanPort.itfGroup != Entry.itfGroup) {
#if 0
					// disable LAN IGMP input for other groups
					char vp[] = "1";
					vp[0] = '1' + i;
					// iptables -A INPUT -d 224.0.0.0/4 -m mark --mark port+1 -j DROP
					// Note: the mark(nfmark) is tagged at NIC driver(re8670.c)
					va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_INPUT, "-d",
					"224.0.0.0/4", "-m", "mark", "--mark", vp, "-j", (char *)FW_DROP);
#endif
                             //disable LAN IGMP input for other groups
					char phy[]="eth0_sw0";
					phy[7] = '0'+ i;
	//				printf("inport i = %d  phy %s \n",i,phy);
					// iptables -A INPUT -d 224.0.0.0/4 -i eth0_sw0 -j DROP
					// Note: the mark(nfmark) is tagged at NIC driver(re8670.c)
					va_cmd(IPTABLES, 8, 1, (char *)FW_ADD, (char *)FW_INPUT, "-d",
					"224.0.0.0/4", (char *)ARG_I,  phy, "-j", (char *)FW_DROP);

                                  
				}
			}
		}
	}
#endif
	// allowing ping to self
	mib_get(MIB_ADSL_LAN_IP, (void *)value);
	strncpy(ipaddr, inet_ntoa(*((struct in_addr *)value)), 16);
	ipaddr[15] = '\0';
	// iptables -A INPUT -s 192.168.1.1 -d 192.168.1.1 -j ACCEPT
	va_cmd(IPTABLES, 8, 1, (char *)FW_ADD, (char *)FW_INPUT, "-s",
		ipaddr, "-d", ipaddr, "-j", (char *)FW_ACCEPT);
	// iptables -A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT
	// andrew. moved to the 1st rule, or returning PING/DNS relay will be blocked.
	//va_cmd(IPTABLES, 8, 1, (char *)FW_ADD, (char *)FW_INPUT, "-m", "state",
	//	"--state", "ESTABLISHED,RELATED", "-j", (char *)FW_ACCEPT);

	// iptables -A INPUT -m state --state NEW -i $LAN_INTERFACE -j ACCEPT
	va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_INPUT, "-m", "state",
		"--state", "NEW", (char *)ARG_I, (char *)LANIF,
		"-j", (char *)FW_ACCEPT);
	
#if 0
	/*--------------------------------------------------------------------
	 * The following are the default action and should be final rules
	 -------------------------------------------------------------------*/
	// iptables -N block
	va_cmd(IPTABLES, 2, 1, "-N", (char *)FW_BLOCK);
	
	// default action
	if (mib_get(MIB_IPF_OUT_ACTION, (void *)value) != 0)
	{
		vInt = (int)(*(unsigned char *)value);
		if (vInt == 1)	// ACCEPT
		{
			// iptables -A block -i $LAN_IF -j ACCEPT
			va_cmd(IPTABLES, 6, 1, (char *)FW_ADD,
				(char *)FW_BLOCK, (char *)ARG_I,
				(char *)LANIF, "-j", (char *)FW_ACCEPT);
		}
	}
	
	if (mib_get(MIB_IPF_IN_ACTION, (void *)value) != 0)
	{
		vInt = (int)(*(unsigned char *)value);
		if (vInt == 1)	// ACCEPT
		{
			// iptables -A block -i ! $LAN_IF -j ACCEPT
			va_cmd(IPTABLES, 7, 1, (char *)FW_ADD,
				(char *)FW_BLOCK, (char *)ARG_I, "!",
				(char *)LANIF, "-j", (char *)FW_ACCEPT);
		}
	}
	
	// iptables -A block -m state --state ESTABLISHED,RELATED -j ACCEPT
	va_cmd(IPTABLES, 8, 1, (char *)FW_ADD, (char *)FW_BLOCK, "-m", "state",
		"--state", "ESTABLISHED,RELATED", "-j", (char *)FW_ACCEPT);
	
	/*
	// iptables -A block -m state --state NEW -i $LAN_INTERFACE -j ACCEPT
	va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_BLOCK, "-m", "state",
		"--state", "NEW", (char *)ARG_I, (char *)LANIF,
		"-j", (char *)FW_ACCEPT);
	*/
	
	// iptables -A block -j DROP
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, "block", "-j", (char *)FW_DROP);
	
	/*
	// iptables -A INPUT -j block
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_INPUT, "-j", "block");
	*/
	
	// iptables -A FORWARD -j block
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", "block");
#endif
#ifdef ZTE_531B_BRIDGE_SC
unsigned char devAddr[MAC_ADDR_LEN];
if (mib_get(MIB_ELAN_MAC_ADDR, (void *)devAddr) != 0)
{
if(!memcmp(devAddr,"\x00\xE0\x4C\x86\x70\x01",MAC_ADDR_LEN))
{
va_cmd(IPTABLES, 10, 1, "-D", (char *)FW_INACC,
		(char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP,
		(char *)FW_DPORT, "23", "-j", (char *)FW_DROP);
}
}
#endif
	
	return 1;
}
#ifdef CONFIG_IP_NF_ALG_ONOFF
//add by ramen return 1--sucessful
int setupAlgOnOff()
{
		unsigned char  value=0;
#ifdef CONFIG_IP_NF_FTP
        	 if(mib_get(MIB_IP_ALG_FTP,&value)&& value==0)
                    system("/bin/echo 0 > /proc/ftp_algonoff");
	   	else
	   		system("/bin/echo 1 > /proc/ftp_algonoff");
#endif
#ifdef CONFIG_IP_NF_H323   
        	 if(mib_get(MIB_IP_ALG_H323,&value)&& value==0)
                    system("/bin/echo 0 >/proc/h323_algonoff");
	  	 else
	   		system("/bin/echo 1 >/proc/h323_algonoff");
#endif
#ifdef CONFIG_IP_NF_IRC
          	if(mib_get(MIB_IP_ALG_IRC,&value)&& value==0)
                    system("/bin/echo 0 >/proc/irc_algonoff");
		else
			 system("/bin/echo 1 >/proc/irc_algonoff");
#endif
#ifdef CONFIG_IP_NF_RTSP
		if(mib_get(MIB_IP_ALG_RTSP,&value)&& value==0)
                    system("/bin/echo 0 >/proc/rtsp_algonoff");
		else
			 system("/bin/echo 1 >/proc/rtsp_algonoff");
#endif
#ifdef CONFIG_IP_NF_QUAKE3
		if(mib_get(MIB_IP_ALG_QUAKE3,&value)&& value==0)
			system("/bin/echo 0 > /proc/quake3_algonoff");     
		else
			system("/bin/echo 1 > /proc/quake3_algonoff");  
#endif
#ifdef CONFIG_IP_NF_CUSEEME
		if(mib_get(MIB_IP_ALG_CUSEEME,&value)&& value==0)
                    system("/bin/echo 0 > /proc/cuseeme_algonoff");  
		else
			system("/bin/echo 1 > /proc/cuseeme_algonoff");  
#endif
#ifdef CONFIG_IP_NF_L2TP
		if(mib_get(MIB_IP_ALG_L2TP,&value)&& value==0)
                    system("/bin/echo 0 > /proc/l2tp_algonoff"); 
		else
			system("/bin/echo 1 > /proc/l2tp_algonoff"); 
#endif
#ifdef CONFIG_IP_NF_IPSEC
		if(mib_get(MIB_IP_ALG_IPSEC,&value)&& value==0)
                    system("/bin/echo 0 > /proc/ipsec_algonoff");    
		else
			system("/bin/echo 1 > /proc/ipsec_algonoff");    
#endif
#ifdef CONFIG_IP_NF_SIP
		if(mib_get(MIB_IP_ALG_SIP,&value)&& value==0)
                    system("/bin/echo 0 > /proc/sip_algonoff");   
		else
			system("/bin/echo 1 > /proc/sip_algonoff");   
#endif
#ifdef CONFIG_IP_NF_PPTP
		if(mib_get(MIB_IP_ALG_PPTP,&value)&& value==0)
                    system("/bin/echo 0 > /proc/pptp_algonoff");
		else
			system("/bin/echo 1 > /proc/pptp_algonoff");
#endif
         return;
}

#endif


#ifdef WEB_REDIRECT_BY_MAC
int start_web_redir_by_mac(void)
{
	int status=0;
	char tmpbuf[MAX_URL_LEN];
	char ipaddr[16], ip_port[32], redir_server[33];
	int  def_port=WEB_REDIR_BY_MAC_PORT;
	
	ipaddr[0]='\0'; ip_port[0]='\0';redir_server[0]='\0';
	if (mib_get(MIB_ADSL_LAN_IP, (void *)tmpbuf) != 0)
	{
		strncpy(ipaddr, inet_ntoa(*((struct in_addr *)tmpbuf)), 16);
		ipaddr[15] = '\0';
		sprintf(ip_port,"%s:%d",ipaddr,def_port);
	}//else ??

	// Enable Bridge Netfiltering					
	//status|=va_cmd("/bin/brctl", 2, 0, "brnf", "on");
	
	//iptables -t nat -N Web_Redirect
	status|=va_cmd(IPTABLES, 4, 1, "-t", "nat","-N","WebRedirectByMAC");
	
	//iptables -t nat -A Web_Redirect -d 192.168.1.1 -j RETURN
	status|=va_cmd(IPTABLES, 8, 1, "-t", "nat","-A","WebRedirectByMAC",
		"-d", ipaddr, "-j", (char *)FW_RETURN);

	//iptables -t nat -A Web_Redirect -p tcp --dport 80 -j DNAT --to-destination 192.168.1.1:8080 
	status|=va_cmd(IPTABLES, 12, 1, "-t", "nat","-A","WebRedirectByMAC",
		"-p", "tcp", "--dport", "80", "-j", "DNAT", 
		"--to-destination", ip_port);
	
	//iptables -t nat -A PREROUTING -p tcp --dport 80 -j Web_Redirect
	status|=va_cmd(IPTABLES, 12, 1, "-t", "nat","-A","PREROUTING",
		"-i", "eth0",
		"-p", "tcp", "--dport", "80", "-j", "WebRedirectByMAC");


{
	int num,i;
	unsigned char tmp2[18];
	MIB_WEB_REDIR_BY_MAC_T	wrm_entry;

	num = mib_chain_total( MIB_WEB_REDIR_BY_MAC_TBL );
	//printf( "\nnum=%d\n", num );
	for(i=0;i<num;i++)
	{
		if( !mib_chain_get( MIB_WEB_REDIR_BY_MAC_TBL, i, (void*)&wrm_entry ) )
			continue;

		sprintf( tmp2, "%02X:%02X:%02X:%02X:%02X:%02X", 
				wrm_entry.mac[0], wrm_entry.mac[1], wrm_entry.mac[2], wrm_entry.mac[3], wrm_entry.mac[4], wrm_entry.mac[5] );
		//printf( "add one mac: %s \n", tmp2 );			
		// iptables -A macfilter -i eth0  -m mac --mac-source $MAC -j ACCEPT/DROP
		status|=va_cmd("/bin/iptables", 10, 1, "-t", "nat", "-I", "WebRedirectByMAC", "-m", "mac", "--mac-source", tmp2, "-j", "RETURN");
	}
}

	return 0;
}
#endif

#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
int start_captiveportal()
{
	int status = 0;
	char tmpbuf[MAX_URL_LEN];
	char lan_ipaddr[16], lan_ip_port[32];
	char ip_mask[24];
	int  def_port = CAPTIVEPORTAL_PORT, i, num;
	FILE *fp;

	lan_ipaddr[0] = '\0';
	lan_ip_port[0] = '\0';

	if (mib_get(MIB_ADSL_LAN_IP, (void *)tmpbuf) != 0)
	{
		strncpy(lan_ipaddr, inet_ntoa(*((struct in_addr *)tmpbuf)), 16);
		lan_ipaddr[15] = '\0';
		sprintf(lan_ip_port, "%s:%d", lan_ipaddr, def_port);
	}
	else
		return -1;

	//iptables -t nat -A PREROUTING -p tcp -d 192.168.1.1 --dport 80 -j RETURN
	status |= va_cmd(IPTABLES, 12, 1, "-t", "nat", "-A", "PREROUTING",	"-p", "tcp", "-d", lan_ipaddr, "--dport", "80", "-j", (char *)FW_RETURN);

	//iptables -t nat -A CaptivePortal -p tcp --dport 80 -j DNAT --to-destination 192.168.1.1:18182
	status |= va_cmd(IPTABLES, 12, 1, "-t", "nat", "-A", "PREROUTING", "-p", "tcp", "--dport", "80", "-j", "DNAT", "--to-destination", lan_ip_port);

	CWMP_CAPTIVEPORTAL_ALLOWED_LIST_T ccal_entry;

	if(!(fp = fopen("/var/cp_allow_ip", "w")))
	{
		fprintf(stderr, "Open file cp_allow_ip failed!");
		return -1;
	}

	num = mib_chain_total(CWMP_CAPTIVEPORTAL_ALLOWED_LIST);

	for( i = 0; i < num; i++ )
	{
		if(!mib_chain_get(CWMP_CAPTIVEPORTAL_ALLOWED_LIST, i, (void*)&ccal_entry))
			continue;

		if( ccal_entry.mask < 32 ) // Valid subnet mask
			sprintf(ip_mask, "%s/%d", inet_ntoa(*(struct in_addr *)&ccal_entry.ip_addr), ccal_entry.mask);
		else	// Invalid subnet mask or unset subnet mask.
			sprintf(ip_mask, "%s", inet_ntoa(*(struct in_addr *)&ccal_entry.ip_addr));

		// iptables -t nat -I PREROUTING -p tcp -d 209.85.175.104/24 --dport 80 -j RETURN
		status |= va_cmd(IPTABLES, 12, 1, "-t", "nat", "-I", "PREROUTING", "-p", "tcp", "-d", ip_mask, "--dport", "80", "-j", "RETURN");
		fprintf(fp, "%s\n", ip_mask);
	}

	fclose(fp);

	return status;
}

int stop_captiveportal()
{
	int status = 0;
	char tmpbuf[MAX_URL_LEN];
	char lan_ipaddr[16], lan_ip_port[32];
	char ip_mask[24];
	int  def_port = CAPTIVEPORTAL_PORT, i;
	FILE *fp;

	if(fp = fopen("/var/cp_allow_ip", "r"))
	{
		char *tmp;

		while(fgets(ip_mask, 24, fp))
		{
			tmp = strchr(ip_mask, '\n');
			*tmp = '\0';

			// iptables -t nat -D PREROUTING -p tcp -d 209.85.175.104/24 --dport 80 -j RETURN
			va_cmd(IPTABLES, 12, 1, "-t", "nat", "-D", "PREROUTING", "-p", "tcp", "-d", ip_mask, "--dport", "80", "-j", "RETURN");
		}

		fclose(fp);
		unlink("/var/cp_allow_ip");
	}
	
	lan_ipaddr[0] = '\0';
	lan_ip_port[0] = '\0';

	if (mib_get(MIB_ADSL_LAN_IP, (void *)tmpbuf) != 0)
	{
		strncpy(lan_ipaddr, inet_ntoa(*((struct in_addr *)tmpbuf)), 16);
		lan_ipaddr[15] = '\0';
		sprintf(lan_ip_port, "%s:%d", lan_ipaddr, def_port);
	}
	else
		return -1;

	//iptables -t nat -A PREROUTING -p tcp -d 192.168.1.1 --dport 80 -j RETURN
	status |= va_cmd(IPTABLES, 12, 1, "-t", "nat", "-D", "PREROUTING",	"-p", "tcp", "-d", lan_ipaddr, "--dport", "80", "-j", (char *)FW_RETURN);

	//iptables -t nat -A CaptivePortal -p tcp --dport 80 -j DNAT --to-destination 192.168.1.1:18182
	status |= va_cmd(IPTABLES, 12, 1, "-t", "nat", "-D", "PREROUTING", "-p", "tcp", "--dport", "80", "-j", "DNAT", "--to-destination", lan_ip_port);

	return status;
}

int Apply_CaptivePortal(int action_type, int id, void *olddata)
{
	switch(action_type)
	{
		case CWMP_RESTART:
			stop_captiveportal();
			start_captiveportal();
			break;
		case CWMP_START:
			start_captiveportal();
			break;
		case CWMP_STOP:
			stop_captiveportal();
			break;
		default:
			return -1;
	}
	return 0;
}
#endif

//--------------------------------------------------------
// WAN startup
// return value:
// 1  : successful
// -1 : failed
int startWan(BOOT_TYPE_T mode)
{
	int vcTotal, i, ret;
	MIB_CE_ATM_VC_T Entry;
	int ipEnabled;
	FILE *fp;
	char vcNum[16];
	
	ret = 1;
	vcTotal = mib_chain_total(MIB_ATM_VC_TBL);
	
	ipEnabled = 0;
	
	for (i = 0; i < vcTotal; i++)
	{
		/* get the specified chain record */
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			return -1;
		
		if (Entry.enable == 0)
			continue;
		
		ret|=startConnection(&Entry);
		
#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC) 

                ipEnabled = 1;
#else
		if ((REMOTE_ACCESS_T)Entry.cmode != ACC_BRIDGED)
			ipEnabled = 1;
#endif


	}
	
#ifdef DEFAULT_GATEWAY_V2
	{
		unsigned char dgw, dgwip[16];
		if (mib_get(MIB_ADSL_WAN_DGW_ITF, (void *)&dgw) != 0) {
			if (dgw == DGW_NONE && getMIB2Str(MIB_ADSL_WAN_DGW_IP, dgwip) == 0) {
				if (ifExistedDGW() == 1)
					va_cmd(ROUTE, 2, 1, ARG_DEL, "default");
				// route add default gw remotip
				va_cmd(ROUTE, 4, 1, ARG_ADD, "default", "gw", dgwip);
			}
		}
	}
#endif
	
	if (ipEnabled && mode == BOOT_LAST)
	{
		if (fp = fopen(PROC_IPFORWARD, "w"))
		{
			fprintf(fp, "1\n");
			fclose(fp);
		}
		
#ifdef CONFIG_USER_ROUTED_ROUTED		
		if (startRip() == -1)
		{
			printf("start RIP failed !\n");
			ret = -1;
		}
#endif

#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
		if (startOspf() == -1)
		{
			printf("start OSPF failed !\n");
			ret = -1;
		}
#endif
		
#ifdef CONFIG_USER_IGMPPROXY
		if (startIgmproxy() == -1)
		{
			printf("start IGMP proxy failed !\n");
			ret = -1;
		}
#endif

/*		
		// execute firewall rules
		if (setupFirewall() == -1)
		{
			printf("execute firewall rules failed !\n");
			ret = -1;
		}
*/
	}

	// execute firewall rules	
	if (setupFirewall() == -1)
	{
		printf("execute firewall rules failed !\n");
		ret = -1;
	}
#ifdef CONFIG_IP_NF_ALG_ONOFF
         if(setupAlgOnOff()==-1)
                   {
                   printf("execute ALG on-off failed!\n");
                   }
#endif
#ifdef DNS_BIND_PVC_SUPPORT
         DnsBindPvcRoute(ADDDNSROUTE);
         printf("execute the DNS bind !!\n");
#endif	
#if 0 //eason path for pvc add/del and conntrack killall
	// Kaohj --- remove all conntracks
	va_cmd("/bin/ethctl", 2, 1, "conntrack", "killall");
#endif
#ifdef QSETUP_WEB_REDIRECT
	ret |= reset_qsetup_redirect();
#endif


#ifdef WEB_REDIRECT_BY_MAC
	if( -1==start_web_redir_by_mac() )
		ret=-1;
#endif

#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
	unsigned char cp_enable;
	char cp_url[MAX_URL_LEN];

	mib_get(MIB_CAPTIVEPORTAL_ENABLE, (void *)&cp_enable);
	mib_get(MIB_CAPTIVEPORTAL_URL, (void *)cp_url);

	if(cp_enable && cp_url[0])
		if( -1 == start_captiveportal() )
			ret = -1;
#endif

	return ret;
}

#ifdef _CWMP_MIB_ /*jiunming, mib for cwmp-tr069*/
int startCWMP(void)
{
	char vChar=0;
	char strPort[16];
	unsigned int conreq_port=0;
	
	//lan interface enable or disable
	mib_get(CWMP_LAN_IPIFENABLE, (void *)&vChar);
	if(vChar==0)
	{
		va_cmd(IFCONFIG, 2, 1, "br0", "down");
		printf("Disable br0 interface\n");
	}
	//eth0 interface enable or disable
	mib_get(CWMP_LAN_ETHIFENABLE, (void *)&vChar);		
	if(vChar==0)
	{
		va_cmd(IFCONFIG, 2, 1, "eth0", "down");
		printf("Disable eth0 interface\n");
	}

	/*add a wan port to pass */
	mib_get(CWMP_CONREQ_PORT, (void *)&conreq_port);
	if(conreq_port==0) conreq_port=7547;
	sprintf(strPort, "%u", conreq_port );
	va_cmd(IPTABLES, 15, 1, ARG_T, "mangle", "-A", (char *)FW_PREROUTING,
		(char *)ARG_I, "!", (char *)LANIF, "-p", (char *)ARG_TCP,
		(char *)FW_DPORT, strPort, "-j", (char *)"MARK", "--set-mark", RMACC_MARK);

	/*start the cwmpClient program*/
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	mib_get(MIB_CWMP_AUTORUN, (void *)&vChar);
	if(vChar==1)
		va_cmd( "/bin/cwmpClient", 0, 0 );
#else
#if 1
	mib_get(CWMP_FLAG, (void *)&vChar);
	if( vChar&CWMP_FLAG_AUTORUN )
		va_cmd( "/bin/cwmpClient", 0, 0 );
#else
	mib_get(CWMP_NO_DEBUG_MSG, (void *)&vChar);
	if( vChar==1 )
		va_cmd( "/bin/cwmpClient", 2, 0, "-Delay", "-NoDebugMsg" );
	else
		va_cmd( "/bin/cwmpClient", 1, 0, "-Delay" );
#endif
#endif
	return 0;
}
#endif
static char open_adsl_drv(void)
{
	if ((adslFp = fopen(adslDevice, "r")) == NULL) {
		printf("ERROR: failed to open %s, error(%s)\n",adslDevice, strerror(errno));
		return 0;
	};
	return 1;
}

static void close_adsl_drv(void)
{
	if(adslFp)
		fclose(adslFp);

	adslFp = NULL;
}

char adsl_drv_get(unsigned int id, void *rValue, unsigned int len)
{
#ifdef EMBED
	if(open_adsl_drv()) {
		obcif_arg	myarg;
	    	myarg.argsize = (int) len;
	    	myarg.arg = (int) (rValue);

		if (ioctl(fileno(adslFp), id, &myarg) < 0) {
//	    	        printf("ADSL ioctl failed! id=%x\n",id );
			close_adsl_drv();
			return 0;
	       };

		close_adsl_drv();
		return 1;
	}
#endif
	return 0;
}

int getMIB2Str(unsigned int id, char *strbuf)
{
	unsigned char buffer[64];
	
	if (!strbuf)
		return -1;
	
	switch (id) {
		// INET address
		case MIB_ADSL_LAN_IP:
		case MIB_ADSL_LAN_SUBNET:
		case MIB_ADSL_LAN_IP2:
		case MIB_ADSL_LAN_SUBNET2:
		case MIB_ADSL_WAN_DNS1:
		case MIB_ADSL_WAN_DNS2:
		case MIB_ADSL_WAN_DNS3:
		case MIB_ADSL_WAN_DHCPS:
		case MIB_DMZ_IP:
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
		case MIB_SNMP_TRAP_IP:
#endif
		case MIB_DHCP_POOL_START:
		case MIB_DHCP_POOL_END:
		case MIB_DHCPS_DNS1:
		case MIB_DHCPS_DNS2:
		case MIB_DHCPS_DNS3:
//#ifdef DHCPS_POOL_COMPLETE_IP
		case MIB_DHCP_SUBNET_MASK:
//#endif
#ifdef AUTO_PROVISIONING
		case MIB_HTTP_SERVER_IP:
#endif
#ifdef WLAN_1x
		case MIB_WLAN_RS_IP:
#endif
#if defined(ZTE_GENERAL_ROUTER_SC) || defined(TIME_ZONE)
		case MIB_NTP_SERVER_IP1:
		case MIB_NTP_SERVER_IP2:
#endif
		case MIB_ADSL_LAN_DHCP_GATEWAY:
#if 1
#if defined(_PRMT_X_CT_COM_DHCP_)||defined(IP_BASED_CLIENT_TYPE)
		case CWMP_CT_STB_MINADDR:
		case CWMP_CT_STB_MAXADDR:
		case CWMP_CT_PHN_MINADDR:
		case CWMP_CT_PHN_MAXADDR:
		case CWMP_CT_CMR_MINADDR:
		case CWMP_CT_CMR_MAXADDR:
		case CWMP_CT_PC_MINADDR:
		case CWMP_CT_PC_MAXADDR:
		case CWMP_CT_HGW_MINADDR:
		case CWMP_CT_HGW_MAXADDR:             
#endif //_PRMT_X_CT_COM_DHCP_		
#endif

#ifdef ADDRESS_MAPPING
#ifndef MULTI_ADDRESS_MAPPING
		case MIB_LOCAL_START_IP:
		case MIB_LOCAL_END_IP:
		case MIB_GLOBAL_START_IP:
		case MIB_GLOBAL_END_IP:
#endif //end of !MULTI_ADDRESS_MAPPING			
#endif	
#ifdef DEFAULT_GATEWAY_V2
		case MIB_ADSL_WAN_DGW_IP:
#endif	
#ifdef SYSLOG_SUPPORT
#ifdef SYSLOG_REMOTE_LOG
		case MIB_SYSLOG_SERVER_IP:
#endif
#ifdef SEND_LOG
		case MIB_LOG_SERVER_IP:
#endif
#endif
#ifdef _PRMT_TR143_
		case TR143_UDPECHO_SRCIP:
#endif //_PRMT_TR143_
			if(!mib_get( id, (void *)buffer))
				return -1;
			// Mason Yu
			if ( ((struct in_addr *)buffer)->s_addr == INADDR_NONE ) {
				sprintf(strbuf, "%s", "");				
			} else {	
				sprintf(strbuf, "%s", inet_ntoa(*((struct in_addr *)buffer)));
			}			
			break;
		// Ethernet address
		case MIB_ELAN_MAC_ADDR:
		case MIB_WLAN_MAC_ADDR:
			if(!mib_get( id,  (void *)buffer))
				return -1;
			#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
			sprintf(strbuf, "%02x-%02x-%02x-%02x-%02x-%02x", buffer[0], buffer[1],						
				buffer[2], buffer[3], buffer[4], buffer[5]);
			#else
			sprintf(strbuf, "%02x%02x%02x%02x%02x%02x", buffer[0], buffer[1],						
				buffer[2], buffer[3], buffer[4], buffer[5]);
			#endif
			break;
		// Char
		case MIB_ADSL_LAN_CLIENT_START:
		case MIB_ADSL_LAN_CLIENT_END:
		case MIB_IGMP_PROXY_ITF:
//#ifdef CONFIG_USER_UPNPD
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)		
		case MIB_UPNP_EXT_ITF:
#endif		
#ifdef IP_PASSTHROUGH
		case MIB_IPPT_ITF:
#endif
#ifdef WLAN_SUPPORT
		case MIB_WLAN_CHAN_NUM:
		case MIB_WLAN_DISABLED:

		// andrew. 2007/05/10
		case MIB_WLAN_ENABLE_1X:
		case MIB_WLAN_ENCRYPT:
		case MIB_WLAN_WPA_AUTH:
		case MIB_WLAN_NETWORK_TYPE:

#ifdef WLAN_WDS
		case MIB_WLAN_WDS_ENABLED:
#endif
		case MIB_WLAN_MODE:
#ifdef WLAN_QoS
		case MIB_WLAN_QoS:
#endif

#ifdef CONFIG_WIFI_SIMPLE_CONFIG // WPS
		case MIB_WSC_DISABLE:
		case MIB_WSC_CONFIGURED:
		case MIB_WSC_AUTH:
		case MIB_WSC_ENC:
		case MIB_WSC_PSK:
		case MIB_WSC_SSID:
#endif
#endif
#ifdef SYSLOG_SUPPORT
		case MIB_SYSLOG_LOG_LEVEL:
		case MIB_SYSLOG_DISPLAY_LEVEL:
#ifdef SYSLOG_REMOTE_LOG
		case MIB_SYSLOG_MODE:
#endif
#endif
#ifdef DEFAULT_GATEWAY_V2
		case MIB_ADSL_WAN_DGW_ITF:
#endif
#if 0
#if defined(_PRMT_X_CT_COM_DHCP_)||defined(IP_BASED_CLIENT_TYPE)
		case CWMP_CT_STB_MINADDR:
		case CWMP_CT_STB_MAXADDR:
		case CWMP_CT_PHN_MINADDR:
		case CWMP_CT_PHN_MAXADDR:
		case CWMP_CT_CMR_MINADDR:
		case CWMP_CT_CMR_MAXADDR:
		case CWMP_CT_PC_MINADDR:
		case CWMP_CT_PC_MAXADDR:
		case CWMP_CT_HGW_MINADDR:
		case CWMP_CT_HGW_MAXADDR:            
#endif //_PRMT_X_CT_COM_DHCP_
#endif
			if(!mib_get( id,  (void *)buffer))
				return -1;
	   		sprintf(strbuf, "%u", *(unsigned char *)buffer); 
	   		break;
	   	// Short
	   	case MIB_BRCTL_AGEINGTIME:
#ifdef WLAN_SUPPORT
	   	case MIB_WLAN_FRAG_THRESHOLD:
	   	case MIB_WLAN_RTS_THRESHOLD:
	   	case MIB_WLAN_BEACON_INTERVAL:
#ifdef WLAN_1x
	   	case MIB_WLAN_RS_PORT:
#endif
#endif
#ifdef SYSLOG_SUPPORT
#ifdef SYSLOG_REMOTE_LOG
		case MIB_SYSLOG_SERVER_PORT:
#endif
#endif
			if(!mib_get( id,  (void *)buffer))
				return -1;
			sprintf(strbuf, "%u", *(unsigned short *)buffer);
			break;
	   	// Interger
		case MIB_ADSL_LAN_DHCP_LEASE:
		
			// Mason Yu
			if(!mib_get( id,  (void *)buffer))
				return -1;			
			// if MIB_ADSL_LAN_DHCP_LEASE=0xffffffff, it indicate an infinate lease
			if ( *(unsigned long *)buffer == 0xffffffff )
				sprintf(strbuf, "-1");
			else						
				sprintf(strbuf, "%u", *(unsigned int *)buffer);
			break;
		// Interger
#ifdef WEB_REDIRECT_BY_MAC
		case MIB_WEB_REDIR_BY_MAC_INTERVAL:
		
			// Mason Yu
			if(!mib_get( id,  (void *)buffer))
				return -1;			
			// if MIB_WEB_REDIR_BY_MAC_INTERVAL=0xffffffff, it indicate an infinate lease
			//if ( *(unsigned long *)buffer == 0xffffffff )
			//	sprintf(strbuf, "-1");
			//else						
				sprintf(strbuf, "%u", *(unsigned int *)buffer);
			break;
#endif
#ifdef IP_PASSTHROUGH
		case MIB_IPPT_LEASE:
#endif
		case MIB_MAXLOGLEN:
#ifdef _CWMP_MIB_
		case CWMP_INFORM_INTERVAL:
#endif
#ifdef DOS_SUPPORT
		case MIB_DOS_SYSSYN_FLOOD:
		case MIB_DOS_SYSFIN_FLOOD:
		case MIB_DOS_SYSUDP_FLOOD:
		case MIB_DOS_SYSICMP_FLOOD:
		case MIB_DOS_PIPSYN_FLOOD:
		case MIB_DOS_PIPFIN_FLOOD:
		case MIB_DOS_PIPUDP_FLOOD:
		case MIB_DOS_PIPICMP_FLOOD:
		case MIB_DOS_BLOCK_TIME:
#endif
#ifdef TCP_UDP_CONN_LIMIT
		case MIB_CONNLIMIT_UDP:
		case MIB_CONNLIMIT_TCP:
#endif 
#ifdef _CWMP_MIB_
		case CWMP_CONREQ_PORT:
#endif
			if(!mib_get( id,  (void *)buffer))
				return -1;
			sprintf(strbuf, "%u", *(unsigned int *)buffer);
			break;
		// String
		case MIB_ADSL_LAN_DHCP_DOMAIN:
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP		
		case MIB_SNMP_SYS_DESCR:
		case MIB_SNMP_SYS_CONTACT:
		case MIB_SNMP_SYS_LOCATION:
		case MIB_SNMP_SYS_OID:
		case MIB_SNMP_COMM_RO:
		case MIB_SNMP_COMM_RW:
#endif
		case MIB_SNMP_SYS_NAME:
#ifdef WLAN_SUPPORT
		case MIB_WLAN_SSID:
#ifdef WLAN_1x
		case MIB_WLAN_RS_PASSWORD:
#endif
#endif
#ifdef AUTO_PROVISIONING
		case MIB_CONFIG_VERSION:
#endif
#if defined(TIME_ZONE) || defined(ZTE_GENERAL_ROUTER_SC)
		case MIB_NTP_TIMEZONE:
#endif
#ifdef ZTE_GENERAL_ROUTER_SC
		case MIB_NTP_ENABLED:
		case MIB_NTP_SERVER_ID:
#endif
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
		case MIB_NTP_SERVER_HOST1: 
		case MIB_NTP_SERVER_HOST2:
/*ping_zhang:20081217*/
#ifdef _CWMP_MIB_
		case CWMP_PROVISIONINGCODE:
		case CWMP_ACS_URL:
		case CWMP_ACS_USERNAME:
		case CWMP_ACS_PASSWORD:
		case CWMP_CONREQ_USERNAME:
		case CWMP_CONREQ_PASSWORD:
		case CWMP_CONREQ_PATH:
		case CWMP_LAN_CONFIGPASSWD:
		case CWMP_SERIALNUMBER:
		case CWMP_DL_COMMANDKEY:
		case CWMP_RB_COMMANDKEY:
		case CWMP_ACS_PARAMETERKEY:
		case CWMP_CERT_PASSWORD:
#ifdef _PRMT_USERINTERFACE_
		case UIF_AUTOUPDATESERVER:
		case UIF_USERUPDATESERVER:
#endif
		case CWMP_SI_COMMANDKEY:
		case CWMP_ACS_KICKURL:
		case CWMP_ACS_DOWNLOADURL:
#ifdef _PRMT_X_CT_COM_PORTALMNT_
		case CWMP_CT_PM_URL4PC:
		case CWMP_CT_PM_URL4STB:
		case CWMP_CT_PM_URL4MOBILE:
#endif
#endif
#ifdef CONFIG_WIFI_SIMPLE_CONFIG //WPS
		case MIB_WSC_PIN:
#endif
#ifdef SYSLOG_SUPPORT
#ifdef SEND_LOG
		case MIB_LOG_SERVER_NAME:
#endif
#endif
		case MIB_SUSER_NAME:
		case MIB_USER_NAME:
			if(!mib_get( id,  (void *)strbuf)){
				return -1;
			}
			break;		
			
		default:
			return -1;
	}
	
	return 0;		
}

int getSYS2Str(SYSID_T id, char *strbuf)
{
	unsigned char buffer[128], vChar;
	struct sysinfo info;
	int updays, uphours, upminutes, len, i;
	time_t tm;
	struct tm tm_time, *ptm_time;
	FILE *fp;
	unsigned char tmpBuf[64], *pStr;
	unsigned short vUShort;
	
	if (!strbuf)
		return -1;
	
	strbuf[0] = '\0';
	
	switch (id) {
		case SYS_UPTIME:
			sysinfo(&info);
			updays = (int) info.uptime / (60*60*24);
			if (updays)
				sprintf(strbuf, "%d day%s, ", updays, (updays != 1) ? "s" : "");
			len = strlen(strbuf);
			upminutes = (int) info.uptime / 60;
			uphours = (upminutes / 60) % 24;
			upminutes %= 60;
			if(uphours)
				sprintf(&strbuf[len], "%2d:%02d", uphours, upminutes);
			else
				sprintf(&strbuf[len], "%d min", upminutes);
			break;
		case SYS_DATE:
	 		time(&tm);
			memcpy(&tm_time, localtime(&tm), sizeof(tm_time));
			strftime(strbuf, 200, "%a %b %e %H:%M:%S %Z %Y", &tm_time);
			break;
		case SYS_YEAR:
	 		time(&tm);
			ptm_time = localtime(&tm);
			snprintf(strbuf, 64, "%d", (ptm_time->tm_year+ 1900));
			break;
		case SYS_MONTH:
	 		time(&tm);
			ptm_time = localtime(&tm);
			snprintf(strbuf, 64, "%d", (ptm_time->tm_mon+ 1));
			break;
		case SYS_DAY:
	 		time(&tm);
			ptm_time = localtime(&tm);
			snprintf(strbuf, 64, "%d", (ptm_time->tm_mday));
			break;
		case SYS_HOUR:
	 		time(&tm);
			ptm_time = localtime(&tm);
			snprintf(strbuf, 64, "%d", (ptm_time->tm_hour));
			break;
		case SYS_MINUTE:
	 		time(&tm);
			ptm_time = localtime(&tm);
			snprintf(strbuf, 64, "%d", (ptm_time->tm_min));
			break;
		case SYS_SECOND:
	 		time(&tm);
			ptm_time = localtime(&tm);
			snprintf(strbuf, 64, "%d", (ptm_time->tm_sec));
			break;
                case SYS_VENDOR:
                        sprintf(strbuf, "%s", VENDOR_TYPE);
                        break;
                case SYS_MODEL:
                        sprintf(strbuf, "%s", MODEL_TYPE);
                        break;
                case SYS_VERSION:
                        sprintf(strbuf, "%s", VERSION_TYPE);
                        break;
		case SYS_FWVERSION:
			#ifdef EMBED
			tmpBuf[0]=0;
			pStr = 0;
			fp = fopen("/etc/version", "r");
			if (fp!=NULL) {
				fgets(tmpBuf, sizeof(tmpBuf), fp);  //main version
				fclose(fp);
				pStr = strchr(tmpBuf, ' ');
				*pStr=0;
			#if 0
				fp = fopen("/proc/version", "r");  
				if (fp!=NULL) {  //build number
					int i;
					fgets(buffer, sizeof(buffer), fp);
					fclose(fp);
					pStr = strchr(buffer, '#');
					if (pStr!=NULL) {
						pStr++; i=0;
						while (pStr[i]!=' ') i++;
						pStr[i]=0;
					} else {
						pStr = buffer;
						buffer[0]=0;
					};
				}
			#endif
			};
			//sprintf(strbuf, "%s.%s", tmpBuf, pStr);
			sprintf(strbuf, "%s", tmpBuf);
			#else
			sprintf(strbuf, "%s.%s", "Acorp", "1");
			#endif
			break;
		case SYS_LAN_DHCP:
			if ( !mib_get( MIB_DHCP_MODE, (void *)buffer) )
				return -1;
			if (DHCP_LAN_SERVER == buffer[0])
				strcpy(strbuf, STR_ENABLE);
			else
				strcpy(strbuf, STR_DISABLE);
			break;
		case SYS_DHCP_LAN_IP:
#if defined(SECONDARY_IP) && !defined(DHCPS_POOL_COMPLETE_IP)
			mib_get(MIB_ADSL_LAN_ENABLE_IP2, (void *)&vChar);
			if (vChar)
				mib_get(MIB_ADSL_LAN_DHCP_POOLUSE, (void *)&vChar);
#else
			vChar = 0;
#endif
			if (vChar == 0) // primary LAN
				getMIB2Str(MIB_ADSL_LAN_IP, strbuf);
			else // secondary LAN
				getMIB2Str(MIB_ADSL_LAN_IP2, strbuf);
			break;
		case SYS_DHCP_LAN_SUBNET:
#if defined(SECONDARY_IP) && !defined(DHCPS_POOL_COMPLETE_IP)
			mib_get(MIB_ADSL_LAN_ENABLE_IP2, (void *)&vChar);
			if (vChar)
				mib_get(MIB_ADSL_LAN_DHCP_POOLUSE, (void *)&vChar);
#else
			vChar = 0;
#endif
			if (vChar == 0) // primary LAN
				getMIB2Str(MIB_ADSL_LAN_SUBNET, strbuf);
			else // secondary LAN
				getMIB2Str(MIB_ADSL_LAN_SUBNET2, strbuf);
			break;
			// Kaohj
		case SYS_DHCPS_IPPOOL_PREFIX:
#if defined(SECONDARY_IP) && !defined(DHCPS_POOL_COMPLETE_IP)
			mib_get(MIB_ADSL_LAN_ENABLE_IP2, (void *)&vChar);
			if (vChar)
				mib_get(MIB_ADSL_LAN_DHCP_POOLUSE, (void *)&vChar);
#else
			vChar = 0;
#endif
			if (vChar == 0) // primary LAN
				mib_get(MIB_ADSL_LAN_IP, (void *)&tmpBuf[0]) ;
			else // secondary LAN
				mib_get(MIB_ADSL_LAN_IP2, (void *)&tmpBuf[0]) ;
			pStr = tmpBuf;
			sprintf(strbuf, "%d.%d.%d.", pStr[0], pStr[1], pStr[2]);
			break;
#ifdef WLAN_SUPPORT
		case SYS_WLAN:
			if ( !mib_get( MIB_WLAN_DISABLED, (void *)buffer) )
				return -1;
			if (0 == buffer[0])
				strcpy(strbuf, STR_ENABLE);
			else
				strcpy(strbuf, STR_DISABLE);
			break;
		case SYS_WLAN_BCASTSSID:
			if ( !mib_get( MIB_WLAN_HIDDEN_SSID, (void *)buffer) )
				return -1;
			if (0 == buffer[0])
				strcpy(strbuf, STR_ENABLE);
			else
				strcpy(strbuf, STR_DISABLE);
			break;
		case SYS_WLAN_BAND:
			if ( !mib_get( MIB_WLAN_BAND, (void *)buffer) )
				return -1;
			strcpy(strbuf, wlan_band[(BAND_TYPE_T)buffer[0]]);
			break;
		case SYS_WLAN_AUTH:
			if ( !mib_get( MIB_WLAN_AUTH_TYPE, (void *)buffer) )
				return -1;
			strcpy(strbuf, wlan_auth[(AUTH_TYPE_T)buffer[0]]);
			break;
		case SYS_WLAN_PREAMBLE:
			if ( !mib_get( MIB_WLAN_PREAMBLE_TYPE, (void *)buffer) )
				return -1;
			strcpy(strbuf, wlan_preamble[(PREAMBLE_T)buffer[0]]);
			break;
		case SYS_WLAN_ENCRYPT:
			if ( !mib_get( MIB_WLAN_ENCRYPT, (void *)buffer) )
				return -1;
#ifdef ENABLE_WPAAES_WPA2TKIP
			if(buffer[0] == ENCRYPT_WPA){
				mib_get( MIB_WLAN_WPA_CIPHER_SUITE, &vChar);
				if(vChar == WPA_CIPHER_AES)
					buffer[0] = 3;
				else
					buffer[0] = 2;
			}else if(buffer[0] == ENCRYPT_WPA2){
				mib_get( MIB_WLAN_WPA2_CIPHER_SUITE, &vChar);
				if(vChar == WPA_CIPHER_TKIP)
					buffer[0] = 5;
				else
					buffer[0] = 4;
			}else
				buffer[0] = 6;
#else
			if (buffer[0] == ENCRYPT_WPA2)
				buffer[0] = 3;
			else if (buffer[0] == ENCRYPT_WPA2_MIXED)
				buffer[0] = 4;
#endif
			strcpy(strbuf, wlan_encrypt[(ENCRYPT_T)buffer[0]]);
			break;
		case SYS_WLAN_PSKFMT:
			if ( !mib_get( MIB_WLAN_WPA_PSK_FORMAT, (void *)buffer) )
				return -1;
			strcpy(strbuf, wlan_pskfmt[buffer[0]]);
			break;
		case SYS_WLAN_PSKVAL:
			if ( !mib_get( MIB_WLAN_WPA_PSK, (void *)buffer) )
				return -1;
			for (len=0; len<strlen(buffer); len++)
				strbuf[len]='*';
			strbuf[len]='\0';
			break;
		case SYS_WLAN_WEP_KEYLEN:
			if ( !mib_get( MIB_WLAN_WEP, (void *)buffer) )
				return -1;
			strcpy(strbuf, wlan_wepkeylen[buffer[0]]);
			break;
		case SYS_WLAN_WEP_KEYFMT:
			if ( !mib_get( MIB_WLAN_WEP_KEY_TYPE, (void *)buffer) )
				return -1;
			strcpy(strbuf, wlan_wepkeyfmt[buffer[0]]);
			break;
		case SYS_WLAN_WPA_MODE:
			if (!mib_get(MIB_WLAN_WPA_AUTH, (void *)buffer))
				return -1;
			if (buffer[0] == WPA_AUTH_AUTO)
				strcpy(strbuf, "Enterprise (RADIUS)");
			else if (buffer[0] == WPA_AUTH_PSK)
				strcpy(strbuf, "Personal (Pre-Shared Key)");
			break;
		case SYS_WLAN_RSPASSWD:
			if (!mib_get(MIB_WLAN_RS_PASSWORD, (void *)buffer))
				return -1;
			for (len=0; len<strlen(buffer); len++)
				strbuf[len]='*';
			strbuf[len]='\0';
			break;
		// Added by Jenny	
		case SYS_TX_POWER:
			if ( !mib_get( MIB_TX_POWER, (void *)&buffer) )
				return -1;
//add by xlyue to support ZTE more power requirement
//#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
#ifdef WLAN_TX_POWER_DISPLAY
			if (0 == buffer[0])
				strcpy(strbuf, "100%");
			else if(1 == buffer[0])
				strcpy(strbuf, "80%");
			else if(2 == buffer[0])
				strcpy(strbuf, "50%");
			else if(3 == buffer[0])
				strcpy(strbuf, "25%");
			else if(4 == buffer[0])
				strcpy(strbuf, "10%");
#else
			if (0 == buffer[0])
				strcpy(strbuf, "15 mW");
			else if(1 == buffer[0])
				strcpy(strbuf, "30 mW");
			else if(2 == buffer[0])
				strcpy(strbuf, "60 mW");
#endif
			break;	
		case SYS_WLAN_MODE:
			if ( !mib_get( MIB_WLAN_MODE, (void *)&buffer) )
				return -1;
			strcpy(strbuf, wlan_mode[(WLAN_MODE_T)buffer[0]]);
			break;	
		case SYS_WLAN_TXRATE:
			if ( !mib_get( MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&buffer) )
				return -1;
			if (0 == buffer[0]){
#ifdef CONFIG_USB_RTL8192SU_SOFTAP
				unsigned int vInt;
				if ( !mib_get( MIB_WLAN_FIX_RATE, (void *)&vInt) )
#else
				if ( !mib_get( MIB_WLAN_FIX_RATE, (void *)&vUShort) )
#endif					
					return -1;
				for (i=0; i<12; i++)
#ifdef CONFIG_USB_RTL8192SU_SOFTAP
					if (1<<i == vInt)
#else						
					if (1<<i == vUShort)
#endif						
						strcpy(strbuf, wlan_rate[i]);
			}
			else if (1 == buffer[0])
				strcpy(strbuf, STR_AUTO);
			break;
		case SYS_WLAN_BLOCKRELAY:
			if ( !mib_get( MIB_WLAN_BLOCK_RELAY, (void *)buffer) )
				return -1;
			if (0 == buffer[0])
				strcpy(strbuf, STR_DISABLE);
			else
				strcpy(strbuf, STR_ENABLE);
			break;
		case SYS_WLAN_BLOCK_ETH2WIR:
			if ( !mib_get( MIB_WLAN_BLOCK_ETH2WIR, (void *)buffer) )
				return -1;
			if (0 == buffer[0])
				strcpy(strbuf, STR_DISABLE);
			else
				strcpy(strbuf, STR_ENABLE);
			break;
		case SYS_WLAN_AC_ENABLED:
			if ( !mib_get( MIB_WLAN_AC_ENABLED, (void *)&buffer) )
				return -1;
			if (0 == buffer[0])
				strcpy(strbuf, STR_DISABLE);
			else if(1 == buffer[0])
				strcpy(strbuf, "Allow Listed");
			else if(2 == buffer[0])
				strcpy(strbuf, "Deny Listed");
			else
				strcpy(strbuf, STR_ERR);
			break;	
		case SYS_WLAN_WDS_ENABLED:
			if ( !mib_get( MIB_WLAN_WDS_ENABLED, (void *)buffer) )
				return -1;
			if (0 == buffer[0])
				strcpy(strbuf, STR_DISABLE);
			else
				strcpy(strbuf, STR_ENABLE);
			break;
#ifdef WLAN_QoS
		case SYS_WLAN_QoS:
			if ( !mib_get( MIB_WLAN_QoS, (void *)buffer) )
				return -1;
			if (0 == buffer[0])
				strcpy(strbuf, STR_DISABLE);
			else
				strcpy(strbuf, STR_ENABLE);
			break;
#endif			
#endif
		case SYS_DNS_MODE:
			if ( !mib_get( MIB_ADSL_WAN_DNS_MODE, (void *)buffer) )
				return -1;
			if (0 == buffer[0])
				strcpy(strbuf, STR_AUTO);
			else
				strcpy(strbuf, STR_MANUAL);
			break;
			
		case SYS_DHCP_MODE:
			if(!mib_get( MIB_DHCP_MODE, (void *)buffer) )
				return -1;
			strcpy(strbuf, dhcp_mode[(DHCP_TYPE_T)buffer[0]]);
			break;

		case SYS_IPF_OUT_ACTION:
			if(!mib_get(MIB_IPF_OUT_ACTION, (void *)buffer) ){
				return -1;
			}
			if (0 == buffer[0])
				strcpy(strbuf, "Deny");
			else if(1 == buffer[0])
				strcpy(strbuf, "Allow");
			else
				strcpy(strbuf, "err");
			break;			
			
		case SYS_DEFAULT_PORT_FW_ACTION:
			if(!mib_get(MIB_PORT_FW_ENABLE, (void *)buffer) ){
				return -1;
			}
			if (0 == buffer[0])
				strcpy(strbuf, "Disable");
			else if(1 == buffer[0])
				strcpy(strbuf, "Enable");
			else
				strcpy(strbuf, "err");
			break;	
#ifdef CONFIG_EXT_SWITCH
		case SYS_MP_MODE:
			if(!mib_get(MIB_MPMODE, (void *)buffer) ){
				return -1;
			}
			if (0 == buffer[0])
				strcpy(strbuf, "None");
			else if(1 == buffer[0])
				strcpy(strbuf, "Port Mapping");
			else if(2 == buffer[0])
				strcpy(strbuf, "Vlan Enable");
			else if(3 == buffer[0])
				strcpy(strbuf, "IP QoS");
			else if(4 == buffer[0])
				strcpy(strbuf, "IGMP Snooping");
			else
				strcpy(strbuf, "err");
			break;	


		// Added by Jenny
		case SYS_IGMP_SNOOPING:
			if(!mib_get( MIB_MPMODE, (void *)&vChar)) {
				return -1;
			}
			if ((vChar>>2)&0x01)
				strcpy(strbuf, STR_ENABLE);
			else if (((vChar>>2)&0x01) == 0)
				strcpy(strbuf, STR_DISABLE);
			else
				strcpy(strbuf, STR_ERR);
			break;
		case SYS_PORT_MAPPING:
			if(!mib_get( MIB_MPMODE, (void *)&vChar)) {
				return -1;
			}
			if (vChar&0x01)
				strcpy(strbuf, STR_ENABLE);
			else if ((vChar&0x01) == 0)
				strcpy(strbuf, STR_DISABLE);
			else
				strcpy(strbuf, STR_ERR);
			break;
#endif
		case SYS_IP_QOS:
			if(!mib_get( MIB_MPMODE, (void *)&vChar)) {
				return -1;
			}
			if ((vChar>>1)&0x01)
				strcpy(strbuf, STR_ENABLE);
			else if (((vChar>>1)&0x01) == 0)
				strcpy(strbuf, STR_DISABLE);
			else
				strcpy(strbuf, STR_ERR);
			break;

		
		// Added by Mason Yu	
		case SYS_IPF_IN_ACTION:
			if ( !mib_get( MIB_IPF_IN_ACTION, (void *)buffer) )
				return -1;
			if (0 == buffer[0])
				strcpy(strbuf, "Deny");
			else if(1 == buffer[0])
				strcpy(strbuf, "Allow");
			else
				strcpy(strbuf, "err");
			break;		
		
#ifdef SECONDARY_IP
		case SYS_LAN_IP2:
			if (!mib_get( MIB_ADSL_LAN_ENABLE_IP2, (void *)&vChar))
				return -1;
			if (vChar == 0)
				strcpy(strbuf, STR_DISABLE);
			else
				strcpy(strbuf, STR_ENABLE);
			break;

		case SYS_LAN_DHCP_POOLUSE:
			if (!mib_get( MIB_ADSL_LAN_DHCP_POOLUSE, (void *)&vChar))
				return -1;
			if (vChar == 0)
				strcpy(strbuf, "Primary LAN");
			else
				strcpy(strbuf, "Secondary LAN");
			break;
#endif
			
#ifdef URL_BLOCKING_SUPPORT
		case SYS_DEFAULT_URL_BLK_ACTION:
			if(!mib_get(MIB_URL_CAPABILITY, (void *)buffer) ){
				return -1;
			}
			if (0 == buffer[0])
				strcpy(strbuf, "Disable");
			else if(1 == buffer[0])
				strcpy(strbuf, "Enable");
			else
				strcpy(strbuf, "err");
			break;
#endif

#ifdef DOMAIN_BLOCKING_SUPPORT
		case SYS_DEFAULT_DOMAIN_BLK_ACTION:
			if(!mib_get(MIB_DOMAINBLK_CAPABILITY, (void *)buffer) ){
				return -1;
			}
			if (0 == buffer[0])
				strcpy(strbuf, "Disable");
			else if(1 == buffer[0])
				strcpy(strbuf, "Enable");
			else
				strcpy(strbuf, "err");
			break;
#endif
		case SYS_DSL_OPSTATE:
			getAdslInfo(ADSL_GET_MODE, buffer, 64);
			if (buffer[0] != '\0')
				sprintf(strbuf, "%s,", buffer);
			getAdslInfo(ADSL_GET_STATE, buffer, 64);
			strcat(strbuf, buffer);
			break;

		default:
			return -1;
	}
	
	return 0;
}

int ifWanNum(const char *name) 
{   	
	int ifnum=0;
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;	
	int type;
	
	if ( !strcmp(name, "all") )
		type = 0;
	else if ( !strcmp(name, "rt") )
		type = 1;	// route interface
	else if ( !strcmp(name, "br") )
		type = 2;	// bridge interface
	else
		type = 1;	// default to route
	
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{  			
			return -1;
		}
		
		if (Entry.enable == 0)
			continue;
		
		if (type == 2) {
			if (Entry.cmode == ADSL_BR1483)
			{				
				ifnum++;
			}
		}
		else {  // rt or all (1 or 0)
			if (type == 1 && Entry.cmode == ADSL_BR1483)
				continue;
			
			ifnum++;
		}
	}
		
	return ifnum;
}

#ifdef WLAN_SUPPORT
#ifdef WLAN_CLIENT
int getWlStaNum( char *interface, int *num )
{
    int skfd;
    unsigned short staNum;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) {
      close( skfd );
      /* If no wireless name : no wireless extensions */
      return -1;
    }

    wrq.u.data.pointer = (caddr_t)&staNum;
    wrq.u.data.length = sizeof(staNum);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLSTANUM, &wrq) < 0) {
      close( skfd );
      return -1;
    }

    *num  = (int)staNum;

    close( skfd );

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
int getWlSiteSurveyResult(char *interface, SS_STATUS_Tp pStatus )
{
    int skfd;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) {
       close( skfd );
      /* If no wireless name : no wireless extensions */
       return -1;
    }

    wrq.u.data.pointer = (caddr_t)pStatus;

    if ( pStatus->number == 0 )
    	wrq.u.data.length = sizeof(SS_STATUS_T);
    else
        wrq.u.data.length = sizeof(pStatus->number);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLGETBSSDB, &wrq) < 0) {
      close( skfd );
      return -1;
    }

    close( skfd );

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
int getWlJoinRequest(char *interface, pBssDscr pBss, unsigned char *res)
{
    int skfd;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) {
      close( skfd );
      /* If no wireless name : no wireless extensions */
      return -1;
    }

    wrq.u.data.pointer = (caddr_t)pBss;
    wrq.u.data.length = sizeof(BssDscr);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLJOINREQ, &wrq) < 0) {
      close( skfd );
      return -1;
    }

    close( skfd );
    
    *res = wrq.u.data.pointer[0];

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
int getWlJoinResult(char *interface, unsigned char *res)
{
    int skfd;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) {
      close( skfd );
      /* If no wireless name : no wireless extensions */
      return -1;
    }

    wrq.u.data.pointer = (caddr_t)res;
    wrq.u.data.length = 1;

    if (iw_get_ext(skfd, interface, SIOCGIWRTLJOINREQSTATUS, &wrq) < 0) {
      close( skfd );
      return -1;
    }

    close( skfd );

    return 0;
}



/////////////////////////////////////////////////////////////////////////////
int getWlSiteSurveyRequest(char *interface, int *pStatus)
{
    int skfd;
    struct iwreq wrq;
    unsigned char result;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) {
      close( skfd );
      /* If no wireless name : no wireless extensions */
      return -1;
    }

    wrq.u.data.pointer = (caddr_t)&result;
    wrq.u.data.length = sizeof(result);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLSCANREQ, &wrq) < 0) {
      close( skfd );
      return -1;
    }

    close( skfd );

    if ( result == 0xff )
    	*pStatus = -1;
    else
	*pStatus = (int) result;

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
int getWlBssInfo(char *interface, bss_info *pInfo)
{
    int skfd;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) {
      close( skfd );
      /* If no wireless name : no wireless extensions */
      return -1;
    }

    wrq.u.data.pointer = (caddr_t)pInfo;
    wrq.u.data.length = sizeof(bss_info);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLGETBSSINFO, &wrq) < 0) {
      close( skfd );
      return -1;
    }

    close( skfd );

    return 0;
}
#endif	// of WLAN_CLIENT

#ifdef WLAN_WDS
/////////////////////////////////////////////////////////////////////////////
int getWdsInfo(char *interface, char *pInfo)
{

    int skfd;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) {
      close( skfd );
      /* If no wireless name : no wireless extensions */
      return -1;
    }

    wrq.u.data.pointer = (caddr_t)pInfo;
    wrq.u.data.length = MAX_WDS_NUM*sizeof(WDS_INFO_T);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLGETWDSINFO, &wrq) < 0) {
      close( skfd );
      return -1;
    }

    close( skfd );

    return 0;
}

#endif

int getWlVersion( char *interface, char *verstr )
{
    int skfd;
    unsigned char vernum[4];
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) {
      close( skfd );
      /* If no wireless name : no wireless extensions */
      return -1;
    }

    memset(vernum, 0, 4);
    wrq.u.data.pointer = (caddr_t)&vernum[0];
    wrq.u.data.length = sizeof(vernum);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLDRVVERSION, &wrq) < 0) {
      close( skfd );
      return -1;
    }

    close( skfd );
    
    sprintf(verstr, "%d.%d.%d", vernum[0], vernum[1], vernum[2]);

    return 0;
}

#endif // of WLAN_SUPPORT

void remote_access_modify(  MIB_CE_ACC_T accEntry, int enable)
{
	//MIB_CE_ACC_Tp paccEntry;
	//MIB_CE_ACC_T accEntry;
	char *act;
	char strPort[8];
	int ret;

	
	if (enable)
		act = (char *)"-I";
	else
		act = (char *)FW_DEL;
	#ifdef ZTE_GENERAL_ROUTER_SC
	char interfaceName[64]={0};
	if((accEntry.protocol == ADSL_PPPoE) || (accEntry.protocol == ADSL_PPPoA) ) {
	sprintf(interfaceName,"ppp%u",PPP_INDEX(accEntry.ifIndex));
	}else
	sprintf(interfaceName,"vc%u",VC_INDEX(accEntry.ifIndex));
	
	#endif
	
	//if (!mib_chain_get(MIB_ACC_TBL, 0, (void *)&accEntry))
	//	return;
	#ifdef ZTE_GENERAL_ROUTER_SC
	#ifdef CONFIG_USER_TELNETD_TELNETD
	if (accEntry.telnet & 0x01) {	// can WAN access
		snprintf(strPort, sizeof(strPort)-1, "%u", accEntry.telnet_port);
		va_cmd(IPTABLES, 14, 1, ARG_T, "mangle", act, (char *)FW_PREROUTING,
		(char *)ARG_I, interfaceName, "-p", (char *)ARG_TCP,
		(char *)FW_DPORT, strPort, "-j", (char *)"MARK", "--set-mark", RMACC_MARK);
		va_cmd(IPTABLES, 12, 1, "-t", "nat", (char *)act,
		(char *)FW_PREROUTING, (char *)ARG_I, interfaceName,
		 "-p", (char *)ARG_TCP,
		(char *)FW_DPORT, "23", "-j",
		(char *)FW_ACCEPT);
	// telnet service: bring up by inetd
		if (accEntry.telnet_port != 23) {
			ret = va_cmd(IPTABLES, 14, 1, "-t", "nat",
					(char *)act, (char *)FW_PREROUTING,
					(char *)ARG_I, interfaceName,
					"-p", ARG_TCP,
					(char *)FW_DPORT, strPort, "-j",
					"REDIRECT", "--to-ports", "23");
		}
		
	}
	#endif
	#else
	// telnet service: bring up by inetd
	#ifdef CONFIG_USER_TELNETD_TELNETD
	if (!(accEntry.telnet & 0x02)) {	// not LAN access
		// iptables -A inacc -i $LAN_IF -p TCP --dport 23 -j DROP
		va_cmd(IPTABLES, 10, 1, act, (char *)FW_INACC,
		(char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP,
		(char *)FW_DPORT, "23", "-j", (char *)FW_DROP);
	}
	/*
	if (paccEntry->telnet & 0x02) {	// can LAN access
		// iptables -A INPUT -i $LAN_IF -p TCP --dport 23 -j ACCEPT
		va_cmd(IPTABLES, 10, 1, act, (char *)FW_INPUT,
		(char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP,
		(char *)FW_DPORT, "23", "-j", (char *)FW_ACCEPT);
	}
	*/
	if (accEntry.telnet & 0x01) {	// can WAN access
		snprintf(strPort, sizeof(strPort)-1, "%u", accEntry.telnet_port);
		va_cmd(IPTABLES, 15, 1, ARG_T, "mangle", act, (char *)FW_PREROUTING,
		(char *)ARG_I, "!", (char *)LANIF, "-p", (char *)ARG_TCP,
		(char *)FW_DPORT, strPort, "-j", (char *)"MARK", "--set-mark", RMACC_MARK);

		// redirect if this is not standard port
		if (accEntry.telnet_port != 23) {
			ret = va_cmd(IPTABLES, 15, 1, "-t", "nat",
					(char *)act, (char *)FW_PREROUTING,
					(char *)ARG_I, "!", (char *)LANIF,
					"-p", ARG_TCP,
					(char *)FW_DPORT, strPort, "-j",
					"REDIRECT", "--to-ports", "23");
		}

		// iptables -A inacc -i ! $LAN_IF -p TCP --dport 23 -j ACCEPT	
		#if 0
		va_cmd(IPTABLES, 11, 1, act, (char *)FW_INACC,
		(char *)ARG_I, "!", (char *)LANIF, "-p", (char *)ARG_TCP,
		(char *)FW_DPORT, "23", "-j", (char *)FW_ACCEPT);

		fprintf(stderr, "telnet = %d\n", accEntry.telnet_port);
		if (accEntry.telnet_port != 23) {
			ret = va_cmd(IPTABLES, 15, 1, "-t", "nat",
					(char *)act, (char *)FW_PREROUTING,
					(char *)ARG_I, "!", (char *)LANIF,
					"-p", ARG_TCP,
					(char *)FW_DPORT, strPort, "-j",
					"REDIRECT", "--to-ports", "23");
			fprintf(stderr, "telnet cmd = %d\n", ret);
		}
		#endif

	}
	#endif
	#endif

	
	#ifdef CONFIG_USER_FTPD_FTPD
	#ifdef ZTE_GENERAL_ROUTER_SC
	// ftp service: bring up by inetd		
	
	if (accEntry.ftp & 0x01) {	// can WAN access	
		snprintf(strPort, sizeof(strPort)-1, "%u", accEntry.ftp_port);
		va_cmd(IPTABLES, 14, 1, ARG_T, "mangle", act, (char *)FW_PREROUTING,
		(char *)ARG_I, interfaceName, "-p", (char *)ARG_TCP,
		(char *)FW_DPORT, strPort, "-j", (char *)"MARK", "--set-mark", RMACC_MARK);
		va_cmd(IPTABLES, 12, 1, "-t", "nat", (char *)act,
								(char *)FW_PREROUTING, (char *)ARG_I, interfaceName,
								 "-p", (char *)ARG_TCP,
								(char *)FW_DPORT, "21", "-j",
								(char *)FW_ACCEPT);
		// redirect if this is not standard port
		if (accEntry.ftp_port != 21) {
			ret = va_cmd(IPTABLES, 14, 1, "-t", "nat",
					(char *)act, (char *)FW_PREROUTING,
					(char *)ARG_I, interfaceName,
					"-p", ARG_TCP,
					(char *)FW_DPORT, strPort, "-j",
					"REDIRECT", "--to-ports", "21");
		}	
		
	}
	
	#else
	// ftp service: bring up by inetd
	if (!(accEntry.ftp & 0x02)) {	// not LAN access
		// iptables -A inacc -i $LAN_IF -p TCP --dport 21 -j DROP
		va_cmd(IPTABLES, 10, 1, act, (char *)FW_INACC,
		(char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP,
		(char *)FW_DPORT, "21", "-j", (char *)FW_DROP);
	}
	/*
	if (paccEntry->ftp & 0x02) {	// can LAN access
		// iptables -A INPUT -i $LAN_IF -p TCP --dport 21 -j ACCEPT
		va_cmd(IPTABLES, 10, 1, act, (char *)FW_INPUT,
		(char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP,
		(char *)FW_DPORT, "21", "-j", (char *)FW_ACCEPT);
	}
	*/
	if (accEntry.ftp & 0x01) {	// can WAN access
		snprintf(strPort, sizeof(strPort)-1, "%u", accEntry.ftp_port);
		va_cmd(IPTABLES, 15, 1, ARG_T, "mangle", act, (char *)FW_PREROUTING,
		(char *)ARG_I, "!", (char *)LANIF, "-p", (char *)ARG_TCP,
		(char *)FW_DPORT, strPort, "-j", (char *)"MARK", "--set-mark", RMACC_MARK);

		// redirect if this is not standard port
		if (accEntry.ftp_port != 21) {
			ret = va_cmd(IPTABLES, 15, 1, "-t", "nat",
					(char *)act, (char *)FW_PREROUTING,
					(char *)ARG_I, "!", (char *)LANIF,
					"-p", ARG_TCP,
					(char *)FW_DPORT, strPort, "-j",
					"REDIRECT", "--to-ports", "21");
		}

		#if 0
		// iptables -A inacc -i ! $LAN_IF -p TCP --dport 21 -j ACCEPT
		va_cmd(IPTABLES, 11, 1, act, (char *)FW_INACC,
		(char *)ARG_I, "!", (char *)LANIF, "-p", (char *)ARG_TCP,
		(char *)FW_DPORT, "21", "-j", (char *)FW_ACCEPT);
		#endif
	}
	#endif
	#endif
	#ifdef CONFIG_USER_TFTPD_TFTPD
	#ifdef ZTE_GENERAL_ROUTER_SC
	// tftp service: bring up by inetd	
	if (accEntry.tftp & 0x01) {	// can WAN access
		// iptables -A inacc -i ! $LAN_IF -p UDP --dport 69 -j ACCEPT
		va_cmd(IPTABLES, 10, 1, act, (char *)FW_INACC,
		(char *)ARG_I, interfaceName, "-p", (char *)ARG_UDP,
		(char *)FW_DPORT, "69", "-j", (char *)FW_ACCEPT);
		va_cmd(IPTABLES, 12, 1, "-t", "nat", (char *)act,
		(char *)FW_PREROUTING, (char *)ARG_I, interfaceName,
		 "-p", (char *)ARG_UDP,
		(char *)FW_DPORT, "69", "-j",
		(char *)FW_ACCEPT);
	}
	#else
	// tftp service: bring up by inetd
	if (!(accEntry.tftp & 0x02)) {	// not LAN access
		// iptables -A inacc -i $LAN_IF -p UDP --dport 69 -j DROP
		va_cmd(IPTABLES, 10, 1, act, (char *)FW_INACC,
		(char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_UDP,
		(char *)FW_DPORT, "69", "-j", (char *)FW_DROP);
	}
	/*
	if (paccEntry->tftp & 0x02) {	// can LAN access
		// iptables -A INPUT -i $LAN_IF -p UDP --dport 69 -j ACCEPT
		va_cmd(IPTABLES, 10, 1, act, (char *)FW_INPUT,
		(char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_UDP,
		(char *)FW_DPORT, "69", "-j", (char *)FW_ACCEPT);
	}
	*/
	if (accEntry.tftp & 0x01) {	// can WAN access
		// iptables -A inacc -i ! $LAN_IF -p UDP --dport 69 -j ACCEPT
		va_cmd(IPTABLES, 11, 1, act, (char *)FW_INACC,
		(char *)ARG_I, "!", (char *)LANIF, "-p", (char *)ARG_UDP,
		(char *)FW_DPORT, "69", "-j", (char *)FW_ACCEPT);
	}
	#endif
	#endif
	#ifdef ZTE_GENERAL_ROUTER_SC
	// HTTP service	
		if (accEntry.web & 0x01) {	// can WAN access
			snprintf(strPort, sizeof(strPort)-1, "%u", accEntry.web_port);
			// iptables -A inacc -i ! $LAN_IF -p TCP --dport 80 -j ACCEPT
			va_cmd(IPTABLES, 14, 1, ARG_T, "mangle", act, (char *)FW_PREROUTING,
				(char *)ARG_I, interfaceName, "-p", (char *)ARG_TCP,
				(char *)FW_DPORT, strPort, "-j", (char *)"MARK", "--set-mark", RMACC_MARK);
			#if 0
			va_cmd(IPTABLES, 11, 1, act, (char *)FW_INACC,
			(char *)ARG_I, "!", (char *)LANIF, "-p", (char *)ARG_TCP,
			(char *)FW_DPORT, "80", "-j", (char *)FW_ACCEPT);
			#endif
			va_cmd(IPTABLES, 12, 1, "-t", "nat", (char *)act,
								(char *)FW_PREROUTING, (char *)ARG_I, interfaceName,
								 "-p", (char *)ARG_TCP,
								(char *)FW_DPORT, "80", "-j",
								(char *)FW_ACCEPT);
			if (accEntry.web_port != 80) {
				va_cmd(IPTABLES, 14, 1, "-t", "nat",
					(char *)act, (char *)FW_PREROUTING,
					(char *)ARG_I, interfaceName,
					"-p", ARG_TCP,
					(char *)FW_DPORT, strPort, "-j",
					"REDIRECT", "--to-ports", "80");
			}
			

		}
	#else
	// HTTP service
	//if (paccEntry->web !=0) {	// have web server
		if (!(accEntry.web & 0x02)) {	// not LAN access
			// iptables -A inacc -i $LAN_IF -p TCP --dport 80 -j DROP
			va_cmd(IPTABLES, 10, 1, act, (char *)FW_INACC,
			(char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP,
			(char *)FW_DPORT, "80", "-j", (char *)FW_DROP);
		}
		/*
		if (paccEntry->web & 0x02) {	// can LAN access
			// iptables -A INPUT -i $LAN_IF -p TCP --dport 80 -j ACCEPT
			va_cmd(IPTABLES, 10, 1, act, (char *)FW_INPUT,
			(char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP,
			(char *)FW_DPORT, "80", "-j", (char *)FW_ACCEPT);
		}
		*/
		if (accEntry.web & 0x01) {	// can WAN access
			snprintf(strPort, sizeof(strPort)-1, "%u", accEntry.web_port);
			// iptables -A inacc -i ! $LAN_IF -p TCP --dport 80 -j ACCEPT
			va_cmd(IPTABLES, 15, 1, ARG_T, "mangle", act, (char *)FW_PREROUTING,
				(char *)ARG_I, "!", (char *)LANIF, "-p", (char *)ARG_TCP,
				(char *)FW_DPORT, strPort, "-j", (char *)"MARK", "--set-mark", RMACC_MARK);
			#if 0
			va_cmd(IPTABLES, 11, 1, act, (char *)FW_INACC,
			(char *)ARG_I, "!", (char *)LANIF, "-p", (char *)ARG_TCP,
			(char *)FW_DPORT, "80", "-j", (char *)FW_ACCEPT);
			#endif
			if (accEntry.web_port != 80) {
				va_cmd(IPTABLES, 15, 1, "-t", "nat",
					(char *)act, (char *)FW_PREROUTING,
					(char *)ARG_I, "!", (char *)LANIF,
					"-p", ARG_TCP,
					(char *)FW_DPORT, strPort, "-j",
					"REDIRECT", "--to-ports", "80");
			}

		}
	#endif

  #ifdef CONFIG_USER_BOA_WITH_SSL
  	  //HTTPS service
		if (!(accEntry.https & 0x02)) {	// not LAN access
			// iptables -A inacc -i $LAN_IF -p TCP --dport 443 -j DROP
			va_cmd(IPTABLES, 10, 1, act, (char *)FW_INACC,
			(char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP,
			(char *)FW_DPORT, "443", "-j", (char *)FW_DROP);
		}
		if (accEntry.https & 0x01) {	// can WAN access
			snprintf(strPort, sizeof(strPort)-1, "%u", accEntry.https_port);
			// iptables -t mangle  -I PREROUTING -i !$LAN_IF -p TCP --dport 443 -j MARK --set-mark 0x1000
			va_cmd(IPTABLES, 15, 1, ARG_T, "mangle", act, (char *)FW_PREROUTING,
				(char *)ARG_I, "!", (char *)LANIF, "-p", (char *)ARG_TCP,
				(char *)FW_DPORT, strPort, "-j", (char *)"MARK", "--set-mark", RMACC_MARK);

			if (accEntry.https_port != 443) {
				va_cmd(IPTABLES, 15, 1, "-t", "nat",
					(char *)act, (char *)FW_PREROUTING,
					(char *)ARG_I, "!", (char *)LANIF,
					"-p", ARG_TCP,
					(char *)FW_DPORT, strPort, "-j",
					"REDIRECT", "--to-ports", "443");
			}

		}
  #endif
  #ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
   #ifdef ZTE_GENERAL_ROUTER_SC
	//}
		if (accEntry.snmp & 0x01) {	// can WAN access
			// iptables -A inacc -i ! $LAN_IF -p UDP --dport 161:162 -m limit
			//  --limit 100/s --limit-burst 500 -j ACCEPT
			va_cmd(IPTABLES, 16, 1, act, (char *)FW_INACC,
				(char *)ARG_I, interfaceName, "-p",
				(char *)ARG_UDP, (char *)FW_DPORT, "161:162", "-m",
				"limit", "--limit", "100/s", "--limit-burst",
				"500", "-j", (char *)FW_ACCEPT);
			va_cmd(IPTABLES, 12, 1, "-t", "nat",(char *)act,
								(char *)FW_PREROUTING, (char *)ARG_I, interfaceName,
								 "-p", (char *)ARG_UDP,
								(char *)FW_DPORT, "161:162", "-j",
								(char *)FW_ACCEPT);
		}
	#else
	// snmp service
	//if (accEntry.snmp !=0) {	// have snmp server		
		if (!(accEntry.snmp & 0x02)) {	// not LAN access
			// iptables -A inacc -i $LAN_IF -p UDP --dport 161:162 -j DROP			
			va_cmd(IPTABLES, 10, 1, act, (char *)FW_INACC,
			(char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_UDP,
			(char *)FW_DPORT, "161:162", "-j", (char *)FW_DROP);
		}
		/*
		if (paccEntry->snmp & 0x02) {	// can LAN access
			// iptables -A INPUT -i $LAN_IF -p UDP --dport 161:162 -j ACCEPT
			va_cmd(IPTABLES, 10, 1, act, (char *)FW_INPUT,
			(char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_UDP,
			(char *)FW_DPORT, "161:162", "-j", (char *)FW_ACCEPT);
		}
		*/
		if (accEntry.snmp & 0x01) {	// can WAN access
			// iptables -A inacc -i ! $LAN_IF -p UDP --dport 161:162 -m limit
			//  --limit 100/s --limit-burst 500 -j ACCEPT
			va_cmd(IPTABLES, 17, 1, act, (char *)FW_INACC,
				(char *)ARG_I, "!", (char *)LANIF, "-p",
				(char *)ARG_UDP, (char *)FW_DPORT, "161:162", "-m",
				"limit", "--limit", "100/s", "--limit-burst",
				"500", "-j", (char *)FW_ACCEPT);
		}
	#endif
	#endif
	#ifdef CONFIG_USER_SSH_DROPBEAR
	#ifdef ZTE_GENERAL_ROUTER_SC
	// ssh service
		if (accEntry.ssh & 0x01) {	// can WAN access
	//}
			va_cmd(IPTABLES, 10, 1, act, (char *)FW_INACC,
			(char *)ARG_I, interfaceName, "-p", (char *)ARG_TCP,
			(char *)FW_DPORT, "22", "-j", (char *)FW_ACCEPT);
			va_cmd(IPTABLES, 12, 1, "-t", "nat", (char *)act,
								(char *)FW_PREROUTING, (char *)ARG_I, interfaceName,
								 "-p", (char *)ARG_TCP,
								(char *)FW_DPORT, "22", "-j",
								(char *)FW_ACCEPT);
		}
	#else
		if (!(accEntry.ssh & 0x02)) {	// not LAN access
			// iptables -A inacc -i $LAN_IF -p TCP --dport 22 -j DROP
			va_cmd(IPTABLES, 10, 1, act, (char *)FW_INACC,
			(char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP,
			(char *)FW_DPORT, "22", "-j", (char *)FW_DROP);
		}
		if (accEntry.ssh & 0x01) {	// can WAN access
			// iptables -A inacc -i ! $LAN_IF -p TCP --dport 22 -j ACCEPT
			va_cmd(IPTABLES, 11, 1, act, (char *)FW_INACC,
			(char *)ARG_I, "!", (char *)LANIF, "-p", (char *)ARG_TCP,
			(char *)FW_DPORT, "22", "-j", (char *)FW_ACCEPT);
		}
	#endif
	#endif
	#ifdef ZTE_GENERAL_ROUTER_SC
	if (accEntry.icmp & 0x01) // can WAN access
	{
		//   --limit 1/s -j ACCEPT
		va_cmd(IPTABLES, 14, 1, act, (char *)FW_INACC,
			(char *)ARG_I,interfaceName, "-p", "ICMP",
			"--icmp-type", "echo-request", "-m", "limit",
			"--limit", "1/s", "-j", (char *)FW_ACCEPT);
		va_cmd(IPTABLES, 16, 1,"-t", "nat", (char *)act, (char *)FW_PREROUTING,
			(char *)ARG_I,interfaceName, "-p", "ICMP",
			"--icmp-type", "echo-request", "-m", "limit",
			"--limit", "1/s", "-j", (char *)FW_ACCEPT);		
	}
	#else
	if (accEntry.icmp & 0x01) // can WAN access
	{
		// iptables -A inacc -i ! $LAN_IF  -p ICMP --icmp-type echo-request -m limit
		//   --limit 1/s -j ACCEPT
		va_cmd(IPTABLES, 15, 1, act, (char *)FW_INACC,
			(char *)ARG_I, "!", (char *)LANIF, "-p", "ICMP",
			"--icmp-type", "echo-request", "-m", "limit",
			"--limit", "1/s", "-j", (char *)FW_ACCEPT);
	}
	#endif
}

void filter_set_remote_access(int enable)
{
        #ifdef ZTE_GENERAL_ROUTER_SC
	int rmtacc_index=0;
	int rmtacc_nums=mib_chain_total(MIB_ACC_TBL);
	MIB_CE_ACC_T accEntry;
	if(!rmtacc_nums) return;
	for(rmtacc_index=0;rmtacc_index<rmtacc_nums;rmtacc_index++)
	{
	memset(&accEntry,'\0',sizeof(MIB_CE_ACC_T));
	mib_chain_get(MIB_ACC_TBL,rmtacc_index,&accEntry);	
	if(accEntry.protocol!=ADSL_BR1483)
	remote_access_modify( accEntry, enable );	
	}	
	#else
	MIB_CE_ACC_T accEntry;
	if (!mib_chain_get(MIB_ACC_TBL, 0, (void *)&accEntry))
		return;
	remote_access_modify( accEntry, enable );	
	#endif
	return;
	
}

#ifdef IP_ACL
void filter_set_acl(int enable)
{
	int i, total;
	struct in_addr src;		
	char ssrc[40];
	MIB_CE_ACL_IP_T Entry;

#ifdef ACL_IP_RANGE
	struct in_addr start,end;		
	char sstart[16],send[16];
#endif

#ifdef MAC_ACL
	MIB_CE_ACL_MAC_T MacEntry;
	char macaddr[18];
#endif

	
	// Added by Mason Yu for ACL.
	// check if ACL Capability is enabled ?
	if (!enable)
		va_cmd(IPTABLES, 2, 1, "-F", "aclblock");
	else {
		// Add policy to aclblock chain
		total = mib_chain_total(MIB_ACL_IP_TBL);
		for (i=0; i<total; i++) {
			if (!mib_chain_get(MIB_ACL_IP_TBL, i, (void *)&Entry))
			{  				
				return;
			}

//    		printf("\nstartip=%x,endip=%x\n",*(unsigned long*)Entry.startipAddr,*(unsigned long*)Entry.endipAddr);
			
			// Check if this entry is enabled
			if ( Entry.Enabled == 1 ) {		
#ifdef ACL_IP_RANGE
				start.s_addr = *(unsigned long*)Entry.startipAddr;
				end.s_addr = *(unsigned long*)Entry.endipAddr;
				strcpy(sstart, inet_ntoa(start));
				strcpy(send, inet_ntoa(end));
				strcpy(ssrc,sstart);
				strcat(ssrc,"-");
				strcat(ssrc,send);
	
#else
				src.s_addr = *(unsigned long *)Entry.ipAddr;
		        	
				// inet_ntoa is not reentrant, we have to
				// copy the static memory before reuse it
				strcpy(ssrc, inet_ntoa(src));
				snprintf(ssrc, 20, "%s/%d", ssrc, Entry.maskbit);

#endif	        	
		        	if ( Entry.Interface == IF_DOMAIN_LAN ) {
		 			
#ifdef ACL_IP_RANGE
					if(*(unsigned long*)Entry.startipAddr != *(unsigned long*)Entry.endipAddr) 
					// iptables -A aclblock -m iprange --src-range x.x.x.x-1x.x.x.x -j RETURN 
						va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, "aclblock", "-i", "br0", "-m", "iprange", "--src-range", ssrc, "-j", (char *)FW_RETURN);
					else
						va_cmd(IPTABLES, 8, 1, (char *)FW_ADD, "aclblock", "-i", "br0", "-s", sstart, "-j", (char *)FW_RETURN);
#else
					// iptables -A INPUT -s xxx.xxx.xxx.xxx	
					va_cmd(IPTABLES, 8, 1, (char *)FW_ADD, "aclblock", "-i", "br0", "-s", ssrc, "-j", (char *)FW_RETURN);
#endif
				} else {
					// iptables -A INPUT -s xxx.xxx.xxx.xxx		
					va_cmd(IPTABLES, 9, 1, (char *)FW_ADD, "aclblock", "-i", "!", "br0", "-s", ssrc, "-j", (char *)FW_RETURN);
				}
			}
		}
#ifdef MAC_ACL
		total = mib_chain_total(MIB_ACL_MAC_TBL);
		for (i=0; i<total; i++) {
			if (!mib_chain_get(MIB_ACL_MAC_TBL, i, (void *)&MacEntry))
			{
				return;
			}
			
			// Check if this entry is enabled
			if ( MacEntry.Enabled == 1 ) {			
				snprintf(macaddr, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
					MacEntry.macAddr[0], MacEntry.macAddr[1],
					MacEntry.macAddr[2], MacEntry.macAddr[3],
					MacEntry.macAddr[4], MacEntry.macAddr[5]);
		        	
		        if ( MacEntry.Interface == IF_DOMAIN_LAN ) {
		 			// iptables -A aclblock -i br0  -m mac --mac-source $MAC -j RETURN
					va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, "aclblock",
							(char *)ARG_I, "br0", "-m", "mac",
							"--mac-source",  macaddr, "-j", (char *)FW_RETURN);	
				} else {
					// iptables -A aclblock -i ! br0  -m mac --mac-source $MAC -j RETURN
					va_cmd(IPTABLES, 11, 1, (char *)FW_ADD, "aclblock",
							(char *)ARG_I, "!", "br0", "-m", "mac",
							"--mac-source",  macaddr, "-j", (char *)FW_RETURN);	
				}
			}
		}
#endif

		// allowing DNS request during ACL enabled
		// iptables -A aclblock -p udp -i br0 --dport 53 -j RETURN			
		va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, "aclblock", "-p", "udp", "-i", "br0",(char*)FW_DPORT, (char *)PORT_DNS, "-j", (char *)FW_RETURN);
		// allowing DHCP request during ACL enabled
		// iptables -A aclblock -p udp -i br0 --dport 67 -j RETURN	
		va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, "aclblock", "-p", "udp", "-i", "br0", (char*)FW_DPORT, (char *)PORT_DHCP, "-j", (char *)FW_RETURN);
#ifndef ZTE_GENERAL_ROUTER_SC
		// ipbables -A aclblock -i !br0 -j RETURN
		// andrew, removed. We use White-List policy here. 2007/11/07
		//va_cmd(IPTABLES, 7, 1, (char *)FW_ADD, "aclblock", "-i", "!", "br0",  "-j", (char *)FW_RETURN);
#endif
		// iptables -A aclblock -j DROP			
		va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, "aclblock", "-j", (char *)FW_DROP);
	}
}
#endif

#ifdef NAT_CONN_LIMIT
void set_conn_limit()
{
	int i, total;
	char ssrc[20];
	char connNum[10];
	MIB_CE_CONN_LIMIT_T Entry;
	
	// Add policy to connlimit chain
	//iptables -A connlimit -m state --state RELATED,ESTABLISHED -j RETURN
	va_cmd(IPTABLES, 8, 1, (char *)FW_ADD, "connlimit", "-m", "state",
		"--state", "ESTABLISHED,RELATED", "-j", (char *)FW_RETURN);
	
	total = mib_chain_total(MIB_CONN_LIMIT_TBL);
	for (i=0; i<total; i++) {
		if (!mib_chain_get(MIB_CONN_LIMIT_TBL, i, (void *)&Entry))
			continue;
				
		// Check if this entry is enabled
		if ( Entry.Enabled == 1 ) { 		
			// inet_ntoa is not reentrant, we have to
			// copy the static memory before reuse it
			strncpy(ssrc, inet_ntoa(*((struct in_addr *)&Entry.ipAddr)), 16);
			ssrc[15] = '\0';

			snprintf(connNum, 10, "%d", Entry.connNum);
			
			// iptables -A connlimit -i br0 -s xxx.xxx.xxx.xxx 	
			va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, "connlimit", "-s", ssrc, "-m", "iplimit", 
				"--iplimit-above", connNum, "--iplimit-mask", "255.255.255.255", "-j", "REJECT");
		}
	}
}
#endif
#ifdef TCP_UDP_CONN_LIMIT
void set_conn_limit()
{
	int 					i, total;
	char 				ssrc[20];
	char 				connNum[10];
	MIB_CE_TCP_UDP_CONN_LIMIT_T Entry;
	
	// Add policy to connlimit chain, allow all established ~
	//iptables -A connlimit -m state --state RELATED,ESTABLISHED -j RETURN
	va_cmd(IPTABLES, 8, 1, (char *)FW_ADD, "connlimit", "-m", "state",
		"--state", "ESTABLISHED,RELATED", "-j", (char *)FW_RETURN);
	
	total = mib_chain_total(MIB_TCP_UDP_CONN_LIMIT_TBL);
	for (i=0; i<total; i++) {
		if (!mib_chain_get(MIB_TCP_UDP_CONN_LIMIT_TBL, i, (void *)&Entry))
			continue;				
		// Check if this entry is enabled and protocol is TCP
		if (( Entry.Enabled == 1 ) && ( Entry.protocol == 0 )) { 		
			// inet_ntoa is not reentrant, we have to
			// copy the static memory before reuse it
			strncpy(ssrc, inet_ntoa(*((struct in_addr *)&Entry.ipAddr)), 16);
			ssrc[15] = '\0';
			snprintf(connNum, 10, "%d", Entry.connNum);
			
			// iptables -A connlimit -p tcp -s 192.168.1.99 -m iplimit --iplimit-above 2 -j REJECT
			va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, "connlimit","-p","tcp", "-s", ssrc, "-m", "iplimit", 
				"--iplimit-above", connNum, "-j", "REJECT");			
		}
		// UDP
		else if  (( Entry.Enabled == 1 ) && ( Entry.protocol == 1 )) { 		
			strncpy(ssrc, inet_ntoa(*((struct in_addr *)&Entry.ipAddr)), 16);
			ssrc[15] = '\0';
			snprintf(connNum, 10, "%d", Entry.connNum);
			
			//iptables -A connlimit -p udp -s 192.168.1.99 -m udplimit --udplimit-above 2 -j REJECT
			va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, "connlimit","-p","udp", "-s", ssrc, "-m", "udplimit", 
				"--udplimit-above", connNum, "-j", "REJECT");			
		}
			
	}

	//The Global rules goes last ~
	//TCP 
	mib_get(MIB_CONNLIMIT_TCP, (void *)&i);
	if (i >0)
	{
		snprintf(connNum, 10, "%d", i);
		va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, "connlimit","-p","tcp",  "-m", "iplimit", 
					"--iplimit-above", connNum, "-j", "REJECT");	
	}
	//UDP
	mib_get(MIB_CONNLIMIT_UDP, (void *)&i);
	if (i >0)
	{
		snprintf(connNum, 10, "%d", i);
		va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, "connlimit","-p","udp",  "-m", "udplimit", 
					"--udplimit-above", connNum, "-j", "REJECT");	
	}

}
#endif//TCP_UDP_CONN_LIMIT

#ifdef URL_BLOCKING_SUPPORT
// Added by Mason Yu for URL Blocking
void filter_set_url(int enable)
{

	int i, j, total, totalKeywd;
	MIB_CE_URL_FQDN_T Entry;
	MIB_CE_KEYWD_FILTER_T entry;	

	// check if URL Capability is enabled ?

	if (!enable)
	{
		va_cmd(IPTABLES, 2, 1, "-F", "urlblock");
		brnfctrl(0,BRNF_MODULE_URLBLOCK);
	}
	else {

		// Add URL policy to urlblock chain
		total = mib_chain_total(MIB_URL_FQDN_TBL);

		for (i=0; i<total; i++) {

			char testURL[MAX_URL_LENGTH+2];
			if (!mib_chain_get(MIB_URL_FQDN_TBL, i, (void *)&Entry))

			{ 
				return;
			}					 		

			strcpy(testURL, Entry.fqdn);
			strcat(testURL, "\r\n");
		 	// iptables -A urlblock -p tcp --dport 80 -m string --url "tw.yahoo.com" -j DROP
			//va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, "urlblock", "-p", "tcp", "--dport", "80", "-m", "string", "--url", Entry.fqdn, "-j", (char *)FW_DROP);
			va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, "urlblock", "-p", "tcp", "--dport", "80", "-m", "string", "--url", testURL, "-j", (char *)FW_DROP);

		}
		
		// Add Keyword filtering policy to urlblock chain	
		totalKeywd = mib_chain_total(MIB_KEYWD_FILTER_TBL);

		for (i=0; i<totalKeywd; i++) {

			if (!mib_chain_get(MIB_KEYWD_FILTER_TBL, i, (void *)&entry))

			{ 
				return;
			}					 		

		 	// iptables -A urlblock -p tcp --dport 80 -m string --url "tw.yahoo.com" -j DROP
			va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, "urlblock", "-p", "tcp", "--dport", "80", "-m", "string", "--url", entry.keyword, "-j", (char *)FW_DROP);

		}
		if ((total > 0) || (totalKeywd > 0))
			brnfctrl(1,BRNF_MODULE_URLBLOCK);
		else 
			brnfctrl(0,BRNF_MODULE_URLBLOCK);
	}

}
#ifdef URL_ALLOWING_SUPPORT

void set_url(int enable){

	int i, j, total, totalKeywd;
	MIB_CE_URL_FQDN_T Entry;

	// check if URL Capability is enabled ?

	if (!enable)

		va_cmd(IPTABLES, 2, 1, "-F", "urlallow");

	else {

		// Add URL policy to urlblock chain
		total = mib_chain_total(MIB_URL_ALLOW_FQDN_TBL);

		for (i=0; i<total; i++) {

			if (!mib_chain_get(MIB_URL_ALLOW_FQDN_TBL, i, (void *)&Entry))

			{ 
				return;
			}					 		

		 	// iptables -A urlallow -p tcp --dport 80 -m string --url "tw.yahoo.com" -j ACCEPT
			va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, "urlallow", "-p", "tcp", "--dport", "80", "-m", "string", "--urlalw", Entry.fqdn, "-j", (char *)FW_ACCEPT);

		}
	//&endofurl& is flag for drop other urls	
              va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, "urlallow", "-p", "tcp", "--dport", "80", "-m", "string", "--urlalw", "&endofurl&" ,"-j", (char *)FW_DROP);
		
	}
 
}
#endif
#endif

#ifdef DOMAIN_BLOCKING_SUPPORT
// Added by Mason Yu for Domain Blocking
void filter_set_domain(int enable)
{
	int i, total;	
	MIB_CE_DOMAIN_BLOCKING_T Entry;
	unsigned char sdest[MAX_DOMAIN_LENGTH];
	int j, k;
    	unsigned char *needle_tmp, *str;
    	char len[MAX_DOMAIN_GROUP];
    	unsigned char seg[MAX_DOMAIN_GROUP][MAX_DOMAIN_SUB_STRING];    	
    	unsigned char cmpStr[MAX_DOMAIN_LENGTH]="\0";
	
	// check if Domain Blocking Capability is enabled ?
	if (!enable)
		va_cmd(IPTABLES, 2, 1, "-F", "domainblk");
	else {
		// Add policy to domainblk chain
		total = mib_chain_total(MIB_DOMAIN_BLOCKING_TBL);
		for (i=0; i<total; i++) {
			if (!mib_chain_get(MIB_DOMAIN_BLOCKING_TBL, i, (void *)&Entry))
			{  				
				return;
			}			
			
			// Mason Yu
			// calculate domain sub string lengh and form the compare sub string.
			// Foe example, If the domain is aa.bbb.cccc, the compare sub string 1 is 0x02 0x61 0x61.
			// The compare sub string 2 is 0x03 0x62 0x62 0x62. The compare sub string 3 is 0x04 0x63 0x63 0x63 0x63.
			needle_tmp = Entry.domain;
			
			for (j=0; (str =strchr(needle_tmp, '.'))!= NULL; j++)
    			{    				
    				*str = '\0';
    					
    				strncpy(seg[j]+1, needle_tmp, (MAX_DOMAIN_SUB_STRING - 1));    				
    				seg[j][MAX_DOMAIN_SUB_STRING - 1]='\0';
    				//printf(" seg[%d]= %s...(1)\n", j, seg[j]);
    				
    				//seg[j][0]= len[j];
    				seg[j][0] = strlen(needle_tmp);
    				//printf(" seg[%d]= %s...(2)\n", j, seg[j]);
    	   	
    				needle_tmp = str+1;
   			}	
    			
    			// calculate the laster domain sub string lengh and form the laster compare sub string    					   			
    			strncpy(seg[j]+1, needle_tmp, (MAX_DOMAIN_SUB_STRING - 1));
    			seg[j][MAX_DOMAIN_SUB_STRING - 1]='\0';
    			
    			seg[j][0]= strlen(needle_tmp);    			
    			//printf(" seg[%d]= %s...(3)\n", j, seg[j]);    
    			
    			// Merge the all compare sub string into a final compare string.
    			for ( k=0; k<=j; k++) {    				  
    				//printf(" seg[%d]= %s", k, seg[k]);
    				strcat(cmpStr, seg[k]);
    				//printf(" cmpStr=%s\n", cmpStr);
    			}
    			//printf("\n");
		 		
		 	// iptables -A domainblk -p udp --dport 53 -m string --domain yahoo.com -j DROP	
			va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, "domainblk", "-p", "udp", "--dport", "53", "-m", "string", "--domain", cmpStr, "-j", (char *)FW_DROP);
			cmpStr[0] = '\0';			
		}		
		
	}
}

int restart_domainBLK()
{		
	unsigned char domainEnable;	

	va_cmd(IPTABLES, 2, 1, "-F", "domainblk");		
	mib_get(MIB_DOMAINBLK_CAPABILITY, (void *)&domainEnable);
	if (domainEnable == 1)  // domain blocking Capability enabled
		filter_set_domain(1);
	else
		filter_set_domain(0);	
	return 0;
}

#endif

#ifdef LAYER7_FILTER_SUPPORT
//star: the name is get from *.pat files of the folder user/iptables/l7-protocols
#define APPNUM  20
#define WEB_NAME_FLAG  1
#define FILTER_NAME_FLAG  0
typedef struct app_name_entry{
	unsigned char * filtername;
	unsigned char * webname;
}app_name_str;

app_name_str apply_name[APPNUM]={
{"qq","QQ及QQ游戏"},
{"msnmessenger","MSN messenger"},
{"yahoo","雅虎通"},
{"bittorrent","BT类软件"},
{"ourgame","联众游戏"},
{"gameabc","边锋网络游戏"},
{"chinagame","中国游戏中心"},
{"haofang","浩方对战平台"},
{"smtp","发送邮件"},
{"pop3","接收邮件"},
{"unknown","unknown"},
NULL
};
void getappname(int appnum, unsigned char * appname, int flag)
{
	int num=0,i;

	for(i=0;i<APPNUM;i++)
	{
		if(!strcmp(apply_name[i].filtername,"unknown"))
			break;
		num++;	
	}

	if(flag == FILTER_NAME_FLAG){
		if((appnum <0) || (appnum >= num))
			snprintf(appname, MAX_APP_NAME, "%s", "unknown");
		else
			snprintf(appname, MAX_APP_NAME, "%s", apply_name[appnum].filtername);
	}

	if(flag == WEB_NAME_FLAG){
		for(i=0;i<num;i++){
			if(!strcmp(appname,apply_name[i].filtername)){
				snprintf(appname, 2*MAX_APP_NAME, "%s", apply_name[i].webname);
				break;
			}
		}
	}
		
	return;
		
}

#endif

#if defined(CONFIG_USER_ROUTED_ROUTED) || defined(CONFIG_USER_ZEBRA_OSPFD_OSPFD)
static const char ROUTED_CONF[] = "/var/run/routed.conf";

int startRip()
{
	FILE *fp;
	unsigned char ripOn, ripInf, ripVer;	
	int rip_pid;
	
	unsigned int entryNum, i;
	MIB_CE_RIP_T Entry;
	char ifname[6], receive_mode[5], send_mode[5];
	
	if (mib_get(MIB_RIP_ENABLE, (void *)&ripOn) == 0)
		ripOn = 0;	
	rip_pid = read_pid((char *)ROUTED_PID);
	if (rip_pid >= 1) {
		// kill it
		if (kill(rip_pid, SIGTERM) != 0) {
			printf("Could not kill pid '%d'", rip_pid);
		}
	}
	
	if (!ripOn)
		return 0;

	printf("start rip!\n");
	if ((fp = fopen(ROUTED_CONF, "w")) == NULL)
	{
		printf("Open file %s failed !\n", ROUTED_CONF);
		return -1;
	}	
	
#if 0	
	if (mib_get(MIB_RIP_VERSION, (void *)&ripVer) == 0)
		ripVer = 1;	// default version 2
	
	fprintf(fp, "version %u\n", ripVer+1);
	
	// LAN interface
	if (mib_get(MIB_ADSL_LAN_RIP, (void *)&ripOn) != 0)
	{
		if (ripOn)
			fprintf(fp, "network br0\n");
	}
	
	// WAN interface
	vcTotal = mib_chain_total(MIB_ATM_VC_TBL);
	
	for (i=0; i<vcTotal; i++)
	{
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			return -1;
		
		if (Entry.enable == 0)
			continue;
		
		if (Entry.cmode != ADSL_BR1483 && Entry.rip)
		{
			if (PPP_INDEX(Entry.ifIndex) != 0x0f)
			{	// PPP interface
				snprintf(ifname, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));
			}
			else
			{	// vc interface
				snprintf(ifname, 6, "vc%u", VC_INDEX(Entry.ifIndex));
			}
			
			fprintf(fp, "network %s\n", ifname);
		}
	}
#endif

	entryNum = mib_chain_total(MIB_RIP_TBL);
	
	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_RIP_TBL, i, (void *)&Entry))
		{
  			printf("Get MIB_RIP_TBL chain record error!\n");
			return;
		}		
		
		if( Entry.ifIndex == 0xff) {
			strncpy(ifname, "br0", strlen("br0"));
			ifname[strlen("br0")] = '\0';
		} else {	
			if (PPP_INDEX(Entry.ifIndex) != 0x0f)
			{	// PPP interface
				snprintf(ifname, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));
			}
			else
			{	// vc interface
				snprintf(ifname, 6, "vc%u", VC_INDEX(Entry.ifIndex));
			}
		}
				
		fprintf(fp, "network %s %d %d\n", ifname, Entry.receiveMode, Entry.sendMode);
	}
	
	fclose(fp);
	// Modified by Mason Yu for always as a supplier for RIP
	//va_cmd(ROUTED, 0, 0);
	va_cmd(ROUTED, 1, 0, "-s");
	return 1;
}
#endif	// of CONFIG_USER_ROUTED_ROUTED

#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
static const char ZEBRA_CONF[] = "/etc/config/zebra.conf";
static const char OSPFD_CONF[] = "/etc/config/ospfd.conf";

// OSPF server configuration
// return value:
// 0  : not enabled
// 1  : successful
// -1 : startup failed
int startOspf()
{
	FILE *fp;
	char *argv[6];
	int pid;
	unsigned char ospfOn;
	unsigned int entryNum, i, j;
	MIB_CE_OSPF_T Entry;
	char *netIp[20];
	unsigned int uMask;
	unsigned int uIp;

	if (mib_get(MIB_OSPF_ENABLE, (void *)&ospfOn) == 0)
		ospfOn = 0;
	//kill old zebra
	pid = read_pid((char *)ZEBRA_PID);
	if (pid >= 1) {
		if (kill(pid, SIGTERM) != 0) {
			printf("Could not kill pid '%d'\n", pid);
		}
	}
	//kill old ospfd
	pid = read_pid((char *)OSPFD_PID);
	if (pid >= 1) {
		if (kill(pid, SIGTERM) != 0)
			printf("Could not kill pid '%d'\n", pid);
	}

	if (!ospfOn)
		return 0;

	printf("start ospf!\n");
	//create zebra.conf
	if ((fp = fopen(ZEBRA_CONF, "w")) == NULL)
	{
		printf("Open file %s failed !\n", ZEBRA_CONF);
		return -1;
	}
	fprintf(fp, "hostname Router\n");
	fprintf(fp, "password zebra\n");
	fprintf(fp, "enable password zebra\n");
	fclose(fp);

	//create ospfd.conf
	if ((fp = fopen(OSPFD_CONF, "w")) == NULL)
	{
		printf("Open file %s failed !\n", OSPFD_CONF);
		return -1;
	}
	fprintf(fp, "hostname ospfd\n");
	fprintf(fp, "password zebra\n");
	fprintf(fp, "enable password zebra\n");
	fprintf(fp, "router ospf\n");
	//ql_xu test.
	//fprintf(fp, "network %s area 0\n", "192.168.2.0/24");
	entryNum = mib_chain_total(MIB_OSPF_TBL);
	
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_OSPF_TBL, i, (void *)&Entry))
		{
  			printf("Get MIB_OSPF_TBL chain record error!\n");
			return -1;
		}

		uIp = *(unsigned int *)Entry.ipAddr;
		uMask = *(unsigned int *)Entry.netMask;
		uIp = uIp & uMask;
		strcpy(netIp, inet_ntoa(*((struct in_addr *)&uIp)));
		
		for (j=0; j<32; j++)
			if ((uMask>>j) & 0x01)
				break;
		uMask = 32 - j;

		sprintf(netIp, "%s/%d", netIp, uMask);
		fprintf(fp, "network %s area 0\n", netIp);
	}
	fclose(fp);

	//start zebra
	argv[1] = "-d";
	argv[2] = "-k";
	argv[3] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s\n", ZEBRA, argv[1], argv[2]);
	do_cmd(ZEBRA, argv, 0);

	//start ospfd
	argv[1] = "-d";
	argv[2] = NULL;
	TRACE(STA_SCRIPT, "%s %s\n", OSPFD, argv[1]);
	do_cmd(OSPFD, argv, 0);
	
	return 1;
}
#endif

#ifdef WLAN_SUPPORT
//check flash config if wlan0 is up, called by BOA, 
//0:down, 1:up
int wlan_is_up(void)
{
  unsigned char vChar;
  int value=0;

    mib_get( MIB_WLAN_DISABLED, (void *)&vChar);
    if (vChar==0) {  // wlan0 is enabled
    	value=1;
    }
	return value;
}
#endif

#include <linux/atm.h>

//up_flag:0, itf down; non-zero: itf up
void itfcfg(char *if_name, int up_flag)
{
#ifdef EMBED
  int fd;

	if (strncmp(if_name, "sar", sizeof("sar"))==0) {  //sar enable/disable
		struct atmif_sioc mysio;
		
		if((fd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0){
			printf("socket open error\n");
			return;
		}
		mysio.number = 0;
		mysio.length = sizeof(struct SAR_IOCTL_CFG);
		mysio.arg = (void *)NULL;
		if (up_flag) ioctl(fd, SIOCDEVPRIVATE+2, &mysio);
		else         ioctl(fd, SIOCDEVPRIVATE+3, &mysio);
		if(fd!=(int)NULL)
		    close(fd);
	} else if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0) {
		int flags;
		if (getInFlags(if_name, &flags) == 1) {
			if (up_flag)
				flags |= IFF_UP;
			else
				flags &= ~IFF_UP;
			
			setInFlags(if_name, flags);
		}
		close(fd);
	}
#endif
}

#ifdef WLAN_SUPPORT
//disabled: 0--WLAN is enabled, nonZero--WLAN is disabled
int update_wlan_disable(unsigned char disabled)
{
  unsigned char vChar=disabled;
  unsigned char vCharget;
  int value=0;

	// Mason Yu 
	mib_get( MIB_WLAN_DISABLED, (void *)&vCharget);
	if ( vCharget != vChar) {
		//printf("Setting WLan interface up or down\n");
		mib_set( MIB_WLAN_DISABLED, (void *)&vChar);
		#if 0
		itfcfg("eth0", 0);
		itfcfg("sar", 0);
		itfcfg((char *)WLANIF, 0);
		
		if(mib_update(CURRENT_SETTING, CONFIG_MIB_ALL) == 0)
			printf("CS Flash error! \n");
		
		itfcfg("eth0", 1);
		itfcfg("sar", 1);
		if (!disabled) itfcfg((char *)WLANIF, 1);
		#endif
#ifdef CONFIG_WIFI_SIMPLE_CONFIG//WPS def WIFI_SIMPLE_CONFIG
//xl_yue: if disable wlan, wps should be disabled too
		mib_set(MIB_WSC_DISABLE, (void *)&vChar);
#ifdef REVERSE_WPS_LED
		if(vChar){
			system("echo 0 > /proc/gpio");
		}else{
			system("echo 1 > /proc/gpio");		}
#endif
#endif
	}
	return value;
}
#endif

#ifdef XOR_ENCRYPT
const char XOR_KEY[] = "tecomtec";
void xor_encrypt(char *inputfile, char *outputfile)
{
	FILE *input  = fopen(inputfile, "rb");
	FILE *output = fopen(outputfile, "wb");

	if(input != NULL && output != NULL) {
		unsigned char buffer[MAX_CONFIG_FILESIZE];
		size_t count, i, j = 0;
		do {
			count = fread(buffer, sizeof *buffer, sizeof buffer, input);
			for(i = 0; i<count; ++i) {
				buffer[i] ^= XOR_KEY[j++];
				if(XOR_KEY[j] == '\0')
					j = 0; /* restart at the beginning of the key */
			}
			fwrite(buffer, sizeof *buffer, count, output);
		} while (count == sizeof buffer);
		fclose(input);
		fclose(output);
	}
}
#endif

//#define uint16 unsigned short
unsigned short 
ipchksum(unsigned char *ptr, int count, unsigned short resid)
{
	register unsigned int sum = resid;
       if ( count==0) 
       	return(sum);
        
	while(count > 1) {
		//sum += ntohs(*ptr);	
		sum += (( ptr[0] << 8) | ptr[1] );
		if ( sum>>31)
			sum = (sum&0xffff) + ((sum>>16)&0xffff);
		ptr += 2;
		count -= 2;
	}

	if (count > 0) 
		sum += (*((unsigned char*)ptr) << 8) & 0xff00;

	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	if (sum == 0xffff) 
		sum = 0;
	return (unsigned short)sum;
}

int getLinkStatus(struct ifreq *ifr)
{
	int status=0;

#ifdef __mips__
	if (do_ioctl(SIOCGMEDIALS, ifr) == 1)
		status = 1;
#endif
	return status;
}


/***port forwarding APIs*******/
/*move from startup.c:iptable_fw()*/
void iptable_fw( int del, int negate, const char *ifname, const char *proto, const char *remoteIP, const char *extPort, const char *dstIP)
{
	char *act;
	
	if(del) act = (char *)FW_DEL;
	else act = (char *)FW_ADD;


//	if (negate && remoteIP) {
	if (negate && remoteIP && extPort) {
		va_cmd(IPTABLES, 17, 1, "-t", "nat",
			(char *)act,	(char *)PORT_FW,
			(char *)ARG_I, "!", (char *)ifname,
			"-p", (char *)proto,
			"-s", (char *)remoteIP,
			(char *)FW_DPORT, extPort, "-j",
			"DNAT", "--to-destination", dstIP);
	} else if (negate && remoteIP) {
		va_cmd(IPTABLES, 15, 1, "-t", "nat",
			(char *)act,	(char *)PORT_FW,
			(char *)ARG_I, "!", (char *)ifname,
			"-p", (char *)proto,
			"-s", (char *)remoteIP,
			"-j", "DNAT", "--to-destination", dstIP);
//	} else if (negate) {
	} else if (negate && extPort) {
		va_cmd(IPTABLES, 15, 1, "-t", "nat",
			(char *)act,	(char *)PORT_FW,
			(char *)ARG_I, "!", (char *)ifname,
			"-p", (char *)proto,
			(char *)FW_DPORT, extPort, "-j",
			"DNAT", "--to-destination", dstIP);
//	} else if (remoteIP) {
	} else if (remoteIP && extPort) {
		va_cmd(IPTABLES, 16, 1, "-t", "nat",
			(char *)act,	(char *)PORT_FW,
			(char *)ARG_I, (char *)ifname,
			"-p", (char *)proto,
			"-s", (char *)remoteIP,
			(char *)FW_DPORT, extPort, "-j",
			"DNAT", "--to-destination", dstIP);
	} else if (negate) {
		va_cmd(IPTABLES, 13, 1, "-t", "nat",
			(char *)act,	(char *)PORT_FW,
			(char *)ARG_I, "!", (char *)ifname,
			"-p", (char *)proto,
			"-j", "DNAT", "--to-destination", dstIP);
	} else if (remoteIP) {
		va_cmd(IPTABLES, 14, 1, "-t", "nat",
			(char *)act,	(char *)PORT_FW,
			(char *)ARG_I, (char *)ifname,
			"-p", (char *)proto,
			"-s", (char *)remoteIP,
			"-j", "DNAT", "--to-destination", dstIP);
//	} else {
	} else if (extPort) {
		va_cmd(IPTABLES, 14, 1, "-t", "nat",
			(char *)act,	(char *)PORT_FW,
			(char *)ARG_I, (char *)ifname,
			"-p", (char *)proto,
			(char *)FW_DPORT, extPort, "-j",
			"DNAT", "--to-destination", dstIP);
	} else {
		va_cmd(IPTABLES, 12, 1, "-t", "nat",
			(char *)act,	(char *)PORT_FW,
			(char *)ARG_I, (char *)ifname,
			"-p", (char *)proto,
			 "-j", "DNAT", "--to-destination", dstIP);
	}

}

/*move from startup.c:iptable_filter()*/
void iptable_filter( int del, int negate, const char *ifname, const char *proto, const char *remoteIP, const char *intPort)
{
	char *strInsert="-I";
	char *strThird="3";
	char *act, *rulenum;
	
	if(del)
	{ 
		act = (char *)FW_DEL;

//		if (negate && remoteIP) {
		if (negate && remoteIP && intPort) {			
			va_cmd(IPTABLES, 15, 1, act,
				(char *)PORT_FW, (char *)ARG_I, "!", ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,  
				"-s", remoteIP,
				(char *)FW_DPORT, intPort, 
				"-j",(char *)FW_ACCEPT);
		} else if (negate && remoteIP) {
			va_cmd(IPTABLES, 13, 1, act,
				(char *)PORT_FW, (char *)ARG_I, "!", ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,  
				"-s", remoteIP,
				"-j",(char *)FW_ACCEPT);
//		} else if (negate) {
		} else if (negate && intPort) {
			va_cmd(IPTABLES, 13, 1, act,
				(char *)PORT_FW, (char *)ARG_I, "!", ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,  
				(char *)FW_DPORT, intPort, 
				"-j",(char *)FW_ACCEPT);
//		} else if (remoteIP) {
		} else if (remoteIP && intPort) {
			va_cmd(IPTABLES, 14, 1, act,
				(char *)PORT_FW, (char *)ARG_I, ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,  
				"-s", remoteIP,
				(char *)FW_DPORT, intPort, 
				"-j",(char *)FW_ACCEPT);
		} else if (negate) {
			va_cmd(IPTABLES, 11, 1, act,
				(char *)PORT_FW, (char *)ARG_I, "!", ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,  
				"-j",(char *)FW_ACCEPT);
		} else if (remoteIP) {
			va_cmd(IPTABLES, 12, 1, act,
				(char *)PORT_FW, (char *)ARG_I, ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,  
				"-s", remoteIP,
				"-j",(char *)FW_ACCEPT);
//		} else {
		} else if (intPort) {
			va_cmd(IPTABLES, 12, 1, act,
				(char *)PORT_FW, (char *)ARG_I, ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,  
				(char *)FW_DPORT, intPort, "-j",
				(char *)FW_ACCEPT);
		} else {
			va_cmd(IPTABLES, 10, 1, act,
				(char *)PORT_FW, (char *)ARG_I, ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,  
				"-j", (char *)FW_ACCEPT);
		}
	}else
	{
		//act = strInsert; //use insert, not append => need rulenum
		//rulenum = strThird;
 		act = (char *)FW_ADD;
 		
//		if (negate && remoteIP) {
		if (negate && remoteIP && intPort) {
			va_cmd(IPTABLES, 15, 1, act,
				(char *)PORT_FW, (char *)ARG_I, "!", ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,  
				"-s", remoteIP,
				(char *)FW_DPORT, intPort, 
				"-j",(char *)FW_ACCEPT);			
		} else if (negate && remoteIP) {
			va_cmd(IPTABLES, 13, 1, act,
				(char *)PORT_FW, (char *)ARG_I, "!", ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,  
				"-s", remoteIP,
				"-j",(char *)FW_ACCEPT);
//		} else if (negate) {
		} else if (negate && intPort) {
			va_cmd(IPTABLES, 13, 1, act,
				(char *)PORT_FW, (char *)ARG_I, "!", ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,  
				(char *)FW_DPORT, intPort, 
				"-j",(char *)FW_ACCEPT);
//		} else if (remoteIP) {
		} else if (remoteIP && intPort) {
			va_cmd(IPTABLES, 14, 1, act,
				(char *)PORT_FW, (char *)ARG_I, ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,  
				"-s", remoteIP,
				(char *)FW_DPORT, intPort, 
				"-j",(char *)FW_ACCEPT);
		} else if (negate) {
			va_cmd(IPTABLES, 11, 1, act,
				(char *)PORT_FW, (char *)ARG_I, "!", ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,  
				"-j",(char *)FW_ACCEPT);
		} else if (remoteIP) {
			va_cmd(IPTABLES, 12, 1, act,
				(char *)PORT_FW, (char *)ARG_I, ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,  
				"-s", remoteIP,
				"-j",(char *)FW_ACCEPT);
//		} else {
		} else if (intPort) {
			va_cmd(IPTABLES, 12, 1, act,
				(char *)PORT_FW, (char *)ARG_I, ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,  
				(char *)FW_DPORT, intPort, "-j",
				(char *)FW_ACCEPT);
		} else {
			va_cmd(IPTABLES, 10, 1, act,
				(char *)PORT_FW, (char *)ARG_I, ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,  
				"-j", (char *)FW_ACCEPT);
		}
	}
}

/*move from startup.c ==> part of setupFirewall()*/
void portfw_modify( MIB_CE_PORT_FW_T *p, int del )
{
	int negate=0, hasRemote=0, hasLocalPort=0, hasExtPort=0;
	char * proto = 0;
	char intPort[32], extPort[32];
	char ipaddr[32], extra[32], ifname[16];

	if(p==NULL) return;

#if 0
	{
		fprintf( stderr,"<portfw_modify>\n" );
		fprintf( stderr,"\taction:%s\n", (del==0)?"ADD":"DEL" );
		fprintf( stderr,"\tifIndex:0x%x\n", p->ifIndex );
		fprintf( stderr,"\tenable:%u\n", p->enable );
		fprintf( stderr,"\tleaseduration:%u\n", p->leaseduration );
		fprintf( stderr,"\tremotehost:%s\n", inet_ntoa(*((struct in_addr *)p->remotehost))  );
		fprintf( stderr,"\texternalport:%u\n", p->externalport );
		fprintf( stderr,"\tinternalclient:%s\n", inet_ntoa(*((struct in_addr *)p->ipAddr)) );
		fprintf( stderr,"\tinternalport:%u\n", p->toPort );
		fprintf( stderr,"\tprotocol:%u\n", p->protoType ); /*PROTO_TCP=1, PROTO_UDP=2*/
		fprintf( stderr,"<end portfw_modify>\n" );
	}
#endif

	if( del==0 ) //add
	{
		char vCh=0;
		mib_get(MIB_PORT_FW_ENABLE, (void *)&vCh);
		if(vCh==0) return;
		
		if (!p->enable) return;
	}
	
/*	snprintf(intPort, 12, "%u", p->fromPort);

	if (p->externalport) {
		snprintf(extPort, sizeof(extPort), "%u", p->externalport);
		snprintf(intPort, sizeof(intPort), "%u", p->fromPort);
		snprintf(ipaddr, sizeof(ipaddr), "%s:%s", inet_ntoa(*((struct in_addr *)p->ipAddr)), intPort);
	} else {
		snprintf(intPort, sizeof(extPort), "%u", p->fromPort);				
		strncpy(extPort, intPort, sizeof(intPort));				
		strncpy(ipaddr, inet_ntoa(*((struct in_addr *)p->ipAddr)), 16);
	}
	*/
    if (p->fromPort) 
    {
        if (p->fromPort == p->toPort)
        {
            snprintf(intPort, sizeof(intPort), "%u", p->fromPort);
        }
        else
        {
            /* "%u-%u" is used by port forwarding */
            snprintf(intPort, sizeof(intPort), "%u-%u", p->fromPort, p->toPort);
        }

        snprintf(ipaddr, sizeof(ipaddr), "%s:%s", inet_ntoa(*((struct in_addr *)p->ipAddr)), intPort);
        
        if (p->fromPort != p->toPort)
        {
            /* "%u:%u" is used by filter */
            snprintf(intPort, sizeof(intPort), "%u:%u", p->fromPort, p->toPort);
        }
        hasLocalPort = 1;
    } 
    else
    {
        snprintf(ipaddr, sizeof(ipaddr), "%s", inet_ntoa(*((struct in_addr *)p->ipAddr)));
        hasLocalPort = 0;
    }

	if (p->externalfromport && p->externaltoport && (p->externalfromport != p->externaltoport)) {
		snprintf(extPort, sizeof(extPort), "%u:%u", p->externalfromport, p->externaltoport);
		hasExtPort = 1;
	} else if (p->externalfromport) {
		snprintf(extPort, sizeof(extPort), "%u", p->externalfromport);
		hasExtPort = 1;
	} else if (p->externaltoport) {
		snprintf(extPort, sizeof(extPort), "%u", p->externaltoport);
		hasExtPort = 1;
	} else {
		hasExtPort = 0;
	}
	//printf( "extPort:%s hasExtPort=%d\n",  extPort, hasExtPort);
	//printf( "entry.externalfromport:%d entry.externaltoport=%d\n",  p->externalfromport, p->externaltoport);
		
	if( p->ifIndex!=0xff ) {
		if( (p->ifIndex & 0xf0)==0xf0 ) //vc
			snprintf( ifname, sizeof(ifname), "vc%u", VC_INDEX(p->ifIndex)  );
		else
			snprintf( ifname, sizeof(ifname), "ppp%u", PPP_INDEX(p->ifIndex)  );				
		negate = 0;
	} else {
	
		strcpy(ifname, LANIF);
		negate = 1;
	}

	if (p->remotehost[0]) {
		snprintf(extra, sizeof(extra), "%s", inet_ntoa(*((struct in_addr *)p->remotehost)));
		hasRemote = 1;
	} else {
		hasRemote = 0;
	}
	
	//fprintf( stderr, "ipaddr:%s, intPort:%s\n", ipaddr, intPort );
	//check something
	//internalclient can't be zeroip
	if( strncmp(ipaddr,"0.0.0.0", 7)==0 ) return;
	//internalport can't be zero
//	if( strcmp(intPort,"0")==0 ) return;
	//fprintf( stderr, "Pass ipaddr:%s, intPort:%s\n", ipaddr, intPort );

	if (p->protoType == PROTO_TCP || p->protoType == PROTO_UDPTCP)
	{
		// iptables -t nat -A PREROUTING -i ! $LAN_IF -p TCP --dport dstPortRange -j DNAT --to-destination ipaddr
//		iptable_fw( del, negate, ifname, ARG_TCP, hasRemote ? extra : 0, extPort, ipaddr);
		iptable_fw( del, negate, ifname, ARG_TCP, hasRemote ? extra : 0, hasExtPort ? extPort : 0, ipaddr);
		// iptables -I ipfilter 3 -i ! $LAN_IF -o $LAN_IF -p TCP --dport dstPortRange -j RETURN
//		iptable_filter( del, negate, ifname, ARG_TCP, hasRemote ? extra : 0, intPort);
		iptable_filter( del, negate, ifname, ARG_TCP, hasRemote ? extra : 0, hasLocalPort ? intPort : 0);
	}
	
	if (p->protoType == PROTO_UDP || p->protoType == PROTO_UDPTCP)
	{				
//		iptable_fw( del, negate, ifname, ARG_UDP, hasRemote ? extra : 0, extPort, ipaddr);
		iptable_fw( del, negate, ifname, ARG_UDP, hasRemote ? extra : 0, hasExtPort ? extPort : 0, ipaddr);
		// iptables -I ipfilter 3 -i ! $LAN_IF -o $LAN_IF -p UDP --dport dstPortRange -j RETURN
//		iptable_filter( del, negate, ifname, ARG_UDP, hasRemote ? extra : 0, intPort);
		iptable_filter( del, negate, ifname, ARG_UDP, hasRemote ? extra : 0, hasLocalPort ? intPort : 0);
	}
}
/***end port forwarding APIs*******/

#if defined(TIME_ZONE) || defined(ZTE_GENERAL_ROUTER_SC)
// return value:
// 1  : successful
// -1 : failed
int startNTP(void)
{
	int status=1;
	char vChar;
	struct in_addr ipAddr;
	char sVal[32], sTZ[16], *pStr;
	char sVal2[32]={0};
	FILE *fp;
	
	mib_get( MIB_NTP_ENABLED, (void *)&vChar);
	if (vChar == 1) 
	{
		// start vsntp and setup timezone
		mib_get(MIB_NTP_TIMEZONE, (void *)sVal);
		pStr = strstr(sVal, " ");
		if (pStr != NULL)
			pStr[0] = '\0';
		snprintf(sTZ, 16, "GMT%s", sVal);
		//setenv("TZ", sTZ, 1);
		if ((fp = fopen("/etc/TZ", "w")) != NULL) {
			fprintf(fp, "%s\n", sTZ);
			fclose(fp);
		}
		
		mib_get( MIB_NTP_SERVER_ID, (void *)&vChar);
		if (vChar == 0) {
			mib_get(MIB_NTP_SERVER_IP1, (void *)&ipAddr);
			strncpy(sVal, inet_ntoa(ipAddr), 16);
			sVal[15] = '\0';
		}
		else
		{
			mib_get(MIB_NTP_SERVER_HOST2, (void *)sVal);
			mib_get(MIB_NTP_SERVER_HOST1, (void *)sVal2);
		}
/*ping_zhang:20081254 START:add to support sntp multi server*/
#ifdef SNTP_MULTI_SERVER
		if(sVal2[0])
			status|=va_cmd(SNTPC, 4, 0, "-s", sVal, "-s", sVal2);
		else
			status|=va_cmd(SNTPC, 2, 0, "-s",sVal);
#else
		status|=va_cmd(SNTPC,1,0,sVal);
#endif
/*ping_zhang:20081254 END*/
	}	
	return status;
}

// return value:
// 1  : successful
// -1 : failed
int stopNTP(void)
{
	int ntp_pid=0;
	int status=1;
	
	ntp_pid = read_pid((char *)SNTPC_PID);
	if (ntp_pid >= 1) 
	{
		//printf("kill SIGKILL to NTP's pid '%d'\n", ntp_pid);
		// kill it
		status = kill(ntp_pid, SIGTERM);
		if (status != 0) 
		{
			printf("Could not kill NTP's pid '%d'\n", ntp_pid);
			return -1;
		}
		// Mason Yu. Kill process in real time.
		// We delete vsntp.pid file on vsntp client process's sigtrem hander.
		#if 0
		else {
			unlink(SNTPC_PID);
		}
		#endif
	}	
	return 1;
}
#endif //TIME_ZONE

//#define DHCPSERVERPID  "/var/run/udhcpd.pid"
#define DHCPRELAYPID  "/var/run/dhcrelay.pid"

static const char DHCPD_CONF[] = "/var/udhcpd/udhcpd.conf";
const char DHCPD_LEASE[] = "/var/udhcpd/udhcpd.leases";
const char DHCPSERVERPID[] = "/var/run/udhcpd.pid";
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
#ifdef IP_BASED_CLIENT_TYPE
	char pcipstart[16], pcipend[16];
	char cmripstart[16], cmripend[16];
	char stbipstart[16], stbipend[16];
	char phnipstart[16], phnipend[16];
	char hgwipstart[16], hgwipend[16];
#endif
	char subnet[16], ipaddr[16], ipaddr2[16];
	char dhcpsubnet[16];
	char dns1[16], dns2[16], dns3[16], dhcps[16];
	char domain[MAX_NAME_LEN];
	unsigned int entryNum, i, j;
	MIB_CE_MAC_BASE_DHCP_T Entry;		
#ifdef IP_PASSTHROUGH
	int ippt;
#endif
	int spc_enable, spc_ip;
	struct in_addr myip;
		
/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	DHCPS_SERVING_POOL_T dhcppoolentry;
	int dhcppoolnum;
	unsigned char macempty[6]={0,0,0,0,0,0};
	char *ipempty="0.0.0.0";
#endif
	char macaddr[20];
	
/*ping_zhang:20080919 END*/
		
	// check if dhcp server on ?
	// Modified by Mason Yu for dhcpmode
	//if (mib_get(MIB_ADSL_LAN_DHCP, (void *)value) != 0)
	if (mib_get(MIB_DHCP_MODE, (void *)value) != 0)
	{
		uInt = (unsigned int)(*(unsigned char *)value);
		if (uInt != 2 )
		{
			return 0;	// dhcp Server not on
		}
	}		
	
#ifdef SECONDARY_IP
	mib_get(MIB_ADSL_LAN_ENABLE_IP2, (void *)value);
	if (value[0])
		mib_get(MIB_ADSL_LAN_DHCP_POOLUSE, (void *)value);
#else
	value[0] = 0;
#endif
	
#ifdef DHCPS_POOL_COMPLETE_IP
	mib_get(MIB_DHCP_SUBNET_MASK, (void *)value);
	strncpy(dhcpsubnet, inet_ntoa(*((struct in_addr *)value)), 16);
	dhcpsubnet[15] = '\0';
	// IP Pool start
	mib_get(MIB_DHCP_POOL_START, (void *)value);
	strncpy(ipstart, inet_ntoa(*((struct in_addr *)value)), 16);
	// IP Pool end
	mib_get(MIB_DHCP_POOL_END, (void *)value);
	strncpy(ipend, inet_ntoa(*((struct in_addr *)value)), 16);
#else
	if (value[0] == 0) { // primary LAN
		if (mib_get(MIB_ADSL_LAN_SUBNET, (void *)value) != 0)
		{
			strncpy(dhcpsubnet, inet_ntoa(*((struct in_addr *)value)), 16);
			dhcpsubnet[15] = '\0';
		}
		else
			return -1;
		
		mib_get(MIB_ADSL_LAN_IP, (void *)value);
	}
	else { // secondary LAN
		if (mib_get(MIB_ADSL_LAN_SUBNET2, (void *)value) != 0)
		{
			strncpy(dhcpsubnet, inet_ntoa(*((struct in_addr *)value)), 16);
			dhcpsubnet[15] = '\0';
		}
		else
			return -1;
		
		mib_get(MIB_ADSL_LAN_IP2, (void *)value);
	}
	// IP pool start address
	mib_get(MIB_ADSL_LAN_CLIENT_START, (void *)&value[3]);
	strncpy(ipstart, inet_ntoa(*((struct in_addr *)value)), 16);
	ipstart[15] = '\0';
	// IP pool end address
	mib_get(MIB_ADSL_LAN_CLIENT_END, (void *)&value[3]);
	strncpy(ipend, inet_ntoa(*((struct in_addr *)value)), 16);
	ipend[15] = '\0';
#endif
	
	// Added by Mason Yu for MAC base assignment. Start			
	if ((fp2 = fopen("/var/dhcpdMacBase.txt", "w")) == NULL)
	{
		printf("***** Open file /var/dhcpdMacBase.txt failed !\n");	
		goto dhcpConf;		
	}

#if 1
//star: reserve local ip for dhcp server
		MIB_CE_MAC_BASE_DHCP_T entry;		
				
		strcpy(macaddr, "localmac");
		//printf("entry.macAddr = %s\n", entry.macAddr);
		if (mib_get(MIB_ADSL_LAN_IP, (void *)value) != 0)
		{
			//strncpy(entry.ipAddr, inet_ntoa(*((struct in_addr *)value)), 16);
			//entry.ipAddr[15] = '\0';
			fprintf(fp2, "%s: %s\n", macaddr, inet_ntoa(*((struct in_addr *)value)));			
		}	

		
#ifdef SECONDARY_IP
		mib_get(MIB_ADSL_LAN_ENABLE_IP2, (void *)value);
		if (value[0] == 1) {
			strcpy(macaddr, "localsecmac");
			//printf("entry.macAddr = %s\n", entry.macAddr);
			if (mib_get(MIB_ADSL_LAN_IP2, (void *)value) != 0)
			{
				//strncpy(entry.ipAddr, inet_ntoa(*((struct in_addr *)value)), 16);
				//entry.ipAddr[15] = '\0';
				fprintf(fp2, "%s: %s\n", macaddr, inet_ntoa(*((struct in_addr *)value)) );
			}		
				
		}
	
#endif
#endif  //#if 1

//star: check mactbl
	struct in_addr poolstart,poolend,macip;
	unsigned long v1;
	
	inet_aton(ipstart, &poolstart);
	inet_aton(ipend, &poolend);
check_mactbl:	
	entryNum = mib_chain_total(MIB_MAC_BASE_DHCP_TBL);
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_MAC_BASE_DHCP_TBL, i, (void *)&Entry))
		{
  			printf("setupDhcpd:Get chain(MIB_MAC_BASE_DHCP_TBL) record error!\n");			
		}
		//inet_aton(Entry.ipAddr, &macip);
		v1 = *((unsigned long *)Entry.ipAddr_Dhcp);
		if( v1 < poolstart.s_addr || v1 > poolend.s_addr )
		//if(macip.s_addr<poolstart.s_addr||macip.s_addr>poolend.s_addr)
		{
			if(mib_chain_delete(MIB_MAC_BASE_DHCP_TBL, i)!=1)
			{	
				printf("setupDhcpd:Delete chain(MIB_MAC_BASE_DHCP_TBL) record error!\n");
				return -1; 
			}
			break;
		}
	}
	if((int)i<((int)entryNum-1))
		goto check_mactbl;


	entryNum = mib_chain_total(MIB_MAC_BASE_DHCP_TBL);
	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_MAC_BASE_DHCP_TBL, i, (void *)&Entry))
		{
  			printf("setupDhcpd:Get chain(MIB_MAC_BASE_DHCP_TBL) record error!\n");			
		}
		
		snprintf(macaddr, 18, "%02x-%02x-%02x-%02x-%02x-%02x",
				Entry.macAddr_Dhcp[0], Entry.macAddr_Dhcp[1],
				Entry.macAddr_Dhcp[2], Entry.macAddr_Dhcp[3],
				Entry.macAddr_Dhcp[4], Entry.macAddr_Dhcp[5]);	
				
		for (j=0; j<17; j++){
			if ( macaddr[j] != '-' ) {
				macaddr[j] = tolower(macaddr[j]);
			}	
		}	
		
		//fprintf(fp2, "%s: %s\n", Entry.macAddr, Entry.ipAddr);
		fprintf(fp2, "%s: %s\n", macaddr, inet_ntoa(*((struct in_addr *)Entry.ipAddr_Dhcp)) );			
	}			
	fclose(fp2);
	// Added by Mason Yu for MAC base assignment. End	
	
dhcpConf:
#ifdef IP_BASED_CLIENT_TYPE
#if 0
	if (mib_get(MIB_ADSL_LAN_IP, (void *)value) != 0)
	{
		strncpy(ipaddr, inet_ntoa(*((struct in_addr *)value)), 16);
		ipaddr[15] = '\0';
	}
	else
		return -1;
#endif

	// PC IP pool 
	if (mib_get(CWMP_CT_PC_MINADDR, (void *)&value[0]) != 0)
	{
		strncpy(pcipstart, inet_ntoa(*((struct in_addr *)value)), 16);
		pcipstart[15] = '\0';
	}
	else
		return -1;


	if (mib_get(CWMP_CT_PC_MAXADDR, (void *)&value[0]) != 0)
	{
		strncpy(pcipend, inet_ntoa(*((struct in_addr *)value)), 16);
		pcipend[15] = '\0';
	}
	else
		return -1;

	// CMR IP pool
	if (mib_get(CWMP_CT_CMR_MINADDR, (void *)&value[0]) != 0)
	{
		strncpy(cmripstart, inet_ntoa(*((struct in_addr *)value)), 16);
		cmripstart[15] = '\0';
	}
	else
		return -1;

	if (mib_get(CWMP_CT_CMR_MAXADDR, (void *)&value[0]) != 0)
	{
		strncpy(cmripend, inet_ntoa(*((struct in_addr *)value)), 16);
		cmripend[15] = '\0';
	}
	else
		return -1;

	// STB IP 
	if (mib_get(CWMP_CT_STB_MINADDR, (void *)&value[0]) != 0)
	{
		strncpy(stbipstart, inet_ntoa(*((struct in_addr *)value)), 16);
		stbipstart[15] = '\0';
	}
	else
		return -1;

	if (mib_get(CWMP_CT_STB_MAXADDR, (void *)&value[0]) != 0)
	{
		strncpy(stbipend, inet_ntoa(*((struct in_addr *)value)), 16);
		stbipend[15] = '\0';
	}
	else
		return -1;

	// PHN IP pool 
	if (mib_get(CWMP_CT_PHN_MINADDR, (void *)&value[0]) != 0)
	{
		strncpy(phnipstart, inet_ntoa(*((struct in_addr *)value)), 16);
		phnipstart[15] = '\0';
	}
	else
		return -1;

	if (mib_get(CWMP_CT_PHN_MAXADDR, (void *)&value[0]) != 0)
	{
		strncpy(phnipend, inet_ntoa(*((struct in_addr *)value)), 16);
		phnipend[15] = '\0';
	}
	else
		return -1;

	// HGW IP pool 
	if (mib_get(CWMP_CT_HGW_MINADDR, (void *)&value[0]) != 0)
	{
		strncpy(hgwipstart, inet_ntoa(*((struct in_addr *)value)), 16);
		hgwipstart[15] = '\0';
	}
	else
		return -1;

	if (mib_get(CWMP_CT_HGW_MAXADDR, (void *)&value[0]) != 0)
	{
		strncpy(hgwipend, inet_ntoa(*((struct in_addr *)value)), 16);
		hgwipend[15] = '\0';
	}
	else
		return -1;
#endif

	
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
	
/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	fprintf(fp,"poolname default\n");
#endif
/*ping_zhang:20080919 END*/

	
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

#ifdef IP_BASED_CLIENT_TYPE
	fprintf(fp, "pcstart %s\n", pcipstart);
	fprintf(fp, "pcend %s\n", pcipend);
	fprintf(fp, "cmrstart %s\n", cmripstart);
	fprintf(fp, "cmrend %s\n", cmripend);
	fprintf(fp, "stbstart %s\n", stbipstart);
	fprintf(fp, "stbend %s\n", stbipend);
	fprintf(fp, "phnstart %s\n", phnipstart);
	fprintf(fp, "phnend %s\n", phnipend);
	fprintf(fp, "hgwstart %s\n", hgwipstart);
	fprintf(fp, "hgwend %s\n", hgwipend);
#endif
	
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
	
	fprintf(fp, "opt subnet %s\n", dhcpsubnet);
	
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
	
	// Kaohj
#ifdef DHCPS_DNS_OPTIONS
	mib_get(MIB_DHCP_DNS_OPTION, (void *)value);
	if (value[0] == 0)
	fprintf(fp, "opt dns %s\n", ipaddr2);
	else { // check manual setting
		mib_get(MIB_DHCPS_DNS1, (void *)&myip);
		strncpy(ipaddr, inet_ntoa(myip), 16);
		ipaddr[15] = '\0';
		fprintf(fp, "opt dns %s\n", ipaddr);
		mib_get(MIB_DHCPS_DNS2, (void *)&myip);
		if (myip.s_addr != 0xffffffff) { // not empty
			strncpy(ipaddr, inet_ntoa(myip), 16);
			ipaddr[15] = '\0';
			fprintf(fp, "opt dns %s\n", ipaddr);
			mib_get(MIB_DHCPS_DNS3, (void *)&myip);
			if (myip.s_addr != 0xffffffff) { // not empty
				strncpy(ipaddr, inet_ntoa(myip), 16);
				ipaddr[15] = '\0';
				fprintf(fp, "opt dns %s\n", ipaddr);
			}
		}
	}
#else
	fprintf(fp, "opt dns %s\n", ipaddr2);
#endif
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
	char opt125_sn[65];
	mib_get(MIB_HW_SERIAL_NUMBER, (void *)opt125_sn);
	fprintf( fp, "opt venspec 3561 4 %s 5 %s 6 %s\n", MANUFACTURER_OUI, opt125_sn, PRODUCT_CLASS );
}
#endif
	
/*write dhcp serving pool config*/
/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	fprintf(fp, "poolend end\n");
	memset(&dhcppoolentry,0,sizeof(DHCPS_SERVING_POOL_T));

/*test code*
	dhcppoolentry.enable=1;
	dhcppoolentry.poolorder=1;
	strcpy(dhcppoolentry.poolname,"poolone");
	strcpy(dhcppoolentry.vendorclass,"MSFT 5.0");
	strcpy(dhcppoolentry.clientid,"");
	strcpy(dhcppoolentry.userclass,"");

	inet_aton("192.168.1.40",((struct in_addr *)(dhcppoolentry.startaddr)));
	inet_aton("192.168.1.50",((struct in_addr *)(dhcppoolentry.endaddr)));
	inet_aton("255.255.255.0",((struct in_addr *)(dhcppoolentry.subnetmask)));
	inet_aton("192.168.1.1",((struct in_addr *)(dhcppoolentry.iprouter)));
	inet_aton("172.29.17.10",((struct in_addr *)(dhcppoolentry.dnsserver1)));
	strcpy(dhcppoolentry.domainname,"poolone.com");
	dhcppoolentry.leasetime=86400;
	dhcppoolentry.dnsservermode=1;
	dhcppoolentry.InstanceNum=1;
	entryNum = mib_chain_total(MIB_DHCPS_SERVING_POOL_TBL);
printf("\nentryNum=%d\n",entryNum);
	if(entryNum==0)
		mib_chain_add(MIB_DHCPS_SERVING_POOL_TBL,(void*)&dhcppoolentry);
****/
	
	entryNum = mib_chain_total(MIB_DHCPS_SERVING_POOL_TBL);

	for(i=0;i<entryNum;i++){
		memset(&dhcppoolentry,0,sizeof(DHCPS_SERVING_POOL_T));
		if(!mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL,i,(void*)&dhcppoolentry))
			continue;
		if(dhcppoolentry.enable==1){
			strncpy(ipstart, inet_ntoa(*((struct in_addr *)(dhcppoolentry.startaddr))), 16);
			strncpy(ipend, inet_ntoa(*((struct in_addr *)(dhcppoolentry.endaddr))), 16);
			strncpy(dhcpsubnet, inet_ntoa(*((struct in_addr *)(dhcppoolentry.subnetmask))), 16);
			strncpy(ipaddr2, inet_ntoa(*((struct in_addr *)(dhcppoolentry.iprouter))), 16);

			if(!strcmp(ipstart,ipempty) || 
				!strcmp(ipend,ipempty) ||
				!strcmp(dhcpsubnet,ipempty))
				continue;
			
			fprintf(fp, "poolname %s\n",dhcppoolentry.poolname);
			fprintf(fp, "cwmpinstnum %d\n",dhcppoolentry.InstanceNum);
			fprintf(fp, "poolorder %u\n",dhcppoolentry.poolorder);
			fprintf(fp, "interface %s\n", LANIF);
			fprintf(fp, "start %s\n", ipstart);
			fprintf(fp, "end %s\n", ipend);

			fprintf(fp, "sourceinterface %u\n",dhcppoolentry.sourceinterface);
			if(dhcppoolentry.vendorclass[0]!=0){
				fprintf(fp, "vendorclass %s\n",dhcppoolentry.vendorclass);
				fprintf(fp, "vendorclassflag %u\n",dhcppoolentry.vendorclassflag);
			}
			if(dhcppoolentry.clientid[0]!=0){
				fprintf(fp, "clientid %s\n",dhcppoolentry.clientid);
				fprintf(fp, "clientidflag %u\n",dhcppoolentry.clientidflag);
			}
			if(dhcppoolentry.userclass[0]!=0){
				fprintf(fp, "userclass %s\n",dhcppoolentry.userclass);
				fprintf(fp, "userclassflag %u\n",dhcppoolentry.userclassflag);
			}
			if(memcmp(dhcppoolentry.chaddr,macempty,6)){
				fprintf(fp, "chaddr %02x%02x%02x%02x%02x%02x\n",dhcppoolentry.chaddr[0],dhcppoolentry.chaddr[1],dhcppoolentry.chaddr[2],
					dhcppoolentry.chaddr[3],dhcppoolentry.chaddr[4],dhcppoolentry.chaddr[5]);
				if(memcmp(dhcppoolentry.chaddrmask,macempty,6))
					fprintf(fp, "chaddrmask %02x%02x%02x%02x%02x%02x\n",dhcppoolentry.chaddrmask[0],dhcppoolentry.chaddrmask[1],dhcppoolentry.chaddrmask[2],
						dhcppoolentry.chaddrmask[3],dhcppoolentry.chaddrmask[4],dhcppoolentry.chaddrmask[5]);
				fprintf(fp, "chaddrflag %u\n",dhcppoolentry.chaddrflag);
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

			if(strcmp(dhcpsubnet,ipempty))
				fprintf(fp, "opt subnet %s\n", dhcpsubnet);
			if(strcmp(ipaddr2,ipempty))
				fprintf(fp, "opt router %s\n", ipaddr2);

			// Kaohj
#ifdef DHCPS_DNS_OPTIONS
			if (dhcppoolentry.dnsservermode == 0)
				fprintf(fp, "opt dns %s\n", ipaddr2);
			else { // check manual setting
				strncpy(ipaddr, inet_ntoa(*((struct in_addr *)(dhcppoolentry.dnsserver1))), 16);
				ipaddr[15] = '\0';
				fprintf(fp, "opt dns %s\n", ipaddr);
				if (memcpy(dhcppoolentry.dnsserver2,ipempty,4)) { // not empty
					strncpy(ipaddr, inet_ntoa(*((struct in_addr *)(dhcppoolentry.dnsserver2))), 16);
					ipaddr[15] = '\0';
					fprintf(fp, "opt dns %s\n", ipaddr);
					if (memcpy(dhcppoolentry.dnsserver3,ipempty,4)) { // not empty
						strncpy(ipaddr, inet_ntoa(*((struct in_addr *)(dhcppoolentry.dnsserver3))), 16);
						ipaddr[15] = '\0';
						fprintf(fp, "opt dns %s\n", ipaddr);
					}
				}
			}
#else
			fprintf(fp, "opt dns %s\n", ipaddr2);
#endif
			fprintf(fp, "opt lease %u\n", (unsigned int)(dhcppoolentry.leasetime));
			if (dhcppoolentry.domainname[0])
				fprintf(fp, "opt domain %s\n", dhcppoolentry.domainname);
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
			char opt125_sn[65];
			mib_get(MIB_HW_SERIAL_NUMBER, (void *)opt125_sn);
			fprintf( fp, "opt venspec 3561 4 %s 5 %s 6 %s\n", MANUFACTURER_OUI, opt125_sn, PRODUCT_CLASS );
			fprintf(fp, "poolend end\n");
		}
#endif
		}
	}
#endif
/*ping_zhang:20080919 END*/
	
	fclose(fp);

//star: retain the lease file between during dhcdp restart
	if((fp = fopen(DHCPD_LEASE, "r")) == NULL)
	{
		if ((fp = fopen(DHCPD_LEASE, "w")) == NULL)
		{
			printf("Open file %s failed !\n", DHCPD_LEASE);
			return -1;
		}
		fprintf(fp, "\n");
		fclose(fp);
	}
	else
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
		if (((struct in_addr *)value)->s_addr != INADDR_NONE && ((struct in_addr *)value)->s_addr != 0)
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
		if (((struct in_addr *)value)->s_addr != INADDR_NONE && ((struct in_addr *)value)->s_addr != 0)
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
		if (((struct in_addr *)value)->s_addr != INADDR_NONE && ((struct in_addr *)value)->s_addr != 0)
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

//star: for the dns restart
	if(unlink(RESOLV))
		printf("\nthe file resolv.conf doesn't exist!\n");
		
	if (symlink(str, RESOLV)) {
			printf("failed to link %s --> %s\n", str, RESOLV);
			return -1;
	}
	
	if (va_cmd(DNSRELAY, 2, 0, "-r", RESOLV))
	    return -1;
	
	return 1;
}

#ifdef IP_QOS
int _setupIPQoSRule(int enable, unsigned int i, MIB_CE_IP_QOS_T qEntry)
{
	MIB_CE_ATM_VC_T pvcEntry;
	char *argv[24], wanif[6];
	char *tc_action=NULL, *fw_action=NULL;
	
	if(enable)
	{
		tc_action = (char *)ARG_ADD;
		fw_action = (char *)FW_ADD;
	}else
	{
		tc_action = (char *)ARG_DEL;
		fw_action = (char *)FW_DEL;
	}
	
	{
		char phy[]="eth0_sw0";
		char saddr[20], daddr[20], sport[6], dport[6], mark[5], prio[4];
		char *psaddr, *pdaddr;
		int idx, tos, mark1p;
		int k, vcnum;
		
#ifdef _CWMP_MIB_
		if( enable )
			if( qEntry.enable==0 ) return 0;
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

		mark1p = _get_classification_mark(i, &qEntry);		
		if (mark1p == 0) return 0;
		#if 0
		// mark the packet:  8-bits(high) |   8-bits(low)
		//                    class id    |  802.1p (if any)
		mark1p = ((i+1) << 8);	// class id
		if (qEntry.m_1p != 0)
			mark1p |= (qEntry.m_1p-1);	// 802.1p
		#endif
		//ql: modify length of mark from 4 to 5
		snprintf(mark, 5, "%d", mark1p);
		
		// Rule
		argv[1] = (char *)ARG_T;
		argv[2] = "mangle";
		argv[3] = (char *)fw_action;
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
#ifdef CONFIG_USB_ETH
			else if (qEntry.phyPort == IFUSBETH_PHYNUM) {
				argv[idx++] = (char *)ARG_I;
				argv[idx++] = (char *)USBETHIF;
			}
#endif
#ifdef WLAN_SUPPORT
			else {
				argv[idx++] = (char *)ARG_I;
#if defined(ZTE_GENERAL_ROUTER_SC) && defined(WLAN_MBSSID)
				argv[idx++]=(char*)(WLANAPIF[qEntry.phyPort -5]);
#else			
				argv[idx++] = (char *)WLANIF;
#endif
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
		//ql_xu commented.
		//argv[idx] = "MARK";
		//argv[idx+1] = "--set-mark";
		//argv[idx+2] = mark;
		//argv[idx+3] = NULL;
		
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
		va_cmd(TC, 15, 1, "filter", (char *)tc_action, "dev", wanif,
			"parent", "1:0", "prio", prio, "protocol", "ip", "handle", mark,
			"fw", "flowid", "1:1");
#endif
		
		// Mark ip tos
#ifdef QOS_DSCP
		if(qEntry.dscp==0) {
#endif
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
#ifdef QOS_DSCP
		}
		else if (qEntry.dscp == 1) {
			if (qEntry.m_ipprio != 0 || qEntry.m_iptos != 0) {
				tos = 0;
				tos = qEntry.m_ipprio << 3;
				tos |= qEntry.m_iptos << 1;
				snprintf(prio, 4, "%d", tos);
				argv[idx] = "DSCP";
				argv[idx+1] = "--set-dscp";
				argv[idx+2] = prio;
				argv[idx+3] = NULL;
				TRACE(STA_SCRIPT, "%s %s %s %s %s %s %s ...--set-dscp %s\n", IPTABLES, argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], prio);
				do_cmd(IPTABLES, argv, 1);
			}
		}
#endif
		
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
			va_cmd(TC, 15, 1, "filter", (char *)tc_action, "dev", wanif,
				"parent", "1:0", "prio", "1", "protocol", "ip", "handle", mark,
				"fw", "flowid", prio);
			#ifdef QOS_SPEED_LIMIT_SUPPORT
			if(qEntry.limitSpeedEnabled)
			{
			char lsParent[8]={0};
			char lsprio[8]={0};			
			int flowidindex=mib_qos_speed_limit_existed(qEntry.limitSpeedRank,qEntry.prior);
			sprintf(lsParent,"1%d:",(int)qEntry.prior+1);
			sprintf(lsprio,"1%d:%d",(int)qEntry.prior+1,flowidindex+12);				
			va_cmd(TC, 15, 1, "filter", (char *)tc_action, "dev", wanif,
				"parent",lsParent, "protocol","ip","prio", "1", "handle",mark,"fw","flowid", lsprio);			
			}
			#endif
		}
	}
	
	return 0;	
}

int setupIPQoSRule(int enable)
{
	unsigned int num, i;
	MIB_CE_IP_QOS_T qEntry;
	
	num = mib_chain_total(MIB_IP_QOS_TBL);
	
	// set IP QoS rule
	for (i=0; i<num; i++) 
	{
		if (!mib_chain_get(MIB_IP_QOS_TBL, i, (void *)&qEntry))
			continue;
#ifdef QOS_DIFFSERV
		if (qEntry.enDiffserv == 1) // Diffserv entry
			continue;
#endif

		_setupIPQoSRule( enable, i, qEntry );
	}
	
	return 0;	
}

int stopIPQ(void)
{
	unsigned int num, i;
	MIB_CE_ATM_VC_T pvcEntry;
	char wanif[6];	
	
	setupIPQoSRule( 0 );
	
	num = mib_chain_total(MIB_ATM_VC_TBL);
	// Create the root "prio" qdisc on wan interface
	for (i = 0; i < num; i++) 
	{
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
		
		// Mason Yu
		// Clear all tc filter rules
		//tc filter del dev ppp0 parent 1:0 prio 2 protocol ip u32
		va_cmd(TC, 11, 1, "filter", (char *)ARG_DEL, "dev", wanif, "parent", "1:0", "prio", "2", "protocol", "ip", "u32");
		
		va_cmd(TC, 5, 1, "qdisc", (char *)ARG_DEL, "dev", wanif, "root");		
		
	}	
	
	// disable IPQoS
//#ifdef CONFIG_EXT_SWITCH  // Mason Yu, For Set IPQOS
	__dev_setupIPQoS(0);
//#endif

	//printf("IP QoS: stopped\n");
}

int setupIPQ(void)
{
	unsigned char enable;
	unsigned int num, i, k;
	MIB_CE_ATM_VC_T pvcEntry;
	MIB_CE_IP_QOS_T qEntry;
	char *argv[24], wanif[6];
#ifdef QOS_DIFFSERV
	unsigned char qosDomain;
	mib_get(MIB_QOS_DIFFSERV, (void *)&qosDomain);
	if (qosDomain == 1)
		return 1;
#endif
#ifdef QOS_SPEED_LIMIT_SUPPORT
	unsigned short pvcbandwidth;
	unsigned int totalpvcbandwidth=0;
	char pvcbandwidthS[16]={0};
	mib_get(MIB_PVC_TOTAL_BANDWIDTH,&pvcbandwidth);	
	//totalpvcbandwidth=pvcbandwidth*1024;
	//sprintf(pvcbandwidthS,"%d",totalpvcbandwidth);
	//ql-- set parameter unit is KBps.
	sprintf(pvcbandwidthS, "%d", pvcbandwidth);
	va_cmd("/bin/sarctl",2,1,"tc",pvcbandwidthS);	
#endif
	num = mib_chain_total(MIB_ATM_VC_TBL);
	// Create the root "prio" qdisc on wan interface
	for (i = 0; i < num; i++) 
	{
		#ifdef QOS_SPEED_LIMIT_SUPPORT
		char s_level[]="0", s_classid[]="1:0", s_handle[]="11:";
		#else
		char s_level[]="0", s_classid[]="1:3", s_handle[]="3:";
		#endif
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
		#ifdef QOS_SPEED_LIMIT_SUPPORT
		//tc qdisc add dev vc0 parent 1:0 handle 10: htb default 10
		char s_htbclassid[8]="10:1";
		char s_htbdefault[8]="11:11";
		memset(pvcbandwidthS,'\0',sizeof(pvcbandwidthS));
		sprintf(pvcbandwidthS,"%dkbps",pvcbandwidth);
		for (k=0; k<IPQOS_NUM_PRIOQ; k++) {
			s_classid[2]++;	
			s_htbclassid[1]++;
			va_cmd(TC, 11, 1, "qdisc", (char *)ARG_ADD, "dev", wanif,
				"parent", s_classid, "handle", s_handle, "htb","default","11");				
			va_cmd(TC, 13, 1, "class", (char *)ARG_ADD, "dev", wanif,
				"parent", s_handle, "classid", s_htbclassid, "htb","rate",pvcbandwidthS,"ceil",pvcbandwidthS);	
			///bin/tc class add dev vc0 parent 11: classid 11:11 htb rate 300kbit ceil 300kbit
			va_cmd(TC, 13, 1, "class", (char *)ARG_ADD, "dev", wanif,
				"parent", s_handle, "classid", s_htbdefault, "htb","rate",pvcbandwidthS,"ceil",pvcbandwidthS);	
			s_handle[1]++;
			s_htbdefault[1]++;
		}	
		//tc class add dev vc0 parent 10: classid 10:1 htb rate 300kbit ceil 300kbit 
		MIB_CE_IP_QOS_SPEEDRANK_T mibqosspeed;
		int mibqosspeedtotalnum=mib_chain_total(MIB_QOS_SPEED_LIMIT);
		for(k=0;k<mibqosspeedtotalnum;k++)
		{
			mib_chain_get(MIB_QOS_SPEED_LIMIT,k,&mibqosspeed);
			//printf("mibqosspeed:index=%d,count=%d,speed=%d,prior=%d\n",
			//	mibqosspeed.index,mibqosspeed.count,mibqosspeed.speed,mibqosspeed.prior);
			char parentid[8]={0};
			char classid[8]={0};
			char speedhtb[16]={0};
			sprintf(parentid,"1%d:",mibqosspeed.prior+1);
			sprintf(classid,"1%d:%d",mibqosspeed.prior+1,mibqosspeed.index+12);
			sprintf(speedhtb,"%dkbps",mibqosspeed.speed);
			//tc class add dev vc0 parent 12: classid 12:1 htb rate 300kbit ceil 300kbit 
			va_cmd(TC, 13, 1, "class", (char *)ARG_ADD, "dev", wanif,
				"parent", parentid, "classid", classid, "htb","rate",speedhtb,"ceil",speedhtb);	
			
		}
		#if 0
		#define QOSSPEEDRANK 5
		strcpy(s_htbclassid,"10:10");
		strcpy(s_handle,"11:");		
		int j=0;
		for(j=0;j<IPQOS_NUM_PRIOQ;j++)
		{
			int maxspeed=300;
			char speedRate[16]={0};
			s_htbclassid[1]++;
			s_htbclassid[4]='0';
			for(k=0;k<QOSSPEEDRANK;k++)
			{				
				s_htbclassid[4]++;
				memset(speedRate,'\0',sizeof(speedRate));		
				sprintf(speedRate,"%dkbit",maxspeed);
				maxspeed-=50;
				va_cmd(TC, 13, 1, "class", (char *)ARG_ADD, "dev", wanif,
				"parent", s_handle, "classid", s_htbclassid, "htb","rate",speedRate,"ceil",speedRate);	
			}
			s_handle[1]++;		
		}
		#endif
		#else
		if (IPQOS_NUM_PRIOQ >= 4) {
			for (k=0; k<=IPQOS_NUM_PRIOQ-4; k++) {
				s_classid[2]++;
				s_handle[0]++;
				va_cmd(TC, 9, 1, "qdisc", (char *)ARG_ADD, "dev", wanif,
				"parent", s_classid, "handle", s_handle, "pfifo");
			}
		}
		#endif
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
#ifdef CONFIG_RE8305
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
		else if(vChar == (char)PRIO_802_1p){ // PRIO_802_1p
#ifdef CONFIG_8021P_PRIO
			unsigned char value[IPQOS_NUM_PKT_PRIO];
			if(mib_get(MIB_8021P_PROI, (void *)value)== 0)
		       {
   			   printf("Get 8021P_PROI  error!\n");
			   return 0;
		        }        
#endif
			for (k=0; k<=(IPQOS_NUM_PKT_PRIO-1); k++) {
				char s_mark[]="0";
				char flow[]="1:0";
				
				s_mark[0] += (k+1);
#ifdef CONFIG_8021P_PRIO	 	
	                     flow[2] += value[k]+1;
//                              printf("flow ::::::::::%s \n",flow);
#else
	                     flow[2] += priomap[k];
#endif
	                    
				va_cmd(TC, 15, 1, "filter", (char *)ARG_ADD, "dev", wanif,
					"parent", "1:0", "prio", "2", "protocol", "ip", "handle", s_mark,
					"fw", "flowid", flow);
			}
		}
#endif
		
	}

	setupIPQoSRule(1);
	
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
//#ifdef CONFIG_EXT_SWITCH    // Mason Yu, For Set IPQOS
	__dev_setupIPQoS(1);
//#endif                      // Mason Yu, For Set IPQOS
	
	//printf("IP QoS: started\n");
}
#endif //#ifdef IP_QOS

/*ql: 20081114 START: new IP QoS*/
#ifdef NEW_IP_QOS_SUPPORT
#define QOS_SETUP_DEBUG
static unsigned int qos_setup_debug = 3;

#ifdef QOS_SETUP_DEBUG
#define QOS_SETUP_PRINT_FUNCTION                    \
    do{if(qos_setup_debug&0x1) fprintf(stderr,"%s: %s  %d\n", __FILE__, __FUNCTION__,__LINE__);}while(0)
#else
#define QOS_SETUP_PRINT_FUNCTION do{}while(0)
#endif

enum qos_policy_t
{
	PLY_PRIO=0,
	PLY_WRR,
	PLY_NONE
};

enum qos_mode_t
{
	QOS_NONE=0,
	QOS_RULE,
	QOS_TC
};

static char* proto2str2layer[] = {
    [0]" ",
    [1]"--ip-proto 6",
    [2]"--ip-proto 17",
    [3]"--ip-proto 1",
};
static char* proto2str[] = {
    [0]" ",
    [1]"-p TCP",
    [2]"-p UDP",
    [3]"-p ICMP",
};

const char QOS_CHAIN_EBT[] =  "ebt_rule";
const char QOS_CHAIN_IPT[] =  "ipt_rule";

/****************************************
* getUpLinkRate:
* DESC: get upstream link rate.
****************************************/
static int getUpLinkRate()
{
	Modem_LinkSpeed vLs;
	unsigned char ret = 0;
	unsigned int total_bandwidth = 1024;//default to be 1Mbps

	ret = adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE);
	if (ret) {
		if(0 != vLs.upstreamRate)
			total_bandwidth = vLs.upstreamRate;
	}

	mib_set(MIB_QOS_UPRATE, (void *)&total_bandwidth);
	
	return total_bandwidth;
}

/************************************************************
* NAME: setupQosChain
* DESC: setup user defined chain in iptable tables or ebtables table
* RETURN: 0 - success; 1 - fail
************************************************************/
int setupQoSChain(int enable)
{
	QOS_SETUP_PRINT_FUNCTION;
	
	if (enable) {
		va_cmd(EBTABLES, 4, 1, "-t", "broute", "-N", "ebt_rule");
		va_cmd(EBTABLES, 6, 1, "-t", "broute", "-A", "BROUTING", "-j", "ebt_rule");

#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
		va_cmd(IPTABLES, 4, 1, "-t", "mangle", "-N", "ipt_rule");
		va_cmd(IPTABLES, 6, 1, "-t", "mangle", "-A", "PREROUTING", "-j", "ipt_rule");
#else
		va_cmd(IPTABLES, 4, 1, "-t", "mangle", "-N", "ipt_rule");
		va_cmd(IPTABLES, 6, 1, "-t", "mangle", "-A", "FORWARD", "-j", "ipt_rule");
#endif
		//va_cmd(IPTABLES, 4, 1, "-t", "mangle", "-N", "qos_rule");
		//va_cmd(IPTABLES, 6, 1, "-t", "mangle", "-A", "FORWARD", "-j", "qos_rule");
	} else {
		va_cmd(EBTABLES, 4, 1, "-t", "broute", "-F", "ebt_rule");
		va_cmd(EBTABLES, 6, 1, "-t", "broute", "-D", "BROUTING", "-j", "ebt_rule");
		va_cmd(EBTABLES, 4, 1, "-t", "broute", "-X", "ebt_rule");

#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
		va_cmd(IPTABLES, 4, 1, "-t", "mangle", "-F", "ipt_rule");
		va_cmd(IPTABLES, 6, 1, "-t", "mangle", "-D", "PREROUTING", "-j", "ipt_rule");
		va_cmd(IPTABLES, 4, 1, "-t", "mangle", "-X", "ipt_rule");
#else
		va_cmd(IPTABLES, 4, 1, "-t", "mangle", "-F", "ipt_rule");
		va_cmd(IPTABLES, 6, 1, "-t", "mangle", "-D", "FORWARD", "-j", "ipt_rule");
		va_cmd(IPTABLES, 4, 1, "-t", "mangle", "-X", "ipt_rule");
#endif
	}
	return 0;
}

/********************************************************************
* NAME: setupQosTcChain
* DESC: setup user defined chain in iptable tables or ebtables table for tc shaping
* RETURN: 0 - success; 1 - fail
********************************************************************/
static int setupQosTcChain(unsigned int enable)
{
	QOS_SETUP_PRINT_FUNCTION;

	if (enable)
	{
		va_cmd(EBTABLES, 4, 1, "-t", "broute", "-N", "ebt_rule");
		va_cmd(EBTABLES, 6, 1, "-t", "broute", "-A", "BROUTING", "-j", "ebt_rule");
		
		va_cmd(IPTABLES, 4, 1, "-t", "filter", "-N", "qos_filter");
		va_cmd(IPTABLES, 7, 1, "-t", "filter", "-I", "FORWARD", "1", "-j", "qos_filter");
		
		va_cmd(IPTABLES, 4, 1, "-t", "mangle", "-N", "qos_traffic");
		va_cmd(IPTABLES, 6, 1, "-t", "mangle", "-A", "POSTROUTING", "-j", "qos_traffic");
	} else {
		va_cmd(EBTABLES, 4, 1, "-t", "broute", "-F", "ebt_rule");
		va_cmd(EBTABLES, 6, 1, "-t", "broute", "-D", "BROUTING", "-j", "ebt_rule");
		va_cmd(EBTABLES, 4, 1, "-t", "broute", "-X", "ebt_rule");
		
		va_cmd(EBTABLES, 4, 1, "-t", "filter", "-F", "INPUT");
		
		va_cmd(IPTABLES, 4, 1, "-t", "filter", "-F", "qos_filter");
		va_cmd(IPTABLES, 6, 1, "-t", "filter", "-D", "FORWARD", "-j", "qos_filter");
		va_cmd(IPTABLES, 4, 1, "-t", "filter", "-X", "qos_filter");

		va_cmd(IPTABLES, 4, 1, "-t", "mangle", "-F", "qos_traffic");
		va_cmd(IPTABLES, 6, 1, "-t", "mangle", "-D", "POSTROUTING", "-j", "qos_traffic");
		va_cmd(IPTABLES, 4, 1, "-t", "mangle", "-X", "qos_traffic");
	}

	return 1;
}

/*******************************************************
* enableIMQ:
* DESC: setup IMQ device and redirect all packet to IMQ queue.
********************************************************/
int enableIMQ()
{
	DOCMDINIT;

	QOS_SETUP_PRINT_FUNCTION;
	
	DOCMDARGVS(IFCONFIG, DOWAIT, "imq0 txqueuelen 10");
	DOCMDARGVS(IFCONFIG, DOWAIT, "imq0 up");

	DOCMDARGVS(IPTABLES, DOWAIT, "-t mangle -A PREROUTING -i br+ -j IMQ --todev 0");

	return 0;
}

/*******************************************************
* cleanup_qos_setting:
* DESC: clean all tc rule and relevant iptables/ebtables rules.
********************************************************/
void cleanupQdiscRule()
{
	MIB_CE_ATM_VC_T pvcEntry;
	int i = 0, vcNum = 0;
	char ifname[6];
	DOCMDINIT;
	
	//clear all tc rule on pvc...
	vcNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<vcNum; i++)
	{
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&pvcEntry) || !pvcEntry.enable)
			continue;
	
		//interface
		if (PPP_INDEX(pvcEntry.ifIndex) != 0x0f)// PPP interface
			snprintf(ifname, 6, "ppp%u", PPP_INDEX(pvcEntry.ifIndex));
		else// vc interface
			snprintf(ifname, 6, "vc%u", VC_INDEX(pvcEntry.ifIndex));
	
		DOCMDARGVS(TC, DOWAIT, "qdisc del dev %s root", ifname);
	}
}

void cleanup_qos_setting()
{
	unsigned char qosmode;
	DOCMDINIT;

	QOS_SETUP_PRINT_FUNCTION;

	mib_get(MIB_QOS_MODE, (void *)&qosmode);
	if (qosmode == QOS_NONE)
		return;
	else if (qosmode == QOS_TC) {
		setupQosTcChain(0);
		cleanupQdiscRule();
	}
	else if (qosmode == QOS_RULE) {
#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
		DOCMDARGVS(IPTABLES, DOWAIT, "-t mangle -D PREROUTING -i br+ -j IMQ --todev 0");
		DOCMDARGVS(IFCONFIG, DOWAIT, "imq0 down");
#endif

		setupQoSChain(0);
		
		//clear all tc rule on imq0.
#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
		DOCMDARGVS(TC, DOWAIT, "qdisc del dev imq0 root");
#else
		cleanupQdiscRule();
#endif
	}
}

/******************************************************
* NAME: setup_filter_rule
* DESC: setup filter rule for relevant qdisc, and tag packet 
                   according to qos rule.
* ARGS: policy 0 - PRIO; 1 - WRR
* RETURN: 0 - success; 1 - fail
******************************************************/
static int setup_filter_rule(unsigned char policy)
{
    MIB_CE_IP_QOS_T entry;
    int i = 0, EntryNum = 0;
	MIB_CE_ATM_VC_T pvcEntry;
	int j = 0, vcNum = 0;
    DOCMDINIT;

    EntryNum = mib_chain_total(MIB_IP_QOS_TBL);
	vcNum = mib_chain_total(MIB_ATM_VC_TBL);
    for(i=0; i<EntryNum; i++)
	{
		unsigned int mark=0;

		if(!mib_chain_get(MIB_IP_QOS_TBL, i, (void*)&entry)||!entry.enable)
		    continue;

		for (j=0; j<vcNum; j++) 
		{
			char ifname[6];
			char phyPort[16]={0};
			char sport[48], dport[48], saddr[48], daddr[48], strmark[48];
			char strdscp[24] = {0};
			char *proto=NULL;
			char *eth_proto = NULL;
#if defined( CONFIG_EXT_SWITCH) || defined(CONFIG_RTL867X_COMBO_PORTMAPPING)
			unsigned int wan_vlan_enable = 0;
#endif
			unsigned int bridge=0;
			unsigned char tos=0, dscp=0;

			if (!mib_chain_get(MIB_ATM_VC_TBL, j, (void *)&pvcEntry))
				continue;
			
			if (!pvcEntry.enable)
				continue;
			
#if defined( CONFIG_EXT_SWITCH) || defined(CONFIG_RTL867X_COMBO_PORTMAPPING)
			if (pvcEntry.vlan)
				wan_vlan_enable = 1;
#endif
			if (pvcEntry.cmode == ADSL_BR1483)
				bridge = 1;
	    
			//interface
			if (PPP_INDEX(pvcEntry.ifIndex) != 0x0f)//PPP interface
			    snprintf(ifname, 6, "ppp%u", PPP_INDEX(pvcEntry.ifIndex));
			else//vc interface
			    snprintf(ifname, 6, "vc%u", VC_INDEX(pvcEntry.ifIndex));

			//dscp match
#ifdef QOS_DSCP_MATCH
			if(0 != entry.qosDscp)
			{
				if (bridge)
					snprintf(strdscp, 24, "--ip-tos 0x%x", (entry.qosDscp-1)&0xFF);
				else
					snprintf(strdscp, 24, "-m dscp --dscp 0x%x", entry.qosDscp>>2);
			}else{
				strdscp[0]='\0';
			}
#endif

			//source address
			if(0 != entry.sip[0]) {
				if(0 != entry.smaskbit) {
					if (bridge)
						snprintf(saddr, 48, "--ip-source %s/%d", 
							inet_ntoa(*(struct in_addr*)entry.sip), entry.smaskbit);
					else
						snprintf(saddr, 48, "-s %s/%d", 
							inet_ntoa(*(struct in_addr*)entry.sip), entry.smaskbit);
				} else {
					if (bridge)
						snprintf(saddr, 48, "--ip-source %s", inet_ntoa(*(struct in_addr*)entry.sip));
					else
						snprintf(saddr, 48, "-s %s", inet_ntoa(*(struct in_addr*)entry.sip));
				}
			} else
				saddr[0]='\0';
			
			//dest address
			if(0 != entry.dip[0]) {
				if(0 != entry.dmaskbit) {
					if (bridge)
						snprintf(daddr, 48, "--ip-destination %s/%d", 
							inet_ntoa(*(struct in_addr*)entry.dip), entry.dmaskbit);
					else
						snprintf(daddr, 48, "-d %s/%d", 
							inet_ntoa(*(struct in_addr*)entry.dip), entry.dmaskbit);
				} else {
					if (bridge)
						snprintf(daddr, 48, "--ip-destination %s", inet_ntoa(*(struct in_addr*)entry.dip));
					else
						snprintf(daddr, 48, "-d %s", inet_ntoa(*(struct in_addr*)entry.dip));
				}
			} else
				daddr[0]='\0';
		
			//protocol
			if (bridge)
				proto = proto2str2layer[entry.protoType];//for ebtables
			else
				proto = proto2str[entry.protoType];//for iptables
			
			//source port (range)
			if((PROTO_NONE == entry.protoType) || 
				(PROTO_ICMP == entry.protoType) || 
				(0 == entry.sPort))
			{//if protocol is icmp or none or port not set, ignore the port
				sport[0] = '\0';
			}
			else
			{
				if (bridge)
					snprintf(sport, 48, "--ip-source-port %d", entry.sPort);
				else
					snprintf(sport, 48, "--sport %d", entry.sPort);
			}
			
			//dest port (range)
			if((PROTO_NONE == entry.protoType) || 
				(PROTO_ICMP == entry.protoType) || 
				(0 == entry.dPort))
			{//if protocol is icmp or none or port not set, ignore the port
				dport[0] = '\0';
			}
			else
			{
				if (bridge)
					snprintf(dport, 48, "--ip-destination-port %d", entry.dPort);
				else
					snprintf(dport, 48, "--dport %d", entry.dPort);
			}
		
			//lan port, USB, eth0_sw0-eth0_sw3, wlan
			if (entry.phyPort != 0xff) {
#if (defined(CONFIG_EXT_SWITCH)  && defined (IP_QOS_VPORT))
				if (entry.phyPort <= SW_PORT_NUM && entry.phyPort >= 0)
					snprintf(phyPort, 16, "-i eth0_sw%d", entry.phyPort);
#else
				if (entry.phyPort == 0)
					snprintf(phyPort, 16, "-i %s", ELANIF);
#endif
#ifdef CONFIG_USB_ETH
				else if (entry.phyPort == IFUSBETH_PHYNUM)
					snprintf(phyPort, 16, "-i %s", USBETHIF);
#endif
#ifdef WLAN_SUPPORT
				else {
#if defined(ZTE_GENERAL_ROUTER_SC) && defined(WLAN_MBSSID)
					snprintf(phyPort, 16, "-i %s", WLANAPIF[entry.phyPort - 5]);
#else
					snprintf(phyPort, 16, "-i %s", WLANIF);
#endif
				}
#endif
			} else {
				if (bridge)
					phyPort[0] = '\0';
				else
					snprintf(phyPort, 16, "-i br0");
			}
			
			//lan 802.1p mark, 0-7 bit
			if(0 != entry.vlan1p) {
				if (bridge)
					snprintf(strmark, 48, "--vlan-prio %d", (entry.vlan1p-1)&0xff);
				else
					snprintf(strmark, 48, "-m mark --mark 0x%x", (entry.vlan1p-1)&0xff);
			} else
				strmark[0] = '\0';

			if (bridge) {
				if(strmark[0] != '\0')//vlan 802.1p priority, use 802.1Q ethernet protocol
				{
					eth_proto = "-p 0x8100";
				}else {//use ipv4 for ethernet protocol
					eth_proto = "-p 0x0800";
				}
			}
		
			//wan 802.1p mark
#if defined( CONFIG_EXT_SWITCH) || defined(CONFIG_RTL867X_COMBO_PORTMAPPING)
			if(wan_vlan_enable && (0 != entry.m_1p))
#else
			if (0 != entry.m_1p)
#endif
				mark = (entry.m_1p-1)&0xf;
			mark |= (i+1) << 8;
		
			//set the mark
			if (bridge) {
				DOCMDARGVS(EBTABLES, DOWAIT, "-t broute -A %s %s %s %s %s %s %s %s %s %s -j mark --set-mark 0x%x", 
					QOS_CHAIN_EBT, phyPort, eth_proto, proto, saddr, sport, daddr, dport, strdscp, strmark, mark);
			} else {
				DOCMDARGVS(IPTABLES, DOWAIT, "-t mangle -A %s %s %s %s %s %s %s %s %s -j MARK --set-mark 0x%x",
					QOS_CHAIN_IPT, phyPort, proto, saddr, sport, daddr, dport, strdscp, strmark, mark);
			}
			
			//reset match_mark
			if(strmark[0] != '\0') {
				if (bridge)
					snprintf(strmark, 48, "--vlan-prio %d", mark);
				else
					snprintf(strmark, 48, "-m mark --mark 0x%x", mark);
			}
			
			//set dscp
#ifdef QOS_DSCP
			if (entry.dscp) {//dscp target
				if (entry.m_dscp != 0) {
					if (bridge) {
						DOCMDARGVS(EBTABLES, DOWAIT, "-t broute -A %s %s %s %s %s %s %s %s %s %s -j ftos --set-ftos 0x%x", 
							QOS_CHAIN_EBT, phyPort, eth_proto, proto, saddr, sport, daddr, dport, strdscp, strmark, (entry.m_dscp-1)&0xff);
					} else {
						DOCMDARGVS(IPTABLES, DOWAIT, "-t mangle -A %s %s %s %s %s %s %s %s %s -j DSCP --set-dscp 0x%x", 
							QOS_CHAIN_IPT, phyPort, proto, saddr, sport, daddr, dport, strdscp, strmark, entry.m_dscp>>2);
					}
				}
			} else  {
#endif
			if ((entry.m_ipprio != 0) || (entry.m_iptos != 0xff)) {
				if (entry.m_ipprio != 0)
					tos = (entry.m_ipprio-1) << 5;
				if (entry.m_iptos != 0xff)
					tos |= entry.m_iptos;

				if (bridge) {
					DOCMDARGVS(EBTABLES, DOWAIT, "-t broute -A %s %s %s %s %s %s %s %s %s %s -j ftos --set-ftos 0x%x", 
						QOS_CHAIN_EBT, phyPort, eth_proto, proto, saddr, sport, daddr, dport, strdscp, strmark, tos);
				} else {
					DOCMDARGVS(IPTABLES, DOWAIT, "-t mangle -A %s %s %s %s %s %s %s %s %s -j TOS --set-tos 0x%x", 
						QOS_CHAIN_IPT, phyPort, proto, saddr, sport, daddr, dport, strdscp, strmark, tos);
				}
			}
#ifdef QOS_DSCP
			}
#endif

#if !defined(CONFIG_IMQ) && !defined(CONFIG_IMQ_MODULE)
			if(PLY_WRR == policy)//weighted round robin
			{
				DOCMDARGVS(TC, DOWAIT, "filter add dev %s parent 1: prio 1 protocol ip handle 0x%x fw flowid 1:%d00",
					ifname, mark, entry.prior+1);
			}
			else if (PLY_PRIO == policy)//priority queue
			{
				DOCMDARGVS(TC, DOWAIT, "filter add dev %s parent 1: prio %d protocol ip handle 0x%x fw flowid 1:%d",
					ifname, entry.prior+1, mark, entry.prior+1);
			}
#endif
		}

#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
		if(PLY_WRR == policy)//weighted round robin
		{
			DOCMDARGVS(TC, DOWAIT, "filter add dev imq0 parent 1: prio 1 protocol ip handle 0x%x fw flowid 1:%d00",
				mark, entry.prior+1);
		}
		else if (PLY_PRIO == policy)//priority queue
		{
			DOCMDARGVS(TC, DOWAIT, "filter add dev imq0 parent 1: prio %d protocol ip handle 0x%x fw flowid 1:%d",
				entry.prior+1, mark, entry.prior+1);
		}
#endif
	}
	
    return 0;
}

/****************************************************************
 *NAME:      setup_traffic_rule
 *DESC:      tc class add dev $DEV parent 10:1 handle 10:$SUBID htb rate $RATE ceil $CEIL
 *           tc filter add dev $DEV parent 10: protocol ip prio 0 handle $HANDLE fw classid 10:$SUBID
 *           iptables -p $PROTO -s $SADDR -d $DADDR --sport $SPORT --dport $DPORT -j MARK --set-mark $HANDLE
 *           setup traffic control for every configuration 
 *ARGS:      index, start from 1
 *RETURN:    0 success, others  fail
 ****************************************************************/
static int setup_traffic_rule(MIB_CE_IP_TC_Tp entry)
{
	char ifname[6];
	char* tc_act = NULL, *fw_act=NULL;
	char* proto1 = NULL, *proto2 = NULL;
	char wanPort[16]={0};
	char  saddr[24], daddr[24], sport[16], dport[16];
	int upLinkRate=0, childRate=0;
	int mark;
	DOCMDINIT;

	QOS_SETUP_PRINT_FUNCTION;

	tc_act = (char*)ARG_ADD;
	fw_act = (char*)FW_ADD;

	if(NULL == entry) {
		printf("Invalid traffic contolling rule!\n");
		goto ERROR;
	}

	if (PPP_INDEX(entry->ifIndex) != 0x0f)//PPP interface
		snprintf(ifname, 16, "ppp%u", PPP_INDEX(entry->ifIndex));
	else//vc interface
		snprintf(ifname, 16, "vc%u", VC_INDEX(entry->ifIndex));

	//wan interface
	snprintf(wanPort, 16, "-o %s", ifname);

	//source address and netmask
	if(0 != entry->srcip)
	{
		if(0 != entry->smaskbits) {
			snprintf(saddr, 24, "-s %s/%d", inet_ntoa(*((struct in_addr*)(&entry->srcip))), entry->smaskbits);
		} else {
			snprintf(saddr, 24, "-s %s", inet_ntoa(*((struct in_addr*)(&entry->srcip))));
		}
	}
	else {//if not specify the source ip
		saddr[0] = '\0';
	}
	
	//destination address and netmask
	if(0 != entry->dstip) {
		if(0 != entry->dmaskbits) {
			snprintf(daddr, 24, "-d %s/%d", inet_ntoa(*((struct in_addr*)(&entry->dstip))), entry->dmaskbits);
		} else {
			snprintf(daddr, 24, "-d %s", inet_ntoa(*((struct in_addr*)(&entry->dstip))));
		}
	} else {//if not specify the dest ip
		daddr[0] = '\0';
	}

	//source port
	if(0 != entry->sport) {
		snprintf(sport, 16, "--sport %d", entry->sport);
	} else {
		sport[0] = '\0';
	}

	//destination port
	if(0 != entry->dport) {
		snprintf(dport, 16, "--dport %d", entry->dport);
	} else {
		dport[0] = '\0';
	}

	//protocol
	if (((0 != entry->sport) || (0 != entry->dport)) && 
		(entry->protoType < 2))
		entry->protoType = 4;

	if(entry->protoType<0||entry->protoType>4)//wrong protocol index
	{
		printf("Wrong protocol\n");
		goto ERROR;
	} else {
		switch(entry->protoType)
		{
			case 0://NONE
				proto1 = " ";
				//filt_proto1 = " ";
				break;
			case 1://ICMP
				proto1 = "-p ICMP";
				//filt_proto1 = "match ip ptotocol 1 0xff";
				break;
			case 2://TCP
				proto1 = "-p TCP";
				//filt_proto1 = "match ip protocol 6 0xff";
				break;
			case 3://UDP
				proto1 = "-p UDP";
				//filt_proto1 = "match ip protocol 17 0xff";
				break;
			case 4://TCP/UDP
				proto1 = "-p TCP";
				proto2 = "-p UDP";
				//filt_proto1 = "match ip protocol 6 0xff";
				//filt_proto2 = "match ip protocol 17 0xff";
				break;
		}
	}

	upLinkRate = entry->limitSpeed;
	if(0 != upLinkRate)
	{
		//get mark
		mark = (entry->entryid<<12);

		//patch: true bandwidth will be a little greater than limit value, so I minish 7% of set limit value ahead.
		int ceil;
		ceil = upLinkRate/100 * 93;

		//childRate = (10 > upLinkRate)?upLinkRate:10;
		childRate = (10>ceil)?ceil:10;

		DOCMDARGVS(TC, DOWAIT, "class %s dev %s parent 1:1 classid 1:%d0 htb rate %dkbit ceil %dkbit mpu 64 overhead 4",
			tc_act, ifname, entry->entryid, childRate, ceil);

		DOCMDARGVS(TC, DOWAIT, "qdisc %s dev %s parent 1:%d0 handle %d1: pfifo", 
			tc_act, ifname, entry->entryid, entry->entryid);

		DOCMDARGVS(TC, DOWAIT, "filter %s dev %s parent 1: protocol ip prio 0 handle 0x%x fw flowid 1:%d0",
			tc_act, ifname, mark, entry->entryid);

		DOCMDARGVS(IPTABLES, DOWAIT,  "-t mangle %s qos_traffic %s %s %s %s %s %s -j MARK --set-mark 0x%x",
			fw_act, wanPort, proto1, saddr, daddr, sport, dport, mark);
       
		/*TCP/UDP?*/
		if(proto2)//setup the other protocol
		{
			DOCMDARGVS(IPTABLES, DOWAIT, "-t mangle %s qos_traffic %s %s %s %s %s %s -j MARK --set-mark 0x%x",
				fw_act, wanPort, proto2, saddr, daddr, sport, dport, mark);
		}
	}
	else 
	{//if uprate=0, forbid traffic matching the rules
		DOCMDARGVS(IPTABLES, DOWAIT, "-t filter %s qos_filter %s %s %s %s %s %s -j DROP",
			fw_act, wanPort, proto1, saddr, daddr, sport, dport);

		/*TCP/UDP again*/
		if(proto2)
		{
			DOCMDARGVS(IPTABLES, DOWAIT, "-t filter %s qos_filter %s %s %s %s %s %s -j DROP",
				fw_act, wanPort, proto2, saddr, daddr, sport, dport);
		}
	}

	return 0;
ERROR:
	return 1;
}


/**************************************************************************
 * NAME:    setup_wrr_queue
 * DESC:    Using the htb qdisc to implement the wrr qdisc(surprised?), not
 *          CBQ, because the CBQ qdisc is so complicated and not very accurate.
 *          The skeleton of wrr(htb):
 *                                     HTB(root qdisc,1:)
 *                                      |
 *                                     HTB(root class,1:1)
 *                  ____________________|________________
 *                 |            |            |           |
 *                HTB          HTB          HTB         HTB
 *         (sub-cls,1:10) (sub-cls,1:20)(sub-cls,1:30)(sub-cls,1:40)
 *
 *         for example, bandwidth is 1024Kbit/s, there are three queues with 
 *         priority 3:2:1, then these queues are allocated rate and ceil is 
 *         1/2, 1/3, 1/6 of total bandwidth.
 *         This function is called when dsl synchronization is completed.
 *ARGS:    
 *RETURN:  0 success, 1 fail. 
**************************************************************************/
static int setup_wrr_queue(void)
{
#if !defined(CONFIG_IMQ) && !defined(CONFIG_IMQ_MODULE)
	MIB_CE_ATM_VC_T vcEntry;
	int i, EntryNum;
	char ifname[6];
#endif
    int j, quantum;
	int rate = 0;
	unsigned short total_bandwidth = 0;
	DOCMDINIT;

	QOS_SETUP_PRINT_FUNCTION;

	total_bandwidth = getUpLinkRate();
	
#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
	//tc qdisc add dev $DEV root handle 1: htb default 400
	DOCMDARGVS(TC,DOWAIT, "qdisc add dev imq0 root handle 1: htb default 400");
	
	//tc class add dev $DEV parent 1: classid 1:1 htb rate $RATE ceil $CEIL 
	DOCMDARGVS(TC, DOWAIT, "class add dev imq0 parent 1: classid 1:1 htb rate %dKbit ceil %dKbit burst 15k",
		total_bandwidth, total_bandwidth);

	for(j=1; j<=4; j++)
	{
		/*ql:20080821 START: when line rate is low than 1Mbps, rate should be smaller...*/
		//rate = 700 * (5-j)/10;
		if (total_bandwidth > 950)
			rate = 70 * (5-j);
		else if (total_bandwidth > 790)
			rate = 60 * (5-j);
		else if (total_bandwidth > 650)
			rate = 50 * (5-j);
		else if (total_bandwidth > 540)
			rate = 40 * (5-j);
		else if (total_bandwidth > 300)
			rate = 30 * (5-j);
		else
			rate = 10 * (5-j);
		/*ql:20080821 END*/
		quantum = 1250 *(5-j);

		//if total bandwidth is too small, then reduce rate value
		/*ql:20080821 START: modify rate according to ceil*/
		//rate = (rate>=ceil)?(10*(5-j)):rate;
		if (rate > total_bandwidth)
		{
			rate = total_bandwidth * (5-j)/10;
		}
		/*ql:20080821 END*/
		//tc class add dev $DEV parent 10:1 classid 10:$SUBID htb rate $RATE ceil $RATE prio $PRIO
		DOCMDARGVS(TC, DOWAIT, "class add dev imq0 parent 1:1 classid 1:%d00 htb rate %dKbit ceil %dKbit prio 0 quantum %d", 
			j, rate, total_bandwidth, quantum);
	}

	//set queue len
	DOCMDARGVS(TC, DOWAIT, "qdisc add dev imq0 parent 1:100 handle 100: pfifo limit 12");
	DOCMDARGVS(TC, DOWAIT, "qdisc add dev imq0 parent 1:200 handle 200: pfifo limit 9");
	DOCMDARGVS(TC, DOWAIT, "qdisc add dev imq0 parent 1:300 handle 300: pfifo limit 6");
	DOCMDARGVS(TC, DOWAIT, "qdisc add dev imq0 parent 1:400 handle 400: pfifo limit 3");
#else
	EntryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<EntryNum; i++)
	{
		if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&vcEntry)||!vcEntry.enable)
			continue;

		//get the interface name(?)
		if (PPP_INDEX(vcEntry.ifIndex) != 0x0f)//PPP interface
			snprintf(ifname, 6, "ppp%u", PPP_INDEX(vcEntry.ifIndex));
		else//vc interface
			snprintf(ifname, 6, "vc%u", VC_INDEX(vcEntry.ifIndex));

		//tc qdisc add dev $DEV root handle 1: htb default 400
		DOCMDARGVS(TC,DOWAIT, "qdisc add dev %s root handle 1: htb default 400",ifname);
		
		//tc class add dev $DEV parent 1: classid 1:1 htb rate $RATE ceil $CEIL 
		DOCMDARGVS(TC, DOWAIT, "class add dev %s parent 1: classid 1:1 htb rate %dKbit ceil %dKbit burst 15k",
			ifname, total_bandwidth, total_bandwidth);

		for(j=1; j<=4; j++)
		{
			/*ql:20080821 START: when line rate is low than 1Mbps, rate should be smaller...*/
			//rate = 700 * (5-j)/10;
			if (total_bandwidth > 950)
				rate = 70 * (5-j);
			else if (total_bandwidth > 790)
				rate = 60 * (5-j);
			else if (total_bandwidth > 650)
				rate = 50 * (5-j);
			else if (total_bandwidth > 540)
				rate = 40 * (5-j);
			else if (total_bandwidth > 300)
				rate = 30 * (5-j);
			else
				rate = 10 * (5-j);
			/*ql:20080821 END*/
			quantum = 1250 *(5-j);
		
			//if total bandwidth is too small, then reduce rate value
			/*ql:20080821 START: modify rate according to ceil*/
			//rate = (rate>=ceil)?(10*(5-j)):rate;
			if (rate > total_bandwidth)
			{
				rate = total_bandwidth * (5-j)/10;
			}
			/*ql:20080821 END*/
			//tc class add dev $DEV parent 10:1 classid 10:$SUBID htb rate $RATE ceil $RATE prio $PRIO
			DOCMDARGVS(TC, DOWAIT, "class add dev %s parent 1:1 classid 1:%d00 htb rate %dKbit ceil %dKbit prio 0 quantum %d", 
				ifname, j, rate, total_bandwidth, quantum);
		}

		//set queue len
		DOCMDARGVS(TC, DOWAIT, "qdisc add dev %s parent 1:100 handle 100: pfifo limit 12", ifname);
		DOCMDARGVS(TC, DOWAIT, "qdisc add dev %s parent 1:200 handle 200: pfifo limit 9", ifname);
		DOCMDARGVS(TC, DOWAIT, "qdisc add dev %s parent 1:300 handle 300: pfifo limit 6", ifname);
		DOCMDARGVS(TC, DOWAIT, "qdisc add dev %s parent 1:400 handle 400: pfifo limit 3", ifname);
	}
#endif

	//now, setup queue rules for wrr qdisc
	setup_filter_rule(PLY_WRR);
	
    return 0;
}

/*******************************************************************************
 *NAME:    setup_prio_queue
 *DESC:    if configurating policy to priority queue,
 *         create priority queues based on struct MIB_CE_IP_QUEUE_CFG_T setting,
 *         The default number of queue is four,1-4. 
 *ARGS:    None
 *RETURN:  0 success, others fail
 *******************************************************************************/
static int setup_prio_queue(void)
{
#if !defined(CONFIG_IMQ) && !defined(CONFIG_IMQ_MODULE)
	MIB_CE_ATM_VC_T vcEntry;
    int i, EntryNum;
	char ifname[6];
#endif
	DOCMDINIT;
    
    QOS_SETUP_PRINT_FUNCTION;
	
#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
	//setup basic config for imq0
	DOCMDARGVS(TC,DOWAIT, "qdisc add dev imq0 root handle 1: prio bands 4 priomap 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3");

	//set queue len
	DOCMDARGVS(TC, DOWAIT, "qdisc add dev imq0 parent 1:1 handle 100: pfifo limit 10");
	DOCMDARGVS(TC, DOWAIT, "qdisc add dev imq0 parent 1:2 handle 200: pfifo limit 10");
	DOCMDARGVS(TC, DOWAIT, "qdisc add dev imq0 parent 1:3 handle 300: pfifo limit 10");
	DOCMDARGVS(TC, DOWAIT, "qdisc add dev imq0 parent 1:4 handle 400: pfifo limit 10");
#else
	EntryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<EntryNum; i++)
	{
		if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&vcEntry) || !vcEntry.enable)
			continue;

		//get the interface name(?)
		if (PPP_INDEX(vcEntry.ifIndex) != 0x0f)//PPP interface
			snprintf(ifname, 6, "ppp%u", PPP_INDEX(vcEntry.ifIndex));
		else//vc interface
			snprintf(ifname, 6, "vc%u", VC_INDEX(vcEntry.ifIndex));

		DOCMDARGVS(TC,DOWAIT, "qdisc add dev %s root handle 1: prio bands 4 priomap 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3",ifname);

		//set queue len
		DOCMDARGVS(TC, DOWAIT, "qdisc add dev %s parent 1:1 handle 100: pfifo limit 10", ifname);
		DOCMDARGVS(TC, DOWAIT, "qdisc add dev %s parent 1:2 handle 200: pfifo limit 10", ifname);
		DOCMDARGVS(TC, DOWAIT, "qdisc add dev %s parent 1:3 handle 300: pfifo limit 10", ifname);
		DOCMDARGVS(TC, DOWAIT, "qdisc add dev %s parent 1:4 handle 400: pfifo limit 10", ifname);
	}
#endif

	//setup ip qos queue rules for pq
	setup_filter_rule(PLY_PRIO);

	return 0;
}

/*******************************************************************************
 * NAME:    setup_traffic_control
 * DESC:    main function to complte trafice controlling, 
 *          setup the basic setting by calling setup_traffic_basic,
 *          and for every configuration by calling setup_traffic_cfg(),
 *          the basic setting includes one root qdisc and root
 *          class, the setting looks like below:
 *                              HTB(root qdisc, handle 10:)
 *                               |
 *                              HTB(root class, classid 10:1)
 *            ___________________|_____________________
 *            |         |        |          |          |
 *           HTB       HTB      HTB        HTB        HTB
 *(subclass id 10:10 rate Xkbit)........       (sub class id 10:N0 rate Ykbit)
 *ARGS:    none
 *RETURN:  0 success, others fail
 *******************************************************************************/
static int setup_traffic_control()
{
	MIB_CE_IP_TC_T  entry;
	MIB_CE_ATM_VC_T vcEntry;
	int i, entry_num =0, vcEntryNum = 0;
	char ifname[6];
	unsigned char totalBandWidthEn = 0;
	unsigned short bandwidth;
	unsigned short rate, ceil;
	DOCMDINIT;
	
	QOS_SETUP_PRINT_FUNCTION;
	
	mib_get(MIB_TOTAL_BANDWIDTH_LIMIT_EN, (void *)&totalBandWidthEn);
	entry_num = mib_chain_total(MIB_IP_QOS_TC_TBL);

	if (!totalBandWidthEn && (0==entry_num))
		return 1;

	if (totalBandWidthEn)
		mib_get(MIB_TOTAL_BANDWIDTH, (void *)&bandwidth);
	else
		bandwidth = getUpLinkRate();
	
	vcEntryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0;i<vcEntryNum; i++)
	{
		if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&vcEntry)||!vcEntry.enable)
			continue;
		
		//patch: actual bandwidth maybe a little greater than configured limit value, so I minish 7% of the configured limit value ahead.
		if (totalBandWidthEn)
			ceil = bandwidth/100 * 93;
		else
			ceil = bandwidth;
		
		if (PPP_INDEX(vcEntry.ifIndex) != 0x0f)//PPP interface
			snprintf(ifname, 16, "ppp%u", PPP_INDEX(vcEntry.ifIndex));
		else//vc interface
			snprintf(ifname, 16, "vc%u", VC_INDEX(vcEntry.ifIndex));
	
		//tc qdisc add dev $DEV root handle 1: htb default 2
		DOCMDARGVS(TC,DOWAIT, "qdisc add dev %s root handle 1: htb default 2 r2q 1", ifname);
	
		//tc class add dev $DEV parent 1: classid 1:1 htb rate $RATE ceil $CEIL
		DOCMDARGVS(TC, DOWAIT, "class add dev %s parent 1: classid 1:1 htb rate %dKbit ceil %dKbit mpu 64 overhead 4 burst 15k",
			ifname, ceil, ceil);

		//patch with above
		rate = (ceil>10)?10:ceil;
		
		//tc class add dev $DEV parent 1:1 classid 1:2 htb rate $RATE ceil $CEIL
		DOCMDARGVS(TC, DOWAIT, "class add dev %s parent 1:1 classid 1:2 htb rate %dKbit ceil %dKbit mpu 64 overhead 4",
			ifname, rate, ceil);
	
		//DOCMDARGVS(TC, DOWAIT, "qdisc add dev %s parent 1:2 handle 2: tbf rate %dkbit latency 50ms burst 1540 mpu 64", 
		//	ifname, total_bandwidth);
		DOCMDARGVS(TC, DOWAIT, "qdisc add dev %s parent 1:2 handle 2: pfifo limit 10", ifname);
	}

	for(i=0; i<entry_num; i++)
	{
		if(!mib_chain_get(MIB_IP_QOS_TC_TBL, i, (void*)&entry))
			continue;

		if (setup_traffic_rule(&entry))
			return 1;
	}

	return 0;
}

int setup_qos_setting()
{
	unsigned char policy;
	unsigned char vChar, qosmode;
	
	__dev_setupIPQoS(1);

	mib_get(MIB_MPMODE, (void *)&vChar);
	if (vChar & 0x02)//qos priority
	{
		qosmode = QOS_RULE;

		//enable IP QoS on IMQ
		va_cmd("/bin/sarctl", 2, 1, "qos_imq", "1");
		
		mib_get(MIB_QOS_POLICY, (void *)&policy);

		setupQoSChain(1);

		if (policy == PLY_PRIO) {//for PRIO
			setup_prio_queue();
		}
		else if (policy == PLY_WRR) {//for WFQ
			setup_wrr_queue();
		}
		setup_default_rule();
		
#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
		enableIMQ();
#endif
	}//traffic shaping
	else
	{
		qosmode = QOS_TC;

		//disable IMQ
		va_cmd("/bin/sarctl", 2, 1, "qos_imq", "0");
		
		setupQosTcChain(1);

		if (setup_traffic_control()) {
			qosmode = QOS_NONE;
			setupQosTcChain(0);
			cleanupQdiscRule();
		}
	}
	mib_set(MIB_QOS_MODE, (void *)&qosmode);
	
	return 0;
}

void take_qos_effect()
{
    //clean up old setting
    cleanup_qos_setting();
    //setup new one
    setup_qos_setting();
}

/******************************************************
* NAME: stop_IPQoS
* DESC: when IP QoS stopped, traffic control will be effective.
******************************************************/
void stop_IPQoS()
{
	unsigned char mode=0;
	
	mib_get(MIB_MPMODE, (void *)&mode);
	mode &= 0xfd;
	mib_set(MIB_MPMODE, (void *)&mode);

	take_qos_effect();
}

int change_queue(unsigned int upLinkRate)
{
    MIB_CE_ATM_VC_T entry;
    int i, j, vcnum=0;
	int rate, quantum;
    char ifname[6] = {0};
	unsigned char qosmode;
	DOCMDINIT;

	mib_get(MIB_QOS_MODE, (void *)&qosmode);

	if (qosmode == QOS_RULE)
	{
#if !defined(CONFIG_IMQ) && !defined(CONFIG_IMQ_MODULE)
	    vcnum = mib_chain_total(MIB_ATM_VC_TBL);
	    for(i=0; i<vcnum; i++)
	    {
			if((!mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&entry)) || (!entry.enable))
			    continue;

			if (PPP_INDEX(entry.ifIndex) != 0x0f)//PPP interface
			    snprintf(ifname, 6, "ppp%u", PPP_INDEX(entry.ifIndex));
			else//vc interface
			    snprintf(ifname, 6, "vc%u", VC_INDEX(entry.ifIndex));

			//tc class change dev $DEV parent 10: classid 10:1 htb rate $RATE ceil $CEIL prio 10
			DOCMDARGVS(TC, UNDOWAIT, "class change dev %s parent 1: classid 1:1 htb rate %dKbit ceil %dKbit burst 15k",
				ifname, upLinkRate, upLinkRate);

			for (j =1; j<=4; j++)
			{
				/*ql:20080821 START: when line rate is low than 1Mbps, rate should be smaller...*/
				//rate = 700 * (5-j)/10;
				if (upLinkRate > 950)
					rate = 70 * (5-j);
				else if (upLinkRate > 790)
					rate = 60 * (5-j);
				else if (upLinkRate > 650)
					rate = 50 * (5-j);
				else if (upLinkRate > 540)
					rate = 40 * (5-j);
				else if (upLinkRate > 300)
					rate = 30 * (5-j);
				else
					rate = 10 * (5-j);
				/*ql:20080821 END*/
				quantum = 1250 *(5-j);
				
				//if total bandwidth is too small, then reduce rate value
				if (rate > upLinkRate)
				{
					rate = upLinkRate * (5-j)/10;
				}
				//add subclass for one queue config
				//tc class add dev $DEV parent 10:1 classid 10:$SUBID htb rate $RATE ceil $RATE prio $PRIO
				DOCMDARGVS(TC, UNDOWAIT, "class change dev %s parent 1:1 classid 1:%d00 htb rate %dKbit ceil %dKbit prio 0 quantum %d", 
					ifname, j, rate, upLinkRate, quantum);
			}
	    }
#else
		//tc class change dev $DEV parent 1: classid 1:1 htb rate $RATE ceil $CEIL
		DOCMDARGVS(TC, UNDOWAIT, "class change dev imq0 parent 1: classid 1:1 htb rate %dKbit ceil %dKbit burst 15k",
			upLinkRate, upLinkRate);

		for (j =1; j<=4; j++)
		{
				/*ql:20080821 START: when line rate is low than 1Mbps, rate should be smaller...*/
				//rate = 70 * (5-j);
				if (upLinkRate > 950)
					rate = 70 * (5-j);
				else if (upLinkRate > 790)
					rate = 60 * (5-j);
				else if (upLinkRate > 650)
					rate = 50 * (5-j);
				else if (upLinkRate > 540)
					rate = 40 * (5-j);
				else if (upLinkRate > 300)
					rate = 30 * (5-j);
				else
					rate = 10 * (5-j);
				/*ql:20080821 END*/
				quantum = 1250 * (5-j);
				/*ql:20080821 START: modify rate according to ceil*/
				//rate = (rate>=ceil)?(10*(5-j)):rate;
				if (rate > upLinkRate)
				{
					rate = upLinkRate * (5-j)/10;
				}
				/*ql:20080821 END*/

				//tc class add dev $DEV parent 10:1 classid 10:$SUBID htb rate $RATE ceil $RATE prio $PRIO
				DOCMDARGVS(TC, UNDOWAIT, "class change dev imq0 parent 1:1 classid 1:%d00 htb rate %dKbit ceil %dKbit prio 0 quantum %d", 
					j, rate, upLinkRate, quantum);
		}
#endif
	}
	else if (qosmode == QOS_TC)
	{
	    vcnum = mib_chain_total(MIB_ATM_VC_TBL);
	    for(i=0; i<vcnum; i++)
	    {
			if((!mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&entry)) || (!entry.enable))
			    continue;

			if (PPP_INDEX(entry.ifIndex) != 0x0f)//PPP interface
			    snprintf(ifname, 6, "ppp%u", PPP_INDEX(entry.ifIndex));
			else//vc interface
			    snprintf(ifname, 6, "vc%u", VC_INDEX(entry.ifIndex));

			//tc class change dev $DEV parent 1: classid 1:1 htb rate $RATE ceil $CEIL
			DOCMDARGVS(TC,UNDOWAIT, "class change dev %s parent 1: classid 1:1 htb rate %dKbit ceil %dKbit",
				ifname, upLinkRate, upLinkRate);
			
			//tc class change dev $DEV parent 1:1 classid 1:2 htb rate $RATE ceil $CEIL
			DOCMDARGVS(TC,UNDOWAIT, "class change dev %s parent 1:1 classid 1:2 htb rate 10Kbit ceil %dKbit",
				ifname, upLinkRate);
	    }
	}

	mib_set(MIB_QOS_UPRATE, (void *)&upLinkRate);

	return 0;
}

int monitor_qos_setting()
{
    Modem_LinkSpeed vLs;
    unsigned char ret;
    unsigned char policy, mode, bandwidthlimit;
	unsigned int dsl_uprate;
	
	mib_get(MIB_QOS_POLICY, (void*)&policy);
	mib_get(MIB_QOS_UPRATE, (void *)&dsl_uprate);
	mib_get(MIB_QOS_MODE, (void *)&mode);
	mib_get(MIB_TOTAL_BANDWIDTH_LIMIT_EN, (void *)&bandwidthlimit);

	if (((mode==QOS_RULE) && (policy == PLY_WRR)) ||
		((mode==QOS_TC) && !bandwidthlimit))
	{//wrr or traffical control with no totalbandwidth restrict
	    ret = adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE);
	    if (ret)			
		{
			if(0 != vLs.upstreamRate)//setup
			{
			    if(dsl_uprate == vLs.upstreamRate)//need not setup
					return 0;
				else if((0 != dsl_uprate) && (dsl_uprate != vLs.upstreamRate))
				{
					change_queue(vLs.upstreamRate);
			    }
			}
		}
	}
    return 0;
}

void setup_default_rule()
{
#if !defined(CONFIG_IMQ) && !defined(CONFIG_IMQ_MODULE)
	MIB_CE_ATM_VC_T vcEntry;
	int i, vcnum;
	char ifname[6];
#endif
	int k;
	unsigned char vChar, policy;
	DOCMDINIT;

	QOS_SETUP_PRINT_FUNCTION;

	mib_get(MIB_QOS_DOMAIN, (void *)&vChar);
	mib_get(MIB_QOS_POLICY, (void*)&policy);

#if !defined(CONFIG_IMQ) && !defined(CONFIG_IMQ_MODULE)
	vcnum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<vcnum; i++) {
		if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&vcEntry)||!vcEntry.enable)
			continue;

		//get the interface name(?)
		if (PPP_INDEX(vcEntry.ifIndex) != 0x0f)//PPP interface
			snprintf(ifname, 6, "ppp%u", PPP_INDEX(vcEntry.ifIndex));
		else//vc interface
			snprintf(ifname, 6, "vc%u", VC_INDEX(vcEntry.ifIndex));

		if (vChar == (char)PRIO_IP) {
#ifdef CONFIG_8021P_PRIO
			unsigned char value[IPQOS_NUM_PKT_PRIO];
			if(mib_get(MIB_PRED_PROI, (void *)value)== 0)
			{
				printf("Get 8021P_PROI  error!\n");
				return 0;
			}        
#endif
			for (k=0; k<=(IPQOS_NUM_PKT_PRIO-1); k++) {
				char pattern[]="0x00";
				int prio, flowid;
	                	
				prio = k<<1;
				if (prio<=9)
					pattern[2] += prio; // highest 3 bits
				else
					pattern[2] = 'a'+(prio-10);
				
#ifdef CONFIG_8021P_PRIO
				flowid = value[k]+1;
#else
				flowid = priomap[k];
#endif
				
				// match ip tos PATTERN MASK
				if(PLY_WRR == policy)//weighted round robin
				{
					DOCMDARGVS(TC, DOWAIT, "filter add dev %s parent 1: prio 2 protocol ip u32 match ip tos %s 0xe0 flowid 1:%d00",
						ifname, pattern, flowid);
				}
				else if (PLY_PRIO == policy)//priority queue
				{
					DOCMDARGVS(TC, DOWAIT, "filter add dev %s parent 1: prio 5 protocol ip u32 match ip tos %s 0xe0 flowid 1:%d",
						ifname, pattern, flowid);
				}
			}
		}
		else if(vChar == (char)PRIO_802_1p){ // PRIO_802_1p
#ifdef CONFIG_8021P_PRIO
			unsigned char value[IPQOS_NUM_PKT_PRIO];
			if(mib_get(MIB_8021P_PROI, (void *)value)== 0)
			{
				printf("Get 8021P_PROI  error!\n");
				return 0;
			}        
#endif
			for (k=0; k<=(IPQOS_NUM_PKT_PRIO-1); k++) {
				int flowid;
				
#ifdef CONFIG_8021P_PRIO
				flowid = value[k]+1;
#else
				flowid = priomap[k];
#endif

				if(PLY_WRR == policy)//weighted round robin
				{
					DOCMDARGVS(TC, DOWAIT, "filter add dev %s parent 1: prio 2 protocol ip handle %d fw flowid 1:%d00",
						ifname, k+1, flowid);
				}
				else if (PLY_PRIO == policy)//priority queue
				{
					DOCMDARGVS(TC, DOWAIT, "filter add dev %s parent 1: prio 5 protocol ip handle %d fw flowid 1:%d",
						ifname, k+1, flowid);
				}
			}
		}
	}
#else
	if (vChar == (char)PRIO_IP) {
#ifdef CONFIG_8021P_PRIO
		unsigned char value[IPQOS_NUM_PKT_PRIO];
		if(mib_get(MIB_PRED_PROI, (void *)value)== 0)
		{
			printf("Get 8021P_PROI  error!\n");
			return 0;
		}        
#endif
		for (k=0; k<=(IPQOS_NUM_PKT_PRIO-1); k++) {
			char pattern[]="0x00";
			int prio, flowid;
	                	
			prio = k<<1;
			if (prio<=9)
				pattern[2] += prio; // highest 3 bits
			else
				pattern[2] = 'a'+(prio-10);
			
#ifdef CONFIG_8021P_PRIO
			flowid = value[k]+1;
#else
			flowid = priomap[k];
#endif
			// match ip tos PATTERN MASK
			if(PLY_WRR == policy)//weighted round robin
			{
				DOCMDARGVS(TC, DOWAIT, "filter add dev imq0 parent 1: prio 2 protocol ip u32 match ip tos %s 0xe0 flowid 1:%d00",
					pattern, flowid);
			}
			else if (PLY_PRIO == policy)//priority queue
			{
				DOCMDARGVS(TC, DOWAIT, "filter add dev imq0 parent 1: prio 5 protocol ip u32 match ip tos %s 0xe0 flowid 1:%d",
					pattern, flowid);
			}
		}
	}
	else if(vChar == (char)PRIO_802_1p){ // PRIO_802_1p
#ifdef CONFIG_8021P_PRIO
		unsigned char value[IPQOS_NUM_PKT_PRIO];
		if(mib_get(MIB_8021P_PROI, (void *)value)== 0)
		{
			printf("Get 8021P_PROI  error!\n");
			return 0;
		}        
#endif
		for (k=0; k<=(IPQOS_NUM_PKT_PRIO-1); k++) {
			int flowid;
			
#ifdef CONFIG_8021P_PRIO
			flowid = value[k]+1;
#else
			flowid = priomap[k];
#endif

			if(PLY_WRR == policy)//weighted round robin
			{
				DOCMDARGVS(TC, DOWAIT, "filter add dev imq0 parent 1: prio 2 protocol ip handle %d fw flowid 1:%d00",
					k+1, flowid);
			}
			else if (PLY_PRIO == policy)//priority queue
			{
				DOCMDARGVS(TC, DOWAIT, "filter add dev imq0 parent 1: prio 5 protocol ip handle %d fw flowid 1:%d",
					k+1, flowid);
			}
		}
	}
#endif
}

#endif
/*ql: 20081114 END*/

#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
// Added by Mason Yu for ILMI(PVC) community string
const char PVCREADSTR[] = "ADSL";
const char PVCWRITESTR[] = "ADSL";
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

#define SNMPPID  "/var/run/snmpd.pid"

int restart_snmp(int flag)
{
	unsigned char value[32];
	int snmppid=0;
	int status=0;

	snmppid = read_pid((char*)SNMPPID);

	//printf("\nsnmppid=%d\n",snmppid);

	if(snmppid > 0) {
		kill(snmppid, 9);
		unlink(SNMPPID);
	}

	if(flag==1){
		status = startSnmp();
	}
	return status;
}
#endif

#define CWMPPID  "/var/run/cwmp.pid"
void off_tr069()
{
	int cwmppid=0;
	int status;

	cwmppid = read_pid((char*)CWMPPID);

	printf("\ncwmppid=%d\n",cwmppid);

	if(cwmppid > 0)
		kill(cwmppid, 15);

}

#ifdef IP_ACL
int restart_acl()
{
	unsigned char aclEnable;

	//va_cmd(IPTABLES, 2, 1, "-F", "aclblock");
	mib_get(MIB_ACL_CAPABILITY, (void *)&aclEnable);
	if (aclEnable == 1)  // ACL Capability is enabled
	{
		filter_set_acl(0);
		filter_set_acl(1);
	}
	else
		filter_set_acl(0);

	return 0;
}
#endif

#ifdef NAT_CONN_LIMIT
//ql_xu add
int restart_connlimit()
{
	unsigned char connlimitEn;

	va_cmd(IPTABLES, 2, 1, "-F", "connlimit");

	mib_get(MIB_NAT_CONN_LIMIT, (void *)&connlimitEn);
	if (connlimitEn == 1)
		set_conn_limit();
}
#endif
#ifdef TCP_UDP_CONN_LIMIT
int restart_connlimit()
{
	unsigned char connlimitEn;

	va_cmd(IPTABLES, 2, 1, "-F", "connlimit");

	mib_get(MIB_CONNLIMIT_ENABLE, (void *)&connlimitEn);
	if (connlimitEn == 1)
		set_conn_limit();
}
#endif 



#ifdef URL_BLOCKING_SUPPORT

int restart_urlblocking()
{		
	unsigned char urlEnable;	

	va_cmd(IPTABLES, 2, 1, "-F", "urlblock");		
	mib_get(MIB_URL_CAPABILITY, (void *)&urlEnable);
	if (urlEnable == 1)  // URL Capability enabled
		filter_set_url(1);
	else
		filter_set_url(0);	
	return 0;
}
#ifdef URL_ALLOWING_SUPPORT
int restart_url()
{
      
	unsigned char urlEnable;	

	va_cmd(IPTABLES, 2, 1, "-F", "urlallow");
	va_cmd(IPTABLES, 2, 1, "-F", "urlblock");
	mib_get(MIB_URL_CAPABILITY, (void *)&urlEnable);
	if (urlEnable == 1)  // URL Capability enabled
		filter_set_url(1);
	else if(urlEnable == 2) //URL allow
		set_url(2);

	return 0;
}
#endif
#endif

//Ip interface
int restart_lanip()
{
	char vChar=0;
	unsigned char ipaddr[32]="";
	unsigned char subnet[32]="";
	unsigned char value[6];
	int vInt;
	int status=0;
	FILE * fp;
#ifdef IP_PASSTHROUGH
	unsigned char ippt_addr[32]="";
	unsigned char ippt_itf;
	unsigned long myipaddr,hisipaddr;
	int ippt_flag = 0;
#endif
	
	itfcfg((char *)LANIF, 0);
#ifdef SECONDARY_IP
	itfcfg((char *)LAN_ALIAS, 0);
#endif
#ifdef IP_PASSTHROUGH
	itfcfg((char *)LAN_IPPT, 0);
	if (mib_get(MIB_IPPT_ITF, (void *)&ippt_itf) != 0) {
		if (ippt_itf != 255) {	// IP passthrough
			fp = fopen ("/tmp/PPPHalfBridge", "r");
			if (fp) {
				fread(&myipaddr, 4, 1, fp);
				// Added by Mason Yu. Access internet fail.
				fclose(fp);
				myipaddr+=1;
				strncpy(ippt_addr, inet_ntoa(*((struct in_addr *)(&myipaddr))), 16);
				ippt_addr[15] = '\0';
				ippt_flag =1;
			}
			
		}
	}
#endif

#ifdef _CWMP_MIB_
	mib_get(CWMP_LAN_IPIFENABLE, (void *)&vChar);
	if (vChar != 0)
#endif
	{
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
			status|=va_cmd(IFCONFIG, 6, 1, (char*)LAN_ALIAS, ipaddr, "netmask", subnet, "mtu", value);
		}
#endif
             
#ifdef IP_PASSTHROUGH
               if(ippt_flag)
			   status|=va_cmd(IFCONFIG, 2, 1, (char*)LAN_IPPT,ippt_addr);
#endif
	
		status|=restart_dhcp();
//		status|=restart_dnsrelay();

#ifdef QSETUP_WEB_REDIRECT
		/*
		unsigned char iptables_flag = 0;

		mib_get(MIB_QSETUP_REDIRECT, (void *)&iptables_flag);
		if (iptables_flag) {
			stop_qsetup_redirect();
			start_qsetup_redirect();
		}
		*/
		FILE *fp;
		int iptables_flag = 0;
		if ((fp = fopen("/tmp/QSetup", "r"))) {
			fscanf(fp, "%d\n", &iptables_flag);
			fclose(fp);
			if (iptables_flag) {
				stop_qsetup_redirect();
				start_qsetup_redirect();
			}
		}
#endif
		return status;
	}
	return -1;
}

//DHCP

#ifdef EMBED
int getOneDhcpClient(char **ppStart, unsigned long *size, char *ip, char *mac, char *liveTime)
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

int restart_dhcp()
{
	unsigned char value[32];
	unsigned int uInt;
	int dhcpserverpid=0,dhcprelaypid=0;
	int tmp_status, status=0;

	if (mib_get(MIB_DHCP_MODE, (void *)value) != 0)
	{
		uInt = (unsigned int)(*(unsigned char *)value);
//		if (uInt != 0 && uInt !=1 && uInt != 2 )
//			return -1;
	}

	dhcpserverpid = read_pid((char*)DHCPSERVERPID);
	dhcprelaypid = read_pid((char*)DHCPRELAYPID);

	//printf("\ndhcpserverpid=%d,dhcprelaypid=%d\n",dhcpserverpid,dhcprelaypid);

	if(dhcpserverpid > 0)
		kill(dhcpserverpid, 15);
	if(dhcprelaypid > 0)
		kill(dhcprelaypid, 15);

	if(uInt != DHCP_LAN_SERVER)
	{
		FILE *fp;
		//star:clean the lease file
		if ((fp = fopen(DHCPD_LEASE, "w")) == NULL)
		{
			printf("Open file %s failed !\n", DHCPD_LEASE);
			return -1;
		}
		fprintf(fp, "\n");
		fclose(fp);
	}
			

	if(uInt == DHCP_LAN_SERVER)
	{
		tmp_status = setupDhcpd();
		if (tmp_status == 1)
		{
			//printf("\nrestart dhcpserver!\n");
			status = va_cmd(DHCPD, 1, 0, DHCPD_CONF);
#ifndef ZTE_531B_BRIDGE_SC
			restart_dnsrelay();
#endif
		} 
		else if (tmp_status == -1)
	   	 	status = -1;
		return status;
	}
	else if(uInt == DHCP_LAN_RELAY)
	{
		startDhcpRelay();
#ifndef ZTE_531B_BRIDGE_SC
		restart_dnsrelay();
#endif
		status=(status==-1)?-1:0;
			
		return status;
	}
	else 
	{
#ifndef ZTE_531B_BRIDGE_SC
		restart_dnsrelay();
#endif
		return 0;
	}

	

	return -1;
}

//DNSRELAY

#define DNSRELAYPID  "/var/run/dnsmasq.pid"

int restart_dnsrelay()  
{
	unsigned char value[32];
	int dnsrelaypid=0, i;

	dnsrelaypid = read_pid((char*)DNSRELAYPID);

	//printf("\ndnsrelaypid=%d\n",dnsrelaypid);

	if(dnsrelaypid > 0)
		kill(dnsrelaypid, SIGTERM);
	
	// Kaohj -- wait for process termination
	for (i=0; i<10000; i++) {
		dnsrelaypid = read_pid((char*)DNSRELAYPID);
		if (dnsrelaypid<=0)
			break;
	}
	
	if (startDnsRelay(BOOT_LAST) == -1)
	{
		printf("restart DNS relay failed !\n");
		return -1;
	}
	return 0;

}

int delPortForwarding( unsigned char ifindex )
{
	int total,i;
		
	total = mib_chain_total( MIB_PORT_FW_TBL );
	//for( i=0;i<total;i++ )
	for( i=total-1;i>=0;i-- )
	{
		MIB_CE_PORT_FW_T *c, port_entity;
		c = &port_entity;
		if( !mib_chain_get( MIB_PORT_FW_TBL, i, (void*)c ) )
			continue;

		if(c->ifIndex==ifindex)
			mib_chain_delete( MIB_PORT_FW_TBL, i );
	}	
	return 0;
}
int updatePortForwarding( unsigned char old_id, unsigned char new_id )
{
	unsigned int total,i;
	
	total = mib_chain_total( MIB_PORT_FW_TBL );
	for( i=0;i<total;i++ )
	{
		MIB_CE_PORT_FW_T *c, port_entity;
		c = &port_entity;
		if( !mib_chain_get( MIB_PORT_FW_TBL, i, (void*)c ) )
			continue;

		if(c->ifIndex==old_id)
		{
			c->ifIndex = new_id;
			mib_chain_update( MIB_PORT_FW_TBL, (unsigned char*)c, i );
		}
	}
	return 0;
}
int delRoutingTable( unsigned char ifindex )
{
	int total,i;
		
	total = mib_chain_total( MIB_IP_ROUTE_TBL );
	//for( i=0;i<total;i++ )
	for( i=total-1;i>=0;i-- )
	{
		MIB_CE_IP_ROUTE_T *c, entity;
		c = &entity;
		if( !mib_chain_get( MIB_IP_ROUTE_TBL, i, (void*)c ) )
			continue;

		if(c->ifIndex==ifindex)
			mib_chain_delete( MIB_IP_ROUTE_TBL, i );
	}	
	return 0;
}
/*ql:20080926 START: delete MIB_DHCP_CLIENT_OPTION_TBL entry to this pvc*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
int delDhcpcOption( unsigned char ifindex )
{
	MIB_CE_DHCP_OPTION_T entry;
	int i, entrynum;

	entrynum = mib_chain_total(MIB_DHCP_CLIENT_OPTION_TBL);
	for (i=entrynum-1; i>=0; i--)
	{
		if (!mib_chain_get(MIB_DHCP_CLIENT_OPTION_TBL, i, (void *)&entry))
			continue;

		if (entry.ifIndex == ifindex)
			mib_chain_delete(MIB_DHCP_CLIENT_OPTION_TBL, i);
	}
	return 0;
}
#endif
/*ql:20080926 END*/
#ifdef NEW_IP_QOS_SUPPORT
int delIpQosTcRule(MIB_CE_ATM_VC_Tp pEntry)
{
	int total, i;
	MIB_CE_IP_TC_T entry;

	total = mib_chain_total(MIB_IP_QOS_TC_TBL);
	for (i=total-1; i>=0; i--)
	{
		mib_chain_get(MIB_IP_QOS_TC_TBL, i, &entry);
		if (entry.ifIndex != pEntry->ifIndex)
			continue;

		mib_chain_delete(MIB_IP_QOS_TC_TBL, i);
	}

	return(1);
}
#endif

int updateRoutingTable( unsigned char old_id, unsigned char new_id )
{
	unsigned int total,i;
	
	total = mib_chain_total( MIB_IP_ROUTE_TBL );
	for( i=0;i<total;i++ )
	{
		MIB_CE_IP_ROUTE_T *c, entity;
		c = &entity;
		if( !mib_chain_get( MIB_IP_ROUTE_TBL, i, (void*)c ) )
			continue;

		if(c->ifIndex==old_id)
		{
			c->ifIndex = new_id;
			mib_chain_update( MIB_IP_ROUTE_TBL, (unsigned char*)c, i );
		}
	}
	return 0;
}

// Added by Mason Yu for take effect in real time.
int apply_PortForwarding( int action_type, int id, void *olddata )
{
	MIB_CE_PORT_FW_T *pOldPort=olddata;
	MIB_CE_PORT_FW_T PortEntry, *pNewPort=NULL;

	//got the lastest entry
	if( mib_chain_get( MIB_PORT_FW_TBL, id,  (void*)&PortEntry )!=0 ) //0:error
		pNewPort =  &PortEntry;
	
	switch( action_type )
	{
	case ACT_RESTART:  //CWMP_RESTART:
		if(pOldPort) portfw_modify( pOldPort, 1 );
	case ACT_START:    //CWMP_START:
		if(pNewPort) portfw_modify( pNewPort, 0 );
		break;
	case ACT_STOP:     //CWMP_STOP:
		if(pOldPort) 
			portfw_modify( pOldPort, 1 );
		else if(pNewPort)
			portfw_modify( pNewPort, 1 );
		break;
	default:
		return -1;
	}
	return 0;
}



#ifdef _CWMP_MIB_

unsigned int findMaxConDevInstNum(void)
{
	unsigned int ret=0, i,num;
	MIB_CE_ATM_VC_T *p,vc_entity;
	
	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		p = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)p ))
			continue;
		if( p->ConDevInstNum > ret )
			ret = p->ConDevInstNum;
	}
	
	return ret;
}

unsigned int findConDevInstNumByPVC(unsigned char vpi, unsigned short vci)
{
	unsigned int ret=0, i,num;
	MIB_CE_ATM_VC_T *p,vc_entity;
	
	if( (vpi==0) && (vci==0) ) return ret;
	
	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		p = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)p ))
			continue;
		if( p->vpi==vpi && p->vci==vci )
		{
			ret = p->ConDevInstNum;
			break;
		}
	}
	
	return ret;
}

unsigned int findMaxPPPConInstNum( unsigned int condev_inst )
{
	unsigned int ret=0, i,num;
	MIB_CE_ATM_VC_T *p,vc_entity;
	
	if(condev_inst==0) return ret;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		p = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)p ))
			continue;
		if( p->ConDevInstNum == condev_inst )
		{
			if( (p->cmode==ADSL_PPPoE) ||
#ifdef PPPOE_PASSTHROUGH
			    ((p->cmode==ADSL_BR1483)&&(p->brmode==BRIDGE_PPPOE)) ||
#endif
			    (p->cmode==ADSL_PPPoA) )
			{
				if( p->ConPPPInstNum > ret )
					ret = p->ConPPPInstNum;
			}
		}
	}
	
	return ret;
}

unsigned int findMaxIPConInstNum( unsigned int condev_inst )
{
	unsigned int ret=0, i,num;
	MIB_CE_ATM_VC_T *p,vc_entity;
	
	if(condev_inst==0) return ret;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		p = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)p ))
			continue;
		if( p->ConDevInstNum==condev_inst )
		{
			if( (p->cmode == ADSL_MER1483) || 
#ifdef PPPOE_PASSTHROUGH
			    ((p->cmode==ADSL_BR1483)&&(p->brmode!=BRIDGE_PPPOE)) ||
#else
			    (p->cmode == ADSL_BR1483) ||
#endif
			    (p->cmode == ADSL_RT1483) )
			{
				if( p->ConIPInstNum > ret )
					ret = p->ConIPInstNum;
			}
		}
	}
	
	return ret;
}
/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
unsigned int findMaxDHCPOptionInstNum( unsigned int usedFor, unsigned int dhcpConSPInstNum)
{
	unsigned int ret=0, i,num;
	MIB_CE_DHCP_OPTION_T *p,DHCPOption_entity;
	
	num = mib_chain_total( MIB_DHCP_SERVER_OPTION_TBL );
	for( i=0; i<num;i++ )
	{
		p = &DHCPOption_entity;
		if( !mib_chain_get( MIB_DHCP_SERVER_OPTION_TBL, i, (void*)p ))
			continue;
		if(p->usedFor!=usedFor || dhcpConSPInstNum != p->dhcpConSPInstNum)
			continue;
		if(p->dhcpOptInstNum>ret)
			ret=p->dhcpOptInstNum;
	}
	
	return ret;
}

int getDHCPOptionByOptInstNum( unsigned int dhcpOptNum, unsigned int dhcpSPNum, unsigned int usedFor, MIB_CE_DHCP_OPTION_T *p, unsigned int *id )
{
	int ret=-1;
	unsigned int i,num;
	
	if( (dhcpOptNum==0) || (p==NULL) || (id==NULL) )
		return ret;
		
	num = mib_chain_total( MIB_DHCP_SERVER_OPTION_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_DHCP_SERVER_OPTION_TBL, i, (void*)p ) )
			continue;

		if( (p->usedFor==usedFor) && (p->dhcpOptInstNum==dhcpOptNum) && (p->dhcpConSPInstNum==dhcpSPNum) )
		{
			*id = i;
			ret = 0;
			break;
		}
	}	
	return ret;	
}

int getDHCPClientOptionByOptInstNum( unsigned int dhcpOptNum, unsigned char ifIndex, unsigned int usedFor, MIB_CE_DHCP_OPTION_T *p, unsigned int *id )
{
	int ret=-1;
	unsigned int i,num;
	
	if( (dhcpOptNum==0) || (p==NULL) || (id==NULL) )
		return ret;
		
	num = mib_chain_total( MIB_DHCP_CLIENT_OPTION_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)p ) )
			continue;

		if( (p->usedFor==usedFor) && (p->dhcpOptInstNum==dhcpOptNum) &&(p->ifIndex==ifIndex) )
		{
			*id = i;
			ret = 0;
			break;
		}
	}	
	return ret;	
}
unsigned int findMaxDHCPClientOptionInstNum(int usedFor, unsigned char ifIndex)
{
	unsigned int ret=0, i,num;
	MIB_CE_DHCP_OPTION_T *p,DHCPOption_entity;
	
	num = mib_chain_total( MIB_DHCP_CLIENT_OPTION_TBL );
	for( i=0; i<num;i++ )
	{
		p = &DHCPOption_entity;
		if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)p ))
			continue;
		if(p->usedFor!=usedFor||p->ifIndex!=ifIndex)
			continue;
		if(p->dhcpOptInstNum>ret)
			ret=p->dhcpOptInstNum;
	}
	
	return ret;

}

unsigned int findDHCPOptionNum(int usedFor, unsigned char ifIndex)
{
	unsigned int ret=0, i,num;
	MIB_CE_DHCP_OPTION_T *p,DHCPOption_entity;
	
	num = mib_chain_total( MIB_DHCP_CLIENT_OPTION_TBL );
	for( i=0; i<num;i++ )
	{
		p = &DHCPOption_entity;
		if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)p ))
			continue;
		if(p->usedFor==usedFor && p->ifIndex==ifIndex)
			ret++;
	}
	
	return ret;

}

unsigned int findMaxDHCPReqOptionOrder(void )
{
	unsigned int ret=0, i,num;
	MIB_CE_DHCP_OPTION_T *p,DHCPSP_entity;
	
	num = mib_chain_total( MIB_DHCP_CLIENT_OPTION_TBL );
	for( i=0; i<num;i++ )
	{
		p = &DHCPSP_entity;
		if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)p ))
			continue;
		if(p->usedFor!=eUsedFor_DHCPClient_Req)
			continue;
		if(p->order>ret)
			ret=p->order;
	}
	
	return ret;
}

unsigned int findMaxDHCPConSPInsNum(void )
{
	unsigned int ret=0, i,num;
	DHCPS_SERVING_POOL_T *p,DHCPSP_entity;
	
	num = mib_chain_total( MIB_DHCPS_SERVING_POOL_TBL );
	for( i=0; i<num;i++ )
	{
		p = &DHCPSP_entity;
		if( !mib_chain_get( MIB_DHCPS_SERVING_POOL_TBL, i, (void*)p ))
			continue;
		if(p->InstanceNum>ret)
			ret=p->InstanceNum;
	}
	
	return ret;
}

unsigned int findMaxDHCPConSPOrder(void )
{
	unsigned int ret=0, i,num;
	DHCPS_SERVING_POOL_T *p,DHCPSP_entity;
	
	num = mib_chain_total( MIB_DHCPS_SERVING_POOL_TBL );
	for( i=0; i<num;i++ )
	{
		p = &DHCPSP_entity;
		if( !mib_chain_get( MIB_DHCPS_SERVING_POOL_TBL, i, (void*)p ))
			continue;
		if(p->poolorder>ret)
			ret=p->poolorder;
	}
	
	return ret;
}

int getDHCPConSPByInstNum( unsigned int dhcpspNum,  DHCPS_SERVING_POOL_T *p, unsigned int *id )
{
	int ret=-1;
	unsigned int i,num;
	
	if( (dhcpspNum==0) || (p==NULL) || (id==NULL) )
		return ret;
		
	num = mib_chain_total( MIB_DHCPS_SERVING_POOL_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_DHCPS_SERVING_POOL_TBL, i, (void*)p ) )
			continue;

		if( p->InstanceNum==dhcpspNum) 
		{
			*id = i;
			ret = 0;
			break;
		}
	}	
	return ret;	
}

#endif
/*ping_zhang:20080919 END*/

MIB_CE_ATM_VC_T *getATMVCByInstNum( unsigned int devnum, unsigned int ipnum, unsigned int pppnum, MIB_CE_ATM_VC_T *p, unsigned int *chainid )
{
	unsigned int i,num;
	
	if( (p==NULL) || (chainid==NULL) ) return NULL;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)p ))
			continue;

		if( (p->ConDevInstNum==devnum) &&
		    (p->ConIPInstNum==ipnum) &&
		    (p->ConPPPInstNum==pppnum) ) 
		{
			*chainid=i;
			return p;
		}
	}
	
	return NULL;
}

#ifdef _CWMP_APPLY_

//RIP
int apply_RIP( int action_type, int id, void *olddata )
{
	//for RIP, id and olddata have no meaning
	switch( action_type )
	{
	case CWMP_START:
	case CWMP_RESTART:
	case CWMP_STOP:
		startRip();
		break;
	default:
		return -1;
	}
	return 0;
}

#ifdef ITF_GROUP
//PortMapping
int apply_PortMapping( int action_type, int id, void *olddata )
{
	
// set debug mode,jiunming test
//DEBUGMODE(STA_INFO|STA_SCRIPT|STA_WARNING|STA_ERR);

	//for PortMapping, id and olddata have no meaning
	switch( action_type )
	{
	case CWMP_RESTART:
		//stop portmapping? but for tr069, once enable, always enable!
	case CWMP_START:
		{
			unsigned char mode=0;
#ifdef CONFIG_EXT_SWITCH
			mib_get(MIB_MPMODE, (void *)&mode);
			if(mode&0x01) setupEth2pvc();
#endif
		}
		break;
	case CWMP_STOP:
		break;
	default:
		return -1;
	}
	return 0;
}
#endif //ITF_GROUP

//ROUTE
int apply_Layer3Forwarding( int action_type, int id, void *olddata )
{
	MIB_CE_IP_ROUTE_T *pOldRoute=olddata;
	MIB_CE_IP_ROUTE_T RouteEntry, *pNewRoute=NULL;

	//got the lastest entry
	if( mib_chain_get( MIB_IP_ROUTE_TBL, id,  (void*)&RouteEntry )!=0 ) //0:error
		pNewRoute =  &RouteEntry;
	
	switch( action_type )
	{
	case CWMP_RESTART:
		if( pOldRoute &&
		    pOldRoute->Enable &&
		    !( pOldRoute->ifIndex==0xff &&
		      pOldRoute->nextHop[0]==0 &&
		      pOldRoute->nextHop[1]==0 &&
		      pOldRoute->nextHop[2]==0 &&
		      pOldRoute->nextHop[3]==0 )
		  )
			route_cfg_modify( pOldRoute, 1 );
	case CWMP_START:
		if( pNewRoute &&
		    pNewRoute->Enable &&
		    !( pNewRoute->ifIndex==0xff &&
		      pNewRoute->nextHop[0]==0 &&
		      pNewRoute->nextHop[1]==0 &&
		      pNewRoute->nextHop[2]==0 &&
		      pNewRoute->nextHop[3]==0 )
		  )
			route_cfg_modify( pNewRoute, -1 );
		break;
	case CWMP_STOP:
		if( pOldRoute &&
		    pOldRoute->Enable &&
		    !( pOldRoute->ifIndex==0xff &&
		      pOldRoute->nextHop[0]==0 &&
		      pOldRoute->nextHop[1]==0 &&
		      pOldRoute->nextHop[2]==0 &&
		      pOldRoute->nextHop[3]==0 )
		  )
			route_cfg_modify( pOldRoute, 1 );
		break;
	default:
		return -1;
	}
	return 0;
}

int apply_DefaultRoute( int action_type, int id, void *olddata )
{
	//for DefaultRoute, id and olddata have no meaning
	switch( action_type )
	{
	case CWMP_RESTART:
		va_cmd(ROUTE, 2, 1, "delete", "default");
	case CWMP_START:
		{
#ifdef DEFAULT_GATEWAY_V2
			unsigned char dgw;
			if (mib_get(MIB_ADSL_WAN_DGW_ITF, (void *)&dgw) != 0)
			{
#ifdef AUTO_PPPOE_ROUTE
				if (dgw != DGW_AUTO)
#endif
				{
					char ifname[16] = "";
					if (ifGetName(dgw, ifname, sizeof(ifname)) != NULL)
					{
#ifdef DEFAULT_GATEWAY_V2
						if (ifExistedDGW() == 1)	// Jenny, delete existed default gateway first
							va_cmd(ROUTE, 2, 1, ARG_DEL, "default");
#endif
						va_cmd(ROUTE, 3, 1, ARG_ADD, "default", ifname);
					}
				}
			}
#else
			MIB_CE_ATM_VC_T *pEntry, vc_entity;
			int total,i;
			total = mib_chain_total(MIB_ATM_VC_TBL);
			for( i=0; i<total; i++ )
			{
				pEntry = &vc_entity;
				if( !mib_chain_get(MIB_ATM_VC_TBL, i, (void*)pEntry ) )
					continue;
				
				if( (pEntry->dgw==1) && (pEntry->enable==1) )
				{
					char ifname[16]="";
					if( ifGetName(pEntry->ifIndex, ifname, sizeof(ifname))!=NULL )
						va_cmd(ROUTE, 3, 1, ARG_ADD, "default", ifname );
				}
			}
#endif
		}
		break;
	case CWMP_STOP:
		va_cmd(ROUTE, 2, 1, "delete", "default");
		break;
	default:
		return -1;
	}
	return 0;
}

#if 0
int apply_PortForwarding( int action_type, int id, void *olddata )
{
	MIB_CE_PORT_FW_T *pOldPort=olddata;
	MIB_CE_PORT_FW_T PortEntry, *pNewPort=NULL;

	//got the lastest entry
	if( mib_chain_get( MIB_PORT_FW_TBL, id,  (void*)&PortEntry )!=0 ) //0:error
		pNewPort =  &PortEntry;
	
	switch( action_type )
	{
	case CWMP_RESTART:
		if(pOldPort) portfw_modify( pOldPort, 1 );
	case CWMP_START:
		if(pNewPort) portfw_modify( pNewPort, 0 );
		break;
	case CWMP_STOP:
		if(pOldPort) portfw_modify( pOldPort, 1 );
		break;
	default:
		return -1;
	}
	return 0;
}
#endif

#if defined(TIME_ZONE) || defined(ZTE_GENERAL_ROUTER_SC)
int apply_NTP( int action_type, int id, void *olddata )
{
	//for NTP, id and olddata have no meaning
	switch( action_type )
	{
	case CWMP_RESTART:
		stopNTP();
	case CWMP_START:
		startNTP();
		break;
	case CWMP_STOP:
		stopNTP();
		break;
	default:
		return -1;
	}
	return 0;
}
#endif //TIME_ZONE

int apply_RemoteAccess( int action_type, int id, void *olddata )
{
	MIB_CE_ACC_T *pOldAcc=olddata;
	MIB_CE_ACC_T AccEntry, *pNewAcc=NULL;

	//got the lastest entry
	if (mib_chain_get(MIB_ACC_TBL, 0, (void *)&AccEntry)) //0:error
		pNewAcc = &AccEntry;

	//void remote_access_modify(  MIB_CE_ACC_T accEntry, int enable)
	switch( action_type )
	{
	case CWMP_RESTART:
		if(pOldAcc) remote_access_modify( *pOldAcc, 0 );
	case CWMP_START:
		if(pNewAcc) remote_access_modify( *pNewAcc, 1 );
		break;
	case CWMP_STOP:
		if(pOldAcc) remote_access_modify( *pOldAcc, 0 );
		break;
	default:
		return -1;
	}
	return 0;
}


#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
#ifdef IP_QOS
int apply_IPQoSRule( int action_type, int id, void *olddata )
{
	MIB_CE_IP_QOS_T *pOldQoS=olddata;

// set debug mode,jiunming test
//DEBUGMODE(STA_INFO|STA_SCRIPT|STA_WARNING|STA_ERR);

	//for IPQoSRule, 
	//id=-1 => take action for all rules, 
	//id>-1 => modify the specific entry 
	switch( action_type )
	{
	case CWMP_RESTART:
		if(id>-1)
			if(pOldQoS) _setupIPQoSRule(0, id, *pOldQoS);
		setupIPQoSRule(0);
	case CWMP_START:
		{
			unsigned char mode=0;
			mib_get(MIB_MPMODE, (void *)&mode);
			if (mode&0x02) setupIPQoSRule(1);
		}
		break;
	case CWMP_STOP:
		if(id>-1) 
			if(pOldQoS) _setupIPQoSRule(0, id, *pOldQoS);
		else
			setupIPQoSRule(0);
		break;
	default:
		return -1;
	}
	return 0;
}
#endif

int apply_IPQoS( int action_type, int id, void *olddata )
{
	
// set debug mode,jiunming test
//DEBUGMODE(STA_INFO|STA_SCRIPT|STA_WARNING|STA_ERR);

	//for IPQoS, id and olddata have no meaning
	switch( action_type )
	{
	case CWMP_RESTART:
#ifndef NEW_IP_QOS_SUPPORT
                stopIPQ();
#else
                stop_IPQoS();
#endif
	case CWMP_START:
		{
			unsigned char mode=0;
			mib_get(MIB_MPMODE, (void *)&mode);
			//ql 20081117 START modify qos function
#ifndef NEW_IP_QOS_SUPPORT
			if (mode&0x02) setupIPQ();
#else
			if (mode & 0x02) take_qos_effect();
#endif
		}
		break;
	case CWMP_STOP:
#ifndef NEW_IP_QOS_SUPPORT
		stopIPQ();
#else
		stop_IPQoS();
#endif
		break;
	default:
		return -1;
	}
	return 0;
}
#endif //#ifdef IP_QOS

int apply_DHCP( int action_type, int id, void *olddata )
{
	//for RIP, id and olddata have no meaning
	switch( action_type )
	{
	case CWMP_START:
	case CWMP_RESTART:
	case CWMP_STOP:
		restart_dhcp();
		break;
	default:
		return -1;
	}
	return 0;
}

int apply_DNS( int action_type, int id, void *olddata )
{
	//for RIP, id and olddata have no meaning
	switch( action_type )
	{
	case CWMP_START:
	case CWMP_RESTART:
	case CWMP_STOP:
		restart_dnsrelay();
		break;
	default:
		return -1;
	}
	return 0;
}

#ifdef CONFIG_IP_NF_ALG_ONOFF
int apply_ALGONOFF( int action_type, int id, void *olddata )
{
	switch( action_type )
	{
	case CWMP_START:
	case CWMP_RESTART:
	case CWMP_STOP:
		setupAlgOnOff();
		break;
	default:
		return -1;
	}
	return 0;

}
#endif
//WIRELESS
#ifdef WLAN_SUPPORT
/*
#define WIRELESSAUTHPID  "/var/run/auth-wlan0.pid"
#define IWCONTROLPID  "/var/run/iwcontrol.pid"

int restart_wlan(int wlaninst)
{
	char vChar;
	ENCRYPT_T encrypt;
	unsigned char no_wlan;
	unsigned char enable1x=0;
	int status = 0;
	int wirelessauthpid=0,iwcontrolpid=0;

// set debug mode,jiunming test
//DEBUGMODE(STA_INFO|STA_SCRIPT|STA_WARNING|STA_ERR);

	wirelessauthpid = read_pid((char*)WIRELESSAUTHPID);
	iwcontrolpid = read_pid((char*)IWCONTROLPID);

	printf("\nwirelessauthpid=%d\n",wirelessauthpid);
	printf("\niwcontrolpid=%d\n",iwcontrolpid);

	if(wirelessauthpid > 0)
		kill(wirelessauthpid, 9);
	if(iwcontrolpid > 0)
		kill(iwcontrolpid, 9);

	status |= va_cmd(IFCONFIG, 2, 1, "wlan0", "down");
#ifdef WLAN_MBSSID
	status |= va_cmd(IFCONFIG, 2, 1, "wlan0-vap0", "down");
	status |= va_cmd(IFCONFIG, 2, 1, "wlan0-vap1", "down");
	status |= va_cmd(IFCONFIG, 2, 1, "wlan0-vap2", "down");
	status |= va_cmd(IFCONFIG, 2, 1, "wlan0-vap3", "down");
#endif

	if (1) //no_wlan == 0)	// WLAN enabled
	{
		status |= startWLan();
		status = (status==-1)?-1:1;
		return status;
	}
	else 
		return 0;
	
	return -1;

}
*/
int apply_WLAN( int action_type, int id, void *olddata )
{
	//for RIP, id and olddata have no meaning
	switch( action_type )
	{
	case CWMP_START:
	case CWMP_RESTART:
	case CWMP_STOP:
		config_WLAN(ACT_RESTART);	
		break;
	default:
		return -1;
	}
	return 0;
}
#endif	// of WLAN_SUPPORT

// Mason Yu
#if 0
//Ethernet
#if defined(ELAN_LINK_MODE)
#include <linux/sockios.h>
struct mii_ioctl_data {
	unsigned short	phy_id;
	unsigned short	reg_num;
	unsigned short	val_in;
	unsigned short	val_out;
};
#endif


int restart_ethernet(int instnum)
{
	char vChar=0;
	int status=0;

	//eth0 interface enable or disable
	mib_get(CWMP_LAN_ETHIFENABLE, (void *)&vChar);	
	
	if(vChar==0)
	{
		va_cmd(IFCONFIG, 2, 1, "eth0", "down");
		printf("Disable eth0 interface\n");
		return 0;
	}
	else
	{
		va_cmd(IFCONFIG, 2, 1, "eth0", "up");
		printf("Enable eth0 interface\n");
#if defined(CONFIG_EXT_SWITCH) 
#if defined(ELAN_LINK_MODE) 
		if(instnum != 0)  //from the setupLinkMode() in startup.c
		{
			int skfd;
			struct ifreq ifr;
			struct mii_ioctl_data *mii = (struct mii_data *)&ifr.ifr_data;
			MIB_CE_SW_PORT_T Port;
			int i, k, total;
			
	
			strcpy(ifr.ifr_name, "eth0");
			
	
			if (!mib_chain_get(MIB_SW_PORT_TBL, instnum-1, (void *)&Port))
				return -1;
			mii->phy_id = instnum-1; // phy i
			mii->reg_num = 4; // register 4
			// set NWAY advertisement
			mii->val_in = 0x0401; // enable flow control capability and IEEE802.3
			if (Port.linkMode == LINK_10HALF || Port.linkMode == LINK_AUTO)
				mii->val_in |= (1<<(5+LINK_10HALF));
			if (Port.linkMode == LINK_10FULL || Port.linkMode == LINK_AUTO)
				mii->val_in |= (1<<(5+LINK_10FULL));
			if (Port.linkMode == LINK_100HALF || Port.linkMode == LINK_AUTO)
				mii->val_in |= (1<<(5+LINK_100HALF));
			if (Port.linkMode == LINK_100FULL || Port.linkMode == LINK_AUTO)
				mii->val_in |= (1<<(5+LINK_100FULL));
			skfd = socket(AF_INET, SOCK_DGRAM, 0);
			if(skfd == -1)
			{
				fprintf(stderr, "Socket Open failed Error\n");
				return -1;
			}
			if (ioctl(skfd, SIOCSMIIREG, &ifr) < 0) {
				fprintf(stderr, "SIOCSMIIREG on %s failed: %s\n", ifr.ifr_name,
					strerror(errno));
				status=-1;
			}

			// restart
			mii->reg_num = 0; // register 0
			mii->val_in = 0x1200; // enable auto-negotiation and restart it
			if (ioctl(skfd, SIOCSMIIREG, &ifr) < 0) {
				fprintf(stderr, "SIOCSMIIREG on %s failed: %s\n", ifr.ifr_name,
					strerror(errno));
				status=-1;
			}
			
			close(skfd);

		}
#endif // 
#else // CONFIG_EXT_SWITCH
#ifdef ELAN_LINK_MODE_INTRENAL_PHY
		int skfd;
		struct ifreq ifr;
		unsigned char mode;
		//struct mii_ioctl_data *mii = (struct mii_data *)&ifr.ifr_data;
		//MIB_CE_SW_PORT_T Port;
		//int i, k, total;
		struct ethtool_cmd ecmd;
			
	
		strcpy(ifr.ifr_name, "eth0");
		ifr.ifr_data = &ecmd;
			
	
		if (!mib_get(MIB_ETH_MODE, (void *)&mode))
			return -1;

		skfd = socket(AF_INET, SOCK_DGRAM, 0);
		if(skfd == -1)
		{
			fprintf(stderr, "Socket Open failed Error\n");
			return -1;
		}
		ecmd.cmd = ETHTOOL_GSET;
		if (ioctl(skfd, SIOCETHTOOL, &ifr) < 0) {
			fprintf(stderr, "ETHTOOL_GSET on %s failed: %s\n", ifr.ifr_name,
				strerror(errno));
			status=-1;
			goto error;
		}

		ecmd.autoneg = AUTONEG_DISABLE;
		switch(mode) {
		case LINK_10HALF:
			ecmd.speed = SPEED_10;
			ecmd.duplex = DUPLEX_HALF;
			break;
		case LINK_10FULL:
			ecmd.speed = SPEED_10;
			ecmd.duplex = DUPLEX_FULL;
			break;
		case LINK_100HALF:
			ecmd.speed = SPEED_100;
			ecmd.duplex = DUPLEX_HALF;
			break;
		case LINK_100FULL:
			ecmd.speed = SPEED_100;
			ecmd.duplex = DUPLEX_FULL;
			break;
		default:
			ecmd.autoneg = AUTONEG_ENABLE;
		}

		ecmd.cmd = ETHTOOL_SSET;
		if (ioctl(skfd, SIOCETHTOOL, &ifr) < 0) {
			fprintf(stderr, "ETHTOOL_SSET on %s failed: %s\n", ifr.ifr_name,
				strerror(errno));
			status=-1;
			goto error;
		}
		/*
		ecmd.cmd = ETHTOOL_NWAY_RST;
		if (ioctl(skfd, SIOCETHTOOL, &ifr) < 0) {
			fprintf(stderr, "ETHTOOL_SSET on %s failed: %s\n", ifr.ifr_name,
				strerror(errno));
			status=-1;
		}
		*/

		error:
		
		close(skfd);
#endif
#endif
		return status;
	}

	return -1;

}
#endif

int apply_ETHER( int action_type, int id, void *olddata )
{
	switch( action_type )
	{
	case CWMP_START:
	case CWMP_RESTART:
	case CWMP_STOP:
		restart_ethernet(id);
		break;
	default:
		return -1;
	}
	return 0;

}

int apply_LANIP( int action_type, int id, void *olddata )
{
	switch( action_type )
	{
	case CWMP_START:
	case CWMP_RESTART:
	case CWMP_STOP:
		restart_lanip();
		break;
	default:
		return -1;
	}
	return 0;

}

#ifdef MAC_FILTER
//jim luo, this function is only used for tr069
int restart_macfilter()   //from part of the setupFirewall() in startup.c
{
	unsigned char value[32];
	MIB_CE_MAC_FILTER_T MacEntry;
	MIB_CE_ATM_VC_T Entry;
	char mac_in_dft, mac_out_dft;
	char eth_mac_ctrl=0, wlan_mac_ctrl=0;
	int total,vcNum,vInt;
	char *policy;
	char srcmacaddr[18], dstmacaddr[18];
	char str_vc[] = "vc0";
	int i;

	total = ifWanNum("br");
	vInt = mib_chain_total(MIB_MAC_FILTER_TBL);
	mib_get(MIB_MACF_OUT_ACTION, (void *)&mac_out_dft);
	mib_get(MIB_MACF_IN_ACTION, (void *)&mac_in_dft);
	mib_get(MIB_ETH_MAC_CTRL, (void *)&eth_mac_ctrl);
	mib_get(MIB_WLAN_MAC_CTRL, (void *)&wlan_mac_ctrl);

	if(total >= 1 && ((mac_out_dft==0 || mac_in_dft==0) || vInt >= 1))
	 {
		// Enable Bridge Netfiltering					
		//va_cmd("/bin/brctl", 2, 0, "brnf", "on");
		brnfctrl(1,BRNF_MODULE_MAC_FILTER);
	}else {
		// Disable Bridge Netfiltering
		//va_cmd("/bin/brctl", 2, 0, "brnf", "off");
		brnfctrl(0,BRNF_MODULE_MAC_FILTER);
	}

	va_cmd(IPTABLES, 2, 1, "-F", (char *)FW_MACFILTER);

	{
		total = mib_chain_total(MIB_MAC_FILTER_TBL);
		vcNum = mib_chain_total(MIB_ATM_VC_TBL);
		
		// Commented by Mason Yu
		//if (total >= 1) {
			// Add chain for MAC filtering
			// iptables -N macfilter
			//va_cmd(IPTABLES, 2, 1, "-N", (char *)FW_MACFILTER);
		//}
		
		for (i = 0; i < total; i++)
		{
			if (!mib_chain_get(MIB_MAC_FILTER_TBL, i, (void *)&MacEntry))
				return -1;
			
			if (MacEntry.action == 0)
				policy = (char *)FW_DROP;
			else
				policy = (char *)FW_RETURN;
				
			
			snprintf(srcmacaddr, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
				MacEntry.srcMac[0], MacEntry.srcMac[1],
				MacEntry.srcMac[2], MacEntry.srcMac[3],
				MacEntry.srcMac[4], MacEntry.srcMac[5]);
			
			
				
			snprintf(dstmacaddr, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
				MacEntry.dstMac[0], MacEntry.dstMac[1],
				MacEntry.dstMac[2], MacEntry.dstMac[3],
				MacEntry.dstMac[4], MacEntry.dstMac[5]);	
			
			
			
			// Added by Mason Yu for Incoming MAC filtering
			if (MacEntry.dir == DIR_OUT) {
				if (   MacEntry.dstMac[0]==0 && MacEntry.dstMac[1]==0
			    	    && MacEntry.dstMac[2]==0 && MacEntry.dstMac[3]==0
		 	    	    && MacEntry.dstMac[4]==0 && MacEntry.dstMac[5]==0 ) {
					if(mac_out_dft || eth_mac_ctrl)
						// iptables -A macfilter -i eth0  -m mac --mac-source $MAC -j ACCEPT/DROP
						va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
							(char *)ARG_I, (char *)ELANIF, "-m", "mac",
							"--mac-source",  srcmacaddr, "-j", policy);	
					if(mac_out_dft ||wlan_mac_ctrl)
						// iptables -A macfilter -i wlan0  -m mac --mac-source $MAC -j ACCEPT/DROP
						va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
							(char *)ARG_I, (char *)WLANIF, "-m", "mac",
							"--mac-source",  srcmacaddr, "-j", policy);	
#ifdef CONFIG_USB_ETH
						// iptables -A macfilter -i usb0  -m mac --mac-source $MAC -j ACCEPT/DROP
						va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
							(char *)ARG_I, (char *)USBETHIF, "-m", "mac",
							"--mac-source",  srcmacaddr, "-j", policy);	
#endif //CONFIG_USB_ETH
					
				}else if (   MacEntry.srcMac[0]==0 && MacEntry.srcMac[1]==0
			    	  	  && MacEntry.srcMac[2]==0 && MacEntry.srcMac[3]==0
		 	                  && MacEntry.srcMac[4]==0 && MacEntry.srcMac[5]==0 ) { 
				
					// iptables -A macfilter -i eth0  -m mac --mac-dst $MAC -j ACCEPT/DROP
					va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
						(char *)ARG_I, (char *)ELANIF, "-m", "mac",
						"--mac-dst",  dstmacaddr, "-j", policy);	
					// iptables -A macfilter -i wlan0  -m mac --mac-dst $MAC -j ACCEPT/DROP
					va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
						(char *)ARG_I, (char *)WLANIF, "-m", "mac",
						"--mac-dst",  dstmacaddr, "-j", policy);
#ifdef CONFIG_USB_ETH
					// iptables -A macfilter -i usb0  -m mac --mac-dst $MAC -j ACCEPT/DROP
					va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
						(char *)ARG_I, (char *)USBETHIF, "-m", "mac",
						"--mac-dst",  dstmacaddr, "-j", policy);
#endif //CONFIG_USB_ETH				
				}else {
					// iptables -A macfilter -i eth0  -m mac --mac-source $MAC --mac-dst $MAC -j ACCEPT/DROP
					va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
						(char *)ARG_I, (char *)ELANIF, "-m", "mac",
						"--mac-source",  srcmacaddr, "--mac-dst",  dstmacaddr, "-j", policy);	
					// iptables -A macfilter -i wlan0  -m mac --mac-source $MAC --mac-dst $MAC -j ACCEPT/DROP
					va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
						(char *)ARG_I, (char *)WLANIF, "-m", "mac",
						"--mac-source",  srcmacaddr, "--mac-dst",  dstmacaddr, "-j", policy);
#ifdef CONFIG_USB_ETH
					// iptables -A macfilter -i usb0  -m mac --mac-source $MAC --mac-dst $MAC -j ACCEPT/DROP
					va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
						(char *)ARG_I, (char *)USBETHIF, "-m", "mac",
						"--mac-source",  srcmacaddr, "--mac-dst",  dstmacaddr, "-j", policy);
#endif //CONFIG_USB_ETH
				}
					
			}
			else {
				for (vInt=0; vInt<vcNum; vInt++) {
					if (!mib_chain_get(MIB_ATM_VC_TBL, vInt, (void *)&Entry))
						continue;
					if (Entry.enable == 0)
						continue;
					
					if (Entry.cmode == ADSL_BR1483) {
						// vc index should be less than 10
						str_vc[2] = '0'+VC_INDEX(Entry.ifIndex);
						
						if (   MacEntry.dstMac[0]==0 && MacEntry.dstMac[1]==0
			    	    		    && MacEntry.dstMac[2]==0 && MacEntry.dstMac[3]==0
		 	    	    		    && MacEntry.dstMac[4]==0 && MacEntry.dstMac[5]==0 ) {
						
							// iptables -A macfilter -i vc0  -m mac --mac-source $MAC -j ACCEPT/DROP
							va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
								(char *)ARG_I, str_vc, "-m", "mac",
								"--mac-source",  srcmacaddr, "-j", policy);
								
						}else if (   MacEntry.srcMac[0]==0 && MacEntry.srcMac[1]==0
			    	  	  		  && MacEntry.srcMac[2]==0 && MacEntry.srcMac[3]==0
		 	                  		  && MacEntry.srcMac[4]==0 && MacEntry.srcMac[5]==0 ) { 
		 	                  		  		
							// iptables -A macfilter -i vc0  -m mac --mac-dst $MAC -j ACCEPT/DROP
							va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
								(char *)ARG_I, str_vc, "-m", "mac",
								"--mac-dst",  dstmacaddr, "-j", policy);
								
						}else {
							// iptables -A macfilter -i vc0  -m mac --mac-source $MAC --mac-dst $MAC -j ACCEPT/DROP
							va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
								(char *)ARG_I, str_vc, "-m", "mac",
								"--mac-source",  srcmacaddr, "--mac-dst",  dstmacaddr, "-j", policy);
						}	
						
							
					}
				}
			}
		}
		
		// Commented by Mason Yu
		//if (total >= 1) {
			// default action
			if (mib_get(MIB_MACF_OUT_ACTION, (void *)value) != 0)
			{
				vInt = (int)(*(unsigned char *)value);
				if (vInt == 0 && eth_mac_ctrl )
				{
					// iptables -A macfilter -i eth0 -j DROP
					va_cmd(IPTABLES, 6, 1, (char *)FW_ADD,
						(char *)FW_MACFILTER, (char *)ARG_I,
						(char *)ELANIF, "-j", (char *)FW_DROP);
				}
				if (vInt == 0 && wlan_mac_ctrl )
				{
					// iptables -A macfilter -i wlan0 -j DROP
					va_cmd(IPTABLES, 6, 1, (char *)FW_ADD,
						(char *)FW_MACFILTER, (char *)ARG_I,
						(char *)WLANIF, "-j", (char *)FW_DROP);
				}
#ifdef CONFIG_USB_ETH
				if (vInt == 0)
				{
					// iptables -A macfilter -i usb0 -j DROP
					va_cmd(IPTABLES, 6, 1, (char *)FW_ADD,
						(char *)FW_MACFILTER, (char *)ARG_I,
						(char *)USBETHIF, "-j", (char *)FW_DROP);
				}
#endif //CONFIG_USB_ETH
			}
			
			if (mib_get(MIB_MACF_IN_ACTION, (void *)value) != 0)
			{
				vInt = (int)(*(unsigned char *)value);
				if (vInt == 0)	// DROP
				{
					for (vInt=0; vInt<vcNum; vInt++) {
						if (!mib_chain_get(MIB_ATM_VC_TBL, vInt, (void *)&Entry))
							continue;
						if (Entry.enable == 0)
							continue;
						
						if (Entry.cmode == ADSL_BR1483) {
							// vc index should be less than 10
							str_vc[2] = '0'+VC_INDEX(Entry.ifIndex);
							// iptables -A macfilter -i vc0 -j DROP
							va_cmd(IPTABLES, 6, 1, (char *)FW_ADD,
								(char *)FW_MACFILTER, (char *)ARG_I,
								str_vc, "-j", (char *)FW_DROP);
						}
					}
				}
			}
			// iptables -A FORWARD -j macfilter
			//va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", (char *)FW_MACFILTER);
		// Commented by Mason Yu
		//}
	}
	printf("\nRestart macfilter!\n");
	return 0;
}

int apply_MACFILTER( int action_type, int id, void *olddata )
{
	switch( action_type )
	{
	case CWMP_START:
	case CWMP_RESTART:
	case CWMP_STOP:
		restart_macfilter();
		break;
	default:
		return -1;
	}
	return 0;

}

#endif

int apply_DDNS( int action_type, int id, void *olddata )
{
	switch( action_type )
	{
	case CWMP_START:
	case CWMP_RESTART:
	case CWMP_STOP:
		restart_ddns();
		break;
	default:
		return -1;
	}
	return 0;

}

#endif //_CWMP_APPLY_

#ifdef _PRMT_TR143_
const char gUDPEchoServerName[]="/bin/udpechoserver";
const char gUDPEchoServerPid[] = "/var/run/udpechoserver.pid";

void UDPEchoConfigSave(struct TR143_UDPEchoConfig *p)
{	
	if(p)
	{
		unsigned char itftype;
		mib_get( TR143_UDPECHO_ENABLE, (void *)&p->Enable );
		mib_get( TR143_UDPECHO_SRCIP, (void *)p->SourceIPAddress );
		mib_get( TR143_UDPECHO_PORT, (void *)&p->UDPPort );
		mib_get( TR143_UDPECHO_PLUS, (void *)&p->EchoPlusEnabled );
		
		mib_get( TR143_UDPECHO_ITFTYPE, (void *)&itftype );
		if(itftype==ITF_WAN)
		{
			int total,i;
			MIB_CE_ATM_VC_T *pEntry, vc_entity;
			
			p->Interface[0]=0;
			total = mib_chain_total(MIB_ATM_VC_TBL);
			for( i=0; i<total; i++ )
			{
				pEntry = &vc_entity;
				if( !mib_chain_get(MIB_ATM_VC_TBL, i, (void*)pEntry ) )
					continue;
				if(pEntry->TR143UDPEchoItf)
				{
					if(pEntry->cmode == ADSL_PPPoE || pEntry->cmode == ADSL_PPPoA)
						sprintf( p->Interface, "ppp%u", PPP_INDEX(pEntry->ifIndex)  );
					else
						sprintf( p->Interface, "vc%u", VC_INDEX(pEntry->ifIndex)  );
				}
			}
		}else if(itftype<ITF_END)
		{
			strcpy( p->Interface, strItf[itftype] );
			LANDEVNAME2BR0(p->Interface);
		}else
			p->Interface[0]=0;
		
	}
	return;	
}

int UDPEchoConfigStart( struct TR143_UDPEchoConfig *p )
{
	if(!p) return -1;

	if( p->Enable )
	{
		char strPort[16], strAddr[32];
		char *argv[10];
		int  i;
		
		if(p->UDPPort==0)
		{ 
			fprintf( stderr, "UDPEchoConfigStart> error p->UDPPort=0\n" );
			return -1;
		}
		sprintf( strPort, "%u", p->UDPPort );
		va_cmd(IPTABLES, 15, 1, ARG_T, "mangle", FW_ADD, (char *)FW_PREROUTING,
			(char *)ARG_I, "!", (char *)LANIF, "-p", (char *)ARG_UDP,
			(char *)FW_DPORT, strPort, "-j", (char *)"MARK", "--set-mark", RMACC_MARK);

		
		i=0;
		argv[i]=gUDPEchoServerName;
		i++;
		argv[i]="-port";
		i++;
		argv[i]=strPort;
		i++;
		if( strlen(p->Interface) > 0 )
		{
			argv[i]="-i";
			i++;
			argv[i]=p->Interface;
			i++;
		}
		if( p->SourceIPAddress[0]!=0 ||
			p->SourceIPAddress[1]!=0 ||
			p->SourceIPAddress[2]!=0 ||
			p->SourceIPAddress[3]!=0  )
		{
			struct in_addr *pSIP = p->SourceIPAddress; 
			argv[i]="-addr";
			i++;
			sprintf( strAddr, "%s", inet_ntoa( *pSIP ) );
			argv[i]=strAddr;
			i++;			
		}
		if( p->EchoPlusEnabled )
		{
			argv[i]="-plus";
			i++;			
		}

		argv[i]=NULL;
		do_cmd( gUDPEchoServerName, argv, 0 );

	}

	return 0;
}

int UDPEchoConfigStop( struct TR143_UDPEchoConfig *p )
{
	char strPort[16];
	int pid;
	int status;

	if(!p) return -1;

	sprintf( strPort, "%u", p->UDPPort );
	va_cmd(IPTABLES, 15, 1, ARG_T, "mangle", FW_DEL, (char *)FW_PREROUTING,
		(char *)ARG_I, "!", (char *)LANIF, "-p", (char *)ARG_UDP,
		(char *)FW_DPORT, strPort, "-j", (char *)"MARK", "--set-mark", RMACC_MARK);
		

	pid = read_pid((char *)gUDPEchoServerPid);
	if (pid >= 1) 
	{
		status = kill(pid, SIGTERM);
		if (status != 0) 
		{
			printf("Could not kill UDPEchoServer's pid '%d'\n", pid);
			return -1;
		}
	}

	return 0;
}

int apply_UDPEchoConfig( int action_type, int id, void *olddata )
{
	struct TR143_UDPEchoConfig newdata;
	int ret=0;
	
	UDPEchoConfigSave(&newdata);
	switch( action_type )
	{
	case CWMP_RESTART:
		if(olddata) UDPEchoConfigStop(olddata);
	case CWMP_START:
		UDPEchoConfigStart(&newdata);
		break;		
	case CWMP_STOP:
		if(olddata) UDPEchoConfigStop(olddata);
		break;
	default:
		ret=-1;
	}
	
	return ret;
}
#endif //_PRMT_TR143_

#ifdef CONFIG_CTC_E8_CLIENT_LIMIT
#define PROCFS_NAME    "/proc/ClientsMonitor"
int proc_write_for_mwband()
{	
	FILE *fp;
	unsigned int vInt;
	unsigned char Value;
	unsigned char curbuffer[1024];
	unsigned char *curptr = curbuffer;
	int len =0;

	if ((fp = fopen(PROCFS_NAME, "w")) == NULL)
	{
		printf("Open file %s failed !\n", PROCFS_NAME);
		return -1;
	}

	memset(curbuffer, 0, 1024);

	len=sprintf(curptr, "flagLimitOnAll "); // set ip to detect...
	 if(len < 0)
	  goto err;
	 curptr+=len;
	 mib_get( CWMP_CT_MWBAND_MODE, (void *)&vInt);
//	 printf("\nflagLimitOnAll=%d\n",vInt);
	 len=sprintf(curptr, "%d\n", vInt);            
	 curptr+=len;
	 
	 len=sprintf(curptr, "limitOnAll "); // set ip to detect...
	 if(len < 0)
	  goto err;
	 curptr+=len;
	 mib_get( CWMP_CT_MWBAND_NUMBER, (void *)&vInt);
//	 printf("\nlimitOnAll=%d\n",vInt);
	 len=sprintf(curptr, "%d\n", vInt);
	 curptr+=len;
	 
	 len=sprintf(curptr, "computerLimitEnable "); // set ip to detect...
	 if(len < 0)
	  goto err;
	 curptr+=len;
	 mib_get( CWMP_CT_MWBAND_PC_ENABLE, (void *)&Value);
//	 printf("\ncomputerLimitEnable=%d\n",Value);
	 len=sprintf(curptr, "%d\n", Value);
	 curptr+=len;
	 
	 len=sprintf(curptr, "limitOnComputer "); // set ip to detect...
	 if(len < 0)
	  goto err;
	 curptr+=len;
	 mib_get( CWMP_CT_MWBAND_PC_NUM, (void *)&vInt);
//	 printf("\nlimitOnComputer=%d\n",vInt);
	 len=sprintf(curptr, "%d\n", vInt);
	 curptr+=len;
	 
	 len=sprintf(curptr, "cameraLimitEnable "); // set ip to detect...
	 if(len < 0)
	  goto err;
	 curptr+=len;
	 mib_get( CWMP_CT_MWBAND_CMR_ENABLE, (void *)&Value);
//	 printf("\ncameraLimitEnable=%d\n",Value);
	 len=sprintf(curptr, "%d\n", Value);
	 curptr+=len;
	 
	 len=sprintf(curptr, "limitOnCamera "); // set ip to detect...
	 if(len < 0)
	  goto err;
	 curptr+=len;
	 mib_get( CWMP_CT_MWBAND_CMR_NUM, (void *)&vInt);
//	 printf("\nlimitOnCamera=%d\n",vInt);
	 len=sprintf(curptr, "%d\n", vInt);
	 curptr+=len;

	fwrite(curbuffer, 1024, 1, fp);
	fclose(fp);
	return 0;
err:
	fclose(fp);
	return -1;

}

#endif //CONFIG_CTC_E8_CLIENT_LIMIT
#endif //_CWMP_MIB_

#define DDNSPID "/var/run/ddns.pid"
int restart_ddns()
{
	int ddnspid=0;

	ddnspid = read_pid((char*)DDNSPID);

	printf("\nddnspid=%d\n",ddnspid);

	if(ddnspid > 0)
		kill(ddnspid, SIGUSR1);

	return 0;
}

int getDisplayWanName(MIB_CE_ATM_VC_T *pEntry, char* name)
{
	if(pEntry==NULL || name==NULL)
		return 0;
	
	if (pEntry->cmode == ADSL_PPPoA) { // ppp0, ...
		snprintf(name, 6, "ppp%u", PPP_INDEX(pEntry->ifIndex));
	}
	else if (pEntry->cmode == ADSL_PPPoE) { // ppp0_vc0, ...
		snprintf(name, 16, "ppp%u_vc%u", PPP_INDEX(pEntry->ifIndex), VC_INDEX(pEntry->ifIndex));
	}
	else { // vc0 ...
		snprintf(name, 5, "vc%u", VC_INDEX(pEntry->ifIndex));
	}
	return 1;
}

//jim for china telecom auto-gen wan name.
#ifdef CTC_WAN_NAME

//jim: to get wan MIB by index... this index is combined index for ppp or vc...
int getWanEntrybyindex(MIB_CE_ATM_VC_T *pEntry, unsigned char ifIndex)
{
	if(pEntry==NULL)
		return -1;
	int mibtotal,i,num=0,totalnum=0;
	MIB_CE_ATM_VC_T Entry;
	mibtotal = mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0;i<mibtotal;i++)
	{
		if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			continue;
		if(Entry.ifIndex == ifIndex)
			break;
	}
	if(i==mibtotal)
		return -1;
	memcpy(pEntry, &Entry, sizeof(Entry));
	return 0;
}

//star: to get ppp index for wanname
int getpppindex(MIB_CE_ATM_VC_T *pEntry)
{
	int ret=-1;
	int mibtotal,i,num=0,totalnum=0;
	unsigned char pppindex,tmpindex;
	MIB_CE_ATM_VC_T Entry;

	if(pEntry->cmode != ADSL_PPPoE && pEntry->cmode != ADSL_PPPoA)
		return ret;
	
	pppindex = (pEntry->ifIndex >> 4) & 0x0f;
	if(pppindex == 0x0f)
		return ret;

	mibtotal = mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0;i<mibtotal;i++)
	{
		if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			continue;
		if(Entry.cmode != ADSL_PPPoE && Entry.cmode != ADSL_PPPoA)
			continue;
		tmpindex = (Entry.ifIndex >> 4) & 0x0f;
		if(tmpindex == 0x0f)
			continue;
		if(Entry.vpi == pEntry->vpi && Entry.vci == pEntry->vci)
		{
			totalnum++;
			if(tmpindex < pppindex)
				num++;
		}
	}

	if(totalnum > 1)
		ret = num;

	return ret;

}

int generateWanName(MIB_CE_ATM_VC_T *entry, char* wanname)
{
	char vpistr[6];
	char vcistr[6];
	
	if(entry==NULL || wanname ==NULL)
		return -1;
	memset(vpistr, 0, sizeof(vpistr));
	memset(vcistr, 0, sizeof(vcistr));
	switch(entry->applicationtype)
	{
		//Internet
		case 0:
			strcpy(wanname, "Internet_");
			break;
		//TR069_Internet
		case 1:
			strcpy(wanname, "TR069_Internet_");
			break;
			//TR069
		case 2:
			strcpy(wanname, "TR069_");
			break;
		//Others
		case 3:
			strcpy(wanname, "Other_");
			break;
		default:
			strcpy(wanname, "Internet_");
			break;
	}	
	if(entry->cmode == ADSL_BR1483)
		strcat(wanname, "B_");
	else
		strcat(wanname, "R_");
	sprintf(vpistr, "%d", entry->vpi);
	sprintf(vcistr, "%d", entry->vci);
	strcat(wanname, vpistr);
	strcat(wanname, "_");
	strcat(wanname, vcistr);
	//star: for multi-ppp in one pvc
	if(entry->cmode == ADSL_PPPoE || entry->cmode == ADSL_PPPoA)
	{	
			char pppindex[6];
			int intindex;
			intindex = getpppindex(entry);
			if(intindex != -1){
					snprintf(pppindex,6,"%u",intindex);
					strcat(wanname, "_");
					strcat(wanname, pppindex);
			}
	}
	
	return 0;
}

int setWanName(char* str, int applicationtype)
{
	switch(applicationtype)
	{
		//Internet
		case 0:
			strcpy(str, "Internet_");
			break;
		//TR069_Internet
		case 1:
			strcpy(str, "TR069_Internet_");
			break;
			//TR069
		case 2:
			strcpy(str, "TR069_");
			break;
		//Others
		case 3:
			strcpy(str, "Other_");
			break;
		default:
			strcpy(str, "Internet_");
			break;
	}	
}

int getWanName(MIB_CE_ATM_VC_T *pEntry, char* name)
{
	if(pEntry==NULL || name==NULL)
		return 0;
#ifdef _CWMP_MIB_	
	if(*(pEntry->WanName))
		strcpy(name, pEntry->WanName);
	else		
#endif		
	{//if not set by ACS. then generate automaticly.  
		char wanname[40];
		memset(wanname, 0, sizeof(wanname));
		generateWanName(pEntry, wanname);
		strcpy(name, wanname);
	}
	return 1;
}


#if 0
int updateWanName(int recordNum)
{
		MIB_CE_ATM_VC_T Entry;
		char vpistr[6];
		char vcistr[6];
		char interfacename[MAX_NAME_LEN];
		memset(vpistr, 0, sizeof(vpistr));
		memset(vcistr, 0, sizeof(vcistr));		
		memset(interfacename, 0, sizeof(interfacename));
			//first found the mib info...
	
		
		if (!mib_chain_get(MIB_ATM_VC_TBL, recordNum, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}
		sprintf(vpistr, "%d", Entry.vpi);
		sprintf(vcistr, "%d", Entry.vci);
		setWanName(interfacename, Entry.applicationtype);
		if(Entry.cmode == ADSL_BR1483)
			strcat(interfacename, "B_");
		else
			strcat(interfacename, "R_");
		strcat(interfacename, vpistr);
		strcat(interfacename, "_");
		strcat(interfacename, vcistr);
		strcpy(Entry.WanName, interfacename);
		mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, recordNum);		
		
}
int updateAllWanNames()
{
		MIB_CE_ATM_VC_T Entry;
		int entryNum;
		int mibcnt;
		char vpistr[6];
		char vcistr[6];
		char interfacename[MAX_NAME_LEN];
		entryNum = mib_chain_total(MIB_ATM_VC_TBL);
		memset(vpistr, 0, sizeof(vpistr));
		memset(vcistr, 0, sizeof(vcistr));		
		memset(interfacename, 0, sizeof(interfacename));
		printf("we just always enable Wan Name auto generated!\n");
			//first found the mib info...
		for(mibcnt=0; mibcnt<entryNum; mibcnt++)
		{
			updateWanName(mibcnt);
			/*
			if (!mib_chain_get(MIB_ATM_VC_TBL, mibcnt, (void *)&Entry))
			{
  				printf("Get chain record error!\n");
				return -1;
			}
			sprintf(vpistr, "%d", Entry.vpi);
			sprintf(vcistr, "%d", Entry.vci);
			setWanName(interfacename, Entry.applicationtype);
			if(Entry.cmode == ADSL_BR1483)
			
				strcat(interfacename, "B_");
			else
				strcat(interfacename, "R_");
			strcat(interfacename, vpistr);
			strcat(interfacename, "_");
			strcat(interfacename, vcistr);
			strcpy(Entry.WanName, interfacename);
			mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, mibcnt);		*/
		}
	
}
#endif //if 0
#endif


#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	//jim used for save/restore config files to distinguish bridge or router.
unsigned int ZTE_Bridge_Config_Keyword=0xB606FBE5;
unsigned int ZTE_Router_Config_Keyword=0xA606EAD5;
void ZTE_Encrypt(char * original, char *encrypted, char initial, int len)
{
	int i;
	//*encrypted=(initial + *original) & 0xFF;
	for(i=0/*1*/; i<len; i++)
	{
		//*(encrypted+i)=*( encrypted+i-1) + *(original+i)) & 0xFF;
		*(encrypted+i)=*(original+i)^0xFF;
	}
return;
} 


void ZTE_Deencrypt(char * original, char *encrypted, char initial, int len)
{
	int i;
	char previous_encrypted_byte;
	previous_encrypted_byte=*encrypted;
	//*original =(256＋*encrypted -initial) & 0xFF;
	for(i=0/*1*/; i<len; i++)
	{
		//char dup_previous_encrypted_byte;
		//dup_previous_encrypted_byte=*(encrypted+i);
		//*(original+i) =(256+dup_previous_encrypted_byte-previous_encrypted_byte) & 0xFF;
		//previous_encrypted_byte= dup_previous_encrypted_byte;
		*(original+i)=*(encrypted+i)^0xFF;
	}
return;
}

#endif

//#ifdef ZTE_LED_REQUEST
#include <linux/atmdev.h>

struct arg{
	unsigned char cmd;
	unsigned int cmd2;
	unsigned int cmd3;
	unsigned int cmd4;
}pass_arg;

#define SIOCETHTEST	0x89a1

//star: for ZTE internet LED request
void Internet_status()
{
#ifdef EMBED
	int skfd, i;
	struct atmif_sioc mysio;
	struct SAR_IOCTL_CFG cfg;
	struct ch_stat stat;
	int mibcnt,entryNum;
	int internet_flag=0;
	int adslflag;
	Modem_LinkSpeed vLs;
	MIB_CE_ATM_VC_T Entry;

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	
	// check for xDSL link
	if (!adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE) || vLs.upstreamRate == 0)
		adslflag = 0;
	else
		adslflag = 1;

	// pvc statistics
	if((skfd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0){
		perror("socket open error");
		return;
	}
	
	mysio.number = 0;

	for (i=0; i < MAX_VC_NUM; i++)
	{
		
		cfg.ch_no = i;
		mysio.arg = (void *)&cfg;
		if(ioctl(skfd, ATM_SAR_GETSTAT, &mysio)<0)
		{
			(void)close(skfd);
			return;
		}

		if (cfg.created == 0)
			continue;	
		
		//check mib chain to see the channel mode of the current channel.
		char ifname[10];
		int flags;
		struct in_addr inAddr;
		for(mibcnt=0; mibcnt<entryNum; mibcnt++)
		{
			if (!mib_chain_get(MIB_ATM_VC_TBL, mibcnt, (void *)&Entry))
			{
				printf("get MIB chain error\n");
				continue;
			}
			
			if(Entry.enable==0)
				continue;
			
			if(Entry.vpi==cfg.vpi && Entry.vci==cfg.vci)
			{
				//found entry in mibs, get channel mode
				cfg.rfc = Entry.cmode;
				{
					if(cfg.rfc != ADSL_BR1483)
					{ 
						if (PPP_INDEX(Entry.ifIndex) != 0x0f)
						{	// PPP interface
							snprintf(ifname, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));
						}
						else
						{	// vc interface
							snprintf(ifname, 6, "vc%u", VC_INDEX(Entry.ifIndex));
						}
						if (getInFlags(ifname, &flags) == 1)
						{
							if (flags & IFF_UP) {
								if (adslflag) {
										if (getInAddr( ifname, IP_ADDR, (void *)&inAddr) == 1)
										{
	//										printf("\nvpi=%d,vci=%d,inAddr=%x,ifname=%s\n",Entry.vpi,Entry.vci,inAddr.s_addr,ifname);
											internet_flag = 1;
											break;
										}
								}
							}
						}
					}
				}
			}
		}
		if(internet_flag == 1)
			break;
	}
	
	(void)close(skfd);
	
 	int fd=0;    
	struct ifreq		ifr1;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	  if(fd< 0){
		printf("LED control fail!\n");
		return;
	  }
	strncpy(ifr1.ifr_name, "eth0", IFNAMSIZ);
	ifr1.ifr_data = (void *)&pass_arg;
	if(internet_flag == 1)
		pass_arg.cmd = 15;
	else
		pass_arg.cmd = 16;
	pass_arg.cmd2 = 0;
	pass_arg.cmd3 = 0;
	pass_arg.cmd4 = 0;
	ioctl(fd, SIOCETHTEST, &ifr1);

	close(fd);

	return;
#endif	
}
//#endif

#ifdef QSETUP_WEB_REDIRECT
int reset_qsetup_redirect() {
	MIB_CE_ATM_VC_T Entry;
	int vcTotal, i,  redirectNum = 0;
//	unsigned char iptables_flag = 0;
	int iptables_flag = 0;
	FILE *fp;

	vcTotal = mib_chain_total(MIB_ATM_VC_TBL);

	for (i = 0; i < vcTotal; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			return -1;
		if (Entry.enable == 0)
			continue;
		if ((REMOTE_ACCESS_T)Entry.cmode == ACC_PPPOE || (REMOTE_ACCESS_T)Entry.cmode == ACC_PPPOA)
			redirectNum ++;
	}

//	mib_get(MIB_QSETUP_REDIRECT, (void *)&iptables_flag);
	if ((fp = fopen("/tmp/QSetup", "r"))) {
		fscanf(fp, "%d\n", &iptables_flag);
		fclose(fp);
	}
	printf("reset_qsetup_redirect: startRedirect=%d\n", iptables_flag);
	if (redirectNum > 0) {
		struct data_to_pass_st msg;
		stop_qsetup_redirect();
		start_qsetup_redirect();
		snprintf(msg.data, BUF_SIZE, "spppctl show 0");
		TRACE(STA_SCRIPT, "%s\n", msg.data);
		write_to_pppd(&msg);
	}
	else if (redirectNum == 0 && iptables_flag)
		stop_qsetup_redirect();
	return 1;
}

int start_qsetup_redirect() {

	int status=0;
	char tmpbuf[6], ipaddr[16];
//	unsigned char startRedirect;	
	int startRedirect = 0;
	FILE *fp;
	
//	mib_get(MIB_QSETUP_REDIRECT, (void *)&startRedirect);
	if ((fp = fopen("/tmp/QSetup", "r"))) {
		fscanf(fp, "%d\n", &startRedirect);
		fclose(fp);
	}
	if (startRedirect)
		return status;

	ipaddr[0] = '\0';
	if (mib_get(MIB_ADSL_LAN_IP, (void *)tmpbuf) != 0) {
		strncpy(ipaddr, inet_ntoa(*((struct in_addr *)tmpbuf)), 16);
		ipaddr[15] = '\0';
	}

	/*
	startRedirect = 1;
	if (!mib_set(MIB_QSETUP_REDIRECT, (void *)&startRedirect)) {
		printf("Set quick setup redirect flag error!");
		return status;
	}
	*/

	//iptables -t nat -A PREROUTING -d ! 192.168.1.1 -p tcp --dport 80 -j REDIRECT --to-port 2046
	//status|=va_cmd(IPTABLES, 15, 1, (char *)ARG_T, "nat", (char *)FW_ADD, (char *)FW_PREROUTING,
	//	"-d", "!", ipaddr, "-p", (char *)ARG_TCP, (char *)FW_DPORT, "80", "-j", "REDIRECT", "--to-port", "2046");
//	printf("start_qsetup_redirect: startRedirect=%d\n", startRedirect);
	//iptables -t nat -N QSetup_Redirect
	status|=va_cmd(IPTABLES, 4, 1, (char *)ARG_T, "nat","-N","QSetup_Redirect");

	//iptables -t nat -A QSetup_Redirect -d 192.168.1.1 -j RETURN
	status|=va_cmd(IPTABLES, 8, 1, (char *)ARG_T, "nat", (char *)FW_ADD, "QSetup_Redirect",
		"-d", ipaddr, "-j", (char *)FW_RETURN);

	//iptables -t nat -A QSetup_Redirect -p tcp --dport 80 -j DNAT --to-destination 192.168.1.1:2046 
	//status|=va_cmd(IPTABLES, 12, 1, (char *)ARG_T, "nat", (char *)FW_ADD, "QSetup_Redirect",
	//	"-p", (char *)ARG_TCP, (char *)FW_DPORT, "80", "-j", "DNAT", "--to-destination", ip_port);
	//iptables -t nat -A QSetup_Redirect -p tcp --dport 80 -j REDIRECT --to-port  2046 
	status|=va_cmd(IPTABLES, 12, 1, (char *)ARG_T, "nat", (char *)FW_ADD, "QSetup_Redirect",
		"-p", (char *)ARG_TCP, (char *)FW_DPORT, "80", "-j", "REDIRECT", "--to-port", "2046");

	//iptables -t nat -A PREROUTING -p tcp --dport 80 -j QSetup_Redirect
	status|=va_cmd(IPTABLES, 10, 1, (char *)ARG_T, "nat", (char *)FW_ADD, (char *)FW_PREROUTING,
		"-p", (char *)ARG_TCP, (char *)FW_DPORT, "80", "-j", "QSetup_Redirect");

	startRedirect = 1;
	fp = fopen("/tmp/QSetup", "w");
	fprintf(fp, "%d", startRedirect);
	fclose(fp);

	printf("start_qsetup_redirect: startRedirect=%d\n", startRedirect);

	return status;	
}

int stop_qsetup_redirect() {

	int status=0;
	char tmpbuf[6], ipaddr[16];
//	unsigned char startRedirect;	
	int startRedirect = 0;
	FILE *fp;
	
//	mib_get(MIB_QSETUP_REDIRECT, (void *)&startRedirect);
	if ((fp = fopen("/tmp/QSetup", "r"))) {
		fscanf(fp, "%d\n", &startRedirect);
		fclose(fp);
	}
	if (!startRedirect)
		return status;

	ipaddr[0] = '\0';
	if (mib_get(MIB_ADSL_LAN_IP, (void *)tmpbuf) != 0) {
		strncpy(ipaddr, inet_ntoa(*((struct in_addr *)tmpbuf)), 16);
		ipaddr[15] = '\0';
	}

	/*
	startRedirect = 0;
	if (!mib_set(MIB_QSETUP_REDIRECT, (void *)&startRedirect)) {
		printf("Set quick setup redirect flag error!");
		return status;
	}
	*/

	//iptables -t nat -D PREROUTING -d ! 192.168.1.1 -p tcp --dport 80 -j REDIRECT --to-port 2046
	//status|=va_cmd(IPTABLES, 15, 1, (char *)ARG_T, "nat", (char *)FW_DEL, (char *)FW_PREROUTING,
	//	"-d", "!", ipaddr, "-p", (char *)ARG_TCP, (char *)FW_DPORT, "80", "-j", "REDIRECT", "--to-port", "2046");
//	printf("stop_qsetup_redirect: startRedirect=%d\n", startRedirect);
	//iptables -t nat -D PREROUTING -p tcp --dport 80 -j QSetup_Redirect
	status|=va_cmd(IPTABLES, 10, 1, (char *)ARG_T, "nat", (char *)FW_DEL, (char *)FW_PREROUTING,
		"-p", (char *)ARG_TCP, (char *)FW_DPORT, "80", "-j", "QSetup_Redirect");

	//iptables -t nat -F QSetup_Redirect
	status|=va_cmd(IPTABLES, 4, 1, (char *)ARG_T, "nat", "-F", "QSetup_Redirect");

	//iptables -t nat -X QSetup_Redirect
	status|=va_cmd(IPTABLES, 4, 1, (char *)ARG_T, "nat","-X","QSetup_Redirect");

	startRedirect = 0;
	fp = fopen("/tmp/QSetup", "w");
	fprintf(fp, "%d", startRedirect);
	fclose(fp);

	printf("stop_qsetup_redirect: startRedirect=%d\n", startRedirect);
//	va_cmd("/bin/ethctl", 2, 1, "conntrack", "killall");
	return status;	
}
#endif

static struct sockaddr_in pingaddr;
static int pingsock = -1;
static long ntransmitted = 0, nreceived = 0, nrepeats = 0;
static int myid = 0;
static int finished = 0;

static int create_icmp_socket(void)
{
	struct protoent *proto;
	int sock;

	proto = getprotobyname("icmp");
	/* if getprotobyname failed, just silently force
	 * proto->p_proto to have the correct value for "icmp" */
	if ((sock = socket(AF_INET, SOCK_RAW,
			(proto ? proto->p_proto : 1))) < 0) {        /* 1 == ICMP */
		printf("cannot create raw socket\n");
	}

	return sock;
}
static int in_cksum(unsigned short *buf, int sz)
{
	int nleft = sz;
	int sum = 0;
	unsigned short *w = buf;
	unsigned short ans = 0;

	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1) {
		*(unsigned char *) (&ans) = *(unsigned char *) w;
		sum += ans;
	}

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	ans = ~sum;
	return (ans);
}

static void pingfinal()
{
	finished = 1;
}
static void sendping()
{
	struct icmp *pkt;
	int c;
	char packet[DEFDATALEN + 8];

	pkt = (struct icmp *) packet;
	pkt->icmp_type = ICMP_ECHO;
	pkt->icmp_code = 0;
	pkt->icmp_cksum = 0;
	pkt->icmp_seq = ntransmitted++;
	pkt->icmp_id = myid;
	pkt->icmp_cksum = in_cksum((unsigned short *) pkt, sizeof(packet));

	c = sendto(pingsock, packet, sizeof(packet), 0,
			   (struct sockaddr *) &pingaddr, sizeof(struct sockaddr_in));
	
	if (c < 0 || c != sizeof(packet)) {
		ntransmitted--;
		finished = 1;
		printf("sock: sendto fail !");
		return;
	}
	
	signal(SIGALRM, sendping);
	if (ntransmitted < PINGCOUNT) {	/* schedule next in 1s */
		alarm(PINGINTERVAL);
	} else {	/* done, wait for the last ping to come back */
		signal(SIGALRM, pingfinal);
		alarm(MAXWAIT);
	}
}

int utilping(char *str){
//	char *submitUrl;
	char tmpBuf[100];
	int c;
	struct hostent *h;
	struct icmp *pkt;
	struct iphdr *iphdr;
	char packet[DEFDATALEN + 8];
	int rcvdseq, ret=0;
	fd_set rset;
	struct timeval tv;
	
	if (str[0]) {
		if ((pingsock = create_icmp_socket()) < 0) {
			perror("socket");
			snprintf(tmpBuf, 100, "ping: socket create error");
			goto setErr_ping;
		}
	
		memset(&pingaddr, 0, sizeof(struct sockaddr_in));
		pingaddr.sin_family = AF_INET;
		
		if ((h = gethostbyname(str)) == NULL) {
			//herror("ping: ");
			//snprintf(tmpBuf, 100, "ping: %s: %s", str, hstrerror(h_errno));
			goto setErr_ping;
		}
		
		if (h->h_addrtype != AF_INET) {
			//strcpy(tmpBuf, T("unknown address type; only AF_INET is currently supported."));
			goto setErr_ping;
		}
	
		memcpy(&pingaddr.sin_addr, h->h_addr, sizeof(pingaddr.sin_addr));

		printf("PING %s (%s): %d data bytes\n",
		   h->h_name,inet_ntoa(*(struct in_addr *) &pingaddr.sin_addr.s_addr),DEFDATALEN);
		
		myid = getpid() & 0xFFFF;
		ntransmitted = nreceived = nrepeats = 0;
		finished = 0;
		rcvdseq=ntransmitted-1;
		FD_ZERO(&rset);
		FD_SET(pingsock, &rset);
		/* start the ping's going ... */
		sendping();
		
		/* listen for replies */
		while (1) {
			struct sockaddr_in from;
			socklen_t fromlen = (socklen_t) sizeof(from);
			int c, hlen, dupflag;
			
			if (finished)
				break;
			
			tv.tv_sec = 1;
			tv.tv_usec = 0;
			
			if (select(pingsock+1, &rset, NULL, NULL, &tv) > 0) {
				if ((c = recvfrom(pingsock, packet, sizeof(packet), 0,
								  (struct sockaddr *) &from, &fromlen)) < 0) {
					
					printf("sock: recvfrom fail !");
					continue;
				}
			}
			else // timeout or error
				continue;
			
			if (c < DEFDATALEN+ICMP_MINLEN)
				continue;
			
			iphdr = (struct iphdr *) packet;
			hlen = iphdr->ihl << 2;
			pkt = (struct icmp *) (packet + hlen);	/* skip ip hdr */
			if (pkt->icmp_id != myid) {
//				printf("not myid\n");
				continue;
			}
			if (pkt->icmp_type == ICMP_ECHOREPLY) {
				++nreceived;
				if (pkt->icmp_seq == rcvdseq) {
					// duplicate
					++nrepeats;
					--nreceived;
					dupflag = 1;
				} else {
					rcvdseq = pkt->icmp_seq;
					dupflag = 0;
					if (nreceived < PINGCOUNT)
					// reply received, send another immediately
						sendping();
				}
				printf("%d bytes from %s: icmp_seq=%u", c,
					   inet_ntoa(*(struct in_addr *) &from.sin_addr.s_addr),
					   pkt->icmp_seq);
				if (dupflag) {
					printf(" (DUP!)");
				}
				printf("\n");
			}
			// if recived though one packet return OK
			if (nreceived > 0) {
			    ret = 1;
				// if recived all packets break this
				if (nreceived >= PINGCOUNT) {
					break;
				}
			}
		}
		FD_CLR(pingsock, &rset);
		close(pingsock);
		pingsock = -1;
	}
	printf("\n--- ping statistics ---\n");
	printf("%ld packets transmitted, ", ntransmitted);
	printf("%ld packets received\n\n", nreceived);
	if (nrepeats)
		printf("%ld duplicates, ", nrepeats);
	printf("\n");
	return ret;
setErr_ping:
	if (pingsock >= 0) {
		close(pingsock);
		pingsock = -1;
	}
	printf("Ping error!!\n\n");
	return ret;
}

void dlbTimeout()
{
	finished = 1;
}

int testOAMLookback(MIB_CE_ATM_VC_Tp pEntry, unsigned char scope, unsigned char type)
{
#ifdef EMBED
	int skfd;
	struct atmif_sioc mysio;
	ATMOAMLBReq lbReq;
	ATMOAMLBState lbState;
	
	if((skfd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0){
		perror("socket open error");
		return -1;
	}
	
	memset(&lbReq, 0, sizeof(ATMOAMLBReq));
	memset(&lbState, 0, sizeof(ATMOAMLBState));
	lbReq.Scope = scope;
	lbReq.vpi = pEntry->vpi;
	if (type == 5)
		lbReq.vci = pEntry->vci;
	else if (type == 4) {
		if (scope == 0)	// Segment
			lbReq.vci = 3;
		else if (scope == 1)	// End-to-end
			lbReq.vci = 4;
	}
	memset(lbReq.LocID, 0xff, 16);	// Loopback Location ID
	mysio.number = 0;	// ATM interface number
	mysio.arg = (void *)&lbReq;
	// Start the loopback test
	if (ioctl(skfd, ATM_OAM_LB_START, &mysio)<0) {
		perror("ioctl: ATM_OAM_LB_START failed !");
		close(skfd);
		return -1;
	}
	
	finished = 0;
	signal(SIGALRM, dlbTimeout);
	alarm(MAXWAIT);
	// Query the loopback status
	mysio.arg = (void *)&lbState;
	lbState.vpi = pEntry->vpi;
	if (type == 5)
		lbState.vci = pEntry->vci;
	else if (type == 4) {
		if (scope == 0)	// Segment
			lbState.vci = 3;
		else if (scope == 1)	// End-to-end
			lbState.vci = 4;
	}
	lbState.Tag = lbReq.Tag;
	
	while (1)
	{
		if (finished)
			break;	// break for timeout
		
		if (ioctl(skfd, ATM_OAM_LB_STATUS, &mysio)<0) {
			perror("ioctl: ATM_OAM_LB_STATUS failed !");
			mysio.arg = (void *)&lbReq;
			ioctl(skfd, ATM_OAM_LB_STOP, &mysio);
			close(skfd);
			return -1;
		}
		
		if (lbState.count[0] > 0)
		{
			break;	// break for loopback success
		}
	}
		
	mysio.arg = (void *)&lbReq;
	// Stop the loopback test
	if (ioctl(skfd, ATM_OAM_LB_STOP, &mysio)<0) {
		perror("ioctl: ATM_OAM_LB_STOP failed !");
		close(skfd);
		return -1;
	}
	close(skfd);
	
	if (!finished) {
		printf("\n--- Loopback cell received successfully ---\n");
		return 1;	// successful
	}
	else {
		printf("\n--- Loopback failed ---\n");
		return 0;	// failed
	}
#else
	return 1;
#endif
}

int defaultGWAddr(char *gwaddr)
{
	char buff[256], ifname[16];
	int flgs;
	//unsigned long int g;
	//struct in_addr gw;
	struct in_addr gw, dest, mask;
	FILE *fp;
	
	if (!(fp=fopen("/proc/net/route", "r"))) {
		printf("Error: cannot open /proc/net/route - continuing...\n");
		return -1;
	}

	fgets(buff, sizeof(buff), fp);
	while (fgets(buff, sizeof(buff), fp) != NULL) {
		//if (sscanf(buff, "%s%lx%*lx%X%", ifname, &g, &flgs) != 3) {
		if (sscanf(buff, "%s%x%x%x%*d%*d%*d%x", ifname, &dest, &gw, &flgs, &mask) != 5) {
			printf("Unsupported kernel route format\n");
			fclose(fp);
			return -1;
		}
		if(flgs & RTF_UP) {
			// default gateway
			if (dest.s_addr == 0 && mask.s_addr == 0) {
				if (gw.s_addr != 0) {
					sprintf(gwaddr, "%s", inet_ntoa(gw));
					fclose(fp);
					return 0;
				}
				else {
					if (getInAddr(ifname, DST_IP_ADDR, (void *)&gw) == 1)
						if (gw.s_addr != 0) {
							sprintf(gwaddr, "%s", inet_ntoa(gw));
							fclose(fp);
							return 0;
						}
				}
			}
		}
		/*if((g == 0) && (flgs & RTF_UP)) {
			if (getInAddr(ifname, DST_IP_ADDR, (void *)&gw) == 1)
				if (gw.s_addr != 0) {
					sprintf(gwaddr, "%s", inet_ntoa(gw));
					fclose(fp);
					return 0;
				}
		}
		if (sscanf(buff, "%*s%*lx%lx%X%", &g, &flgs) != 2) {
			printf("Unsupported kernel route format\n");
			fclose(fp);
			return -1;
		}
		if(flgs & RTF_UP) {
			gw.s_addr = g;
			if (gw.s_addr != 0) {
				sprintf(gwaddr, "%s", inet_ntoa(gw));
				fclose(fp);
				return 0;
			}
		}*/
	}
	fclose(fp);
	return -1;
}

int pdnsAddr(char *dnsaddr) {

	FILE *fp;
	char buff[256];
	if ( (fp = fopen("/var/resolv.conf", "r")) == NULL ) {
		printf("Unable to open resolver file\n");
		return -1;
	}

	fgets(buff, sizeof(buff), fp);
	if (sscanf(buff, "nameserver %s", dnsaddr) != 1) {
		printf("Unsupported kernel route format\n");
		fclose(fp);
		return -1;
	}
	fclose(fp);
	return 0;
}

void writeFlash(void){
	#if 0
	unsigned char no_wlan = 0;
	// wait for sar to finish processing all the queueing packets
	sleep(2);
	itfcfg("sar", 0);
	itfcfg((char *)ELANIF, 0);
#ifdef WLAN_SUPPORT
	mib_get(MIB_WLAN_DISABLED, (void *)&no_wlan);
	if (!no_wlan)
		//itfcfg((char *)WLANIF, 0);
		stopwlan();
#endif
	#endif // if #if 0
	/* upgdate to flash */
	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
	#if 0
	itfcfg("sar", 1);
	itfcfg((char *)ELANIF, 1);
#ifdef WLAN_SUPPORT
	mib_get(MIB_WLAN_DISABLED, (void *)&no_wlan);
	if(!no_wlan) {
		startWLan();
#if 0
		unsigned char encrypt, enable1x = 0;
		int auth_pid_fd = -1, status = -1;
		itfcfg((char *)WLANIF, 1);
		mib_get(MIB_WLAN_ENCRYPT, (void *)&encrypt);
#ifdef WLAN_1x
		mib_get( MIB_WLAN_ENABLE_1X, (void *)&enable1x);
#endif
		sleep(1);		

		if ((encrypt >= 2 || enable1x==1) && vChar == 0) {// 802.1x enabled			
			do {
				status = va_cmd(AUTH, 4, 1, WLANIF, LANIF, "auth", WLAN_AUTH_CONF);
				while ((auth_pid_fd = open(AUTH_PID, O_WRONLY)) == -1)
					usleep(30000);				
				close(auth_pid_fd);
				usleep(2000000);
				status |= va_cmd(IWCONTROL, 1, 1, WLANIF);
			} while (status != 0);
		}
#endif
	}
#endif // of WLAN_SUPPORT
	#endif // of #if 0
	sleep(1);
}

#ifdef ACCOUNT_CONFIG
//Jenny, get user account privilege
int getAccPriv(char *user)
{
	int totalEntry, i;
	MIB_CE_ACCOUNT_CONFIG_T Entry;
	char suName[MAX_NAME_LEN], usName[MAX_NAME_LEN];

	mib_get(MIB_SUSER_NAME, (void *)suName);
	mib_get(MIB_USER_NAME, (void *)usName);
	if (strcmp(suName, user) == 0)
		return (int)PRIV_ROOT;
	else if (strcmp(usName, user) == 0)
		return (int)PRIV_USER;
	totalEntry = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL);
	for (i=0; i<totalEntry; i++)
	{
		if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, i, (void *)&Entry))
			continue;
		if (strcmp(Entry.userName, user) == 0)
			return Entry.privilege;
	}
	return -1;
}
#endif

//add by ramen

void DnsBindPvcRoute(int mode)
{
#ifdef DNS_BIND_PVC_SUPPORT
char *operationMode[2]={"add","del"};
unsigned char DnsBindPvcEnable,DnsBindPvc1;
struct in_addr dnsIP1;
char* argv[7];
int i=0;
mib_get(MIB_DNS_BIND_PVC_ENABLE,(void *)&DnsBindPvcEnable);
if(DnsBindPvcEnable)
{
for(i=0;i<3;i++)
	{
		mib_get(MIB_ADSL_WAN_DNS1+i, (void *)&dnsIP1);	
		mib_get(MIB_DNS_BIND_PVC1+i,&DnsBindPvc1);
		if(dnsIP1.s_addr&&(DnsBindPvc1!=255)){			
			char interfaceName[8]={0};
			char DnsIp[20]={0};
			//route del -host 192.168.2.241 dev vc0
			argv[1]=operationMode[mode];
			argv[2]="-host";
			strcpy(DnsIp,inet_ntoa(dnsIP1));
			printf("dnsip=%s DnsBindPvc1=%d\n",DnsIp,DnsBindPvc1);
			argv[3]=DnsIp;
			argv[4]="dev";
			if(DnsBindPvc1>=0xf0) snprintf(interfaceName,sizeof(interfaceName),"vc%d",VC_INDEX(DnsBindPvc1));
			else snprintf(interfaceName,sizeof(interfaceName),"ppp%d",PPP_INDEX(DnsBindPvc1));
			argv[5]=interfaceName;
			argv[6]=NULL;
			printf("/bin/route %s %s %s %s  %s!\n",argv[1],argv[2],argv[3],argv[4],argv[5]);
			do_cmd("/bin/route",argv,1);
			}
	}
}
#endif
return;
}
//jim support dsl disconnection when firmware upgrade from Local Side.....
//retrun value: 1-local        0 - wan         -1 - error
int isAccessFromLocal(unsigned int ip)
{
	unsigned int uLanIp;
	unsigned int uLanMask;
	char secondIpEn;
	unsigned int uSecondIp;
	unsigned int uSecondMask;

	if (!mib_get( MIB_ADSL_LAN_IP, (void *)&uLanIp ))
		return -1;
	if (!mib_get( MIB_ADSL_LAN_SUBNET, (void *)&uLanMask ))
		return -1;
	
	if ( (ip & uLanMask) == (uLanIp & uLanMask) ) {//in the same subnet with LAN port
		return 1;
	} else {
		if (!mib_get( MIB_ADSL_LAN_ENABLE_IP2, (void *)&secondIpEn ))
			return -1;
		
		if (secondIpEn == 1) {//second IP is enabled
			if (!mib_get( MIB_ADSL_LAN_IP2, (void *)&uSecondIp))
				return -1;
			if (!mib_get( MIB_ADSL_LAN_SUBNET2, (void *)&uSecondMask))
				return -1;

			if ( (ip & uSecondMask) == (uSecondIp & uSecondMask) )//in the same subnet with LAN port
				return 1;
		}
	}
	
	return 0;
}

// Jenny, for checking duplicated destination address
int checkRoute(MIB_CE_IP_ROUTE_T rtEntry, int idx)
{
	//unsigned char destNet[4];
	unsigned long destID, netMask, nextHop;
	unsigned int totalEntry = mib_chain_total(MIB_IP_ROUTE_TBL); /* get chain record size */
	MIB_CE_IP_ROUTE_T Entry;
	int i;

	/*destNet[0] = rtEntry.destID[0] & rtEntry.netMask[0];
	destNet[1] = rtEntry.destID[1] & rtEntry.netMask[1];
	destNet[2] = rtEntry.destID[2] & rtEntry.netMask[2];
	destNet[3] = rtEntry.destID[3] & rtEntry.netMask[3];*/
	destID = *((unsigned long *)&rtEntry.destID);
	netMask = *((unsigned long *)&rtEntry.netMask);
	nextHop = *((unsigned long *)&rtEntry.nextHop);

	// check if route exists
	for (i=0; i<totalEntry; i++) {
		long pdestID, pnetMask, pnextHop;
		unsigned char pdID[4];
		char *temp;
		if (i == idx)
			continue;
		if (!mib_chain_get(MIB_IP_ROUTE_TBL, i, (void *)&Entry))
			return 0;

		pdID[0] = Entry.destID[0] & Entry.netMask[0];
		pdID[1] = Entry.destID[1] & Entry.netMask[1];
		pdID[2] = Entry.destID[2] & Entry.netMask[2];
		pdID[3] = Entry.destID[3] & Entry.netMask[3];
		temp = inet_ntoa(*((struct in_addr *)pdID));
		pdestID = ntohl(inet_addr(temp));
		pnetMask = *((unsigned long *)&Entry.netMask);
		pnextHop = *((unsigned long *)&Entry.nextHop);
		if (pdestID == destID && pnetMask == netMask && pnextHop == nextHop && rtEntry.ifIndex == Entry.ifIndex)
			return 0;
	}
	return 1;
}

int isValidIpAddr(char *ipAddr)
{
	long field[4];

	if (sscanf(ipAddr, "%ld.%ld.%ld.%ld", &field[0], &field[1], &field[2], &field[3]) != 4) 
		return 0;

	if (field[0] < 1 || field[0] > 254 || field[0] == 127 || field[1] < 0 || field[1] > 255 || field[2] < 0 || field[2] > 255 || field[3] < 0 || field[3] > 254)
		return 0;

	if (inet_addr(ipAddr) == -1)
		return 0;
    
	return 1;
}

int isValidHostID(char *ip, char *mask)
{
	long hostIp, netMask, hid, mbit;
	int i, bit, bitcount = 0;

	inet_aton(mask, (struct in_addr *)&netMask);
	hostIp = ntohl(inet_addr(ip));

	hid = ~netMask & hostIp;
	if (hid == 0x0)
		return 0;
	mbit = 0;
	while (1) {
		if (netMask & 0x80000000) {
			mbit++;
			netMask <<= 1;
		}
		else
			break;
	}
	mbit = 32 - mbit;
	for (i=0; i<mbit; i++) {
		bit = hid & 1L;
		if (bit)
			bitcount ++;
		hid >>= 1;
	}
	if (bitcount == mbit)
		return 0;
	return 1;
}

int isValidNetmask(char *mask, int checkbyte)
{
	long netMask;
	int i, bit, isChanged = 0;

	netMask = ntohl(inet_addr(mask));

	// Check most byte (must be 255) and least significant bit (must be 0)
	if (checkbyte) {
		bit = (netMask & 0xFF000000L) >> 24;
		if (bit != 255)
			return 0;
	}

//	bit = netMask & 1L;
//	if (bit)
//		return 0;

	// make sure the bit pattern changes from 0 to 1 only once
	for (i=1; i<31; i++) {
		netMask >>= 1;
		bit = netMask & 1L;

		if (bit) {
			if (!isChanged)
				isChanged = 1;
		}
		else {
			if (isChanged)
				return 0;
		}
	}

	return 1;
}

// check whether an IP address is in the same subnet
int isSameSubnet(char *ipAddr1, char *ipAddr2, char *mask)
{
	long netAddr1, netAddr2, netMask;

	netAddr1 = inet_addr(ipAddr1);
	netAddr2 = inet_addr(ipAddr2);
	netMask = inet_addr(mask);

	if ((netAddr1 & netMask) != (netAddr2 & netMask)) 
		return 0;

	return 1;
}

int isValidMacString(char *MacStr)
{
	int i;
	
	if(strlen(MacStr) != 17){
		return 0;
	}

	for(i=0;i<17;i++){
		if((i+1)%3 == 0){
			if(MacStr[i] != ':')
				return 0;
		}else{
			if(!((MacStr[i] >= '0' && MacStr[i] <= '9') 
				|| (MacStr[i] >= 'a' && MacStr[i] <= 'f') 
				|| (MacStr[i] >= 'A' && MacStr[i] <= 'F'))){
				return 0;
			}
		}
	}
	return 1;
}

int isValidMacAddr(char *macAddr)
{
	// Check for bad, multicast, broadcast, or null address
	if ((macAddr[0] & 1) || (macAddr[0] & macAddr[1] & macAddr[2] & macAddr[3] & macAddr[4] & macAddr[5]) == 0xff
		|| (macAddr[0] | macAddr[1] | macAddr[2] | macAddr[3] | macAddr[4] | macAddr[5]) == 0x00)
		return 0;

	return 1;
}

// check whether an IP address is in the LAN subnet
// return value: -1: error, 0: WAN, 1: LAN
int isLanSubnet(char *ipAddr)
{
	long netAddr, netMask, netIp;

	if (!mib_get(MIB_ADSL_LAN_IP, (void *)&netAddr))
		return -1;
	if (!mib_get(MIB_ADSL_LAN_SUBNET, (void *)&netMask))
		return -1;
	
	netIp = inet_addr(ipAddr);
	if ((netAddr & netMask) != (netIp & netMask)) 
		return 0;

	return 1;
}

#ifdef CONFIG_WIFI_SIMPLE_CONFIG //WPS
// this is a workaround for wscd to get MIB id without including "mib.h" (which causes compile error), andrew

const int gMIB_WSC_CONFIGURED         = MIB_WSC_CONFIGURED;           
const int gMIB_WLAN_SSID              = MIB_WLAN_SSID;
const int gMIB_WSC_SSID               = MIB_WSC_SSID;
const int gMIB_WLAN_AUTH_TYPE         = MIB_WLAN_AUTH_TYPE;
const int gMIB_WLAN_ENCRYPT           = MIB_WLAN_ENCRYPT;
const int gMIB_WSC_AUTH               = MIB_WSC_AUTH;
const int gMIB_WLAN_WPA_AUTH          = MIB_WLAN_WPA_AUTH;
const int gMIB_WLAN_WPA_PSK           = MIB_WLAN_WPA_PSK;
const int gMIB_WLAN_WPA_PSK_FORMAT    = MIB_WLAN_WPA_PSK_FORMAT;
const int gMIB_WSC_PSK                = MIB_WSC_PSK;
const int gMIB_WLAN_WPA_CIPHER_SUITE  = MIB_WLAN_WPA_CIPHER_SUITE;
const int gMIB_WLAN_WPA2_CIPHER_SUITE = MIB_WLAN_WPA2_CIPHER_SUITE;
const int gMIB_WLAN_WEP               = MIB_WLAN_WEP;
const int gMIB_WLAN_WEP64_KEY1        = MIB_WLAN_WEP64_KEY1;
const int gMIB_WLAN_WEP64_KEY2        = MIB_WLAN_WEP64_KEY2;
const int gMIB_WLAN_WEP64_KEY3        = MIB_WLAN_WEP64_KEY3;
const int gMIB_WLAN_WEP64_KEY4        = MIB_WLAN_WEP64_KEY4;
const int gMIB_WLAN_WEP128_KEY1       = MIB_WLAN_WEP128_KEY1;
const int gMIB_WLAN_WEP128_KEY2       = MIB_WLAN_WEP128_KEY2;
const int gMIB_WLAN_WEP128_KEY3       = MIB_WLAN_WEP128_KEY3;
const int gMIB_WLAN_WEP128_KEY4       = MIB_WLAN_WEP128_KEY4;
const int gMIB_WLAN_WEP_DEFAULT_KEY   = MIB_WLAN_WEP_DEFAULT_KEY;
const int gMIB_WLAN_WEP_KEY_TYPE      = MIB_WLAN_WEP_KEY_TYPE;
const int gMIB_WSC_ENC                = MIB_WSC_ENC;
const int gMIB_WSC_CONFIG_BY_EXT_REG  = MIB_WSC_CONFIG_BY_EXT_REG;
const int gMIB_WSC_PIN = MIB_WSC_PIN;
const int gMIB_ELAN_MAC_ADDR = MIB_ELAN_MAC_ADDR;
const int gMIB_WLAN_MODE = MIB_WLAN_MODE;
const int gMIB_WSC_REGISTRAR_ENABLED = MIB_WSC_REGISTRAR_ENABLED;
const int gMIB_WLAN_CHAN_NUM = MIB_WLAN_CHAN_NUM;
const int gMIB_WSC_UPNP_ENABLED = MIB_WSC_UPNP_ENABLED;
const int gMIB_WSC_METHOD = MIB_WSC_METHOD;
const int gMIB_WSC_MANUAL_ENABLED = MIB_WSC_MANUAL_ENABLED;
const int gMIB_SNMP_SYS_NAME = MIB_SNMP_SYS_NAME;
const int gMIB_WLAN_NETWORK_TYPE = MIB_WLAN_NETWORK_TYPE;
#ifdef MIB_WLAN_RESTART_SUPPORT
const int gMIB_WLAN_RESTART = MIB_WLAN_RESTART;
#endif
int mib_update_all()		
{
	return mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
}
#endif
#ifdef QOS_SPEED_LIMIT_SUPPORT
//return -1 --not existed
int mib_qos_speed_limit_existed(int speed,int prior)
{
	int entryTotalNum=mib_chain_total(MIB_QOS_SPEED_LIMIT);
	int i=0;
	MIB_CE_IP_QOS_SPEEDRANK_T entry;
	for(i=0;i<entryTotalNum;i++)
		{
		    mib_chain_get(MIB_QOS_SPEED_LIMIT, i, &entry);
		   if(entry.speed==speed&&entry.prior==prior)
		   	return entry.index;
		}
	return -1;
}
#endif

//Ethernet
#if defined(ELAN_LINK_MODE)
#include <linux/sockios.h>
struct mii_ioctl_data {
	unsigned short	phy_id;
	unsigned short	reg_num;
	unsigned short	val_in;
	unsigned short	val_out;
};
#endif

int restart_ethernet(int instnum)
{
	char vChar=0;
	int status=0;

#ifdef _CWMP_MIB_
#ifdef _CWMP_APPLY_

	//eth0 interface enable or disable
	mib_get(CWMP_LAN_ETHIFENABLE, (void *)&vChar);	
	
	if(vChar==0)
	{
		va_cmd(IFCONFIG, 2, 1, "eth0", "down");
		printf("Disable eth0 interface\n");
		return 0;
	}
	else
	{
#endif
#endif		
		va_cmd(IFCONFIG, 2, 1, "eth0", "up");
		printf("Enable eth0 interface\n");
#if defined(CONFIG_EXT_SWITCH) 
#if defined(ELAN_LINK_MODE) 
		if(instnum != 0)  //from the setupLinkMode() in startup.c
		{
			int skfd;
			struct ifreq ifr;
			struct mii_ioctl_data *mii = (struct mii_data *)&ifr.ifr_data;
			MIB_CE_SW_PORT_T Port;
			int i, k, total;
			
	
			strcpy(ifr.ifr_name, "eth0");
			
	
			if (!mib_chain_get(MIB_SW_PORT_TBL, instnum-1, (void *)&Port))
				return -1;
			mii->phy_id = instnum-1; // phy i
			mii->reg_num = 4; // register 4
			// set NWAY advertisement
			mii->val_in = 0x0401; // enable flow control capability and IEEE802.3
			if (Port.linkMode == LINK_10HALF || Port.linkMode == LINK_AUTO)
				mii->val_in |= (1<<(5+LINK_10HALF));
			if (Port.linkMode == LINK_10FULL || Port.linkMode == LINK_AUTO)
				mii->val_in |= (1<<(5+LINK_10FULL));
			if (Port.linkMode == LINK_100HALF || Port.linkMode == LINK_AUTO)
				mii->val_in |= (1<<(5+LINK_100HALF));
			if (Port.linkMode == LINK_100FULL || Port.linkMode == LINK_AUTO)
				mii->val_in |= (1<<(5+LINK_100FULL));
			skfd = socket(AF_INET, SOCK_DGRAM, 0);
			if(skfd == -1)
			{
				fprintf(stderr, "Socket Open failed Error\n");
				return -1;
			}
			if (ioctl(skfd, SIOCSMIIREG, &ifr) < 0) {
				fprintf(stderr, "SIOCSMIIREG on %s failed: %s\n", ifr.ifr_name,
					strerror(errno));
				status=-1;
			}

			// restart
			mii->reg_num = 0; // register 0
			mii->val_in = 0x1200; // enable auto-negotiation and restart it
			if (ioctl(skfd, SIOCSMIIREG, &ifr) < 0) {
				fprintf(stderr, "SIOCSMIIREG on %s failed: %s\n", ifr.ifr_name,
					strerror(errno));
				status=-1;
			}
			
			close(skfd);

		}
#endif // 
#else // CONFIG_EXT_SWITCH
#ifdef ELAN_LINK_MODE_INTRENAL_PHY
		int skfd;
		struct ifreq ifr;
		unsigned char mode;
		//struct mii_ioctl_data *mii = (struct mii_data *)&ifr.ifr_data;
		//MIB_CE_SW_PORT_T Port;
		//int i, k, total;
		struct ethtool_cmd ecmd;
			
	
		strcpy(ifr.ifr_name, "eth0");
		ifr.ifr_data = &ecmd;
			
	
		if (!mib_get(MIB_ETH_MODE, (void *)&mode))
			return -1;

		skfd = socket(AF_INET, SOCK_DGRAM, 0);
		if(skfd == -1)
		{
			fprintf(stderr, "Socket Open failed Error\n");
			return -1;
		}
		ecmd.cmd = ETHTOOL_GSET;
		if (ioctl(skfd, SIOCETHTOOL, &ifr) < 0) {
			fprintf(stderr, "ETHTOOL_GSET on %s failed: %s\n", ifr.ifr_name,
				strerror(errno));
			status=-1;
			goto error;
		}

		ecmd.autoneg = AUTONEG_DISABLE;
		switch(mode) {
		case LINK_10HALF:
			ecmd.speed = SPEED_10;
			ecmd.duplex = DUPLEX_HALF;
			break;
		case LINK_10FULL:
			ecmd.speed = SPEED_10;
			ecmd.duplex = DUPLEX_FULL;
			break;
		case LINK_100HALF:
			ecmd.speed = SPEED_100;
			ecmd.duplex = DUPLEX_HALF;
			break;
		case LINK_100FULL:
			ecmd.speed = SPEED_100;
			ecmd.duplex = DUPLEX_FULL;
			break;
		default:
			ecmd.autoneg = AUTONEG_ENABLE;
		}

		ecmd.cmd = ETHTOOL_SSET;
		if (ioctl(skfd, SIOCETHTOOL, &ifr) < 0) {
			fprintf(stderr, "ETHTOOL_SSET on %s failed: %s\n", ifr.ifr_name,
				strerror(errno));
			status=-1;
			goto error;
		}
		/*
		ecmd.cmd = ETHTOOL_NWAY_RST;
		if (ioctl(skfd, SIOCETHTOOL, &ifr) < 0) {
			fprintf(stderr, "ETHTOOL_SSET on %s failed: %s\n", ifr.ifr_name,
				strerror(errno));
			status=-1;
		}
		*/

		error:
		
		close(skfd);
#endif
#endif
		return status;
#ifdef _CWMP_MIB_
#ifdef _CWMP_APPLY_
	}
#endif
#endif

	return -1;

}

#ifdef ELAN_LINK_MODE
// return value:
// 0  : successful
// -1 : failed
int setupLinkMode()
{
	int skfd;
	struct ifreq ifr;
	struct mii_ioctl_data *mii = (struct mii_data *)&ifr.ifr_data;
	//MIB_CE_SW_PORT_Tp pPort;
	MIB_CE_SW_PORT_T Port;
	int i, k, total;
	int status=0;
	
	strcpy(ifr.ifr_name, "eth0");
	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	total = mib_chain_total(MIB_SW_PORT_TBL);

	for (i=0; i<total; i++) {
		if (!mib_chain_get(MIB_SW_PORT_TBL, i, (void *)&Port))
			continue;
		mii->phy_id = i; // phy i
		mii->reg_num = 4; // register 4
		// set NWAY advertisement
		mii->val_in = 0x0401; // enable flow control capability and IEEE802.3
		if (Port.linkMode == LINK_10HALF || Port.linkMode == LINK_AUTO)
			mii->val_in |= (1<<(5+LINK_10HALF));
		if (Port.linkMode == LINK_10FULL || Port.linkMode == LINK_AUTO)
			mii->val_in |= (1<<(5+LINK_10FULL));
		if (Port.linkMode == LINK_100HALF || Port.linkMode == LINK_AUTO)
			mii->val_in |= (1<<(5+LINK_100HALF));
		if (Port.linkMode == LINK_100FULL || Port.linkMode == LINK_AUTO)
			mii->val_in |= (1<<(5+LINK_100FULL));
		
		if (ioctl(skfd, SIOCSMIIREG, &ifr) < 0) {
			fprintf(stderr, "SIOCSMIIREG on %s failed: %s\n", ifr.ifr_name,
				strerror(errno));
			status=-1;
		}
		
		// restart
		mii->reg_num = 0; // register 0
		mii->val_in = 0x1200; // enable auto-negotiation and restart it
		if (ioctl(skfd, SIOCSMIIREG, &ifr) < 0) {
			fprintf(stderr, "SIOCSMIIREG on %s failed: %s\n", ifr.ifr_name,
				strerror(errno));
			status=-1;
		};
	}
	if(skfd!=-1) close(skfd);
	return status;
}
#endif // of ELAN_LINK_MODE


// Jenny, update PPPoE session information
void pppoe_session_update(void *t)
{
	unsigned int totalEntry;
	MIB_CE_PPPOE_SESSION_T Entry;
	int i, found=0, selected;
	PPPOE_DRV_CTRL *p = (PPPOE_DRV_CTRL *)t;

	totalEntry = mib_chain_total(MIB_PPPOE_SESSION_TBL); /* get chain record size */
	for (i=0; i<totalEntry; i++) {
		if (!mib_chain_get(MIB_PPPOE_SESSION_TBL, i, (void *)&Entry)) {
  			printf("Get chain record error!\n");
			return;
		}

		if (Entry.vpi == p->vpi && Entry.vci == p->vci) {
			if (found > 0)
				if (Entry.ifNo != p->ifno) {
					MIB_CE_PPPOE_SESSION_T pEntry;
					mib_chain_get(MIB_PPPOE_SESSION_TBL, selected, (void *)&pEntry);
					if (pEntry.ifNo == p->ifno)
						break;
				}
			selected = i;
			found ++;
		}
	}
	if (found != 0) {
		mib_chain_get(MIB_PPPOE_SESSION_TBL, selected, (void *)&Entry);
		memcpy((unsigned char *)Entry.acMac, (unsigned char *)p->remote.sll_addr, 6);
		Entry.sessionId = p->session;
		if (Entry.ifNo != p->ifno) {	// for mult-session support
			Entry.ifNo = p->ifno;
			Entry.vpi = p->vpi;
			Entry.vci = p->vci;
			mib_chain_add(MIB_PPPOE_SESSION_TBL, (unsigned char*)&Entry);
		}
		else
			mib_chain_update(MIB_PPPOE_SESSION_TBL, (void *)&Entry, selected);
	}
	else {
		Entry.ifNo = p->ifno;
		Entry.vpi = p->vpi;
		Entry.vci = p->vci;
		memcpy((unsigned char *)Entry.acMac, (unsigned char *)p->remote.sll_addr, 6);
		Entry.sessionId = p->session;
		mib_chain_add(MIB_PPPOE_SESSION_TBL, (unsigned char*)&Entry);
	}
}

void save_pppoe_sessionid(void *t)
{
#if 0
int totalEntry,i;
MIB_CE_ATM_VC_T Entry;
printf("%s###########################start\n",__FUNCTION__);
totalEntry = mib_chain_total(MIB_ATM_VC_TBL); 
for(i=0;i<totalEntry;i++)
{
   if(!mib_chain_get(MIB_ATM_VC_TBL,i,&Entry))
   	printf("error!!!!\n");
   else
   	printf("%d/%d vpi=%d,vci=%d\n",i,totalEntry,Entry.vpi,Entry.vci);
}
printf("%s###########################end\n",__FUNCTION__);
#endif
	PPPOE_DRV_CTRL *p = (PPPOE_DRV_CTRL *)t;
	int auth_pid_fd=-1;
	mib_backup(CONFIG_MIB_CHAIN);	// backup current MIB in RAM
	if(mib_load(CURRENT_SETTING, CONFIG_MIB_CHAIN)) {	//get valid setting from flash
		pppoe_session_update(t);
		#if 0
		itfcfg("eth0", 0);
		// wait for sar to finish processing all the queueing packets
		sleep(2);
		itfcfg("sar", 0);
#ifdef WLAN_SUPPORT
		//unsigned char encrypt, no_wlan;
		//unsigned char enable1x=0;
		//int status = -1;
		unsigned char no_wlan;
		mib_get(MIB_WLAN_DISABLED, (void *)&no_wlan);
		if (!no_wlan)
			//itfcfg((char *)WLANIF, 0);
			stopwlan();
#endif
		#endif // if #if 0
		/* upgdate to flash */
		if(mib_update(CURRENT_SETTING, CONFIG_MIB_CHAIN) == 0)
			printf("CS Flash error! \n");
		#if 0
		itfcfg("sar", 1);
		itfcfg("eth0", 1);
#ifdef WLAN_SUPPORT
		if (!no_wlan) {
			startWLan();
#if 0
			itfcfg((char *)WLANIF, 1);
			mib_get(MIB_WLAN_ENCRYPT, (void *)&encrypt);
#ifdef WLAN_1x
			mib_get( MIB_WLAN_ENABLE_1X, (void *)&enable1x);
#endif
			// Mason Yu
			sleep(1);		

			if ((encrypt >= 2 || enable1x==1) && no_wlan == 0) {// 802.1x enabled			
				do {
					status = va_cmd(AUTH, 4, 1, WLANIF, LANIF, "auth", WLAN_AUTH_CONF);
					while ((auth_pid_fd = open(AUTH_PID, O_WRONLY)) == -1)
						usleep(30000);				
					close(auth_pid_fd);
					usleep(2000000);
					status |= va_cmd(IWCONTROL, 1, 1, WLANIF);
				} while (status != 0);
			}
#endif
		}
#endif //WLAN_SUPPORT
		#endif // of #if 0
	}
	else
  		printf("CS Flash load error! \n");
	mib_restore(CONFIG_MIB_CHAIN);	// restore previous MIB setting
	pppoe_session_update(t);
#if 0
printf("%s###########################start\n",__FUNCTION__);
totalEntry = mib_chain_total(MIB_ATM_VC_TBL); 
for(i=0;i<totalEntry;i++)
{
   if(!mib_chain_get(MIB_ATM_VC_TBL,i,&Entry))
   	printf("error!!!!\n");
   else
   	printf("%d/%d vpi=%d,vci=%d\n",i,totalEntry,Entry.vpi,Entry.vci);
}
printf("%s###########################end\n",__FUNCTION__);
	MIB_CE_PPPOE_SESSION_T Entryp;
	int  found=0, selected,totalEntryp;
          totalEntryp = mib_chain_total(MIB_PPPOE_SESSION_TBL); /* get chain record size */
	for (i=0; i<totalEntryp; i++) {
		if (!mib_chain_get(MIB_PPPOE_SESSION_TBL, i, (void *)&Entryp)) {
  			printf("Get chain record error!\n");
			return;
		}
		printf("vpi=%d vci=%d  sessionid=%d\n",Entryp.vpi,Entryp.vci,Entryp.sessionId);
	}
#endif
}

// Mason Yu
static struct callout *callout = NULL;	/* Callout list */
static struct timeval timenow;		/* Current time */
void untimeout(handle);

/*
 * timeout - Schedule a timeout.
 *
 * Note that this timeout takes the number of seconds, NOT hz (as in
 * the kernel).
 */
void
timeout(func, arg, time, handle)
    void (*func) __P((void *));
    void *arg;
    int time;
    struct callout *handle;
{
    struct callout *p, **pp;

    untimeout(handle);
    
    handle->c_arg = arg;
    handle->c_func = func;
    gettimeofday(&timenow, NULL);
    handle->c_time.tv_sec = timenow.tv_sec + time;
    handle->c_time.tv_usec = timenow.tv_usec;
  
    /*
     * Find correct place and link it in.
     */
    for (pp = &callout; (p = *pp); pp = &p->c_next)
	if (handle->c_time.tv_sec < p->c_time.tv_sec
	    || (handle->c_time.tv_sec == p->c_time.tv_sec
		&& handle->c_time.tv_usec < p->c_time.tv_usec))
	    break;
    handle->c_next = p;
    *pp = handle;
}


/*
 * untimeout - Unschedule a timeout.
 */
void
untimeout(handle)
struct callout *handle;
{
    struct callout **copp, *freep;

    /*
     * Find first matching timeout and remove it from the list.
     */
    for (copp = &callout; (freep = *copp); copp = &freep->c_next)
	if (freep == handle) {
	    *copp = freep->c_next;
	    break;
	}
}


/*
 * calltimeout - Call any timeout routines which are now due.
 */
void
calltimeout()
{
    struct callout *p;

    while (callout != NULL) {
		p = callout;
		if (gettimeofday(&timenow, NULL) < 0)
		    //fatal("Failed to get time of day: %m");
	    	printf("Failed to get time of day: %m");
		if (!(p->c_time.tv_sec < timenow.tv_sec
		      || (p->c_time.tv_sec == timenow.tv_sec
			  && p->c_time.tv_usec <= timenow.tv_usec)))
	    	break;		/* no, it's not time yet */
		callout = p->c_next;
		(*p->c_func)(p->c_arg);
    }
}


/*
 * timeleft - return the length of time until the next timeout is due.
 */
static struct timeval *
timeleft(tvp)
    struct timeval *tvp;
{
    if (callout == NULL)
	return NULL;

    gettimeofday(&timenow, NULL);
    tvp->tv_sec = callout->c_time.tv_sec - timenow.tv_sec;
    tvp->tv_usec = callout->c_time.tv_usec - timenow.tv_usec;
    if (tvp->tv_usec < 0) {
	tvp->tv_usec += 1000000;
	tvp->tv_sec -= 1;
    }
    if (tvp->tv_sec < 0)
	tvp->tv_sec = tvp->tv_usec = 0;

    return tvp;
}

#ifdef WEB_REDIRECT_BY_MAC
struct callout landingPage_ch;
void clearLandingPageRule(void *dummy)
{	
	int status=0;
	char ipaddr[16], ip_port[32];
	char tmpbuf[MAX_URL_LEN];
	int  def_port=WEB_REDIR_BY_MAC_PORT;
	int num, i;
	unsigned int uLTime;
	
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-F", "WebRedirectByMAC");
	
	ipaddr[0]='\0'; ip_port[0]='\0';
	
	if (mib_get(MIB_ADSL_LAN_IP, (void *)tmpbuf) != 0)
	{
		strncpy(ipaddr, inet_ntoa(*((struct in_addr *)tmpbuf)), 16);
		ipaddr[15] = '\0';
		sprintf(ip_port,"%s:%d",ipaddr,def_port);
	}
	
	//iptables -t nat -A Web_Redirect -d 192.168.1.1 -j RETURN
	status|=va_cmd(IPTABLES, 8, 1, "-t", "nat","-A","WebRedirectByMAC",
		"-d", ipaddr, "-j", (char *)FW_RETURN);		
	
	//iptables -t nat -A Web_Redirect -p tcp --dport 80 -j DNAT --to-destination 192.168.1.1:8080 
	status|=va_cmd(IPTABLES, 12, 1, "-t", "nat","-A","WebRedirectByMAC",
		"-p", "tcp", "--dport", "80", "-j", "DNAT", 
		"--to-destination", ip_port);		
		
	num = mib_chain_total( MIB_WEB_REDIR_BY_MAC_TBL );
	for ( i=0; i<num; i++) {
		mib_chain_delete( MIB_WEB_REDIR_BY_MAC_TBL, i );			
	}
	
	//update to the flash
	#if 0
	itfcfg("sar", 0);
	itfcfg("eth0", 0);
	#endif
	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
	#if 0
	itfcfg("sar", 1);
	itfcfg("eth0", 1);
	#endif
         
        mib_get(MIB_WEB_REDIR_BY_MAC_INTERVAL, (void *)&uLTime);
	TIMEOUT(clearLandingPageRule, 0, uLTime, landingPage_ch);
}
#endif



#ifdef AUTO_DETECT_DMZ
#define AUTO_DMZ_INTERVAL 30
static int getDhcpClientIP(char **ppStart, unsigned long *size, char *ip)
{
	struct dhcpOfferedAddr {
        	u_int8_t chaddr[16];
        	u_int32_t yiaddr;       /* network order */        	
        	u_int32_t expires;      /* host order */        	
	};
	struct dhcpOfferedAddr entry;
	if (*size < sizeof(entry))
		return -1;
	entry = *((struct dhcpOfferedAddr *)*ppStart);
	*ppStart = *ppStart + sizeof(entry);
	*size = *size - sizeof(entry);
	if (entry.expires == 0) 		
		return 0;
	if (entry.chaddr[0]==0&&entry.chaddr[1]==0&&entry.chaddr[2]==0&&entry.chaddr[3]==0&&entry.chaddr[4]==0&&entry.chaddr[5]==0)
		return 0;
	strcpy(ip, inet_ntoa(*((struct in_addr *)&entry.yiaddr)));
	return 1;
}

static int Get1stArp(char *dmzIP)
{
	FILE *fp;
	char  buf[256];
	char tmp0[32],tmp1[32],tmp2[32];
	int dmzFlags;
	dmzIP[0] = 0; // "" empty
	fp = fopen("/proc/net/arp", "r");
	if (fp == NULL)
		printf("read arp file fail!\n");
	else {
		fgets(buf, 256, fp);//first line!?
		while(fgets(buf, 256, fp) >0) {
			//sscanf(buf, "%s", dmzIP);
			sscanf(buf,"%s	%*s	0x%x %s %s %s ", dmzIP, &dmzFlags,tmp0,tmp1,tmp2);
			if ((dmzFlags == 0) || (strncmp(tmp2,"br",2)!=0))
				continue;
			return 1;
		}
		fclose(fp);
		return 0;
	}
	return 0;
}
static int searchArp(char *dhcpIp)
{
	FILE *fp;
	char  buf[256], dmzIP[20];
	fp = fopen("/proc/net/arp", "r");
	if (fp == NULL)
		printf("read arp file fail!\n");
	else {
		fgets(buf, 256, fp);
		while(fgets(buf, 256, fp)) {
			sscanf(buf, "%s", dmzIP);
			if (!strcmp(dmzIP, dhcpIp)) {	// entry found!
				fclose(fp);
				return 1;
			}
		}
		fclose(fp);
		return 0;
	}
}

static void clearDMZ(char *ip)
{
	va_cmd(IPTABLES, 13, 1, "-t", "nat", (char *)FW_DEL, (char *)IPTABLE_DMZ, (char *)ARG_I, "!",
		(char *)LANIF, "-p", (char *)ARG_UDP, (char *)FW_DPORT, "161:162", "-j", (char *)FW_ACCEPT);
#ifdef _CWMP_MIB_
	va_cmd(IPTABLES, 13, 1, "-t", "nat", (char *)FW_DEL, (char *)IPTABLE_DMZ, (char *)ARG_I, "!",
		(char *)LANIF, "-p", (char *)ARG_TCP, (char *)FW_DPORT, "9999", "-j", (char *)FW_ACCEPT);	  
#endif
	va_cmd(IPTABLES, 11, 1, "-t", "nat", (char *)FW_DEL, (char *)IPTABLE_DMZ, (char *)ARG_I, "!",
		(char *)LANIF, "-j", "DNAT", "--to-destination", ip);
	va_cmd(IPTABLES, 11, 1, "-D", (char *)IPTABLE_DMZ, (char *)ARG_I, "!",
		(char *)LANIF, (char *)ARG_O, (char *)LANIF, "-d", ip, "-j", (char *)FW_ACCEPT);
}

static void setDMZ(char *ip, char *s_entryNum)
{
	//snmp don't forward to DMZ
	// iptables -t nat -A PREROUTING -i ! $LAN_IF -p UDP --dport 161:162 -j ACCEPT
	va_cmd(IPTABLES, 13, 1, "-t", "nat", (char *)FW_ADD, (char *)IPTABLE_DMZ, (char *)ARG_I, "!",
		(char *)LANIF, "-p", (char *)ARG_UDP, (char *)FW_DPORT, "161:162", "-j", (char *)FW_ACCEPT);
#ifdef _CWMP_MIB_
	va_cmd(IPTABLES, 13, 1, "-t", "nat", (char *)FW_ADD, (char *)IPTABLE_DMZ, (char *)ARG_I, "!",
		(char *)LANIF, "-p", (char *)ARG_TCP, (char *)FW_DPORT, "9999", "-j", (char *)FW_ACCEPT);	  
#endif
	va_cmd(IPTABLES, 11, 1, "-t", "nat", (char *)FW_ADD, (char *)IPTABLE_DMZ, (char *)ARG_I, "!",
		(char *)LANIF, "-j", "DNAT", "--to-destination", ip);
	// iptables -I ipfilter 3 -i ! $LAN_IF -o $LAN_IF -d $DMZ_IP -j RETURN
	va_cmd(IPTABLES, 11, 1, (char *)FW_ADD, (char *)IPTABLE_DMZ, (char *)ARG_I, "!",
		(char *)LANIF, (char *)ARG_O, (char *)LANIF, "-d", ip, "-j", (char *)FW_ACCEPT);
}

#define _DHCPD_PID_FILE			"/var/run/udhcpd.pid"
struct callout autoDMZ_ch;
char autoDMZ = 0;

void poll_autoDMZ(void *dummy)
{
	// signal dhcp client to renew
	struct stat status;
	char dhcpIP[40], *buffer = NULL, *ptr;
	FILE *fp;
	char dmz_ip_str[20];


	if (autoDMZ == 0) // search 1st arp
	{
		unsigned long ulDmz, ulDhcp;
		unsigned char ucPreStat;
		int entryNum, ret;
		char s_entryNum[8], ip[32];
		struct in_addr ipAddr, dmzIp;

		fflush(stdout);
		if (Get1stArp(dmz_ip_str) == 1)
		{
			if (strlen(dmz_ip_str) == 0)
				goto end; // error

			autoDMZ = 1;
			mib_get(MIB_DMZ_ENABLE, (void *)&ucPreStat);
			mib_get(MIB_DMZ_IP, (void *)&dmzIp);
			ulDmz = *((unsigned long *)&dmzIp);
			strncpy(ip, inet_ntoa(dmzIp), 16); // ip -> old dmz
			if (strcmp(ip,dmz_ip_str) == 0)
				goto end; // no changed!
			
			inet_aton(dmz_ip_str, &ipAddr);
			ip[15] = '\0';	
			entryNum = mib_chain_total (MIB_IP_PORT_FILTER_TBL) + 3;
			snprintf(s_entryNum, 8, "%d", entryNum);
			if (ucPreStat && ulDmz != 0)
				clearDMZ(ip);
			
			if (mib_set(MIB_DMZ_IP, (void *)&ipAddr) == 0)
				printf("Set DMZ MIB error!\n");
				
				if (!mib_set(MIB_DMZ_ENABLE, (void *)&autoDMZ))
					printf("Set DMZ Capability error!\n");
				setDMZ(dmz_ip_str, s_entryNum);
				goto end;

		}
			
	}
	else  // check dhcp and then arp
	{
		unsigned long ulDmz, ulDhcp;
		unsigned char ucPreStat;
		struct in_addr ipAddr, dmzIp;
		int entryNum, ret;
		char s_entryNum[8], ip[32];
		// siganl DHCP server to update lease file
		int pid = read_pid(_DHCPD_PID_FILE);
		
		if (pid > 0)	// renew
			kill(pid, SIGUSR1);
		usleep(1000);

		if (stat(DHCPD_LEASE, &status) < 0)
			goto end;

		// read DHCP server lease file
		buffer = malloc(status.st_size);
		if (buffer == NULL)
			goto end;
		fp = fopen(DHCPD_LEASE, "r");
		if (fp == NULL)
			goto end;
		fread(buffer, 1, status.st_size, fp);
		fclose(fp);
		ptr = buffer;
		
		while (1) {			
			if (getDhcpClientIP(&ptr, &status.st_size, dhcpIP) == 1)
			{
				mib_get(MIB_DMZ_ENABLE, (void *)&ucPreStat);
				mib_get(MIB_DMZ_IP, (void *)&dmzIp);
				ulDmz = *((unsigned long *)&dmzIp);
				strncpy(ip, inet_ntoa(dmzIp), 16);
				if (strcmp(ip, dhcpIP) ==0 )	//dhcp still in using..
				{
					goto end;
				}
			}
			else // find the 1st arp
			{
				// uses the 1st entry
				if (Get1stArp(dmz_ip_str) == 1)
				{
					if (strlen(dmz_ip_str) == 0)
						goto end; // error

					autoDMZ = 1;
					mib_get(MIB_DMZ_ENABLE, (void *)&ucPreStat);
					mib_get(MIB_DMZ_IP, (void *)&dmzIp);
					ulDmz = *((unsigned long *)&dmzIp);
					strncpy(ip, inet_ntoa(dmzIp), 16); // ip -> old dmz
					inet_aton(dmz_ip_str, &ipAddr);
					ip[15] = '\0';	
					if (strcmp(ip,dmz_ip_str) == 0)
						goto end; // still the same one
					
					entryNum = mib_chain_total (MIB_IP_PORT_FILTER_TBL) + 3;
					snprintf(s_entryNum, 8, "%d", entryNum);
					if (ucPreStat && ulDmz != 0)
						clearDMZ(ip);
					
					if (mib_set(MIB_DMZ_IP, (void *)&ipAddr) == 0)
						printf("Set DMZ MIB error!\n");
						
						if (!mib_set(MIB_DMZ_ENABLE, (void *)&autoDMZ))
							printf("Set DMZ Capability error!\n");
						setDMZ(dmz_ip_str, s_entryNum);
						goto end;

				}
				else
				{
					// clear rules
					mib_get(MIB_DMZ_IP, (void *)&dmzIp);					
					strncpy(ip, inet_ntoa(dmzIp), 16); // ip -> old dmz
					ip[15] = '\0';	
					
					clearDMZ(ip);
					*((unsigned long *)&dmzIp) = 0;
					autoDMZ = 0;
					mib_set(MIB_DMZ_ENABLE, (void *)&autoDMZ);
					mib_set(MIB_DMZ_IP, (void *)&dmzIp);										
				}
				goto end;
			}
		}
	}


end:
	if (buffer)
		free(buffer);
	TIMEOUT(poll_autoDMZ, 0, AUTO_DMZ_INTERVAL, autoDMZ_ch);
}
#endif

void lbTimeout()
{
	finished_OAMLB = 1;
}


void oamLBTimeout(void *dummy)
{
	printf("***** finished_OAMLB = 1\n");
	finished_OAMLB = 1;
}

int doOAMLookback(MIB_CE_ATM_VC_Tp pEntry)
{
#ifdef EMBED
	int	skfd;
	struct atmif_sioc mysio;
	ATMOAMLBReq lbReq;
	ATMOAMLBState lbState;
	//int i;
	time_t currTime, preTime;
	
	if((skfd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0){
		perror("socket open error");
		return -1;
	}
	
	memset(&lbReq, 0, sizeof(ATMOAMLBReq));
	memset(&lbState, 0, sizeof(ATMOAMLBState));
	lbReq.Scope = 0;	// Segment
	lbReq.vpi = pEntry->vpi;
	lbReq.vci = pEntry->vci;
	memset(lbReq.LocID, 0xff, 16);	// Loopback Location ID
	mysio.number = 0;	// ATM interface number
	mysio.arg = (void *)&lbReq;
	// Start the loopback test
	if (ioctl(skfd, ATM_OAM_LB_START, &mysio)<0) {
		perror("ioctl: ATM_OAM_LB_START failed !");
		close(skfd);
		return -1;
	}
	
	finished_OAMLB = 0;
	// Mason Yu. If receive a SIGALRM, the all timer will be disable.
	// So I use "time()" to schedule Timer.
	//signal(SIGALRM, lbTimeout);
	//alarm(MAXWAIT);	
	
	// Mason Yu. I can not use "TIMEOUT()" to register timer.
	//TIMEOUT(oamLBTimeout, 0, 1, oamLookBK_ch);
	time(&currTime);
	preTime = currTime;
	
	// Query the loopback status
	mysio.arg = (void *)&lbState;
	lbState.vpi = pEntry->vpi;
	lbState.vci = pEntry->vci;
	lbState.Tag = lbReq.Tag;
	
	while (1)
	{
		// Mason Yu. Use time() to schedule Timer.
		time(&currTime);
	    	if (currTime - preTime >= 1) {
	    		printf("This OAMLB is time out!\n");
	    		finished_OAMLB = 1;
	    	}
	    	
		if (finished_OAMLB)
			break;	// break for timeout
		
		if (ioctl(skfd, ATM_OAM_LB_STATUS, &mysio)<0) {
			perror("ioctl: ATM_OAM_LB_STATUS failed !");
			mysio.arg = (void *)&lbReq;
			ioctl(skfd, ATM_OAM_LB_STOP, &mysio);
			close(skfd);
			return -1;
		}
		
		if (lbState.count[0] > 0)
		{
			break;	// break for loopback success
		}
	}
		
	mysio.arg = (void *)&lbReq;
	// Stop the loopback test
	if (ioctl(skfd, ATM_OAM_LB_STOP, &mysio)<0) {
		perror("ioctl: ATM_OAM_LB_STOP failed !");
		close(skfd);
		return -1;
	}
	close(skfd);
	
	if (finished_OAMLB)
		return 0;	// failed
	else
		return 1;	// successful
#else
	return 1;
#endif
}

int doOAMLookback_F5_EndToEnd(MIB_CE_ATM_VC_Tp pEntry)
{
#ifdef EMBED
	int	skfd;
	struct atmif_sioc mysio;
	ATMOAMLBReq lbReq;
	ATMOAMLBState lbState;
	//int i;
	time_t currTime, preTime;
	
	if((skfd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0){
		perror("socket open error(F5 End-To-End)");
		return -1;
	}
	
	memset(&lbReq, 0, sizeof(ATMOAMLBReq));
	memset(&lbState, 0, sizeof(ATMOAMLBState));
	lbReq.Scope = 1;	// End-To-End
	lbReq.vpi = pEntry->vpi;
	lbReq.vci = pEntry->vci;
	memset(lbReq.LocID, 0xff, 16);	// Loopback Location ID
	mysio.number = 0;	// ATM interface number
	mysio.arg = (void *)&lbReq;
	// Start the loopback test
	if (ioctl(skfd, ATM_OAM_LB_START, &mysio)<0) {
		perror("ioctl: ATM_OAM_LB_START(F5 End-To-End) failed !");
		close(skfd);
		return -1;
	}
	
	finished_OAMLB_F5_EndToEnd = 0;
	// Mason Yu. If receive a SIGALRM, the all timer will be disable.
	// So I use "time()" to schedule Timer.
	//signal(SIGALRM, lbTimeout);
	//alarm(MAXWAIT);	
	
	// Mason Yu. I can not use "TIMEOUT()" to register timer.
	//TIMEOUT(oamLBTimeout, 0, 1, oamLookBK_ch);
	time(&currTime);
	preTime = currTime;
	
	// Query the loopback status
	mysio.arg = (void *)&lbState;
	lbState.vpi = pEntry->vpi;
	lbState.vci = pEntry->vci;
	lbState.Tag = lbReq.Tag;
	
	while (1)
	{
		// Mason Yu. Use time() to schedule Timer.
		time(&currTime);
	    	if (currTime - preTime >= 1) {
	    		printf("This OAMLB(F5 End-To-End) is time out!\n");
	    		finished_OAMLB_F5_EndToEnd = 1;
	    	}
	    	
		if (finished_OAMLB_F5_EndToEnd)
			break;	// break for timeout
		
		if (ioctl(skfd, ATM_OAM_LB_STATUS, &mysio)<0) {
			perror("ioctl: ATM_OAM_LB_STATUS(F5 End-To-End) failed !");
			mysio.arg = (void *)&lbReq;
			ioctl(skfd, ATM_OAM_LB_STOP, &mysio);
			close(skfd);
			return -1;
		}
		
		if (lbState.count[0] > 0)
		{
			break;	// break for loopback success
		}
	}
		
	mysio.arg = (void *)&lbReq;
	// Stop the loopback test
	if (ioctl(skfd, ATM_OAM_LB_STOP, &mysio)<0) {
		perror("ioctl: ATM_OAM_LB_STOP(F5 End-To-End) failed !");
		close(skfd);
		return -1;
	}
	close(skfd);
	
	if (finished_OAMLB_F5_EndToEnd)
		return 0;	// failed
	else
		return 1;	// successful
#else
	return 1;
#endif
}


// Timer for auto search PVC
#if defined(AUTO_PVC_SEARCH_TR068_OAMPING) || defined(AUTO_PVC_SEARCH_PURE_OAMPING) || defined(AUTO_PVC_SEARCH_AUTOHUNT)
struct callout autoSearchPVC_ch;
int sok=0;
#define MAX_PVC_SEARCH_PAIRS 16


#ifdef AUTO_PVC_SEARCH_TR068_OAMPING
typedef struct pvc_entry {
	unsigned long vpi;
	unsigned long vci;
} PVC_T;

PVC_T pvcList[] = {{0, 35}, {8, 35}, {0, 43}, {0, 51}, {0, 59},
	{8, 43}, {8, 51}, {8, 59}, {0}
};
#endif

void succ_AutoSearchPVC()
{
	fin_AutoSearchPVC = 1;
	UNTIMEOUT(stopAutoSearchPVC, 0, autoSearchPVC_ch);
	printf("***** Auto Search PVC is successful and stopped ! *****\n");
	syslog(LOG_INFO, "Auto Search PVC is successful and stopped !\n");
}

#if defined(AUTO_PVC_SEARCH_AUTOHUNT)
void StopSarAutoPvcSearch(int vpi,int vci)
{
	FILE *fp;
	if (fp=fopen("/proc/AUTO_PVC_SEARCH", "w") ) 
	{				
//		printf("StopSarAutoPvcSearch: received (%d,%d) inform SAR driver stop auto-pvc-search\n", vpi,vci);

		fprintf( fp, "0 %d,%d\n", vpi,vci);
		fclose(fp);
	} else {
		printf("Open /proc/AUTO_PVC_SEARCH failed! Can't stop SAR driver doing auto-pvc-search\n");
	}			
}
#endif

// Mason Yu
#ifdef AUTO_PVC_SEARCH_TR068_OAMPING
void startAutoPVCSearchTR068OAMPing_F5_Segment(MIB_CE_ATM_VC_Tp pEntry, unsigned long bitmap)
{
	int idx;
	unsigned long mask=1;
	unsigned long ovpi, ovci;
	unsigned char vChar;
	BOOT_TYPE_T btmode;
	int i;	
	
	if (!pEntry || !pEntry->enable)
		goto search_table_tr068;		
    
        if (doOAMLookback(pEntry)!=1) 
	{
		if(i==0)					
			stopConnection(pEntry);	
	
search_table_tr068:						
		// start searching ...
		printf("TR068 OAMPing(F5 Segment):Default pvc failed, start searching ...\n");
		ovpi = pEntry->vpi;
		ovci = pEntry->vci;
	
		//Retrieve PVC list from Flash	
		for (idx=0; pvcList[idx].vpi || pvcList[idx].vci; idx++) {
			if (bitmap & mask) {
				//stopConnection(pEntry);
				pEntry->vpi = pvcList[idx].vpi;
				pEntry->vci = pvcList[idx].vci;
				//startConnection(pEntry);
				if (doOAMLookback(pEntry)) {
					// That's it
					printf("TR068 OAMPing(F5 Segment):That's it! vpi=%d, vci=%d\n", pEntry->vpi, pEntry->vci);
					sok=1;
					break;
				}
			}
			mask <<= 1;
		}

		if (sok) { // search ok, set it
			setVC0Connection(pEntry->vpi, pEntry->vci);
			succ_AutoSearchPVC();
		}
		else { // search failed, back to original
			//stopConnection(pEntry);
			pEntry->vpi = ovpi;
			pEntry->vci = ovci;
			//startConnection(pEntry);
			printf("TR068 OAMPing(F5 Segment):Auto-search pvc failed !\n");
		}
	}
	else
	{
		printf("TR068 OAMPing(F5 Segment):first-pvc oam loopback ok!\n");
		sok=1;
#ifdef AUTO_PVC_SEARCH_AUTOHUNT		
		StopSarAutoPvcSearch(pEntry->vpi, pEntry->vci);
#endif		
       		succ_AutoSearchPVC();
   }
}

void startAutoPVCSearchTR068OAMPing_F5_EndToEnd(MIB_CE_ATM_VC_Tp pEntry, unsigned long bitmap)
{
	int idx;
	unsigned long mask=1;
	unsigned long ovpi, ovci;
	unsigned char vChar;
	BOOT_TYPE_T btmode;
	int i;	
	
	if (!pEntry || !pEntry->enable)
		goto search_table_tr068;		
    
        if (doOAMLookback_F5_EndToEnd(pEntry)!=1) 
	{
		if(i==0)					
			stopConnection(pEntry);	
	
search_table_tr068:						
		// start searching ...
		printf("TR068 OAMPing(F5 End-To-End):Default pvc failed, start searching ...\n");
		ovpi = pEntry->vpi;
		ovci = pEntry->vci;
	
		//Retrieve PVC list from Flash	
		for (idx=0; pvcList[idx].vpi || pvcList[idx].vci; idx++) {
			if (bitmap & mask) {
				//stopConnection(pEntry);
				pEntry->vpi = pvcList[idx].vpi;
				pEntry->vci = pvcList[idx].vci;
				//startConnection(pEntry);
				if (doOAMLookback_F5_EndToEnd(pEntry)) {
					// That's it
					printf("TR068 OAMPing(F5 End-To-End):That's it! vpi=%d, vci=%d\n", pEntry->vpi, pEntry->vci);
					sok=1;
					break;
				}
			}
			mask <<= 1;
		}

		if (sok) { // search ok, set it
			setVC0Connection(pEntry->vpi, pEntry->vci);
			succ_AutoSearchPVC();
		}
		else { // search failed, back to original
			//stopConnection(pEntry);
			pEntry->vpi = ovpi;
			pEntry->vci = ovci;
			//startConnection(pEntry);
			printf("TR068 OAMPing(F5 End-To-End):Auto-search pvc failed !\n");
		}
	}
	else
	{
		printf("TR068 OAMPing(F5 End-To-End):first-pvc oam loopback ok!\n");
		sok=1;
#ifdef AUTO_PVC_SEARCH_AUTOHUNT		
		StopSarAutoPvcSearch(pEntry->vpi, pEntry->vci);
#endif		
       		succ_AutoSearchPVC();
   }
}
#endif

#ifdef AUTO_PVC_SEARCH_PURE_OAMPING
void startAutoPVCSearchPureOAMPing_F5_Segment(MIB_CE_ATM_VC_Tp pEntry, unsigned long bitmap)
{
	int idx;
	unsigned long mask=1;
	unsigned long ovpi, ovci;
	unsigned char vChar;
	BOOT_TYPE_T btmode;
	int i;

	//  read PVC table from flash
	unsigned int entryNum, Counter;
	//MIB_AUTO_PVC_SEARCH_Tp entryP;
	MIB_AUTO_PVC_SEARCH_T Entry;
	entryNum = mib_chain_total(MIB_AUTO_PVC_SEARCH_TBL);
	if(entryNum > MAX_PVC_SEARCH_PAIRS)
		entryNum = MAX_PVC_SEARCH_PAIRS;
			

	if (!pEntry || !pEntry->enable)
		goto search_table;		
   
        if (doOAMLookback(pEntry)!=1) 
	{
		//if(i==0)					
			stopConnection(pEntry);	
search_table:					
		// start searching ...
		printf("Pure OAMping(F5 Segment):Default pvc failed, start searching ...\n");
		ovpi = pEntry->vpi;
		ovci = pEntry->vci;
		
		//Retrieve PVC list
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
				printf("Pure OAMping(F5 Segment):That's it! vpi=%d, vci=%d\n", pEntry->vpi, pEntry->vci);
				printf("Pure OAMping(F5 Segment):Inform SAR driver to stop pvc search\n");					
#ifdef AUTO_PVC_SEARCH_AUTOHUNT
				StopSarAutoPvcSearch(pEntry->vpi, pEntry->vci);
#endif
				sok=1;
				break;
			}
			
		}


		if (sok) { // search ok, set it
			setVC0Connection(pEntry->vpi, pEntry->vci);
			succ_AutoSearchPVC();
		}
		else { // search failed, back to original
			//stopConnection(pEntry);
			pEntry->vpi = ovpi;
			pEntry->vci = ovci;
			//startConnection(pEntry);
			printf("Pure OAMping(F5 Segment): Auto-search PVC failed !\n");
		}
	}
	else
	{
		printf("Pure OAMping(F5 Segment): first-pvc oam loopback ok!\n");
		sok=1;
#ifdef AUTO_PVC_SEARCH_AUTOHUNT
		StopSarAutoPvcSearch(pEntry->vpi, pEntry->vci);
#endif
		succ_AutoSearchPVC();	
       	
   }
}

void startAutoPVCSearchPureOAMPing_F5_EndToEnd(MIB_CE_ATM_VC_Tp pEntry, unsigned long bitmap)
{
	int idx;
	unsigned long mask=1;
	unsigned long ovpi, ovci;
	unsigned char vChar;
	BOOT_TYPE_T btmode;
	int i;

	//  read PVC table from flash
	unsigned int entryNum, Counter;
	//MIB_AUTO_PVC_SEARCH_Tp entryP;
	MIB_AUTO_PVC_SEARCH_T Entry;
	entryNum = mib_chain_total(MIB_AUTO_PVC_SEARCH_TBL);
	if(entryNum > MAX_PVC_SEARCH_PAIRS)
		entryNum = MAX_PVC_SEARCH_PAIRS;
			

	if (!pEntry || !pEntry->enable)
		goto search_table;		
   
        if (doOAMLookback_F5_EndToEnd(pEntry)!=1) 
	{
		//if(i==0)					
			stopConnection(pEntry);	
search_table:					
		// start searching ...
		printf("Pure OAMping(F5 End-To-End):Default pvc failed, start searching ...\n");
		ovpi = pEntry->vpi;
		ovci = pEntry->vci;
		
		//Retrieve PVC list
		for(Counter=0; Counter< entryNum; Counter++)
		{
			if (!mib_chain_get(MIB_AUTO_PVC_SEARCH_TBL, Counter, (void *)&Entry))
				continue;
			//stopConnection(pEntry);
			pEntry->vpi = Entry.vpi;
			pEntry->vci = Entry.vci;
			//startConnection(pEntry);
			if (doOAMLookback_F5_EndToEnd(pEntry)) {
				// That's it
				printf("Pure OAMping(F5 End-To-End):That's it! vpi=%d, vci=%d\n", pEntry->vpi, pEntry->vci);
				printf("Pure OAMping(F5 End-To-End):Inform SAR driver to stop pvc search\n");					
#ifdef AUTO_PVC_SEARCH_AUTOHUNT
				StopSarAutoPvcSearch(pEntry->vpi, pEntry->vci);
#endif
				sok=1;
				break;
			}
			
		}


		if (sok) { // search ok, set it
			setVC0Connection(pEntry->vpi, pEntry->vci);
			succ_AutoSearchPVC();
		}
		else { // search failed, back to original
			//stopConnection(pEntry);
			pEntry->vpi = ovpi;
			pEntry->vci = ovci;
			//startConnection(pEntry);
			printf("Pure OAMping(F5 End-To-End): Auto-search PVC failed !\n");
		}
	}
	else
	{
		printf("Pure OAMping(F5 End-To-End): first-pvc oam loopback ok!\n");
		sok=1;
#ifdef AUTO_PVC_SEARCH_AUTOHUNT
		StopSarAutoPvcSearch(pEntry->vpi, pEntry->vci);
#endif
		succ_AutoSearchPVC();	
       	
   }
}
#endif


// Mason Yu
// Find ch1~7 except ch0 and check if the ch1~7's VPI/VCI are the same as Searched VPI/VCI by Auto PVC Search.
void ifAutoPVCisExist(unsigned int vpi, unsigned vci)
{
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	// find VC from ch1 not ch0. Because the ch0 is used for Auto PVC Search			
	for (i = 1; i < entryNum; i++)
	{									
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			return;					
		
		if ( Entry.vpi == vpi && Entry.vci == vci ) {												
			stopConnection(&Entry);
			
			if(mib_chain_delete(MIB_ATM_VC_TBL, i) != 1) {
				printf("ifAutoPVCisExist: Delete chain record error!");							
			}
			break;
		}
	}
}

static MIB_CE_ATM_VC_T vc0_Entry;

// Mason Yu
// If the configuration has include ch0, we just update ch0 with Searched VPI/VCI.
// If the configuration has not include ch0, we add a new entry(ch0) with Searched VPI/VCI.
int setVC0Connection(unsigned int vpi, unsigned int vci)
{	
	MIB_CE_ATM_VC_T Entry;	
	
	/* Retrieve ch 0's entry */
	if (!mib_chain_get(MIB_ATM_VC_TBL, 0, (void *)&Entry)) {
		// The ch0 does not exist
		printf("setVC0Connection: The ch0 does not exist\n");		
		
		// sarctl pvcnumber 1
		va_cmd("/bin/sarctl",2,1,"pvcnumber","1");
		memset(&Entry, 0x00, sizeof(Entry));
		
		Entry.ifIndex = 0xf0;
		Entry.vpi = vpi;
		Entry.qos = 0;
		Entry.vci = vci;
		Entry.pcr = ATM_MAX_US_PCR;
		Entry.encap = 1;
		Entry.napt = 1;
		Entry.mtu = 1500;
		Entry.enable = 1;
		
		/* create a new connection */	
		if(Entry.enable) {
			printf("setVC0Connection: start connection (%d/%d)\n", vpi, vci);
			startConnection(&Entry);
		}	
		
		if(mib_chain_add(MIB_ATM_VC_TBL, (unsigned char*)&Entry) != 1){
			printf("setVC0Connection:Error! Add MIB_ATM_VC_TBL chain record for Auto PVC Search.\n");
			return 0;				
		}
		mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);

	} else {  
		// The ch0 exist
		// Check if the searched VC exist on configure?		
		ifAutoPVCisExist(vpi, vci);		
  		
		/* remove the default connection */		
		stopConnection(&Entry);   		
  	
		Entry.vpi = vpi;
		Entry.vci = vci;
		
		/* create a new connection */		
		if(Entry.enable) {
			printf("setVC0Connection: start connection (%d/%d)\n", vpi, vci);
			startConnection(&Entry);
		}
		
		mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, 0);
		mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
	}
}

void ResetSarAutoHunt(int vpi,int vci, int encap)
{
	FILE *fp;
	if (fp=fopen("/proc/AUTO_PVC_SEARCH", "w") ) 
	{				
//		printf("StopSarAutoPvcSearch: received (%d,%d) inform SAR driver stop auto-pvc-search\n", vpi,vci);

		fprintf( fp, "2 %d,%d,%d\n", vpi,vci,encap);
		fclose(fp);
	} else {
		printf("Open /proc/AUTO_PVC_SEARCH failed! Can't reset SAR Driver to default setting for autoHunt\n");
	}			
}


#ifdef AUTO_PVC_SEARCH_AUTOHUNT
#define MAX_PVC_SEARCH_PAIRS 16
static void StartAutoHunt() 
{
	FILE *fp;

	//MIB_AUTO_PVC_SEARCH_Tp entryP;
	MIB_AUTO_PVC_SEARCH_T Entry;
	unsigned int entryNum,i;
	unsigned char tmp[12], tmpBuf[MAX_PVC_SEARCH_PAIRS*12];

	entryNum = mib_chain_total(MIB_AUTO_PVC_SEARCH_TBL);
	memset(tmpBuf, 0, sizeof(tmpBuf));
	for(i=0;i<entryNum; i++) {
		memset(tmp, 0, 12);
		//mib_chain_get(MIB_AUTO_PVC_SEARCH_TBL, i, (void *)&Entry);
		//if (!Entry)
		//	continue;
		if (!mib_chain_get(MIB_AUTO_PVC_SEARCH_TBL, i, (void *)&Entry))
			continue;
		sprintf(tmp,"(%d %d)", Entry.vpi, Entry.vci); 
		strcat(tmpBuf, tmp);
		
	}
	//printf("StartSarAutoPvcSearch: inform SAR %s\n", tmpBuf);
	

	if (fp=fopen("/proc/AUTO_PVC_SEARCH", "w") ) 
	{				
		fprintf(fp, "1%s\n", tmpBuf);	//write pvc list stored in flash to SAR driver
//		printf("StartSarAutoPvcSearch: Inform SAR driver to start auto-pvc-search\n");

		fclose(fp);
	} else {
		printf("Open /proc/AUTO_PVC_SEARCH failed! Can't start SAR driver doing auto-pvc-search\n");
	}	
	
}
#endif

#ifdef _LINUX_2_6_
unsigned int file_read_number(const char *filename) 
{
	FILE *fp;
	char buffer[10];
	unsigned int mask = 0;
	
	fp = fopen(filename, "r");
	
	if (!fp) {		
		return 0;
	}

	fgets(buffer, sizeof(buffer), fp);
	if (1 != sscanf(buffer, "%u", &mask)) {
		goto out;
	}
	fclose(fp);
	return mask;
	
out:
	fclose(fp);
	return 0;
	
}
#endif 
void stopAutoSearchPVC(void *dummy)
{
	MIB_CE_ATM_VC_T Entry;
	FILE *fp;
	int oamPingResult, do_autoHunt;		
	long i;	
	
	printf("***** Auto Search PVC is fail and stopped ! *****\n");
	syslog(LOG_INFO, "Auto Search PVC is fail and stopped");
		
	fp = fopen("/tmp/autoPVC", "r");
	if(fp){
		fscanf(fp, "OAMPing:%d AutoHUNT:%d\n", &oamPingResult, &do_autoHunt);
		fclose(fp);
		unlink("/tmp/autoPVC");
		
		// Auto search PVC is finished.
		fin_AutoSearchPVC = 1;
		
	} else {
		//TIMEOUT(stopAutoSearchPVC, 0, INTERVAL_AUTOPVC, autoSearchPVC_ch);		
		return;
	}	
	
	// Mason Yu. Delay 10 sec to make sure that the AutoHunt is finished.	
	//for(i=0; i<100000000; i++);			
	
	if ( oamPingResult == 0 && autoHunt_found == 0) {	
		printf("Reset SAR Driver for Auto Search PVC!\n");
		if (!mib_chain_get(MIB_ATM_VC_TBL, 0, (void *)&Entry))
			return;		
		
		if ( do_autoHunt == 1 ) {			
			//ResetSarAutoHunt(5, 35, 1);
			ResetSarAutoHunt(Entry.vpi, Entry.vci, Entry.encap);
		}
				
		// Reset ch0
		//setVC0Connection(5, 35);
		setVC0Connection(Entry.vpi, Entry.vci);			
	}
	
	
}

int startAutoSearchPVC()
{
	unsigned char autosearch;
	int i;
	MIB_CE_ATM_VC_T Entry;		
	
	if (mib_get(MIB_ATM_VC_AUTOSEARCH, (void *)&autosearch) != 0)
	{
		if (autosearch == 1)
		{
			unsigned long map = 0xffffffff;
			MIB_CE_ATM_VC_Tp pFirstEntry=NULL;
			Modem_LinkSpeed vLs;
			vLs.upstreamRate=0;
			unsigned int entryNum;
			// Timer for auto search PVC		
			FILE *fp;
			
			printf("***** Auto Search PVC started *****\n");
			syslog(LOG_INFO, "Auto Search PVC started");
			TIMEOUT(stopAutoSearchPVC, 0, INTERVAL_AUTOPVC, autoSearchPVC_ch);
					
			#if 0
			// wait until showtime
			while (!adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs,
				RLCM_GET_LINK_SPEED_SIZE) || vLs.upstreamRate == 0) {
				usleep(1000000);
			}
			usleep(2000000);	//AUTO-PVC-SEARCH
			#endif
									
			// do first-pvc auto search per [TR-068, I-88]
			// Modified by Mason Yu			
			entryNum = mib_chain_total(MIB_ATM_VC_TBL);
			
			for (i = 0; i < entryNum; i++)
			{
				unsigned long mask;
				int k;
				
				if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
					return -1;
				// Retrieve pvc list from flash
#ifdef AUTO_PVC_SEARCH_TR068_OAMPING
				mask = 1;
				for (k=0; pvcList[k].vpi || pvcList[k].vci; k++) {
					if (pvcList[k].vpi == Entry.vpi &&
						pvcList[k].vci == Entry.vci)
					{
						map ^= mask;	// set bit to zero
						break;
					}
					mask <<= 1;
				}
#endif
				if (pFirstEntry==NULL && Entry.enable) // get the first pvc
				{
					pFirstEntry = &Entry;
					// Added by Mason Yu
					break;
				}
				}
			
			// Modified by Mason Yu
			if (pFirstEntry==NULL) {
				pFirstEntry = &Entry;
			}
			
			//if (pFirstEntry)
#ifdef AUTO_PVC_SEARCH_TR068_OAMPING
				startAutoPVCSearchTR068OAMPing_F5_Segment(pFirstEntry, map);
				if ( sok == 0 ) {
					setVC0Connection(pFirstEntry->vpi, pFirstEntry->vci);
					startAutoPVCSearchTR068OAMPing_F5_EndToEnd(pFirstEntry, map);
				}
#elif defined(AUTO_PVC_SEARCH_PURE_OAMPING)
				startAutoPVCSearchPureOAMPing_F5_Segment(pFirstEntry, map);
				if ( sok == 0 ){
					setVC0Connection(pFirstEntry->vpi, pFirstEntry->vci);
					startAutoPVCSearchPureOAMPing_F5_EndToEnd(pFirstEntry, map);
				}
#else
				printf("Just only do AutoHunt to find PVC\n");
#endif
			// Timer for auto search PVC			
			fp = fopen("/tmp/autoPVC", "w+");
			if(fp){
				fprintf(fp, "OAMPing:%d AutoHUNT:0\n", sok);
				fclose(fp);
			}			 	
			

#ifdef AUTO_PVC_SEARCH_AUTOHUNT
			//OAM loopback not found, start SAR driver auto-pvc-search
			if(sok==0){
				printf("***** startRest: call SAR driver to start autoHunt ****\n");
				// Kaohj
				StartAutoHunt();				
				//cmd_start_autohunt();
				
				// Timer for auto search PVC
				fp = fopen("/tmp/autoPVC", "w+");
				if(fp){
					fprintf(fp, "OAMPing:0 AutoHUNT:1\n");
					fclose(fp);
				}
				
		        }   
		} else {
		
			StopSarAutoPvcSearch(0,0);
			return -1;
		}		  
#else
        	} else {
        		return -1;
        	}
             
#endif
	}
	return 1;
}
	

#endif

void restartWAN()
{

//eason path for pvc add/del and conntrack killall
#if 1
	va_cmd(IFCONFIG, 2, 1, ELANIF, "down");
	va_cmd(IFCONFIG, 2, 1, BRIF, "down");	
	va_cmd("/bin/ethctl", 2, 1, "conntrack", "killall");
	va_cmd(IFCONFIG, 2, 1, ELANIF, "up");		
	va_cmd(IFCONFIG, 2, 1, BRIF, "up");
#endif



	RegisterPVCnumber();		
	cleanAllFirewallRule();
	restart_dnsrelay();
#ifdef CONFIG_EXT_SWITCH
#ifdef VLAN_GROUP
	setupEth2pvc_vlan_disable();
#endif
#endif
	startWan(BOOT_LAST);
#ifdef CONFIG_EXT_SWITCH
#ifdef VLAN_GROUP
	setupEth2pvc_vlan();
#endif
#endif
}

#ifdef IP_PASSTHROUGH
void set_IPPT_LAN_access()
{
	unsigned char ippt_itf, value[32];
	
	// IP Passthrough, LAN access
	if (mib_get(MIB_IPPT_ITF, (void *)&ippt_itf) != 0)
		if (ippt_itf != 255) {	// IP passthrough
			mib_get(MIB_IPPT_LANACC, (void *)value);
			if (value[0] == 0)	// disable LAN access
				// iptables -A FORWARD -i $LAN_IF -o $LAN_IF -j DROP
				va_cmd(IPTABLES, 8, 1, (char *)FW_ADD,
					(char *)FW_FORWARD, (char *)ARG_I,
					(char *)LANIF, (char *)ARG_O,
					(char *)LANIF, "-j", (char *)FW_DROP);
		}
}

void clean_IPPT_LAN_access()
{	
	// iptables -D FORWARD -i $LAN_IF -o $LAN_IF -j DROP
	va_cmd(IPTABLES, 8, 1, (char *)FW_DEL,
		(char *)FW_FORWARD, (char *)ARG_I,
		(char *)LANIF, (char *)ARG_O,
		(char *)LANIF, "-j", (char *)FW_DROP);		
}

// Mason Yu
void restartIPPT(struct ippt_para para)
{
	unsigned int entryNum, i, idx;
	MIB_CE_ATM_VC_T Entry;
	FILE *fp;	
	int restar_dhcpd_flag=0;
	unsigned long myipaddr, hisipaddr;
	char pppif[6], globalIP_str[16];
	
	//printf("Take effect for IPPT and old_ippt_itf=%d new_ippt_itf=%d\n", para.old_ippt_itf, para.new_ippt_itf);
	//printf("Take effect for IPPT and old_ippt_lease=%d new_ippt_lease=%d\n", para.old_ippt_lease, para.new_ippt_lease);
	//printf("Take effect for IPPT and old_ippt_lanacc=%d new_ippt_lanacc=%d\n", para.old_ippt_lanacc, para.new_ippt_lanacc);	
       	
       	// Stop IPPT
       	// If old_ippt_itf != 255 and new_ippt_itf != old_ippt_itf, it is that the IPPT is enabled. We should clear some configurations.	
	if ( para.old_ippt_itf != 255  && para.new_ippt_itf != para.old_ippt_itf) {
		// (1)  set restart DHCP server flag with 1.
		restar_dhcpd_flag = 1;  // on restart DHCP server flag
		
		// (2) Delete /tmp/PPPHalfBridge file for DHCP Server
       		fp = fopen("/tmp/PPPHalfBridge", "r");
       		if (fp) {
       			fread(&myipaddr, 4, 1, fp);	 	         
	 	       	fread(&hisipaddr, 4, 1, fp);
       			unlink("/tmp/PPPHalfBridge");
       			fclose(fp);       			
       		}
       	
       		// (3) Delete /tmp/IPoAHalfBridge file for DHCP Server
       		fp = fopen("/tmp/IPoAHalfBridge", "r");
       		if (fp) {
       			fread(&myipaddr, 4, 1, fp);	 	         
	 	       	fread(&hisipaddr, 4, 1, fp);
       			unlink("/tmp/IPoAHalfBridge");
       			fclose(fp);
       		}		
		
		// Change Public IP to string
		snprintf(globalIP_str, 16, "%d.%d.%d.%d", (int)(myipaddr>>24)&0xff, (int)(myipaddr>>16)&0xff, (int)(myipaddr>>8)&0xff, (int)(myipaddr)&0xff);		
		
		// (4) Delete LAN IPPT interface
		va_cmd(IFCONFIG, 2, 1, (char*)LAN_IPPT,"down");
		
		// (5) Delete a public IP's route		
               	va_cmd(ROUTE, 5, 1, ARG_DEL, "-host", globalIP_str, "dev", LANIF);
               	
		// (6) Restart the previous IPPT WAN connection.
		entryNum = mib_chain_total(MIB_ATM_VC_TBL);			
		for (i = 0; i < entryNum; i++) {				
			/* Retrieve entry */
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) {
				printf("restartIPPT: cannot get ATM_VC_TBL-1(ch=%d) entry\n", i);
				return 0;
			}
        	
			/* remove connection on driver*/
			if (para.old_ippt_itf == Entry.ifIndex) {		
				stopConnection(&Entry);
				
				// If this connection is PPPoE/oA, we should kill the old NAPT rule in POSTROUTING chain.				
				// When channel mode is rt1483, NAPT rule will be deleted by stopConnection().
				if (Entry.cmode == ADSL_PPPoE || Entry.cmode == ADSL_PPPoA) {					
					snprintf(pppif, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));					
        				va_cmd(IPTABLES, 10, 1, "-t", "nat", FW_DEL, "POSTROUTING",
			 			"-o", pppif, "-j", "SNAT", "--to-source", globalIP_str);			 					 		      				
				}				
				
				startConnection(&Entry);								
				break;
			}			
		}		 		
	}
	
	// Start New IPPT
	if ( para.new_ippt_itf != 255 && para.new_ippt_itf != para.old_ippt_itf) {
		// (1)  set restart DHCP server flag with 1.
		restar_dhcpd_flag = 1;  // on restart DHCP server flag
		
		// (2) Config WAN interface and reconnect to DSL.
		entryNum = mib_chain_total(MIB_ATM_VC_TBL);			
		for (i = 0; i < entryNum; i++) {				
			/* Retrieve entry */
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) {
				printf("restartIPPT: cannot get ATM_VC_TBL-2(ch=%d) entry\n", i);
				return 0;
			}
			
			if (para.new_ippt_itf == Entry.ifIndex) {
				// If this connection is PPPoE/oA, we should kill the old NAPT rule in POSTROUTING chain.
				// When channel mode is rt1483, NAPT rule will be deleted by stopConnection().
				if ( (Entry.cmode == ADSL_PPPoE || Entry.cmode == ADSL_PPPoA)) {
					snprintf(pppif, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));
					va_cmd(IPTABLES, 8, 1, "-t", "nat", FW_DEL, "POSTROUTING",
			 			"-o", pppif, "-j", "MASQUERADE");
				}				
				
				stopConnection(&Entry);
				startConnection(&Entry);				
				break;
			}
		}			
	}  //  if ( new_ippt_itf != 255 )
	
	// Check IPPT Lease Time
	// Here we just concern about IPPT is enable.
	if ( para.old_ippt_lease != para.new_ippt_lease && para.new_ippt_itf != 255) {
		restar_dhcpd_flag = 1;  // on restart DHCP server flag
		//printf("change IPPT Lease Time\n");
	}		
	
	// Check IPPT LAN Access
	if ( para.new_ippt_itf != 255 || para.old_ippt_itf != 255) {
		if ( para.old_ippt_lanacc == 0 && para.old_ippt_itf != 255)
			clean_IPPT_LAN_access();
		set_IPPT_LAN_access();				
	} 
	
	// Restart DHCP Server
	if ( restar_dhcpd_flag == 1 ) {
		restar_dhcpd_flag = 0;  // off restart DHCP server flag
		restart_dhcp();	
	}

}
#endif

#ifdef DOS_SUPPORT
void setup_dos_protection()
{
	unsigned int dos_enable;
	unsigned int dos_syssyn_flood;
	unsigned int dos_sysfin_flood;
	unsigned int dos_sysudp_flood;
	unsigned int dos_sysicmp_flood;
	unsigned int dos_pipsyn_flood;
	unsigned int dos_pipfin_flood;
	unsigned int dos_pipudp_flood;
	unsigned int dos_pipicmp_flood;
	unsigned int dos_block_time;
	unsigned char buffer[256],dosstr[256];
	int dostmpip[4],dostmpmask[4];
	int dosip,dosmask;
	FILE *dosfp;
	
	if (!mib_get( MIB_DOS_ENABLED,  (void *)&dos_enable)){
		printf("DOS parameter failed!\n");
	}
	if (!mib_get( MIB_DOS_SYSSYN_FLOOD,  (void *)&dos_syssyn_flood)){
		printf("DOS parameter failed!\n");
	}
	if (!mib_get( MIB_DOS_SYSFIN_FLOOD,  (void *)&dos_sysfin_flood)){
		printf("DOS parameter failed!\n");
	}
	if (!mib_get( MIB_DOS_SYSUDP_FLOOD,  (void *)&dos_sysudp_flood)){
		printf("DOS parameter failed!\n");
	}
	if (!mib_get( MIB_DOS_SYSICMP_FLOOD,  (void *)&dos_sysicmp_flood)){
		printf("DOS parameter failed!\n");
	}
	if (!mib_get( MIB_DOS_PIPSYN_FLOOD,  (void *)&dos_pipsyn_flood)){
		printf("DOS parameter failed!\n");
	}
	if (!mib_get( MIB_DOS_PIPFIN_FLOOD,  (void *)&dos_pipfin_flood)){
		printf("DOS parameter failed!\n");
	}
	if (!mib_get( MIB_DOS_PIPUDP_FLOOD,  (void *)&dos_pipudp_flood)){
		printf("DOS parameter failed!\n");
	}
	if (!mib_get( MIB_DOS_PIPICMP_FLOOD,  (void *)&dos_pipicmp_flood)){
		printf("DOS parameter failed!\n");
	}
	if (!mib_get( MIB_DOS_BLOCK_TIME,  (void *)&dos_block_time)){
		printf("DOS parameter failed!\n");
	}
	
	//get ip
	if(!mib_get( MIB_ADSL_LAN_IP, (void *)buffer))
		return -1;
	sscanf(inet_ntoa(*((struct in_addr *)buffer)),"%d.%d.%d.%d",&dostmpip[0],&dostmpip[1],&dostmpip[2],&dostmpip[3]);	
	dosip= dostmpip[0]<<24 | dostmpip[1]<<16 | dostmpip[2]<<8 | dostmpip[3];
	//get mask
	if(!mib_get( MIB_ADSL_LAN_SUBNET, (void *)buffer))
		return -1;
	sscanf(inet_ntoa(*((struct in_addr *)buffer)),"%d.%d.%d.%d",&dostmpmask[0],&dostmpmask[1],&dostmpmask[2],&dostmpmask[3]);	
	dosmask= dostmpmask[0]<<24 | dostmpmask[1]<<16 | dostmpmask[2]<<8 | dostmpmask[3];
	dosip &= dosmask;
	sprintf(dosstr,"1 %x %x %d %d %d %d %d %d %d %d %d %d\n",
		dosip,dosmask,dos_enable,dos_syssyn_flood,dos_sysfin_flood,dos_sysudp_flood,
		dos_sysicmp_flood,dos_pipsyn_flood,dos_pipfin_flood,dos_pipudp_flood,
		dos_pipicmp_flood,dos_block_time);
	//printf("dosstr:%s\n\n",dosstr);
	dosstr[strlen(dosstr)]=NULL;
	if (dosfp = fopen("/proc/enable_dos", "w"))
	{
			fprintf(dosfp, "%s",dosstr);
			fclose(dosfp);
	}	
}
#endif

#ifdef MULTI_IC_SUPPORT
/*
* ql:20080729 START: get register value(4 bytes)
* parameter: addr - register address
*/
int getRegValue(unsigned int addr)
{
	int skfd;
	struct atmif_sioc mysio;
	unsigned int ptr[3];

	if((skfd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0){
		perror("socket open error");
		return -1;
	}

	ptr[0] = addr;
	ptr[1] = 4;
	mysio.number = 0;
	mysio.length = sizeof(struct SAR_IOCTL_CFG);
	mysio.arg = (void *)ptr;
	if ((ioctl(skfd, SAR_READ_MEM, &mysio))<0) {
		printf("read reg %08X failed!\n", addr);
		close(skfd);
		return -1;
	}
	
	close(skfd);
	
	return(ptr[2]);
}

/*
* ql: 20080729 START: use getImgKey() to gain the right key of relevant IC version.
*/
int getImgKey()
{
	unsigned int key;
	unsigned int reg_clk;
	unsigned short sachem_ver_reg;
	int icVer;

	sachem_ver_reg = (unsigned short)getRegValue(0xb8600020);
	if (-1 == sachem_ver_reg)
		return 0;
	
	if (!sachem_ver_reg) {//I think it must be 8671, read value to ensure
		sachem_ver_reg = (unsigned short)getRegValue(0xb8000020);
		switch(sachem_ver_reg) {
			case 0x1a08:
			case 0x081a:
			case 0x1a0a:
			case 0x0a1a:
			case 0x1a1a:
			case 0x1a2a:
			case 0x2a1a:
				icVer = IC8671;
				break;
			default:
				printf("unknown sachem version!\n");
				icVer = 0;
				break;
		}
	} else {
		unsigned int reg_clk;
		
		reg_clk = getRegValue(0xB8003200);
		if (-1 == reg_clk)
			icVer = 0;
		else if ((reg_clk & 0x00001f00) == 0)
			icVer = IC8672;
		else if ((reg_clk & 0x00100000) == 0x00100000)
			icVer = IC8671B;
		else
			icVer = IC8671B_costdown;
	}
	
	switch(icVer)
	{
		case IC8671:
			key = APPLICATION_IMG_8671;
			break;
		//case IC8671P:
		//	key = APPLICATION_IMG_8671P;
		//	break;
		case IC8672:
			key = APPLICATION_IMG_8672;
			break;
		case IC8671B:
			key = APPLICATION_IMG_8671B;
			break;
		case IC8671B_costdown:
			key = APPLICATION_IMG_8671B_CD;
			break;
		default:
			/*ql:20080729 START: if sachem register read fail, then don't check image key*/
			//key = 0;
			key = APPLICATION_IMG_ALL;
			/*ql:20080729 END*/
			break;
	}
	printf("%s:%08X\n", __FUNCTION__, key);

	return(key);
}
#endif

/*ql: 20080729 END*/

int pppdbg_get(int unit)
{
	char buff[256];
	FILE *fp;
	int pppinf, pppdbg = 0;

	if (fp = fopen(PPP_DEBUG_LOG, "r")) {
		while (fgets(buff, sizeof(buff), fp) != NULL) {
			if (sscanf(buff, "%d:%d", &pppinf, &pppdbg) != 2)
				break;
			else {
				if (pppinf == unit)
					break;
			}
		}
		fclose(fp);
	}
	return pppdbg;
}

//cathy, update pvc link time
struct sysinfo * updateLinkTime(unsigned char update){
	static struct sysinfo info;
	
	if(update) {	// down --> up		
		sysinfo(&info);	
	}
	
	return &info;
}
