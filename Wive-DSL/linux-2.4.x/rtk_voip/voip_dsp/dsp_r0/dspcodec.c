
//
// dspcodec.cpp
//

#include <linux/string.h>
#include <net/ip.h>
//#include "rtk_voip.h"
#include "dspcodec_0.h"
#include "dspparam.h"
#include "assert.h"
#include "codec_descriptor.h"

//for change country and still play tone.
#include "typedef.h"
extern Flag fPlayTone[][CH_TONE];
#include "rtk_voip.h"//for change country and still play tone.
extern Word16  WhichTone[][CH_TONE];//for change country and still play tone.

/* extern variable */
extern CDspcodecResponse RECV_ADDR[];
extern CDspcodecParm SEND_ADDR[];
extern int sess_num;


/* global variable */
#ifdef SUPPORT_DYNAMIC_PAYLOAD
int DPSize[MAX_SESS_NUM] = {1};

#ifdef DYNAMIC_PAYLOAD_VER1
int DPInitSeq[MAX_SESS_NUM] = {0};
int DPInitStamp[MAX_SESS_NUM] = {0};
#else
uint16 DPInSeq[MAX_SESS_NUM] = {0};
uint16 DPOutSeq[MAX_SESS_NUM] = {0};
uint16 DPExpSeq[MAX_SESS_NUM] = {0};
uint32 DPExpTimeStamp[MAX_SESS_NUM] = {0};
uint16 DPNotFirstPacket[MAX_SESS_NUM] = {0};
 #ifdef SUPPORT_SYNC_OUT_OF_SEQ
uint16 DPEnableSyncInSeq[MAX_SESS_NUM] = {0};
uint16 DPSyncInSeq[MAX_SESS_NUM] = {0};
 #endif
#endif

#endif	// SUPPORT_DYNAMIC_PAYLOAD

#ifdef SUPPORT_DETECT_LONG_TERM_NO_RTP
extern uint32 RTP_RxPacketCount[MAX_SESS_NUM];
extern uint32 RTP_RxPacketCountPrev[MAX_SESS_NUM];
extern uint32 RTP_RxPacketWaitCount[MAX_SESS_NUM];
#endif

// declarations of constants and static data
//
/*	handsome add function 2005.12.1     */
int iCountry = DSPCODEC_COUNTRY_USA;
int dialTone = 0;
int ringTone = 1;
int busyTone = 2;
int waitingTone = 3;
uint32 cust = 0;

// writing states
typedef enum
{
	DSPCODEC_WSTATE_NORMALJITTER,
	DSPCODEC_WSTATE_SYNCJITTER
} DSPCODEC_WSTATE;

#define RTP_SEQ_MOD		(1 << 16)

#ifdef RESERVE_SPACE_FOR_SLOW
enum {	// for nOwner
	R0_OWN = 0,			// ready for R1 
	R1_OWN = 1,			// R0 can place new data
	R0_RESERVE = 2,		// reserved for slow packet (out-of-order)
};
#endif

/* static variable */
static bool bCreated[MAX_SESS_NUM] = {false};                                       // can create only once

static int over_load_cnt[MAX_SESS_NUM]={0}, outof_seq_cnt[MAX_SESS_NUM]={0};    // sandro move from DspcodecWrite()
#ifdef SUPPORT_SYNC_OUT_OF_SEQ
static uint16 outof_seq_prevSeq[MAX_SESS_NUM] = { 0 };
#endif

//WJF 930806 added, when reached tx_jit_buf_high_threshold, touch_high_threshold will be 1, 
// touch_high_threshold will be 0 when down to tx_jit_buf_low_threshold

#if defined( SUPPORT_ADJUST_JITTER ) && !defined( RESERVE_SPACE_FOR_SLOW )
extern int jbc_max_delay[MAX_SESS_NUM];
extern int jbc_target_delay[MAX_SESS_NUM];
#endif
int tx_jit_buf_low_threshold[MAX_SESS_NUM]={10};
int tx_jit_buf_high_threshold[MAX_SESS_NUM]={30};

int touch_high_threshold[MAX_SESS_NUM]={0};

const codec_algo_desc_t *ppNowCodecAlgorithmDesc[MAX_SESS_NUM];

uint32 nRxRtpStatsLostPacket[MAX_SLIC_CH_NUM];

//static volatile CDspcodecParm* pSendParm;		// the jitter buffer of sending data to RISC1
//static volatile CDspcodecResponse* pRecvParm;	// the jitter buffer of receiving data from 
#define pSendParm	SEND_ADDR
#define pRecvParm	RECV_ADDR

typedef struct
{
	DSPCODEC_ALGORITHM	m_uCodingAlgorithm;  	// which coding algorithm used: G711u, G711a, G723.1a53, G723.1a63, G729
	CDspcodecConfig		m_xConfig;				// remember DSP configuration
	DSPCODEC_INTERFACE	m_nInterface;			// interface to play
	int32				m_nRingType;			// which ring to play

#ifdef SUPPORT_TONE_PROFILE
	int32				m_nBaseTone;			// index of base tone ( dial tone)
#else
	int32				m_nDialTone;			// index of dial tone
	int32				m_nRingTone;			// index of ring tone
	int32				m_nBusyTone;			// index of busy tone
	int32				m_nWaitingTone;			// index of waiting tone
#endif

	// data members for writing and reading
	uint32				m_nPosDec;				// the position to put data to decoding buffer
	uint32				m_nPosEnc;				// the position to get encoded data
	uint32				m_nPreSeq;				// the sequence no. of the last packet put
	bool					m_bSeqGet;				// m_nPreSeq is got or not
	uint32				m_nPreTick;				// the running tick of RISC1 of the last packet put
	uint32				m_nPreTimeStamp;		// the timestamp of the last packet put
	DSPCODEC_WSTATE	m_nWState;				// the writing state
	int32				m_nInitTd;				// the initial Td (vary according to codec)
	int32				m_nPreTd;				// the time delayed of the last packet put
	int32				m_nAveTd;				// the average Td
	int32				m_nTdOffset;			// the offset of Td
	uint32				m_njSN;					// the sequence no. of the frame to sync jitter
	// statistics for monitoring or debugging
	uint32				m_nFrameSent;			// the number of frame written in buffer
	uint32				m_nEarlyFrame;			// the number of dropped early frame
	uint32				m_nLateFrame;			// the number of dropped late frame
	uint32				m_nNotOwner;			// owner bit isn't mine, cannot write
	uint32				m_nDuplicate;			// the frame is duplicated
	uint32				m_nFrameRecv;			// the number of frame got from buffer
	uint32				m_nIntNum;				// the number RISC1 has sent interrupts
	uint32				m_nR1ResetNum;			// the number of reseting RISC1
} DspcodecData;
static DspcodecData Dsp_data[MAX_SESS_NUM];

#ifdef RESERVE_SPACE_FOR_SLOW
extern const uint32 * const p_DspDecWrite[];
const uint32 * const p_DspDecWrite[MAX_SESS_NUM] = {
 #if SESS_NUM == 2
	&Dsp_data[ 0 ].m_nPosDec, 
	&Dsp_data[ 1 ].m_nPosDec, 
 #elif SESS_NUM == 4
	&Dsp_data[ 0 ].m_nPosDec, 
	&Dsp_data[ 1 ].m_nPosDec, 
	&Dsp_data[ 2 ].m_nPosDec, 
	&Dsp_data[ 3 ].m_nPosDec, 
 #else
  #error "Not init DSP decode writing pointer."
 #endif
};
#endif

#define debug_warning		//printk	/* warning level debug */
#define debug_error			printk		/* error level debug */
#define debug_dev			//printk	/* development debug */
#define debug_cd			printk		/* codec descriptor */

//
// static functions
//
// compare the sequence numbers
static bool RtpSeqGreater(uint16 a, uint16 b)
{
	if((a > (RTP_SEQ_MOD - 20000) && b < 20000) || (a < 20000 && b > (RTP_SEQ_MOD - 20000)))
		return (a < b);
	else
		return (a > b);
}

// send command to RISC1 and wait for acknowledgement
static RESULT SendCommand(uint32 sid)
{
	extern void CmdParser(uint32 sid);

	RESULT retval = DSPCODEC_ERROR_RESPONSE;

	pRecvParm[sid].nOwner = 0;

	CmdParser(sid);

	switch(pSendParm[sid].uCommand)
	{
	case DSPCODEC_COMMAND_INITIALIZE:
		if(pRecvParm[sid].uAck == DSPCODEC_ACK_INITIALIZE_DONE)
			retval = DSPCODEC_SUCCESS;
		break;
	case DSPCODEC_COMMAND_SETCONFIG:
		if(pRecvParm[sid].uAck == DSPCODEC_ACK_SETCONFIG_DONE)
			retval = DSPCODEC_SUCCESS;
		break;
	case DSPCODEC_COMMAND_START:
		if(pRecvParm[sid].uAck == DSPCODEC_ACK_START_DONE)
			retval = DSPCODEC_SUCCESS;
		break;
	case DSPCODEC_COMMAND_STARTMIXING:
		if(pRecvParm[sid].uAck == DSPCODEC_ACK_STARTMIXING_DONE)
			retval = DSPCODEC_SUCCESS;
		break;
	case DSPCODEC_COMMAND_STOP:
		if(pRecvParm[sid].uAck == DSPCODEC_ACK_STOP_DONE)
			retval = DSPCODEC_SUCCESS;
		break;
	case DSPCODEC_COMMAND_MUTE:
		if(pRecvParm[sid].uAck == DSPCODEC_ACK_MUTE_DONE)
			retval = DSPCODEC_SUCCESS;
		break;
	case DSPCODEC_COMMAND_PLAYTONE:
		if(pRecvParm[sid].uAck == DSPCODEC_ACK_PLAYTONE_DONE)
			retval = DSPCODEC_SUCCESS;
		break;
	case DSPCODEC_COMMAND_JITTERSYNC:
		if(pRecvParm[sid].uAck == DSPCODEC_ACK_JITTERSYNC_DONE)
			retval = DSPCODEC_SUCCESS;
		break;
	case DSPCODEC_COMMAND_GETVERSION:
		if(pRecvParm[sid].uAck == DSPCODEC_ACK_GETVERSION_DONE)
			retval = DSPCODEC_SUCCESS;
		break;
	case DSPCODEC_COMMAND_DEBUG:
		if(pRecvParm[sid].uAck == DSPCODEC_ACK_DEBUG)
			retval = DSPCODEC_SUCCESS;
		break;
	case DSPCODEC_COMMAND_ALIVE:
		if(pRecvParm[sid].uAck == DSPCODEC_ACK_ALIVE)
			retval = DSPCODEC_SUCCESS;
		break;
	default:
		retval = DSPCODEC_ERROR_RESPONSE;
		break;
	}

	pRecvParm[sid].nOwner = 0;
	return retval;

}

// send JITTERSYNC command to RISC1
static RESULT SyncJitterBuf(uint32 sid, uint32 njSN, int32 nTdOffset)
{
	RESULT retval;

	memset((void *)(&(pSendParm[sid].xParm)), 0, sizeof(CDspcodecJitterSyncParm));
	pSendParm[sid].uCommand = DSPCODEC_COMMAND_JITTERSYNC;
	pSendParm[sid].nOwner = 0;
	pSendParm[sid].xParm.xJitterSyncParm.njSN = njSN;
	pSendParm[sid].xParm.xJitterSyncParm.nTdOffset = nTdOffset;

	retval = SendCommand(sid);

	if(retval != DSPCODEC_SUCCESS)
	{
//		printf("Dspcodec: !!! Send command [JITTERSYNC] fail !!!\n");
	}
	return retval;
}

//
// public functions
//

#ifdef SUPPORT_DYNAMIC_PAYLOAD
void DynamicPayloadInit( uint32 sid )
{
	DPSize[sid] = 1;

 #ifdef DYNAMIC_PAYLOAD_VER1 //tyhuang:111
	DPInitSeq[sid] = 0;
	DPInitStamp[sid] = 0;
 #else
	DPInSeq[sid] = 0;
	DPOutSeq[sid] = 0;
	DPExpSeq[sid] = 0;
	DPExpTimeStamp[sid] = 0;
	DPNotFirstPacket[sid] = 0;
  #ifdef SUPPORT_SYNC_OUT_OF_SEQ
	DPSyncInSeq[sid] = 0;
	DPEnableSyncInSeq[sid] = 0;
  #endif
 #endif	
}
#endif /* SUPPORT_DYNAMIC_PAYLOAD */

void DspcodecInitVar(void)
{
	uint32 sid;
	for(sid=0; sid<sess_num; sid++)
	{

#ifdef SUPPORT_DYNAMIC_PAYLOAD
		DynamicPayloadInit( sid );
#endif

#ifdef SUPPORT_DETECT_LONG_TERM_NO_RTP
		RTP_RxPacketCount[ sid ] = 0;
		RTP_RxPacketCountPrev[ sid ] = 0;
		RTP_RxPacketWaitCount[ sid ] = 0;
#endif

		bCreated[sid] = false;					// can create only once

#ifdef SUPPORT_ADJUST_JITTER
 #ifdef RESERVE_SPACE_FOR_SLOW
		tx_jit_buf_high_threshold[sid] = ( BUFFER_NUM_DEC - 1 ) - 5;
		tx_jit_buf_low_threshold[sid] = ( BUFFER_NUM_DEC - 1 ) - 10;
 #else
		tx_jit_buf_high_threshold[sid] = jbc_max_delay[sid] + 5;	/* set high threshold equal max delay add 5*/
		tx_jit_buf_low_threshold[sid] = jbc_target_delay[sid];
 #endif
#else
		tx_jit_buf_low_threshold[sid]=10;
		tx_jit_buf_high_threshold[sid]=30;
#endif
		touch_high_threshold[sid]=0;

		over_load_cnt[sid]=0;
		outof_seq_cnt[sid]=0;
	}

}

void DspcodecUp(uint32 sid) /* Called in DSP_init , DSP_CodecRestart*/
{
	int i;

#if 0
	if(bCreated[sid])
	{
		printk("Session %d is already DspcodecUp!\n",sid);
		assert(0);
	}
	bCreated[sid] = true;
#endif

	//pSendParm = (CDspcodecParm *)SEND_ADDR;
	//pRecvParm = (CDspcodecResponse *)RECV_ADDR;

	/* initialize member data	config default value
	 */
	Dsp_data[sid].m_xConfig.pToneTable 	= (char*)ToneTable;
	Dsp_data[sid].m_xConfig.pCtrlParm 		= (char*)&CtrlParam;
	Dsp_data[sid].m_xConfig.bVAD 			= false;
	Dsp_data[sid].m_xConfig.bAES 			= false;
	Dsp_data[sid].m_xConfig.bPLC 			= false;
	Dsp_data[sid].m_xConfig.nVadLevel 		= 0;
	Dsp_data[sid].m_xConfig.nHangoverTime 	= 70;
	Dsp_data[sid].m_xConfig.nBGNoiseLevel 	= -30;
	Dsp_data[sid].m_xConfig.nAttenRange 	= -30;
	Dsp_data[sid].m_xConfig.nTXGain 		= 0;
	Dsp_data[sid].m_xConfig.nRXGain 		= 0;
	Dsp_data[sid].m_xConfig.nTRRatio 		= 0;
	Dsp_data[sid].m_xConfig.nRTRatio 		= 0;
	/* other data members */
	Dsp_data[sid].m_nInterface 	= DSPCODEC_INTERFACE_HANDSET;
	Dsp_data[sid].m_nRingType 	= 0;

	/*	handsome add function 2005.12.1     */
	DspcodecSetCountry(sid, iCountry);
	Dsp_data[sid].m_nIntNum 		= 0;
	Dsp_data[sid].m_nR1ResetNum 	= 0;

	// clear the owner bits of jitter buffer and receiving buffer
	for(i=0; i<BUFFER_NUM_DEC; i++)/*BUFFER_NUM_DEC = 256*/
		pSendParm[sid].xDecBuffer[i].nOwner = 1;
	for(i=0; i<BUFFER_NUM_ENC; i++)
		pRecvParm[sid].xEncOutput[i].nOwner = 0;
	pSendParm[sid].nOwner = 0;
	pSendParm[sid].nSeqNo = 0;
	pSendParm[sid].nFramtPut = 0;
	pRecvParm[sid].nOwner = 0;
	pRecvParm[sid].nSeqNo = 0;

	// load program of RISC1 to memory and turn on RISC1
	DspcodecResetR1(sid);
}

void DspcodecDown(void)
{
}

RESULT DspcodecInitialize(uint32 sid, DSPCODEC_ALGORITHM uCodingAlgorithm)
{
	int i;
	RESULT retval;

#ifdef Doing_723_decode_play_test
	uCodingAlgorithm = DSPCODEC_ALGORITHM_G711U ;
#endif

	if( ( ppNowCodecAlgorithmDesc[ sid ] = GetCodecAlgoDesc( uCodingAlgorithm ) ) 
		== NULL )
	{
		printk("Dspcodec: !!! Wrong parameter of uCodingAlgorithm %d !!!\n", uCodingAlgorithm);
		assert(0);
		return DSPCODEC_ERROR_ALGORITHM;
	}

	// clear the owner bits of jitter buffer and receiving buffer
	for(i=0; i<BUFFER_NUM_DEC; i++)
	{
		pSendParm[sid].xDecBuffer[i].nOwner = 1;/* Like a passive sense*/
		pSendParm[sid].xDecBuffer[i].nSeqNo = 0;
	}
	for(i=0; i<BUFFER_NUM_ENC; i++)
		pRecvParm[sid].xEncOutput[i].nOwner = 0;/* A active sense */
	pSendParm[sid].nOwner = 0;
	pSendParm[sid].nSeqNo = 0;
	pSendParm[sid].nFramtPut = 0;
	pRecvParm[sid].nOwner = 0;
	pRecvParm[sid].nSeqNo = 0;
	pRecvParm[sid].bPutNew = 0;

	memset((void *)(&(pSendParm[sid].xParm)), 0, sizeof(CDspcodecInitializeParm));
	Dsp_data[sid].m_uCodingAlgorithm = uCodingAlgorithm;
	pSendParm[sid].uCommand = DSPCODEC_COMMAND_INITIALIZE;
	pSendParm[sid].nOwner = 0;
	pSendParm[sid].xParm.xInitializeParm.uCodingAlgorithm 		= Dsp_data[sid].m_uCodingAlgorithm;
	pSendParm[sid].xParm.xInitializeParm.xConfig.pToneTable 	= Dsp_data[sid].m_xConfig.pToneTable;
	pSendParm[sid].xParm.xInitializeParm.xConfig.pCtrlParm 		= Dsp_data[sid].m_xConfig.pCtrlParm;
	pSendParm[sid].xParm.xInitializeParm.xConfig.bVAD 		= Dsp_data[sid].m_xConfig.bVAD;
	pSendParm[sid].xParm.xInitializeParm.xConfig.nVadLevel 		= Dsp_data[sid].m_xConfig.nVadLevel;
	pSendParm[sid].xParm.xInitializeParm.xConfig.bAES 		= Dsp_data[sid].m_xConfig.bAES;
	pSendParm[sid].xParm.xInitializeParm.xConfig.nHangoverTime 	= Dsp_data[sid].m_xConfig.nHangoverTime;
	pSendParm[sid].xParm.xInitializeParm.xConfig.nBGNoiseLevel 	= Dsp_data[sid].m_xConfig.nBGNoiseLevel;
	pSendParm[sid].xParm.xInitializeParm.xConfig.nAttenRange 	= Dsp_data[sid].m_xConfig.nAttenRange;
	pSendParm[sid].xParm.xInitializeParm.xConfig.nTXGain 		= Dsp_data[sid].m_xConfig.nTXGain;
	pSendParm[sid].xParm.xInitializeParm.xConfig.nRXGain 		= Dsp_data[sid].m_xConfig.nRXGain;
	pSendParm[sid].xParm.xInitializeParm.xConfig.nTRRatio 		= Dsp_data[sid].m_xConfig.nTRRatio;
	pSendParm[sid].xParm.xInitializeParm.xConfig.nRTRatio 		= Dsp_data[sid].m_xConfig.nRTRatio;
	pSendParm[sid].xParm.xInitializeParm.xConfig.bPLC 		= Dsp_data[sid].m_xConfig.bPLC;
	pSendParm[sid].xParm.xInitializeParm.xConfig.nJitterDelay	= Dsp_data[sid].m_xConfig.nJitterDelay;
	pSendParm[sid].xParm.xInitializeParm.xConfig.nMaxDelay		= Dsp_data[sid].m_xConfig.nMaxDelay;
	pSendParm[sid].xParm.xInitializeParm.xConfig.nJitterFactor	= Dsp_data[sid].m_xConfig.nJitterFactor;

	retval = SendCommand(sid);

	// init some data members for writing and reading
	Dsp_data[sid].m_nPosDec = 0;
	Dsp_data[sid].m_nPosEnc = 0;
	Dsp_data[sid].m_nPreSeq = 0;
	Dsp_data[sid].m_bSeqGet = false;
	Dsp_data[sid].m_nPreTick = 0;
	Dsp_data[sid].m_nPreTimeStamp = 0;
	Dsp_data[sid].m_nWState = DSPCODEC_WSTATE_NORMALJITTER;
#ifdef CONFIG_RTK_VOIP_G7231
	if((Dsp_data[sid].m_uCodingAlgorithm == DSPCODEC_ALGORITHM_G7231A53) ||
		(Dsp_data[sid].m_uCodingAlgorithm == DSPCODEC_ALGORITHM_G7231A63))
	{
		Dsp_data[sid].m_nInitTd = G723PRETD;

#ifndef SUPPORT_ADJUST_JITTER
		tx_jit_buf_high_threshold[sid] = 30 ;
		tx_jit_buf_low_threshold[sid] = 10 ;
#endif
	}
	else 
#endif /* CONFIG_RTK_VOIP_G7231 */
#ifdef CONFIG_RTK_VOIP_G729AB
	if(Dsp_data[sid].m_uCodingAlgorithm == DSPCODEC_ALGORITHM_G729)
	{
		Dsp_data[sid].m_nInitTd = G729PRETD;

#ifndef SUPPORT_ADJUST_JITTER
		tx_jit_buf_high_threshold[sid] = 40 ;
		tx_jit_buf_low_threshold[sid] = 10 ;
#endif
	}
	else
#endif /* CONFIG_RTK_VOIP_G729AB */
	{
		Dsp_data[sid].m_nInitTd = G711PRETD;

#ifndef SUPPORT_ADJUST_JITTER
		tx_jit_buf_high_threshold[sid] = 40 ;
		tx_jit_buf_low_threshold[sid] = 10 ;
#endif
	}
	Dsp_data[sid].m_nPreTd = Dsp_data[sid].m_nInitTd;
	Dsp_data[sid].m_nAveTd = Dsp_data[sid].m_nInitTd;
	Dsp_data[sid].m_nTdOffset = 0;
	Dsp_data[sid].m_njSN = 0;
	Dsp_data[sid].m_nFrameSent = 0;
	Dsp_data[sid].m_nEarlyFrame = 0;
	Dsp_data[sid].m_nLateFrame = 0;
	Dsp_data[sid].m_nNotOwner = 0;
	Dsp_data[sid].m_nDuplicate = 0;
	Dsp_data[sid].m_nFrameRecv = 0;

#ifdef SUPPORT_DYNAMIC_PAYLOAD
	DynamicPayloadInit( sid );
#endif

#ifdef SUPPORT_DETECT_LONG_TERM_NO_RTP
	RTP_RxPacketCount[ sid ] = 0;
	RTP_RxPacketCountPrev[ sid ] = 0;
	RTP_RxPacketWaitCount[ sid ] = 0;
#endif

	if(retval != DSPCODEC_SUCCESS)
	{
	}
	return retval;
}

RESULT DspcodecSetConfig(uint32 sid, CDspcodecConfig* pConfig)
{
	RESULT retval;

	assert(pConfig);

	memset((void *)(&(pSendParm[sid].xParm)), 0, sizeof(CDspcodecSetConfigParm));
	Dsp_data[sid].m_xConfig = *pConfig;
	pSendParm[sid].uCommand = DSPCODEC_COMMAND_SETCONFIG;
	pSendParm[sid].nOwner = 0;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.pToneTable 	= Dsp_data[sid].m_xConfig.pToneTable;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.pCtrlParm 	= Dsp_data[sid].m_xConfig.pCtrlParm;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.bVAD 	= Dsp_data[sid].m_xConfig.bVAD;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.nVadLevel 	= Dsp_data[sid].m_xConfig.nVadLevel;
/*	pSendParm[sid].xParm.xSetConfigParm.xConfig.bAES 	= Dsp_data[sid].m_xConfig.bAES;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.nHangoverTime = Dsp_data[sid].m_xConfig.nHangoverTime;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.nBGNoiseLevel = Dsp_data[sid].m_xConfig.nBGNoiseLevel;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.nAttenRange = Dsp_data[sid].m_xConfig.nAttenRange;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.nTXGain 	= Dsp_data[sid].m_xConfig.nTXGain;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.nRXGain 	= Dsp_data[sid].m_xConfig.nRXGain;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.nTRRatio 	= Dsp_data[sid].m_xConfig.nTRRatio;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.nRTRatio 	= Dsp_data[sid].m_xConfig.nRTRatio;
*/	pSendParm[sid].xParm.xSetConfigParm.xConfig.bPLC 	= Dsp_data[sid].m_xConfig.bPLC;

	retval = SendCommand(sid);

	if(retval != DSPCODEC_SUCCESS)
	{
//		printf("Dspcodec: !!! Send command [SETCONFIG] fail !!!\n");
	}
	return retval;
}

RESULT DspcodecGetConfig(uint32 sid, CDspcodecConfig* pConfig)
{
	assert(pConfig);

	pConfig->pToneTable		= Dsp_data[sid].m_xConfig.pToneTable;
	pConfig->pCtrlParm 		= Dsp_data[sid].m_xConfig.pCtrlParm;
	pConfig->bVAD 			= Dsp_data[sid].m_xConfig.bVAD;
	pConfig->nVadLevel 		= Dsp_data[sid].m_xConfig.nVadLevel;
/*	pConfig->bAES 			= Dsp_data[sid].m_xConfig.bAES;
	pConfig->nHangoverTime 		= Dsp_data[sid].m_xConfig.nHangoverTime;
	pConfig->nBGNoiseLevel 		= Dsp_data[sid].m_xConfig.nBGNoiseLevel;
	pConfig->nAttenRange 		= Dsp_data[sid].m_xConfig.nAttenRange;
	pConfig->nTXGain 		= Dsp_data[sid].m_xConfig.nTXGain;
	pConfig->nRXGain 		= Dsp_data[sid].m_xConfig.nRXGain;
	pConfig->nTRRatio 		= Dsp_data[sid].m_xConfig.nTRRatio;
	pConfig->nRTRatio 		= Dsp_data[sid].m_xConfig.nRTRatio;
*/	pConfig->bPLC 			= Dsp_data[sid].m_xConfig.bPLC;
	return DSPCODEC_SUCCESS;
}

#if 0
DSPCODEC_ALGORITHM DspcodecGetAlgorithm(uint32 sid)
{
	/* DSPCODEC_ALGORITHM_UNKNOW can be used, only if open source part. */
	if(sid >= sess_num)
		return DSPCODEC_ALGORITHM_UNKNOW;

	return Dsp_data[sid].m_uCodingAlgorithm;
}
#endif

RESULT DspcodecStart(uint32 sid, DSPCODEC_ACTION uAction)
{
	RESULT retval;
	int i;

	if(uAction > DSPCODEC_ACTION_DECODE)
	{
		printk("Dspcodec: !!! Wrong parameter of uAction %d !!!\n", uAction);
		assert(uAction <= DSPCODEC_ACTION_DECODE);
		return DSPCODEC_ERROR_ACTION;
	}

	memset((void *)(&(pSendParm[sid].xParm)), 0, sizeof(CDspcodecStartParm));
	pSendParm[sid].uCommand = DSPCODEC_COMMAND_START;
	pSendParm[sid].nOwner = 0;
	pSendParm[sid].xParm.xStartParm.uCodingAction = uAction;

	retval = SendCommand(sid);

	// re-initialize local variables
	if(uAction == DSPCODEC_ACTION_DECODE)	// only write()
	{
		for(i=0; i<BUFFER_NUM_ENC; i++)
			pRecvParm[sid].xEncOutput[i].nOwner = 0;
		pRecvParm[sid].nOwner = 0;
		pRecvParm[sid].nSeqNo = 0;

		Dsp_data[sid].m_nPosEnc = 0;
		Dsp_data[sid].m_nFrameRecv = 0;
	}
	else if(uAction == DSPCODEC_ACTION_ENCODE)	// only read()
	{
		for(i=0; i<BUFFER_NUM_DEC; i++)
		{
			pSendParm[sid].xDecBuffer[i].nOwner = 1;
			pSendParm[sid].xDecBuffer[i].nSeqNo = 0;
		}
		pSendParm[sid].nOwner = 0;
		pSendParm[sid].nSeqNo = 0;
		pSendParm[sid].nFramtPut = 0;

		Dsp_data[sid].m_nPosDec = 0;
		Dsp_data[sid].m_nPreSeq = 0;
		Dsp_data[sid].m_bSeqGet = false;
		Dsp_data[sid].m_nPreTick = 0;
		Dsp_data[sid].m_nPreTimeStamp = 0;
		Dsp_data[sid].m_nWState = DSPCODEC_WSTATE_NORMALJITTER;
#ifdef CONFIG_RTK_VOIP_G7231
		if((Dsp_data[sid].m_uCodingAlgorithm == DSPCODEC_ALGORITHM_G7231A53) ||
			(Dsp_data[sid].m_uCodingAlgorithm == DSPCODEC_ALGORITHM_G7231A63))
		{
			Dsp_data[sid].m_nInitTd = G723PRETD;
		} else 
#endif
#ifdef CONFIG_RTK_VOIP_G729AB
		if(Dsp_data[sid].m_uCodingAlgorithm == DSPCODEC_ALGORITHM_G729)
		{
			Dsp_data[sid].m_nInitTd = G729PRETD;
		} else
#endif
		{
			Dsp_data[sid].m_nInitTd = G711PRETD;
		}
		Dsp_data[sid].m_nPreTd = Dsp_data[sid].m_nInitTd;
		Dsp_data[sid].m_nAveTd = Dsp_data[sid].m_nInitTd;
		Dsp_data[sid].m_nTdOffset = 0;
		Dsp_data[sid].m_njSN = 0;
		Dsp_data[sid].m_nFrameSent = 0;
		Dsp_data[sid].m_nEarlyFrame = 0;
		Dsp_data[sid].m_nLateFrame = 0;
		Dsp_data[sid].m_nNotOwner = 0;
		Dsp_data[sid].m_nDuplicate = 0;

#ifdef SUPPORT_DYNAMIC_PAYLOAD
		DynamicPayloadInit( sid );
#endif

#ifdef SUPPORT_DETECT_LONG_TERM_NO_RTP
		RTP_RxPacketCount[ sid ] = 0;
		RTP_RxPacketCountPrev[ sid ] = 0;
		RTP_RxPacketWaitCount[ sid ] = 0;
#endif
	}

	return retval;
}

void reset_jitter_buffer(uint32 sid)
{
	int i ;

	// clear the owner bits of jitter buffer and receiving buffer
	for(i=0; i<BUFFER_NUM_DEC; i++)
	{
		pSendParm[sid].xDecBuffer[i].nOwner = 1;
		pSendParm[sid].xDecBuffer[i].nSeqNo = 0;
	}
	for(i=0; i<BUFFER_NUM_ENC; i++)
		pRecvParm[sid].xEncOutput[i].nOwner = 0;
	pSendParm[sid].nOwner = 0;
	pSendParm[sid].nSeqNo = 0;
	pSendParm[sid].nFramtPut = 0;
	pRecvParm[sid].nOwner = 0;
	pRecvParm[sid].nSeqNo = 0;
}

RESULT DspcodecStop(uint32 sid)
{
	RESULT retval;

	pSendParm[sid].uCommand = DSPCODEC_COMMAND_STOP;
	pSendParm[sid].nOwner = 0;
	retval = SendCommand(sid);

	if(retval != DSPCODEC_SUCCESS)
	{
	}
	return retval;
}

RESULT DspcodecMute(uint32 sid, DSPCODEC_MUTEDIRECTION uDirection)
{
	RESULT retval;

	if(uDirection > DSPCODEC_MUTEDIRECTION_NONE)
	{
		printk("Dspcodec: !!! Wrong parameter of uDirection %d !!!\n", uDirection);
		assert(uDirection <= DSPCODEC_MUTEDIRECTION_NONE);
		return DSPCODEC_ERROR_MUTEDIRECTION;
	}

	memset((void *)(&(pSendParm[sid].xParm)), 0, sizeof(CDspcodecMuteParm));
	pSendParm[sid].uCommand = DSPCODEC_COMMAND_MUTE;
	pSendParm[sid].nOwner = 0;
	pSendParm[sid].xParm.xMuteParm.uMuteDirection = uDirection;
	retval = SendCommand(sid);

	if(retval != DSPCODEC_SUCCESS)
	{
	}
	return retval;
}

/*
 dtmf_mode = 0 : RFC2833
 dtmf_mode = 1 : INFO
 dtmf_mode = 2 : Inband
*/
char dtmf_mode[MAX_SLIC_CH_NUM] = {0};
RESULT DspcodecPlayTone(uint32 sid, DSPCODEC_TONE nTone, bool bFlag, DSPCODEC_TONEDIRECTION path)
{
	RESULT retval;
	uint8 value;
	if((nTone < DSPCODEC_TONE_0) || (nTone > DSPCODEC_TONE_KEY))
	{
		assert((nTone >= DSPCODEC_TONE_0) && (nTone <= DSPCODEC_TONE_KEY));
		return DSPCODEC_ERROR_TONE;
	}

	if(path > DSPCODEC_TONEDIRECTION_BOTH)
	{
		assert(path <= DSPCODEC_TONEDIRECTION_BOTH);
		return DSPCODEC_ERROR_PARAMETER;
	}

	memset((void *)(&(pSendParm[sid].xParm)), 0, sizeof(CDspcodecPlayToneParm));
	pSendParm[sid].uCommand = DSPCODEC_COMMAND_PLAYTONE;
	pSendParm[sid].nOwner = 0;

	if((nTone >= DSPCODEC_TONE_0) && (nTone <= DSPCODEC_TONE_HASHSIGN))
	{
		pSendParm[sid].xParm.xPlayToneParm.nTone = nTone;
#ifndef CONFIG_RTK_VOIP_IP_PHONE
		if( dtmf_mode[0]==2/*_inband*/ )	// hc$ note, must follow chid
			path = DSPCODEC_TONEDIRECTION_BOTH ;	//WJF 931122 debug
#endif
	}
	else if ((nTone >= DSPCODEC_TONE_0_CONT) && (nTone <= DSPCODEC_TONE_HASHSIGN_CONT))
	{	// thlin+ continous DTMF tone play for RFC2833
		pSendParm[sid].xParm.xPlayToneParm.nTone = nTone-DSPCODEC_TONE_0_CONT+DIGIT_0_CONT;
	}
	else
	{
		switch(nTone)
		{
#ifdef SUPPORT_TONE_PROFILE

		case DSPCODEC_TONE_DIAL:
		case DSPCODEC_TONE_STUTTERDIAL:
		case DSPCODEC_TONE_MESSAGE_WAITING:
		case DSPCODEC_TONE_CONFIRMATION:
		case DSPCODEC_TONE_RINGING:
		case DSPCODEC_TONE_BUSY:
		case DSPCODEC_TONE_CONGESTION:
		case DSPCODEC_TONE_ROH:
		case DSPCODEC_TONE_DOUBLE_RING:
		case DSPCODEC_TONE_SIT_NOCIRCUIT:
		case DSPCODEC_TONE_SIT_INTERCEPT:
		case DSPCODEC_TONE_SIT_VACANT:
		case DSPCODEC_TONE_SIT_REORDER:
		case DSPCODEC_TONE_CALLING_CARD_WITHEVENT:
		case DSPCODEC_TONE_CALLING_CARD:
		case DSPCODEC_TONE_CALL_WAITING:
		case DSPCODEC_TONE_CALL_WAITING_2:
		case DSPCODEC_TONE_CALL_WAITING_3:
		case DSPCODEC_TONE_CALL_WAITING_4:
		case DSPCODEC_TONE_INGRESS_RINGBACK:
			pSendParm[sid].xParm.xPlayToneParm.nTone = Dsp_data[sid].m_nBaseTone + nTone - DSPCODEC_TONE_DIAL;
			break;
		case DSPCODEC_TONE_HOLD:
			pSendParm[sid].xParm.xPlayToneParm.nTone = HOLDING;
			break;
		case DSPCODEC_TONE_OFFHOOKWARNING:
			pSendParm[sid].xParm.xPlayToneParm.nTone = OFFHOOKWARING;
			break;
		case DSPCODEC_TONE_RING:
			pSendParm[sid].xParm.xPlayToneParm.nTone = RING_1 + Dsp_data[sid].m_nRingType;
			break;
#else
		case DSPCODEC_TONE_DIAL:
			pSendParm[sid].xParm.xPlayToneParm.nTone = Dsp_data[sid].m_nDialTone;
			break;
		case DSPCODEC_TONE_RINGING:
			pSendParm[sid].xParm.xPlayToneParm.nTone = Dsp_data[sid].m_nRingTone;
			break;
		case DSPCODEC_TONE_BUSY:
			pSendParm[sid].xParm.xPlayToneParm.nTone = Dsp_data[sid].m_nBusyTone;
			break;
		case DSPCODEC_TONE_HOLD:
			pSendParm[sid].xParm.xPlayToneParm.nTone = HOLDING;
			break;
		case DSPCODEC_TONE_CALL_WAITING:
			pSendParm[sid].xParm.xPlayToneParm.nTone = Dsp_data[sid].m_nWaitingTone;
			break;
		case DSPCODEC_TONE_OFFHOOKWARNING:
			pSendParm[sid].xParm.xPlayToneParm.nTone = OFFHOOKWARING;
			break;
		case DSPCODEC_TONE_RING:
			pSendParm[sid].xParm.xPlayToneParm.nTone = RING_1 + Dsp_data[sid].m_nRingType;
			break;
#endif	// #ifdef SUPPORT_TONE_PROFILE

#ifdef SW_DTMF_CID	// hc+ 1124 for DTMF CID
		case DSPCODEC_TONE_A:
		case DSPCODEC_TONE_B:
		case DSPCODEC_TONE_C:
		case DSPCODEC_TONE_D:		
			pSendParm[sid].xParm.xPlayToneParm.nTone = DIGIT_A + (nTone-DSPCODEC_TONE_A);			
			break;
#endif

		case DSPCODEC_TONE_FSK_SAS:		// sandro+ 2006/07/24 for call waiting tone
			pSendParm[sid].xParm.xPlayToneParm.nTone = FSK_SAS;
			break;
			
		case DSPCODEC_TONE_FSK_ALERT:	// hc+ 1229 for off hook FSK CID
			pSendParm[sid].xParm.xPlayToneParm.nTone = FSK_ALERT;
			break;

		case DSPCODEC_TONE_NTT_IIT_TONE:
			pSendParm[sid].xParm.xPlayToneParm.nTone = NTT_IIT_TONE;
			printk("ntt_tone,%d\n",NTT_IIT_TONE);
			break;
		
		case DSPCODEC_TONE_KEY:
			pSendParm[sid].xParm.xPlayToneParm.nTone = DIGIT_PONDA;
			break;
		default:
			/* should never go here */
			assert(0);
		}
	}
	pSendParm[sid].xParm.xPlayToneParm.bFlag = bFlag;
	pSendParm[sid].xParm.xPlayToneParm.uToneDirection = path;

	retval = SendCommand(sid);

	if(retval != DSPCODEC_SUCCESS)
	{
		printk("Playtone command not successful\n");
	}
	value = value;	// remove compiling warning
	return retval;
}

//WJF 930806 added, when reached tx_jit_buf_high_threshold, touch_high_threshold will be 1, 
// touch_high_threshold will be 0 when down to tx_jit_buf_low_threshold
//int tx_jit_buf_low_threshold=10, tx_jit_buf_high_threshold=30, touch_high_threshold=0 ;
///static char local_buf[640] ;
//WJF 930806, return 1 if tx jitter buffer over loaded, DSP not fast enough to play the voice
int check_if_jitter_buffer_over_loaded(uint32 sid, int k, int r, int tx_jitter_buffer_load)
{
#ifndef RESERVE_SPACE_FOR_SLOW
	int i, n;
	
	tx_jitter_buffer_load=0;
	n = (k+1) % BUFFER_NUM_DEC ;	//next position

	if( pSendParm[sid].xDecBuffer[k].nOwner == 1 )
	{
		touch_high_threshold[sid] = 0 ;
		return touch_high_threshold[sid] ;
	}

	k = (n+BUFFER_NUM_DEC-tx_jit_buf_high_threshold[sid]) % BUFFER_NUM_DEC ;	//previous 30 position
	for(i=0; i<tx_jit_buf_high_threshold[sid]; i++)
	{
		if( pSendParm[sid].xDecBuffer[k].nOwner == 0 )
			++tx_jitter_buffer_load ;
		k = (k+1) % BUFFER_NUM_DEC ;	//previous 30 + next i position
	}
#endif	// !RESERVE_SPACE_FOR_SLOW

	if( touch_high_threshold[sid] )
	{
		if( tx_jitter_buffer_load < tx_jit_buf_low_threshold[sid] )
			touch_high_threshold[sid] = 0 ;
	}
	else if( tx_jitter_buffer_load >= tx_jit_buf_high_threshold[sid] )
	{
		touch_high_threshold[sid] = 1 ;
	}

	return touch_high_threshold[sid];
}

#ifdef SUPPORT_SYNC_OUT_OF_SEQ
static void SyncDynamicPayload( uint32 sid, uint16 seq )
{
	DPNotFirstPacket[ sid ] = 0;
	DPSyncInSeq[ sid ] = seq;
	DPEnableSyncInSeq[ sid ] = 1;
	
	printk( "Synchronize to %d after 30 out-of-seq(%d).\n", seq, sid );
}
#endif /* SUPPORT_SYNC_OUT_OF_SEQ */

//#define debug_in_console
//WJF 930806 notice :
// The nSeq written into the xDecBuffer[] better be in sequence
//  otherwise, the DSP could induce some delay, even stop to play and no voice heard
///static uint32 nSeqPrev=0;
int32 DspcodecWrite(uint32 chid, uint32 sid, uchar* pBuf, int32 nSize, uint32 nSeq, uint32 nTimestamp)
{
	extern Word16 jBufPtr[];
	uint32 jbuf_ri;
	int jbuf_load;

	uint32 n = 0, retval = 0;
	uint32 nowTick;
	int32  diffTimeStamp, Td = 0;
	
#ifdef RESERVE_SPACE_FOR_SLOW
	uint16 seqDelta, seqRes;
	uint16 i;
	uint16 seq;
#endif

	if((pBuf == NULL) || (nSize == 0))
		return 0;

	if(!Dsp_data[sid].m_bSeqGet)		// the very first frame
	{
		if( !pRecvParm[sid].bPutNew )
			return 0;	/* wait for jbc ready */
	
		Dsp_data[sid].m_nFrameSent++;
		pSendParm[sid].nSeqNo = nSeq;
		pSendParm[sid].nFramtPut = Dsp_data[sid].m_nFrameSent;
		pSendParm[sid].xDecBuffer[0].nSeqNo = nSeq;
		pSendParm[sid].xDecBuffer[0].nTimeStamp = nTimestamp;
		pSendParm[sid].xDecBuffer[0].nSize = nSize;
		pSendParm[sid].xDecBuffer[0].nTd = Dsp_data[sid].m_nInitTd;
		memcpy((void *)pSendParm[sid].xDecBuffer[0].pBuffer, pBuf, nSize);
		pSendParm[sid].xDecBuffer[0].nOwner = 0;
		retval = nSize;
		Dsp_data[sid].m_nPreSeq = nSeq;
		Dsp_data[sid].m_bSeqGet = true;
		Dsp_data[sid].m_nPreTick = pRecvParm[sid].nRunTick;
		Dsp_data[sid].m_nPreTimeStamp = nTimestamp;
		Dsp_data[sid].m_nAveTd = (32703 * (short)(Dsp_data[sid].m_nAveTd / 128) + 65 * (short)(Dsp_data[sid].m_nInitTd / 128)) >> 8;
	}
#ifdef RESERVE_SPACE_FOR_SLOW
	else if( RtpSeqGreater(nSeq, Dsp_data[sid].m_nPreSeq) )		// normal frames
#else
	else if( RtpSeqGreater(nSeq, Dsp_data[sid].m_nPreSeq) || (outof_seq_cnt[sid]>30) )		// normal frames
#endif
	{

#ifdef RESERVE_SPACE_FOR_SLOW
		jbuf_ri = jBufPtr[ sid ];
		
		// k is written index; r index is not read yet
		if( Dsp_data[sid].m_nPosDec == jbuf_ri ) {
			if( pSendParm[sid].xDecBuffer[jbuf_ri].nOwner == 1 )
				jbuf_load = 0;	// init state.
			else
				jbuf_load = 1;
		} else if( ( ( Dsp_data[sid].m_nPosDec + 1 ) % BUFFER_NUM_DEC ) == jbuf_ri )
		{	// out of buffer 
			// In our design, buffer will not be full.
			jbuf_load = 0;
		} else if( jbuf_ri > Dsp_data[sid].m_nPosDec ) {
			jbuf_load = Dsp_data[sid].m_nPosDec + 1 + BUFFER_NUM_DEC - jbuf_ri;
		} else {
			jbuf_load = Dsp_data[sid].m_nPosDec + 1 - jbuf_ri;
		}
#else
		jbuf_load = 0;	// unused if RESERVE_SPACE_FOR_SLOW not defined
#endif //RESERVE_SPACE_FOR_SLOW
		
		//WJF 930806 added, discard the packet if this jitter buffer is over loaded
		if( check_if_jitter_buffer_over_loaded(sid, Dsp_data[sid].m_nPosDec, jbuf_ri, jbuf_load) )
		{
			if( ++over_load_cnt[sid] > 30 )	//WJF 931106 needed to do, reset buffer if over loaded too long
			{
			}	
			
			switch( over_load_cnt[sid] ) {
			case 1:
				debug_error("D%d ", sid);
				break;
			case 10:
				debug_error("Da ");
				break;
			}
			
			Dsp_data[sid].m_nPreSeq = nSeq;
			Dsp_data[sid].m_nLateFrame++;
		}
		else if( pRecvParm[sid].bPutNew )
		{			
			over_load_cnt[sid] = 0 ;
			n = (Dsp_data[sid].m_nPosDec+1)%BUFFER_NUM_DEC ;

#ifdef RESERVE_SPACE_FOR_SLOW
			seqDelta = ( uint16 )( nSeq - Dsp_data[sid].m_nPreSeq - 1 );
			
			if( outof_seq_cnt[sid] > 30 ) {
				// too many out-of-seq, don't fill SKIP
			} else if( seqDelta ) {
				/* 
				 * Maximum reserved space is high threshold -5, because
				 * it receives many frames per packet.
				 */
				if( tx_jit_buf_high_threshold[ sid ] - 5 <= jbuf_load )
					seqRes = 0;
				else if( tx_jit_buf_high_threshold[ sid ] - 5 <= jbuf_load + seqDelta ) {
					seqRes = tx_jit_buf_high_threshold[ sid ] - 5 - jbuf_load - 1;	// 1 for arrive packet
				} else
					seqRes = seqDelta;

				seq = nSeq - seqRes;
					
				for( i = 0; i < seqRes; i ++ ) {
					// fill seq# to reserved space only
					pSendParm[sid].xDecBuffer[n].nSeqNo = seq ++;
					pSendParm[sid].xDecBuffer[n].nTimeStamp = 0;
					pSendParm[sid].xDecBuffer[n].nSize = 0;
					pSendParm[sid].xDecBuffer[n].nTd = 0;
					pSendParm[sid].xDecBuffer[n].nOwner = R0_RESERVE; // 2
				
					n = (n+1)%BUFFER_NUM_DEC;
				}
			}
#endif // RESERVE_SPACE_FOR_SLOW

			Dsp_data[sid].m_nPreSeq = nSeq;
			Dsp_data[sid].m_nPosDec = n;

			nowTick = pRecvParm[sid].nRunTick;
			diffTimeStamp = (nTimestamp > Dsp_data[sid].m_nPreTimeStamp) ? (nTimestamp - Dsp_data[sid].m_nPreTimeStamp) :
				(0xffffffff - Dsp_data[sid].m_nPreTimeStamp + nTimestamp + 1);
			Td = (int32)(nowTick - Dsp_data[sid].m_nPreTick) - diffTimeStamp + Dsp_data[sid].m_nPreTd;
			//WJF 931121 added, check Td to avoid unreasonable situation
			if( (Td > (Dsp_data[sid].m_nInitTd+16000)) || (Td < (Dsp_data[sid].m_nInitTd-16000)) )
				Td = Dsp_data[sid].m_nInitTd;
			if(pSendParm[sid].xDecBuffer[n].nOwner == 1)
				Dsp_data[sid].m_nFrameSent++;
			else
				Dsp_data[sid].m_nNotOwner++;

			pSendParm[sid].nSeqNo = nSeq;
			pSendParm[sid].nFramtPut = Dsp_data[sid].m_nFrameSent;
			pSendParm[sid].xDecBuffer[n].nSeqNo = nSeq;
			pSendParm[sid].xDecBuffer[n].nTimeStamp = nTimestamp;
			pSendParm[sid].xDecBuffer[n].nSize = nSize;

			if((Dsp_data[sid].m_nWState == DSPCODEC_WSTATE_SYNCJITTER) && RtpSeqGreater(Dsp_data[sid].m_njSN, nSeq))
				pSendParm[sid].xDecBuffer[n].nTd = Td + Dsp_data[sid].m_nTdOffset;
			else
				pSendParm[sid].xDecBuffer[n].nTd = Td;

			memcpy((void *)pSendParm[sid].xDecBuffer[n].pBuffer, pBuf, nSize);
			pSendParm[sid].xDecBuffer[n].nOwner = 0;
			retval = nSize;
			Dsp_data[sid].m_nPreTick = nowTick;
			Dsp_data[sid].m_nPreTimeStamp = nTimestamp;
			Dsp_data[sid].m_nPreTd = Td;

			if(Dsp_data[sid].m_nWState == DSPCODEC_WSTATE_NORMALJITTER)
			{
				Dsp_data[sid].m_nAveTd = (32703 * (short)(Dsp_data[sid].m_nAveTd / 128) + 65 * (short)(Td / 128)) >> 8;
				if((Dsp_data[sid].m_nAveTd > (TDOFFSET_PO + Dsp_data[sid].m_nInitTd)) ||
					(Dsp_data[sid].m_nAveTd < (TDOFFSET_NE + Dsp_data[sid].m_nInitTd)))
				{
					Dsp_data[sid].m_nTdOffset = (Dsp_data[sid].m_nAveTd > (TDOFFSET_PO + Dsp_data[sid].m_nInitTd))?
   						TDOFFSET_PO : TDOFFSET_NE;
					Dsp_data[sid].m_njSN = nSeq;
					SyncJitterBuf(sid, Dsp_data[sid].m_njSN, Dsp_data[sid].m_nTdOffset);
					Dsp_data[sid].m_nPreTd -= Dsp_data[sid].m_nTdOffset;
					Dsp_data[sid].m_nWState = DSPCODEC_WSTATE_SYNCJITTER;
				}
			}
			else
			{
				if(RtpSeqGreater(pRecvParm[sid].nSeqNo, Dsp_data[sid].m_njSN))
				{
					Dsp_data[sid].m_nAveTd -= Dsp_data[sid].m_nTdOffset;
					Dsp_data[sid].m_nTdOffset = 0;
					Dsp_data[sid].m_nWState = DSPCODEC_WSTATE_NORMALJITTER;
				}
			}
		}
		else
		{
			Dsp_data[sid].m_nEarlyFrame++;
		}

		outof_seq_cnt[sid] = 0;
	}
	else	// out of sequence packet, recover sequence if 30 continuous outofseq happenned
	{
#ifdef RESERVE_SPACE_FOR_SLOW
		bool bReservedSpaceFound = false;
		
		jbuf_ri = jBufPtr[ sid ];

		n = Dsp_data[sid].m_nPosDec;
			
		if( ( ( n + 1 ) % BUFFER_NUM_DEC ) != jbuf_ri ) {
			// If not empty, search for reserved space
			nowTick = pRecvParm[sid].nRunTick;
			
			for( i = 0; i < tx_jit_buf_high_threshold[ sid ]; i ++ ) {
				
				if( pSendParm[sid].xDecBuffer[n].nOwner == R0_RESERVE ) {
					if( pSendParm[sid].xDecBuffer[n].nSeqNo == nSeq ) {
											
						// fill reserved space
						pSendParm[sid].xDecBuffer[n].nSeqNo = nSeq;
						pSendParm[sid].xDecBuffer[n].nTimeStamp = nTimestamp;
						pSendParm[sid].xDecBuffer[n].nSize = nSize;
			
						if((Dsp_data[sid].m_nWState == DSPCODEC_WSTATE_SYNCJITTER) && RtpSeqGreater(Dsp_data[sid].m_njSN, nSeq))
							pSendParm[sid].xDecBuffer[n].nTd = Td + Dsp_data[sid].m_nTdOffset;
						else
							pSendParm[sid].xDecBuffer[n].nTd = Td;
						
						memcpy((void *)pSendParm[sid].xDecBuffer[n].pBuffer, pBuf, nSize);
						pSendParm[sid].xDecBuffer[n].nOwner = 0;
						retval = nSize;
						Dsp_data[sid].m_nPreTick = nowTick;
						Dsp_data[sid].m_nPreTimeStamp = nTimestamp;
						Dsp_data[sid].m_nPreTd = Td;						
					
						outof_seq_cnt[sid] = 0;
						bReservedSpaceFound = true;
						break;
					}
				} 
	
				if( jbuf_ri == n )	// meet read index
					break;
				
				n = (n+BUFFER_NUM_DEC-1)%BUFFER_NUM_DEC;	// n --;
			}
		}
		
		if( !bReservedSpaceFound )
#endif // RESERVE_SPACE_FOR_SLOW
		{
			//debug_warning("%d:OOSD(%d,%d) ", sid, Dsp_data[sid].m_nPreSeq, nSeq);
			debug_warning("OSD%d ", sid);

#ifdef SUPPORT_SYNC_OUT_OF_SEQ			
			if( ( uint16 )( outof_seq_prevSeq[ sid ] + 1 ) == ( uint16 )nSeq ) {
				if( ++ outof_seq_cnt[ sid ] > 30 ) {
					SyncDynamicPayload( sid, Dsp_data[sid].m_nPreSeq );
					outof_seq_cnt[ sid ] = 0;
				}
			} else
				outof_seq_cnt[ sid ] = 0;
			
			outof_seq_prevSeq[ sid ] = nSeq;
#else
			++outof_seq_cnt[sid] ;
#endif /* SUPPORT_SYNC_OUT_OF_SEQ */
			
			Dsp_data[sid].m_nLateFrame++;
		}
	}

	return retval;
}

int32 DspcodecRead(uint32 chid, uint32 sid, uchar* pBuf, int32 nSize)
{
	int length = 0;

	if((pBuf == NULL) || (nSize == 0))
		return 0;

	if(pRecvParm[sid].xEncOutput[Dsp_data[sid].m_nPosEnc].nOwner == 1)
	{
		if(pRecvParm[sid].xEncOutput[Dsp_data[sid].m_nPosEnc].nSize == 0)
			length = -1;	// invalid length to distinguish that it is silence
		else
		{
			length = (nSize >= pRecvParm[sid].xEncOutput[Dsp_data[sid].m_nPosEnc].nSize)?
				pRecvParm[sid].xEncOutput[Dsp_data[sid].m_nPosEnc].nSize : nSize;

			memcpy(pBuf, (void *)pRecvParm[sid].xEncOutput[Dsp_data[sid].m_nPosEnc].pBuffer, length);
			Dsp_data[sid].m_nFrameRecv++;
		}
		pRecvParm[sid].xEncOutput[Dsp_data[sid].m_nPosEnc].nOwner = 0;
		Dsp_data[sid].m_nPosEnc++;
		if(Dsp_data[sid].m_nPosEnc == BUFFER_NUM_ENC)
			Dsp_data[sid].m_nPosEnc = 0;
	}

	return length;
}

RESULT DspcodecSetVolume(int32 nVal)
{
#ifdef __ECOS
	uint8 vol, value;
	if((nVal < 0) || (nVal > 10))
	{
		printk("Dspcodec: !!! Wrong parameter of nVal %d !!!\n", nVal);
		assert((nVal >= 0) && (nVal <= 10));
		return DSPCODEC_ERROR_PARAMETER;
	}
	vol = RxGainMap[nVal];
	value = CodecRead(0x02);
	value = (value & 0xf0) | vol;
	CodecWrChk(0x02, value);
//	printf("Dspcodec: Set volume gain: [%ddB]\n", (int)vol*2-28);
#endif
	return DSPCODEC_SUCCESS;
}

RESULT DspcodecSetEncGain(int32 nVal)
{
#ifdef __ECOS
	uint8 vol, value;
	if((nVal < 0) || (nVal > 10))
	{
		assert((nVal >= 0) && (nVal <= 10));
		return DSPCODEC_ERROR_PARAMETER;
	}
	vol = TxGainMap[nVal];
#ifdef NEW_CODEC_TX_GAIN
	CodecWrChk(0x03, vol);
#else
	value = CodecRead(0x03);
	value = (value & 0xf0) | vol;
	CodecWrChk(0x03, value);
#endif
#endif
	return DSPCODEC_SUCCESS;
}

RESULT DspcodecSetSidetoneGain(int32 nVal)
{
#ifdef __ECOS
	uint8 vol, value;
	if((nVal < 0) || (nVal > 10))
	{
		assert((nVal >= 0) && (nVal <= 10));
		return DSPCODEC_ERROR_PARAMETER;
	}
	vol = STGainMap[nVal];
	value = CodecRead(0x02);
	value = (value & 0x0f) | (vol << 4);
	CodecWrChk(0x02, value);
#endif
	return DSPCODEC_SUCCESS;
}

RESULT DspcodecSidetoneSwitch(bool bFlag)
{
#ifdef __ECOS
	uint8 value;

	Dsp_data.m_bSidetoneOn = bFlag;
	if(bFlag)
	{
		if(Dsp_data.m_nMediaOn)
		{
			// turn on sidetone
			value = CodecRead(0x00);
			value = value & 0xef;
			CodecWrChk(0x00, value);
		}
	}
	else
	{
		if(Dsp_data.m_nMediaOn)
		{
			// turn off sidetone
			value = CodecRead(0x00);
			value = value | 0x10;
			CodecWrChk(0x00, value);
		}
	}
#endif
	return DSPCODEC_SUCCESS;
}

RESULT DspcodecSetRingType(uint32 sid, int32 nRing)
{
	if((nRing < 0) || (nRing > 9))
	{
//		printk("Dspcodec: !!! Wrong parameter of nRing %d !!!\n", nRing);
		assert((nRing >= 0) && (nRing <= 9));
		return DSPCODEC_ERROR_PARAMETER;
	}

	Dsp_data[sid].m_nRingType = nRing;
	return DSPCODEC_SUCCESS;
}

RESULT DspcodecSetCountry(uint32 sid, int32 nCountry)
{
	int k;
#ifdef SUPPORT_TONE_PROFILE
	int pre_country_tone;

	pre_country_tone = Dsp_data[sid].m_nBaseTone;
#else
	int pre_m_nDialTone;
	int pre_m_nRingTone;
	int pre_m_nBusyTone;
	int pre_m_nWaitingTone;

	pre_m_nDialTone = Dsp_data[sid].m_nDialTone;
	pre_m_nRingTone = Dsp_data[sid].m_nRingTone;
	pre_m_nBusyTone = Dsp_data[sid].m_nBusyTone;
	pre_m_nWaitingTone = Dsp_data[sid].m_nWaitingTone;
#endif //#ifdef SUPPORT_TONE_PROFILE
	switch(nCountry)
	{
#ifdef SUPPORT_TONE_PROFILE
		case DSPCODEC_COUNTRY_USA:
			Dsp_data[sid].m_nBaseTone = USA_DIAL;
			break;
		case DSPCODEC_COUNTRY_UK:
			Dsp_data[sid].m_nBaseTone = UK_DIAL;
			break;
		case DSPCODEC_COUNTRY_AUSTRALIA:
			Dsp_data[sid].m_nBaseTone = AUSTRALIA_DIAL;
			break;
		case DSPCODEC_COUNTRY_HK:
			Dsp_data[sid].m_nBaseTone = HK_DIAL;
			break;
		case DSPCODEC_COUNTRY_JP:
			Dsp_data[sid].m_nBaseTone = JP_DIAL;
			break;
		case DSPCODEC_COUNTRY_SE:
			Dsp_data[sid].m_nBaseTone = SE_DIAL;
			break;
		case DSPCODEC_COUNTRY_GR:
			Dsp_data[sid].m_nBaseTone = GR_DIAL;
			break;
		case DSPCODEC_COUNTRY_FR:
			Dsp_data[sid].m_nBaseTone = FR_DIAL;
			break;
		case DSPCODEC_COUNTRY_TR:
			Dsp_data[sid].m_nBaseTone = TR_DIAL;
			break;
		case DSPCODEC_COUNTRY_BE:
			Dsp_data[sid].m_nBaseTone = BE_DIAL;
			break;
		case DSPCODEC_COUNTRY_FL:
			Dsp_data[sid].m_nBaseTone = FL_DIAL;
			break;
		case DSPCODEC_COUNTRY_IT:
			Dsp_data[sid].m_nBaseTone = IT_DIAL;
			break;
		case DSPCODEC_COUNTRY_CN:
			Dsp_data[sid].m_nBaseTone = CN_DIAL;
			break;
		case DSPCODEC_COUNTRY_CUSTOME:
			Dsp_data[sid].m_nBaseTone = CUSTOM_DIAL;
			break;
#else
		case DSPCODEC_COUNTRY_USA:
			Dsp_data[sid].m_nDialTone = USA_DIAL;
			Dsp_data[sid].m_nRingTone = USA_RING;
			Dsp_data[sid].m_nBusyTone = USA_BUSY;
			Dsp_data[sid].m_nWaitingTone = USA_WAITING;
			break;
		case DSPCODEC_COUNTRY_UK:
			Dsp_data[sid].m_nDialTone = UK_DIAL;
			Dsp_data[sid].m_nRingTone = UK_RING;
			Dsp_data[sid].m_nBusyTone = UK_BUSY;
			Dsp_data[sid].m_nWaitingTone = USA_WAITING;
			break;
		case DSPCODEC_COUNTRY_AUSTRALIA:
			Dsp_data[sid].m_nDialTone = AUSTRALIA_DIAL;
			Dsp_data[sid].m_nRingTone = AUSTRALIA_RING;
			Dsp_data[sid].m_nBusyTone = AUSTRALIA_BUSY;
			Dsp_data[sid].m_nWaitingTone = AUSTRALIA_WAITING;
			break;
		case DSPCODEC_COUNTRY_HK:
			Dsp_data[sid].m_nDialTone = HK_DIAL;
			Dsp_data[sid].m_nRingTone = HK_RING;
			Dsp_data[sid].m_nBusyTone = HK_BUSY;
			Dsp_data[sid].m_nWaitingTone = HK_WAITING;
			break;
		case DSPCODEC_COUNTRY_JP:
			Dsp_data[sid].m_nDialTone = JP_DIAL;
			Dsp_data[sid].m_nRingTone = JP_RING;
			Dsp_data[sid].m_nBusyTone = JP_BUSY;
			Dsp_data[sid].m_nWaitingTone = JP_WAITING;
			break;
		case DSPCODEC_COUNTRY_SE:
			Dsp_data[sid].m_nDialTone = SE_DIAL;
			Dsp_data[sid].m_nRingTone = SE_RING;
			Dsp_data[sid].m_nBusyTone = SE_BUSY;
			Dsp_data[sid].m_nWaitingTone = SE_WAITING;
			break;
		case DSPCODEC_COUNTRY_GR:
			Dsp_data[sid].m_nDialTone = GR_DIAL;
			Dsp_data[sid].m_nRingTone = GR_RING;
			Dsp_data[sid].m_nBusyTone = GR_BUSY;
			Dsp_data[sid].m_nWaitingTone = GR_WAITING;
			break;
		case DSPCODEC_COUNTRY_FR:
			Dsp_data[sid].m_nDialTone = FR_DIAL;
			Dsp_data[sid].m_nRingTone = FR_RING;
			Dsp_data[sid].m_nBusyTone = FR_BUSY;
			Dsp_data[sid].m_nWaitingTone = FR_WAITING;
			break;
		case DSPCODEC_COUNTRY_TR:
			Dsp_data[sid].m_nDialTone = TR_DIAL;
			Dsp_data[sid].m_nRingTone = TR_RING;
			Dsp_data[sid].m_nBusyTone = TR_BUSY;
			Dsp_data[sid].m_nWaitingTone = TR_WAITING;
			break;
		case DSPCODEC_COUNTRY_BE:
			Dsp_data[sid].m_nDialTone = BE_DIAL;
			Dsp_data[sid].m_nRingTone = BE_RING;
			Dsp_data[sid].m_nBusyTone = BE_BUSY;
			Dsp_data[sid].m_nWaitingTone = BE_WAITING;
			break;
		case DSPCODEC_COUNTRY_FL:
			Dsp_data[sid].m_nDialTone = FL_DIAL;
			Dsp_data[sid].m_nRingTone = FL_RING;
			Dsp_data[sid].m_nBusyTone = FL_BUSY;
			Dsp_data[sid].m_nWaitingTone = FL_WAITING;
			break;
		case DSPCODEC_COUNTRY_IT:
			Dsp_data[sid].m_nDialTone = IT_DIAL;
			Dsp_data[sid].m_nRingTone = IT_RING;
			Dsp_data[sid].m_nBusyTone = IT_BUSY;
			Dsp_data[sid].m_nWaitingTone = IT_WAITING;
			break;
		case DSPCODEC_COUNTRY_CN:
			Dsp_data[sid].m_nDialTone = CN_DIAL;
			Dsp_data[sid].m_nRingTone = CN_RING;
			Dsp_data[sid].m_nBusyTone = CN_BUSY;
			Dsp_data[sid].m_nWaitingTone = CN_WAITING;
			break;
		case DSPCODEC_COUNTRY_CUSTOME:
			Dsp_data[sid].m_nDialTone = dialTone+CUSTOM_TONE1;
			Dsp_data[sid].m_nRingTone = ringTone+CUSTOM_TONE1;
			Dsp_data[sid].m_nBusyTone = busyTone+CUSTOM_TONE1;
			Dsp_data[sid].m_nWaitingTone = waitingTone+CUSTOM_TONE1;
			break;
#endif
		default:
//			printf("Dspcodec: !!! Wrong parameter of nCountry %d !!!\n", nCountry);
			assert(0);
			return DSPCODEC_ERROR_PARAMETER;
	}
#ifdef SUPPORT_TONE_PROFILE

	if(pre_country_tone !=Dsp_data[sid].m_nBaseTone)//change country
	{
		for (k=0; k<CH_TONE; k++)
		{
			if (fPlayTone[sid][k] == 1
			   &&(WhichTone[sid][k]>= pre_country_tone)
			   &&(WhichTone[sid][k]<= (pre_country_tone+DSPCODEC_TONE_INGRESS_RINGBACK-DSPCODEC_TONE_DIAL)) )
			{
				printk("change tone when change country,sid=%d",sid);
				WhichTone[sid][k]=WhichTone[sid][k]+Dsp_data[sid].m_nBaseTone-pre_country_tone;
			}

		}
	}

#else
	if(  (pre_m_nDialTone != Dsp_data[sid].m_nDialTone)
	   ||(pre_m_nRingTone != Dsp_data[sid].m_nRingTone)
	   ||(pre_m_nBusyTone != Dsp_data[sid].m_nBusyTone)
	   ||(pre_m_nWaitingTone != Dsp_data[sid].m_nWaitingTone)  )//change country
	{
		for (k=0; k<CH_TONE; k++)
		{
			if (fPlayTone[sid][k] == 1 )
			{
				switch(WhichTone[sid][k])
				{
					case pre_m_nDialTone:
						WhichTone[sid][k] = Dsp_data[sid].m_nDialTone;
						printk("change tone when change country");
						break;
					case pre_m_nRingTone:
						WhichTone[sid][k] = Dsp_data[sid].m_nRingTone;
						printk("change tone when change country");
						break;
					case pre_m_nBusyTone:
						WhichTone[sid][k] = Dsp_data[sid].m_nBusyTone;
						printk("change tone when change country");
						break;
					case pre_m_nWaitingTone:
						WhichTone[sid][k] = Dsp_data[sid].m_nWaitingTone;
						printk("change tone when change country");
						break;
				}
			}

	}

#endif

	return DSPCODEC_SUCCESS;
}

RESULT DspcodecSetInterface(DSPCODEC_INTERFACE nInterface)
{
#ifdef __ECOS
	uint8 value;

	if(nInterface == DSPCODEC_INTERFACE_HANDSET)
	{
		Dsp_data[chid].m_nInterface = nInterface;
		value = CodecRead(0x00);
		value = value & 0xfc;
		CodecWrChk(0x00, value);
	}
	else if(nInterface == DSPCODEC_INTERFACE_HANDFREE)
	{
		Dsp_data[chid].m_nInterface = nInterface;
		value = CodecRead(0x00);
		value = (value & 0xfc) | 0x01;
		CodecWrChk(0x00, value);
	}
	else if(nInterface == DSPCODEC_INTERFACE_HEADSET)
	{
		Dsp_data[chid].m_nInterface = nInterface;
		value = CodecRead(0x00);
		value = (value & 0xfc) | 0x02;
		CodecWrChk(0x00, value);
	}
	else
	{
//		printf("Dspcodec: !!! No such interface %d !!!\n", nInterface);
		assert(0);
		return DSPCODEC_ERROR_PARAMETER;
	}
#endif
	return DSPCODEC_SUCCESS;
}

int32 DspcodecGetVersion(char *pBuf, int32 nLen)
{
	int n = 0;
	RESULT retval;

	if(!pBuf || !nLen)
		return 0;

#ifdef __ECOS
	sem_wait(&Dsp_data.m_semDspAccess);
	pSendParm->uCommand = DSPCODEC_COMMAND_GETVERSION;
	pSendParm->nOwner = 0;
	retval = SendCommand(sid);
	if(retval == DSPCODEC_SUCCESS)
	{
		n = (nLen > (int)strlen((char*)(pRecvParm->pPrintBuff)))?
			strlen((char*)(pRecvParm->pPrintBuff)) : (nLen - 1);
		strncpy(pBuf, (char*)(pRecvParm->pPrintBuff), n + 1);
	}
	sem_post(&Dsp_data.m_semDspAccess);

	if(retval != DSPCODEC_SUCCESS)
	{
//		printf("Dspcodec: !!! Send command [GETVERSION] fail !!!\n");
	}
#else
	strncpy(pBuf, "N/A", 4);
	n = 3;
#endif
	retval = retval;	// remove compiling warning
	return n;
}

void DspcodecResetR1(uint32 sid)
{
	RESULT retval;
	// send config to RISC1
	memset((void *)(&(pSendParm[sid].xParm)), 0, sizeof(CDspcodecSetConfigParm));
	pSendParm[sid].uCommand = DSPCODEC_COMMAND_SETCONFIG;
	pSendParm[sid].nOwner = 0;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.pToneTable 	= Dsp_data[sid].m_xConfig.pToneTable;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.pCtrlParm 	= Dsp_data[sid].m_xConfig.pCtrlParm;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.bVAD 		= Dsp_data[sid].m_xConfig.bVAD;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.nVadLevel 	= Dsp_data[sid].m_xConfig.nVadLevel;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.bAES 		= Dsp_data[sid].m_xConfig.bAES;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.nHangoverTime = Dsp_data[sid].m_xConfig.nHangoverTime;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.nBGNoiseLevel = Dsp_data[sid].m_xConfig.nBGNoiseLevel;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.nAttenRange = Dsp_data[sid].m_xConfig.nAttenRange;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.nTXGain 	= Dsp_data[sid].m_xConfig.nTXGain;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.nRXGain 	= Dsp_data[sid].m_xConfig.nRXGain;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.nTRRatio 	= Dsp_data[sid].m_xConfig.nTRRatio;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.nRTRatio 	= Dsp_data[sid].m_xConfig.nRTRatio;
	pSendParm[sid].xParm.xSetConfigParm.xConfig.bPLC 		= Dsp_data[sid].m_xConfig.bPLC;

	retval = SendCommand(sid);

	if(retval != DSPCODEC_SUCCESS)
	{
	}

}

void DspcodecDebugR1(uint32 sid)
{
	RESULT retval;
	int index=0, wait_cnt=0, length ;
	char * p_buf ;

	pSendParm[sid].uCommand = DSPCODEC_COMMAND_DEBUG;
	pSendParm[sid].nOwner = 0;
	retval = SendCommand(sid);

	if(retval != DSPCODEC_SUCCESS)
	{
	}

	while(1)
	{
		if(pRecvParm[sid].xEncOutput[index].nOwner == 1)
		{
			if(pRecvParm[sid].xEncOutput[index].nSize == 0)
				break ;
			else
			{
				length = pRecvParm[sid].xEncOutput[index].nSize;
				p_buf = (char*)(pRecvParm[sid].xEncOutput[index].pBuffer);
				pRecvParm[sid].xEncOutput[index].nOwner = 0;
				++index ;
				if(index == BUFFER_NUM_ENC)
					index = 0;
			}
			wait_cnt = 0 ;
		}
		if( ++wait_cnt > 1000 ) break ;
	}
}

#ifdef SUPPORT_DYNAMIC_PAYLOAD
int32 DPDspcodecWrite(uint32 chid, uint32 sid, uchar* pBuf, int32 nSize, uint32 nSeq, uint32 nTimestamp)
{
	int nLen,i,k;
	uint16 seq;
	uint32 timestamp;
	int TempLen = 0;		// length of voice packet
	int TempStamp = 0;
 #ifdef SUPPORT_APPENDIX_SID
	int TempLenSID = 4;		// length of SID packet (give a special value to prevent that we don't initalize this value in some codec.)
	int bAppendixSID = 0;
 #endif
 	uchar *pSrcBuf;
 #ifdef SUPPORT_ADJUST_JITTER
        extern unsigned short rx_frames_per_packet[MAX_SESS_NUM];
	extern void InitAdjustJitterOfJBC( uint32 ssid, unsigned short nRxFramePerPacket );
 #endif
	extern void AnnouncePacketRecivce_OnTime( uint32 sid );
	extern void AnnouncePacketRecivce_Slow( uint32 sid );
	extern void AnnouncePacketRecivce_Loss( uint32 sid );

 	const codec_algo_desc_t *pCurrentCodecDescriptor;
 	FN_GetFrameInfo_FrameLength pfnGetFrameLength;
 	int bVolatileFrameSize;
 	
	/*
	 * Step 1. According to codec algorithm, frame payload length, 
	 *         frame timestamp and SID payload length are given.
	 */
	pCurrentCodecDescriptor = ppNowCodecAlgorithmDesc[ sid ];
	
 	if( pCurrentCodecDescriptor == NULL ) {
 		printk("\n warning; (%d)codec algorithm is not defined, but for r1 writting!", sid );
		return 0;
	}
	
	pfnGetFrameLength = pCurrentCodecDescriptor ->fnGetR0FrameLength;
		
  #ifdef SUPPORT_APPENDIX_SID
	TempLenSID = ( *pCurrentCodecDescriptor ->fnGetR0SidLength )( nSize );
  #endif
  	TempLen = ( *pfnGetFrameLength )( pBuf );
  	bVolatileFrameSize = pCurrentCodecDescriptor ->bR0VolatileFrameLength;
  	TempStamp = pCurrentCodecDescriptor ->nR0FrameTimestamp;
	
	/*
	 * Step 2. Calculate number of frames in this packet.
	 */
	nLen = nSize;

	i = ( *pCurrentCodecDescriptor ->fnGetNumberOfFramesInCurrentPacket )
			( pBuf, nSize, TempLen, TempLenSID, &bAppendixSID );
 
	if(i>0)
		DPSize[sid] = i;

 #ifdef SUPPORT_ADJUST_JITTER
	if( rx_frames_per_packet[ sid ] < i )
		InitAdjustJitterOfJBC( sid, i );
 #endif // SUPPORT_ADJUST_JITTER

	/*
	 * Step 3. Write N frames into DSP codec.
	 */
#ifdef DYNAMIC_PAYLOAD_VER1	//tyhuang:111
	seq = DPInitSeq[sid];
	timestamp = DPInitStamp[sid];
	for(k=0;k<i;k++)
	{
		DspcodecWrite(chid, sid, pBuf+TempLen*k, TempLen, seq, timestamp);
		seq++;
		timestamp +=TempStamp;
	}
	DPInitSeq[sid] = seq;
	DPInitStamp[sid] = timestamp;
#else // ! DYNAMIC_PAYLOAD_VER1
	DPOutSeq[sid] = nSeq;
	timestamp = nTimestamp;
	seq = DPInSeq[sid] + 1;
	pSrcBuf = pBuf;
	if( DPNotFirstPacket[ sid ] == 0 ) {
		// sync. first packet or resume phone call.
		DPExpSeq[sid] = nSeq;
		DPNotFirstPacket[ sid ] = 1;
		
 #ifdef SUPPORT_SYNC_OUT_OF_SEQ
 		if( DPEnableSyncInSeq[ sid ] ) {
 			seq = DPInSeq[ sid ] = DPSyncInSeq[ sid ] + 1;
 			DPEnableSyncInSeq[ sid ] = 0;
 		}
 #endif

		//debug_dev( "First packet seq#(%d): %d\n", sid, nSeq );
	}
	
	if(DPExpSeq[sid] == nSeq) 	//arrival on time
	{
		for(k=0;k<i;k++)
		{
			if( bVolatileFrameSize )
				TempLen = ( *pfnGetFrameLength )( pSrcBuf );
		
			DspcodecWrite(chid, sid, pSrcBuf, TempLen, seq, timestamp);
			seq ++;
			timestamp +=TempStamp;
			pSrcBuf += TempLen;
		}
 #ifdef SUPPORT_APPENDIX_SID
 		if( bAppendixSID ) {
 			DspcodecWrite(chid, sid, pSrcBuf, TempLenSID, seq, timestamp);
			seq ++;
			timestamp +=TempStamp;
 		}
 #endif
		AnnouncePacketRecivce_OnTime( sid );
	}
	//else if(DPExpSeq[sid] > nSeq)			//slow (out of order)
	else if( RtpSeqGreater( DPExpSeq[sid], nSeq ) )	
	{
 #if 0
		//debug_dev("slow?\n");//drop it?
		return 0;
 #else
  #ifdef RESERVE_SPACE_FOR_SLOW
  		// assign correct seq#
   		seq = DPInSeq[sid] - ( DPExpTimeStamp[sid] - nTimestamp )/TempStamp + 1;
  #else
		//syn
		DPExpSeq[sid] = nSeq;
  #endif
  
		for(k=0;k<i;k++)
		{
			if( bVolatileFrameSize )
				TempLen = ( *pfnGetFrameLength )( pSrcBuf );
				
			DspcodecWrite(chid, sid, pSrcBuf, TempLen, seq, timestamp);
			seq ++;
			timestamp +=TempStamp;
			pSrcBuf += TempLen;
		}
  #ifdef SUPPORT_APPENDIX_SID
 		if( bAppendixSID ) {
 			DspcodecWrite(chid, sid, pSrcBuf, TempLenSID, seq, timestamp);
			seq ++;
			timestamp +=TempStamp;
 		}
  #endif
  		AnnouncePacketRecivce_Slow( sid );
  #ifdef RESERVE_SPACE_FOR_SLOW
  		// Don't update variables related to dynamic paylaod 
  		return nSize;
  #endif
 #endif	// if 0
	}
	else								//lose?
	{
		/*
		 * How to deal with packet loss after SID? JBC will solve it.
		 * We give a scenario to describe packet loss:
		 *      Pv  Psid	NTx		NTx		loss	Pv
		 * seq# 0	1						2		3
		 * time 0	80		(160)	(240)	320		400
		 *     << After Dynamic Payload >>
		 * seq#	0	1								?
		 * time	0	80								400 
		 * 				--> (400-80)/80=4 --> seq# = 5 ><
		 */
		uint32 lost_pkt;
		uint32 lost_payload;
		lost_pkt = nSeq - DPExpSeq[sid];	//lost packet no
		lost_payload = (nTimestamp - DPExpTimeStamp[sid])/TempStamp;	//lost payload
		//debug_warning("%d:L%d(%d, %d, %d) ",sid, lost_payload, lost_pkt, nSeq, DPExpSeq[sid]);
		debug_warning("%d:L%d ",sid, lost_pkt);
		seq += lost_payload;
		nRxRtpStatsLostPacket[ chid ] += lost_pkt;
		//debug_dev("DPSeq %d->%d\n", DPInSeq[sid], seq);
		for(k=0;k<i;k++)
		{
			if( bVolatileFrameSize )
				TempLen = ( *pfnGetFrameLength )( pSrcBuf );
				
			DspcodecWrite(chid, sid, pSrcBuf, TempLen, seq, timestamp);
			seq ++;
			timestamp +=TempStamp;
			pSrcBuf += TempLen;
		}
 #ifdef SUPPORT_APPENDIX_SID
 		if( bAppendixSID ) {
 			DspcodecWrite(chid, sid, pSrcBuf, TempLenSID, seq, timestamp);
			seq ++;
			timestamp +=TempStamp;
 		}
 #endif
 		AnnouncePacketRecivce_Loss( sid );
//	debug_dev("lost?\n");
	}
	DPInSeq[sid] = seq-1;
	DPExpSeq[sid] = nSeq + 1;
	DPExpTimeStamp[sid] = timestamp;// + TempStamp;
#endif // DYNAMIC_PAYLOAD_VER1

	return nSize;
}
#endif // SUPPORT_DYNAMIC_PAYLOAD

void DspcodecWriteSnyc( uint32 sid )
{
#if defined( SUPPORT_DYNAMIC_PAYLOAD ) && !defined( DYNAMIC_PAYLOAD_VER1 )
	DPNotFirstPacket[ sid ] = 0;
#endif // SUPPORT_DYNAMIC_PAYLOAD && !DYNAMIC_PAYLOAD_VER1
}

void DspcodecWriteSkipSeqNo( uint32 sid, uint32 nSeq )
{
	uint32 diff;

	//if( diff = ( nSeq - DPExpSeq[ sid ] ) )
	//	debug_dev( "LS:%d\n", diff );
	
	DPExpSeq[ sid ] = nSeq + 1;
}

#ifdef SUPPORT_TONE_PROFILE
RESULT DspcodecSetCustomTone(DSPCODEC_TONE nTone, ToneCfgParam_t *pToneCfg)
{
	uint32 idx = 0;
	ToneCfgParam_t *pToneTable = NULL;

	if((nTone < DSPCODEC_TONE_DIAL) || (nTone > DSPCODEC_TONE_INGRESS_RINGBACK))
		return DSPCODEC_ERROR_PARAMETER;
#if 1
	idx = nTone - DSPCODEC_TONE_DIAL;
	pToneTable = (ToneCfgParam_t *)&ToneTable[CUSTOM_DIAL+idx];
	pToneTable->ToneType = pToneCfg->ToneType;
	pToneTable->cycle = pToneCfg->cycle;
	pToneTable->cadNUM = pToneCfg->cadNUM;
	pToneTable->CadOn0 = pToneCfg->CadOn0;
	pToneTable->CadOff0 = pToneCfg->CadOff0;
	pToneTable->CadOn1 = pToneCfg->CadOn1;
	pToneTable->CadOff1 = pToneCfg->CadOff1;
	pToneTable->CadOn2 = pToneCfg->CadOn2;
	pToneTable->CadOff2 = pToneCfg->CadOff2;
	pToneTable->CadOn3 = pToneCfg->CadOn3;
	pToneTable->CadOff3 = pToneCfg->CadOff3;
	pToneTable->PatternOff = pToneCfg->PatternOff;
	pToneTable->ToneNUM = pToneCfg->ToneNUM;
	pToneTable->Freq0 = pToneCfg->Freq0;
	pToneTable->Freq1 = pToneCfg->Freq1;
	pToneTable->Freq2 = pToneCfg->Freq2;
	pToneTable->Freq3 = pToneCfg->Freq3;
	pToneTable->Gain0 = pToneCfg->Gain0;
	pToneTable->Gain1 = pToneCfg->Gain1;
	pToneTable->Gain2 = pToneCfg->Gain2;
	pToneTable->Gain3 = pToneCfg->Gain3;

#endif
	return DSPCODEC_SUCCESS;
}
#endif	/* #ifdef SUPPORT_TONE_PROFILE */

/*	handsome add function 2005.12.1     */
int32 setTone(aspToneCfgParam_t* pToneCfg)//thlin modify
{
#ifdef SUPPORT_TONE_PROFILE
	short *pToneTable = (short*)&ToneTable[CUSTOM_TONE1+cust];
	pToneTable[0]=pToneCfg->toneType;
	pToneTable[1]=pToneCfg->cycle;
	pToneTable[2]=pToneCfg->cadNUM;
	pToneTable[3]=pToneCfg->CadOn0;
	pToneTable[4]=pToneCfg->CadOff0;
	pToneTable[5]=pToneCfg->CadOn1;
	pToneTable[6]=pToneCfg->CadOff1;
	pToneTable[7]=pToneCfg->CadOn2;
	pToneTable[8]=pToneCfg->CadOff2;
	pToneTable[9]=pToneCfg->CadOn3;
	pToneTable[10]=pToneCfg->CadOff3;
	pToneTable[11]=pToneCfg->PatternOff;
	pToneTable[12]=pToneCfg->ToneNUM;
	pToneTable[13]=pToneCfg->Freq1;
	pToneTable[14]=pToneCfg->Freq2;
	pToneTable[15]=pToneCfg->Freq3;
	pToneTable[16]=pToneCfg->Freq4;
	pToneTable[17]=pToneCfg->Gain1;
	pToneTable[18]=pToneCfg->Gain2;
	pToneTable[19]=pToneCfg->Gain3;
	pToneTable[20]=pToneCfg->Gain4;
#if 0   //thlin
	if(cust == dialTone)
		DspcodecSetCustomTone(DSPCODEC_TONE_DIAL, (ToneCfgParam_t *)pToneTable);
	if(cust == ringTone)
		DspcodecSetCustomTone(DSPCODEC_TONE_RINGING, (ToneCfgParam_t *)pToneTable);
	if(cust == busyTone)
		DspcodecSetCustomTone(DSPCODEC_TONE_BUSY, (ToneCfgParam_t *)pToneTable);
	if(cust == waitingTone)
		DspcodecSetCustomTone(DSPCODEC_TONE_CALL_WAITING, (ToneCfgParam_t *)pToneTable);
#endif

#else
	ToneTable[CUSTOM_TONE1+cust][0]=pToneCfg->toneType;
	ToneTable[CUSTOM_TONE1+cust][1]=pToneCfg->cycle;
	ToneTable[CUSTOM_TONE1+cust][2]=pToneCfg->cadNUM;
	ToneTable[CUSTOM_TONE1+cust][3]=pToneCfg->CadOn0;
	ToneTable[CUSTOM_TONE1+cust][4]=pToneCfg->CadOn1;
	ToneTable[CUSTOM_TONE1+cust][5]=pToneCfg->CadOn2;
	ToneTable[CUSTOM_TONE1+cust][6]=pToneCfg->CadOn3;
	ToneTable[CUSTOM_TONE1+cust][7]=pToneCfg->CadOff0;
	ToneTable[CUSTOM_TONE1+cust][8]=pToneCfg->CadOff1;
	ToneTable[CUSTOM_TONE1+cust][9]=pToneCfg->CadOff2;
	ToneTable[CUSTOM_TONE1+cust][10]=pToneCfg->CadOff3;
	ToneTable[CUSTOM_TONE1+cust][11]=pToneCfg->PatternOff;
	ToneTable[CUSTOM_TONE1+cust][12]=pToneCfg->ToneNUM;
	ToneTable[CUSTOM_TONE1+cust][13]=pToneCfg->Gain1;
	ToneTable[CUSTOM_TONE1+cust][14]=pToneCfg->Gain2;
	ToneTable[CUSTOM_TONE1+cust][15]=pToneCfg->Gain3;
	ToneTable[CUSTOM_TONE1+cust][16]=pToneCfg->Gain4;
	ToneTable[CUSTOM_TONE1+cust][17]=pToneCfg->Freq1;
	ToneTable[CUSTOM_TONE1+cust][18]=pToneCfg->Freq2;
	ToneTable[CUSTOM_TONE1+cust][19]=pToneCfg->Freq3;
	ToneTable[CUSTOM_TONE1+cust][20]=pToneCfg->Freq4;
#endif
	return SUCCESS;
}

/*	handsome add function 2005.12.1     */
int32 setToneCountry(gDSP_aspToneCountry_t* pCfg)
{
	uint32 sid;
#ifdef SUPPORT_TONE_PROFILE
	short *pToneTable = NULL;
#endif

	iCountry = pCfg->CountryId;

	if(dialTone != pCfg->iDial)
	{
	dialTone = pCfg->iDial;
#ifdef SUPPORT_TONE_PROFILE
		pToneTable = (short*)&ToneTable[CUSTOM_TONE1+dialTone];
		DspcodecSetCustomTone(DSPCODEC_TONE_DIAL, (ToneCfgParam_t *)pToneTable);
#endif
	}
	if(ringTone != pCfg->iRing)
	{
 	ringTone = pCfg->iRing;
#ifdef SUPPORT_TONE_PROFILE
		pToneTable = (short*)&ToneTable[CUSTOM_TONE1+dialTone];
		DspcodecSetCustomTone(DSPCODEC_TONE_RINGING, (ToneCfgParam_t *)pToneTable);
#endif
	}
	if(busyTone != pCfg->iBusy)
	{
	busyTone = pCfg->iBusy;
#ifdef SUPPORT_TONE_PROFILE
		pToneTable = (short*)&ToneTable[CUSTOM_TONE1+dialTone];
		DspcodecSetCustomTone(DSPCODEC_TONE_BUSY, (ToneCfgParam_t *)pToneTable);
#endif
	}
	if(waitingTone != pCfg->iWaiting)
	{
	waitingTone = pCfg->iWaiting;
#ifdef SUPPORT_TONE_PROFILE
		pToneTable = (short*)&ToneTable[CUSTOM_TONE1+dialTone];
		DspcodecSetCustomTone(DSPCODEC_TONE_CALL_WAITING, (ToneCfgParam_t *)pToneTable);
#endif
	}

//#ifdef SUPPORT_TONE_PROFILE
	for(sid=0; sid<sess_num; sid++)
		DspcodecSetCountry(sid, iCountry);
//#endif

	return SUCCESS;
}

