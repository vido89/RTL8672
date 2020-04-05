/*
 *	Prmt_capabilities.c
 *
 *	TR104 capabilities parameters
 *
 */

#include "prmt_limit.h"
#include "prmt_capabilities.h"
#include "str_utility.h"

static int getCapabilitiesCodecsEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
static int objCapabilitiesCodecs(char *name, struct CWMP_LEAF *entity, int type, void *data);
static int getCapabilitiesSipEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
static int getCapabilitiesEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);


/*---- Capability-Codec leaves -----*/
struct CWMP_OP tCapabilitiesCodecsLeafOP = { getCapabilitiesCodecsEntity,	NULL };
struct CWMP_PRMT tCapabilitiesCodecsLeafInfo[] =
{
/*(name,					type,				flag,			op)*/
{"EntryId",			eCWMP_tUINT,		CWMP_READ,		&tCapabilitiesCodecsLeafOP},
{"Codec",				eCWMP_tSTRING,		CWMP_READ,	&tCapabilitiesCodecsLeafOP},
{"BitRate",				eCWMP_tUINT,		CWMP_READ,	&tCapabilitiesCodecsLeafOP},
{"PacketizationPeriod",		eCWMP_tSTRING,		CWMP_READ,	&tCapabilitiesCodecsLeafOP},
{"SilenceSuppression",		eCWMP_tBOOLEAN,		CWMP_READ,	&tCapabilitiesCodecsLeafOP},
};

enum eCapabilitiesCodecsLeaf
{
	eCPBEntryId,
	eCPBCodec,
	eCPBBitRate,	
	eCPBPacketizationPeriod,		
	eCPBSilenceSuppression,		
};

struct CWMP_LEAF tCapabilitiesCodecsEntityLeaf[] =
{
{ &tCapabilitiesCodecsLeafInfo[eCPBEntryId] },
{ &tCapabilitiesCodecsLeafInfo[eCPBCodec] },
{ &tCapabilitiesCodecsLeafInfo[eCPBBitRate] },
{ &tCapabilitiesCodecsLeafInfo[eCPBPacketizationPeriod] },
{ &tCapabilitiesCodecsLeafInfo[eCPBSilenceSuppression] },
{ NULL }
};

/* ---- Capability-Codec objects -----*/
struct CWMP_PRMT tCodecsOjbectInfo[] =
{
/*(name,	type,		flag,					op)*/
{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eCodecsOjbect
{
	eCodec0
};
struct CWMP_LINKNODE tCodecsObject[] =
{
/*info,  			leaf,			next,		sibling,		instnum)*/
{&tCodecsOjbectInfo[eCodec0],	tCapabilitiesCodecsEntityLeaf,		NULL,		NULL,			0},
};

/* ---- Capability-SIP leaves ----- */
struct CWMP_OP tCapabilitiesSIPLeafOP = { getCapabilitiesSipEntity,	NULL };
struct CWMP_PRMT tCapabilitiesSIPLeafInfo[] =
{
/*(name,				type,				flag,			op)*/
{"Role",				eCWMP_tSTRING,		CWMP_READ,	&tCapabilitiesSIPLeafOP},
{"Extensions",			eCWMP_tSTRING,		CWMP_READ,	&tCapabilitiesSIPLeafOP},
{"Transports",			eCWMP_tSTRING,		CWMP_READ,	&tCapabilitiesSIPLeafOP},
{"URISchemes",		eCWMP_tSTRING,		CWMP_READ,	&tCapabilitiesSIPLeafOP},
};
enum eCapabilitiesSIPLeaf
{
	eCPBRole,
	eCPBExtensions,
	eCPBTransports,	
	eCPBURISchemes,		
};

struct CWMP_LEAF tCapabilitiesSIPEntityLeaf[] =
{
{ &tCapabilitiesSIPLeafInfo[eCPBRole] },
{ &tCapabilitiesSIPLeafInfo[eCPBExtensions] },
{ &tCapabilitiesSIPLeafInfo[eCPBTransports] },
{ &tCapabilitiesSIPLeafInfo[eCPBURISchemes] },
{ NULL },
};

/* ---- Capability leaves -----*/
struct CWMP_OP tCapabilitiesLeafOP = { getCapabilitiesEntity,	NULL };
struct CWMP_PRMT tCapabilitiesLeafInfo[] =
{
/*(name,					type,				flag,			op)*/
{"MaxProfileCount",		eCWMP_tUINT,		CWMP_READ,	&tCapabilitiesLeafOP},
{"MaxLineCount",			eCWMP_tUINT,		CWMP_READ,	&tCapabilitiesLeafOP},
{"MaxSessionCount",		eCWMP_tUINT,		CWMP_READ,	&tCapabilitiesLeafOP},
{"SignallingProtocols",		eCWMP_tSTRING,		CWMP_READ,	&tCapabilitiesLeafOP},
{"FaxT38",				eCWMP_tBOOLEAN,	CWMP_READ,	&tCapabilitiesLeafOP},
{"FaxPassThrough",		eCWMP_tBOOLEAN	,	CWMP_READ,	&tCapabilitiesLeafOP},
{"ModemPassThrough",		eCWMP_tBOOLEAN	,	CWMP_READ,	&tCapabilitiesLeafOP},
{"ToneGeneration",			eCWMP_tBOOLEAN	,	CWMP_READ,	&tCapabilitiesLeafOP},
{"RingGeneration",			eCWMP_tBOOLEAN	,	CWMP_READ,	&tCapabilitiesLeafOP},
{"VoicePortTests",			eCWMP_tBOOLEAN	,	CWMP_READ,	&tCapabilitiesLeafOP},

};
enum eCapabilitiesLeaf
{
	eCPBMaxProfileCount,
	eCPBMaxLineCount,
	eCPBMaxSessionCount,
	eCPBSignallingProtocols,
	eCPBFaxT38,
	eCPBFaxPassThrough,
	eCPBModemPassThrough,
	eCPBToneGeneration,
	eCPBRingGeneration,
	eCPBVoicePortTests,
};
struct CWMP_LEAF tCapabilitiesEntityLeaf[] =
{
{ &tCapabilitiesLeafInfo[eCPBMaxProfileCount] },
{ &tCapabilitiesLeafInfo[eCPBMaxLineCount] },
{ &tCapabilitiesLeafInfo[eCPBMaxSessionCount] },
{ &tCapabilitiesLeafInfo[eCPBSignallingProtocols] },
{ &tCapabilitiesLeafInfo[eCPBFaxT38] },
{ &tCapabilitiesLeafInfo[eCPBFaxPassThrough] },
{ &tCapabilitiesLeafInfo[eCPBModemPassThrough] },
{ &tCapabilitiesLeafInfo[eCPBToneGeneration] },
{ &tCapabilitiesLeafInfo[eCPBRingGeneration] },
{ &tCapabilitiesLeafInfo[eCPBVoicePortTests] },
{ NULL }
};
/* --- Capabilities objects ----*/
struct CWMP_OP tCPBCodecs_OP = { NULL, objCapabilitiesCodecs };

struct CWMP_PRMT tCapabilitiesEntityObjectInfo[] = 
{
/*(name,			type,		flag,			)*/
{"SIP",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"Codecs",		eCWMP_tOBJECT,	CWMP_READ,	&tCPBCodecs_OP},
};

enum eCapabilitiesObjects
{
	eCPBSIP,
	eCPBCodecs
};


struct CWMP_NODE tCapabilitiesEntityObject[] = 
{
{&tCapabilitiesEntityObjectInfo[eCPBSIP],	tCapabilitiesSIPEntityLeaf,			NULL},
{&tCapabilitiesEntityObjectInfo[eCPBCodecs],	NULL,			NULL},
{NULL,							NULL,	NULL}
};

/* It must be listed as same as that of voip_flash.h */
const lstCodecs_t lstCodecs[] = {
	{ "G.711MuLaw",	80 * 100 * 8,		"10,20,30",	1 },
	{ "G.711ALaw",	80 * 100 * 8,		"10,20,30",	1 },
#ifdef  CONFIG_RTK_VOIP_G729AB
	{ "G.729",		10 * 100 * 8,		"10,20,30",	1 },
#endif /*CONFIG_RTK_VOIP_G729AB*/
#ifdef CONFIG_RTK_VOIP_G7231
	{ "G.723.1",	5300,				"30,60",	1 },
	{ "G.723.1",	6300,				"30,60",	1 },
#endif /*CONFIG_RTK_VOIP_G7231*/
#ifdef  CONFIG_RTK_VOIP_G726
	{ "G.726",		16 * 1000,			"10,20,30",	1 },
	{ "G.726",		24 * 1000,			"10,20,30",	1 },
	{ "G.726",		32 * 1000,			"10,20,30",	1 },
	{ "G.726",		40 * 1000,			"10,20,30",	1 },
#endif /*CONFIG_RTK_VOIP_G726*/
#ifdef CONFIG_RTK_VOIP_GSMFR
	{ "GSM-FR",		33 * 100 * 8 / 2,	"20,40",	0 },
#endif /*CONFIG_RTK_VOIP_GSMFR*/
};

#define NUM_OF_LIST_CODEC		( sizeof( lstCodecs ) / sizeof( lstCodecs[ 0 ] ) )

CT_ASSERT( NUM_OF_LIST_CODEC == MAX_CODEC_LIST );

static int getCapabilitiesCodecsEntity(char *name, struct CWMP_LEAF *leaf, int *type, void **data)
{
	struct CWMP_PRMT	*entity = leaf->info;
	const char *pszLastname = entity ->name;
	unsigned int nObjectNum, nObjectIdx;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	if( !GetOneBasedInstanceNumber_Capabilities_Codecs( name, &nObjectNum ) )
		return ERR_9007;
	
	nObjectIdx = nObjectNum - 1;	/* to zero-based */
	
	if( nObjectIdx >= NUM_OF_LIST_CODEC )
		return ERR_9007;
			
	*type = entity ->type;

	if( strcmp( pszLastname, "EntryId" ) == 0 ) {
		*data = uintdup( nObjectNum );
	} else if( strcmp( pszLastname, "Codec" ) == 0 ) {
		*data = strdup( lstCodecs[ nObjectIdx ].pszCodec );
	} else if( strcmp( pszLastname, "BitRate" ) == 0 ) {
		*data = uintdup( lstCodecs[ nObjectIdx ].nBitRate );
	} else if( strcmp( pszLastname, "PacketizationPeriod" ) == 0 ) {
		*data = strdup( lstCodecs[ nObjectIdx ].pszPacketizationPeriod );
	} else if( strcmp( pszLastname, "SilenceSuppression" ) == 0 ) {
		*data = booldup( lstCodecs[ nObjectIdx ].bSilenceSupression );
	} else {
		*data = NULL;
		return ERR_9005;
	}	

	return 0;
}

static int objCapabilitiesCodecs(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;

	switch( type ) {
	case eCWMP_tINITOBJ:
	{

		struct CWMP_LINKNODE**c = (struct CWMP_LINKNODE **)data;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;


		if( create_Object( c, tCodecsObject, sizeof(tCodecsObject), NUM_OF_LIST_CODEC, 1 ) < 0 )
			return -1;
			
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

static int getCapabilitiesSipEntity(char *name, struct CWMP_LEAF *leaf, int *type, void **data)
{
	struct CWMP_PRMT	*entity = leaf->info;
	const char *pszLastname = entity ->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity ->type;

	if( strcmp( pszLastname, "Role" ) == 0 ) {
		*data = strdup( SIP_ROLE );
	} else if( strcmp( pszLastname, "Extensions" ) == 0 ) {
		*data = strdup( "" );
	} else if( strcmp( pszLastname, "Transports" ) == 0 ) {
		*data = strdup( SIP_TRANSPORTS );
	} else if( strcmp( pszLastname, "URISchemes" ) == 0 ) {
		*data = strdup(SIP_URI_SCHEMES);
	} else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}

static int getCapabilitiesEntity(char *name, struct CWMP_LEAF *leaf, int *type, void **data)
{
	struct CWMP_PRMT	*entity = leaf->info;
	const char *pszLastname = entity ->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity ->type;

	if( strcmp( pszLastname, "MaxProfileCount" ) == 0 ) {
		*data = uintdup( MAX_PROFILE_COUNT );
	} else if( strcmp( pszLastname, "MaxSessionCount" ) == 0 ) {
		*data = uintdup( MAX_SESSION_COUNT );
	} else if( strcmp( pszLastname, "SignallingProtocols" ) == 0 ) {
		*data = strdup( SIGNALING_PROTOCOLS );	/* "SIP" */
	} else if( strcmp( pszLastname, "FaxT38" ) == 0 ) {
#ifdef CONFIG_RTK_VOIP_T38
		*data = booldup(TRUE);
	} else if( strcmp( pszLastname, "FaxPassThrough" ) == 0 ) {
		*data = booldup(TRUE);
	} else if( strcmp( pszLastname, "ModemPassThrough" ) == 0 ) {
		*data = booldup(TRUE);
#else
		*data = booldup(FALSE);
	} else if( strcmp( pszLastname, "FaxPassThrough" ) == 0 ) {
		*data = booldup(FALSE);
	} else if( strcmp( pszLastname, "ModemPassThrough" ) == 0 ) {
		*data = booldup(FALSE);
#endif
	} else if( strcmp( pszLastname, "ToneGeneration" ) == 0 ) {
		*data = booldup( SUPPORT_TONE_GENERATION_OBJECT );	/* Off -> No this object */
	} else if( strcmp( pszLastname, "RingGeneration" ) == 0 ) {
		*data = booldup( SUPPORT_RING_GENERATION_OBJECT );	/* Off -> No this object */
	} else if( strcmp( pszLastname, "VoicePortTests" ) == 0 ) {
		*data = booldup( SUPPORT_VOICE_LINE_TESTS_OBJECT );	/* Off -> No this object */
	} else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}

