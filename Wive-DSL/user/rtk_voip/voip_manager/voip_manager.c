#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
// fsk date & time sync
#include <time.h>
#include <sys/time.h>
// fsk date & time sync
#include "voip_manager.h"

#define SUPPORT_RTCP
		     /* define SUPPORT_RTCP to support RTCP.
                      * It also need to define it in rtk_voip.h for kernel space.
		      * Thlin add 2006-07-04
		      */

#define SETSOCKOPT(optid, varptr, vartype, qty) \
        { \
                int     sockfd; \
                if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) == -1) { \
                        return -1; \
                } \
                if (setsockopt(sockfd, IPPROTO_IP, optid, (void *)varptr, sizeof(vartype)*qty) != 0) { \
                        close(sockfd); \
                        return -2; \
                } \
                close(sockfd); \
        }

static int32 rtk_OpenRtpSession(uint32 chid, uint32 sid);
static int no_resource_flag[VOIP_CH_NUM]={0};

// voip feature from kernel config 
int g_VoIP_Feature = 0;

/* If DAA is used by CHn, set g_DAA_used[CHn]=1, otherwise, set g_DAA_used[CHn]=0 */
int g_DAA_used[MAX_SLIC_NUM] = {0};

int32 rtk_InitDSP(int ch)
{
	// init rtp session
	rtk_OpenRtpSession(ch, 0);
	rtk_OpenRtpSession(ch, 1);

	// reset rtp and codec
	rtk_SetRtpSessionState(ch, 0, rtp_session_inactive);
	rtk_SetRtpSessionState(ch, 1, rtp_session_inactive);

	// reset resource
	rtk_SetTranSessionID(ch, 255);

	// Always enable FXO pcm for Caller ID detection.
	if (ch >= RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
		rtk_enable_pcm(ch, 1);

    return 0;
}

int32 rtk_SetRtpConfig(rtp_config_t *cfg)
{
    TstVoipMgrSession stVoipMgrSession;

    stVoipMgrSession.ch_id = cfg->chid;
    stVoipMgrSession.m_id = cfg->sid;
    stVoipMgrSession.ip_src_addr = cfg->remIp;
    stVoipMgrSession.udp_src_port = cfg->remPort;
    stVoipMgrSession.ip_dst_addr = cfg->extIp;
    stVoipMgrSession.udp_dst_port = cfg->extPort;
    stVoipMgrSession.tos = cfg->tos;
	stVoipMgrSession.rfc2833_payload_type_local = cfg->rfc2833_payload_type_local;
	stVoipMgrSession.rfc2833_payload_type_remote = cfg->rfc2833_payload_type_remote;
    if (cfg->isTcp)
    {
        printf("==> SUPPORT UDP ONLY\n");
        return -1;
    }
    else
    {
        stVoipMgrSession.protocol = 0x11;
    }

    SETSOCKOPT(VOIP_MGR_SET_SESSION, &stVoipMgrSession, TstVoipMgrSession, 1);

    return 0;
}

#if 0
/*
 * @ingroup VOIP_RTP_SESSION
 * @brief Get RTP configuration
 * @param chid The channel number.
 * @param sid The session number.
 * @param cfg The RTP configuration.
 * @retval 0 Success
 * @note Not implement.
 */
int32 rtk_GetRtpConfig(uint32 chid, uint32 sid, rtp_config_t *cfg)
{
    return -1;
}
#endif

int32 rtk_SetRtcpConfig(rtp_config_t *cfg, unsigned short rtcp_tx_interval)	//thlin+ for Rtcp
{
#ifdef SUPPORT_RTCP

    TstVoipRtcpSession stVoipRtcpSession;

    stVoipRtcpSession.ch_id = cfg->chid;
    stVoipRtcpSession.m_id = cfg->sid;
    stVoipRtcpSession.ip_src_addr = cfg->remIp;
    stVoipRtcpSession.rtcp_src_port = cfg->remPort+1;	// thlin: rtcp port = rtp port +1
    stVoipRtcpSession.ip_dst_addr = cfg->extIp;
    stVoipRtcpSession.rtcp_dst_port = cfg->extPort+1;	// thlin: rtcp port = rtp port +1
    if (cfg->isTcp)
    {
        printf("==> SUPPORT UDP ONLY\n");
        return -1;
    }
    else
    {
        stVoipRtcpSession.protocol = 0x11;
    }

    SETSOCKOPT(VOIP_MGR_SET_RTCP_SESSION, &stVoipRtcpSession, TstVoipRtcpSession, 1);
    
    TstVoipValue stVoipValue;
    stVoipValue.ch_id = cfg->chid;
    stVoipValue.value5 = rtcp_tx_interval; /*unit: ms*/
    /* If rtcp_tx_interval is equal to 0, then RTCP Tx is disable.*/
    SETSOCKOPT(VOIP_MGR_SET_RTCP_TX_INTERVAL, &stVoipValue, TstVoipValue, 1);

#endif
    return 0;
}

int32 rtk_SetRtpPayloadType(payloadtype_config_t *cfg)
{
    TstVoipPayLoadTypeConfig stVoipPayLoadTypeConfig;

    stVoipPayLoadTypeConfig.ch_id = cfg->chid;
    stVoipPayLoadTypeConfig.m_id = cfg->sid;
    stVoipPayLoadTypeConfig.local_pt = cfg->local_pt;
    stVoipPayLoadTypeConfig.remote_pt = cfg->remote_pt;
    stVoipPayLoadTypeConfig.uPktFormat = cfg->uPktFormat;
    stVoipPayLoadTypeConfig.nG723Type = cfg->nG723Type;
    stVoipPayLoadTypeConfig.nFramePerPacket = cfg->nFramePerPacket;
    stVoipPayLoadTypeConfig.bVAD = cfg->bVAD;
    stVoipPayLoadTypeConfig.nJitterDelay = cfg->nJitterDelay;
    stVoipPayLoadTypeConfig.nMaxDelay = cfg->nMaxDelay;
    stVoipPayLoadTypeConfig.nJitterFactor = cfg->nJitterFactor;

    SETSOCKOPT(VOIP_MGR_SETRTPPAYLOADTYPE, &stVoipPayLoadTypeConfig, TstVoipPayLoadTypeConfig, 1);

    return 0;
}

#if 0
/*
 * @ingroup VOIP_RTP_SESSION
 * @brief Get RTP payload type
 * @param chid The channel number.
 * @param sid The session number.
 * @param cfg The RTP payload configuration.
 * @retval 0 Success
 * @note Not implement.
 */  
int32 rtk_GetRtpPayloadType(uint32 chid, uint32 sid, payloadtype_config_t *cfg)
{
    return -1;
}
#endif

int32 rtk_SetRtpSessionState(uint32 chid, uint32 sid, RtpSessionState state)
{
	TstVoipRtpSessionState stVoipRtpSessionState;
	stVoipRtpSessionState.ch_id = chid;
	stVoipRtpSessionState.m_id = sid;
	stVoipRtpSessionState.state = state;
	SETSOCKOPT(VOIP_MGR_SETRTPSESSIONSTATE, &stVoipRtpSessionState, TstVoipRtpSessionState, 1);
	
    if ( rtp_session_inactive == state )   //disable both rx and tx
    {
		TstVoipValue stVoipValue;
		TstVoipCfg stVoipCfg;

   		 stVoipCfg.ch_id = chid;
    		stVoipCfg.m_id = sid;
		stVoipCfg.enable = 0;
		// close RTP
		SETSOCKOPT(VOIP_MGR_UNSET_SESSION, &stVoipCfg, TstVoipCfg, 1);
#ifdef SUPPORT_RTCP
		// close RTCP
		SETSOCKOPT(VOIP_MGR_UNSET_RTCP_SESSION, &stVoipCfg, TstVoipCfg, 1); //thlin+ for Rtcp
#endif
		// close Codec
		stVoipValue.ch_id = chid;
		stVoipValue.m_id = sid;
		SETSOCKOPT(VOIP_MGR_DSPCODECSTOP, &stVoipValue, TstVoipValue, 1);
    }

    return 0;
}

#if 0
/*
 * @ingroup VOIP_RTP_SESSION
 * @brief Get RTP Session State
 * @param chid The channel number.
 * @param sid The session number.
 * @param pstate The RTP Session State.
 * @retval 0 Success
 * @note Not implement.
 */  
int32 rtk_GetRtpSessionState(uint32 chid, uint32 sid, RtpSessionState *pstate)
{
    return -1;
}
#endif

int g_bDisableRingFXS = 0;

int32 rtk_DisableRingFXS(int bDisable)
{
	g_bDisableRingFXS = bDisable;
	return 0;
}

int32 rtk_SetRingFXS(uint32 chid, uint32 bRinging)
{
    TstVoipSlicRing stVoipSlicRing;

	if (g_bDisableRingFXS)
	{
		// quiet mode
		return 0;
	}

    stVoipSlicRing.ch_id = chid;
    stVoipSlicRing.ring_set = bRinging;
    SETSOCKOPT(VOIP_MGR_SLIC_RING, &stVoipSlicRing, TstVoipSlicRing, 1);

    return 0;
}

#if 0
/*
 * @ingroup VOIP_PHONE_RING
 * @brief Check which channel is ringing
 * @param chid The channel number.
 * @param pRinging The ringing state.
 * @retval 0 Success
 * @note Not implement.
 */  
int32 rtk_GetRingFXS(uint32 chid, uint32 *pRinging)
{
    return -1;
}
#endif

#ifdef SUPPORT_IVR_HOLD_TONE

#define G729_HOLD_TONE		/* borrow g723 code to play */

#ifdef G729_HOLD_TONE
#define IVRCODEC		"729"
#define IVRFRAMESIZE	10
#define IVRFILLTWICE
#else
#define IVRCODEC		"723"
#define IVRFRAMESIZE	24
#undef  IVRFILLTWICE
#endif

FILE *g_fpG723[ 2 ];
int gStartG723[ 2 ];

void InitG723IvrFile( void )
{
	int i;
	
	for( i = 0; i < 2; i ++ ) {
	
		if( g_fpG723[ i ] )
			fclose( g_fpG723[ i ] );

		/* place G.723 sample file in /bin/723_raw */
		if( ( g_fpG723[ i ] = fopen( "/bin/" IVRCODEC "_raw", "rb" ) ) == NULL )
			printf( "Open /bin/" IVRCODEC "_raw fail(%d)\n", i );
	}
}

void StartG723IvrFile( int chid )
{
	gStartG723[ chid ] = 1;
}

void StopG723IvrFile( int chid )
{
	gStartG723[ chid ] = 0;
	rtk_IvrStopPlaying( chid );
}

void RefillG723IvrFile( int chid )
{
	/* Try to refill G723 IVR buffer periodically. */
	unsigned char buff723[ IVRFRAMESIZE * 10 ];
	unsigned int count723;
	unsigned int copied;
	FILE * const fp723 = g_fpG723[ chid ];
#ifdef IVRFILLTWICE
	unsigned int time = 2;
#endif

	if( !gStartG723[ chid ] )
		return;
				
#ifdef IVRFILLTWICE
	while( time -- )
#endif
	{
		count723 = fread( buff723, IVRFRAMESIZE, 10, fp723 );
		
		if( count723 ) {
	  #ifdef G729_HOLD_TONE
	  		copied = rtk_IvrStartPlayG729( chid, count723, buff723 );
	  #else
			copied = rtk_IvrStartPlayG72363( chid, count723, buff723 );
	  #endif
			
			if( count723 != copied )
				fseek( fp723, -IVRFRAMESIZE * ( int )( count723 - copied ), SEEK_CUR );
		}
	
		if( feof( fp723 ) )
			fseek( fp723, 0, SEEK_SET );
	}
}

#if 0
void main_program_pseudo_code( void )
{
	int chid;
	
	/* initialize once */
	InitG723IvrFile();
	
	while( 1 ) {
		/* receive message */
		/* Then, process this message or receive timeout. */
		
		for( chid = 0; chid < 2; chid ++ ) {
			
			/* refill IVR buffer in a certain period. */
			RefillG723IvrFile( chid );
			
			/* You may play hold tone somewhere, but it will play IVR. */
			{
				rtk_SetPlayTone( chid, sid, DSPCODEC_TONE_HOLD, 1, path );
			}
		}
	}
	
}
#endif
#endif /* SUPPORT_IVR_HOLD_TONE */
	
int32 rtk_SetPlayTone(uint32 chid, uint32 sid, DSPCODEC_TONE nTone, uint32 bFlag,
	DSPCODEC_TONEDIRECTION Path)
{
    TstVoipPlayToneConfig cfg;

#ifdef SUPPORT_IVR_HOLD_TONE    
    static int bPrevHoldTone[ 2 ] = { 0, 0 };
    
    /* hold tone use ivr to play */
    if( nTone == DSPCODEC_TONE_HOLD && bFlag ) {
    	StartG723IvrFile( chid );
    	bPrevHoldTone[ chid ] = 1;
    	printf( "StartG723IvrFile(%d)\n", chid );
    	return;	/* don't play tone */
    } else if( bPrevHoldTone[ chid ] ) {
    	StopG723IvrFile( chid );
    	bPrevHoldTone[ chid ] = 0;
    	printf( "StopG723IvrFile(%d)\n", chid );
    }
#endif /* SUPPORT_IVR_HOLD_TONE */

	/* RING is incoming ring tone, and RINGING is ring back tone. */
    //if (nTone == DSPCODEC_TONE_RING)
    //    nTone = DSPCODEC_TONE_RINGING;

    cfg.ch_id = chid;
    cfg.m_id = sid;
    cfg.nTone = nTone;
    cfg.bFlag = bFlag;
    cfg.path = Path;

    SETSOCKOPT(VOIP_MGR_SETPLAYTONE, &cfg, TstVoipPlayToneConfig, 1);

    return 0;
}

#if 0
/*
 * @ingroup VOIP_PHONE_TONE
 * @brief Check tone of session
 * @param chid The channel number.
 * @param sid The session number.
 * @param pTone The tone type.
 * @param pFlag The tone state.
 * @param pPath The tone direction.
 * @retval 0 Success
 * @note Not implement.
 */  
int32 rtk_GetPlayTone(uint32 chid, uint32 sid, DSPCODEC_TONE *pTone, uint32 *pFlag,
	DSPCODEC_TONEDIRECTION *pPath)
{
    return -1;
}
#endif

unsigned short rtk_VoIP_resource_check(TstVoipMgr3WayCfg *stVoipMgr3WayCfg)
{
    	TstVoipResourceCheck stVoipResourceCheck;

    if (stVoipMgr3WayCfg == NULL)
    {
    		stVoipResourceCheck._3way_session_info.enable = 255;
	}
	else
	{
		memcpy(&stVoipResourceCheck._3way_session_info, stVoipMgr3WayCfg, sizeof(TstVoipMgr3WayCfg));
	}
        SETSOCKOPT(VOIP_MGR_VOIP_RESOURCE_CHECK, &stVoipResourceCheck, TstVoipResourceCheck, 1);
        
	return stVoipResourceCheck.resource;
}

int32 rtk_GetFxsEvent(uint32 chid, SIGNSTATE *pval)
{

	TstVoipSlicHook stVoipSlicHook;
	TstVoipCfg stVoipCfg;
	TstVoipValue stVoipValue;

	*pval = SIGN_NONE;

#ifdef NO_SLIC
return 0;
#endif

	stVoipSlicHook.ch_id = chid;
	SETSOCKOPT(VOIP_MGR_SLIC_HOOK, &stVoipSlicHook, TstVoipSlicHook, 1);
	//printf("***** %d ******\n", stVoipSlicHook.hook_status);
        if (stVoipSlicHook.hook_status == 4) /* PHONE_STILL_OFF_HOOK */
        {
		// detect DTMF
		stVoipValue.ch_id = chid;
		stVoipValue.value = 'X';
		SETSOCKOPT(VOIP_MGR_DTMF_GET, &stVoipValue, TstVoipValue, 1);

		if (stVoipValue.value >= '1' && stVoipValue.value <= '9')
			*pval = SIGN_KEY1 + stVoipValue.value - '1';
		else if (stVoipValue.value == '0')
			*pval = SIGN_KEY0;
		else if (stVoipValue.value == '*')
			*pval = SIGN_STAR;
		else if (stVoipValue.value == '#')
		    *pval = SIGN_HASH;
		else
		    *pval = SIGN_OFFHOOK_2;

		return 0;
        }
	else if (stVoipSlicHook.hook_status == 1) /* PHONE_OFF_HOOK */
        {
		*pval = SIGN_OFFHOOK; // off-hook
#if 1
		/* when phone offhook, enable pcm and DTMF detection */
		stVoipCfg.ch_id = chid;
		stVoipCfg.enable = stVoipSlicHook.hook_status;

		SETSOCKOPT(VOIP_MGR_PCM_CFG, &stVoipCfg, TstVoipCfg, 1);
/*
		if (VOIP_RESOURCE_UNAVAILABLE == rtk_VoIP_resource_check(NULL))
		{
			no_resource_flag[chid] = 1;
			*pval = 0;
			usleep(500000);//after phone off-hook, wait for a second,and then play IVR.
#if 1
			char text[]={IVR_TEXT_ID_NO_RESOURCE, '\0'};
			printf("play ivr (%d)...\n", chid);
			rtk_IvrStartPlaying( chid, text );
#else
			rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
#endif
			return 0;
		}
*/
		SETSOCKOPT(VOIP_MGR_DTMF_CFG, &stVoipCfg, TstVoipCfg, 1);
		stVoipCfg.enable = 1;
		 /* when phone offhook, enable fax detection */
		SETSOCKOPT(VOIP_MGR_FAX_OFFHOOK, &stVoipCfg, TstVoipCfg, 1);
#endif
		return 0;
        }
        else if (stVoipSlicHook.hook_status == 0) /* PHONE_ON_HOOK */
        {
		*pval = SIGN_ONHOOK;	// on-hook
#if 1
            	/* when phone onhook, stop play tone, disable pcm and DTMF detection */
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_DIAL, 0, DSPCODEC_TONEDIRECTION_LOCAL);
		usleep(100000); // [Important] sleep >= 100ms. MUST add delay for ACMW to stop tone!
		
		stVoipCfg.ch_id = chid;
		stVoipCfg.enable = stVoipSlicHook.hook_status;
		SETSOCKOPT(VOIP_MGR_PCM_CFG, &stVoipCfg, TstVoipCfg, 1);

		if (no_resource_flag[chid] == 1)
		{
			no_resource_flag[chid] = 0;
			*pval = 0;
#if 1
			printf("stop play ivr(%d)...\n", chid);
			rtk_IvrStopPlaying(chid);
#else
			rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_BUSY, 0, DSPCODEC_TONEDIRECTION_LOCAL);
#endif
			return 0;
		}
		SETSOCKOPT(VOIP_MGR_DTMF_CFG, &stVoipCfg, TstVoipCfg, 1);

		/* when phone onhook, re-init CED detection */
		SETSOCKOPT(VOIP_MGR_ON_HOOK_RE_INIT, &stVoipCfg, TstVoipCfg, 1);
		
#endif
		return 0;
        }
        else if (stVoipSlicHook.hook_status == 2) /* PHONE_FLASH_HOOK */
        {
		*pval = SIGN_FLASHHOOK;
           	return 0;
        }
        else if (stVoipSlicHook.hook_status == 3) /* PHONE_STILL_ON_HOOK */
	{
		return 0;
	}
	else if (stVoipSlicHook.hook_status == 11) /* FXO_RING_ON */
	{
		*pval = SIGN_RING_ON;
		return 0;
	}
	else if (stVoipSlicHook.hook_status == 12)  /* FXO_RING_OFF */
	{
		*pval = SIGN_RING_OFF;
		return 0;
	}
	else
        {
		printf("==>UNKOWN SLIC STATUS (%d)\n", chid);   /* HOOK FIFO empty also belong to this case */
            	return -1;
        }
}



int32 rtk_GetFxoEvent(uint32 chid, SIGNSTATE *pval)
{

	TstVoipSlicHook stVoipSlicHook;
	TstVoipCfg stVoipCfg;
	TstVoipValue stVoipValue;

	*pval = SIGN_NONE;

	stVoipSlicHook.ch_id = chid;
	SETSOCKOPT(VOIP_MGR_SLIC_HOOK, &stVoipSlicHook, TstVoipSlicHook, 1);
	if (stVoipSlicHook.hook_status == 10) /* FXO_STILL_OFF_HOOK */
	{
		// detect DTMF
		stVoipValue.ch_id = chid;
		stVoipValue.value = 'X';
		SETSOCKOPT(VOIP_MGR_DTMF_GET, &stVoipValue, TstVoipValue, 1);

		if (stVoipValue.value >= '1' && stVoipValue.value <= '9')
		    *pval = SIGN_KEY1 + stVoipValue.value - '1';
		else if (stVoipValue.value == '0')
		    *pval = SIGN_KEY0;
		else if (stVoipValue.value == '*')
		    *pval = SIGN_STAR;
		else if (stVoipValue.value == '#')
		    *pval = SIGN_HASH;
		else
		    *pval = SIGN_OFFHOOK_2;
			return 0;
	}
	else if (stVoipSlicHook.hook_status == 11) /* FXO_RING_ON */
	{
		*pval = SIGN_OFFHOOK; // off-hook
		if (VOIP_RESOURCE_UNAVAILABLE == rtk_VoIP_resource_check(NULL))
		{
			no_resource_flag[chid] = 1;
			*pval = 0;
#ifdef DAA_IVR
			usleep(500000);//after phone off-hook, wait for a second,and then play IVR.
			char text[]={IVR_TEXT_ID_NO_RESOURCE, '\0'};
			printf("play ivr...\n");
			rtk_FXO_offhook(chid);
			rtk_IvrStartPlaying( chid, text );
			//rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
#else			
#endif
			return 0;
		}
		stVoipCfg.ch_id = chid;
		stVoipCfg.enable = 1;
		 /* when phone offhook, enable fax detection */
		SETSOCKOPT(VOIP_MGR_FAX_OFFHOOK, &stVoipCfg, TstVoipCfg, 1);
		return 0;
	}
	else if (
		stVoipSlicHook.hook_status == 12 ||  /* FXO_RING_OFF */
		stVoipSlicHook.hook_status == 13) /* FXO_BUSY_TONE */
	{
       	*pval = SIGN_ONHOOK;	// on-hook
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_DIAL, 0, DSPCODEC_TONEDIRECTION_LOCAL);
		usleep(100000); // [Important] sleep >= 100ms. MUST add delay for ACMW to stop tone!
		if (no_resource_flag[chid] == 1)
		{
			no_resource_flag[chid] = 0;
			*pval = 0;
#ifdef DAA_IVR
			printf("stop play ivr...\n");
			rtk_IvrStopPlaying(chid);
			rtk_FXO_onhook(chid);
#endif			
			//rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_BUSY, 0, DSPCODEC_TONEDIRECTION_LOCAL);
			return 0;
		}
		/* when phone onhook, re-init CED detection */
		stVoipCfg.ch_id = chid;
		SETSOCKOPT(VOIP_MGR_ON_HOOK_RE_INIT, &stVoipCfg, TstVoipCfg, 1);
		return 0;
	}
	else if (stVoipSlicHook.hook_status == 7) /* FXO_OFF_HOOK */
	{
		return 0;
	}
	else if (stVoipSlicHook.hook_status == 6) /* FXO_ON_HOOK */
	{
		return 0;
	}
	else if (stVoipSlicHook.hook_status == 8) /* FXO_FLASH_HOOK */
	{
		*pval = SIGN_FLASHHOOK;	// flash-hook
		return 0;
	}
	else if (stVoipSlicHook.hook_status == 9) /* FXO_STILL_ON_HOOK */
	{
		return 0;
	}
	else if (stVoipSlicHook.hook_status == 14) /* FXO_CALLER_ID */
	{
		return 0;
	}
	else
	{
		printf("==>UNKOWN FXO STATUS (%d) = %d\n", chid, stVoipSlicHook.hook_status);
		return -1;
	}
}

/*
 * @ingroup VOIP_RTP_SESSION
 * @brief Init the RTP session
 * @param chid The channel number.
 * @param sid The session number.
 * @retval 0 Success
 * @note internal use
 */
int32 rtk_OpenRtpSession(uint32 chid, uint32 sid)
{
    	TstVoipCfg stVoipCfg;

        stVoipCfg.ch_id = chid;
        stVoipCfg.m_id = sid;
        stVoipCfg.enable = 1;
        SETSOCKOPT(VOIP_MGR_CTRL_RTPSESSION, &stVoipCfg, TstVoipCfg, 1);
	return 0;
}

int32 rtk_SetTranSessionID(uint32 chid, uint32 sid)
{
    	TstVoipCfg stVoipCfg;

        stVoipCfg.ch_id = chid;
        stVoipCfg.m_id = sid;
        stVoipCfg.enable = 1;
        SETSOCKOPT(VOIP_MGR_CTRL_TRANSESSION_ID, &stVoipCfg, TstVoipCfg, 1);
	return 0;
}

int32 rtk_GetTranSessionID(uint32 chid, uint32* psid)
{
    	TstVoipCfg stVoipCfg;

        stVoipCfg.ch_id = chid;
//        stVoipCfg.m_id = sid;
        stVoipCfg.enable = 0;
        SETSOCKOPT(VOIP_MGR_CTRL_TRANSESSION_ID, &stVoipCfg, TstVoipCfg, 1);
	*psid = stVoipCfg.t_id;
	return 0;
}

int32 rtk_SetConference(TstVoipMgr3WayCfg *stVoipMgr3WayCfg)
{
	SETSOCKOPT(VOIP_MGR_SETCONFERENCE, stVoipMgr3WayCfg, TstVoipMgr3WayCfg, 1);
	return 0;
}

// 0:rfc2833  1: sip info  2: inband
int32 rtk_SetDTMFMODE(uint32 chid, uint32 mode)
{
	TstVoipCfg stVoipCfg;

	stVoipCfg.ch_id = chid;
	stVoipCfg.enable = mode;

	SETSOCKOPT(VOIP_MGR_SET_DTMF_MODE, &stVoipCfg, TstVoipCfg, 1);

	return 0;
}

#if 0
/*
 * @ingroup VOIP_RTP_DTMF
 * @brief Get the DTMF mode
 * @param chid The channel number.
 * @param sid The session number.
 * @param pmode The DTMF mode.
 * @retval 0 Success
 * @note Not implement.
 */  
int32 rtk_GetDTMFMODE(uint32 chid, uint32 sid, uint32 *pmode)
{
	return -1;
}
#endif

// thlin 2006-05-04
int32 rtk_Set_echoTail(uint32 chid, uint32 echo_tail)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.value = echo_tail;

	SETSOCKOPT(VOIP_MGR_SET_ECHO_TAIL_LENGTH, &stVoipValue, TstVoipValue, 1);

	return 0;
}

int32 rtk_Set_G168_LEC(uint32 chid, uint32 support_lec)	/* This function can turn on/off G168 LEC. */
{
        TstVoipCfg stVoipCfg;
        stVoipCfg.ch_id = chid;
        stVoipCfg.enable = support_lec;
        SETSOCKOPT(VOIP_MGR_SET_G168_LEC_CFG, &stVoipCfg, TstVoipCfg, 1);
	return 0;
}

#if 0
// thlin 2006-05-04
/**
 * @ingroup VOIP_PHONE_FXS
 * @brief Set the Tx Gain of FXS
 * @param chid The channel number.
 * @param gain The gain value 0..9 means -6~+3db. Default value is 6 (0db).
 * @retval 0 Success
 */
int32 rtk_Set_SLIC_Tx_Gain(uint32 chid, uint32 gain)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.value = gain;

	SETSOCKOPT(VOIP_MGR_SET_SLIC_TX_GAIN, &stVoipValue, TstVoipValue, 1);

	return 0;
}

// thlin 2006-05-04
/**
 * @ingroup VOIP_PHONE_FXS
 * @brief Set the Rx Gain of FXS
 * @param chid The channel number.
 * @param gain The gain value 0..9 means -6~+3db. Default value is 6 (0db).
 * @retval 0 Success
 */
int32 rtk_Set_SLIC_Rx_Gain(uint32 chid, uint32 gain)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.value = gain;

	SETSOCKOPT(VOIP_MGR_SET_SLIC_RX_GAIN, &stVoipValue, TstVoipValue, 1);

	return 0;
}
#endif

int32 rtk_Set_SLIC_Ring_Cadence(uint32 chid, uint16 cad_on_msec, uint16 cad_off_msec)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.value5 = cad_on_msec;
	stVoipValue.value6 = cad_off_msec;

	SETSOCKOPT(VOIP_MGR_SET_SLIC_RING_CADENCE, &stVoipValue, TstVoipValue, 1);

	return 0;
}

// thlin 2006-05-10
int32 rtk_Set_DAA_Tx_Gain(uint32 gain)
{

	if ((g_VoIP_Feature & DAA_TYPE_MASK) == REAL_DAA || (g_VoIP_Feature & DAA_TYPE_MASK) == REAL_DAA_NEGO)
	{
		TstVoipValue stVoipValue;

		stVoipValue.value = gain;

		SETSOCKOPT(VOIP_MGR_SET_DAA_TX_GAIN, &stVoipValue, TstVoipValue, 1);
	}
	return 0;

}

// thlin 2006-05-10
int32 rtk_Set_DAA_Rx_Gain(uint32 gain)
{

	if ((g_VoIP_Feature & DAA_TYPE_MASK) == REAL_DAA || (g_VoIP_Feature & DAA_TYPE_MASK) == REAL_DAA_NEGO)
	{
		TstVoipValue stVoipValue;

		stVoipValue.value = gain;

		SETSOCKOPT(VOIP_MGR_SET_DAA_RX_GAIN, &stVoipValue, TstVoipValue, 1);
	}
	return 0;

}

// thlin 2006-06-07
int32 rtk_Set_Flash_Hook_Time(uint32 chid, uint32 min_time, uint32 time)
{
	TstVoipHook stHookTime;

	stHookTime.ch_id = chid;
	stHookTime.flash_time_min = min_time;
	stHookTime.flash_time = time;

	SETSOCKOPT(VOIP_MGR_SET_FLASH_HOOK_TIME, &stHookTime, TstVoipHook, 1);

	return 0;
}


int32 rtk_SetRTPRFC2833(uint32 chid, uint32 sid, uint32 digit)
{
	TstVoip2833 stRFC2833;

	stRFC2833.ch_id = chid;
	stRFC2833.sid = sid;
	stRFC2833.digit = digit;
	stRFC2833.duration = 100; // refer to 865x
	SETSOCKOPT(VOIP_MGR_SEND_RFC2833_PKT_CFG, &stRFC2833, TstVoip2833, 1);

	return 0;
}

/*
 * @ingroup VOIP_PHONE_CALLERID
 * @brief Generate CallerID via DTMF
 * @param chid The channel number.
 * @param pval The Caller ID.
 * @retval 0 Success
 * @note internal use
 */
static int32 rtk_SetDtmfCIDState(uint32 chid, char *str_cid)
{
	TstVoipCID stCIDstr;

	stCIDstr.ch_id = chid;
	strcpy(stCIDstr.string, str_cid);
	SETSOCKOPT(VOIP_MGR_DTMF_CID_GEN_CFG, &stCIDstr, TstVoipCID, 1);

	return 0;
}

/*
 * @ingroup VOIP_PHONE_CALLERID
 * @brief Check the CallerID generation via DTMF is done
 * @param chid The channel number.
 * @param pstate 0 if done
 * @retval 0 Success
 * @note internal use
 */
static int32 rtk_GetDtmfCIDState(uint32 chid, uint32 *cid_state)
{
	TstVoipCID stCIDstr;

	stCIDstr.ch_id = chid;
	SETSOCKOPT(VOIP_MGR_GET_CID_STATE_CFG, &stCIDstr, TstVoipCID, 1);
	*cid_state = stCIDstr.cid_state;

	return 0;
}

int32 rtk_GetCEDToneDetect(uint32 chid, FAXSTATE *pval)
{
        TstVoipCfg stVoipCfg;
        stVoipCfg.ch_id = chid;
        SETSOCKOPT(VOIP_MGR_FAX_MODEM_PASS_CFG, &stVoipCfg, TstVoipCfg, 1);
        //*pval = stVoipCfg.enable ? FAX_DETECT : FAX_IDLE;
				*pval = stVoipCfg.enable;
	return 0;
}

int32 rtk_GetFaxEndDetect(uint32 chid, uint32 *pval)
{
        TstVoipCfg stVoipCfg;
        stVoipCfg.ch_id = chid;
        SETSOCKOPT(VOIP_MGR_FAX_END_DETECT, &stVoipCfg, TstVoipCfg, 1);
	*pval = stVoipCfg.enable;
	//t.38 fax end detect, 1:fax end.
	return 0;
}

int32 rtk_eanblePCM(uint32 chid, uint32 val)
{
	TstVoipCfg stVoipCfg;

	stVoipCfg.ch_id = chid;
	stVoipCfg.enable = val;
	SETSOCKOPT(VOIP_MGR_PCM_CFG, &stVoipCfg, TstVoipCfg, 1);
	return 0;
}

int32 rtk_Gen_Dtmf_CID(uint32 chid, char *str_cid)
{
	int i = 0;
	rtk_eanblePCM(chid, 1); // enable PCM before generating dtmf caller id
	// set cid_state
	if (str_cid[0] == 0)               // not null str
		rtk_SetDtmfCIDState(chid, "0123456789");   // replace "0123456789" with From CID
	else
		rtk_SetDtmfCIDState(chid, str_cid);

	// polling cid_state until be clear
	int32 tmp=-1;
	do
	{
		if(rtk_GetDtmfCIDState(chid, &tmp) != 0)
			break;
		usleep(50000);  // 50ms
		if ((i++) > 120 )// wait 6 sec, if not get cid state =0, then break.
			break;
	}
	while (tmp);

        //printf("Get DTMF CID state = 0\n");
	rtk_eanblePCM(chid, 0); // disable PCM after generating dtmf caller id
	return 0;
}

int32 rtk_Gen_FSK_CID(uint32 chid, char *str_cid, char *str_date, char *str_cid_name, char mode)
{
	time_t timer = time(0);		// fsk date & time sync
	TstVoipCID stCIDstr;

	if(!mode)			// on-hook
		rtk_eanblePCM(chid, 1); // enable PCM before generating fsk caller id

	stCIDstr.ch_id = chid;
	stCIDstr.cid_mode = mode;       //0:on-hook
	// set cid string
	if (str_cid[0] == 0)               // not null str
		strcpy(stCIDstr.string, "0123456789");   // replace "0123456789" with From CID
	else
		strcpy(stCIDstr.string, str_cid);
	
	if (str_date && str_date[0])
	{
		strcpy(stCIDstr.string2, str_date);
	}
	else
	{
	//strftime(stCIDstr.string2, 9, "%m%d%H%M", gmtime(&timer));
	strftime(stCIDstr.string2, 9, "%m%d%H%M", localtime(&timer));
	}
	if(str_cid_name)
		strcpy(stCIDstr.cid_name, str_cid_name);
	SETSOCKOPT(VOIP_MGR_FSK_CID_GEN_CFG, &stCIDstr, TstVoipCID, 1);

	int32 tmp=-1;
	do
	{
		if(rtk_GetFskCIDState(chid, &tmp) != 0)
			break;
		usleep(50000);  // 50ms
	}
	while (tmp);
	if(!mode)			//on-hook
		rtk_eanblePCM(chid, 0); // disable PCM after generating fsk caller id
	return 0;
}



int32 rtk_Gen_FSK_ALERT(uint32 chid, char *str_cid)
{
	TstVoipCID stCIDstr;

	stCIDstr.ch_id = chid;
	strcpy(stCIDstr.string, str_cid);
	SETSOCKOPT(VOIP_MGR_FSK_ALERT_GEN_CFG, &stCIDstr, TstVoipCID, 1);

	return 0;
}

int32 rtk_Gen_FSK_VMWI(uint32 chid, char *state, char mode)  /* state:	point to the address of value to set VMWI state. 0 : off; 1 : on*/
{
	TstVoipCID stCIDstr;

	stCIDstr.ch_id = chid;
	stCIDstr.cid_mode = mode;       //0:on-hook
	strcpy(stCIDstr.string, state);
	SETSOCKOPT(VOIP_MGR_SET_FSK_VMWI_STATE, &stCIDstr, TstVoipCID, 1);

	return 0;
}

int32 rtk_Set_FSK_Area(uint32 chid, char area)   /* area -> 0:Bellcore 1:ETSI 2:BT 3:NTT */
{
	TstVoipCID stCIDstr;

	stCIDstr.ch_id = chid;
	stCIDstr.cid_mode = area;
	stCIDstr.cid_gain = 2; /* Only support multiple 1~5 */

	SETSOCKOPT(VOIP_MGR_SET_FSK_AREA, &stCIDstr, TstVoipCID, 1);

	return 0;
}

#if 0
/*
 * unused
 */
int32 rtk_SetRunRing(char* number)
{
	return -1;
}
#endif

int32 rtk_Hold_Rtp(uint32 chid, uint32 sid, uint32 enable)
{
    TstVoipCfg stVoipCfg;
	
#if 0
    stVoipCfg.ch_id = chid;
    stVoipCfg.m_id = sid;
    stVoipCfg.enable = enable;
    SETSOCKOPT(VOIP_MGR_HOLD, &stVoipCfg, TstVoipCfg, 1);
#else
    stVoipCfg.ch_id = chid;
    stVoipCfg.m_id = sid;
    stVoipCfg.enable = !enable;
    SETSOCKOPT(VOIP_MGR_RTP_CFG, &stVoipCfg, TstVoipCfg, 1);
#endif

    return 0;
}

int32 rtk_enable_pcm(uint32 chid, int32 bEnable)
{
    TstVoipCfg stVoipCfg;

    stVoipCfg.ch_id = chid;
    stVoipCfg.enable = bEnable;
    SETSOCKOPT(VOIP_MGR_PCM_CFG, &stVoipCfg, TstVoipCfg, 1);
    SETSOCKOPT(VOIP_MGR_DTMF_CFG, &stVoipCfg, TstVoipCfg, 1);
}

uint32 rtk_Get_SLIC_Reg_Val(uint32 chid, uint32 reg)
{
	TstVoipSlicReg stSlicReg;

	stSlicReg.ch_id = chid;
	stSlicReg.reg_num = reg;
	SETSOCKOPT(VOIP_MGR_GET_SLIC_REG_VAL, &stSlicReg, TstVoipSlicReg, 1);
	return stSlicReg.reg_val;
}

int rtk_Get_DAA_Used_By_Which_SLIC(uint32 chid)
{

	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;

	SETSOCKOPT(VOIP_MGR_GET_DAA_USED_BY_WHICH_SLIC, &stVoipValue, TstVoipValue, 1);

	return stVoipValue.value;
	
}

int rtk_DAA_on_hook(uint32 chid)// for virtual DAA on-hook(channel ID is FXS channel)
{
	if ((g_VoIP_Feature & DAA_TYPE_MASK) != NO_DAA )
	{
		TstVoipCfg stVoipCfg;
	    	stVoipCfg.ch_id = chid;

		SETSOCKOPT(VOIP_MGR_DAA_ON_HOOK, &stVoipCfg, TstVoipCfg, 1);
        }
	return 0;
}

int rtk_DAA_off_hook(uint32 chid)// for virtual DAA off-hook(channel ID is FXS channel)
{
	if ((g_VoIP_Feature & DAA_TYPE_MASK) != NO_DAA )
	{
		TstVoipCfg stVoipCfg;
	    	stVoipCfg.ch_id = chid;

		SETSOCKOPT(VOIP_MGR_DAA_OFF_HOOK, &stVoipCfg, TstVoipCfg, 1);

		return stVoipCfg.enable; /* 1: success, 0xff: line not connect or busy or not support */
	}
	else
	{
		return 0;
	}
}

int rtk_DAA_ring(uint32 chid)
{

	if ((g_VoIP_Feature & DAA_TYPE_MASK) != NO_DAA )
	{
		TstVoipCfg stVoipCfg;
	    	stVoipCfg.ch_id = chid;

		SETSOCKOPT(VOIP_MGR_DAA_RING, &stVoipCfg, TstVoipCfg, 1);

		return stVoipCfg.enable;
	}
	else
	{
		return 0;
	}
}

int32 rtk_Set_Tone_Country(voipCfgParam_t *voip_ptr)
{
	TstVoipValue stVoipValue;
	char country;

  	country = voip_ptr->tone_of_country;

	stVoipValue.value = country;
	SETSOCKOPT(VOIP_MGR_SET_TONE_OF_COUNTRY, &stVoipValue, TstVoipValue, 1);

	if (country == TONE_CUSTOMER)
	{
                
		stVoipValue.value1 = voip_ptr->tone_of_custdial;
		stVoipValue.value2 = voip_ptr->tone_of_custring;
		stVoipValue.value3 = voip_ptr->tone_of_custbusy;;
		stVoipValue.value4 = voip_ptr->tone_of_custwaiting;
		SETSOCKOPT(VOIP_MGR_USE_CUST_TONE, &stVoipValue, TstVoipValue, 1);
	}

	return 0;
}


int32 rtk_Set_Custom_Tone(uint8 custom, st_ToneCfgParam *pstToneCfgParam)
{
	TstVoipValue stVoipValue;
	TstVoipToneCfg stVoipToneCfg;

	stVoipValue.value = custom;
	memcpy(&stVoipToneCfg, pstToneCfgParam, sizeof(TstVoipToneCfg));
	SETSOCKOPT(VOIP_MGR_SET_TONE_OF_CUSTOMIZE, &stVoipValue, TstVoipValue, 1);
	SETSOCKOPT(VOIP_MGR_SET_CUST_TONE_PARAM, &stVoipToneCfg, TstVoipToneCfg, 1);

	return 0;
}

int rtk_Set_SLIC_Reg_Val(int chid, int reg, int value)
{
	TstVoipSlicReg stSlicReg;

	stSlicReg.ch_id = chid;
	stSlicReg.reg_num = reg;
	stSlicReg.reg_val = value;
	SETSOCKOPT(VOIP_MGR_SET_SLIC_REG_VAL, &stSlicReg, TstVoipSlicReg, 1);
	return 0;
}

int rtk_reset_slic(int chid)
{
	TstVoipSlicRestart stVoipSlicRestart;

	stVoipSlicRestart.ch_id = chid;
	stVoipSlicRestart.codec_law = 2; // u-law
	SETSOCKOPT(VOIP_MGR_SLIC_RESTART, &stVoipSlicRestart, TstVoipSlicRestart, 1);
	return 0;
}

int rtk_debug(int dbg_flag)
{
	TstVoipValue stVoipValue;

	stVoipValue.value = dbg_flag;
	SETSOCKOPT(VOIP_MGR_DEBUG, &stVoipValue, TstVoipValue, 1);
	return 0;
}

int rtk_8305_switch(unsigned short phy,unsigned short reg,unsigned short value,unsigned short r_w)
{
	TstVoipSwitch switch_value;

	switch_value.phy = phy;
	switch_value.reg = reg;
	switch_value.value = value;
	switch_value.read_write = r_w;
	SETSOCKOPT(VOIP_MGR_8305_SWITCH_VAL, &switch_value, TstVoipSwitch,1);
	return 0;
}


int rtk_WAN_Clone_MAC(unsigned char* MAC){
	TstVoipCloneMAC MAC_address;
	memcpy(MAC_address.CloneMACAddress, MAC, 6);
	SETSOCKOPT(VOIP_MGR_SET_WAN_CLONE_MAC, &MAC_address, TstVoipCloneMAC,1);
	return 0;
}





#ifdef CONFIG_RTK_VOIP_WAN_VLAN

int rtk_switch_wan_3_vlan(voipCfgParam_t *voip_ptr)
{
	TstVoipSwitch_3_VLAN_tag wan_3_vlan_tag;

	if (voip_ptr->wanVlanEnable) {
		wan_3_vlan_tag.enable = voip_ptr->wanVlanEnable;
		wan_3_vlan_tag.vlanIdVoice =  voip_ptr->wanVlanIdVoice;
		wan_3_vlan_tag.priorityVoice =  voip_ptr->wanVlanPriorityVoice;
		wan_3_vlan_tag.cfiVoice =  voip_ptr->wanVlanCfiVoice;
                wan_3_vlan_tag.vlanIdData =  voip_ptr->wanVlanIdData;
                wan_3_vlan_tag.priorityData =  voip_ptr->wanVlanPriorityData;
                wan_3_vlan_tag.cfiData =  voip_ptr->wanVlanCfiData;
                wan_3_vlan_tag.vlanIdVideo =  voip_ptr->wanVlanIdVideo;
                wan_3_vlan_tag.priorityVideo =  voip_ptr->wanVlanPriorityVideo;
                wan_3_vlan_tag.cfiVideo =  voip_ptr->wanVlanCfiVideo;
	} else {
		memset(&wan_3_vlan_tag, 0, sizeof(wan_3_vlan_tag));
	}
	SETSOCKOPT(VOIP_MGR_WAN_3_VLAN_TAG, &wan_3_vlan_tag, TstVoipSwitch_VLAN_tag,1);
	return 0;
}


int rtk_switch_wan_2_vlan(voipCfgParam_t *voip_ptr)
{
	TstVoipSwitch_2_VLAN_tag wan_2_vlan_tag;

	if (voip_ptr->wanVlanEnable) {
		wan_2_vlan_tag.enable = voip_ptr->wanVlanEnable;
		wan_2_vlan_tag.vlanIdVoice =  voip_ptr->wanVlanIdVoice;
		wan_2_vlan_tag.priorityVoice =  voip_ptr->wanVlanPriorityVoice;
		wan_2_vlan_tag.cfiVoice =  voip_ptr->wanVlanCfiVoice;
                wan_2_vlan_tag.vlanIdData =  voip_ptr->wanVlanIdData;
                wan_2_vlan_tag.priorityData =  voip_ptr->wanVlanPriorityData;
                wan_2_vlan_tag.cfiData =  voip_ptr->wanVlanCfiData;
	} else {
		memset(&wan_2_vlan_tag, 0, sizeof(wan_2_vlan_tag));
	}
	SETSOCKOPT(VOIP_MGR_WAN_2_VLAN_TAG, &wan_2_vlan_tag, TstVoipSwitch_VLAN_tag,1);
	return 0;
}


int rtk_switch_wan_vlan(voipCfgParam_t *voip_ptr)
{
	TstVoipSwitch_VLAN_tag wan_vlan_tag;

	if (voip_ptr->wanVlanEnable) {
		wan_vlan_tag.enable = voip_ptr->wanVlanEnable;
		wan_vlan_tag.vlanId =  voip_ptr->wanVlanIdVoice;
		wan_vlan_tag.priority =  voip_ptr->wanVlanPriorityVoice;
		wan_vlan_tag.cfi =  voip_ptr->wanVlanCfiVoice;
	} else {
		memset(&wan_vlan_tag, 0, sizeof(wan_vlan_tag));
	}
	SETSOCKOPT(VOIP_MGR_WAN_VLAN_TAG, &wan_vlan_tag, TstVoipSwitch_VLAN_tag,1);
	return 0;
}

#endif // CONFIG_RTK_VOIP_WAN_VLAN

#if 0
int rtk_8305_switch_wan_vlan_tag(unsigned char enable,unsigned short wan_vlan_id)
{
	TstVoipSwitch_VLAN_tag wan_vlan_tag;

	wan_vlan_tag.WAN_VLAN_TAG_ENABLE = enable;
	wan_vlan_tag.WAN_VLAN_ID = wan_vlan_id;
	SETSOCKOPT(VOIP_MGR_WAN_VLAN_TAG, &wan_vlan_tag, TstVoipSwitch_VLAN_tag,1);
	return 0;
}

int rtk_8305_switch_bridge_mode(unsigned char bridge_mode_enable)
{
	SETSOCKOPT(VOIP_MGR_BRIDGE_MODE, &bridge_mode_enable, unsigned char,1);
	return 0;
}
#endif

#ifdef CONFIG_RTK_VOIP_IVR
int rtk_IvrStartPlaying( unsigned int chid, unsigned char *pszText2Speech )
{
	TstVoipPlayIVR_Text stVoipPlayIVR_Text;
	int len;

	stVoipPlayIVR_Text.ch_id = chid;
	stVoipPlayIVR_Text.type = IVR_PLAY_TYPE_TEXT;
	memcpy( stVoipPlayIVR_Text.szText2speech, pszText2Speech, MAX_LEN_OF_IVR_TEXT );
	stVoipPlayIVR_Text.szText2speech[ MAX_LEN_OF_IVR_TEXT ] = '\x0';

	SETSOCKOPT( VOIP_MGR_PLAY_IVR, &stVoipPlayIVR_Text, TstVoipPlayIVR_Text, 1 );

	return ( int )stVoipPlayIVR_Text.playing_period_10ms;
}

int rtk_IvrStartPlayG72363( unsigned int chid, unsigned int nFrameCount, const unsigned char *pData )
{
	TstVoipPlayIVR_G72363 stVoipPlayIVR;
	
	if( nFrameCount > MAX_FRAMES_OF_G72363 )
		nFrameCount = MAX_FRAMES_OF_G72363;
	
	stVoipPlayIVR.ch_id = chid;
	stVoipPlayIVR.type = IVR_PLAY_TYPE_G723_63;
	stVoipPlayIVR.nFramesCount = nFrameCount;
	memcpy( stVoipPlayIVR.data, pData, nFrameCount * 24 );

    SETSOCKOPT(VOIP_MGR_PLAY_IVR, &stVoipPlayIVR, TstVoipPlayIVR_G72363, 1);
    
    return stVoipPlayIVR.nRetCopiedFrames;
}

int rtk_IvrStartPlayG729( unsigned int chid, unsigned int nFrameCount, const unsigned char *pData )
{
	TstVoipPlayIVR_G729 stVoipPlayIVR;
	
	if( nFrameCount > MAX_FRAMES_OF_G729 )
		nFrameCount = MAX_FRAMES_OF_G729;
	
	stVoipPlayIVR.ch_id = chid;
	stVoipPlayIVR.type = IVR_PLAY_TYPE_G729;
	stVoipPlayIVR.nFramesCount = nFrameCount;
	memcpy( stVoipPlayIVR.data, pData, nFrameCount * 10 );

    SETSOCKOPT(VOIP_MGR_PLAY_IVR, &stVoipPlayIVR, TstVoipPlayIVR_G729, 1);
    
    return stVoipPlayIVR.nRetCopiedFrames;
}

int rtk_IvrPollPlaying( unsigned int chid )
{
	TstVoipPollIVR stVoipPollIVR;

	stVoipPollIVR.ch_id = chid;

	SETSOCKOPT( VOIP_MGR_POLL_IVR, &stVoipPollIVR, TstVoipPollIVR, 1 );

	return ( int )stVoipPollIVR.bPlaying;
}

int rtk_IvrStopPlaying( unsigned int chid )
{
	TstVoipStopIVR stVoipStopIVR;

	stVoipStopIVR.ch_id = chid;

	SETSOCKOPT( VOIP_MGR_STOP_IVR, &stVoipStopIVR, TstVoipStopIVR, 1 );

	return 0;
}
#endif /* CONFIG_RTK_VOIP_IVR */

int rtk_sip_register(unsigned int chid, unsigned int isOK)
{
   	TstVoipCfg stVoipCfg;

	stVoipCfg.ch_id = chid;
	stVoipCfg.enable = isOK;
	SETSOCKOPT(VOIP_MGR_SIP_REGISTER, &stVoipCfg, TstVoipCfg, 1);

	return 0;
}

int32 rtk_Set_CID_DTMF_MODE(uint32 chid, char area, char cid_dtmf_mode)
{
	TstVoipCID stCIDstr;

	stCIDstr.ch_id = chid;
	stCIDstr.cid_mode = area;
	stCIDstr.cid_dtmf_mode = cid_dtmf_mode;

	SETSOCKOPT(VOIP_MGR_SET_CID_DTMF_MODE, &stCIDstr, TstVoipCID, 1);

	return 0;
}


int32 rtk_SIP_INFO_play_tone(unsigned int chid, unsigned int ssid, DSPCODEC_TONE tone, unsigned int duration)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.m_id = ssid;
	stVoipValue.value = tone;
	stVoipValue.value5 = duration;

	SETSOCKOPT(VOIP_MGR_PLAY_SIP_INFO, &stVoipValue, TstVoipValue, 1);

	return 0;
}


int32 rtk_Set_SPK_AGC(uint32 chid, uint32 support_gain)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.value = support_gain;

	SETSOCKOPT(VOIP_MGR_SET_SPK_AGC, &stVoipValue, TstVoipValue, 1);


	return 0;
}

int32 rtk_Set_SPK_AGC_LVL(uint32 chid, uint32 level)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.value = level;

	SETSOCKOPT(VOIP_MGR_SET_SPK_AGC_LVL, &stVoipValue, TstVoipValue, 1);


	return 0;
}


int32 rtk_Set_SPK_AGC_GUP(uint32 chid, uint32 gain)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.value = gain;

	SETSOCKOPT(VOIP_MGR_SET_SPK_AGC_GUP, &stVoipValue, TstVoipValue, 1);


	return 0;
}


int32 rtk_Set_SPK_AGC_GDOWN(uint32 chid, uint32 gain)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.value = gain;

	SETSOCKOPT(VOIP_MGR_SET_SPK_AGC_GDOWN, &stVoipValue, TstVoipValue, 1);


	return 0;
}


int32 rtk_Set_MIC_AGC(uint32 chid, uint32 support_gain)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.value = support_gain;

	SETSOCKOPT(VOIP_MGR_SET_MIC_AGC, &stVoipValue, TstVoipValue, 1);


	return 0;
}

int32 rtk_Set_MIC_AGC_LVL(uint32 chid, uint32 level)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.value = level;

	SETSOCKOPT(VOIP_MGR_SET_MIC_AGC_LVL, &stVoipValue, TstVoipValue, 1);


	return 0;
}

int32 rtk_Set_MIC_AGC_GUP(uint32 chid, uint32 gain)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.value = gain;

	SETSOCKOPT(VOIP_MGR_SET_MIC_AGC_GUP, &stVoipValue, TstVoipValue, 1);


	return 0;
}


int32 rtk_Set_MIC_AGC_GDOWN(uint32 chid, uint32 gain)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.value = gain;

	SETSOCKOPT(VOIP_MGR_SET_MIC_AGC_GDOWN, &stVoipValue, TstVoipValue, 1);


	return 0;
}

#if 0

int32 rtk_Get_DAA_ISR_FLOW(unsigned int chid ,unsigned int mid)
{
	unsigned int flow;
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	stVoipValue.m_id = mid;

	SETSOCKOPT(VOIP_MGR_GET_DAA_ISR_FLOW, &stVoipValue, TstVoipValue, 1);
	flow = stVoipValue.value;

	return flow;
}

/* Usage:
	  rtk_Set_DAA_ISR_FLOW(chid, DAA_FLOW_NORMAL)
	  rtk_Set_DAA_ISR_FLOW(chid, DAA_FLOW_3WAY_CONFERENCE)
	  rtk_Set_DAA_ISR_FLOW(chid, DAA_FLOW_CALL_FORWARD)
*/
int32 rtk_Set_DAA_ISR_FLOW(unsigned int chid, unsigned int mid, unsigned int flow)
{
	TstVoipValue stVoipValue;
	int res;

	stVoipValue.ch_id = chid;
	stVoipValue.m_id = mid;
	stVoipValue.value = flow;


	if( flow == 0 ) /* Normal */
	{
		if (rtk_Get_DAA_ISR_FLOW(chid, mid) == DAA_FLOW_CALL_FORWARD)
			rtk_eanblePCM(chid, 0);
		rtk_DAA_on_hook(chid);
		res = 1;
	}
	else if ( flow == 1 ) /* PSTN 3-way conference */
	{
		res = rtk_DAA_off_hook(chid);

		if (res == 0xff)
			stVoipValue.value = DAA_FLOW_NORMAL;
	}
	else if ( flow == 2 ) /* PSTN call forward */
	{
		rtk_eanblePCM(chid, 1);
		res = rtk_DAA_off_hook(chid);

		if (res == 0xff)
		{
			rtk_eanblePCM(chid, 0);
			stVoipValue.value = DAA_FLOW_NORMAL;
		}
	}

	SETSOCKOPT(VOIP_MGR_SET_DAA_ISR_FLOW, &stVoipValue, TstVoipValue, 1);


	return res; /* 1: success, 0xff: line not connect or busy or not support */
}

#endif

int32 rtk_Dial_PSTN_Call_Forward(uint32 chid, uint32 sid, char *cf_no_str)
{
   	char cf_no[21];
   	int len = strlen(cf_no_str), i;

   	strcpy(cf_no, cf_no_str);

   	usleep(200000);  // 200ms
        printf("PSTN Call Forward Dial: ");
   	for(i = 0; i < len; i++)
   	{
   		rtk_SetPlayTone(chid, sid, cf_no[i]-'0', 1, DSPCODEC_TONEDIRECTION_LOCAL);
                printf("%d ", cf_no[i]-'0');
   		usleep(100000);  // 100ms
   		rtk_SetPlayTone(chid, sid, cf_no[i]-'0', 0, DSPCODEC_TONEDIRECTION_LOCAL);
   		usleep(50000);  // 50ms
   	}
        printf("\n");

	return 0;
}

#if 0

int32 rtk_Set_PSTN_HOLD_CFG(unsigned int slic_chid, unsigned int daa_chid, unsigned int config)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = slic_chid;
	stVoipValue.value2 = daa_chid;
	stVoipValue.value = config; /* 1: Hold, 0: Un-Hold*/


	SETSOCKOPT(VOIP_MGR_SET_DAA_PCM_HOLD_CFG, &stVoipValue, TstVoipValue, 1);


	return 0;
}

int32 rtk_Get_DAA_BusyTone_Status(unsigned int daa_chid)
{
	TstVoipValue stVoipValue;
	int busy_flag;	/* Busy tone is  1: Detected, 0: NOT detected. */

	stVoipValue.ch_id = daa_chid;

	SETSOCKOPT(VOIP_MGR_GET_DAA_BUSY_TONE_STATUS, &stVoipValue, TstVoipValue, 1);
	busy_flag = stVoipValue.value;

	return busy_flag;
}

#endif

int32 rtk_Get_DAA_CallerID(uint32 chid, char *str_cid, char *str_date)                     
{
	TstVoipCID stCIDstr;

	stCIDstr.daa_id = chid;

	SETSOCKOPT(VOIP_MGR_GET_DAA_CALLER_ID, &stCIDstr, TstVoipCID, 1);

	strcpy(str_cid, stCIDstr.string);
	strcpy(str_date, stCIDstr.string2);

	return 0;
}

int32 rtk_Get_VoIP_Feature(void)
{
   	TstVoipCfg stVoipCfg;

	SETSOCKOPT(VOIP_MGR_GET_FEATURE, &stVoipCfg, TstVoipCfg, 1);

	g_VoIP_Feature = stVoipCfg.cfg;

	return 0;
}

int32 rtk_Set_CID_Det_Mode(uint32 chid, int auto_det, int cid_det_mode)
{
   	TstVoipCfg stVoipCfg;

   	stVoipCfg.ch_id = chid;
   	stVoipCfg.enable = auto_det; /* 0: disable 1: enable(NTT) 2: enable (NOT NTT) */
   	stVoipCfg.cfg = cid_det_mode;

	SETSOCKOPT(VOIP_MGR_SET_CID_DET_MODE, &stVoipCfg, TstVoipCfg, 1);

	return 0;
}

int32 rtk_GetFskCIDState(uint32 chid, uint32 *cid_state)
{
	TstVoipCID stCIDstr;

	stCIDstr.ch_id = chid;
	SETSOCKOPT(VOIP_MGR_GET_FSK_CID_STATE_CFG, &stCIDstr, TstVoipCID, 1);
	*cid_state = stCIDstr.cid_state;

	return 0;
}

int32 rtk_Set_CID_FSK_GEN_MODE(unsigned int chid, unsigned int isOK)
{
   	TstVoipCfg stVoipCfg;

	stVoipCfg.ch_id = chid;
	stVoipCfg.enable = isOK;
	SETSOCKOPT(VOIP_MGR_SET_CID_FSK_GEN_MODE, &stVoipCfg, TstVoipCfg, 1);

	return 0;
}

int32 rtk_Set_Voice_Gain(uint32 chid, int spk_gain, int mic_gain)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;   
	stVoipValue.value = spk_gain;
	stVoipValue.value1 = mic_gain;
	
	SETSOCKOPT(VOIP_MGR_SET_VOICE_GAIN, &stVoipValue, TstVoipValue, 1);

	return 0;   
}

int32 rtk_Gen_FSK_CID_VMWI(uint32 chid, char *str_cid, char *str_date, char *str_cid_name, char mode, char msg_type)
{
	TstVoipCID stCIDstr;

	stCIDstr.ch_id = chid;
	stCIDstr.cid_mode = mode;       //0:on-hook
	stCIDstr.cid_msg_type = msg_type;	// FSK_MSG_CALLSETUP:cid or FSK_MSG_MWSETUP:vmwi
	if (str_cid[0] == 0)               // not null str
		strcpy(stCIDstr.string, "0123456789");   // replace "0123456789" with From CID
	else
		strcpy(stCIDstr.string, str_cid);
	strcpy(stCIDstr.string2, str_date);
	strcpy(stCIDstr.cid_name, str_cid_name);

	int32 tmp=-1;
	do
	{
		if(rtk_GetFskCIDState(chid, &tmp) != 0)
			break;
		usleep(50000);  // 50ms
	}
	while (tmp);

	SETSOCKOPT(VOIP_MGR_FSK_CID_VMWI_GEN_CFG, &stCIDstr, TstVoipCID, 1);
	// remember set slic in transmit mode, enable DSP pcm.
	return 0;
}


int32 rtk_Set_GetPhoneStat(TstVoipCfg* pstVoipCfg)
{
	SETSOCKOPT(VOIP_MGR_GET_SLIC_STAT, pstVoipCfg, TstVoipCfg, 1);
	return 0;
}

int32 rtk_Set_GETDATA_Mode(TstVoipdataget_o* pstVoipdataget_o)
{
	SETSOCKOPT(VOIP_MGR_SET_GETDATA_MODE, pstVoipdataget_o, TstVoipdataget_o, 1);
	return 0;
}

int32 rtk_Get_Rtp_Statistics( uint32 chid, uint32 bReset, TstVoipRtpStatistics *pstVoipRtpStatistics )
{
	pstVoipRtpStatistics ->chid = chid;
	pstVoipRtpStatistics ->bResetStatistics = bReset;
	
	SETSOCKOPT( VOIP_MGR_GET_RTP_STATISTICS, pstVoipRtpStatistics, TstVoipRtpStatistics, 1 );

	return 0;
}

int32 rtk_Set_DSCP(int32 sip_rtp_dscp)
{
	SETSOCKOPT(VOIP_MGR_SET_DSCP, &sip_rtp_dscp, int32, 1);
	return 0;
}

#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
int rtk_Set_IPhone(unsigned int function_type, unsigned int reg, unsigned int value)
{
	IPhone_test iphone;
	
	iphone.function_type = function_type;
	iphone.reg = reg;
	iphone.value = value;
	SETSOCKOPT(VOIP_MGR_IPHONE_TEST, &iphone, IPhone_test, 1);
	return 0;
}
#endif

/******************** New Add for AudioCodes Solution ****************/
int32 rtk_Onhook_Action(uint32 chid)
{
    	TstVoipCfg stVoipCfg;

        stVoipCfg.ch_id = chid;
        SETSOCKOPT(VOIP_MGR_SLIC_ONHOOK_ACTION, &stVoipCfg, TstVoipCfg, 1);
	return 0;
}

int32 rtk_Offhook_Action(uint32 chid)
{
    	TstVoipCfg stVoipCfg;

        stVoipCfg.ch_id = chid;
        SETSOCKOPT(VOIP_MGR_SLIC_OFFHOOK_ACTION, &stVoipCfg, TstVoipCfg, 1);
	return 0;
}

/**********************************************************************/

#ifdef CONFIG_RTK_VOIP_IP_PHONE
int rtk_GetIPPhoneHookStatus( uint32 *pHookStatus )
{
	TstKeypadHookStatus stKeypadHookStatus;
	
	stKeypadHookStatus.cmd = KEYPAD_CMD_HOOK_STATUS;
	SETSOCKOPT( VOIP_MGR_CTL_KEYPAD, &stKeypadHookStatus, TstKeypadHookStatus, 1);
	
	if( stKeypadHookStatus.status ) 
		printf( "-------------------------------------------\nOff-hook\n" );
	else
		printf( "-------------------------------------------\nOn-hook\n" );

	return 0;
}
#endif /* CONFIG_RTK_VOIP_IP_PHONE */

int rtk_Set_flush_fifo(uint32 chid)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	SETSOCKOPT(VOIP_MGR_FLUSH_FIFO, &stVoipValue, TstVoipValue, 1);

	return 0;
}

int rtk_line_check(uint32 chid)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;
	SETSOCKOPT(VOIP_MGR_LINE_CHECK, &stVoipValue, TstVoipValue, 1);

	return stVoipValue.value;
	/* for FXS: 0: connect, 1: not connect */
	/* for FXO: 0: connect, 1: not connect, 2: busy */
}

int rtk_FXO_offhook(uint32 chid)// for real DAA off-hook(channel is FXO channel)
{
	if ((g_VoIP_Feature & DAA_TYPE_MASK) != NO_DAA )
	{
		TstVoipCfg stVoipCfg;
	    	stVoipCfg.ch_id = chid;

		SETSOCKOPT(VOIP_MGR_FXO_OFF_HOOK, &stVoipCfg, TstVoipCfg, 1);

		return stVoipCfg.enable; /* 1: success, 0xff: line not connect or busy or not support */
	}
	else
	{
		return 0;
	}
}

int rtk_FXO_onhook(uint32 chid)// for real DAA on-hook(channel is FXO channel)
{
	if ((g_VoIP_Feature & DAA_TYPE_MASK) != NO_DAA )
	{
		TstVoipCfg stVoipCfg;
	    	stVoipCfg.ch_id = chid;

		SETSOCKOPT(VOIP_MGR_FXO_ON_HOOK, &stVoipCfg, TstVoipCfg, 1);
        }
	return 0;
}

int rtk_FXO_RingOn(uint32 chid)
{
	TstVoipCfg stVoipCfg;
    	stVoipCfg.ch_id = chid;
    	stVoipCfg.cfg = 11; //FXO_RING_ON
        	
    	SETSOCKOPT(VOIP_MGR_HOOK_FIFO_IN, &stVoipCfg, TstVoipCfg, 1);
}

int rtk_FXO_Busy(uint32 chid)
{
	TstVoipCfg stVoipCfg;
    	stVoipCfg.ch_id = chid;
    	stVoipCfg.cfg = 13; //FXO_BUSY_TONE	

    	SETSOCKOPT(VOIP_MGR_HOOK_FIFO_IN, &stVoipCfg, TstVoipCfg, 1);
}

uint32 rtk_Get_SLIC_Ram_Val(uint8 chid, uint16 reg)
{
	TstVoipSlicRam stSlicReg;

	stSlicReg.ch_id = chid;
	stSlicReg.reg_num = reg;
	SETSOCKOPT(VOIP_MGR_GET_SLIC_RAM_VAL, &stSlicReg, TstVoipSlicRam, 1);
	return stSlicReg.reg_val;
}

int rtk_Set_SLIC_Ram_Val(uint8 chid, uint16 reg, uint32 value)
{
	TstVoipSlicRam stSlicReg;

	stSlicReg.ch_id = chid;
	stSlicReg.reg_num = reg;
	stSlicReg.reg_val = value;
	SETSOCKOPT(VOIP_MGR_SET_SLIC_RAM_VAL, &stSlicReg, TstVoipSlicRam, 1);
	return 0;
}



