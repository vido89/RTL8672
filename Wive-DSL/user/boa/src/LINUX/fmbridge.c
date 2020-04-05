/*
 *      Web server handler routines for Bridge stuffs
 *
 */


/*-- System inlcude files --*/
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/if_bridge.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#include "debug.h"
#include "../defs.h"

#define BRCTL_GET_FDB_ENTRIES 18
struct bridge;
//struct bridge_info;
struct port;
int br_socket_fd;

struct bridge
{
	struct bridge *next;

	int ifindex;
	char ifname[IFNAMSIZ];
	struct port *firstport;
	struct port *ports[256];
//	struct bridge_info info;
};

#ifdef __i386__
#define _LITTLE_ENDIAN_
#endif

/*-- Macro declarations --*/
#ifdef _LITTLE_ENDIAN_
#define ntohdw(v) ( ((v&0xff)<<24) | (((v>>8)&0xff)<<16) | (((v>>16)&0xff)<<8) | ((v>>24)&0xff) )

#else
#define ntohdw(v) (v)
#endif

///////////////////////////////////////////////////////////////////
void formBridge(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str, *submitUrl;
	char tmpBuf[100];
#ifndef NO_ACTION
	int pid;
#endif
	unsigned char stp;
#if	defined(APPLY_CHANGE) || defined(ZTE_GENERAL_ROUTER_SC)			
	char *argv[5];
#endif

	// Set Ageing Time
	str = websGetVar(wp, T("ageingTime"), T(""));
	if (str[0]) {
		unsigned short time;
		time = (unsigned short) strtol(str, (char**)NULL, 10);
		if ( mib_set(MIB_BRCTL_AGEINGTIME, (void *)&time) == 0) {
			strcpy(tmpBuf, T("Set bridge ageing time MIB error!"));
			goto setErr_bridge;
		}
#if	defined(APPLY_CHANGE) || defined(ZTE_GENERAL_ROUTER_SC)		
		argv[1]="setageing";
		argv[2]=(char*)BRIF;
		argv[3]=str;
		argv[4]=NULL;
		TRACE(STA_SCRIPT, "%s %s %s %s\n", BRCTL, argv[1],
		argv[2], argv[3]);
		do_cmd(BRCTL, argv, 1);
#endif
	}

	// Set STP
	str = websGetVar(wp, T("stp"), T(""));
	if (str[0]) {
		if (str[0] == '0')
			stp = 0;
		else
			stp = 1;
		if ( !mib_set(MIB_BRCTL_STP, (void *)&stp)) {
			strcpy(tmpBuf, T("Set STP mib error!"));
			goto setErr_bridge;
		}
		
#if	defined(APPLY_CHANGE) || defined(ZTE_GENERAL_ROUTER_SC)		
		if (stp == 1)	// on
		{	// brctl setfd br0 20
			argv[1]="setfd";
			argv[2]=(char*)BRIF;
			argv[3]="20";
			argv[4]=NULL;
			TRACE(STA_SCRIPT, "%s %s %s %s\n", BRCTL, argv[1], argv[2], argv[3]);
			do_cmd(BRCTL, argv, 1);
		}
		
		argv[1]="stp";
		argv[2]=(char*)BRIF;
		
		if (stp == 0)
			argv[3]="off";
		else
			argv[3]="on";
		
		argv[4]=NULL;
		TRACE(STA_SCRIPT, "%s %s %s %s\n", BRCTL, argv[1], argv[2], argv[3]);
		do_cmd(BRCTL, argv, 1);
#endif
	}
	/* upgdate to flash */
//	mib_update(CURRENT_SETTING);

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

	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	OK_MSG(submitUrl);
  	return;

setErr_bridge:
	ERR_MSG(tmpBuf);
}

static void __dump_fdb_entry(webs_t wp, struct __fdb_entry *f)
{
	unsigned long long tvusec;
	int sec,usec;
	
	// jiffies to tv
	tvusec = (1000000ULL*f->ageing_timer_value)/HZ;
	sec = tvusec/1000000;
	usec = tvusec - 1000000 * sec;
	
	if (f->is_local)
		websWrite(wp, T("<tr bgcolor=#b7b7b7><td><font size=2>%3i\t</td>"
			"<td><font size=2>%.2x-%.2x-%.2x-%.2x-%.2x-%.2x\t</td>"
			"<td><font size=2>%s\t\t</td>"
			"<td><font size=2>%s\t</td></tr>"),
			f->port_no, f->mac_addr[0], f->mac_addr[1], f->mac_addr[2],
			f->mac_addr[3], f->mac_addr[4], f->mac_addr[5],
			Tyes, "---");
	else
		websWrite(wp, T("<tr bgcolor=#b7b7b7><td><font size=2>%3i\t</td>"
			"<td><font size=2>%.2x-%.2x-%.2x-%.2x-%.2x-%.2x\t</td>"
			"<td><font size=2>%s\t\t</td>"
			"<td><font size=2>%4i.%.2i\t</td></tr>"),
			f->port_no, f->mac_addr[0], f->mac_addr[1], f->mac_addr[2],
			f->mac_addr[3], f->mac_addr[4], f->mac_addr[5],
			f->is_local?Tyes:Tno, sec, usec/10000);
}

int bridgeFdbList(int eid, webs_t wp, int argc, char_t **argv)
{
	struct bridge *br;
	struct __fdb_entry fdb[256];
	int offset;
	unsigned long args[4];
	struct ifreq ifr;

/*
	br = bridge_list;
	while (br != NULL) {
		if (!strcmp(br->ifname, "br0"))
			break;

		br = br->next;
	}
	
	if (br == NULL)
	{
		websWrite(wp, T("%s"), "br0 interface not exists !!");
		return 0;
	}
*/
	
	offset = 0;
	args[0] = BRCTL_GET_FDB_ENTRIES;
	args[1] = (unsigned long)fdb;
	args[2] = 256;
	if ((br_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		websWrite(wp, T("%s"), "socket not avaiable !!");
		return 0;
	}
//	memcpy(ifr.ifr_name, br->ifname, IFNAMSIZ);
	memcpy(ifr.ifr_name, "br0", IFNAMSIZ);
	((unsigned long *)(&ifr.ifr_data))[0] = (unsigned long)args;
	while (1) {
		int i;
		int num;

		args[3] = offset;
		num = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);

		if (num <= 0)
		{
			if (num < 0)
				websWrite(wp, T("%s"), Tbrg_not_exist);
			break;
		}

		for (i=0;i<num;i++)
			__dump_fdb_entry(wp, fdb+i);

		offset += num;
	}
	close(br_socket_fd);
}

int ARPTableList(int eid, webs_t wp, int argc, char_t **argv)
{
	FILE *fp;
	char  buf[256];
	char arg1[20],arg2[20],arg4[20];
	int arg3;
	int nBytesSent=0;
	int enabled;
	
	fp = fopen("/proc/net/arp", "r");
	if (fp == NULL){
		printf("read arp file fail!\n");
		goto err1;
	}
    fgets(buf,256,fp);
	while(fgets(buf,256,fp)){
		sscanf(buf,"%s	%s	0x%x	%s",arg1,arg2,&arg3,arg4);
		if (!arg3) {
			continue;
		}
		#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
		int i=0;
		for(i=0;i<strlen(arg4);i++)
			{
			  if(arg4[i]==':')
			  	arg4[i]='-';
			}
		#endif
		websWrite(wp, T("<tr bgcolor=#b7b7b7><td><font size=2>%s\t</td>"
		"<td><font size=2>%s\t</td>"),
		arg1,arg4);		
	}

	fclose(fp);
err1:
	return nBytesSent;
}


/////////////////////////////////////////////////////////////////////////////
void formRefleshFdbTbl(webs_t wp, char_t *path, char_t *query)
{
	char_t *submitUrl;
	char_t *strSubmit;
	FILE *fp;
	char  buf[256];
	char arg1[20],arg2[20],arg3[20],arg4[20];

	submitUrl = websGetVar(wp, T("submit-url"), T(""));

	strSubmit = websGetVar(wp, T("refresh"), T(""));
	if (strSubmit[0]) {
		websRedirect(wp, submitUrl);
		return;
	}

	strSubmit = websGetVar(wp, T("clear"), T(""));
	if (strSubmit[0]) {//clear arp table
		fp = fopen("/proc/net/arp", "r");
		if (fp == NULL){
			printf("read arp file fail!\n");
			goto ERR;
		}
		fgets(buf,256, fp);
		
		while(fgets(buf,256,fp)){
			memset(arg1, 20, 0);
			sscanf(buf,"%s	%s	%s	%s", arg1,arg2,arg3,arg4);
			va_cmd("/bin/arp", 2, 1, "-d", arg1);
		}
		fclose(fp);
	}
ERR:
	websRedirect(wp, submitUrl);
}

