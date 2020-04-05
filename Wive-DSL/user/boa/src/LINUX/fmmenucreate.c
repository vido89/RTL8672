/*
*  fmmenucreate.c is used to create menu
*  added bu xl_yue
*/
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "../webs.h"
#include "mib.h"
#include "webform.h"
#include "utility.h"
#include "vendor.h"
//add by ramen to include the autoconf.h created by kernel
#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../../uClibc/include/linux/autoconf.h"
#endif
#ifdef WEB_MENU_USE_NEW
#ifdef CONFIG_DEFAULT_WEB	// default pages  
const char menuTitle[] = "Site contents:";
#elif defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC) ||defined(ZTE_GENERAL_ROUTER_EN)
const char menuTitle[] = "ZXDSL531B";
#endif
#ifdef CONFIG_DEFAULT_WEB	// default pages
/*
 *	2nd Layer Menu
 */
struct RootMenu childmenu_dns[] = {
  {"DNS Server", MENU_URL, "dns.asp", "DNS Server Configuration", 0, 0},
#ifdef CONFIG_USER_DDNS
  {"Dynamic DNS", MENU_URL, "ddns.asp", "DDNS Configuration", 0, 0},
#endif
  {0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_fw[] = {
#ifdef CONFIG_IP_NF_ALG_ONOFF
  {"ALG", MENU_URL, "algonoff.asp", "ALG on-off", 0, 0},
#endif
  {"IP/Port Filtering", MENU_URL, "fw-ipportfilter.asp",
   "Setup IP/Port filering", 0, 0},
#ifdef MAC_FILTER
  {"MAC Filtering", MENU_URL, "fw-macfilter.asp", "Setup MAC filering", 0, 0},
#endif
  {"Port Forwarding", MENU_URL, "fw-portfw.asp", "Setup port-forwarding", 0,
   0},
#ifdef URL_BLOCKING_SUPPORT
  {"URL Blocking", MENU_URL, "url_blocking.asp", "URL Blocking Setting", 0,
   0},
#endif
#ifdef DOMAIN_BLOCKING_SUPPORT
  {"Domain Blocking", MENU_URL, "domainblk.asp", "Domain Blocking Setting", 0,
   0},
#endif
#ifdef PARENTAL_CTRL
  {"Parental Control", MENU_URL, "parental-ctrl.asp", "Parental Control Setting", 0,
   0},
#endif
#ifdef TCP_UDP_CONN_LIMIT
  {"Connection Limit", MENU_URL, "connlimit.asp", "Connection Limit Setting", 0,
   0},
#endif // TCP_UDP_CONN_LIMIT
#ifdef NATIP_FORWARDING
  {"NAT IP Forwarding", MENU_URL, "fw-ipfw.asp", "Setup NAT IP Mapping", 0,
   0},
#endif
#ifdef PORT_TRIGGERING
  {"Port Triggering", MENU_URL, "gaming.asp", "Setup Port Triggering", 0, 0},
#endif
  {"DMZ", MENU_URL, "fw-dmz.asp", "Setup DMZ",0, 0},  	
#ifdef ADDRESS_MAPPING
#ifdef MULTI_ADDRESS_MAPPING
 // Eric Chen add for True
  {"NAT Rule Configuration", MENU_URL, "multi_addr_mapping.asp", "Setup NAT Rule",0, 0},
#else //!MULTI_ADDRESS_MAPPING
 // Mason Yu on True
  {"NAT Rule Configuration", MENU_URL, "address_mapping.asp", "Setup NAT Rule",0, 0},
  #endif// end of !MULTI_ADDRESS_MAPPING
#endif
  {0, 0, 0, 0, 0, 0}

};

/*
 *	First Layer Menu
 */
struct RootMenu childmenu_lan[] = {
  {0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_wlan[] = {
  {"Basic Settings", MENU_URL, "wlbasic.asp",   "Setup wireless basic configuration", 0, 0},
  {"Advanced Settings", MENU_URL, "wladvanced.asp",   "Setup wireless advanced configuration", 0, 0},
#ifdef WLAN_MBSSID
  {"Security", MENU_URL, "wlwpa_mbssid.asp", "Setup wireless security", 0, 0},
#else
  {"Security", MENU_URL, "wlwpa.asp", "Setup wireless security", 0, 0},
#endif
#ifdef WLAN_ACL
  {"Access Control", MENU_URL, "wlactrl.asp",   "Setup access control list for wireless clients", 0, 0},
#endif
#ifdef WLAN_WDS
  {"WDS", MENU_URL, "wlwds.asp", "WDS Settings", 0, 0},
#endif
#ifdef WLAN_CLIENT
  {"Site Survey", MENU_URL, "wlsurvey.asp", "Wireless Site Survey", 0, 0},
#endif
#ifdef CONFIG_WIFI_SIMPLE_CONFIG	// WPS
  {"WPS", MENU_URL, "wlwps.asp", "Wireless Protected Setup", 0, 0},
#endif
#ifdef WLAN_MBSSID
  {"MBSSID", MENU_URL, "wlmbssid.asp", "Wireless Multiple BSSID Setting", 0,   0},
#endif
  {0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_wan[] = {
  {"Channel Config", MENU_URL, "wanadsl.asp", "Setup Channel Mode", 0, 0},
  {"ATM Settings", MENU_URL, "wanatm.asp", "Setup ATM", 0, 0},
  {"ADSL Settings", MENU_URL, "/adv/adsl-set.asp", "Setup ADSL", 0, 0},
#ifdef CONFIG_PPTP_CLIENT
  {"PPTP Settings", MENU_URL, "/adv/pptp-set.asp", "Setup ADSL", 0, 0},
#endif
#ifdef CONFIG_L2TP_CLIENT
  {"PPTP Settings", MENU_URL, "/adv/l2tp-set.asp", "Setup ADSL", 0, 0},
#endif
#ifdef CONFIG_IPSEC_CLIENT
  {"PPTP Settings", MENU_URL, "/adv/ipsec-set.asp", "Setup ADSL", 0, 0},
#endif
  {0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_service[] = {
  //{"DHCP Mode", MENU_URL, "dhcpmode.asp", "DHCP Mode Configuration", 0, 0},
  {"DHCP Settings", MENU_URL, "dhcpd.asp", "DHCP Configuration", 0, 0},
  //{"DHCP Relay", MENU_URL, "dhcrelay.asp", "DHCP Relay Configuration", 0, 0},
  {"DNS", MENU_FOLDER, &childmenu_dns, "",
   sizeof (childmenu_dns) / sizeof (struct RootMenu) - 1, 0},
  {"Firewall", MENU_FOLDER, &childmenu_fw, "",
   sizeof (childmenu_fw) / sizeof (struct RootMenu) - 1, 0},
#ifdef CONFIG_USER_IGMPPROXY
  {"IGMP Proxy", MENU_URL, "igmproxy.asp", "IGMP Proxy Configuration", 0, 0},
#endif
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
  {"UPnP", MENU_URL, "upnp.asp", "UPnP Configuration", 0, 0},
#endif
#ifdef CONFIG_USER_ROUTED_ROUTED
  {"RIP", MENU_URL, "rip.asp", "RIP Configuration", 0, 0},
#endif
#ifdef WEB_REDIRECT_BY_MAC
  {"Landing Page", MENU_URL, "landing.asp", "Landing Page Configuration", 0, 0},
#endif
  {0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_adv[] = {
  {"ARP table", MENU_URL, "arptable.asp", "ARP table", 0, 0},
  {"Bridging", MENU_URL, "bridging.asp", "Bridge Configuration", 0, 0},
  {"Routing", MENU_URL, "routing.asp", "Routing Configuration", 0, 0},
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
  {"SNMP", MENU_URL, "snmp.asp", "SNMP Protocol Configuration", 0, 0},
#endif
#if (defined( CONFIG_EXT_SWITCH) && defined(ITF_GROUP)) || defined(CONFIG_RTL867X_COMBO_PORTMAPPING)
#ifdef BPORTMAPPING_SUPPORT
  {"Port Mapping", MENU_URL, "eth2pvc.asp", "Assign Ethernet to PVC", 0, 0},
#endif
#endif
#ifdef NEW_IP_QOS_SUPPORT
  {"QoS Classification", MENU_URL, "ipqos_sc.asp", "Setup IP QoS", 0, 0},
#endif
#ifdef VLAN_GROUP
#ifdef BPORTMAPPING_SUPPORT
  {"Port Mapping", MENU_URL, "eth2pvc_vlan.asp", "Port-vlan mapping", 0, 0},
#endif
#endif
#ifdef IP_QOS
  {"IP QoS", MENU_URL, "ipqos.asp", "IP QoS Configuration", 0, 0},
#endif
#ifdef QOS_DIFFSERV
  {"DiffServ", MENU_URL, "diffserv.asp", "Differentiated Services Setting", 0, 0},
#endif
#ifdef CONFIG_EXT_SWITCH
#ifdef ELAN_LINK_MODE
  {"Link Mode", MENU_URL, "lnkmode.asp", "Ethernet Link Mode Setting", 0, 0},
#endif
#else 
#ifdef ELAN_LINK_MODE_INTRENAL_PHY
    {"Link Mode", MENU_URL, "eth.asp", "Ethernet Link Mode Setting", 0, 0},
#endif
#endif
  {"Remote Access", MENU_URL, "rmtacc.asp", "Services Access Control", 0, 0},
  {"Others", MENU_URL, "others.asp", "Other advanced Configuration", 0, 0},
  {0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_diag[] = {
  {"Ping", MENU_URL, "ping.asp", "Ping test", 0, 0},
  {"ATM Loopback", MENU_URL, "oamloopback.asp", "ATM OAM loopback test", 0,
   0},
  {"ADSL", MENU_URL, "/adv/adsl-diag.asp", "ADSL Tone Diagnostics", 0, 0},
#ifdef DIAGNOSTIC_TEST
  {"Diagnostic Test", MENU_URL, "diag-test.asp", "Diagnostic test", 0, 0},
#endif
  {0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_admin[] = {
#ifdef FINISH_MAINTENANCE_SUPPORT
  {"Finish Maintenance", MENU_URL, "finishmaintenance.asp",
   "Finish Maintenance", 0, 0},
#endif
  {"Commit/Reboot", MENU_URL, "reboot.asp", "Commit/reboot the system", 0, 0},
#ifdef CONFIG_SAVE_RESTORE
  {"Backup/Restore", MENU_URL, "saveconf.asp",
   "Backup/restore current settings", 0, 0},
#endif
#ifdef ACCOUNT_LOGIN_CONTROL
  {"Logout", MENU_URL, "adminlogout.asp", "Logout", 0, 0},
#endif
#ifdef SYSLOG_SUPPORT
#ifndef SEND_LOG
  {"System Log", MENU_URL, "syslog.asp", "Show system log", 0, 0},
#else
  {"System Log", MENU_URL, "syslog_server.asp", "Show system log", 0, 0},
#endif
#endif
#ifdef DOS_SUPPORT
  {"DOS", MENU_URL, "dos.asp", "Denial of service", 0, 0},
#endif
#ifdef ACCOUNT_CONFIG
  {"User Account", MENU_URL, "userconfig.asp", "Setup user account", 0, 0},	// Jenny, user account config page
#else
  {"Password", MENU_URL, "password.asp", "Setup access password", 0, 0},
#endif
#ifdef WEB_UPGRADE
#ifdef UPGRADE_V1
  {"Upgrade Firmware", MENU_URL, "upload.asp", "Update firmware image", 0, 0},
#else
#ifdef UPGRADE_V2
  {"Upgrade Firmware", MENU_URL, "upload2.asp", "Update firmware image", 0,
   0},
#endif // of UPGRADE_V2
#ifdef UPGRADE_V3
  {"Upgrade Firmware", MENU_URL, "upload3.asp", "Update firmware image", 0,
   0},
#endif // of UPGRADE_V3
#endif // of UPGRADE_V1
#endif // of WEB_UPGRADE
#ifdef IP_ACL
  {"ACL Config", MENU_URL, "acl.asp", "ACL Setting", 0, 0},
#endif
#ifdef TIME_ZONE
  {"Time Zone", MENU_URL, "ntp.asp", "Setup Time Zone", 0, 0},
#endif
#ifdef AUTO_PROVISIONING
  {"Auto-Provisionning", MENU_URL, "autoprovision.asp",
   "Auto-Provisioning Configuration", 0, 0},
#endif
#ifdef _CWMP_MIB_
  {"TR-069 Config", MENU_URL, "tr069config.asp", "TR-069 Configuration", 0,
   0},
#endif
//added by xl_yue
#ifdef USE_LOGINWEB_OF_SERVER
  {"Logout", MENU_URL, "/admin/logout.asp", "Logout", 0, 0},
#endif
  {0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_statis[] = {
  {"Interfaces", MENU_URL, "stats.asp", "Display packet statistics", 0, 0},
  {"ADSL", MENU_URL, "/adv/adsl-statis.asp", "Display ADSL statistics", 0, 0},
  {0, 0, 0, 0, 0, 0}
};

//added by eric for VOIP web pages  20070904
#ifdef VOIP_SUPPORT
struct RootMenu childmenu_VoIP[] = {
  {"Port1", MENU_URL, "voip_general.asp?port=0", "Setup VoIP Port 1", 0, 0},
  {"Port2", MENU_URL, "voip_general.asp?port=1", "Setup VoIP Port 2", 0, 0},
  {"FXO", MENU_URL, "voip_general.asp?port=2", "Configure FXO connection", 0, 0},  
  {"Tone", MENU_URL, "voip_tone.asp", "Configure SLIC tones", 0, 0},
  {"Ring", MENU_URL, "voip_ring.asp", "Configure Ring Types of Phone", 0, 0},

  {"Other", MENU_URL, "voip_other.asp", "Other Services", 0, 0},  
  {"Config", MENU_URL, "voip_config.asp", "Configuration Upload/Download", 0, 0},  
  {0, 0, 0, 0, 0, 0}
};

#endif
/*
 *	Root Menu
 */
struct RootMenu rootmenu[] = {
  {"Status", MENU_URL, "status.asp", "View the Status", 0, 0},
  {"LAN", MENU_URL, "tcpiplan.asp", "Setup LAN Interface", 0, 0},
#ifdef WLAN_SUPPORT
  {"Wireless", MENU_FOLDER, &childmenu_wlan, "",
   sizeof (childmenu_wlan) / sizeof (struct RootMenu) - 1, 0},
#endif
  {"WAN", MENU_FOLDER, &childmenu_wan, "",
   sizeof (childmenu_wan) / sizeof (struct RootMenu) - 1, 0},
  {"Services", MENU_FOLDER, &childmenu_service, "",
   sizeof (childmenu_service) / sizeof (struct RootMenu) - 1, 0},
#ifdef VOIP_SUPPORT
  {"VoIP", MENU_FOLDER, &childmenu_VoIP, "",
   sizeof (childmenu_VoIP) / sizeof (struct RootMenu) - 1, 0},
#endif
  {"Advance", MENU_FOLDER, &childmenu_adv, "",
   sizeof (childmenu_adv) / sizeof (struct RootMenu) - 1, 0},
  {"Diagnostic", MENU_FOLDER, &childmenu_diag, "",
   sizeof (childmenu_diag) / sizeof (struct RootMenu) - 1, 0},
  {"Admin", MENU_FOLDER, &childmenu_admin, "",
   sizeof (childmenu_admin) / sizeof (struct RootMenu) - 1, 0},
  {"Statistics", MENU_FOLDER, &childmenu_statis, "",
   sizeof (childmenu_statis) / sizeof (struct RootMenu) - 1, 0},
  {0, 0, 0, 0, 0, 0}
};
#endif // of CONFIG_DEFAULT_WEB
#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
struct RootMenu childmenu_statistic[] = {
  {"�ӿ�ͳ��", MENU_URL, "stats_sc.asp", "Display current flow status", 0, 0},
  {"DSLͳ��", MENU_URL, "/adv/adsl-statis_sc.asp",
   "Display current adsl status", 0, 0},
  {0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_system[] = {
  {"ϵͳ��Ϣ", MENU_URL, "/admin/status_sc.asp",
   "Display current base status", 0, 0},
  {"��������", MENU_URL, "lan_status_sc.asp", "Display current lan status", 0,
   0},
#ifdef WLAN_SUPPORT
  {"��������", MENU_URL, "wlan_status_sc.asp", "Display current wlan status",
   0, 0},
#endif
  {"��������", MENU_URL, "wan_status_sc.asp", "Display current wan status", 0,
   0},
#ifdef CONFIG_EXT_SWITCH
#ifdef ITF_GROUP
  {"�˿ڰ�", MENU_URL, "status_eth2pvc_sc.asp",
   "Display current portband status", 0, 0},
#endif
#endif
#ifdef ZTE_GENERAL_ROUTER_SC
  {"·��״̬", MENU_URL, "routeinfo.asp", "Display current route info", 0, 0},
#endif
  {"����ͳ��", MENU_FOLDER, &childmenu_statistic, "",
   sizeof (childmenu_statistic) / sizeof (struct RootMenu) - 1, 0},
  {"ARP�б�", MENU_URL, "arptable_sc.asp", "Display current arp table status",
   0, 0},
  {0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_quickset[] = {
  {"�����趨", MENU_URL, "fc-page1-ch.asp", "Quick setting", 0, 0},
  {0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_lan[] = {
  {"LAN�ӿ�", MENU_URL, "tcpiplan_sc.asp", "LAN�ӿ�", 0, 0},
  {"DHCP����", MENU_URL, "dhcpd_sc.asp", "DHCP����", 0, 0},
  {0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_wan[] = {
  {"�ӿ�����", MENU_URL, "wanadsl_sc.asp", "�ӿ�����", 0, 0},
  {"ADSL����", MENU_URL, "/adv/adsl-set_sc.asp", "ADSL����", 0, 0},
  {0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_wlan[] = {
  {"��������", MENU_URL, "/admin/wlbasic_sc.asp",
   "Setup wireless basic configuration", 0, 0},
#ifdef WLAN_MBSSID
  {"��ȫ����", MENU_URL, "/admin/wlwpa_mbssid_sc.asp",
   "Setup wireless security", 0, 0},
#else
  {"��ȫ����", MENU_URL, "/admin/wlwpa_sc.asp", "Setup wireless security", 0,
   0},
#endif
  {"�߼�����", MENU_URL, "/admin/wladvanced_sc.asp",
   "Setup wireless advanced configuration", 0, 0},
#ifdef WLAN_ACL
  {"�������", MENU_URL, "/admin/wlactrl_sc.asp",
   "Setup access control list for wireless clients", 0, 0},
#endif
#ifdef ZTE_GENERAL_ROUTER_SC
#ifdef WLAN_WDS
  {"WDS����", MENU_URL, "/admin/wlwds_sc.asp", "WDS Settings", 0, 0},
#endif
#endif
#ifdef CONFIG_WIFI_SIMPLE_CONFIG
  {"WPS����", MENU_URL, "/admin/wlwps_sc.asp", "WPS Settings", 0, 0},
#endif
  {0, 0, 0, 0, 0, 0}
};
#ifdef ZTE_GENERAL_ROUTER_SC
	struct RootMenu childmenu_firewall[]={
		{"IP/Port ����", MENU_URL,"fw-ipportfilter_sc.asp", "no tip",0,0},
		{"MAC ����", MENU_URL,"fw-macfilter_sc.asp", "no tip",0,0},
#ifdef URL_ALLOWING_SUPPORT
		{"URL �趨", MENU_URL,"url_sc.asp", "URL  ����",0,0},
#elif  defined(URL_BLOCKING_SUPPORT)&&(!defined(URL_ALLOWING_SUPPORT)) 
              {"URL ����", MENU_URL,"url_blocking_sc.asp", "URL  Blocking ����",0,0},
#endif
#ifdef LAYER7_FILTER_SUPPORT
		{"Ӧ�������ֹ", MENU_URL,"fw-layer7filter_sc.asp", "no tip",0,0},		
#endif
		{0,0,0,0,0,0}

	};
#endif
#ifdef ZTE_GENERAL_ROUTER_SC
struct RootMenu childmenu_vmserver[] = {
  {"��������", MENU_URL, "/admin/vrtsrv_sc.asp", "��������", 0, 0},
  {"DMZ����", MENU_URL, "/admin/fw-dmz_sc.asp", "DMZ����", 0, 0},
  {0, 0, 0, 0, 0, 0}
};
#endif
#ifdef ZTE_GENERAL_ROUTER_SC
	struct RootMenu childmenu_routersetting[]={
#ifndef CONFIG_USER_ZEBRA_OSPFD_OSPFD
		{"��̬·��", MENU_URL,"rip_sc.asp", "��̬·������",0,0},
#else
		{"��̬·��", MENU_URL,"ospf.asp", "��̬·������",0,0},
#endif
		{"��̬·��", MENU_URL,"routing_sc.asp", "��̬·������",0,0},
		{0,0,0,0,0,0}

	};
#endif
#ifdef ZTE_GENERAL_ROUTER_SC
struct RootMenu childmenu_advothersetting[] = {
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
  {"UPNP����", MENU_URL, "/admin/upnp_sc.asp", "UPnP configuration", 0, 0},
#endif
#ifdef CONFIG_USER_IGMPPROXY
  {"IGMP����", MENU_URL, "igmproxy_sc.asp", "IGMP ��������", 0, 0},
#endif
  {"�Ų�������", MENU_URL, "/admin/bridging_sc.asp",   "configure bridge parameters", 0, 0},
  {"IP PassThrough", MENU_URL, "others_sc.asp", "IP PassThrough ����", 0, 0},
#ifdef DOS_SUPPORT
  {"DoS����", MENU_URL, "dos.asp", "Denial of service", 0, 0},
#endif
  {0, 0, 0, 0, 0, 0}
};
#endif
struct RootMenu childmenu_adv[] = {
#ifdef ZTE_GENERAL_ROUTER_SC
#ifdef CONFIG_EXT_SWITCH
#ifdef ITF_GROUP
  {"�˿ڰ�", MENU_URL, "/admin/eth2pvc_sc.asp", "Assign Ethernet to PVC", 0,   0},
#endif
#endif 
   {"QOS����", MENU_URL, "ipqos_sc.asp", "Setup QoS", 0, 0},
  {"DNS����", MENU_URL, "dns_sc.asp", "Setup how to obtain DNS servers", 0,0},
  {"���������", MENU_FOLDER, &childmenu_vmserver, "",  sizeof (childmenu_vmserver) / sizeof (struct RootMenu) - 1, 0},
   {"·������", MENU_FOLDER, &childmenu_routersetting, "", sizeof (childmenu_routersetting) / sizeof (struct RootMenu) - 1, 0},
  {"��������", MENU_FOLDER, &childmenu_firewall, "",  sizeof (childmenu_firewall) / sizeof (struct RootMenu) - 1, 0},  

  {"��������", MENU_FOLDER, &childmenu_advothersetting, "", sizeof (childmenu_advothersetting) / sizeof (struct RootMenu) - 1, 0},
#endif

  {0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_admin[] = {
  {"�û�����", MENU_URL, "/admin/password_sc.asp",   "Setup access password", 0, 0},
  #if !defined( ZTE_531B_BRIDGE_SC) && ( defined(ZTE_GENERAL_ROUTER_SC) || defined(TIME_ZONE) )
  {"ʱ������", MENU_URL, "/admin/date_sc.asp", "Setup Time Zone", 0, 0},
#endif
#ifdef SYSLOG_SUPPORT
  {"ϵͳ��־", MENU_URL, "/admin/syslog_sc.asp", "Show system log", 0, 0},
#endif
#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
  {"ˢ������", MENU_URL, "refresh.asp", "Refresh Setting", 0, 0},
#endif
#ifdef ZTE_GENERAL_ROUTER_SC
  {"SNMP����", MENU_URL, "snmp_sc.asp", "Snmp Setting", 0, 0},
#endif
#if defined(ZTE_GENERAL_ROUTER_SC)&&defined(_CWMP_MIB_)
  {"TR069����", MENU_URL, "tr069config_sc.asp", "Tr069 Setting", 0, 0},
#endif
#ifdef IP_ACL
#ifndef MAC_ACL
  {"ACL ����", MENU_URL, "acl_sc.asp", "ACL Setting", 0, 0},
#else
  {"ACL ����", MENU_URL, "acl_adv.asp", "ACL Setting", 0, 0},
#endif
#endif
#ifdef ZTE_GENERAL_ROUTER_SC
  {"Զ�̷��ʿ���", MENU_URL, "rmtacc_sc.asp", "Services Access Control", 0,
   0},
#endif
#ifdef CONFIG_SAVE_RESTORE
  {"���ñ���/�ָ�", MENU_URL, "/admin/saveconf_sc.asp",   "Backup/restore current settings", 0, 0},
#endif
#ifdef WEB_UPGRADE
#ifdef UPGRADE_V1
  {"�����汾", MENU_URL, "/admin/upload_sc.asp", "Update firmware image", 0,
   0},
#endif
#endif
{"����/����", MENU_URL, "/admin/reboot_sc.asp", "Commit/reboot the system",
   0, 0},

#ifdef NAT_CONN_LIMIT
	{"����������", MENU_URL, "connlimit.asp", "Connection Limit", 0, 0},
#endif
#ifdef CONFIG_IP_NF_ALG_ONOFF
  {"ALG����", MENU_URL, "algonoff_sc.asp", "ALG on-off", 0,
   0},
 #endif
 {0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_diag[] = {
  {"Ping", MENU_URL, "ping-ch.asp", "Ping test", 0, 0}, 
  {"ADSL�ز�", MENU_URL, "/adv/adsl-diag-ch.asp", "ADSL Tone Diagnostics", 0, 0},
  {"ATM ����", MENU_URL, "oamloopback-ch.asp", "ATM OAM loopback test", 0,   0},
#ifdef DIAGNOSTIC_TEST
  {"���Ӳ���", MENU_URL, "diag-test-ch.asp", "Diagnostic test", 0, 0},
#endif
		{0,0,0,0,0,0}
	};
struct RootMenu rootmenu[] = {
  {"ϵͳ״̬", MENU_FOLDER, &childmenu_system, "",
   sizeof (childmenu_system) / sizeof (struct RootMenu) - 1, 0},
  {"��������", MENU_FOLDER, &childmenu_quickset, "",
   sizeof (childmenu_quickset) / sizeof (struct RootMenu) - 1, 0},
  {"��������", MENU_FOLDER, &childmenu_lan, "",
   sizeof (childmenu_lan) / sizeof (struct RootMenu) - 1, 0},
#ifdef WLAN_SUPPORT
  {"��������", MENU_FOLDER, &childmenu_wlan, "",
   sizeof (childmenu_wlan) / sizeof (struct RootMenu) - 1, 0},
#endif
  {"��������", MENU_FOLDER, &childmenu_wan, "",
   sizeof (childmenu_wan) / sizeof (struct RootMenu) - 1, 0},
  {"�߼��趨", MENU_FOLDER, &childmenu_adv, "",
   sizeof (childmenu_adv) / sizeof (struct RootMenu) - 1, 0},
  {"ϵͳ����", MENU_FOLDER, &childmenu_admin, "",
   sizeof (childmenu_admin) / sizeof (struct RootMenu) - 1, 0},
  {"ϵͳ���", MENU_FOLDER, &childmenu_diag, "",
   sizeof (childmenu_diag) / sizeof (struct RootMenu) - 1, 0},
  {0, 0, 0, 0, 0, 0}
};
#endif // of defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
//add by ramen for 531b router en version
#ifdef ZTE_GENERAL_ROUTER_EN
struct RootMenu childmenu_statistic[] = {
  {"Interface Statistic", MENU_URL, "stats_en.asp", "Display current flow status", 0, 0},
  {"DSL Statistic", MENU_URL, "/adv/adsl-statis_en.asp",   "Display current adsl status", 0, 0},
  {0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_system[] = {
  {"System", MENU_URL, "/admin/status_en.asp",  "Display current base status", 0, 0},
  {"LAN Network", MENU_URL, "lan_status_en.asp", "Display current lan status", 0,
   0},
#ifdef WLAN_SUPPORT
  {"Wireless Network", MENU_URL, "wlan_status_en.asp", "Display current wlan status",
   0, 0},
#endif
  {"WAN Network", MENU_URL, "wan_status_en.asp", "Display current wan status", 0,
   0},
#ifdef CONFIG_EXT_SWITCH
#ifdef ITF_GROUP
#ifdef BPORTMAPPING_SUPPORT
  {"Port Mapping", MENU_URL, "status_eth2pvc_en.asp", "Display current portband status", 0, 0},
#endif
#endif
#endif
  {"Routing Info", MENU_URL, "routeinfo_en.asp", "Display current route info", 0, 0},
  {"Network Statistic", MENU_FOLDER, &childmenu_statistic, "",   sizeof (childmenu_statistic) / sizeof (struct RootMenu) - 1, 0},
  {"ARP Info", MENU_URL, "arptable_en.asp", "Display current arp table status",   0, 0},
  {0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_quickset[] = {
  {"Quick Setting", MENU_URL, "fc-page1_en.asp", "Quick setting", 0, 0},  {0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_lan[] = {
  {"LAN Interface", MENU_URL, "tcpiplan_en.asp", "LAN Interface", 0, 0},
  {"DHCP ", MENU_URL, "dhcpd_en.asp", "DHCP Setting", 0, 0},  {0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_wan[] = {
  {"Channel Setting", MENU_URL, "wanadsl_en.asp", "Channel Setting", 0, 0},
  {"ADSL Setting", MENU_URL, "/adv/adsl-set_en.asp", "ADSL Setting", 0, 0},
  {0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_wlan[] = {
  {"Basic Setting", MENU_URL, "/admin/wlbasic_en.asp",   "Setup wireless basic configuration", 0, 0},
#ifdef WLAN_MBSSID
  {"Secutity Setting", MENU_URL, "/admin/wlwpa_mbssid_en.asp",   "Setup wireless security", 0, 0},
#else
  {"Secutity Setting", MENU_URL, "/admin/wlwpa_en.asp", "Setup wireless security", 0,
   0},
#endif
  {"Advance Setting", MENU_URL, "/admin/wladvanced_en.asp",   "Setup wireless advanced configuration", 0, 0},
#ifdef WLAN_ACL
  {"WLAN Access Control", MENU_URL, "/admin/wlactrl_en.asp",   "Setup access control list for wireless clients", 0, 0},
#endif
#ifdef WLAN_WDS
  {"WDS Setting", MENU_URL, "/admin/wlwds_en.asp", "WDS Settings", 0, 0},
#endif
#ifdef CONFIG_WIFI_SIMPLE_CONFIG
  {"WPS Setting", MENU_URL, "/admin/wlwps_en.asp", "WPS Settings", 0, 0},
#endif
  {0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_firewall[]={
		{"IP/Port  Filter", MENU_URL,"fw-ipportfilter_en.asp", "no tip",0,0},
		{"MAC  Filter", MENU_URL,"fw-macfilter_en.asp", "no tip",0,0},
#ifdef URL_ALLOWING_SUPPORT
		{"URL Allowing", MENU_URL,"url_en.asp", "URL  Allowing ",0,0},
#elif  defined(URL_BLOCKING_SUPPORT)&&(!defined(URL_ALLOWING_SUPPORT)) 
              {"URL Blocking", MENU_URL,"url_blocking_en.asp", "URL  Blocking",0,0},
#endif
#ifdef LAYER7_FILTER_SUPPORT
		{"Application Forbidden", MENU_URL,"fw-layer7filter_en.asp", "no tip",0,0},	
#endif
		{0,0,0,0,0,0}
	};
struct RootMenu childmenu_vmserver[] = {
  {"Virtual Service", MENU_URL, "/admin/vrtsrv_en.asp", "Services Settings", 0, 0},
  {"DMZ Setting", MENU_URL, "/admin/fw-dmz_en.asp", "DMZ Settings", 0, 0},
  {0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_routersetting[]={
#ifndef CONFIG_USER_ZEBRA_OSPFD_OSPFD
		{"Rip Setting", MENU_URL,"rip_en.asp", "Rip Setting",0,0},
#else
		{"Ospf Setting", MENU_URL,"ospf_en.asp", "Ospf Setting",0,0},
#endif
		{"Static Routing", MENU_URL,"routing_en.asp", "Static Routing",0,0},
		{0,0,0,0,0,0}
	};
struct RootMenu childmenu_advothersetting[] = {
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
  {"UPNP Setting", MENU_URL, "/admin/upnp_en.asp", "UPnP configuration", 0, 0},
#endif
#ifdef CONFIG_USER_IGMPPROXY
  {"IGMP Proxy", MENU_URL, "igmproxy_en.asp", "IGMP Proxy", 0, 0},
#endif
  {"Bridge Setting", MENU_URL, "/admin/bridging_en.asp",   "configure bridge parameters", 0, 0},
  {"IP PassThrough", MENU_URL, "others_en.asp", "IP PassThrough Setting", 0, 0},
  #ifdef DOS_SUPPORT
{"DoS Setting", MENU_URL, "dos_en.asp", "Denial of service", 0, 0},
   #endif
  {0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_adv[] = {
   #ifdef CONFIG_EXT_SWITCH
#ifdef ITF_GROUP
#ifdef BPORTMAPPING_SUPPORT
 	 {"Port Mapping", MENU_URL, "/admin/eth2pvc_en.asp", "Assign Ethernet to PVC", 0,   0},
#endif
#endif
#endif
	{"IP QOS", MENU_URL, "ipqos_en.asp", "Setup QoS", 0, 0},
  	{"DNS Setting", MENU_URL, "dns_en.asp", "Setup how to obtain DNS servers", 0,   0},
  	{"Virtual Server", MENU_FOLDER, &childmenu_vmserver, "",   sizeof (childmenu_vmserver) / sizeof (struct RootMenu) - 1, 0},
	{"Routing Setting", MENU_FOLDER, &childmenu_routersetting, "",   sizeof (childmenu_routersetting) / sizeof (struct RootMenu) - 1, 0},
	{"Firewall Setting", MENU_FOLDER, &childmenu_firewall, "",   sizeof (childmenu_firewall) / sizeof (struct RootMenu) - 1, 0},
	{"Other Setting", MENU_FOLDER, &childmenu_advothersetting, "",   sizeof (childmenu_advothersetting) / sizeof (struct RootMenu) - 1, 0},   
  {0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_admin[] = {
 {"Passwd", MENU_URL, "/admin/password_en.asp",   "Setup access password", 0, 0},
 #ifdef TIME_ZONE
  {"Time Zone", MENU_URL, "/admin/date_en.asp", "Setup Time Zone", 0, 0},
#endif
#ifdef SYSLOG_SUPPORT
  {"System Log", MENU_URL, "/admin/syslog_en.asp", "Show system log", 0, 0},
#endif
 {"Refresh Setting", MENU_URL, "refresh_en.asp", "Refresh Setting", 0, 0},
  {"SNMP Setting", MENU_URL, "snmp_en.asp", "Snmp Setting", 0, 0},
  #ifdef _CWMP_MIB_
  {"TR069 Setting", MENU_URL, "tr069config_en.asp", "Tr069 Setting", 0, 0},
#endif
#ifdef IP_ACL
#ifndef MAC_ACL
  {"ACL Setting", MENU_URL, "acl_en.asp", "ACL Setting", 0, 0},
#else
  {"ACL Setting", MENU_URL, "acl_adv_en.asp", "ACL Setting", 0, 0},
#endif
#endif
{"Remote Access Control", MENU_URL, "rmtacc_en.asp", "Services Access Control", 0,   0},
#ifdef CONFIG_SAVE_RESTORE
  {"Backup/Restore", MENU_URL, "/admin/saveconf_en.asp",   "Backup/restore current settings", 0, 0},
#endif
#ifdef WEB_UPGRADE
#ifdef UPGRADE_V1
  {"Update Firmware", MENU_URL, "/admin/upload_en.asp", "Update firmware image", 0,   0},
#endif
#endif
{"Save/Reboot", MENU_URL, "/admin/reboot_en.asp", "Commit/reboot the system",   0, 0},
#ifdef NAT_CONN_LIMIT
	{"Connection Limit", MENU_URL, "connlimit_en.asp", "Connection Limit", 0, 0},
#endif
#ifdef CONFIG_IP_NF_ALG_ONOFF
  {"ALG On-Off", MENU_URL, "algonoff_en.asp", "ALG on-off", 0,   0},
 #endif 
  {0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_diag[] = {
  {"Ping", MENU_URL, "ping_en.asp", "Ping test", 0, 0},
  {"ATM Loopback", MENU_URL, "oamloopback_en.asp", "ATM OAM loopback test", 0,   0},
  {"ADSL", MENU_URL, "/adv/adsl-diag_en.asp", "ADSL Tone Diagnostics", 0, 0},
#ifdef DIAGNOSTIC_TEST
  {"Diagnostic test", MENU_URL, "diag-test_en.asp", "Diagnostic test", 0, 0},
#endif
  {0,0,0,0,0,0}
};
struct RootMenu rootmenu[] = {
  {"Status", MENU_FOLDER, &childmenu_system, "",   sizeof (childmenu_system) / sizeof (struct RootMenu) - 1, 0},
  {"Quick Setting", MENU_FOLDER, &childmenu_quickset, "",   sizeof (childmenu_quickset) / sizeof (struct RootMenu) - 1, 0},
  {"LAN Network", MENU_FOLDER, &childmenu_lan, "",   sizeof (childmenu_lan) / sizeof (struct RootMenu) - 1, 0},
#ifdef WLAN_SUPPORT
  {"WLAN Network", MENU_FOLDER, &childmenu_wlan, "",   sizeof (childmenu_wlan) / sizeof (struct RootMenu) - 1, 0},
#endif
  {"WAN Network", MENU_FOLDER, &childmenu_wan, "",  sizeof (childmenu_wan) / sizeof (struct RootMenu) - 1, 0},
  {"Adv Setting", MENU_FOLDER, &childmenu_adv, "",   sizeof (childmenu_adv) / sizeof (struct RootMenu) - 1, 0},
  {"System Managment", MENU_FOLDER, &childmenu_admin, "",   sizeof (childmenu_admin) / sizeof (struct RootMenu) - 1, 0},
  {"System Diagnose", MENU_FOLDER, &childmenu_diag, "",   sizeof (childmenu_diag) / sizeof (struct RootMenu) - 1, 0},
  {0, 0, 0, 0, 0, 0}
};
#endif // of defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)

/*
 * return: number of menu folders
 */
int
calcFolderNum (struct RootMenu *menu, int *maxchild)
{
  int i;
  int num;
  num = 0;
  while (menu->u.addr)
    {
      if (menu->type == MENU_FOLDER)
	{
	  if (menu->childrennums > *maxchild)
	    *maxchild = menu->childrennums;
	  num++;
	  num += calcFolderNum ((struct RootMenu *) menu->u.addr, maxchild);
	}
      menu++;
    }
  return num;
}
int
addMenu (webs_t wp, struct RootMenu *menu, int layer, int *folder_id,
	 int isMenuEnd)
{
  struct RootMenu *pMenu, *pChild;
  int index;
  int bytes = 0;
  int fid;
  int i;
  unsigned int peol, mask;
  index = 0;
  fid = *folder_id;
  pMenu = menu;
  peol = pMenu->eol;
  // traverse this layer
  while (pMenu->u.addr != 0)
    {
      if (pMenu->type == MENU_FOLDER)
	{
	  if (layer == 0)
	    {
	      bytes +=
		websWrite (wp,
			   T
			   ("<div ID='Main%d' onClick=\"expandMenu('Submenu%d','Btn%d', 0); return false\">\n"
			    "<table border=0 cellpadding=0 cellspacing=0>\n"),
			   *folder_id, *folder_id, *folder_id);
	      bytes += websWrite (wp, T ("<tr>"));
	    }
	  else
	    {
	      bytes +=
		websWrite (wp,
			   T
			   ("<tr ID='Main%d' onClick=\"expandMenu('Submenu%d','Btn%d', %d); return false\">\n"),
			   *folder_id, *folder_id, *folder_id,
			   pMenu->childrennums);
	    }
	  for (i = 0; i < layer; i++)
	    {
	      bytes +=
		websWrite (wp, T ("<td width=18 height=18>%s</td>\n"),
			   (peol & (0x01 << i)) ? "" :
			   "<img src=menu-images/menu_bar.gif width=18 height=18>");
	    }
	  // display DIR
	  bytes +=
	    websWrite (wp,
		       T
		       ("<td width=18 height=18><img src=menu-images/%s width=18 height=18></td>\n"),
		       (pMenu +
			1)->u.addr ? "menu_tee.gif" : "menu_corner.gif");
	  bytes +=
	    websWrite (wp,
		       T
		       ("<td width=18 height=18><img src=menu-images/menu_folder_closed.gif id='Btn%d' width=18 height=18></td>\n"),
		       *folder_id);
	  bytes +=
	    websWrite (wp,
		       T
		       ("<td height=18 colspan=%d><a href=\"\"><span class=\"link\">%s</span></a></td>\n"),
		       3 - layer, pMenu->name);
	  if (layer == 0)
	    bytes += websWrite (wp, T ("</tr>\n</table></div>\n"));
	  else
	    bytes += websWrite (wp, T ("</tr>\n"));
	  // expanding ....
	  bytes +=
	    websWrite (wp,
		       T
		       ("<div ID='Submenu%dChild' style=\"display:none\">\n"),
		       *folder_id);
	  if (layer == 0)
	    bytes +=
	      websWrite (wp,
			 T
			 ("<table border=0 cellpadding=0 cellspacing=0>\n"));
	  //if (layer != 0 || index != 0)
	  *folder_id = *folder_id + 1;
	  pMenu->eol = peol;	// copy the eol history
	  // calculate my eol bit
	  // clear bit
	  mask = 0xffffffff ^ (0x01 << layer);
	  pMenu->eol &= mask;
	  // add bit
	  if ((pMenu + 1)->u.addr == 0)	// It's EoL
	    pMenu->eol |= (0x01 << layer);
	  pChild = (struct RootMenu *) pMenu->u.addr;
	  pChild->eol = pMenu->eol;	// deliver eol to the succeed
	  if ((pMenu + 1)->u.addr)
	    addMenu (wp, pChild, layer + 1, folder_id, 0);
	  else
	    addMenu (wp, pChild, layer + 1, folder_id, 1);
	  if (layer == 0)
	    bytes += websWrite (wp, T ("</table>"));
	  bytes += websWrite (wp, T ("</div>\n"));
	}
      else if (pMenu->type == MENU_URL)
	{
	  if (layer == 0)
	    {
	      bytes += websWrite (wp, T ("<div ID='Menu%dChild'>\n"), index);
	      bytes +=
		websWrite (wp,
			   T
			   ("<table border=0 cellpadding=0 cellspacing=0>\n"));
	      bytes += websWrite (wp, T ("<tr>"));
	    }
	  else
	    {
	      if (layer != 1)
		bytes +=
		  websWrite (wp, T ("<tr ID='Submenu%dChild%d'>\n"), fid - 1,
			     index);
	      else
		bytes += websWrite (wp, T ("<tr>\n"));
	    }
	  for (i = 0; i < layer; i++)
	    {
	      bytes +=
		websWrite (wp, T ("<td width=18 height=18>%s</td>\n"),
			   (peol & (0x01 << i)) ? "" :
			   "<img src=menu-images/menu_bar.gif width=18 height=18>");
	    }
	  // display URL
	  bytes +=
	    websWrite (wp,
		       T
		       ("<td width=18 height=18><img src=menu-images/%s width=18 height=18></td>\n"),
		       (pMenu +
			1)->u.addr ? "menu_tee.gif" : "menu_corner.gif");
	  bytes +=
	    websWrite (wp,
		       T
		       ("<td width=18 height=18><img src=menu-images/menu_link_default.gif width=18 height=18></td>\n"));
	  bytes +=
	    websWrite (wp,
		       T
		       ("<td height=18 colspan=%d><a href=%s target=\"view\" class=\"link\"><span title=\"%s\" class=\"link\">%s</span></a></td></tr>\n"),
		       3 - layer, pMenu->u.url, pMenu->tip, pMenu->name);
	  if (layer == 0)
	    bytes += websWrite (wp, T ("</table></div>\n"));
	}
      index++;
      pMenu++;
    }				// of while()
  return bytes;
}
int
addMenuHead (webs_t wp, char *menuname, int index, unsigned char isEnd)
{
  int bytes = 0;
  bytes +=
    websWrite (wp,
	       T
	       ("<div ID='Main%d' onClick=\"expandMenu('Submenu%d','Btn%d','menuChildTable%d'); return false\" >\n"
		"<table  border=0 cellpadding=0 cellspacing=0>\n <tr>\n"
		" <td width=18 height=18><img src=\"menu-images/%s\" width=18 height=18></td>\n"
		" <td width=18 height=18><img src=menu-images/menu_folder_closed.gif id='Btn%d'  width=18 height=18></td>\n"
		"<td  height=18 colspan=3><a href=\"\"><span class=\"link\">%s</span></a></td>\n</tr>\n</table></div>\n"),
	       index, index, index, index,
	       ((isEnd) ? "menu_corner.gif" : "menu_tee.gif"), index,
	       menuname);
  bytes += websWrite (wp, T ("<div ID='Submenu%dChild' class='Child' >\n"
			     "<table   border=0 cellpadding=0 cellspacing=0 style=\"display:none\" id=\"menuChildTable%d\">\n"),
		      index, index);
  return bytes;
}
int
addinMenuHead (webs_t wp, char *menuname, int index, unsigned char nums,
	       unsigned char isChildEnd)
{
  int bytes = 0;
  bytes +=
    websWrite (wp,
	       T
	       ("<tr  ID='Main%d' onClick=\"onlyexpandMenu('Submenu%d','Btn%d','pbtn%d',%d,%d); return false\">\n"
		" <td width=18 height=18><img src=menu-images/menu_bar.gif width=18 height=18></td>\n"),
	       index, index, index, index, nums, isChildEnd);
  bytes +=
    websWrite (wp,
	       T
	       ("<td width=18 height=18><img src=\"menu-images/%s\" width=18 height=18 id='pbtn%d'></td>\n"
		"<td height=18 width=18><img src=menu-images/menu_folder_closed.gif width=18 height=18 id='Btn%d' ></td>\n"
		" <td height=18  colspan=2><span class=\"link\">%s</span></td>\n </tr>\n"),
	       ((isChildEnd) ? "menu_corner_plus.gif" : "menu_tee_plus.gif"),
	       index, index, menuname);
  return bytes;
}
int
addMenuTail (webs_t wp)
{
  return websWrite (wp, T ("</table></div>\n"));
}
int
addChildMenu (webs_t wp, char *menuname, char *url, char *tip,
	      unsigned char isEnd, unsigned char isRootMenuEnd)
{
  return websWrite (wp, T ("<tr><td width=18 height=18>%s</td>\n"
			   "<td width=18 height=18><img src=\"menu-images/%s\" width=18 height=18></td>\n<td width=18 height=18><img src=menu-images/menu_link_default.gif width=18 height=18></td>\n"
			   "<td  height=18 colspan=2><a href=%s target=\"view\" class=\"link\"><span title=\"%s\" class=\"link\">%s</span></a></td></tr>\n"),
		    ((isRootMenuEnd) ? "&nbsp&nbsp" :
		     "<img src=\"menu-images/menu_bar.gif\" width=18 height=18>"),
		    ((isEnd) ? "menu_corner.gif" : "menu_tee.gif"), url, tip,
		    menuname);
}
int
addInChildMenu (webs_t wp, char *menuname, char *url, char *tip, int index,
		int IdIndex, unsigned char isEnd, unsigned char isChildEnd)
{
  int nBytes = 0;
  if (index == 0)
    nBytes +=
      websWrite (wp, T ("<div ID='Submenu%dChild' class='Child'>\n"),
		 IdIndex);
  nBytes =
    websWrite (wp,
	       T ("<tr ID='Submenu%dChild%d' style=\"display:none\">"
		  "<td width=18 height=18><img src=menu-images/menu_bar.gif width=18 height=18></td>\n"
		  "<td width=18 height=18>%s</td>\n"
		  "<td width=18 height=18><img src=menu-images/%s width=18 height=18></td>\n"
		  "<td width=18 height=18><img src=menu-images/menu_link_default.gif width=18 height=18></td>\n"
		  "<td ><a href=%s target=\"view\" class=\"link\"><span title=\"%s\" >%s</span></a></td></tr>\n"),
	       IdIndex, index,
	       ((isChildEnd) ? "&nbsp&nbsp" :
		"<img src=menu-images/menu_bar.gif width=18 height=18>"),
	       ((isEnd) ? "menu_corner.gif" : "menu_tee.gif"), url, tip,
	       menuname);
  if (isEnd)
    websWrite (wp, T ("</div>\n"));
  return nBytes;
}
// parm: 1 -> javascript support
//       0 -> no js
#if 0
int
zteStatusMenu (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *parm;
  if (ejArgs (argc, argv, T ("%s"), &parm) < 1)
    {
      websError (wp, 400, T ("Insufficient args\n"));
      return -1;
    }
#if 0
  if (parm[0] == '1')
    {
      websWrite (wp, T ("menu.addItem(\"ϵͳ״̬\");"
			"stus = new MTMenu();"
			"stus.addItem(\"ϵͳ��Ϣ\", \"/admin/status_sc.asp\", \"\",\"Display current base status\");"
			"stus.addItem(\"��������\", \"lan_status_sc.asp\", \"\",\"Display current lan status\");"
			"stus.addItem(\"��������\", \"wlan_status_sc.asp\", \"\",\"Display current wlan status\");"
			"stus.addItem(\"��������\", \"wan_status_sc.asp\", \"\",\"Display current wan status\");"
			"stus.addItem(\"�˿ڰ�\", \"status_eth2pvc_sc.asp\", \"\",\"Display current portband status\");"));
      websWrite (wp, T ("stus.addItem(\"����ͳ��\");"
			"stis = new MTMenu();"
			"stis.addItem(\"�ӿ�ͳ��\", \"stats_sc.asp\", \"\",\"Display current flow status\");"
			"stis.addItem(\"DSLͳ��\", \"/adv/adsl-statis_sc.asp\", \"\",\"Display current adsl status\");"
			"stus.makeLastSubmenu(stis);"));
      websWrite (wp,
		 T
		 ("stus.addItem(\"ARP�б�\", \"arptable_sc.asp\", \"\",\"Display current arp table status\");"
		  "menu.makeLastSubmenu(stus);"));
    }
  else
    {
      websWrite (wp, T ("<tr><td><b>ϵͳ״̬</b></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/status_sc.asp\" target=\"view\">ϵͳ��Ϣ</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"lan_status_sc.asp\" target=\"view\">��������</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"wlan_status_sc.asp\" target=\"view\">��������</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"wan_status_sc.asp\" target=\"view\">��������</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"status_eth2pvc_sc.asp\" target=\"view\">�˿ڰ�</a></td></tr>\n"));
      websWrite (wp, T ("<tr><td>&nbsp;&nbsp;<b>����ͳ��</b></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"stats_sc.asp\" target=\"view\">�ӿ�ͳ��</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"/adv/adsl-statis_sc.asp\" target=\"view\">DSLͳ��</a></td></tr>\n"));
      websWrite (wp,
		 T
		 ("<tr><td>&nbsp;&nbsp;<a href=\"arptable_sc.asp\" target=\"view\">ARP�б�</a></td></tr>\n"));
    }
#endif
  if (parm[0] == '1')
    {
      addMenuHead (wp, "ϵͳ״̬", 0);
      addChildMenu (wp, "ϵͳ��Ϣ", "/admin/status_sc.asp",
		    "Display current base status");
      addChildMenu (wp, "��������", "lan_status_sc.asp",
		    "Display current lan status");
      addChildMenu (wp, "��������", "wlan_status_sc.asp",
		    "Display current wlan status");
      addChildMenu (wp, "��������", "wan_status_sc.asp",
		    "Display current wan status");
      addChildMenu (wp, "�˿ڰ�", "status_eth2pvc_sc.asp",
		    "Display current portband status");
      websWrite (wp, T ("<tr><td colspan=\"2\">\n"));
      addinMenuHead (wp, "����ͳ��", 7);
      websWrite (wp,
		 T
		 ("<tr><td>&nbsp&nbsp&nbsp</td><td><img src=menu-images/menu_link_default.gif  hspace=10></td>\n"
		  "<td align=center><a href=%s target=\"view\"><span title=\"%s\" class=\"link\">%s</span></a></td></tr>\n"),
		 "stats_sc.asp", "Display current flow status", "�ӿ�ͳ��");
      websWrite (wp,
		 T
		 ("<tr><td>&nbsp&nbsp&nbsp</td><td><img src=menu-images/menu_link_default.gif  hspace=10></td>\n"
		  "<td align=center><a href=%s target=\"view\"><span title=\"%s\" class=\"link\">%s</span></a></td></tr>\n"),
		 "/adv/adsl-statis_sc.asp", "Display current adsl status",
		 "DSLͳ��");
      addMenuTail (wp);
      websWrite (wp, T ("</td></tr>"));
      addChildMenu (wp, "ARP�б�", "arptable_sc.asp",
		    "Display current arp table status");
      addMenuTail (wp);
    }
  else
    {
      websWrite (wp, T ("<tr><td><b>ϵͳ״̬</b></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/status_sc.asp\" target=\"view\">ϵͳ��Ϣ</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"lan_status_sc.asp\" target=\"view\">��������</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"wlan_status_sc.asp\" target=\"view\">��������</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"wan_status_sc.asp\" target=\"view\">��������</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"status_eth2pvc_sc.asp\" target=\"view\">�˿ڰ�</a></td></tr>\n"));
      websWrite (wp, T ("<tr><td>&nbsp;&nbsp;<b>����ͳ��</b></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"stats_sc.asp\" target=\"view\">�ӿ�ͳ��</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"/adv/adsl-statis_sc.asp\" target=\"view\">DSLͳ��</a></td></tr>\n"));
      websWrite (wp,
		 T
		 ("<tr><td>&nbsp;&nbsp;<a href=\"arptable_sc.asp\" target=\"view\">ARP�б�</a></td></tr>\n"));
    }
  return 0;
}
int
zteQuickSetMenu (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *parm;
  if (ejArgs (argc, argv, T ("%s"), &parm) < 1)
    {
      websError (wp, 400, T ("Insufficient args\n"));
      return -1;
    }
  if (parm[0] == '1')
    {
      websWrite (wp, T ("\n<table border=0 cellpadding=0 cellspacing=0>\n"));
      websWrite (wp,
		 T
		 ("<tr><td><img src=menu-images/menu_link_default.gif></td>\n"
		  "<td><a href=%s target=\"view\"><span title=\"%s\" class=\"link\">%s</span></a></td></tr>\n"),
		 "fc-page1-ch.asp", "Quick setup", "�����趨");
      websWrite (wp, T ("</table>\n"));
      /*websWrite(wp, T("menu.addItem(\"�����趨\", \"fc-page1-ch.asp\", \"\", \"Setup LAN Interface\");")); */
    }
  else
    {
      websWrite (wp,
		 T
		 ("<tr><td><a href=\"fc-page1-ch.asp\" target=\"view\">�����趨</a></td></tr>\n"));
    }
  return 0;
}
int
zteLanMenu (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *parm;
  if (ejArgs (argc, argv, T ("%s"), &parm) < 1)
    {
      websError (wp, 400, T ("Insufficient args\n"));
      return -1;
    }
  if (parm[0] == '1')
    {
      addMenuHead (wp, "��������", 1);
      addChildMenu (wp, "LAN�ӿ�", "tcpiplan_sc.asp", "LAN�ӿ�");
      addChildMenu (wp, "DHCP����", "dhcpd_sc.asp", "DHCP����");
      addMenuTail (wp);
    }
  else
    {
      websWrite (wp, T ("<tr><td><b>��������</b></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"tcpiplan_sc.asp\" target=\"view\">LAN�ӿ�</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"dhcpd_sc.asp\" target=\"view\">DHCP����</a></td></tr>\n"));
    }
  return 0;
}
int
zteWlanMenu (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *parm;
  if (ejArgs (argc, argv, T ("%s"), &parm) < 1)
    {
      websError (wp, 400, T ("Insufficient args\n"));
      return -1;
    }
  if (parm[0] == '1')
    {
      addMenuHead (wp, "��������", 2);
      addChildMenu (wp, "��������", "/admin/wlbasic_sc.asp",
		    "Setup wireless basic configuration");
      addChildMenu (wp, "�߼�����", "/admin/wladvanced_sc.asp",
		    "Setup wireless advanced configuration");
#ifdef WLAN_MBSSID
      addChildMenu (wp, "��ȫ����", "/admin/wlwpa_mbssid_sc.asp",
		    "Setup wireless security");
#else
      addChildMenu (wp, "��ȫ����", "/admin/wlwpa_sc.asp",
		    "Setup wireless security");
#endif
#ifdef WLAN_ACL
      addChildMenu (wp, "�������", "/admin/wlactrl_sc.asp",
		    "Setup access control list for wireless clients");
#endif
//xl_yue: 531b bridge do not need wds
#ifdef ZTE_GENERAL_ROUTER_SC
#ifdef WLAN_WDS
      addChildMenu (wp, "WDS����", "/admin/wlwds_sc.asp", "WDS Settings");
#endif
#ifdef WLAN_CLIENT
      addChildMenu (wp, "վ��ɨ��", "/admin/wlsurvey.asp",
		    "Wireless Site Survey");
#endif
#endif
#ifdef CONFIG_WIFI_SIMPLE_CONFIG	//WPS def WLAN_CLIENT
//              addChildMenu(wp,"WPS����", "/admin/wlwps.asp", "Wireless Protected Setup");
#endif
#ifdef WLAN_MBSSID
//              addChildMenu(wp,"MBSSID����", "/admin/wlmbssid.asp", "Wireless Multiple BSSID Setting");
#endif
      addMenuTail (wp);
    }
  else
    {
      websWrite (wp, T ("<tr><td><b>��������</b></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlbasic_sc.asp\" target=\"view\">��������</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/wladvanced_sc.asp\" target=\"view\">�߼�����</a></td></tr>"
#ifdef WLAN_MBSSID
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlwpa_mbssid_sc.asp\" target=\"view\">��ȫ����</a></td></tr>\n"
#else
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlwpa_sc.asp\" target=\"view\">��ȫ����</a></td></tr>\n"
#endif
#ifdef WLAN_ACL
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlactrl_sc.asp\" target=\"view\">�������</a></td></tr>\n"
#endif
//xl_yue: 531b bridge do not need wds
#ifdef ZTE_GENERAL_ROUTER_SC
#ifdef WLAN_WDS
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlwds_sc.asp\" target=\"view\">WDS����</a></td></tr>\n"
#endif
#ifdef WLAN_CLIENT
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlsurvey.asp\" target=\"view\">վ��ɨ��</a></td></tr>\n"
#endif
#endif
#ifdef CONFIG_WIFI_SIMPLE_CONFIG	// WPS
//                      "<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlwps.asp\" target=\"view\">WPS����</a></td></tr>\n"
#endif
#ifdef WLAN_MBSSID
//                      "<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlmbssid_sc.asp\" target=\"view\">MBSSID����</a></td></tr>\n"
#endif
		 ));
    }
  return 0;
}
int
zteWanMenu (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *parm;
  if (ejArgs (argc, argv, T ("%s"), &parm) < 1)
    {
      websError (wp, 400, T ("Insufficient args\n"));
      return -1;
    }
  if (parm[0] == '1')
    {
      addMenuHead (wp, "��������", 3);
      addChildMenu (wp, "�ӿ�����", "wanadsl_sc.asp", "�ӿ�����");
      addChildMenu (wp, "ADSL����", "/adv/adsl-set_sc.asp", "ADSL����");
      addMenuTail (wp);
    }
  else
    {
      websWrite (wp, T ("<tr><td><b>��������</b></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"wanadsl_sc.asp\" target=\"view\">�ӿ�����</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"/adv/adsl-set_sc.asp\" target=\"view\">ADSL����</a></td></tr>\n"));
    }
  return 0;
}
int
zteAdvanceMenu (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *parm;
  if (ejArgs (argc, argv, T ("%s"), &parm) < 1)
    {
      websError (wp, 400, T ("Insufficient args\n"));
      return -1;
    }
  if (parm[0] == '1')
    {
      addMenuHead (wp, "�߼��趨", 4);
#ifdef CONFIG_EXT_SWITCH
#ifdef ITF_GROUP
      addChildMenu (wp, "�˿ڰ�", "/admin/eth2pvc_sc.asp",
		    "Assign Ethernet to PVC");
#endif
#endif
      addMenuTail (wp);
    }
  else
    {
      websWrite (wp, T ("<tr><td><b>�߼��趨</b></td></tr>\n"
#ifdef CONFIG_EXT_SWITCH
#ifdef ITF_GROUP
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/eth2pvc_sc.asp\" target=\"view\">�˿ڰ�</a></td></tr>\n"
#endif
#endif
		 ));
    }
  return 0;
}
int
zteDiagMenu (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *parm;
  if (ejArgs (argc, argv, T ("%s"), &parm) < 1)
    {
      websError (wp, 400, T ("Insufficient args\n"));
      return -1;
    }
  if (parm[0] == '1')
    {
      addMenuHead (wp, "ϵͳ���", 6);
      addChildMenu (wp, "Ping", "ping-ch.asp", "Ping test");
      addChildMenu (wp, "ATM Loopback", "oamloopback-ch.asp",
		    "ATM OAM loopback test");
      addChildMenu (wp, "ADSL", "/adv/adsl-diag-ch.asp",
		    "ADSL Tone Diagnostics");
#ifdef DIAGNOSTIC_TEST
      addChildMenu (wp, "��ϲ���", "diag-test-ch.asp", "Diagnostic test");
#endif
      addMenuTail (wp);
    }
  else
    {
      websWrite (wp, T ("<tr><td><b>ϵͳ���</b></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"ping-ch.asp\" target=\"view\">Ping</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"oamloopback-ch.asp\" target=\"view\">ATM Loopback</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"/adv/adsl-diag-ch.asp\" target=\"view\">ADSL</a></td></tr>\n"
#ifdef DIAGNOSTIC_TEST
			"<tr><td>&nbsp;&nbsp;<a href=\"diag-test-ch.asp\" target=\"view\">��ϲ���</a></td></tr>\n"
#endif
		 ));
    }
  return 0;
}
int
zteAdminMenu (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *parm;
  if (ejArgs (argc, argv, T ("%s"), &parm) < 1)
    {
      websError (wp, 400, T ("Insufficient args\n"));
      return -1;
    }
  if (parm[0] == '1')
    {
      addMenuHead (wp, "ϵͳ����", 5);
      addChildMenu (wp, "�û�/�������", "/admin/password_sc.asp",
		    "Setup access password");
#ifdef CONFIG_SAVE_RESTORE
      addChildMenu (wp, "���ñ���/�ָ�", "/admin/saveconf_sc.asp",
		    "Backup/restore current settings");
#endif
#ifdef WEB_UPGRADE
#ifdef UPGRADE_V1
      addChildMenu (wp, "�����汾", "/admin/upload_sc.asp",
		    "Update firmware image");
#else
#ifdef UPGRADE_V2
      addChildMenu (wp, "�����汾", "/admin/upload2.asp",
		    "Update firmware image");
#endif // of UPGRADE_V2
#ifdef UPGRADE_V3
      addChildMenu (wp, "�����汾", "/admin/upload3.asp",
		    "Update firmware image");
#endif // of UPGRADE_V3
#endif // of UPGRADE_V1
#endif // of WEB_UPGRADE
#ifdef FINISH_MAINTENANCE_SUPPORT
      //added by xl_yue
      addChildMenu (wp, "ά������", "/finishmaintenance.asp",
		    "Finish Maintenance");
#endif
      addChildMenu (wp, "����/����", "/admin/reboot_sc.asp",
		    "Commit/reboot the system");
#if defined(TIME_ZONE) && !defined(ZTE_GENERAL_ROUTER_SC)
      addChildMenu (wp, "ʱ������", "ntp.asp", "Setup Time Zone");
#elif defined(ZTE_GENERAL_ROUTER_SC)
      addChildMenu (wp, "ʱ������", "date_sc.asp", "Setup Time Zone");
#endif
#ifdef SYSLOG_SUPPORT
      addChildMenu (wp, "ϵͳ��־", "/admin/syslog_sc.asp",
		    "Show system log");
#endif
/*
#ifdef DOS_SUPPORT
			"admin.addItem(\"DOS����\", \"dos.asp\", \"\", \"Denial of service\");"
#endif
*/
      /*
         "admin.addItem(\"Remote Config\", \"rconfig.asp\", \"\", \"Remote configuration from WAN side\");"
       */
#ifdef IP_ACL
      addChildMenu (wp, "���ʿ���", "/admin/acl_sc.asp", "ACL Setting");
#endif
      addChildMenu (wp, "SNMP����", "/admin/snmp_sc.asp", "SNMP Setting");
/*
#ifdef AUTO_PROVISIONING
			"admin.addItem(\"Auto-Provisionning\", \"autoprovision.asp\", \"\", \"Auto-Provisioning Configuration\");"
#endif
*/
/*531B bridge do not need
#ifdef _CWMP_MIB_
			"admin.addItem(\"TR-069����\", \"tr069config.asp\", \"\", \"TR-069 Configuration\");"
#endif
*/
//added by xl_yue
#ifdef USE_LOGINWEB_OF_SERVER
      addChildMenu (wp, "�˳���¼", "/admin/logout.asp", "Logout");
#endif
#ifdef ACCOUNT_LOGIN_CONTROL
      addChildMenu (wp, "�˳���¼", "/admin/adminlogout.asp", "Logout");
#endif
      addMenuTail (wp);
    }
  else
    {
      websWrite (wp, T ("<tr><td><b>ϵͳ����</b></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/password_sc.asp\" target=\"view\">�û�/�������</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/saveconf_sc.asp\" target=\"view\">���ñ���/�ָ�</a></td></tr>\n"
#ifdef WEB_UPGRADE
#ifdef UPGRADE_V1
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/upload_sc.asp\" target=\"view\">�����汾</a></td></tr>\n"
#else
#ifdef UPGRADE_V2
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/upload2.asp\" target=\"view\">�����汾</a></td></tr>\n"
#endif // of UPGRADE_V2
#ifdef UPGRADE_V3
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/upload3.asp\" target=\"view\">�����汾</a></td></tr>\n"
#endif // of UPGRADE_V3
#endif // of UPGRADE_V1
#endif // of WEB_UPGRADE
#ifdef FINISH_MAINTENANCE_SUPPORT
			//added by xl_yue
			"<tr><td>&nbsp;&nbsp;<a href=\"/finishmaintenance.asp\" target=\"view\">ά������</a></td></tr>\n"
#endif
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/reboot_sc.asp\" target=\"view\">����/����</a></td></tr>\n"
#if defined(TIME_ZONE) && !defined(ZTE_GENERAL_ROUTER_SC)
			"<tr><td>&nbsp;&nbsp;<a href=\"ntp.asp\" target=\"view\">ʱ������</a></td></tr>\n"
#elif defined(ZTE_GENERAL_ROUTER_SC)
			"<tr><td>&nbsp;&nbsp;<a href=\"date_sc.asp\" target=\"view\">ʱ������</a></td></tr>\n"
#endif
#ifdef SYSLOG_SUPPORT
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/syslog_sc.asp\" target=\"view\">ϵͳ��־</a></td></tr>\n"
#endif
/*
#ifdef DOS_SUPPORT
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/dos.asp\" target=\"view\">DOS����</a></td></tr>\n"
#endif
*/
			/*
			   "<tr><td>&nbsp;&nbsp;<a href=\"rconfig.asp\" target=\"view\">Remote Config</a></td></tr>\n"
			 */
#ifdef IP_ACL
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/acl_sc.asp\" target=\"view\">���ʿ���</a></td></tr>\n"
#endif
/*
#ifdef AUTO_PROVISIONING
			"<tr><td>&nbsp;&nbsp;<a href=\"autoprovision.asp\" target=\"view\">Auto-Provisioning Configuration</a></td></tr>\n"
#endif
*/
/* 531B do not need
#ifdef _CWMP_MIB_
			"<tr><td>&nbsp;&nbsp;<a href=\"tr069config.asp\" target=\"view\">TR-069 Config</a></td></tr>\n"
#endif
*/
//added by xl_yue
#ifdef USE_LOGINWEB_OF_SERVER
//                      "<tr><td>&nbsp;&nbsp;<a href=\"/admin/login.asp\" target=\"view\">Login</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/logout.asp\" target=\"view\">�˳���¼</a></td></tr>\n"
#endif
#ifdef ACCOUNT_LOGIN_CONTROL
			//added by xl_yue
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/adminlogout.asp\" target=\"view\">�˳���¼</a></td></tr>\n"
#endif
		 ));
    }
  return 0;
}
#endif
#else // of WEB_MENU_USE_NEW
// parm: 1 -> javascript support
//       0 -> no js
int
statusMenu (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *parm;
  if (ejArgs (argc, argv, T ("%s"), &parm) < 1)
    {
      websError (wp, 400, T ("Insufficient args\n"));
      return -1;
    }
  if (parm[0] == '1')
    {
      websWrite (wp,
		 T
		 ("menu.addItem(\"Status\", \"/admin/status.asp\", \"\",\"Display current status\");\n"));
    }
  else
    {
      websWrite (wp,
		 T
		 ("<tr><td><a href=\"/admin/status.asp\" target=\"view\">Status</a></td></tr>\n"));
    }
  return 0;
}
int
lanMenu (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *parm;
  if (ejArgs (argc, argv, T ("%s"), &parm) < 1)
    {
      websError (wp, 400, T ("Insufficient args\n"));
      return -1;
    }
  if (parm[0] == '1')
    {
      websWrite (wp,
		 T
		 ("menu.addItem(\"LAN Interface\", \"tcpiplan.asp\", \"\", \"Setup LAN Interface\");\n"));
    }
  else
    {
      websWrite (wp,
		 T
		 ("<tr><td><a href=\"tcpiplan.asp\" target=\"view\">LAN Interface</a></td></tr>\n"));
    }
  return 0;
}
int
wlanMenu (int eid, webs_t wp, int argc, char_t ** argv)
{
#ifdef WLAN_SUPPORT
  char *parm;
  if (ejArgs (argc, argv, T ("%s"), &parm) < 1)
    {
      websError (wp, 400, T ("Insufficient args\n"));
      return -1;
    }
  //if (wlan_is_up()) {
  if (parm[0] == '1')
    {
      websWrite (wp, T ("menu.addItem(\"Wireless\");\n"
			"wlan = new MTMenu();\n"
			"wlan.addItem(\"Basic Settings\", \"/admin/wlbasic.asp\", \"\", \"Setup wireless basic configuration\");\n"
			"wlan.addItem(\"Advanced Settings\", \"/admin/wladvanced.asp\", \"\", \"Setup wireless advanced configuration\");\n"
#ifdef WLAN_MBSSID
			"wlan.addItem(\"Security\", \"/admin/wlwpa_mbssid.asp\", \"\", \"Setup wireless security\");\n"));
#else
			"wlan.addItem(\"Security\", \"/admin/wlwpa.asp\", \"\", \"Setup wireless security\");\n"));
#endif
#ifdef WLAN_ACL
      websWrite (wp,
		 T
		 ("wlan.addItem(\"Access Control\", \"/admin/wlactrl.asp\", \"\", \"Setup access control list for wireless clients\");\n"));
#endif
#ifdef WLAN_WDS
      websWrite (wp,
		 T
		 ("wlan.addItem(\"WDS\", \"/admin/wlwds.asp\", \"\", \"WDS Settings\");\n"));
#endif
#ifdef WLAN_CLIENT
      websWrite (wp,
		 T
		 ("wlan.addItem(\"Site Survey\", \"/admin/wlsurvey.asp\", \"\", \"Wireless Site Survey\");\n"));
#endif
#ifdef CONFIG_WIFI_SIMPLE_CONFIG	//WPS def WLAN_CLIENT
      websWrite (wp,
		 T
		 ("wlan.addItem(\"WPS\", \"/admin/wlwps.asp\", \"\", \"Wireless Protected Setup\");\n"));
#endif
#ifdef WLAN_MBSSID
      websWrite (wp,
		 T
		 ("wlan.addItem(\"MBSSID\", \"/admin/wlmbssid.asp\", \"\", \"Wireless Multiple BSSID Setting\");\n"));
#endif
      websWrite (wp, T ("menu.makeLastSubmenu(wlan);\n"));
    }
  else
    {
      websWrite (wp, T ("<tr><td><b>Wireless</b></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlbasic.asp\" target=\"view\">Basic Settings</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/wladvanced.asp\" target=\"view\">Advanced Settings</a></td></tr>"
#ifdef WLAN_MBSSID
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlwpa_mbssid.asp\" target=\"view\">Security</a></td></tr>\n"));
#else
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlwpa.asp\" target=\"view\">Security</a></td></tr>\n"));
#endif
#ifdef WLAN_ACL
      websWrite (wp,
		 T
		 ("<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlactrl.asp\" target=\"view\">Access Control</a></td></tr>\n"));
#endif
#ifdef WLAN_WDS
      websWrite (wp,
		 T
		 ("<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlwds.asp\" target=\"view\">WDS Settings</a></td></tr>\n"));
#endif
#ifdef WLAN_CLIENT
      websWrite (wp,
		 T
		 ("<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlsurvey.asp\" target=\"view\">Wireless Site Survey</a></td></tr>\n"));
#endif
#ifdef CONFIG_WIFI_SIMPLE_CONFIG	// WPS
      websWrite (wp,
		 T
		 ("<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlwps.asp\" target=\"view\">Wireless Protected Setup</a></td></tr>\n"));
#endif
#ifdef WLAN_MBSSID
      websWrite (wp,
		 T
		 ("<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlmbssid.asp\" target=\"view\">Wireless Multiple BSSID Setting</a></td></tr>\n"));
#endif
    }
  //};
#endif
  return 0;
}
int
wanMenu (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *parm;
  if (ejArgs (argc, argv, T ("%s"), &parm) < 1)
    {
      websError (wp, 400, T ("Insufficient args\n"));
      return -1;
    }
  if (parm[0] == '1')
    {
      websWrite (wp, T ("menu.addItem(\"WAN Interface\");\n"
			"wan = new MTMenu();\n"
			"wan.addItem(\"Channel Config\", \"wanadsl.asp\", \"\", \"Setup Channel Mode\");\n"
			"wan.addItem(\"ATM Settings\", \"wanatm.asp\", \"\", \"Setup ATM\");\n"
			"wan.addItem(\"ADSL Settings\", \"/adv/adsl-set.asp\", \"\", \"Setup ADSL\");\n"
			"menu.makeLastSubmenu(wan);\n"));
    }
  else
    {
      websWrite (wp, T ("<tr><td><b>WAN Interface</b></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"wanadsl.asp\" target=\"view\">Channel Config</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"wanatm.asp\" target=\"view\">ATM Settings</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"/adv/adsl-set.asp\" target=\"view\">ADSL Settings</a></td></tr>\n"));
    }
  return 0;
}
int
srvMenu (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *parm;
  if (ejArgs (argc, argv, T ("%s"), &parm) < 1)
    {
      websError (wp, 400, T ("Insufficient args\n"));
      return -1;
    }
  if (parm[0] == '1')
    {
      websWrite (wp, T ("menu.addItem(\"Services\");\n"
			"service = new MTMenu();\n"
			"service.addItem(\"DHCP Mode\", \"dhcpmode.asp\", \"\", \"DHCP Mode Configuration\");\n"
			"service.addItem(\"DHCP Server\", \"dhcpd.asp\", \"\", \"DHCP Server Configuration\");\n"
			"service.addItem(\"DHCP Relay\", \"dhcrelay.asp\", \"\", \"DHCP Relay Configuration\");\n"));
      websWrite (wp, T ("service.addItem(\"DNS\");\n"
			"dns = new MTMenu();\n"
			"dns.addItem(\"DNS Server\", \"dns.asp\", \"\", \"DNS Configuration\");\n"
#ifdef CONFIG_USER_DDNS
			"dns.addItem(\"Dynamic DNS\", \"ddns.asp\", \"\", \"DDNS Configuration\");\n"
#endif
			"service.makeLastSubmenu(dns);\n"));
      websWrite (wp, T ("service.addItem(\"Firewall\");\n"
			"firewall = new MTMenu();\n"
			"firewall.addItem(\"IP/Port Filtering\", \"fw-ipportfilter.asp\", \"\", \"Setup IP/Port filering\");\n"
#ifdef MAC_FILTER
			"firewall.addItem(\"MAC Filtering\", \"/admin/fw-macfilter.asp\", \"\", \"Setup MAC filering\");\n"
#endif
			"firewall.addItem(\"Port Forwarding\", \"fw-portfw.asp\", \"\", \"Setup port-forwarding\");\n"));
#ifdef URL_BLOCKING_SUPPORT
      websWrite (wp,
		 T
		 ("firewall.addItem(\"URL Blocking\", \"url_blocking.asp\", \"\", \"URL Blocking Setting\");\n"));
#endif
#ifdef DOMAIN_BLOCKING_SUPPORT
      websWrite (wp,
		 T
		 ("firewall.addItem(\"Domain Blocking\", \"domainblk.asp\", \"\", \"Domain Blocking Setting\");\n"));
#endif
#ifdef NATIP_FORWARDING
      websWrite (wp,
		 T
		 ("firewall.addItem(\"NAT IP Forwarding\", \"fw-ipfw.asp\", \"\", \"Setup NAT IP Mapping\");\n"));
#endif
#ifdef PORT_TRIGGERING
      websWrite (wp,
		 T
		 ("firewall.addItem(\"Port Triggering\", \"gaming.asp\", \"\", \"Setup Port Triggering\");\n"));
#endif
      websWrite (wp,
		 T
		 ("firewall.addItem(\"DMZ\", \"fw-dmz.asp\", \"\", \"Setup DMZ\");\n"
		  "service.makeLastSubmenu(firewall);\n"
#ifdef CONFIG_USER_IGMPPROXY
		  "service.addItem(\"IGMP Proxy\", \"igmproxy.asp\", \"\", \"IGMP Proxy Configuration\");\n"
#endif
//#ifdef CONFIG_USER_UPNPD
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
		  "service.addItem(\"UPnP\", \"upnp.asp\", \"\", \"UPnP Configuration\");\n"
#endif
#ifdef CONFIG_USER_ROUTED_ROUTED
		  "service.addItem(\"RIP\", \"rip.asp\", \"\", \"RIP Configuration\");\n"
#endif
#ifdef WEB_REDIRECT_BY_MAC
		  "service.addItem(\"Landing Page\", \"landong.asp\", \"\", \"Landing Page Configuration\");\n"
#endif
		  "menu.makeLastSubmenu(service);\n"));
      websWrite (wp, T ("menu.makeLastSubmenu(service);\n"));
    }
  else
    {
      websWrite (wp, T ("<tr><td><b>Services</b></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"dhcpmode.asp\" target=\"view\">DHCP Mode</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"dhcpd.asp\" target=\"view\">DHCP Server</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"dhcrelay.asp\" target=\"view\">DHCP Relay</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"dns.asp\" target=\"view\">DNS</a></td></tr>\n"
#ifdef CONFIG_USER_DDNS
			"<tr><td>&nbsp;&nbsp;<a href=\"ddns.asp\" target=\"view\">Dynamic DNS</a></td></tr>\n"
#endif
		 ));
      websWrite (wp, T ("<tr><td>&nbsp;&nbsp;<b>Firewall</b></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"fw-ipportfilter.asp\" target=\"view\">IP/Port Filtering</a></td></tr>\n"
#ifdef MAC_FILTER
			"<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"/admin/fw-macfilter.asp\" target=\"view\">MAC Filtering</a></td></tr>\n"
#endif
			"<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"fw-portfw.asp\" target=\"view\">Port Forwarding</a></td></tr>\n"));
#ifdef URL_BLOCKING_SUPPORT
      websWrite (wp,
		 T
		 ("<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"url_blocking.asp\" target=\"view\">URL Blocking</a></td></tr>\n"));
#endif
#ifdef DOMAIN_BLOCKING_SUPPORT
      websWrite (wp,
		 T
		 ("<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"domainblk.asp\" target=\"view\">Domain Blocking</a></td></tr>\n"));
#endif
#ifdef NATIP_FORWARDING
      websWrite (wp,
		 T
		 ("<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"fw-ipfw.asp\" target=\"view\">NAT IP Forwarding</a></td></tr>\n"));
#endif
#ifdef PORT_TRIGGERING
      websWrite (wp,
		 T
		 ("<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"gaming.asp\" target=\"view\">Port Triggering</a></td></tr>\n"));
#endif
      websWrite (wp,
		 T
		 ("<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"fw-dmz.asp\" target=\"view\">DMZ</a></td></tr>"
#ifdef CONFIG_USER_IGMPPROXY
		  "<tr><td>&nbsp;&nbsp;<a href=\"igmproxy.asp\" target=\"view\">IGMP Proxy</a></td></tr>"
#endif
//#ifdef CONFIG_USER_UPNPD
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
		  "<tr><td>&nbsp;&nbsp;<a href=\"upnp.asp\" target=\"view\">UPnP</a></td></tr>"
#endif
#ifdef CONFIG_USER_ROUTED_ROUTED
		  "<tr><td>&nbsp;&nbsp;<a href=\"rip.asp\" target=\"view\">RIP</a></td></tr>"
#endif
#ifdef WEB_REDIRECT_BY_MAC
		  "<tr><td>&nbsp;&nbsp;<a href=\"landing.asp\" target=\"view\">Landing Page</a></td></tr>"
#endif
		 ));
    }
  return 0;
}
int
advanceMenu (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *parm;
  if (ejArgs (argc, argv, T ("%s"), &parm) < 1)
    {
      websError (wp, 400, T ("Insufficient args\n"));
      return -1;
    }
  if (parm[0] == '1')
    {
      websWrite (wp, T ("menu.addItem(\"Advance\");\n"
			"advance = new MTMenu();\n"
			"advance.addItem(\"ARP table\", \"arptable.asp\", \"\", \"ARP table\");\n"
			"advance.addItem(\"Bridging\", \"bridging.asp\", \"\", \"Bridge Configuration\");\n"
			"advance.addItem(\"Routing\", \"routing.asp\", \"\", \"Routing Configuration\");\n"
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
			"advance.addItem(\"SNMP\", \"snmp.asp\", \"\", \"SNMP Protocol Configuration\");\n"
#endif
#if (defined( CONFIG_EXT_SWITCH) && defined(ITF_GROUP)) || defined(CONFIG_RTL867X_COMBO_PORTMAPPING)
#ifdef BPORTMAPPING_SUPPORT
			"advance.addItem(\"Port Mapping\", \"eth2pvc.asp\", \"\", \"Assign Ethernet to PVC\");\n"
#endif
#endif
#ifdef IP_QOS
			"advance.addItem(\"IP QoS\", \"ipqos.asp\", \"\", \"IP QoS Configuration\");\n"
#ifdef CONFIG_EXT_SWITCH
#ifdef ELAN_LINK_MODE
			"advance.addItem(\"Link Mode\", \"lnkmode.asp\", \"\", \"Ethernet Link Mode Setting\");\n"
#endif
#endif
			"advance.addItem(\"Remote Access\", \"rmtacc.asp\", \"\", \"Services Access Control\");\n"
			"advance.addItem(\"Others\", \"others.asp\", \"\", \"Other advanced Configuration\");\n"
			"menu.makeLastSubmenu(advance);\n"));
    }
  else
    {
      websWrite (wp, T ("<tr><td><b>Advance</b></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"arptable.asp\" target=\"view\">ARP Table</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"bridging.asp\" target=\"view\">Bridging</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"routing.asp\" target=\"view\">Routing</a></td></tr>\n"
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
			"<tr><td>&nbsp;&nbsp;<a href=\"snmp.asp\" target=\"view\">SNMP</a></td></tr>\n"
#endif
#if (defined( CONFIG_EXT_SWITCH) && defined(ITF_GROUP)) || defined(CONFIG_RTL867X_COMBO_PORTMAPPING)
#ifdef BPORTMAPPING_SUPPORT
			"<tr><td>&nbsp;&nbsp;<a href=\"eth2pvc.asp\" target=\"view\">Port Mapping</a></td></tr>\n"
#endif
#endif
#endif
#ifdef CONFIG_EXT_SWITCH
#ifdef ELAN_LINK_MODE
			"<tr><td>&nbsp;&nbsp;<a href=\"lnkmode.asp\" target=\"view\">Link Mode</a></td></tr>\n"
#endif
#endif
			"<tr><td>&nbsp;&nbsp;<a href=\"rmtacc.asp\" target=\"view\">Remote Access</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"others.asp\" target=\"view\">Others</a></td></tr>\n"));
    }
  return 0;
}
int
diagMenu (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *parm;
  if (ejArgs (argc, argv, T ("%s"), &parm) < 1)
    {
      websError (wp, 400, T ("Insufficient args\n"));
      return -1;
    }
  if (parm[0] == '1')
    {
      websWrite (wp, T ("menu.addItem(\"Diagnostic\");\n"
			"diag = new MTMenu();\n"
			"diag.addItem(\"Ping\", \"ping.asp\", \"\", \"Ping test\");\n"
			"diag.addItem(\"ATM Loopback\", \"oamloopback.asp\", \"\", \"ATM OAM loopback test\");\n"
			"diag.addItem(\"ADSL\", \"/adv/adsl-diag.asp\", \"\", \"ADSL Tone Diagnostics\");\n"
#ifdef DIAGNOSTIC_TEST
			"diag.addItem(\"Diagnostic Test\", \"diag-test.asp\", \"\", \"Diagnostic test\");\n"
#endif
		 ));
      websWrite (wp, T ("menu.makeLastSubmenu(diag);\n"));
    }
  else
    {
      websWrite (wp, T ("<tr><td><b>Diagnostic</b></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"ping.asp\" target=\"view\">Ping</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"oamloopback.asp\" target=\"view\">ATM Loopback</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"/adv/adsl-diag.asp\" target=\"view\">ADSL</a></td></tr>\n"
#ifdef DIAGNOSTIC_TEST
			"<tr><td>&nbsp;&nbsp;<a href=\"diag-test.asp\" target=\"view\">Diagnostic Test</a></td></tr>\n"
#endif
		 ));
    }
  return 0;
}
int
adminMenu (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *parm;
  if (ejArgs (argc, argv, T ("%s"), &parm) < 1)
    {
      websError (wp, 400, T ("Insufficient args\n"));
      return -1;
    }
  if (parm[0] == '1')
    {
      websWrite (wp, T ("menu.addItem(\"Admin\");\n" "admin = new MTMenu();\n"
#ifdef FINISH_MAINTENANCE_SUPPORT
			//added by xl_yue
			"admin.addItem(\"Finish Maintenance\", \"/finishmaintenance.asp\", \"\", \"Finish Maintenance\");\n"
#endif
			"admin.addItem(\"Commit/Reboot\", \"/admin/reboot.asp\", \"\", \"Commit/reboot the system\");\n"
#ifdef CONFIG_SAVE_RESTORE
			"admin.addItem(\"Backup/Restore\", \"/admin/saveconf.asp\", \"\", \"Backup/restore current settings\");\n"
#endif
#ifdef ACCOUNT_LOGIN_CONTROL
			//added by xl_yue
			"admin.addItem(\"Logout\", \"/admin/adminlogout.asp\", \"\", \"Logout\");\n"
#endif
#ifdef SYSLOG_SUPPORT
#ifndef SEND_LOG
			"admin.addItem(\"System Log\", \"syslog.asp\", \"\", \"Show system log\");\n"
#else
			"admin.addItem(\"System Log\", \"syslog_server.asp\", \"\", \"Show system log\");\n"
#endif
#endif
#ifdef DOS_SUPPORT
			"admin.addItem(\"DOS\", \"dos.asp\", \"\", \"Denial of service\");\n"
#endif
			"admin.addItem(\"Password\", \"password.asp\", \"\", \"Setup access password\");\n"));
      websWrite (wp, T (
#ifdef WEB_UPGRADE
#ifdef UPGRADE_V1
			 "admin.addItem(\"Upgrade Firmware\", \"/admin/upload.asp\", \"\", \"Update firmware image\");\n"
#else
#ifdef UPGRADE_V2
			 "admin.addItem(\"Upgrade Firmware\", \"/admin/upload2.asp\", \"\", \"Update firmware image\");\n"
#endif // of UPGRADE_V2
#ifdef UPGRADE_V3
			 "admin.addItem(\"Upgrade Firmware\", \"/admin/upload3.asp\", \"\", \"Update firmware image\");\n"
#endif // of UPGRADE_V3
#endif // of UPGRADE_V1
#endif // of WEB_UPGRADE
			 /*
			    "admin.addItem(\"Remote Config\", \"rconfig.asp\", \"\", \"Remote configuration from WAN side\");\n"
			  */
#ifdef IP_ACL
			 "admin.addItem(\"ACL Config\", \"/admin/acl.asp\", \"\", \"ACL Setting\");\n"
#endif
#ifdef TIME_ZONE
			 "admin.addItem(\"Time Zone\", \"ntp.asp\", \"\", \"Setup Time Zone\");\n"
#endif
#ifdef AUTO_PROVISIONING
			 "admin.addItem(\"Auto-Provisionning\", \"autoprovision.asp\", \"\", \"Auto-Provisioning Configuration\");\n"
#endif
#ifdef _CWMP_MIB_
			 "admin.addItem(\"TR-069 Config\", \"tr069config.asp\", \"\", \"TR-069 Configuration\");\n"
#endif
//added by xl_yue
#ifdef USE_LOGINWEB_OF_SERVER
//                      "admin.addItem(\"Login\", \"/admin/login.asp\", \"\", \"Login\");\n"
			 "admin.addItem(\"Logout\", \"/admin/logout.asp\", \"\", \"Logout\");\n"
#endif
		 ));
      websWrite (wp, T ("menu.makeLastSubmenu(admin);\n"));
    }
  else
    {
      websWrite (wp, T ("<tr><td><b>Admin</b></td></tr>\n"
#ifdef FINISH_MAINTENANCE_SUPPORT
			//added by xl_yue
			"<tr><td>&nbsp;&nbsp;<a href=\"/finishmaintenance.asp\" target=\"view\">Finish Maintenance</a></td></tr>\n"
#endif
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/reboot.asp\" target=\"view\">Commit/Reboot</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/saveconf.asp\" target=\"view\">Backup/Restore</a></td></tr>\n"
#ifdef ACCOUNT_LOGIN_CONTROL
			//added by xl_yue
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/adminlogout.asp\" target=\"view\">Logout</a></td></tr>\n"
#endif
#ifdef SYSLOG_SUPPORT
#ifndef
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/syslog.asp\" target=\"view\">System log</a></td></tr>\n"
#else
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/syslog_server.asp\" target=\"view\">System log</a></td></tr>\n"
#endif
#endif
#ifdef DOS_SUPPORT
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/dos.asp\" target=\"view\">DOS</a></td></tr>\n"
#endif
			"<tr><td>&nbsp;&nbsp;<a href=\"password.asp\" target=\"view\">Password</a></td></tr>\n"));
      websWrite (wp, T (
#ifdef WEB_UPGRADE
#ifdef UPGRADE_V1
			 "<tr><td>&nbsp;&nbsp;<a href=\"/admin/upload.asp\" target=\"view\">Upgrade Firmware</a></td></tr>\n"
#else
#ifdef UPGRADE_V2
			 "<tr><td>&nbsp;&nbsp;<a href=\"/admin/upload2.asp\" target=\"view\">Upgrade Firmware</a></td></tr>\n"
#endif // of UPGRADE_V2
#ifdef UPGRADE_V3
			 "<tr><td>&nbsp;&nbsp;<a href=\"/admin/upload3.asp\" target=\"view\">Upgrade Firmware</a></td></tr>\n"
#endif // of UPGRADE_V3
#endif // of UPGRADE_V1
#endif // of WEB_UPGRADE
/*
			"<tr><td>&nbsp;&nbsp;<a href=\"rconfig.asp\" target=\"view\">Remote Config</a></td></tr>\n"
*/
#ifdef IP_ACL
			 "<tr><td>&nbsp;&nbsp;<a href=\"/admin/acl.asp\" target=\"view\">ACL Config</a></td></tr>\n"
#endif
#ifdef TIME_ZONE
			 "<tr><td>&nbsp;&nbsp;<a href=\"ntp.asp\" target=\"view\">Time Zone Setting</a></td></tr>\n"
#endif
#ifdef AUTO_PROVISIONING
			 "<tr><td>&nbsp;&nbsp;<a href=\"autoprovision.asp\" target=\"view\">Auto-Provisioning Configuration</a></td></tr>\n"
#endif
#ifdef _CWMP_MIB_
			 "<tr><td>&nbsp;&nbsp;<a href=\"tr069config.asp\" target=\"view\">TR-069 Config</a></td></tr>\n"
#endif
//added by xl_yue
#ifdef USE_LOGINWEB_OF_SERVER
//                      "<tr><td>&nbsp;&nbsp;<a href=\"/admin/login.asp\" target=\"view\">Login</a></td></tr>\n"
			 "<tr><td>&nbsp;&nbsp;<a href=\"/admin/logout.asp\" target=\"view\">Logout</a></td></tr>\n"
#endif
		 ));
    }
  return 0;
}
int
statisticsMenu (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *parm;
  if (ejArgs (argc, argv, T ("%s"), &parm) < 1)
    {
      websError (wp, 400, T ("Insufficient args\n"));
      return -1;
    }
  if (parm[0] == '1')
    {
      websWrite (wp, T ("menu.addItem(\"Statistics\");\n"
			"statis = new MTMenu();\n"
			"statis.addItem(\"Interfaces\", \"stats.asp\", \"\", \"Display packet statistics\");\n"
			"statis.addItem(\"ADSL\", \"/adv/adsl-statis.asp\", \"\", \"Display ADSL statistics\");\n"
			"menu.makeLastSubmenu(statis);\n"));
    }
  else
    {
      websWrite (wp, T ("<tr><td><b>Statistics</b></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"stats.asp\" target=\"view\">Interfaces</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"/adv/adsl-statis.asp\" target=\"view\">ADSL</a></td></tr>\n"));
    }
  return 0;
}
#endif // of WEB_MENU_USE_NEW
int
createMenu (int eid, webs_t wp, int argc, char_t ** argv)
{
#ifdef WEB_MENU_USE_NEW
  int i = 0, totalIdNums = 0, maxchildrensize = 0;
  int IdIndex = 0;
  unsigned char isRootMenuEnd = 0;
  //calc the id nums and the max children size
  totalIdNums = calcFolderNum (rootmenu, &maxchildrensize);
  //product the js code
  addMenuJavaScript (wp, totalIdNums, maxchildrensize);
  //create the header
  websWrite (wp, T ("<body  onload=\"initIt()\" bgcolor=\"#dadfdb\" >\n"));
  websWrite (wp,
	     T("<table width=100%% border=0 cellpadding=0 cellspacing=0>\n<tr><td  width=100%% align=left>\n"));
  websWrite (wp, T ("<table  border=0 cellpadding=0 cellspacing=0>\n"
		    "<tr><td width=18 height=18><img src=menu-images/menu_root.gif width=18 height=18></td>\n"
		    "<td  height=18 colspan=4 class=link><font size=3>%s</font></td></tr>\n </table>\n"),
	     menuTitle);
  if (rootmenu[1].u.addr)
    addMenu (wp, &rootmenu[0], 0, &IdIndex, 0);
  else
    addMenu (wp, &rootmenu[0], 0, &IdIndex, 1);
  websWrite (wp, T ("</td></tr>\n</table>\n"));
  /*zteStatusMenu(eid, wp, argc, argv);
     zteQuickSetMenu(eid, wp, argc, argv);
     zteLanMenu(eid, wp, argc, argv);
     zteWlanMenu(eid, wp, argc, argv);
     zteWanMenu(eid, wp, argc, argv);
     zteAdvanceMenu(eid, wp, argc, argv);
     zteAdminMenu(eid, wp, argc, argv);
     zteDiagMenu(eid, wp, argc, argv); */
#else
  statusMenu (eid, wp, argc, argv);
  lanMenu (eid, wp, argc, argv);
  wlanMenu (eid, wp, argc, argv);
  wanMenu (eid, wp, argc, argv);
  srvMenu (eid, wp, argc, argv);
  advanceMenu (eid, wp, argc, argv);
  diagMenu (eid, wp, argc, argv);
  adminMenu (eid, wp, argc, argv);
  statisticsMenu (eid, wp, argc, argv);
#endif
  return 0;
}
