#include <stdio.h>
#include "web_voip.h"

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

static char cid_det[CID_MAX][25] = {
	"FSK_BELLCORE", "FSK_ETSI", "FSK_BT", "FSK_NTT", "DTMF"
};

int char_replace(const char *src, char old_char, char new_char, char *result)
{
	int i;

	for (i=0; src[i]; i++)
	{
		if (src[i] == old_char)
			result[i] = new_char;
		else
			result[i] = src[i];
	}

	result[i] = 0;
	return 0;
}

#if CONFIG_RTK_VOIP_PACKAGE_865X
int asp_voip_OtherGet(webs_t wp, int argc, char_t **argv)
#else
int asp_voip_OtherGet(int ejid, webs_t wp, int argc, char_t **argv)
#endif
{
	voipCfgParam_t *pVoIPCfg;
	char key_display[FUNC_KEY_LENGTH];
#ifdef SUPPORT_DSCP
	int i;
#endif

	if (voip_flash_get(&pVoIPCfg) != 0)
		return -1;

	if (strcmp(argv[0], "funckey_transfer")==0)
	{
		char_replace(pVoIPCfg->funckey_transfer, '.', '*', key_display);
		websWrite(wp, "%s", key_display);
	}
	else if (strcmp(argv[0], "funckey_pstn")==0)
	{
		char_replace(pVoIPCfg->funckey_pstn, '.', '*', key_display);
		websWrite(wp, "%s", key_display);
	}
	else if (strcmp(argv[0], "display_funckey_pstn")==0)
	{
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
		websWrite(wp, "%s", "");
#else
		websWrite(wp, "%s", "style=\"display:none\"");
#endif
	}
	else if (strcmp(argv[0], "auto_dial_display_title") == 0)
	{
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		/* nothing */
#else
		websWrite(wp, "<p>\n<b>Dial Option</b>\n");
#endif
	}
	else if (strcmp(argv[0], "auto_dial_display") == 0)
	{
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		websWrite(wp, "style=\"display:none\"");
#endif
	}
	else if (strcmp(argv[0], "auto_dial")==0)
	{
		websWrite(wp, "%d", pVoIPCfg->auto_dial & AUTO_DIAL_TIME);
	}
	else if (strcmp(argv[0], "auto_dial_always") == 0)
	{
		websWrite(wp, "%s", pVoIPCfg->auto_dial & AUTO_DIAL_ALWAYS ? "checked" : "");
	}
	else if (strcmp(argv[0], "off_hook_alarm_display_title") == 0)
	{
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		/* nothing */
#else
		websWrite(wp, "<p>\n<b>Off-Hook Alarm</b>\n");
#endif
	}
	else if (strcmp(argv[0], "off_hook_alarm_display") == 0)
	{
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		websWrite(wp, "style=\"display:none\"");
#endif
	}
	else if (strcmp(argv[0], "off_hook_alarm")==0)
	{
		websWrite(wp, "%d", pVoIPCfg->off_hook_alarm);
	}
	else if (strcmp(argv[0], "caller_id_auto_det")==0)
	{
		websWrite(wp, "<input type=\"radio\" name=\"caller_id_auto_det\" value=0 onClick=enable_cid_det_mode() %s>Off",
			pVoIPCfg->cid_auto_det_select == 0 ? "checked" : "");
		websWrite(wp, "<input type=\"radio\" name=\"caller_id_auto_det\" value=1 onClick=enable_cid_det_mode() %s>On (NTT Support)",
			pVoIPCfg->cid_auto_det_select == 1 ? "checked" : "");
   		websWrite(wp, "<input type=\"radio\" name=\"caller_id_auto_det\" value=2 onClick=enable_cid_det_mode() %s>On (NTT Not Support)",
			pVoIPCfg->cid_auto_det_select == 2 ? "checked" : "");
	}
	else if (strcmp(argv[0], "caller_id_det")==0)
	{	
		websWrite(wp, "<select name=caller_id_det %s>", 
			pVoIPCfg->cid_auto_det_select != 0 ? "disabled" : "");

		for (i=0; i<CID_MAX ;i++)
		{
			if (i == pVoIPCfg->caller_id_det_mode)
				websWrite(wp, "<option selected>%s</option>", cid_det[i]);
			else
				websWrite(wp, "<option>%s</option>", cid_det[i]);
		}

		websWrite(wp, "</select>");
	}
#ifdef CONFIG_RTK_VOIP_DRIVERS_SI3050
	else if (strcmp(argv[0], "display_cid_det")==0)
		websWrite(wp, "%s", "");
#else
	else if (strcmp(argv[0], "display_cid_det")==0)
		websWrite(wp, "%s", "style=\"display:none\"");
#endif
	else if (strcmp(argv[0], "wanVlanEnable")==0)
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
		for (i=0; i<DSCP_MAX ;i++)
		{
			if (i == pVoIPCfg->rtpDscp)
				websWrite(wp, "<option selected>%s</option>", dscp[i]);
			else
				websWrite(wp, "<option>%s</option>", dscp[i]);
		}
	}

	else if (strcmp(argv[0], "sipDscp")==0)
	{
		for (i=0; i<DSCP_MAX ;i++)
		{
			if (i == pVoIPCfg->sipDscp)
				websWrite(wp, "<option selected>%s</option>", dscp[i]);
			else
				websWrite(wp, "<option>%s</option>", dscp[i]);
		}
	}
#endif
	else
	{
		return -1;
	}

	return 0;
}

void asp_voip_OtherSet(webs_t wp, char_t *path, char_t *query)
{
	voipCfgParam_t *pVoIPCfg;
	char key_display[FUNC_KEY_LENGTH];
	char *ptr;
	int i;
		
	if (voip_flash_get(&pVoIPCfg) != 0)
		return;

	strcpy(key_display, websGetVar(wp, T("funckey_transfer"), T("*1")));
	char_replace(key_display, '*', '.', pVoIPCfg->funckey_transfer);

#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
	strcpy(key_display, websGetVar(wp, T("funckey_pstn"), T("*0")));
	char_replace(key_display, '*', '.', pVoIPCfg->funckey_pstn);
#endif

	pVoIPCfg->auto_dial = (atoi(websGetVar(wp, T("auto_dial"), T("5")))) & AUTO_DIAL_TIME;
	if (gstrcmp(websGetVar(wp, T("auto_dial_always"), T("")), "on") == 0)
		pVoIPCfg->auto_dial |= AUTO_DIAL_ALWAYS;

	pVoIPCfg->off_hook_alarm = atoi(websGetVar(wp, T("off_hook_alarm"), T("30")));

#ifdef CONFIG_RTK_VOIP_DRIVERS_SI3050
	pVoIPCfg->cid_auto_det_select = atoi(websGetVar(wp, T("caller_id_auto_det"), T("")));
	ptr	 = websGetVar(wp, T("caller_id_det"), T(""));
	for(i=0; i<CID_MAX; i++)
		if (!gstrcmp(ptr, cid_det[i]))
			break;

	pVoIPCfg->caller_id_det_mode = (i == CID_MAX) ? CID_DTMF : i;
#endif

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
	pVoIPCfg->sipDscp	= i;
#endif

	voip_flash_set(pVoIPCfg);
	web_restart_solar();
	websRedirect(wp, T("/voip_other.asp"));
}
