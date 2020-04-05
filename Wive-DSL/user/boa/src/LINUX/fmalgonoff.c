/*
 * =====================================================================================
 *
 *       Filename:  fmalgonoff.c
 *
 *    Description:  control the on-off of ALG by the web
 *
 *        Version:  1.0
 *        Created:  08/16/07 15:54:05    
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Ramen.Shen (Mr), ramen_shen@realsil.com.cn
 *        Company:  REALSIL Microelectronics Inc
 *
 * =====================================================================================
 */

/*-- System inlcude files --*/
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/route.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../../uClibc/include/linux/autoconf.h"
#endif
#ifdef CONFIG_IP_NF_ALG_ONOFF
enum algtype{
#ifdef CONFIG_IP_NF_FTP
    ALG_TYPE_FTP=0,
#endif
#ifdef CONFIG_IP_NF_H323	
    ALG_TYPE_H323,
#endif
#ifdef CONFIG_IP_NF_IRC
    ALG_TYPE_IRC,
 #endif
 #ifdef CONFIG_IP_NF_RTSP
    ALG_TYPE_RTSP,
 #endif
 #ifdef CONFIG_IP_NF_QUAKE3
    ALG_TYPE_QUAKE3,
 #endif
 #ifdef CONFIG_IP_NF_CUSEEME
    ALG_TYPE_CUSEEME,
 #endif
 #ifdef CONFIG_IP_NF_L2TP
    ALG_TYPE_L2TP,
 #endif
 #ifdef CONFIG_IP_NF_IPSEC
    ALG_TYPE_IPSEC,
 #endif
 #ifdef CONFIG_IP_NF_SIP
    ALG_TYPE_SIP,
#endif
 #ifdef CONFIG_IP_NF_PPTP
    ALG_TYPE_PPTP,  
#endif
    ALG_TYPE_MAX
};
struct {
	unsigned char id;
	unsigned int mibalgid;
	char* name;
}algTypeName[]={
#ifdef CONFIG_IP_NF_FTP
{ALG_TYPE_FTP,MIB_IP_ALG_FTP,"ftp"},
#endif
#ifdef CONFIG_IP_NF_H323
{ALG_TYPE_H323,MIB_IP_ALG_H323,"h323"},
#endif
#ifdef CONFIG_IP_NF_IRC
{ALG_TYPE_IRC,MIB_IP_ALG_IRC,"irc"},
#endif
#ifdef CONFIG_IP_NF_RTSP
{ALG_TYPE_RTSP,MIB_IP_ALG_RTSP,"rtsp"},
#endif
#ifdef CONFIG_IP_NF_QUAKE3
{ALG_TYPE_QUAKE3,MIB_IP_ALG_QUAKE3,"quake3"},
#endif
#ifdef CONFIG_IP_NF_CUSEEME
{ALG_TYPE_CUSEEME,MIB_IP_ALG_CUSEEME,"cuseeme"},
#endif
#ifdef CONFIG_IP_NF_L2TP
{ALG_TYPE_L2TP,MIB_IP_ALG_L2TP,"l2tp"},
#endif
#ifdef CONFIG_IP_NF_IPSEC
{ALG_TYPE_IPSEC,MIB_IP_ALG_IPSEC,"ipsec"},
#endif
#ifdef CONFIG_IP_NF_SIP
{ALG_TYPE_SIP,MIB_IP_ALG_SIP,"sip"},
#endif
 #ifdef CONFIG_IP_NF_PPTP
{ALG_TYPE_PPTP, MIB_IP_ALG_PPTP,"pptp"}, 
 #endif
{ALG_TYPE_MAX,0,NULL}
};
void formALGOnOff(webs_t wp, char_t *path, char_t *query)
{

	char_t	*str, *submitUrl;
	char tmpBuf[100];	
	char cmdstr[128]={0};
	//char cmdbuf[8]={0};
	unsigned char  algenable=0;
	str = websGetVar(wp, T("apply"), T(""));
	if (str[0]) {
	int i=0;	
	for(i=0;i<ALG_TYPE_MAX;i++)
		{
		char algTypeStr[32]={0};
		snprintf(algTypeStr,sizeof(algTypeStr),"%s_algonoff",algTypeName[i].name);
		str=websGetVar(wp, algTypeStr, T(""));
		if(str[0])
			{
			algenable=str[0]-'0';
			  snprintf(cmdstr,sizeof(cmdstr),"/proc/%s_algonoff",algTypeName[i].name);
			 FILE *fp=fopen(cmdstr,"w");
			 if(fp)
			 	{
			 	fwrite(str,sizeof(char),1,fp);
				fclose(fp);
			 	}
			if(!mib_set(algTypeName[i].mibalgid,(void*)&algenable))
				printf("%s mib set %d error!\n",__FUNCTION__,algTypeName[i].mibalgid);
			}		
		}			
 	}	
	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
	return;	
}
void GetAlgTypes(webs_t wp)
{
int i = 0;
for(i=0;i<ALG_TYPE_MAX;i++)
{
#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
	websWrite(wp,T("<tr>\n <td ><font size=2>%s</td>\n"
	"  <td ><font size=2><input type=\"radio\" name=%s_algonoff value=1 >∆Ù”√   </font></td>\n"
	"     <td ><font size=2><input type=\"radio\" name=%s_algonoff value=0 >πÿ±’   </font> </td>\n " 	
	"</tr>\n"),algTypeName[i].name,algTypeName[i].name,algTypeName[i].name);
#else
	websWrite(wp,T("<tr>\n <td ><font size=2>%s</td>\n"
	"  <td ><font size=2><input type=\"radio\" name=%s_algonoff value=1 >Enable</font></td>\n"
	"     <td ><font size=2><input type=\"radio\" name=%s_algonoff value=0 >Disable</font> </td>\n " 	
	"</tr>\n"),algTypeName[i].name,algTypeName[i].name,algTypeName[i].name);
#endif
}
return;
}
void CreatejsAlgTypeStatus(webs_t wp)
{
unsigned char  value=0;
int i =0;
for(i=0;i<ALG_TYPE_MAX;i++)
{
	mib_get(algTypeName[i].mibalgid,&value);	
	websWrite(wp,"document.algof.%s_algonoff[%d].checked=true;\n",algTypeName[i].name,!value&0x01);	
}	
return;

}
void initAlgOnOff(webs_t wp)
{
	return;   
}
#endif



