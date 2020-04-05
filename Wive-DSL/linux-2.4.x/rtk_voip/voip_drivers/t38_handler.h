#ifndef __T38_HANDLER_H__
#define __T38_HANDLER_H__

typedef enum {
	T38_STOP = 0,
	T38_START,
} t38_running_state_t;

extern t38_running_state_t t38RunningState[MAX_SLIC_CH_NUM];

extern int32 PCM_handler_T38( unsigned int chid );

/* T.38 codec API */
extern void T38_API_Initialize( uint32 chid );
extern uint32 T38_API_PutPacket( uint32 chid, const unsigned char *pPacketInBuffer, uint32 nPacketInSize );
extern uint32 T38_API_EncodeDecodeProcessAndDoSend( uint32 chid,
									const unsigned short *pPcmInBuffer,
									unsigned short *pPcmOutBuffer );

#endif /* __T38_HANDLER_H__ */

