		/******************************************************
		 *						      *
		 *	Realtek VoIP-ATA DTMF DECODER HEADER FILE     *
		 *						      *
		 *	Author : thlin@realtek.com.tw		      *
		 *						      *
		 *	Date: 2005-10-11			      *
		 *						      *
		 *	Copyright 2005 Realtek Semiconductor Corp.    *
		 *						      *
		 ******************************************************/

#ifndef DTMF_DEC_H
#define DTMF_DEC_H

#include "../../include/rtk_voip.h"
#ifdef FXO_CALLER_ID_DET
#include "fsk_det.h"
extern TstVoipciddet cid_res;
#endif

/*
 ***************************************************************************************************************************
 * Description of sturcture Dtmf_det_out:
 * Assume DTMF key "6" is pressed for several seconds, digit response "6" only one time when DTMF detecter detect it. 
 * However, digitOnOff response result of each detection. I.e. digitOnOff response "6" while DTMF key "6" is still pressed.
 ***************************************************************************************************************************
 */

typedef enum
{
	DTMF_POWER_LEVEL_MINUS_5DBM = 0x120000,
	DTMF_POWER_LEVEL_MINUS_32DBM = 0x6FF

}TdtmfPowerLevel;


typedef struct
{
	char digit;	 
	char digitOnOff; 

}Dtmf_det_out;


/* dtmf_dec.c function prototype */

Dtmf_det_out dtmf_dec(unsigned char *adr, uint32 page_size, unsigned char CH, TdtmfPowerLevel dtmf_pwr_level);
void dtmf_start(unsigned char CH);
void dtmf_stop(unsigned char CH);
void dtmf_cid_det(unsigned char *adr, uint32 page_size, unsigned char daa_chid, TdtmfPowerLevel level);
void dmtf_cid_det_init(void);

#endif
