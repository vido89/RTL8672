#include <stdio.h>
#include "web_voip.h"

#ifdef SUPPORT_CODEC_DESCRIPTOR
#include "codec_table.h"
#endif

char dtmf[DTMF_MAX][9] = {"RFC2833", "SIP INFO", "Inband"};

char cid[CID_MAX][25] = {"FSK_BELLCORE", "FSK_ETSI", "FSK_BT", "FSK_NTT", "DTMF"};

char cid_dtmf[CID_DTMF_MAX][8] = {"DTMF_A","DTMF_B","DTMF_C","DTMF_D"};

#ifdef SUPPORT_VOICE_QOS
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

char *supported_codec_string[SUPPORTED_CODEC_MAX] = {
	"G711-ulaw",
	"G711-alaw",
#ifdef CONFIG_RTK_VOIP_G729AB
	"G729",
#endif	
#ifdef CONFIG_RTK_VOIP_G7231
	"G723",
#endif	
#ifdef CONFIG_RTK_VOIP_G726
	"G726-16k",
	"G726-24k",
	"G726-32k",
	"G726-40k",
#endif	
#ifdef CONFIG_RTK_VOIP_GSMFR
	"GSM-FR",
#endif
#ifdef CONFIG_RTK_VOIP_ILBC
	"iLBC",
#endif
	};

#ifdef SUPPORT_CODEC_DESCRIPTOR
CT_ASSERT( ( sizeof( supported_codec_string ) / sizeof( supported_codec_string[ 0 ] ) ) == SUPPORTED_CODEC_MAX );
CT_ASSERT( SUPPORTED_CODEC_MAX == NUM_CODEC_TABLE );
#endif

void asp_sip_codec_var(webs_t wp, voipCfgPortParam_t *pCfg)
{
	int i;

	for (i=0; i<SUPPORTED_CODEC_MAX; i++)
	{
		websWrite(wp,
			"<input type=hidden id=preced_id name=preced%d value=-1>", i
			);
	}

	websWrite(wp,
		"<input type=hidden name=codec_num value=%d>", 
		SUPPORTED_CODEC_MAX
		);
}

void asp_sip_codec(webs_t wp, voipCfgPortParam_t *pCfg)
{
	int i, j;
#ifdef SUPPORT_CUSTOMIZE_FRAME_SIZE
	int step, loop;
#endif
		
	websWrite(wp,
		"<tr align=center>" \
		"<td bgColor=#aaddff width=85 rowspan=2>Type</td>"
		);

// framesize is reserved
#ifdef SUPPORT_CUSTOMIZE_FRAME_SIZE
	websWrite(wp,
		"<td bgColor=#ddeeff width=85 rowspan=2>Packetization</td>"
		);
#endif

	websWrite(wp,
		"<td bgColor=#ddeeff colspan=%d>Precedence</td>",
		SUPPORTED_CODEC_MAX
		);

#if defined(CONFIG_RTK_VOIP_G7231) || defined(CONFIG_RTK_VOIP_ILBC)
	websWrite(wp,
		"<td bgColor=#ddeeff width=60 rowspan=2>Mode</td>"
		);
#endif

	websWrite(wp, "</tr>\n");

	// Draw precedence number
	websWrite(wp, "<tr align=center>");
	for (i=0; i<SUPPORTED_CODEC_MAX; i++)
	{
		websWrite(wp, "<td bgColor=#ddeeff>%d</td>", i + 1);
	}
	websWrite(wp, "</tr>\n");

	// Draw Codecs
	for (i=0; i<SUPPORTED_CODEC_MAX; i++)
	{
		// codec name
		websWrite(wp, 
			"<tr>" \
			"<td bgColor=#aaddff>%s</td>", 
			supported_codec_string[i]
			);

// framesize is reserved
#ifdef SUPPORT_CUSTOMIZE_FRAME_SIZE
		// framesize
		websWrite(wp,
			"<td bgColor=#ddeeff>" \
			"<select name=frameSize%d>",
			i
			);

		switch (i)
		{
		case SUPPORTED_CODEC_G711U:
		case SUPPORTED_CODEC_G711A:
			// 10, 20 ... 60 ms
			step = 10;
			loop = 6;
			break;
		case SUPPORTED_CODEC_G723:
			// 30, 60, 90 ms
			step = 30;
			loop = 3;
			break;
		default:
			// 10, 20 ... 90ms
			step = 10;
			loop = 9;
			break;
		}

		for (j=0; j<loop; j++)
			websWrite(wp, 
				"<option %s value=%d>%d ms</option>",
				j == pCfg->frame_size[i] ? "selected" : "",
				j,
				step * (j + 1)
				);

		websWrite(wp, 
			"</select>" \
			"</td>\n"
			);
#endif /* SUPPORT_CUSTOMIZE_FRAME_SIZE */

		// precedence
		for (j=0; j<SUPPORTED_CODEC_MAX; j++)
		{
			websWrite(wp,
				"<td bgColor=#ddeeff align=center>" \
				"<input type=checkbox name=precedence %s onclick=\"checkPrecedence(this, %d, %d)\">" \
				"</td>\n",
				j == pCfg->precedence[i] ? "checked" : "",
				i, j
				);
		}

#if defined(CONFIG_RTK_VOIP_G7231) || defined(CONFIG_RTK_VOIP_ILBC)
#ifdef CONFIG_RTK_VOIP_G7231
		// G723 rate
		if (i == SUPPORTED_CODEC_G723)
		{
			websWrite(wp,
				"<td bgColor=#ddeeff>" \
				"<select name=g7231Rate>" \
				"<option %s>6.3k</option>" \
				"<option %s>5.3k</option>" \
				"</select>" \
				"</td>",
				pCfg->g7231_rate == G7231_RATE63 ? "selected" : "",
				pCfg->g7231_rate == G7231_RATE53 ? "selected" : ""
				);
		} 
		else
#endif		
#ifdef CONFIG_RTK_VOIP_ILBC
		if (i == SUPPORTED_CODEC_ILBC)
		{
			websWrite(wp,
				"<td bgColor=#ddeeff>" \
				"<select name=iLBC_mode>" \
				"<option %s>30ms</option>" \
				"<option %s>20ms</option>" \
				"</select>" \
				"</td>",
				pCfg->iLBC_mode == ILBC_30MS ? "selected" : "",
				pCfg->iLBC_mode == ILBC_20MS ? "selected" : ""
				);
		} 
		else
#endif
		{
			websWrite(wp, "<td bgColor=#ddeeff></td>");
		}
#endif // support 723 or iLBC

		websWrite(wp, "</tr>\n");
	}
}

void asp_volumne(webs_t wp, int nVolumne)
{
	int i;

	for(i=0; i<10; i++)
	{
		if (i == nVolumne)
			websWrite(wp, "<option selected>%d</option>", (i+1));
		else
			websWrite(wp, "<option>%d</option>", (i+1));				
	}
}

void asp_agc_gainup(webs_t wp, int nagc_db)
{
	int i;

	for(i=0; i<9; i++)
	{
		if (i == nagc_db)
			websWrite(wp, "<option selected>%d</option>", (i+1));
		else
			websWrite(wp, "<option>%d</option>", (i+1));				
	}

}

void asp_agc_gaindown(webs_t wp, int nagc_db)
{
	int i;

	for(i=0; i<9; i++)
	{
		if (i == nagc_db)
			websWrite(wp, "<option selected>-%d</option>", (i+1));
		else
			websWrite(wp, "<option>-%d</option>", (i+1));				
	}

}
void asp_maxDelay(webs_t wp, int nMaxDelay)
{
	int i;

#if 1
	if( nMaxDelay < 13 )
		nMaxDelay = 13;
	else if( nMaxDelay > 30 )
		nMaxDelay = 30;

	for(i=13; i<=30; i++)
	{
		if (nMaxDelay == i)
			websWrite(wp, "<option value=%d selected>%d</option>", i, i * 10);
		else
			websWrite(wp, "<option value=%d >%d</option>", i, i * 10);
	}
#else
	for(i=60; i<=180; i+=30)
	{
		if (nMaxDelay == i)
			websWrite(wp, "<option selected>%d</option>", i);
		else
			websWrite(wp, "<option>%d</option>", i);
	}
#endif
}

void asp_echoTail(webs_t wp, int nEchoTail)
{
	int i;
	char option[] = {1, 2, 4, 8, 16, 32};

	for(i=0; i<sizeof(option); i++)
	{
		if (option[i] == nEchoTail)
			websWrite(wp, "<option selected>%d</option>", option[i]);
		else
			websWrite(wp, "<option>%d</option>", option[i]);
	}
}

void asp_jitterDelay(webs_t wp, int nJitterDelay)
{
	int i;
	
	if( nJitterDelay < 4 || nJitterDelay > 10 )
		nJitterDelay = 4;
	
	for( i = 4; i <= 10; i ++ ) {
		if( i == nJitterDelay )
			websWrite(wp, "<option value=%d selected>%d</option>", i, i * 10);
		else
			websWrite(wp, "<option value=%d>%d</option>", i, i * 10);
	}
}

void asp_jitterFactor(webs_t wp, int nJitterFactor)
{
	int i;
	
	if( nJitterFactor < 0 || nJitterFactor > 13 )
		nJitterFactor = 7;
	
	for( i = 1; i <= 13; i ++ ) {
		if( i == nJitterFactor )
			websWrite(wp, "<option value=%d selected>%d</option>", i, i );
		else
			websWrite(wp, "<option value=%d>%d</option>", i, i );
	}
}

void asp_sip_speed_dial(webs_t wp, voipCfgPortParam_t *pCfg)
{
	int i;

	for (i=0; i<MAX_SPEED_DIAL; i++)
	{
		websWrite(wp, 
			"<tr bgcolor=#ddeeff>" \
			"<td align=center>%d</td>", i);

		websWrite(wp,
			"<td><input type=text id=spd_name name=spd_name%d size=10 maxlength=%d value=\"%s\"></td>",
			i, MAX_SPEED_DIAL_NAME - 1, pCfg->speed_dial[i].name);

		websWrite(wp,
			"<td><input type=text id=spd_url name=spd_url%d size=20 maxlength=%d value=\"%s\" onChange=\"spd_dial_edit()\"></td>",
			i, MAX_SPEED_DIAL_URL - 1, pCfg->speed_dial[i].url);

		websWrite(wp,
			"<td align=center><input type=checkbox name=spd_sel %s></td>",
			pCfg->speed_dial[i].url[0] ? "" : "disabled");

		websWrite(wp, "</tr>");
	}
}

#if CONFIG_RTK_VOIP_PACKAGE_865X
int asp_voip_GeneralGet(webs_t wp, int argc, char_t **argv)
#else
int asp_voip_GeneralGet(int ejid, webs_t wp, int argc, char_t **argv)
#endif
{
	voipCfgParam_t *pVoIPCfg;
	voipCfgPortParam_t *pCfg;
	int i;
	int voip_port;

	if (voip_flash_get(&pVoIPCfg) != 0)
		return -1;

	voip_port = atoi(websGetVar(wp, T("port"), "0"));
	if (voip_port < 0 || voip_port >= VOIP_PORTS)
		return -1;

	pCfg = &pVoIPCfg->ports[voip_port];

	if (strcmp(argv[0], "voip_port")==0)
	{
		websWrite(wp, "%d", voip_port);
	}
	// proxy
	else if (strcmp(argv[0], "proxy")==0)
	{
		websWrite(wp, "<p><b>Default Proxy</b>\n"  \
			"<table cellSpacing=1 cellPadding=2 border=0>\n" \
			"<tr>\n" \
			"<td bgColor=#aaddff width=155>Select Default Proxy</td>\n" \
			"<td bgColor=#ddeeff width=170>"
		);

		websWrite(wp, "<select name=default_proxy onchange=\"default_proxy_change()\">");
		for (i=0; i<MAX_PROXY ;i++)
		{
			websWrite(wp, "<option value=%d %s>Proxy%d</option>",
				i,
				i == pCfg->default_proxy ? "selected" : "",
				i
			);
		}

		websWrite(wp, "</select>");
		websWrite(wp, "</td></tr></table>");

		for (i=0; i<MAX_PROXY; i++)
		{
			websWrite(wp, "<p><b>Proxy%d</b>\n", i);

			// account
			websWrite(wp, 
				"<table cellSpacing=1 cellPadding=2 border=0>\n" \
				"<tr>\n" \
				"<td bgColor=#aaddff width=155>Display Name</td>\n" \
				"<td bgColor=#ddeeff width=170>\n" \
				"<input type=text id=display name=display%d size=20 maxlength=39 value=%s></td>\n" \
				"</tr>\n",
				i, pCfg->proxies[i].display_name);
			websWrite(wp,
				"<tr>\n" \
				"<td bgColor=#aaddff>Number</td>\n" \
 				"<td bgColor=#ddeeff>\n" \
				"<input type=text id=number name=number%d size=20 maxlength=39 value=%s></td>\n" \
				"</tr>\n",
				i, pCfg->proxies[i].number);
			websWrite(wp,
				"<tr>\n" \
				"<td bgColor=#aaddff>Login ID</td>\n" \
				"<td bgColor=#ddeeff>\n" \
				"<input type=text id=loginID name=loginID%d size=20 maxlength=39 value=%s></td>\n" \
				"</tr>\n",
				i, pCfg->proxies[i].login_id);
			websWrite(wp,
				"<tr>\n" \
				"<td bgColor=#aaddff>Password</td>\n" \
				"<td bgColor=#ddeeff>\n" \
				"<input type=password id=password name=password%d size=20 maxlength=39 value=%s></td>\n" \
				"</tr>\n",
				i, pCfg->proxies[i].password);
			// register server
			websWrite(wp,
				"<tr>\n" \
				"<td bgColor=#aaddff>Proxy</td>\n" \
				"<td bgColor=#ddeeff>\n" \
				"<input type=checkbox id=proxyEnable name=proxyEnable%d %s>Enable\n" \
				"</td></tr>\n",
				i, (pCfg->proxies[i].enable & PROXY_ENABLED) ? "checked" : "");
			websWrite(wp,
				"<tr>\n" \
				"<td bgColor=#aaddff>Proxy Addr</td>\n" \
				"<td bgColor=#ddeeff>\n" \
				"<input type=text id=proxyAddr name=proxyAddr%d size=20 maxlength=39 value=%s></td>\n" \
				"</tr>\n",
				i, pCfg->proxies[i].addr); 
			websWrite(wp,
				"<tr>\n" \
				"<td bgColor=#aaddff>Proxy Port</td>\n" \
				"<td bgColor=#ddeeff>\n" \
				"<input type=text id=proxyPort name=proxyPort%d size=10 maxlength=5 value=%d></td>\n" \
				"</tr>\n",
				i, pCfg->proxies[i].port);
			websWrite(wp,
				"<tr>\n" \
				"<td bgColor=#aaddff>SIP Domain</td>\n" \
				"<td bgColor=#ddeeff>" \
				"<input type=text id=domain_name name=domain_name%d size=20 maxlength=39 value=%s></td>\n" \
				"</tr>\n",
				i, pCfg->proxies[i].domain_name);
			websWrite(wp,
				"<tr>\n" \
				"<td bgColor=#aaddff>Reg Expire (sec)</td>\n" \
				"<td bgColor=#ddeeff>\n" \
				"<input type=text id=regExpiry name=regExpiry%d size=20 maxlength=5 value=%d></td>\n"
				"</tr>\n",
				i, pCfg->proxies[i].reg_expire);
			// nat traversal server
			websWrite(wp,
				"<tr>\n" \
				"<td bgColor=#aaddff>Outbound Proxy</td>\n" \
				"<td bgColor=#ddeeff><input type=checkbox id=obEnable name=obEnable%d %s>Enable</td>\n" \
				"</tr>\n",
				i, (pCfg->proxies[i].outbound_enable) ? "checked" : "");
			websWrite(wp,
				"<tr>\n" \
				"<td bgColor=#aaddff>Outbound Proxy Addr</td>\n" \
				"<td bgColor=#ddeeff>" \
				"<input type=text id=obProxyAddr name=obProxyAddr%d size=20 maxlength=39 value=%s></td>\n" \
				"</tr>\n",
				i, pCfg->proxies[i].outbound_addr);
			websWrite(wp,
				"<tr>\n" \
				"<td bgColor=#aaddff>Outbound Proxy Port</td>\n" \
				"<td bgColor=#ddeeff>" \
				"<input type=text id=obProxyPort name=obProxyPort%d size=10 maxlength=5 value=%d></td>\n" \
				"</tr>\n",
				i, pCfg->proxies[i].outbound_port);
			websWrite(wp,
				"<tr>\n"	\
				"<td bgColor=#aaddff>Nortel SoftSwitch</td>\n" \
				"<td bgColor=#ddeeff>\n" \
    			"<input type=checkbox id=proxyNortel name=proxyNortel%d %s %s onclick=\"check_nortel_proxy()\">Enable\n" \
				"</td></tr>\n",
				i, 
				(pCfg->proxies[i].enable & PROXY_NORTEL) ? "checked" : "",
				i == pCfg->default_proxy ? "" : "disabled");
			websWrite(wp,
				"<tr>\n" \
				"<td bgColor=#aaddff>Register Status</td>\n" \
				"<td bgColor=#ddeeff><iframe src=voip_sip_status.asp?port=%d&index=%d " \
				"frameborder=0 height=20 width=160 scrolling=no marginheight=0 marginwidth=0>\n" \
				"</iframe></td>\n" \
				"</tr>\n",
				voip_port, i);
			websWrite(wp, "</table>");
		}
	}
	else if (strcmp(argv[0], "stun")==0)
	{
		websWrite(wp,
			"<tr>\n" \
			"<td bgColor=#aaddff>Stun</td>\n" \
			"<td bgColor=#ddeeff><input type=checkbox id=stunEnable name=stunEnable %s>Enable</td>\n" \
			"</tr>\n",
			(pCfg->stun_enable) ? "checked" : "");
		websWrite(wp,
			"<tr>\n" \
			"<td bgColor=#aaddff>Stun Server Addr</td>\n" \
			"<td bgColor=#ddeeff>\n" \
			"<input type=text id=stunAddr name=stunAddr size=20 maxlength=39 value=%s></td>\n" \
			"</tr>\n",
			pCfg->stun_addr);
		websWrite(wp,
			"<tr>\n" \
			"<td bgColor=#aaddff>Stun Server Port</td>\n" \
			"<td bgColor=#ddeeff>\n" \
			"<input type=text id=stunPort name=stunPort size=10 maxlength=5 value=%d></td>\n" \
			"</tr>\n",
			pCfg->stun_port);
	}
	else if (strcmp(argv[0], "registerStatus")==0) 
	{
		FILE *fh;
		char buf[MAX_VOIP_PORTS * MAX_PROXY];

		i = atoi(websGetVar(wp, T("index"), "0"));
		if (i < 0 || i >= MAX_PROXY)
		{
			printf("Unknown proxy index %d", i);
			websWrite(wp, "%s", "ERROR");
			return 0;
		}
//		fprintf(stderr, "proxy index %d", i);

		if ((pCfg->proxies[i].enable & PROXY_ENABLED) == 0) {
			websWrite(wp, "%s", "Not Registered");
			return 0;
		}
		
		fh = fopen(_PATH_TMP_STATUS, "r");
		if (!fh) {
			printf("Warning: cannot open %s. Limited output.\n", _PATH_TMP_STATUS);
			printf("\nerrno=%d\n", errno);
		}

		memset(buf, 0, sizeof(buf));
		if (fgets(buf, sizeof(buf), fh) == NULL) {
			printf("Web: The content of /tmp/status is NULL!!\n");
			printf("\nerrno=%d\n", errno);
			websWrite(wp, "%s", "ERROR");
		}
		else {
//			fprintf(stderr, "buf is %s.\n", buf);
			switch (buf[voip_port * VOIP_PORTS + i]) {
				case '0':
					websWrite(wp, "%s", "Not Registered");
					break;
				case '1':
					websWrite(wp, "%s", "Registered");
					break;
				default:
					websWrite(wp, "%s", "ERROR");
					break;
			}
		}

		fclose(fh);
	}
	// advanced
	else if (strcmp(argv[0], "sipPort")==0)
		websWrite(wp, "%d", pCfg->sip_port);
	else if (strcmp(argv[0], "sipPorts")==0)
	{
		for (i=0; i<VOIP_PORTS; i++)
		{
			websWrite(wp, 
				"<input type=hidden id=sipPorts name=sipPorts value=\"%d\">", 
				pVoIPCfg->ports[i].sip_port);
		}
	}
	else if (strcmp(argv[0], "rtpPort")==0)
		websWrite(wp, "%d", pCfg->media_port);
	else if (strcmp(argv[0], "rtpPorts")==0)
	{
		for (i=0; i<VOIP_PORTS; i++)
		{
			websWrite(wp, 
				"<input type=hidden id=rtpPorts name=rtpPorts value=\"%d\">", 
				pVoIPCfg->ports[i].media_port);
		}
	}
	else if (strcmp(argv[0], "dtmfMode")==0)
	{
		for (i=0; i<DTMF_MAX; i++)
		{
			if (i == pCfg->dtmf_mode)
				websWrite(wp, "<option selected>%s</option>", dtmf[i]);
			else
				websWrite(wp, "<option>%s</option>", dtmf[i]);
		}
	}
	else if (strcmp(argv[0], "caller_id")==0)
	{
		websWrite(wp, "<select name=caller_id %s>",
			voip_port >= SLIC_CH_NUM ? "disabled" : "");

		for (i=0; i<CID_MAX ;i++)
		{
			if (i == (pCfg->caller_id_mode & 7))
				websWrite(wp, "<option selected>%s</option>", cid[i]);
			else
				websWrite(wp, "<option>%s</option>", cid[i]);
		}

		websWrite(wp, "</select>");
	}
#ifdef SUPPORT_VOICE_QOS
	else if (strcmp(argv[0], "display_voice_qos")==0) 
		websWrite(wp, "%s", "");
	else if (strcmp(argv[0], "voice_qos")==0)
	{
		for (i=0; i<DSCP_MAX ;i++)
		{
			if (i == pCfg->voice_qos)
				websWrite(wp, "<option selected>%s</option>", dscp[i]);
			else
				websWrite(wp, "<option>%s</option>", dscp[i]);
		}
	}
#else
	else if (strcmp(argv[0], "display_voice_qos")==0)
		websWrite(wp, "%s", "style=\"display:none\"");
	else if (strcmp(argv[0], "voice_qos")==0)
	{
		websWrite(wp, "%s", "");
	}		
#endif	
	else if (strcmp(argv[0], "sipInfo_duration")==0)
	{
		if (pCfg->dtmf_mode == DTMF_SIPINFO)
			websWrite(wp, "%d", pCfg->sip_info_duration);
		else
			websWrite(wp, "%d disabled", pCfg->sip_info_duration);
	}
	else if (strcmp(argv[0], "payloadType")==0)
	{
		if (pCfg->dtmf_mode == DTMF_RFC2833)
			websWrite(wp, "%d", pCfg->payload_type);
		else
			websWrite(wp, "%d disabled", pCfg->payload_type);
	}
	else if (strcmp(argv[0], "call_waiting")==0)
		websWrite(wp, "%s", (pCfg->call_waiting_enable) ? "checked" : "");
	else if (strcmp(argv[0], "call_waiting_cid")==0)
	{
		if (pCfg->call_waiting_enable == 0)
			websWrite(wp, "%s", (pCfg->call_waiting_cid) ? "checked disabled" : "disabled");
		else
			websWrite(wp, "%s", (pCfg->call_waiting_cid) ? "checked" : "");
	}
	// forward
	else if (strcmp(argv[0], "CFAll")==0)
	{
   		websWrite(wp, "<input type=\"radio\" name=\"CFAll\" value=0 %s>Off",
   			pCfg->uc_forward_enable == 0 ? "checked" : "");
   		websWrite(wp, "<input type=\"radio\" name=\"CFAll\" value=1 %s>VoIP",
   			pCfg->uc_forward_enable == 1 ? "checked" : "");
		websWrite(wp, "<input type=\"radio\" name=\"CFAll\" value=2 %s %s>PSTN",
			pCfg->uc_forward_enable == 2 ? "checked" : "",
			voip_port < SLIC_CH_NUM ? "disabled" : "");
	}
	else if (strcmp(argv[0], "CFAll_No")==0)
	{
		websWrite(wp, "<input type=text name=CFAll_No size=20 maxlength=39 value=%s>",
			pCfg->uc_forward);
			
	}
	else if (strcmp(argv[0], "CFBusy")==0)
	{
   		websWrite(wp, "<input type=\"radio\" name=\"CFBusy\" value=0 %s %s>Off",
 			pCfg->busy_forward_enable == 0 ? "checked" : "",
			voip_port >= SLIC_CH_NUM ? "disabled" : "");
		websWrite(wp, "<input type=\"radio\" name=\"CFBusy\" value=1 %s %s>VoIP",
    		pCfg->busy_forward_enable == 1 ? "checked" : "",
			voip_port >= SLIC_CH_NUM ? "disabled" : "");
	}
	else if (strcmp(argv[0], "CFBusy_No")==0)
	{
		websWrite(wp, "<input type=text name=CFBusy_No size=20 maxlength=39 value=\"%s\" %s>",
			pCfg->busy_forward,
			voip_port >= SLIC_CH_NUM ? "disabled=true" : "");
	}
	else if (strcmp(argv[0], "CFNoAns")==0)
	{
   		websWrite(wp, "<input type=\"radio\" name=\"CFNoAns\" value=0 %s %s>Off",
			pCfg->na_forward_enable == 0 ? "checked" : "",
			voip_port >= SLIC_CH_NUM ? "disabled" : "");
		websWrite(wp, "<input type=\"radio\" name=\"CFNoAns\" value=1 %s %s>VoIP",
			pCfg->na_forward_enable == 1 ? "checked" : "",
			voip_port >= SLIC_CH_NUM ? "disabled" : "");
	}
	else if (strcmp(argv[0], "CFNoAns_No")==0)
	{
		websWrite(wp, "<input type=text name=CFNoAns_No size=20 maxlength=39 value=\"%s\" %s>",
			pCfg->na_forward,
			voip_port >= SLIC_CH_NUM ? "disabled=true" : "");
	}
	else if (strcmp(argv[0], "CFNoAns_Time")==0)
	{
		websWrite(wp, "<input type=text name=CFNoAns_Time size=20 maxlength=39 value=%d %s>",
			pCfg->na_forward_time,
			voip_port >= SLIC_CH_NUM ? "disabled=true" : "");
	}
	// Speed dial
	else if (strcmp(argv[0], "speed_dial_display_title") == 0)
	{
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		/* FXS channel has no speed_dial, but FXO still need it. */
		if( voip_port < SLIC_CH_NUM )
			;
		else 
#endif
		{
			websWrite(wp, "<p>\n<b>Speed Dial</b>\n" );
		}
	}
	else if (strcmp(argv[0], "speed_dial_display") == 0)
	{
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		/* FXS channel has no speed_dial, but FXO still need it. */
		if( voip_port < SLIC_CH_NUM )
			websWrite(wp, "style=\"display:none\"");
#endif
	}
	else if (strcmp(argv[0], "speed_dial")==0)
		asp_sip_speed_dial(wp, pCfg);
#ifdef CONFIG_RTK_VOIP_DIALPLAN
	else if (strcmp(argv[0], "display_dialplan_title")==0) {
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		/* FXS channel has no dialplan, but FXO still need it. */
		if( voip_port < SLIC_CH_NUM )
			;
		else
#endif
		{
			websWrite(wp, "%s", "<p><b>Dial plan</b>");
		}
	} else if (strcmp(argv[0], "display_dialplan")==0) {
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		/* FXS channel has no dialplan, but FXO still need it. */
		if( voip_port < SLIC_CH_NUM )
			websWrite(wp, "style=\"display:none\"");
		else
#endif
		{
			websWrite(wp, "%s", "");
		}
	} else if (strcmp(argv[0], "dialplan")==0)
		websWrite(wp, "%s", pCfg->dialplan);
	else if (strcmp(argv[0], "ReplaceRuleOption")==0) {
    	websWrite(wp, "<input type=\"radio\" name=\"ReplaceRuleOption\" value=1 %s>On",
    				( pCfg ->replace_rule_option ? "checked" : "" ) );
    	websWrite(wp, "<input type=\"radio\" name=\"ReplaceRuleOption\" value=0 %s>Off",
    				( !pCfg ->replace_rule_option ? "checked" : "" ) );
	} else if (strcmp(argv[0], "ReplaceRuleSource")==0)
		websWrite(wp, "%s", pCfg->replace_rule_source);
	else if (strcmp(argv[0], "ReplaceRuleTarget")==0)
		websWrite(wp, "%s", pCfg->replace_rule_target);
	else if (strcmp(argv[0], "AutoPrefix")==0)
		websWrite(wp, "%s", pCfg->auto_prefix);
	else if (strcmp(argv[0], "PrefixUnsetPlan")==0)
		websWrite(wp, "%s", pCfg->prefix_unset_plan);
#else
	else if (strcmp(argv[0], "display_dialplan_title")==0)
		websWrite(wp, "%s", "");
  	else if (strcmp(argv[0], "display_dialplan")==0)
  		websWrite(wp, "%s", "style=\"display:none\"");
  	else if (strcmp(argv[0], "dialplan")==0)
		websWrite(wp, "%s", "");
  	else if (strcmp(argv[0], "ReplaceRuleOption")==0)
    	websWrite(wp, "%s", "");
	else if (strcmp(argv[0], "ReplaceRuleSource")==0)
		websWrite(wp, "%s", "");
	else if (strcmp(argv[0], "ReplaceRuleTarget")==0)
		websWrite(wp, "%s", "");
	else if (strcmp(argv[0], "AutoPrefix")==0)
		websWrite(wp, "%s", "");
	else if (strcmp(argv[0], "PrefixUnsetPlan")==0)
		websWrite(wp, "%s", "");
#endif /* CONFIG_RTK_VOIP_DIALPLAN */

	// DSP
	else if (strcmp(argv[0], "codec_var") == 0)
		asp_sip_codec_var(wp, pCfg);
	else if (strcmp(argv[0], "codec") == 0)
		asp_sip_codec(wp, pCfg);
	else if (strcmp(argv[0], "slic_txVolumne")==0)
		asp_volumne(wp, pCfg->slic_txVolumne);
	else if (strcmp(argv[0], "slic_rxVolumne")==0)
		asp_volumne(wp, pCfg->slic_rxVolumne);
	else if (strcmp(argv[0], "maxDelay")==0)
		asp_maxDelay(wp, pCfg->maxDelay);
	else if (strcmp(argv[0], "echoTail")==0)
		asp_echoTail(wp, pCfg->echoTail);
	else if (strcmp(argv[0], "flash_hook_time")==0)
	{
		websWrite(wp, "<input type=text name=flash_hook_time_min size=4 maxlength=5 value=%d %s>" \
			" <  Flash Time  < " \
			"<input type=text name=flash_hook_time size=4 maxlength=5 value=%d %s>",
			pCfg->flash_hook_time_min,
			voip_port >= SLIC_CH_NUM ? "disabled" : "",
			pCfg->flash_hook_time,
			voip_port >= SLIC_CH_NUM ? "disabled" : ""
		);
	}
	else if (strcmp(argv[0], "spk_voice_gain")==0)
		websWrite(wp, "%d", pCfg->spk_voice_gain);
	else if (strcmp(argv[0], "mic_voice_gain")==0)
		websWrite(wp, "%d", pCfg->mic_voice_gain);
	else if (strcmp(argv[0], "useVad")==0)
		websWrite(wp, "%s", (pCfg->vad) ? T("checked") : T(""));
	else if (strcmp(argv[0], "CFuseSpeaker")==0)
		websWrite(wp, "%s", (pCfg->speaker_agc) ? T("checked") : T(""));
	else if (strcmp(argv[0], "CF_spk_AGC_level")==0)
		asp_agc_gainup(wp, pCfg->spk_agc_lvl);	
	else if (strcmp(argv[0], "CF_spk_AGC_up_limit")==0)
		asp_agc_gainup(wp, pCfg->spk_agc_gu);
	else if (strcmp(argv[0], "CF_spk_AGC_down_limit")==0)
		asp_agc_gaindown(wp, pCfg->spk_agc_gd);
	else if (strcmp(argv[0], "CFuseMIC")==0)
		websWrite(wp, "%s", (pCfg->mic_agc) ? T("checked") : T(""));
	else if (strcmp(argv[0], "CF_mic_AGC_level")==0)
		asp_agc_gainup(wp, pCfg->mic_agc_lvl);	
	else if (strcmp(argv[0], "CF_mic_AGC_up_limit")==0)
		asp_agc_gainup(wp, pCfg->mic_agc_gu);
	else if (strcmp(argv[0], "CF_mic_AGC_down_limit")==0)
		asp_agc_gaindown(wp, pCfg->mic_agc_gd);
	else if (strcmp(argv[0], "FSKdatesync")==0)
	{
		websWrite(wp, "<input type=checkbox name=FSKdatesync size=20 %s %s>Enable",
			(pCfg->caller_id_mode & 0x080) ? T("checked") : T(""), 
			voip_port >= SLIC_CH_NUM ? "disabled" : "");
	}
	else if (strcmp(argv[0], "revPolarity")==0)
	{
		websWrite(wp, "<input type=checkbox name=revPolarity size=20 %s %s>Enable",
			(pCfg->caller_id_mode & 0x040) ? T("checked") : T(""),
			voip_port >= SLIC_CH_NUM ? "disabled" : "");
	}
	else if (strcmp(argv[0], "sRing")==0)
	{
		websWrite(wp, "<input type=checkbox name=sRing size=20 %s %s>Enable",
			(pCfg->caller_id_mode & 0x020) ? T("checked") : T(""),
			voip_port >= SLIC_CH_NUM ? "disabled" : "");
	}
	else if (strcmp(argv[0], "dualTone")==0)
	{
		websWrite(wp, "<input type=checkbox name=dualTone size=20 %s %s>Enable",
			(pCfg->caller_id_mode & 0x010) ? T("checked") : T(""),
			voip_port >= SLIC_CH_NUM ? "disabled" : "");
	}
	else if (strcmp(argv[0], "PriorRing")==0)
	{
		websWrite(wp, "<input type=checkbox name=PriorRing size=20 %s>Enable",
			(pCfg->caller_id_mode & 0x008) ? T("checked") : T(""));
	}
	else if (strcmp(argv[0], "cid_dtmfMode_S")==0)
	{
		websWrite(wp, "<select name=cid_dtmfMode_S %s>", 
			voip_port >= SLIC_CH_NUM ? "disabled" : "");

		for (i=0; i<CID_DTMF_MAX; i++)
		{
			if (i == (pCfg->cid_dtmf_mode & 0x03))
				websWrite(wp, "<option selected>%s</option>", cid_dtmf[i]);
			else
				websWrite(wp, "<option>%s</option>", cid_dtmf[i]);
		}

		websWrite(wp, "</select>");
	}
	else if (strcmp(argv[0], "cid_dtmfMode_E")==0)
	{
		websWrite(wp, "<select name=cid_dtmfMode_E %s>",
			voip_port >= SLIC_CH_NUM ? "disabled" : "");

		for (i=0; i<CID_DTMF_MAX; i++)
		{
			if (i == ((pCfg->cid_dtmf_mode>>2) & 0x03))
				websWrite(wp, "<option selected>%s</option>", cid_dtmf[i]);
			else
				websWrite(wp, "<option>%s</option>", cid_dtmf[i]);
		}
	
		websWrite(wp, "</select>");
	}
	else if (strcmp(argv[0], "SoftFskGen")==0)
	{
		websWrite(wp, "<input type=checkbox name=SoftFskGen size=20 %s %s>Enable",
			pCfg->cid_fsk_gen_mode ? T("checked") : T(""),
			voip_port >= SLIC_CH_NUM ? "disabled" : "");
	}
	else if (strcmp(argv[0], "jitterDelay")==0)
		asp_jitterDelay(wp, pCfg->jitter_delay);
	else if (strcmp(argv[0], "jitterFactor")==0)
		asp_jitterFactor(wp, pCfg->jitter_factor);
#ifdef CONFIG_RTK_VOIP_T38	/*kernel config true*/
	else if (strcmp(argv[0], "T38_BUILD")==0)
		websWrite(wp, "%s", "");
	//T.38 config
	else if(strcmp(argv[0], "useT38")==0)
		websWrite(wp, "%s", (pCfg->useT38) ? "checked" : "");
	else if(strcmp(argv[0], "T38_PORT")==0)
		websWrite(wp, "%d", pCfg->T38_port);
	else if(strcmp(argv[0], "t38Ports")==0)
	{
		for (i=0; i<VOIP_PORTS; i++)
		{
			websWrite(wp, 
				"<input type=hidden id=t38Ports name=t38Ports value=\"%d\">", 
				pVoIPCfg->ports[i].T38_port);
		}
	}
#else /*CONFIG_RTK_VOIP_T38*/
	else if (strcmp(argv[0], "T38_BUILD")==0)
		websWrite(wp, "%s", "style=\"display:none\"");
	else if(strcmp(argv[0], "useT38")==0)
		websWrite(wp, "%s", "");
	else if(strcmp(argv[0], "T38_PORT")==0)
		websWrite(wp, "%d",0);
#endif /*CONFIG_RTK_VOIP_T38*/
	else if (strcmp(argv[0], "hotline_option_display_title") == 0)
	{
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		/* FXS channel has no hotline option, but FXO still need it. */
		if( voip_port < SLIC_CH_NUM )
			;
		else
#endif
		{
			websWrite(wp, "<p>\n<b>Hot Line</b>\n");
		}
	}
	else if (strcmp(argv[0], "hotline_option_display") == 0)
	{
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		/* FXS channel has no hotline option, but FXO still need it. */
		if( voip_port < SLIC_CH_NUM )
			websWrite(wp, "style=\"display:none\"");
#endif
	}
	else if (strcmp(argv[0], "hotline_enable") == 0)
	{
		websWrite(wp, "%s", (pCfg->hotline_enable) ? "checked" : "");
	}
	else if (strcmp(argv[0], "hotline_number") == 0)
	{
		websWrite(wp, "%s", pCfg->hotline_number);
	}
	else if (strcmp(argv[0], "dnd_always") == 0)
	{
		websWrite(wp, "%s", (pCfg->dnd_mode == 2) ? "checked" : "");
	}
	else if (strcmp(argv[0], "dnd_enable") == 0)
	{
		websWrite(wp, "%s", (pCfg->dnd_mode == 1) ? "checked" : "");
	}
	else if (strcmp(argv[0], "dnd_disable") == 0)
	{
		websWrite(wp, "%s", (pCfg->dnd_mode == 0) ? "checked" : "");
	}
	else if (strcmp(argv[0], "dnd_from_hour") == 0)
	{
		websWrite(wp, "%.2d", pCfg->dnd_from_hour);
	}
	else if (strcmp(argv[0], "dnd_from_min") == 0)
	{
		websWrite(wp, "%.2d", pCfg->dnd_from_min);
	}
	else if (strcmp(argv[0], "dnd_to_hour") == 0)
	{
		websWrite(wp, "%.2d", pCfg->dnd_to_hour);
	}
	else if (strcmp(argv[0], "dnd_to_min") == 0)
	{
		websWrite(wp, "%.2d", pCfg->dnd_to_min);
	}
	else
	{
		return -1;
	}

	return 0;
}

void asp_voip_GeneralSet(webs_t wp, char_t *path, char_t *query)
{
	voipCfgParam_t *pVoIPCfg;
	voipCfgPortParam_t *pCfg;
	int i;
	char *ptr;
	char szFrameSize[10], szPrecedence[12];
	char redirect_url[50];
	int voip_port;
	char szName[20];

	if (voip_flash_get(&pVoIPCfg) != 0)
		return;

	voip_port = atoi(websGetVar(wp, T("voipPort"), "0"));
	if (voip_port < 0 || voip_port >= VOIP_PORTS)
		return;

	pCfg = &pVoIPCfg->ports[voip_port];

	pCfg->default_proxy = atoi(websGetVar(wp, T("default_proxy"), "0"));

	/* Sip Proxy */ 
	for (i=0; i< MAX_PROXY; i++)
	{
		/* Sip Account */ 
		sprintf(szName, "display%d", i);
		strcpy(pCfg->proxies[i].display_name, websGetVar(wp, szName, T("")));
		sprintf(szName, "number%d", i);
		strcpy(pCfg->proxies[i].number, websGetVar(wp, szName, T("")));
		sprintf(szName, "loginID%d", i);
		strcpy(pCfg->proxies[i].login_id, websGetVar(wp, szName, T("")));
		sprintf(szName, "password%d", i);
		strcpy(pCfg->proxies[i].password, websGetVar(wp, szName, T("")));

		/* Register Server */ 
		pCfg->proxies[i].enable = 0;

		sprintf(szName, "proxyEnable%d", i);
		if (gstrcmp(websGetVar(wp, szName, T("")), "on") == 0)
			pCfg->proxies[i].enable |= PROXY_ENABLED;

		sprintf(szName, "proxyNortel%d", i);
		if (gstrcmp(websGetVar(wp, szName, T("")), "on") == 0)
			pCfg->proxies[i].enable |= PROXY_NORTEL;

		sprintf(szName, "proxyAddr%d", i);
		strcpy(pCfg->proxies[i].addr, websGetVar(wp, szName, T("")));

		sprintf(szName, "proxyPort%d", i);
		pCfg->proxies[i].port = atoi(websGetVar(wp, szName, T("5060")));
		if (pCfg->proxies[i].port == 0) 
			pCfg->proxies[i].port = 5060;

		sprintf(szName, "domain_name%d", i);
		strcpy(pCfg->proxies[i].domain_name, websGetVar(wp, szName, T("")));

		sprintf(szName, "regExpiry%d", i);
		pCfg->proxies[i].reg_expire = atoi(websGetVar(wp, szName, T("60")));

		/* NAT Travsersal Server */ 
		sprintf(szName, "obEnable%d", i);
		pCfg->proxies[i].outbound_enable = !gstrcmp(websGetVar(wp, szName, T("")), "on");
		sprintf(szName, "obProxyPort%d", i);
		pCfg->proxies[i].outbound_port = atoi(websGetVar(wp, szName, T("5060")));
		if (pCfg->proxies[i].outbound_port == 0)
			pCfg->proxies[i].outbound_port = 5060;

		sprintf(szName, "obProxyAddr%d", i);
		strcpy(pCfg->proxies[i].outbound_addr, websGetVar(wp, szName, T("")));
	}

	/* NAT Traversal */
	pCfg->stun_enable = !gstrcmp(websGetVar(wp, "stunEnable", T("")), "on");
	pCfg->stun_port	= atoi(websGetVar(wp, "stunPort", T("3478")));
	strcpy(pCfg->stun_addr, websGetVar(wp, "stunAddr", T("")));

	/* Advanced */
	pCfg->sip_port 	= atoi(websGetVar(wp, T("sipPort"), T("5060")));
	pCfg->media_port 	= atoi(websGetVar(wp, T("rtpPort"), T("9000")));

	ptr	 = websGetVar(wp, T("dtmfMode"), T(""));
	for(i=0; i<DTMF_MAX; i++)
	{
		if (!gstrcmp(ptr, dtmf[i]))
			break;
	}
	if (i == DTMF_MAX)
		i = DTMF_INBAND;
	
	pCfg->dtmf_mode 		= i;
	pCfg->payload_type 		= atoi(websGetVar(wp, T("payloadType"), T("96")));
	pCfg->sip_info_duration		= atoi(websGetVar(wp, T("sipInfo_duration"), T("250")));
	pCfg->call_waiting_enable = !gstrcmp(websGetVar(wp, T("call_waiting"), T("")), "on");
	pCfg->call_waiting_cid = !gstrcmp(websGetVar(wp, T("call_waiting_cid"), T("")), "on");
	
	/* Forward Mode */ 
	//pCfg->uc_forward_enable = !gstrcmp(websGetVar(wp, T("CFAll"), T("")), "on");
	pCfg ->uc_forward_enable = atoi( websGetVar(wp, T("CFAll"), T("")) );
	strcpy(pCfg->uc_forward, websGetVar(wp, T("CFAll_No"), T("")));

	//pCfg->busy_forward_enable = !gstrcmp(websGetVar(wp, T("CFBusy"), T("")), "on");
	pCfg ->busy_forward_enable = atoi( websGetVar(wp, T("CFBusy"), T("")) );
	strcpy(pCfg->busy_forward, websGetVar(wp, T("CFBusy_No"), T("")));

	//pCfg->na_forward_enable = !gstrcmp(websGetVar(wp, T("CFNoAns"), T("")), "on");
	pCfg ->na_forward_enable = atoi( websGetVar(wp, T("CFNoAns"), T("")) );
	pCfg->na_forward_time	  	= atoi(websGetVar(wp, T("CFNoAns_Time"), T("")));
	strcpy(pCfg->na_forward, websGetVar(wp, T("CFNoAns_No"), T("")));

	/* Speed Dial */
	for (i=0; i<MAX_SPEED_DIAL; i++)
	{
		char szBuf[20];

		sprintf(szBuf, "spd_name%d", i);
		strcpy(pCfg->speed_dial[i].name, websGetVar(wp, szBuf, T("")));
		sprintf(szBuf, "spd_url%d", i);
		strcpy(pCfg->speed_dial[i].url, websGetVar(wp, szBuf, T("")));
	}

#ifdef CONFIG_RTK_VOIP_DIALPLAN
	/* Dial Plan */
	strcpy(pCfg->dialplan, websGetVar(wp, T("dialplan"), T("")));
	pCfg ->replace_rule_option = atoi( websGetVar(wp, T("ReplaceRuleOption"), T("")) );
	strcpy(pCfg->replace_rule_source, websGetVar(wp, T("ReplaceRuleSource"), T("")));
	strcpy(pCfg->replace_rule_target, websGetVar(wp, T("ReplaceRuleTarget"), T("")));
	strcpy(pCfg->auto_prefix, websGetVar(wp, T("AutoPrefix"), T("")));
	strcpy(pCfg->prefix_unset_plan, websGetVar(wp, T("PrefixUnsetPlan"), T("")));		
#endif
	
	/* DSP */
	for(i=0; i<SUPPORTED_CODEC_MAX; i++)
	{
		sprintf(szFrameSize, T("frameSize%d"), i);
		pCfg->frame_size[i] = atoi(websGetVar(wp, szFrameSize, T("0")));
		sprintf(szPrecedence, T("preced%d"), i);
		pCfg->precedence[i] = atoi(websGetVar(wp, szPrecedence, T("-1")));
	}

	pCfg->vad = !gstrcmp(websGetVar(wp, T("useVad"), T("")), "on");	
	pCfg->speaker_agc = !gstrcmp(websGetVar(wp, T("CFuseSpeaker"), T("")), "on");
	pCfg->spk_agc_lvl = atoi(websGetVar(wp, T("CF_spk_AGC_level"), T("1"))) - 1;
	pCfg->spk_agc_gu = atoi(websGetVar(wp, T("CF_spk_AGC_up_limit"), T("6"))) - 1;
	pCfg->spk_agc_gd = (-(atoi(websGetVar(wp, T("CF_spk_AGC_down_limit"), T("-6"))))) - 1 ;	
	pCfg->mic_agc = !gstrcmp(websGetVar(wp, T("CFuseMIC"), T("")), "on");
	pCfg->mic_agc_lvl = atoi(websGetVar(wp, T("CF_mic_AGC_level"), T("1"))) - 1;
	pCfg->mic_agc_gu = atoi(websGetVar(wp, T("CF_mic_AGC_up_limit"), T("6"))) - 1;
	pCfg->mic_agc_gd = (-(atoi(websGetVar(wp, T("CF_mic_AGC_down_limit"), T("-6"))))) - 1 ;		
	pCfg->g7231_rate = (gstrcmp(websGetVar(wp, T("g7231Rate"), T("")), "5.3k")) ? G7231_RATE63 :  G7231_RATE53;
	pCfg->iLBC_mode = (gstrcmp(websGetVar(wp, T("iLBC_mode"), T("")), "30ms") == 0) ? ILBC_30MS : ILBC_20MS ;
	pCfg->slic_txVolumne = atoi(websGetVar(wp, T("slic_txVolumne"), T("1"))) - 1;
	pCfg->slic_rxVolumne = atoi(websGetVar(wp, T("slic_rxVolumne"), T("1"))) - 1;
	pCfg->maxDelay  = atoi(websGetVar(wp, T("maxDelay"), T("13")));
	pCfg->echoTail  = atoi(websGetVar(wp, T("echoTail"), T("2")));
	pCfg->jitter_delay = atoi(websGetVar(wp, T("jitterDelay"), T("4")));
	pCfg->jitter_factor = atoi(websGetVar(wp, T("jitterFactor"), T("7")));

	ptr	 = websGetVar(wp, T("caller_id"), T(""));
	for(i=0; i<CID_MAX; i++)
	{
		if (!gstrcmp(ptr, cid[i]))
			break;
	}
	if (i == CID_MAX)
		i = CID_DTMF;
	
	if(!gstrcmp(websGetVar(wp, T("FSKdatesync"), T("")), "on"))
		i=i | 0x80U;
	
	if(!gstrcmp(websGetVar(wp, T("revPolarity"), T("")), "on"))
		i=i | 0x40U;
			
	if(!gstrcmp(websGetVar(wp, T("sRing"), T("")), "on"))
		i=i | 0x20U;	
	
	if(!gstrcmp(websGetVar(wp, T("dualTone"), T("")), "on"))
		i=i | 0x10U;
		
	if(!gstrcmp(websGetVar(wp, T("PriorRing"), T("")), "on"))
		i=i | 0x08U;
	
	pCfg->caller_id_mode = i;
	
	ptr	 = websGetVar(wp, T("cid_dtmfMode_S"), T(""));
	for(i=0; i<CID_DTMF_MAX; i++)
	{
		if (!gstrcmp(ptr, cid_dtmf[i]))
			break;
	}
	if (i == CID_DTMF_MAX)
		i = CID_DTMF_D;
	pCfg->cid_dtmf_mode = i;
	
	ptr	 = websGetVar(wp, T("cid_dtmfMode_E"), T(""));
	for(i=0; i<CID_DTMF_MAX; i++)
	{
		if (!gstrcmp(ptr, cid_dtmf[i]))
			break;
	}
	if (i == CID_DTMF_MAX)
		i = CID_DTMF_D;
	pCfg->cid_dtmf_mode |= (i<<2);
	
	pCfg->cid_fsk_gen_mode = !gstrcmp(websGetVar(wp, T("SoftFskGen"), T("")), "on");

        i = atoi(websGetVar(wp, T("flash_hook_time"), T("")));
	if ((i >= 100) && ( i <= 2000))
		pCfg->flash_hook_time = i;
	else
		pCfg->flash_hook_time = 300;
		
        i = atoi(websGetVar(wp, T("flash_hook_time_min"), T("")));
	if ( i >= pCfg->flash_hook_time )
		pCfg->flash_hook_time_min = 0; 
	else
		pCfg->flash_hook_time_min = i;

	i = atoi(websGetVar(wp, T("spk_voice_gain"), T("")));
	if ((i>= -32) && (i<=31) )
		pCfg->spk_voice_gain=i;
	else
		pCfg->spk_voice_gain=0;

	i = atoi(websGetVar(wp, T("mic_voice_gain"), T("")));
	if ((i>= -32) && (i<=31) )
		pCfg->mic_voice_gain=i;
	else
		pCfg->mic_voice_gain=0;

#ifdef SUPPORT_VOICE_QOS
	ptr	 = websGetVar(wp, T("voice_qos"), T(""));
	for(i=0; i<DSCP_MAX; i++)
	{
		if (!gstrcmp(ptr, dscp[i]))
			break;
	}
	if (i == DSCP_MAX)
		i = DSCP_CS0;
	
	pCfg->voice_qos	= i;
#endif
		 
/*++T.38 added by Jack Chan 24/01/07 for VoIP++*/
#ifdef CONFIG_RTK_VOIP_T38
	pCfg->useT38 = !gstrcmp(websGetVar(wp, T("useT38"), T("")), "on");
	pCfg->T38_port = atoi(websGetVar(wp, T("T38_PORT"), T("49172")));
#else /*CONFIG_RTK_VOIP_T38*/
	pCfg->useT38 = 0;
	pCfg->T38_port = 0;
#endif /*#ifdef CONFIG_RTK_VOIP_T38*/
/*--end--*/

	// Hot line
	pCfg->hotline_enable = strcmp(websGetVar(wp, T("hotline_enable"), T("")), "on") == 0;
	if (pCfg->hotline_enable)
	{
		strcpy(pCfg->hotline_number, websGetVar(wp, T("hotline_number"), T("")));
	}

	// DND
	pCfg->dnd_mode = atoi(websGetVar(wp, T("dnd_mode"), T("0")));
	if (pCfg->dnd_mode == 1)
	{
		pCfg->dnd_from_hour = atoi(websGetVar(wp, T("dnd_from_hour"), T("0")));
		pCfg->dnd_from_min = atoi(websGetVar(wp, T("dnd_from_min"), T("0")));
		pCfg->dnd_to_hour = atoi(websGetVar(wp, T("dnd_to_hour"), T("0")));
		pCfg->dnd_to_min = atoi(websGetVar(wp, T("dnd_to_min"), T("0")));
	}

	voip_flash_set(pVoIPCfg);

	web_restart_solar();

	sprintf(redirect_url, "/voip_general.asp?port=%d", voip_port);
	websRedirect(wp, redirect_url);
}

#if CONFIG_RTK_VOIP_PACKAGE_865X
int asp_voip_DialPlanGet(webs_t wp, int argc, char_t **argv)
#else
int asp_voip_DialPlanGet(int ejid, webs_t wp, int argc, char_t **argv)
#endif
{
	return 0;
}

void asp_voip_DialPlanSet(webs_t wp, char_t *path, char_t *query)
{
	websRedirect(wp, T("/dialplan.asp"));
}
