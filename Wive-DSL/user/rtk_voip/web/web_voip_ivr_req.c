#include <stdio.h>
#include "web_voip.h"

static int GetIvrReqCheckboxValue( webs_t wp,
								   unsigned char *pCheckboxName, 
								   unsigned char *pValue )
{
	unsigned char temp;
	
	temp = !gstrcmp(websGetVar(wp, pCheckboxName, T("")), "on");
	
	if( temp ) {
		/* enable */
		if( *pValue != 1 ) {
			*pValue = 1;
			return 1;		/* update to 'on' */
		}
	} else {
		temp = !gstrcmp(websGetVar(wp, pCheckboxName, T("")), "off");
		
		if( temp ) {
			/* disable */
			if( *pValue != 0 ) {
				*pValue = 0;
				return 1;	/* update to 'off' */
			}
		}
	}
	
	return 0;	/* no update */
}

static int GetIvrReqTextValue( webs_t wp,
							   unsigned char *pTextName, 
							   unsigned char *pszValue )
{
	char_t *pszResponse;
	
	if( ( pszResponse = websGetVar( wp, pTextName, NULL ) ) ) {
		
		strcpy( pszValue, pszResponse );
	
		return 1;	/* update string */
	}

	return 0;	/* no update */
}

static int SetIvrReqCodecSequence( voipCfgPortParam_t *pCfg, webs_t wp )
{
	unsigned char oldPrecedence;
	unsigned char oldFirstCodec;
	unsigned char firstCodec;
#ifdef CONFIG_RTK_VOIP_G7231
	unsigned char rate;
#endif

	/* first_codec is a novel parameter for IVR only. */
	firstCodec = atoi(websGetVar(wp, T("first_codec"), "99"));	/* SUPPORTED_CODEC_MAX < 99 */
#ifdef CONFIG_RTK_VOIP_G7231
	rate = atoi(websGetVar(wp, T("rate"), "0"));
#endif
	
	if( firstCodec < SUPPORTED_CODEC_MAX ) {

		/* search for the first one */
		for( oldFirstCodec = 0; oldFirstCodec < SUPPORTED_CODEC_MAX; oldFirstCodec ++ )
			if( pCfg ->precedence[ oldFirstCodec ] == 0 )
				goto label_first_codec_found;
		
		return 0;	/* No first codec ??!! */

label_first_codec_found:
		/* bring codec to be first one */
		oldPrecedence = pCfg ->precedence[ firstCodec ];
		pCfg ->precedence[ firstCodec ] = 0;
#ifdef CONFIG_RTK_VOIP_G7231
		if( firstCodec == SUPPORTED_CODEC_G723 )
			pCfg ->g7231_rate = rate;
#endif
#ifdef CONFIG_RTK_VOIP_ILBC		
		if( firstCodec == SUPPORTED_CODEC_ILBC )
			pCfg ->iLBC_mode = rate;
#endif
		
		/* set old first one */
		pCfg ->precedence[ oldFirstCodec ] = oldPrecedence;

		return 1;
	}
	
	return 0;
}

static int SetIvrReqVoipPart( voipCfgParam_t *pVoIPCfg, 
							  webs_t wp, char_t *path, char_t *query )
{
	voipCfgPortParam_t *pCfg;
	int voip_port;
	unsigned char temp;
	int temp2;
	int ret = 0;

	voip_port = atoi(websGetVar(wp, T("voipPort"), "0"));
	pCfg = &pVoIPCfg->ports[voip_port];	

	/* Codec Sequence */
	if( SetIvrReqCodecSequence( pCfg, wp ) )
		ret = 1;
	
	/* Handset gain */
	temp = atoi(websGetVar(wp, T("slic_txVolumne"), T("0")));
	
	if( temp > 0 ) {
		pCfg->slic_txVolumne = temp - 1;
		ret = 1;
	}
	
	/* Handset volume */
	temp = atoi(websGetVar(wp, T("slic_rxVolumne"), T("0")));
	
	if( temp > 0 ) {
		pCfg->slic_rxVolumne = temp - 1;
		ret = 1;
	}

	/* Speaker volume gain */
	temp2 = atoi(websGetVar(wp, T("spk_voice_gain"), T("100")));
	
	if( temp2 >= -32 && temp2 <= 31 ) {
		pCfg->spk_voice_gain = ( char )temp2;
		ret = 1;
	}
	
	/* Mic volume volume */
	temp2 = atoi(websGetVar(wp, T("mic_voice_gain"), T("100")));
	
	if( temp2 >= -32 && temp2 <= 31 ) {
		pCfg->mic_voice_gain = ( char )temp2;
		ret = 1;
	}
	
	/* Enable/disable call waiting */
	if( GetIvrReqCheckboxValue( wp, T("call_waiting"), 
								&pCfg->call_waiting_enable ) )
	{
		ret = 1;
	}
	
	/* Forward setting */
	/* 1. Always forward */
	if( GetIvrReqCheckboxValue( wp, T("CFAll"), 
								&pCfg->uc_forward_enable ) )
	{
		ret = 1;
	}
	
	if( GetIvrReqTextValue( wp, T("CFAll_No"), pCfg->uc_forward ) )
		ret = 1;
	
	/* 2. Busy forward */
	if( GetIvrReqCheckboxValue( wp, T("CFBusy"), 
								&pCfg->busy_forward_enable ) )
	{
		ret = 1;
	}
	
	if( GetIvrReqTextValue( wp, T("CFBusy_No"), pCfg->busy_forward ) )
		ret = 1;
	
	/* 3. No answer forward */
	if( GetIvrReqCheckboxValue( wp, T("CFNoAns"), 
								&pCfg->na_forward_enable ) )
	{
		ret = 1;
	}	
	
	if( GetIvrReqTextValue( wp, T("CFNoAns_No"), pCfg->na_forward ) )
		ret = 1;
		
	temp2 = atoi(websGetVar(wp, T("CFNoAns_Time"), T("-1")));
	
	if( temp2 >= 0 ) {
		pCfg->na_forward_time = temp2;
		ret = 1;
	}

	return ret;
}

static int SetIvrReqNetPart( webs_t wp, char_t *path, char_t *query )
{
#if 0	
	union {
		unsigned char *pszIP;
		unsigned char *pszMask;
		unsigned char *pszGateway;
		unsigned char *pszDns;
		unsigned char bSaveAndReboot;
		unsigned char bResetToDefault;
	} webVars;

	/* Fixed IP */
	webVars.pszIP = websGetVar(wp, T("lan_ip"), NULL);
	
	if( webVars.pszIP ) {
		// TODO:
	}

	/* Netmask */
	webVars.pszMask = websGetVar(wp, T("lan_mask"), NULL);

	if( webVars.pszMask ) {
		// TODO:
	}
	
	/* Gateway */
	webVars.pszGateway = websGetVar(wp, T("lan_gateway"), NULL);

	if( webVars.pszGateway ) {
		// TODO:
	}

	/* DNS */
	webVars.pszDns = websGetVar(wp, T("dns1"), NULL);

	if( webVars.pszDns ) {
		// TODO:
	}
	
	/* Save and reboot */
	webVars.bSaveAndReboot = atoi( websGetVar(wp, T("saveAndReboot"), T("0")) );
	
	if( webVars.bSaveAndReboot ) {
		// TODO: 
	}

	/* Reset to default */
	webVars.bResetToDefault = atoi( websGetVar(wp, T("reset2default"), T("0")) );
	
	if( webVars.bResetToDefault ) {
		// TODO: 
	}
#endif

	return 0;
}

void asp_voip_IvrReqSet(webs_t wp, char_t *path, char_t *query)
{
	voipCfgParam_t *pVoIPCfg;
	int bUpdateVoipFlash = 0, bUpdateNetFlash = 0;

	printf( "----------------------------\nasp_voip_IvrReqSet\n" );

	printf( "%s\n", websGetVar(wp, T("ivr_test"), T("*0")));

		
	/* Set VoIP part */
	if (voip_flash_get(&pVoIPCfg) != 0)
		return;
		
	if( SetIvrReqVoipPart( pVoIPCfg, wp, path, query ) )
		bUpdateVoipFlash = 1;

	/* Set Net configuration part */
	if( SetIvrReqNetPart( wp, path, query ) )
		bUpdateNetFlash = 1;

	/* Write to flash */
	if( bUpdateNetFlash ) {
		/* If update network settings, it will restart whole system. */
		// TODO: how to deal with net config part??
	
	} else if( bUpdateVoipFlash ) {
		/* If update voip only, it will restart solar. */
		voip_flash_set(pVoIPCfg);

		web_restart_solar();
	}

	websRedirect(wp, T("/voip_ivr_req.asp"));
}

