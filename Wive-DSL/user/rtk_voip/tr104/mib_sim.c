#include <string.h>
#include "mib_def.h"
#include "mib_sim.h"
#include "prmt_limit.h"

typedef int boolean;

typedef struct VoiceProfile_Line_Codec_List_s {
	//unsigned int			EntryID;
	//char					Codec[ 64 ];
	//unsigned int			BitRate;
	char					PacketizationPeriod[ 64 ];
	boolean					SilenceSuppression;
	unsigned int			Priority;
} VoiceProfile_Line_Codec_List_t;

typedef struct VoiceProfile_Line_Codec_s {
	VoiceProfile_Line_Codec_List_t	List[ MAX_CODEC_LIST ];
} VoiceProfile_Line_Codec_t;

typedef struct VoiceProfile_Line_Stats_s {
	char					reserved;
} VoiceProfile_Line_Stats_t;

typedef struct VoiceProfile_Line_SIP_s {
	char					AuthUserName[ 128 ];
	char					AuthPassword[ 128 ];
	char					URI[ 389 ];
} VoiceProfile_Line_SIP_t;

typedef struct VoiceProfile_Line_s {
	enable_t				Enable;
	char					DirectoryNumber[ 32 ];
	line_status_t			Status;
	VoiceProfile_Line_SIP_t	SIP;
	VoiceProfile_Line_Stats_t	Stats;
	VoiceProfile_Line_Codec_t	Codec;
} VoiceProfile_Line_t;

typedef struct VoiceProfile_SIP_s {
	char					ProxyServer[ 256 ];
	unsigned int			ProxyServerPort;
	transport_t				ProxyServerTransport;
	char					RegistrationServer[ 256 ];
	unsigned int			RegistrationServerPort;
	transport_t				ResistrationServerTransport;
	char					UserAgentDomain[ 256 ];
	unsigned int			UserAgentPort;
	transport_t				UserAgentTransport;
	char					OutboundProxy[ 256 ];
	unsigned int			OutboundProxyPort;
	char					Organization[ 256 ];
	unsigned int			RegistrationPeriod;
} VoiceProfile_SIP_t;

typedef struct ServiceProvideInfo_s {
	char					Name[ 256 ];
} ServiceProvideInfo_t;

typedef struct VoiceProfile_s {
	enable_t				Enable;
	boolean					Reset;
	unsigned int			NumberOfLine;
	char					Name[ 64 ];
	signaling_protocol_t	SignalingProtocol;
	unsigned int			MaxSessions;
	DTMF_method_t			DTMFMethod;
	ServiceProvideInfo_t	ServiceProvideInfo;
	VoiceProfile_SIP_t		SIP;
	VoiceProfile_Line_t		Line[ MAX_LINE_PER_PROFILE ];
} VoiceProfile_t;

VoiceProfile_t	VoiceProfile[ MAX_PROFILE_COUNT ] = { {
	ENABLE,
	0,
	2,	/* Number of Line */
	"Profile 1",
	SIP,
	2,
	IN_BAND,
	{	/* ServiceProvideInfo */
		"Powered by rtkvoip",
	},
	{	/* SIP */
		"192.168.1.254",
		5060,
		UDP,
		"192.168.1.254",
		5060,
		UDP,
		"172.21.69.34",
		9990,
		UDP,
		"172.21.69.33",
		9980,
		"organization header",
		3600,
	},
	{	/* Line */
		{	/* Line.1 --> In simulation, it was seen as channel 0. */
			ENABLE,
			"1",	/* DirectoryNumber */
			UP,
			{	/* SIP */
				"rtkvoip",
				"rtkvoip123",
				"cpe@localhost",
			},
			{	/* Stats */
				'1',	/* dynamic retrieve */
			},
			{	/* Codec */
				{	/* List */
					{ "20", 1, 1 },		/* G.711MuLaw */
					{ "20", 1, 2 },		/* G.711ALaw */
					{ "20", 1, 3 },		/* G.726 16k */
					{ "20", 1, 4 },		/* G.726 24k */
					{ "20", 1, 5 },		/* G.726 32k */
					{ "20", 1, 6 },		/* G.726 40k */
					{ "20", 1, 7 },		/* G.729 */
					{ "30", 1, 8 },		/* G.723.1 5.3k */
					{ "30", 1, 9 },		/* G.723.1 6.3k */
					{ "20", 1, 10 },	/* GSM-FR */
				}
			},
		},
		{	/* Line.2 --> In simulation, it was seen as channel 1. */
			DISABLE,
			"2",	/* DirectoryNumber */
			DISABLED,
			{	/* SIP */
				"rtkvoip2",
				"rtkvoip2123",
				"cpe2@localhost",
			},
			{	/* Stats */
				'1',	/* dynamic retrieve */
			},
			{	/* Codec */
				{	/* List */
					{ "10", 0, 10 },	/* G.711MuLaw */
					{ "10", 0, 9 },		/* G.711ALaw */
					{ "10", 0, 8 },		/* G.726 16k */
					{ "10", 0, 7 },		/* G.726 24k */
					{ "10", 0, 6 },		/* G.726 32k */
					{ "10", 0, 5 },		/* G.726 40k */
					{ "10", 0, 4 },		/* G.729 */
					{ "60", 0, 3 },		/* G.723.1 5.3k */
					{ "60", 0, 2 },		/* G.723.1 6.3k */
					{ "40", 0, 1 },		/* GSM-FR */
				}
			},
		},
	}
} };

int mib_get_type1( idMib_t idMib, unsigned int nVoiceProfileInstNum, 
				   void *pData )
{	
	/* VoiceProfile.{ i }. */
	const VoiceProfile_t *pVoiceProfile;
	
	if( nVoiceProfileInstNum >= MAX_PROFILE_COUNT )
		return 0;

	pVoiceProfile = &VoiceProfile[ nVoiceProfileInstNum ];

	switch( idMib ) {
	case MIB_VOICE_PROFILE__ENABLE:
		*( ( enable_t * )pData ) = pVoiceProfile ->Enable;
		break;
		
	case MIB_VOICE_PROFILE__RESET:
		*( ( boolean * )pData ) = pVoiceProfile ->Reset;
		break;
		
	case MIB_VOICE_PROFILE__NUMBER_OF_LINES:
		*( ( unsigned int * )pData ) = pVoiceProfile ->NumberOfLine;
		break;
		
	case MIB_VOICE_PROFILE__NAME:
		strcpy( ( char * )pData, pVoiceProfile ->Name );
		break;

	case MIB_VOICE_PROFILE__SIGNALING_PROTOCOL:
		*( ( signaling_protocol_t * )pData ) = pVoiceProfile ->SignalingProtocol;
		break;
		
	case MIB_VOICE_PROFILE__MAX_SESSIONS:
		*( ( unsigned int * )pData ) = pVoiceProfile ->MaxSessions;
		break;
	
	case MIB_VOICE_PROFILE__DTMF_METHOD:
		*( ( DTMF_method_t * )pData ) = pVoiceProfile ->DTMFMethod;
		break;
		
	case MIB_VOICE_PROFILE__SERVICE_PROVIDE_INFO__NAME:
		strcpy( ( char * )pData, pVoiceProfile ->ServiceProvideInfo.Name );
		break;

	case MIB_VOICE_PROFILE__SIP__PROXY_SERVER:
		strcpy( ( char * )pData, pVoiceProfile ->SIP.ProxyServer );
		break;
		
	case MIB_VOICE_PROFILE__SIP__PROXY_SERVER_PORT:
		*( ( unsigned int * )pData ) = pVoiceProfile ->SIP.ProxyServerPort;
		break;
		
	case MIB_VOICE_PROFILE__SIP__PROXY_SERVER_TRANSPORT:
		*( ( transport_t * )pData ) = pVoiceProfile ->SIP.ProxyServerTransport;
		break;
		
	case MIB_VOICE_PROFILE__SIP__REGISTRATION_SERVER:
		strcpy( ( char * )pData, pVoiceProfile ->SIP.RegistrationServer );
		break;
		
	case MIB_VOICE_PROFILE__SIP__REGISTRATION_SERVER_PORT:
		*( ( unsigned int * )pData ) = pVoiceProfile ->SIP.RegistrationServerPort;
		break;
		
	case MIB_VOICE_PROFILE__SIP__REGISTRATION_SERVER_TRANSPORT:
		*( ( transport_t * )pData ) = pVoiceProfile ->SIP.ResistrationServerTransport;
		break;
		
	case MIB_VOICE_PROFILE__SIP__USER_AGENT_DOMAIN:
		strcpy( ( char * )pData, pVoiceProfile ->SIP.UserAgentDomain );
		break;

	case MIB_VOICE_PROFILE__SIP__USER_AGENT_PORT:
		*( ( unsigned int * )pData ) = pVoiceProfile ->SIP.UserAgentPort;
		break;

	case MIB_VOICE_PROFILE__SIP__USER_AGENT_TRANSPORT:
		*( ( transport_t * )pData ) = pVoiceProfile ->SIP.UserAgentTransport;
		break;

	case MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY:
		strcpy( ( char * )pData, pVoiceProfile ->SIP.OutboundProxy );
		break;

	case MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY_PORT:
		*( ( unsigned int * )pData ) = pVoiceProfile ->SIP.OutboundProxyPort;
		break;

	case MIB_VOICE_PROFILE__SIP__ORGANIZATION:
		strcpy( ( char * )pData, pVoiceProfile ->SIP.Organization );
		break;

	case MIB_VOICE_PROFILE__SIP__REGISTRATION_PERIOD:
		*( ( unsigned int * )pData ) = pVoiceProfile ->SIP.RegistrationPeriod;
		break;
		
	default:
		return 0;	/* range error */
		break;
	}

	return 1;
}

int mib_set_type1( idMib_t idMib, unsigned int nVoiceProfileInstNum, 
				   void *pData )
{	
	/* VoiceProfile.{ i }. */
	VoiceProfile_t *pVoiceProfile;

	if( nVoiceProfileInstNum >= MAX_PROFILE_COUNT )
		return 0;

	pVoiceProfile = &VoiceProfile[ nVoiceProfileInstNum ];

	switch( idMib ) {
	case MIB_VOICE_PROFILE__ENABLE:
		pVoiceProfile ->Enable = *( ( enable_t * )pData );
		break;
		
	case MIB_VOICE_PROFILE__RESET:
		pVoiceProfile ->Reset = *( ( boolean * )pData );
		break;
		
	case MIB_VOICE_PROFILE__NUMBER_OF_LINES:
		pVoiceProfile ->NumberOfLine = *( ( unsigned int * )pData );
		break;
		
	case MIB_VOICE_PROFILE__NAME:
		strcpy( pVoiceProfile ->Name, ( char * )pData );
		break;

	case MIB_VOICE_PROFILE__SIGNALING_PROTOCOL:
		pVoiceProfile ->SignalingProtocol = *( ( signaling_protocol_t * )pData );
		break;
		
	case MIB_VOICE_PROFILE__MAX_SESSIONS:
		pVoiceProfile ->MaxSessions = *( ( unsigned int * )pData );
		break;
	
	case MIB_VOICE_PROFILE__DTMF_METHOD:
		pVoiceProfile ->DTMFMethod = *( ( DTMF_method_t * )pData );
		break;
		
	case MIB_VOICE_PROFILE__SERVICE_PROVIDE_INFO__NAME:
		strcpy( pVoiceProfile ->ServiceProvideInfo.Name, ( char * )pData );
		break;

	case MIB_VOICE_PROFILE__SIP__PROXY_SERVER:
		strcpy( pVoiceProfile ->SIP.ProxyServer, ( char * )pData );
		break;
		
	case MIB_VOICE_PROFILE__SIP__PROXY_SERVER_PORT:
		pVoiceProfile ->SIP.ProxyServerPort = *( ( unsigned int * )pData );
		break;
		
	case MIB_VOICE_PROFILE__SIP__PROXY_SERVER_TRANSPORT:
		pVoiceProfile ->SIP.ProxyServerTransport = *( ( transport_t * )pData );
		break;
		
	case MIB_VOICE_PROFILE__SIP__REGISTRATION_SERVER:
		strcpy( pVoiceProfile ->SIP.RegistrationServer, ( char * )pData );
		break;
		
	case MIB_VOICE_PROFILE__SIP__REGISTRATION_SERVER_PORT:
		pVoiceProfile ->SIP.RegistrationServerPort = *( ( unsigned int * )pData );
		break;
		
	case MIB_VOICE_PROFILE__SIP__REGISTRATION_SERVER_TRANSPORT:
		pVoiceProfile ->SIP.ResistrationServerTransport = *( ( transport_t * )pData );
		break;
		
	case MIB_VOICE_PROFILE__SIP__USER_AGENT_DOMAIN:
		strcpy( pVoiceProfile ->SIP.UserAgentDomain, ( char * )pData );
		break;

	case MIB_VOICE_PROFILE__SIP__USER_AGENT_PORT:
		pVoiceProfile ->SIP.UserAgentPort = *( ( unsigned int * )pData );
		break;

	case MIB_VOICE_PROFILE__SIP__USER_AGENT_TRANSPORT:
		pVoiceProfile ->SIP.UserAgentTransport = *( ( transport_t * )pData );
		break;

	case MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY:
		strcpy( pVoiceProfile ->SIP.OutboundProxy, ( char * )pData );
		break;

	case MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY_PORT:
		pVoiceProfile ->SIP.OutboundProxyPort = *( ( unsigned int * )pData );
		break;

	case MIB_VOICE_PROFILE__SIP__ORGANIZATION:
		strcpy( pVoiceProfile ->SIP.Organization, ( char * )pData );
		break;

	case MIB_VOICE_PROFILE__SIP__REGISTRATION_PERIOD:
		pVoiceProfile ->SIP.RegistrationPeriod = *( ( unsigned int * )pData );
		break;
		
	default:
		return 0;	/* range error */
		break;
	}

	return 1;
}

int mib_get_type2( idMib_t idMib, unsigned int nVoiceProfileInstNum, 
				   unsigned int nLineInstNum,
				   void *pData )
{
	/* VoiceProfile.{ i }.Line.{i}. */
	const VoiceProfile_Line_t *pVoiceProfile_Line;

	if( ( nVoiceProfileInstNum >= MAX_PROFILE_COUNT ) ||
		( nLineInstNum >= MAX_LINE_PER_PROFILE ) )
	{
		return 0;
	}
	
	pVoiceProfile_Line = &VoiceProfile[ nVoiceProfileInstNum ].Line[ nLineInstNum ];

	switch( idMib ) {
	case MIB_VOICE_PROFILE__LINE__ENABLE:
		*( ( enable_t * )pData ) = pVoiceProfile_Line ->Enable;
		break;

	case MIB_VOICE_PROFILE__LINE__DIRECTOR_NUMBER:
		strcpy( pData, pVoiceProfile_Line ->DirectoryNumber );
		break;

	case MIB_VOICE_PROFILE__LINE__STATUS:
		*( ( line_status_t * )pData ) = pVoiceProfile_Line ->Status;
		break;

	case MIB_VOICE_PROFILE__LINE__SIP__AUTH_USER_NAME:
		strcpy( pData, pVoiceProfile_Line ->SIP.AuthUserName );
		break;

	case MIB_VOICE_PROFILE__LINE__SIP__AUTH_PASSWORD:
		strcpy( pData, pVoiceProfile_Line ->SIP.AuthPassword );
		break;

	case MIB_VOICE_PROFILE__LINE__SIP__URI:
		strcpy( pData, pVoiceProfile_Line ->SIP.URI );
		break;

	case MIB_VOICE_PROFILE__LINE__SIP__STATS__PACKETS_SENT:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__PACKETS_RECEIVED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__BYTES_SENT:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__BYTES_RECEIVED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__PACKETS_LOST:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__INCOMING_CALLS_RECEIVED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__INCOMING_CALLS_ANSWERED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__INCOMING_CALLS_CONNECTED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__INCOMING_CALLS_FAILED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__OUTGOING_CALLS_ATTEMPTED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__OUTGOING_CALLS_ANSWERED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__OUTGOING_CALLS_CONNECTED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__OUTGOING_CALLS_FAILED:
		break;
		
	default:
		return 0;	/* range error */
		break;
	}

	return 1;
}

int mib_set_type2( idMib_t idMib, unsigned int nVoiceProfileInstNum, 
				   unsigned int nLineInstNum,
				   void *pData )
{
	/* VoiceProfile.{ i }.Line.{i}. */
	VoiceProfile_Line_t *pVoiceProfile_Line;

	if( ( nVoiceProfileInstNum >= MAX_PROFILE_COUNT ) ||
		( nLineInstNum >= MAX_LINE_PER_PROFILE ) )
	{
		return 0;
	}

	pVoiceProfile_Line = &VoiceProfile[ nVoiceProfileInstNum ].Line[ nLineInstNum ];
	
	switch( idMib ) {
	case MIB_VOICE_PROFILE__LINE__ENABLE:
		pVoiceProfile_Line ->Enable = *( ( enable_t * )pData );
		break;

	case MIB_VOICE_PROFILE__LINE__DIRECTOR_NUMBER:
		strcpy( pVoiceProfile_Line ->DirectoryNumber, pData );
		break;

	case MIB_VOICE_PROFILE__LINE__STATUS:
		pVoiceProfile_Line ->Status = *( ( line_status_t * )pData );
		break;

	case MIB_VOICE_PROFILE__LINE__SIP__AUTH_USER_NAME:
		strcpy( pVoiceProfile_Line ->SIP.AuthUserName, pData );
		break;

	case MIB_VOICE_PROFILE__LINE__SIP__AUTH_PASSWORD:
		strcpy( pVoiceProfile_Line ->SIP.AuthPassword, pData );
		break;

	case MIB_VOICE_PROFILE__LINE__SIP__URI:
		strcpy( pVoiceProfile_Line ->SIP.URI, pData );
		break;
		
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__PACKETS_SENT:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__PACKETS_RECEIVED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__BYTES_SENT:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__BYTES_RECEIVED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__PACKETS_LOST:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__INCOMING_CALLS_RECEIVED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__INCOMING_CALLS_ANSWERED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__INCOMING_CALLS_CONNECTED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__INCOMING_CALLS_FAILED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__OUTGOING_CALLS_ATTEMPTED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__OUTGOING_CALLS_ANSWERED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__OUTGOING_CALLS_CONNECTED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__OUTGOING_CALLS_FAILED:
		break;
		
	default:
		return 0;	/* range error */
		break;
	}

	return 1;
}

int mib_get_type3( idMib_t idMib, unsigned int nVoiceProfileInstNum, 
				   unsigned int nLineInstNum, unsigned int nListInstNum,
				   void *pData )
{
	/* VoiceProfile.{ i }.Line.{i}.Codec.List.{i}. */
	const VoiceProfile_Line_Codec_List_t *pCodeList;
	
	if( ( nVoiceProfileInstNum >= MAX_PROFILE_COUNT ) ||
		( nLineInstNum >= MAX_LINE_PER_PROFILE ) ||
		( nListInstNum >= MAX_CODEC_LIST ) )
	{
		return 0;
	}

	pCodeList = &VoiceProfile[ nVoiceProfileInstNum ].Line[ nLineInstNum ].Codec.List[ nListInstNum ];

	switch( idMib ) {
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__ENTRY_ID:
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__CODEC:
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__BITRATE:
		break;
		
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__PACKETIZATION_PERIOD:
		strcpy( pData, pCodeList ->PacketizationPeriod );
		break;
		
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__SILENCE_SUPPRESSION:
		*( ( boolean * )pData ) = pCodeList ->SilenceSuppression;
		break;
		
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__PRIORITY:
		*( ( unsigned int * )pData ) = pCodeList ->Priority;
		break;
		
	default:
		return 0;	/* range error */
		break;
	}

	return 1;
}

int mib_set_type3( idMib_t idMib, unsigned int nVoiceProfileInstNum, 
				   unsigned int nLineInstNum, unsigned int nListInstNum,
				   void *pData )
{
	/* VoiceProfile.{ i }.Line.{i}.Codec.List.{i}. */
	VoiceProfile_Line_Codec_List_t *pCodeList;

	if( ( nVoiceProfileInstNum >= MAX_PROFILE_COUNT ) ||
		( nLineInstNum >= MAX_LINE_PER_PROFILE ) ||
		( nListInstNum >= MAX_CODEC_LIST ) )
	{
		return 0;
	}

	pCodeList = &VoiceProfile[ nVoiceProfileInstNum ].Line[ nLineInstNum ].Codec.List[ nListInstNum ];

	switch( idMib ) {
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__ENTRY_ID:
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__CODEC:
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__BITRATE:
		break;
		
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__PACKETIZATION_PERIOD:
		strcpy( pCodeList ->PacketizationPeriod, pData );
		break;
		
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__SILENCE_SUPPRESSION:
		pCodeList ->SilenceSuppression = *( ( boolean * )pData );
		break;
		
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__PRIORITY:
		pCodeList ->Priority = *( ( unsigned int * )pData );
		break;

	default:
		return 0;	/* range error */
		break;
	}

	return 1;
}


