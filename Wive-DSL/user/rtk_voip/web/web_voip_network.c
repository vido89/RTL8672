#include <stdio.h>
#include "web_voip.h"

char hwnat[2][10]={"Disable", "Enable"};

#ifdef SUPPORT_DSCP
char dscp[DSCP_MAX][25] = {"Class 0 (DSCP 0x00)",
			"Class 1 (DSCP 0x08)",
			"Class 2 (DSCP 0x10)",
			"Class 3 (DSCP 0x18)",
			"Class 4 (DSCP 0x20)",
			"Class 5 (DSCP 0x28)",
			"Class 6 (DSCP 0x30)",
			"Class 7 (DSCP 0x38)",
			"EF (DSCP 0x2e)"
			};
#endif

void asp_voip_NetSet(webs_t wp, char_t *path, char_t *query)
{
	voipCfgParam_t *pVoIPCfg;
	int i ;
	char* ptr;
	if (web_flash_get(&pVoIPCfg) != 0)
		return;

#ifdef CONFIG_RTK_VOIP_WAN_VLAN
	pVoIPCfg->wanVlanEnable = !gstrcmp(websGetVar(wp, T("wanVlanEnable"), T("")), "on");
	if (pVoIPCfg->wanVlanEnable) {
		pVoIPCfg->wanVlanIdVoice = atoi(websGetVar(wp, T("wanVlanIdVoice"), T("2")));
		pVoIPCfg->wanVlanPriorityVoice = atoi(websGetVar(wp, T("wanVlanPriorityVoice"), T("0")));
		pVoIPCfg->wanVlanCfiVoice = atoi(websGetVar(wp, T("wanVlanCfiVoice"), T("0")));
		pVoIPCfg->wanVlanIdData = atoi(websGetVar(wp, T("wanVlanIdData"), T("2")));
		pVoIPCfg->wanVlanPriorityData = atoi(websGetVar(wp, T("wanVlanPriorityData"), T("0")));
		pVoIPCfg->wanVlanCfiData = atoi(websGetVar(wp, T("wanVlanCfiData"), T("0")));
		pVoIPCfg->wanVlanIdVideo = atoi(websGetVar(wp, T("wanVlanIdVideo"), T("2")));
		pVoIPCfg->wanVlanPriorityVideo = atoi(websGetVar(wp, T("wanVlanPriorityVideo"), T("0")));
		pVoIPCfg->wanVlanCfiVideo = atoi(websGetVar(wp, T("wanVlanCfiVideo"), T("0")));
	}
#endif

#ifdef SUPPORT_DSCP
	ptr	 = websGetVar(wp, T("rtpDscp"), T(""));
	for(i=0; i<DSCP_MAX; i++)
	{
		if (!gstrcmp(ptr, dscp[i]))
			break;
	}
	if (i == DSCP_MAX)
		i = DSCP_CS0;
	pVoIPCfg->rtpDscp	= i;
	
	ptr	 = websGetVar(wp, T("sipDscp"), T(""));
	for(i=0; i<DSCP_MAX; i++)
	{
		if (!gstrcmp(ptr, dscp[i]))
			break;
	}
	if (i == DSCP_MAX)
		i = DSCP_CS0;
	pVoIPCfg->sipDscp = i;
	
#endif

#ifdef CONFIG_RTL865X_HW_TABLES		
	ptr = websGetVar(wp, T("hwnat_enable"), T(""));

	for(i=0; i < 2; i++)
	{
		if (!gstrcmp(ptr, hwnat[i]))
			break;
	}
	pVoIPCfg->hwnat_enable = i;
#endif

#ifdef CONFIG_RTL865XC	
	pVoIPCfg->bandwidth_LANPort0_Egress = atoi(websGetVar(wp, T("LANPort0_Bandwidth_out"), T("0")));
	pVoIPCfg->bandwidth_LANPort1_Egress = atoi(websGetVar(wp, T("LANPort1_Bandwidth_out"), T("0")));
	pVoIPCfg->bandwidth_LANPort2_Egress = atoi(websGetVar(wp, T("LANPort2_Bandwidth_out"), T("0")));	
	pVoIPCfg->bandwidth_LANPort3_Egress = atoi(websGetVar(wp, T("LANPort3_Bandwidth_out"), T("0")));
	pVoIPCfg->bandwidth_WANPort_Egress = atoi(websGetVar(wp, T("WANPort_Bandwidth_out"), T("0")));
	pVoIPCfg->bandwidth_LANPort0_Ingress = atoi(websGetVar(wp, T("LANPort0_Bandwidth_in"), T("0")));
	pVoIPCfg->bandwidth_LANPort1_Ingress = atoi(websGetVar(wp, T("LANPort1_Bandwidth_in"), T("0")));
	pVoIPCfg->bandwidth_LANPort2_Ingress = atoi(websGetVar(wp, T("LANPort2_Bandwidth_in"), T("0")));
	pVoIPCfg->bandwidth_LANPort3_Ingress = atoi(websGetVar(wp, T("LANPort3_Bandwidth_in"), T("0")));
	pVoIPCfg->bandwidth_WANPort_Ingress = atoi(websGetVar(wp, T("WANPort_Bandwidth_in"), T("0")));
#endif	
	web_flash_set(pVoIPCfg);

#ifdef CONFIG_RTL865XC
#ifndef NO_ACTION
	run_init_script("all");
#endif
#else
	web_restart_solar();
#endif
	websRedirect(wp, T("/voip_network.asp"));
}


#if CONFIG_RTK_VOIP_PACKAGE_865X
int asp_voip_NetGet(webs_t wp, int argc, char_t **argv)
#else
int asp_voip_NetGet(int ejid, webs_t wp, int argc, char_t **argv)
#endif
{	
	voipCfgParam_t *pVoIPCfg;
	int index;
	
	if (web_flash_get(&pVoIPCfg) != 0)
		return -1;
	
	if(strcmp(argv[0], "wanVlanEnable")==0)
		websWrite(wp, "%s", (pVoIPCfg->wanVlanEnable) ? "checked" : "");
	else if (strcmp(argv[0], "wanVlanIdVoice")==0)
		websWrite(wp, "%d", pVoIPCfg->wanVlanIdVoice);
	else if (strcmp(argv[0], "wanVlanPriorityVoice")==0)
		websWrite(wp, "%d", pVoIPCfg->wanVlanPriorityVoice);
	else if (strcmp(argv[0], "wanVlanCfiVoice")==0)
		websWrite(wp, "%d", pVoIPCfg->wanVlanCfiVoice);
	else if (strcmp(argv[0], "wanVlanIdData")==0)
		websWrite(wp, "%d", pVoIPCfg->wanVlanIdData);
	else if (strcmp(argv[0], "wanVlanPriorityData")==0)
		websWrite(wp, "%d", pVoIPCfg->wanVlanPriorityData);
	else if (strcmp(argv[0], "wanVlanCfiData")==0)
		websWrite(wp, "%d", pVoIPCfg->wanVlanCfiData);
	else if (strcmp(argv[0], "wanVlanIdVideo")==0)
		websWrite(wp, "%d", pVoIPCfg->wanVlanIdVideo);
	else if (strcmp(argv[0], "wanVlanPriorityVideo")==0)
		websWrite(wp, "%d", pVoIPCfg->wanVlanPriorityVideo);
	else if (strcmp(argv[0], "wanVlanCfiVideo")==0)
		websWrite(wp, "%d", pVoIPCfg->wanVlanCfiVideo);
#ifdef CONFIG_RTK_VOIP_WAN_VLAN
	else if (strcmp(argv[0], "display_wanVlan")==0)
		websWrite(wp, "%s", "");
#else
	else if (strcmp(argv[0], "display_wanVlan")==0)
		websWrite(wp, "%s", "style=\"display:none\"");
#endif

#ifdef SUPPORT_DSCP
	else if (strcmp(argv[0], "rtpDscp")==0)
	{
		for (index=0; index<DSCP_MAX ;index++)
		{
			if (index == pVoIPCfg->rtpDscp)
				websWrite(wp, "<option selected>%s</option>", dscp[index]);
			else
				websWrite(wp, "<option>%s</option>", dscp[index]);
		}
	}

	else if (strcmp(argv[0], "sipDscp")==0)
	{
		for (index=0; index<DSCP_MAX ;index++)
		{
			if (index == pVoIPCfg->sipDscp)
				websWrite(wp, "<option selected>%s</option>", dscp[index]);
			else
				websWrite(wp, "<option>%s</option>", dscp[index]);
		}
	}
#endif
#ifdef CONFIG_RTL865X_HW_TABLES
	else if (strcmp(argv[0], "display_hwnat")==0)
		websWrite(wp, "%s", "");
#else
	else if (strcmp(argv[0], "display_hwnat")==0)
		websWrite(wp, "%s", "style=\"display:none\"");
#endif
	else if(strcmp(argv[0], "hwnat_enable")==0)
	{
		for (index=0; index < 2 ;index++)
		{
			if (index == pVoIPCfg->hwnat_enable)
				websWrite(wp, "<option selected>%s</option>", hwnat[index]);
			else
				websWrite(wp, "<option>%s</option>", hwnat[index]);	
		}
	}

#ifdef CONFIG_RTL865XC
	else if (strcmp(argv[0], "display_bandwidth_mgr")==0)
		websWrite(wp, "%s", "");
#else
	else if (strcmp(argv[0], "display_bandwidth_mgr")==0)
		websWrite(wp, "%s", "style=\"display:none\"");
#endif
	
	else if (strcmp(argv[0], "LANPort0_Bandwidth_out")==0)
		websWrite(wp, "%d", pVoIPCfg->bandwidth_LANPort0_Egress);
	else if (strcmp(argv[0], "LANPort1_Bandwidth_out")==0)
		websWrite(wp, "%d", pVoIPCfg->bandwidth_LANPort1_Egress);
	else if (strcmp(argv[0], "LANPort2_Bandwidth_out")==0)
		websWrite(wp, "%d", pVoIPCfg->bandwidth_LANPort2_Egress);
	else if (strcmp(argv[0], "LANPort3_Bandwidth_out")==0)
		websWrite(wp, "%d", pVoIPCfg->bandwidth_LANPort3_Egress);
	else if (strcmp(argv[0], "WANPort_Bandwidth_out")==0)
		websWrite(wp, "%d", pVoIPCfg->bandwidth_WANPort_Egress);
	else if (strcmp(argv[0], "LANPort0_Bandwidth_in")==0)
		websWrite(wp, "%d", pVoIPCfg->bandwidth_LANPort0_Ingress);
	else if (strcmp(argv[0], "LANPort1_Bandwidth_in")==0)
		websWrite(wp, "%d", pVoIPCfg->bandwidth_LANPort1_Ingress);
	else if (strcmp(argv[0], "LANPort2_Bandwidth_in")==0)
		websWrite(wp, "%d", pVoIPCfg->bandwidth_LANPort2_Ingress);
	else if (strcmp(argv[0], "LANPort3_Bandwidth_in")==0)
		websWrite(wp, "%d", pVoIPCfg->bandwidth_LANPort3_Ingress);
	else if (strcmp(argv[0], "WANPort_Bandwidth_in")==0)
		websWrite(wp, "%d", pVoIPCfg->bandwidth_WANPort_Ingress);
	else
	{
		return -1;	
	}
	return 0;	
}

