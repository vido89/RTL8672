#ifndef _CODEC_MEM_H_
#define _CODEC_MEM_H_

#include "dmem_stack.h"

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
#define IMEM_SIZE       0x01000
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
#define IMEM_SIZE       0x02000
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8671
#define IMEM_SIZE       0x01000
#endif


extern unsigned long __imem_start;
extern unsigned long __dmem_start;

/* g729 dmem */
extern unsigned long __g729_dmem_start;

/* g729 imem */
extern unsigned long __load_start_IMEM_G729ENC;
extern unsigned long __load_stop_IMEM_G729ENC;
extern unsigned long __load_start_IMEM_G729DEC;
extern unsigned long __load_stop_IMEM_G729DEC;
extern unsigned long __load_start_IMEM_G729;
extern unsigned long __load_stop_IMEM_G729;
extern unsigned long __IMEM_G729_START;

/* g723.1 dmem */
extern const unsigned long __g7231_dmem_start;

/* g723.1 imem */
extern unsigned long __load_start_IMEM_G7231ENC;
extern unsigned long __load_stop_IMEM_G7231ENC;
extern unsigned long __load_start_IMEM_G7231DEC;
extern unsigned long __load_stop_IMEM_G7231DEC;
extern unsigned long __load_start_IMEM_G7231;
extern unsigned long __load_stop_IMEM_G7231;
extern unsigned long __IMEM_G7231_START;

/* g726 dmem */
extern unsigned long __g726_dmem_start;

/* gsm-fr dmem */
extern const unsigned long __gsmfr_dmem_start;

/* lec dmem */
extern unsigned long __lec_dmem_start;

extern int set_codec_mem(int type, int state, int g726_rate);
extern void set_DMEM(int start);
extern void set_and_fill_IMEM(int start, int end);

/* g.729 stack size */
#define G729_DUMMY_SSIZE 1560
#define G729_DMEM_SSIZE 488//488
#define G729ENC_SSIZE 392 /* 456*4 bytes */
#define G729DEC_SSIZE 392 /* 224*4 bytes */

extern unsigned long g729_dmem_stack[G729_DMEM_SSIZE];
extern unsigned long g729_orig_sp;
extern unsigned long g729_dmem_sp;

/* g.7231 stack size */
#define G7231_DUMMY_SSIZE 1728
#define G7231_DMEM_SSIZE 320//360
#define G7231ENC_SSIZE 256//320 /* 320*4 bytes */
#define G7231DEC_SSIZE 256//320 /* 320*4 bytes */

extern unsigned long g7231_dmem_stack[G7231_DMEM_SSIZE];
extern unsigned long g7231_orig_sp;
extern unsigned long g7231_dmem_sp;

/* g.726 stack size */
#define G726_DUMMY_SSIZE 1328
#define G726_DMEM_SSIZE 720//360
#define G726ENC_SSIZE 700//320 /* 320*4 bytes */
#define G726DEC_SSIZE 700//320 /* 320*4 bytes */

extern unsigned long g726_dmem_stack[G726_DMEM_SSIZE];
extern unsigned long g726_orig_sp;
extern unsigned long g726_dmem_sp;

/* gsmfr stack size */
#define GSMFR_DUMMY_SSIZE 1328
#define GSMFR_DMEM_SSIZE 720//360
#define GSMFRENC_SSIZE 700//320 /* 320*4 bytes */
#define GSMFRDEC_SSIZE 700//320 /* 320*4 bytes */

extern unsigned long gsmfr_dmem_stack[GSMFR_DMEM_SSIZE];
extern unsigned long gsmfr_orig_sp;
extern unsigned long gsmfr_dmem_sp;

/* lec stack size */
#define LEC_DUMMY_SSIZE 1948//1984
#define LEC_DMEM_SSIZE 100//64
#define LEC_SSIZE 920

extern unsigned long lec_dmem_stack[LEC_DMEM_SSIZE];
extern unsigned long lec_orig_sp;
extern unsigned long lec_dmem_sp;

#endif /* _CODEC_MEM_H_ */
