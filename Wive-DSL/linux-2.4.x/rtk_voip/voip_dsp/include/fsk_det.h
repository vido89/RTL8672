#ifndef FSK_DET_H_
#define FSK_DET_H_



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


typedef struct
{
	char number[25]; //caller id - number
	char date[9]; //caller id - date
	char cid_valid;
}
TstVoipciddet;

extern TstVoipciddet stVoipciddet[];

void init_cid_det_si3500(unsigned char chid);
void cid_det_si3050(unsigned short* page_addr, unsigned char chid);



#endif /* FSK_DET_H_ */
