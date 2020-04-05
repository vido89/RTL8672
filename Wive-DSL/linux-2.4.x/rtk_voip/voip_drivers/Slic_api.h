#ifndef _SLIC_API_H_
#define _SLIC_API_H_

#include <linux/config.h>
#define PHONE_ON_HOOK		0
#define PHONE_OFF_HOOK		1
#define PHONE_FLASH_HOOK	2
#define PHONE_STILL_ON_HOOK	3
#define PHONE_STILL_OFF_HOOK	4
#define PHONE_UNKNOWN		5

typedef enum
{
	COUNTRY_USA,
	COUNTRY_UK,
	COUNTRY_AUSTRALIA,
	COUNTRY_HK,
	COUNTRY_JP,
	COUNTRY_SE,
	COUNTRY_GR,
	COUNTRY_FR,
	COUNTRY_TR,
	COUNTRY_BE,
	COUNTRY_FL,
	COUNTRY_IT,
	COUNTRY_CN,
	COUNTRY_CUSTOME
}COUNTRY;




extern unsigned char slic_order;

typedef struct {
	unsigned char CH;		// CH = 0 ~ 3
	unsigned char ring_set;		// Ring_ON: ring_set = 1 ,  Ring_OFF: ring_set = 0 

} ring_struct;

typedef struct {
	unsigned char CH;		// CH:0 - 3
	unsigned char change;		// 1: Change. 0: No-Change
	unsigned char hook_status;	// 1: Off-Hook, 0: On-Hook
} hook_struck;

/*Slic_api.c function prototype*/
void SLIC_reset(int CH, int codec_law, unsigned char slic_number);
void CID_for_FSK_HW(unsigned int chid, char mode, unsigned char msg_type, char *str, char *str2, char *cid_name);
void CID_for_FSK_SW(unsigned int chid, char mode, unsigned char msg_type, char *str, char *str2, char *cid_name);
void FXS_Ring(ring_struct *ring);
int FXS_Check_Ring(ring_struct *ring);
void Hook_state(hook_struck *hook);
void Set_SLIC_Tx_Gain(unsigned char chid, unsigned char tx_gain);
void Set_SLIC_Rx_Gain(unsigned char chid, unsigned char rx_gain);
void SLIC_Choice(unsigned char number);
void SLIC_Set_Ring_Cadence_ON(unsigned char Slic_order, unsigned short Msec);
void SLIC_Set_Ring_Cadence_OFF(unsigned char Slic_order, unsigned short Msec);
void Init_Event_Polling_Use_Timer(void);
void SLIC_Hook_Polling(hook_struck *hook, unsigned int flash_hook_min_duration, unsigned int flash_hook_duration);
void OnHookLineReversal(int chid, unsigned char bReversal);
void SendNTTCAR(int chid);
void disableOscillators(void);
void SetOnHookTransmissionAndBackupRegister(int chid);
void RestoreBackupRegisterWhenSetOnHookTransmission(void);
void SLIC_Set_Impendance(unsigned char chid, unsigned short country, unsigned short impd);
#define SLIC_PCM_OFF            0
#define SLIC_PCM_ON             1
void SLIC_Set_PCM_state(int chid, int enable);

/*Slic_api.c variable extern*/
extern char fsk_cid_state[];
extern volatile char fsk_alert_state[];
extern char fsk_alert_time[];

extern char fsk_spec_areas[];

#endif	/*end _SLIC_API_H_ */
