#ifndef _DSP_MAIN_H
#define _DSP_MAIN_H

#include "rtk_voip.h"
#include "myTypes.h"
#include "../rtp/RtpPacket.h"
#ifdef SUPPORT_RTCP
#include "../rtp/RtcpPacket.h"
#endif
//#include "rtp/Rtp.h"

#define G723_RATE63 0
#define G723_RATE53 1

struct _rtp_config_s
{
	uint32 isTcp;
	uint32 remIp;
	uint16 remPort;
	uint32 extIp;
	uint16 extPort;

	uint32 chid;
	uint32 mid;

//	RtpPayloadType uPktFormat;
};
typedef struct _rtp_config_s rtp_config_t;

#ifdef SUPPORT_RTCP
struct _rtcp_config_s
{
	uint32 bOpenRtcp;
	uint16 rtcpRemPort;
	uint16 rtcpExtPort;

	uint32 chid;
	uint32 mid;
};
typedef struct _rtcp_config_s rtcp_config_t;

#endif

typedef struct _channel_config_s
{
#ifdef SUPPORT_3WAYS_AUDIOCONF
	uint32 isConference;
#endif
	uint32 tranSessId;
	uint32 RegSessNum;
        uint32 RegSessId[MAX_SESS_NUM];
	uint32 resource;
	
}channel_config_t;

typedef struct _session_config_s
{
	char   isOpen;
	uint32 myChanId;

}session_config_t;

int DSP_init( void );
int32 DSP_rtpWrite( RtpPacket* pst );
int32 DSP_Read(uint32 chid, uint32 sid, uchar* pBuf, int32 nSize);
int32 DSP_Write(uint32 chid, uint32 sid, uchar* pBuf, int32 nSize);
int32 DSP_CodecRestart(uint32 chid, uint32 sid, RtpPayloadType uPktFormat, uint32 nFramePerPacket, int32 nG723Type, bool bVAD, uint32 nJitterDelay, uint32 nMaxDelay, uint32 nJitterFactor);
void hc_SetPlayTone(uint32 chid, uint32 sid, uint32 nTone, uint bFlag, uint path);
uint32 API_OpenSid(uint32 chid, uint32 mid);
uint32 API_GetSid(uint32 chid, uint32 mid);
uint32 API_GetMid(uint32 chid, uint32 sid);
uint32 API_CloseSid(uint32 chid, uint32 mid);
void API_InitVar(void);

#ifdef SUPPORT_RTCP
int32 DSP_rtcpRead( uint32 chid, uint32 sid, void* packet, uint32 pktLen);
int32 DSP_rtcpWrite( RtcpPacket* pst );
#endif

#ifdef SUPPORT_3WAYS_AUDIOCONF
uint32 chanInfo_SetConference(uint32 chid, uint32 bEnable);
uint32 chanInfo_IsConference(uint32 chid);
#endif
int32 chanInfo_SetTranSessionID(uint32 chid, uint32 sid);
int32 chanInfo_CloseSessionID(uint32 chid, uint32 sid);
uint32 chanInfo_GetTranSessionID(uint32 chid);
uint32 chanInfo_GetRegSessionNum(uint32 chid);
uint32 chanInfo_GetRegSessionID(uint32 chid, uint32 reg_idx);
uint32* chanInfo_GetRegSessionRank(uint32 chid);
uint32 chanInfo_GetChannelbySession(uint32 sid);
uint32 chanInfo_IsSessionFull(void);
uint32 chanInfo_IsActiveSession(uint32 chid, uint32 sid);
uint32 GetFreeSession(uint32 chid);
int32 SetSessionFree(uint32 sid);
int32 DSP_pktRx( uint32 chid, uint32 mid, void* packet, uint32 pktLen, uint32 flags );
#endif		//_DSP_MAIN_H
