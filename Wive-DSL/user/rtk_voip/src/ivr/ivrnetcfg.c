#include <sys/ioctl.h>
#include <sys/socket.h>
#include <stdarg.h>
#include <stdio.h>
#include <resolv.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "voip_manager.h"
#include "ivrnetcfg.h"

#ifdef NETWORK_SETTINGS_IN_RAM
#define NET_PARAM_SET_MASK_DHCP		0x0001L	/* bit 0 */
#define NET_PARAM_SET_MASK_IP		0x0002L	/* bit 1 */
#define NET_PARAM_SET_MASK_MASK		0x0004L	/* bit 2 */
#define NET_PARAM_SET_MASK_GATEWAY	0x0008L	/* bit 3 */
#define NET_PARAM_SET_MASK_DNS		0x0010L	/* bit 4 */

typedef struct ivr_net_param_s {
	unsigned long	set;
	int 			dhcp;
	in_addr_t		ip;
	in_addr_t		mask;
	in_addr_t		gateway;
	in_addr_t		dns;
} ivr_net_param_t;

static ivr_net_param_t ivrRamNetParam;
#endif /* NETWORK_SETTINGS_IN_RAM */

#if 0
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
	???
#else
 #if BUILD_MODE == 1	/* AP */
const unsigned char pHttpHeaderTcp[] = { "GET /goform/formTcpipSetup?" };
 #elif BUILD_MODE == 2	/* GW */
const unsigned char pHttpHeaderTcp[] = { "GET /goform/formWanTcpipSetup?" }; 
const unsigned char pHttpHeaderTcpLan[] = { "GET /goform/formTcpipSetup?" };
 #else
   ???
 #endif
#endif
#else
const char pHttpHeaderTcpWan[] = { "GET /goform/formWanTcpipSetup?" }; 
const char pHttpHeaderTcpLan[] = { "GET /goform/formTcpipSetup?" };
#endif

const char pHttpHeader[] = { "GET /goform/voip_ivrreq_set?voipPort=%d" };
const char pHttpFooter[] = { " HTTP/1.1\n\n" };
const char pszCheckboxOn[] = { "on" };
const char pszCheckboxOff[] = { "off" };

extern voip_flash_share_t *g_pVoIPShare;
extern int g_mapSupportedCodec[_CODEC_MAX];

#ifdef NETWORK_SETTINGS_IN_RAM
void IvrClearNetworkSettingsInRam( void )
{
	ivrRamNetParam.set = 0;
}

static void ShowIvrNetworkSettingsInRam( void )
{
	printf( "IVR: current settings in RAM...\n" );

	if( ivrRamNetParam.set == 0 ) {
		printf( "\tNothing\n" );
	} else if( ivrRamNetParam.set & NET_PARAM_SET_MASK_DHCP ) {
		printf( "\tDHCP\n" );
	} else {
		if( ivrRamNetParam.set & NET_PARAM_SET_MASK_IP )
			printf( "\tIP:%d.%d.%d.%d\n", 
						*( ( unsigned char * )&ivrRamNetParam.ip + 0 ),
						*( ( unsigned char * )&ivrRamNetParam.ip + 1 ),
						*( ( unsigned char * )&ivrRamNetParam.ip + 2 ),
						*( ( unsigned char * )&ivrRamNetParam.ip + 3 ) );

		if( ivrRamNetParam.set & NET_PARAM_SET_MASK_MASK )
			printf( "\tMask:%d.%d.%d.%d\n", 
						*( ( unsigned char * )&ivrRamNetParam.mask + 0 ),
						*( ( unsigned char * )&ivrRamNetParam.mask + 1 ),
						*( ( unsigned char * )&ivrRamNetParam.mask + 2 ),
						*( ( unsigned char * )&ivrRamNetParam.mask + 3 ) );
		
		if( ivrRamNetParam.set & NET_PARAM_SET_MASK_GATEWAY )
			printf( "\tGateway:%d.%d.%d.%d\n", 
						*( ( unsigned char * )&ivrRamNetParam.gateway + 0 ),
						*( ( unsigned char * )&ivrRamNetParam.gateway + 1 ),
						*( ( unsigned char * )&ivrRamNetParam.gateway + 2 ),
						*( ( unsigned char * )&ivrRamNetParam.gateway + 3 ) );
		
		if( ivrRamNetParam.set & NET_PARAM_SET_MASK_DNS )
			printf( "\tDNS:%d.%d.%d.%d\n", 
						*( ( unsigned char * )&ivrRamNetParam.dns + 0 ),
						*( ( unsigned char * )&ivrRamNetParam.dns + 1 ),
						*( ( unsigned char * )&ivrRamNetParam.dns + 2 ),
						*( ( unsigned char * )&ivrRamNetParam.dns + 3 ) );
	}
}
#endif /* NETWORK_SETTINGS_IN_RAM */

static int IvrSetRawHttpToWebServer( unsigned char *pHttpContent )
{
	int sockfd;
	struct sockaddr_in dest;
	int ret = 1;

	if( ( sockfd = socket( PF_INET, SOCK_STREAM, 0 ) ) == -1 )
		return 0;
	
	bzero(&dest, sizeof(dest));
	dest.sin_family = PF_INET;
	dest.sin_port = htons(80);
	inet_aton("localhost", &dest.sin_addr);

	if( connect(sockfd, ( struct sockaddr * )&dest, sizeof(dest)) == -1 ) {
		ret = 0;
		goto label_close_socket;
	}

	if( send(sockfd, pHttpContent, strlen( pHttpContent ), 0) == -1 )
		ret = 0;
	
	printf( "Ivr req: %s\n", pHttpContent );
	
label_close_socket:
	close( sockfd );

	return ret;	
}

#if 0
#if BUILD_MODE == 2	/* GW */
static int IvrSetNetCfgToWebServer( int bLan, unsigned char *format, ... )
#else
static int IvrSetNetCfgToWebServer( unsigned char *format, ... )
#endif
#endif

static int IvrSetNetCfgToWebServer( int bLan, unsigned char *format, ... )
{
	unsigned char buffer[ 512 ];
	va_list va_cfg;

#if 0	
	/* header */
#if BUILD_MODE == 2	/* GW */
	if( bLan )
		sprintf( buffer, pHttpHeaderTcpLan );
	else
		sprintf( buffer, pHttpHeaderTcp );
#else
	sprintf( buffer, pHttpHeaderTcp );
#endif
#endif

	if( bLan )
		sprintf( buffer, pHttpHeaderTcpLan );
	else
		sprintf( buffer, pHttpHeaderTcpWan );

	/* parameters */
	va_start( va_cfg, format );
	
	vsprintf( buffer + strlen( buffer ), format, va_cfg );
	
	/* footer */
	strcat( buffer, pHttpFooter );
	
	return IvrSetRawHttpToWebServer( buffer );
}

#ifdef NETWORK_SETTINGS_IN_RAM
static int IvrApplyRamNetCfgToWebServer( int bLan )
{
	int ret = 0;
 	unsigned char buffer[ 512 ];
 	unsigned int pos = 0;
 	unsigned char dhcp;

 	if( bLan ) {	/* LAN */
 		if( ivrRamNetParam.set & NET_PARAM_SET_MASK_DHCP )
 			ret = IvrSetNetCfgToWebServer( 1 /* LAN */, "dhcp=1" );
 		else {
 			if( ivrRamNetParam.set & NET_PARAM_SET_MASK_IP ) {
 				dhcp = GetDhcpValueToSetFixedIP();
 				sprintf( &buffer[ pos ], "lan_ip=%d.%d.%d.%d&dhcp=%d&", 
						*( ( unsigned char * )&ivrRamNetParam.ip + 0 ),
						*( ( unsigned char * )&ivrRamNetParam.ip + 1 ),
						*( ( unsigned char * )&ivrRamNetParam.ip + 2 ),
						*( ( unsigned char * )&ivrRamNetParam.ip + 3 ), 
						dhcp );
 				pos += strlen( &buffer[ pos ] );
 			}
 				
 			if( ivrRamNetParam.set & NET_PARAM_SET_MASK_MASK ) {
 				sprintf( &buffer[ pos ], "lan_mask=%d.%d.%d.%d&", 
 						*( ( unsigned char * )&ivrRamNetParam.mask + 0 ),
						*( ( unsigned char * )&ivrRamNetParam.mask + 1 ),
						*( ( unsigned char * )&ivrRamNetParam.mask + 2 ),
						*( ( unsigned char * )&ivrRamNetParam.mask + 3 ) );
 				pos += strlen( &buffer[ pos ] );
 			}
 			
 			if( ivrRamNetParam.set & NET_PARAM_SET_MASK_GATEWAY ) {
 				sprintf( &buffer[ pos ], "lan_gateway=%d.%d.%d.%d&", 
 						*( ( unsigned char * )&ivrRamNetParam.gateway + 0 ),
						*( ( unsigned char * )&ivrRamNetParam.gateway + 1 ),
						*( ( unsigned char * )&ivrRamNetParam.gateway + 2 ),
						*( ( unsigned char * )&ivrRamNetParam.gateway + 3 ) );
 				pos += strlen( &buffer[ pos ] );
 			}
 			
 			if( ivrRamNetParam.set & NET_PARAM_SET_MASK_DNS ) {
 				sprintf( &buffer[ pos ], "dns1=%d.%d.%d.%d&", 
 						*( ( unsigned char * )&ivrRamNetParam.dns + 0 ),
						*( ( unsigned char * )&ivrRamNetParam.dns + 1 ),
						*( ( unsigned char * )&ivrRamNetParam.dns + 2 ),
						*( ( unsigned char * )&ivrRamNetParam.dns + 3 ) );
 				pos += strlen( &buffer[ pos ] );
 			}
 			
 			if( pos ) {
 				buffer[ -- pos ] = '\x0';	/* remove tail '&' */
 				ret = IvrSetNetCfgToWebServer( 1 /* LAN */, buffer );
 			}
 		}
 	} else {		/* WAN */
 		if( ivrRamNetParam.set & NET_PARAM_SET_MASK_DHCP )
 			ret = IvrSetNetCfgToWebServer( 0 /* WAN */, "wanType=autoIp" );
 		else {
 			if( ivrRamNetParam.set & NET_PARAM_SET_MASK_IP ) {
 				sprintf( &buffer[ pos ], "wan_ip=%d.%d.%d.%d&wanType=fixedIp&", 
						*( ( unsigned char * )&ivrRamNetParam.ip + 0 ),
						*( ( unsigned char * )&ivrRamNetParam.ip + 1 ),
						*( ( unsigned char * )&ivrRamNetParam.ip + 2 ),
						*( ( unsigned char * )&ivrRamNetParam.ip + 3 ) );
 				pos += strlen( &buffer[ pos ] );
 			}
 				
 			if( ivrRamNetParam.set & NET_PARAM_SET_MASK_MASK ) {
 				sprintf( &buffer[ pos ], "wan_mask=%d.%d.%d.%d&", 
 						*( ( unsigned char * )&ivrRamNetParam.mask + 0 ),
						*( ( unsigned char * )&ivrRamNetParam.mask + 1 ),
						*( ( unsigned char * )&ivrRamNetParam.mask + 2 ),
						*( ( unsigned char * )&ivrRamNetParam.mask + 3 ) );
 				pos += strlen( &buffer[ pos ] );
 			}
 			
 			if( ivrRamNetParam.set & NET_PARAM_SET_MASK_GATEWAY ) {
 				sprintf( &buffer[ pos ], "wan_gateway=%d.%d.%d.%d&", 
 						*( ( unsigned char * )&ivrRamNetParam.gateway + 0 ),
						*( ( unsigned char * )&ivrRamNetParam.gateway + 1 ),
						*( ( unsigned char * )&ivrRamNetParam.gateway + 2 ),
						*( ( unsigned char * )&ivrRamNetParam.gateway + 3 ) );
 				pos += strlen( &buffer[ pos ] );
 			}
 			
 			if( ivrRamNetParam.set & NET_PARAM_SET_MASK_DNS ) {
 				sprintf( &buffer[ pos ], "dnsMode=dnsManual&dns1=%d.%d.%d.%d&", 
 						*( ( unsigned char * )&ivrRamNetParam.dns + 0 ),
						*( ( unsigned char * )&ivrRamNetParam.dns + 1 ),
						*( ( unsigned char * )&ivrRamNetParam.dns + 2 ),
						*( ( unsigned char * )&ivrRamNetParam.dns + 3 ) );
 				pos += strlen( &buffer[ pos ] );
 			}
 
  			if( pos ) {
  				buffer[ -- pos ] = '\x0';	/* remove tail '&' */
 				ret = IvrSetNetCfgToWebServer( 0 /* WAN */, buffer );
 			}			
 		}
 	}
 	
 	return ret;
}
#endif /* NETWORK_SETTINGS_IN_RAM */

static int IvrSetVoipCfgToWebServer( unsigned int chid, unsigned char *format, ... )
{
	unsigned char buffer[ 512 ];
	va_list va_cfg;
	
	/* header */
	sprintf( buffer, pHttpHeader, chid );
	
	/* parameters */
	va_start( va_cfg, format );
	
	vsprintf( buffer + strlen( buffer ), format, va_cfg );
	
	/* footer */
	strcat( buffer, pHttpFooter );
	
	return IvrSetRawHttpToWebServer( buffer );
}

/* ************** Change Network Settings ************** */
int IvrSetNetworkDhcpClientCfg( void )
{
	/* ivr command: IVR_COMMAND_DHCP_CLIENT */
#ifdef NETWORK_SETTINGS_IN_RAM
	ivrRamNetParam.set |= NET_PARAM_SET_MASK_DHCP;
	ivrRamNetParam.dhcp = 1;
	
	ShowIvrNetworkSettingsInRam();
	
	return 1;
#else	
 ???
 #if defined (CONFIG_RTK_VOIP_DRIVERS_PCM8651) || defined (CONFIG_RTK_VOIP_DRIVERS_PCM865xC)
	???
 #else
  #if BUILD_MODE == 1	/* AP */
	return IvrSetNetCfgToWebServer( "dhcp=1" );
  #elif BUILD_MODE == 2	/* GW */
 	OPMODE_T opmode;
 	
 	opmode = GetGatewayOperationMode();
 
 	if( opmode == BRIDGE_MODE )	/* bridge */
 		return IvrSetNetCfgToWebServer( 1 /* LAN */, "dhcp=1" );
 	else						/* gateway, WISP */
 		return IvrSetNetCfgToWebServer( 0 /* WAN */, "wanType=autoIp" );
  #else
   ???
  #endif /* BUILD_MODE */
 #endif /*#if defined (CONFIG_RTK_VOIP_DRIVERS_PCM8651) || defined (CONFIG_RTK_VOIP_DRIVERS_PCM865xC)*/
#endif /* NETWORK_SETTINGS_IN_RAM */
}

int IvrSetNetworkFixedIpCfg( const unsigned char *pszFixedIP )
{
	/* ivr command: IVR_COMMAND_SET_FIXED_IP */
#ifdef NETWORK_SETTINGS_IN_RAM
	ivrRamNetParam.set &= ~NET_PARAM_SET_MASK_DHCP;
	ivrRamNetParam.set |= NET_PARAM_SET_MASK_IP;
	ivrRamNetParam.ip = inet_addr( pszFixedIP );
	
	ShowIvrNetworkSettingsInRam();
	
	return 1;
#else	
 ???
 #if defined (CONFIG_RTK_VOIP_DRIVERS_PCM8651) || defined (CONFIG_RTK_VOIP_DRIVERS_PCM865xC)
	???
 #else
  #if BUILD_MODE == 1	/* AP */
	unsigned char dhcp;
	
	dhcp = GetDhcpValueToSetFixedIP();

	return IvrSetNetCfgToWebServer( "lan_ip=%s&dhcp=%d", pszFixedIP, dhcp );
  #elif BUILD_MODE == 2	/* GW */
 	OPMODE_T opmode;
	unsigned char dhcp;
	 	
 	opmode = GetGatewayOperationMode();

	if( opmode == BRIDGE_MODE ) {	/* bridge */
		dhcp = GetDhcpValueToSetFixedIP();
		return IvrSetNetCfgToWebServer( 1 /* LAN */, "lan_ip=%s&dhcp=%d", pszFixedIP, dhcp );
	} else							/* gateway, WISP */
 		return IvrSetNetCfgToWebServer( 0 /* WAN */, "wan_ip=%s&wanType=fixedIp", pszFixedIP );
  #else
   ???
  #endif /* BUILD_MODE */
 #endif /* #if defined (CONFIG_RTK_VOIP_DRIVERS_PCM8651) || defined (CONFIG_RTK_VOIP_DRIVERS_PCM865xC)*/
#endif /* NETWORK_SETTINGS_IN_RAM */
}

int IvrSetNetworkNetmaskCfg( const unsigned char *pszNetmask )
{
	/* ivr command: IVR_COMMAND_SET_NETMASK */
#ifdef NETWORK_SETTINGS_IN_RAM
	ivrRamNetParam.set |= NET_PARAM_SET_MASK_MASK;
	ivrRamNetParam.mask = inet_addr( pszNetmask );
	
	ShowIvrNetworkSettingsInRam();

	return 1;
#else	
 ???
 #if defined (CONFIG_RTK_VOIP_DRIVERS_PCM8651) || defined (CONFIG_RTK_VOIP_DRIVERS_PCM865xC)
	???
 #else
  #if BUILD_MODE == 1	/* AP */
	return IvrSetNetCfgToWebServer( "lan_mask=%s", pszNetmask );
  #elif BUILD_MODE == 2	/* GW */
 	OPMODE_T opmode;
 	
 	opmode = GetGatewayOperationMode();
 	
 	if( opmode == BRIDGE_MODE ) 	/* bridge */
 		return IvrSetNetCfgToWebServer( 1 /* LAN */, "lan_mask=%s", pszNetmask );
 	else							/* gateway, WISP */
		return IvrSetNetCfgToWebServer( 0 /* WAN */, "wan_mask=%s", pszNetmask );
  #else
   ???
  #endif /* BUILD_MODE */
 #endif /* #if defined (CONFIG_RTK_VOIP_DRIVERS_PCM8651) || defined (CONFIG_RTK_VOIP_DRIVERS_PCM865xC)*/
#endif /* NETWORK_SETTINGS_IN_RAM */
}

int IvrSetNetworkGatewayCfg( const unsigned char *pszGateway )
{
	/* ivr command: IVR_COMMAND_SET_GATEWAY */
#ifdef NETWORK_SETTINGS_IN_RAM
	ivrRamNetParam.set |= NET_PARAM_SET_MASK_GATEWAY;
	ivrRamNetParam.gateway = inet_addr( pszGateway );
	
	ShowIvrNetworkSettingsInRam();

	return 1;
#else	
 ???
 #if defined (CONFIG_RTK_VOIP_DRIVERS_PCM8651) || defined (CONFIG_RTK_VOIP_DRIVERS_PCM865xC)
	???
 #else
  #if BUILD_MODE == 1	/* AP */
	return IvrSetNetCfgToWebServer( "lan_gateway=%s", pszGateway );
  #elif BUILD_MODE == 2	/* GW */
 	OPMODE_T opmode;
 	
 	opmode = GetGatewayOperationMode();

	if( opmode == BRIDGE_MODE ) 	/* bridge */
		return IvrSetNetCfgToWebServer( 1 /* LAN */, "lan_gateway=%s", pszGateway );		
	else							/* gateway, WISP */
		return IvrSetNetCfgToWebServer( 0 /* WAN */, "wan_gateway=%s", pszGateway );
  #else
   ???
  #endif /* BUILD_MODE */
 #endif /* #if defined (CONFIG_RTK_VOIP_DRIVERS_PCM8651) || defined (CONFIG_RTK_VOIP_DRIVERS_PCM865xC)*/
#endif /* NETWORK_SETTINGS_IN_RAM */
}

int IvrSetNetworkDnsCfg( const unsigned char *pszDNS )
{
	/* ivr command: IVR_COMMAND_SET_DNS0 */
#ifdef NETWORK_SETTINGS_IN_RAM
	ivrRamNetParam.set |= NET_PARAM_SET_MASK_DNS;
	ivrRamNetParam.dns = inet_addr( pszDNS );
	
	ShowIvrNetworkSettingsInRam();

	return 1;
#else
 ???
 #if defined (CONFIG_RTK_VOIP_DRIVERS_PCM8651) || defined (CONFIG_RTK_VOIP_DRIVERS_PCM865xC)
	???
 #else
  #if BUILD_MODE == 1	/* AP */
	return IvrSetNetCfgToWebServer( "dns1=%s", pszDNS );
  #elif BUILD_MODE == 2	/* GW */
 	OPMODE_T opmode;
 	
 	opmode = GetGatewayOperationMode();

	if( opmode == BRIDGE_MODE ) 	/* bridge */
		return IvrSetNetCfgToWebServer( 1 /* LAN */, "dns1=%s", pszDNS );
	else							/* gateway, WISP */
		return IvrSetNetCfgToWebServer( 0 /* WAN */, "dnsMode=dnsManual&dns1=%s", pszDNS );
  #else
   ???
  #endif
 #endif
#endif /* NETWORK_SETTINGS_IN_RAM */
}

/* ************** System Settings ************** */
int IvrSystemSaveAndRebootCfg( void )
{
	/* ivr command: IVR_COMMAND_SAVE_AND_REBOOT */
#ifdef NETWORK_SETTINGS_IN_RAM
 	int ret = 0;

#if 0
 #if BUILD_MODE == 1	/* AP */
 	ret = IvrApplyRamNetCfgToWebServer( 1 /* LAN */ );
 #elif BUILD_MODE == 2	/* GW */
 	OPMODE_T opmode;
 	
 	opmode = GetGatewayOperationMode();
 
 	if( opmode == BRIDGE_MODE )		/* bridge */
 		ret = IvrApplyRamNetCfgToWebServer( 1 /* LAN */ );
 	else							/* gateway, WISP */
 		ret = IvrApplyRamNetCfgToWebServer( 0 /* WAN */ );
 #else
 	???
 #endif 
#endif

 	ret = IvrApplyRamNetCfgToWebServer( g_pVoIPShare->net_cfg.ivr_lan );
 	/* clear settings */
 	IvrClearNetworkSettingsInRam();
 	
 	return ret;
#else
	/* NOTE: We define a novel parameter "saveAndReboot". */
	// TODO: This option is unused.
 #if BUILD_MODE == 1	/* AP */
	return IvrSetNetCfgToWebServer( "&saveAndReboot=1" );
 #elif BUILD_MODE == 2	/* GW */
	return IvrSetNetCfgToWebServer( 0, "&saveAndReboot=1" );
 #else
   ???
 #endif /* BUILD_MODE */
#endif /* NETWORK_SETTINGS_IN_RAM */
}

int IvrSystemResetToDefaultCfg( void )
{
	/* ivr command: IVR_COMMAND_RESET_TO_DEFAULT */
	if ((g_VoIP_Feature & PLATFORM_MASK) == PLATFORM_8186)
	{
		return IvrSetRawHttpToWebServer( 
				"GET /goform/formSaveConfig?reset=1 HTTP/1.1\n\n" );
	}

	return 0;

#if 0
#if defined (CONFIG_RTK_VOIP_DRIVERS_PCM8651) || defined (CONFIG_RTK_VOIP_DRIVERS_PCM865xC)
	???
#else
	return IvrSetRawHttpToWebServer( 
				"GET /goform/formSaveConfig?reset=1 HTTP/1.1\n\n" );
#endif	
#endif
	
#if 0
	/* NOTE: We define a novel parameter "reset2default". */
	
	return IvrSetNetCfgToWebServer( 0, "&reset2default=1" );
#endif
}

/* ************** Change VoIP Settings ************** */	
int IvrSetVoipCodecSequenceCfg( unsigned int chid, unsigned char firstCodec )
{
	/* ivr command: IVR_COMMAND_CODEC_SEQUENCE */
	/* NOTE: We define a novel parameter "first_codec" and "rate". */
	unsigned char rate = 0;
	unsigned int delta;
	
	if( ( g_VoIP_Feature & CODEC_G723_SUPPORT ) || 
		( g_VoIP_Feature & CODEC_iLBC_SUPPORT ) )
	{
		/* firstCode: 0 ~ g_nMaxCodec */
		
		if( g_VoIP_Feature & CODEC_G723_SUPPORT ) {
			if( firstCodec < g_mapSupportedCodec[_CODEC_G723] ) {
				/* do nothing */
			} else if( firstCodec == g_mapSupportedCodec[_CODEC_G723] ) {
				/* G723 6.3k -> do nothing */
			} else if( firstCodec == g_mapSupportedCodec[_CODEC_G723] + 1 ) {
				/* G723 5.3k */
				firstCodec = g_mapSupportedCodec[_CODEC_G723];
				rate = 1;
			} else {
				firstCodec -= 1;
				goto label_do_more_judge;
			}
			
			goto label_set_codec_sequenc_done;
		}

label_do_more_judge:
		if( g_VoIP_Feature & CODEC_iLBC_SUPPORT ) {
			if( firstCodec < g_mapSupportedCodec[_CODEC_ILBC] ) {
				/* do nothing */
			} else if( firstCodec == g_mapSupportedCodec[_CODEC_ILBC] ) {
				/* iLBC 30ms -> do nothing */
			} else if( firstCodec == g_mapSupportedCodec[_CODEC_ILBC] + 1 ) {
				/* iLBC 20ms */
				firstCodec = g_mapSupportedCodec[_CODEC_ILBC];
				rate = 1;
			} else {
				firstCodec -= 1;
			}			
		}

label_set_codec_sequenc_done:	
		return IvrSetVoipCfgToWebServer( chid, "&first_codec=%d&rate=%d", firstCodec, rate );
	}

	return IvrSetVoipCfgToWebServer( chid, "&first_codec=%d", firstCodec );	
}

int IvrSetVoipHandsetGainCfg( unsigned int chid, unsigned char gain )
{
	/* ivr command: IVR_COMMAND_HANDSET_GAIN */
	return IvrSetVoipCfgToWebServer( chid, "&slic_txVolumne=%d", gain );
}

int IvrSetVoipHandsetVolumeCfg( unsigned int chid, unsigned char volume )
{
	/* ivr command: IVR_COMMAND_HANDSET_VOLUME */
	return IvrSetVoipCfgToWebServer( chid, "&slic_rxVolumne=%d", volume );
}

int IvrSetVoipSpeakerVolumeGainCfg( unsigned int chid, unsigned char gain )
{
	/* ivr command: IVR_COMMAND_SPEAKER_VOLUME_GAIN */
	const int sign_gain = ( ( gain <= 31 ) ? ( int )gain : ( int )gain - 64 );
	
	return IvrSetVoipCfgToWebServer( chid, "&spk_voice_gain=%d", sign_gain );
}

int IvrSetVoipMicVolumeVolumeCfg( unsigned int chid, unsigned char gain )
{
	/* ivr command: IVR_COMMAND_MIC_VOLUME_GAIN */
	const int sign_gain = ( ( gain <= 31 ) ? ( int )gain : ( int )gain - 64 );

	return IvrSetVoipCfgToWebServer( chid, "&mic_voice_gain=%d", sign_gain );
}

int IvrSetVoipEnableCallWaitingCfg( unsigned int chid, unsigned char enable )
{
	/* ivr command: IVR_COMMAND_ENABLE_CALL_WAITING / IVR_COMMAND_DISABLE_CALL_WAITING */
	return IvrSetVoipCfgToWebServer( chid, "&call_waiting=%s", ( enable ? pszCheckboxOn : pszCheckboxOff ) );	
}

int IvrSetVoipForwardSettingCfg( unsigned int chid, forward_type_t type, 
								 unsigned char noAnswerTime,
								 const unsigned char *pszForwardPhoneNumber )
{
	/* ivr command: IVR_COMMAND_FORWARD_SETTING */
	const unsigned char *pszNameOfForwardType;
	const unsigned char *pszNameOfForwardNumber;
	const unsigned char *pszNameOfNoAnswerTime;
	
	switch( type ) {
	case FORWARD_TYPE_IMMEDIATE:
		pszNameOfForwardType = "CFAll";
		pszNameOfForwardNumber = "CFAll_No";
		pszNameOfNoAnswerTime = NULL;
		break;
	case FORWARD_TYPE_BUSY:
		pszNameOfForwardType = "CFBusy";
		pszNameOfForwardNumber = "CFBusy_No";
		pszNameOfNoAnswerTime = NULL;
		break;
	case FORWARD_TYPE_NO_ANSWER:
		pszNameOfForwardType = "CFNoAns";
		pszNameOfForwardNumber = "CFNoAns_No";
		pszNameOfNoAnswerTime = "CFNoAns_Time";
		break;
	default:
		return 0;
	}

	if( pszNameOfNoAnswerTime == NULL )
		return IvrSetVoipCfgToWebServer( chid, "&%s=on&%s=%s", pszNameOfForwardType, pszNameOfForwardNumber, pszForwardPhoneNumber );

	return IvrSetVoipCfgToWebServer( chid, "&%s=on&%s=%s&%s=%d", pszNameOfForwardType, pszNameOfForwardNumber, pszForwardPhoneNumber, pszNameOfNoAnswerTime, noAnswerTime );
}

int IvrSetVoipDisableForwardSettingCfg( unsigned int chid )
{
	/* ivr command: IVR_COMMAND_DISABLE_FORWARD_SETTING */
	return IvrSetVoipCfgToWebServer( chid, "&CFAll=off&CFBusy=off&CFNoAns=off" );
}

