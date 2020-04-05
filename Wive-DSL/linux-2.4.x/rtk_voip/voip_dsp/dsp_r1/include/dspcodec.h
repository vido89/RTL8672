#ifndef R1_DSPCODEC_H
#define R1_DSPCODEC_H

//
// dspcodec.h
//

#include "typedef.h"
#include "voip_params.h"
#include "codec_def.h"

#ifdef SUPPORT_DYNAMIC_PAYLOAD
 #define BUFFER_SIZE_DSP_DEC_R1     80	/* max size of one frame is 80 bytes, when g711 codec */
 #ifdef MULTI_FRAME_BUFFER_SIZE
  #define BUFFER_SIZE_DSP_ENC_R1	MULTI_FRAME_BUFFER_SIZE
 #else
  #define BUFFER_SIZE_DSP_ENC_R1	640	/* 640/80 = 8 (Frames per packet) */
 #endif
 #define BUFFER_NUM_DEC		64	// modified 256 -> 64 (keep it to be 2^n)
 #define BUFFER_NUM_ENC		8//32	/* Since SID can generate many 'NotTx', its size should be greater than freames per packet. */
#else
 #define BUFFER_SIZE_DSP_DEC_R1		640 // modified 320 -> 640
 #define BUFFER_SIZE_DSP_ENC_R1		640 // modified 320 -> 640
 #define BUFFER_NUM_DEC		64	// modified 256 -> 64 (keep it to be 2^n)
 #define BUFFER_NUM_ENC		32
#endif /* SUPPORT_DYNAMIC_PAYLOAD */

// error type returned
typedef enum 
{
	DSPCODEC_SUCCESS,
	DSPCODEC_ERROR_ALGORITHM,
	DSPCODEC_ERROR_ACTION,
	DSPCODEC_ERROR_MUTEDIRECTION,
	DSPCODEC_ERROR_TONE,
	DSPCODEC_ERROR_DATALENGTH,
	DSPCODEC_ERROR_RESPONSE,
	DSPCODEC_ERROR_PARAMETER
}DSPCODEC_ERROR_TYPE;

// which command send to RISC1
typedef enum
{
    DSPCODEC_COMMAND_INITIALIZE,
    DSPCODEC_COMMAND_SETCONFIG,
    DSPCODEC_COMMAND_START,
    DSPCODEC_COMMAND_STARTMIXING,
    DSPCODEC_COMMAND_STOP,
    DSPCODEC_COMMAND_MUTE,
    DSPCODEC_COMMAND_PLAYTONE,
    DSPCODEC_COMMAND_JITTERSYNC,
    DSPCODEC_COMMAND_GETVERSION,
    DSPCODEC_COMMAND_DEBUG,
    DSPCODEC_COMMAND_ALIVE
} DSPCODEC_COMMAND;

// acknowledge got from RISC1
typedef enum
{
    DSPCODEC_ACK_INITIALIZE_DONE,
    DSPCODEC_ACK_SETCONFIG_DONE,
    DSPCODEC_ACK_START_DONE,
    DSPCODEC_ACK_STARTMIXING_DONE,
    DSPCODEC_ACK_STOP_DONE,
    DSPCODEC_ACK_MUTE_DONE,
    DSPCODEC_ACK_PLAYTONE_DONE,
    DSPCODEC_ACK_JITTERSYNC_DONE,
    DSPCODEC_ACK_GETVERSION_DONE,
    DSPCODEC_ACK_DEBUG,
    DSPCODEC_ACK_ALIVE,
    DSPCODEC_ACK_PRINTMSG,
    DSPCODEC_ACK_ILLEGAL_COMMAND
} DSPCODEC_ACK;

// how to play data
typedef enum
{
    DSPCODEC_ACTION_NORMAL,                     // encode + decode
    DSPCODEC_ACTION_ENCODE,
    DSPCODEC_ACTION_DECODE
} DSPCODEC_ACTION;

// which direction to mute
typedef enum
{
    DSPCODEC_MUTEDIRECTION_IN,
    DSPCODEC_MUTEDIRECTION_OUT,
    DSPCODEC_MUTEDIRECTION_BOTH,
    DSPCODEC_MUTEDIRECTION_NONE
} DSPCODEC_MUTEDIRECTION;

// structure of buffer communicated between RISC0 and RISC1
typedef struct 
{
    int32                   nOwner;             // prevent non-consistent access. 0: RISC0, 1: RISC1, 2: Skip(For A.P.O.)
    BOOL                    bTalkSpurtMarker;   // the marker of beginning of a talk spurt
    uint32                  nSeqNo;             // the sequence number of this frame
    uint32                  nTimeStamp;         // the timestamp of this frame
    int32                   nDelayNo;           // the number of frame to dalay
    int32                   nSize;              // the length of data/frame
    int32                   nTd;                // the time delayed
    char                    pBuffer[BUFFER_SIZE_DSP_DEC_R1];   // data buffer
}CFrameDecBuffer;

typedef struct 
{
    int32                   nOwner;             // prevent non-consistent access. 0: RISC0, 1: RISC1, 2: Skip(For A.P.O.)
    BOOL                    bTalkSpurtMarker;   // the marker of beginning of a talk spurt
    uint32                  nSeqNo;             // the sequence number of this frame
    uint32                  nTimeStamp;         // the timestamp of this frame
    int32                   nDelayNo;           // the number of frame to dalay
    int32                   nSize;              // the length of data/frame
    int32                   nTd;                // the time delayed
    char                    pBuffer[BUFFER_SIZE_DSP_ENC_R1];   // data buffer
}CFrameEncBuffer;

// structure of configuration
typedef struct 
{
    char*                   pToneTable;         // pointer to the tone table
    char*                   pCtrlParm;          // pointer to the parameter structure
    BOOL                    bVAD;               // true for on, false for off. NOTE: always false for G711u,a
    int32                   nVadLevel;          // VAD active threshold value. NOTE: always 0 for G711u,a
    BOOL                    bAES;               // Acoustic Echo Suppressor, true for on, false for off
    int32                   nHangoverTime;      // hangover time in ms
    int32                   nBGNoiseLevel;      // background noise level in dB
    int32                   nAttenRange;        // attentuated range in dB
    int32                   nTXGain;            // TX gain in dB
    int32                   nRXGain;            // RX gain in dB
    int32                   nTRRatio;           // TR ratio in dB
    int32                   nRTRatio;           // RT ratio in dB
    BOOL					bPLC;				// packet loss concealment	
    uint32					nJitterDelay;		// jitter delay configuration in unit of 10ms
    uint32					nMaxDelay;			// max delay configuration in unit of 10ms
    uint32					nJitterFactor;		// optimization factor of jitter delay 
}CDspcodecConfig;

// initialization parameter passed to RISC1
typedef struct 
{
    DSPCODEC_ALGORITHM       uCodingAlgorithm;    // which coding algorism used: G711u, G711a, G723.1a53, G723.1a63, G729
    CDspcodecConfig         xConfig;
}CDspcodecInitializeParm;

// config parameter passed to RISC1
typedef struct 
{
    CDspcodecConfig         xConfig;
}CDspcodecSetConfigParm;

// start parameter passed to RISC1
typedef struct 
{
    DSPCODEC_ACTION         uCodingAction;      // normal, encode, or decode
}CDspcodecStartParm;

// start mixing parameter passed to RISC1
typedef struct 
{
    DSPCODEC_ALGORITHM       uCh1CodingAlgorithm; // ch1 coding algorithm
    DSPCODEC_ALGORITHM       uCh2CodingAlgorithm; // ch2 coding algorithm
}CDspcodecStartMixingParm;
 
// mute parameter passed to RISC1
typedef struct 
{
    DSPCODEC_MUTEDIRECTION  uMuteDirection;     // which direction to mute
}CDspcodecMuteParm;

// tone parameter passed to RISC1
typedef struct 
{
    uint32           		nTone;              // which tone to play
    BOOL                    bFlag;              // start playing or stop playing
    DSPCODEC_TONEDIRECTION	uToneDirection;		// cz 920417: which direction to play tone
}CDspcodecPlayToneParm;

typedef struct
{
    uint32                  njSN;               // the sequence number of starting sync
    int32                   nTdOffset;          // the number of sample to be deducted from Td
} CDspcodecJitterSyncParm;

// THE parameter passed to RISC1 while interrupt
typedef struct 
{
    DSPCODEC_COMMAND        uCommand;
    int32                   nOwner;             // prevent non-consistent access. 0: RISC0, 1: RISC1
    uint32                  nSeqNo;             // the last frame that has been received
    uint32                  nFramtPut;          // the number of frames put in jitter buffer    
    union
    {
        CDspcodecInitializeParm  xInitializeParm;
        CDspcodecSetConfigParm   xSetConfigParm;
        CDspcodecStartParm       xStartParm;
        CDspcodecStartMixingParm xStartMixingParm;
        CDspcodecMuteParm        xMuteParm;
        CDspcodecPlayToneParm    xPlayToneParm;
        CDspcodecJitterSyncParm  xJitterSyncParm;
    } xParm;
    CFrameDecBuffer         xDecBuffer[BUFFER_NUM_DEC];    
}CDspcodecParm;

// THE response got from RISC1
	
typedef struct 
{
	DSPCODEC_ACK            uAck;
	int32                   nOwner;             // prevent non-consistent access. 0: RISC0, 1: RISC1
	uint32                  nSeqNo;             // the last frame that has been played
	uint32                  nRunTick;           // the running tick kept by RISC1
	BOOL                    bPutNew;            // flag to indicate it is allowable to put new or not
//	char                    pPrintBuff[BUFFER_SIZE_DSP_R1];
	CFrameEncBuffer         xEncOutput[BUFFER_NUM_ENC]; 
}CDspcodecResponse;

// Sandro move here
extern CDspcodecResponse RECV_ADDR[];
extern CDspcodecParm SEND_ADDR[];

#endif

