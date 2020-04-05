/*
 *	Realtek RTL8186 PCM Controller Driver
 *
 *	Author : thlin@realtek.com.tw
 *
 *	2005.09.05	
 *
 *	Copyright 2005 Realtek Semiconductor Corp.
 */
#include <linux/config.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/errno.h>	
#include <linux/types.h>	
#include <linux/delay.h>  	
#include <linux/interrupt.h>	
#include <linux/ioctl.h>
#include <asm/system.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
#define Virtual2Physical(x)             (((int)x) & 0x1fffffff)
#define Physical2Virtual(x)             (((int)x) | 0x80000000)
#define Virtual2NonCache(x)             (((int)x) | 0x20000000)
#define Physical2NonCache(x)            (((int)x) | 0xa0000000)
#define RTL865X_REG_BASE (0xBD010000)
#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8186
#include <asm/rtl8186.h>
#define EXTERNEL_CLK

#include "gpio.h"

#if RTL8186PV_RELAY_USE_Z  // For Z-version board 8186PV
#define RELAY_SW_CTRL_GPIOC	//for gpioC use. pull relay high. for Z-version board 1*FXS 1*FXO.
#else
#ifdef CONFIG_RTK_VOIP_DRIVERS_8186V_ROUTER
#define RELAY_SW_CTRL_GPIOD	//for gpioD used.pull relay high. for new 4-LAN EV board. 2006-07.
#else
#define RELAY_SW_CTRL_GPIOE	//for gpioE used.pull relay high.
#endif
#endif

#if 1 /* 0: relay for 8186V of Z version  1:relay for 8186V of v210 */
#define _V210_RELAY_R_ 
#else
#define _RELAY_8186V_Z_
#endif

#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8671
#include "gpio.h"
#include "../include/rtl8671.h"
#endif


#include "rtl_types.h"
#include "mem.h"
#include "rtk_voip.h"

#include "cp3_profile.h"

#include "pcm_interface.h"
#include "spi.h"
#include "si3210init.h"
#include "Slic_api.h"
#include "daa.h"
#ifndef AUDIOCODES_VOIP
#include "codec_def.h"
#include "codec_descriptor.h"
#include "../voip_dsp/include/dtmf_dec.h"

#ifdef FXO_CALLER_ID_DET
#include "fsk_det.h"
extern long cid_type[MAX_SLIC_CH_NUM];
#endif
#endif /*AUDIOCODES_VOIP*/

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
#include "voip_support.h"
#include "dsp_main.h"
voip_callback_t toRegister[MAX_SLIC_CH_NUM];
#endif

#ifdef 	CONFIG_RTK_VOIP_LED
#include "led.h"
#endif

#ifdef CONFIG_RTK_VOIP_IVR
#include "../voip_dsp/ivr/ivr.h"
#endif

#ifdef T38_STAND_ALONE_HANDLER
#include "t38_handler.h"
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8186) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM8671)
#include "./iphone/lcm.h"
#include "./iphone/WM8510.h"
#elif
#include "./iphone/base_gpio.h"
#include "./iphone/lcm.h"
#include "./iphone/WM8510.h"
#endif
#endif

#if defined (AUDIOCODES_VOIP)
#define __pcmImem	__attribute__ ((section(".speedup.text")))
#endif

#ifndef SUPPORT_CUT_DSP_PROCESS
extern Word16 nPcmIntPeriod[PCM_CH_NUM];            // pcm interrupt period (ms), set in INITIAL Cmd
#endif

//#define DISABLE_PCI	// kenny: temporary add. Shall be moved to somewhere, bootcode!?

int law = 2; // linear: 0 , a-law: 1, u-law: 2. It set the pcm controller and slic to desired mode.

char pcmctrl_time_slot[4]={0, 1, 2, 3};	//thlin+: Set time slot for each channel of the pcmctrl.

//#define AUDIOCODES_VOTING_MECHANISM

#if defined (AUDIOCODES_VOIP)
uint32 chanInfo_GetTranSessionID(uint32 chid) {return 2*chid;}

#define AC_PCM_ISR_CTLWRD_RESET_CH(ch) (0xFF000000 >> ((ch)*8))
#define AC_PCM_ISR_WRITE(b) pcm_outl(PCMISR, (b));

#define AC_PCM_IMR_READ() pcm_inl(PCMIMR)
#define AC_PCM_IMR_WRITE(b) pcm_outl(PCMIMR,(b))
#define AC_PCM_IMR_CTLWRD_P0OK(ch) BIT((4*(MAXCH-(ch)-1)+3))
#define AC_PCM_IMR_CTLWRD_P1OK(ch) BIT((4*(MAXCH-(ch)-1)+2))
#define AC_PCM_IMR_CTLWRD_TBUA(ch) BIT((4*(MAXCH-(ch)-1)+1))
#define AC_PCM_IMR_CTLWRD_RBUA(ch) BIT(4*(MAXCH-(ch)-1))

#define AC_PCM_CHCNR_READ() pcm_inl(PCMCHCNR)
#define AC_PCM_CHCNR_WRITE(b) pcm_outl(PCMCHCNR,(b))
#define AC_PCM_CHCNR_CTLWRD_RX_TX(ch) (BIT((8*(MAXCH-(ch)-1)+1)) | BIT(8*(MAXCH-(ch)-1)))
#define AC_PCM_CHCNR_CTLWRD_TX(ch) BIT((8*(MAXCH-(ch)-1)+1))
#define AC_PCM_CHCNR_CTLWRD_RX(ch) BIT(8*(MAXCH-(ch)-1))


#define __pcmIsr00	__attribute__ ((section(".text.pcmisr.page_1.0")))
#define __pcmIsr01	__attribute__ ((section(".text.pcmisr.page_1.1")))
#define __pcmIsr02	__attribute__ ((section(".text.pcmisr.page_1.2")))

 #define __pcmifdata00	__attribute__ ((section(".data.pcmif.page_0.0"))) 
 #define __pcmifdata01	__attribute__ ((section(".data.pcmif.page_0.1"))) 
 #define __pcmifdata02	__attribute__ ((section(".data.pcmif.page_0.2")))  
 #define __pcmifdata03	__attribute__ ((section(".data.pcmif.page_0.3"))) 
 #define __pcmifdata04	__attribute__ ((section(".data.pcmif.page_0.4"))) 
 #define __pcmifdata05	__attribute__ ((section(".data.pcmif.page_0.5"))) 
 #define __pcmifdata06	__attribute__ ((section(".data.pcmif.page_0.6"))) 
 #define __pcmifdata07	__attribute__ ((section(".data.pcmif.page_0.7"))) 
 #define __pcmifdata08	__attribute__ ((section(".data.pcmif.page_0.8"))) 
 #define __pcmifdata09	__attribute__ ((section(".data.pcmif.page_0.9"))) 
 #define __pcmifdata10	__attribute__ ((section(".data.pcmif.page_0.10"))) 
 #define __pcmifdata11	__attribute__ ((section(".data.pcmif.page_0.11"))) 
 #define __pcmifdata12	__attribute__ ((section(".data.pcmif.page_0.12"))) 
 #define __pcmifdata13	__attribute__ ((section(".data.pcmif.page_0.13"))) 
 #define __pcmifdata14	__attribute__ ((section(".data.pcmif.page_0.14")))  
 #define __pcmifdata15	__attribute__ ((section(".data.pcmif.page_0.15")))  
 #define __pcmifdata16	__attribute__ ((section(".data.pcmif.page_0.16")))
 #define __pcmifdata17	__attribute__ ((section(".data.pcmif.page_0.14")))  
 #define __pcmifdata18	__attribute__ ((section(".data.pcmif.page_0.18")))  
 #define __pcmifdata19	__attribute__ ((section(".data.pcmif.page_0.19")))
 #define __pcmifdata20	__attribute__ ((section(".data.pcmif.page_0.20")))

 #else
 
 #define __pcmIsr00
 #define __pcmIsr01
 #define __pcmIsr02

 #define __pcmifdata00
 #define __pcmifdata01
 #define __pcmifdata02
 #define __pcmifdata03
 #define __pcmifdata04
 #define __pcmifdata05
 #define __pcmifdata06
 #define __pcmifdata07
 #define __pcmifdata08
 #define __pcmifdata09
 #define __pcmifdata10
 #define __pcmifdata11
 #define __pcmifdata12
 #define __pcmifdata13
 #define __pcmifdata14
 #define __pcmifdata15
 #define __pcmifdata16
 #define __pcmifdata17
 #define __pcmifdata18
 #define __pcmifdata19
 #define __pcmifdata20
 
#endif



/**********************Select the Ti Codec or Si3210 or DAA supported ***********************************/

//#define _TI_Codec_

#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210) ||  defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215)
	#define _Si321x_
#endif
	
#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226)
	#define _LEGERITY_		
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_W682388
	#define _Winbond_
#endif	

/************************************************************************/
/**********If you want to split off_hook and on_hook for solar(voip_mgr_netfilter.c),****/
/**********you have to define macro SPLIT_DAA_ON_OFF_HOOK.*******************************/
#define SPLIT_DAA_ON_OFF_HOOK
/*********************************************/
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
#define DEV_NAME	"pcmctrl"
#define PCM_MAJOR	244	//use 0 for assigning major number dynamicly.
int pcmctrl_major = PCM_MAJOR;		//device major number
#endif
struct pcm_priv* pcmctrl_devices;
int slic_ch_num = SLIC_CH_NUM;		//nunmber of slic
int sess_num = SESS_NUM;
int rtcp_sid_offset = RTCP_SID_OFFSET;
//#define reg_isr
#ifdef reg_isr
static int(*isr_callback)(unsigned int status);
#endif

#ifdef PCM_HANDLER_USE_TASKLET
__pcmifdata20 struct tasklet_struct pcm_handler_tasklet;
#endif


__pcmifdata10 int txpage[MAX_SLIC_CH_NUM];
__pcmifdata09 int rxpage[MAX_SLIC_CH_NUM];
__pcmifdata16 int tr_cnt[MAX_SLIC_CH_NUM];
__pcmifdata17 char chanEnabled[MAX_SLIC_CH_NUM]; 

#if defined (AUDIOCODES_VOIP)
__pcmifdata05 unsigned long _chVoting = 0;
#define AC_PCM_VOTING_CH_ON(ch) _chVoting |= (1<<(ch))
#define AC_PCM_VOTING_CH_OFF(ch) _chVoting &= ~(1<<(ch))
#define AC_PCM_VOTING_GET() _chVoting
#endif

__pcmifdata06 static unsigned long ChanTxPage[8] = {CH0P0TOK, CH0P1TOK, CH1P0TOK, CH1P1TOK, CH2P0TOK, CH2P1TOK, CH3P0TOK, CH3P1TOK};
__pcmifdata07 static unsigned long ChanRxPage[8] = {CH0P0ROK, CH0P1ROK, CH1P0ROK, CH1P1ROK, CH2P0ROK, CH2P1ROK, CH3P0ROK, CH3P1ROK};

__pcmifdata08 uint32 **pRxBuf, **pTxBuf;
#ifdef SUPPORT_PCM_FIFO
__pcmifdata01 uint32 rx_fifo[MAX_SLIC_CH_NUM][PCM_FIFO_SIZE][RX_BUF_SIZE/(sizeof(uint32))];
__pcmifdata04 uint32 tx_fifo[MAX_SLIC_CH_NUM][PCM_FIFO_SIZE][TX_BUF_SIZE/(sizeof(uint32))];
__pcmifdata11 unsigned char tx_fifo_cnt_w[MAX_SLIC_CH_NUM]={0}, tx_fifo_cnt_r[MAX_SLIC_CH_NUM]={0};
__pcmifdata12 unsigned char rx_fifo_cnt_w[MAX_SLIC_CH_NUM]={0}, rx_fifo_cnt_r[MAX_SLIC_CH_NUM]={0};
__pcmifdata13 int fifo_start[MAX_SLIC_CH_NUM];
#define NEXT_FIFO_ENTRY(x) ( (x+1)==PCM_FIFO_SIZE? 0: (x+1) )
#define PREV_FIFO_ENTRY(x) ( (x)==0? (PCM_FIFO_SIZE-1): (x-1) )
#endif

#ifdef REDUCE_PCM_FIFO_MEMCPY
__pcmifdata14 uint32* pRxBufTmp; // For reducing memcpy
__pcmifdata15 uint32* pTxBufTmp;
#else
__pcmifdata02 uint32 RxBufTmp[MAX_SLIC_CH_NUM][RX_BUF_SIZE/4];
__pcmifdata03 uint32 TxBufTmp[MAX_SLIC_CH_NUM][TX_BUF_SIZE/4];
#endif

__pcmifdata18 uint32 rxlayerbuf[MAX_SLIC_CH_NUM];
__pcmifdata19 uint32 txlayerbuf[MAX_SLIC_CH_NUM];

int tx_mute[PCM_CH_NUM] = {0};
int rx_mute[PCM_CH_NUM] = {0};


extern int32 PCM_handler(unsigned int chid);

/****************** DTMF DET & REMOVAL RELATED DEFINE *************************/
#ifdef DTMF_DEC_ISR
int printk_flag = 1;	// declare for check pagesize before DTMF Det.
extern unsigned char dtmf_chid[];
#ifdef SUPPORT_SLIC_GAIN_CFG
uint32 g_txVolumneGain[MAX_SLIC_CH_NUM];
uint32 g_rxVolumneGain[MAX_SLIC_CH_NUM];
#endif
Word16 det_buff[MAX_SLIC_CH_NUM][RX_BUF_SIZE/2];  // Word16
Dtmf_det_out dtmf_digit;
#endif
extern int16 tx_comp[];

#ifdef DAA_RX_DET
Word16 det_buff_daa[RX_BUF_SIZE/2] __attribute__((aligned(8)));
#endif


#ifdef DTMF_REMOVAL_ISR
unsigned char dtmf_removal[MAX_SLIC_CH_NUM] = {0};// declare for dtmf removal, 0: disable  1: enable (value is assigned to 1 when dtmf tone is detected, otherwise, 0!)
unsigned char dtmf_removal_flag[MAX_SLIC_CH_NUM] = {0};
unsigned char dtmf_removal_cnt[4] = {3, 3, 3 ,3};
extern char dtmf_mode[]; /* 0:rfc2833  1: sip info  2: inband  */
int clean_forward[4] = {1, 1, 1, 1};
#endif
extern unsigned char remote_is_rfc2833[]; /* 0: remote is not rfc2833, don't do dtmf removal, else is rfc2833, do dtmf removal */

#ifdef SUPPORT_FAX_PASS_ISR
extern unsigned char fax_offhook[]; 
extern int ced_check[];
extern char FaxFlag[];
extern char ModemFlag[];
extern void modem_det(unsigned int chid, unsigned short* page_addr);
extern void DisableDspFunctionsIfModemOrFaxIsDetected( int sid );
#endif

#ifdef SUPPORT_AES_ISR
static int aes_do_times[MAX_SLIC_CH_NUM] = {0}, aes_do_cnt[MAX_SLIC_CH_NUM] = {0};
static int aes_tx_fifo_size[MAX_SLIC_CH_NUM] = {0};
extern unsigned char support_AES[];
#endif

#ifndef AUDIOCODES_VOIP
extern const codec_type_desc_t *ppStartCodecTypeDesc[MAX_SESS_NUM];
#endif

#ifdef SUPPORT_LEC_G168_ISR
#define FSIZE 4
__pcmifdata00 Word16 LEC_RinBuf[MAX_SLIC_CH_NUM][80*PCM_PERIOD*FSIZE];   /* buf size = 80*4*PCM_PERIOD(ex: 2) samples = 80ms */  
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
#define SYNC_POINT 3					/* sync point(8651B GNET EVB) =  3 pages + 8 samples ~ 60ms ago */
#define SYNC_SAMPLE 8
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
#define SYNC_POINT 3					/* sync point(8186 V210, V110, V100) =  3 pages + 8 samples ~ 40ms ago */
#define SYNC_SAMPLE 8
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8671
#define SYNC_POINT 3
#define SYNC_SAMPLE 8
#endif
#ifdef LEC_USE_CIRC_BUF
Word16 LEC_Rin_CircBuf[MAX_SLIC_CH_NUM][80*PCM_PERIOD*FSIZE+SYNC_SAMPLE];/*using circular buffer, size = 80*4*PCM_PERIOD(ex: 2) samples = 80ms*/ /* add SYNC_SAMPLE to write data */
Word16 LEC_buf_windex[MAX_SLIC_CH_NUM];                                  /* circular buffer write index */
Word16 LEC_buf_rindex[MAX_SLIC_CH_NUM];                                  /* circular buffer read index */
#endif   //end of LEC_USE_CIRC_BUF
extern void LEC_g168(char chid, Word16 *pRin, Word16 *pSin, Word16 *pEx);
#include "lec.h"
#endif

#ifdef SUPPORT_AES_FOR_DAA
Word16 AES_RinBuf[80*PCM_PERIOD];
extern void AES(uint32 chid, Word16 *pEB, Word16 *pDB);
#endif

extern unsigned char support_lec_g168[] ;	// 0: LEC disable  1: LEC enable

#ifdef LEC_G168_ISR_SYNC_P
short vpat[16]={32767,30272,23170,12539,0,-12539,-23170,-30272,-32767,-30272,-23170,-12539, 0,12539,23170,30272 };
char sync = 0;
static int cnt_time=0, sync_frame=0, sync_frame_R=0, sync_start=0;
int level = 2000;
#endif

#ifdef SEND_RFC2833_ISR
#include "dsp_main.h"
extern unsigned char RtpOpen[];
extern int RtpTx_transmitEvent_ISR( uint32 chid, uint32 sid, int event);
int sned_flag[MAX_SLIC_CH_NUM] = {0};
int g_digit[MAX_SLIC_CH_NUM]={0};
#endif

#ifdef CONFIG_RTK_SLIC_DAA_NEGOTIATION
unsigned int *pTxBuf_DAA, *pRxBuf_DAA;
unsigned int *ptxtmp_DAA, *prxtmp_DAA;
//unsigned short int Tx_FIFO_DAA[PCM_FIFO_SIZE*PCM_SAMPLE_DAA],Rx_FIFO_DAA[PCM_FIFO_SIZE*PCM_SAMPLE_DAA];
unsigned char txpage_DAA=0, rxpage_DAA=0;
uint32 tx_fifo_DAA[PCM_FIFO_SIZE][TX_BUF_SIZE/(sizeof(uint32))];
uint32 rx_fifo_DAA[PCM_FIFO_SIZE][RX_BUF_SIZE/(sizeof(uint32))];
unsigned char tx_fifo_cnt_w_DAA = 0, tx_fifo_cnt_r_DAA = 0;
unsigned char rx_fifo_cnt_w_DAA = 0, rx_fifo_cnt_r_DAA = 0;
int fifo_start_DAA;
uint32* pRxBufTmp_DAA; // For reducing memcpy
uint32* pTxBufTmp_DAA;
unsigned int DAA_ISR_FLOW[MAX_SLIC_CH_NUM] = {0}; //0: default flow, 1: 3-way conference(PSTN), 2: call forward(VoIP<->PSTN)
uint32 g_txVolumneGain_DAA = 7; // init value: 0dB
uint32 g_rxVolumneGain_DAA = 7; // init value: 0dB
#endif

#if defined (AUDIOCODES_VOIP)
/*** Play Hold Tone for DAA Tx Path ***/
unsigned int ToneBuf_DAA[TX_BUF_SIZE/4]={0};
#else
extern unsigned int ToneBuf_DAA[];
#endif

int g_DAA_used[MAX_SLIC_CH_NUM] = {0};   /* DAA is 0: NOT used, 1: used by SLIC N(use channel n), 2: used and held. */

#ifdef FXO_BUSY_TONE_DET
#include "tone_det_i.h"
#endif

#if 0
#ifdef RTK_VOICE_RECORD
#include "type.h"
#include "../voip_dsp/dsp_r1/include/typedef.h"
#include "voip_control.h"
extern TstVoipdataget stVoipdataget[];
#endif //#ifdef RTK_VOICE_RECORD
#endif

#ifdef RTK_VOICE_RECORD
#include "type.h"
#include "../voip_dsp/dsp_r1/include/typedef.h"
#include "voip_control.h"
char txdatabuf[DATAGETBUFSIZE];
char rxdatabuf[DATAGETBUFSIZE];
//char txdatabuf2[DATAGETBUFSIZE];
char rxdatabuf2[DATAGETBUFSIZE];
TstVoipdataget stVoipdataget[4]={{0,0,0,0,0,0,0,txdatabuf, rxdatabuf, rxdatabuf2},
				 {0,0,0,0,0,0,0,txdatabuf, rxdatabuf, rxdatabuf2},
				 {0,0,0,0,0,0,0,txdatabuf, rxdatabuf, rxdatabuf2},
				 {0,0,0,0,0,0,0,txdatabuf, rxdatabuf, rxdatabuf2} };
#endif //#ifdef RTK_VOICE_RECORD


extern long voice_gain_spk[];//0 is mute, 1 is -31dB ~~~ 32 is 0dB , 33 is 1dB ~~~ 63 is 31dB

//========================================================//

int pcm_set_page_size(unsigned int chid, unsigned int size)  
{
	/* Write the reg PCMBSIZE to set pagesize. */
	
	unsigned int n_size;
	n_size = (size/4 - 1);
	pcm_outl(PCMBSIZE, pcm_inl(PCMBSIZE) & (~(0xFF << (8 * (MAXCH-chid-1)))));//set to zero
	pcm_outl(PCMBSIZE, pcm_inl(PCMBSIZE)|(n_size << (8 * (MAXCH-chid-1))));	//set pagesize
	
	//PDBUG("set channel %d page size = %d\n", chid, size);
	// too many console message will cause R0, T0
	//printk("set channel %d page size = %d\n", chid, size);
	return 0;	
}

#ifndef OPTIMIZATION
unsigned int pcm_get_page_size(unsigned int chid)
{
	/* Read the reg PCMBSIZE to get pagesize*/
	unsigned int pagesize, n_size;	/* Actual pagesize which can get from "pcm_get_page_size()". 
 					It's different from the PCMPAGE_SIZE define in header file. */
	
	n_size = ((pcm_inl(PCMBSIZE) >>( 8 * (MAXCH-chid-1)))) & 0xFF;
	pagesize = 4*(n_size + 1);

//	PDBUG("get channel %d page size = %d\n", chid, pagesize);

	return pagesize;
}
#endif

/* Set Tx, Rx own bit to PCM Controller. */
#if defined (AUDIOCODES_VOIP)
static inline
#endif
void pcm_set_tx_own_bit(unsigned int chid, unsigned int pageindex)
{
	//printk("tx:%d\n", pageindex);
	pcm_outl(TX_BSA(chid), pcm_inl(TX_BSA(chid))|BIT(pageindex));
	//printk("CH0TXBSA= 0x%x\n", pcm_inl(CH0TXBSA));
	//PDBUG("set chid %d tx own bit %d to HW \n",chid, pageindex );
}

#if defined (AUDIOCODES_VOIP)
static inline
#endif
void pcm_set_rx_own_bit(unsigned int chid, unsigned int pageindex)
{
	//printk("rx:%d\n", pageindex);
	pcm_outl(RX_BSA(chid), pcm_inl(RX_BSA(chid))|BIT(pageindex));
	//printk("CH0RXBSA= 0x%x\n", pcm_inl(CH0RXBSA));
	//PDBUG("set chid %d rx own bit %d to HW\n",chid, pageindex );
}

/* clean interrupt pending bits */
void pcm_clean_isr(unsigned int statusval)
{
	pcm_outl(PCMISR, statusval);
	//PDBUG("clean pending status bits.\n");
}


/* Get the Tx, Rx base address */
unsigned int pcm_get_tx_base_addr(unsigned int chid)
{
	unsigned int txbaseaddr;
 	txbaseaddr = (pcm_inl(TX_BSA(chid)) & 0xFFFFFFFC)|0xA0000000; // to virtual non-cached address
	
	PDBUG(" get Tx base addresss = 0x%x\n", txbaseaddr);
	return txbaseaddr;
	
}


unsigned int pcm_get_rx_base_addr(unsigned int chid)
{
	unsigned int rxbaseaddr;
 	rxbaseaddr = (pcm_inl(RX_BSA(chid)) & 0xFFFFFFFC)|0xA0000000; // to virtual non-cached address
 	PDBUG(" get Rx base addresss = 0x%x\n", rxbaseaddr);
	return rxbaseaddr;
}


void pcm_enable(void)
{
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651_T	
	/* config LSB of GPIOB as PCM */
	pcm_outl(0xbd01200c, pcm_inl(0xbd01200c) | 0x000f0000UL);
	pcm_outl(0xbd012054, pcm_inl(0xbd012054) & (~0x000f0000UL));
	/*config MII as GPIOD/E*/
	pcm_outl(0xbc803018, pcm_inl(0xbc803018) | 0x30000000UL);
	/* config GPIOD/E */
	pcm_outl(0xbd012070, pcm_inl(0xbd012070) & 0x0000ffffUL);
#else
	/* config GPIO as PCM */
	//REG32(PABCCNR) |= 0x00ff0000UL;		// configure as dedicated peripheral
	pcm_outl(0xbd01200c, pcm_inl(0xbd01200c) | 0x00ff0000UL);
	//REG32(PABCPTCR) &= ~0x00ff0000UL;
	pcm_outl(0xbd012054, pcm_inl(0xbd012054) & (~0x00ff0000UL));
#if 0
	//REG32(0x203c) = 0xa5000000;
	pcm_outl(0xbd01203c, 0xa5000000);/*Watch dog disable*/
#endif	

#endif	//endif CONFIG_RTK_VOIP_DRIVERS_PCM8651_T
#endif	
	pcm_outl(PCMCR, PCM_ENABLE);		
	pcm_outl(PCMCR, 0);
	pcm_outl(PCMCR, PCM_ENABLE);		
	PDBUG("Enable PCM interface. PCMCR(%X) = 0x%X\n",PCMCR, pcm_inl(PCMCR));
}

void pcm_disable(void)
{		
	pcm_outl(PCMCR, 0);
	PDBUG("Disable PCM interface. PCMCR = 0x%X\n", pcm_inl(PCMCR));
}


void pcm_tx_rx_enable(unsigned int chid)
{
	pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR)|(BIT((8*(MAXCH-chid-1)+1)) | BIT(8*(MAXCH-chid-1))));	
}


void pcm_tx_enable(unsigned int chid)
{
	pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR)|BIT((8*(MAXCH-chid-1)+1)));
//	kenny: print induces multiple TOKs/ROKs
//	PDBUG("enable channel %d Tx\n", chid);
}

void pcm_tx_disable(unsigned int chid)
{
	pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR) & (~ BIT((8*(MAXCH-chid-1)+1))));
	//PDBUG("disable channel %d Tx\n", chid);
}

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
// disable WLAN Analog for lower temperature
void wlan_analog_disable(void)
{
	*(unsigned char *)0xbd400050 = 0xc0;
	*(unsigned char *)0xbd400059 = 0x44;
	*(unsigned int *)0xbd400054 = 0xa00fea59;
	PDBUG("disable wlan analog:(0xbd400054:%X) \n", *(unsigned int *)0xbd400054);
}
#endif

void pcm_rx_enable(unsigned int chid)
{
	pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR)|BIT(8*(MAXCH-chid-1)));
//	kenny: print induces multiple TOKs/ROKs
//	PDBUG("enable channel %d Rx\n", chid);
}

void pcm_rx_disable(unsigned int chid)
{
	pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR) & (~ BIT(8*(MAXCH-chid-1))));
	//PDBUG("disable channel %d Rx\n", chid);
}

void pcm_isr_reset(unsigned int chid)
{
	//printk("1 PCMISR= 0x%x\n", pcm_inl(PCMISR));	
	pcm_outl(PCMISR, 0xFF000000 >> (chid*8));	
	//printk("2 PCMISR= 0x%x\n", pcm_inl(PCMISR));	
}

void pcm_imr_enable(unsigned int chid, unsigned char type)
{
	//PDBUG("enable channel %d IMR\n", chid);
	switch(type)
	{
		case P0OK:
			pcm_outl(PCMIMR, pcm_inl(PCMIMR)|BIT((4*(MAXCH-chid-1)+3)));
			//PDBUG("enable channel %d IMR P0OK\n", chid);
			break;

		case P1OK:
			pcm_outl(PCMIMR, pcm_inl(PCMIMR)|BIT((4*(MAXCH-chid-1)+2)));
			//PDBUG("enable channel %d IMR P1OK\n", chid);
			break;

		case TBUA:
			pcm_outl(PCMIMR, pcm_inl(PCMIMR)|BIT((4*(MAXCH-chid-1)+1)));
			//PDBUG("enable channel %d IMR TUUA\n", chid);
			break;

		case RBUA:
			pcm_outl(PCMIMR, pcm_inl(PCMIMR)|BIT(4*(MAXCH-chid-1)));
			//PDBUG("enable channel %d IMR RBUA\n", chid);
			break;

		default:
			printk("enable channel %d IMR type error!\n", chid);
			break;
			
	}
	
//	print_pcm();
	
}

void pcm_imr_disable(unsigned int chid, unsigned char type)
{
	//PDBUG("disable channel %d IMR\n", chid);
	switch(type)
	{
		case P0OK:
			pcm_outl(PCMIMR, pcm_inl(PCMIMR) & (~ BIT((4*(MAXCH-chid-1)+3))));
			//PDBUG("disable channel %d IMR P0OK\n", chid);
			break;

		case P1OK:
			pcm_outl(PCMIMR, pcm_inl(PCMIMR) & (~ BIT((4*(MAXCH-chid-1)+2))));
			//PDBUG("disable channel %d IMR P0OK\n", chid);
			break;

		case TBUA:
			pcm_outl(PCMIMR, pcm_inl(PCMIMR) & (~ BIT((4*(MAXCH-chid-1)+1))));
			//PDBUG("disable channel %d IMR P0OK\n", chid);
			break;

		case RBUA:
			pcm_outl(PCMIMR, pcm_inl(PCMIMR) & (~ BIT(4*(MAXCH-chid-1))));
			//PDBUG("disable channel %d IMR P0OK\n", chid);
			break;

		default:
			printk("disable channel %d IMR type error!\n", chid);
			break;
			
	}
	
}


static void EnaPcmIntr(uint32 chid)
{
	//pcm_imr_enable(chid, P0OK | P1OK | TBUA | RBUA);
	pcm_isr_reset(chid);
	pcm_imr_enable(chid, P0OK);
	pcm_imr_enable(chid, P1OK );
	pcm_imr_enable(chid, TBUA );
	pcm_imr_enable(chid, RBUA );
}

static void DisPcmIntr(uint32 chid)
{
	//pcm_imr_disable(chid, P0OK | P1OK | TBUA | RBUA);
	pcm_imr_disable(chid, P0OK);
	pcm_imr_disable(chid, P1OK );
	pcm_imr_disable(chid, TBUA );
	pcm_imr_disable(chid, RBUA );
	pcm_isr_reset(chid);
}

#ifdef SUPPORT_PCM_FIFO
static void PCM_FIFO_Init(unsigned int chid)
{	
	tx_fifo_cnt_w[chid]=0; 
	tx_fifo_cnt_r[chid]=0;
#if DTMF_REMOVAL_FORWARD_SIZE == 3 && PCM_PERIOD == 2
	/* If odd removal length, one is given as initial value. */
	if (dtmf_removal_flag[chid] == 1)
		rx_fifo_cnt_w[chid]=1; 
	else
		rx_fifo_cnt_w[chid]=0; 
#elif PCM_PERIOD == 1
	rx_fifo_cnt_w[chid]=0; 
#else
	/* If even removal length, zero is admited. */
	//???
	rx_fifo_cnt_w[chid]=0; 
#endif
	rx_fifo_cnt_r[chid]=0;
	fifo_start[chid]=0;
#ifdef CONFIG_RTK_SLIC_DAA_NEGOTIATION
	
	tx_fifo_cnt_w_DAA=0;
	tx_fifo_cnt_r_DAA=0;
	rx_fifo_cnt_w_DAA=0;
	rx_fifo_cnt_r_DAA=0;
	
#endif	
}
#endif


static void init_var(void)
{
	int i;

	for(i=0; i<SLIC_CH_NUM; i++)
	{
		chanEnabled[i] = FALSE;
		
		Init_Hook_Polling(i);

#ifdef SUPPORT_PCM_FIFO
		PCM_FIFO_Init(i);
#endif	

		rxlayerbuf[i] = (uint32)((uint32)pcm_get_rx_base_addr(i));
		txlayerbuf[i] = (uint32)((uint32)pcm_get_tx_base_addr(i));

		txpage[i] = 0;
		rxpage[i] = 0;
		tr_cnt[i] = 0;

#ifdef CONFIG_RTK_VOIP_IVR
		InitializeIVR( i );
#endif
	}



	pRxBuf = (uint32**)((uint32)&rxlayerbuf[0]);
	pTxBuf = (uint32**)((uint32)&txlayerbuf[0]);

	BOOT_MSG("pTxBuf[0]=%p\n", pTxBuf[0]);
	BOOT_MSG("pRxBuf[0]=%p\n", pRxBuf[0]);
	BOOT_MSG("pTxBuf[1]=%p\n", pTxBuf[1]);
	BOOT_MSG("pRxBuf[1]=%p\n", pRxBuf[1]);
	
	ProfileInit();
}


void PCM_recovery(unsigned int chid, unsigned int *pcm_isr, unsigned int *tr_cnt, unsigned int *txpage,  unsigned int *rxpage) {
	//printk(" P_RECOVERY ");
	printk(".");
	*pcm_isr = 0;
	*tr_cnt = 0;
	*txpage = 0;
	*rxpage = 0;
#if 1
	pcm_disableChan(chid);
	PCM_restart(chid);
#else
	unsigned int pcmimr;
	
	pcmimr = pcm_inl(PCMIMR);
	pcm_outl(PCMCHCNR,pcm_inl(PCMCHCNR)&(~(0x03000000>>(chid*8))));
	pcm_outl(PCMIMR,0x0);
	pcm_clean_isr((0xFF000000>>(chid*8)));
	
	pcm_set_tx_own_bit(chid,0);
	pcm_set_tx_own_bit(chid,1);
	pcm_set_rx_own_bit(chid,0);
	pcm_set_rx_own_bit(chid,1);
	pcm_outl(PCMCHCNR,pcm_inl(PCMCHCNR)|(0x03000000>>(chid*8)));	
	pcm_outl(PCMIMR,pcmimr);

	return;	
#endif	
}


void reset_pcmsp(uint32 chid)
{
	int j;
	j = pcm_get_page_size(chid);
	memset(pRxBuf[chid], 0, j);
	memset(pTxBuf[chid], 0, j);
}

#if DMEN_LEC || DMEN_STACK_LEC
extern __lec_dmem_start;
#endif

#if !defined (AUDIOCODES_VOIP)
int32 PCM_RX(uint32 chid)
{
	int i, ii, j, SessNum;
	uint32 rx_rp, ssid;
	unsigned int stmp;
	Word16 s1, s2;
#ifndef OPTIMIZATION
#ifdef SUPPORT_FAX_PASS_ISR	
	uint32 sid;
	sid = chanInfo_GetTranSessionID(chid);
	if(sid == SESSION_NULL)
		sid = chid;
#endif
#endif
	uint32 *pRxFifoCurWriting;

	save_radiax_reg(); /* save radiax register value */

	
#ifdef PCM_PERIOD
	rx_rp = rxpage[chid] * (40*PCM_PERIOD);	//count page0/page1 offset
#else		
#ifdef SUPPORT_CUT_DSP_PROCESS
	rx_rp = rxpage[chid] * 40;	//count page0/page1 offset

#else
	rx_rp = rxpage[chid] * (nPcmIntPeriod[chid] << 2);	//count page0/page1 offset
#endif
#endif

#ifdef SUPPORT_PCM_FIFO //==========================================================//

#ifdef SUPPORT_LEC_G168_ISR
#if DMEN_LEC || DMEN_STACK_LEC
	set_DMEM(&__lec_dmem_start);
#endif
#if DMEN_LEC
	LEC_dmem_load(chid);	
#endif

#endif   	  

#ifdef SUPPORT_AES_ISR

	aes_do_cnt[chid] = 0;
	aes_tx_fifo_size[chid] = pcm_get_tx_PCM_FIFO_SIZE(chid);
	
	if (aes_tx_fifo_size[chid] < PCM_PERIOD)
		aes_do_times[chid] = aes_tx_fifo_size[chid];
	else
		aes_do_times[chid] = PCM_PERIOD;
	
	//printk("%d \n", aes_do_times[chid]);
#endif
	  
   	for (i=0; i < PCM_PERIOD; i++) 
	{
		
		/*** Check if Rx FIFO full or not ***/
		if (((rx_fifo_cnt_w[chid]+1)%PCM_FIFO_SIZE) == rx_fifo_cnt_r[chid]) 
		{
			// Full
			printk("RF(%d) ", chid);
			break;
		}

		
	#ifdef LEC_G168_ISR_SYNC_P
    	
    		if (sync == 0)
    		{
	    		for (ii=0; ii< 40; ii++)
    			{
    				stmp = *(&pRxBuf[chid][rx_rp]+(i*PCM_PERIOD_10MS_SIZE>>2)+ii);
    			
    				s1 = (short)((stmp >> 16) & 0x0000ffff);
				s2 = (short)((stmp & 0x0000ffff));
			
				//printk("%d ", s1);
			
				if (cnt_time > 100 && sync != 1)
				{	
					
					if ( s1 > level || s2 >level || s1 < -level || s2 < -level )
					{
						sync =1;
						printk("****** sync! sync_frame=%d, sync_frame_R=%d, i=%d, ii=%d ******\n", sync_frame, sync_frame_R, i, ii);
						
					}
				}
			
				if (sync ==1)
				{
					printk("%d\n", s1);
					printk("%d\n", s2);
				}
    			}
    		}    		
	#endif
    	
   
#ifdef DTMF_DEC_ISR
    	
    	/**************** Create det_buff from PCM RX FIFO *****************/
#ifdef SUPPORT_SLIC_GAIN_CFG
    	
#ifdef SUPPORT_CUT_DSP_PROCESS        
		for (ii=0; ii<40; ii++)
#else
		for (ii=0; ii<(int)(nPcmIntPeriod[chid] << 2); ii++)
#endif
	        {   
			stmp = *(&pRxBuf[chid][rx_rp]+(i*PCM_PERIOD_10MS_SIZE>>2)+ii);
		
			s1 = (short)((stmp >> 16) & 0x0000ffff);
			s2 = (short)((stmp & 0x0000ffff));

			det_buff[chid][ii<<1] = (int) (s1 * tx_comp[g_txVolumneGain[chid]]) >> 8;
			det_buff[chid][(ii<<1)+1] = (int) (s2 * tx_comp[g_txVolumneGain[chid]]) >> 8;

		}
#endif

#ifdef RTK_VOICE_RECORD
		if(stVoipdataget[chid].write_enable==4)
		{
			memcpy(&stVoipdataget[chid].rxbuf[stVoipdataget[chid].rx_writeindex],&pRxBuf[chid][rx_rp]+(i*PCM_PERIOD_10MS_SIZE>>2),160);
			stVoipdataget[chid].rx_writeindex= (stVoipdataget[chid].rx_writeindex+160)%DATAGETBUFSIZE;
		}
#endif //#ifdef RTK_VOICE_RECORD

	/**********************************************
		 *                                    *
		 * 	   DTMF Detection	      *
		 *                                    *
		 ****************************************************/		
			
		if (dtmf_chid[chid] == 1)
		{
#ifndef OPTIMIZATION
			if (printk_flag)
			{
				PRINT_MSG("\n------ PageSize before DTMF in ISR: %d ------\n", pcm_get_page_size(chid));
				printk_flag = 0;
			}
#endif			
			
		#ifdef FEATURE_COP3_DTMFDEC
			ProfileEnterPoint(PROFILE_INDEX_DTMFDEC);
		#endif
		
		#ifdef SUPPORT_SLIC_GAIN_CFG	
			dtmf_digit = dtmf_dec(det_buff[chid], PCM_PERIOD_10MS_SIZE, chid, DTMF_POWER_LEVEL_MINUS_32DBM);
		#else	
			dtmf_digit = dtmf_dec((char *)(&pRxBuf[chid][rx_rp]+(i*PCM_PERIOD_10MS_SIZE>>2)), PCM_PERIOD_10MS_SIZE, chid, DTMF_POWER_LEVEL_MINUS_32DBM);
		#endif	
			dtmf_in(chid, dtmf_digit.digit);
		
		#ifdef FEATURE_COP3_DTMFDEC
			ProfileExitPoint(PROFILE_INDEX_DTMFDEC);
			ProfilePerDump(PROFILE_INDEX_DTMFDEC,1000);
                #endif
                }
                
#endif	//DTMF_DEC_ISR	


                
#ifdef SEND_RFC2833_ISR  

	/**********************************************                 
		 *                                    *                 
		 * 	    Send RFC2833 Packet	      *                 
		 *                                    *                 
		 ****************************************************/	
		               
		        /* ASCII Table: char 0-9 -> Dec# 48-57
		         *		char A-D -> Dec# 65-68 
		         *		char  *  -> Dec# 42 
		         *		char  #  -> Dec# 35 
		         */
		        
		        /* RTP Evevt:   0-9   ->  0-9
		         *		*     ->  10	
		         *		#     ->  11
		         *		A-D   ->  12-15
		         *		Flash ->  16
		         */

  #ifdef CONFIG_RTK_VOIP_IP_PHONE
			if (( ppStartCodecTypeDesc[ chid ] != NULL ) && dtmf_mode[chid] != 1/*SIP INFO*/)
  #else
			if (dtmf_removal_flag[chid] && ( ppStartCodecTypeDesc[ chid ] != NULL ) && dtmf_mode[chid] != 1/*SIP INFO*/)
  #endif
			{
  #ifndef CONFIG_RTK_VOIP_IP_PHONE
				if (dtmf_digit.digit >= 48 && dtmf_digit.digit <= 57 && sned_flag[chid] == 0)	/* 0 to 9  */
				{
					sned_flag[chid] = 1;
					g_digit[chid] = dtmf_digit.digit-48;
					//printk("->%d\n",g_digit[chid]);
				}
				else if (dtmf_digit.digit >= 65 && dtmf_digit.digit <= 68 && sned_flag[chid] == 0)	/* A to D  */
				{
					sned_flag[chid] = 1;
					g_digit[chid] = dtmf_digit.digit-65+12;
					//printk("->%d\n",g_digit[chid]);
				}
				else if (dtmf_digit.digit == 42 && sned_flag[chid] == 0)	/* (*) */
				{
					sned_flag[chid] = 1;
					g_digit[chid] = dtmf_digit.digit-42+10;
					//printk("->%d\n",g_digit[chid]);
				}
				else if (dtmf_digit.digit == 35 && sned_flag[chid] == 0)	/* (#) */
				{
					sned_flag[chid] = 1;
					g_digit[chid] = dtmf_digit.digit-35+11;
					//printk("->%d\n",g_digit[chid]);
				}
  #endif /* !CONFIG_RTK_VOIP_IP_PHONE */

				if (sned_flag[chid] == 1 && (i%PCM_PERIOD) == 0)
				{
					SessNum = chanInfo_GetRegSessionNum(chid);

					for (j=0; j < SessNum; j++)
					{	
						ssid = chanInfo_GetRegSessionID(chid, j);
						if (ssid != SESSION_NULL)
						{
							if (RtpOpen[ssid])
								RtpTx_transmitEvent_ISR(chid, ssid, g_digit[chid]);
						}
						
						
					}
				}
			}
			
#endif	//SEND_RFC2833_ISR	
	
	/**********************************************
		 *                                    *
		 * 	FAX & MODEM Detection	      *
		 *                                    *
		 ****************************************************/

#ifdef SUPPORT_FAX_PASS_ISR

			/* callee should detect CED, when it is off-hook instead of codec start. */
			//if ( ( ppStartCodecTypeDesc[ chid ] != NULL ) )  /* 0:stop, 1:G711, 2:G7231, 3:G729, 4:G726 */
			{
				/************** FAX & MODEM ***************/	
				if (!ModemFlag[chid] && !FaxFlag[chid])
				{
#ifdef OPTIMIZATION
					uint32 sid;
					sid = chanInfo_GetTranSessionID(chid);
					if(sid == SESSION_NULL)
						sid = chid;
#endif

			#ifdef SUPPORT_SLIC_GAIN_CFG	
					modem_det(chid, det_buff[chid]);
			#else	
					modem_det(chid, (short*)(&pRxBuf[chid][rx_rp]+(i*PCM_PERIOD_10MS_SIZE>>2)));
			#endif	
				
					if( ModemFlag[chid] ) 
						DisableDspFunctionsIfModemOrFaxIsDetected( sid );
				}
								
			}
#endif //SUPPORT_FAX_PASS_ISR

		
	/**********************************************
		 *                                    *
		 * 	Line Echo Canceller	      *
		 *                                    *
		 ****************************************************/
		 
		#ifdef FEATURE_COP3_LEC				
				/* Add CP3 to caculate cycle need by LEC */
			ProfileEnterPoint(PROFILE_INDEX_LEC);
		#endif	
		 
#ifdef SUPPORT_LEC_G168_ISR

#ifdef CONFIG_RTK_SLIC_DAA_NEGOTIATION
		/* DAA_ISR_FLOW[]=2(call forward to PSTN), =1(3way with PSTN or call out by PSTN) */
		if ( support_lec_g168[chid] && ( ppStartCodecTypeDesc[ chid ] != NULL ) && DAA_ISR_FLOW[chid] != DRIVER_FLOW_PSTN_FORWARD || DAA_ISR_FLOW[chid] == DRIVER_FLOW_PSTN_3WAY)
#else
		if ( support_lec_g168[chid] && ( ppStartCodecTypeDesc[ chid ] != NULL ) )
#endif
		{
			if(rx_mute[chid] == 1)
                                memset(rx_fifo[chid][rx_fifo_cnt_w[chid]], 0,  PCM_PERIOD_10MS_SIZE); 
			else
			{	
#ifdef EXPER_AEC
#ifndef LEC_USE_CIRC_BUF
				AEC_g168( chid, (Word16*)&LEC_RinBuf[chid][80*PCM_PERIOD*(FSIZE-SYNC_POINT)+i*PCM_PERIOD_10MS_SIZE/2-SYNC_SAMPLE], 
#else
				AEC_g168( chid, (Word16*)&LEC_Rin_CircBuf[chid][80*PCM_PERIOD*LEC_buf_rindex[chid] +i*PCM_PERIOD_10MS_SIZE/2], 
#endif
				   		(Word16*)(&pRxBuf[chid][rx_rp]+(i*PCM_PERIOD_10MS_SIZE>>2)),
                                                (Word16*)(rx_fifo[chid][rx_fifo_cnt_w[chid]]));
#else
#ifndef LEC_USE_CIRC_BUF
				LEC_g168( chid, (Word16*)&LEC_RinBuf[chid][80*PCM_PERIOD*(FSIZE-SYNC_POINT)+i*PCM_PERIOD_10MS_SIZE/2-SYNC_SAMPLE], 
#else
				LEC_g168( chid, (Word16*)&LEC_Rin_CircBuf[chid][80*PCM_PERIOD*LEC_buf_rindex[chid] +i*PCM_PERIOD_10MS_SIZE/2], 
#endif
				   		(Word16*)(&pRxBuf[chid][rx_rp]+(i*PCM_PERIOD_10MS_SIZE>>2)),
                                                (Word16*)(rx_fifo[chid][rx_fifo_cnt_w[chid]]));
#endif

#ifdef EXPER_NR
				extern int iphone_handfree;
				if(iphone_handfree==1)
					NR( chid, (Word16*)(rx_fifo[chid][rx_fifo_cnt_w[chid]]), (Word16*)(rx_fifo[chid][rx_fifo_cnt_w[chid]]));
#endif

			}
		}
		else
#endif
		{
			if(rx_mute[chid] == 1)
                                memset(rx_fifo[chid][rx_fifo_cnt_w[chid]], 0,  PCM_PERIOD_10MS_SIZE); 
			else
                                memcpy(rx_fifo[chid][rx_fifo_cnt_w[chid]], &pRxBuf[chid][rx_rp]+(i*PCM_PERIOD_10MS_SIZE>>2),  PCM_PERIOD_10MS_SIZE);  
		}

		#ifdef FEATURE_COP3_LEC            
			ProfileExitPoint(PROFILE_INDEX_LEC);
			ProfilePerDump(PROFILE_INDEX_LEC,1000);
			//ProfileDump(PROFILE_INDEX_LEC, PROFILE_INDEX_LEC,1000);
		#endif


	/**********************************************
		 *                                    *
		 * 	Acoustic Echo Suppression     *
		 *                                    *
		 ****************************************************/
#ifdef SUPPORT_AES_ISR
	
		if ((support_AES[chid] == 0) || (ppStartCodecTypeDesc[chid] == NULL ) || (aes_do_cnt[chid] == aes_do_times[chid]))
			goto SKIP_AES;
		
#ifdef SUPPORT_CUT_DSP_PROCESS	
		for (ii=0; ii<40; ii++)
#else
		for (ii=0; ii<(int)(nPcmIntPeriod[chid] << 2); ii++)
#endif
        	{ 	
        		Word16 s1, s2, s3, s4;
			// read from pcm controller Rx page 
     			stmp = *((uint32 *)(&pRxBuf[chid][rx_rp]+(i*PCM_PERIOD_10MS_SIZE>>2)+ii));
			s1 = (short)((stmp >> 16) & 0x0000ffff);
			s2 = (short)((stmp & 0x0000ffff));

			// read from Tx fifo			
                        stmp = tx_fifo[chid][(tx_fifo_cnt_r[chid]+aes_do_cnt[chid])%PCM_FIFO_SIZE][ii];
			s3 = (short)((stmp >> 16) & 0x0000ffff);
			s4 = (short)((stmp & 0x0000ffff));

			AES(chid, (Word16*)&s1, (Word16*)&s3);
			AES(chid, (Word16*)&s2, (Word16*)&s4);
		
			// write to Rx fifo
			stmp = ((unsigned int)s1 << 16) + ((unsigned int)s2 & 0x0000ffff);
                        rx_fifo[chid][rx_fifo_cnt_w[chid]][ii] = stmp;
			
			// write to Tx fifo
			stmp = ((unsigned int)s3 << 16) + ((unsigned int)s4 & 0x0000ffff);			
                        tx_fifo[chid][(tx_fifo_cnt_r[chid]+aes_do_cnt[chid])%PCM_FIFO_SIZE][ii] = stmp;

		}

		aes_do_cnt[chid]++;
		

SKIP_AES:

#endif
	

	/**********************************************
		 *                                    *
		 * 	    DTMF REMOVAL	      *
		 *                                    *
		 ****************************************************/		
	
#ifdef DTMF_REMOVAL_ISR

		if (dtmf_removal_flag[chid] && ( ppStartCodecTypeDesc[ chid ] != NULL ) )
		{
			if ( dtmf_mode[chid] == 0 /*RFC2833*/ && remote_is_rfc2833[chid] != 0 || dtmf_mode[chid] == 1 /*SIP INFO*/ )
			{
			
				if(dtmf_removal[chid]) 
				{
					
#ifdef DTMF_REMOVAL_FORWARD
					if (clean_forward[chid] == 1 )
					{
	
						for (j=1; j <= pcm_get_rx_PCM_FIFO_SIZE(chid); j++)
						{
							if (rx_fifo_cnt_w[chid]-j >= 0)
							{
                                                                memset(rx_fifo[chid][(rx_fifo_cnt_w[chid]-j)], 0, PCM_PERIOD_10MS_SIZE);
								//printk("%d\n", rx_fifo_cnt_w[chid]-j);
							}
							else
							{
                                                                memset(rx_fifo[chid][(PCM_FIFO_SIZE-j+rx_fifo_cnt_w[chid])], 0, PCM_PERIOD_10MS_SIZE);
								//printk("%d\n", PCM_FIFO_SIZE-j+rx_fifo_cnt_w[chid]);
							}
							
						}
	
						clean_forward[chid] = 0;
					}
	
#endif //DTMF_REMOVAL_FORWARD
                                        memset(rx_fifo[chid][rx_fifo_cnt_w[chid]], 0, PCM_PERIOD_10MS_SIZE);
					dtmf_removal_cnt[chid] = 0;
				}
				else	
				{	/* To futher remove 30 ms after dtmf_removal =0 
				         * to remove the de-bounce annoying sound of the 
				         * telephone number buttons.
				         */
				         
					if (dtmf_removal_cnt[chid]< 3)
					{
                                                memset(rx_fifo[chid][rx_fifo_cnt_w[chid]], 0, PCM_PERIOD_10MS_SIZE);
						dtmf_removal_cnt[chid]++;
					}
					else
					{
						clean_forward[chid] = 1;
					}
				}
			
			}
		}	
		
#endif // DTMF_REMOVAL_ISR

                pRxFifoCurWriting = rx_fifo[chid][rx_fifo_cnt_w[chid]];

		/***** Clean PCM Rx (CED) *****/
                extern int clean_pcm_rx_cnt[MAX_SLIC_CH_NUM];
                extern char fax_2100_cnt_tx[MAX_SLIC_CH_NUM];
		
		/*
		 * Clean PCM rx:
		 * 1. detect fax ced tone     -> clean 2 seconds
		 * 2. 2.1k cnt > 2 (80ms)     -> clean, until modem or fax flag  
		 */
		if( ( clean_pcm_rx_cnt[ chid ] ) || 
			( fax_2100_cnt_tx[chid] > 2 ) )  
		{
			if( clean_pcm_rx_cnt[ chid ] )
				clean_pcm_rx_cnt[ chid ] --;
			
			/* clean CED and its gap while switching codec */
			memset(pRxFifoCurWriting, 0, PCM_PERIOD_10MS_SIZE);
		} 
		
	/**** Count the Rx FIFO write index ****/
	        rx_fifo_cnt_w[chid] = (rx_fifo_cnt_w[chid] +1)% PCM_FIFO_SIZE;  

        
	}//for (i=0; i < PCM_PERIOD; i++) 
	
	



#ifdef SUPPORT_LEC_G168_ISR
#if DMEN_LEC
	LEC_dmem_save(chid);
#endif
#endif

    	#ifdef LEC_G168_ISR_SYNC_P
 	
 	if (sync_start ==1 && sync ==0)
		sync_frame_R++;   
		
    	#endif
   	
		


#else	//=======================================================================//

#ifdef SUPPORT_CUT_DSP_PROCESS

	for (i=0; i<40; i++)
#else
	for (i=0; i<(int)(nPcmIntPeriod[chid] << 2); i++)
#endif
	{   
		RxBufTmp[chid][i] = pRxBuf[chid][rx_rp++];
	}	
		
#endif	//SUPPORT_PCM_FIFO
	//=======================================================================//
	
	//printk("(%d, %d) ", rx_fifo_cnt_w[chid], rx_fifo_cnt_r[chid]); 
	
	
	load_radiax_reg(); /* load saved radiax register value */
	

	return SUCCESS;
}
#else //AUDIOCODES_VOIP
	
#ifndef ASM
#define ASM __asm__ volatile
#endif


#define SET_MMD(Var1)		ASM("mtru	%0, $24" : : "d"((long)(Var1))); \
							ASM("nop");\
							ASM("nop")

#define SET_MMD_ZERO()		ASM("mtru	$0, $24"); \
							ASM("nop");\
							ASM("nop")

#define GET_MMD(Var1)		ASM("mfru	%0, $24" : "=d"((long)(Var1))); \
							ASM("nop");\
							ASM("nop")


/* Circular buffer registers */

#define cbs0	$0		/* cs0 */
#define cbs1	$1		/* cs1 */
#define cbs2	$2		/* cs2 */
#define cbe0	$4		/* ce0 */
#define cbe1	$5		/* ce1 */
#define cbe2	$6		/* ce2 */

/* Zero Overhead Loop Control registers */

#define lps0	$16		/* ls0 */
#define lpe0	$17		/* le0 */
#define lpc0	$18		/* lc0 */

/* MAC Mode Register */

#define mmd	$24		/* md */

#define __str2(x) #x
#define __str(x) __str2(x)

#define NO_REORDER_OR_AT() \
					ASM(".set noreorder");\
					ASM(".set noat")

#define REORDER_OR_AT() \
					ASM(".set reorder");\
					ASM(".set at")


#define GET_MxH(regLo, regHi, x)	\
{	\
					ASM("mfa	%0, m"__str(x)"h" : "=d"((unsigned long)(regLo))); \
					ASM("mfa	%0, m"__str(x)"h ,8" : "=d"((unsigned long)(regHi))); \
}

#define GET_MxL(regLo, regHi, x)	\
{	\
					ASM("mfa	%0, m"__str(x)"l" : "=d"((unsigned long)(regLo))); \
					ASM("mfa	%0, m"__str(x)"l ,8" : "=d"((unsigned long)(regHi))); \
}

#define SET_MxH(regLo, regHi, x)	\
{	\
					ASM("mta2	%0, m"__str(x)"h" :: "d"((unsigned long)(regLo))); \
					ASM("mta2.g	%0, m"__str(x)"h" :: "d"((unsigned long)(regHi))); \
}


#define SET_MxL(regLo, regHi, x)	\
{	\
					ASM("mta2	%0, m"__str(x)"l" :: "d"((unsigned long)(regLo))); \
					ASM("mta2.g	%0, m"__str(x)"l" :: "d"((unsigned long)(regHi))); \
}

#define GET_RADIAX_REG(reg, x)	ASM("mfru	%0,"__str(x) : "=d"((unsigned long)(reg)))

#define SET_RADIAX_REG(reg, x)	ASM("mtru	%0, "__str(x): : "d"((unsigned long)(reg)))

struct pt_acc {
	unsigned long mdl_l;
	unsigned long mdl_h;
	unsigned long mdh_l;
	unsigned long mdh_h;
};

struct pt_radiax {
	/* Saved accumolators. */
	struct pt_acc acc[4];

	/* Other saved registers. */
	unsigned long mmd;
	unsigned long lps0;
	unsigned long lpe0;
	unsigned long lpc0;
#if ! defined (AUDIOCODES_VOIP)
	unsigned long cbs0;
	unsigned long cbs1;
	unsigned long cbs2;
	unsigned long cbe0;
	unsigned long cbe1;
	unsigned long cbe2;
#endif
};

static inline void save_radiax_reg(struct pt_radiax *radiax_regs)
{
	NO_REORDER_OR_AT();
//radiax reg
	GET_RADIAX_REG(radiax_regs->lps0, lps0);
	GET_RADIAX_REG(radiax_regs->lpe0, lpe0);
	GET_RADIAX_REG(radiax_regs->lpc0, lpc0);
#if ! defined (AUDIOCODES_VOIP)
	GET_RADIAX_REG(radiax_regs->cbs0, cbs0);
	GET_RADIAX_REG(radiax_regs->cbs1, cbs1);
	GET_RADIAX_REG(radiax_regs->cbs2, cbs2);

	GET_RADIAX_REG(radiax_regs->cbe0, cbe0);
	GET_RADIAX_REG(radiax_regs->cbe1, cbe1);
	GET_RADIAX_REG(radiax_regs->cbe2, cbe2);
#endif
//radiax accumolator
	
	GET_MxH(radiax_regs->acc[0].mdh_l,
			  radiax_regs->acc[0].mdh_h,
			  0);

	GET_MxH(radiax_regs->acc[1].mdh_l,
			  radiax_regs->acc[1].mdh_h,
			  1);
	GET_MxH(radiax_regs->acc[2].mdh_l,
			  radiax_regs->acc[2].mdh_h,
			  2);
	GET_MxH(radiax_regs->acc[3].mdh_l,
			  radiax_regs->acc[3].mdh_h,
			  3);

	GET_MxL(radiax_regs->acc[0].mdl_l,
			  radiax_regs->acc[0].mdl_h,
			  0);

	GET_MxL(radiax_regs->acc[1].mdl_l,
			  radiax_regs->acc[1].mdl_h,
			  1);
	GET_MxL(radiax_regs->acc[2].mdl_l,
			  radiax_regs->acc[2].mdl_h,
			  2);
	GET_MxL(radiax_regs->acc[3].mdl_l,
			  radiax_regs->acc[3].mdl_h,
			  3);

	GET_RADIAX_REG(radiax_regs->mmd, mmd);

	REORDER_OR_AT();

}

static inline void restore_radiax_reg(struct pt_radiax *radiax_regs)
{
	NO_REORDER_OR_AT();

	SET_RADIAX_REG(radiax_regs->lps0, lps0);
	SET_RADIAX_REG(radiax_regs->lpe0, lpe0);
	SET_RADIAX_REG(radiax_regs->lpc0, lpc0);
#if ! defined (AUDIOCODES_VOIP)
	SET_RADIAX_REG(radiax_regs->cbs0, cbs0);
	SET_RADIAX_REG(radiax_regs->cbs1, cbs1);
	SET_RADIAX_REG(radiax_regs->cbs2, cbs2);

	SET_RADIAX_REG(radiax_regs->cbe0, cbe0);
	SET_RADIAX_REG(radiax_regs->cbe1, cbe1);
	SET_RADIAX_REG(radiax_regs->cbe2, cbe2);
#endif

	SET_MxH(radiax_regs->acc[0].mdh_l,
			  radiax_regs->acc[0].mdh_h,
			  0);

	SET_MxH(radiax_regs->acc[1].mdh_l,
			  radiax_regs->acc[1].mdh_h,
			  1);
	SET_MxH(radiax_regs->acc[2].mdh_l,
			  radiax_regs->acc[2].mdh_h,
			  2);
	SET_MxH(radiax_regs->acc[3].mdh_l,
			  radiax_regs->acc[3].mdh_h,
			  3);

	SET_MxL(radiax_regs->acc[0].mdl_l,
			  radiax_regs->acc[0].mdl_h,
			  0);

	SET_MxL(radiax_regs->acc[1].mdl_l,
			  radiax_regs->acc[1].mdl_h,
			  1);
	SET_MxL(radiax_regs->acc[2].mdl_l,
			  radiax_regs->acc[2].mdl_h,
			  2);
	SET_MxL(radiax_regs->acc[3].mdl_l,
			  radiax_regs->acc[3].mdl_h,
			  3);

	SET_RADIAX_REG(radiax_regs->mmd, mmd);

	REORDER_OR_AT();

}

extern void ACMWModemTx(char chid, short *pOutBuff);
extern void ACMWLEC_g168(char chid, short *pRin, short *pSin, short *pEx);

__pcmIsr02 int32 PCM_RX(uint32 chid)
{
	int i, ii, j, SessNum;
	uint32 rx_rp, ssid;
	unsigned int stmp;
	Word16 s1, s2;
	volatile struct pt_radiax radiax_regs;

	save_radiax_reg(&radiax_regs); /* save radiax register value */


#ifdef PCM_PERIOD
	rx_rp = rxpage[chid] * (40*PCM_PERIOD);	//count page0/page1 offset
#else
#ifdef SUPPORT_CUT_DSP_PROCESS
	rx_rp = rxpage[chid] * 40;	//count page0/page1 offset

#else
	rx_rp = rxpage[chid] * (nPcmIntPeriod[chid] << 2);	//count page0/page1 offset
#endif
#endif

#ifdef SUPPORT_PCM_FIFO //==========================================================//

   	for (i=0; i < PCM_PERIOD; i++)
	{

		/*** Check if Rx FIFO full or not ***/
		if (((rx_fifo_cnt_w[chid]+1)%PCM_FIFO_SIZE) == rx_fifo_cnt_r[chid]) 
		{
			// Full
			printk("RF(%d) ", chid);
			break;
		}

	#ifdef LEC_G168_ISR_SYNC_P
    	
    		if (sync == 0)
    		{
	    		for (ii=0; ii< 40; ii++)
    			{
    				stmp = *(&pRxBuf[chid][rx_rp]+(i*PCM_PERIOD_10MS_SIZE>>2)+ii);
    			
    				s1 = (short)((stmp >> 16) & 0x0000ffff);
				s2 = (short)((stmp & 0x0000ffff));
			
				//printk("%d ", s1);
			
				if (cnt_time > 100 && sync != 1)
				{	
					
					if ( s1 > level || s2 >level || s1 < -level || s2 < -level )
					{
						sync =1;
						printk("****** sync! sync_frame=%d, sync_frame_R=%d, i=%d, ii=%d ******\n", sync_frame, sync_frame_R, i, ii);
						
					}
				}
			
				if (sync ==1)
				{
					printk("%d\n", s1);
					printk("%d\n", s2);
				}
    			}
    		}    		
	#endif

	/**********************************************
		 *                                    *
		 * 	Line Echo Canceller	      *
		 *                                    *
		 ****************************************************/
		if (1)
		{
			if(rx_mute[chid] == 1)
				memset(rx_fifo[chid][rx_fifo_cnt_w[chid]], 0,  PCM_PERIOD_10MS_SIZE); 
			else
			{
				ACMWLEC_g168( chid, (Word16*)&LEC_RinBuf[chid][80*PCM_PERIOD*(FSIZE-SYNC_POINT)+i*PCM_PERIOD_10MS_SIZE/2-(SYNC_SAMPLE-8)],
				   					(Word16*)(&pRxBuf[chid][rx_rp]+(i*PCM_PERIOD_10MS_SIZE>>2)),
									(Word16*)(rx_fifo[chid][rx_fifo_cnt_w[chid]]));
			}
		}
		else
		{
			if(rx_mute[chid] == 1)
				memset(rx_fifo[chid][rx_fifo_cnt_w[chid]], 0,  PCM_PERIOD_10MS_SIZE); 
			else
				memcpy(rx_fifo[chid][rx_fifo_cnt_w[chid]], &pRxBuf[chid][rx_rp]+(i*PCM_PERIOD_10MS_SIZE>>2),  PCM_PERIOD_10MS_SIZE);
		}

	/**** Count the Rx FIFO write index ****/
	        rx_fifo_cnt_w[chid] = (rx_fifo_cnt_w[chid] +1)% PCM_FIFO_SIZE;


	}//for (i=0; i < PCM_PERIOD; i++)
	
	#ifdef LEC_G168_ISR_SYNC_P
 	
 	if (sync_start ==1 && sync ==0)
		sync_frame_R++;   
		
    	#endif

#else	//=======================================================================//

#ifdef SUPPORT_CUT_DSP_PROCESS

	for (i=0; i<40; i++)
#else
	for (i=0; i<(int)(nPcmIntPeriod[chid] << 2); i++)
#endif
	{
		RxBufTmp[chid][i] = pRxBuf[chid][rx_rp++];
	}

#endif	//SUPPORT_PCM_FIFO
	//=======================================================================//

	//printk("(%d, %d) ", rx_fifo_cnt_w[chid], rx_fifo_cnt_r[chid]);


	restore_radiax_reg(&radiax_regs); /* load saved radiax register value */


	return SUCCESS;
}
	
#endif


#ifdef reg_isr
int pcm_isr_register(int(*callback)(unsigned int status_val))
{
	//myreg=(struct my_reg *) kmalloc(sizeof(struct my_reg), GFP_ATOMIC);
	//myreg->isr_callback = (void *)callback;
	isr_callback = (void *)callback;
	return 0;
}
#endif


#ifdef PCM_HANDLER_USE_CLI
// disable All GIMR except PCM and Timer
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
unsigned short old_pcm_gimr_value;
#define GIMR_PCM (0xBD010000)
#define GIMR_MASK_PCM (0x081)
#elif CONFIG_RTK_VOIP_DRIVERS_PCM8651
unsigned long old_pcm_gimr_value;
#define GIMR_PCM (0xBD012000)
#define GIMR_MASK_PCM (0x80040000)
#elif CONFIG_RTK_VOIP_DRIVERS_PCM8671
unsigned short old_pcm_gimr_value;
#define GIMR_PCM (0xb9c03010)
#define GIMR_MASK_PCM (0x4020)
#endif

static inline void cli_pcm(void){
	unsigned long flags;
	save_flags(flags); cli();

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
	old_pcm_gimr_value = *(volatile unsigned short *)GIMR_PCM;
	*(volatile unsigned short *)GIMR_PCM = GIMR_MASK_PCM;
#elif CONFIG_RTK_VOIP_DRIVERS_PCM8651
	old_pcm_gimr_value = *(volatile unsigned long *)GIMR_PCM;
	*(volatile unsigned long *)GIMR_PCM = GIMR_MASK_PCM;
#elif CONFIG_RTK_VOIP_DRIVERS_PCM8671
	old_pcm_gimr_value = *(volatile unsigned short *)GIMR_PCM;
	*(volatile unsigned short *)GIMR_PCM = GIMR_MASK_PCM;
#endif
	restore_flags(flags);
}

static inline void sti_pcm(void){
	unsigned long flags;
	save_flags(flags); cli();

	// restore GIMR
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
	*(volatile unsigned short *)GIMR_PCM = old_pcm_gimr_value;
#elif CONFIG_RTK_VOIP_DRIVERS_PCM8651
	*(volatile unsigned long *)GIMR_PCM = old_pcm_gimr_value;
#elif CONFIG_RTK_VOIP_DRIVERS_PCM8671
	*(volatile unsigned short *)GIMR_PCM = old_pcm_gimr_value;
#endif

	restore_flags(flags);
}

#endif /* PCM_HANDLER_USE_CLI */

#if defined (AUDIOCODES_VOIP)
#include "../../acmw_lx/ac_drv/AC49xDrv_Config.h"
#define PCM_10MSLEN_INSHORT 80

typedef volatile struct {
	unsigned short *RxBuff;
	unsigned short *TxBuff;
}Tacmw_transiverBuff;

static Tacmw_transiverBuff transiverBuff[ACMW_MAX_NUM_CH];
extern int ACMWPcmProcess(const Tacmw_transiverBuff *transiverBuff, const unsigned int maxCh, const unsigned int size);
#endif

#ifdef SUPPORT_PCM_FIFO
#if defined (AUDIOCODES_VOIP)
__pcmImem int PCM_handler_2(unsigned long *dummy)
#else
int PCM_handler_2(unsigned long *dummy)
#endif
{
	static unsigned int chid = 0;
	unsigned int f_cnt;
	unsigned int i;
	unsigned int stmp;

	
#ifdef PCM_HANDLER_USE_CLI
	cli_pcm();
#endif


	for (f_cnt = 0; f_cnt < (SLIC_CH_NUM*PCM_FIFO_SIZE*2); f_cnt++, chid = (++chid%SLIC_CH_NUM)) 
	{
		if (chanEnabled[chid] == 0)
			continue;	
			
#if defined (AUDIOCODES_VOIP)			
		for(i=0; i < ACMW_MAX_NUM_CH; i++)
		{
			transiverBuff[i].TxBuff = NULL;
			transiverBuff[i].RxBuff = NULL;
		}
#endif			
				
				

		//printk("2: %d-%X\n", pcm_get_read_rx_fifo_addr(chid, &pRxBufTmp), pRxBufTmp);
#ifdef REDUCE_PCM_FIFO_MEMCPY
		if( pcm_get_read_rx_fifo_addr(chid, (void*)&pRxBufTmp))
#else
		if( pcm_read_rx_fifo(chid, (void*)&RxBufTmp[chid][0]))
#endif			
		{
			break;
		}
		else if (pcm_get_write_tx_fifo_addr(chid, &pTxBufTmp))
		{
			break;
		}
#ifdef CONFIG_RTK_SLIC_DAA_NEGOTIATION
		else
		{
			if ( (DAA_ISR_FLOW[chid] == DRIVER_FLOW_PSTN_3WAY) || (DAA_ISR_FLOW[chid] == DRIVER_FLOW_PSTN_FORWARD) )
			{	
				if( pcm_get_read_rx_fifo_addr_DAA(2, (void*)&pRxBufTmp_DAA))
				{
					//daa rx fifo empty
					//printk("ARE ");
					break;
				}
				else
				{
					
					if( pcm_get_write_tx_fifo_addr_DAA(2, &pTxBufTmp_DAA))
						printk("TF1(daa)\n");
#ifdef SUPPORT_AES_FOR_DAA						
					memcpy(&AES_RinBuf[0], &AES_RinBuf[80], PCM_PERIOD_10MS_SIZE); /* shift right page to left page */
#endif							
					/* DAA Tx = SLIC Rx */
					if (DAA_ISR_FLOW[chid] == DRIVER_FLOW_PSTN_3WAY) /* 3-way with PSTN, or call out with PSTN */
					{
						if (g_DAA_used[chid] == DAA_USE)
							memcpy(pTxBufTmp_DAA, pRxBufTmp, PCM_PERIOD_10MS_SIZE);
#ifdef SUPPORT_AES_FOR_DAA			
                                                        /************ Backup AES Rin Buffer Part I ==> copy rx_fifo[][] to AES_RinBuf[] *********/            
								
							memcpy(&AES_RinBuf[80], pRxBufTmp, PCM_PERIOD_10MS_SIZE);
								
							/*****************************************************************************************/
#endif
					}
						
					/* add SLIC Rx and DAA Rx to SLIC RX, then send to RTP */
					for (i=0; i< 40; i++)
					{
                                                rx_fifo[chid][rx_fifo_cnt_r[chid]][i]+=
                                                        rx_fifo_DAA[rx_fifo_cnt_r_DAA][i];
					}
					
				}
			}
			
		}
			
#endif //CONFIG_RTK_SLIC_DAA_NEGOTIATION	 
	
	
#ifdef T38_STAND_ALONE_HANDLER
		if( t38RunningState[ chid ] == T38_START )
			PCM_handler_T38( chid );
		else
#endif	 
		{
			
#if defined (AUDIOCODES_VOIP)
			pcm_get_read_rx_fifo_addr(chid, (void*)&transiverBuff[chid].RxBuff);
			pcm_get_write_tx_fifo_addr(chid, (void*)&transiverBuff[chid].TxBuff);
			
			#ifdef FEATURE_COP3_PCM_HANDLER
			ProfileEnterPoint(PROFILE_INDEX_PCM_HANDLER);
			#endif
			//activate AC MiddelWare		
			ACMWPcmProcess( &transiverBuff[0], ACMW_MAX_NUM_CH, PCM_10MSLEN_INSHORT );
			
			#ifdef FEATURE_COP3_PCM_HANDLER
			ProfileExitPoint(PROFILE_INDEX_PCM_HANDLER);
			ProfilePerDump(PROFILE_INDEX_PCM_HANDLER,1000);
			#endif
			
			if(transiverBuff[chid].TxBuff != NULL)
				pcm_write_tx_fifo_done(chid);

#else
			PCM_handler(chid);
#endif
		}
		
		


                /***** Backup AES Rin Buffer Part II ==> copy tx_fifo[][] to AES_RinBuf[] *****/                      
#ifdef SUPPORT_AES_FOR_DAA

		if (DAA_ISR_FLOW[chid] == DRIVER_FLOW_PSTN_3WAY) /* copy tx_fifo[][] to AES_RinBuf[] */
		{
			for (i=0; i< 40; i++)
			{
				stmp = tx_fifo[chid][PREV_FIFO_ENTRY(tx_fifo_cnt_w[chid])][i];
				AES_RinBuf[80+2*i] += (short)((stmp >> 16) & 0x0000ffff);;
				AES_RinBuf[80+2*i+1] += (short)((stmp & 0x0000ffff));
			}
		}
		else if (DAA_ISR_FLOW[chid] == DRIVER_FLOW_PSTN_FORWARD)
		{
			memcpy(&AES_RinBuf[80], tx_fifo[chid][PREV_FIFO_ENTRY(tx_fifo_cnt_w[chid])], PCM_PERIOD_10MS_SIZE); 
		}

#endif				
		/********************************************************************************/	
		
#ifdef CONFIG_RTK_SLIC_DAA_NEGOTIATION			

		/* Decode PCM copy to DAA Tx */
		
		if (DAA_ISR_FLOW[chid] == DRIVER_FLOW_PSTN_3WAY)
		{	
			int fifo_index;
			
			if (tx_fifo_cnt_w[chid] == 0)
				fifo_index= (PCM_FIFO_SIZE - tx_fifo_cnt_w[chid]-1);
			else
				fifo_index= (tx_fifo_cnt_w[chid]-1);
							
			if (g_DAA_used[chid] == DAA_USE_HOLD) // hold
			{
				for (i=0; i< 40; i++)
					pTxBufTmp_DAA[i] = ToneBuf_DAA[i];
			}
			else if (g_DAA_used[chid] == DAA_USE)
			{
				for (i=0; i< 40; i++)
                                        pTxBufTmp_DAA[i] += tx_fifo[chid][fifo_index][i];
			}

		}
		else if (DAA_ISR_FLOW[chid] == DRIVER_FLOW_PSTN_FORWARD)
		{
			if (tx_fifo_cnt_w[chid] == 0)
                                memcpy(pTxBufTmp_DAA, tx_fifo[chid][(PCM_FIFO_SIZE - tx_fifo_cnt_w[chid]-1)], PCM_PERIOD_10MS_SIZE);
			else
                                memcpy(pTxBufTmp_DAA, tx_fifo[chid][(tx_fifo_cnt_w[chid]-1)], PCM_PERIOD_10MS_SIZE);          }
		
		
		/* SLIC Tx = DAA Rx */
		if (DAA_ISR_FLOW[chid] == DRIVER_FLOW_PSTN_3WAY) /* 3-way with PSTN, or call out with PSTN */
		{
			if (tx_fifo_cnt_w[chid] == 0)
			{
				for (i=0; i< 40; i++)
                                        tx_fifo[chid][(PCM_FIFO_SIZE - tx_fifo_cnt_w[chid]-1)][i] += pRxBufTmp_DAA[i];
			}
			else
			{
				for (i=0; i< 40; i++)
                                        tx_fifo[chid][(tx_fifo_cnt_w[chid]-1)][i] += pRxBufTmp_DAA[i];
			}
		}
		

		
		if ( (DAA_ISR_FLOW[chid] == DRIVER_FLOW_PSTN_3WAY) || (DAA_ISR_FLOW[chid] == DRIVER_FLOW_PSTN_FORWARD) )
		{
			if (pcm_write_tx_fifo_done_DAA(2))
				printk("TF(daa)\n");
		}

#endif //CONFIG_RTK_SLIC_DAA_NEGOTIATION
			
#ifdef REDUCE_PCM_FIFO_MEMCPY
		pcm_read_rx_fifo_done(chid);

#ifdef CONFIG_RTK_SLIC_DAA_NEGOTIATION			
		if ( (DAA_ISR_FLOW[chid] == DRIVER_FLOW_PSTN_3WAY) || (DAA_ISR_FLOW[chid] == DRIVER_FLOW_PSTN_FORWARD) )
			pcm_read_rx_fifo_done_DAA(2);
#endif

#endif			
	

	} // for
	
	if (f_cnt >= (SLIC_CH_NUM*PCM_FIFO_SIZE*2))
		printk("PCM tasklet: f_cnt =%d\n", f_cnt);
#ifdef PCM_HANDLER_USE_CLI
	sti_pcm();
#endif		
	return 0;
}

#if defined (AUDIOCODES_VOIP)
__pcmImem void PCM_handler_3()
#else
void PCM_handler_3()
#endif
{
#ifdef PCM_HANDLER_USE_TASKLET
	tasklet_hi_schedule(&pcm_handler_tasklet);	
#else
	PCM_handler_2(NULL);
#endif
}

#endif // SUPPORT_PCM_FIFO



#ifdef SUPPORT_PCM_FIFO
#if defined (AUDIOCODES_VOIP)
__pcmIsr01  
#endif
int32 pcm_ISR(uint32 pcm_isr)
{
	static uint32 chid = 0;
	uint32 tx_isrpage, rx_isrpage, orig_isr = pcm_isr;
	uint32 f_cnt;

#ifndef OPTIMIZATION   
	static uint32 last_isr=0; 
#endif

#ifdef FEATURE_COP3_PCMISR
	//ProfileEnterPoint(PROFILE_INDEX_PCMISR);
#endif	

#if !defined (AUDIOCODES_VOTING_MECHANISM)
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
#if 1
	if( pcm_isr & 0x0F0F0F0F )
	{
		if(pcm_isr & 0x0F000000)
			printk("BU0 ");
		if(pcm_isr & 0x000F0000)
			printk("BU1 ");
	}
#endif
	pcm_isr &= 0xF0F0F0F0;
#endif
#endif /*AUDIOCODES_VOTING_MECHANISM*/

	for (f_cnt = 0; f_cnt < SLIC_CH_NUM*2; f_cnt++, chid = (++chid%SLIC_CH_NUM)) 
	{
#if ! defined (AUDIOCODES_VOTING_MECHANISM)	
	    if (chanEnabled[chid] == FALSE)
			continue;		
#endif

#ifndef OPTIMIZATION
	    int page_size = pcm_get_page_size(chid);
#endif

	    if ( pcm_isr & (0xF0000000 >> (chid*8))) {

		tx_isrpage = 2*chid + txpage[chid];
		if( (pcm_isr & ChanTxPage[tx_isrpage]) )
		{		
#if defined (AUDIOCODES_VOTING_MECHANISM)	
			if (chanEnabled[chid] == TRUE)	{
#endif
#ifdef LEC_USE_CIRC_BUF		
			LEC_buf_windex[chid]=(LEC_buf_windex[chid]+1)%FSIZE;
#endif
			if (tx_fifo_cnt_r[chid] == tx_fifo_cnt_w[chid])
			{
				// TX FIFO is empty!
				//memset(&pTxBuf[chid][txpage[chid]*pcm_get_page_size(chid)>>2], 0, pcm_get_page_size(chid));	
				printk("TE(%d) ", chid);
			}
			else
			{

				if (fifo_start[chid] == 0) 
				{
					// Let RX/TX FIFO has one extra page to avoid TX FIFO empty!
					if ( tx_fifo_cnt_w[chid] >= TX_FIFO_START_NUM) { // suppose tx_fifo_cnt_r[chid] is 0 at the begining of a call!!
						fifo_start[chid] = 1;
						//printk("tx fifo_start(%d,%d)\n", tx_fifo_cnt_r[chid], tx_fifo_cnt_w[chid]);
					}
				} 
				
				if (fifo_start[chid] == 1) {
					int i;
#ifdef OPTIMIZATION
					uint32* txbuf = &pTxBuf[chid][txpage[chid]*(pcm_get_page_size(chid)>>2)];
#else					
					uint32* txbuf = &pTxBuf[chid][txpage[chid]*page_size>>2];
#endif					
					pcm_read_tx_fifo(chid, txbuf);

#ifdef CONFIG_RTK_VOIP_IVR
					if (tx_mute[chid] !=  1)
					{
						for (i=0; i < PCM_PERIOD; i++)
							MixIvrSpeechWaveToPlayoutBuffer( chid, txbuf + i*(PCM_PERIOD_10MS_SIZE/4) );
					}
#endif
#if 0
					voice_gain( txbuf, PCM_PERIOD*80, voice_gain_spk[chid]);
#endif

#ifdef RTK_VOICE_RECORD
					if(stVoipdataget[chid].write_enable==4)
					{
						for (i=0; i < PCM_PERIOD; i++)
						{
							memcpy(&stVoipdataget[chid].txbuf[stVoipdataget[chid].tx_writeindex],txbuf + i*(PCM_PERIOD_10MS_SIZE/4),160);
							stVoipdataget[chid].tx_writeindex= (stVoipdataget[chid].tx_writeindex+160)%DATAGETBUFSIZE;
						}
					}
#endif //#ifdef RTK_VOICE_RECORD
				} else
#ifdef OPTIMIZATION
					memset(&pTxBuf[chid][txpage[chid]*(pcm_get_page_size(chid)>>2)], 0, pcm_get_page_size(chid));
#else					
					memset(&pTxBuf[chid][txpage[chid]*page_size>>2], 0, page_size);
#endif					

			}
#if defined (AUDIOCODES_VOTING_MECHANISM)	
		}
#endif
			pcm_set_tx_own_bit(chid, txpage[chid]);
			pcm_isr &= ~ChanTxPage[tx_isrpage];
			txpage[chid] ^= 1;
		} 

		rx_isrpage = 2*chid + rxpage[chid];
		if( pcm_isr & ChanRxPage[rx_isrpage] ) {	

#ifdef LEC_USE_CIRC_BUF
			LEC_buf_rindex[chid]=(LEC_buf_rindex[chid]+1)%FSIZE;
#endif
#if defined (AUDIOCODES_VOTING_MECHANISM)	
			if (chanEnabled[chid] == TRUE)	PCM_RX(chid);
#else
			PCM_RX(chid);
#endif
			pcm_set_rx_own_bit(chid, rxpage[chid]);
			pcm_isr &= ~ChanRxPage[rx_isrpage];
			rxpage[chid] ^= 1;									
		}


	    } // end of while


	    
	} // end of for
		
#ifndef OPTIMIZATION   
	last_isr = orig_isr;

	if (pcm_isr != 0)
		printk(" %X ", pcm_isr);
#endif	


#ifdef FEATURE_COP3_PCMISR
	//ProfileExitPoint(PROFILE_INDEX_PCMISR);
	//ProfilePerDump(PROFILE_INDEX_PCMISR,1000);
#endif

#ifdef OPTIMIZATION
#ifdef PCM_HANDLER_USE_TASKLET
	tasklet_hi_schedule(&pcm_handler_tasklet);	
#else
	PCM_handler_2(NULL);
#endif
#else
	PCM_handler_3();
#endif
	
	return 0;
}
#else // SUPPORT_PCM_FIFO

int32 pcm_ISR(uint32 pcm_isr)
{
	uint32 chid, tx_isrpage, rx_isrpage, orig_isr = pcm_isr;
	static uint32 last_isr=0; 

#ifdef FEATURE_COP3_PCMISR
	ProfileEnterPoint(PROFILE_INDEX_PCMISR);
#endif	

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651

#if 1
	if( pcm_isr & 0x0F0F0F0F )
	{
		if(pcm_isr & 0x0F000000)
			printk("BU0 ");
		if(pcm_isr & 0x000F0000)
			printk("BU1 ");
	}
#endif
	pcm_isr &= 0xF0F0F0F0;
#endif


	for(chid=0; chid<SLIC_CH_NUM; chid++)
	{
		if (chanEnabled[chid] == FALSE)
			continue;
			
	    while ( pcm_isr & (0xF0000000 >> (chid*8))) {

// Handle Tx First	
		tx_isrpage = 2*chid + txpage[chid];
		if( (pcm_isr & ChanTxPage[tx_isrpage]) && (tr_cnt[chid] == 0))
		{
			if (tr_cnt[chid] > 0) {
				printk("TX: tr_cnt[chid]=%d, isr=%X(%X), page(%d,%d)\n", tr_cnt[chid], orig_isr, last_isr, txpage[chid], rxpage[chid]);
				while(1) ;			
			}
			tr_cnt[chid]++;
			memcpy( &pTxBuf[chid][txpage[chid]*pcm_get_page_size(chid)>>2], &TxBufTmp[chid][0] , pcm_get_page_size(chid) );
			pcm_set_tx_own_bit(chid, txpage[chid]);
			pcm_isr &= ~ChanTxPage[tx_isrpage];
			txpage[chid] ^= 1;
			//printk(" t%d ", frame_count%10);
			//printk("t");
		} 

		// Check ROK to avoid system down!
		rx_isrpage = 2*chid + rxpage[chid];
		if ( (pcm_isr & ChanRxPage[rx_isrpage]) == 0 ) {
			int pcm_isr2 = pcm_inl(PCMISR) & ChanRxPage[rx_isrpage]; // ROK only
			pcm_clean_isr(pcm_isr2);
			pcm_isr |= pcm_isr2;
			//printk("R\n");
		}
				


		if( pcm_isr & ChanRxPage[rx_isrpage] ) {	
			//printk("r");
			if (tr_cnt[chid] == 0) { // wait for TOK: for db-121 un-buffed board only?
				//PCM_recovery(chid, &pcm_isr, &tr_cnt[chid], &txpage[chid], &rxpage[chid]);
				//continue;
				printk(".");
				tr_cnt[chid]++;
			}

			tr_cnt[chid]--;
			
		#ifdef FEATURE_COP3_PCM_RX
			ProfileEnterPoint(PROFILE_INDEX_PCM_RX);
		#endif	
			PCM_RX(chid);
			pcm_set_rx_own_bit(chid, rxpage[chid]);
		#ifdef FEATURE_COP3_PCM_RX
			ProfileExitPoint(PROFILE_INDEX_PCM_RX);
			ProfilePerDump(PROFILE_INDEX_PCM_RX,1000);
		#endif
			//pcm_set_rx_own_bit(chid, rxpage[chid]);
			pcm_isr &= ~ChanRxPage[rx_isrpage];
			rxpage[chid] ^= 1;
		#ifdef FEATURE_COP3_PCM_HANDLER
			ProfileEnterPoint(PROFILE_INDEX_PCM_HANDLER);
		#endif					
			PCM_handler(chid);
			//printk(" r2%d ", frame_count%10);
		#ifdef FEATURE_COP3_PCM_HANDLER
			ProfileExitPoint(PROFILE_INDEX_PCM_HANDLER);
			ProfilePerDump(PROFILE_INDEX_PCM_HANDLER,1000);
		#endif
			// Check TOK to avoid TU
			tx_isrpage = 2*chid + txpage[chid];
			if ( (pcm_isr & ChanTxPage[tx_isrpage]) == 0 ) {
				int pcm_isr2 = pcm_inl(PCMISR) & ChanTxPage[tx_isrpage]; 
				if (pcm_isr2 != 0) {
					pcm_clean_isr(pcm_isr2);
					pcm_isr |= pcm_isr2;

					tr_cnt[chid]++;

					memcpy( &pTxBuf[chid][txpage[chid]*pcm_get_page_size(chid)>>2], &TxBufTmp[chid][0] , pcm_get_page_size(chid) );

					pcm_set_tx_own_bit(chid, txpage[chid]);
					pcm_isr &= ~ChanTxPage[tx_isrpage];
					txpage[chid] ^= 1;
					//printk("T");
				}	
				//printk("R\n");
			}

		}

	    } // end of while
	    

	
	
	    
	} // end of for
		
	last_isr = orig_isr;

	if (pcm_isr != 0)
		printk(" %X ", pcm_isr);

#ifdef FEATURE_COP3_PCMISR
	ProfileExitPoint(PROFILE_INDEX_PCMISR);
	ProfilePerDump(PROFILE_INDEX_PCMISR,1000);
#endif

	return 0;
}
#endif // SUPPORT_PCM_FIFO

#ifdef CONFIG_RTK_VOIP_MODULE
#undef SUPPORT_SYS_DMEM_STACK
#else
#if ! defined (AUDIOCODES_VOIP)
#ifdef PCM_HANDLER_USE_TASKLET
#define SUPPORT_SYS_DMEM_STACK
#endif
#endif
#endif

#ifdef SUPPORT_SYS_DMEM_STACK
#include "../../voip_dsp/dsp_r1/common/util/codec_mem.h"
#endif

#define CHECK_PCM_ISR_AGAIN

#if defined (AUDIOCODES_VOIP)
__pcmIsr00 
#endif
void pcm_interrupt(int32 irq, void *dev_instance, struct pt_regs *regs)
{
	unsigned long flags;
	unsigned int status_val;

	save_flags(flags); cli();
	
#ifdef SUPPORT_SYS_DMEM_STACK
	set_DMEM(&__sys_dmem_start);

	//sys_orig_sp = 0;
	sys_dmem_sp = &(sys_dmem_stack[SYS_DMEM_SSIZE]);
	entry_dmem_stack(&sys_orig_sp, &sys_dmem_sp);
#endif    

	
#ifdef FEATURE_COP3_PCMISR
	ProfileEnterPoint(PROFILE_INDEX_PCMISR);
#endif		
	
	//printk("1 PCMISR= 0x%x\n", pcm_inl(PCMISR));

#ifdef CHECK_PCM_ISR_AGAIN
	//int pcm_isr_cnt = 0;
	while((status_val = pcm_inl(PCMISR)))
#else
	if((status_val = pcm_inl(PCMISR)))
#endif	
	{
#ifdef CHECK_PCM_ISR_AGAIN
#if 0
		pcm_isr_cnt++;
#endif		
#endif
#ifdef OPTIMIZATION
#if defined (AUDIOCODES_VOTING_MECHANISM)
		AC_PCM_ISR_WRITE(status_val);
#else
		pcm_outl(PCMISR, status_val);
#endif
#else
#if defined (AUDIOCODES_VOTING_MECHANISM)
		AC_PCM_ISR_WRITE(status_val);
#else	
		pcm_clean_isr(status_val);
#endif		
#endif		
		//if ((status_val& pcm_inl(PCMIMR)) & 0xF0F0F0F0) { // TOK and ROK only
#ifdef CONFIG_RTK_SLIC_DAA_NEGOTIATION
		if ((status_val) & 0x0000F000) { // TOK and ROK only
			pcm_ISR_DAA(status_val & 0x0000F000);
		}
		
		if ((status_val) & 0xF0F000F0) { // TOK and ROK only
			pcm_ISR(status_val & 0xF0F000F0);
		}
#else		
		if ((status_val) & 0xF0F0F0F0) { // TOK and ROK only
			pcm_ISR(status_val & 0xF0F0F0F0);
		}
#endif		
			
#if ! defined (AUDIOCODES_VOTING_MECHANISM)
	
#if 1
		unsigned int maskval;
		int channel=0;
		//if((maskval = (status_val & ISR_MASK(channel))))
		if((maskval = (status_val & 0x0F0F0F0F))) // Buffer Unavailable only
		{
#if 0		
			if(maskval & ROK_MASK(channel))
				printk("ROK.....");
			if(maskval & TOK_MASK(channel))
				printk("TOK.....");
#endif		
			channel=0;
			if(maskval & RBU_MASK(channel))
				printk("R0 ");
			if(maskval & TBU_MASK(channel))	
				printk("T0 ");
			channel=1;
			if(maskval & RBU_MASK(channel))
				printk("R1 ");
			if(maskval & TBU_MASK(channel))	
				printk("T1 ");
#ifdef CONFIG_RTK_SLIC_DAA_NEGOTIATION		
			channel=2;
			if(maskval & RBU_MASK(channel))
				printk("R2 ");
			if(maskval & TBU_MASK(channel))	
				printk("T2 ");
#endif			
			//printk("\n");
		}
#endif
#endif /*AUDIOCODES_VOTING_MECHANISM*/	

	}

#ifdef CHECK_PCM_ISR_AGAIN
#if 0
	if ( pcm_isr_cnt > 1)
		printk(" (%d) ", pcm_isr_cnt);
#endif		
#endif

#ifdef FEATURE_COP3_PCMISR
	ProfileExitPoint(PROFILE_INDEX_PCMISR);
#endif

#ifdef SUPPORT_SYS_DMEM_STACK
	leave_dmem_stack(&sys_orig_sp);
#endif
	restore_flags(flags);
	
#ifdef FEATURE_COP3_PCMISR
	ProfilePerDump(PROFILE_INDEX_PCMISR,1000);
#endif	
		
}

static void pcmdev_malloc(struct pcm_priv* pcm_dev, unsigned char CH)
{
		/*  BUFFER_SIZE : define the allocated memory size for PCM tx, rx buffer(2 page) */
		/*  pcmdev_malloc() transfer allocated tx, rx buffer to noncache region and map to physical address */

				
		pcm_dev->tx_allocated = kmalloc(BUFFER_SIZE, GFP_KERNEL);
		
		if(pcm_dev->tx_allocated)
		{
			pcm_dev->rx_allocated = kmalloc(BUFFER_SIZE, GFP_KERNEL);
			
			if(pcm_dev->rx_allocated)
			{
				//assign non-cached address.
				pcm_dev->tx_buffer = (unsigned char *)Virtual2NonCache(pcm_dev->tx_allocated);
				pcm_dev->rx_buffer = (unsigned char *)Virtual2NonCache(pcm_dev->rx_allocated);

				PDBUG("Tx get noncache addr at 0x%X\n", (unsigned int)pcm_dev->tx_buffer);
				PDBUG("Rx get noncache addr at 0x%X\n", (unsigned int)pcm_dev->rx_buffer);

				//set TX, Rx buffer
				pcm_outl(TX_BSA(CH), (unsigned int)(Virtual2Physical(pcm_dev->tx_buffer)));	
				pcm_outl(RX_BSA(CH), (unsigned int)(Virtual2Physical(pcm_dev->rx_buffer)));
				PDBUG("Virtula2Physical OK!\n");	
				
				//set all allocated buffer to 0
				memset(pcm_dev->tx_allocated, 0, BUFFER_SIZE);
				memset(pcm_dev->rx_allocated, 0, BUFFER_SIZE);
				PDBUG("Set all allocated buffer to 0 OK!\n");
			}
		}
}
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186

static int pcm_fasync_fun(int fd, struct file *filp, int mode)
{
	struct pcm_priv *pcm_dev;
	pcm_dev = &(pcmctrl_devices[0]);
	return fasync_helper(fd, filp, mode, &(pcm_dev->pcm_fasync));
}

struct file_operations pcmctrl_fops = {
	fasync:         pcm_fasync_fun,
};
#endif
int __init pcmctrl_init(void)	
{
#if defined (AUDIOCODES_VOIP)
	PCM_init();
#endif

#ifdef SUPPORT_AES_FOR_DAA
	Init_AES(); /* Must be init before pcm_ISR_DAA() work, because AES is used in pcm_ISR_DAA(). */	
#endif
	int chid;
	for (chid=0; chid<MAX_SLIC_CH_NUM; chid++)
	{
		pcm_set_tx_mute(chid, FALSE);
		pcm_set_rx_mute(chid, FALSE);
	}

#ifdef RELAY_SW_CTRL_GPIOE	//for gpioE used.pull relay high.	
	#define GPEF_DIR  *((volatile unsigned int *)0xbd010144)
	#define GPEF_DATA  *((volatile unsigned int *)0xbd010140)	
	BOOT_MSG("GPEF_DIR = %x\n",GPEF_DIR);
	GPEF_DIR = GPEF_DIR | 0x01; 
	BOOT_MSG("GPEF_DIR = %x\n",GPEF_DIR);
#ifdef _V210_RELAY_R_
	GPEF_DATA = GPEF_DATA | 0x01;
#endif
#ifdef _RELAY_8186V_Z_ 
	GPEF_DATA = GPEF_DATA & 0xfffffffe;
#endif

#endif

#ifndef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
#ifdef RELAY_SW_CTRL_GPIOD //for gpioD used.pull relay high.	
	#define GPCD_DIR  *((volatile unsigned int *)0xbd010134)
	#define GPCD_DATA  *((volatile unsigned int *)0xbd010130)	
	BOOT_MSG("GPCD_DIR = %x\n",GPCD_DIR);
	GPCD_DIR = GPCD_DIR | 0x40; 		// GPIOD6
	BOOT_MSG("GPCD_DIR = %x\n",GPCD_DIR);
	GPCD_DATA = GPCD_DATA | 0x40;	
	
#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA_2_RELAY_SUPPORT
	#define GPAB_DIR *((volatile unsigned int *)0xbd010124)
	#define GPAB_DATA *((volatile unsigned int *)0xbd010120)
	BOOT_MSG("GPAB_DIR = %x\n",GPAB_DIR);
	GPAB_DIR = GPAB_DIR | 0x20; 		// GPIOA5
	BOOT_MSG("GPAB_DIR = %x\n",GPAB_DIR);
	GPAB_DATA = GPAB_DATA | 0x20;	
#endif
#endif
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA
	virtual_daa_init();	
#endif

#ifdef 	CONFIG_RTK_VOIP_LED
	LED_Init();
#endif

#ifdef  RELAY_SW_CTRL_GPIOC//for gpioC used.pull relay high.	
	#define GPCD_DIR  *((volatile unsigned int *)0xbd010134)
	#define GPCD_DATA  *((volatile unsigned int *)0xbd010130)	
	BOOT_MSG("GPCD_DIR = %x\n",GPCD_DIR);
	GPCD_DIR = GPCD_DIR | 0x01; 
	BOOT_MSG("GPCD_DIR = %x\n",GPCD_DIR);
	GPCD_DATA = GPCD_DATA & 0xfffffffe;
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
	int i;
	int ret;
#endif
	printk("\n====== RTK PCM Controller Initialization =======\n ");
	int result; 
	unsigned char ch = 0;
	char strbuf[NAME_SIZE];
	struct pcm_priv* pcm_dev;
#ifdef EXTERNEL_CLK
	unsigned long tmp;
	tmp = pcm_inl(0xbd01003c);
	BOOT_MSG("before 0xbd01003c content = %x\n", tmp);
	tmp |= 0x40000;
	pcm_outl(0xbd01003c, tmp);
	tmp = pcm_inl(0xbd01003c);
	BOOT_MSG("after 0xbd01003c content = %x\n", tmp);
	//rtl_outl(0xbd01003c, rtl_inl(0xbd01003c) | 0x40000 );
	
	//tmp = rtl_inl(0xbd01003c);
#endif

#ifdef DISABLE_PCI
	tmp = pcm_inl(0xbd01003c);
	BOOT_MSG("before PCI disabled, 0xbd01003c content = %x\n", tmp);
	tmp |= 0xC0000000;
	pcm_outl(0xbd01003c, tmp);
	tmp = pcm_inl(0xbd01003c);
	BOOT_MSG("after PCI disabled, 0xbd01003c content = %x\n", tmp);
#endif

	pcmctrl_devices = kmalloc((slic_ch_num) * sizeof(struct pcm_priv), GFP_KERNEL);
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
	int res = register_chrdev(pcmctrl_major, DEV_NAME, &pcmctrl_fops);

	if(res < 0){
		PERR("Can't register PCM controller devices.");
		return res;	
	}
#endif
	
//========================== Memory Allocated and Request for IRQ =======================//	
	if(pcmctrl_devices)
	{
		memset(pcmctrl_devices, 0, (slic_ch_num) * sizeof(struct pcm_priv));
		
		pcm_enable();	// pcm must be enable before init SLIC, or it will fail during initialization.

		for (ch = 0; ch <slic_ch_num; ch++)
		{
			sprintf(strbuf, "pcm%d", ch);
			memcpy(pcmctrl_devices[ch].name, strbuf, NAME_SIZE);
			pcm_dev = &pcmctrl_devices[ch];
			pcmdev_malloc(pcm_dev, ch);


			//====================== Set Compander =======================//
						
			switch(law)
			{
				case 0:// Linear
				pcm_outl(PCMCHCNR,0);	
				BOOT_MSG("Set linear mode, PCMCHCNR = 0x%x \n", pcm_inl(PCMCHCNR));			
				break;
				case 1:// A-law
				pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR)|BIT((8*(MAXCH-ch-1)+3))|BIT((8*(MAXCH-ch-1)+2))); 
				BOOT_MSG("Set A-law, PCMCHCNR = 0x%x \n", pcm_inl(PCMCHCNR));
				break;
				
				case 2:// U-law
				pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR)|BIT((8*(MAXCH-ch-1)+3)));
				BOOT_MSG("Set U-law, PCMCHCNR = 0x%x \n", pcm_inl(PCMCHCNR));
				break;
			}
			
		}
		
		init_var();
		
                //=================== SLIC Initialization ==============//
#ifdef _Si321x_	
		init_spi(0);
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
		init_spi(1);
#endif
                for (ch = 0; ch <slic_ch_num; ch++)
		{
			proslic_init_all(ch, law, ch+1);
			//proslic_init_all(0, law, ch+2);// (pcm_CH, law, slic_order)
			//SLIC_Choice(ch+1);		
		}
			
		//SLIC_CHANNEL_CHANGE(1,0);
		//printk("RTL8186V_bonding_check()=%d\n",RTL8186V_bonding_check());
#endif	

#ifdef _LEGERITY_
		init_spi(0);
		
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651_T
		Legerity_GPIO_dir_set(1,1,1);
		Legerity_GPIO_dir_set(2,1,1);
		Legerity_GPIO_data(1,1,0);
		mdelay(50);
		Legerity_GPIO_data(2,1,0);		
#endif		
		
                for (ch = 0; ch <slic_ch_num; ch++)
		{
			printk("===================Legerity slic %d===================\n",ch+1);
			Legerity_slic_init_all(ch);		
			Legerity_system_state(ch,0,1);
		}
#endif

#ifdef _Winbond_
		init_spi(0);
		printk("================== Winbond slic init... ===================\n");
		SlicInit();
#endif		
		//channel 1 compander enable
		BOOT_MSG("set channel 1 compander\n");
		pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR)|BIT((8*(MAXCH-2)+3))); //chiminer modify
		BOOT_MSG("Set U-law, PCMCHCNR = 0x%x \n", pcm_inl(PCMCHCNR)); //chiminer modify

                //=================== DAA Initialization ==============//
#ifndef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA
#ifdef CONFIG_RTK_VOIP_DRIVERS_DAA_SI3050

			//setting tx,rx buffer for DAA
                        //ch = slic_ch_num+DAA_DEVS-1;//the element of pcmctrl_devices[]
			//sprintf(strbuf, "pcm%d", ch);
			//memcpy(pcmctrl_devices[ch].name, strbuf, NAME_SIZE);
			//pcm_dev = &pcmctrl_devices[ch];
			//pcmdev_malloc(pcm_dev, 2);//channel 2 for DAA


			/***** Set Compander *****/
			ch = 2;	//channel 2 for DAA		
			switch(law)
			{
				case 0:// Linear
				pcm_outl(PCMCHCNR,0);	
				BOOT_MSG("Set linear mode, PCMCHCNR = 0x%x \n", pcm_inl(PCMCHCNR));			
				break;
				case 1:// A-law
				pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR)|BIT((8*(MAXCH-ch-1)+3))|BIT((8*(MAXCH-ch-1)+2))); 
				BOOT_MSG("Set A-law, PCMCHCNR = 0x%x \n", pcm_inl(PCMCHCNR));
				break;
				
				case 2:// U-law
				pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR)|BIT((8*(MAXCH-ch-1)+3)));
				BOOT_MSG("Set U-law, PCMCHCNR = 0x%x \n", pcm_inl(PCMCHCNR));
				break;
			}
		
			//pcm_outl(PCMIMR, pcm_inl(PCMIMR)|BIT(7)|BIT(6)|BIT(5)|BIT(4));//enable TOK,ROK,TU and RU of ch2
			
			init_spi(1); //for DAA spi mode init.

			daa_init_all(0);
	
			//pcm_outl(PCMTSR,0x00010203);
			//pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR)|BIT(8)|BIT(9));//enable tx and rx of ch2
		#if 0
			//pcm_outl(PCMTSR,0x00010204);
			pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR)|BIT(24)|BIT(25));//enable tx and rx of ch0
			pcm_outl(PCMCR,pcm_inl(PCMCR)|0x02);
			DAA_Tx_Gain_ctrl(1,3,0,0);
			DAA_Rx_Gain_ctrl(1,6,0,0);
			//going_off_hook();
			on_hook_daa();
		#endif
#endif
#endif	

//===================IP_phone Initialization======================//
#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
		//init WM8510
		init_i2c_gpio();
		WM8510_init();
		//init LED	
		init_led_74hc164_gpio();
		//init LCD module
		LCM_Software_Init();
		Write_string_to_LCM("Startup...",0,0,1);
#endif
	
		result = 0;
	}
	else
	{
		printk("Allocate Memory failed. \n");
		result = -ENOMEM;
	}
	
//******************PCM time slot setup***********//
	pcm_channel_slot();  //chiminer modify	
	
#ifdef _TI_Codec_	
	codec_init(U_law);			/* Initialize Ti Codec, Linear:0 A_law: 1 U_law:2 */
#endif
                pcm_dev = pcmctrl_devices;
		result = request_irq(PCM_IRQ, pcm_interrupt, SA_INTERRUPT, pcm_dev->name, pcm_dev);// NULL OK
		if(result)
		{
			printk("Can't request IRQ for channel %d.\n", ch);
			//PERR(KERN_ERR "Can't request IRQ for channel %d.\n", ch);
			return result;
				
		}
		printk("Request IRQ for PCM Controller OK!.\n");

#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_RX)
	for( i = 0 ; i < SLIC_CH_NUM ; i++ )
	{
		toRegister[i].msTick = 10000; /* tick service is triggered every 100ms. */
		toRegister[i].msRemain = 0;
		toRegister[i].chid = i;

		toRegister[i].fpTick  = NULL; /*tick*/
		toRegister[i].fpPktRx = DSP_pktRx; /* packet recv */
		toRegister[i].fpPcmInt= pcm_ISR; /* pcm interrupt */
		//toRegister[i].fpPcmInt = NULL;
		ret = voip_register_callback( i, &toRegister[i] );
		printk("ROME driver register result ch[%d]: %d\n",i,ret);
	}
	printk("Registered into ROME voip_support\n");
#endif

#if ! defined (AUDIOCODES_VOIP)
#ifdef EVENT_POLLING_TIMER
	Init_Event_Polling_Use_Timer();
#endif
#endif

//====================memory allocation for DAA===========================//
#ifdef CONFIG_RTK_SLIC_DAA_NEGOTIATION
	ptxtmp_DAA = kmalloc(2048, GFP_KERNEL);
	if (!ptxtmp_DAA)
	{	
		printk("DAA tx memory allocated error\n");
		return;
	}
	prxtmp_DAA = kmalloc(2048, GFP_KERNEL);
	if (!prxtmp_DAA)
	{	
		printk("DAArtx memory allocated error\n");
		return;
	}
	//set all allocated buffer to 0
	memset(	ptxtmp_DAA,0,2048);
	memset(	prxtmp_DAA,0,2048);
	//set TX, Rx buffer
	pcm_outl(TX_BSA(2), (Virtual2Physical(Virtual2NonCache(ptxtmp_DAA))));	
	pcm_outl(RX_BSA(2), (Virtual2Physical(Virtual2NonCache(prxtmp_DAA))));
	
	//set virtual none cachable region
	pTxBuf_DAA = (uint32)ptxtmp_DAA | 0x20000000;
	pRxBuf_DAA = (uint32)prxtmp_DAA | 0x20000000;
	BOOT_MSG("pTxBuf_DAA=%x,pRxBuf_DAA=%x\n",pTxBuf_DAA,pRxBuf_DAA);
	//set page size
	pcm_set_page_size(2, PCM_SAMPLE_DAA*2*PCM_PERIOD);
	//enable ch2
	EnaPcmIntr(2);
	int i;
	for(i=0; i<2; i++)
	{
		pcm_set_tx_own_bit(2, i);
		pcm_set_rx_own_bit(2, i);
	}
	pcm_tx_rx_enable(2);
	pcm_clean_isr(0xFFFFFFFF);
	
#endif	

#ifdef SUPPORT_SLIC_GAIN_CFG
	for (chid=0; chid< SLIC_CH_NUM; chid++)
	{
		g_txVolumneGain[chid] = 6;// init value: 0dB
 		g_rxVolumneGain[chid] = 6;// init value: 0dB	
	}
#endif
	
	print_pcm();

	printk("================= FISISH ===============\n ");
	
	return result;
		
}

void __exit pcmctrl_cleanup(void)
{
	pcm_disable();
	kfree(pcmctrl_devices);
#ifdef CONFIG_RTK_SLIC_DAA_NEGOTIATION
	kfree(ptxtmp_DAA);
	kfree(prxtmp_DAA);
#endif	
}

void print_pcm(void)	// thlin+ for debug
{
	BOOT_MSG("PCMCR= 0x%x\n", pcm_inl(PCMCR));
	BOOT_MSG("PCMCHCNR= 0x%x\n", pcm_inl(PCMCHCNR));
	BOOT_MSG("PCMBSIZE= 0x%x\n", pcm_inl(PCMBSIZE));
	BOOT_MSG("CH0TXBSA= 0x%x\n", pcm_inl(CH0TXBSA));
	BOOT_MSG("CH0RXBSA= 0x%x\n", pcm_inl(CH0RXBSA));
	BOOT_MSG("CH1TXBSA= 0x%x\n", pcm_inl(CH1TXBSA));
	BOOT_MSG("CH1RXBSA= 0x%x\n", pcm_inl(CH1RXBSA));
	BOOT_MSG("CH2TXBSA= 0x%x\n", pcm_inl(CH2TXBSA));
	BOOT_MSG("CH2RXBSA= 0x%x\n", pcm_inl(CH2RXBSA));
	BOOT_MSG("PCMTSR= 0x%x\n", pcm_inl(PCMTSR));
	BOOT_MSG("PCMIMR= 0x%x\n", pcm_inl(PCMIMR));
	BOOT_MSG("PCMISR= 0x%x\n", pcm_inl(PCMISR));	
}



void PCM_init(void)
{
        unsigned long flags;
#if defined (AUDIOCODES_VOTING_MECHANISM)
	int nPeriod, reg_pbsize, i;
#endif
        save_flags(flags); cli();

        uint32 chid;
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8671
	/*enable pcm interrupt of GIMR */
	//pcm_outw(0xb9c03010,pcm_inw(0xb9c03010)|0x10);
	//printk("GIMR=%x\n",pcm_inw(0xb9c03010));
	/*enable pcm module of rtl8671 and set gpio for mimic-spi used not JTAG*/
	pcm_outl(0xb9c04000,(pcm_inl(0xb9c04000)&0xffffffef)|0x4000);
	BOOT_MSG("sys config.=%x\n",pcm_inl(0xb9c04000));
#endif

        //init_var();     // can get gloable tx rx base address
        for(chid=0; chid<MAX_SLIC_CH_NUM; chid++)
        {
#if ! defined (AUDIOCODES_VOTING_MECHANISM)   
                pcm_rx_disable(chid);
                pcm_tx_disable(chid);
#else
		pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR) & (~(AC_PCM_CHCNR_CTLWRD_RX(chid))));
		pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR) & (~(AC_PCM_CHCNR_CTLWRD_TX(chid))));
#endif

        }

#ifdef SUPPORT_AES_ISR
	Init_AES();
	for (chid=0; chid < SLIC_CH_NUM; chid++)
	{
		AES_ON(chid);
	}
#endif

#ifdef PCM_HANDLER_USE_TASKLET
	tasklet_init(&pcm_handler_tasklet, PCM_handler_2, NULL);
	BOOT_MSG("=== PCM Handler Tasklet Initialization ===\n");
#endif	

        restore_flags(flags);

        //print_pcm();
        //return;
}

void PCM_shutdown(void)
{
	uint32 chid;

	for (chid=0; chid<SLIC_CH_NUM; chid++)
	{
		pcm_disableChan(chid);
	}
	
#ifdef PCM_HANDLER_USE_TASKLET
	tasklet_kill(&pcm_handler_tasklet);
#endif	
	
}

#if !defined (AUDIOCODES_VOTING_MECHANISM)
void pcm_enableChan(unsigned int chid)
{
	int reg_pbsize;
	Word16 nPeriod;
	int		i;

	if(SLIC_CH_NUM > MAX_SLIC_CH_NUM)
		return;
	// chan id from 0 ~ (SLIC_CH_NUM - 1)
	if(chid >= SLIC_CH_NUM)
		return;
	if(chanEnabled[chid] == TRUE)
		return;

	txpage[chid] = 0;
	rxpage[chid] = 0;
#ifdef LEC_USE_CIRC_BUF	
	//LEC_buf_windex[chid] =SYNC_POINT;
	LEC_buf_windex[chid] =2;    /* initial value=2, tested in 8186 and 8651B */
	LEC_buf_rindex[chid] =0;
#endif
#ifdef PCM_PERIOD
	nPeriod = 10*PCM_PERIOD; // 10ms * PCM_PERIOD
#else	
#ifdef SUPPORT_CUT_DSP_PROCESS
	nPeriod = 10; //10ms
#else
        extern Word16 nPcmIntPeriod[];
	nPeriod = nPcmIntPeriod[chid];
#endif
#endif
	reg_pbsize = nPeriod * 8 * 2;		// 8 samples/ms * 2 bytes/sample
	pcm_set_page_size(chid, reg_pbsize);
	//pcm_get_page_size(chid);
#if 1
	for(i=0; i<2; i++)
	{
		pcm_set_tx_own_bit(chid, i);
		pcm_set_rx_own_bit(chid, i);
	}
#endif
#ifdef SUPPORT_PCM_FIFO
	PCM_FIFO_Init(chid);
#endif	
	chanEnabled[chid] = TRUE;
	pcm_tx_rx_enable(chid);
	EnaPcmIntr(chid);

}
#else //defined (AUDIOCODES_VOTING_MECHANISM)



void pcm_enableChan(unsigned int chid)
{
	unsigned long ac_isr = 0,ac_imr = 0,ac_chnr=0,j,voting;
	int reg_pbsize;
	Word16 nPeriod;
	int		i;

	if(SLIC_CH_NUM > MAX_SLIC_CH_NUM)
		return;
	// chan id from 0 ~ (SLIC_CH_NUM - 1)
	if(chid >= SLIC_CH_NUM)
		return;

	if(chanEnabled[chid] == TRUE)
		return;

#ifdef SUPPORT_PCM_FIFO
	PCM_FIFO_Init(chid);
#endif	
	

	chanEnabled[chid] = TRUE;
	voting = AC_PCM_VOTING_GET();
	AC_PCM_VOTING_CH_ON(chid);	
	if (voting != 0) return;

	nPeriod = 10*PCM_PERIOD; // 10ms * PCM_PERIOD
	reg_pbsize = nPeriod * 8 * 2;		// 8 samples/ms * 2 bytes/sample
	for(j=0; j<SLIC_CH_NUM; j++)
	{
		pcm_set_page_size(j, reg_pbsize);
		for(i=0; i<2; i++)
		{
			pcm_set_tx_own_bit(j, i);
			pcm_set_rx_own_bit(j, i);
		}
		txpage[j] = 0;
		rxpage[j] = 0;
	}

	
	for(i=0; i<SLIC_CH_NUM; i++) 
	{
		ac_chnr |= AC_PCM_CHCNR_CTLWRD_RX_TX(i);
		ac_imr |=(	AC_PCM_IMR_CTLWRD_P0OK(i)|
				AC_PCM_IMR_CTLWRD_P1OK(i)|
				AC_PCM_IMR_CTLWRD_TBUA(i)|
				AC_PCM_IMR_CTLWRD_RBUA(i));
		ac_isr |=AC_PCM_ISR_CTLWRD_RESET_CH(i);
	}
	//pcm_tx_rx_enable
	AC_PCM_CHCNR_WRITE(AC_PCM_CHCNR_READ()|(ac_chnr));

	//EnaPcmIntr
	AC_PCM_ISR_WRITE(ac_isr);
	AC_PCM_IMR_WRITE(AC_PCM_IMR_READ()|(ac_imr));

}
#endif

#if !defined (AUDIOCODES_VOTING_MECHANISM)
void pcm_disableChan(unsigned int chid)
{
	DisPcmIntr(chid);
	if(chanEnabled[chid] == FALSE)
		return;

	if(SLIC_CH_NUM > MAX_SLIC_CH_NUM)
		return;
	// chan id from 0 ~ (SLIC_CH_NUM - 1)
	if(chid >= SLIC_CH_NUM)
		return;

	pcm_rx_disable(chid);
	pcm_tx_disable(chid);

	chanEnabled[chid] = FALSE;
}
#else
void pcm_disableChan(unsigned int chid)
{
	unsigned long ac_isr = 0,ac_imr = 0,ac_chnr=0,i,j;

	if (AC_PCM_VOTING_GET() == 0) 
	{
		return; //already all off;
	}

	AC_PCM_VOTING_CH_OFF(chid);

	if (AC_PCM_VOTING_GET() == 0)
	{
		ac_imr = AC_PCM_IMR_READ();
		for(i=0; i<SLIC_CH_NUM; i++) 
		{
			ac_imr |=(	AC_PCM_IMR_CTLWRD_P0OK(i)|
						AC_PCM_IMR_CTLWRD_P1OK(i)|
						AC_PCM_IMR_CTLWRD_TBUA(i)|
						AC_PCM_IMR_CTLWRD_RBUA(i));
			ac_isr |=AC_PCM_ISR_CTLWRD_RESET_CH(i);
		}
		AC_PCM_IMR_WRITE(AC_PCM_IMR_READ()&(~(ac_imr)));
		AC_PCM_ISR_WRITE(ac_isr);
	}

	if (AC_PCM_VOTING_GET() == 0)
	{
		for(i=0; i<SLIC_CH_NUM; i++) ac_chnr |= AC_PCM_CHCNR_CTLWRD_RX(i);
		AC_PCM_CHCNR_WRITE(AC_PCM_CHCNR_READ()&(~(ac_chnr)));			
		ac_chnr = 0;
		for(i=0; i<SLIC_CH_NUM; i++) ac_chnr |= AC_PCM_CHCNR_CTLWRD_TX(i);		
		AC_PCM_CHCNR_WRITE(AC_PCM_CHCNR_READ()&(~(ac_chnr)));			
	}
	chanEnabled[chid] = FALSE;
}
#endif


void PCM_restart(unsigned int chid)
{
#if ! defined (AUDIOCODES_VOTING_MECHANISM)
	txpage[chid] = 0;
	rxpage[chid] = 0;
#endif	
	tr_cnt[chid] = 0;
	pcm_enableChan(chid);

}


#ifndef SUPPORT_PCM_FIFO
unsigned char* pcm_get_rx_buf_addr(unsigned int chid)
{

	return (unsigned char*)RxBufTmp[chid];
}

unsigned char* pcm_get_tx_buf_addr(unsigned int chid)
{	
	return (unsigned char*)TxBufTmp[chid];	
}
#endif

#ifndef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA
#ifdef CONFIG_RTK_VOIP_DRIVERS_DAA_SI3050
int voip_mgr_daa_off_hook(char chid) 
{
	int daa_off_res, daa_check=0, k;
	
	for (k=0; k<SLIC_CH_NUM; k++)
			daa_check += g_DAA_used[k];
	
	if (chid == 0)
	{
		if (daa_check == DAA_NOT_USE)
		{
			printk("daa off-hook\n");
			pcm_set_tx_mute(chid, TRUE); /* thlin: mute PCM Tx to avoid listening the click noise when daa on/off-hook switching  */
			daa_off_res = going_off_hook();
			mdelay(250);
			pcm_set_tx_mute(chid, FALSE); /* Un-mute PCM Tx */
		}
		else
		{
			printk("daa is used!\n");
			daa_off_res = 0xff;
		}
		     	
#ifndef CONFIG_RTK_SLIC_DAA_NEGOTIATION
		if (daa_off_res == 1)
		{
			pcm_outl(PCMTSR, 0x00010203);  //set time slot
			pcm_outl(PCMIMR, pcm_inl(PCMIMR)&0x0F0F);  //mask ch0 and ch2 interrupt
			pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR)|BIT(8)|BIT(9)|BIT(11)|BIT(24)|BIT(25)|BIT(27));//enable compander, tx and rx of ch0 and ch2
			pcm_outl(PCMCR, pcm_inl(PCMCR)|0x02);	//connect channel 0, 2 together
		}
#endif
		     	
		
		//for (i=0;i<5000000000;i++);	//for testing
		#ifndef SPLIT_DAA_ON_OFF_HOOK
		while ((readDirectReg(68)&0x01));  //put the hook down
		on_hook_daa();
		printk("daa on-hook\n");
		pcm_outl(PCMCR,pcm_inl(PCMCR)& 0xfffc);	//dis-connect channel together
		#endif
	}
	else if (chid == 1)
	{
		if (daa_check == DAA_NOT_USE)
		{	
			printk("daa off-hook\n");
			pcm_set_tx_mute(chid, TRUE); /* thlin: mute PCM Tx to avoid listening the click noise when daa on/off-hook switching  */
			daa_off_res =going_off_hook();
			mdelay(250);
			pcm_set_tx_mute(chid, FALSE); /* Un-mute PCM Tx */
		}
		else
		{
			printk("daa is used!\n");
			daa_off_res = 0xff;
		}

#ifndef CONFIG_RTK_SLIC_DAA_NEGOTIATION
	
		if (daa_off_res == 1)
		{
			pcm_outl(PCMTSR, 0x00010203);  //set time slot
			pcm_outl(PCMIMR, pcm_inl(PCMIMR)|0xF00F);  //mask ch1 and ch2 interrupt
			pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR)|BIT(8)|BIT(9)|BIT(11)|BIT(16)|BIT(17)|BIT(19));//enable compander, tx and rx of ch1 and ch2
			pcm_outl(PCMCR, pcm_inl(PCMCR)|0x04);	//connect channel 1, 2 together
		}
#endif


	}
	else
	{
		printk("** ERROR **: Switch to DAA(PSTN) is not support now for pcm channel %d\n", chid);
		//printk("** ERROR **: at voip_mgr_daa_off_hook\n");
		daa_off_res = 0xff;
	}
	
	if (daa_off_res == 1)
		g_DAA_used[chid] = DAA_USE;

	//printk("******* g_DAA_used[%d] = %d ********\n", chid, g_DAA_used[chid]);
		
	return daa_off_res; /* 1: success, 0xff: line not connect or busy or not support */
}

void voip_mgr_daa_on_hook(char chid) 
{
	if (chid == 0)
	{
		on_hook_daa();
		printk("daa on-hook\n");
		pcm_outl(PCMCR,pcm_inl(PCMCR)& 0xfffc);	//dis-connect channel together
	}
	else if (chid == 1)
	{
		//printk("** ERROR **: Switch to DAA(PSTN) is not support now for pcm channel %d\n", chid);
		on_hook_daa();
		printk("daa on-hook\n");
		pcm_outl(PCMCR,pcm_inl(PCMCR)& 0xfff0);	//dis-connect channel together
	}
	else
	{
		printk("** ERROR **: Switch to DAA(PSTN) is not support now for pcm channel %d\n", chid);
		//printk("** ERROR **: at voip_mgr_daa_on_hook\n");
	}
	
	g_DAA_used[chid] = DAA_NOT_USE;
	
	return;
}
	
#endif
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA

#include "virtual_daa.h"
extern char relay_2_PSTN_flag[];	/* 1: relay to PSTN , 0: relay to SLIC */

int voip_mgr_daa_off_hook(char chid) 
{
	char res, relay_ID;	/* relay_id should match to ch_id. i.e. each pcm channel can relay to PSTN. */
	relay_ID = chid;
	/* relay switch to PSTN */
	res = virtual_daa_relay_switch(relay_ID, RELAY_PSTN);

	/* set the global flag to indicate that CHx is switch to PSTN (for phone hook polling used)*/
	if (res == RELAY_SUCCESS)
	{
		relay_2_PSTN_flag[relay_ID] = 1;
		g_DAA_used[chid] = DAA_USE;
	}
		
	return 1;
}

void voip_mgr_daa_on_hook(char chid) 
{
	char res, relay_ID;
	relay_ID = chid;
	/* relay switch to SLIC */
	res = virtual_daa_relay_switch(relay_ID, RELAY_SLIC);
	
	/* set the global flag to indicate that CHx is switch to SLIC (for phone hook polling used)*/
	if (res == RELAY_SUCCESS)
	{
		relay_2_PSTN_flag[relay_ID] = 0;
		g_DAA_used[chid] = DAA_NOT_USE;
	}
}

#endif

void pcm_channel_slot(void)
{
	//default ch0:slot0 ,ch1:slot1 ,ch2:slot2 ,ch3:slot3.	
	pcm_outl(PCMTSR,0x00010203);  //set time slot	
	return;
}	


#ifdef SUPPORT_PCM_FIFO

#ifndef PCM_PERIOD
/*
 * read one-page data from RX FIFO. (Deprecated! Use pcm_get_read_rx_fifo_addr() instead.)
 */
int pcm_read_rx_fifo(unsigned int chid, void* dst)
{	
	int page_size;
	if ( rx_fifo_cnt_w[chid] == rx_fifo_cnt_r[chid] ) {
		// Empty
		return -1;
	}
	page_size = pcm_get_page_size(chid);
        //memcpy(dst, &rx_fifo[chid][((page_size * rx_fifo_cnt_r[chid])>>2)], page_size);
        memcpy(dst, rx_fifo[chid][rx_fifo_cnt_r[chid]], page_size);
        rx_fifo_cnt_r[chid] = (rx_fifo_cnt_r[chid] +1 )% PCM_FIFO_SIZE;             
	return 0;
}
#endif

#ifdef PCM_PERIOD
int pcm_write_rx_fifo(unsigned int chid, void* src)
{
	int i;
	for (i=0; i < PCM_PERIOD; i++) {
		if (((rx_fifo_cnt_w[chid]+1)%PCM_FIFO_SIZE) == rx_fifo_cnt_r[chid]) {
			// Full
			return -(i+1);
		}
		// 10ms pcm data -> 160 bytes
                //memcpy(&rx_fifo[chid][((PCM_PERIOD_10MS_SIZE * rx_fifo_cnt_w[chid])>>2)], src+(i*PCM_PERIOD_10MS_SIZE),  PCM_PERIOD_10MS_SIZE);
                memcpy(rx_fifo[chid][rx_fifo_cnt_w[chid]], src+(i*PCM_PERIOD_10MS_SIZE),  PCM_PERIOD_10MS_SIZE);
	        rx_fifo_cnt_w[chid] = NEXT_FIFO_ENTRY(rx_fifo_cnt_w[chid]);
        }
	return 0;
}
#else
int pcm_write_rx_fifo(unsigned int chid, void* src)
{
	int page_size;
	if (((rx_fifo_cnt_w[chid]+1)%PCM_FIFO_SIZE) == rx_fifo_cnt_r[chid]) {
		// Full
		return -1;
	}
	page_size = pcm_get_page_size(chid);
        //memcpy(&rx_fifo[chid][((page_size * rx_fifo_cnt_w[chid])>>2)], src,  page_size);
        memcpy(rx_fifo[chid][rx_fifo_cnt_w[chid]], src,  page_size);
        rx_fifo_cnt_w[chid] = (rx_fifo_cnt_w[chid] +1)% PCM_FIFO_SIZE;              
	return 0;
}
#endif /* PCM_PERIOD */


#ifdef PCM_PERIOD
int pcm_read_tx_fifo(unsigned int chid, void* dst)
{	
	int i, ii, fi;
	
    
    #ifdef SUPPORT_LEC_G168_ISR	
	
	/* shift the LEC_RinBuf to one left frame */
#if 1
	for (fi =0; fi < FSIZE-1; fi++)	
    	{
    		memcpy(&LEC_RinBuf[chid][80*PCM_PERIOD*fi], &LEC_RinBuf[chid][80*PCM_PERIOD*(fi+1)], 80*PCM_PERIOD*2 /*byte*/);	
    	}
#else
    	memmove(&LEC_RinBuf[chid][0], &LEC_RinBuf[chid][80*PCM_PERIOD*1], 80*PCM_PERIOD*2*(FSIZE-1) /*byte*/);
#endif
    	
    #endif
    	
	for (i=0; i < PCM_PERIOD; i++) 
	{
		if ( tx_fifo_cnt_w[chid] == tx_fifo_cnt_r[chid] ) {
			// Empty
			return -(i+1);
		}
		
    #ifndef LEC_G168_ISR_SYNC_P
#if defined (AUDIOCODES_VOIP)
                ACMWModemTx(chid, tx_fifo[chid][tx_fifo_cnt_r[chid]]);
#endif		
                memcpy(dst+(i*PCM_PERIOD_10MS_SIZE), tx_fifo[chid][tx_fifo_cnt_r[chid]], PCM_PERIOD_10MS_SIZE);

	#ifdef CONFIG_RTK_SLIC_DAA_NEGOTIATION		
		if (DAA_ISR_FLOW[chid] == DRIVER_FLOW_PSTN_FORWARD)
			memset(dst+(i*PCM_PERIOD_10MS_SIZE), 0, PCM_PERIOD_10MS_SIZE); // mute tx pcm, so when DAA_ISR_FLOW[chid]=2, LEC is not needed.
	#endif
		if (tx_mute[chid] == 1)
			memset(dst+(i*PCM_PERIOD_10MS_SIZE), 0, PCM_PERIOD_10MS_SIZE);
			
    #else
		if (cnt_time < 200)
		    	memset(dst+(i*PCM_PERIOD_10MS_SIZE), 0, PCM_PERIOD_10MS_SIZE);
		else if (cnt_time >= 200 && cnt_time < 250)
		{
 			 		   	
 			for (ii=0; ii < PCM_PERIOD_10MS_SIZE; ii++)
 			{
 				//*((short*)(dst+(i*PCM_PERIOD_10MS_SIZE)+ii)) = vpat[ii%16];
				memset(dst+(i*PCM_PERIOD_10MS_SIZE)+ii, vpat[ii%16], 2);
 			}
			sync_start=1; 		
 		}
 		else
    			memset(dst+(i*PCM_PERIOD_10MS_SIZE), 0, PCM_PERIOD_10MS_SIZE);
    #endif
		
    
    #ifdef SUPPORT_LEC_G168_ISR
        #ifndef LEC_USE_CIRC_BUF
		/* update the tx pcm data to LEC_RinBuf */
		memcpy(&LEC_RinBuf[chid][80*PCM_PERIOD*(FSIZE-1) + i*PCM_PERIOD_10MS_SIZE/2], 
                        tx_fifo[chid][tx_fifo_cnt_r[chid]], PCM_PERIOD_10MS_SIZE);
        #else
		memcpy(&LEC_Rin_CircBuf[chid][80*PCM_PERIOD*LEC_buf_windex[chid] + i*PCM_PERIOD_10MS_SIZE/2 + SYNC_SAMPLE], 
                       	tx_fifo[chid][tx_fifo_cnt_r[chid]], PCM_PERIOD_10MS_SIZE);
                if( (LEC_buf_windex[chid]==(FSIZE-1)) && (i==(PCM_PERIOD-1)) )
			memcpy(&LEC_Rin_CircBuf[chid][0], &LEC_Rin_CircBuf[chid][80*PCM_PERIOD*(FSIZE)], SYNC_SAMPLE*2);
        #endif
    #endif
    
	        tx_fifo_cnt_r[chid] = (tx_fifo_cnt_r[chid] +1 )% PCM_FIFO_SIZE;             
	        
	}

#ifdef LEC_G168_ISR_SYNC_P
	
	cnt_time++;
	
	if (cnt_time%10 == 0)
		printk("%d ",cnt_time);
	
	if (sync_start ==1 && sync ==0)
		sync_frame++;
#endif	
	return 0;
}
#else
int pcm_read_tx_fifo(unsigned int chid, void* dst)
{	
	int page_size;
	if ( tx_fifo_cnt_w[chid] == tx_fifo_cnt_r[chid] ) {
		// Empty
		return -1;
	}
	page_size = pcm_get_page_size(chid);
        memcpy(dst, tx_fifo[chid][tx_fifo_cnt_r[chid])], page_size);
        tx_fifo_cnt_r[chid] = (tx_fifo_cnt_r[chid] +1 )% PCM_FIFO_SIZE;             
	return 0;
}
#endif

int pcm_write_tx_fifo(unsigned int chid, void* src)
{
	int page_size;
	if (((tx_fifo_cnt_w[chid]+1)%PCM_FIFO_SIZE) == tx_fifo_cnt_r[chid]) {
		// Full
		return -1;
	}
#ifdef PCM_PERIOD
	page_size = PCM_PERIOD_10MS_SIZE;
#else
	page_size = pcm_get_page_size(chid);
#endif	
        memcpy(tx_fifo[chid][tx_fifo_cnt_w[chid]], src,  page_size);
        tx_fifo_cnt_w[chid] = (tx_fifo_cnt_w[chid] +1)% PCM_FIFO_SIZE;              
	return 0;
}


int pcm_get_read_rx_fifo_addr(unsigned int chid, void** addr)
{
	int page_size;
	if ( rx_fifo_cnt_w[chid] == rx_fifo_cnt_r[chid] ) {
		// Empty
		return -1;
	}

#ifdef DTMF_REMOVAL_FORWARD	/* 
				 * Forward remove DTMF_REMOVAL_FORWARD_SIZE*10 ms. 
				 * The larger, DTMF removal more clean, but longer delay.
				 */
	if (dtmf_removal_flag[chid] == 1) // RFC2833, SIP_INFO 
	{ 
		if (pcm_get_rx_PCM_FIFO_SIZE(chid) > DTMF_REMOVAL_FORWARD_SIZE) 
		{				
		#ifdef PCM_PERIOD
			page_size = PCM_PERIOD_10MS_SIZE;
		#else
			page_size = pcm_get_page_size(chid);
		#endif	
                        *addr = rx_fifo[chid][rx_fifo_cnt_r[chid]];
				
			return 0;
		}
		else
			return -1;
	}

#endif //DTMF_REMOVAL_FORWARD

#ifdef PCM_PERIOD
	page_size = PCM_PERIOD_10MS_SIZE;
#else
	page_size = pcm_get_page_size(chid);
#endif	
        *addr = rx_fifo[chid][rx_fifo_cnt_r[chid]];
	return 0;
}

int pcm_read_rx_fifo_done(unsigned int chid)
{
	if ( rx_fifo_cnt_w[chid] == rx_fifo_cnt_r[chid] ) {
		// Empty
		return -1;
	}
        rx_fifo_cnt_r[chid] = (rx_fifo_cnt_r[chid] +1 )% PCM_FIFO_SIZE;             
	return 0;
}


int pcm_get_write_tx_fifo_addr(unsigned int chid, void** addr)
{
	int page_size;
	if (((tx_fifo_cnt_w[chid]+1)%PCM_FIFO_SIZE) == tx_fifo_cnt_r[chid]) {
		// Full
		return -1;
	}
#ifdef PCM_PERIOD
	page_size = PCM_PERIOD_10MS_SIZE;
#else
	page_size = pcm_get_page_size(chid);
#endif	
        *addr = tx_fifo[chid][tx_fifo_cnt_w[chid]];
	return 0;
}

int pcm_write_tx_fifo_done(unsigned int chid)
{
	if (((tx_fifo_cnt_w[chid]+1)%PCM_FIFO_SIZE) == tx_fifo_cnt_r[chid]) {
		// Full
		return -1;
	}
        tx_fifo_cnt_w[chid] = (tx_fifo_cnt_w[chid] +1)% PCM_FIFO_SIZE;              
	return 0;
}

/*
 * return page number
 */
int pcm_get_tx_PCM_FIFO_SIZE(unsigned int chid)
{
	if (tx_fifo_cnt_w[chid] >= tx_fifo_cnt_r[chid])
		return (tx_fifo_cnt_w[chid] - tx_fifo_cnt_r[chid]);
	else
                return ((PCM_FIFO_SIZE-tx_fifo_cnt_r[chid])+ tx_fifo_cnt_w[chid]);
}

int pcm_get_rx_PCM_FIFO_SIZE(unsigned int chid)
{
	if (rx_fifo_cnt_w[chid] >= rx_fifo_cnt_r[chid])
		return (rx_fifo_cnt_w[chid] - rx_fifo_cnt_r[chid]);
	else
                return ((PCM_FIFO_SIZE-rx_fifo_cnt_r[chid])+ rx_fifo_cnt_w[chid]);
}		

#endif // SUPPORT_PCM_FIFO

/*
 *for DAA used only
 */
#ifdef CONFIG_RTK_SLIC_DAA_NEGOTIATION

int pcm_get_read_rx_fifo_addr_DAA(unsigned int chid, void** addr)
{
	int page_size;
	if ( rx_fifo_cnt_w_DAA == rx_fifo_cnt_r_DAA ) 
	{
		// Empty
		return -1;
	}


#ifdef PCM_PERIOD
	page_size = PCM_PERIOD_10MS_SIZE;
#else
	page_size = pcm_get_page_size(chid);
#endif	
        *addr = rx_fifo_DAA[rx_fifo_cnt_r_DAA];
	
	//printk("r%d\n", rx_fifo_cnt_r_DAA);
		
	return 0;
}

int pcm_read_rx_fifo_done_DAA(unsigned int chid)
{
	if ( rx_fifo_cnt_w_DAA == rx_fifo_cnt_r_DAA ) 
	{
		// Empty
		return -1;
        }
        rx_fifo_cnt_r_DAA = (rx_fifo_cnt_r_DAA +1 )% PCM_FIFO_SIZE;   
        
        //printk("r%d\n", rx_fifo_cnt_r_DAA);
                  
	return 0;
}




int pcm_get_write_tx_fifo_addr_DAA(unsigned int chid, void** addr)
{	
	int page_size;
	if (((tx_fifo_cnt_w_DAA+1)%PCM_FIFO_SIZE) == tx_fifo_cnt_r_DAA) 
	{
		// Full
		return -1;
	}
#ifdef PCM_PERIOD
	page_size = PCM_PERIOD_10MS_SIZE;
#else
	page_size = pcm_get_page_size(chid);
#endif
        *addr = tx_fifo_DAA[tx_fifo_cnt_w_DAA];
	return 0;
}

int pcm_write_tx_fifo_done_DAA(unsigned int chid)
{
	if (((tx_fifo_cnt_w_DAA+1)%PCM_FIFO_SIZE) == tx_fifo_cnt_r_DAA) 
	{
		// Full
		return -1;
	}
        tx_fifo_cnt_w_DAA = (tx_fifo_cnt_w_DAA +1)% PCM_FIFO_SIZE; 

        //printk("w%d\n", tx_fifo_cnt_w_DAA);
		
	return 0;
}



int32 PCM_RX_DAA(uint32 chid)
{
	int i, j, k, daa_check=0;
	uint32 rx_rp;
	unsigned int stmp;
	Word16 s1, s2;
	

	save_radiax_reg(); /* save radiax register value */

	rx_rp = rxpage_DAA * (40*PCM_PERIOD);	//count page0/page1 offset
 	  
	for (i=0;i<PCM_PERIOD;i++)
	{

		for (j=0; j< 40; j++)
		{
				
			stmp = pRxBuf_DAA[rx_rp+(i*PCM_PERIOD_10MS_SIZE>>2)+j];
			s1 = (short)((stmp >> 16) & 0x0000ffff);
			s2 = (short)((stmp & 0x0000ffff));

#ifdef DAA_RX_DET	// Create det_buff_daa[]
			det_buff_daa[j<<1] = (int) (s1 * tx_comp[g_rxVolumneGain_DAA-1]) >> 8;
			det_buff_daa[(j<<1)+1] = (int) (s2 * tx_comp[g_rxVolumneGain_DAA-1]) >> 8;		
#endif				
		}
#ifdef RTK_VOICE_RECORD
		if(stVoipdataget[chid].write_enable==1)
		{
			memcpy(&stVoipdataget[chid].rxbuf[stVoipdataget[chid].rx_writeindex],&pRxBuf_DAA[rx_rp+(i*PCM_PERIOD_10MS_SIZE>>2)],160);
			stVoipdataget[chid].rx_writeindex= (stVoipdataget[chid].rx_writeindex+160)%DATAGETBUFSIZE;
		}
#endif //#ifdef RTK_VOICE_RECORD

	/* FXO Energy Detection */
#ifdef ENERGY_DET_FXO
		extern int32 energy_det( int chid , short *buffer, int buflen );
		extern int energy_in(uint32 ch_id, char input);
		extern int eng_det_flag[];
		
		if (eng_det_flag[chid])
		{
			energy_in(chid, energy_det(chid, (short *)(det_buff_daa), PCM_PERIOD_10MS_SIZE/2));
		}
#endif

		
#ifdef FXO_CALLER_ID_DET
		
		if ( (auto_cid_det[chid] !=  AUTO_CID_DET_OFF) || (cid_type[chid] == CID_TYPE_DTMF) )
		{
			/* DMTF CID DET */
			dtmf_cid_det(det_buff_daa, PCM_PERIOD_10MS_SIZE, chid, DTMF_POWER_LEVEL_MINUS_32DBM);			
		}
		
		if ( (auto_cid_det[chid] !=  AUTO_CID_DET_OFF) || (cid_type[chid] != CID_TYPE_DTMF) )
		{
			/* FSK CID DET */
			cid_det_si3050(det_buff_daa, chid);
		}
#endif


#ifdef FXO_BUSY_TONE_DET
		
		for (k=0; k<SLIC_CH_NUM; k++)
			daa_check += g_DAA_used[k];
			
		if (daa_check != DAA_NOT_USE)	
			busy_tone_det(det_buff_daa);
#endif

		/*** Check if Rx FIFO full or not ***/
		// (((rx_fifo_cnt_w_DAA+1)%PCM_FIFO_SIZE) == rx_fifo_cnt_r_DAA) 
		if (((rx_fifo_cnt_w_DAA+1) == rx_fifo_cnt_r_DAA)
			||((rx_fifo_cnt_w_DAA==(PCM_FIFO_SIZE-1))&&(rx_fifo_cnt_w_DAA==0)))
		{
			// Full
			//printk("ARF ");
			continue;
		}
				
#ifdef RTK_VOICE_RECORD
		//if(stVoipdataget[chid].write_enable==1)
		if(stVoipdataget[0].write_enable&2)
		{
			//memcpy(&stVoipdataget[chid].txbuf[stVoipdataget[chid].tx_writeindex],&AES_RinBuf[80*i],160);
			memcpy(&stVoipdataget[0].txbuf[stVoipdataget[0].tx_writeindex],&AES_RinBuf[80*i],160);
			//stVoipdataget[chid].tx_writeindex= (stVoipdataget[chid].tx_writeindex+160)%DATAGETBUFSIZE;
			stVoipdataget[0].tx_writeindex= (stVoipdataget[0].tx_writeindex+160)%DATAGETBUFSIZE;

			//memcpy(&stVoipdataget[chid].rxbuf[stVoipdataget[chid].rx_writeindex],&pRxBuf_DAA[rx_rp+(i*PCM_PERIOD_10MS_SIZE>>2)],160);
			memcpy(&stVoipdataget[0].rxbuf[stVoipdataget[0].rx_writeindex],&pRxBuf_DAA[rx_rp+(i*PCM_PERIOD_10MS_SIZE>>2)],160);
			//stVoipdataget[chid].rx_writeindex= (stVoipdataget[chid].rx_writeindex + 160)%DATAGETBUFSIZE;
			stVoipdataget[0].rx_writeindex= (stVoipdataget[0].rx_writeindex + 160)%DATAGETBUFSIZE;
		}
#endif //#ifdef RTK_VOICE_RECORD
	
		if(rx_mute[chid] == 1)
                        memset(rx_fifo_DAA[rx_fifo_cnt_w_DAA], 0,  PCM_PERIOD_10MS_SIZE);
		else
		{
			for (j=0; j< 40; j++)
			{
				stmp = pRxBuf_DAA[rx_rp+(i*PCM_PERIOD_10MS_SIZE>>2)+j];
				s1 = (short)((stmp >> 16) & 0x0000ffff);
				s2 = (short)((stmp & 0x0000ffff));
#ifdef SUPPORT_BANDPASS_FOR_DAA
				s1 = daa_bandpass(s1);	//200-3400hz bandpass
				s2 = daa_bandpass(s2);	//200-3400hz bandpass
#ifndef SUPPORT_AES_FOR_DAA
				stmp = ((unsigned int)s1 << 16) + ((unsigned int)s2 & 0x0000ffff);
#endif
#endif
				
#ifdef SUPPORT_AES_FOR_DAA				
				AES(0, (Word16 *)&s1, (Word16 *) &AES_RinBuf[80*i+2*j]);
				AES(0, (Word16 *)&s2, (Word16 *) &AES_RinBuf[80*i+2*j+1]);
				stmp = ((unsigned int)s1 << 16) + ((unsigned int)s2 & 0x0000ffff);
#endif
                                rx_fifo_DAA[rx_fifo_cnt_w_DAA][j] = stmp;
			}
		}
		
#ifdef RTK_VOICE_RECORD
		//if(stVoipdataget[chid].write_enable==1)
		if(  (stVoipdataget[0].write_enable &2)
		   &&(stVoipdataget[0].write_enable &16) )
		{
			//memcpy(&stVoipdataget[chid].rxbuf[stVoipdataget[chid].rx_writeindex2],&rx_fifo_DAA[rx_fifo_cnt_w_DAA],160);
			memcpy(&stVoipdataget[0].rxbuf2[stVoipdataget[0].rx_writeindex2],&rx_fifo_DAA[rx_fifo_cnt_w_DAA],160);
			//stVoipdataget[chid].rx_writeindex2= (stVoipdataget[chid].rx_writeindex2 + 160)%DATAGETBUFSIZE;
			stVoipdataget[0].rx_writeindex2= (stVoipdataget[0].rx_writeindex2 + 160)%DATAGETBUFSIZE;
		}
#endif //#ifdef RTK_VOICE_RECORD
		
		/**** Count the Rx FIFO write index ****/
	        //rx_fifo_cnt_w_DAA = (rx_fifo_cnt_w_DAA +1)% PCM_FIFO_SIZE;  
		  rx_fifo_cnt_w_DAA = (rx_fifo_cnt_w_DAA +1);
		  if (rx_fifo_cnt_w_DAA >=PCM_FIFO_SIZE) rx_fifo_cnt_w_DAA-=PCM_FIFO_SIZE;
		
	        //printk("w%d\n", rx_fifo_cnt_w_DAA);  
		
        
	}
	load_radiax_reg();
	return SUCCESS;
}


int pcm_read_tx_fifo_DAA(unsigned int chid, void* dst)
{
	int i, j;  
	
    	
	for (i=0;i<PCM_PERIOD;i++)
	{		
		if ( tx_fifo_cnt_w_DAA == tx_fifo_cnt_r_DAA ) 
		{
			// Empty
			printk("e ");
			return -(i+1);
		}
		
		if (tx_mute[chid] == 1)
			memset(dst+(i*PCM_PERIOD_10MS_SIZE), 0, PCM_PERIOD_10MS_SIZE);
		else
                        memcpy(dst+(i*PCM_PERIOD_10MS_SIZE), tx_fifo_DAA[tx_fifo_cnt_r_DAA], PCM_PERIOD_10MS_SIZE);

		    
	        tx_fifo_cnt_r_DAA = (tx_fifo_cnt_r_DAA +1 )% PCM_FIFO_SIZE;    
	        
	        //printk("r%d\n", tx_fifo_cnt_r_DAA);         
		
	}

	return 0;
}
 
		
int32 pcm_ISR_DAA(uint32 pcm_isr)
		{
	uint32 chid, tx_isrpage, rx_isrpage, orig_isr = pcm_isr;
	static uint32 last_isr=0; 
			
	chid = 2; /* DAA used */
		
	while ( pcm_isr ) 
	{
		tx_isrpage = 2*chid + txpage_DAA;
		
		if( (pcm_isr & ChanTxPage[tx_isrpage]) )
			{
			if (tx_fifo_cnt_r_DAA == tx_fifo_cnt_w_DAA)
			{
				// TX FIFO is empty!
				//printk("ATE ");
			} 
			else
			{

				if (fifo_start_DAA == 0) 
				{
					// Let RX/TX FIFO has one extra page to avoid TX FIFO empty!
					if ( tx_fifo_cnt_w_DAA >= TX_FIFO_START_NUM) { // suppose tx_fifo_cnt_r_DAA is 0 at the begining of a call!!
						fifo_start_DAA = 1;
						printk("====tx fifo_start_DAA(%d,%d)====\n", tx_fifo_cnt_r_DAA, tx_fifo_cnt_w_DAA);
					}
				}
			
				if (fifo_start_DAA == 1) 
				{
					pcm_read_tx_fifo_DAA(chid, &pTxBuf_DAA[txpage_DAA*pcm_get_page_size(chid)>>2]);
					//printk("r ");

				}
				else 
				{	
					memset(&pTxBuf_DAA[txpage_DAA*pcm_get_page_size(chid)>>2], 0, pcm_get_page_size(chid));
					printk("0 ");
				}
			
			}	
			pcm_set_tx_own_bit(chid, txpage_DAA);
			pcm_isr &= ~ChanTxPage[tx_isrpage];
			txpage_DAA ^= 1;
		}
		
		rx_isrpage = 2*chid + rxpage_DAA;
		
		if( pcm_isr & ChanRxPage[rx_isrpage] ) 
		{	
			PCM_RX_DAA(chid);
			pcm_set_rx_own_bit(chid, rxpage_DAA);
			pcm_isr &= ~ChanRxPage[rx_isrpage];
			rxpage_DAA ^= 1;									

		}

	} // end of while
			
			
	last_isr = orig_isr;

	if (pcm_isr != 0)
		printk(" %X ", pcm_isr);


	return 0;
} 
 
#endif //CONFIG_RTK_SLIC_DAA_NEGOTIATION


void pcm_set_tx_mute(unsigned char chid, int enable)
{
	unsigned long flags;
	save_flags(flags); cli();
	
	if (enable)
		tx_mute[chid] = 1; /* mute */
	else
		tx_mute[chid] = 0; /* not mute */
	
	restore_flags(flags);
}


void pcm_set_rx_mute(unsigned char chid, int enable)
{
	unsigned long flags;
	save_flags(flags); cli();
	
	if (enable)
		rx_mute[chid] = 1; /* mute */
	else
		rx_mute[chid] = 0; /* not mute */

	restore_flags(flags);
}

void pcm_Hold_DAA(unsigned char chid)
{
	//pcm_set_tx_mute(chid, TRUE); /* should play hold tone or music */
	pcm_set_rx_mute(chid, TRUE);
}

void pcm_UnHold_DAA(unsigned char chid)
{
	//pcm_set_tx_mute(chid, FALSE);
	pcm_set_rx_mute(chid, FALSE);
}

#ifndef CONFIG_RTK_VOIP_MODULE
module_init(pcmctrl_init);
module_exit(pcmctrl_cleanup);
#endif
