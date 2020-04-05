/*
 *	option.h
*/

#ifndef INCLUDE_OPTIONS_H
#define INCLUDE_OPTIONS_H

#if defined(EMBED) || defined(__KERNEL__)
#include <linux/config.h>
#else
#include "../../../../uClibc/include/linux/autoconf.h"
#endif

#ifdef EMBED
#include <config/autoconf.h>
#else
#include "../../../../config/autoconf.h"
#endif

//---------------------------ping test tune
#define PINGCOUNT      4
#define DEFDATALEN     32
#define PINGINTERVAL   2       /* second */
#define MAXWAIT        10

//-------------------update/loadconfig prop
#define ENABLE_SIGNATURE_ADV
#define KEEP_CRITICAL_HW_SETTING	1
#define DSL_DOWN_DURING_LOCAL_UPLD	1
#define INCLUDE_DEFAULT_VALUE		1
#define APPLY_CHANGE			1
#define CLOSE_ITF_BEFORE_WRITE		1
#define CLOSE_OTHER_PROCESS		1

#undef KEEP_CRITICAL_CURRENT_SETTING
#undef CONFIG_USBCLIENT

//--------------------------------web prop
#define MINIMIZE_RAM_USAGE		1
#define WEB_UPGRADE			1

#ifdef MINIMIZE_RAM_USAGE
#define CONFIG_8M_SDRAM			1
#endif

#undef SUPPORT_AUTH_DIGEST
#undef WEB_REDIRECT_BY_MAC

#ifndef CONFIG_GUI_WEB
#define WEB_MENU_USE_NEW		1
#undef	WEB_MENU_USE_MTM
#endif

#ifdef WEB_UPGRADE
//#undef	UPGRADE_V1
#define	UPGRADE_V1			1
#undef	UPGRADE_V2
#define	UPGRADE_V3
#endif

#ifdef CONFIG_GUI_WEB
#define BOA_UNAUTH_FILE
#endif

//-------------------------------WIFI prop
#ifdef CONFIG_USER_WIRELESS_TOOLS
#define WLAN_SUPPORT			1
#define WLAN_WPA			1
#define WLAN_8185AG			1
#define WLAN_1x				1
#define WLAN_ACL			1
#define WLAN_QoS			1
#define WLAN_ONOFF_BUTTON_SUPPORT	1
#define WLAN_BUTTON_LED			1
#define ENABLE_WPAAES_WPA2TKIP		1
#define WLAN_IAPP			1

#undef MIB_WLAN_RESTART_SUPPORT
#undef MSG_WLAN_RESTART_SUPPORT
#undef CMD_WLAN_RESTART
#undef WLAN_CLIENT
#undef WIFI_TEST
#undef REVERSE_WPS_LED
#undef WLAN_TX_POWER_DISPLAY
#undef E8A_CHINA

#ifdef CONFIG_USER_WIRELESS_WDS
#define WLAN_WDS			1
#endif

#ifdef CONFIG_USER_WIRELESS_MBSSID
#define WLAN_MBSSID			1
#define MAX_WLAN_VAP			4
#endif
#endif //CONFIG_USER_WIRELESS_TOOLS

#ifdef  CONFIG_USER_AUTO_PROVISIONING
#define AUTO_PROVISIONING		1
#endif

//----------------------------------Tunnels setup
#undef CONFIG_PPTP_CLIENT
#undef CONFIG_L2TP_CLIENT
#undef CONFIG_IPSEC_CLIENT

//----------------------------------Network prop
#define NATIP_FORWARDING		1
#define MAC_FILTER			1
#define IP_PASSTHROUGH			1
#define IP_ACL				1
#define SECONDARY_IP			1
#define SPPPD_NO_IDLE			1
#define PPPOE_PASSTHROUGH		1

#undef CONFIG_SPPPD_STATICIP
#undef PPP_QUICK_CONNECT
#undef FORCE_IGMP_V2
#undef PORT_TRIGGERING
#undef URL_BLOCKING_SUPPORT
#undef URL_ALLOWING_SUPPORT
#undef DOMAIN_BLOCKING_SUPPORT
#undef ADDRESS_MAPPING
#undef MULTI_ADDRESS_MAPPING
#undef CONFIG_IGMP_FORBID

//uncomment for TCP/UDP connection limit
//#define TCP_UDP_CONN_LIMIT	1

//#define ZTE_GENERAL_ROUTER_SC 		1
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
#undef PORT_FORWARD_ADVANCE
//star: for set acl ip range
#undef ACL_IP_RANGE
//star: for set ip pool for different client type
#undef IP_BASED_CLIENT_TYPE
//star: for layer7 filter
#undef LAYER7_FILTER_SUPPORT
#endif

// Mason Yu
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC) || defined(CONFIG_GUI_WEB)
#undef AUTO_PVC_SEARCH_TR068_OAMPING
#undef AUTO_PVC_SEARCH_PURE_OAMPING
#undef AUTO_PVC_SEARCH_AUTOHUNT
#else
#undef AUTO_PVC_SEARCH_TR068_OAMPING
#define AUTO_PVC_SEARCH_PURE_OAMPING
#define AUTO_PVC_SEARCH_AUTOHUNT
#endif

#ifndef AUTO_PVC_SEARCH_TR068_OAMPING
#define AUTO_PVC_SEARCH_TABLE
#endif


#ifdef CONFIG_USER_VSNTP
#define TIME_ZONE			1
#endif

//--------------------------------QOS prop
#undef UPSTREAM_TRAFFIC_CTL
#undef QOS_DIFFSERV
#ifdef CONFIG_NET_SCHED
#ifdef CONFIG_USER_IPROUTE2_TC_TC
#define NEW_IP_QOS_SUPPORT		1
#if defined(NEW_IP_QOS_SUPPORT) || defined(QOS_DIFFSERV)
#define CONFIG_8021P_PRIO		1
#undef IP_QOS_VPORT
#undef IP_QOS
#undef QOS_SPEED_LIMIT_SUPPORT
#ifdef CONFIG_NETFILTER_XT_MATCH_DSCP
#define QOS_DSCP_MATCH			1
#define QOS_DSCP			1
#endif
#endif
#endif
#endif

#ifdef CONFIG_USER_IPROUTE2_IP_IP
#ifdef IP_QOS
#define IP_POLICY_ROUTING		1
#endif
#endif


#define ITF_GROUP			1
#define DIAGNOSTIC_TEST			1
#undef VLAN_GROUP
#undef ELAN_LINK_MODE
#undef ELAN_LINK_MODE_INTRENAL_PHY

//xl_yue
#ifdef ZTE_GENERAL_ROUTER_SC
#define	DOS_SUPPORT			1
#else
#undef	DOS_SUPPORT
#endif

#ifdef CONFIG_GUI_WEB
#define DEFAULT_GATEWAY_V2		1
#define QSETUP_WEB_REDIRECT		1
#else
#define DEFAULT_GATEWAY_V1	//set dgw per pvc
#undef DEFAULT_GATEWAY_V2	// set dgw interface in routing page
#undef QSETUP_WEB_REDIRECT
#endif	//CONFIG_GUI_WEB

#ifndef DEFAULT_GATEWAY_V2
#ifndef DEFAULT_GATEWAY_V1
#define DEFAULT_GATEWAY_V1		1
#endif
#endif
#ifdef DEFAULT_GATEWAY_V2
#define AUTO_PPPOE_ROUTE		1
#endif

#undef XOR_ENCRYPT
#undef XML_TR069
#define TELNET_IDLE_TIME	300 //10*30 sec. Please compiler boa and telnetd
#define TELNET_CLI

#ifdef CONFIG_USER_CWMP_TR069
#ifndef XML_TR069
#define XML_TR069
#endif  //XML_TR069
#define _CWMP_MIB_				1
#ifdef CONFIG_USER_CWMP_WITH_SSL
#define _CWMP_WITH_SSL_				1
#endif //CONFIG_USER_CWMP_WITH_SSL
#define	_CWMP_APPLY_				1
#define _PRMT_SERVICES_				1
#define _PRMT_CAPABILITIES_			1
#define _PRMT_DEVICECONFIG_			1
//#define _PRMT_USERINTERFACE_			1
/*disable connection request authentication*/
//#define _TR069_CONREQ_AUTH_SELECT_		1
#ifdef CONFIG_USER_TR143
#define _PRMT_TR143_				1
#endif //CONFIG_USER_TR143
#ifdef CONFIG_USB_ETH
#define _PRMT_USB_ETH_				1
#endif //CONFIG_USB_ETH


/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#define _PRMT_WT107_				1
#ifdef _PRMT_WT107_
#ifdef CONFIG_USER_BUSYBOX_TRACEROUTE
#define _SUPPORT_TRACEROUTE_PROFILE_		1
#endif //CONFIG_USER_BUSYBOX_TRACEROUTE
#define _SUPPORT_CAPTIVEPORTAL_PROFILE_		1
#define _SUPPORT_ADSL2DSLDIAG_PROFILE_		1
#define _SUPPORT_ADSL2WAN_PROFILE_		1
#endif //_PRMT_WT107_


#undef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
#define SNTP_MULTI_SERVER			1

//#define _PRMT_X_CT_EXT_ENABLE_			1
#ifdef _PRMT_X_CT_EXT_ENABLE_
	/*TW's ACS has some problem with this extension field*/
	#define _INFORM_EXT_FOR_X_CT_		1
    #ifdef _PRMT_SERVICES_
	#define _PRMT_X_CT_COM_IPTV_		1
	#define _PRMT_X_CT_COM_MWBAND_		1
	//#define _PRMT_X_CT_COM_MONITOR_		1
	//#define _PRMT_X_CT_COM_VPDN_		1
    #endif //_PRMT_SERVICES_
	#define	_PRMT_X_CT_COM_DDNS_		1
	#define _PRMT_X_CT_COM_ALG_		1
	#define _PRMT_X_CT_COM_ACCOUNT_		1
	#define _PRMT_X_CT_COM_RECON_		1
	#define _PRMT_X_CT_COM_PORTALMNT_	1
	#define _PRMT_X_CT_COM_SRVMNG_		1	/*ServiceManage*/
	//#define _PRMT_X_CT_COM_PPPOE_		1
	#define _PRMT_X_CT_COM_PPPOEv2_		1
    #ifdef WLAN_SUPPORT
	#define _PRMT_X_CT_COM_WLAN_		1
    #endif //WLAN_SUPPORT
	#define _PRMT_X_CT_COM_DHCP_		1
	#define _PRMT_X_CT_COM_WANEXT_		1
#endif //_PRMT_X_CT_EXT_ENABLE_
#endif //CONFIG_USER_CWMP_TR069


//Jimluo
#if	defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
#define DEVICE_TYPE_5
#endif


//Jimluo
#if	defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
#define CTC_WAN_NAME
#else
#undef CTC_WAN_NAME
#endif

//ql add
#undef	RESERVE_KEY_SETTING

#define SYSLOG_SUPPORT			1
#define WEB_ENABLE_PPP_DEBUG		0

#undef WEB_DEBUG_MSG
#undef CRASHDEBUG
#undef DEBUGNONINET

#ifdef SYSLOG_SUPPORT
#undef SYSLOG_REMOTE_LOG
#endif

// Mason Yu
#undef SEND_LOG

//xl_yue add,when finishing maintenance,inform ITMS to change password
#undef FINISH_MAINTENANCE_SUPPORT

//xl_yue add,web logining is maintenanced by web server
#undef	USE_LOGINWEB_OF_SERVER

#ifdef USE_LOGINWEB_OF_SERVER
//xl_yue add,if have logined error for three times continuely,please relogin after 1 minute
#define LOGIN_ERR_TIMES_LIMITED 5
//xl_yue add,only one user can login with the same account at the same time
#undef ONE_USER_LIMITED		
//xl_yue add,if no action lasting for 5 minutes,auto logout
#define AUTO_LOGOUT_SUPPORT	1
#undef USE_BASE64_MD5_PASSWD
#endif

/*######################*/
//4 jim_luo Bridge Mode only access on web
#undef  BRIDGE_ONLY_ON_WEB

//4 E8-A unsupport save and restore configuration file, then should remark belwo macro CONFIG_SAVE_RESTORE
#define CONFIG_SAVE_RESTORE

//E8-A unsupport web upgrade image, we should enable #undef WEB_UPGRADE at line 52
/*########################*/

//jiunming, for e8b telecomacount enable
//#define CTC_TELECOM_ACCOUNT		1

//add by ramen
#undef DNS_BIND_PVC_SUPPORT

#define  DHCPS_POOL_COMPLETE_IP		1
#define  DHCPS_DNS_OPTIONS		1
/* Modify default config for Telefonica */
#undef  TELEFONICA_DEFAULT_CFG
#undef  ZHONE_DEFAULT_CFG
#undef ACCOUNT_CONFIG
#undef MULTI_USER_PRIV
#ifdef ZHONE_DEFAULT_CFG
#undef MULTI_USER_PRIV
#endif
#ifdef MULTI_USER_PRIV
#define ACCOUNT_CONFIG
#endif

/*ql:20080729 START: different image header for different IC type*/
#undef MULTI_IC_SUPPORT
/*ql:20080729 END*/

/*xl_yue:20090210 add cli cmdedit*/
#ifdef CONFIG_USER_CMD_CLI
#define CONFIG_CLI_CMD_EDIT
#define CONFIG_CLI_TAB_FEATURE
#endif

#endif  // INCLUDE_OPTIONS_H

#undef AUTO_DETECT_DMZ

#ifdef CONFIG_USER_PARENTAL_CONTROL
#define PARENTAL_CTRL
#endif

#undef CONFIG_11N_SAGEM_WEB

