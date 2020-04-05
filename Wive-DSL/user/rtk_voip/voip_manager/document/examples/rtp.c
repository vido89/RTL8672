#include "voip_manager.h"

int main(int argc, char *argv[])
{
	SIGNSTATE val;
	rtp_config_t rtp_config;
	payloadtype_config_t codec_config;
	int ActiveSession;
	int bStartRTP;
	int dtmf_val[SIGN_HASH + 1] = {0,1,2,3,4,5,6,7,8,9,0,10,11};
	int chid = 0;

	if (argc != 4)
	{
		printf("usage: %s chid src_ip dest_ip\n", argv[0]);
		return 0;
	}

	// init channel
	chid = atoi(argv[1]);
	// init dsp
	rtk_InitDSP(chid);
	rtk_SetDTMFMODE(chid, DTMF_RFC2833);	// set dtmf mode
	// init flag
	bStartRTP = 0;
	ActiveSession = 0;
	while (1)
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
		case SIGN_STAR:
		case SIGN_HASH:
			if (bStartRTP && ActiveSession == 0)
				rtk_SetRTPRFC2833(chid, 0, dtmf_val[val]);
			break;
		case SIGN_OFFHOOK:
			// call rtk_Offhook_Action at first
			rtk_Offhook_Action(chid);
			ActiveSession = 0;
			rtk_SetTranSessionID(chid, ActiveSession);

			// set rtp session
			memset(&rtp_config, 0, sizeof(rtp_config));
			rtp_config.chid = chid;		// use channel 
			rtp_config.sid = ActiveSession;
			rtp_config.isTcp = 0;		// use udp
			rtp_config.extIp = inet_addr(argv[2]);
			rtp_config.remIp = inet_addr(argv[3]);
			rtp_config.extPort = htons(9000 + chid);
			rtp_config.remPort = htons(9000 + (chid ? 0 : 1));
			rtp_config.tos = 0;
			rtp_config.rfc2833_payload_type_local = 96;
			rtp_config.rfc2833_payload_type_remote = 96;
			rtk_SetRtpConfig(&rtp_config);

			// set rtp payload, and other session parameters.
			codec_config.chid = chid;
			codec_config.sid = ActiveSession;
			codec_config.local_pt = 18;		// G729 use static pt 18
			codec_config.remote_pt = 18;	// G729 use static pt 18
			codec_config.uPktFormat = rtpPayloadG729;
			codec_config.nG723Type = 0;
			codec_config.nFramePerPacket = 1;
			codec_config.bVAD = 0;
			codec_config.nJitterDelay = 4;
			codec_config.nMaxDelay = 13;
			codec_config.nJitterFactor = 7;
			rtk_SetRtpPayloadType(&codec_config);

			// start rtp (channel number = 0, session number = 0)
			rtk_SetRtpSessionState(chid, ActiveSession, rtp_session_sendrecv);
			bStartRTP = 1;
			printf("%s:%d -> %s:%d\n",
				argv[2], 9000 + chid, argv[3], 9000 + (chid ? 0 : 1));
			break;

		case SIGN_ONHOOK:
			// close rtp
			rtk_SetRtpSessionState(chid, 0, rtp_session_inactive);
			rtk_SetRtpSessionState(chid, 1, rtp_session_inactive);
			bStartRTP = 0;
			// call rtk_Offhook_Action at last
			rtk_Onhook_Action(chid);
			break;

		case SIGN_FLASHHOOK:
			ActiveSession = !ActiveSession;
			rtk_SetTranSessionID(chid, ActiveSession);
			if (ActiveSession == 1)
			{
				// hold session
				rtk_Hold_Rtp(chid, 0, 1);
				// play dial tone
				rtk_SetPlayTone(chid, 1, DSPCODEC_TONE_DIAL, 1, DSPCODEC_TONEDIRECTION_LOCAL);
			}
			else
			{
				// resume session
				rtk_Hold_Rtp(chid, 0, 0);
				// stop dial tone
				rtk_SetPlayTone(chid, 1, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
			}
			break;

		default:
			break;
		}

		usleep(100000); // 100ms
	}

	return 0;
}
