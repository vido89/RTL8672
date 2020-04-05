#include "voip_manager.h"

int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		printf("usage: %s DTMF_MODE(0:RFC2833 / 1:SIP_INFO / 2:INBAND) \n", argv[0]);
		return 0;
	}
	if( atoi(argv[1])!=0 && atoi(argv[1])!=1 && atoi(argv[1])!=2)
	{
		printf("DTMF_MODE: RFC2833(0) SIP_INFO(1) INBAND(2)\n", argv[0]);
		return 0;
	}
	
	SIGNSTATE val;
	rtp_config_t rtp_config;
	payloadtype_config_t codec_config;
	
	int dtmf_mode = atoi(argv[1]);
	int dtmf_val[SIGN_HASH + 1] = {0,1,2,3,4,5,6,7,8,9,0,10,11};
	int chid;
	
	for(chid = 0 ; chid <SLIC_CH_NUM ;chid ++)
	{
		rtk_InitDSP(chid);
		rtk_Set_flush_fifo(chid);	
		rtk_SetDTMFMODE(chid, dtmf_mode);	// set dtmf mode
	}
	
	while (1)
	{	for(chid = 0 ; chid <SLIC_CH_NUM ; chid ++)
		{
			rtk_GetFxsEvent(chid, &val);
			switch (val)
			{
			case SIGN_KEY1:
			case SIGN_KEY2:
			case SIGN_KEY3:
			case SIGN_KEY4:
			case SIGN_KEY5:
			case SIGN_KEY6:
			case SIGN_KEY7:
			case SIGN_KEY8:
			case SIGN_KEY9:
			case SIGN_KEY0:
				break;
			case SIGN_STAR:
				rtk_IvrStartPlaying  (chid, IVR_DIR_LOCAL, "STAR");
				break;
			case SIGN_HASH:
				rtk_IvrStartPlaying  (chid, IVR_DIR_REMOTE, "HASH");
				break;
			case SIGN_OFFHOOK:
				// call rtk_Offhook_Action at first
				rtk_Offhook_Action(chid);
				rtk_SetTranSessionID(chid, 0);
	
				// set rtp session
				memset(&rtp_config, 0, sizeof(rtp_config));
				rtp_config.chid = chid;		// use channel 
				rtp_config.sid = 0;
				rtp_config.isTcp = 0;		// use udp
				rtp_config.extIp = inet_addr("127.0.0.1");
				rtp_config.remIp = inet_addr("127.0.0.1");
				rtp_config.extPort = htons(9000 + chid);
				rtp_config.remPort = htons(9000 + (chid ? 0 : 1));
				rtp_config.tos = 0;
				if(dtmf_mode == DTMF_RFC2833)
				{
					rtp_config.rfc2833_payload_type_local = 96;
					rtp_config.rfc2833_payload_type_remote = 96;
				}
				rtk_SetRtpConfig(&rtp_config);
	
				// set rtp payload, and other session parameters.
				codec_config.chid = chid;
				codec_config.sid = 0;
				codec_config.local_pt = 18;	// G729 use static pt 18
				codec_config.remote_pt = 18;	// G729 use static pt 18
				codec_config.uPktFormat = rtpPayloadG729;
				codec_config.nG723Type = 0;
				codec_config.nFramePerPacket = 1;
				codec_config.bVAD = 0;
				codec_config.nG726Packing = 2;
				codec_config.nJitterDelay = 4;
				codec_config.nMaxDelay = 13;
				codec_config.nJitterFactor = 7;
				rtk_SetRtpPayloadType(&codec_config);
				// start rtp (channel number = 0, session number = 0)
				rtk_SetRtpSessionState(chid, 0, rtp_session_sendrecv);
				break;
	
			case SIGN_ONHOOK:
				// close rtp
				rtk_SetRtpSessionState(chid, 0, rtp_session_inactive);
				// call rtk_Offhook_Action at last
				rtk_Onhook_Action(chid);
				break;
	
			default:
				break;
			}
		}
		usleep(100000); // 100ms
	}
	return 0;
}
