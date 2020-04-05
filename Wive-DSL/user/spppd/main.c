
#include "if_sppp.h"
#include "pppoe.h"
#include "pppoa.h"
#include "signal.h"
#include "adslif.h"
#include <rtk/utility.h>


#define SPPPD_RUNFILE		"/var/run/spppd.pid"
#define N_SPPP 8

#ifdef CONFIG_NO_REDIAL
int  no_redial = 0;  
#endif

struct spppreq sprt;
struct pppoe_param_s poert;
struct pppoa_param_s poart;

// Kaohj, list of outstanding PPPoE request
//struct spppreq *sprt_list = NULL;
//int in_pppoe_disc = 0;

#define pppoatm_overhead(x) (x ? 6 : 2)

int add_cfg_ppp(char *arg)
{
	sprt.cmd = 0;
	sprt.ppp.unit = atoi(arg);
	sprt.ppp.debug = 0;
	sprt.ppp.lcp.mru = PP_MTU;
}

int del_cfg_ppp(char *arg)
{
	sprt.cmd = 1;
	sprt.ppp.unit = atoi(arg);
}

int show_cfg_ppp(char *arg)
{
	sprt.cmd = 2;
	sprt.ppp.unit = atoi(arg);
}

// Added by Jenny for PPP connect manually
int up_cfg_ppp(char *arg)
{
	sprt.cmd = 3;
	sprt.ppp.unit = atoi(arg);
}

// Added by Jenny for PPP disconnect manually
int down_cfg_ppp(char *arg)
{
	sprt.cmd = 4;
	sprt.ppp.unit = atoi(arg);
}

#ifdef PPP_QUICK_CONNECT
int qc_cfg_ppp(char *arg)
{
	sprt.cmd = 10;
	sprt.ppp.unit = atoi(arg);
}
#endif

// Added by Jenny for PPP connect manually
int new_cfg_ppp(char *arg)
{
	sprt.cmd = 5;
	sprt.ppp.unit = atoi(arg);
	sprt.ppp.debug = 0;
	sprt.ppp.lcp.mru = PP_MTU;
}

 

// Jenny, keepalive timer setting
int keepalive_cfg(char *arg)
{
	sprt.cmd = 6;
	if (atoi(arg)!=0) {
		keepalive_timer = atoi(arg);
#ifdef _CWMP_MIB_
		sppp_lcp_echo_log();
#endif
		return 0;
	}
	return -1;
}


// Added by Mason Yu for Remote Management
int set_rm_pppoe_test(char *arg)
{
	sprt.cmd = 7;
	sprt.ppp.unit = atoi(arg);
}

int reconnect_cfg_ppp(char *arg)
{
	sprt.cmd = 8;
	sprt.ppp.unit = atoi(arg);
}

int dev_pppoe(char *arg)
{
	FILE *fp;
	int in_turn=0;
	char	buff[64], tmp1[20];
	/* device is PPPoE */
	sprt.ppp.over = 1;
	sprt.dev = &poert;
	strcpy(poert.dev_name, arg);
	if (fp=fopen("/proc/net/atm/br2684", "r")) 
		while (fgets(buff, sizeof(buff), fp) != NULL) {
			if (in_turn==0) {
				sscanf(buff, "%*s%s", tmp1);
				tmp1[strlen(tmp1)-1]='\0';
				if (strcmp(arg, tmp1) != 0) {
					in_turn ^= 0x01;
					fgets(buff, sizeof(buff), fp);
				}
			}
			else {
				sscanf(buff, "%*s%s", tmp1);
				sscanf(tmp1, "0.%hu.%hu:", &poert.vpi, &poert.vci);
				break;
			}
			in_turn ^= 0x01;
		}
	fclose(fp);
	return 0;
}		

int dev_pppoa(char *arg)
{
	sprt.ppp.over = 0;
	/* device is PPPoA */
	sprt.dev = &poart;

	memset(&poart.addr, 0, sizeof(struct sockaddr_atmpvc));
	if (text2atm(arg, &poart.addr, sizeof(struct sockaddr_atmpvc), T2A_PVC | T2A_NAME) < 0)
		return -1;

	/* default value */
	poart.encaps = 1;
	memset(&poart.qos, 0, sizeof(struct atm_qos));
	poart.qos.txtp.traffic_class = poart.qos.rxtp.traffic_class = ATM_UBR;
	if (text2qos("ubr:pcr=7600", &poart.qos, 0)) {
		printf("Can't parse QoS: %s", arg);
		return -1;
	}
	poart.qos.txtp.max_sdu = 1500 + pppoatm_overhead(poart.encaps);
	poart.qos.rxtp.max_sdu = 1500 + pppoatm_overhead(poart.encaps);
	poart.qos.aal = ATM_AAL5;
		
	return 0;
}

int set_pppoa_encaps(char *arg)
{
	if(sprt.dev == &poart) {
		poart.encaps = atoi(arg);
		return 0;
	}
	return -1;
}	

int set_pppoa_qos(char *arg)
{
	if(sprt.dev == &poart) {
  		memset(&poart.qos, 0, sizeof(struct atm_qos));
  		poart.qos.txtp.traffic_class = poart.qos.rxtp.traffic_class = ATM_UBR;
   		if (text2qos(arg, &poart.qos, 0)) {
   			printf("Can't parse QoS: %s", arg);
   			return -1;
   		}
		poart.qos.txtp.max_sdu = 1500 + pppoatm_overhead(poart.encaps);
		poart.qos.rxtp.max_sdu = 1500 + pppoatm_overhead(poart.encaps);
		poart.qos.aal = ATM_AAL5;
		return 0;
	}
	return -1;
}

int set_username(char *arg)
{
	if(arg) {
		/* Authentication */
		strcpy(sprt.ppp.myauth.name, arg);
		return 0;
	}
	return -1;
}	

int set_password(char *arg)
{
	if(arg) {
		/* Authentication */
		strcpy(sprt.ppp.myauth.secret, arg);
		return 0;
	}
	return -1;
}	


int set_mode(char *arg) 
{	
	if(arg) {
		/* Direct/Auto */
		sprt.ppp.mode = atoi(arg);
		return 0;
	}
	return -1;
}

int set_mru(char *arg) 
{	
	if(arg) {
		/* our MRU */
		sprt.ppp.lcp.mru = atoi(arg);
		return 0;
	}
	return -1;
}

int set_default_gw(char *arg) 
{
int gw;
	if(arg) {
		gw = atoi(arg);
		if(gw == 1)
			sprt.ppp.dgw = 1;
		return 0;
	}
	return -1;
}

// Jenny, keepalive setting
int set_keepalive(char *arg)
{
	if (arg) {
		if (atoi(arg) == 1)
			sprt.ppp.diska = 1;
		return 0;
	}
	return -1;
}

int set_timeout(char *arg)
{
#ifdef AUTO_PPPOE_ROUTE
	unsigned char dgw;
#endif
	if(atoi(arg)!=0) {
		//if(sprt.ppp.dgw) {
#ifdef AUTO_PPPOE_ROUTE
		mib_get( MIB_ADSL_WAN_DGW_ITF, (void *)&dgw);	// Jenny, check default gateway
		if(dgw == DGW_AUTO || sprt.ppp.dgw) // Jenny, if set to auto(0xef)
#else
		if(sprt.ppp.dgw)
#endif
		{
			// Mason Yu
			// Because the sysytem send keepalive packets, every KEEPALIVE_INTERVAL(20) seconds
				sprt.ppp.timeout = atoi(arg) * 60;	// Jenny
		}else
			sprt.ppp.timeout = 0;
	}
	return -1;
	
}

int set_debug(char *arg) 
{
	int debug;
	if(arg) {
		debug = atoi(arg);
		if(debug == 1)
			sprt.ppp.debug = 1;
		return 0;
	}
	return -1;
}

// Added by Mason Yu for Half Bridge
int set_ippt_flag(char *arg)
{	
    int ippt_flag;
    //printf("Set IPPT Flag\n");
    
	if(arg) {
		ippt_flag = atoi(arg);
		if(ippt_flag == 1)
		    //printf("Set IPPT Flag OK\n");
			sprt.ppp.ippt_flag = 1;
#ifdef ZTE_GENERAL_ROUTER_SC
		else if(ippt_flag==2)  //alex_huang for ip unnumbered
			{
			  sprt.ppp.ippt_flag=2;
			}
#endif			
		return 0;
	}
	return -1;
		
}

int set_acname(char *arg)
{
	if(sprt.dev == &poert) {
		if(arg) {
			strcpy(poert.ACName, arg);
			return 0;
		}
	}
	return -1;
}	

int set_auth(char *arg)
{
	char *start=arg;
	
	if(arg) {
		while (*start) {
			*start = tolower(*start);
			start++;
		}
		if (strcmp("auto", arg)==0)
			sprt.ppp.myauth.proto = 0x0;
		else if (strcmp("chap", arg)==0)
			sprt.ppp.myauth.proto = PPP_CHAP;
		else if (strcmp("pap", arg)==0)
			sprt.ppp.myauth.proto = PPP_PAP;
		else
			return -1;
		
		return 0;
	}
	return -1;
}	


#ifdef _CWMP_MIB_
int set_servicename(char *arg)
{
	if(sprt.dev == &poert) {
		if(arg) {
			strcpy(poert.ServiceName, arg);
			return 0;
		}
	}
	return -1;
}

int set_disconnect(char *arg)
{
	if(atoi(arg)!=0)
		sprt.ppp.autoDisconnectTime = atoi(arg);
	else
		sprt.ppp.autoDisconnectTime = 0;
	return 0;
}

int set_delay(char *arg)
{
	if(atoi(arg)!=0)
		sprt.ppp.warnDisconnectDelay = atoi(arg);
	else
		sprt.ppp.warnDisconnectDelay = 0;
	return 0;
}
#endif

int set_debug2(char *arg) 
{
	sprt.cmd = 9;
	sprt.ppp.unit = atoi(arg);
}

#ifdef ENABLE_PPP_SYSLOG
// Jenny, debug message output setting
int set_syslog(char *arg)
{
	sprt.cmd = 12;
	dbg_syslog = atoi(arg);
}
#endif

#ifdef CONFIG_USER_PPPOE_PROXY   
int set_proxy(char *arg)
{       
	if(arg) {
		sprt.ppp.enable_pppoe_proxy= atoi(arg);
		printf("Enable_pppoe_proxy ++++= %d\n",sprt.ppp.enable_pppoe_proxy);
		return 0;
	}
	return -1;
   

}
#endif
#ifdef CONFIG_TR069_DNS_ISOLATION
int  set_tr069_dns(char *arg)
{
   if(arg){
   	sprt.ppp.isTr069_interface = atoi(arg);
	printf("*************set tr069 dns =%d .......\n",sprt.ppp.isTr069_interface);
	return 0;
   }
   return -1;
}

#endif

#ifdef  CONFIG_SPPPD_STATICIP
int set_staticip(char *arg)
{
	unsigned long  ourip  ;

	if (arg) {
		ourip = strtoul(arg, NULL, 16);
		sprt.ppp.ipcp.myipaddr = ourip;
		sprt.ppp.staticip = 1;		   
	}
}
                        
#endif

#ifdef CONFIG_NO_REDIAL
int  set_no_dial(char *arg)
{


   if(arg){
   	
   	   no_redial= atoi(arg);
	  printf("********no redial  == %d \n",no_redial);
	  sprt.cmd = 11; 	
   }
   

}
#endif



///////////////////////////////////////////////////////////////////////

struct command
{
	int		needs_arg;
	int		num_string_arg;
	char	*name;
	void	(*func)(char *arg);
};


#define SERVER_FIFO_NAME "/tmp/ppp_serv_fifo"
#define CLIENT_FIFO_NAME "/tmp/ppp_cli_fifo"
#if 0
#define BUFFER_SIZE	256

struct data_to_pass_st {
	int	id;
	char data[BUFFER_SIZE];
};
#endif


static struct command commands[] = {
	{0, 1, "show", show_cfg_ppp},
	{1, 1, "del", del_cfg_ppp},
	{0, 1, "add", add_cfg_ppp},
	{1, 1, "rec", reconnect_cfg_ppp},
	{0, 1, "down", down_cfg_ppp},	// Added by Jenny for PPP connecting/disconnecting manually
	{0, 1, "up", up_cfg_ppp},
#ifdef PPP_QUICK_CONNECT
	{0, 1, "qconnect", qc_cfg_ppp},
#endif	
	{0, 1, "new", new_cfg_ppp},
	{0, 1, "katimer", keepalive_cfg},	// Added by Jenny for keepalive timer setting
	{0, 1, "dbg", set_debug2},		// Jenny, for real time debugging
	{0, 1, "pppoa", dev_pppoa},
	{0, 1, "pppoe", dev_pppoe},
	{1, 1, "encaps", set_pppoa_encaps},
	{1, 1, "qos", set_pppoa_qos},
	{0, 1, "username", set_username},
	{0, 1, "password", set_password},
	{0, 1, "mode", set_mode},
	{0, 1, "mru", set_mru},
	{0, 1, "gw", set_default_gw},
	{0, 1, "diska", set_keepalive},	// Added by Jenny for keepalive setting
	{0, 1, "timeout", set_timeout},
	{0, 1, "debug", set_debug},
	{0, 1, "ippt", set_ippt_flag},   // Added by Mason Yu for Half Bridge
	{0, 1, "acname", set_acname},
	{1, 1, "auth", set_auth},
	{0, 1, "pppoetest", set_rm_pppoe_test},  // Added by Mason Yu
#ifdef _CWMP_MIB_
	{0, 1, "disctimer", set_disconnect},	// Jenny, set auto disconnect timer
	{0, 1, "discdelay", set_delay},	// Jenny, set disconnect delay time
	{0, 1, "servicename", set_servicename},	// Jenny, set service name
#endif
#ifdef ENABLE_PPP_SYSLOG
	{0, 1, "syslog", set_syslog},		// Jenny, show debug message to 1: syslog /0: console
#endif
#ifdef CONFIG_USER_PPPOE_PROXY
	{0, 1,"proxy", set_proxy},        //alex_huang for pppoe proxy
#endif
#ifdef CONFIG_TR069_DNS_ISOLATION   //alex_huang for tr069 interface e8b
        {0, 1,"tr069_dns", set_tr069_dns}, 

#endif
#ifdef CONFIG_SPPPD_STATICIP
	{ 0, 1, "staticip", set_staticip}, //alex_huang for static ip setting 
#endif	
#ifdef CONFIG_NO_REDIAL
       { 0,1,"noredial",set_no_dial},//alex
#endif
	{0, 0, NULL, NULL}
};

struct command *mpoa_command_lookup(char *arg)
{
	int i;
	for(i=0; commands[i].name!=NULL; i++)
		if(!strcmp(arg, commands[i].name))
			return (&commands[i]);
	return NULL;
}


int server_fifo_fd, client_fifo_fd;

int init_msg (void)
{
  	
	mkfifo(SERVER_FIFO_NAME, 0777);
	mkfifo(CLIENT_FIFO_NAME, 0777);

	server_fifo_fd = open(SERVER_FIFO_NAME, O_RDONLY);
	if (server_fifo_fd == -1) {
		fprintf(stderr, "Server fifo failure\n");
		return -1;
	}

#if 0
	client_fifo_fd = open(CLIENT_FIFO_NAME, O_WRONLY);
	if (client_fifo_fd == -1) {
		fprintf(stderr, "Client fifo failure\n");
		return -1;
	}
#endif

	return 0;
}

int poll_cc(int fd)
{
int bytesToRead = 0;
	(void)ioctl(fd, FIONREAD, (int)&bytesToRead);
	return bytesToRead;
}


#define MAX_ARGS	32
#define MAX_ARG_LEN	64

// Kaohj, put PPPoE request into pending list
/*
static void add_into_list(struct spppreq *req)
{
	struct spppreq *t_sprt, *psprt;
	struct pppoe_param_s *t_poert;
	
	t_sprt = malloc(sizeof(struct spppreq));
	memcpy(t_sprt, req, sizeof(struct spppreq));
	t_poert = malloc(sizeof(struct pppoe_param_s));
	memcpy(t_poert, req->dev, sizeof(struct pppoe_param_s));
	t_sprt->dev = (void *)t_poert;
	
	if (sprt_list == NULL) {
		sprt_list = t_sprt;
		t_sprt->next = NULL;
	}
	else {
		psprt = sprt_list;
		while (psprt->next != NULL)
			psprt = psprt->next;
		// add into list
		psprt->next = t_sprt;
		t_sprt->next = NULL;
	}
}

static void process_poe_list()
{
	struct spppreq *psprt;
	
	psprt = sprt_list;
	while (psprt != NULL) {
		//printf("process_poe_list: add_ppp()\n");
		add_ppp(psprt->ppp.unit, psprt);
		sprt_list = sprt_list->next;
		free(psprt->dev);
		free(psprt);
		psprt = sprt_list;
	}
}
*/

#ifdef EMBED
static void log_pid()
{
	FILE *f;
	pid_t pid;
	char *pidfile = SPPPD_RUNFILE;
	
	pid = getpid();
	if((f = fopen(pidfile, "w")) == NULL)
		return;
	fprintf(f, "%d\n", pid);
	fclose(f);
}
#endif

int recv_msg (void)
{
	struct data_to_pass_st 	msg;
	int	read_res;
	int i, c;
  	int argc;
	char argv[MAX_ARGS][MAX_ARG_LEN+1];
  	char *arg_ptr;
  	
  	if(poll_cc(server_fifo_fd)) {
		argc = 0;
		// Kaohj
		//do {
			read_res = read(server_fifo_fd, &msg, sizeof(msg));
			if(read_res > 0) {
				int arg_idx = 0;
				arg_ptr = msg.data;
				for(i=0; arg_ptr[i]!='\0'; i++) {
					if(arg_ptr[i]==' '){
						argv[argc][arg_idx]='\0';
						argc++;
						arg_idx=0;
					}
					else {
						if(arg_idx<MAX_ARG_LEN) {
							argv[argc][arg_idx]=arg_ptr[i];
							arg_idx++;
						}
					}
				}
				argv[argc][arg_idx]='\0';
			}
		// Kaohj
		//} while (read_res > 0);

		memset(&sprt, 0, sizeof sprt);
		memset(&poert, 0, sizeof poert);
		memset(&poart, 0, sizeof poart);
		sprt.cmd = -1;
              for(c=0; commands[c].name!=NULL; c++)
		   commands[c].needs_arg=0;
		

		for(c=0; commands[c].name!=NULL; c++) {
			for(i=1; i<argc; i++) {
				  if(!strcmp(argv[i],"password"))
				 {
				    set_password(argv[++i]);
				    continue;
				 }
				if(!strcmp(argv[i],"username"))
			        {     
			           set_username(argv[++i]);
					continue;
			        }
			        
				if(!strcmp(argv[i], commands[c].name)) {
					if(commands[c].num_string_arg)
						if(commands[c].needs_arg==0){
						commands[c].func(argv[i+1]);
						  commands[c].needs_arg=1;
						}
				}
			}
		}
		switch(sprt.cmd) {
		case 0:	/* add */
			// Kaohj
			//if (in_pppoe_disc && sprt.ppp.over == 1)
			//	add_into_list(&sprt);
			//else
			add_ppp(sprt.ppp.unit, &sprt);
			break;
		case 1:
			del_ppp(sprt.ppp.unit);
			break;
		case 2:
			/* for debug only */
			//show_ppp(sprt.ppp.unit);
			ppp_status();
			break;
		case 3:	// Jenny
			ppp_up(sprt.ppp.unit);
			break;
		case 4:	// Jenny
			ppp_down(sprt.ppp.unit);
			sppp_last_connection_error(sprt.ppp.unit, ERROR_USER_DISCONNECT);
			syslog(LOG_INFO, "spppd: %s: user disconnected\n", sprt.ppp.if_name);
			break;
		case 5:	// Jenny
			ppp_new(sprt.ppp.unit, &sprt);
			break;
		case 6:	// Jenny
			break;
		case 7:	// Mason Yu
			rm_pppoe_test(sprt.ppp.unit, &sprt);
			break;
		case 8:	// Jenny
			reconnect_ppp(sprt.ppp.unit);
			break;
		case 9:	// Jenny
			//debug_set(sprt.ppp.unit);
			debug_set(sprt.ppp.unit, &sprt);
			break;
#ifdef PPP_QUICK_CONNECT
		case 10:	// Jenny
			ppp_quick_connect(sprt.ppp.unit, sprt.ppp.myauth.name, sprt.ppp.myauth.secret);
			break;
#endif
             case 11:
			 printf("redial----\n");
			 break	 ;
#ifdef ENABLE_PPP_SYSLOG
		case 12: // Jenny
			syslog_set(dbg_syslog);
			break;
#endif
		default:
			printf("spppd command error!\n");
			break;
		}
	}

	return 0;
}	


void dial_start(int signum);

static char adslDevice[] = "/dev/adsl0";
static FILE* adslFp = NULL;
#if 0
static char open_adsl_drv(void)
{
	if ((adslFp = fopen(adslDevice, "r")) == NULL) {
		printf("ERROR: failed to open %s, error(%s)\n",adslDevice, strerror(errno));
		return 0;
	};
	return 1;
}

static void close_adsl_drv(void)
{
	if(adslFp)
		fclose(adslFp);

	adslFp = NULL;
}

char adsl_drv_get(unsigned int id, void *rValue, unsigned int len)
{
#ifdef EMBED
	if(open_adsl_drv()) {
		obcif_arg	myarg;
	    	myarg.argsize = (int) len;
	    	myarg.arg = (int) (rValue);

		if (ioctl(fileno(adslFp), id, &myarg) < 0) {
//	    	        printf("ADSL ioctl failed! id=%d\n",id );
			close_adsl_drv();
			return 0;
	       };

		close_adsl_drv();
		return 1;
	}
#endif
	return 0;
}
#endif

// Jenny, polling ADSL status
void poll_adslstatus(void)
{
	Modem_LinkSpeed vLs;
	vLs.upstreamRate=0;
	int i;

	if (!adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE) || vLs.upstreamRate == 0) {
			stop_interface();
	}
	else {
		if (!ppp_term_flag)
			sppp_term_send();	// send TERM-REQ to release existed PPP connection
		if (!adsl_PADT_flag)
			pre_release_pppoe();	// Jenny, send PADT packet to release existed PPPoE connection
	}
}

static void handle_pipe(int sig) {
	fprintf(stderr, "got epipe! %d\n", sig);
	return;
}

int main(void)
{
	struct sigaction act, actio;	
	// setup 
	act.sa_handler = handle_pipe;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGPIPE, &act, 0);

	act.sa_handler = handle_pipe;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT, &act, 0);

	act.sa_handler = handle_pipe;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGQUIT, &act, 0);

	act.sa_handler = handle_pipe;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGHUP, &act, 0);

#ifdef QSETUP_WEB_REDIRECT
//	unsigned char startRedirect = 0;
//	if (!mib_set(MIB_QSETUP_REDIRECT, (void *)&startRedirect))
//		printf("Set quick setup redirect flag error!");
	int startRedirect = 0;
	FILE *fp = fopen("/tmp/QSetup", "w");
	fprintf(fp, "%d", startRedirect);
	fclose(fp);
#endif

	init_msg();
	init_ppp_unit();
	
	/* update ppp status files */
	ppp_status();

	signal(SIGUSR1, dial_start);  //for trap
	signal(SIGUSR2, ppp_status);	// Jenny, update PPP status file
#ifdef EMBED
	log_pid();
#endif
	
	while(1) {
		recv_msg();
		// Kaohj
		// process the outstanding commands for PPPoE
		//process_poe_list();
		process_poe();
		calltimeout();
		sppp_recv();
		poll_adslstatus();
	}
}

