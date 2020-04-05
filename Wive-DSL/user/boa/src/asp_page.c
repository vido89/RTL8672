#include "asp_page.h"
#include "port.h" // ANDREW
#ifdef SUPPORT_ASP
#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../uClibc/include/linux/autoconf.h"
#endif
#include <stdio.h>
#include "boa.h"
//ANDREW #include "rtl865x/rome_asp.c"

asp_name_t *root_asp=NULL;
form_name_t *root_form=NULL;
temp_mem_t root_temp;
char *query_temp_var=NULL;

//ANDREW int websAspDefine(char* name, int (*fn)(request * req,  int argc, char **argv))
int websAspDefine(const char* const name, int (*fn)(int eid, request * req,  int argc, char **argv))
{
	asp_name_t *asp;
	asp=(asp_name_t*)malloc(sizeof(asp_name_t));
	if(asp==NULL) return ERROR;
	/*
	asp->pagename=(char *)malloc(strlen(name)+1);
	strcpy(asp->pagename,name);
	*/
	asp->pagename = name; // andrew, use the constant directly
	
	asp->next=NULL;
	asp->function=fn;

	
	if(root_asp==NULL) 
		root_asp=asp;
	else
	{
		//modify the previous's next pointer
		asp_name_t *now_asp;
		now_asp=root_asp;
		while(now_asp->next!=NULL) now_asp=now_asp->next;
		now_asp->next=asp;
	}		
	return SUCCESS;	
}

#ifdef MULTI_USER_PRIV
typedef enum {PRIV_USER=0, PRIV_ENG=1, PRIV_ROOT=2};	// copy from mib.h
struct formPrivilege formPrivilegeList[] = {
	{T("formTcpipLanSetup"), PRIV_ROOT},
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	{T("formRefresh"), PRIV_USER},
#endif
#ifdef _CWMP_MIB_
	{T("formTR069Config"), PRIV_ROOT},
	{T("formTR069CPECert"), PRIV_ROOT},
	{T("formTR069CACert"), PRIV_ROOT},
#endif
	{T("formPortFw"), PRIV_ROOT},
#ifdef NATIP_FORWARDING
	{T("formIPFw"), PRIV_ROOT},
#endif
#ifdef PORT_TRIGGERING
	{T("formGaming"), PRIV_ROOT},
#endif
	{T("formFilter"), PRIV_ROOT},
#ifdef LAYER7_FILTER_SUPPORT
	{T("formlayer7"), PRIV_ROOT},
#endif
#ifdef PARENTAL_CTRL
	{T("formParentCtrl"), PRIV_ROOT},
#endif
#ifdef ZTE_GENERAL_ROUTER_SC
	{T("formVrtsrv"), PRIV_ROOT},
	{T("formAddVrtsrv"), PRIV_ROOT},
#endif
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	{T("formFastConf"), PRIV_USER},
	{T("formFastConf2"), PRIV_USER},
	{T("formFastConf3"), PRIV_USER},
	{T("formFastConf4"), PRIV_USER},
	{T("formFcPppWan"), PRIV_USER},
	{T("formFcPppType"), PRIV_USER},
	{T("formFcMerWan"), PRIV_USER},
	{T("formFcIPoA"), PRIV_USER},
#endif
	{T("formDMZ"), PRIV_ROOT},
	{T("formPasswordSetup"), PRIV_ROOT},
	{T("formUserPasswordSetup"), PRIV_USER},
#ifdef ACCOUNT_CONFIG
	{T("formAccountConfig"), PRIV_ROOT},
#endif
#ifdef WEB_UPGRADE
	{T("formUpload"), PRIV_ENG},
#endif
	{T("formSaveConfig"), PRIV_ENG},
	{T("formSnmpConfig"), PRIV_ROOT},
	{T("formSetAdsl"), PRIV_ROOT},
	{T("formStatAdsl"), PRIV_USER},
	{T("formDiagAdsl"), PRIV_ROOT},
	{T("formStats"), PRIV_USER},
	{T("formRconfig"), PRIV_ROOT},
	{T("formSysCmd"), PRIV_ROOT},
#ifdef SYSLOG_SUPPORT
	{T("formSysLog"), PRIV_USER},
#endif
	{T("formSetAdslTone"), PRIV_ROOT},
	{T("formStatus"), PRIV_USER},
	{T("formStatusModify"), PRIV_ROOT},
#ifdef ZTE_GENERAL_ROUTER_SC
	{T("formTimezone"), PRIV_ROOT},
#endif
	{T("formWanAtm"), PRIV_ROOT},
	{T("formWanAdsl"), PRIV_ENG},
	{T("formPPPEdit"), PRIV_ENG},
	{T("formIPEdit"), PRIV_ENG},
	{T("formBrEdit"), PRIV_ENG},
	{T("formBridging"), PRIV_ROOT},
	{T("formRefleshFdbTbl"), PRIV_USER},
	{T("formRouting"), PRIV_ENG},
	{T("formRefleshRouteTbl"), PRIV_USER},
	{T("formDhcpServer"), PRIV_ROOT},
	{T("formReflashClientTbl"), PRIV_USER},
	{T("formDNS"), PRIV_ROOT},
#ifdef CONFIG_USER_DDNS	
	{T("formDDNS"), PRIV_USER},
#endif
	{T("formDhcrelay"), PRIV_ROOT},
	{T("formPing"), PRIV_USER},
	{T("formOAMLB"), PRIV_USER},
#ifdef ADDRESS_MAPPING	
	{T("formAddressMap"), PRIV_ROOT},
#endif
#ifdef FINISH_MAINTENANCE_SUPPORT
	{T("formFinishMaintenance"), PRIV_ROOT},
#endif
#ifdef ACCOUNT_LOGIN_CONTROL
	{T("formAdminLogout"), PRIV_ROOT},
	{T("formUserLogout"), PRIV_USER},
#endif
#ifdef USE_LOGINWEB_OF_SERVER
	{T("formLogin"), PRIV_USER},
	{T("formLogout"), PRIV_USER},
#endif
	{T("formReboot"), PRIV_USER},
	{T("formDhcpMode"), PRIV_ROOT},
#ifdef CONFIG_USER_IGMPPROXY
	{T("formIgmproxy"), PRIV_ROOT},
#endif
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
	{T("formUpnp"), PRIV_ROOT},
#endif
#if defined(CONFIG_USER_ROUTED_ROUTED) || defined(CONFIG_USER_ZEBRA_OSPFD_OSPFD)
	{T("formRip"), PRIV_ROOT},
#endif
#ifdef IP_ACL
	{T("formACL"), PRIV_ROOT},
#endif
#ifdef NAT_CONN_LIMIT
	{T("formConnlimit"), PRIV_ROOT},
#endif
	{T("formmacBase"), PRIV_ROOT},
#ifdef URL_BLOCKING_SUPPORT	
	{T("formURL"), PRIV_ROOT},
#endif
#ifdef DOMAIN_BLOCKING_SUPPORT
	{T("formDOMAINBLK"), PRIV_ROOT},
#endif
#ifdef TIME_ZONE
	{T("formNtp"), PRIV_ROOT},
#endif
	{T("formOthers"), PRIV_ROOT},
#ifdef AUTO_PROVISIONING
	{T("formAutoProvision"), PRIV_ROOT},
#endif
#ifdef CONFIG_EXT_SWITCH
#ifdef ITF_GROUP
	{T("formItfGroup"), PRIV_ROOT},
#endif
#ifdef ELAN_LINK_MODE
	{T("formLinkMode"), PRIV_ROOT},
#endif
#else // of CONFIG_EXT_SWITCH 
#ifdef ELAN_LINK_MODE_INTRENAL_PHY
	{T("formLinkMode"), PRIV_ROOT},
#endif
#endif	// of CONFIG_EXT_SWITCH
#ifdef IP_QOS
	{T("formIPQoS"), PRIV_ROOT},
#endif
	{T("formSAC"), PRIV_ROOT},
#ifdef CONFIG_GUI_WEB
	{T("formPPPAuth"), PRIV_USER},
#endif
#ifdef WLAN_SUPPORT
	{T("formWlanSetup"), PRIV_ROOT},
	{T("formWep"), PRIV_ROOT},
	{T("formWirelessTbl"), PRIV_USER},
#ifdef WLAN_ACL
	{T("formWlAc"), PRIV_ROOT},
#endif
	{T("formAdvanceSetup"), PRIV_ROOT},
#ifdef WLAN_WPA
	{T("formWlEncrypt"), PRIV_ROOT},
#endif
#ifdef WLAN_WDS
	{T("formWlWds"), PRIV_ROOT},
#endif
#ifdef WLAN_CLIENT
	{T("formWlSiteSurvey"), PRIV_ROOT},
#endif
#ifdef CONFIG_WIFI_SIMPLE_CONFIG
	{T("formWsc"), PRIV_ROOT},
#endif
#endif	// of WLAN_SUPPORT
#ifdef DIAGNOSTIC_TEST
	{T("formDiagTest"), PRIV_USER},
#endif
#ifdef DOS_SUPPORT
	{T("formDosCfg"), PRIV_ROOT},
#endif
#ifdef WLAN_MBSSID
	{T("formWlanMBSSID"), PRIV_ROOT},
#endif
#ifdef CONFIG_IP_NF_ALG_ONOFF
	{T("formALGOnOff"), PRIV_ROOT},
#endif
	{"\0", 0}
};

unsigned char getPagePriv(const char* const name)
{
	struct formPrivilege * pformPrivilege = formPrivilegeList;
	while(strlen(pformPrivilege->pagename)) {
		if (strcmp(pformPrivilege->pagename, name) == 0)
			return pformPrivilege->privilege;
		pformPrivilege ++;
	}
	return PRIV_ROOT;
}
#endif

int websFormDefine(const char* const name, void (*fn)(request * req,  char* path, char* query))
{

	form_name_t *form;
	form=(form_name_t *)malloc(sizeof(form_name_t));
	if(form==NULL) return ERROR;
	/*
	form->pagename=(char *)malloc(strlen(name)+1);
	strcpy(form->pagename,name);
	*/
	form->pagename=name;// andrew, use the constant directly
#ifdef MULTI_USER_PRIV
//	form->privilege = priv;
	form->privilege = getPagePriv(form->pagename);
#endif
	form->next=NULL;
	form->function=fn;
	
	if(root_form==NULL) 
		root_form=form;
	else
	{
		//modify the previous's next pointer
		form_name_t *now_form;
		now_form=root_form;
		while(now_form->next!=NULL) now_form=now_form->next;
		now_form->next=form;
	}		
//	printf("websFormDefine: form->pagename=%s form->privilege=%d\n", form->pagename, form->privilege);
	return SUCCESS;	

}

int addTempStr(char *str)
{
	temp_mem_t *temp,*newtemp;
	temp=&root_temp;
	while(temp->next)
	{
		temp=temp->next;
	}
	newtemp=(temp_mem_t *)malloc(sizeof(temp_mem_t));
	if(newtemp==NULL) return FAILED;
	newtemp->str=str;
	newtemp->next=NULL;
	temp->next=newtemp;	
	return SUCCESS;
}

void freeAllTempStr(void)
{
	temp_mem_t *temp,*ntemp;
	temp=root_temp.next;
	root_temp.next=NULL;
	while(1)
	{
		if(temp==NULL) break;
		ntemp=temp->next;
		free(temp->str);
		free(temp);
		temp=ntemp;
	}
}	

int getcgiparam(char *dst,char *query_string,char *param,int maxlen)
{
	int len,plen;
	int y;

	plen=strlen(param);
	while (*query_string)
	{
		len=strlen(query_string);

		if ((len=strlen(query_string))>plen)
			if (!strncmp(query_string,param,plen))
				if (query_string[plen]=='=')
				{//copy parameter
					query_string+=plen+1;
					y=0;
					while ((*query_string)&&(*query_string!='&'))
					{	
						if ((*query_string=='%')&&(strlen(query_string)>2))
							if ((isxdigit(query_string[1]))&&(isxdigit(query_string[2])))
							{
								if (y<maxlen)
									dst[y++]=((toupper(query_string[1])>='A'?toupper(query_string[1])-'A'+0xa:toupper(query_string[1])-'0') << 4)
									+ (toupper(query_string[2])>='A'?toupper(query_string[2])-'A'+0xa:toupper(query_string[2])-'0');
								query_string+=3;
								continue;
							}
						if (*query_string=='+')
						{
							if (y<maxlen)
								dst[y++]=' ';
							query_string++;
							continue;
						}
						if (y<maxlen)
							dst[y++]=*query_string;
						query_string++;
					}
					if (y<maxlen) dst[y]=0;
					return y;
				}
		while ((*query_string)&&(*query_string!='&')) query_string++;
		query_string++;
	}
	if (maxlen) dst[0]=0;
	return -1;
}


char *websGetVar(request *req, char *var, char *defaultGetValue)
{
	char *buf;
	struct stat statbuf;
	int ret=-1;
	
	if (req->method == M_POST)
	{
		int i;
		fstat(req->post_data_fd, &statbuf);
		buf=(char *)malloc(statbuf.st_size+1);
		if(buf==NULL) return defaultGetValue;
		lseek(req->post_data_fd, SEEK_SET, 0);
		read(req->post_data_fd,buf,statbuf.st_size);
		buf[statbuf.st_size]=0;
		i=statbuf.st_size - 1;
		while(i > 0)
		{
			if((buf[i]==0x0a)||(buf[i]==0x0d)) buf[i]=0;
			else break;
			i--;
		}
	}
	else
	{	
		buf=req->query_string;
	}

	if(buf!=NULL)
	{
		ret=getcgiparam(query_temp_var,buf,var,MAX_QUERY_TEMP_VAL_SIZE);	
	}
	
	if(req->method == M_POST) free(buf);
	//printf("query str=%s ret=%d var=%s query_temp_var=%s\n",buf,ret,var,query_temp_var);
	if(ret<0) return defaultGetValue;
	buf=(char *)malloc(ret+1);
	memcpy(buf,query_temp_var,ret);
	buf[ret]=0;
	addTempStr(buf);
	return (char *)buf;	//this buffer will be free by freeAllTempStr().

}

unsigned int __flash_base;
unsigned int __crmr_addr;

//ANDREW int32 board_cfgmgr_init(uint8 syncAll);

void websAspInit(int argc,char **argv)
{
   rtl8670_AspInit();
   query_temp_var=(char *)malloc(MAX_QUERY_TEMP_VAL_SIZE);
	if(query_temp_var==NULL) exit(0);
   
#ifdef ANDREW
	int shmid=0;
#ifndef __uClinux__
	__flash_base=rtl865x_mmap(0x1e000000,0x1000000); /* 0xbe000000~0xbeffffff */
	__crmr_addr=rtl865x_mmap(0x1c805000,0x1000)+0x0104; /* 0xbc805104 */
	printf("__flash_base=0x%08x\n",__flash_base);
	printf("__crmr_addr=0x%08x\n",__crmr_addr);
#endif
	shmid=shmget(SHM_PROMECFGPARAM,sizeof(romeCfgParam_t),0666|IPC_CREAT);
	pRomeCfgParam=shmat(shmid,(void*)0,0);

/*
 *	Initialize the memory allocator. Allow use of malloc and start 
 *	with a 60K heap.  For each page request approx 8KB is allocated.
 *	60KB allows for several concurrent page requests.  If more space
 *	is required, malloc will be used for the overflow.
 */
	
	if (strstr(argv[0],"ip-up") || strstr(argv[0],"ip-down"))
	{
		ipupdown(argc,argv);
		goto main_end;
	}

	/* =============================================
		We always init cfgmgr module before all procedures
	============================================= */
	#if defined(CONFIG_RTL865X_DIAG_LED)
	if (board_cfgmgr_init(TRUE) != SUCCESS)
	{
		//re865xIoctl("eth0",SET_VLAN_IP,*(uint32*)(&(pRomeCfgParam->pptpCfgParam.ipAddr)),netmask,0);
		//re865xIoctl(char * name, uint32 arg0, uint32 arg1, uint32 arg2, uint32 arg3);
		re865xIoctl("eth0",RTL8651_IOCTL_DIAG_LED,2,0,0);
	}
	#else
	board_cfgmgr_init(TRUE); //before any flash read/write	
	#endif /* CONFIG_RTL865X_DIAG_LED */

	if(argc>1)
	{

		/* =============================================
			Process argument
		============================================= */

		if (!strcmp(argv[1],"default"))
		{
			/*
				user call "boa default"
				--> We must do factory default after init cfgmgr system.
			*/
			flashdrv_init();
			cfgmgr_factoryDefault(0);
			printf("cfgmgr_factoryDefault() done!\n");
		}

		else if (!strcmp(argv[1],"save"))
		{
			flashdrv_init();
			cfgmgr_task();
			printf("call cfgmgr_task() to save romeCfgParam to flash done!\n");
		}

		else if (!strcmp(argv[1],"-c")) //start
		{
			/* demo system configuration inialization entry point
			 * defined in board.c
			 */
			sysInit();
			goto conti;
		}
		
		else if (!strcmp(argv[1],"bound"))
		{
			dhcpcIPBound();
		}

		else if (!strcmp(argv[1], "renew"))
		{
			dhcpcIPRenew();
		}

		else if (!strcmp(argv[1],"deconfig"))
		{
			dhcpcDeconfig();
		}

		else if (!strcmp(argv[1],"addacl"))
		{			
			acl_tableDriverSingleHandle(ACL_SINGLE_HANDLE_ADD,atoi(argv[2]),atoi(argv[3]));
		}

		else if (!strcmp(argv[1],"delacl"))
		{			
			acl_tableDriverSingleHandle(ACL_SINGLE_HANDLE_DEL,atoi(argv[2]),atoi(argv[3]));
		}
		
		goto main_end;
		
	}

conti:
	root_temp.next=NULL;
	root_temp.str=NULL;	


	websFormDefine(T("asp_setLan"), asp_setLan);
	websFormDefine(T("asp_setDmz"), asp_setDmz);
	websFormDefine(T("asp_dhcpServerAdvance"), asp_dhcpServerAdvance);
	websFormDefine(T("asp_setDhcpClient"), asp_setDhcpClient);
	websFormDefine(T("asp_setPppoeWizard"), asp_setPppoeWizard);
	websFormDefine(T("asp_setPppoe"), asp_setPppoe);
	websFormDefine(T("asp_setUnnumberedWizard"), asp_setUnnumberedWizard);
	websFormDefine(T("asp_setNat"), asp_setNat);
	websFormDefine(T("asp_setPing"), asp_setPing);
	websFormDefine(T("asp_setRouting"), asp_setRouting);
	websFormDefine(T("asp_setPort"), asp_setPort);
	websFormDefine(T("asp_setStaticWizard"), asp_setStaticWizard);
#if defined(CONFIG_RTL865X_PPTPL2TP)||defined(CONFIG_RTL865XB_PPTPL2TP)
	websFormDefine(T("asp_setPptpWizard"), asp_setPptpWizard);	
	websFormDefine(T("asp_setL2tpWizard"), asp_setL2tpWizard);	
	websFormDefine(T("asp_setDhcpL2tpWizard"), asp_setDhcpL2tpWizard);	
#endif	
	websFormDefine(T("asp_upload"), asp_upload);
	websFormDefine(T("asp_setUrlFilter"), asp_setUrlFilter);
	websFormDefine(T("asp_restart"), asp_restart);
	websFormDefine(T("asp_systemDefault"), asp_systemDefault);
	websFormDefine(T("asp_wanIp"), asp_wanIp);
	websFormDefine(T("asp_setAcl"), asp_setAcl);
	websFormDefine(T("asp_setTZ"), asp_setTZ);
	websFormDefine(T("asp_setDdns"), asp_setDdns);
	websFormDefine(T("asp_setSpecialApplication"), asp_setSpecialApplication);
	websFormDefine(T("asp_setGaming"), asp_setGaming);	
	websFormDefine(T("asp_setServerPort"), asp_setServerPort);
	websFormDefine(T("asp_setEventLog"), asp_setEventLog);
	websFormDefine(T("asp_setUpnp"), asp_setUpnp);
	websFormDefine(T("asp_setDos"), asp_setDos);	
	websFormDefine(T("asp_setDosProc"), asp_setDosProc);	
	websFormDefine(T("asp_setAlg"), asp_setAlg);		
	websFormDefine(T("asp_setBt"), asp_setBt);			
	websFormDefine(T("asp_setUser"), asp_setUser);
	websFormDefine(T("asp_setMailAlert"), asp_setMailAlert);
	websFormDefine(T("asp_setRemoteLog"), asp_setRemoteLog);	
	websFormDefine(T("asp_setRemoteMgmt"), asp_setRemoteMgmt);
	websFormDefine(T("asp_setWlanBasic"), asp_setWlanBasic);	
	websFormDefine(T("asp_setWlanAdvance"), asp_setWlanAdvance);	
	websFormDefine(T("asp_setWlanSecurity"), asp_setWlanSecurity);	
	websFormDefine(T("asp_setWlanAccessCtrl"), asp_setWlanAccessCtrl);		
	websFormDefine(T("asp_selectWlanNeighborAp"), asp_selectWlanNeighborAp);
	websFormDefine(T("asp_setUdpBlocking"), asp_setUdpBlocking);	
	websFormDefine(T("asp_setQos"), asp_setQos);
	websFormDefine(T("asp_setQos1"), asp_setQos1);
	websFormDefine(T("asp_setpseudovlan"), asp_setpseudovlan);
	websFormDefine(T("asp_setPbnat"), asp_setPbnat);
	websFormDefine(T("asp_setFlash"), asp_setFlash);
	websFormDefine(T("asp_setRateLimit"), asp_setRateLimit);
	websFormDefine(T("asp_setRatio_qos"), asp_setRatio_qos);
	websFormDefine(T("asp_setModeConfig"), asp_setModeConfig);
	websFormDefine(T("asp_setRipConfig"), asp_setRipConfig);
	websFormDefine(T("asp_setPassthru"), asp_setPassthru);
	websFormDefine(T("asp_setMcast"), asp_setMcast);
	websFormDefine(T("asp_setNaptOpt"), asp_setNaptOpt);
	websAspDefine(T("asp_rateLimit"), asp_rateLimit);
	websAspDefine(T("asp_acl"), asp_acl);
	websAspDefine(T("asp_dmz"), asp_dmz);
	websAspDefine(T("asp_ddns"), asp_ddns);
	websAspDefine(T("asp_upnp"), asp_upnp);
	websAspDefine(T("asp_dos"), asp_dos);
	websAspDefine(T("asp_dosProc"), asp_dosProc);
	websAspDefine(T("asp_alg"), asp_alg);
	websAspDefine(T("asp_bt"), asp_bt);	
	websAspDefine(T("asp_user"), asp_user);
	websAspDefine(T("asp_mailAlert"), asp_mailAlert);	
	websAspDefine(T("asp_remoteLog"), asp_remoteLog);		
	websAspDefine(T("asp_remoteMgmt"), asp_remoteMgmt);	
	websAspDefine(T("asp_urlFilter"), asp_urlFilter);
	websAspDefine(T("asp_pppoe"), asp_pppoe);	
	websAspDefine(T("asp_ping"), asp_ping);	
	websAspDefine(T("asp_routing"), asp_routing);		
	websAspDefine(T("asp_port"), asp_port);		
	websAspDefine(T("asp_TZ"), asp_TZ);	
	websAspDefine(T("asp_TZ2"), asp_TZ2);	
	websAspDefine(T("asp_eventLog"), asp_eventLog);	
	websAspDefine(T("asp_serverPort"), asp_serverPort);	
	websAspDefine(T("asp_specialApplication"), asp_specialApplication);
	websAspDefine(T("asp_gaming"), asp_gaming);	
	websAspDefine(T("asp_statusExtended"), asp_statusExtended);
	websAspDefine(T("asp_configChanged"), asp_configChanged);
	websAspDefine(T("asp_flashGetCfgParam"), asp_flashGetCfgParam);
	websAspDefine(T("asp_dhcpServerLeaseList"), asp_dhcpServerLeaseList);
	websAspDefine(T("asp_flashGetPppoeParam"), asp_flashGetPppoeParam);
	websAspDefine(T("asp_getWanAddress"), asp_getWanAddress);
	websAspDefine(T("asp_flashGetCloneMac"), asp_flashGetCloneMac);	
	websAspDefine(T("asp_flashGetString"), asp_flashGetString);
	websAspDefine(T("asp_flashGetIpElement"), asp_flashGetIpElement);
	websAspDefine(T("asp_wlanBasic"), asp_wlanBasic);	
	websAspDefine(T("asp_wlanAdvance"), asp_wlanAdvance);	
	websAspDefine(T("asp_wlanSecurity"), asp_wlanSecurity);	
	websAspDefine(T("asp_wlanAccessCtrl"), asp_wlanAccessCtrl);	
	websAspDefine(T("asp_wlanClientList"), asp_wlanClientList);
	websAspDefine(T("asp_wlanNeighborApList"), asp_wlanNeighborApList);
	websAspDefine(T("asp_updateFW"), asp_updateFW);
	websAspDefine(T("asp_reboot"), asp_reboot);
	websAspDefine(T("asp_udpBlocking"), asp_udpBlocking);			
#if defined(CONFIG_RTL865X_PPTPL2TP)||defined(CONFIG_RTL865XB_PPTPL2TP)
	websAspDefine(T("asp_pptpWizard"), asp_pptpWizard);		
	websAspDefine(T("asp_l2tpWizard"), asp_l2tpWizard);			
	websAspDefine(T("asp_dhcpL2tpWizard"), asp_dhcpL2tpWizard);
#endif
	websAspDefine(T("asp_qos"), asp_qos);
	websAspDefine(T("asp_pseudovlan"), asp_pseudovlan);
	websAspDefine(T("asp_getLanPortStatus"), asp_getLanPortStatus);
	websAspDefine(T("asp_getWanPortStatus"), asp_getWanPortStatus);
	websAspDefine(T("asp_pbnat"), asp_pbnat);
	websAspDefine(T("asp_flash"), asp_flash);
	websAspDefine(T("asp_ratio_qos"), asp_ratio_qos);
	websAspDefine(T("asp_webcam"), asp_webcam);
	websAspDefine(T("asp_naptOpt"), asp_naptOpt);
	websAspDefine(T("asp_modeConfig"), asp_modeConfig);
	websAspDefine(T("asp_RipConfig"), asp_RipConfig);	
//	websAspDefine(T("asp_aclPage"), asp_aclPage);
	websAspDefine(T("asp_serverpPage"), asp_serverpPage);
	websAspDefine(T("asp_urlfilterPage"), asp_urlfilterPage);
	websAspDefine(T("asp_qosPage"), asp_qosPage);
	websAspDefine(T("asp_ratelimitPage"), asp_ratelimitPage);
	websAspDefine(T("asp_ratio_qosPage"), asp_ratio_qosPage);
	websAspDefine(T("asp_routingPage"), asp_routingPage);
	websAspDefine(T("asp_ripPage"), asp_ripPage);
	websAspDefine(T("asp_ddnsPage"), asp_ddnsPage);
	websAspDefine(T("asp_specialapPage"), asp_specialapPage);
	websAspDefine(T("asp_gamingPage"), asp_gamingPage);	
	websAspDefine(T("asp_algPage"), asp_algPage);
	websAspDefine(T("asp_dmzPage"), asp_dmzPage);
	websAspDefine(T("asp_dosPage"), asp_dosPage);
	websAspDefine(T("asp_udpblockingPage"), asp_udpblockingPage);
	websAspDefine(T("asp_pbnatPage"), asp_pbnatPage);
	websAspDefine(T("asp_pingPage"), asp_pingPage);
	websAspDefine(T("asp_naptoptPage"), asp_naptoptPage);
	websAspDefine(T("asp_pseudovlanPage"), asp_pseudovlanPage);
	websAspDefine(T("asp_passthruPage"), asp_passthruPage);
	websAspDefine(T("asp_mcastPage"), asp_mcastPage);
	websAspDefine(T("asp_bittorrentPage"), asp_bittorrentPage);
	websAspDefine(T("asp_passthru"), asp_passthru);
	websAspDefine(T("asp_mcast"), asp_mcast);
	websAspDefine(T("asp_IpsecExist"), asp_IpsecExist);
#ifdef CONFIG_KLIPS
	websAspDefine(T("asp_flashGetIpsec"), asp_flashGetIpsec);	
	websAspDefine(T("asp_GetIpsecStatus"), asp_GetIpsecStatus);	
	websFormDefine(T("asp_setIpsecBasic"), asp_setIpsecBasic);
	websFormDefine(T("asp_IpsecConnect"), asp_IpsecConnect);
#endif


	query_temp_var=(char *)malloc(MAX_QUERY_TEMP_VAL_SIZE);
	if(query_temp_var==NULL) exit(0);

	return;
	
main_end:
	shmdt(pRomeCfgParam);
	exit(0);
#endif // UNUSED	
}

int  websRedirect(request *req,char *url)
{
	req->buffer_end=0;
	send_redirect_perm(req,url);
	return SUCCESS;
}

int allocNewBuffer(request *req)
{
	char *newBuffer;
	newBuffer=(char *)malloc(req->max_buffer_size*2+1);
	if(newBuffer==NULL) return FAILED;
	memcpy(newBuffer,req->buffer,req->max_buffer_size);
	req->max_buffer_size<<=1;	
	free(req->buffer);	
	req->buffer=newBuffer;
	return SUCCESS;
}

int websWriteBlock(request *req, char *buf, int nChars)
{

	int bob=nChars;	
	int pos = 0;

#ifndef SUPPORT_ASP		
	if((bob+req->buffer_end)>BUFFER_SIZE) bob=BUFFER_SIZE- req->buffer_end;
#else
	while((bob+req->buffer_end)>req->max_buffer_size) 
	{
		int ret;
		#if MINIMIZE_RAM_USAGE
		int byte_to_send;

		byte_to_send = req->max_buffer_size - req->buffer_end;
		if (byte_to_send > bob)
			byte_to_send = bob;

		memcpy(req->buffer + req->buffer_end,	buf + pos, byte_to_send);
		req->buffer_end += byte_to_send;
		pos += byte_to_send;
		bob -= byte_to_send;

		ret = req_flush_retry(req);
                
                #else  // MINIMIZE_RAM_USAGE

                ret=allocNewBuffer(req);
		if(ret==FAILED) {bob=BUFFER_SIZE- req->buffer_end; break;}

		#endif // MINIMIZE_RAM_USAGE
	}
#endif

	if(bob>0)
	{
		memcpy(req->buffer + req->buffer_end,	buf+pos, bob);	
		req->buffer_end+=bob;
		return 0;
	}
	return -1;
}

void handleForm(request *req)
{
	char *ptr;
	#define SCRIPT_ALIAS "/goform/"
	// kaotest
	char *myalias;
	#define SCRIPT_USERALIAS "/goform/admin/"
	ptr=strstr(req->request_uri,SCRIPT_USERALIAS);
	if (ptr==NULL) {
	ptr=strstr(req->request_uri,SCRIPT_ALIAS);
		myalias = "/goform/";
	}
	else
		myalias = "/goform/admin/";
//	printf("into handleForm %s........\n",req->request_uri);

	if(ptr==NULL)
	{
		send_r_not_found(req);
		return;
	}
	else
	{
		form_name_t *now_form;
		// kaotest
		ptr+=strlen(myalias);
		//ptr+=strlen(SCRIPT_ALIAS);

		now_form=root_form;
		while(1)
		{
			if (	(strlen(ptr) == strlen(now_form->pagename)) &&
				(memcmp(ptr,now_form->pagename,strlen(now_form->pagename))==0))
			{
 #ifdef VOIP_SUPPORT
  /* for VoIP config load page */
				if(0 == strncmp(now_form->pagename,"voip_config_load", strlen("voip_config_load"))){
					now_form->function(req,NULL,NULL);
					send_r_request_ok(req);	
				}else{
					send_r_request_ok(req);
					now_form->function(req,NULL,NULL);
				}
#else
#ifdef MULTI_USER_PRIV
				//printf("handleForm: req->user=%s form->pagename=%s form->privilege=%d\n", req->user, now_form->pagename, now_form->privilege);
				//websRedirect(req, now_form->pagename);
				if ((int)now_form->privilege > getAccPriv(req->user)) {
					websHeader(req);
				   	websWrite(req, T("<body><blockquote><h2><font color=\"#FF0000\">Access Denied !!</font></h2><p>\n"));
				   	websWrite(req, T("<h4>Sorry, you don't have enough privilege to take this action.</h4>\n"));
					websWrite(req, T("<form><input type=\"button\" onclick=\"history.go (-1)\" value=\"&nbsp;&nbsp;OK&nbsp;&nbsp\" name=\"OK\"></form></blockquote></body>"));
				   	websFooter(req);
					websDone(req, 200);
				}
				else {
					send_r_request_ok(req);		/* All's well */	
					now_form->function(req,NULL,NULL);
				}
#else
				send_r_request_ok(req);		/* All's well */	
				now_form->function(req,NULL,NULL);
#endif
#endif				
				freeAllTempStr();
				//websWrite(req,"okokok\n");			
				return;
			}
			
			if(now_form->next==NULL) break;
			now_form=now_form->next;
		}

		send_r_not_found(req);
		return;
		

	}
	

}

void handleScript(request *req,char *left1,char *right1)	
{
	char *left=left1,*right=right1;
	asp_name_t *now_asp;
	uint32 funcNameLength;
	int i;
	left+=2;
	right-=1;   
	while(1)
	{
		while(*left==' ') {if(left>=right) break;++left;}
		while(*left==';') {if(left>=right) break;++left;}
		while(*left=='(') {if(left>=right) break;++left;}
		while(*left==')') {if(left>=right) break;++left;}
		while(*left==',') {if(left>=right) break;++left;}
		if(left>=right) break;

		/* count the function name length */
		{
			char *ptr = left;

			funcNameLength = 0;
			while (*ptr!='(' && *ptr!=' '){
				ptr++;
				funcNameLength++;
				if ((uint32)ptr >= (uint32)right) {
					break;
				}
			}
		}

		now_asp=root_asp;
		while(1)
		{
         if (!now_asp) //ANDREW
            break;     //ANDREW
			if(	(strlen(now_asp->pagename) == funcNameLength) &&
				(memcmp(left,now_asp->pagename,strlen(now_asp->pagename))==0))
			{
				char *leftc,*rightc;
				int argc=0;
				char *argv[8]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
				left+=strlen(now_asp->pagename);
				
				while(1) 
				{
					int size,exit=0;
					while(1)
					{
						if(*left==')') {exit=1; break;}
						if(*left=='\"') break;
						if((unsigned int)left>(unsigned int)right) {exit=1; break;}
						left++;	
					}
					
					if(exit==1) break;					
					leftc=left;
					leftc++;
					rightc=strstr(leftc,"\"");
					if(rightc==NULL) break;
					size=(unsigned int)rightc-(unsigned int)leftc+1;
					argv[argc]=(char *)malloc(size);
					if(argv[argc]==NULL) break;
					memcpy(argv[argc],leftc,size-1);
					argv[argc][size-1]=0;
					argc++;
					left=rightc+1;
				}

				//ANDREW now_asp->function(req,argc,argv);
            now_asp->function(0, req,argc,argv); // ANDREW

				for(i=0;i<argc;i++) free(argv[i]);
				break;
			}

			if(now_asp->next==NULL) break;
			now_asp=now_asp->next;
		}
		++left;
	}

}


#endif
