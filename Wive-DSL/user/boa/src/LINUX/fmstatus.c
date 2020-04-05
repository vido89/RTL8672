/*
 *      Web server handler routines for System status
 *
 */

#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#include "debug.h"
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_bridge.h>
#include "adsl_drv.h"
#include <stdio.h>
#include <fcntl.h>
#include "signal.h"
#include "../defs.h"
#include "boa.h"

static const char IF_UP[] = "up";
static const char IF_DOWN[] = "down";
static const char IF_NA[] = "n/a";
#ifdef EMBED
const char PROC_NET_ATM_BR[] = "/proc/net/atm/br2684";
#ifdef CONFIG_ATM_CLIP
const char PROC_NET_ATM_CLIP[] = "/proc/net/atm/pvc";
#endif
const char PPPOE_CONF[] = "/var/ppp/pppoe.conf";
const char PPPOA_CONF[] = "/var/ppp/pppoa.conf";
const char PPP_PID[] = "/var/run/spppd.pid";
#ifdef CTC_WAN_NAME
const char PPP_CONF[] = "/var/ppp/ppp.conf";
#endif
#endif

void formStatus(webs_t wp, char_t *path, char_t *query)
{
	char_t *submitUrl, *strSubmitR, *strSubmitP;
	struct data_to_pass_st msg;
	char tmpBuf[100], buff[256];
	unsigned int i, cflag[MAX_PPP_NUM]={0}, flag, inf;
	FILE *fp;
	
	// Added by Jenny, for PPP connecting/disconnecting
	for (i=0; i<MAX_PPP_NUM; i++) {
		char_t tmp[15], tp[10];
		
		sprintf(tp, "ppp%d", i);
		if (find_ppp_from_conf(tp)) {
			if (fp=fopen("/tmp/ppp_up_log", "r")) {
				while ( fgets(buff, sizeof(buff), fp) != NULL ) {
					if(sscanf(buff, "%d %d", &inf, &flag) != 2)
						break;
					else {
						if (inf == i)
							cflag[i] = flag;
					}
				}
				fclose(fp);
			}
			sprintf(tmp, "submitppp%d", i);
			strSubmitP = websGetVar(wp, tmp, T(""));
			if ( strSubmitP[0] ) {
#ifdef ZTE_GENERAL_ROUTER_SC
				if ((strcmp(strSubmitP, "连接") == 0))
#else
				if ((strcmp(strSubmitP, "Connect") == 0))
#endif
				{
					if (cflag[i]) {
						snprintf(msg.data, BUF_SIZE, "spppctl up %u", i);
						usleep(3000000);
						TRACE(STA_SCRIPT, "%s\n", msg.data);
						write_to_pppd(&msg);
							//add by ramen to resolve for clicking "connect" button twice.
				#ifdef ZTE_GENERAL_ROUTER_SC
						 int waittime=time_counter;

                      				while (time_counter<waittime+10)
				                        {
				                          pause();
				                        }
					
				#endif
					}
				}
#ifdef ZTE_GENERAL_ROUTER_SC
     				else if (strcmp(strSubmitP, "断开") == 0)
#else
				else if (strcmp(strSubmitP, "Disconnect") == 0)
#endif
				{
                                   
					snprintf(msg.data, BUF_SIZE, "spppctl down %u", i);
					TRACE(STA_SCRIPT, "%s\n", msg.data);
					write_to_pppd(&msg);
						//add by ramen to resolve for clicking "disconnect" button twice.
				#ifdef ZTE_GENERAL_ROUTER_SC
						 int waittime=time_counter;

                      				while (time_counter<waittime+2)
				                        {
				                          pause();
				                        }
					
				#endif
				}
				else {
#ifdef ZTE_GENERAL_ROUTER_SC
					strcpy(tmpBuf, T("无效的PPP行为!"));
#else
					strcpy(tmpBuf, T("Invalid PPP action!"));
#endif
					goto setErr_filter;
				}
			}
		}
	}
	
	strSubmitR = websGetVar(wp, T("refresh"), T(""));
	// Refresh
	if (strSubmitR[0]) {
		goto setOk_filter;
	}
	
setOk_filter:
	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
  	return;

setErr_filter:
	ERR_MSG(tmpBuf);
}

void formDate(webs_t wp, char_t *path, char_t *query)
{
	char_t *strVal, *submitUrl;
	time_t tm;
	struct tm tm_time;
	
	time(&tm);
	memcpy(&tm_time, localtime(&tm), sizeof(tm_time));
	
	strVal = websGetVar(wp, T("sys_month"), T(""));
	if (strVal[0])
		tm_time.tm_mon = atoi(strVal);
	
	strVal = websGetVar(wp, T("sys_day"), T(""));
	if (strVal[0])
		tm_time.tm_mday = atoi(strVal);
	
	strVal = websGetVar(wp, T("sys_year"), T(""));
	if (strVal[0])
		tm_time.tm_year = atoi(strVal) - 1900;
	
	strVal = websGetVar(wp, T("sys_hour"), T(""));
	if (strVal[0])
		tm_time.tm_hour = atoi(strVal);
	
	strVal = websGetVar(wp, T("sys_minute"), T(""));
	if (strVal[0])
		tm_time.tm_min = atoi(strVal);
	
	strVal = websGetVar(wp, T("sys_second"), T(""));
	if (strVal[0])
		tm_time.tm_sec = atoi(strVal);
	
	tm = mktime(&tm_time);
	
	if (stime(&tm) < 0) {
		perror("cannot set date");
	}
	
	OK_MSG1("System Date has been modified successfully.<p>"
		"Please reflesh your \"Status\" page.", NULL);
	return;
}

#ifdef ZTE_GENERAL_ROUTER_SC
void formTimezone(webs_t wp, char_t *path, char_t *query)
{
	char_t *strVal, *submitUrl, *tmpStr;
	char enabled=0, ntpServerIdx;
	char tmpBuf[100];
	struct in_addr ipAddr ;
	time_t tm;
	struct tm tm_time;

	tmpStr = websGetVar(wp, T("sys_tz"), T(""));
	if (tmpStr[0]){
		FILE *fp;
		
		if ( mib_set(MIB_NTP_TIMEZONE, (void *)tmpStr) == 0) {
			strcpy(tmpBuf, T(Tset_tz));
			goto setErr_tz;
		}
		strVal = strstr(tmpStr, " ");
		if (strVal != NULL)
			strVal[0] = '\0';
		snprintf(tmpBuf, 16, "GMT%s", tmpStr);
		//setenv("TZ", tmpBuf, 1);
		if ((fp = fopen("/etc/TZ", "w")) != NULL) {
			fprintf(fp, "%s\n", tmpBuf);
			fclose(fp);
		}
	}
	
	strVal = websGetVar(wp, T("ntpEnabled"), T(""));
	if (strVal[0] == '0') {
		time(&tm);
		memcpy(&tm_time, localtime(&tm), sizeof(tm_time));
		
		strVal = websGetVar(wp, T("sys_month"), T(""));
		if (strVal[0])
			tm_time.tm_mon = atoi(strVal);
		
		strVal = websGetVar(wp, T("sys_day"), T(""));
		if (strVal[0])
			tm_time.tm_mday = atoi(strVal);
		
		strVal = websGetVar(wp, T("sys_year"), T(""));
		if (strVal[0]){
			//printf("tm_year: %d\n", atoi(strVal));
			tm_time.tm_year = atoi(strVal) - 1900;
		}
		
		strVal = websGetVar(wp, T("sys_hour"), T(""));
		if (strVal[0])
			tm_time.tm_hour = atoi(strVal);
		
		strVal = websGetVar(wp, T("sys_minute"), T(""));
		if (strVal[0])
			tm_time.tm_min = atoi(strVal);
		
		strVal = websGetVar(wp, T("sys_second"), T(""));
		if (strVal[0])
			tm_time.tm_sec = atoi(strVal);
		
		tm = mktime(&tm_time);
		
		if (stime(&tm) < 0) {
			perror(Tset_date_err);
		}

		goto setSz_OK;
	}
		
	tmpStr = websGetVar(wp, T("enabled"), T(""));
	if(!strcmp(tmpStr, "ON"))
		enabled = 1 ;
	else 
		enabled = 0 ;
	if ( mib_set( MIB_NTP_ENABLED, (void *)&enabled) == 0) {
		strcpy(tmpBuf, T(Tset_ntp_ena));
		goto setErr_tz;
	}

	tmpStr = websGetVar(wp, T("ntpServerId"), T(""));  
	if(tmpStr[0]){
		ntpServerIdx = tmpStr[0] - '0' ;
		if ( mib_set(MIB_NTP_SERVER_ID, (void *)&ntpServerIdx) == 0) {
			strcpy(tmpBuf, T(Tset_tz));
			goto setErr_tz;
		}
	}
		
	tmpStr = websGetVar(wp, T("ntpServerIp1"), T(""));  
	if(tmpStr[0]){
		inet_aton(tmpStr, &ipAddr);
		if ( mib_set(MIB_NTP_SERVER_IP1, (void *)&ipAddr) == 0) {
			strcpy(tmpBuf, T(Tset_ntp_svr));
			goto setErr_tz;
		} 
	}
		
	tmpStr = websGetVar(wp, T("ntpServerIp2"), T(""));  
	if(tmpStr[0]){
		inet_aton(tmpStr, &ipAddr);
		if ( mib_set(MIB_NTP_SERVER_IP2,(void *) &ipAddr ) == 0) {
			strcpy(tmpBuf, T(Tset_ntp_ip));
			goto setErr_tz;
		}
	}
	if (enabled)
		startNTP();
	else
		stopNTP();
setSz_OK:
	submitUrl=websGetVar(wp, T("submit-url"), T(""));
	if (submitUrl[0]){
		OK_MSG1(Ttime_zone1"<p>"Ttime_zone2, submitUrl);
	} else
		websDone(wp, 200);
	return;
	
setErr_tz:
	ERR_MSG(tmpBuf);
	return;
/*refreshPage:
	submitUrl=websGetVar(wp, T("submit-url"), T(""));
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);*/
}
#endif

//jim china telecom
#ifdef CTC_WAN_NAME
//jim we define it in utility.c
/*
static int setWanName(char* str, int applicationtype)
{
	switch(applicationtype)
	{
		//Internet
		case 0:
			strcpy(str, "Internet_");
			break;
		//TR069_Internet
		case 1:
			strcpy(str, "TR069_Internet_");
			break;
			//TR069
		case 2:
			strcpy(str, "TR069_");
			break;
		//Others
		case 3:
			strcpy(str, "Other_");
			break;
		default:
			strcpy(str, "Internet_");
			break;
	}	
}*/
int getATMEntrybyVPIVCIUsrPswd(MIB_CE_ATM_VC_T* Entry, int vpi, int vci, char* username, char* password, char *ifname)
{
	int entryNum;
	int mibcnt;
	int ret=0;
	char tmpifname[6];
	if(Entry==NULL )
	{
		return -1;
	}
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for(mibcnt=0; mibcnt<entryNum; mibcnt++)
	{
		if (!mib_chain_get(MIB_ATM_VC_TBL, mibcnt, (void *)Entry))
		{
			return -1;
		}

		if(Entry->cmode == ADSL_PPPoE || Entry->cmode == ADSL_PPPoA)//star:for multi-ppp name
			snprintf(tmpifname, 6, "ppp%u", PPP_INDEX(Entry->ifIndex));
		else
			snprintf(tmpifname, 6, "vc%u", VC_INDEX(Entry->ifIndex));
		
		if(username==NULL || password==NULL)
		{
			//ignore PPP
			if((Entry->cmode!=ADSL_PPPoE &&Entry->cmode!=ADSL_PPPoA) && 
					Entry->vpi==vpi && Entry->vci==vci && !strcmp(tmpifname, ifname)) 
				return 0; //found...
		}
		else if((Entry->cmode==ADSL_PPPoE ||Entry->cmode==ADSL_PPPoA)&& Entry->vpi==vpi && Entry->vci==vci && 
			!strcmp(Entry->pppUsername, username)    && !strcmp(Entry->pppPassword, password) && !strcmp(tmpifname, ifname))
		{
			return 0; //found
		}
	}
	return -1; //not found
}
#endif

// Jenny, current status
int wanConfList(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;

	unsigned int data, data2;
	char	buff[256], tmp1[20], tmp2[20], tmp3[20], tmp4[20];
	char	*temp;
	int in_turn=0, vccount=0, ifcount=0, isPPP=0, pppConnectStatus=0, adslflag=0, pppDod=0;
	int i, ifindices[256], br_socket_fd, spid;
	unsigned long arg[4];
	struct ifreq ifr;
	FILE *fp;
	struct flock flpoe, flpoa;
	int fdpoe, fdpoa;
	Modem_LinkSpeed vLs;

	struct status_info {
		unsigned char ifIndex;
		char ifname[IFNAMSIZ];
		char devname[IFNAMSIZ];
		unsigned int tvpi;
		unsigned int tvci;
		char encaps[8];
		char protocol[10];
		char ipAddr[20];
		char remoteIp[20];
		char *strStatus;
		char uptime[20];
		char totaluptime[20];
	};
	struct status_info sEntry[MAX_VC_NUM+MAX_PPP_NUM];
	struct status_info vcEntry[MAX_VC_NUM];

#ifdef EMBED
	// get spppd pid
	if ((fp = fopen(PPP_PID, "r"))) {
		fscanf(fp, "%d\n", &spid);
		fclose(fp);
	}
	else
		printf("spppd pidfile not exists\n");
	
	kill(spid, SIGUSR2);
#endif
#ifdef ZTE_531B_BRIDGE_SC
nBytesSent += websWrite(wp, T("<tr bgcolor=\"#808080\">"
	"<td width=\"30%%\" align=center><font size=2><b>端口名称</b></font></td>\n"
	"<td width=\"10%%\" align=center><font size=2><b>VPI/VCI</b></font></td>\n"
	"<td width=\"20%%\" align=center><font size=2><b>封装类型</b></font></td>\n"
	"<td width=\"20%%\" align=center><font size=2><b>协议类型</b></font></td>\n"
	//"<td width=\"20%%\" align=center><font size=2><b>IP 地址</b></td>\n"
	//"<td width=\"20%%\" align=center><font size=2><b>路由地址</b></td>\n"
	"<td width=\"20%%\" align=center><font size=2><b>端口状态</b></font></td></font></tr>\n"));
#else 
#ifdef ZTE_GENERAL_ROUTER_SC
nBytesSent += websWrite(wp, T("<tr bgcolor=\"#808080\">"
	"<td width=\"20%%\" align=center><font size=2><b>端口名称</b></font></td>\n"
	"<td width=\"10%%\" align=center><font size=2><b>VPI/VCI</b></font></td>\n"
	"<td width=\"6%%\" align=center><font size=2><b>NAPT</b></font></td>\n"
	"<td width=\"10%%\" align=center><font size=2><b>封装类型</b></font></td>\n"
	"<td width=\"10%%\" align=center><font size=2><b>协议类型</b></font></td>\n"
	"<td width=\"17%%\" align=center><font size=2><b>IP 地址</b></font></td>\n"
	"<td width=\"17%%\" align=center><font size=2><b>路由地址</b></font></td>\n"
	"<td width=\"10%%\" align=center><font size=2><b>端口状态</b></font></td></tr>\n"));
#else
	nBytesSent += websWrite(wp, T("<tr bgcolor=\"#808080\">"
	"<td width=\"8%%\" align=center><font size=2><b>Interface</b></font></td>\n"
	"<td width=\"12%%\" align=center><font size=2><b>VPI/VCI</b></font></td>\n"
	"<td width=\"12%%\" align=center><font size=2><b>Encap</b></font></td>\n"
	"<td width=\"12%%\" align=center><font size=2><b>Protocol</b></font></td>\n"
	"<td width=\"22%%\" align=center><font size=2><b>IP Address</b></font></td>\n"
	"<td width=\"22%%\" align=center><font size=2><b>Gateway</b></font></td>\n"
	"<td width=\"12%%\" align=center><font size=2><b>Status</b></font></td></tr>\n"));
#endif
#endif

	in_turn = 0;
#ifdef EMBED
	if (!(fp=fopen(PROC_NET_ATM_BR, "r")))
		printf("%s not exists.\n", PROC_NET_ATM_BR);
	else {
		while ( fgets(buff, sizeof(buff), fp) != NULL ) {
			if (in_turn==0)
				if(sscanf(buff, "%*s%s", tmp1)!=1) {
					printf("Unsuported pvc configuration format\n");
					break;
				}
				else {
					vccount ++;
					tmp1[strlen(tmp1)-1]='\0';
					strcpy(vcEntry[vccount-1].ifname, tmp1);
					strcpy(vcEntry[vccount-1].devname, tmp1);
				}
			else
				if(sscanf(buff, "%*s%s%*s%s", tmp1, tmp2)!=2) {
					printf("Unsuported pvc configuration format\n");
					break;
				}
				else {
					sscanf(tmp1, "0.%u.%u:", &vcEntry[vccount-1].tvpi, &vcEntry[vccount-1].tvci);
					sscanf(tmp2, "%u,", &data);
					strcpy(vcEntry[vccount-1].protocol, "");
					if (data==1 || data == 4)
						strcpy(vcEntry[vccount-1].encaps, "LLC");
					else if (data==0 || data==3)
						strcpy(vcEntry[vccount-1].encaps, "VCMUX");
					if (data==3 || data==4)
						strcpy(vcEntry[vccount-1].protocol, "rt1483");
				}
			in_turn ^= 0x01;
		}
		fclose(fp);
	}

#ifdef CONFIG_ATM_CLIP
	if (!(fp=fopen(PROC_NET_ATM_CLIP, "r")))
		printf("%s not exists.\n", PROC_NET_ATM_CLIP);
	else {
		fgets(buff, sizeof(buff), fp);
		while ( fgets(buff, sizeof(buff), fp) != NULL ) {
			char *p = strstr(buff, "CLIP");
			if (p != NULL) {
				if (sscanf(buff, "%*d%u%u%*d%*d%*s%*d%*s%*s%s%s", &data, &data2, tmp1, tmp2) != 4) {
					printf("Unsuported 1577 configuration format\n");
					break;
				}
				else {
					vccount ++;
					sscanf(tmp1, "Itf:%s", tmp3);
					strcpy(vcEntry[vccount-1].ifname, strtok(tmp3, ","));
					sscanf(tmp2, "Encap:%s", tmp4);
					strcpy(vcEntry[vccount-1].encaps, strtok(tmp4, "/"));
					strcpy(vcEntry[vccount-1].protocol, "rt1577");
					vcEntry[vccount-1].tvpi = data;
					vcEntry[vccount-1].tvci = data2;
				}
			}
		}
		fclose(fp);
	}
#endif

	// file locking
	fdpoe = open(PPPOE_CONF, O_RDWR);
	fdpoa = open(PPPOA_CONF, O_RDWR);
	if ((fdpoe != -1) && (fdpoe != -1)) {
		flpoe.l_type = flpoa.l_type = F_RDLCK;
		flpoe.l_whence = flpoa.l_whence = SEEK_SET;
		flpoe.l_start = flpoa.l_start = 0;
		flpoe.l_len = flpoa.l_len = 0;
		flpoe.l_pid = flpoa.l_pid = getpid();
		if (fcntl(fdpoe, F_SETLKW, &flpoe) == -1)
			printf("pppoe read lock failed\n");
		if (fcntl(fdpoa, F_SETLKW, &flpoa) == -1)
			printf("pppoa read lock failed\n");
	}
	
	if (!(fp=fopen(PPPOA_CONF, "r")))
		printf("%s not exists.\n", PPPOA_CONF);
	else {
		fgets(buff, sizeof(buff), fp);
		while ( fgets(buff, sizeof(buff), fp) != NULL )
			if (sscanf(buff, "%s%u%u%*s%s%*s%*d%*d%*d%s%s", tmp1, &data, &data2, tmp2, tmp3, tmp4) != 6) {
				printf("Unsuported pppoa configuration format\n");
				break;
			}
			else {
				ifcount ++;
				// ifIndex --- ppp index(no vc index)
				sEntry[ifcount-1].ifIndex = ((tmp1[3]-'0')<<4)|0x0f;
				strcpy(sEntry[ifcount-1].ifname, tmp1);
				strcpy(sEntry[ifcount-1].encaps, tmp2);
				strcpy(sEntry[ifcount-1].protocol, "PPPoA");
				sEntry[ifcount-1].tvpi = data;
				sEntry[ifcount-1].tvci = data2;
				strcpy(sEntry[ifcount-1].uptime, tmp3);
				strcpy(sEntry[ifcount-1].totaluptime, tmp4);
			}
		fclose(fp);
	}

	if (!(fp=fopen(PPPOE_CONF, "r")))
		printf("%s not exists.\n", PPPOE_CONF);
	else {
		fgets(buff, sizeof(buff), fp);
		while ( fgets(buff, sizeof(buff), fp) != NULL )
			if(sscanf(buff, "%s%s%*s%*s%*s%s%s", tmp1, tmp2, tmp3, tmp4) != 4) {
				printf("Unsuported pppoe configuration format\n");
				break;
			}
			else
				for (i=0; i<vccount; i++)
					if (strcmp(vcEntry[i].devname, tmp2) == 0) {
						ifcount ++;
						// ifIndex --- ppp index + vc index
						sEntry[ifcount-1].ifIndex = ((tmp1[3]-'0')<<4)|(vcEntry[i].devname[2]-'0');
						strcpy(sEntry[ifcount-1].ifname, tmp1);
						strcpy(sEntry[ifcount-1].devname, vcEntry[i].devname);
						strcpy(sEntry[ifcount-1].encaps, vcEntry[i].encaps);
						strcpy(sEntry[ifcount-1].protocol, "PPPoE");
						sEntry[ifcount-1].tvpi = vcEntry[i].tvpi;
						sEntry[ifcount-1].tvci = vcEntry[i].tvci;
						strcpy(sEntry[ifcount-1].uptime, tmp3);
						strcpy(sEntry[ifcount-1].totaluptime, tmp4);
					}
		fclose(fp);
	}

	// file unlocking
	if ((fdpoe != -1) && (fdpoe != -1)) {
		flpoe.l_type = flpoa.l_type = F_UNLCK;
		if (fcntl(fdpoe, F_SETLK, &flpoe) == -1)
			printf("pppoe read unlock failed\n");
		if (fcntl(fdpoa, F_SETLK, &flpoa) == -1)
			printf("pppoa read unlock failed\n");
		close(fdpoe);
		close(fdpoa);
	}
	
	for (i=0; i<vccount; i++) {
		int j, vcfound=0;
		for (j=0; j<ifcount; j++)
			if (strcmp(vcEntry[i].ifname, sEntry[j].devname) == 0) {	// PPPoE-used device
				vcfound = 1;
				break;
			}
		if (!vcfound) {	// VC not used for PPPoE, add to list
			ifcount ++;
			// ifIndex --- vc index (no ppp index)
			sEntry[ifcount-1].ifIndex = 0xf0|(vcEntry[i].ifname[2]-'0');
			strcpy(sEntry[ifcount-1].ifname, vcEntry[i].ifname);
			strcpy(sEntry[ifcount-1].encaps, vcEntry[i].encaps);
			strcpy(sEntry[ifcount-1].protocol, vcEntry[i].protocol);
			sEntry[ifcount-1].tvpi = vcEntry[i].tvpi;
			sEntry[ifcount-1].tvci = vcEntry[i].tvci;
		}
	}

#endif

	#if 0
	if ((br_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		printf("socket not avaiable !!");
	arg[0] = BRCTL_GET_PORT_LIST;
	arg[1] = (unsigned long)ifindices;
	arg[2] = 0;
	arg[3] = 0;
	strncpy(ifr.ifr_name, BRIF, IFNAMSIZ);
	((unsigned long *)(&ifr.ifr_data))[0] = (unsigned long)arg;
	ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);
	for (i=255; i>=0; i--) {
		unsigned int ifindex;
		int j;
		if (!ifindices[i])
			continue;
		ifindex = ifindices[i];
		for (j=0; j<ifcount; j++) {
			if (if_nametoindex(sEntry[j].ifname) == ifindex)
				strcpy(sEntry[j].protocol, "br1483");
		}
	}
	close(br_socket_fd);
	#endif

	// check for xDSL link
	if (!adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE) || vLs.upstreamRate == 0)
		adslflag = 0;
	else
		adslflag = 1;
		
	in_turn = 0;
	for (i=0; i<ifcount; i++) {
		struct in_addr inAddr;
		int flags;
		pppConnectStatus = 0;
		isPPP=0;
		char if_display_name[16];
		int totalNum, k;
		MIB_CE_ATM_VC_T entry;

#ifdef EMBED
		// Kaohj --- interface name to be displayed
		if_display_name[0] = 0;
		totalNum = mib_chain_total(MIB_ATM_VC_TBL);
		for(k=0; k<totalNum; k++)
		{
			mib_chain_get(MIB_ATM_VC_TBL, k, (void *)&entry);
			if (sEntry[i].ifIndex == entry.ifIndex) {
				getDisplayWanName(&entry, if_display_name);
				break;
			}
				
		}
		if (getInAddr( sEntry[i].ifname, IP_ADDR, (void *)&inAddr) == 1)
		{
			temp = inet_ntoa(inAddr);
			if (getInFlags( sEntry[i].ifname, &flags) == 1)
				if ((strcmp(temp, "10.0.0.1") == 0) && flags & IFF_POINTOPOINT)	// IP Passthrough or IP unnumbered
					strcpy(sEntry[i].ipAddr, STR_UNNUMBERED);
				else if (strcmp(temp, "64.64.64.64") == 0)
					strcpy(sEntry[i].ipAddr, "");
				else
					strcpy(sEntry[i].ipAddr, temp);
		}
		else
#endif
			strcpy(sEntry[i].ipAddr, "");
	
#ifdef EMBED
		if (getInAddr( sEntry[i].ifname, DST_IP_ADDR, (void *)&inAddr) == 1)
		{
			temp = inet_ntoa(inAddr);
			if (strcmp(temp, "10.0.0.2") == 0)
				strcpy(sEntry[i].remoteIp, STR_UNNUMBERED);
			else if (strcmp(temp, "64.64.64.64") == 0)
				strcpy(sEntry[i].remoteIp, "");
			else
				strcpy(sEntry[i].remoteIp, temp);
			if (getInFlags( sEntry[i].ifname, &flags) == 1)
				if (flags & IFF_BROADCAST) {
					unsigned char value[32];
					snprintf(value, 32, "%s.%s", (char *)MER_GWINFO, sEntry[i].ifname);
					if (fp = fopen(value, "r")) {
						fscanf(fp, "%s\n", sEntry[i].remoteIp);
						//strcpy(sEntry[i].protocol, "mer1483");
						fclose(fp);
					}
					else
						strcpy(sEntry[i].remoteIp, "");
				}
		}
		else
#endif
			strcpy(sEntry[i].remoteIp, "");

///ql_xu: add
	
		if (!strcmp(sEntry[i].protocol, ""))
		{
			int mibCnt;
			
			totalNum = mib_chain_total(MIB_ATM_VC_TBL);
			for(mibCnt=0; mibCnt<totalNum; mibCnt++)
			{
				if (!mib_chain_get(MIB_ATM_VC_TBL, mibCnt, (void *)&entry))
				{
					printf("get MIB chain error\n");
					return -1;
				}
				if(entry.vpi==sEntry[i].tvpi && entry.vci==sEntry[i].tvci)
				{
					//found entry in mibs, get channel mode
					switch(entry.cmode) {
					case ADSL_MER1483:
						memset(sEntry[i].protocol, 0, 10);
						strcpy(sEntry[i].protocol, "mer1483");
						break;
					case ADSL_BR1483:
						memset(sEntry[i].protocol, 0, 10);
						strcpy(sEntry[i].protocol, "br1483");
						break;
					default:
						break;
					}
					break;
				}
			}
		}

			
		// set status flag
		if (getInFlags( sEntry[i].ifname, &flags) == 1)
		{
			if (flags & IFF_UP) {
				if (!adslflag)
					sEntry[i].strStatus = (char *)IF_DOWN;
				else {
					if (strcmp(sEntry[i].protocol, "br1483") == 0)
						sEntry[i].strStatus = (char *)IF_UP;
					else
						if (getInAddr(sEntry[i].ifname, IP_ADDR, (void *)&inAddr) == 1) {
							temp = inet_ntoa(inAddr);
							if (strcmp(temp, "64.64.64.64"))
								sEntry[i].strStatus = (char *)IF_UP;
							else
								sEntry[i].strStatus = (char *)IF_DOWN;
						}
						else
							sEntry[i].strStatus = (char *)IF_DOWN;
				}
			}
			else
				sEntry[i].strStatus = (char *)IF_DOWN;
		}
		else
			sEntry[i].strStatus = (char *)IF_NA;
		
		if (strcmp(sEntry[i].protocol, "PPPoE")==0 || strcmp(sEntry[i].protocol, "PPPoA")==0)
			isPPP = 1;
		else
			isPPP = 0;
		if (isPPP) {
			int totalNum, mibCnt;
			MIB_CE_ATM_VC_T entry;
			if (strcmp(sEntry[i].strStatus, (char *)IF_UP) == 0)
				pppConnectStatus = 1;
			else {
				pppConnectStatus = 0;
				sEntry[i].ipAddr[0] = '\0';
				sEntry[i].remoteIp[0] = '\0';
			}
			totalNum = mib_chain_total(MIB_ATM_VC_TBL);
			for (mibCnt=0; mibCnt<totalNum; mibCnt++) {
				mib_chain_get(MIB_ATM_VC_TBL, mibCnt, (void *)&entry);
				if (entry.vpi==sEntry[i].tvpi && entry.vci==sEntry[i].tvci) {
					//found entry in mibs, get PPP connection type
					if (entry.pppCtype == CONNECT_ON_DEMAND && entry.pppIdleTime != 0)
						pppDod = 1;
					break;
				}
			}
		}

		if (in_turn == 0)
			nBytesSent += websWrite(wp, T("<tr bgcolor=\"#EEEEEE\">\n"));
		else
			nBytesSent += websWrite(wp, T("<tr bgcolor=\"#DDDDDD\">\n"));
		
		in_turn ^= 0x01;
#ifdef CTC_WAN_NAME
#ifdef EMBED
		//jim determine china telecom's auto-generated WAN name...
{
		char applicationname[30];
		MIB_CE_ATM_VC_T Entry;
		int entryNum;
		int mibcnt;
		char vpistr[6];
		char vcistr[6];
		char username[MAX_NAME_LEN];
		char password[MAX_NAME_LEN];
		char interfacename[MAX_NAME_LEN];
		entryNum = mib_chain_total(MIB_ATM_VC_TBL);
#ifdef _CWMP_MIB_ 
		char setname[30];
		memset(setname, 0, 30);
#endif
		char ifname[6];
		char strNapt[10];
		
		
		memset(applicationname, 0, sizeof(applicationname));
		memset(vpistr, 0, sizeof(vpistr));
		memset(vcistr, 0, sizeof(vcistr));		
		memset(interfacename, 0, sizeof(interfacename));
		sprintf(vpistr, "%d", sEntry[i].tvpi);
		sprintf(vcistr, "%d", sEntry[i].tvci);		
		if(!strcmp(sEntry[i].protocol, "br1483"))
		{
			//ql
			strcpy(strNapt, "--");
			
			//bridge mode
			//first found the mib info...
			for(mibcnt=0; mibcnt<entryNum; mibcnt++)
			{
				if (!mib_chain_get(MIB_ATM_VC_TBL, mibcnt, (void *)&Entry))
				{
  					websError(wp, 400, T("Get chain record error!\n"));
					return -1;
				}
				if(Entry.cmode == ADSL_PPPoE || Entry.cmode == ADSL_PPPoA)//star:for multi-ppp name
					snprintf(ifname, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));
				else
					snprintf(ifname, 6, "vc%u", VC_INDEX(Entry.ifIndex));
				if(Entry.vpi==sEntry[i].tvpi && Entry.vci==sEntry[i].tvci && !strcmp(ifname,sEntry[i].ifname))
				{
					//found entry in mibs
					#ifdef _CWMP_MIB_ 
					if(*(Entry.WanName))
						strcpy(setname, Entry.WanName);
					#endif
					setWanName(applicationname, Entry.applicationtype);
					break;
				}
			}
			if(mibcnt==entryNum)
			{
				printf("not found mib entry for this bridge interface\n");
				//set to default application name....
				strcpy(applicationname, "Internet_");
			}
			strcat(applicationname, "B_");
			
			
		}
		else
		{ 
			//ql-- 070926
			for(mibcnt=0; mibcnt<entryNum; mibcnt++) {
				if (!mib_chain_get(MIB_ATM_VC_TBL, mibcnt, (void *)&Entry))
				{
  					websError(wp, 400, T("Get chain record error!\n"));
					return -1;
				}
				if(Entry.cmode == ADSL_PPPoE || Entry.cmode == ADSL_PPPoA)//star:for multi-ppp name
					snprintf(ifname, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));
				else
					snprintf(ifname, 6, "vc%u", VC_INDEX(Entry.ifIndex));
				if(Entry.vpi==sEntry[i].tvpi && Entry.vci==sEntry[i].tvci && !strcmp(ifname,sEntry[i].ifname))
				{
					//found entry in mibs
					if (Entry.napt)
						strcpy(strNapt, "允许");
					else
						strcpy(strNapt, "禁止");
					break;
				}
			}
			
			if(!strcmp(sEntry[i].protocol, "PPPoE") || !strcmp(sEntry[i].protocol, "PPPoA") )
			{
				memset(username, 0, sizeof(username));
				memset(password, 0, sizeof(password));
				if (!(fp=fopen(PPP_CONF, "r")))
					printf("%s not exists.\n", PPP_CONF);
				else {
					int mruxxx;
					int validelements;
					fgets(buff, sizeof(buff), fp);
#if 0
					while ( fgets(buff, sizeof(buff), fp) != NULL )
						if (sscanf(buff, "%s%*s%*s%*d%*s%s%s%*d", interfacename, username, password) != 3) {
							printf("Unsuported ppp configuration format\n");
							break;
						}
						else {
							if(!strcmp(sEntry[i].ifname, interfacename))
							{
								//found the ppp connection's username/password info
								break;
							}
						}
#else
					while ( fgets(buff, sizeof(buff), fp) != NULL )
					{
						validelements=sscanf(buff, "%s%*s%*s%*d%*s%s%s%d", interfacename, username, password, &mruxxx);
						//printf("username=%s, password=%s, ifname=%s, vpi=%d, vci=%d\n", username, password, sEntry[i].ifname, sEntry[i].tvpi, sEntry[i].tvci);
						 //jim since null username or password is valid setting for 531b zte, then the ppp.conf format will be changed
						switch (validelements) {
							case 2:  //both username and  password are NULL...
								memset(username, 0, sizeof(username));									
							case 3: //only  password are NULL...
								memset(password, 0, sizeof(password));
							case 4:
								if(!strcmp(sEntry[i].ifname, interfacename))
								{
									//found the ppp connection's username/password info
									goto found_ppp_local_0;
								}
								break;
							default:  //invalid format...
								printf("Unsuported ppp configuration format\n");
								goto found_ppp_local_0;
						}					
					}
#endif
found_ppp_local_0:					
					fclose(fp);
				}
				
				if(0==getATMEntrybyVPIVCIUsrPswd(&Entry, sEntry[i].tvpi, sEntry[i].tvci, username, password, sEntry[i].ifname))
				{
					//found in mit 
					#ifdef _CWMP_MIB_ 
					if(*(Entry.WanName))
						strcpy(setname, Entry.WanName);
					#endif
					setWanName(applicationname, Entry.applicationtype);
				}else
				{
					printf("not found mib entry for this PPP interface\n");
					//set to default application name....
					strcpy(applicationname, "Internet_");
				}
			}
			else
			{	//rt1483 rt1577
				if(0==getATMEntrybyVPIVCIUsrPswd(&Entry, sEntry[i].tvpi, sEntry[i].tvci, NULL, NULL, sEntry[i].ifname))
				{
					//found in mit 
					#ifdef _CWMP_MIB_ 
					if(*(Entry.WanName))
						strcpy(setname, Entry.WanName);
					#endif
					setWanName(applicationname, Entry.applicationtype);
				}else
				{
					printf("not found mib entry for this PPP interface\n");
					//set to default application name....
					strcpy(applicationname, "Internet_");
				}
				
			}
			strcat(applicationname, "R_");
		}
			strcat(applicationname, vpistr);
			strcat(applicationname, "_");
			strcat(applicationname, vcistr);
			//star: for multi-ppp in one pvc
			if(Entry.cmode == ADSL_PPPoE || Entry.cmode == ADSL_PPPoA)
			{	
					char pppindex[6];
					int intindex;
					intindex = getpppindex(&Entry);
					if(intindex != -1){
							snprintf(pppindex,6,"%u",intindex);
							strcat(applicationname, "_");
							strcat(applicationname, pppindex);
					}
			}

		#ifdef _CWMP_MIB_ 
		if(*setname)
			strcpy(applicationname, setname);
		#endif
#ifdef ZTE_531B_BRIDGE_SC
nBytesSent += websWrite(wp, T(
		"<td align=center width=\"30%%\"><font size=2>Internet_B_%u_%u</td>\n"
		"<td align=center width=\"10%%\"><font size=2>%u/%u</td>\n"
		"<td align=center width=\"20%%\"><font size=2>%s</td>\n"
		"<td align=center width=\"20%%\"><font size=2>%s</td>\n"
		"<td align=center width=\"20%%\"><font size=2>%s\n"),
		sEntry[i].tvpi,sEntry[i].tvci, sEntry[i].tvpi, sEntry[i].tvci, sEntry[i].encaps, 
		sEntry[i].protocol, sEntry[i].strStatus);
#else		
		nBytesSent += websWrite(wp, T(
		"<td align=center width=\"20%%\"><font size=2>%s</td>\n"
		"<td align=center width=\"10%%\"><font size=2>%u/%u</td>\n"
		"<td align=center width=\"6%%\"><font size=2>%s</td>\n"
		"<td align=center width=\"10%%\"><font size=2>%s</td>\n"
		"<td align=center width=\"10%%\"><font size=2>%s</td>\n"
		"<td align=center width=\"17%%\"><font size=2>%s</td>\n"
		"<td align=center width=\"17%%\"><font size=2>%s</td>\n"
		"<td align=center width=\"10%%\"><font size=2>%s\n"),
		applicationname, sEntry[i].tvpi, sEntry[i].tvci, strNapt, sEntry[i].encaps, 
		sEntry[i].protocol, sEntry[i].ipAddr, sEntry[i].remoteIp, sEntry[i].strStatus);
#endif
}
	#endif //EMBED
#else
#ifdef ZTE_531B_BRIDGE_SC
nBytesSent += websWrite(wp, T(
		"<td align=center width=\"30%%\"><font size=2>Internet_B_%u_%u</td>\n"
		"<td align=center width=\"10%%\"><font size=2>%u/%u</td>\n"
		"<td align=center width=\"20%%\"><font size=2>%s</td>\n"
		"<td align=center width=\"20%%\"><font size=2>%s</td>\n"
		"<td align=center width=\"20%%\"><font size=2>%s\n"),
		sEntry[i].tvpi,sEntry[i].tvci, sEntry[i].tvpi, sEntry[i].tvci, sEntry[i].encaps, 
		sEntry[i].protocol, sEntry[i].strStatus);
#else
		nBytesSent += websWrite(wp, T(
		"<td align=center width=\"5%%\"><font size=2>%s</td>\n"
		"<td align=center width=\"5%%\"><font size=2>%u/%u</td>\n"
		"<td align=center width=\"5%%\"><font size=2>%s</td>\n"
		"<td align=center width=\"5%%\"><font size=2>%s</td>\n"
		"<td align=center width=\"10%%\"><font size=2>%s</td>\n"
		"<td align=center width=\"10%%\"><font size=2>%s</td>\n"
		"<td align=center width=\"23%%\"><font size=2>%s\n"),
		if_display_name, sEntry[i].tvpi, sEntry[i].tvci, sEntry[i].encaps, 
		sEntry[i].protocol, sEntry[i].ipAddr, sEntry[i].remoteIp, sEntry[i].strStatus);
#endif
#endif
		if (isPPP) {
			nBytesSent += websWrite(wp, T(" %s / %s "), sEntry[i].uptime, sEntry[i].totaluptime);
			//if (adslflag)
			if (adslflag && !pppDod)
#ifdef ZTE_GENERAL_ROUTER_SC
				if (strcmp(sEntry[i].protocol, "PPPoE")==0)
					nBytesSent += websWrite(wp, T(
					"<input type=\"submit\" id=\"%s\" value=\"%s\" name=\"submit%s\" onClick=\"disButton('%s')\">"),
					sEntry[i].ifname, (pppConnectStatus==1)?"断开":"连接", sEntry[i].ifname,sEntry[i].ifname);
				else
					nBytesSent += websWrite(wp, T(
					"<input type=\"submit\" value=\"%s\" name=\"submit%s\">")
					, (pppConnectStatus==1)?"断开":"连接", sEntry[i].ifname);

#else
				if (strcmp(sEntry[i].protocol, "PPPoE")==0)
					nBytesSent += websWrite(wp, T(
					"<input type=\"submit\" id=\"%s\" value=\"%s\" name=\"submit%s\" onClick=\"disButton('%s')\">"),
					sEntry[i].ifname, (pppConnectStatus==1)?"Disconnect":"Connect", sEntry[i].ifname,sEntry[i].ifname);
				else
					nBytesSent += websWrite(wp, T(
					"<input type=\"submit\" value=\"%s\" name=\"submit%s\">")
					, (pppConnectStatus==1)?"Disconnect":"Connect", sEntry[i].ifname);
#endif
		}
		nBytesSent += websWrite(wp, T("</td></tr>\n"));
	}
	return nBytesSent;
}

#if 0
int wanConfList(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;

	unsigned int entryNum, i;
	MIB_CE_ATM_VC_Tp pEntry;
	char	*mode, vcc[12], *aal5Encap;
	char	ipAddr[20], remoteIp[20];
	char	*strStatus, *temp;
#ifdef IP_PASSTHROUGH
	unsigned char	ippt_itf;
#endif
	int in_turn=0;

#ifdef IP_PASSTHROUGH
	mib_get(MIB_IPPT_ITF, (void *)&ippt_itf);
#endif
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);

	nBytesSent += websWrite(wp, T("<tr bgcolor=\"#808080\">"
	"<td width=\"5%%\" align=center><font size=2><b>Interface</b></td>\n"
	"<td width=\"8%%\" align=center><font size=2><b>VPI/VCI</b></td>\n"
	"<td width=\"8%%\" align=center><font size=2><b>Encap</b></td>\n"
	"<td width=\"8%%\" align=center><font size=2><b>Protocol</b></td>\n"
	"<td width=\"13%%\" align=center><font size=2><b>IP Address</b></td>\n"
	"<td width=\"13%%\" align=center><font size=2><b>Gateway</b></td>\n"
	"<td width=\"8%%\" align=center><font size=2><b>Status</b></td></font></tr>\n"));

	for (i=0; i<entryNum; i++) {
		char ifname[6];
		struct in_addr inAddr;
		int flags;

		pEntry = (MIB_CE_ATM_VC_Tp) mib_chain_get(MIB_ATM_VC_TBL, i); /* get the specified chain record */

		if(pEntry == NULL)
		{
  			websError(wp, 400, T("Get chain record error!\n"));
			return -1;
		}
		
		if (pEntry->enable == 0)
			continue;
		
		mode = 0;
		
		if (pEntry->cmode == ADSL_PPPoE)
			mode = "PPPoE";
		else if (pEntry->cmode == ADSL_PPPoA)
			mode = "PPPoA";
		else if (pEntry->cmode == ADSL_BR1483)
			mode = "br1483";
		else if (pEntry->cmode == ADSL_MER1483)
			mode = "mer1483";
		else if (pEntry->cmode == ADSL_RT1483)
			mode = "rt1483";
		
		snprintf(vcc, 12, "%u/%u", pEntry->vpi, pEntry->vci);
		
		aal5Encap = 0;
		if (pEntry->encap == 0)
			aal5Encap = "VCMUX";
		else
			aal5Encap = "LLC";
		
		if (pEntry->cmode == ADSL_PPPoE || pEntry->cmode == ADSL_PPPoA)
		{
			snprintf(ifname, 6, "ppp%u", PPP_INDEX(pEntry->ifIndex));
			
#ifdef IP_PASSTHROUGH
			if (pEntry->ifIndex == ippt_itf)
				strcpy(ipAddr, STR_UNNUMBERED);
			else
#endif
#ifdef EMBED
			if (getInAddr( ifname, IP_ADDR, (void *)&inAddr) == 1)
			{
				temp = inet_ntoa(inAddr);
				strcpy(ipAddr, temp);
			}
			else
#endif
				strcpy(ipAddr, "");
			
#ifdef IP_PASSTHROUGH
			if (pEntry->ifIndex == ippt_itf)
				strcpy(remoteIp, STR_UNNUMBERED);
			else
#endif
#ifdef EMBED
			if (getInAddr( ifname, DST_IP_ADDR, (void *)&inAddr) == 1)
			{
				temp = inet_ntoa(inAddr);
				strcpy(remoteIp, temp);
			}
			else
#endif
				strcpy(remoteIp, "");
			
			// set status flag
			if (getInFlags( ifname, &flags) == 1)
			{
				if (flags & IFF_UP)
					strStatus = (char *)IF_UP;
				else
					strStatus = (char *)IF_DOWN;
			}
			else
				strStatus = (char *)IF_NA;
		}
		else
		{
			snprintf(ifname, 5, "vc%u", VC_INDEX(pEntry->ifIndex));
			
#ifdef IP_PASSTHROUGH
			if (pEntry->ifIndex == ippt_itf) { // IP unnumbered
				strcpy(ipAddr, STR_UNNUMBERED);
				strcpy(remoteIp, STR_UNNUMBERED);
			}
			else
#endif
			if (pEntry->ipDhcp == (char)DHCP_DISABLED)
			{
				if (pEntry->cmode == ADSL_BR1483) {
					strcpy(ipAddr, "");
					strcpy(remoteIp, "");
				}
				else {
					// static IP address
					temp = inet_ntoa(*((struct in_addr *)pEntry->ipAddr));
					strcpy(ipAddr, temp);
					
					temp = inet_ntoa(*((struct in_addr *)pEntry->remoteIpAddr));
					strcpy(remoteIp, temp);
				}
			}
			else
			{
				// DHCP enabled
#ifdef EMBED
				if (getInAddr( ifname, IP_ADDR, (void *)&inAddr) == 1)
				{
					temp = inet_ntoa(inAddr);
					strcpy(ipAddr, temp);
				}
				else
#endif
					strcpy(ipAddr, "");
				
#ifdef EMBED
				if (getInAddr( ifname, DST_IP_ADDR, (void *)&inAddr) == 1)
				{
					temp = inet_ntoa(inAddr);
					strcpy(remoteIp, temp);
				}
				else
#endif
					strcpy(remoteIp, "");
			}
				
			// set status flag
			if (getInFlags( ifname, &flags) == 1)
			{
				if (flags & IFF_UP) {
					if (pEntry->cmode == ADSL_BR1483)
						strStatus = (char *)IF_UP;
					else {
						if (getInAddr( ifname, IP_ADDR, (void *)&inAddr) == 1)
							strStatus = (char *)IF_UP;
						else
							strStatus = (char *)IF_DOWN;
					}
				}
				else
					strStatus = (char *)IF_DOWN;
			}
			else
				strStatus = (char *)IF_NA;
		}
		
		if (in_turn == 0)
			nBytesSent += websWrite(wp, T("<tr bgcolor=\"#EEEEEE\">\n"));
		else
			nBytesSent += websWrite(wp, T("<tr bgcolor=\"#DDDDDD\">\n"));
		
		in_turn ^= 0x01;
		nBytesSent += websWrite(wp, T(
		"<td align=center width=\"5%%\"><font size=2>%s</td>\n"
		"<td align=center width=\"8%%\"><font size=2>%s</td>\n"
		"<td align=center width=\"8%%\"><font size=2>%s</td>\n"
		"<td align=center width=\"8%%\"><font size=2>%s</td>\n"
		"<td align=center width=\"13%%\"><font size=2>%s</td>\n"
		"<td align=center width=\"13%%\"><font size=2>%s</td>\n"
		"<td align=center width=\"8%%\"><font size=2>%s</td></tr>\n"),
		ifname, vcc, aal5Encap, mode, ipAddr, remoteIp, strStatus);
	}

	return nBytesSent;
}
#endif
