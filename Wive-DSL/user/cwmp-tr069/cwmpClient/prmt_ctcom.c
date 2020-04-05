#include "prmt_ctcom.h"




/*************************************************************************************************************************/
/***********_PRMT_X_CT_COM_ACCOUNT_****************************************************************************/
/*************************************************************************************************************************/
#ifdef _PRMT_X_CT_COM_ACCOUNT_
struct CWMP_OP tCTAccountLeafOP = { getCTAccount, setCTAccount };
struct CWMP_PRMT tCTAccountLeafInfo[] =
{
/*(name,		type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCTAccountLeafOP},
{"Password",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTAccountLeafOP}
};
enum eCTAccountLeaf
{
	eCTTA_Enable,
	eCTTA_Password
};
struct CWMP_LEAF tCTAccountLeaf[] =
{
{ &tCTAccountLeafInfo[eCTTA_Enable] },
{ &tCTAccountLeafInfo[eCTTA_Password] },
{ NULL }
};
/****************************************/
int getCTAccount(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	struct in_addr ipAddr;
	char buff[256]={0};
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		mib_get( MIB_CTC_ACCOUNT_ENABLE, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else if( strcmp( lastname, "Password" )==0 )
	{
		*data = strdup( "" );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setCTAccount(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	if( strcmp( lastname, "Enable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(MIB_CTC_ACCOUNT_ENABLE, (void *)&vChar);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_UserAccount, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "Password" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		if(strlen(buf)==0) return ERR_9001;
		if(strlen(buf)>=MAX_NAME_LEN) return ERR_9001;
		mib_set(MIB_SUSER_PASSWORD, (void *)buf);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_UserAccount, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else{
		return ERR_9005;
	}
	
	return 0;
}
#endif /*_PRMT_X_CT_COM_ACCOUNT_*/
/*************************************************************************************************************************/
/***********END _PRMT_X_CT_COM_ACCOUNT_****************************************************************************/
/*************************************************************************************************************************/









/*************************************************************************************************************************/
/***********_PRMT_X_CT_COM_ALG_****************************************************************************/
/*************************************************************************************************************************/
#ifdef _PRMT_X_CT_COM_ALG_
struct CWMP_OP tXCTCOMALGLeafOP = { getXCTCOMALG, setXCTCOMALG };
struct CWMP_PRMT tXCTCOMALGLeafInfo[] =
{
/*(name,		type,		flag,		op)*/
{"H323Enable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tXCTCOMALGLeafOP},
{"SIPEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tXCTCOMALGLeafOP},
{"RTSPEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tXCTCOMALGLeafOP},
{"L2TPEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tXCTCOMALGLeafOP},
{"IPSECEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tXCTCOMALGLeafOP},
};
enum eXCTCOMALGLeaf
{
	eCT_H323Enable,
	eCT_SIPEnable,
	eCT_RTSPEnable,
	eCT_L2TPEnable,
	eCT_IPSECEnable
};
struct CWMP_LEAF tXCTCOMALGLeaf[] =
{
{ &tXCTCOMALGLeafInfo[eCT_H323Enable] },
{ &tXCTCOMALGLeafInfo[eCT_SIPEnable] },
{ &tXCTCOMALGLeafInfo[eCT_RTSPEnable] },
{ &tXCTCOMALGLeafInfo[eCT_L2TPEnable] },
{ &tXCTCOMALGLeafInfo[eCT_IPSECEnable] },
{ NULL }
};
/*******************************************/
int getXCTCOMALG(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "H323Enable" )==0 )
	{
#ifdef CONFIG_IP_NF_H323
#ifdef CONFIG_IP_NF_ALG_ONOFF
		mib_get( MIB_IP_ALG_H323, (void *)&vChar);
#else
		vChar=1;
#endif
#endif
		*data = booldup( vChar );
	}else if( strcmp( lastname, "SIPEnable" )==0 )
	{
#ifdef CONFIG_IP_NF_SIP
#ifdef CONFIG_IP_NF_ALG_ONOFF
		mib_get( MIB_IP_ALG_SIP, (void *)&vChar);
#else
		vChar=1;
#endif
#endif
		*data = booldup( vChar );
	}else if( strcmp( lastname, "RTSPEnable" )==0 )
	{
#ifdef CONFIG_IP_NF_RTSP
#ifdef CONFIG_IP_NF_ALG_ONOFF
		mib_get( MIB_IP_ALG_RTSP, (void *)&vChar);
#else
		vChar=1;
#endif
#endif
		*data = booldup( vChar );
	}else if( strcmp( lastname, "L2TPEnable" )==0 )
	{
#ifdef CONFIG_IP_NF_L2TP
#ifdef CONFIG_IP_NF_ALG_ONOFF
		mib_get( MIB_IP_ALG_L2TP, (void *)&vChar);
#else
		vChar=1;
#endif
#endif
		*data = booldup( vChar );
	}else if( strcmp( lastname, "IPSECEnable" )==0 )
	{
#ifdef CONFIG_IP_NF_IPSEC
#ifdef CONFIG_IP_NF_ALG_ONOFF
		mib_get( MIB_IP_ALG_IPSEC, (void *)&vChar);
#else
		vChar=1;
#endif
#endif
		*data = booldup( vChar );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setXCTCOMALG(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif
#ifndef CONFIG_IP_NF_ALG_ONOFF
	 	return ERR_9008;
#else
	if( strcmp( lastname, "H323Enable" )==0 )
#ifdef CONFIG_IP_NF_H323
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(MIB_IP_ALG_H323, (void *)&vChar);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_L, apply_ALGONOFF, CWMP_RESTART, 0, NULL, 0 );     
		return 0;
#else
		return 1;
#endif
	}
#else
		return ERR_9005;
#endif
		

	if( strcmp( lastname, "SIPEnable" )==0 )
#ifdef CONFIG_IP_NF_SIP
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(MIB_IP_ALG_SIP, (void *)&vChar);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_L, apply_ALGONOFF, CWMP_RESTART, 0, NULL, 0 );     
		return 0;
#else
		return 1;
#endif
	}
#else
		return ERR_9005;
#endif

	if( strcmp( lastname, "RTSPEnable" )==0 )
#ifdef CONFIG_IP_NF_RTSP
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(MIB_IP_ALG_RTSP, (void *)&vChar);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_L, apply_ALGONOFF, CWMP_RESTART, 0, NULL, 0 );     
		return 0;
#else
		return 1;
#endif
	}
#else
		return ERR_9005;
#endif

	if( strcmp( lastname, "L2TPEnable" )==0 )
#ifdef CONFIG_IP_NF_L2TP
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(MIB_IP_ALG_L2TP, (void *)&vChar);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_L, apply_ALGONOFF, CWMP_RESTART, 0, NULL, 0 );     
		return 0;
#else
		return 1;
#endif
	}
#else
		return ERR_9005;
#endif

	if( strcmp( lastname, "IPSECEnable" )==0 )
#ifdef CONFIG_IP_NF_IPSEC
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(MIB_IP_ALG_IPSEC, (void *)&vChar);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_L, apply_ALGONOFF, CWMP_RESTART, 0, NULL, 0 );     
		return 0;
#else
		return 1;
#endif
	}
#else
		return ERR_9005;
#endif
	
	return 0;
#endif
}
#endif //_PRMT_X_CT_COM_ALG_
/*************************************************************************************************************************/
/***********END _PRMT_X_CT_COM_ALG_****************************************************************************/
/*************************************************************************************************************************/










/*************************************************************************************************************************/
/***********_PRMT_X_CT_COM_RECON_****************************************************************************/
/*************************************************************************************************************************/
#ifdef _PRMT_X_CT_COM_RECON_
struct CWMP_OP tCT_ReConLeafOP = { getCT_ReCon,	setCT_ReCon };
struct CWMP_PRMT tCT_ReConLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_ReConLeafOP},
};
enum eCT_ReConLeafInfo
{
	eCT_RCEnable
};
struct CWMP_LEAF tCT_ReConLeaf[] =
{
{ &tCT_ReConLeafInfo[eCT_RCEnable] },
{ NULL }
};
/***********************************/
int getCT_ReCon(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		mib_get( CWMP_CT_RECON_ENABLE, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setCT_ReCon(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	if( strcmp( lastname, "Enable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(CWMP_CT_RECON_ENABLE, (void *)&vChar);
#ifdef CONFIG_NO_REDIAL
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_Reconnect, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
#else
		return 0;
#endif
	}else{
		return ERR_9005;
	}
	
	return 0;
}
#endif //_PRMT_X_CT_COM_RECON_
/*************************************************************************************************************************/
/***********END _PRMT_X_CT_COM_RECON_****************************************************************************/
/*************************************************************************************************************************/









/*************************************************************************************************************************/
/***********_PRMT_X_CT_COM_PORTALMNT_****************************************************************************/
/*************************************************************************************************************************/
#ifdef _PRMT_X_CT_COM_PORTALMNT_
struct CWMP_OP tCT_PortalMNTLeafOP = { getCT_PortalMNT, setCT_PortalMNT };
struct CWMP_PRMT tCT_PortalMNTLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_PortalMNTLeafOP},
{"PortalUrl-Computer",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_PortalMNTLeafOP},
{"PortalUrl-STB",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_PortalMNTLeafOP},
{"PortalUrl-Phone",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_PortalMNTLeafOP},
};
enum eCT_PortalMNTLeaf
{
	eCT_PMEnable,
	eCT_PMComputer,
	eCT_PMSTB,
	eCT_PMPhone
};
struct CWMP_LEAF tCT_PortalMNTLeaf[] =
{
{ &tCT_PortalMNTLeafInfo[eCT_PMEnable] },
{ &tCT_PortalMNTLeafInfo[eCT_PMComputer] },
{ &tCT_PortalMNTLeafInfo[eCT_PMSTB] },
{ &tCT_PortalMNTLeafInfo[eCT_PMPhone] },
{ NULL } 
};
/********************************************/
int getCT_PortalMNT(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	unsigned char buf[256]={0};
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
#ifdef CONFIG_BOA_WEB_E8B_CH
		mib_get( MIB_FP_ENABLE, (void *)&vChar);
#else
		mib_get( CWMP_CT_PM_ENABLE, (void *)&vChar);
#endif
		*data = booldup( vChar!=0 );
	}else if( strcmp( lastname, "PortalUrl-Computer" )==0 )
	{
#ifdef CONFIG_BOA_WEB_E8B_CH
		mib_get( MIB_FP_PC_ADDR, (void *)buf);
#else
		mib_get( CWMP_CT_PM_URL4PC, (void *)buf);
#endif
		*data = strdup( buf );	
	}else if( strcmp( lastname, "PortalUrl-STB" )==0 )
	{
#ifdef CONFIG_BOA_WEB_E8B_CH
		mib_get( MIB_FP_STB_ADDR, (void *)buf);
#else
		mib_get( CWMP_CT_PM_URL4STB, (void *)buf);
#endif
		*data = strdup( buf );	
	}else if( strcmp( lastname, "PortalUrl-Phone" )==0 )
	{
#ifdef CONFIG_BOA_WEB_E8B_CH
		mib_get( MIB_FP_MOB_ADDR, (void *)buf);
#else
		mib_get( CWMP_CT_PM_URL4MOBILE, (void *)buf);
#endif
		*data = strdup( buf );	
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setCT_PortalMNT(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	if( strcmp( lastname, "Enable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
#ifdef CONFIG_BOA_WEB_E8B_CH
		mib_set( MIB_FP_ENABLE, (void *)&vChar);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_PortalMNT, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
#else
		mib_set(CWMP_CT_PM_ENABLE, (void *)&vChar);
		return 0;
#endif

	}else if( strcmp( lastname, "PortalUrl-Computer" )==0 )
	{
		if(buf==NULL) return ERR_9007;
#ifdef CONFIG_BOA_WEB_E8B_CH
		mib_set( MIB_FP_PC_ADDR, (void *)buf);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_PortalMNT, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
#else
		mib_set( CWMP_CT_PM_URL4PC, (void *)buf);
		return 0;
#endif
	}else if( strcmp( lastname, "PortalUrl-STB" )==0 )
	{
		if(buf==NULL) return ERR_9007;
#ifdef CONFIG_BOA_WEB_E8B_CH
		mib_set( MIB_FP_STB_ADDR, (void *)buf);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_PortalMNT, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
#else
		mib_set( CWMP_CT_PM_URL4STB, (void *)buf);
		return 0;
#endif
	}else if( strcmp( lastname, "PortalUrl-Phone" )==0 )
	{
		if(buf==NULL) return ERR_9007;
#ifdef CONFIG_BOA_WEB_E8B_CH
		mib_set( MIB_FP_MOB_ADDR, (void *)buf);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_PortalMNT, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
#else
		mib_set( CWMP_CT_PM_URL4MOBILE, (void *)buf);
		return 0;
#endif
	}else{
		return ERR_9005;
	}
	
	return 0;
}
#endif //_PRMT_X_CT_COM_PORTALMNT_
/*************************************************************************************************************************/
/***********END _PRMT_X_CT_COM_PORTALMNT_****************************************************************************/
/*************************************************************************************************************************/










/*************************************************************************************************************************/
/***********_PRMT_X_CT_COM_SRVMNG_****************************************************************************/
/*************************************************************************************************************************/
#ifdef _PRMT_X_CT_COM_SRVMNG_
struct CWMP_OP tCTServiceLeafOP = { getCTService, setCTService };
struct CWMP_PRMT tCTServiceLeafInfo[] =
{
/*(name,		type,		flag,			op)*/
{"FtpEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCTServiceLeafOP},
{"FtpUserName",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTServiceLeafOP},
{"FtpPassword",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTServiceLeafOP},
{"FtpPort",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tCTServiceLeafOP},
{"TelnetEnable",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCTServiceLeafOP},
{"TelnetUserName",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTServiceLeafOP},
{"TelnetPassword",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTServiceLeafOP},
{"TelnetPort",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tCTServiceLeafOP}
};
enum eCTServiceLeaf
{
	eCT_FtpEnable,
	eCT_FtpUserName,
	eCT_FtpPassword,
	eCT_FtpPort,
	eCT_TelnetEnable,
	eCT_TelnetUserName,
	eCT_TelnetPassword,
	eCT_TelnetPort
};
struct CWMP_LEAF tCTServiceLeaf[] =
{
{ &tCTServiceLeafInfo[eCT_FtpEnable] },
{ &tCTServiceLeafInfo[eCT_FtpUserName] },
{ &tCTServiceLeafInfo[eCT_FtpPassword] },
{ &tCTServiceLeafInfo[eCT_FtpPort] },
{ &tCTServiceLeafInfo[eCT_TelnetEnable] },
{ &tCTServiceLeafInfo[eCT_TelnetUserName] },
{ &tCTServiceLeafInfo[eCT_TelnetPassword] },
{ &tCTServiceLeafInfo[eCT_TelnetPort] },
{ NULL }
};
/*************************************************/
int getCTService(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	struct in_addr ipAddr;
	char buff[256]={0};
	MIB_CE_ACC_T Entry;
#ifdef CONFIG_BOA_WEB_E8B_CH
	unsigned short ftpport =0;
#endif
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	if (!mib_chain_get(MIB_ACC_TBL, 0, (void *)&Entry))
		return ERR_9002;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "FtpEnable" )==0 )
	{
		//LAN&WAN are enabled => return 1
#ifdef CONFIG_BOA_WEB_E8B_CH
		mib_get(MIB_BFTPD_ENABLE,(void *)&vChar);
		*data = booldup( (Entry.ftp==0x3 && vChar != 0) );
#else
		*data = booldup( Entry.ftp==0x3 );
#endif
	}else if( strcmp( lastname, "FtpUserName" )==0 )
	{
#ifdef CONFIG_BOA_WEB_E8B_CH
		mib_get(MIB_FTP_USER, (void *)buff);
#else
		mib_get(MIB_SUSER_NAME, (void *)buff);
#endif
		 *data=strdup( buff );
	}else if( strcmp( lastname, "FtpPassword" )==0 )
	{
#if 1
		*data=strdup( "" );
#else
		mib_get(MIB_SUSER_PASSWORD, (void *)buff);
		*data=strdup( buff );
#endif
	}else if( strcmp( lastname, "FtpPort" )==0 )
	{
#ifdef CONFIG_BOA_WEB_E8B_CH
		mib_get(MIB_FTP_PORT,(void *)&ftpport);
		*data = intdup((int)ftpport);
#else
		*data = intdup( Entry.ftp_port );
#endif
	}else if( strcmp( lastname, "TelnetEnable" )==0 )
	{
		//LAN&WAN are enabled => return 1
		*data = booldup( Entry.telnet==0x3 );
	}else if( strcmp( lastname, "TelnetUserName" )==0 )
	{
#ifdef CONFIG_BOA_WEB_E8B_CH
		mib_get(MIB_TELNET_USER, (void *)buff);
#else
		mib_get(MIB_SUSER_NAME, (void *)buff);
#endif
		 *data=strdup( buff );
	}else if( strcmp( lastname, "TelnetPassword" )==0 )
	{
#if 1
		*data=strdup( "" );
#else
		mib_get(MIB_SUSER_PASSWORD, (void *)buff);
		*data=strdup( buff );
#endif
	}else if( strcmp( lastname, "TelnetPort" )==0 )
	{
		*data = intdup( Entry.telnet_port );	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setCTService(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;
#ifdef CONFIG_BOA_WEB_E8B_CH
	unsigned short ftpport = 0;
#endif
	MIB_CE_ACC_T Entry;
#ifdef _CWMP_APPLY_
	MIB_CE_ACC_T OldEntry;
#endif
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	if (!mib_chain_get(MIB_ACC_TBL, 0, (void *)&Entry))
		return ERR_9002;
#ifdef _CWMP_APPLY_
	memcpy( &OldEntry, &Entry, sizeof(MIB_CE_ACC_T) );
#endif

	if( strcmp( lastname, "FtpEnable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
#ifdef CONFIG_BOA_WEB_E8B_CH
		vChar = (*i==0) ? 0:1;
		mib_set(MIB_BFTPD_ENABLE,(void *)&vChar);
#endif
		Entry.ftp=(*i==0)?0:0x3;
		mib_chain_update(MIB_ACC_TBL, (char *)&Entry, 0 );

#ifdef _CWMP_APPLY_
#ifdef CONFIG_BOA_WEB_E8B_CH
		apply_add( CWMP_PRI_N, apply_FtpdCfgChanged, CWMP_RESTART, 0, NULL, 0 ); 
#endif
		apply_add( CWMP_PRI_N, apply_RemoteAccess, CWMP_RESTART, 0, &OldEntry, sizeof(MIB_CE_ACC_T) );

		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "FtpUserName" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		if(strlen(buf)==0) return ERR_9001;
		if(strlen(buf)>=MAX_NAME_LEN) return ERR_9001;
#ifdef CONFIG_BOA_WEB_E8B_CH
		mib_set(MIB_FTP_USER, (void *)buf);
#else
		mib_set(MIB_SUSER_NAME, (void *)buf);
#endif
#ifdef _CWMP_APPLY_
#ifdef CONFIG_BOA_WEB_E8B_CH
		apply_add( CWMP_PRI_N, apply_FtpdCfgChanged, CWMP_RESTART, 0, NULL, 0 ); 
#else
		apply_add( CWMP_PRI_N, apply_UserAccount, CWMP_RESTART, 0, NULL, 0 );
#endif
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "FtpPassword" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		if(strlen(buf)==0) return ERR_9001;
		if(strlen(buf)>=MAX_NAME_LEN) return ERR_9001;
#ifdef CONFIG_BOA_WEB_E8B_CH
		mib_set(MIB_FTP_PASSWD, (void *)buf);
#else
		mib_set(MIB_SUSER_PASSWORD, (void *)buf);
#endif
#ifdef _CWMP_APPLY_
#ifdef CONFIG_BOA_WEB_E8B_CH
		apply_add( CWMP_PRI_N, apply_FtpdCfgChanged, CWMP_RESTART, 0, NULL, 0 ); 
#else
		apply_add( CWMP_PRI_N, apply_UserAccount, CWMP_RESTART, 0, NULL, 0 );
#endif
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "FtpPort" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpint;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		if(*i<0 || *i>65535 ) return ERR_9007;
#ifdef CONFIG_BOA_WEB_E8B_CH
		ftpport = (unsigned short)(*i);
		mib_set(MIB_FTP_PORT,(void *)&ftpport);
#else
		Entry.ftp_port = *i;
		mib_chain_update(MIB_ACC_TBL, (char *)&Entry, 0 );
#endif
#ifdef _CWMP_APPLY_
#ifdef CONFIG_BOA_WEB_E8B_CH
		apply_add( CWMP_PRI_N, apply_FtpdCfgChanged, CWMP_RESTART, 0, NULL, 0 ); 
#else
		apply_add( CWMP_PRI_N, apply_RemoteAccess, CWMP_RESTART, 0, &OldEntry, sizeof(MIB_CE_ACC_T) );
#endif
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "TelnetEnable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		Entry.telnet=(*i==0)?0:0x3;
		mib_chain_update(MIB_ACC_TBL, (char *)&Entry, 0 );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_RemoteAccess, CWMP_RESTART, 0, &OldEntry, sizeof(MIB_CE_ACC_T) );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "TelnetUserName" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		if(strlen(buf)==0) return ERR_9001;
		if(strlen(buf)>=MAX_NAME_LEN) return ERR_9001;
#ifdef CONFIG_BOA_WEB_E8B_CH
		mib_set(MIB_TELNET_USER, (void *)buf);
		return 0;
#else
		mib_set(MIB_SUSER_NAME, (void *)buf);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_UserAccount, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
#endif
	}else if( strcmp( lastname, "TelnetPassword" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		if(strlen(buf)==0) return ERR_9001;
		if(strlen(buf)>=MAX_NAME_LEN) return ERR_9001;
#ifdef CONFIG_BOA_WEB_E8B_CH
		mib_set(MIB_TELNET_PASSWD, (void *)buf);
		return 0;
#else
		mib_set(MIB_SUSER_PASSWORD, (void *)buf);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_UserAccount, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
#endif
	}else if( strcmp( lastname, "TelnetPort" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpint;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		if(*i<0 || *i>65535 ) return ERR_9007;
		Entry.telnet_port = *i;
		mib_chain_update(MIB_ACC_TBL, (char *)&Entry, 0 );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_RemoteAccess, CWMP_RESTART, 0, &OldEntry, sizeof(MIB_CE_ACC_T) );
		return 0;
#else
		return 1;
#endif
	}else{
		return ERR_9005;
	}
	
	return 0;
}
#endif //_PRMT_X_CT_COM_SRVMNG_
/*************************************************************************************************************************/
/***********END _PRMT_X_CT_COM_SRVMNG_****************************************************************************/
/*************************************************************************************************************************/









/*************************************************************************************************************************/
/***********_PRMT_X_CT_COM_SYSLOG_****************************************************************************/
/*************************************************************************************************************************/
#ifdef _PRMT_X_CT_COM_SYSLOG_
struct CWMP_OP tCT_SyslogLeafOP = { getCT_Syslog, setCT_Syslog };
struct CWMP_PRMT tCT_SyslogLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_SyslogLeafOP},
{"Level",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_SyslogLeafOP},
};
enum eCT_SyslogLeaf
{
	eCT_SyslogEnable,
	eCT_SyslogLevel,
};
struct CWMP_LEAF tCT_SyslogLeaf[] =
{
{ &tCT_SyslogLeafInfo[eCT_SyslogEnable] },
{ &tCT_SyslogLeafInfo[eCT_SyslogLevel] },
{ NULL } 
};
/*****************************************/
char* syslog_level[9]={
	"0:Emergency",
	"1:Alert",
	"2:Critical",
	"3:Error",
	"4:Warning",
	"5:Notice",
	"6:Informational",
	"7:Debug",
	NULL
};
static char getlevelnum(char *buf)
{
	char i;

	for(i=0;i<8;i++){
		if(!strcmp(buf,syslog_level[i]))
			return i;
	}
	return -1;
}
int getCT_Syslog(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	unsigned char buf[256]={0};
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
#ifdef CONFIG_BOA_WEB_E8B_CH
		mib_get(MIB_SYSLOG_ENABLE, (void *)&vChar);
#else
		mib_get(MIB_SYSLOG, (void *)&vChar);
#endif
		*data = booldup( vChar!=0 );
	}else if( strcmp( lastname, "Level" )==0 )
	{
#ifdef CONFIG_BOA_WEB_E8B_CH
		mib_get(MIB_SYSLOG_RECORDE_GRADE, (void *)&vChar);
		if(vChar<0||vChar>7)
			return ERR_9002;
		sprintf(buf,"%s",syslog_level[vChar]);
		*data = strdup(buf);	
#endif
		//mib_get( CWMP_CT_PM_URL4PC, (void *)buf);
		//*data = strdup( buf );	
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setCT_Syslog(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	if( strcmp( lastname, "Enable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
#ifdef CONFIG_BOA_WEB_E8B_CH
		mib_set(MIB_SYSLOG_ENABLE, (void *)&vChar);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_Syslog, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
#else
		mib_set(MIB_SYSLOG, (void *)&vChar);
		return 1;
#endif
	}else if( strcmp( lastname, "Level" )==0 )
	{
#ifdef CONFIG_BOA_WEB_E8B_CH
		char level=getlevelnum(buf);
		if(level==-1)
			return ERR_9007;
		mib_set(MIB_SYSLOG_RECORDE_GRADE, (void *)&level);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_Syslog, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
#else
		//if(buf==NULL) return ERR_9007;
		//mib_set( CWMP_CT_PM_URL4PC, (void *)buf);
		return 0;
#endif
	}else{
		return ERR_9005;
	}
	
	return 0;
}
#endif //_PRMT_X_CT_COM_SYSLOG_
/*************************************************************************************************************************/
/***********END _PRMT_X_CT_COM_SYSLOG_****************************************************************************/
/*************************************************************************************************************************/









/*************************************************************************************************************************/
/***********CONFIG_MIDDLEWARE****************************************************************************/
/*************************************************************************************************************************/
#ifdef CONFIG_MIDDLEWARE
struct CWMP_OP tCT_MiddlewareMgtLeafOP = { getCT_MiddlewareMgt, setCT_MiddlewareMgt };
struct CWMP_PRMT tCT_MiddlewareMgtLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Tr069Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_MiddlewareMgtLeafOP},
{"MiddlewareURL",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_MiddlewareMgtLeafOP},
};

enum eCT_MiddlewareMgtLeaf
{
	eCT_MiddlewareURL,
	eCT_Tr069Enable,
};
struct CWMP_LEAF tCT_MiddlewareMgtLeaf[] =
{
{ &tCT_MiddlewareMgtLeafInfo[eCT_MiddlewareURL] },
{ &tCT_MiddlewareMgtLeafInfo[eCT_Tr069Enable] },
{ NULL } 
};
/***************************************************/
int getCT_MiddlewareMgt(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	unsigned char addrBuf[256+1]={0};
	int port = 0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Tr069Enable" )==0 )
	{
		mib_get(CWMP_TR069_ENABLE, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else if( strcmp( lastname, "MiddlewareURL" )==0 )
	{
		mib_get(CWMP_MIDWARE_SERVER_ADDR,(void *)addrBuf);
		if(strlen(addrBuf) == 0)
			strcpy(addrBuf,"0.0.0.0");
		mib_get(CWMP_MIDWARE_SERVER_PORT,(void *)&port);
		sprintf(addrBuf,"%s:%d",addrBuf,port);
		*data = strdup(addrBuf);
	}else{
		return ERR_9005;
	}
	
	return 0;
}

#define MAX_MIDWARE_URL_LEN	256
int setCT_MiddlewareMgt(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	unsigned char buf[MAX_MIDWARE_URL_LEN+1];
	int port;
	char * delim = ":";
	char * pAddr=NULL;
	char * pPort=NULL;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	if( strcmp( lastname, "Tr069Enable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(CWMP_TR069_ENABLE, (void *)&vChar);
		return 1;
	}else if( strcmp( lastname, "MiddlewareURL" )==0 )
	{
		if(strlen(data) > MAX_MIDWARE_URL_LEN || strlen(data) == 0)
			return -1;
		strcpy(buf,(char *)data);
		pAddr = strtok(buf,delim);
		pPort = strtok(NULL,delim);
		if(!pAddr || !pPort || strlen(pAddr)==0 || strlen(pPort)==0)
			return -1;
		port = atoi(pPort);
		if(!mib_set(CWMP_MIDWARE_SERVER_ADDR,(void *)pAddr))
			return -1;
		if(!mib_set(CWMP_MIDWARE_SERVER_PORT,(void *)&port))
			return -1;
		
		return 0;
	}else{
		return ERR_9005;
	}
	
	return 0;
}
#endif //CONFIG_MIDDLEWARE
/*************************************************************************************************************************/
/***********END CONFIG_MIDDLEWARE****************************************************************************/
/*************************************************************************************************************************/









/*************************************************************************************************************************/
/***********_PRMT_X_CT_COM_PPPOE_****************************************************************************/
/*************************************************************************************************************************/
#if 0
#ifdef _PRMT_X_CT_COM_PPPOE_
int getCT_PPPoE(char *name, struct sCWMP_ENTITY *entity, int *type, void **data);
int setCT_PPPoE(char *name, struct sCWMP_ENTITY *entity, int type, void *data);
int objCT_PPPoE(char *name, struct sCWMP_ENTITY *entity, int type, void *data);
struct sCWMP_ENTITY tCT_PPPoEEntity[] =
{
/*(name,			type,		flag,			accesslist,	getvalue,	setvalue,	next_table,	sibling)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	NULL,		getCT_PPPoE,	setCT_PPPoE,	NULL,		NULL},
{"MAXUser",			eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	NULL,		getCT_PPPoE,	setCT_PPPoE,	NULL,		NULL},
{"Interface",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	NULL,		getCT_PPPoE,	setCT_PPPoE,	NULL,		NULL},
{"",				eCWMP_tNONE,	0,			NULL,		NULL,		NULL,		NULL,		NULL}
};
struct sCWMP_ENTITY tCT_PPPoE[] =
{
/*(name,			type,		flag,					accesslist,	getvalue,	setvalue,	next_table,	sibling)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL,		NULL,		NULL,		tCT_PPPoEEntity,NULL}
};
/**********************************/
extern unsigned int getInstNum( char *name, char *objname );
unsigned int getPPPoEInstNum( char *name )
{
	return getInstNum( name, "X_CT-COM_PPPoEProxy" );
}
int getPPPoEMibEntry( unsigned int instnum, unsigned int *id, CWMP_CT_PPPOE_T *p )
{
	int num,i;
	
	if( (instnum==0) || (p==NULL) || (id==NULL) ) return -1;
	num = mib_chain_total( CWMP_CT_PPPOE_TBL );
	for(i=0;i<num;i++)
	{
		if( !mib_chain_get( CWMP_CT_PPPOE_TBL, i, (void*)p ) )
			continue;
			
		if( p->InstanceNum==instnum )
		{
			*id = i;
			return 0;
		}
	}
	return -1;
}

int getCT_PPPoE(char *name, struct sCWMP_ENTITY *entity, int *type, void **data)
{
	char		*lastname = entity->name;
	CWMP_CT_PPPOE_T *p, ct_entry;
	unsigned int	instnum=0, mibid=0;
	unsigned char	vChar=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	instnum = getPPPoEInstNum( name );
	if(instnum==0) return ERR_9007;
	p = &ct_entry;
	if( getPPPoEMibEntry( instnum, &mibid, p )<0) return ERR_9002;
	
	
	*type = entity->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		*data = booldup( p->Enable!=0 );
	}else if( strcmp( lastname, "MAXUser" )==0 )
	{
		*data = uintdup( p->MAXUser );
	}else if( strcmp( lastname, "Interface" )==0 )
	{
		char buf[256]="";
		if( p->ifIndex!=0xff )
			transfer2PathName( p->ifIndex, buf  );
		*data = strdup( buf );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setCT_PPPoE(char *name, struct sCWMP_ENTITY *entity, int type, void *data)
{
	char	*lastname = entity->name;
	char	*buf=data;
	CWMP_CT_PPPOE_T *p, ct_entry;
	unsigned int	instnum=0, mibid=0;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	instnum = getPPPoEInstNum( name );
	if(instnum==0) return ERR_9007;
	p = &ct_entry;
	if( getPPPoEMibEntry( instnum, &mibid, p )<0) return ERR_9002;


	if( strcmp( lastname, "Enable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *pInt = &tmpbool;		
#else
		int *pInt = data;
#endif
		if( pInt==NULL ) return ERR_9007;
		p->Enable=(*pInt==0)?0:1;
		mib_chain_update( CWMP_CT_PPPOE_TBL, (unsigned char*)p, mibid );
		return 1;
	}else if( strcmp( lastname, "MAXUser" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *pUint = &tmpuint;		
#else
		unsigned int *pUint = data;
#endif
		if(pUint==NULL) return ERR_9007;
		p->MAXUser = *pUint;
		mib_chain_update( CWMP_CT_PPPOE_TBL, (unsigned char*)p, mibid );
		return 1;		
	}else if( strcmp( lastname, "Interface" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		if(strlen(buf)!=0)
		{
			unsigned int ifindex=0;
			ifindex = transfer2IfIndex(buf);
			if(ifindex==0xff) return ERR_9007;
			p->ifIndex=ifindex;
		}else
			p->ifIndex=0xff;
		mib_chain_update( CWMP_CT_PPPOE_TBL, (unsigned char*)p, mibid );
		return 1;
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int objCT_PPPoE(char *name, struct sCWMP_ENTITY *entity, int type, void *data)
{
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);

	switch( type )
	{
	case eCWMP_tINITOBJ:
	     {
		int num=0,MaxInstNum=0,i;
		struct sCWMP_ENTITY **c = (struct sCWMP_ENTITY **)data;
		CWMP_CT_PPPOE_T *p,ct_entity;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		num = mib_chain_total( CWMP_CT_PPPOE_TBL );
		for( i=0; i<num;i++ )
		{
			p = &ct_entity;
			if( !mib_chain_get( CWMP_CT_PPPOE_TBL, i, (void*)p ) )
				continue;
			
			if( p->InstanceNum==0 ) //maybe createn by web or cli
			{
				MaxInstNum++;
				p->InstanceNum = MaxInstNum;
				mib_chain_update( CWMP_CT_PPPOE_TBL, (unsigned char*)p, i );
			}else
				MaxInstNum = p->InstanceNum;
			if( create_Object( c, tCT_PPPoE, sizeof(tCT_PPPoE), 1, MaxInstNum ) < 0 )
				return -1;
		}
		add_objectNum( name, MaxInstNum );
		return 0;
	     }
	case eCWMP_tADDOBJ:
	     {
	     	int ret;
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
		ret = add_Object( name, &entity->next_table,  tCT_PPPoE, sizeof(tCT_PPPoE), data );
		if( ret >= 0 )
		{
			CWMP_CT_PPPOE_T fentry;
			memset( &fentry, 0, sizeof( CWMP_CT_PPPOE_T ) );
			fentry.Enable = 0;
			fentry.MAXUser = 0;
			fentry.ifIndex = 0xff;
			fentry.InstanceNum = *(unsigned int*)data;
			mib_chain_add( CWMP_CT_PPPOE_TBL, (unsigned char*)&fentry );
		}
		return ret;
	     }
	case eCWMP_tDELOBJ:
	     {
	     	int ret, id;
	     	
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
	     	
	     	id = getChainID( entity->next_table, *(int*)data  );
	     	if(id==-1) return ERR_9005;
	     	mib_chain_delete( CWMP_CT_PPPOE_TBL, id );	
		ret = del_Object( name, &entity->next_table, *(int*)data );
		if( ret == 0 ) return 1;
		return ret;
	     }
	case eCWMP_tUPDATEOBJ:	
	     {
	     	int num=0,i;
	     	struct sCWMP_ENTITY *old_table;
	     	
	     	num = mib_chain_total( CWMP_CT_PPPOE_TBL );
	     	old_table = entity->next_table;
	     	entity->next_table = NULL;
	     	for( i=0; i<num;i++ )
	     	{
	     		struct sCWMP_ENTITY *remove_entity=NULL;
			CWMP_CT_PPPOE_T *p,ct_entity;

			p = &ct_entity;
			if( !mib_chain_get( CWMP_CT_PPPOE_TBL, i, (void*)p ) )
				continue;
			
			remove_entity = remove_SiblingEntity( &old_table, p->InstanceNum );
			if( remove_entity!=NULL )
			{
				add_SiblingEntity( &entity->next_table, remove_entity );
			}else{ 
				unsigned int MaxInstNum=p->InstanceNum;
					
				add_Object( name, &entity->next_table,  tCT_PPPoE, sizeof(tCT_PPPoE), &MaxInstNum );
				if(MaxInstNum!=p->InstanceNum)
				{
					p->InstanceNum = MaxInstNum;
					mib_chain_update( CWMP_CT_PPPOE_TBL, (unsigned char*)p, i );
				}
			}	
	     	}
	     	
	     	if( old_table )
	     		destroy_ParameterTable( old_table );	     	

	     	return 0;
	     }
	}
	
	return -1;
}
#endif //_PRMT_X_CT_COM_PPPOE_
#endif //if 0
/*************************************************************************************************************************/
/***********END _PRMT_X_CT_COM_PPPOE_****************************************************************************/
/*************************************************************************************************************************/







/*************************************************************************************************************************/
/***********_PRMT_X_CT_COM_IPTV_****************************************************************************/
/*************************************************************************************************************************/
#ifdef _PRMT_X_CT_COM_IPTV_
struct CWMP_OP tCT_IPTVLeafOP = { getCT_IPTV, setCT_IPTV };
struct CWMP_PRMT tCT_IPTVLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"IGMPEnable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_IPTVLeafOP},
{"STBNumber",			eCWMP_tUINT,	CWMP_READ,	&tCT_IPTVLeafOP},
};
enum eCT_IPTVLeaf
{
	eCTIGMPEnable,
	eCTSTBNumber
};
struct CWMP_LEAF tCT_IPTVLeaf[]=
{
{ &tCT_IPTVLeafInfo[eCTIGMPEnable] },
{ &tCT_IPTVLeafInfo[eCTSTBNumber] },
{ NULL }
};
int getCT_IPTV(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	unsigned int  vUint=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "IGMPEnable" )==0 )
	{
		//mib_get( CWMP_CT_IPTV_IGMPENABLE, (void *)&vChar);
		mib_get( MIB_MPMODE, (void *)&vChar);
		if((vChar>>2)&0x01!=0) // igmp snooping
			*data = booldup(1);
		else{
			mib_get( MIB_IGMP_PROXY, (void *)&vChar);  //igmp proxy
			if(vChar==1)
				*data = booldup(1);
			else
				*data = booldup(0);
		}
	}else if( strcmp( lastname, "STBNumber" )==0 )
	{
		mib_get( CWMP_CT_IPTV_STBNUMBER, (void *)&vUint);
		*data = uintdup( vUint );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setCT_IPTV(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	if( strcmp( lastname, "IGMPEnable" )==0 )
	{

#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(CWMP_CT_IPTV_IGMPENABLE, (void *)&vChar);
	}
/*
	else if( strcmp( lastname, "STBNumber" )==0 )
	{
		unsigned int *i = data;
		if( i==NULL ) return ERR_9007;
		mib_set( CWMP_CT_IPTV_STBNUMBER, (void *)i);
	}
*/
	else{
		return ERR_9005;
	}
	
	return 0;
}
#endif //_PRMT_X_CT_COM_IPTV_
/*************************************************************************************************************************/
/***********END _PRMT_X_CT_COM_IPTV_****************************************************************************/
/*************************************************************************************************************************/







/*************************************************************************************************************************/
/***********_PRMT_X_CT_COM_MONITOR_****************************************************************************/
/*************************************************************************************************************************/
#ifdef _PRMT_X_CT_COM_MONITOR_
struct CWMP_OP tCT_MONITORLeafOP = { getCT_MONITOR, setCT_MONITOR };
struct CWMP_PRMT tCT_MONITORLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_MONITORLeafOP},
{"MonitorNumber",		eCWMP_tUINT,	CWMP_READ,		&tCT_MONITORLeafOP}
};
enum eCT_MONITORLeaf
{
	eCTMNT_Enable,
	eCTMNT_Number
};
struct CWMP_LEAF tCT_MONITORLeaf[] =
{
{ &tCT_MONITORLeafInfo[eCTMNT_Enable] },
{ &tCT_MONITORLeafInfo[eCTMNT_Number] },
{ NULL }
};
int getCT_MONITOR(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	unsigned int  vUint=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		mib_get( CWMP_CT_MNT_ENABLE, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else if( strcmp( lastname, "MonitorNumber" )==0 )
	{
		mib_get( CWMP_CT_MNT_NUMBER, (void *)&vUint);
		*data = uintdup( vUint );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setCT_MONITOR(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	if( strcmp( lastname, "Enable" )==0 )
	{

#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(CWMP_CT_MNT_ENABLE, (void *)&vChar);
	}else{
		return ERR_9005;
	}
	
	return 0;
}
#endif //_PRMT_X_CT_COM_MONITOR_
/*************************************************************************************************************************/
/***********_PRMT_X_CT_COM_MONITOR_****************************************************************************/
/*************************************************************************************************************************/








/*************************************************************************************************************************/
/***********_PRMT_X_CT_COM_VPDN_****************************************************************************/
/*************************************************************************************************************************/
#ifdef _PRMT_X_CT_COM_VPDN_
struct CWMP_OP tCT_VPDNLeafOP = { getCT_VPDN, setCT_VPDN };
struct CWMP_PRMT tCT_VPDNLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_VPDNLeafOP}
};
enum eCT_VPDNLeaf
{
	eCTVPDN_Enable
};
struct CWMP_LEAF tCT_VPDNLeaf[] =
{
{ &tCT_VPDNLeafInfo[eCTVPDN_Enable] },
{ NULL }
};
int getCT_VPDN(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	unsigned int  vUint=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		mib_get( CWMP_CT_VPDN_ENABLE, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setCT_VPDN(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	if( strcmp( lastname, "Enable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif

		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(CWMP_CT_VPDN_ENABLE, (void *)&vChar);
	}else{
		return ERR_9005;
	}
	
	return 0;
}
#endif //_PRMT_X_CT_COM_VPDN_
/*************************************************************************************************************************/
/***********END _PRMT_X_CT_COM_VPDN_****************************************************************************/
/*************************************************************************************************************************/










/*************************************************************************************************************************/
/***********_PRMT_X_CT_COM_MWBAND_****************************************************************************/
/*************************************************************************************************************************/
#ifdef _PRMT_X_CT_COM_MWBAND_
struct CWMP_OP tCT_MWBANDLeafOP = { getCT_MWBAND, setCT_MWBAND };
struct CWMP_PRMT tCT_MWBANDLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Mode",			eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP},
{"TotalTerminalNumber",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP},
{"STBRestrictEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP},
{"STBNumber",			eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP},
{"CameraRestrictEnable",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP},
{"CameraNumber",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP},
{"ComputerRestrictEnable",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP},
{"ComputerNumber",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP},
{"PhoneRestrictEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP},
{"PhoneNumber",			eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP}
};
enum eCT_MWBANDLeaf
{
	eCTMMode,
	eCTMTotalTerminalNumber,
	eCTMSTBRestrictEnable,
	eCTMSTBNumber,
	eCTMCameraRestrictEnable,
	eCTMCameraNumber,
	eCTMComputerRestrictEnable,
	eCTMComputerNumber,
	eCTMPhoneRestrictEnable,
	eCTMPhoneNumber
};
struct CWMP_LEAF tCT_MWBANDLeaf[] =
{
{ &tCT_MWBANDLeafInfo[eCTMMode] },
{ &tCT_MWBANDLeafInfo[eCTMTotalTerminalNumber] },
{ &tCT_MWBANDLeafInfo[eCTMSTBRestrictEnable] },
{ &tCT_MWBANDLeafInfo[eCTMSTBNumber] },
{ &tCT_MWBANDLeafInfo[eCTMCameraRestrictEnable] },
{ &tCT_MWBANDLeafInfo[eCTMCameraNumber] },
{ &tCT_MWBANDLeafInfo[eCTMComputerRestrictEnable] },
{ &tCT_MWBANDLeafInfo[eCTMComputerNumber] },
{ &tCT_MWBANDLeafInfo[eCTMPhoneRestrictEnable] },
{ &tCT_MWBANDLeafInfo[eCTMPhoneNumber] },
{ NULL }
};
int getCT_MWBAND(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	unsigned int  vUint=0;
	int	vInt=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Mode" )==0 )
	{
		mib_get( CWMP_CT_MWBAND_MODE, (void *)&vInt);
		*data = intdup( vInt );
	}else if( strcmp( lastname, "TotalTerminalNumber" )==0 )
	{
		mib_get( CWMP_CT_MWBAND_NUMBER, (void *)&vUint);
		*data = uintdup( vUint );
	}else if( strcmp( lastname, "STBRestrictEnable" )==0 )
	{
		mib_get( CWMP_CT_MWBAND_STB_ENABLE, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else if( strcmp( lastname, "STBNumber" )==0 )
	{
		mib_get( CWMP_CT_MWBAND_STB_NUM, (void *)&vUint);
		*data = uintdup( vUint );
	}else if( strcmp( lastname, "CameraRestrictEnable" )==0 )
	{
		mib_get( CWMP_CT_MWBAND_CMR_ENABLE, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else if( strcmp( lastname, "CameraNumber" )==0 )
	{
		mib_get( CWMP_CT_MWBAND_CMR_NUM, (void *)&vUint);
		*data = uintdup( vUint );
	}else if( strcmp( lastname, "ComputerRestrictEnable" )==0 )
	{
		mib_get( CWMP_CT_MWBAND_PC_ENABLE, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else if( strcmp( lastname, "ComputerNumber" )==0 )
	{
		mib_get( CWMP_CT_MWBAND_PC_NUM, (void *)&vUint);
		*data = uintdup( vUint );
	}else if( strcmp( lastname, "PhoneRestrictEnable" )==0 )
	{
		mib_get( CWMP_CT_MWBAND_PHN_ENABLE, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else if( strcmp( lastname, "PhoneNumber" )==0 )
	{
		mib_get( CWMP_CT_MWBAND_PHN_NUM, (void *)&vUint);
		*data = uintdup( vUint );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setCT_MWBAND(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif

	if( strcmp( lastname, "Mode" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpint;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		if( *i<0 || *i>2 ) return ERR_9007;
		mib_set(CWMP_CT_MWBAND_MODE, (void *)i);
	}else if( strcmp( lastname, "TotalTerminalNumber" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *i = &tmpuint;		
#else
		unsigned int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		mib_set( CWMP_CT_MWBAND_NUMBER, (void *)i);
	}else if( strcmp( lastname, "STBRestrictEnable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(CWMP_CT_MWBAND_STB_ENABLE, (void *)&vChar);
	}else if( strcmp( lastname, "STBNumber" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *i = &tmpuint;		
#else
		unsigned int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		mib_set( CWMP_CT_MWBAND_STB_NUM, (void *)i);
	}else if( strcmp( lastname, "CameraRestrictEnable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(CWMP_CT_MWBAND_CMR_ENABLE, (void *)&vChar);
	}else if( strcmp( lastname, "CameraNumber" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *i = &tmpuint;		
#else
		unsigned int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		mib_set( CWMP_CT_MWBAND_CMR_NUM, (void *)i);
	}else if( strcmp( lastname, "ComputerRestrictEnable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(CWMP_CT_MWBAND_PC_ENABLE, (void *)&vChar);
	}else if( strcmp( lastname, "ComputerNumber" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *i = &tmpuint;		
#else
		unsigned int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		mib_set( CWMP_CT_MWBAND_PC_NUM, (void *)i);		
	}else if( strcmp( lastname, "PhoneRestrictEnable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(CWMP_CT_MWBAND_PHN_ENABLE, (void *)&vChar);
	}else if( strcmp( lastname, "PhoneNumber" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *i = &tmpuint;		
#else
		unsigned int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		mib_set( CWMP_CT_MWBAND_PHN_NUM, (void *)i);
	}else{
		return ERR_9005;
	}
	
#ifdef CONFIG_CTC_E8_CLIENT_LIMIT
	proc_write_for_mwband();
	return 0;
#else
	return 1;
#endif
}
#endif //_PRMT_X_CT_COM_MWBAND_
/*************************************************************************************************************************/
/***********END _PRMT_X_CT_COM_MWBAND_****************************************************************************/
/*************************************************************************************************************************/

