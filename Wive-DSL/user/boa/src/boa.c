/* vi:set tabstop=2 cindent shiftwidth=2: */
/*
 *  Boa, an http server
 *  Copyright (C) 1995 Paul Phillips <psp@well.com>
 *  Some changes Copyright (C) 1996 Charles F. Randall <crandall@goldsys.com>
 *  Some changes Copyright (C) 1996 Larry Doolittle <ldoolitt@jlab.org>
 *  Some changes Copyright (C) 1996,97,98 Jon Nelson <nels0988@tc.umn.edu>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 1, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/* boa: boa.c */
// run BOA as console (menu-driven CLI)
//#define CONSOLE_BOA

#include "asp_page.h"
#include "boa.h"
#include <grp.h>
#include "syslog.h"
#include <sys/param.h>
#include <signal.h>
#ifdef SERVER_SSL
#ifdef USES_MATRIX_SSL
#include <sslSocket.h>
#include <pthread.h>
#else 
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <pthread.h>
#endif /*USES_MATRIX_SSL*/
#endif /*SERVER_SSL*/

// Added by Mason Yu(2 level)
#include <stdio.h>
#include "md5.h"
#include "./LINUX/mib.h"
#include "./LINUX/utility.h"
// Added by ql
#include "./LINUX/mib_reserve.h"

#ifdef CONFIG_USER_DDNS
#include "./LINUX/adslif.h"
#endif

//#ifdef MSG_WLAN_RESTART_SUPPORT
#include "msgq.h"
//#endif

#ifdef PARENTAL_CTRL
#include <time.h>
#endif
//add by ramen to use struct PPPOE_DRV_CTRL for save_pppoe_sessionid()
#include "../../spppd/pppoe.h"
//#define MAX_NAME_LEN		30
//added by xl_yue
#ifdef ACCOUNT_LOGIN_CONTROL
#define	LOGIN_WAIT_TIME	60   //about 1 minute
#define	ACCOUNT_TIMEOUT		300	//about 5 minutes
#endif

#define        LOG_PID         0x01
int server_s;					/* boa socket */

static int this_pid;

#ifdef SERVER_SSL
#define SSL_KEYF "/etc/ssl_key.pem"
#define SSL_CERTF "/etc/ssl_cert.pem"
	int server_ssl;				/*ssl socket */
	int do_ssl = 1;					/*We want to actually perform all of the ssl stuff.*/
	int do_sock = 1;				/*We may not want to actually connect to normal sockets*/
#ifdef USES_MATRIX_SSL
	sslConn_t		*mtrx_cp = NULL;
	sslKeys_t			*mtrx_keys;
#else
	SSL_CTX *ctx;				/*SSL context information*/
	SSL_METHOD *meth;			/*SSL method information*/
#endif	
	int ssl_server_port = 443;		/*The port that the server should listen on*/
	/*Note that per socket ssl information is stored in */
#ifdef INET6
	struct sockaddr_in6 server_sockaddr;		/* boa ssl socket address */
#else
	struct sockaddr_in ssl_server_sockaddr;		/* boa ssl socket address */
#endif/*INET6*/
extern int InitSSLStuff(void);
extern void get_ssl_request(void);
#endif /*SERVER_SSL*/

int backlog = SO_MAXCONN;
#ifdef INET6
struct sockaddr_in6 server_sockaddr;		/* boa socket address */
#else
struct sockaddr_in server_sockaddr;		/* boa socket address */
#endif

struct timeval req_timeout;		/* timeval for select */

extern char *optarg;			/* getopt */

fd_set block_read_fdset;
fd_set block_write_fdset;

int sighup_flag = 0;			/* 1 => signal has happened, needs attention */
int sigchld_flag = 0;			/* 1 => signal has happened, needs attention */
int lame_duck_mode = 0;

time_t time_counter = 0;

int sock_opt = 1;
int do_fork = 1;

static int max_fd = 0;

// Added by Mason Yu
char suName[MAX_NAME_LEN];
char usName[MAX_NAME_LEN];

//add by xl_yue
#ifdef USE_LOGINWEB_OF_SERVER

#ifdef ONE_USER_LIMITED
struct account_status suStatus;
struct account_status usStatus;
#endif

struct user_info * user_login_list = NULL;

#ifdef LOGIN_ERR_TIMES_LIMITED
struct errlogin_entry * errlogin_list = NULL;
#endif

#endif

//add by xl_yue
#ifdef ACCOUNT_LOGIN_CONTROL
struct account_info su_account;
struct account_info us_account;
struct errlogin_entry * errlogin_list = NULL;
#endif

// Mason Yu
#ifdef WEB_REDIRECT_BY_MAC
extern struct callout landingPage_ch;
#endif

//add by xl_yue
#ifdef USE_LOGINWEB_OF_SERVER
struct user_info * search_login_list(request * req)
{
	struct user_info * pUser_info;
	for(pUser_info = user_login_list;pUser_info;pUser_info = pUser_info->next){
		if(!strcmp(req->remote_ip_addr,pUser_info->remote_ip_addr))
			break;
	}
	return pUser_info;
}

void ulist_free_login_entry(struct user_info * bUser_info,struct user_info * pUser_info)
{
	if(pUser_info == user_login_list)
		user_login_list = pUser_info->next;
	else
		bUser_info->next = pUser_info->next;

	if(pUser_info->directory)
		free(pUser_info->directory);

#ifdef ONE_USER_LIMITED
	if(pUser_info->paccount)
		pUser_info->paccount->busy = 0;
#endif

	free(pUser_info);
}

int free_from_login_list(request * req)
{
	struct user_info * pUser_info;
	struct user_info * bUser_info = NULL;

	for(pUser_info = user_login_list;pUser_info;bUser_info = pUser_info,pUser_info = pUser_info->next){
		if(!strcmp(pUser_info->remote_ip_addr, req->remote_ip_addr))
			break;
	}

	if(!pUser_info)
		return 0;

	ulist_free_login_entry(bUser_info,pUser_info);

	return 1;
}

#ifdef LOGIN_ERR_TIMES_LIMITED
//added by xl_yue
struct errlogin_entry * search_errlog_list(request * req)
{
	struct errlogin_entry * perrlog;

	for(perrlog = errlogin_list;perrlog;perrlog = perrlog->next){
		if(!strcmp(perrlog->remote_ip_addr, req->remote_ip_addr))
			break;
	}

	return perrlog;
}
void ulist_free_errlog_entry(struct errlogin_entry * berrlog,struct errlogin_entry * perrlog)
{
	//unlist and free
	if(perrlog == errlogin_list)
		errlogin_list = perrlog->next;
	else
		berrlog->next = perrlog->next;

	free(perrlog);
}

//added by xl_yue
int free_from_errlog_list(request * req)
{
	struct errlogin_entry * perrlog;
	struct errlogin_entry * berrlog = NULL;

	for(perrlog = errlogin_list;perrlog;berrlog = perrlog,perrlog = perrlog->next){
		if(!strcmp(perrlog->remote_ip_addr, req->remote_ip_addr))
			break;
	}

	if(!perrlog)
		return 0;

	ulist_free_errlog_entry(berrlog,perrlog);
	return 1;
}
#endif
#endif

#ifdef EMBED
static void log_pid()
{
	FILE *f;
	pid_t pid;
	char *pidfile = BOA_RUNFILE;

	pid = getpid();
	this_pid = pid;
	if((f = fopen(pidfile, "w")) == NULL)
		return;
	fprintf(f, "%d\n", pid);
	fclose(f);
}
#endif



//jim luo
//#if 0
#ifdef CONFIG_CTC_E8_CLIENT_LIMIT
// int maxPCClients=1;//MAXPCCLIENTS;
// int maxCameraClients=1;//MAXCAMERACLIENTS;
#define CLIENTSMONITOR  "/proc/ClientsMonitor"
 struct itimerval read_proc_interval;
//struct CTC_Clients accepted_PC_Clients[MAXPCCLIENTS];
//struct CTC_Clients accepted_Camera_Clients[MAXCAMERACLIENTS];
static void read_proc_handler(int dummy)
{
	FILE *fd;
	int totallen;
	int parsedlen=0;
	int i;
	char buffer[1024];
	char *cur_ptr=buffer;
	char *str_end=buffer;
	int str_found;
	char cmd_str[256];
	//printf("read proc \n");
	fd=fopen(CLIENTSMONITOR, "r");
	if(fd==NULL)
	{
		printf("file %s open failed\n", CLIENTSMONITOR);
		return -1;
	}
	memset(buffer, 0, sizeof(buffer));
	totallen=fread(buffer, 1, 1024, fd);
	if(totallen!=0)
		printf("read size=%d, buffer=%s\n", totallen, buffer);
	if(totallen < 0)
		goto err;

	while(1)
	{
		str_found=0;
		memset(cmd_str, 0, sizeof(cmd_str));
		for(i=0; i<totallen-parsedlen; i++)
		{
			if(*str_end=='\n')
			{
				str_found=1;
				break;
			}
			str_end++;
		}
		if(str_found)
		{
			memcpy(cmd_str, cur_ptr, (unsigned)str_end-(unsigned)cur_ptr);
			printf("cmd parsed out: %s", cmd_str);
			system(cmd_str);//execute it....
			parsedlen+=i;
			str_end+=1;
			cur_ptr=str_end;

		}else
		{
			break;
		}
	}
err:
	fclose(fd);
	return;
}
#endif


// Kaohj
// enable menu-driven CLI pthread
//#define MENU_CLI

#ifdef MENU_CLI
/******************************
 *	Start the CLI pthread
 */

void climenu (void);

void startCli()
{
	pthread_t ptCliId;
	if (pthread_create( &ptCliId, 0, &climenu, 0 )!=0)
		return;
}
#endif

/**************************************
 *	Start the Save/reboot pthread
 */
#include <semaphore.h>
extern sem_t semSave;
extern void startSaved();

// Added by Mason Yu
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

	MD5Update(&mc, pass, strlen(pass));
	}
	MD5Final(final, &mc);
	strcpy(encoded_passwd,"$1$");
	base64encode(final, encoded_passwd+3, 16);
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

	/*if ( !mib_get(MIB_USER_NAME, (void *)usName) ) {
		printf("ERROR: Get user name from MIB database failed.\n");
		return;
	}*/ //disable user

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


	#if 0 //def _CWMP_MIB_
	{
		char PasswdMD5[256+1];
		char *tmpStr = PasswdMD5;
		if ((fp = fopen("/var/digest.passwd", "w")) == NULL)
		{
			printf("***** Open file /var/digest.passwd failed !\n");
			return;
		}
		mib_get(CWMP_CONREQ_USERNAME, (void *)tmpStr);
		fprintf(fp, "%s\n", tmpStr);
		mib_get(CWMP_CONREQ_PASSWORD, (void *)tmpStr);
		fprintf(fp, "%s\n", tmpStr);
		fclose(fp);
        }
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
	/*if (!mib_get(MIB_USER_NAME, (void *)usName)) {
		printf("ERROR: Get user name from MIB database failed.\n");
		return;
	}*/ //disable  user
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

	#if 0
	//#ifdef _CWMP_MIB_
	{
		char PasswdMD5[256];
		char *tmpStr = PasswdMD5;
		if ((fp = fopen("/var/digest.passwd", "w")) == NULL)
		{
			printf("***** Open file /var/digest.passwd failed !\n");
			return;
		}	
		mib_get(CWMP_CONREQ_USERNAME, (void *)tmpStr);
		fprintf(fp, "%s\n", tmpStr); 
		mib_get(CWMP_CONREQ_PASSWORD, (void *)tmpStr);
		fprintf(fp, "%s\n", tmpStr);
		fclose(fp);
        }
        #endif
}
#endif
#endif


#ifdef WLAN_WEB_REDIRECT //jiunming,web_redirect
int redir_server_s;
void get_redir_request(void);
int init_redirect_socket(void)
{
	int	ret = -1;
#ifdef INET6
	struct sockaddr_in6 server_sockaddr;		/* socket address */
#else
	struct sockaddr_in server_sockaddr;		/* socket address */
#endif


#ifdef INET6
	if ((redir_server_s = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)) == -1)
#else
	if ((redir_server_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
#endif
		die(NO_CREATE_SOCKET);

	/* server socket is nonblocking */
	if (fcntl(redir_server_s, F_SETFL, NOBLOCK) == -1)
		die(NO_FCNTL);

	if ((setsockopt(redir_server_s, SOL_SOCKET, SO_REUSEADDR, (void *) &sock_opt,
					sizeof(sock_opt))) == -1)
		die(NO_SETSOCKOPT);

	/* internet socket */
	//maybe, binding to 127.0.0.1 is better.
#ifdef INET6
	server_sockaddr.sin6_family = AF_INET6;
	memcpy(&server_sockaddr.sin6_addr,&in6addr_any,sizeof(in6addr_any));
	server_sockaddr.sin6_port = htons(8080);
#else
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_sockaddr.sin_port = htons(8080);
#endif

	if (bind(redir_server_s, (struct sockaddr *) &server_sockaddr,
			 sizeof(server_sockaddr)) == -1)
		die(NO_BIND);

	//printf("\nCreate a web redirect socket at port 8080.");

	if (listen(redir_server_s, 5) == -1)
		die(NO_LISTEN);

	return redir_server_s;
}
#endif


#ifdef WEB_REDIRECT_BY_MAC
int mac_redir_server_s;
int mac_redir_server_port=WEB_REDIR_BY_MAC_PORT;
void get_mac_redir_request(void);
int init_mac_redirect_socket(void)
{
	int	ret = -1;
#ifdef INET6
	struct sockaddr_in6 server_sockaddr;		/* socket address */
#else
	struct sockaddr_in server_sockaddr;		/* socket address */
#endif


#ifdef INET6
	if ((mac_redir_server_s = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)) == -1)
#else
	if ((mac_redir_server_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
#endif
		die(NO_CREATE_SOCKET);

	/* server socket is nonblocking */
	if (fcntl(mac_redir_server_s, F_SETFL, NOBLOCK) == -1)
		die(NO_FCNTL);

	if ((setsockopt(mac_redir_server_s, SOL_SOCKET, SO_REUSEADDR, (void *) &sock_opt,
					sizeof(sock_opt))) == -1)
		die(NO_SETSOCKOPT);

	/* internet socket */
	//maybe, binding to 127.0.0.1 is better.
#ifdef INET6
	server_sockaddr.sin6_family = AF_INET6;
	memcpy(&server_sockaddr.sin6_addr,&in6addr_any,sizeof(in6addr_any));
	server_sockaddr.sin6_port = htons(mac_redir_server_port);
#else
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_sockaddr.sin_port = htons(mac_redir_server_port);
#endif

	if (bind(mac_redir_server_s, (struct sockaddr *) &server_sockaddr,
			 sizeof(server_sockaddr)) == -1)
		die(NO_BIND);

	//printf("\nCreate a web redirect socket at port mac_redir_server_port.");

	if (listen(mac_redir_server_s, 5) == -1)
		die(NO_LISTEN);

	return mac_redir_server_s;
}
#endif

#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
int captiveportal_server_s;
void get_captiveportal_request(void);
int init_captiveportal_socket(void)
{
	int	ret = -1;
#ifdef INET6
	struct sockaddr_in6 server_sockaddr;		/* socket address */
#else
	struct sockaddr_in server_sockaddr;		/* socket address */
#endif

#ifdef INET6
	if ((captiveportal_server_s = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)) == -1)
#else
	if ((captiveportal_server_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
#endif
		die(NO_CREATE_SOCKET);

	/* server socket is nonblocking */
	if (fcntl(captiveportal_server_s, F_SETFL, NOBLOCK) == -1)
		die(NO_FCNTL);

	if ((setsockopt(captiveportal_server_s, SOL_SOCKET, SO_REUSEADDR, (void *) &sock_opt, sizeof(sock_opt))) == -1)
		die(NO_SETSOCKOPT);

	/* internet socket */
	//maybe, binding to 127.0.0.1 is better.
#ifdef INET6
	server_sockaddr.sin6_family = AF_INET6;
	memcpy(&server_sockaddr.sin6_addr, &in6addr_any, sizeof(in6addr_any));
	server_sockaddr.sin6_port = htons(CAPTIVEPORTAL_PORT);
#else
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_sockaddr.sin_port = htons(CAPTIVEPORTAL_PORT);
#endif

	if (bind(captiveportal_server_s, (struct sockaddr *) &server_sockaddr, sizeof(server_sockaddr)) == -1)
		die(NO_BIND);

	//printf("\nCreate a web redirect socket at port %d\n", CAPTIVEPORTAL_PORT);

	if (listen(captiveportal_server_s, 5) == -1)
		die(NO_LISTEN);

	return captiveportal_server_s;
}
#endif

#ifdef QSETUP_WEB_REDIRECT
int qsetup_server_s;
void get_qsetup_request(void);
int init_qsetup_socket(void)
{
	int	ret = -1;
#ifdef INET6
	struct sockaddr_in6 server_sockaddr;		/* socket address */
#else
	struct sockaddr_in server_sockaddr;		/* socket address */
#endif

#ifdef INET6
	if ((qsetup_server_s = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)) == -1)
#else
	if ((qsetup_server_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
#endif
		die(NO_CREATE_SOCKET);

	/* server socket is nonblocking */
	if (fcntl(qsetup_server_s, F_SETFL, NOBLOCK) == -1)
		die(NO_FCNTL);

	if ((setsockopt(qsetup_server_s, SOL_SOCKET, SO_REUSEADDR, (void *) &sock_opt,
					sizeof(sock_opt))) == -1)
		die(NO_SETSOCKOPT);

	/* internet socket */
	//maybe, binding to 127.0.0.1 is better.
#ifdef INET6
	server_sockaddr.sin6_family = AF_INET6;
	memcpy(&server_sockaddr.sin6_addr,&in6addr_any,sizeof(in6addr_any));
	server_sockaddr.sin6_port = htons(2046);
#else
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_sockaddr.sin_port = htons(2046);
#endif

	if (bind(qsetup_server_s, (struct sockaddr *) &server_sockaddr,
			 sizeof(server_sockaddr)) == -1)
		die(NO_BIND);

	if (listen(qsetup_server_s, 5) == -1)
		die(NO_LISTEN);

	return qsetup_server_s;
}
#endif	//QSETUP_WEB_REDIRECT

#if 0
int mib_chain_backup(int id, unsigned char ** ptr)
{
	unsigned int chainSize;
	unsigned int size;
	int index, idx=0;
	unsigned int currentRecord=0;
	MIB_CHAIN_ENTRY_Tp pChain = NULL;

	chainSize = __mib_chain_total(id);
	index = __mib_chain_mib2tbl_id(id);
	if (chainSize > 0)
	{
		size += chainSize * mib_chain_record_table[index].per_record_size;
	}

	*ptr = (unsigned char *)malloc(chainSize);
	pChain = mib_chain_record_table[index].pChainEntry;
	while(pChain != NULL)
	{
		memcpy(*ptr+idx, pChain->pValue, mib_chain_record_table[index].per_record_size);
		pChain = pChain->pNext;
		idx += mib_chain_record_table[index].per_record_size;
		currentRecord++;
	}
	if (currentRecord != chainSize)
	{
		TRACE("(currentRecord!=chainSize) \n");
		return 0;
	}
	return 1;
}
#endif

//#ifdef MSG_WLAN_RESTART_SUPPORT
int boa_msg_id;

static int initBoaMsgQ()
{
	key_t key;
	int   queue_id;

	/* Create unique key via call to ftok() */
	key = ftok("/bin/init", 'w');

	if ((queue_id = open_queue(key, MQ_CREATE)) == -1) {
		perror("open_queue");
		return -1;
	}

	//printf("queue_id=%d\n", queue_id);
	return queue_id;
}

#define CMD_PPPOE_SESSIONID 2
void boa_msg_handler()
{
//	struct mymsgbuf qbuf;
	struct boamsgbuf qbuf;
	int ret;

	qbuf.mtype = this_pid;
	ret = read_message_nowait(boa_msg_id,&qbuf,this_pid);
	if(ret > 0){
#ifdef WLAN_SUPPORT
#ifdef MSG_WLAN_RESTART_SUPPORT
		if(qbuf.msg.cmd == CMD_WLAN_RESTART){
			config_WLAN(ACT_RESTART);
		}
#endif
#endif
		if(qbuf.msg.cmd == CMD_PPPOE_SESSIONID){
			//printf("#####CMD_PPPOE_SESSIONID\n");
			PPPOE_DRV_CTRL p;
			memcpy(&p, qbuf.msg.mtext, sizeof(PPPOE_DRV_CTRL));
			save_pppoe_sessionid(&p);
		}
	}
}

//#endif

int main(int argc, char **argv)
{
	int c;						/* command line arg */
	int s_port = 80;
	FILE *fp, *fp2;
	// Added by Mason Yu for Reset button
	FILE *fp3, *fp4;
    	int i, i2;
    	char reset_button_flag=1;
//xl_yue
#ifdef ACCOUNT_LOGIN_CONTROL
	struct errlogin_entry * perrlog;
	struct errlogin_entry * berrlog = NULL;
#endif

//xl_yue add
#ifdef USE_LOGINWEB_OF_SERVER
	struct user_info * pUser_info;
	struct user_info * bUser_info = NULL;

#ifdef LOGIN_ERR_TIMES_LIMITED
	struct errlogin_entry * perrlog;
	struct errlogin_entry * berrlog = NULL;
#endif

#endif


	Modem_LinkSpeed vLs;
	vLs.upstreamRate=0;
#ifdef CONFIG_USER_DDNS
	unsigned int totalEntry;
	MIB_CE_ATM_VC_T Entry;
	int j, startDDNS=0;
#endif
	char ifname[6];
	pid_t boa_pid;
	unsigned char value[16];

#ifdef SUPPORT_ASP
	websAspInit(argc,argv);
#endif
#ifdef EMBED
	log_pid();
//#endif
//#ifdef MSG_WLAN_RESTART_SUPPORT
	if((boa_msg_id = initBoaMsgQ()) < 0)
		printf("create boa_msg_id error\n");
//#endif
#endif
	openlog("boa", LOG_PID, 0);
#ifdef SERVER_SSL
	while ((c = getopt(argc, argv, "p:vc:dns")) != -1)
#else
	while ((c = getopt(argc, argv, "p:vc:d")) != -1)
#endif /*SERVER_SSL*/
	{
		switch (c) {
		case 'c':
			server_root = strdup(optarg);
			break;
		case 'v':
			verbose_logs = 1;
			break;
		case 'd':
			do_fork = 0;
			break;
#ifdef EMBED
		case 'p':
			s_port= atoi(optarg);
			break;
#endif
#if SERVER_SSL
		case 'n':
			do_sock = 0;		/*We don't want to do normal sockets*/
			break;
		case 's':
			do_ssl = 0;		/*We don't want to do ssl sockets*/
			break;

#endif /*SERVER_SSL*/
		default:
#if 0
			fprintf(stderr, "Usage: %s [-v] [-s] [-n] [-c serverroot] [-d]\n", argv[0]);
#endif
			exit(1);
		}
	}

	fixup_server_root();

#ifdef EMBED
	// Added by Mason Yu(2 level)
	fp = fopen("/var/boaSuper.passwd", "w+");
	fp2 = fopen("/var/boaUser.passwd", "w+");
	fclose(fp);
	fclose(fp2);
	#if SUPPORT_AUTH_DIGEST
	fp = fopen("/var/DigestUser.passwd", "w+");
	fclose(fp);
	fp = fopen("/var/DigestSuper.passwd", "w+");
	fclose(fp);
	#endif
#endif


	read_config_files();
#ifdef EMBED
	if(s_port != 80)
		set_server_port(s_port);
#endif
#ifdef BOA_TIME_LOG
	open_logs();
#endif
	create_common_env();

#ifdef SERVER_SSL
	if (do_ssl) {
		if (InitSSLStuff() != 1) {
			/*TO DO - emit warning the SSL stuff will not work*/
			syslog(LOG_ALERT, "Failure initialising SSL support - ");
			if (do_sock == 0) {
				syslog(LOG_ALERT, "    normal sockets disabled, so exiting");fflush(NULL);
				return 0;
			} else {
				syslog(LOG_ALERT, "    supporting normal (unencrypted) sockets only");fflush(NULL);
				do_sock = 2;
			}
	  }
	} else
		do_sock = 2;
#endif /*SERVER_SSL*/

#ifdef SERVER_SSL
	if(do_sock){
#endif /*SERVER_SSL*/
#ifdef INET6
	if ((server_s = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)) == -1)
#else
	if ((server_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
#endif
		die(NO_CREATE_SOCKET);

	/* server socket is nonblocking */
	if (fcntl(server_s, F_SETFL, NOBLOCK) == -1)
		die(NO_FCNTL);

	if ((setsockopt(server_s, SOL_SOCKET, SO_REUSEADDR, (void *) &sock_opt,
					sizeof(sock_opt))) == -1)
		die(NO_SETSOCKOPT);

	/* internet socket */
#ifdef INET6
	server_sockaddr.sin6_family = AF_INET6;
	memcpy(&server_sockaddr.sin6_addr,&in6addr_any,sizeof(in6addr_any));
	server_sockaddr.sin6_port = htons(server_port);
#else
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_sockaddr.sin_port = htons(server_port);
#endif

	if (bind(server_s, (struct sockaddr *) &server_sockaddr,
			 sizeof(server_sockaddr)) == -1)
		die(NO_BIND);

	/* listen: large number just in case your kernel is nicely tweaked */
	if (max_connections != -1)
		backlog = MIN(backlog, max_connections);
	if (listen(server_s, backlog) == -1)
		die(NO_LISTEN);

	if (server_s > max_fd)
		max_fd = server_s;
#ifdef SERVER_SSL
	}
#endif /*SERVER_SSL*/


#ifdef WLAN_WEB_REDIRECT //jiunming,web_redirect
	if( init_redirect_socket() > 0 )
		if (redir_server_s > max_fd)
			max_fd = redir_server_s;
#endif
#ifdef WEB_REDIRECT_BY_MAC
	if( init_mac_redirect_socket() > 0 )
		if (mac_redir_server_s > max_fd)
			max_fd = mac_redir_server_s;
#endif
#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
	if( init_captiveportal_socket() > 0 )
		if (captiveportal_server_s > max_fd)
			max_fd = captiveportal_server_s;
#endif
#ifdef QSETUP_WEB_REDIRECT
	if( init_qsetup_socket() > 0 )
		if (qsetup_server_s > max_fd)
			max_fd = qsetup_server_s;
#endif

	init_signals();
#ifdef CONSOLE_BOA
	signal(SIGINT, SIG_IGN);
#endif
	/* background ourself */

#ifndef EMBED
	if (do_fork)
		if (fork())
			exit(0);
#endif

	/* close server socket on exec
	 * so cgi's can't write to it */

	if (fcntl(server_s, F_SETFD, 1) == -1) {
#if 0
		perror("can't set close-on-exec on server socket!");
#endif
		exit(0);
	}


#if 0
	/* close STDIN on exec so cgi's can't read from it */
	if (fcntl(STDIN_FILENO, F_SETFD, 1) == -1) {
#if 0
		perror("can't set close-on-exec on STDIN!");
#endif
		exit(0);
	}
#endif

	/* translate paths to server_chroot */
	chroot_aliases();
	chroot_virtual_hosts();

#ifdef USE_CHROOT
	if (server_chroot)
	{
		if (!strncmp(server_root,server_chroot,strlen(server_chroot)))
				strcpy(server_root,server_root + strlen(server_chroot) -
						(server_chroot[strlen(server_chroot)-1]=='/'?1:0) );
		else
		{
#ifdef BOA_TIME_LOG
			log_error_time();
			fprintf(stderr,"Warning: server_root not accessible from %s\n",
					server_chroot);
#endif
			syslog(LOG_ERR, "server root not accessible");
		}
	}
  if (server_chroot)
  {
    if (!strncmp(dirmaker,server_chroot,strlen(server_chroot)))
        strcpy(dirmaker,dirmaker + strlen(server_chroot) -
            (server_chroot[strlen(server_chroot)-1]=='/'?1:0) );
    else
    {
#ifdef BOA_TIME_LOG
      log_error_time();
      fprintf(stderr,"Warning: directory maker not accessible from %s\n",
          server_chroot);
#endif
	  syslog(LOG_ERR, "directory maker not accessible");
    }
  }
#endif

	auth_check();	/* Check Auth'ed directories */

	DBG(printf("main:give out privs\n");)
	/* give away our privs if we can */
#ifdef EMBED
	server_gid = getgid();
	server_uid = getuid();
#else
	if (getuid() == 0) {
		struct passwd *passwdbuf;
		passwdbuf = getpwuid(server_uid);
		if (passwdbuf == NULL)
			die(GETPWUID);
		if (initgroups(passwdbuf->pw_name, passwdbuf->pw_gid) == -1)
			die(INITGROUPS);
                if (server_chroot)
                  if (chroot(server_chroot))
                    die(CANNOT_CHROOT);
		if (setgid(server_gid) == -1)
			die(NO_SETGID);
		if (setuid(server_uid) == -1)
			die(NO_SETUID);
	} else {
		if (server_gid || server_uid) {
#ifdef BOA_TIME_LOG
			log_error_time();
			fprintf(stderr, "Warning: "
					"Not running as root: no attempt to change to uid %d gid %d\n",
					server_uid, server_gid);
#endif
		}
		server_gid = getgid();
		server_uid = getuid();
	}
#endif /* EMBED */

	/* main loop */

#ifdef EMBED

	// Added by Mason Yu(2 level)
	writePasswdFile();

#ifdef MENU_CLI
	// start climenu thread
	startCli();
#endif
	// start save/reboot thread
	sem_init(&semSave, 0, 0);
	startSaved();
#endif
	timestamp();
	FD_ZERO(&block_read_fdset);
	FD_ZERO(&block_write_fdset);

	status.connections = 0;
	status.requests = 0;
	status.errors = 0;
//#if 0
//jim


#ifdef EMBED
#ifdef CONFIG_CTC_E8_CLIENT_LIMIT
	//memset(accepted_PC_Clients, 0, sizeof(accepted_PC_Clients));
	//memset(accepted_Camera_Clients, 0, sizeof(accepted_Camera_Clients));
#ifdef _CWMP_MIB_
	proc_write_for_mwband();
#endif
	printf("creat process to hander read_proc_handler\n");
	{
		pid_t pid;
		pid=fork();
		if(pid==-1)
		{
			printf("fork error\n");
			return;
		}
		if(pid==0)
		{	//child process
			while(1)
			{
				sleep(1);
				read_proc_handler(0);  //for iptables
			}
		}
		if(pid)
		{
			printf("child process pid=%d created in boa\n ", pid);
		}
	}

#endif
// Kaohj --- hook auto-hunt register here
#ifdef AUTO_PVC_SEARCH_AUTOHUNT
	void register_pvc(int signum);
	// Auto-pvc search ---- auto-hunt
	boa_pid = (int)getpid();
	//printf("Inform boa PID %d to kernel by sarctl\n", boa_pid);
	sprintf(value,"%d",boa_pid);

        /* inform kernel the boa process ID for the further kernel signal trigger */
	if (va_cmd("/bin/sarctl",2,1,"boa_pid",value))
		printf("sarctl boa_pid %s failed\n", value);

	signal(SIGUSR2, register_pvc);
#endif
#endif // of EMBED

#ifdef PARENTAL_CTRL
	parent_ctrl_table_init();
#endif

#ifdef EMBED
#ifdef WEB_REDIRECT_BY_MAC		
	TIMEOUT(clearLandingPageRule, 0, 10, landingPage_ch);
#endif

#ifdef AUTO_DETECT_DMZ
extern struct callout autoDMZ_ch;
	TIMEOUT(poll_autoDMZ, 0, 10, autoDMZ_ch);
#endif

// Timer for auto search PVC
#if defined(AUTO_PVC_SEARCH_TR068_OAMPING) || defined(AUTO_PVC_SEARCH_PURE_OAMPING) || defined(AUTO_PVC_SEARCH_AUTOHUNT)
	extern struct callout autoSearchPVC_ch;	
	
#endif
#endif // of EMBED

	while (1) {
		static int dsl_link = 0;
		static int do_AutoSearchPVC = 0;
		int mypid;
		unsigned char vChar;

#ifdef EMBED
                calltimeout();	

		// Kaohj --- Internet LED
		//time_t newtime,oldtime=0;
//star: for wlan on/off button
#if defined(WLAN_SUPPORT) && defined(CONFIG_WLAN_ON_OFF_BUTTON)
		int button_flag;
		unsigned char wlan_flag;
		FILE *fpwlan;
		fpwlan = fopen("/proc/wlan_onoff","r+");
		if(fpwlan)
		{
			fscanf(fpwlan,"%d",&button_flag);

			if(button_flag==1)
			{
				mib_get( MIB_WLAN_DISABLED, (void *)&vChar);
				if(vChar == 0)
					vChar = 1;
				else
					vChar = 0;
				update_wlan_disable(vChar);
				//mib_set( MIB_WLAN_DISABLED, (void *)&vChar);

				if(mib_update(CURRENT_SETTING, CONFIG_MIB_ALL) == 0)
					printf("CS Flash error! \n");
				config_WLAN(ACT_RESTART);

				wlan_flag = '0';
				fwrite(&wlan_flag,1,1,fpwlan);
			}
			fclose(fpwlan);
		}
#endif
//cathy, wsc write "R" to gpio for restarting wlan, if "2" is read from gpio->restart wlan
#if defined(WLAN_SUPPORT) && defined(CONFIG_WIFI_SIMPLE_CONFIG)
		{
			int wps_wlan_restart;
			//unsigned char wps_flag;
			FILE *fpwps;
			fpwps = fopen("/proc/gpio","r+");
			if(fpwps) {
				fscanf(fpwps,"%d",&wps_wlan_restart);
				if(wps_wlan_restart==2) {
					printf("wps wlan restart\n");
					config_WLAN(ACT_RESTART);
				}
				fclose(fpwps);
			}
		}
#endif
//#endif

#ifdef MIB_WLAN_RESTART_SUPPORT
	mib_get(MIB_WLAN_RESTART,(void *)&vChar);
	if(vChar){
		config_WLAN(ACT_RESTART);
		vChar = 0;
		mib_set(MIB_WLAN_RESTART,(void *)&vChar);
	}
#endif
	boa_msg_handler();
#endif // of EMBED


#ifdef EMBED
		// Added by Mason Yu for check Reset button
		// (1) Check Reset to default
		fp3=fopen("/proc/load_default","r");
		
		if (fp3 && reset_button_flag)
		{
        		fscanf(fp3,"%d",&i);
        		if(i==1)
        		{
            		sleep (5);
            		printf("Going to Reload Default\n");
#ifdef	RESERVE_KEY_SETTING
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
#else
					va_cmd("/bin/flash", 2, 1, "default", "cs");
					semSave.__sem_value=SEM_REBOOT;
#endif
					sem_post(&semSave);
					reset_button_flag = 0;
        		}
        		fclose(fp3);
        	}

        	// (2) Check Reset to current
		fp4=fopen("/proc/load_reboot","r");

		if (fp4 && reset_button_flag)
		{
        		fscanf(fp4,"%d",&i2);
        		if(i2==1)
        		{
            		printf("Going to Reboot\n");
#ifdef	RESERVE_KEY_SETTING
					//ql add to reserve some key parameters
					//first backup current setting
					mib_backup(CONFIG_MIB_ALL);
					//mib_record_reserve(CONFIG_MIB_ALL);

					//va_cmd("/bin/flash", 2, 1, "default", "cs");
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
            		// wake up the Save/reboot thread
					sem_post(&semSave);
					reset_button_flag = 0;
        		}
        		fclose(fp4);
        	}

#endif // of EMBED
#ifdef EMBED
        	// Added by Mason Yu for Start DDNS client with static Link(route1483/MER with Dhcp client)
       		if ( adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs,
			RLCM_GET_LINK_SPEED_SIZE) && vLs.upstreamRate != 0) {

#ifdef CONFIG_USER_DDNS
	        	if ( startDDNS == 0 ) {
				//sleep(1000);
				for (j=0; j<2000; j++)

				totalEntry = mib_chain_total(MIB_ATM_VC_TBL); /* get chain record size */
				for (i=0; i<totalEntry; i++) {
					mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry); /* get the specified chain record */

					if ( (Entry.cmode == ADSL_RT1483) || ((Entry.cmode == ADSL_MER1483) && (Entry.ipDhcp == DHCP_DISABLED)) ) {
						sprintf(ifname, "vc%u", VC_INDEX(Entry.ifIndex));
						va_cmd("/bin/updateddctrl", 1, 1, ifname);
					}
				}
				startDDNS = 1;
			}
#endif
			if (dsl_link == 0) { // down --> up
				unsigned char igmpItf;
				int flags;
				restart_dnsrelay();
				syslog(LOG_INFO, "DSL Link up");
				// signal dhcp client to renew
				mypid = read_pid((char*)DHCPC_PID);
				if (mypid > 0) { // renew
					kill(mypid, SIGUSR1);
				}
				updateLinkTime(1);
#ifdef CONFIG_USER_IGMPPROXY
				// Kaohj -- if igmpproxy upstream interface is up, signal to reset multicast MFC
				mypid = read_pid("/var/run/igmp_pid");
				if (mypid >= 1) {
					if (mib_get(MIB_IGMP_PROXY_ITF, (void *)&igmpItf) != 0)
					{
						if (igmpItf != 0xff)
						{
							if (PPP_INDEX(igmpItf) != 0x0f)
								snprintf(ifname, 6, "ppp%u", PPP_INDEX(igmpItf));
							else
								snprintf(ifname, 5, "vc%u", VC_INDEX(igmpItf));
							if (getInFlags( ifname, &flags) == 1){
								if (flags & IFF_UP)
									kill(mypid, SIGUSR2);
							}
						}
						else
						{
							printf("Error: IGMP proxy interface not available !\n");
						}
					}
				}
#endif // of CONFIG_USER_IGMPPROXY
			}
			dsl_link = 1;
			
			// Mason Yu
#if defined(AUTO_PVC_SEARCH_TR068_OAMPING) || defined(AUTO_PVC_SEARCH_PURE_OAMPING) || defined(AUTO_PVC_SEARCH_AUTOHUNT)			
			if ( do_AutoSearchPVC == 0 ) {												
				if (-1 != startAutoSearchPVC()) {
					do_AutoSearchPVC = 1;					
				}				
			}
#endif
		}
		else {
			if (dsl_link == 1) { // up --> down
				syslog(LOG_INFO, "DSL Link down");
			}
			dsl_link = 0;
			
			// Mason Yu
#if defined(AUTO_PVC_SEARCH_TR068_OAMPING) || defined(AUTO_PVC_SEARCH_PURE_OAMPING) || defined(AUTO_PVC_SEARCH_AUTOHUNT)
			//printf("fin_AutoSearchPVC=%d sok=%d autoHunt_found=%d\n", fin_AutoSearchPVC, sok, autoHunt_found);
			if ( fin_AutoSearchPVC == 0 && sok == 0 && autoHunt_found == 0 && do_AutoSearchPVC== 1) {
				do_AutoSearchPVC = 0;
				UNTIMEOUT(stopAutoSearchPVC, 0, autoSearchPVC_ch);				
			}
#endif
		}
#endif // of EMBED
		
		// jim the internet polling will slow down boa response time....
		// espacially, it will cause ZTE 531B web upgrade fail....
		// Kaohj --- Internet LED (moved from startup.c)
		//newtime = time(0);
		//if(newtime-oldtime>=5){
		//	Internet_status();
		//	oldtime = newtime;
		//}
//xl_yue add,if no action lasting for 5 minutes,auto logout
#ifdef USE_LOGINWEB_OF_SERVER
		for(pUser_info = user_login_list;pUser_info;bUser_info = pUser_info,pUser_info = pUser_info->next){
#ifdef AUTO_LOGOUT_SUPPORT
			//if no action lasting for 5 minutes,auto logout
			if(time_counter - pUser_info->last_time > LOGIN_AUTOEXIT_TIME)
#else
			//default:1 day
			if(time_counter - pUser_info->last_time > LOGIN_MAX_TIME)
#endif
			{
				ulist_free_login_entry(bUser_info,pUser_info);
			}
		}

#ifdef LOGIN_ERR_TIMES_LIMITED
		for(perrlog = errlogin_list;perrlog;berrlog = perrlog,perrlog = perrlog->next){
			if(time_counter - perrlog->last_time > LOGIN_ERR_WAIT_TIME){//after 1 mintue, unlist and free
				ulist_free_errlog_entry(berrlog,perrlog);
			}
		}
#endif
#endif

#ifdef PARENTAL_CTRL
		if ((time_counter & 0x07) == 0 )
		{
			time_t tm;
			struct tm the_tm;

	 		time(&tm);
			memcpy(&the_tm, localtime(&tm), sizeof(the_tm));
			parent_ctrl_table_rule_update();
			//printf("!!!UPdate!! \r\n");
			//parent_ctrl_table_rule_update();

//			printf("day of the week:%d\r\n",(int)the_tm.tm_wday);
//			printf("month:%d",the_tm.tm_mon);
//			printf("time_counter:%d\r\n",time_counter);

		}
#endif


#ifdef ACCOUNT_LOGIN_CONTROL
		//added by xl_yue,check the errlogin_list and free the entrys that have pass 1 minutes
		for(perrlog = errlogin_list;perrlog;berrlog = perrlog,perrlog = perrlog->next){

			if(time_counter - perrlog->last_time > LOGIN_WAIT_TIME){//after 1 mintue, unlist and free
				if(perrlog == errlogin_list)
					errlogin_list = perrlog->next;
				else
					berrlog->next = perrlog->next;

				free(perrlog);
			}
		}

		//Added by xl_yue;
		if((time_counter - su_account.last_time > ACCOUNT_TIMEOUT) && su_account.account_busy){		//account timeout,then logout
			su_account.account_timeout = 1;
			su_account.account_busy = 0;
//			printf("su_account timeout\n");
//		}else{
//			su_account.account_timeout = 0;
//			printf("su_account return not timeout\n");
		}
		//added by xl_yue
		if((time_counter - us_account.last_time > ACCOUNT_TIMEOUT) && us_account.account_busy){
			us_account.account_timeout = 1;
			us_account.account_busy = 0;
//			printf("us_account timeout\n");
//		}else{
//			us_account.account_timeout = 0;
//			printf("us_account return not timeout\n");
		}
#endif

		if (sighup_flag)
			sighup_run();
		if (sigchld_flag)
			sigchld_run();

		switch(lame_duck_mode) {
			case 1:
				lame_duck_mode_run(server_s);
                        case 2:
				if (!request_ready && !request_block)
					die(SHUTDOWN);
				break;
			default:
				break;
		}

		/* move selected req's from request_block to request_ready */
		fdset_update();

		if (!request_ready) {
			request *current;

			max_fd = 0;
			max_fd = MAX(server_s, max_fd);
#ifdef WLAN_WEB_REDIRECT //jiunming,web_redirect
			max_fd = MAX(redir_server_s, max_fd);
			FD_SET(redir_server_s, &block_read_fdset);
#endif
#ifdef WEB_REDIRECT_BY_MAC
			max_fd = MAX(mac_redir_server_s, max_fd);
			FD_SET(mac_redir_server_s, &block_read_fdset);
#endif
#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
			max_fd = MAX(captiveportal_server_s, max_fd);
			FD_SET(captiveportal_server_s, &block_read_fdset);
#endif
#ifdef QSETUP_WEB_REDIRECT
			max_fd = MAX(qsetup_server_s, max_fd);
			FD_SET(qsetup_server_s, &block_read_fdset);
#endif
#ifdef SERVER_SSL
			if (do_sock < 2)
				max_fd = MAX(server_ssl, max_fd);
#endif
			for (current = request_block; current; current = current->next) {
				max_fd = MAX(current->fd, max_fd);
				max_fd = MAX(current->data_fd, max_fd);
				max_fd = MAX(current->post_data_fd, max_fd);
			}

			if (select(max_fd + 1, &block_read_fdset, &block_write_fdset, NULL,
					   (request_block ? &req_timeout : NULL)) == -1) {
				if (errno == EINTR || errno == EBADF)
					continue;	/* while(1) */
				else
					die(SELECT);
			}
#ifdef SERVER_SSL
			if(do_sock){
				if (FD_ISSET(server_s, &block_read_fdset))
					get_request();
			}
#else
			if (FD_ISSET(server_s, &block_read_fdset))
				get_request();
#ifdef WLAN_WEB_REDIRECT //jiunming,web_redirect
			if (FD_ISSET(redir_server_s, &block_read_fdset))
				get_redir_request();
#endif
#ifdef WEB_REDIRECT_BY_MAC
			if (FD_ISSET(mac_redir_server_s, &block_read_fdset))
				get_mac_redir_request();
#endif
#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
			if (FD_ISSET(captiveportal_server_s, &block_read_fdset))
				get_captiveportal_request();
#endif
#ifdef QSETUP_WEB_REDIRECT
			if (FD_ISSET(qsetup_server_s, &block_read_fdset))
				get_qsetup_request();
#endif
#endif /*SERVER_SSL*/

#ifdef SERVER_SSL
			if (do_sock < 2) {
				if(FD_ISSET(server_ssl, &block_read_fdset)){ /*If we have the main SSL server socket*/
/*					printf("SSL request received!!\n");*/
					get_ssl_request();
				}
			}
#endif /*SERVER_SSL*/
		}
		process_requests();		/* any blocked req's move from request_ready to request_block */
	}
}

/*
 * Name: fdset_update
 *
 * Description: iterate through the blocked requests, checking whether
 * that file descriptor has been set by select.  Update the fd_set to
 * reflect current status.
 */

void fdset_update(void)
{
	request *current, *next;
	time_t current_time;

	current = request_block;

	current_time = time_counter;

	while (current) {
		time_t time_since;
		next = current->next;

		time_since = current_time - current->time_last;

		/* hmm, what if we are in "the middle" of a request and not
		 * just waiting for a new one... perhaps check to see if anything
		 * has been read via header position, etc... */
		if (current->kacount && (time_since >= ka_timeout) && !current->logline) {
			SQUASH_KA(current);
			free_request(&request_block, current);
		} else if (time_since > REQUEST_TIMEOUT) {
#ifdef BOA_TIME_LOG
			log_error_doc(current);
			fputs("connection timed out\n", stderr);
#endif
			SQUASH_KA(current);
			free_request(&request_block, current);
		} else if (current->buffer_end) {
			if (FD_ISSET(current->fd, &block_write_fdset))
				ready_request(current);
		} else {
			switch (current->status) {
			case PIPE_WRITE:
			case WRITE:
				if (FD_ISSET(current->fd, &block_write_fdset))
					ready_request(current);
				else
					FD_SET(current->fd, &block_write_fdset);
				break;
			case PIPE_READ:
				if (FD_ISSET(current->data_fd, &block_read_fdset))
					ready_request(current);
				else
					FD_SET(current->data_fd, &block_read_fdset);
				break;
			case BODY_WRITE:
				if (FD_ISSET(current->post_data_fd, &block_write_fdset))
					ready_request(current);
				else
					FD_SET(current->post_data_fd, &block_write_fdset);
				break;
			default:
				if (FD_ISSET(current->fd, &block_read_fdset))
					ready_request(current);
				else
					FD_SET(current->fd, &block_read_fdset);
				break;
			}
		}
		current = next;
	}

	if (!lame_duck_mode &&
      (max_connections == -1 || status.connections < max_connections)) {
#ifdef SERVER_SSL
    if (do_sock) {
			FD_SET(server_s, &block_read_fdset);	/* server always set */
		}
#else
		FD_SET(server_s, &block_read_fdset);	/* server always set */
#endif /*SERVER_SSL*/
	} else {
		if (server_s != -1
#ifdef SERVER_SSL
        && do_sock
#endif
        )
			FD_CLR(server_s, &block_read_fdset);
	}

#ifdef SERVER_SSL
	if (do_sock < 2) {
		if (max_connections == -1 || status.connections < max_connections) {
		  FD_SET(server_ssl, &block_read_fdset);
 	   /* printf("Added server_ssl to fdset\n");*/
		} else {
		  FD_CLR(server_ssl, &block_read_fdset);
		}
	}
#endif /*SERVER_SSL*/

	req_timeout.tv_sec = (ka_timeout ? ka_timeout : REQUEST_TIMEOUT);
	req_timeout.tv_usec = 0l;	/* reset timeout */
}

/*
 * Name: die
 * Description: die with fatal error
 */

void die(int exit_code)
{
#ifdef BOA_TIME_LOG
	log_error_time();

	switch (exit_code) {
	case SERVER_ERROR:
		fputs("fatal error: exiting\n", stdout);
		break;
	case OUT_OF_MEMORY:
		perror("malloc");
		break;
	case NO_CREATE_SOCKET:
		perror("socket create");
		break;
	case NO_FCNTL:
		perror("fcntl");
		break;
	case NO_SETSOCKOPT:
		perror("setsockopt");
		break;
	case NO_BIND:
		perror("bind");
		break;
	case NO_LISTEN:
		perror("listen");
		break;
	case NO_SETGID:
		perror("setgid/initgroups");
		break;
	case NO_SETUID:
		perror("setuid");
		break;
	case NO_OPEN_LOG:
		perror("logfile fopen");	/* ??? */
		break;
	case SELECT:
		perror("select");
		break;
	case GETPWUID:
		perror("getpwuid");
		break;
	case INITGROUPS:
		perror("initgroups");
		break;
	case CANNOT_CHROOT:
                perror("chroot");
                break;
	case SHUTDOWN:
		fputs("completing shutdown\n", stderr);
		break;
	default:
		break;
	}
#endif
	syslog(LOG_WARNING, "Shutting down - %d", exit_code);

	fclose(stderr);
	exit(exit_code);
}

#ifdef SERVER_SSL
#ifdef USES_MATRIX_SSL
int
InitSSLStuff(void)
{
 	syslog(LOG_NOTICE, "Enabling SSL security system");
	if ((server_ssl = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
		syslog(LOG_ALERT,"Couldn't create socket for ssl");
		die(NO_CREATE_SOCKET);
		return 0;
	}
	
	/* server socket is nonblocking */
	if (fcntl(server_ssl, F_SETFL, NOBLOCK) == -1){
		syslog(LOG_ALERT, "%s, %i:Couldn't fcntl", __FILE__, __LINE__);
		die(NO_FCNTL);
		return 0;
	}
	if ((setsockopt(server_ssl, SOL_SOCKET, SO_REUSEADDR, (void *) &sock_opt,
		sizeof(sock_opt))) == -1){
		syslog(LOG_ALERT,"%s, %i:Couldn't sockopt", __FILE__,__LINE__);
		die(NO_SETSOCKOPT);
		return 0;
	}
	/* internet socket */
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_sockaddr.sin_port = htons(ssl_server_port);
	
	if (bind(server_ssl, (struct sockaddr *) &server_sockaddr,
		sizeof(server_sockaddr)) == -1){
		syslog(LOG_ALERT, "Couldn't bind ssl to port %d", ntohs(server_sockaddr.sin_port));
		die(NO_BIND);
		return 0;
	}

	/* listen: large number just in case your kernel is nicely tweaked */
	if (listen(server_ssl, backlog) == -1){
		die(NO_LISTEN);
		return 0;		
	}

	if (server_ssl > max_fd)
		max_fd = server_ssl;

	/*Matrix SSL initialization */	
	if (matrixSslOpen() < 0) {
		die(NO_SSL);
		fprintf(stderr, "matrixSslOpen failed, exiting...");
		return 0;
	}

	/* read ssl key and cert from file*/
	if (matrixSslReadKeys(&mtrx_keys,SSL_CERTF ,SSL_KEYF , NULL, NULL) < 0)  {
		fprintf(stderr, "Error reading or parsing %s or %s.\n", 
			SSL_CERTF, SSL_KEYF);
		return 0;
	}
		
	return 1;
}

#else /*!USES_MATRIX_SSL*/
int
InitSSLStuff(void)
{
	syslog(LOG_NOTICE, "Enabling SSL security system");
#ifdef INET6
	if ((server_ssl = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)) == -1)
	{
		die(NO_CREATE_SOCKET);
		return 0;
	}
#else
	if ((server_ssl = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
		syslog(LOG_ALERT,"Couldn't create socket for ssl");
		die(NO_CREATE_SOCKET);
		return 0;
	}
#endif /*INET6*/

	/* server socket is nonblocking */
	if (fcntl(server_ssl, F_SETFL, NOBLOCK) == -1){
		syslog(LOG_ALERT, "%s, %i:Couldn't fcntl", __FILE__, __LINE__);
		die(NO_FCNTL);
		return 0;
	}

	if ((setsockopt(server_ssl, SOL_SOCKET, SO_REUSEADDR, (void *) &sock_opt,
		sizeof(sock_opt))) == -1){
		syslog(LOG_ALERT,"%s, %i:Couldn't sockopt", __FILE__,__LINE__);
		die(NO_SETSOCKOPT);
		return 0;
	}

	/* internet socket */
#ifdef INET6
	server_sockaddr.sin6_family = AF_INET6;
	memcpy(&server_sockaddr.sin6_addr,&in6addr_any,sizeof(in6addr_any));
	server_sockaddr.sin6_port = htons(ssl_server_port);
#else
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_sockaddr.sin_port = htons(ssl_server_port);
#endif

	if (bind(server_ssl, (struct sockaddr *) &server_sockaddr,
		sizeof(server_sockaddr)) == -1){
		syslog(LOG_ALERT, "Couldn't bind ssl to port %d", ntohs(server_sockaddr.sin_port));
		die(NO_BIND);
		return 0;
	}

	/* listen: large number just in case your kernel is nicely tweaked */
	if (listen(server_ssl, backlog) == -1){
		die(NO_LISTEN);
		return 0;
	}

	if (server_ssl > max_fd)
		max_fd = server_ssl;

	/*Init all of the ssl stuff*/
//	i don't know why this line is commented out... i found it like that - damion may-02
/*	SSL_load_error_strings();*/
	SSLeay_add_ssl_algorithms();
	meth = SSLv23_server_method();
	if(meth == NULL){
		ERR_print_errors_fp(stderr);
		syslog(LOG_ALERT, "Couldn't create the SSL method");
		die(NO_SSL);
		return 0;
	}
	ctx = SSL_CTX_new(meth);
	if(!ctx){
		syslog(LOG_ALERT, "Couldn't create a connection context\n");
		ERR_print_errors_fp(stderr);
		die(NO_SSL);
		return 0;
	}

	if (SSL_CTX_use_certificate_file(ctx, SSL_CERTF, SSL_FILETYPE_PEM) <= 0) {
		syslog(LOG_ALERT, "Failure reading SSL certificate file: %s",SSL_CERTF);fflush(NULL);
		close(server_ssl);
		return 0;
	}
	syslog(LOG_DEBUG, "Loaded SSL certificate file: %s",SSL_CERTF);fflush(NULL);

	if (SSL_CTX_use_PrivateKey_file(ctx, SSL_KEYF, SSL_FILETYPE_PEM) <= 0) {
		syslog(LOG_ALERT, "Failure reading private key file: %s",SSL_KEYF);fflush(NULL);
		close(server_ssl);
		return 0;
	}
	syslog(LOG_DEBUG, "Opened private key file: %s",SSL_KEYF);fflush(NULL);

	if (!SSL_CTX_check_private_key(ctx)) {
		syslog(LOG_ALERT, "Private key does not match the certificate public key");fflush(NULL);
		close(server_ssl);
		return 0;
	}

	/*load and check that the key files are appropriate.*/
	syslog(LOG_NOTICE,"SSL security system enabled");
	return 1;
}
#endif /* USES_MATRIX_SSL */
#endif /*SERVER_SSL*/
