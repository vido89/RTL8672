#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include "web_voip.h"
#if CONFIG_RTK_VOIP_PACKAGE_867X
#include <fcntl.h>
#endif /* CONFIG_RTK_VOIP_PACKAGE_867X */


#define FIFO_SOLAR "/var/run/solar_control.fifo"

char *sipVersion = VOIP_SIP_VERSION;
char *dspVersion = VOIP_DSP_VERSION;

static void web_voip_abort(int sig)
{
	printf("webs: web_voip_abort\n");
	voip_flash_server_stop();
	exit(0);
}

int web_voip_init()
{
	FILE *fh;
	char buf[MAX_VOIP_PORTS * MAX_PROXY];

	/* To show the register status on Web page. */
	fh = fopen(_PATH_TMP_STATUS, "w");
	if (!fh) {
		printf("Warning: cannot open %s. Limited output.\n", _PATH_TMP_STATUS);
		printf("\nerrno=%d\n", errno);
	}
	else {
		memset(buf, (int) '0', sizeof(buf));
		fwrite(buf, 1, sizeof(buf), fh);
		fclose(fh);
	}

	// do web init
	websAspDefine(T("voip_general_get"), asp_voip_GeneralGet);
	websFormDefine(T("voip_general_set"), asp_voip_GeneralSet);
	websAspDefine(T("voip_dialplan_get"), asp_voip_DialPlanGet);
	websFormDefine(T("voip_dialplan_set"), asp_voip_DialPlanSet);
	websAspDefine(T("voip_tone_get"), asp_voip_ToneGet);
	websFormDefine(T("voip_tone_set"), asp_voip_ToneSet);
	websAspDefine(T("voip_ring_get"), asp_voip_RingGet);
	websFormDefine(T("voip_ring_set"), asp_voip_RingSet);
	websAspDefine(T("voip_other_get"), asp_voip_OtherGet);
	websFormDefine(T("voip_other_set"), asp_voip_OtherSet);
	websAspDefine(T("voip_config_get"), asp_voip_ConfigGet);
	websFormDefine(T("voip_config_set"), asp_voip_ConfigSet);
	websFormDefine(T("voip_config_save"), asp_voip_ConfigSave);
	websFormDefine(T("voip_config_load"), asp_voip_ConfigLoad);
//add for fxo
#if 1
	websFormDefine(T("voip_fxo_set"), asp_voip_FxoSet);
	websAspDefine(T("voip_fxo_get"), asp_voip_FxoGet);	
#endif // add fxo asp	
#ifdef CONFIG_RTK_VOIP_IVR
	websFormDefine(T("voip_ivrreq_set"), asp_voip_IvrReqSet);
#endif

	signal(SIGINT, web_voip_abort);
	signal(SIGTERM, web_voip_abort);

#if CONFIG_RTK_VOIP_PACKAGE_865X
/*If 865x, do voip_flash_server_start() at Init voip in boa/src/rtl/proc/proc_general.c */
#else
	if (voip_flash_server_start() != 0)
	{
		fprintf(stderr, "web_voip_init: voip_flash_server_start failed\n");
		return -1;
	}
#endif

	return 0;
}

int web_restart_solar()
{
	int h_pipe, res, refresh;
	FILE *fh;

	refresh = voip_flash_server_update();
	if (refresh == -1)
		return -1;

	// restart solar, it will reload flash settings
	if (access(FIFO_SOLAR, F_OK) != 0)
	{
		fprintf(stderr, "access %s failed\n", FIFO_SOLAR);
		return -1;
	}

	h_pipe = open(FIFO_SOLAR, O_WRONLY | O_NONBLOCK);
	if (h_pipe == -1)
	{
		fprintf(stderr, "open %s failed\n", FIFO_SOLAR);
		return -1;
	}

#if 0
	if (refresh)
	{
		fprintf(stderr, "web_restart_solar: refresh...\n");
		res = write(h_pipe, "r\n", 2);		// refresh solar
	}
	else
	{
		fprintf(stderr, "web_restart_solar: restart...\n");
		res = write(h_pipe, "x\n", 2);		// restart solar
	}
#else
	// always restatr solar for networking:
	// restart solar for multiple VLANs on WAN port.
	fprintf(stderr, "web_restart_solar: restart...\n");
	res = write(h_pipe, "x\n", 2);		// restart solar
#endif

	if (res == -1)
	{
		fprintf(stderr, "write %s failed\n", FIFO_SOLAR);
		close(h_pipe);
		return -1;
	}

	close(h_pipe);

	/* To show the register status on Web page. */
	fh = fopen(_PATH_TMP_STATUS, "w");
	if (!fh) {
		printf("Warning: cannot open %s. Limited output.\n", _PATH_TMP_STATUS);
		printf("\nerrno=%d\n", errno);
	}
	else {
		fwrite("00", 1, 2, fh);
		printf("Reset register status...\n");
		fclose(fh);
	}

	return 0;
}

void run_init_script_announcement( char *arg )
{
	/* web is going to execute 'init.sh' */
	voip_flash_server_update();
}

#if CONFIG_RTK_VOIP_PACKAGE_865X

int asp_voip_getInfo(webs_t wp, int argc, char_t **argv)
{
	char_t	*name;

   	if (argc != 1) {
   		websWrite(wp, "Insufficient args %d\n", argc);
   		return -1;
   	}	
	name = argv[0];
	
	if (strcmp(name, T("voip_status")) == 0)
	{
		websWrite(wp,
			"<tr>" \
			"<td width=100%% colspan=2 bgcolor=#008000><font color=#FFFFFF size=2><b>VoIP</b></font></td>" \
			"</tr>" \
			"<tr bgcolor=#DDDDDD>" \
			"<td width=40%%><font size=2><b>SIP Version</b></td>" \
			"<td width=60%%><font size=2>%s</td>" \
			"</tr>" \
			"<tr bgcolor=#EEEEEE>" \
			"<td width=40%%><font size=2><b>DSP Version</b></td>" \
			"<td width=60%%><font size=2>%s</td>" \
			"</tr>", sipVersion, dspVersion);
		return 0;
	}
	else if (strcmp(name, T("voip_menu")) == 0)
	{
		websWrite(wp, 
			"<tr><td><span id=spanHead_voip onclick=\"menu_voip()\">" \
			"<a href=#><B>VoIP</a><BR>" \
			"</span>" \
			"<div id=spanMenu_voip STYLE=\"display:none\">" \
			"<table CELLSPACING=0 CELLPADDING=0>");

		for (i=0; i<VOIP_PORTS; i++)
		{
			if (VOIP_PORTS == 1)
				websWrite(wp,
					"<tr><td width=10></td><td><a href=voip_general.asp?port=0 target=show_area><span style=\"font-size: 12pt;\">General</span></a></td></tr>");
			else if (i < SLIC_CH_NUM)
				websWrite(wp,
					"<tr><td width=10></td><td><a href=voip_general.asp?port=%d target=show_area><span style=\"font-size: 12pt;\">FXS %d</span></a></td></tr>", i, i);
			else
				websWrite(wp,
					"<tr><td width=10></td><td><a href=voip_general.asp?port=1 target=show_area><span style=\"font-size: 12pt;\">FXO %d</span></a></td></tr>", i, i - SLIC_CH_NUM);
		}

		websWrite(wp,
			"<tr><td width=10></td><td><a href=voip_tone.asp target=show_area><span style=\"font-size: 12pt;\">Tone</span></a></td></tr>" \
			"<tr><td width=10></td><td><a href=voip_ring.asp target=show_area><span style=\"font-size: 12pt;\">Ring</span></a></td></tr>" \
			"<tr><td width=10></td><td><a href=voip_other.asp target=show_area><span style=\"font-size: 12pt;\">Other</span></a></td></tr>" \
			"<tr><td width=10></td><td><a href=voip_config.asp target=show_area><span style=\"font-size: 12pt;\">Config</span></a></td></tr>");
		websWrite(wp,
			"</table></div>" \
			"</td></tr>");		
		return 0;
	}

	return -1;
}

#else

int asp_voip_getInfo(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t	*name;
	int i;

   	if (ejArgs(argc, argv, T("%s"), &name) < 1) {
   		websError(wp, 400, T("Insufficient args\n"));
   		return -1;
   	}
	
	if (strcmp(name, T("voip_status")) == 0)
	{
		return websWrite(wp,
			"<tr>" \
			"<td width=100%% colspan=2 bgcolor=#008000><font color=#FFFFFF size=2><b>VoIP</b></font></td>" \
			"</tr>" \
			"<tr bgcolor=#DDDDDD>" \
			"<td width=40%%><font size=2><b>SIP Version</b></td>" \
			"<td width=60%%><font size=2>%s</td>" \
			"</tr>" \
			"<tr bgcolor=#EEEEEE>" \
			"<td width=40%%><font size=2><b>DSP Version</b></td>" \
			"<td width=60%%><font size=2>%s</td>" \
			"</tr>", sipVersion, dspVersion);
	}
	else if (strcmp(name, T("voip_menu")) == 0)
	{
		websWrite(wp,"document.write('" \
				"<tr><td><b>VoIP Settings</b></td></tr>");

		for (i=0; i<VOIP_PORTS; i++)
		{
			if (VOIP_PORTS == 1)
				websWrite(wp,
					"<tr><td><a href=voip_general.asp?port=0 target=view>General</a></td></tr>");
			else if (i < SLIC_CH_NUM)
				websWrite(wp,
					"<tr><td><a href=voip_general.asp?port=%d target=view>FXS %d</a></td></tr>",
					i, i);
			else
				websWrite(wp,
					"<tr><td><a href=voip_general.asp?port=%d target=view>FXO %d</a></td></tr>",
					i, i - SLIC_CH_NUM);
		}

		return websWrite(wp,
				"<tr><td><a href=voip_tone.asp target=view>Tone</a></td></tr>" \
				"<tr><td><a href=voip_ring.asp target=view>Ring</a></td></tr>" \
				"<tr><td><a href=voip_other.asp target=view>Other</a></td></tr>" \
				"<tr><td><a href=voip_config.asp target=view>Config</a></td></tr>" \
				"')");
	}
	else if (strcmp(name, T("voip_tree_menu")) == 0)
	{
		websWrite(wp, 
				"menu.addItem('VoIP Settings');" \
				"voip = new MTMenu();");

		for (i=0; i<VOIP_PORTS; i++)
		{
			if (VOIP_PORTS == 1)
				websWrite(wp, 
					"voip.addItem('General', 'voip_general.asp?port=0', '', 'Setup General settings');");
			else if (i < SLIC_CH_NUM)
				websWrite(wp, 
					"voip.addItem('FXS %d', 'voip_general.asp?port=%d', '', 'Setup FXS%d settings');",
					i, i, i);
			else 
				websWrite(wp, 
					"voip.addItem('FXO %d', 'voip_general.asp?port=%d', '', 'Setup FXO%d settings');",
					i - SLIC_CH_NUM, i, i);
		}

		return websWrite(wp, 
			"voip.addItem('Tone', 'voip_tone.asp', '', 'Setup Tone settings');" \
			"voip.addItem('Ring', 'voip_ring.asp', '', 'Setup Ring settings');" \
			"voip.addItem('Other', 'voip_other.asp', '', 'Setup Other settings');" \
			"voip.addItem('Config', 'voip_config.asp', '', 'Config settings');" \
			"menu.makeLastSubmenu(voip);");
	}

	return -1;
}

#endif
