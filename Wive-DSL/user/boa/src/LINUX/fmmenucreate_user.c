/*

*  fmmenucreate_user.c is used to create menu

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



#ifdef WEB_MENU_USE_NEW


#ifdef CONFIG_DEFAULT_WEB	// default pages

const char menuTitle_user[] = "Site contents:";

#elif defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC) ||defined(ZTE_GENERAL_ROUTER_EN)

const char menuTitle_user[] = "ZXDSL531B";

#endif


#ifdef CONFIG_DEFAULT_WEB	// default pages



/*

 *	First Layer Menu

 */

struct RootMenu childmenu_wlan_user[] = {

	{"Basic Settings", MENU_URL, "wlbasic.asp", "Setup wireless basic configuration", 0, 0},

	{"Advanced Settings", MENU_URL, "wladvanced.asp", "Setup wireless advanced configuration", 0, 0},

#ifdef WLAN_MBSSID

	{"Security", MENU_URL, "wlwpa_mbssid.asp", "Setup wireless security", 0, 0},

#else

	{"Security", MENU_URL, "wlwpa.asp", "Setup wireless security", 0, 0},

#endif

#ifdef WLAN_ACL

	{"Access Control", MENU_URL, "wlactrl.asp", "Setup access control list for wireless clients", 0, 0},

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

	{"MBSSID", MENU_URL, "wlmbssid.asp", "Wireless Multiple BSSID Setting", 0, 0},

#endif

	{0, 0, 0, 0, 0, 0}

};



struct RootMenu childmenu_wan_user[] = {

	{"Channel Config", MENU_URL, "wanadsl_user.asp", "Setup Channel Mode", 0, 0},

	{0, 0, 0, 0, 0, 0}

};



struct RootMenu childmenu_fw_user[] = {

#ifdef MAC_FILTER

	{"MAC Filtering", MENU_URL, "fw-macfilter.asp", "Setup MAC filering", 0, 0},

#endif

	{0, 0, 0, 0, 0, 0}

};



struct RootMenu childmenu_admin_user[] = {

	{"Commit/Reboot", MENU_URL, "reboot.asp", "Commit/reboot the system", 0, 0},

#ifdef ACCOUNT_LOGIN_CONTROL

	{"Logout", MENU_URL, "adminlogout.asp", "Logout", 0, 0},

#endif

	{"Password", MENU_URL, "user-password.asp", "Setup access password", 0, 0},

#ifdef IP_ACL

	{"ACL Config", MENU_URL, "acl.asp", "ACL Setting", 0, 0},

#endif

//added by xl_yue

#ifdef USE_LOGINWEB_OF_SERVER

	{"Logout", MENU_URL, "/admin/logout.asp", "Logout", 0, 0},

#endif

	{0, 0, 0, 0, 0, 0}

};



/*

 *	Root Menu

 */

struct RootMenu rootmenu_user[] = {

	{"Status", MENU_URL, "status.asp", "Display Current Status", 0, 0},

#ifdef WLAN_SUPPORT

	{"Wireless", MENU_FOLDER, &childmenu_wlan_user, "", sizeof(childmenu_wlan_user) / sizeof(struct RootMenu) - 1, 0},

#endif

	{"WAN", MENU_FOLDER, &childmenu_wan_user, "", sizeof(childmenu_wan_user) / sizeof(struct RootMenu) - 1, 0},

	{"Firewall", MENU_FOLDER, &childmenu_fw_user, "", sizeof(childmenu_fw_user) / sizeof(struct RootMenu) - 1, 0},

	{"Admin", MENU_FOLDER, &childmenu_admin_user, "", sizeof(childmenu_admin_user) / sizeof(struct RootMenu) - 1, 0},

	{0, 0, 0, 0, 0, 0}

};

#endif				// of CONFIG_DEFAULT_WEB



#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)

struct RootMenu childmenu_statistic_user[] = {
	{"接口统计", MENU_URL, "stats_sc.asp", "Display current flow status", 0, 0},
	{"DSL统计", MENU_URL, "/admin/adsl-statis_sc.asp", "Display current adsl status", 0, 0},
	{0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_system_user[] = {
	{"系统信息", MENU_URL, "/admin/status_sc.asp", "Display current base status", 0, 0},
	{"本地网络", MENU_URL, "lan_status_sc.asp", "Display current lan status", 0, 0},
#ifdef WLAN_SUPPORT
	{"无线网络", MENU_URL, "wlan_status_sc.asp", "Display current wlan status", 0, 0},
#endif
	{"广域网络", MENU_URL, "wan_status_sc.asp", "Display current wan status", 0, 0},
#ifdef CONFIG_EXT_SWITCH
#ifdef ITF_GROUP
	{"端口绑定", MENU_URL, "status_eth2pvc_sc.asp", "Display current portband status", 0, 0},
#endif

#endif

	{"网络统计", MENU_FOLDER, &childmenu_statistic_user, "", sizeof(childmenu_statistic_user) / sizeof(struct RootMenu) - 1, 0},

	{"ARP列表", MENU_URL, "arptable_sc.asp", "Display current arp table status", 0, 0},

	{0, 0, 0, 0, 0, 0}

};

struct RootMenu childmenu_lan_user[] = {

	{"LAN接口", MENU_URL, "tcpiplan_brg_sc.asp", "LAN接口", 0, 0},

	{"DHCP配置", MENU_URL, "dhcpd_brg_sc.asp", "DHCP配置", 0, 0},

	{0, 0, 0, 0, 0, 0}

};

struct RootMenu childmenu_wan_user[] = {

	{"接口配置", MENU_URL, "wanadsl_brg_sc.asp", "接口配置", 0, 0},

	{"ADSL配置", MENU_URL, "/adv/adsl-set_brg_sc.asp", "ADSL配置", 0, 0},

	{0, 0, 0, 0, 0, 0}

};

struct RootMenu childmenu_wlan_user[] = {

	{"基本设置", MENU_URL, "/admin/wlbasic_sc.asp", "Setup wireless basic configuration", 0, 0},

#ifdef WLAN_MBSSID

	{"安全设置", MENU_URL, "/admin/wlwpa_mbssid_sc.asp", "Setup wireless security", 0, 0},

#else

	{"安全设置", MENU_URL, "/admin/wlwpa_sc.asp", "Setup wireless security", 0, 0},

#endif

	{"高级设置", MENU_URL, "/admin/wladvanced_sc.asp", "Setup wireless advanced configuration", 0, 0},

#ifdef WLAN_ACL

	{"接入控制", MENU_URL, "/admin/wlactrl_sc.asp", "Setup access control list for wireless clients", 0, 0},

#endif

#ifdef ZTE_GENERAL_ROUTER_SC

#ifdef WLAN_WDS

	{"WDS设置", MENU_URL, "/admin/wlwds_sc.asp", "WDS Settings", 0, 0},

#endif

#endif

#ifdef CONFIG_WIFI_SIMPLE_CONFIG
	{"WPS设置", MENU_URL, "/admin/wlwps_sc.asp", "WPS Settings", 0, 0},
#endif

	{0, 0, 0, 0, 0, 0}

};

#ifdef ZTE_GENERAL_ROUTER_SC

struct RootMenu childmenu_firewall_user[] = {



	{"IP/Port 过滤", MENU_URL, "fw-ipportfilter.asp", "no tip", 0, 0},

	{"MAC 过滤", MENU_URL, "noname.asp", "no tip", 0, 0},

	{"URL Blocking", MENU_URL, "noname.asp", "no tip", 0, 0},

	{0, 0, 0, 0, 0, 0}

};

#endif

#ifdef ZTE_GENERAL_ROUTER_SC

struct RootMenu childmenu_vmserver_user[] = {

	{"服务设置", MENU_URL, "vrtsrv_sc.asp", "服务设置", 0, 0},

	{"DMZ设置", MENU_URL, "fw-dmz_sc.asp", "DMZ设置", 0, 0},

	{0, 0, 0, 0, 0, 0}

};

#endif

#ifdef ZTE_GENERAL_ROUTER_SC

struct RootMenu childmenu_routersetting_user[] = {

	{"动态路由", MENU_URL, "noname.asp", "no tip", 0, 0},

	{"静态路由", MENU_URL, "noname.asp", "no tip", 0, 0},

	{0, 0, 0, 0, 0, 0}

};

#endif

#ifdef ZTE_GENERAL_ROUTER_SC

struct RootMenu childmenu_advothersetting_user[] = {

	{"IGMP代理", MENU_URL, "noname.asp", "no tip", 0, 0},

	{"UPNP设置", MENU_URL, "upnp_sc.asp", "UPnP configuration", 0, 0},

	{"桥参数配置", MENU_URL, "bridging_sc.asp", "configure bridge parameters", 0, 0},

	{"IP PassThrough", MENU_URL, "noname.asp", "no tip", 0, 0},

	{0, 0, 0, 0, 0, 0}

};

#endif

struct RootMenu childmenu_adv_user[] = {

#ifdef ZTE_GENERAL_ROUTER_SC

	{"DNS设置", MENU_URL, "noname.asp", "no tip", 0, 0},

	{"防火墙设置", MENU_FOLDER, &childmenu_firewall_user, "", sizeof(childmenu_firewall_user) / sizeof(struct RootMenu) - 1, 0},

	{"虚拟服务器", MENU_FOLDER, &childmenu_vmserver_user, "", sizeof(childmenu_vmserver_user) / sizeof(struct RootMenu) - 1, 0},

	{"路由设置", MENU_FOLDER, &childmenu_routersetting_user, "", sizeof(childmenu_routersetting_user) / sizeof(struct RootMenu) - 1, 0},

	{"IP QOS", MENU_URL, "noname.asp", "no tip", 0, 0},

	{"其他配置", MENU_FOLDER, &childmenu_advothersetting_user, "", sizeof(childmenu_advothersetting_user) / sizeof(struct RootMenu) - 1, 0},

#endif

#ifdef CONFIG_EXT_SWITCH

#ifdef ITF_GROUP

	{"端口绑定", MENU_URL, "/admin/eth2pvc_sc.asp", "Assign Ethernet to PVC", 0, 0},

#endif

#endif

	{0, 0, 0, 0, 0, 0}

};

struct RootMenu childmenu_admin_user[] = {

#ifdef IP_ACL

//              {"访问控制", MENU_URL, "/admin/acl_sc.asp", "ACL Setting"},

#endif
	{"用户密码管理", MENU_URL, "/admin/password_sc.asp", "Setup access password", 0, 0},
	#ifdef SYSLOG_SUPPORT
	{"系统日志", MENU_URL, "/admin/syslog_sc.asp", "Show system log", 0, 0},
	#endif
#ifdef CONFIG_SAVE_RESTORE
	{"配置备份恢复", MENU_URL, "/admin/saveconf_sc.asp", "Backup/restore current settings", 0, 0},
#endif
#ifdef WEB_UPGRADE
#ifdef UPGRADE_V1
	{"升级版本", MENU_URL, "/admin/upload_sc.asp", "Update firmware image", 0, 0},
#endif
#endif
#if defined( ZTE_GENERAL_ROUTER_SC) || defined(TIME_ZONE)

//              {"时区管理", MENU_URL,"date_sc.asp", "Setup Time Zone",0,0},

#endif


{"保存/重启", MENU_URL, "/admin/reboot_sc.asp", "save and reboot", 0, 0},


#ifdef ZTE_GENERAL_ROUTER_SC

//              {"SNMP配置", MENU_URL,"noname.asp", "no tip",0,0},

//              {"TR069配置", MENU_URL,"noname.asp", "no tip",0,0},

//              {"ACL配置", MENU_URL,"noname.asp", "no tip",0,0},

#endif

	{0, 0, 0, 0, 0, 0}

};

struct RootMenu childmenu_diag_user[] = {

	{"Ping", MENU_URL, "ping-ch.asp", "Ping test", 0, 0},

	{"ATM Loopback", MENU_URL, "oamloopback-ch.asp", "ATM OAM loopback test", 0, 0},

	{"ADSL", MENU_URL, "/adv/adsl-diag-ch.asp", "ADSL Tone Diagnostics", 0, 0},

#ifdef DIAGNOSTIC_TEST

	{"诊断测试", MENU_URL, "diag-test-ch.asp", "Diagnostic test", 0, 0},

#endif

	{0, 0, 0, 0, 0, 0}

};

struct RootMenu rootmenu_user[] = {

	{"系统状态", MENU_FOLDER, &childmenu_system_user, "", sizeof(childmenu_system_user) / sizeof(struct RootMenu) - 1, 0},

//              {"本地网络", MENU_FOLDER,&childmenu_lan_user,"",sizeof(childmenu_lan_user)/sizeof(struct RootMenu)-1,0},
#ifdef WLAN_SUPPORT
//              {"无线网络", MENU_FOLDER,&childmenu_wlan_user,"",sizeof(childmenu_wlan_user)/sizeof(struct RootMenu)-1,0},
#endif
//              {"广域网络", MENU_FOLDER,&childmenu_wan_user,"",sizeof(childmenu_wan_user)/sizeof(struct RootMenu)-1,0},

//              {"高级设定", MENU_FOLDER,&childmenu_adv_user,"",sizeof(childmenu_adv_user)/sizeof(struct RootMenu)-1,0},

	{"系统管理", MENU_FOLDER, &childmenu_admin_user, "", sizeof(childmenu_admin_user) / sizeof(struct RootMenu) - 1, 0},

//              {"系统诊断", MENU_FOLDER,&childmenu_diag_user,"",sizeof(childmenu_diag_user)/sizeof(struct RootMenu)-1,0},

	{0, 0, 0, 0, 0, 0}

};

// parm: 1 -> javascript support

//       0 -> no js

#endif				// of defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
//add by ramen for 531b router english version
#ifdef ZTE_GENERAL_ROUTER_EN

struct RootMenu childmenu_statistic_user[] = {
	{"Inteface Statistic", MENU_URL, "stats_en.asp", "Display current flow status", 0, 0},
	{"DSL Statisitc", MENU_URL, "/admin/adsl-statis_en.asp", "Display current adsl status", 0, 0},
	{0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_system_user[] = {
	{"System", MENU_URL, "/admin/status_en.asp", "Display current base status", 0, 0},
	{"LAN Network", MENU_URL, "lan_status_en.asp", "Display current lan status", 0, 0},
#ifdef WLAN_SUPPORT
	{"WLAN Network", MENU_URL, "wlan_status_en.asp", "Display current wlan status", 0, 0},
#endif
	{"WAN Network", MENU_URL, "wan_status_en.asp", "Display current wan status", 0, 0},
#ifdef CONFIG_EXT_SWITCH
#ifdef ITF_GROUP
#ifdef BPORTMAPPING_SUPPORT
	{"Port Mapping", MENU_URL, "status_eth2pvc_en.asp", "Display current portband status", 0, 0},
#endif
#endif

#endif

	{"Network Statistic", MENU_FOLDER, &childmenu_statistic_user, "", sizeof(childmenu_statistic_user) / sizeof(struct RootMenu) - 1, 0},

	{"ARP Info", MENU_URL, "arptable_en.asp", "Display current arp table status", 0, 0},

	{0, 0, 0, 0, 0, 0}

};
struct RootMenu childmenu_admin_user[] = {

#ifdef IP_ACL

//              {"访问控制", MENU_URL, "/admin/acl_sc.asp", "ACL Setting"},

#endif
	{"Password", MENU_URL, "/admin/password_en.asp", "Setup access password", 0, 0},
	#ifdef SYSLOG_SUPPORT
	{"System log", MENU_URL, "/admin/syslog_en.asp", "Show system log", 0, 0},
	#endif
	#ifdef CONFIG_SAVE_RESTORE
	{"Backup/Restore", MENU_URL, "/admin/saveconf_en.asp", "Backup/restore current settings", 0, 0},
	#endif
#ifdef WEB_UPGRADE
#ifdef UPGRADE_V1
	{"Upgrade Firmware", MENU_URL, "/admin/upload_en.asp", "Update firmware image", 0, 0},
#endif
#endif
	{"Commit/Reboot", MENU_URL, "/admin/reboot_en.asp", "save and reboot", 0, 0},

#if defined( ZTE_GENERAL_ROUTER_SC) || defined(TIME_ZONE)

//              {"时区管理", MENU_URL,"date_sc.asp", "Setup Time Zone",0,0},

#endif



#ifdef ZTE_GENERAL_ROUTER_SC

//              {"SNMP配置", MENU_URL,"noname.asp", "no tip",0,0},

//              {"TR069配置", MENU_URL,"noname.asp", "no tip",0,0},

//              {"ACL配置", MENU_URL,"noname.asp", "no tip",0,0},

#endif

	{0, 0, 0, 0, 0, 0}

};

struct RootMenu childmenu_diag_user[] = {

	{"Ping", MENU_URL, "ping-ch.asp", "Ping test", 0, 0},

	{"ATM Loopback", MENU_URL, "oamloopback-ch.asp", "ATM OAM loopback test", 0, 0},

	{"ADSL", MENU_URL, "/adv/adsl-diag-ch.asp", "ADSL Tone Diagnostics", 0, 0},

#ifdef DIAGNOSTIC_TEST

	{"诊断测试", MENU_URL, "diag-test-ch.asp", "Diagnostic test", 0, 0},

#endif

	{0, 0, 0, 0, 0, 0}

};

struct RootMenu rootmenu_user[] = {

	{"System", MENU_FOLDER, &childmenu_system_user, "", sizeof(childmenu_system_user) / sizeof(struct RootMenu) - 1, 0},

//              {"本地网络", MENU_FOLDER,&childmenu_lan_user,"",sizeof(childmenu_lan_user)/sizeof(struct RootMenu)-1,0},
#ifdef WLAN_SUPPORT
//              {"无线网络", MENU_FOLDER,&childmenu_wlan_user,"",sizeof(childmenu_wlan_user)/sizeof(struct RootMenu)-1,0},
#endif
//              {"广域网络", MENU_FOLDER,&childmenu_wan_user,"",sizeof(childmenu_wan_user)/sizeof(struct RootMenu)-1,0},

//              {"高级设定", MENU_FOLDER,&childmenu_adv_user,"",sizeof(childmenu_adv_user)/sizeof(struct RootMenu)-1,0},

	{"System managment", MENU_FOLDER, &childmenu_admin_user, "", sizeof(childmenu_admin_user) / sizeof(struct RootMenu) - 1, 0},

//              {"系统诊断", MENU_FOLDER,&childmenu_diag_user,"",sizeof(childmenu_diag_user)/sizeof(struct RootMenu)-1,0},

	{0, 0, 0, 0, 0, 0}

};

// parm: 1 -> javascript support

//       0 -> no js

#endif				// of defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)


#else				// of of WEB_MENU_USE_NEW



// parm: 1 -> javascript support

//       0 -> no js

int statusMenu_user(int eid, webs_t wp, int argc, char_t ** argv)
{

	char *parm;



	if (ejArgs(argc, argv, T("%s"), &parm) < 1)
	  {

		  websError(wp, 400, T("Insufficient args\n"));

		  return -1;

	  }



	if (parm[0] == '1')
	  {

		  websWrite(wp, T("menu.addItem(\"Status\", \"status.asp\", \"\", \"Display current status\");"));

	  }

	else
	  {

		  websWrite(wp, T("<tr><td><a href=\"status.asp\" target=\"view\">Status</a></td></tr>\n"));

	  }



	return 0;

}

int wlanMenu_user(int eid, webs_t wp, int argc, char_t ** argv)
{

#ifdef WLAN_SUPPORT

	char *parm;



	if (ejArgs(argc, argv, T("%s"), &parm) < 1)
	  {

		  websError(wp, 400, T("Insufficient args\n"));

		  return -1;

	  }


	//if (wlan_is_up()) {

	if (parm[0] == '1')
	  {

		  websWrite(wp, T("menu.addItem(\"Wireless\");" "wlan = new MTMenu();" "wlan.addItem(\"Basic Settings\", \"/admin/wlbasic.asp\", \"\", \"Setup wireless basic configuration\");" "wlan.addItem(\"Advanced Settings\", \"/admin/wladvanced.asp\", \"\", \"Setup wireless advanced configuration\");"
#ifdef WLAN_MBSSID
				  "wlan.addItem(\"Security\", \"/admin/wlwpa_mbssid.asp\", \"\", \"Setup wireless security\");"));

#else
				  "wlan.addItem(\"Security\", \"/admin/wlwpa.asp\", \"\", \"Setup wireless security\");"));

#endif



#ifdef WLAN_ACL

		  websWrite(wp, T("wlan.addItem(\"Access Control\", \"/admin/wlactrl.asp\", \"\", \"Setup access control list for wireless clients\");"));

#endif

#ifdef WLAN_WDS

		  websWrite(wp, T("wlan.addItem(\"WDS\", \"/admin/wlwds.asp\", \"\", \"WDS Settings\");"));

#endif

#ifdef WLAN_CLIENT

		  websWrite(wp, T("wlan.addItem(\"Site Survey\", \"/admin/wlsurvey.asp\", \"\", \"Wireless Site Survey\");"));

#endif



#ifdef CONFIG_WIFI_SIMPLE_CONFIG	//WPS def WLAN_CLIENT

		  websWrite(wp, T("wlan.addItem(\"WPS\", \"/admin/wlwps.asp\", \"\", \"Wireless Protected Setup\");"));

#endif



#ifdef WLAN_MBSSID

		  websWrite(wp, T("wlan.addItem(\"MBSSID\", \"/admin/wlmbssid.asp\", \"\", \"Wireless Multiple BSSID Setting\");"));

#endif



		  websWrite(wp, T("menu.makeLastSubmenu(wlan);"));

	  }

	else
	  {



		  websWrite(wp, T("<tr><td><b>Wireless</b></td></tr>\n" "<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlbasic.asp\" target=\"view\">Basic Settings</a></td></tr>\n" "<tr><td>&nbsp;&nbsp;<a href=\"/admin/wladvanced.asp\" target=\"view\">Advanced Settings</a></td></tr>"
#ifdef WLAN_MBSSID
				  "<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlwpa_mbssid.asp\" target=\"view\">Security</a></td></tr>\n"));

#else
				  "<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlwpa.asp\" target=\"view\">Security</a></td></tr>\n"));

#endif



#ifdef WLAN_ACL

		  websWrite(wp, T("<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlactrl.asp\" target=\"view\">Access Control</a></td></tr>\n"));

#endif

#ifdef WLAN_WDS

		  websWrite(wp, T("<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlwds.asp\" target=\"view\">WDS Settings</a></td></tr>\n"));

#endif

#ifdef WLAN_CLIENT

		  websWrite(wp, T("<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlsurvey.asp\" target=\"view\">Wireless Site Survey</a></td></tr>\n"));

#endif



#ifdef CONFIG_WIFI_SIMPLE_CONFIG	// WPS

		  websWrite(wp, T("<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlwps.asp\" target=\"view\">Wireless Protected Setup</a></td></tr>\n"));

#endif

#ifdef WLAN_MBSSID

		  websWrite(wp, T("<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlmbssid.asp\" target=\"view\">Wireless Multiple BSSID Setting</a></td></tr>\n"));

#endif

	  }

	//};

#endif

	return 0;

}



int wanMenu_user(int eid, webs_t wp, int argc, char_t ** argv)
{

	char *parm;



	if (ejArgs(argc, argv, T("%s"), &parm) < 1)
	  {

		  websError(wp, 400, T("Insufficient args\n"));

		  return -1;

	  }



	if (parm[0] == '1')
	  {

		  websWrite(wp, T("menu.addItem(\"WAN Interface\");" "wan = new MTMenu();" "wan.addItem(\"Channel Config\", \"wanadsl_user.asp\", \"\", \"Setup Channel Mode\");" "menu.makeLastSubmenu(wan);"));

	  }

	else
	  {

		  websWrite(wp, T("<tr><td><b>WAN Interface</b></td></tr>\n" "<tr><td>&nbsp;&nbsp;<a href=\"wanadsl_user.asp\" target=\"view\">Channel Config</a></td></tr>\n"));

	  }



	return 0;

}



int firewallMenu_user(int eid, webs_t wp, int argc, char_t ** argv)
{

	char *parm;



	if (ejArgs(argc, argv, T("%s"), &parm) < 1)
	  {

		  websError(wp, 400, T("Insufficient args\n"));

		  return -1;

	  }



	if (parm[0] == '1')
	  {

		  websWrite(wp, T("menu.addItem(\"Firewall\");" "service = new MTMenu();" "service.addItem(\"MAC Filtering\", \"fw-macfilter.asp\", \"\", \"Setup MAC filering\");" "menu.makeLastSubmenu(service);"));

	  }

	else
	  {

		  websWrite(wp, T("<tr><td><b>Firewall</b></td></tr>\n" "<tr><td>&nbsp;&nbsp;<a href=\"fw-macfilter.asp\" target=\"view\">MAC Filtering</a></td></tr>\n"));

	  }



	return 0;

}

int adminMenu_user(int eid, webs_t wp, int argc, char_t ** argv)
{

	char *parm;



	if (ejArgs(argc, argv, T("%s"), &parm) < 1)
	  {

		  websError(wp, 400, T("Insufficient args\n"));

		  return -1;

	  }



	if (parm[0] == '1')
	  {

		  websWrite(wp, T("menu.addItem(\"Admin\");" "admin = new MTMenu();" "admin.addItem(\"Commit/Reboot\", \"reboot.asp\", \"\", \"Commit/reboot the system\");" "admin.addItem(\"Password\", \"user-password.asp\", \"\", \"Setup access password\");" "admin.addItem(\"ACL Config\", \"acl.asp\", \"\", \"ACL Setting\");"
#ifdef ACCOUNT_LOGIN_CONTROL
				  "admin.addItem(\"Logout\", \"userlogout.asp\", \"\", \"Logout\");"
#endif
#ifdef USE_LOGINWEB_OF_SERVER
				  "admin.addItem(\"Logout\", \"/admin/logout.asp\", \"\", \"Logout\");"
#endif
				  "menu.makeLastSubmenu(admin);"));

	  }

	else
	  {

		  websWrite(wp, T("<tr><td><b>Admin</b></td></tr>\n" "<tr><td>&nbsp;&nbsp;<a href=\"reboot.asp\", target=\"view\">Commit/Reboot</a></td></tr>\n" "<tr><td>&nbsp;&nbsp;<a href=\"user-password.asp\" target=\"view\">Password</a></td></tr>\n" "<tr><td>&nbsp;&nbsp;<a href=\"acl.asp\" target=\"view\">ACL Config</a></td></tr>\n"
#ifdef ACCOUNT_LOGIN_CONTROL
				  "<tr><td>&nbsp;&nbsp;<a href=\"userlogout.asp\" target=\"view\">Logout</a></td></tr>\n"
#endif
#ifdef USE_LOGINWEB_OF_SERVER
				  "<tr><td>&nbsp;&nbsp;<a href=\"/admin/logout.asp\" target=\"view\">Logout</a></td></tr>\n"
#endif
			    ));

	  }

	return 0;

}



/*

int statisticsMenu_user(int eid, webs_t wp, int argc, char_t **argv)

{

	char *parm;

	

	if (ejArgs(argc, argv, T("%s"), &parm) < 1) {

		websError(wp, 400, T("Insufficient args\n"));

		return -1;

	}

	

	if ( parm[0] == '1' ) {

		websWrite(wp, T(

			"menu.addItem(\"Statistics\");"

			"statis = new MTMenu();"

			"statis.addItem(\"Interfaces\", \"stats.asp\", \"\", \"Display packet statistics\");"

			"statis.addItem(\"ADSL\", \"/adv/adsl-statis.asp\", \"\", \"Display ADSL statistics\");"

			"menu.makeLastSubmenu(statis);"

			));

	}

	else {

		websWrite(wp, T(

			"<tr><td><b>Statistics</b></td></tr>\n"

			"<tr><td>&nbsp;&nbsp;<a href=\"stats.asp\" target=\"view\">Interfaces</a></td></tr>\n"

			"<tr><td>&nbsp;&nbsp;<a href=\"/adv/adsl-statis.asp\" target=\"view\">ADSL</a></td></tr>\n"

			));

	}



	return 0;

}

*/

#endif				// of WEB_MENU_USE_NEW



int createMenu_user(int eid, webs_t wp, int argc, char_t ** argv)
{

#ifdef WEB_MENU_USE_NEW

	int i = 0, totalIdNums = 0, maxchildrensize = 0;

	int IdIndex = 0;

	unsigned char isRootMenuEnd = 0;



	//calc the id nums and the max children size

	totalIdNums = calcFolderNum(rootmenu_user, &maxchildrensize);

	//product the js code

	addMenuJavaScript(wp, totalIdNums, maxchildrensize);

	//create the header
/* add by yq_zhou 09.2.02 add sagem logo for 11n*/
#ifdef CONFIG_11N_SAGEM_WEB
  websWrite (wp, T ("<body  onload=\"initIt()\" bgcolor=\"#FFFFFF\" >\n"));
#else
  websWrite (wp, T ("<body  onload=\"initIt()\" bgcolor=\"#000000\" >\n"));
#endif
	websWrite(wp, T("<table width=100%% border=0 cellpadding=0 cellspacing=0>\n<tr><td  width=100%% align=left>\n"));

	websWrite(wp, T("<table  border=0 cellpadding=0 cellspacing=0>\n" "<tr><td width=18 height=18><img src=menu-images/menu_root.gif width=18 height=18></td>\n" "<td  height=18 colspan=4 class=link><font size=3>%s</font></td></tr>\n </table>\n"), menuTitle_user);



	if (rootmenu_user[1].u.addr)

		addMenu(wp, &rootmenu_user[0], 0, &IdIndex, 0);

	else

		addMenu(wp, &rootmenu_user[0], 0, &IdIndex, 1);



	websWrite(wp, T("</td></tr>\n</table>\n"));

	/*zteStatusMenu(eid, wp, argc, argv);

	   zteQuickSetMenu(eid, wp, argc, argv);

	   zteLanMenu(eid, wp, argc, argv);

	   zteWlanMenu(eid, wp, argc, argv);

	   zteWanMenu(eid, wp, argc, argv);

	   zteAdvanceMenu(eid, wp, argc, argv);

	   zteAdminMenu(eid, wp, argc, argv);

	   zteDiagMenu(eid, wp, argc, argv); */



#else

	statusMenu_user(eid, wp, argc, argv);

//      lanMenu_user(eid, wp, argc, argv);

	wlanMenu_user(eid, wp, argc, argv);

	wanMenu_user(eid, wp, argc, argv);

	firewallMenu_user(eid, wp, argc, argv);

//      srvMenu_user(eid, wp, argc, argv);

//      advanceMenu_user(eid, wp, argc, argv);

//      diagMenu_user(eid, wp, argc, argv);

	adminMenu_user(eid, wp, argc, argv);

//      statisticsMenu_user(eid, wp, argc, argv);

#endif

	return 0;

}
