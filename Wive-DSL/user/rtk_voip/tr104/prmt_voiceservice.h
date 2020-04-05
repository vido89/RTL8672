#ifndef __PRMT_VOICE_SERVICE_H__
#define __PRMT_VOICE_SERVICE_H__

#include "prmt_igd.h"

//extern struct sCWMP_ENTITY tVoiceService[];

#ifdef OLD
int objVoiceService(char *name, struct sCWMP_ENTITY *entity, int type, void *data);
#else 
int objVoiceService(char *name, struct CWMP_LEAF *e, int type, void *data);
#endif


#endif /* __PRMT_VOICE_SERVICE_H__ */

