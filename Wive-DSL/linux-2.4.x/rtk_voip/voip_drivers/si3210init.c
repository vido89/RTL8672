/*
* Copyright c                  Realtek Semiconductor Corporation, 2005  
* All rights reserved.
* 
* Program : Proslic Si3210 initialial process 
* Abstract : Si3210init.c will use spi.c(SPI driver) to powerup and initialize the Si3210. 
* Author :  thlin (thlin@realtek.com.tw)      
* $Id: si3210init.c,v 1.5 2008/02/28 08:56:51 eric Exp $ 
*/




#include <linux/config.h>
#include <linux/sched.h>	/* jiffies is defined */
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/errno.h>	// error codes
#include <linux/types.h>	// size_t
#include <linux/delay.h>  	// udelay
#include <linux/ioctl.h>
#include <linux/slab.h>
#include <asm/system.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include "si3210init.h"
#include "spi.h"
#include "Slic_api.h"
#include "rtk_voip.h"
#include "voip_params.h"
#include "rtl_types.h"
#include "fsk.h"

#ifndef AUDIOCODES_VOIP
#include "codec_def.h"	/* CODEC_TYPE_G7231 */
#include "codec_descriptor.h"
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
//#include "../voip_dsp/dsp_main.h"
#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8186
#include <asm/rtl8186.h>
//---chiminer 2005-12-27-----------//
//It's for SPI daisy-chain mode.It must be the same with spi.c.
#define chiminer_daisy_chain 1
#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8671
//It's for SPI daisy-chain mode.It must be the same with spi.c.
#define chiminer_daisy_chain 1
#include "../include/rtl8671.h"
#endif
//------------------------------//

#define PowerLeakTest 1	    // 1: Do powerLeakTest().  0: Not do powerLeakTest()

#define cyg_current_time()	jiffies
typedef unsigned long cyg_tick_count_t;

extern unsigned char init ; // for fskInitializaton(void)

//-
void reset_slic_parameter();
/* Si3210: The following Array contains: */
indirectRegister  indirectRegisters_3210[] =
{
/* Reg#			Label		Initial Value  */

{	0,	"DTMF_ROW_0_PEAK",	0x55C2	},
{	1,	"DTMF_ROW_1_PEAK",	0x51E6	},
{	2,	"DTMF_ROW2_PEAK",	0x4B85	},
{	3,	"DTMF_ROW3_PEAK",	0x4937	},
{	4,	"DTMF_COL1_PEAK",	0x3333	},
{	5,	"DTMF_FWD_TWIST",	0x0202	},
{	6,	"DTMF_RVS_TWIST",	0x0202	},
{	7,	"DTMF_ROW_RATIO",	0x0198	},
{	8,	"DTMF_COL_RATIO",	0x0198	},
{	9,	"DTMF_ROW_2ND_ARM",0x0611	},
{	10,	"DTMF_COL_2ND_ARM",	0x0202	},
{	11,	"DTMF_PWR_MIN_",	0x00E5	},
{	12,	"DTMF_OT_LIM_TRES",	0x0A1C	},
{	13,	"OSC1_COEF",		0x7b30	},
{	14,	"OSC1X",				0x0063	},
{	15,	"OSC1Y",				0x0000	},
{	16,	"OSC2_COEF",		0x7870	},
{	17,	"OSC2X",				0x007d	},
{	18,	"OSC2Y",				0x0000	},
{	19,	"RING_V_OFF",		0x0000	},
{	20,	"RING_OSC",			0x7EF0	},
{	21,	"RING_X",			0x0160	},
{	22,	"RING_Y",			0x0000	},
{	23,	"PULSE_ENVEL",		0x2000	},
{	24,	"PULSE_X",			0x2000	},
{	25,	"PULSE_Y",			0x0000	},
#if 1
{	26,	"RECV_DIGITAL_GAIN",	0x4000	},
{	27,	"XMIT_DIGITAL_GAIN",	0x4000	},
#else
{	26,	"RECV_DIGITAL_GAIN",	0x1000	},
{	27,	"XMIT_DIGITAL_GAIN",	0x1000	},
#endif
{	28,	"LOOP_CLOSE_TRES",	0x1000	},
{	29,	"RING_TRIP_TRES",	0x3600	},
{	30,	"COMMON_MIN_TRES",	0x1000	},
#if 1
{	31,	"COMMON_MAX_TRES",	0x0080	},
#else
{	31,	"COMMON_MAX_TRES",	0x0200	},
#endif
{	32,	"PWR_ALARM_Q1Q2",	0x07c0	},
{	33,	"PWR_ALARM_Q3Q4",	0x376f	},
{	34,	"PWR_ALARM_Q5Q6",	0x1B80	},
{	35,	"LOOP_CLSRE_FlTER",	0x8000	},
{	36,	"RING_TRIP_FILTER",	0x0320	},
{	37,	"TERM_LP_POLE_Q1Q2",0x008c	},
#if 1
{	38,	"TERM_LP_POLE_Q3Q4",0x008c	},
#else
{	38,	"TERM_LP_POLE_Q3Q4",0x0100	},
#endif
{	39,	"TERM_LP_POLE_Q5Q6",0x0010	},
#if 1
{	40,	"CM_BIAS_RINGING",	0x0200	},
#else
{	40,	"CM_BIAS_RINGING",	0x0C00	},
#endif
{	41,	"DCDC_MIN_V",		0x0C00	},
{	43,	"LOOP_CLOSE_TRES Low",0x1000},
{	99,	"FSK 0 FREQ PARAM",	0x00DA	},
{	100,	"FSK 0 AMPL PARAM",	0x6B60	},
{	101,	"FSK 1 FREQ PARAM",	0x0074	},
{	102,	"FSK 1 AMPl PARAM",	0x79C0	},
{	103,	"FSK 0to1 SCALER",	0X1120	},
{	104,	"FSK 1to0 SCALER",	0x3BE0	},
{	97,	"RCV_FLTR",				0	},
{0,"",0},
};



//------------------------------------------


/* Si3215: The following Array contains: */
indirectRegister  indirectRegisters_3215[] =
{
/* Reg#			Label		Initial Value  */

{	0,	"OSC1_COEF",		0x7b30	},
{	1,	"OSC1X",				0x0063	},
{	2,	"OSC1Y",				0x0000	},
{	3,	"OSC2_COEF",		0x7870	},
{	4,	"OSC2X",				0x007d	},
{	5,	"OSC2Y",				0x0000	},
{	6,	"RING_V_OFF",		0x0000	},
{	7,	"RING_OSC",			0x7EF0	},
{	8,	"RING_X",			0x0160	},
{	9,	"RING_Y",			0x0000	},
{	10,	"PULSE_ENVEL",		0x2000	},
{	11,	"PULSE_X",			0x2000	},
{	12,	"PULSE_Y",			0x0000	},
#if 1
{	13,	"RECV_DIGITAL_GAIN",	0x4000	},// Unit Gain
{	14,	"XMIT_DIGITAL_GAIN",	0x4000	},
//{	13,	"RECV_DIGITAL_GAIN",	0x2900	},// GNET Used
//{	14,	"XMIT_DIGITAL_GAIN",	0x3800	},
#else
{	13,	"RECV_DIGITAL_GAIN",	0x1380	},// Foxcom Suggest
{	14,	"XMIT_DIGITAL_GAIN",	0x6bb0	},
#endif
{	15,	"LOOP_CLOSE_TRES",	0x1000	},
{	16,	"RING_TRIP_TRES",	0x3600	},
{	17,	"COMMON_MIN_TRES",	0x1000	},
#if 1
{	18,	"COMMON_MAX_TRES",	0x0080	},
#else
{	18,	"COMMON_MAX_TRES",	0x0200	},
#endif
{	19,	"PWR_ALARM_Q1Q2",	0x07c0	},
{	20,	"PWR_ALARM_Q3Q4",	0x376f	},
{	21,	"PWR_ALARM_Q5Q6",	0x1B80	},
{	22,	"LOOP_CLSRE_FlTER",	0x8000	},
{	23,	"RING_TRIP_FILTER",	0x0320	},
{	24,	"TERM_LP_POLE_Q1Q2",0x008c	},
#if 1
{	25,	"TERM_LP_POLE_Q3Q4",0x008c	},
#else
{	25,	"TERM_LP_POLE_Q3Q4",0x0100	},
#endif
{	26,	"TERM_LP_POLE_Q5Q6",0x0010	},
#if 1
{	27,	"CM_BIAS_RINGING",	0x0200	},
#else
{	27,	"CM_BIAS_RINGING",	0x0C00	},
#endif
{	64,	"DCDC_MIN_V",		0x0C00	},
{	66,	"LOOP_CLOSE_TRES Low",0x1000},
{	69,	"FSK 0 FREQ PARAM",	0x00DA	},
{	70,	"FSK 0 AMPL PARAM",	0x6B60	},
{	71,	"FSK 1 FREQ PARAM",	0x0074	},
{	72,	"FSK 1 AMPl PARAM",	0x79C0	},
{	73,	"FSK 0to1 SCALER",	0X1120	},
{	74,	"FSK 1to0 SCALER",	0x3BE0	},
//{	97,	"RCV_FLTR",		0	},
{0,"",0},
};



//--------------------------------------------


/****************** COMMON TELEPHONEY TONES*********************************/
tone_struct DialTone = {  /* OSC1= 350 Hz OSC2= 440 Hz .0975 Volts -18 dBm */
	{0x7b30,0x0063,0,0,0,0,0},{0x7870,0x007d,0,0,0,0,0}
};
tone_struct ReorderTone = {	/* OSC1= 480 Hz OSC2 = 620 .0975 Volts -18 dBm */
	{0x7700,0x0089,0,0x09,0x60,0x06,0x40},{0x7120,0x00b2,0,0x09,0x60,0x06,0x40}
};
tone_struct CongestionTone = { /* OSC1= 480 Hz OSC2 = 620 .0975 Volts -18 dBM */
	{0x7700,0x0089,0,0x06,0x40,0x09,0x60},{0x7120,0x00b2,0,0x06,0x40,0x09,0x60}
};
tone_struct RingbackPBX = {	/* OSC1 = 440 Hz OSC2= 480 .0975 Volts -18 dBM */
	{0x7870,0x007d,0,0x1f,0x40,0x5d,0xc0},{0x7700,0x0089,0,0x1f,0x40,0x5d,0xc0}
};
tone_struct RingbackNormal = { /* OSC1 = 440 Hz OSC2 = 480 .0975 Volts -18 dBm */
	{0x7870,0x007d,0,0x3e,0x80,0x7d,0x00},{0x7700,0x0089,0,0x3e,0x80,0x7d,0x00}
};
tone_struct BusySignal = { /* OSC1= 480  OSC2 = 620 .0975 Voltz -18 dBm 8*/
	{0x7700,0x0089,0,0x0f,0xa0,0x0f,0xa0},{0x7120,0x00b2,0,0x0f,0xa0,0x0f,0xa0}
};

tone_struct RingbackJapan = { /* OSC1 = 400 Hz OSC2 = 435 .0975 Volts -18 dBm */
	{0x79c0,0x00e9,0,0x1f,0x40,0x3e,0x80},{0x7940,0x00f2,0,0x1f,0x40,0x3e,0x80}

};


tone_struct BusyJapan = { /* OSC1 = 400 Hz OSC2 = 435 .0975 Volts -18 dBm */
	{0x79c0,0x00e9,0,0x0f,0xa0,0x0f,0xa0},{0,0,0,0,0,0,0}

};

tone_struct JapanDialTone = { /* OSC1 = 400 Hz OSC2 = 435 .0975 Volts -18 dBm */
	{0x79c0,0x00e9,0,0,0,0,0},{0,0,0,0,0,0,0}

};


/*************************0.0 Dbm0 tones*****************************************/
tone_struct dual_tones[]= { // Define(s) Touch-Tones & Call progress tones
	// OSC 1  x     y on_h on_l			OSC 2
	{{0x5ea0,0x08a5,0,0x03,0x20,0,0}, {0x3fc0,0x0cec,0,0x03,0x20,0,0}}, // DTMF 0
	{{0x6d50,0x0643,0,0x03,0x20,0,0}, {0x4a80,0x0b79,0,0x03,0x20,0,0}}, // DTMF 1
	{{0x6d50,0x0643,0,0x03,0x20,0,0}, {0x3fc0,0x0cec,0,0x03,0x20,0,0}}, // DTMF 2
	{{0x6d50,0x0643,0,0x03,0x20,0,0}, {0x3320,0x0ea0,0,0x03,0x20,0,0}}, // DTMF 3
	{{0x6950,0x06f6,0,0x03,0x20,0,0}, {0x4a80,0x0b79,0,0x03,0x20,0,0}}, // DTMF 4
	{{0x6950,0x06f6,0,0x03,0x20,0,0}, {0x3fc0,0x0cec,0,0x03,0x20,0,0}}, // DTMF 5
	{{0x6950,0x06f6,0,0x03,0x20,0,0}, {0x3320,0x0ea0,0,0x03,0x20,0,0}}, // DTMF 6
	{{0x6460,0x07c3,0,0x03,0x20,0,0}, {0x4a80,0x0b79,0,0x03,0x20,0,0}}, // DTMF 7
	{{0x6460,0x07c3,0,0x03,0x20,0,0}, {0x3fc0,0x0cec,0,0x03,0x20,0,0}}, // DTMF 8
	{{0x6460,0x07c3,0,0x03,0x20,0,0}, {0x3320,0x0ea0,0,0x03,0x20,0,0}}, // DTMF 9	
	{{0x5ea0,0x08a5,0,0x03,0x20,0,0}, {0x4a80,0x0b79,0,0x03,0x20,0,0}}, // DTMF *
	{{0x5ea0,0x08a5,0,0x03,0x20,0,0}, {0x3320,0x0ea0,0,0x03,0x20,0,0}}, // DTMF #
	{{0x6d50,0x0643,0,0x03,0x20,0,0}, {0x2460,0x10ab,0,0x03,0x20,0,0}}, // DTMF A		
	{{0x6950,0x06f6,0,0x03,0x20,0,0}, {0x2460,0x10ab,0,0x03,0x20,0,0}}, // DTMF B
	{{0x6460,0x07c3,0,0x03,0x20,0,0}, {0x2460,0x10ab,0,0x03,0x20,0,0}}, // DTMF C
	{{0x5ea0,0x08a5,0,0x03,0x20,0,0}, {0x2460,0x10ab,0,0x03,0x20,0,0}}  // DTMF D
};



char * exceptionStrings[] = 
{ " ProSLIC not communicating", "Time out durring Power Up", "Time out durring Power Down",

"Power is Leaking; might be a short"," Tip or Ring Ground Short",

"Too Many Q1 Power Alarms" ,"Too Many Q2 Power Alarms" ,"Too Many Q3 Power Alarms" 

"Too Many Q4 Power Alarms" ,"Too Many Q5 Power Alarms" ,"Too Many Q6 Power Alarms" };


chipStruct chipData ; /* Represents a proslics state, cached information, and timers */


void exception (enum exceptions e)
/* This is where an embedded system would call its exception handler */
/* This code runs a print out routine */
{
	printk( "\n                 E X C E P T I O N: %s\n",exceptionStrings[e] );
}


void show_PCLK_freq(void)
{
	int v = readDirectReg(13);
	int freq = 9999;
	switch (v)
	{
		case 64:
			freq = 512;
			BOOT_MSG("The Frequency of PCLK(DR 13) is %d KHz\n", freq);
			break;
		case 48:
			freq =1024;
			BOOT_MSG("The Frequency of PCLK(DR 13) is %d KHz\n", freq);
			break;
		case 32:
			freq =2048;
			BOOT_MSG("The Frequency of PCLK(DR 13) is %d KHz\n", freq);
			break;
		case 16:
			freq =4096;
			BOOT_MSG("The Frequency of PCLK(DR 13) is %d KHz\n", freq);
			break;
		case 0:
			freq =8192;
			BOOT_MSG("The Frequency of PCLK(DR 13) is %d KHz\n", freq);
			break;		
		default:
			printk("The Frequency of PCLK(DR 13) Error!\n", freq);
			break;
	}
}

void SetSi321xPCM(int chid, int enable)
{
        unsigned long flags, reg_val;
        save_flags(flags); cli();
 
        reg_val = readDirectReg(1);
 
        SLIC_Choice(chid+1);
        if (enable == 1)
        {
                writeDirectReg(1, reg_val|0x20);
                //printk("Enable SLIC PCM(%d)\n", chid);
        }
        else if (enable == 0)
        {
                writeDirectReg(1, reg_val&0xDF);
                //printk("Disable SLIC PCM(%d)\n", chid);
        }
 
        restore_flags(flags);
}

void initializeDirectRegisters(int ch_indirect)
{

	#if chiminer_daisy_chain
		writeDirectReg(0,	0x80	);/*0X80    Serial Interface for daisy-chain mode*/
	#else
		writeDirectReg(0,	INIT_DR0	);/*0X00	Serial Interface */
	#endif
	writeDirectReg(1,	INIT_DR1	);/*0X28	PCM Mode */
	if (slic_order == 1) {//time slot 0.chiminer definiation
		writeDirectReg(2,	0x01	);	
		writeDirectReg(3,	INIT_DR3	);/*0x00	PCM TX Clock Slot High Byte */
		writeDirectReg(4,	0x01	);	
		writeDirectReg(5,	INIT_DR5	);/*0x00	PCM RX Clock Slot High Byte */
		BOOT_MSG("slic_order = %d\n",slic_order);
	} else {		//time slot 1.chiminer definiation
		writeDirectReg(2,	0x09	);	
		writeDirectReg(3,	INIT_DR3	);/*0x00	PCM TX Clock Slot High Byte */
		writeDirectReg(4,	0x09	);	
		writeDirectReg(5,	INIT_DR5	);/*0x00	PCM RX Clock Slot High Byte */
		BOOT_MSG("slic_order = %d\n",slic_order);
	}
	
	writeDirectReg(8,	INIT_DR8	);/*0X00	Loopbacks (digital loopback default) */
	writeDirectReg(9,	INIT_DR9	);/*0x00	Transmit and receive path gain and control */
	writeDirectReg(10,	INIT_DR10	);/*0X28	Initialization Two-wire impedance (600  and enabled) */
	writeDirectReg(11,	INIT_DR11	);/*0x33	Transhybrid Balance/Four-wire Return Loss */
	writeDirectReg(18,	INIT_DR18	);/*0xff	Normal Oper. Interrupt Register 1 (clear with 0xFF) */
	writeDirectReg(19,	INIT_DR19	);/*0xff	Normal Oper. Interrupt Register 2 (clear with 0xFF) */
	writeDirectReg(20,	INIT_DR20	);/*0xff	Normal Oper. Interrupt Register 3 (clear with 0xFF) */
	writeDirectReg(21,	INIT_DR21	);/*0xff	Interrupt Mask 1 */
	writeDirectReg(22,	INIT_DR22	);/*0xff	Initialization Interrupt Mask 2 */
	writeDirectReg(23,	INIT_DR23	);/*0xff	 Initialization Interrupt Mask 3 */
	writeDirectReg(32,	INIT_DR32	);/*0x00	Oper. Oscillator 1 Controltone generation */
	writeDirectReg(33,	INIT_DR33	);/*0x00	Oper. Oscillator 2 Controltone generation */
	writeDirectReg(34,	INIT_DR34	);/*0X18	34 0x22 0x00 Initialization Ringing Oscillator Control */
	writeDirectReg(35,	INIT_DR35	);/*0x00	Oper. Pulse Metering Oscillator Control */
	writeDirectReg(36,	INIT_DR36	);/*0x00	36 0x24 0x00 Initialization OSC1 Active Low Byte (125 탎/LSB) */
	writeDirectReg(37,	INIT_DR37	);/*0x00	37 0x25 0x00 Initialization OSC1 Active High Byte (125 탎/LSB) */
	writeDirectReg(38,	INIT_DR38	);/*0x00	38 0x26 0x00 Initialization OSC1 Inactive Low Byte (125 탎/LSB) */
	writeDirectReg(39,	INIT_DR39	);/*0x00	39 0x27 0x00 Initialization OSC1 Inactive High Byte (125 탎/LSB) */
	writeDirectReg(40,	INIT_DR40	);/*0x00	40 0x28 0x00 Initialization OSC2 Active Low Byte (125 탎/LSB) */
	writeDirectReg(41,	INIT_DR41	);/*0x00	41 0x29 0x00 Initialization OSC2 Active High Byte (125 탎/LSB) */
	writeDirectReg(42,	INIT_DR42	);/*0x00	42 0x2A 0x00 Initialization OSC2 Inactive Low Byte (125 탎/LSB) */
	writeDirectReg(43,	INIT_DR43	);/*0x00	43 0x2B 0x00 Initialization OSC2 Inactive High Byte (125 탎/LSB) */
	writeDirectReg(44,	INIT_DR44	);/*0x00	44 0x2C 0x00 Initialization Pulse Metering Active Low Byte (125 탎/LSB) */
	writeDirectReg(45,	INIT_DR45	);/*0x00	45 0x2D 0x00 Initialization Pulse Metering Active High Byte (125 탎/LSB) */
	writeDirectReg(46,	INIT_DR46	);/*0x00	46 0x2E 0x00 Initialization Pulse Metering Inactive Low Byte (125 탎/LSB) */
	writeDirectReg(47,	INIT_DR47	);/*0x00	47 0x2F 0x00 Initialization Pulse Metering Inactive High Byte (125 탎/LSB) */
	writeDirectReg(48,	INIT_DR48	);/*0X80	48 0x30 0x00 0x80 Initialization Ringing Osc. Active Timer Low Byte (2 s,125 탎/LSB) */
	writeDirectReg(49,	INIT_DR49	);/*0X3E	49 0x31 0x00 0x3E Initialization Ringing Osc. Active Timer High Byte (2 s,125 탎/LSB) */
	writeDirectReg(50,	INIT_DR50	);/*0X00	50 0x32 0x00 0x00 Initialization Ringing Osc. Inactive Timer Low Byte (4 s, 125 탎/LSB) */
	writeDirectReg(51,	INIT_DR51	);/*0X7D	51 0x33 0x00 0x7D Initialization Ringing Osc. Inactive Timer High Byte (4 s, 125 탎/LSB) */
	writeDirectReg(52,	INIT_DR52	);/*0X00	52 0x34 0x00 Normal Oper. FSK Data Bit */
	writeDirectReg(63,	INIT_DR63	);/*0X54	63 0x3F 0x54 Initialization Ringing Mode Loop Closure Debounce Interval */
	writeDirectReg(64,	INIT_DR64	);/*0x00	64 0x40 0x00 Normal Oper. Mode Byte뾭rimary control */
	writeDirectReg(65,	INIT_DR65	);/*0X61	65 0x41 0x61 Initialization External Bipolar Transistor Settings */
	writeDirectReg(66,	INIT_DR66	);/*0X03	66 0x42 0x03 Initialization Battery Control */
	writeDirectReg(67,	INIT_DR67	);/*0X1F	67 0x43 0x1F Initialization Automatic/Manual Control */
	writeDirectReg(69,	INIT_DR69	);/*0X0C	69 0x45 0x0A 0x0C Initialization Loop Closure Debounce Interval (1.25 ms/LSB) */
	writeDirectReg(70,	INIT_DR70	);/*0X0A	70 0x46 0x0A Initialization Ring Trip Debounce Interval (1.25 ms/LSB) */
	writeDirectReg(71,	INIT_DR71	);/*0X01	71 0x47 0x00 0x01 Initialization Off-Hook Loop Current Limit (20 mA + 3 mA/LSB) */
	writeDirectReg(72,	INIT_DR72	);/*0X20	72 0x48 0x20 Initialization On-Hook Voltage (open circuit voltage) = 48 V(1.5 V/LSB) */
	writeDirectReg(73,	INIT_DR73	);/*0X02	73 0x49 0x02 Initialization Common Mode VoltageVCM = 3 V(1.5 V/LSB) */
	writeDirectReg(74,	INIT_DR74	);/*0X32	74 0x4A 0x32 Initialization VBATH (ringing) = 75 V (1.5 V/LSB) */
	writeDirectReg(75,	INIT_DR75	);/*0X10	75 0x4B 0x10 Initialization VBATL (off-hook) = 24 V (TRACK = 0)(1.5 V/LSB) */
	if (chipData.type != 3)
		writeDirectReg(92,	INIT_DR92	);/*0x7f	92 0x5C 0xFF 7F Initialization DCDC Converter PWM Period (61.035 ns/LSB) */
	else
		writeDirectReg(92,	INIT_SI3210M_DR92	);/*0x7f	92 0x5C 0xFF 7F Initialization DCDC Converter PWM Period (61.035 ns/LSB) */

	writeDirectReg(93,	INIT_DR93	);/*0x14	93 0x5D 0x14 0x19 Initialization DCDC Converter Min. Off Time (61.035 ns/LSB) */
	writeDirectReg(96,	INIT_DR96	);/*0x00	96 0x60 0x1F Initialization Calibration Control Register 1(written second and starts calibration) */
	writeDirectReg(97,	INIT_DR97	);/*0X1F	97 0x61 0x1F Initialization Calibration Control Register 2(written before Register 96) */
	writeDirectReg(98,	INIT_DR98	);/*0X10	98 0x62 0x10 Informative Calibration result (see data sheet) */
	writeDirectReg(99,	INIT_DR99	);/*0X10	99 0x63 0x10 Informative Calibration result (see data sheet) */
	writeDirectReg(100,	INIT_DR100	);/*0X11	100 0x64 0x11 Informative Calibration result (see data sheet) */
	writeDirectReg(101,	INIT_DR101	);/*0X11	101 0x65 0x11 Informative Calibration result (see data sheet) */
	writeDirectReg(102,	INIT_DR102	);/*0x08	102 0x66 0x08 Informative Calibration result (see data sheet) */
	writeDirectReg(103,	INIT_DR103	);/*0x88	103 0x67 0x88 Informative Calibration result (see data sheet) */
	writeDirectReg(104,	INIT_DR104	);/*0x00	104 0x68 0x00 Informative Calibration result (see data sheet) */
	writeDirectReg(105,	INIT_DR105	);/*0x00	105 0x69 0x00 Informative Calibration result (see data sheet) */
	writeDirectReg(106,	INIT_DR106	);/*0x20	106 0x6A 0x20 Informative Calibration result (see data sheet) */
	writeDirectReg(107,	INIT_DR107	);/*0x08	107 0x6B 0x08 Informative Calibration result (see data sheet) */
	writeDirectReg(108,	INIT_DR108	);/*0xEB	108 0x63 0x00 0xEB Initialization Feature enhancement register */
}



void read_all_Direct_Reg(void)		/** Read all direct register **/
{
	int Reg;
	unsigned char Regval;
	
	for ( Reg=0; Reg<7; Reg++)
	{
		Regval = readDirectReg(Reg);
		printk("The value of direct Reg %d is 0x%x\n", Reg, Regval);
	}
	
	for ( Reg=8; Reg<12; Reg++)
	{
		Regval = readDirectReg(Reg);
		printk("The value of direct Reg %d is 0x%x\n", Reg, Regval);
	}
	
	for ( Reg=13; Reg<16; Reg++)
	{
		Regval = readDirectReg(Reg);
		printk("The value of direct Reg %d is 0x%x\n", Reg, Regval);
	}
	
	
	for ( Reg=18; Reg<25; Reg++)
	{
		Regval = readDirectReg(Reg);
		printk("The value of direct Reg %d is 0x%x\n", Reg, Regval);
	}
	
	for ( Reg=28; Reg<53; Reg++)
	{
		Regval = readDirectReg(Reg);
		printk("The value of direct Reg %d is 0x%x\n", Reg, Regval);
	}
	
	for ( Reg=63; Reg<90; Reg++)
	{
		Regval = readDirectReg(Reg);
		printk("The value of direct Reg %d is 0x%x\n", Reg, Regval);
	}
	
	for ( Reg=92; Reg<109; Reg++)
	{
		Regval = readDirectReg(Reg);
		printk("The value of direct Reg %d is 0x%x\n", Reg, Regval);
	} 
	
}



void initializeIndirectRegisters(void)
{
	unsigned char i=0;
	
#ifdef _SI3210_
	while (indirectRegisters_3210[i].initial || indirectRegisters_3210[i].address ) 
	{
		writeIndirectReg(indirectRegisters_3210[i].address, indirectRegisters_3210[i].initial);
		i++;
	}
#else
	while (indirectRegisters_3215[i].initial || indirectRegisters_3215[i].address ) 
	{
		writeIndirectReg(indirectRegisters_3215[i].address, indirectRegisters_3215[i].initial);
		i++;
	}
#endif

}

#if 0
void printIndirectRegisters(void)
{ 
	unsigned char i=0;

#ifdef _SI3210_	
	while (indirectRegisters_3210[i].initial || indirectRegisters_3210[i].address )
	{
		printk ("\n");
		printk(" %s = 0x%4.2X  should be 0x%4.2X ",indirectRegisters_3210[i].name,readDirectReg(i),indirectRegisters_3210[i].initial );
		i++;
	}
	
#else	
	while (indirectRegisters_3215[i].initial || indirectRegisters_3215[i].address )
	{
		printk ("\n");
		printk(" %s = 0x%4.2X  should be 0x%4.2X ",indirectRegisters_3215[i].name,readDirectReg(i),indirectRegisters_3215[i].initial );
		i++;
	}
#endif
}
#endif



void verifyIndirectRegisters(void)
{ 
	int passed = 1;  
	unsigned short i,j, initial;
	for (i=0; i<42; i++) {
		j=readIndirectReg((unsigned char) i);
#ifdef _SI3210_		
		initial= indirectRegisters_3210[i].initial;
#else		
		initial= indirectRegisters_3215[i].initial;
#endif		
		if ( j != initial )
		{
#ifdef _SI3210_
			//printk("\n %s  iREG %X = %X  should be %X ",indirectRegisters_3210[i].name,i,j,initial );
#else
			//printk("\n %s  iREG %X = %X  should be %X ",indirectRegisters_3215[i].name,i,j,initial );
#endif
			passed = 0;
		}	
	}

#if 0
	if (passed) 
	{ 
		printk("Initialization of Indirect Registers completed successfully.\n");
		
	} else 
	{
//		printk("\n");
		printk("Initialization of Indirect Registers UNSUCCESSFULLY.\n");
//			key();
//		exit(1);
	}
#endif
}


int powerUp(void)
{ 
	unsigned char vBat ; 
	int i=0, powerTime=0;
	
	cyg_tick_count_t initial_time = cyg_current_time();

	if (chipData.type == 3)  /* M version correction */
	{
		writeDirectReg(92,0x60);/* M version */
		writeDirectReg(93,0x38);/* M version */
	}
	else	
	{
		/* set the period of the DC-DC converter to 1/64 kHz  START OUT SLOW*/
		writeDirectReg(92, 0x7f);
		writeDirectReg(93, 0x12);
	}

	writeDirectReg(14, 0); 	 /* Engage the DC-DC converter */

	while ((vBat=readDirectReg(82)) < 0xc0)
	{ 	

		if (i++ > 2000)
		{
			printk("Time for powerup!\n");
			if ((cyg_current_time() - initial_time ) > 50) 	/* 0.5 seconds */
			{
				exception(TIMEoUTpOWERuP);			
				return FAILED;
			}  
		}
		
	}
	powerTime= cyg_current_time() - initial_time;
	if ( powerTime > 50)/* 0.5 seconds */
	{
		printk("\nWarning Power Up took %d milliseconds.\t",powerTime*10);
		printk("more than 0.5 seconds\n");
	}
 	
	writeDirectReg(93, 0x80);  /* DC-DC Calibration  */ 
	BOOT_MSG("Wait for DC-DC Calibration to complete\n");

	/* Wait for DC-DC Calibration to complete */
	while(0x80&readDirectReg(93));  // Wait for DC-DC Calibration to complete
	BOOT_MSG("power up complete!\n");
	return SUCCESS;
}
 
int powerLeakTest(void)
{ 
	unsigned char vBat ;	
	writeDirectReg(64,0);
	writeDirectReg(14, 0x10);   //DC-DC Converter Power-off control 
	//setState(POWERlEAKtEST);  /* This may be used in a future revision of the code to break up the bring-up */

	mdelay(1000);	// delay for a while(<1000ms), and then check the vBat voltage is under 6 volts or not.

	if( (vBat=readDirectReg(82)) < 0x4 )  // 6 volts
	{
	 	exception(POWERlEAK);
		return FAILED;
	}
	BOOT_MSG("Power not leaking!\n"); 
	return SUCCESS;
}

unsigned short manualCalibrate(void)
{ 
	unsigned char x,y,i,progress=0; // progress contains individual bits for the Tip and Ring Calibrations

	//Initialized DR 98 and 99 to get consistant results.
	// 98 and 99 are the results registers and the search should have same intial conditions.
	writeDirectReg(98,0x10); // This is necessary if the calibration occurs other than at reset time
	writeDirectReg(99,0x10);

	for ( i=0x1f; i>0; i--)
	{
		writeDirectReg(98,i);
		mdelay(40);
		if((readDirectReg(88)) == 0)
		{	
			progress|=1;
			x=i;
			break;
		}
		//else if( readDirectReg( 88 ) > 0 )		// Howard. 2004.10.4
			//continue ;
//		else
			//exception( Howard_test ) ;
	} // for

	for ( i=0x1f; i>0; i--)
	{
		writeDirectReg(99,i);
		mdelay(40);
		if((readDirectReg(89)) == 0)
		{
			progress|=2;
			y=i;
			break;
		}
	}//for

	return progress;
}



void calibrate(void)
{ 
	unsigned char x,y,i=0,progress=0;
	int ii;
	/* Do Flush durring powerUp and calibrate */
	writeDirectReg(21,DISABLE_ALL_DR21);/*  	Disable all interupts in DR21 */
	writeDirectReg(22,DISABLE_ALL_DR22);/*	Disable all interupts in DR22 */
	writeDirectReg(23,DISABLE_ALL_DR23);/*	Disabel all interupts in DR23 */
	writeDirectReg(64,OPEN_DR64);

	/*(0x18)Calibrations without the ADC and DAC offset and without common mode calibration. */
	writeDirectReg(97,STANDARD_CAL_DR97);
	
	/* (0x47)Calibrate common mode and differential DAC mode DAC + ILIM */
	writeDirectReg(96,STANDARD_CAL_DR96);

	BOOT_MSG("wait DR 96 calibration (0x47)");
	while (readDirectReg(96) != 0 );
	BOOT_MSG(" ---> OK!\n");
	/* This is necessary if the calibration occurs other than at reset time */
	writeDirectReg(98,0x10);
	writeDirectReg(99,0x10);
	for ( i=0x1f; i>0; i--)
	{
		writeDirectReg(98,i);
		mdelay(40);
		
		if((readDirectReg(88)) == 0)
		{	
			progress|=1;
			x=i;
			break;
		}
	}
	for ( i=0x1f; i>0; i--)
	{
		writeDirectReg(99,i);
		mdelay(40);
		
		if((readDirectReg(89)) == 0)
		{
			progress|=2;
			y=i;
			break;
		}
	}
	writeDirectReg(23,ENB2_DR23);  /* enable interrupt for the balance Cal */
	if((readDirectReg(68) & 0x3) & 4)
		return ;

	writeDirectReg(97,BIT_CALCM_DR97); /* this is a singular calibration bit for longitudinal calibration */
	writeDirectReg(96,0x40);
	BOOT_MSG("wait DR 96 calibration (0x40)");
	while(readDirectReg(96) != 0 );
	BOOT_MSG(" ---> OK!\n");

	
	/*
		Initialized DR 98 and 99 to get consistant results.
 		98 and 99 are the results registers and the search 
 		should have same intial conditions.
		The following is the manual gain mismatch calibration
		This is also available as a function
	 */

	mdelay(10);

#ifdef _SI3210_
	for( i = 88 ; i <= 95 ; i++ )
		writeIndirectReg( i , 0x0000 ) ;
	writeIndirectReg( 97 , 0x0000 ) ;
	for( i = 193 ; i <= 211 ; i++ )
		writeIndirectReg( i , 0x0000 ) ;
#else
	for( i = 75 ; i <= 82 ; i++ )
		writeIndirectReg( i , 0x0000 ) ;
	writeIndirectReg( 84 , 0x0000 ) ;
	for( i = 208 ; i <= 211 ; i++ )
		writeIndirectReg( i , 0x0000 ) ;
#endif
	/* The preceding is the manual gain mismatch calibration
  	   The following is the longitudinal Balance Cal
  	 */
	goActive();
	//writeDirectReg(64,OPEN_DR64);

   	writeDirectReg(21,INIT_DR21);
    	writeDirectReg(22,INIT_DR22);
    	writeDirectReg(23,INIT_DR23);
	/*The preceding is the longitudinal Balance Cal*/
}

void clearInterrupts(void)
{
	/* Writing ones to the status registers clears out any pending interrupts */
	writeDirectReg(	20	,	INIT_DR20	);
	writeDirectReg(	19	,	INIT_DR19	);
	writeDirectReg(	18	,	INIT_DR18	);
}


void goActive(void)
{
	int ii;

	writeDirectReg(64,1);	
	/* LOOP STATE REGISTER SET TO ACTIVE */
	/* Active works for on-hook and off-hook see spec. */
	/* The phone hook-switch sets the off-hook and on-hook substate*/

	mdelay(100);
}

/*

//generate ring cadence to FXS port from SLIC
void Ring_On_FXS(void)
{
//	printf("\nRing On");
	writeDirectReg( 64 , 0x4 ) ;		// ringing
//	writeDirectReg(LINE_STATE, 0x04);	// ringing // REG 64 , 0x04
}

void Ring_Off_FXS(void)
{
//	printf("\nRing Off");
	writeDirectReg( 64 , 0x1 ) ;
//	writeDirectReg(LINE_STATE, 0x0); // REG 64 , 0x0
}

*/



int Ring_FXS_Silicon(ring_struct *ring)		
{
	unsigned long flags;

	save_flags(flags); cli();

	SLIC_Choice(ring->CH + 1);
	if (ring->ring_set == 1)
		writeDirectReg( 64 , 0x4 );	//Ring On
	else if (ring->ring_set == 0)
		writeDirectReg( 64 , 0x1 ) ;	//Ring Off

	restore_flags(flags);

	return 0;
}


int Check_Ring_FXS_Silicon(ring_struct *ring)		
{
	unsigned long flags;
	int ringer; //0: ring off, 1: ring on

	save_flags(flags); cli();

	SLIC_Choice(ring->CH + 1);
	
	if ((readDirectReg(64)&0x70)==0x40)
		ringer = 1;
	else
		ringer = 0;
		
	restore_flags(flags);

	return ringer;
}



int proslic_initialize(int ch_slic_init)
{
	unsigned short temp[5];

	int ret = SUCCESS;
	int k= 0;


	/*  Begin Sanity check  Optional */
	if( (readDirectReg(8) != 2) || (readDirectReg(64) != 0) || (readDirectReg(11) != 0x33) )	/* Step 8 */
	{
		//exception(PROSLICiNSANE); 
		//return FAILED;
		//skip sanity check, this is not alway successful
		printk("\r\n[FIX ME] SLIC SANITY doesn't pass, continue... ");
	}



	/* End Sanity check */
	
	//setState(INITIALIZING); 	// This will have more effect in future release			
	
	initializeIndirectRegisters();

	/* settings */
	writeDirectReg(8, 0);/* audio path no loop back */
	writeDirectReg(108, 0xEB);/* enhancement enable register */
	if (chipData.type ==0) /* Si3210 only */
	{
		writeDirectReg(67,0x17);/* Automatic control setting, why bit[3] is set to 0? which means no automatic switching to low battery in off-hook state */
		writeDirectReg(66,0x1);/* VBAT tracks VRING */
	}
	if (chipData.version <=2 )/*REVISION B and before*/
		writeDirectReg(73,4);/* set common mode voltage to 6 volts */

	/*  Flush ProSLIC digital filters by setting Coefficients to clear */
	/*  Save OSC control, Atimer, Itimer of OSC1 OSC2 */
#ifdef _SI3210_
	temp[0] = readIndirectReg(35);	writeIndirectReg(35, 0x8000);
	temp[1] = readIndirectReg(36);	writeIndirectReg(36, 0x8000);
	temp[2] = readIndirectReg(37);	writeIndirectReg(37, 0x8000);
	temp[3] = readIndirectReg(38);	writeIndirectReg(38, 0x8000);
	temp[4] = readIndirectReg(39);	writeIndirectReg(39, 0x8000);
#else	
	temp[0] = readIndirectReg(22);
	temp[1] = readIndirectReg(23);
	temp[2] = readIndirectReg(24);
	temp[3] = readIndirectReg(25);
	temp[4] = readIndirectReg(26);
#endif

	show_PCLK_freq();
	

	/* Do Flush durring powerUp and calibrate */
	if (chipData.type == 0 || chipData.type==3) /*  Si3210 */
	{
		ret = powerUp();  /* Turn on the DC-DC converter and verify voltage. */
		if(ret!=SUCCESS)
		{
			printk("PowerUp Failed\n");
			return ret;
		}
		
#if PowerLeakTest
		ret = powerLeakTest(); /* Check for power leaks */
		if(ret!=SUCCESS)
		{
			printk("Power leak\n");
			return ret;
		}
		ret = powerUp(); /* Turn on the DC-DC converter again */
		if(ret!=SUCCESS)
		{
			printk("PowerUp Failed\n");
			return ret;
		}
#endif
	}

	show_PCLK_freq();
	initializeDirectRegisters(ch_slic_init);
	BOOT_MSG("Start calibrate ...\n");
	calibrate();						/* Step 14 ~ Step 20 */
	BOOT_MSG("Calibrate OK !\n");	

	show_PCLK_freq();
	
	writeIndirectReg(35, temp[0]);
	writeIndirectReg(36, temp[1]);
	writeIndirectReg(37, temp[2]);
	writeIndirectReg(38, temp[3]);
	writeIndirectReg(39, temp[4]);
	clearInterrupts();
	goActive();
	BOOT_MSG("ProSLIC device initializaed\n");
	return SUCCESS;
		
}




int proslic_all_initialized=0;
static unsigned char count_slic_parameter=0;

static void proslic_init_main(int mu_a_law, int ch_slic)
{

	//static unsigned char count=0;
	int freq, set_law = 0;
	char* freqs[ ] = {"8192","4028","2048","1024","512","256","1536","768","32768"};
	/* printable list of frequencies the PCM clock of the proslic opperates at */

	#if chiminer_daisy_chain
		if (!count_slic_parameter)
			//It can only be written once,if two slic will be initialized.
			writeDirectReg_nodaisy(0,0x80|readDirectReg_nodaisy(0));  
		BOOT_MSG("Reg0_internal=%x\n",readDirectReg(0));
		count_slic_parameter++;
	#endif
	
	chipData.version = 0xf & readDirectReg(0); 
	chipData.type = (0x30 & readDirectReg(0)) >> 4;	
	printk("SLIC  type : %d\t", chipData.type);
	printk("version : %c\n", 'A'-1 + chipData.version);

		
	proslic_initialize(ch_slic);  /* initialize the ProSLIC */			/* Step 8 ~ Step 21 */
	//printk("proslic_initialize OK...\n");
	
	show_PCLK_freq();
	
#if 0
	// Ring cadence 1 sec ON , 2 sec off 
	writeDirectReg( 48 , 0x80 ) ;	// ring on low byte
	writeDirectReg( 49 , 0x30 ) ;	// ring on high byte
	writeDirectReg( 50 , 0x80 ) ;	// ring off low byte
	writeDirectReg( 51 , 0x30 ) ;	// ring off high byte
#else
	SLIC_Set_Ring_Cadence_ON_Silicon(slic_order, 1500 /*msec*/ );
	SLIC_Set_Ring_Cadence_OFF_Silicon(slic_order, 1000 /*msec*/ );

#endif
	
	writeDirectReg( 34 , 0x18 ) ;	// enable ring active and inactive timer

	writeDirectReg( 71 , 0x04 ) ;	//Loop current limit, 4 = 21+3*4 mA


//	writeDirectReg( 69 , 0x7f ) ;	//onhook-offhook debounce time = 159 ms

	// jason++ 2005/04/26 
/*	writeDirectReg( 65 , 0x11 ) ;
	writeDirectReg( 66 , 0x1 ) ;
	writeDirectReg( 69 , 0x0c ) ;
*/	


#if 0
	writeDirectReg( 63 , 0x54 ) ;
	writeDirectReg( 67 , 0x1f ) ;
	writeDirectReg( 69 , 0x0c ) ;
	writeDirectReg( 70 , 0x0a ) ;						/* Step 25 optional */
	writeDirectReg( 65 , 0x11 ) ;
	writeDirectReg( 66 , 0x1 ) ;
	writeDirectReg( 71 , 0x1 ) ;
	writeDirectReg( 72 , 0x20 ) ;
	writeDirectReg( 73 , 0x2 ) ;						/* Step 26 optional */
	writeIndirectReg( 35 , 0x8000 ) ;
	writeIndirectReg( 36 , 0x320 ) ;
	writeIndirectReg( 37 , 0x8c ) ;
	writeIndirectReg( 38 , 0x8c ) ;
	writeIndirectReg( 39 , 0x10 ) ;					/* Step 27 write indirect reg */
#endif


	
	if (mu_a_law == 0)
	{
		set_law = 0x38;		/* linear */
	}
	else if (mu_a_law == 1)
	{
		set_law = 0x20;		/* a_law */
	}
	else if (mu_a_law == 2)
	{
		set_law = 0x28;		/* mu_law */
	}

	writeDirectReg(1, set_law) ;	 //Enable PCM, and set_law

	/* print out the PCM clock frequecy and the revision */
	freq = readDirectReg(13)>>4;  /* Read the frequency */
	printk("PCM clock =  %s KHz   Rev %c\n",  freqs[freq], 'A'-1 + chipData.version); 

	show_PCLK_freq();
	
}

void proslic_init_all(int ch_, int pcm_law, unsigned char slic_number)
{
	unsigned long flags;
	
	printk("Start to initialize SLIC CH[%d] ...\n", ch_);		
	save_flags(flags); cli();
	SLIC_Choice(slic_number);
	proslic_init_main(pcm_law, ch_);	/* linear: 0, a_law: 1, u_law: 2 */
	restore_flags(flags);
	printk("Initialize SLIC CH[%d] OK!\n", ch_);
	printk("\n");

}



void disableOscillators_Silicon(void)
{ 
	// Turns off OSC1 and OSC2
	unsigned char i;
	//printf("Disabling Oscillators!!!\n");
	for ( i=32; i<=45; i++) 
		if (i !=34)  // Don't write to the ringing oscillator control
			writeDirectReg(i,0);
}


void genTone(tone_struct tone) 
{ 
	// Uber function to extract values for oscillators from tone_struct
	// place them in the registers, and enable the oscillators.
	unsigned char osc1_ontimer_enable=0, osc1_offtimer_enable=0, osc2_ontimer_enable=0, osc2_offtimer_enable=0;
	int enable_osc2=0;

	//loopBackOff();
	disableOscillators_Silicon(); // Make sure the oscillators are not already on.

	if (tone.osc1.coeff == 0 || tone.osc1.x == 0) {
		// Error!
		printk("You passed me a invalid struct!\n");
		return;
	}
	// Setup osc 1
	writeIndirectReg( OSC1_COEF, tone.osc1.coeff);
	writeIndirectReg( OSC1X, tone.osc1.x);
	writeIndirectReg( OSC1Y, tone.osc1.y);
	//printf("OUt-> 0x%04x\n",tone.osc1.coeff);
	// Active Timer

	if (tone.osc1.on_hi_byte != 0) 
	{
		writeDirectReg( OSC1_ON__LO, tone.osc1.on_low_byte);
		writeDirectReg( OSC1_ON_HI, tone.osc1.on_hi_byte);
		osc1_ontimer_enable = 0x10;
	}
	
	// Inactive Timer
	if (tone.osc1.off_hi_byte != 0) 
	{
		writeDirectReg( OSC1_OFF_LO, tone.osc1.off_low_byte);
		writeDirectReg( OSC1_OFF_HI, tone.osc1.off_hi_byte);
		osc1_offtimer_enable = 0x08;
	}
	
	if (tone.osc2.coeff != 0) 
	{
		// Setup OSC 2
		writeIndirectReg( OSC2_COEF, tone.osc2.coeff);
		writeIndirectReg( OSC2X, tone.osc2.x);
		writeIndirectReg( OSC2Y, tone.osc2.y);
		
		// Active Timer
		if (tone.osc1.on_hi_byte != 0) {
			writeDirectReg( OSC2_ON__LO, tone.osc2.on_low_byte);
			writeDirectReg( OSC2_ON_HI, tone.osc2.on_hi_byte);
			osc2_ontimer_enable = 0x10;
		}
		// Inactive Timer
		if (tone.osc1.off_hi_byte != 0) {
			writeDirectReg( OSC2_OFF_LO, tone.osc2.off_low_byte);
			writeDirectReg( OSC2_OFF_HI, tone.osc2.off_hi_byte);
			osc2_offtimer_enable = 0x08;
		}
		enable_osc2 = 1;
	}
	// Assign to receive path, Osc1 enable, Inactive, Active timer enable.
	writeDirectReg( OSC1, (unsigned char)(0x06 | osc1_ontimer_enable | osc1_offtimer_enable));	
	if (enable_osc2)
		writeDirectReg( OSC2, (unsigned char)(0x06 | osc2_ontimer_enable | osc2_offtimer_enable));// Assign to receive path, Osc2 enable, Inactive, Active timer enable.
	return;
}


/*** thlin+: gen_tone->CH (channel ID) does not work yet. 2005-10-31 ***/
void GenProcessTone_Silicon(genTone_struct *gen_tone)	
{
	switch(gen_tone->tone2play)
	{
		case DialTone_Def:
			genTone(DialTone);
			break;
			
		case ReorderTone_Def:
			genTone(ReorderTone);
			break;
			
		case CongestionTone_Def:
			genTone(CongestionTone);
			break;
			
		case RingbackPBX_Def:
			genTone(RingbackPBX);
			break;
			
		case RingbackNormal_Def:
			genTone(RingbackNormal);
			break;
			
		case BusySignal_Def:
			genTone(BusySignal);
			break;
				
		case RingbackJapan_Def:
			genTone(RingbackJapan);
			break;
			
		case JapanDialTone_Def:
			genTone(JapanDialTone);
			break;
		
		case DisableTone:
			disableOscillators_Silicon();
			break;
					
		default:
			break;
			
			
	}
}


void genDTMFTone(genTone_struct *gen_tone)	
{
	switch(gen_tone->tone2play)
	{
	
		case DMTF_0:
			genTone(dual_tones[0]);
			break;
			
		case DMTF_1:
			genTone(dual_tones[1]);
			break;
			
		case DMTF_2:
			genTone(dual_tones[2]);
			break;
			
		case DMTF_3:
			genTone(dual_tones[3]);
			break;
			
		case DMTF_4:
			genTone(dual_tones[4]);
			break;
			
		case DMTF_5:
			genTone(dual_tones[5]);
			break;
			
		case DMTF_6:
			genTone(dual_tones[6]);
			break;
			
		case DMTF_7:
			genTone(dual_tones[7]);
			break;
			
		case DMTF_8:
			genTone(dual_tones[8]);
			break;
			
		case DMTF_9:
			genTone(dual_tones[9]);
			break;
			
		case DMTF_Star:
			genTone(dual_tones[10]);
			break;
			
		case DMTF_Square:
			genTone(dual_tones[11]);
			break;
			
		case DMTF_A:
			genTone(dual_tones[12]);
			break;
			
		case DMTF_B:
			genTone(dual_tones[13]);
			break;
			
		case DMTF_C:
			genTone(dual_tones[14]);
			break;
			
		case DMTF_D:
			genTone(dual_tones[15]);
			break;
			
		case DisableTone:
			disableOscillators_Silicon();
			break;
		
		default:
			break;
			
			
	}
}


void reset_SLIC(int codec_law, unsigned char slic_number)		/* linear: 0, a_law: 1, u_law: 2 */
{
	reset_slic_parameter();//re-init spi
	
	
	pcm_disableChan(0);			// disable ch0 pcm
	proslic_init_all(0, codec_law, 1);	//re-init slic order 1
	
	if (SLIC_CH_NUM == 2)
	{
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
	init_spi(1);
#endif		
		pcm_disableChan(1);			// disable ch1 pcm
		proslic_init_all(0, codec_law, 2);	//re-init slic order 2
	}
}



void SLIC_CHANNEL_CHANGE(unsigned char order,unsigned char number) {
	if ((order == 1 && number == 0) || (order == 2 && number == 1)){
		SLIC_Choice(1);
		writeDirectReg(2,	0x01	);	
		writeDirectReg(3,	INIT_DR3	);/*0x00	PCM TX Clock Slot High Byte */
		writeDirectReg(4,	0x01	);	
		writeDirectReg(5,	INIT_DR5	);/*0x00	PCM RX Clock Slot High Byte */
		
		SLIC_Choice(2);
		writeDirectReg(2,	0x09	);	
		writeDirectReg(3,	INIT_DR3	);/*0x00	PCM TX Clock Slot High Byte */
		writeDirectReg(4,	0x09	);	
		writeDirectReg(5,	INIT_DR5	);/*0x00	PCM RX Clock Slot High Byte */
	} else if ((order == 1 && number == 1) || (order == 2 && number == 0)) {
		SLIC_Choice(1);
		writeDirectReg(2,	0x09	);	
		writeDirectReg(3,	INIT_DR3	);/*0x00	PCM TX Clock Slot High Byte */
		writeDirectReg(4,	0x09	);	
		writeDirectReg(5,	INIT_DR5	);/*0x00	PCM RX Clock Slot High Byte */
	
		SLIC_Choice(2);
		writeDirectReg(2,	0x01	);	
		writeDirectReg(3,	INIT_DR3	);/*0x00	PCM TX Clock Slot High Byte */
		writeDirectReg(4,	0x01	);	
		writeDirectReg(5,	INIT_DR5	);/*0x00	PCM RX Clock Slot High Byte */
	} else	{
		printk("No such device or no such channel\n");
		return;
	}	  

	SLIC_Choice(order);	
	printk("The channel of Slic %d  is changed to channel %d\n",order,number);
	
	return;	
}

/* Table for set the silicon SLIC rx/tx gain: -6dB ~ 3dB, space: 1dB */
short slic_gain_table_silicon[10] = {0x2010, 0x23F0, 0x2860, 0x2D40, 0x32D0,
				     0x3900, 0x4000, 0x47C0, 0x5090, 0x5A60};

void SLIC_Set_Tx_Gain_Silicon(unsigned char chid, unsigned char tx_gain)
{
	unsigned long flags;
		
	save_flags(flags); cli();
	SLIC_Choice(chid+1);// select SLIC	
#ifdef _SI3210_
	//writeIndirectReg(27, 0x1000+0x500*tx_gain);// for Tx digital gain, step +0.5dB (0x500)	
	writeIndirectReg(27, slic_gain_table_silicon[tx_gain-1]);
#else
	//writeIndirectReg(14, 0x20+0x920*tx_gain);//tx_gain: 0 to 13 (-7dB to 6dB), step +1dB (0x920)
	writeIndirectReg(14, slic_gain_table_silicon[tx_gain-1]);
#endif
	restore_flags(flags);
}

void SLIC_Set_Rx_Gain_Silicon(unsigned char chid, unsigned char rx_gain)
{
	unsigned long flags;

	save_flags(flags); cli();
	SLIC_Choice(chid+1);// select SLIC
#ifdef _SI3210_
	//writeIndirectReg(26, 0x1000+0x500*rx_gain);// for Rx digital gain, step +0.5dB (0x500)	
	writeIndirectReg(26, slic_gain_table_silicon[rx_gain-1]);
#else
	//writeIndirectReg(13, 0x20+0x920*rx_gain);//rx_gain: 0 to 13 (-7dB to 6dB), step +1dB (0x920)
	writeIndirectReg(13, slic_gain_table_silicon[rx_gain-1]);
#endif
	restore_flags(flags);
}

void SLIC_Set_Ring_Cadence_ON_Silicon(unsigned char slic__order, unsigned short msec)
{
	unsigned int l_time, h_time, tmp;
	unsigned long flags;

	tmp = msec << 3;
	if (tmp > 255)
	{
		l_time = 255; 
		h_time = (tmp - 255)>>8;
	}
	else
	{
		l_time = tmp;
		h_time = 0;
	}
		
	save_flags(flags); cli();
	SLIC_Choice(slic__order);		// select SLIC
	writeDirectReg( 48 , l_time ) ;	// ring on low byte
	writeDirectReg( 49 , h_time ) ;	// ring on high byte
	restore_flags(flags);
}

void SLIC_Set_Ring_Cadence_OFF_Silicon(unsigned char slic__order, unsigned short msec)
{
	unsigned int l_time, h_time, tmp;
	unsigned long flags;

	tmp = msec << 3;
	if (tmp > 255)
	{
		l_time = 255; 
		h_time = (tmp - 255)>>8;
	}
	else
	{
		l_time = tmp;
		h_time = 0;
	}

	save_flags(flags); cli();
	SLIC_Choice(slic__order);		// select SLIC
	writeDirectReg( 50 , l_time ) ;	// ring off low byte
	writeDirectReg( 51 , h_time ) ;	// ring off high byte
	restore_flags(flags);
}


void SLIC_Set_Impendance_Silicon(unsigned char slic__order, unsigned short country, unsigned short impd)
{
	unsigned long flags;
	save_flags(flags); cli();
	SLIC_Choice(slic__order);
	
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210
	
	switch (country)
	{
		case COUNTRY_USA:
			impd = INIT_DR10; //0x28(600  and enabled)
			break;
		case COUNTRY_AUSTRALIA:
			impd = 0X0D; 
			break;
		case COUNTRY_GR:
			impd = 0x0E; 
			break;
		case COUNTRY_UK:
		case COUNTRY_HK:
		case COUNTRY_JP:
		case COUNTRY_SE:
		case COUNTRY_FR:
		case COUNTRY_TR:
		case COUNTRY_BE:
		case COUNTRY_FL:
		case COUNTRY_IT:
		case COUNTRY_CN:
			impd = INIT_DR10; 
			PRINT_MSG(" Not support impedance of this country. Set to default SLIC impedance 600 ohm.\n");
			break;
		default:
			impd = INIT_DR10; 
			PRINT_MSG(" Not support impedance of this country. Set to default SLIC impedance 600 ohm.\n");
			break;
	}
			
	writeDirectReg(10, impd);
	
#elif defined CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215

	switch (country)
	{
		case COUNTRY_USA:
			impd = INIT_DR10; //0x28(600  and enabled)
			break;
		case COUNTRY_AUSTRALIA:
			impd = 0X0D; 
			break;
		case COUNTRY_GR:
			impd = 0x0E; 
			break;
		case COUNTRY_JP:
			impd = 0x0A;	//600 ohm + 1uF; requires external resistor RZREF = 12 k. and C3, C4 = 100 nF.
			PRINT_MSG("Impednace for Japan: requires external resistor RZREF = 12 k. and C3, C4 = 100 nF for SLIC si3215\n");
			break;
		case COUNTRY_CN:
			impd = 0x0F;	//200 ohm + (680 ohm || 100 nF)
			break;
		case COUNTRY_UK:
		case COUNTRY_HK:
		case COUNTRY_SE:
		case COUNTRY_FR:
		case COUNTRY_TR:
		case COUNTRY_BE:
		case COUNTRY_FL:
		case COUNTRY_IT:
			impd = INIT_DR10; 
			PRINT_MSG(" Not support impedance of this country. Set to default SLIC impedance 600 ohm.\n");
			break;
		default:
			impd = INIT_DR10; 
			PRINT_MSG(" Not support impedance of this country. Set to default SLIC impedance 600 ohm.\n");
			break;
	}

	writeDirectReg(10, impd);
	
#endif
	restore_flags(flags);
}

/********************* FSK Caller ID Related Function *******************/
// hc+ 1215 for bkup register value at the beginning, then write back to related register at last,
// for recover original function
static unsigned char dreg64, dreg108;	// linefeed control & Enhance enable
static unsigned char dreg[20];	// direct reg18-23, dreg32-45 (skip 34), total 6+14=20 bytes
static unsigned char ireg[6];	// indirect reg99-104, total 6 bytes


void bkup_reg(void)
{	unsigned char i;

	// direct reg
	dreg64 = readDirectReg(64);
	dreg108 = readDirectReg(108);
	//printk("reg64=%02x\n",dreg64);

	// direct reg
	for (i=0; i<6; i++)
		dreg[i] = readDirectReg(18+i);
	for (i=0; i<14; i++)
		dreg[6+i] = readDirectReg(32+i);		

	// indirect reg for 3215
	for (i=0; i<6; i++)
		ireg[i] = readIndirectReg(69+i);
}

void restore_reg(void)
{	unsigned char i;

	// direct reg
	for (i=0; i<6; i++)
		writeDirectReg(18+i, dreg[i]);
	for (i=0; i<14; i++)
	{	if (i!=2)
			writeDirectReg(32+i, dreg[6+i]);		
	}

	// indirect reg for 3215
	for (i=0; i<6; i++)
		writeIndirectReg(69+i, ireg[i]);

	// direct reg 
	writeDirectReg(108, dreg108);
	writeDirectReg(64, dreg64);
}
void reset_slic_parameter()
{
	init_spi(0);
	count_slic_parameter=0;		
}

#if ! defined (AUDIOCODES_VOIP)
//======================= FSK CID =========================//

void cid_fsk_alert_gen(uint32 chid, int32 sid, char *str)
{
	const int per_digit = 9;	// per 9 frame (10ms) for one digit (def. 80~85ms)
	const int pre_dsil = 6;		// pre silence (def. 40~50ms)
	const int end_dsil = 18;	// end silence (def. <100ms+75)

	static char init=1;		// hc+ 1213 for init state, 1 : init state, 0 : normal playout

	static int cnt_per_digit;		
	static int cnt_pre_dsil;		
	static int cnt_end_dsil;		
	static int cid_cnt;		// frame counter for cnt_pre_dsil + CID + cnt_end_dsil
	static char cid_num[4];		// alert string for playout
	static char cid_len;		// alert string length
	static char cid_reg64, cid_reg64_prev;
	static char last_digit = DSPCODEC_TONE_NONE;

	// initial state for initial partial parameter by case
	if (init)	
	{	// adjust by codec
#ifdef CONFIG_RTK_VOIP_G7231
#ifndef SUPPORT_CUT_DSP_PROCESS
		if (CheckG723StartCodecType( chid ))	// G723
		{	cnt_per_digit = per_digit/3;
			cnt_pre_dsil = pre_dsil/3;
			cnt_end_dsil = end_dsil/3;
		}
		else	// G711 & G729
#endif
#endif
		{	cnt_per_digit = per_digit;
			cnt_pre_dsil = pre_dsil;
			cnt_end_dsil = end_dsil;
		}
		
		cid_cnt = 0;	// start from 0

		// interpret str to cid_num
		cid_len = 1;	

		cid_num[0] = str[0];	
		PRINT_MSG("\nFSK alert digit = %d, len = %d\n", cid_num[0], cid_len);	

		init = 0;	// into playout state

		cid_reg64 = readDirectReg(64);
		if ( (cid_reg64 & 0x07) != 2 )	// force for audio path power on
		{	
			cid_reg64_prev = cid_reg64;	// record it
			PRINT_MSG("Reg64 = 0x%02x\n", cid_reg64);
			writeDirectReg(64,2);
	}	}
	
	if (!init)		// generate DTMF CID waveform
	{		
		if (cid_cnt>=cnt_pre_dsil) 	// not prefix silence
		{			
			if ( cid_cnt<(cnt_pre_dsil+cnt_per_digit*cid_len+cnt_end_dsil) )	// in alert + end silence
			{	
				if ( !(cid_cnt>=(cnt_pre_dsil+cnt_per_digit*cid_len) ) )	// in alert
				{	
					if (cid_cnt%cnt_per_digit==0)	// set middle cont for display & record as last digit
					{	PRINT_MSG("%d ", cid_num[(cid_cnt-cnt_pre_dsil)/cnt_per_digit]);
						if (cid_num[(cid_cnt-cnt_pre_dsil)/cnt_per_digit]!=DSPCODEC_TONE_NONE)	// only record last non silence digit
							last_digit = cid_num[(cid_cnt-cnt_pre_dsil)/cnt_per_digit];
					}	
					if (cid_num[(cid_cnt-cnt_pre_dsil)/cnt_per_digit]!=DSPCODEC_TONE_NONE)	// not silence digit
					//	DspcodecPlayTone(chid, cid_num[(cid_cnt-cnt_pre_dsil)/cnt_per_digit], true, DSPCODEC_TONEDIRECTION_LOCAL);
						// Must use this function
						hc_SetPlayTone(chid, sid, cid_num[(cid_cnt-cnt_pre_dsil)/cnt_per_digit], true, DSPCODEC_TONEDIRECTION_LOCAL);	

				}
				else		// in end silence
				{
				}

			}
			else	// after end silence do once only
			{	
				// for turn off last one tone		
				hc_SetPlayTone(chid, sid, last_digit, false, DSPCODEC_TONEDIRECTION_LOCAL);	// hc$ 1206 for last digit
				PRINT_MSG("\nlast_digit = %d\n", last_digit);

				fsk_alert_state[chid] = 0;			
				init = 1; 

				writeDirectReg(64,cid_reg64_prev);
				PRINT_MSG("\nEnd of FSK alert,  chid = %d.", chid);

				PRINT_MSG("\n");
				
		}	}
		else	// in prefix silence
		{
		}
		cid_cnt++;	
	}
}

#endif

/* Gavin+ 2006/07/21 */
void OnHookLineReversal_Silicon(int chid, unsigned char bReversal)
{
	SLIC_Choice(chid + 1);
	
	if(bReversal)
		writeDirectReg(64, 6);  //Reverse On-Hook Transmission
	else
		writeDirectReg(64, 2);	//Forward On-Hook Transmission
}


// Gavin add
// function name : SendNTTCAR
// description : Send CAR message before first ring
// author : Gavin
void SendNTTCAR_Silicon(int chid)
{
	int protect_cnt = 0;
#ifdef _SI3210_	
	int i, b_num[]={19, 20, 21, 22, 34, 48, 49, 50, 51, 74};// for Si3210
#else
	int i, b_num[]={6, 7, 8, 9, 21, 48, 49, 50, 51, 74};	// for Si3215
#endif
	unsigned short s_buf[4];
	unsigned char c_buf[10];
	unsigned short s[]={0x0000, 0x7f60, 0x0172, 0x0000, 0x18, 0xa0, 0x0f, 0xa0, 0x0f, 0x3f};

	SLIC_Choice(chid + 1);
	
	/************** backup the register ***************/
	for ( i=0;i<4;i++)
		s_buf[i] = readIndirectReg(b_num[i]);
	for ( i=4;i<10;i++)
		c_buf[i] = readDirectReg(b_num[i]);
	
	/*********** To Create Short Ring *****************/
	//send CAR
	for (i=0;i<4;i++)
		writeIndirectReg(b_num[i], s[i]);
	for (i=4;i<10;i++)
		writeDirectReg(b_num[i], s[i]);
	
	writeDirectReg(64, 0x04); // ringing // REG 64 , 0x04

 	mdelay(100); // delay for a while
 	// cyg_thread_delay(600) ;    // hold for 6 seconds
 	
	/*********** Check Phone Hook State ***************/

	if (readDirectReg(68)&4) // if phone on-hook
		while(!(readDirectReg(68) &1))  //wait phone off-hook atuomatically
		{
			if (protect_cnt == 30)	// wait 6 sec, if no off-hook-> break to prevent watch dog reset.
				break;
			mdelay(200);
			protect_cnt ++;
		}
			
	/******* Set Reverse On-Hook Transmission *********/		
	
	OnHookLineReversal(chid, 1); // if phone off-hook, set Reverse On-Hook Transmission
	
	/************** restore the register ***************/
	for ( i=0;i<4;i++)
		writeIndirectReg(b_num[i],s_buf[i]);
	for ( i=4;i<10;i++)
		writeDirectReg(b_num[i],c_buf[i]);
	

}


#if ! defined (AUDIOCODES_VOIP)
void fskInitialization (void)	// hcv, set/enable OSC1 active timer, then set FSK Amp/Freq of space/mark, set FSK as Rx path  
{	
	init =0 ;  // gloabal variable used to detect critical timing violation
    	       // if init =2 => more than 1/1200th second passed between interrupt

/*Step One - Initialize the Registers (AN32)*/
	writeDirectReg( REVC,0x40);	// hc$ 1215 open it
										// set to revision C FSK mode on  Now this is done ealier
										// proslic.c initializeDirectRegisters() reg108=0xEB, so bit6 already set as 1 for FSK	
	
		// set OSC1 active timer low/high byte for baud rate
	writeDirectReg(36,19);  // 19 is twenty ticks  20/24000 sec = 1/1200 sec
	writeDirectReg(37,0x0); // 0 is zero MSB of timer

		// set reg52 to 1 (mark)
	writeDirectReg(FSK_DATA,1);  /* Mark is the default value */
	writeDirectReg(FSK_DATA,1);  /* Mark is the default value */
/* writen twice to fill double buffer which has logic to detect
   bit transition
*/

		// only enable OSC1 active timer
	writeDirectReg(21,0x01); /*  Mask Register #1  Active Interrupt*/

/*

INFO: case0 = 0.997673,case1 = 0.998714,case2 = 1.001328,case3 = 1.002373
case1 < case0

	// Note : sample rate is 24, not 8k
	// (A) Space 
INFO: Settings for  2200 as initial frequency are - OSCn = 0x6b60, OSCnX = 0x01b4	<= (A)  2200 Hz 
	// (B) Mark
INFO: Settings for  1200 as initial frequency are - OSCn = 0x79c0, OSCnX = 0x00e9	<= (B)  1200 Hz
	// (B1) Space -> Mark
INFO: Settings for  2200 to  1200  transition are - OSCn = 0x79c0, OSCnX = 0x1110	<= (B1) 2200->1200 
	// (A1) Mark -> Space
INFO: Settings for  1200 to  2200  transition are - OSCn = 0x6b60, OSCnX = 0x3c00	<= (A1) 1200->2200
INFO: Compound gain variation = 0.999756
*/
#ifndef _SI3210_

	/* SI-3215 */
	switch(fsk_spec_mode&7)	// hc$ 0220	
	{
		case FSK_Bellcore:  // BellCore
			writeIndirectReg(FSK_X_0,0x07F9); 	// reg99  FSK Amplitude Coefficient for Space.		<= (A)  2200 Hz Space
			writeIndirectReg(FSK_COEFF_0,0x5320);	// reg100 FSK Frequency Coefficient for Space.		<= (A)  
			writeIndirectReg(FSK_X_1,0x0426);	// reg101 FSK Amplitude Coefficient for Mark.		<= (B)  1200 Hz Mark
			writeIndirectReg(FSK_COEFF_1,0x7210);	// reg102 FSK Frequency Coefficient for Mark.		<= (B)
			writeIndirectReg(FSK_X_01,0x10B0);	// reg103 FSK Transition Parameter from 0 to 1.		<= (B1) 2200->1200
			writeIndirectReg(FSK_X_10,0x3D60);	// reg104 FSK Transition Parameter from 1 to 0.		<= (A1) 1200->2200
			writeDirectReg(OSC1,0x56);  		// reg32  FSK mode receiver, Oscillator 1 Control
			break;					// b7 - OSC1 signal status,
								//		0 : o/p signal inactive
								// b6 - OSC1 automatic register reload ( for FSK signaling)
								//		1 : continue read register parameter & o/p signal  							 
								// b5 - OSC1 zero cross enable
								//		0 : signal terminate after active timer expires
								// b4 - OSC1 active timer enable 
								//		1 : enable timer	
								// b3 - OSC1 inactive timer enable
								//		0 : disable timer
								// b2 - OSC1 enable
								//		1 : enable OSC1
								// b1:0 - OSC1 signal o/p routing
								//		00 : no path (o/p not connected)
								//		01 : to transmit path	
								//		10 : to receive path
								//		11 : to both path
		case FSK_ETSI:	// ETSI 	
		case FSK_BT:  // BT (?Vrms)

			writeIndirectReg(FSK_X_0,0x0791); 		// reg99  FSK Amplitude Coefficient for Space.		<= (A)  2100 Hz Space
			writeIndirectReg(FSK_COEFF_0	,0x56E0);	// reg100 FSK Frequency Coefficient for Space.		<= (A)  
			writeIndirectReg(FSK_X_1,0x0483);		// reg101 FSK Amplitude Coefficient for Mark.		<= (B)  1300 Hz Mark
			writeIndirectReg(FSK_COEFF_1	,0x6FB0);	// reg102 FSK Frequency Coefficient for Mark.		<= (B)
			writeIndirectReg(FSK_X_01,0x1310);		// reg103 FSK Transition Parameter from 0 to 1.		<= (B1) 2100->1300
			writeIndirectReg(FSK_X_10,0x35B0);		// reg104 FSK Transition Parameter from 1 to 0.		<= (A1) 1300->2100
			writeDirectReg(OSC1,0x56);  			// reg32  FSK mode receiver, Oscillator 1 Control

			break;

		case FSK_NTT:  // NTT (?Vrms)
			writeIndirectReg(FSK_X_0,0x0791); 		// reg99  FSK Amplitude Coefficient for Space.		<= (A)  2100 Hz Space
			writeIndirectReg(FSK_COEFF_0	,0x56E0);	// reg100 FSK Frequency Coefficient for Space.		<= (A)  
			writeIndirectReg(FSK_X_1,0x0483);		// reg101 FSK Amplitude Coefficient for Mark.		<= (B)  1300 Hz Mark
			writeIndirectReg(FSK_COEFF_1	,0x6FB0);	// reg102 FSK Frequency Coefficient for Mark.		<= (B)
			writeIndirectReg(FSK_X_01,0x1310);		// reg103 FSK Transition Parameter from 0 to 1.		<= (B1) 2100->1300
			writeIndirectReg(FSK_X_10,0x35B0);		// reg104 FSK Transition Parameter from 1 to 0.		<= (A1) 1300->2100
			writeDirectReg(OSC1,0x56);  			// reg32  FSK mode receiver, Oscillator 1 Control
//			writeDirectReg(36, 0x13);			// gavin
//			writeDirectReg(37, 0x00);			// gavin	2100Hz, 1300Hz, 0.13 Vrms
//			printk("\n NTT");
			break;
	}
#else
	/* SI-3210 */
	switch(fsk_spec_mode&7)	// hc$ 0220	
	{
		case FSK_Bellcore:  // BellCore
			writeIndirectReg(FSK_X_0,0x01b4); 			// reg99  FSK Amplitude Coefficient for Space.		<= (A)  2200 Hz Space
			writeIndirectReg(FSK_COEFF_0	,0x6b60);	// reg100 FSK Frequency Coefficient for Space.		<= (A)  
			writeIndirectReg(FSK_X_1,0x00e9);			// reg101 FSK Amplitude Coefficient for Mark.		<= (B)  1200 Hz Mark
			writeIndirectReg(FSK_COEFF_1	,0x79c0);	// reg102 FSK Frequency Coefficient for Mark.		<= (B)
			writeIndirectReg(FSK_X_01,0x1110);			// reg103 FSK Transition Parameter from 0 to 1.		<= (B1) 2200->1200
			writeIndirectReg(FSK_X_10,0x3c00);			// reg104 FSK Transition Parameter from 1 to 0.		<= (A1) 1200->2200
			writeDirectReg(OSC1,0x56);  				// reg32  FSK mode receiver, Oscillator 1 Control
			break;										//							 b7 - OSC1 signal status,
														//									0 : o/p signal inactive
														//							 b6 - OSC1 automatic register reload ( for FSK signaling)
														//									1 : continue read register parameter & o/p signal  							 
														//							 b5 - OSC1 zero cross enable
														//									0 : signal terminate after active timer expires
														//							 b4 - OSC1 active timer enable 
														//									1 : enable timer	
														//							 b3 - OSC1 inactive timer enable
														//									0 : disable timer
														//							 b2 - OSC1 enable
														//									1 : enable OSC1
														//							 b1:0 - OSC1 signal o/p routing
														//									00 : no path (o/p not connected)
														//									01 : to transmit path	
														//									10 : to receive path
														//									11 : to both path
		case FSK_ETSI:	// ETSI 	
		
		case FSK_BT:  // BT (?Vrms)
			writeIndirectReg(FSK_X_0,0x010e); 			// reg99  FSK Amplitude Coefficient for Space.		<= (A)  2100 Hz Space
			writeIndirectReg(FSK_COEFF_0	,0x6d20);	// reg100 FSK Frequency Coefficient for Space.		<= (A)  
			writeIndirectReg(FSK_X_1,0x00a4);			// reg101 FSK Amplitude Coefficient for Mark.		<= (B)  1300 Hz Mark
			writeIndirectReg(FSK_COEFF_1	,0x78b0);	// reg102 FSK Frequency Coefficient for Mark.		<= (B)
			writeIndirectReg(FSK_X_01,0x1370);			// reg103 FSK Transition Parameter from 0 to 1.		<= (B1) 2100->1300
			writeIndirectReg(FSK_X_10,0x34b0);			// reg104 FSK Transition Parameter from 1 to 0.		<= (A1) 1300->2100
			writeDirectReg(OSC1,0x56);  				// reg32  FSK mode receiver, Oscillator 1 Control
			break;

		
		case FSK_NTT:  // NTT (?Vrms)
			writeIndirectReg(FSK_X_0,0x010e); 			// reg99  FSK Amplitude Coefficient for Space.		<= (A)  2100 Hz Space
			writeIndirectReg(FSK_COEFF_0	,0x6d20);	// reg100 FSK Frequency Coefficient for Space.		<= (A)  
			writeIndirectReg(FSK_X_1,0x00a4);			// reg101 FSK Amplitude Coefficient for Mark.		<= (B)  1300 Hz Mark
			writeIndirectReg(FSK_COEFF_1	,0x78b0);	// reg102 FSK Frequency Coefficient for Mark.		<= (B)
			writeIndirectReg(FSK_X_01,0x1370);			// reg103 FSK Transition Parameter from 0 to 1.		<= (B1) 2100->1300
			writeIndirectReg(FSK_X_10,0x34b0);			// reg104 FSK Transition Parameter from 1 to 0.		<= (A1) 1300->2100
			writeDirectReg(OSC1,0x56);  				// reg32  FSK mode receiver, Oscillator 1 Control
//			writeDirectReg(36, 0x13);//gavin
//			writeDirectReg(37, 0x00);//gavin	2100Hz, 1300Hz, 0.13 Vrms
//			printk("\n NTT");
			break;
	}
#endif

/*Step Two - Enable the Oscillator (AN32)*/
	//writeDirectReg(21,0x01); /*  Mask Register #1  Active Interrupt*/
	//writeDirectReg(OSC1,0x56);  // reg32  FSK mode receiver, Oscillator 1 Control
}

#endif /*AUDIOCODES_VOIP*/


