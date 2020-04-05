#ifndef	_TONE_DET_H_
#define _TONE_DET_H_


#define TONE_TIME_POWER_THS 800

#define TONE_FREQ_TIME_RATIO_THS 4	/* tone freq-power/time-power ratio threshold, 3 means 2^3=8 */
#define TONE2_FREQ_TIME_RATIO_THS 3	/* tone freq-power/time-power ratio threshold, 2 means 2^2=4 */

#define FREQ_NA	       -1
#define FREQ_400HZ	0
#define FREQ_425HZ	1
#define FREQ_440HZ	2
#define FREQ_450HZ	3
#define FREQ_480HZ	4
#define FREQ_620HZ	5

#define BELLCORE_FREQ_1200 	0x16791824		// 0.17557 1200hz 2*cos(2*pi*1200/8000)-1
#define ETSI_FREQ_1300 		0x05C27748		// 0.04500 1300hz 2*cos(2*pi*1300/8000)-1



typedef struct

{
	unsigned int t_power; 		//power (time domain)
	
	unsigned int tone_power; 	//power (freq domain)

}
TstVoiptonedet;



typedef struct

{
	unsigned int t_power; 		//power (time domain)
	
	unsigned int tone1_power; 	//power (freq domain of tone 1)
	unsigned int tone2_power; 	//power (freq domain of tone 2)

}
TstVoiptone2det;


/* tone_det.c Function Prototype */
int tone_detection(unsigned char *adr);
int fsk_freq_det(unsigned char *adr, int daa_chid);

/* tone_det.c Variables */
extern int det_freq[];


#endif


