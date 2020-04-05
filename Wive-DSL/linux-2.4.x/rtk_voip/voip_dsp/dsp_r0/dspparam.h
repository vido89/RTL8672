
//
// dspparam.h
//

#ifndef _DSPPARAM_H_
#define _DSPPARAM_H_

#include "myTypes.h"
#include "rtk_voip.h"

// tone types
#define ADDITIVE		0
#define MODULATED		1
#define SUCC			2
#define SUCC_ADD		3

#define MAX_CAD_NUM		4		// max candence number
#define MAX_FREQUENCY		4		// max Frequency number

/* ToneTable index enumerator */
typedef enum
{
	DIGIT_0,				//941+1336 (cont.)
	DIGIT_1,				//697+1209 (cont.)
	DIGIT_2,				//697+1336 (cont.)
	DIGIT_3,				//697+1477 (cont.)
	DIGIT_4,				//770+1209 (cont.)
	DIGIT_5,				//770+1336 (cont.)
	DIGIT_6,				//770+1477 (cont.)
	DIGIT_7,				//852+1209 (cont.)
	DIGIT_8,				//852+1336 (cont.)
	DIGIT_9,				//852+1477 (cont.)
	DIGIT_STAR,				//941+1209 (cont.)
	DIGIT_PONDA,				//941+1477 (cont.)

	OFFHOOKWARING,				//950/1400/1800 (3*0.33)
	HOLDING,				//400/524 (0.5 - 2.5 - 0.5 - 2.0)
	
	// ringing tone
	RING_1,					//1000/1067 (0.083 - 0.083)
	RING_2,					//900/1067 (0.083 - 0.083)
	RING_3,					//800/1067 (0.083 - 0.083)
	RING_4,					//700/1067 (0.083 - 0.083)
	RING_5,					//600/1067 (0.083 - 0.083)
	RING_6,					//500/1067 (0.083 - 0.083)				
	RING_7,					//800/1067   30(rpt)*(2(tone)*0.033(on)) - 4.0(off)
	RING_8,					//1333/1455/1621  40(rpt)*(3(tone)*0.017(on)) - 4.0(off)
	RING_9,					//800/1067/1333   30(rpt)*(3(tone)*0.017(on)) - 4.0(off)
	RING_10,				//800/925/1037    30(rpt)*(3(tone)*0.035(on)) - 4.0(off)

#ifdef SUPPORT_TONE_PROFILE
	// USA
	USA_DIAL,				// 350+440 (cont.)
	USA_STUTTERDIAL,			// 350+440 3*(0.1 - 0.1)
	USA_MESSAGE_WAITING,			// 350+440 10*(0.1 - 0.1)
	USA_CONFIRMATION,			// 350+440 3*(0.1 - 0.1)
	USA_RING,				// 440+480 (2.0 - 4.0)
	USA_BUSY,				// 480+620 (0.5 - 0.5)
	USA_CONGESTION,				// 480+620 (0.24 - 0.26)
	USA_ROH,				// 1400+2060+2450+2600 (0.1 - 0.1)
	USA_DOUBLE_RING,			// 440+480 (0.5 - 0.2 - 0.5 - 4.0)
	USA_SIT_NOCIRCUIT,			// 950/1400/1750 (3*0.5 - 1)
	USA_SIT_INTERCEPT,			// 950/1400/1750 (0.25(on)+0.25(on)+0.5(on) - 1(off))
	USA_SIT_VACANT,				// 950/1400/1750 (0.4(on)+0.25(on)+0.4(on) - 1(off))
	USA_SIT_REORDER,			// 950/1400/1750 (0.25(on)+0.4(on)+0.4(on) - 1(off))
	USA_CALLING_CARD_WITHEVENT,		// (941+1477)/(440+350) (0.06(on) - 1 (off) - 0.94(on) - 1(off))
	USA_CALLING_CARD,			// (941+1477)/(440+350) (0.06(on) + 0.94(on))
	USA_CALL_WAITING_1,			// 440 (0.3 - 10.0)
	USA_CALL_WAITING_2,			// 440 (0.1 - 0.1 - 0.1 - 10.0)
	USA_CALL_WAITING_3,			// 440 (0.1 - 0.1 - 0.1 - 0.1 - 0.1 - 10.0)
	USA_CALL_WAITING_4,			// 440 (0.1 - 0.1 - 0.3 - 0.1 - 0.1 - 10.0)
//	USA_CLASS2SAS_CAS1,			// 
//	USA_CLASS2SAS_CAS2,			// 
//	USA_CLASS2SAS_CAS3,			// 
//	USA_CLASS2SAS_CAS4,			// 
	USA_INGRESS_RINGBACK,			// 440+480+100 (2.0 - 4.0)

	// UK
	UK_DIAL,				// 350+440 (cont.)
	UK_STUTTERDIAL,				// 350+440 3*(0.1 - 0.1)
	UK_MESSAGE_WAITING,			// 350+440 10*(0.1 - 0.1)
	UK_CONFIRMATION,			// 350+440 3*(0.1 - 0.1)
	UK_RING,				// 440+480 (2.0 - 4.0)
	UK_BUSY,				// 480+620 (0.5 - 0.5)
	UK_CONGESTION,				// 480+620 (0.24 - 0.26)
	UK_ROH,					// 1400+2060+2450+2600 (0.1 - 0.1)
	UK_DOUBLE_RING,				// 440+480 (0.5 - 0.2 - 0.5 - 4.0)
	UK_SIT_NOCIRCUIT,			// 950/1400/1750 (3*0.5 - 1)
	UK_SIT_INTERCEPT,			// 950/1400/1750 (0.25(on)+0.25(on)+0.5(on) - 1(off))
	UK_SIT_VACANT,				// 950/1400/1750 (0.4(on)+0.25(on)+0.4(on) - 1(off))
	UK_SIT_REORDER,				// 950/1400/1750 (0.25(on)+0.4(on)+0.4(on) - 1(off))
	UK_CALLING_CARD_WITHEVENT,		// (941+1477)/(440+350) (0.06(on) - 1 (off) - 0.94(on) - 1(off))
	UK_CALLING_CARD,			// (941+1477)/(440+350) (0.06(on) + 0.94(on))
	UK_CALL_WAITING_1,			// 440 (0.3 - 10.0)
	UK_CALL_WAITING_2,			// 440 (0.1 - 0.1 - 0.1 - 10.0)
	UK_CALL_WAITING_3,			// 440 (0.1 - 0.1 - 0.1 - 0.1 - 0.1 - 10.0)
	UK_CALL_WAITING_4,			// 440 (0.1 - 0.1 - 0.3 - 0.1 - 0.1 - 10.0)
//	UK_CLASS2SAS_CAS1,			// 
//	UK_CLASS2SAS_CAS2,			// 
//	UK_CLASS2SAS_CAS3,			// 
//	UK_CLASS2SAS_CAS4,			// 
	UK_INGRESS_RINGBACK,			// 440+480+100 (2.0 - 4.0)

	// AUSTRALIA
	AUSTRALIA_DIAL,				// 425*25 (cont.)
	AUSTRALIA_STUTTERDIAL,			// 350+440 3*(0.1 - 0.1)
	AUSTRALIA_MESSAGE_WAITING,		// 350+440 10*(0.1 - 0.1)
	AUSTRALIA_CONFIRMATION,			// 350+440 3*(0.1 - 0.1)
	AUSTRALIA_RING,				// 400*17 (0.4 - 0.2 - 0.4 - 2.0)
	AUSTRALIA_BUSY,				// 400 (0.375 - 0.375)
	AUSTRALIA_CONGESTION,			// 400 (0.375 - 0.375)
	AUSTRALIA_ROH,				// 1400+2060+2450+2600 (0.1 - 0.1)
	AUSTRALIA_DOUBLE_RING,			// 440+480 (0.5 - 0.2 - 0.5 - 4.0)
	AUSTRALIA_SIT_NOCIRCUIT,		// 950/1400/1750 (3*0.5 - 1)
	AUSTRALIA_SIT_INTERCEPT,		// 950/1400/1750 (0.25(on)+0.25(on)+0.5(on) - 1(off))
	AUSTRALIA_SIT_VACANT,			// 950/1400/1750 (0.4(on)+0.25(on)+0.4(on) - 1(off))
	AUSTRALIA_SIT_REORDER,			// 950/1400/1750 (0.25(on)+0.4(on)+0.4(on) - 1(off))
	AUSTRALIA_CALLING_CARD_WITHEVENT,	// (941+1477)/(440+350) (0.06(on) - 1 (off) - 0.94(on) - 1(off))
	AUSTRALIA_CALLING_CARD,			// (941+1477)/(440+350) (0.06(on) + 0.94(on))
	AUSTRALIA_CALL_WAITING_1,		// 440 (0.3 - 10.0)
	AUSTRALIA_CALL_WAITING_2,		// 440 (0.1 - 0.1 - 0.1 - 10.0)
	AUSTRALIA_CALL_WAITING_3,		// 440 (0.1 - 0.1 - 0.1 - 0.1 - 0.1 - 10.0)
	AUSTRALIA_CALL_WAITING_4,		// 440 (0.1 - 0.1 - 0.3 - 0.1 - 0.1 - 10.0)
//	AUSTRALIA_CLASS2SAS_CAS1,		// 
//	AUSTRALIA_CLASS2SAS_CAS2,		// 
//	AUSTRALIA_CLASS2SAS_CAS3,		// 
//	AUSTRALIA_CLASS2SAS_CAS4,		// 
	AUSTRALIA_INGRESS_RINGBACK,		// 440+480+100 (2.0 - 4.0)

	// Hong Kong
	HK_DIAL,				// 350+440 (cont.)
	HK_STUTTERDIAL,				// 350+440 3*(0.1 - 0.1)
	HK_MESSAGE_WAITING,			// 350+440 10*(0.1 - 0.1)
	HK_CONFIRMATION,			// 350+440 3*(0.1 - 0.1)
	HK_RING,				// 440+480 (2.0 - 4.0)
	HK_BUSY,				// 480+620 (0.5 - 0.5)
	HK_CONGESTION,				// 480+620 (0.24 - 0.26)
	HK_ROH,					// 1400+2060+2450+2600 (0.1 - 0.1)
	HK_DOUBLE_RING,				// 440+480 (0.5 - 0.2 - 0.5 - 4.0)
	HK_SIT_NOCIRCUIT,			// 950/1400/1750 (3*0.5 - 1)
	HK_SIT_INTERCEPT,			// 950/1400/1750 (0.25(on)+0.25(on)+0.5(on) - 1(off))
	HK_SIT_VACANT,				// 950/1400/1750 (0.4(on)+0.25(on)+0.4(on) - 1(off))
	HK_SIT_REORDER,				// 950/1400/1750 (0.25(on)+0.4(on)+0.4(on) - 1(off))
	HK_CALLING_CARD_WITHEVENT,		// (941+1477)/(440+350) (0.06(on) - 1 (off) - 0.94(on) - 1(off))
	HK_CALLING_CARD,			// (941+1477)/(440+350) (0.06(on) + 0.94(on))
	HK_CALL_WAITING_1,			// 440 (0.3 - 10.0)
	HK_CALL_WAITING_2,			// 440 (0.1 - 0.1 - 0.1 - 10.0)
	HK_CALL_WAITING_3,			// 440 (0.1 - 0.1 - 0.1 - 0.1 - 0.1 - 10.0)
	HK_CALL_WAITING_4,			// 440 (0.1 - 0.1 - 0.3 - 0.1 - 0.1 - 10.0)
//	HK_CLASS2SAS_CAS1,			// 
//	HK_CLASS2SAS_CAS2,			// 
//	HK_CLASS2SAS_CAS3,			// 
//	HK_CLASS2SAS_CAS4,			// 
	HK_INGRESS_RINGBACK,			// 440+480+100 (2.0 - 4.0)

	// Japan
	JP_DIAL,				// 350+440 (cont.)
	JP_STUTTERDIAL,				// 350+440 3*(0.1 - 0.1)
	JP_MESSAGE_WAITING,			// 350+440 10*(0.1 - 0.1)
	JP_CONFIRMATION,			// 350+440 3*(0.1 - 0.1)
	JP_RING,				// 440+480 (2.0 - 4.0)
	JP_BUSY,				// 480+620 (0.5 - 0.5)
	JP_CONGESTION,				// 480+620 (0.24 - 0.26)
	JP_ROH,					// 1400+2060+2450+2600 (0.1 - 0.1)
	JP_DOUBLE_RING,				// 440+480 (0.5 - 0.2 - 0.5 - 4.0)
	JP_SIT_NOCIRCUIT,			// 950/1400/1750 (3*0.5 - 1)
	JP_SIT_INTERCEPT,			// 950/1400/1750 (0.25(on)+0.25(on)+0.5(on) - 1(off))
	JP_SIT_VACANT,				// 950/1400/1750 (0.4(on)+0.25(on)+0.4(on) - 1(off))
	JP_SIT_REORDER,				// 950/1400/1750 (0.25(on)+0.4(on)+0.4(on) - 1(off))
	JP_CALLING_CARD_WITHEVENT,		// (941+1477)/(440+350) (0.06(on) - 1 (off) - 0.94(on) - 1(off))
	JP_CALLING_CARD,			// (941+1477)/(440+350) (0.06(on) + 0.94(on))
	JP_CALL_WAITING_1,			// 440 (0.3 - 10.0)
	JP_CALL_WAITING_2,			// 440 (0.1 - 0.1 - 0.1 - 10.0)
	JP_CALL_WAITING_3,			// 440 (0.1 - 0.1 - 0.1 - 0.1 - 0.1 - 10.0)
	JP_CALL_WAITING_4,			// 440 (0.1 - 0.1 - 0.3 - 0.1 - 0.1 - 10.0)
//	JP_CLASS2SAS_CAS1,			// 
//	JP_CLASS2SAS_CAS2,			// 
//	JP_CLASS2SAS_CAS3,			// 
//	JP_CLASS2SAS_CAS4,			// 
	JP_INGRESS_RINGBACK,			// 440+480+100 (2.0 - 4.0)

	// Sweden
	SE_DIAL,				// 350+440 (cont.)
	SE_STUTTERDIAL,				// 350+440 3*(0.1 - 0.1)
	SE_MESSAGE_WAITING,			// 350+440 10*(0.1 - 0.1)
	SE_CONFIRMATION,			// 350+440 3*(0.1 - 0.1)
	SE_RING,				// 440+480 (2.0 - 4.0)
	SE_BUSY,				// 480+620 (0.5 - 0.5)
	SE_CONGESTION,				// 480+620 (0.24 - 0.26)
	SE_ROH,					// 1400+2060+2450+2600 (0.1 - 0.1)
	SE_DOUBLE_RING,				// 440+480 (0.5 - 0.2 - 0.5 - 4.0)
	SE_SIT_NOCIRCUIT,			// 950/1400/1750 (3*0.5 - 1)
	SE_SIT_INTERCEPT,			// 950/1400/1750 (0.25(on)+0.25(on)+0.5(on) - 1(off))
	SE_SIT_VACANT,				// 950/1400/1750 (0.4(on)+0.25(on)+0.4(on) - 1(off))
	SE_SIT_REORDER,				// 950/1400/1750 (0.25(on)+0.4(on)+0.4(on) - 1(off))
	SE_CALLING_CARD_WITHEVENT,		// (941+1477)/(440+350) (0.06(on) - 1 (off) - 0.94(on) - 1(off))
	SE_CALLING_CARD,			// (941+1477)/(440+350) (0.06(on) + 0.94(on))
	SE_CALL_WAITING_1,			// 440 (0.3 - 10.0)
	SE_CALL_WAITING_2,			// 440 (0.1 - 0.1 - 0.1 - 10.0)
	SE_CALL_WAITING_3,			// 440 (0.1 - 0.1 - 0.1 - 0.1 - 0.1 - 10.0)
	SE_CALL_WAITING_4,			// 440 (0.1 - 0.1 - 0.3 - 0.1 - 0.1 - 10.0)
//	SE_CLASS2SAS_CAS1,			// 
//	SE_CLASS2SAS_CAS2,			// 
//	SE_CLASS2SAS_CAS3,			// 
//	SE_CLASS2SAS_CAS4,			// 
	SE_INGRESS_RINGBACK,			// 440+480+100 (2.0 - 4.0)

	// Germany
	GR_DIAL,				// 350+440 (cont.)
	GR_STUTTERDIAL,				// 350+440 3*(0.1 - 0.1)
	GR_MESSAGE_WAITING,			// 350+440 10*(0.1 - 0.1)
	GR_CONFIRMATION,			// 350+440 3*(0.1 - 0.1)
	GR_RING,				// 440+480 (2.0 - 4.0)
	GR_BUSY,				// 480+620 (0.5 - 0.5)
	GR_CONGESTION,				// 480+620 (0.24 - 0.26)
	GR_ROH,					// 1400+2060+2450+2600 (0.1 - 0.1)
	GR_DOUBLE_RING,				// 440+480 (0.5 - 0.2 - 0.5 - 4.0)
	GR_SIT_NOCIRCUIT,			// 950/1400/1750 (3*0.5 - 1)
	GR_SIT_INTERCEPT,			// 950/1400/1750 (0.25(on)+0.25(on)+0.5(on) - 1(off))
	GR_SIT_VACANT,				// 950/1400/1750 (0.4(on)+0.25(on)+0.4(on) - 1(off))
	GR_SIT_REORDER,				// 950/1400/1750 (0.25(on)+0.4(on)+0.4(on) - 1(off))
	GR_CALLING_CARD_WITHEVENT,		// (941+1477)/(440+350) (0.06(on) - 1 (off) - 0.94(on) - 1(off))
	GR_CALLING_CARD,			// (941+1477)/(440+350) (0.06(on) + 0.94(on))
	GR_CALL_WAITING_1,			// 440 (0.3 - 10.0)
	GR_CALL_WAITING_2,			// 440 (0.1 - 0.1 - 0.1 - 10.0)
	GR_CALL_WAITING_3,			// 440 (0.1 - 0.1 - 0.1 - 0.1 - 0.1 - 10.0)
	GR_CALL_WAITING_4,			// 440 (0.1 - 0.1 - 0.3 - 0.1 - 0.1 - 10.0)
//	GR_CLASS2SAS_CAS1,			// 
//	GR_CLASS2SAS_CAS2,			// 
//	GR_CLASS2SAS_CAS3,			// 
//	GR_CLASS2SAS_CAS4,			// 
	GR_INGRESS_RINGBACK,			// 440+480+100 (2.0 - 4.0)

	// France
	FR_DIAL,				// 350+440 (cont.)
	FR_STUTTERDIAL,				// 350+440 3*(0.1 - 0.1)
	FR_MESSAGE_WAITING,			// 350+440 10*(0.1 - 0.1)
	FR_CONFIRMATION,			// 350+440 3*(0.1 - 0.1)
	FR_RING,				// 440+480 (2.0 - 4.0)
	FR_BUSY,				// 480+620 (0.5 - 0.5)
	FR_CONGESTION,				// 480+620 (0.24 - 0.26)
	FR_ROH,					// 1400+2060+2450+2600 (0.1 - 0.1)
	FR_DOUBLE_RING,				// 440+480 (0.5 - 0.2 - 0.5 - 4.0)
	FR_SIT_NOCIRCUIT,			// 950/1400/1750 (3*0.5 - 1)
	FR_SIT_INTERCEPT,			// 950/1400/1750 (0.25(on)+0.25(on)+0.5(on) - 1(off))
	FR_SIT_VACANT,				// 950/1400/1750 (0.4(on)+0.25(on)+0.4(on) - 1(off))
	FR_SIT_REORDER,				// 950/1400/1750 (0.25(on)+0.4(on)+0.4(on) - 1(off))
	FR_CALLING_CARD_WITHEVENT,		// (941+1477)/(440+350) (0.06(on) - 1 (off) - 0.94(on) - 1(off))
	FR_CALLING_CARD,			// (941+1477)/(440+350) (0.06(on) + 0.94(on))
	FR_CALL_WAITING_1,			// 440 (0.3 - 10.0)
	FR_CALL_WAITING_2,			// 440 (0.1 - 0.1 - 0.1 - 10.0)
	FR_CALL_WAITING_3,			// 440 (0.1 - 0.1 - 0.1 - 0.1 - 0.1 - 10.0)
	FR_CALL_WAITING_4,			// 440 (0.1 - 0.1 - 0.3 - 0.1 - 0.1 - 10.0)
//	FR_CLASS2SAS_CAS1,			// 
//	FR_CLASS2SAS_CAS2,			// 
//	FR_CLASS2SAS_CAS3,			// 
//	FR_CLASS2SAS_CAS4,			// 
	FR_INGRESS_RINGBACK,			// 440+480+100 (2.0 - 4.0)

	// Tr57
	TR_DIAL,				// 350+440 (cont.)
	TR_STUTTERDIAL,				// 350+440 3*(0.1 - 0.1)
	TR_MESSAGE_WAITING,			// 350+440 10*(0.1 - 0.1)
	TR_CONFIRMATION,			// 350+440 3*(0.1 - 0.1)
	TR_RING,				// 440+480 (2.0 - 4.0)
	TR_BUSY,				// 480+620 (0.5 - 0.5)
	TR_CONGESTION,				// 480+620 (0.24 - 0.26)
	TR_ROH,					// 1400+2060+2450+2600 (0.1 - 0.1)
	TR_DOUBLE_RING,				// 440+480 (0.5 - 0.2 - 0.5 - 4.0)
	TR_SIT_NOCIRCUIT,			// 950/1400/1750 (3*0.5 - 1)
	TR_SIT_INTERCEPT,			// 950/1400/1750 (0.25(on)+0.25(on)+0.5(on) - 1(off))
	TR_SIT_VACANT,				// 950/1400/1750 (0.4(on)+0.25(on)+0.4(on) - 1(off))
	TR_SIT_REORDER,				// 950/1400/1750 (0.25(on)+0.4(on)+0.4(on) - 1(off))
	TR_CALLING_CARD_WITHEVENT,		// (941+1477)/(440+350) (0.06(on) - 1 (off) - 0.94(on) - 1(off))
	TR_CALLING_CARD,			// (941+1477)/(440+350) (0.06(on) + 0.94(on))
	TR_CALL_WAITING_1,			// 440 (0.3 - 10.0)
	TR_CALL_WAITING_2,			// 440 (0.1 - 0.1 - 0.1 - 10.0)
	TR_CALL_WAITING_3,			// 440 (0.1 - 0.1 - 0.1 - 0.1 - 0.1 - 10.0)
	TR_CALL_WAITING_4,			// 440 (0.1 - 0.1 - 0.3 - 0.1 - 0.1 - 10.0)
//	TR_CLASS2SAS_CAS1,			// 
//	TR_CLASS2SAS_CAS2,			// 
//	TR_CLASS2SAS_CAS3,			// 
//	TR_CLASS2SAS_CAS4,			// 
	TR_INGRESS_RINGBACK,			// 440+480+100 (2.0 - 4.0)

	// Belgium
	BE_DIAL,				// 350+440 (cont.)
	BE_STUTTERDIAL,				// 350+440 3*(0.1 - 0.1)
	BE_MESSAGE_WAITING,			// 350+440 10*(0.1 - 0.1)
	BE_CONFIRMATION,			// 350+440 3*(0.1 - 0.1)
	BE_RING,				// 440+480 (2.0 - 4.0)
	BE_BUSY,				// 480+620 (0.5 - 0.5)
	BE_CONGESTION,				// 480+620 (0.24 - 0.26)
	BE_ROH,					// 1400+2060+2450+2600 (0.1 - 0.1)
	BE_DOUBLE_RING,				// 440+480 (0.5 - 0.2 - 0.5 - 4.0)
	BE_SIT_NOCIRCUIT,			// 950/1400/1750 (3*0.5 - 1)
	BE_SIT_INTERCEPT,			// 950/1400/1750 (0.25(on)+0.25(on)+0.5(on) - 1(off))
	BE_SIT_VACANT,				// 950/1400/1750 (0.4(on)+0.25(on)+0.4(on) - 1(off))
	BE_SIT_REORDER,				// 950/1400/1750 (0.25(on)+0.4(on)+0.4(on) - 1(off))
	BE_CALLING_CARD_WITHEVENT,		// (941+1477)/(440+350) (0.06(on) - 1 (off) - 0.94(on) - 1(off))
	BE_CALLING_CARD,			// (941+1477)/(440+350) (0.06(on) + 0.94(on))
	BE_CALL_WAITING_1,			// 440 (0.3 - 10.0)
	BE_CALL_WAITING_2,			// 440 (0.1 - 0.1 - 0.1 - 10.0)
	BE_CALL_WAITING_3,			// 440 (0.1 - 0.1 - 0.1 - 0.1 - 0.1 - 10.0)
	BE_CALL_WAITING_4,			// 440 (0.1 - 0.1 - 0.3 - 0.1 - 0.1 - 10.0)
//	BE_CLASS2SAS_CAS1,			// 
//	BE_CLASS2SAS_CAS2,			// 
//	BE_CLASS2SAS_CAS3,			// 
//	BE_CLASS2SAS_CAS4,			// 
	BE_INGRESS_RINGBACK,			// 440+480+100 (2.0 - 4.0)

	// Finland
	FL_DIAL,				// 350+440 (cont.)
	FL_STUTTERDIAL,				// 350+440 3*(0.1 - 0.1)
	FL_MESSAGE_WAITING,			// 350+440 10*(0.1 - 0.1)
	FL_CONFIRMATION,			// 350+440 3*(0.1 - 0.1)
	FL_RING,				// 440+480 (2.0 - 4.0)
	FL_BUSY,				// 480+620 (0.5 - 0.5)
	FL_CONGESTION,				// 480+620 (0.24 - 0.26)
	FL_ROH,					// 1400+2060+2450+2600 (0.1 - 0.1)
	FL_DOUBLE_RING,				// 440+480 (0.5 - 0.2 - 0.5 - 4.0)
	FL_SIT_NOCIRCUIT,			// 950/1400/1750 (3*0.5 - 1)
	FL_SIT_INTERCEPT,			// 950/1400/1750 (0.25(on)+0.25(on)+0.5(on) - 1(off))
	FL_SIT_VACANT,				// 950/1400/1750 (0.4(on)+0.25(on)+0.4(on) - 1(off))
	FL_SIT_REORDER,				// 950/1400/1750 (0.25(on)+0.4(on)+0.4(on) - 1(off))
	FL_CALLING_CARD_WITHEVENT,		// (941+1477)/(440+350) (0.06(on) - 1 (off) - 0.94(on) - 1(off))
	FL_CALLING_CARD,			// (941+1477)/(440+350) (0.06(on) + 0.94(on))
	FL_CALL_WAITING_1,			// 440 (0.3 - 10.0)
	FL_CALL_WAITING_2,			// 440 (0.1 - 0.1 - 0.1 - 10.0)
	FL_CALL_WAITING_3,			// 440 (0.1 - 0.1 - 0.1 - 0.1 - 0.1 - 10.0)
	FL_CALL_WAITING_4,			// 440 (0.1 - 0.1 - 0.3 - 0.1 - 0.1 - 10.0)
//	FL_CLASS2SAS_CAS1,			// 
//	FL_CLASS2SAS_CAS2,			// 
//	FL_CLASS2SAS_CAS3,			// 
//	FL_CLASS2SAS_CAS4,			// 
	FL_INGRESS_RINGBACK,			// 440+480+100 (2.0 - 4.0)

	// Italy
	IT_DIAL,				// 350+440 (cont.)
	IT_STUTTERDIAL,				// 350+440 3*(0.1 - 0.1)
	IT_MESSAGE_WAITING,			// 350+440 10*(0.1 - 0.1)
	IT_CONFIRMATION,			// 350+440 3*(0.1 - 0.1)
	IT_RING,				// 440+480 (2.0 - 4.0)
	IT_BUSY,				// 480+620 (0.5 - 0.5)
	IT_CONGESTION,				// 480+620 (0.24 - 0.26)
	IT_ROH,					// 1400+2060+2450+2600 (0.1 - 0.1)
	IT_DOUBLE_RING,				// 440+480 (0.5 - 0.2 - 0.5 - 4.0)
	IT_SIT_NOCIRCUIT,			// 950/1400/1750 (3*0.5 - 1)
	IT_SIT_INTERCEPT,			// 950/1400/1750 (0.25(on)+0.25(on)+0.5(on) - 1(off))
	IT_SIT_VACANT,				// 950/1400/1750 (0.4(on)+0.25(on)+0.4(on) - 1(off))
	IT_SIT_REORDER,				// 950/1400/1750 (0.25(on)+0.4(on)+0.4(on) - 1(off))
	IT_CALLING_CARD_WITHEVENT,		// (941+1477)/(440+350) (0.06(on) - 1 (off) - 0.94(on) - 1(off))
	IT_CALLING_CARD,			// (941+1477)/(440+350) (0.06(on) + 0.94(on))
	IT_CALL_WAITING_1,			// 440 (0.3 - 10.0)
	IT_CALL_WAITING_2,			// 440 (0.1 - 0.1 - 0.1 - 10.0)
	IT_CALL_WAITING_3,			// 440 (0.1 - 0.1 - 0.1 - 0.1 - 0.1 - 10.0)
	IT_CALL_WAITING_4,			// 440 (0.1 - 0.1 - 0.3 - 0.1 - 0.1 - 10.0)
//	IT_CLASS2SAS_CAS1,			// 
//	IT_CLASS2SAS_CAS2,			// 
//	IT_CLASS2SAS_CAS3,			// 
//	IT_CLASS2SAS_CAS4,			// 
	IT_INGRESS_RINGBACK,			// 440+480+100 (2.0 - 4.0)

	// China
	CN_DIAL,				// 450 (cont.)
	CN_STUTTERDIAL,				// 450 (4.0 - 0.4)
	CN_MESSAGE_WAITING,			// 400 (cont.)
	CN_CONFIRMATION,			// 350+440 3*(0.1 - 0.1)
	CN_RING,				// 440+480 (2.0 - 4.0)
	CN_BUSY,				// 480+620 (0.5 - 0.5)
	CN_CONGESTION,				// 480+620 (0.24 - 0.26)
	CN_ROH,					// 1400+2060+2450+2600 (0.1 - 0.1)
	CN_DOUBLE_RING,				// 440+480 (0.5 - 0.2 - 0.5 - 4.0)
	CN_SIT_NOCIRCUIT,			// 950/1400/1750 (3*0.5 - 1)
	CN_SIT_INTERCEPT,			// 950/1400/1750 (0.25(on)+0.25(on)+0.5(on) - 1(off))
	CN_SIT_VACANT,				// 950/1400/1750 (0.4(on)+0.25(on)+0.4(on) - 1(off))
	CN_SIT_REORDER,				// 950/1400/1750 (0.25(on)+0.4(on)+0.4(on) - 1(off))
	CN_CALLING_CARD_WITHEVENT,		// (941+1477)/(440+350) (0.06(on) - 1 (off) - 0.94(on) - 1(off))
	CN_CALLING_CARD,			// (941+1477)/(440+350) (0.06(on) + 0.94(on))
	CN_CALL_WAITING_1,			// 440 (0.3 - 10.0)
	CN_CALL_WAITING_2,			// 440 (0.1 - 0.1 - 0.1 - 10.0)
	CN_CALL_WAITING_3,			// 440 (0.1 - 0.1 - 0.1 - 0.1 - 0.1 - 10.0)
	CN_CALL_WAITING_4,			// 440 (0.1 - 0.1 - 0.3 - 0.1 - 0.1 - 10.0)
//	CN_CLASS2SAS_CAS1,			// 
//	CN_CLASS2SAS_CAS2,			// 
//	CN_CLASS2SAS_CAS3,			// 
//	CN_CLASS2SAS_CAS4,			// 
	CN_INGRESS_RINGBACK,			// 440+480+100 (2.0 - 4.0)

	// Customer Spec.
	CUSTOM_DIAL,				// 350+440 (cont.)
	CUSTOM_STUTTERDIAL,			// 350+440 3*(0.1 - 0.1)
	CUSTOM_MESSAGE_WAITING,			// 350+440 10*(0.1 - 0.1)
	CUSTOM_CONFIRMATION,			// 350+440 3*(0.1 - 0.1)
	CUSTOM_RING,				// 440+480 (2.0 - 4.0)
	CUSTOM_BUSY,				// 480+620 (0.5 - 0.5)
	CUSTOM_CONGESTION,			// 480+620 (0.24 - 0.26)
	CUSTOM_ROH,				// 1400+2060+2450+2600 (0.1 - 0.1)
	CUSTOM_DOUBLE_RING,			// 440+480 (0.5 - 0.2 - 0.5 - 4.0)
	CUSTOM_SIT_NOCIRCUIT,			// 950/1400/1750 (3*0.5 - 1)
	CUSTOM_SIT_INTERCEPT,			// 950/1400/1750 (0.25(on)+0.25(on)+0.5(on) - 1(off))
	CUSTOM_SIT_VACANT,			// 950/1400/1750 (0.4(on)+0.25(on)+0.4(on) - 1(off))
	CUSTOM_SIT_REORDER,			// 950/1400/1750 (0.25(on)+0.4(on)+0.4(on) - 1(off))
	CUSTOM_CALLING_CARD_WITHEVENT,		// (941+1477)/(440+350) (0.06(on) - 1 (off) - 0.94(on) - 1(off))
	CUSTOM_CALLING_CARD,			// (941+1477)/(440+350) (0.06(on) + 0.94(on))
	CUSTOM_CALL_WAITING_1,			// 440 (0.3 - 10.0)
	CUSTOM_CALL_WAITING_2,			// 440 (0.1 - 0.1 - 0.1 - 10.0)
	CUSTOM_CALL_WAITING_3,			// 440 (0.1 - 0.1 - 0.1 - 0.1 - 0.1 - 10.0)
	CUSTOM_CALL_WAITING_4,			// 440 (0.1 - 0.1 - 0.3 - 0.1 - 0.1 - 10.0)
//	CUSTOM_CLASS2SAS_CAS1,			// 
//	CUSTOM_CLASS2SAS_CAS2,			// 
//	CUSTOM_CLASS2SAS_CAS3,			// 
//	CUSTOM_CLASS2SAS_CAS4,			// 
	CUSTOM_INGRESS_RINGBACK,		// 440+480+100 (2.0 - 4.0)

#else

	USA_DIAL,				//350+440 (cont.)
	USA_RING,				//440+480 (2.0 - 4.0)
	USA_BUSY,				//480+620 (0.5 - 0.5)
	USA_CONGESTION,				//480+620 (0.25 - 0.25)
	USA_SPEC_INFO,				//950/1400/1800 (3*0.33)
	USA_WARING,				//440 (2.0 - 10.0 - 0.5 - 10)
	USA_RECORD,				//1400 (0.5 - 15.0)
	USA_WAITING,				//440 (0.3 - 10.0)

	UK_DIAL,				//350+440 (cont.)
	UK_RING,				//400+450 (0.4 - 0.2 - 0.4 - 2.0)
	UK_BUSY,				//400 (0.375 - 0.375)
	UK_CONGESTION,				//400 (0.4 - 0.35 - 0.225 - 0.525)
	UK_NUM_UNOB,				//400 (cont.)
	UK_PAY_RECG,				//1200/800 (0.2 - 0.2 - 0.2 - 2.0)
	UK_PAY,					//400 (0.125 - 0.125)
	UK_WAITING,				//440 (0.3 - 10.0)

	AUSTRALIA_DIAL,				//425*25 (cont.)
	AUSTRALIA_RING,				//400*17 (0.4 - 0.2 - 0.4 - 2.0)
	AUSTRALIA_BUSY,				//400 (0.375 - 0.375)
	AUSTRALIA_CONGESTION,			//400 (0.375 - 0.375)
	AUSTRALIA_WARING,			//425 (0.1 - 0.1 - 0.1 - 4.7)
	AUSTRALIA_NUM_UNOB,			//400 (2.5 - 0.5)
	AUSTRALIA_RECORD,			//1400 (0.425 - 14.525)
	AUSTRALIA_PAY_RECG,			//1100+1750/750+1450 (0.075 - 0.15 - 0.075 - 2.7)
	AUSTRALIA_WAITING,			//425/525 (0.1 - 0.1 - 0.1 - 4.7)

	HK_DIAL,				//350+440 (cont.)
	HK_RING,				//400+450 (0.4 - 0.2 - 0.4 - 2.0)
	HK_BUSY,				//400 (0.375 - 0.375)
	HK_CONGESTION,				//400 (0.4 - 0.35 - 0.225 - 0.525)
	HK_WAITING,				//440 (0.3 - 10.0)

	JP_DIAL,				//350+440 (cont.)
	JP_RING,				//400+450 (0.4 - 0.2 - 0.4 - 2.0)
	JP_BUSY,				//400 (0.375 - 0.375)
	JP_CONGESTION,				//400 (0.4 - 0.35 - 0.225 - 0.525)
	JP_WAITING,				//440 (0.3 - 10.0)

	SE_DIAL,				//350+440 (cont.)
	SE_RING,				//400+450 (0.4 - 0.2 - 0.4 - 2.0)
	SE_BUSY,				//400 (0.375 - 0.375)
	SE_CONGESTION,				//400 (0.4 - 0.35 - 0.225 - 0.525)
	SE_WAITING,				//440 (0.3 - 10.0)

	GR_DIAL,				//350+440 (cont.)
	GR_RING,				//400+450 (0.4 - 0.2 - 0.4 - 2.0)
	GR_BUSY,				//400 (0.375 - 0.375)
	GR_CONGESTION,				//400 (0.4 - 0.35 - 0.225 - 0.525)
	GR_WAITING,				//440 (0.3 - 10.0)

	FR_DIAL,				//350+440 (cont.)
	FR_RING,				//400+450 (0.4 - 0.2 - 0.4 - 2.0)
	FR_BUSY,				//400 (0.375 - 0.375)
	FR_CONGESTION,				//400 (0.4 - 0.35 - 0.225 - 0.525)
	FR_WAITING,				//440 (0.3 - 10.0)

	TR_DIAL,				//350+440 (cont.)
	TR_RING,				//400+450 (0.4 - 0.2 - 0.4 - 2.0)
	TR_BUSY,				//400 (0.375 - 0.375)
	TR_CONGESTION,				//400 (0.4 - 0.35 - 0.225 - 0.525)
	TR_WAITING,				//440 (0.3 - 10.0)

	BE_DIAL,				//350+440 (cont.)
	BE_RING,				//400+450 (0.4 - 0.2 - 0.4 - 2.0)
	BE_BUSY,				//400 (0.375 - 0.375)
	BE_CONGESTION,				//400 (0.4 - 0.35 - 0.225 - 0.525)
	BE_WAITING,				//440 (0.3 - 10.0)

	FL_DIAL,				//350+440 (cont.)
	FL_RING,				//400+450 (0.4 - 0.2 - 0.4 - 2.0)
	FL_BUSY,				//400 (0.375 - 0.375)
	FL_CONGESTION,				//400 (0.4 - 0.35 - 0.225 - 0.525)
	FL_WAITING,				//440 (0.3 - 10.0)

	IT_DIAL,				//350+440 (cont.)
	IT_RING,				//400+450 (0.4 - 0.2 - 0.4 - 2.0)
	IT_BUSY,				//400 (0.375 - 0.375)
	IT_CONGESTION,				//400 (0.4 - 0.35 - 0.225 - 0.525)
	IT_WAITING,				//440 (0.3 - 10.0)

	CN_DIAL,				//350+440 (cont.)
	CN_RING,				//400+450 (0.4 - 0.2 - 0.4 - 2.0)
	CN_BUSY,				//400 (0.375 - 0.375)
	CN_CONGESTION,				//400 (0.4 - 0.35 - 0.225 - 0.525)
	CN_WAITING,				//440 (0.3 - 10.0)

#endif	/* #ifdef SUPPORT_TONE_PROFILE */

/*	handsome add function 2005.12.1     */
	CUSTOM_TONE1,
	CUSTOM_TONE2,
	CUSTOM_TONE3,
	CUSTOM_TONE4,
	CUSTOM_TONE5,
	CUSTOM_TONE6,
	CUSTOM_TONE7,
	CUSTOM_TONE8,

#ifdef SW_DTMF_CID

	//========= for DTMF CID =========
	DIGIT_A,				// (48) 697+1633 (cont.) transmit start	
	DIGIT_B,				// (49) 770+1633 (cont.) transmit infomation
	DIGIT_C,				// (50) 852+1633 (cont.) stop
	DIGIT_D,				// (51) 941+1633 (cont.) forward start 
	//================================

#endif
	// sandro+ 2006/07/24 for SAS tone
	FSK_SAS,					// (52) 440 alert signal
	// hc+ 1229 for off hook FSK CID
	FSK_ALERT,				// (52) 2130+2750 alert signal
	NTT_IIT_TONE,


	// thlin+ continous DTMF tone play for RFC2833
	DIGIT_0_CONT,				//
	DIGIT_1_CONT,				//
	DIGIT_2_CONT,				//
	DIGIT_3_CONT,				//
	DIGIT_4_CONT,				//
	DIGIT_5_CONT,				//
	DIGIT_6_CONT,				//
	DIGIT_7_CONT,				//
	DIGIT_8_CONT,				//
	DIGIT_9_CONT,				//
	DIGIT_STAR_CONT,			//
	DIGIT_PONDA_CONT,			//
	DIGIT_A_CONT,				// 
	DIGIT_B_CONT,				// 
	DIGIT_C_CONT,				// 
	DIGIT_D_CONT				// 


} TONES ;

typedef enum
{
	JB_ADPTSPEED_SLOW,
	JB_ADPTSPEED_MEDIUM,
	JB_ADPTSPEED_FAST
} JB_ADPTSPEED;

typedef enum
{
	JB_TRCKSENS_LOW,
	JB_TRCKSENS_MEDIUM,
	JB_TRCKSENS_MEDIUMHIGH,
	JB_TRCKSENS_HIGH
} JB_TRCKSENS;

typedef struct
{
	JB_ADPTSPEED	speed;
	JB_TRCKSENS		sensitivity;
	int				factor;
	int				max_delay;
	int				min_delay;
}
JB_CONTROL_PARAMETER;


#ifdef SUPPORT_TONE_PROFILE
typedef struct __CadCfgParam
{
	Word16 ON;
	Word16 OFF;
} CadCfgParam_t;

typedef struct __ToneCfgParam
{
	Word16	ToneType;
	Word16	cycle;

	Word16	cadNUM;

	Word16	CadOn0;
	Word16	CadOff0;
	Word16	CadOn1;
	Word16	CadOff1;
	Word16	CadOn2;
	Word16	CadOff2;
	Word16	CadOn3;
	Word16	CadOff3;

	Word16 PatternOff;
	Word16 ToneNUM;

	Word16 Freq0;
	Word16 Freq1;
	Word16 Freq2;
	Word16 Freq3;
	Word16 Gain0;
	Word16 Gain1;
	Word16 Gain2;
	Word16 Gain3;
} ToneCfgParam_t;
#endif	// #ifdef SUPPORT_TONE_PROFILE

#ifdef SUPPORT_TONE_PROFILE
extern ToneCfgParam_t ToneTable[];
#else
extern short ToneTable[][21];
#endif
extern JB_CONTROL_PARAMETER CtrlParam;

#endif // _DSPPARAM_H_
