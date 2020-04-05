#include <sys/time.h>
#include "voip_manager.h"

int send_2833_main(int argc, char *argv[])
{
	if (argc == 5)
	{
		rtk_SetRTPRFC2833(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
	}
	else if (argc == 3)
	{
		rtk_SetRFC2833SendByAP(atoi(argv[1]), atoi(argv[2]));
	}
	else
	{
		printf("****** Usage ******\n");
		printf("'send_2833 chid 0' to disable send RFC2833 by AP.\n");
		printf("'send_2833 chid 1' to enable send RFC2833 by AP.\n");
		printf("'send_2833 chid sid dtmf_event duration' to send RFC2833 event\n");
		printf("Note: this test program is workable until you set to 2833 mode, and during VoIP call\n");
	}


	return 0;
}


