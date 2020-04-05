/*
 *      Web server handler routines for Packet Capture stuffs
 *
 */

/*-- System inlcude files --*/
#include <sys/socket.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "../defs.h"

#ifdef CONFIG_USER_TCPDUMP_WEB
// static webs_t gwp;

void kill_tcpdump(void);

void kill_tcpdump()
{
	FILE *fp;
	char temp_pid[10];
	int tcpdump_pid;
	fp = fopen("/var/run/tcpdump.pid","rt");
	if (fp)
   	{
		fgets(temp_pid,sizeof(temp_pid)-1,fp);
		fclose(fp);
		tcpdump_pid = atoi(temp_pid);
		if (tcpdump_pid > 0)
			kill(tcpdump_pid,SIGKILL);
	}
}

void formCapture(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str;
    int status=0;
    char tmpbuf[MAX_URL_LEN];
    char ipaddr[16], ip_port[32];
	
	websHeader(wp);
	websWrite(wp, T("<body><blockquote>"));

	str = websGetVar(wp, T("dostart"), T(""));

	if (strcmp(str,"yes") == 0) // start capture
	{
		str = websGetVar(wp, T("tcpdumpArgs"), T(""));

		websWrite(wp, T("<br>Your capture arguments: "));
		websWrite(wp, T(str));

		websWrite(wp, T("<p>\n"));

		sprintf(tmpbuf,"/bin/tcpdump -g %s &",str);
		kill_tcpdump();
		system(tmpbuf);
		sleep(1);
		websWrite(wp, T("Packet dump has started!<br>\n"));

		ipaddr[0]='\0'; ip_port[0]='\0';
		if (mib_get(MIB_ADSL_LAN_IP, (void *)tmpbuf) != 0)
		{
			strncpy(ipaddr, inet_ntoa(*((struct in_addr *)tmpbuf)), 16);
			ipaddr[15] = '\0';
			sprintf(tmpbuf,"http://%s:%d/tcpdump.cap",ipaddr,19222);
		}

		websWrite(wp, T("Please click <a href='"));
		websWrite(wp, T(tmpbuf));
		websWrite(wp, T("'>here</a> once now to start the download of the capture file.<p>\n"));
		websWrite(wp, T("To stop it please return to the Diagnostics / Packet Dump page and click 'Stop'.\n<br>"));
		websWrite(wp, T("Do not stop the download by the browser's download manager.\n"));
	}
	else
	{
		kill_tcpdump();
        websWrite(wp, T("<br>Capture file closed."));
	}

	websWrite(wp, T("</blockquote>"));
	websFooter(wp);
	websDone(wp, 200);
	
  	return;

}
#endif // of CONFIG_USER_TCPDUMP_WEB

