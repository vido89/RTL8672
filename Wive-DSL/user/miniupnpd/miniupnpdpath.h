/* $Id: miniupnpdpath.h,v 1.6 2008/05/23 09:06:59 adsmt Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006-2008 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef __MINIUPNPDPATH_H__
#define __MINIUPNPDPATH_H__

#include "config.h"

/* Paths and other URLs in the miniupnpd http server */
// DEVICEDESC_PATH
#ifdef TR_064
#define ROOTDESC_PATH "/devicedesc.xml"

#define DEVICEINFO_PATH "/DeviceInfo.xml"
#define DEVICEINFO_CONTROLURL	"/ctl/DevInfo"
#define DEVICEINFO_EVENTURL		"/evt/DevInfo"

#define DEVICECONFIG_PATH "/DeviceConfig.xml"
#define DEVICECONFIG_CONTROLURL	"/ctl/DevCfg"
#define DEVICECONFIG_EVENTURL		"/evt/DevCfg"

#define LANCONFIGSECURITY_PATH	"/LANConfigSecurity.xml"
#define LANCONFIGSECURITY_CONTROLURL	"/ctl/LANCfgSec"
#define LANCONFIGSECURITY_EVENTURL	"/evt/LANCfgSec"

#define MANAGEMENTSERVER_PATH "/ManagementServer.xml"
#define MANAGEMENTSERVER_CONTROLURL	"/ctl/MgSer"
#define MANAGEMENTSERVER_EVENTURL	"/evt/MgSer"

#define TIME_PATH	"/Time.xml"
#define TIME_CONTROLURL	"/ctl/Time"
#define TIME_EVENTURL	"/evt/Time"

#define USERINTERFACE_PATH	"/UserInterface.xml"
#define USERINTERFACE_CONTROLURL	"/ctl/UserInf"
#define USERINTERFACE_EVENTURL	"/evt/UserInf"

#define WANDSLCONNECTIONMANAGEMENT_PATH "/WANDSLConnectionManagement.xml"
#define WANDSLCONNECTIONMANAGEMENT_CONTROLURL "/ctl/WANDLCnMg"
#define WANDSLCONNECTIONMANAGEMENT_EVENTURL "/evt/WANDLCnMg"

#define WANDSLINTERFACECONFIG_PATH "/WANDSLInterfaceConfig.xml"
#define WANDSLINTERFACECONFIG_CONTROLURL "/ctl/WANDLInCfg"
#define WANDSLINTERFACECONFIG_EVENTURL "/evt/WANDLInCfg"

#define WANETHERNETINTERFACECONFIG_PATH "/WANEthernetInterfaceConfig.xml"
#define WANETHERNETINTERFACECONFIG_CONTROLURL "/ctl/WANEhInCfg"
#define WANETHERNETINTERFACECONFIG_EVENTURL "/evt/WANEhInCfg"

#define WANPPPC_PATH			"/WANPPPCn.xml"
#define WANPPPC_CONTROLURL	"/ctl/PPPConn"
#define WANPPPC_EVENTURL		"/evt/PPPConn"

#define HOSTS_PATH			"/Hosts.xml"
#define HOSTS_CONTROLURL	"/ctl/Hosts"
#define HOSTS_EVENTURL		"/evt/Hosts"

#define WLANCONFIGURATION_PATH			"/WLANConfiguration.xml"
#define WLANCONFIGURATION_CONTROLURL	"/ctl/WLANCfg"
#define WLANCONFIGURATION_EVENTURL		"/evt/WLANCfg"

#define LANETHERNETINTERFACECONFIG_PATH			"/LANEthernetInterfaceConfig.xml"
#define LANETHERNETINTERFACECONFIG_CONTROLURL	"/ctl/LANEhInCfg"
#define LANETHERNETINTERFACECONFIG_EVENTURL		"/evt/LANEhInCfg"

#define LANHOSTCONFIGMANAGEMENT_PATH "/LANHostConfigManagement.xml"
#define LANHOSTCONFIGMANAGEMENT_CONTROLURL	"/ctl/LANHtCfgMg"
#define LANHOSTCONFIGMANAGEMENT_EVENTURL		"/evt/LANHtCfgMg"
#else  // TR_064 else
#define ROOTDESC_PATH 		"/rootDesc.xml"
#endif  // TR_064 End

#ifdef CLOSE_SSDP
#ifdef TR_064
#define ROOT_DESC_NAME "devicedesc"
#else
#define ROOT_DESC_NAME "rootDesc"
#endif
#endif

#ifdef HAS_DUMMY_SERVICE
#define DUMMY_PATH			"/dummy.xml"
#endif

#if defined(ENABLE_L3F_SERVICE) || defined(TR_064)
#define L3F_PATH			"/L3Fwd.xml"
#define L3F_CONTROLURL		"/ctl/L3F"
#define L3F_EVENTURL		"/evt/L3F"
#endif

#define WANCFG_PATH			"/WanCommonIfc1.xml"
#define WANCFG_CONTROLURL	"/ctl/CmnIfCfg"
#define WANCFG_EVENTURL		"/evt/CmnIfCfg"

#define WANDSLLINKCONFIG_PATH "/WanDslLink.xml"
#define WANDSLLINKCONFIG_CONTROLURL "/ctl/WanDslLk"
#define WANDSLLINKCONFIG_EVENTURL "/evt/WanDslLk"

#define WANIPC_PATH			"/WanIpConn.xml"
#define WANIPC_CONTROLURL	"/ctl/IPConn"
#define WANIPC_EVENTURL		"/evt/IPConn"
#endif

