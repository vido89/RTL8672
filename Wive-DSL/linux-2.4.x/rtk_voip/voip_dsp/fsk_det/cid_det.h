#ifndef CID_DET_H_
#define CID_DET_H_

#define DAA_RING_DET (1<<2)
#define DAA_OFF_HOOK_DET (1<<0)
#define DAA_RING (1<<2)
#define DAA_OFF_HOOK (1<<0)

#define RDTN (1<<6)
#define RDTP (1<<5)


//in fsk.c

#define ETSI_LF_RATE 26
//2pi=160sample, low frequency=1300hz  1300/8000*160=26
#define ETSI_HF_RATE 42
//2pi=160sample, hi frequency=2100hz  2100/8000*160=42
#define BELL202_LF_RATE 24
//2pi=160sample, low frequency=1200hz  1200/8000*160=24
#define BELL202_HF_RATE 44
//2pi=160sample, hi frequency=2200hz  2200/8000*160=44



//in cid_det.c
extern long auto_cid_det[];
extern long cid_type[];

enum _AUTO_CID_DET_
{
    AUTO_CID_DET_OFF = 0,
    AUTO_CID_DET_ON_NTT,
    AUTO_CID_DET_ON_NO_NTT,
    AUTO_CID_DET_MAX
};

enum _CID_TYPE_
{
    CID_TYPE_FSK_BELLCORE = 0,
    CID_TYPE_FSK_ETSI,
    CID_TYPE_FSK_BT,
    CID_TYPE_FSK_NTT,
    CID_TYPE_DTMF,
    CID_TYPE_MAX
};


//in cid_fsk_tab.c
extern short sin_coef16_160[];


typedef struct
{
	char number[25]; //caller id - number
	char date[9]; //caller id - date
	char cid_valid;
}
TstVoipciddet;


__attribute__((aligned(8))) typedef struct
{
	uint32 hifreq;			// the space freq (0)
	uint32 lofreq;			// the mark freq (1)
	uint32 hifreq_table_addr;	//the sine table address of the space freq
	uint32 lofreq_table_addr;	//the sine table address of the mark freq
	uint32 fsk_win_start_addr;	//fsk_window_data_start_addr
	uint32 fsk_win_in_addr;		//input the address of fsk window data when previous exit
	uint32 fsk_sum_win_addr;	//input the address of (sum per window data)
}
TstVoipCidFskInput;

typedef struct
{
	uint8 bit_buf[128];		// the rude (0/1) bit data
	uint8 byte_buf[260];		// the decoded byte date (according the start/stop bit in rude (0/1) bit data)
	uint16 bit_buf_length;		// the length of the rude (0/1) bit data
	uint16 byte_buf_length;		// the length of the decoded btye daa
}
TstVoipCidFskDecode;

extern char fsk_initial_flag2[];
extern char fsk_decode_complete_flag[];

extern unsigned char fsk_callerid_data[][150];
extern TstVoipCidFskDecode stVoipCidFskDecode[];
extern TstVoipciddet stVoipciddet[];

void fsk_decode(unsigned short* page_addr, unsigned char chid);
void init_cid_det_si3500(unsigned char chid);
void cid_det_si3050(unsigned short* page_addr, unsigned char chid);

#endif /* CID_DET_H_ */

