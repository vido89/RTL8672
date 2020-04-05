/*
 *      Web server handler routines for Commit/reboot stuffs
 *
 */


/*-- System inlcude files --*/
/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#include <semaphore.h>
//alex_huang 
#ifdef EMBED
#include <linux/config.h>
#include <config/autoconf.h>
#else
#include "../../../../uClibc/include/linux/autoconf.h"
#include "../../../../config/autoconf.h"
#endif

extern sem_t semSave;

//static void (*restart)(void) = (void *)0xbfc00000;

///////////////////////////////////////////////////////////////////
void formReboot(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str, *submitUrl;
	char tmpBuf[100];
	BOOT_TYPE_T cur_btmode, btmode;
	unsigned char vChar;
	int k;
#ifndef NO_ACTION
	int pid;
#endif

#if 0
	// Commit
	str = websGetVar(wp, T("save"), T(""));
	if (str[0]) {
		int time;
		
		if (mib_get(MIB_BOOT_MODE, (void *)&vChar))
			cur_btmode = (BOOT_TYPE_T)vChar;
		else
			cur_btmode = BOOT_LAST;
		
		str = websGetVar(wp, T("rebootMode"), T(""));
		
		if (str[0]) {
			if ( str[0] == '0' )
				btmode = BOOT_LAST;
			else if ( str[0] == '1' )
				btmode = BOOT_DEFAULT;
			else if ( str[0] == '2' )
				btmode = BOOT_UPGRADE;
			else {
				strcpy(tmpBuf, T("Invalid boot mode value!"));
				goto setErr_reboot;
			}
			
			vChar = (unsigned char) btmode;
			if ( mib_set(MIB_BOOT_MODE, (void *)&vChar) == 0) {
				strcpy(tmpBuf, T("Set boot mode MIB error!"));
				goto setErr_reboot;
			}
		}
		
		/* upgdate to flash */
		// only "boot from last" can be updated
		if (cur_btmode == BOOT_LAST) {
			if (mib_update(CURRENT_SETTING) == 0) {
				strcpy(tmpBuf, T("Warning : Commit cs fail !"));
				goto setErr_reboot;
			}
		}
		
		if (cur_btmode != btmode) {
			if (mib_update(HW_SETTING) == 0) {
				strcpy(tmpBuf, T("Warning : Commit hs fail !"));
				goto setErr_reboot;
			}
		}
	}

	// Reboot
	str = websGetVar(wp, T("reboot"), T(""));
	if (str[0]) {
		OK_MSG1("The system is restarting ...\n", NULL);
//		restart();
		va_cmd("/bin/reboot", 0, 0);
	}
#else
	// Reset to Default
	str = websGetVar(wp, T("reset"), T(""));
	
	// Commented by Mason Yu. for not use default setting
	/*
	if (str[0])
	{
            #if (defined(CONFIG_RTL867X_NETLOG)  && defined (CONFIG_USER_NETLOGGER_SUPPORT))
               va_cmd("/bin/netlogger",1,1,"disable");
	        sleep(1);
           #endif
  
		mib_load(DEFAULT_SETTING, CONFIG_MIB_ALL);
	}
	*/
	
	/* Save and reboot the system */
	websHeader(wp);
	#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	websWrite(wp, T("<META HTTP-EQUIV=Refresh CONTENT=\"60; URL=/status_sc.asp\">\n"));
	#else
	// Kaohj, not necessary if we have timer countdown
	//websWrite(wp, T("<META HTTP-EQUIV=Refresh CONTENT=\"60; URL=/status.asp\">\n"));
	#endif
	//--- Add timer countdown
        websWrite(wp, T("<head><style>\n" \
        "#cntdwn{ border-color: white;border-width: 0px;font-size: 12pt;color: red;text-align:left; font-weight:bold; font-family: Courier;}\n" \
        "</style><script language=javascript>\n" \
        "var h=60;\n" \
        "function stop() { clearTimeout(id); }\n"\
        "function start() { h--; if (h >= 0) { frm.time.value = h; frm.textname.value='System rebooting, Please wait ...'; id=setTimeout(\"start()\",1000); }\n" \
        "if (h == 0) { window.open(\"/status.asp\",target=\"view\"); }}\n" \
        "</script></head>"));
        websWrite(wp, T(
        "<body bgcolor=white  onLoad=\"start();\" onUnload=\"stop();\"><blockquote>" \
        "<form name=frm><B><font color=red><INPUT TYPE=text NAME=textname size=40 id=\"cntdwn\">\n" \
        "<INPUT TYPE=text NAME=time size=5 id=\"cntdwn\"></font></form></blockquote></body>") );
        //--- End of timer countdown
   	websWrite(wp, T("<body><blockquote><h4>\n"));
   	/*
   	websWrite(wp, T("The System is Restarting ...</h4>\n"));
   	websWrite(wp, T("The DSL Router has been configured and is rebooting.<br><br>\n"));
   	websWrite(wp, T("Close the DSL Router Configuration window and wait"
   		" for 2 minutes before reopening your web browser."
   		" If necessary, reconfigure your PC's IP address to match"
   		" your new configuration.\n"));
   	*/
   	websWrite(wp, T("%s</h4>\n"), rebootWord0);
   	websWrite(wp, T("%s<br><br>\n"), rebootWord1);
   	websWrite(wp, T("%s\n"), rebootWord2);
   	websWrite(wp, T("</blockquote></body>"));
   	websFooter(wp);
	websDone(wp, 200);
	
#ifdef EMBED
#ifndef RESERVE_KEY_SETTING
#if defined (ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	if (str[0]) {
		semSave.__sem_value=SEM_DEFAULT;
	}	
#else
	// Mason Yu
	if (str[0]) {
		// commented by Mason Yu. for not use default setting
		//va_cmd("/bin/flash", 2, 1, "default", "ds");		
		va_cmd("/bin/flash", 2, 1, "default", "cs");
	}
	
	// wake up the Save/reboot thread
	// Mason Yu
	if (str[0]) {
		semSave.__sem_value=SEM_REBOOT;
	}	
#endif
#else//ql-- reserve key setting
	//first backup current setting
	mib_backup(CONFIG_MIB_ALL);
						
	mib_clear(CONFIG_MIB_ALL);
#if 1
	//now retrieve the key parameters.
#ifdef _CWMP_MIB_
	mib_retrive_table(CWMP_ACS_URL);
	mib_retrive_table(CWMP_ACS_USERNAME);
	mib_retrive_table(CWMP_ACS_PASSWORD);
#endif
	mib_retrive_table(MIB_BRCTL_AGEINGTIME);
	mib_retrive_table(MIB_BRCTL_STP);
	mib_retrive_chain(MIB_ATM_VC_TBL);
#ifdef CONFIG_EXT_SWITCH
	mib_retrive_chain(MIB_SW_PORT_TBL);
#endif
	mib_retrive_chain(MIB_IP_ROUTE_TBL);
#endif
	// wake up the reboot thread
	//added by ql: just to upgrade the current setting
	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
	semSave.__sem_value=SEM_REBOOT;
#endif
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	if(!str[0])    //reset to default is not set....
	{
		str = websGetVar(wp, T("commit"), T(""));
		if(str[0])
		{
			semSave.__sem_value=0;   // tell saved to commit and reboot .   jim 
		}else						// only to reboot board.
			semSave.__sem_value=SEM_REBOOT;
	}
#endif
	sem_post(&semSave);
#endif
	
#if 0
	// close interfaces
	//sleep(1);
	sync();
	va_cmd(IFCONFIG, 2, 1, (char *)ELANIF, "down");
	va_cmd(IFCONFIG, 2, 1, (char *)WLANIF, "down");
	/* upgdate to flash */
	if (mib_update(CURRENT_SETTING) == 0) {
		strcpy(tmpBuf, T("Warning : Commit setting fail !"));
		goto setErr_reboot;
	}
	va_cmd("/bin/reboot", 0, 0);
#endif
	
#endif
#ifndef NO_ACTION
	pid = fork();
        if (pid)
                waitpid(pid, NULL, 0);
        else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _CONFIG_SCRIPT_PROG);
#ifdef HOME_GATEWAY
		execl( tmpBuf, _CONFIG_SCRIPT_PROG, "gw", "bridge", NULL);
#else
		execl( tmpBuf, _CONFIG_SCRIPT_PROG, "ap", "bridge", NULL);
#endif
                exit(1);
        }
#endif

#if 0
	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	OK_MSG(submitUrl);
#endif
  	return;

setErr_reboot:
	ERR_MSG(tmpBuf);
}
