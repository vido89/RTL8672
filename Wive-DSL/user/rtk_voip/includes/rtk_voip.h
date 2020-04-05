
#ifndef _RTK_VOIP_H
#define _RTK_VOIP_H
#include <linux/config.h>

#define VOIP_SIP_VERSION "0.8"
#define VOIP_DSP_VERSION "0.8"

#define PCM_HANDLER_USE_TASKLET

#ifdef PCM_HANDLER_USE_TASKLET
#define SUPPORT_PCM_FIFO
	#ifndef CONFIG_RTK_VOIP_MODULE
#define FEATURE_DMEM_STACK_CLI
	#endif /* CONFIG_RTK_VOIP_MODULE */
#define PCM_HANDLER_USE_CLI
#else
#define RTP_TX_USE_TASKLET		/* To avoid wlan_tx() called when interrupt is disabled */
#endif


//#define LEC_G168_ISR_SYNC_P
#define SUPPORT_LEC_G168_ISR		/* SUPPORT_LEC_G168_ISR is definded for ATA.*/
//#define SUPPORT_AES_ISR //for DAA channel

#ifdef SUPPORT_LEC_G168_ISR
#ifndef SUPPORT_PCM_FIFO
#define SUPPORT_PCM_FIFO
#endif
#define LEC_USE_CIRC_BUF		/* using circular buffer in LEC, for out-of-order tx rx isr */
#endif

#ifdef SUPPORT_PCM_FIFO			/* If SUPPORT_PCM_FIFO is defined, following items must be defined. */
#define PCM_FIFO_SIZE 		10
#define PCM_PERIOD			2	/* Unit: 10ms */
#define PCM_PERIOD_10MS_SIZE	160	/* Unit: byte */
#define TX_FIFO_START_NUM 	(PCM_PERIOD)	/* PCM_PERIOD <= TX_FIFO_START_NUM < PCM_FIFO_SIZE */

#define REDUCE_PCM_FIFO_MEMCPY
#ifdef CONFIG_RTK_VOIP_G7231
#if ! defined (AUDIOCODES_VOIP)
/**
 * PCM_handler() consumes and produces 10-ms voice.
 * DspProcess() consumes and produces one frame every time.
 * So, DspProcess() is called only if its frame is full.
 */
#define SUPPORT_CUT_DSP_PROCESS
#endif
#endif
#endif


#ifdef CONFIG_RTK_VOIP_MODULE
#define SYSTEM_IMEM 0			/* 1: enable set_system_imem() in DspProcess() and G.72x codec imem will be set every frame.
                                   	   0: disable set_system_imem  and G.72x codec imem will be set once if codec doesn't change.
                                   	 */
#else
#define SYSTEM_IMEM 1			/* 1: enable set_system_imem() in DspProcess() and G.72x codec imem will be set every frame.
                                   	   0: disable set_system_imem  and G.72x codec imem will be set once if codec doesn't change.
                                   	 */
#endif

//#define SIMPLIFIED_AP 		/* i.e. Simplified WSOLA algorithm, fast but may induce lower MOS value */
#define DISABLE_DEC_SLOW 		/* if SUPPORT_DYNAMIC_PAYLOAD is enabled, DEC_SLOW is not used (including G.723!! ) */


#define SUPPORT_3WAYS_AUDIOCONF
#define SUPPORT_ADJUST_JITTER
#ifdef SUPPORT_ADJUST_JITTER
  #define SUPPORT_DYNAMIC_JITTER_DELAY
  #define SUPPORT_IDEAL_MODE_JITTER_DELAY	/* Ideal mode cause minimum delay */
#endif


#define SUPPORT_COMFORT_NOISE

#ifdef SUPPORT_COMFORT_NOISE
 #define SIMPLIFIED_COMFORT_NOISE	/* for g711/g726 only */
 #define SIMPLIFIED_CN_VER	3	/* 1: all zeros, 2: plc, 3: by NoiseLevel */

 /* If not defined CONFIG_RTK_VOIP_G729AB, we can use SIMPLIFIED_COMFORT_NOISE only. */
 #if !defined( CONFIG_RTK_VOIP_G729AB ) && !defined( SIMPLIFIED_COMFORT_NOISE )
  #undef SUPPORT_COMFORT_NOISE
 #endif
#endif


#define SUPPORT_TONE_PROFILE		/* support more tone of different countries. Please refer to "voip_params.h" for detail. */

#define SUPPORT_DETECT_LONG_TERM_NO_RTP


#ifdef CONFIG_RTK_VOIP_SLIC_NUM_1
#define	SLIC_CH_NUM		1		/* Support PCM channel number, number range is 1~4. */
#endif
#ifdef CONFIG_RTK_VOIP_SLIC_NUM_2
#define	SLIC_CH_NUM		2		/* Support PCM channel number, number range is 1~4. */
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
#define	SLIC_CH_NUM		1		/* Support PCM channel number, number range is 1~4. */
#endif

//#if (VOIP_CH_NUM > 1)
//#define SIMPLIFIED_TWO_CHANNEL_729
//#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_SI3050
#define VIRTUAL_DAA_CH_NUM	0
#ifdef CONFIG_RTK_VOIP_DAA_NUM_1
#define DAA_CH_NUM		1
#elif defined CONFIG_RTK_VOIP_DAA_NUM_2
#define DAA_CH_NUM		2
#endif
#elif defined (CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA)
#define DAA_CH_NUM		0
#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA
#define VIRTUAL_DAA_CH_NUM	1
#elif defined (CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA_2_RELAY_SUPPORT)
#define VIRTUAL_DAA_CH_NUM	2
#endif
#else
#define DAA_CH_NUM		0
#define VIRTUAL_DAA_CH_NUM	0
#endif

#define PCM_CH_NUM 		4		/* Support PCM channel number, Max number is 4. */
#define MAX_VOIP_CH_NUM		4
#define VOIP_CH_NUM		(SLIC_CH_NUM + DAA_CH_NUM)

#define MAX_SESS_NUM 		2*MAX_VOIP_CH_NUM
#define SESS_NUM		2*VOIP_CH_NUM

#if ! defined (AUDIOCODES_VOIP)

#else
//#include "acmw_userdef.h"
//#define AUDIOCODES_VOTING_MECHANISM
#define ACMWPCM_HANDLER	1
#define ACMW_PLAYBACK	1	// 1: use AudioCodes IVR, 0: use RTK IVR
#endif /*AUDIOCODES_VOIP*/

#define MAX_RTP_TRAP_SESSION	2*SESS_NUM

/* rtcp definition */
/* define SUPPORT_RTCP to support RTCP.
 * It also need to define it in voip_manger.c for user space.
 * Thlin add 2006-07-04
 */
#define SUPPORT_RTCP
/* rtcp mid offset*/
#define RTCP_SID_OFFSET		SESS_NUM


// dtmf definition
#define CH_TONE 2			/* number of channel of playtone function */

// dynamic payload & multi-frame
#define SUPPORT_DYNAMIC_PAYLOAD
#define SUPPORT_MULTI_FRAME
#define MULTI_FRAME_BUFFER_SIZE	480	/* 6 frames per packet (maximum size of one frame is 80 when g711 codec) */
#define SUPPORT_BASEFRAME_10MS		/* undefine this MACRO will not work! */
//#define SUPPORT_FORCE_VAD
#ifdef SUPPORT_DYNAMIC_PAYLOAD
//#define DYNAMIC_PAYLOAD_VER1
#endif

#if defined( SUPPORT_DYNAMIC_PAYLOAD ) && !defined( DYNAMIC_PAYLOAD_VER1 )
#define SUPPORT_APPENDIX_SID		/* some packets contain voice in font of SID */
#define RESERVE_SPACE_FOR_SLOW		/* to place out of order packet into correct position */
#endif

#define NEW_JITTER_BUFFER_WI_DESIGN
#define CLEAN_JITTER_BUFFER_PARAMS

//#define SUPPORT_CODEC_DESCRIPTOR

//#define SUPPORT_CUSTOMIZE_FRAME_SIZE	/* turn on 'frame per packet' option in web configuration */

/* ================== DTMF DETECTION ==================== */
#if ! defined (AUDIOCODES_VOIP)
#define DTMF_DEC_ISR
#endif

#ifdef DTMF_DEC_ISR
#define DTMF_REMOVAL_ISR
#define DTMF_REMOVAL_FORWARD
#endif

#ifdef DTMF_REMOVAL_FORWARD
#define DTMF_REMOVAL_FORWARD_SIZE 3	/* removal length is (3 + PCM_PERIOD) */
					/*
					 * Forward remove DTMF_REMOVAL_FORWARD_SIZE*10 ms.
					 * The larger size, DTMF removal more clean, but longer delay.
					 */
#endif

/* ====================FAX DETECTION ==================== */
#ifdef DTMF_DEC_ISR
#define SUPPORT_FAX_PASS_ISR
#endif

/* ================== RFC2833 SEND =================== */
#define SUPPORT_RFC_2833

#ifdef DTMF_DEC_ISR
#define SEND_RFC2833_ISR
//#define RTP_SNED_TASKLET		/* To avoid wlan_tx() called when interrupt is disabled */
								/* Thlin: Enable rtp send tasklet to send RFC2833 packets in tasklet. */
#endif

/* ====================================================== */


/* software DTMF CID generate */
#define SW_DTMF_CID

#define SUPPORT_USERDEFINE_TONE

#define CHANNEL_NULL	0xff
#define SESSION_NULL	0xff

#ifdef SUPPORT_3WAYS_AUDIOCONF
	#define CONF_OFF	0xff
#endif


#define EVENT_POLLING_TIMER		/* Init a timer for Hook Polling usage. Accuracy: 10 ms */

//#define SUPPORT_VOICE_QOS // replaced by SUPPORT_DSCP.
#define SUPPORT_DSCP //  Move SIP and RTP QoS setting UI to VoIP "Other" page and support dynamic DSCP settings for SIP and RTP.

#ifdef CONFIG_RTK_VOIP_DRIVERS_8186V_ROUTER
#define CONFIG_RTK_VOIP_WAN_VLAN // Support VLAN setting of WAN port on Web UI!
#define CONFIG_RTK_VOIP_CLONE_MAC // Support WAN MAC CLONE for RTL8306
#define SUPPORT_IP_ADDR_QOS // Note: SUPPORT_IP_ADDR_QOS will casue packet lost temporarily!
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
#ifndef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
#define CONFIG_RTK_VOIP_LED	       /* V210 EV Board LED Control */
#endif
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8672
#define VOIP_YCJJ_ALGORITHM_TYPE1
#define VOIP_CPU_CACHE_WRITE_BACK
#endif

#if defined (CONFIG_RTK_VOIP_DRIVERS_FXO) && !defined (CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA)

/****** For SLIC and DAA Negotiation *****/
#ifdef CONFIG_RTK_VOIP_DRIVERS_SI3050
#define CH_NUM_DAA	1 		// The number of the DAA which can support negotiation with SLIC
#define DAA_RX_DET
#if ! defined (AUDIOCODES_VOIP)
//#define SUPPORT_BANDPASS_FOR_DAA	//add 200-3400hz bandpass for DAA RX
#define FXO_BUSY_TONE_DET
#define FXO_CALLER_ID_DET
#endif
#else
#define CH_NUM_DAA	0
#endif
/*****************************************/
#else
#define CH_NUM_DAA	0
#endif


/********** For voice record DEBUG ******/
//#define RTK_VOICE_RECORD
#ifdef RTK_VOICE_RECORD
#define DATAGETBUFSIZE	(10*1120)	//10*1120byte = 700*80short 700ms voice date
#define EACH_DATAGETBUFSIZE 1120
#endif //#ifdef RTK_VOICE_RECORD

/********** For experimental AEC ******/
//#define EXPER_AEC
/********** For experimental NR  *****/
//#define EXPER_NR


#define OPTIMIZATION

#define ENERGY_DET_PCM_IN

#include "voip_feature.h"

extern char bDebugMsg;
extern char bBootMsg;
#define PRINT_MSG(fmt, args...) do { if (bDebugMsg) printk(fmt, ## args); } while (0)
#define BOOT_MSG(fmt, args...) do { if (bBootMsg) printk(fmt, ## args);} while (0)


//#define SUPPORT_SLIC_GAIN_CFG // define this to enable SLIC gain config (include DTMF compensation)

#ifdef CONFIG_RTK_VOIP_T38
 #define T38_STAND_ALONE_HANDLER
#endif

#define VOIP_RESOURCE_CHECK	// Define VOIP_RESOURCE_CHECK to enable VoIP resource check.
				// Max. resource is two encode/decode channel.
				// VOIP_RESOURCE_CHECK is only for Realtek Solution, AudioCodes always check resource(2 channel).

#if defined(CONFIG_RTK_VOIP_IP_PHONE) || defined(CONFIG_CWMP_TR069)
#define SUPPORT_VOIP_FLASH_WRITE	/* flash write module */
#endif

//#define SUPPORT_IVR_HOLD_TONE	/* Use IVR G.723 to play HOLD tone */

/* For voice gain adjust object (note: adjust object only can adjust once) */
#define VOICE_GAIN_ADJUST_IVR
#define VOICE_GAIN_ADJUST_VOICE
//#define VOICE_GAIN_ADJUST_TONE_VOICE
//#define VOICE_GAIN_ADJUST_IVR_TONE_VOICE

#define PCM_LOOP_MODE_DRIVER
//#define PCM_LOOP_MODE_CONTROL

#define NEW_REMOTE_TONE_ENTRY
#ifndef CONFIG_RTK_VOIP_IP_PHONE
#define DISABLE_NEW_REMOTE_TONE	/* disable new remote tone, so we suggest to comment it */
#endif

#define AC_RESET		"\x1B[0m"	/* clears all colors and styles (to white on black) */
#define AC_FORE_RED		"\x1B[31m"	/* foreground red */
#define AC_FORE_GREEN		"\x1B[32m"	/* foreground green */
#define AC_FORE_YELLOW		"\x1B[33m"	/* foreground yellow */
#define AC_FORE_BlUE		"\x1B[34m"	/* foreground blue */

#endif //_RTK_VOIP_H

