#ifndef	_BUSYTONE_DET_H_
#define _BUSYTONE_DET_H_

#define BUSY_ON_CNT	25
#define BUSY_OFF_CNT	15
#define BUSY_ON_TIMES	2
#define BUSY_OFF_TIMES	2

/* busytone_det.c Function Prototype */
void busy_tone_det(unsigned char *adr);
void busy_tone_flag_set(int cfg);
int  busy_tone_flag_get(void);
void busy_tone_det_init(void);

/* busytone_det.c Variables */
extern int busy_tone_flag;
extern int busy_cnt;
extern int busy_on;
extern int busy_off;
extern int wait_busy_off;

#endif


/*

Country		Dial Tone (Hz)		Busy Tone (Hz)
		
USA		350+440			480+620
Hong Kong	350+440			480+620
UK		350+440			400
Australia	425+25			400
Japan		400			400
Italy		425			425
Finland		425			425
Sweden		425			425
Germany		425			425
TR57		425			425
France		440			440
Belgium		440			440
China		450			450

*/