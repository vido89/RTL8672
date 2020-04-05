#ifndef __CODEC_DESCRIPTOR_H__
#define __CODEC_DESCRIPTOR_H__

#include "rtk_voip.h"

#ifdef CONFIG_RTK_VOIP_G7231
#define NUM_G723_CODEC_TYPE		1
#define NUM_G723_CODEC_ALGO		2	/* 5.3k / 6.3k */
#define NUM_G723_CODEC_PAYLOAD	1
#else
#define NUM_G723_CODEC_TYPE		0
#define NUM_G723_CODEC_ALGO		0
#define NUM_G723_CODEC_PAYLOAD	0
#endif

#ifdef CONFIG_RTK_VOIP_G729AB
#define NUM_G729_CODEC_TYPE		1
#define NUM_G729_CODEC_ALGO		1
#define NUM_G729_CODEC_PAYLOAD	1
#else
#define NUM_G729_CODEC_TYPE		0
#define NUM_G729_CODEC_ALGO		0
#define NUM_G729_CODEC_PAYLOAD	0
#endif

#ifdef CONFIG_RTK_VOIP_G726
#define NUM_G726_CODEC_TYPE		1
#define NUM_G726_CODEC_ALGO		4	/* 16, 24, 32, 40k */
#define NUM_G726_CODEC_PAYLOAD	4	/* 16, 24, 32, 40k */
#else
#define NUM_G726_CODEC_TYPE		0
#define NUM_G726_CODEC_ALGO		0
#define NUM_G726_CODEC_PAYLOAD	0
#endif

#ifdef CONFIG_RTK_VOIP_GSMFR
#define NUM_GSMFR_CODEC_TYPE	1
#define NUM_GSMFR_CODEC_ALGO	1
#define NUM_GSMFR_CODEC_PAYLOAD	1
#else
#define NUM_GSMFR_CODEC_TYPE	0
#define NUM_GSMFR_CODEC_ALGO	0
#define NUM_GSMFR_CODEC_PAYLOAD	0
#endif

#ifdef CONFIG_RTK_VOIP_T38
#define NUM_T38_CODEC_TYPE		1
#define NUM_T38_CODEC_ALGO		1
#define NUM_T38_CODEC_PAYLOAD	1
#else
#define NUM_T38_CODEC_TYPE		0
#define NUM_T38_CODEC_ALGO		0
#define NUM_T38_CODEC_PAYLOAD	0
#endif

/* ==================================================================
 * Codec classified by algorithm (711a, 711u, 726 16k/24k/32k/40k ...)
 * ================================================================== */
#include "rtl_types.h"
#include "../dsp_r1/include/dspcodec.h"

/* ****************************************************************** 
 * Numerical Definition
 * ****************************************************************** */
#define NUM_OF_ALGO_CODEC_DESC		( 2 /* G711 a/u-law */ + 						\
									  NUM_G723_CODEC_ALGO + NUM_G729_CODEC_ALGO +	\
									  NUM_G726_CODEC_ALGO + NUM_GSMFR_CODEC_ALGO +	\
									  NUM_T38_CODEC_ALGO )

/* used to get frame length, SID length, and frame timestamp */
typedef enum FRAME_INFO_S {
	FRAME_INFO_FRAME_LENGTH,
	FRAME_INFO_VOLATILE_FRAME_LENGTH,
	FRAME_INFO_SID_LENGTH,
	FRAME_INFO_FRAME_TIMESTAMP,
} FRAME_INFO_T;

/* ****************************************************************** 
 * Structure Definition
 * ****************************************************************** */
typedef uint32 ( *FN_GetFrameInfo_FrameLength )( const unsigned char *pBuffer );
typedef uint32 ( *FN_GetFrameInfo_SidLength )( uint32 nSize );
typedef uint32 ( *FN_GetNumberOfFramesInCurrentPacket ) ( 
											const unsigned char *pBuffer,
											uint32 nSize, 
											int TempLen,
											int TempLenSID,
											int *pbAppendixSID );
typedef void ( *FN_R1CmdParserInitialize )( uint32 sid, const CDspcodecInitializeParm *pInitializeParam );

typedef struct codec_algo_desc_s {
	DSPCODEC_ALGORITHM codecAlgorithm;
	/* R0: === Dspcodec write === */	
	FN_GetFrameInfo_FrameLength fnGetR0FrameLength;
	int bR0VolatileFrameLength;
	FN_GetFrameInfo_SidLength fnGetR0SidLength;
	uint32 nR0FrameTimestamp;
	FN_GetNumberOfFramesInCurrentPacket fnGetNumberOfFramesInCurrentPacket;
	/* R1: CmdParser */
	FN_R1CmdParserInitialize fnR1CmdParserInitialize;
} codec_algo_desc_t;

extern const codec_algo_desc_t g_codecAlgoDesc[];

/* ==================================================================
 * Codec classified by type (711, 726 ...)
 * ================================================================== */
//#include "../dsp_r1/r1_main.h"

/* If NUM_OF_CODEC_TYPE_DESC is 4, then it indicates 0 ~ 3. */
#define NUM_OF_CODEC_TYPE_DESC		( 1 /* G711 */ + 								\
									  NUM_G723_CODEC_TYPE + NUM_G729_CODEC_TYPE +	\
									  NUM_G726_CODEC_TYPE +	NUM_GSMFR_CODEC_TYPE +	\
									  NUM_T38_CODEC_TYPE )

/* ****************************************************************** 
 * Structure Definition
 * ****************************************************************** */
typedef void ( *FN_CodecDecPhase )( uint32 sid );
typedef void ( *FN_CodecEncPhase )( uint32 sid );
typedef void ( *FN_AddDecPlaytoneBuffer )( uint32 sid, int *pFrameLen );
typedef void ( *FN_HighPassFiltering )( uint32 sid );
typedef void ( *FN_R1CmdParserStart )( uint32 sid, const CDspcodecStartParm *pStartParam );
typedef void ( *FN_R1CmdParserStop )( uint32 sid );
typedef int  ( *FN_IsJbcSidFrame )( uint32 sid, uint32 nSize );
typedef void ( *FN_SetCodecMem )(int state, int g726_rate);
typedef void ( *FN_SetLecG168SyncPoint )( unsigned int chid );

typedef enum {
	UDP_CARRY_TYPE_RTP	= 0,	/* IP + UDP + RTP */
	UDP_CARRY_TYPE_T38	= 1,	/* IP + UDP + T38 */
} UdpCarryType_t;

typedef struct codec_type_desc_s {
	enum START_CODEC_TYPE codecStartType;
	/* R1: r1_main */
	enum CODEC_STATE *pCodecState;
	FN_CodecDecPhase fnCodecDecPhase;
	FN_CodecEncPhase fnCodecEncPhase;
	FN_AddDecPlaytoneBuffer fnAddDecPlaytoneBuffer;
	FN_HighPassFiltering fnHighPassFiltering;
	uint16 nToneBudgetSize;
	/* R1: CmdParser */
	FN_R1CmdParserStart fnR1CmdParserStart;
	FN_R1CmdParserStop fnR1CmdParserStop;
	/* R1: JBC */
	FN_IsJbcSidFrame fnIsJbcSidFrame;
	uint32 nFramePeriod;
	/* R1: codec mem */
#ifndef CONFIG_RTK_VOIP_MODULE
	FN_SetCodecMem fnSetCodecMem;
#endif /* !CONFIG_RTK_VOIP_MODULE */
	/* PCM handler */
	uint32 nDspProcessCutStep;
	/* UDP send + recv */
	UdpCarryType_t nUdpCarryType;
} codec_type_desc_t;

extern const codec_type_desc_t g_codecTypeDesc[];

/* ==================================================================
 * Codec classified by rtp payload type (711u, 711a, 726 ...)
 * ================================================================== */

#define NUM_OF_CODEC_PAYLOAD_DESC	( 2 /* G711 a/u */ + 								\
									  NUM_G723_CODEC_PAYLOAD + NUM_G729_CODEC_PAYLOAD +	\
									  NUM_G726_CODEC_PAYLOAD + NUM_GSMFR_CODEC_PAYLOAD +	\
									  NUM_T38_CODEC_PAYLOAD )

typedef uint32 ( *FN_GetTxNumberOfFrame )( uint32 nSize, const char *pBuffer );

typedef struct codec_payload_desc_s {
	RtpPayloadType payloadType;
	/* map payload type --> algorithm */
	DSPCODEC_ALGORITHM codecAlgorithm;
	DSPCODEC_ALGORITHM codecAlgorithm2;
	/* rtp main */
	int nRecvFrameRate;
	int nTranFrameRate;
	const char *pszSetCodecPrompt;
	uint32 nFrameBytes;
	uint32 nHThresTxFramePerPacket;
	/* rtp term */
	int nSidTxFrameBytes;
	uint32 nFramePeriod;
	/* rtp transmitter */
	uint32 nFrameTimestamp;
	FN_GetTxNumberOfFrame fnGetTxNumberOfFrame;
} codec_payload_desc_t;

extern const codec_payload_desc_t g_codecPayloadDesc[];

/* ==================================================================
 * Others
 * ================================================================== */

/* ****************************************************************** 
 * Help Functions
 * ****************************************************************** */

/* Check functions */
extern int IsAnyCodecDecodeState( uint32 sid );
extern int IsJbcSidFrameOfThisCodec( uint32 sid, uint32 nSize );
extern enum START_CODEC_TYPE GetGlobalStartCodecType( uint32 sid );
extern int CheckG711StartCodecType( uint32 sid );
extern int CheckG723StartCodecType( uint32 sid );
extern int CheckG729StartCodecType( uint32 sid );

/* ---------------- Get codec type functions ---------------- */
extern const enum START_CODEC_TYPE nCodecTypeID_G711;
extern const enum START_CODEC_TYPE nCodecTypeID_G723;
extern const enum START_CODEC_TYPE nCodecTypeID_G729;
extern const enum START_CODEC_TYPE nCodecTypeID_G726;
extern const enum START_CODEC_TYPE nCodecTypeID_GSMfr;
extern const enum START_CODEC_TYPE nCodecTypeID_T38;

/* ---------------- codec algorithm ID declare ---------------- */
extern const DSPCODEC_ALGORITHM nCodecAlgorithm_G7231A63;
extern const DSPCODEC_ALGORITHM nCodecAlgorithm_G72616;
extern const DSPCODEC_ALGORITHM nCodecAlgorithm_G72624;
extern const DSPCODEC_ALGORITHM nCodecAlgorithm_G72632;
extern const DSPCODEC_ALGORITHM nCodecAlgorithm_G72640;

/* Get descriptor functions */
extern const codec_payload_desc_t *GetCodecPayloadDesc( RtpPayloadType payloadType );
extern const codec_type_desc_t *GetCodecTypeDesc( enum START_CODEC_TYPE type );
extern const codec_algo_desc_t *GetCodecAlgoDesc( DSPCODEC_ALGORITHM algo );

/* Do something for all codec */
extern void InitializeAllCodecState( uint32 sid );
extern void DoAllCodecDecPhase( uint32 sid );
extern void DoAllCodecEncPhase( uint32 sid );

/* Misc */
extern void NullAddDecPlaytoneBuffer( uint32 sid, int *pFrameLen );

/* ****************************************************************** 
 * Assistant Definition
 * ****************************************************************** */
#define CT_ASSERT( expr )		extern int __ct_assert[ 2 * ( expr ) - 1 ]

#endif /* __CODEC_DESCRIPTOR_H__ */

