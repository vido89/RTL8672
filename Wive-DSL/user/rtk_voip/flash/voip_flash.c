/*
 * voip_flash.c: VoIP Flash File
 *
 * Authors: Rock <shaofu@realtek.com.tw>
 *
 */

#include <string.h>
#include "voip_flash.h"
#include "voip_flash_mib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>
#include "voip_flash.h"
#include "voip_flash_tool.h"

#if CONFIG_RTK_VOIP_PACKAGE_865X
	#include "rtl_cfg.h"
	#include "rtl_board.h"
	#include <sys/shm.h>
#elif CONFIG_RTK_VOIP_PACKAGE_867X
	#include <sys/shm.h>
	#include "mib.h"
#else
	#include "apmib.h"
#endif

#ifdef SUPPORT_CODEC_DESCRIPTOR
#include "codec_table.h"
#endif

#ifdef CONFIG_RTK_VOIP_G729AB
#define G729AB_DEF_FRAME_SIZE	( 2 - 1 ),
#else
#define G729AB_DEF_FRAME_SIZE
#endif

#ifdef CONFIG_RTK_VOIP_G7231
#define G723_DEF_FRAME_SIZE		( 1 - 1 ),
#else
#define G723_DEF_FRAME_SIZE
#endif

#ifdef CONFIG_RTK_VOIP_G726
#define G726_16_DEF_FRAME_SIZE	( 2 - 1 ),
#define G726_24_DEF_FRAME_SIZE	( 2 - 1 ),
#define G726_32_DEF_FRAME_SIZE	( 2 - 1 ),
#define G726_40_DEF_FRAME_SIZE	( 2 - 1 ),
#else
#define G726_16_DEF_FRAME_SIZE
#define G726_24_DEF_FRAME_SIZE
#define G726_32_DEF_FRAME_SIZE
#define G726_40_DEF_FRAME_SIZE
#endif

#ifdef CONFIG_RTK_VOIP_GSMFR
#define GSMFR_DEF_FRAME_SIZE	( 1 - 1 ),
#else
#define GSMFR_DEF_FRAME_SIZE
#endif

#ifdef CONFIG_RTK_VOIP_ILBC
#define ILBC_DEF_FRAME_SIZE		( 1 - 1 ),
#else
#define ILBC_DEF_FRAME_SIZE
#endif

#define	M_DEFAULT_FRAME_SIZE_LIST								\
	( 2 - 1 ),				/* SUPPORTED_CODEC_G711U = 0, */	\
	( 2 - 1 ),				/* SUPPORTED_CODEC_G711A, */		\
	G729AB_DEF_FRAME_SIZE	/* SUPPORTED_CODEC_G729, */			\
	G723_DEF_FRAME_SIZE		/* SUPPORTED_CODEC_G723, */			\
	G726_16_DEF_FRAME_SIZE	/* SUPPORTED_CODEC_G726_16, */		\
	G726_24_DEF_FRAME_SIZE	/* SUPPORTED_CODEC_G726_24, */		\
	G726_32_DEF_FRAME_SIZE	/* SUPPORTED_CODEC_G726_32, */		\
	G726_40_DEF_FRAME_SIZE	/* SUPPORTED_CODEC_G726_40, */		\
	GSMFR_DEF_FRAME_SIZE	/* SUPPORTED_CODEC_GSMFR, */		\
	ILBC_DEF_FRAME_SIZE		/* SUPPORTED_CODEC_ILBC, */			\

#ifdef SUPPORT_CODEC_DESCRIPTOR
/* add the new codec to frame list */
CT_ASSERT( SUPPORTED_CODEC_MAX == NUM_CODEC_TABLE );
#endif

#define M_DEFAULT_PRECEDENACE_LIST		\
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9

#ifdef SUPPORT_CODEC_DESCRIPTOR
/* number of list should be identical to _CODEC_MAX */
CT_ASSERT( _CODEC_MAX == 10 );
#endif

voipCfgParam_t voipCfgParamDefault =
{
	VOIP_FLASH_SIGNATURE,
	VOIP_FLASH_VER,
	VOIP_FLASH_FEATURE,
	VOIP_FLASH_EXT_FEATURE,
	VOIP_FLASH_MIB_VER,
	0,					// rfc_flags
	TONE_USA,			// tone_of_country 
	TONE_CUSTOMER_1,	// tone_of_custdial 
	TONE_CUSTOMER_2,	// tone_of_custring
	TONE_CUSTOMER_3,	// tone_of_custbusy
	TONE_CUSTOMER_4,	// tone_of_custwaiting
	TONE_CUSTOMER_1,	// tone_of_customize
	{{0}},				// cust_tone_para
	8,					// ring_cad 
	RING_GROUP_1,		// ring_group 
	{0},				// ring_phone_num
	{RING_CADENCE_1, RING_CADENCE_1, RING_CADENCE_1, RING_CADENCE_1},	// ring_cadence_use
	RING_CADENCE_1,		// ring_cadence_sel 
	{1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500},	// ring_cadon
	{1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500},	// ring_cadoff
	".0",				// funckey_pstn: default is *0
	".1",				// funckey_transfer: default is *1
	5,					// auto dial
	30,					// off hook alarm
	2,					// caller id auto detection selection  0: disable 1: enable(NTT) 2: enable (NOT NTT) 
	CID_DTMF,				// caller id detection mode
	0,					// auto_cfg_ver
	0,					// auto_cfg_mode
	{0},				// auto_cfg_http_addr
	80,					// auto_cfg_http_port
	{0},				// auto_cfg_file_path
	0,					// auto_cfg_expire
	0, 				// wanVlanEnable;
	1,				// wanVlanIdVocie;
	7,				// wanVlanPriorityVoice;
	0,				// wanVlanCfiVoice;
	2,				// wanVlanIdData;
	0,				// wanVlanPriorityData;
	0,				// wanVlanCfiData;	
	3,				// wanVlanIdVideo;
	5,				// wanVlanPriorityVideo;
	0,				// wanVlanCfiVideo;	
	7,				// daa_txVolumne
	7,				// daa_rxVolumne
	8,				// rtpDscp
	8,				// sipDscp
	// ports[VOIP_PORTS]
	{
		// port 0
		{
			// proxies
			{
				// proxies[0]
				{
					// account
			"\0",				// display_name
			"\0",				// number
			"\0",				// login_id
			"\0",				// password
					// register server
					0,					// enable
					"\0",				// addr
					DEF_SIP_PROXY_PORT,	// port
			"\0",				// domain_name
					60,					// reg_expire
					// nat traversal server
					0,							// outbound_enable
					"\0",						// outbound_addr
					DEF_OUTBOUND_PROXY_PORT,	// outbound_port
				},
			#if (MAX_PROXY > 1)
				// proxies[1] .. proxies[MAX_PROXY - 1]
			#endif
			},
			0,							// default_proxy
			// NAT Traversal
			0,					// stun_enable
			"\0",				// stun_addr
			DEF_STUN_SERVER_PORT,	// stun_port
			// advanced
			DEF_SIP_LOCAL_PORT,	// sip_port
			DEF_RTP_PORT,		// media_port
			DTMF_INBAND,		// dtmf_mode
			96,					// payload_type
			250,					// sip info duration
			1,					// call_waiting_enable
			// forward
			0,					// uc_forward_enable
			"\0",				// uc_forward
			0,					// busy_forward_enable
			"\0",				// busy_forward
			0,					// na_forward_enable
			"\0",				// na_forward
			0,					// na_forward_time
			{{{0}}},			// speed_dial
			0,					// replace rule option
			"\0",				// replace rule source plan
			"\0",				// replace rule target
			"\0",				// dialplan
			"\0",				// auto prefix
			"\0",				// prefix unset plan
			// codec
			{M_DEFAULT_FRAME_SIZE_LIST},// frame_size
			{M_DEFAULT_PRECEDENACE_LIST}, 	// precedence
			0,					// vad
			G7231_RATE63,		// g7231_rate
			ILBC_30MS,			// iLBC_mode
			// DSP
			6,				// slic_txVolumne
			6,				// slic_rxVolumne
			4,					// jitter delay 
			13,					// maxDelay
			7,					// jitter factor 
			2,					// echoTail
			(CID_DTMF|0x08),			// caller_id_mode
			0,				//use call waiting caller ID or not.0: disable 1: enable
			8,				// cid_dtmf_mode
			0,				// speaker agc
			0,				// speaker agc require level
			5,				// speaker agc max gain up
			5,				// speaker agc max gain down
			0,				// mic agc
			0,				// mic agc require level
			5,				// mic agc max gain up
			5,				// mic agc max gain down
			1,				// fsk gen mode 0:hardware, 1: software dsp gen
			0,				// spk voice gain
			0,				// mic voice gain
			// QoS
			DSCP_EF,
			// T.38
			0,				//enable T.38
			DEF_T38_PORT,	//T.38 default port	
			0,				// hotline_enable: 0 is disable
			{0},			// hotline_number
			0,				// dnd mode: 0 is disable
			0,				// dnd_from_hour
			0,				// dnd_from_min
			0,				// dnd_to_hour
			0,				// dnd_to_min
			300,			// flash_hook_time
			30,				// flash_hook_time_min
		},
#if (VOIP_CH_NUM > 1)
		// other ports
#endif
	}
};

#if CONFIG_RTK_VOIP_PACKAGE_867X
voipCfgParam_t VoipEntry;
#endif

int flash_voip_default(voipCfgParam_t *pVoIPCfg)
{
	int i, j;

	memcpy(pVoIPCfg, &voipCfgParamDefault, sizeof(voipCfgParam_t));
	for (i=0; i<VOIP_PORTS; i++)
	{
		if (i > 0)
	{
		memcpy(&pVoIPCfg->ports[i], &pVoIPCfg->ports[0], sizeof(pVoIPCfg->ports[0]));
		pVoIPCfg->ports[i].sip_port = DEF_SIP_LOCAL_PORT + i;
		pVoIPCfg->ports[i].media_port = DEF_RTP_PORT + i * 4;
		pVoIPCfg->ports[i].T38_port = DEF_T38_PORT + i * 2;
	}

		for (j=1; j<MAX_PROXY; j++)
		{
			memcpy(&pVoIPCfg->ports[i].proxies[j], &pVoIPCfg->ports[0].proxies[0], 
				sizeof(pVoIPCfg->ports[0].proxies[0]));
		}
	}

	return 0;
}

int flash_voip_check(voipCfgParam_t *pVoIPCfg)
{
	int i;

	if ((pVoIPCfg->signature != VOIP_FLASH_SIGNATURE) ||
		(pVoIPCfg->version != VOIP_FLASH_VER))
	{
		// do voip flash default
		fprintf(stderr, "==> voip flash default\n");
		flash_voip_default(pVoIPCfg);
		return 1;
	}
	else if (pVoIPCfg->feature != VOIP_FLASH_FEATURE)
	{
		// start check feature in the same flash version

		if (!RTK_VOIP_PLATFORM_CHECK(pVoIPCfg->feature))
		{
			// do nothing
		}

		if (!RTK_VOIP_ROUTER_CHECK(pVoIPCfg->feature))
		{	
			// do nothing
		}

		if (!RTK_VOIP_DAA_CHECK(pVoIPCfg->feature))
		{
			// do nothing
		}

		if (!RTK_VOIP_SLIC_CHECK(pVoIPCfg->feature))
		{
			int n;

			fprintf(stderr, "==> voip ports default\n");
			// if slic number in flash < current, default new ports in flash
			n = RTK_VOIP_SLIC_NUM(pVoIPCfg->feature);
			for (i=n; i<VOIP_PORTS; i++)
			{
				memcpy(&pVoIPCfg->ports[i], &voipCfgParamDefault.ports[0], sizeof(pVoIPCfg->ports[0]));
				pVoIPCfg->ports[i].sip_port = DEF_SIP_LOCAL_PORT + i;
				pVoIPCfg->ports[i].media_port = DEF_RTP_PORT + i * 4;
				pVoIPCfg->ports[i].T38_port = DEF_T38_PORT + i * 2;
			}
		}
		else if (!RTK_VOIP_CODEC_CHECK(pVoIPCfg->feature))
		{
			// do voip codec flash default
			fprintf(stderr, "==> voip codec flash default\n");
			for (i=0; i<VOIP_PORTS; i++)
			{
				memcpy(&pVoIPCfg->ports[i].frame_size,
					&voipCfgParamDefault.ports[0].frame_size,
					sizeof(pVoIPCfg->ports[i].frame_size) +
					sizeof(pVoIPCfg->ports[i].precedence)
					);
			}
		}

		// write current feature to flash
		memcpy(&pVoIPCfg->feature, &voipCfgParamDefault.feature,
			sizeof(pVoIPCfg->feature));

		return 1;
	}

	return 0;
}

int voip_flash_set(voipCfgParam_t *cfg)
{
	voipCfgAll_t cfg_all;

	memset(&cfg_all, 0, sizeof(cfg_all));
	memcpy(&cfg_all.current_setting, cfg, sizeof(voipCfgParam_t));
	cfg_all.mode |= VOIP_CURRENT_SETTING;
	return voip_flash_write(&cfg_all);
}

#if CONFIG_RTK_VOIP_PACKAGE_865X

int voip_flash_read(voipCfgAll_t *cfg_all, int mode)
{
	if ((mode & VOIP_CURRENT_SETTING) == 0)
		return -1;

	memset(cfg_all, 0, sizeof(*cfg_all));
	cfg_all->mode = VOIP_CURRENT_SETTING;

	if (cfgmgr_read(CFGMGR_TABID_VOIP, (void*) &cfg_all->current_setting, 
		sizeof(voipCfgParam_t)))
	{
		fprintf(stderr, "sysInit: cfgmgr read voipCfgParam fail\n");
		/* take proper actions */
		return -1;
	}

	return 0;
}

int voip_flash_write(voipCfgAll_t *cfg_all)
{
	if ((cfg_all->mode & VOIP_CURRENT_SETTING) == 0)
		return -1;

	cfgmgr_write(CFGMGR_TABID_VOIP, (void*) &cfg_all->current_setting, 
		sizeof(voipCfgParam_t));
	cfgmgr_task();
	return 0;
}

int voip_flash_get(voipCfgParam_t **cfg)
{
	extern romeCfgParam_t *pRomeCfgParam;
	
	if (pRomeCfgParam == NULL)
	{
		fprintf(stderr, "Please do voip_flash_read first.\n");
		return -1;
	}

	(*cfg) = &pRomeCfgParam->voipCfgParam;
	return 0;
}

int voip_flash_get_default(voipCfgParam_t **cfg)
{
	return -1;
}

int voip_flash_server_read(voip_flash_share_t *pVoIPShare)
{
	extern romeCfgParam_t *pRomeCfgParam;
	
	if (pRomeCfgParam == NULL)
	{
		fprintf(stderr, "Please do voip_flash_read first.\n");
		return -1;
	}

	memcpy(&pVoIPShare->voip_cfg, &pRomeCfgParam->voipCfgParam, sizeof(pVoIPShare->voip_cfg));
	pVoIPShare->net_cfg.dhcp = (pRomeCfgParam->ifCfgParam[0].connType == IFCFGPARAM_CONNTYPE_PPPOE) || 
		(pRomeCfgParam->ifCfgParam[0].connType == IFCFGPARAM_CONNTYPE_DHCPC);
	pVoIPShare->net_cfg.ip = *(uint32*) (pRomeCfgParam->ifCfgParam[0].ipAddr);
	pVoIPShare->net_cfg.netmask = *(uint32*) (pRomeCfgParam->ifCfgParam[0].ipMask);
	pVoIPShare->net_cfg.gateway = *(uint32*) (pRomeCfgParam->ifCfgParam[0].gwAddr);
	pVoIPShare->net_cfg.dns = *(uint32*) (pRomeCfgParam->ifCfgParam[0].dnsPrimaryAddr);

	// get wan interface name for ioctl
	pVoIPShare->net_cfg.ivr_lan = 0;
	strcpy(pVoIPShare->net_cfg.ivr_interface, "eth0");

	return 0;
}

int voip_flash_server_write(voip_flash_share_t *pVoIPShare)
{
	extern romeCfgParam_t *pRomeCfgParam;
	
	if (pRomeCfgParam == NULL)
	{
		fprintf(stderr, "Please do voip_flash_read first.\n");
		return -1;
	}

	memcpy(&pRomeCfgParam->voipCfgParam, &pVoIPShare->voip_cfg, sizeof(pRomeCfgParam->voipCfgParam));
	pRomeCfgParam->ifCfgParam[0].connType = (pVoIPShare->net_cfg.dhcp == NET_DHCP_DISABLED) ? 
		IFCFGPARAM_CONNTYPE_STATIC : IFCFGPARAM_CONNTYPE_DHCPC;
	*(uint32*)(pRomeCfgParam->ifCfgParam[0].ipAddr) = pVoIPShare->net_cfg.ip;
	*(uint32*)(pRomeCfgParam->ifCfgParam[0].ipMask) = pVoIPShare->net_cfg.netmask;
	*(uint32*)(pRomeCfgParam->ifCfgParam[0].gwAddr) = pVoIPShare->net_cfg.gateway;
	*(uint32*)(pRomeCfgParam->ifCfgParam[0].dnsPrimaryAddr) = pVoIPShare->net_cfg.dns;
	return 0;
}

#elif CONFIG_RTK_VOIP_PACKAGE_867X

int voip_flash_read(voipCfgAll_t *cfg_all, int mode)
{
	int totalEntry = 0;

	if( NULL == cfg_all){
		fprintf(stderr, "voip configure is NULL.\n");
		return -1;
	}

	if ((mode & VOIP_CURRENT_SETTING) == 0)
		return -1;

	memset(cfg_all, 0, sizeof(*cfg_all));
	cfg_all->mode = VOIP_CURRENT_SETTING;

	if(mib_load(CURRENT_SETTING, CONFIG_MIB_CHAIN)){	//load flash to memory
		totalEntry = mib_chain_total(MIB_VOIP_CFG_TBL);		//count the total VOIP records in memory
		if(0 == totalEntry){	//no entry in flash
			if(mib_chain_add(MIB_VOIP_CFG_TBL, (unsigned char*) &cfg_all->current_setting)){ 
				if(mib_update(CURRENT_SETTING,CONFIG_MIB_CHAIN)){ //update to flash-add voip config
					fprintf(stdout, "adding voip config to flash.\n");				
					return 0;
				}else{
					fprintf(stderr, "fail to update voip config to flash.\n");
					return -1;
				}
			}else{
				fprintf(stderr, "fail to add voip config to memory.\n");
				return -1;
			}
		}else{	//already exists in flash
			if(mib_chain_get(MIB_VOIP_CFG_TBL, 0, (void*) &cfg_all->current_setting)){	//get memory voip config
				return 0;
			}
			else{
				fprintf(stderr, "get voip config fail.\n");
				return -1;
			}
		}
	}else{
		fprintf(stderr, "Load mib chain fail.\n");
		return -1;
	}

	return 0;
}

int voip_flash_write(voipCfgAll_t *cfg_all)
{
	if( NULL == cfg_all){
		fprintf(stderr, "voip configure is NULL.\n");
		return -1;
	}
	
	if ((cfg_all->mode & VOIP_CURRENT_SETTING) == 0)
		return -1;

	mib_backup(CONFIG_MIB_CHAIN);	// backup current MIB in RAM
	if(mib_chain_update(MIB_VOIP_CFG_TBL, (void *) &cfg_all->current_setting, 0)){
		if(mib_update(CURRENT_SETTING,CONFIG_MIB_CHAIN)){
			return 0;
		}else{
			fprintf(stderr, "update voip flash fail.\n");
			return -1;
		}
	}else{
		fprintf(stderr, "update voip config to memory fail.\n");
		return -1;
	}
	mib_restore(CONFIG_MIB_CHAIN); //write back to memory
	return 0;
}

int voip_flash_get(voipCfgParam_t **ppVoIPCfg)
{
	unsigned int totalEntry = 0;
	
	totalEntry = mib_chain_total(MIB_VOIP_CFG_TBL);
	if( totalEntry > 0 ){
		if(mib_chain_get(MIB_VOIP_CFG_TBL, 0, (void*)&VoipEntry)){
			*ppVoIPCfg = &VoipEntry;
			return 0;
		}else{
			fprintf(stderr, "read voip config fail.\n");
		}
	}else{
		fprintf(stderr, "flash do no have voip configuration.\n");
		return -1;
	}
	return -1;
}

int voip_flash_get_default(voipCfgParam_t **cfg)
{
	return -1;
}

int voip_flash_server_read(voip_flash_share_t *pVoIPShare)
{
	/*unsigned char dhcp;
	unsigned char ipAddr[IP_ADDR_LEN];
	unsigned char subnetMask[IP_ADDR_LEN];
	unsigned char defaultGateway[IP_ADDR_LEN];
	unsigned char dns1[IP_ADDR_LEN];*/
	unsigned int totalEntry = 0;

	printf("++++++The size of voipConfig %d\n", sizeof(voipCfgParam_t));
	totalEntry = mib_chain_total(MIB_VOIP_CFG_TBL);
	if( totalEntry > 0 ){
		if(mib_chain_get(MIB_VOIP_CFG_TBL, 0, (void*)&VoipEntry)){
			memcpy(&pVoIPShare->voip_cfg, &VoipEntry, sizeof(VoipEntry));
			return 0;
		}else{
			fprintf(stderr, "read voip config fail.\n");
			return -1;
		}
	}else{
		fprintf(stderr, "flash do no have voip configuration, add now....\n");
		if(mib_chain_add(MIB_VOIP_CFG_TBL, (unsigned char*)&VoipEntry)){ //add to the voip setting to memory
			flash_voip_default(&VoipEntry); // set default configuration
			if(mib_chain_update(MIB_VOIP_CFG_TBL, (void *)&VoipEntry, 0)){
				memcpy(&pVoIPShare->voip_cfg, &VoipEntry, sizeof(VoipEntry));
				return 0;
			}else{
				fprintf(stderr,"update voip configuration to memory fail!\n");
				return -1;
			}
		}else{
			fprintf(stderr, "fail to add voip config to memory.\n");
			return -1;
		}
		return -1;
	}
	
	return -1;
}

int voip_flash_server_write(voip_flash_share_t *pVoIPShare)
{
	unsigned int totalEntry = 0;
	
	totalEntry = mib_chain_total(MIB_VOIP_CFG_TBL);
	if( totalEntry > 0 ){
		if(mib_chain_get(MIB_VOIP_CFG_TBL, 0, (void*)&VoipEntry)){
			memcpy(&VoipEntry, &pVoIPShare->voip_cfg, sizeof(VoipEntry));
			return 0;
		}else{
			fprintf(stderr, "read voip config fail.\n");
		}
	}else{
		fprintf(stderr, "flash do no have voip configuration.\n");
		return -1;
	}
	
	return -1;
}

#else // CONFIG_RTK_VOIP_PACKAGE_867X

int voip_flash_read(voipCfgAll_t *cfg_all, int mode)
{
	int (*mib_init)(void);

	mib_init = (pMib == NULL) ? apmib_init : apmib_reinit;
	if (mib_init() == 0)
	{
		fprintf(stderr, "Init MIB failed! size of MIB = %x\n", sizeof(*pMib));
		return -1;
	}

	memset(cfg_all, 0, sizeof(*cfg_all));
	if (mode & VOIP_CURRENT_SETTING)
	{
		memcpy(&cfg_all->current_setting, &pMib->voipCfgParam, sizeof(voipCfgParam_t));
		cfg_all->mode |= VOIP_CURRENT_SETTING;
	}

	if (mode & VOIP_DEFAULT_SETTING)
	{
		memcpy(&cfg_all->default_setting, &pMibDef->voipCfgParam, sizeof(voipCfgParam_t));
		cfg_all->mode |= VOIP_DEFAULT_SETTING;
	}

	return 0;
}

int voip_flash_write(voipCfgAll_t *cfg_all)
{
	int err = 0;

	if (cfg_all->mode == VOIP_CURRENT_SETTING)
	{
		// update current setting only (WEB)
		memcpy(&pMib->voipCfgParam, &cfg_all->current_setting, sizeof(voipCfgParam_t));
		return apmib_update(CURRENT_SETTING) ? 0 : -1;
	}
	
	if (cfg_all->mode & VOIP_CURRENT_SETTING)
	{
		memcpy(&pMib->voipCfgParam, &cfg_all->current_setting, sizeof(voipCfgParam_t));
		err = apmib_updateFlash(CURRENT_SETTING, (char *) pMib, sizeof(APMIB_T), 2, CURRENT_SETTING_VER) ? 0 : -1;
	}

	if ((err == 0) && (cfg_all->mode & VOIP_DEFAULT_SETTING))
	{
		memcpy(&pMibDef->voipCfgParam, &cfg_all->default_setting, sizeof(voipCfgParam_t));
		err = apmib_updateFlash(DEFAULT_SETTING, (char *) pMibDef, sizeof(APMIB_T), 2, DEFAULT_SETTING_VER) ? 0 : -1;
	}

	return err;
}

int voip_flash_get(voipCfgParam_t **ppVoIPCfg)
{
	if (pMib == NULL)
	{
		fprintf(stderr, "Please do voip_flash_read first.\n");
		return -1;
	}

	(*ppVoIPCfg) = &pMib->voipCfgParam;
	return 0;
}

int voip_flash_get_default(voipCfgParam_t **ppVoIPCfg)
{
	if (pMibDef == NULL)
	{
		fprintf(stderr, "Please do voip_flash_read first.\n");
		return -1;
	}

	(*ppVoIPCfg) = &pMibDef->voipCfgParam;
	return 0;
}

int voip_flash_server_read(voip_flash_share_t *pVoIPShare)
{
	if (pMib == NULL)
	{
		fprintf(stderr, "Please do voip_flash_read first.\n");
		return -1;
	}

	memcpy(&pVoIPShare->voip_cfg, &pMib->voipCfgParam, sizeof(pVoIPShare->voip_cfg));

#ifdef HOME_GATEWAY
	if (pMib->opMode != BRIDGE_MODE)
	{
		pVoIPShare->net_cfg.ivr_lan = 0;
		pVoIPShare->net_cfg.dhcp = (pMib->wanDhcp == DHCP_CLIENT) || (pMib->wanDhcp == PPPOE);
		memcpy(&pVoIPShare->net_cfg.ip, &pMib->wanIpAddr, sizeof(pVoIPShare->net_cfg.ip));
		memcpy(&pVoIPShare->net_cfg.netmask, &pMib->wanSubnetMask, sizeof(pVoIPShare->net_cfg.netmask));
		memcpy(&pVoIPShare->net_cfg.gateway, &pMib->wanDefaultGateway, sizeof(pVoIPShare->net_cfg.gateway));
		memcpy(&pVoIPShare->net_cfg.dns, &pMib->dns1, sizeof(pVoIPShare->net_cfg.dns));
	}
	else
#endif
	{
		pVoIPShare->net_cfg.ivr_lan = 1;
		pVoIPShare->net_cfg.dhcp = (pMib->dhcp == DHCP_CLIENT) || (pMib->dhcp == PPPOE);
		memcpy(&pVoIPShare->net_cfg.ip, &pMib->ipAddr, sizeof(pVoIPShare->net_cfg.ip));
		memcpy(&pVoIPShare->net_cfg.netmask, &pMib->subnetMask, sizeof(pVoIPShare->net_cfg.netmask));
		memcpy(&pVoIPShare->net_cfg.gateway, &pMib->defaultGateway, sizeof(pVoIPShare->net_cfg.gateway));
		memcpy(&pVoIPShare->net_cfg.dns, &pMib->dns1, sizeof(pVoIPShare->net_cfg.dns));
	}
	
	// get wan interface name for ioctl
#ifdef HOME_GATEWAY
	switch (pMib->opMode)
	{
	case BRIDGE_MODE:
		strcpy(pVoIPShare->net_cfg.ivr_interface, "br0");
		break;
	case WISP_MODE:
		strcpy(pVoIPShare->net_cfg.ivr_interface, "wlan0");
		break;
	default:
		strcpy(pVoIPShare->net_cfg.ivr_interface, "eth1");
		break;
	}
#else
	if (pMib->opMode == WISP_MODE)
		strcpy(pVoIPShare->net_cfg.ivr_interface, "wlan0");
	else
		strcpy(pVoIPShare->net_cfg.ivr_interface, "br0");
#endif

	return 0;
}

int voip_flash_server_write(voip_flash_share_t *pVoIPShare)
{
	if (pMib == NULL)
	{
		fprintf(stderr, "Please do voip_flash_read first.\n");
		return -1;
	}

	memcpy(&pMib->voipCfgParam, &pVoIPShare->voip_cfg, sizeof(pMib->voipCfgParam));

#ifdef HOME_GATEWAY
	if (pMib->opMode != BRIDGE_MODE)
	{
		pMib->wanDhcp = (pVoIPShare->net_cfg.dhcp == NET_DHCP_DISABLED) ? DHCP_DISABLED : DHCP_CLIENT;
		memcpy(&pMib->wanIpAddr, &pVoIPShare->net_cfg.ip, sizeof(pMib->wanIpAddr));
		memcpy(&pMib->wanSubnetMask, &pVoIPShare->net_cfg.netmask, sizeof(pMib->wanSubnetMask));
		memcpy(&pMib->wanDefaultGateway, &pVoIPShare->net_cfg.gateway, sizeof(pMib->wanDefaultGateway));
		memcpy(&pMib->dns1, &pVoIPShare->net_cfg.dns, sizeof(pMib->dns1));
	}
	else
#endif
	{
		pMib->dhcp = (pVoIPShare->net_cfg.dhcp == NET_DHCP_DISABLED) ? DHCP_DISABLED : DHCP_CLIENT;
		memcpy(&pMib->ipAddr, &pVoIPShare->net_cfg.ip, sizeof(pMib->ipAddr));
		memcpy(&pMib->subnetMask, &pVoIPShare->net_cfg.netmask, sizeof(pMib->subnetMask));
		memcpy(&pMib->defaultGateway, &pVoIPShare->net_cfg.gateway, sizeof(pMib->defaultGateway));
		memcpy(&pMib->dns1, &pVoIPShare->net_cfg.dns, sizeof(pMib->dns1));
	}

	return 0;
}

#endif // CONFIG_RTK_VOIP_PACKAGE_867X

int flash_voip_cmd(int param_cnt, char *param_var[])
{
	voipCfgAll_t cfg_all;

	if (param_cnt == 0)
	{
		// dump voip flash
		voip_flash_read(&cfg_all, VOIP_ALL_SETTING);
		if (cfg_all.mode & VOIP_CURRENT_SETTING)
			voip_mibtbl_write(&cfg_all.current_setting, fileno(stdout), VOIP_CURRENT_SETTING);
		if (cfg_all.mode & VOIP_DEFAULT_SETTING)
			voip_mibtbl_write(&cfg_all.default_setting, fileno(stdout), VOIP_DEFAULT_SETTING);
		return 0;
	}
	else if (param_cnt == 1)
	{
		if (strcmp(param_var[0], "check") == 0)
		{
			int size;
			int need_default;

			if (voip_mibtbl_check(&size) != 0)
			{
				fprintf(stderr, "VoIP Command Error: check mib table failed!\n");
				return -1;
			}

			fprintf(stderr, "VoIP MIB Info: mib size = %d, size = %d\n", size, sizeof(cfg_all.current_setting));

			// flash check and update
			voip_flash_read(&cfg_all, VOIP_ALL_SETTING);
			need_default = 0;
			if (cfg_all.mode & VOIP_CURRENT_SETTING)
				need_default = flash_voip_check(&cfg_all.current_setting);
			if (cfg_all.mode & VOIP_DEFAULT_SETTING)
				need_default |= flash_voip_check(&cfg_all.default_setting);
			if (need_default)
				voip_flash_write(&cfg_all);

			return 0;
		}
		else if (strcmp(param_var[0], "default") == 0)
		{
			// do voip flash default
			fprintf(stderr, "==> voip flash default... ");

			voip_flash_read(&cfg_all, VOIP_ALL_SETTING);
			if (cfg_all.mode & VOIP_CURRENT_SETTING)
				flash_voip_default(&cfg_all.current_setting);
			if (cfg_all.mode & VOIP_DEFAULT_SETTING)
				flash_voip_default(&cfg_all.default_setting);
			voip_flash_write(&cfg_all);

			fprintf(stderr, "done.\n");
			return 0;
		}
	}
	else if (param_cnt == 2)
	{
		if (voip_flash_read(&cfg_all, VOIP_ALL_SETTING) != 0)
		{
			fprintf(stderr, "VoIP Command Error: read flash failed\n");
			return -1;
		}

		if (strcmp(param_var[0], "get") == 0)
		{
			return voip_mibtbl_get(param_var[1], &cfg_all, fileno(stdout));
		}
		else if (strcmp(param_var[0], "-in") == 0) // import
		{
			int status;
			
			status = flash_voip_import_from_file(&cfg_all, param_var[1]);
			if (status != 0)
				return status;

			return voip_flash_write(&cfg_all);
		}
		else if (strcmp(param_var[0], "-ot") == 0) // export to text file
		{
			return flash_voip_export_to_file(&cfg_all, param_var[1], 0);
		}
		else if (strcmp(param_var[0], "-ob") == 0) // export to config file
		{
			return flash_voip_export_to_file(&cfg_all, param_var[1], 1);
		}
	}
	else if (param_cnt == 3)
	{
		if (strcmp(param_var[0], "set") == 0)
		{
			if ((voip_flash_read(&cfg_all, VOIP_ALL_SETTING) == 0) &&
				(voip_mibtbl_set(param_var[1], param_var[2], &cfg_all, fileno(stdout)) == 0))
				return voip_flash_write(&cfg_all);
		}
	}
	else if(param_cnt == 4)	// convert mode
	{
		char *buf, *text;
		int buf_len, text_len;

		if (strcmp(param_var[0], "-in") != 0)
			return -1;

		if (strcmp(param_var[2], "-ot") != 0 &&
			strcmp(param_var[2], "-ob") != 0)
			return -1;

		// read file
		if (flash_voip_read_file(param_var[1], &buf, &buf_len) != 0)
		{
			fprintf(stderr, "VoIP MIB Convert Error: read failed\n");
			return -1;
		}

		// decode to text data
		if (flash_voip_decode(buf, buf_len, &text, &text_len) != 0)
		{
			fprintf(stderr, "VoIP MIB Convert Error: decode failed\n");
			free(buf);
			return -1;
		}

		free(buf); // free unused buffer

		// import text for checking
		memset(&cfg_all, 0, sizeof(cfg_all));
		if (flash_voip_import_text(&cfg_all, text, text_len) != 0)
		{
			fprintf(stderr, "VoIP MIB Convert Error: import text failed\n");
			free(text);
			return -1;
		}

		// don't need check feature here
		// it will check different feature on importing to flash
		if ((cfg_all.mode & VOIP_CURRENT_SETTING) &&
			(flash_voip_import_check(&cfg_all.current_setting) < 0))
		{
			fprintf(stderr, "VoIP MIB Convert Error: import check failed\n");
			free(text);
			return -1;
		}

		if ((cfg_all.mode & VOIP_DEFAULT_SETTING) &&
			(flash_voip_import_check(&cfg_all.default_setting) < 0))
		{
			fprintf(stderr, "VoIP Deafult MIB Convert Error: import check failed\n");
			free(text);
			return -1;
		}

		if (strcmp(param_var[2], "-ot") == 0)
		{
			// output to text file directly
			if (flash_voip_write_file(param_var[3], text, text_len) != 0)
			{
				fprintf(stderr, "VoIP MIB Convert Error: write failed\n");
				free(text);
				return -1;
			}

			free(text);
			return 0;
		}
		
		// config file
		// 1. encode to config format
		if (flash_voip_encode(text, text_len, &buf, &buf_len) != 0)
		{
			fprintf(stderr, "VoIP MIB Convert Error: encode failed\n");
			free(text);
			return -1;
		}

		free(text); // free unused buffer

		// 2. output to config file
		if (flash_voip_write_file(param_var[3], buf, buf_len) != 0)
		{
			fprintf(stderr, "VoIP MIB Convert Error: write failed\n");
			free(buf);
			return -1;
		}
		
		free(buf);
		return 0;
	}

	return -1;
}

