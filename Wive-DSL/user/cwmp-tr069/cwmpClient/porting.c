#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include "prmt_igd.h"
#include "prmt_ippingdiag.h"
#include "prmt_wancondevice.h"
#include "prmt_wandsldiagnostics.h"
#include "prmt_wanatmf5loopback.h"
#include "prmt_deviceinfo.h"
#ifdef _PRMT_TR143_
#include "prmt_tr143.h"
#endif //_PRMT_TR143_
#ifdef _SUPPORT_TRACEROUTE_PROFILE_
#include "prmt_traceroute.h"
#endif //_SUPPORT_TRACEROUTE_PROFILE_
#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
#include "prmt_captiveportal.h"
#endif //_SUPPORT_CAPTIVEPORTAL_PROFILE_
#include <config/autoconf.h>

#define CWMP_HTTP_REALM		"realtek.com.tw"
#define CONFIG_DIR		"/var/config"
/*notification*/
#define	NOTIFY_FILENAME		CONFIG_DIR"/CWMPNotify.txt"
#ifdef CONFIG_MIDDLEWARE
#define	MW_NOTIFY_FILENAME		CONFIG_DIR"/MWNotify.txt"
#endif
/*certificates*/
#define CA_FNAME		CONFIG_DIR"/cacert.pem"
#define CERT_FNAME		CONFIG_DIR"/client.pem"
#define DEF_CA_FN		"/etc/cacert.pem"
#define DEF_CERT_FN		"/etc/client.pem"

//ql
//#include "../../boa/src/LINUX/options.h"
#include <rtk/options.h>
#ifdef ENABLE_SIGNATURE_ADV
extern int upgrade;
#endif

/*here is the reason why to disable this: the mib-related APIs will handle this*/
#if 0
//xl_yue:close this macro for 8672
//jiunming, this code is shared by 8672 and 8671
#ifndef _LINUX_2_6_
//8671 case
#define __CLOSE_INTERFACE_BEFORE_WRITE_
#endif //_LINUX_2_6_
#endif //if 0

#ifdef CONFIG_MIDDLEWARE
#include <rtk/midwaredefs.h>

int sendOpertionDoneMsg2MidIntf(char opertion)
{
	int spid;
	FILE * spidfile;
	int msgid;
	int ret;
 	struct mwMsg sendMsg;
	char * sendBuf = sendMsg.msg_data;

	*sendBuf = OP_OperationDone;	/*Opcode*/
	*(sendBuf+1) = 1;				/*N*/
	*(sendBuf+2) = TYPE_Operation;	/*type*/
	W_WORD2CHAR((sendBuf+3), 1);	/*length*/
	*(sendBuf+5) = opertion;		/*value:'1'- PING;'2'-ATMF5Loopback;'3'-DSL*/

	msgid = msgget((key_t)1357,  0666);
	if(msgid <= 0){
		//fprintf(stdout,"get cwmp msgqueue error!\n");
		return -1;
	}

	/* get midware interface pid*/
	if ((spidfile = fopen(MW_INTF_RUNFILE, "r"))) {
		fscanf(spidfile, "%d\n", &spid);
		fclose(spidfile);
	}else{
		//fprintf(stdout,"midware interface pidfile not exists\n");
		return -1;
	}

	sendMsg.msg_type = spid;
	sendMsg.msg_datatype = PACKET_OK;
	if(msgsnd(msgid, (void *)&sendMsg, MW_MSG_SIZE, 0) < 0){
		fprintf(stdout,"send message to midwareintf error!\n");
		return -1;
	}

 	return 0;
}
#endif

/*********************************************************************/
/* utility */
/*********************************************************************/
/* copy from boa/src/LINUX/parser.c */
int upgradeFirmware( char *fwFilename )
{
	int rv;
	FILE *fp=NULL, *fp2=NULL;
	long filelen;
	char buf[10], output[256];
	
	CWMPDBG( 1, ( stderr, "<%s:%d>Ready to upgrade the new firmware\n", __FUNCTION__, __LINE__ ) );
	if (!(fp = fopen(fwFilename, "rb"))) 
	{
		CWMPDBG( 0, ( stderr, "<%s:%d>Image file open fail(%s)\n", __FUNCTION__, __LINE__, fwFilename ) );
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	filelen = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if (filelen <= 0)
	{		
		CWMPDBG( 0, ( stderr, "<%s:%d>Image file not exist\n", __FUNCTION__, __LINE__ ) );
		fclose(fp);
		return -1;
	}	
	fclose(fp);

	//check the image header
	if (!cmd_check_image(fwFilename,0)) //return 0:error, others:success
	{
		CWMPDBG( 0, ( stderr, "<%s:%d>Image Checksum Failed!\n", __FUNCTION__, __LINE__ ) );
	   	return -1;
	}

	if( cmd_upload( fwFilename,0 )==0 ) return -1;

	return 0;
}


/*********************************************************************/
/* porting functions */
/*********************************************************************/
void port_setuppid(void)
{
	int tr069_pid;
	unsigned char value[32];
	
	tr069_pid = (int)getpid();
	sprintf(value,"%d",tr069_pid);
	if (va_cmd("/bin/sarctl",2,1,"tr069_pid",value))
		printf("sarctl tr069_pid %s failed\n", value);
}	


int port_init_userdata( struct cwmp_userdata *data )
{
	char buf[256 + 1];
	unsigned char ch;
	unsigned int uVal;
	
	if(data)
	{
		memset( data, 0, sizeof( struct cwmp_userdata ) );
		
		//relative to SOAP header
		data->ID = 1;
		data->HoldRequests = 0;
		data->NoMoreRequests = 0;
		data->CPE_MaxEnvelopes = 1;
		data->ACS_MaxEnvelopes = 1;
		
		//cwmp:fault
		data->FaultCode=0;

		//others
		data->RB_CommandKey = mib_get(CWMP_RB_COMMANDKEY, buf) ? strdup(buf) : NULL;
		data->SI_CommandKey = mib_get(CWMP_SI_COMMANDKEY, buf) ? strdup(buf) : NULL;
		data->Reboot = 0;
		data->FactoryReset = 0;
		
		
		//download
		data->DownloadState = DOWNLD_NONE;
		data->DownloadWay = DLWAY_NONE;
		data->DLCommandKey = mib_get(CWMP_DL_COMMANDKEY, buf) ? strdup(buf) : NULL;
		data->DLStartTime = mib_get(CWMP_DL_STARTTIME, &uVal) ? uVal : 0;
		data->DLCompleteTime = mib_get(CWMP_DL_COMPLETETIME, &uVal) ? uVal : 0;
		data->DLFaultCode = mib_get(CWMP_DL_FAULTCODE, &uVal) ? uVal : 0;

		//inform
		data->EventCode= mib_get(CWMP_INFORM_EVENTCODE, &uVal) ? uVal : 0;
		data->NotifyParameter=NULL;


		// andrew
		data->server_port = mib_get(CWMP_CONREQ_PORT, &uVal) ? uVal : 7547;
		if( mib_get(CWMP_CONREQ_PATH, &buf[1]) )
		{
			if( buf[1]!='/' )
			{
				buf[0]='/';
				data->server_path = strdup( buf );
			}else
				data->server_path = strdup( &buf[1] );
		}else{
			data->server_path =  strdup("/");
		}
		data->url = mib_get(CWMP_ACS_URL, buf) ? strdup(buf) : strdup("");
		data->username = mib_get(CWMP_ACS_USERNAME, buf) ? strdup(buf) : NULL;
		data->password = mib_get(CWMP_ACS_PASSWORD, buf) ? strdup(buf) : NULL;

		data->conreq_username = mib_get(CWMP_CONREQ_USERNAME, buf) ? strdup(buf) : NULL;
		data->conreq_password = mib_get(CWMP_CONREQ_PASSWORD, buf) ? strdup(buf) : NULL;

		data->PeriodicInform = mib_get(CWMP_INFORM_ENABLE, &ch) ? ch : 1;
		data->InformInterval = mib_get(CWMP_INFORM_INTERVAL, &uVal) ? uVal : 60;
		data->InformTime = mib_get(CWMP_INFORM_TIME, &uVal) ? uVal : 0;

		//use the wan ip address as realm??
		data->realm = CWMP_HTTP_REALM;		

		//client certificate passowrd
		data->cert_passwd = mib_get( CWMP_CERT_PASSWORD, buf ) ? strdup(buf) : NULL;
		{
			struct stat file_stat;
			
			if( stat( CERT_FNAME, &file_stat )<0 )
				data->cert_path = strdup(DEF_CERT_FN);
			else
				data->cert_path = strdup(CERT_FNAME);

			if( stat( CA_FNAME, &file_stat )<0 )
				data->ca_cert = strdup(DEF_CA_FN);
			else
				data->ca_cert = strdup(CA_FNAME);

		}
		
#ifdef CONFIG_MIDDLEWARE
		mib_get(CWMP_TR069_ENABLE,(void *)&ch);
		if(ch == 0){	/*midware enabled*/
			data->notify_filename = strdup( MW_NOTIFY_FILENAME );
		}else
#endif
		data->notify_filename = strdup( NOTIFY_FILENAME );
		data->url_changed = 0;
		
		/*china-telecom has a extension for inform message, X_OUI_AssocDevice*/
		/*0: diable this field, 1:enable this filed*/
#ifdef _INFORM_EXT_FOR_X_CT_
		{
			unsigned char cwmp_flag=0;

			data->inform_ct_ext = 0; 
			if( mib_get( CWMP_FLAG, (void *)&cwmp_flag ) )
			{
				if( cwmp_flag & CWMP_FLAG_CTINFORMEXT )
					data->inform_ct_ext = 1; 
			}
		}
#else
		data->inform_ct_ext = 0; 
#endif //_INFORM_EXT_FOR_X_CT_
	}
	
	return 0;
}


int port_update_userdata( struct cwmp_userdata *data, int is_discon )
{
	char buf[256 + 1];
	unsigned char ch, *pnew, *pold;
	unsigned int vUint=0;

	if( mib_get(CWMP_CONREQ_USERNAME, buf)!=0 )
	{
		if( (data->conreq_username==NULL) ||
		    (strcmp(data->conreq_username,buf)!=0) )
		{
			pnew = strdup(buf);
			pold = data->conreq_username;
			data->conreq_username = pnew;
			if(pold) free(pold);
		}
	}
	
	if( mib_get(CWMP_CONREQ_PASSWORD, buf)!=0 )
	{
		if( (data->conreq_password==NULL) ||
		    (strcmp(data->conreq_password,buf)!=0) )
		{
			pnew = strdup(buf);
			pold = data->conreq_password;
			data->conreq_password = pnew;
			if(pold) free(pold);
		}
	}

#if defined(FINISH_MAINTENANCE_SUPPORT) || defined(CONFIG_MIDDLEWARE)
	vUint=0;
	if( mib_get(CWMP_INFORM_EVENTCODE, &vUint)!=0 )
	{
		if( vUint & EC_X_CT_COM_ACCOUNT )
		{
	//		fprintf( stderr, "get EC_X_CT_COM_ACCOUNT\n" );
			cwmpSendEvent( EC_X_CT_COM_ACCOUNT );
			vUint = vUint&(~EC_X_CT_COM_ACCOUNT);
			mib_set(CWMP_INFORM_EVENTCODE, &vUint);
		}
	}
#endif

	//update the acs url/username/password only when the connection is disconnected
	if(is_discon)
	{
		if( mib_get(CWMP_ACS_URL, buf)!=0 )
		{
			if( (data->url==NULL) ||
			    (strcmp(data->url,buf)!=0) )
			{
				pnew = strdup(buf);
				pold = data->url;
				data->url = pnew;
				if(pold) free(pold);
				//reset something??
				data->url_changed = 1;
			}
		}
	
		if( mib_get(CWMP_ACS_USERNAME, buf)!=0 )
		{
			if( (data->username==NULL) ||
			    (strcmp(data->username,buf)!=0) )
			{
				pnew = strdup(buf);
				pold = data->username;
				data->username = pnew;
				if(pold) free(pold);
			}
		}
	
		if( mib_get(CWMP_ACS_PASSWORD, buf)!=0 )
		{
			if( (data->password==NULL) ||
			    (strcmp(data->password,buf)!=0) )
			{
				pnew = strdup(buf);
				pold = data->password;
				data->password = pnew;
				if(pold) free(pold);
			}
		}
	}

	return 0;
}

void port_save_reboot( struct cwmp_userdata *user, int reboot_flag )
{
	unsigned char vChar=0;

	if( user )
	{
		//reboot commandkey
		if(user->RB_CommandKey)
			mib_set( CWMP_RB_COMMANDKEY, user->RB_CommandKey );
		else
			mib_set( CWMP_RB_COMMANDKEY, "" );

		//scheduleinform commandkey
		if(user->SI_CommandKey)
			mib_set( CWMP_SI_COMMANDKEY, user->SI_CommandKey );
		else
			mib_set( CWMP_SI_COMMANDKEY, "" );
	
		//related to download
		if(user->DLCommandKey)
			mib_set( CWMP_DL_COMMANDKEY, user->DLCommandKey );
		else
			mib_set( CWMP_DL_COMMANDKEY, "" );
				
		mib_set( CWMP_DL_STARTTIME, &user->DLStartTime );
		mib_set( CWMP_DL_COMPLETETIME, &user->DLCompleteTime );
		mib_set( CWMP_DL_FAULTCODE, &user->DLFaultCode );

		//inform
		mib_set( CWMP_INFORM_EVENTCODE, &user->EventCode );
	}



	//fprintf( stderr, "<%s:%d>\n", __FUNCTION__, __LINE__ );fflush(stderr);
#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
	/*to avoid the dead when wrriting the flash*/
	itfcfg("sar", 0);
	itfcfg("eth0", 0);
#ifdef WLAN_SUPPORT
	itfcfg("wlan0", 0);
#endif
#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/

	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
	
	if(reboot_flag==0)
	{
#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_

		itfcfg("sar", 1);
		vChar=1;
		mib_get(CWMP_LAN_ETHIFENABLE, (void *)&vChar);
		if(vChar) itfcfg("eth0", 1);
#ifdef WLAN_SUPPORT
		vChar=0;
		mib_get( MIB_WLAN_DISABLED, (void *)&vChar);
		if( vChar==0 )	itfcfg("wlan0", 1);
#endif //WLAN_SUPPORT
#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
	}
	//fprintf( stderr, "<%s:%d>\n", __FUNCTION__, __LINE__ );fflush(stderr);


	if(reboot_flag)
	{
		CWMPDBG( 0, ( stderr, "<%s:%d>The system is restarting ...\n", __FUNCTION__, __LINE__ ) );
		cmd_reboot();
		exit(0);
	}

}

void port_factoryreset_reboot(void)
{
	CWMPDBG( 3, ( stderr, "<%s:%d>\n", __FUNCTION__, __LINE__ ) );
	// Commented by Mason Yu. for not use default setting
	//mib_load(DEFAULT_SETTING, CONFIG_MIB_ALL);
	//va_cmd("/bin/flash", 2, 1, "default", "ds");
#ifdef CONFIG_BOA_WEB_E8B_CH
	va_cmd("/bin/flash", 3, 1, "default", "cs", "tr69");
#else
	va_cmd("/bin/flash", 2, 1, "default", "cs");
#endif


	CWMPDBG( 0, ( stderr, "<%s:%d>The system is restarting ...\n", __FUNCTION__, __LINE__ ) );
#ifdef CONFIG_BOA_WEB_E8B_CH
	sleep(5); //wait finishing setdefault configuration for 16M flash
#endif
	cmd_reboot();
	exit(0);
}

int port_before_download( int file_type, char *target )
{
	fprintf( stderr, "<%s:%d> file type:%d, target:%s\n", __FUNCTION__, __LINE__, file_type, target?target:"" );
	
	if(target==NULL) return -1;
	
	switch( file_type )
	{
	case DLTYPE_IMAGE:
		strcpy( target, "/tmp/vm.img" );
		break;
//	case DLTYPE_WEB: //not support right now
//		strcpy( target, "/tmp/web.bin" );
//		break;
	case DLTYPE_CONFIG:
		strcpy( target, "/tmp/config.xml" );
		break;
	}
//ql
#ifndef CONFIG_MIDDLEWARE
#ifndef ENABLE_SIGNATURE_ADV
	cmd_killproc(ALL_PID & ~(1<<PID_CWMP));	
#endif
#endif
	return 0;
}

int port_after_download( int file_type, char *target )
{
	
	switch(file_type)
	{
	case DLTYPE_IMAGE:
		//update firmware
#ifdef ENABLE_SIGNATURE_ADV
		if (upgrade == 2) {
#endif
			if( upgradeFirmware( target ) ) //return 0: success
			{
				CWMPDBG( 0, ( stderr, "<%s:%d>Image Checksum Failed!\n", __FUNCTION__, __LINE__ ) );
				return ERR_9010; 
			}
#ifdef ENABLE_SIGNATURE_ADV
		}
#endif
		break;
	case DLTYPE_CONFIG:
		if( va_cmd("/bin/LoadxmlConfig",0,1) )
		{
			fprintf( stderr, "exec /bin/LoadxmlConfig error!\n" );
			return ERR_9002; 
		}
#ifdef CONFIG_BOA_WEB_E8B_CH
		FILE *fpin;
		int filelen;
		fpin=fopen(target,"r");
		if(fpin>0){
			fseek(fpin, 0, SEEK_END);
			filelen = ftell(fpin);
			fseek(fpin, 0, SEEK_SET);
			fclose(fpin);
			if(filelen<=0)
				return ERR_9002;
		}else
			return ERR_9002;
#endif
		break;
	}
	return 0;
}


int port_before_upload( int file_type, char *target )
{
	if( target==NULL ) return -1;

	switch(file_type)
	{
	case DLTYPE_CONFIG:
		strcpy( target, "/tmp/config.xml" );
		if( va_cmd("/bin/CreatexmlConfig",0,1) )
		{
			fprintf( stderr, "exec /bin/CreatexmlConfig error!\n" );
		}
		break;
	case DLTYPE_LOG:
		//ifndef CONFIG_USER_BUSYBOX_SYSLOGD, send empty http put(content-length=0)
#ifdef CONFIG_BOA_WEB_E8B_CH
		strcpy( target, "/var/config/syslogd.txt" );
#else
		strcpy( target, "/var/log/messages" );
#endif
		break;
	}

	fprintf( stderr, "<%s:%d> file type:%d, target:%s\n", __FUNCTION__, __LINE__, file_type, target?target:"" );

	return 0;
}


int port_after_upload( int file_type, char *target )
{
	if( target==NULL ) return -1;

	//remove the target file
	switch(file_type)
	{
	case DLTYPE_CONFIG:
		if( strlen(target) )	remove( target );
		break;
	case DLTYPE_LOG:
		//not have to remove the log file
		break;
	}

	return 0;	
}

int port_notify_save( char *name )
{
	if( va_cmd( "/bin/flatfsd",1,1,"-s" ) )
		CWMPDBG( 0, ( stderr, "<%s:%d>exec 'flatfsd -s' error!\n", __FUNCTION__, __LINE__ ) );

	return 0;
}

int port_session_closed(struct cwmp_userdata *data)
{
	unsigned int delay=3;

	if (gStartPing)
	{
#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		/*delay some seconds to ready*/
		{while(delay!=0) delay=sleep(delay);}
#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
		cwmpStartPingDiag();
		gStartPing = 0;
	}else if(gStartDSLDiag)
	{
		cwmpSetCpeHold( 1 );
		cwmpStartDSLDiag();
		gStartDSLDiag=0;
	}else if(gStartATMF5LB)
	{
		cwmpStartATMF5LB();
		gStartATMF5LB=0;
#ifdef _PRMT_TR143_
	}else if(gStartTR143DownloadDiag)
	{
#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		/*delay some seconds to ready*/
		{while(delay!=0) delay=sleep(delay);}
#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
		StartTR143DownloadDiag();
		gStartTR143DownloadDiag=0;
	}else if(gStartTR143UploadDiag)
	{
#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		/*delay some seconds to ready*/
		{while(delay!=0) delay=sleep(delay);}
#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
		StartTR143UploadDiag();
		gStartTR143UploadDiag=0;
#endif //_PRMT_TR143_
#ifdef _SUPPORT_TRACEROUTE_PROFILE_
        }else if(gStartTraceRouteDiag)
	{
	#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		/*delay some seconds to ready*/
		{while(delay!=0) delay=sleep(delay);}
	#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
		StartTraceRouteDiag();
		gStartTraceRouteDiag=0;
#endif //_SUPPORT_TRACEROUTE_PROFILE_
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
        }else if(gStartReset)
	{
	#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		/*delay some seconds to ready*/
		{while(delay!=0) delay=sleep(delay);}
	#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
		cwmpStartReset();
		gStartReset=0;
#endif
/*ping_zhang:20081217 END*/
	}

	unlink(CONFIG_FILE_NAME);
#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
	unlink(FILE4CaptivePortal);
#endif //_SUPPORT_CAPTIVEPORTAL_PROFILE_
	
	//update the userdata
	
	return 0;
}

int port_backup_config( void )
{
	int ret=0;
	fprintf( stderr, "<%s:%d>\n", __FUNCTION__, __LINE__ );fflush(stderr);
#if 1
	ret = mib_backup(CONFIG_MIB_ALL);
	//fprintf( stderr, "<%s:%d>mib_backup return: %d\n", __FUNCTION__, __LINE__, ret );fflush(stderr);	
#else
	ret = mib_backup(CONFIG_MIB_TABLE);
	//fprintf( stderr, "<%s:%d>mib_backup return: %d\n", __FUNCTION__, __LINE__, ret );fflush(stderr);	
	ret = mib_backup(CONFIG_MIB_CHAIN);
	//fprintf( stderr, "<%s:%d>mib_backup return: %d\n", __FUNCTION__, __LINE__, ret );fflush(stderr);	
#endif
	return 0;
}

int port_restore_config( void )
{
	int ret=0;
	fprintf( stderr, "<%s:%d>\n", __FUNCTION__, __LINE__ );fflush(stderr);
#if 1
	ret=mib_restore(CONFIG_MIB_ALL);
	//fprintf( stderr, "<%s:%d>mib_restore return: %d\n", __FUNCTION__, __LINE__, ret );fflush(stderr);	
#else
	ret=mib_restore(CONFIG_MIB_TABLE);
	//fprintf( stderr, "<%s:%d>mib_restore return: %d\n", __FUNCTION__, __LINE__, ret );fflush(stderr);	
	ret=mib_restore(CONFIG_MIB_CHAIN);
	//fprintf( stderr, "<%s:%d>mib_restore return: %d\n", __FUNCTION__, __LINE__, ret );fflush(stderr);	
#endif
	return 0;
}

#ifdef CONFIG_BOA_WEB_E8B_CH
int isTR069(char *name)
{
	unsigned int devnum,ipnum,pppnum;
	MIB_CE_ATM_VC_T Entry;
	unsigned int total;
	int i;

	devnum = getWANConDevInstNum( name );
	ipnum  = getWANIPConInstNum( name );
	pppnum = getWANPPPConInstNum( name );

	total = mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0;i<total;i++){
		if(mib_chain_get(MIB_ATM_VC_TBL,i,&Entry)!=1)
			continue;
		if(Entry.ConDevInstNum==devnum 
			&& Entry.ConIPInstNum==ipnum 
			&& Entry.ConPPPInstNum==pppnum
			&& (Entry.applicationtype==0 || Entry.applicationtype==2)){
			return 1;
		}	
	}

	return 0;

}
int isINTERNET(char *name)
{
	unsigned int devnum,ipnum,pppnum;
	MIB_CE_ATM_VC_T Entry;
	unsigned int total;
	int i;

	devnum = getWANConDevInstNum( name );
	ipnum  = getWANIPConInstNum( name );
	pppnum = getWANPPPConInstNum( name );

	total = mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0;i<total;i++){
		if(mib_chain_get(MIB_ATM_VC_TBL,i,&Entry)!=1)
			continue;
		if(Entry.ConDevInstNum==devnum 
			&& Entry.ConIPInstNum==ipnum 
			&& Entry.ConPPPInstNum==pppnum
			&& (Entry.applicationtype==0 || Entry.applicationtype==1)){
			return 1;
		}	
	}

	return 0;


}

#endif

