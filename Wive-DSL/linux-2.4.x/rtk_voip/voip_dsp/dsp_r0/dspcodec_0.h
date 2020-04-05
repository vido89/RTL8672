
//
// dspcodec.h
//

#ifndef _DSPCODEC_H_
#define _DSPCODEC_H_

#ifdef __ECOS
#include <pkgconf/system.h>
#include <pkgconf/io.h>
#include <cyg/io/io.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/io/devtab.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_io.h>
#endif

#include "myTypes.h"
#include "dspcodec.h"
#include "voip_params.h"

#define G711PRETD           (500*80)			// (500*160)
#define G723PRETD           (500*240)
#define G729PRETD           (500*80)			// (500*160)
#define TDOFFSET_PO         64000               // 8sec
#define TDOFFSET_NE         (-64000)            // -8sec

// country tone set
typedef enum
{
	DSPCODEC_COUNTRY_USA,
	DSPCODEC_COUNTRY_UK,
	DSPCODEC_COUNTRY_AUSTRALIA,
	DSPCODEC_COUNTRY_HK,
	DSPCODEC_COUNTRY_JP,
	DSPCODEC_COUNTRY_SE,
	DSPCODEC_COUNTRY_GR,
	DSPCODEC_COUNTRY_FR,
	DSPCODEC_COUNTRY_TR,
	DSPCODEC_COUNTRY_BE,
	DSPCODEC_COUNTRY_FL,
	DSPCODEC_COUNTRY_IT,
	DSPCODEC_COUNTRY_CN,
	DSPCODEC_COUNTRY_CUSTOME
} DSPCODEC_COUNTRY;

// input/output interfaces
typedef enum
{
	DSPCODEC_INTERFACE_HANDSET,
	DSPCODEC_INTERFACE_HANDFREE,
	DSPCODEC_INTERFACE_HEADSET
} DSPCODEC_INTERFACE;

/*	handsome add function 2005.12.1     */
typedef struct aspToneCfgParam
{
	uint32	toneType;
	uint16	cycle;

	uint16	cadNUM;

	uint32	CadOn0;
	uint32	CadOn1;
	uint32	CadOn2;
	uint32	CadOn3;
	uint32	CadOff0;
	uint32	CadOff1;
	uint32	CadOff2;
	uint32	CadOff3;

	uint32 PatternOff;
	uint32 ToneNUM;

	uint32	Freq1;
	uint32	Freq2;
	uint32	Freq3;
	uint32	Freq4;

	int32 Gain1;
	int32 Gain2;
	int32 Gain3;
	int32 Gain4;

} aspToneCfgParam_t;

/*	handsome add function 2005.12.1     */
typedef struct gDSP_aspTone
{
	uint32 custID;
	aspToneCfgParam_t* DSP_aspToneCfg;
} gDSP_aspTone_t;


/*	handsome add function 2005.12.1     */
typedef struct aspToneCountryCfgParam
{
	uint8		CountryId;
	uint8		iDial;
	uint8		iRing;
	uint8		iBusy;
	uint8		iWaiting;

} gDSP_aspToneCountry_t;

// DSP interface functions
void   DspcodecInitVar(void);

void   DspcodecUp(uint32 sid);

void   DspcodecDown(void);

RESULT DspcodecInitialize(uint32 sid, DSPCODEC_ALGORITHM uCodingAlgorithm);

RESULT DspcodecSetConfig(uint32 sid, CDspcodecConfig* pConfig);

RESULT DspcodecGetConfig(uint32 sid, CDspcodecConfig* pConfig);

RESULT DspcodecStart(uint32 sid, DSPCODEC_ACTION uAction);

RESULT DspcodecStop(uint32 sid);

RESULT DspcodecMute(uint32 sid, DSPCODEC_MUTEDIRECTION uDirection);

RESULT DspcodecPlayTone(uint32 sid, DSPCODEC_TONE nTone, bool bFlag, DSPCODEC_TONEDIRECTION nPath);

int32  DspcodecWrite(uint32 chid, uint32 sid, uchar* pBuf, int32 nSize, uint32 nSeq, uint32 nTimestamp);

int32  DspcodecRead(uint32 chid, uint32 sid, uchar* pBuf, int32 nSize);

RESULT DspcodecSetVolume(int32 nVal);

RESULT DspcodecSetEncGain(int32 nVal);

RESULT DspcodecSetSidetoneGain(int32 nVal);

RESULT DspcodecSidetoneSwitch(bool bFlag);

RESULT DspcodecSetRingType(uint32 sid, int32 nRing);

RESULT DspcodecSetCountry(uint32 sid, int32 nCountry);

RESULT DspcodecSetInterface(DSPCODEC_INTERFACE nInterface);

int32  DspcodecGetVersion(char *pBuf, int32 nLen);

void   DspcodecResetR1(uint32 sid);

void   DspcodecDebugR1(uint32 sid);

int32 setTone(aspToneCfgParam_t* pToneCfg); // thlin modify

//int32 DSP_SetToneInfo(uint32* data);
int32 setToneCountry(gDSP_aspToneCountry_t* pCfg);					/*	handsome add function 2005.12.1     */

#endif	// _DSPCODEC_H_
