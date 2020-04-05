#ifndef _KEYPAD_MAP_H_
#define _KEYPAD_MAP_H_

#define KEY_0			0x17
#define KEY_1		    0x18
#define KEY_2		    0x49
#define KEY_3		    0x36
#define KEY_4		    0x01
#define KEY_5		    0x15
#define KEY_6		    0x37
#define KEY_7		    0x13
#define KEY_8		    0x39
#define KEY_9		    0x38
#define KEY_STAR	    0x14
#define KEY_POUND	    0x16

//#define KEY_MENU	        0x33	// ??
//#define KEY_TXT_NUM		0x23	// ??
//#define KEY_INVOL_PLUS		0x21	// ??
//#define KEY_INVOL_MINUS		0x51	// ??
//#define KEY_NET			0x56	// ??
//#define KEY_MESSAGE		0x02	// ??
//#define KEY_M1			0x41	// ??
//#define KEY_M2			0x01	// ??
//#define KEY_M3			0x11	// ??
//#define KEY_M4			0x15	// ??
//#define KEY_M5			0x05	// ??
//#define KEY_PhoneBook		0x45	// ??

#define KEY_OUTVOL_PLUS		0x45
#define KEY_OUTVOL_MINUS	0x46
#define KEY_SPEAKER		0x78

#define KEY_OK			0x59
#define KEY_CANCEL		0x25

#define KEY_UP			0x02
#define KEY_DOWN		0x12
#define KEY_LEFT		0x23
#define KEY_RIGHT		0x35

#define KEY_CONFERENCE	0x48
#define KEY_PICK		0x08
#define KEY_TRANSFER	0x89
#define KEY_REDIAL		0x06
#define KEY_HOLD		0x69

#define KEY_LINE1		0x04
#define KEY_LINE2		0x57
#define KEY_F1			0x56
#define KEY_F2			0x34
#define KEY_F3			0x05
#define KEY_FORWARD		0x27
#define KEY_DND			0x24
#define KEY_MISSED		0x47
#define KEY_VMS			0x79
#define KEY_BLIND_XFER	0x67
#define KEY_MUTE		0x19
#define KEY_HEADSET		0x58

#define KEY_HOOK		0x0F	/* TODO: modify */

#if 0
struct keypad_dev_s
{
	volatile unsigned char flags;	//0: own by driver, 1: own by AP
	unsigned char input_count;	//AP should clear to 0
	unsigned char data_string[50];
	
};
#endif

struct keypad_dev_s
{
	volatile unsigned char flags;	//0: own by driver, 1: own by AP
	unsigned char data_string;	
};
	

typedef struct keypad_dev_s keypad_dev_t;

extern keypad_dev_t keypad_data_pool;






#endif	//_KEYPAD_MAP_H_
