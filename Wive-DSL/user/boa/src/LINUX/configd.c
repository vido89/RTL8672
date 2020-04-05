/*
 * configd.c --- main file for configuration server
 * --- By Kaohj
 */ 
#include <semaphore.h>
#include "../msgq.h"
#include "mib.h"	// for FLASH_BLOCK_SIZE
#include <stdio.h>
#include <signal.h>
#include "../syslog.h"
// Added by Mason Yu
#include "utility.h"

#define CONFIGD_RUNFILE	"/var/run/configd.pid"
int msgqueue_id;
int this_pid;

#ifdef CONFIG_USER_DDNS
sem_t semDdnsc;
extern int g_killprocess;
extern char g_ddns_ifname[10];
#define AUTO_STARTDDNS_INTERVAL	60*60*24	/* start auto-startDDNSC every 24 hours */
//#define AUTO_STARTDDNS_INTERVAL	5	/* start auto-startDDNSC every 24 hours */
#endif

// Mason Yu On True
#ifdef SYSLOG_SUPPORT
#ifdef SEND_LOG
sem_t semSyslogC;
#define AUTO_STARTSYSLOGC_INTERVAL 60*60*24	/* start auto_startSYSLOGC every 24 hours */
#endif
#endif

static void log_pid()
{
	FILE *f;
	pid_t pid;
	char *pidfile = CONFIGD_RUNFILE;

	pid = getpid();
	this_pid = pid;
	
	if((f = fopen(pidfile, "w")) == NULL)
		return;
	fprintf(f, "%d\n", pid);
	fclose(f);
}

void init_signals(void)
{
	struct sigaction sa;

	sa.sa_flags = 0;

	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGSEGV);
	sigaddset(&sa.sa_mask, SIGBUS);
	sigaddset(&sa.sa_mask, SIGTERM);
	sigaddset(&sa.sa_mask, SIGHUP);
	sigaddset(&sa.sa_mask, SIGINT);
	sigaddset(&sa.sa_mask, SIGPIPE);
	sigaddset(&sa.sa_mask, SIGCHLD);
	sigaddset(&sa.sa_mask, SIGUSR1);
#ifdef EMBED
//	sigaddset(&sa.sa_mask, SIGUSR2);  // andrew
#endif
/*
	sigaddset(&sa.sa_mask, SIGALRM);

	sa.sa_handler = sigsegv;
	sigaction(SIGSEGV, &sa, NULL);

	sa.sa_handler = sigbus;
	sigaction(SIGBUS, &sa, NULL);

	sa.sa_handler = sigterm;
	sigaction(SIGTERM, &sa, NULL);

	sa.sa_handler = sighup;
	sigaction(SIGHUP, &sa, NULL);

	sa.sa_handler = sigint;
	sigaction(SIGINT, &sa, NULL);

	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, NULL);

	sa.sa_handler = sigchld;
	sigaction(SIGCHLD, &sa, NULL);

	sa.sa_handler = sigusr1;
	sigaction(SIGUSR1, &sa, NULL);

#ifdef EMBED
	// handle msg queue event
	sa.sa_handler = sigusr2;
	sigaction(SIGUSR2, &sa, NULL);
#endif
*/
}

static int initMsgQ()
{
	key_t key;
	int   queue_id;
	
	/* Create unique key via call to ftok() */
	key = ftok("/bin/init", 'k');
	
	if ((queue_id = open_queue(key, MQ_CREATE)) == -1) {
		perror("open_queue");
		return -1;
	}
	
	//printf("queue_id=%d\n", queue_id);
	return queue_id;
}

void msg_handler() {
	struct mymsgbuf qbuf;	
	
	while (1) {		
		read_message(msgqueue_id, &qbuf, this_pid);
		//printf("process message...\n");
		msgProcess(&qbuf);
		send_message(msgqueue_id, qbuf.mtype, qbuf.request, &qbuf.msg);
	}
}

int flashdrv_filewrite(FILE *fp,int size,void  *dstP) //Brian
{
	int i;
	int nWritten; /* completed bytes */
	//volatile unsigned char data[4096];
	volatile unsigned short *dstAddr;
	unsigned char *block;
	nWritten = 0;
	dstAddr = dstP;
	
	block = malloc(FLASH_BLOCK_SIZE);
	if(!block)
	return 1;

	// Kaohj -- destroying image
	//printf("destroying image ...\n");
	flash_write((void *)block, (char *)dstP+0x120000, FLASH_BLOCK_SIZE);
	
	while ( nWritten < size )
	{
		int nRead;

		/* fill buffer with file */
		i = (FLASH_BLOCK_SIZE > (size-nWritten)) ? (size-nWritten) : FLASH_BLOCK_SIZE;
		nRead = fread (block, 1, i, fp);
		
		printf("flashWrite --> %08x (len %d)\n", dstAddr, nRead);
		//flash_write( (void*)block, (void*)dstAddr, nRead );
		if (!flash_write((void*)block, (void*)dstAddr, nRead)) {
			if (block)
				free(block);
			return 1;
		}

		dstAddr += nRead >> 1;
		nWritten += nRead;
	}
	
	free(block);
	return 0;
}

#ifdef CONFIG_USER_DDNS	
void auto_startDDNSC(void)
{			
	strcpy(g_ddns_ifname, "all");
	sem_post(&semDdnsc);
	signal (SIGALRM, auto_startDDNSC);
	alarm(AUTO_STARTDDNS_INTERVAL);
}

//#ifdef _CWMP_APPLY_
//star: add for restarting ddns
void restartddns()
{
	strcpy(g_ddns_ifname, "all");
	sem_post(&semDdnsc);
	return;
}
static void log_ddnspid()
{
	FILE *f;
	pid_t pid;
	char *pidfile = "/var/run/ddns.pid";

	pid = getpid();
	printf("\nddnspid=%d\n",pid);
	
	if((f = fopen(pidfile, "w")) == NULL)
		return;
	fprintf(f, "%d\n", pid);
	fclose(f);

}
//#endif
void ddnsC (void)
{
	unsigned int entryNum, i;
	char account[70];	
	MIB_CE_DDNS_T Entry;	
	

	/* Establish a handler for SIGALRM signals. */
	signal(SIGALRM, auto_startDDNSC);
	alarm(AUTO_STARTDDNS_INTERVAL);

//#ifdef _CWMP_APPLY_
	//star: add for restarting ddns
	log_ddnspid();
	signal(SIGUSR1, restartddns);
//#endif
	
	// wait for user start DDNS Client
	while(1) {
		sem_wait(&semDdnsc);
	//	printf("ddnsC: g_ddns_ifname=%s\n", g_ddns_ifname);
		
		entryNum = mib_chain_total(MIB_DDNS_TBL);

		for (i=0; i<entryNum; i++) {
        	
			if (!mib_chain_get(MIB_DDNS_TBL, i, (void *)&Entry))
			{
  				printf("ddnsC:Get chain record error!\n");
				continue;
			}
			  
			// Check all variables that updatedd need
			if ( strlen(Entry.username) == 0 ) {
				printf("ddnsC: username/email is NULL!!\n");
				continue;
			}	
			
			if ( strlen(Entry.password) == 0 ) {
				printf("ddnsC: password/key is NULL!!\n");
				continue;
			}
			
			if ( strlen(Entry.hostname) == 0 ) {
				printf("ddnsC: Hostname is NULL!!\n");
				continue;
			}
			
			if ( strlen(Entry.interface) == 0 ) {
				printf("ddnsC: Interface is NULL!!\n");
				continue;
			}
			
			if ( Entry.Enabled != 1 ) {
				printf("ddnsC: The account is disabled!!\n");
				continue;
			}

//			if ( (strcmp(g_ddns_ifname, Entry.interface) == 0) || (strcmp(Entry.interface, "br0") == 0) 
//			if ( (strcmp(Entry.interface, "all") == 0) || (strcmp(g_ddns_ifname, Entry.interface) == 0) || (strcmp(Entry.interface, "br0") == 0) 
//				|| (strcmp(g_ddns_ifname, "all") == 0) ) {
				if ( strcmp(Entry.provider, "dyndns") == 0 || strcmp(Entry.provider, "tzo") == 0) {
					sprintf(account, "%s:%s", Entry.username, Entry.password);
				}else {
					//sprintf(account, "%s:%s", Entry.email, Entry.key);
					printf("ddnsC: Not support this provider\n");
					syslog(LOG_INFO, "ddnsC: Not support this provider %s\n", Entry.provider);
					continue;
				}
				
				
				if ( g_killprocess == KILL_PROCESS_OFF ) {			
					printf("ddnsC: Start updatedd with %s interface\n", Entry.interface);
					syslog(LOG_INFO, "ddnsC: %s Start updatedd - provider is %s\n", g_ddns_ifname, Entry.provider);
					// The 3rd arg(1) is that the commane will be blocked before the command is end.  
					va_cmd("/bin/updatedd", 3, 1, Entry.provider, account, Entry.hostname);							
				}				
//			}						
		}
		
	}	
}

void startDdnsc()
{
	pthread_t ptDdnscId;
	if (pthread_create( &ptDdnscId, 0, &ddnsC, 0 )!=0)
		return;
}
#endif


// Mason Yu On True
#ifdef SYSLOG_SUPPORT
#ifdef SEND_LOG
void startSendLogToServer()
{
	FILE *fp;
	unsigned char username[30], password[30];
	char ipaddr[20], tmpStr[5], tmpBuf[30];
	
	if ((fp = fopen("/var/ftpput.txt", "w")) == NULL)
	{
		printf("***** Open file /var/ftpput.txt failed !\n");
		return;
	}	
			
	if ( !mib_get(MIB_LOG_SERVER_IP, (void *)tmpStr)) {
		printf("Get LOG Server IP error!\n");
		return;
	}	
	strncpy(ipaddr, inet_ntoa(*((struct in_addr *)tmpStr)), 16);
	ipaddr[15] = '\0';
	//printf("ipaddr=%s\n", ipaddr);
	
	if ( !mib_get(MIB_LOG_SERVER_NAME, (void *)username)) {
		printf("Get user name for LOG Server IP error!\n");
		return;
	}
	//printf("username=%s\n", username);
	
	if ( !mib_get(MIB_LOG_SERVER_PASSWORD, (void *)password)) {
		printf("Get user name for LOG Server IP error!\n");
		return;
	}
	//printf("username=%s\n", password);
		
	fprintf(fp, "open %s\n", ipaddr);
	fprintf(fp, "user %s %s\n", username, password);	
	fprintf(fp, "lcd /var/log\n");
	fprintf(fp, "bin\n");	
	fprintf(fp, "put messages\n");
	fprintf(fp, "bye\n");
	fprintf(fp, "quit\n");
	fclose(fp);
	
	system("/bin/ftp -inv < /var/ftpput.txt");
	//va_cmd("/bin/ftp", 3, 0, "-inv", "-f", "/var/ftpput.txt");
	
	return;
}

void auto_startSYSLOGC(void)
{			
	//strcpy(g_ddns_ifname, "all");
	sem_post(&semSyslogC);
	signal(SIGALRM, auto_startSYSLOGC);
	alarm(AUTO_STARTSYSLOGC_INTERVAL);
}

static void log_syslogCpid()
{
	FILE *f;
	pid_t pid;
	char *pidfile = "/var/run/syslogC.pid";

	pid = getpid();
	printf("\nsyslogC=%d\n",pid);
	
	if((f = fopen(pidfile, "w")) == NULL)
		return;
	fprintf(f, "%d\n", pid);
	fclose(f);

}

void syslogC (void)
{
	unsigned int entryNum, i;
	char account[70];	
	MIB_CE_DDNS_T Entry;	
	

	/* Establish a handler for SIGALRM signals. */
	signal(SIGALRM, auto_startSYSLOGC);
	alarm(AUTO_STARTSYSLOGC_INTERVAL);	
	
	log_syslogCpid();
	
	signal(SIGUSR1, startSendLogToServer);
	
	// wait for user start Syslog Client
	while(1) {
		sem_wait(&semSyslogC);
		printf("Auto send syslog file to FTP Server\n");		
		startSendLogToServer();
	}	
}

void startSyslogC()
{
	pthread_t ptSyslogCId;
	if (pthread_create( &ptSyslogCId, 0, &syslogC, 0 )!=0)
		return;
}
#endif  // #ifdef SEND_LOG
#endif  // #ifdef SYSLOG_SUPPORT


int main(void)
{
	log_pid();
	init_signals();


#ifdef _CWMP_MIB_
	//restore
	if( va_cmd( "/bin/flatfsd",1,1,"-r" ) )
		printf( "[%d]:exec 'flatfsd -r' error!",__FILE__ );
#endif

/*
 *	Init MIB
 */
	if ( mib_init() == 0 ) {
		printf("[configd] Initialize MIB failed!\n");
//		error(E_L, E_LOG, T("Initialize MIB failed!\n"));
		return -1;
	}
	
	if ((msgqueue_id=initMsgQ()) < 0) {
		return -1;
	}

#ifdef CONFIG_USER_DDNS	
	// Added by Mason Yu
	// start DDNS Ctrl thread
	sem_init(&semDdnsc, 0, 0);
	startDdnsc();	
#endif
	
// Mason Yu on True
#ifdef SYSLOG_SUPPORT
#ifdef SEND_LOG
	sem_init(&semSyslogC, 0, 0);
	startSyslogC();
#endif
#endif
	msg_handler();
	printf("[configd]: should not be here !!\n");
	return 0;
}

