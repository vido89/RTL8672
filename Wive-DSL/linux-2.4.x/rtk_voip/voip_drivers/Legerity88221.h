#include "Slic_api.h"
#ifndef _Legerity88221_H_
#define _Legerity88221_H_
extern unsigned char slic_order;

extern char SiLabsID2[];
#define _Legerity_debug_
//--------------default filter value------------------------------------------//
//if define #define default_filter_value,programed filter values is not valid.
//#define _default_filter_value_
//----------------------------------------------------------------------------//

//----FSK for specified country. GR_30 for America, ITU_V23 for Europ.--------//
#define FSK_country 0  //0: GR_30, 1: ITU_V23

#if (FSK_country == 0)
	#define GR_30
#elif (FSK_country == 1)
	#define ITU_V23
#endif
//----------------------------------------------------------------------------//

/* mark by chiminer
typedef struct {
	unsigned char CH;		// CH = 0 ~ 3
	unsigned char ring_set;		// Ring_ON: ring_set = 1 ,  Ring_OFF: ring_set = 0 

} ring_struct;*/

typedef struct 
{
	unsigned char _DISN;
	unsigned char _ZFIR[10];
	unsigned char _ZIIR[5];
	unsigned char _RF[14];
	unsigned char _XF[12];
	unsigned char _GR[2];
	unsigned char _GX[2];
	unsigned char _ANLG;
	unsigned char _BFIR[14];
	unsigned char _BIIR[2];

} TstLe88221Impendance;

//---------Legerity88221.c function prototype---------//
void Legerity_slic_init_all(unsigned char slic_id);
void Legerity_hard_reset(void);
void Legerity_tx_rx_gain(unsigned char slic_id, Le88xxx *data, unsigned char wri_re);
void Legerity_TX_slic_gain(unsigned char slic_id, unsigned char txgain);
void Legerity_RX_slic_gain(unsigned char slic_id, unsigned char rxgain);

int Legerity_FXS_ring(ring_struct *slic_id);
unsigned char Legerity_system_state(unsigned char slic_id, unsigned char state, unsigned char wri_re);
unsigned char Legerity_HOOK_state(hook_struck *hook);
void Legerity_GPIO_dir_set(unsigned char slic_id, unsigned char IO1_dir, unsigned char IO2_dir);
void Legerity_GPIO_data(unsigned char slic_id, unsigned char IO1_data, unsigned char IO2_data);
void Legerity_DTMF_CallerID_main(unsigned char slic_id, char *data);
void Legerity_Set_ONHOOK_Trans(unsigned char slic_id);
void SLIC_Set_Ring_Cadence_ON_Legerity(unsigned char slic__order, unsigned short msec);
void SLIC_Set_Ring_Cadence_OFF_Legerity(unsigned char slic__order, unsigned short msec);
void SLIC_Set_Impendance_Legerity(unsigned char slic__order, unsigned short country, unsigned short impd);
void Legerity_slic_set_rx_pcm(int chid, int enable);
void Legerity_slic_set_tx_pcm(int chid, int enable);
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226
void Legerity_Wideband_ctrl(unsigned char select);
extern unsigned char wideband_mode_ctrl;
#endif
//----------------------------------------------------//


//----------------System state---------------------//
//for Legerity_system_state() used.//
#define DISCONNECT		0x00	//ACT = 0
#define TIP_OPEN		0x01	//ACT = 0
#define RING_OPEN		0x02	//ACT = 0
#define ACTIVE_LOW_BATTERY	0x03	//ACT = 1
#define LOWER_POWER_IDLE	0x04	//ACT = 0
#define RINGING			0x07	//ACT = 1
#define LOW_GAIN		0x08	//ACT = 1
#define ACTIVE_MID_BATTERY	0x0B	//ACT = 1
#define SHUTDOWN		0x0F	//ACT = 0
//------------------------------------------------//

//---------------HOOK state-----------------------//
#define On_hook			0x00
#define Off_hook		0x01
//------------------------------------------------//

//-------------Caller ID (FSK) parameter----------//
#ifdef GR_30
	#define FRQC		0x1777
	#define AMPC		0x27D4
	#define FRQD		0x0CCD
	#define AMPD		0x27D4
#endif

#ifdef ITU_V23
	#define FRQC		0x1666
	#define AMPC		0x27D4
	#define FRQD		0x0DDE
	#define AMPD		0x27D4
#endif
//-------------------------------------------------//


#define FSK_Bellcore	0
#define FSK_ETSI	1
#define FSK_BT		2
#define FSK_NTT		3



/************Caller ID (DTMF) parameter***********************/
#define	FRQC_FRQD(x,y)		(((int)((x)*2.73+1)<<16) | (int)(((y)*2.73+1)))
#define	DTMF_1			FRQC_FRQD(697,1209)
#define	DTMF_2			FRQC_FRQD(697,1336)
#define	DTMF_3			FRQC_FRQD(697,1477)
#define	DTMF_4			FRQC_FRQD(770,1209)
#define	DTMF_5			FRQC_FRQD(770,1336)
#define	DTMF_6			FRQC_FRQD(770,1477)
#define	DTMF_7			FRQC_FRQD(852,1209)
#define	DTMF_8			FRQC_FRQD(852,1336)
#define	DTMF_9			FRQC_FRQD(852,1477)
#define	DTMF_0			FRQC_FRQD(941,1336)
#define	DTMF_A			FRQC_FRQD(697,1633)
#define	DTMF_B			FRQC_FRQD(770,1633)
#define	DTMF_C			FRQC_FRQD(852,1633)
#define	DTMF_D			FRQC_FRQD(941,1633)
#define AMP			0x27D4
/***************************************************/

//-----------Interrupt mask bit--------------------//
#define unmaskbit(x)		(1<<(x))
#define MCFAIL			unmaskbit(7)
#define MOCALMY_Z		unmaskbit(6)
#define MTEMPA			unmaskbit(5)
#define MIO			unmaskbit(4)
#define MCAD			unmaskbit(3)
#define MCID			unmaskbit(2)
#define MGNK			unmaskbit(1)
#define MHOOK			unmaskbit(0)
//-------------------------------------------------//

//-------Switching regulator parameter and control-------------//
#define S_R_Byte1		0x00
#define S_R_Byte2		0x05
#define S_R_Byte3		0x80
#define S_R_Ctrl		0x07
//-------------------------------------------------//

//------Loop supervision parameter------------------//
#define	L_S_Byte1		0x1B//0x19
#define	L_S_Byte2		0x84//0x88
#define	L_S_Byte3		0xC6//0xA4
#define	L_S_Byte4		0x05//0x00
//--------------------------------------------------//

//------DC Feed parameter---------------------------//
#define DC_FEED_Byte1		0x31
#define DC_FEED_Byte2		0x08
//--------------------------------------------------//

//-------The frequency and amplitude Generator A, B and Biad--------//
//frqA: 20Hz, ampA: 75V. frqB:0 Hz, ampB: 0V. Bias: 0V.
#define OTHER_FEATURE		0x00
#define BIAS			0x0000
#define FRQA			0x0037
#define AMPA			0x3E2D
#define FRQB			0x0000
#define AMPB			0x0000
//------------------------------------------------------------------//

//-------Cadence timer coefficient----------------//
//Cadence on: 2sec, cadence off: 2sec.
#define CADON			0x0190
#define CADOFF			0x0190
//------------------------------------------------//

//-------R filter coefficient---------------------//
#define RCm46_36		0x2D//0xDC
#define RCm26_16		0x01//0x01
#define RCm40_30		0x2B//0x22
#define RCm20_10		0xB0//0xD0
#define RCm41_31		0x5A//0x2A
#define RCm21_11		0x33//0xA4
#define RCm42_32		0x24//0x3B
#define RCm22_12		0x5C//0x3F
#define RCm43_33		0x35//0x3F
#define RCm23_13		0xA4//0x9F
#define RCm44_34		0x5A//0xAA
#define RCm24_14		0x3D//0x97
#define RCm45_35		0x33//0xAB
#define RCm25_15		0xB6//0x8F
//wideband used
#define RCm46_36_w		0xDC
#define RCm26_16_w		0x01
#define RCm40_30_w		0x3A
#define RCm20_10_w		0xA0
#define RCm41_31_w		0x25
#define RCm21_11_w		0xB2
#define RCm42_32_w		0xBA
#define RCm22_12_w		0x2D
#define RCm43_33_w		0x52
#define RCm23_13_w		0x46
#define RCm44_34_w		0xAB
#define RCm24_14_w		0xDF
#define RCm45_35_w		0xCD
#define RCm25_15_w		0x36
//--------------------------------------------------//
 
//-------GR filter coefficient---------------------//
#define GRCm40_30		0xA8//0xA2
#define GRCm20_10		0x71//0x21
//wideband
#define GRCm40_30_w		0xA2
#define GRCm20_10_w		0xB0
//-------------------------------------------------//

//-------X filter coefficient----------------------//
#define XCm40_30		0x3A//0xA8
#define XCm20_10		0x10//0x70
#define XCm41_31		0x3D//0xAD
#define XCm21_11		0x3D//0x8F
#define XCm42_32		0xB2//0xBD
#define XCm22_12		0xA7//0xAE
#define XCm43_33		0x6B//0xFB
#define XCm23_13		0xA5//0x97
#define XCm44_34		0x2A//0x5A
#define XCm24_14		0xCE//0x27
#define XCm45_35		0x2A//0x32
#define XCm25_15		0x8F//0xBE
//wideband used
#define XCm40_30_w		0x28
#define XCm20_10_w		0x60
#define XCm41_31_w		0xF3
#define XCm21_11_w		0x2D
#define XCm42_32_w		0xAA
#define XCm22_12_w		0xA5
#define XCm43_33_w		0xBD
#define XCm23_13_w		0xA5
#define XCm44_34_w		0x2A
#define XCm24_14_w		0xCD
#define XCm45_35_w		0x42
#define XCm25_15_w		0xD7
//--------------------------------------------------//

//------GX filter coefficient-----------------------//
#define GXCm40_30		0xA9//0x32
#define GXCm20_10		0xF0//0x10
//wideband used
#define GXCm40_30_w		0x62
#define GXCm20_10_w		0x20
//-------------------------------------------------//

//------B FIR filter coefficient-------------------//
#define BCm32_22		0x2A//0x2A
#define BCm12_33		0x42//0x43
#define BCm23_13		0x22//0x22
#define BCm34_24		0x4B//0x4B
#define BCm14_35		0x1C//0x1A
#define BCm25_15		0xA3//0xC3
#define BCm36_26		0xA8//0xB8
#define BCm16_37		0xFF//0xFA
#define BCm27_17		0x8F//0x8F
#define BCm38_28		0xAA//0xD9
#define BCm18_39		0xF5//0xF7
#define BCm29_19		0x9F//0x9F
#define BCm310_210		0xBA//0xB9
#define BCm110			0xF0//0xF0
//wideband used
#define BCm32_22_w		0x2B
#define BCm12_33_w		0x5E
#define BCm23_13_w		0x22
#define BCm34_24_w		0xBC
#define BCm14_35_w		0x17
#define BCm25_15_w		0xA3
#define BCm36_26_w		0xC9
#define BCm16_37_w		0xF9
#define BCm27_17_w		0x85
#define BCm38_28_w		0xAB
#define BCm18_39_w		0xEA
#define BCm29_19_w		0x8F
#define BCm310_210_w		0xA9
#define BCm110_w			0xF0
//-------------------------------------------------//

//------B IIR filter coefficient-------------------//
#define BCm411_311		0x2E
#define BCm211_111		0x01
//wideband used
#define BCm411_311_w		0x2E
#define BCm211_111_w		0x01
//-------------------------------------------------//

//------Z FIR filter coefficient-------------------//
#define ZCm40_30		0xBA//0x22
#define ZCm20_10		0xEB//0x3D
#define ZCm41_31		0x2A//0xA2
#define ZCm21_11		0x2C//0xAD
#define ZCm42_32		0xB5//0xAF
#define ZCm22_12		0x25//0xA6
#define ZCm43_33		0xAA//0x2E
#define ZCm23_13		0x24//0xA5
#define ZCm44_34		0x2C//0x23
#define ZCm24_14		0x3D//0xAE
//wideband used
#define ZCm40_30_w		0xBA
#define ZCm20_10_w		0x2A
#define ZCm41_31_w		0x4A
#define ZCm21_11_w		0x3B
#define ZCm42_32_w		0x98
#define ZCm22_12_w		0xA3
#define ZCm43_33_w		0x42
#define ZCm23_13_w		0xA2
#define ZCm44_34_w		0x2C
#define ZCm24_14_w		0xAB
//-------------------------------------------------//
 
//------Z IIR filter coefficient-------------------//
#define ZCm45_35		0xAA//0x23
#define ZCm25_15		0xBA//0xCA
#define ZCm26_16		0x27//0x37
#define ZCm47_37		0x9F//0x9F
#define ZCm27_17		0x01//0x01
//wideband used
#define ZCm45_35_w		0xCC
#define ZCm25_15_w		0xAA
#define ZCm26_16_w		0x37
#define ZCm47_37_w		0x9F
#define ZCm27_17_w		0x01
//-------------------------------------------------//

//----DISN coefficient-----------------------------//
#define DISN			0xEA//0xF5
//wideband used
#define DISN_w			0xFC
//-------------------------------------------------//
#endif //_Legerity88221_H_
