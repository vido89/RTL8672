/*
 *	climenu.c
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "options.h"
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <linux/atm.h>
#include <linux/atmdev.h>
#include <sys/sysinfo.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/if_bridge.h>
#include	<sys/param.h>
#include <net/route.h>
#include <pwd.h>
#ifdef HAVE_SHADOW_H
#include <shadow.h>
#endif

#include "mib.h"
#include "adsl_drv.h"
#include "utility.h"
#include "webform.h"
#include "../md5.h"
#include <crypt.h>
#include <time.h>
#include <sys/stat.h>


#define CLIDEBUG printf
// Kaohj, defined if you want to clear the screen
#define CLEAR_SCR

#ifdef CLEAR_SCR
#define CLEAR		printf("\033[H\033[J")
#else
#define CLEAR		(void)0
#endif
#define MENU_LINE	printf("%s\n", menuBar);
#define MSG_LINE	printf("%s\n", messageBar);

//#define MAX_POE_PER_VC		2
#define FALSE 0
#define TRUE  1

typedef enum {
	SECURITY_ROOT,
	SECURITY_SUPERUSER,
	SECURITY_USER
} SECURITY_LEVEL;

static char strbuf[256];
static const char *emptystr = "";
static const char *menuBar = "------------------------------------------------------------";
static const char *messageBar = "-------------------------------------------------------------------------";
static const char IF_UP[] = "up";
static const char IF_DOWN[] = "down";
static const char IF_NA[] = "n/a";
static const char IF_DISABLE[] = "Disable";
static const char IF_ENABLE[]= "Enable";
static const char IF_ON[] = "On";
static const char IF_OFF[] = "Off";
static const char *strQos[] = { "UBR", "CBR", "nrt-VBR", "rt-VBR" };
static const char *g_pszMode[] = { "br1483", "mer1483", "PPPoE", "PPPoA", "rt1483"
                                   #ifdef CONFIG_ATM_CLIP
                                   , "rt1577"
                                   #endif                                    
                                  };
static const char NOT_AUTHORIZED[] = "\nNot Authorized!\n";

static SECURITY_LEVEL loginLevel;	// security level: each login user has its own security level

unsigned char first_time=1;
unsigned char dbg_enable=0;


static int exstat=1;
static void leave(void) __attribute__ ((noreturn));	/* abort cli shell */
void printWaitStr();

// Mason Yu
char suName[MAX_NAME_LEN];
char usName[MAX_NAME_LEN];

static char base64chars[64] = "abcdefghijklmnopqrstuvwxyz"
                              "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";

/*
 * Name: base64encode()
 *
 * Description: Encodes a buffer using BASE64.
 */
void base64encode(unsigned char *from, char *to, int len)
{
  while (len) {
    unsigned long k;
    int c;

    c = (len < 3) ? len : 3;
    k = 0;
    len -= c;
    while (c--)
      k = (k << 8) | *from++;
    *to++ = base64chars[ (k >> 18) & 0x3f ];
    *to++ = base64chars[ (k >> 12) & 0x3f ];
    *to++ = base64chars[ (k >> 6) & 0x3f ];
    *to++ = base64chars[ k & 0x3f ];
  }
  *to++ = 0;
}

void calPasswdMD5(char *pass, char *passMD5)
{
	char temps[0x100],*pwd;
	struct MD5Context mc;
 	unsigned char final[16];
	char encoded_passwd[0x40];
  	/* Encode password ('pass') using one-way function and then use base64
	 encoding. */

	MD5Init(&mc);
	{

	//printf("calPasswdMD5: pass=%s\n", pass);
	MD5Update(&mc, pass, strlen(pass));
	}
	MD5Final(final, &mc);
	strcpy(encoded_passwd,"$1$");
	base64encode(final, encoded_passwd+3, 16);
        //printf("encoded_passwd=%s for %s\n",encoded_passwd, pass);

        strcpy(passMD5, encoded_passwd);

}

#ifdef EMBED
#ifndef MULTI_USER_PRIV
// Added by Mason Yu(2 level)
void writePasswdFile()
{
	FILE *fp, *fp2;
	char suPasswd[MAX_NAME_LEN], usPasswd[MAX_NAME_LEN];
	char suPasswdMD5[50], usPasswdMD5[50];
#ifdef ACCOUNT_CONFIG
	MIB_CE_ACCOUNT_CONFIG_T entry;
	unsigned int totalEntry;
	int i;

	totalEntry = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL); /* get chain record size */
#endif

	// Added by Mason Yu for write superUser into Current Setting
	if ( !mib_get(MIB_SUSER_NAME, (void *)suName) ) {
		printf("ERROR: Get superuser name from MIB database failed.\n");
		return;
	}

	if ( !mib_get(MIB_USER_NAME, (void *)usName) ) {
		printf("ERROR: Get user name from MIB database failed.\n");
		return;
	}

	if ( !mib_get(MIB_SUSER_PASSWORD, (void *)suPasswd) ) {
		printf("ERROR: Set superuser password to MIB database failed.\n");
		return;
	}

	if ( !mib_get(MIB_USER_PASSWORD, (void *)usPasswd) ) {
		printf("ERROR: Set superuser password to MIB database failed.\n");
		return;
	}

	calPasswdMD5(suPasswd, suPasswdMD5);
	calPasswdMD5(usPasswd, usPasswdMD5);
	//calPasswdMD5("realtek", PasswdMD5);
	//printf("PasswdMD5=%s\n", PasswdMD5);


	// boa.passwd  "/var/snmpComStr.conf"
	if ((fp = fopen("/var/boaUser.passwd", "w")) == NULL)
	{
		printf("***** Open file /var/boaUser.passwd failed !\n");
		return;
	}

	if ((fp2 = fopen("/var/boaSuper.passwd", "w")) == NULL)
	{
		printf("***** Open file /var/boaSuper.passwd failed !\n");
		return;
	}

#ifndef ZTE_531B_BRIDGE_SC
	fprintf(fp, "%s:%s\n", usName, usPasswdMD5);
#endif
	fprintf(fp, "%s:%s\n", suName, suPasswdMD5);
	fprintf(fp2, "%s:%s\n", suName, suPasswdMD5);

#ifdef ACCOUNT_CONFIG
	for (i=0; i<totalEntry; i++) {
		if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, i, (void *)&entry)) {
			printf("ERROR: Get account configuration information from MIB database failed.\n");
			fclose(fp);
			fclose(fp2);
			return;
		}
		calPasswdMD5(entry.userPassword, usPasswdMD5);

		switch(entry.privilege) {
			case PRIV_USER:
				#ifndef ZTE_531B_BRIDGE_SC
				fprintf(fp, "%s:%s\n", entry.userName, usPasswdMD5);
				#endif
				break;

			case PRIV_ENG:
				break;

			case PRIV_ROOT:
				fprintf(fp, "%s:%s\n", entry.userName, usPasswdMD5);
				fprintf(fp2, "%s:%s\n", entry.userName, usPasswdMD5);
				break;

			default:
				printf("Wrong privilege!\n");
				break;
		}
	}
#endif

	fclose(fp);
	fclose(fp2);

	#ifdef SUPPORT_AUTH_DIGEST
	if(!(fp = fopen("/var/DigestUser.passwd", "w")))
	{
		printf("***** Open file /var/DigestUser.passwd failed !\n");
		return;
	}
	fprintf(fp, "%s\n%s\n%s\n%s\n", usName, usPasswd, suName, suPasswd);
	fclose(fp);

	if(!(fp = fopen("/var/DigestSuper.passwd", "w")))
	{
		printf("***** Open file /var/DigestSuper.passwd failed!\n");
		return;
	}
	fprintf(fp, "%s\n%s\n", suName, suPasswd);
	fclose(fp);
	#endif


}
#else
void writePasswdFile()
{
	FILE *fp;	
#ifdef SUPPORT_AUTH_DIGEST
	FILE *fpd;	
#endif
	char suPasswd[MAX_NAME_LEN], usPasswd[MAX_NAME_LEN];
	char suPasswdMD5[50], usPasswdMD5[50];
	MIB_CE_ACCOUNT_CONFIG_T entry;
	unsigned int totalEntry;
	int i;

	if (!mib_get(MIB_SUSER_NAME, (void *)suName)) {
		printf("ERROR: Get superuser name from MIB database failed.\n");
		return;
	}
	if (!mib_get(MIB_USER_NAME, (void *)usName)) {
		printf("ERROR: Get user name from MIB database failed.\n");
		return;
	}
	if (!mib_get(MIB_SUSER_PASSWORD, (void *)suPasswd)) {
		printf("ERROR: Set superuser password to MIB database failed.\n");
		return;
	}
	if (!mib_get(MIB_USER_PASSWORD, (void *)usPasswd)) {
		printf("ERROR: Set user password to MIB database failed.\n");
		return;
	}
	calPasswdMD5(suPasswd, suPasswdMD5);
	calPasswdMD5(usPasswd, usPasswdMD5);
#ifdef SUPPORT_AUTH_DIGEST
	if(!(fpd = fopen("/var/DigestUser.passwd", "w")))
	{
		printf("***** Open file /var/DigestUser.passwd failed !\n");
		return;
	}
	fprintf(fpd, "%s\n%s\n%s\n%s\n", usName, usPasswd, suName, suPasswd);
#endif
	if ((fp = fopen("/var/boaUser.passwd", "w")) == NULL) {
		printf("***** Open file /var/boaUser.passwd failed !\n");
		return;
	}
	fprintf(fp, "%s:%s\n", usName, usPasswdMD5);
	fprintf(fp, "%s:%s\n", suName, suPasswdMD5);

	totalEntry = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL); /* get chain record size */
	for (i=0; i<totalEntry; i++) {
		if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, i, (void *)&entry)) {
			printf("ERROR: Get account configuration information from MIB database failed.\n");
			fclose(fp);
			return;
		}
		calPasswdMD5(entry.userPassword, usPasswdMD5);
#ifdef SUPPORT_AUTH_DIGEST
		fprintf(fpd, "%s\n%s\n", entry.userName, entry.userPassword);
#endif
		fprintf(fp, "%s:%s\n", entry.userName, usPasswdMD5);
	}
#ifdef SUPPORT_AUTH_DIGEST
	fclose(fpd);
#endif
	fclose(fp);

}
#endif
#endif

void write_etcPassword()
{
	FILE *fp;
	char userName[MAX_NAME_LEN], userPass[MAX_NAME_LEN];
	char *xpass;
#ifdef ACCOUNT_CONFIG
	MIB_CE_ACCOUNT_CONFIG_T entry;
	unsigned int totalEntry;
#endif
	int i;

	fp = fopen("/var/passwd", "w+");
#ifdef ACCOUNT_CONFIG
	totalEntry = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL); /* get chain record size */
	for (i = 0; i < totalEntry; i++) {
		if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, i, (void *)&entry)) {
			printf("ERROR: Get account configuration information from MIB database failed.\n");
			return;
		}
		strcpy(userName, entry.userName);
		strcpy(userPass, entry.userPassword);
		xpass = crypt(userPass, "$1$");
		if (xpass) {
			//#ifdef CONFIG_USER_MENU_CLI
			#ifdef CONFIG_USER_CLI
			if (entry.privilege == (unsigned char)PRIV_ROOT)
				fprintf(fp, "%s:%s:0:0::/tmp:/bin/cli\n", userName, xpass);
			else
				fprintf(fp, "%s:%s:1:0::/tmp:/bin/cli\n", userName, xpass);
			#else
			if (entry.privilege == (unsigned char)PRIV_ROOT)
				fprintf(fp, "%s:%s:0:0::/tmp:/bin/sh\n", userName, xpass);
			else
				fprintf(fp, "%s:%s:1:0::/tmp:/bin/sh\n", userName, xpass);
			#endif
		}
	}
#endif
	mib_get( MIB_SUSER_NAME, (void *)userName );
	mib_get( MIB_SUSER_PASSWORD, (void *)userPass );
	xpass = crypt(userPass, "$1$");
	if (xpass)
//#ifdef CONFIG_USER_MENU_CLI
#ifdef CONFIG_USER_CLI
		fprintf(fp, "%s:%s:0:0::/tmp:/bin/cli\n", userName, xpass);
#else
		fprintf(fp, "%s:%s:0:0::/tmp:/bin/sh\n", userName, xpass);
#endif
		
	// Added by Mason Yu for others user	
	mib_get( MIB_SUPER_NAME, (void *)userName );
	mib_get( MIB_SUPER_PASSWORD, (void *)userPass );
	xpass = crypt(userPass, "$1$");
	if (xpass)
//#ifdef CONFIG_USER_MENU_CLI
#ifdef CONFIG_USER_CLI
		fprintf(fp, "%s:%s:0:0::/tmp:/bin/cli\n", userName, xpass);
#else
		fprintf(fp, "%s:%s:0:0::/tmp:/bin/sh\n", userName, xpass);
#endif

			
	mib_get( MIB_USER_NAME, (void *)userName );
	if (userName[0]) {
		mib_get( MIB_USER_PASSWORD, (void *)userPass );				
		xpass = crypt(userPass, "$1$");
		if (xpass)
//#ifdef CONFIG_USER_MENU_CLI
#ifdef CONFIG_USER_CLI
			fprintf(fp, "%s:%s:1:0::/tmp:/bin/cli\n", userName, xpass);
#else
			fprintf(fp, "%s:%s:1:0::/tmp:/bin/sh\n", userName, xpass);
#endif

	}	
	fclose(fp);
	chmod("/var/tmp", 0x1fd);	// let owner and group have write access
}


static int check_access(SECURITY_LEVEL level)
{
	/*if (loginLevel > level) {
		printf(NOT_AUTHORIZED);
		printWaitStr();
		return 0;
	}*/ //moved to login
	return 1;
}

// map: bit map of used interface, ppp index (0~15) is mapped into high 16 bits,
// while vc index (0~15) is mapped into low 16 bits.
// return: interface index, high nibble for PPP index and low nibble for vc index.
//		0xef: PPP not available
//		0xff: vc not available
static unsigned char if_find_index(int cmode, unsigned int map)
{
	int i;
	unsigned char index;
	
	// find the first available vc index (mpoa interface)
	i = 0;
	for (i=0; i<MAX_VC_NUM; i++)
	{
		if (!((map>>i) & 1))
			break;
	}
	
	if (i != MAX_VC_NUM)
		index = i;
	else
		return 0xff;
	
	if (cmode == ADSL_PPPoE || cmode == ADSL_PPPoA)
	{
		// find an available PPP index
		map >>= 16;
		i = 0;
		while (map & 1)
		{
			map >>= 1;
			i++;
		}
		if (i<=(MAX_PPP_NUM-1))
			index |= i << 4;	// high nibble for PPP index
		else
			return 0xef;
		
		if (cmode == ADSL_PPPoA)
			index |= 0x0f;	// PPPoA doesn't use mpoa interface, set to 0x0f (don't care)
	}
	else
	{
		// don't care the PPP index
		index |= 0xf0;
	}
	return index;
}

static void resolveServiceDependency(idx)
{
	MIB_CE_ATM_VC_T Entry;
#ifdef CONFIG_USER_IGMPPROXY
	unsigned char igmp_proxy;
#endif
#ifdef IP_PASSTHROUGH
	unsigned char ippt_itf;
	unsigned int ippt_lease;
#endif
	struct data_to_pass_st msg;
	
	/* get the specified chain record */
	if (!mib_chain_get(MIB_ATM_VC_TBL, idx, (void *)&Entry))
	{
		return;
	}
	
#ifdef CONFIG_USER_IGMPPROXY
	// resolve IGMP proxy dependency
	if(!mib_get( MIB_IGMP_PROXY_ITF,  (void *)&igmp_proxy))
		return;
	if (Entry.ifIndex == igmp_proxy)
	{ // This interface is IGMP proxy interface
		igmp_proxy = 0xff;	// set to default
		mib_set(MIB_IGMP_PROXY_ITF, (void *)&igmp_proxy);
		igmp_proxy = 0;	// disable IGMP proxy
		mib_set(MIB_IGMP_PROXY, (void *)&igmp_proxy);
	}
#endif
	
#ifdef IP_PASSTHROUGH
	// resolve IP passthrough dependency
	if(!mib_get( MIB_IPPT_ITF,  (void *)&ippt_itf))
		return;
	if (Entry.ifIndex == ippt_itf)
	{ // This interface is IP passthrough interface
		ippt_itf = 0xff;	// set to default
		mib_set(MIB_IPPT_ITF, (void *)&ippt_itf);
		ippt_lease = 600;	// default to 10 min.
		mib_set(MIB_IPPT_LEASE, (void *)&ippt_lease);
	}
#endif
}

static char *getMibInfo(int id) {
	if (getMIB2Str(id, strbuf) == 0)
		return strbuf;
	
	return (char *)emptystr;
}

/* Display wan configuration */
/* int bShowIndex: indicates whether show the index */
int _wanConfList(int bShowIndex)
{
    unsigned int entryNum, i;
    MIB_CE_ATM_VC_T Entry;
    char vcc[12], *aal5Encap;
    const char *mode;
#ifdef CONFIG_GUI_WEB
    char *pszNapt, ipAddr[20], szMask[20], remoteIp[20], szUserName[P_MAX_NAME_LEN];
#else
    char *pszNapt, ipAddr[20], szMask[20], remoteIp[20], szUserName[MAX_PPP_NAME_LEN+1];
#endif
#ifdef DEFAULT_GATEWAY_V1
    const char *strDroute;
#endif
    char    *strStatus, *temp;
    int in_turn = 0, adslflag;

    if (bShowIndex)
    {
        printf("%-4s", "Idx");
    }
    
#ifdef DEFAULT_GATEWAY_V1
    printf("%-6s%-7s%-7s%-8s%-16s%-16s%-16s%-5s%-30s%-8s%-7s\n","Inf","PVC","Encap","Mode","IPAddr","RemoteIP","Mask","NAPT","UserName","Status","DRoute");
#else
    printf("%-6s%-7s%-7s%-8s%-16s%-16s%-16s%-5s%-30s%-8s\n","Inf","PVC","Encap","Mode","IPAddr","RemoteIP","Mask","NAPT","UserName","Status");
#endif

    MSG_LINE;

    entryNum = mib_chain_total(MIB_ATM_VC_TBL);
    //cathy, for  bug B011
    Modem_LinkSpeed vLs;
    // check for xDSL link
    if (!adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE) || vLs.upstreamRate == 0)
        adslflag = 0;
    else
        adslflag = 1;

    for (i=0; i<entryNum; i++) 
    {
        char ifname[6];
        struct in_addr inAddr;
        int flags;

        if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
        {
            return -1;
        }

        // set status flag
        if (Entry.enable == 0)
        {
            strStatus = (char *)IF_DISABLE;
        }
        else
        {
            strStatus = (char *)IF_ENABLE;
        }

        /* Initialize */
        ipAddr[0]     = '\0';
        szMask[0]     = '\0';
        remoteIp[0]   = '\0';
        szUserName[0] = '\0';
        pszNapt       = "";
#ifdef DEFAULT_GATEWAY_V1
        strDroute     = "";
#endif
        
        mode = 0;
#ifdef CONFIG_ATM_CLIP
        if (Entry.cmode <= ADSL_RT1577)
#else
        if (Entry.cmode <= ADSL_RT1483)
#endif
        {
            mode = g_pszMode[Entry.cmode];
        }

        snprintf(vcc, 12, "%u/%u", Entry.vpi, Entry.vci);

        aal5Encap = 0;
        if (Entry.encap == 0)
        {
            aal5Encap = "VCMUX";
        }
        else
        {
            aal5Encap = "LLC";
        }

        if (Entry.napt == 0)
        {
            pszNapt = (char*)IF_OFF;
        }
        else
        {
            pszNapt = (char*)IF_ON;
        }
        
        if (Entry.cmode == ADSL_PPPoE || Entry.cmode == ADSL_PPPoA)
        {
#ifdef CONFIG_GUI_WEB
            strncpy(szUserName, Entry.pppUsername, P_MAX_NAME_LEN-1);
            szUserName[P_MAX_NAME_LEN-1] = '\0';
            //szUserName[P_MAX_NAME_LEN] = '\0';
#else
            strncpy(szUserName, Entry.pppUsername, MAX_PPP_NAME_LEN);
            szUserName[MAX_NAME_LEN] = '\0';
            //szUserName[MAX_NAME_LEN] = '\0';
#endif
            
            snprintf(ifname, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));

#ifdef CONFIG_SPPPD_STATICIP
            if (Entry.cmode == ADSL_PPPoE && Entry.pppIp) 
            {
                temp = inet_ntoa(*((struct in_addr *)Entry.ipAddr));
                strcpy(ipAddr, temp);
                
                temp = inet_ntoa(*((struct in_addr *)Entry.netMask));
                strcpy(szMask, temp);
            }
#endif

        }
        else
        {
            snprintf(ifname, 5, "vc%u", VC_INDEX(Entry.ifIndex));
            
            if ((Entry.ipDhcp == (char)DHCP_DISABLED)
                && (!Entry.ipunnumbered)
                && (Entry.cmode != ADSL_BR1483))
            {
                // static IP address
                temp = inet_ntoa(*((struct in_addr *)Entry.ipAddr));
                strcpy(ipAddr, temp);

                temp = inet_ntoa(*((struct in_addr *)Entry.remoteIpAddr));
                strcpy(remoteIp, temp);

                temp = inet_ntoa(*((struct in_addr *)Entry.netMask)); // Jenny, subnet mask
                strcpy(szMask, temp);
            }
            
        }

#ifdef DEFAULT_GATEWAY_V1
        if (Entry.dgw == 0) // Jenny, default route
            strDroute = IF_OFF;
        else
            strDroute = IF_ON;
#endif

        in_turn ^= 0x01;

        if (bShowIndex)
        {
            printf("%-4d", i);
        }
        
#ifdef DEFAULT_GATEWAY_V1
        printf("%-6s%-7s%-7s%-8s%-16s%-16s%-16s%-5s%-30s%-8s%-7s\n", ifname, vcc, aal5Encap, 
                mode, ipAddr, remoteIp, szMask, pszNapt, szUserName, strStatus, strDroute);
#else
        printf("%-6s%-7s%-7s%-8s%-16s%-16s%-16s%-5s%-30s%-8s\n", ifname, vcc, aal5Encap, 
                mode, ipAddr, remoteIp, szMask, pszNapt, szUserName, strStatus);
#endif
    }

    return entryNum;
}

/* Display wan connection */
/* int bShowIndex: indicates whether show the index */
int wanConnList(int bShowIndex)
{
    unsigned int uiEntryNum, i;
    MIB_CE_ATM_VC_T stEntry;
    char szVcc[12], *szAal5Encap, szIpAddr[20], szGateway[20], *pszStatus, *pszTemp;
    const char *pszMode;
    int iAdslflag;
    Modem_LinkSpeed stLs;

    if (bShowIndex)
    {
        printf("%-4s%-10s%-8s%-7s%-9s%-16s%-16s%-7s\n","Idx","Interface","VPI/VCI","Encap","Protocol","IP Address","Gateway","Status");
    }
    else
    {
        printf("%-10s%-8s%-7s%-9s%-16s%-16s%-7s\n","Interface","VPI/VCI","Encap","Protocol","IP Address","Gateway","Status");
    }
    
    MSG_LINE;

    uiEntryNum = mib_chain_total(MIB_ATM_VC_TBL);
    if (!adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&stLs, RLCM_GET_LINK_SPEED_SIZE) || stLs.upstreamRate == 0)
    {
        iAdslflag = 0;
    }
    else
    {
        iAdslflag = 1;
    }

    for (i = 0; i < uiEntryNum; i++) 
    {
        char szIfname[6];
        struct in_addr stInAddr;
        int iFlags;

        if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&stEntry))
        {
            return -1;
        }

        if (stEntry.enable == 0)
            continue;

        /* Initialize */
        szIpAddr[0]  = '\0';
        szGateway[0] = '\0';
        
        pszMode = 0;
#ifdef CONFIG_ATM_CLIP
        if (stEntry.cmode <= ADSL_RT1577)
#else
        if (stEntry.cmode <= ADSL_RT1483)
#endif
        {
            pszMode = g_pszMode[stEntry.cmode];
        }

        snprintf(szVcc, 12, "%u/%u", stEntry.vpi, stEntry.vci);

        if (stEntry.encap == 0)
        {
            szAal5Encap = "VCMUX";
        }
        else
        {
            szAal5Encap = "LLC";
        }
        
        if ((ADSL_PPPoE == stEntry.cmode) || (ADSL_PPPoA == stEntry.cmode))
        {
            snprintf(szIfname, 6, "ppp%u", PPP_INDEX(stEntry.ifIndex));
            
#ifdef EMBED
            if (getInAddr(szIfname, IP_ADDR, (void *)&stInAddr) == 1)
            {
                pszTemp = inet_ntoa(stInAddr);
                if (getInFlags(szIfname, &iFlags) == 1)
                {
                    // Jenny,  buglist B063, check if IP Passthrough or IP unnumbered
                    if ((strcmp(pszTemp, "10.0.0.1") == 0) && iFlags & IFF_POINTOPOINT)
                    {
                        strcpy(szIpAddr, STR_UNNUMBERED);
                    }
			else if (strcmp(pszTemp, "64.64.64.64") == 0)
				strcpy(szIpAddr, "");
                    else
                    {
                        strcpy(szIpAddr, pszTemp);
                    }
                }
            }
            
            if (getInAddr(szIfname, DST_IP_ADDR, (void *)&stInAddr) == 1)
            {
                pszTemp = inet_ntoa(stInAddr);
                if (strcmp(pszTemp, "10.0.0.2") == 0)
                // Jenny,  buglist B063, check if IP Passthrough or IP unnumbered
                {
                    strcpy(szGateway, STR_UNNUMBERED);
                }
		else if (strcmp(pszTemp, "64.64.64.64") == 0)
			strcpy(szGateway, "");
                else
                {
                    strcpy(szGateway, pszTemp);
                }
            }
#endif

             // set status flag
            if (getInFlags(szIfname, &iFlags) == 1)
            {
                if (iFlags & IFF_UP)
                {
			if (getInAddr(szIfname, IP_ADDR, (void *)&stInAddr) == 1) {
				if (stInAddr.s_addr == 0x40404040)
					pszStatus = (char *)IF_DOWN;
				else
					pszStatus = (char *)IF_UP;
			}
                }
                else 
                {
                    //cathy, for  bug B011
                    pszStatus = (char *)IF_DOWN;
                    szIpAddr[0]  = '\0';
                    szGateway[0] = '\0';
                }
            }
            else
            {
                pszStatus = (char *)IF_NA;
            }
        }
        else
        {
            snprintf(szIfname, 5, "vc%u", VC_INDEX(stEntry.ifIndex));
            
#ifdef EMBED
            // Jenny, sync cli IP status with web page
            if (getInAddr(szIfname, IP_ADDR, (void *)&stInAddr) == 1)
            {
                pszTemp = inet_ntoa(stInAddr);
                if (getInFlags(szIfname, &iFlags) == 1)
                {
                    if ((strcmp(pszTemp, "10.0.0.1") == 0) && iFlags & IFF_POINTOPOINT)
                    // IP Passthrough or IP unnumbered
                    {
                        strcpy(szIpAddr, STR_UNNUMBERED);
                    }
                    else
                    {
                        strcpy(szIpAddr, pszTemp);
                    }
                }
            }
            
            if (getInAddr(szIfname, DST_IP_ADDR, (void *)&stInAddr) == 1)
            {
                pszTemp = inet_ntoa(stInAddr);
                if (strcmp(pszTemp, "10.0.0.2") == 0)
                {
                    strcpy(szGateway, STR_UNNUMBERED);
                }
                else
                {
                    strcpy(szGateway, pszTemp);
                }
                
                if (getInFlags(szIfname, &iFlags) == 1)
                {
                    if (iFlags & IFF_BROADCAST) 
                    {
                        unsigned char aucValue[32];
                        FILE *pstFile;
                        
                        snprintf(aucValue, 32, "%s.%s", (char *)MER_GWINFO, szIfname);
                        if (pstFile = fopen(aucValue, "r")) 
                        {
                            fscanf(pstFile, "%s\n", szGateway);
                            fclose(pstFile);
                        }
                        else
                        {
                            szGateway[0] = '\0';
                        }
                    }
                }
            }
#endif

            // set status flag
            if (getInFlags(szIfname, &iFlags) == 1)
            {
                if ((iFlags & IFF_UP) && iAdslflag)
                {
                    //cathy, for  bug B011
                    pszStatus = (char *)IF_UP;
                }
                else
                {
                    pszStatus = (char *)IF_DOWN;
                    szIpAddr[0]  = '\0';
                    szGateway[0] = '\0';
                }
            }
            else
            {
                pszStatus = (char *)IF_NA;
            }
        }

        if (bShowIndex)
        {
            printf("%-4d%-10s%-8s%-7s%-9s%-16s%-16s%-7s\n", i, szIfname, szVcc, szAal5Encap, 
                    pszMode, szIpAddr, szGateway, pszStatus);
        }
        else
        {
            printf("%-10s%-8s%-7s%-9s%-16s%-16s%-7s\n", szIfname, szVcc, szAal5Encap, 
                    pszMode, szIpAddr, szGateway, pszStatus);
        }
    }

    return uiEntryNum;
}

static int cli_atmVcList()
{
	//int nBytesSent=0;

	unsigned int entryNum, i, k;
	MIB_CE_ATM_VC_T Entry;
	char	vpi[6], vci[6], *qos, pcr[6], scr[6], mbs[6];
	char cdvt[12];
	char *temp;
	int vcList[MAX_VC_NUM], found;

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	printf("Idx\tPVC\tQoS\tPCR\tCDVT\tSCR\tMBS\n");
	MSG_LINE;
	if (!entryNum) {
   		printf("No data!\n\n");
   		return 0;
   	}
	memset(vcList, 0, MAX_VC_NUM*4);

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}
		
		// skip duplicate vc display
		found = 0;
		for (k=0; k<MAX_VC_NUM && vcList[k] != 0; k++) {
			if (vcList[k] == VC_INDEX(Entry.ifIndex)) {
				found = 1;
				break;
			}
		}
		if (found)
			continue;
		else
			vcList[k] = VC_INDEX(Entry.ifIndex);
		
		snprintf(vpi, 6, "%u", Entry.vpi);
		snprintf(vci, 6, "%u", Entry.vci);
		snprintf(pcr, 6, "%u", Entry.pcr);
		snprintf(cdvt, 12, "%u", Entry.cdvt);
		
		if (Entry.qos <= 3)
			qos = (char *)strQos[Entry.qos];
		
		if(Entry.qos > 1) {
			snprintf(scr, 6, "%u", Entry.scr);
			snprintf(mbs, 6, "%u", Entry.mbs);
		} else {
			strcpy(scr, "---");
			strcpy(mbs, "---");
		}
		
		printf("%d\t%s/%s\t%s\t%s\t%s\t%s\t%s\n", i,vpi,vci,qos,pcr,cdvt,scr,mbs);
	}
	
	return entryNum;
}

int getInputNum()
{
	char buf[128];
	int num;
	
	fgets((char *)buf, 127, stdin);
	if ( buf[0]==0) return(0);
	//printf("Enter buf= %s\n", buf);
	if(sscanf( buf, "%d", &num)==0) return(0);
	//CLIDEBUG("Enter num=%d\n", num);
	return( num);
}

int getInputStr( char *value, int maxlen, char *pcIsInputInvalid)
{
	char buf[128];
	int num;
	
	value[0]=0;
	fgets((char *)buf, 127, stdin);
	buf[127] = 0;
	if (buf[strlen(buf)-1]=='\n')
		buf[strlen(buf)-1] = 0;
	
	if (( buf[0]==0) || ( strlen(buf) > maxlen))
	{
		if (NULL != pcIsInputInvalid)
		{
			*pcIsInputInvalid = 1;
		}
		
		printf("Invalid value!\n");
		return(0);
	}
	
	strncpy(value, buf, maxlen);
	//CLIDEBUG("Enter value=%s len=%d\n", value, strlen(value));
	
	if (NULL != pcIsInputInvalid)
	{
		*pcIsInputInvalid = 0;
	}
	
	return( strlen(value));
}

// get typed input.
// return (0) if user enter nothing.
//        (1) if input is valid. 
enum {
	INPUT_TYPE_UINT,
	INPUT_TYPE_STRING,
	INPUT_TYPE_IPADDR,
	INPUT_TYPE_IPMASK,
	INPUT_TYPE_ETHADDR,
	INPUT_TYPE_INT
};
static int getTypedInputDefault(int type, char *prompt, void *rv, void *arg1, void *arg2) {
	while (1) {
	
		printf(prompt);
		strbuf[0] = 0;
		if (INPUT_TYPE_STRING == type) {
			char *pc;
			fgets((char *)rv, (int)arg1, stdin);
			pc = (char *)rv;
			pc[strlen(pc)-1] = 0; // remove the ending LF         
			return 1;
		}      
		fgets(strbuf, sizeof(strbuf) - 1, stdin);
		strbuf[strlen(strbuf)-1] = 0; // remove the ending LF
		//printf("input len %d\n", strlen(strbuf));
		if (strlen(strbuf) == 0)
			return -2;
		
		switch (type) {
		case INPUT_TYPE_IPADDR:
			if (inet_aton(strbuf, rv))
				return 1;
			break;
			
		case INPUT_TYPE_IPMASK:
			if (inet_aton(strbuf, rv))
				return 1;
			break;      
			
		case INPUT_TYPE_UINT:
			do {
				unsigned int num;
				unsigned int min, max;            
				if (1 != sscanf(strbuf, "%u", &num))
					break;
				if (arg1) {
					min = *(unsigned int *)arg1;
					if (num < min)
						break;
				}
				
				if (arg2) {
					max = *(unsigned int *)arg2;               
					if (num > max)
						break;
				}            
				*(unsigned int *)rv = num;
				return 1;
			} while (0);
			break;
			
		default:
			return (-1);
		}
	} // while
	
	return 0;
}
static int getTypedInput(int type, char *prompt, void *rv, void *arg1, void *arg2) {
	while (1) {
	
		printf(prompt);
		strbuf[0] = 0;
		if (INPUT_TYPE_STRING == type) {
			char *pc;
			int len;
			
			len = (int)arg1;
			fgets((char *)rv, len, stdin);
			pc = (char *)rv;
			pc[len-1] = 0;
			if (pc[strlen(pc)-1]=='\n')
				pc[strlen(pc)-1] = 0;
			if (strlen(pc) == 0)
				return 0;
			return 1;
		}      
		fgets(strbuf, sizeof(strbuf) - 1, stdin);
		strbuf[strlen(strbuf)-1] = 0; // remove the ending LF
		//printf("input len %d\n", strlen(strbuf));
		//if (strlen(strbuf) == 0)
		if (strlen(strbuf) == 0  && type != INPUT_TYPE_INT)
			return 0;
		
		switch (type) {
		case INPUT_TYPE_IPADDR:
			if (!isValidIpAddr(strbuf)) {
				printf("Invalid IP address %s.\n", strbuf);
				return 0;
			}
			if (inet_aton(strbuf, rv))
				return 1;
			break;
			
		case INPUT_TYPE_IPMASK:
			if (!isValidNetmask(strbuf, 1)) {
				printf("Invalid subnet mask %s.\n", strbuf);
				return 0;
			}
			if (inet_aton(strbuf, rv))
				return 1;
			break;      
			
		case INPUT_TYPE_UINT:
			do {
				unsigned int num;
				unsigned int min, max;            
				if (1 != sscanf(strbuf, "%u", &num))
					break;                        
				if (arg1) {
					min = *(unsigned int *)arg1;               
					if (num < min)
						break;
				}
				
				if (arg2) {
					max = *(unsigned int *)arg2;               
					if (num > max)
						break;
				}            
				*(unsigned int *)rv = num;
				return 1;
			} while (0);
			break;
			
			case INPUT_TYPE_INT:
				do {
					int num;
					int min, max;
					if (strlen(strbuf) == 0)
						return -1;
					if (1 != sscanf(strbuf, "%d", &num))
						break;
					if (arg1) {
						min = *(int *)arg1;
						if (num < min)
							break;
					}

					if (arg2) {
						max = *(int *)arg2;
						if (num > max)
							break;
					}
					*(int *)rv = num;
					return 1;
				} while (0);
				break;

		default:
			return (-1);
		}
	} // while
	
	return 0;
}

static int getInputIpAddr(char *prompt, struct in_addr *rv) {
	return (getTypedInput(INPUT_TYPE_IPADDR, prompt, (void *)rv, 0, 0));
}

static int getInputIpMask(char *prompt, struct in_addr *rv) {
	return (getTypedInput(INPUT_TYPE_IPMASK, prompt, (void *)rv, 0, 0));
}

static int getInputString(char *prompt, char *rv, int len) {
	return (getTypedInput(INPUT_TYPE_STRING, prompt, (void *)rv, (void *)len, 0));
}

static int getInputUint(char *prompt, unsigned int *rv, unsigned int *min, unsigned int *max) {
	return (getTypedInput(INPUT_TYPE_UINT, prompt, (void *)rv, min, max));
}

static int getInputInt(char *prompt, int *rv, int *min, int *max) {
	return (getTypedInput(INPUT_TYPE_INT, prompt, (void *)rv, min, max));
}

static int getInputOption(unsigned int *rv, unsigned int min0, unsigned int max0) {
	char buf[32];
	int min=min0, max=max0;
	
	sprintf( buf, "Enter the option(%d-%d): ", min, max);
	return (getInputUint( buf, rv, &min, &max));
}

void printWaitStr()
{
	char buf[128];
	
	getInputString("\nPress Enter key to continue...", buf, sizeof(buf)-1);
}


// Jenny, check CLI user/passwd
int auth_cli(const char *name, const char *passwd)
{
	char *xpasswd;
	struct passwd *p;

	p = getpwnam (name);
	if (p == NULL)
		return 1;

	if (p->pw_uid != 0)
		return 1;
		
#if defined(HAVE_GETSPNAM) && defined(HAVE_SHADOW_H)
	if (p->pw_passwd == NULL || strlen (p->pw_passwd) == 1)
	{
		struct  spwd *spw;

		setspent ();
		spw = getspnam (p->pw_name);
		if (spw != NULL)
			p->pw_passwd = spw->sp_pwdp;
		endspent ();
	}
#endif 
	xpasswd = crypt (passwd, p->pw_passwd);
	return  (!xpasswd || strcmp (xpasswd, p->pw_passwd) != 0);
}

/*************************************************************************************************/
int getNameServers(char *buf) {

	FILE *fp;
	char buffer[128], tmpbuf[64];
	int count = 0;
	memset(buf, 0x00, 256);
	if ( (fp = fopen("/var/resolv.conf", "r")) == NULL )
		return -1;

	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		if (sscanf(buffer, "nameserver %s", tmpbuf) != 1)
			continue;

		if (count == 0)
			sprintf(buf, "%s", tmpbuf);
		else {
			sprintf(buffer, ", %s", tmpbuf);
			strcat(buf, buffer);
		}
		count ++;
	}
	fclose(fp);
	return 0;
}

#ifndef RTF_UP
/* Keep this in sync with /usr/src/linux/include/linux/route.h */
#define RTF_UP          0x0001	/* route usable                 */
#define RTF_GATEWAY     0x0002	/* destination is a gateway     */
#define RTF_HOST        0x0004	/* host entry (net otherwise)   */
#define RTF_REINSTATE   0x0008	/* reinstate route after tmout  */
#define RTF_DYNAMIC     0x0010	/* created dyn. (by redirect)   */
#define RTF_MODIFIED    0x0020	/* modified dyn. (by redirect)  */
#define RTF_MTU         0x0040	/* specific MTU for this route  */
#ifndef RTF_MSS
#define RTF_MSS         RTF_MTU	/* Compatibility :-(            */
#endif
#define RTF_WINDOW      0x0080	/* per route window clamping    */
#define RTF_IRTT        0x0100	/* Initial round trip time      */
#define RTF_REJECT      0x0200	/* Reject route                 */
#endif

// Jenny, get default gateway information
int getDGW(char *buf)
{
	char buff[256];
	int flgs;
	struct in_addr gw, dest, mask, inAddr;
	char ifname[16], dgw[16];
	FILE *fp;

	memset(buf, 0x00, 256);
	if (!(fp=fopen("/proc/net/route", "r"))) {
		printf("Error: cannot open /proc/net/route - continuing...\n");
		return -1;
	}

	fgets(buff, sizeof(buff), fp);
	while (fgets(buff, sizeof(buff), fp) != NULL) {
		if (sscanf(buff, "%s%x%x%x%*d%*d%*d%x", ifname, &dest, &gw, &flgs, &mask) != 5) {
			printf("Unsuported kernel route format\n");
			fclose(fp);
			return -1;
		}
		if(flgs & RTF_UP) {
			if (getInAddr(ifname, IP_ADDR, (void *)&inAddr) == 1) {
				if (inAddr.s_addr == 0x40404040) {
					sprintf(buf, "");
					fclose(fp);
					return 0;
				}
			}
			// default gateway
			if (dest.s_addr == 0 && mask.s_addr == 0) {
				if (gw.s_addr != 0) {
					strncpy(dgw,  inet_ntoa(gw), 16);
					sprintf(buf, "%s", dgw);
					fclose(fp);
					return 0;
				}
				else {
					sprintf(buf, "%s", ifname);
					fclose(fp);
					return 0;
				}
			}
		}
	}
	fclose(fp);
	return 0;
}

void showStatus()
{
	CLEAR;
	printf("\n");
	MSG_LINE;
	//printf("-------------------------------------------------------------------------\n");
	printf("                        Acorp ADSL Router Status                           \n");
	MSG_LINE;
	//printf("-------------------------------------------------------------------------\n");
	printf("This page shows the current status and some basic settings of the device.\n");
	MSG_LINE;
	//printf("-------------------------------------------------------------------------\n");
	printf("System\n");
	printf("Product Name\t\t: %s\n", getMibInfo(MIB_SNMP_SYS_NAME));
	getSYS2Str(SYS_UPTIME, strbuf);
	printf("Uptime\t\t\t: %s\n", strbuf);
	getSYS2Str(SYS_DATE, strbuf);
	printf("Date/Time\t\t: %s\n", strbuf);
	getSYS2Str(SYS_FWVERSION, strbuf);
	printf("Firmware Version\t: %s\n", strbuf);
	getAdslInfo(ADSL_GET_VERSION, strbuf, 256);
	printf("DSP Version\t\t: %s\n", strbuf);
#ifdef WLAN_SUPPORT
	getWlVersion(WLANIF, strbuf);
	printf("Wireless Version\t: %s\n", strbuf);
#endif
	getNameServers(strbuf);
	printf("Name Servers\t\t: %s\n", strbuf);
	getDGW(strbuf);
	printf("Default Gateway\t\t: %s\n", strbuf);

	printf("\nDSL\n");
	
	printf("Operational Status\t: ");
	getAdslInfo(ADSL_GET_MODE, strbuf, 256);
	if (strbuf[0])
		printf("%s, ", strbuf);
	getAdslInfo(ADSL_GET_STATE, strbuf, 256);
	printf("%s\n", strbuf);
	
	getAdslInfo(ADSL_GET_RATE_US, strbuf, 256);
	printf("Upstream Speed\t\t: %s kbps\n", strbuf);
	
	
	getAdslInfo(ADSL_GET_RATE_DS, strbuf, 256);
	printf("Downstream Speed\t: %s kbps", strbuf);

	getAdslInfo(ADSL_GET_LATENCY, strbuf, 256);
	printf("\nChannel mode\t\t: %s\n", strbuf);	
	printf("\nLAN Configuration\n");
	printf("IP Address\t\t: %s\n", getMibInfo(MIB_ADSL_LAN_IP));
	printf("Subnet Mask\t\t: %s\n", getMibInfo(MIB_ADSL_LAN_SUBNET));
	getSYS2Str(SYS_LAN_DHCP, strbuf);
	printf("DHCP Server\t\t: %s\n", strbuf);
	printf("MAC Address\t\t: %s\n", getMibInfo(MIB_ELAN_MAC_ADDR));
	printf("\nWAN Configuration\n");
	wanConnList(0);
	printWaitStr();
}


/*************************************************************************************************/
void setLANInterface()
{
	struct in_addr inIp, inMask;
	char mode;
	int snum;
	unsigned int snoop, num, min, max;
#ifdef SECONDARY_IP
	unsigned char vChar;
	unsigned int ip2en, dhcp_pool;
	struct in_addr inIp2, inMask2;
#endif
	
	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                           (2) LAN Interface Setup                           \n");
		printf("-------------------------------------------------------------------------\n");
		printf("This page is used to configure the LAN interface of your ADSL Router.    \n");
		printf("Here you may change the setting for IP address, subnet mask, etc..      \n");
		printf("-------------------------------------------------------------------------\n");
		printf("Interface Name\t: br0\n");
		printf("IP Address\t: %s\n",getMibInfo(MIB_ADSL_LAN_IP));
		printf("Subnet Mask\t: %s\n", getMibInfo(MIB_ADSL_LAN_SUBNET));
#ifdef SECONDARY_IP
		getSYS2Str(SYS_LAN_IP2, strbuf);
		printf("Secondary IP\t: %s\n", strbuf);
		mib_get(MIB_ADSL_LAN_ENABLE_IP2, (void *)&vChar);
		if (vChar != 0) {
			printf("IP Address\t: %s\n",getMibInfo(MIB_ADSL_LAN_IP2));
			printf("Subnet Mask\t: %s\n", getMibInfo(MIB_ADSL_LAN_SUBNET2));
#ifndef DHCPS_POOL_COMPLETE_IP
			getSYS2Str(SYS_LAN_DHCP_POOLUSE, strbuf);
			printf("DHCP pool\t: %s\n", strbuf);
#endif
		}
#endif
#ifdef CONFIG_EXT_SWITCH
		getSYS2Str(SYS_IGMP_SNOOPING, strbuf);
		printf("IGMP Snooping\t: %s\n\n", strbuf);
#endif
		printf("\n(1) Set                          (2) Quit\n");
		if (!getInputOption( &snum, 1, 2))
			continue;
		switch( snum)
		{
		case 1://(1) Set
			if (!check_access(SECURITY_SUPERUSER))
				break;
			printf("Old IP address: %s\n", getMibInfo(MIB_ADSL_LAN_IP));         
			if (0 == getInputIpAddr("New IP address:", &inIp))
				continue;
			
			printf("Old subnet mask address: %s\n",  getMibInfo(MIB_ADSL_LAN_SUBNET));
			if (0 == getInputIpMask("New subnet mask address:", &inMask))
				continue;         

#ifdef SECONDARY_IP
			min=1;max=2;      
			if (0 == getInputUint("Secondary IP (1)Disable (2)Enable :", &num, &min, &max))
				continue;

			ip2en = (unsigned char)(num - 1);
			if (ip2en == 1) {
				printf("Old IP address: %s\n", getMibInfo(MIB_ADSL_LAN_IP2));
				if (0 == getInputIpAddr("New IP address:", &inIp2))
					continue;

				printf("Old subnet mask address: %s\n", getMibInfo(MIB_ADSL_LAN_SUBNET2));
				if (0 == getInputIpMask("New subnet mask address:", &inMask2))
					continue;         

#ifndef DHCPS_POOL_COMPLETE_IP
				min=1;max=2;      
				if (0 == getInputUint("DHCP pool (1)Primary LAN (2)Secondary LAN :", &num, &min, &max))
					continue;

				dhcp_pool = (unsigned char)(num - 1);
#endif
			}
#endif

#ifdef CONFIG_EXT_SWITCH
			min=1;max=2;      
			if (0 == getInputUint("IGMP Snooping (1)Disable (2)Enable :", &num, &min, &max))
				continue;
			
			snoop = (unsigned char)(num - 1);
			// bitmap for virtual lan port function
			// Port Mapping: bit-0
			// QoS : bit-1
			// IGMP snooping: bit-2
			mib_get(MIB_MPMODE, (void *)&mode);
			if (snoop) {
				mode |= 0x04;
				// take effect immediately
				__dev_setupIGMPSnoop(1);
			}
			else {
				mode &= 0xfb;
				__dev_setupIGMPSnoop(0);
				if (mode==0)
					__dev_setupVirtualPort(0); // disable virtual port
			}
#endif
			
			if ( !mib_set( MIB_ADSL_LAN_IP, (void *)&inIp)) {
				printf("Set IP-address error!");			   
			}
			
			if ( !mib_set(MIB_ADSL_LAN_SUBNET, (void *)&inMask)) {
				printf("Set subnet-mask error!");
			}
			
#ifdef SECONDARY_IP
			if (ip2en == 1)
				vChar = 1;
			else
				vChar = 0;
			if (!mib_set(MIB_ADSL_LAN_ENABLE_IP2, (void *)&vChar))
				printf("Set secondary IP error!");

			if (ip2en == 1) {
				if (!mib_set( MIB_ADSL_LAN_IP2, (void *)&inIp2))
					printf("%s\n", strSetIPerror);			   

				if (!mib_set(MIB_ADSL_LAN_SUBNET2, (void *)&inMask2))
					printf("%s\n", strSetMaskerror);

#ifndef DHCPS_POOL_COMPLETE_IP
				if (dhcp_pool == 1)
					vChar = 1;
				else
					vChar = 0;
				if (!mib_set(MIB_ADSL_LAN_DHCP_POOLUSE, (void *)&vChar))
					printf("Set DHCP pool error!");
#endif
			}
#endif

#ifdef CONFIG_EXT_SWITCH
			if (!mib_set(MIB_MPMODE, (void *)&mode))
				printf("Set IGMP Snooping error!");
#endif
#if defined(APPLY_CHANGE)
			// Take effect in real time
			restart_lanip();
#endif
			break;
			
		case 2://(2) Quit
			return;
		}//end switch, WAN Interface Menu
	}//end while, WAN Interface Menu
}

int login_flag=0; // Jenny, login_flag=1 from console
int console_flag=0; // cli forked by console
#ifdef WLAN_SUPPORT
/*************************************************************************************************/
// Jenny
void showClient()
{
	char *buff;
	WLAN_STA_INFO_Tp pInfo;
	int i;
	char tmpbuf[20];
	
	if ( (buff = calloc(1, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1))) == 0 ) {
		printf("Allocate buffer failed!\n");
		return;
	}

	if ( getWlStaInfo((char *)WLANIF,  (WLAN_STA_INFO_Tp)buff ) < 0 ) {
		printf("Read wlan sta info failed!\n");
		free(buff);
		return;
	}

	printf("\n");
	printf("Active Wireless Client Table\n");
	MSG_LINE;
	printf("This table shows the MAC address, transmission, reception packet \n");
	printf("counters and encrypted status for each associated wireless client.\n");
	MSG_LINE;
	printf("MAC Address        Tx Pkt    Rx Pkt    TxRate  PowerSaving  Expired Time\n");
	MSG_LINE;
	
	for (i=1; i<=MAX_STA_NUM; i++) {
		pInfo = (WLAN_STA_INFO_Tp)&buff[i*sizeof(WLAN_STA_INFO_T)];
		if (pInfo->aid && (pInfo->flag & STA_INFO_FLAG_ASOC)) {
			snprintf(tmpbuf, 20, "%02x:%02x:%02x:%02x:%02x:%02x", 
				pInfo->addr[0], pInfo->addr[1], pInfo->addr[2],	
				pInfo->addr[3], pInfo->addr[4], pInfo->addr[5]);
			printf("%-19s%-10d%-10d%d%-6s%-13s%d\n",
				tmpbuf, pInfo->tx_packets, pInfo->rx_packets, pInfo->txOperaRates/2, ((pInfo->txOperaRates%2) ? ".5" : "" ),
				( (pInfo->flag & STA_INFO_FLAG_ASLEEP) ? "Yes" : "No"), pInfo->expired_time/100);
		}
	}
	free(buff);
	printWaitStr();
}

void setWlanBasic()
{
	unsigned int choice,num,min,max;
	unsigned char wlan;
	unsigned char ssid[32];
	int flags;
	
	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("                           Wireless Basic Settings                       \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("This page is used to configure the parameters for wireless LAN clients   \n");
		printf("which may connect to your Access Point. Here you may change wireless     \n");
		printf("encryption settings as well as wireless network parameters.              \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		getSYS2Str(SYS_WLAN, strbuf);
		printf("Wireless Interface: %s\n", strbuf);
		getSYS2Str(SYS_WLAN_BAND, strbuf);
		printf("Band: %s\n", strbuf);
		getSYS2Str(SYS_WLAN_MODE, strbuf);
		printf("Mode: %s\n", strbuf);
		printf("SSID: %s\n", getMibInfo(MIB_WLAN_SSID));
		printf("Channel Number: %s\n", getMibInfo(MIB_WLAN_CHAN_NUM));
		getSYS2Str(SYS_TX_POWER, strbuf);	 // Jenny
		printf("Radio Power (mW): %s\n\n", strbuf);
		printf("(1) Set                          (2) Show Active Clients\n");
		printf("(3) Quit\n");
		choice = -1;
		if (console_flag)
			getInputOption(&choice, 0, 3); // Jenny, hidden wireless enable/disable selection
		else
			getInputOption(&choice, 1, 3); // Jenny, unable to enable/disable wireless from remote login
		switch(choice)
		{
		case 0://(0) Jenny, Wireless enable/disable selection
			min=1;max=2;      
			if (0 == getInputUint("Wireless Interface (1)Enable (2)Disable :", &num, &min, &max))
				continue;
			wlan = (unsigned char)(num - 1);
			mib_set(MIB_WLAN_DISABLED, (void *)&wlan);
			break;
		case 1: //(1) Set
			min=1;max=2;      
			if (0 == getInputUint("Wireless Interface (1)Enable (2)Disable :", &num, &min, &max))
				continue;
			wlan = (unsigned char)(num - 1);
			//mib_set(MIB_WLAN_DISABLED, (void *)&wlan);
			//mib_get(MIB_WLAN_DISABLED, (void *)&wlan); // Jenny, get current wireless interface status
			if (wlan == 0) { // enable
				unsigned char band, mode, chno, txPower;
				
				min=1;max=3;
				if (0 == getInputUint("Band (1)2.4 GHz(B) (2)2.4 GHz(G) (3)2.4 GHz(B+G): ", &num, &min, &max))
					continue;
				band = (unsigned char)num;
				
#if (defined(WLAN_WDS) && defined(WLAN_CLIENT))
				min=1;max=3;	// Jenny, WLAN mode
				if (0 == getInputUint("Mode (1)AP (2)Client (3)WDS: ", &num, &min, &max))
					continue;
				mode = (unsigned char)(num-1);
				if (mode == CLIENT_MODE) {
					ENCRYPT_T encrypt;
					char_t vChar;
					mib_get( MIB_WLAN_ENCRYPT,  (void *)&vChar);
					encrypt = (ENCRYPT_T)vChar;
					if (encrypt == ENCRYPT_WPA) {
						mib_get( MIB_WLAN_WPA_AUTH, (void *)&vChar);
						if (vChar & 1) { // radius
							printf("You cannot set client mode with WPA-RADIUS!\nPlease change the encryption method in security menu first.\n");
							continue;
						}
					}
					else if (encrypt == ENCRYPT_WEP) {
						mib_get( MIB_WLAN_ENABLE_1X, (void *)&vChar);
						if (vChar & 1) {
							printf("You cannot set client mode with WEP-802.1x!\nPlease change the encryption method in security menu first.\n");
							continue;
						}
					}
				}
#else
#ifdef WLAN_WDS
				min=1;max=2;
				if (0 == getInputUint("Mode (1)AP (2)AP+WDS: ", &num, &min, &max))
					continue;
				if (num == 1)
					mode = (unsigned char)AP_MODE;
				else if (num == 2)
					mode = (unsigned char)AP_WDS_MODE;
				else
					continue;
#endif
#ifdef WLAN_CLIENT
				min=1;max=2;
				if (0 == getInputUint("Mode (1)AP (2)Client: ", &num, &min, &max))
					continue;
				mode = (unsigned char)(num-1);
				if (mode == CLIENT_MODE) {
					ENCRYPT_T encrypt;
					char_t vChar;
					mib_get( MIB_WLAN_ENCRYPT,  (void *)&vChar);
					encrypt = (ENCRYPT_T)vChar;
					if (encrypt == ENCRYPT_WPA) {
						mib_get( MIB_WLAN_WPA_AUTH, (void *)&vChar);
						if (vChar & 1) { // radius
							printf("You cannot set client mode with WPA-RADIUS!\nPlease change the encryption method in security menu first.\n");
							continue;
						}
					}
					else if (encrypt == ENCRYPT_WEP) {
						mib_get( MIB_WLAN_ENABLE_1X, (void *)&vChar);
						if (vChar & 1) {
							printf("You cannot set client mode with WEP-802.1x!\nPlease change the encryption method in security menu first.\n");
							continue;
						}
					}
				}
#endif
#endif	// of WLAN_WDS && WLAN_CLIENT

				if (0 == getInputString("SSID: ", ssid, sizeof(ssid)))
					continue;
				
				min=0;max=11;
				if (0 == getInputUint("Channel Number (0 ~ 11): ", &num, &min, &max))
					continue;
				chno = (unsigned char)num;
//modified by xl_yue
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
				min=1;max=5;
				if (0 == getInputUint("Radio Power (1)100%% (2)80%% (3)50%% (4)25%% (5)10%% : ", &num, &min, &max))
					continue;
#else				
				min=1;max=3;
				if (0 == getInputUint("Radio Power (1)25 %% (2)50 %% (3)100 %%: ", &num, &min, &max))
					continue;
#endif
				txPower = (unsigned char)(num-1); // Jenny
				
				mib_set(MIB_WLAN_BAND, (void *)&band);
				mib_set(MIB_WLAN_MODE, (void *)&mode);	// Jenny, WLAN mode
				mib_set(MIB_WLAN_SSID, (void *)ssid);
				mib_set(MIB_WLAN_CHAN_NUM, (void *)&chno);
				// Jenny: txPower
				mib_set(MIB_TX_POWER, (void *)&txPower);
//#ifdef APPLY_CHANGE
				//config_WLAN(ACT_RESTART);
//#endif
			}
			update_wlan_disable(wlan);
			if (getInFlags("wlan0", &flags) == 1) {
				if (wlan)
					flags &= ~IFF_UP;
				else
					flags |= IFF_UP;
				setInFlags("wlan0", flags);
			}
			mib_set(MIB_WLAN_DISABLED, (void *)&wlan);
			config_WLAN(ACT_RESTART);
			break;
		case 2://(2) Show Active Clients
			showClient();
			break;
		case 3://(3) Quit
			return;
		}//end switch, Wireless Basic Settings
	}//end while, Wireless Basic Settings
}

void setWlanAdv()
{
	unsigned int choice,num,min,max;
	unsigned char auth, preamble, hiddenSSID, blockRelay, block_eth2wir;
	unsigned short frag, rts, beacon;
	const char rate_mask[] = {3, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2};
	const char *rate_name[] = {"Auto", "1M", "2M", "5.5M", "11M", "6M", "9M", "12M", "18M", "24M", "36M", "48M", "54M"};
	unsigned char band, txRate, autort, mask, tmpstr[10], reqstr[128];;
	unsigned short txrate, uShort;
	int i, idx;
	
	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("                           Wireless Advanced Settings                    \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("These settings are only for more technically advanced users who have a   \n");
		printf("sufficient knowledge about wireless LAN. These settings should not be    \n");
		printf("changed unless you know what effect the changes will have on your        \n");
		printf("Access Point.\n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		getSYS2Str(SYS_WLAN_AUTH, strbuf);
		printf("Authentication Type: %s\n", strbuf);
		printf("Fragment Threshold: %s\n", getMibInfo(MIB_WLAN_FRAG_THRESHOLD));
		printf("RTS Threshold: %s\n", getMibInfo(MIB_WLAN_RTS_THRESHOLD));
		printf("Beacon Interval: %s\n", getMibInfo(MIB_WLAN_BEACON_INTERVAL));
		getSYS2Str(SYS_WLAN_TXRATE, strbuf);	// Jenny, Data Rate
		printf("Data Rate: %s\n", strbuf);
		getSYS2Str(SYS_WLAN_PREAMBLE, strbuf);
		printf("Preamble Type: %s\n", strbuf);
		getSYS2Str(SYS_WLAN_BCASTSSID, strbuf);
		printf("Broadcast SSID: %s\n", strbuf);
		getSYS2Str(SYS_WLAN_BLOCKRELAY, strbuf);	// Jenny, Relay Blocking
		printf("Relay Blocking: %s\n", strbuf);
		getSYS2Str(SYS_WLAN_BLOCK_ETH2WIR, strbuf);
		printf("Ethernet to Wireless Blocking: %s\n\n", strbuf);

		printf("(1) Set                          (2) Quit\n");
		if (!getInputOption(&choice, 1, 2))
			continue;
		switch(choice)
		{
		case 1://(1) Set
			min=1;max=3;
			if (0 == getInputUint("Authentication Type (1)Open System (2)Shared Key (3)Auto: ", &num, &min, &max))
				continue;
			auth = (unsigned char)num-1;
				
			min=256;max=2346;
			if (0 == getInputUint("Fragment Threshold (256 ~ 2346): ", &num, &min, &max))
				continue;
			frag = (unsigned short)num;
			
			min=0;max=2347;
			if (0 == getInputUint("RTS Threshold (0 ~ 2347): ", &num, &min, &max))
				continue;
			rts = (unsigned short)num;
			
			min=20;max=1024;
			if (0 == getInputUint("Beacon Interval (20 ~ 1024 ms): ", &num, &min, &max))
				continue;
			beacon = (unsigned short)num;
			
			// Jenny, Data Rate
			mib_get( MIB_WLAN_BAND, (void *)&band);
			mib_get( MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&autort);
			mask=0;
			if (autort)
				txrate=0;
			if (band & 1)
				mask |= 1;
			if ((band & 2) || (band & 4))
				mask |= 2;
			strcpy(reqstr, "Data Rate");
			for (idx=0, i=0; i<=12; i++) {
				if (rate_mask[i] & mask) {
					idx++;
					if (i == 0)
						txrate = 0;
					else
						txrate = 1 << (i-1);
					sprintf(tmpstr, "(%d)%s ", idx, rate_name[i]);
					strcat(reqstr, tmpstr);
				}
			}
			strcat(reqstr, ": ");
			min=1;max=idx;
			if (0 == getInputUint(reqstr, &num, &min, &max))
				continue;
			txRate = (unsigned char)(num-1);

			// set tx rate
			if ( txRate == 0 )	// auto
				autort = 1;
			else  {
				autort = 0;
#ifdef WLAN_8185AG
				uShort = txRate;
				uShort = 1 << (uShort-1);
#else
				if (txRate == 1 )  // 11M
					uShort = TX_RATE_11M;
				else if (txRate == 2 ) // 5.5M
					uShort = TX_RATE_5M;
				else if (txRate == 3 ) // 2M
					uShort = TX_RATE_2M;
				else if (txRate == 4 ) // 1M
					uShort = TX_RATE_1M;
				else {
					printf("invalud value of tx rate!");
					continue;
				}
#endif
			}

//modified by xl_yue
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
			min=1;max=3;
			if (0 == getInputUint("Preamble Type (1)Long Preamble (2)Short Preamble (3)Auto Preamble: ", &num, &min, &max))
				continue;
#else			
			min=1;max=2;
			if (0 == getInputUint("Preamble Type (1)Long Preamble (2)Short Preamble: ", &num, &min, &max))
				continue;
#endif
			preamble = (unsigned char)num - 1;
			
			min=1;max=2;
			if (0 == getInputUint("Broadcast SSID (1)Enable (2)Disable: ", &num, &min, &max))
				continue;
			hiddenSSID = (unsigned char)num - 1;
			
			min=1;max=2;
			if (0 == getInputUint("Relay Blocking (1)Enable (2)Disable: ", &num, &min, &max))
				continue;
			blockRelay = ((unsigned char)num - 1)^1;	// Jenny
			
			min=1;max=2;
			if (0 == getInputUint("Ethernet to Wireless Blocking (1)Enable (2)Disable: ", &num, &min, &max))
				continue;
			block_eth2wir = ((unsigned char)num - 1)^1;

			mib_set(MIB_WLAN_AUTH_TYPE, (void *)&auth);
			mib_set(MIB_WLAN_FRAG_THRESHOLD, (void *)&frag);
			mib_set(MIB_WLAN_RTS_THRESHOLD, (void *)&rts);
			mib_set(MIB_WLAN_BEACON_INTERVAL, (void *)&beacon);
			// Jenny: Data Rate
			mib_set(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&autort);
#ifdef WLAN_8185AG
			mib_set(MIB_WLAN_FIX_RATE, (void *)&uShort);
#else
			mib_set(MIB_WLAN_SUPPORTED_RATE, (void *)&uShort);
			mib_set(MIB_WLAN_BASIC_RATE, (void *)&uShort);
#endif
			mib_set(MIB_WLAN_PREAMBLE_TYPE, (void *)&preamble);
			mib_set(MIB_WLAN_HIDDEN_SSID, (void *)&hiddenSSID);
			mib_set(MIB_WLAN_BLOCK_RELAY, (void *)&blockRelay);
			mib_set(MIB_WLAN_BLOCK_ETH2WIR, (void *)&block_eth2wir);
//#ifdef APPLY_CHANGE
			config_WLAN(ACT_RESTART);
//#endif
				
			break;
		case 2://(2) Quit
			return;
		}//end switch, Wireless Advanced Settings
	}//end while, Wireless Advanced Settings
}

void setWlanSecurity()
{
	unsigned int choice, num, min, max, len;
	unsigned char keylen, keyfmt, defkey, encrypt, pskfmt, vChar, use1x, wpaCipher, wpaAuth;
	unsigned char key1[32], key2[32], key3[32], key4[32], tmpBuf[64];
	int enableRS = 0, getPSK = 0; 
	unsigned short rsPort;
	char psk[70];
	
	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("                           Wireless Security Setup                       \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("This page allows you setup the wireless security. Turn on WEP or WPA by  \n");
		printf("using Encryption Keys could prevent any unauthorized access to your      \n");
		printf("wireless network.\n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		getSYS2Str(SYS_WLAN_ENCRYPT, strbuf);
		printf("Encryption: %s\n", strbuf);
		mib_get(MIB_WLAN_ENABLE_1X, (void *)&use1x);
		printf("Use 802.1x Authentication: %s\n", use1x? STR_ENABLE: STR_DISABLE);
		printf("WEP Key:\n");
		getSYS2Str(SYS_WLAN_WEP_KEYLEN, strbuf);
		printf("  Key Length: %s\n", strbuf);
		getSYS2Str(SYS_WLAN_WEP_KEYFMT, strbuf);
		printf("  Key Format: %s\n", strbuf);
		mib_get( MIB_WLAN_WEP_DEFAULT_KEY, (void *)&defkey);
		printf("  Default Tx Key: Key%d\n", defkey+1);
		getSYS2Str(SYS_WLAN_WPA_MODE, strbuf);
		printf("WPA Authentication Mode: %s\n", strbuf);
		getSYS2Str(SYS_WLAN_PSKFMT, strbuf);
		printf("Pre-Shared Key Format: %s\n", strbuf);
		getSYS2Str(SYS_WLAN_PSKVAL, strbuf);
		printf("Pre-Shared Key: %s\n", strbuf);
		printf("Authentication RADIUS Server:\n");
		mib_get(MIB_WLAN_RS_PORT, (void *)&rsPort);
		printf("  Port: %d\n", rsPort);
		mib_get(MIB_WLAN_RS_IP, (void *)tmpBuf);
		printf("  IP address: %d.%d.%d.%d\n", tmpBuf[0], tmpBuf[1], tmpBuf[2], tmpBuf[3]);
		getSYS2Str(SYS_WLAN_RSPASSWD, strbuf);
		printf("  Password: %s\n\n", strbuf);
		printf("(1) Set                          (2) Quit\n");
		
		if (!getInputOption(&choice, 1, 2))
			continue;
		switch(choice)
		{
		case 1://(1) Set
#ifdef ENABLE_WPAAES_WPA2TKIP
			min=0;max=6;
			if (0 == getInputUint("Encryption (0)None (1)WEP (2)WPA(TKIP) (3)WPA(AES) (4)WPA2(AES) (5)WPA2(TKIP) (6)WPA2 Mixed: ", &num, &min, &max))
				continue;
#else
			min=0;max=4;
			if (0 == getInputUint("Encryption (0)None (1)WEP (2)WPA (TKIP) (3)WPA2 (AES) (4)WPA2 Mixed: ", &num, &min, &max))
				continue;
#endif
			encrypt = (unsigned char)num;

			if (encrypt == ENCRYPT_DISABLED) {
#ifdef WLAN_1x
				if (0 == getInputUint("802.1x Authentication (1)Disable (2)Enable: ", &num, &min, &max))
					continue;
				use1x = (unsigned char)(num-1);
				if (use1x == 1)
					enableRS = 1;
#endif
			}
			else if (encrypt == ENCRYPT_WEP) {
#ifdef WLAN_1x
				if (0 == getInputUint("802.1x Authentication (1)Disable (2)Enable: ", &num, &min, &max))
					continue;
				use1x = (unsigned char)(num-1);
				if (use1x == 1)
					enableRS = 1;
#endif

				// set WEP key
				min=1;max=2;
				if (0 == getInputUint("Key Length (1)64-bit (2)128-bit: ", &num, &min, &max))
					continue;
				keylen = (unsigned char)num;

				min=1;max=2;
				if (keylen == WEP64) {
					if (0 == getInputUint("Key Format (1)ASCII (5 chars) (2)Hex (10 chars): ", &num, &min, &max))
						continue;
					keyfmt = (unsigned char)num - 1;
					if (keyfmt == 0)
						len = WEP64_KEY_LEN;
					else
						len = WEP64_KEY_LEN*2;
				}
				else {
					if (0 == getInputUint("Key Format (1)ASCII (13 chars) (2)Hex (26 chars): ", &num, &min, &max))
						continue;
					keyfmt = (unsigned char)num - 1;
					if (keyfmt == 0)
						len = WEP128_KEY_LEN;
					else
						len = WEP128_KEY_LEN*2;
				}

				min=1;max=4;
				if (0 == getInputUint("Default Tx Key (1)Key1 (2)Key2 (3)Key3 (4)Key4: ", &num, &min, &max))
					continue;
				defkey = (unsigned char)num - 1;

				key1[0] = key2[0] = key3[0] = key4[0] = 0;

				// Key1
				if (0 == getInputString("Key1: ", tmpBuf, sizeof(tmpBuf)))
					continue;
				if (strlen(tmpBuf) == len) {
					if (keyfmt == 0) // ASCII
						strcpy(key1, tmpBuf);
					else {// Hex
						if (!string_to_hex(tmpBuf, key1, len))
							continue;
					}
				}
				else if (strlen(tmpBuf) != 0)
					continue;

				// Key2
				if (0 == getInputString("Key2: ", tmpBuf, sizeof(tmpBuf)))
					continue;
				if (strlen(tmpBuf) == len) {
					if (keyfmt == 0) // ASCII
						strcpy(key2, tmpBuf);
					else {// Hex
						if (!string_to_hex(tmpBuf, key2, len))
							continue;
					}
				}
				else if (strlen(tmpBuf) != 0)
					continue;

				// Key3
				if (0 == getInputString("Key3: ", tmpBuf, sizeof(tmpBuf)))
					continue;
				if (strlen(tmpBuf) == len) {
					if (keyfmt == 0) // ASCII
						strcpy(key3, tmpBuf);
					else {// Hex
						if (!string_to_hex(tmpBuf, key3, len))
							continue;
					}
				}
				else if (strlen(tmpBuf) != 0)
					continue;

				// Key4
				if (0 == getInputString("Key4: ", tmpBuf, sizeof(tmpBuf)))
					continue;
				if (strlen(tmpBuf) == len) {
					if (keyfmt == 0) // ASCII
						strcpy(key4, tmpBuf);
					else {// Hex
						if (!string_to_hex(tmpBuf, key4, len))
							continue;
					}
				}
				else if (strlen(tmpBuf) != 0)
					continue;

				/*mib_set( MIB_WLAN_WEP, (void *)&keylen);
				mib_set( MIB_WLAN_WEP_KEY_TYPE, (void *)&keyfmt);
				mib_set( MIB_WLAN_WEP_DEFAULT_KEY, (void *)&defkey);
				if (keylen == WEP64) {
					if (key1[0] != 0)
						mib_set(MIB_WLAN_WEP64_KEY1, (void *)key1);
					if (key2[0] != 0)
						mib_set(MIB_WLAN_WEP64_KEY2, (void *)key2);
					if (key3[0] != 0)
						mib_set(MIB_WLAN_WEP64_KEY3, (void *)key3);
					if (key4[0] != 0)
						mib_set(MIB_WLAN_WEP64_KEY4, (void *)key4);
				}
				else {
					if (key1[0] != 0)
						mib_set(MIB_WLAN_WEP128_KEY1, (void *)key1);
					if (key2[0] != 0)
						mib_set(MIB_WLAN_WEP128_KEY2, (void *)key2);
					if (key3[0] != 0)
						mib_set(MIB_WLAN_WEP128_KEY3, (void *)key3);
					if (key4[0] != 0)
						mib_set(MIB_WLAN_WEP128_KEY4, (void *)key4);
				}*/
			}
			else if (encrypt >= ENCRYPT_WPA) {
#ifdef ENABLE_WPAAES_WPA2TKIP
				if (encrypt == 2 || encrypt == 3){
					if(encrypt == 2)
						wpaCipher = WPA_CIPHER_TKIP;
					else
						wpaCipher = WPA_CIPHER_AES;
					//mib_set( MIB_WLAN_WPA_CIPHER_SUITE, (void *)&vChar);
					encrypt = ENCRYPT_WPA;
				}else if (encrypt == 4 || encrypt == 5){
					if(encrypt == 4)
						wpaCipher = WPA_CIPHER_AES;
					else
						wpaCipher = WPA_CIPHER_TKIP;
					//mib_set( MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&vChar);
					encrypt = ENCRYPT_WPA2;
				}
#else
				if (encrypt == 3)
					encrypt = 4;
				else if (encrypt == 4)
					encrypt = 6;
#endif
				min=1; max=2;
				if (0 == getInputUint("WPA Authentication Mode: (1)Enterprise (RADIUS) (2)Personal (Pre-Shared Key): ", &num, &min, &max))
					continue;
				wpaAuth = (unsigned char)num;
				if (wpaAuth == WPA_AUTH_AUTO)
					enableRS = 1;
				else if (wpaAuth == WPA_AUTH_PSK)
					getPSK = 1;
				//mib_set(MIB_WLAN_WPA_AUTH, (void *)&vChar);

				if (getPSK == 1) {
					// set pre-shared key
					min=1;max=2;
					if (0 == getInputUint("Pre-Shared Key Format (1)Passphrase (2)Hex (64 chars): ", &num, &min, &max))
						continue;
					pskfmt = (unsigned char)num - 1;

					if (pskfmt == 0) {// Passphrase
						if (0 == getInputString("Pre-Shared Key (8 ~ 63 chars): ", psk, 65))
							continue;
						len = strlen(psk);
						if (len < 8 || len > 63)
							continue;
					}
					else { // Hex
						if (0 == getInputString("Pre-Shared Key (64 chars): ", psk, 66))
							continue;
						len = strlen(psk);
						if (len != 64 || !string_to_hex(psk, tmpBuf, 64))
							continue;
					}

					//mib_set(MIB_WLAN_WPA_PSK_FORMAT, (void *)&pskfmt);
					//mib_set(MIB_WLAN_WPA_PSK, (void *)strbuf);
				}
			}
			if (enableRS == 1) {
				int intVal;
				struct in_addr inIp;
				printf("Authentication RADIUS Server:\n");
				min = 1; max = 65535;
				if (0 == getInputUint("Port: ", &intVal, &min, &max))
					continue;
				rsPort = (unsigned short)intVal;
				if (0 == getInputIpAddr("IP Address: ", &inIp))
					continue;
				if (0 == getInputString("Password: ", strbuf, MAX_PSK_LEN - 1))
					continue;
				if (strbuf[0] && strlen(strbuf) > MAX_PSK_LEN) {
					printf(strRSPwdTooLong);
					printWaitStr();
					continue;
				}
				mib_set(MIB_WLAN_RS_PORT, (void *)&rsPort);
				mib_set(MIB_WLAN_RS_IP, (void *)&inIp);
				mib_set(MIB_WLAN_RS_PASSWORD, (void *)strbuf);
			}
			if (encrypt == ENCRYPT_WEP) {
				mib_set(MIB_WLAN_WEP, (void *)&keylen);
				mib_set(MIB_WLAN_WEP_KEY_TYPE, (void *)&keyfmt);
				mib_set(MIB_WLAN_WEP_DEFAULT_KEY, (void *)&defkey);
				if (keylen == WEP64) {
					if (key1[0] != 0)
						mib_set(MIB_WLAN_WEP64_KEY1, (void *)key1);
					if (key2[0] != 0)
						mib_set(MIB_WLAN_WEP64_KEY2, (void *)key2);
					if (key3[0] != 0)
						mib_set(MIB_WLAN_WEP64_KEY3, (void *)key3);
					if (key4[0] != 0)
						mib_set(MIB_WLAN_WEP64_KEY4, (void *)key4);
				}
				else {
					if (key1[0] != 0)
						mib_set(MIB_WLAN_WEP128_KEY1, (void *)key1);
					if (key2[0] != 0)
						mib_set(MIB_WLAN_WEP128_KEY2, (void *)key2);
					if (key3[0] != 0)
						mib_set(MIB_WLAN_WEP128_KEY3, (void *)key3);
					if (key4[0] != 0)
						mib_set(MIB_WLAN_WEP128_KEY4, (void *)key4);
				}
			}
			else if (encrypt >= ENCRYPT_WPA) {
#ifdef ENABLE_WPAAES_WPA2TKIP
				mib_set(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&wpaCipher);
#endif
				mib_set(MIB_WLAN_WPA_AUTH, (void *)&wpaAuth);
				if (getPSK == 1) {
					mib_set(MIB_WLAN_WPA_PSK_FORMAT, (void *)&pskfmt);
					mib_set(MIB_WLAN_WPA_PSK, (void *)psk);
				}
			}
#ifdef WLAN_1x
			mib_set(MIB_WLAN_ENABLE_1X, (void *)&use1x);
#endif
			mib_set(MIB_WLAN_ENCRYPT, (void *)&encrypt);
			config_WLAN(ACT_RESTART);
			break;
		case 2://(2) Quit
			return;
		}//end switch, Wireless Security Setup
	}//end while, Wireless Security Setup
}

#ifdef WLAN_ACL
// Jenny
//void addWLanAC(unsigned char *mac)
int addWLanAC(unsigned char *mac)
{
	MIB_CE_WLAN_AC_T macEntry;
	MIB_CE_WLAN_AC_T Entry;
	int entryNum, i, intVal;

	if (strlen(mac) != 12 || !string_to_hex(mac, macEntry.macAddr, 12) || !isValidMacAddr(macEntry.macAddr)) {
		printf("Error! Invalid MAC address.");
		return 0;
	}

	entryNum = mib_chain_total(MIB_WLAN_AC_TBL);
	if (entryNum >= MAX_WLAN_AC_NUM) {
		printf("Cannot add new entry because table is full!");
		return 0;
	}

	// set to MIB. Check if entry exists
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_WLAN_AC_TBL, i, (void *)&Entry)) {
			printf("Get chain record error!\n");
			return 0;
		}

		if (!memcmp(macEntry.macAddr, Entry.macAddr, 6)) {
			printf("Entry already exists!");
			return 0;
		}
	}

	intVal = mib_chain_add(MIB_WLAN_AC_TBL, (unsigned char*)&macEntry);
	if (intVal == 0) {
		printf(strAddChainerror);
		return 0;
	}
	else if (intVal == -1) {
		printf(strTableFull);
		return 0;
	}
	return 1;
}

// Jenny
void showWLanACL();
//void delWLanAC(){
int delWLanAC()
{
	int mibTblId;
	unsigned int totalEntry;
	unsigned int index;
	int del,min,max;

	mibTblId = MIB_WLAN_AC_TBL;

	totalEntry = mib_chain_total(mibTblId); /* get chain record size */
	if (totalEntry==0) {
		printf("Empty table!\n");
		return 0;	
	}
	min=1;max=2;
	if (getInputUint("Delete (1)One (2)All :",&del,&min,&max)==0){
		printf("Invalid selection!\n");
		return 0;
	}

	if (del==2)
		mib_chain_clear(mibTblId); /* clear chain record */
	else if (del==1) {
		showWLanACL();
		min=1;max=totalEntry+1;
		getInputUint( "Select the index to delete:",&index, &min,&max);
		if (index>totalEntry || index<=0) {
			printf("Error selection!\n");
			return 0;
		}
		if (mib_chain_delete(mibTblId, index-1) != 1) {
			printf("Delete chain record error!");
			return 0;
		}
	}
	return 1;
}

// Jenny
void showWLanACL()
{
	int entryNum, i;
	MIB_CE_WLAN_AC_T Entry;
	char tmpbuf[20];
	
	CLEAR;
	printf("\n");
	printf("Current Access Control List\n");
	printf("Idx  MAC Address\n");
	printf("---------------------------------------\n");
	
	entryNum = mib_chain_total(MIB_WLAN_AC_TBL);
	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_WLAN_AC_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return;
		}
		snprintf(tmpbuf, 20, "%02x:%02x:%02x:%02x:%02x:%02x",
			Entry.macAddr[0], Entry.macAddr[1], Entry.macAddr[2],
			Entry.macAddr[3], Entry.macAddr[4], Entry.macAddr[5]);
		printf("%-5d%-12s\n", i+1, tmpbuf);
	}
}

// Jenny, Wireless Access Control
void setWlanAC()
{
	int snum, min, max, sel, ret;
	unsigned char vChar;
	unsigned char mac[16];

	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		printf("                           Wireless Access Control                         \n");
		MSG_LINE;
		printf("If you choose 'Allowed Listed', only those clients whose wireless MAC \n");
		printf("addresses are in the access control list will be able to connect to your \n");
		printf("Access Point. When 'Deny Listed' is selected, these wireless clients on \n");
		printf("the list will not be able to connect the Access Point.\n");
		MSG_LINE;
		getSYS2Str(SYS_WLAN_AC_ENABLED, strbuf);
		printf("Wireless Access Control Mode: %s\n",strbuf);	   
		MSG_LINE;
		printf("(1) Set                          (2) Add\n");
		printf("(3) Delete                       (4) Show Access Control List\n");
		printf("(5) Quit\n");
		if (!getInputOption( &snum, 1, 5))
			continue;
      		switch( snum)
		{
			case 1://(1) Set
				//getSYS2Str(SYS_WLAN_AC_ENABLED, strbuf);
				//printf("Wireless Access Control Mode: %s\n",strbuf);       
				min=1;max=3;
				if(getInputUint("Set Wireless access control mode (1)Disable (2)Allow Listed (3)Deny Listed:", &sel, &min, &max)==0)
					continue;
				vChar = (unsigned char)(sel-1);
				mib_set(MIB_WLAN_AC_ENABLED, (void *)&vChar);
//#ifdef APPLY_CHANGE
				config_WLAN(ACT_RESTART);
//#endif
         			break;
			case 2://(2) Add
				mib_get(SYS_WLAN_AC_ENABLED, (void *)&vChar);
				if (vChar) {	// WLACL enabled
					if (0 == getInputString("MAC Address: (ex. 00E086710502) ", mac, sizeof(mac)))
						continue;
					ret = addWLanAC(mac);
//#ifdef APPLY_CHANGE
					if (ret)
						config_WLAN(ACT_RESTART);
					else
						printWaitStr();
//#endif
				}
				else {
					printf("Wireless Access Control is disabled!!\n");
					printWaitStr();
				}
         			break;
			case 3://(3) Delete
				ret = delWLanAC();
//#ifdef APPLY_CHANGE
				if (ret)
					config_WLAN(ACT_RESTART);
				else
					printWaitStr();
//#endif
         			break;
      			case 4://(4) Show Access Control List
         			showWLanACL();
				printWaitStr();
         			break;
      			case 5://(5) Quit
         			return;
      		}//end switch, Port Forwarding
	}
}
#endif

#ifdef WLAN_WDS
// Jenny
//void addWLanWDS(unsigned char *mac, unsigned char *comment)
int addWLanWDS(unsigned char *mac, unsigned char *comment)
{
	WDS_T macEntry;
	unsigned char entryNum;
	int intVal;

	if (strlen(mac) != 12 || !string_to_hex(mac, macEntry.macAddr, 12) || !isValidMacAddr(macEntry.macAddr)) {
		printf("Error! Invalid MAC address.");
		return 0;
	}

	if (comment != NULL) {
		if (strlen(comment) > COMMENT_LEN-1) {
			printf("Error! Comment length too long.");
			return 0;
		}
		strcpy(macEntry.comment, comment);
	}
	else
		macEntry.comment[0] = '\0';

	if (!mib_get(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
		printf("Get entry number error!");
		return 0;
	}
	if ( (entryNum + 1) > MAX_WDS_NUM) {
		printf("Cannot add new entry because table is full!");
		return 0;
	}

	// set to MIB.
	intVal = mib_chain_add(MIB_WDS_TBL, (void *)&macEntry);
	if (intVal == 0) {
		printf(strAddChainerror);
		return 0;
	}
	else if (intVal == -1) {
		printf(strTableFull);
		return 0;
	}
	entryNum++;
	if (!mib_set(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
		printf("Set entry number error!");
		return 0;
	}
	return 1;
}

// Jenny
void showWLanWDS();
//void delWLanWDS()
int delWLanWDS()
{
	unsigned char entryNum, delNum=0;
	unsigned int index;
	int del,min,max;

	delNum=0;
	mib_get(MIB_WLAN_WDS_NUM, (void *)&entryNum); /* get chain record size */
	if (entryNum==0){
		printf("Empty table!\n");
		return 0;	
	}
	min=1; max=2;
	if (getInputUint("Delete (1)One (2)All :",&del,&min,&max)==0){
		printf("Invalid selection!\n");
		return 0;
	}

	if (del == 2) {
		mib_chain_clear(MIB_WDS_TBL); /* clear chain record */
		entryNum = 0;
		if (!mib_set(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
			printf("Set entry number error!");
			return 0;
		}
	}
	else if (del==1) {
		showWLanWDS();
		min=1; max=entryNum;
		getInputUint("Select the index to delete:",&index, &min,&max);
		if (index>entryNum || index<=0) {
			printf("Error selection!\n");
			return 0;
		}
		if (mib_chain_delete(MIB_WDS_TBL, index-1) != 1) {
			printf("Delete chain record error!");
			return 0;
		}
		delNum++;
		entryNum -= delNum; 
		if (!mib_set(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
			printf("Get entry number error!");
			return 0;
		}
	}
	return 1;
}

// Jenny
void showWLanWDS()
{
	unsigned char entryNum;
	int i;
	WDS_T entry;
	WDS_T Entry;
	char tmpbuf[20];
	
	printf("\n");
	printf("Current WDS AP List\n");
	printf("Idx  MAC Address        Comment\n");
	printf("---------------------------------------------\n");
	
	mib_get(MIB_WLAN_WDS_NUM, (void *)&entryNum);
	for (i=0; i<entryNum; i++) {
		*((char *)&entry) = (char)i;
		
		if (!mib_chain_get(MIB_WDS_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return;
		}
		snprintf(tmpbuf, 20, "%02x:%02x:%02x:%02x:%02x:%02x", 
			Entry.macAddr[0], Entry.macAddr[1], Entry.macAddr[2],	
			Entry.macAddr[3], Entry.macAddr[4], Entry.macAddr[5]);
		printf("%-5d%-19s%-20s\n", i+1, tmpbuf, Entry.comment);
	}
}

// Jenny, WDS
void setWlWDS()
{
	int snum, min, max, sel, ret;
	unsigned char vChar, mode, disWlan;
	unsigned char mac[16], comment[25];
	
	mib_get( MIB_WLAN_DISABLED, (void *)&disWlan);
	mib_get(MIB_WLAN_MODE, (void *)&mode); // get current WLAN mode
	if (disWlan || mode == AP_MODE || mode == CLIENT_MODE) {
		printf("WDS mode is disabled.\n");
		printWaitStr();
		return;
	}

	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		printf("                           WDS Settings                         \n");
		MSG_LINE;
		printf("Wireless Distribution System uses wireless media to communicate with \n");
		printf("other APs, like the Ethernet does. To do this, you must set these APs \n");
		printf("in the same channel and set MAC address of other APs which you want \n");
		printf("to communicate with in the table and then enable the WDS.\n");
		MSG_LINE;
		getSYS2Str(SYS_WLAN_WDS_ENABLED, strbuf);
		printf("WDS: %s\n",strbuf); 	  
		MSG_LINE;
		printf("(1) Set                          (2) Add WDS AP\n");
		printf("(3) Delete                       (4) Show WDS AP List\n");
		printf("(5) Quit\n");
		if (!getInputOption( &snum, 1, 5))
			continue;
		
      		switch( snum)
		{
			case 1://(1) Set
				//getSYS2Str(SYS_WLAN_WDS_ENABLED, strbuf);
				//printf("WDS: %s\n",strbuf);       
				min=1;max=2;
				if(getInputUint("Set WDS (1)Disable (2)Enable:",&sel,&min,&max)==0)
					continue;
				vChar = (unsigned char)(sel-1);
				mib_set( MIB_WLAN_WDS_ENABLED, (void *)&vChar);
//#ifdef APPLY_CHANGE
				config_WLAN(ACT_RESTART);
//#endif
         			break;
			case 2://(2) Add
				mib_get(MIB_WLAN_WDS_ENABLED, (void *)&vChar);
				if (vChar) {	// WDS enabled
					if (0 == getInputString("MAC Address: (ex. 00E086710502) ", mac, sizeof(mac)))
						continue;
					//if (0 == getInputString("Comment: ", comment, sizeof(comment)))
					//	continue;
					getTypedInputDefault(INPUT_TYPE_STRING,"Comment[None]: ", comment, sizeof(comment)-1, 0);
         				ret = addWLanWDS(mac, comment);
//#ifdef APPLY_CHANGE
					if (ret)
						config_WLAN(ACT_RESTART);
					else
						printWaitStr();
//#endif
				}
				else {
					printf("WDS is disabled!!\n");
					printWaitStr();
				}
         			break;
			case 3://(3) Delete
				ret = delWLanWDS();
//#ifdef APPLY_CHANGE
				if (ret)
					config_WLAN(ACT_RESTART);
				else
					printWaitStr();
//#endif
         			break;
      			case 4://(4) Show WDS AP List
         			showWLanWDS();
				printWaitStr();
         			break;
      			case 5://(5) Quit
         			return;
      		}//end switch, Port Forwarding
	}
}
#endif	// of WLAN_WDS

#ifdef WLAN_CLIENT
// Jenny
static SS_STATUS_Tp pStatus=NULL;
int ssNum;
void showWlSiteSurvey()
{
	WLAN_MODE_T mode;
	unsigned char mib_mode;
	bss_info bss;
	int i=0;
	BssDscr *pBss;
	char tmpbuf[20], ssidbuf[40];
	
	if (pStatus==NULL) {
		pStatus = calloc(1, sizeof(SS_STATUS_T));
		if ( pStatus == NULL ) {
			printf("Allocate buffer failed!\n");
			return;
		}
	}
	pStatus->number = 0; // request BSS DB
	if ( getWlSiteSurveyResult(WLANIF, pStatus) < 0 ) {
		printf("Read site-survey status failed!");
		free(pStatus);
		pStatus = NULL;
		return;
	}
	if ( !mib_get( MIB_WLAN_MODE, (void *)&mib_mode) ) {
		printf("Get MIB_WLAN_MODE MIB failed!");
		return;
	}
	mode=mib_mode;
	if ( getWlBssInfo(WLANIF, &bss) < 0) {
		printf("Get bssinfo failed!");
		return;
	}

	printf("\n");
	printf("Idx  SSID             BSSID              Channel  Type    Encrypt  Signal\n");
	MSG_LINE;

	ssNum = 0;
	for (i=0; i<pStatus->number && pStatus->number!=0xff; i++) {
		pBss = &pStatus->bssdb[i];
		snprintf(tmpbuf, 20, "%02x:%02x:%02x:%02x:%02x:%02x", 
			pBss->bdBssId[0], pBss->bdBssId[1], pBss->bdBssId[2],
			pBss->bdBssId[3], pBss->bdBssId[4], pBss->bdBssId[5]);
		memcpy(ssidbuf, pBss->bdSsIdBuf, pBss->bdSsId.Length);
		ssidbuf[pBss->bdSsId.Length] = '\0';
		printf("%-5d%-17s%-19s%-9s%-8s%-9s%-8s\n",
			i+1, ssidbuf, tmpbuf, pBss->ChannelNumber, ((pBss->bdCap & cIBSS) ? "Ad Hoc" : "AP"),
			((pBss->bdCap & cPrivacy) ? "Yes" : "No"), pBss->rssi);
		ssNum ++;
	}
}

// Jenny
void connectWlSiteSurvey()
{
	unsigned int idx, min, max;
	unsigned char mode, res;
	NETWORK_TYPE_T net;
	char tmpBuf[100];
	int chan, wait_time;

	mib_get(MIB_WLAN_MODE, (void *)&mode); // get current WLAN mode
	if (mode != CLIENT_MODE) {
		printf("Client mode is disabled.\n");
		return;
	}
	if (ssNum == 0) {
		printf("No Access Point found!!\n");
		return;
	}

	min=1; max=ssNum;
	if (0 == getInputUint("\nConnect: ", &idx, &min, &max))
		return;   
	if ( idx > pStatus->number ) { // invalid index
		printf("Connect failed!");
		return;
	}

	// Set SSID, network type to MIB
	memcpy(tmpBuf, pStatus->bssdb[idx].bdSsIdBuf, pStatus->bssdb[idx].bdSsId.Length);
	tmpBuf[pStatus->bssdb[idx].bdSsId.Length] = '\0';
	if ( mib_set(MIB_WLAN_SSID, (void *)tmpBuf) == 0) {
		printf("Set SSID error!");
		return;
	}
	if ( pStatus->bssdb[idx].bdCap & cESS )
		net = INFRASTRUCTURE;
	else
		net = ADHOC;
	if ( mib_set(MIB_WLAN_NETWORK_TYPE, (void *)&net) == 0) {
		printf("Set MIB_WLAN_NETWORK_TYPE failed!");
		return;
	}
	if (net == ADHOC) {
		chan = pStatus->bssdb[idx].ChannelNumber;
		if ( mib_set( MIB_WLAN_CHAN_NUM, (void *)&chan) == 0) {
			printf("Set channel number error!");
			return;
		}
	}

	res = idx-1;
	wait_time = 0;
	while (1) {
		if ( getWlJoinRequest(WLANIF, &pStatus->bssdb[idx], &res) < 0 ) {
			printf("Join request failed!");
			return;
		}
		if ( res == 1 ) { // wait
			if (wait_time++ > 5) {
				printf("connect-request timeout!");
				return;
			}
			sleep(1);
			continue;
		}
		break;
	}

	if ( res == 2 ) // invalid index
		printf("Connect failed!\n");
	else {
		wait_time = 0;
		while (1) {
			if ( getWlJoinResult(WLANIF, &res) < 0 ) {
				printf("Get Join result failed!");
				return;
			}
			if ( res != 0xff ) { // completed
				if (wait_time++ > 10) {
					printf("connect timeout!");
					return;
				}
				break;
			}
			sleep(1);
		}

		if ( res!=STATE_Bss && res!=STATE_Ibss_Idle && res!=STATE_Ibss_Active )
			printf("Connect failed!\n");
		else
			printf("Connect successfully!\n");
	}
}

// Jenny, Wireless Site Survey
void setWlSiteSurvey()
{
	int snum;
	unsigned char disWlan;
	
	mib_get( MIB_WLAN_DISABLED, (void *)&disWlan);
	if (disWlan)
		return;
	
	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		printf("                           Wireless Site Survey                         \n");
		MSG_LINE;
		printf("This page provides tool to scan the wireless network. If any Access \n");
		printf("Point or IBSS is found, you could choose to connect it manually when \n");
		printf("client mode is enabled.\n");
		MSG_LINE;
		printf("(1) Show                         (2) Connect\n");
		printf("(3) Quit\n");
		if (!getInputOption( &snum, 1, 3))
			continue;
		
      		switch( snum)
		{
      			case 1://(1) Show
         			showWlSiteSurvey();
         			break;
      			case 2://(2) Connect
         			connectWlSiteSurvey();
         			break;
      			case 3://(2) Quit
         			return;
      		}//end switch, Port Forwarding
	}
}
#endif	// of WLAN_CLIENT

/*************************************************************************************************/
void setWireless()
{
	int snum;
	
	while (1)
	{
		CLEAR;
		printf("\n");
		MENU_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("                 (3) Wireless Menu                                       \n");
		MENU_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("(1) Basic Settings               (2) Advanced Settings\n");
#ifdef WLAN_ACL
		printf("(3) Security                     (4) Access Control\n");
#if (defined(WLAN_WDS) && defined(WLAN_CLIENT))
		printf("(5) WDS                          (6) Site Survey\n");
		printf("(7) Quit\n");
		if (getInputOption( &snum, 1, 7) == 0)
			continue;
#else
#ifdef WLAN_WDS
		printf("(5) WDS                          (6) Quit\n");
		if (getInputOption( &snum, 1, 6) == 0)
			continue;
#else
#ifdef WLAN_CLIENT
		printf("(5) Site Survey                  (6) Quit\n");
		if (getInputOption( &snum, 1, 6) == 0)
			continue;
#else
		printf("(5) Quit\n");
		if (getInputOption( &snum, 1, 5) == 0)
			continue;
#endif	// of WLAN_CLIENT
#endif	// of WLAN_WDS
#endif	// of WLAN_WDS && WLAN_CLIENT
#else
#if (defined(WLAN_WDS) && defined(WLAN_CLIENT))
		printf("(3) Security                     (4) WDS\n");
		printf("(5) Site Survey                  (6) Quit\n");
		if (getInputOption( &snum, 1, 6) == 0)
			continue;
#else
#ifdef WLAN_WDS
		printf("(3) Security                     (4) WDS\n");
		printf("(5) Quit\n");
		if (getInputOption( &snum, 1, 5) == 0)
			continue;
#else
#ifdef WLAN_CLIENT
		printf("(3) Security                     (4) Site Survey\n");
		printf("(5) Quit\n");
		if (getInputOption( &snum, 1, 5) == 0)
			continue;
#else
		printf("(3) Security                     (4) Quit\n");
		if (getInputOption( &snum, 1, 4) == 0)
			continue;
#endif	// of WLAN_CLIENT
#endif	// of WLAN_WDS
#endif	// of WLAN_WDS && WLAN_CLIENT
#endif	// of WLAN_ACL
		
		switch( snum)
		{
		case 1://(1) Basic Settings
			setWlanBasic();
			break;
		case 2://(2) Advanced Settings
			setWlanAdv();
			break;
		case 3://(3) Security
			setWlanSecurity();
			break;
#ifdef WLAN_ACL
		case 4://(4) Access Control
			setWlanAC();
			break;
#if (defined(WLAN_WDS) && defined(WLAN_CLIENT))
		case 5://(5) WDS
			setWlWDS();
			break;
		case 6://(6) Site Survey
			setWlSiteSurvey();
			break;
		case 7://(7) Quit
			return;
#else
#ifdef WLAN_WDS
		case 5://(5) WDS
			setWlWDS();
			break;
		case 6://(6) Quit
			return;
#else
#ifdef WLAN_CLIENT
		case 5://(5) Site Survey
			setWlSiteSurvey();
			break;
		case 6://(6) Quit
			return;
#else
		case 5://(5) Quit
			return;
#endif	// of WLAN_CLIENT
#endif	// of WLAN_WDS
#endif	// of WLAN_WDS && WLAN_CLIENT
#else
#if (defined(WLAN_WDS) && defined(WLAN_CLIENT))
		case 4://(4) WDS
			setWlWDS();
			break;
		case 5://(5) Site Survey
			setWlSiteSurvey();
			break;
		case 6://(6) Quit
			return;
#else
#ifdef WLAN_WDS
		case 4://(4) WDS
			setWlWDS();
			break;
		case 5://(5) Quit
			return;
#else
#ifdef WLAN_CLIENT
		case 4://(4) Site Survey
			setWlSiteSurvey();
			break;
		case 5://(5) Quit
			return;
#else
		case 4://(4) Quit
			return;
#endif	// of WLAN_CLIENT
#endif	// of WLAN_WDS
#endif	// of WLAN_WDS && WLAN_CLIENT
#endif	// of WLAN_ACL
		default:
			printf("!! Invalid Selection !!\n");
		}//end switch, (3) Wireless Menu
	}//end while, (3) Wireless Menu
}
#endif

static int getVcInput(MIB_CE_ATM_VC_Tp entry, MIB_CE_ATM_VC_Tp old) {
	unsigned int min,max;
	unsigned int num;   
	
	entry->qos = 0;
	entry->pcr = 7600;   
	
	min=0;max=255;
	if (0 == getInputUint("VPI:", &num, &min, &max))
		return 0;
	entry->vpi = (unsigned char)num;
	
	min=0;max=65535;
	if (0 == getInputUint("VCI:", &num, &min, &max))
		return 0;
	entry->vci = (unsigned short)num;
	
	min=1;max=2;
	if (0 == getInputUint("Encapsulation (1)LLC (2)VC-Mux :", &num, &min, &max))
		return 0;
	if (num == 1)
		entry->encap = 1;
	else
		entry->encap = 0;
#ifdef ZTE_531B_BRIDGE_SC
	max=1;//change from 5 to 1
//	if (0 == getInputUint("Channel Mode (1)1483 Bridged (2)1483 MER (3)PPPoE (4)PPPoA (5)1483 Routed :", &num, &min, &max))
	if (0 == getInputUint("Channel Mode (1)1483 Bridged:", &num, &min, &max))
#else
	max=5;
	if (0 == getInputUint("Channel Mode (1)1483 Bridged (2)1483 MER (3)PPPoE (4)PPPoA (5)1483 Routed :", &num, &min, &max))
#endif
		return 0;
	entry->cmode = num-1;
	
	if (entry->cmode != ADSL_BR1483) {
		max=2;
		if (0 == getInputUint("NAPT (1)Enable (2)Disable :", &num, &min, &max))
			return 0;
		if (num == 1)
			entry->napt = 1;
		else
			entry->napt = 0;
	} else {
		entry->napt = 0;
	}
	
	max=2;
	if (0 == getInputUint("The Status of Channel (1)Enable (2)Disable :", &num, &min, &max))
		return 0;
	if (num==1)
		entry->enable = 1;
	else
		entry->enable = 0;
	
	entry->brmode = 1;
	
	// 1483 bridged
	if (entry->cmode == ADSL_BR1483)
	{
		entry->brmode = 0;
		entry->ipAddr[0]=0;
		entry->ipAddr[1]=0;
		entry->ipAddr[2]=0;
		entry->ipAddr[3]=0;
		entry->remoteIpAddr[0]=0;
		entry->remoteIpAddr[1]=0;
		entry->remoteIpAddr[2]=0;
		entry->remoteIpAddr[3]=0;
		entry->netMask[0]=0;
		entry->netMask[1]=0;
		entry->netMask[2]=0;
		entry->netMask[3]=0;
#ifdef DEFAULT_GATEWAY_V1
		entry->dgw=0;
#endif
		entry->cdvt=0;

	} else // PPP connection
	if (entry->cmode == ADSL_PPPoE || entry->cmode == ADSL_PPPoA)
	{
		printf("PPP Settings\n");
#ifdef CONFIG_GUI_WEB
		if (0 == getInputString("Username: ", entry->pppUsername, P_MAX_NAME_LEN-1))
			return 0;
		
		if (0 == getInputString("Password: ", entry->pppPassword, P_MAX_NAME_LEN-1))
			return 0;
#else
		if (0 == getInputString("Username: ", entry->pppUsername, MAX_PPP_NAME_LEN))
		//if (0 == getInputString("Username: ", entry->pppUsername, MAX_NAME_LEN))
			return 0;
		
		if (0 == getInputString("Password: ", entry->pppPassword, MAX_NAME_LEN-1))
		//if (0 == getInputString("Password: ", entry->pppPassword, MAX_NAME_LEN))
			return 0;
#endif
		
		min=1;max=3;
		if (0 == getInputUint("Connect Type (1)Continuous (2)Connect on Demand (3)Manual :", &num, &min, &max))
			return 0;
		
		if (1 == num)
			entry->pppCtype = CONTINUOUS;
		else if (2 == num)
			entry->pppCtype = CONNECT_ON_DEMAND;
		else
			entry->pppCtype = MANUAL;
		
//		if (entry->pppCtype != CONTINUOUS) {
		if (entry->pppCtype == CONNECT_ON_DEMAND) {	// Jenny
			min=1;max=65535;
			if (0 == getInputUint("Idle Time (min) :", &num, &min, &max))
				return 0;
			entry->pppIdleTime = num;
		}
		      
	} else // WAN
	{
		printf("WAN IP Settings :\n");
		min=1;max=2;
		if (entry->cmode == ADSL_RT1483) {
			//num == 1;
			if (0 == getInputUint("Type (1)Fixed IP (2)Unnumbered :", &num, &min, &max))
				return 0;
		}
		else {
			//min=1;max=2;
			if (0 == getInputUint("Type (1)Fixed IP (2)DHCP :", &num, &min, &max))
				return 0;
		}
		if (num == 1) {
			char *temp, strIp[20], strGW[20], strMask[20];
			entry->ipDhcp = DHCP_DISABLED;
			if (0 == getInputIpAddr("Local IP Address :", (struct in_addr *)&entry->ipAddr)) return 0;
			if (0 == getInputIpAddr("Remote IP Address :", (struct in_addr *)&entry->remoteIpAddr)) return 0;
			if (0 == getInputIpMask("Subnet Mask :", (struct in_addr *)&entry->netMask)) return 0;
			
			temp = inet_ntoa(*((struct in_addr *)entry->ipAddr));
			strcpy(strIp, temp);
			temp = inet_ntoa(*((struct in_addr *)entry->remoteIpAddr));
			strcpy(strGW, temp);
			temp = inet_ntoa(*((struct in_addr *)entry->netMask));
			strcpy(strMask, temp);
			if (!isValidHostID(strIp, strMask)) {
				printf("Invalid IP/Subnet Mask combination!!\n");
				return 0;
			}
			if (!isSameSubnet(strIp, strGW, strMask)) {
				printf("Invalid IP address! It should be located in the same subnet.\n");
				return 0;
			}
		} else {
			if (entry->cmode == ADSL_RT1483)
				entry->ipunnumbered = 1;
			else
				entry->ipDhcp = DHCP_CLIENT;
		}
	}


#ifdef DEFAULT_GATEWAY_V1
	if (entry->cmode != ADSL_BR1483)
	{
		max=2;
		if (0 == getInputUint("Default Route Setting (1)Disable (2)Enable :", &num, &min, &max))
			return 0;
		if (num==1)
			entry->dgw = 0;
		else
			entry->dgw = 1;
	}
#endif
	
	return 1;
}
/*************************************************************************************************/
void addChannelConfiguration()
{
	MIB_CE_ATM_VC_T entry;    
	
	// Added by Mason Yu for AC Name is not NULL(WAN can not get IP Address on PPPoE Mode)
	memset(&entry, 0x00, sizeof(entry));
	
	if (getVcInput(&entry, 0) == 0)
	   return;
	deleteAllConnection();
	
	// now do the actual set.
	do {
		unsigned int ifMap;	// high half for PPP bitmap, low half for vc bitmap
#ifdef DEFAULT_GATEWAY_V1
		//int drouted=-1;
		int drouted = 0;
#endif
		int i, cnt, pIdx, intVal;
		unsigned int totalEntry;
		MIB_CE_ATM_VC_T Entry;
		//MIB_CE_ATM_VC_Tp pmyEntry;
		
		ifMap = 0;
		cnt=0;
		totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
		if (totalEntry >= MAX_VC_NUM) {
			printf(strMaxVc);
			printf("\n");
			restartWAN();
			return;
		}

		for (i=0; i<totalEntry; i++) {
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			{
	  			printf("Get chain record error!\n");
				restartWAN();
				return;
			}
			
			if (Entry.vpi == entry.vpi && Entry.vci == entry.vci)
			{
				cnt++;
				pIdx = i;
				//pmyEntry = &Entry;
			}
			
			ifMap |= 1 << (Entry.ifIndex & 0x0f);	// vc map
			ifMap |= (1 << 16) << ((Entry.ifIndex >> 4) & 0x0f);	// PPP map

#ifdef DEFAULT_GATEWAY_V1
			//if ((drouted == -1) && Entry.dgw && (Entry.cmode != ADSL_BR1483))	// default route entry
			//	drouted = i;
			if (Entry.cmode != ADSL_BR1483)
				if (Entry.dgw)
					drouted = 1;
#endif
		}
		
		if (cnt == 0)	// pvc not exists
		{
			entry.ifIndex = if_find_index(entry.cmode, ifMap);
			if (entry.ifIndex == 0xff)
			{
				printf("Error: Maximum number of VC exceeds !\n");
				restartWAN();
				return;
			}
			else if (entry.ifIndex == 0xef)
			{
				printf("Error: Maximum number of PPP exceeds !\n");
				restartWAN();
				return;
			}
#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
			entry.ConDevInstNum = 1 + findMaxConDevInstNum();
			if( (entry.cmode==ADSL_PPPoE) || 
#ifdef PPPOE_PASSTHROUGH
			    ((entry.cmode==ADSL_BR1483)&&(entry.brmode==BRIDGE_PPPOE)) ||
#endif
			    (entry.cmode==ADSL_PPPoA) )
				entry.ConPPPInstNum = 1;
			else
				entry.ConIPInstNum = 1;
			//fprintf( stderr, "<%s:%d>NewInstNum=>ConDev:%u, PPPCon:%u, IPCon:%u\n", __FILE__, __LINE__, entry.ConDevInstNum, entry.ConPPPInstNum, entry.ConIPInstNum );
#endif
		}
		else	// pvc exists
		{
			if (!mib_chain_get(MIB_ATM_VC_TBL, pIdx, (void *)&Entry)) {	// for multisession PPPoE, get existed pvc config
				printf(errGetEntry);
				printf("\n");
				restartWAN();
				return;
			}
			//if (pmyEntry->cmode == ADSL_PPPoE)
			if (Entry.cmode == ADSL_PPPoE && entry.cmode == ADSL_PPPoE)
			{
				if (cnt<MAX_POE_PER_VC)
				{	// get the pvc info.
					//entry.qos = pmyEntry->qos;
					//entry.pcr = pmyEntry->pcr;
					//entry.encap = pmyEntry->encap;
					entry.qos = Entry.qos;
					entry.pcr = Entry.pcr;
					entry.encap = Entry.encap;
					ifMap &= 0xffff0000; // don't care the vc part
					entry.ifIndex = if_find_index(entry.cmode, ifMap);
					if (entry.ifIndex == 0xef)
					{
						printf("Error: Maximum number of PPP exceeds !\n");
						restartWAN();
						return;
					}
					entry.ifIndex &= 0xf0;
					//entry.ifIndex |= (pmyEntry->ifIndex&0x0f);
					entry.ifIndex |= (Entry.ifIndex&0x0f);
#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
					//entry.ConDevInstNum = pmyEntry->ConDevInstNum;
					//entry.ConPPPInstNum = 1 + findMaxPPPConInstNum( pmyEntry->ConDevInstNum );
					entry.ConDevInstNum = Entry.ConDevInstNum;
					entry.ConPPPInstNum = 1 + findMaxPPPConInstNum( Entry.ConDevInstNum );
					//fprintf( stderr, "<%s:%d>NewInstNum=>ConDev:%u, PPPCon:%u, IPCon:%u\n", __FILE__, __LINE__, entry.ConDevInstNum, entry.ConPPPInstNum, entry.ConIPInstNum );
#endif
				}
				else
				{
					printf("Maximum number of PPPoE connections exceeds in this vc!\n");
					restartWAN();
					return;
				}
			}
			else
			{
				printf("Connection already exists!\n");
				restartWAN();
				return;
			}
		}
		
#ifdef DEFAULT_GATEWAY_V1
		if (entry.cmode != ADSL_BR1483)
			if (drouted && entry.dgw) {
				printf(strDrouteExist);
				printf("\n");
				restartWAN();
				return;
			}
#endif
		if (entry.cmode == ADSL_PPPoE)
			entry.mtu = 1452;
		else
			entry.mtu = 1500;

#ifdef CONFIG_EXT_SWITCH
				// VLAN
				entry.vlan = 0; // disable
				entry.vid = 0; // VLAN tag
				entry.vprio = 0; // priority bits (0 ~ 7)
				entry.vpass = 0; // no pass-through
#endif
		intVal = mib_chain_add(MIB_ATM_VC_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			printf(strAddChainerror);
			printf("\n");
			restartWAN();
			return;
		}
		else if (intVal == -1) {
			printf(strTableFull);
			printf("\n");
			restartWAN();
			return;
		}
		restartWAN();
	} while (0);		
	
	//ql 20081118 START restart IP QoS
#ifdef NEW_IP_QOS_SUPPORT
	take_qos_effect();
#endif
}


void deleteChannelConfiguration()
{
   //char vpi[4], vci[4];
   //unsigned int vpi, vci;
   unsigned int idx, min, max;

   CLEAR;
   max = _wanConfList(1); 
   printf("\n\n");

   min=0;
   if (0 == getInputUint("PVC to delete: ", &idx, &min, &max))
      return;   

   deleteAllConnection();
   resolveServiceDependency(idx);
#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
   {
	MIB_CE_ATM_VC_T cwmp_entry;
	if (mib_chain_get(MIB_ATM_VC_TBL, idx, (void *)&cwmp_entry))
	{
		delPortForwarding( cwmp_entry.ifIndex );
		delRoutingTable( cwmp_entry.ifIndex );
	}
   }
#endif
   if(mib_chain_delete(MIB_ATM_VC_TBL, idx) != 1) {
      printf("Delete chain record error!\n");
      restartWAN();
	  
	//ql 20081118 START restart IP QoS
#ifdef NEW_IP_QOS_SUPPORT
	take_qos_effect();
#endif
	
      return;
   }   
#ifdef CONFIG_EXT_SWITCH
#ifdef ITF_GROUP
   {
	   int wanPortNum;
	   unsigned int swNum, vcNum;
	   MIB_CE_SW_PORT_T Entry;
	   MIB_CE_ATM_VC_T pvcEntry;
	   int j, grpnum;
	   char mygroup;
   
	   vcNum = mib_chain_total(MIB_ATM_VC_TBL);
	   for (grpnum=1; grpnum<=4; grpnum++) {
		   wanPortNum = 0;

		   for (j=0; j<vcNum; j++) {
			   if (!mib_chain_get(MIB_ATM_VC_TBL, j, (void *)&pvcEntry)) {
				   printf("Get chain record error!\n");
				   restartWAN();
				   return;
			   }

			   if (pvcEntry.enable == 0 || pvcEntry.itfGroup!=grpnum)
				   continue;
			   wanPortNum++;
		   }

		   if (0 == wanPortNum) {
			   printf("delete port mapping group %d\n", grpnum);
			   //release LAN ports
			   swNum = mib_chain_total(MIB_SW_PORT_TBL);
			   for (j=swNum; j>0; j--) {
				   if (!mib_chain_get(MIB_SW_PORT_TBL, j-1, (void *)&Entry)) {
					   restartWAN();
					   return;
				   }
				   if (Entry.itfGroup == grpnum) {
					   Entry.itfGroup = 0;
					   mib_chain_update(MIB_SW_PORT_TBL, (void *)&Entry, j-1);
				   }
			   }
#ifdef WLAN_SUPPORT
			   //release wlan0
			   mib_get(MIB_WLAN_ITF_GROUP, (void *)&mygroup);
			   if (mygroup == grpnum) {
				   mygroup = 0;
				   mib_set(MIB_WLAN_ITF_GROUP, (void *)&mygroup);
			   }
#endif
#ifdef WLAN_MBSSID
			   //release MBSSID
			   for (j=1; j<5; j++) {
				   mib_get(MIB_WLAN_VAP0_ITF_GROUP+j-1, (void *)&mygroup);
				   if (mygroup == grpnum) {
					   mygroup = 0;
					   mib_set(MIB_WLAN_VAP0_ITF_GROUP+j-1, (void *)&mygroup);
				   }
			   }
#endif
			   setgroup("", grpnum);
			   break;
		   }
	   }
   }//end
#endif	// of ITF_GROUP
#endif
   restartWAN();

	//ql 20081118 START restart IP QoS
#ifdef NEW_IP_QOS_SUPPORT
	take_qos_effect();
#endif
}

static int pppConnect(int idx, int isConnect) {
   MIB_CE_ATM_VC_T Entry;
   char ifname[6];
   int havePPP=0;
   struct data_to_pass_st msg;
   char qosParms[32];

   if (!mib_chain_get(MIB_ATM_VC_TBL, idx, (void *)&Entry))
      return (-1);

   if (Entry.cmode != ACC_PPPOE && Entry.cmode != ACC_PPPOA) {
			printf("not allowed for this connection !\n");
			return (-1);
	}
		
	snprintf(ifname, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));
	havePPP = find_ppp_from_conf(ifname);

   if (isConnect) {
      if (!havePPP) {
			// create a PPP connection
			printf("create %s here\n", ifname);
			if (Entry.cmode == ACC_PPPOE)	// PPPoE
			{
				// spppctl add 0 pppoe vc0 username USER password PASSWORD gw 1
				snprintf(msg.data, BUF_SIZE,
					"spppctl add %u pppoe vc%u username %s password %s gw 1",
					PPP_INDEX(Entry.ifIndex), VC_INDEX(Entry.ifIndex),
					Entry.pppUsername, Entry.pppPassword);
			}
			else	// PPPoA
			{
				if ((ATM_QOS_T)Entry.qos == ATMQOS_UBR)
				{
					snprintf(qosParms, 32, "ubr:pcr=%u", Entry.pcr);
				}
				else if ((ATM_QOS_T)Entry.qos == ATMQOS_UBR)
				{
					snprintf(qosParms, 32, "cbr:pcr=%u", Entry.pcr);
				}
				else	// rt-vbr or nrt-vbr
				{
					snprintf(qosParms, 32, "vbr:pcr=%u,scr=%u,mbs=%u",
						Entry.pcr, Entry.scr, Entry.mbs);
				}
				
				snprintf(msg.data, BUF_SIZE,
					"spppctl add %u pppoa %u.%u encaps %d qos %s "
					"username %s password %s gw 1",
					PPP_INDEX(Entry.ifIndex), Entry.vpi, Entry.vci, Entry.encap,
					qosParms, Entry.pppUsername, Entry.pppPassword);
			}
			//TRACE(STA_SCRIPT, "%s\n", msg.data);
			write_to_pppd(&msg);
		}
		else
		{
			/* printf("%s already exists\n", ifname); */
            /* Up this interface */
            printf("Connecting %s\n", ifname);
            // spppctl up ppp
            snprintf(msg.data, BUF_SIZE,
                "spppctl up %u", PPP_INDEX(Entry.ifIndex));
            write_to_pppd(&msg);
		}
		
		//goto setOk_filter;
   } else {
      
		if (havePPP) 
        {
            /* Disconnect ppp */
            printf("Disconnecting %s\n", ifname);
            // spppctl down ppp
            snprintf(msg.data, BUF_SIZE,
                "spppctl down %u", PPP_INDEX(Entry.ifIndex));
            write_to_pppd(&msg);
		}
		else
			printf("%s not exists\n", ifname);
				
   }


   return 0;
}

void connectChannelConfiguration()
{   
   unsigned int idx, min, max;

   max = wanConnList(1); 
   printf("\n\n");

   min=0;   
   if (0 == getInputUint("Connect: ", &idx, &min, &max))
      return;   

   pppConnect(idx, 1);
}

void disconnectChannelConfiguration()
{
   unsigned int idx, min, max;

   max = wanConnList(1); 
   printf("\n\n");

   min=0;   
   if (0 == getInputUint("Disconnect: ", &idx, &min, &max))
      return;   

   pppConnect(idx, 0); 
}

#if defined(AUTO_PVC_SEARCH_TR068_OAMPING) || defined(AUTO_PVC_SEARCH_PURE_OAMPING) || defined(AUTO_PVC_SEARCH_AUTOHUNT)
void showAutoPVCTbl()
{
	unsigned int entryNum, i;
	MIB_AUTO_PVC_SEARCH_T Entry;

	entryNum = mib_chain_total(MIB_AUTO_PVC_SEARCH_TBL);
	printf("\n");
	printf("Current Auto-PVC Table:\n");
	printf("Idx   VPI       VCI\n");
	printf("-------------------------------------------------------------------------\n");
	if (!entryNum)
		printf("No data!\n\n");

	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_AUTO_PVC_SEARCH_TBL, i, (void *)&Entry)) {
  			printf("Get chain record error!\n");
			return;
		}
		printf("%-6d%-10d%d\n", i+1, Entry.vpi, Entry.vci);
	}
}

void addAutoPVCPair()
{
	MIB_AUTO_PVC_SEARCH_T entry, Entry;
	unsigned int num, min, max;
	int mibtotal, i, intVal;

	memset(&entry, 0x00, sizeof(entry));
	min=0; max=255;
	if (0 == getInputUint("VPI:", &num, &min, &max))
		return 0;
	entry.vpi = (unsigned short)num;

	min=0;max=65535;
	if (0 == getInputUint("VCI:", &num, &min, &max))
		return 0;
	entry.vci = (unsigned int)num;

	mibtotal = mib_chain_total(MIB_AUTO_PVC_SEARCH_TBL);
	for (i=0; i<mibtotal; i++) {
		mib_chain_get(MIB_AUTO_PVC_SEARCH_TBL, i, (void*)&Entry);
		if(entry.vpi == Entry.vpi && entry.vci == Entry.vci){
			printf("Auto-PVC pair already exists!\n");
			goto setErr_pvc;
		}
	}

	intVal = mib_chain_add(MIB_AUTO_PVC_SEARCH_TBL, (unsigned char*)&entry);
	if (intVal == 0) {
		printf(strAddChainerror);
		goto setErr_pvc;
	}	
	else if (intVal == -1) {
		printf(strTableFull);
		goto setErr_pvc;
	}	
	return;

	setErr_pvc:
		printWaitStr();
		return;
}

void setAutoPVCSearch()
{
	unsigned int choice, num, min, max;
	unsigned char vChar;
	char enabled;
	int intVal;

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                           Auto PVC Search                             \n");
		printf("-------------------------------------------------------------------------\n");
		mib_get(MIB_ATM_VC_AUTOSEARCH, (void *)&enabled);
		printf("Auto-PVC Search: %s\n", enabled? STR_ENABLE: STR_DISABLE);
		if (enabled)
			showAutoPVCTbl();
		printf("-------------------------------------------------------------------------\n");
		printf("(1) Set                          (2) Add\n");
		printf("(3) Delete                       (4) Quit\n");
		if (!getInputOption( &choice, 1, 4))
			continue;
		switch (choice) {
		case 1://(1) Set         
			if (!check_access(SECURITY_SUPERUSER))
				break;
			min = 1; max = 2;
			if (0 == getInputUint("Auto-PVC Search (1) Disable (2) Enable: ", &num, &min, &max))
				continue;
			vChar = (unsigned char)(num - 1);
			if (!mib_set(MIB_ATM_VC_AUTOSEARCH, (void *)&vChar)) {
				printf("Set auto-pvc search error!");
				printWaitStr();
			}
			break;
			
		case 2://(2) Add
			if (!check_access(SECURITY_SUPERUSER))
				break;
			addAutoPVCPair();
			break;

		case 3://(3) Delete
			if (!check_access(SECURITY_SUPERUSER))
				break;
			num = mib_chain_total(MIB_AUTO_PVC_SEARCH_TBL); /* get chain record size */
			if (num == 0) {
				printf("Empty table!\n");
				printWaitStr();
				continue;
			}
			showAutoPVCTbl();
			min=1; max=num;
			if (getInputUint("Select the index to delete:", &intVal, &min, &max) == 0) {
				printf("Error selection!\n");
				printWaitStr();
				continue;
			}
			if(mib_chain_delete(MIB_AUTO_PVC_SEARCH_TBL, intVal - 1) != 1) {
				printf("Delete chain record error!");
				printWaitStr();
			}
			break;

		case 4://(4) Quit
			return;
		}
	}
}
#endif

void showChannelConfiguration()
{
   printf("\n");
   _wanConfList(1);
   printWaitStr();
}
void setChannelConfig()
{
	int snum;
	
	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("                           Channel Configuration                         \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("This page is used to configure the parameters for the channel operation  \n");
		printf("modes of your ADSL Modem/Router.                                         \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("(1) Add                          (2) Delete\n");
		printf("(3) Show                         (4) Connect\n");
		printf("(5) Disconnect                   (6) Auto-PVC Search\n");
		printf("(7) Quit\n");
		if (!getInputOption( &snum, 1, 7))
			continue;
		
		switch( snum)
		{
		case 1://(1) Add
			if (!check_access(SECURITY_SUPERUSER))
				break;
			addChannelConfiguration();
			break;
		case 2://(2) Delete
			if (!check_access(SECURITY_SUPERUSER))
				break;
			deleteChannelConfiguration();
			break;
		case 3://(3) Show
			showChannelConfiguration();
			break;
		case 4://(4) Connect
			connectChannelConfiguration();
			break;
		case 5://(5) Disconnect
			disconnectChannelConfiguration();
			break;
		case 6://(6) Auto-PVC Search
#if defined(AUTO_PVC_SEARCH_TR068_OAMPING) || defined(AUTO_PVC_SEARCH_PURE_OAMPING) || defined(AUTO_PVC_SEARCH_AUTOHUNT)
			setAutoPVCSearch();
#else
			printf("Not supported\n");
			printWaitStr();
#endif
			break;
		case 7://(7) Quit
			return;
		}//end switch, Channel Configuration
	}//end while, Channel Configuration
}
void modifyATMSettings()
{
	unsigned int pvcnum, num, min, max;
	//char vpi[4], vci[4];
	//int qos, pcr, cdvt, scr, mbs;
	int i, k,totalEntry;
	MIB_CE_ATM_VC_T Entry;
	MIB_CE_ATM_VC_T entry;   
	
	CLEAR;
	totalEntry = max = cli_atmVcList(); //_wanConfList(1); 
	printf("\n\n");
	
	min=0;   
	if (0 == getInputUint("PVC to modify: ", &pvcnum, &min, &max))
		return;   
	
	if (!mib_chain_get(MIB_ATM_VC_TBL, pvcnum, (void *)&Entry))
	{
		printf("Get chain record error!\n");
		return;
	}
	
	i = num;
	
	memcpy(&entry, &Entry, sizeof(entry));
	
	
	/*printf("VPI:");
	if ( getInputStr( vpi, sizeof(vpi)-1)==0) return;
	printf("VCI:");
	if ( getInputStr( vci, sizeof(vci)-1)==0) return;*/
	
	printf("Old QoS: %s\n", strQos[Entry.qos]);
	//printf("New QoS (1)UBR (2)CBR (3)rt-VBR (4)nrt-VBR :");
	min=1;max=4;
	if (0 == getInputUint("New QoS (1)UBR (2)CBR (3)nrt-VBR (4)rt-VBR: ", &num, &min, &max))
		return;
	entry.qos = num - 1;
	
	printf("Old PCR: %d\n", Entry.pcr);
	
	min=0;max=65535;
	if (0 == getInputUint("New PCR:", &num, &min, &max))
	   return;
	entry.pcr = num;
	
	max=0xffffffff;
	printf("Old CDVT: %d\n", Entry.cdvt);
	if (0 == getInputUint("New CDVT:", &num, &min, &max))
		return;
	entry.cdvt = num;
	
	if (entry.qos == 2 || entry.qos == 3)
	{   
		max=65535;
		printf("Old SCR: %d\n", Entry.scr);
		printf("New SCR:");
		if (0 == getInputUint("New SCR:", &num, &min, &max))
			return;
		entry.scr = num;
		
		printf("Old MBS: %d\n", Entry.mbs);
		if (0 == getInputUint("New MBS:", &num, &min, &max))
			return;
		entry.mbs = num;      
	}

#if defined(APPLY_CHANGE)
		// Take effect in real time
		stopConnection(&Entry);
#endif	
	memcpy(&Entry, &entry, sizeof(entry));
	mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, pvcnum);
	
	// synchronize this vc across all interfaces
	for (k=i+1; k<totalEntry; k++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, k, (void *)&Entry)) {
			if (Entry.vpi == entry.vpi && Entry.vci == entry.vci) {
				Entry.qos = entry.qos;
				Entry.pcr = entry.pcr;
				Entry.cdvt = entry.cdvt;
				Entry.scr = entry.scr;
				Entry.mbs = entry.mbs;
				mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, k);
			}
		}
	}
	
#if defined(APPLY_CHANGE)
		// Take effect in real time
		startConnection(&entry);
#endif
}
	
void setATMSettings()
{
	int snum;
	
	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("                           ATM Settings                                  \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("This page is used to configure the parameters for the ATM of your ADSL   \n");
		printf("Router. Here you may change the setting for VPI, VCI, QoS etc ...        \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n\n");
		printf("Current ATM VC Table:\n\n");
		cli_atmVcList();
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n\n");
		printf("\n(1) Modify                       (2) Quit\n");
		if (!getInputOption( &snum, 1, 2))
			continue;
		
		switch( snum)
		{
		case 1://(1) Modify
			if (!check_access(SECURITY_SUPERUSER))
				break;
			modifyATMSettings();
			break;
		case 2://(2) Quit
			return;
		}//end switch, ATM Settings
	}//end while, ATM Settings
}

//Jenny
void showADSLSettings(){
	unsigned char vChar;
	unsigned short mode;
	
	mib_get( MIB_ADSL_MODE, (void *)&mode);
	printf("ADSL modulation:\n");	// ADSL modulation
	if (mode & ADSL_MODE_GLITE)
		printf("\tG.Lite:\t  Enable\n");
	else
		printf("\tG.Lite:\t  Disable\n");
	if (mode & ADSL_MODE_GDMT)
		printf("\tG.Dmt:\t  Enable\n");
	else
		printf("\tG.Dmt:\t  Disable\n");
	if (mode & ADSL_MODE_T1413)
		printf("\tT1.413:\t  Enable\n");
	else
		printf("\tT1.413:\t  Disable\n");
	if (mode & ADSL_MODE_ADSL2)
		printf("\tADSL2:\t  Enable\n");
	else
		printf("\tADSL2:\t  Disable\n");
	if (mode & ADSL_MODE_ADSL2P)
		printf("\tADSL2+:\t  Enable\n");
	else
		printf("\tADSL2+:\t  Disable\n");
        printf("AnnexB Option:\n");     // AnnexB Option
        if (mode & ADSL_MODE_ANXB)
                printf("\tAnnexB:\t  Enable\n");
        else
	        printf("\tAnnexB:\t  Disable\n");
	printf("AnnexL Option:\n");	// AnnexL Option
	if (mode & ADSL_MODE_ANXL)
		printf("\tAnnexL:\t  Enable\n");
	else
		printf("\tAnnexL:\t  Disable\n");
	printf("AnnexM Option:\n");	// AnnexM Option
	if (mode & ADSL_MODE_ANXM)
		printf("\tAnnexM:\t  Enable\n");
	else
		printf("\tAnnexM:\t  Disable\n");
	
	mib_get( MIB_ADSL_OLR, (void *)&vChar);
	printf("ADSL Capability:\n");	// OLR type
	if (vChar & 1)
		printf("\tBitswap:  Enable\n");
	else
		printf("\tBitswap:  Disable\n");
	if (vChar & 2)
		printf("\tSRA:\t  Enable\n");
	else
		printf("\tSRA:\t  Disable\n");
}

//Jenny
void changeADSLSettings(){
	int min,max,sel, xmode;
	char_t olr;
	unsigned short mode;

	// annex B and annex A are incompatible,
	mib_get(MIB_ADSL_MODE, (void *)&mode);
	mode &= ADSL_MODE_ANXB;


	min=1;max=2;
	if (getInputUint("G.Lite (1) Enable (2) Disable:",&sel,&min,&max)==0) {
		printf("Invalid selection!\n");
		return;
	}
	if (sel==1)
		mode |= ADSL_MODE_GLITE;
	if (getInputUint("G.Dmt (1) Enable (2) Disable:",&sel,&min,&max)==0) {
		printf("Invalid selection!\n");
		return;
	}
	if (sel==1)
		mode |= ADSL_MODE_GDMT;
	if (getInputUint("T1.413 (1) Enable (2) Disable:",&sel,&min,&max)==0) {
		printf("Invalid selection!\n");
		return;
	}
	if (sel==1)
		mode |= ADSL_MODE_T1413;
	if (getInputUint("ADSL2 (1) Enable (2) Disable:",&sel,&min,&max)==0) {
		printf("Invalid selection!\n");
		return;
	}
	if (sel==1)
		mode |= ADSL_MODE_ADSL2;
	if (getInputUint("ADSL2+ (1) Enable (2) Disable:",&sel,&min,&max)==0) {
		printf("Invalid selection!\n");
		return;
	}
	if (sel==1)
		mode |= ADSL_MODE_ADSL2P;
		
		
	if (!(mode & ADSL_MODE_ANXB)) {
		if (getInputUint("AnnexL (1) Enable (2) Disable:",&sel,&min,&max)==0) {
			printf("Invalid selection!\n");
			return;
		}
		if (sel==1)
			mode |= ADSL_MODE_ANXL;
	
		if (getInputUint("AnnexM (1) Enable (2) Disable:",&sel,&min,&max)==0) {
			printf("Invalid selection!\n");
			return;
		}
		if (sel==1)
			mode |= ADSL_MODE_ANXM;
	}
	
	olr = 0;
	if (getInputUint("Bitswap (1) Enable (2) Disable:",&sel,&min,&max)==0) {
		printf("Invalid selection!\n");
		return;
	}
	if (sel==1)
		olr |= 1;
	if (getInputUint("SRA (1) Enable (2) Disable:",&sel,&min,&max)==0) {
		printf("Invalid selection!\n");
		return;
	}
	if (sel==1)
		olr |= 2;

	mib_set(MIB_ADSL_MODE, (void *)&mode);
	mib_set(MIB_ADSL_OLR, (void *)&olr);

	adsl_drv_get(RLCM_MODEM_RETRAIN, NULL, 0);
	xmode=0;
	if (mode & (ADSL_MODE_GDMT|ADSL_MODE_T1413))
		xmode |= 1;	// ADSL1
	if (mode & ADSL_MODE_ADSL2)
		xmode |= 2;	// ADSL2
	if (mode & ADSL_MODE_ADSL2P)
		xmode |= 4;	// ADSL2+
	adsl_drv_get(RLCM_SET_XDSL_MODE, (void *)&xmode, 4);

	xmode = mode & (ADSL_MODE_T1413|ADSL_MODE_GDMT);	//  1: ansi, 2: g.dmt, 8:g.lite
	adsl_drv_get(RLCM_SET_ADSL_MODE, (void *)&xmode, 4);	
	
	if (mode & ADSL_MODE_ANXL)	// Annex L
		xmode = 3; // Wide-Band & Narrow-Band Mode
	else
		xmode = 0;
	adsl_drv_get(RLCM_SET_ANNEX_L, (void *)&xmode, 4);
	
	if (mode & ADSL_MODE_ANXM)	// Annex M
		xmode = 1;
	else
		xmode = 0;
	adsl_drv_get(RLCM_SET_ANNEX_M, (void *)&xmode, 4);
	
	xmode = (int)olr;
	if (xmode == 2)// SRA (should include bitswap)
		xmode = 3;
	adsl_drv_get(RLCM_SET_OLR_TYPE, (void *)&xmode, 4);
}

//Jenny
void setADSLSettings()
{
	int min,max,sel;
	while (1)
   	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		printf("                      ADSL Settings             \n");
		MSG_LINE;
		showADSLSettings();
		min=1;max=2;
		if(getInputUint("Select index (1) Set ADSL Settings (2) Quit:",&sel,&min,&max)==0){
			printf("Invalid selection!\n");
			return;
		}
		switch(sel){
			case 1:
				if (!check_access(SECURITY_SUPERUSER))
					break;
				changeADSLSettings();
				break;
			case 2:
				return;
		}
	}
}

void setWANInterface()
{
	int snum;
	
	while (1)
	{
		CLEAR;
		printf("\n");
		MENU_LINE;
		//printf("------------------------------------------------------------\n");
		printf("                 (4) WAN Interface Menu                         \n");
		MENU_LINE;
		//printf("------------------------------------------------------------\n");
		printf("(1) Channel Config               (2) ATM Settings\n");
		printf("(3) ADSL Settings                (4) Quit\n");
		if (!getInputOption( &snum, 1, 4))
			continue;
		
		switch( snum)
		{
		case 1://(1) Channel Config
			setChannelConfig();
			break;
		case 2://(2) ATM Settings
			setATMSettings();
			break;
		case 3://(3) Channel Config
			setADSLSettings();
			break;
		case 4://(4) Quit
			return;
		default:
			printf("!! Invalid Selection !!\n");
		}//end switch, WAN Interface Menu
	}//end while, WAN Interface Menu
}
/*************************************************************************************************/

void setDHCPMode(){
	int snum;
	DHCP_TYPE_T dtmode;
	unsigned char vChar;
	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                           DHCP Mode Configuration                                   \n");
		printf("-------------------------------------------------------------------------\n");
		printf("Use this page to set and configure the Dynamic Host Protocol mode for your device. With DHCP, IP  \n");
		printf("addresses for your LAN are administered and distributed as needed by this device or an ISP device.  \n");
		printf("-------------------------------------------------------------------------\n");
		getSYS2Str(SYS_DHCP_MODE, strbuf);
		printf("DHCP Mode: %s\n",strbuf);
		printf("(1) Set                          (2) Quit\n\n");  
		if (!getInputOption( &snum, 1, 2))
			continue;
		
		switch( snum)
		{
			case 1://(1) Set DHCP Mode
				if (!check_access(SECURITY_SUPERUSER))
					break;
#ifdef ZTE_531B_BRIDGE_SC
				printf("Set DHCP Mode 0:None, 1:DHCP Server\n");
				if (!getInputOption( &snum, 0, 1))
					break;
				if ( snum==0 )
					dtmode = DHCP_LAN_NONE;
				else if ( snum == 1 )
					dtmode = DHCP_LAN_SERVER;
				else {
					printf("Invalid selection\n");
					break;
#else
				printf("Set DHCP Mode 0:None, 1:DHCP Relay, 2:DHCP Server\n");
				if (!getInputOption( &snum, 0, 2))
					break;
				if ( snum==0 )
					dtmode = DHCP_LAN_NONE;
				else if ( snum == 1 )
					dtmode = DHCP_LAN_RELAY;
				else if ( snum == 2 )
					dtmode = DHCP_LAN_SERVER;
				else {
					printf("Invalid selection\n");
					break;
#endif
				}
			
				vChar = (unsigned char) dtmode;
				if ( mib_set(MIB_DHCP_MODE, (void *)&vChar) == 0) {
					printf("Set DHCP Mode error!\n");
				}
#if defined(APPLY_CHANGE)
				// Take effect in real time
				restart_dhcp();
#endif
				break;
			
			case 2://(2) Quit
				return;
			default:
				printf("!! Invalid Selection !!\n");
		}//end switch, (7) Diagnostic Menu
	}
}

void addMacBase()
{
	MIB_CE_MAC_BASE_DHCP_T entry, Entry;
	int mibtotal, i, intVal;
	unsigned char macAddr[MAC_ADDR_LEN], mac[17], ip[15];
	struct in_addr assignedIP;
	unsigned long v1, v2;
	
	memset(&entry, 0x00, sizeof(entry));
	if (getInputString( "Host MAC Address(xx-xx-xx-xx-xx-xx): ", mac, sizeof(mac)+2) == 0)
		 return;
	//strcpy(entry.macAddr, mac);
	for (i=0; i<17; i++) {
		if ((i+1)%3 != 0)
			mac[i-(i+1)/3] = mac[i];
	}
	mac[12] = '\0';
	if (strlen(mac) != 12  || !string_to_hex(mac, entry.macAddr_Dhcp, 12) || !isValidMacAddr(entry.macAddr_Dhcp)) {
		printf(strInvdMACAddr);
		goto setErr_mac;
	}

	if (getInputString( "Assigned IP Address(xxx.xxx.xxx.xxx): ", ip, sizeof(ip)+2) == 0)
		return;
	//strcpy(entry.ipAddr, ip);
	if (!inet_aton(ip, (struct in_addr *)&entry.ipAddr_Dhcp ) || !isValidIpAddr(ip)) {
		printf("Invalid IP address %s.\n", ip);
		goto setErr_mac;
	}
	
	mibtotal = mib_chain_total(MIB_MAC_BASE_DHCP_TBL);
	for (i=0; i<mibtotal; i++) {
		mib_chain_get(MIB_MAC_BASE_DHCP_TBL, i, (void*)&Entry);
		v1 = *((unsigned long *)Entry.ipAddr_Dhcp);
		v2 = *((unsigned long *)entry.ipAddr_Dhcp);
		
		if ( (	Entry.macAddr_Dhcp[0]==entry.macAddr_Dhcp[0] && Entry.macAddr_Dhcp[1]==entry.macAddr_Dhcp[1] && Entry.macAddr_Dhcp[2]==entry.macAddr_Dhcp[2] &&
	     		Entry.macAddr_Dhcp[3]==entry.macAddr_Dhcp[3] && Entry.macAddr_Dhcp[4]==entry.macAddr_Dhcp[4] && Entry.macAddr_Dhcp[5]==entry.macAddr_Dhcp[5] ) || (v1==v2)  ) {		     	
			
			printf(strStaticipexist);
			goto setErr_mac;
		}
		
	}

	intVal = mib_chain_add(MIB_MAC_BASE_DHCP_TBL, (unsigned char*)&entry);		
	if (intVal == 0) {
		printf(strAddChainerror);
		goto setErr_mac;
	}	
	else if (intVal == -1) {
		printf(strTableFull);
		goto setErr_mac;
	}		
	return;	
setErr_mac:
	printWaitStr();
	return;
}

void showMACTable()
{
	unsigned int entryNum, i;
	MIB_CE_MAC_BASE_DHCP_T Entry;
	char macaddr[20];

	entryNum = mib_chain_total(MIB_MAC_BASE_DHCP_TBL);
	CLEAR;
	printf("\n");
	printf("MAC-Base Assignment Table:\n");
	printf("Idx  Host MAC Address         Assigned IP Address\n");
	printf("-------------------------------------------------------------------------\n");
	if (!entryNum)
		printf("No data!\n\n");

	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_MAC_BASE_DHCP_TBL, i, (void *)&Entry)) {
  			printf("Get chain record error!\n");
			return;
		}
		snprintf(macaddr, 18, "%02x-%02x-%02x-%02x-%02x-%02x",
				Entry.macAddr_Dhcp[0], Entry.macAddr_Dhcp[1],
				Entry.macAddr_Dhcp[2], Entry.macAddr_Dhcp[3],
				Entry.macAddr_Dhcp[4], Entry.macAddr_Dhcp[5]);
				
		printf("%-5d%-25s%s\n", i+1, macaddr, inet_ntoa(*((struct in_addr *)Entry.ipAddr_Dhcp)) );
	}
}

void setMacBase()
{
	int snum, num, i, intVal;
	unsigned int min, max;

	while (1) {
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("			Static IP Assignment Table							  \n");
		printf("-------------------------------------------------------------------------\n");
		printf("This page is used to configure the static IP base on MAC Address. \n");
		printf("You can assign/delete the static IP. The Host MAC Address, please \n");
		printf("input a string with hex number. Such as \"00-d0-59-c6-12-43\". \n");
		printf("The Assigned IP Address, please input a string with digit. Such as \n");
		printf("\"192.168.1.100\". \n");
		printf("-------------------------------------------------------------------------\n");

		printf("(1) Add                (2) Delete\n");
		printf("(3) Show               (4) Quit\n");
		if (!getInputOption(&snum, 1, 4))
			continue;

		switch (snum) {
			case 1://(1) Add
				if (!check_access(SECURITY_SUPERUSER))
					break;
				addMacBase();
#if defined(APPLY_CHANGE)
				// Take effect in real time
				restart_dhcp();
#endif
				break;

			case 2://(2) Delete
				if (!check_access(SECURITY_SUPERUSER))
					break;
				num = mib_chain_total(MIB_MAC_BASE_DHCP_TBL); /* get chain record size */
				if (num == 0) {
					printf("Empty table!\n");
					printWaitStr();
					continue;
				}
				showMACTable();
				min=1; max=num;
				if (getInputUint("Select the index to delete:", &intVal, &min, &max) == 0) {
					printf("Error selection!\n");
					printWaitStr();
					continue;
				}
				if(mib_chain_delete(MIB_MAC_BASE_DHCP_TBL, intVal-1) != 1) {
					printf("Delete chain record error!");
					printWaitStr();
				}
#if defined(APPLY_CHANGE)
				// Take effect in real time
				restart_dhcp();
#endif
				break;

			case 3://(3)Show
				showMACTable();
				printWaitStr();
				break;

			case 4://(4) Quit
				return;
		}
	}
}

void setDHCPServer()
{
	//char dhcpRangeStart[16], dhcpRangeEnd[16], dname[32];
	//int snum, dhcp, ltime;
	unsigned int choice,num,min,max;
	unsigned char dhcp, buffer[32], *ip, *mask;
	struct in_addr lanIp, lanMask, inGatewayIp;
	unsigned char ipprefix[16];
#ifdef DHCPS_POOL_COMPLETE_IP
	struct in_addr inPoolStart, inPoolEnd;
#endif
#ifdef DHCPS_DNS_OPTIONS
	char *pDns;
	unsigned char dnsopt;
	struct in_addr dns1, dns2, dns3;
#endif
	
	
// Kaohj
#ifdef SECONDARY_IP
	mib_get( MIB_ADSL_LAN_ENABLE_IP2, (void *)&buffer );
	if (buffer[0])
		mib_get(MIB_ADSL_LAN_DHCP_POOLUSE, (void *)buffer);
#else
	buffer[0] = 0;
#endif
	if (buffer[0] == 0) { // primary LAN
	if(!mib_get(MIB_ADSL_LAN_IP, (void *)&lanIp) || !mib_get(MIB_ADSL_LAN_SUBNET, (void *)&lanMask))
		return;
	}
	else { // secondary LAN
		if(!mib_get(MIB_ADSL_LAN_IP2, (void *)&lanIp) || !mib_get(MIB_ADSL_LAN_SUBNET2, (void *)&lanMask))
			return;
	}

// Kaohj
#ifndef DHCPS_POOL_COMPLETE_IP
	getSYS2Str(SYS_DHCPS_IPPOOL_PREFIX, ipprefix);
#endif
	
	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                           DHCP Server                                   \n");
		printf("-------------------------------------------------------------------------\n");
		printf("Enable the DHCP Server if you are using this device as a DHCP server.    \n");
		printf("This page lists the IP address pools available to hosts on your LAN.     \n");
		printf("The device distributes numbers in the pool to hosts on your network as   \n");
		printf("they request Internet access.                                            \n");
		printf("-------------------------------------------------------------------------\n");
		printf("LAN IP Address: %s\t", inet_ntoa(lanIp));
		printf("Subnet Mask: %s\n", inet_ntoa(lanMask));
		getSYS2Str(SYS_LAN_DHCP, strbuf);
		printf("DHCP Server: %s\n", strbuf);
		mib_get( MIB_ADSL_LAN_IP, (void *)buffer);
		// Kaohj
		#ifndef DHCPS_POOL_COMPLETE_IP
		printf("IP Pool Range: %s%s - ", ipprefix, getMibInfo(MIB_ADSL_LAN_CLIENT_START));
		printf("%s%s\n",  ipprefix, getMibInfo(MIB_ADSL_LAN_CLIENT_END));
		#else
		printf("IP Pool Range: %s - ", getMibInfo(MIB_DHCP_POOL_START));
		printf("%s\n", getMibInfo(MIB_DHCP_POOL_END));
		printf("Subnet Mask: %s\n", getMibInfo(MIB_DHCP_SUBNET_MASK));
		#endif
		printf("Max Lease Time: %s\n", getMibInfo(MIB_ADSL_LAN_DHCP_LEASE));
		printf("Domain Name: %s\n", getMibInfo(MIB_ADSL_LAN_DHCP_DOMAIN));
		printf("Gateway Address: %s\n", getMibInfo(MIB_ADSL_LAN_DHCP_GATEWAY));
		// Kaohj
#ifdef DHCPS_DNS_OPTIONS
		mib_get(MIB_DHCP_DNS_OPTION, (void *)&dnsopt);
		printf("DNS Options: %s\n", dnsopt==0?"Use DNS Relay":"Use Manual Setting");
		if (dnsopt == 1) { // manual setting
			printf("  DNS1: %s\n", getMibInfo(MIB_DHCPS_DNS1));
			pDns = getMibInfo(MIB_DHCPS_DNS2);
			if (!strcmp(pDns, "0.0.0.0"))
				printf("  DNS2: \n");
			else
				printf("  DNS2: %s\n", pDns);
			pDns = getMibInfo(MIB_DHCPS_DNS3);
			if (!strcmp(pDns, "0.0.0.0"))
				printf("  DNS3: \n");
			else
				printf("  DNS3: %s\n", pDns);
		}
#endif
		printf("(1) Set                          (2) Show Client\n");
		printf("(3) MAC-Base Assignment          (4) Quit\n");
 		if (!getInputOption(&choice, 1, 4))
			continue;
		
		switch(choice)		
		{
		case 1://(1) Set
			if (!check_access(SECURITY_SUPERUSER))
				break;
			{
				char prompt[64];
				unsigned char uStart, uEnd;
				unsigned int uLTime;
#ifdef DHCPS_POOL_COMPLETE_IP
				struct in_addr dhcp_mask;
#endif
				
				ip = (unsigned char *)&lanIp;
				dhcp = DHCP_SERVER;
				// Kaohj
				#ifndef DHCPS_POOL_COMPLETE_IP
				sprintf(prompt, "Start IP: %s", ipprefix);
				min=0;max=255;
				if (0 == getInputUint(prompt, &num, &min, &max))
					continue;
				uStart = (unsigned char)num;
				
				sprintf(prompt, "End IP: %s", ipprefix);
				min=0;max=255;
				if (0 == getInputUint(prompt, &num, &min, &max))
					continue;
				uEnd = (unsigned char)num;
				
				if (uEnd <= uStart) {
					printf("Invalid DHCP client range!\n");
					continue;
				}
				#else
				if (0 == getInputIpAddr("Start IP: ", &inPoolStart))
					continue;
				if (0 == getInputIpAddr("End IP: ", &inPoolEnd))
					continue;
				if (inPoolStart.s_addr >= inPoolEnd.s_addr) {
					printf("Invalid DHCP client range!\n");
					printWaitStr();
					continue;
				}
				if (0 == getInputIpMask("Subnet Mask: ", &dhcp_mask))
					continue;
				#endif
				
				min=0;max=0xffffffff;
				if (0 == getInputUint("Max Lease Time: ", &uLTime, &min, &max))
					continue;
				
				if (0 == getInputString("Domain Name: ", strbuf, sizeof(strbuf)-1))
					continue;
					
				// Kaohj
				#ifndef DHCPS_POOL_COMPLETE_IP
				if ( !mib_set(MIB_ADSL_LAN_CLIENT_START, (void *)&uStart)) {
					printf("Set DHCP client start range error!\n");
					return;
				}
				if ( !mib_set(MIB_ADSL_LAN_CLIENT_END, (void *)&uEnd)) {
					printf("Set DHCP client end range error!\n");
					return;
				}
				#else
				mib_set( MIB_DHCP_POOL_START, (void *)&inPoolStart);
				mib_set( MIB_DHCP_POOL_END, (void *)&inPoolEnd);
				mib_set( MIB_DHCP_SUBNET_MASK, (void *)&dhcp_mask);
				#endif
				if ( !mib_set(MIB_ADSL_LAN_DHCP_LEASE, (void *)&uLTime)) {
					printf("Set DHCP lease time error!\n");
					return;
				}
				if ( !mib_set(MIB_ADSL_LAN_DHCP_DOMAIN, (void *)strbuf)) {
					printf("Set DHCP Domain Name error!\n");
					return;
				}
				
				if (0 == getInputString("Gateway Address: ", strbuf, sizeof(strbuf)-1))
					continue;
				//printf("strbuf=%s\n", strbuf);
				
				if ( !inet_aton(strbuf, &inGatewayIp) ) {
					printf("CLI: Invalid DHCP Server Gateway Address value!");
					continue;
				}
				if ( !mib_set( MIB_ADSL_LAN_DHCP_GATEWAY, (void *)&inGatewayIp)) {
					printf("CLI: Set DHCP Server Gateway Address error!");
					continue;
				}				
				// Kaohj
				#ifdef DHCPS_DNS_OPTIONS
				min=1;max=2;
				if (0 == getInputUint("DNS options: (1)Use DNS Relay (2)Set DNS Manually: ", &num , &min, &max))
					continue;
				dnsopt = num-1;
				if (dnsopt == 1) { // set by manual
					dns1.s_addr=0xffffffff;
					dns2.s_addr=0xffffffff;
					dns3.s_addr=0xffffffff;
				
					if (0==getInputIpAddr("DNS1: ", &dns1))
						continue;
					if (0!=getInputIpAddr("DNS2: ", &dns2))
						getInputIpAddr("DNS3: ", &dns3);
				}
				#endif
				#ifdef DHCPS_DNS_OPTIONS
				mib_set(MIB_DHCP_DNS_OPTION, (void *)&dnsopt);
				mib_set(MIB_DHCPS_DNS1, (void *)&dns1);
				mib_set(MIB_DHCPS_DNS2, (void *)&dns2);
				mib_set(MIB_DHCPS_DNS3, (void *)&dns3);
				#endif
#if defined(APPLY_CHANGE)
				// Take effect in real time
				restart_dhcp();
#endif
			}
			
			break;
		case 2: //Show Client
			{
				struct stat status;
				int element=0, ret, pid;
				char ipAddr[40], macAddr[40], liveTime[80], *buf=NULL, *ptr;
				FILE *fp;

				printf("\n");
				printf("Active DHCP Client Table\n");
				printf("--------------------------------------------------------------------------\n");
				printf("This table shows the assigned IP address, MAC address and time expired for \n");
				printf("each DHCP leased client. \n");
				printf("--------------------------------------------------------------------------\n");
				printf("IP Address          MAC Address         Time Expired(s)\n");
				printf("--------------------------------------------------------------------------\n");
				// siganl DHCP server to update lease file
				pid = read_pid(DHCPSERVERPID);
				if (pid > 0)
					kill(pid, SIGUSR1);
				usleep(1000);
				if (stat(DHCPD_LEASE, &status) < 0)
					goto end_dhcp;

				// read DHCP server lease file
				buf = malloc(status.st_size);
				if (buf == NULL)
					goto end_dhcp;
				fp = fopen(DHCPD_LEASE, "r");
				if (fp == NULL)
					goto end_dhcp;
				fread(buf, 1, status.st_size, fp);
				fclose(fp);

				ptr = buf;
				while (1) {
					ret = getOneDhcpClient(&ptr, &status.st_size, ipAddr, macAddr, liveTime);
					if (ret < 0)
						break;
					if (ret == 0)
						continue;
					printf("%-20s%-20s%s\n", ipAddr, macAddr, liveTime);
					element ++;
				}
				if (!element)
					printf("No data!\n\n");
				end_dhcp:
					if (buf)
						free(buf);
					printWaitStr();
					break;
			}
		case 3: //(3) MAC-Base Assignment 
			setMacBase();
			break;
		case 4://(4) Quit
			return;
		}//end switch, WAN Interface Menu
	}//end while, WAN Interface Menu
}


void setDHCPRelay(){
	int snum;
	DHCP_TYPE_T dtmode;
	unsigned char vChar;
	char str[16];
	struct in_addr dhcps;

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                           DHCP Relay Configuration                                  \n");
		printf("-------------------------------------------------------------------------\n");
		printf("This page is used to configure the DHCP server ip addresses for DHCP Relay. \n");
		printf("-------------------------------------------------------------------------\n");
		printf("DHCP Server Address: %s\n",getMibInfo(MIB_ADSL_WAN_DHCPS));
		printf("(1) Set                          (2) Quit\n");  
		if (!getInputOption( &snum, 1, 2))
			continue;
		
		switch( snum)
		{
			case 1://(1) Set DHCP Mode
				
				if (!check_access(SECURITY_SUPERUSER))
					break;
				if (0 == getInputString("DHCP Server Address: ", str, 16))
					break;

				if ( !inet_aton(str, &dhcps) ) {
					printf("Invalid DHCPS address value!\n");
				}

				if ( !mib_set(MIB_ADSL_WAN_DHCPS, (void *)&dhcps)) {
	  				printf("Set DHCPS MIB error!");
				}
#if defined(APPLY_CHANGE)
				// Take effect in real time
				restart_dhcp();
#endif
				break;
			
			case 2://(2) Quit
				return;
			default:
				printf("!! Invalid Selection !!\n");
		}//end switch, (7) Diagnostic Menu
	}

}


void setDHCPPage(){
	int snum;

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                           DHCP Menu                                   \n");
		printf("-------------------------------------------------------------------------\n");
#ifdef ZTE_531B_BRIDGE_SC
		printf("(1) DHCP Mode				(2) DHCP Server\n");  
		printf("(3) Quit		\n");
#else
		printf("(1) DHCP Mode				(2) DHCP Server\n");  
		printf("(3) DHCP Relay				(4) Quit		\n");
#endif
		if (!getInputOption( &snum, 1, 4))
			continue;
		
		switch( snum)
		{
			case 1://(1) Ping
				setDHCPMode();
				break;
			case 2://(2) ATM Loopback
				setDHCPServer();
				break;
#ifndef ZTE_531B_BRIDGE_SC
			case 3:
				setDHCPRelay();
				break;
			case 4://(4) Quit
#else
			case 3:
#endif
				return;
			default:
				printf("!! Invalid Selection !!\n");
		}//end switch, (7) Diagnostic Menu
	}//end while, (7) Diagnostic Menu
}

void setDNSServer()
{ 
	unsigned int choice,num,min,max;
	unsigned char vChar;
	char *pDns;
	DNS_TYPE_T dns;
	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                           DNS Configuration                             \n");
		printf("-------------------------------------------------------------------------\n");
		printf("This page is used to configure the DNS server ip addresses for DNS Relay.\n");
		printf("-------------------------------------------------------------------------\n");
		printf("DNS Configuration:\n\n");
		getSYS2Str(SYS_DNS_MODE, strbuf);
		printf("  Mode: %s\n", strbuf);
		printf("  DNS1: %s\n", getMibInfo(MIB_ADSL_WAN_DNS1));
		pDns = getMibInfo(MIB_ADSL_WAN_DNS2);
		if (!strcmp(pDns, "0.0.0.0"))
			printf("  DNS2: \n");
		else
			printf("  DNS2: %s\n", pDns);
		pDns = getMibInfo(MIB_ADSL_WAN_DNS3);
		if (!strcmp(pDns, "0.0.0.0"))
			printf("  DNS3: \n");
		else
			printf("  DNS3: %s\n", pDns);
		
		printf("(1) Set                          (2) Quit\n");
		if (!getInputOption( &choice, 1, 2))
			continue;
		min=1;max=2;
		switch (choice)
		{
		case 1://(1) Set         
			if (!check_access(SECURITY_SUPERUSER))
				break;
			if (0 == getInputUint("(1)Attain DNS Automatically (2)Set DNS Manually: ", &num , &min, &max))
				continue;
			
			if(2==num)
			{
				struct in_addr dns1, dns2, dns3;
				dns = DNS_MANUAL;
				
				dns1.s_addr=0;
				dns2.s_addr=0;
				dns3.s_addr=0;
				
				if (0==getInputIpAddr("DNS1: ", &dns1))
					continue;
				if (0!=getInputIpAddr("DNS2: ", &dns2))
					getInputIpAddr("DNS3: ", &dns3);
				
				if ( 
					!mib_set(MIB_ADSL_WAN_DNS1, (void *)&dns1) ||
					!mib_set(MIB_ADSL_WAN_DNS2, (void *)&dns2) ||
					!mib_set(MIB_ADSL_WAN_DNS3, (void *)&dns3)
				) 
				{
					printf("Set DNS MIB error!");
					return;
				}
			} else {
				dns = DNS_AUTO;
			}
			
			vChar = (unsigned char) dns;
			if ( !mib_set(MIB_ADSL_WAN_DNS_MODE, (void *)&vChar)) {
				printf("Set DNS MIB error!");			   
			}
			printWaitStr();
			break;
			
		case 2://(2) Quit
			return;
		}//end switch, DNS Configuration
	}//end while, DNS Configuration
}

#ifdef CONFIG_USER_DDNS
static int getDDNS(MIB_CE_DDNS_Tp entry) {
	unsigned int num, min, max, sel;

	min=1;max=2;
	if (0 == getInputUint("DDNS provider: (1)DynDNS.org (2)TZO :", &sel, &min, &max))
		return 0;
	if (0 == getInputString("Hostname: ", entry->hostname, 34))
		return 0;
	if (0 == getInputUint("Status (1)Enable (2)Disable :", &num, &min, &max))
		return 0;
	if (num==1)
		entry->Enabled = 1;
	else
		entry->Enabled = 0;
	if (sel == 1) {
		strcpy(entry->provider, "dyndns");
		if (0 == getInputString("Username: ", entry->username, 34))
			return 0;
		if (0 == getInputString("Password: ", entry->password, 34))
			return 0;
	}
	else {
		strcpy(entry->provider, "tzo");
		if (0 == getInputString("Email: ", entry->username, 34))
			return 0;
		if (0 == getInputString("Key: ", entry->password, 34))
			return 0;
	}
	strcpy(entry->interface, "all");
	return 1;
}

void showDDNSTable()
{
	unsigned int entryNum, i;
	MIB_CE_DDNS_T Entry;

	entryNum = mib_chain_total(MIB_DDNS_TBL);

	CLEAR;
	printf("\n");
	printf("Dynamic DNS Table:\n");
	printf("Idx State   Hostname                           Service   Username\n");
   	printf("-------------------------------------------------------------------------\n");
   	if (!entryNum)
   		printf("No data!\n\n");

	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_DDNS_TBL, i, (void *)&Entry)) {
  			printf("Get chain record error!\n");
			return;
		}
		printf("%-4d%-8s%-35s%-10s%s\n", i+1, Entry.Enabled ? "Enable" : "Disable", Entry.hostname, Entry.provider, Entry.username);
	}
}

void setDDNS()
{
	unsigned int choice, min, max;
	DNS_TYPE_T dns;
	MIB_CE_DDNS_T entry, tmpEntry;
	int num, i, intVal;

	memset(&entry, 0x00, sizeof(entry));
	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                           Dynamic DNS                             \n");
		printf("-------------------------------------------------------------------------\n");
		printf("This page is used to configure the Dynamic DNS address from DynDNS.org \n");
		printf("or TZO. Here you can Add/Remove to configure Dynamic DNS. \n");
		printf("-------------------------------------------------------------------------\n");
		printf("(1) Add                          (2) Remove\n");
		printf("(3) Show                         (4) Quit\n");
		if (!getInputOption(&choice, 1, 4))
			continue;
		min=1; max=4;
		switch (choice) {
			case 1://(1) Add         
				if (!check_access(SECURITY_SUPERUSER))
					break;
				if (getDDNS(&entry) == 0)
					continue;
				
				num = mib_chain_total(MIB_DDNS_TBL); /* get chain record size */
				// check duplication
				for (i=0; i<num; i++) {
					mib_chain_get(MIB_DDNS_TBL, i, (void *)&tmpEntry);
					if (strcmp(entry.provider, tmpEntry.provider))
						continue;
					if (strcmp(entry.hostname, tmpEntry.hostname))
						continue;
					if (strcmp(entry.username, tmpEntry.username))
						continue;
					if (strcmp(entry.password, tmpEntry.password))
						continue;
					// entry duplication
					printf("Entry already exists!\n");
					goto setErr_ddns;
				}
				intVal = mib_chain_add(MIB_DDNS_TBL, (unsigned char*)&entry);
				if (intVal == 0)
					printf("Add Dynamic DNS chain record error.\n");
				else if (intVal == -1)
					printf(strTableFull);
				break;

		case 2: //(2) Delete
			if (!check_access(SECURITY_SUPERUSER))
				break;
			num = mib_chain_total(MIB_DDNS_TBL); /* get chain record size */
			if (num == 0) {
				printf("Empty table!\n");
				printWaitStr();
				continue;
			}
			showDDNSTable();
			min=1; max=num;
			if (getInputUint("Select the index to delete:", &intVal, &min, &max) == 0) {
				printf("Error selection!\n");
				printWaitStr();
				continue;
			}
			if(mib_chain_delete(MIB_DDNS_TBL, intVal-1) != 1)
				printf("Delete chain record error!");
			break;

		case 3: //(4)Show
			showDDNSTable();
			break;

		case 4://(4) Quit
			return;
		}//end switch, DNS Configuration
	setErr_ddns:
		printWaitStr();
		printf("\n");
	}//end while, DNS Configuration
}
#endif

void setDNS()
{
	int snum;

	while (1) {
		CLEAR;
		printf("\n");
		printf("-----------------------------------------------------------\n");
		printf("                         (2) DSN                                    \n");
		printf("-----------------------------------------------------------\n");
		printf("(1) DNS Server                  (2) Dynamic DNS\n");
		printf("(3) Quit\n");
		if (!getInputOption(&snum, 1, 3))
			continue;

		switch (snum) {
			case 1://(1) DNS Server 
				setDNSServer();
				break;
			case 2://(2) Dynamic DNS
#ifdef CONFIG_USER_DDNS
				setDDNS();
#else
				printf("Not supported\n");
				printWaitStr();
#endif
				break;
			case 3://(3) Quit
				return;
			default:
				printf("!! Invalid Selection !!\n");
		}
	}
}

/*************************************************************************************************/
void showIPPortFilteringTable()
{	
	unsigned int entryNum, i;
	MIB_CE_IP_PORT_FILTER_T Entry;
	char *dir, *ract;
	char *type, *sip, *dip;
	char sipaddr[20],dipaddr[20], sportRange[20], dportRange[20];
	unsigned char vChar;
	
	CLEAR;
	// Mason Yu
	printf("Default Action:\n");
	printf("----------------------------------------------\n");
	if ( !mib_get( MIB_IPF_OUT_ACTION, (void *)&vChar) )
		return;
	if (0 == vChar) 
		printf("Outgoing Default Action: Deny\n");
	else 
		printf("Outgoing Default Action: Allow\n");
		
		
	if ( !mib_get( MIB_IPF_IN_ACTION, (void *)&vChar) )
		return;
	if (0 == vChar)	
		printf("InComing Default Action: Deny\n");
	else
		printf("InComing Default Action: Allow\n");
		
	
	// IP/Filter Rule Table
	entryNum = mib_chain_total(MIB_IP_PORT_FILTER_TBL);

   	printf("\n");
   	printf("Current Filter Table:\n");
	//printf("Idx  Local IP Address  Port Range  Port Type  Protocol Rule  Action  Comment \n");
	printf("Idx Direction Protocol SrcAddress         SrcPort     DstAddress         DstPort     Rule_Action\n");
   	printf("--------------------------------------------------------------------------------\n");
   	if (!entryNum)
   		printf("No data!\n");

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_IP_PORT_FILTER_TBL, i, (void *)&Entry))
		{
  			printf("showIPPortFilteringTable: Get chain record error!\n");
			return;
		}
		
		if (Entry.dir == DIR_OUT)
			dir = "Outgoing";
		else
			dir = "Incoming";
		
		// Modified by Mason Yu for Block ICMP packet
		if ( Entry.protoType == PROTO_ICMP ) 
		{
			type = (char *)ARG_ICMP;
		}	
		else if ( Entry.protoType == PROTO_TCP )
			type = (char *)ARG_TCP;
		else
			type = (char *)ARG_UDP;

		sip = inet_ntoa(*((struct in_addr *)Entry.srcIp));
		//printf("Entry.srcIp=%x\n", *(unsigned long *)(Entry.srcIp));
		
		if ( !strcmp(sip, "0.0.0.0"))
			sip = (char *)BLANK;
		else {
			if (Entry.smaskbit==0)
				snprintf(sipaddr, 20, "%s", sip);
			else
				snprintf(sipaddr, 20, "%s/%d", sip, Entry.smaskbit);
			sip = sipaddr;
		}
		
		
		if ( Entry.srcPortFrom == 0)
			strcpy(sportRange, BLANK);
		else if ( Entry.srcPortFrom == Entry.srcPortTo )
			snprintf(sportRange, 20, "%d", Entry.srcPortFrom);
		else
			snprintf(sportRange, 20, "%d-%d", Entry.srcPortFrom, Entry.srcPortTo);		
      		
		dip = inet_ntoa(*((struct in_addr *)Entry.dstIp));
		if ( !strcmp(dip, "0.0.0.0"))
			dip = (char *)BLANK;
		else {
			if (Entry.dmaskbit==0)
				snprintf(dipaddr, 20, "%s", dip);
			else
				snprintf(dipaddr, 20, "%s/%d", dip, Entry.dmaskbit);
			dip = dipaddr;
		}

		if ( Entry.dstPortFrom == 0)
			strcpy(dportRange, BLANK);
		else if ( Entry.dstPortFrom == Entry.dstPortTo )
			snprintf(dportRange, 20, "%d", Entry.dstPortFrom);
		else
			snprintf(dportRange, 20, "%d-%d", Entry.dstPortFrom, Entry.dstPortTo);

		if ( Entry.action == 0 )
			ract = "Deny";
		else
			ract = "Allow";	
				
	 	printf("%-4d%-10s%-9s%-19s%-12s%-19s%-12s%s\n",i+1, dir, type, sip, sportRange, dip, dportRange, ract);			
	}



}

void filterAdd(unsigned char filterMode,struct in_addr *sip, struct in_addr *smask,unsigned short sfromPort,unsigned short stoPort,
                struct in_addr *dip, struct in_addr *dmask,unsigned short dfromPort,unsigned short dtoPort,
		unsigned char ptType,unsigned char prType){
	int mibTblId;
	unsigned int totalEntry;
	char * strVal;
	MIB_CE_IP_PORT_FILTER_T filterEntry;
	unsigned char noIP=1;
	char *strFrom, *strTo;
	unsigned long v1, v2, v3;
	struct in_addr *pAddr;	
	unsigned char *tmpip;
	int i, intVal;	
	
	unsigned long mask, mbit;
	
	
	mibTblId = MIB_IP_PORT_FILTER_TBL;
	totalEntry = mib_chain_total(mibTblId); /* get chain record size */

	memset(&filterEntry, 0x00, sizeof(filterEntry));
	
	// Protocol Type
	if ( prType==1 ) {
		filterEntry.protoType = PROTO_ICMP;
	}
	else if ( prType==2  ) {
		filterEntry.protoType = PROTO_TCP;
	}
	else if ( prType==3 ) {
		filterEntry.protoType = PROTO_UDP;
	}
	else {
		printf("Error! Invalid protocol type.\n");
		goto setErr_filter;
	}
	
	// Direction
	if ( ptType==1 ) {
		filterEntry.dir = DIR_OUT;
	}
	else if ( ptType==2  ) {
		filterEntry.dir = DIR_IN;
	}	
	else {
		printf("Error! Invalid direction.\n");
		goto setErr_filter;
	}
	
	
	// Src Host
	if (filterEntry.protoType != PROTO_TCP && filterEntry.protoType != PROTO_UDP)
		sfromPort = 0;
	
	pAddr = (struct in_addr *)filterEntry.srcIp;
	pAddr->s_addr = sip->s_addr;
	
	mbit=0;
			
	while (1) {
		if (smask->s_addr&0x80000000) {
			mbit++;
			smask->s_addr <<= 1;
		}
		else
			break;
	}
			
	filterEntry.smaskbit = mbit;
	filterEntry.srcPortFrom = (unsigned short)sfromPort;
	filterEntry.srcPortTo = (unsigned short)stoPort;
	
	if ( filterEntry.srcPortFrom  > filterEntry.srcPortTo ) {
		printf("Error! Invalid src port range.\n");
		goto setErr_filter;
	}
	
	// Dst Host
	if (filterEntry.protoType != PROTO_TCP && filterEntry.protoType != PROTO_UDP)
		dfromPort = 0;
	
	pAddr = (struct in_addr *)filterEntry.dstIp;
	pAddr->s_addr = dip->s_addr;
	
	mbit=0;
			
	while (1) {
		if (dmask->s_addr&0x80000000) {
			mbit++;
			dmask->s_addr <<= 1;
		}
		else
			break;
	}
			
	filterEntry.dmaskbit = mbit;
	
	filterEntry.dstPortFrom = (unsigned short)dfromPort;
	filterEntry.dstPortTo = (unsigned short)dtoPort;

	if ( filterEntry.dstPortFrom  > filterEntry.dstPortTo ) {
		printf("Error! Invalid Dst port range.\n");
		goto setErr_filter;
	}
	
	// Rule Action
	if (filterMode==1)
		filterEntry.action = 0;
	else if (filterMode==2)
		filterEntry.action = 1;
	else {
		printf("Invalid Rule Action value!\n");
		goto setErr_filter;
	}
	
	intVal = mib_chain_add(MIB_IP_PORT_FILTER_TBL, (unsigned char*)&filterEntry);
	if (intVal == 0) {
		printf(strAddChainerror);
		goto setErr_filter;
	}
	else if (intVal == -1) {
		printf(strTableFull);
		goto setErr_filter;
	}
	return;
	
	
setErr_filter:
	printf("set FW error!\n");
}

//tylo
void filterdel(){
	int mibTblId;
	unsigned int totalEntry;
	unsigned int i,index;
	int del,min,max;
	
	mibTblId = MIB_IP_PORT_FILTER_TBL;

	totalEntry = mib_chain_total(mibTblId); /* get chain record size */
	if(totalEntry==0){
		printf("Empty table!\n");
		return;	
	}
	min=1;max=2;
	if(getInputUint("Delete (1)One (2)All :",&del,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}

	if(del==2)
	{
		mib_chain_clear(mibTblId); /* clear chain record */
	}
	else if(del==1)
	{
		showIPPortFilteringTable();
		min=1;max=totalEntry+1;
		getInputUint( "Select the index to delete:",&index, &min,&max);
		if(index>totalEntry || index<=0){
			printf("Error selection!\n");
			return;
		}
		if(mib_chain_delete(mibTblId, index-1) != 1) {
			printf("Delete chain record error!");
		}
	}

}

void setIPPortFiltering()
{
   int min, max, sel;
   struct in_addr sip, smask, dip, dmask;
   unsigned char filterMode,ptType,prType;
   unsigned short sfromPort, stoPort, dfromPort, dtoPort;
   int snum, del;
   char inAct, outAct;
   unsigned char vChar;
   
   while (1)
   {
   		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                              IP/Port Filtering                          \n");
		printf("-------------------------------------------------------------------------\n");
      		printf("Entries in this table are used to restrict certain types of data packets \n");
      		printf("from your local network to Internet through the Gateway. Use of such     \n");
     		printf("filters can be helpful in securing or restricting your local network.    \n");
      		printf("Incoming Domain: LAN    Outgoing Domain: WAN                             \n");
		printf("-------------------------------------------------------------------------\n");		
      		//printf("Default Action :\n");
      		printf("(1) Set                    (2) Add\n");
      		printf("(3) Delete                 (4) Show IP/Port Filtering Default Action and Table\n");
      		printf("(5) Quit\n");
      		if (!getInputOption( &snum, 1, 5))
      			continue;
      		
      		switch( snum)
		{
		case 1://(1) Set
			if (!check_access(SECURITY_SUPERUSER))
				break;
			getSYS2Str(SYS_IPF_OUT_ACTION, strbuf);
         		printf("Old Outgoing Default Action: %s\n",strbuf);   
			min = 1; max = 2;
			if (0 == getInputUint("New Outgoing Default Action (1)Deny (2)Allow: ", &sel, &min, &max))
				continue;
			outAct = (char)sel-1;
			
			getSYS2Str(SYS_IPF_IN_ACTION, strbuf);
         		printf("Old Incoming Default Action : %s\n",strbuf);   
			if (0 == getInputUint("New Incoming Default Action (1)Deny (2)Allow: ", &sel, &min, &max))
				continue;
			inAct = (char)sel-1;
			
			if ( mib_set( MIB_IPF_OUT_ACTION, (void *)&outAct) == 0) {
				printf("Set Default Filter Action error!");
			}
			
			if ( mib_set( MIB_IPF_IN_ACTION, (void *)&inAct) == 0) {
				printf("Set Default Filter Action error!");
			}
#if defined(APPLY_CHANGE)
			// Take effect in real time
			restart_IPFilter_DMZ();
#endif			
         		break;
		case 2://(2) Add
			if (!check_access(SECURITY_SUPERUSER))
				break;
			// Rule Action
			min = 1; max = 2;
			if (0 == getInputUint("Rule Action (1)Deny (2)Allow: ", &sel, &min, &max))
				continue;
			filterMode = (char)sel;
			
         		// Src Host
			if (0 == getInputIpAddr("Src IP Address:", &sip))
				continue;
			
			//if (0 == getInputIpAddr("Src Subnet Mask:", &smask))
			if (0 == getInputIpMask("Src Subnet Mask:", &smask))
				continue;
			
			min = 0; max = 65535;
			if (0 == getInputUint("Src Port Range Start(1~65535, 0: ignore port range) Start:", &sel, &min, &max))
			    continue;
			sfromPort = (unsigned short)sel;
			
			if (0 != sfromPort)
			{
				min = sel;
				if (0 == getInputUint("Src Port Range(1~65535) End:", &sel, &min, &max))
					continue;
				stoPort = (unsigned short)sel;
			}
			else
			{
				stoPort = 0;
			}

         		// Dst Host
			if (0 == getInputIpAddr("Dst IP Address:", &dip))
				continue;
			
			//if (0 == getInputIpAddr("Dst Subnet Mask:", &dmask))
			if (0 == getInputIpMask("Dst Subnet Mask:", &dmask))
				continue;
			
			min = 0; max = 65535;
			if (0 == getInputUint("Dst Port Range Start(1~65535, 0: ignore port range) Start:", &sel, &min, &max))
			    continue;
			dfromPort = (unsigned short)sel;
			
			if (0 != dfromPort)
			{
				min = sel;
				if (0 == getInputUint("Dst Port Range(1~65535) End:", &sel, &min, &max))
					continue;
				dtoPort = (unsigned short)sel;
			}
			else
			{
				dtoPort = 0;
			}

         		// Direction
			min = 1; max = 2;
			if (0 == getInputUint("Direction (1)Outgoing (2)Incoming: ", &sel, &min, &max))
				continue;
			ptType = (unsigned char)sel;
			
         		// Protocol
			min = 1; max = 3;
			if (0 == getInputUint("Protocol (1)ICMP (2)TCP (3)UDP: ", &sel, &min, &max))
				continue;
			prType = (unsigned char)sel;
			
         		filterAdd(filterMode, &sip, &smask, sfromPort, stoPort,
                			      &dip, &dmask, dfromPort, dtoPort, ptType, prType);
#if defined(APPLY_CHANGE)
			// Take effect in real time
			restart_IPFilter_DMZ();
#endif			
         		break;
		case 3://(3) Delete
			if (!check_access(SECURITY_SUPERUSER))
				break;
        		filterdel();
#if defined(APPLY_CHANGE)
			// Take effect in real time
			restart_IPFilter_DMZ();
#endif
			printWaitStr();
         		break;
      		case 4://(4) Show IP/Port Filtering Table
         		showIPPortFilteringTable();
			printWaitStr();
         		break;
      		case 5://(5) Quit
         		return;
      		}//end switch, IP/Port Filtering
   }//end while, IP/Port Filtering
}

#ifdef MAC_FILTER
//tylo
void showIMACFilteringTable(){
	int nBytesSent=0;

	unsigned int entryNum, i;
	MIB_CE_MAC_FILTER_T Entry;
	char ract[6];
	char tmpbuf[100], tmpbuf2[100];
	char *dir, *outAct, *inAct;
	unsigned char vChar;
	
	
	entryNum = mib_chain_total(MIB_MAC_FILTER_TBL);
	
	CLEAR;
	printf("\n");
	printf("Outgoing and Incoming Default Action :\n");
	printf("-------------------------------------------------------------------------\n");
	if ( !mib_get( MIB_MACF_OUT_ACTION, (void *)&vChar) )
		return;
	if (0 == vChar) 
		outAct = "Deny";
	else 
		outAct = "Allow";	
	printf("Outgoing : %s\n", outAct);	
	
	
	if ( !mib_get( MIB_MACF_IN_ACTION, (void *)&vChar) )
		return;
	if (0 == vChar) 
		inAct = "Deny";
	else 
		inAct = "Allow";	
	printf("Incoming : %s\n", inAct);
	
	
   	printf("\n");
   	printf("Current Filter Table:\n");
	printf("Idx  Direction  Src MAC Address     Dst MAC Address      Rule Action  \n");
   	printf("-------------------------------------------------------------------------\n");
   	if (!entryNum)
   		printf("No data!\n\n");

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_MAC_FILTER_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return;
		}
		
		if (Entry.dir == DIR_OUT)
			dir = "Outgoing";
		else
			dir = "Incoming";
			
		if ( Entry.action == 0 )
			strcpy(ract,"Deny");
		else
			strcpy(ract,"Allow");
		
		if (   Entry.srcMac[0]==0 && Entry.srcMac[1]==0
		    && Entry.srcMac[2]==0 && Entry.srcMac[3]==0
		    && Entry.srcMac[4]==0 && Entry.srcMac[5]==0 ) {
			strcpy(tmpbuf, "------");
		}else {		
			snprintf(tmpbuf, 100, ("%02x:%02x:%02x:%02x:%02x:%02x"),
				Entry.srcMac[0], Entry.srcMac[1], Entry.srcMac[2],
				Entry.srcMac[3], Entry.srcMac[4], Entry.srcMac[5]);
		}
		
		if (   Entry.dstMac[0]==0 && Entry.dstMac[1]==0
		    && Entry.dstMac[2]==0 && Entry.dstMac[3]==0
		    && Entry.dstMac[4]==0 && Entry.dstMac[5]==0 ) {		    	
			strcpy(tmpbuf2, "------");	
		}else {
			snprintf(tmpbuf2, 100, ("%02x:%02x:%02x:%02x:%02x:%02x"),
				Entry.dstMac[0], Entry.dstMac[1], Entry.dstMac[2],
				Entry.dstMac[3], Entry.dstMac[4], Entry.dstMac[5]);	
		}
		
		printf("%-5d%-11s%-20s%-21s%-10s\n",i+1,dir,tmpbuf,tmpbuf2,ract);

	}


}

void MACFilterAdd(unsigned char act,unsigned char *src_macaddr,unsigned char *dst_macaddr,unsigned char dir){
	MIB_CE_MAC_FILTER_T macEntry;
	int mibTblId,i, intVal;
	unsigned int totalEntry;
	memset(&macEntry, 0x00, sizeof(macEntry));
	mibTblId = MIB_MAC_FILTER_TBL;


	totalEntry = mib_chain_total(mibTblId); /* get chain record size */


	if (act==1)
		macEntry.action = 0;
	else if (act==2)
		macEntry.action = 1;
	else {
		printf("Invalid Rule Action value!\n");
		goto setErr_filter;
	}


	if (dir==1)
		macEntry.dir = 0;
	else if (dir==2)
		macEntry.dir = 1;
	else {
		printf("Invalid Direction value!\n");
		goto setErr_filter;
	}


	if ( !src_macaddr[0] && !dst_macaddr[0]) {
		printf("Error! No mac address to set.\n");
		goto setErr_filter;
	}
	
	if (src_macaddr[0]) {
		if (strlen(src_macaddr) != 12 || !string_to_hex(src_macaddr, macEntry.srcMac, 12) || !isValidMacAddr(macEntry.srcMac)) {
			printf("Error! Invalid Src MAC address.\n");
			goto setErr_filter;
		}
	}
	
	if (dst_macaddr[0]) {
		if (strlen(dst_macaddr) != 12 || !string_to_hex(dst_macaddr, macEntry.dstMac, 12) || !isValidMacAddr(macEntry.dstMac)) {
			printf("Error! Invalid Dst MAC address.\n");
			goto setErr_filter;
		}	
	}


	intVal = mib_chain_add(MIB_MAC_FILTER_TBL, (unsigned char*)&macEntry);
	if (intVal == 0) {
		printf(strAddChainerror);
		goto setErr_filter;
	}
	else if (intVal == -1) {
		printf(strTableFull);
		goto setErr_filter;
	}
	return;

setErr_filter:
	printWaitStr();
	return;

}

//tylo
void MACFilterDel(){
	int mibTblId;
	unsigned int totalEntry;
	unsigned int i,index;
	int del,min,max;
	
	mibTblId = MIB_MAC_FILTER_TBL;

	totalEntry = mib_chain_total(mibTblId); /* get chain record size */
	if(totalEntry==0){
		printf("Empty table!\n");
		return;	
	}
	min=1;max=2;
	if(getInputUint("Delete (1)One (2)All :",&del,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}

	if(del==2)
	{
		mib_chain_clear(mibTblId); /* clear chain record */
	}
	else if(del==1)
	{
		showIMACFilteringTable();
		min=1;max=totalEntry+1;
		getInputUint( "Select the index to delete:",&index, &min,&max);
		if(index>totalEntry || index<=0){
			printf("Error selection!\n");
			return;
		}
		if(mib_chain_delete(mibTblId, index-1) != 1) {
			printf("Delete chain record error!");
		}
	}


}


//tylo
void setMACFiltering()
{
   unsigned char src_mac[12], dst_mac[12];
   int snum,  del;
   unsigned char filterMode, outgoingMode, incomingMode, dir;
   unsigned char vChar;
   char inAct, outAct;
   int min, max, sel;

   while (1)
   {
   	CLEAR;
	printf("\n");
	printf("-------------------------------------------------------------------------\n");
	printf("                              MAC Filtering                              \n");
	printf("-------------------------------------------------------------------------\n");
      	printf("Entries in this table are used to restrict certain types of data packets \n");
      	printf("from your local network to Internet through the Gateway. Use of such     \n");
      	printf("filters can be helpful in securing or restricting your local network.    \n");
	printf("Incoming Domain: LAN    Outgoing Domain: WAN                             \n");
	printf("-------------------------------------------------------------------------\n");
	printf("Current Filter Table:\n");
	printf("MAC Address Rule Action Comment Select \n");
	printf("-------------------------------------------------------------------------\n");

      	printf("(1) Set Default Action           (2) Add\n");
      	printf("(3) Delete			 (4) Show\n");
      	printf("(5) Quit\n");
      	if (!getInputOption( &snum, 1, 5))
      		continue;
      	
      switch( snum)
	{
	case 1://(1) Set Default Action	
		if (!check_access(SECURITY_SUPERUSER))
			break;
		min = 1; max = 2;
		if (0 == getInputUint("Outgoing (1) Deny (2) Allow: ", &sel, &min, &max))
			continue;
		outAct = (char)sel - 1;
		if (0 == getInputUint("Incoming (1) Deny (2) Allow: ", &sel, &min, &max))
			continue;
		inAct = (char)sel - 1;
		
		mib_set( MIB_MACF_OUT_ACTION, (void *)&outAct);
		outAct = !outAct;
		mib_set( MIB_ETH_MAC_CTRL, (void *)&outAct);
		mib_set( MIB_WLAN_MAC_CTRL, (void *)&outAct);	
		
		
		mib_set( MIB_MACF_IN_ACTION, (void *)&inAct);
#if defined(APPLY_CHANGE)
		// Take effect in real time
		setupMacFilter();
#endif
		break;
		
	case 2://(2) Add
		if (!check_access(SECURITY_SUPERUSER))
			break;
		min = 1; max = 2;
		if (0 == getInputUint("Rule Action (1)Deny (2)Allow: ", &sel, &min, &max))
			continue;
		filterMode = (unsigned char)sel;
		
		if (0 == getInputUint("Direction (1)Outgoing (2)Incoming: ", &sel, &min, &max))
			continue;
		dir = (unsigned char)sel;
		
		if ( getInputString( "Source MAC Address:",src_mac, sizeof(src_mac)+2)==0);//continue;
		if ( getInputString( "Destination MAC Address:",dst_mac, sizeof(dst_mac)+2)==0);//continue;		
         	
         	MACFilterAdd(filterMode, src_mac, dst_mac, dir);
#if defined(APPLY_CHANGE)
		// Take effect in real time
		setupMacFilter();
#endif
        	break;
        
	case 3://(3) Delete
		if (!check_access(SECURITY_SUPERUSER))
			break;
         	MACFilterDel();
#if defined(APPLY_CHANGE)
		// Take effect in real time
		setupMacFilter();
#endif
		printWaitStr();
        	break;
        
        case 4://(4)Show
         	showIMACFilteringTable();
		printWaitStr();
         	break;
         	
      	case 5://(5) Quit
        	return;
        
      }//end switch, MAC Filtering
   }//end while, MAC Filtering
}
#endif // of MAC_FILTER

//tylo
void showForwardingTable()
{
	unsigned int entryNum, i;
	MIB_CE_PORT_FW_T Entry;
	char	type[8], portRange[20], *ip, szLocalIP[20], *pszStatus, szRemotHost[20], szPublicPortRange[20], szIfName[16];
	
	CLEAR;
	printf("\n");
	printf("Current Port Forwarding Table\n");
	printf("%-5s%-18s%-10s%-15s%-15s%-9s%-16s%-15s%-10s\n", "Idx",  "Local IP Address",  "Protocol",  
           "Local Port", "Comment", "Enable", "Remote Host", "Public Port", "Interface");
	printf("-------------------------------------------------------------------------------------------------------\n");

	entryNum = mib_chain_total(MIB_PORT_FW_TBL);
	if (!entryNum)
		printf("No data!\n");

	for (i = 0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_PORT_FW_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return;
		}

		ip = inet_ntoa(*((struct in_addr *)Entry.ipAddr));
		strcpy(szLocalIP, ip);
		if ( !strcmp(szLocalIP, "0.0.0.0"))
		{
			strcpy(szLocalIP ,"----");
		}

		if ( Entry.protoType == PROTO_UDPTCP )
		{
			strcpy(type,"TCP+UDP");
		}
		else if ( Entry.protoType == PROTO_TCP )
		{
			strcpy(type,"TCP");
		}
		else
		{
			strcpy(type,"UDP");
		}

		if ( Entry.fromPort == 0)
		{
			strcpy(portRange, "----");
		}
		else if ( Entry.fromPort == Entry.toPort )
		{
			snprintf(portRange, 20, "%d", Entry.fromPort);
		}
		else
		{
			snprintf(portRange, 20, "%d-%d", Entry.fromPort, Entry.toPort);
		}

        if (Entry.enable)
        {
            pszStatus = (char *)IF_ENABLE;
        }
        else
        {
            pszStatus = (char *)IF_DISABLE;
        }

		ip = inet_ntoa(*((struct in_addr *)Entry.remotehost));
        strcpy(szRemotHost, ip);        
		if ( !strcmp(szRemotHost, "0.0.0.0"))
		{
			strcpy(szRemotHost ,"----");
		}

		if ( Entry.externalfromport == 0)
		{
			strcpy(szPublicPortRange, "----");
		}
		else if ( Entry.externalfromport == Entry.externaltoport )
		{
			snprintf(szPublicPortRange, 20, "%d", Entry.externalfromport);
		}
		else
		{
			snprintf(szPublicPortRange, 20, "%d-%d", Entry.externalfromport, Entry.externaltoport);
		}

		if (Entry.ifIndex == 0xff)
		{
			strcpy( szIfName, "----" );
		}
        else if((Entry.ifIndex & 0xf0) == 0xf0)
		{
			sprintf(szIfName, "vc%u", VC_INDEX(Entry.ifIndex));
		}
        else
		{
			sprintf(szIfName, "ppp%u", PPP_INDEX(Entry.ifIndex));
		}
        
		printf("%-5d%-18s%-10s%-15s%-15s%-9s%-16s%-15s%-10s\n", i + 1, szLocalIP, type, portRange, 
            Entry.comment, pszStatus, szRemotHost, szPublicPortRange, szIfName);
	}

	printWaitStr();
	return;
}

//tylo
void addPortFW(struct in_addr *ip, unsigned short fromPort,unsigned short toPort,
                    unsigned char protocol,unsigned char *comment, int iWanInf, int enable, 
                    char *pszRemoteHost, unsigned short usExternalFromPort, unsigned short usExternalToPort)
{

    char_t *submitUrl, *strAddPort, *strDelPort, *strVal, *strDelAllPort;
    char_t *strIp, *strFrom, *strTo;
    char tmpBuf[100];
    int intVal;
    unsigned int entryNum, i, uiCommentLen;
    MIB_CE_PORT_FW_T entry;
    struct in_addr curIpAddr, curSubnet;
    unsigned long v1, v2, v3;
    unsigned char vChar;
    struct in_addr *paddr;

    memset(  &entry, 0, sizeof(MIB_CE_PORT_FW_T) );

    paddr = (struct in_addr *)&entry.ipAddr;
    paddr->s_addr = ip->s_addr;
    mib_get( MIB_ADSL_LAN_IP,  (void *)&curIpAddr);
    mib_get( MIB_ADSL_LAN_SUBNET,  (void *)&curSubnet);

    v1 = *((unsigned long *)entry.ipAddr);
    v2 = *((unsigned long *)&curIpAddr);
    v3 = *((unsigned long *)&curSubnet);

    if ( (v1 & v3) != (v2 & v3) ) {
    	printf("Invalid IP address! It should be set within the current subnet.\n");
	printWaitStr();
    	return;
    }

    entry.fromPort = fromPort;
    entry.toPort = toPort;
    if ( entry.fromPort  > entry.toPort ) 
    {
    	printf("Error! Invalid local port range.\n");
	printWaitStr();
    	return;
    }

    if (0 == protocol)
    {
    	entry.protoType = PROTO_UDPTCP;
    }
    else if (1== protocol)
    {
    	entry.protoType = PROTO_TCP;
    }
    else if (2 == protocol)
    {
    	entry.protoType = PROTO_UDP;
    }
    else 
    {
    	printf("Error! Invalid protocol type.");
	printWaitStr();
    	return;
    }

    if (comment[0]) 
    {
        uiCommentLen = strlen(comment);
        
    	if (uiCommentLen > COMMENT_LEN-1) 
        {
    		printf("Error! Comment length too long.");
		printWaitStr();
    		return;
    	}
        
    	strcpy(entry.comment, comment);
    }

    entry.ifIndex = (unsigned char)iWanInf; /* iWanInf has been checked. */
    entry.enable  = enable;
    entry.externalfromport = usExternalFromPort;
    entry.externaltoport = usExternalToPort;
    if ( entry.externalfromport  > entry.externaltoport ) 
    {
    	printf("Error! Invalid public port range.\n");
	printWaitStr();
    	return;
    }
    
    if (pszRemoteHost[0])
    {
        if (1 != inet_aton(pszRemoteHost, (struct in_addr *)&entry.remotehost))
        {
            printf("Error! Incorrect remote host ip addr.");
	    printWaitStr();
            return;
        }
    }


	intVal = mib_chain_add(MIB_PORT_FW_TBL, (unsigned char*)&entry);
	if (intVal == 0) {
		printf(strAddChainerror);
		printWaitStr();
		return;
	}
	else if (intVal == -1) {
		printf(strTableFull);
		printWaitStr();
    	return;
    }    
    return;
}

///ql_xu add: for virtual server
#ifdef ZTE_GENERAL_ROUTER_SC
void showVrtsvrTable()
{
	unsigned int entryNum, i;
	MIB_CE_VTL_SVR_T Entry;
	char type[8], portRange[20], *ip;
	char svrName[16];
	unsigned short svrPort;
	
	printf("\n");
	printf("Current Virtual Server Table\n");
	printf("Idx  Local_Svr_Name Server_IP_Addr  Protocol  Wan_Port_Range  Svr_Port\n");
	printf("----------------------------------------------------------------------\n");
	
	entryNum = mib_chain_total(MIB_VIRTUAL_SVR_TBL);
	
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_VIRTUAL_SVR_TBL, i, (void *)&Entry))
		{
			printf("Get chain record error!\n");
			return -1;
		}

		strcpy(svrName, Entry.svrName);
		
		ip = inet_ntoa(*((struct in_addr *)Entry.svrIpAddr));
	
		if ( Entry.protoType == PROTO_UDPTCP || Entry.protoType == PROTO_NONE)
			strcpy(type,"TCP+UDP");
		else if ( Entry.protoType == PROTO_TCP )
			strcpy(type,"TCP");
		else
			strcpy(type,"UDP");

		svrPort = Entry.svrStartPort;
	
		if ( Entry.wanStartPort == Entry.wanEndPort )
			snprintf(portRange, 20, "%d", Entry.wanStartPort);
		else
			snprintf(portRange, 20, "%d-%d", Entry.wanStartPort, Entry.wanEndPort);
	
		printf("%-5d%-15s%-16s%-10s%-16s%-5d",i+1,svrName,ip,type,portRange,svrPort);
		printf("\n");
	}
}

void addVirtSvr(MIB_CE_VTL_SVR_T entry)
{
	struct in_addr curIpAddr, secondIpAddr, curSubnet, secondSubnet;
	unsigned long v1, v2, v3, v4, v5;
	char enable;
	MIB_CE_ACC_T Entry;
	MIB_CE_VTL_SVR_T vrtEntry;
	int totalEntry;
	int entryIdx;

	totalEntry = mib_chain_total(MIB_ACC_TBL);
	for (entryIdx=0; entryIdx<totalEntry; entryIdx++) {
		if(!mib_chain_get(MIB_ACC_TBL, entryIdx, &Entry))
		{
			printf("get MIB_ACC_TBL error!\n");
			return 0;
		}
		if (Entry.telnet==0x01) {
			if ((entry.wanStartPort <= Entry.telnet_port) && (entry.wanEndPort >= Entry.telnet_port)) {
				printf("wan port range is conflict with remote access telnet port %d!\n", Entry.telnet_port);
				return;
			}
		}
		if (Entry.ftp==0x01) {
			if ((entry.wanStartPort <= Entry.ftp_port) && (entry.wanEndPort >= Entry.ftp_port)) {
				printf("wan port range is conflict with remote access ftp port %d!\n", Entry.ftp_port);
				return;
			}
		}
		if (Entry.web==0x01) {
			if ((entry.wanStartPort <= Entry.web_port) && (entry.wanEndPort >= Entry.web_port)) {
				printf("wan port range is conflict with remote access web port %d!\n", Entry.web_port);
				return;
			}
		}
	}

	totalEntry = mib_chain_total(MIB_VIRTUAL_SVR_TBL);
	for (entryIdx=0; entryIdx<totalEntry; entryIdx++) {
		if(!mib_chain_get(MIB_VIRTUAL_SVR_TBL, entryIdx, &vrtEntry))
		{
			printf("get MIB_VIRTUAL_SVR_TBL error!\n");
			return 0;
		}
		if( ((entry.wanStartPort>=vrtEntry.wanStartPort) && (entry.wanStartPort<=vrtEntry.wanEndPort)) ||
			((entry.wanEndPort>=vrtEntry.wanStartPort) && (entry.wanEndPort<=vrtEntry.wanEndPort)) ||
			((entry.wanStartPort<=vrtEntry.wanStartPort) && (entry.wanEndPort>=vrtEntry.wanEndPort)) )
		{
			printf("wan port range has been used by %s(%d-%d) server£¬please reset it!", vrtEntry.svrName, 
				vrtEntry.wanStartPort, vrtEntry.wanEndPort);
			return false;
		}
	}
	
	mib_get( MIB_ADSL_LAN_IP,  (void *)&curIpAddr );
	mib_get( MIB_ADSL_LAN_SUBNET,  (void *)&curSubnet );
	mib_get( MIB_ADSL_LAN_ENABLE_IP2, (void *)&enable );
	mib_get( MIB_ADSL_LAN_IP2, (void *)&secondIpAddr );
	mib_get( MIB_ADSL_LAN_SUBNET2, (void *)&secondSubnet);
		
	v1 = *((unsigned long *)entry.svrIpAddr);
	v2 = *((unsigned long *)&curIpAddr);
	v3 = *((unsigned long *)&curSubnet);
	v4 = *((unsigned long *)&secondIpAddr);
	v5 = *((unsigned long *)&secondSubnet);
	
	if ((v1 & v3) != (v2 & v3)) {
		if (enable) {
			if ((v1 & v5) == (v4 & v5)) {
				goto check_OK;
			}
		}
		printf("Invalid IP address! It should be set within the current subnet.\n");
		return;
	}

check_OK:
	int intVal = mib_chain_add(MIB_VIRTUAL_SVR_TBL, (unsigned char*)&entry);
	if (intVal == 0) {
		printf(strAddChainerror);
	}
	else if (intVal == -1)
		printf(strTableFull);
	return;
}

void delVrtSvr()
{
	int mibTblId;
	unsigned int totalEntry;
	unsigned int i,index;
	int del,min,max;
		
	mibTblId = MIB_VIRTUAL_SVR_TBL;
	
	totalEntry = mib_chain_total(mibTblId); /* get chain record size */
	if(totalEntry==0){
		printf("Empty table!\n");
		return; 
	}
	min=1;max=2;
	if(getInputUint("Delete (1)One (2)All :",&del,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}
	
	if(del==2)
	{
		mib_chain_clear(mibTblId); /* clear chain record */
	}
	else if(del==1)
	{
		showVrtsvrTable();
		min=1;
		max=totalEntry+1;
		getInputUint( "\nSelect the index to delete:",&index, &min,&max);
		if(index>totalEntry || index<=0){
			printf("Error selection!\n");
			return;
		}
		if(mib_chain_delete(mibTblId, index-1) != 1) {
			printf("Delete chain record error!");
		}
	}
	
}

#endif

//tylo
void delPortFW(){
	int mibTblId;
	unsigned int totalEntry;
	unsigned int i,index;
	int del,min,max;
	
	mibTblId = MIB_PORT_FW_TBL;

	totalEntry = mib_chain_total(mibTblId); /* get chain record size */
	if(totalEntry==0){
		printf("Empty table!\n");
		return;	
	}
	min=1;max=2;
	if(getInputUint("Delete (1)One (2)All :",&del,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}

	if(del==2)
	{
		for (i=0; i<totalEntry; i++) 
        {
			MIB_CE_PORT_FW_T CheckEntry;
	
			if (!mib_chain_get(MIB_PORT_FW_TBL, i, (void *)&CheckEntry)) 
            {
    			printf("Error get port fw rule!\n");
    			return;
			}			
		}
        
		mib_chain_clear(mibTblId); /* clear chain record */
	}
	else if(del==1)
	{
		showForwardingTable();
		min=1;max=totalEntry+1;
		getInputUint( "Select the index to delete:",&index, &min,&max);
		if(index>totalEntry || index<=0){
			printf("Error selection!\n");
			return;
		}
        	
		if(mib_chain_delete(mibTblId, index - 1) != 1)
        	{
			printf("Delete chain record error!");
		}
	}
    
    	return;
}

int getInputWanIfIndex(int *piIfIndex)
{
	unsigned int uiEntryNum, i, uiIfNum = 1, uiMinIfSeq, uiIfSeq;
	MIB_CE_ATM_VC_T stEntry;
    unsigned char *pucIfIndex;
    int retVal;
	
    printf("Wan interface:\n(1) any\n");
    
	uiEntryNum = mib_chain_total(MIB_ATM_VC_TBL);
    pucIfIndex = (unsigned char *)malloc(uiEntryNum + 1);
    if (NULL == pucIfIndex)
    {
        return 0;
    }

    *pucIfIndex = (unsigned char)255;
	for (i = 0; i < uiEntryNum; i++) 
    {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&stEntry))
		{
            printf("Can't get wan interface!\n");
            free(pucIfIndex);
			return 0;
		}
		
		if ((0 == stEntry.enable) || (ADSL_BR1483 == stEntry.cmode))
		{
			continue;
		}
			
		if (PPP_INDEX(stEntry.ifIndex) != 0x0f)
		{	// PPP interface
		    printf("(%u) ppp%u\n", uiIfNum + 1, PPP_INDEX(stEntry.ifIndex));
		}
		else
		{	// vc interface
		    printf("(%u) vc%u\n", uiIfNum + 1, VC_INDEX(stEntry.ifIndex));
		}

        *(pucIfIndex + uiIfNum) = (unsigned char)(stEntry.ifIndex);
        uiIfNum++;
	}

    uiMinIfSeq = 1;
	retVal=getTypedInputDefault(INPUT_TYPE_UINT,"Select Wan Interface[(1) any]: ",&uiIfSeq,&uiMinIfSeq,&uiIfNum);
	if(retVal==0){
		printf("Invalid value!\n");
		printWaitStr();
		free(pucIfIndex);
		return 0;
	} else if(retVal==-2)
		uiIfSeq=1; // default value

    *piIfIndex = *(pucIfIndex + uiIfSeq - 1);
    free(pucIfIndex);
    return 1;
}

//tylo
void setPortForwarding()
{
   char ip[16], comment[20], szRemoteHost[16], cIsInputInvalid;
   int snum, del, enabled;
   unsigned short fromPort, toPort, usExternalFromPort, usExternalToPort;
   int iWanIfIndex, iRuleEnable;
#ifdef ZTE_GENERAL_ROUTER_SC
   MIB_CE_VTL_SVR_T entry;
   int nameLen;
#endif
   unsigned char protocol;
   int min,max,sel, retVal;
   struct in_addr remoteIp;
   char pbuf[64];
   unsigned char vChar;

   while (1)
   {
   		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
#ifdef ZTE_GENERAL_ROUTER_SC
		printf("                              Virtual Server                             \n");
#else
		printf("                              Port Forwarding                            \n");
#endif
		printf("-------------------------------------------------------------------------\n");
      	printf("Entries in this table allow you to automatically redirect common network \n");
      	printf("services to a specific machine behind the NAT firewall. These settings   \n");
      	printf("are only necessary if you wish to host some sort of server like a web    \n");
      	printf("server or mail server on the private local network behind your Gateway's \n");
      	printf("NAT firewall.                                                            \n");
		printf("-------------------------------------------------------------------------\n");
#ifdef ZTE_GENERAL_ROUTER_SC
      	printf("Virtual Server:\n");
      	printf("(1) Add                          (2) Delete\n");
      	printf("(3) Show Forwarding Table        (4) Quit\n");
#else
      	printf("Port Forwarding:\n");
		printf("(1) Set 				(2) Add\n");
		printf("(3) Delete				(4) Show Forwarding Table\n");
		printf("(5) Quit\n");
#endif
      	if (!getInputOption( &snum, 1, 5))
      		continue;
      		
      	switch( snum)
		{
#ifndef ZTE_GENERAL_ROUTER_SC
		case 1://(1) Set
			if (!check_access(SECURITY_SUPERUSER))
				break;
			getSYS2Str(SYS_DEFAULT_PORT_FW_ACTION, strbuf);
			printf("Old Port Forwarding: %s\n",strbuf);       
			min=1;max=2;
			if(getInputUint("New Port Forwarding (1)Enable (2)Disable:",&sel,&min,&max)==0){
				printf("Invalid selection!\n");
				continue;
			}

			if ( sel==1)
				vChar = 1;
			else
				vChar = 0;
			if ( mib_set( MIB_PORT_FW_ENABLE, (void *)&vChar) == 0) {
				printf("Set enabled flag error!\n");
			}
#if defined(APPLY_CHANGE)
			setupPortFW();
#endif
         		break;
		case 2://(2) Add
			if (!check_access(SECURITY_SUPERUSER))
				break;
			if (0 == getInputIpAddr("Local IP Address: ", &ip))
				continue;
                	
                	//min = 0;
                	min = 1;
                	max = 65535;
			retVal=getTypedInputDefault(INPUT_TYPE_UINT,"Local Port Range(1~65535) Start[None]: ",&sel,&min,&max);
			if(retVal==0){
				printf("Invalid value!\n");
				printWaitStr();
				continue;
			} else if(retVal==-2)
				fromPort=0;
			else
				fromPort=(unsigned short)sel;
                	
                	if (0 != fromPort)
                	{
                	    	min = fromPort;
                	    	sprintf(pbuf, "Local Port Range(%d~65535) End[%d]: ", fromPort, fromPort);
				retVal=getTypedInputDefault(INPUT_TYPE_UINT,pbuf,&sel,&min,&max);
				if(retVal==0){
					printf("Invalid value!\n");
					printWaitStr();
					continue;
				} else if(retVal==-2)
					toPort=fromPort;
				else
					toPort=(unsigned short)sel;
                	}
                	else
                	{
                	    toPort = 0;
                	}
                	
                	min = 1;
                	max = 3;
                	if (0 == getInputUint("Protocol (1)Both (2)TCP (3)UDP :", &sel, &min, &max))
                	{
                	    continue;
                	}
                	protocol = (unsigned char)(sel - 1);
                	
			retVal=getTypedInputDefault(INPUT_TYPE_STRING,"Comment[None]: ",comment,sizeof(comment)-1,0);
                	
                	if (0 == getInputWanIfIndex(&iWanIfIndex))
                	{
                	    continue;
                	}
                	
                	min = 1;
                	max = 2;
                	if (0 == getInputUint("Rule (1)Enable (2)Disable:", &sel, &min, &max))
                	{
                	    continue;
                	}
					if (1 == sel)
					{
						iRuleEnable = 1;
					}
					else
					{
						iRuleEnable = 0;
					}
                	
                	szRemoteHost[0] = '\0';
			retVal=getTypedInputDefault(INPUT_TYPE_IPADDR,"Remote IP Address[None]: ",&remoteIp,0,0);
			if(retVal==0){
				printf("Invalid value!\n");
				printWaitStr();
				continue;
			} else if(retVal==1)
				strcpy(szRemoteHost, inet_ntoa(remoteIp));
                	
                	//min = 0;
                	min = 1;
                	max = 65535;
			retVal=getTypedInputDefault(INPUT_TYPE_UINT,"Public Port Range(1~65535) Start[None]: ",&sel,&min,&max);
			if(retVal==0){
				printf("Invalid value!\n");
				printWaitStr();
				continue;
			} else if(retVal==-2)
				usExternalFromPort=0;
			else
				usExternalFromPort=(unsigned short)sel;
                	
                	if (0 != usExternalFromPort)
                	{
                	    	min = usExternalFromPort;
                	    	sprintf(pbuf, "Public Port Range(%d~65535) End[%d]: ", usExternalFromPort, usExternalFromPort);
				retVal=getTypedInputDefault(INPUT_TYPE_UINT,pbuf,&sel,&min,&max);
				if(retVal==0){
					printf("Invalid value!\n");
					printWaitStr();
					continue;
				} else if(retVal==-2)
					usExternalToPort=usExternalFromPort;
				else
					usExternalToPort=(unsigned short)sel;
                	}
                	else
                	{
                	    usExternalToPort = 0;
                	}
                	
         		addPortFW(&ip, fromPort, toPort, protocol, comment, iWanIfIndex, iRuleEnable, 
                	  szRemoteHost, usExternalFromPort, usExternalToPort);
#if defined(APPLY_CHANGE)
                	setupPortFW();
#endif
         		break;
		case 3://(3) Delete
			if (!check_access(SECURITY_SUPERUSER))
				break;
			delPortFW();
#if defined(APPLY_CHANGE)
			setupPortFW();
#endif
			printWaitStr();
         		break;
      		case 4://(4) Show Forwarding Table
         		showForwardingTable();
         		break;
      		case 5://(5) Quit
         		return;
#else
		case 1://(1) Add
			printf("Server Name:");
			if ( (nameLen=getInputStr(entry.svrName, 16))==0 ) {
				printf("\nname length error: should be 1~16\n");
				break;
			}
			entry.svrName[nameLen-1]='\0';
			
			printf("Server Ip Address:");
			if ( getInputStr(ip, sizeof(ip)-1)==0 )
				break;
			inet_aton(ip, (struct in_addr *)&entry.svrIpAddr);
			
			printf("Server Port:");
			entry.svrStartPort = entry.svrEndPort = getInputNum();
			if ( entry.svrStartPort<1 || entry.svrStartPort>65535 ) {
				printf("\nserver port set error!\n");
				break;
			}
			
			printf("WAN Port Range Start:");
			entry.wanStartPort = getInputNum();
			if ( entry.wanStartPort<1 || entry.wanStartPort>65535 ) {
				printf("\nwan start port set error!\n");
				break;
			}
			
			printf("WAN Port Range End:");
			entry.wanEndPort = getInputNum();
			if ( entry.wanEndPort<1 || entry.wanEndPort>65535 || entry.wanEndPort<entry.wanStartPort ) {
				printf("\nwan port range error!\n");
				break;
			}
			
			printf("Protocol (1)Both (2)TCP (3)UDP :");
			protocol = getInputNum();
			if ( protocol==0 || protocol>3 )
				break;
			switch(protocol)
			{
				case 1:
					entry.protoType = PROTO_UDPTCP;
					break;
				case 2:
					entry.protoType = PROTO_TCP;
					break;
				case 3:
					entry.protoType = PROTO_UDP;
					break;
			}
			
			addVirtSvr(entry);
         		break;
		case 2://(2) Delete
			delVrtSvr();
         		break;
      		case 3://(3) ShowVirtual Server Table
         		showVrtsvrTable();
         		break;
      		case 4://(4) Quit
         		return;
#endif
		}//end switch, Port Forwarding
   }//end while, Port Forwarding
}

#ifdef URL_BLOCKING_SUPPORT
//Jenny
void showURLBlkTable(int mibTblId){
	unsigned int entryNum, i;
	MIB_CE_URL_FQDN_T EntryFQDN;
	MIB_CE_KEYWD_FILTER_T EntryKeyword;

	entryNum = mib_chain_total(mibTblId);

	CLEAR;
	printf("\n");
	if (mibTblId == MIB_URL_FQDN_TBL) {
   		printf("URL Blocking Table:\n");
		printf("Idx     FQDN\n");
	}
	else if (mibTblId == MIB_KEYWD_FILTER_TBL) {
		printf("Keyword Filtering Table:\n");
		printf("Idx 	Filtered Keyword\n");
	}
   	printf("-------------------------------------------------------------------------\n");
   	if (!entryNum)
   		printf("No data!\n\n");

	if (mibTblId == MIB_URL_FQDN_TBL) {
		for (i=0; i<entryNum; i++) {
			if (!mib_chain_get(mibTblId, i, (void *)&EntryFQDN)) {
	  			printf("Get chain record error!\n");
				return;
			}
			printf("%-8d%s\n", i+1, EntryFQDN.fqdn);
		}
	}
	else if (mibTblId == MIB_KEYWD_FILTER_TBL) {
		for (i=0; i<entryNum; i++) {
			if (!mib_chain_get(mibTblId, i, (void *)&EntryKeyword)) {
	  			printf("Get chain record error!\n");
				return;
			}
			printf("%-8d%s\n", i+1, EntryKeyword.keyword);
		}
	}
}

//Jenny
void URLBlkDel(int mibTblId)
{
	unsigned int totalEntry;
	unsigned int i, index;
	int del, min, max;
	
	totalEntry = mib_chain_total(mibTblId); /* get chain record size */
	if (totalEntry==0) {
		printf("Empty table!\n");
		return;	
	}
	min=1; max=2;
	if (getInputUint("Delete (1)One (2)All :", &del, &min, &max) == 0) {
		printf("Invalid selection!\n");
		return;
	}

	if (del == 2)
		mib_chain_clear(mibTblId); /* clear chain record */
	else if (del == 1) {
		showURLBlkTable(mibTblId);
		min=1; max=totalEntry+1;
		getInputUint("Select the index to delete:", &index, &min, &max);
		if (index > totalEntry || index <= 0) {
			printf("Error selection!\n");
			return;
		}
		if(mib_chain_delete(mibTblId, index-1) != 1)
			printf("Delete chain record error!");
	}
}

//Jenny
void setURL(int mibTblId)
{
	MIB_CE_URL_FQDN_T entryFQDN;
	MIB_CE_KEYWD_FILTER_T entryKeyword;
	int snum, i, intVal;
	unsigned int totalEntry;

	while (1) {
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		if (mibTblId == MIB_URL_FQDN_TBL)
			printf("            FQDN Blocking                             \n");
		else if (mibTblId == MIB_KEYWD_FILTER_TBL)
			printf("            Keyword Filtering                             \n");
		printf("-------------------------------------------------------------------------\n");

		printf("(1) Add				(2) Delete\n");
		printf("(3) Show			(4) Quit\n");
		if (!getInputOption(&snum, 1, 4))
			continue;

		switch (snum) {
			case 1:	//(1) Add
				if (!check_access(SECURITY_SUPERUSER))
					break;
				totalEntry = mib_chain_total(mibTblId); /* get chain record size */
				if (mibTblId == MIB_URL_FQDN_TBL) {
					if (getInputString("FQDN: ", strbuf, MAX_URL_LENGTH - 1) == 0) ; //continue;
					for (i = 0 ; i < totalEntry; i++) {  
						if (!mib_chain_get(mibTblId, i, (void *)&entryFQDN)) {
							printf(errGetEntry);
							printf("\n");
							return;
						}
						if (!strcmp(entryFQDN.fqdn, strbuf)){
							printf(TstrUrlExist);
							printf("\n");
							return;
						}
					}
					strcpy(entryFQDN.fqdn, strbuf);
					intVal = mib_chain_add(mibTblId, (unsigned char*)&entryFQDN);
				}
				else if (mibTblId == MIB_KEYWD_FILTER_TBL) {
					if (getInputString("Keyword: ", strbuf, MAX_KEYWD_LENGTH - 1) == 0) ;
					for (i = 0 ; i < totalEntry; i++) {  
						if (!mib_chain_get(mibTblId, i, (void *)&entryKeyword)) {
							printf(errGetEntry);
							printf("\n");
							return;
						}
						if (!strcmp(entryKeyword.keyword, strbuf)){
							printf(TstrUrlExist);
							printf("\n");
							return;
						}
					}
					strcpy(entryKeyword.keyword, strbuf);
					intVal = mib_chain_add(mibTblId, (unsigned char*)&entryKeyword);
				}
				if (intVal == 0) {
					printf("Add URL chain record error.\n");
					return;
				}		
				else if (intVal == -1) {
					printf(strTableFull);
					return;
				}		
				restart_urlblocking();
				break;

			case 2:	//(2) Delete
				if (!check_access(SECURITY_SUPERUSER))
					break;
				URLBlkDel(mibTblId);
				printWaitStr();
				break;

			case 3:	//(3)Show
				showURLBlkTable(mibTblId);
				printWaitStr();
				break;

			case 4:	//(4) Quit
				return;
		}
	}
}

// Jenny
void setURLBlocking()
{
	   int snum, min, max, sel;
	   unsigned char vChar;
	
	   while (1)
	   {
			CLEAR;
			printf("\n");
			printf("-------------------------------------------------------------------------\n");
			printf("			  URL Blocking							 \n");
			printf("-------------------------------------------------------------------------\n");
			printf("This page is used to configure the Blocked FQDN(Such as tw.yahoo.com)\n");
			printf("and filtered keyword. Here you can add/delete FQDN and filtered keyword.\n");
			printf("-------------------------------------------------------------------------\n");
			printf("URL Blocking:\n");
			printf("(1) Set 					 (2) FQDN Blocking\n");
			printf("(3) Keyword Filtering				 (4) Quit\n");
			if (!getInputOption(&snum, 1, 4))
				continue;
				
			switch (snum) {
				case 1://(1) Set
					if (!check_access(SECURITY_SUPERUSER))
						break;
					getSYS2Str(SYS_DEFAULT_URL_BLK_ACTION, strbuf);
					printf("Old URL Blocking: %s\n",strbuf); 	  
					min=1; max=2;
					if (getInputUint("New URL Blocking (1)Enable (2)Disable:", &sel, &min, &max) == 0) {
						printf("Invalid selection!\n");
						continue;
					}
					if (sel == 1)
						vChar = 1;
					else
						vChar = 0;
					if (mib_set(MIB_URL_CAPABILITY, (void *)&vChar) == 0)
						printf("Set enabled flag error!\n");
					break;

				case 2://(2) FQDN Blocking
					if (!check_access(SECURITY_SUPERUSER))
						break;
					setURL(MIB_URL_FQDN_TBL);
					break;

				case 3://(3) Keyword Filtering
					if (!check_access(SECURITY_SUPERUSER))
						break;
					setURL(MIB_KEYWD_FILTER_TBL);
					break;

				case 4://(4) Quit
					return;
			}//end switch, Port Forwarding
	   }//end while, Port Forwarding
}
#endif

#ifdef DOMAIN_BLOCKING_SUPPORT
//Jenny
void showDomainBlkTable(){
	unsigned int entryNum, i;
	MIB_CE_DOMAIN_BLOCKING_T Entry;

	entryNum = mib_chain_total(MIB_DOMAIN_BLOCKING_TBL);

	CLEAR;
	printf("\n");
	printf("Domain Block Table:\n");
	printf("Idx     Domain\n");
   	printf("-------------------------------------------------------------------------\n");
   	if (!entryNum)
   		printf("No data!\n\n");

	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_DOMAIN_BLOCKING_TBL, i, (void *)&Entry)) {
  			printf("Get chain record error!\n");
			return;
		}
		printf("%-8d%s\n", i+1, Entry.domain);
	}
}

// Jenny
void DomainBlkDel()
{
	unsigned int totalEntry;
	unsigned int i, index;
	int del, min, max;
	
	totalEntry = mib_chain_total(MIB_DOMAIN_BLOCKING_TBL); /* get chain record size */
	if (totalEntry==0) {
		printf("Empty table!\n");
		return;
	}
	min=1; max=2;
	if (getInputUint("Delete (1)One (2)All :", &del, &min, &max) == 0) {
		printf("Invalid selection!\n");
		return;
	}

	if (del == 2)
		mib_chain_clear(MIB_DOMAIN_BLOCKING_TBL); /* clear chain record */
	else if (del == 1) {
		showDomainBlkTable();
		min=1; max=totalEntry+1;
		getInputUint("Select the index to delete:", &index, &min, &max);
		if (index > totalEntry || index <= 0) {
			printf("Error selection!\n");
			return;
		}
		if(mib_chain_delete(MIB_DOMAIN_BLOCKING_TBL, index-1) != 1)
			printf("Delete chain record error!");
	}
}

// Jenny
void setDomainBlocking()
{
	MIB_CE_DOMAIN_BLOCKING_T entry;
	int snum, i, intVal, min, max, sel;
	unsigned int totalEntry;
	unsigned char vChar;

	while (1) {
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
			printf("            Domain Blocking                             \n");
		printf("-------------------------------------------------------------------------\n");

		printf("(1) Set			(2) Add\n");
		printf("(3) Delete		(4) Show\n");
		printf("(5) Quit\n");
		if (!getInputOption(&snum, 1, 5))
			continue;

		switch (snum) {
			case 1://(1) Set
				if (!check_access(SECURITY_SUPERUSER))
					break;
				getSYS2Str(SYS_DEFAULT_DOMAIN_BLK_ACTION, strbuf);
				printf("Old Domain Blocking: %s\n",strbuf);	  
				min=1; max=2;
				if (getInputUint("New Domain Blocking (1)Enable (2)Disable:", &sel, &min, &max) == 0) {
					printf("Invalid selection!\n");
					continue;
				}
				if (sel == 1)
					vChar = 1;
				else
					vChar = 0;
				if (mib_set(MIB_DOMAINBLK_CAPABILITY, (void *)&vChar) == 0)
					printf("Set enabled flag error!\n");
#if defined(APPLY_CHANGE)
				restart_domainBLK();
#endif
				break;
			
			case 2:	//(2) Add
				if (!check_access(SECURITY_SUPERUSER))
					break;
				if (getInputString("Domain: ", strbuf, MAX_DOMAIN_LENGTH - 1) == 0)
					break;
				totalEntry = mib_chain_total(MIB_DOMAIN_BLOCKING_TBL);
				for (i = 0 ; i < totalEntry; i++) {  
					if (!mib_chain_get(MIB_DOMAIN_BLOCKING_TBL, i, (void *)&entry)) {
						printf(errGetEntry);
						goto setErr_domainblk;
					}
					if (!strcmp(entry.domain, strbuf)){
						printf(strMACInList);
						goto setErr_domainblk;
					}
				}
				strcpy(entry.domain, strbuf);
				intVal = mib_chain_add(MIB_DOMAIN_BLOCKING_TBL, (unsigned char*)&entry);
				if (intVal == 0) {
					printf("Add Domain Blocking chain record error.\n");
					return;
				}
				else if (intVal == -1) {
					printf(strTableFull);
					return;
				}
#if defined(APPLY_CHANGE)
				restart_domainBLK();
#endif
				break;

			case 3:	//(3) Delete
				if (!check_access(SECURITY_SUPERUSER))
					break;
				DomainBlkDel();
				//printWaitStr();
#if defined(APPLY_CHANGE)
				restart_domainBLK();
#endif
				break;

			case 4:	//(4)Show
				showDomainBlkTable();
				//printWaitStr();
				break;

			case 5:	//(5) Quit
				return;
		}
		setErr_domainblk:
			printWaitStr();
			printf("\n");
	}
}
#endif

//tylo
/*
 * Return: 0: fail, 1: successful
 */
int dmzset(int enable, char *ip){
	char vChar;
	struct in_addr ipAddr, curIpAddr, curSubnet, secondIpAddr, secondSubnet;
	unsigned long v1, v2, v3;
#ifdef SECONDARY_IP
	unsigned long v4, v5;
	char ip2Enable;
#endif
	
	vChar = (char)(enable-1);

	if(vChar==1) {
		memcpy(&ipAddr,ip,4);
		//inet_aton(ip, (struct in_addr *)&ipAddr);
		mib_get( MIB_ADSL_LAN_IP,  (void *)&curIpAddr);
		mib_get( MIB_ADSL_LAN_SUBNET,  (void *)&curSubnet);
#ifdef SECONDARY_IP
		mib_get( MIB_ADSL_LAN_ENABLE_IP2, (void *)&ip2Enable );
		mib_get( MIB_ADSL_LAN_IP2, (void *)&secondIpAddr );
		mib_get( MIB_ADSL_LAN_SUBNET2, (void *)&secondSubnet);
#endif
		
		v1 = *((unsigned long *)&ipAddr);
		v2 = *((unsigned long *)&curIpAddr);
		v3 = *((unsigned long *)&curSubnet);
#ifdef SECONDARY_IP
		v4 = *((unsigned long *)&secondIpAddr);
		v5 = *((unsigned long *)&secondSubnet);
#endif
		
		if (v1) {
			if ( (((v1 & v3) != (v2 & v3))
#ifdef SECONDARY_IP
			 && !ip2Enable) ||
			( enable && ((v1 & v5) != (v4 & v5)) && ((v1 & v3) != (v2 & v3))
#endif
			 )) {
				printf("Invalid IP address! It should be set within the current subnet.\n");
				goto setErr_dmz;
			}
		}
		if ( mib_set(MIB_DMZ_IP, (void *)&ipAddr) == 0) {
			printf("Set DMZ MIB error!");
			goto setErr_dmz;
		}
	} // of if (vChar == 1)
	if ( mib_set(MIB_DMZ_ENABLE, (void *)&vChar) == 0) {
		printf("Set enabled flag error!");
		goto setErr_dmz;
	}

	return 1;
setErr_dmz:
	printf("Set DMZ error!\n");
	return 0;
}

//tylo
void setDMZ()
{
   char dmzip[16], state;
   int snum, dmz;
   int min, max, sel;

   while (1)
   {
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                                   DMZ                                   \n");
		printf("-------------------------------------------------------------------------\n");
      		printf("A Demilitarized Zone is used to provide Internet services without        \n");
      		printf("sacrificing unauthorized access to its local private network. Typically, \n"); 
      		printf("the DMZ host contains devices accessible to Internet traffic, such as Web\n");
      		printf("(HTTP ) servers, FTP servers, SMTP (e-mail) servers and DNS servers.     \n");
		printf("-------------------------------------------------------------------------\n");
      		/*if( DMZ is enable)
		   printf("DMZ Host IP Address: \n");
      		*/
		mib_get(MIB_DMZ_ENABLE, (void *)&state);
      		printf("\nDMZ: %s\n", state? STR_ENABLE:STR_DISABLE);
      		printf("DMZ IP: %s\n\n", getMibInfo(MIB_DMZ_IP));

      		printf("(1) Set                          (2) Quit\n\n");
      		if (!getInputOption( &snum, 1, 2))
      			continue;
      		
      		switch( snum)
		{
			case 1://(1) Set
				if (!check_access(SECURITY_SUPERUSER))
					break;
				min = 1; max = 2;
				if (0 == getInputUint("New DMZ (1)Disable (2)Enable :", &dmz, &min, &max))
					continue;
				
         			if( dmz==2)
         			{
           				if (0 == getInputIpAddr("DMZ Host IP Address:", (struct in_addr *)&dmzip[0]))
               					continue;
         			}
         			if (!dmzset(dmz,dmzip))
					printWaitStr();
#if defined(APPLY_CHANGE)
				setupDMZ();
#endif
         			break;
      			case 2://(2) Quit
         			return;
      		}//end switch, DMZ
   }//end while, DMZ
}


typedef struct menu_item{
	char  display_name[128];
	void (*fn)(void);
} MENU_ITEM;


#ifdef TCP_UDP_CONN_LIMIT
void showconnlimitTable()
{
	unsigned int entryNum, i,unum ,tnum;
	MIB_CE_TCP_UDP_CONN_LIMIT_T 	Entry;
	char *dir, *ract;
	char ipaddr[20];
	struct in_addr 			dest;
	unsigned char vChar;
	
	CLEAR;

	mib_get( MIB_CONNLIMIT_ENABLE, (void *)&vChar);
	mib_get( MIB_CONNLIMIT_TCP, (void *)&tnum);
	mib_get( MIB_CONNLIMIT_UDP, (void *)&unum);
	printf(" Connection Limit Global Settings:\n");
	printf("----------------------------------------------\n");
	printf(" Connection Limit: %s\n", vChar? "Enable":"Disable");
	printf(" Global TCP connection Limit: %d\n", tnum);
	printf(" Global UDP connection Limit: %d\n", unum);

	printf("\n Connection Limit Current Entries:\n");
	printf("%8s %16s %10s %10s\n","Index","Local IP","Protocol","Max ports");
	printf("----------------------------------------------\n");

	entryNum = mib_chain_total(MIB_TCP_UDP_CONN_LIMIT_TBL);

	for (i=0; i<entryNum; i++)
	{		
		if (!mib_chain_get(MIB_TCP_UDP_CONN_LIMIT_TBL, i, (void *)&Entry))
		{
			continue;
		}
		dest.s_addr = *(unsigned long *)Entry.ipAddr;
		strcpy(ipaddr, inet_ntoa(dest));
		
		printf("%8d%16s %10s %10d\n",i+1,ipaddr,Entry.protocol?"UDP":"TCP",Entry.connNum);	
	}
	printWaitStr();
	return;
		
}

void delConnlimt()
{
	int mibTblId;
	unsigned int totalEntry;
	unsigned int i,index;
	int del,min,max;
	
	mibTblId = MIB_TCP_UDP_CONN_LIMIT_TBL;

	totalEntry = mib_chain_total(mibTblId); /* get chain record size */
	if(totalEntry==0){
		printf("Empty table!\n");
		return;	
	}
	min=1;max=2;
	if(getInputUint("Delete (1)One (2)All :",&del,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}

	if(del==2)
	{
		mib_chain_clear(mibTblId); /* clear chain record */
		restart_connlimit();// iptable control goes here
	}
	else if(del==1)
	{
		showconnlimitTable();
		min=1;max=totalEntry+1;
		getInputUint( "Select the index to delete:",&index, &min,&max);
		if(index>totalEntry || index<=0){
			printf("Error selection!\n");
			return;
		}
		if(mib_chain_delete(mibTblId, index - 1) != 1)
		{
			printf("Delete chain record error!");
		}
		restart_connlimit();// iptable control goes here
	}
    
    return;
}

void setConnLimit()
{
	int 			snum, i;
	int 			min, max;
	int			sel,tcpl,udpl;
 	MIB_CE_TCP_UDP_CONN_LIMIT_T	entry,tmpentry;
	char			buff[32];
	unsigned 	char	selc,exist = 0;

	
	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                              Connection Limit Setting                          \n");
		printf("-------------------------------------------------------------------------\n");
		printf(" Entries in this table allow you to limit the number of TCP/UDP ports used  \n");
		printf(" by internal users \n");
		printf("-------------------------------------------------------------------------\n");
		printf("(1) Set                     (2) Add\n");
      		printf("(3) Delete                  (4) Show Connection Limit Table\n");
      		printf("(5) Quit\n");
      		if (!getInputOption( &snum, 1, 5))
      			continue;
      	
		switch( snum)
		{
			case 1://(1) Set Default Action	
				if (!check_access(SECURITY_SUPERUSER))
					break;
				min = 1; max = 2;
				if (0 == getInputUint("Connection Limit (1) Enable (2) Disable: ", &sel, &min, &max))
					continue;
				min = 0; max = 9999;
				if (0 == getInputUint("Global TCP maximun ports per user (0 for no limit): ", &tcpl, &min, &max))
					continue;
				if (0 == getInputUint("Global UDP maximun ports per user (0 for no limit): ", &udpl, &min, &max))
					continue;
				if (sel != 1)
					selc = 0;//disable
				else
					selc =1;
				mib_set( MIB_CONNLIMIT_ENABLE, &selc);
				mib_set( MIB_CONNLIMIT_TCP, (void *)&tcpl);
				mib_set( MIB_CONNLIMIT_UDP, (void *)&udpl);	
				restart_connlimit();// iptable control goes here
				break;
		
			case 2://(2) Add
				if (!check_access(SECURITY_SUPERUSER))
					break;
				min = 1; max = 2;
				if (0 == getInputUint("Protocol: (1)TCP (2)UDP: ", &sel, &min, &max))
					continue;
				entry.protocol = sel -1; //0 = TCP
				
				entry.Enabled = 1;
				if (0 == getInputString("IP Address: ", &buff, sizeof(buff)-1))
					continue;
				else {
					struct in_addr curIpAddr, curSubnet;
					unsigned long v1, v2, v3;
					if ( !inet_aton(buff, (struct in_addr *)&entry.ipAddr) ) {
						continue;
					}			
					mib_get( MIB_ADSL_LAN_IP,  (void *)&curIpAddr);
					mib_get( MIB_ADSL_LAN_SUBNET,  (void *)&curSubnet);

					v1 = *((unsigned long *)entry.ipAddr);
					v2 = *((unsigned long *)&curIpAddr);
					v3 = *((unsigned long *)&curSubnet);

					if ( (v1 & v3) != (v2 & v3) ) {
						continue;
					}
				}
				min = 1;
				if (entry.protocol == 0)
					mib_get(MIB_CONNLIMIT_TCP,&max);
				else 
					mib_get(MIB_CONNLIMIT_UDP,&max);
				if (max == 0) // 0 means no limit
					max = 9999;
		         	if (0 == getInputUint("Max number of ports: (1~ Global Limitation): ", &sel, &min, &max))
					continue;
				entry.connNum = sel;
				max = mib_chain_total(MIB_TCP_UDP_CONN_LIMIT_TBL);
				exist = 0;
				for(i=0;i<max;i++){
					if(!mib_chain_get(MIB_TCP_UDP_CONN_LIMIT_TBL,i,(void*)&tmpentry))
						continue;
					if(((*(int*)(entry.ipAddr))==(*(int*)(tmpentry.ipAddr))) &&
						(entry.protocol == tmpentry.protocol))
					{
						exist = 1;
					}
				}

				if (exist == 0)
				{
					mib_chain_add(MIB_TCP_UDP_CONN_LIMIT_TBL, (void *)&entry) ;
					restart_connlimit();// iptable control goes here
				}
	        		break;
			case 3://(3) Delete
				if (!check_access(SECURITY_SUPERUSER))
					break;
				delConnlimt();				
		        	break;
			case 4://show
				showconnlimitTable();
				break;
			case 5://(5) Quit
	        		return;		

		}
	}
}
#endif 
#ifdef ADDRESS_MAPPING
#ifdef MULTI_ADDRESS_MAPPING


static void showAddrMapTable()
{
	unsigned int entryNum, i,unum ,tnum;
	MIB_CE_MULTI_ADDR_MAPPING_LIMIT_T 	Entry;
	char *dir, *ract;
	char lsip[20],leip[20],gsip[20],geip[20];
	struct in_addr 		dest;
	unsigned char vChar;
	
	CLEAR;	
	printf("\n Address Mapping Current Entries:\n");
	printf("%6s %16s %16s %16s %16s\n","Index","Local Start IP","Local End IP","Global Start IP", "Global End IP");
	printf("---------------------------------------------------------------------------\n");

	entryNum = mib_chain_total(MULTI_ADDRESS_MAPPING_LIMIT_TBL);

	for (i=0; i<entryNum; i++)
	{		
		if (!mib_chain_get(MULTI_ADDRESS_MAPPING_LIMIT_TBL, i, (void *)&Entry))
		{
			continue;
		}
		dest.s_addr = *(unsigned long *)Entry.lsip;
		strcpy(lsip, inet_ntoa(dest));
		dest.s_addr = *(unsigned long *)Entry.leip;
		if (dest.s_addr)
			strcpy(leip, inet_ntoa(dest));
		else
			strcpy(leip, "  ---  ");
		dest.s_addr = *(unsigned long *)Entry.gsip;
		strcpy(gsip, inet_ntoa(dest));
		dest.s_addr = *(unsigned long *)Entry.geip;
		if (dest.s_addr)
			strcpy(geip, inet_ntoa(dest));
		else 
			strcpy(geip, "  ---  ");
		
		printf("%6d %16s %16s %16s %16s\n",i+1, lsip, leip, gsip, geip);
	}
	printWaitStr();
	return;
		
}

static void delAddrMap()
{
	int mibTblId;
	unsigned int totalEntry;
	unsigned int i,index;
	int del,min,max;
	
	mibTblId = MULTI_ADDRESS_MAPPING_LIMIT_TBL;

	totalEntry = mib_chain_total(mibTblId); /* get chain record size */
	if(totalEntry==0){
		printf("Empty table!\n");
		return;	
	}
	min=1;max=2;
	if(getInputUint("Delete (1)One (2)All :",&del,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}

	if(del==2)
	{
		config_AddressMap(ACT_STOP);
		mib_chain_clear(mibTblId); /* clear chain record */
		config_AddressMap(ACT_START);
	}
	else if(del==1)
	{
		showAddrMapTable();
		min=1;max=totalEntry+1;
		getInputUint( "Select the index to delete:",&index, &min,&max);
		if(index>totalEntry || index<=0){
			printf("Error selection!\n");
			return;
		}
		config_AddressMap(ACT_STOP);
		if(mib_chain_delete(mibTblId, index - 1) != 1)
		{
			printf("Delete chain record error!");
		}
		config_AddressMap(ACT_START);
	}    
    return;
}

static void setAddrMap()
{
	int 			snum, i;
	int 			min, max;
	int			sel,tcpl,udpl;
 	MIB_CE_MULTI_ADDR_MAPPING_LIMIT_T	entry,tmpentry;
	char			buff[32];
	unsigned 	char	selc,exist = 0;
//typedef enum { ADSMAP_NONE=0, ADSMAP_ONE_TO_ONE=1, =2, ADSMAP_MANY_TO_MANY=3, ADSMAP_ONE_TO_MANY=4 } ADSMAP_T;
	
	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                              Address Mappinig Setting                         	 \n");
		printf("-------------------------------------------------------------------------\n");
		printf(" Entries in this table allow you to add one to one, many to one,  \n");
		printf(" many to many, many to one addres mappings. The maximum number is 16. \n");
		printf("-------------------------------------------------------------------------\n");
		printf("(1) Show                     (2) Add\n");
      		printf("(3) Delete                   (4) Quit\n");
      		if (!getInputOption( &snum, 1, 4))
      			continue;

		memset(&entry, 0, sizeof(entry));
		switch( snum)
		{
			case 1://show
				showAddrMapTable();
				break;
			case 2://(2) Add
				if (!check_access(SECURITY_SUPERUSER))
					break;
				max = mib_chain_total(MULTI_ADDRESS_MAPPING_LIMIT_TBL);
				if (max >=15)
					break;
				min = 1; max = 4;
				if (0 == getInputUint("Address Mapping Type: \r\n(1)One to One (2)Many to one: (3)Many to Many (4)One to Many :", &sel, &min, &max))
					continue;
				entry.addressMapType = sel; 

				// local start ip
				if (0 == getInputString("Local Start IP: ", &buff, sizeof(buff)-1))
					continue;
				else {
					if ( !inet_aton(buff, (struct in_addr *)&entry.lsip) ) {
						continue;
					}			
				}
				//local end ip
				if ((entry.addressMapType==ADSMAP_MANY_TO_ONE)||(entry.addressMapType==ADSMAP_MANY_TO_MANY))
				{
					if (0 == getInputString("Local End IP: ", &buff, sizeof(buff)-1))
						continue;
					else {
						if ( !inet_aton(buff, (struct in_addr *)&entry.leip) ) {
							continue;
						}			
					}

				}
				//Global Start IP
				if (0 == getInputString("Global Start IP: ", &buff, sizeof(buff)-1))
						continue;
				else {
					if ( !inet_aton(buff, (struct in_addr *)&entry.gsip) ) {
						continue;
					}			
				}
				
				//global end ip
				if ((entry.addressMapType==ADSMAP_ONE_TO_MANY)||(entry.addressMapType==ADSMAP_MANY_TO_MANY))
				{
					if (0 == getInputString("Global End IP: ", &buff, sizeof(buff)-1))
						continue;
					else {
						if ( !inet_aton(buff, (struct in_addr *)&entry.geip) ) {
							continue;
						}			
					}

				}
				config_AddressMap(ACT_STOP);
				mib_chain_add(MULTI_ADDRESS_MAPPING_LIMIT_TBL, (void *)&entry) ;
				config_AddressMap(ACT_START);
	        		break;
			case 3://(3) Delete
				if (!check_access(SECURITY_SUPERUSER))
					break;
				delAddrMap();				
		        	break;

			case 4://(5) Quit
	        		return;		

		}
	}
}

#endif 
#endif

void printspace(unsigned int x)
{
	while (x--)
	{
		printf(" ");
	}
}
void setFirewall()
{
	int snum,x;
	int submenu,tmpi=0;;
	MENU_ITEM	*pm;
	unsigned char	buf[256];
	MENU_ITEM fw_menu[] = {
		{"IP/Port Filtering",	setIPPortFiltering},
		{"MAC Filtering",		setMACFiltering},	
		{"Port Forwarding",		setPortForwarding},		
#ifdef URL_BLOCKING_SUPPORT
		{"URL Blocking",		setURLBlocking}, 	
#endif
#ifdef DOMAIN_BLOCKING_SUPPORT
		{"Domain Blocking",	setDomainBlocking},	
#endif
		{"DMZ",				setDMZ},	
#ifdef TCP_UDP_CONN_LIMIT
		{"Connection Limit",	setConnLimit},			
#endif		
#ifdef ADDRESS_MAPPING
#ifdef MULTI_ADDRESS_MAPPING
		{"Address Mapping",	setAddrMap},			
#endif	
#endif
	
		{"",NULL}
	};
	
	while (1)
	{
		tmpi = 0;
		CLEAR;
		printf("\n");
		printf("-----------------------------------------------------------\n");
		printf("                (3) Firewall Menu                          \n");
		printf("-----------------------------------------------------------\n");
		for (pm =&fw_menu[0] ;pm->fn!= NULL;pm++)
		{
			tmpi++;
			printf("(%d) %s",tmpi, pm->display_name);
			if (tmpi&0x01)
				printspace(33-4-strlen(pm->display_name));
			else
				printf("\n");								
		}
		tmpi++;
		printf("(%d) Quit \n",tmpi);
		if (!getInputOption( &snum, 1, tmpi))
		      	continue;
		if (snum == tmpi)
			return;
		fw_menu[snum-1].fn();
	}
}
void setFirewall_old()
{
   int snum;

   while (1)
   {
      CLEAR;
      printf("\n");
      printf("-----------------------------------------------------------\n");
      printf("                (3) Firewall Menu                          \n");
      printf("-----------------------------------------------------------\n");
#ifndef ZTE_531B_BRIDGE_SC
      printf("(1) IP/Port Filtering            (2) MAC Filtering\n");
#ifdef ZTE_GENERAL_ROUTER_SC
      printf("(3) Virtual Server               (4) DMZ\n");
#else
	  printf("(3) Port Forwarding              (4) DMZ\n");
#endif
      printf("(5) Quit\n");
#else
	  printf("(1) MAC Filtering                (2) Quit\n");
#endif
#ifndef ZTE_531B_BRIDGE_SC
      if (!getInputOption( &snum, 1, 5))
      	continue;
      
      switch( snum)
      {
      case 1://(1) IP/Port Filtering
         setIPPortFiltering();
         break;
      case 2://(2) MAC Filtering
#ifdef MAC_FILTER
         setMACFiltering();
#endif
         break;
      case 3://(3) Port Forwarding
         setPortForwarding();
         break;
      case 4://(4) DMZ
         setDMZ();
         break;
      case 5://(5) Quit
         return;
      default:
         printf("!! Invalid Selection !!\n");
      }//end switch, (3) Firewall Menu
#else
	  if (!getInputOption( &snum, 1, 2))
      	continue;
      
      switch( snum)
      {
      case 1://(2) MAC Filtering
#ifdef MAC_FILTER
         setMACFiltering();
#endif
         break;
      case 2://(5) Quit
         return;
      default:
         printf("!! Invalid Selection !!\n");
      }//end switch, (3) Firewall Menu
#endif
   }//end while, (3) Firewall Menu
}

#ifdef CONFIG_USER_IGMPPROXY
//tylo
unsigned char IGMPsel[8];
int IGMPProxylist(){
	int ifnum=0;
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	//char_t  buffer[3];
	char_t *name;
	int type=1;//rt
	unsigned char IGMPselcnt=0;

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	printf("Idx  Interface\n");
	printf("----------------\n");
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}
		
		if (Entry.enable == 0)
			continue;
		
		if (type == 2) {
			if (Entry.cmode == ADSL_BR1483)
			{
				IGMPsel[IGMPselcnt]=Entry.ifIndex;
				IGMPselcnt++;
				printf("%-5dvc%u\n",
					IGMPselcnt, VC_INDEX(Entry.ifIndex));
				
				ifnum++;
			}
		}
		else { // rt or all (1 or 0)
			if (type == 1 && Entry.cmode == ADSL_BR1483)
				continue;
			if (PPP_INDEX(Entry.ifIndex) != 0x0f)
			{	// PPP interface
				IGMPsel[IGMPselcnt]=Entry.ifIndex;
				IGMPselcnt++;

				printf("%-5dppp%u\n",
					IGMPselcnt, PPP_INDEX(Entry.ifIndex));
			}
			else
			{	// vc interface
				IGMPsel[IGMPselcnt]=Entry.ifIndex;
				IGMPselcnt++;

				printf("%-5dvc%u\n",
					IGMPselcnt, VC_INDEX(Entry.ifIndex));
			}
			ifnum++;
		}
	}

	return ifnum;
}

//tylo
void IGMPProxy(unsigned char act){
	char	*str, *submitUrl;
	char tmpBuf[100];
	FILE *fp;
	char * argv[8];
	char ifname[6];
	unsigned char proxy, proxy_if;
	unsigned char if_num;
	int igmp_pid;
	unsigned int min,max,sel,test;

	if (act==1)
		proxy = 1;
	else
		proxy = 0;
	if ( !mib_set(MIB_IGMP_PROXY, (void *)&proxy)) {
		strcpy(tmpBuf, T("Set IGMP proxy error!"));
		goto setErr_igmp;
	}

	if(proxy==0 && act==0)
		return;
	if ( act == 2 )
		goto KILL_IGMPD;

	max=IGMPProxylist();
	min=1;
	if(getInputUint("Select interface:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}
	proxy_if=IGMPsel[sel-1];
	if ( !mib_set(MIB_IGMP_PROXY_ITF, (void *)&proxy_if)) {
		printf("Set IGMP proxy interface index error!\n");
		goto setErr_igmp;
	}

#ifdef EMBED
#define RUNFILE "/var/run/igmp_pid"
//static const char LANIF[] = "br0";
//static const char IGMPROXY[] = "/bin/igmpproxy";
KILL_IGMPD:
#if defined(APPLY_CHANGE)
	// Take effect in real time
	igmp_pid = read_pid(RUNFILE);
	
	if (igmp_pid >= 1) {
		// kill it
		if (kill(igmp_pid, SIGTERM) != 0) {
			printf("Could not kill pid '%d'", igmp_pid);
		}
	}
	
	if (proxy == 1) {
		// start the igmp proxy
		if_num = PPP_INDEX(proxy_if);
		if (if_num != 0x0f)
			snprintf(ifname, 6, "ppp%u", if_num);
		else
			snprintf(ifname, 5, "vc%u", VC_INDEX(proxy_if));
			
		argv[1] = ifname;
		argv[2] = (char *)LANIF;
		argv[3] = NULL;
		//printf("%s %s %s\n", IGMPROXY, argv[1], argv[2]);
		do_cmd(IGMPROXY, argv, 0);
		printWaitStr();
	}
#endif
#endif
	return;
setErr_igmp:
	printf("Set IGMP Proxy error!\n");
	return;
}

//tylo
void setIGMPProxy()
{
   int snum, proxy_act;
   unsigned char proxy_if;
   char ifname[16];
   char enabled;

   while (1)
    {
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                           IGMP Proxy Configuration                      \n");
		printf("-------------------------------------------------------------------------\n");
      		printf("IGMP proxy enables the system to issue IGMP host messages on behalf of   \n");
      		printf("hosts that the system discovered through standard IGMP interfaces. The   \n");
      		printf("system acts as a proxy for its hosts when you enable it by doing the     \n");
      		printf("follows:                                                                 \n");
      		printf(". Enable IGMP proxy on WAN interface (upstream), which connects to a     \n");
      		printf("  running IGMP. \n");
      		printf(". Enable IGMP on LAN interface (downstream), which connects to its hosts.\n");
		printf("-------------------------------------------------------------------------\n");
		mib_get(MIB_IGMP_PROXY, (void *)&enabled);
		printf("IGMP Proxy: %s\n", enabled? STR_ENABLE:STR_DISABLE);
		if (enabled) {
			mib_get(MIB_IGMP_PROXY_ITF, (void *)&proxy_if);
			ifname[0] = 0;
			ifGetName(proxy_if, ifname, sizeof(ifname));
			printf("Proxy Interface: %s\n", ifname);
		}
      		printf("\n(1) Set                          (2) Quit\n\n");
      		if (!getInputOption( &snum, 1, 2))
      			continue;
      		
      		switch( snum)
		{
			case 1://(1) Set
				if (!check_access(SECURITY_SUPERUSER))
					break;
         			printf("IGMP Proxy (1)Enable (2)Disable :");
         			if( (proxy_act=getInputNum()) ==0) continue;
         			IGMPProxy(proxy_act);
         			break;
      			case 2://(2) Quit
         			return;
      		}//end switch, IGMP Proxy Configuration
   }//end while, IGMP Proxy Configuration
}
#endif	// of CONFIG_USER_IGMPPROXY

#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
void setUPnP()
{
	int snum, min, max, sel;
	unsigned char ext_if, vChar, upnp;
	char ifname[6];
	char enabled;

	while (1) {
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                         UPnP Configuration                      \n");
		printf("-------------------------------------------------------------------------\n");
      		printf("This page is used to configure UPnP. The system acts as a daemon when you \n");
      		printf("enable it and select WAN interface (upstream) that will use UPnP. \n");
		printf("-------------------------------------------------------------------------\n");
		mib_get(MIB_UPNP_DAEMON, (void *)&enabled);
		printf("UPnP: %s\n", enabled? STR_ENABLE:STR_DISABLE);
		if (enabled) {
			mib_get(MIB_UPNP_EXT_ITF, (void *)&ext_if);
			ifname[0] = 0;
			ifGetName(ext_if, ifname, sizeof(ifname));
			printf("WAN Interface: %s\n", ifname);
		}
      		printf("\n(1) Set                          (2) Quit\n\n");
      		if (!getInputOption( &snum, 1, 2))
      			continue;
      		switch( snum) {
			case 1://(1) Set
				if (!check_access(SECURITY_SUPERUSER))
					break;
				min = 1; max = 2;
				if (0 == getInputUint("UPnP (1) Disable (2) Enable: ", &sel, &min, &max))
					continue;
				upnp = (unsigned char)(sel - 1);
				if (upnp==0)
					goto end_upnp;
				max = IGMPProxylist();
				min=1;
				if (getInputUint("Select interface:", &sel, &min, &max)==0){
					printf("Invalid selection!\n");
					goto setErr_upnp;
				}
				vChar = IGMPsel[sel-1];
				if (!mib_set(MIB_UPNP_DAEMON, (void *)&upnp)) {
					printf("Set UPnP error!");
					goto setErr_upnp;
				}
				if (!mib_set(MIB_UPNP_EXT_ITF, (void *)&vChar)) {
					printf("Set UPnP interface index error!\n");
					goto setErr_upnp;
				}
         			break;
      			case 2://(2) Quit
         			return;
      		}
		setErr_upnp:
			printWaitStr();
		end_upnp:
			printf("\n");
	}
}
#endif

#ifdef CONFIG_USER_ROUTED_ROUTED
//tylo
/* Return value:
 * 0: fail
 * 1: successful
 */
int RIPconfig(){
	unsigned char ripVal;
	unsigned char ripValGet;
	unsigned int min,max,sel;
	char msgstr[10];

	mib_get(MIB_RIP_ENABLE, (void *)&ripValGet);
	if(ripValGet==1)
		strcpy(msgstr,"ON");
	else
		strcpy(msgstr,"OFF");
	printf("RIP: %s\n",msgstr);
	min=1;max=2;
	if(getInputUint("New RIP (1)ON (2)OFF :",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		return 0;
	}

	if (sel==1)
		ripVal = 1;
	else
		ripVal = 0;	// default "off"
	if (!mib_set(MIB_RIP_ENABLE, (void *)&ripVal)) {
		printf("Set RIP error!");
	}

	return 1;
}
#endif	// of CONFIG_USER_ROUTED_ROUTED

// Kaohj
int showRip()
{
	unsigned int entryNum, i;
	MIB_CE_RIP_T Entry;
	char ifname[6];
	char *receive_mode, *send_mode;
	char mode_none[] = "None";
	char mode_rip1[] = "RIP1";
	char mode_rip2[] = "RIP2";
	char mode_both[] = "Both";
	char mode_rip1comp[] = "RIP1COMPAT";
	
	entryNum = mib_chain_total(MIB_RIP_TBL);
	printf("Index\tIface\tReceive\tSend\n");
	MSG_LINE;
	if (!entryNum) {
		printf("No data!\n\n");
		return 0;
	}
	
	for (i=0; i<entryNum; i++) {

		mib_chain_get(MIB_RIP_TBL, i, (void *)&Entry);
		
		if( Entry.ifIndex == 0xff) {
			strncpy(ifname, "br0", 6);
			ifname[5] = '\0';
		} else {
			ifGetName(Entry.ifIndex, ifname, 6);
		}
		
		if ( Entry.receiveMode == RIP_NONE ) {
			receive_mode = mode_none;
		} else if ( Entry.receiveMode == RIP_V1 ) {
			receive_mode = mode_rip1;
		} else if ( Entry.receiveMode == RIP_V2 ) {
			receive_mode = mode_rip2;
		} else if ( Entry.receiveMode == RIP_V1_V2 ) {
			receive_mode = mode_both;
		} else {
			receive_mode = mode_none;
		}	
		
		if ( Entry.sendMode == RIP_NONE ) {
			send_mode = mode_none;
		} else if ( Entry.sendMode == RIP_V1 ) {
			send_mode = mode_rip1;
		} else if ( Entry.sendMode == RIP_V2 ) {
			send_mode = mode_rip2;
		} else if ( Entry.sendMode == RIP_V1_COMPAT ) {
			send_mode = mode_rip1comp;
		} else {
			send_mode = mode_none;
		}
		
		printf("%d\t%s\t%s\t%s\n", i+1, ifname, receive_mode, send_mode);
	}
	return(entryNum);
}

//tylo
unsigned char RIPsel[8];
int showItf(unsigned char sel)//0:show all interface without bridge   1://show rip interface 2:show all itf
{
	int ifnum=0;
	
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	char ifname[6];
	char_t  buffer[3];
	
	// check WAN
	printf("Idx  Interface\n");
	printf("-------------------\n");
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	//printf("entrynum=%d\n",entryNum);
	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}
		
		if (Entry.enable == 0)
			continue;
		//printf("cmode:%d  rip:%d\n",Entry.cmode,Entry.rip);
		if(sel==1){
			if (Entry.cmode != ADSL_BR1483 && Entry.rip)
			{
				if (PPP_INDEX(Entry.ifIndex) != 0x0f)
				{	// PPP interface
					snprintf(ifname, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));
				}
				else
				{	// vc interface
					snprintf(ifname, 6, "vc%u", VC_INDEX(Entry.ifIndex));
				}
				printf("%-5d%s\n",ifnum+1, ifname);
				RIPsel[ifnum]=Entry.ifIndex;
				ifnum++;
			}
		}
		else if(sel==0){
			if (Entry.cmode != ADSL_BR1483)
			{
				if (PPP_INDEX(Entry.ifIndex) != 0x0f)
				{	// PPP interface
					snprintf(ifname, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));
				}
				else
				{	// vc interface
					snprintf(ifname, 6, "vc%u", VC_INDEX(Entry.ifIndex));
				}
				printf("%-5d%s\n",ifnum+1, ifname);
				RIPsel[ifnum]=Entry.ifIndex;
				ifnum++;
			}
		}
		else{
			if (PPP_INDEX(Entry.ifIndex) != 0x0f)
			{	// PPP interface
				snprintf(ifname, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));
			}
			else
			{	// vc interface
				snprintf(ifname, 6, "vc%u", VC_INDEX(Entry.ifIndex));
			}
			printf("%-5d%s\n",ifnum+1, ifname);
			RIPsel[ifnum]=Entry.ifIndex;
			ifnum++;
		}
	}
	//snprintf(buffer, 3, "%u", ifnum);
	//ejSetResult(eid, buffer);
	return ifnum;
}


#ifdef CONFIG_USER_ROUTED_ROUTED
//tylo
/* Return value:
 * 0: fail
 * 1: successful
 */
int RIPAddDel(){
	unsigned int min,max,sel,idxsel;
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T vcEntry;
	MIB_CE_RIP_T ripEntry;


	min=1;max=2;
	if(getInputUint("(1)Add RIP Interface (2)Del RIP Interface :",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		return 0;
	}

	if(sel==1){//add
		int intVal;
		max=showItf(0);
		if(max==0)
			return 0;
		if(getInputUint("Select index:",&idxsel,&min,&max)==0){
			return 0;
		}
		
		//mib_chain_get(MIB_ATM_VC_TBL, idxsel-1, (void *)&vcEntry);
		// get the selected VC
		entryNum = mib_chain_total(MIB_ATM_VC_TBL);
		for (i=0; i<entryNum; i++) {
        	
			mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&vcEntry);
			
			if (vcEntry.enable == 0)
				continue;
			if (vcEntry.cmode != ADSL_BR1483)
			{
				idxsel--;
				if (!idxsel)
					break;
			}
		}
			
		// check RIP table
		entryNum = mib_chain_total(MIB_RIP_TBL);
		for (i=0; i<entryNum; i++) {
			mib_chain_get(MIB_RIP_TBL, i, (void *)&ripEntry);
			if (ripEntry.ifIndex == vcEntry.ifIndex) {
				printf("Entry already exists!\n");
				printWaitStr();
				return 0;
			}
		}
		
		memset(&ripEntry, '\0', sizeof(MIB_CE_RIP_T));
		ripEntry.ifIndex = vcEntry.ifIndex;
		
		max = 4;
		if(getInputUint("Receive Mode (1)None (2)RIPv1 (3)RIPv2 (4)Both: ",&sel,&min,&max)==0){
			return 0;
		}
		ripEntry.receiveMode = sel-1;
		
		max = 4;
		if(getInputUint("Send Mode (1)None (2)RIPv1 (3)RIPv2 (4)RIP1COMPAT: ",&sel,&min,&max)==0){
			return 0;
		}
		ripEntry.sendMode = sel-1;
		if (sel == 4)
			ripEntry.sendMode = RIP_V1_COMPAT;
		
		intVal = mib_chain_add(MIB_RIP_TBL, (unsigned char*)&ripEntry);
		if (intVal == 0) {
			printf(strAddChainerror);
			return 0;
		}
		else if (intVal == -1) {
			printf(strTableFull);
			return 0;
		}
	}
	else{//del
		max = showRip();
		if(max==0) {
			printWaitStr();
			return 0;
		}
		if(getInputUint("Select index:",&idxsel,&min,&max)==0){
			printf("Invalid selection!\n");
			return 0;
		}

		if(mib_chain_delete(MIB_RIP_TBL, idxsel-1) != 1)
			printf("Failed\n");
	}
	return 1;
}

//tylo
void setRIP()
{
   int snum, ret;
   unsigned char ripOn;

   while (1)
    {
      CLEAR;
      printf("\n");
      printf("-------------------------------------------------------------------------\n");
      printf("                           RIP Configuration                             \n");
      printf("-------------------------------------------------------------------------\n");
      printf("Enable the RIP if you are using this device as a RIP-enabled router to   \n");
      printf("communicate with others using the Routing Information Protocol. This page\n");
      printf("is used to select the interfaces on your device is that use RIP, and the  \n");
      printf("version of the protocol used.                                            \n");
		printf("-------------------------------------------------------------------------\n");
	mib_get(MIB_RIP_ENABLE, (void *)&ripOn);
	printf("RIP: %s\n", ripOn?STR_ENABLE:STR_DISABLE);
      printf("(1) RIP Enable/Disable         (2)Add/Del Interface\n");
      printf("(3) Show RIP interface         (4)Quit\n\n");
      if (!getInputOption( &snum, 1, 4))
      	continue;
      ret = 0;
      switch( snum)
	{
		case 1://(1) Set
			if (!check_access(SECURITY_SUPERUSER))
				break;
			ret = RIPconfig();
         		break;
      		case 2://(2)add/del
			if (!check_access(SECURITY_SUPERUSER))
				break;
      			ret = RIPAddDel();
      			break;
      		case 3://(3)show
			showRip();
			printWaitStr();
      			break;
     		case 4://(4) Quit
         		return;
      	}//end switch, RIP Configuration
#if defined(APPLY_CHANGE)
	// Take effect in real time
      	if (ret)
      		startRip();
#endif      
   }//end while, RIP Configuration
}
#endif	// of CONFIG_USER_ROUTED_ROUTED

void setServices()
{
   int snum;

   while (1)
   {
	CLEAR;
      printf("\n");
      printf("-----------------------------------------------------------\n");
      printf("                (5) Services Menu                          \n");
      printf("-----------------------------------------------------------\n");
      printf("(1) DHCP Type                    (2) DNS\n");
#ifndef ZTE_531B_BRIDGE_SC
      printf("(3) Firewall                     (4) IGMP Proxy\n");
      printf("(5) UPnP                         (6) RIP\n");
      printf("(7) Quit\n");
#else
 	  printf("(3) Firewall                     (4) Quit\n");
#endif
      if (!getInputOption( &snum, 1, 7))
      	continue;
      
      switch( snum)
      {
      case 1://(1) DHCP Server
         setDHCPPage();
         break;
      case 2://(2) DNS
         setDNS();
#if defined(APPLY_CHANGE)
	// Take effect in real time
	startDnsRelay();
#endif
         break;
      case 3://(3) Firewall
         setFirewall();
         break;
#ifndef ZTE_531B_BRIDGE_SC
#ifdef CONFIG_USER_IGMPPROXY
      case 4://(4) IGMP Proxy
         setIGMPProxy();
         break;
#endif
      case 5://(5) UPnP
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
	  setUPnP();
#else
	  printf("Not supported\n");
	  printWaitStr();
#endif
         break;
#ifdef CONFIG_USER_ROUTED_ROUTED
      case 6://(6) RIP
         setRIP();
         break;
#endif
      case 7://(7) Quit
         return;
#else
	  case 4://Quit
	  	 return;
#endif
      default:
         printf("!! Invalid Selection !!\n");
      }//end switch, (5) Services Menu
   }//end while, (5) Services Menu
}
/*************************************************************************************************/
void showARPTable()
{
	FILE *fp;
	char arg1[20], arg2[20];
	int arg3;

	printf("\n");
	printf("-------------------------------------------------------------------------\n");
	printf("		   ARP Table 				 \n");
	printf("-------------------------------------------------------------------------\n");
	printf("This table shows a list of learned MAC addresses.\n");
	printf("-------------------------------------------------------------------------\n");
	printf("IP Address          MAC Address\n");
	printf("-------------------------------------------------------------------------\n");

	fp = fopen("/proc/net/arp", "r");
	if (fp == NULL){
		printf("read arp file fail!\n");
		return;
	}
	fgets(strbuf, 256, fp);
	while (fgets(strbuf, 256, fp)) {
		sscanf(strbuf, "%s%*x 0x%x%s", arg1, &arg3, arg2);
		if (!arg3)
			continue;
		printf("%-20s%s\n", arg1, arg2);
	}

	fclose(fp);
}

//tylo
int br_socket_fd;
static void __dump_fdb_entry(struct __fdb_entry *f)
{
	unsigned long long tvusec;
	int sec,usec;
	
	// jiffies to tv
	tvusec = (1000000ULL*f->ageing_timer_value)/HZ;
	sec = tvusec/1000000;
	usec = tvusec - 1000000 * sec;
	
	printf("%-9i%.2x:%.2x:%.2x:%.2x:%.2x:%.2x  %-11s%4i.%.2i\n",
		f->port_no, f->mac_addr[0], f->mac_addr[1], f->mac_addr[2],
		f->mac_addr[3], f->mac_addr[4], f->mac_addr[5],
		f->is_local?"yes":"no", sec, usec/10000);
}

void showMACs()
{
	struct bridge *br;
	struct __fdb_entry fdb[256];
	int offset;
	unsigned long args[4];
	struct ifreq ifr;

   printf("\n");
   printf("-------------------------------------------------------------------------\n");
   printf("                       Bridge Forwarding Database Table                  \n");
   printf("-------------------------------------------------------------------------\n");
   printf("This table shows a list of learned MAC addresses for this bridge.        \n");
   printf("-------------------------------------------------------------------------\n");
   printf("Port No      MAC Address            Is Local?         Ageing Timer       \n");
   printf("-------------------------------------------------------------------------\n");
	
	offset = 0;
	args[0] = BRCTL_GET_FDB_ENTRIES;
	args[1] = (unsigned long)fdb;
	args[2] = 256;
	if ((br_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("socket not avaiable !!\n");
		return;
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
				printf("br0 interface not exists !!\n");
			break;
}

		for (i=0;i<num;i++)
			__dump_fdb_entry(fdb+i);

		offset += num;
	}
	close(br_socket_fd);

}

//tylo
void setBridging()
{
   int snum, ageingTime;
   unsigned short time;
   unsigned char stp;
   char msgstr[10];
   int min,max,sel;
#if defined(APPLY_CHANGE)
   char *argv[5];
   char str[10];
   //unsigned char str_cmd[100];
#endif

   while (1)
    {
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                           Bridge Configuration                          \n");
		printf("-------------------------------------------------------------------------\n");
      		printf("This page is used to configure the bridge parameters. Here you can change\n");
      		printf("the settings or view some information on the bridge and its attached     \n");
      		printf("ports.                                                                   \n");
		printf("-------------------------------------------------------------------------\n");
      		printf("(1) Set                          (2) Show MACs\n");
      		printf("(3) Quit\n");
      		if (!getInputOption( &snum, 1, 3))
      			continue;
      		
      		switch( snum)
		{
			case 1://(1) Set
				if (!check_access(SECURITY_SUPERUSER))
					break;
				mib_get(MIB_BRCTL_AGEINGTIME, (void *)&time);
        			printf("Old Ageing Time: %d(seconds)\n",time);
         			printf("New Ageing Time(seconds):");
         			if( (ageingTime=getInputNum()) ==0) continue;
         			time=(unsigned short)ageingTime;
				if ( mib_set(MIB_BRCTL_AGEINGTIME, (void *)&time) == 0) {
					printf("Set bridge ageing time MIB error!");
				}

#if defined(APPLY_CHANGE)				
				sprintf(str,"%u",time);
				//printf("str=%s\n", str);
				argv[1]="setageing";
				argv[2]=(char*)BRIF;
				argv[3]=str;
				argv[4]=NULL;
				//sprintf(str_cmd, "%s %s %s %s\n", BRCTL, argv[1], argv[2], argv[3]);
				//printf("str_cmd=%s\n", str_cmd);
				do_cmd(BRCTL, argv, 1);
#endif

				mib_get(MIB_BRCTL_STP, (void *)&stp);
				if(stp==1)
					strcpy(msgstr,"Enabled");
				else
					strcpy(msgstr,"Disabled");
				min=1;max=2;
         			printf("Old 802.1d Spanning Tree: %s\n",msgstr);
         			printf("New 802.1d Spanning Tree (1)Enabled (2)Disabled:\n");
         			if(getInputUint("Select index:",&sel,&min,&max)==0){
					printf("Invalid selection!\n");
					return;
				}
				if (sel== 1)
					stp = 1;
				else
					stp = 0;
				if ( !mib_set(MIB_BRCTL_STP, (void *)&stp)) {
					printf("Set STP mib error!");
				}

#if defined(APPLY_CHANGE)		
				if (stp == 1)	// on
				{	// brctl setfd br0 20
					argv[1]="setfd";
					argv[2]=(char*)BRIF;
					argv[3]="20";
					argv[4]=NULL;
					//sprintf(str_cmd, "%s %s %s %s\n", BRCTL, argv[1], argv[2], argv[3]);
					//printf("str_cmd=%s\n", str_cmd);
					do_cmd(BRCTL, argv, 1);
				}
				
				argv[1]="stp";
				argv[2]=(char*)BRIF;
				
				if (stp == 0)
					argv[3]="off";
				else
					argv[3]="on";
				
				argv[4]=NULL;
				//sprintf(str_cmd, "%s %s %s %s\n", BRCTL, argv[1], argv[2], argv[3]);
				//printf("str_cmd=%s\n", str_cmd);
				do_cmd(BRCTL, argv, 1);
#endif
         			break;
      			case 2://(2) Show MACs
         			showMACs();
				printWaitStr();
         			break;
      			case 3://(3) Quit
         			return;
      		}//end switch, Bridge Configuration
   }//end while, Bridge Configuration
}

//tylo
void showRoutes()
{

	int nBytesSent=0;
	char buff[256];
	int flgs;
	unsigned long int d,g,m;
	struct in_addr dest;
	struct in_addr gw;
	struct in_addr mask;
	char sdest[16], sgw[16], iface[6];
	FILE *fp;

	CLEAR;
	printf("\n");
	printf("-------------------------------------------------------------------------\n");
	printf("                       IP Route Table                                    \n");
	printf("-------------------------------------------------------------------------\n");
	printf("This table shows a list of destination routes commonly accessed by your  \n");
	printf("network.                                                                 \n");
	printf("-------------------------------------------------------------------------\n");
	printf("Destination      Subnet Mask      NextHop          Iface\n");
	printf("-------------------------------------------------------------------------\n");
	
	if (!(fp=fopen("/proc/net/route", "r"))) {
		fclose(fp);
		printf("Error: cannot open /proc/net/route !!\n");
		printWaitStr();
		return;
	}


	fgets(buff, sizeof(buff), fp);
	
	while( fgets(buff, sizeof(buff), fp) != NULL ) {
		if(sscanf(buff, "%s%lx%lx%X%*d%*d%*d%lx",
		   iface, &d, &g, &flgs, &m)!=5) {
			printf("Error: Unsuported kernel route format !!\n");
			printWaitStr();
			return;
		}
		
		if(flgs & RTF_UP) {
			dest.s_addr = d;
			gw.s_addr   = g;
			mask.s_addr = m;
			// inet_ntoa is not reentrant, we have to
			// copy the static memory before reuse it
			strcpy(sdest, inet_ntoa(dest));
			strcpy(sgw,  (gw.s_addr==0   ? "*" : inet_ntoa(gw)));
		
			printf("%-17s%-17s%-17s%s\n",sdest, inet_ntoa(mask), sgw, iface);
		}
	}
	
	fclose(fp);
	printWaitStr();
	return;
}

//tylo
int showStaticRoutes()
{
	unsigned int entryNum, i;
	MIB_CE_IP_ROUTE_T Entry;
	unsigned long int d,g,m;
	struct in_addr dest;
	struct in_addr gw;
	struct in_addr mask;
	char sdest[16], sgw[16];
	char interface_name[30];
	
	entryNum = mib_chain_total(MIB_IP_ROUTE_TBL);
	
	printf("\n-------------------------------------------------------------------------------\n");
	printf("                       Static Route Table                                    \n");
	printf("-------------------------------------------------------------------------------\n");
	//printf("Idx  Subnet Mask      NextHop          Iface\n");
	printf("Idx  State    Destination      Subnet Mask      NextHop          Metric   Iface\n");
	printf("-------------------------------------------------------------------------------\n");
	
	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_IP_ROUTE_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return;
		}
			
		dest.s_addr = *(unsigned long *)Entry.destID;
		gw.s_addr   = *(unsigned long *)Entry.nextHop;
		mask.s_addr = *(unsigned long *)Entry.netMask;
		// inet_ntoa is not reentrant, we have to
		// copy the static memory before reuse it
		strcpy(sdest, inet_ntoa(dest));
		strcpy(sgw, inet_ntoa(gw));
		if (Entry.ifIndex == 0xff)
			strcpy( interface_name, "---" );
		else if ((Entry.ifIndex & 0xf0) == 0xf0)
			sprintf(interface_name, "vc%u", VC_INDEX(Entry.ifIndex));
		else
			sprintf(interface_name, "ppp%u", PPP_INDEX(Entry.ifIndex));
		//printf("%-5d%-17s%-17s%s\n",i+1, sdest, inet_ntoa(mask), sgw);
		printf("%-5d%-9s%-17s%-17s%-17s%-9d%s\n", i+1, Entry.Enable ? "Enable" : "Disable", sdest, inet_ntoa(mask), sgw, Entry.FWMetric, interface_name);
	}
	
	return 0;
}

int routeIflist(){
	int ifnum=0;
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	unsigned char selcnt = 0;

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	printf("Idx  Interface\n");
	printf("----------------\n");
	printf("0    Any\n");
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) {
  			printf("Get chain record error!\n");
			return -1;
		}
		
		if (Entry.enable == 0)
			continue;
		
		if (PPP_INDEX(Entry.ifIndex) != 0x0f) {	// PPP interface
			selcnt ++;
			printf("%-5dppp%u\n", selcnt, PPP_INDEX(Entry.ifIndex));
		}
		else {	// vc interface
			selcnt ++;
			printf("%-5dvc%u\n", selcnt, VC_INDEX(Entry.ifIndex));
		}
		ifnum ++;
	}

	return ifnum;
}

//tylo
//void addroutes(char *destNet,char *subMask,char *nextHop){
void addroutes()
{
	MIB_CE_IP_ROUTE_T entry;
	MIB_CE_ATM_VC_T vcEntry;
	int min, max, num, intVal, i;
	unsigned char destNet[4];

	if (0 == getInputIpAddr("Destination IP address: ", (struct in_addr *)&entry.destID))
		return;
	if (0 == getInputIpMask("Subnet Mask: ", (struct in_addr *)&entry.netMask))
		return;
	for (i=0; i<4; i++) {
		destNet[i] = entry.destID[i] & entry.netMask[i];
		if (destNet[i] != entry.destID[i]) {
			printf("The specified mask parameter is invalid. (Destination & Mask) != Destination.\n");
			return;
		}
	}

	min = 0; max = 65535;
	intVal=getTypedInputDefault(INPUT_TYPE_UINT,"Metric[1]: ",&num,&min,&max);
	if (intVal == 0)
		return;
	else if (intVal == -2)
		entry.FWMetric = 1;
	else
		entry.FWMetric = num;

	max = routeIflist();
	min = 0;
	if (getInputUint("Select interface: ", &num, &min, &max) == 0) {
		printf("Invalid selection!\n");
		return;
	}
	if (num == min) {
		entry.ifIndex = 0xff;
		if (0 == getInputIpAddr("Next Hop: ", (struct in_addr *)&entry.nextHop))
			return;
	}
	else {
		mib_chain_get(MIB_ATM_VC_TBL, num-1, (void *)&vcEntry);
		entry.ifIndex = vcEntry.ifIndex;
		getInputIpAddr("Next Hop: ", (struct in_addr *)&entry.nextHop);
	}

	max=2;
	if (0 == getInputUint("State (1)Enable (2)Disable : ", &num, &min, &max))
		return;
	if (num==1)
		entry.Enable = 1;
	else
		entry.Enable = 0;

	if (!checkRoute(entry, -1)) {
		printf(Tinvalid_rule);
		printf("\n");
		return;
	}
	
	intVal = mib_chain_add(MIB_IP_ROUTE_TBL, (unsigned char*)&entry);
	if (intVal == 0) {
		printf(strAddChainerror);
		return;
	}
	else if (intVal == -1) {
		printf(strTableFull);
		return;
	}
	route_cfg_modify(&entry, 0);
}

//tylo
void delroutes(){
	
	unsigned int i;
	int min,max,sel,skfd;
	unsigned int idx;
	MIB_CE_IP_ROUTE_T Entry;
	struct rtentry rt;
	unsigned int totalEntry = mib_chain_total(MIB_IP_ROUTE_TBL); /* get chain record size */
	struct sockaddr_in *s_in;

	CLEAR;
	showStaticRoutes();
	
	min=1;max=totalEntry;
	if(getInputUint("Select index:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}

	idx = sel-1;			
				
	/* get the specified chain record */
	if (!mib_chain_get(MIB_IP_ROUTE_TBL, idx, (void *)&Entry))
		return;
	/* Clean out the RTREQ structure. */
	memset((char *) &rt, 0, sizeof(struct rtentry));
	rt.rt_flags = RTF_UP;
	s_in = (struct sockaddr_in *)&rt.rt_dst;
	s_in->sin_family = AF_INET;
	s_in->sin_port = 0;
	s_in->sin_addr = *(struct in_addr *)Entry.destID;
					
	s_in = (struct sockaddr_in *)&rt.rt_genmask;
	s_in->sin_family = AF_INET;
	s_in->sin_port = 0;
	s_in->sin_addr = *(struct in_addr *)Entry.netMask;
					
	s_in = (struct sockaddr_in *)&rt.rt_gateway;
	s_in->sin_family = AF_INET;
	s_in->sin_port = 0;
	s_in->sin_addr = *(struct in_addr *)Entry.nextHop;
					
	rt.rt_flags |= RTF_GATEWAY;
					
	// delete from chain record
	if(mib_chain_delete(MIB_IP_ROUTE_TBL, idx) != 1) {
		printf("Delete chain record error!");
		return;
	}
					
	/* Create a socket to the INET kernel. */
	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("Create socket error!\n");
		return;
	}
					
	/* Tell the kernel to delete this route. */
	if (ioctl(skfd, SIOCDELRT, &rt) < 0) {
		printf("kernel delete route error!\n");
		close(skfd);
		return;
	}
					
	/* Close the socket. */
	(void) close(skfd);
	return;
}

#ifdef DEFAULT_GATEWAY_V2
//Jenny
unsigned char DGWsel[8];
int DGWlist(){
	int ifnum=0;
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	int type=1;//rt
	unsigned char DGWselcnt = 0;

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	printf("Idx  Interface\n");
	printf("----------------\n");
	printf("0    None\n");
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}
		
		if (Entry.enable == 0)
			continue;
		
		if (type == 2) {
			if (Entry.cmode == ADSL_BR1483)
			{
				DGWsel[DGWselcnt] = Entry.ifIndex;
				DGWselcnt ++;
				printf("%-5dvc%u\n",	DGWselcnt, VC_INDEX(Entry.ifIndex));
				ifnum ++;
			}
		}
		else { // rt or all (1 or 0)
			if (type == 1 && Entry.cmode == ADSL_BR1483)
				continue;
			if (PPP_INDEX(Entry.ifIndex) != 0x0f)
			{	// PPP interface
				DGWsel[DGWselcnt] = Entry.ifIndex;
				DGWselcnt ++;
				printf("%-5dppp%u\n", DGWselcnt, PPP_INDEX(Entry.ifIndex));
			}
			else
			{	// vc interface
				DGWsel[DGWselcnt] = Entry.ifIndex;
				DGWselcnt ++;
				printf("%-5dvc%u\n", DGWselcnt, VC_INDEX(Entry.ifIndex));
			}
			ifnum ++;
		}
	}
#ifdef AUTO_PPPOE_ROUTE
	ifnum ++;
	printf("%d    Auto\n", ifnum);
#endif

	return ifnum;
}

//Jenny
void showDGW(){
	unsigned char vChar;

	mib_get( MIB_ADSL_WAN_DGW_ITF, (void *)&vChar);
	if (vChar == DGW_NONE)
		printf("Default Gateway: None\n");
#ifdef AUTO_PPPOE_ROUTE
	else if (vChar == DGW_AUTO)
		printf("Default Gateway: Auto\n");
#endif
	else {
		if (PPP_INDEX(vChar) != 0x0f)		// PPP interface
			printf("Default Gateway: ppp%u\n", PPP_INDEX(vChar));
		else		// vc interface
			printf("Default Gateway: vc%u\n", VC_INDEX(vChar));
	}
}

//Jenny
void setDGW(){
	int min, max, sel;
	unsigned char dgw_itf;

	max = DGWlist();
	min = 0;
	if(getInputUint("Select interface: ", &sel, &min, &max) == 0) {
		printf("Invalid selection!\n");
		return;
	}

	if (sel == min)
		dgw_itf = DGW_NONE;
#ifdef AUTO_PPPOE_ROUTE
	else if (sel == max)
		dgw_itf = DGW_AUTO;
#endif
	else
		dgw_itf = DGWsel[sel-1];
	
	if ( !mib_set(MIB_ADSL_WAN_DGW_ITF, (void *)&dgw_itf)) {
		printf("Set default gateway interface index error!\n");
		return;
	}
	return;
}
#endif

//tylo
void setRouting()
{
   //char destNet[16], subMask[16], nextHop[16];
   int snum;

   while (1)
   {
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                           Routing Configuration                         \n");
		printf("-------------------------------------------------------------------------\n");
		printf("This page is used to configure the routing information. Here you can     \n");
		printf("add/delete IP routes.                                                    \n");
		//printf("-------------------------------------------------------------------------\n");
#ifdef DEFAULT_GATEWAY_V2
		printf("-------------------------------------------------------------------------\n");
		showDGW();
#endif
		//printf("Static Route Table:\n");
		//printf("Select Destination Subnet Mask NextHop\n");
		showStaticRoutes();
		printf("-------------------------------------------------------------------------------\n");
		printf("(1) Add                          (2) Delete\n");
#ifdef DEFAULT_GATEWAY_V2
		printf("(3) Show Routes                  (4) Set Default Gateway\n");
		printf("(5) Quit\n");
      if (!getInputOption( &snum, 1, 5))
#else
		printf("(3) Show Routes                  (4) Quit\n");
      if (!getInputOption( &snum, 1, 4))
#endif
      	continue;
      
      switch( snum)
		{
		case 1://(1) Add
		if (!check_access(SECURITY_SUPERUSER))
			break;
		CLEAR;
		addroutes();
         break;
      case 2://(2) Delete
	if (!check_access(SECURITY_SUPERUSER))
		break;
      	  delroutes();
         break;
      case 3://(3) Show Routes
         showRoutes();
         break;
#ifdef DEFAULT_GATEWAY_V2
      case 4://(4) Set Default Gateway
         setDGW();
         break;
      case 5://(5) Quit
#else
      case 4://(4) Quit
#endif
         return;
      }//end switch, Routing Configuration 
   }//end while, Routing Configuration 
}

//tylo
int validateKey(char *str)
{
	int i,len;
	len=strlen(str);
   	for (i=0; i<len; i++) {
		if ( (str[i] >= '0' && str[i] <= '9') ||(str[i] == '.' ) )
			continue;
		return 0;
  	}
  	return 1;
}

//tylo
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
void SNMPconfig(char *snmpSysDescr,char *snmpSysContact,char *snmpSysName,char *snmpSysLocation,
				char *snmpSysObjectID,char *snmpTrapIpAddr,char *snmpCommunityRO,char *snmpCommunityRW){
	char *str, *submitUrl;
	struct in_addr trap_ip;
	static char tmpBuf[100];

	str = snmpTrapIpAddr;
	if(str[0]==0)
		goto setErr_pass;

	if ( !inet_aton(str, &trap_ip) ) {
		printf("Invalid Trap IP value!\n");
		goto setErr_pass;
	}
	if ( !mib_set(MIB_SNMP_TRAP_IP, (void *)&trap_ip)) {
		printf("Set snmpTrapIpAddr mib error!\n");
		goto setErr_pass;
	}

	str = snmpSysObjectID;
	if(str[0]==0)
		goto setErr_pass;

	if(!validateKey(str)){
		printf("Invalid Object ID value. It should be fill with OID string.\n");
		goto setErr_pass;
	}
	if ( !mib_set(MIB_SNMP_SYS_OID, (void *)str)) {
		printf("Set snmpSysObjectID mib error!\n");
		goto setErr_pass;
	}

	str = snmpSysDescr;
	if(str[0]==0)
		goto setErr_pass;
	if ( !mib_set(MIB_SNMP_SYS_DESCR, (void *)str)) {
		printf("Set snmpSysDescr mib error!\n");
		goto setErr_pass;
	}

	str = snmpSysContact;
	if(str[0]==0)
		goto setErr_pass;

	if ( !mib_set(MIB_SNMP_SYS_CONTACT, (void *)str)) {
		printf("Set snmpSysContact mib error!\n");
		goto setErr_pass;
	}

	str = snmpSysName;
	if(str[0]==0)
		goto setErr_pass;

	if ( !mib_set(MIB_SNMP_SYS_NAME, (void *)str)) {
		printf("Set snmpSysName mib error!\n");
		goto setErr_pass;
	}

	str = snmpSysLocation;
	if(str[0]==0)
		goto setErr_pass;

	if ( !mib_set(MIB_SNMP_SYS_LOCATION, (void *)str)) {
		printf("Set snmpSysLocation mib error!\n");
		goto setErr_pass;
	}

	str = snmpCommunityRO;
	if(str[0]==0)
		goto setErr_pass;

	if ( !mib_set(MIB_SNMP_COMM_RO, (void *)str)) {
		printf("Set snmpCommunityRO mib error!\n");
		goto setErr_pass;
	}

	str = snmpCommunityRW;
	if(str[0]==0)
		goto setErr_pass;

	if ( !mib_set(MIB_SNMP_COMM_RW, (void *)str)) {
		printf("Set snmpCommunityRW mib error!\n");
		goto setErr_pass;
	}

	printf("Set SNMP config success!\n");
  	return;

 setErr_pass:
	printf("Set SNMP config fail!\n");	

}

void showSNMP(){
	char str[256];
	struct in_addr trap_ip;
	unsigned char enable;
	
	if ( !mib_get( MIB_SNMPD_ENABLE, (void *)&enable)) {
		printf("Get snmpdEnable error!\n");
	}
	printf("SNMP:%s\n", (enable==1)? "enable":"disable");
	
	if ( !mib_get(MIB_SNMP_SYS_DESCR, (void *)str)) {
		printf("Get snmpSysDescr mib error!\n");
	}
	printf("System Description:%s\n",str);
	
	if ( !mib_get(MIB_SNMP_SYS_CONTACT, (void *)str)) {
		printf("Get mib error!\n");
	}
	printf("System Contact:%s\n",str);
	
	if ( !mib_get(MIB_SNMP_SYS_NAME, (void *)str)) {
		printf("Get mib error!\n");
	}
	printf("System Name:%s\n",str);
	
	if ( !mib_get(MIB_SNMP_SYS_LOCATION, (void *)str)) {
		printf("Get mib error!\n");
	}
	printf("System Location:%s\n",str);
	
	if ( !mib_get(MIB_SNMP_SYS_OID, (void *)str)) {
		printf("Get mib error!\n");
	}
	printf("System Object ID:%s\n",str);
	
	if ( !mib_get(MIB_SNMP_TRAP_IP, (void *)&trap_ip)) {
		printf("Get mib error!\n");
	}
	printf("Trap IP Address:%s\n",inet_ntoa(trap_ip));
	
	if ( !mib_get(MIB_SNMP_COMM_RO, (void *)str)) {
		printf("Get mib error!\n");
	}
	printf("Community name (read-only):%s\n",str);
	
	if ( !mib_get(MIB_SNMP_COMM_RW, (void *)str)) {
		printf("Get mib error!\n");
	}
	printf("Community name (read-write):%s\n\n",str);

}


void setSNMP()
{
   char snmpSysDescr[65], snmpSysContact[65], snmpSysName[65], snmpTrapIpAddr[16];
   char snmpSysLocation[65], snmpSysObjectID[65], snmpCommunityRO[65], snmpCommunityRW[65];
   int snum, enable;
   unsigned char set_enable;

   while (1)
   {
	CLEAR;
	printf("\n");
	printf("-------------------------------------------------------------------------\n");
	printf("                      SNMP Protocol Configuration                        \n");
	printf("-------------------------------------------------------------------------\n");
      	printf("This page is used to configure the SNMP protocol. Here you may change the\n");
      	printf("setting for system description, trap ip address, community name, etc.. \n");
	printf("-------------------------------------------------------------------------\n");
	showSNMP();
	
       printf("(1) Set                          (2) Quit\n");
       if (!getInputOption( &snum, 1, 2))
       	continue;
       
      switch( snum)
		{
		case 1://(1) Set
		if (!check_access(SECURITY_SUPERUSER))
			break;
	 // Mason Yu
	 printf("SNMP:(1) enable (2) disable\n");
	 if (!getInputOption( &enable, 1, 2))
       		continue;
       	 if ( enable == 1 )
       	 	set_enable = 1;   // on
       	 else
       	 	set_enable = 0;   // off
       	 mib_set(MIB_SNMPD_ENABLE, (void *)&set_enable);
       	 
       	 if ( enable == 2 )
       	 	goto START_SNMP; 
       	 	
         if (0 == getInputString("System Description :", snmpSysDescr, sizeof(snmpSysDescr)-1))
            continue;
         if (0 == getInputString("System Contact :", snmpSysContact, sizeof(snmpSysContact)-1))
            continue;
         if (0 == getInputString("System Name :", snmpSysName, sizeof(snmpSysName)-1))
            continue;
         if (0 == getInputString("System Location :", snmpSysLocation, sizeof(snmpSysLocation)-1))
            continue;
         if (0 == getInputString("System Object ID :", snmpSysObjectID, sizeof(snmpSysObjectID)-1))
            continue;
         if (0 == getInputString("Trap IP Address:", snmpTrapIpAddr,sizeof(snmpTrapIpAddr)-1))
            continue;
         if (0 == getInputString("Community name (read-only) :", snmpCommunityRO, sizeof(snmpCommunityRO)-1))
            continue;
         if (0 == getInputString("Community name (read-write) :", snmpCommunityRW, sizeof(snmpCommunityRW)-1))
            continue;
         SNMPconfig(snmpSysDescr,snmpSysContact,snmpSysName,snmpSysLocation,snmpSysObjectID,
         		snmpTrapIpAddr,snmpCommunityRO,snmpCommunityRW);
START_SNMP:
#if defined(APPLY_CHANGE)
		if ( enable == 1 )       // on
			restart_snmp(1);
		else 
			restart_snmp(0);  // off
#endif
         break;
      case 2://(2) Quit
         return;
      }//end switch, SNMP Protocol Configuration 
   }//end while, SNMP Protocol Configuration 
}
#endif

//tylo
unsigned char BridgeVC[8];
int showBridgeVC()
{
	int ifnum=0;
	
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	char ifname[6];
	char_t  buffer[3];
	
	// check WAN
	printf("Idx  Interface\n");
	printf("-------------------\n");
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}
		
		if (Entry.enable == 0)
			continue;
		

		if (Entry.cmode == ADSL_BR1483)
		{
			snprintf(ifname, 6, "vc%u", VC_INDEX(Entry.ifIndex));
			printf("%-5d%s\n",ifnum+1, ifname);
			BridgeVC[ifnum]=Entry.ifIndex;
			ifnum++;
		}
	}
	return ifnum;
}

// Jenny
unsigned char WanVC[16];
//int showWanVC()
int showWanVC(int dft)
{
	int ifnum=0;
	
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	char ifname[6];
	char_t  buffer[3];
	
	// check WAN
	printf("Idx  Interface\n");
	printf("-------------------\n");
	if (dft)
		printf("0    Default\n");
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}
		
		if (Entry.enable == 0)
			continue;
		

		if (Entry.cmode == ADSL_BR1483)
		{
			snprintf(ifname, 6, "vc%u", VC_INDEX(Entry.ifIndex));
			printf("%-5d%s\n",ifnum+1, ifname);
			WanVC[ifnum]=Entry.ifIndex;
			ifnum++;
		}
		else {
			if (PPP_INDEX(Entry.ifIndex) != 0x0f)
			{	// PPP interface
				snprintf(ifname, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));
				printf("%-5d%s\n",ifnum+1, ifname);
				WanVC[ifnum]=Entry.ifIndex;
			}
			else
			{	// vc interface
				snprintf(ifname, 6, "vc%u", VC_INDEX(Entry.ifIndex));
				printf("%-5d%s\n",ifnum+1, ifname);
				WanVC[ifnum]=Entry.ifIndex;
			}
			ifnum++;
		}
	}
	return ifnum;
}

#ifdef CONFIG_EXT_SWITCH
#ifdef ITF_GROUP
// int grp=0: default group
// int grp=1: Group 1
// int grp=2: Group 2
// int grp=3: Group 3
// int grp=4: Group 4

// int action=0: printf all interface
// int action=1: printf all interface to be added
// int action=2: printf all interface to be removed
int display_grp_ifName(int grp, int action)
{	
	int i, ifnum, num;
	struct itfInfo itfs[16];
	char availitf[512];	
	char tmp[5];	
	
	ifnum = get_group_ifinfo(itfs, 16, grp);	
	availitf[0]=0;
	if (ifnum>=1) {
		strncat( availitf, "(1)", 64);
		strncat( availitf, itfs[0].name, 64);		
		
		ifnum--;
		for (i=0; i<ifnum; i++) {
			sprintf(tmp, "(%d)", i+2);
			strncat(availitf, tmp, 64);
			strncat(availitf, itfs[i+1].name, 64);			
		}
		++ifnum;
	}
	
	if ( action == PM_PRINT ) {
		if ( availitf[0] == 0 )
			printf("The Group %d does not include any interface.\n", grp);
		else
			printf("The Group %d include %s interfaces.\n", grp, availitf);
	} else if ( action == PM_ADD ) {
		if ( availitf[0] == 0 )
			printf("There is no any available interface that can be added into Group %d.\n", grp);
		else
			printf("Please select one of the following interfaces and add it into Group.\n%s\n", availitf);
	} else if  ( action == PM_REMOVE ) {
		if ( availitf[0] == 0 )
			printf("There is no any available interface that can be removed from Group %d.\n", grp);
		else
			printf("Please select one of the following interfaces and remove it from Group %d.\n%s\n", grp, availitf);
	} else
		printf("display_grp_ifName: error action.\n");
		
	return ifnum;
	
}

// int grp=0: default group
// int grp=1: Group 1
// int grp=2: Group 2
// int grp=3: Group 3
// int grp=4: Group 4
char *get_all_IFID(int grp)
{	
	int i, ifnum;
	struct itfInfo itfs[16];	
	char *availval;	
	
	ifnum = get_group_ifinfo(itfs, 16, grp);	
	availval = (char *)malloc(128);
	availval[0]=0;
	if (ifnum>=1) {		
		snprintf( availval, 64, "%d", IF_ID(itfs[0].ifdomain, itfs[0].ifid));		
		
		ifnum--;
		for (i=0; i<ifnum; i++) {			
			snprintf( availval, 64, "%s,%d", availval, IF_ID(itfs[i+1].ifdomain, itfs[i+1].ifid));
		}
	}
	//printf("The group %d include [%s]\n", grp, availval);
	return (char *)availval;
	
}

// int grp=0: default group
// int grp=1: Group 1
// int grp=2: Group 2
// int grp=3: Group 3
// int grp=4: Group 4
char *get_specify_one_IFID(int grp, int sel)
{	
	int i, ifnum;
	struct itfInfo itfs[16];	
	char *availval;	
	
	ifnum = get_group_ifinfo(itfs, 16, grp);	
	availval = (char *)malloc(10);
	availval[0]=0;
	if (ifnum>=1) {		
		for (i=0; i<ifnum; i++) {
			if ( i == (sel-1)) {			
				snprintf( availval, 64, "%d", IF_ID(itfs[i].ifdomain, itfs[i].ifid));
				//printf("The No%d IFID in group %d is [%s]\n", sel, grp, availval);
				return (char *)availval;			
			}
			
		}
	}	
	return (char *)availval;
	
}

// int grp=0: default group
// int grp=1: Group 1
// int grp=2: Group 2
// int grp=3: Group 3
// int grp=4: Group 4
int del_IFID_from_group(int grp, int sel)
{	
	int i, ifnum;
	struct itfInfo itfs[16];	
	char *availval;	
	char del_availval[64];
	
	ifnum = get_group_ifinfo(itfs, 16, grp);	
	availval = (char *)malloc(128);
	availval[0]=0;
	if (ifnum>=1) {				
		for (i=0; i<ifnum; i++) {			
			if ( (sel-1) != i ) {
				if ( availval[0] == 0 ) 
					snprintf( availval, 64, "%d", IF_ID(itfs[i].ifdomain, itfs[i].ifid));
				else			
					snprintf( availval, 64, "%s,%d", availval, IF_ID(itfs[i].ifdomain, itfs[i].ifid));
			} else
				snprintf( del_availval, 64, "%d", IF_ID(itfs[i].ifdomain, itfs[i].ifid));
		}
		
	}
	//printf("The remain IFID in group %d is [%s]\n", grp, availval);
		
	// Set Port Mapping
	if ( availval[0]==0 ) {
		//printf("del_availval=%s\n", del_availval);
		setgroup(del_availval, 0);
	} else {
		setgroup(availval, grp);
		setgroup(del_availval, 0);
	}
	
	free(availval);
	return 1;	
}

int get_port_mapping_status()
{
	unsigned char mode=0;
#ifdef CONFIG_EXT_SWITCH
	mib_get(MIB_MPMODE, (void *)&mode);
	if(mode&0x01)
		return 1;
	else
		return 0;
#endif
}

void config_grp(enum PortMappingGrp grp){
	int snum, num;
	char *default_IFID, *select_IFID;
	
	while (1)
   	{
   		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                      Group %d Configuration                        \n", grp);
		printf("-------------------------------------------------------------------------\n");
		// Get all interface from specify gruop
       		display_grp_ifName(grp, PM_PRINT);
		printf("\n");
		printf("Please select one of the following methods.\n");			
       		printf("(1) Add interface				(2) Remove Interface\n");
       		printf("(3) Quit\n");       		
       		if (!getInputOption( &snum, 1, 3))
       			continue;
   		switch( snum)
		{
			case 1: // (1) Add interface into specify group
				if ( get_port_mapping_status() == 1 ) {    				
					num = display_grp_ifName(PM_DEFAULTGRP, PM_ADD);
					if ( num >= 1 ) {
						if (!getInputOption( &snum, 1, num))
       							continue;       				
       						
       						// set specify Group
       						select_IFID = get_specify_one_IFID(PM_DEFAULTGRP, snum);  // get select IFID for default Group
       						setgroup(select_IFID, grp);
       						free(select_IFID);
       						
       						// set default Group
       						default_IFID = get_all_IFID(PM_DEFAULTGRP);       						
       						setgroup(default_IFID, PM_DEFAULTGRP);
       						free(default_IFID);
       					} else
       						printWaitStr();
       					
       					// Take effect
       					setupEth2pvc();					
       				} else {
       					printf("Please enable Port Mapping first.\n");
       					printWaitStr();
       				}       					
         			break;
         		case 2: // (2) Remove Interface from specify group
         			if ( get_port_mapping_status() == 1 ) {         				
         				num = display_grp_ifName(grp, PM_REMOVE);
         				if ( num >= 1 ) {
						if (!getInputOption( &snum, 1, num))
       							continue;
       					
       						del_IFID_from_group(grp, snum);
       					} else
       						printWaitStr();
       					
       					// Take effect
       					setupEth2pvc();	
       				} else {
       					printf("Please enable Port Mapping first.\n");
       					printWaitStr();
       				}      					
         			break;         		
     	 		case 3: //(3) Quit
         			return;
      		}//end switch       		
   	} // end while
}

void setPortMapping(){
	int snum;
    	unsigned char mode;
    	
	while (1)
   	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                      Port Mapping Configuration                         \n");
		printf("-------------------------------------------------------------------------\n");
      		printf("This page is used to configure the Port mapping. Here you may change the \n");
      		printf("Group interface with ADD/Remove method\n");
		printf("-------------------------------------------------------------------------\n");
		printf("Port Mapping: %s\n", (get_port_mapping_status())? "enabled":"disabled");
		printf("\n");		
       		printf("(1) Enable/Disable Port Mapping			(2) Config Group 1\n");
       		printf("(3) Config Group 2				(4) Config Group 3\n");
       		printf("(5) Config Group 4				(6) Quit\n");
       		if (!getInputOption( &snum, 1, 6))
       			continue;
       
      		switch( snum)
		{
			case 1://(1) Enable/Disable Port Mapping
				if (!check_access(SECURITY_SUPERUSER))
					break;
	 			
				printf("Port Mapping:(1) enable (2) disable\n");
				if (!getInputOption( &snum, 1, 2))
       					continue;  
       					
				mib_get(MIB_MPMODE, (void *)&mode);				
				if ( snum == 1 ) {					
					mode |= 0x01;
				}else
					mode &= 0xfe;
				mib_set(MIB_MPMODE, (void *)&mode);
				
				if(mode&0x01)
					setupEth2pvc();
				else {					
					setupEth2pvc_disable();					
				}
         			break;
         		case 2: // (2) Config Group 1
         			config_grp(PM_GROUP1);
         			break;
         		case 3: // (3) Config Group 2
         			config_grp(PM_GROUP2);
         			break;
         		case 4: // (4) Config Group 3
         			config_grp(PM_GROUP3);
         			break;
         		case 5: // (5) Config Group 4
         			config_grp(PM_GROUP4);
         			break;
     	 		case 6://(2) Quit
         			return;
      		}//end switch, Port Mappping Configuration 
   	}//end while, Port Mapping Configuration
}

//tylo
void eth2pvclist()
{
	unsigned int entryNum, i;
	MIB_CE_SW_PORT_T Entry;
	
	entryNum = mib_chain_total(MIB_SW_PORT_TBL);
	getSYS2Str(SYS_PORT_MAPPING, strbuf);
	printf("Port Mapping\t: %s\n\n", strbuf);
	printf("Current Port Mapping\n");
	printf("---------------------\n");
	for (i=1; i<=entryNum; i++) {
		if (!mib_chain_get(MIB_SW_PORT_TBL, entryNum-i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return;
		}
		if(Entry.pvcItf==0xff)
			printf("LAN%d   Default\n",i);
		else
			if (PPP_INDEX(Entry.pvcItf) != 0x0f)		// PPP interface
				printf("LAN%d   ppp%u\n",i, PPP_INDEX(Entry.pvcItf));
			else		// vc interface
				printf("LAN%d   vc%u\n", i, VC_INDEX(Entry.pvcItf));
//			printf("port%d   VC%d\n", i, VC_INDEX(Entry.pvcItf));
	}
	printf("---------------------\n");
}
#endif	// of ITF_GROUP

//tylo




#ifdef ELAN_LINK_MODE
char port_value[4]={ 3, 2, 1, 0 };

//tylo
void showLinkmode(){
	int i;
	MIB_CE_SW_PORT_T Entry;

	printf("Current Link Mode\n");
	for(i=0;i<SW_PORT_NUM;i++){
		printf("LAN%d:  ",port_value[i]+1);
		if (!mib_chain_get(MIB_SW_PORT_TBL, i, (void *)&Entry))
			continue;
		if(Entry.linkMode==LINK_10HALF)
			printf("10 Half Mode\n");
		else if(Entry.linkMode==LINK_10FULL)
			printf("10 Full Mode\n");
		else if(Entry.linkMode==LINK_100HALF)
			printf("100 Half Mode\n");
		else if(Entry.linkMode==LINK_100FULL)
			printf("100 Full Mode\n");
		else
			printf("Auto Mode\n");
	}
}

//tylo
void setLink(){
	int min,max,sel,sel2;
	MIB_CE_SW_PORT_T Entry;

	min=1;max=4;
	//if(getInputUint("Select switch port (1)eth0_sw0 (2)eth0_sw1 (3)eth0_sw2 (4)eth0_sw3 :",&sel,&min,&max)==0){
	if(getInputUint("Select switch port (1)LAN1 (2)LAN2 (3)LAN3 (4)LAN4 :",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}

	if(getInputUint("Select switch port (1)10 Half (2)10 Full (3)100 Half (4)100 Full (5)Auto Mode :",&sel2,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}

	if (!mib_chain_get(MIB_SW_PORT_TBL, port_value[sel-1], (void *)&Entry))
		return;
	Entry.linkMode = sel2-1;
	mib_chain_update(MIB_SW_PORT_TBL, (void *)&Entry, port_value[sel-1]);
	return;
}

//tylo
void setLinkmode(){
	int snum;
	unsigned char vChar;
	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                     ethernet Link Speed/Duplex Mode                     \n");
		printf("-------------------------------------------------------------------------\n");
      		printf("Set the Ethernet link speed/duplex mode. \n ");
		printf("-------------------------------------------------------------------------\n");
		showLinkmode();
		printf("------------------------------------\n");
		printf("(1) Set link			(2) Quit\n");
      		if (!getInputOption( &snum, 1, 2))
      			continue;
      		
      		switch(snum){
			case 1:
				setLink();
#if defined(APPLY_CHANGE)
				setupLinkMode();	
#endif
				break;
			case 2:
				return;
      		}

	}

}
#endif	// of ELAN_LINK_MODE

#else // of CONFIG_EXT_SWITCH

#ifdef ELAN_LINK_MODE_INTRENAL_PHY
//tylo
void showLinkmode(){
	int i;
	unsigned char mode;
	printf("Current Link Mode\n");
	if (!mib_get(MIB_ETH_MODE, (void *)&mode))
		return;
	if(mode==LINK_10HALF)
		printf("10 Half Mode\n");
	else if(mode==LINK_10FULL)
		printf("10 Full Mode\n");
	else if(mode==LINK_100HALF)
		printf("100 Half Mode\n");
	else if(mode==LINK_100FULL)
		printf("100 Full Mode\n");
	else
		printf("Auto Mode\n");

}

//tylo
void setLink(){
	int min,max,sel,sel2;
	unsigned char mode;
	//MIB_CE_SW_PORT_T Entry;

	min=1;max=5;
	//if(getInputUint("Select switch port (1)eth0_sw0 (2)eth0_sw1 (3)eth0_sw2 (4)eth0_sw3 :",&sel,&min,&max)==0){
	//	printf("Invalid selection!\n");
	//	return;
	//}

	if(getInputUint("Select switch port (1)10 Half (2)10 Full (3)100 Half (4)100 Full (5)Auto Mode :",&sel2,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}

	mode = (unsigned char)(sel2 - 1);
	mib_set(MIB_ETH_MODE, (void *)&mode);

	restart_ethernet(1);
}

//tylo
void setLinkmode(){
	int snum;
	unsigned char vChar;
	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                     Ethernet Link Speed/Duplex Mode                     \n");
		printf("-------------------------------------------------------------------------\n");
      		printf("Set the Ethernet link speed/duplex mode. \n ");
		printf("-------------------------------------------------------------------------\n");
		showLinkmode();
		printf("------------------------------------\n");
		printf("(1) Set link			(2) Quit\n");
      		if (!getInputOption( &snum, 1, 2))
      			continue;
      		
      		switch(snum){
			case 1:
				setLink();
				break;
			case 2:
				return;
      		}

	}

}
#endif

#endif	// of CONFIG_EXT_SWITCH

//tylo
#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
int QosList()
{
	unsigned int entryNum, i, k;
	MIB_CE_IP_QOS_T Entry;
	char *psip, *pdip, sip[20], dip[20];
	const char *type;
	char sport[10], dport[10];
	unsigned int mask, smask, dmask;
	char strPhy[]="LAN0", *pPhy;
	//char_t  buffer[3];
	const char *pPrio, *pIPrio, *pTos, *p1p;
#ifdef NEW_IP_QOS_SUPPORT
#ifdef QOS_DSCP_MATCH
	char mDscp[5];
#endif
	char pDscp[5];
#endif

	entryNum = mib_chain_total(MIB_IP_QOS_TBL);
#ifdef NEW_IP_QOS_SUPPORT
#ifndef QOS_DSCP_MATCH
	printf("%-4s%-19s%-8s%-19s%-8s%-7s%-9s%-6s%-7s%-6s%-8s%s\n", 
		"Idx", "SrcIP", "SrcPort", "DstIP", "DstPort", "Proto", "LanPort", "Prio", "IPrec", "DSCP", "802.1p", "IPTos");
#else
	printf("%-4s%-19s%-8s%-19s%-8s%-7s%-9s%-6s%-6s%-7s%-9s%-8s%s\n", 
		"Idx", "SrcIP", "SrcPort", "DstIP", "DstPort", "Proto", "LanPort", "dscp", "Prio", "IPrec", "IP_DSCP", "802.1p", "IPTos");
#endif
#else
  	//printf("Idx  Src IP               Src Port  Des IP               Des Port  Protocol  Phy Port  Priority  Outbound\n");
	//printf("     Traffic Classification Rules                                                     Mark\n");
  	printf("Idx  Src IP               Src Port  Des IP               Des Port  Protocol Lan Port Priority IP Precd IP Tos               Wan 802.1p\n");
#endif

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_IP_QOS_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}
		//source ip
		psip = inet_ntoa(*((struct in_addr *)Entry.sip));
		if ( !strcmp(psip, "0.0.0.0"))
			psip = (char *)BLANK;
		else {
			if (Entry.smaskbit==0)
				snprintf(sip, 20, "%s", psip);
			else
				snprintf(sip, 20, "%s/%d", psip, Entry.smaskbit);
			psip = sip;
		}
		// source port
		if (Entry.sPort == 0)
			strcpy(sport, BLANK);
		else
			snprintf(sport, 10, "%d", Entry.sPort);
		
		// destination ip
		pdip = inet_ntoa(*((struct in_addr *)Entry.dip));
		if ( !strcmp(pdip, "0.0.0.0"))
			pdip = (char *)BLANK;
		else {
			if (Entry.dmaskbit==0)
				snprintf(dip, 20, "%s", pdip);
			else
				snprintf(dip, 20, "%s/%d", pdip, Entry.dmaskbit);
			pdip = dip;
		}
		// destination port
		if (Entry.dPort == 0)
			strcpy(dport, BLANK);
		else
			snprintf(dport, 10, "%d", Entry.dPort);
		
		if ( Entry.protoType == PROTO_TCP )
			type = ARG_TCP;
		else if ( Entry.protoType == PROTO_UDP )
			type = ARG_UDP;
		else if ( Entry.protoType == PROTO_ICMP )
			type = ARG_ICMP;
		else
			type = (char *)BLANK;
		
		// Assign Priority
		if (Entry.prior <= (IPQOS_NUM_PRIOQ-1))
			pPrio = prioLevel[Entry.prior];
		else
			// should not be here !!
			pPrio = prioLevel[2];

		if (Entry.m_ipprio == 0)
			pIPrio = BLANK;
		else
			pIPrio = n0to7[Entry.m_ipprio];
		
		if (Entry.m_iptos == 0xff)
			pTos = BLANK;
		else {
			int mask, i;
			mask = i = 1;
			while (i <= 5) {
				if (Entry.m_iptos & mask)
					break;
				else {
					i++;
					mask<<=1;
				}
			}
			if (i >= 6)
				i = 1;
			pTos = ipTos[i];
		}

#ifdef NEW_IP_QOS_SUPPORT
		if (Entry.m_dscp == 0)
			strcpy(pDscp, BLANK);
		else
			snprintf(pDscp, 5, "%d", Entry.m_dscp>>2);

#ifdef QOS_DSCP_MATCH
		if (Entry.qosDscp == 0)
			strcpy(mDscp, BLANK);
		else
			snprintf(mDscp, 5, "%d", Entry.qosDscp>>2);
#endif
#endif

		if (Entry.m_1p == 0)
			p1p = BLANK;
		else
			p1p = n0to7[Entry.m_1p];

		pPhy = strPhy;
		if (Entry.phyPort == 0xff)
			pPhy = (char *)BLANK;
#if (defined(CONFIG_EXT_SWITCH)  && defined(IP_QOS_VPORT))
		else if (Entry.phyPort < SW_PORT_NUM)
			strPhy[3] = '0' + virt2user[Entry.phyPort];
#else
		else if (Entry.phyPort == 0)
			strPhy[3] = '0';
#endif
#ifdef CONFIG_USB_ETH
		else if (Entry.phyPort == IFUSBETH_PHYNUM)
			pPhy = (char *)USBETHIF;
#endif //CONFIG_USB_ETH
#ifdef WLAN_SUPPORT
		else
			pPhy = (char *)WLANIF;
#endif //ZTE_GENERAL_ROUTER_SC
#ifdef NEW_IP_QOS_SUPPORT
#ifndef QOS_DSCP_MATCH
		printf("%-4d%-19s%-8s%-19s%-8s%-7s%-9s%-6s%-7s%-6s%-8s%s\n",
			i+1, psip, sport, pdip, dport, type, pPhy, pPrio, pIPrio, pDscp, p1p, pTos);
#else
		printf("%-4d%-19s%-8s%-19s%-8s%-7s%-9s%-6s%-6s%-7s%-9s%-8s%s\n", 
			i+1, psip, sport, pdip, dport, type, pPhy, mDscp, pPrio, pIPrio, pDscp, p1p, pTos);
#endif
#else
		printf("%-5d%-21s%-10s%-21s%-10s%-9s%-9s%-9s%-9s%-21s%s\n",
			i+1, psip, sport, pdip, dport, type, pPhy, pPrio, pIPrio, pTos, p1p);
#endif
		/*else
			strPhy[3] = '0' + virt2user[Entry.phyPort];
		
		printf("%-5d%-21s%-10s%-21s%-10s%-10s%-10s%-10d",
			i+1,psip, sport, pdip, dport, type, pPhy, Entry.prior);
		
		if (PPP_INDEX(Entry.outif) != 0x0f)
		{	// PPP interface
			printf("ppp%u\n",PPP_INDEX(Entry.outif));
     		}
     		else
		{	// vc interface
			printf("vc%u \n",VC_INDEX(Entry.outif));
     		}
      			*/
	}

	//snprintf(buffer, 3, "%u", entryNum);
	//ejSetResult(eid, buffer);
	return entryNum;
}

//tylo
void Qosadd(){
	char strSAddr[18], strDAddr[18], strSport[10], strDport[10];
	int intVal, retVal;
	int min,max,sel;
	unsigned int  totalEntry, i;
	MIB_CE_IP_QOS_T entry;
	unsigned long  mask, mbit;
	unsigned char vChar;
	char strVal[5];
	char tmpstr[256] = {0};
	const char *pPrio;
	struct itfInfo itfs[16];
	int type, ifnum = 0, itfid, itfdomain;
	struct in_addr ipaddr;
	
	memset(&entry, 0, sizeof(entry));
	totalEntry = mib_chain_total(MIB_IP_QOS_TBL); /* get chain record size */
	
#ifdef _CWMP_MIB_
	// enable
	entry.enable = 1;
#endif

	/* Add new qos entry */
	if (totalEntry >= MAX_QOS_RULE)
	{
		printf("Error: Maximum number of Qos rule exceeds !");
		goto setErr_qos;
	}
		
	// protocol type
	min=0;max=3;
	retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Protocol: (1)TCP (2)UDP (3)ICMP [None]: ", &sel, &min, &max);
	if (retVal == 0) {
		printf("Invalid value!\n");
		printWaitStr();
		return;
	} else if (retVal == -2)
		entry.protoType = 0;
	else
		entry.protoType = (unsigned char)sel;
	/*if(getInputUint("Protocol: (0)None (1)TCP (2)UDP (3)ICMP :",&sel,&min,&max)==0){
		printf("Invalid protocol selection!\n");
		return;
	}
	
	entry.protoType = sel;*/
	
	strSAddr[0] = 0;
	retVal = getTypedInputDefault(INPUT_TYPE_IPADDR, "Source IP[None]: ", &ipaddr, 0, 0);
	if (retVal == 0) {
		printf("Invalid value!\n");
		printWaitStr();
		return;
	} else if (retVal==1)
		strcpy(strSAddr, inet_ntoa(ipaddr));

	/*if (0 == getInputString("Source IP :", strSAddr, sizeof(strSAddr)-1))
		return;
		
	if (0 == getInputString("Destination IP :", strDAddr, sizeof(strDAddr)-1))
		return;*/
		
	// Source address
	if (strSAddr[0]) {
		if (!isValidIpAddr(strSAddr)) {
			printf("Invalid IP address %s.\n", strSAddr);
			goto setErr_qos;
		}
		inet_aton(strSAddr, (struct in_addr *)&entry.sip);
		if (entry.sip[0]=='\0') {
			printf("Error! Source IP.");
			goto setErr_qos;
		}

		retVal = getTypedInputDefault(INPUT_TYPE_IPMASK, "Source Netmask[None]: ", &ipaddr, 0, 0);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			strcpy(strSAddr, inet_ntoa(ipaddr));
			if (!isValidNetmask(strSAddr, 1)) {
				printf("Invalid subnet mask %s.\n", strSAddr);
				goto setErr_qos;
			}
			inet_aton(strSAddr, (struct in_addr *)&mask);
			if (mask==0) {
				printf("Error! Source Netmask.");
				goto setErr_qos;
			}
			mask = htonl(mask);
			mbit=0; intVal=0;
			for (i=0; i<32; i++) {
				if (mask&0x80000000) {
					if (intVal) {
						printf("Error! Source Netmask.");
						goto setErr_qos;
					}
					mbit++;
				}
				else
					intVal=1;
				mask <<= 1;
			}
			entry.smaskbit = mbit;
		} else if (retVal == -2)
			entry.smaskbit = 32;
		/*if (0 == getInputString("Source Netmask :", strSAddr, sizeof(strSAddr)-1))
			return;
		
		if (strSAddr[0]) {
			inet_aton(strSAddr, (struct in_addr *)&mask);
			if (mask==0) {
				printf("Error! Source Netmask.");
				goto setErr_qos;
			}
			mask = htonl(mask);
			mbit=0; intVal=0;
			for (i=0; i<32; i++) {
				if (mask&0x80000000) {
					if (intVal) {
						printf("Error! Source Netmask.");
						goto setErr_qos;
					}
					mbit++;
				}
				else
					intVal=1;
				mask <<= 1;
			}
			entry.smaskbit = mbit;
		}
		else
			entry.smaskbit = 32;*/
	}
	
	// source port
	if (entry.protoType == PROTO_TCP || entry.protoType == PROTO_UDP) {
		min = 1; max = 65535;
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Source Port(1~65535) [None]: ", &sel, &min, &max);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == -2)
			entry.sPort =0;
		else
			entry.sPort = (unsigned short)sel;
		/*if (0 == getInputString("Source Port :", strSport, sizeof(strSport)-1))
			return;
	
		if (strSport[0]) {
			if ( !string_to_dec(strSport, &intVal) || intVal<1 || intVal>65535) {
				printf("Error! Invalid source port.");
				goto setErr_qos;
			}
			entry.sPort = (unsigned short)intVal;
		}*/
	}
	// Mason Yu
	//printf("strSport=%s intVal=%d  entry.sPort=%d\n", strSport, intVal, entry.sPort); 
	
	// Destination address
	strDAddr[0] = 0;
	retVal = getTypedInputDefault(INPUT_TYPE_IPADDR, "Destination IP[None]: ", &ipaddr, 0, 0);
	if (retVal == 0) {
		printf("Invalid value!\n");
		printWaitStr();
		return;
	} else if (retVal == 1)
		strcpy(strDAddr, inet_ntoa(ipaddr));

	if (strDAddr[0]) {
		if (!isValidIpAddr(strDAddr)) {
			printf("Invalid IP address %s.\n", strDAddr);
			goto setErr_qos;
		}
		inet_aton(strDAddr, (struct in_addr *)&entry.dip);
		if (entry.dip[0]=='\0') {
			printf("Error! Destination IP.");
			goto setErr_qos;
		}
        	
		retVal = getTypedInputDefault(INPUT_TYPE_IPMASK, "Destination Netmask[None]: ", &ipaddr, 0, 0);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			strcpy(strDAddr, inet_ntoa(ipaddr));
			if (!isValidNetmask(strDAddr, 1)) {
				printf("Invalid subnet mask %s.\n", strDAddr);
				goto setErr_qos;
			}
			inet_aton(strDAddr, (struct in_addr *)&mask);
			if (mask==0) {
				printf("Error! Source Netmask.");
				goto setErr_qos;
			}
			mask = htonl(mask);
			mbit=0; intVal=0;
			for (i=0; i<32; i++) {
				if (mask&0x80000000) {
					if (intVal) {
						printf("Error! Source Netmask.");
						goto setErr_qos;
					}
					mbit++;
				}
				else
					intVal=1;
				mask <<= 1;
			}
			entry.dmaskbit = mbit;
		} else if (retVal == -2)
			entry.dmaskbit = 32;
		/*if (0 == getInputString("Destination Netmask :", strDAddr, sizeof(strDAddr)-1))
			return;
		
		inet_aton(strDAddr, (struct in_addr *)&mask);
		if (strDAddr[0]) {
			if (mask==0) {
				printf("Error! Destination Netmask.");
				goto setErr_qos;
			}
			mask = htonl(mask);
			mbit=0;  intVal=0;
			for (i=0; i<32; i++) {
				if (mask&0x80000000) {
					if (intVal) {
						printf("Error! Destination Netmask.");
						goto setErr_qos;
					}
					mbit++;
				}
				else
					intVal=1;
				mask <<= 1;
			}
			entry.dmaskbit = mbit;
		}
		else
			entry.dmaskbit = 32;*/
	}

	/*// source port
	if (entry.protoType == PROTO_TCP || entry.protoType == PROTO_UDP) {
		if (0 == getInputString("Source Port :", strSport, sizeof(strSport)-1))
			return;
	
		if (strSport[0]) {
			if ( !string_to_dec(strSport, &intVal) || intVal<1 || intVal>65535) {
				printf("Error! Invalid source port.");
				goto setErr_qos;
			}
			entry.sPort = (unsigned short)intVal;
		}
	}*/
	// destination port
	if (entry.protoType == PROTO_TCP || entry.protoType == PROTO_UDP) {
		min = 1; max = 65535;
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Destination Port(1~65535) [None]: ", &sel, &min, &max);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == -2)
			entry.dPort =0;
		else
			entry.dPort = (unsigned short)sel;
		/*if (0 == getInputString("Destination port :", strDport, sizeof(strDport)-1))
			return;
		
		if (strDport[0]) {
			if ( !string_to_dec(strDport, &intVal) || intVal<1 || intVal>65535) {
				printf("Error! Invalid destination port.");
				goto setErr_qos;
			}
			entry.dPort = (unsigned short)intVal;
		}*/
	}

	// physical port
#ifdef CONFIG_USB_ETH
	type = (DOMAIN_ELAN|DOMAIN_WLAN|DOMAIN_ULAN);
#else
	type = (DOMAIN_ELAN|DOMAIN_WLAN);
#endif //CONFIG_USB_ETH
	ifnum = get_domain_ifinfo(itfs, 16, type);
	if (ifnum == 0)
		goto setErr_qos;
	snprintf(tmpstr, sizeof(tmpstr), "Physical Port:");
	for (i=0; i<ifnum; i++) {
		snprintf(tmpstr, sizeof(tmpstr), "%s (%u)%s", tmpstr, i+1, itfs[i].name);
	}
	min = 1; max = ifnum;
	snprintf(tmpstr, sizeof(tmpstr), "%s [None] : ", tmpstr);
	retVal = getTypedInputDefault(INPUT_TYPE_UINT, tmpstr, &sel, &min, &max);
	if (retVal == 0) {
		printf("Invalid value!\n");
		printWaitStr();
		return;
	} else if (retVal == -2)
		entry.phyPort = 0xff;
	else {
		itfid = IF_ID(itfs[sel-1].ifdomain, itfs[sel-1].ifid);
		itfdomain = IF_DOMAIN(itfid);
		itfid = itfid & 0x0ff;
		if (itfdomain == DOMAIN_ELAN)
			entry.phyPort = itfid;
#ifdef WLAN_SUPPORT
		else if (itfdomain == DOMAIN_WLAN)
#ifdef WLAN_MBSSID
			entry.phyPort = 5 + itfid;
#else
			entry.phyPort = 5;	// wlan0
#endif				
#endif
#ifdef CONFIG_USB_ETH
		else if (itfdomain == DOMAIN_ULAN)
			entry.phyPort = IFUSBETH_PHYNUM;	// usb0
#endif //CONFIG_USB_ETH
	}
/*#ifdef CONFIG_USB_ETH
	min=0;max=5;
	if(getInputUint("Physical Port: (0)None (1)LAN1 (2)LAN2 (3)LAN3 (4)LAN4 (5)USB :",&sel,&min,&max)==0)
#else
	min=0;max=4;
	if(getInputUint("Physical Port: (0)None (1)LAN1 (2)LAN2 (3)LAN3 (4)LAN4 :",&sel,&min,&max)==0)
#endif
	{
		printf("Invalid protocol selection!\n");
		return;
	}
	if (sel) {
		for (i=0; i<SW_PORT_NUM; i++) {
			if (virt2user[i] == sel)
				break;
		}
		if (i!=SW_PORT_NUM)
			entry.phyPort = i;
		else
			entry.phyPort = 0xff;
#ifdef CONFIG_USB_ETH
		//the fifth option is usb0
		if (sel==5) entry.phyPort = IFUSBETH_PHYNUM;
#endif //CONFIG_USB_ETH
	}
	else
		entry.phyPort = 0xff;*/

	pPrio = prioLevel[0];
	snprintf(tmpstr, sizeof(tmpstr), "Outbound Priority: (1)%s(highest)", pPrio);
	for (i=1; i<IPQOS_NUM_PRIOQ-1; i++) {
		pPrio = prioLevel[i];
		snprintf(tmpstr, sizeof(tmpstr), "%s (%d)%s", tmpstr, i+1, pPrio);
	}
	pPrio = prioLevel[IPQOS_NUM_PRIOQ-1];
	snprintf(tmpstr, sizeof(tmpstr), "%s (%d)%s(lowest)[Default] : ", tmpstr, IPQOS_NUM_PRIOQ, pPrio);
	min = 1; max = IPQOS_NUM_PRIOQ;
	retVal = getTypedInputDefault(INPUT_TYPE_UINT, tmpstr, &sel, &min, &max);
	if (retVal == 0) {
		printf("Invalid value!\n");
		printWaitStr();
		return;
	} else if (retVal == -2)
		entry.prior = 3;	//default
	else
		entry.prior = (unsigned char)(sel - 1);

	min = 1; max = 8;
	retVal = getTypedInputDefault(INPUT_TYPE_UINT, "802.1p: (1)0 (2)1 (3)2 (4)3 (5)4 (6)5 (7)6 (8)7 [None]: ", &sel, &min, &max);
	if (retVal == 0) {
		printf("Invalid value!\n");
		printWaitStr();
		return;
	} else if (retVal == -2)
		entry.m_1p = 0;
	else
		entry.m_1p = (unsigned char)sel;


	min = 1; max = 8;
	retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Precedence: (1)0 (2)1 (3)2 (4)3 (5)4 (6)5 (7)6 (8)7 [None]: ", &sel, &min, &max);
	if (retVal == 0) {
		printf("Invalid value!\n");
		printWaitStr();
		return;
	} else if (retVal == -2)
		entry.m_ipprio = 0;
	else
		entry.m_ipprio = (unsigned char)sel;

	min = 1; max = 5;
	retVal = getTypedInputDefault(INPUT_TYPE_UINT, "TOS: (1)Normal Service (2)Minimize Cost (3)Maximize Reliability (4)Maximize Throughput (5)Minimize Delay [None]: ", &sel, &min, &max);
	if (retVal == 0) {
		printf("Invalid value!\n");
		printWaitStr();
		return;
	} else if (retVal == -2)
		entry.m_iptos = 0xff;
	else
		entry.m_iptos = (unsigned char)(1 << (sel - 1));
	/*
	// priority
	if (0 == getInputString("Priority(0~2) :(2)", strVal, sizeof(strVal)-1))
		return;
	
	if ( strVal[0] ) {
		if ( !string_to_dec(strVal, &intVal) || intVal<0 || intVal>2) {
			printf("Error! Invalid priority (0~2).");
			goto setErr_qos;
		}
		entry.prior = (unsigned char)intVal;
	}
	else
		entry.prior = 2; // default to Low

	printf("Outbound Iterface\n");
	min=1;max=showItf(2);
	if(getInputUint("Select index :",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}
	entry.outif = (unsigned char)RIPsel[sel-1];
	*/

	intVal = mib_chain_add(MIB_IP_QOS_TBL, (unsigned char*)&entry);
	if (intVal == 0) {
		printf(strAddChainerror);
		goto setErr_qos;
	}
	else if (intVal == -1) {
		printf(strTableFull);
		goto setErr_qos;
	}
#if defined(APPLY_CHANGE)
	/*ql 20081117 START modify qos take effect function*/
#ifdef NEW_IP_QOS_SUPPORT
	take_qos_effect();
#endif
#endif

	return;
setErr_qos:
	printf("Set QoS rule error!\n");
	printWaitStr();

}

#ifdef NEW_IP_QOS_SUPPORT
int QosTcList()
{
	MIB_CE_IP_TC_T Entry;
	unsigned int entryNum, i;
	char ifname[6];
	char *psip, *pdip, sip[20], dip[20];
	const char *type;
	char sport[10], dport[10];
	char rate[10], index[10];

	printf("%-5s%-6s%-19s%-8s%-19s%-8s%-8s%-8s\n", "Idx","Inf","Src IP","SrcPort","Dst IP","DstPort","Proto","Rate");
  	//printf("Idx  Wan Inf  Src IP               Src Port  Des IP               Des Port  Protocol Rate\n");

	entryNum = mib_chain_total(MIB_IP_QOS_TC_TBL);
	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_IP_QOS_TC_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}
		
		//index
		snprintf(index, 10, "%d", Entry.entryid);
		
		//wan interface
		if (PPP_INDEX(Entry.ifIndex) != 0x0f)//PPP interface
			snprintf(ifname, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));
		else//vc interface
			snprintf(ifname, 6, "vc%u", VC_INDEX(Entry.ifIndex));

		//source ip
		psip = inet_ntoa(*(struct in_addr *)&Entry.srcip);
		if ( !strcmp(psip, "0.0.0.0"))
			psip = (char *)BLANK;
		else {
			if (Entry.smaskbits==0)
				snprintf(sip, 20, "%s", psip);
			else
				snprintf(sip, 20, "%s/%d", psip, Entry.smaskbits);
			psip = sip;
		}
		
		// source port
		if (Entry.sport == 0)
			strcpy(sport, BLANK);
		else
			snprintf(sport, 10, "%d", Entry.sport);
		
		// destination ip
		pdip = inet_ntoa(*(struct in_addr *)&Entry.dstip);
		if ( !strcmp(pdip, "0.0.0.0"))
			pdip = (char *)BLANK;
		else {
			if (Entry.dmaskbits==0)
				snprintf(dip, 20, "%s", pdip);
			else
				snprintf(dip, 20, "%s/%d", pdip, Entry.dmaskbits);
			pdip = dip;
		}
		
		// destination port
		if (Entry.dport == 0)
			strcpy(dport, BLANK);
		else
			snprintf(dport, 10, "%d", Entry.dport);

		 if ( Entry.protoType == 1 )
			type = ARG_ICMP;
		if ( Entry.protoType == 2 )
			type = ARG_TCP;
		else if ( Entry.protoType == 3 )
			type = ARG_UDP;
		else if ( Entry.protoType == 4 )
			type = ARG_TCPUDP;
		else
			type = (char *)BLANK;

		snprintf(rate, 10, "%d", Entry.limitSpeed);
		
		printf("%-5s%-6s%-19s%-8s%-19s%-8s%-8s%-8s\n", index, ifname, psip, sport, pdip, dport, type, rate);
	}

	return entryNum;
}

unsigned char tcSelIfindex[8];
int QosTcInflist()
{
	MIB_CE_ATM_VC_T Entry;
	unsigned int entryNum, i;
	int ifnum=0;

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	printf("Idx  Interface\n");
	printf("----------------\n");
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}
		
		if (Entry.enable == 0)
			continue;
		
		if (Entry.cmode == ADSL_BR1483)
			continue;
		
		if (PPP_INDEX(Entry.ifIndex) != 0x0f)
		{	// PPP interface
			tcSelIfindex[ifnum++]=Entry.ifIndex;
			printf("%-5dppp%u\n",
				ifnum, PPP_INDEX(Entry.ifIndex));
		}
		else
		{	// vc interface
			tcSelIfindex[ifnum++]=Entry.ifIndex;
			printf("%-5dvc%u\n",
				ifnum, VC_INDEX(Entry.ifIndex));
		}
	}

	return ifnum;
}

void QosTcadd()
{
	char strSAddr[18], strDAddr[18], strSport[10], strDport[10];
	int intVal, retVal;
	int min,max,sel;
	unsigned int  totalEntry, i;
	MIB_CE_IP_TC_T entry;
	unsigned long  mask, mbit;
	unsigned char vChar;
	char strVal[5];
	char tmpstr[256] = {0};
	const char *pPrio;
	struct itfInfo itfs[16];
	int type, ifnum = 0, itfid, itfdomain;
	struct in_addr ipaddr;
	unsigned char map[MAX_QOS_RULE+1]={0};
	int entryid = 1;
	
	memset(&entry, 0, sizeof(entry));
	totalEntry = mib_chain_total(MIB_IP_QOS_TC_TBL); /* get chain record size */
	
	/* Add new qos entry */
	if (totalEntry >= MAX_QOS_RULE)
	{
		printf("Error: Maximum number of Qos rule exceeds !");
		goto setErr_qos;
	}
	
	//allocate a free rule id for new entry
	for(i=0; i<totalEntry;i++)
	{
		if(!mib_chain_get(MIB_IP_QOS_TC_TBL, i, &entry))
			continue;
		map[entry.entryid] = 1;
	}
	for(i=1;i<=MAX_QOS_RULE;i++)
	{
		if(!map[i])
		{
			entryid = i;
			break;
		}
	}
	entry.entryid = entryid;

	//interface
	max=QosTcInflist();
	min=1;
	if(getInputUint("Select interface:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}
	entry.ifIndex = tcSelIfindex[sel-1];

	// protocol type
	min=0;max=4;
	retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Protocol: (0)NONE (1)ICMP (2)TCP (3)UDP (4)TCP/UDP: ", &sel, &min, &max);
	if (retVal == 0) {
		printf("Invalid value!\n");
		printWaitStr();
		return;
	} else if (retVal == -2)
		entry.protoType = 0;
	else
		entry.protoType = (unsigned char)sel;

	//source IP
	strSAddr[0] = 0;
	retVal = getTypedInputDefault(INPUT_TYPE_IPADDR, "Source IP[None]: ", &ipaddr, 0, 0);
	if (retVal == 0) {
		printf("Invalid value!\n");
		printWaitStr();
		return;
	}else if (retVal==1) {
		strcpy(strSAddr, inet_ntoa(ipaddr));
	}

	// Source address
	if (strSAddr[0]) {
		if (!isValidIpAddr(strSAddr)) {
			printf("Invalid IP address %s.\n", strSAddr);
			goto setErr_qos;
		}
		inet_aton(strSAddr, (struct in_addr *)&entry.srcip);
		if (entry.srcip == 0) {
			printf("Error! Source IP.");
			goto setErr_qos;
		}

		retVal = getTypedInputDefault(INPUT_TYPE_IPMASK, "Source Netmask[None]: ", &ipaddr, 0, 0);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			strcpy(strSAddr, inet_ntoa(ipaddr));
			if (!isValidNetmask(strSAddr, 1)) {
				printf("Invalid subnet mask %s.\n", strSAddr);
				goto setErr_qos;
			}
			inet_aton(strSAddr, (struct in_addr *)&mask);
			if (mask==0) {
				printf("Error! Source Netmask.");
				goto setErr_qos;
			}
			mask = htonl(mask);
			mbit=0; intVal=0;
			for (i=0; i<32; i++) {
				if (mask&0x80000000) {
					if (intVal) {
						printf("Error! Source Netmask.");
						goto setErr_qos;
					}
					mbit++;
				}
				else
					intVal=1;
				mask <<= 1;
			}
			entry.smaskbits = mbit;
		} else if (retVal == -2)
			entry.smaskbits = 32;
	} else {
		entry.srcip = 0;
		entry.smaskbits = 0;
	}
	
	// Destination address
	strDAddr[0] = 0;
	retVal = getTypedInputDefault(INPUT_TYPE_IPADDR, "Destination IP[None]: ", &ipaddr, 0, 0);
	if (retVal == 0) {
		printf("Invalid value!\n");
		printWaitStr();
		return;
	} else if (retVal == 1)
		strcpy(strDAddr, inet_ntoa(ipaddr));

	if (strDAddr[0]) {
		if (!isValidIpAddr(strDAddr)) {
			printf("Invalid IP address %s.\n", strDAddr);
			goto setErr_qos;
		}
		inet_aton(strDAddr, (struct in_addr *)&entry.dstip);
		if (entry.dstip==0) {
			printf("Error! Destination IP.");
			goto setErr_qos;
		}
        	
		retVal = getTypedInputDefault(INPUT_TYPE_IPMASK, "Destination Netmask[None]: ", &ipaddr, 0, 0);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			strcpy(strDAddr, inet_ntoa(ipaddr));
			if (!isValidNetmask(strDAddr, 1)) {
				printf("Invalid subnet mask %s.\n", strDAddr);
				goto setErr_qos;
			}
			inet_aton(strDAddr, (struct in_addr *)&mask);
			if (mask==0) {
				printf("Error! Source Netmask.");
				goto setErr_qos;
			}
			mask = htonl(mask);
			mbit=0; intVal=0;
			for (i=0; i<32; i++) {
				if (mask&0x80000000) {
					if (intVal) {
						printf("Error! Source Netmask.");
						goto setErr_qos;
					}
					mbit++;
				}
				else
					intVal=1;
				mask <<= 1;
			}
			entry.dmaskbits = mbit;
		} else if (retVal == -2)
			entry.dmaskbits = 32;
	} else {
		entry.dstip = 0;
		entry.dmaskbits = 0;
	}

	// source port
	if (entry.protoType >= 2) {
		min = 1; max = 65535;
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Source Port(1~65535) [None]: ", &sel, &min, &max);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == -2)
			entry.sport =0;
		else
			entry.sport = (unsigned short)sel;
	} else {
		entry.sport = 0;
	}
	
	// destination port
	if (entry.protoType >= 2) {
		min = 1; max = 65535;
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Destination Port(1~65535) [None]: ", &sel, &min, &max);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == -2)
			entry.dport =0;
		else
			entry.dport = (unsigned short)sel;
	} else {
		entry.dport = 0;
	}
	
	//uplink rate
	min=0;max=0xffffffff;
	if (0 == getInputUint("Uplink Rate(0): ", &entry.limitSpeed, &min, &max)) {
		printf("Invalid value!\n");
		printWaitStr();
		return;
	}
				
	intVal = mib_chain_add(MIB_IP_QOS_TC_TBL, (unsigned char*)&entry);
	if (intVal == 0) {
		printf(strAddChainerror);
		goto setErr_qos;
	}
	else if (intVal == -1) {
		printf(strTableFull);
		goto setErr_qos;
	}
#if defined(APPLY_CHANGE)
	/*ql 20081117 START modify qos take effect function*/
#ifdef NEW_IP_QOS_SUPPORT
	take_qos_effect();
#endif
#endif

	return;
setErr_qos:
	printf("Set QoS rule error!\n");
	printWaitStr();

}
#endif

//tylo
void Qosdel(){
	int min,max,sel,sel2;

	min=1;max=2;
	if(getInputUint("(1) Delete one  (2) Delete all: ",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}

	if(sel==1){//delete one
		min=1;max=QosList();
		if(getInputUint("Select index :",&sel2,&min,&max)==0){
			printf("Invalid selection!\n");
			return;
		}
		if(mib_chain_delete(MIB_IP_QOS_TBL, sel2-1) != 1) {
			printf("Delete chain record error!");
		}

	}
	else{//delete all
		mib_chain_clear(MIB_IP_QOS_TBL); /* clear chain record */
	}
#if defined(APPLY_CHANGE)
	/*ql 20081117 START modify qos take effect function*/
#ifdef NEW_IP_QOS_SUPPORT
	take_qos_effect();
#endif
#endif
}

#ifdef NEW_IP_QOS_SUPPORT
void QosTcdel()
{
	int min,max,sel,sel2;

	min=1;max=2;
	if(getInputUint("(1) Delete one  (2) Delete all: ",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}

	if(sel==1){//delete one
		min=1;max=QosTcList();
		if(getInputUint("Select index :",&sel2,&min,&max)==0){
			printf("Invalid selection!\n");
			return;
		}
		if(mib_chain_delete(MIB_IP_QOS_TC_TBL, sel2-1) != 1) {
			printf("Delete chain record error!");
		}

	}
	else{//delete all
		mib_chain_clear(MIB_IP_QOS_TC_TBL); /* clear chain record */
	}
#if defined(APPLY_CHANGE)
	/*ql 20081117 START modify qos take effect function*/
#ifdef NEW_IP_QOS_SUPPORT
	take_qos_effect();
#endif
#endif
}

//ql
void setQosPolicy()
{
	unsigned char policy;
	int min=1, max=2, sel;
	
	mib_get(MIB_QOS_POLICY, (void *)&policy);
	printf("IP QoS Policy:\t %s\n", (policy==1)?"WRR":"PRIO");

	printf("new IP QoS Policy:\t (1)PRIO (2)WRR\n\n");
	if (getInputUint("Select index:",&sel,&min,&max)) {
		if (sel == 1)
			policy = 0;//PRIO
		else
			policy = 1;//WRR
		mib_set(MIB_QOS_POLICY, (void *)&policy);
		
#if defined(APPLY_CHANGE)
		/*ql 20081117 START modify qos take effect function*/
#ifdef NEW_IP_QOS_SUPPORT
		take_qos_effect();
#endif
#endif
	}

	return;
}

void setQosTotalbandwidth()
{
	unsigned short bandwidth;
	unsigned char enable;
	int min=0, max=1, sel;
	
	mib_get(MIB_TOTAL_BANDWIDTH_LIMIT_EN, (void *)&enable);
	printf("\ntotalbandwidth limit:\t %s\n", enable?"Enable":"Disable");

	mib_get(MIB_TOTAL_BANDWIDTH, (void *)&bandwidth);
	printf("totalbandwidth:\t %dKbit\n\n", bandwidth);

	max=0xffffffff;
	if (getInputUint("set totalbandwidth(Kbit):", &sel, &min, &max)) {
		if (sel==0) {
			bandwidth = 0;
			enable = 0;
		} else {
			bandwidth = sel;
			enable = 1;
		}
		mib_set(MIB_TOTAL_BANDWIDTH, (void *)&bandwidth);
		mib_set(MIB_TOTAL_BANDWIDTH_LIMIT_EN, (void *)&enable);

#if defined(APPLY_CHANGE)
		/*ql 20081117 START modify qos take effect function*/
#ifdef NEW_IP_QOS_SUPPORT
		take_qos_effect();
#endif
#endif
	} else {
		printf("Invalid value!\n");
		printWaitStr();
	}
	
	return;
}
#endif

void enable_IPQOS()
{
	int snum;
	unsigned char vChar;
	unsigned char mode=0;
	
	while(1) 
	{			
		//ql modify
#ifdef IP_QOS
		mib_get(MIB_MPMODE, (void *)&vChar);
		if (vChar&0x02) stopIPQ();
		
		//vChar=MP_IPQOS;
		vChar |= MP_IPQOS;
		if ( mib_set(MIB_MPMODE, (void *)&vChar) == 0) {
			printf("Set multiport mode MIB error!");
		}
#endif		
		CLEAR;
		printf("\n");
			
		printf("-------------------------------------------------------------------------\n");
		printf("                         Config IP QoS Rule                              \n");
		printf("-------------------------------------------------------------------------\n");
#ifdef NEW_IP_QOS_SUPPORT
      		printf("Entries in this table are used to assign the precedence for each incoming\n ");
      		printf("packet based on physical LAN port, TCP/UDP port number, and source/\n");
		printf("destination IP address/subnet masks.\n");
		printf("You can add/delete/show the rules with the following options.\n");
		printf("-------------------------------------------------------------------------\n");
		printf("(1) Enable IP QoS			(2) Policy\n");
      		printf("(3) IP QoS rule add			(4) IP QoS rule del\n");
		printf("(5) Show IP QoS table			(6) Quit\n");
#else
		printf("The IP QoS is enabled. You can add/delete/show the rules with the        \n");
		printf("followings  options.\n");			
		printf("(1) IP QoS rule add			(2) IP QoS rule del\n");
      		printf("(3) Show IP QoS table			(4) Quit\n");
#endif
#ifdef NEW_IP_QOS_SUPPORT
      		if (!getInputOption( &snum, 1, 6)) {      				      				
      			continue;
		}
#else
      		if (!getInputOption( &snum, 1, 4)) {      				      				
#if defined(APPLY_CHANGE)				
			setupIPQ();				
#endif			
      			continue;
      							
		}
#endif
#ifdef IP_QOS
      		switch(snum){
			case 1:
				if (!check_access(SECURITY_SUPERUSER))
					break;
				Qosadd();
				break;
			case 2:
				if (!check_access(SECURITY_SUPERUSER))
					break;
				Qosdel();
				break;
			case 3:
				QosList();
				printWaitStr();
				break;
			case 4:
#if defined(APPLY_CHANGE)				
				setupIPQ();				
#endif
				return;					
      		}
#else
#ifdef NEW_IP_QOS_SUPPORT
      		switch(snum){
			case 1:
				mib_get(MIB_MPMODE, (void *)&vChar);

				vChar |= MP_IPQOS;
				if ( mib_set(MIB_MPMODE, (void *)&vChar) == 0) {
					printf("Set multiport mode MIB error!");
				}
#if defined(APPLY_CHANGE)				
				/*ql 20081117 START modify qos take effect function*/
				take_qos_effect();
#endif
				break;
			case 2:
				setQosPolicy();
				break;
			case 3:
				if (!check_access(SECURITY_SUPERUSER))
					break;
				Qosadd();
				break;
			case 4:
				if (!check_access(SECURITY_SUPERUSER))
					break;
				Qosdel();
				break;
			case 5:
				QosList();
				printWaitStr();
				break;
			case 6:
				return;
      		}
#endif
#endif
#if defined(APPLY_CHANGE)
		/*ql 20081117 START modify qos take effect function*/
#ifdef IP_QOS
		setupIPQ();
#endif
#endif
	} // while(1)
}
 

void disable_IPQOS()
{
	int snum;
	unsigned char vChar;	
	
#ifdef IP_QOS
	while(1) {
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                         Disable IP QoS                                  \n");
		printf("-------------------------------------------------------------------------\n");
		printf("(1) disable IP QoS			(2) Quit\n");      	
        		
      		if (!getInputOption( &snum, 1, 2)) {
      			continue;
		}	
		
		// Disable IP QoS
		if ( snum == 1 ) {		
			mib_get(MIB_MPMODE, (void *)&vChar);
			if (vChar&0x02) stopIPQ();
			
			vChar &= 0xfd;
			mib_set(MIB_MPMODE, (void *)&vChar);
		} else {
			return;
		}
	}	
#endif
#ifdef NEW_IP_QOS_SUPPORT
	while(1) 
	{			
		CLEAR;
		printf("\n");
		
		printf("-------------------------------------------------------------------------\n");
		printf("                         IP QoS Traffic Control                             \n");
		printf("-------------------------------------------------------------------------\n");
      		printf("Entries in this table are used to shape traffic based on Wan interface, \n ");
      		printf("TCP/UDP port number, and source/destination IP address/subnet masks. \n");
		printf("You can add/delete/show traffic shaping rules with the following options.\n");
		printf("-------------------------------------------------------------------------\n");
		printf("(1) Disable IP QoS			(2) total bandwidth limit\n");
		printf("(3) traffic shaping rule add		(4) traffic shaping rule del\n");
		printf("(5) Show rule				(6) Quit\n");
        	
      		if (!getInputOption( &snum, 1, 6)) {
      			continue;
		}
		
      		switch(snum){
			case 1:
				mib_get(MIB_MPMODE, (void *)&vChar);
				vChar &= 0xfd;
				if ( mib_set(MIB_MPMODE, (void *)&vChar) == 0) {
					printf("Set multiport mode MIB error!");
				}
		
#if defined(APPLY_CHANGE)
				take_qos_effect();
#endif
				break;
			case 2:
				setQosTotalbandwidth();
				break;
			case 3:
				if (!check_access(SECURITY_SUPERUSER))
					break;
				QosTcadd();
				break;
			case 4:
				if (!check_access(SECURITY_SUPERUSER))
					break;
				QosTcdel();
				break;
			case 5:
				QosTcList();
				printWaitStr();
				break;
			case 6:
				return;					
      		}
	}

#endif
}


//tylo
void setQoS(){
	int snum, min,max,sel;
	unsigned char vChar, qos;
	unsigned char mode=0;
	
	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                     	IP QoS                      \n");
		printf("-------------------------------------------------------------------------\n");
#ifdef NEW_IP_QOS_SUPPORT
		printf("If IP QoS enable, you can assign the precedence for each \n");
		printf("incoming packets. And If IP QoS disable, you can shaping \n");
		printf("upstream as you want. \n");
printf("-------------------------------------------------------------------------\n");
#else
      		printf("Entries in this table are used to assign the precedence for each \n ");
      		printf("incoming packet based on packet based on physical LAN port, \nTCP/UDP port number, and source/destination IP address/subnet masks \n");
#endif
		getSYS2Str(SYS_IP_QOS, strbuf);
		printf("IP QoS\t: %s\n\n", strbuf);		
		
		min=1;max=3;      
		if (0 == getInputUint("IP QoS (1)Disable (2)Enable (3) Quit:", &sel, &min, &max)) {
			//return;			
			continue;
		}
		
		if( sel == 3)
			return;	
		
		qos = (unsigned char)(sel - 1);

		if (qos) {
			enable_IPQOS();
			
		}
		else
		{
			disable_IPQOS();
		}
	}
}
#endif	// of IP_QOS

//tylo
#ifdef ZTE_GENERAL_ROUTER_SC
int showRemoteAccess(int *pvcSelectIndex)
#else 
void showRemoteAccess()
#endif
{
	MIB_CE_ACC_T Entry;
	int sel=0;
#ifdef ZTE_GENERAL_ROUTER_SC
	MIB_CE_ATM_VC_T pvcEntry;
	int entryNum;
	int mibcnt;		
	char interfacename[MAX_NAME_LEN];
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	unsigned char forSelect=0;
	unsigned char pvcRouter=0;
	int min,max;
	unsigned char pvcRouterFlag[8];//MAX PVC numbers
	 for(mibcnt=0;mibcnt<entryNum;mibcnt++)
	          {
		  if (!mib_chain_get(MIB_ATM_VC_TBL, mibcnt, (void *)&pvcEntry))
						{
		  					printf("get MIB_ATM_VC_TBL error!\n");
							return -1;
						}
		if(pvcEntry.cmode!=ADSL_BR1483)// ADSL_BR1483 ADSL_MER1483 ADSL_PPPoE ADSL_PPPoA	ADSL_RT1483	ADSL_RT1577
			      	{
			      	
			      	 #ifdef CTC_WAN_NAME
					if(pvcEntry.enable == 0)
						continue;
					getWanName(&pvcEntry, interfacename);
//					sprintf(interfacename,"Internet_R_%u_%u",Entry.vpi,Entry.vci);
				#else
				  	if((pvcEntry.cmode == ADSL_PPPoE) || (pvcEntry.cmode == ADSL_PPPoA) ) {
							 sprintf(interfacename, "PPP%u", PPP_INDEX(pvcEntry.ifIndex));
				  	}else
				  	{
				  	                     sprintf(interfacename, "vc%u", PPP_INDEX(pvcEntry.ifIndex));
				  	}
		             #endif
				printf("[%d]:     %s\n",pvcRouter,interfacename);
				pvcRouter++;	
				pvcRouterFlag[mibcnt]=pvcRouter;
							
			}
		   else pvcRouterFlag[mibcnt]=0;
	 	}
	 if(pvcRouter)   	{
	 	char tmpstr[256]={0};
		snprintf(tmpstr,sizeof(tmpstr),"Select one router pvc as following[0-%d]:",pvcRouter-1);
		 min=0;
		 max=pvcRouter-1;		
	 	if(getInputUint(tmpstr,&sel,&min,&max)==0){
			printf("Invalid selection!\n");
			return;
		}	
		for(mibcnt=0;mibcnt<entryNum;mibcnt++)
			{
			  if(pvcRouterFlag[mibcnt]==sel+1) sel=mibcnt;
			  break;
			}
	 	}
	 else {
	 	printf("Please configure one router mode pvc.\n");
		return;
	 	}
	 *pvcSelectIndex=sel;
	 
#endif
	if (!mib_chain_get(MIB_ACC_TBL,sel, (void *)&Entry)) {
		printf("Get MIB fail!\n");
		return;
	}

	
	//TELNET
#ifdef CONFIG_USER_TELNETD_TELNETD
#ifndef ZTE_GENERAL_ROUTER_SC//default router
	if (Entry.telnet & 0x02)
		printf("Telnet  LAN:  Enabled\n");
	else 
		printf("Telnet  LAN:  Disabled\n");
#endif
	if (Entry.telnet & 0x01)
#ifdef ZTE_GENERAL_ROUTER_SC
	printf("Telnet  WAN:  Enabled		port:%d\n",Entry.telnet_port);
#else
	if (Entry.telnet_port == 23)
		printf("Telnet  WAN:  Enabled\n");
	else
		printf("Telnet  WAN:  Enabled on port %d\n", Entry.telnet_port);
#endif
		
	else 
		printf("Telnet  WAN:  Disabled\n");
#endif
#ifdef CONFIG_USER_FTP_FTP_FTP
#ifndef ZTE_GENERAL_ROUTER_SC//default router
	//FTP
	if (Entry.ftp & 0x02)
		printf("FTP     LAN:  Enabled\n");
	else 
		printf("FTP     LAN:  Disabled\n");
#endif
	if (Entry.ftp & 0x01)
#ifdef ZTE_GENERAL_ROUTER_SC
		printf("FTP     WAN:  	port:%d\n",Entry.ftp_port);
#else
		if (Entry.ftp_port == 21)
			printf("FTP     WAN:  Enabled\n");
		else
			printf("FTP     WAN:  Enabled on port %d\n", Entry.ftp_port);
#endif
	else 
		printf("FTP     WAN:  Disabled\n");
#endif
#ifdef CONFIG_USER_TFTPD_TFTPD
#ifndef ZTE_GENERAL_ROUTER_SC//default router
	//TFTP	
	if (Entry.tftp & 0x02)
		printf("TFTP    LAN:  Enabled\n");
	else 
		printf("TFTP    LAN:  Disabled\n");
#endif
	if (Entry.tftp & 0x01)
		printf("TFTP    WAN:  Enabled\n");
	else 
		printf("TFTP    WAN:  Disabled\n");
#endif
#ifndef ZTE_GENERAL_ROUTER_SC//default router
	//HTTP
	if (Entry.web & 0x02)
		printf("WEB     LAN:  Enabled\n");
	else 
		printf("WEB     LAN:  Disabled\n");
#endif

	if (Entry.web & 0x01)
#ifdef ZTE_GENERAL_ROUTER_SC
		printf("WEB     WAN:  	port:%d\n",Entry.web_port);
#else
		if (Entry.web_port == 80)
			printf("WEB     WAN:  Enabled\n");
		else
			printf("WEB     WAN:  Enabled on port %d\n", Entry.web_port);
#endif
	else 
		printf("WEB     WAN:  Disabled\n");
//HTTPS
#ifdef CONFIG_USER_BOA_WITH_SSL
#ifndef ZTE_GENERAL_ROUTER_SC//default router
	if (Entry.https & 0x02)
		printf("HTTPS   LAN:  Enabled\n");
	else 
		printf("HTTPS   LAN:  Disabled\n");
#endif
	if (Entry.https & 0x01)
#ifdef ZTE_GENERAL_ROUTER_SC
	printf("HTTPS  WAN:  Enabled		port:%d\n",Entry.https_port);
#else
	if (Entry.https_port == 443)
		printf("HTTPS   WAN:  Enabled\n");
	else
		printf("HTTPS   WAN:  Enabled on port %d\n", Entry.https_port);
#endif
		
	else 
		printf("HTTPS   WAN:  Disabled\n");
#endif //end of CONFIG_USER_BOA_WITH_SSL

	
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
#ifndef ZTE_GENERAL_ROUTER_SC//default router
	//SNMP
	if (Entry.snmp & 0x02)
		printf("SNMP    LAN:  Enabled\n");
	else 
		printf("SNMP    LAN:  Disabled\n");
#endif
	if (Entry.snmp & 0x01)
		printf("SNMP    WAN:  Enabled\n");
	else 
		printf("SNMP    WAN:  Disabled\n");
#endif
#ifdef CONFIG_USER_SSH_DROPBEAR
#ifndef ZTE_GENERAL_ROUTER_SC//default router
	//SSH
	if (Entry.ssh & 0x02)
		printf("SSH     LAN:  Enabled\n");
	else 
		printf("SSH     LAN:  Disabled\n");
#endif
	if (Entry.ssh & 0x01)
		printf("SSH     WAN:  Enabled\n");
	else 
		printf("SSH     WAN:  Disabled\n");
#endif
#ifndef ZTE_GENERAL_ROUTER_SC//default router
	//ICMP
	printf("ICMP    LAN:  Enabled\n");
#endif
	if (Entry.icmp & 0x01)
		printf("ICMP    WAN:  Enabled\n");
	else 
		printf("ICMP    WAN:  Disabled\n");

}

//tylo
void changeRemoteAccess(int pvcIndex)
{
	int min,max,sel;
	int portmin,portmax;
	MIB_CE_ACC_T Entry;
	MIB_CE_ACC_T entry;

	if (!mib_chain_get(MIB_ACC_TBL, pvcIndex, (void *)&Entry)) {
		memset(&entry, '\0', sizeof(MIB_CE_ACC_T));
		mib_chain_add(MIB_ACC_TBL, (unsigned char*)&entry);
		if (!mib_chain_get(MIB_ACC_TBL, pvcIndex, (void *)&Entry))
			return;
	}
	
	filter_set_remote_access(0);

	min=1;max=2;
	portmin=1; portmax=65535;
#ifdef CONFIG_USER_TELNETD_TELNETD
#ifndef ZTE_GENERAL_ROUTER_SC//default router
	//TELNET
	if(getInputUint("Telnet  LAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}
	Entry.telnet = 0;//default Disable
	if(sel==1)
		Entry.telnet |= 0x02;	
#endif
	if(getInputUint("Telnet  WAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}
	if(sel==1)
		{
		Entry.telnet |= 0x01;
		int inputRet=getTypedInputDefault(INPUT_TYPE_UINT,"Telnet  WAN open port[23]:",&sel,&portmin,&portmax);
		if(inputRet==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
		}else if(inputRet==-2)
		Entry.telnet_port=23;
		else
		Entry.telnet_port=sel;
		}
#endif
#ifndef ZTE_GENERAL_ROUTER_SC//default router
	//FTP
	if(getInputUint("FTP  LAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}
	Entry.ftp = 0;//default Disable
	if(sel==1)
		Entry.ftp |= 0x02;
#endif
	
	if(getInputUint("FTP  WAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}

	if(sel==1)
		{
		Entry.ftp |= 0x01;
		
		int inputRet=getTypedInputDefault(INPUT_TYPE_UINT,"FTP  WAN open port[21]:",&sel,&portmin,&portmax);
		if(inputRet==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
		}else if(inputRet==-2)
		Entry.ftp_port=21;
		else
		Entry.ftp_port=sel;
		}
#ifdef CONFIG_USER_TFTPD_TFTPD
#ifndef ZTE_GENERAL_ROUTER_SC//default router
	//TFTP
	if(getInputUint("TFTP  LAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}
	Entry.tftp = 0;//default Disable
	if(sel==1)
		Entry.tftp |= 0x02;
#endif
	
	if(getInputUint("TFTP  WAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}
	if(sel==1)
		Entry.tftp |= 0x01;
#endif
#ifndef ZTE_GENERAL_ROUTER_SC//default router
	//WEB
	if(getInputUint("WEB  LAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}
	Entry.web  = 0;//default Disable
	if(sel==1)
		Entry.web |= 0x02;
#endif
	
	if(getInputUint("WEB  WAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}
	if(sel==1)
		{
		Entry.web |= 0x01;
		
		int inputRet=getTypedInputDefault(INPUT_TYPE_UINT,"WEB  WAN open port[80]:",&sel,&portmin,&portmax);
		if(inputRet==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
		}else if(inputRet==-2)
		Entry.web_port=80;
		else
		Entry.web_port=sel;
		}
#ifdef CONFIG_USER_BOA_WITH_SSL
#ifndef ZTE_GENERAL_ROUTER_SC//default router
	//HTTPS
	if(getInputUint("HTTPS  LAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}
	Entry.https  = 0;//default Disable
	if(sel==1)
		Entry.https |= 0x02;	
#endif
	if(getInputUint("HTTPS  WAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}
	if(sel==1) {
		Entry.https |= 0x01;
		int inputRet=getTypedInputDefault(INPUT_TYPE_UINT,"HTTPS  WAN open port[443]:",&sel,&portmin,&portmax);
		if(inputRet==0){
			printf("Invalid selection!\n");
			goto err_rmacc;
		}else if(inputRet==-2)
			Entry.https_port=443;
		else
			Entry.https_port=sel;
	}
#endif //end of CONFIG_USER_BOA_WITH_SSL	
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
#ifndef ZTE_GENERAL_ROUTER_SC//default router
	//SNMP
	if(getInputUint("SNMP  LAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}
	Entry.snmp  = 0;//default Disable
	if(sel==1)
		Entry.snmp |= 0x02;
#endif
	
	if(getInputUint("SNMP  WAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}
	if(sel==1)
		Entry.snmp |= 0x01;
#endif
#ifdef CONFIG_USER_SSH_DROPBEAR
#ifndef ZTE_GENERAL_ROUTER_SC//default router
	//SSH
	if(getInputUint("SSH  LAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}
	Entry.ssh  = 0;//default Disable
	if(sel==1)
		Entry.ssh |= 0x02;
#endif
	
	if(getInputUint("SSH  WAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}
	if(sel==1)
		Entry.ssh |= 0x01;
#endif
#ifndef ZTE_GENERAL_ROUTER_SC//default router
	//ICMP
	printf("ICMP  LAN: Enable\n");
#endif
	if(getInputUint("ICMP  WAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}
	Entry.icmp  = 0x02; // always LAN enable
	if(sel==1)
		Entry.icmp |= 0x01;
#ifdef ZTE_GENERAL_ROUTER_SC
    MIB_CE_ATM_VC_T pvcEntry;
    mib_chain_get(MIB_ATM_VC_TBL,pvcIndex,(void*)&pvcEntry);
    Entry.ifIndex=pvcEntry.ifIndex;
    Entry.vpi=pvcEntry.vpi;
    Entry.vci=pvcEntry.vci;
    Entry.protocol=pvcEntry.cmode;
#endif	
	mib_chain_update(MIB_ACC_TBL, (void *)&Entry, pvcIndex);
err_rmacc:
	filter_set_remote_access(1);
}

//tylo
void setRemoteAccess(){
	int min,max,sel;
	while (1)
   	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("                      Remote Access             \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
      		printf("This page is used to enable/disable management services for the LAN and WAN. \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
                 #ifdef ZTE_GENERAL_ROUTER_SC
		int pvcSelectIndex=-1;
		showRemoteAccess(&pvcSelectIndex);
		#else
		showRemoteAccess();
		#endif
		min=1;max=2;
		if(getInputUint("Select index (1) Set Remote Access (2) Quit:",&sel,&min,&max)==0){
			printf("Invalid selection!\n");
			return;
		}
		switch(sel){
			case 1:
				if (!check_access(SECURITY_SUPERUSER))
					break;
		#ifdef ZTE_GENERAL_ROUTER_SC
				changeRemoteAccess(pvcSelectIndex);
		#else
				changeRemoteAccess(0);
		#endif
				break;
			case 2:
				return;
		}
	}
}

//Jenny
void showOthers(){
	unsigned char vChar, laChar;

#ifdef IP_PASSTHROUGH
	mib_get( MIB_IPPT_ITF, (void *)&vChar);
	if (vChar == 0xff)
		printf("IP PassThrough: None\n");
	else {
		if (PPP_INDEX(vChar) != 0x0f)		// PPP interface
			printf("IP PassThrough: ppp%u\n", PPP_INDEX(vChar));
		else		// vc interface
			printf("IP PassThrough: vc%u\n", VC_INDEX(vChar));
	
		printf("Lease Time: %s Seconds\n", getMibInfo(MIB_IPPT_LEASE));

		mib_get( MIB_IPPT_LANACC, (void *)&laChar);
		if (laChar)
			printf("LAN Access: Enable\n");
		else
			printf("LAN Access: Disable\n");
	}
#endif
}

//tylo
unsigned char IPPTsel[8];
int IPPTlist(){
	int ifnum=0;
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	char_t *name;
	int type=3;	// point-to-point interface
	unsigned char IPPTselcnt=0;

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	printf("Idx  Interface\n");
	printf("----------------\n");
	printf("0    None\n");
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}
		
		if (Entry.enable == 0)
			continue;
		
		if (type != 2) { // rt or all (1 or 0)
			if ((type == 1 || type == 3) && Entry.cmode == ADSL_BR1483)
				continue;
			
			// check for p-2-p link
			if (type == 3 && Entry.cmode == ADSL_MER1483)
				continue;
			
			if (PPP_INDEX(Entry.ifIndex) != 0x0f)
			{	// PPP interface
				IPPTsel[IPPTselcnt]=Entry.ifIndex;
				IPPTselcnt++;
				printf("%-5dppp%u\n", IPPTselcnt, PPP_INDEX(Entry.ifIndex));
			}
			else
			{	// vc interface
				IPPTsel[IPPTselcnt]=Entry.ifIndex;
				IPPTselcnt++;
				printf("%-5dvc%u\n",	IPPTselcnt, VC_INDEX(Entry.ifIndex));
			}
			ifnum++;
		}
	}

	return ifnum;
}

//Jenny
//void setIPPT(struct ippt_para para)
struct ippt_para setIPPT()
{

	int min, max, sel, num;	

	unsigned char ippt_itf=255, old_ippt_itf, ippt_lanacc=0, old_ippt_lanacc;
	unsigned int ippt_lease=0, old_ippt_lease;
	struct ippt_para para;
	
	// Get old index of IPPT interface
	mib_get(MIB_IPPT_ITF, (void *)&old_ippt_itf);
	para.old_ippt_itf= old_ippt_itf;
	//printf("para.old_ippt_itf=%d\n", para.old_ippt_itf);
	
	// Get old IPPT Lease Time
	mib_get(MIB_IPPT_LEASE, (void *)&old_ippt_lease);
	para.old_ippt_lease= old_ippt_lease;
	//printf("para.old_ippt_lease=%d\n", para.old_ippt_lease);
	
	// Get old IPPT LAN access flag
	mib_get(MIB_IPPT_LANACC, (void *)&old_ippt_lanacc);
	para.old_ippt_lanacc= old_ippt_lanacc;
	//printf("para.old_ippt_lanacc=%d\n", para.old_ippt_lanacc);
		
	max=IPPTlist();
	min=0;
	if(getInputUint("Select interface: ",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}

	if (sel != 0) {		
		ippt_itf=IPPTsel[sel-1];				
		para.new_ippt_itf= ippt_itf;		
		
		min=0;max=0xffffffff;
		if (0 == getInputUint("Lease Time (seconds) : ", &ippt_lease, &min, &max))
			return;
		para.new_ippt_lease= ippt_lease;		
		
		min=1;max=2;      
		if (0 == getInputUint("LAN Access (1)Disable (2)Enable :", &num, &min, &max))
			return;
		ippt_lanacc = (unsigned char)(num - 1);
		para.new_ippt_lanacc= ippt_lanacc;
		
		if ( !mib_set(MIB_IPPT_LEASE, (void *)&ippt_lease)) {
			printf("Set IP passthrough lease time error!\n");
			return;
		}
		if ( !mib_set(MIB_IPPT_LANACC, (void *)&ippt_lanacc)) {
			printf("Set IP passthrough LAN access error!\n");
			return;
		}
	}
	else {
		ippt_itf=0xff;
		para.new_ippt_itf= ippt_itf;
		para.new_ippt_lease= ippt_lease;
		para.new_ippt_lanacc= ippt_lanacc;
	}
	
	if ( !mib_set(MIB_IPPT_ITF, (void *)&ippt_itf)) {
		printf("Set IP passthrough interface index error!\n");
		return;
	}
	return para;
}

//Jenny
void setOthers() {
	int snum;
	struct ippt_para para;
	while (1) {
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                      Other Advanced Configuration             \n");
		printf("-------------------------------------------------------------------------\n");
      		printf("Here you can set some other advanced settings.\n");
		printf("-------------------------------------------------------------------------\n");
		showOthers();
		printf("\n(1) Set IP PassThrough			(2) Quit\n");
      		if (!getInputOption( &snum, 1, 2))
      			continue;
      		
      		switch(snum) {
			case 1:
				if (!check_access(SECURITY_SUPERUSER))
					break;
				para = setIPPT();
#if defined(APPLY_CHANGE)				
				restartIPPT(para);
#endif				
				break;
			case 2:
				return;
		 	default:
		 		printf("!! Invalid Selection !!\n");
      		}
	}
}

void setAdvance()
{
	int snum;
	
	while (1)
	{
		CLEAR;
		CLEAR;
		printf("\n");
		printf("------------------------------------------------------------\n");
		printf("                 (6) Advance Menu                           \n");
		printf("------------------------------------------------------------\n");
#ifndef ZTE_531B_BRIDGE_SC
		printf("(1)  ARP Table                    (2)  Bridging\n");
		printf("(3)  Routing                      (4)  SNMP\n");
		printf("(5)  Port Mapping                 (6)  IP Qos\n");
		printf("(7)  Link Mode                    (8)  Remote Access\n");
		printf("(9)  Others                       (10) Quit\n");
		if (!getInputOption( &snum, 1, 10))
			continue;
		switch( snum)
		{
		case 1://(1) ARP table
			CLEAR;
			showARPTable();
			printWaitStr();
			break;
		case 2://(2) Bridging
		   setBridging();
		   break;
		case 3://(3) Routing
		   setRouting();
		   break;
		case 4://(4) SNMP
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
		   setSNMP();
#endif
		   break;
		case 5://(5) Port Mapping
#if defined(CONFIG_EXT_SWITCH) && defined(ITF_GROUP)
			CLEAR;
			eth2pvclist();
			if (!check_access(SECURITY_SUPERUSER))
				break;
			setPortMapping();
#else
			printf("Not supported\n");
			printWaitStr();
#endif
			break;
		case 6://(6) IP QoS
#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
			setQoS();
#else
			printf("Not supported\n");
			printWaitStr();
#endif
			break;
		case 7://(7) Link Mode
#if defined(ELAN_LINK_MODE) || defined(ELAN_LINK_MODE_INTRENAL_PHY)
			setLinkmode();
#else
			printf("Not supported\n");
			printWaitStr();
#endif
			break;
		case 8://(8) Remote access
			setRemoteAccess();
			break;
		case 9://(9) Others
#ifdef IP_PASSTHROUGH
			setOthers();
#else
			printf("Not supported\n");
			printWaitStr();
#endif
			break;
		case 10://(10) Quit
			return;
#else
		printf("(1) Bridging                     (2) Port Mapping\n");
		printf("(3) Link Mode                    (4) Quit\n");
		if (!getInputOption( &snum, 1, 4))
			continue;
		switch( snum)
		{
		case 1://(1) Bridging
		   setBridging();
		   break;
#ifdef CONFIG_EXT_SWITCH
#ifdef ITF_GROUP
		case 2://(2) Port Mapping
			CLEAR;
			eth2pvclist();
			setPortMapping();
			break;
#endif
#ifdef ELAN_LINK_MODE
		case 3://(3) Link Mode
			setLinkmode();
			break;
#endif
#endif
		case 4://(4) Quit
			return;
#endif
		default:
			printf("!! Invalid Selection !!\n");
		}//end switch, (6) Advance Menu
	}//end while, (6) Advance Menu
}

/*************************************************************************************************/
#define MAX_DSL_TONE 512
void startADSLDiagnostic()
{
	int dbgmode;
	char mode;
	unsigned char tone[64];
	int ldstate, diagflag = 1, i, chan = 256;
	short *hlog, *snr, *qln;
	int intp, fp;
	char str[16];
	Modem_LinkSpeed vLs;

	memset(tone, 0, sizeof(tone));
	hlog = malloc(sizeof(short)*(MAX_DSL_TONE*3+HLOG_ADDITIONAL_SIZE));
	snr = malloc(sizeof(short)*MAX_DSL_TONE);
	qln = malloc(sizeof(short)*MAX_DSL_TONE);

	printf("\nADSL Diagnostic starts!! \nThe test result will come out later. Please wait ...\n\n");
#ifdef _USE_NEW_IOCTL_FOR_DSLDIAG_
	//fprintf( stderr, "use RLCM_ENABLE_DIAGNOSTIC to start dsldiag\n" );
	mode=0;
	adsl_drv_get(RLCM_ENABLE_DIAGNOSTIC, (void *)&mode, sizeof(int));//Lupin
#else
	dbgmode = 41;
	adsl_drv_get(RLCM_DEBUG_MODE, (void *)&dbgmode, sizeof(int));
#endif
	adsl_drv_get(RLCM_MODEM_RETRAIN, NULL, 0);
	// wait until showtime
	while (!adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE) || vLs.upstreamRate == 0)
		usleep(1000000);
	usleep(5000000);
	adsl_drv_get(RLCM_GET_LD_STATE, (void *)&ldstate, 4);
	if (ldstate != 0)
		printf(Tadsl_diag_suc);
	else
		printf(Tadsl_diag_fail);
	// get the channel number
	if(adsl_drv_get(RLCM_GET_SHOWTIME_XDSL_MODE, (void *)&mode, 1)) {
		//ramen to clear the first 3 bit
		mode &= 0x1F;
		if (mode < 5) //adsl1/adsl2
			chan = 256;
		else
			chan = 512;
	}
	intp = fp = 0;
	str[0] = 0;

	if (adsl_drv_get(RLCM_GET_DIAG_HLOG, (void *)hlog, sizeof(short)*(MAX_DSL_TONE*3+HLOG_ADDITIONAL_SIZE))) {
		adsl_drv_get(RLCM_GET_DIAG_SNR, (void *)snr, sizeof(short)*MAX_DSL_TONE);
		adsl_drv_get(RLCM_GET_DIAG_QLN, (void *)qln, sizeof(short)*MAX_DSL_TONE);
		diagflag = 1;
		printf("\n\t\t\t%s\t%s\n", Tdownstream, Tupstream);
		printf("Hlin Scale\t\t%d\t\t %d\n", (unsigned short)hlog[chan*3+1], (unsigned short)hlog[chan*3]);
		printf("Loop Attenuation(dB)\t%d.%d\t\t %d.%d\n", hlog[chan*3+3]/10, abs(hlog[chan*3+3]%10), hlog[chan*3+2]/10, abs(hlog[chan*3+2]%10));
		printf("Signal Attenuation(dB)\t%d.%d\t\t %d.%d\n", hlog[chan*3+5]/10, abs(hlog[chan*3+5]%10), hlog[chan*3+4]/10, abs(hlog[chan*3+4]%10));
		printf("SNR Margin(dB)\t\t%d.%d\t\t %d.%d\n", hlog[chan*3+7]/10, abs(hlog[chan*3+7]%10), hlog[chan*3+6]/10, abs(hlog[chan*3+6]%10));
		printf("Attainable Rate(Kbps)\t%d\t\t %d\n", hlog[chan*3+9], hlog[chan*3+8]);
		printf("Output Power(dBm)\t%d.%d\t\t %d.%d\n", hlog[chan*3+11]/10, abs(hlog[chan*3+11]%10), hlog[chan*3+10]/10, abs(hlog[chan*3+10]%10));
	}
	else {
		diagflag = 0;
		printf("\t\t\t%s\t%s\n", Tdownstream, Tupstream);
		printf("Hlin Scale\n");
		printf("Loop Attenuation(dB)\n");
		printf("Signal Attenuation(dB)\n");
		printf("SNR Margin(dB)\n");
		printf("Attainable Rate(Kbps)\n");
		printf("Output Power(dBm)\n");
	}
	
	printf("\n%s\tH.Real\tH.Image\tSNR\tQLN\tHlog", Ttone_num);
	for (i = 0; i < chan; i++) {
		printf("\n%d", i);
		if (diagflag) {
			intp = hlog[i+chan]/1000;
			fp = abs(hlog[i+chan]%1000);
			if (fp<0) {
				fp = -fp;
				if (intp == 0)
					snprintf(str, 16, "-0.%03d", fp);
				else
					snprintf(str, 16, "%d.%03d", intp, fp);
			}
			else
				snprintf(str, 16, "%d.%03d", intp, fp);
		}
		printf("\t\t%s", str);
		if (diagflag) {
			intp = hlog[i+chan*2]/1000;
			fp = abs(hlog[i+chan*2]%1000);
			if (fp<0) {
				fp = -fp;
				if (intp == 0)
					snprintf(str, 16, "-0.%03d", fp);
				else
					snprintf(str, 16, "%d.%03d", intp, fp);
			}
			else
				snprintf(str, 16, "%d.%03d", intp, fp);
		}
		printf("\t%s", str);
		if (diagflag) {
			intp = snr[i]/10;
			fp = abs(snr[i]%10);
			snprintf(str, 16, "%d.%d", intp, fp);
		}
		printf("\t%s", str);
		if (diagflag) {
			intp = qln[i]/10;
			fp = abs(qln[i]%10);
			if (fp<0) {
				if (intp != 0)
					snprintf(str, 16, "%d.%d", intp, -fp);
				else
					snprintf(str, 16, "-%d.%d", intp, -fp);
			}
			else
				snprintf(str, 16, "%d.%d", intp, fp);
		}
		printf("\t%s", str);
		if (diagflag) {
			intp = hlog[i]/10;
			fp = abs(hlog[i]%10);
			if (fp<0) {
				if (intp != 0)
					snprintf(str, 16, "%d.%d", intp, -fp);
				else
					snprintf(str, 16, "-%d.%d", intp, -fp);
			}
			else
				snprintf(str, 16, "%d.%d", intp, fp);
		}
		printf("\t%s", str);
	}
	printWaitStr();
	free(snr);
	free(qln);
	free(hlog);
}

void setPing()
{
	char pingAddr[16];
	int conti;
	
	while (1)
	{
		printf("\n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("                           Ping Diagnostic                               \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("This page is used to send ICMP ECHO_REQUEST packets to network host. The \n");
		printf("diagnostic result will then be displayed.                                \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		
		printf("Host Address :");
		if ( getInputStr( pingAddr, sizeof(pingAddr)-1, NULL)==0)return;
		//cliping(pingAddr);
		utilping(pingAddr);
		printf("Continue (1)Yes (2)No :");
		if( (conti=getInputNum()) ==2) return;
		continue;
	}//end while, Ping Diagnostic
}

static int finished = 0;
static void lbTimeout()
{
	finished = 1;
}
void cliOamLb(int oam_flow,int oam_vpi,int oam_vci,char *oam_llid)
{
#ifdef EMBED
	char	*str, *submitUrl;
	char tmpBuf[100];
	int	skfd,i,j;
	struct atmif_sioc mysio;
	ATMOAMLBReq lbReq;
	ATMOAMLBState lbState;
	int curidx, len;
	char *tmpStr;
	unsigned char *tmpValue;
	

	if((skfd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0){
		printf("socket open error");
		return;
	}
	
	memset(&lbReq, 0, sizeof(ATMOAMLBReq));
	memset(&lbState, 0, sizeof(ATMOAMLBState));
	
	if (oam_flow == 0)
		lbReq.Scope = 0;	// Segment
	else if (oam_flow== 1)
		lbReq.Scope = 1;	// End-to-End
	
	// VPI (0~255)
	if ( oam_vpi>255) {
		strcpy(tmpBuf, "Connection not exists!");
		goto setErr_oamlb;
	}
	else
		lbReq.vpi = oam_vpi;

	// VCI (0~65535)
	if ( oam_vci>65535) {
		strcpy(tmpBuf, ("Connection not exists!"));
		goto setErr_oamlb;
	}
	else
		lbReq.vci = oam_vci;

	str = oam_llid;
	// convert max of 32 hex decimal string into its 16 octets value
	len = strlen(str);
	curidx = 16;
	for (i=0; i<32; i+=2)
	{
		// Loopback Location ID
		curidx--;
		tmpValue = (unsigned char *)&lbReq.LocID[curidx];
		if (len > 0)
		{
			len -= 2;
			if (len < 0)
				len = 0;
			tmpStr = str + len;
			*tmpValue = strtoul(tmpStr, 0, 16);
			*tmpStr='\0';
		}
		else
			*tmpValue = 0;
	}
		
	mysio.number = 0;	// ATM interface number
	mysio.arg = (void *)&lbReq;
	// Start the loopback test
	if (ioctl(skfd, ATM_OAM_LB_START, &mysio)<0) {
		strcpy(tmpBuf, "ioctl: ATM_OAM_LB_START failed !");
		close(skfd);
		goto setErr_oamlb;
	}
	
	finished = 0;
	signal(SIGALRM, lbTimeout);
	alarm(MAXWAIT);
	// Query the loopback status
	mysio.arg = (void *)&lbState;
	lbState.vpi = oam_vpi;
	lbState.vci = oam_vci;
	lbState.Tag = lbReq.Tag;
	
	while (1)
	{
		if (finished)
			break;	// break for timeout
		
		if (ioctl(skfd, ATM_OAM_LB_STATUS, &mysio)<0) {
			strcpy(tmpBuf, "ioctl: ATM_OAM_LB_STATUS failed !");
			mysio.arg = (void *)&lbReq;
			ioctl(skfd, ATM_OAM_LB_STOP, &mysio);
			close(skfd);
			goto setErr_oamlb;
		}
		
		if (lbState.count[0] > 0)
		{
			break;	// break for loopback success
		}
	}
		
	mysio.arg = (void *)&lbReq;
	// Stop the loopback test
	if (ioctl(skfd, ATM_OAM_LB_STOP, &mysio)<0) {
		strcpy(tmpBuf, "ioctl: ATM_OAM_LB_STOP failed !");
		close(skfd);
		goto setErr_oamlb;
	}
	close(skfd);


	if (!finished)
	{
		printf("\n--- Loopback cell received successfully ---\n");
	}
	else
	{
		printf("\n--- Loopback failed ---\n");
	}
	printf("\n");
	
  	return;

setErr_oamlb:
	printf("%s\n\n",tmpBuf);
#endif
}

void setATMLoopback()
{
	char oam_llid[33];
	int oam_flow, oam_vpi, oam_vci;
	int conti;
	
	while (1)
	{
		printf("\n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("           OAM Fault Management - Connectivity Verification              \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("Connectivity verification is supported by the use of the OAM loopback    \n");
		printf("capability for both VP and VC connections. This page is used to perform  \n");
		printf("the VCC loopback function to check the connectivity of the VCC.          \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		
		printf("Flow Type (1)F5 Segment (2)F5 End-to-End : ");
		if( (oam_flow=getInputNum()) ==0) continue;
		printf("VPI:");
		oam_vpi=getInputNum();
		printf("VCI: ");
		oam_vci=getInputNum();
		printf("Loopback Location ID (default:FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF) :\n");
		if ( getInputStr( oam_llid, sizeof(oam_llid)-1, NULL)==0)continue;
		cliOamLb(oam_flow,oam_vpi,oam_vci,oam_llid);
		printf("Continue (1)Yes (2)No : ");
		if( (conti=getInputNum()) ==2) return;
		continue;
	}//end while, OAM Fault Management
}

#ifdef DIAGNOSTIC_TEST
static const char R_PASS[] = ": PASS";
static const char R_FAIL[] = " : FAIL";
static int cmode = 0;
int eth=0, adslflag=0, pppserver=0, auth=0, ipup=0, lb5s=0, lb5e=0, lb4s=0, lb4e=0, dgw=0, pdns=0;

void runDiagTest(int idx)
{
	int inf=-1, i, pppif;
	MIB_CE_ATM_VC_T Entry;
	unsigned int entryNum;
	FILE *fp;
	char buff[10], ifname[6];
	MIB_CE_ATM_VC_Tp pEntry;

	Modem_LinkSpeed vLs;

	if (!adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE) || vLs.upstreamRate == 0)
		adslflag = 0;
	else
		adslflag = 1;

	mib_chain_get(MIB_ATM_VC_TBL, idx, (void *)&Entry);
	inf = Entry.ifIndex;
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);

	for (i=0;i<entryNum;i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) {
			printf("Get chain record error!\n");
			return;
		}
		if (Entry.enable == 0)
			continue;
		if (inf == -1)
			inf = Entry.ifIndex;
		if (Entry.ifIndex == inf) {
			if (Entry.cmode != ADSL_BR1483) {
				struct in_addr inAddr;
				int flags;
				cmode = 1;
				if (PPP_INDEX(Entry.ifIndex) != 0x0f) { // PPP Interface
					int pppflag;
					cmode = 2;
					sprintf(ifname, "ppp%d", PPP_INDEX(Entry.ifIndex));
					if (fp = fopen("/tmp/ppp_diag_log", "r")) {
						while (fgets(buff, sizeof(buff), fp) != NULL) {
							sscanf(buff, "%d:%d", &pppif, &pppflag);
							if (pppif == PPP_INDEX(Entry.ifIndex))
									break;
						}
						fclose(fp);
					}
					switch(pppflag) {
					case 1:
						pppserver = 1;
						auth = ipup = 0;
						break;
					case 2:
						pppserver = auth = 1;
						ipup = 0;
						break;
					case 3:
						pppserver = auth = ipup = 1;
						break;
					case 0:
					default:
						pppserver = auth = ipup = 0;
						break;
					}
				}
				else
					sprintf(ifname, "vc%u", VC_INDEX(Entry.ifIndex));
			}
			else
				cmode = 0;
			break;
		}
	}
	pEntry = &Entry;
	if (adslflag) {
		lb5s = testOAMLookback(pEntry, 0, 5);
		lb5e = testOAMLookback(pEntry, 1, 5);
		lb4s = testOAMLookback(pEntry, 0, 4);
		lb4e = testOAMLookback(pEntry, 1, 4);
		if (cmode > 0) {
			char pingaddr[16];
			memset(pingaddr, 0x00, 16);
			if (defaultGWAddr(pingaddr))
				dgw = 0;
			else
				dgw = utilping(pingaddr);
			memset(pingaddr, 0x00, 16);
			if (pdnsAddr(pingaddr))
				pdns = 0;
			else
				pdns = utilping(pingaddr);
		}
	}
}

void diagTestResult(int idx)
{
	struct ifreq ifrq;

	strncpy(ifrq.ifr_name, ELANIF, sizeof(ifrq.ifr_name));
	ifrq.ifr_name[ sizeof(ifrq.ifr_name)-1] = 0;
	eth = getLinkStatus(&ifrq);
	runDiagTest(idx);
	CLEAR;
	printf("\n");
	printf("%s\n", Tlan_conn_chk);
	printf("%s%s\n\n", Ttest_eth_conn, (eth)?R_PASS: R_FAIL);
	printf("%s\n", Tadsl_conn_chk);
	printf("%s%s\n", Ttest_adsl_syn, (adslflag)?R_PASS: R_FAIL);
	printf("%s%s\n", Ttest_oam_f5_seg, (lb5s)?R_PASS: R_FAIL);
	printf("%s%s\n", Ttest_oam_f5_end, (lb5e)?R_PASS: R_FAIL);
	printf("%s%s\n", Ttest_oam_f4_seg, (lb4s)?R_PASS: R_FAIL);
	printf("%s%s\n\n", Ttest_oam_f4_end, (lb4e)?R_PASS: R_FAIL);
	if (cmode > 0) {
		printf("%s\n", Tint_conn_chk);
		if (cmode == 2) {
			printf("%s%s\n", Ttest_ppps_conn, (pppserver)?R_PASS: R_FAIL);
			printf("%s%s\n", Ttest_auth, (auth==1)?R_PASS: R_FAIL);
			printf("%s%s\n", Ttest_assigned_ip, (ipup)?R_PASS: R_FAIL);
		}
		printf("%s%s\n", Tping_def_gw, (dgw)?R_PASS: R_FAIL);
		printf("%s%s\n\n", Tping_pri_dnss, (pdns)?R_PASS: R_FAIL);
	}
	cmode = eth = adslflag = pppserver = auth = ipup = lb5s = lb5e = lb4s = lb4e = dgw = pdns=0;
	printWaitStr();
}

void setDiagnosticTest()
{
	int sel, min, max, conti;

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("			Diagnostic Test 				  \n");
		printf("-------------------------------------------------------------------------\n");
		printf("The DSL Router is capable of testing your DSL connection. The individual \n");
		printf("tests are listed below. If a test displays a fail status, click \"Run \n");
		printf("Diagnostic Test\" button again to make sure the fail status is consistent. \n");
		printf("-------------------------------------------------------------------------\n");

		max = showWanVC(0);
		if (max == 0) {
			printf("No WAN Interface!\n");
			printWaitStr();
			return;
		}
		min = 1;
		if (getInputUint("Select the Internet Connection:", &sel, &min, &max) == 0) {
			printf("Invalid selection!\n");
			printWaitStr();
			return;
		}
		diagTestResult(sel-1);
		printf("Continue (1)Yes (2)No : ");
		if ((conti=getInputNum()) == 2)
			return;
		continue;
	}
}
#endif

void setDiagnostic()
{
	int snum;
	char chVal[2];
	
	while (1)
	{
		CLEAR;
	 	printf("\n");
	 	printf("------------------------------------------------------------\n");
	 	printf("                 (7) Diagnostic Menu                        \n");
	 	printf("------------------------------------------------------------\n");
	 	printf("(1) Ping                         (2) ATM Loopback\n");
	 	printf("(3) ADSL Diagnostic              (4) Diagnostic Test\n");
	 	printf("(5) Quit\n");
	 	if (!getInputOption( &snum, 1, 5))
	 		continue;
	 	
	 	switch( snum)
	 	{
	 	case 1://(1) Ping
	 		setPing();
	 		break;
	 	case 2://(2) ATM Loopback
	 		setATMLoopback();
	 		break;
		case 3://(3) ADSL Diagnostic
			if (adsl_drv_get(RLCM_GET_SHOWTIME_XDSL_MODE, (void *)&chVal[0], 1)) {
				chVal[0] &= 0x1F;
				if (chVal[0] == 3 || chVal[0] == 5)  // ADSL2/2+
					startADSLDiagnostic();
				else {
					printf("Not supported! Only ADSL2/2+ support this function.\n");
					printWaitStr();
				}
			}
			break;
		case 4://(4) Diagnostic Test
			#ifdef DIAGNOSTIC_TEST
			setDiagnosticTest();
			#else
			printf("Not supported\n");
			printWaitStr();
			#endif
			break;
	 	case 5://(5) Quit
	 		return;
	 	default:
	 		printf("!! Invalid Selection !!\n");
	 	}//end switch, (7) Diagnostic Menu
	}//end while, (7) Diagnostic Menu
}

/*************************************************************************************************/
#include <semaphore.h>
extern sem_t semSave;
void setCommitReboot()
{
	int snum, rebootMode, rebootflag;
	unsigned char vChar;
	BOOT_TYPE_T cur_btmode;
	 
	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("                           Commit/Reboot                                 \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("This page is used to commit changes to system memory and reboot your     \n");
		printf("system.                                                                  \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("(1) Commit&Reboot                (2) Restore to Default\n");
		printf("(3) Quit\n");
		if (!getInputOption( &snum, 1, 3))
			continue;
		
		switch( snum)
		{
		case 1://(1) Set
         		printf("Commit and Reboot? (1)Yes (2)No :");
          		if( (rebootMode=getInputNum()) ==0) break;
          		if (rebootMode == 1) {
				/* Save and reboot the system */
   				printf("%s\n\n", rebootWord0);
   				printf("%s\n", rebootWord1);
   				printf("%s\n", rebootWord2);
				// wake up the Save/reboot thread
				sem_post(&semSave);
				sem_wait(&semSave);
			}
	 		break;
		case 2://(2) Restore to Default
			printf("Restore to Default and Reboot? (1)Yes (2)No :");
          		if( (rebootMode=getInputNum()) ==0) break;
          		if (rebootMode == 1) {
#if (defined(CONFIG_RTL867X_NETLOG)  && defined (CONFIG_USER_NETLOGGER_SUPPORT))          
			        va_cmd("/bin/netlogger",1,1,"disable");
					  	        sleep(1);
#endif
          			// load default
				//mib_load(DEFAULT_SETTING, CONFIG_MIB_ALL);
				
				// Mason Yu
				//va_cmd("/bin/flash", 2, 1, "default", "ds");
				va_cmd("/bin/flash", 2, 1, "default", "cs");
				semSave.__sem_value=SEM_REBOOT;
		
				/* Save and reboot the system */
				// wake up the Save/reboot thread
				sem_post(&semSave);
				//sem_wait(&semSave);
			}
			break;
		case 3://(3) Quit
			return;
		}//end switch, Commit/Reboot
	}//end while, Commit/Reboot
}

#ifdef ACCOUNT_CONFIG
static void addUser()
{
	char newuser[MAX_NAME_LEN], newpass[MAX_NAME_LEN], confpass[MAX_NAME_LEN], suName[MAX_NAME_LEN], usName[MAX_NAME_LEN];
	int entryNum, i, intVal;
	MIB_CE_ACCOUNT_CONFIG_T Entry;

	printf("Please input User Name:");
	if (getInputStr(newuser, sizeof(newuser) - 1, NULL) == 0) return;
	if (newuser[strlen(newuser) - 1] == '\n')
		newuser[strlen(newuser) - 1] = 0;

	if (strlen(newuser) > 0) {
		mib_get(MIB_SUSER_NAME, (void *)suName);
		mib_get(MIB_USER_NAME, (void *)usName);
		if ((strcmp(suName, newuser) == 0) || (strcmp(usName, newuser) == 0)) {
			printf("ERROR: user already exists!");
			return;
		}
		entryNum = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL);
		for (i=0; i<entryNum; i++) {
			if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, i, (void *)&Entry))
	  			printf ("Get chain record error!\n");
			if (strcmp(Entry.userName, newuser) == 0) {
				printf("ERROR: user already exists!");
				return;
			}
		}
	}
	else {
		printf("Error! User name cannot be empry!\n");
		return;
	}
	strncpy(Entry.userName, newuser, MAX_NAME_LEN-1);
	Entry.userName[MAX_NAME_LEN-1] = '\0';
	//Entry.userName[MAX_NAME_LEN] = '\0';

	printf("Password:");
	if (getInputStr(newpass, sizeof(newpass) - 1, NULL) == 0) return;
	printf("Confirme Password:");
	if (getInputStr(confpass, sizeof(confpass) - 1, NULL) == 0) return;    
	if (newpass[strlen(newpass) - 1] == '\n')
		newpass[strlen(newpass) - 1] = 0;
	if (confpass[strlen(confpass) - 1] == '\n')
		confpass[strlen(confpass) - 1] = 0;

	if (strcmp(newpass, confpass) == 0) {   	
		strncpy(Entry.userPassword, newpass, MAX_NAME_LEN-1);
		Entry.userPassword[MAX_NAME_LEN-1] = '\0';
		//Entry.userPassword[MAX_NAME_LEN] = '\0';
		Entry.privilege = (unsigned char)PRIV_USER;
		intVal = mib_chain_add(MIB_ACCOUNT_CONFIG_TBL, (unsigned char*)&Entry);
		if (intVal == 0) {
			printf(strAddChainerror);
   			return;
		}
		else if (intVal == -1) {
			printf(strTableFull);
   			return;
		}
	} else
		printf("Confirmed Password is not corrct ! Plaese Input it again\n");
}

static void modifyPassword()
{
	char oldpass[MAX_NAME_LEN], newpass[MAX_NAME_LEN], confpass[MAX_NAME_LEN], oldMIBpass[MAX_NAME_LEN];
	int entryNum, i, selnum, min;
	MIB_CE_ACCOUNT_CONFIG_T Entry;

	entryNum = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL) + 2;
	min = 1;;
	if(getInputUint("Select user account:", &selnum, &min, &entryNum) == 0){
		printf("Invalid selection!\n");
		return;
	}
	printf("Old Password:");
	if (getInputStr(oldpass, sizeof(oldpass) - 1, NULL) == 0) return;
	if (oldpass[strlen(oldpass) - 1] == '\n')
		oldpass[strlen(oldpass) - 1] = 0;
	if (selnum == 1) {
   		if (!mib_get(MIB_SUSER_PASSWORD, (void *)oldMIBpass))
			printf("ERROR(modifyPassword): Get super user password from MIB database failed.\n");
   		if (strcmp(oldpass, oldMIBpass) != 0) {
   			printf("Old Password is not correct ! Please Input it again\n");
			return;
   		}
	}
	else if (selnum == 2) {
   		if (!mib_get(MIB_USER_PASSWORD, (void *)oldMIBpass))
			printf("ERROR(modifyPassword): Get user password from MIB database failed.\n");
   		if (strcmp(oldpass, oldMIBpass) != 0) {
   			printf("Old Password is not correct ! Please Input it again\n");
			return;
   		}
	}
	else {
		if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, selnum -3, (void *)&Entry))
			printf("Get chain record error!\n");
		if (strcmp(Entry.userPassword, oldpass) != 0) {
			printf("Old Password is not corrct ! Plaese Input it again\n");
			return;
		}
	}

	printf("New Password:");
	if (getInputStr(newpass, sizeof(newpass) - 1, NULL) == 0) return;
	printf("Confirme Password:");
	if (getInputStr(confpass, sizeof(confpass) - 1, NULL) == 0) return;    

	if (newpass[strlen(newpass) - 1] == '\n')
		newpass[strlen(newpass) - 1] = 0;
	if (confpass[strlen(confpass) - 1] == '\n')
		confpass[strlen(confpass) - 1] = 0;

	if (strcmp(newpass, confpass) == 0) {   	
		if (selnum == 1) {
			if (!mib_set(MIB_SUSER_PASSWORD, (void *)confpass))
				printf("ERROR(modifyPassword): Set Super user password to MIB database failed.\n");	
		} else if (selnum == 2) {
			if (!mib_set(MIB_USER_PASSWORD, (void *)confpass))
				printf("ERROR(modifyPassword): Set user password to MIB database failed.\n");	
		} else {
			strncpy(Entry.userPassword, newpass, MAX_NAME_LEN-1);
			Entry.userPassword[MAX_NAME_LEN-1] = '\0';
			//Entry.userPassword[MAX_NAME_LEN] = '\0';
			Entry.privilege = (unsigned char)getAccPriv(Entry.userName);
			mib_chain_update(MIB_ACCOUNT_CONFIG_TBL, (void *)&Entry, selnum -3);
		}
	} else
		printf("Confirmed Password is not corrct ! Plaese Input it again\n");
}
#endif

// Kaohj -- tftp update image/configuration and backup configuration
#ifdef CONFIG_USER_BUSYBOX_TFTP
void setTftp()
{
	char serverIP[32], fname[32];
	char *pIp;
	struct in_addr inIp;
	int snum;
	
	if (0 == getInputIpAddr("Tftp server address:", &inIp))
		return;
	pIp = inet_ntoa(*((struct in_addr *)&inIp));
	strncpy(serverIP, pIp, sizeof(serverIP));
	serverIP[sizeof(serverIP)-1] = 0;

	while (1) {
		CLEAR;
		printf("\n");
		MSG_LINE;
		printf("                           Tftp utility                                \n");
		MSG_LINE;
		printf("Update firmware/configuration or backup configuration from/to a tftp server.\n");
		printf("Server address: %s\n", serverIP);
		MSG_LINE;
   		
		printf("(1) Update image                            (2) Update config.\n");
   		printf("(3) Backup config.                          (4) Quit\n");
   		if (!getInputOption( &snum, 1, 4))
   			continue;
   		
		switch( snum)
		{
		case 1:
			printf("Image update\n");
			if (0==getInputString("Image file name:", fname, sizeof(fname)))
				break;
			va_cmd("/bin/tftp", 5, 1, "-g", "-i", "-f", fname, serverIP);
			printWaitStr();
			break;
		case 2:
			printf("Configuration update\n");
			if (0==getInputString("Configuration file name:", fname, sizeof(fname)))
				break;
			va_cmd("/bin/tftp", 5, 1, "-g", "-c", "-f", fname, serverIP);
			printWaitStr();
			break;
		case 3:
			printf("Configuration backup\n");
			if (0==getInputString("Configuration file name:", fname, sizeof(fname)))
				break;
			va_cmd("/bin/tftp", 5, 1, "-p", "-c", "-f", fname, serverIP);
			printWaitStr();
			break;
		case 4:
			return;
		}
   	}
}
#endif

void setPassword()
{
   char oldpass[30], newpass[30], confpass[30], oldMIBpass[30];
   int snum;
   int i;
   	int nameChange;
	char usName[MAX_NAME_LEN];
#ifdef ACCOUNT_CONFIG
	MIB_CE_ACCOUNT_CONFIG_T Entry;
	int entryNum;
#endif
   
	
   while(1) {
   
   	printf("\n");
	MSG_LINE;
   	//printf("-------------------------------------------------------------------------\n");
#ifdef ACCOUNT_CONFIG
   	printf("                           Account Configuration                                \n");
#else
   	printf("                           Password Setup                                \n");
#endif
	MSG_LINE;
   	//printf("-------------------------------------------------------------------------\n");
   	printf("This page is used to set the account to access the web server of ADSL    \n");
   	printf("Router.                                                                  \n");
	MSG_LINE;
   	//printf("-------------------------------------------------------------------------\n");
   	
#ifdef ACCOUNT_CONFIG
	printf("\nIdx  UserName\n");
	printf("----------------\n");
	mib_get(MIB_SUSER_NAME, (void *)usName);
	printf ("1    %s\n", usName);
	mib_get(MIB_USER_NAME, (void *)usName);
	printf ("2    %s\n", usName);
	entryNum = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL);
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, i, (void *)&Entry))
  			printf ("Get chain record error!\n");
		printf ("%-5d%s\n", i+3, Entry.userName);
	}

   	printf("\n(1) Add User                          (2) Set Password\n");
   	printf("(3) Quit\n");
	if (!getInputOption(&snum, 1, 3))
		continue;

	switch (snum) {
		case 1:
			addUser();
#if defined(APPLY_CHANGE)   		
   			// Added by Mason Yu for take effect on real time
			writePasswdFile();
			write_etcPassword();	// Jenny
#endif			
   			break;

		case 2:
			modifyPassword();
#if defined(APPLY_CHANGE)   		
   			// Added by Mason Yu for take effect on real time
			writePasswdFile();
			write_etcPassword();	// Jenny
#endif			
   			break;

		case 3:
			return;
   	}
#else
   	printf("(1) Set Super User                          (2) Set General User\n");
   	printf("(3) Quit\n\n");
   	if (!getInputOption( &snum, 1, 3))
   		continue;
   	
   	switch( snum)
   	{
   	case 1:
   	case 2:
   		//printf("User Name:");
   		//if ( getInputStr( username, sizeof(username)-1)==0)return;
   		if (snum == 1) {
			mib_get(MIB_SUSER_NAME, (void *)usName);
   			printf("User Name: %s\n", usName);
   		}
   		else {
			mib_get(MIB_USER_NAME, (void *)usName);
   			printf("User Name: %s\n", usName);
   		}
		if (0==getInputString("User Name(Press 'Enter' to leave unchanged):", usName, sizeof(usName)))
			nameChange = 0;
		else
			nameChange = 1;
   		
CONFIRM:   
		printf("Old Password:");
   		if ( getInputStr( oldpass, sizeof(oldpass)-1, NULL)==0)break;
   		
   		printf("New Password:");
   		if ( getInputStr( newpass, sizeof(newpass)-1, NULL)==0)break;
   		
   		printf("Confirmed Password:");
   		if ( getInputStr( confpass, sizeof(confpass)-1, NULL)==0)break;    
   		
   		if ( snum == 1 ) {
   			if ( !mib_get(MIB_SUSER_PASSWORD, (void *)oldMIBpass) ) {
				printf("ERROR(setPassword): Get super user password from MIB database failed.\n");	
  			}
   		}else if ( snum == 2 ) {
   			if ( !mib_get(MIB_USER_PASSWORD, (void *)oldMIBpass) ) {
				printf("ERROR(setPassword): Get user password from MIB database failed.\n");	
  			}
   		}   		
   		   			
   		if ( strcmp(oldpass, oldMIBpass) != 0 ) {
   			printf("Old Password is not correct ! Please Input it again\n");
   			//goto CONFIRM;
   			break;
   		}
   	
   		if ( strcmp(newpass, confpass) == 0 ) {   	
   		 	/* Set user account to MIB */
   		 	//printf("username=%s  newpass=%s  confpass=%s\n", username, newpass, confpass);
   		 	
   		 	if ( snum == 1 ) {
   		 		if (nameChange)
   		 			mib_set(MIB_SUSER_NAME, (void *)usName);
   				if ( !mib_set(MIB_SUSER_PASSWORD, (void *)confpass) ) {
					printf("ERROR(setPassword): Set Super user password to MIB database failed.\n");	
  				}
  			}else if ( snum == 2 ) {
   		 		if (nameChange)
   		 			mib_set(MIB_USER_NAME, (void *)usName);
  				if ( !mib_set(MIB_USER_PASSWORD, (void *)confpass) ) {
					printf("ERROR(setPassword): Set user password to MIB database failed.\n");	
  				}
  			}	
   		}else  {
   			printf("Confirmed Password is not correct ! Please Input it again\n");
   			//goto CONFIRM;
   			break;
   		}	
#if defined(APPLY_CHANGE)   		
   		// Added by Mason Yu for take effect on real time
		writePasswdFile();
		write_etcPassword();	// Jenny
#endif
   		break;
   		
   	case 3:
		return;
   	}
#endif

   } // while(1)   	
		
}


// remote config status flag: 0: disabled, 1: enabled
//#define MAX_NAME_LEN					30
static int srandomCalled = 0;


#ifdef IP_ACL
void aclDel()
{
	unsigned int totalEntry;
	unsigned int i, index;
	int del, min, max;
	
	totalEntry = mib_chain_total(MIB_ACL_IP_TBL); /* get chain record size */
	if (totalEntry==0) {
		printf("Empty table!\n");
		return;
	}
	min=1; max=2;
	if (getInputUint("Delete (1)One (2)All :", &del, &min, &max) == 0) {
		printf("Invalid selection!\n");
		return;
	}

	if (del == 2)
		mib_chain_clear(MIB_ACL_IP_TBL); /* clear chain record */
	else if (del == 1) {
		min=1; max=totalEntry;
		if (getInputUint("Select the index to delete:", &index, &min, &max) == 0) {
			printf("Error selection!\n");
			return;
		}
		if(mib_chain_delete(MIB_ACL_IP_TBL, index-1) != 1)
			printf("Delete chain record error!");
	}
}

void setACL()
{
	int snum, aclmode, conf, delnum;
	unsigned char vChar;
	unsigned int entryNum, i;
	MIB_CE_ACL_IP_T Entry;	
	struct in_addr dest;	
	char sdest[16];
	unsigned long mask, mbit;
	int min, max, sel, intVal;
	
	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("                           ACL Configuration                             \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("This page is used to configure the IP Address for Access Control List.   \n");
		printf("Here you can add/delete IP Address.                                      \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("ACL Configuration:\n\n");
		
		if ( !mib_get( MIB_ACL_CAPABILITY, (void *)&vChar) )
			return;
		if (0 == vChar)
			printf("1. ACL Capability: Disabled\n");
		else
			printf("1. ACL Capability: Enabled\n");
		
		printf("2. ACL Table\n");
	  	printf("   Idx  State   Interface  IP Address\n");
		printf("---------------------------------------------\n");
		
		entryNum = mib_chain_total(MIB_ACL_IP_TBL);
		for (i=0; i<entryNum; i++) {
        	
			if (!mib_chain_get(MIB_ACL_IP_TBL, i, (void *)&Entry))
			{
  				printf("Get chain record error!\n");				
			}
				
			dest.s_addr = *(unsigned long *)Entry.ipAddr;
			
			// inet_ntoa is not reentrant, we have to
			// copy the static memory before reuse it
			strcpy(sdest, inet_ntoa(dest));
			snprintf(sdest, 20, "%s/%d", sdest, Entry.maskbit);
			printf("   %-5d%-8s%-11s%s\n", i+1, Entry.Enabled ? "Enable" : "Disable", (Entry.Interface == IF_DOMAIN_LAN)? "LAN" : "WAN", sdest);
			//printf("(%d) IP Address: %s\n", i+1, sdest);			
			
		}		
		
		printf("\n(1) Capability                   (2) Add");
		printf("\n(3) Delete                       (4) Quit\n\n");
      		if (!getInputOption( &snum, 1, 4))
      			continue;
		
		switch( snum)
		{
		case 1: // (1) Capability
			min = 1; max = 2;
			if (0 == getInputUint("ACL Capability (1) Disable (2) Enable: ", &sel, &min, &max))
				continue;
			vChar = (unsigned char)(sel-1);
         		mib_set(MIB_ACL_CAPABILITY, (void *)&vChar);
         		break;
         	case 2: // (2) Add
			if (0==getInputIpAddr("IP Address(xxx.xxx.xxx.xxx): ", (struct in_addr *)&Entry.ipAddr[0]))
				continue;
			
			//if (0==getInputIpAddr("Subnet Mask: ", (struct in_addr *)&mask))
			if (0==getInputIpMask("Subnet Mask: ", (struct in_addr *)&mask))
				continue;
			
			mbit=0;
			while (1) {
				if (mask&0x80000000) {
					mbit++;
					mask <<= 1;
				}
				else
					break;
			}
			Entry.maskbit = mbit;
			
			min = 1; max = 2;
			if (0 == getInputUint("Interface (1) LAN (2) WAN: ", &sel, &min, &max))
				continue;
			Entry.Interface = (sel==1 ? IF_DOMAIN_LAN:IF_DOMAIN_WAN);

			if (0 == getInputUint("State (1)Disable (2)Enable: ", &sel, &min, &max))
				continue;
			Entry.Enabled = sel-1;
			
			intVal = mib_chain_add(MIB_ACL_IP_TBL, (unsigned char*)&Entry);
			if (intVal == 0) {
				printf(strAddChainerror);					
			}
			else if (intVal == -1)
				printf(strTableFull);					
			break;
		case 3: // (3) Delete
			aclDel();
			printWaitStr();
			break;
		case 4: // (4) Quit
			return;
		}

#if defined(APPLY_CHANGE)
		restart_acl();
#endif			
		//return;
	}
		
	
}
#endif



#ifdef CONFIG_USER_CWMP_TR069
static void setTr069Config(int action) {
	char tmpStr[256+1];
	char vChar;
	unsigned int vInt, vMin, vMax;
	char old_vChar;
	char changeflag=0;
	unsigned int informInterv;
	char strPath[256+1];
	int old_port;
	unsigned char cwmp_flag=0;
	char cwmp_flag_value=0;
	
	// Enable/disable TR069
	if ( action == 1) {    // Enable/disable TR069
		
		if( mib_get( CWMP_FLAG, (void *)&cwmp_flag ) )
		{
			printf("TR069: %s\n", (cwmp_flag & CWMP_FLAG_AUTORUN)? "Enable":"Disable");
			
			vMin = 1; vMax =2;
			if (0==getInputUint("Config TR069 (1) Disable (2) Enable :", &vInt, &vMin, &vMax))
				return;
			vChar = (vInt == 1) ? 0 : 1;
		
			if( vChar == 0) {
				if ( cwmp_flag & CWMP_FLAG_AUTORUN ) 
					changeflag = 1;				 	
				 
				cwmp_flag = cwmp_flag & (~CWMP_FLAG_AUTORUN);
				cwmp_flag_value = 0;
			}else {
				if ( !(cwmp_flag & CWMP_FLAG_AUTORUN) ) 
					changeflag = 1;				
				 	
				cwmp_flag = cwmp_flag | CWMP_FLAG_AUTORUN;
				cwmp_flag_value = 1;
			}
			
			if ( !mib_set( CWMP_FLAG, (void *)&cwmp_flag)) {
				return;
			}
		}else{
			return;
		}
	} else if ( action == 2 ){    // Modify TR069
		// Get TR069 adminStatus(Disable or Enable)
		mib_get( CWMP_FLAG, (void *)&cwmp_flag );
		if ( cwmp_flag & CWMP_FLAG_AUTORUN ) 
			cwmp_flag_value = 1;
		else
			cwmp_flag_value = 0;
			
		// ACS URL
		if (0==getInputString("ACS URL:", tmpStr, sizeof(tmpStr)))
			return;
		mib_set(CWMP_ACS_URL, (void *)tmpStr);
		
		// ACS Username
		if (0==getInputString("ACS Username:", tmpStr, sizeof(tmpStr)))
			return;
		mib_set(CWMP_ACS_USERNAME, (void *)tmpStr);
		
		// ACS Password
		if (0==getInputString("ACS Password:", tmpStr, sizeof(tmpStr)))
			return;
		mib_set(CWMP_ACS_PASSWORD, (void *)tmpStr);
        	
		// Inform
		vMin = 1; vMax =2;
		if (0==getInputUint("Inform (1) Disable (2) Enable :", &vInt, &vMin, &vMax))
			return;
		vChar = (vInt == 1) ? 0 : 1;
		mib_get( CWMP_INFORM_ENABLE, (void*)&old_vChar);
		if(old_vChar != vChar){
			changeflag = 1;
			if ( !mib_set( CWMP_INFORM_ENABLE, (void *)&vChar)) {
				return;
			}
		}			
		
		// Inform Interval
		if (vChar) {
			vMin=1; vMax =4294967295;
			if (0==getInputUint("Inform Interval :", &vInt, &vMin, &vMax))
				return;
			mib_get( CWMP_INFORM_INTERVAL, (void*)&informInterv);
			if(vInt != informInterv){
				changeflag = 1;
				if ( !mib_set( CWMP_INFORM_INTERVAL, (void *)&vInt)) {
					return;
				}
			}		
		}
        	
        	
		printf("\nConnection Request\n");
		if (0==getInputString("Username:", tmpStr, sizeof(tmpStr)))
			return;
		mib_set(CWMP_CONREQ_USERNAME, (void *)tmpStr);
        	
		if (0==getInputString("Password:", tmpStr, sizeof(tmpStr)))
			return;
		mib_set(CWMP_CONREQ_PASSWORD, (void *)tmpStr);
        	
		// Path
		if (0==getInputString("Path:", tmpStr, 32 ))
			return;
		mib_get( CWMP_CONREQ_PATH, (void *)strPath);		
		if (strcmp(tmpStr,strPath)!=0){
			changeflag = 1;
			if ( !mib_set( CWMP_CONREQ_PATH, (void *)tmpStr)) {
				return;
			}
		}	
		
		// Port
		vMin = 1; vMax =65535;
		if (0==getInputUint("Port:", &vInt, &vMin, &vMax))
			return;
		mib_get( CWMP_CONREQ_PORT, (void *)&old_port);
		if ( vInt != old_port ) {
			changeflag = 1;
			if ( !mib_set( CWMP_CONREQ_PORT, (void *)&vInt)) {
				return;
			}
		}
	}

	// Mason Yu
#ifdef APPLY_CHANGE
	if ( changeflag ) {
		if ( cwmp_flag_value == 0 ) {  // disable TR069
			off_tr069();
		} else {                       // enable TR069
			off_tr069();
			if (-1==startCWMP()){				
				printf("Start tr069 Fail(CLI)\n");
				return;
			}
		}	
	}
#endif

}

void setTR069() {
	int snum;
	char tmpStr[256+1];
	char vChar;
	int vInt;
	while (1)
	{
		CLEAR;
		MSG_LINE;
		printf("                           TR-069 Configuration                             \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("(1) Show                 (2) Modify\n");
		printf("(3) Enable/Disable       (4) Quit\n");      

		if (!getInputOption( &snum, 1, 4))
			continue;

		switch( snum)
		{
		case 1://(1) Show
			mib_get(CWMP_ACS_URL, (void *)tmpStr);
			printf("ACS URL: %s\n", tmpStr);
			mib_get(CWMP_ACS_USERNAME, (void *)tmpStr);
			printf("ACS Username: %s\n", tmpStr);
			mib_get(CWMP_ACS_PASSWORD, (void *)tmpStr);
			printf("ACS Password: %s\n", tmpStr);
			mib_get(CWMP_INFORM_ENABLE, (void *)&vChar);
			printf("Inform: %s\n", vChar ? "Enabled" : "Disabled");
			if (vChar) {				
				mib_get(CWMP_INFORM_INTERVAL, (void *)&vInt);
				printf("Inform Interval: %d\n", vInt);
			}
			printf("\nConnection Request\n");
			mib_get(CWMP_CONREQ_USERNAME, (void *)tmpStr);
			printf("Username: %s\n", tmpStr);
			mib_get(CWMP_CONREQ_PASSWORD, (void *)tmpStr);
			printf("Password: %s\n", tmpStr);			
			mib_get(CWMP_CONREQ_PATH, (void *)tmpStr);
			printf("PATH: %s\n", tmpStr);
			mib_get(CWMP_CONREQ_PORT, (void *)&vInt);
			printf("Port: %d\n", vInt);
			
			printWaitStr();
			break;
		case 2:
			setTr069Config(2);   // Modify TR069
			break;
		case 3: 
			setTr069Config(1);   // Enable/disable TR069
			break;
		case 4:
			
			return;
		}

	} 
}

#endif

#ifdef CONFIG_USER_FTP_FTP_FTP
static void backupConfig_FTP() {
	char serverIP[16], username[30], password[30], filename[30], fname[30];
	FILE *fp; 

	if (0 == getInputString("FTP server IP:", serverIP, sizeof(serverIP)))
		return;
	if (0 == getInputString("User name:", username, sizeof(username)))
		return;
	if (0 == getInputString("Password:", password, sizeof(password)))
		return;
	if (0 == getInputString("Config file name:", filename, sizeof(filename)))
		return;
		
	call_cmd("/bin/CreatexmlConfig", 0, 1);	
	sprintf(fname, "/tmp/%s", filename);
	cmd_xml2file("/tmp/config.xml", fname);
	
	if ((fp = fopen("/var/ftpput_conf.txt", "w")) == NULL) {
		printf("***** Open file /var/ftpput_conf.txt failed !\n");
		return;
	}
 
	fprintf(fp, "open %s\n", serverIP);
	fprintf(fp, "user %s %s\n", username, password);  
	fprintf(fp, "lcd /tmp\n");             // It is a directory that config file exist on ADSL
	fprintf(fp, "bin\n");
	fprintf(fp, "put %s\n", filename);       // It is name of the config file on ADSL	
	fprintf(fp, "bye\n");
	fprintf(fp, "quit\n");
	fclose(fp);

	system("/bin/ftp -inv < /var/ftpput_conf.txt");
	//va_cmd("/bin/ftp", 3, 1, "-inv", "-f", "/var/ftpput_conf.txt");
}


static void restoreConfig_FTP() {
	char serverIP[16], username[30], password[30], filename[30], fname[30];
	int ret = -1;
	FILE *fp; 

	if (0 == getInputString("FTP server IP:", serverIP, sizeof(serverIP)))
		return;
	if (0 == getInputString("User name:", username, sizeof(username)))
		return;
	if (0 == getInputString("Password:", password, sizeof(password)))
		return;
	if (0 == getInputString("Config file name:", filename, sizeof(filename)))
		return;
		
	if ((fp = fopen("/var/ftpget_conf.txt", "w")) == NULL) {
		printf("***** Open file /var/ftpget_conf.txt failed !\n");
		return;
	}

	fprintf(fp, "open %s\n", serverIP);
	fprintf(fp, "user %s %s\n", username, password);  
	fprintf(fp, "lcd /tmp\n");             // The config file will be saved to this directory on ADSL
	fprintf(fp, "bin\n");	
	fprintf(fp, "get %s\n", filename);       // It is name of the config file on ADSL
	fprintf(fp, "bye\n");
	fprintf(fp, "quit\n");
	fclose(fp);

	system("/bin/ftp -inv < /var/ftpget_conf.txt");
	
	sprintf(fname, "/tmp/%s", filename);
	cmd_file2xml(fname, "/tmp/config.xml");
	
	printf("Load config, please wait ...\n");
	ret = call_cmd("/bin/LoadxmlConfig", 0, 1);
	if (ret == 0) { // load ok
		printf("Writing ...\n");
		mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
	}
	else { // load fail
		va_cmd("/bin/adslctrl",1,1,"disablemodemline");
		sleep(1);	
		sync();
		va_cmd("/bin/sarctl",1,1,"disable");
		va_cmd(IFCONFIG, 2, 1, (char *)ELANIF, "down");
		va_cmd(IFCONFIG, 2, 1, (char *)WLANIF, "down");
		printf("Parsing error!\n");
	}
	/* reboot the system */
	printf("Rebooting ...\n");
	cmd_reboot();
	
	/*
	if (ret == 0)
		printf("Please commit and reboot the systme to take effect the new configuration.\n");
	else
		printf("ERROR: Restore Config file failed! Invalid config file!\n");
	*/
}

static void updateImage_FTP() {
	char serverIP[16], username[30], password[30], filename[30], fname[30];
	int ret = -1;
	FILE *fp; 

	if (0 == getInputString("FTP server IP:", serverIP, sizeof(serverIP)))
		return;
	if (0 == getInputString("User name:", username, sizeof(username)))
		return;
	if (0 == getInputString("Password:", password, sizeof(password)))
		return;
	if (0 == getInputString("Image file name:", filename, sizeof(filename)))
		return;
		
	if ((fp = fopen("/var/ftpget_img.txt", "w")) == NULL) {
		printf("***** Open file /var/ftpget_img.txt failed !\n");
		return;
	}
 
	fprintf(fp, "open %s\n", serverIP);
	fprintf(fp, "user %s %s\n", username, password);  
	fprintf(fp, "lcd /tmp\n");               // The config file will be saved to this directory on ADSL
	fprintf(fp, "bin\n");
	fprintf(fp, "get %s\n", filename);       // It is name of the config file on ADSL
	fprintf(fp, "bye\n");
	fprintf(fp, "quit\n");
	fclose(fp);

	system("/bin/ftp -inv < /var/ftpget_img.txt");
	
	sprintf(fname, "/tmp/%s", filename);
	printf("Updating image ...\n");
	fflush(0);
	if (cmd_check_image(fname, 0)) {
		//fprintf(stderr, "Image Checksum Failed\n");
		cmd_upload(fname, 0);
	}
	cmd_reboot();
}

void setFtp()
{
	int snum;
	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		printf("                           Ftp utility                                      \n");
		MSG_LINE;
		printf("Update firmware/configuration or backup configuration from/to a Ftp server. \n");		
		MSG_LINE;
		
		/*
		printf("\n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("                 Backup/Restore Settings                                 \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("This page allows you to backup current settings to a file or restore the \n");
		printf("file which was saved previously by FTP. Besides, you could reset the current\n");
		printf("configuration to factory default.                                        \n");
		MSG_LINE;
		*/
		
		//printf("-------------------------------------------------------------------------\n");
	   	printf("(1) Backup Configuration File        (2) Restore Configuration File\n");
   		printf("(3) Update image                     (4) Quit\n");
	   	if (!getInputOption(&snum, 1, 4))
   			continue;
	
		switch (snum) {
			case 1:
				backupConfig_FTP();
				printWaitStr();
				break;
			case 2:
				restoreConfig_FTP();
				printWaitStr();
				break;
			case 3:
				updateImage_FTP();
				printWaitStr();
				break;
			case 4:
				return;
		}
	}	
}
#endif

#ifdef SYSLOG_SUPPORT
int ShowSysLog()
{
	FILE *fp;
	char  buf[150];		
	unsigned char vChar;

	if(first_time==1){
		mib_get(MIB_ADSL_DEBUG, (void *)&vChar);
		if(vChar==1)
			dbg_enable=1;
		first_time=0;
	}

	if(dbg_enable==0){
		fp = fopen("/var/log/messages.old", "r");
		if (fp) {
			while (fgets(buf, 150, fp))
				printf("%s\n", buf);
			fclose(fp);
		}
		
		fp = fopen("/var/log/messages", "r");
		if (fp == NULL){
			//printf("read log file fail!\n");
			goto err1;
		}
       	 
		while(fgets(buf,150,fp)){
			printf("%s\n", buf);
		}

		fclose(fp);
		printWaitStr();
		return 1;
	}
err1:
	return 0;
}



/* Return value:
 * 0: fail
 * 1: successful
 */
int SystemLogConfig(){
	unsigned char setVal;
	unsigned char getVal;
	unsigned int min,max,sel;
	char msgstr[10];

	mib_get(MIB_SYSLOG, (void *)&getVal);
	if(getVal==1)
		strcpy(msgstr,"ON");
	else
		strcpy(msgstr,"OFF");
	printf("System Log: %s\n",msgstr);
	min=1;max=2;
	if(getInputUint("New System Log (1)ON (2)OFF :",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		return 0;
	}

	if (sel==1)
		setVal = 1;
	else
		setVal = 0;	// default "off"
	if (!mib_set(MIB_SYSLOG, (void *)&setVal)) {
		printf("Set System Log error!");
	}

#if defined(APPLY_CHANGE)
	stopLog();
	startLog();
#endif	
	return 1;
}

void setSystemLog()
{
	int snum;
	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		printf("                           System Log                                       \n");
		MSG_LINE;
		//printf("Update firmware/configuration or backup configuration from/to a Ftp server. \n");		
		MSG_LINE;
		
		//printf("-------------------------------------------------------------------------\n");
	   	printf("(1) System Log Enable/Disable        (2) Show Log\n");
   		printf("(3) Clear Log                        (4) Quit\n");
	   	if (!getInputOption(&snum, 1, 4))
   			continue;
	
		switch (snum) {
			case 1:
				if (!check_access(SECURITY_SUPERUSER))
					break;
				SystemLogConfig();				
				break;
			case 2:
				if (!check_access(SECURITY_SUPERUSER))
					break;
				ShowSysLog();
				break;
			case 3:
				unlink("/var/log/messages.old");
				unlink("/var/log/messages");
				printf("Clear Log is finished !!\n");
				printWaitStr();
				break;		
			case 4:
				return;
		}
	}	
}
#endif

#ifdef TIME_ZONE
void showCurTime()
{
	time_t tm;
	struct tm tm_time, *ptm_time;
	unsigned char strbuf[70];
	
	time(&tm);
	ptm_time = localtime(&tm);
	snprintf(strbuf, 64, "%d", (ptm_time->tm_year+ 1900));
	printf("Yr:%s ", strbuf);
	
	snprintf(strbuf, 64, "%d", (ptm_time->tm_mon+ 1));
	printf("Mon:%s ", strbuf);
	
	snprintf(strbuf, 64, "%d", (ptm_time->tm_mday));
	printf("Day:%s ", strbuf);
	
	snprintf(strbuf, 64, "%d", (ptm_time->tm_hour));
	printf("Hr:%s ", strbuf);
	
	snprintf(strbuf, 64, "%d", (ptm_time->tm_min));
	printf("Mn:%s ", strbuf);
	
	snprintf(strbuf, 64, "%d", (ptm_time->tm_sec));
	printf("Sec:%s\n", strbuf);
	
	printWaitStr();
}

int setCurTime()
{
	struct tm tm_time;
	time_t tm;	
	int min, max, num, intVal;
	
	tm_time.tm_isdst = -1;  /* Be sure to recheck dst. */
	
	min = 1902; max = 65535;	
	if ( 0 == getInputInt("Year(1902 ~): ", &num, &min, &max) )
            return 0;		
	tm_time.tm_year = num - 1900;
	
	min = 1; max = 12;
	if (0 == getInputInt("Mon: ", &num, &min, &max))
             return 0;	
	tm_time.tm_mon = num-1;
	
	min = 1; max = 31;
	if (0 == getInputInt("Day: ", &num, &min, &max))
             return 0;	
	tm_time.tm_mday = num;
	
	min = 0; max = 24;
	if (0 == getInputInt("Hour: ", &num, &min, &max))
             return 0;	
	tm_time.tm_hour = num;
	
	min = 0; max = 60;
	if (0 == getInputInt("Minute: ", &num, &min, &max))
             return 0;	
	tm_time.tm_min = num;
	
	min = 0; max = 60;
	if (0 == getInputInt("Second: ", &num, &min, &max))
             return 0;	
	tm_time.tm_sec = num;
	
	tm = mktime(&tm_time);
	if(tm < 0){
		printf("set Time Error(1)\n");		
	}
	if(stime(&tm) < 0){
		printf("set Time Error(2)\n");
	}
	 return 1;
}

/* Return value:
 * 0: fail
 * 1: successful
 */
int SNTPCconfig(){
	unsigned char ValSet;
	unsigned char ValGet;
	unsigned int min,max,sel;
	char msgstr[10];

	mib_get(MIB_NTP_ENABLED, (void *)&ValGet);
	if(ValGet==1)
		strcpy(msgstr,"ON");
	else
		strcpy(msgstr,"OFF");
	printf("SNTP Client: %s\n",msgstr);
	min=1;max=2;
	if(getInputUint("New SNTP Client (1)ON (2)OFF :",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		return 0;
	}

	if (sel==1)
		ValSet = 1;
	else
		ValSet = 0;	// default "off"
	if (!mib_set(MIB_NTP_ENABLED, (void *)&ValSet)) {
		printf("Set SNTP Client error!");
	}

	return 1;
}

const unsigned char *NTPServer[] = {"192.5.41.41", "192.5.41.209", "130.149.17.8", "203.117.180.36"};

const unsigned char *TimeZoneVal[] = { "+0 0",
"+12 1", "+11 1", "+10 1", "+9 1", "+8 1", "+7 1", "+7 2", "+6 1", "+6 2", "+6 3",
"+5 1",  "+5 2",  "+5 3",  "+4 1", "+4 2", "+4 3", "+3 1", "+3 2", "+3 3", "+2 1",
"+1 1",  "+0 1",  "+0 2",  "-1 1", "-1 2", "-1 3", "-1 4", "-1 5", "-1 6", "-2 1",
"-2 2",  "-2 3",  "-2 4",  "-2 5", "-2 6", "-3 1", "-3 2", "-3 3", "-3 4", "-4 1",
"-4 2",  "-4 3",  "-5 1",  "-5 2", "-5 3", "-6 1", "-6 2", "-7 1", "-8 1", "-8 2",
"-8 3",  "-8 4",  "-9 1",  "-9 2", "-9 3", "-9 4", "-9 5", "-10 1","-10 2","-10 3",
"-10 4", "-10 5", "-11 1", "-12 1","-12 2"};

/* Return value:
 * 0: fail
 * 1: successful
 */
int selTimeZone(){	
	unsigned int min,max,sel_list;		
	FILE *fp;
	char *strVal;
	char tmpBuf[15], setstr[15];
	
	min=1;max=65;
	printf("Time Zone List:\n");
	printf("\t(1) (GMT-12:00)Eniwetok, Kwajalein\n");
	printf("\t(2) (GMT-11:00)Midway Island, Samoa\n");
	printf("\t(3) (GMT-10:00)Hawaii\n");
	printf("\t(4) (GMT-09:00)Alaska\n");
	printf("\t(5) (GMT-08:00)Pacific Time (US & Canada); Tijuana\n");
	printf("\t(6) (GMT-07:00)Arizona\n");
	printf("\t(7) (GMT-07:00)Mountain Time (US & Canada)\n");
	printf("\t(8) (GMT-06:00)Central Time (US & Canada)\n");
	printf("\t(9) (GMT-06:00)Mexico City, Tegucigalpa\n");
	printf("\t(10)(GMT-06:00)Saskatchewan\n");
	printf("\t(11)(GMT-05:00)Bogota, Lima, Quito\n");
	printf("\t(12)(GMT-05:00)Eastern Time (US & Canada)\n");
	printf("\t(13)(GMT-05:00)Indiana (East)\n");
	printf("\t(14)(GMT-04:00)Atlantic Time (Canada)\n");
	printf("\t(15)(GMT-04:00)Caracas, La Paz\n");
	printf("\t(16)(GMT-04:00)Santiago\n");
	printf("\t(17)(GMT-03:30)Newfoundland\n");
	printf("\t(18)(GMT-03:00)Brasilia\n");
	printf("\t(19)(GMT-03:00)Buenos Aires, Georgetown\n");
	printf("\t(20)(GMT-02:00)Mid-Atlantic\n");
	printf("\t(21)(GMT-01:00)Azores, Cape Verde Is.\n");
	printf("\t(22)(GMT)Casablanca, Monrovia\n");
	printf("\t(23)(GMT)Greenwich Mean Time: Dublin, Edinburgh, Lisbon, London\n");
	printf("\t(24)(GMT+01:00)Amsterdam, Berlin, Bern, Rome, Stockholm, Vienna\n");
	printf("\t(25)(GMT+01:00)Belgrade, Bratislava, Budapest, Ljubljana, Prague\n");
	printf("\t(26)(GMT+01:00)Barcelona, Madrid\n");
	printf("\t(27)(GMT+01:00)Brussels, Copenhagen, Madrid, Paris, Vilnius\n");
	printf("\t(28)(GMT+01:00)Paris\n");
	printf("\t(29)(GMT+01:00)Sarajevo, Skopje, Sofija, Warsaw, Zagreb\n");
	printf("\t(30)(GMT+02:00)Athens, Istanbul, Minsk\n");
	printf("\t(31)(GMT+02:00)Bucharest\n");
	printf("\t(32)(GMT+02:00)Cairo\n");
	printf("\t(33)(GMT+02:00)Harare, Pretoria\n");
	printf("\t(34)(GMT+02:00)Helsinki, Riga, Tallinn\n");
	printf("\t(35)(GMT+02:00)Jerusalem\n");
	printf("\t(36)(GMT+03:00)Baghdad, Kuwait, Riyadh\n");
	printf("\t(37)(GMT+03:00)Moscow, St. Petersburg, Volgograd\n");
	printf("\t(38)(GMT+03:00)Mairobi\n");
	printf("\t(39)(GMT+03:30)Tehran\n");
	printf("\t(40)(GMT+04:00)Abu Dhabi, Muscat\n");
	printf("\t(41)(GMT+04:00)Baku, Tbilisi\n");
	printf("\t(42)(GMT+04:30)Kabul\n");
	printf("\t(43)(GMT+05:00)Ekaterinburg\n");
	printf("\t(44)(GMT+05:00)Islamabad, Karachi, Tashkent\n");
	printf("\t(45)(GMT+05:30)Bombay, Calcutta, Madras, New Delhi\n");
	printf("\t(46)(GMT+06:00)Astana, Almaty, Dhaka\n");
	printf("\t(47)(GMT+06:00)Colombo\n");
	printf("\t(48)(GMT+07:00)Bangkok, Hanoi, Jakarta\n");
	printf("\t(49)(GMT+08:00)Beijing, Chongqing, Hong Kong, Urumqi\n");
	printf("\t(50)(GMT+08:00)Perth\n");
	printf("\t(51)(GMT+08:00)Singapore\n");
	printf("\t(52)(GMT+08:00)Taipei\n");
	printf("\t(53)(GMT+09:00)Osaka, Sapporo, Tokyo\n");
	printf("\t(54)(GMT+09:00)Seoul\n");
	printf("\t(55)(GMT+09:00)Yakutsk\n");
	printf("\t(56)(GMT+09:30)Adelaide\n");
	printf("\t(57)(GMT+09:30)Darwin\n");
	printf("\t(58)(GMT+10:00)Brisbane\n");
	printf("\t(59)(GMT+10:00)Canberra, Melbourne, Sydney\n");
	printf("\t(60)(GMT+10:00)Guam, Port Moresby\n");
	printf("\t(61)(GMT+10:00)Hobart\n");
	printf("\t(62)(GMT+10:00)Vladivostok\n");
	printf("\t(63)(GMT+11:00)Magadan, Solomon Is., New Caledonia\n");
	printf("\t(64)(GMT+12:00)Auckland, Wllington\n");
	printf("\t(65)(GMT+12:00)Fiji, Kamchatka, Marshall Is.\n");
		
	if(getInputUint(": ",&sel_list,&min,&max)==0){
		printf("Invalid selection!\n");
		return 0;
	}	
	
	strcpy(setstr, TimeZoneVal[sel_list]);	
	if ( mib_set(MIB_NTP_TIMEZONE, (void *)setstr) == 0) {
		printf("Set Time Zone error for CLI!");
		return 0;
	}

	strVal = strstr(setstr, " ");	
	if (strVal != NULL)
		strVal[0] = '\0';	

	snprintf(tmpBuf, 10, "GMT%s", setstr);	
	//setenv("TZ", tmpBuf, 1);
	if ((fp = fopen("/etc/TZ", "w")) != NULL) {
		fprintf(fp, "%s\n", tmpBuf);
		fclose(fp);
	}

	return 1;
}


/* Return value:
 * 0: fail
 * 1: successful
 */
int setSNTPServer(){	
	unsigned int min,max,sel, sel_list;
	char server_id;
	struct in_addr ipAddr ;	
	
	printf("Please select one method to set SNTP Server.\n");
	min=1;max=2;
	if(getInputUint("Method: (1)Select a SNTP Server from List (2) Manual SNTP Server setting: ",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		return 0;
	}
	
	if ( sel == 1 ) {	
		min=1;max=4;
		printf("Time Zone List:\n");
		printf("\t(1)192.5.41.41    - North America\n");
		printf("\t(2)192.5.41.209   - North America\n");
		printf("\t(3)130.149.17.8   - Europe\n");
		printf("\t(4)203.117.180.36 - Asia Pacific\n");		

		if(getInputUint(": ",&sel_list,&min,&max)==0){
			printf("Invalid selection!\n");
			return 0;
		}
		
		server_id = 0;
		mib_set( MIB_NTP_SERVER_ID, (void *)&server_id);
		
		inet_aton(NTPServer[sel_list-1], &ipAddr);
		if ( mib_set(MIB_NTP_SERVER_IP1, (void *)&ipAddr) == 0) {
			printf("Set NTP server from List error!");
			return 0;
		} 
	} else {
		if (0 == getInputIpAddr("New SNTP Server IP address:", &ipAddr))
			return 0;
		server_id = 1;
		mib_set( MIB_NTP_SERVER_ID, (void *)&server_id);
			
		if ( mib_set(MIB_NTP_SERVER_IP2, (void *)&ipAddr) == 0) {
			printf("Set NTP server Manually error!");
			return 0;
		}
	}
	return 1;
}

#if defined(APPLY_CHANGE)
int restartSNTPC()
{
	unsigned char ValGet;
	
	mib_get(MIB_NTP_ENABLED, (void *)&ValGet);
	if(ValGet==1) {
		stopNTP();
		startNTP();
	} else {
		stopNTP();		
	}
}
#endif

void setTimeZone()
{
	int snum, ret;
	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		printf("                           Time Zone                                          \n");
		MSG_LINE;
		printf("You can maintain the system time by synchronizing with a public time server   \n");
		printf("over the Internet  \n");		
		MSG_LINE;
		
		//printf("-------------------------------------------------------------------------\n");
	   	printf("(1) Show Current Time                (2) Set Current Time\n");
   		printf("(3) Select Time Zone                 (4) Enable/Disable SNTP Client\n");
   		printf("(5) Set SNTP server                  (6) Quit\n");
	   	if (!getInputOption(&snum, 1, 6))
   			continue;
		ret = 0;
		switch (snum) {
			case 1:  //(1) Show Current Time
				if (!check_access(SECURITY_SUPERUSER))
					break;
				showCurTime();								
				break;
			case 2: // (2) Set Current Time
				if (!check_access(SECURITY_SUPERUSER))
					break;
				setCurTime();											
				break;
			case 3:	// (3) Select Time Zone			
				ret = selTimeZone();
				break;		
			case 4: // (4) Enable/Disable SNTP Client
				ret = SNTPCconfig();				
				break;
			case 5: // (5) Set SNTP server
				ret = setSNTPServer();
				break;
			case 6:
				return;			
		}
#if defined(APPLY_CHANGE)
		if ( ret )
			restartSNTPC();	
#endif	
	}	
}
#endif

#ifdef DOS_SUPPORT
#define DOSENABLE						0x1
#define DOSSYSFLOODSYN					0x2
#define DOSSYSFLOODFIN					0x4
#define DOSSYSFLOODUDP					0x8
#define DOSSYSFLOODICMP				0x10
#define DOSIPFLOODSYN					0x20
#define DOSIPFLOODFIN					0x40
#define DOSIPFLOODUDP					0x80
#define DOSIPFLOODICMP					0x100
#define DOSTCPUDPPORTSCAN				0x200
#define DOSPORTSCANSENSI				0x800000
#define DOSICMPSMURFENABLED			0x400
#define DOSIPLANDENABLED				0x800
#define DOSIPSPOOFENABLED				0x1000
#define DOSIPTEARDROPENABLED			0x2000
#define DOSPINTOFDEATHENABLED			0x4000
#define DOSTCPSCANENABLED				0x8000
#define DOSTCPSYNWITHDATAENABLED		0x10000
#define DOSUDPBOMBENABLED				0x20000
#define DOSUDPECHOCHARGENENABLED	0x40000
#define DOSSOURCEIPBLOCK				0x400000
void showDoS()
{
	unsigned int mode = 0, uIntValue = 0;

	mib_get(MIB_DOS_ENABLED, (void *)&mode);

	printf("DoS Block\t\t\t: %s\n", (mode & DOSENABLE)?STR_ENABLE: STR_DISABLE);
	mib_get(MIB_DOS_SYSSYN_FLOOD, (void *)&uIntValue);
	printf("Whole System Flood: SYN\t\t: %s\t%d packets/second\n", (mode & DOSSYSFLOODSYN)?STR_ENABLE: STR_DISABLE, uIntValue);
	mib_get(MIB_DOS_SYSFIN_FLOOD, (void *)&uIntValue);
	printf("Whole System Flood: FIN\t\t: %s\t%d packets/second\n", (mode & DOSSYSFLOODFIN)?STR_ENABLE: STR_DISABLE, uIntValue);
	mib_get(MIB_DOS_SYSUDP_FLOOD, (void *)&uIntValue);
	printf("Whole System Flood: UDP\t\t: %s\t%d packets/second\n", (mode & DOSSYSFLOODUDP)?STR_ENABLE: STR_DISABLE, uIntValue);
	mib_get(MIB_DOS_SYSICMP_FLOOD, (void *)&uIntValue);
	printf("Whole System Flood: ICMP\t: %s\t%d packets/second\n", (mode & DOSSYSFLOODICMP)?STR_ENABLE: STR_DISABLE, uIntValue);
	mib_get(MIB_DOS_PIPSYN_FLOOD, (void *)&uIntValue);
	printf("Per-Source IP Flood: SYN\t: %s\t%d packets/second\n", (mode & DOSIPFLOODSYN)?STR_ENABLE: STR_DISABLE, uIntValue);
	mib_get(MIB_DOS_PIPFIN_FLOOD, (void *)&uIntValue);
	printf("Per-Source IP Flood: FIN\t: %s\t%d packets/second\n", (mode & DOSIPFLOODFIN)?STR_ENABLE: STR_DISABLE, uIntValue);
	mib_get(MIB_DOS_PIPUDP_FLOOD, (void *)&uIntValue);
	printf("Per-Source IP Flood: UDP\t: %s\t%d packets/second\n", (mode & DOSIPFLOODUDP)?STR_ENABLE: STR_DISABLE, uIntValue);
	mib_get(MIB_DOS_PIPICMP_FLOOD, (void *)&uIntValue);
	printf("Per-Source IP Flood: ICMP\t: %s\t%d packets/second\n", (mode & DOSIPFLOODICMP)?STR_ENABLE: STR_DISABLE, uIntValue);
	printf("TCP/UDP PortScan\t\t: %s\t%s Sensitivity\n", (mode & DOSTCPUDPPORTSCAN)?STR_ENABLE: STR_DISABLE, (mode & DOSPORTSCANSENSI)?"High": "Low");
	printf("ICMP Smurf\t\t\t: %s\n", (mode & DOSICMPSMURFENABLED)?STR_ENABLE: STR_DISABLE);
	printf("IP Land\t\t\t\t: %s\n", (mode & DOSIPLANDENABLED)?STR_ENABLE: STR_DISABLE);
	printf("IP Spoof\t\t\t: %s\n", (mode & DOSIPSPOOFENABLED)?STR_ENABLE: STR_DISABLE);
	printf("IP TearDrop\t\t\t: %s\n", (mode & DOSIPTEARDROPENABLED)?STR_ENABLE: STR_DISABLE);
	printf("PingOfDeath\t\t\t: %s\n", (mode & DOSPINTOFDEATHENABLED)?STR_ENABLE: STR_DISABLE);
	printf("TCP Scan\t\t\t: %s\n", (mode & DOSTCPSCANENABLED)?STR_ENABLE: STR_DISABLE);
	printf("TCP SynWithData\t\t\t: %s\n", (mode & DOSTCPSYNWITHDATAENABLED)?STR_ENABLE: STR_DISABLE);
	printf("UDP Bomb\t\t\t: %s\n", (mode & DOSUDPBOMBENABLED)?STR_ENABLE: STR_DISABLE);
	printf("UDP EchoChargen\t\t\t: %s\n", (mode & DOSUDPECHOCHARGENENABLED)?STR_ENABLE: STR_DISABLE);
	mib_get(MIB_DOS_BLOCK_TIME, (void *)&uIntValue);
	printf("Source IP Blocking\t\t: %s\t%d Block Interval (second)\n", (mode & DOSSOURCEIPBLOCK)?STR_ENABLE: STR_DISABLE, uIntValue);
}

void changeDoS()
{
	int min, max, sel;
	unsigned int enabled = 0;
	unsigned int floodCount = 0, blockTimer = 0;

	mib_get(MIB_DOS_ENABLED, (void *)&enabled);
	min=1; max=2;	  
	if (0 == getInputUint("DoS Block (1)Disable (2)Enable :", &sel, &min, &max))
		return;
	if (sel == 1) {
		enabled = 0;
		mib_set(MIB_DOS_ENABLED, (void *)&enabled);
		return;
	}
	else if (sel == 2) {
		unsigned int num, floodCount, retVal, floodmin, floodmax;
		unsigned int sysfloodSYNcount, sysfloodFINcount, sysfloodUDPcount, sysfloodICMPcount;
		unsigned int ipfloodSYNcount, ipfloodFINcount, ipfloodUDPcount, ipfloodICMPcount;
		unsigned int blockTimer;

		enabled |= DOSENABLE;

		min=1; max=2;	  
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Whole System Flood: SYN (1)Disable (2)Enable :", &num, &min, &max);
		mib_get(MIB_DOS_SYSSYN_FLOOD, (void *)&floodCount);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSSYSFLOODSYN;
			else if (num == 2) {
				enabled |= DOSSYSFLOODSYN;
				floodmin = 0; floodmax = 65535;	  
				int inputRet = getTypedInputDefault(INPUT_TYPE_UINT,"Flood Count (packets/second) :", &floodCount, &floodmin, &floodmax);
				if (inputRet == 0) {
					printf("Invalid selection!\n");
					printWaitStr();
					return;
				}
			}
		}
		sysfloodSYNcount = floodCount;
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Whole System Flood: FIN (1)Disable (2)Enable :", &num, &min, &max);
		mib_get(MIB_DOS_SYSFIN_FLOOD, (void *)&floodCount);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSSYSFLOODFIN;
			else if (num == 2) {
				enabled |= DOSSYSFLOODFIN;
				floodmin = 0; floodmax = 65535;   
				int inputRet = getTypedInputDefault(INPUT_TYPE_UINT,"Flood Count (packets/second) :", &floodCount, &floodmin, &floodmax);
				if (inputRet == 0) {
					printf("Invalid selection!\n");
					printWaitStr();
					return;
				}
			}
		}
		sysfloodFINcount = floodCount;
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Whole System Flood: UDP (1)Disable (2)Enable :", &num, &min, &max);
		mib_get(MIB_DOS_SYSUDP_FLOOD, (void *)&floodCount);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSSYSFLOODUDP;
			else if (num == 2) {
				enabled |= DOSSYSFLOODUDP;
				floodmin = 0; floodmax = 65535;   
				int inputRet = getTypedInputDefault(INPUT_TYPE_UINT,"Flood Count (packets/second) :", &floodCount, &floodmin, &floodmax);
				if (inputRet == 0) {
					printf("Invalid selection!\n");
					printWaitStr();
					return;
				}
			}
		}
		sysfloodUDPcount = floodCount;
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Whole System Flood: ICMP (1)Disable (2)Enable :", &num, &min, &max);
		mib_get(MIB_DOS_SYSICMP_FLOOD, (void *)&floodCount);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSSYSFLOODICMP;
			else if (num == 2) {
				enabled |= DOSSYSFLOODICMP;
				floodmin = 0; floodmax = 65535;   
				int inputRet = getTypedInputDefault(INPUT_TYPE_UINT,"Flood Count (packets/second) :", &floodCount, &floodmin, &floodmax);
				if (inputRet == 0) {
					printf("Invalid selection!\n");
					printWaitStr();
					return;
				}
			}
		}
		sysfloodICMPcount = floodCount;
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Per-Source IP Flood: SYN (1)Disable (2)Enable :", &num, &min, &max);
		mib_get(MIB_DOS_PIPSYN_FLOOD, (void *)&floodCount);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSIPFLOODSYN;
			else if (num == 2) {
				enabled |= DOSIPFLOODSYN;
				floodmin = 0; floodmax = 65535;   
				int inputRet = getTypedInputDefault(INPUT_TYPE_UINT,"Flood Count (packets/second) :", &floodCount, &floodmin, &floodmax);
				if (inputRet == 0) {
					printf("Invalid selection!\n");
					printWaitStr();
					return;
				}
			}
		}
		ipfloodSYNcount = floodCount;
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Per-Source IP Flood: FIN (1)Disable (2)Enable :", &num, &min, &max);
		mib_get(MIB_DOS_PIPFIN_FLOOD, (void *)&floodCount);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSIPFLOODFIN;
			else if (num == 2) {
				enabled |= DOSIPFLOODFIN;
				floodmin = 0; floodmax = 65535;   
				int inputRet = getTypedInputDefault(INPUT_TYPE_UINT,"Flood Count (packets/second) :", &floodCount, &floodmin, &floodmax);
				if (inputRet == 0) {
					printf("Invalid selection!\n");
					printWaitStr();
					return;
				}
			}
		}
		ipfloodFINcount = floodCount;
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Per-Source IP Flood: UDP (1)Disable (2)Enable :", &num, &min, &max);
		mib_get(MIB_DOS_PIPUDP_FLOOD, (void *)&floodCount);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSIPFLOODUDP;
			else if (num == 2) {
				enabled |= DOSIPFLOODUDP;
				floodmin = 0; floodmax = 65535;   
				int inputRet = getTypedInputDefault(INPUT_TYPE_UINT,"Flood Count (packets/second) :", &floodCount, &floodmin, &floodmax);
				if (inputRet == 0) {
					printf("Invalid selection!\n");
					printWaitStr();
					return;
				}
			}
		}
		ipfloodUDPcount = floodCount;
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Per-Source IP Flood: ICMP (1)Disable (2)Enable :", &num, &min, &max);
		mib_get(MIB_DOS_PIPICMP_FLOOD, (void *)&floodCount);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSIPFLOODICMP;
			else if (num == 2) {
				enabled |= DOSIPFLOODICMP;
				floodmin = 0; floodmax = 65535;   
				int inputRet = getTypedInputDefault(INPUT_TYPE_UINT,"Flood Count (packets/second) :", &floodCount, &floodmin, &floodmax);
				if (inputRet == 0) {
					printf("Invalid selection!\n");
					printWaitStr();
					return;
				}
			}
		}
		ipfloodICMPcount = floodCount;
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "TCP/UDP PortScan (1)Disable (2)Enable :", &num, &min, &max);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSTCPUDPPORTSCAN;
			else if (num == 2) {
				enabled |= DOSTCPUDPPORTSCAN;
				int inputRet = getTypedInputDefault(INPUT_TYPE_UINT,"Sensitivity: (1)Low (2)High :", &floodCount, &min, &max);
				if (inputRet == 0) {
					printf("Invalid selection!\n");
					printWaitStr();
					return;
				} else if (inputRet == -2)
					enabled &= ~DOSPORTSCANSENSI;
				else {
					if (floodCount == 1)
						enabled &= ~DOSPORTSCANSENSI;
					else
						enabled |= DOSPORTSCANSENSI;
				}
			}
		}
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "ICMP Smurf (1)Disable (2)Enable :", &num, &min, &max);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSICMPSMURFENABLED;
			else
				enabled |= DOSICMPSMURFENABLED;
		}
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "IP Land (1)Disable (2)Enable :", &num, &min, &max);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSIPLANDENABLED;
			else
				enabled |= DOSIPLANDENABLED;
		}
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "IP Spoof (1)Disable (2)Enable :", &num, &min, &max);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSIPSPOOFENABLED;
			else
				enabled |= DOSIPSPOOFENABLED;
		}
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "IP TearDrop (1)Disable (2)Enable :", &num, &min, &max);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSIPTEARDROPENABLED;
			else
				enabled |= DOSIPTEARDROPENABLED;
		}
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "PingOfDeath (1)Disable (2)Enable :", &num, &min, &max);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSPINTOFDEATHENABLED;
			else
				enabled |= DOSPINTOFDEATHENABLED;
		}
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "TCP Scan (1)Disable (2)Enable :", &num, &min, &max);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSTCPSCANENABLED;
			else
				enabled |= DOSTCPSCANENABLED;
		}
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "TCP SynWithData (1)Disable (2)Enable :", &num, &min, &max);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSTCPSYNWITHDATAENABLED;
			else
				enabled |= DOSTCPSYNWITHDATAENABLED;
		}
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "UDP Bomb (1)Disable (2)Enable :", &num, &min, &max);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSUDPBOMBENABLED;
			else
				enabled |= DOSUDPBOMBENABLED;
		}
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "UDP EchoChargen (1)Disable (2)Enable :", &num, &min, &max);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSUDPECHOCHARGENENABLED;
			else
				enabled |= DOSUDPECHOCHARGENENABLED;
		}
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Source IP Blocking: (1)Disable (2)Enable :", &num, &min, &max);
		mib_get(MIB_DOS_BLOCK_TIME, (void *)&blockTimer);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSSOURCEIPBLOCK;
			else if (num == 2) {
				enabled |= DOSSOURCEIPBLOCK;
				floodmin = 0; floodmax = 65535;   
				int inputRet = getTypedInputDefault(INPUT_TYPE_UINT,"Block Interval (second) :", &blockTimer, &floodmin, &floodmax);
				if (inputRet == 0) {
					printf("Invalid selection!\n");
					printWaitStr();
					return;
				}
			}
		}
		mib_set(MIB_DOS_ENABLED, (void *)&enabled);
		mib_set(MIB_DOS_SYSSYN_FLOOD, (void *)&sysfloodSYNcount);
		mib_set(MIB_DOS_SYSFIN_FLOOD, (void *)&sysfloodFINcount);
		mib_set(MIB_DOS_SYSUDP_FLOOD, (void *)&sysfloodUDPcount);
		mib_set(MIB_DOS_SYSICMP_FLOOD, (void *)&sysfloodICMPcount);
		mib_set(MIB_DOS_PIPSYN_FLOOD, (void *)&ipfloodSYNcount);
		mib_set(MIB_DOS_PIPFIN_FLOOD, (void *)&ipfloodFINcount);
		mib_set(MIB_DOS_PIPUDP_FLOOD, (void *)&ipfloodUDPcount);
		mib_set(MIB_DOS_PIPICMP_FLOOD, (void *)&ipfloodICMPcount);
		mib_set(MIB_DOS_BLOCK_TIME, (void *)&blockTimer);
	}
}

void setDoS()
{
	int min, max, sel;
	while (1) {
		CLEAR;
		printf("\n");
		MSG_LINE;
		printf("                      DoS Setting             \n");
		MSG_LINE;
      		printf("DoS(\"denial-of-service\") attack which is launched by hacker aims to prevent \n");
      		printf("legal user from taking normal services. In this page you can configure to \n");
		printf("prevent some kinds of DOS attack.\n");
		MSG_LINE;
		showDoS();
		min=1; max=2;
		if (getInputUint("(1) Set DoS Block (2) Quit:", &sel, &min, &max)==0) {
			printf("Invalid selection!\n");
			return;
		}
		switch (sel) {
			case 1:
				if (!check_access(SECURITY_SUPERUSER))
					break;
				changeDoS();
#if defined(APPLY_CHANGE)
				setup_dos_protection();
#endif
				break;
			case 2:
				return;
		}
	}
}
#endif

void setAdmin()
{
	int snum;
	
	while (1)
	{
		CLEAR;
		printf("\n");
		printf("------------------------------------------------------------\n");
		printf("                 (8) Admin Menu                             \n");
		printf("------------------------------------------------------------\n");
		printf("(1) Commit/Reboot                (2) Tftp\n");
#ifdef ACCOUNT_CONFIG
		printf("(3) Account Configuration        (4) ACL Configuration\n");
#else
		printf("(3) Password                     (4) ACL Configuration\n");      
#endif
		printf("(5) Tr-069                       (6) Ftp\n");
		printf("(7) System Log                   (8) Time Zone\n");
		printf("(9) DOS                          (10) Quit\n");
		
		//if (!getInputOption( &snum, 1, 5))
		if (!getInputOption( &snum, 1, 10))
			continue;
		
		switch( snum)
		{
		case 1://(1) Commit/Reboot
			setCommitReboot();
			break;
		// kaotets
		case 2://(2) Tftp
			#ifdef CONFIG_USER_BUSYBOX_TFTP
			setTftp();
			#else
			printf("Not supported\n");
			printWaitStr();
			#endif
			break;
		case 3://(//(2) Password
			setPassword();
			break;
		case 4://(3) ACL Config        
#ifdef IP_ACL
			setACL();
#endif
			break;
		case 5:
			#if CONFIG_USER_CWMP_TR069
			setTR069();
			#else
			printf("Not supported\n");
			printWaitStr();
			#endif
			break;
		case 6:  // (6) Ftp
			#ifdef CONFIG_USER_FTP_FTP_FTP
			setFtp();
			#else
			printf("Not supported\n");
			printWaitStr();
			#endif
			break;
		case 7:  // (7) System Log
#ifdef SYSLOG_SUPPORT
			setSystemLog();
#else
			printf("Not supported\n");
#endif
			break;
		case 8:  // (8) Time Zone
#ifdef TIME_ZONE
			setTimeZone();
#else
			printf("Not supported\n");
#endif
			break;
		case 9:  // (9) DoS
#ifdef DOS_SUPPORT
			setDoS();
#else
			printf("Not supported\n");
			printWaitStr();
#endif
			break;
		case 10:
			return;
		default:
			break;
		}//end switch, (8) Admin Menu
	}//end while, (8) Admin Menu
}
/*************************************************************************************************/
#define _PATH_PROCNET_DEV	"/proc/net/dev"
static char *get_name(char *name, char *p)
{
    while (isspace(*p))
	p++;
    while (*p) {
	if (isspace(*p))
	    break;
	if (*p == ':') {	/* could be an alias */
	    char *dot = p, *dotname = name;
	    *name++ = *p++;
	    while (isdigit(*p))
		*name++ = *p++;
	    if (*p != ':') {	/* it wasn't, backup */
		p = dot;
		name = dotname;
	    }
	    if (*p == '\0')
		return NULL;
	    p++;
	    break;
	}
	*name++ = *p++;
    }
    *name++ = '\0';
    return p;
}
void setStatistics()
{
    
    
	FILE *fh;
#ifdef EMBED
	int skfd, i;
	struct atmif_sioc mysio;
	struct SAR_IOCTL_CFG cfg;
	struct ch_stat stat;
#endif
	char buf[512];
	unsigned long rx_pkt, rx_err, rx_drop, tx_pkt, tx_err, tx_drop;
	struct sysinfo info;

	CLEAR;
	printf("\n");
	MSG_LINE;
	//printf("-------------------------------------------------------------------------\n");
	printf("                                Statistics                               \n");
	MSG_LINE;
	//printf("-------------------------------------------------------------------------\n");
	printf("This page shows the packet statistics for transmission and reception     \n");
	printf("regarding to network interface.                                          \n");
	MSG_LINE;
	//printf("-------------------------------------------------------------------------\n");
	printf("\nItf\tRx_pkt\tRx_err\tRx_drop\tTx_pkt\tTx_err\tTx_drop\n");
	MSG_LINE;
	//printf("-------------------------------------------------------------------------\n");
	
	// Ethernet statistics
	fh = fopen(_PATH_PROCNET_DEV, "r");
	if (!fh) {
		//fprintf(stderr, "Warning: cannot open %s (%s).\n",
		//	_PATH_PROCNET_DEV, strerror(errno));
		return;
	}
	fgets(buf, sizeof buf, fh);	/* eat line */
	fgets(buf, sizeof buf, fh);
	while (fgets(buf, sizeof buf, fh)) {
		char *s, name[IFNAMSIZ];
		s = get_name(name, buf);    
		if(    (!strcmp("eth0", name)) 
#ifdef CONFIG_USB_ETH
		    || (!strcmp( USBETHIF, name))
#endif //CONFIG_USB_ETH
		) {
			sscanf(s,
			"%*lu %lu %lu %lu %*lu %*lu %*lu %*lu %*lu %lu %lu %lu %*lu %*lu %*lu %*lu",
			&rx_pkt, &rx_err, &rx_drop, &tx_pkt, &tx_err, &tx_drop);
   			printf("%s\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\n", name, rx_pkt, rx_err, rx_drop, tx_pkt, tx_err, tx_drop);
		}
	}	
	fclose(fh);

	
#ifdef EMBED
	// pvc statistics
	if((skfd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0){
		perror("socket open error");
		return;
	}
	
	mysio.number = 0;
	
	for (i=0; i < MAX_VC_NUM; i++)
	{
		cfg.ch_no = i;
		mysio.arg = (void *)&cfg;
		if(ioctl(skfd, ATM_SAR_GETSTAT, &mysio)<0)
		{
			(void)close(skfd);
			return;
		}
		
		if (cfg.created == 0)
			continue;
		
      printf("%u/%u\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\n", 
      cfg.vpi, cfg.vci, cfg.stat.rx_pkt_cnt, cfg.stat.rx_pkt_fail,
		cfg.stat.rx_crc_error, cfg.stat.tx_desc_ok_cnt,
		cfg.stat.tx_pkt_fail_cnt, cfg.stat.send_desc_lack);
	}
	(void)close(skfd);
#endif

	MSG_LINE;
	//printf("-------------------------------------------------------------------------\n");
	/*
	sysinfo(&info);
	info.mem_unit*=1024;
	info.totalram/=info.mem_unit;
	info.freeram/=info.mem_unit;
	printf("Memory Usage:\n");
	printf("Total: %d kB\tFree: %d kB\n",info.totalram, info.freeram);
	*/
	printWaitStr();
}


void adslStatistic()
{
	CLEAR;
	printf("\n");
	MSG_LINE;
	//printf("-------------------------------------------------------------------------\n");
	printf("                        Statistics -- ADSL                           \n");
	MSG_LINE;
	//printf("-------------------------------------------------------------------------\n");
	printf("This page shows the current ADSL statistics.\n");
	MSG_LINE;
	//printf("-------------------------------------------------------------------------\n");	
	
	getAdslInfo(ADSL_GET_MODE, strbuf, 256);
	printf("Mode\t\t\t: %s\n", strbuf);	
	getAdslInfo(ADSL_GET_LATENCY, strbuf, 256);
	printf("Latency\t\t\t: %s\n", strbuf);		
	getAdslInfo(ADSL_GET_TRELLIS, strbuf, 256);
	printf("Trellis Coding\t\t: %s\n", strbuf);	
	getAdslInfo(ADSL_GET_STATE, strbuf, 256);
	printf("Status\t\t\t: %s\n", strbuf);	
	getAdslInfo(ADSL_GET_POWER_LEVEL, strbuf, 256);
	printf("Power Level\t\t: %s\n", strbuf);
	printf("\n");
	
	printf("\t\t\t\t\t\tDownstream");
	printf("\tUptream\n");
	getAdslInfo(ADSL_GET_SNR_DS, strbuf, 256);
	printf("SNR Margin (dB)\t\t\t\t\t: %s", strbuf);	
	getAdslInfo(ADSL_GET_SNR_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);		
	
	getAdslInfo(ADSL_GET_LPATT_DS, strbuf, 256);
	printf("Attenuation (dB)\t\t\t\t: %s", strbuf);	
	getAdslInfo(ADSL_GET_LPATT_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);
	
	getAdslInfo(ADSL_GET_POWER_DS, strbuf, 256);
	printf("Power (dBm)\t\t\t\t\t: %s", strbuf);	
	getAdslInfo(ADSL_GET_POWER_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);	
	
	getAdslInfo(ADSL_GET_ATTRATE_DS, strbuf, 256);
	printf("Attainable Rate (Kbps)\t\t\t\t: %s", strbuf);	
	getAdslInfo(ADSL_GET_ATTRATE_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);
	
	getAdslInfo(ADSL_GET_RATE_DS, strbuf, 256);
	printf("Rate (Kbps)\t\t\t\t\t: %s", strbuf);	
	getAdslInfo(ADSL_GET_RATE_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);
	
	getAdslInfo(ADSL_GET_K_DS, strbuf, 256);
	printf("K (number of bytes in DMT frame)\t\t: %s", strbuf);	
	getAdslInfo(ADSL_GET_K_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);
	
	getAdslInfo(ADSL_GET_R_DS, strbuf, 256);
	printf("R (number of check bytes in RS code word)\t: %s", strbuf);	
	getAdslInfo(ADSL_GET_R_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);
	
	getAdslInfo(ADSL_GET_S_DS, strbuf, 256);
	printf("S (RS code word size in DMT frame)\t\t: %s", strbuf);	
	getAdslInfo(ADSL_GET_S_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);
	
	getAdslInfo(ADSL_GET_D_DS, strbuf, 256);
	printf("D (interleaver depth)\t\t\t\t: %s", strbuf);	
	getAdslInfo(ADSL_GET_D_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);
	
	getAdslInfo(ADSL_GET_DELAY_DS, strbuf, 256);
	printf("Delay (msec)\t\t\t\t\t: %s", strbuf);	
	getAdslInfo(ADSL_GET_DELAY_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);
	
	getAdslInfo(ADSL_GET_FEC_DS, strbuf, 256);
	printf("FEC\t\t\t\t\t\t: %s", strbuf);	
	getAdslInfo(ADSL_GET_FEC_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);
	
	getAdslInfo(ADSL_GET_CRC_DS, strbuf, 256);
	printf("CRC\t\t\t\t\t\t: %s", strbuf);	
	getAdslInfo(ADSL_GET_CRC_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);
	
	getAdslInfo(ADSL_GET_ES_DS, strbuf, 256);
	printf("Total ES\t\t\t\t\t: %s", strbuf);	
	getAdslInfo(ADSL_GET_ES_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);
	
	getAdslInfo(ADSL_GET_SES_DS, strbuf, 256);
	printf("Total SES\t\t\t\t\t: %s", strbuf);	
	getAdslInfo(ADSL_GET_SES_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);
	
	getAdslInfo(ADSL_GET_UAS_DS, strbuf, 256);
	printf("Total UAS\t\t\t\t\t: %s", strbuf);	
	getAdslInfo(ADSL_GET_UAS_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);
	
	printf("\n");
	printWaitStr();
}	


void setStatistics_start()
{
   int snum;

   while (1)
   {
	CLEAR;
      printf("\n");
      printf("-----------------------------------------------------------\n");
      printf("                (9) Statistic Menu                         \n");
      printf("-----------------------------------------------------------\n");
      printf("(1) Interface                    (2) ADSL\n");      
      printf("(3) Quit\n");
      getInputOption( &snum, 1, 3);
      switch( snum)
      {
      case 1://(1) Interface
         setStatistics();
         break;
      case 2://(2) ADSL               
         adslStatistic();         
         break;
      case 3://(3) Quit
         return;
      default:
         printf("!! Invalid Selection !!\n");
      }//end switch, (9) Statistic Menu
   }//end while, (9) Statistic Menu	
}

/*************************************************************************************************/

void leave(void)
{
	_exit(exstat);
	/* NOTREACHED */
}

#ifdef EMBED
#define CLI_RUNFILE	"/var/run/cli.pid"
static void log_pid()
{
	FILE *f;
	pid_t pid;
	char *pidfile = CLI_RUNFILE;

	pid = getpid();
	if((f = fopen(pidfile, "w")) == NULL)
		return;
	fprintf(f, "%d\n", pid);
	fclose(f);
}
#endif

// Kaohj
static struct passwd *getrealpass(const char *user) {
	struct passwd *pwp;
	
	pwp = getpwnam(user);
	if (pwp == NULL)
		return NULL;
	return pwp;
}

static void sigexit(int dummy)
{
	if (!console_flag)
		unlink(CLI_RUNFILE);
	exit(0);
}

int main (int argc, char *argv[])
{
	int snum, auth_denied=1; //auth_denied=1: Login Incorrect, auth_denied=0: Login Success
	int flag, login=0;
	unsigned char uname[10], *upasswd;
	struct passwd *pwp;
	char loginUser[MAX_NAME_LEN];
	
	// start save/reboot thread
	sem_init(&semSave, 0, 0);
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, sigexit);
	signal(SIGHUP, sigexit);
	startSaved();

	// check the login option
	// Usage: cli -l -c -u loginName
	while ((flag = getopt(argc, argv, "lcu:")) != EOF) {
		switch (flag) {
			case 'l':
				login = 1;
				break;
			case 'c':
				console_flag = 1;
				break;
			case 'u':
				if (optarg == NULL)
					break;
				strncpy(loginUser, optarg, MAX_NAME_LEN);
				break;
			default:
				break;
		}
	}
	
	if (!console_flag)
#ifdef EMBED
		log_pid();
#endif
	// Kaohj, get the login priviledge
	pwp = getrealpass(loginUser);
	//printf("uid=%d\n", pwp->pw_uid);
	if (pwp && pwp->pw_uid == 0)
		loginLevel = SECURITY_ROOT;
	else
		loginLevel = SECURITY_USER;
	
	login_flag = login;
	while (1)
	{
		// Jenny, CLI user/passwd
		if (login && auth_denied) {
			uname[0] = '\0';
			while (strlen(uname) == 0) {
				if (0 == getInputString("User: ", uname, sizeof(uname)))
					printf("!! Input User Name !!\n");
			}
				
			upasswd = getpass("Password: ");
			if ((auth_denied = auth_cli(uname, upasswd)) != 0)
			{
				printf("Login Incorrect!\n\n");
				auth_denied = 1;
			}
		}
		else
		{
			CLEAR;
			printf("\n");
			MENU_LINE;
			printf("                         ADSL Main Menu                     \n");
			MENU_LINE;
			printf("(1)  Status                     (2)  LAN Interface\n");
			printf("(3)  Wireless                   (4)  WAN Interface\n");
			printf("(5)  Services                   (6)  Advance\n");
			printf("(7)  Diagnostic                 (8)  Admin\n");
			printf("(9)  Statistics                 (10) Exit\n");
			printf("(11) Reboot\n");
			snum = -1;
			getInputOption( &snum, 0, 11);
			if (exstat==0)
				leave();
			switch( snum)
			{
				case 0:// shell
					printf("\nShell command mode\nEnter \"exit\" to exit shell...\n");
					va_cmd("/bin/sh", 0, 1);
					break;
				case 1://(1) Status
					showStatus();
					break;
				case 2://(2) LAN Interface
					setLANInterface();
					break;
				case 3://(3) Wireless
#ifdef WLAN_SUPPORT
					setWireless();
#else
					printf("Not supported\n");
					printWaitStr();
#endif
					break;
				case 4://(4) WAN Interface
					setWANInterface();
					break;
				case 5://(5) Services
					setServices();
					break;
				case 6://(6) Advance
					setAdvance();
					break;
				case 7://(7) Diagnostic
					setDiagnostic();
					break;
				case 8://(8) Admin
					setAdmin();
					break;
				case 9://(9) Statistics
					setStatistics_start();
					break;
				case 10://(10) Exit
					sigexit(0);
				case 11://(11) Reboot
					setCommitReboot();
					break;
#ifdef EMBED
					if (!console_flag)
						unlink(CLI_RUNFILE);
#endif
					return 0;
					break;
				default:
					break;
			}//end switch, Realtek ADSL Main Menu
		} // Jenny, end if auth
	}//end While, Realtek ADSL Main Menu
	return 0;
}
