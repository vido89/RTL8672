/*
 * voip_flash.h: VoIP Flash Header
 *
 * Authors: Rock <shaofu@realtek.com.tw>
 *
 */

#ifndef __VOIP_FLASH_H
#define __VOIP_FLASH_H

#ifdef CVVOIP

// don't care feature in CVVOIP
#define SLIC_NUM 2
#define VOIP_CH_NUM 2

#else

#include "kernel_config.h"
#include "rtk_voip.h"
#ifdef CONFIG_RTK_VOIP_IP_PHONE
#include "ui_flash_layout.h"
#endif

#endif

//------------------------------------------------------------------
//			VOIP FLASH FEATURE
//------------------------------------------------------------------
#define VOIP_FLASH_SIGNATURE			0x766f6970		// voip 
#define VOIP_CONFIG_SIGNATURE			0x08010806		// 8186

#define VOIP_FLASH_VER					28
// ver 2: add call_waiting_cid 
// ver 3: add funckey_transfer, funckey_pstn, off_hook_alarm, auto_dial
// ver 4: 
//	- change default sip port (couldn't be the same sip port)
//	- add speed dial
// ver 5:
//  - change speed dial length: 31->61
//	- hide credit_quota
//	- add auto config setting
//  - add mib version!
// ver 6:
//  - add jitter delay configuration
// ver 7:
//  - add QoS TOS/DSCP
// ver 8:
//  - add min flash hook time
// ver 9:
//  - add caller id dtmf start/end digit configruation
// ver 10:
//  - add dial plan
// ver 11:
//  - add sip info duration
// ver 12:
//  - modify dial plan layout: divide 256 bytes into many parts including
//    original dial plan, replace rule, and auto prefix.
// ver 13:
//  - add agc config
// ver 14:
//  - enlarge _CODEC_MAX 
// ver 15:
//  - add caller id detection mode 
// ver 16:
//  - add caller id auto detection selection 
// ver 17:
//  - move daa volume to structure voipCfgParam_s
// ver 18:
//	- change feature to 32 bits
//  - move ports[] to last field of voipCfgParam_s for handling port number not match issue
// ver 19:
//  - add FSK caller id gen mode selection
// ver 20:
// - add T.38 config
// ver 21:
// - add voice gain config
// ver 22:
// - add rtpDscp, sipDscp
// ver 23:
// - add hotline, dnd
// ver 24:
// - add VLAN for Voice and Data traffic
// ver 25:
// - use hotline/dnd per port
// ver 26:
// - add video vlan tag
// ver 27:
// - add optimization factor of jitter buffer 
// ver 28:
// - add iLBC
// - move flash hook time from voipCfgParam_s to voipCfgPortParam_s
// - support multi proxy

#define VOIP_FLASH_MIB_VER				0x0000

#define	VOIP_FLASH_FEATURE				RTK_VOIP_FEATURE
#define VOIP_FLASH_EXT_FEATURE 			0x00000000

//------------------------------------------------------------------
//			VOIP FLASH Control
//------------------------------------------------------------------
#ifdef X86_SIMULATE
#define VOIP_PATHNAME					"/bin/sh"	// use for ftok
#else
#define VOIP_PATHNAME					"/bin/solar"	// use for ftok
#endif
#define VOIP_SEM_EVENT					0				// number in sem_set = 0
#define VOIP_SEM_MUTEX					1				// number in sem_set = 1
#define VOIP_SHARE_NOPORTS_SIZE (sizeof(voip_flash_share_t) - sizeof(voipCfgPortParam_t) * VOIP_PORTS)
#define VOIP_SHARE_SIZE(n) (VOIP_SHARE_NOPORTS_SIZE + sizeof(voipCfgPortParam_t) * (n))

//------------------------------------------------------------------
//			VOIP FLASH STRUCT
//------------------------------------------------------------------

#define	VOIP_PORTS				VOIP_CH_NUM			///< channel number
#define	MAX_VOIP_PORTS			PCM_CH_NUM			///< max channel numbera

#define DNS_LEN					40		///< max DNS length
#define FW_LEN					40		///< max forward URI length
#define	DEF_SIP_PROXY_PORT		5060
#define	DEF_SIP_LOCAL_PORT		5060
#define	DEF_RTP_PORT			9000
#define	DEF_OUTBOUND_PROXY_PORT		5060
#define	DEF_STUN_SERVER_PORT		  3478
#define FUNC_KEY_LENGTH			3		///< max function key length
#define MAX_SPEED_DIAL			10		///< 0..9
#define MAX_SPEED_DIAL_NAME		11
#define MAX_SPEED_DIAL_URL		61
#define MAX_AUTO_CONFIG_PATH	61
#define MAX_REPLACE_RULE_SOURCE	80
#define MAX_REPLACE_RULE_TARGET	10
#define MAX_DIALPLAN_LENGTH		80
#define MAX_AUTO_PREFIX			5
#define MAX_PREFIX_UNSET_PLAN	80
#define DEF_T38_PORT			(DEF_RTP_PORT + VOIP_PORTS * 4)
#define MAX_PROXY				2

// proxy types
enum __PROXY_ENABLE__
{
	PROXY_DISABLED = 0,
	PROXY_ENABLED = 0x0001,
	PROXY_NORTEL = 0x0002,
};

/**
 * @ingroup VOIP_RTP_SESSION
 * Enumeration for codec type
 */
enum __CODEC_TYPE__
{
	_CODEC_NOTSUPPORT = -1,
	_CODEC_G711U = 0,
	_CODEC_G711A,
	_CODEC_G729,
	_CODEC_G723,
	_CODEC_G726_16,
	_CODEC_G726_24,
	_CODEC_G726_32,
	_CODEC_G726_40,
	_CODEC_GSMFR,
	_CODEC_ILBC,
	_CODEC_MAX
};

enum
{
	SUPPORTED_CODEC_G711U = 0,
	SUPPORTED_CODEC_G711A,
#ifdef CONFIG_RTK_VOIP_G729AB
	SUPPORTED_CODEC_G729,
#endif	
#ifdef CONFIG_RTK_VOIP_G7231
	SUPPORTED_CODEC_G723,
#endif	
#ifdef CONFIG_RTK_VOIP_G726
	SUPPORTED_CODEC_G726_16,
	SUPPORTED_CODEC_G726_24,
	SUPPORTED_CODEC_G726_32,
	SUPPORTED_CODEC_G726_40,
#endif	
#ifdef CONFIG_RTK_VOIP_GSMFR
	SUPPORTED_CODEC_GSMFR,
#endif
#ifdef CONFIG_RTK_VOIP_ILBC
	SUPPORTED_CODEC_ILBC,
#endif
	SUPPORTED_CODEC_MAX
};

/**
 * @ingroup VOIP_RTP_SESSION
 * Enumeration for G723 type
 */
enum __G7231_RATE__
{
	G7231_RATE63 = 0,
	G7231_RATE53,
};

/**
 * @ingroup VOIP_RTP_SESSION
 * Enumeration for iLBC mode
 */
enum __ILBC_MODE_
{
	ILBC_30MS = 0,
	ILBC_20MS
};

/**
 * @ingroup VOIP_RTP_DTMF
 * Enumeration for DTMF type
 */
enum __DTMF_TYPE__
{
	DTMF_RFC2833 = 0,
	DTMF_SIPINFO,
	DTMF_INBAND,
	DTMF_MAX
};

/**
 * @ingroup VOIP_PHONE_CALLERID
 * Enumeration for CID area
 */
enum _CID_AREA_
{
    CID_FSK_BELLCORE = 0,
    CID_FSK_ETSI,
    CID_FSK_BT,
    CID_FSK_NTT,
    CID_DTMF,
    CID_MAX
};

/**
 * @ingroup VOIP_PHONE_CALLERID
 * Enumeration for CID DTMF start/end digit
 */
enum _CID_DTMF_
{
    CID_DTMF_A = 0,
    CID_DTMF_B,
    CID_DTMF_C,
    CID_DTMF_D,
    CID_DTMF_MAX
};


/**
 * @ingroup VOIP_PHONE_TONE
 * Enumeration for Tone Country
 */
enum _TONE_COUNTRY_
{
    TONE_USA = 0,
    TONE_UK,
    TONE_AUSTRALIA,
    TONE_HK,
    TONE_JAPAN,
    TONE_SWEDEN,
    TONE_GERMANY,
    TONE_FRANCE,
    TONE_TR57,
    TONE_BELGIUM,
    TONE_FINLAND,
    TONE_ITALY,
    TONE_CHINA,
    TONE_CUSTOMER,
    TONE_MAX
};

/**
 * @ingroup VOIP_PHONE_TONE
 * Enumeration for Tone Type
 */
enum _TONE_TYPE_
{
    TONE_TYPE_ADDITIVE = 0,
    TONE_TYPE_MODULATED,
    TONE_TYPE_SUCC,
    //TONE_TYPE_SUCC_ADD,
    TONE_TYPE_MAX
};

/**
 * @ingroup VOIP_PHONE_TONE
 * Enumeration for Tone Cycle
 */
enum _TONE_CYCLE_
{
    TONE_CYCLE_CONTINUOUS = 0,
    TONE_CYCLE_BURST,
    TONE_CYCLE_CADENCE,
    TONE_CYCLE_MAX
};

/*
 * @ingroup VOIP_CONFIG
 * Enumeration for Tone Customer
 */
enum _TONE_CUSTOMER_
{
    TONE_CUSTOMER_1 = 0,
    TONE_CUSTOMER_2,
    TONE_CUSTOMER_3,
    TONE_CUSTOMER_4,
    TONE_CUSTOMER_5,
    TONE_CUSTOMER_6,
    TONE_CUSTOMER_7,
    TONE_CUSTOMER_8,
    TONE_CUSTOMER_MAX
};

/*
 * @ingroup VOIP_CONFIG
 * Enumeration for Ring Group
 */
enum _RING_GROUP_
{
    RING_GROUP_1 = 0,
    RING_GROUP_2,
    RING_GROUP_3,
    RING_GROUP_4,
    RING_GROUP_MAX
};


/*
 * @ingroup VOIP_CONFIG
 * Enumeration for Ring Customer
 */
enum _RING_CADENCE_
{
    RING_CADENCE_1 = 0,
    RING_CADENCE_2,
    RING_CADENCE_3,
    RING_CADENCE_4,
    RING_CADENCE_5,
    RING_CADENCE_6,
    RING_CADENCE_7,
    RING_CADENCE_8,
    RING_CADENCE_MAX
};

typedef enum {
	NET_DHCP_DISABLED = 0,
	NET_DHCP_CLIENT
} net_dhcp_t;

typedef enum {
	NET_CFG_DHCP,
	NET_CFG_IP,
	NET_CFG_NETMASK,
	NET_CFG_GATEWAY,
	NET_CFG_DNS,
} net_cfg_t;

/**
 * @ingroup VOIP_CONFIG
 * Enumeration for Voice QoS TOS/DSCP Class
 */
enum _DSCP_CS_
{
    DSCP_CS0 = 0,
    DSCP_CS1,
    DSCP_CS2,
    DSCP_CS3,
    DSCP_CS4,
    DSCP_CS5,
    DSCP_CS6,
    DSCP_CS7,
    DSCP_EF,
    DSCP_MAX
};

/*
 * Auto Dial:
 *	- Auto Dial Time: 0x0001~0x000f (1~15 sec)
 *	- AUTO_DIAL_ALWAYS (Not check HASH) mask: 0x0010
 */
enum {
	AUTO_DIAL_DISABLE	= 0x0000,
	AUTO_DIAL_TIME		= 0x000f,
	AUTO_DIAL_ALWAYS	= 0x0010
};

#pragma pack(push, 1)

typedef struct ToneCfgParam	st_ToneCfgParam;
typedef struct SpeedDialCfg_s SpeedDialCfg_t;
typedef struct voipCfgProxy_s voipCfgProxy_t;
typedef struct voipCfgPortParam_s voipCfgPortParam_t;
typedef struct voipCfgParam_s voipCfgParam_t;

/*
 * @ingroup VOIP_PHONE_RING
 * Enumeration for Tone Customize
 */
struct ToneCfgParam
{
	unsigned long	toneType;
	unsigned short	cycle;
	
	unsigned short	cadNUM;
	
	unsigned long	CadOn0;
	unsigned long	CadOn1;
	unsigned long	CadOn2;
	unsigned long	CadOn3;
	unsigned long	CadOff0;
	unsigned long	CadOff1;
	unsigned long	CadOff2;
	unsigned long	CadOff3;
	
	unsigned long PatternOff;
	unsigned long ToneNUM;
	
	unsigned long	Freq1;
	unsigned long	Freq2;
	unsigned long	Freq3;
	unsigned long	Freq4;
	
	long Gain1;
	long Gain2;
	long Gain3;
	long Gain4;
	
};

struct SpeedDialCfg_s
{
	char			name[MAX_SPEED_DIAL_NAME];
	char			url[MAX_SPEED_DIAL_URL];
};

/*
 * @ingroup VOIP_CONFIG
 * @brief Structure for UI
 */
typedef struct ui_falsh_layout1_s {
	unsigned long	test;
} ui_falsh_layout1_t;

/*
 * @ingroup VOIP_CONFIG
 * @brief Structure for Proxy
 */
struct voipCfgProxy_s
{
	// account
	char display_name[DNS_LEN];				///< display name @sa DNS_LEN
	char number[DNS_LEN];					///< username @sa DNS_LEN
	char login_id[DNS_LEN];					///< login id @sa DNS_LEN
	char password[DNS_LEN];					///< password @sa DNS_LEN
	// register server
	unsigned short enable;					///< enable/disable register server setting, see __PROXY_ENABLE__
	char addr[DNS_LEN];						///< register server address
	unsigned short port;					///< default: 5060
	char domain_name[DNS_LEN];				///< register domain name, used in "from", "to" like user@domain
	unsigned int reg_expire;				///< default: 60 sec
	// nat traversal server
	unsigned short outbound_enable;			///< enable/disable outbound proxy setting
	char outbound_addr[DNS_LEN];			///< outbound proxy addr
	unsigned short outbound_port;			///< outbound proxy port
};

/*
 * @ingroup VOIP_CONFIG
 * @brief Structure for sip handling
 */
struct voipCfgPortParam_s
{
	// general
	voipCfgProxy_t proxies[MAX_PROXY];	///< proxy server setting
	unsigned char default_proxy;

	// nat 
	unsigned char stun_enable;				///< stun enable
	char stun_addr[DNS_LEN];				///< stun server
	unsigned short stun_port;				///< stun port

	// advanced
	unsigned short	sip_port;				///< default: 5060
	unsigned short	media_port;				///< default: 9000
	unsigned char	dtmf_mode;				///< default: DTMF_INBAND @sa __DTMF_TYPE__
	unsigned short	payload_type;				///< payload type in RTP if enale RFC2833
	unsigned short	sip_info_duration;			///< sip info duration if enable SIP Info
	unsigned char	call_waiting_enable;			///< call waiting option

	// forward
	unsigned char	uc_forward_enable;			///< unconditional forward option
	char		uc_forward[FW_LEN];			///< unconditional forward address
	unsigned char	busy_forward_enable;			///< busy forward option
	char		busy_forward[FW_LEN];			///< busy forward address
	unsigned char	na_forward_enable;			///< no answer forward option
	char		na_forward[FW_LEN];			///< no answer forward address
	unsigned short	na_forward_time;			///< no answer forward time setting

	// speed dial
	SpeedDialCfg_t	speed_dial[MAX_SPEED_DIAL];

	// dial plan
	unsigned char	replace_rule_option;							///< replace rule option
	unsigned char	replace_rule_source[ MAX_REPLACE_RULE_SOURCE ];	///< replace rule source plan
	unsigned char	replace_rule_target[ MAX_REPLACE_RULE_TARGET ];	///< replace rule target
	unsigned char	dialplan[MAX_DIALPLAN_LENGTH];					///< dialplan
	unsigned char 	auto_prefix[ MAX_AUTO_PREFIX ];					///< auto prefix
	unsigned char 	prefix_unset_plan[ MAX_PREFIX_UNSET_PLAN ];		///< prefix unset plan

	// codec
	unsigned char 	frame_size[_CODEC_MAX];			///< 0: 1 frame per packet
	unsigned char	precedence[_CODEC_MAX];			///< default: 2, 0, 1, 3 (G729,G711u,G711a,G723) @sa __CODEC_TYPE__
	unsigned char 	vad;					///< 0: disable 1: enable
	unsigned char	g7231_rate;				///< 5.3k or 6.3k
	unsigned char	iLBC_mode;				///< 30ms or 20ms

	// DSP
	unsigned char	slic_txVolumne;				///< 0..9
	unsigned char	slic_rxVolumne;				///< 0..9
	unsigned char	jitter_delay;			///< jitter delay in unit of 10ms
	unsigned char	maxDelay;				///< jitter-buffer max delay 5~10 : 50~100 ms, def:9
	unsigned char	jitter_factor;			///< jitter factor, 1: optimize to delay, 12: optimize to quality, 13: fix delay for FAX 
	unsigned char	echoTail;				///< echo-tail 0~5 : 1, 2, 4, 8, 16, 32 ms, def:3 
	unsigned char	caller_id_mode;				///< DTMF(1) or FSK(0)
	unsigned char	call_waiting_cid;			///< use call waiting caller ID or not.0: disable 1: enable
	unsigned char	cid_dtmf_mode;				///< configure the DTMF caller id start/end digit.
	unsigned char	speaker_agc;				///< 0: disable 1: enable
	unsigned char	spk_agc_lvl;				///< 0..8
	unsigned char	spk_agc_gu;				///< 0..8
	unsigned char	spk_agc_gd;				///< 0..8
	unsigned char	mic_agc;				///< 0: disable 1: enable
	unsigned char	mic_agc_lvl;				///< 0..8
	unsigned char	mic_agc_gu;				///< 0..8
	unsigned char	mic_agc_gd;				///< 0..8
	unsigned char	cid_fsk_gen_mode;			///< 0:hardware, 1: software dsp gen
	char		spk_voice_gain;				///< -32~31, Mute:-32,
	char		mic_voice_gain;				///< -32~31, Mute:-32,
	
	// QoS
	unsigned char	voice_qos;

	//T.38
	unsigned char useT38;					//enable T.38
	unsigned short T38_port;				//T.38 port
	
	// Hotline
	unsigned char 	hotline_enable;				// 0: disable, 1: enable
	char			hotline_number[DNS_LEN];

	// DND
	unsigned char	dnd_mode;			// 0: disable, 1: dnd period, 2: dnd_always
	unsigned char	dnd_from_hour;		// 0..23
	unsigned char	dnd_from_min;		// 0..59
	unsigned char	dnd_to_hour;		// 0..23
	unsigned char	dnd_to_min;			// 0..59

	// flash-hook time
	unsigned short	flash_hook_time;
	unsigned short	flash_hook_time_min;
};

struct voipCfgParam_s {
	// voip flash check
	unsigned long	signature;				// voip flash check
	unsigned short	version;				// update if big change (size/offset not match..)
	unsigned long	feature;				// 32 bit feature 
	unsigned long	extend_feature;			// 32 extend feature

	unsigned short mib_version; // update if some mib name change  (ex: VOIP.PORT[0].NUMBER => VOIP.PORT[0].USER.NUMBER)
								// used in import/export flash setting 
								// if mib version not match, then import will fail

	// RFC flags
	unsigned int	rfc_flags;				///< RFC flags
	
	//tone			///< unit: 10ms
	unsigned char 	tone_of_country;
	unsigned char 	tone_of_custdial;
	unsigned char 	tone_of_custring;
	unsigned char 	tone_of_custbusy;
	unsigned char 	tone_of_custwaiting;
	unsigned char 	tone_of_customize;
	st_ToneCfgParam	cust_tone_para[TONE_CUSTOMER_MAX];			// for customized tone (up to 8 tones)
	
	//ring
	unsigned char	ring_cad;
	unsigned char	ring_group;
	unsigned int	ring_phone_num[RING_GROUP_MAX];
	unsigned char	ring_cadence_use[RING_GROUP_MAX];
	unsigned char	ring_cadence_sel;
	unsigned short	ring_cadon[RING_CADENCE_MAX];
	unsigned short	ring_cadoff[RING_CADENCE_MAX];

	// function key
	char			funckey_pstn[FUNC_KEY_LENGTH];				// default is *0
	char			funckey_transfer[FUNC_KEY_LENGTH];			// default is *1
	
	// other
	unsigned short	auto_dial;								// 0, 3~9 sec, default is 5 sec, 0 is disable
	unsigned short	off_hook_alarm;								// 0, 10~60 sec, default is 30 sec, 0 is disable
	unsigned short  cid_auto_det_select;							// caller id auto detection selection
	unsigned short	caller_id_det_mode;							// caller id detection mode	
	
	// auto config
	unsigned short	auto_cfg_ver;								// load config if version is newer
	unsigned char	auto_cfg_mode;								// 0: disable, 1: http
	char			auto_cfg_http_addr[DNS_LEN];
	unsigned short	auto_cfg_http_port;
	char			auto_cfg_file_path[MAX_AUTO_CONFIG_PATH];
	unsigned short	auto_cfg_expire;							// 0..365 days

	// VLAN setting of WAN Port
	unsigned char	wanVlanEnable;
	// VLAN for Voice and protocl packets (SIP, STUN, DHCP, ARP, etc.)
	unsigned short	wanVlanIdVoice;
	unsigned char	wanVlanPriorityVoice;
	unsigned char	wanVlanCfiVoice;
	// VLAN for Data
	unsigned short	wanVlanIdData;
	unsigned char	wanVlanPriorityData;
	unsigned char	wanVlanCfiData;
	// VLAN for Video
	unsigned short	wanVlanIdVideo;
	unsigned char	wanVlanPriorityVideo;
	unsigned char	wanVlanCfiVideo;

	// FXO valume
	unsigned char	daa_txVolumne;				///< 1..10
	unsigned char	daa_rxVolumne;				///< 1..10

	// DSCP
	unsigned char	rtpDscp;
	unsigned char	sipDscp;

	voipCfgPortParam_t ports[VOIP_PORTS];
	
	// UI 
#ifdef CONFIG_RTK_VOIP_IP_PHONE
	ui_falsh_layout_t ui;
#endif
};

// 
// config header
// 
// valid import by config header check:
// 	1. mib_version have to match (mib name ok)
//	2. feature and extend feature match, or
//     current(flash) version == config(file) version
//	   (It will skip the not match part on import).
//
// config file struct:
// | ------------------- |
// | config file header  |
// | ------------------- |
// | voip encode config  |
// | ------------------- |
// | checksum (1 byte)   |
// | ------------------- |
typedef struct VOIP_CFG_HEADER_S VOIP_CFG_HEADER;

struct VOIP_CFG_HEADER_S {
    unsigned long signature; // config file signature
	unsigned short len;
};

typedef struct flash_netcfg_s {
	int dhcp;
	unsigned long ip;
	unsigned long netmask;
	unsigned long gateway;
	unsigned long dns;
	int ivr_lan;
	char ivr_interface[10];
} flash_netcfg_t;

typedef struct voip_flash_share_s {
	flash_netcfg_t net_cfg;
	voipCfgParam_t voip_cfg;
} voip_flash_share_t;

typedef struct {
	int mode;
	voipCfgParam_t current_setting;
	voipCfgParam_t default_setting;
} voipCfgAll_t;

#pragma pack(pop)

//------------------------------------------------------------------
//			VOIP FLASH Write
//------------------------------------------------------------------
typedef enum {
	VOIP_FLASH_WRITE_CLIENT_IPPHONE,
	VOIP_FLASH_WRITE_CLIENT_TR069,
	VOIP_FLASH_WRITE_CLIENT_RESERVE_0,	/* other application can use these reserved IDs */
	VOIP_FLASH_WRITE_CLIENT_RESERVE_1,
	VOIP_FLASH_WRITE_CLIENT_RESERVE_2,
	VOIP_FLASH_WRITE_CLIENT_RESERVE_3,
	MAX_VOIP_FLASH_WRITE_CLIENT,
	/* If a client doesn't use writing function, let it to be a invalid ID. */
	VOIP_FLASH_WRITE_CLIENT_SOLAR = MAX_VOIP_FLASH_WRITE_CLIENT,
	VOIP_FLASH_WRITE_CLIENT_IVRSERVER = MAX_VOIP_FLASH_WRITE_CLIENT,
} cid_t;

// ---------------------------------------------------------------
// Flash Interface
// ---------------------------------------------------------------

int flash_voip_default(voipCfgParam_t *pVoIPCfg);

int flash_voip_check(voipCfgParam_t *pVoIPCfg);

int flash_voip_cmd(int param_cnt, char *param_var[]);

/**
 * @ingroup VOIP_FLASH
 * @brief get voip configurations from flash
 * @param cfg_all The voip configuration.
 * @param mode used in 8186 only (VOIP_CURRENT_SETTING or VOIP_DEFAULT_SETTING)
 * @retval 0 Success
 */
int voip_flash_read(voipCfgAll_t *cfg_all, int mode);

/**
 * @ingroup VOIP_FLASH
 * @brief set voip configurations to flash
 * @param cfg_all The voip configuration.
 * @retval 0 Success
 */
int voip_flash_write(voipCfgAll_t *cfg_all);

/**
 * @ingroup VOIP_FLASH
 * @brief get voip configuration from temporary memory
 * @param ppVoIPCfg The voip configuration.
 * @retval 0 Success
 * @note It is workable after voip_flash_read or web has started
 */
int voip_flash_get(voipCfgParam_t **ppVoIPCfg);
int voip_flash_get_default(voipCfgParam_t **ppVoIPCfg);

/**
 * @ingroup VOIP_FLASH
 * @brief set voip configuration via temporary memory
 * @param pVoIPCfg The voip configuration.
 * @retval 0 Success
 */
int voip_flash_set(voipCfgParam_t *pVoIPCfg);

int voip_flash_server_read(voip_flash_share_t *pVoIPShare);
int voip_flash_server_write(voip_flash_share_t *pVoIPShare);

int voip_flash_server_start(void);
void voip_flash_server_stop(void);
int voip_flash_server_update(void);

int voip_flash_client_init(voip_flash_share_t **ppVoIPShare, cid_t cid);
void voip_flash_client_close(void);
int voip_flash_client_update(void);

extern unsigned char GetDhcpValueToSetFixedIP( void );
extern int net_cfg_flash_read( net_cfg_t cfg, void *pCfgData );
extern int GetGatewayOperationMode( void );

// ---------------------------------------------------------------
// Flash Variables
// ---------------------------------------------------------------

extern voipCfgParam_t voipCfgParamDefault;
#if CONFIG_RTK_VOIP_PACKAGE_867X
extern voipCfgParam_t VoipEntry;
#endif

#endif
