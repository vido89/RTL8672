#include "prmt_limit.h"
#include "prmt_voice_profile_line.h"
#include "prmt_capabilities.h"	/* for lstCodecs */
#include "mib_def.h"
#include "mib_tr104.h"
#include "str_mib.h"
#include "str_utility.h"
#include "voip_manager.h"

#include "cwmpevt.h"
cwmpEvtMsg pEvtMsg; //for compilation error, must check!
static int getVoiceProfileLineCodecListEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
static int setVoiceProfileLineCodecListEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

static int objVoiceProfileLineCodecList(char *name, struct CWMP_LEAF *entity, int type, void *data);

static int getVoiceProfileLineStatusEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
static int setVoiceProfileLineStatusEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

static int getVoiceProfileLineCodecEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);

static int getVoiceProfileLineSipEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
static int setVoiceProfileLineSipEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

static int getVoiceProfileLineEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
static int setVoiceProfileLineEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

//--- VoiceProfileLine-Codec leaf ---//
struct CWMP_OP tVPLineCodecLeafOP = { getVoiceProfileLineCodecEntity,	NULL };
struct CWMP_PRMT tVPLineCodecLeafInfo[] =
{ /*(name,					type,				flag,							op)*/
{"TransmitCodec",				eCWMP_tSTRING,		CWMP_READ ,		&tVPLineCodecLeafOP},
{"ReceiveCodec",				eCWMP_tSTRING,		CWMP_READ ,		&tVPLineCodecLeafOP},
{"TransmitBitRate",				eCWMP_tUINT,		CWMP_READ ,		&tVPLineCodecLeafOP},
{"ReceiveBitRate",				eCWMP_tUINT,		CWMP_READ ,		&tVPLineCodecLeafOP},
{"TransmitSilenceSuppression",	eCWMP_tBOOLEAN,	CWMP_READ,		&tVPLineCodecLeafOP},
{"ReceiveSilenceSuppression",	eCWMP_tBOOLEAN,	CWMP_READ,		&tVPLineCodecLeafOP},
{"TransmitPacketizationPeriod",	eCWMP_tUINT,		CWMP_READ,		&tVPLineCodecLeafOP},

};

enum eVoiceProfileLineCodecLeaf
{
	eVPLineCodecTransmitCodec,
	eVPLineCodecReceiveCodec,
	eVPLineCodecTransmitBitRate,
	eVPLineCodecReceiveBitRate,
	eVPLineCodecTransmitSilenceSuppression,
	eVPLineCodecReceiveSilenceSuppression,
	eVPLineCodecTransmitPacketizationPeriod
};

struct CWMP_LEAF tVPLineCodecEntityLeaf[] =
{
{ &tVPLineCodecLeafInfo[eVPLineCodecTransmitCodec] },
{ &tVPLineCodecLeafInfo[eVPLineCodecReceiveCodec] },
{ &tVPLineCodecLeafInfo[eVPLineCodecTransmitBitRate] },
{ &tVPLineCodecLeafInfo[eVPLineCodecReceiveBitRate] },
{ &tVPLineCodecLeafInfo[eVPLineCodecTransmitSilenceSuppression] },
{ &tVPLineCodecLeafInfo[eVPLineCodecReceiveSilenceSuppression] },
{ &tVPLineCodecLeafInfo[eVPLineCodecTransmitPacketizationPeriod] },
{ NULL }
};



//--- VoiceProfileLine-SIP leaf ---//
struct CWMP_OP tVPLineSIPLeafOP = { getVoiceProfileLineSipEntity,	setVoiceProfileLineSipEntity };
struct CWMP_PRMT tVPLineSIPLeafInfo[] =
{
/*(name,					type,				flag,							op)*/
{"AuthUserName",			eCWMP_tSTRING,		CWMP_READ | CWMP_WRITE,	&tVPLineSIPLeafOP},
{"AuthPassword",			eCWMP_tSTRING,		CWMP_READ | CWMP_WRITE,	&tVPLineSIPLeafOP},
{"URI",					eCWMP_tSTRING,		CWMP_READ | CWMP_WRITE,	&tVPLineSIPLeafOP},
};

enum eVoiceProfileLineSIPLeaf
{
	eVPLineSIPAuthUserName,
	eVPLineSIPAuthPassword,		
	eVPLineSIPURI,	
};

struct CWMP_LEAF tVPLineSIPEntityLeaf[] =
{
{ &tVPLineSIPLeafInfo[eVPLineSIPAuthUserName] },
{ &tVPLineSIPLeafInfo[eVPLineSIPAuthPassword] },
{ &tVPLineSIPLeafInfo[eVPLineSIPURI] },
{ NULL }
};

//--- VoiceProfileLine-Stats leaf ---//
struct CWMP_OP tVPLineStatsLeafOP = { getVoiceProfileLineStatusEntity,	setVoiceProfileLineStatusEntity };
struct CWMP_PRMT tVPLineStatsLeafInfo[] =
{
/*(name,						type,				flag,							op)*/
{"PacketsSent",				eCWMP_tUINT,		CWMP_READ,					&tVPLineStatsLeafOP},
{"PacketsReceived",			eCWMP_tUINT,		CWMP_READ,					&tVPLineStatsLeafOP},
{"BytesSent",					eCWMP_tUINT,		CWMP_READ,					&tVPLineStatsLeafOP},
{"BytesReceived",				eCWMP_tUINT,		CWMP_READ,					&tVPLineStatsLeafOP},
{"PacketsLost",				eCWMP_tUINT,		CWMP_READ,					&tVPLineStatsLeafOP},
{"IncomingCallsReceived",		eCWMP_tUINT,		CWMP_READ,					&tVPLineStatsLeafOP},
{"IncomingCallsAnswered",		eCWMP_tUINT,		CWMP_READ,					&tVPLineStatsLeafOP},
{"IncomingCallsConnected",		eCWMP_tUINT,		CWMP_READ,					&tVPLineStatsLeafOP},
{"IncomingCallsFailed",			eCWMP_tUINT,		CWMP_READ,					&tVPLineStatsLeafOP},
{"OutgoingCallsAttempted",		eCWMP_tUINT,		CWMP_READ,					&tVPLineStatsLeafOP},
{"OutgoingCallsAnswered",		eCWMP_tUINT,		CWMP_READ,					&tVPLineStatsLeafOP},
{"OutgoingCallsConnected",		eCWMP_tUINT,		CWMP_READ,					&tVPLineStatsLeafOP},
{"OutgoingCallsFailed",			eCWMP_tUINT,		CWMP_READ,					&tVPLineStatsLeafOP},
};

enum eVoiceProfileLineStatsLeaf
{
	eVPLineStatsPacketsSent,
	eVPLineStatsPacketsReceived,
	eVPLineStatsBytesSent,
	eVPLineStatsBytesReceived,
	eVPLineStatsPacketsLost,
	eVPLineStatsIncomingCallsReceived,
	eVPLineStatsIncomingCallsAnswered,
	eVPLineStatsIncomingCallsConnected,
	eVPLineStatsIncomingCallsFailed,
	eVPLineStatsOutgoingCallsAttempted,
	eVPLineStatsOutgoingCallsAnswered,
	eVPLineStatsOutgoingCallsConnected,
	eVPLineStatsOutgoingCallsFailed	
};

struct CWMP_LEAF tVPLineStatsEntityLeaf[] =
{
{ &tVPLineStatsLeafInfo[eVPLineStatsPacketsSent] },
{ &tVPLineStatsLeafInfo[eVPLineStatsPacketsReceived] },
{ &tVPLineStatsLeafInfo[eVPLineStatsBytesSent] },
{ &tVPLineStatsLeafInfo[eVPLineStatsBytesReceived] },
{ &tVPLineStatsLeafInfo[eVPLineStatsPacketsLost] },
{ &tVPLineStatsLeafInfo[eVPLineStatsIncomingCallsReceived] },
{ &tVPLineStatsLeafInfo[eVPLineStatsIncomingCallsAnswered] },
{ &tVPLineStatsLeafInfo[eVPLineStatsIncomingCallsConnected] },
{ &tVPLineStatsLeafInfo[eVPLineStatsIncomingCallsFailed] },
{ &tVPLineStatsLeafInfo[eVPLineStatsOutgoingCallsAttempted] },
{ &tVPLineStatsLeafInfo[eVPLineStatsOutgoingCallsAnswered] },
{ &tVPLineStatsLeafInfo[eVPLineStatsOutgoingCallsConnected] },
{ &tVPLineStatsLeafInfo[eVPLineStatsOutgoingCallsFailed] },
{ NULL }
};

//--- VoiceProfileLine leaf ---//
struct CWMP_OP tVPLineLeafOP = { getVoiceProfileLineEntity,	setVoiceProfileLineEntity };
struct CWMP_PRMT tVoiceProfileLineLeafInfo[] =
{
/*(name,					type,				flag,							op)*/
{"Enable",				eCWMP_tSTRING,		CWMP_READ | CWMP_WRITE,	&tVPLineLeafOP},
{"DirectoryNumber",		eCWMP_tSTRING,		CWMP_READ | CWMP_WRITE,	&tVPLineLeafOP},
{"Status",				eCWMP_tSTRING,		CWMP_READ,					&tVPLineLeafOP},
};

enum eVoiceProfileLineLeaf
{
	eVPLineEnable,
	eVPLineDirectoryNumber,
	eVPLineStatus,
};

struct CWMP_LEAF tVoiceProfileLineEntityLeaf[] =
{
{ &tVoiceProfileLineLeafInfo[eVPLineEnable] },
{ &tVoiceProfileLineLeafInfo[eVPLineDirectoryNumber] },
{ &tVoiceProfileLineLeafInfo[eVPLineStatus] },
{ NULL }
};
//--- VoiceProfileLineCodecList leaf ---//
struct CWMP_OP tVPLineCodecListLeafOP = { getVoiceProfileLineCodecListEntity,	setVoiceProfileLineCodecListEntity };
struct CWMP_PRMT tVPLineCodecListLeafInfo[] =
{
/*(name,					type,				flag,							op)*/
{"EntryID",				eCWMP_tUINT,		CWMP_READ ,					&tVPLineCodecListLeafOP},
{"Codec",				eCWMP_tSTRING,		CWMP_READ ,					&tVPLineCodecListLeafOP},
{"BitRate",				eCWMP_tUINT,		CWMP_READ ,					&tVPLineCodecListLeafOP},
{"PacketizationPeriod",		eCWMP_tSTRING,		CWMP_READ|CWMP_WRITE ,	&tVPLineCodecListLeafOP},
{"SilenceSuppression",		eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE ,	&tVPLineCodecListLeafOP},
{"Enable",				eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE ,	&tVPLineCodecListLeafOP},
{"Priority",				eCWMP_tUINT,		CWMP_READ|CWMP_WRITE ,	&tVPLineCodecListLeafOP},
};

enum eVPLineCodecListLeaf
{
	eVPLineCodecListEntryID,
	eVPLineCodecListCodec,
	eVPLineCodecListBitRate,
	eVPLineCodecListPacketizationPeriod,
	eVPLineCodecListSilenceSuppression,
	eVPLineCodecLisEnablet,
	eVPLineCodecListPriority,
};

struct CWMP_LEAF tVPLineCodecListEntityLeaf[] =
{
{ &tVPLineCodecListLeafInfo[eVPLineCodecListEntryID] },
{ &tVPLineCodecListLeafInfo[eVPLineCodecListCodec] },
{ &tVPLineCodecListLeafInfo[eVPLineCodecListBitRate] },
{ &tVPLineCodecListLeafInfo[eVPLineCodecListPacketizationPeriod] },
{ &tVPLineCodecListLeafInfo[eVPLineCodecListSilenceSuppression] },
{ &tVPLineCodecListLeafInfo[eVPLineCodecLisEnablet] },
{ &tVPLineCodecListLeafInfo[eVPLineCodecListPriority] },
{ NULL }
};



//Instance VoiceService.{i}.VoiceProfile.{i}.Line.-{i}.Codec.List.{i}. //
struct CWMP_PRMT tVoiceProfileLineCodecListOjbectInfo[] =
{
/*(name,	type,		flag,					op)*/
{"0",					eCWMP_tOBJECT,		CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eVoiceProfileLineCodecListOjbect
{
	eVoiceProfileLineCodecList0
};
struct CWMP_LINKNODE tVoiceProfileLineCodecListObject[] =
{
/*info,  																leaf,			next,		sibling,		instnum)*/
//{&tVoiceProfileLineCodecListOjbectInfo[eVoiceProfileLineCodecList0],		NULL,		NULL,		NULL,			0},
{&tVoiceProfileLineCodecListOjbectInfo[eVoiceProfileLineCodecList0],	tVPLineCodecListEntityLeaf,		NULL,		NULL,			0},
};

// --- VoiceProfileLineCodec 's objects ------
struct CWMP_OP tVPLineCodecLisTObjOP = { NULL, objVoiceProfileLineCodecList};
struct CWMP_PRMT tVoiceProfileLineCodecObjectInfo[] = 
{

/*(name,				type,			flag,			OP)*/
{"List",			eCWMP_tOBJECT,	CWMP_READ,	&tVPLineCodecLisTObjOP},
};
enum eVoiceProfileLineCodecObjects
{
	eVPLineCodecList,
};


struct CWMP_NODE tVPLineCodecEntityObjects[] = 
{
{&tVoiceProfileLineCodecObjectInfo[eVPLineCodecList],		NULL,	NULL},
{NULL,		NULL,	NULL},
};

// --- VoiceProfileLine's objects ------
struct CWMP_PRMT tVoiceProfileLineEntityObjectInfo[] = 
{
/*(name,				type,			flag,			OP)*/
{"SIP",				eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"Codec",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"Stats",				eCWMP_tOBJECT,	CWMP_READ,	NULL},
};
enum eVoiceProfileLineEntityObjects
{
	eVPLineSIP,
	eVPLineCodec,
	eVPLineStats
};


struct CWMP_NODE tVoiceProfileLineEntityObject[] = 
{
{&tVoiceProfileLineEntityObjectInfo[eVPLineSIP],		tVPLineSIPEntityLeaf,		NULL},
//{&tVoiceProfileLineEntityObjectInfo[eVPLineCodec],	tVPLineCodecEntityLeaf,		NULL},
{&tVoiceProfileLineEntityObjectInfo[eVPLineCodec],	tVPLineCodecEntityLeaf,		tVPLineCodecEntityObjects},
{&tVoiceProfileLineEntityObjectInfo[eVPLineStats],	tVPLineStatsEntityLeaf,	NULL},
{NULL,		NULL,	NULL},
};

// ---- VoiceProfileLine  instance -----
struct CWMP_PRMT tVoiceProfileLineOjbectInfo[] =
{
/*(name,	type,		flag,					op)*/
{"0",					eCWMP_tOBJECT,		CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eVoiceProfileLineOjbect
{
	eVoiceProfileLine0
};
struct CWMP_LINKNODE tVoiceProfileLineObject[] =
{
/*info,  									leaf,							next,		sibling,		instnum)*/
{&tVoiceProfileLineOjbectInfo[eVoiceProfileLine0],	tVoiceProfileLineEntityLeaf,		tVoiceProfileLineEntityObject,		NULL,			0},
};


static int getVoiceProfileLineCodecListEntity(char *name, struct CWMP_LEAF *leaf, int *type, void **data)
{
	struct CWMP_PRMT	*entity = leaf->info;
	/* VoiceServer.{i}.VoiceProfile.{i}.SIP.Line.{i}.Codec.List.{i}. */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nCodecListInstNum;
	unsigned int nNumberOfLine;
	union {
		char PacketizationPeriod[ 64 ];
		int SilenceSuppression;
		unsigned int Priority;
	} s;

	const char *pszLastname = entity ->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
	
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line_List( name, &nVoiceProfileInstNum, &nSipLineInstNum, &nCodecListInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;
	
	if( -- nCodecListInstNum >= MAX_CODEC_LIST )	/* convert to zero based */
		return -1;
	
	*type = entity ->type;

	if( strcmp( pszLastname, "EntryId" ) == 0 ) {
		*data = uintdup( nCodecListInstNum + 1 );	/* 1 based */
	} else if( strcmp( pszLastname, "Codec" ) == 0 ) {
		*data = strdup( lstCodecs[ nCodecListInstNum ].pszCodec );
	} else if( strcmp( pszLastname, "BitRate" ) == 0 ) {
		*data = uintdup( lstCodecs[ nCodecListInstNum ].nBitRate );
	} else if( strcmp( pszLastname, "PacketizationPeriod" ) == 0 ) {
		mib_get_type3( MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__PACKETIZATION_PERIOD, 
					   nVoiceProfileInstNum, nSipLineInstNum, nCodecListInstNum,
					   &s.PacketizationPeriod );
		*data = strdup( s.PacketizationPeriod );
	} else if( strcmp( pszLastname, "SilenceSuppression" ) == 0 ) {
		mib_get_type3( MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__SILENCE_SUPPRESSION, 
					   nVoiceProfileInstNum, nSipLineInstNum, nCodecListInstNum,
					   &s.SilenceSuppression );
		*data = booldup( s.SilenceSuppression );
	} else if( strcmp( pszLastname, "Priority" ) == 0 ) {
		mib_get_type3( MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__PRIORITY, 
					   nVoiceProfileInstNum, nSipLineInstNum, nCodecListInstNum,
					   &s.Priority );
		*data = uintdup( s.Priority );
	} else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}

static int setVoiceProfileLineCodecListEntity(char *name, struct CWMP_LEAF *leaf, int type, void *data)
{
	struct CWMP_PRMT	*entity = leaf->info;
	/* VoiceService.{i}.VoiceProfile.{i}.Line.{i}.Codec.List.{i}. */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nCodecListInstNum;
	unsigned int nNumberOfLine;
	const char *pszLastname = entity ->name;
#ifdef _CWMP_APPLY_
	int	ret_succ = 0;
#else
	int	ret_succ = 1;	
#endif
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
	
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line_List( name, &nVoiceProfileInstNum, &nSipLineInstNum, &nCodecListInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;
	
	if( -- nCodecListInstNum >= MAX_CODEC_LIST )	/* convert to zero based */
		return -1;

	if( entity ->type != type )
		return ERR_9006;

	if( strcmp( pszLastname, "EntryId" ) == 0 ) {
		return ERR_9008;
	} else if( strcmp( pszLastname, "Codec" ) == 0 ) {
		return ERR_9008;
	} else if( strcmp( pszLastname, "BitRate" ) == 0 ) {
		return ERR_9008;
	} else if( strcmp( pszLastname, "PacketizationPeriod" ) == 0 ) {
		mib_set_type3( MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__PACKETIZATION_PERIOD, 
					   nVoiceProfileInstNum, nSipLineInstNum, nCodecListInstNum,
					   data );
		return ret_succ;
	} else if( strcmp( pszLastname, "SilenceSuppression" ) == 0 ) {
		mib_set_type3( MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__SILENCE_SUPPRESSION, 
					   nVoiceProfileInstNum, nSipLineInstNum, nCodecListInstNum,
					   data );
		return ret_succ;
	} else if( strcmp( pszLastname, "Priority" ) == 0 ) {
		mib_set_type3( MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__PRIORITY, 
					   nVoiceProfileInstNum, nSipLineInstNum, nCodecListInstNum,
					   data );
		return ret_succ;
	} else {
		return ERR_9005;
	}

	return 0;
}


static int getVoiceProfileLineStatusEntity(char *name, struct CWMP_LEAF *leaf, int *type, void **data)
{
	struct CWMP_PRMT	*entity = leaf->info;
	/* VoiceServer.{i}.VoiceProfile.{i}.SIP.Line.{i}.Stats. */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;
	unsigned int chid;
	extern cwmpEvtMsg pEvtMsg;
	union {
		TstVoipRtpStatistics rtpStatistics;
	} s;

	const char *pszLastname = entity ->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;
	
	*type = entity ->type;

	/*
	 * FIXME: Now, instance number of 'Line' is seens as channel ID,  
	 *        but specification indicate thta DirectoryNumber or PhyReferenceList 
	 *        is used to identify or associate with physical interface. 
	 */
	chid = nSipLineInstNum;

	if( strcmp( pszLastname, "PacketsSent" ) == 0 ) {
		if( rtk_Get_Rtp_Statistics( chid, 0, &s.rtpStatistics ) )
			return ERR_9002;	/* Internal error */
		*data = uintdup( s.rtpStatistics.nTxRtpStatsCountPacket );
	} else if( strcmp( pszLastname, "PacketsReceived" ) == 0 ) {
		if( rtk_Get_Rtp_Statistics( chid, 0, &s.rtpStatistics ) )
			return ERR_9002;	/* Internal error */
		*data = uintdup( s.rtpStatistics.nRxRtpStatsCountPacket );
	} else if( strcmp( pszLastname, "BytesSent" ) == 0 ) {
		if( rtk_Get_Rtp_Statistics( chid, 0, &s.rtpStatistics ) )
			return ERR_9002;	/* Internal error */
		*data = uintdup( s.rtpStatistics.nTxRtpStatsCountByte );
	} else if( strcmp( pszLastname, "BytesReceived" ) == 0 ) {
		if( rtk_Get_Rtp_Statistics( chid, 0, &s.rtpStatistics ) )
			return ERR_9002;	/* Internal error */
		*data = uintdup( s.rtpStatistics.nRxRtpStatsCountByte );
	} else if( strcmp( pszLastname, "PacketsLost" ) == 0 ) {
		if( rtk_Get_Rtp_Statistics( chid, 0, &s.rtpStatistics ) )
			return ERR_9002;	/* Internal error */
		*data = uintdup( s.rtpStatistics.nRxRtpStatsLostPacket );
	} else if( strcmp( pszLastname, "IncomingCallsReceived" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsReceived);
		printf("@@@@@Line %d IncomingCallsReceived:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsReceived);
	} else if( strcmp( pszLastname, "IncomingCallsAnswered" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsAnswered);
		printf("@@@@@Line %d IncomingCallsAnswered:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsAnswered);
	} else if( strcmp( pszLastname, "IncomingCallsConnected" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsConnected);
		printf("@@@@@Line %d IncomingCallsConnected:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsConnected);
	} else if( strcmp( pszLastname, "IncomingCallsFailed" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsFailed);
		printf("@@@@@Line %d IncomingCallsFailed:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsFailed);
	} else if( strcmp( pszLastname, "OutgoingCallsAttempted" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsAttempted);
		printf("@@@@@Line %d OutgoingCallsAttempted:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsAttempted);
	} else if( strcmp( pszLastname, "OutgoingCallsAnswered" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsAnswered);
		printf("@@@@@Line %d OutgoingCallsAnswered:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsAnswered);
	} else if( strcmp( pszLastname, "OutgoingCallsConnected" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsConnected);
		printf("@@@@@Line %d OutgoingCallsConnected:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsConnected);
	} else if( strcmp( pszLastname, "OutgoingCallsFailed" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsFailed);
		printf("@@@@@Line %d OutgoingCallsFailed:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsFailed);
	} else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}

static int setVoiceProfileLineStatusEntity(char *name, struct CWMP_LEAF *leaf, int type, void *data)
{
	return ERR_9008;	/* Parameters are read only */
}

static int getVoiceProfileLineCodecEntity(char *name, struct CWMP_LEAF *leaf, int *type, void **data)
{
	struct CWMP_PRMT	*entity = leaf->info;
	/* VoiceServer.{i}.VoiceProfile.{i}.SIP.Line.{i}.SIP. */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;
	union {
		char AuthUserName[ 128 ];
		char AuthPassword[ 128 ];
		char URI[ 389 ];
	} s;

	const char *pszLastname = entity ->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;
	
	*type = entity ->type;


	if(strcmp( pszLastname, "TransmitCodec" ) == 0 ) {
		*data = strdup( "none" );
	}else if (strcmp( pszLastname, "ReceiveCodec" ) == 0 ) {
		*data = strdup( "none" );
	}else if (strcmp( pszLastname, "TransmitBitRate" ) == 0 ) {	
		*data = uintdup(0);
	}else if (strcmp( pszLastname, "ReceiveBitRate" ) == 0 ) {	
		*data = uintdup(0);
	}else if (strcmp( pszLastname, "TransmitSilenceSuppression" ) == 0 ) {	
		*data = booldup(0);
	}else if (strcmp( pszLastname, "ReceiveSilenceSuppression" ) == 0 ) {	
		*data = booldup(0);
	}else if (strcmp( pszLastname, "TransmitPacketizationPeriod" ) == 0 ) {	
		*data = uintdup(20);//?
	}else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}


static int getVoiceProfileLineSipEntity(char *name, struct CWMP_LEAF *leaf, int *type, void **data)
{
	struct CWMP_PRMT	*entity = leaf->info;
	/* VoiceServer.{i}.VoiceProfile.{i}.SIP.Line.{i}.SIP. */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;
	union {
		char AuthUserName[ 128 ];
		char AuthPassword[ 128 ];
		char URI[ 389 ];
	} s;

	const char *pszLastname = entity ->name;
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;
	
	*type = entity ->type;

	if( strcmp( pszLastname, "AuthUserName" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__SIP__AUTH_USER_NAME, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.AuthUserName );
		*data = strdup( s.AuthUserName );
	} else if( strcmp( pszLastname, "AuthPassword" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__SIP__AUTH_PASSWORD, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   s.AuthPassword );
		*data = strdup( s.AuthPassword );
	} else if( strcmp( pszLastname, "URI" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__SIP__URI, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.URI );
		*data = strdup( s.URI );
	} else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}

static int setVoiceProfileLineSipEntity(char *name, struct CWMP_LEAF *leaf, int type, void *data)
{
	struct CWMP_PRMT	*entity = leaf->info;
	/* VoiceServer.{i}.VoiceProfile.{i}.SIP.Line.{i}.SIP. */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;
	const char *pszLastname = entity ->name;
#ifdef _CWMP_APPLY_
	int	ret_succ = 0;
#else
	int	ret_succ = 1;	
#endif
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;

	if( entity ->type != type )
		return ERR_9006;

	if( strcmp( pszLastname, "AuthUserName" ) == 0 ) {
		mib_set_type2( MIB_VOICE_PROFILE__LINE__SIP__AUTH_USER_NAME, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   data );
		return ret_succ;
	} else if( strcmp( pszLastname, "AuthPassword" ) == 0 ) {
		mib_set_type2( MIB_VOICE_PROFILE__LINE__SIP__AUTH_PASSWORD, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   data );
		return ret_succ;
	} else if( strcmp( pszLastname, "URI" ) == 0 ) {
		mib_set_type2( MIB_VOICE_PROFILE__LINE__SIP__URI, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   data );
		return ret_succ;
	} else {
		return ERR_9005;
	}

	return 0;
}

static int getVoiceProfileLineEntity(char *name, struct CWMP_LEAF *leaf, int *type, void **data)
{
	struct CWMP_PRMT	*entity = leaf->info;
	/* VoiceServer.{i}.VoiceProfile.{i}.SIP.Line.{i}. */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;
	union {
		enable_t Enable;
		char DirectoryNumber[ 32 ];
		line_status_t Status;
	} s;
	
	const char *pszLastname = entity ->name;
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;


	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;


	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;

	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;
	
	*type = entity ->type;

	if( strcmp( pszLastname, "Enable" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__ENABLE, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.Enable );
		*data = strdup_Enable( s.Enable );
	} else if( strcmp( pszLastname, "DirectoryNumber" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__DIRECTOR_NUMBER, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   s.DirectoryNumber );
		*data = strdup( s.DirectoryNumber );
	} else if( strcmp( pszLastname, "Status" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__STATUS, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.Status );
		*data = strdup_LineStatus( s.Status );
	} else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}

static int setVoiceProfileLineEntity(char *name, struct CWMP_LEAF *leaf, int type, void *data)
{
	struct CWMP_PRMT	*entity = leaf->info;
	/* VoiceServer.{i}.VoiceProfile.{i}.SIP.Line.{i}. */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;
	union {
		enable_t Enable;
		char DirectoryNumber[ 32 ];
		line_status_t Status;
	} s;
	const char *pszLastname = entity ->name;
#ifdef _CWMP_APPLY_
	int	ret_succ = 0;
#else
	int	ret_succ = 1;	
#endif
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;
	
	if( entity ->type != type )
		return ERR_9006;

	if( strcmp( pszLastname, "Enable" ) == 0 ) {
		str2id_Enable( data, &s.Enable );
		mib_set_type2( MIB_VOICE_PROFILE__LINE__ENABLE, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.Enable );
		return ret_succ;
	} else if( strcmp( pszLastname, "DirectoryNumber" ) == 0 ) {
		mib_set_type2( MIB_VOICE_PROFILE__LINE__DIRECTOR_NUMBER, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   data );
		return ret_succ;
	} else if( strcmp( pszLastname, "Status" ) == 0 ) {
		return ERR_9008;
	} else {
		return ERR_9005;
	}

	return 0;
}

int objVoiceProfileLine(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;

	switch( type ) {
	case eCWMP_tINITOBJ:
	{
#if 1		
		int num=0,i;
		struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;
		
		//MIB_CE_IP_ROUTE_T *p,route_entity;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		num = 1;//mib_chain_total( MIB_IP_ROUTE_TBL );
		for( i=0; i<num;i++ )
		{
			//p = &route_entity;
			//if( !mib_chain_get( MIB_IP_ROUTE_TBL, i, (void*)p ) )
			//	continue;
			
			//if( p->InstanceNum==0 ) //maybe createn by web or cli
			//{
			//	MaxInstNum++;
			//	p->InstanceNum = MaxInstNum;
			//	mib_chain_update( MIB_IP_ROUTE_TBL, (unsigned char*)p, i );
			//}else
			//	MaxInstNum = p->InstanceNum;
			
			if( create_Object( c, tVoiceProfileLineObject, sizeof(tVoiceProfileLineObject), MAX_LINE_PER_PROFILE, 1 ) < 0 )
				return -1;
			//c = & (*c)->sibling;
		}
		add_objectNum( name, 1 );
#endif		
		return 0;
	}

	}
	return -1;
}

static int objVoiceProfileLineCodecList(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;

	switch( type ) {
	case eCWMP_tINITOBJ:
	{
#if 1		
		int num=0,i;
		struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;
		
		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		num = 1;//mib_chain_total( MIB_IP_ROUTE_TBL );
		for( i=0; i<num;i++ )
		{
			if( create_Object( c, tVoiceProfileLineCodecListObject, sizeof(tVoiceProfileLineCodecListObject), MAX_CODEC_LIST, 1 ) < 0 )			
				return -1;
		}
		add_objectNum( name, 1 );
#endif		
		return 0;
	}

	}
	return -1;
}

