#include "voip_manager.h"

int fskgen_main(int argc, char *argv[])
{
	if (argc == 5)
	{
		if (atoi(argv[2]) == 1)//type I
		{
			rtk_Set_CID_FSK_GEN_MODE(atoi(argv[1]), 1); // soft gen
			rtk_Set_FSK_Area(atoi(argv[1])/*chid*/, atoi(argv[3])/*area*/);   /* area -> 0:Bellcore 1:ETSI 2:BT 3:NTT */
			rtk_Gen_FSK_CID(atoi(argv[1]), argv[4]/*cid*/, (void *) 0, "abc@xyz.com", 0);
		}
		else if (atoi(argv[2]) == 2)//type II
		{
			rtk_Set_CID_FSK_GEN_MODE(atoi(argv[1]), 1); // soft gen
			//printf("pcm enable\n");
			//rtk_eanblePCM(atoi(argv[1]), 1);
			rtk_Set_FSK_Area(atoi(argv[1])/*chid*/, atoi(argv[3])/*area*/);   /* area -> 0:Bellcore 1:ETSI 2:BT 3:NTT */
			rtk_Gen_FSK_CID(atoi(argv[1])/*chid*/, argv[4]/*cid*/, "09011311", "abc@xyz.com", 1);
			//rtk_eanblePCM(atoi(argv[1]), 0);
		}
		else
			printf("wrong fsk type: should be type-I(1) or type-II(2)\n");
	}
	else
		printf("use error! Method: fskgen chid fsk_type fsk_area caller_id \n");

	return 0;
}

/*
		   - fsk_area[2:0] => 0 BELLCORE, 1: ETSI, 2: BT, 3: NTT\n" \
		"  - fsk_area[bit7]=> FSK date & time sync\n" \
		"  - fsk_area[bit6]=> reverse polarity before caller id (For FSK)\n" \
		"  - fsk_area[bit5]=> short ring before caller id (For FSK)\n" \
		"  - fsk_area[bit4]=> dual alert tone before caller id (For FSK)\n" \
		"  - fsk_area[bit3]=> caller id Prior Ring (FSK & DTMF)\n"
*/
