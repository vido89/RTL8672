/*
 *      Include file of utility.c
 *
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *      Authors: Dick Tam	<dicktam@realtek.com.tw>
 *
 *
 */

#ifndef INCLUDE_UTILITY_H
#define INCLUDE_UTILITY_H

#include <net/if.h>
#include <stdarg.h>
#include <stdio.h>
#include <net/route.h>
#include "adslif.h"
#include "mib.h"
#include "sysconfig.h"

//jim 
#include "options.h"

// defined to use pppd, otherwise, use spppd if not defined
//#define USE_PPPD

#ifdef CONFIG_USER_PPPOE_PROXY
#include <linux/config.h>
#include <net/if.h>		/* ifreq struct         */
#include <net/if_arp.h>
#include <net/route.h>
#include <netinet/in.h>    
#include <linux/if_ether.h>
#include <linux/if_pppox.h>
#include <linux/ppp_defs.h>
#include <linux/if_ppp.h>
//extern int ppp_proxy_ioctl(pppoe_proxy *if_pp_proxy,int cmd );
extern int has_pppoe_init;
#endif

#define BUF_SIZE		256
#define MAX_POE_PER_VC		5
struct data_to_pass_st {
	int	id;
	char data[BUF_SIZE];
};

// Mason Yu. For Set IPQOS
#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
#define		SETIPQOS		0x01

/*
 * Structure used in SIOCSIPQOS request.
 */

struct ifIpQos
{
	int	cmd;
	char	enable;	
};
#endif

// Mason Yu
#ifdef IP_PASSTHROUGH
struct ippt_para
{
	unsigned char old_ippt_itf;
	unsigned char new_ippt_itf;
	unsigned char old_ippt_lanacc;
	unsigned char new_ippt_lanacc;
	unsigned int old_ippt_lease;
	unsigned int new_ippt_lease;
};
#endif

#if defined(CONFIG_EXT_SWITCH) || defined(CONFIG_RTL867X_COMBO_PORTMAPPING)
#define		VLAN_ENABLE		0x01
#define		VLAN_SETINFO		0x02
#define		VLAN_SETPVIDX		0x03
#define		VLAN_SETTXTAG		0x04
#define		VLAN_DISABLE1PPRIORITY	0x05
#define		VLAN_SETIGMPSNOOP	0x06
#define		VLAN_SETPORTMAPPING	0x07
#define		VLAN_SETIPQOS		0x08
#define		VLAN_VIRTUAL_PORT	0x09
#define		VLAN_SETVLANGROUPING	0x0a

#ifdef CONFIG_IGMP_FORBID
#define           IGMP_FORBID              0x0a
#endif
#define		TAG_DCARE	0x03
#define		TAG_ADD		0x02
#define		TAG_REMOVE	0x01
#define		TAG_REPLACE	0x00

/*
 * Structure used in SIOCSIFVLAN request.
 */

struct ifvlan
{
	int	cmd;
	char	enable;
	short	vlanIdx;
	short	vid;
	char		disable_priority;
	int	member;
	int	port;
	char	txtag;
};

struct brmap {
	int	brid;
	unsigned char pvcIdx;
};

extern const int virt2user[];

// Mason Yu
enum PortMappingGrp
{
	PM_DEFAULTGRP=0,
	PM_GROUP1=1,
	PM_GROUP2=2,
	PM_GROUP3=3,
	PM_GROUP4=4
};

enum PortMappingAction
{
	PM_PRINT=0,
	PM_ADD=1,
	PM_REMOVE=2	
};
#endif

struct itfInfo
{
	#define	DOMAIN_ELAN	0x1
	#define	DOMAIN_WAN	0x2
	#define	DOMAIN_WLAN	0x4
	#define	DOMAIN_ULAN	0x8	//usbeth
	int	ifdomain;
	int	ifid;
	char	name[40];// changed by jim
};

// IF_ID(domain, id)
#define IF_ID(x, y)		(x<<8)|y
#define IF_DOMAIN(x)		(x>>8)
#define IFGROUP_NUM		5


//#ifdef CONFIG_USBCLIENT
#ifdef CONFIG_RE8306
#define IFWLAN_SHIFT		6
#else
#define IFWLAN_SHIFT		5
#endif

#ifdef WLAN_MBSSID
#define WLAN_MBSSID_NUM		4
#else
#define WLAN_MBSSID_NUM		0
#endif

#ifdef CONFIG_USB_ETH
#define IFUSBETH_SHIFT		(IFWLAN_SHIFT+WLAN_MBSSID_NUM+1)
#define IFUSBETH_PHYNUM		(SW_PORT_NUM+5+1)  //for ipqos.phyPort (5: wlanphy max, 1:usb0)
#endif //CONFIG_USB_ETH


#define	IPQOS_NUM_PKT_PRIO	8
#define	IPQOS_NUM_PRIOQ		4

struct mymsgbuf;

typedef enum { IP_ADDR, DST_IP_ADDR, SUBNET_MASK, DEFAULT_GATEWAY, HW_ADDR } ADDR_T;
typedef enum {
	SYS_UPTIME,
	SYS_DATE,
	SYS_YEAR,
	SYS_MONTH,
	SYS_DAY,
	SYS_HOUR,
	SYS_MINUTE,
	SYS_SECOND,
	SYS_FWVERSION,
	SYS_LAN_DHCP,
	SYS_DHCP_LAN_IP,
	SYS_DHCP_LAN_SUBNET,
	SYS_DHCPS_IPPOOL_PREFIX,
	SYS_DNS_MODE,
	SYS_WLAN,
	SYS_WLAN_BAND,
	SYS_WLAN_AUTH,
	SYS_WLAN_PREAMBLE,
	SYS_WLAN_BCASTSSID,
	SYS_WLAN_ENCRYPT,
	SYS_WLAN_PSKFMT,
	SYS_WLAN_PSKVAL,
	SYS_WLAN_WEP_KEYLEN,
	SYS_WLAN_WEP_KEYFMT,
	SYS_WLAN_WPA_MODE,
	SYS_WLAN_RSPASSWD,
	SYS_TX_POWER,
	SYS_WLAN_MODE,
	SYS_WLAN_TXRATE,
	SYS_WLAN_BLOCKRELAY,
	SYS_WLAN_AC_ENABLED,
	SYS_WLAN_WDS_ENABLED,
	SYS_WLAN_QoS,
	SYS_DHCP_MODE,
	SYS_IPF_OUT_ACTION,
	SYS_DEFAULT_PORT_FW_ACTION,
	SYS_MP_MODE,
	SYS_IGMP_SNOOPING,
	SYS_PORT_MAPPING,
	SYS_IP_QOS,
	SYS_IPF_IN_ACTION,
	SYS_WLAN_BLOCK_ETH2WIR,
	SYS_DNS_SERVER,
	SYS_LAN_IP2,
	SYS_LAN_DHCP_POOLUSE,
	SYS_DEFAULT_URL_BLK_ACTION,
	SYS_DEFAULT_DOMAIN_BLK_ACTION,
	SYS_DSL_OPSTATE,
        SYS_VENDOR,
        SYS_MODEL,
        SYS_VERSION
} SYSID_T;

// enumeration of user process bit-shift for process bit-mapping
typedef enum {
	PID_DNSMASQ=0,
	PID_SNMPD,
	PID_WEB,
	PID_CLI,
	PID_DHCPD,
	PID_DHCPRELAY,
	PID_INETD,
	PID_TELNETD,
	PID_FTPD,
	PID_TFTPD,
	PID_SSHD,
	PID_SYSLOGD,
	PID_KLOGD,
	PID_IGMPPROXY,
	PID_RIPD,
	PID_WATCHDOGD,
	PID_SNTPD,
	PID_MPOAD,
	PID_SPPPD,
	PID_UPNPD,
	PID_UPDATEDD,
	PID_CWMP /*tr069/cwmpClient pid,jiunming*/
} PID_SHIFT_T;



#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
extern unsigned int ZTE_Bridge_Config_Keyword;
extern unsigned int ZTE_Router_Config_Keyword;
void ZTE_Encrypt(char * original, char *encrypted, char initial, int len);


void ZTE_Deencrypt(char * original, char *encrypted, char initial, int len);
extern int prio2QoS[4]; //mapping priority to ATM QoS     0,1---> CBR (1); 2 --->rtvbr(2); 3---->ubr(0). 
extern int QoS2Prio[4];
#endif
enum PortMappingPriority
{
	HighestPrio=0,
	HighPrio=1,
	MediumPrio=2,
	lowPrio=3
};


//added by xl_yue: supporting wlcountry list
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
struct wlCountryReg{
	char country[32];
	int regDomain;
};
extern struct wlCountryReg wlCountryRegList[];
#endif


#define		PID_SHIFT(x)		(1<<x)
#define		NET_PID			PID_SHIFT(PID_MPOAD)|PID_SHIFT(PID_SPPPD)
#define		ALL_PID			0xffffffff & ~(NET_PID)

int getInAddr(char *interface, ADDR_T type, void *pAddr);
int getInFlags(char *interface, int *flags );
int setInFlags(char *interface, int flags );
int INET_resolve(char *name, struct sockaddr *sa);
int INET_addroute(struct rtentry *rt);
short Max(short* array, int len);
short Min(short* array, int len);
int read_pid(char *filename);
int getLinkStatus(struct ifreq *ifr);

#define MAX_CONFIG_FILESIZE 200000
// Added by Kaohj
extern const char LANIF[];
extern const char LAN_ALIAS[];	// alias for secondary IP
extern const char LAN_IPPT[];	// alias for IP passthrough
extern const char WLANIF[];
extern const char ELANIF[];
extern const char BRIF[];
extern const char VC_BR[];
extern const char LLC_BR[];
extern const char VC_RT[];
extern const char BLANK[];
extern const char LLC_RT[];
extern const char PORT_DHCP[];
extern const char ARG_ADD[];
extern const char ARG_ENCAPS[];
extern const char ARG_QOS[];
extern const char ARG_255x4[];
extern const char ARG_0x4[];
extern const char ARG_BKG[];
extern const char ARG_I[];
extern const char ARG_O[];
extern const char ARG_T[];
extern const char ARG_TCP[];
extern const char ARG_UDP[];
//#ifdef ZTE_GENERAL_ROUTER_SC
#if defined(ZTE_GENERAL_ROUTER_SC) || defined(NEW_IP_QOS_SUPPORT)
extern const char ARG_TCPUDP[];
#endif
extern const char ARG_ICMP[];
extern const char FW_BLOCK[];
extern const char FW_INACC[];
extern const char FW_IPFILTER[];
extern const char PORT_FW[];
extern const char IPTABLE_DMZ[];
extern const char IPTABLE_IPFW[];
extern const char IPTABLE_IPFW2[];
extern const char FW_MACFILTER[];
extern const char FW_DROP[];
extern const char FW_ACCEPT[];
extern const char FW_RETURN[];
extern const char FW_FORWARD[];
extern const char FW_INPUT[];
extern const char FW_PREROUTING[];
extern const char FW_DPORT[];
extern const char FW_SPORT[];
extern const char FW_ADD[];
extern const char FW_DEL[];
#ifdef PORT_FORWARD_ADVANCE
extern const char FW_PPTP[];
extern const char FW_L2TP[];
extern const char *PFW_Gategory[];
extern const char *PFW_Rule[];
int config_PFWAdvance( int action_type );
#endif
extern const char RMACC_MARK[];
extern const char CONFIG_HEADER[];
extern const char CONFIG_TRAILER[];
extern const char CONFIG_XMLFILE[];
extern const char CONFIG_XMLENC[];
extern const char PPP_SYSLOG[];
extern const char PPP_DEBUG_LOG[];

extern const char ADSLCTRL[];
extern const char IFCONFIG[];
extern const char IWPRIV[];
extern const char BRCTL[];
extern const char MPOAD[];
extern const char MPOACTL[];
extern const char DHCPD[];
extern const char DHCPC[];
extern const char DNSRELAY[];
extern const char SPPPD[];
extern const char SPPPCTL[];
extern const char WEBSERVER[];
extern const char UPGRADE_WEB[];
extern const char SNMPD[];
extern const char ROUTE[];
extern const char IPTABLES[];
/*ql 20081114 START need ebtables support*/
extern const char EBTABLES[];
extern const char ZEBRA[];
#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
extern const char OSPFD[];
#endif
extern const char RIPD[];
extern const char ROUTED[];
extern const char IGMPROXY[];
extern const char TC[];
#ifdef WLAN_SUPPORT
extern const char IWPRIV[];
extern const char AUTH[];
extern const char IWCONTROL[];
extern const char AUTH_PID[];
#endif
extern const char NETLOGGER[];
#if defined(TIME_ZONE) || defined(ZTE_GENERAL_ROUTER_SC)
extern const char SNTPC[];
extern const char SNTPC_PID[];
#endif

extern const char PROC_DYNADDR[];
extern const char PROC_IPFORWARD[];
extern const char PROC_FORCE_IGMP_VERSION[];
extern const char PPPD_FIFO[];
extern const char MPOAD_FIFO[];
#ifdef WLAN_SUPPORT
extern const char WLAN_AUTH_CONF[];
#endif
extern const char STR_DISABLE[];
extern const char STR_ENABLE[];
extern const char STR_UNNUMBERED[];
extern const char rebootWord0[];
extern const char rebootWord1[];
extern const char rebootWord2[];
extern const char errGetEntry[];
extern const char MER_GWINFO[];

extern const char *n0to7[];
extern const char *prioLevel[];
extern const int priomap[];;
extern const char *ipTos[];
//alex
#ifdef CONFIG_8021P_PRIO
extern const char *set1ptable[];
#ifdef NEW_IP_QOS_SUPPORT
extern const char *setpredtable[];
#endif
#endif

#ifdef CONFIG_USB_ETH
extern const char USBETHIF[];
#endif //CONFIG_USB_ETH

#ifdef WLAN_WDS
extern const char WDSIF[];
#endif
extern const char STR_NULL[];
extern const char DHCPC_PID[];
extern const char DHCPD_LEASE[];
extern const char DHCPSERVERPID[];
#ifdef XOR_ENCRYPT
//Jenny, Configuration file encryption
extern const char XOR_KEY[];
void xor_encrypt(char *inputfile, char *outputfile);
#endif

#ifdef CONFIG_EXT_SWITCH
void __dev_setupVirtualPort(int flag);
void __dev_register_swport(void);
void __dev_setupIGMPSnoop(int flag);	// enable/disable IGMP snooping
void __dev_setupPortMapping(int flag);
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
void setgroup(char *list, int grpnum,  enum PortMappingPriority priority);
#else
#ifdef ITF_GROUP
void setgroup(char *list, int grpnum);
#else
#ifdef VLAN_GROUP
void setgroup(char *list, int grpnum, int vid, int dhcps);
#endif
#endif	// of ITF_GROUP
#endif	// of ZTE_531B_BRIDGE_SC ...
int get_group_ifinfo(struct itfInfo *info, int len, int grpnum);
#endif
void __dev_setupIPQoS(int flag);
int get_domain_ifinfo(struct itfInfo *info, int len, int ifdomain);

int do_cmd(const char *filename, char *argv [], int dowait);
int va_cmd(const char *cmd, int num, int dowait, ...);  //return 0:OK, other:fail
int va_cmd_no_echo(const char *cmd, int num, int dowait, ...);  //return 0:OK, other:fail
int call_cmd(const char *filename, int num, int dowait, ...);	//return 0:OK, other:fail
void write_to_pppd(struct data_to_pass_st *);
int write_to_mpoad(struct data_to_pass_st *);
int startDhcpc(char *inf, MIB_CE_ATM_VC_Tp pEntry);
int startIP(char *inf, MIB_CE_ATM_VC_Tp pEntry, REMOTE_ACCESS_T ipEncap);
void stopPPP(void);
int startPPP(char *inf, MIB_CE_ATM_VC_Tp pEntry, char *qos, REMOTE_ACCESS_T pppEncap);
int find_ppp_from_conf(char *pppif);
int get_classification_mark(int entryNo);
int startConnection(MIB_CE_ATM_VC_Tp pEntry);
void stopConnection(MIB_CE_ATM_VC_Tp pEntry);
int msgProcess(struct mymsgbuf *qbuf);
int flashdrv_filewrite(FILE *fp,int size,void  *dstP);
#ifdef CONFIG_USER_IGMPPROXY
int startIgmproxy(void);
#endif
void addStaticRoute(void);
void deleteStaticRoute(void);

#ifdef WLAN_SUPPORT
int getWlStaInfo( char *interface,  WLAN_STA_INFO_Tp pInfo );
int setupWDS(void);
int setupWLan(void);
void restartWlan(void);
int config_WLAN( int action_type );
#endif

char adsl_drv_get(unsigned int id, void *rValue, unsigned int len);
int getMIB2Str(unsigned int id, char *str);
int getSYS2Str(SYSID_T id, char *str);
int ifWanNum(const char *type); /* type="all", "rt", "br" */
void filter_set_remote_access(int enable);
#ifdef IP_ACL
void filter_set_acl(int enable);
#endif
#ifdef NAT_CONN_LIMIT
int restart_connlimit(void);
void set_conn_limit(void);
#endif
#ifdef TCP_UDP_CONN_LIMIT
int restart_connlimit(void);
void set_conn_limit(void);
#endif
#ifdef URL_BLOCKING_SUPPORT
void filter_set_url(int enable);
#endif
#ifdef URL_ALLOWING_SUPPORT
void  set_url(int enable);
#endif
void itfcfg(char *if_name, int up_flag);
#ifdef DOMAIN_BLOCKING_SUPPORT
void filter_set_domain(int enable);
#endif
int startRip(void);
#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
int startOspf(void);
#endif


#ifdef WLAN_CLIENT
#define SSID_LEN	32
#define	MAX_BSS_DESC	64
typedef struct _OCTET_STRING {
    unsigned char *Octet;
    unsigned short Length;
} OCTET_STRING;
typedef enum _BssType {
    infrastructure = 1,
    independent = 2,
} BssType;
typedef	struct _IbssParms {
    unsigned short	atimWin;
} IbssParms;
typedef struct _BssDscr {
    unsigned char bdBssId[6];
    unsigned char bdSsIdBuf[SSID_LEN];
    OCTET_STRING  bdSsId;
    BssType bdType;
    unsigned short bdBcnPer;			// beacon period in Time Units
    unsigned char bdDtimPer;			// DTIM period in beacon periods
    unsigned long bdTstamp[2];			// 8 Octets from ProbeRsp/Beacon
    IbssParms bdIbssParms;			// empty if infrastructure BSS
    unsigned short bdCap;				// capability information
    unsigned char ChannelNumber;			// channel number
    unsigned long bdBrates;
    unsigned long bdSupportRates;		
    unsigned char bdsa[6];			// SA address
    unsigned char rssi, sq;			// RSSI and signal strength
    unsigned char network;			// 1: 11B, 2: 11G, 4:11G
} BssDscr, *pBssDscr;
typedef struct _sitesurvey_status {
    unsigned char number;
    unsigned char pad[3];
    BssDscr bssdb[MAX_BSS_DESC];
} SS_STATUS_T, *SS_STATUS_Tp;
typedef enum _Capability {
    cESS 		= 0x01,
    cIBSS		= 0x02,
    cPollable		= 0x04,
    cPollReq		= 0x01,
    cPrivacy		= 0x10,
    cShortPreamble	= 0x20,
} Capability;
typedef enum _Synchronization_Sta_State{
    STATE_Min		= 0,
    STATE_No_Bss	= 1,
    STATE_Bss		= 2,
    STATE_Ibss_Active	= 3,
    STATE_Ibss_Idle	= 4,
    STATE_Act_Receive	= 5,
    STATE_Pas_Listen	= 6,
    STATE_Act_Listen	= 7,
    STATE_Join_Wait_Beacon = 8,
    STATE_Max		= 9
} Synchronization_Sta_State;
typedef enum _wlan_mac_state {
    STATE_DISABLED=0, STATE_IDLE, STATE_SCANNING, STATE_STARTED, STATE_CONNECTED, STATE_WAITFORKEY
} wlan_mac_state;
typedef struct _bss_info {
    unsigned char state;
    unsigned char channel;
    unsigned char txRate;
    unsigned char bssid[6];
    unsigned char rssi, sq;	// RSSI  and signal strength
    unsigned char ssid[SSID_LEN+1];
} bss_info;

#endif	// of WLAN_CLIENT
#ifdef WLAN_WDS
typedef enum _wlan_wds_state {
    STATE_WDS_EMPTY=0, STATE_WDS_DISABLED, STATE_WDS_ACTIVE
} wlan_wds_state;

typedef struct _WDS_INFO {
	unsigned char	state;
	unsigned char	addr[6];
	unsigned long	tx_packets;
	unsigned long	rx_packets;
	unsigned long	tx_errors;
	unsigned char	txOperaRate;
} WDS_INFO_T, *WDS_INFO_Tp;

#endif //WLAN_WDS

/*ql:20080729 START: use icVer to record the version of IC*/
#ifdef MULTI_IC_SUPPORT
#define IC8671				1
//#define IC8671P				2
#define IC8672				3
#define IC8671B				4
#define IC8671B_costdown	5
#endif
/*ql:20080729 END*/

typedef struct {
	unsigned int	key;		/* magic key */

#define BOOT_IMAGE             0xB0010001
#define CONFIG_IMAGE           0xCF010002
/*ql:20080729 START: different IC with different IMG KEY*/
#ifndef MULTI_IC_SUPPORT
#define APPLICATION_IMAGE      0xA0000003
#else
#define APPLICATION_IMG_8671       0xA0000003
//#define APPLICATION_IMG_8671P      0xA0000002
#define APPLICATION_IMG_8672       0xA0000004
#define APPLICATION_IMG_8671B      0xA0000008
#define APPLICATION_IMG_8671B_CD   0xA0000010
/*ql:20080729 START: if sachem register read fail, then don't check image key*/
#define APPLICATION_IMG_ALL        0xA0000000
/*ql:20080729 END*/
#endif
/*ql:20080729 END*/
#define BOOTPTABLE             0xB0AB0004


	unsigned int	address;	/* image loading DRAM address */
	unsigned int	length;		/* image length */
	unsigned int	entry;		/* starting point of program */
	unsigned short	chksum;		/* chksum of */
	
	unsigned char	type;
#define KEEPHEADER    0x01   /* set save header to flash */
#define FLASHIMAGE    0x02   /* flash image */
#define COMPRESSHEADER    0x04       /* compress header */
#define MULTIHEADER       0x08       /* multiple image header */
#define IMAGEMATCH        0x10       /* match image name before upgrade */
	
	
	unsigned char	   date[25];  /* sting format include 24 + null */
	unsigned char	   version[16];
        unsigned int  *flashp;  /* pointer to flash address */

} IMGHDR;

//ql_xu ---signature header
#define SIG_LEN			20
typedef struct {
	unsigned int sigLen;	//signature len
	unsigned char sigStr[SIG_LEN];	//signature content
	unsigned short chksum;	//chksum of imghdr and img
}SIGHDR;


unsigned short ipchksum(unsigned char *ptr, int count, unsigned short resid);

// Added by Mason Yu
#define SEM_REBOOT	1
#define SEM_UPGRADE	2
#define SEM_UPGRADE_2	3
#define SEM_DEFAULT   4

#ifdef ZTE_GENERAL_ROUTER_SC
int setupVtlsvr();
#endif

void portfw_modify( MIB_CE_PORT_FW_T *p, int del );
#if defined(TIME_ZONE) || defined(ZTE_GENERAL_ROUTER_SC)
int startNTP(void);
int stopNTP(void);
#endif
#if defined( ITF_GROUP) || defined(CONFIG_RTL867X_COMBO_PORTMAPPING)
void setupEth2pvc(void);
#endif
#if defined(IP_QOS) || defined(QOS_DIFFSERV)
int setupIPQoSRule(int enable);
int stopIPQ(void);
int setupIPQ(void);
#endif
/*ql:20081114 START: support GRED*/
#ifdef NEW_IP_QOS_SUPPORT
#define UNDOWAIT 0
#define DOWAIT 1
#define MAX_SPACE_LEGNTH 1024

#define DOCMDINIT \
		char cmdargvs[MAX_SPACE_LEGNTH]={0};\
		int argvs_index=1;\
		char *_argvs[32];

#define DOCMDARGVS(cmd,dowait,format,args...) \
		argvs_index=1;\
		memset(cmdargvs,0,sizeof(cmdargvs));\
		memset(_argvs,0,sizeof(_argvs));\
		snprintf(cmdargvs,sizeof(cmdargvs),format,##args);\
		fprintf(stderr,"%s %s\n",cmd,cmdargvs);\
		_argvs[argvs_index]=strtok(cmdargvs," ");\
		while(_argvs[argvs_index]){\
			_argvs[++argvs_index]=strtok(NULL," ");\
		}\
		do_cmd(cmd,_argvs,dowait);
#endif
/*ql:20081114 END*/

#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
int delPortForwarding( unsigned char ifindex );
int updatePortForwarding( unsigned char old_id, unsigned char new_id );
int delRoutingTable( unsigned char ifindex );
int updateRoutingTable( unsigned char old_id, unsigned char new_id );
unsigned int findMaxConDevInstNum(void);
unsigned int findConDevInstNumByPVC(unsigned char vpi, unsigned short vci);
unsigned int findMaxPPPConInstNum( unsigned int condev_inst );
unsigned int findMaxIPConInstNum( unsigned int condev_inst );
MIB_CE_ATM_VC_T *getATMVCByInstNum( unsigned int devnum, unsigned int ipnum, unsigned int pppnum, MIB_CE_ATM_VC_T *p, unsigned int *chainid );
int apply_PortForwarding( int action_type, int id, void *olddata );

#ifdef _CWMP_APPLY_
int apply_RIP( int action_type, int id, void *olddata );
int apply_Layer3Forwarding( int action_type, int id, void *olddata );
int apply_DefaultRoute( int action_type, int id, void *olddata );
//int apply_PortForwarding( int action_type, int id, void *olddata );
#if defined(TIME_ZONE) || defined(ZTE_GENERAL_ROUTER_SC)
int apply_NTP( int action_type, int id, void *olddata );
#endif //TIME_ZONE
int apply_RemoteAccess( int action_type, int id, void *olddata );
int apply_DHCP( int action_type, int id, void *olddata );
int apply_DNS( int action_type, int id, void *olddata );
int apply_WLAN( int action_type, int id, void *olddata );
int apply_ETHER( int action_type, int id, void *olddata );
int apply_LANIP( int action_type, int id, void *olddata );
int apply_MACFILTER( int action_type, int id, void *olddata );
int apply_DDNS( int action_type, int id, void *olddata );
#if defined( ITF_GROUP) || defined(CONFIG_RTL867X_COMBO_PORTMAPPING)
int apply_PortMapping( int action_type, int id, void *olddata );
#endif //ITF_GROUP
#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
int apply_IPQoSRule( int action_type, int id, void *olddata );
int apply_IPQoS( int action_type, int id, void *olddata );
#endif //IP_QOS
#ifdef CONFIG_IP_NF_ALG_ONOFF
int apply_ALGONOFF( int action_type, int id, void *olddata );
#endif
#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
int Apply_CaptivePortal(int action_type, int id, void *olddata);
#endif
#endif //_CWMP_APPLY_
#endif //_CWMP_MIB_

extern void Internet_status(void);
int getDisplayWanName(MIB_CE_ATM_VC_T *pEntry, char* name);

#ifdef CTC_WAN_NAME
int generateWanName(MIB_CE_ATM_VC_T *entry, char* wanname);
int setWanName(char* str, int applicationtype);
int getWanName(MIB_CE_ATM_VC_T *pEntry, char* name);
#endif
int getATMEntrybyVPIVCIUsrPswd(MIB_CE_ATM_VC_T* Entry, int vpi, int vci, char* username, char* password, char* ifname);

#ifdef QSETUP_WEB_REDIRECT
int start_qsetup_redirect(void);
int stop_qsetup_redirect(void);
#endif
void writeFlash(void);
int utilping(char *str);
extern void DnsBindPvcRoute(int mode);
int isAccessFromLocal(unsigned int ip);
#ifdef QOS_SPEED_LIMIT_SUPPORT
int mib_qos_speed_limit_existed(int speed,int prior);
#endif

#ifdef _PRMT_TR143_
struct TR143_UDPEchoConfig
{
	unsigned char	Enable;
	unsigned char	EchoPlusEnabled;
	unsigned short	UDPPort;
	unsigned char	Interface[32];
	unsigned char	SourceIPAddress[4];
};
extern const char gUDPEchoServerName[];
extern const char gUDPEchoServerPid[];
void UDPEchoConfigSave(struct TR143_UDPEchoConfig *p);
int UDPEchoConfigStart( struct TR143_UDPEchoConfig *p );
int UDPEchoConfigStop( struct TR143_UDPEchoConfig *p );
int apply_UDPEchoConfig( int action_type, int id, void *olddata );
#endif //_PRMT_TR143_

/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
enum eTStatus
{
	eTStatusDisabled,
	eTStatusUnsynchronized,
	eTStatusSynchronized,
	eTStatusErrorFailed,/*Error_FailedToSynchronize*/
	eTStatusError
};
#endif
/*ping_zhang:20081217 END*/
#endif

void pppoe_session_update(void *p);
void save_pppoe_sessionid(void *p);
// Added by Magician for external use
int deleteAllConnection(void);
int RegisterPVCnumber(void);
void cleanAllFirewallRule(void);
int restart_dnsrelay(void); //Jenny
int startWan(BOOT_TYPE_T mode);

// Mason Yu
#define WEB_REDIRECT_BY_MAC_INTERVAL	12*60*60	/* Polling DHCP release table every 12 hours */
#define TIMEOUT(fun, arg1, arg2, handle) 	timeout(fun,arg1,arg2, &handle)
#define UNTIMEOUT(fun, arg, handle)		untimeout(&handle)
void clearLandingPageRule(void *dummy);

// Mason Yu. Timer for auto search PVC
#if defined(AUTO_PVC_SEARCH_TR068_OAMPING) || defined(AUTO_PVC_SEARCH_PURE_OAMPING) || defined(AUTO_PVC_SEARCH_AUTOHUNT)
extern void ifAutoPVCisExist(unsigned int vpi, unsigned vci);
extern int setVC0Connection(unsigned int vpi, unsigned int vci);
void stopAutoSearchPVC(void *dummy);
extern int autoHunt_found;
extern int fin_AutoSearchPVC;
extern int sok;
#define INTERVAL_AUTOPVC 45
#endif
#ifdef MULTI_IC_SUPPORT
int getImgKey();
#endif

void poll_autoDMZ(void *dummy);
void restartWAN(void);

