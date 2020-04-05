#include "voip_manager.h"

int slic_reset_main(int argc, char *argv[])
{
	if (argc == 2)
		printf("I am alive!\n");	

	//rtk_reset_slic(atoi(argv[1]));
	rtk_reset_slic();
	
	return 0;
}

