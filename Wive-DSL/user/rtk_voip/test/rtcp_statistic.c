#include "voip_manager.h"

int rtcp_main(int argc, char *argv[])
{
	TstVoipRtpStatistics stVoipRtpStatistics;
	
	if (argc == 3)
	{
		rtk_Get_Rtp_Statistics(atoi(argv[1]), atoi(argv[2]), &stVoipRtpStatistics);

		printf("Rx(byte) = %d\n", stVoipRtpStatistics.nRxRtpStatsCountByte);
		printf("Rx(pkt) = %d\n", stVoipRtpStatistics.nRxRtpStatsCountPacket);
		printf("Rx(lost pkt) = %d\n", stVoipRtpStatistics.nRxRtpStatsLostPacket);
		printf("Tx(byte) = %d\n", stVoipRtpStatistics.nTxRtpStatsCountByte);
		printf("Tx(pkt) = %d\n", stVoipRtpStatistics.nTxRtpStatsCountPacket);
	}
	else
		printf("use error! Method: rtcp_statistic  chid  reset(0 or 1) \n");
	
	return 0;
}


