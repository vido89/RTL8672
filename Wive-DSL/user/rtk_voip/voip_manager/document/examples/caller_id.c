#include "voip_manager.h"

void ShowUsage(char *cmd)
{
	printf("usage: %s <mode> <caller_id> [FSK area] [DTMF mode]\n" \
		"  - mode => 0 is DTMF, 1 is FSK\n" \
		"  - caller_id => caller id string\n" \
		"  - FSK area[2:0] => 0 BELLCORE, 1: ETSI, 2: BT, 3: NTT\n" \
		"  - FSK area[bit7]=> FSK date & time sync\n" \
		"  - FSK area[bit6]=> reverse polarity before caller id (For FSK)\n" \
		"  - FSK area[bit5]=> short ring before caller id (For FSK)\n" \
		"  - FSK area[bit4]=> dual alert tone before caller id (For FSK)\n" \
		"  - FSK area[bit3]=> caller id Prior Ring (FSK & DTMF)\n" \
		"  - DTMF mode[1:0]=> Start digit, 0:A, 1:B, 2:C, 3:D\n" \
		"  - DTMF mode[3:2]=> End digit, 0:A, 1:B, 2:C, 3:D\n" , cmd);
	exit(0);
}

int main(int argc, char *argv[])
{
	unsigned int mode, fsk_area, dtmf_mode;

	if (argc < 3)
	{
		ShowUsage(argv[0]);
	}

	mode = atoi(argv[1]);
	switch (mode)
	{
	case 0:
		if (argc == 5)
		{
			fsk_area = atoi(argv[3]);
			dtmf_mode = atoi(argv[4]);
			rtk_Set_CID_DTMF_MODE(0, fsk_area, dtmf_mode); //for DTMF caller id prior ring or not. & set the caller id start/end digit
		}
		else if(argc == 4)
		{
			fsk_area = atoi(argv[3]);
			dtmf_mode = 0;	// start/end digit : A.
			rtk_Set_CID_DTMF_MODE(0, fsk_area, dtmf_mode); //for DTMF caller id prior ring or not.
		}
		rtk_Gen_Dtmf_CID(0, argv[2]);
		break;
	case 1:
		if (argc == 4)
		{
			fsk_area = atoi(argv[3]);
			if ((fsk_area&7) < CID_DTMF)
			{
				/*if ((fsk_area&7) == CID_FSK_NTT)
				**{
				**	printf("not support NTT area currently\n");
				**	return 0;
				**}
				*/
				rtk_Set_FSK_Area(0, fsk_area);
			}
			else
				ShowUsage(argv[0]);
		}
		rtk_Gen_FSK_CID(0, argv[2], (void *) 0, (void *) 0, 0); // FSK TypeI
		break;
	default:
		ShowUsage(argv[0]);
	}

	rtk_SetRingFXS(0, 1);
	sleep(1);
	rtk_SetRingFXS(0, 0);

	return 0;
}
