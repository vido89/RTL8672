/**
 * @file voip_manager.h
 * @brief VoIP control API
 */

#ifndef __VOIP_MANAGER_H
#define __VOIP_MANAGER_H

#include "voip_types.h"
#include "voip_flash.h"
#include "voip_params.h"
#include "voip_control.h"

#define MAX_SELF_TONE			8
#define MAX_CUSTOME				4

#define PHONE_ON_HOOK			0
#define PHONE_OFF_HOOK			1
#define PHONE_FLASH_HOOK		2
#define PHONE_STILL_ON_HOOK		3
#define PHONE_STILL_OFF_HOOK	4
#define PHONE_UNKNOWN			5

#define DAA_FLOW_NORMAL			0
#define DAA_FLOW_3WAY_CONFERENCE	1
#define DAA_FLOW_CALL_FORWARD		2

#define DAA_CH	2

/**
 * @ingroup VOIP_PHONE
 * Enumeration for phone state
 */
typedef enum
{
    SIGN_NONE = 0 ,
    SIGN_KEY1 = 1 ,
    SIGN_KEY2 = 2 ,
    SIGN_KEY3 = 3 ,
    SIGN_KEY4 = 4,
    SIGN_KEY5 = 5 ,
    SIGN_KEY6 = 6 ,
    SIGN_KEY7 = 7 ,
    SIGN_KEY8 = 8 ,
    SIGN_KEY9 = 9 ,
    SIGN_KEY0 = 10 ,
    SIGN_STAR = 11,
    SIGN_HASH = 12 ,
    SIGN_ONHOOK = 13 ,
    SIGN_OFFHOOK = 18,
    SIGN_FLASHHOOK = 19,
    SIGN_AUTODIAL = 20,
    SIGN_OFFHOOK_2 = 21,
    SIGN_RING_ON = 22,
    SIGN_RING_OFF = 23
} SIGNSTATE ;

/**
 * @ingroup VOIP_SESSION_FAX
 * Enumeration for fax state
 */
typedef enum
{
    FAX_IDLE = 0,
    FAX_DETECT ,
    MODEM_DETECT,
    FAX_RUN,
    MODEM_RUN
} FAXSTATE;

/**
 * @ingroup VOIP_SESSION_RTP
 * @brief Structure for RTP handling
 */
typedef struct
{
    uint32 chid;        ///< channel id 
    uint32 sid;  		///< session id
    uint32 isTcp;       ///< tcp = 1 or udp = 0
    uint32 remIp;       ///< remote ip
    uint16 remPort;		///< remote port
    uint32 extIp;       ///< local ip
    uint16 extPort;		///< local port
    uint8 tos;		///< QoS
	uint16 rfc2833_payload_type_local;	///< rfc2833 payload type 
	uint16 rfc2833_payload_type_remote;	///< remote rfc2833 payload type
} rtp_config_t;

/**
 * @ingroup VOIP_SESSION_RTP
 * @brief Structure for RTP payload type
 * @sa __G7231_RATE__
 */
typedef struct
{
    uint32 chid;					///< channel id
    uint32 sid;						///< session id
    RtpPayloadType uPktFormat;		///< payload type in rtk_voip
	RtpPayloadType local_pt;		///< payload type in local 
	RtpPayloadType remote_pt;		///< payload type in remote answer
    int32 nG723Type;				///< see __G7231_RATE__
    int32 nFramePerPacket;			// reserved
    int32 bVAD;                     ///< Vad on: 1, off: 0
    uint32 nJitterDelay;			///< jitter delay
    uint32 nMaxDelay;				///< max delay
    uint32 nJitterFactor;			///< optimzation factor of jitter buffer 
} payloadtype_config_t;

/** 
 * @ingroup VOIP_DSP
 * @brief Get VoIP Feature
 * @retval 0 Success
 */
int32 rtk_Get_VoIP_Feature(void);

/**
 * @ingroup VOIP_DSP
 * @brief Interface Initialization
 * @param ch The channel number.
 * @retval 0 Success
 */
int32 rtk_InitDSP(int ch);

/**
 * @ingroup VOIP_DSP
 * @brief Set echo tail for LEC
 * @param chid The channel number.
 * @param echo_tail The echo tail length (ms). 
 * @retval 0 Success
 * @note 1~16ms in G.711, 1~4ms in G.729/G.723 
 */
int32 rtk_Set_echoTail(uint32 chid, uint32 echo_tail);

/**
 * @ingroup VOIP_DSP
 * @brief Light LED if SIP Register OK
 * @param chid The channel number.
 * @param isOK 1: SIP register OK, 0: SIP register failed
 * @retval 0 Success
 */
int rtk_sip_register(unsigned int chid, unsigned int isOK);

/**
 * @ingroup VOIP_PHONE
 * @brief Get the FXS state
 * @param chid The channel number.
 * @param pval The phone state.
 * @retval 0 Success
 * @sa rtk_Offhook_Action, rtk_Onhook_Action
 */  
int32 rtk_GetFxsEvent(uint32 chid, SIGNSTATE *pval);

/**
 * @ingroup VOIP_PHONE
 * @brief Get the FXO state
 * @param chid The channel number.
 * @param pval The FXO state.
 * @retval 0 Success
 * @sa rtk_Offhook_Action, rtk_Onhook_Action
 */ 
int32 rtk_GetFxoEvent(uint32 chid, SIGNSTATE *pval);

/** 
 * @ingroup VOIP_FXS
 * @brief Setup the flash hook time
 * @param chid The channel number.
 * @param min_time The min time period for flash hook.
 * @param time The max time period for flash hook.
 * @retval 0 Success
 */
int32 rtk_Set_Flash_Hook_Time(uint32 chid, uint32 min_time, uint32 time);

/**
 * @ingroup VOIP_FXS_RING
 * @brief Enable/Diable FXS Ringing 
 * @param chid The FXS channel number.
 * @param bRinging 1: enable ring, 0: disable ring
 * @retval 0 Success
 */  
int32 rtk_SetRingFXS(uint32 chid, uint32 bRinging);

/*
 * @ingroup VOIP_FXS_RING
 * @brief Disable all FXS Ring
 * @param bDisable flag to disable all FXS Ring
 * @retval 0 Success
 * @note Test Issue.
 */  
int32 rtk_DisableRingFXS(int bDisable);

/**
 * @ingroup VOIP_FXS_RING
 * @brief Set the Ring Cadence of FXS
 * @param chid The FXS channel number.
 * @param cad_on_msec The time period of Cadence-On.
 * @param cad_off_msec The time period of Cadence-Off.
 * @retval 0 Success
 */  
int32 rtk_Set_SLIC_Ring_Cadence(uint32 chid, uint16 cad_on_msec, uint16 cad_off_msec);

/**
 * @ingroup VOIP_FXS_CALLERID
 * @brief Caller ID generation via DTMF
 * @param chid The FXS channel number.
 * @param str_cid The Caller ID
 * @retval 0 Success
 */  
int32 rtk_Gen_Dtmf_CID(uint32 chid, char *str_cid);

/**
 * @ingroup VOIP_FXS_CALLERID
 * @brief Set the DTMF Caller ID Generation Mode
 * @param chid The FXS channel number.
 * @param area set the before ring or after ring send cid
 * @param cid_dtmf_mode set the caller id start/end digit
 * @retval 0 Success
 */
int32 rtk_Set_CID_DTMF_MODE(uint32 chid, char area, char cid_dtmf_mode);

/**
 * @ingroup VOIP_FXS_CALLERID
 * @brief Caller ID generation via FSK
 * @param chid The FXS channel number.
 * @param str_cid The Caller ID
 * @param str_date The Date
 * @param str_cid_name The Caller ID Name
 * @param mode 0: type I, 1: type II
 * @retval 0 Success
 */  
int32 rtk_Gen_FSK_CID(uint32 chid, char *str_cid, char *str_date, char *str_cid_name, char mode);

/**
 * @ingroup VOIP_FXS_CALLERID
 * @brief Set the FSK Area
 * @param chid The FXS channel number.
 * @param area The area of FSK. (see _CID_AREA_),
 *                  bit 0, 1: Area -> 0: Bellcore, 1: ETSI, 2: BT, 3: NTT 
 *                  bit 3: Caller ID Prior First Ring, 
 *                  bit 4: Dual Tone before Caller ID (Fsk Alert Tone),
 *					bit 5: Short Ring before Caller ID, 
 *                  bit 6: Reverse Polarity before Caller ID (Line Reverse),
 *					bit 7: FSK Date & Time Sync and Display Name
 * @retval 0 Success
 * @sa _CID_AREA_
 */
int32 rtk_Set_FSK_Area(uint32 chid, char area);

/**
 * @ingroup VOIP_FXS_CALLERID
 * @brief get the fsk caller id send complete or not.
 * @param chid The FXS channel number.
 * @param cid_state 0:fsk cid send complete 1:sending fsk cid
 */
int32 rtk_GetFskCIDState(uint32 chid, uint32 *cid_state);

/**
 * @ingroup VOIP_FXS_CALLERID
 * @brief Set the FSK Caller ID Generation Mode,
 * @param chid The FXS channel number.
 * @param isOK 0: hardward gen fsk caller id, 1: software gen fsk caller id
 * @retval 0 Success
 */
int32 rtk_Set_CID_FSK_GEN_MODE(unsigned int chid, unsigned int isOK);

/**
 * @ingroup VOIP_FXS_CALLERID
 * @brief Generate the VMWI via FSK
 * @param chid The FXS channel number.
 * @param state The address of value to set VMWI state. (0: off, 1: on)
 * @param mode 0: type I, 1: type II
 * @retval 0 Success
 */  
int32 rtk_Gen_FSK_VMWI(uint32 chid, char *state, char mode);

/**
 * @ingroup VOIP_FXS_CALLERID
 * @brief Caller ID and VMWI generation via FSK
 * @param chid The FXS channel number.
 * @param str_cid The Caller ID or VMWI
 * @param str_date The Date
 * @param str_cid_name The Caller ID Name
 * @param mode 0: type I, 1: type II
 * @param msg_type FSK_MSG_CALLSETUP:cid or FSK_MSG_MWSETUP:vmwi
 * @retval 0 Success
 */
int32 rtk_Gen_FSK_CID_VMWI(uint32 chid, char *str_cid, char *str_date, char *str_cid_name, char mode, char msg_type);

/**
 * @ingroup VOIP_DSP
 * @brief Set the speaker and mic voice gain
 * @param chid The channel number.
 * @param spk_gain -32~31 dB (-32: mute, default is 0 dB)
 * @param mic_gain -32~31 dB (-32: mute, default is 0 dB)
 * @retval 0 Success
 */
int32 rtk_Set_Voice_Gain(uint32 chid, int spk_gain, int mic_gain);

/**
 * @ingroup VOIP_DSP
 * @brief rtk_Set_SPK_AGC
 * @param chid The channel number.
 * @param support_gain support_gain
 * @retval 0 Success
 */
int32 rtk_Set_SPK_AGC(uint32 chid, uint32 support_gain);

/**
 * @ingroup VOIP_DSP
 * @brief rtk_Set_SPK_AGC_LVL
 * @param chid The channel number.
 * @param level level
 * @retval 0 Success
 */
int32 rtk_Set_SPK_AGC_LVL(uint32 chid, uint32 level);

/**
 * @ingroup VOIP_DSP
 * @brief rtk_Set_SPK_AGC_GUP
 * @param chid The channel number.
 * @param gain gain
 * @retval 0 Success
 */
int32 rtk_Set_SPK_AGC_GUP(uint32 chid, uint32 gain);

/**
 * @ingroup VOIP_DSP
 * @brief rtk_Set_SPK_AGC_GDOWN
 * @param chid The channel number.
 * @param gain gain
 * @retval 0 Success
 */
int32 rtk_Set_SPK_AGC_GDOWN(uint32 chid, uint32 gain);

/**
 * @ingroup VOIP_DSP
 * @brief rtk_Set_MIC_AGC
 * @param chid The channel number.
 * @param support_gain support_gain
 * @retval 0 Success
 */
int32 rtk_Set_MIC_AGC(uint32 chid, uint32 support_gain);

/**
 * @ingroup VOIP_DSP
 * @brief rtk_Set_MIC_AGC_LVL
 * @param chid The channel number.
 * @param level level
 * @retval 0 Success
 */
int32 rtk_Set_MIC_AGC_LVL(uint32 chid, uint32 level);

/**
 * @ingroup VOIP_DSP
 * @brief rtk_Set_MIC_AGC_GUP
 * @param chid The channel number.
 * @param gain gain
 * @retval 0 Success
 */
int32 rtk_Set_MIC_AGC_GUP(uint32 chid, uint32 gain);

/**
 * @ingroup VOIP_DSP
 * @brief rtk_Set_MIC_AGC_GDOWN
 * @param chid The channel number.
 * @param gain gain
 * @retval 0 Success
 */
int32 rtk_Set_MIC_AGC_GDOWN(uint32 chid, uint32 gain);

/**
 * @ingroup VOIP_FXO
 * @param chid The FXO channel number
 * @brief Off-Hook in PSTN line
 * @retval 0 Success
 */
int rtk_DAA_off_hook(uint32 chid);

/* Obsolete API (replaced by rtk_GetFxoEvent)
 * @ingroup VOIP_FXO
 * @param chid The specified FXS channel for FXO
 * @brief Check Ringing via PSTN line
 * @retval 1 PSTN Ringing
 */
int rtk_DAA_ring(uint32 chid);

/**
 * @ingroup VOIP_FXO
 * @param chid The FXO channel number
 * @brief On-Hook in PSTN line
 * @retval 0 Success
 */
int rtk_DAA_on_hook(uint32 chid);

/* Obsolete API (replaced by rtk_Set_Voice_Gain)
 * @ingroup VOIP_FXO_GAIN
 * @brief Set the Tx Gain of FXO
 * @param gain The gain value (1..10).
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int32 rtk_Set_DAA_Tx_Gain(uint32 gain);

/* Obsolete API (replaced by rtk_Set_Voice_Gain)
 * @ingroup VOIP_FXO_GAIN
 * @brief Set the Rx Gain of FXO
 * @param gain The gain value (1..10).
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int32 rtk_Set_DAA_Rx_Gain(uint32 gain);

/** 
 * @ingroup VOIP_FXO_CALLERID
 * @brief Get the FXO Detected Caller ID
 * @param chid The FXO channel number
 * @param str_cid The Caller ID String
 * @param str_date The Date String
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int32 rtk_Get_DAA_CallerID(uint32 chid, char *str_cid, char *str_date);

/** 
 * @ingroup VOIP_FXO_CALLERID
 * @brief Set the FXO Caller ID Detection Mode
 * @param chid The FXO channel number
 * @param auto_det 0: Disable Caller ID Auto Detection,
 *                 1: Enable Caller ID Auto Detection (NTT Support),
 *                 2: Enable Caller ID Auto Detection (NTT Not Support)
 * @param cid_det_mode  The Caller ID Mode for Caller ID Detection
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int32 rtk_Set_CID_Det_Mode(uint32 chid, int auto_det, int cid_det_mode);

/* Obsolete API
 * @ingroup VOIP_FXO
 * @brief Set the DAA ISR Flow
 * @param chid The channel number
 * @param sid The session number
 * @param flow The DAA ISR Flow => 0: Normal, 1: PSTN 3-way conference, 2: PSTN call forward
 * @retval 1: success
 * @retval 0xff: line not connect or busy or not support
 * @note Virtual DAA not support
 */
int32 rtk_Set_DAA_ISR_FLOW(unsigned int chid, unsigned int sid, unsigned int flow);

/* Obsolete API
 * @ingroup VOIP_FXO
 * @brief Get the DAA ISR Flow
 * @param chid The channel number
 * @param sid The session number
 * @retval 0: Normal
 * @retval 1: PSTN 3-way conference
 * @retval 2: PSTN call forward
 * @note Virtual DAA not support
 */
int32 rtk_Get_DAA_ISR_FLOW(unsigned int chid, unsigned int sid);

/** 
 * @ingroup VOIP_FXO
 * @brief Dial PSTN Number for PSTN Call Forward
 * @param chid The FXO channel number
 * @param sid  The seesion number
 * @param *cf_no_str The Pointer of the PSTN Number String
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int32 rtk_Dial_PSTN_Call_Forward(uint32 chid, uint32 sid, char *cf_no_str);

/* Obsolete API
 * @ingroup VOIP_FXO
 * @brief (Un)Hold PSTN Line
 * @param slic_chid The slic channel number
 * @param daa_chid The daa channel number
 * @param config  The configuration of Hold(config=1) or Un-Hold(config=0)
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int32 rtk_Set_PSTN_HOLD_CFG(unsigned int slic_chid, unsigned int daa_chid, unsigned int config);

/* Obsolete API (replaced by rtk_GetFxoEvent)
 * @ingroup VOIP_FXO
 * @brief Get if DAA detect the busy tone
 * @param daa_chid The daa channel number
 * @retval busy_flag Busy tone is  1: Detected, 0: NOT detected
 * @note Virtual DAA not support
 */
int32 rtk_Get_DAA_BusyTone_Status(unsigned int daa_chid);

/* Obsolete API
 * @ingroup VOIP_FXO
 * @brief Check which FXS channel is replaced by FXO 
 * @param chid The FXS channel number
 * @retval 1: TRUE, 0: FALSE
 * @note Virtual DAA not support
 */
int rtk_Get_DAA_Used_By_Which_SLIC(uint32 chid);

/**
 * @ingroup VOIP_SESSION
 * @brief Assign the active session.
 * @param chid The channel number.
 * @param sid The session number.
 * @retval 0 Success
 */  
int32 rtk_SetTranSessionID(uint32 chid, uint32 sid);

/*
 * @ingroup VOIP_SESSION
 * @brief Get active session by channel number
 * @param chid The specified hannel number.
 * @param psid The session number.
 * @retval 0 Success
 */  
int32 rtk_GetTranSessionID(uint32 chid, uint32* psid);

/**
 * @ingroup VOIP_SESSION_TONE
 * @brief Play/Stop the assigned Tone 
 * @param chid The channel number.
 * @param sid The session number.
 * @param nTone The tone type.
 * @param bFlag 1: play, 0: stop
 * @param Path The tone direction.
 * @retval 0 Success
 */  
int32 rtk_SetPlayTone(uint32 chid, uint32 sid, DSPCODEC_TONE nTone, uint32 bFlag, 
	DSPCODEC_TONEDIRECTION Path);

/**
 * @ingroup VOIP_SESSION_TONE
 * @brief Set the Tone of Country
 * @param voip_ptr The configuration of VoIP.
 * @retval 0 Success
 * @note The parameter is depend on the flash layout.
 */
int32 rtk_Set_Tone_Country(voipCfgParam_t *voip_ptr);

/**
 * @ingroup VOIP_SESSION_TONE
 * @brief Set the custom tone
 * @param custom The custom index.
 * @param pstToneCfgParam The custom tone configuration.
 * @retval 0 Success
 */  
int32 rtk_Set_Custom_Tone(uint8 custom, st_ToneCfgParam *pstToneCfgParam);

/**
 * @ingroup VOIP_SESSION_RTP
 * @brief Set RTP configuration
 * @param cfg The RTP configuration.
 * @retval 0 Success
 */
int32 rtk_SetRtpConfig(rtp_config_t* cfg);

/**
 * @ingroup VOIP_SESSION_RTP
 * @brief Set RTCP configuration
 * @param cfg The RTCP configuration.
 * @param rtcp_tx_interval The RTCP TX Interval
 * @retval 0 Success
 */
int32 rtk_SetRtcpConfig(rtp_config_t *cfg, unsigned short rtcp_tx_interval);

/**
 * @ingroup VOIP_SESSION_RTP
 * @brief Set RTP payload type
 * @param cfg The RTP payload configuration.
 * @retval 0 Success
 */  
int32 rtk_SetRtpPayloadType(payloadtype_config_t* cfg);

/**
 * @ingroup VOIP_SESSION_RTP
 * @brief Set RTP Session State
 * @param chid The channel number.
 * @param sid The session number.
 * @param state The RTP Session State.
 * @retval 0 Success
 */  
int32 rtk_SetRtpSessionState(uint32 chid, uint32 sid, RtpSessionState state);

/**
 * @ingroup VOIP_SESSION_RTP
 * @brief Enable/Disable conference.
 * @param stVoipMgr3WayCfg The conference setting
 * @retval 0 Success
 */  
int32 rtk_SetConference(TstVoipMgr3WayCfg *stVoipMgr3WayCfg);

/**
 * @ingroup VOIP_SESSION_RTP
 * @brief Hold/Resume RTP
 * @param chid The channel number.
 * @param sid The session number.
 * @param enable 1: Hold, 0: Resume
 * @retval 0 Success
 */  
int32 rtk_Hold_Rtp(uint32 chid, uint32 sid, uint32 enable);

/**
 * @ingroup VOIP_SESSION_RTP
 * @brief Get RTP statistics by channel. 
 * @param chid The channel number.
 * @param bReset Reset statistics of the channel. 
 * @param pstVoipRtpStatistics RTP statistics. (If bReset is set, statistics are all zeros.)
 */
int32 rtk_Get_Rtp_Statistics( uint32 chid, uint32 bReset, TstVoipRtpStatistics *pstVoipRtpStatistics );

/**
 * @ingroup VOIP_SESSION_DTMF
 * @brief Set the DTMF mode
 * @param chid The channel number.
 * @param mode The DTMF mode. (see __DTMF_TYPE__)
 * @retval 0 Success
 * @sa __DTMF_TYPE__ 
 */  
int32 rtk_SetDTMFMODE(uint32 chid, uint32 mode);

/**
 * @ingroup VOIP_SESSION_DTMF
 * @brief Send DTMF via RFC2833 
 * @param chid The channel number.
 * @param sid The session number.
 * @param digit The digit of user input. (0..11, means 0..9,*,#)
 * @retval 0 Success
 */  
int32 rtk_SetRTPRFC2833(uint32 chid, uint32 sid, uint32 digit);

/** 
 * @ingroup VOIP_SESSION_DTMF
 * @brief Play tone when receive SIP INFO
 * @param chid The channel number
 * @param sid The session number
 * @param tone The tone need to play
 * @param duration The tone duration (ms)
 * @retval 0 Success
 */
int32 rtk_SIP_INFO_play_tone(unsigned int chid, unsigned int sid, DSPCODEC_TONE tone, unsigned int duration);

/**
 * @ingroup VOIP_SESSION_FAX
 * @brief Fax/Modem detection
 * @param chid The channel number.
 * @param pval The fax state.
 * @retval 0 Success
 */  
int32 rtk_GetCEDToneDetect(uint32 chid, FAXSTATE *pval);

/** 
 * @ingroup VOIP_IVR
 * @brief Play a text speech
 * @param chid The channel number.
 * @param pszText2Speech The text to speech.
 * @retval Playing interval in unit of 10ms.
 */
int rtk_IvrStartPlaying( unsigned int chid, unsigned char *pszText2Speech );

/** 
 * @ingroup VOIP_IVR
 * @brief Play a G.723 6.3k voice 
 * @param chid The channel number.
 * @param nFrameCount The number of frame to be decoded. 
 * @param pData Point to data. 
 * @retval Copied frames, so less or equal to nFrameCount. 
 */
int rtk_IvrStartPlayG72363( unsigned int chid, unsigned int nFrameCount, const unsigned char *pData );

/** 
 * @ingroup VOIP_IVR
 * @brief Poll whether it is playing or not
 * @param chid The channel number.
 * @retval 1 Playing
 * @retval 0 Stopped 
 */
int rtk_IvrPollPlaying( unsigned int chid );

/** 
 * @ingroup VOIP_IVR
 * @brief Stop playing immediately
 * @param chid The channel number.
 * @retval 0 Success
 */
int rtk_IvrStopPlaying( unsigned int chid );

/** 
 * @ingroup VOIP_PHONE
 * @brief OffHook Action function. 
 * @param chid The channel number.
 * @retval 0 Success
 * @Note Call it always after Off-Hook action start
 * @sa rtk_GetFxsEvent, rtk_GetFxoEvent
 */
int32 rtk_Offhook_Action(uint32 chid);

/** 
 * @ingroup VOIP_PHONE
 * @brief OnHook Action function. 
 * @param chid The channel number.
 * @retval 0 Success
 * @Note Call it always before On-Hook action done
 * @sa rtk_GetFxsEvent, rtk_GetFxoEvent
 */
int32 rtk_Onhook_Action(uint32 chid);

int32 rtk_Set_DSCP(int32 sip_rtp_dscp);

#ifdef CONFIG_RTK_VOIP_WAN_VLAN
/**
 * @ingroup VOIP_VLAN
 * @brief setup 3 VLANs for Voice, Data, and Video traffic. Note: Traffic from/to Video Port (Port 3) are tagged with Video VLAN TAG, Traffic from/to LAN/WLAN are tagged with Data VLAN TAG, and other traffic are tagged with Voice VLAN TAG.
 * @param voip_ptr voip configuration
 * @retval 0 Success
 */
int rtk_switch_wan_3_vlan(voipCfgParam_t *voip_ptr);

/**
 * @ingroup VOIP_VLAN
 * @brief setup 2 VLANs for Voice and Data traffic. Note: Traffic from/to LAN/WLAN are tagged with Data VLAN TAG, and other traffic are tagged with Voice VLAN TAG.
 * @param voip_ptr voip configuration
 * @retval 0 Success
 */
int rtk_switch_wan_2_vlan(voipCfgParam_t *voip_ptr);

/*
 * internal use
 * @brief rtk_switch_wan_vlan
 * @param voip_ptr voip configuration
 * @retval 0 Success
 */
int rtk_switch_wan_vlan(voipCfgParam_t *voip_ptr);
#endif // CONFIG_RTK_VOIP_WAN_VLAN

/*
 * internal use
 * @brief rtk_8305_switch
 * @param phy phy
 * @param reg reg
 * @param value value
 * @param r_w r_w
 * @retval 0 Success
 */
int rtk_8305_switch(unsigned short phy,unsigned short reg,unsigned short value,unsigned short r_w);

/*
 * internal use
 */
int32 rtk_enable_pcm(uint32 chid, int32 bEnable);

/**
 * @ingroup VOIP_IP_PHONE
 * @brief Get hook status of IP Phone
 * @param pHookStatus Hook status. 1: on-hook, 0: off-hook
 */
extern int rtk_GetIPPhoneHookStatus( uint32 *pHookStatus );

/**
* @ingroup VOIP_RESOURCE_CHECK
* @brief Get the VoIP Middleware Resource status
* @param stVoipMgr3WayCfg 3way conf session info
* @retval 1: resource available, 0: resource un-available
*/
unsigned short rtk_VoIP_resource_check(TstVoipMgr3WayCfg *stVoipMgr3WayCfg);

/**
 * @ingroup VOIP_PHONE
 * @brief Get the phone state only (don't read fifo)
 * @param pstVoipCfg The config
 * @retval 0 Success
 * @sa rtk_GetFxsEvent
 */  
int32 rtk_Set_GetPhoneStat(TstVoipCfg* pstVoipCfg);

/**
 * @ingroup VOIP_PHONE
 * @brief Flush the VoIP kernel used fifo.
 * @param chid The channel number.
 * @retval 0 Success
 */
int rtk_Set_flush_fifo(uint32 chid);
 
/**
 * @ingroup VOIP_PHONE
 * @brief check line status
 * @param chid The channel number.
 * @retval 0 Success
 */
int rtk_line_check(uint32 chid);

/**
 * @ingroup VOIP_FXO
 * @brief FXO off-hook
 * @param chid The FXO channel number.
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int rtk_FXO_offhook(uint32 chid);

/**
 * @ingroup VOIP_FXO
 * @brief FXO on-hook
 * @param chid The FXO channel number.
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int rtk_FXO_onhook(uint32 chid);

/**
 * @ingroup VOIP_FXO
 * @brief FXO Ring
 * @param chid The FXO channel number.
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int rtk_FXO_RingOn(uint32 chid);

/**
 * @ingroup VOIP_FXO
 * @brief FXO Busy
 * @param chid The FXO channel number.
 * @retval 0 Success
 * @note Virtual DAA not support
 */
int rtk_FXO_Busy(uint32 chid);

/**
 * @ingroup VOIP_DSP
 * @brief The variables contaions the VoIP Feature
 * @note call rtk_Get_VoIP_Feature first
 */
extern int g_VoIP_Feature;

#endif // __VOIP_MANAGER_H
