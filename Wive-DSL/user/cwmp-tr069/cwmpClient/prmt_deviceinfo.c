#include "prmt_deviceinfo.h"
#include <sys/sysinfo.h>
#ifdef _PRMT_X_CT_EXT_ENABLE_
#include "prmt_ctcom.h"
#endif //_PRMT_X_CT_EXT_ENABLE_
#ifdef _PRMT_X_CT_COM_ALG_
#include "../../../uClibc/include/linux/autoconf.h"
#endif
//#include "../../boa/src/LINUX/options.h"
#include <rtk/options.h>
#include <config/autoconf.h>

#ifdef CONFIG_MIDDLEWARE
#define MANUFACTURER_STR	"REALTEK"
#else 
#if defined(ZTE_531b_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
#define MANUFACTURER_STR	"ZTE."
#else
#define MANUFACTURER_STR	"REALTEK SEMICONDUCTOR CORP."
#endif
#endif
#define MANUFACTUREROUI_STR	"00E04C"
#define SPECVERSION_STR		"1.0"
#define HWVERSION_STR		"8671x"



struct CWMP_OP tVendorCfgEntityLeafOP = { getVendorCfgEntity, NULL };
struct CWMP_PRMT tVendorCfgEntityLeafInfo[] =
{
/*(name,	type,		flag,		op)*/
{"Name",	eCWMP_tSTRING,	CWMP_READ,	&tVendorCfgEntityLeafOP},
{"Version",	eCWMP_tSTRING,	CWMP_READ,	&tVendorCfgEntityLeafOP},
{"Date",	eCWMP_tDATETIME,CWMP_READ,	&tVendorCfgEntityLeafOP},
{"Description",	eCWMP_tSTRING,	CWMP_READ,	&tVendorCfgEntityLeafOP},
};
enum eVendorCfgEntityLeaf
{
	eVCName,
	eVCVersion,
	eVCDate,
	eDescription
};
struct CWMP_LEAF tVendorCfgEntityLeaf[] =
{
{ &tVendorCfgEntityLeafInfo[eVCName] },
{ &tVendorCfgEntityLeafInfo[eVCVersion] },
{ &tVendorCfgEntityLeafInfo[eVCDate] },
{ &tVendorCfgEntityLeafInfo[eDescription] },
{ NULL }
};


struct CWMP_PRMT tVendorConfigObjectInfo[] =
{
/*(name,	type,		flag,		op)*/
{"1",		eCWMP_tOBJECT,	CWMP_READ,	NULL}
};
enum eVendorConfigObject
{
	eVC1
};
struct CWMP_NODE tVendorConfigObject[] =
{
/*info,  				leaf,			next)*/
{&tVendorConfigObjectInfo[eVC1],	tVendorCfgEntityLeaf, 	NULL},
{NULL, 					NULL, 			NULL}
};


struct CWMP_OP tDeviceInfoLeafOP = { getDeviceInfo, setDeviceInfo };
struct CWMP_PRMT tDeviceInfoLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Manufacturer",		eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
{"ManufacturerOUI",		eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
{"ModelName",			eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
{"Description",			eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
{"ProductClass",		eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
{"SerialNumber",		eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
{"HardwareVersion",		eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
{"SoftwareVersion",		eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
{"ModemFirmwareVersion",	eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
{"EnabledOptions",		eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
/*AdditionalHardwareVersion*/
/*AdditionalSoftwareVersion*/
{"SpecVersion",			eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
{"ProvisioningCode",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tDeviceInfoLeafOP},
{"UpTime",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tDeviceInfoLeafOP},
{"FirstUseDate",		eCWMP_tDATETIME,CWMP_READ,		&tDeviceInfoLeafOP},
{"DeviceLog",			eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,&tDeviceInfoLeafOP},
{"VendorConfigFileNumberOfEntries",eCWMP_tUINT,	CWMP_READ,		&tDeviceInfoLeafOP}
};
enum eDeviceInfoLeaf
{
	eDIManufacturer,
	eDIManufacturerOUI,
	eDIModelName,
	eDIDescription,
	eDIProductClass,
	eDISerialNumber,
	eDIHardwareVersion,
	eDISoftwareVersion,
	eDIModemFirmwareVersion,
	eDIEnabledOptions,
	eDISpecVersion,
	eDIProvisioningCode,
	eDIUpTime,
	eDIFirstUseDate,
	eDIDeviceLog,
	eDIVendorConfigFileNumberOfEntries
};
struct CWMP_LEAF tDeviceInfoLeaf[] =
{
{ &tDeviceInfoLeafInfo[eDIManufacturer] },
{ &tDeviceInfoLeafInfo[eDIManufacturerOUI] },
{ &tDeviceInfoLeafInfo[eDIModelName] },
{ &tDeviceInfoLeafInfo[eDIDescription] },
{ &tDeviceInfoLeafInfo[eDIProductClass] },
{ &tDeviceInfoLeafInfo[eDISerialNumber] },
{ &tDeviceInfoLeafInfo[eDIHardwareVersion] },
{ &tDeviceInfoLeafInfo[eDISoftwareVersion] },
{ &tDeviceInfoLeafInfo[eDIModemFirmwareVersion] },
{ &tDeviceInfoLeafInfo[eDIEnabledOptions] },
{ &tDeviceInfoLeafInfo[eDISpecVersion] },
{ &tDeviceInfoLeafInfo[eDIProvisioningCode] },
{ &tDeviceInfoLeafInfo[eDIUpTime] },
{ &tDeviceInfoLeafInfo[eDIFirstUseDate] },
{ &tDeviceInfoLeafInfo[eDIDeviceLog] },
{ &tDeviceInfoLeafInfo[eDIVendorConfigFileNumberOfEntries] },
{ NULL	}
};
struct CWMP_PRMT tDeviceInfoObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"VendorConfigFile",		eCWMP_tOBJECT,	CWMP_READ,		NULL},
#ifdef _PRMT_X_CT_COM_ACCOUNT_
{"X_CT-COM_TeleComAccount",	eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#ifdef _PRMT_X_CT_COM_ALG_
{"X_CT-COM_ALGAbility",		eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#ifdef _PRMT_X_CT_COM_RECON_
{"X_CT-COM_ReConnect",		eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#ifdef _PRMT_X_CT_COM_PORTALMNT_
{"X_CT-COM_PortalManagement",	eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#ifdef _PRMT_X_CT_COM_SRVMNG_
{"X_CT-COM_ServiceManage",	eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#ifdef _PRMT_X_CT_COM_SYSLOG_
{"X_CT-COM_Syslog",	eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#ifdef CONFIG_MIDDLEWARE
{"X_CT-COM_MiddlewareMgt",	eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
};
enum eDeviceInfoObject
{
	eDIVendorConfigFile,
#ifdef _PRMT_X_CT_COM_ACCOUNT_
	eDIX_CTCOM_TeleComAccount,
#endif
#ifdef _PRMT_X_CT_COM_ALG_
	eDIX_CTCOM_ALGAbility,
#endif
#ifdef _PRMT_X_CT_COM_RECON_
	eDIX_CTCOM_ReConnect,
#endif
#ifdef _PRMT_X_CT_COM_PORTALMNT_
	eDIX_CTCOM_PortalManagement,
#endif
#ifdef _PRMT_X_CT_COM_SRVMNG_
	eDIX_CTCOM_ServiceManage,
#endif
#ifdef _PRMT_X_CT_COM_SYSLOG_
	eDIX_CTCOM_Syslog,
#endif
#ifdef CONFIG_MIDDLEWARE
	eDIX_CTCOM_MiddlewareMgt
#endif
};
struct CWMP_NODE tDeviceInfoObject[] =
{
/*info,  						leaf,		next)*/
{ &tDeviceInfoObjectInfo[eDIVendorConfigFile],		NULL,		tVendorConfigObject },
#ifdef _PRMT_X_CT_COM_ACCOUNT_
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_TeleComAccount],	tCTAccountLeaf,	NULL },
#endif
#ifdef _PRMT_X_CT_COM_ALG_
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_ALGAbility],	tXCTCOMALGLeaf,	NULL },
#endif
#ifdef _PRMT_X_CT_COM_RECON_
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_ReConnect],		tCT_ReConLeaf,	NULL },
#endif
#ifdef _PRMT_X_CT_COM_PORTALMNT_
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_PortalManagement],	tCT_PortalMNTLeaf,NULL },
#endif
#ifdef _PRMT_X_CT_COM_SRVMNG_
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_ServiceManage],	tCTServiceLeaf,	NULL },
#endif
#ifdef _PRMT_X_CT_COM_SYSLOG_
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_Syslog],	tCT_SyslogLeaf,	NULL },
#endif
#ifdef CONFIG_MIDDLEWARE
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_MiddlewareMgt],	tCT_MiddlewareMgtLeaf,	NULL },
#endif
{ NULL,							NULL,		NULL }
};


#ifdef _PRMT_DEVICECONFIG_
struct CWMP_OP tDeviceConfigLeafOP = { getDeviceConfig, setDeviceConfig };
struct CWMP_PRMT tDeviceConfigLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"PersistentData",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tDeviceConfigLeafOP},
{"ConfigFile",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tDeviceConfigLeafOP}
};
enum eDeviceConfigLeaf
{
	eDCPersistentData,
	eDCConfigFile
};
struct CWMP_LEAF tDeviceConfigLeaf[] =
{
{ &tDeviceConfigLeafInfo[eDCPersistentData] },
{ &tDeviceConfigLeafInfo[eDCConfigFile] },
{ NULL	}
};
#endif //_PRMT_DEVICECONFIG_

int getVendorCfgEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char buf[256]={0};
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Name" )==0 )
	{
		*data = strdup( "config.xml" );
	}else if( strcmp( lastname, "Version" )==0 )
	{
		getSYS2Str( SYS_FWVERSION, buf );
		*data = strdup( buf ); /*use the software version as config version*/
	}else if( strcmp( lastname, "Date" )==0 )
	{
		*data = timedup( 0 );//unknown time
	}else if( strcmp( lastname, "Description" )==0 )
	{
		*data = strdup( "" ); 
	}else{
		return ERR_9005;
	}
	
	return 0;
}


int getDeviceInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char buf[256]={0};
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Manufacturer" )==0 )
	{
		*data = strdup( MANUFACTURER_STR );
	}else if( strcmp( lastname, "ManufacturerOUI" )==0 )
	{
		*data = strdup( MANUFACTUREROUI_STR );
	}else if( strcmp( lastname, "ModelName" )==0 )
	{
		mib_get( MIB_SNMP_SYS_NAME, (void *)buf);
		*data = strdup( buf );
	}
	else if( strcmp( lastname, "Description" )==0 )
	{
		mib_get( MIB_SNMP_SYS_DESCR, (void *)buf);
		*data = strdup( buf );
	}
	else if( strcmp( lastname, "ProductClass" )==0 )
	{
		*data = strdup( "IGD" );
	}else if( strcmp( lastname, "SerialNumber" )==0 )
	{
#ifdef CONFIG_BOA_WEB_E8B_CH
		mib_get( CWMP_SERIALNUMBER, (void *)buf);
#else
		mib_get( MIB_HW_SERIAL_NUMBER, (void *)buf);
#endif
		*data = strdup( buf );
	}else if( strcmp( lastname, "HardwareVersion" )==0 )
	{
		*data = strdup( HWVERSION_STR );
	}else if( strcmp( lastname, "SoftwareVersion" )==0 )
	{
		getSYS2Str( SYS_FWVERSION, buf );
		*data = strdup( buf );
	}else if( strcmp( lastname, "ModemFirmwareVersion" )==0 )
	{
		getAdslInfo( ADSL_GET_VERSION, buf, 256 );
		*data = strdup( buf );
	}else if( strcmp( lastname, "EnabledOptions" )==0 )
	{
		*data = strdup( "" );
	}else if( strcmp( lastname, "SpecVersion" )==0 )
	{
		*data = strdup( SPECVERSION_STR );
	}else if( strcmp( lastname, "ProvisioningCode" )==0 )
	{
		mib_get( CWMP_PROVISIONINGCODE, (void *)buf);
		*data = strdup( buf );	
	}else if( strcmp( lastname, "UpTime" )==0 )
	{
		struct sysinfo info;
		sysinfo(&info);
		*data = uintdup( info.uptime );
	}else if( strcmp( lastname, "FirstUseDate" )==0 )
	{
		*data = timedup( 0 );
	}else if( strcmp( lastname, "DeviceLog" )==0 )
	{
#if defined(CONFIG_USER_BUSYBOX_SYSLOGD) || defined(SYSLOG_SUPPORT)
		*type = eCWMP_tFILE; /*special case*/
#ifdef CONFIG_BOA_WEB_E8B_CH
		*data = strdup( "/var/config/syslogd.txt" );
#else
		*data = strdup( "/var/log/messages" );
#endif
#else
		*data = strdup( "" );
#endif //#ifdef CONFIG_USER_BUSYBOX_SYSLOGD
	}else if( strcmp( lastname, "VendorConfigFileNumberOfEntries" )==0 )
	{
		*data = uintdup( 1 );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setDeviceInfo(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char 	*buf=data;
	int  	len=0;	
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;
	
	if( strcmp( lastname, "ProvisioningCode" )==0 )
	{
		if( buf ) len = strlen( buf );
		if( len ==0 )
			mib_set( CWMP_PROVISIONINGCODE, (void *)"");
		else if( len < 64 )
			mib_set( CWMP_PROVISIONINGCODE, (void *)buf);
		else
			return ERR_9007;
			
		return 0;
	}else
		return ERR_9005; 
	return 0;
}

#ifdef _PRMT_DEVICECONFIG_
int getDeviceConfig(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "PersistentData" )==0 )
	{
		*data = strdup( "" );
	}else if( strcmp( lastname, "ConfigFile" )==0 )
	{
		if( va_cmd("/bin/CreatexmlConfig",0,1) )
		{
			fprintf( stderr, "<%s:%d>exec /bin/CreatexmlConfig error!\n", __FUNCTION__, __LINE__  );
			return ERR_9002;
		}
		// rename
		rename("/tmp/config.xml", CONFIG_FILE_NAME);
		*type = eCWMP_tFILE; /*special case*/
		*data = strdup(CONFIG_FILE_NAME);
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setDeviceConfig(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "PersistentData" )==0 )
	{
		return ERR_9001;
	}else if( strcmp( lastname, "ConfigFile" )==0 )
	{
		char *buf=data;
		int  buflen=0;
		FILE *fp=NULL;
		
		if( buf==NULL ) return ERR_9007;
		buflen = strlen(buf);
		if( buflen==0 ) return ERR_9007;
		
		fp=fopen( "/tmp/config.xml", "w" );
		if(fp)
		{
			int retlen=0, id;

			fprintf( stderr, "New config length:%d\n", buflen );
#if 0
			retlen = fwrite( buf, 1, buflen, fp );
#else
			/*somehow, the '\n'is gone between lines, 
			  but LoadxmlConfig needs it to parse config.
			  the better way is to rewirte the parsing code of LoadxmlConfig*/
			for( id=0;id<buflen;id++ )
			{
				if( (id>0) && (buf[id-1]=='>') && (buf[id]=='<') )
					if( fputc( '\n',fp )==EOF )
						break;
				
				if(fputc( buf[id],fp )==EOF)
					break;
					
				retlen=id+1;
			}
			fputc( '\n',fp );
#endif
			fclose(fp);
			if( retlen==buflen )
			{
				if( va_cmd("/bin/LoadxmlConfig",0,1) )
				{
					fprintf( stderr, "<%s:%d>exec /bin/LoadxmlConfig error!\n", __FUNCTION__, __LINE__ );
					return ERR_9002; 
				}
			}else
				return ERR_9002;
		}else
			return ERR_9002;
		return 1;
	}else{
		return ERR_9005;
	}
	
	return 0;

}
#endif //_PRMT_DEVICECONFIG_

