#ifndef	_TONE_DET_I_H_
#define _TONE_DET_I_H_


#define FREQ_NA	       -1
#define FREQ_400HZ	0
#define FREQ_425HZ	1
#define FREQ_440HZ	2
#define FREQ_450HZ	3
#define FREQ_480HZ	4
#define FREQ_620HZ	5



/* tone_det.c Variables */
extern int det_freq[];


/* busytone_det.c Function Prototype */
void busy_tone_det(unsigned char *adr);
void busy_tone_flag_set(int cfg);
int  busy_tone_flag_get(void);
void busy_tone_det_init(void);


#endif

