#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/wrapper.h>
#include <asm/uaccess.h>
#include <asm/unaligned.h>
#include <linux/config.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/stddef.h>
#include <linux/netfilter.h>
#include <linux/delay.h>  	// udelay

#if ! defined (AUDIOCODES_VOIP)

#include "../include/type.h"
#include "dsp_main.h"
#include "rtk_voip.h"
#include "../voip_dsp/rtp/Rtcp.h"
#include "../voip_rx/rtk_trap.h"
#include "../voip_drivers/daa.h"
#include "voip_mgr_netfilter.h"
#include "../voip_drivers/pcm_interface.h"
#include "../voip_drivers/fsk.h"
#include "../voip_drivers/Slic_api.h"
#include "../voip_drivers/spi.h"
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
#include "voip_support.h"
#endif
#include "../voip_dsp/dsp_r0/dspparam.h"
#ifdef VIRTUAL_DAA
#include "../voip_drivers/virtual_daa.h"
#endif
#ifdef CONFIG_RTK_VOIP_IVR
#include "../voip_dsp/ivr/ivr.h"
#endif
#ifdef SUPPORT_LEC_G168_ISR
#include "../voip_dsp/include/lec.h"
#endif
#ifdef 	CONFIG_RTK_VOIP_LED
#include "../voip_drivers/led.h"
#endif
#ifdef CONFIG_RTK_VOIP_IP_PHONE
#include "../voip_drivers/iphone/WM8510.h"	/* for type of AI_AO_select() */
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221
#include "../voip_drivers/spi.h"		/* for slic oh-hook or off-hook stat pulling */
#endif

#else /*AUDIOCODES_VOIP*/


#include <linux/types.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <asm/io.h>
#include <asm/system.h>
#include <linux/init.h>
#include <linux/devfs_fs_kernel.h>
#include <linux/spinlock.h>
#include <asm/semaphore.h>

#include "RTK_AC49xApi_Interface.h"
#include "userdef.h"
#include "voip_mgr_netfilter.h"
#include "rtk_voip.h"
#include "pcm_interface.h"
#include "Slic_api.h"
#include "spi.h"
#include "daa.h"
#include "fsk.h"
#ifdef VIRTUAL_DAA
#include "virtual_daa.h"
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
#include "voip_support.h"
#endif
#include "../include/type.h"
#include "../voip_rx/rtk_trap.h"

#endif /*AUDIOCODES_VOIP*/

#ifdef T38_STAND_ALONE_HANDLER
#include "../voip_drivers/t38_handler.h"
#endif /* T38_STAND_ALONE_HANDLER */

#ifdef SUPPORT_DSCP
int rtp_tos = 0;
#endif

unsigned int gVoipFeature = 0;
int eng_det_flag[PCM_CH_NUM] = {0};
/**********For channel 1 testing*******/
//#define _chiminer_slic2_ch1
#define PCM_CHCNR *((volatile unsigned int *)0xBD280004)
TstTwoChannelCfg astTwoChannelCfg[SLIC_CH_NUM];

#if ! defined (AUDIOCODES_VOIP)

//global
int dsp_init_first = NULL;
int g_voip_debug = 0;

//choice 8305SC or 8306SD
#if 0
#define _8305SC_ // doesn't work
#else
#define _8306SD_
#endif

//external_mode=0: gateway_mode, external_mode=1: bridge_mode
static unsigned int external_mode = 0;
// static variable
//static rtp_config_t rtp_config[MAX_SLIC_CH_NUM];
static TstVoipPayLoadTypeConfig astVoipPayLoadTypeConfig[MAX_SESS_NUM];

static unsigned char pcm_flag[MAX_SLIC_CH_NUM] = {0};

extern struct pcm_priv* pcmctrl_devices;
extern int DSP_init_done ;
extern Tuint32 rtpConfigOK[MAX_SESS_NUM];
extern uint32 rtpHold[MAX_SESS_NUM];
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
extern struct RTK_TRAP_profile *filter[];
#endif

//-------------- For FAX Detection -------------
extern unsigned char fax_offhook[];
//----------- For dtmf cid generation ----------
#ifdef SW_DTMF_CID
extern char dtmf_cid_state[];
extern char cid_str[];
#endif
extern char cid_dtmf_mode[];
//----------- For dtmf removal -----------------
extern char dtmf_mode[];
extern unsigned char dtmf_removal_flag[];
//----------------------------------------------
#ifdef SUPPORT_LEC_G168_ISR
extern unsigned int Tap_len;	// LEC coef length
#endif

#ifdef SUPPORT_SLIC_GAIN_CFG
extern uint32 g_txVolumneGain[];
extern uint32 g_rxVolumneGain[];
#endif

#ifdef CONFIG_RTK_SLIC_DAA_NEGOTIATION
extern uint32 g_txVolumneGain_DAA;
extern uint32 g_rxVolumneGain_DAA;
#endif

extern int hook_in(uint32 ch_id, char input);
extern int hook_out(uint32 ch_id, char * output);
extern unsigned int flash_hook_time;
extern unsigned int flash_hook_min_time;
extern unsigned char RtcpOpen[];
extern unsigned char RtpOpen[];
extern uint32 cust;

#ifdef SUPPORT_TONE_PROFILE
extern ToneCfgParam_t ToneTable[];
#else
extern short ToneTable[][];
#endif

extern long talk_flag[];
extern unsigned char support_lec_g168[];
extern unsigned char rfc2833_payload_type_local[];
extern unsigned char rfc2833_payload_type_remote[];
unsigned char remote_is_rfc2833[MAX_SLIC_CH_NUM]; /* 0: remote is not rfc2833, don't do dtmf removal, else is rfc2833, do dtmf removal */

extern int g_SIP_Info_flag[];		/* 0: skip  1: run  SIP Info play routine */
extern int g_SIP_Info_play[];		/* 0: stop 1: start play */
extern int g_SIP_Info_tone[];		/* the tone of SIP Info play */
extern int g_SIP_Info_play_time[];	/* SIP Info play tone duration */
extern int g_SIP_Info_tone_buf[][10];
extern int g_SIP_Info_time_buf[][10];
extern int g_SIP_Info_buf_w[];

/* agc */
extern unsigned char spk_agc_mode[];
extern unsigned char spk_agc_lvl[];
extern unsigned char spk_agc_gup[];
extern unsigned char spk_agc_gdown[];
extern unsigned char mic_agc_mode[];
extern unsigned char mic_agc_lvl[];
extern unsigned char mic_agc_gup[];
extern unsigned char mic_agc_gdown[];

#else /*AUDIOCODES_VOIP*/

uint32 g_txVolumneGain[MAX_SLIC_CH_NUM];
uint32 g_rxVolumneGain[MAX_SLIC_CH_NUM];

//external_mode=0: gateway_mode, external_mode=1: bridge_mode
static unsigned int external_mode = 0;

//choice 8305SC or 8306SD
#if 0
#define _8305SC_
#else
#define _8306SD_
#endif
static TstVoipPayLoadTypeConfig astVoipPayLoadTypeConfig[MAX_SESS_NUM];
static unsigned char pcm_flag[MAX_SLIC_CH_NUM] = {0};
static int set_3way[MAX_SLIC_CH_NUM] = {0}, active_ch; //3-way conf: set_3way=1, else 0.
static char dtmf_mode[MAX_SLIC_CH_NUM] = {0};
static unsigned char rfc2833_payload_type_local[MAX_SESS_NUM];
static unsigned char rfc2833_payload_type_remote[MAX_SESS_NUM];
static int IsMode3[MAX_SLIC_CH_NUM] = {0}; 
extern struct pcm_priv* pcmctrl_devices;
extern Tuint32 rtpConfigOK[MAX_SESS_NUM];
extern uint32 rtpHold[MAX_SESS_NUM];
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
extern struct RTK_TRAP_profile *filter[];
#endif
extern void SLIC_reset(int CH, int codec_law, unsigned char slic_number);
extern void CID_for_FSK(unsigned int chid, char mode, unsigned char msg_type, char *str, char *str2);
extern void FXS_Ring(ring_struct *ring);
extern void Set_SLIC_Tx_Gain(unsigned char chid, unsigned char tx_gain);
extern void Set_SLIC_Rx_Gain(unsigned char chid, unsigned char rx_gain);
#ifdef CONFIG_RTK_SLIC_DAA_NEGOTIATION
extern uint32 g_txVolumneGain_DAA;
extern uint32 g_rxVolumneGain_DAA;
#endif
extern int hook_out(uint32 ch_id, char * output);
extern unsigned int flash_hook_time;
extern unsigned int flash_hook_min_time;
extern int32 Ac49xTxPacketCB( uint32 channel, uint32 mid, void* packet, uint32 pktLen, uint32 flags );
char fsk_spec_areas[4];	// bit0-2: FSK Type
			// bit 3: Normal Ring
			// bit 4: Fsk Alert Tone
			// bit 5: Short Ring
			// bit 6: Line Reverse
			// bit 7: Date, Time Sync and Name
int fsk_cid_gain[MAX_SLIC_CH_NUM] = {1};
char dtmf_cid_state[MAX_SLIC_CH_NUM]={0};
unsigned char cid_str[21];
char cid_dtmf_mode[MAX_SLIC_CH_NUM];     // for DTMF start/end digit
int tone_idx;
int CustomToneTable[8][24];

unsigned char spk_agc_mode[MAX_SLIC_CH_NUM];
unsigned char mic_agc_mode[MAX_SLIC_CH_NUM];
unsigned char agc_enable[MAX_SLIC_CH_NUM]={0};
unsigned char spk_agc_gup;
unsigned char spk_agc_gdown;
unsigned char mic_agc_gup;
unsigned char mic_agc_gdown;
#endif /*AUDIOCODES_VOIP*/


#ifdef CONFIG_RTK_SLIC_DAA_NEGOTIATION
extern unsigned int DAA_ISR_FLOW[];
#endif
int sid_owner[MAX_SESS_NUM]= {0};   /* Session own by 0: SLIC, 1: DAA */
extern int g_DAA_used[];

#ifdef FXO_BUSY_TONE_DET
#include "../voip_dsp/include/tone_det_i.h"
#endif

#ifdef FXO_CALLER_ID_DET
#include "../voip_dsp/include/fsk_det.h"
#include "../voip_dsp/include/dtmf_dec.h"
int fsk_cid_valid[MAX_SLIC_CH_NUM] = {0};
int dtmf_cid_valid[MAX_SLIC_CH_NUM] = {0};
#endif

extern unsigned int fsk_gen_mode[];	// 0: hardware FSK caller id, 1:software FSK caller id

#ifdef RTK_VOICE_RECORD
#include "voip_control.h"
extern TstVoipdataget stVoipdataget[];
TstVoipdataget_o stVoipdataget_o;
#endif //#ifdef RTK_VOICE_RECORD

extern long voice_gain_spk[];//0 is mute, 1 is -31dB ~~~ 32 is 0dB , 33 is 1dB ~~~ 63 is 31dB
extern long voice_gain_mic[];

unsigned int fax_end_flag[MAX_SLIC_CH_NUM];      //for t.38 fax end detect.

/*
void dsp_process_rtp_rx(Tuint8 CH, Tuint32 media_type, void *ptr_data, Tint32 data_len)
{
	int test;
#if 0
	test = ((int *)ptr_data)[0];
	PRINT_MSG("sequence = %x\n",test);
#endif
	kfree(ptr_data);
}
*/

#ifdef CONFIG_RTK_VOIP_LED
        volatile unsigned int sip_registed[MAX_SLIC_CH_NUM];     //register ok:1, no register:0
	volatile unsigned int pstn_ringing;	//pstn incoming ring:1 ,no ring and voip incoming ring:0
	volatile unsigned int daa_hook_status;  //off-hook:1 ,on-hook:0
        volatile unsigned int slic_hook_status[MAX_SLIC_CH_NUM]; //off-hook:1 ,on-hook:0 ,flash-hook:2
        volatile unsigned int fxs_ringing[MAX_SLIC_CH_NUM];                      //no ring:0 ,voip incoming ring:1
	volatile unsigned int daa_ringing;
#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA
        extern char relay_2_PSTN_flag[MAX_SLIC_CH_NUM]; /* 1: relay to PSTN , 0: relay to SLIC */
#endif
void led_state_watcher(unsigned int chid)
{
	// FXS
	if (fxs_ringing[chid] || slic_hook_status[chid])
		fxs_led_state(chid, 2); // blinking
	else {
		if (sip_registed[chid]) {
			fxs_led_state(chid, 1); // on
		} else {
			fxs_led_state(chid, 0); // off
		}
	}	

	// PSTN
#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA
	if (pstn_ringing || relay_2_PSTN_flag[0] || relay_2_PSTN_flag[1]) {
#else
	if (pstn_ringing) {
#endif
		FXO_LED_STATE(2); // blinking
	} else 	{
#if SLIC_CH_NUM == 2	
		if (sip_registed[0] && sip_registed[1]) {
#else
		if (sip_registed[chid]) {
#endif	
			FXO_LED_STATE(0); // off
		} else {
			FXO_LED_STATE(1); // on
		}
	}
}
#endif


#ifdef AUDIOCODES_VOIP

/************************************************************************/
/*									*/
/*  voip_mgr_set_rtp_1: This function register RTP session              */
/*									*/
/************************************************************************/

int voip_mgr_set_rtp_1(TstVoipMgrRtpCfg stVoipMgrRtpCfg)
{
      	int ret = NULL;
      	Tuint32 s_id_rtp, s_id_rtcp;
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
	Tint32 result;
	voip_rtp_port_t rtp_pair;
#endif
	unsigned long flags;
	TstVoipMgrSession stVoipMgrSession;
	
	stVoipMgrSession.ch_id = stVoipMgrRtpCfg.ch_id;
	stVoipMgrSession.m_id = stVoipMgrRtpCfg.m_id;
	stVoipMgrSession.ip_src_addr = stVoipMgrRtpCfg.ip_src_addr;
	stVoipMgrSession.ip_dst_addr = stVoipMgrRtpCfg.ip_dst_addr;
	stVoipMgrSession.udp_src_port = stVoipMgrRtpCfg.udp_src_port;
	stVoipMgrSession.udp_dst_port = stVoipMgrRtpCfg.udp_dst_port;
	stVoipMgrSession.protocol = stVoipMgrRtpCfg.protocol;
	stVoipMgrSession.tos = stVoipMgrRtpCfg.tos;
			
	stVoipMgrSession.protocol = UDP_PROTOCOL;

	s_id_rtp = (2*stVoipMgrRtpCfg.ch_id+PROTOCOL__RTP-1);
	s_id_rtcp = (2*stVoipMgrRtpCfg.ch_id+PROTOCOL__RTCP-1);
	PRINT_MSG("SET RTP(%d)\n", s_id_rtp);
	//PRINT_MSG("VOIP_MGR_SETRTPPAYLOADTYPE(ch=%d, mid=%d)\n", stVoipMgrRtpCfg.ch_id, stVoipMgrRtpCfg.m_id);
	//PRINT_MSG("VOIP_MGR_RTP_CFG:ch_id = %d, state = %d\n", stVoipMgrRtpCfg.ch_id, stVoipMgrRtpCfg.state);

	
	#if 1
	PRINT_MSG("ch_id = %d\n", stVoipMgrSession.ch_id);
	PRINT_MSG("m_id = %d\n", stVoipMgrSession.m_id);
	PRINT_MSG("Rtp s_id = %d\n", s_id_rtp);
	PRINT_MSG("Rtp ip_src_addr = %d.%d.%d.%d\n", ((unsigned char *)(&stVoipMgrSession.ip_src_addr))[0], ((unsigned char *)(&stVoipMgrSession.ip_src_addr))[1], ((unsigned char *)(&stVoipMgrSession.ip_src_addr))[2], ((unsigned char *)(&stVoipMgrSession.ip_src_addr))[3]);
	PRINT_MSG("Rtp ip_dst_addr = %d.%d.%d.%d\n", ((unsigned char *)(&stVoipMgrSession.ip_dst_addr))[0], ((unsigned char *)(&stVoipMgrSession.ip_dst_addr))[1], ((unsigned char *)(&stVoipMgrSession.ip_dst_addr))[2], ((unsigned char *)(&stVoipMgrSession.ip_dst_addr))[3]);
	PRINT_MSG("Rtp udp_src_port = %d\n", ntohs(stVoipMgrSession.udp_src_port));
	PRINT_MSG("Rtp udp_dst_port = %d\n", ntohs(stVoipMgrSession.udp_dst_port));
	PRINT_MSG("protocol = 0x%x\n", stVoipMgrSession.protocol);
	#endif

	#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
	if(filter[s_id_rtp]!=0) 
	{
         	PRINT_MSG("rtp: s_id %d in used, please unregister first\n", s_id_rtp);
		return -1;
        }
	if(filter[s_id_rtcp]!=0) 
	{
         	PRINT_MSG("rtcp: s_id %d in used, please unregister first\n", s_id_rtcp);
		return -1;
        }
	#endif
	
	save_flags(flags); cli();
	#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_RX)
	rtp_pair.isTcp = FALSE;
	rtp_pair.remIp = stVoipMgrRtpCfg.ip_src_addr;
	rtp_pair.remPort = stVoipMgrRtpCfg.udp_src_port;
	rtp_pair.extIp = stVoipMgrRtpCfg.ip_dst_addr;
	rtp_pair.extPort = stVoipMgrRtpCfg.udp_dst_port;
	rtp_pair.chid = stVoipMgrRtpCfg.ch_id;
	/* rtp register */
	rtp_pair.mid = s_id_rtp;
	result = voip_register_RTPport( /*chid*/ rtp_pair.chid, /*mid*/ rtp_pair.mid, &rtp_pair );
	if(result < 0 )
		PRINT_MSG("865x register RTP failed\n");
	else
	{
		rfc2833_payload_type_local[s_id_rtp] = stVoipMgrRtpCfg.rfc2833_payload_type_local;
        	rfc2833_payload_type_remote[s_id_rtp] = stVoipMgrRtpCfg.rfc2833_payload_type_remote;
		PRINT_MSG("865x register RTP success\n");
        
        	/** Check if local GW is RFC2833 mode, and remote GW support RFC2833, then set ACMW to RFC2833 mode. **/
        	if ( dtmf_mode[stVoipMgrRtpCfg.ch_id] == 0 )/*RFC2833*/
		{
			if (rfc2833_payload_type_remote[s_id_rtp] != 0)
			{	
				RtkAc49xApiSetIbsTransferMode(stVoipMgrRtpCfg.ch_id, IBS_TRANSFER_MODE__RELAY_ENABLE_VOICE_MUTE);
				//RtkAc49xApiSetDtmfErasureMode(stVoipMgrRtpCfg.ch_id, DTMF_ERASURE_MODE__ERASE_1_COMPONENT);
			}
			else
			{
				RtkAc49xApiSetIbsTransferMode(stVoipMgrRtpCfg.ch_id, IBS_TRANSFER_MODE__TRANSPARENT_THROUGH_VOICE);
			}
		}
		/*******************************************************************************************************/
	}
	/* rtcp register (rtcp port = rtp port + 1) */
	rtp_pair.remPort = rtp_pair.remPort + 1;
	rtp_pair.extPort = rtp_pair.extPort + 1;	
	rtp_pair.mid = s_id_rtcp;
	result = voip_register_RTPport( /*chid*/ rtp_pair.chid, /*mid*/ rtp_pair.mid, &rtp_pair );
	if(result < 0 )
		PRINT_MSG("865x register RTP failed\n");
	else
		PRINT_MSG("865x register RTP success\n");
	#else
	/* rtp register */
	stVoipMgrSession.m_id = PROTOCOL__RTP;
	rtk_trap_register(&stVoipMgrSession, stVoipMgrSession.ch_id, stVoipMgrSession.m_id, s_id_rtp,  Ac49xTxPacketCB);
	/* rtcp register (rtcp port = rtp port + 1) */
	stVoipMgrSession.udp_src_port = stVoipMgrSession.udp_src_port + 1;
	stVoipMgrSession.udp_dst_port = stVoipMgrSession.udp_dst_port + 1;
	stVoipMgrSession.m_id = PROTOCOL__RTCP;
	rtk_trap_register(&stVoipMgrSession, stVoipMgrSession.ch_id, stVoipMgrSession.m_id, s_id_rtcp,  Ac49xTxPacketCB);
	rfc2833_payload_type_local[s_id_rtp] = stVoipMgrRtpCfg.rfc2833_payload_type_local;
        rfc2833_payload_type_remote[s_id_rtp] = stVoipMgrRtpCfg.rfc2833_payload_type_remote;
        
        /** Check if local GW is RFC2833 mode, and remote GW support RFC2833, then set ACMW to RFC2833 mode. **/
        if ( dtmf_mode[stVoipMgrRtpCfg.ch_id] == 0 )/*RFC2833*/
	{
		if (rfc2833_payload_type_remote[s_id_rtp] != 0) 
		{	
			RtkAc49xApiSetIbsTransferMode(stVoipMgrRtpCfg.ch_id, IBS_TRANSFER_MODE__RELAY_ENABLE_VOICE_MUTE);
			//RtkAc49xApiSetDtmfErasureMode(stVoipMgrRtpCfg.ch_id, DTMF_ERASURE_MODE__ERASE_1_COMPONENT);
		}
		else if (rfc2833_payload_type_remote[s_id_rtp] == 0) 
		{
			RtkAc49xApiSetIbsTransferMode(stVoipMgrRtpCfg.ch_id, IBS_TRANSFER_MODE__TRANSPARENT_THROUGH_VOICE);
		}
	}
	/*******************************************************************************************************/
	#endif
	
	restore_flags(flags);
	return 0;
}


/********************************************************************************/
/*										*/
/*  voip_mgr_set_rtp_2: This function set the payload type and active RTP       */
/*  Berfor calling voip_mgr_set_rtp_2, RTP session must be register.		*/
/*  I.e. voip_mgr_set_rtp_1 must be called.					*/
/*										*/
/********************************************************************************/

int voip_mgr_set_rtp_2(TstVoipMgrRtpCfg stVoipMgrRtpCfg)
{
      	int ret = NULL;
      	Tuint32 s_id_rtp, s_id_rtcp;
	unsigned long flags;

	s_id_rtp = (2*stVoipMgrRtpCfg.ch_id+PROTOCOL__RTP-1);
	s_id_rtcp = (2*stVoipMgrRtpCfg.ch_id+PROTOCOL__RTCP-1);

#ifdef T38_STAND_ALONE_HANDLER
	if( stVoipMgrRtpCfg.uPktFormat == rtpPayloadT38_Virtual ) 
	{
		T38_API_Initialize( stVoipMgrRtpCfg.ch_id );
		PRINT_MSG("MGR: Initialize T38(%d)\n", stVoipMgrRtpCfg.ch_id);
		t38RunningState[ stVoipMgrRtpCfg.ch_id ] = T38_START;
	}
	else
	{
		t38RunningState[ stVoipMgrRtpCfg.ch_id ] = T38_STOP;		
		RtkAc49xApiSetRtpChannelConfiguration(stVoipMgrRtpCfg.ch_id, s_id_rtp, stVoipMgrRtpCfg.uPktFormat, stVoipMgrRtpCfg.nG723Type, stVoipMgrRtpCfg.bVAD);
	}
#else
		RtkAc49xApiSetRtpChannelConfiguration(stVoipMgrRtpCfg.ch_id, s_id_rtp, stVoipMgrRtpCfg.uPktFormat, stVoipMgrRtpCfg.nG723Type, stVoipMgrRtpCfg.bVAD);	
#endif
		
	
     	if(stVoipMgrRtpCfg.state ==1)
      	{
      		if (rtpConfigOK[s_id_rtp] == 0)
			ret = RtkAc49xApiActiveRegularRtp(stVoipMgrRtpCfg.ch_id, s_id_rtp); /* return 0 : success */
		
		if (ret == 0)
		{
			rtpConfigOK[s_id_rtp] = 1;
			rtpConfigOK[s_id_rtcp] = 1;
			PRINT_MSG(".open RTP(%d)\n", s_id_rtp);
		}
	}
	else
	{
		if (rtpConfigOK[s_id_rtp] == 1)
			ret = RtkAc49xApiCloseRegularRtp(stVoipMgrRtpCfg.ch_id, s_id_rtp); /* return 0 : success */
		
		if (ret == 0)
		{
			rtpConfigOK[s_id_rtp] = 0;
			rtpConfigOK[s_id_rtcp] = 0;
			PRINT_MSG(".close RTP(%d)\n", s_id_rtp);
		}
	}
	
	restore_flags(flags);
	return 0;
	
}

/********************************************************************************/
/*										*/
/*  voip_mgr_unset_rtp_1: This function un-register RTP session			*/
/*										*/
/********************************************************************************/

/////// voip_mgr_set_rtp_2: This function set the payload type and active RTP ///////
int voip_mgr_unset_rtp_1(int chid, int mid)
{
	Tuint32 s_id_rtp, s_id_rtcp;
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
	Tint32 result;
	voip_rtp_port_t rtp_pair;
#endif
	int ret = NULL;
	unsigned long flags;
	
	s_id_rtp = (2*chid + PROTOCOL__RTP-1);
	s_id_rtcp = (2*chid + PROTOCOL__RTCP-1);
	PRINT_MSG("UNSET RTP(%d)\n", s_id_rtp);

	save_flags(flags); cli();
	
	#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_RX)
	rtp_pair.chid = stVoipCfg.ch_id;
	
	rtp_pair.mid = s_id_rtp;
	result = voip_register_RTPport( /*chid*/ rtp_pair.chid, /*mid*/ rtp_pair.mid, NULL );
	if( result < 0 )
		PRINT_MSG("Unregister 865x RTP port  failed\n");
        else
		PRINT_MSG("Unregister 865x RTP port  successfully\n");
	
	rtp_pair.mid = s_id_rtcp;
	result = voip_register_RTPport( /*chid*/ rtp_pair.chid, /*mid*/ rtp_pair.mid, NULL );
	if( result < 0 )
		PRINT_MSG("Unregister 865x RTP port  failed\n");
        else
		PRINT_MSG("Unregister 865x RTP port  successfully\n");       
	#else
	rtk_trap_unregister(s_id_rtp);
	rtk_trap_unregister(s_id_rtcp);
	#endif
	
	restore_flags(flags);
}

/********************************************************************************/
/*										*/
/*  voip_mgr_unset_rtp_2: This function de-active the RTP			*/
/*  Usually, voip_mgr_unset_rtp_1 should be used to unrgister and then          */
/*  call the voip_mgr_unset_rtp_2 to deactive RTP.				*/ 
/*										*/
/********************************************************************************/

int voip_mgr_unset_rtp_2(int chid, int mid)
{
	Tuint32 s_id_rtp, s_id_rtcp;

	int ret = NULL;
	unsigned long flags;
	
	s_id_rtp = (2*chid + PROTOCOL__RTP-1);
	s_id_rtcp = (2*chid + PROTOCOL__RTCP-1);

	save_flags(flags); cli();
	
	
	if (rtpConfigOK[s_id_rtp] == 1)
		ret = RtkAc49xApiCloseRegularRtp(chid, s_id_rtp); /* return 0 : success */
		
	if (ret == 0)
	{
		rtpConfigOK[s_id_rtp] = 0;
		rtpConfigOK[s_id_rtcp] = 0;
		PRINT_MSG("close RTP(%d)\n", s_id_rtp);
	}

	restore_flags(flags);
}

int SaveCustomTone(TstVoipToneCfg *pToneCfg)
{
	extern int CustomToneTable[8][24];
	
	/*    
	TONE_TYPE_ADDITIVE = 0
    	TONE_TYPE_MODULATED = 1
    	TONE_TYPE_SUCC = 2
    	TONE_TYPE_SUCC_ADD =3
    	*/
    	
	if (pToneCfg->toneType == 2)//SUCC
	{
		CustomToneTable[tone_idx][0] = CALL_PROGRESS_SIGNAL_TYPE__SPECIAL_INFORMATION_TONE ;
		//PRINT_MSG("SPECIAL_INFO...\n");
	}
	else
	{
		if (pToneCfg->cycle == 0)
		{
			CustomToneTable[tone_idx][0] = CALL_PROGRESS_SIGNAL_TYPE__CONTINUOUS;
			//PRINT_MSG("CONTINUOUS...\n");
		}
		else if (pToneCfg->cycle == 1)
		{
			CustomToneTable[tone_idx][0] = CALL_PROGRESS_SIGNAL_TYPE__BURST; // play one cycle
			//PRINT_MSG("BURST...\n");
		}
		else if (pToneCfg->cycle > 1)
		{
			CustomToneTable[tone_idx][0] = CALL_PROGRESS_SIGNAL_TYPE__CADENCE;
			//PRINT_MSG("CADENCE...\n");
		}
	}
	
	CustomToneTable[tone_idx][1] = pToneCfg->Freq1;		//ToneAFrequency 
	CustomToneTable[tone_idx][2] = pToneCfg->Freq2;		//ToneB_OrAmpModulationFrequency 
	CustomToneTable[tone_idx][3] = 10;			//TwistThreshold
	
	if (pToneCfg->toneType == 2)//SUCC
		CustomToneTable[tone_idx][4] = pToneCfg->Freq3;	//ThirdToneOfTripleBatchDurationTypeFrequency 
	else
		CustomToneTable[tone_idx][4] = 0;
	//PRINT_MSG("Freq3=%d\n", CustomToneTable[tone_idx][4]);


	CustomToneTable[tone_idx][5] = 0;			//HighEnergyThreshold 
	CustomToneTable[tone_idx][6] = 35;			//LowEnergyThreshold 
	CustomToneTable[tone_idx][7] = 15;			//SignalToNoiseRatioThreshold 
	CustomToneTable[tone_idx][8] = 10;			//FrequencyDeviationThreshold 
	CustomToneTable[tone_idx][9] = pToneCfg->Gain1;		//ToneALevel
	
	if ((pToneCfg->toneType == 1) || pToneCfg->toneType == 2)//Modulate or SUCC
		CustomToneTable[tone_idx][10] = 0;		//ToneBLevel 
	else
		CustomToneTable[tone_idx][10] = pToneCfg->Gain2;
		
	//PRINT_MSG("ToneBLevel=%d\n", CustomToneTable[tone_idx][10]);

	/*
	AM Factor is the AM Modulation index, its range us between 0%-100%
	AMFACTOR field range is between 0-50 (unit 0.02) corresponding to 0%-100%. 
	In the example I chose AMFACTOR=25 (50%)
	*/
       	if (pToneCfg->toneType == 1)//Modulate
		CustomToneTable[tone_idx][11] = 25;		//AmFactor
	else
		CustomToneTable[tone_idx][11] = 0;
		
	//PRINT_MSG("AmFactor=%d\n", CustomToneTable[tone_idx][11]);
		
	CustomToneTable[tone_idx][12] = (pToneCfg->CadOn0)/10;	//DetectionTimeOrCadenceFirstOnOrBurstDuration  
	CustomToneTable[tone_idx][13] = (pToneCfg->CadOff0)/10;	//CadenceFirstOffDuration 
	CustomToneTable[tone_idx][14] = (pToneCfg->CadOn1)/10;	//CadenceSecondOnDuration 
	CustomToneTable[tone_idx][15] = (pToneCfg->CadOff1)/10;	//CadenceSecondOffDuration 
	CustomToneTable[tone_idx][16] = (pToneCfg->CadOn2)/10;	//CadenceThirdOnDuration 
	CustomToneTable[tone_idx][17] = (pToneCfg->CadOff2)/10;	//CadenceThirdOffDuration 
	CustomToneTable[tone_idx][18] = (pToneCfg->CadOn3)/10;	//CadenceFourthOnDuration 
	CustomToneTable[tone_idx][19] = (pToneCfg->CadOff3)/10;	//CadenceFourthOffDuration
	CustomToneTable[tone_idx][20] = 0;			//CadenceVoiceAddedWhileFirstOff 
	CustomToneTable[tone_idx][21] = 0;			//CadenceVoiceAddedWhileSecondOff 
	CustomToneTable[tone_idx][22] = 0;			//CadenceVoiceAddedWhileThirdOff 
	CustomToneTable[tone_idx][23] = 0;			//CadenceVoiceAddedWhileFourthOff 

#if 0	
	pToneCfg->cadNUM;
	pToneCfg->PatternOff;
	pToneCfg->ToneNUM;
#endif
	return 0;
}

#endif //AUDIOCODES_VOIP


//#define TEST_UNALIGN
#ifdef TEST_UNALIGN
#include "../include/dmem_stack.h"
char test_data[8] = {0, 1, 2, 3, 4, 5, 6, 7};
int *test_p;
#endif
			
int do_voip_mgr_set_ctl(struct sock *sk, int cmd, void *user, unsigned int len)
{
	/* 
	 * If a local variable is merely used in a certain case, 
	 * we can share the memory. 
	 */
	union {
		TstVoipRtpStatistics	stInstVoipRtpStatistics;
#ifdef CONFIG_RTK_VOIP_IVR
		TstVoipPlayIVR_Header	stInstVoipPlayIVR_Header;
		TstVoipPollIVR		stInstVoipPollIVR;
		TstVoipStopIVR		stInstVoipStopIVR;
#endif		
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		TstVoicePath_t		stInstVoicePath;
#endif
		TstVoipMgr3WayCfg 	stInstVoipMgr3WayCfg;
		TstVoipMgrSession 	stInstVoipMgrSession;
		TstVoipRtcpSession 	stInstVoipRtcpSession;
		TstVoipPayLoadTypeConfig stInstVoipPayLoadTypeConfig;
		TstVoipTest 		stInstVoipTest;
		TstVoipRtpSessionState 	stInstVoipRtpSessionState;
		TstVoipPlayToneConfig 	stInstVoipPlayToneConfig;
		TstVoipCfg 		stInstVoipCfg;
		TstVoipValue 		stInstVoipValue;
		TstVoipSlicHook 	stInstVoipSlicHook;
		TstVoipSlicRing 	stInstVoipSlicRing;
		TstVoipSlicTone 	stInstVoipSlicTone;
		TstVoipSwitch 		Inst_switch_value;
		TstVoipSwitch_VLAN_tag 	Inst_wan_vlan_tag;
		TstVoipSwitch_2_VLAN_tag Inst_wan_2_vlan_tag;
		TstVoipSwitch_3_VLAN_tag Inst_wan_3_vlan_tag;
		TstVoipSlicRestart 	stInstVoipSlicRestart;
		TstVoipCID 		stInstCIDstr;//For dtmf cid generation
		TstVoip2833 		stInstRFC2833;//For sending RFC2833 packet
		TstVoipSlicReg		stInstSlicReg;
		TstVoipHook 		stInstHookTime;
		TstVoipToneCfg 		stInstVoipToneCfg;
		TstVoipResourceCheck 	stInstVoipResourceCheck;
		
	} local_share;

	/* Rename to conventional naming, and remember #undef in end of function. */
	#define stVoipRtpStatistics		local_share.stInstVoipRtpStatistics
	#define stVoipPlayIVR_Header		local_share.stInstVoipPlayIVR_Header
	#define stVoipPollIVR			local_share.stInstVoipPollIVR
	#define stVoipStopIVR			local_share.stInstVoipStopIVR
	#define stVoicePath			local_share.stInstVoicePath
	#define stVoipMgr3WayCfg		local_share.stInstVoipMgr3WayCfg	
	#define stVoipMgrSession		local_share.stInstVoipMgrSession	
	#define stVoipRtcpSession		local_share.stInstVoipRtcpSession	
	#define stVoipPayLoadTypeConfig		local_share.stInstVoipPayLoadTypeConfig	
	#define stVoipTest			local_share.stInstVoipTest		
	#define stVoipRtpSessionState		local_share.stInstVoipRtpSessionState	
	#define stVoipPlayToneConfig		local_share.stInstVoipPlayToneConfig	
	#define stVoipCfg			local_share.stInstVoipCfg		
	#define stVoipValue			local_share.stInstVoipValue		
	#define stVoipSlicHook			local_share.stInstVoipSlicHook		
	#define stVoipSlicRing			local_share.stInstVoipSlicRing		
	#define stVoipSlicTone			local_share.stInstVoipSlicTone		
	#define switch_value			local_share.Inst_switch_value		
	#define wan_vlan_tag			local_share.Inst_wan_vlan_tag		
	#define wan_2_vlan_tag			local_share.Inst_wan_2_vlan_tag		
	#define wan_3_vlan_tag			local_share.Inst_wan_3_vlan_tag		
	#define stVoipSlicRestart		local_share.stInstVoipSlicRestart	
	#define stCIDstr			local_share.stInstCIDstr		
	#define stRFC2833			local_share.stInstRFC2833		
	#define stSlicReg			local_share.stInstSlicReg		
	#define stHookTime			local_share.stInstHookTime		
	#define stVoipToneCfg			local_share.stInstVoipToneCfg		
	#define stVoipResourceCheck		local_share.stInstVoipResourceCheck	
	
	
	unsigned char bridge_mode;
	ring_struct ring;
	unsigned int data[4], i;
	unsigned long flags;
	Tuint32 daa_ring;
	short *pToneTable;
	Tuint32 ch_id, m_id, s_id, s_id_rtp, s_id_rtcp, temp;
	int ret = NULL;
		
#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE	
	IPhone_test iphone;
#endif	

#ifdef RTK_VOICE_RECORD
	//TstVoipdataget_o stVoipdataget_o;    move to globe, reduce stack size
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
	Tint32 result;
	voip_rtp_port_t rtp_pair;
#endif

#if ! defined (AUDIOCODES_VOIP)
	CRtpConfig RtpConfig;
#endif


  switch (cmd)
  {
  
#if ! defined (AUDIOCODES_VOIP)

	case VOIP_MGR_SET_SESSION:
		PRINT_MSG("SET RTP SESSION\n");
      		copy_from_user(&stVoipMgrSession, (TstVoipMgrSession *)user, sizeof(TstVoipMgrSession));
		s_id = API_GetSid(stVoipMgrSession.ch_id, stVoipMgrSession.m_id);
#if 0
		PRINT_MSG("ch_id = %d\n", stVoipMgrSession.ch_id);
		PRINT_MSG("m_id = %d\n", stVoipMgrSession.m_id);
		PRINT_MSG("Rtp s_id = %d\n", s_id);
		PRINT_MSG("Rtp ip_src_addr = %d.%d.%d.%d\n", ((unsigned char *)(&stVoipMgrSession.ip_src_addr))[0], ((unsigned char *)(&stVoipMgrSession.ip_src_addr))[1], ((unsigned char *)(&stVoipMgrSession.ip_src_addr))[2], ((unsigned char *)(&stVoipMgrSession.ip_src_addr))[3]);
		PRINT_MSG("Rtp ip_dst_addr = %d.%d.%d.%d\n", ((unsigned char *)(&stVoipMgrSession.ip_dst_addr))[0], ((unsigned char *)(&stVoipMgrSession.ip_dst_addr))[1], ((unsigned char *)(&stVoipMgrSession.ip_dst_addr))[2], ((unsigned char *)(&stVoipMgrSession.ip_dst_addr))[3]);
		PRINT_MSG("Rtp udp_src_port = %d\n", ntohs(stVoipMgrSession.udp_src_port));
		PRINT_MSG("Rtp udp_dst_port = %d\n", ntohs(stVoipMgrSession.udp_dst_port));
		PRINT_MSG("protocol = 0x%x\n", stVoipMgrSession.protocol);
#endif

		save_flags(flags); cli();

#ifdef SUPPORT_IP_ADDR_QOS
		disable_eth8186_rx();
		rtl8306_disable_all_ports();
		rtl8306_setAsicQosIPAddress(
			s_id%2,
			stVoipMgrSession.ip_src_addr,
			0xFFFFFFFF, 
			1);
		rtl8306_asicSoftReset();
		rtl8306_enable_all_ports();
		enable_eth8186_rx();
#endif
		
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
		if(filter[s_id]!=0) {
                  PRINT_MSG("rtp: s_id %d in used, please unregister first\n", s_id);
		  break;
        	}
#endif

		RtpSession_renew(s_id);
#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_RX)
		rtp_pair.isTcp = FALSE;
		rtp_pair.remIp = stVoipMgrSession.ip_src_addr;
		rtp_pair.remPort = stVoipMgrSession.udp_src_port;
		rtp_pair.extIp = stVoipMgrSession.ip_dst_addr;
		rtp_pair.extPort = stVoipMgrSession.udp_dst_port;
		rtp_pair.chid = stVoipMgrSession.ch_id;
		rtp_pair.mid = s_id;
		result = voip_register_RTPport( /*chid*/ rtp_pair.chid, /*mid*/ rtp_pair.mid, &rtp_pair );
		if(result < 0 )
		{
                        PRINT_MSG("865x register RTP failed\n");
                        RtpOpen[s_id] = 0;
		}
		else
		{
                        PRINT_MSG("865x register RTP success\n");
                        RtpOpen[s_id] = 1;
                        rfc2833_payload_type_local[s_id] = stVoipMgrSession.rfc2833_payload_type_local;
                        rfc2833_payload_type_remote[s_id] = stVoipMgrSession.rfc2833_payload_type_remote;
		}
#else
//added by eric for voip input queue
#ifdef 	CONFIG_RTK_VOIP_RX_INPUT_QUEUE
		set_input_rtp_port(stVoipMgrSession.udp_dst_port ,stVoipMgrSession.udp_dst_port+1,stVoipMgrSession.ch_id);
#endif
		rtk_trap_register(&stVoipMgrSession, stVoipMgrSession.ch_id, stVoipMgrSession.m_id, s_id,  DSP_pktRx);
                RtpOpen[s_id] = 1;
                rfc2833_payload_type_local[s_id] = stVoipMgrSession.rfc2833_payload_type_local;
                rfc2833_payload_type_remote[s_id] = stVoipMgrSession.rfc2833_payload_type_remote;
                remote_is_rfc2833[stVoipMgrSession.ch_id] = stVoipMgrSession.rfc2833_payload_type_remote;;
#endif
		talk_flag[stVoipMgrSession.ch_id] = talk_flag[stVoipMgrSession.ch_id] + 1;
		g_SIP_Info_play[s_id] = 1; /* Init for SIP Info play tone */

		restore_flags(flags);
#if 0
                PRINT_MSG("chid = %d, talk_flag = %d \n", stVoipMgrSession.ch_id, talk_flag[stVoipMgrSession.ch_id]);
#endif
		break;

	case VOIP_MGR_UNSET_SESSION:
                PRINT_MSG("UNSET RTP SESSION\n");
      		copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
		s_id = API_GetSid(stVoipCfg.ch_id, stVoipCfg.m_id);
                PRINT_MSG("s_id = %d\n", s_id);

		save_flags(flags); cli();
#ifdef SUPPORT_IP_ADDR_QOS
#if 1 // not necessary?!
		disable_eth8186_rx();
		rtl8306_disable_all_ports();
		rtl8306_setAsicQosIPAddress(
			s_id%2,
			0,
			0, 
			0);
		rtl8306_asicSoftReset();
		rtl8306_enable_all_ports();
		enable_eth8186_rx();		
#endif		
#endif                
		
#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_RX)
		rtp_pair.chid = stVoipCfg.ch_id;
		rtp_pair.mid = s_id;
		result = voip_register_RTPport( /*chid*/ rtp_pair.chid, /*mid*/ rtp_pair.mid, NULL );
		if( result < 0 )
		{
                        PRINT_MSG("Unregister 865x RTP port  failed\n");
		}
                else
                        RtpOpen[s_id] = 0;
#else
		rtk_trap_unregister(s_id);
#ifdef 	CONFIG_RTK_VOIP_RX_INPUT_QUEUE
		unset_input_rtp_port(s_id);
#endif
                RtpOpen[s_id] = 0;
#endif
		if(stVoipCfg.ch_id<4)
		{
			if(talk_flag[stVoipCfg.ch_id]>0)
			{
				talk_flag[stVoipCfg.ch_id] = talk_flag[stVoipCfg.ch_id] - 1;
			}else{
				talk_flag[stVoipCfg.ch_id] = 0;
			}
		}
		restore_flags(flags);
#if 0
                PRINT_MSG("chid = %d, talk_flag = %d \n", stVoipCfg.ch_id, talk_flag[stVoipCfg.ch_id]);
#endif
		break;

        case VOIP_MGR_SET_RTCP_SESSION:
#ifdef SUPPORT_RTCP
                PRINT_MSG("SET RTCP SESSION\n");
      		copy_from_user(&stVoipRtcpSession, (TstVoipRtcpSession *)user, sizeof(TstVoipRtcpSession));
      		s_id = API_GetSid(stVoipRtcpSession.ch_id, stVoipRtcpSession.m_id);
#if 0
                PRINT_MSG("ch_id = %d\n", stVoipRtcpSession.ch_id);
                PRINT_MSG("m_id = %d\n", stVoipRtcpSession.m_id);
                PRINT_MSG("Rtcp s_id = %d\n", s_id+ RTCP_SID_OFFSET);
                PRINT_MSG("Rtcp ip_src_addr = %d.%d.%d.%d\n", ((unsigned char *)(&stVoipRtcpSession.ip_src_addr))[0], ((unsigned char *)(&stVoipRtcpSession.ip_src_addr))[1], ((unsigned char *)(&stVoipRtcpSession.ip_src_addr))[2], ((unsigned char *)(&stVoipRtcpSession.ip_src_addr))[3]);
                PRINT_MSG("Rtcp ip_dst_addr = %d.%d.%d.%d\n", ((unsigned char *)(&stVoipRtcpSession.ip_dst_addr))[0], ((unsigned char *)(&stVoipRtcpSession.ip_dst_addr))[1], ((unsigned char *)(&stVoipRtcpSession.ip_dst_addr))[2], ((unsigned char *)(&stVoipRtcpSession.ip_dst_addr))[3]);
                PRINT_MSG("Rtcp udp_src_port = %d\n", ntohs(stVoipRtcpSession.rtcp_src_port));
                PRINT_MSG("Rtcp udp_dst_port = %d\n", ntohs(stVoipRtcpSession.rtcp_dst_port));
                PRINT_MSG("protocol = 0x%x\n", stVoipRtcpSession.protocol);
#endif
      		save_flags(flags); cli();
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
      		if(filter[s_id+ RTCP_SID_OFFSET]!=0) {
                  PRINT_MSG("rtcp: s_id %d in used, please unregister first\n", s_id+ RTCP_SID_OFFSET);
		  break;
        	}
#endif
		RtpSession_renew(s_id+ RTCP_SID_OFFSET);

#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_RX)
		//thlin: need test for 8651
		rtp_pair.isTcp = FALSE;
		rtp_pair.remIp = stVoipRtcpSession.ip_src_addr;
		rtp_pair.remPort = stVoipRtcpSession.rtcp_src_port;
		rtp_pair.extIp = stVoipRtcpSession.ip_dst_addr;
		rtp_pair.extPort = stVoipRtcpSession.rtcp_dst_port;
		rtp_pair.chid = stVoipRtcpSession.ch_id;
		rtp_pair.mid = s_id+ RTCP_SID_OFFSET;
		result = voip_register_RTPport(rtp_pair.chid, rtp_pair.mid, &rtp_pair );
		if(result < 0 )
		{
                        PRINT_MSG("865x register RTCP failed\n");
                        RtcpOpen[s_id] = 0;
		}
		else
		{
                        PRINT_MSG("865x register RTCP success\n");
                        RtcpOpen[s_id] = 1;
		}
#else
      		if (!rtk_trap_register(&stVoipRtcpSession, stVoipRtcpSession.ch_id, stVoipRtcpSession.m_id, s_id+ RTCP_SID_OFFSET,  DSP_pktRx))//register ok, return 0.
      			RtcpOpen[s_id] = 1;
#endif
      		restore_flags(flags);
#endif
		break;
	case VOIP_MGR_UNSET_RTCP_SESSION:
#ifdef SUPPORT_RTCP
                PRINT_MSG("UNSET RTCP SESSION\n");
      		copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
		s_id = API_GetSid(stVoipCfg.ch_id, stVoipCfg.m_id);
		extern unsigned short RtcpMeanTxInterval;
		if (RtcpMeanTxInterval != 0)
			RtcpTx_transmitRTCPBYE(s_id);
                PRINT_MSG("s_id = %d\n", s_id+ RTCP_SID_OFFSET);
		save_flags(flags); cli();
#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_RX)
		//thlin: need test for 8651
		rtp_pair.chid = stVoipCfg.ch_id;
		rtp_pair.mid = s_id+ RTCP_SID_OFFSET;
		result = voip_register_RTPport( /*chid*/ rtp_pair.chid, /*mid*/ rtp_pair.mid, NULL );
		if( result < 0 )
		{
                        PRINT_MSG("Unregister 865x RTCP port  failed\n");
		}
                else
                        RtpOpen[s_id] = 0;
#else
		rtk_trap_unregister(s_id+ RTCP_SID_OFFSET);
		RtcpOpen[s_id] = 0;
#endif
		restore_flags(flags);
#endif
		break;

	case VOIP_MGR_SET_RTCP_TX_INTERVAL:
		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
		save_flags(flags); cli();
		extern unsigned short RtcpMeanTxInterval;
		/* If RtcpMeanTxInterval is equal to 0, then RTCP Tx is disable.*/
		RtcpMeanTxInterval = stVoipValue.value5;
		restore_flags(flags);
		break;
		
	case VOIP_MGR_INIT_GNET:
                PRINT_MSG("INIT GNET\n");
		save_flags(flags); cli();
		//DSP_init();
                PRINT_MSG("useless now\n");
		restore_flags(flags);
                PRINT_MSG("INIT GNET FINISH\n");
		break;
	case VOIP_MGR_LOOPBACK:
                PRINT_MSG("SET LOOPBACK MODE\n");
      		copy_from_user(&stVoipTest, (TstVoipTest *)user, sizeof(TstVoipTest));
		//DSP_internal_loopback(stVoipTest.ch_id, stVoipTest.enable);
                PRINT_MSG("useless now\n");
                PRINT_MSG("chid = %d, enable = %d \n", stVoipTest.ch_id, stVoipTest.enable );
                PRINT_MSG("SET LOOPBACK MODE FINISH\n");
		break;
	case VOIP_MGR_GNET:
                //PRINT_MSG("GNET IOCTL ENTER\n");
      		copy_from_user(data, (unsigned int *)user, 16);
		/*
		for(i=0;i<4;i++)
                PRINT_MSG("kernel space:data[%d]=%d\n", i , data[i]);
		data[3] = 101;
		copy_to_user(user, data, 16);
		*/
		//DSP_ioctl(data);
                PRINT_MSG("GNET IOCTL CLOSE\n");
                //PRINT_MSG("GNET IOCTL EXIT\n");
		break;

	case VOIP_MGR_SETRTPPAYLOADTYPE:
                PRINT_MSG("VOIP_MGR_SETRTPPAYLOADTYPE\n");
		copy_from_user(&stVoipPayLoadTypeConfig, (TstVoipPayLoadTypeConfig *)user, sizeof(TstVoipPayLoadTypeConfig));
		ch_id = stVoipPayLoadTypeConfig.ch_id;
		m_id = stVoipPayLoadTypeConfig.m_id;
		s_id = API_GetSid(ch_id, m_id);
#if 0
                PRINT_MSG("ch_id = %d\n",ch_id);
                PRINT_MSG("m_id = %d\n",m_id);
                PRINT_MSG("s_id = %d\n",s_id);
                PRINT_MSG("uPktFormat = %d\n",stVoipPayLoadTypeConfig.uPktFormat);
                PRINT_MSG("nFramePerPacke t= %d\n",stVoipPayLoadTypeConfig.nFramePerPacket);
                PRINT_MSG("nG723Type = %d\n",stVoipPayLoadTypeConfig.nG723Type);
                PRINT_MSG("bVAD = %d\n",stVoipPayLoadTypeConfig.bVAD);
#endif

        	astVoipPayLoadTypeConfig[s_id].ch_id = ch_id;
        	astVoipPayLoadTypeConfig[s_id].m_id = m_id;
        	astVoipPayLoadTypeConfig[s_id].uPktFormat = stVoipPayLoadTypeConfig.uPktFormat;
        	astVoipPayLoadTypeConfig[s_id].nFramePerPacket = stVoipPayLoadTypeConfig.nFramePerPacket;
        	astVoipPayLoadTypeConfig[s_id].nG723Type = stVoipPayLoadTypeConfig.nG723Type;
        	astVoipPayLoadTypeConfig[s_id].bVAD = stVoipPayLoadTypeConfig.bVAD;
#if 1  //chiminer modify
		astTwoChannelCfg[ch_id].ch_id = ch_id;
		astTwoChannelCfg[ch_id].s_id = s_id;
		astTwoChannelCfg[ch_id].uPktFormat = stVoipPayLoadTypeConfig.uPktFormat;
		astTwoChannelCfg[ch_id].channel_enable = 1;
#endif

#if 1
		/* Now, it is merely an experimental parameter. */
		stVoipPayLoadTypeConfig.nJitterFactor = 7;
#endif

		save_flags(flags); cli();
        	DSP_CodecRestart(ch_id, s_id,
        					 stVoipPayLoadTypeConfig.uPktFormat,
        					 stVoipPayLoadTypeConfig.nFramePerPacket,
        					 stVoipPayLoadTypeConfig.nG723Type,
        					 stVoipPayLoadTypeConfig.bVAD,
        					 stVoipPayLoadTypeConfig.nJitterDelay,
        					 stVoipPayLoadTypeConfig.nMaxDelay,
        					 stVoipPayLoadTypeConfig.nJitterFactor);
		restore_flags(flags);
#ifdef T38_STAND_ALONE_HANDLER
		if( stVoipPayLoadTypeConfig.uPktFormat == rtpPayloadT38_Virtual ) {
			T38_API_Initialize( ch_id );
			PRINT_MSG("MGR: Initialize T38\n");
			t38RunningState[ ch_id ] = T38_START;
		} else
			t38RunningState[ ch_id ] = T38_STOP;		
#endif
		break;

	case VOIP_MGR_SETRTPSESSIONSTATE:
      		copy_from_user(&stVoipRtpSessionState, (TstVoipRtpSessionState *)user, sizeof(TstVoipRtpSessionState));
		ch_id = stVoipRtpSessionState.ch_id;
		m_id = stVoipRtpSessionState.m_id;
		
		s_id = API_GetSid(ch_id, m_id);
             PRINT_MSG("VOIP_MGR_SETRTPSESSIONSTATE:ch_id=%d, m_id=%d, s_id=%d, state=%d\n", stVoipRtpSessionState.ch_id, stVoipRtpSessionState.m_id, s_id, stVoipRtpSessionState.state);

		switch(stVoipRtpSessionState.state){
			case rtp_session_sendonly:	//pass through
			case rtp_session_recvonly: //pass through
			case rtp_session_sendrecv:
			{
				extern void DspcodecWriteSnyc( uint32 sid );
				DspcodecWriteSnyc( s_id );
		  		rtpConfigOK[s_id]=1;
		 		DSP_init_done = 1;
			}
			break;
			case rtp_session_inactive:
				rtpConfigOK[s_id]=0;
				astTwoChannelCfg[stVoipRtpSessionState.ch_id].channel_enable = 0;
			break;
		}
		save_flags(flags); cli();
		RtpTerminal_SetSessionState(s_id, stVoipRtpSessionState.state);
		restore_flags(flags);
		break;

	case VOIP_MGR_SETPLAYTONE:
      		copy_from_user(&stVoipPlayToneConfig, (TstVoipPlayToneConfig*)user, sizeof(TstVoipPlayToneConfig));

		ch_id = stVoipPlayToneConfig.ch_id;
		m_id = stVoipPlayToneConfig.m_id;

		#ifdef _chiminer_slic2_ch1
		    ch_id=1;
                    PRINT_MSG("ch_id=%d\n",ch_id);
		#endif

		s_id = API_GetSid(ch_id, m_id);
                PRINT_MSG("VOIP_MGR_SETPLAYTONE:ch_id=%d, m_id=%d, s_id=%d, nTone=%d, bFlag=%d, path=%d\n", ch_id, m_id, s_id, stVoipPlayToneConfig.nTone, stVoipPlayToneConfig.bFlag, stVoipPlayToneConfig.path);

		save_flags(flags); cli();
		hc_SetPlayTone(ch_id, s_id, stVoipPlayToneConfig.nTone, stVoipPlayToneConfig.bFlag, stVoipPlayToneConfig.path);
		restore_flags(flags);
/*
		save_flags(flags); cli();

        	if(stVoipPlayToneConfig.bFlag)
        	{
                  	if(s_nCurPlayTone[ch_id] != -1)
                  	{
                    		//if(pcfg->nTone != nCurPlayTone)
                    		DspcodecPlayTone(ch_id, s_nCurPlayTone[ch_id], 0, stVoipPlayToneConfig.path);
            	    		// before open a new tone, must close old first
                  	}
                  	s_nCurPlayTone[ch_id] = stVoipPlayToneConfig.nTone;
                  	DspcodecPlayTone(ch_id, s_nCurPlayTone[ch_id], stVoipPlayToneConfig.bFlag, stVoipPlayToneConfig.path);
         	}
        	else
        	{
                  	if(s_nCurPlayTone[ch_id] != -1)
                  	{
                   		DspcodecPlayTone(ch_id, s_nCurPlayTone[ch_id], stVoipPlayToneConfig.bFlag, stVoipPlayToneConfig.path);
                    		s_nCurPlayTone[ch_id] = -1;
			    	//if(pcfg->nTone != nCurPlayTone)
			    	//DspcodecPlayTone(chid, nCurPlayTone, 0, pcfg->path);
			    	//before open a new tone, must close old first
                  	}
                }
        	s_tonepath[ch_id] = stVoipPlayToneConfig.path;

		restore_flags(flags);
*/
                //PRINT_MSG("%s-%d\n", __FUNCTION__, __LINE__);
		break;

	case VOIP_MGR_DSPSETCONFIG: //test only
      		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
                PRINT_MSG("VOIP_MGR_DSPSETCONFIG:value=%d\n", stVoipValue.value);
		//DSP_SetConfig(0, false, rtpPayloadPCMU, 0);
		save_flags(flags); cli();
		DSP_SetConfig(0, false, stVoipValue.value, 0, 5, 8, 7);
		restore_flags(flags);
		break;
		
	case VOIP_MGR_DSPCODECSTART: //test only
      		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
                PRINT_MSG("VOIP_MGR_DSPCODECSTART:value=%d\n", stVoipValue.value);
		save_flags(flags); cli();
		DspcodecStart(0, stVoipValue.value);
		restore_flags(flags);
		break;

	case VOIP_MGR_RTP_CFG:
      		copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
		s_id = API_GetSid(stVoipCfg.ch_id, stVoipCfg.m_id);
                PRINT_MSG("VOIP_MGR_RTP_CFG:ch_id = %d, m_id = %d, s_id=%d, enable = %d\n", stVoipCfg.ch_id,  stVoipCfg.m_id, s_id, stVoipCfg.enable);
		if(stVoipCfg.enable ==1){
		  extern void DspcodecWriteSnyc( uint32 sid );
		  DspcodecWriteSnyc( s_id );

		  rtpConfigOK[s_id]=1;
		  DSP_init_done = 1;
		} else if(stVoipCfg.enable ==0) {
		  rtpConfigOK[s_id]=0;
		  //DSP_init_done = 0;
#if 1 //chiminer modify
		astTwoChannelCfg[stVoipCfg.ch_id].channel_enable = 0;
#endif
		}
		break;

	case VOIP_MGR_INIT_GNET2:
                PRINT_MSG("INIT GNET2\n");
		if(dsp_init_first == NULL ) {
		  rtk_voip_dsp_init();
		  dsp_init_first = 1;
		}
                PRINT_MSG("INIT GNET2 FINISH\n");
		break;

	case VOIP_MGR_DTMF_CFG:
      		copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
                PRINT_MSG("VOIP_MGR_DTMF_CFG:ch_id = %d, enable = %d\n", stVoipCfg.ch_id, stVoipCfg.enable);
		#ifdef _chiminer_slic2_ch1
		    stVoipCfg.ch_id=1;
                    PRINT_MSG("stVoipCfg.ch_id=%d\n",stVoipCfg.ch_id);
		#endif
		if(stVoipCfg.enable == 1)
			dtmf_start((char)stVoipCfg.ch_id);
		else if ( stVoipCfg.enable == 0)
			dtmf_stop((char) stVoipCfg.ch_id);
		break;

	case VOIP_MGR_SLIC_HOOK:
      		copy_from_user(&stVoipSlicHook, (TstVoipSlicHook *)user, sizeof(TstVoipSlicHook));

#ifdef EVENT_POLLING_TIMER
		hook_out(stVoipSlicHook.ch_id, &stVoipSlicHook.hook_status);
#endif
#ifdef CONFIG_RTK_VOIP_LED
		//CH_ID = stVoipSlicHook.ch_id;
#endif
		switch (stVoipSlicHook.hook_status)
		{
			case PHONE_OFF_HOOK:
                                PRINT_MSG("VOIP_MGR_SLIC_HOOK: PHONE_OFF_HOOK\n");
#ifdef CONFIG_RTK_VOIP_LED
				fxs_ringing[stVoipSlicHook.ch_id] = 0;
#endif
				break;
			case PHONE_ON_HOOK:
                                PRINT_MSG("VOIP_MGR_SLIC_HOOK: PHONE_ON_HOOK\n");
#ifdef CONFIG_RTK_VOIP_LED
				fxs_ringing[stVoipSlicHook.ch_id] = 0;
#endif
				break;
			case PHONE_FLASH_HOOK:
                                PRINT_MSG("VOIP_MGR_SLIC_HOOK: PHONE_FLASH_HOOK\n");
				break;
		}
#ifdef CONFIG_RTK_VOIP_LED
		if ( stVoipSlicHook.hook_status <= PHONE_FLASH_HOOK )
		{
			slic_hook_status[stVoipSlicHook.ch_id] = stVoipSlicHook.hook_status;
			led_state_watcher(stVoipSlicHook.ch_id);
                }
#endif
      		copy_to_user(user, &stVoipSlicHook, sizeof(TstVoipSlicHook));
		break;

	case VOIP_MGR_SLIC_TONE:
                PRINT_MSG("VOIP_MGR_SLIC_TONE\n");
      		copy_from_user(&stVoipSlicTone, (TstVoipSlicTone *)user, sizeof(TstVoipSlicTone));
		save_flags(flags); cli();
		SLIC_GenProcessTone(&stVoipSlicTone);
		restore_flags(flags);
		break;

	case VOIP_MGR_DSPCODECSTOP:
                PRINT_MSG("VOIP_MGR_DSPCODECSTOP\n");
      		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
		s_id = API_GetSid(stVoipValue.ch_id, stVoipValue.m_id);
#if 1
                PRINT_MSG("ch_id =%d, m_id=%d, s_id=%d\n", stVoipValue.ch_id, stVoipValue.m_id, s_id);
#endif
		save_flags(flags); cli();
		DspcodecStop(s_id);
#ifdef T38_STAND_ALONE_HANDLER
		t38RunningState[ stVoipValue.ch_id ] = T38_STOP;
#endif
		restore_flags(flags);
		break;

	case VOIP_MGR_HOLD:
		copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
                s_id = API_GetSid(stVoipCfg.ch_id, stVoipCfg.m_id);
                PRINT_MSG("VOIP_MGR_HOLD:ch_id = %d, m_id = %d, s_id=%d, enable = %d\n", stVoipCfg.ch_id,  stVoipCfg.m_id, s_id, stVoipCfg.enable);
		save_flags(flags); cli();
                if(stVoipCfg.enable ==1){
                  rtpConfigOK[s_id]=0;
		  rtpHold[s_id]=1;
                } else if(stVoipCfg.enable ==0) {
                  rtpConfigOK[s_id]=1;
		  rtpHold[s_id]=0;
                }
		restore_flags(flags);
                break;

	case VOIP_MGR_CTRL_RTPSESSION:
		{
      		copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
                PRINT_MSG("VOIP_MGR_CTRL_RTPSESSION:ch_id = %d, m_id = %d, enable = %d\n", stVoipCfg.ch_id, stVoipCfg.m_id, stVoipCfg.enable);
		save_flags(flags); cli();
                if(stVoipCfg.enable ==1){
		  API_OpenSid(stVoipCfg.ch_id, stVoipCfg.m_id);
                } else if(stVoipCfg.enable ==0) {
		  API_CloseSid(stVoipCfg.ch_id, stVoipCfg.m_id);
		}
		restore_flags(flags);
		}
		break;

	case VOIP_MGR_CTRL_TRANSESSION_ID:
		{
      		copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
			PRINT_MSG("VOIP_MGR_CTRL_TRANSESSION_ID:ch_id = %d, m_id = %d, enable = %d\n", stVoipCfg.ch_id, stVoipCfg.m_id, stVoipCfg.enable);
		#ifdef _chiminer_slic2_ch1
		    stVoipCfg.ch_id=1;
            PRINT_MSG("stVoipCfg.ch_id=%d\n",stVoipCfg.ch_id);
		#endif
			extern channel_config_t chanInfo[];
			save_flags(flags); cli();
                  
            if(stVoipCfg.enable ==1)
            {
            	if (stVoipCfg.m_id == 255)
               	{
					chanInfo[stVoipCfg.ch_id].resource = 0;
					PRINT_MSG("resource[%d]=%d\n", stVoipCfg.ch_id, chanInfo[stVoipCfg.ch_id].resource);
                }
                else
                {
                	chanInfo[stVoipCfg.ch_id].resource = 1;
                	PRINT_MSG("resource[%d]=%d\n", stVoipCfg.ch_id, chanInfo[stVoipCfg.ch_id].resource);
                }
                s_id = API_GetSid(stVoipCfg.ch_id, stVoipCfg.m_id);
	 	  		chanInfo_SetTranSessionID(stVoipCfg.ch_id, s_id);
			} 
			else if(stVoipCfg.enable ==0) 
			{
		  		temp = chanInfo_GetTranSessionID(stVoipCfg.ch_id);
		  		stVoipCfg.t_id = temp;
			}
			restore_flags(flags);
			copy_to_user(user, &stVoipCfg, sizeof(TstVoipCfg));
		}
		break;

	case VOIP_MGR_SETCONFERENCE:
                copy_from_user(&stVoipMgr3WayCfg, (TstVoipMgr3WayCfg *)user, sizeof(TstVoipMgr3WayCfg));
		PRINT_MSG("VOIP_MGR_SETCONFERENCE:ch_id = %d, enable = %d\n", stVoipMgr3WayCfg.ch_id, stVoipMgr3WayCfg.enable);
		save_flags(flags); cli();
		extern channel_config_t chanInfo[];
		if (stVoipMgr3WayCfg.enable == 1)
		{
			chanInfo[stVoipMgr3WayCfg.ch_id].resource = 2;
		}
		chanInfo_SetConference(stVoipMgr3WayCfg.ch_id, stVoipMgr3WayCfg.enable);
		restore_flags(flags);
		break;

	case VOIP_MGR_ON_HOOK_RE_INIT:

    		copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
                PRINT_MSG("VOIP_MGR_ON_HOOK_RE_INIT:ch_id = %d, enable = %d\n", stVoipCfg.ch_id, stVoipCfg.enable);
		#ifdef _chiminer_slic2_ch1
		    stVoipCfg.ch_id=1;
                    PRINT_MSG("stVoipCfg.ch_id=%d\n",stVoipCfg.ch_id);
		#endif
			Init_CED_Det(stVoipCfg.ch_id);

		#ifdef SUPPORT_LEC_G168_ISR
			LEC_re_init(stVoipCfg.ch_id);
		#endif

		#ifdef EXPER_AEC
			AEC_re_init(stVoipCfg.ch_id);
		#endif

		#ifdef EXPER_NR
			NR_re_init(stVoipCfg.ch_id);
		#endif

#ifdef FXO_CALLER_ID_DET
		dmtf_cid_det_init();
		init_cid_det_si3500(2);
		for (i=0; i < SLIC_CH_NUM; i++)
		{
			fsk_cid_valid[i] = 0;
			dtmf_cid_valid[i] = 0;
		}
#endif
		break;
		
	case VOIP_MGR_DTMF_CID_GEN_CFG:
#ifdef SW_DTMF_CID
                //PRINT_MSG("VOIP_MGR_DTMF_CID_GEN_CFG\n");
		copy_from_user(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));
#if 1
		if(!(fsk_spec_areas[stCIDstr.ch_id]&0x08))// send DTMF callerid after ring.
		{
			ring.CH = stCIDstr.ch_id;
			ring.ring_set = 1;
			FXS_Ring(&ring);
			mdelay(1000);
			ring.ring_set = 0;
			FXS_Ring(&ring);
		}
#endif // end if 0
		dtmf_cid_state[stCIDstr.ch_id]=1;
		strcpy(cid_str, stCIDstr.string);
#endif
		break;

	case VOIP_MGR_GET_CID_STATE_CFG:
#ifdef SW_DTMF_CID
		copy_from_user(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));
		stCIDstr.cid_state = dtmf_cid_state[stCIDstr.ch_id];
		copy_to_user(user, &stCIDstr, sizeof(TstVoipCID));
                //PRINT_MSG("VOIP_MGR_GET_CID_STATE_CFG = %d\n", stCIDstr.cid_state);
#endif
		break;

	case VOIP_MGR_SET_DTMF_MODE:// 0:rfc2833  1: sip info  2: inband
		copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

		if (stVoipCfg.enable == 2)
		{
			dtmf_removal_flag[stVoipCfg.ch_id] = 0; /* No DTMF removal */
			dtmf_mode[stVoipCfg.ch_id] = 2;
		}
		else if (stVoipCfg.enable == 1)
		{
			dtmf_removal_flag[stVoipCfg.ch_id] = 1; /* Do DTMF removal */
			dtmf_mode[stVoipCfg.ch_id] = 1;
		}
		else if (stVoipCfg.enable == 0)
		{
			dtmf_removal_flag[stVoipCfg.ch_id] = 1; /* Do DTMF removal */
			dtmf_mode[stVoipCfg.ch_id] = 0;
		}
		break;

	case VOIP_MGR_FAX_OFFHOOK://when phone offhook, set fax_offhook[]=1;
		copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
		fax_offhook[stVoipCfg.ch_id] = stVoipCfg.enable;
		break;

	case VOIP_MGR_FSK_CID_GEN_CFG:
		copy_from_user(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));
		fsk_cid_state[stCIDstr.ch_id]=1;
#if defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210 ) || defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215 )
		if (fsk_gen_mode[stCIDstr.ch_id] == 0)// HW
			CID_for_FSK_HW(stCIDstr.ch_id, stCIDstr.cid_mode, FSK_MSG_CALLSETUP , stCIDstr.string, stCIDstr.string2, stCIDstr.cid_name);
		else if (fsk_gen_mode[stCIDstr.ch_id] == 1) // SW
			CID_for_FSK_SW(stCIDstr.ch_id, stCIDstr.cid_mode, FSK_MSG_CALLSETUP , stCIDstr.string, stCIDstr.string2, stCIDstr.cid_name);
#elif defined CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221
		/* Legrity 88221 HW FSK CID gen is not supported, so it always use SW FSK CID gen. 2007-04-13*/
		CID_for_FSK_SW(stCIDstr.ch_id, stCIDstr.cid_mode, FSK_MSG_CALLSETUP , stCIDstr.string, stCIDstr.string2, stCIDstr.cid_name);
#endif
                break;

	case VOIP_MGR_SEND_RFC2833_PKT_CFG:
		copy_from_user(&stRFC2833, (TstVoip2833 *)user, sizeof(TstVoip2833));
#ifdef SUPPORT_RFC_2833
	#ifndef SEND_RFC2833_ISR
		RtpTx_transmitEvent(stRFC2833.ch_id, stRFC2833.sid, stRFC2833.digit, stRFC2833.duration);
	#elif defined( CONFIG_RTK_VOIP_IP_PHONE ) && defined( SEND_RFC2833_ISR )
		{
                        extern unsigned char send_2833_count_down[MAX_SLIC_CH_NUM];
                        extern int sned_flag[MAX_SLIC_CH_NUM];
                        extern int g_digit[MAX_SLIC_CH_NUM];
		
			sned_flag[ stRFC2833.ch_id ] = 1;
			g_digit[ stRFC2833.ch_id ] = stRFC2833.digit;
			send_2833_count_down[ stRFC2833.ch_id ] = stRFC2833.duration / ( PCM_PERIOD * 10 );
													  // = 100 / ( PCM_PERIOD * 10 );	/* 100ms */
		}
	#endif
		/* Thlin: Send RFC2833 packets has been move to PCM_RX() */
#endif
		break;

	case 	VOIP_MGR_SET_ECHO_TAIL_LENGTH:
		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
#ifdef SUPPORT_LEC_G168_ISR
		Tap_len = 8*stVoipValue.value; //stVoipValue.value : unit is ms
		LEC_g168_init(stVoipValue.ch_id, LEC|LEC_NLP);
#endif
		break;
		
	case 	VOIP_MGR_SET_G168_LEC_CFG:
		copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
		support_lec_g168[stVoipCfg.ch_id] = stVoipCfg.enable; /* 0: LEC disable  1: LEC enable */
		break;

	case VOIP_MGR_SET_FSK_VMWI_STATE:// thlin +
		copy_from_user(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));
		/* If CID mode is DTMF mode, don't gen VMWI message.*/
		if ((fsk_spec_areas[stCIDstr.ch_id]&7) == 4)//DTMF mode
		{
			PRINT_MSG("\x1B[31mCID mode isn't FSK type, don't support VMWI.\x1B[0m\n");
			break;
		}
		fsk_cid_state[stCIDstr.ch_id]=1;
		if (fsk_gen_mode[stCIDstr.ch_id] == 0)// HW
		{
			CID_for_FSK_HW(stCIDstr.ch_id, stCIDstr.cid_mode, FSK_MSG_MWSETUP, stCIDstr.string, stCIDstr.string2, stCIDstr.cid_name); // fsk date & time sync
		}
		else if (fsk_gen_mode[stCIDstr.ch_id] == 1)// SW
		{
			init_softfskcidGen(stCIDstr.ch_id);
			CID_for_FSK_SW(stCIDstr.ch_id, stCIDstr.cid_mode, FSK_MSG_MWSETUP, stCIDstr.string, stCIDstr.string2, stCIDstr.cid_name); // fsk date & time sync
		}
		break;
		
	case VOIP_MGR_SET_FSK_AREA:
		copy_from_user(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));
		fsk_spec_areas[stCIDstr.ch_id] = stCIDstr.cid_mode; /* 0:Bellcore 1:ETSI 2:BT 3:NTT */
		// fsk_spec_areas[]:
		// bit0-2: FSK Type
		// bit 3: Caller ID Prior First Ring
		// bit 4: Dual Tone before Caller ID (Fsk Alert Tone)
		// bit 5: Short Ring before Caller ID
		// bit 6: Reverse Polarity before Caller ID (Line Reverse)
		// bit 7: FSK Date & Time Sync and Display Name
		extern int fsk_cid_gain[];
		fsk_cid_gain[stCIDstr.ch_id] = stCIDstr.cid_gain; /* Only support multiple 1~5 */
                PRINT_MSG("\nReset the fsk setting %d\n",fsk_spec_areas[stCIDstr.ch_id]);
		break;

	case VOIP_MGR_FSK_ALERT_GEN_CFG:
		copy_from_user(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));
		fsk_alert_state[stCIDstr.ch_id] = 1;
		strcpy(cid_str, stCIDstr.string);
		break;


	case VOIP_MGR_SET_TONE_OF_COUNTRY:
		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
		uint32 sid, chid;
		for(sid=0; sid<SESS_NUM; sid++)	//Set the same country for each session.
			DspcodecSetCountry( sid, /*country*/stVoipValue.value);
		for (chid=0; chid<SLIC_CH_NUM; chid++)
			SLIC_Set_Impendance(chid, stVoipValue.value /*country*/, 0 /* impedance value: reserved */);
		
#ifdef FXO_BUSY_TONE_DET
		switch (stVoipValue.value)
		{
			case 0://USA
			case 3://HK
				det_freq[0] = FREQ_480HZ;
				det_freq[1] = FREQ_620HZ;
				break;
			case 7://France
			case 9://Belgium
				det_freq[0] = FREQ_440HZ;
				det_freq[1] = FREQ_NA;
				break;
			case 1://UK
			case 2://Australia
			case 4://Japan
				det_freq[0] = FREQ_400HZ;
				det_freq[1] = FREQ_NA;
				break;
			case 5://Sweden
			case 6://Germany
			case 8://TR57
			case 10://Finland
			case 11://Italy
				det_freq[0] = FREQ_425HZ;
				det_freq[1] = FREQ_NA;
				break;
			case 12://China
				det_freq[0] = FREQ_450HZ;
				det_freq[1] = FREQ_NA;
				break;

			default://USA
				det_freq[0] = FREQ_480HZ;
				det_freq[1] = FREQ_620HZ;
				break;
		}
#endif
		break;
	case VOIP_MGR_SET_TONE_OF_CUSTOMIZE:
		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
		cust = stVoipValue.value;
		break;
	case VOIP_MGR_SET_CUST_TONE_PARAM:
		copy_from_user(&stVoipToneCfg, (TstVoipToneCfg *)user, sizeof(TstVoipToneCfg));
#if 1
		/*
		Because AudioCodes doesn't support play tone with fixd cycle, RTK could. 
		To be identical, when RTK get cycle=2, 
		change it to 0(continuous play tone with cadence)
		*/
		if (stVoipToneCfg.cycle == 0)	//Continuous
		{
			//set CadOn0 to non-zero value to ensure play continuous tone
			stVoipToneCfg.CadOn0 = 100;
			// set other CadOn/Off to zero to ensure not enter cadence tone
			stVoipToneCfg.CadOff0 = 0;
			stVoipToneCfg.CadOn1 = 0;
			stVoipToneCfg.CadOff1 = 0;
			stVoipToneCfg.CadOn2 = 0;
			stVoipToneCfg.CadOff2 = 0;
			stVoipToneCfg.CadOn3 = 0;
			stVoipToneCfg.CadOff3 = 0;
		}
		else if (stVoipToneCfg.cycle == 2)	//Cadence
			stVoipToneCfg.cycle = 0;	//Continuous
		
		//web setting unit is (-dBm)
		stVoipToneCfg.Gain1 = (-1)*stVoipToneCfg.Gain1;
		
		// if tone type is succeed, use the Gain1 value as the gain of other frequency.
		if (stVoipToneCfg.toneType == 2)//SUCC
		{
			stVoipToneCfg.Gain2 = stVoipToneCfg.Gain1;
			stVoipToneCfg.Gain3 = stVoipToneCfg.Gain1;
			stVoipToneCfg.Gain4 = stVoipToneCfg.Gain1;
		}
		else
		{
			stVoipToneCfg.Gain2 = (-1)*stVoipToneCfg.Gain2;
			stVoipToneCfg.Gain3 = (-1)*stVoipToneCfg.Gain3;
			stVoipToneCfg.Gain4 = (-1)*stVoipToneCfg.Gain4;
		}
#endif
		setTone(&stVoipToneCfg);
		break;
	case VOIP_MGR_USE_CUST_TONE:
		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

		pToneTable = (short*)&ToneTable[CUSTOM_TONE1+stVoipValue.value1];	//dial
		DspcodecSetCustomTone(DSPCODEC_TONE_DIAL, (ToneCfgParam_t *)pToneTable);

		pToneTable = (short*)&ToneTable[CUSTOM_TONE1+stVoipValue.value2];	//ring
		DspcodecSetCustomTone(DSPCODEC_TONE_RINGING, (ToneCfgParam_t *)pToneTable);

		pToneTable = (short*)&ToneTable[CUSTOM_TONE1+stVoipValue.value3];	//busy
		DspcodecSetCustomTone(DSPCODEC_TONE_BUSY, (ToneCfgParam_t *)pToneTable);

		pToneTable = (short*)&ToneTable[CUSTOM_TONE1+stVoipValue.value4];	//waiting
		DspcodecSetCustomTone(DSPCODEC_TONE_CALL_WAITING, (ToneCfgParam_t *)pToneTable);

		break;
		
	case VOIP_MGR_VOIP_RESOURCE_CHECK:
		copy_from_user(&stVoipResourceCheck, (TstVoipResourceCheck *)user, sizeof(TstVoipResourceCheck));
#if ! defined (VOIP_RESOURCE_CHECK)	
		stVoipResourceCheck.resource = VOIP_RESOURCE_AVAILABLE;
#else		
		extern channel_config_t chanInfo[];	
		stVoipResourceCheck.resource = VOIP_RESOURCE_UNAVAILABLE;
		int resource_cnt=0;
		for (chid=0; chid<SLIC_CH_NUM; chid++)	
		{
			resource_cnt += chanInfo[chid].resource;
		}
		if (resource_cnt < 2)
		{
			stVoipResourceCheck.resource = VOIP_RESOURCE_AVAILABLE;
		}
			
		if (stVoipResourceCheck._3way_session_info.enable != 255)
		{
			int mmid, isLoopBack[2] = {0};
			/* Check which mid session is LoopBack. Note: If src ip = dst ip, this session is LoopBack. */			
			for (mmid=0; mmid<=1; mmid++)
      			{
				if (stVoipResourceCheck._3way_session_info.rtp_cfg[mmid].ip_src_addr == stVoipResourceCheck._3way_session_info.rtp_cfg[mmid].ip_dst_addr)
				{
					isLoopBack[mmid] = 1;
					PRINT_MSG("isLoopBack[%d]=%d\n", mmid, isLoopBack[mmid]);
					stVoipResourceCheck.resource = VOIP_RESOURCE_AVAILABLE;
				}
			}
		}
#endif
		copy_to_user(user, &stVoipResourceCheck, sizeof(TstVoipResourceCheck));
		break;
		
	case VOIP_MGR_DEBUG:
		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
		PRINT_MSG("VOIP_MGR_DEBUG: dbg=%d\n", stVoipValue.value);
		save_flags(flags); cli();
		g_voip_debug = stVoipValue.value;
		restore_flags(flags);
#ifdef TEST_UNALIGN
		if (g_voip_debug == 1)
		{
			test_p = (int *)(test_data + 3);
			printk("data = %p, p = %p\n", test_data, test_p);
			printk("*p = %d\n", *test_p);
		}
		else if (g_voip_debug == 2)
		{
			save_flags(flags); cli();
			set_DMEM(&__sys_dmem_start);
			sys_dmem_sp = &(sys_dmem_stack[SYS_DMEM_SSIZE]);
			entry_dmem_stack(&sys_orig_sp, &sys_dmem_sp);
			printk("enter: sys_orig_sp = %x\n", sys_orig_sp);

			test_p = (int *)(test_data + 3);
			printk("data = %p, p = %p\n", test_data, test_p);
			printk("*p = %d\n", *test_p);

			leave_dmem_stack(&sys_orig_sp);
			printk("leave1: sys_orig_sp = %x\n", sys_orig_sp);
			restore_flags(flags);
		}
#endif
		break;

#ifdef CONFIG_RTK_VOIP_IVR
	case VOIP_MGR_PLAY_IVR:
		/* Do copy_from_user() in PlayIvrDispatcher() */
		PlayIvrDispatcher( &stVoipPlayIVR_Header, user );
		copy_to_user(user, &stVoipPlayIVR_Header, sizeof(TstVoipPlayIVR_Header));
		break;

	case VOIP_MGR_POLL_IVR:
		copy_from_user(&stVoipPollIVR, (TstVoipPollIVR *)user, sizeof(TstVoipPollIVR));
		stVoipPollIVR.bPlaying =
				PollIvrPlaying( stVoipPollIVR.ch_id );
		copy_to_user(user, &stVoipPollIVR, sizeof(TstVoipPollIVR));
		break;

	case VOIP_MGR_STOP_IVR:
		copy_from_user(&stVoipStopIVR, (TstVoipStopIVR *)user, sizeof(TstVoipStopIVR));
		StopIvrPlaying( stVoipStopIVR.ch_id );
		break;
#endif /* CONFIG_RTK_VOIP_IVR */

	case VOIP_MGR_SET_CID_DTMF_MODE:
		copy_from_user(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));
		fsk_spec_areas[stCIDstr.ch_id] = stCIDstr.cid_mode; 	/* when DTMF mode, set the before ring or after ring send cid */
		cid_dtmf_mode[stCIDstr.ch_id] = stCIDstr.cid_dtmf_mode;	/* when DTMF mode, set the caller id start/end digit */
		break;

	case VOIP_MGR_PLAY_SIP_INFO:
		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

		int index;

		s_id = API_GetSid(stVoipValue.ch_id, stVoipValue.m_id);
		index = g_SIP_Info_buf_w[s_id];
		g_SIP_Info_flag[s_id] = 1;
		g_SIP_Info_tone_buf[s_id][index] = stVoipValue.value;
		g_SIP_Info_time_buf[s_id][index] = stVoipValue.value5/10;
		g_SIP_Info_buf_w[s_id] = (g_SIP_Info_buf_w[s_id] + 1)%10;

		break;

	case VOIP_MGR_SET_DAA_ISR_FLOW:
		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
#ifdef CONFIG_RTK_SLIC_DAA_NEGOTIATION
		save_flags(flags); cli();
		DAA_ISR_FLOW[stVoipValue.ch_id] = stVoipValue.value;
		s_id = API_GetSid(stVoipValue.ch_id, stVoipValue.m_id);

		if (stVoipValue.value != DRIVER_FLOW_NORMAL)// not normal flow
				sid_owner[s_id] = 1; //DAA own
		else
				sid_owner[s_id] = 0; //SLIC own
		restore_flags(flags);
                PRINT_MSG("rtk_Set_DAA_ISR_FLOW: chid=%d, flow=%d, mid=%d, sid=%d\n", stVoipValue.ch_id, stVoipValue.value, stVoipValue.m_id, s_id);
#endif
		break;
		
	case VOIP_MGR_GET_DAA_ISR_FLOW:
		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
#ifdef CONFIG_RTK_SLIC_DAA_NEGOTIATION
		save_flags(flags); cli();
		stVoipValue.value = DAA_ISR_FLOW[stVoipValue.ch_id];
		restore_flags(flags);
		copy_to_user(user, &stVoipValue, sizeof(TstVoipValue));
                PRINT_MSG("rtk_Get_DAA_ISR_FLOW: chid=%d, flow=%d\n", stVoipValue.ch_id, stVoipValue.value);
#endif
		break;
		
	case VOIP_MGR_GET_DAA_USED_BY_WHICH_SLIC:
		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
		save_flags(flags); cli();
		stVoipValue.value = g_DAA_used[stVoipValue.ch_id];
		restore_flags(flags);
		copy_to_user(user, &stVoipValue, sizeof(TstVoipValue));
		break;
		
	case VOIP_MGR_SET_DAA_PCM_HOLD_CFG:
		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
#ifdef CONFIG_RTK_SLIC_DAA_NEGOTIATION
		save_flags(flags); cli();
		if (stVoipValue.value == 1)
		{
			pcm_Hold_DAA(stVoipValue.value2); /* hold daa used pcm channel */

			if ( g_DAA_used[stVoipValue.ch_id] == DAA_USE)
				g_DAA_used[stVoipValue.ch_id] = DAA_USE_HOLD; // hold
		}
		else
		{
			pcm_UnHold_DAA(stVoipValue.value2);

			if ( g_DAA_used[stVoipValue.ch_id] == DAA_USE_HOLD)
				g_DAA_used[stVoipValue.ch_id] = DAA_USE; // un-hold
		}
		restore_flags(flags);
                PRINT_MSG("VOIP_MGR_SET_DAA_PCM_HOLD_CFG: chid=%d, enable=%d\n", stVoipValue.ch_id, stVoipValue.value);
#endif
		break;
		
	case VOIP_MGR_GET_DAA_BUSY_TONE_STATUS:
		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
#ifdef FXO_BUSY_TONE_DET
		save_flags(flags); cli();
		stVoipValue.value = busy_tone_flag_get();/* Busy tone is  1: Detected, 0: NOT detected. */
		restore_flags(flags);
		copy_to_user(user, &stVoipValue, sizeof(TstVoipValue));
#endif
		break;
		
	case VOIP_MGR_GET_DAA_CALLER_ID:
#ifdef FXO_CALLER_ID_DET
		copy_from_user(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));
		save_flags(flags); cli();

		if (cid_res.cid_valid == 1)
		{
			for (i=0; i < SLIC_CH_NUM; i++)
				dtmf_cid_valid[i] = 1;
		}
		else if (stVoipciddet[stCIDstr.daa_id].cid_valid == 1)
		{
			for (i=0; i < SLIC_CH_NUM; i++)
				fsk_cid_valid[i] = 1;
		}

		if (dtmf_cid_valid[stCIDstr.ch_id] == 1) // DTMF
		{
			strcpy(stCIDstr.string, cid_res.number); // caller id
			stCIDstr.string2[0] = 0;
			dmtf_cid_det_init();
			dtmf_cid_valid[stCIDstr.ch_id] = 0;
                        PRINT_MSG("-->dtmf get cid (%d)\n", stCIDstr.ch_id);
		}
		else if (fsk_cid_valid[stCIDstr.ch_id] == 1) // FSK
		{
			strcpy(stCIDstr.string, stVoipciddet[stCIDstr.daa_id].number);	// caller id
			strcpy(stCIDstr.string2, stVoipciddet[stCIDstr.daa_id].date);	// date
			init_cid_det_si3500(stCIDstr.daa_id);
			fsk_cid_valid[stCIDstr.ch_id] = 0;
                        PRINT_MSG("-->fsk get cid (%d)\n", stCIDstr.ch_id);
		}
		else
		{
			stCIDstr.string[0] = 0;
			stCIDstr.string2[0] = 0;
		}

		restore_flags(flags);
		copy_to_user(user, &stCIDstr, sizeof(TstVoipCID));
#endif
		break;
		
	case VOIP_MGR_SET_CID_DET_MODE:
#ifdef FXO_CALLER_ID_DET
		copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
		save_flags(flags); cli();
		auto_cid_det[stVoipCfg.ch_id] = stVoipCfg.enable;
		cid_type[stVoipCfg.ch_id] = stVoipCfg.cfg;
		restore_flags(flags);
#endif
		break;
		
	case VOIP_MGR_SET_SPK_AGC:
		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
		spk_agc_mode[stVoipValue.ch_id]=stVoipValue.value;
		break;
		
	case VOIP_MGR_SET_SPK_AGC_LVL:
		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
		spk_agc_lvl[stVoipValue.ch_id]=stVoipValue.value;
		break;
		
	case VOIP_MGR_SET_SPK_AGC_GUP:
		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
		spk_agc_gup[stVoipValue.ch_id]=stVoipValue.value;
		break;
		
	case VOIP_MGR_SET_SPK_AGC_GDOWN:
		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
		spk_agc_gdown[stVoipValue.ch_id]=stVoipValue.value;
		break;
		
	case VOIP_MGR_SET_MIC_AGC:
		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
		mic_agc_mode[stVoipValue.ch_id]=stVoipValue.value;
		break;
		
	case VOIP_MGR_SET_MIC_AGC_LVL:
		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
		mic_agc_lvl[stVoipValue.ch_id]=stVoipValue.value;
		break;
		
	case VOIP_MGR_SET_MIC_AGC_GUP:
		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
		mic_agc_gup[stVoipValue.ch_id]=stVoipValue.value;
		break;
		
	case VOIP_MGR_SET_MIC_AGC_GDOWN:
		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
		mic_agc_gdown[stVoipValue.ch_id]=stVoipValue.value;
		break;
		
	case VOIP_MGR_GET_FSK_CID_STATE_CFG:
		copy_from_user(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));
		stCIDstr.cid_state = fsk_cid_state[stCIDstr.ch_id];

		if(fsk_gen_mode[stCIDstr.ch_id])
		{
			if (!fsk_cid_state[stCIDstr.ch_id])
				init_softfskcidGen(stCIDstr.ch_id);
		}
		copy_to_user(user, &stCIDstr, sizeof(TstVoipCID));
		break;
		
	case VOIP_MGR_SET_CID_FSK_GEN_MODE:
		copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
		fsk_gen_mode[stVoipCfg.ch_id]=stVoipCfg.enable;
		break;
		
	case VOIP_MGR_FSK_CID_VMWI_GEN_CFG:
		copy_from_user(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));
		fsk_cid_state[stCIDstr.ch_id]=1;
		// remember set slic in transmit mode, enable DSP pcm.
		init_softfskcidGen(stCIDstr.ch_id);
		genSoftFskCID(stCIDstr.ch_id, stCIDstr.cid_mode, stCIDstr.cid_msg_type, stCIDstr.string, stCIDstr.string2, stCIDstr.cid_name);
		break;
		
	case VOIP_MGR_SET_VOICE_GAIN:
		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
		voice_gain_spk[stVoipValue.ch_id]=stVoipValue.value+32;//0 is mute, 1 is -31dB ~~~ 32 is 0dB , 33 is 1dB ~~~ 63 is 31dB
		voice_gain_mic[stVoipValue.ch_id]=stVoipValue.value1+32;//0 is mute, 1 is -31dB ~~~ 32 is 0dB , 33 is 1dB ~~~ 63 is 31dB
		break;
		
#if 1	/* debug T.38 */
 #ifdef CONFIG_RTK_VOIP_T38
	case VOIP_MGR_GET_T38_PCMIN:
	{
		TstT38PcmIn stT38PcmIn;
		extern uint32 T38Dump_GetPcm( unsigned short *pPcmIn, uint32 *priPcm );

		copy_from_user(&stT38PcmIn, (TstVoipCID *)user, sizeof(TstT38PcmIn));

		stT38PcmIn.ret = T38Dump_GetPcm( stT38PcmIn.pcmIn, &stT38PcmIn.snPcm );

		copy_to_user(user, &stT38PcmIn, sizeof(TstT38PcmIn));
	}
		break;

	case VOIP_MGR_GET_T38_PACKETIN:
	{
		TstT38PacketIn stT38PacketIn;
		extern uint32 T38Dump_GetPacket( uint32 snPcm, unsigned char *pPacket );

		copy_from_user(&stT38PacketIn, (TstVoipCID *)user, sizeof(TstT38PacketIn));

		stT38PacketIn.nSize = T38Dump_GetPacket( stT38PacketIn.snPcm,
												 stT38PacketIn.packetIn );

		copy_to_user(user, &stT38PacketIn, sizeof(TstT38PacketIn));
	}
		break;
 #endif /* CONFIG_RTK_VOIP_T38 */
#endif /* debug T.38 */

	case VOIP_MGR_GET_RTP_STATISTICS:
	{
		extern void ResetRtpStatsCount( uint32 chid );
                extern uint32 nRxRtpStatsCountByte[MAX_SLIC_CH_NUM];
                extern uint32 nRxRtpStatsCountPacket[MAX_SLIC_CH_NUM];
                extern uint32 nRxRtpStatsLostPacket[MAX_SLIC_CH_NUM];
                extern uint32 nTxRtpStatsCountByte[MAX_SLIC_CH_NUM];
                extern uint32 nTxRtpStatsCountPacket[MAX_SLIC_CH_NUM];
		
		copy_from_user(&stVoipRtpStatistics, (TstVoipRtpStatistics *)user, sizeof(TstVoipRtpStatistics));
		
		if( ( ch_id = stVoipRtpStatistics.chid ) > SLIC_CH_NUM )
			break;	/* unexpected chid */
		
		if( stVoipRtpStatistics.bResetStatistics )	/* reset statistics? */
			ResetRtpStatsCount( ch_id );

		/* ok. copy statistics data */
		stVoipRtpStatistics.nRxRtpStatsCountByte 	= nRxRtpStatsCountByte[ ch_id ];
		stVoipRtpStatistics.nRxRtpStatsCountPacket 	= nRxRtpStatsCountPacket[ ch_id ];
		stVoipRtpStatistics.nRxRtpStatsLostPacket 	= nRxRtpStatsLostPacket[ ch_id ];
		stVoipRtpStatistics.nTxRtpStatsCountByte 	= nTxRtpStatsCountByte[ ch_id ];
		stVoipRtpStatistics.nTxRtpStatsCountPacket 	= nTxRtpStatsCountPacket[ ch_id ];

		copy_to_user(user, &stVoipRtpStatistics, sizeof(TstVoipRtpStatistics));
	}
		break;

	case VOIP_MGR_FAX_MODEM_PASS_CFG:
    		copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
    	/* 1: fax, 2: modem */
		stVoipCfg.enable = CED_routine(stVoipCfg.ch_id);
		copy_to_user(user, &stVoipCfg, sizeof(TstVoipCfg));
		break;	

	case VOIP_MGR_FAX_END_DETECT:
		copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
		stVoipCfg.enable = fax_end_flag[stVoipCfg.ch_id];
		copy_to_user(user, &stVoipCfg, sizeof(TstVoipCfg));
		break;//t.38 fax end detect, 1:fax end.

	case VOIP_MGR_ENERGY_DETECT:
#ifdef ENERGY_DET_PCM_IN
		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
		save_flags(flags); cli();
		/* 
		energy_out return value : 0~ 91 dB. 
		If return value = -1, it means FIFO is empty.
		*/
		eng_det_flag[stVoipValue.ch_id] = stVoipValue.value; // 1: enable, 0: disable - energy detect
		stVoipValue.value2 = energy_out(stVoipValue.ch_id);
		restore_flags(flags);
      		copy_to_user(user, &stVoipValue, sizeof(TstVoipValue));
#endif
		break;

#else /* AUDIOCODES_VOIP */

  		/** RTP/RTCP SESSION **/
  		case VOIP_MGR_SET_SESSION:
      			copy_from_user(&stVoipMgrSession, (TstVoipMgrSession *)user, sizeof(TstVoipMgrSession));
			stVoipMgrSession.protocol = UDP_PROTOCOL;

			s_id_rtp = (2*stVoipMgrSession.ch_id+PROTOCOL__RTP-1);
			s_id_rtcp = (2*stVoipMgrSession.ch_id+PROTOCOL__RTCP-1);
			PRINT_MSG("SET RTP(%d)\n", s_id_rtp);
			//PRINT_MSG("s_id_rtp = %d\n", s_id_rtp);
			//PRINT_MSG("s_id_rtcp = %d\n", s_id_rtcp);
			
			#if 0
			PRINT_MSG("ch_id = %d\n", stVoipMgrSession.ch_id);
			PRINT_MSG("m_id = %d\n", stVoipMgrSession.m_id);
			PRINT_MSG("Rtp s_id = %d\n", s_id);
			PRINT_MSG("Rtp ip_src_addr = %d.%d.%d.%d\n", ((unsigned char *)(&stVoipMgrSession.ip_src_addr))[0], ((unsigned char *)(&stVoipMgrSession.ip_src_addr))[1], ((unsigned char *)(&stVoipMgrSession.ip_src_addr))[2], ((unsigned char *)(&stVoipMgrSession.ip_src_addr))[3]);
			PRINT_MSG("Rtp ip_dst_addr = %d.%d.%d.%d\n", ((unsigned char *)(&stVoipMgrSession.ip_dst_addr))[0], ((unsigned char *)(&stVoipMgrSession.ip_dst_addr))[1], ((unsigned char *)(&stVoipMgrSession.ip_dst_addr))[2], ((unsigned char *)(&stVoipMgrSession.ip_dst_addr))[3]);
			PRINT_MSG("Rtp udp_src_port = %d\n", ntohs(stVoipMgrSession.udp_src_port));
			PRINT_MSG("Rtp udp_dst_port = %d\n", ntohs(stVoipMgrSession.udp_dst_port));
			PRINT_MSG("protocol = 0x%x\n", stVoipMgrSession.protocol);
			#endif

			#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
			if(filter[s_id_rtp]!=0) 
			{
                 	 	PRINT_MSG("rtp: s_id %d in used, please unregister first\n", s_id_rtp);
		  		break;
        		}
			if(filter[s_id_rtcp]!=0) 
			{
                 	 	PRINT_MSG("rtcp: s_id %d in used, please unregister first\n", s_id_rtcp);
		  		break;
        		}
			#endif
			
			save_flags(flags); cli();
			#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_RX)
			rtp_pair.isTcp = FALSE;
			rtp_pair.remIp = stVoipMgrSession.ip_src_addr;
			rtp_pair.remPort = stVoipMgrSession.udp_src_port;
			rtp_pair.extIp = stVoipMgrSession.ip_dst_addr;
			rtp_pair.extPort = stVoipMgrSession.udp_dst_port;
			rtp_pair.chid = stVoipMgrSession.ch_id;
			/* rtp register */
			rtp_pair.mid = s_id_rtp;
			result = voip_register_RTPport( /*chid*/ rtp_pair.chid, /*mid*/ rtp_pair.mid, &rtp_pair );
			if(result < 0 )
				PRINT_MSG("865x register RTP failed\n");
			else
			{
				rfc2833_payload_type_local[s_id_rtp] = stVoipMgrSession.rfc2833_payload_type_local;
                        	rfc2833_payload_type_remote[s_id_rtp] = stVoipMgrSession.rfc2833_payload_type_remote;
				PRINT_MSG("865x register RTP success\n");
                	
                		/** Check if local GW is RFC2833 mode, and remote GW support RFC2833, then set ACMW to RFC2833 mode. **/
                		if ( dtmf_mode[stVoipMgrSession.ch_id] == 0 )/*RFC2833*/
				{
					if (rfc2833_payload_type_remote[s_id_rtp] != 0)
					{	
						RtkAc49xApiSetIbsTransferMode(stVoipMgrSession.ch_id, IBS_TRANSFER_MODE__RELAY_ENABLE_VOICE_MUTE);
						//RtkAc49xApiSetDtmfErasureMode(stVoipMgrSession.ch_id, DTMF_ERASURE_MODE__ERASE_1_COMPONENT);
					}
					else
					{
						RtkAc49xApiSetIbsTransferMode(stVoipMgrSession.ch_id, IBS_TRANSFER_MODE__TRANSPARENT_THROUGH_VOICE);
					}
				}
				/*******************************************************************************************************/
			}
			/* rtcp register (rtcp port = rtp port + 1) */
			rtp_pair.remPort = rtp_pair.remPort + 1;
			rtp_pair.extPort = rtp_pair.extPort + 1;	
			rtp_pair.mid = s_id_rtcp;
			result = voip_register_RTPport( /*chid*/ rtp_pair.chid, /*mid*/ rtp_pair.mid, &rtp_pair );
			if(result < 0 )
				PRINT_MSG("865x register RTP failed\n");
			else
				PRINT_MSG("865x register RTP success\n");
			#else
			/* rtp register */
			stVoipMgrSession.m_id = PROTOCOL__RTP;
			rtk_trap_register(&stVoipMgrSession, stVoipMgrSession.ch_id, stVoipMgrSession.m_id, s_id_rtp,  Ac49xTxPacketCB);
			/* rtcp register (rtcp port = rtp port + 1) */
			stVoipMgrSession.udp_src_port = stVoipMgrSession.udp_src_port + 1;
			stVoipMgrSession.udp_dst_port = stVoipMgrSession.udp_dst_port + 1;
			stVoipMgrSession.m_id = PROTOCOL__RTCP;
			rtk_trap_register(&stVoipMgrSession, stVoipMgrSession.ch_id, stVoipMgrSession.m_id, s_id_rtcp,  Ac49xTxPacketCB);
			rfc2833_payload_type_local[s_id_rtp] = stVoipMgrSession.rfc2833_payload_type_local;
                	rfc2833_payload_type_remote[s_id_rtp] = stVoipMgrSession.rfc2833_payload_type_remote;
                
                	/** Check if local GW is RFC2833 mode, and remote GW support RFC2833, then set ACMW to RFC2833 mode. **/
                	if ( dtmf_mode[stVoipMgrSession.ch_id] == 0 )/*RFC2833*/
			{
				if (rfc2833_payload_type_remote[s_id_rtp] != 0) 
				{	
					RtkAc49xApiSetIbsTransferMode(stVoipMgrSession.ch_id, IBS_TRANSFER_MODE__RELAY_ENABLE_VOICE_MUTE);
					//RtkAc49xApiSetDtmfErasureMode(stVoipMgrSession.ch_id, DTMF_ERASURE_MODE__ERASE_1_COMPONENT);
				}
				else if (rfc2833_payload_type_remote[s_id_rtp] == 0) 
				{
					RtkAc49xApiSetIbsTransferMode(stVoipMgrSession.ch_id, IBS_TRANSFER_MODE__TRANSPARENT_THROUGH_VOICE);
				}
			}
			/*******************************************************************************************************/
			#endif
						
			restore_flags(flags);
			break;
		
		case VOIP_MGR_UNSET_SESSION:
      			copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

			s_id_rtp = (2*stVoipCfg.ch_id+PROTOCOL__RTP-1);
			s_id_rtcp = (2*stVoipCfg.ch_id+PROTOCOL__RTCP-1);
			PRINT_MSG("UNSET RTP(%d)\n", s_id_rtp);
			//PRINT_MSG("s_id_rtp = %d\n", s_id_rtp);
			//PRINT_MSG("s_id_rtcp = %d\n", s_id_rtcp);
			save_flags(flags); cli();
			
			#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_RX)
			rtp_pair.chid = stVoipCfg.ch_id;
			
			rtp_pair.mid = s_id_rtp;
			result = voip_register_RTPport( /*chid*/ rtp_pair.chid, /*mid*/ rtp_pair.mid, NULL );
			if( result < 0 )
				PRINT_MSG("Unregister 865x RTP port  failed\n");
                	else
				PRINT_MSG("Unregister 865x RTP port  successfully\n");
			
			rtp_pair.mid = s_id_rtcp;
			result = voip_register_RTPport( /*chid*/ rtp_pair.chid, /*mid*/ rtp_pair.mid, NULL );
			if( result < 0 )
				PRINT_MSG("Unregister 865x RTP port  failed\n");
                	else
				PRINT_MSG("Unregister 865x RTP port  successfully\n");       
			#else
			rtk_trap_unregister(s_id_rtp);
			rtk_trap_unregister(s_id_rtcp);
			#endif

			restore_flags(flags);
			break;
		
        	case VOIP_MGR_SET_RTCP_SESSION:
			/* For ACMW, RTCP is bonded with RTP. This IO Ctrl not need to implement.*/
			break;
			
		case VOIP_MGR_UNSET_RTCP_SESSION:
			/* For ACMW, RTCP is bonded with RTP. This IO Ctrl not need to implement. */
			break;

		case VOIP_MGR_SET_RTCP_TX_INTERVAL:
			copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
			save_flags(flags); cli();
			/* If RtcpMeanTxInterval is equal to 0, then RTCP Tx is disable.*/
			RtkAc49xApiSetRtcpTxInterval(stVoipValue.ch_id, stVoipValue.value5);
			restore_flags(flags);
			break;

		case VOIP_MGR_SETRTPPAYLOADTYPE:
			copy_from_user(&stVoipPayLoadTypeConfig, (TstVoipPayLoadTypeConfig *)user, sizeof(TstVoipPayLoadTypeConfig));

			PRINT_MSG("VOIP_MGR_SETRTPPAYLOADTYPE(ch=%d, mid=%d)\n", stVoipPayLoadTypeConfig.ch_id, stVoipPayLoadTypeConfig.m_id);
			s_id = (2*stVoipPayLoadTypeConfig.ch_id + PROTOCOL__RTP - 1);
			save_flags(flags); cli();
			#ifdef T38_STAND_ALONE_HANDLER
			if( stVoipPayLoadTypeConfig.uPktFormat == rtpPayloadT38_Virtual ) 
			{
				T38_API_Initialize( stVoipPayLoadTypeConfig.ch_id );
				PRINT_MSG("MGR: Initialize T38(%d)\n", stVoipPayLoadTypeConfig.ch_id);
				t38RunningState[ stVoipPayLoadTypeConfig.ch_id ] = T38_START;
				restore_flags(flags);
				break;
			}
			else
				t38RunningState[ stVoipPayLoadTypeConfig.ch_id ] = T38_STOP;		
			#endif
			RtkAc49xApiSetRtpChannelConfiguration(stVoipPayLoadTypeConfig.ch_id, s_id, stVoipPayLoadTypeConfig.uPktFormat, stVoipPayLoadTypeConfig.nG723Type, stVoipPayLoadTypeConfig.bVAD);
			if (CHANNEL_STATE__ACTIVE_RTP != RtkAc49xGetChannelState(stVoipPayLoadTypeConfig.ch_id))
				RtkAc49xApiSetJBDelay(stVoipPayLoadTypeConfig.ch_id, 10*stVoipPayLoadTypeConfig.nMaxDelay, 10*stVoipPayLoadTypeConfig.nJitterDelay, stVoipPayLoadTypeConfig.nJitterFactor);
			restore_flags(flags);
			break;

		case VOIP_MGR_SETRTPSESSIONSTATE:
	      		copy_from_user(&stVoipRtpSessionState, (TstVoipRtpSessionState *)user, sizeof(TstVoipRtpSessionState));
	      		
	      		ret = NULL;
			ch_id = stVoipRtpSessionState.ch_id;
			m_id = stVoipRtpSessionState.m_id;
			
			s_id_rtp = 2*ch_id + PROTOCOL__RTP - 1;
      			s_id_rtcp = 2*ch_id + PROTOCOL__RTCP - 1;
      			
	             	PRINT_MSG("VOIP_MGR_SETRTPSESSIONSTATE:ch_id=%d, m_id=%d, s_id=%d, state=%d\n", ch_id, m_id, s_id_rtp, stVoipRtpSessionState.state);

			save_flags(flags); cli();
			
			switch(stVoipRtpSessionState.state)
			{
				case rtp_session_sendonly://pass through
				case rtp_session_recvonly: //pass through
				case rtp_session_sendrecv:
				{
					if (rtpConfigOK[s_id_rtp] == 0)
					ret = RtkAc49xApiActiveRegularRtp(ch_id, s_id_rtp); /* return 0 : success */
				
					if (ret == 0)
					{
						rtpConfigOK[s_id_rtp] = 1;
						rtpConfigOK[s_id_rtcp] = 1;
						PRINT_MSG("open RTP(%d)\n", s_id_rtp);
					}
				}
				break;
				
				case rtp_session_inactive:
				
					if (rtpConfigOK[s_id_rtp] == 1)
					ret = RtkAc49xApiCloseRegularRtp(ch_id, s_id_rtp); /* return 0 : success */
				
					if (ret == 0)
					{
						rtpConfigOK[s_id_rtp] = 0;
						rtpConfigOK[s_id_rtcp] = 0;
						PRINT_MSG("close RTP(%d)\n", s_id_rtp);
					}
				break;
			}

			restore_flags(flags);
			
			break;

		/** Play Tone **/
		case VOIP_MGR_SETPLAYTONE:
      			copy_from_user(&stVoipPlayToneConfig, (TstVoipPlayToneConfig*)user, sizeof(TstVoipPlayToneConfig));
			save_flags(flags); cli();
      			s_id = 2*stVoipPlayToneConfig.ch_id + PROTOCOL__RTP - 1;
      			RtkAc49xApiPlayTone(stVoipPlayToneConfig.ch_id, s_id, stVoipPlayToneConfig.nTone, stVoipPlayToneConfig.bFlag, stVoipPlayToneConfig.path);
      			restore_flags(flags);
      			break;

		case VOIP_MGR_RTP_CFG:
      			copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

      			PRINT_MSG("VOIP_MGR_RTP_CFG:ch_id = %d, enable = %d\n", stVoipCfg.ch_id, stVoipCfg.enable);
      			ret = NULL;

			s_id_rtp = 2*stVoipCfg.ch_id + PROTOCOL__RTP - 1;
      			s_id_rtcp = 2*stVoipCfg.ch_id + PROTOCOL__RTCP - 1;
      			

     			if(stVoipCfg.enable ==1)
      			{
      				if (rtpConfigOK[s_id_rtp] == 0)
					ret = RtkAc49xApiActiveRegularRtp(stVoipCfg.ch_id, s_id_rtp); /* return 0 : success */
				
				if (ret == 0)
				{
					rtpConfigOK[s_id_rtp] = 1;
					rtpConfigOK[s_id_rtcp] = 1;
					PRINT_MSG("open RTP(%d)\n", s_id_rtp);
				}
			}
			else
			{
				if (rtpConfigOK[s_id_rtp] == 1)
					ret = RtkAc49xApiCloseRegularRtp(stVoipCfg.ch_id, s_id_rtp); /* return 0 : success */
				
				if (ret == 0)
				{
					rtpConfigOK[s_id_rtp] = 0;
					rtpConfigOK[s_id_rtcp] = 0;
					PRINT_MSG("close RTP(%d)\n", s_id_rtp);
				}
			}

			break;

		/** DTMF Event **/
		case VOIP_MGR_DTMF_CFG:
      			copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
                	//PRINT_MSG("VOIP_MGR_DTMF_CFG:ch_id = %d, enable = %d\n", stVoipCfg.ch_id, stVoipCfg.enable);
                	//RtkAc49xApiSetDtmfDetection(stVoipCfg.ch_id, stVoipCfg.enable);
                	// NOTE: 
                	// Can NOT call RtkAc49xApiSetDtmfDetection() here, or it will cause acmw to close regular rtp fail.
                	// Developer should make sure that enable DTMF det after acmw channel open, and disable DTMF det before acmw channel close.
                	// Or it will not work and may result some bad effect.
                	// Now, enable / disable DTMF det is integrated in IO Ctrl VOIP_MGR_SLIC_ONHOOK_ACTION and VOIP_MGR_SLIC_OFFHOOK_ACTION.
			break;

		/** SLIC EVENT **/	
		case VOIP_MGR_SLIC_HOOK:
      			copy_from_user(&stVoipSlicHook, (TstVoipSlicHook *)user, sizeof(TstVoipSlicHook));
			#ifdef EVENT_POLLING_TIMER
			hook_out(stVoipSlicHook.ch_id, &stVoipSlicHook.hook_status);
			#endif
			switch (stVoipSlicHook.hook_status)
			{
				case PHONE_OFF_HOOK:
					//PRINT_MSG("VOIP_MGR_SLIC_HOOK: PHONE_OFF_HOOK\n");
					break;
				case PHONE_ON_HOOK:
					//PRINT_MSG("VOIP_MGR_SLIC_HOOK: PHONE_ON_HOOK\n");	
					break;
				case PHONE_FLASH_HOOK:
					//PRINT_MSG("VOIP_MGR_SLIC_HOOK: PHONE_FLASH_HOOK\n");
					break;
				default:
					break;
			}
			copy_to_user(user, &stVoipSlicHook, sizeof(TstVoipSlicHook));
			break;

		case VOIP_MGR_DSPCODECSTOP:
	                PRINT_MSG("VOIP_MGR_DSPCODECSTOP\n");
	      		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
			save_flags(flags); cli();
			#ifdef T38_STAND_ALONE_HANDLER
			t38RunningState[ stVoipValue.ch_id ] = T38_STOP;
			#endif
			restore_flags(flags);
			break;

               	case VOIP_MGR_SETCONFERENCE:
			copy_from_user(&stVoipMgr3WayCfg, (TstVoipMgr3WayCfg *)user, sizeof(TstVoipMgr3WayCfg));
			PRINT_MSG("VOIP_MGR_SETCONFERENCE:ch_id = %d, enable = %d\n", stVoipMgr3WayCfg.ch_id, stVoipMgr3WayCfg.enable);
         
			int mmid;
			int isMode3[2] = {0};

			/* Check which mid session is mode3. Note: If src ip = dst ip, this session is mode3. */			
			for (mmid=0; mmid<=1; mmid++)
      			{
				if (stVoipMgr3WayCfg.rtp_cfg[mmid].ip_src_addr == stVoipMgr3WayCfg.rtp_cfg[mmid].ip_dst_addr)
				{
					isMode3[mmid] = 1;
					PRINT_MSG("isMode3[%d]=%d\n", mmid, isMode3[mmid]);
				}
			}
			
			if(stVoipMgr3WayCfg.enable == 1)
      			{
     				IsMode3[stVoipMgr3WayCfg.ch_id] = isMode3[0] | isMode3[1];
     				
      				////////// Register RTP Session //////////
      				for (mmid=0; mmid<=1; mmid++)
      				{
      					///// The rule to decide the mid(0, 1) use which sid resource((rtp,rtcp)-(0,1), (2,3)) ////
	      				///// If mid 0 is mode3, then it always use the sid (2,3), and mid 1 use sid (0,1) ////////
	      				///// If mid 1 is mode3, then it always use the sid (2,3), and mid 0 use sid (0,1) ////////
	      				///// If both mid is NOT mode3, then mid 1 always use sid (2,3), mid 0 always use sid (0,1).
	      				if (isMode3[mmid] == 1)
	      					stVoipMgr3WayCfg.rtp_cfg[mmid].ch_id = stVoipMgr3WayCfg.rtp_cfg[mmid].ch_id^1;
	      				else if ((stVoipMgr3WayCfg.rtp_cfg[mmid].m_id == 1) && isMode3[mmid^1] == 0)
						stVoipMgr3WayCfg.rtp_cfg[mmid].ch_id = stVoipMgr3WayCfg.rtp_cfg[mmid].ch_id^1;
					//////////////////////////////////////////////////////////////////////////////////////////
						
					if (isMode3[mmid] == 0)
					{
						voip_mgr_unset_rtp_1(stVoipMgr3WayCfg.rtp_cfg[mmid].ch_id, 0 /*don't care here*/);
						voip_mgr_unset_rtp_2(stVoipMgr3WayCfg.rtp_cfg[mmid].ch_id, 0 /*don't care here*/);
						voip_mgr_set_rtp_1(stVoipMgr3WayCfg.rtp_cfg[mmid]);
					}
					else if (isMode3[mmid] == 1)
					{
						/// If mmid is mode3, just de-active RTP, don't unregister sid session used by mmid.
						voip_mgr_unset_rtp_2(stVoipMgr3WayCfg.rtp_cfg[mmid].ch_id, 0 /*don't care here*/);
					}
					
				}
				
				
      				
      				////////// Active 3-Way Conference ///////////
      				RtkAc49xApiOnhookAction(stVoipMgr3WayCfg.ch_id);
      				if (IsMode3[stVoipMgr3WayCfg.ch_id] == 1)
      					RtkAc49xApiOnhookAction(stVoipMgr3WayCfg.ch_id^1);
      				RtkAc49xApiOffhookAction(stVoipMgr3WayCfg.ch_id);
      				
      				if (IsMode3[stVoipMgr3WayCfg.ch_id] == 1)
      					RtkAc49xApiActiveOrDeactive3WayConference(stVoipMgr3WayCfg.ch_id, _3_WAY_CONFERENCE_MODE__3);
      				else
      					RtkAc49xApiActiveOrDeactive3WayConference(stVoipMgr3WayCfg.ch_id, _3_WAY_CONFERENCE_MODE__1);
      				
      				for (mmid=0; mmid<=1; mmid++)
      				{	
	      				stVoipMgr3WayCfg.rtp_cfg[mmid].state = 1; //for open RTP
	      				
	      				PRINT_MSG("isMode3[%d]=%d\n", mmid, isMode3[mmid]);
      					if (isMode3[mmid] != 1)
      					{
	      					if (IsMode3[stVoipMgr3WayCfg.ch_id] == 1)
	      					{
	      						if (stVoipMgr3WayCfg.rtp_cfg[mmid].m_id == 1)
		      						stVoipMgr3WayCfg.rtp_cfg[mmid].ch_id = stVoipMgr3WayCfg.ch_id;
		      				}
      						voip_mgr_set_rtp_2(stVoipMgr3WayCfg.rtp_cfg[mmid]);
      					}
      				}
      				
      			}
      			else if(stVoipMgr3WayCfg.enable == 0)
      			{
      				RtkAc49xApiActiveOrDeactive3WayConference(stVoipMgr3WayCfg.ch_id, _3_WAY_CONFERENCE_MODE__DISABLE);
      				
      				voip_mgr_unset_rtp_1(stVoipMgr3WayCfg.ch_id, 0 /*don't care here*/);
      				voip_mgr_unset_rtp_2(stVoipMgr3WayCfg.ch_id, 0 /*don't care here*/);
      				
      				if (IsMode3[stVoipMgr3WayCfg.ch_id] == 0)
       				{
      					voip_mgr_unset_rtp_1(stVoipMgr3WayCfg.ch_id^1, 0 /*don't care here*/);
      					voip_mgr_unset_rtp_2(stVoipMgr3WayCfg.ch_id^1, 0 /*don't care here*/);
      				}
      				else if (IsMode3[stVoipMgr3WayCfg.ch_id] == 1)
       				{
      					voip_mgr_unset_rtp_2(stVoipMgr3WayCfg.ch_id^1, 0 /*don't care here*/);
      				}
      				
      				RtkAc49xApiOnhookAction(stVoipMgr3WayCfg.ch_id);
				RtkAc49xApiOnhookAction(stVoipMgr3WayCfg.ch_id^1);
				RtkAc49xApiOffhookAction(stVoipMgr3WayCfg.ch_id);
				
				if (IsMode3[stVoipMgr3WayCfg.ch_id] == 1)
      				{
      					RtkAc49xApiOffhookAction(stVoipMgr3WayCfg.ch_id^1);//must
      					
      					for (mmid=0; mmid<=1; mmid++)
      					{
      						if ((isMode3[mmid] == 1) && (stVoipMgr3WayCfg.rtp_cfg[mmid].ip_src_addr != 0))
      						{
      							stVoipMgr3WayCfg.rtp_cfg[mmid].state = 1; //for open RTP
      							stVoipMgr3WayCfg.rtp_cfg[mmid].ch_id = stVoipMgr3WayCfg.rtp_cfg[mmid].ch_id^1;
      							voip_mgr_set_rtp_2(stVoipMgr3WayCfg.rtp_cfg[mmid]);
      						}
      					}
      					
      				}
				
				IsMode3[stVoipMgr3WayCfg.ch_id] = 0;
      			}


			break;

		case VOIP_MGR_ON_HOOK_RE_INIT:
			copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
			dtmf_cid_state[stVoipCfg.ch_id] = 0;
			flush_fax_modem_fifo(stVoipCfg.ch_id);
			break;

		case VOIP_MGR_DTMF_CID_GEN_CFG:
			copy_from_user(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));
			dtmf_cid_state[stCIDstr.ch_id]=1;
			strcpy(cid_str, stCIDstr.string);
			RtkAc49xApiSendDtmfCallerId(stCIDstr.ch_id, 0, cid_str);
			break;

		case VOIP_MGR_GET_CID_STATE_CFG: // Get DTMF CID state
			copy_from_user(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));
			stCIDstr.cid_state = dtmf_cid_state[stCIDstr.ch_id];
			copy_to_user(user, &stCIDstr, sizeof(TstVoipCID));
			break;

		case VOIP_MGR_SET_DTMF_MODE: 
			copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
		
			if (stVoipCfg.enable == 2) // Inband
			{
				dtmf_mode[stVoipCfg.ch_id] = 2;
				RtkAc49xApiSetIbsTransferMode(stVoipCfg.ch_id, IBS_TRANSFER_MODE__TRANSPARENT_THROUGH_VOICE);
			}
			else if (stVoipCfg.enable == 1) //SIP Info
			{
				dtmf_mode[stVoipCfg.ch_id] = 1;
				RtkAc49xApiSetIbsTransferMode(stVoipCfg.ch_id, IBS_TRANSFER_MODE__RELAY_DISABLE_VOICE_MUTE);
			}
			else if (stVoipCfg.enable == 0) // RFC2833
			{
				dtmf_mode[stVoipCfg.ch_id] = 0;
				/* 
				 * Here, NOT set the ACMW Ibs Transfer Mode to RFC2833.
				 * The time to set ACMW Ibs Transfer Mode to RFC2833 is in time of setting RTP session. 
				 * See the IO control VOIP_MGR_SET_SESSION.
				 */
			}
			break;

		case VOIP_MGR_FSK_CID_GEN_CFG:
			copy_from_user(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));
			fsk_cid_state[stCIDstr.ch_id] = 1;
			RtkAc49xApiSendFskCallerId(stCIDstr.ch_id, stCIDstr.cid_mode, FSK_MSG_CALLSETUP, stCIDstr.string,  stCIDstr.string2, stCIDstr.cid_name);
                	break;

      		/** Echo Canceller Length Config **/	
 		case VOIP_MGR_SET_ECHO_TAIL_LENGTH:
			copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
			Tac49xEchoCancelerLength ec_length;
			if (stVoipValue.value == 8) //stVoipValue.value : unit is ms
				ec_length = ECHO_CANCELER_LENGTH__8_MSEC;
			else if (stVoipValue.value == 16)
				ec_length = ECHO_CANCELER_LENGTH__16_MSEC;
			else if (stVoipValue.value == 24)
				ec_length = ECHO_CANCELER_LENGTH__24_MSEC;
			else if (stVoipValue.value == 32)
				ec_length = ECHO_CANCELER_LENGTH__32_MSEC;
			else
				PRINT_MSG("Only 8, 16, 24 and 32 msec EC lengths are supported.\n");
				
			RtkAc49xApiUpdateEchoCancellerLength(stVoipValue.ch_id, ec_length);
			break;

		case VOIP_MGR_SEND_RFC2833_PKT_CFG:
			copy_from_user(&stRFC2833, (TstVoip2833 *)user, sizeof(TstVoip2833));
			/* This IO ctrl is not necessary for acmw. */
			break;
					
		case 	VOIP_MGR_SET_G168_LEC_CFG:
			copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
			/* This IO ctrl is the switch of LEC. */ /* 0: LEC disable  1: LEC enable */
			break;

		case VOIP_MGR_SET_FSK_VMWI_STATE:
			copy_from_user(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));
			fsk_cid_state[stCIDstr.ch_id] = 1;
			RtkAc49xApiSendVmwi(stCIDstr.ch_id, FSK_MSG_MWSETUP, stCIDstr.string);
			break;

		case VOIP_MGR_SET_FSK_AREA:
			copy_from_user(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));
			fsk_spec_areas[stCIDstr.ch_id] = stCIDstr.cid_mode; /* 0:Bellcore 1:ETSI 2:BT 3:NTT */
			fsk_cid_gain[stCIDstr.ch_id] = stCIDstr.cid_gain; /* Only support multiple 1~5 */
                	PRINT_MSG("\nReset the fsk setting %d\n",fsk_spec_areas[stCIDstr.ch_id]);
			break;

		case VOIP_MGR_FSK_ALERT_GEN_CFG:
			copy_from_user(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));
			//fsk_alert_state[stCIDstr.ch_id] = 1;
			//strcpy(cid_str, stCIDstr.string);
			break;

		case VOIP_MGR_SET_CID_DTMF_MODE:
			copy_from_user(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));
			fsk_spec_areas[stCIDstr.ch_id] = stCIDstr.cid_mode; 	/* when DTMF mode, set the before ring or after ring send cid */
			cid_dtmf_mode[stCIDstr.ch_id] = stCIDstr.cid_dtmf_mode;	/* when DTMF mode, set the caller id start/end digit */
			break;

		case VOIP_MGR_PLAY_SIP_INFO:
			copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

			RtkAc49xApiGenDtmfTone(stVoipValue.ch_id, stVoipValue.value, stVoipValue.value5, 0 /*Off-duration*/);
			break;

		/* DAA */
		case VOIP_MGR_SET_DAA_ISR_FLOW:
			copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
	#ifdef CONFIG_RTK_SLIC_DAA_NEGOTIATION
			save_flags(flags); cli();
			DAA_ISR_FLOW[stVoipValue.ch_id] = stVoipValue.value;
			s_id = 2*stVoipValue.ch_id+ PROTOCOL__RTP - 1;
	
			if (stVoipValue.value != DRIVER_FLOW_NORMAL)// not normal flow
					sid_owner[s_id] = 1; //DAA own
			else
					sid_owner[s_id] = 0; //SLIC own
			restore_flags(flags);
	                PRINT_MSG("rtk_Set_DAA_ISR_FLOW: chid=%d, flow=%d, mid=%d, sid=%d\n", stVoipValue.ch_id, stVoipValue.value, stVoipValue.m_id, s_id);
	#endif
			break;
			
		case VOIP_MGR_GET_DAA_ISR_FLOW:
			copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
	#ifdef CONFIG_RTK_SLIC_DAA_NEGOTIATION
			save_flags(flags); cli();
			stVoipValue.value = DAA_ISR_FLOW[stVoipValue.ch_id];
			restore_flags(flags);
			copy_to_user(user, &stVoipValue, sizeof(TstVoipValue));
	                PRINT_MSG("rtk_Get_DAA_ISR_FLOW: chid=%d, flow=%d\n", stVoipValue.ch_id, stVoipValue.value);
	#endif
			break;
			
		case VOIP_MGR_GET_DAA_USED_BY_WHICH_SLIC:
			copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
			save_flags(flags); cli();
			stVoipValue.value = g_DAA_used[stVoipValue.ch_id];
			restore_flags(flags);
			copy_to_user(user, &stVoipValue, sizeof(TstVoipValue));
			break;
			
		case VOIP_MGR_SET_DAA_PCM_HOLD_CFG:
			copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
	#ifdef CONFIG_RTK_SLIC_DAA_NEGOTIATION
			save_flags(flags); cli();
			if (stVoipValue.value == 1)
			{
				pcm_Hold_DAA(stVoipValue.value2); /* hold daa used pcm channel */
	
				if ( g_DAA_used[stVoipValue.ch_id] == DAA_USE)
					g_DAA_used[stVoipValue.ch_id] = DAA_USE_HOLD; // hold
			}
			else
			{
				pcm_UnHold_DAA(stVoipValue.value2);
	
				if ( g_DAA_used[stVoipValue.ch_id] == DAA_USE_HOLD)
					g_DAA_used[stVoipValue.ch_id] = DAA_USE; // un-hold
			}
			restore_flags(flags);
	                PRINT_MSG("VOIP_MGR_SET_DAA_PCM_HOLD_CFG: chid=%d, enable=%d\n", stVoipValue.ch_id, stVoipValue.value);
	#endif
			break;
			
		case VOIP_MGR_GET_DAA_BUSY_TONE_STATUS:
			copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
	#ifdef FXO_BUSY_TONE_DET
			save_flags(flags); cli();
			stVoipValue.value = busy_tone_flag_get();/* Busy tone is  1: Detected, 0: NOT detected. */
			restore_flags(flags);
			copy_to_user(user, &stVoipValue, sizeof(TstVoipValue));
	#endif
			break;
			
		case VOIP_MGR_GET_DAA_CALLER_ID:
	#ifdef FXO_CALLER_ID_DET
			copy_from_user(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));
			save_flags(flags); cli();
	
			if (cid_res.cid_valid == 1)
			{
				for (i=0; i < SLIC_CH_NUM; i++)
					dtmf_cid_valid[i] = 1;
			}
			else if (stVoipciddet[stCIDstr.daa_id].cid_valid == 1)
			{
				for (i=0; i < SLIC_CH_NUM; i++)
					fsk_cid_valid[i] = 1;
			}
	
			if (dtmf_cid_valid[stCIDstr.ch_id] == 1) // DTMF
			{
				strcpy(stCIDstr.string, cid_res.number); // caller id
				stCIDstr.string2[0] = 0;
				dmtf_cid_det_init();
				dtmf_cid_valid[stCIDstr.ch_id] = 0;
	                        PRINT_MSG("-->dtmf get cid (%d)\n", stCIDstr.ch_id);
			}
			else if (fsk_cid_valid[stCIDstr.ch_id] == 1) // FSK
			{
				strcpy(stCIDstr.string, stVoipciddet[stCIDstr.daa_id].number);	// caller id
				strcpy(stCIDstr.string2, stVoipciddet[stCIDstr.daa_id].date);	// date
				init_cid_det_si3500(stCIDstr.daa_id);
				fsk_cid_valid[stCIDstr.ch_id] = 0;
	                        PRINT_MSG("-->fsk get cid (%d)\n", stCIDstr.ch_id);
			}
			else
			{
				stCIDstr.string[0] = 0;
				stCIDstr.string2[0] = 0;
			}
	
			restore_flags(flags);
			copy_to_user(user, &stCIDstr, sizeof(TstVoipCID));
	#endif
			break;

		case VOIP_MGR_SET_CID_DET_MODE:
	#ifdef FXO_CALLER_ID_DET
			copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
			save_flags(flags); cli();
			auto_cid_det[stVoipCfg.ch_id] = stVoipCfg.enable;
			cid_type[stVoipCfg.ch_id] = stVoipCfg.cfg;
			restore_flags(flags);
	#endif
			break;

		/** AGC **/	
		case VOIP_MGR_SET_SPK_AGC:
			copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
			spk_agc_mode[stVoipValue.ch_id]=stVoipValue.value;
			agc_enable[stVoipValue.ch_id] =  spk_agc_mode[stVoipValue.ch_id];
			RtkAc49xApiAgcConfig(stVoipValue.ch_id, agc_enable[stVoipValue.ch_id], AGC_LOCATION__AT_DECODER_OUTPUT);
			break;
		
		case VOIP_MGR_SET_MIC_AGC:
			copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
			mic_agc_mode[stVoipValue.ch_id]=stVoipValue.value;
			agc_enable[stVoipValue.ch_id] = agc_enable[stVoipValue.ch_id] | mic_agc_mode[stVoipValue.ch_id];
			if (mic_agc_mode[stVoipValue.ch_id] == 1)
				RtkAc49xApiAgcConfig(stVoipValue.ch_id, agc_enable[stVoipValue.ch_id], AGC_LOCATION__AT_ENCODER_INPUT);
			break;
	
		case VOIP_MGR_SET_MIC_AGC_LVL:				
			copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
			
			if (mic_agc_mode[stVoipValue.ch_id]==1)// MIC AGC is enable
			{
				Tac49xAgcTargetEnergy tar_eng;
				/* stVoipValue.value range: 0(small:-25dBm) to 8(large:-1dBm), space: 3dBm */
				tar_eng = AGC_TARGET_ENERGY__minus25_DBM - 3*stVoipValue.value;
				RtkAc49xApiAgcEnergySlope(stVoipValue.ch_id, tar_eng, AGC_GAIN_SLOPE__1_00_DB_SEC);
			}
			break;
							
		case VOIP_MGR_SET_SPK_AGC_LVL:
			copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
			
			if (spk_agc_mode[stVoipValue.ch_id]==1)// SPK AGC is enable
			{
			Tac49xAgcTargetEnergy tar_eng;
				/* stVoipValue.value range: 0(small:-25dBm) to 8(large:-1dBm), space: 3dBm */
				tar_eng = AGC_TARGET_ENERGY__minus25_DBM - 3*stVoipValue.value;
			RtkAc49xApiAgcEnergySlope(stVoipValue.ch_id, tar_eng, AGC_GAIN_SLOPE__1_00_DB_SEC);
			}
			break;
		
		case VOIP_MGR_SET_SPK_AGC_GUP:
			copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
			spk_agc_gup = stVoipValue.value;
			break;
			
		case VOIP_MGR_SET_SPK_AGC_GDOWN:
			copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
			spk_agc_gdown = stVoipValue.value;
			//Note: IO ctrl VOIP_MGR_SET_SPK_AGC_GUP should be called first.
			if (spk_agc_mode[stVoipValue.ch_id] == 1)
				RtkAc49xApiAgcDeviceConfig(spk_agc_gdown+1, spk_agc_gup+1);
			break;
		
		case VOIP_MGR_SET_MIC_AGC_GUP:
			copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
			mic_agc_gup = stVoipValue.value;
			break;
			
		case VOIP_MGR_SET_MIC_AGC_GDOWN:
			copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
			mic_agc_gdown = stVoipValue.value;
			//Note: IO ctrl VOIP_MGR_SET_MIC_AGC_GUP should be called first.
			if (mic_agc_mode[stVoipValue.ch_id] == 1)
				RtkAc49xApiAgcDeviceConfig(mic_agc_gdown+1, mic_agc_gup+1);
			break;
			
		case VOIP_MGR_GET_FSK_CID_STATE_CFG: // Get FSK CID state
			copy_from_user(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));
			stCIDstr.cid_state = fsk_cid_state[stCIDstr.ch_id];
			copy_to_user(user, &stCIDstr, sizeof(TstVoipCID));
			break;

		case VOIP_MGR_FSK_CID_VMWI_GEN_CFG:
			copy_from_user(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));
			/* Not used */
			break;
			
		/** Voice Gain Adjustment **/
		case VOIP_MGR_SET_VOICE_GAIN:
			copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
			Tac49xVoiceGain mic_gain, spk_gain;
			
			spk_gain = stVoipValue.value+32;  //0 is mute, 1 is -31dB ~~~ 32 is 0dB , 33 is 1dB ~~~ 63 is 31dB
			mic_gain = stVoipValue.value1+32; //0 is mute, 1 is -31dB ~~~ 32 is 0dB , 33 is 1dB ~~~ 63 is 31dB
			
			RtkAc49xApiSetVoiceGain(stVoipValue.ch_id, mic_gain, spk_gain);
			break;

		case VOIP_MGR_GET_RTP_STATISTICS:
			copy_from_user(&stVoipRtpStatistics, (TstVoipRtpStatistics *)user, sizeof(TstVoipRtpStatistics));

			extern uint32 nRxRtpStatsCountByte[];
			extern uint32 nRxRtpStatsCountPacket[];
			extern uint32 nRxRtpStatsLostPacket[];
			extern uint32 nTxRtpStatsCountByte[];
			extern uint32 nTxRtpStatsCountPacket[];
			extern uint32 gRtcpStatsUpdOk[];
					
			if( ( ch_id = stVoipRtpStatistics.chid ) > SLIC_CH_NUM )
				break;	/* unexpected chid */
		
			if( stVoipRtpStatistics.bResetStatistics )	/* reset statistics? */
			{
				RtkAc49xApiResetRtpStatistics(ch_id);
			}

			RtkAc49xApiGetRtpStatistics(ch_id);
			
			PRINT_MSG("wait");
			while(!gRtcpStatsUpdOk[ch_id])
			{
				PRINT_MSG(".");
			}
			
			
			/* ok. copy statistics data */
			stVoipRtpStatistics.nRxRtpStatsCountByte 	= nRxRtpStatsCountByte[ ch_id ];
			stVoipRtpStatistics.nRxRtpStatsCountPacket 	= nRxRtpStatsCountPacket[ ch_id ];
			stVoipRtpStatistics.nRxRtpStatsLostPacket 	= nRxRtpStatsLostPacket[ ch_id ];
			stVoipRtpStatistics.nTxRtpStatsCountByte 	= nTxRtpStatsCountByte[ ch_id ];
			stVoipRtpStatistics.nTxRtpStatsCountPacket 	= nTxRtpStatsCountPacket[ ch_id ];

			gRtcpStatsUpdOk[ch_id] = 0;
			
			//PRINT_MSG("CH%d-(%d, %d, %d, %d, %d)\n", stVoipValue.ch_id, nRxRtpStatsCountByte[stVoipValue.ch_id], nRxRtpStatsCountPacket[stVoipValue.ch_id],
			// nRxRtpStatsLostPacket[stVoipValue.ch_id], nTxRtpStatsCountByte[stVoipValue.ch_id], nTxRtpStatsCountPacket[stVoipValue.ch_id]);

			copy_to_user(user, &stVoipRtpStatistics, sizeof(TstVoipRtpStatistics));

			break;
			
		/** FAX/MODEM PASS THROUGH **/
		case VOIP_MGR_FAX_MODEM_PASS_CFG:
    			copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
			stVoipCfg.enable = fax_modem_out(stVoipCfg.ch_id);
			copy_to_user(user, &stVoipCfg, sizeof(TstVoipCfg));
			if (stVoipCfg.enable != 0)
			{
				PRINT_MSG("Get FAX Event from FIFO, ch=%d, enable=%d\n", stVoipCfg.ch_id, stVoipCfg.enable);
			}
			break;

		case VOIP_MGR_ENERGY_DETECT:
#ifdef ENERGY_DET_PCM_IN
			copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
			save_flags(flags); cli();
			/* 
			energy_out return value : max is 0dBm. 
			If return value = -1, it means FIFO is empty.
			*/
			eng_det_flag[stVoipValue.ch_id] = stVoipValue.value; // 1: enable, 0: disable - energy detect
			stVoipValue.value2 = energy_out(stVoipValue.ch_id);
			restore_flags(flags);
      			copy_to_user(user, &stVoipValue, sizeof(TstVoipValue));
#endif
			break;
		
		case VOIP_MGR_SLIC_ONHOOK_ACTION:
			copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
			// Disable DTMF Detection Before Close ACMW Channel
			RtkAc49xApiSetDtmfDetection(stVoipCfg.ch_id, CONTROL__DISABLE);
			RtkAc49xApiOnhookAction(stVoipCfg.ch_id);
			break;
			
		case VOIP_MGR_SLIC_OFFHOOK_ACTION:
			copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
			RtkAc49xApiOffhookAction(stVoipCfg.ch_id);
			// Enable DTMF Detection After Open ACMW Channel
			RtkAc49xApiSetDtmfDetection(stVoipCfg.ch_id, CONTROL__ENABLE);
			break;

		case VOIP_MGR_SET_TONE_OF_COUNTRY:
			copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
			if ( DSPCODEC_COUNTRY_CUSTOME != stVoipValue.value)
			{
				RtkAc49xApiSetCountryTone(stVoipValue.value);
        		uint32 chid;
				for (chid=0; chid<SLIC_CH_NUM; chid++)
		            SLIC_Set_Impendance(chid, stVoipValue.value /*country*/, 0 /* impedance value: reserved */);
			}
			break;
			
		case VOIP_MGR_SET_TONE_OF_CUSTOMIZE:
			copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
			tone_idx = stVoipValue.value;
			break;
			
		case VOIP_MGR_SET_CUST_TONE_PARAM:
			copy_from_user(&stVoipToneCfg, (TstVoipToneCfg *)user, sizeof(TstVoipToneCfg));
			SaveCustomTone(&stVoipToneCfg);
			break;
			
		case VOIP_MGR_USE_CUST_TONE:
			copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
			RtkAc49xApiSetCustomTone(&stVoipValue);
			break;

		case VOIP_MGR_VOIP_RESOURCE_CHECK:
			copy_from_user(&stVoipResourceCheck, (TstVoipResourceCheck *)user, sizeof(TstVoipResourceCheck));
			
			int chid;
			stVoipResourceCheck.resource = VOIP_RESOURCE_UNAVAILABLE;
			
			for (chid=0; chid < ACMW_MAX_NUM_CH; chid++)
			{
				if (CHANNEL_STATE__IDLE == RtkAc49xGetChannelState(chid))
				{
					stVoipResourceCheck.resource = VOIP_RESOURCE_AVAILABLE;
				}			
			}
			
			if (stVoipResourceCheck._3way_session_info.enable != -1)
			{
				int mmid, isLoopBack[2] = {0};

				/* Check which mid session is LoopBack. Note: If src ip = dst ip, this session is LoopBack. */			
				for (mmid=0; mmid<=1; mmid++)
      				{
					if (stVoipResourceCheck._3way_session_info.rtp_cfg[mmid].ip_src_addr == stVoipResourceCheck._3way_session_info.rtp_cfg[mmid].ip_dst_addr)
					{
						isLoopBack[mmid] = 1;
						PRINT_MSG("isLoopBack[%d]=%d\n", mmid, isLoopBack[mmid]);
						stVoipResourceCheck.resource = VOIP_RESOURCE_AVAILABLE;
					}
				}
			}

			copy_to_user(user, &stVoipResourceCheck, sizeof(TstVoipResourceCheck));
			break;

#ifdef CONFIG_RTK_VOIP_IVR
		case VOIP_MGR_PLAY_IVR:
			/* Do copy_from_user() in PlayIvrDispatcher() */
			PlayIvrDispatcher( &stVoipPlayIVR_Header, user );
			copy_to_user(user, &stVoipPlayIVR_Header, sizeof(TstVoipPlayIVR_Header));
			break;
	
		case VOIP_MGR_POLL_IVR:
			copy_from_user(&stVoipPollIVR, (TstVoipPollIVR *)user, sizeof(TstVoipPollIVR));
			stVoipPollIVR.bPlaying =
				PollIvrPlaying( stVoipPollIVR.ch_id );
			copy_to_user(user, &stVoipPollIVR, sizeof(TstVoipPollIVR));
			break;
	
		case VOIP_MGR_STOP_IVR:
			copy_from_user(&stVoipStopIVR, (TstVoipStopIVR *)user, sizeof(TstVoipStopIVR));
			StopIvrPlaying( stVoipStopIVR.ch_id );
			break;
#endif/* CONFIG_RTK_VOIP_IVR */
			
		case VOIP_MGR_GET_T38_PCMIN:
			PRINT_MSG("VOIP_MGR_GET_T38_PCMIN: NOT support for ACMW\n");
			break;
	
		case VOIP_MGR_GET_T38_PACKETIN:
			PRINT_MSG("VOIP_MGR_GET_T38_PACKETIN: NOT support for ACMW\n");
			break;
						
		case VOIP_MGR_INIT_GNET:
		case VOIP_MGR_LOOPBACK:
		case VOIP_MGR_GNET:
		case VOIP_MGR_DSPSETCONFIG:		
		case VOIP_MGR_DSPCODECSTART:
		case VOIP_MGR_INIT_GNET2:
		case VOIP_MGR_SLIC_TONE:
		case VOIP_MGR_HOLD:
		case VOIP_MGR_CTRL_RTPSESSION:
		case VOIP_MGR_CTRL_TRANSESSION_ID:
		case VOIP_MGR_FAX_OFFHOOK:
		case VOIP_MGR_DEBUG:
		case VOIP_MGR_SET_CID_FSK_GEN_MODE:
	                //PRINT_MSG("This IO Ctrl is NOT support at AudioCodes solution.\n");
			break;



#endif /* AUDIOCODES_VOIP */  
 


	case VOIP_MGR_SIGNALTEST:
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
		kill_fasync(&( ((struct pcm_priv*)pcmctrl_devices)->pcm_fasync), SIGIO, POLL_IN);
#endif
		break;

	case VOIP_MGR_PCM_CFG:
		{
      		copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
                PRINT_MSG("VOIP_MGR_PCM_CFG:ch_id = %d, enable = %d\n", stVoipCfg.ch_id, stVoipCfg.enable);
		save_flags(flags); cli();
		if(stVoipCfg.enable ==1) //ON
		{ 
		    //pcm_enableChan(stVoipCfg.ch_id);
		    #ifdef _chiminer_slic2_ch1
		    stVoipCfg.ch_id=1;
                    PRINT_MSG("stVoipCfg.ch_id=%d\n",stVoipCfg.ch_id);
		    #endif
		    PCM_restart(stVoipCfg.ch_id);
		    pcm_flag[stVoipCfg.ch_id] = 1;
			SLIC_Set_PCM_state(stVoipCfg.ch_id, SLIC_PCM_ON);

		 }
		 else if(stVoipCfg.enable == 0)   //OFF
		 {
		    pcm_disableChan(stVoipCfg.ch_id);
		    pcm_flag[stVoipCfg.ch_id] = 0;
		SLIC_Set_PCM_state(stVoipCfg.ch_id, SLIC_PCM_OFF);
		  }
		restore_flags(flags);
		
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
		  /* pkshih: move them out of cli() */
                  PRINT_MSG("stVoipCfg.ch_id=%d\n",stVoipCfg.ch_id); //chiminer modify
                  PRINT_MSG("PCMCHCNR=%x\n",PCM_CHCNR);//chiminer modify
#endif
		}
		break;

	case VOIP_MGR_DTMF_GET:
      		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
		save_flags(flags); cli();
		dtmf_out(stVoipValue.ch_id, &stVoipValue.value);
		restore_flags(flags);
                //PRINT_MSG(" m:%c ", stVoipValue.value);
      		copy_to_user(user, &stVoipValue, sizeof(TstVoipValue));
		break;
		
	case VOIP_MGR_SLIC_RING:
      		copy_from_user(&stVoipSlicRing, (TstVoipSlicRing *)user, sizeof(TstVoipSlicRing));
                PRINT_MSG("VOIP_MGR_SLIC_RING:ch_id =%d, ring_set = %d\n", stVoipSlicRing.ch_id, stVoipSlicRing.ring_set);
#ifdef CONFIG_RTK_VOIP_LED
		if (stVoipSlicRing.ring_set == 1 && daa_ringing == 0)
			fxs_ringing[stVoipSlicRing.ch_id] = 1;
		if (stVoipSlicRing.ring_set == 0)
			fxs_ringing[stVoipSlicRing.ch_id] = 0;
		led_state_watcher(stVoipSlicRing.ch_id);
#endif

		FXS_Ring(&stVoipSlicRing);
		break;
		
	case VOIP_MGR_SLIC_RESTART:
      		copy_from_user(&stVoipSlicRestart, (TstVoipSlicRestart*)user, sizeof(TstVoipSlicRestart));
                PRINT_MSG("VOIP_MGR_SLIC_RESTART:ch_id:%d, codec_law=%d\n", stVoipSlicRestart.ch_id, stVoipSlicRestart.codec_law);
                save_flags(flags); cli();
		SLIC_reset(stVoipSlicRestart.ch_id, stVoipSlicRestart.codec_law, stVoipSlicRestart.ch_id+1);
		restore_flags(flags);
		break;

	case 	VOIP_MGR_GET_SLIC_REG_VAL:
		copy_from_user(&stSlicReg, (TstVoipSlicReg *)user, sizeof(TstVoipSlicReg));
		save_flags(flags); cli();
		SLIC_Choice(stSlicReg.ch_id + 1);
#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215)
       		stSlicReg.reg_val = readDirectReg(stSlicReg.reg_num);
#elif defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_W682388)
       		stSlicReg.reg_val = ReadReg(stSlicReg.reg_num);
#endif
		PRINT_MSG("VOIP_MGR_GET_SLIC_REG_VAL: ch%d read %d = %x\n", stSlicReg.ch_id, stSlicReg.reg_num, stSlicReg.reg_val);
        	restore_flags(flags);
		copy_to_user(user, &stSlicReg, sizeof(TstVoipSlicReg));
		break;
		
	case 	VOIP_MGR_SET_SLIC_REG_VAL:
		copy_from_user(&stSlicReg, (TstVoipSlicReg *)user, sizeof(TstVoipSlicReg));
		save_flags(flags); cli();
		SLIC_Choice(stSlicReg.ch_id + 1);
                PRINT_MSG("VOIP_MGR_SET_SLIC_REG_VAL: ch%d write %d = %x\n", stSlicReg.ch_id, stSlicReg.reg_num, stSlicReg.reg_val);
#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215)
        	writeDirectReg(stSlicReg.reg_num, stSlicReg.reg_val);
#elif defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_W682388)
        	WriteReg(stSlicReg.reg_num, stSlicReg.reg_val);
#endif
		restore_flags(flags);
		copy_to_user(user, &stSlicReg, sizeof(TstVoipSlicReg));
		break;

	case 	VOIP_MGR_SET_SLIC_TX_GAIN:
#ifdef SUPPORT_SLIC_GAIN_CFG
		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
		g_txVolumneGain[stVoipValue.ch_id] = stVoipValue.value;
		Set_SLIC_Tx_Gain(stVoipValue.ch_id, g_txVolumneGain[stVoipValue.ch_id]+1);
#endif
		break;
		
	case 	VOIP_MGR_SET_SLIC_RX_GAIN:
#ifdef SUPPORT_SLIC_GAIN_CFG
		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
		g_rxVolumneGain[stVoipValue.ch_id] = stVoipValue.value;
		Set_SLIC_Rx_Gain(stVoipValue.ch_id, g_rxVolumneGain[stVoipValue.ch_id]+1);
#endif
		break;
		
	case 	VOIP_MGR_SET_SLIC_RING_CADENCE:
		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
		SLIC_Set_Ring_Cadence(stVoipValue.ch_id, stVoipValue.value5 /* msec */, stVoipValue.value6 /* msec */);
		break;		
		
#ifdef CONFIG_RTK_VOIP_DRIVERS_DAA_SI3050
	case VOIP_MGR_DAA_RING:
		copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
		static unsigned int counter = 0, counter_1 = 0;;

	#ifndef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA
		/* 0:Not ring state  1:Ringing now  2:Line not connect */
		if (stVoipCfg.ch_id == 0)
			daa_ring = ring_detection_DAA();
		else if (stVoipCfg.ch_id == 1)
			daa_ring = ring_detection_DAA();
		else
			daa_ring = 0;
		/*
		   Because only pcm channel 0 is connected to DAA is implemented(2006-10-13),
		   when DAA incomimg ring is detected, we only ring the phone 0 (pcm channel 0).
		   Once pcm channel N connect to DAA is finished, it is needed to also ring
		   the phone N (pcm channel N).
		*/
	#else
		/* 0:Not ring state  1:Ringing now */
		if (stVoipCfg.ch_id == 0)
			daa_ring = virtual_daa_ring_det();
		else if (stVoipCfg.ch_id == 1)
#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA_2_RELAY_SUPPORT
			daa_ring = virtual_daa_ring_det();
#else
			daa_ring = 0;
#endif
		else
			daa_ring = 0;
		/*
		   Because V210 only have one Relay for phone 0 (pcm channel 0), when DAA
		   incomimg ring is detected, we only ring the phone 0 (pcm channel 0).
		   Once Relay for phone N is attached, it is needed to also ring
		   the phone N (pcm channel N).
		*/
	#endif
#ifdef CONFIG_RTK_VOIP_LED

		if ((daa_ring == 1) && (stVoipCfg.ch_id == 0))
			counter++;
		if ((daa_ring == 0) && (stVoipCfg.ch_id == 0))
			counter_1++;
		if ((daa_ring) == 1 && (counter == 2) && (stVoipCfg.ch_id == 0)) {
			counter = 0;
			pstn_ringing = 1;

		} else if ((daa_ring) == 0 && (counter_1 == 50) && (stVoipCfg.ch_id == 0))
		{
			counter_1 = 0;
			pstn_ringing = 0;

		}
		daa_ringing = daa_ring;
		led_state_watcher(0);
#endif

		stVoipCfg.enable = daa_ring;
		copy_to_user(user, &stVoipCfg, sizeof(TstVoipCfg));
		break;
		
	case VOIP_MGR_DAA_OFF_HOOK:
		copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
		pcm_flag[stVoipCfg.ch_id] = 0;   /* when daa off hook, pcm will be disable.*/

		/* 1: success, 0xff: line not connect or busy or not support */
		stVoipCfg.enable = voip_mgr_daa_off_hook(stVoipCfg.ch_id);
#ifdef CONFIG_RTK_VOIP_LED
		if (stVoipCfg.enable == 1)
			daa_hook_status = 1;
		else if (stVoipCfg.enable == 0xff)
			daa_hook_status = 0;
		led_state_watcher(0);
#endif
		copy_to_user(user, &stVoipCfg, sizeof(TstVoipCfg));
		break;		
		
	case VOIP_MGR_DAA_ON_HOOK:
		copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
		voip_mgr_daa_on_hook(stVoipCfg.ch_id);
#ifdef CONFIG_RTK_VOIP_LED
		if (stVoipCfg.ch_id == 0)
			daa_hook_status = 0;

		led_state_watcher(0);
#endif
#ifdef FXO_BUSY_TONE_DET
		busy_tone_det_init();
#endif
		break;
		
	case VOIP_MGR_SET_DAA_TX_GAIN:// thlin +
		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
	#ifndef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA
	#ifdef CONFIG_RTK_SLIC_DAA_NEGOTIATION
		g_txVolumneGain_DAA = stVoipValue.value;
	#endif
		DAA_Tx_Gain_Web(stVoipValue.value);
	#endif
		break;
		
	case VOIP_MGR_SET_DAA_RX_GAIN:// thlin +
		copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
	#ifndef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA
	#ifdef CONFIG_RTK_SLIC_DAA_NEGOTIATION
		g_rxVolumneGain_DAA = stVoipValue.value;
	#endif
		DAA_Rx_Gain_Web(stVoipValue.value);
	#endif
		break;
		
#endif /* CONFIG_RTK_VOIP_DRIVERS_DAA_SI3050 */

	case VOIP_MGR_SET_FLASH_HOOK_TIME:
		copy_from_user(&stHookTime, (TstVoipHook *)user, sizeof(TstVoipHook));
		#ifdef EVENT_POLLING_TIMER
		flash_hook_time = (stHookTime.flash_time)/10;
		flash_hook_min_time = (stHookTime.flash_time_min)/10;
		#else		
		#ifdef PCM_PERIOD
		flash_hook_time = (stHookTime.flash_time)/(10*PCM_PERIOD);
		flash_hook_min_time = (stHookTime.flash_time_min)/(10*PCM_PERIOD);
		#else
		flash_hook_time = (stHookTime.flash_time)/10;
		flash_hook_min_time = (stHookTime.flash_time_min)/10;
		#endif /*PCM_PERIOD*/
		#endif /*EVENT_POLLING_TIMER*/	
		break;

#ifdef CONFIG_RTK_VOIP_DRIVERS_8186V_ROUTER
	case VOIP_MGR_8305_SWITCH_VAL:
		copy_from_user(&switch_value, (TstVoipSwitch *)user, sizeof(TstVoipSwitch));
		save_flags(flags); cli();
		if (switch_value.read_write == 0)
			MII_read(switch_value.phy,switch_value.reg,0);
		else if (switch_value.read_write == 1)
			MII_write(switch_value.phy,switch_value.reg,switch_value.value,0);
#ifdef _8305SC_
		else if (switch_value.read_write == 2)
			rtl8305_init();
#endif
		else if (switch_value.read_write == 3)
			rtl8306SD_init();
		else if (switch_value.read_write == 4)
			rtl8306_debug();
		else if (switch_value.read_write == 5)
			rtl8306_reset();
#ifdef _8305SC_
		else if (switch_value.read_write == 6)
			rtl8305_wan_vlan(0x6ab,0x05,1);
#endif
		else if (switch_value.read_write == 7)
			rtl8306_wan_vlan(0x6ab,0x05,1);
#if 0
#ifdef _8305SC_
		else if (switch_value.read_write == 8)
			rtl8305_WAN_LAN_VLAN_TAG(0x6ab,0x7cd);
#endif
		else if (switch_value.read_write == 9)
			rtl8306_WAN_LAN_VLAN_TAG(0x6ab,0x7cd);
#endif			
#ifdef _8305SC_
		else if (switch_value.read_write == 10) {
			rtl8305_restore_switch();
			external_mode = 1;
		}
#endif
		else if (switch_value.read_write == 11) {
			rtl8306_restore_switch();
			external_mode = 1;
		}
		/************ For Testing ************/
		else if (switch_value.read_write == 12)
			print_pcm();
#ifdef CONFIG_RTK_SLIC_DAA_NEGOTIATION
		else if (switch_value.read_write == 13)
			going_off_hook();
		else if (switch_value.read_write == 14)
			on_hook_daa();
		else if (switch_value.read_write == 15)
		{
			DAA_ISR_FLOW[0] = 0;
                        PRINT_MSG("Set DAA_ISR_FLOW[0] = 0\n" );
		}
		else if (switch_value.read_write == 16)
		{
			DAA_ISR_FLOW[0] = 1;
                        PRINT_MSG("Set DAA_ISR_FLOW[0] = 1\n" );
		}
		else if (switch_value.read_write == 17)
		{
			DAA_ISR_FLOW[0] = 2;
                        PRINT_MSG("Set DAA_ISR_FLOW[0] = 2\n" );
		}
		else if (switch_value.read_write == 18)
		{
			DAA_ISR_FLOW[1] = 0;
                        PRINT_MSG("Set DAA_ISR_FLOW[1] = 0\n" );
		}
		else if (switch_value.read_write == 19)
		{
			DAA_ISR_FLOW[1] = 1;
                        PRINT_MSG("Set DAA_ISR_FLOW[1] = 1\n" );
		}
		else if (switch_value.read_write == 20)
		{
			DAA_ISR_FLOW[1] = 2;
                        PRINT_MSG("Set DAA_ISR_FLOW[1] = 2\n" );
		}
#endif
		/**************************************/
		else if (switch_value.read_write == 21)
			rtl8306_bandwidth_control(4000,1);
		else if (switch_value.read_write == 22)
			rtl8306_bandwidth_control(8000,0);
		else if (switch_value.read_write == 23)
			rtl8306_QoS_restore();
		else if (switch_value.read_write == 24)
			rtl8306_QoS_mechanism();
		else if (switch_value.read_write == 25) {
			//rtl8306_wan_multi_vlan(0x6ab,0x7cd);
			rtl8306_wan_2_vlan(
						1707, 5, 0,
						1997, 0, 0);
		} else if (switch_value.read_write == 26)
                        rtl8306_wan_3_vlan(
						1707, 7, 0,
						1997, 0, 0,
						2287, 5, 0);
		else if (switch_value.read_write == 27)
			rtl8306_setAsicQosPortRate(5,0x9c,1,1);
		else if (switch_value.read_write == 28)
			rtl8306_setAsicQosPortRate(5,0x9c,1,0);
		else if (switch_value.read_write == 29)
			rtl8306_setAsicQosPortRate(0,0x4e,0,1);
		else if (switch_value.read_write == 30)
			rtl8306_setAsicQosPortRate(0,0x4e,0,0);
		else if (switch_value.read_write == 31)
			rtl8306_vlan_dump();

		restore_flags(flags);
		break;
		
	case VOIP_MGR_WAN_VLAN_TAG:
		copy_from_user(&wan_vlan_tag, (TstVoipSwitch_VLAN_tag *)user, sizeof(TstVoipSwitch_VLAN_tag));
#if 1 // debug only
                PRINT_MSG("wan_vlan_tag.enable =%d\n", wan_vlan_tag.enable);
                PRINT_MSG("wan_vlan_tag.vlanId =%d\n", wan_vlan_tag.vlanId);
                PRINT_MSG("wan_vlan_tag.priority =%d\n", wan_vlan_tag.priority);
                PRINT_MSG("wan_vlan_tag.cfi =%d\n", wan_vlan_tag.cfi);
#endif
		save_flags(flags); cli();
		if (external_mode == 0) {
                        if (wan_vlan_tag.enable == 1)
			{
			#ifdef _8305SC_
                                rtl8305_wan_vlan(wan_vlan_tag.vlanId,wan_vlan_tag.priority,wan_vlan_tag.cfi);
			#endif
			#ifdef _8306SD_
                                rtl8306_wan_vlan(wan_vlan_tag.vlanId,wan_vlan_tag.priority,wan_vlan_tag.cfi);
			#endif
                        } else // if (wan_vlan_tag.enable == 2)
			{
			#ifdef _8305SC_
				rtl8305_init();
			#endif
			#ifdef _8306SD_
				rtl8306SD_init();
			#endif
			}
		}
		restore_flags(flags);
		break;
		

	case VOIP_MGR_WAN_2_VLAN_TAG:
		copy_from_user(&wan_2_vlan_tag, (TstVoipSwitch_2_VLAN_tag *)user, sizeof(TstVoipSwitch_2_VLAN_tag));
#if 1 // debug only
                PRINT_MSG("wan_2_vlan_tag.enable =%d\n", wan_2_vlan_tag.enable);
                PRINT_MSG("wan_2_vlan_tag.vlanIdVoice =%d\n", wan_2_vlan_tag.vlanIdVoice);
                PRINT_MSG("wan_2_vlan_tag.priorityVoice =%d\n", wan_2_vlan_tag.priorityVoice);
                PRINT_MSG("wan_2_vlan_tag.cfiVoice =%d\n", wan_2_vlan_tag.cfiVoice);
                PRINT_MSG("wan_2_vlan_tag.vlanIdData =%d\n", wan_2_vlan_tag.vlanIdData);
                PRINT_MSG("wan_2_vlan_tag.priorityData =%d\n", wan_2_vlan_tag.priorityData);
                PRINT_MSG("wan_2_vlan_tag.cfiData =%d\n", wan_2_vlan_tag.cfiData);
#endif
		save_flags(flags); cli();
		if (external_mode == 0) {
                        if (wan_2_vlan_tag.enable == 1)
			{
				rtl8306_wan_2_vlan(
					wan_2_vlan_tag.vlanIdVoice, wan_2_vlan_tag.priorityVoice, wan_2_vlan_tag.cfiVoice,
					wan_2_vlan_tag.vlanIdData, wan_2_vlan_tag.priorityData, wan_2_vlan_tag.cfiData
				);
                        } else // if (wan_2_vlan_tag.enable == 2)
			{
				rtl8306SD_init();
			}
		}
		restore_flags(flags);
		break;
		
	case VOIP_MGR_WAN_3_VLAN_TAG:
		copy_from_user(&wan_3_vlan_tag, (TstVoipSwitch_3_VLAN_tag *)user, sizeof(TstVoipSwitch_3_VLAN_tag));
                PRINT_MSG("wan_3_vlan_tag.enable =%d\n", wan_3_vlan_tag.enable);
                PRINT_MSG("wan_3_vlan_tag.vlanIdVoice =%d\n", wan_3_vlan_tag.vlanIdVoice);
                PRINT_MSG("wan_3_vlan_tag.priorityVoice =%d\n", wan_3_vlan_tag.priorityVoice);
                PRINT_MSG("wan_3_vlan_tag.cfiVoice =%d\n", wan_3_vlan_tag.cfiVoice);
                PRINT_MSG("wan_3_vlan_tag.vlanIdData =%d\n", wan_3_vlan_tag.vlanIdData);
                PRINT_MSG("wan_3_vlan_tag.priorityData =%d\n", wan_3_vlan_tag.priorityData);
                PRINT_MSG("wan_3_vlan_tag.cfiData =%d\n", wan_3_vlan_tag.cfiData);
                PRINT_MSG("wan_3_vlan_tag.vlanIdVideo =%d\n", wan_3_vlan_tag.vlanIdVideo);
                PRINT_MSG("wan_3_vlan_tag.priorityVideo =%d\n", wan_3_vlan_tag.priorityVideo);
                PRINT_MSG("wan_3_vlan_tag.cfiVideo =%d\n", wan_3_vlan_tag.cfiVideo);

		save_flags(flags); cli();
		if (external_mode == 0) {
                        if (wan_3_vlan_tag.enable == 1)
			{
				rtl8306_wan_3_vlan(
					wan_3_vlan_tag.vlanIdVoice, wan_3_vlan_tag.priorityVoice, wan_3_vlan_tag.cfiVoice,
					wan_3_vlan_tag.vlanIdData, wan_3_vlan_tag.priorityData, wan_3_vlan_tag.cfiData,
					wan_3_vlan_tag.vlanIdVideo, wan_3_vlan_tag.priorityVideo, wan_3_vlan_tag.cfiVideo
				);
                        } else 
			{
				rtl8306SD_init();
			}
		}
		restore_flags(flags);
		break;


	case VOIP_MGR_BRIDGE_MODE:
		copy_from_user(&bridge_mode, (unsigned char *)user, sizeof(unsigned char));
		save_flags(flags); cli();
		if (bridge_mode == 1)
		{
		#ifdef _8305SC_
			rtl8305_restore_switch();
		#endif
		#ifdef _8306SD_
			rtl8306_restore_switch();
		#endif
		} else if (bridge_mode == 2)
		{
		#ifdef _8305SC_
			rtl8305_init();
		#endif
		#ifdef _8306SD_
			rtl8306SD_init();
		#endif
		}
		restore_flags(flags);
		break;
#endif /*CONFIG_RTK_VOIP_DRIVERS_8186V_ROUTER*/

	case VOIP_MGR_SIP_REGISTER:
		copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

#ifdef CONFIG_RTK_VOIP_LED
		if ( sip_registed[stVoipCfg.ch_id] != stVoipCfg.enable )
		{
			sip_registed[stVoipCfg.ch_id] = stVoipCfg.enable;
			led_state_watcher(stVoipCfg.ch_id);
		}
#endif

#if 0
		save_flags(flags); cli();
		if (LED.device == 0)
			FXS_LED_STATE(LED.state);
		else if (LED.device == 1)
			FXO_LED_STATE(LED.state);
		else if (LED.device == 2)
			SIP_LED_STATE(LED.state);
		else if (LED.device == 3)
			FXS_ONE_LED_STATE(LED.state);
		restore_flags(flags);
#endif
		break;		
		
	case VOIP_MGR_GET_FEATURE:
		copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
		save_flags(flags); cli();
		stVoipCfg.cfg = RTK_VOIP_FEATURE;
		//gVoipFeature = RTK_VOIP_FEATURE; //move to rtk_voip_mgr_init_module
                PRINT_MSG("== VoIP Feature: 0x%x ==\n", RTK_VOIP_FEATURE);
		restore_flags(flags);
		copy_to_user(user, &stVoipCfg, sizeof(TstVoipCfg));
		break;

	case VOIP_MGR_GET_SLIC_STAT:
		copy_from_user(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
		save_flags(flags); cli();

		if(stVoipCfg.ch_id < 2)
		{
#if defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210 ) || defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215 )
			SLIC_Choice(stVoipCfg.ch_id + 1);	 //choice slic1 or slic2. //chiminer
			stVoipCfg.cfg = readDirectReg(68)&0x01; /* 1:off-hook  0:on-hook */
#elif defined CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
			stVoipCfg.cfg = iphone_hook_detect();	/* 1:off-hook  0:on-hook */
#elif defined CONFIG_RTK_VOIP_DRIVERS_SLIC_W682388
			stVoipCfg.cfg = GetHookState(stVoipCfg.ch_id);	/* 1:off-hook  0:on-hook */
#elif defined CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221
			Le88xxx data;
			readLegerityReg( 0x4F, &data);
			if (stVoipCfg.ch_id == 0)
				stVoipCfg.cfg = data.byte1&0x01;	/* 1:off-hook  0:on-hook */
			else if (stVoipCfg.ch_id == 1)
				stVoipCfg.cfg = data.byte2&0x01;	/* 1:off-hook  0:on-hook */
#endif
		}
		else
		{
#ifdef CONFIG_RTK_SLIC_DAA_NEGOTIATION
			stVoipCfg.cfg = readDAAReg(29);
#endif
		}

		restore_flags(flags);
		copy_to_user(user, &stVoipCfg, sizeof(TstVoipCfg));
		break;

	case VOIP_MGR_SET_GETDATA_MODE:
#ifdef RTK_VOICE_RECORD
		copy_from_user(&stVoipdataget_o, (TstVoipdataget_o *)user, sizeof(TstVoipdataget_o) - 1120);
		int temp_readindex,temp_writeindex;

		stVoipdataget[stVoipdataget_o.ch_id].write_enable=stVoipdataget_o.write_enable;

		if(!stVoipdataget_o.write_enable)
		{
			stVoipdataget[stVoipdataget_o.ch_id].tx_readindex=0;
			stVoipdataget[stVoipdataget_o.ch_id].tx_writeindex=0;
			stVoipdataget[stVoipdataget_o.ch_id].rx_readindex=0;
			stVoipdataget[stVoipdataget_o.ch_id].rx_writeindex=0;
			stVoipdataget[stVoipdataget_o.ch_id].rx_readindex2=0;
			stVoipdataget[stVoipdataget_o.ch_id].rx_writeindex2=0;
		}

		if(stVoipdataget_o.mode&0x1)//tx
		{
			temp_readindex=stVoipdataget[stVoipdataget_o.ch_id].tx_readindex;
			temp_writeindex=stVoipdataget[stVoipdataget_o.ch_id].tx_writeindex;

			if(   ((temp_writeindex>temp_readindex) && ((temp_writeindex-temp_readindex)>=1120))
			   || ((temp_writeindex<temp_readindex) && ((DATAGETBUFSIZE - temp_readindex + temp_writeindex)>=1120))  )
			{
				if(temp_writeindex>temp_readindex)
					stVoipdataget_o.length = temp_writeindex-temp_readindex;
				else//(temp_writeindex<temp_readindex)
					stVoipdataget_o.length = DATAGETBUFSIZE - temp_readindex + temp_writeindex;

				memcpy(stVoipdataget_o.buf,&stVoipdataget[stVoipdataget_o.ch_id].txbuf[temp_readindex],1120);
				temp_readindex=(temp_readindex+1120)%DATAGETBUFSIZE;
			}
			else
				stVoipdataget_o.length=0;

			stVoipdataget[stVoipdataget_o.ch_id].tx_readindex=temp_readindex;
		}
		else if(stVoipdataget_o.mode&0x2)	// rx2
		{
			temp_readindex=stVoipdataget[stVoipdataget_o.ch_id].rx_readindex2;
			temp_writeindex=stVoipdataget[stVoipdataget_o.ch_id].rx_writeindex2;

			if(   ((temp_writeindex>temp_readindex) && ((temp_writeindex-temp_readindex)>=1120))
			   || ((temp_writeindex<temp_readindex) && ((DATAGETBUFSIZE - temp_readindex + temp_writeindex)>=1120))  )
			{
				if(temp_writeindex>temp_readindex)
					stVoipdataget_o.length = temp_writeindex-temp_readindex;
				else//(temp_writeindex<temp_readindex)
					stVoipdataget_o.length = DATAGETBUFSIZE - temp_readindex + temp_writeindex;

				memcpy(stVoipdataget_o.buf,&stVoipdataget[stVoipdataget_o.ch_id].rxbuf2[temp_readindex],1120);
				temp_readindex=(temp_readindex+1120)%DATAGETBUFSIZE;
			}
			else
				stVoipdataget_o.length=0;

			stVoipdataget[stVoipdataget_o.ch_id].rx_readindex2=temp_readindex;
		}
		else
		{
			temp_readindex=stVoipdataget[stVoipdataget_o.ch_id].rx_readindex;
			temp_writeindex=stVoipdataget[stVoipdataget_o.ch_id].rx_writeindex;

			if(   ((temp_writeindex>temp_readindex) && ((temp_writeindex-temp_readindex)>=1120))
			   || ((temp_writeindex<temp_readindex) && ((DATAGETBUFSIZE - temp_readindex + temp_writeindex)>=1120))  )
			{
				if(temp_writeindex>temp_readindex)
					stVoipdataget_o.length = temp_writeindex-temp_readindex;
				else//(temp_writeindex<temp_readindex)
					stVoipdataget_o.length = DATAGETBUFSIZE - temp_readindex + temp_writeindex;

				memcpy(stVoipdataget_o.buf,&stVoipdataget[stVoipdataget_o.ch_id].rxbuf[temp_readindex],1120);
				temp_readindex=(temp_readindex+1120)%DATAGETBUFSIZE;
			}
			else
				stVoipdataget_o.length=0;

			stVoipdataget[stVoipdataget_o.ch_id].rx_readindex=temp_readindex;
		}

		if(stVoipdataget_o.length>=1120)
			copy_to_user(user, &stVoipdataget_o, sizeof(TstVoipdataget_o));
		else
			copy_to_user(user, &stVoipdataget_o, sizeof(TstVoipdataget_o)-1120+stVoipdataget_o.length);
#else
                PRINT_MSG("please define RTK_VOICE_RECORD in rtk_voip.h\n");
#endif //#ifdef RTK_VOICE_RECORD
		break;
		
#ifdef SUPPORT_DSCP
	case VOIP_MGR_SET_DSCP:
{
		int dscp;	
		int sip_dscp, rtp_dscp;
		copy_from_user(&dscp, (int *)user, sizeof(int));

		sip_dscp = (dscp & 0xFF00)>>8;
		rtp_dscp = (dscp & 0xFF);

		PRINT_MSG("\nsip_dscp = %x\n", sip_dscp);
		PRINT_MSG("\nrtp_dscp = %x\n", rtp_dscp);

#ifdef CONFIG_RTK_VOIP_DRIVERS_8186V_ROUTER
		rtl8306_set_DSCP(sip_dscp,rtp_dscp);
#endif
		// DSCP to TOS
		rtp_tos = dscp << 2;
		break;
}		
#endif


#ifdef CONFIG_RTK_VOIP_IP_PHONE
	case VOIP_MGR_CTL_KEYPAD:
		{
		extern void do_keypad_opt_ctl( void *pUser, unsigned int len );
		do_keypad_opt_ctl( user, len );
		}
		break;
	case VOIP_MGR_CTL_LCM:
		{
		extern void do_lcm_opt_ctl( void *pUser, unsigned int len );
		do_lcm_opt_ctl( user, len );
		}
		break;
	case VOIP_MGR_CTL_VOICE_PATH:
		{
		extern void AI_AO_select(unsigned char type);
		static const unsigned char select_type[] = 
			{ MIC1_SPEAKER, MIC2_MONO, SPEAKER_ONLY, MONO_ONLY };

		copy_from_user(&stVoicePath, (IPhone_test *)user, sizeof(TstVoicePath_t));
		
		AI_AO_select( select_type[ stVoicePath.vpath ] );
		}
		break;
#endif /* CONFIG_RTK_VOIP_IP_PHONE */
#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
	case VOIP_MGR_IPHONE_TEST:
		copy_from_user(&iphone, (IPhone_test *)user, sizeof(IPhone_test));
		save_flags(flags); cli();
		if (iphone.function_type == 0)
			write_WM8510(iphone.reg, iphone.value);
		else if (iphone.function_type == 1)
			WM8510_fake_read(iphone.reg);
		else if (iphone.function_type == 2)		
			Audio_interface_loopback_test(1);
		else if (iphone.function_type == 3)
			Audio_interface_loopback_test(0);	
		else if (iphone.function_type == 4)
			led_shower(iphone.reg);
		restore_flags(flags);
		break;
#endif //CONFIG_RTK_VOIP_DRIVERS_IP_PHONE		

	default:
                PRINT_MSG("IOCTL no command meet\n");
		break;
  }
  return 0;

	/* undef local variable share memory */
	#undef stVoipRtpStatistics
	#undef stVoipPlayIVR_Header	
	#undef stVoipPollIVR
	#undef stVoipStopIVR	
	#undef stVoipMgr3WayCfg		
	#undef stVoipMgrSession		
	#undef stVoipRtcpSession		
	#undef stVoipPayLoadTypeConfig			
	#undef stVoipTest			
	#undef stVoipRtpSessionState		
	#undef stVoipPlayToneConfig		
	#undef stVoipCfg			
	#undef stVoipValue			
	#undef stVoipSlicHook				
	#undef stVoipSlicRing				
	#undef stVoipSlicTone				
	#undef switch_value			
	#undef wan_vlan_tag			
	#undef wan_2_vlan_tag				
	#undef wan_3_vlan_tag				
	#undef stVoipSlicRestart		
	#undef stCIDstr			
	#undef stRFC2833			
	#undef stSlicReg			
	#undef stHookTime			
	#undef stVoipToneCfg			
	#undef stVoipResourceCheck		
}





#if ! defined (AUDIOCODES_VOIP)
int do_voip_mgr_get_ctl(struct sock *sk, int cmd, void *user, int *len)
{
  struct RTK_TRAP_profile *myptr;
  TstVoipMgrSession stVoipMgrSession;
  switch(cmd)
  {
	case VOIP_MGR_GET_SESSION:
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
                PRINT_MSG("GET SESSION\n");
      		copy_from_user(&stVoipMgrSession, (TstVoipMgrSession *)user, sizeof(TstVoipMgrSession));
		if( get_filter(stVoipMgrSession.ch_id, myptr)!=0 ) {
			stVoipMgrSession.result = -1;
      			copy_to_user(user, &stVoipMgrSession, sizeof(TstVoipMgrSession));
			break;
		}

        	stVoipMgrSession.ip_src_addr = myptr->ip_src_addr;
        	stVoipMgrSession.ip_dst_addr = myptr->ip_dst_addr;
        	stVoipMgrSession.udp_src_port= myptr->udp_src_port;
        	stVoipMgrSession.udp_dst_port= myptr->udp_dst_port;
        	stVoipMgrSession.protocol = myptr->protocol;
		stVoipMgrSession.result = 0;
      		copy_to_user(user, &stVoipMgrSession, sizeof(TstVoipMgrSession));
		break;
#else
                PRINT_MSG("Not defined in 865x platform\n");
#endif
	default:
		break;
  }
  return 0;
 }

#else /*AUDIOCODES_VOIP*/

int do_voip_mgr_get_ctl(struct sock *sk, int cmd, void *user, int *len)
{
 	switch (cmd)
  	{	

		default:
		break;
		
	}
	return 0;
}

#endif /*AUDIOCODES_VOIP*/


struct nf_sockopt_ops voip_mgr_sockopts = {
                { NULL, NULL }, PF_INET,
                VOIP_MGR_BASE_CTL, VOIP_MGR_SET_MAX+1, do_voip_mgr_set_ctl,
                VOIP_MGR_BASE_CTL, VOIP_MGR_GET_MAX+1, do_voip_mgr_get_ctl
};

