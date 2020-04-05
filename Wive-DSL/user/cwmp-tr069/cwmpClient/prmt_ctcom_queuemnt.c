#include "prmt_ctcom_queuemnt.h"
#include "prmt_queuemnt.h"

int getClassIndex();

#ifdef _PRMT_X_CT_COM_QOS_

#define IPQOS_APP_NUM  2

extern unsigned int getInstNum( char *name, char *objname );
unsigned int getCT_AppInstNum( char *name );
unsigned int getCT_QueueInstNum( char *name );
unsigned int getCT_ClassInstNum( char *name );
unsigned int findCT_ClassInstNum(void);
int getCT_ClassEntryByInstNum(unsigned int instnum, MIB_CE_IP_QUEUE_RULE_T *p, unsigned int *id);
int getCT_ClassEntryByAppindex(unsigned int appindex, MIB_CE_IP_QUEUE_RULE_T *p, unsigned int *id);
int getCT_ClassEntryByMode(unsigned int mode, MIB_CE_IP_QUEUE_RULE_T *p, unsigned int *id);
int getTr069Index();
int getIPTVIndex();
void deloldMode(unsigned char mode);
void setPvcQos(int ifIndex);
void updateQos();


struct CWMP_OP tCT_QueueEntityLeafOP = { getCT_QueueEntity, setCT_QueueEntity };
struct CWMP_PRMT tCT_QueueEntityLeafInfo[] =
{
/*(name,		type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,		&tCT_QueueEntityLeafOP},
{"Priority",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCT_QueueEntityLeafOP},
{"Weight",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,		&tCT_QueueEntityLeafOP},
};
enum eCT_QueueEntityLeaf
{
	eCT_QE_Enable,
	eCT_QE_Priority,
	eCT_QE_Weight,
};
struct CWMP_LEAF tCT_QueueEntityLeaf[] =
{
{ &tCT_QueueEntityLeafInfo[eCT_QE_Enable] },
{ &tCT_QueueEntityLeafInfo[eCT_QE_Priority] },
{ &tCT_QueueEntityLeafInfo[eCT_QE_Weight] },
{ NULL }
};

struct CWMP_PRMT tCT_QueueObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eCT_QueueObject
{
	eCT_QueueObject0
};
struct CWMP_LINKNODE tCT_QueueObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tCT_QueueObjectInfo[eCT_QueueObject0],	tCT_QueueEntityLeaf,	NULL,		NULL,			0},
};


#if 0
struct sCWMP_ENTITY tPolicerEntity[] =
{
/*(name,		type,		flag,			accesslist,	getvalue,	setvalue,	next_table,	sibling)*/
{"PolicerKey",		eCWMP_tUINT,	CWMP_READ,		NULL,		getPolicerEntity,NULL,		NULL,		NULL},
{"PolicerEnable",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	NULL,		getPolicerEntity,setPolicerEntity,NULL,		NULL},
{"PolicerStatus",	eCWMP_tSTRING,	CWMP_READ,		NULL,		getPolicerEntity,NULL,		NULL,		NULL},
{"CommittedRate",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	NULL,		getPolicerEntity,setPolicerEntity,NULL,		NULL},
{"CommittedBurstSize",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	NULL,		getPolicerEntity,setPolicerEntity,NULL,		NULL},
/*ExcessBurstSize*/
/*PeakRate*/
/*PeakBurstSize*/
{"MeterType",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	NULL,		getPolicerEntity,setPolicerEntity,NULL,		NULL},
{"PossibleMeterTypes",	eCWMP_tSTRING,	CWMP_READ,		NULL,		getPolicerEntity,NULL,		NULL,		NULL},
{"ConformingAction",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	NULL,		getPolicerEntity,setPolicerEntity,NULL,		NULL},
/*PartialConformingAction*/
{"NonConformingAction",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	NULL,		getPolicerEntity,setPolicerEntity,NULL,		NULL},
{"CountedPackets",	eCWMP_tUINT,	CWMP_READ,		NULL,		getPolicerEntity,NULL,		NULL,		NULL},
{"CountedBytes",	eCWMP_tUINT,	CWMP_READ,		NULL,		getPolicerEntity,NULL,		NULL,		NULL},
{"",			eCWMP_tNONE,	0,			NULL,		NULL,		NULL,		NULL,		NULL}
};

struct sCWMP_ENTITY tPolicer[] =
{
/*(name,			type,		flag,					accesslist,	getvalue,	setvalue,	next_table,	sibling)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL,		NULL,		NULL,		tPolicerEntity,	NULL}
//{"1",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL,		NULL,		NULL,		tPolicerEntity,	NULL}
};
#endif


struct CWMP_OP tCT_ClassEntityLeafOP = { getCT_ClassEntity, setCT_ClassEntity };
struct CWMP_PRMT tCT_ClassEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"ClassQueue",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,		&tCT_ClassEntityLeafOP},
{"Type",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_ClassEntityLeafOP},
{"Max",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,		&tCT_ClassEntityLeafOP},
{"Min",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_ClassEntityLeafOP},
{"ProtocolList",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_ClassEntityLeafOP},
{"DSCPMarkValue",			eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCT_ClassEntityLeafOP},
{"802-1_P_Value",			eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCT_ClassEntityLeafOP},
};
enum eCT_ClassEntityLeaf
{
	eCT_QC_ClassQueue,
	eCT_QC_Type,
	eCT_QC_Max,
	eCT_QC_Min,
	eCT_QC_ProtocolList,
	eCT_QC_DSCPMarkValue,
	eCT_QC_8021_P_Value,
};
struct CWMP_LEAF tCT_ClassEntityLeaf[] =
{
{ &tCT_ClassEntityLeafInfo[eCT_QC_ClassQueue] },
{ &tCT_ClassEntityLeafInfo[eCT_QC_Type] },
{ &tCT_ClassEntityLeafInfo[eCT_QC_Max] },
{ &tCT_ClassEntityLeafInfo[eCT_QC_Min] },
{ &tCT_ClassEntityLeafInfo[eCT_QC_ProtocolList] },
{ &tCT_ClassEntityLeafInfo[eCT_QC_DSCPMarkValue] },
{ &tCT_ClassEntityLeafInfo[eCT_QC_8021_P_Value] },
{ NULL }
};

struct CWMP_PRMT tCT_ClassObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eCT_ClassObject
{
	eCT_ClassObject0
};
struct CWMP_LINKNODE tCT_ClassObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tCT_ClassObjectInfo[eCT_ClassObject0],	tCT_ClassEntityLeaf,	NULL,		NULL,			0},
};


struct CWMP_OP tCT_AppEntityLeafOP = { getCT_AppEntity, setCT_AppEntity };
struct CWMP_PRMT tCT_AppEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"AppName",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,		&tCT_AppEntityLeafOP},
{"ClassQueue",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCT_AppEntityLeafOP},
};
enum eCT_AppEntityLeaf
{
	eCT_QA_AppName,
	eCT_QA_ClassQueue,
};
struct CWMP_LEAF tCT_AppEntityLeaf[] =
{
{ &tCT_AppEntityLeafInfo[eCT_QA_AppName] },
{ &tCT_AppEntityLeafInfo[eCT_QA_ClassQueue] },
{ NULL }
};

struct CWMP_PRMT tCT_AppObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eCT_AppObject
{
	eCT_AppObject0
};
struct CWMP_LINKNODE tCT_AppObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tCT_AppObjectInfo[eCT_AppObject0],	tCT_AppEntityLeaf,	NULL,		NULL,			0},
};

struct CWMP_OP tCT_QueueMntLeafOP = { getCT_QueueMnt,	setCT_QueueMnt };
struct CWMP_PRMT tCT_QueueMntLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Mode",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_QueueMntLeafOP},
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,		&tCT_QueueMntLeafOP},
{"Bandwidth",	     eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,		&tCT_QueueMntLeafOP},
{"Plan",      eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,		&tCT_QueueMntLeafOP},
{"EnableForceWeight",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,		&tCT_QueueMntLeafOP},
{"EnableDSCPMark",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,		&tCT_QueueMntLeafOP},
{"Enable802-1_P",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,		&tCT_QueueMntLeafOP},
};
enum eCT_QueueMntLeaf
{
	eCT_Q_Mode,
	eCT_Q_Enable,
	eCT_Q_Bandwidth,
	eCT_Q_Plan,
	eCT_Q_EnableForceWeight,
	eCT_Q_EnableDSCPMark,
	eCT_Q_Enable8021_P,
};
struct CWMP_LEAF tCT_QueueMntLeaf[] =
{
{ &tCT_QueueMntLeafInfo[eCT_Q_Mode] },
{ &tCT_QueueMntLeafInfo[eCT_Q_Enable] },
{ &tCT_QueueMntLeafInfo[eCT_Q_Bandwidth] },
{ &tCT_QueueMntLeafInfo[eCT_Q_Plan] },
{ &tCT_QueueMntLeafInfo[eCT_Q_EnableForceWeight] },
{ &tCT_QueueMntLeafInfo[eCT_Q_EnableDSCPMark] },
{ &tCT_QueueMntLeafInfo[eCT_Q_Enable8021_P] },
{ NULL }
};

struct CWMP_OP tCT_QM_App_OP = { NULL, objCT_App };
struct CWMP_OP tCT_QM_Class_OP = { NULL, objCT_Class };
//struct CWMP_OP tQM_Policer_OP = { NULL, objPolicer };
struct CWMP_OP tCT_QM_Queue_OP = { NULL, objCT_Queue };
struct CWMP_PRMT tCT_QueueMntObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"App",		eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tCT_QM_App_OP},
{"Classification",		eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tCT_QM_Class_OP},
//{"Policer",			eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tQM_Policer_OP},
{"PriorityQueue",			eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tCT_QM_Queue_OP}
};
enum eCT_QueueMntObject
{
	eCT_Q_App,
	eCT_Q_Classification,
	//eQ_Policer,
	eCT_Q_Queue
};
struct CWMP_NODE tCT_QueueMntObject[] =
{
/*info,  					leaf,			node)*/
{&tCT_QueueMntObjectInfo[eCT_Q_App],	NULL,			NULL},
{&tCT_QueueMntObjectInfo[eCT_Q_Classification],	NULL,			NULL},
{&tCT_QueueMntObjectInfo[eCT_Q_Queue],		NULL,			NULL},
{NULL,						NULL,			NULL}
};


int getCT_QueueEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char 	buff[256]={0};
	unsigned char vChar=0;
	unsigned int  qinst=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	qinst = getCT_QueueInstNum( name );
	if(qinst==0) return ERR_9005;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "Enable" )==0 )
	{
		*data=booldup( 1 );
	}else if( strcmp( lastname, "Priority" )==0 )
	{
		 *data=uintdup( qinst );
	}else if( strcmp( lastname, "Weight" )==0 )
	{
		 *data=uintdup( 0 );
	}else{
		return ERR_9005;
	}

	return 0;
}


int setCT_QueueEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned int  qinst=0;
	
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

	qinst = getCT_QueueInstNum( name );
	if(qinst==0) return ERR_9005;

	if( strcmp( lastname, "Enable" )==0 ){
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if(i==NULL) return ERR_9007;
		return 0;
		
	}else if( strcmp( lastname, "Priority" )==0 ){
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *i = &tmpuint;		
#else
		unsigned int *i = data;
#endif
		if(i==NULL) return ERR_9007;
		if(*i!=qinst)  return ERR_9007;
		return 0;
		
	}else if( strcmp( lastname, "Weight" )==0 ){
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *i = &tmpuint;		
#else
		unsigned int *i = data;
#endif
		if(i==NULL) return ERR_9007;
		if(*i<0 || *i>100)   return ERR_9007;
		return 0;
		
	}else{
		return ERR_9005;
	}

	return 0;
}


int objCT_Queue(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);

	switch( type )
	{
	case eCWMP_tINITOBJ:
	     {
	     	int num=0,MaxInstNum=0,i;
	     	struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;
	     	
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
	     	
	     	num=IPQOS_NUM_PRIOQ;
		for( i=0; i<num;i++ )
		{
			MaxInstNum = i+1;
			if( create_Object( c, tCT_QueueObject, sizeof(tCT_QueueObject), 1, MaxInstNum ) < 0 )
				return -1;
		}
		printf("\nmaxnum=%d\n",MaxInstNum);
		add_objectNum( name, MaxInstNum );
		return 0;
	     }
	case eCWMP_tADDOBJ:
	     {
		return ERR_9001;
	     }
	case eCWMP_tDELOBJ:
	     {
		return ERR_9001;
	     }
	case eCWMP_tUPDATEOBJ:	
	     {
	     	return 0;
	     }
	}
	return -1;
}


char *LANString[4]={	"InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1", //eth0_sw0
			"InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.2", //eth0_sw1
			"InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.3", //eth0_sw2
			"InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.4"  //eth0_sw3
		 };

int getCT_ClassEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	unsigned int  instnum=0,chainid=0;
	char buf[256]={0};
	MIB_CE_IP_QUEUE_RULE_T *p, qos_entity;
	int ret;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	instnum=getCT_ClassInstNum(name);
	if(instnum!=1) return ERR_9005;
	p = &qos_entity;
	ret=getCT_ClassEntryByMode( MODEIPTV, p, &chainid );
	//if(p->InstanceNum!=instnum)   return ERR_9005;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "ClassQueue" )==0 )
	{
		if(ret<0)
			*data = uintdup(4);
		else
			*data = uintdup( p->prio );
	}else if( strcmp( lastname, "Type" )==0 )
	{
		if(ret<0)
			*data=strdup( "" );
		else{
			if(p->modeTr69==MODEIPTV)
			 	*data=strdup( "LANInterface" );
			else
				*data=strdup( "" );
		}
	}else if( strcmp( lastname, "Max" )==0 )
	{
		if(ret<0)
			*data=strdup( "" );
		else{
			if((p->modeTr69==MODEIPTV) && (p->phyPort>0 && p->phyPort<5))
			 	*data=strdup( LANString[p->phyPort-1] );
			else
				*data=strdup( "" );
		}
	}else if( strcmp( lastname, "Min" )==0 )
	{
		if(ret<0)
			*data=strdup( "" );
		else{
			if((p->modeTr69==MODEIPTV) && (p->phyPort>0 && p->phyPort<5))
			 	*data=strdup( LANString[p->phyPort] );
			else
				*data=strdup( "" );
		}
	}else if( strcmp( lastname, "ProtocolList" )==0 )
	{
		if(ret<0)
			*data=strdup( "" );
		else{
			switch(p->protoType){
				case 1:
					strcpy(buf,"ICMP");
					break;
				case 2:
					strcpy(buf,"TCP");
					break;
				case 3:
					strcpy(buf,"UDP");
					break;
				case 4:
					strcpy(buf,"TCP,UDP");
					break;
				default:
					strcpy(buf,"");
					break;
			}
			*data = strdup(buf);
		}
	}else if( strcmp( lastname, "DSCPMarkValue" )==0 )
	{
		if(ret<0)
			*data=uintdup( 0 );
		else{
			if( p->m_dscp==0 )
			 	*data=uintdup( 0 );
			else
				*data=uintdup( ((int)p->m_dscp)/4 );
		}
	}else if( strcmp( lastname, "802-1_P_Value" )==0 )
	{
		if(ret<0)
			*data=uintdup( 0 );
		else{
			if( p->m_1p==0 )
			 	*data=uintdup( 0 );
			else
				*data=uintdup( (int)p->m_1p-1 );
		}
	}else{
		return ERR_9005;
	}

	return 0;
}

int setCT_ClassEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;
	unsigned int  instnum=0,chainid=0;
	MIB_CE_IP_QUEUE_RULE_T *p, qos_entity;
	struct in_addr in;
	char	*pzeroip="0.0.0.0";
	
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

	instnum=getCT_ClassInstNum(name);
	if(instnum!=1) return ERR_9005;
	p = &qos_entity;
	if(getCT_ClassEntryByMode( MODEIPTV, p, &chainid )<0) return ERR_9001;

	if( strcmp( lastname, "ClassQueue" )==0 )
	{	
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *i = &tmpuint;		
#else
		unsigned int *i = data;
#endif
		if(i==NULL) return ERR_9007;

		p->prio=*i;	
		mib_chain_update(MIB_IP_QUEUE_RULE_TBL,p,chainid);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, sizeof(MIB_CE_IP_QUEUE_RULE_T) );
		return 0;
#else
		return 1;
#endif
	}
	else if( strcmp( lastname, "Type" )==0 )
	{	
		
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;

		if(!strcmp(buf,"LANInterface"))
			return 0;
		else
			return ERR_9001;
	}
	else if( strcmp( lastname, "Max" )==0 )
	{	
		int i;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;

		for(i=0;i<4;i++){
			if(strstr(buf,LANString[i])!=0)
				break;
		}
		if(i<=4)
			p->phyPort=i+1;
		else
			return ERR_9007;
		mib_chain_update(MIB_IP_QUEUE_RULE_TBL,p,chainid);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, sizeof(MIB_CE_IP_QUEUE_RULE_T) );
		return 0;
#else
		return 1;
#endif
	}
	else if( strcmp( lastname, "Min" )==0 )
	{	
		int i;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;

		for(i=0;i<4;i++){
			if(strstr(buf,LANString[i])!=0)
				break;
		}
		if(i<=4)
			p->phyPort=i;
		else
			return ERR_9007;
		mib_chain_update(MIB_IP_QUEUE_RULE_TBL,p,chainid);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, sizeof(MIB_CE_IP_QUEUE_RULE_T) );
		return 0;
#else
		return 1;
#endif
	}
	else if( strcmp( lastname, "ProtocolList" )==0 )
	{	
		int i;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;

		if(strstr(buf,"TCP") && strstr(buf,"UDP"))
			p->protoType=4;
		else if(strstr(buf,"ICMP"))
			p->protoType=1;
		else if(strstr(buf,"TCP"))
			p->protoType=2;
		else if(strstr(buf,"UDP"))
			p->protoType=3;
		else 
			return ERR_9007;
		mib_chain_update(MIB_IP_QUEUE_RULE_TBL,p,chainid);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, sizeof(MIB_CE_IP_QUEUE_RULE_T) );
		return 0;
#else
		return 1;
#endif			
	}
	else if( strcmp( lastname, "DSCPMarkValue" )==0 )
	{	
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *i = &tmpuint;		
#else
		unsigned int *i = data;
#endif
		if(i==NULL) return ERR_9007;
		if( *i<0  || *i>63 ) return ERR_9007;
		p->m_dscp = (unsigned char)(*i)*4;
		mib_chain_update( MIB_IP_QUEUE_RULE_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, sizeof(MIB_CE_IP_QUEUE_RULE_T) );
		return 0;
#else
		return 1;
#endif		
	}
	else if( strcmp( lastname, "802-1_P_Value" )==0 )
	{	
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *i = &tmpuint;		
#else
		unsigned int *i = data;
#endif
		if(i==NULL) return ERR_9007;
		if( *i<-1 || *i>7 ) return ERR_9007;
		p->m_1p = (unsigned char)(*i+1);
		mib_chain_update( MIB_IP_QUEUE_RULE_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, sizeof(MIB_CE_IP_QUEUE_RULE_T) );
		return 0;
#else
		return 1;
#endif	
	}
	else
		return ERR_9005;
	return 0;
}


int objCT_Class(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);

	switch( type )
	{
	case eCWMP_tINITOBJ:
		{
	     	int num=0,MaxInstNum=0,i;
	     	struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;
	     	
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
	     	
	     	num=1;
		for( i=0; i<num;i++ )
		{
			MaxInstNum = i+1;
			if( create_Object( c, tCT_ClassObject, sizeof(tCT_ClassObject), 1, MaxInstNum ) < 0 )
				return -1;
		}
		add_objectNum( name, MaxInstNum );
		return 0;
	     }
	/*
	     {
		unsigned int num=0,MaxInstNum=0,i;
		struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;
		MIB_CE_IP_QUEUE_RULE_T *p, qos_entity;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		MaxInstNum = findCT_ClassInstNum();
		num = mib_chain_total( MIB_IP_QUEUE_RULE_TBL );
		for( i=0; i<num;i++ )
		{
			p = &qos_entity;
			if( !mib_chain_get( MIB_IP_QUEUE_RULE_TBL, i, (void*)p ) )
				continue;

			if(p->modeTr69!=MODEIPTV)
				continue;
			
			if( p->InstanceNum==0 ) //maybe createn by web or cli
			{
				MaxInstNum++;
				p->InstanceNum = MaxInstNum;
				mib_chain_update( MIB_IP_QUEUE_RULE_TBL, (unsigned char*)p, i );
			}
			if( create_Object( c, tCT_ClassObject, sizeof(tCT_ClassObject), 1, p->InstanceNum ) < 0 )
				return -1;
			//c = & (*c)->sibling;
		}
		add_objectNum( name, MaxInstNum );
	
		return 0;	     		     	
	     }
	*/
	case eCWMP_tADDOBJ:
		/*
	     {
	     	int ret;
	     	unsigned int num=0;
	     	
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
		num = mib_chain_total( MIB_IP_QOS_TBL );
		if(num>=MAX_QOS_RULE) return ERR_9004;
		
		ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tCT_ClassObject, sizeof(tCT_ClassObject), data );
		if( ret >= 0 )
		{
			MIB_CE_IP_QOS_T qos_entry;
			memset( &qos_entry, 0, sizeof( MIB_CE_IP_QOS_T ) );
			qos_entry.InstanceNum = *(unsigned int*)data;
			qos_entry.phyPort=0xff;
			qos_entry.prior=3;
			qos_entry.m_iptos=0xff;
			mib_chain_add( MIB_IP_QOS_TBL, (unsigned char*)&qos_entry );
		}
		return ret;	
	     }*/
	     	return ERR_9001;
	case eCWMP_tDELOBJ:
		/*
	     {
		unsigned int i,num;
		int ret=0;
		MIB_CE_IP_QOS_T *p,qos_entity;
	     	
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
	
		num = mib_chain_total( MIB_IP_QOS_TBL );
		for( i=0; i<num;i++ )
		{
			p = &qos_entity;
			if( !mib_chain_get( MIB_IP_QOS_TBL, i, (void*)p ))
				continue;
			if( p->InstanceNum == *(unsigned int*)data )
				break;
		}	     	
		if(i==num) return ERR_9005;
		
#ifdef _CWMP_APPLY_
		//why don't use CWMP_RESTART?
		//for IPQoS, use the chain index to count the mark number, get_classification_mark()
		//after deleting one rule, the mark numbers will change
		//hence, stop all rules=>delete one rule=>start the rest rules;
#ifdef IP_QOS
		apply_IPQoSRule( CWMP_STOP, -1, NULL );
#endif
#endif
	     	mib_chain_delete( MIB_IP_QOS_TBL, i );
		ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
#ifdef _CWMP_APPLY_
#ifdef IP_QOS
		apply_IPQoSRule( CWMP_START, -1, NULL );
#endif
#else
		if( ret == 0 ) return 1;
#endif
		return ret;
	     }
	     */
	     return ERR_9001;
	case eCWMP_tUPDATEOBJ:	
	/*
	     {
	     	int num=0,i;
	     	struct CWMP_LINKNODE *old_table;
	     	
	     	num = mib_chain_total( MIB_IP_QUEUE_RULE_TBL );
	     	old_table = (struct CWMP_LINKNODE *)entity->next;
	     	entity->next = NULL;
	     	for( i=0; i<num;i++ )
	     	{
	     		struct CWMP_LINKNODE *remove_entity=NULL;
			MIB_CE_IP_QUEUE_RULE_T *p,qos_entity;

			p = &qos_entity;
			if( !mib_chain_get( MIB_IP_QUEUE_RULE_TBL, i, (void*)p ) )
				continue;

			if( p->modeTr69!=MODEIPTV)
				continue;
			
			remove_entity = remove_SiblingEntity( &old_table, p->InstanceNum );
			if( remove_entity!=NULL )
			{
				add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
			}else{ 
				unsigned int MaxInstNum=p->InstanceNum;
					
				add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tCT_ClassObject, sizeof(tCT_ClassObject), &MaxInstNum );
				if(MaxInstNum!=p->InstanceNum)
				{
					p->InstanceNum = MaxInstNum;
					mib_chain_update( MIB_IP_QUEUE_RULE_TBL, (unsigned char*)p, i );
				}
			}	
	     	}
	     	
	     	if( old_table )
	     		destroy_ParameterTable( (struct CWMP_NODE *)old_table );	     	
	     	return 0;
	     }
	     */
	     return 0;
	}
	return -1;
}

int getCT_AppEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	unsigned int  instnum=0,chainid=0;
	char buf[256]={0};
	MIB_CE_IP_QUEUE_RULE_T *p, qos_entity;
	p=&qos_entity;
	int ret;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	instnum=getCT_AppInstNum(name);
	if(instnum==0) return ERR_9005;
	ret=getCT_ClassEntryByAppindex( instnum, p, &chainid );

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "AppName" )==0 )
	{
		if(ret<0)
			*data = strdup(" ");
		else{
			switch(p->modeTr69){
				case MODEINTERNET:
					strcpy(buf,"");
					break;
				case MODEVOIP:
					strcpy(buf,"VOIP");
					break;
				case MODETR069:
					strcpy(buf,"TR069");
					break;
				default:
					strcpy(buf,"");
					break;
			}
			*data = strdup( buf );
		}
	}else if( strcmp( lastname, "ClassQueue" )==0 )
	{
		if(ret<0)
			*data=uintdup(4);
		else
			 *data=uintdup( p->prio );
	}else{
		return ERR_9005;
	}

	return 0;
}

int setCT_AppEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;
	unsigned int  instnum=0,chainid=0;
	MIB_CE_IP_QUEUE_RULE_T *p, qos_entity;
	struct in_addr in;
	char	*pzeroip="0.0.0.0";
	int ret;
	
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
	instnum=getCT_AppInstNum(name);
	if(instnum==0) return ERR_9005;
	p=&qos_entity;
	ret=getCT_ClassEntryByAppindex( instnum, p, &chainid );

	if( strcmp( lastname, "AppName" )==0 )
	{	
		unsigned char mode=MODEINTERNET;
		int chainindex=0;
		MIB_CE_IP_QUEUE_RULE_T *pqos, tmp_qos_entity;
		pqos=&tmp_qos_entity;
		if( buf==NULL ) return ERR_9007;

		if(!strcmp(buf,"TR069"))
			mode=MODETR069;
		else if(!strcmp(buf,"VOIP"))
			mode=MODEVOIP;
		else
			return 0;

		if(getCT_ClassEntryByMode( mode, pqos, &chainindex )<0)   return ERR_9003;

		pqos->appindex=instnum;

		if(mode==MODETR069){
			char vStr[256+1];
			char acsurl[256+1];
			struct hostent *host;
			struct in_addr acsaddr;
			char DstIp[20]={0};

			mib_get(CWMP_ACS_URL,(void*)vStr);
			set_endpoint(acsurl,vStr);
			host=gethostbyname(acsurl);
			if(host==NULL)
				return ERR_9002;
			memcpy((char *) &(acsaddr.s_addr), host->h_addr_list[0], host->h_length);
			pqos->dstip = acsaddr.s_addr;
		}

		mib_chain_update(MIB_IP_QUEUE_RULE_TBL,pqos,chainindex);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, sizeof(MIB_CE_IP_QUEUE_RULE_T) );
		return 0;
#else
		return 1;
#endif
	}
	else if( strcmp( lastname, "ClassQueue" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *i = &tmpuint;		
#else
		unsigned int *i = data;
#endif
		if(i==NULL) return ERR_9007;

		if(ret<0)
			return ERR_9001;

		p->prio=*i;

		mib_chain_update(MIB_IP_QUEUE_RULE_TBL,p,chainid);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, sizeof(MIB_CE_IP_QUEUE_RULE_T) );
		return 0;
#else
		return 1;
#endif
	}
	else
		return ERR_9005;
	
	return 0;
}

int objCT_App(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);

	switch( type )
	{
	case eCWMP_tINITOBJ:
	     {
	     	int num=0,MaxInstNum=0,i;
	     	struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;
	     	
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
	     	
	     	num=IPQOS_APP_NUM;
		for( i=0; i<num;i++ )
		{
			MaxInstNum = i+1;
			if( create_Object( c, tCT_AppObject, sizeof(tCT_AppObject), 1, MaxInstNum ) < 0 )
				return -1;
		}
		printf("\nmaxnum=%d\n",MaxInstNum);
		add_objectNum( name, MaxInstNum );
		return 0;
	     }
	case eCWMP_tADDOBJ:
	     
		return ERR_9001;

	case eCWMP_tDELOBJ:
		
	     return ERR_9001;
		 
	case eCWMP_tUPDATEOBJ:	
	     return 0;
	}
	return -1;
}

int getCT_QueueMnt(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char 	buff[256]={0};
	unsigned char vChar=0;
	unsigned int num=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	MIB_CE_IP_QUEUE_RULE_T *p, qos_entity;
	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "Mode" )==0 )
	{
		int i,total;
		MIB_CE_ATM_VC_T entry;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
		
		total=mib_chain_total(MIB_ATM_VC_TBL);
		for(i=0;i<total;i++){
			if(!mib_chain_get(MIB_ATM_VC_TBL,i,&entry))
				continue;
			if(entry.dgw==1&&entry.qosenable==1){
				sprintf(buff,"%s","INTERNET");
				break;
			}
		}

		num = mib_chain_total( MIB_IP_QUEUE_RULE_TBL );
		for( i=0; i<num;i++ )
		{
			p = &qos_entity;
			if( !mib_chain_get( MIB_IP_QUEUE_RULE_TBL, i, (void*)p ) )
				continue;

			if(p->modeTr69==MODEINTERNET)
				continue;

			printf("\np->modeTr69=%d\n",p->modeTr69);

			switch(p->modeTr69){
				case MODETR069:
					if(buff[0]==0)
						strcat(buff,"TR069");
					else
						strcat(buff,",TR069");
					break;
				case MODEIPTV:
					if(buff[0]==0)
						strcat(buff,"IPTV");
					else
						strcat(buff,",IPTV");
					break;
				case MODEVOIP:
					if(buff[0]==0)
						strcat(buff,"VOIP");
					else
						strcat(buff,",VOIP");
					break;
				case MODEOTHER:
					if(buff[0]==0)
						strcat(buff,"OTHER");
					else
						strcat(buff,",OTHER");
					break;
				default:
					break;
			}
		}
		*data = strdup( buff );
	}
	else if( strcmp( lastname, "Enable" )==0 ){
		int enable=0;


	}
	else if( strcmp( lastname, "Bandwidth" )==0 ){
		*data = uintdup(0);
	}
	else if( strcmp( lastname, "Plan" )==0 ){
		unsigned char policy=0;
		mib_get(MIB_QOS_QUEUE_POLICY, &policy);
		if(policy==0)
		 	*data=strdup( "priority" );
		else
			*data=strdup( "weight" );
	}
	else if( strcmp( lastname, "EnableForceWeight" )==0 ){
		*data = booldup(0);
	}
	else if( strcmp( lastname, "EnableDSCPMark" )==0 ){
		*data = booldup(0);
	}
	else if( strcmp( lastname, "Enable802-1_P" )==0 ){
		*data = uintdup(0);
	}
	else{
		return ERR_9005;
	}

	return 0;
}

int setCT_QueueMnt(char *name, struct CWMP_LEAF *entity, int type, void *data)
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
		int enableinter=0;
		MIB_CE_IP_QUEUE_RULE_T *p, qos_entity;
		p=&qos_entity;
		
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;

		printf("\nset ct mode=%s\n",buf);

		deloldMode(MODETR069);
		deloldMode(MODEIPTV);
		deloldMode(MODEVOIP);
		deloldMode(MODEOTHER);

		if(strstr(buf,"TR069")!=0){
			memset(p,0,sizeof(MIB_CE_IP_QUEUE_RULE_T));
			p->index= getClassIndex();
			p->ifIndex=getTr069Index();
			if(p->ifIndex < 0)
				return ERR_9002;
			p->prio=4;
			p->modeTr69=MODETR069;
			p->state=0;
			p->appindex=1;
			sprintf(p->RuleName,"%s","rule_tr069");
			mib_chain_add(MIB_IP_QUEUE_RULE_TBL,p);
			setPvcQos(p->ifIndex);
		}
		if(strstr(buf,"IPTV")!=0){
			memset(p,0,sizeof(MIB_CE_IP_QUEUE_RULE_T));
			p->index= getClassIndex();
			p->ifIndex=getIPTVIndex();
			if(p->ifIndex < 0)
				return ERR_9002;
			p->prio=4;
			p->modeTr69=MODEIPTV;
			p->state=0;
			sprintf(p->RuleName,"%s","rule_iptv");
			mib_chain_add(MIB_IP_QUEUE_RULE_TBL,p);
			setPvcQos(p->ifIndex);
		}

		if(strstr(buf,"INTERNET")!=0)
			enableinter=1;
		else
			enableinter=0;
		{
			int i,total;
			MIB_CE_ATM_VC_T entry;
			total=mib_chain_total(MIB_ATM_VC_TBL);
			for(i=0;i<total;i++){
				if(!mib_chain_get(MIB_ATM_VC_TBL,i,&entry))
					continue;
				if(entry.dgw==1){
					entry.qosenable=enableinter;
					mib_chain_update(MIB_ATM_VC_TBL,&entry,i);	
					break;
				}
			}
		}
		
		updateQos();
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, sizeof(MIB_CE_IP_QUEUE_RULE_T) );
		return 0;
#else
		return 1;
#endif			
	}
	else if( strcmp( lastname, "Enable" )==0 ){
		int j,num;
		unsigned char enable;
		MIB_CE_IP_QUEUE_RULE_T *p, qos_entity;
		p=&qos_entity;
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if(i==NULL) return ERR_9007;
		enable=(*i==0)?0:1;
		num=mib_chain_total(MIB_IP_QUEUE_RULE_TBL);
		for(j=0;j<num;j++){
			if(!mib_chain_get(MIB_IP_QUEUE_RULE_TBL,j,p))
				continue;
			if(p->modeTr69!=MODEINTERNET)
				p->state=enable;
			mib_chain_update(MIB_IP_QUEUE_RULE_TBL,p,j);
		}
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, sizeof(MIB_CE_IP_QUEUE_RULE_T) );
		return 0;
#else
		return 1;
#endif	
		
	}
	else if( strcmp( lastname, "Bandwidth" )==0 ){
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *i = &tmpuint;		
#else
		unsigned int *i = data;
#endif
		if(i==NULL) return ERR_9007;
		return 0;		
	}
	else if( strcmp( lastname, "Plan" )==0 ){
		unsigned char policy=0;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if(!strcmp(buf,"priority"))  policy=0;
		else if(!strcmp(buf,"weight"))   policy=1;
		else return ERR_9007;
		if(!mib_set(MIB_QOS_QUEUE_POLICY, &policy))
			return ERR_9002;
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}
	else if( strcmp( lastname, "EnableForceWeight" )==0 ){
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if(i==NULL) return ERR_9007;
		return 0;		
	}
	else if( strcmp( lastname, "EnableDSCPMark" )==0 ){
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if(i==NULL) return ERR_9007;
		return 0;		
	}
	else if( strcmp( lastname, "Enable802-1_P" )==0 ){
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *i = &tmpuint;		
#else
		unsigned int *i = data;
#endif
		if(i==NULL) return ERR_9007;
		return 0;		
	}
	else{
		return ERR_9005;
	}

	return 0;
}


/**************************************************************************************/
/* utility functions*/
/**************************************************************************************/
unsigned int getCT_AppInstNum( char *name )
{
	return getInstNum( name, "App" );
}

unsigned int getCT_QueueInstNum( char *name )
{
	return getInstNum( name, "PriorityQueue" );
}

unsigned int getCT_ClassInstNum( char *name )
{
	return getInstNum( name, "Classification" );
}

unsigned int findCT_ClassInstNum(void)
{
	unsigned int ret=0, i,num;
	MIB_CE_IP_QUEUE_RULE_T *p,qos_entity;
	
	num = mib_chain_total( MIB_IP_QUEUE_RULE_TBL );
	for( i=0; i<num;i++ )
	{
		p = &qos_entity;
		if( !mib_chain_get( MIB_IP_QUEUE_RULE_TBL, i, (void*)p ))
			continue;
		if(p->modeTr69==MODEINTERNET)
			continue;
		if( p->InstanceNum > ret )
			ret = p->InstanceNum;
	}
	
	return ret;
}

int getCT_ClassEntryByInstNum(unsigned int instnum, MIB_CE_IP_QUEUE_RULE_T *p, unsigned int *id)
{
	int ret=-1;
	unsigned int i,num;
	
	if( (instnum==0) || (p==NULL) || (id==NULL) ) return ret;
	
	num = mib_chain_total( MIB_IP_QUEUE_RULE_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_IP_QUEUE_RULE_TBL, i, (void*)p ))
			continue;
		if(p->modeTr69==MODEINTERNET)
			continue;
		if( p->InstanceNum == instnum )
		{
			*id = i;
			ret=0;
			break;
		}
	}
	
	return ret;
}

int getCT_ClassEntryByAppindex(unsigned int appindex, MIB_CE_IP_QUEUE_RULE_T *p, unsigned int *id)
{
	int ret=-1;
	unsigned int i,num;
	
	if( (appindex==0) || (p==NULL) || (id==NULL) ) return ret;
	
	num = mib_chain_total( MIB_IP_QUEUE_RULE_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_IP_QUEUE_RULE_TBL, i, (void*)p ))
			continue;
		if(p->modeTr69==MODEINTERNET)
			continue;
		if( p->appindex == appindex )
		{
			*id = i;
			ret=0;
			break;
		}
	}
	
	return ret;
}

int getCT_ClassEntryByMode(unsigned int mode, MIB_CE_IP_QUEUE_RULE_T *p, unsigned int *id)
{
	int ret=-1;
	unsigned int i,num;
	
	if( (mode==0) || (p==NULL) || (id==NULL) ) return ret;
	
	num = mib_chain_total( MIB_IP_QUEUE_RULE_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_IP_QUEUE_RULE_TBL, i, (void*)p ))
			continue;
		if(p->modeTr69==MODEINTERNET)
			continue;
		if( p->modeTr69 == mode )
		{
			*id = i;
			ret=0;
			break;
		}
	}
	
	return ret;
}

int getTr069Index()
{
	MIB_CE_ATM_VC_T entry;
	int i,num,total;

	total=mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0;i<total;i++){
		if(!mib_chain_get(MIB_ATM_VC_TBL,i,&entry))
			continue;
		if(entry.applicationtype==2) //TR069
			return entry.ifIndex;
	}
	return -1;
}

int getIPTVIndex()
{
	MIB_CE_ATM_VC_T entry;
	int i,num,total;

	total=mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0;i<total;i++){
		if(!mib_chain_get(MIB_ATM_VC_TBL,i,&entry))
			continue;
		if(entry.applicationtype==3) //OTHER
			return entry.ifIndex;
	}
	return -1;
}

void deloldMode(unsigned char mode)
{
	MIB_CE_IP_QUEUE_RULE_T *p, qos_entity;
	int i,num;

	num = mib_chain_total( MIB_IP_QUEUE_RULE_TBL );
	for( i=0; i<num;i++ )
	{
		p = &qos_entity;
		if( !mib_chain_get( MIB_IP_QUEUE_RULE_TBL, i, (void*)p ) )
			continue;
		if(p->modeTr69==mode){
			MIB_CE_ATM_VC_T entry;
			int j,total;
			total=mib_chain_total(MIB_ATM_VC_TBL);
			for(j=0;j<total;j++){
				if(!mib_chain_get(MIB_ATM_VC_TBL,j,&entry))
					continue;
				if(entry.ifIndex==p->ifIndex)
					entry.qosenable=0;
				mib_chain_update(MIB_ATM_VC_TBL,&entry,j);
			}
			mib_chain_delete(MIB_IP_QUEUE_RULE_TBL,i);
		}
	}
}

void setPvcQos(int ifIndex){
	MIB_CE_ATM_VC_T entry;
	int i,total;
	total=mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0;i<total;i++){
		if(!mib_chain_get(MIB_ATM_VC_TBL,i,&entry))
			continue;
		if(entry.ifIndex==ifIndex)
			entry.qosenable=1;
		mib_chain_update(MIB_ATM_VC_TBL,&entry,i);
	}
}

void updateQos()
{
	MIB_CE_ATM_VC_T entry;
	int i,total;
	total=mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0;i<total;i++){
		if(!mib_chain_get(MIB_ATM_VC_TBL,i,&entry))
			continue;
		if(entry.qosenable==0){
			delIpQosQueue(entry.ifIndex);
			delIpQosRule(entry.ifIndex);
			delIpQosTrafficRule(entry.ifIndex);
		}else
			addIpQosQueue(entry.ifIndex);	
	}
}
#endif

#ifdef _PRMT_X_STD_QOS_

extern unsigned int getInstNum( char *name, char *objname );
unsigned int getQueueInstNum( char *name );
unsigned int getClassInstNum( char *name );
unsigned int findClassInstNum(void);
int getClassEntryByInstNum(unsigned int instnum, MIB_CE_IP_QUEUE_RULE_T *p, unsigned int *id);
int getQWaninterface( char *name, int instnum );
int getClassQueue(MIB_CE_IP_QUEUE_RULE_T* p);

#define MAX_CLASS_RULE 256
#define MAX_QUEUE_NUM  256


struct CWMP_OP tQueueEntityLeafOP = { getQueueEntity, setQueueEntity };
struct CWMP_PRMT tQueueEntityLeafInfo[] =
{
/*(name,		type,		flag,			op)*/
{"QueueKey",		eCWMP_tUINT,	CWMP_READ,		&tQueueEntityLeafOP},
{"QueueEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
{"QueueStatus",		eCWMP_tSTRING,	CWMP_READ,		&tQueueEntityLeafOP},
{"QueueInterface",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
{"QueueBufferLength",	eCWMP_tUINT,	CWMP_READ,		&tQueueEntityLeafOP},
//{"QueueWeight",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
{"QueuePrecedence",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
//{"REDThreshold",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
//{"REDPercentage",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
{"DropAlgorithm",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
{"SchedulerAlgorithm",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
//{"ShapingRate",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP},
//{"ShapingBurstSize",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tQueueEntityLeafOP}
};
enum eQueueEntityLeaf
{
	eQE_QueueKey,
	eQE_QueueEnable,
	eQE_QueueStatus,
	eQE_QueueInterface,
	eQE_QueueBufferLength,
	eQE_QueuePrecedence,
	eQE_DropAlgorithm,
	eQE_SchedulerAlgorithm
};
struct CWMP_LEAF tQueueEntityLeaf[] =
{
{ &tQueueEntityLeafInfo[eQE_QueueKey] },
{ &tQueueEntityLeafInfo[eQE_QueueEnable] },
{ &tQueueEntityLeafInfo[eQE_QueueStatus] },
{ &tQueueEntityLeafInfo[eQE_QueueInterface] },
{ &tQueueEntityLeafInfo[eQE_QueueBufferLength] },
{ &tQueueEntityLeafInfo[eQE_QueuePrecedence] },
{ &tQueueEntityLeafInfo[eQE_DropAlgorithm] },
{ &tQueueEntityLeafInfo[eQE_SchedulerAlgorithm] },
{ NULL }
};

struct CWMP_PRMT tQueueObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eQueueObject
{
	eQueueObject0
};
struct CWMP_LINKNODE tQueueObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tQueueObjectInfo[eQueueObject0],	tQueueEntityLeaf,	NULL,		NULL,			0},
};


#if 0
struct sCWMP_ENTITY tPolicerEntity[] =
{
/*(name,		type,		flag,			accesslist,	getvalue,	setvalue,	next_table,	sibling)*/
{"PolicerKey",		eCWMP_tUINT,	CWMP_READ,		NULL,		getPolicerEntity,NULL,		NULL,		NULL},
{"PolicerEnable",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	NULL,		getPolicerEntity,setPolicerEntity,NULL,		NULL},
{"PolicerStatus",	eCWMP_tSTRING,	CWMP_READ,		NULL,		getPolicerEntity,NULL,		NULL,		NULL},
{"CommittedRate",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	NULL,		getPolicerEntity,setPolicerEntity,NULL,		NULL},
{"CommittedBurstSize",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	NULL,		getPolicerEntity,setPolicerEntity,NULL,		NULL},
/*ExcessBurstSize*/
/*PeakRate*/
/*PeakBurstSize*/
{"MeterType",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	NULL,		getPolicerEntity,setPolicerEntity,NULL,		NULL},
{"PossibleMeterTypes",	eCWMP_tSTRING,	CWMP_READ,		NULL,		getPolicerEntity,NULL,		NULL,		NULL},
{"ConformingAction",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	NULL,		getPolicerEntity,setPolicerEntity,NULL,		NULL},
/*PartialConformingAction*/
{"NonConformingAction",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	NULL,		getPolicerEntity,setPolicerEntity,NULL,		NULL},
{"CountedPackets",	eCWMP_tUINT,	CWMP_READ,		NULL,		getPolicerEntity,NULL,		NULL,		NULL},
{"CountedBytes",	eCWMP_tUINT,	CWMP_READ,		NULL,		getPolicerEntity,NULL,		NULL,		NULL},
{"",			eCWMP_tNONE,	0,			NULL,		NULL,		NULL,		NULL,		NULL}
};

struct sCWMP_ENTITY tPolicer[] =
{
/*(name,			type,		flag,					accesslist,	getvalue,	setvalue,	next_table,	sibling)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL,		NULL,		NULL,		tPolicerEntity,	NULL}
//{"1",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL,		NULL,		NULL,		tPolicerEntity,	NULL}
};
#endif


struct CWMP_OP tClassEntityLeafOP = { getClassEntity, setClassEntity };
struct CWMP_PRMT tClassEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"ClassificationKey",		eCWMP_tUINT,	CWMP_READ,		&tClassEntityLeafOP},
{"ClassificationEnable",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"ClassificationStatus",	eCWMP_tSTRING,	CWMP_READ,		&tClassEntityLeafOP},
{"ClassificationOrder",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"ClassInterface",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"DestIP",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"DestMask",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"DestIPExclude",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"SourceIP",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"SourceMask",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"SourceIPExclude",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"Protocol",			eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"ProtocolExclude",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"DestPort",			eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"DestPortRangeMax",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"DestPortExclude",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"SourcePort",			eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"SourcePortRangeMax",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"SourcePortExclude",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"SourceMACAddress",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
/*SourceMACMask*/
//{"SourceMACExclude",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"DestMACAddress",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
/*DestMACMask*/
//{"DestMACExclude",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
/*Ethertype, EthertypeExclude*/
/*SSAP, SSAPExclude*/
/*DSAP, DSAPExclude*/
/*LLCControl, LLCControlExclude*/
/*SNAPOUI, SNAPOUIExclude*/
/*SourceVendorClassID, SourceVendorClassIDExclude*/
/*DestVendorClassID, DestVendorClassIDExclude*/
/*SourceClientID, SourceClientIDExclude*/
/*DestClientID, DestClientIDExclude*/
/*SourceUserClassID, SourceUserClassIDExclude*/
/*DestUserClassID, DestUserClassIDExclude*/
/*TCPACK, TCPACKExclude*/
/*IPLengthMin, IPLengthMax, IPLengthExclude*/
{"DSCPCheck",			eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"DSCPExclude",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"DSCPMark",			eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"EthernetPriorityCheck",	eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"EthernetPriorityExclude",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"EthernetPriorityMark",	eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"VLANIDCheck",			eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"VLANIDExclude",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"ForwardingPolicy",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
//{"ClassPolicer",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP},
{"ClassQueue",			eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tClassEntityLeafOP}
/*ClassApp*/
};
enum eClassEntityLeaf
{
	eQC_ClassificationKey,
	eQC_ClassificationEnable,
	eQC_ClassificationStatus,
	eQC_ClassificationOrder,
	eQC_ClassInterface,
	eQC_DestIP,
	eQC_DestMask,
	eQC_SourceIP,
	eQC_SourceMask,
	eQC_Protocol,
//	eQC_ProtocolExclude,
	eQC_DestPort,
	eQC_DestPortRangeMax,
	eQC_SourcePort,
	eQC_SourcePortRangeMax,
	eQC_DSCPCheck,
	eQC_DSCPMark,
	eQC_EthernetPriorityCheck,
	eQC_EthernetPriorityMark,
	eQC_ClassQueue
};
struct CWMP_LEAF tClassEntityLeaf[] =
{
{ &tClassEntityLeafInfo[eQC_ClassificationKey] },
{ &tClassEntityLeafInfo[eQC_ClassificationEnable] },
{ &tClassEntityLeafInfo[eQC_ClassificationStatus] },
{ &tClassEntityLeafInfo[eQC_ClassificationOrder] },
{ &tClassEntityLeafInfo[eQC_ClassInterface] },
{ &tClassEntityLeafInfo[eQC_DestIP] },
{ &tClassEntityLeafInfo[eQC_DestMask] },
{ &tClassEntityLeafInfo[eQC_SourceIP] },
{ &tClassEntityLeafInfo[eQC_SourceMask] },
{ &tClassEntityLeafInfo[eQC_Protocol] },
//{ &tClassEntityLeafInfo[eQC_ProtocolExclude] },
{ &tClassEntityLeafInfo[eQC_DestPort] },
{ &tClassEntityLeafInfo[eQC_DestPortRangeMax] },
{ &tClassEntityLeafInfo[eQC_SourcePort] },
{ &tClassEntityLeafInfo[eQC_SourcePortRangeMax] },
{ &tClassEntityLeafInfo[eQC_DSCPCheck] },
{ &tClassEntityLeafInfo[eQC_DSCPMark] },
{ &tClassEntityLeafInfo[eQC_EthernetPriorityCheck] },
{ &tClassEntityLeafInfo[eQC_EthernetPriorityMark] },
{ &tClassEntityLeafInfo[eQC_ClassQueue] },
{ NULL }
};

struct CWMP_PRMT tClassObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eClassObject
{
	eClassObject0
};
struct CWMP_LINKNODE tClassObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tClassObjectInfo[eClassObject0],	tClassEntityLeaf,	NULL,		NULL,			0},
};



struct CWMP_OP tQueueMntLeafOP = { getQueueMnt,	setQueueMnt };
struct CWMP_PRMT tQueueMntLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tQueueMntLeafOP},
{"MaxQueues",			eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"MaxClassificationEntries",	eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"ClassificationNumberOfEntries",eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"MaxAppEntries",		eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"AppNumberOfEntries",		eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"MaxFlowEntries",		eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"FlowNumberOfEntries",		eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"MaxPolicerEntries",		eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"PolicerNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"MaxQueueEntries",		eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
{"QueueNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tQueueMntLeafOP},
//{"DefaultForwardingPolicy",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tQueueMntLeafOP},
//{"DefaultPolicer",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tQueueMntLeafOP},
//{"DefaultQueue",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tQueueMntLeafOP},
//{"DefaultDSCPMark",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tQueueMntLeafOP},
{"DefaultEthernetPriorityMark",	eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tQueueMntLeafOP},
{"AvailableAppList",		eCWMP_tSTRING,	CWMP_READ,		&tQueueMntLeafOP}
};
enum eQueueMntLeaf
{
	eQ_Enable,
	eQ_MaxQueues,
	eQ_MaxClassificationEntries,
	eQ_ClassificationNumberOfEntries,
	eQ_MaxAppEntries,
	eQ_AppNumberOfEntries,
	eQ_MaxFlowEntries,
	eQ_FlowNumberOfEntries,
	eQ_MaxPolicerEntries,
	eQ_PolicerNumberOfEntries,
	eQ_MaxQueueEntries,
	eQ_QueueNumberOfEntries,
	eQ_DefaultEthernetPriorityMark,
	eQ_AvailableAppList
};
struct CWMP_LEAF tQueueMntLeaf[] =
{
{ &tQueueMntLeafInfo[eQ_Enable] },
{ &tQueueMntLeafInfo[eQ_MaxQueues] },
{ &tQueueMntLeafInfo[eQ_MaxClassificationEntries] },
{ &tQueueMntLeafInfo[eQ_ClassificationNumberOfEntries] },
{ &tQueueMntLeafInfo[eQ_MaxAppEntries] },
{ &tQueueMntLeafInfo[eQ_AppNumberOfEntries] },
{ &tQueueMntLeafInfo[eQ_MaxFlowEntries] },
{ &tQueueMntLeafInfo[eQ_FlowNumberOfEntries] },
{ &tQueueMntLeafInfo[eQ_MaxPolicerEntries] },
{ &tQueueMntLeafInfo[eQ_PolicerNumberOfEntries] },
{ &tQueueMntLeafInfo[eQ_MaxQueueEntries] },
{ &tQueueMntLeafInfo[eQ_QueueNumberOfEntries] },
{ &tQueueMntLeafInfo[eQ_DefaultEthernetPriorityMark] },
{ &tQueueMntLeafInfo[eQ_AvailableAppList] },
{ NULL }
};


struct CWMP_OP tQM_Class_OP = { NULL, objClass };
//struct CWMP_OP tQM_Policer_OP = { NULL, objPolicer };
struct CWMP_OP tQM_Queue_OP = { NULL, objQueue };
struct CWMP_PRMT tQueueMntObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Classification",		eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tQM_Class_OP},
//{"Policer",			eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tQM_Policer_OP},
{"Queue",			eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tQM_Queue_OP}
};
enum tQueueMntObject
{
	eQ_Classification,
	//eQ_Policer,
	eQ_Queue
};
struct CWMP_NODE tQueueMntObject[] =
{
/*info,  					leaf,			node)*/
{&tQueueMntObjectInfo[eQ_Classification],	NULL,			NULL},
{&tQueueMntObjectInfo[eQ_Queue],		NULL,			NULL},
{NULL,						NULL,			NULL}
};


int getQueueEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char 	buff[256]={0};
	unsigned char vChar=0;
	unsigned int  qinst=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	qinst = getQueueInstNum( name );
	if(qinst==0) return ERR_9005;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "QueueKey" )==0 )
	{
		*data=uintdup( qinst );
	}else if( strcmp( lastname, "QueueEnable" )==0 )
	{
		 *data=booldup( 1 );
	}else if( strcmp( lastname, "QueueStatus" )==0 )
	{
		 *data=strdup( "Enabled" );
	}else if( strcmp( lastname, "QueueInterface" )==0 )
	{
		char buf[256];
		int ret=0;
		ret=getQWaninterface(buf,qinst);
		if(ret==0)
			 *data=strdup( buf );
		else
			*data=strdup("");
	}else if( strcmp( lastname, "QueueBufferLength" )==0 )
	{
		 *data=uintdup( 8*2048 );
//	}else if( strcmp( lastname, "QueueWeight" )==0 )
//	{
//		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "QueuePrecedence" )==0 )
	{
		 *data=uintdup( ((qinst-1)%IPQOS_NUM_PRIOQ)+1 );
//	}else if( strcmp( lastname, "REDThreshold" )==0 )
//	{
//		 *data=uintdup( 0 );
//	}else if( strcmp( lastname, "REDPercentage" )==0 )
//	{
//		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "DropAlgorithm" )==0 )
	{
		 *data=strdup( "DT" );
	}else if( strcmp( lastname, "SchedulerAlgorithm" )==0 )
	{
		unsigned char policy=0;
		mib_get(MIB_QOS_QUEUE_POLICY, &policy);
		if(policy==0)
		 	*data=strdup( "SP" );
		else
			*data=strdup( "WRR" );
//	}else if( strcmp( lastname, "ShapingRate" )==0 )
//	{
//		 *data=intdup( -1 );
//	}else if( strcmp( lastname, "ShapingBurstSize" )==0 )
//	{
//		 *data=uintdup( 0 );
	}else{
		return ERR_9005;
	}
	
	return 0;
}


int setQueueEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned int  qinst=0;
	
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


	qinst = getQueueInstNum( name );
	if(qinst==0) return ERR_9005;

	if( strcmp( lastname, "QueueEnable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if(i==NULL) return ERR_9007;
		if(*i==0) return ERR_9001;
		return 0;
	}else if( strcmp( lastname, "QueueInterface" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		if( strlen(buf)!=0 ) return ERR_9001;
		return 0;
//	}else if( strcmp( lastname, "QueueWeight" )==0 )
//	{
	}else if( strcmp( lastname, "QueuePrecedence" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *i = &tmpuint;		
#else
		unsigned int *i = data;
#endif
		if(i==NULL) return ERR_9007;
		if(*i!=((qinst-1)%IPQOS_NUM_PRIOQ+1)) return ERR_9001;
		return 0;
//	}else if( strcmp( lastname, "REDThreshold" )==0 )
//	{
//	}else if( strcmp( lastname, "REDPercentage" )==0 )
//	{
	}else if( strcmp( lastname, "DropAlgorithm" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		if( strcmp( buf, "DT" )!=0 ) return ERR_9001;
		return 0;
	}else if( strcmp( lastname, "SchedulerAlgorithm" )==0 )
	{
		unsigned char policy=0;
		if(buf==NULL) return ERR_9007;
		if( strcmp( buf, "SP" )==0 ) policy=0;
		else if( strcmp( buf, "WRR" )==0 ) policy=1;
		else return ERR_9001;
		if(!mib_set(MIB_QOS_QUEUE_POLICY, &policy))
			return ERR_9002;
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
//	}else if( strcmp( lastname, "ShapingRate" )==0 )
//	{
//	}else if( strcmp( lastname, "ShapingBurstSize" )==0 )
//	{
	}else{
		return ERR_9005;
	}
	
	return 0;
}


int objQueue(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);

	switch( type )
	{
	case eCWMP_tINITOBJ:
	     {
	     	int num=0,MaxInstNum=0,i,j;
	     	struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;
		int total;
	     	
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		total=mib_chain_total(MIB_IP_QUEUE_LEN_TBL);
		
	     	num=IPQOS_NUM_PRIOQ;
		for( i=0; i<(num*total);i++ )
		{
			MaxInstNum = i+1;
			if( create_Object( c, tQueueObject, sizeof(tQueueObject), 1, MaxInstNum ) < 0 )
				return -1;
		}
		add_objectNum( name, MaxInstNum );
		return 0;
	     }
	case eCWMP_tADDOBJ:
	     {
		return ERR_9001;
	     }
	case eCWMP_tDELOBJ:
	     {
		return ERR_9001;
	     }
	case eCWMP_tUPDATEOBJ:	
		{
	     	int num=0,i,MaxInstNum=0;
	     	struct CWMP_LINKNODE *old_table;
		int total;

		total=mib_chain_total(MIB_IP_QUEUE_LEN_TBL);
	     	num=IPQOS_NUM_PRIOQ;

	     	old_table = (struct CWMP_LINKNODE *)entity->next;
	     	entity->next = NULL;
		for( i=0; i<(num*total);i++ )
		{
			MaxInstNum = i+1;
			add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tQueueObject, sizeof(tQueueObject), &MaxInstNum );
		}
		if( old_table )
	     		destroy_ParameterTable( (struct CWMP_NODE *)old_table );	     

	     	return 0;
	     }
	}
	return -1;
}

#if 0
int getPolicerEntity(char *name, struct sCWMP_ENTITY *entity, int *type, void **data)
{
	char	*lastname = entity->name;
	unsigned char vChar=0;
	struct in_addr ipAddr;
	char buff[256]={0};
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->type;
	*data = NULL;
	if( strcmp( lastname, "PolicerKey" )==0 )
	{
		*data=uintdup( 0 );
	}else if( strcmp( lastname, "PolicerEnable" )==0 )
	{
		 *data=booldup( 0 );
	}else if( strcmp( lastname, "PolicerStatus" )==0 )
	{
		 *data=strdup( "Disabled" );
	}else if( strcmp( lastname, "CommittedRate" )==0 )
	{
		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "CommittedBurstSize" )==0 )
	{
		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "MeterType" )==0 )
	{
		 *data=strdup( "SimpleTokenBucket" );
	}else if( strcmp( lastname, "PossibleMeterTypes" )==0 )
	{
		 *data=strdup( "SimpleTokenBucket,SingleRateThreeColor,TwoRateThreeColor" );
	}else if( strcmp( lastname, "ConformingAction" )==0 )
	{
		 *data=strdup( "Null" );
	}else if( strcmp( lastname, "NonConformingAction" )==0 )
	{
		 *data=strdup( "Drop" );
	}else if( strcmp( lastname, "CountedPackets" )==0 )
	{
		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "CountedBytes" )==0 )
	{
		 *data=uintdup( 0 );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setPolicerEntity(char *name, struct sCWMP_ENTITY *entity, int type, void *data)
{
	char	*lastname = entity->name;
	char	*buf=data;
	unsigned char vChar=0;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->type!=type ) return ERR_9006;

	if( strcmp( lastname, "PolicerEnable" )==0 )
	{
	}else if( strcmp( lastname, "CommittedRate" )==0 )
	{
	}else if( strcmp( lastname, "CommittedBurstSize" )==0 )
	{
	}else if( strcmp( lastname, "MeterType" )==0 )
	{
	}else if( strcmp( lastname, "ConformingAction" )==0 )
	{
	}else if( strcmp( lastname, "NonConformingAction" )==0 )
	{
	}else{
		return ERR_9005;
	}
	
	return 0;
}




int objPolicer(char *name, struct sCWMP_ENTITY *entity, int type, void *data)
{
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);

	switch( type )
	{
	case eCWMP_tINITOBJ:
	     {
		return 0;
	     }
	case eCWMP_tADDOBJ:
	     {
		return 0;
	     }
	case eCWMP_tDELOBJ:
	     {
		return 0;
	     }
	case eCWMP_tUPDATEOBJ:	
	     {
	     	return 0;
	     }
	}
	return -1;
}
#endif

int getClassEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	unsigned int  instnum=0,chainid=0;
	char buf[256]={0};
	MIB_CE_IP_QUEUE_RULE_T *p, qos_entity;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	printf("\nname=%s\n",name);
	instnum=getClassInstNum(name);
	if(instnum==0) return ERR_9005;
	p = &qos_entity;
	if(getClassEntryByInstNum( instnum, p, &chainid )<0) return ERR_9002;

	*type = entity->info->type;
	*data = NULL;

	printf("\ninstnum=%d\n",p->InstanceNum);

	if( strcmp( lastname, "ClassificationKey" )==0 )
	{
		*data = uintdup( p->InstanceNum );
	}else if( strcmp( lastname, "ClassificationEnable" )==0 )
	{
		 *data=booldup( p->state );
	}else if( strcmp( lastname, "ClassificationStatus" )==0 )
	{
		if( p->state==0 )
		 	*data=strdup( "Disabled" );
		else
			*data=strdup( "Enabled" );
	}else if( strcmp( lastname, "ClassificationOrder" )==0 )
	{
		 *data=uintdup( chainid+1 );
	}else if( strcmp( lastname, "ClassInterface" )==0 )
	{
		if( p->phyPort==0 )
			*data=strdup( "LAN" );
#if (defined(CONFIG_EXT_SWITCH)  && defined(IP_QOS_VPORT))
		else if( p->phyPort==1 )
			*data=strdup("InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1");
		else if( p->phyPort==2 )
			*data=strdup("InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.2");
		else if( p->phyPort==3 )
			*data=strdup("InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.3");
		else if( p->phyPort==4 )
			*data=strdup("InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.4");
#else
		else if( p->phyPort==1 )
			*data=strdup("InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1");
#endif
		else 
			return ERR_9002;
	}else if( strcmp( lastname, "DestIP" )==0 )
	{
		sprintf(buf, "%s", inet_ntoa(*((struct in_addr *)&p->dstip)));
		*data = strdup( buf );
	}else if( strcmp( lastname, "DestMask" )==0 )
	{
		unsigned long mask=0;
		int i,mbit;
		mbit = p->dmaskbits;
		for(i=0;i<32;i++)
		{
			mask = mask << 1;
			if(mbit)
			{ 
				mask = mask | 0x1;
				mbit--;
			}
		}
		mask = ntohl(mask);
		sprintf(buf, "%s", inet_ntoa(*((struct in_addr *)&mask)));
		*data = strdup( buf );
//	}else if( strcmp( lastname, "DestIPExclude" )==0 )
//	{
//		 *data=booldup( 0 );
	}else if( strcmp( lastname, "SourceIP" )==0 )
	{
		sprintf(buf, "%s", inet_ntoa(*((struct in_addr *)&p->srcip)));
		*data = strdup( buf );
	}else if( strcmp( lastname, "SourceMask" )==0 )
	{
		unsigned long mask=0;
		int i,mbit;
		mbit = p->smaskbits;
		for(i=0;i<32;i++)
		{
			mask = mask << 1;
			if(mbit)
			{ 
				mask = mask | 0x1;
				mbit--;
			}
		}
		mask = ntohl(mask);
		sprintf(buf, "%s", inet_ntoa(*((struct in_addr *)&mask)));
		*data = strdup( buf );
//	}else if( strcmp( lastname, "SourceIPExclude" )==0 )
//	{
//		 *data=booldup( 0 );
	}else if( strcmp( lastname, "Protocol" )==0 )
	{
		if( p->protoType==PROTO_NONE )
			*data=intdup( -1 );
		else if( p->protoType==2 )//TCP
			*data=intdup( 6 );
		else if( p->protoType==3 )//UDP
			*data=intdup( 17 );
		else if( p->protoType==1 )//ICMP
			*data=intdup( 1 );
		else
			return ERR_9002;			
//	}else if( strcmp( lastname, "ProtocolExclude" )==0 )
//	{
//		 *data=booldup( 0 );
	}else if( strcmp( lastname, "DestPort" )==0 )
	{
		if( p->dstPortStart==0 )
			*data=intdup( -1 );
		else
			*data=intdup( p->dstPortStart );
	}else if( strcmp( lastname, "DestPortRangeMax" )==0 )
	{
		if( p->dstPortStart < p->dstPortEnd
		   && p->dstPortStart >= 0
		   && p->dstPortEnd >= 0)
		 	*data=intdup( p->dstPortEnd-p->dstPortStart );
		else
			*data=intdup(-1);
		
//	}else if( strcmp( lastname, "DestPortExclude" )==0 )
//	{
//		 *data=booldup( 0 );
	}else if( strcmp( lastname, "SourcePort" )==0 )
	{
		if( p->srcPortStart==0 )
			*data=intdup( -1 );
		else
			*data=intdup( p->srcPortStart );
	}else if( strcmp( lastname, "SourcePortRangeMax" )==0 )
	{
		 if( p->srcPortStart < p->srcPortEnd
		   && p->srcPortStart >= 0
		   && p->srcPortEnd >= 0)
		 	*data=intdup( p->srcPortEnd-p->srcPortStart );
		else
			*data=intdup(-1);
//	}else if( strcmp( lastname, "SourcePortExclude" )==0 )
//	{
//		 *data=booldup( 0 );
//	}else if( strcmp( lastname, "SourceMACAddress" )==0 )
//	{
//		 *data=strdup( "" );
//	}else if( strcmp( lastname, "SourceMACExclude" )==0 )
//	{
//		 *data=booldup( 0 );
//	}else if( strcmp( lastname, "DestMACAddress" )==0 )
//	{
//		 *data=strdup( "" );
//	}else if( strcmp( lastname, "DestMACExclude" )==0 )
//	{
//		 *data=booldup( 0 );
//	}else if( strcmp( lastname, "DSCPCheck" )==0 )
//	{
//		 *data=intdup( -1 );
//	}else if( strcmp( lastname, "DSCPExclude" )==0 )
//	{
//		 *data=booldup( 0 );
//	}else if( strcmp( lastname, "DSCPMark" )==0 )
//	{
//		 *data=intdup( -1 );
	}else if( strcmp( lastname, "DSCPCheck" )==0 )
	{
		if( p->dscp==0 )
		 	*data=intdup( -1 );
		else
			*data=intdup( ((int)p->dscp)/4 );
	}else if( strcmp( lastname, "DSCPMark" )==0 )
	{
		if( p->m_dscp==0 )
		 	*data=intdup( -1 );
		else
			*data=intdup( ((int)p->m_dscp)/4 );
	}else if( strcmp( lastname, "EthernetPriorityCheck" )==0 )
	{
		if( p->vlan1p==0 )
		 	*data=intdup( -1 );
		else
			*data=intdup( (int)p->vlan1p-1 );
//	}else if( strcmp( lastname, "EthernetPriorityExclude" )==0 )
//	{
//		 *data=booldup( 0 );
	}else if( strcmp( lastname, "EthernetPriorityMark" )==0 )
	{
		if( p->m_1p==0 )
		 	*data=intdup( -1 );
		else
			*data=intdup( (int)p->m_1p-1 );
//	}else if( strcmp( lastname, "VLANIDCheck" )==0 )
//	{
//		 *data=intdup( -1 );
//	}else if( strcmp( lastname, "VLANIDExclude" )==0 )
//	{
//		 *data=booldup( 0 );
//	}else if( strcmp( lastname, "ForwardingPolicy" )==0 )
//	{
//		 *data=uintdup( 0 );
//	}else if( strcmp( lastname, "ClassPolicer" )==0 )
//	{
//		 *data=intdup( -1 );
	}else if( strcmp( lastname, "ClassQueue" )==0 )
	{
		int queue=getClassQueue(p);
		if(queue==-1)
			return ERR_9002;
		 *data=intdup( queue);
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setClassEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;
	unsigned int  instnum=0,chainid=0;
	MIB_CE_IP_QUEUE_RULE_T *p, qos_entity, old_qos_entity;
	struct in_addr in;
	char	*pzeroip="0.0.0.0";
	
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

	instnum=getClassInstNum(name);
	if(instnum==0) return ERR_9005;
	p = &qos_entity;
	if(getClassEntryByInstNum( instnum, p, &chainid )<0) return ERR_9002;
	memcpy( &old_qos_entity, &qos_entity, sizeof(MIB_CE_IP_QUEUE_RULE_T) );

	if( strcmp( lastname, "ClassificationEnable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if(i==NULL) return ERR_9007;
		p->state = (*i==0) ? 0:1;
		mib_chain_update( MIB_IP_QUEUE_RULE_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QUEUE_RULE_T) );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "ClassificationOrder" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *i = &tmpuint;		
#else
		unsigned int *i = data;
#endif
		if(i==NULL) return ERR_9007;
		if( *i==0 ) return ERR_9007;
		if( *i!=(chainid+1) ) return ERR_9001;
		return 0;
	}else if( strcmp( lastname, "ClassInterface" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strcmp( buf, "LAN" )==0 )
			p->phyPort=0xff;
#if (defined(CONFIG_EXT_SWITCH)  && defined(IP_QOS_VPORT))
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.4" )==0 )
			p->phyPort=0;
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.3" )==0 )
			p->phyPort=1;
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.2" )==0 )
			p->phyPort=2;
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1" )==0 )
			p->phyPort=3;
#else
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.1" )==0 )
			p->phyPort=0;
#endif
#ifdef WLAN_SUPPORT
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.1" )==0 )
			p->phyPort=5;
#if 0 //def WLAN_MBSSID
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.2" )==0 )
			p->phyPort=6;
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.3" )==0 )
			p->phyPort=7;
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.4" )==0 )
			p->phyPort=8;
		else if( strcmp( buf, "InternetGatewayDevice.LANDevice.1.WLANConfiguration.5" )==0 )
			p->phyPort=9;
#endif //WLAN_MBSSID
#endif //WLAN_SUPPORT
		else
			return ERR_9007;
		mib_chain_update( MIB_IP_QUEUE_RULE_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QUEUE_RULE_T) );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "DestIP" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) buf=pzeroip;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		memcpy( &(p->dstip), &in, sizeof(struct in_addr) );
		mib_chain_update( MIB_IP_QUEUE_RULE_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QUEUE_RULE_T) );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "DestMask" )==0 )
	{
		unsigned long mask;
		int intVal, i, mbit;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) buf=pzeroip;
		if( inet_aton( buf, (struct in_addr *)&mask )==0 ) //the ip address is error.
			return ERR_9007;
		mask = htonl(mask);
		mbit=0; intVal=0;
		for (i=0; i<32; i++)
		{
			if (mask&0x80000000)
			{
				if (intVal) return ERR_9007;
				mbit++;
			}
			else
				intVal=1;
			mask <<= 1;
		}
		p->dmaskbits = mbit;
		mib_chain_update( MIB_IP_QUEUE_RULE_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QUEUE_RULE_T) );
		return 0;
#else
		return 1;
#endif
//	}else if( strcmp( lastname, "DestIPExclude" )==0 )
//	{
	}else if( strcmp( lastname, "SourceIP" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) buf=pzeroip;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		memcpy( &(p->srcip), &in, sizeof(struct in_addr) );
		mib_chain_update( MIB_IP_QUEUE_RULE_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QUEUE_RULE_T) );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "SourceMask" )==0 )
	{
		unsigned long mask;
		int intVal, i, mbit;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) buf=pzeroip;
		if( inet_aton( buf, (struct in_addr *)&mask )==0 ) //the ip address is error.
			return ERR_9007;
		mask = htonl(mask);
		mbit=0; intVal=0;
		for (i=0; i<32; i++)
		{
			if (mask&0x80000000)
			{
				if (intVal) return ERR_9007;
				mbit++;
			}
			else
				intVal=1;
			mask <<= 1;
		}
		p->smaskbits = mbit;
		mib_chain_update( MIB_IP_QUEUE_RULE_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QUEUE_RULE_T) );
		return 0;
#else
		return 1;
#endif
//	}else if( strcmp( lastname, "SourceIPExclude" )==0 )
//	{
	}else if( strcmp( lastname, "Protocol" )==0 )
	{

#ifdef _PRMT_X_CT_COM_DATATYPE
		int *pro = &tmpint;		
#else
		int *pro = data;
#endif
		if(pro==NULL) return ERR_9007;
		switch(*pro)
		{
		case -1:
			p->protoType=PROTO_NONE;
			break;
		case 1:
			p->protoType=1; //ICMP
			break;
		case 6:
			p->protoType=2; //TCP
			break;
		case 17:
			p->protoType=3; //UDP
			break;
		default:
			return ERR_9001;
		}
		mib_chain_update( MIB_IP_QUEUE_RULE_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QUEUE_RULE_T) );
		return 0;
#else
		return 1;
#endif
//	}else if( strcmp( lastname, "ProtocolExclude" )==0 )
//	{
	}else if( strcmp( lastname, "DestPort" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *dport = &tmpint;		
#else
		int *dport = data;
#endif
		if( dport==NULL ) return ERR_9007;
		if( *dport==-1 )
			p->dstPortStart = p->dstPortEnd = 0;
		else if( (*dport>=1) && (*dport<=65535) )
			p->dstPortStart = p->dstPortEnd = *dport;
		else
			return ERR_9007;
		mib_chain_update( MIB_IP_QUEUE_RULE_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QUEUE_RULE_T) );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "DestPortRangeMax" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *dport = &tmpint;		
#else
		int *dport = data;
#endif	
		if( dport==NULL ) return ERR_9007;
		if( *dport==-1)
			p->dstPortEnd=p->dstPortStart;
		else if( (*dport>=1) && (*dport<=65535) )
			p->dstPortEnd=p->dstPortStart+*dport;
		else
			return ERR_9007;
		mib_chain_update( MIB_IP_QUEUE_RULE_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QUEUE_RULE_T) );
		return 0;
#else
		return 1;
#endif	
//	}else if( strcmp( lastname, "DestPortExclude" )==0 )
//	{
	}else if( strcmp( lastname, "SourcePort" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *sport = &tmpint;		
#else
		int *sport = data;
#endif
		if( sport==NULL ) return ERR_9007;
		if( *sport==-1 )
			p->srcPortStart = p->srcPortEnd = 0;
		else if( (*sport>=1) && (*sport<=65535) )
			p->srcPortStart = p->srcPortEnd = *sport;
		else
			return ERR_9007;
		mib_chain_update( MIB_IP_QUEUE_RULE_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QUEUE_RULE_T) );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "SourcePortRangeMax" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *dport = &tmpint;		
#else
		int *dport = data;
#endif	
		if( dport==NULL ) return ERR_9007;
		if( *dport==-1)
			p->srcPortEnd=p->srcPortStart;
		else if( (*dport>=1) && (*dport<=65535) )
			p->srcPortEnd=p->srcPortStart+*dport;
		else
			return ERR_9007;
		mib_chain_update( MIB_IP_QUEUE_RULE_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QUEUE_RULE_T) );
		return 0;
#else
		return 1;
#endif	
//	}else if( strcmp( lastname, "SourcePortExclude" )==0 )
//	{
//	}else if( strcmp( lastname, "SourceMACAddress" )==0 )
//	{
//	}else if( strcmp( lastname, "SourceMACExclude" )==0 )
//	{
//	}else if( strcmp( lastname, "DestMACAddress" )==0 )
//	{
//	}else if( strcmp( lastname, "DestMACExclude" )==0 )
//	{
	}else if( strcmp( lastname, "DSCPCheck" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpint;		
#else
		int *i = data;
#endif
		if(i==NULL) return ERR_9007;
		if( *i<0  || *i>63 ) return ERR_9007;
		p->dscp = (unsigned char)(*i)*4;
		mib_chain_update( MIB_IP_QUEUE_RULE_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QUEUE_RULE_T) );
		return 0;
#else
		return 1;
#endif
//	}else if( strcmp( lastname, "DSCPExclude" )==0 )
//	{
	}else if( strcmp( lastname, "DSCPMark" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpint;		
#else
		int *i = data;
#endif
		if(i==NULL) return ERR_9007;
		if( *i<0  || *i>63 ) return ERR_9007;
		p->m_dscp = (unsigned char)(*i)*4;
		mib_chain_update( MIB_IP_QUEUE_RULE_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QUEUE_RULE_T) );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "EthernetPriorityCheck" )==0 )
	{

#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpint;		
#else
		int *i = data;
#endif
		if(i==NULL) return ERR_9007;
		if( *i<-1 || *i>7 ) return ERR_9007;
		p->vlan1p = (unsigned char)(*i+1);
		mib_chain_update( MIB_IP_QUEUE_RULE_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QUEUE_RULE_T) );
		return 0;
#else
		return 1;
#endif
//	}else if( strcmp( lastname, "EthernetPriorityExclude" )==0 )
//	{
	}else if( strcmp( lastname, "EthernetPriorityMark" )==0 )
	{

#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpint;		
#else
		int *i = data;
#endif
		if(i==NULL) return ERR_9007;
		if( *i<-1 || *i>7 ) return ERR_9007;
		p->m_1p = (unsigned char)(*i+1);
		mib_chain_update( MIB_IP_QUEUE_RULE_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QUEUE_RULE_T) );
		return 0;
#else
		return 1;
#endif
//	}else if( strcmp( lastname, "VLANIDCheck" )==0 )
//	{
//	}else if( strcmp( lastname, "VLANIDExclude" )==0 )
//	{
//	}else if( strcmp( lastname, "ForwardingPolicy" )==0 )
//	{
//	}else if( strcmp( lastname, "ClassPolicer" )==0 )
//	{
	}else if( strcmp( lastname, "ClassQueue" )==0 )
	{

		int idx,num;
		MIB_CE_IP_QUEUE_LEN_T entry;
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpint;		
#else
		int *i = data;
#endif
		num=mib_chain_total(MIB_IP_QUEUE_LEN_TBL);
		if(i==NULL) return ERR_9007;
		if( *i<1 || *i>(IPQOS_NUM_PRIOQ*num) ) return ERR_9007;
		p->prio = (unsigned char)((*i-1)%IPQOS_NUM_PRIOQ+1);
		idx=(*i)/IPQOS_NUM_PRIOQ;
		mib_chain_get(MIB_IP_QUEUE_LEN_TBL,idx,(void*)&entry);
		p->ifIndex=entry.ifIndex;
		
		mib_chain_update( MIB_IP_QUEUE_RULE_TBL, (unsigned char*)p, chainid );
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_IPQoSRule, CWMP_RESTART, chainid, &old_qos_entity, sizeof(MIB_CE_IP_QUEUE_RULE_T) );
		return 0;
#else
		return 1;
#endif
	}else{
		return ERR_9005;
	}
	
	return 0;
}


int objClass(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);

	switch( type )
	{
	case eCWMP_tINITOBJ:
	     {
		unsigned int num=0,MaxInstNum=0,i;
		struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;
		MIB_CE_IP_QUEUE_RULE_T *p, qos_entity;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		MaxInstNum = findClassInstNum();
		num = mib_chain_total( MIB_IP_QUEUE_RULE_TBL );
		for( i=0; i<num;i++ )
		{
			p = &qos_entity;
			if( !mib_chain_get( MIB_IP_QUEUE_RULE_TBL, i, (void*)p ) )
				continue;

			if(p->modeTr69!=MODEINTERNET)
				continue;
			
			if( p->InstanceNum==0 ) //maybe createn by web or cli
			{
				MaxInstNum++;
				p->InstanceNum = MaxInstNum;
				mib_chain_update( MIB_IP_QUEUE_RULE_TBL, (unsigned char*)p, i );
			}
			if( create_Object( c, tClassObject, sizeof(tClassObject), 1, p->InstanceNum ) < 0 )
				return -1;
			//c = & (*c)->sibling;
		}
		add_objectNum( name, MaxInstNum );
		return 0;	     		     	
	     }
	case eCWMP_tADDOBJ:
	     {
	     	int ret;
	     	unsigned int num=0;
		int ifindex=0;
	     	
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
		num = mib_chain_total( MIB_IP_QUEUE_RULE_TBL );
		if(num>=MAX_CLASS_RULE) return ERR_9004;

		ifindex=getClassIfindex();
		if(ifindex==-1)
			return -1;
	
		ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tClassObject, sizeof(tClassObject), data );
		if( ret >= 0 )
		{
			MIB_CE_IP_QUEUE_RULE_T qos_entry;
			memset( &qos_entry, 0, sizeof( MIB_CE_IP_QUEUE_RULE_T ) );
			qos_entry.InstanceNum = *(unsigned int*)data;
			qos_entry.phyPort=0;
			qos_entry.prio=4;
			qos_entry.ifIndex=ifindex;
			qos_entry.state=0;
			strcpy(qos_entry.RuleName,"rule_name");
			qos_entry.index=getClassIndex();
			mib_chain_add( MIB_IP_QUEUE_RULE_TBL, (unsigned char*)&qos_entry );
		}
		return ret;
	     }
	case eCWMP_tDELOBJ:
	     {
		unsigned int i,num;
		int ret=0;
		MIB_CE_IP_QUEUE_RULE_T *p,qos_entity;
	     	
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
	
		num = mib_chain_total( MIB_IP_QUEUE_RULE_TBL );
		for( i=0; i<num;i++ )
		{
			p = &qos_entity;
			if( !mib_chain_get( MIB_IP_QUEUE_RULE_TBL, i, (void*)p ))
				continue;
			if( p->InstanceNum == *(unsigned int*)data )
				break;
		}	     	
		if(i==num) return ERR_9005;
		

	     	mib_chain_delete( MIB_IP_QUEUE_RULE_TBL, i );
		ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
#ifdef _CWMP_APPLY_
		apply_IPQoSRule( CWMP_START, -1, NULL );
#else
		if( ret == 0 ) return 1;
#endif
		return ret;
	     }
	case eCWMP_tUPDATEOBJ:	
	     {
	     	int num=0,i;
	     	struct CWMP_LINKNODE *old_table;
	     	
	     	num = mib_chain_total( MIB_IP_QUEUE_RULE_TBL );
	     	old_table = (struct CWMP_LINKNODE *)entity->next;
	     	entity->next = NULL;
	     	for( i=0; i<num;i++ )
	     	{
	     		struct CWMP_LINKNODE *remove_entity=NULL;
			MIB_CE_IP_QUEUE_RULE_T *p,qos_entity;

			p = &qos_entity;
			if( !mib_chain_get( MIB_IP_QUEUE_RULE_TBL, i, (void*)p ) )
				continue;

			if(p->modeTr69!=MODEINTERNET)
				continue;
			
			remove_entity = remove_SiblingEntity( &old_table, p->InstanceNum );
			if( remove_entity!=NULL )
			{
				add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
			}else{ 
				unsigned int MaxInstNum=p->InstanceNum;
					
				add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tClassObject, sizeof(tClassObject), &MaxInstNum );
				if(MaxInstNum!=p->InstanceNum)
				{
					p->InstanceNum = MaxInstNum;
					mib_chain_update( MIB_IP_QUEUE_RULE_TBL, (unsigned char*)p, i );
				}
			}	
	     	}
	     	
	     	if( old_table )
	     		destroy_ParameterTable( (struct CWMP_NODE *)old_table );	     	

	     	return 0;
	     }
	}
	return -1;
}


int getQueueMnt(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char 	buff[256]={0};
	unsigned char vChar=0;
	unsigned int num=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		*data = booldup( 1 );
	}else if( strcmp( lastname, "MaxQueues" )==0 )
	{
		*data=uintdup( MAX_QUEUE_NUM );
	}else if( strcmp( lastname, "MaxClassificationEntries" )==0 )
	{
		*data=uintdup( MAX_CLASS_RULE );
	}else if( strcmp( lastname, "ClassificationNumberOfEntries" )==0 )
	{
		num = mib_chain_total(MIB_IP_QUEUE_RULE_TBL);
		*data=uintdup( num );
	}else if( strcmp( lastname, "MaxAppEntries" )==0 )
	{
		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "AppNumberOfEntries" )==0 )
	{
		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "MaxFlowEntries" )==0 )
	{
		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "FlowNumberOfEntries" )==0 )
	{
		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "MaxPolicerEntries" )==0 )
	{
		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "PolicerNumberOfEntries" )==0 )
	{
		 *data=uintdup( 0 );
	}else if( strcmp( lastname, "MaxQueueEntries" )==0 )
	{
		 *data=uintdup( MAX_QUEUE_NUM );
	}else if( strcmp( lastname, "QueueNumberOfEntries" )==0 )
	{
		int total=mib_chain_total(MIB_IP_QUEUE_LEN_TBL);
		 *data=uintdup( IPQOS_NUM_PRIOQ*total );
//	}else if( strcmp( lastname, "DefaultForwardingPolicy" )==0 )
//	{
//		 *data=uintdup( 0 );
//	}else if( strcmp( lastname, "DefaultPolicer" )==0 )
//	{
//		 *data=intdup( -1 );
//	}else if( strcmp( lastname, "DefaultQueue" )==0 )
//	{
//		 *data=uintdup( 0 );
//	}else if( strcmp( lastname, "DefaultDSCPMark" )==0 )
//	{
//		 *data=intdup( -1 );
	}else if( strcmp( lastname, "DefaultEthernetPriorityMark" )==0 )
	{
		 *data=intdup( -1 );
	}else if( strcmp( lastname, "AvailableAppList" )==0 )
	{
		 *data=strdup( "" );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setQueueMnt(char *name, struct CWMP_LEAF *entity, int type, void *data)
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
		if(i==NULL) return ERR_9007;
/*
		mib_get(MIB_MPMODE, (void *)&vChar);
		if(*i==0)
			vChar &= 0xfd;
		else
			vChar |= 0x02;
		mib_set(MIB_MPMODE, (void *)&vChar);
#ifdef _CWMP_APPLY_
		//apply_IPQoS has higher priority than apply_IPQoSRule
		apply_add( CWMP_PRI_H, apply_IPQoS, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
*/
		return 0;
//	}else if( strcmp( lastname, "DefaultForwardingPolicy" )==0 )
//	{
//	}else if( strcmp( lastname, "DefaultPolicer" )==0 )
//	{
//	}else if( strcmp( lastname, "DefaultQueue" )==0 )
//	{
//	}else if( strcmp( lastname, "DefaultDSCPMark" )==0 )
//	{
	}else if( strcmp( lastname, "DefaultEthernetPriorityMark" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpint;		
#else
		int *i = data;
#endif
		if(i==NULL) return ERR_9007;
		if(*i!=-1) return ERR_9001;
		return 0;
	}else{
		return ERR_9005;
	}
	
	return 0;
}


/**************************************************************************************/
/* utility functions*/
/**************************************************************************************/

unsigned int getQueueInstNum( char *name )
{
	return getInstNum( name, "Queue" );
}

unsigned int getClassInstNum( char *name )
{
	return getInstNum( name, "Classification" );
}

unsigned int findClassInstNum(void)
{
	unsigned int ret=0, i,num;
	MIB_CE_IP_QUEUE_RULE_T *p,qos_entity;
	
	num = mib_chain_total( MIB_IP_QUEUE_RULE_TBL );
	for( i=0; i<num;i++ )
	{
		p = &qos_entity;
		if( !mib_chain_get( MIB_IP_QUEUE_RULE_TBL, i, (void*)p ))
			continue;
#ifdef _PRMT_X_CT_COM_QOS_
		if(p->modeTr69!=MODEINTERNET)
			continue;
#endif
		if( p->InstanceNum > ret )
			ret = p->InstanceNum;
	}
	
	return ret;
}

int getClassEntryByInstNum(unsigned int instnum, MIB_CE_IP_QUEUE_RULE_T *p, unsigned int *id)
{
	int ret=-1;
	unsigned int i,num;
	
	if( (instnum==0) || (p==NULL) || (id==NULL) ) return ret;
	
	num = mib_chain_total( MIB_IP_QUEUE_RULE_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_IP_QUEUE_RULE_TBL, i, (void*)p ))
			continue;
#ifdef _PRMT_X_CT_COM_QOS_
		if(p->modeTr69!=MODEINTERNET)
			continue;
#endif
		if( p->InstanceNum == instnum )
		{
			*id = i;
			ret=0;
			break;
		}
	}
	
	return ret;
}

int getQWaninterface( char *name, int instnum )
{
	int total,i;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;
	MIB_CE_IP_QUEUE_LEN_T tmpEntry;

	if( name==NULL ) return -1;
	name[0]=0;

	i = (instnum-1)/IPQOS_NUM_PRIOQ;

	if (!mib_chain_get(MIB_IP_QUEUE_LEN_TBL, i, &tmpEntry))
		return -1;
	
	total = mib_chain_total(MIB_ATM_VC_TBL);
	for( i=0; i<total; i++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get(MIB_ATM_VC_TBL, i, (void*)pEntry ) )
			continue;
		if(pEntry->ifIndex==tmpEntry.ifIndex)
		{
			char strfmt[]="InternetGatewayDevice.WANDevice.1.WANConnectionDevice.%d.%s.%d"; //wt-121v8-3.33, no trailing dot
			char ipstr[]="WANIPConnection";
			char pppstr[]="WANPPPConnection";
			char *pconn=NULL;
			unsigned int instnum=0;

			if( (pEntry->cmode==ADSL_PPPoE) || 
#ifdef PPPOE_PASSTHROUGH
			    ((pEntry->cmode==ADSL_BR1483)&&(pEntry->cmode==BRIDGE_PPPOE)) ||
#endif
			    (pEntry->cmode==ADSL_PPPoA) )
			{
				pconn = pppstr;
				instnum = pEntry->ConPPPInstNum;
			}else{
				pconn = ipstr;
				instnum = pEntry->ConIPInstNum;
			}

			if( pEntry->connDisable==0 )
			{
				sprintf( name, strfmt, pEntry->ConDevInstNum , pconn, instnum );
				break;
			}else
				return -1;

		}
	}
	//name = InternetGatewayDevice.WANDevice.1.WANConnection.2.WANPPPConnection.1.
	return 0;
}

int getClassIfindex()
{
	MIB_CE_IP_QUEUE_LEN_T tmpEntry;
	int total=0,i;

	total=mib_chain_total(MIB_IP_QUEUE_LEN_TBL);

	if(total<=0)
		return -1;

	for(i=0;i<total;i++){
		if( !mib_chain_get(MIB_IP_QUEUE_LEN_TBL, i, (void*)&tmpEntry ) )
			continue;
		return tmpEntry.ifIndex;
	}

	return -1;
}

int getClassQueue(MIB_CE_IP_QUEUE_RULE_T* p)
{
	int total,i;
	MIB_CE_IP_QUEUE_LEN_T tmpEntry;
	
	total=mib_chain_total(MIB_IP_QUEUE_LEN_TBL);

	for(i=0;i<total;i++){
		if( !mib_chain_get(MIB_IP_QUEUE_LEN_TBL, i, (void*)&tmpEntry ) )
			continue;
		if(tmpEntry.ifIndex==p->ifIndex)
			break;
	}

	if(i>=total)
		return -1;

	return(i*IPQOS_NUM_PRIOQ+p->prio);

}
#endif
#ifdef _PRMT_X_STD_QOS_
int getClassIndex()
{
    	unsigned char map[MAX_CLASS_RULE+1]={0};
   	int entryNum = 0, index=0,i=0;
	MIB_CE_IP_QUEUE_RULE_T entry;
	
	entryNum = mib_chain_total(MIB_IP_QUEUE_RULE_TBL);

	if( entryNum>=MAX_CLASS_RULE)
	{
		printf("You cannot add one new rule when queue is full.");
		return -1;
	}
	//get index for this rule
	for(i=0;i<entryNum; i++) {
		if(!mib_chain_get(MIB_IP_QUEUE_RULE_TBL, i, &entry))
			continue;
		map[entry.index] = 1;
	}
	for(i=1; i<=MAX_CLASS_RULE; i++)
	{
		if(!map[i])
		{
			index = i;
			break;
		}
		else if(i == MAX_CLASS_RULE)
		{
			printf("You cannot add queue rule any more, pls delte some!\n");
			return -1;
		}
	}

	return index;

}
#endif