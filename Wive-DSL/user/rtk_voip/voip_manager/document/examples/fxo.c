#include "voip_manager.h"

// test option:
// 	- FXO_DIAL_FXS0 has defined
//		* fxo ring will dial fxs0 automatically
//	- FXO_DIAL_FXS0 has not defined
//		* fxo ring will prompt user dial voip number via IVR
#define FXO_DIAL_FXS0_AUTO

#define FUNCKEY_FXO_0	".0"
#define FUNCKEY_FXS_0 	".1"
#define FUNCKEY_FXS_1 	".2"

#define CHANNEL_IS_FXS(chid)	((chid) < SLIC_CH_NUM)
#define CHANNEL_IS_FXO(chid)	((chid) >= SLIC_CH_NUM)

#define NULL	((void *) 0)

enum {
	FXO_NO_DAA = 0,
	FXO_VIRTUAL_DAA,
	FXO_DAA
};

enum {
	STATE_IDLE = 0,
	STATE_RING,
	STATE_CONNECT,
	STATE_DAA_RING,			// virtual daa state
	STATE_DAA_CONNECT		// virtual daa state
};

enum {
	DIAL_NONE = 0,
	DIAL_FXO_0,
	DIAL_FXS_0,
	DIAL_FXS_1,
	DIAL_OTHER,
};

typedef struct channel_s channel_t;

struct channel_s {
	// phone info
	int chid;
	int state;
	char dial_code[101];
	int dial_index;
	// rtp info
	unsigned long ip;
	unsigned short port;
	// remote channel (just peer simulation)
	channel_t *remote;
};

void channel_init(channel_t *channel, int chid);
void channel_reset(channel_t *channel, int chid);
int channel_input(channel_t *channel, SIGNSTATE val);
void channel_dial_local(channel_t *channel, channel_t *remote_channel);
void channel_dial_out(channel_t *channel);
void channel_switch_to_fxo(channel_t *channel);	// virtual daa
void channel_rtp_start(channel_t *channel);
void channel_rtp_stop(channel_t *channel);

// global variable
int fxo_type;
channel_t channels[VOIP_CH_NUM];

void channel_init(channel_t *channel, int chid)
{
	memset(channel, 0, sizeof(*channel));
	// phone
	channel->chid = chid;
	channel->state = STATE_IDLE;
	channel->dial_code[0] = 0;
	channel->dial_index = 0;
	// rtp
	channel->ip = ntohl(inet_addr("127.0.0.1"));
	channel->port = 9000 + chid;
	// reomte 
	channel->remote = NULL;
}

int channel_input(channel_t *channel, SIGNSTATE val)
{
	if (channel->dial_index >= sizeof(channel->dial_code) - 1) 
	{
		printf("%s: out of buffer\n", __FUNCTION__);
		return DIAL_NONE;
	}

	if (val >= SIGN_KEY1 && val <= SIGN_KEY9) 
		channel->dial_code[channel->dial_index++] = (char) '0' + val;
	else if (val == SIGN_KEY0)
		channel->dial_code[channel->dial_index++] = '0';
	else if (val == SIGN_STAR)
		channel->dial_code[channel->dial_index++] = '.';
	else if (val == SIGN_HASH)
		return DIAL_OTHER;

	channel->dial_code[channel->dial_index] = 0;

	// check local dial
	if (strcmp(channel->dial_code, FUNCKEY_FXO_0) == 0)
		return DIAL_FXO_0;
	else if (strcmp(channel->dial_code, FUNCKEY_FXS_0) == 0)
		return DIAL_FXS_0;
	else if (strcmp(channel->dial_code, FUNCKEY_FXS_1) == 0)
		return DIAL_FXS_1;

	return DIAL_NONE;
}

void channel_dial_local(channel_t *channel, channel_t *remote_channel)
{
	if (remote_channel->chid == channel->chid)
	{
		printf("%s: couldn't dial itself\n", __FUNCTION__);
		rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
		return;
	}

	// 1. create local rtp
	channel->remote = remote_channel;
	channel_rtp_start(channel);
	channel->state = STATE_CONNECT;

	// 2. signal remote channel
	remote_channel->remote = channel;

	if (CHANNEL_IS_FXS(remote_channel->chid))
	{
		rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_RINGING, 1, DSPCODEC_TONEDIRECTION_LOCAL);
		// signal fxs
		rtk_SetRingFXS(remote_channel->chid, 1);
	}
	else
	{	
		// signal fxo
		rtk_FXO_RingOn(remote_channel->chid);
	}

	remote_channel->state = STATE_RING;
}

void channel_dial_out(channel_t *channel)
{
	printf("%s: dial %s done.\n", __FUNCTION__, channel->dial_code);
	rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
}

void channel_switch_to_fxo(channel_t *channel)
{
	if (rtk_DAA_off_hook(channel->chid) == 0)
	{
		printf("%s: ok.\n", __FUNCTION__);
	}
	else	// pstn out failed
	{
		printf("%s: failed.\n", __FUNCTION__);
		rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
	}
}

void channel_rtp_start(channel_t *channel)
{
	rtp_config_t rtp_config;
	payloadtype_config_t codec_config;

	// set rtp session
	rtp_config.chid = channel->chid;
	rtp_config.sid = 0;
	rtp_config.isTcp = 0;		// use udp
	rtp_config.extIp = htonl(channel->ip);
	rtp_config.remIp = htonl(channel->remote->ip);
	rtp_config.extPort = htons(channel->port);
	rtp_config.remPort = htons(channel->remote->port);
	rtp_config.tos = 0;
	rtp_config.rfc2833_payload_type_local = 0;		// use in-band
	rtp_config.rfc2833_payload_type_remote = 0;		// use in-band
	rtk_SetRtpConfig(&rtp_config);

	// set rtp payload, and other session parameters.
	codec_config.chid = channel->chid;
	codec_config.sid = 0;
	codec_config.local_pt = 0;
	codec_config.remote_pt = 0;
	codec_config.uPktFormat = rtpPayloadPCMU;
	codec_config.nG723Type = 0;
	codec_config.nFramePerPacket = 1;
	codec_config.bVAD = 0;
	codec_config.nJitterDelay = 4;
	codec_config.nMaxDelay = 13;
	codec_config.nJitterFactor = 7;
	rtk_SetRtpPayloadType(&codec_config);

	// start rtp 
	rtk_SetRtpSessionState(channel->chid, 0, rtp_session_sendrecv);
}

void channel_rtp_stop(channel_t *channel)
{
	rtk_SetRtpSessionState(channel->chid, 0, rtp_session_inactive);
}

void channel_handle_input(channel_t *channel, SIGNSTATE val)
{
	int ret;

	rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
	ret = channel_input(channel, val);
	switch (ret)
	{
	case DIAL_FXO_0:
		if (fxo_type == FXO_NO_DAA)
		{
			printf("%s: no fxo\n", __FUNCTION__);
			rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
			return;
		}
		if (fxo_type == FXO_VIRTUAL_DAA)
		{
			channel_switch_to_fxo(channel);							// switch to fxo
		}
		else
		{
			channel_dial_local(channel, &channels[SLIC_CH_NUM]);	// dial fxo 0
		}
		break;
	case DIAL_FXS_0:
		channel_dial_local(channel, &channels[0]);	// dial fxs 0
		break;
	case DIAL_FXS_1:
		if (SLIC_CH_NUM < 2)
		{
			printf("%s: no fxs 1\n", __FUNCTION__);
			rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
			return;
		}
		channel_dial_local(channel, &channels[1]);	// dial fxs 1
		break;
	case DIAL_OTHER:
		channel_dial_out(channel);
		break;
	default:
		// nothing
		break;
	}
}

void channel_handle_onhook(channel_t *channel)
{
	switch (channel->state)
	{
	case STATE_CONNECT:
		channel_rtp_stop(channel);
		if (CHANNEL_IS_FXS(channel->chid))
		{
			// fxs
		}
		else
		{
			// daa
			rtk_FXO_onhook(channel->chid);
		}

		// check remote channel is terminated 
		if (channel->remote->remote == NULL) 
			break;

		// signal remote channel
		if (CHANNEL_IS_FXS(channel->remote->chid))
		{
			rtk_SetPlayTone(channel->remote->chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
		}
		else
		{
			rtk_FXO_Busy(channel->remote->chid);
		}
		break;
	case STATE_DAA_CONNECT:
		// virtual daa
		rtk_DAA_on_hook(channel->chid);
		break;
	default:
		break;
	}

	rtk_IvrStopPlaying(channel->chid); // clear ivr
	rtk_SetTranSessionID(channel->chid, 255);	// clear resource count
	rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);	// clear tone
	channel_init(channel, channel->chid);
}

void channel_handle_offhook(channel_t *channel)
{
	// active session 0, and record resource + 1
	rtk_SetTranSessionID(channel->chid, 0);

	switch (channel->state)
	{
	case STATE_IDLE:
		if (CHANNEL_IS_FXS(channel->chid))
		{
			// fxs
			rtk_SetPlayTone(channel->chid, 0, DSPCODEC_TONE_DIAL, 1, DSPCODEC_TONEDIRECTION_LOCAL);
		}
		else
		{
			char caller_id[21];
			char caller_date[9];

		#ifdef FXO_DIAL_FXS0_AUTO
			rtk_Get_DAA_CallerID(channel->chid, caller_id, caller_date);
			if (caller_id[0])
				printf("caller id is %s\n", caller_id);
			else
				printf("no caller id detect\n");

			channel_dial_local(channel, &channels[0]);	// dial fxs 0
		#else
			char text[] = {IVR_TEXT_ID_PLZ_ENTER_NUMBER, '\0'};

			usleep(500000); // after phone off-hook, wait for a second, and then play IVR
			rtk_FXO_offhook(channel->chid);
			rtk_IvrStartPlaying(channel->chid, text);
		#endif
		}
		break;
	case STATE_RING:
		if (CHANNEL_IS_FXS(channel->chid))
		{
			// fxs
			rtk_SetRingFXS(channel->chid, 0);
			rtk_SetPlayTone(channel->remote->chid, 0, DSPCODEC_TONE_RINGING, 0, DSPCODEC_TONEDIRECTION_LOCAL);
			channel_rtp_start(channel);
			channel->state = STATE_CONNECT;
		}
		else
		{
			// daa
			rtk_FXO_offhook(channel->chid);
			channel_rtp_start(channel);
			channel->state = STATE_CONNECT;
		}
				
		// ack remote channel
		if (CHANNEL_IS_FXS(channel->remote->chid))
		{
			// fxs
		}
		else
		{
			// daa
			rtk_FXO_offhook(channel->remote->chid);
		}
		break;
	case STATE_DAA_RING:
		// virtual daa
		rtk_DAA_off_hook(channel->chid);
		channel->state = STATE_DAA_CONNECT;
		break;
	default:
		break;
	}
}

int main()
{
	int i;
	SIGNSTATE val;
	channel_t *channel;

	rtk_Get_VoIP_Feature();

	if ((g_VoIP_Feature & DAA_TYPE_MASK) == NO_DAA)
		fxo_type = FXO_NO_DAA;
	else if ((g_VoIP_Feature & DAA_TYPE_MASK) == VIRTUAL_DAA)
		fxo_type = FXO_VIRTUAL_DAA;
	else
		fxo_type = FXO_DAA;

	// init 
	for (i=0; i<VOIP_CH_NUM; i++)
	{
		// init dsp
		rtk_InitDSP(i);

		// init phone
		rtk_Set_Voice_Gain(i, 0, 0);	// 0db
		rtk_Set_echoTail(i, 4);			// 4ms

		// init fxs & fxo
		if (CHANNEL_IS_FXS(i))
		{
			// fxs
			rtk_Set_Flash_Hook_Time(i, 0, 300);			// 0~300 ms
			rtk_Set_SLIC_Ring_Cadence(i, 1500, 1500);	// 1500 ms
			rtk_Set_CID_DTMF_MODE(i, 
				CID_DTMF | 0x08, 						// DTMF mode, Prior Ring
				CID_DTMF_A | CID_DTMF_C << 2			// start digit = A, end digit = C
			);
		}
		else
		{
			// fxo
			rtk_Set_CID_Det_Mode(i,
				2,										// auto detect mode (NOT NTT)
				CID_DTMF
			);
		}

		// set dtmf mode
		rtk_SetDTMFMODE(i, DTMF_INBAND);

		// flush kernel fifo before app run
		rtk_Set_flush_fifo(i);

		// init local data
		channel_init(&channels[i], i);
	}

	// main loop
main_loop:

	for (i=0; i<VOIP_CH_NUM; i++)
	{
		channel = &channels[i];

		if (CHANNEL_IS_FXS(i))
			rtk_GetFxsEvent(i, &val);
		else
			rtk_GetFxoEvent(i, &val);

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
			if (channel->state == STATE_IDLE)
			{
				channel_handle_input(channel, val);
			}
			break;
		case SIGN_ONHOOK:
			channel_handle_onhook(channel);
			rtk_Onhook_Action(i);	// rtk_Onhook_Action have to be the last step in SIGN_ONHOOK
			break;
		case SIGN_OFFHOOK:
			rtk_Offhook_Action(i);	// rtk_Offhook_Action have to be the first step in SIGN_OFFHOOK
			channel_handle_offhook(channel);
			break;
    	case SIGN_RING_ON:		// virtual daa ring on
			if (channel->state == STATE_IDLE)
			{
				channel->state = STATE_DAA_RING;
				rtk_SetRingFXS(channel->chid, 1);
			}
			break;
		case SIGN_RING_OFF: 	// virtual daa ring off
			if (channel->state == STATE_DAA_RING)
			{
				rtk_SetRingFXS(channel->chid, 0);
				channel->state = STATE_IDLE;
			}
			break;
		case SIGN_FLASHHOOK:
			break;
		case SIGN_OFFHOOK_2: // off-hook but no event
			break;
		case SIGN_NONE:
			break;
		default:
			printf("unknown(%d)\n", val);
			break;
		}

		usleep(100000 / VOIP_CH_NUM); // 100ms
	}

	goto main_loop;

	return 0;
}
