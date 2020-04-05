#include "prmt_limit.h"
#include "prmt_voice_profile.h"
#include "prmt_voice_profile_line.h"
#include "mib_def.h"
#include "mib_tr104.h"
#include "str_utility.h"
#include "str_mib.h"

static int getVoiceProfileSipEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
static int setVoiceProfileSipEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

static int getServiceProviderInfoEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
static int setServiceProviderInfoEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

static int getVoiceProfileEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
static int setVoiceProfileEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

extern int apply_VoIP( int action_type, int id, void *olddata );

/*--- VoiceProfile leaf ---*/
struct CWMP_OP tVoiceProfileLeafOP = { getVoiceProfileEntity,	setVoiceProfileEntity };
struct CWMP_PRMT tVoiceProfileLeafInfo[] =
{
/*(name,					type,				flag,							op)*/
{"Enable",				eCWMP_tSTRING,		CWMP_READ | CWMP_WRITE,	&tVoiceProfileLeafOP},
{"Reset",					eCWMP_tBOOLEAN,		CWMP_READ | CWMP_WRITE,	&tVoiceProfileLeafOP},
{"NumberOfLines",			eCWMP_tUINT,		CWMP_READ,					&tVoiceProfileLeafOP},
{"Name",					eCWMP_tSTRING,		CWMP_READ | CWMP_WRITE,	&tVoiceProfileLeafOP},
{"SignallingProtocol",		eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileLeafOP},
{"MaxSessions",			eCWMP_tUINT,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileLeafOP},
{"DTMFMethod",			eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileLeafOP},
};

enum eVoiceProfileLeaf
{
	eVPEnable,
	eVPReset,
	eVPNumberOfLines,
	eVPName,
	eVPSignallingProtocol,
	eVPMaxSessions,
	eVPDTMFMethod,
};

struct CWMP_LEAF tVoiceProfileEntityLeaf[] =
{
{ &tVoiceProfileLeafInfo[eVPEnable] },
{ &tVoiceProfileLeafInfo[eVPReset] },
{ &tVoiceProfileLeafInfo[eVPNumberOfLines] },
{ &tVoiceProfileLeafInfo[eVPName] },
{ &tVoiceProfileLeafInfo[eVPSignallingProtocol] },
{ &tVoiceProfileLeafInfo[eVPMaxSessions] },
{ &tVoiceProfileLeafInfo[eVPDTMFMethod] },
{ NULL }
};
/* ---- VoiceProfile-SIP leaf   -----*/
struct CWMP_OP tVPSIPLeafOP = { getVoiceProfileSipEntity,	setVoiceProfileSipEntity };
struct CWMP_PRMT tVPSIPLeafInfo[] =
{
/*(name,						type,				flag,							op)*/
{"ProxyServer",				eCWMP_tSTRING,		CWMP_READ | CWMP_WRITE,	&tVPSIPLeafOP},
{"ProxyServerPort",			eCWMP_tUINT,		CWMP_READ | CWMP_WRITE,	&tVPSIPLeafOP},
{"ProxyServerTransport",		eCWMP_tSTRING,		CWMP_READ | CWMP_WRITE,	&tVPSIPLeafOP},
{"RegistrarServer",			eCWMP_tSTRING,		CWMP_READ | CWMP_WRITE,	&tVPSIPLeafOP},
{"RegistrarServerPort",		eCWMP_tUINT,		CWMP_READ | CWMP_WRITE,	&tVPSIPLeafOP},
{"RegistrarServerTransport",	eCWMP_tSTRING,		CWMP_READ | CWMP_WRITE,	&tVPSIPLeafOP},
{"UserAgentDomain",			eCWMP_tSTRING,		CWMP_READ | CWMP_WRITE,	&tVPSIPLeafOP},
{"UserAgentPort",				eCWMP_tUINT,		CWMP_READ | CWMP_WRITE,	&tVPSIPLeafOP},
{"UserAgentTransport",			eCWMP_tSTRING,		CWMP_READ | CWMP_WRITE,	&tVPSIPLeafOP},
{"OutboundProxy",				eCWMP_tSTRING,		CWMP_READ | CWMP_WRITE,	&tVPSIPLeafOP},
{"OutboundProxyPort",			eCWMP_tUINT,		CWMP_READ | CWMP_WRITE,	&tVPSIPLeafOP},
{"Organization",				eCWMP_tSTRING,		CWMP_READ | CWMP_WRITE,	&tVPSIPLeafOP},
{"RegistrationPeriod",			eCWMP_tUINT,		CWMP_READ | CWMP_WRITE,	&tVPSIPLeafOP},
};

enum eVPSIPLeaf
{
	eVPSIPProxyServer,
	eVPSIPProxyServerPort,
	eVPSIPProxyServerTransport,
	eVPSIPRegistrationServer,
	eVPSIPRegistrationServerPort,
	eVPSIPRegistrationServerTransport,
	eVPSIPUserAgentDomain,
	eVPSIPUserAgentPort,
	eVPSIPUserAgentTransport,
	eVPSIPOutboundProxy,
	eVPSIPOutboundProxyPort,
	eVPSIPOrganization,
	eVPSIPRegistrationPeriod,
};

struct CWMP_LEAF tVPSIPLeaf[] =
{
{ &tVPSIPLeafInfo[eVPSIPProxyServer] },
{ &tVPSIPLeafInfo[eVPSIPProxyServerPort] },
{ &tVPSIPLeafInfo[eVPSIPProxyServerTransport] },
{ &tVPSIPLeafInfo[eVPSIPRegistrationServer] },
{ &tVPSIPLeafInfo[eVPSIPRegistrationServerPort] },
{ &tVPSIPLeafInfo[eVPSIPRegistrationServerTransport] },
{ &tVPSIPLeafInfo[eVPSIPUserAgentDomain] },
{ &tVPSIPLeafInfo[eVPSIPUserAgentPort] },
{ &tVPSIPLeafInfo[eVPSIPUserAgentTransport] },
{ &tVPSIPLeafInfo[eVPSIPOutboundProxy] },
{ &tVPSIPLeafInfo[eVPSIPOutboundProxyPort] },
{ &tVPSIPLeafInfo[eVPSIPOrganization] },
{ &tVPSIPLeafInfo[eVPSIPRegistrationPeriod] },
{ NULL }
};


/* ---- VoiceProfile-ServiceProviderInfo leaf   ----- */
struct CWMP_OP tVPServiceProviderLeafOP = { getServiceProviderInfoEntity,	setServiceProviderInfoEntity };
struct CWMP_PRMT tVPServiceProviderLeafInfo[] =
{
/*(name,				type,				flag,			op)*/
{"Name",				eCWMP_tSTRING,		CWMP_READ | CWMP_WRITE,	NULL},
};

enum eVPServiceProviderLeaf
{
	eVPSPName
};

struct CWMP_LEAF tVPServiceProviderInfoLeaf[] =
{
{ &tVPServiceProviderLeafInfo[eVPSPName] },
{ NULL }
};
/* ---- VoiceProfile-ServiceProviderInfo object   ----- */
struct CWMP_OP tVoiceProfileLineLeafOP = { NULL, objVoiceProfileLine};

struct CWMP_PRMT tVoiceProfileEntityObjectInfo[] = 
{
/*(name,				type,			flag,			OP)*/
{"ServiceProviderInfo",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"SIP",				eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"Line",				eCWMP_tOBJECT,	CWMP_READ,	&tVoiceProfileLineLeafOP},
//{"Line",				eCWMP_tOBJECT,	CWMP_READ,	NULL},
};
enum eVoiceProfileEntityObjects
{
	eVPServiceProviderInfo,
	eVPSIP,
	eVPLine
};


struct CWMP_NODE tVoiceProfileEntityObject[] = 
{
{&tVoiceProfileEntityObjectInfo[eVPServiceProviderInfo],	&tVPServiceProviderInfoLeaf,		NULL},
{&tVoiceProfileEntityObjectInfo[eVPSIP],				&tVPSIPLeaf,						NULL},
{&tVoiceProfileEntityObjectInfo[eVPLine],				NULL,							NULL},
{NULL,												NULL,							NULL},
};


/* ---- VoiceProfile object  objects ----- */
struct CWMP_PRMT tVoiceProfileOjbectInfo[] =
{
/*(name,	type,		flag,					op)*/
{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eVoiceProfileOjbect
{
	eVoiceProfile0
};
struct CWMP_LINKNODE tVoiceProfileObject[] =
{
/*info,  			leaf,			next,		sibling,		instnum)*/
{&tVoiceProfileOjbectInfo[eVoiceProfile0],	tVoiceProfileEntityLeaf,		tVoiceProfileEntityObject,		NULL,			0},
};

static int getVoiceProfileSipEntity(char *name, struct CWMP_LEAF *leaf, int *type, void **data)
{
	struct CWMP_PRMT	*entity = leaf->info;

	/* VoiceServer.{i}.VoiceProfile.{i}.SIP */
	unsigned int nVoiceProfileInstNum;
	union {
		char ProxyServer[ 256 ];
		unsigned int ProxyServerPort;
		transport_t ProxyServerTransport;
		char RegistrarServer[ 256 ];
		unsigned int RegistrarServerPort;
		transport_t RegistrarServerTransport;
		char UserAgentDomain[ 256 ];
		unsigned int UserAgentPort;
		transport_t UserAgentTransport;
		char OutboundProxy[ 256 ];
		unsigned int OutboundProxyPort;
		char Organization[ 256 ];
		unsigned int RegistrationPeriod;
	} s;
	const char *pszLastname = entity ->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	*type = entity ->type;	
		
	if( strcmp( pszLastname, "ProxyServer" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__PROXY_SERVER, nVoiceProfileInstNum, &s.ProxyServer );
		*data = strdup( s.ProxyServer );
	} else if( strcmp( pszLastname, "ProxyServerPort" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__PROXY_SERVER_PORT, nVoiceProfileInstNum, &s.ProxyServerPort );
		*data = uintdup( s.ProxyServerPort );
	} else if( strcmp( pszLastname, "ProxyServerTransport" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__PROXY_SERVER_TRANSPORT, nVoiceProfileInstNum, &s.ProxyServerTransport );
		*data = strdup_Transport( s.ProxyServerTransport );
	} else if( strcmp( pszLastname, "RegistrarServer" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__REGISTRATION_SERVER, nVoiceProfileInstNum, &s.RegistrarServer );
		*data = strdup( s.RegistrarServer );
	} else if( strcmp( pszLastname, "RegistrarServerPort" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__REGISTRATION_SERVER_PORT, nVoiceProfileInstNum, &s.RegistrarServerPort );
		*data = uintdup( s.RegistrarServerPort );
	} else if( strcmp( pszLastname, "RegistrarServerTransport" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__REGISTRATION_SERVER_TRANSPORT, nVoiceProfileInstNum, &s.RegistrarServerTransport );
		*data = strdup_Transport( s.RegistrarServerTransport );
	} else if( strcmp( pszLastname, "UserAgentDomain" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__USER_AGENT_DOMAIN, nVoiceProfileInstNum, &s.UserAgentDomain );
		*data = strdup( s.UserAgentDomain );
	} else if( strcmp( pszLastname, "UserAgentPort" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__USER_AGENT_PORT, nVoiceProfileInstNum, &s.UserAgentPort );
		*data = uintdup( s.UserAgentPort );
	} else if( strcmp( pszLastname, "UserAgentTransport" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__USER_AGENT_TRANSPORT, nVoiceProfileInstNum, &s.UserAgentTransport );
		*data = strdup_Transport( s.UserAgentTransport );
	} else if( strcmp( pszLastname, "OutboundProxy" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY, nVoiceProfileInstNum, &s.OutboundProxy );
		*data = strdup( s.OutboundProxy );
	} else if( strcmp( pszLastname, "OutboundProxyPort" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY_PORT, nVoiceProfileInstNum, &s.OutboundProxyPort );
		*data = uintdup( s.OutboundProxyPort );
	} else if( strcmp( pszLastname, "Organization" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__ORGANIZATION, nVoiceProfileInstNum, &s.Organization );
		*data = strdup( s.Organization );
	} else if( strcmp( pszLastname, "RegistrationPeriod" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__REGISTRATION_PERIOD, nVoiceProfileInstNum, &s.RegistrationPeriod );
		*data = uintdup( s.RegistrationPeriod );
	} else {
		*data = NULL;
		return ERR_9005;
	}
	
	return 0;
}

static int setVoiceProfileSipEntity(char *name, struct CWMP_LEAF *leaf, int type, void *data)
{
	struct CWMP_PRMT	*entity = leaf->info;
#ifdef _CWMP_APPLY_
	int	ret_succ = 0;
#else
	int	ret_succ = 1;	
#endif
	/* VoiceServer.{i}.VoiceProfile.{i}.SIP */
	unsigned int nVoiceProfileInstNum;
	union {
		char ProxyServer[ 256 ];
		unsigned int ProxyServerPort;
		transport_t ProxyServerTransport;
		char RegistrarServer[ 256 ];
		unsigned int RegistrarServerPort;
		transport_t RegistrarServerTransport;
		char UserAgentDomain[ 256 ];
		unsigned int UserAgentPort;
		transport_t UserAgentTransport;
		char OutboundProxy[ 256 ];
		unsigned int OutboundProxyPort;
		char Organization[ 256 ];
		unsigned int RegistrationPeriod;
	} s;
	const char *pszLastname = entity ->name;

	


	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
		
	if( entity ->type != type )
		return ERR_9006;
	
	if( strcmp( pszLastname, "ProxyServer" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__PROXY_SERVER, nVoiceProfileInstNum, data );
		return ret_succ;
	} else if( strcmp( pszLastname, "ProxyServerPort" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__PROXY_SERVER_PORT, nVoiceProfileInstNum, data );
		return ret_succ;
	} else if( strcmp( pszLastname, "ProxyServerTransport" ) == 0 ) {
		if( str2id_SignalingProtocol( data, &s.ProxyServerTransport ) ){
			mib_set_type1( MIB_VOICE_PROFILE__SIP__PROXY_SERVER_TRANSPORT, nVoiceProfileInstNum, &s.ProxyServerTransport );
			return ret_succ;
		}else{
			return ERR_9007;
		}
	} else if( strcmp( pszLastname, "RegistrarServer" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__REGISTRATION_SERVER, nVoiceProfileInstNum, data );
		return ret_succ;
	} else if( strcmp( pszLastname, "RegistrarServerPort" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__REGISTRATION_SERVER_PORT, nVoiceProfileInstNum, data );
		return ret_succ;
	} else if( strcmp( pszLastname, "RegistrarServerTransport" ) == 0 ) {
		if( str2id_SignalingProtocol( data, &s.RegistrarServerTransport ) ){
			mib_set_type1( MIB_VOICE_PROFILE__SIP__REGISTRATION_SERVER_TRANSPORT, nVoiceProfileInstNum, &s.RegistrarServerTransport );
			return ret_succ;
		}else{
			return ERR_9007;
		}
	} else if( strcmp( pszLastname, "UserAgentDomain" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__USER_AGENT_DOMAIN, nVoiceProfileInstNum, data );
		return ret_succ;
	} else if( strcmp( pszLastname, "UserAgentPort" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__USER_AGENT_PORT, nVoiceProfileInstNum, data );
		return ret_succ;
	} else if( strcmp( pszLastname, "UserAgentTransport" ) == 0 ) {
		if( str2id_SignalingProtocol( data, &s.UserAgentTransport ) ){
			mib_set_type1( MIB_VOICE_PROFILE__SIP__USER_AGENT_TRANSPORT, nVoiceProfileInstNum, &s.UserAgentTransport );
			return ret_succ;
		}else{
			return ERR_9007;
		}
	} else if( strcmp( pszLastname, "OutboundProxy" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY, nVoiceProfileInstNum, data );
		return ret_succ;
	} else if( strcmp( pszLastname, "OutboundProxyPort" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY_PORT, nVoiceProfileInstNum, data );
		return ret_succ;
	} else if( strcmp( pszLastname, "Organization" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__ORGANIZATION, nVoiceProfileInstNum, data );
		return ret_succ;
	} else if( strcmp( pszLastname, "RegistrationPeriod" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__REGISTRATION_PERIOD, nVoiceProfileInstNum, data );
		return ret_succ;
	} else {
		return ERR_9005;
	}	
	
	return 0;
}

static int getServiceProviderInfoEntity(char *name, struct CWMP_LEAF *leaf, int *type, void **data)
{
	struct CWMP_PRMT	*entity = leaf->info;
	/* VoiceServer.{i}.VoiceProfile.{i}.ServiceProviderInfo */
	unsigned int nVoiceProfileInstNum;
	char szName[ 256 ];
	const char *pszLastname = entity ->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	*type = entity ->type;	
		
	if( strcmp( pszLastname, "Name" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SERVICE_PROVIDE_INFO__NAME, nVoiceProfileInstNum, &szName );
		*data = strdup( szName );
	} else {
		*data = NULL;
		return ERR_9005;
	}
	
	return 0;
}

static int setServiceProviderInfoEntity(char *name, struct CWMP_LEAF *leaf, int type, void *data)
{
	struct CWMP_PRMT	*entity = leaf->info;
	/* VoiceServer.{i}.VoiceProfile.{i}.ServiceProviderInfo */
	unsigned int nVoiceProfileInstNum;
	const char *pszLastname = entity ->name;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
		
	if( entity ->type != type )
		return ERR_9006;
	
	if( strcmp( pszLastname, "Name" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SERVICE_PROVIDE_INFO__NAME, nVoiceProfileInstNum, data );
	} else {
		return ERR_9005;
	}	
	
	return 0;
}

static int getVoiceProfileEntity(char *name, struct CWMP_LEAF *leaf, int *type, void **data)
{
	struct CWMP_PRMT	*entity = leaf->info;
	/* VoiceServer.{i}.VoiceProfile.{i}. */
	unsigned int nVoiceProfileInstNum;
	union {
		enable_t Enable;
		int Reset;
		unsigned int NumberOfLines;
		char Name[ 40 ];
		signaling_protocol_t SignallingProtocol;
		unsigned int MaxSessions;
		DTMF_method_t DTMFMethod;
	} s;

	const char *pszLastname = entity ->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;

	*type = entity ->type;

	if( strcmp( pszLastname, "Enable" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__ENABLE, nVoiceProfileInstNum, &s.Enable );
		*data = strdup_Enable( s.Enable );
	} else if( strcmp( pszLastname, "Reset" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__RESET, nVoiceProfileInstNum, &s.Reset );
		*data = booldup( s.Reset );
	} else if( strcmp( pszLastname, "NumberOfLines" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &s.NumberOfLines );
		*data = uintdup( s.NumberOfLines );
	} else if( strcmp( pszLastname, "Name" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__NAME, nVoiceProfileInstNum, &s.Name );
		*data = strdup( s.Name );
	} else if( strcmp( pszLastname, "SignallingProtocol" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIGNALING_PROTOCOL, nVoiceProfileInstNum, &s.SignallingProtocol );
		*data = strdup_SignalingProtocol( s.SignallingProtocol );
	} else if( strcmp( pszLastname, "MaxSessions" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__MAX_SESSIONS, nVoiceProfileInstNum, &s.MaxSessions );
		*data = uintdup( s.MaxSessions );
	} else if( strcmp( pszLastname, "DTMFMethod" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__DTMF_METHOD, nVoiceProfileInstNum, &s.DTMFMethod );
		*data = strdup_DTMFMethod( s.DTMFMethod );	
	} else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}

static int setVoiceProfileEntity(char *name, struct CWMP_LEAF *leaf, int type, void *data)
{
	struct CWMP_PRMT	*entity = leaf->info;
	int ret_succ;
	/* VoiceServer.{i}.VoiceProfile.{i}. */
	unsigned int nVoiceProfileInstNum;
	union {
		enable_t Enable;
		int Reset;
		unsigned int NumberOfLines;
		char Name[ 40 ];
		signaling_protocol_t SignallingProtocol;
		unsigned int MaxSessions;
		DTMF_method_t DTMFMethod;
	} s;

	const char *pszLastname = entity ->name;

	
#ifdef _CWMP_APPLY_
	ret_succ = 0;
#else
	ret_succ = 1;	
#endif
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
		
	if( entity ->type != type )
		return ERR_9006;

	if( strcmp( pszLastname, "Enable" ) == 0 ) {
		if( str2id_Enable( data, &s.Enable ) ){
			mib_set_type1( MIB_VOICE_PROFILE__ENABLE, nVoiceProfileInstNum, &s.Enable );
			return ret_succ;
		}else{
			return ERR_9007;
		}
	} else if( strcmp( pszLastname, "Reset" ) == 0 ) {
		s.Reset = *( ( int * )data );
		mib_set_type1( MIB_VOICE_PROFILE__RESET, nVoiceProfileInstNum, &s.Reset );
		return ret_succ;
	} else if( strcmp( pszLastname, "NumberOfLines" ) == 0 ) {
		return ERR_9008;	/* Read only */
	} else if( strcmp( pszLastname, "Name" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__NAME, nVoiceProfileInstNum, data );
		return ret_succ;
	} else if( strcmp( pszLastname, "SignallingProtocol" ) == 0 ) {
		if( str2id_SignalingProtocol( data, &s.SignallingProtocol ) ){
			mib_set_type1( MIB_VOICE_PROFILE__SIGNALING_PROTOCOL, nVoiceProfileInstNum, &s.SignallingProtocol );
			return ret_succ;
		}else{
			return ERR_9007;
		}
	} else if( strcmp( pszLastname, "MaxSessions" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__MAX_SESSIONS, nVoiceProfileInstNum, data );
		return ret_succ;
	} else if( strcmp( pszLastname, "DTMFMethod" ) == 0 ) {
		if( str2id_DTMFMethod( data, &s.DTMFMethod ) ){
			mib_set_type1( MIB_VOICE_PROFILE__DTMF_METHOD, nVoiceProfileInstNum, &s.DTMFMethod );
			return ret_succ;
		}else{
			return ERR_9007;
		}
	} else {
		return ERR_9005;
	}
	
	return 0;
	//return 1;
}

int objVoiceProfile(char *name, struct CWMP_LEAF *e, int type, void *data)
{

	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	
	switch( type ) {
	case eCWMP_tINITOBJ:
	{
		
		int num=0,i;
		struct CWMP_LINKNODE**c = (struct CWMP_LINKNODE **)data;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		num = 1;
		for( i=0; i<num;i++ )
		{
			if( create_Object( c, tVoiceProfileObject, sizeof(tVoiceProfileObject), MAX_PROFILE_COUNT, 1 ) < 0 )
				return -1;
		}
		add_objectNum( name, 1 );
	
		return 0;

	}
		break;
		
	case eCWMP_tADDOBJ:
	case eCWMP_tDELOBJ:
	case eCWMP_tUPDATEOBJ:	
		break;
	}
	return -1;
}

