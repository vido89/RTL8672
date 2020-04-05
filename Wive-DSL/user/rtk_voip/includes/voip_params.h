/**
 * @file voip_params.h
 * @brief VoIP control parameters
 */

#ifndef VOIP_PARAMS_H
#define VOIP_PARAMS_H

#include "rtk_voip.h"

/**
 * @ingroup VOIP_SESSION_RTP
 * Enumeration for supported rtp payload
 */
typedef enum
{
    rtpPayloadUndefined = -1,
    rtpPayloadPCMU = 0,
    rtpPayload1016 = 1,
    rtpPayloadG726_32 = 2,
    rtpPayloadGSM = 3,
    rtpPayloadG723 = 4,
    rtpPayloadDVI4_8KHz = 5,
    rtpPayloadDVI4_16KHz = 6,
    rtpPayloadLPC = 7,
    rtpPayloadPCMA = 8,
    rtpPayloadG722 = 9,
    rtpPayloadL16_stereo = 10,
    rtpPayloadL16_mono = 11,
    rtpPayloadQCELP = 12,
    rtpPayloadMPA = 14,
    rtpPayloadG728 = 15,
    rtpPayloadDVI4_11KHz = 16,
    rtpPayloadDVI4_22KHz = 17,
    rtpPayloadG729 = 18,
    rtpPayloadCN = 19,
    rtpPayloadG726_40    = 21,
    rtpPayloadG726_24    = 22,
    rtpPayloadG726_16    = 23,
    rtpPayloadCelB = 25,
    rtpPayloadJPEG = 26,
    rtpPayloadNV = 28,
    rtpPayloadH261 = 31,
    rtpPayloadMPV = 32,
    rtpPayloadMP2T = 33,
    rtpPayloadH263 = 34,
    // fake codec 
    rtpPayloadSilence = 35,
	// dynamic payload type in rtk_voip
	rtpPayload_iLBC = 97,
	rtpPayload_iLBC_20ms = 98,
    rtpPayloadDTMF_RFC2833 = 101,
    rtpPayloadT38_Virtual = 110,
    rtpPayloadCiscoRtp = 121,
    rtpPayloadL16_8k_mono = 122,
} RtpPayloadType;

/**
 * @ingroup VOIP_SESSION_RTP
 * Enumeration for rtp session state
 */
typedef enum
{
    rtp_session_undefined = -1,
    rtp_session_inactive = 0,
    rtp_session_sendonly = 1,
    rtp_session_recvonly = 2,
    rtp_session_sendrecv = 3
} RtpSessionState;

#ifdef AUDIOCODES_VOIP
// country tone set
typedef enum
{
	DSPCODEC_COUNTRY_USA,
	DSPCODEC_COUNTRY_UK,
	DSPCODEC_COUNTRY_AUSTRALIA,
	DSPCODEC_COUNTRY_HK,
	DSPCODEC_COUNTRY_JP,
	DSPCODEC_COUNTRY_SE,
	DSPCODEC_COUNTRY_GR,
	DSPCODEC_COUNTRY_FR,
	DSPCODEC_COUNTRY_TR,
	DSPCODEC_COUNTRY_BE,
	DSPCODEC_COUNTRY_FL,
	DSPCODEC_COUNTRY_IT,
	DSPCODEC_COUNTRY_CN,
	DSPCODEC_COUNTRY_CUSTOME
} DSPCODEC_COUNTRY;
#endif

/**
 * @ingroup VOIP_SESSION_TONE
 * Enumeration for which tone to play
 */
typedef enum
{
	DSPCODEC_TONE_NONE = -1,
	DSPCODEC_TONE_0 = 0,
	DSPCODEC_TONE_1,
	DSPCODEC_TONE_2,
	DSPCODEC_TONE_3,
	DSPCODEC_TONE_4,
	DSPCODEC_TONE_5,
	DSPCODEC_TONE_6,
	DSPCODEC_TONE_7,
	DSPCODEC_TONE_8,
	DSPCODEC_TONE_9,
	DSPCODEC_TONE_STARSIGN,				// *
	DSPCODEC_TONE_HASHSIGN,				// #

#ifdef SUPPORT_TONE_PROFILE

	DSPCODEC_TONE_DIAL,				// code = 66 in RFC2833
	DSPCODEC_TONE_STUTTERDIAL,			// 
	DSPCODEC_TONE_MESSAGE_WAITING,			// 
	DSPCODEC_TONE_CONFIRMATION,			// 
	DSPCODEC_TONE_RINGING, /* ring back tone */	// code = 70 in RFC2833
	DSPCODEC_TONE_BUSY,				// code = 72 in RFC2833
	DSPCODEC_TONE_CONGESTION,			// 
	DSPCODEC_TONE_ROH,				// 
	DSPCODEC_TONE_DOUBLE_RING,			// 
	DSPCODEC_TONE_SIT_NOCIRCUIT,			// 
	DSPCODEC_TONE_SIT_INTERCEPT,			// 
	DSPCODEC_TONE_SIT_VACANT,			// 
	DSPCODEC_TONE_SIT_REORDER,			// 
	DSPCODEC_TONE_CALLING_CARD_WITHEVENT,		// 
	DSPCODEC_TONE_CALLING_CARD,			// 
	DSPCODEC_TONE_CALL_WAITING,			// code = 79 in RFC2833
	DSPCODEC_TONE_CALL_WAITING_2,			// 
	DSPCODEC_TONE_CALL_WAITING_3,			// 
	DSPCODEC_TONE_CALL_WAITING_4,			// 
	DSPCODEC_TONE_INGRESS_RINGBACK,			// 

	DSPCODEC_TONE_HOLD,				// code = 76 in RFC2833
	DSPCODEC_TONE_OFFHOOKWARNING,			// code = 88 in RFC2833
	DSPCODEC_TONE_RING,				// code = 89 in RFC2833

#else
	DSPCODEC_TONE_DIAL,				// code = 66 in RFC2833
	DSPCODEC_TONE_RINGING,				// code = 70 in RFC2833
	DSPCODEC_TONE_BUSY,				// code = 72 in RFC2833
	DSPCODEC_TONE_HOLD,				// code = 76 in RFC2833

	DSPCODEC_TONE_CALL_WAITING,			// code = 79 in RFC2833
	DSPCODEC_TONE_OFFHOOKWARNING,			// code = 88 in RFC2833
	DSPCODEC_TONE_RING,				// code = 89 in RFC2833

#endif // #ifdef SUPPORT_TONE_PROFILE

#ifdef SW_DTMF_CID
	// hc+ 1124 for DTMF CID =================
	DSPCODEC_TONE_A,				// DTMF digit A (19)
	DSPCODEC_TONE_B,				// DTMF digit B (20)
	DSPCODEC_TONE_C,				// DTMF digit C (21)
	DSPCODEC_TONE_D,				// DTMF digit D (22)
	//=========================================
#endif
	// sandro+ 2006/07/24 for SAS tone
	DSPCODEC_TONE_FSK_SAS,				// alert signal (23)
	// hc+ 1229 for off hook FSK CID
	DSPCODEC_TONE_FSK_ALERT,			// alert signal (23)
	
	DSPCODEC_TONE_NTT_IIT_TONE,			// for ntt type 2 caller id		

	// thlin+ continous DTMF tone play for RFC2833
	DSPCODEC_TONE_0_CONT,				
	DSPCODEC_TONE_1_CONT,
	DSPCODEC_TONE_2_CONT,
	DSPCODEC_TONE_3_CONT,
	DSPCODEC_TONE_4_CONT,
	DSPCODEC_TONE_5_CONT,
	DSPCODEC_TONE_6_CONT,
	DSPCODEC_TONE_7_CONT,
	DSPCODEC_TONE_8_CONT,
	DSPCODEC_TONE_9_CONT,
	DSPCODEC_TONE_STARSIGN_CONT,			
	DSPCODEC_TONE_HASHSIGN_CONT,			
	DSPCODEC_TONE_A_CONT,				
	DSPCODEC_TONE_B_CONT,				
	DSPCODEC_TONE_C_CONT,				
	DSPCODEC_TONE_D_CONT,				

	DSPCODEC_TONE_KEY				// the others key tone

} DSPCODEC_TONE;

/**
 * @ingroup VOIP_SESSION_TONE
 * Enumeration for which direction to play tone
 */
typedef enum									
{
	DSPCODEC_TONEDIRECTION_LOCAL,			// local 
	DSPCODEC_TONEDIRECTION_REMOTE,			// remote 
	DSPCODEC_TONEDIRECTION_BOTH			// local and remote
} DSPCODEC_TONEDIRECTION;

/**
 * @ingroup VOIP_IVR
 * Play Types in IVR
 * We provide many types of input
 */
typedef enum {
	IVR_PLAY_TYPE_TEXT,		/* play a string, such as '192.168.0.0' */
	IVR_PLAY_TYPE_G723_63,	/* play a G723 6.3k data (24 bytes per 30 ms) */
	IVR_PLAY_TYPE_G729,		/* play a G729 data (10 bytes per 10ms) */
	IVR_PLAY_TYPE_G711A,	/* play a G711 a-law (80 bytes per 10ms) */
} IvrPlayType_t;

/**
 * @ingroup VOIP_IVR
 * IVR play direction (for playing TEXT only) 
 * One can play IVR in local or remote. 
 */
typedef enum {
	IVR_DIR_LOCAL,
	IVR_DIR_REMOTE,
} IvrPlayDir_t;

/*
 * @ingroup VOIP_IVR
 * Specified Text Type
 * Execpt to ASCII within 0~127, we define special speech above 128.
 */
enum {
	IVR_TEXT_ID_DHCP		= 128,
	IVR_TEXT_ID_FIX_IP,
	IVR_TEXT_ID_NO_RESOURCE,
	IVR_TEXT_ID_PLZ_ENTER_NUMBER,	
	///<<&&ID5&&>>	/* DON'T remove this line, it helps wizard to generate ID. */
	//IVR_TEXT_ID_xxx,	
};

#ifdef CONFIG_RTK_VOIP_IP_PHONE
/* We list keypad control in follow */
typedef enum {
	KEYPAD_CMD_SET_TARGET,
	KEYPAD_CMD_SIG_DEBUG,
	KEYPAD_CMD_READ_KEY,
	KEYPAD_CMD_HOOK_STATUS,
} keypad_cmd_t;

/* We list LCM control in follow */
typedef enum {
	LCM_CMD_DISPLAY_ON_OFF,
	LCM_CMD_MOVE_CURSOR_POS,
	LCM_CMD_DRAW_TEXT,
} lcm_cmd_t;

/* Control voice path */
typedef enum {
	VPATH_MIC1_SPEAKER,
	VPATH_MIC2_MONO,
	VPATH_SPEAKER_ONLY,
	VPATH_MONO_ONLY,
} vpath_t;
#endif /* CONFIG_RTK_VOIP_IP_PHONE */

typedef enum 
{
	VOIP_RESOURCE_UNAVAILABLE=0,
	VOIP_RESOURCE_AVAILABLE,
}Voip_reosurce_t;

#endif
