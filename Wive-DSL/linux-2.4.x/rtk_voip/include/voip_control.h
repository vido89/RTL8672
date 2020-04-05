/**
 * @file voip_control.h
 * @brief VoIP control structure
 */

#ifndef VOIP_CONTROL_H
#define VOIP_CONTROL_H

#include "voip_params.h"

#define VOIP_MGR_BASE_CTL				64+1024+64+64+900

#define VOIP_MGR_SET_EBL				VOIP_MGR_BASE_CTL+1
#define VOIP_MGR_SET_SESSION 				VOIP_MGR_BASE_CTL+2
#define VOIP_MGR_INIT_GNET				VOIP_MGR_BASE_CTL+3
#define VOIP_MGR_LOOPBACK 				VOIP_MGR_BASE_CTL+4
#define VOIP_MGR_GNET					VOIP_MGR_BASE_CTL+5
#define VOIP_MGR_SIGNALTEST				VOIP_MGR_BASE_CTL+6
#define VOIP_MGR_SETRTPPAYLOADTYPE			VOIP_MGR_BASE_CTL+7
#define VOIP_MGR_SETRTPSESSIONSTATE			VOIP_MGR_BASE_CTL+8
#define VOIP_MGR_SETPLAYTONE				VOIP_MGR_BASE_CTL+9
#define VOIP_MGR_DSPSETCONFIG				VOIP_MGR_BASE_CTL+10
#define VOIP_MGR_DSPCODECSTART 				VOIP_MGR_BASE_CTL+11
#define VOIP_MGR_PCM_CFG				VOIP_MGR_BASE_CTL+12
#define VOIP_MGR_RTP_CFG				VOIP_MGR_BASE_CTL+13
#define VOIP_MGR_INIT_GNET2				VOIP_MGR_BASE_CTL+14
#define VOIP_MGR_UNSET_SESSION				VOIP_MGR_BASE_CTL+15
#define VOIP_MGR_DTMF_CFG				VOIP_MGR_BASE_CTL+16
#define VOIP_MGR_DTMF_GET				VOIP_MGR_BASE_CTL+17
#define VOIP_MGR_SLIC_HOOK				VOIP_MGR_BASE_CTL+18
#define VOIP_MGR_SLIC_RING				VOIP_MGR_BASE_CTL+19
#define VOIP_MGR_SLIC_TONE				VOIP_MGR_BASE_CTL+20
#define VOIP_MGR_DSPCODECSTOP				VOIP_MGR_BASE_CTL+21
#define VOIP_MGR_SLIC_RESTART				VOIP_MGR_BASE_CTL+22
#define VOIP_MGR_HOLD					VOIP_MGR_BASE_CTL+23
#define VOIP_MGR_CTRL_RTPSESSION 			VOIP_MGR_BASE_CTL+24
#define VOIP_MGR_CTRL_TRANSESSION_ID			VOIP_MGR_BASE_CTL+25
#define VOIP_MGR_SETCONFERENCE 				VOIP_MGR_BASE_CTL+26
#define VOIP_MGR_ON_HOOK_RE_INIT			VOIP_MGR_BASE_CTL+27
#define VOIP_MGR_FAX_MODEM_PASS_CFG			VOIP_MGR_BASE_CTL+28
#define VOIP_MGR_DTMF_CID_GEN_CFG			VOIP_MGR_BASE_CTL+29
#define VOIP_MGR_GET_CID_STATE_CFG			VOIP_MGR_BASE_CTL+30
#define VOIP_MGR_SET_DTMF_MODE				VOIP_MGR_BASE_CTL+31
#define VOIP_MGR_FAX_OFFHOOK				VOIP_MGR_BASE_CTL+32
#define VOIP_MGR_FSK_CID_GEN_CFG			VOIP_MGR_BASE_CTL+33
#define VOIP_MGR_SEND_RFC2833_PKT_CFG			VOIP_MGR_BASE_CTL+34
#define VOIP_MGR_GET_SLIC_REG_VAL			VOIP_MGR_BASE_CTL+35
#define VOIP_MGR_SET_ECHO_TAIL_LENGTH			VOIP_MGR_BASE_CTL+36
#define VOIP_MGR_SET_SLIC_TX_GAIN			VOIP_MGR_BASE_CTL+37
#define VOIP_MGR_SET_SLIC_RX_GAIN			VOIP_MGR_BASE_CTL+38
#define VOIP_MGR_DAA_RING				VOIP_MGR_BASE_CTL+39
#define VOIP_MGR_DAA_OFF_HOOK				VOIP_MGR_BASE_CTL+40
#define VOIP_MGR_DAA_ON_HOOK				VOIP_MGR_BASE_CTL+41
#define VOIP_MGR_SET_DAA_TX_GAIN			VOIP_MGR_BASE_CTL+42
#define VOIP_MGR_SET_DAA_RX_GAIN			VOIP_MGR_BASE_CTL+43
#define VOIP_MGR_SET_FSK_VMWI_STATE			VOIP_MGR_BASE_CTL+44
#define VOIP_MGR_SET_FSK_AREA				VOIP_MGR_BASE_CTL+45
#define VOIP_MGR_FSK_ALERT_GEN_CFG			VOIP_MGR_BASE_CTL+46
#define VOIP_MGR_SET_FLASH_HOOK_TIME			VOIP_MGR_BASE_CTL+47
#define VOIP_MGR_SET_RTCP_SESSION			VOIP_MGR_BASE_CTL+48
#define VOIP_MGR_UNSET_RTCP_SESSION			VOIP_MGR_BASE_CTL+49
#define VOIP_MGR_SET_TONE_OF_COUNTRY			VOIP_MGR_BASE_CTL+50
#define VOIP_MGR_SET_TONE_OF_CUSTOMIZE			VOIP_MGR_BASE_CTL+51
#define VOIP_MGR_SET_CUST_TONE_PARAM			VOIP_MGR_BASE_CTL+52
#define VOIP_MGR_USE_CUST_TONE				VOIP_MGR_BASE_CTL+53
#define VOIP_MGR_SET_SLIC_RING_CADENCE			VOIP_MGR_BASE_CTL+54
#define VOIP_MGR_SET_SLIC_REG_VAL			VOIP_MGR_BASE_CTL+55
#define VOIP_MGR_DEBUG					VOIP_MGR_BASE_CTL+56
#define VOIP_MGR_8305_SWITCH_VAL			VOIP_MGR_BASE_CTL+57
#define VOIP_MGR_SET_G168_LEC_CFG			VOIP_MGR_BASE_CTL+58
#define VOIP_MGR_PLAY_IVR				VOIP_MGR_BASE_CTL+59
#define VOIP_MGR_POLL_IVR				VOIP_MGR_BASE_CTL+60
#define VOIP_MGR_STOP_IVR				VOIP_MGR_BASE_CTL+61
#define VOIP_MGR_WAN_VLAN_TAG				VOIP_MGR_BASE_CTL+62
#define VOIP_MGR_BRIDGE_MODE				VOIP_MGR_BASE_CTL+63
#define VOIP_MGR_SIP_REGISTER				VOIP_MGR_BASE_CTL+64
#define VOIP_MGR_SET_CID_DTMF_MODE			VOIP_MGR_BASE_CTL+65
#define VOIP_MGR_PLAY_SIP_INFO				VOIP_MGR_BASE_CTL+66
#define VOIP_MGR_SET_SPK_AGC				VOIP_MGR_BASE_CTL+67
#define VOIP_MGR_SET_SPK_AGC_LVL			VOIP_MGR_BASE_CTL+68
#define VOIP_MGR_SET_SPK_AGC_GUP			VOIP_MGR_BASE_CTL+69
#define VOIP_MGR_SET_SPK_AGC_GDOWN			VOIP_MGR_BASE_CTL+70
#define VOIP_MGR_SET_MIC_AGC				VOIP_MGR_BASE_CTL+71
#define VOIP_MGR_SET_MIC_AGC_LVL			VOIP_MGR_BASE_CTL+72
#define VOIP_MGR_SET_MIC_AGC_GUP			VOIP_MGR_BASE_CTL+73
#define VOIP_MGR_SET_MIC_AGC_GDOWN			VOIP_MGR_BASE_CTL+74
#define VOIP_MGR_SET_DAA_ISR_FLOW			VOIP_MGR_BASE_CTL+75
#define VOIP_MGR_GET_DAA_ISR_FLOW			VOIP_MGR_BASE_CTL+76
#define VOIP_MGR_SET_DAA_PCM_HOLD_CFG			VOIP_MGR_BASE_CTL+77
#define VOIP_MGR_GET_DAA_BUSY_TONE_STATUS		VOIP_MGR_BASE_CTL+78
#define VOIP_MGR_GET_DAA_CALLER_ID			VOIP_MGR_BASE_CTL+79
#define VOIP_MGR_GET_FEATURE				VOIP_MGR_BASE_CTL+80
#define VOIP_MGR_SET_CID_DET_MODE			VOIP_MGR_BASE_CTL+81
#define VOIP_MGR_GET_FSK_CID_STATE_CFG			VOIP_MGR_BASE_CTL+82
#define VOIP_MGR_GET_DAA_USED_BY_WHICH_SLIC		VOIP_MGR_BASE_CTL+83
#define VOIP_MGR_SET_CID_FSK_GEN_MODE			VOIP_MGR_BASE_CTL+84
#define VOIP_MGR_GET_T38_PCMIN				VOIP_MGR_BASE_CTL+85
#define VOIP_MGR_GET_T38_PACKETIN			VOIP_MGR_BASE_CTL+86
#define VOIP_MGR_SET_VOICE_GAIN				VOIP_MGR_BASE_CTL+87
#define VOIP_MGR_FSK_CID_VMWI_GEN_CFG			VOIP_MGR_BASE_CTL+88
#define VOIP_MGR_ENERGY_DETECT				VOIP_MGR_BASE_CTL+89
#define VOIP_MGR_GET_RTP_STATISTICS			VOIP_MGR_BASE_CTL+90
#define VOIP_MGR_GET_SLIC_STAT				VOIP_MGR_BASE_CTL+93
#define VOIP_MGR_SET_GETDATA_MODE			VOIP_MGR_BASE_CTL+94
#define VOIP_MGR_SET_DSCP				VOIP_MGR_BASE_CTL+95
#define VOIP_MGR_IPHONE_TEST                		VOIP_MGR_BASE_CTL+96
#define VOIP_MGR_CTL_KEYPAD				VOIP_MGR_BASE_CTL+97
#define VOIP_MGR_CTL_LCM				VOIP_MGR_BASE_CTL+98
#define VOIP_MGR_SLIC_ONHOOK_ACTION			VOIP_MGR_BASE_CTL+99	// only used in AudioCodes
#define VOIP_MGR_SLIC_OFFHOOK_ACTION			VOIP_MGR_BASE_CTL+100	// only used in AudioCodes
#define VOIP_MGR_WAN_2_VLAN_TAG				VOIP_MGR_BASE_CTL+101
#define VOIP_MGR_CTL_VOICE_PATH				VOIP_MGR_BASE_CTL+102
#define VOIP_MGR_SET_RTCP_TX_INTERVAL			VOIP_MGR_BASE_CTL+103	
#define VOIP_MGR_FAX_END_DETECT				VOIP_MGR_BASE_CTL+104
#define VOIP_MGR_VOIP_RESOURCE_CHECK			VOIP_MGR_BASE_CTL+105
#define VOIP_MGR_WAN_3_VLAN_TAG				VOIP_MGR_BASE_CTL+106

#define VOIP_MGR_SET_MAX				VOIP_MGR_BASE_CTL+110

#define VOIP_MGR_GET_EBL				VOIP_MGR_BASE_CTL+1
#define VOIP_MGR_GET_SESSION 				VOIP_MGR_BASE_CTL+2
#define VOIP_MGR_GET_MAX				VOIP_MGR_BASE_CTL+110

typedef struct
{
	// RTP session info
	Tuint8 ch_id;
	Tint32 m_id;
	Tuint32 ip_src_addr;
	Tuint32 ip_dst_addr;
	Tuint16 udp_src_port;
	Tuint16 udp_dst_port;
        Tuint16 rtcp_src_port;
        Tuint16 rtcp_dst_port;
	Tuint8 protocol;
	Tuint8 tos;
	Tuint16 rfc2833_payload_type_local;
	Tuint16 rfc2833_payload_type_remote;
	//Payload info
	RtpPayloadType uPktFormat;
	Tint32 nG723Type;
	Tint32 nFramePerPacket;
	Tint32 bVAD;
	Tuint32 nJitterDelay;
	Tuint32 nMaxDelay;
	Tuint32 nJitterFactor;
	// RTP session state
	Tuint8	state;
	Tint32 result;
}
TstVoipMgrRtpCfg;


typedef struct
{
	Tuint8 ch_id;
	Tuint8 enable;
	TstVoipMgrRtpCfg rtp_cfg[2];
}
TstVoipMgr3WayCfg;

typedef struct
{
	Tuint32 ip_src_addr;
	Tuint32 ip_dst_addr;
	Tuint16 udp_src_port;
	Tuint16 udp_dst_port;
	Tuint8 protocol;
	Tuint8 ch_id;
	Tint32 m_id;
	Tint32 result;
	Tuint8 tos;
	Tuint16 rfc2833_payload_type_local;
	Tuint16 rfc2833_payload_type_remote;
}
TstVoipMgrSession;

typedef struct
{
 	Tuint32 ip_src_addr;
        Tuint32 ip_dst_addr;
        Tuint16 rtcp_src_port;
        Tuint16 rtcp_dst_port;
        Tuint8 protocol;
        Tuint8 ch_id;
        Tint32 m_id;
}
TstVoipRtcpSession;


typedef struct
{
	Tuint32 ch_id;
	Tuint8	enable;
}
TstVoipTest;

typedef struct
{
	Tuint32 ch_id;
	Tuint32 m_id;
	RtpPayloadType uPktFormat;
	Tint32 nG723Type;
	Tint32 nFramePerPacket;
	Tint32 bVAD;                                     // bool
	Tint32 result;
	Tuint32 nJitterDelay;
	Tuint32 nMaxDelay;
	Tuint32 nJitterFactor;
}
TstVoipPayLoadTypeConfig;

typedef struct
{
	Tuint32 ch_id;
	Tuint32 m_id;
	Tuint8	state;
}
TstVoipRtpSessionState;

typedef struct
{
	Tuint32 ch_id;
	Tuint32 m_id;
	DSPCODEC_TONE nTone;
	Tuint32 bFlag;
	DSPCODEC_TONEDIRECTION path;
}
TstVoipPlayToneConfig;

typedef struct
{
	Tuint32 ch_id;
	Tuint32 m_id;
	Tuint32 t_id;
	Tuint32 enable; //0-> off, 1->ON
	Tuint32 cfg;
}
TstVoipCfg;

typedef struct
{
	Tuint32 ch_id;
	Tuint32 m_id;
	Tint8	value;
	Tint8	value1;
	Tint8	value2;
	Tint8	value3;
	Tint8	value4;
	Tint16	value5;
	Tint16	value6;
}
TstVoipValue;

typedef struct
{
	unsigned char ch_id;          // CH:0 - 3
	unsigned char change;         // 1: Change. 0: No-Change
	unsigned char hook_status;    // 1: Off-Hook, 0: On-Hook
}
TstVoipSlicHook;

typedef struct
{
	unsigned char ch_id;          // CH = 0 ~ 3
	unsigned char ring_set;       // Ring_ON: ring_set = 1 , Ring_OFF: ring_set = 0
}
TstVoipSlicRing;

typedef struct
{
	unsigned char ch_id;
	unsigned int tone2play;
}
TstVoipSlicTone;

typedef struct
{
	unsigned char ch_id;
	unsigned int codec_law; //linear: 0, a_law: 1, u_law: 2
}
TstVoipSlicRestart;

typedef struct
{
	unsigned char ch_id;
	unsigned char daa_id;
	unsigned char cid_state;
	unsigned char cid_mode;
	unsigned char cid_gain;
	unsigned char cid_msg_type;
	char string[21];
	char string2[9];	// for date & time sync
	char cid_name[51];
	unsigned char cid_dtmf_mode;
}
TstVoipCID;//For dtmf cid generation

typedef struct
{
	unsigned char ch_id;
	unsigned char sid;
	unsigned char digit;
	unsigned char duration;


}
TstVoip2833;//For sending RFC2833 packet

typedef struct
{
	unsigned char ch_id;
	unsigned char reg_num;
	unsigned char reg_val;
}
TstVoipSlicReg;

typedef struct
{
	unsigned char ch_id;
	unsigned char s_id;
	RtpPayloadType uPktFormat;
	unsigned char channel_enable;
}
TstTwoChannelCfg; //for two channel simplify g729

typedef struct
{
	unsigned char ch_id;
	unsigned int  flash_time;
	unsigned int  flash_time_min;
	unsigned int  dummy;
}
TstVoipHook;


/**
 * @ingroup VOIP_PHONE_TONE
 * Structure for tone configuration
 * @sa _TONE_TYPE_
 */
typedef struct
{
	unsigned long	toneType;	///< see _TONE_TYPE_
	unsigned short	cycle;		///< "<0": illegal value, "0": represent "continuous tone", ">0": cycle number

	unsigned short	cadNUM;		///< Cadence number (in SUCC and SUCC_ADD mode, it represent repeat number of sequence)

	unsigned long	CadOn0;		///< Cadence On0 time (ms)
	unsigned long	CadOn1;		///< Cadence On1 time (ms)
	unsigned long	CadOn2;		///< Cadence On2 time (ms)
	unsigned long	CadOn3;		///< Cadence On3 time (ms)
	unsigned long	CadOff0;	///< Cadence Off0 time (ms)
	unsigned long	CadOff1;	///< Cadence Off1 time (ms)
	unsigned long	CadOff2;	///< Cadence Off2 time (ms)
	unsigned long	CadOff3;	///< Cadence Off3 time (ms)

	unsigned long PatternOff;	///< pattern Off time (ms)
	unsigned long ToneNUM;		///< tone number (1..4)

	unsigned long	Freq1;		///< Freq1 (Hz)
	unsigned long	Freq2;		///< Freq2 (Hz)
	unsigned long	Freq3;		///< Freq3 (Hz)
	unsigned long	Freq4;		///< Freq4 (Hz)

	long Gain1;					///< Gain1 (db)
	long Gain2;					///< Gain2 (db)
	long Gain3;					///< Gain3 (db)
	long Gain4;					///< Gain4 (db)

}
TstVoipToneCfg;

typedef struct
{
	unsigned short phy;
	unsigned short reg;
	unsigned short value;
	unsigned short read_write;
} TstVoipSwitch;

#ifdef CONFIG_RTK_VOIP_IVR

#define MAX_LEN_OF_IVR_TEXT		40
typedef struct {
	unsigned char ch_id;
	IvrPlayType_t type;		/* == IVR_PLAY_TYPE_TEXT */
	unsigned int playing_period_10ms;	/* output */
	unsigned int ret1_unused;			/* output */
	/* --- Above part should be identical to TstVoipPlayIVR_Header --- */
	unsigned char szText2speech[ MAX_LEN_OF_IVR_TEXT + 1 ];
} TstVoipPlayIVR_Text;

#define MAX_FRAMES_OF_G72363	10
typedef struct {
	unsigned char ch_id;
	IvrPlayType_t type;		/* == IVR_PLAY_TYPE_G723_63 */
	unsigned int playing_period_10ms;	/* output */
	unsigned int nRetCopiedFrames;		/* output */
	/* --- Above part should be identical to TstVoipPlayIVR_Header --- */
	unsigned int  nFramesCount;
	unsigned char data[ 24 * MAX_FRAMES_OF_G72363 ];
} TstVoipPlayIVR_G72363;

typedef struct {
	unsigned char ch_id;
	IvrPlayType_t type;
	unsigned int playing_period_10ms;	/* output */
	unsigned int ret1;					/* output, each codec has different meaning */
} TstVoipPlayIVR_Header;

typedef struct {
	unsigned char ch_id;
	unsigned char bPlaying;		/* output: play or not */
} TstVoipPollIVR;

typedef struct {
	unsigned char ch_id;
} TstVoipStopIVR;
#endif /* CONFIG_RTK_VOIP_IVR */

typedef struct
{
	unsigned char enable;
	unsigned short vlanId;
	unsigned char priority;
	unsigned char cfi;
} TstVoipSwitch_VLAN_tag;

typedef struct
{
	unsigned char enable;
	unsigned short vlanIdVoice;
	unsigned char priorityVoice;
	unsigned char cfiVoice;
	unsigned short vlanIdData;
	unsigned char priorityData;
	unsigned char cfiData;
} TstVoipSwitch_2_VLAN_tag;

typedef struct
{
	unsigned char enable;
	unsigned short vlanIdVoice;
	unsigned char priorityVoice;
	unsigned char cfiVoice;
	unsigned short vlanIdData;
	unsigned char priorityData;
	unsigned char cfiData;
	unsigned short vlanIdVideo;
	unsigned char priorityVideo;
	unsigned char cfiVideo;
} TstVoipSwitch_3_VLAN_tag;


typedef struct {
	unsigned short pcmIn[ 480 ];
	unsigned int snPcm;
	unsigned int ret;
} TstT38PcmIn;

typedef struct {
	unsigned short packetIn[ 300 ];
	unsigned int nSize;
	unsigned int snPcm;
} TstT38PacketIn;

typedef struct
{
	long ch_id;
	long write_enable;
	long length;
	long mode;// 1:tx, 0:rx
	char buf[1120];
} TstVoipdataget_o;

typedef struct
{
	long tx_readindex;	//tx data read index
	long tx_writeindex;	//tx data write index
	long write_enable;
	long rx_readindex;
	long rx_writeindex;
	long rx_readindex2;
	long rx_writeindex2;
	char *txbuf;		//tx buffer
	char *rxbuf;		//rx buffer
	char *rxbuf2;		//rx2 buffer
}
TstVoipdataget;

typedef struct {
	unsigned char chid;						/* input */
	unsigned char bResetStatistics;			/* input */
	/* follows are output */
	unsigned long nRxRtpStatsCountByte;
	unsigned long nRxRtpStatsCountPacket;
	unsigned long nRxRtpStatsLostPacket;
	unsigned long nTxRtpStatsCountByte;
	unsigned long nTxRtpStatsCountPacket;
} TstVoipRtpStatistics;

typedef struct 
{
	Voip_reosurce_t resource;
	TstVoipMgr3WayCfg _3way_session_info;
} TstVoipResourceCheck;

#ifdef CONFIG_RTK_VOIP_IP_PHONE
 #ifdef __KERNEL__
  #include <linux/types.h>	// pid_t
 #else
  #include <sys/types.h>	// pid_t
 #endif

/* We list keypad control in follow */
typedef unsigned short wkey_t;	/* wide key */

typedef struct TstKeypadGeneral_s {
	keypad_cmd_t cmd;
} TstKeypadGeneral;

typedef struct TstKeypadSetTarget_s {
	keypad_cmd_t	cmd;	// = KEYPAD_CMD_SET_TARGET
	pid_t			pid;
} TstKeypadSetTarget;

typedef struct TstKeypadSignalDebug_s {
	keypad_cmd_t	cmd;	// = KEYPAD_CMD_SIG_DEBUG
	wkey_t			wkey;
} TstKeypadSignalDebug;

typedef struct TstKeypadReadKey_s {
	keypad_cmd_t	cmd;	// = KEYPAD_CMD_READ_KEY
	wkey_t			wkey;
	unsigned char	validKey;
} TstKeypadReadKey;

typedef struct TstKeypadHookStatus_s {
	keypad_cmd_t	cmd;	// = KEYPAD_CMD_HOOK_STATUS
	unsigned char	status;	/* 0: on-hook, 1: off-hook */
} TstKeypadHookStatus;

typedef union {
	TstKeypadGeneral		General;
	TstKeypadSetTarget		SetTarget;
	TstKeypadSignalDebug	SignalDebug;
	TstKeypadReadKey		ReadKey;
	TstKeypadHookStatus		HookStatus;
} TstKeypadCtl;

/* We list LCM control in follow */
typedef struct TstLcmDisplayOnOff_s {
	lcm_cmd_t		cmd;	// = LCD_CMD_DISPLAY_ON_OFF
	unsigned char	bDisplayOnOff;	/* 1: on, 0: off, others: invalid */
	unsigned char	bCursorOnOff;	/* 1: on, 0: off, others: invalid */
	unsigned char	bCursorBlink;	/* 1: on, 0: off, others: invalid */
} TstLcmDisplayOnOff;

typedef struct TstLcmMoveCursorPosition_s {
	lcm_cmd_t		cmd;	// = LCM_CMD_MOVE_CURSOR_POS
	int				x;
	int				y;
} TstLcmMoveCursorPosition;

#define MAX_LEN_OF_DRAW_TEXT	16	/* NOT include null-terminator */
typedef struct TstLcmDrawText_s {
	lcm_cmd_t		cmd;	// = LCM_CMD_DRAW_TEXT
	int				x;
	int				y;
	unsigned char 	szText[ MAX_LEN_OF_DRAW_TEXT ];
	int				len;
} TstLcmDrawText;

typedef struct TstLcmGeneral_s {
	lcm_cmd_t		cmd;
} TstLcmGeneral;

typedef union TstLcmCtl_s {
	TstLcmGeneral				General;
	TstLcmDisplayOnOff			DisplayOnOff;
	TstLcmMoveCursorPosition	MoveCursorPosition;
	TstLcmDrawText				DrawText;
} TstLcmCtl;

/* Control voice path */
typedef struct TstVoicePath_s {
	vpath_t	vpath;
} TstVoicePath_t;
#endif /* CONFIG_RTK_VOIP_IP_PHONE */

#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
typedef struct
{
	unsigned int function_type;
	unsigned int reg;
	unsigned int value;
} IPhone_test;
#endif

#endif


