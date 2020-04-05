#include <signal.h>
#include "prmt_igd.h"
#ifdef CONFIG_MIDDLEWARE
#include <rtk/midwaredefs.h>
#endif

void cwmp_show_help( void )
{
	fprintf( stderr, "cwmpClient:\n" );
	fprintf( stderr, "	-SendGetRPC:	send GetRPCMethods to ACS\n" );
	fprintf( stderr, "	-SSLAuth:	ACS need certificate the CPE\n" );
	fprintf( stderr, "	-SkipMReboot:	do not send 'M Reboot' event code\n" );
	fprintf( stderr, "	-Delay: 	delay some seconds to start\n" );
	fprintf( stderr, "	-NoDebugMsg: 	do not show debug message\n" );
	fprintf( stderr, "	-h or -help: 	show help\n" );
	fprintf( stderr, "\n" );
	fprintf( stderr, "	if no arguments, read the setting from mib\n" );
	fprintf( stderr, "\n" );
}

/*refer to climenu.c*/
#define CWMP_RUNFILE	"/var/run/cwmp.pid"
static void log_pid()
{
	FILE *f;
	pid_t pid;
	char *pidfile = CWMP_RUNFILE;

	pid = getpid();
	if((f = fopen(pidfile, "w")) == NULL)
		return;
	fprintf(f, "%d\n", pid);
	fclose(f);
}

static void clr_pid()
{
	FILE *f;
	char *pidfile = CWMP_RUNFILE;

	if((f = fopen(pidfile, "r")) != NULL){
		unlink(CWMP_RUNFILE);
		fclose(f);
	}
#ifdef CONFIG_MIDDLEWARE
	pidfile = CWMP_MIDPROC_RUNFILE;
	if((f = fopen(pidfile, "r")) != NULL){
		unlink(CWMP_MIDPROC_RUNFILE);
		fclose(f);
	}
#endif
}

void clear_child(int i)
{
	int status;
	pid_t chidpid;

	chidpid=wait( &status );
#ifdef _PRMT_TR143_
#ifdef CONFIG_USER_FTP_FTP_FTP
	if(chidpid!=-1)
		checkPidforFTPDiag( chidpid );
#endif //CONFIG_USER_FTP_FTP_FTP
#endif //_PRMT_TR143_
#ifdef _SUPPORT_TRACEROUTE_PROFILE_
	if(chidpid!=-1)
		checkPidforTraceRouteDiag( chidpid );
#endif //_SUPPORT_TRACEROUTE_PROFILE_
	return;
}

#if defined(FINISH_MAINTENANCE_SUPPORT) || defined(CONFIG_MIDDLEWARE)
void handle_x_ct_account()
{
	unsigned int vUint=0;
//	printf("\n%s\n",__FUNCTION__);
	if( mib_get(CWMP_INFORM_EVENTCODE, &vUint)!=0 )
	{
		vUint = vUint|(EC_X_CT_COM_ACCOUNT);
		mib_set(CWMP_INFORM_EVENTCODE, &vUint);
	}	
	return;
}
#endif //FINISH_MAINTENANCE_SUPPORT

void handle_term()
{
	clr_pid();
	exit(0);
}

void sigusr2_handler(int i)
{
	fprintf( stderr, "sigusr2_handler: pthread_exit for pid=%d\n", getpid() );
	pthread_exit(NULL);
}


int main(int argc, char **argv)
{
	//fprintf( stderr, "Start %s process\n", argv[0] );
#ifdef CONFIG_MIDDLEWARE
	unsigned char vChar;
	struct CWMP_NODE * selRoot;
#endif

	log_pid();

	/*Note: signal,SIGUSR2, may be used in the tr069 core library later, *
	 *      don't use the signal,SIGUSR2, for other purpose              */	
	signal( SIGUSR2,sigusr2_handler);
#ifndef CONFIG_MIDDLEWARE	
	signal( SIGCHLD, clear_child);	//set this signal process function according to CWMP_TR069_ENABLE below if MIDDLEWARE is defined
     #ifdef FINISH_MAINTENANCE_SUPPORT
	signal( SIGUSR1,handle_x_ct_account);
     #endif //FINISH_MAINTENANCE_SUPPORT
#endif
	signal( SIGTERM,handle_term);
	
	if( argc >= 2 )
	{
		int i;
		for(i=1;i<argc;i++)
		{
			if( strcmp( argv[i], "-SendGetRPC" )==0 )
			{
				cwmpinit_SendGetRPC(1);
				fprintf( stderr, "<%s>Send GetPRCMethods to ACS\n",__FILE__ );
			}else if( strcmp( argv[i], "-SSLAuth" )==0 )
			{
				cwmpinit_SSLAuth(1);
				fprintf( stderr, "<%s>Set using certificate auth.\n",__FILE__ );
			}else if( strcmp( argv[i], "-SkipMReboot" )==0 )
			{
				cwmpinit_SkipMReboot(1);
				fprintf( stderr, "<%s>Set skipping MReboot event code\n",__FILE__ );
			}else if( strcmp( argv[i], "-Delay" )==0 )
			{
				cwmpinit_DelayStart(30);
				fprintf( stderr, "<%s>Set Delay!\n", __FILE__ );
			}else if( strcmp( argv[i], "-NoDebugMsg" )==0 )
			{
				cwmpinit_NoDebugMsg(1);
				fprintf( stderr, "<%s>Set No Debug Message!\n", __FILE__ );
			}else if( strcmp( argv[i], "-h" )==0 || strcmp( argv[i], "-help" )==0 )
			{
				cwmp_show_help();
				exit(0);
			}else
			{
				fprintf( stderr, "<%s>Error argument: %s\n", __FILE__,argv[i] );
			}
		}
	}else{
		unsigned char cwmp_flag=0;
		//read the flag, CWMP_FLAG, from mib
		if ( mib_get( CWMP_FLAG, (void *)&cwmp_flag)!=0 )
		{
			printf("\ncwmp_flag=%x\n",cwmp_flag);
			if( (cwmp_flag&CWMP_FLAG_DEBUG_MSG)==0 )
			{
				fprintf( stderr, "<%s>Set No Debug Message!\n", __FILE__ );
				cwmpinit_NoDebugMsg(1);
			}
				
			if( cwmp_flag&CWMP_FLAG_CERT_AUTH )
			{
				fprintf( stderr, "<%s>Set using certificate auth.\n",__FILE__ );
				cwmpinit_SSLAuth(1);
			}
				
			if( cwmp_flag&CWMP_FLAG_SENDGETRPC )
			{
				fprintf( stderr, "<%s>Send GetPRCMethods to ACS\n",__FILE__ );
				cwmpinit_SendGetRPC(1);
			}
			
			if( cwmp_flag&CWMP_FLAG_SKIPMREBOOT )
			{
				fprintf( stderr, "<%s>Set skipping MReboot event code\n",__FILE__ );
				cwmpinit_SkipMReboot(1);
			}
				
			if( cwmp_flag&CWMP_FLAG_DELAY )
			{
				fprintf( stderr, "<%s>Set Delay!\n", __FILE__ );
				cwmpinit_DelayStart(30);
			}

			if( cwmp_flag&CWMP_FLAG_SELFREBOOT)
			{
				fprintf( stderr, "<%s>Set SelfReboot!\n", __FILE__ );
				cwmpinit_SelfReboot(1);
			}
				
		}

		if ( mib_get( CWMP_FLAG2, (void *)&cwmp_flag)!=0 )
		{
#ifdef _TR069_CONREQ_AUTH_SELECT_
			if( cwmp_flag&CWMP_FLAG2_DIS_CONREQ_AUTH)
			{
				fprintf( stderr, "<%s>Set DisConReqAuth!\n", __FILE__ );
				cwmpinit_DisConReqAuth(1);
			}
#endif

			if( cwmp_flag&CWMP_FLAG2_DEFAULT_WANIP_IN_INFORM)
				cwmpinit_OnlyDefaultWanIPinInform(1);
			else
				cwmpinit_OnlyDefaultWanIPinInform(0);

		}		
	}
	
//startRip();
	printf("\nenter cwmp_main!\n");

#ifdef CONFIG_MIDDLEWARE
	mib_get(CWMP_TR069_ENABLE,(void*)&vChar);
	if(vChar == 1){
		signal( SIGCHLD, clear_child);
		signal( SIGUSR1,handle_x_ct_account);
		selRoot = tROOT;	//start tr069
	}else{
		selRoot = mw_tROOT;	//start midware,mw_tROOT have tROOTAppendInfo
	}
	cwmp_main( selRoot );
#else
	cwmp_main( tROOT );
#endif

	return 0;
}
