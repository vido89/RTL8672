/*
* Copyright c                  Realtek Semiconductor Corporation, 2005  
* All rights reserved.
* 
* Program : Proslic Si3210 initialial process Header File
* Abstract :  
* Author :  thlin (thlin@realtek.com.tw)      
* $Id: si3210init.h,v 1.3 2007/11/16 07:04:30 eric Exp $ 
*/
#ifndef _SI3210INIT
#define _SI3210INIT
#include <linux/config.h>
#include "rtl_types.h"
#include "Slic_api.h"


#define LINEAR	0
#define A_LAW	1
#define U_LAW	2

#define DMTF_0	 	0	// DMTF digit: 0
#define DMTF_1	 	1	// DMTF digit: 1
#define DMTF_2	 	2	
#define DMTF_3	 	3
#define DMTF_4	 	4
#define DMTF_5	 	5
#define DMTF_6	 	6
#define DMTF_7	 	7 
#define DMTF_8	 	8
#define DMTF_9	 	9	// DMTF digit: 9
#define DMTF_Star  	10	// DMTF digit: *
#define DMTF_Square 	11	// DMTF digit: #
#define DMTF_A	 	12	// DMTF digit: A
#define DMTF_B	 	13	// DMTF digit: B
#define DMTF_C	 	14	// DMTF digit: C
#define DMTF_D	 	15	// DMTF digit: D
#define DialTone_Def 		16
#define ReorderTone_Def		17 
#define CongestionTone_Def	18 
#define RingbackPBX_Def 	19 
#define RingbackNormal_Def	20
#define BusySignal_Def 		21
#define RingbackJapan_Def 	22 
#define BusyJapan_Def		23
#define JapanDialTone_Def	24 
#define DisableTone		25





enum slic_state{ 
	MAKEbUSY,				// 0
	STATEcHANGE,			// 1
	DIALtONE,				// 2
	INITIALIZING,			// 3
	POWERuP,				// 4
	CALIBRATE,				// 5
	PROGRAMMEDrING,		// 6
	POWERdOWN,			// 7
	POWERlEAKtEST,			// 8
	MAKErINGbACKjAPAN,	// 9
	MAKEbUSYjAPAN,		// 10
	RINGbACKjAPAN,			// 11
	MAKErINGbACK,			// 12
	RINGbACK,				// 13
	MAKErEORDER,			// 14
	REORDER,				// 15
	MAKEcONGESTION,		// 16
	CONGESTION,			// 17
	PRENEON,				// 18
	NEON,					// 19
	CALLBACKpROGRAMMED,	// 20
	BUSY,					// 21
	CALLBACK,				// 22
	CALLING,					// 23
	MAKEoFFHOOK,			// 24
	ONHOOK,					// 25
	OFFHOOK,				// 26
	DIGITDECODING,			// 27
	LOOPtRANSITION,			// 28
	FIRSTrING,				// 29
	DEFEREDcALLBACKpROGRAMMED,// 30
	CALLERiD,				// 31
	RINGING,				// 32
	DTMFtRANISTION			// 33
} ;


typedef struct {
	//unsigned char chip_number;
	enum slic_state state,newState,previousState;
	int digit_count;
	char DTMF_digits[20];
	unsigned long interrupt;
	unsigned char eventEnable;
	unsigned char hook_status;
	unsigned long On_Hook_time;
	unsigned long Off_Hook_time;
	char	version,type;
	struct{ enum { TYPE1, TYPE2, TYPE3 } ringType;
			int nextCadenceEntryIndex;
		}  ringCadenceCordinates;
	unsigned char ringCount;
	int qLog[6];
	unsigned long eventNumber;
} chipStruct;

typedef struct {
	unsigned char address;
	char *name;
	unsigned short initial;
} indirectRegister;


typedef struct {
	unsigned short coeff;
	unsigned short x;
	unsigned short y;
	unsigned char on_hi_byte;
	unsigned char on_low_byte;
	unsigned char off_hi_byte;
	unsigned char off_low_byte;
} Oscillator;

typedef struct {
	Oscillator osc1;
	Oscillator osc2;
} tone_struct; 

/* mark by chiminer
typedef struct {
	unsigned char CH;		// CH:0 - 3
	unsigned char change;		// 1: Change. 0: No-Change
	unsigned char hook_status;	// 1: Off-Hook, 0: On-Hook
} hook_struck;*/

/* mark by chiminer
typedef struct {
	unsigned char CH;		// CH = 0 ~ 3
	unsigned char ring_set;		// Ring_ON: ring_set = 1 ,  Ring_OFF: ring_set = 0 

} ring_struct;*/

typedef struct {
	unsigned char CH;
	unsigned int tone2play;
} genTone_struct;

enum exceptions 
{
	PROSLICiNSANE,
	TIMEoUTpOWERuP,
	TIMEoUTpOWERdOWN,
	POWERlEAK,
	TIPoRrINGgROUNDsHORT,
	POWERaLARMQ1,
	POWERaLARMQ2,
	POWERaLARMQ3,
	POWERaLARMQ4, 
	POWERaLARMQ5,
	POWERaLARMQ6,
};

/*******************  Function Prototype in Si3210init.c  ********************/
void SLIC_CHANNEL_CHANGE(unsigned char order,unsigned char number);
void SLIC_Choice(unsigned char number); 
void show_PCLK_freq(void);
void read_all_Direct_Reg(void)	;
//void proslic_init_all(int ch_, int pcm_law);
void proslic_init_all(int ch_, int pcm_law,unsigned char slic_number);//chiminer modify 
int Hook_Polling(hook_struck *hook);		// Success: return 0, Fail: return -1.
int Ring_FXS_Silicon(ring_struct *ring);		// Success: return 0, Fail: return -1.
/****************************************************************************************/
void GenProcessTone_Silicon(genTone_struct *gen_tone);		// generate by SLIC.
void genDTMFTone(genTone_struct *gen_tone);
/*** class of tone : *** 
DialTone ,ReorderTone, CongestionTone, RingbackPBX, RingbackNormal,
BusySignal, RingbackJapan, BusyJapan, JapanDialTone, dual_tones[0]~dual_tones[15] (DTMF)
******************************************************************************************/

//void Ring_On_FXS(void);
//void Ring_Off_FXS(void);
void SLIC_Set_Tx_Gain_Silicon(unsigned char chid, unsigned char tx_gain);
void SLIC_Set_Rx_Gain_Silicon(unsigned char chid, unsigned char rx_gain);
void SLIC_Set_Ring_Cadence_ON_Silicon(unsigned char chid, unsigned short msec);
void SLIC_Set_Ring_Cadence_OFF_Silicon(unsigned char chid, unsigned short msec);
/********************************************************************/
void SetSi321xPCM(int chid, int enable);
void reset_SLIC(int codec_law, unsigned char slic_number);
void goActive(void);

/* Direct Register Initial Value */
#define	INIT_DR0	0X00	//	Serial Interface
#define	INIT_DR1	0X28	//	PCM Mode
#define	INIT_DR2	0X00	//	PCM TX Clock Slot Low Byte (1 PCLK cycle/LSB)
#define	INIT_DR3	0x00	//	PCM TX Clock Slot High Byte
#define	INIT_DR4	0x00	//	PCM RX Clock Slot Low Byte (1 PCLK cycle/LSB)
#define	INIT_DR5	0x00	//	PCM RX Clock Slot High Byte
#define	INIT_DR6	0x00	//	DIO Control (external battery operation, Si3211/12)
#define	INIT_DR8	0X00	//	Loopbacks (digital loopback default)
#define	INIT_DR9	0x00	//	Transmit and receive path gain and control
#define	INIT_DR10	0X28	//	Initialization Two-wire impedance (600  and enabled)
#define	INIT_DR11	0x33	//	Transhybrid Balance/Four-wire Return Loss
#define	INIT_DR14	0X10	//	Powerdown Control 1
#define	INIT_DR15	0x00	//	Initialization Powerdown Control 2
#define	INIT_DR18	0xff	//	Normal Oper. Interrupt Register 1 (clear with 0xFF)
#define	INIT_DR19	0xff	//	Normal Oper. Interrupt Register 2 (clear with 0xFF)
#define	INIT_DR20	0xff	//	Normal Oper. Interrupt Register 3 (clear with 0xFF)
#define	INIT_DR21	0xff	//	Interrupt Mask 1
#define	INIT_DR22	0xff	//	Initialization Interrupt Mask 2
#define	INIT_DR23	0xff	//	 Initialization Interrupt Mask 3
#define	INIT_DR32	0x00	//	Oper. Oscillator 1 Control뾲one generation
#define	INIT_DR33	0x00	//	Oper. Oscillator 2 Control뾲one generation
#define	INIT_DR34	0X18	//	34 0x22 0x00 Initialization Ringing Oscillator Control
#define	INIT_DR35	0x00	//	Oper. Pulse Metering Oscillator Control
#define	INIT_DR36	0x00	//	36 0x24 0x00 Initialization OSC1 Active Low Byte (125 탎/LSB)
#define	INIT_DR37	0x00	//	37 0x25 0x00 Initialization OSC1 Active High Byte (125 탎/LSB)
#define	INIT_DR38	0x00	//	38 0x26 0x00 Initialization OSC1 Inactive Low Byte (125 탎/LSB)
#define	INIT_DR39	0x00	//	39 0x27 0x00 Initialization OSC1 Inactive High Byte (125 탎/LSB)
#define	INIT_DR40	0x00	//	40 0x28 0x00 Initialization OSC2 Active Low Byte (125 탎/LSB)
#define	INIT_DR41	0x00	//	41 0x29 0x00 Initialization OSC2 Active High Byte (125 탎/LSB)
#define	INIT_DR42	0x00	//	42 0x2A 0x00 Initialization OSC2 Inactive Low Byte (125 탎/LSB)
#define	INIT_DR43	0x00	//	43 0x2B 0x00 Initialization OSC2 Inactive High Byte (125 탎/LSB)
#define	INIT_DR44	0x00	//	44 0x2C 0x00 Initialization Pulse Metering Active Low Byte (125 탎/LSB)
#define	INIT_DR45	0x00	//	45 0x2D 0x00 Initialization Pulse Metering Active High Byte (125 탎/LSB)
#define	INIT_DR46	0x00	//	46 0x2E 0x00 Initialization Pulse Metering Inactive Low Byte (125 탎/LSB)
#define	INIT_DR47	0x00	//	47 0x2F 0x00 Initialization Pulse Metering Inactive High Byte (125 탎/LSB)
#define	INIT_DR48	0X80	//	48 0x30 0x00 0x80 Initialization Ringing Osc. Active Timer Low Byte (2 s,125 탎/LSB)
#define	INIT_DR49	0X3E	//	49 0x31 0x00 0x3E Initialization Ringing Osc. Active Timer High Byte (2 s,125 탎/LSB)
#define	INIT_DR50	0X00	//	50 0x32 0x00 0x00 Initialization Ringing Osc. Inactive Timer Low Byte (4 s, 125 탎/LSB)
#define	INIT_DR51	0X7D	//	51 0x33 0x00 0x7D Initialization Ringing Osc. Inactive Timer High Byte (4 s, 125 탎/LSB)
#define	INIT_DR52	0X00	//	52 0x34 0x00 Normal Oper. FSK Data Bit
#define	INIT_DR63	0X54	//	63 0x3F 0x54 Initialization Ringing Mode Loop Closure Debounce Interval
#define	INIT_DR64	0x00	//	64 0x40 0x00 Normal Oper. Mode Byte뾭rimary control
#define	INIT_DR65	0X61	//	65 0x41 0x61 Initialization External Bipolar Transistor Settings
#define	INIT_DR66	0X03	//	66 0x42 0x03 Initialization Battery Control
#define	INIT_DR67	0X1F	//	67 0x43 0x1F Initialization Automatic/Manual Control
#define	INIT_DR69	0X0C	//	69 0x45 0x0A 0x0C Initialization Loop Closure Debounce Interval (1.25 ms/LSB)
#define	INIT_DR70	0X0A	//	70 0x46 0x0A Initialization Ring Trip Debounce Interval (1.25 ms/LSB)
#define	INIT_DR71	0X01	//	71 0x47 0x00 0x01 Initialization Off-Hook Loop Current Limit (20 mA + 3 mA/LSB)
#define	INIT_DR72	0X20	//	72 0x48 0x20 Initialization On-Hook Voltage (open circuit voltage) = 48 V(1.5 V/LSB)
#define	INIT_DR73	0X02	//	73 0x49 0x02 Initialization Common Mode Voltage뾙CM = ? V(?.5 V/LSB)
#define	INIT_DR74	0X32	//	74 0x4A 0x32 Initialization VBATH (ringing) = ?5 V (?.5 V/LSB)
#define	INIT_DR75	0X10	//	75 0x4B 0x10 Initialization VBATL (off-hook) = ?4 V (TRACK = 0)(?.5 V/LSB)
#define	INIT_DR92	0x7f	//	92 0x5C  7F Initialization DC뺻C Converter PWM Period (61.035 ns/LSB)
#define	INIT_DR93	0x14	//	93 0x5D 0x14 0x19 Initialization DC뺻C Converter Min. Off Time (61.035 ns/LSB)
#define	INIT_DR96	0x00	//	96 0x60 0x1F Initialization Calibration Control Register 1(written second and starts calibration)
#define	INIT_DR97	0X1F	//	97 0x61 0x1F Initialization Calibration Control Register 2(written before Register 96)
#define	INIT_DR98	0X10	//	98 0x62 0x10 Informative Calibration result (see data sheet)
#define	INIT_DR99	0X10	//	99 0x63 0x10 Informative Calibration result (see data sheet)
#define	INIT_DR100	0X11	//	100 0x64 0x11 Informative Calibration result (see data sheet)
#define	INIT_DR101	0X11	//	101 0x65 0x11 Informative Calibration result (see data sheet)
#define	INIT_DR102	0x08	//	102 0x66 0x08 Informative Calibration result (see data sheet)
#define	INIT_DR103	0x88	//	103 0x67 0x88 Informative Calibration result (see data sheet)
#define	INIT_DR104	0x00	//	104 0x68 0x00 Informative Calibration result (see data sheet)
#define	INIT_DR105	0x00	//	105 0x69 0x00 Informative Calibration result (see data sheet)
#define	INIT_DR106	0x20	//	106 0x6A 0x20 Informative Calibration result (see data sheet)
#define	INIT_DR107	0x08	//	107 0x6B 0x08 Informative Calibration result (see data sheet)
#define	INIT_DR108	0xEB	//	108 0x63 0x00 0xEB Initialization Feature enhancement register
#define INIT_SI3210M_DR92	0x60  //  92 0x60 Initialization DC뺻C Converter PWM Period (61.035 ns/LSB)
#define INIT_SI3210M_DR93 	0x38  //  92 0x60 Initialization DC뺻C Converter PWM Period (61.035 ns/LSB)

#define DISABLE_ALL_DR21	0
#define DISABLE_ALL_DR22	0
#define DISABLE_ALL_DR23	0
#define	OPEN_DR64		0
#define	STANDARD_CAL_DR97	0x1E	/*	Calibrations without the ADC and DAC offset and without common mode calibration. */
#define	STANDARD_CAL_DR96	0x47	/*	Calibrate common mode and differential DAC mode DAC + ILIM */
#define ENB2_DR23		1<<2	/* enable interrupt for the balance Cal */
#define	BIT_CALCM_DR97		0x01	/*	CALCM Common Mode Balance Calibration. */



#define FSK_Bellcore	0
#define FSK_ETSI	1
#define FSK_BT		2
#define FSK_NTT		3


#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210
#define _SI3210_ 	1
#endif

/*----------Indirect Register of SI-321X(decimal)----------*/

#ifdef _SI3210_	/* SI-3210 */

#define	OSC1_COEF		13
#define	OSC1X			14
#define	OSC1Y			15
#define	OSC2_COEF		16
#define	OSC2X			17
#define	OSC2Y			18
#define	RING_V_OFF		19
#define	RING_OSC_COEF		20
#define	RING_X			21
#define	RING_Y			22

#define	RECV_DIGITAL_GAIN	26
#define	XMIT_DIGITAL_GAIN	27


#define	REVC			108		
#define	FSK_X_0			99	
#define	FSK_COEFF_0		100
#define	FSK_X_1			101
#define	FSK_COEFF_1		102
#define	FSK_X_01		103
#define	FSK_X_10		104

#else	/* SI-3215 */

#define	OSC1_COEF		0
#define	OSC1X			1
#define	OSC1Y			2
#define	OSC2_COEF		3
#define	OSC2X			4
#define	OSC2Y			5
#define	RING_V_OFF		6
#define	RING_OSC_COEF		7
#define	RING_X			8
#define	RING_Y			9
//#define	PULSE_ENVEL		10
//#define	PULSE_X			11
//#define	PULSE_Y			12
#define	RECV_DIGITAL_GAIN	13
#define	XMIT_DIGITAL_GAIN	14

#define	FSK_X_0			69	/*	x	sign	fsk_x_0[15:0]												*/
#define	FSK_COEFF_0		70	/*	x	sign	fsk_coeff_0[15:0]												*/
#define	FSK_X_1			71	/*	x	sign	fsk_x_1[15:0]												*/
#define	FSK_COEFF_1		72	/*	x	sign	fsk_coeff_1[15:0]												*/
#define	FSK_X_01		73	/*	x	sign	fsk_x_01[15:0]												*/
#define	FSK_X_10		74	/*	x	sign	fsk_x_10[15:0]	*/

#endif


/*---------- Direct Register for SI-321X(decimal) ---------*/

#define	PCM_MODE		1
#define	AUDIO_GAIN		9
#define	HYBRID			11

/* INTERRUPTS */
#define	INTRPT_STATUS1		18
#define	INTRPT_STATUS2		19
#define	INTRPT_STATUS3		20
#define	INTRPT_MASK1		21
#define	INTRPT_MASK2		22
#define	INTRPT_MASK3		23

/* OSCILLATORS */
#define	OSC1			32
#define	OSC2			33
#define	RING_OSC_CTL		34
//#define	PULSE_OSC	35
#define	OSC1_ON__LO		36
#define	OSC1_ON_HI		37
#define	OSC1_OFF_LO		38
#define	OSC1_OFF_HI		39
#define	OSC2_ON__LO		40
#define	OSC2_ON_HI		41
#define	OSC2_OFF_LO		42
#define	OSC2_OFF_HI		43

//#define	PULSE_ON__LO	44
//#define	PULSE_ON_HI	45
//#define	PULSE_OFF_LO	46
//#define	PULSE_OFF_HI	47
#define	RING_ON__LO		48
#define	RING_ON_HI		49
#define	RING_OFF_LO		50
#define	RING_OFF_HI		51
#define	FSK_DATA		52

#define	REVC			108	/*	0	ilim_max	fsk_revc	dc_err_en	zs_ext	batsel_pd	lcr_sense	en_subtr	hyst_en	*/




/*--------------------------------------------------------
 *    SLIC 321x Interrupt bits: direct 19
 *--------------------------------------------------------*/
#define RTIP (1<<0) /*Ring trip interrupt pending*/
#define LCIP (1<<1) /*Loop closure transition interrupt pending*/
#define Q1AP (1<<2) /*Q1 power alarm*/
#define Q2AP (1<<3) /*Q2 power alarm*/
#define Q3AP (1<<4) /*Q3 power alarm*/
#define Q4AP (1<<5) /*Q4 power alarm*/
#define Q5AP (1<<6) /*Q5 power alarm*/
#define Q6AP (1<<7) /*Q6 power alarm*/



extern tone_struct DialTone;
extern tone_struct ReorderTone;
extern tone_struct CongestionTone;
extern tone_struct RingbackPBX;
extern tone_struct RingbackNormal;
extern tone_struct BusySignal;
extern tone_struct RingbackJapan;
extern tone_struct BusyJapan;
extern tone_struct JapanDialTone;
extern tone_struct dual_tones[];

#endif
