/*-----------------------------------------------------------------
 * File: boamain.c
 *-----------------------------------------------------------------*/
 
#include "webform.h"
#include "mib.h"
#include "../defs.h"
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
//xl_yue
#include "utility.h"

/*+++++add by Jack for VoIP project 20/03/07+++++*/
#ifdef VOIP_SUPPORT
#include "web_voip.h"
#endif /*VOIP_SUPPORT*/
/*-----end-----*/

void rtl8670_AspInit() {
   /*
 *	ASP script procedures and form functions.
 */
	websAspDefine(T("getInfo"), getInfo);
	// Kaohj
	websAspDefine(T("checkWrite"), checkWrite);
	//websAspDefine(T("getIndex"), getIndex);
	websAspDefine(T("initPage"), initPage);
#ifdef WLAN_QoS
	websAspDefine(T("ShowWmm"), ShowWmm);
#endif
	   // add by yq_zhou 1.20
	websAspDefine(T("write_wladvanced"), write_wladvanced);

/* add by yq_zhou 09.2.02 add sagem logo for 11n*/
/*#ifdef CONFIG_11N_SAGEM_WEB
	websAspDefine(T("writeTitle"), write_title);
	websAspDefine(T("writeLogoBelow"), write_logo_below);
#endif*/
	websAspDefine(T("portFwList"), portFwList);
#ifdef NATIP_FORWARDING
	websAspDefine(T("ipFwList"), ipFwList);
#endif
	websAspDefine(T("ipPortFilterList"), ipPortFilterList);
#ifdef MAC_FILTER
	websAspDefine(T("macFilterList"), macFilterList);
#endif

	websAspDefine(T("atmVcList"), atmVcList);	
	websAspDefine(T("atmVcList2"), atmVcList2);	
	websAspDefine(T("wanConfList"), wanConfList);	
	websAspDefine(T("getNameServer"), getNameServer);	
	websAspDefine(T("getDefaultGW"), getDefaultGW);

	websFormDefine(T("formTcpipLanSetup"), formTcpipLanSetup);	// TCP/IP Lan Setting Form
	websAspDefine(T("lan_setting"), lan_setting);	
	websAspDefine(T("checkIP2"), checkIP2);	
	websAspDefine(T("lanScript"), lan_script);	
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	websFormDefine(T("formRefresh"), formRefresh);
#endif

#ifdef WEB_REDIRECT_BY_MAC
	websFormDefine(T("formLanding"), formLanding);
#endif

#ifdef _CWMP_MIB_
	websFormDefine(T("formTR069Config"), formTR069Config);
	websFormDefine(T("formTR069CPECert"), formTR069CPECert);
	websFormDefine(T("formTR069CACert"), formTR069CACert);

	websAspDefine(T("TR069ConPageShow"), TR069ConPageShow);
#endif
	websAspDefine(T("portFwTR069"), portFwTR069);
#ifdef WEB_MENU_USE_MTM
	websAspDefine(T("srvMenu"), srvMenu);
#endif
	websFormDefine(T("formPortFw"), formPortFw);					// Firewall Port forwarding Setting Form
#ifdef NATIP_FORWARDING
	websFormDefine(T("formIPFw"), formIPFw);					// Firewall NAT IP forwarding Setting Form
#endif
#ifdef PORT_TRIGGERING
	websFormDefine(T("formGaming"), formGaming);					// Firewall Port Triggering Setting Form
#endif
	websFormDefine(T("formFilter"), formFilter);					// Firewall IP, Port, Mac Filter Setting Form
#ifdef LAYER7_FILTER_SUPPORT
	websFormDefine(T("formlayer7"), formLayer7);                            //star: for layer7 filter
	websAspDefine(T("AppFilterList"), AppFilterList); 
#endif
#ifdef PARENTAL_CTRL
	websFormDefine(T("formParentCtrl"), formParentCtrl);                            
	websAspDefine(T("parentControlList"), parentalCtrlList); 
#endif
#ifdef ZTE_GENERAL_ROUTER_SC
	websFormDefine(T("formVrtsrv"), formVrtsrv);
	websFormDefine(T("formAddVrtsrv"), formAddVrtsrv);
#endif
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	websFormDefine(T("formFastConf"), formFastConf);
	websFormDefine(T("formFastConf2"), formFastConf2);
	websFormDefine(T("formFastConf3"), formFastConf3);
	websFormDefine(T("formFastConf4"), formFastConf4);
	websFormDefine(T("formFcPppWan"), formFcPppWan);
	websFormDefine(T("formFcPppType"), formFcPppType);
	websFormDefine(T("formFcMerWan"), formFcMerWan);
	websFormDefine(T("formFcIPoA"), formFcIPoA);
	//websFormDefine(T("formFastConfEnd"), formFastConfEnd);
	//websAspDefine(T("showFastConfP1"), showFastConfP1);
#endif
	websFormDefine(T("formDMZ"), formDMZ);						// Firewall DMZ Setting Form

	websFormDefine(T("formPasswordSetup"), formPasswordSetup);	// Management Password Setting Form
	websFormDefine(T("formUserPasswordSetup"), formUserPasswordSetup);	// Management User Password Setting Form
#ifdef ACCOUNT_CONFIG
	websFormDefine(T("formAccountConfig"), formAccountConfig);	// Management Account Configuration Setting Form
	websAspDefine(T("accountList"), accountList);	
#endif
#ifdef WEB_UPGRADE
	websFormDefine(T("formUpload"), formUpload);					// Management Upload Firmware Setting Form
#endif
#if 1
	websFormDefine(T("formSaveConfig"), formSaveConfig);			// Management Upload/Download Configuration Setting Form
#endif
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
	websFormDefine(T("formSnmpConfig"), formSnmpConfig);			// SNMP Configuration Setting Form
#endif
#if 0 // DSL driver page
	websFormDefine(T("formAdslDrv"), formAdslDrv);				// ADSL Driver Setting Form
#endif
	websFormDefine(T("formSetAdsl"), formSetAdsl);				// ADSL Driver Setting Form
	websFormDefine(T("formStatAdsl"), formStatAdsl);			// ADSL Statistics Form
	websFormDefine(T("formDiagAdsl"), formDiagAdsl);			// ADSL Driver Diag Form
	websFormDefine(T("formStats"), formStats);				// Packet Statistics Form
	websFormDefine(T("formRconfig"), formRconfig);				// Remote Configuration Form
	websFormDefine(T("formSysCmd"),  formSysCmd);				// hidden page for system command
	websAspDefine(T("sysCmdLog"), sysCmdLog);
#ifdef SYSLOG_SUPPORT
	websFormDefine(T("formSysLog"),  formSysLog);				// hidden page for system command
	websAspDefine(T("sysLogList"), sysLogList);			// Web Log
	websAspDefine(T("RemoteSyslog"), RemoteSyslog); // Jenny, for remote system log
#endif
	websFormDefine(T("formSetAdslTone"), formSetAdslTone);		// ADSL Driver Setting Tone Form
#if 0
	websAspDefine(T("adslDrvSnrTblGraph"), adslDrvSnrTblGraph);
	websAspDefine(T("adslDrvSnrTblList"), adslDrvSnrTblList);
	websAspDefine(T("adslDrvBitloadTblGraph"), adslDrvBitloadTblGraph);
	websAspDefine(T("adslDrvBitloadTblList"), adslDrvBitloadTblList);
#endif
	websAspDefine(T("adslToneDiagTbl"), adslToneDiagTbl);
	websAspDefine(T("adslToneDiagList"), adslToneDiagList);
	websAspDefine(T("adslToneConfDiagList"), adslToneConfDiagList);
	websAspDefine(T("memDump"), memDump);	
	websAspDefine(T("pktStatsList"), pktStatsList);	

	websFormDefine(T("formStatus"), formStatus);			// Status Form
	websFormDefine(T("formStatusModify"), formDate);
#ifdef ZTE_GENERAL_ROUTER_SC
	websFormDefine(T("formTimezone"), formTimezone);
#endif

	websFormDefine(T("formWanAtm"), formAtmVc);			// Atm VC Configuration Setting Form
	websFormDefine(T("formWanAdsl"), formAdsl);			// ADSL Configuration Setting Form
	websFormDefine(T("formPPPEdit"), formPPPEdit);			// PPP interface Configuration Form
	websFormDefine(T("formIPEdit"), formIPEdit);			// IP interface Configuration Form
	websFormDefine(T("formBrEdit"), formBrEdit);			// Bridged interface Configuration Form
	
	websFormDefine(T("formBridging"), formBridge);			// Bridge Configuration Form
	websAspDefine(T("bridgeFdbList"), bridgeFdbList);
#ifdef ZTE_GENERAL_ROUTER_SC
	websAspDefine(T("virtualSvrList"), virtualSvrList);
#endif
	websAspDefine(T("ARPTableList"), ARPTableList);
	websFormDefine(T("formRefleshFdbTbl"), formRefleshFdbTbl);

	websFormDefine(T("formRouting"), formRoute);			// Routing Configuration Form
	websAspDefine(T("showStaticRoute"), showStaticRoute);	
	websAspDefine(T("ShowDefaultGateway"), ShowDefaultGateway);	// Jenny, for DEFAULT_GATEWAY_V2	
	websAspDefine(T("ShowRouteInf"), ShowRouteInf);
	websAspDefine(T("GetDefaultGateway"), GetDefaultGateway);
	websFormDefine(T("formRefleshRouteTbl"), formRefleshRouteTbl);
	websAspDefine(T("routeList"), routeList);	

	websFormDefine(T("formDhcpServer"), formDhcpd);	// DHCP Server Setting Form
	websAspDefine(T("dhcpClientList"), dhcpClientList);
	websFormDefine(T("formReflashClientTbl"), formReflashClientTbl);
	
	websFormDefine(T("formDNS"), formDns);			// DNS Configuration Form
#ifdef CONFIG_USER_DDNS	
	websFormDefine(T("formDDNS"), formDDNS);
	websAspDefine(T("showDNSTable"), showDNSTable);
#endif	
	websFormDefine(T("formDhcrelay"), formDhcrelay);	// DHCPRelay Configuration Form
	websFormDefine(T("formPing"), formPing);		// Ping diagnostic Form
	websFormDefine(T("formOAMLB"), formOamLb);		// OAM Loopback diagnostic Form
#ifdef ADDRESS_MAPPING	
	websFormDefine(T("formAddressMap"), formAddressMap);	// AddressMap Configuration Form
#ifdef MULTI_ADDRESS_MAPPING
	websAspDefine(T("AddressMapList"), showMultAddrMappingTable);
#endif //MULTI_ADDRESS_MAPPING
#endif
	
#ifdef FINISH_MAINTENANCE_SUPPORT
	websFormDefine(T("formFinishMaintenance"), formFinishMaintenance);		// xl_yue added,inform itms
#endif
#ifdef ACCOUNT_LOGIN_CONTROL
	websFormDefine(T("formAdminLogout"), formAdminLogout);		// xl_yue added,
	websFormDefine(T("formUserLogout"), formUserLogout);		// xl_yue added,
#endif
//added by xl_yue
#ifdef USE_LOGINWEB_OF_SERVER
	websFormDefine(T("formLogin"), formLogin);		// xl_yue added,
	websFormDefine(T("formLogout"), formLogout);		// xl_yue added,
	// Kaohj
	websAspDefine(T("passwd2xmit"), passwd2xmit);
#endif

	websFormDefine(T("formReboot"), formReboot);		// Commit/reboot Form
	websFormDefine(T("formDhcpMode"), formDhcpMode);	// DHCP Mode Configuration Form
	
#ifdef CONFIG_USER_IGMPPROXY
	websFormDefine(T("formIgmproxy"), formIgmproxy);	// IGMP Configuration Form
#endif
	websAspDefine(T("if_wan_list"), ifwanList);
//#ifdef CONFIG_USER_UPNPD
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
	websFormDefine(T("formUpnp"), formUpnp);		// UPNP Configuration Form	
#endif
#if defined(CONFIG_USER_ROUTED_ROUTED) || defined(CONFIG_USER_ZEBRA_OSPFD_OSPFD)
	websFormDefine(T("formRip"), formRip);			// RIP Configuration Form
#endif
#ifdef CONFIG_USER_ROUTED_ROUTED
	websAspDefine(T("showRipIf"), showRipIf);
#endif
#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
	websAspDefine(T("showOspfIf"), showOspfIf);
#endif
	
#ifdef IP_ACL
	websFormDefine(T("formACL"), formACL);                  // ACL Configuration Form
	websAspDefine(T("showACLTable"), showACLTable);
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	websAspDefine(T("showACLIpAddr"), showACLIpAddr);
#endif
#endif
#ifdef MAC_ACL
	websAspDefine(T("showACLMacTable"), showACLMacTable);
#endif
#ifdef NAT_CONN_LIMIT
	websFormDefine(T("formConnlimit"), formConnlimit);
	websAspDefine(T("showConnLimitTable"), showConnLimitTable);
#endif
#ifdef TCP_UDP_CONN_LIMIT
	websFormDefine(T("formConnlimit"), formConnlimit);
	websAspDefine(T("connlmitList"), showConnLimitTable);

#endif //TCP_UDP_CONN_LIMIT
	websFormDefine(T("formmacBase"), formmacBase); 		// MAC-Base Assignment for DHCP Server
	websAspDefine(T("showMACBaseTable"), showMACBaseTable);

#ifdef URL_BLOCKING_SUPPORT	
	websFormDefine(T("formURL"), formURL);                  // URL Configuration Form
	websAspDefine(T("showURLTable"), showURLTable);
	websAspDefine(T("showKeywdTable"), showKeywdTable);
#endif
#ifdef URL_ALLOWING_SUPPORT
       websAspDefine(T("showURLALLOWTable"), showURLALLOWTable);
#endif 



#ifdef DOMAIN_BLOCKING_SUPPORT
	websFormDefine(T("formDOMAINBLK"), formDOMAINBLK);                  // Domain Blocking Configuration Form
	websAspDefine(T("showDOMAINBLKTable"), showDOMAINBLKTable);
#endif

#ifdef TIME_ZONE
	websFormDefine(T("formNtp"), formNtp);
#endif
	
	websFormDefine(T("formOthers"), formOthers);	// Others advance Configuration Form
#ifdef WEB_MENU_USE_MTM
	websAspDefine(T("diagMenu"), diagMenu);	//xl_yue
	websAspDefine(T("adminMenu"), adminMenu);	//xl_yue
//xl_yue
#if !defined(ZTE_531B_BRIDGE_SC) && !defined(ZTE_GENERAL_ROUTER_SC)	
	websAspDefine(T("userAddAdminMenu"), userAddAdminMenu);
#endif
#endif

#ifdef AUTO_PROVISIONING
	websFormDefine(T("formAutoProvision"), formAutoProvision);	// Auto-Rovisioning Configuration Form
#endif
	
//xl_yue
#if !defined(ZTE_531B_BRIDGE_SC) && !defined(ZTE_GENERAL_ROUTER_SC)
	websAspDefine(T("vportMenu"), vportMenu);
#endif
#ifdef CONFIG_RTL867X_COMBO_PORTMAPPING
	websFormDefine(T("formItfGroup"), formItfGroup);	// Interface grouping Form
	websAspDefine(T("itfGrpList"), ifGroupList);	
	//websAspDefine(T("eth2pvcStatus"), eth2pvcStatus);
	//websAspDefine(T("showItfGrpList"), showIfGroupList);

#endif
#ifdef CONFIG_EXT_SWITCH
	//websFormDefine(T("formMpMode"), formMpMode);	// Eth-to-pvc mapping Form
	//websFormDefine(T("formEth2Pvc"), formEth2pvc);	// Eth-to-pvc mapping Form
	//websAspDefine(T("eth2pvcPost"), eth2pvcPost);
#ifdef ITF_GROUP
	websFormDefine(T("formItfGroup"), formItfGroup);	// Interface grouping Form
	websAspDefine(T("itfGrpList"), ifGroupList);	
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	websAspDefine(T("eth2pvcStatus"), eth2pvcStatus);
	websAspDefine(T("showItfGrpList"), showIfGroupList);
#endif
#endif
#ifdef VLAN_GROUP
	websFormDefine(T("formVlanGroup"), formVlanGroup);	// Vlan grouping Form
	websAspDefine(T("itfGrpList"), ifGroupList);
#endif
	//websFormDefine(T("formVlanCfg"), formVlan);	// Vlan Configuration Form
	//websAspDefine(T("vlanPost"), vlanPost);
#ifdef ELAN_LINK_MODE
	websFormDefine(T("formLinkMode"), formLink);	// Ethernet Link Form
	websAspDefine(T("show_lan"), show_lanport);
#endif
#else // of CONFIG_EXT_SWITCH 
#ifdef ELAN_LINK_MODE_INTRENAL_PHY
	websFormDefine(T("formLinkMode"), formLink);	
#endif
#endif	// of CONFIG_EXT_SWITCH
#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
	websFormDefine(T("formIPQoS"), formQos);	// IP QoS Form
	websAspDefine(T("dft_qos"), default_qos);
	websAspDefine(T("ipQosList"), qosList);
#ifdef QOS_DIFFSERV
	websFormDefine(T("formDiffServ"), formDiffServ);
	websAspDefine(T("diffservList"), diffservList);
#endif
	websAspDefine(T("pq_egress"), priority_outif);
	websAspDefine(T("mark_dscp"), confDscp);
#ifdef QOS_DSCP_MATCH
	websAspDefine(T("match_dscp"), match_dscp);
#endif
#endif
#ifdef NEW_IP_QOS_SUPPORT
	websFormDefine(T("formQosShape"), formQosShape);
	websAspDefine(T("initTraffictlPage"),initTraffictlPage);
#endif
#ifdef CONFIG_8021P_PRIO
       websAspDefine(T("setting1p"), setting_1ppriority);
#ifdef NEW_IP_QOS_SUPPORT
       websAspDefine(T("settingpred"), setting_predprio);
#endif
#endif
	websAspDefine(T("if_lan_list"), iflanList);
	websAspDefine(T("pr_egress"), policy_route_outif);

	websFormDefine(T("formSAC"), formAcc);	// Services Access Control
	websAspDefine(T("accPost"), accPost);
	websAspDefine(T("rmtaccItem"), accItem);
	//websAspDefine(T("autopvcStatus"), autopvcStatus);	// auto-pvc search	
	//websAspDefine(T("showPVCList"), showPVCList);	// auto-pvc search		
	websAspDefine(T("ShowAutoPVC"), ShowAutoPVC);	// auto-pvc search				
	websAspDefine(T("ShowApplicationMode"), ShowApplicationMode);	// China Telecom jim...	
	websAspDefine(T("ShowChannelMode"), ShowChannelMode);	// China Telecom jim...		
	websAspDefine(T("ShowPPPIPSettings"), ShowPPPIPSettings);	// China Telecom jim...	
	websAspDefine(T("ShowNAPTSetting"), ShowNAPTSetting);	// China Telecom jim...	
#ifdef CONFIG_GUI_WEB	// Jenny
	websAspDefine(T("fwGuiMenu"), fwGuiMenu);
	websAspDefine(T("dnsGuiMenu"), dnsGuiMenu);
	websAspDefine(T("aclGuiMenu"), aclGuiMenu);
	websAspDefine(T("diagGuiMenu"), diagGuiMenu);
	websAspDefine(T("adminGuiMenu"), adminGuiMenu);
	websAspDefine(T("vportGuiMenu"), vportGuiMenu);
	websAspDefine(T("wlanGuiMenu"), wlanGuiMenu);
	websFormDefine(T("formPPPAuth"), formPPPAuth);
	websAspDefine(T("getPPPInfo"), getPPPInfo);
	websAspDefine(T("checkPPPStatus"), checkPPPStatus);
#else
//xl_yue
	websAspDefine(T("createMenu"), createMenu);
	websAspDefine(T("createMenu_user"), createMenu_user);
#endif
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	websAspDefine(T("ZTESoftwareVersion"), ZTESoftwareVersion);
#endif
#ifdef WLAN_SUPPORT
	//for WLAN enable/disable web control
#ifdef WEB_MENU_USE_MTM
	websAspDefine(T("wlanMenu"), wlanMenu);	//xl_yue
#endif
	websAspDefine(T("wlanStatus"), wlanStatus);
	websFormDefine(T("formWlanSetup"), formWlanSetup);
	websFormDefine(T("formWep"), formWep);
#ifdef WLAN_ACL
	websAspDefine(T("wlAcList"), wlAcList);
    websAspDefine(T("wlShowAcList"),wlShowAcList); 
#endif
	websAspDefine(T("wirelessClientList"), wirelessClientList);
	websFormDefine(T("formWirelessTbl"), formWirelessTbl);

#ifdef WLAN_ACL
	websFormDefine(T("formWlAc"), formWlAc);
#endif
	websFormDefine(T("formAdvanceSetup"), formAdvanceSetup);

#ifdef WLAN_WPA
	websFormDefine(T("formWlEncrypt"), formWlEncrypt);
#endif
#ifdef WLAN_WDS
	websFormDefine(T("formWlWds"), formWlWds);
	websAspDefine(T("wlWdsList"),wlWdsList);
#endif
#ifdef WLAN_CLIENT
	websFormDefine(T("formWlSiteSurvey"), formWlSiteSurvey);
	websAspDefine(T("wlSiteSurveyTbl"),wlSiteSurveyTbl);
#endif

#ifdef CONFIG_WIFI_SIMPLE_CONFIG //WPS
	websFormDefine(T("formWsc"), formWsc);
#endif
#endif	// of WLAN_SUPPORT

	websAspDefine(T("oamSelectList"), oamSelectList);
#ifdef DIAGNOSTIC_TEST
	websFormDefine(T("formDiagTest"), formDiagTest);	// Diagnostic test
	websAspDefine(T("lanTest"), lanTest);	// Ethernet LAN connection test
	websAspDefine(T("adslTest"), adslTest);	// ADSL service provider connection test
	websAspDefine(T("internetTest"), internetTest);	// Internet service provider connection test
#endif
#ifdef DOS_SUPPORT
	websFormDefine(T("formDosCfg"), formDosCfg);
#endif
#ifdef WLAN_MBSSID
	websAspDefine(T("wlmbssid_asp"), wlmbssid_asp);	
	websAspDefine(T("postSSID"), postSSID);
	websAspDefine(T("postSSIDWEP"), postSSIDWEP);
	websFormDefine(T("formWlanMBSSID"), formWlanMBSSID);
	websAspDefine(T("checkSSID"), checkSSID);
	websAspDefine(T("SSIDStr"), SSIDStr);	
#endif
#ifdef WLAN_SUPPORT
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
//xl_yue: translocate mbssid configuration page to basic setting page
	websAspDefine(T("LinkMBSSIDPage"), LinkMBSSIDPage);
#endif
#endif	

#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
// star: for dhcp server
	websAspDefine(T("show_IP_range"), show_IP_range);
#endif

#ifdef PORT_FORWARD_ADVANCE
	websFormDefine(T("formPFWAdvance"), formPFWAdvance);	
	websAspDefine(T("showPFWAdvTable"), showPFWAdvTable);	
#endif
	websAspDefine(T("showPFWAdvForm"), showPFWAdvForm);

/*
 *	Create the Form handlers for the User Management pages
 */
#ifdef USER_MANAGEMENT_SUPPORT
	formDefineUserMgmt();
#endif
//add by ramen for ALG on-off
#ifdef CONFIG_IP_NF_ALG_ONOFF
websFormDefine(T("formALGOnOff"), formALGOnOff);
#endif
/*+++++add by Jack for VoIP project 20/03/07+++++*/
#ifdef VOIP_SUPPORT
	web_voip_init();
#endif /*VOIP_SUPPORT*/
/*-----end-----*/
#ifdef WEB_ENABLE_PPP_DEBUG
websAspDefine(T("ShowPPPSyslog"), ShowPPPSyslog); // Jenny, show PPP debug to syslog
#endif
	websAspDefine(T("DSLuptime"), DSLuptime);

}


#ifdef AUTO_PVC_SEARCH_AUTOHUNT
void register_pvc(int signum)
{
	FILE *fp;
	unsigned int vpi, vci, entryNum, i, idx;
	MIB_CE_ATM_VC_T Entry;	
	
	printf("signal (%x) handler register_pvc got called\n", signum);	
	if (fp=fopen("/proc/AUTO_PVC_SEARCH", "r") ) {
		fscanf( fp, "%d,%d", &vpi, &vci);
		printf("register_pvc (%d, %d)\n", vpi, vci);		
		fclose(fp);
		
		autoHunt_found = 1;  // Timer for auto search PVC		
		setVC0Connection(vpi, vci);
		succ_AutoSearchPVC();
	} else {
		printf("Open /proc/AUTO_PVC_SEARCH failed\n");
	}
}
#endif // of AUTO_PVC_SEARCH_AUTOHUNT


void initLanPage(webs_t wp)
{
	unsigned char vChar;
#ifdef SECONDARY_IP
	char dhcp_pool;
#endif
	
#ifdef SECONDARY_IP
	mib_get( MIB_ADSL_LAN_ENABLE_IP2, (void *)&vChar);
	if (vChar!=0) {
		//websWrite(wp, "%s.tcpip.enable_ip2.value = 1;\n", DOCUMENT);
		websWrite(wp, "%s.tcpip.enable_ip2.checked = true;\n", DOCUMENT);
	}
	#ifndef DHCPS_POOL_COMPLETE_IP
	mib_get(MIB_ADSL_LAN_DHCP_POOLUSE, (void *)&dhcp_pool);
	websWrite(wp, "%s.tcpip.dhcpuse[%d].checked = true;\n", DOCUMENT, dhcp_pool);
	#endif
	websWrite(wp, "updateInput();\n");
#endif

#ifdef CONFIG_EXT_SWITCH
	mib_get( MIB_MPMODE, (void *)&vChar);
	// bitmap for virtual lan port function
	// Port Mapping: bit-0
	// QoS : bit-1
	// IGMP snooping: bit-2
	websWrite(wp, "%s.tcpip.snoop[%d].checked = true;\n", DOCUMENT, (vChar>>2)&0x01);
#ifdef CONFIG_IGMP_FORBID
        mib_get( MIB_IGMP_FORBID_ENABLE, (void *)&vChar);
        websWrite(wp, "%s.tcpip.igmpforbid[%d].checked = true;\n", DOCUMENT, vChar);
#endif
#endif

#ifdef WLAN_SUPPORT
	mib_get( MIB_WLAN_BLOCK_ETH2WIR, (void *)&vChar);
	websWrite(wp, "%s.tcpip.BlockEth2Wir[%d].checked = true;\n", DOCUMENT, vChar==0?0:1);
#endif	
}


void initSetDslPage(webs_t wp)
{
	unsigned char vChar;
	unsigned short mode;
	
	mib_get( MIB_ADSL_MODE, (void *)&mode);
	// TODO: adsl mode
	if (mode & ADSL_MODE_GLITE)
		websWrite(wp, "%s.set_adsl.glite.checked = true;\n", DOCUMENT);
	if (mode & ADSL_MODE_T1413)
		websWrite(wp, "%s.set_adsl.t1413.checked = true;\n", DOCUMENT);
	if (mode & ADSL_MODE_GDMT)
		websWrite(wp, "%s.set_adsl.gdmt.checked = true;\n", DOCUMENT);
	if (mode & ADSL_MODE_ADSL2)
		websWrite(wp, "%s.set_adsl.adsl2.checked = true;\n", DOCUMENT);
	if (mode & ADSL_MODE_ADSL2P)
		websWrite(wp, "%s.set_adsl.adsl2p.checked = true;\n", DOCUMENT);
	if (mode & ADSL_MODE_ANXB) {
		websWrite(wp, "%s.set_adsl.anxb.checked = true;\n", DOCUMENT);
		websWrite(wp, "%s.set_adsl.anxl.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.set_adsl.anxm.disabled = true;\n", DOCUMENT);
	}
	else {
		if (mode & ADSL_MODE_ANXL)
			websWrite(wp, "%s.set_adsl.anxl.checked = true;\n", DOCUMENT);
		if (mode & ADSL_MODE_ANXM)
			websWrite(wp, "%s.set_adsl.anxm.checked = true;\n", DOCUMENT);
	}
	
	mib_get( MIB_ADSL_OLR, (void *)&vChar);
	
	if (vChar & 1)
		websWrite(wp, "%s.set_adsl.bswap.checked = true;\n", DOCUMENT);
	if (vChar & 2)
		websWrite(wp, "%s.set_adsl.sra.checked = true;\n", DOCUMENT);
}

void initDiagDslPage(webs_t wp)
{
	char chVal[2];
	if(adsl_drv_get(RLCM_GET_SHOWTIME_XDSL_MODE, (void *)&chVal[0], 1)) {
		chVal[0]&=0x1F;
		if (chVal[0] == 3 || chVal[0] == 5)  // ADSL2/2+
			websWrite(wp, "%s.diag_adsl.start.disabled = false;\n", DOCUMENT);
	}
}

#ifdef PORT_TRIGGERING
int portTrgList(webs_t wp)
{
	unsigned int entryNum, i;
	MIB_CE_PORT_TRG_T Entry;
	char	*type, portRange[20], *ip;

	entryNum = mib_chain_total(MIB_PORT_TRG_TBL);

	websWrite(wp,"<tr><td bgColor=#808080>Name</td><td bgColor=#808080>IP</td>"
		"<td bgColor=#808080>TCP Port to Open</td><td bgColor=#808080>UDP Port to Open</td><td bgColor=#808080>Enable</td><td bgColor=#808080>Actions</td></tr>\n");
	
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_PORT_TRG_TBL, i, (void *)&Entry))
		{
  			websError(wp, 400, T("Get chain record error!\n"));
			return -1;
		}
		
		//Name
		websWrite(wp,"<tr><td bgColor=#C0C0C0>%s</td>\n",Entry.name);
        	
		//IP
		websWrite(wp,"<td bgColor=#C0C0C0>%s</td>\n",inet_ntoa(*((struct in_addr *)Entry.ip)));
        	
		//TCP port to open
		websWrite(wp,"<td bgColor=#C0C0C0>%s</td>\n",Entry.tcpRange);
        	
		//UDP port to open
		websWrite(wp,"<td bgColor=#C0C0C0>%s</td>\n",Entry.udpRange);
        	
		//Enable
		websWrite(wp,"<td bgColor=#C0C0C0>%s</td>\n",(Entry.enable==1)?"Enable":"Disable");
		
		//Actions
		websWrite(wp,"<td bgColor=#C0C0C0>");
		websWrite(wp, T(
		"<a href=\"#?edit\" onClick=\"editClick(%d)\">"
		"<image border=0 src=\"graphics/edit.gif\" alt=\"Post for editing\" /></a>"), i);
		
		websWrite(wp, T(
		"<a href=\"#?delete\" onClick=\"delClick(%d)\">"
		"<image border=0 src=\"graphics/del.gif\" alt=Delete /></td></tr>\n"), i);
	}
	
	return 0;
}

int gm_postIndex=-1;

void initGamingPage(webs_t wp)
{
	char *ipaddr;
	char *idx;
	int del;
	char ipaddr2[16]={0};
	MIB_CE_PORT_TRG_T Entry;
	int found=0;
	
	ipaddr=websGetVar(wp,"ip","");
	idx=websGetVar(wp,"idx","");
	del=atoi(websGetVar(wp,"del",""));

	if (gm_postIndex >= 0) { // post this entry
		if (!mib_chain_get(MIB_PORT_TRG_TBL, gm_postIndex, (void *)&Entry))
			found = 0;
		else
			found = 1;
		gm_postIndex = -1;
	}
	
	if(del!=0)
	{
		websWrite(wp,
		"<body onLoad=\"document.formname.submit()\">");
	}
	else
	{
		websWrite(wp,
		"<body bgcolor=\"#ffffff\" text=\"#000000\" onLoad=\"javascript:formLoad();\">");
		websWrite(wp, "<blockquote><h2><font color=\"#0000FF\">Port Triggering</font></h2>\n");
		websWrite(wp, "<table border=0 width=850 cellspacing=4 cellpadding=0><tr><td><hr size=1 noshade align=top></td></tr>\n");
		//<b>%s Game Rule</b>%s",(strlen(idx)==0)?"Add":"Edit",(strlen(idx)==0)?"":" [<a href=\"gaming.asp\">Add New</a>]");
	}


	websWrite(wp,
	"<form action=/goform/formGaming method=POST name=formname>\n");
	
	if(del!=0)
	{
		int i=atoi(idx);
		websWrite(wp,"<input type=hidden name=idx value=\"%d\">",i);
		websWrite(wp,"<input type=hidden name=del value=1></form>");
		return;
	}

	websWrite(wp,
	"<table width=850 cellSpacing=1 cellPadding=2 border=0>\n" \
	"<tr><font size=2><td bgColor=#808080>Name</td><td bgColor=#808080>IP Address</td><td bgColor=#808080>TCP Port to Open</td><td bgColor=#808080>UDP Port to Open</td><td bgColor=#808080>Enable</td></tr>\n");
	
	websWrite(wp,
	"<tr><td bgColor=#C0C0C0><font size=2><input type=text name=\"game\" size=16  maxlength=20 value=\"%s\">&lt;&lt; <select name=\"gamelist\" onChange=\"javascript:changeItem();\"></select></td>" \
	"<td bgColor=#C0C0C0><input type=text name=\"ip\" size=12  maxlength=15 value=\"%s\"></td>" \
	"<td bgColor=#C0C0C0><input type=text name=\"tcpopen\" size=20  maxlength=31 value=\"%s\"></td>" \
	"<td bgColor=#C0C0C0><input type=text name=\"udpopen\" size=20  maxlength=31 value=\"%s\"></td>" \
	"<td bgColor=#C0C0C0><input type=checkbox name=\"open\" value=1 %s></td>" \
	"<input type=hidden name=idx value=%s>" \
	"</tr></table>\n"
	, found? (char *)Entry.name:""
	, found? (char *)inet_ntoa(*((struct in_addr *)Entry.ip)):"0.0.0.0"
	, found? (char *)Entry.tcpRange:""
	, found? (char *)Entry.udpRange:""
	, found? (Entry.enable==1?"checked":""):""
	,(strlen(idx)==0)?"-1":idx
	);
	
	websWrite(wp,
	"<input type=submit value=Add name=add onClick=\"return addClick()\">&nbsp;&nbsp;&nbsp;&nbsp;" \
	"<input type=submit value=Modify name=modify onClick=\"return addClick()\">&nbsp;&nbsp;&nbsp;&nbsp;" \
	"<input type=reset value=Reset><BR><BR>\n");
	websWrite(wp,
	"<input type=hidden value=/gaming.asp name=submit-url>");

	websWrite(wp,
	"<b>Game Rules List</b>\n" \
//	"<input type=hidden name=ms value=%d>\n" \
/*	"onSubmit=\"return checkRange();\"" \ */
	"<table cellSpacing=1 cellPadding=2 border=0>\n");
	
	portTrgList(wp);
	
	websWrite(wp, "</form>\n");
}
#endif

#ifdef CONFIG_USER_ROUTED_ROUTED
void initRipPage(webs_t wp)
{
	if (ifWanNum("rt") ==0) {
		websWrite(wp, "%s.rip.rip_on[0].disabled = true;\n", DOCUMENT);
		websWrite(wp, "\t%s.rip.rip_on[1].disabled = true;\n", DOCUMENT);
		websWrite(wp, "\t%s.rip.rip_ver.disabled = true;\n", DOCUMENT);
		websWrite(wp, "\t%s.rip.rip_if.disabled = true;\n", DOCUMENT);
		websWrite(wp, "\t%s.rip.ripAdd.disabled = true;\n", DOCUMENT);
		websWrite(wp, "\t%s.rip.ripSet.disabled = true;\n", DOCUMENT);
		websWrite(wp, "\t%s.rip.ripReset.disabled = true;", DOCUMENT);
	}
	websWrite(wp, "\t%s.rip.ripDel.disabled = true;\n", DOCUMENT);
}
#endif

#ifdef CONFIG_RTL867X_COMBO_PORTMAPPING
void initPortMapPage(webs_t wp)
{
	unsigned char vChar;
	
	mib_get( MIB_MPMODE, (void *)&vChar);
	websWrite(wp, "%s.eth2pvc.pmap[%d].checked = true;\n", DOCUMENT, vChar&0x01);
}

#endif

#ifdef CONFIG_EXT_SWITCH
#if 0
void initMpModePage(webs_t wp)
{
	unsigned char vChar;
	
	mib_get( MIB_MPMODE, (void *)&vChar);
	//websWrite(wp, "%s.mpmode.mpMode.value = %d;\n", DOCUMENT, (int)vChar);
	// bitmap for virtual lan port function
	// Port Mapping: bit-0
	// QoS : bit-1
	websWrite(wp, "%s.mpmode.pmap[%d].checked = true;\n", DOCUMENT, vChar&0x01);
	websWrite(wp, "%s.mpmode.qos[%d].checked = true;\n", DOCUMENT, (vChar>>1)&0x01);
}
#endif

void initPortMapPage(webs_t wp)
{
#ifdef ITF_GROUP
	unsigned char vChar;
	
	mib_get( MIB_MPMODE, (void *)&vChar);
	websWrite(wp, "%s.eth2pvc.pmap[%d].checked = true;\n", DOCUMENT, vChar&0x01);
	/*
	if (!(vChar&0x01)) { // Port Mapping not enabled
		websWrite(wp, "%s.eth2pvc.vc0.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.eth2pvc.vc1.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.eth2pvc.vc2.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.eth2pvc.vc3.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.eth2pvc.save.disabled = true;\n", DOCUMENT);
	}
	*/
#endif
}

void initVlanGroupPage(webs_t wp)
{
#ifdef VLAN_GROUP
	unsigned char vChar;
	
	mib_get( MIB_MPMODE, (void *)&vChar);
	websWrite(wp, "%s.eth2pvc.pmap[%d].checked = true;\n", DOCUMENT, vChar&0x01);
#endif
}

#if 0
void initVlanPage(webs_t wp)
{
	unsigned char vChar;
	
	mib_get( MIB_MPMODE, (void *)&vChar);
	if (vChar != 2) { // not vlan config
		websWrite(wp, "%s.vlan.pa0.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.vlan.pa1.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.vlan.pa2.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.vlan.pa3.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.vlan.tag_a.disabled = true;\n", DOCUMENT);
		
		websWrite(wp, "%s.vlan.pb0.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.vlan.pb1.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.vlan.pb2.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.vlan.pb3.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.vlan.tag_b.disabled = true;\n", DOCUMENT);
		
		websWrite(wp, "%s.vlan.pc0.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.vlan.pc1.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.vlan.pc2.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.vlan.pc3.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.vlan.tag_c.disabled = true;\n", DOCUMENT);
		
		websWrite(wp, "%s.vlan.pd0.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.vlan.pd1.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.vlan.pd2.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.vlan.pd3.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.vlan.tag_d.disabled = true;\n", DOCUMENT);
		
		websWrite(wp, "%s.vlan.pidx0.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.vlan.act0.disabled = true;\n", DOCUMENT);
		
		websWrite(wp, "%s.vlan.pidx1.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.vlan.act1.disabled = true;\n", DOCUMENT);
		
		websWrite(wp, "%s.vlan.pidx2.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.vlan.act2.disabled = true;\n", DOCUMENT);
		
		websWrite(wp, "%s.vlan.pidx3.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.vlan.act3.disabled = true;\n", DOCUMENT);
		
		websWrite(wp, "%s.vlan.save.disabled = true;\n", DOCUMENT);
	}
}
#endif

void initLinkPage(webs_t wp)
{
#ifdef ELAN_LINK_MODE
	unsigned int entryNum, i;
	MIB_CE_SW_PORT_T Entry;
	char ports[]="p0";
	
	entryNum = mib_chain_total(MIB_SW_PORT_TBL);
	
	for (i=0; i<entryNum; i++) {
		if (mib_chain_get(MIB_SW_PORT_TBL, i, (void *)&Entry)) {
			ports[1]=i + '0';
			websWrite(wp, "%s.link.%s.value = %d;\n", DOCUMENT, ports, Entry.linkMode);
		}
	}
#endif
}

#else
#ifdef ELAN_LINK_MODE_INTRENAL_PHY
void initLinkPage(webs_t wp)
{

	unsigned int entryNum, i;
	//MIB_CE_SW_PORT_T Entry;
	char ports[]="p0";
	unsigned char mode;
	
	//entryNum = mib_chain_total(MIB_SW_PORT_TBL);
	if (mib_get(MIB_ETH_MODE, &mode)) {
		websWrite(wp, "%s.link.%s.value = %d;\n", DOCUMENT, ports, mode);
	}
}

#endif
#endif	// of CONFIG_EXT_SWITCH
	
void initIpQosPage(webs_t wp)
{
#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
	unsigned char vChar;
	unsigned int entryNum;
#ifdef NEW_IP_QOS_SUPPORT
	unsigned char policy;
#endif
#ifdef QOS_DIFFSERV
	unsigned char qosDomain;

	mib_get(MIB_QOS_DIFFSERV, (void *)&qosDomain);
	mib_get(MIB_MPMODE, (void *)&vChar);
	if (qosDomain == 1)
		websWrite(wp, "%s.qos.qosen[0].checked = true;\n", DOCUMENT);
	else
		websWrite(wp, "%s.qos.qosen[%d].checked = true;\n", DOCUMENT, (vChar>>1)&0x01);
#else
	
	mib_get( MIB_MPMODE, (void *)&vChar);
	websWrite(wp, "%s.qos.qosen[%d].checked = true;\n", DOCUMENT, (vChar>>1)&0x01);
#endif

#ifdef NEW_IP_QOS_SUPPORT
	mib_get( MIB_QOS_POLICY, (void *)&policy);
	websWrite(wp, "%s.qos.qosPolicy[%d].checked = true;\n", DOCUMENT, policy&0x01);
#endif
	/*
	if (!(vChar&0x02)) { // IP Qos not enabled
		websWrite(wp, "%s.qos.sip.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.qos.smask.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.qos.dip.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.qos.dmask.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.qos.sport.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.qos.dport.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.qos.prot.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.qos.phyport.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.qos.out_if.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.qos.prio.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.qos.ipprio.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.qos.tos.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.qos.m1p.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.qos.addqos.disabled = true;\n", DOCUMENT);
	}
	*/
#ifdef QOS_SPEED_LIMIT_SUPPORT
	if ((vChar&0x02)) { // IP Qos  enabled
		websWrite(wp,T("document.getElementById('pvcbandwidth').style.display = 'block';\n"));
		unsigned short bandwidth;
		mib_get(MIB_PVC_TOTAL_BANDWIDTH,&bandwidth);
		printf("bandwidth=%d\n",bandwidth);
		websWrite(wp,T("document.upbandwidthfm.upbandwidth.value=%d;"),bandwidth);
		
	}
#endif
	if (ifWanNum("all") == 0)
		websWrite(wp, "%s.qos.addqos.disabled = true;\n", DOCUMENT);
	
	entryNum = mib_chain_total(MIB_IP_QOS_TBL);
	if (entryNum == 0) {
		websWrite(wp, "%s.qostbl.delSel.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.qostbl.delAll.disabled = true;\n", DOCUMENT);
	}

	
//#ifndef IP_QOS_VPORT
#ifndef CONFIG_RE8305
	mib_get( MIB_QOS_DOMAIN, (void *)&vChar);
	websWrite(wp, "%s.qos.qosdmn.value = %d;\n", DOCUMENT, vChar);
#ifdef CONFIG_8021P_PRIO
	websWrite(wp, "enable8021psetting();\n");
#endif
#endif
//#endif
#endif
}

#ifdef QOS_DIFFSERV
void initDiffservPage(webs_t wp)
{
	unsigned char vChar, phbclass;
	unsigned int entryNum, i;
	MIB_CE_IP_QOS_T Entry;
	
	mib_get(MIB_QOS_DIFFSERV, (void *)&vChar);
	mib_get(MIB_DIFFSERV_PHBCLASS, (void *)&phbclass);
	websWrite(wp, "%s.diffserv.qoscap[%d].checked = true;\n", DOCUMENT, vChar);
	entryNum = mib_chain_total(MIB_IP_QOS_TBL);
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_IP_QOS_TBL, i, (void *)&Entry)) {
  			websError(wp, 400, T("Get chain record error!\n"));
			return;
		}
		if (Entry.enDiffserv == 0) // IP QoS entry
			continue;
		if (Entry.m_ipprio != phbclass) // only get active PHB class
			continue;
		websWrite(wp, "%s.diffserv.totalbandwidth.value = %d;\n", DOCUMENT, Entry.totalBandwidth);
		websWrite(wp, "%s.diffserv.htbrate.value = %d;\n", DOCUMENT, Entry.htbRate);
		websWrite(wp, "%s.diffserv.latency.value = %d;\n", DOCUMENT, Entry.latency);
		websWrite(wp, "%s.diffserv.phbclass.value = %d;\n", DOCUMENT, phbclass);
		websWrite(wp, "%s.diffserv.interface.value = %d;\n", DOCUMENT, Entry.ifIndex);
		return;
	}
}
#endif

void initOthersPage(webs_t wp)
{
	unsigned char vChar;
	
#ifdef IP_PASSTHROUGH
	mib_get( MIB_IPPT_ITF, (void *)&vChar);
	if (vChar == 0xff) {
		websWrite(wp, "%s.others.ltime.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.others.lan_acc.disabled = true;\n", DOCUMENT);
	}
	
	mib_get( MIB_IPPT_LANACC, (void *)&vChar);
	if (vChar == 1)
		websWrite(wp, "%s.others.lan_acc.checked = true\n", DOCUMENT);
#endif
}

void initRebootPage(webs_t wp)
{
	unsigned char vChar;
	
	mib_get( MIB_BOOT_MODE, (void *)&vChar);
	websWrite(wp, "%s.cmboot.rebootMode.value = %d;\n", DOCUMENT, vChar);
}

#ifdef TIME_ZONE
void initNtpPage(webs_t wp)
{
	unsigned char vChar;
	
	mib_get( MIB_NTP_ENABLED, (void *)&vChar);
	if (vChar == 1)
		websWrite(wp, "%s.time.enabled.checked = true;\n", DOCUMENT);
	
	mib_get( MIB_NTP_SERVER_ID, (void *)&vChar);
	websWrite(wp, "%s.time.ntpServerId[%d].checked = true;\n", DOCUMENT, vChar&0x01);
}
#endif

#ifdef WLAN_SUPPORT

//xl_yue
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
int showAscIIKey(unsigned char * str, int len, unsigned char * dstStr)
{
	int i,j;
	for (i=0;i<len;i++)
	{
		//decide whether str[i] can be displayed or not
		if((str[i] <= 0x20) || (str[i] >=  0x7F))
			return 0;
	}
	for(i=0,j=0;i<len;i++,j++)
	{
		if(str[i] == '"' || str[i] == '\\'){
			dstStr[j++] = '\\';
		}
		dstStr[j] = str[i];
	}
	dstStr[j] = '\0';
	return 1;
}
#endif
void initWlWepPage(webs_t wp)
{
	unsigned char vChar;
	unsigned char buffer[20];
	unsigned char buffer2[40];
//xl_yue
#ifdef WLAN_SUPPORT
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)

	mib_get(MIB_WLAN_WEP, (void *)&vChar);
	if (vChar == 0){
			vChar = 1;
	}
	websWrite(wp, "%s.formWep.length.value = %d;\n", DOCUMENT, vChar);
	mib_get(MIB_WLAN_WEP_KEY_TYPE, (void *)&vChar);
	websWrite(wp, "%s.formWep.format.value = %d;\n", DOCUMENT, vChar+1);
	mib_get(MIB_WLAN_WEP_DEFAULT_KEY, (void *)&vChar);
	websWrite(wp, "%s.formWep.defaultTxKeyId.value = %d;\n", DOCUMENT, vChar+1);

	mib_get(MIB_WLAN_WEP64_KEY1, (void *)buffer);
	if(showAscIIKey(buffer,5,buffer2)){
//		websWrite(wp, "asc64_key1[0]=\"%c%c%c%c%c\";\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4]);
		websWrite(wp, "asc64_key1[0]=\"%s\";\n",buffer2);
	}else{
		websWrite(wp, "asc64_key1[0]=\"\";\n");
	}
	websWrite(wp, "hex64_key1[0]=\"%02x%02x%02x%02x%02x\";\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4]);


	mib_get(MIB_WLAN_WEP64_KEY2, (void *)buffer);
	if(showAscIIKey(buffer,5,buffer2)){
//		websWrite(wp, "asc64_key2[0]=\"%c%c%c%c%c\";\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4]);
		websWrite(wp, "asc64_key2[0]=\"%s\";\n",buffer2);
	}else{
		websWrite(wp, "asc64_key2[0]=\"\";\n");
	}
	websWrite(wp, "hex64_key2[0]=\"%02x%02x%02x%02x%02x\";\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4]);

	mib_get(MIB_WLAN_WEP64_KEY3, (void *)buffer);
	if(showAscIIKey(buffer,5,buffer2)){
//		websWrite(wp, "asc64_key3[0]=\"%c%c%c%c%c\";\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4]);
		websWrite(wp, "asc64_key3[0]=\"%s\";\n",buffer2);
	}else{
		websWrite(wp, "asc64_key3[0]=\"\";\n");
	}
	websWrite(wp, "hex64_key3[0]=\"%02x%02x%02x%02x%02x\";\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4]);

	mib_get(MIB_WLAN_WEP64_KEY4, (void *)buffer);
	if(showAscIIKey(buffer,5,buffer2)){
//		websWrite(wp, "asc64_key4[0]=\"%c%c%c%c%c\";\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4]);
		websWrite(wp, "asc64_key4[0]=\"%s\";\n",buffer2);
	}else{
		websWrite(wp, "asc64_key4[0]=\"\";\n");
	}
	websWrite(wp, "hex64_key4[0]=\"%02x%02x%02x%02x%02x\";\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4]);

	mib_get(MIB_WLAN_WEP128_KEY1, (void *)buffer);
	if(showAscIIKey(buffer,13,buffer2)){
/*		websWrite(wp, "asc128_key1[0]=\"%c%c%c%c%c%c%c%c%c%c%c%c%c\";\n",
				buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],
				buffer[6],buffer[7],buffer[8],buffer[9],buffer[10],buffer[11],buffer[12]);
*/
		websWrite(wp, "asc128_key1[0]=\"%s\";\n",buffer2);

	}else{
		websWrite(wp, "asc128_key1[0]=\"\";\n");
	}
	websWrite(wp, "hex128_key1[0]=\"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\";\n",
				buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],
				buffer[6],buffer[7],buffer[8],buffer[9],buffer[10],buffer[11],buffer[12]);

	mib_get(MIB_WLAN_WEP128_KEY2, (void *)buffer);
	if(showAscIIKey(buffer,13,buffer2)){
/*		websWrite(wp, "asc128_key2[0]=\"%c%c%c%c%c%c%c%c%c%c%c%c%c\";\n",
				buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],
				buffer[6],buffer[7],buffer[8],buffer[9],buffer[10],buffer[11],buffer[12]);
*/
		websWrite(wp, "asc128_key2[0]=\"%s\";\n",buffer2);
	}else{
		websWrite(wp, "asc128_key2[0]=\"\";\n");
	}
	websWrite(wp, "hex128_key2[0]=\"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\";\n",
				buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],
				buffer[6],buffer[7],buffer[8],buffer[9],buffer[10],buffer[11],buffer[12]);

	mib_get(MIB_WLAN_WEP128_KEY3, (void *)buffer);
	if(showAscIIKey(buffer,13,buffer2)){
/*		websWrite(wp, "asc128_key3[0]=\"%c%c%c%c%c%c%c%c%c%c%c%c%c\";\n",
				buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],
				buffer[6],buffer[7],buffer[8],buffer[9],buffer[10],buffer[11],buffer[12]);
*/
		websWrite(wp, "asc128_key3[0]=\"%s\";\n",buffer2);
	}else{
		websWrite(wp, "asc128_key3[0]=\"\";\n");
	}
	websWrite(wp, "hex128_key3[0]=\"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\";\n",
				buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],
				buffer[6],buffer[7],buffer[8],buffer[9],buffer[10],buffer[11],buffer[12]);

	mib_get(MIB_WLAN_WEP128_KEY4, (void *)buffer);
	if(showAscIIKey(buffer,13,buffer2)){
/*		websWrite(wp, "asc128_key4[0]=\"%c%c%c%c%c%c%c%c%c%c%c%c%c\";\n",
				buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],
				buffer[6],buffer[7],buffer[8],buffer[9],buffer[10],buffer[11],buffer[12]);
*/
		websWrite(wp, "asc128_key4[0]=\"%s\";\n",buffer2);
	}else{
		websWrite(wp, "asc128_key4[0]=\"\";\n");
	}
	websWrite(wp, "hex128_key4[0]=\"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\";\n",
				buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],
				buffer[6],buffer[7],buffer[8],buffer[9],buffer[10],buffer[11],buffer[12]);

#ifdef WLAN_MBSSID
	int i;
	MIB_CE_MBSSIB_WEP_T Entry;
	MIB_CE_MBSSIB_T EntryWPA;
	
	for(i=1;i<5;i++){
		
		if (!mib_chain_get(MIB_MBSSIB_WEP_TBL, i, (void *)&Entry)) {  		
  			printf("Error! Get MIB_MBSSIB_TBL(Root) error.\n");
  			return 1;					
		}	

		if (!mib_chain_get(MIB_MBSSIB_TBL, i, (void *)&EntryWPA)) {  		
  			printf("Error! Get MIB_MBSSIB_TBL(Root) error.\n");
  			return 1;					
		}

		if(showAscIIKey(Entry.wep64Key1,5,buffer2)){
//			websWrite(wp, "asc64_key1[%d]=\"%c%c%c%c%c\";\n",i,Entry.wep64Key1[0],Entry.wep64Key1[1],Entry.wep64Key1[2],Entry.wep64Key1[3],Entry.wep64Key1[4]);
			websWrite(wp, "asc64_key1[%d]=\"%s\";\n",i,buffer2);
		}else{
			websWrite(wp, "asc64_key1[%d]=\"\";\n",i);
		}
		
		if(showAscIIKey(Entry.wep64Key2,5,buffer2)){
//			websWrite(wp, "asc64_key2[%d]=\"%c%c%c%c%c\";\n",i,Entry.wep64Key2[0],Entry.wep64Key2[1],Entry.wep64Key2[2],Entry.wep64Key2[3],Entry.wep64Key2[4]);
			websWrite(wp, "asc64_key2[%d]=\"%s\";\n",i,buffer2);
		}else{
			websWrite(wp, "asc64_key2[%d]=\"\";\n",i);
		}
		
		if(showAscIIKey(Entry.wep64Key3,5,buffer2)){
//			websWrite(wp, "asc64_key3[%d]=\"%c%c%c%c%c\";\n",i,Entry.wep64Key3[0],Entry.wep64Key3[1],Entry.wep64Key3[2],Entry.wep64Key3[3],Entry.wep64Key3[4]);
			websWrite(wp, "asc64_key3[%d]=\"%s\";\n",i,buffer2);
		}else{
			websWrite(wp, "asc64_key3[%d]=\"\";\n",i);
		}
		
		if(showAscIIKey(Entry.wep64Key4,5,buffer2)){
//			websWrite(wp, "asc64_key4[%d]=\"%c%c%c%c%c\";\n",i,Entry.wep64Key4[0],Entry.wep64Key4[1],Entry.wep64Key4[2],Entry.wep64Key4[3],Entry.wep64Key4[4]);
			websWrite(wp, "asc64_key4[%d]=\"%s\";\n",i,buffer2);
		}else{
			websWrite(wp, "asc64_key4[%d]=\"\";\n",i);
		}

		if(showAscIIKey(Entry.wep128Key1,13,buffer2)){
/*			websWrite(wp, "asc128_key1[%d]=\"%c%c%c%c%c%c%c%c%c%c%c%c%c\";\n",
					i,Entry.wep128Key1[0],Entry.wep128Key1[1],Entry.wep128Key1[2],Entry.wep128Key1[3],Entry.wep128Key1[4],Entry.wep128Key1[5],
					Entry.wep128Key1[6],Entry.wep128Key1[7],Entry.wep128Key1[8],Entry.wep128Key1[9],Entry.wep128Key1[10],Entry.wep128Key1[11],Entry.wep128Key1[12]);
*/
			websWrite(wp, "asc128_key1[%d]=\"%s\";\n",i,buffer2);
		}else{
			websWrite(wp, "asc128_key1[%d]=\"\";\n",i);
		}

		if(showAscIIKey(Entry.wep128Key2,13,buffer2)){
/*			websWrite(wp, "asc128_key2[%d]=\"%c%c%c%c%c%c%c%c%c%c%c%c%c\";\n",
					i,Entry.wep128Key2[0],Entry.wep128Key2[1],Entry.wep128Key2[2],Entry.wep128Key2[3],Entry.wep128Key2[4],Entry.wep128Key2[5],
					Entry.wep128Key2[6],Entry.wep128Key2[7],Entry.wep128Key2[8],Entry.wep128Key2[9],Entry.wep128Key2[10],Entry.wep128Key2[11],Entry.wep128Key2[12]);
*/
			websWrite(wp, "asc128_key2[%d]=\"%s\";\n",i,buffer2);
		}else{
			websWrite(wp, "asc128_key2[%d]=\"\";\n",i);
		}

		if(showAscIIKey(Entry.wep128Key3,13,buffer2)){
/*			websWrite(wp, "asc128_key3[%d]=\"%c%c%c%c%c%c%c%c%c%c%c%c%c\";\n",
					i,Entry.wep128Key3[0],Entry.wep128Key3[1],Entry.wep128Key3[2],Entry.wep128Key3[3],Entry.wep128Key3[4],Entry.wep128Key3[5],
					Entry.wep128Key3[6],Entry.wep128Key3[7],Entry.wep128Key3[8],Entry.wep128Key3[9],Entry.wep128Key3[10],Entry.wep128Key3[11],Entry.wep128Key3[12]);
*/
			websWrite(wp, "asc128_key3[%d]=\"%s\";\n",i,buffer2);
		}else{
			websWrite(wp, "asc128_key3[%d]=\"\";\n",i);
		}

		if(showAscIIKey(Entry.wep128Key4,13,buffer2)){
/*			websWrite(wp, "asc128_key4[%d]=\"%c%c%c%c%c%c%c%c%c%c%c%c%c\";\n",
					i,Entry.wep128Key4[0],Entry.wep128Key4[1],Entry.wep128Key4[2],Entry.wep128Key4[3],Entry.wep128Key4[4],Entry.wep128Key4[5],
					Entry.wep128Key4[6],Entry.wep128Key4[7],Entry.wep128Key4[8],Entry.wep128Key4[9],Entry.wep128Key4[10],Entry.wep128Key4[11],Entry.wep128Key4[12]);
*/
			websWrite(wp, "asc128_key4[%d]=\"%s\";\n",i,buffer2);
		}else{
			websWrite(wp, "asc128_key4[%d]=\"\";\n",i);
		}

		websWrite(wp, "hex64_key1[%d]=\"%02x%02x%02x%02x%02x\";\n",i,Entry.wep64Key1[0],Entry.wep64Key1[1],Entry.wep64Key1[2],Entry.wep64Key1[3],Entry.wep64Key1[4]);
		websWrite(wp, "hex64_key2[%d]=\"%02x%02x%02x%02x%02x\";\n",i,Entry.wep64Key2[0],Entry.wep64Key2[1],Entry.wep64Key2[2],Entry.wep64Key2[3],Entry.wep64Key2[4]);
		websWrite(wp, "hex64_key3[%d]=\"%02x%02x%02x%02x%02x\";\n",i,Entry.wep64Key3[0],Entry.wep64Key3[1],Entry.wep64Key3[2],Entry.wep64Key3[3],Entry.wep64Key3[4]);
		websWrite(wp, "hex64_key4[%d]=\"%02x%02x%02x%02x%02x\";\n",i,Entry.wep64Key4[0],Entry.wep64Key4[1],Entry.wep64Key4[2],Entry.wep64Key4[3],Entry.wep64Key4[4]);
		websWrite(wp, "hex128_key1[%d]=\"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\";\n",
					i,Entry.wep128Key1[0],Entry.wep128Key1[1],Entry.wep128Key1[2],Entry.wep128Key1[3],Entry.wep128Key1[4],Entry.wep128Key1[5],
					Entry.wep128Key1[6],Entry.wep128Key1[7],Entry.wep128Key1[8],Entry.wep128Key1[9],Entry.wep128Key1[10],Entry.wep128Key1[11],Entry.wep128Key1[12]);
		websWrite(wp, "hex128_key2[%d]=\"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\";\n",
					i,Entry.wep128Key2[0],Entry.wep128Key2[1],Entry.wep128Key2[2],Entry.wep128Key2[3],Entry.wep128Key2[4],Entry.wep128Key2[5],
					Entry.wep128Key2[6],Entry.wep128Key2[7],Entry.wep128Key2[8],Entry.wep128Key2[9],Entry.wep128Key2[10],Entry.wep128Key2[11],Entry.wep128Key2[12]);
		websWrite(wp, "hex128_key3[%d]=\"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\";\n",
					i,Entry.wep128Key3[0],Entry.wep128Key3[1],Entry.wep128Key3[2],Entry.wep128Key3[3],Entry.wep128Key3[4],Entry.wep128Key3[5],
					Entry.wep128Key3[6],Entry.wep128Key3[7],Entry.wep128Key3[8],Entry.wep128Key3[9],Entry.wep128Key3[10],Entry.wep128Key3[11],Entry.wep128Key3[12]);
		websWrite(wp, "hex128_key4[%d]=\"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\";\n",
					i,Entry.wep128Key4[0],Entry.wep128Key4[1],Entry.wep128Key4[2],Entry.wep128Key4[3],Entry.wep128Key4[4],Entry.wep128Key4[5],
					Entry.wep128Key4[6],Entry.wep128Key4[7],Entry.wep128Key4[8],Entry.wep128Key4[9],Entry.wep128Key4[10],Entry.wep128Key4[11],Entry.wep128Key4[12]);
	}

#endif
	//websWrite(wp, "%s.formWep.wepSSID[0].checked = true;\n", DOCUMENT);
	websWrite(wp, "checkedSSID = 0;\n");

#else
	mib_get( MIB_WLAN_WEP, (void *)&vChar );
	if (vChar == 0)
		vChar = 1;
	websWrite(wp, "%s.formWep.length.value = %d;\n", DOCUMENT, vChar);
	mib_get( MIB_WLAN_WEP_KEY_TYPE, (void *)&vChar );
	websWrite(wp, "%s.formWep.format.value = %d;\n", DOCUMENT, vChar+1);
	mib_get( MIB_WLAN_WEP_DEFAULT_KEY, (void *)&vChar );
	websWrite(wp, "%s.formWep.defaultTxKeyId.value = %d;\n", DOCUMENT, vChar+1);
#endif
#endif
}

#ifdef WLAN_WPA
void initWlWpaPage(webs_t wp)
{
	unsigned char vChar;
	unsigned char buffer[255];
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	char strbuf2[20];
	unsigned short rsPort;
	unsigned char value[30];
#endif

	mib_get( MIB_WLAN_ENCRYPT, (void *)&vChar);
//xl_yue
#ifdef ENABLE_WPAAES_WPA2TKIP
	if(vChar == ENCRYPT_WPA){
		mib_get( MIB_WLAN_WPA_CIPHER_SUITE, (void *)&buffer[0]);
		if(buffer[0] == WPA_CIPHER_AES)
			vChar = ENCRY_WPA_AES;
		else
			vChar = ENCRY_WPA_TKIP;
	}else if(vChar == ENCRYPT_WPA2){
		mib_get( MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&buffer[0]);
		if(buffer[0] == WPA_CIPHER_AES)
			vChar = ENCRY_WPA2_AES;
		else
			vChar = ENCRY_WPA2_TKIP;
	}
#endif
	websWrite(wp, "%s.formEncrypt.method.value = %d;\n", DOCUMENT, vChar);

	mib_get( MIB_WLAN_WPA_PSK_FORMAT, (void *)&vChar);
	websWrite(wp, "%s.formEncrypt.pskFormat.value = %d;\n", DOCUMENT, vChar);
//xl_yue
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	mib_get( MIB_WLAN_WPA_PSK, (void *)buffer);
	websWrite(wp, "%s.formEncrypt.pskValue.value = '%s';\n", DOCUMENT, buffer);
	websWrite(wp, "current_psk = '%s';\n",buffer);
	mib_get( MIB_WLAN_WPA_PSK_FORMAT, (void *)&vChar);
	websWrite(wp, "current_psk_format = %d;\n",vChar);
	//added by xl_yue: create random wpa key
	if (mib_get(MIB_ELAN_MAC_ADDR, (void *)value) != 0)
	{
		value[0] = value[0]^value[4]^value[5];
		value[1] = value[1]^value[4]^value[5];
		value[2] = value[2]^value[4]^value[5];
		value[3] = value[3]^value[4]^value[5];
		websWrite(wp, "random_wpa_key_hex = '%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x';\n",
				value[0],value[1],value[2],value[3],value[0],value[1],value[2],value[3],value[0],value[1],value[2],value[3],value[0],value[1],value[2],value[3],
				value[0],value[1],value[2],value[3],value[0],value[1],value[2],value[3],value[0],value[1],value[2],value[3],value[0],value[1],value[2],value[3]);
		websWrite(wp, "random_wpa_key_pass = '%02x%02x%02x%02x';\n",value[0],value[1],value[2],value[3]);
	}
#endif

#ifdef WLAN_1x
	mib_get( MIB_WLAN_WEP, (void *)&vChar);
	if(vChar!=0)
		websWrite(wp, "%s.formEncrypt.wepKeyLen[%d].checked = true;\n", DOCUMENT, vChar-1);

	mib_get( MIB_WLAN_ENABLE_1X, (void *)&vChar);
	if(vChar==1)
		websWrite(wp, "%s.formEncrypt.use1x.checked = true;\n", DOCUMENT);
	mib_get( MIB_WLAN_WPA_AUTH, (void *)&vChar);
	websWrite(wp, "%s.formEncrypt.wpaAuth[%d].checked = true;\n", DOCUMENT, vChar-1);
#else
	websWrite(wp, "%s.formEncrypt.wpaAuth.disabled = true;\n", DOCUMENT);
	websWrite(wp, "%s.formEncrypt.wepKeyLen.disabled = true;\n", DOCUMENT);
	websWrite(wp, "%s.formEncrypt.use1x.disabled = true;\n", DOCUMENT);

#endif
//added by xl_yue
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	// RsIp
	mib_get(MIB_WLAN_RS_IP, (void *)buffer);
	if ( ((struct in_addr *)buffer)->s_addr == INADDR_NONE ) {
		sprintf(strbuf2, "%s", "");				
	} else {	
		sprintf(strbuf2, "%s", inet_ntoa(*((struct in_addr *)buffer)));
	}	
	websWrite(wp, "%s.formEncrypt.radiusIP.value = '%s';\n", DOCUMENT, strbuf2);
	mib_get(MIB_WLAN_RS_PORT, (void *)&rsPort);
	websWrite(wp, "%s.formEncrypt.radiusPort.value = %d;\n", DOCUMENT, rsPort);
	mib_get(MIB_WLAN_RS_PASSWORD, (void *)buffer);
	websWrite(wp, "%s.formEncrypt.radiusPass.value = '%s';\n", DOCUMENT, buffer);		
#endif
}
#endif

void initWlBasicPage(webs_t wp)
{
	unsigned char vChar;
//added by xl_yue
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	//selIndex must start from 0, it should keep accordance with wlCountryRegList's index,
	int selIndex = 0;
	struct wlCountryReg * pwlCountryReg = wlCountryRegList;
#endif
	mib_get( MIB_WLAN_DISABLED, (void *)&vChar);
	if (vChar!=0)
		// hidden type
		websWrite(wp, "%s.wlanSetup.wlanDisabled.value = \"ON\";\n", DOCUMENT);
		// checkbox type
		//websWrite(wp, "%s.wlanSetup.wlanDisabled.checked = true;\n", DOCUMENT);
	mib_get( MIB_WLAN_BAND, (void *)&vChar);
		websWrite(wp, "%s.wlanSetup.band.value = %d;\n", DOCUMENT, vChar-1);
#ifdef CONFIG_USB_RTL8192SU_SOFTAP             //add by yq_zhou 2.10
	mib_get( MIB_WLAN_CHANNEL_WIDTH,(void *)&vChar);          
		websWrite(wp, "%s.wlanSetup.chanwid.value = %d;\n", DOCUMENT, vChar-1);
	mib_get( MIB_WLAN_CONTROL_BAND,(void *)&vChar); 
		websWrite(wp, "%s.wlanSetup.ctlband.value = %d;\n", DOCUMENT, vChar-1);
#endif
//xl_yue:translocate to wlbasic_setting for ZTE531B BRIDGE
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	mib_get( MIB_WLAN_AUTH_TYPE, (void *)&vChar);
	websWrite(wp, "%s.wlanSetup.authType[%d].checked = true;\n", DOCUMENT, vChar);
	mib_get( MIB_WLAN_HIDDEN_SSID, (void *)&vChar);
	websWrite(wp, "%s.wlanSetup.hiddenSSID[%d].checked = true;\n", DOCUMENT, vChar);
	mib_get( MIB_WLAN_COUNTRY_AREA, (void *)&vChar);
	websWrite(wp, "%s.wlanSetup.domain.selectedIndex = %d;\n", DOCUMENT, vChar);
//	mib_get( MIB_TX_POWER, (void *)&vChar);
//	websWrite(wp, "%s.wlanSetup.txpower.selectedIndex = %d;\n", DOCUMENT, vChar);

	while(strlen(pwlCountryReg->country)){
		websWrite(wp, "regDomainList[%d] = %d\n",selIndex,pwlCountryReg->regDomain);
		pwlCountryReg++;
		selIndex++;
	}
#endif
#ifdef WLAN_TX_POWER_DISPLAY
	mib_get( MIB_TX_POWER, (void *)&vChar);
	websWrite(wp, "%s.wlanSetup.txpower.selectedIndex = %d;\n", DOCUMENT, vChar);
#endif

}

//#ifdef WLAN_WDS
void initWlWDSPage(webs_t wp){
	unsigned char disWlan,mode;
	mib_get( MIB_WLAN_DISABLED, (void *)&disWlan);
	mib_get( MIB_WLAN_MODE, (void *)&mode);
	if(disWlan || mode != AP_WDS_MODE){
		websWrite(wp,"%s.formWlWdsAdd.wlanWdsEnabled.disabled = true;\n",DOCUMENT);
	}
}

void initWlSurveyPage(webs_t wp){
	websWrite(wp,"%s.formWlSiteSurvey.refresh.disabled = true;\n",DOCUMENT);
#ifdef WLAN_CLIENT
	websWrite(wp,"%s.formWlSiteSurvey.refresh.disabled = false;\n",DOCUMENT);
#endif
}
//#endif

void initWlAdvPage(webs_t wp)
{
	unsigned char vChar;
#ifdef WIFI_TEST
	unsigned short vShort;
#endif
//xl_yue:translocate to wlbasic_setting  for ZTE531B BRIDGE
#if !(defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC))
	mib_get( MIB_WLAN_AUTH_TYPE, (void *)&vChar);
	websWrite(wp, "%s.advanceSetup.authType[%d].checked = true;\n", DOCUMENT, vChar);
#endif
#if 0
#ifdef WIFI_TEST
	mib_get( MIB_WLAN_SUPPORTED_RATE, (void *)&vShort);
	if (vShort & 1)
		websWrite(wp, "%s.advanceSetup.operRate1M.checked = true;\n", DOCUMENT);
	if (vShort & 2)
		websWrite(wp, "%s.advanceSetup.operRate2M.checked = true;\n", DOCUMENT);
	if (vShort & 4)
		websWrite(wp, "%s.advanceSetup.operRate5M.checked = true;\n", DOCUMENT);
	if (vShort & 8)
		websWrite(wp, "%s.advanceSetup.operRate11M.checked = true;\n", DOCUMENT);
	if (vShort & 16)
		websWrite(wp, "%s.advanceSetup.operRate6M.checked = true;\n", DOCUMENT);
	if (vShort & 32)
		websWrite(wp, "%s.advanceSetup.operRate9M.checked = true;\n", DOCUMENT);
	if (vShort & 64)
		websWrite(wp, "%s.advanceSetup.operRate12M.checked = true;\n", DOCUMENT);
	if (vShort & 128)
		websWrite(wp, "%s.advanceSetup.operRate18M.checked = true;\n", DOCUMENT);
	if (vShort & 256)
		websWrite(wp, "%s.advanceSetup.operRate24M.checked = true;\n", DOCUMENT);
	if (vShort & 512)
		websWrite(wp, "%s.advanceSetup.operRate36M.checked = true;\n", DOCUMENT);
	if (vShort & 1024)
		websWrite(wp, "%s.advanceSetup.operRate48M.checked = true;\n", DOCUMENT);
	if (vShort & 2048)
		websWrite(wp, "%s.advanceSetup.operRate54M.checked = true;\n", DOCUMENT);
	
	mib_get( MIB_WLAN_BASIC_RATE, (void *)&vShort);
	if (vShort & 1)
		websWrite(wp, "%s.advanceSetup.basicRate1M.checked = true;\n", DOCUMENT);
	if (vShort & 2)
		websWrite(wp, "%s.advanceSetup.basicRate2M.checked = true;\n", DOCUMENT);
	if (vShort & 4)
		websWrite(wp, "%s.advanceSetup.basicRate5M.checked = true;\n", DOCUMENT);
	if (vShort & 8)
		websWrite(wp, "%s.advanceSetup.basicRate11M.checked = true;\n", DOCUMENT);
	if (vShort & 16)
		websWrite(wp, "%s.advanceSetup.basicRate6M.checked = true;\n", DOCUMENT);
	if (vShort & 32)
		websWrite(wp, "%s.advanceSetup.basicRate9M.checked = true;\n", DOCUMENT);
	if (vShort & 64)
		websWrite(wp, "%s.advanceSetup.basicRate12M.checked = true;\n", DOCUMENT);
	if (vShort & 128)
		websWrite(wp, "%s.advanceSetup.basicRate18M.checked = true;\n", DOCUMENT);
	if (vShort & 256)
		websWrite(wp, "%s.advanceSetup.basicRate24M.checked = true;\n", DOCUMENT);
	if (vShort & 512)
		websWrite(wp, "%s.advanceSetup.basicRate36M.checked = true;\n", DOCUMENT);
	if (vShort & 1024)
		websWrite(wp, "%s.advanceSetup.basicRate48M.checked = true;\n", DOCUMENT);
	if (vShort & 2048)
		websWrite(wp, "%s.advanceSetup.basicRate54M.checked = true;\n", DOCUMENT);
	
	mib_get(MIB_WLAN_DTIM_PERIOD, (void *)&vChar);
	websWrite(wp, "%s.advanceSetup.dtimPeriod.value = %d;\n", DOCUMENT, vChar);
#endif
#endif
	mib_get( MIB_WLAN_PREAMBLE_TYPE, (void *)&vChar);
	websWrite(wp, "%s.advanceSetup.preamble[%d].checked = true;\n", DOCUMENT, vChar);
//xl_yue:translocate to wlbasic_setting  for ZTE531B BRIDGE
#if !(defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC))
	mib_get( MIB_WLAN_HIDDEN_SSID, (void *)&vChar);
	websWrite(wp, "%s.advanceSetup.hiddenSSID[%d].checked = true;\n", DOCUMENT, vChar);
#endif
	mib_get( MIB_WLAN_BLOCK_RELAY, (void *)&vChar);
	printf("%s  vchar is %c",__FUNCTION__,vChar);
	websWrite(wp, "%s.advanceSetup.block[%d].checked = true;\n", DOCUMENT, vChar==0?1:0);
#ifdef CONFIG_USB_RTL8192SU_SOFTAP			 //add by yq_zhou 1.20
	mib_get( MIB_WLAN_PROTECTION_DISABLED, (void *)&vChar);           
	websWrite(wp, "%s.advanceSetup.protection[%d].checked = true;\n", DOCUMENT, vChar);
	mib_get( MIB_WLAN_AGGREGATION, (void *)&vChar);
	websWrite(wp, "%s.advanceSetup.aggregation[%d].checked = true;\n", DOCUMENT, vChar); 
	mib_get( MIB_WLAN_SHORTGI_ENABLED, (void *)&vChar);
	websWrite(wp, "%s.advanceSetup.shortGI0[%d].checked = true;\n", DOCUMENT, vChar);
#endif
#ifdef WLAN_QoS
	mib_get( MIB_WLAN_QoS, (void *)&vChar);
	websWrite(wp, "%s.advanceSetup.WmmEnabled[%d].checked = true;\n", DOCUMENT, vChar==0?1:0);
#endif	
}

#ifdef WLAN_MBSSID
void initWLMBSSIDPage(webs_t wp)
{
	MIB_CE_MBSSIB_T Entry;
	int i=0;	
	
	for (i=1; i<=4; i++) {
		if (!mib_chain_get(MIB_MBSSIB_TBL, i, (void *)&Entry)) {  		
  			printf("Error! Get MIB_MBSSIB_TBL(initWLMBSSIDPage) error.\n");
  			return;					
		}
		websWrite(wp, "%s.WlanMBSSID.authType%d[%d].checked = true;\n", DOCUMENT, i-1, Entry.authType);
	}	
}

void initWlWpaMbssidPage(webs_t wp)
{
	unsigned char vChar;
	unsigned char buffer[255];
	
	char_t *name;
	MIB_CE_MBSSIB_T Entry;
	char strbuf[MAX_PSK_LEN+1], strbuf2[20];
	int len;
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	unsigned char value[30];
#endif

	/*
	if (!mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry)) {  		
  		printf("Error! Get MIB_MBSSIB_TBL(Root) error.\n");
  		return 1;					
	}
	*/
	MBSSID_GetRootEntry(&Entry);
	
	// wpaPSK
	for (len=0; len<strlen(Entry.wpaPSK); len++)
		strbuf[len]='*';
	strbuf[len]='\0';
	
	// RsIp
	if ( ((struct in_addr *)Entry.rsIpAddr)->s_addr == INADDR_NONE ) {
		sprintf(strbuf2, "%s", "");				
	} else {	
		sprintf(strbuf2, "%s", inet_ntoa(*((struct in_addr *)Entry.rsIpAddr)));
	}

	vChar = Entry.encrypt;
#ifdef ENABLE_WPAAES_WPA2TKIP
	if(Entry.encrypt == ENCRYPT_WPA){
		if(Entry.unicastCipher == WPA_CIPHER_AES)
			vChar = ENCRY_WPA_AES;
		else
			vChar = ENCRY_WPA_TKIP;
	}else if(Entry.encrypt == ENCRYPT_WPA2){
		if(Entry.wpa2UnicastCipher == WPA_CIPHER_AES)
			vChar = ENCRY_WPA2_AES;
		else
			vChar = ENCRY_WPA2_TKIP;
	}
#endif
	websWrite(wp, "%s.formEncrypt.method.value = %d;\n", DOCUMENT, vChar);	
	websWrite(wp, "%s.formEncrypt.pskFormat.value = %d;\n", DOCUMENT, Entry.wpaPSKFormat);

#ifdef WLAN_1x	
	if( Entry.wep!=0 )
		websWrite(wp, "%s.formEncrypt.wepKeyLen[%d].checked = true;\n", DOCUMENT, Entry.wep-1);
	
	if( Entry.enable1X==1 )
		websWrite(wp, "%s.formEncrypt.use1x.checked = true;\n", DOCUMENT);	
	websWrite(wp, "%s.formEncrypt.wpaAuth[%d].checked = true;\n", DOCUMENT, Entry.wpaAuth-1);
#else
	websWrite(wp, "%s.formEncrypt.wpaAuth.disabled = true;\n", DOCUMENT);
	websWrite(wp, "%s.formEncrypt.wepKeyLen.disabled = true;\n", DOCUMENT);
	websWrite(wp, "%s.formEncrypt.use1x.disabled = true;\n", DOCUMENT);
#endif	
//xl_yue
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	websWrite(wp, "%s.formEncrypt.pskValue.value = '%s';\n", DOCUMENT, Entry.wpaPSK);
	websWrite(wp, "current_psk_format = %d;\n",Entry.wpaPSKFormat);
	websWrite(wp, "current_psk = '%s';\n",Entry.wpaPSK);
	//added by xl_yue: create random wpa key
	if (mib_get(MIB_ELAN_MAC_ADDR, (void *)value) != 0)
	{
		value[0] = value[0]^value[4]^value[5];
		value[1] = value[1]^value[4]^value[5];
		value[2] = value[2]^value[4]^value[5];
		value[3] = value[3]^value[4]^value[5];
		websWrite(wp, "random_wpa_key_hex = '%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x';\n",
				value[0],value[1],value[2],value[3],value[0],value[1],value[2],value[3],value[0],value[1],value[2],value[3],value[0],value[1],value[2],value[3],
				value[0],value[1],value[2],value[3],value[0],value[1],value[2],value[3],value[0],value[1],value[2],value[3],value[0],value[1],value[2],value[3]);
		websWrite(wp, "random_wpa_key_pass = '%02x%02x%02x%02x';\n",value[0],value[1],value[2],value[3]);
	}
#else
	websWrite(wp, "%s.formEncrypt.pskValue.value = '%s';\n", DOCUMENT, strbuf);
#endif
	websWrite(wp, "%s.formEncrypt.radiusIP.value = '%s';\n", DOCUMENT, strbuf2);	
	websWrite(wp, "%s.formEncrypt.radiusPort.value = %d;\n", DOCUMENT, Entry.rsPort);	
	websWrite(wp, "%s.formEncrypt.radiusPass.value = '%s';\n", DOCUMENT, Entry.rsPassword);	
}

#endif

#ifdef WLAN_ACL
void initWlAclPage(webs_t wp)
{
	unsigned char vChar;
	
	mib_get( MIB_WLAN_DISABLED, (void *)&vChar);
	if (vChar==0) // enabled
		websWrite(wp,"wlanDisabled=0;\n");
	else // disabled
		websWrite(wp,"wlanDisabled=1;\n");
	
	mib_get( MIB_WLAN_MODE, (void *)&vChar);
	websWrite(wp,"wlanMode=%d;\n", vChar);
	
	mib_get( MIB_WLAN_AC_ENABLED, (void *)&vChar);
//xl_yue
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	websWrite(wp,"%s.formWlAcMode.wlanAcEnabled.selectedIndex=%d\n", DOCUMENT, vChar);
	websWrite(wp,"aclMode=%d\n",vChar);
#else
	websWrite(wp,"%s.formWlAcAdd.wlanAcEnabled.selectedIndex=%d\n", DOCUMENT, vChar);
#endif
}
#endif
#endif // of WLAN_SUPPORT

#ifdef DIAGNOSTIC_TEST
void initDiagTestPage(webs_t wp)
{
	unsigned int inf;
	FILE *fp;
	
	if (fp = fopen("/tmp/diaginf", "r")) {
		fscanf(fp, "%d", &inf);
		websWrite(wp, "%s.diagtest.wan_if.value = %d;\n", DOCUMENT, inf);
		fclose(fp);
	}
}
#endif

void ShowWmm(int eid, webs_t wp, int argc, char_t **argv)
{
#ifdef WLAN_QoS
//xl_yue
	websWrite(wp, 
	"<tr><td width=\"30%%\"><font size=2><b>WMM support:</b></td>" \
      	"<td width=\"70%%\"><font size=2>" \
      "<input type=\"radio\" name=WmmEnabled value=1>Enabled&nbsp;&nbsp;" \
      "<input type=\"radio\" name=WmmEnabled value=0>Disabled</td></tr>");
#endif
}

///////////////////////////////////////////////////////////
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
void initFcPage1(webs_t wp) {
	if ((fcEntry.vpi !=0) ||(fcEntry.vci != 0)) {
		websWrite(wp, "document.formFastConfig.vpi.value=\"%d\";\n",fcEntry.vpi);
		websWrite(wp, "document.formFastConfig.vci.value=\"%d\";\n",fcEntry.vci);
	}
}

void initFcAdslEncap(webs_t wp) {
	if (fcEntry2.encap_tmp== 0) {
		websWrite(wp, "<option selected value=\"1\">LLC/SNAP</option>\n");
		websWrite(wp, "<option value=\"0\">VC-Mux</option>");
	} else {
		if (fcEntry.encap == 0) {
			websWrite(wp, "<option selected value=\"0\">VC-Mux</option>\n");
			websWrite(wp, "<option value=\"1\">LLC/SNAP</option>");
		} else {
			websWrite(wp, "<option selected value=\"1\">LLC/SNAP</option>\n");
			websWrite(wp, "<option value=\"0\">VC-Mux</option>");
		}
	}
}

void initFcWanType(webs_t wp) {
	int idx;
#ifndef ZTE_GENERAL_ROUTER_SC
	websWrite(wp, "document.formFastConfig2.wantype.checked=true;");
    websWrite(wp, "document.formFastConfig2.wantype.focus();");
#else
	switch(fcEntry.cmode)
	{
		case ADSL_BR1483:
			idx = 4;
			break;
		case ADSL_MER1483:
			idx = 2;
			break;
		case ADSL_PPPoE:
			idx = 1;
			break;
		case ADSL_PPPoA:
			idx = 0;
			break;
		case ADSL_RT1483:
			idx = 3;
			break;
		default:
			break;
	}
	websWrite(wp, T("document.formFastConfig2.wantype[%d].checked=true;"), idx);
    websWrite(wp, T("document.formFastConfig2.wantype[%d].focus();"), idx);
#endif
}

void initLan2Dhcp(webs_t wp){
	if (fcEntry2.enable_ip2)
		websWrite(wp, T("document.formLanConf3.bakIp.checked=true;\n"));
	if (fcEntry2.enable_dhcp)
		websWrite(wp, T("document.formLanConf3.DhcpS.checked=true;\n"));
}

void initFcPage3(webs_t wp) {
	if (fcEntry2.enable_ip2) {
		websWrite(wp, T("document.formLanConf3.lan2IpAddress.value=\"%s\";\n"), 
			inet_ntoa(*((struct in_addr *)fcEntry2.lanIp2)));
		websWrite(wp, T("document.formLanConf3.lan2SubnetMask.value=\"%s\";\n"), 
			inet_ntoa(*((struct in_addr *)fcEntry2.subnetMask2)));
	}
	if (fcEntry2.enable_dhcp) {
		websWrite(wp, T("document.formLanConf3.startIp.value=\"%s\";\n"), 
			inet_ntoa(*((struct in_addr *)fcEntry2.dhcpStartIp)));
		websWrite(wp, T("document.formLanConf3.endIp.value=\"%s\";\n"), 
			inet_ntoa(*((struct in_addr *)fcEntry2.dhcpEndIp)));
		websWrite(wp, T("document.formLanConf3.time_dd.value=\"%d\";\n"), fcEntry2.day);
		websWrite(wp, T("document.formLanConf3.time_hh.value=\"%d\";\n"), fcEntry2.hour);
		websWrite(wp, T("document.formLanConf3.time_mm.value=\"%d\";\n"), fcEntry2.min);
	}
}

void initFcPage4(webs_t wp) {
	char tmpBuf[40];
	char timeBuf[100];

	if (fcEntry2.enable_dhcp) {
		strcpy(tmpBuf, inet_ntoa(*((struct in_addr *)fcEntry2.dhcpStartIp)));
		strcat(tmpBuf+strlen(tmpBuf), " ～ ");
		strcat(tmpBuf+strlen(tmpBuf), inet_ntoa(*((struct in_addr *)fcEntry2.dhcpEndIp)));

		sprintf(timeBuf, Tdd_hh_mm, fcEntry2.day, fcEntry2.hour, fcEntry2.min);
		if (fcEntry2.day == -1)
			strcat(timeBuf, " (租期不限)");
	
		websWrite(wp, T("<tr><td class='hd' bgcolor=\"#E0E0E0\"><font size=2><b>%s</b></td>"
			"<td><font size=2>%s</td></tr>"), Tdhcp_ip_range, tmpBuf);
		websWrite(wp, T("<tr><td class='hd' bgcolor=\"#E0E0E0\"><font size=2><b>%s</b></td>"
      			"<td><font size=2>%s</td></tr>"), Tdhcp_lease_t, timeBuf);
	}
}


void initFcPage4Wanconf(webs_t wp) {
	if (fcEntry.cmode != ADSL_BR1483) {
		char napt[20], wanIp[40], resGw[20], dns[40];

		if (fcEntry.napt)
			strcpy(napt, "Enabled");
		else
			strcpy(napt, "Disabled");
		
		if (fcEntry.cmode == ADSL_PPPoE || fcEntry.cmode == ADSL_PPPoA)
			strcpy(wanIp, Tauto_assign);
		else if (fcEntry.cmode == ADSL_RT1483 || fcEntry.cmode == ADSL_MER1483) {
			if ((fcEntry.ipDhcp == DHCP_CLIENT) && (fcEntry.ipunnumbered != 1))
				strcpy(wanIp, Tauto_assign);
			else if (fcEntry.ipDhcp == DHCP_DISABLED) {
				strcpy(wanIp, inet_ntoa(*((struct in_addr *)fcEntry.ipAddr)));
				strcat(wanIp, " / ");
				strcat(wanIp, inet_ntoa(*((struct in_addr *)fcEntry.netMask)));
			}
			else
				strcpy(wanIp, Tno_wan_ip);
		}
		
		if (fcEntry.cmode == ADSL_PPPoE || fcEntry.cmode == ADSL_PPPoA)
			strcpy(resGw, Tauto_assign);
		else if (fcEntry.cmode == ADSL_MER1483) {
			if (fcEntry.ipDhcp == DHCP_DISABLED)
				strcpy(resGw, inet_ntoa(*((struct in_addr *)fcEntry.remoteIpAddr)));
			else
				strcpy(resGw, Tauto_assign);
		}
		else if (fcEntry.cmode == ADSL_RT1483)
			strcpy(resGw, Tauto_assign);

		if (fcEntry.cmode == ADSL_PPPoE || fcEntry.cmode == ADSL_PPPoA)
			strcpy(dns, Tauto_assign);
		else if (fcEntry.cmode == ADSL_MER1483 || fcEntry.cmode == ADSL_RT1483) {
			if (fcEntry2.dnsMode == DNS_MANUAL) {
				strcpy(dns, inet_ntoa(*((struct in_addr *)fcEntry2.dns1)));
				if ( ((struct in_addr *)fcEntry2.dns2)->s_addr ) {
					strcat(dns, ", ");
					strcat(dns, inet_ntoa(*((struct in_addr *)fcEntry2.dns2)));
				}
			} else
				strcpy(dns, Tauto_assign);
		}
		websWrite(wp, T("<tr><td class='hd' bgcolor=\"#E0E0E0\"><font size=2><b>%s</b></td>"
			"<td><font size=2>%s</td></tr>"), Tnapt, napt);
		websWrite(wp, T("<tr><td class='hd' bgcolor=\"#E0E0E0\"><font size=2><b>%s</b></td>"
			"<td><font size=2>%s</td></tr>"), Twan_ip, wanIp);
		websWrite(wp, T("<tr><td class='hd' bgcolor=\"#E0E0E0\"><font size=2><b>%s</b></td>"
			"<td><font size=2>%s</td></tr>"), Tres_gw, resGw);
		websWrite(wp, T("<tr><td class='hd' bgcolor=\"#E0E0E0\"><font size=2><b>%s</b></td>"
			"<td><font size=2>%s</td></tr>"), Tdnss, dns);
	}
}

void initRefresh(webs_t wp)
{
	unsigned short time;

	if (!mib_get(MIB_REFRESH_TIME, (void *)&time)) {
		printf("get MIB_REFRESH_TIME failed!\n");
		return;
	}
	websWrite(wp, T("document.refresh.interval.value=\"%d\";\n"), time);
}

#endif
#ifdef ZTE_GENERAL_ROUTER_SC
void initFcPppType(webs_t wp) {
	if (fcEntry.pppUsername)
		websWrite(wp, T("document.formFcPppType.pppUserName.value=\"%s\"\n"), fcEntry.pppUsername);
	if (fcEntry.pppPassword)
		websWrite(wp, T("document.formFcPppType.pppPassword.value=\"%s\"\n"), fcEntry.pppPassword);

	websWrite(wp, T("document.formFcPppType.pppConnectType[%d].checked=true;\n"), fcEntry.pppCtype);
	if (fcEntry.pppIdleTime) {
		websWrite(wp, T("document.formFcPppType.pppIdleTime.value=\"%d\"\n"), fcEntry.pppIdleTime);
		websWrite(wp, T("document.formFcPppType.pppIdleTime2.value=\"%d\"\n"), fcEntry.pppIdleTime);
	}
}

void initFcPppWan(webs_t wp) {
	if (fcEntry.napt == 1)
		websWrite(wp, T("document.formFcPppWan.naptEnabled.checked=true;\n"));
	else
		websWrite(wp, T("document.formFcPppWan.naptEnabled.checked=false;\n"));

	if (fcEntry.dgw == 1)
		websWrite(wp, T("document.formFcPppWan.droute.checked=true;\n"));
	else
		websWrite(wp, T("document.formFcPppWan.droute.checked=false;\n"));
	
#ifdef CONFIG_SPPPD_STATICIP
	websWrite(wp, T("document.formFcPppWan.ipMode[%d].checked=true;"), fcEntry.pppIp);
#endif
}

void initFcMerWan(webs_t wp){
	if (fcEntry.ipDhcp == (char)DHCP_CLIENT)
		websWrite(wp, T("document.FcMerWan.ipmode[0].checked=true;\n"));
	else if (fcEntry.ipDhcp == (char)DHCP_DISABLED)		//fixed IP
		websWrite(wp, T("document.FcMerWan.ipmode[1].checked=true;\n"));

	if ( ((struct in_addr *)fcEntry.ipAddr)->s_addr )
		websWrite(wp, T("document.FcMerWan.ip.value=\"%s\"\n"), 
			inet_ntoa(*((struct in_addr *)fcEntry.ipAddr)));
	else
		websWrite(wp, T("document.FcMerWan.ip.value=\"%s\"\n"), "0.0.0.0");

	if ( ((struct in_addr *)fcEntry.netMask)->s_addr )
		websWrite(wp, T("document.FcMerWan.netmask.value=\"%s\"\n"),
			inet_ntoa(*((struct in_addr *)fcEntry.netMask)));
	else
		websWrite(wp, T("document.FcMerWan.netmask.value=\"%s\"\n"), "255.255.255.0");

	if ( ((struct in_addr *)fcEntry.remoteIpAddr)->s_addr )
		websWrite(wp, T("document.FcMerWan.remoteIp.value=\"%s\"\n"), 
			inet_ntoa(*((struct in_addr *)fcEntry.remoteIpAddr)));
	else
		websWrite(wp, T("document.FcMerWan.remoteIp.value=\"%s\"\n"), "0.0.0.0");
	
	if (fcEntry2.dnsMode == (char)DNS_AUTO)
		websWrite(wp, T("document.FcMerWan.dnsMode[0].checked=true;\n"));
	else if (fcEntry2.dnsMode == (char)DNS_MANUAL)		//fixed DNS
		websWrite(wp, T("document.FcMerWan.dnsMode[1].checked=true;\n"));

	if ( ((struct in_addr *)fcEntry2.dns1)->s_addr ) {
		websWrite(wp, T("document.FcMerWan.dns1.value=\"%s\"\n"), 
			inet_ntoa(*((struct in_addr *)fcEntry2.dns1)));

		if ( ((struct in_addr *)fcEntry2.dns2)->s_addr )
			websWrite(wp, T("document.FcMerWan.dns2.value=\"%s\"\n"), 
				inet_ntoa(*((struct in_addr *)fcEntry2.dns2)));		
	} else {
		websWrite(wp, T("document.FcMerWan.dns1.value=\"%s\"\n"), "0.0.0.0");
		websWrite(wp, T("document.FcMerWan.dns2.value=\"%s\"\n"), "0.0.0.0");
	}

	if (fcEntry.napt)
		websWrite(wp, T("document.FcMerWan.napt.checked=true;\n"));
	if (fcEntry.dgw)
		websWrite(wp, T("document.FcMerWan.droute.checked=true;\n"));
}

void initFcIPoA(webs_t wp) {
	if (fcEntry.ipDhcp == (char)DHCP_CLIENT && fcEntry.ipunnumbered != 1)
		websWrite(wp, T("document.FcIPoA.ipmode[1].checked=true;\n"));
	else if (fcEntry.ipDhcp == (char)DHCP_DISABLED)		//fixed IP
		websWrite(wp, T("document.FcIPoA.ipmode[2].checked=true;\n"));
	else
		websWrite(wp, T("document.FcIPoA.ipmode[0].checked=true;\n"));

	if ( ((struct in_addr *)fcEntry.ipAddr)->s_addr )
		websWrite(wp, T("document.FcIPoA.ip.value=\"%s\"\n"), 
			inet_ntoa(*((struct in_addr *)fcEntry.ipAddr)));
	else
		websWrite(wp, T("document.FcIPoA.ip.value=\"%s\"\n"), "0.0.0.0");

	if ( ((struct in_addr *)fcEntry.netMask)->s_addr )
		websWrite(wp, T("document.FcIPoA.netmask.value=\"%s\"\n"),
			inet_ntoa(*((struct in_addr *)fcEntry.netMask)));
	else
		websWrite(wp, T("document.FcIPoA.netmask.value=\"%s\"\n"), "255.255.255.0");
	
	if (fcEntry2.dnsMode == (char)DNS_AUTO)
		websWrite(wp, T("document.FcIPoA.dnsMode[0].checked=true;\n"));
	else if (fcEntry2.dnsMode == (char)DNS_MANUAL)		//fixed DNS
		websWrite(wp, T("document.FcIPoA.dnsMode[1].checked=true;\n"));

	if ( ((struct in_addr *)fcEntry2.dns1)->s_addr ) {
		websWrite(wp, T("document.FcIPoA.dns1.value=\"%s\"\n"), 
			inet_ntoa(*((struct in_addr *)fcEntry2.dns1)));

		if ( ((struct in_addr *)fcEntry2.dns2)->s_addr )
			websWrite(wp, T("document.FcIPoA.dns2.value=\"%s\"\n"), 
				inet_ntoa(*((struct in_addr *)fcEntry2.dns2)));		
	} else {
		websWrite(wp, T("document.FcIPoA.dns1.value=\"%s\"\n"), "0.0.0.0");
		websWrite(wp, T("document.FcIPoA.dns2.value=\"%s\"\n"), "0.0.0.0");
	}

	if (fcEntry.napt)
		websWrite(wp, T("document.FcIPoA.napt.checked=true;\n"));
	if (fcEntry.dgw)
		websWrite(wp, T("document.FcIPoA.droute.checked=true;\n"));

}

void initTime(webs_t wp)
{
	time_t tm;
	struct tm tm_time;
	unsigned int vInt;
	unsigned char vChar;
	char temp[100];
	
	time(&tm);
	memcpy(&tm_time, localtime(&tm), sizeof(struct tm));
	/*//patch: when current year is not in sys_year list
	if (tm_time.tm_year >= 109 || tm_time.tm_year < 104) {
		websWrite(wp, T("document.timeZone.sys_year.options[5].text=%d;\n"), tm_time.tm_year+1900);
		websWrite(wp, T("document.timeZone.sys_year.selectedIndex=5;\n"));
	} else
		websWrite(wp, T("document.timeZone.sys_year.selectedIndex=%d;\n"), tm_time.tm_year-104);

	websWrite(wp, T("document.timeZone.sys_month.selectedIndex=%d;\n"), tm_time.tm_mon);
	websWrite(wp, T("document.timeZone.sys_day.selectedIndex=%d;\n"), tm_time.tm_mday-1);
	websWrite(wp, T("document.timeZone.sys_hour.selectedIndex=%d;\n"), tm_time.tm_hour);
	websWrite(wp, T("document.timeZone.sys_minute.selectedIndex=%d;\n"), tm_time.tm_min);
	websWrite(wp, T("document.timeZone.sys_second.selectedIndex=%d;\n"), tm_time.tm_sec);*/

	sprintf(temp, "%d年%d月%d日 %d:%d:%d", tm_time.tm_year+1900, tm_time.tm_mon+1, 
		tm_time.tm_mday, tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
	websWrite(wp, T("document.all.sysClock.innerHTML=\"%s\";\n"), temp);
	
}

void initVrtSrv(webs_t wp)
{
	unsigned int totalEntry;
	
	totalEntry = mib_chain_total(MIB_VIRTUAL_SVR_TBL);
	if (totalEntry > 0)
		websWrite(wp, T("&nbsp;<input type=\"submit\" name=\"delete\" value=\"删除\">\n"));
}
#if 0
void initNtpSvr(webs_t wp)
{
	unsigned char vChar;
	
	mib_get( MIB_NTP_ENABLED, (void *)&vChar);
	if (vChar == 1)
		websWrite(wp, "%s.timeZone.enabled.checked = true;\n", DOCUMENT);
	else
		websWrite(wp, "%s.timeZone.enabled.checked = false;\n", DOCUMENT);
	
	mib_get( MIB_NTP_SERVER_ID, (void *)&vChar);
	websWrite(wp, "%s.timeZone.ntpServerId[%d].checked = true;", DOCUMENT, vChar&0x01);
}
#endif

#if 0
void showlan2(webs_t wp) {
	char *strVal;
	
	strVal = websGetVar(wp, T("bakIp"), T(""));
    	if(strVal[0]){
        	websWrite(wp, T("<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\">"
    			"<tr><td width=\"150\">第二局域网 IP 地址:</td>"
                	"<td><input type=\"text\" name=\"lan2IpAddress\" value=\"0.0.0.0\"></td></tr>"
                	"<tr><td>第二局域网子网掩码:</td>"
                	"<td><input type=\"text\" name=\"lan2SubnetMask\" value=\"0.0.0.0\"></td></tr>"
                	"</table><br>"));
      	} else {
      	}
}

void showDhcpInfo(webs_t wp) {
	char *strVal;
	
	strVal = websGetVar(wp, T("DhcpS"), T(""));
	if (strVal[0]) {
		websWrite(wp, T("<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\">"
			"<tr><td height=\"30\">起始 IP 地址:</td>"
			"<td><input type=\"text\" name=\"startIp\" value=\"192.168.1.2\"></td></tr>"
			"<tr><td height=\"30\" >终止 IP 地址:</td>"
			"<td><input type=\"text\" name=\"endIp\" value=\"192.168.1.254\"></td></tr>"
    			"<tr><td height=\"30\">租约时间:&nbsp; </td>"
			"<td height=\"30\" name=\"dhmstr\">&nbsp;"
      			"<input type=\"text\" name=\"time_dd\" size=\"1\" maxlength=\"4\" value=\"1\">&nbsp;天&nbsp;"				
      			"<input type=\"text\" name=\"time_hh\" size=\"1\" value=\"0\">&nbsp;小时&nbsp;"
      			"<input type=\"text\" name=\"time_mm\" size=\"1\" value=\"0\">&nbsp;分&nbsp;</td></tr>"
			"</table>"));
	} else {
	}
}
#endif
#endif 


#ifndef ZTE_GENERAL_ROUTER_SC //star
extern int store_day,store_hour,store_minute;
void initDhcpd(webs_t wp)
{
	unsigned char vChar;
	int time;
	int day,hour,minute;
	
	mib_get( MIB_DHCP_MODE, (void *)&vChar);
	if(vChar == DHCP_LAN_SERVER)
		vChar = 0;
	else
		vChar = 1;
#ifdef ZTE_531B_BRIDGE_SC
	mib_get(MIB_ADSL_LAN_DHCP_LEASE, (void*)&time);
	if(time == -1){
		websWrite(wp, "%s.dhcpd.time_dd.value = -1;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.time_hh.value = -1;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.time_mm.value = -1;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.time_dd.defaultValue = -1;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.time_hh.defaultValue = -1;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.time_mm.defaultValue = -1;\n", DOCUMENT);
	}
	else{
		day = time/86400;
		hour = (time%86400)/3600;
		minute = ((time%86400)%3600)/60;
		store_day = day;
		store_hour = hour;
		store_minute = minute;
		websWrite(wp, "%s.dhcpd.time_dd.value = %d;\n", DOCUMENT,day);
		websWrite(wp, "%s.dhcpd.time_hh.value = %d;\n", DOCUMENT,hour);
		websWrite(wp, "%s.dhcpd.time_mm.value = %d;\n", DOCUMENT,minute);
		websWrite(wp, "%s.dhcpd.time_dd.defaultValue = %d;\n", DOCUMENT,day);
		websWrite(wp, "%s.dhcpd.time_hh.defaultValue = %d;\n", DOCUMENT,hour);
		websWrite(wp, "%s.dhcpd.time_mm.defaultValue = %d;\n", DOCUMENT,minute);
	}
#endif
	if(vChar == 0)
	{
		websWrite(wp, "%s.dhcpd.dhcpRangeStart.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.dhcpRangeEnd.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.dhcpClientTbl.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.dname.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.ip.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.macIpTbl.disabled = false;\n", DOCUMENT);
#ifdef ZTE_531B_BRIDGE_SC
		websWrite(wp, "%s.dhcpd.time_dd.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.time_hh.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.time_mm.disabled = false;\n", DOCUMENT);

		websWrite(wp, "%s.dhcpd.dhcpdenable[0].checked = true;\n", DOCUMENT);
#else
		websWrite(wp, "%s.dhcpd.ltime.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.save.disabled = false;\n", DOCUMENT);
#endif
#ifdef IP_BASED_CLIENT_TYPE
		websWrite(wp, "%s.dhcpd.dhcppcRangeStart.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.dhcppcRangeEnd.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.dhcpcmrRangeStart.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.dhcpcmrRangeEnd.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.dhcpstbRangeStart.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.dhcpstbRangeEnd.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.dhcpphnRangeStart.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.dhcpphnRangeEnd.disabled = false;\n", DOCUMENT);
#endif

	}
	else
	{
		websWrite(wp, "%s.dhcpd.dhcpRangeStart.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.dhcpRangeEnd.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.dhcpClientTbl.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.dname.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.ip.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.macIpTbl.disabled = true;\n", DOCUMENT);
#ifdef ZTE_531B_BRIDGE_SC
		websWrite(wp, "%s.dhcpd.time_dd.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.time_hh.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.time_mm.disabled = true;\n", DOCUMENT);

		websWrite(wp, "%s.dhcpd.dhcpdenable[1].checked = true;\n", DOCUMENT);
#else
		websWrite(wp, "%s.dhcpd.ltime.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.save.disabled = true;\n", DOCUMENT);
#endif
#ifdef IP_BASED_CLIENT_TYPE
		websWrite(wp, "%s.dhcpd.dhcppcRangeStart.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.dhcppcRangeEnd.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.dhcpcmrRangeStart.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.dhcpcmrRangeEnd.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.dhcpstbRangeStart.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.dhcpstbRangeEnd.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.dhcpphnRangeStart.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.dhcpd.dhcpphnRangeEnd.disabled = true;\n", DOCUMENT);
#endif

	}
}
#ifndef ZTE_531B_BRIDGE_SC
void initDhcprelay(webs_t wp)
{
	unsigned char vChar;
	int time;
	int day,hour,minute;
	
	mib_get( MIB_DHCP_MODE, (void *)&vChar);
	if(vChar == DHCP_LAN_RELAY)
		vChar = 0;
	else
		vChar = 1;
	
	if(vChar == 0)
	{
		websWrite(wp, "%s.dhcrelay.dhcps.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.dhcrelay.save.disabled = false;\n", DOCUMENT);
	}
	else
	{
		websWrite(wp, "%s.dhcrelay.dhcps.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.dhcrelay.save.disabled = true;\n", DOCUMENT);
	}
}
#endif // of ifndef ZTE_531B_BRIDGE_SC
#endif // of ifndef ZTE_GENERAL_ROUTER_SC 
void initDhcpMode(webs_t wp)
{
	unsigned char vChar;
	char buf[16];

// Kaohj --- assign DHCP pool ip prefix; no pool prefix for complete IP pool
#ifdef DHCPS_POOL_COMPLETE_IP
	websWrite(wp, "pool_ipprefix='';\n");
#else
	getSYS2Str(SYS_DHCPS_IPPOOL_PREFIX, buf);
	websWrite(wp, "pool_ipprefix='%s';\n", buf);
#endif
// Kaohj
#ifdef DHCPS_DNS_OPTIONS
	websWrite(wp, "en_dnsopt=1;\n");
	mib_get(MIB_DHCP_DNS_OPTION, (void *)&vChar);
	websWrite(wp, "dnsopt=%d;\n", vChar);
#else
	websWrite(wp, "en_dnsopt=0;\n");
#endif
	mib_get( MIB_DHCP_MODE, (void *)&vChar);
	websWrite(wp, "%s.dhcpd.dhcpdenable[%d].checked = true;\n", DOCUMENT, vChar);
}

void initDhcpMacbase(webs_t wp)
{
	char buf[16];
// Kaohj --- assign DHCP pool ip prefix; no pool prefix for complete IP pool
#ifdef DHCPS_POOL_COMPLETE_IP
	websWrite(wp, "pool_ipprefix='';\n");
#else
	getSYS2Str(SYS_DHCPS_IPPOOL_PREFIX, buf);
	websWrite(wp, "pool_ipprefix='%s';\n", buf);
#endif
}

#ifdef ADDRESS_MAPPING
#ifdef MULTI_ADDRESS_MAPPING
initAddressMap(webs_t wp)
{
	websWrite(wp, "%s.addressMap.lsip.disabled = false;\n", DOCUMENT);
	websWrite(wp, "%s.addressMap.leip.disabled = true;\n", DOCUMENT);
	websWrite(wp, "%s.addressMap.gsip.disabled = false;\n", DOCUMENT);
	websWrite(wp, "%s.addressMap.geip.disabled = true;\n", DOCUMENT);
	websWrite(wp, "%s.addressMap.addressMapType.selectedIndex= 0;\n", DOCUMENT);
}
#else 
initAddressMap(webs_t wp)
{
	unsigned char vChar;
	
	mib_get( MIB_ADDRESS_MAP_TYPE, (void *)&vChar);
	
	if(vChar == ADSMAP_NONE) {         // None
		websWrite(wp, "%s.addressMap.lsip.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.addressMap.leip.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.addressMap.gsip.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.addressMap.geip.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.addressMap.addressMapType.selectedIndex= 0;\n", DOCUMENT);
		
	} else if (vChar == ADSMAP_ONE_TO_ONE) {  // One-to-One
		websWrite(wp, "%s.addressMap.lsip.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.addressMap.leip.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.addressMap.gsip.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.addressMap.geip.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.addressMap.addressMapType.selectedIndex= 1;\n", DOCUMENT);
				
	} else if (vChar == ADSMAP_MANY_TO_ONE) {  // Many-to-One
		websWrite(wp, "%s.addressMap.lsip.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.addressMap.leip.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.addressMap.gsip.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.addressMap.geip.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.addressMap.addressMapType.selectedIndex= 2;\n", DOCUMENT);
		
	} else if (vChar == ADSMAP_MANY_TO_MANY) {   // Many-to-Many
		websWrite(wp, "%s.addressMap.lsip.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.addressMap.leip.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.addressMap.gsip.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.addressMap.geip.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.addressMap.addressMapType.selectedIndex= 3;\n", DOCUMENT);
		
	} 
	// Masu Yu on True
#if 1
	else if (vChar == ADSMAP_ONE_TO_MANY) {   // One-to-Many
		websWrite(wp, "%s.addressMap.lsip.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.addressMap.leip.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.addressMap.gsip.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.addressMap.geip.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.addressMap.addressMapType.selectedIndex= 4;\n", DOCUMENT);
		
	}
#endif	
}
#endif // MULTI_ADDRESS_MAPPING
#endif

#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
#ifdef IP_ACL
extern int aclapplyflag;
void initAcl(webs_t wp)
{
	unsigned char vChar;
	
	mib_get( MIB_ACL_CAPABILITY, (void *)&vChar);
	if(vChar == 0)//disable
	{
#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)	
#ifdef MAC_ACL
		websWrite(wp, "%s.acl.updateMacACL.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.acl.delMac.disabled = true;\n", DOCUMENT);
#endif
		websWrite(wp, "%s.acl.updateACL.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.acl.delIP.disabled = true;\n", DOCUMENT);
#else
		websWrite(wp, "%s.getElementById(\"acltable\").style.display = \"none\";\n", DOCUMENT);
		websWrite(wp, "%s.acl.enable.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.acl.interface.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.acl.aclIP.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.acl.addIP.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.acl.updateACL.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.acl.delIP.disabled = true;\n", DOCUMENT);
#endif
	}
	else//enable
	{
#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)		
		websWrite(wp, "%s.acl.updateACL.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.acl.delIP.disabled = true;\n", DOCUMENT);	
#ifdef MAC_ACL
		websWrite(wp, "%s.acl.updateMacACL.disabled = true;\n", DOCUMENT);
		websWrite(wp, "%s.acl.delMac.disabled = true;\n", DOCUMENT);
#endif
#else
		websWrite(wp, "%s.getElementById(\"acltable\").style.display = \"block\";\n", DOCUMENT);
		websWrite(wp, "%s.acl.enable.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.acl.interface.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.acl.aclIP.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.acl.addIP.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.acl.updateACL.disabled = false;\n", DOCUMENT);
		websWrite(wp, "%s.acl.delIP.disabled = false;\n", DOCUMENT);
#endif
		

	}
	if(aclapplyflag == 1)
	{
		websWrite(wp, "%s.getElementById(\"applymsg\").style.display = \"block\";\n", DOCUMENT);
	}
	else
	{
		websWrite(wp, "%s.getElementById(\"applymsg\").style.display = \"none\";\n", DOCUMENT);
	}	

}
#endif

//ql_xu add
#ifdef NAT_CONN_LIMIT
extern int connlimitflag;
void initConnLimit(webs_t wp)
{
	unsigned char vChar;

	websWrite(wp, "%s.connlimit.update.disabled = true;\n", DOCUMENT);
	websWrite(wp, "%s.connlimit.del.disabled = true;\n", DOCUMENT);

	if (connlimitflag == 1)
		websWrite(wp, "%s.getElementById(\"connlimitapply\").style.display = \"block\";\n", DOCUMENT);
	else
		websWrite(wp, "%s.getElementById(\"connlimitapply\").style.display = \"none\";\n", DOCUMENT);
}
#endif
#endif

#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
//ql
void initOspf(webs_t wp)
{
	unsigned char vChar;
	
#ifdef CONFIG_USER_ROUTED_ROUTED
	mib_get( MIB_RIP_ENABLE, (void *)&vChar );
	if (1 == vChar) {
		websWrite(wp, "%s.rip.igp.selectedIndex = 0;\n", DOCUMENT);
		websWrite(wp, "%s.rip.rip_on[1].checked = true;\n", DOCUMENT);
		return;
	}
	mib_get( MIB_OSPF_ENABLE, (void *)&vChar);
	if (1 == vChar) {
		websWrite(wp, "%s.rip.igp.selectedIndex = 1;\n", DOCUMENT);
		websWrite(wp, "%s.rip.rip_on[1].checked = true;\n", DOCUMENT);
		return;
	}
#else
	mib_get( MIB_OSPF_ENABLE, (void *)&vChar);
	if (1 == vChar) {
		websWrite(wp, "%s.rip.igp.selectedIndex = 0;\n", DOCUMENT);
		websWrite(wp, "%s.rip.rip_on[1].checked = true;\n", DOCUMENT);
		return;
	}
#endif
	//default
	websWrite(wp, "%s.rip.igp.selectedIndex = 0;\n", DOCUMENT);
	websWrite(wp, "%s.rip.rip_on[0].checked = true;\n", DOCUMENT);
}
#endif

/////////////////////////////////////////////////////////////
// Kaohj
int initPage(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t *name;
	
	if (ejArgs(argc, argv, T("%s"), &name) < 1) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}
	
	if ( !strcmp(name, T("lan")) )
		initLanPage(wp);
	if ( !strcmp(name, T("setdsl")) )
		initSetDslPage(wp);
	if ( !strcmp(name, T("diagdsl")) )
		initDiagDslPage(wp);
#ifdef PORT_TRIGGERING
	if ( !strcmp(name, T("gaming")) )
		initGamingPage(wp);
#endif
#ifdef CONFIG_USER_ROUTED_ROUTED
	if ( !strcmp(name, T("rip")) )
		initRipPage(wp);
#endif
#ifdef CONFIG_RTL867X_COMBO_PORTMAPPING
	if ( !strcmp(name, T("portMap")) )
		initPortMapPage(wp);
#endif
#ifdef CONFIG_EXT_SWITCH
#if 0
	if ( !strcmp(name, T("mpmode")) )
		initMpModePage(wp);
#endif
#ifdef ITF_GROUP
	if ( !strcmp(name, T("portMap")) )
		initPortMapPage(wp);
#endif
#ifdef VLAN_GROUP
	if ( !strcmp(name, T("vlanGroup")) )
		initVlanGroupPage(wp);
#endif
#if 0
	if ( !strcmp(name, T("vlan")) )
		initVlanPage(wp);
#endif
#ifdef ELAN_LINK_MODE
	if ( !strcmp(name, T("linkMode")) )
		initLinkPage(wp);
#endif
#else
#ifdef ELAN_LINK_MODE_INTRENAL_PHY
	if ( !strcmp(name, T("linkMode")) )
		initLinkPage(wp);
#endif
#endif	// of CONFIG_EXT_SWITCH
#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
	if ( !strcmp(name, T("ipqos")) )
		initIpQosPage(wp);
#endif
#ifdef QOS_DIFFSERV
	if (!strcmp(name, T("diffserv")))
		initDiffservPage(wp);
#endif
	if ( !strcmp(name, T("others")) )
		initOthersPage(wp);
	if ( !strcmp(name, T("reboot")) )
		initRebootPage(wp);
#ifdef TIME_ZONE
	if ( !strcmp(name, T("ntp")) )
		initNtpPage(wp);
#endif
#ifdef WLAN_SUPPORT
	if ( !strcmp(name, T("wlwep")) )
		initWlWepPage(wp);
#ifdef WLAN_WPA
	if ( !strcmp(name, T("wlwpa")) )
		initWlWpaPage(wp);
#endif	
#ifdef WLAN_MBSSID
	if ( !strcmp(name, T("wlwpa_mbssid")) )
		initWlWpaMbssidPage(wp);
#endif
	if ( !strcmp(name, T("wlbasic")) )
		initWlBasicPage(wp);
	if ( !strcmp(name, T("wladv")) )
		initWlAdvPage(wp);
#ifdef WLAN_MBSSID
	// Mason Yu
	if ( !strcmp(name, T("wlmbssid")) ) {		
		initWLMBSSIDPage(wp);
	}
#endif
#ifdef WLAN_WDS
	if ( !strcmp(name, T("wlwds")) )
		initWlWDSPage(wp);
#endif
#ifdef WLAN_CLIENT
	if ( !strcmp(name, T("wlsurvey")) )
		initWlSurveyPage(wp);

#endif
#ifdef WLAN_ACL
	if ( !strcmp(name, T("wlactrl")) )
		initWlAclPage(wp);
#endif
#endif
	
#ifdef DIAGNOSTIC_TEST
	if ( !strcmp(name, T("diagTest")) )
		initDiagTestPage(wp);
#endif

#ifdef DOS_SUPPORT
	if ( !strcmp(name, T("dos")) )
		initDosPage(wp);
#endif


#ifndef ZTE_GENERAL_ROUTER_SC
	if( !strcmp(name, T("dhcpd")))
		initDhcpd(wp);
#ifndef ZTE_531B_BRIDGE_SC
	if( !strcmp(name, T("dhcprelay")))
		initDhcprelay(wp);
#endif
#endif

#ifdef ADDRESS_MAPPING
	if( !strcmp(name, T("addressMap")))
		initAddressMap(wp);
#endif

	if ( !strcmp(name, T("dhcp-mode")) )
		initDhcpMode(wp);
	if ( !strcmp(name, T("dhcp-macbase")) )
		initDhcpMacbase(wp);
#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
#ifdef IP_ACL
	if( !strcmp(name, T("acl")))
		initAcl(wp);
#endif
#ifdef NAT_CONN_LIMIT
	if (!strcmp(name, T("connlimit")))
		initConnLimit(wp);
#endif
#endif
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	if (!strcmp(name, T("fc-page1")))
		initFcPage1(wp);

	if (!strcmp(name, T("fc-adslEncap")))
		initFcAdslEncap(wp);

	//if (!strcmp(name, T("lanip2")))
	//	showlan2(wp);

	//if (!strcmp(name, T("dhcp")))
	//	showDhcpInfo(wp);

	if (!strcmp(name, T("lan2-dhcp")))
		initLan2Dhcp(wp);
	
	if (!strcmp(name, T("fc-page3")))
		initFcPage3(wp);

	if (!strcmp(name, T("fc-page4")))
		initFcPage4(wp);
	
	if (!strcmp(name, T("wanconf")))
		initFcPage4Wanconf(wp);

	if (!strcmp(name, T("wantype")))
		initFcWanType(wp);

	if (!strcmp(name, T("refresh")))
		initRefresh(wp);
#endif
#ifdef ZTE_GENERAL_ROUTER_SC
	if (!strcmp(name, T("pppIdleTime")))
		initFcPppType(wp);

	if (!strcmp(name, T("mer-wan")))
		initFcMerWan(wp);

	if (!strcmp(name, T("ipoa")))
		initFcIPoA(wp);

	if (!strcmp(name, T("timeZone")))
		initTime(wp);

	if (!strcmp(name, T("vrtSrv")))
		initVrtSrv(wp);

	if (!strcmp(name, T("napt")))
		initFcPppWan(wp);

	//if (!strcmp(name, T("ntpSvr")))
	//	initNtpSvr(wp);
#endif
//add by ramen
#ifdef CONFIG_IP_NF_ALG_ONOFF
if (!strcmp(name, T("algonoff")))
	initAlgOnOff(wp);
#endif
#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
	if (!strcmp(name, T("ospf")))
		initOspf(wp);
#endif
	if (!strcmp(name, T("qconnect")))
		initProcessPPP(wp);
	if (!strcmp(name, T("syslog")))
		initSyslogPage(wp);
#ifdef WEB_ENABLE_PPP_DEBUG
	if ( !strcmp(name, T("pppSyslog")) )
		initPPPSyslog(wp);
#endif
	if (!strcmp(name, T("dgw")))
		initDgwPage(wp);
	return 0;
}

#ifdef DOS_SUPPORT
#define DOSENABLE	0x1
#define DOSSYSFLOODSYN	0x2
#define DOSSYSFLOODFIN	0x4
#define	DOSSYSFLOODUDP	0x8
#define DOSSYSFLOODICMP	0x10
#define DOSIPFLOODSYN	0x20
#define DOSIPFLOODFIN	0x40
#define DOSIPFLOODUDP	0x80
#define DOSIPFLOODICMP	0x100
#define DOSTCPUDPPORTSCAN 0x200
#define DOSPORTSCANSENSI  0x800000
#define DOSICMPSMURFENABLED	0x400
#define DOSIPLANDENABLED	0x800
#define DOSIPSPOOFENABLED	0x1000
#define DOSIPTEARDROPENABLED	0x2000
#define DOSPINTOFDEATHENABLED	0x4000
#define DOSTCPSCANENABLED	0x8000
#define DOSTCPSYNWITHDATAENABLED	0x10000
#define DOSUDPBOMBENABLED		0x20000
#define DOSUDPECHOCHARGENENABLED	0x40000
#define DOSSOURCEIPBLOCK		0x400000
void initDosPage(webs_t wp){
	unsigned int mode;
	
	mib_get( MIB_DOS_ENABLED, (void *)&mode);
	if (mode & DOSENABLE){
		websWrite(wp, "%s.DosCfg.dosEnabled.checked = true;\n", DOCUMENT);
		if (mode & DOSSYSFLOODSYN)
			websWrite(wp, "%s.DosCfg.sysfloodSYN.checked = true;\n", DOCUMENT);
		if (mode & DOSSYSFLOODFIN)
			websWrite(wp, "%s.DosCfg.sysfloodFIN.checked = true;\n", DOCUMENT);
		if (mode & DOSSYSFLOODUDP)
			websWrite(wp, "%s.DosCfg.sysfloodUDP.checked = true;\n", DOCUMENT);
		if (mode & DOSSYSFLOODICMP)
			websWrite(wp, "%s.DosCfg.sysfloodICMP.checked = true;\n", DOCUMENT);
		if (mode & DOSIPFLOODSYN)
			websWrite(wp, "%s.DosCfg.ipfloodSYN.checked = true;\n", DOCUMENT);
		if (mode & DOSIPFLOODFIN)
			websWrite(wp, "%s.DosCfg.ipfloodFIN.checked = true;\n", DOCUMENT);
		if (mode & DOSIPFLOODUDP)
			websWrite(wp, "%s.DosCfg.ipfloodUDP.checked = true;\n", DOCUMENT);
		if (mode & DOSIPFLOODICMP)
			websWrite(wp, "%s.DosCfg.ipfloodICMP.checked = true;\n", DOCUMENT);
		if (mode & DOSTCPUDPPORTSCAN)
			websWrite(wp, "%s.DosCfg.TCPUDPPortScan.checked = true;\n", DOCUMENT);
		if (mode & DOSPORTSCANSENSI)
			websWrite(wp, "%s.DosCfg.portscanSensi.value = 1;\n", DOCUMENT);
		if (mode & DOSICMPSMURFENABLED)
			websWrite(wp, "%s.DosCfg.ICMPSmurfEnabled.checked = true;\n", DOCUMENT);
		if (mode & DOSIPLANDENABLED)
			websWrite(wp, "%s.DosCfg.IPLandEnabled.checked = true;\n", DOCUMENT);
		if (mode & DOSIPSPOOFENABLED)
			websWrite(wp, "%s.DosCfg.IPSpoofEnabled.checked = true;\n", DOCUMENT);
		if (mode & DOSIPTEARDROPENABLED)
			websWrite(wp, "%s.DosCfg.IPTearDropEnabled.checked = true;\n", DOCUMENT);
		if (mode & DOSPINTOFDEATHENABLED)
			websWrite(wp, "%s.DosCfg.PingOfDeathEnabled.checked = true;\n", DOCUMENT);
		if (mode & DOSTCPSCANENABLED)
			websWrite(wp, "%s.DosCfg.TCPScanEnabled.checked = true;\n", DOCUMENT);
		if (mode & DOSTCPSYNWITHDATAENABLED)
			websWrite(wp, "%s.DosCfg.TCPSynWithDataEnabled.checked = true;\n", DOCUMENT);
		if (mode & DOSUDPBOMBENABLED)
			websWrite(wp, "%s.DosCfg.UDPBombEnabled.checked = true;\n", DOCUMENT);
		if (mode & DOSUDPECHOCHARGENENABLED)
			websWrite(wp, "%s.DosCfg.UDPEchoChargenEnabled.checked = true;\n", DOCUMENT);
		if (mode & DOSSOURCEIPBLOCK)
			websWrite(wp, "%s.DosCfg.sourceIPblock.checked = true;\n", DOCUMENT);
		
	}
}
#endif

#ifdef PPP_QUICK_CONNECT
#include "debug.h"
void initProcessPPP(webs_t wp){
	MIB_CE_ATM_VC_T Entry;
	unsigned int totalEntry;
	int i;

	totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<totalEntry; i++) {
		struct data_to_pass_st msg;
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) {
  			printf(T(strGetChainerror));
			return;
		}
		if(Entry.enable == 0)
			continue;
		if (Entry.cmode == ADSL_PPPoE) {
			snprintf(msg.data, BUF_SIZE, "spppctl qconnect %u username %s password %s", PPP_INDEX(Entry.ifIndex), Entry.pppUsername, Entry.pppPassword);
			TRACE(STA_SCRIPT, "%s\n", msg.data);
			write_to_pppd(&msg);
		}
		else if (Entry.cmode == ADSL_PPPoA) {
			stopConnection(&Entry);
			startConnection(&Entry);
		}
	}
}
#else
#include <semaphore.h>
extern sem_t semSave;
void initProcessPPP(webs_t wp){
#ifdef EMBED
	printf("initProcessPPP: Save and reboot the system\n");
	sem_post(&semSave);
#endif
	return;
}
#endif

void initSyslogPage(webs_t wp)
{
	unsigned char vChar;

	mib_get(MIB_SYSLOG, (void *)&vChar);
	websWrite(wp, "changelogstatus();");
}

void initDgwPage(webs_t wp)
{
#ifdef DEFAULT_GATEWAY_V2
	unsigned char dgw, dgwip[16];
	mib_get(MIB_ADSL_WAN_DGW_ITF, (void *)&dgw);
	getMIB2Str(MIB_ADSL_WAN_DGW_IP, dgwip);
	websWrite(wp, "\tdgwstatus = %d;\n", dgw);
	websWrite(wp, "\tgtwy = '%s';\n", dgwip);
#endif
}

#ifdef WEB_ENABLE_PPP_DEBUG
void initPPPSyslog(webs_t wp)
{
	int enable = 0;
	FILE *fp;

	if (fp = fopen(PPP_SYSLOG, "r")) {
		fscanf(fp, "%d", &enable);
		fclose(fp);
	}
	websWrite(wp, "%s.formSysLog.pppcap[%d].checked = true;\n", DOCUMENT, enable);
}
#endif

