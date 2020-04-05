
//
// rtpterm.cpp
//

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/string.h>
//#include <string.h>
#include "rtpterm.h"
#include "rtpTypes.h"
#include "Rtp.h"
#include "../dsp_r0/dspcodec_0.h"
#include "../dsp_r0/dspparam.h"
#include "rtk_voip.h"

//#include <debug.h>

#include "codec_descriptor.h"

///////////////////////////////////////////////////////////////////
// global variable
int m_nSIDFrameLen[MAX_SESS_NUM];                                           // the length of SID frame
#ifdef SUPPORT_RTCP
extern RtpSessionState sessionState[MAX_SESS_NUM];
extern RtpSessionError sessionError[MAX_SESS_NUM];
unsigned char RtcpOpen[MAX_SESS_NUM] = {0}; /* if RtcpOpen[sid] =1, it means Rtcp session sid if open. else if 0, close.*/
#endif
unsigned char RtpOpen[MAX_SESS_NUM] = {0}; /* if RtpOpen[sid] =1, it means Rtp session sid if open. else if 0, close.*/
///////////////////////////////////////////////////////////////////
// static variable
//static CRtpConfig m_xConfig[MAX_SESS_NUM];                                        // the configuration
CRtpConfig m_xConfig[MAX_SESS_NUM];
static int m_nFrameLen[MAX_SESS_NUM];                                                       // the length of frames
static int m_nFrameNum[MAX_SESS_NUM];                                                       // the num of frames
static int m_nFramePerPacket[MAX_SESS_NUM];                                 // the max num of frames packed into a packet
/*static int m_nSIDFrameLen[MAX_SESS_NUM];  */                                      // the length of SID frame
static BOOL m_bSilenceState[MAX_SESS_NUM];                                  // true if in silence state
static char m_aFrameBuffer[MAX_SESS_NUM][512];                              // the buffer to put frames

// for playing tone while receiving 2833 packet
//static BOOL m_bPlayTone[MAX_SESS_NUM];                                            // the flag indicates playing tone
BOOL m_bPlayTone[MAX_SESS_NUM];                                             // hc$ 1201 as global
int check_2833_stop[MAX_SESS_NUM] = {0};
int play_2833_time[MAX_SESS_NUM]={0};
unsigned short play_2833_time_cnt[MAX_SESS_NUM]={0};						// must be unsigned short.
int play_2833_timeout_cnt[MAX_SESS_NUM]={0};
int get_2833_edge[MAX_SESS_NUM]={0};
static uint32 m_uTimestamp[MAX_SESS_NUM];                                           // the timestamp of the event
//static uint32 m_uTone[MAX_SESS_NUM];                                                      // the event
uint32 m_uTone[MAX_SESS_NUM];                                               // hc$ 1201 as global
static int m_nCount[MAX_SESS_NUM];                                                          // how many packets received
extern void DspcodecWriteSkipSeqNo( uint32 sid, uint32 nSeq );
unsigned char rfc2833_payload_type_local[MAX_SESS_NUM];

static uint32 session;
//
// CRtpTerminal - constructor/destructor
//
//static char m_pBuf[1600];
//unsigned int m_uBufSize;

void CRtpTerminal_Init(uint32 sid, CRtpConfig *pConfig)
{
	const codec_payload_desc_t *pCodecPayloadDesc;

	m_xConfig[sid] = *pConfig;
//	m_uBufSize = 1600;

	m_nFrameLen[sid] = 0;
	m_nFrameNum[sid] = 0;

	pCodecPayloadDesc = GetCodecPayloadDesc( m_xConfig[sid].m_uPktFormat );
	
	if( pCodecPayloadDesc ) {
		m_nFramePerPacket[sid] = _idiv32(m_xConfig[sid].m_nTranFrameRate, 
										 pCodecPayloadDesc ->nTranFrameRate );
		m_nSIDFrameLen[sid] = pCodecPayloadDesc ->nSidTxFrameBytes;
 #ifdef SUPPORT_RTCP
		m_xConfig[sid].m_nPeriod = m_nFramePerPacket[sid] * 
								   pCodecPayloadDesc ->nFramePeriod;
 #endif		
	} else {
		printk("[%s] Unknown frame type %d\n", __FUNCTION__, m_xConfig[sid].m_uPktFormat);
		assert(0);
	}

	m_bSilenceState[sid] = FALSE;
	m_bPlayTone[sid] = FALSE;
	m_nCount[sid] = 0;

	m_xConfig[sid].m_uTRMode = rtp_session_sendrecv;

#ifdef SUPPORT_RTCP
	RtpOpen[sid] = 0;
	RtcpOpen[sid] = 0;

	//sessionState[sid] = rtp_session_sendrecv;
#endif
	RtpRx_setFormat(sid, m_xConfig[sid].m_uPktFormat, m_xConfig[sid].m_nRecvFrameRate);
	RtpTx_setFormat(sid, m_xConfig[sid].m_uPktFormat, m_xConfig[sid].m_nTranFrameRate);
}

//
// CRtpTerminal - public interface
//

/* 
We get the data by the following layout:
 +---------+
 |         |
 |  Data   |
 |         |
 +---------+
 | Seq No. |
 +---------+
 |timestamp|
 +---------+
 */

int32 RtpTerminal_Read(uint32* pchid, uint32* psid, UCHAR *pBuf, int32 nSize)
{
	RtpPacket *p = NULL;
	RtpEventDTMFRFC2833 *eventPayload = NULL;
	int32 nGet = 0, nPos;
	uint32 chid, sid;
	RtpSeqNumber seqNo;


	if(++session >= SESS_NUM)
		session = 0;
	

	assert(pBuf);
	p = Rtp_receive();

	if(p)
	{
		*pchid = p->chid;	/* set receive channel id */
		*psid = p->sid;
		chid = *pchid;
		sid = *psid;
		seqNo = getSequence(p);

		//printk("%d\n", rfc2833_payload_type_local[sid]);
		if(getPayloadType(p) == rfc2833_payload_type_local[sid])
		{
			eventPayload = (RtpEventDTMFRFC2833*)(getPayloadLoc(p));
			
			if(!m_bPlayTone[sid])	/* no playtone */
			{
				if(!eventPayload->edge)	/* not edge */
				{
					if(eventPayload->event > DSPCODEC_TONE_HASHSIGN)
					{
						printk("RtpTerm: Receive event %d illegal\n", eventPayload->event);
					}
					else	/* get timestamp & DTMF digit then playtone */
					{
						m_bPlayTone[sid] = true;
						m_uTimestamp[sid] = getRtpTime(p);
						m_uTone[sid] = eventPayload->event;
						play_2833_time[sid]= eventPayload->duration;
						DspcodecWriteSkipSeqNo(sid, seqNo);
						//printk("Start\n");
						printk("Get 2833 Event:%d\n", eventPayload->event);

					}
				}
			}
			else	/* playing tone */
			{
				if(getRtpTime(p) == m_uTimestamp[sid])
				{
					if(eventPayload->edge)
					{
						get_2833_edge[sid] = 1;
						play_2833_time[sid]= eventPayload->duration;
						DspcodecWriteSkipSeqNo(sid, seqNo);
						//printk("edge\n");
					}
					else
					{
						play_2833_time[sid]= eventPayload->duration;
						DspcodecWriteSkipSeqNo(sid, seqNo);
					}
				}
				else /* get another event, getRtpTime() != m_uTimestamp */
				{
					if(eventPayload->event > DSPCODEC_TONE_HASHSIGN)
					{
						printk("RtpTerm: Receive event %d illegal\n", eventPayload->event);
					}
					else
					{	/* Thlin: Should not go to here*/
						printk("RFC2833 receive error!\n"); 
					}
				}
			}
			/* invalid length to distinguish that it is an event packet */
			nGet = -1;
		}
		else	/* non RFC2833 packet */
		{

				if(nSize >= getPayloadUsage(p) + 8)
					nGet = getPayloadUsage(p);
				else
					assert(0);

				memcpy(pBuf, getPayloadLoc(p), nGet);
				if(nGet & 0x00000003)		/* not align */
					nPos = 4 - (nGet & 0x00000003) + nGet;
				else
					nPos = nGet;
				*((uint32 *)(pBuf + nPos)) = getSequence(p);
				*((uint32 *)(pBuf + nPos + 4)) = getRtpTime(p);
		}
		p->own = OWNBYDSP;
	}

#ifdef SUPPORT_RTCP
	//printk("--- RtcpOpen[%d] = %d --- \n", session, RtcpOpen[session]);
	if(RtcpOpen[session] == 1)	//thlin: if RtcpOpen =1 , do RtpSession_processRTCP, eles NOT.
        {
		RtpSession_processRTCP(session);
        }
#endif
	return nGet;
}

int32 RtpTerminal_Write(uint32 chid, uint32 sid, uchar *pBuf, int32 nSize)
{
	/* no more frame */
	if(nSize == 0)
		return 0;

	/* silence period */
	else if(nSize == -1)	// invalid length to distinguish that it is silence
	{
		RtpTx_addTimestampOfOneFrame(sid);
#if 0	// pkshih: why?
 #ifndef __ECOS
		if(m_xConfig[sid].m_uPktFormat == rtpPayloadG729)
			RtpTx_addTimestamp(sid);
 #endif
#endif
		return 0;
	}

	/* normal frame or SID frame */
	else
	{
		assert(pBuf);
//		ASSERT(pBuf);
		//send_data_in_ppp_format( 2 , "write rtpTerminal" , 17 ) ;		// Howard. 2004.12.8 for debug
		if(nSize == m_nSIDFrameLen[sid])	// SID frame
		{
			if(m_bSilenceState[sid])
			{
				RtpTx_setMarkerOnce(sid);
				RtpTx_transmitRaw(chid, sid, (char*)pBuf, (int)nSize);
				RtpTx_subTimestampIfNecessary( sid, ( char * )pBuf, nSize );
#if 0	// pkshih: why??
 #ifndef __ECOS
				if(m_xConfig[sid].m_uPktFormat == rtpPayloadG729)
					RtpTx_addTimestamp(sid);
 #endif
				if((m_xConfig[sid].m_uPktFormat == rtpPayloadPCMU) || (m_xConfig[sid].m_uPktFormat == rtpPayloadPCMA))
					RtpTx_addTimestamp(sid);
#endif
			}
			else
			{
				if(m_nFrameLen[sid] != 0)
				{
					RtpTx_subTimestamp(sid);
					RtpTx_transmitRaw(chid, sid, m_aFrameBuffer[sid], m_nFrameLen[sid]);
					RtpTx_subTimestampIfNecessary( sid, m_aFrameBuffer[sid], m_nFrameLen[sid] );
					m_nFrameLen[sid] = 0;
					m_nFrameNum[sid] = 0;
				}
				RtpTx_transmitRaw(chid, sid, (char*)pBuf, (int)nSize);
				RtpTx_subTimestampIfNecessary( sid, ( char * )pBuf, nSize );
#if 0	// pkshih: why??
 #ifndef __ECOS
				if(m_xConfig[sid].m_uPktFormat == rtpPayloadG729)
					RtpTx_addTimestamp(sid);
 #endif
				if((m_xConfig[sid].m_uPktFormat == rtpPayloadPCMU) || (m_xConfig[sid].m_uPktFormat == rtpPayloadPCMA))
					RtpTx_addTimestamp(sid);
#endif
				m_bSilenceState[sid] = true;
			}
		}
		else	// normal frame
		{
			if(m_bSilenceState[sid])
			{
				RtpTx_setMarkerOnce(sid);
				m_bSilenceState[sid] = false;
			}
			memcpy(m_aFrameBuffer[sid]+m_nFrameLen[sid], pBuf, nSize);
			m_nFrameLen[sid] += nSize;
			m_nFrameNum[sid]++;
			if(m_nFrameNum[sid] == m_nFramePerPacket[sid])
			{
				RtpTx_transmitRaw(chid, sid, m_aFrameBuffer[sid], m_nFrameLen[sid]);
				RtpTx_subTimestampIfNecessary( sid, m_aFrameBuffer[sid], m_nFrameLen[sid] );
				m_nFrameLen[sid] = 0;
				m_nFrameNum[sid] = 0;
			}
			else
				RtpTx_addTimestamp(sid);
		}

		return nSize;
	}
}

void RtpTerminal_setFormat(uint32 sid, RtpPayloadType type, int recvRate, int tranRate)
{
	RtpTx_setFormat(sid, type, tranRate);
	RtpRx_setFormat(sid, type, tranRate);
}

/* The very function that used to config endpoint */
void RtpTerminal_SetConfig(uint32 chid, uint32 sid, void *pConfig)
{
	bool rtpConfig_changed = false;
	bool rtcpConfig_changed = false;
	CRtpConfig* pRtpConfig = NULL;
	const codec_payload_desc_t *pCodecPayloadDesc;

	assert(pConfig);
	pRtpConfig = (CRtpConfig *)pConfig;

#if 0
//#ifdef SUPPORT_RTCP

	/* RTCP part */
	if(m_xConfig[sid].m_nRtcpRemotePort != pRtpConfig->m_nRtcpRemotePort)
	{
		rtcpConfig_changed = true;
		m_xConfig[sid].m_nRtcpRemotePort = pRtpConfig->m_nRtcpRemotePort;
	}
	if(m_xConfig[sid].m_nRtcpLocalPort != pRtpConfig->m_nRtcpLocalPort)
	{
		rtcpConfig_changed = true;
		m_xConfig[sid].m_nRtcpLocalPort = pRtpConfig->m_nRtcpLocalPort;
	}

if(rtcpConfig_changed==true)
	{
		if(RtpOpen[sid])	/* if Rtp is open, then can open and close Rtcp session*/
		{
			if((pRtpConfig->m_nRtcpRemotePort == 0) || (pRtpConfig->m_nRtcpLocalPort == 0))
			{
				/*
				   As our rule, we close the rtcp session,
				   when the Rtcp remote port or local port is zero.
				*/
				if(RtcpOpen[sid])
				{
					RtcpTx_transmitRTCPBYE(sid);
					rtcp_Session_UnConfig(chid, sid);
					RtcpOpen[sid] = 0;
				}
			}

		}
		else
		{
			/*
			   If rtp session is already closed,
			   we only need to close the flag and do not need to close rtcp session.
			   The rtcp session is already closed when rtp_session_inactive.
			*/
			if(RtcpOpen[sid])
				if((pRtpConfig->m_nRtcpRemotePort == 0) || (pRtpConfig->m_nRtcpLocalPort == 0))
					RtcpOpen[sid] = 0;
			return;
		}
	}
	/*-------------------------------------------------------------------------*/

	if (pRtpConfig->m_uTRMode == rtp_session_inactive)/*Wish to stop RTP transfer*/
	{
		if(RtpOpen[sid])
		{

			if(RtcpOpen[sid])
			{
				RtcpTx_transmitRTCPBYE(sid);
				rtcp_Session_UnConfig(chid, sid);
			}

			//rtp_Session_UnConfig(chid, sid);	/* clear voip port register by ROME driver function */


			RtpOpen[sid] = 0;
		}
		m_xConfig[sid].m_uTRMode = pRtpConfig->m_uTRMode;
		RtpSession_setSessionState(sid, m_xConfig[sid].m_uTRMode);
		printk("\nRTP session stop on ch:%d system session:%d\n", chid, sid);
		DSP_CodecStop(chid, sid);
		return;
	}

	
#endif //by bruce
	m_xConfig[sid].m_uPktFormat = pRtpConfig->m_uPktFormat;
	m_xConfig[sid].m_nRecvFrameRate = pRtpConfig->m_nRecvFrameRate;
	m_xConfig[sid].m_nTranFrameRate = pRtpConfig->m_nTranFrameRate;
	m_xConfig[sid].m_uTRMode = pRtpConfig->m_uTRMode;
	RtpTerminal_setFormat(sid, m_xConfig[sid].m_uPktFormat, m_xConfig[sid].m_nRecvFrameRate, m_xConfig[sid].m_nTranFrameRate);
	RtpSession_setSessionState(sid, m_xConfig[sid].m_uTRMode);

	m_nFrameLen[sid] = 0;
	m_nFrameNum[sid] = 0;

	pCodecPayloadDesc = GetCodecPayloadDesc( m_xConfig[sid].m_uPktFormat );
	
	if( pCodecPayloadDesc ) {
		m_nFramePerPacket[sid] = _idiv32(m_xConfig[sid].m_nTranFrameRate, 
										 pCodecPayloadDesc ->nTranFrameRate );
		m_nSIDFrameLen[sid] = pCodecPayloadDesc ->nSidTxFrameBytes;
	} else {
		printk("[%s] Unknown frame type %d\n", __FUNCTION__, m_xConfig[sid].m_uPktFormat);
		assert(0);
	}

	m_bSilenceState[sid] = false;
	m_bPlayTone[sid] = false;
	m_nCount[sid] = 0;

	RtpRx_setFormat(sid, m_xConfig[sid].m_uPktFormat, m_xConfig[sid].m_nRecvFrameRate);
	RtpTx_setFormat(sid, m_xConfig[sid].m_uPktFormat, m_xConfig[sid].m_nTranFrameRate);
#if 0
	if (pRtpConfig->m_uTRMode == rtp_session_inactive)
	{
		//rtp_Session_UnConfig(chid, sid);	// clear voip port register
//		chanInfo_CloseSessionID(chid, sid);

		m_xConfig[sid].m_uTRMode = pRtpConfig->m_uTRMode;
		RtpSession_setSessionState(sid, m_xConfig[sid].m_uTRMode);

		m_xConfig[sid].m_nRtpRemoteIP = 0;
		m_xConfig[sid].m_nRtpRemotePort = 0;

		return;
	}
	
#endif
	if (pRtpConfig->m_uTRMode == rtp_session_inactive)
	{
		m_xConfig[sid].m_nRtpRemoteIP = 0;
		m_xConfig[sid].m_nRtpRemotePort = 0;
	}
}

/*+++++ add by Jack for set session state+++++*/
void RtpTerminal_SetSessionState(uint32 sid, uint32 state){
	m_xConfig[sid].m_uTRMode = state;
	RtpSession_setSessionState(sid, m_xConfig[sid].m_uTRMode);

}
/*-----end-----*/
void RtpTerminal_GetConfig(uint32 sid, void *pConfig)
{
	assert(pConfig);
	*((CRtpConfig *)pConfig) = m_xConfig[sid];
}

#ifdef SUPPORT_RFC_2833
unsigned char RtpTerminal_SendDTMFEvent(uint32 chid, uint32 sid, int32 nEvent, int duration)

{
	return RtpTx_transmitEvent(chid, sid, nEvent, duration);
}
#endif

#if 0
void RtpTerminal_TransmitData(void)
{
	// transmit data from source terminals to destination terminals
	// note:
	// the data multiplexing is done in target (sink) terminals rather than in stream
	//
	uint32 chid;
	int nRealSize;
	while (1)
	{
		nRealSize = RtpTerminal_Read(&chid, m_pBuf, m_uBufSize);
		DSP_Write(chid, m_pBuf, nRealSize);
		if(nRealSize == 0)
			break;
	}

	while(1)
	{
		nRealSize = DSP_Read(&chid, m_pBuf, m_uBufSize);
		RtpTerminal_Write(chid, m_pBuf, nRealSize);
		if(nRealSize == 0)
			break;
	}
}
#endif

/////////////////////////////////////////////////////////////////////////
// for RTP session
void RtpSession_setSessionState (uint32 sid, RtpSessionState state)
{
#ifdef SUPPORT_RTCP
	sessionState[sid] = state;
#endif
	switch(state)
	{
		case rtp_session_inactive:
			printk("+++++Debug:go to rtp_session_inactive %d+++++\n",sid);
			RtpRx_setMode(sid, rtprecv_droppacket);
			RtpTx_setMode(sid, rtptran_droppacket);
			break;
		case rtp_session_sendonly:
			printk("+++++Debug:go to rtp_session_sendonly %d+++++\n",sid);
			RtpRx_setMode(sid, rtprecv_droppacket);
			RtpTx_setMode(sid, rtptran_normal);
			break;
		case rtp_session_recvonly:
			printk("+++++Debug:go to rtp_session_recvonly %d+++++\n",sid);
			RtpRx_setMode(sid, rtprecv_normal);
			RtpTx_setMode(sid, rtptran_droppacket);
			break;
		case rtp_session_sendrecv:
			printk("+++++Debug:go to rtp_session_sendrecv %d+++++\n",sid);
			RtpRx_setMode(sid, rtprecv_normal);
			RtpTx_setMode(sid, rtptran_normal);
			break;
		default:
			assert(0);
			break;
	}
}

void RtpSession_renew(uint32 sid)
{
	RtpTx_renewSession(sid);
	/*
	 * pkshih uncomment following. 
	 * Because sourceSet[] is set, updateSource() will try to change 
	 * source and drop packets in begining of session. 
	 * We guess that it affects voice quality only if VAD is on, 
	 * because voice frames are almost silence in begining.
	 * In other words, drop these silence frames will not affect 
	 * voice quality.
	 *
	 * WARNING: I don't know why someone comment it!!
	 */
	RtpRx_renewSession(sid);

#ifdef SUPPORT_RTCP
	if( sid >= RTCP_SID_OFFSET ) {
		sid -= RTCP_SID_OFFSET;
		
		RtcpTx_InitByID( sid );
		RtcpRx_InitByID( sid );
	}
#endif
}

#ifdef SUPPORT_RTCP
void RtpSession_processRTCP (uint32 sid)
{

	if (RtcpTx_checkIntervalRTCP(sid))
	{
		RtcpTx_transmitRTCP(sid);
		//printk("T%d\n", sid);
	}

	RtcpRx_receiveRTCP(sid);

    return ;
}
#endif
