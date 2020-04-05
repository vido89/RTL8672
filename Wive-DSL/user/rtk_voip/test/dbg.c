#include "voip_manager.h"

int dbg_main(int argc, char *argv[])
{
	if (argc == 2)
	{
		int dbg;

		dbg = atoi(argv[1]);
		rtk_debug(dbg);
		printf("set dbg flag = %d\n", dbg);
	}
	else
	{
		printf("use: %s dbg_flag\n", argv[0]);
	}

	return 0;
}

