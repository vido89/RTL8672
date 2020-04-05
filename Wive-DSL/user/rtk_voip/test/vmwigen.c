#include <sys/time.h>
#include "voip_manager.h"

int vmwigen_main(int argc, char *argv[])
{
	if (argc == 3)
	{
		char vmwi_on = 1, vmwi_off = 0;
		int i;	
		
		rtk_enable_pcm(atoi(argv[1]), 1);
		
		if (atoi(argv[2]) == 1) //on
		{
			
			for(i=0; i<6000000; i++);
				
			/* Set VMWI message on in on_hook mode */
			rtk_Gen_FSK_VMWI( atoi(argv[1])/*chid*/, &vmwi_on, 0);
			usleep(2000000);
		}
		else if (atoi(argv[2]) == 0) //off
		{
			/* Set VMWI message off in on_hook mode */
			rtk_Gen_FSK_VMWI( atoi(argv[1])/*chid*/, &vmwi_off, 0);
			usleep(2000000);
		}
		
		rtk_enable_pcm(atoi(argv[1]), 0);		
				
	}
	else
		printf("use error! Method: vmwigen chid on(1)/off(0)\n");

	return 0;
}

