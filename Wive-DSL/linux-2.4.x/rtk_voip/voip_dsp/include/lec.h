#if !defined(_LEC_H_)
#define _LEC_H_

#include "../../include/rtl_types.h"

/* LEC Mode Mask */
#define LEC             0x01	/* LEC */
#define LEC_NLP     	0x02	/* LEC+NLP */
#define LEC_NLP_CNG     0x04	/* LEC+NLP+CNG */

#ifdef CONFIG_RTK_VOIP_MODULE
#define DMEN_LEC	0
#define ASM_LEC		1
#define DMEN_STACK_LEC  0
#else
#define DMEN_LEC	0
#define ASM_LEC		1
#define DMEN_STACK_LEC  0
#endif

/* lec.c function calls prototype */

void LEC_re_init(chid);

void LEC_g168_init(unsigned char chid, unsigned char type);

void LEC_g168(char chid, Word16 *pRin, Word16 *pSin, Word16 *pEx);

#endif

