/*
 *      Header file of MIB
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *      Authors: Dick Tam	<dicktam@realtek.com.tw>
 *
 */


#ifndef INCLUDE_MIB_H
#define INCLUDE_MIB_H

#ifdef EMBED
#include <linux/config.h>
#include <rtk/options.h>
#include <config/autoconf.h>
#else
#include "../../../../uClibc/include/linux/autoconf.h"
#include "../../../../config/autoconf.h"
#include "options.h"
#endif
/*+++++add by Jack for VoIP project 20/03/07+++++*/
#ifdef VOIP_SUPPORT
#include "voip_flash.h"
#include "voip_flash_mib.h"
#endif /*VOIP_SUPPORT*/
/*-----end-----*/


#ifndef WIN32
#define __PACK__				__attribute__ ((packed))
#else
#define __PACK__
#endif

#ifdef WIN32
#pragma pack()
#endif

/*
 * Flash File System 
 */

typedef enum { UNKNOWN_SETTING=0, RUNNING_SETTING=1, HW_SETTING=2, DEFAULT_SETTING=4, CURRENT_SETTING=8 } CONFIG_DATA_T;
typedef enum { CONFIG_MIB_ALL=0, CONFIG_MIB_TABLE, CONFIG_MIB_CHAIN } CONFIG_MIB_T;

#define SIGNATURE_LEN				8
#define HS_CONF_SETTING_SIGNATURE_TAG		((char *)"ADSL-HS-")
#define DS_CONF_SETTING_SIGNATURE_TAG		((char *)"ADSL-DS-")
#define CS_CONF_SETTING_SIGNATURE_TAG		((char *)"ADSL-CS-")
#define WEB_SIGNATURE_TAG			((char *)"ADSL-WEB")
#define FLASH_FILE_SYSTEM_VERSION		1
#define FLASH_DEFAULT_TO_ALL			0
#define FLASH_DEFAULT_TO_AUGMENT		1


/* File header */
typedef struct param_header {
	unsigned char signature[SIGNATURE_LEN] __PACK__;
	unsigned char version __PACK__;
	unsigned char checksum __PACK__;
	unsigned int len __PACK__;
} PARAM_HEADER_T, *PARAM_HEADER_Tp;

/* Firmware image header */
typedef struct _header_ {
 // Kao
	unsigned long signature;
	unsigned long startAddr;
	unsigned long len;
} IMG_HEADER_T, *IMG_HEADER_Tp;

#define FIRMWARE_MAGIC_NUMBER		0xa3d275e9
#define FIRMWARE_PARAM_SIZE		0x10
#define DST_IMAGE_ADDR			0x80000000

/* scramble saved configuration data */
#define ENCODE_DATA(data,len)
#define DECODE_DATA(data,len)

// Added by Mason Yu
#define ACT_NONE				0
#define ACT_START				1
#define ACT_STOP				2
#define ACT_RESTART				3


/*
#define ENCODE_DATA(data,len) { \
	int i; \
	for (i=0; i<len; i++) \
		data[i] = ~ ( data[i] + 0x38); \
}

#define DECODE_DATA(data,len) { \
	int i; \
	for (i=0; i<len; i++) \
		data[i] = ~data[i] - 0x38;	\
}
*/

/* Do checksum and verification for configuration data */
#ifndef WIN32
static inline unsigned char CHECKSUM(unsigned char *data, unsigned int len)
#else
__inline unsigned char CHECKSUM(unsigned char *data, unsigned int len)
#endif
{
	unsigned int i;
	unsigned char sum=0;

	for (i=0; i<len; i++) {
		sum += data[i];
	}
	
	sum = ~sum + 1;
	return sum;
}










/*
 * Webpage gzip/unzip
 */
#define GZIP_MAX_NAME_LEN		60
typedef struct file_entry {
	char name[GZIP_MAX_NAME_LEN];
	unsigned long size;
} FILE_ENTRY_T, *FILE_ENTRY_Tp;


#define DWORD_SWAP(v) ( (((v&0xff)<<24)&0xff000000) | \
						((((v>>8)&0xff)<<16)&0xff0000) | \
						((((v>>16)&0xff)<<8)&0xff00) | \
						(((v>>24)&0xff)&0xff) )
#define WORD_SWAP(v) ((unsigned short)(((v>>8)&0xff) | ((v<<8)&0xff00)))










/*
 * ADSL Router MIB ID
 */
#define CS_ENTRY_ID					0						

#define MIB_ADSL_LAN_IP					CS_ENTRY_ID + 1
#define MIB_ADSL_LAN_SUBNET				CS_ENTRY_ID + 2
#define MIB_ADSL_LAN_IP2				CS_ENTRY_ID + 3
#define MIB_ADSL_LAN_SUBNET2				CS_ENTRY_ID + 4
#define MIB_ADSL_LAN_DHCP				CS_ENTRY_ID + 5
#define MIB_ADSL_LAN_CLIENT_START			CS_ENTRY_ID + 6
#define MIB_ADSL_LAN_CLIENT_END				CS_ENTRY_ID + 7
#define MIB_ADSL_LAN_DHCP_LEASE				CS_ENTRY_ID + 8
#define MIB_ADSL_LAN_DHCP_DOMAIN			CS_ENTRY_ID + 9
#define MIB_ADSL_LAN_RIP				CS_ENTRY_ID + 10
#define MIB_ADSL_LAN_AUTOSEARCH				CS_ENTRY_ID + 11
#define MIB_ADSL_LAN_ENABLE_IP2				CS_ENTRY_ID + 12
#define MIB_ADSL_LAN_DHCP_POOLUSE			CS_ENTRY_ID + 13
#define MIB_DHCP_POOL_START				CS_ENTRY_ID + 14
#define MIB_DHCP_POOL_END				CS_ENTRY_ID + 15
#define MIB_DHCP_DNS_OPTION				CS_ENTRY_ID + 16

#ifdef DEFAULT_GATEWAY_V2
#define MIB_ADSL_WAN_DGW_IP			CS_ENTRY_ID + 17
#define MIB_ADSL_WAN_DGW_ITF			CS_ENTRY_ID + 18
#endif
#define MIB_ADSL_WAN_DNS_MODE				CS_ENTRY_ID + 19
#define MIB_ADSL_WAN_DNS1				CS_ENTRY_ID + 20
#define MIB_ADSL_WAN_DNS2				CS_ENTRY_ID + 21
#define MIB_ADSL_WAN_DNS3				CS_ENTRY_ID + 22
#define MIB_ADSL_CONNECTION_MODE			CS_ENTRY_ID + 24
#define MIB_ADSL_ENCAP_MODE				CS_ENTRY_ID + 25

#define MIB_ADSL_MODE					CS_ENTRY_ID + 26
#define MIB_ADSL_OLR					CS_ENTRY_ID + 27

#define MIB_RIP_ENABLE					CS_ENTRY_ID + 28
#define MIB_RIP_INTERFACE				CS_ENTRY_ID + 29
#define MIB_RIP_VERSION					CS_ENTRY_ID + 30

#define MIB_IPF_OUT_ACTION				CS_ENTRY_ID + 32
#define MIB_IPF_IN_ACTION				CS_ENTRY_ID + 33
#define MIB_MACF_OUT_ACTION				CS_ENTRY_ID + 34
#define MIB_MACF_IN_ACTION				CS_ENTRY_ID + 35
#define MIB_PORT_FW_ENABLE				CS_ENTRY_ID + 36
#define MIB_DMZ_ENABLE					CS_ENTRY_ID + 37
#define MIB_DMZ_IP					CS_ENTRY_ID + 38
#ifdef NATIP_FORWARDING
#define MIB_IP_FW_ENABLE				CS_ENTRY_ID + 39
#endif
	
#define MIB_USER_NAME					CS_ENTRY_ID + 41
#define MIB_USER_PASSWORD				CS_ENTRY_ID + 42
#define MIB_DEVICE_TYPE					CS_ENTRY_ID + 43
#define MIB_INIT_LINE					CS_ENTRY_ID + 44
#define MIB_INIT_SCRIPT					CS_ENTRY_ID + 45

#if defined(CONFIG_USER_SNMPD_SNMPD_V2CTRAP) || defined(_CWMP_MIB_)
#define MIB_SNMP_SYS_DESCR				CS_ENTRY_ID + 51
#endif //defined(CONFIG_USER_SNMPD_SNMPD_V2CTRAP) || defined(_CWMP_MIB_)
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
#define MIB_SNMP_SYS_OID				CS_ENTRY_ID + 52
#define MIB_SNMP_SYS_CONTACT				CS_ENTRY_ID + 53
#define MIB_SNMP_SYS_LOCATION				CS_ENTRY_ID + 55
#define MIB_SNMP_TRAP_IP				CS_ENTRY_ID + 56
#define MIB_SNMP_COMM_RO				CS_ENTRY_ID + 57
#define MIB_SNMP_COMM_RW				CS_ENTRY_ID + 58
#endif
#define MIB_SNMP_SYS_NAME				CS_ENTRY_ID + 54

#define MIB_ATM_LOOPBACK				CS_ENTRY_ID + 59
#define MIB_ATM_MODE					CS_ENTRY_ID + 60
#define MIB_ATM_VC_SWITCH				CS_ENTRY_ID + 61
#define MIB_ATM_MAC1					CS_ENTRY_ID + 62
#define MIB_ATM_MAC2					CS_ENTRY_ID + 63
#define MIB_ATM_VC_AUTOSEARCH				CS_ENTRY_ID + 64
// Kao
#define MIB_BRCTL_AGEINGTIME				CS_ENTRY_ID + 71
#define MIB_BRCTL_STP					CS_ENTRY_ID + 72

#define MIB_MPMODE					CS_ENTRY_ID + 73
#define MIB_QOS_DOMAIN					CS_ENTRY_ID + 74
#ifdef QOS_DIFFSERV
#define MIB_QOS_DIFFSERV					CS_ENTRY_ID + 75
#define MIB_DIFFSERV_PHBCLASS				CS_ENTRY_ID + 76
#endif

#define MIB_IGMP_PROXY					CS_ENTRY_ID + 77
#define MIB_IGMP_PROXY_ITF				CS_ENTRY_ID + 78
#define MIB_IPPT_ITF					CS_ENTRY_ID + 79
#define MIB_IPPT_LEASE					CS_ENTRY_ID + 80
#define MIB_IPPT_LANACC					CS_ENTRY_ID + 81
#define MIB_SPC_ENABLE					CS_ENTRY_ID + 82
#define MIB_SPC_IPTYPE					CS_ENTRY_ID + 83
#define MIB_ACL_CAPABILITY				CS_ENTRY_ID + 84
#define MIB_ADSL_WAN_DHCPS				CS_ENTRY_ID + 85
#define MIB_DHCP_MODE					CS_ENTRY_ID + 86
#if defined(URL_BLOCKING_SUPPORT)||defined(URL_ALLOWING_SUPPORT)
#define MIB_URL_CAPABILITY				CS_ENTRY_ID + 87  // Mason Yu for URL Blocking
#endif
#define MIB_NTP_ENABLED 				CS_ENTRY_ID + 88
#define MIB_NTP_SERVER_ID 				CS_ENTRY_ID + 89
#define MIB_NTP_TIMEZONE 				CS_ENTRY_ID + 90
#define MIB_NTP_SERVER_IP1				CS_ENTRY_ID + 91
#define MIB_NTP_SERVER_IP2				CS_ENTRY_ID + 92				
#define MIB_NTP_SERVER_HOST1				CS_ENTRY_ID + 93
#define MIB_NTP_SERVER_HOST2				CS_ENTRY_ID + 94				
#define	MIB_UPNP_DAEMON					CS_ENTRY_ID + 95  // Mason Yu for UPNP
#define MIB_UPNP_EXT_ITF				CS_ENTRY_ID + 96
#ifdef DOMAIN_BLOCKING_SUPPORT
#define MIB_DOMAINBLK_CAPABILITY			CS_ENTRY_ID + 97  // Mason Yu for Domain Blocking
#endif

#ifdef CONFIG_IGMP_FORBID
#define MIB_IGMP_FORBID_ENABLE                 CS_ENTRY_ID + 98     //alex_huang for igmp forbid       
#endif
#ifdef WLAN_SUPPORT
// WLAN MIB id
#define MIB_WLAN_SSID					CS_ENTRY_ID + 101
#define MIB_WLAN_CHAN_NUM				CS_ENTRY_ID + 102
#define MIB_WLAN_WEP					CS_ENTRY_ID + 103
#define MIB_WLAN_WEP64_KEY1				CS_ENTRY_ID + 104
#define MIB_WLAN_WEP64_KEY2				CS_ENTRY_ID + 105
#define MIB_WLAN_WEP64_KEY3				CS_ENTRY_ID + 106
#define MIB_WLAN_WEP64_KEY4				CS_ENTRY_ID + 107
#define MIB_WLAN_WEP128_KEY1				CS_ENTRY_ID + 108
#define MIB_WLAN_WEP128_KEY2				CS_ENTRY_ID + 109
#define MIB_WLAN_WEP128_KEY3				CS_ENTRY_ID + 110
#define MIB_WLAN_WEP128_KEY4				CS_ENTRY_ID + 111
#define MIB_WLAN_WEP_KEY_TYPE				CS_ENTRY_ID + 112
#define MIB_WLAN_WEP_DEFAULT_KEY			CS_ENTRY_ID + 113
#define MIB_WLAN_FRAG_THRESHOLD				CS_ENTRY_ID + 114
#define MIB_WLAN_SUPPORTED_RATE				CS_ENTRY_ID + 115
#define MIB_WLAN_BEACON_INTERVAL			CS_ENTRY_ID + 116
#define MIB_WLAN_PREAMBLE_TYPE				CS_ENTRY_ID + 117
#define MIB_WLAN_BASIC_RATE				CS_ENTRY_ID + 118
#define MIB_WLAN_RTS_THRESHOLD				CS_ENTRY_ID + 119
#define MIB_WLAN_AUTH_TYPE				CS_ENTRY_ID + 120
#define MIB_WLAN_HIDDEN_SSID				CS_ENTRY_ID + 121
#define MIB_WLAN_DISABLED				CS_ENTRY_ID + 122
#define MIB_ALIAS_NAME					CS_ENTRY_ID + 123
// Added by Mason Yu for TxPower
#define MIB_TX_POWER					CS_ENTRY_ID + 124 
 //cathy, for multicast rate
#ifdef CONFIG_USB_RTL8187SU_SOFTAP	
#define MIB_WLAN_MLCSTRATE					CS_ENTRY_ID + 125
#endif
                                        		
#ifdef WLAN_WPA
#define MIB_WLAN_ENCRYPT				CS_ENTRY_ID + 126
#define MIB_WLAN_ENABLE_SUPP_NONWPA			CS_ENTRY_ID + 127
#define MIB_WLAN_SUPP_NONWPA				CS_ENTRY_ID + 128
#define MIB_WLAN_WPA_AUTH				CS_ENTRY_ID + 129
#define MIB_WLAN_WPA_CIPHER_SUITE			CS_ENTRY_ID + 130
#define MIB_WLAN_WPA_PSK				CS_ENTRY_ID + 131
#define MIB_WLAN_WPA_GROUP_REKEY_TIME			CS_ENTRY_ID + 132
#ifdef WLAN_1x
#define MIB_WLAN_RS_IP					CS_ENTRY_ID + 133
#define MIB_WLAN_RS_PORT				CS_ENTRY_ID + 134
#define MIB_WLAN_RS_PASSWORD				CS_ENTRY_ID + 135
#define MIB_WLAN_ENABLE_1X				CS_ENTRY_ID + 136
#endif
#define MIB_WLAN_WPA_PSK_FORMAT				CS_ENTRY_ID + 137
#define MIB_WLAN_WPA2_PRE_AUTH				CS_ENTRY_ID + 138
#define MIB_WLAN_WPA2_CIPHER_SUITE			CS_ENTRY_ID + 139
#endif                                  		
                                        		
#define MIB_WLAN_INACTIVITY_TIME			CS_ENTRY_ID + 140
#define MIB_WLAN_RATE_ADAPTIVE_ENABLED			CS_ENTRY_ID + 141
                                        		
#ifdef WLAN_ACL
// access control MIB id
#define MIB_WLAN_AC_ENABLED				CS_ENTRY_ID + 142
#define MIB_WLAN_AC_NUM					CS_ENTRY_ID + 143
#define MIB_WLAN_AC_ADDR				CS_ENTRY_ID + 144
#define MIB_WLAN_AC_ADDR_ADD				CS_ENTRY_ID + 145
#define MIB_WLAN_AC_ADDR_DEL				CS_ENTRY_ID + 146
#define MIB_WLAN_AC_ADDR_DELALL				CS_ENTRY_ID + 147
#endif

#define MIB_WLAN_DTIM_PERIOD				CS_ENTRY_ID + 150
#define MIB_WLAN_MODE					CS_ENTRY_ID + 151
#define MIB_WLAN_NETWORK_TYPE				CS_ENTRY_ID + 152
#define MIB_WLAN_DEFAULT_SSID				CS_ENTRY_ID + 153	// used while configured as Ad-hoc and no any other Ad-hoc could be joined
								// it will use this default SSID to start BSS
#ifdef WLAN_WPA
#define MIB_WLAN_ACCOUNT_RS_ENABLED			CS_ENTRY_ID + 160
#define MIB_WLAN_ACCOUNT_RS_IP				CS_ENTRY_ID + 161
#define MIB_WLAN_ACCOUNT_RS_PORT			CS_ENTRY_ID + 162
#define MIB_WLAN_ACCOUNT_RS_PASSWORD			CS_ENTRY_ID + 163
#define MIB_WLAN_ACCOUNT_UPDATE_ENABLED			CS_ENTRY_ID + 164
#define MIB_WLAN_ACCOUNT_UPDATE_DELAY			CS_ENTRY_ID + 165
#define MIB_WLAN_ENABLE_MAC_AUTH			CS_ENTRY_ID + 166
#define MIB_WLAN_RS_RETRY				CS_ENTRY_ID + 167
#define MIB_WLAN_RS_INTERVAL_TIME			CS_ENTRY_ID + 168
#define MIB_WLAN_ACCOUNT_RS_RETRY			CS_ENTRY_ID + 169
#define MIB_WLAN_ACCOUNT_RS_INTERVAL_TIME		CS_ENTRY_ID + 170
#endif

#ifdef WLAN_IAPP
#define MIB_WLAN_IAPP_DISABLED				CS_ENTRY_ID + 180
#endif

//#ifdef WLAN_WDS
#define MIB_WLAN_WDS_ENABLED				CS_ENTRY_ID + 181
#define MIB_WLAN_WDS_NUM				CS_ENTRY_ID + 182
#define MIB_WLAN_WDS					CS_ENTRY_ID + 183
#define MIB_WLAN_WDS_ADD				CS_ENTRY_ID + 184
#define MIB_WLAN_WDS_DEL				CS_ENTRY_ID + 185
#define MIB_WLAN_WDS_DELALL				CS_ENTRY_ID + 186
#define MIB_WLAN_WDS_WEP				CS_ENTRY_ID + 187
#define MIB_WLAN_WDS_WEP64_KEY1				CS_ENTRY_ID + 188
#define MIB_WLAN_WDS_WEP64_KEY2				CS_ENTRY_ID + 189
#define MIB_WLAN_WDS_WEP64_KEY3				CS_ENTRY_ID + 190
#define MIB_WLAN_WDS_WEP64_KEY4				CS_ENTRY_ID + 191
#define MIB_WLAN_WDS_WEP128_KEY1			CS_ENTRY_ID + 192
#define MIB_WLAN_WDS_WEP128_KEY2			CS_ENTRY_ID + 193
#define MIB_WLAN_WDS_WEP128_KEY3			CS_ENTRY_ID + 194
#define MIB_WLAN_WDS_WEP128_KEY4			CS_ENTRY_ID + 195
#define MIB_WLAN_WDS_WEP_KEY_TYPE			CS_ENTRY_ID + 196
#define MIB_WLAN_WDS_WEP_DEFAULT_KEY			CS_ENTRY_ID + 197
//#endif

#ifdef WLAN_8185AG
#define MIB_WLAN_BAND					CS_ENTRY_ID + 198
#define MIB_WLAN_FIX_RATE				CS_ENTRY_ID + 199
#endif

#define MIB_WLAN_PRIVACY_CHECK				CS_ENTRY_ID + 200
#define MIB_WLAN_BLOCK_RELAY				CS_ENTRY_ID + 201
#define MIB_NAT25_MAC_CLONE				CS_ENTRY_ID + 202
// Added by jiunming for setting an option "ethernet to wireless blocking" in page wladvanced.asp
#define MIB_WLAN_BLOCK_ETH2WIR				CS_ENTRY_ID + 203
#define MIB_WLAN_ITF_GROUP				CS_ENTRY_ID + 204

#ifdef WLAN_WEB_REDIRECT  //jiunming,web_redirect
#define MIB_WLAN_WEB_REDIR_URL				CS_ENTRY_ID + 205
#endif
#ifdef WLAN_QoS	//Added for WMM support
#define MIB_WLAN_QoS						CS_ENTRY_ID + 206
#endif
#define MIB_WLAN_DIG					CS_ENTRY_ID + 207
#define MIB_WLAN_BEACON_ADVERTISEMENT	CS_ENTRY_ID+208
#endif
// Jenny, system log
#define MIB_SYSLOG_LOG_LEVEL			CS_ENTRY_ID + 215
#define MIB_SYSLOG_DISPLAY_LEVEL		CS_ENTRY_ID + 216
#ifdef SYSLOG_REMOTE_LOG
#define MIB_SYSLOG_MODE				CS_ENTRY_ID + 217
#define MIB_SYSLOG_SERVER_IP			CS_ENTRY_ID + 218
#define MIB_SYSLOG_SERVER_PORT		CS_ENTRY_ID + 219
#endif

// Added by Mason Yu for write superUser into Current Setting
#define MIB_SUSER_NAME					CS_ENTRY_ID + 220
#define MIB_SUSER_PASSWORD				CS_ENTRY_ID + 221
#define MIB_ADSL_TONE					CS_ENTRY_ID + 222
// ioctl for direct bridge mode, jiunming
#define MIB_DIRECT_BRIDGE_MODE				CS_ENTRY_ID + 223
#define MIB_ADSL_HIGH_INP				CS_ENTRY_ID + 224
#ifdef AUTO_PROVISIONING
// Jenny, configuration file version
#define MIB_CONFIG_VERSION				CS_ENTRY_ID + 225
// Jenny, http server IP for auto-provisioning
#define MIB_HTTP_SERVER_IP				CS_ENTRY_ID + 226
#endif
#if 0
// Jenny, PPPoE AC MAC address
#define MIB_PPPOE_AC_MAC_ADDR				CS_ENTRY_ID + 227
// Jenny,identifier for PPPoE session
#define MIB_PPPOE_SESSION				CS_ENTRY_ID + 228
#endif
#define MIB_SYSLOG					CS_ENTRY_ID + 230
#define MIB_MAXLOGLEN					CS_ENTRY_ID + 231
#define MIB_ADSL_DEBUG					CS_ENTRY_ID + 232
#define MIB_ETH_MAC_CTRL				CS_ENTRY_ID + 233
#define MIB_WLAN_MAC_CTRL				CS_ENTRY_ID + 234



//for DoS
#ifdef DOS_SUPPORT
#define MIB_DOS_ENABLED		CS_ENTRY_ID + 235
#define MIB_DOS_SYSSYN_FLOOD		CS_ENTRY_ID + 236
#define MIB_DOS_SYSFIN_FLOOD		CS_ENTRY_ID + 237
#define MIB_DOS_SYSUDP_FLOOD		CS_ENTRY_ID + 238
#define MIB_DOS_SYSICMP_FLOOD		CS_ENTRY_ID + 239
#define MIB_DOS_PIPSYN_FLOOD		CS_ENTRY_ID + 240
#define MIB_DOS_PIPFIN_FLOOD		CS_ENTRY_ID + 241
#define MIB_DOS_PIPUDP_FLOOD		CS_ENTRY_ID + 242
#define MIB_DOS_PIPICMP_FLOOD		CS_ENTRY_ID + 243
#define MIB_DOS_BLOCK_TIME		CS_ENTRY_ID + 244
#endif

// Mason Yu for DHCP Server Gateway address 
#define MIB_ADSL_LAN_DHCP_GATEWAY			CS_ENTRY_ID + 245
#ifdef WLAN_MBSSID
#define MIB_WLAN_VAP0_ITF_GROUP				CS_ENTRY_ID + 246
#define MIB_WLAN_VAP1_ITF_GROUP				CS_ENTRY_ID + 247
#define MIB_WLAN_VAP2_ITF_GROUP				CS_ENTRY_ID + 248
#define MIB_WLAN_VAP3_ITF_GROUP				CS_ENTRY_ID + 249
#endif

// WPS
//#ifdef WIFI_SIMPLE_CONFIG
#define MIB_WSC_DISABLE 				CS_ENTRY_ID + 250
#define MIB_WSC_METHOD					CS_ENTRY_ID + 251
#define MIB_WSC_CONFIGURED				CS_ENTRY_ID + 252
#define MIB_WSC_PIN					CS_ENTRY_ID + 253
#define MIB_WSC_AUTH					CS_ENTRY_ID + 254
#define MIB_WSC_ENC					CS_ENTRY_ID + 255
#define MIB_WSC_MANUAL_ENABLED 				CS_ENTRY_ID + 256
#define MIB_WSC_PSK					CS_ENTRY_ID + 257
#define MIB_WSC_SSID					CS_ENTRY_ID + 258
#define MIB_WSC_UPNP_ENABLED				CS_ENTRY_ID + 259
#define MIB_WSC_REGISTRAR_ENABLED 			CS_ENTRY_ID + 260
#define MIB_WSC_CONFIG_BY_EXT_REG 			CS_ENTRY_ID + 261
//#endif

#ifdef CTC_TELECOM_ACCOUNT
#define MIB_CTC_ACCOUNT_ENABLE				CS_ENTRY_ID + 262
#endif //CTC_TELECOM_ACCOUNT

//added by xl_yue
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
#define MIB_WLAN_COUNTRY_AREA				CS_ENTRY_ID + 263
#define MIB_WLAN_REG_DOMAIN				CS_ENTRY_ID + 264
#endif
#ifdef CONFIG_IP_NF_ALG_ONOFF
#ifdef CONFIG_IP_NF_FTP
#define MIB_IP_ALG_FTP         CS_ENTRY_ID+265
#endif
#ifdef CONFIG_IP_NF_H323
#define MIB_IP_ALG_H323         CS_ENTRY_ID+266
#endif
#ifdef CONFIG_IP_NF_IRC
#define MIB_IP_ALG_IRC         CS_ENTRY_ID+267
#endif
#ifdef CONFIG_IP_NF_RTSP
#define MIB_IP_ALG_RTSP         CS_ENTRY_ID+268
#endif
#ifdef CONFIG_IP_NF_QUAKE3
#define MIB_IP_ALG_QUAKE3        CS_ENTRY_ID+269
#endif
#ifdef CONFIG_IP_NF_CUSEEME
#define MIB_IP_ALG_CUSEEME         CS_ENTRY_ID+270
#endif
#ifdef CONFIG_IP_NF_L2TP
#define MIB_IP_ALG_L2TP         CS_ENTRY_ID+271
#endif
#ifdef CONFIG_IP_NF_IPSEC
#define MIB_IP_ALG_IPSEC         CS_ENTRY_ID+272
#endif
#ifdef CONFIG_IP_NF_SIP
#define MIB_IP_ALG_SIP         CS_ENTRY_ID+273
#endif
#ifdef CONFIG_IP_NF_PPTP
#define MIB_IP_ALG_PPTP         CS_ENTRY_ID+274
#endif
#endif
#ifdef DNS_BIND_PVC_SUPPORT
#define MIB_DNS_BIND_PVC_ENABLE CS_ENTRY_ID+275
#define MIB_DNS_BIND_PVC1 CS_ENTRY_ID+276
#define MIB_DNS_BIND_PVC2 CS_ENTRY_ID+277
#define MIB_DNS_BIND_PVC3 CS_ENTRY_ID+278
#endif
//ql_xu add
#ifdef NAT_CONN_LIMIT
#define MIB_NAT_CONN_LIMIT			CS_ENTRY_ID + 279
#endif

#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
#define MIB_OSPF_ENABLE				CS_ENTRY_ID + 280
#endif

//alex_huang
#ifdef CONFIG_8021P_PRIO
#define MIB_8021P_PROI                     CS_ENTRY_ID+281
#endif
//add by ramen
#ifdef QOS_SPEED_LIMIT_SUPPORT
#define MIB_PVC_TOTAL_BANDWIDTH  CS_ENTRY_ID+282
#endif

//#ifdef ADDRESS_MAPPING
#define MIB_ADDRESS_MAP_TYPE				CS_ENTRY_ID + 283
#define MIB_LOCAL_START_IP				CS_ENTRY_ID + 284
#define MIB_LOCAL_END_IP				CS_ENTRY_ID + 285
#define MIB_GLOBAL_START_IP				CS_ENTRY_ID + 286
#define MIB_GLOBAL_END_IP				CS_ENTRY_ID + 287
//#endif 

#define MIB_CS_VERSION					CS_ENTRY_ID + 288

//ql
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
#define MIB_REFRESH_TIME				CS_ENTRY_ID + 289
#endif

//xl_yue
#ifdef MIB_WLAN_RESTART_SUPPORT
#define MIB_WLAN_RESTART				CS_ENTRY_ID + 290
#endif

#define MIB_SNMP_AUTORUN				CS_ENTRY_ID + 291
#define MIB_CWMP_AUTORUN				CS_ENTRY_ID + 292

#ifndef CONFIG_EXT_SWITCH 
#ifdef ELAN_LINK_MODE_INTRENAL_PHY
#define MIB_ETH_MODE					CS_ENTRY_ID + 293
#endif
#endif 

#ifdef SYSLOG_SUPPORT
#ifdef SEND_LOG
#define MIB_LOG_SERVER_IP			 	CS_ENTRY_ID + 294
#define MIB_LOG_SERVER_NAME				CS_ENTRY_ID + 295
#define MIB_LOG_SERVER_PASSWORD				CS_ENTRY_ID + 296
#endif
#endif
//#ifdef QSETUP_WEB_REDIRECT
//#define MIB_QSETUP_REDIRECT			CS_ENTRY_ID + 297
//#endif
#define MIB_DHCPS_DNS1					CS_ENTRY_ID + 298
#define MIB_DHCPS_DNS2					CS_ENTRY_ID + 299
#define MIB_DHCPS_DNS3					CS_ENTRY_ID + 300

#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
#define MIB_SNMPD_ENABLE				CS_ENTRY_ID + 301
#endif

#ifdef TCP_UDP_CONN_LIMIT
#define MIB_CONNLIMIT_ENABLE			CS_ENTRY_ID + 302
#define MIB_CONNLIMIT_UDP				CS_ENTRY_ID + 303
#define MIB_CONNLIMIT_TCP				CS_ENTRY_ID + 304
#endif //TCP_UDP_CONN_LIMIT


#ifdef WEB_REDIRECT_BY_MAC
#define MIB_WEB_REDIR_BY_MAC_URL			CS_ENTRY_ID + 305
#define MIB_WEB_REDIR_BY_MAC_INTERVAL			CS_ENTRY_ID + 306
#endif

#ifdef CONFIG_USB_ETH
#define MIB_USBETH_ITF_GROUP				CS_ENTRY_ID + 307
#endif //CONFIG_USB_ETH
#ifdef CONFIG_RTL867X_COMBO_PORTMAPPING
#define MIB_ETH_ITF_GROUP				CS_ENTRY_ID + 308
#endif
/*ql:20081114 START: new IP QoS*/
#ifdef NEW_IP_QOS_SUPPORT
#define MIB_QOS_UPRATE				CS_ENTRY_ID+309
#define MIB_QOS_POLICY				CS_ENTRY_ID+310
#define MIB_TOTAL_BANDWIDTH			CS_ENTRY_ID+311
#define MIB_TOTAL_BANDWIDTH_LIMIT_EN	CS_ENTRY_ID+312
#define MIB_QOS_MODE				CS_ENTRY_ID+313
#endif
/*ql:20081114 END*/
#if defined(CONFIG_8021P_PRIO) && defined(NEW_IP_QOS_SUPPORT)
#define MIB_PRED_PROI			CS_ENTRY_ID+314
#endif
//#ifdef DHCPS_POOL_COMPLETE_IP
#define MIB_DHCP_SUBNET_MASK		CS_ENTRY_ID+315
//#endif
#ifdef WLAN_QoS	//Added for WMM support
#define MIB_WLAN_APSD_ENABLE						CS_ENTRY_ID + 316
#endif

#ifdef CONFIG_USB_RTL8192SU_SOFTAP			// add by yq_zhou 1.20
#define MIB_WLAN_PROTECTION_DISABLED                 CS_ENTRY_ID + 317  
#define MIB_WLAN_AGGREGATION 				CS_ENTRY_ID + 318
#define MIB_WLAN_SHORTGI_ENABLED			CS_ENTRY_ID + 319	
#endif 

#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
#define MIB_CAPTIVEPORTAL_ENABLE CS_ENTRY_ID+321
#define MIB_CAPTIVEPORTAL_URL CS_ENTRY_ID+322
#endif

#ifdef CONFIG_USB_RTL8192SU_SOFTAP		//add by yq_zhou 2.10
#define MIB_WLAN_CHANNEL_WIDTH  			CS_ENTRY_ID + 320	      
#define MIB_WLAN_CONTROL_BAND				CS_ENTRY_ID + 323	
#endif 

#define HS_ENTRY_ID					400

#define MIB_SUPER_NAME					HS_ENTRY_ID + 1
#define MIB_SUPER_PASSWORD				HS_ENTRY_ID + 2
#define MIB_BOOT_MODE					HS_ENTRY_ID + 3
#define MIB_ELAN_MAC_ADDR				HS_ENTRY_ID + 4
#define MIB_WLAN_MAC_ADDR				HS_ENTRY_ID + 5
//#if WLAN_SUPPORT

#define MIB_HW_REG_DOMAIN				HS_ENTRY_ID + 6
#define MIB_HW_RF_TYPE					HS_ENTRY_ID + 7
//#ifndef WLAN_8185AG
#define MIB_HW_TX_POWER					HS_ENTRY_ID + 8
//#else
#define MIB_HW_TX_POWER_CCK				HS_ENTRY_ID + 9
#define MIB_HW_TX_POWER_OFDM				HS_ENTRY_ID + 10
//#endif
#define MIB_HW_ANT_DIVERSITY				HS_ENTRY_ID + 11
#define MIB_HW_TX_ANT					HS_ENTRY_ID + 12
#define MIB_HW_CS_THRESHOLD				HS_ENTRY_ID + 13
#define MIB_HW_CCA_MODE					HS_ENTRY_ID + 14
#define MIB_HW_PHY_TYPE					HS_ENTRY_ID + 15
#define MIB_HW_LED_TYPE					HS_ENTRY_ID + 16

//#endif // of WLAN_SUPPORT

#define MIB_BYTE_TEST					HS_ENTRY_ID + 17
#define MIB_WORD_TEST					HS_ENTRY_ID + 18
#define MIB_DWORD_TEST					HS_ENTRY_ID + 19
#define MIB_INTERGER_TEST1				HS_ENTRY_ID + 20
#define MIB_INTERGER_TEST2				HS_ENTRY_ID + 21
//#if WLAN_SUPPORT
#define MIB_WIFI_SUPPORT                       HS_ENTRY_ID+23
//#endif
#define MIB_HW_SERIAL_NUMBER				HS_ENTRY_ID + 24

#define CHAIN_ENTRY_TBL_ID				350

#define MIB_IP_PORT_FILTER_TBL				CHAIN_ENTRY_TBL_ID + 1
#define MIB_MAC_FILTER_TBL				CHAIN_ENTRY_TBL_ID + 2
#define MIB_PORT_FW_TBL					CHAIN_ENTRY_TBL_ID + 3
#define MIB_ATM_VC_TBL					CHAIN_ENTRY_TBL_ID + 4
#define MIB_IP_ROUTE_TBL				CHAIN_ENTRY_TBL_ID + 5
#define MIB_ACL_IP_TBL					CHAIN_ENTRY_TBL_ID + 6
#ifdef WLAN_SUPPORT
#ifdef WLAN_ACL
#define MIB_WLAN_AC_TBL					CHAIN_ENTRY_TBL_ID + 7
#endif
#endif
#if defined( CONFIG_EXT_SWITCH)
#define MIB_SW_PORT_TBL					CHAIN_ENTRY_TBL_ID + 8
#define MIB_VLAN_TBL					CHAIN_ENTRY_TBL_ID + 9
#endif
#define MIB_IP_QOS_TBL					CHAIN_ENTRY_TBL_ID + 10
#define MIB_ACC_TBL					CHAIN_ENTRY_TBL_ID + 11
#ifdef WLAN_WDS
#define MIB_WDS_TBL	   			        CHAIN_ENTRY_TBL_ID + 12
#endif

#ifdef PORT_TRIGGERING
#define MIB_PORT_TRG_TBL				CHAIN_ENTRY_TBL_ID + 13
#endif

#ifdef NATIP_FORWARDING
#define MIB_IP_FW_TBL					CHAIN_ENTRY_TBL_ID + 14
#endif
#ifdef URL_BLOCKING_SUPPORT
#define MIB_URL_FQDN_TBL				CHAIN_ENTRY_TBL_ID + 15
#endif
#ifdef _CWMP_MIB_ /*jiunming, mib for cwmp-tr069*/
#ifdef WLAN_SUPPORT
#define CWMP_PSK_TBL					CHAIN_ENTRY_TBL_ID + 16
#endif
#endif /*_CWMP_MIB_*/
#define MIB_MAC_BASE_DHCP_TBL				CHAIN_ENTRY_TBL_ID + 17
#define MIB_AUTO_PVC_SEARCH_TBL				CHAIN_ENTRY_TBL_ID + 18	// for auto-pvc-search
#ifdef CONFIG_USER_DDNS
#define MIB_DDNS_TBL					CHAIN_ENTRY_TBL_ID + 19
#endif

#ifdef _PRMT_X_CT_COM_PPPOE_
#define CWMP_CT_PPPOE_TBL				CHAIN_ENTRY_TBL_ID + 20
#endif /*_PRMT_X_CT_COM_PPPOE_*/

#define MIB_PPPOE_SESSION_TBL				CHAIN_ENTRY_TBL_ID + 21	// Jenny, add for PPPoE session information

#ifdef DOMAIN_BLOCKING_SUPPORT
#define MIB_DOMAIN_BLOCKING_TBL				CHAIN_ENTRY_TBL_ID + 22
#endif

#ifdef URL_BLOCKING_SUPPORT
#define MIB_KEYWD_FILTER_TBL				CHAIN_ENTRY_TBL_ID + 23
#endif

#define MIB_RIP_TBL					CHAIN_ENTRY_TBL_ID + 24

#ifdef WLAN_MBSSID
#define MIB_MBSSIB_TBL					CHAIN_ENTRY_TBL_ID + 25
#define MIB_MBSSIB_WEP_TBL				CHAIN_ENTRY_TBL_ID + 26
#endif

#ifdef ACCOUNT_CONFIG
#define MIB_ACCOUNT_CONFIG_TBL				CHAIN_ENTRY_TBL_ID + 27	// Jenny, user account table
#endif

#ifdef ZTE_GENERAL_ROUTER_SC
#define MIB_VIRTUAL_SVR_TBL			CHAIN_ENTRY_TBL_ID + 28
#endif

//ql_xu add:
#ifdef MAC_ACL
#define MIB_ACL_MAC_TBL				CHAIN_ENTRY_TBL_ID + 29
#endif
#ifdef NAT_CONN_LIMIT
#define MIB_CONN_LIMIT_TBL			CHAIN_ENTRY_TBL_ID + 30
#endif

#ifdef URL_ALLOWING_SUPPORT
#define MIB_URL_ALLOW_FQDN_TBL            	CHAIN_ENTRY_TBL_ID + 31
#endif

#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
#define MIB_OSPF_TBL				CHAIN_ENTRY_TBL_ID + 32
#endif

#ifdef  QOS_SPEED_LIMIT_SUPPORT
#define MIB_QOS_SPEED_LIMIT	   		CHAIN_ENTRY_TBL_ID + 33
#endif

#ifdef LAYER7_FILTER_SUPPORT
#define MIB_LAYER7_FILTER_TBL     		CHAIN_ENTRY_TBL_ID + 34
#endif

/*+++++add by Jack for VoIP project 20/03/07+++++*/
#ifdef VOIP_SUPPORT
#define MIB_VOIP_CFG_TBL			CHAIN_ENTRY_TBL_ID+35
#endif // VOIP_SUPPORT
/*-----end-----*/

#ifdef PARENTAL_CTRL
#define MIB_PARENTAL_CTRL_TBL			CHAIN_ENTRY_TBL_ID+36
#endif 

#ifdef TCP_UDP_CONN_LIMIT
#define MIB_TCP_UDP_CONN_LIMIT_TBL		CHAIN_ENTRY_TBL_ID+37
#endif //TCP_UDP_CONN_LIMIT


#ifdef WEB_REDIRECT_BY_MAC
#define MIB_WEB_REDIR_BY_MAC_TBL		CHAIN_ENTRY_TBL_ID+38
#endif
#define MULTI_ADDRESS_MAPPING_LIMIT_TBL		CHAIN_ENTRY_TBL_ID+39
#define MIB_PFW_ADVANCE_TBL			CHAIN_ENTRY_TBL_ID+40

//ql 20081119 for IP QoS traffic shaping
#ifdef NEW_IP_QOS_SUPPORT
#define MIB_IP_QOS_TC_TBL			CHAIN_ENTRY_TBL_ID+41
#endif

/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
#define MIB_DHCP_SERVER_OPTION_TBL		CHAIN_ENTRY_TBL_ID+42
#define MIB_DHCP_CLIENT_OPTION_TBL        	CHAIN_ENTRY_TBL_ID+43
#define MIB_DHCPS_SERVING_POOL_TBL		CHAIN_ENTRY_TBL_ID+44
#endif
/*ping_zhang:20080919 END*/

#ifdef WLAN_QoS
#define MIB_WLAN_QOS_AP_TBL		CHAIN_ENTRY_TBL_ID+45
#define MIB_WLAN_QOS_STA_TBL		CHAIN_ENTRY_TBL_ID+46
#endif

// Mason Yu. If we add a new chain record, please increase the "MIB_CHAIN_TBL_COUNT" value.
// And the MIB_CHAIN_TBL_COUNT is the total numbers of chain plus 1.
#define MIB_CHAIN_TBL_END			CHAIN_ENTRY_TBL_ID+47

#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
#define CWMP_CAPTIVEPORTAL_ALLOWED_LIST		CHAIN_ENTRY_TBL_ID+48
#endif

#ifdef _CWMP_MIB_ /*jiunming, mib for cwmp-tr069*/
#define CWMP_ID						500
#define CWMP_PROVISIONINGCODE				CWMP_ID + 1
#define CWMP_ACS_URL					CWMP_ID + 2
#define CWMP_ACS_USERNAME				CWMP_ID + 3
#define CWMP_ACS_PASSWORD				CWMP_ID + 4
#define CWMP_INFORM_ENABLE				CWMP_ID + 5
#define CWMP_INFORM_INTERVAL				CWMP_ID + 6
#define CWMP_INFORM_TIME				CWMP_ID + 7
#define CWMP_CONREQ_USERNAME				CWMP_ID + 8
#define CWMP_CONREQ_PASSWORD				CWMP_ID + 9
#define CWMP_ACS_UPGRADESMANAGED			CWMP_ID + 10
#define CWMP_LAN_CONFIGPASSWD				CWMP_ID + 11
#define CWMP_SERIALNUMBER				CWMP_ID + 12

#define CWMP_DHCP_SERVERCONF				CWMP_ID + 13
#define CWMP_LAN_IPIFENABLE				CWMP_ID + 14
#define CWMP_LAN_ETHIFENABLE				CWMP_ID + 15

#define CWMP_WLAN_BASICENCRY				CWMP_ID + 16
#define CWMP_WLAN_WPAENCRY				CWMP_ID + 17

#define CWMP_DL_COMMANDKEY				CWMP_ID + 18
#define CWMP_DL_STARTTIME				CWMP_ID + 19
#define CWMP_DL_COMPLETETIME				CWMP_ID + 20
#define CWMP_DL_FAULTCODE				CWMP_ID + 21

#define CWMP_INFORM_EVENTCODE				CWMP_ID + 22
#define CWMP_RB_COMMANDKEY				CWMP_ID + 23
#define CWMP_ACS_PARAMETERKEY				CWMP_ID + 24

#define CWMP_CERT_PASSWORD				CWMP_ID + 25

#define CWMP_FLAG					CWMP_ID + 26
#define CWMP_SI_COMMANDKEY				CWMP_ID + 27	/*ScheduleInform's commandkey*/

#ifdef _PRMT_USERINTERFACE_						/*InternetGatewayDevice.UserInterface.*/
#define UIF_PW_REQUIRED					CWMP_ID + 28 	/*PasswordRequired*/
#define UIF_PW_USER_SEL					CWMP_ID + 29	/*PasswordUserSelectable*/
#define UIF_UPGRADE					CWMP_ID + 30	/*UpgradeAvailable*/
#define UIF_WARRANTYDATE				CWMP_ID + 31	/*WarrantyDate*/
#define UIF_AUTOUPDATESERVER				CWMP_ID + 32	/*AutoUpdateServer*/
#define UIF_USERUPDATESERVER				CWMP_ID + 33	/*UserUpdateServer*/
#endif /*_PRMT_USERINTERFACE_*/

#ifdef _PRMT_X_CT_COM_IPTV_
#define CWMP_CT_IPTV_IGMPENABLE				CWMP_ID + 34
#define CWMP_CT_IPTV_STBNUMBER				CWMP_ID + 35
#endif

#ifdef _PRMT_X_CT_COM_RECON_
#define CWMP_CT_RECON_ENABLE				CWMP_ID + 36
#endif //_PRMT_X_CT_COM_RECON_

#ifdef _PRMT_X_CT_COM_MWBAND_
#define CWMP_CT_MWBAND_MODE				CWMP_ID + 37
#define CWMP_CT_MWBAND_NUMBER				CWMP_ID + 38
#define CWMP_CT_MWBAND_STB_ENABLE			CWMP_ID + 39
#define CWMP_CT_MWBAND_STB_NUM				CWMP_ID + 40
#define CWMP_CT_MWBAND_CMR_ENABLE			CWMP_ID + 41
#define CWMP_CT_MWBAND_CMR_NUM				CWMP_ID + 42 
#define CWMP_CT_MWBAND_PC_ENABLE			CWMP_ID + 43
#define CWMP_CT_MWBAND_PC_NUM				CWMP_ID + 44
#define CWMP_CT_MWBAND_PHN_ENABLE			CWMP_ID + 45
#define CWMP_CT_MWBAND_PHN_NUM				CWMP_ID + 46
#endif /*_PRMT_X_CT_COM_MWBAND_*/

#ifdef _PRMT_X_CT_COM_PORTALMNT_
#define CWMP_CT_PM_ENABLE				CWMP_ID + 47
#define CWMP_CT_PM_URL4PC				CWMP_ID + 48
#define CWMP_CT_PM_URL4STB				CWMP_ID + 49
#define CWMP_CT_PM_URL4MOBILE				CWMP_ID + 50
#endif /*_PRMT_X_CT_COM_PORTALMNT_*/


#if defined( _PRMT_X_CT_COM_DHCP_)||defined(IP_BASED_CLIENT_TYPE)
#define CWMP_CT_STB_MINADDR				CWMP_ID + 51
#define CWMP_CT_STB_MAXADDR				CWMP_ID + 52
#define CWMP_CT_PHN_MINADDR				CWMP_ID + 53
#define CWMP_CT_PHN_MAXADDR				CWMP_ID + 54
#define CWMP_CT_CMR_MINADDR				CWMP_ID + 55
#define CWMP_CT_CMR_MAXADDR				CWMP_ID + 56
#define CWMP_CT_PC_MINADDR				CWMP_ID + 57
#define CWMP_CT_PC_MAXADDR				CWMP_ID + 58
#define CWMP_CT_HGW_MINADDR             		CWMP_ID + 59
#define CWMP_CT_HGW_MAXADDR             		CWMP_ID + 60
#endif //_PRMT_X_CT_COM_DHCP_


#ifdef _PRMT_X_CT_COM_MONITOR_
#define CWMP_CT_MNT_ENABLE				CWMP_ID + 61
#define CWMP_CT_MNT_NUMBER				CWMP_ID + 62
#endif

#ifdef _PRMT_X_CT_COM_VPDN_
#define CWMP_CT_VPDN_ENABLE				CWMP_ID + 63
#endif

#define CWMP_ACS_KICKURL				CWMP_ID + 74
#define CWMP_ACS_DOWNLOADURL				CWMP_ID + 75
#define CWMP_CONREQ_PORT				CWMP_ID + 76 /*port for connection request*/
#define CWMP_CONREQ_PATH				CWMP_ID + 77 /*path for connection request*/
#define CWMP_FLAG2					CWMP_ID + 78 

//#ifdef _PRMT_TR143_
#define TR143_UDPECHO_ENABLE				CWMP_ID + 79
#define TR143_UDPECHO_ITFTYPE				CWMP_ID + 80
#define TR143_UDPECHO_SRCIP				CWMP_ID + 81
#define TR143_UDPECHO_PORT				CWMP_ID + 82
#define TR143_UDPECHO_PLUS				CWMP_ID + 83
//#endif //_PRMT_TR143_

#else /*_CWMP_MIB_*/
#define CWMP_ID						500
#if defined( _PRMT_X_CT_COM_DHCP_)||defined(IP_BASED_CLIENT_TYPE)
#define CWMP_CT_STB_MINADDR				CWMP_ID + 51
#define CWMP_CT_STB_MAXADDR				CWMP_ID + 52
#define CWMP_CT_PHN_MINADDR				CWMP_ID + 53
#define CWMP_CT_PHN_MAXADDR				CWMP_ID + 54
#define CWMP_CT_CMR_MINADDR				CWMP_ID + 55
#define CWMP_CT_CMR_MAXADDR				CWMP_ID + 56
#define CWMP_CT_PC_MINADDR				CWMP_ID + 57
#define CWMP_CT_PC_MAXADDR				CWMP_ID + 58
#define CWMP_CT_HGW_MINADDR             		CWMP_ID + 59
#define CWMP_CT_HGW_MAXADDR             		CWMP_ID + 60
#endif //_PRMT_X_CT_COM_DHCP_
#endif
/*
 * MIB value and constant
 */
#ifdef CONFIG_GUI_WEB
#define P_MAX_NAME_LEN					50
#endif
#define MAX_NAME_LEN					30
#define MAX_PPP_NAME_LEN				63
#define MAX_FILTER_NUM					20
#define MAX_VC_NUM					8
#define MAX_PPP_NUM					8
#define MAX_IFINDEX					7
#define COMMENT_LEN					20
#define IP_ADDR_LEN					4
#define MAC_ADDR_LEN					6
#define SNMP_STRING_LEN					64
#define MAX_VER_LEN					10

#ifdef QOS_DIFFSERV
#define MAX_QOS_RULE					20	// add 1 for diffserv rule
#else
#define MAX_QOS_RULE					16
#endif
#ifdef CONFIG_EXT_SWITCH
#ifdef CONFIG_USBCLIENT
#define SW_PORT_NUM					5
#else
#define SW_PORT_NUM					4
#endif
#define VLAN_NUM					5
#else // CONFIG_EXT_SWITCH
#ifdef CONFIG_RTL867X_COMBO_PORTMAPPING
#define SW_PORT_NUM					2
#else
#define SW_PORT_NUM					0
#endif
#endif

#define IF_DOMAIN_LAN					0x10
#define IF_DOMAIN_ELAN					0x20
#define IF_DOMAIN_WLAN					0x40
#define IF_DOMAIN_WAN					0x80

#define RIP_NONE					0
#define RIP_V1						1
#define RIP_V2						2
#define RIP_V1_V2					3
#define RIP_V1_COMPAT					4

//#ifdef WLAN_SUPPORT

#define MAX_SSID_LEN					33
#define WEP64_KEY_LEN					5
#define WEP128_KEY_LEN					13
#define MAX_CHAN_NUM					14

//#ifdef WLAN_WPA
#define MAX_PSK_LEN					64
#define MAX_RS_PASS_LEN					32
//#endif

#define TX_RATE_1M					0x01
#define TX_RATE_2M					0x02
#define TX_RATE_5M					0x04
#define TX_RATE_11M					0x08
                                        		
//#ifdef WLAN_8185AG                      		
#define TX_RATE_6M					0x10
#define TX_RATE_9M					0x20
#define TX_RATE_12M					0x40
#define TX_RATE_18M					0x80
#define TX_RATE_24M					0x100
#define TX_RATE_36M					0x200
#define TX_RATE_48M					0x400
#define TX_RATE_54M					0x800
//#endif                                  		
                                        		
#define MAX_WLAN_AC_NUM					20

#define MAXFNAME					60

//#ifdef WLAN_WDS
#define MAX_WDS_NUM					8
//#endif

#define MAX_STA_NUM					64	// max support sta number

/* flag of sta info */
#define STA_INFO_FLAG_AUTH_OPEN     			0x01
#define STA_INFO_FLAG_AUTH_WEP      			0x02
#define STA_INFO_FLAG_ASOC          			0x04
#define STA_INFO_FLAG_ASLEEP        			0x08

//#ifdef WLAN_WEB_REDIRECT //jiunming,web_redirect
#define MAX_URL_LEN					128
//#endif
//#endif // of WLAN_SUPPORT

// Added by Mason Yu for URL Blocking
#if defined(URL_BLOCKING_SUPPORT)||defined(URL_ALLOWING_SUPPORT)
#define MAX_URL_LENGTH					128
#define MAX_KEYWD_LENGTH				20
#endif

#ifdef DOMAIN_BLOCKING_SUPPORT
#define MAX_DOMAIN_LENGTH				120     // The value( 8 * 14=112) must be not less than 50.(because the domainblk.asp limit the value as 50.
#define MAX_DOMAIN_GROUP				8
#define MAX_DOMAIN_SUB_STRING				15
#endif

#define ENCAP_VCMUX					0
#define ENCAP_LLC					1

#define ADSL_BR1483		0
#define ADSL_MER1483		1
#define ADSL_PPPoE		2
#define ADSL_PPPoA		3
#define ADSL_RT1483		4
#ifdef CONFIG_ATM_CLIP
#define ADSL_RT1577		5
#endif

#define BRIDGE_ETHERNET		0
#define BRIDGE_PPPOE		1
#define BRIDGE_DISABLE		2


// ADSL MODE
#define ADSL_MODE_T1413		0x0001
#define ADSL_MODE_GDMT		0x0002
#define ADSL_MODE_GLITE		0x0008
#define ADSL_MODE_ADSL2		0x0010
#define ADSL_MODE_ANXL		0x0020
#define ADSL_MODE_ADSL2P	0x0040
#define ADSL_MODE_ANXM		0x0080
#define ADSL_MODE_ANXB		0x0100

#define ATM_MAX_US_PCR		6000

#ifdef _CWMP_MIB_ /*jiunming, mib for cwmp-tr069*/
/*flag for CWMP_FLAG setting*/
#define CWMP_FLAG_DEBUG_MSG	0x01
#define CWMP_FLAG_CERT_AUTH	0x02
#define CWMP_FLAG_SENDGETRPC	0x04
#define CWMP_FLAG_SKIPMREBOOT	0x08
#define CWMP_FLAG_DELAY		0x10
#define CWMP_FLAG_AUTORUN	0x20
#define CWMP_FLAG_CTINFORMEXT	0x40
#define CWMP_FLAG_SELFREBOOT    0x80
/*flag for CWMP_FLAG2 setting*/
#define CWMP_FLAG2_DIS_CONREQ_AUTH		0x01  /*disable connection request authentication*/
#define CWMP_FLAG2_DEFAULT_WANIP_IN_INFORM	0x02  /*bring the default wan ip in the inform*/

/*action type for applying new values*/
#define CWMP_NONE		0
#define CWMP_START		1
#define CWMP_STOP		2
#define CWMP_RESTART		3

/*EC_xxxxx event must consist with those defined in cwmp_rpc.h*/
#define EC_X_CT_COM_ACCOUNT	0x10000	/*X_CT-COM_ACCOUNTCHANGE*/

#ifdef _PRMT_X_CT_COM_WANEXT_
#define X_CT_SRV_TR069		0x01
#define X_CT_SRV_INTERNET	0x02
#define X_CT_SRV_OTHER		0x04
#endif //_PRMT_X_CT_COM_WANEXT_
#endif //_CWMP_MIB_

/*-- Macro declarations --*/
#define VC_INDEX(x)					(x & 0x0f)
#define PPP_INDEX(x)					(x  >> 4)

// Jenny, default gateway interface
#ifdef DEFAULT_GATEWAY_V2
#define DGW_NONE	0xff
#ifdef AUTO_PPPOE_ROUTE
#define DGW_AUTO	0xef
#endif
#endif

// Added by Mason Yu for ADSL Tone
#define MAX_ADSL_TONE		64           // Added by Mason Yu for correct Tone Mib Type
#define MAX_GAMING		8

// Mason Yu
#ifdef PORT_FORWARD_ADVANCE
typedef enum {PFW_VPN=0, PFW_GAME=1 } PFW_GATEGORY_T;
typedef enum {PFW_PPTP=0, PFW_L2TP=1 } PFW_RULE_T;
#endif
typedef enum { DHCP_DISABLED=0, DHCP_CLIENT=1, DHCP_SERVER=2, PPPOE=3 } DHCP_T;
typedef enum { DNS_AUTO=0, DNS_MANUAL } DNS_TYPE_T;
typedef enum { CONTINUOUS=0, CONNECT_ON_DEMAND, MANUAL } PPP_CONNECT_TYPE_T;
typedef enum { PPP_AUTH_AUTO=0, PPP_AUTH_PAP, PPP_AUTH_CHAP } PPP_AUTH_T;
typedef enum { PROTO_NONE=0, PROTO_TCP=1, PROTO_UDP=2, PROTO_ICMP=3, PROTO_UDPTCP } PROTO_TYPE_T;
typedef enum { DIR_OUT=0, DIR_IN } DIR_T;
typedef enum { DHCP_LAN_NONE=0, DHCP_LAN_RELAY=1, DHCP_LAN_SERVER=2 } DHCP_TYPE_T;
typedef enum { BOOT_LAST=0, BOOT_DEFAULT=1, BOOT_UPGRADE=2 } BOOT_TYPE_T;
#ifdef CONFIG_ATM_CLIP
typedef enum { ACC_BRIDGED=0, ACC_MER, ACC_PPPOE, ACC_PPPOA, ACC_ROUTED, ACC_IPOA } REMOTE_ACCESS_T;
#else
typedef enum { ACC_BRIDGED=0, ACC_MER, ACC_PPPOE, ACC_PPPOA, ACC_ROUTED } REMOTE_ACCESS_T;
#endif
typedef enum { ATMQOS_UBR=0, ATMQOS_CBR, ATMQOS_VBR_NRT, ATMQOS_VBR_RT } ATM_QOS_T;
//#ifdef CONFIG_EXT_SWITCH 
typedef enum { MP_NONE=0, MP_PORT_MAP=1, MP_VLAN=2, MP_IPQOS=3, MP_IGMPSNOOP=4 } MP_TYPE_T;
typedef enum { LINK_10HALF=0, LINK_10FULL, LINK_100HALF, LINK_100FULL, LINK_AUTO } LINK_TYPE_T;
//#endif 
typedef enum { PRIO_IP, PRIO_802_1p } PRIO_DOMAIN_T;
#ifdef ACCOUNT_CONFIG
typedef enum { PRIV_USER=0, PRIV_ENG=1, PRIV_ROOT=2 } ACC_PRIV_T;
#endif
typedef enum { RMT_FTP=0, RMT_TFTP, RMT_TELNET, RMT_PING, RMT_SNMP, RMT_WEB, RMT_HTTPS, RMT_SSH } REMACC_TYPE_T;


//#ifdef _PRMT_TR143_
typedef enum
{ 
	ITF_ALL=0,
	ITF_WAN,	//wan pppx or vcx
	ITF_LAN,	//br0
	
	ITF_ETH0,	//eth0
	ITF_ETH0_SW0,	//eth0_sw0
	ITF_ETH0_SW1,	//eth0_sw1
	ITF_ETH0_SW2,	//eth0_sw2
	ITF_ETH0_SW3,	//eth0_sw3
	
	ITF_WLAN0,	//wlan0
	ITF_WLAN0_VAP0,	//wlan0-vap0
	ITF_WLAN0_VAP1,	//wlan0-vap1
	ITF_WLAN0_VAP2,	//wlan0-vap2
	ITF_WLAN0_VAP3,	//wlan0-vap3

	ITF_USB0,	//usb0
	
	ITF_END		//last one
} ITF_T;
extern char *strItf[];
#define LANDEVNAME2BR0(a) do{ if(a && (strncmp(a, "eth0", 4)==0||strncmp(a, "wlan0", 5)==0||strncmp(a, "usb0", 4)==0)) strcpy(a, "br0"); }while(0)
//#endif //_PRMT_TR143_

// Mason Yu on True
#ifdef ADDRESS_MAPPING
typedef enum { ADSMAP_NONE=0, ADSMAP_ONE_TO_ONE=1, ADSMAP_MANY_TO_ONE=2, ADSMAP_MANY_TO_MANY=3, ADSMAP_ONE_TO_MANY=4 } ADSMAP_T;
typedef struct addressMap_ip {
	unsigned char lsip[16];
	unsigned char leip[16];
	unsigned char gsip[16];
	unsigned char geip[16];
	unsigned char srcRange[32];
	unsigned char globalRange[32];
} ADDRESSMAP_IP_T;
#endif

#ifdef WLAN_SUPPORT

#ifdef WLAN_WPA
typedef enum { ENCRYPT_DISABLED=0, ENCRYPT_WEP=1, ENCRYPT_WPA=2, ENCRYPT_WPA2=4, ENCRYPT_WPA2_MIXED=6 } ENCRYPT_T;
typedef enum { SUPP_NONWPA_NONE=0,SUPP_NONWPA_WEP=1,SUPP_NONWPA_1X=2} SUPP_NONWAP_T;
typedef enum { WPA_AUTH_AUTO=1, WPA_AUTH_PSK=2 } WPA_AUTH_T;
typedef enum { WPA_CIPHER_TKIP=1, WPA_CIPHER_AES=2 } WPA_CIPHER_T;
#ifdef ENABLE_WPAAES_WPA2TKIP
typedef enum { ENCRY_DISABLED=0, ENCRY_WEP=1, ENCRY_WPA_TKIP=2, ENCRY_WPA_AES=3, ENCRY_WPA2_AES=4, ENCRY_WPA2_TKIP=5, ENCRY_MIXED=6 } ENCRY_T;
#endif
#endif

typedef enum { WEP_DISABLED=0, WEP64=1, WEP128=2 } WEP_T;
typedef enum { KEY_ASCII=0, KEY_HEX } KEY_TYPE_T;
//modified by xl_yue
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
typedef enum { LONG_PREAMBLE=0, SHORT_PREAMBLE=1, AUTO_PREAMBLE=2} PREAMBLE_T;
#else
typedef enum { LONG_PREAMBLE=0, SHORT_PREAMBLE=1 } PREAMBLE_T;
#endif
typedef enum { AUTH_OPEN=0, AUTH_SHARED, AUTH_BOTH } AUTH_TYPE_T;
typedef enum { RF_INTERSIL=1, RF_RFMD=2, RF_PHILIP=3, RF_MAXIM=4, RF_GCT=5,
#ifdef WLAN_8185AG
 		RF_MAXIM_AG=6, RF_ZEBRA=7,
#endif
	     } RF_TYPE_T;
typedef enum { AP_MODE=0, CLIENT_MODE=1, WDS_MODE=2, AP_WDS_MODE=3 } WLAN_MODE_T;
typedef enum { INFRASTRUCTURE=0, ADHOC=1 } NETWORK_TYPE_T;

#ifdef WLAN_8185AG
typedef enum { BAND_11B=1, BAND_11G=2, BAND_11BG=3, BAND_11A=4 } BAND_TYPE_T;
#endif

/* WLAN sta info structure */
//tylo, should sync. with the struct in wlan driver
typedef struct wlan_sta_info {
        unsigned short  aid;
        unsigned char   addr[6];
        unsigned long   tx_packets;
        unsigned long   rx_packets;
	unsigned long	expired_time;  // 10 mini-sec
	unsigned short  flag;
        unsigned char   txOperaRates;
        unsigned char	rssi;	
        unsigned long	link_time;		// 1 sec unit	
        unsigned long	tx_fail;	
        unsigned long	tx_bytes;	
        unsigned long	rx_bytes;
#ifdef CONFIG_USB_RTL8192SU_SOFTAP        
        unsigned char network;
        unsigned char ht_info;	// bit0: 0=20M mode, 1=40M mode; bit1: 0=longGI, 1=shortGI
        unsigned char 	resv[6];
#else
	 unsigned char 	resv[8];
#endif
} WLAN_STA_INFO_T, *WLAN_STA_INFO_Tp;

#ifdef CONFIG_USB_RTL8192SU_SOFTAP
typedef struct wlan_rate{
unsigned int id;
unsigned char rate[20];
}WLAN_RATE_T, *WLAN_RATE_Tp;
typedef enum { 
	MCS0=0x80, 
	MCS1=0x81, 
	MCS2=0x82,
	MCS3=0x83,
	MCS4=0x84,
	MCS5=0x85,
	MCS6=0x86,
	MCS7=0x87,
	MCS8=0x88,
	MCS9=0x89,
	MCS10=0x8a,
	MCS11=0x8b,
	MCS12=0x8c,
	MCS13=0x8d,
	MCS14=0x8e,
	MCS15=0x8f
	} RATE_11N_T;
#endif

typedef struct macfilter_type {
	unsigned char macAddr[6] __PACK__;
	unsigned char comment[COMMENT_LEN] __PACK__;
} MACFILTER_T, *MACFILTER_Tp;

#ifdef WLAN_WDS
typedef MACFILTER_T WDS_T;
typedef MACFILTER_Tp WDS_Tp;
#endif


#endif // of WLAN_SUPPORT
/*
 * MIB struct
 */
typedef struct config_setting {
	//unsigned char version __PACK__;	// config_setting version
	// TCP/IP stuffs
	unsigned char ipAddr[IP_ADDR_LEN] __PACK__;
	unsigned char subnetMask[IP_ADDR_LEN] __PACK__;
	unsigned char enable_ip2 __PACK__; // 0 - disabled, 1 - enabled
	unsigned char ipAddr2[IP_ADDR_LEN] __PACK__;
	unsigned char subnetMask2[IP_ADDR_LEN] __PACK__;
	unsigned char defaultGateway[IP_ADDR_LEN] __PACK__;
#ifdef DEFAULT_GATEWAY_V2
	unsigned char dgwItf __PACK__; // Jenny, default gateway interface index
#endif
	unsigned char dhcp __PACK__; // DHCP flag, 0 - disabled, 1 - client, 2 - server
	unsigned char dhcp_pooluse __PACK__; // DHCP server pool uses on 0 - Primary LAN, 1 - Secondary LAN
	unsigned char dhcps_dns_opt __PACK__; // 0: use dnsRelay; 1: use manual setting
	unsigned char dhcps_dns1[IP_ADDR_LEN] __PACK__;
	unsigned char dhcps_dns2[IP_ADDR_LEN] __PACK__;
	unsigned char dhcps_dns3[IP_ADDR_LEN] __PACK__;
	unsigned char rip __PACK__; // RIP flag, 0 - disabled, 1 - enabled
	unsigned char dhcpClientStart __PACK__; // DHCP client start range
	unsigned char dhcpClientEnd __PACK__; // DHCP client end range
	unsigned char ipDhcpStart[IP_ADDR_LEN] __PACK__;
	unsigned char ipDhcpEnd[IP_ADDR_LEN] __PACK__;
	unsigned int dhcpLTime __PACK__; // DHCP server max lease time in seconds
	unsigned char dhcpDomain[MAX_NAME_LEN] __PACK__; // DHCP option Domain Name
	unsigned char lanAutoSearch __PACK__;		// the LAN ip auto-search

	// web server account
	unsigned char userName[MAX_NAME_LEN] __PACK__; // user name
	unsigned char userPassword[MAX_NAME_LEN] __PACK__; // user assword
	unsigned char deviceType __PACK__; // bridge: 0 or router: 1
	unsigned char initLine __PACK__; // init adsl line on startup
	unsigned char initScript __PACK__; // init system with user configuration on startup

	unsigned char wanDhcp __PACK__; // DHCP flag for WAN port, 0 - disabled, 1 - DHCP client
	unsigned char wanIpAddr[IP_ADDR_LEN] __PACK__;
	unsigned char wanSubnetMask[IP_ADDR_LEN] __PACK__;
	unsigned char wanDefaultGateway[IP_ADDR_LEN] __PACK__;
	unsigned char pppUserName[MAX_NAME_LEN] __PACK__;
	unsigned char pppPassword[MAX_NAME_LEN] __PACK__;
	DNS_TYPE_T dnsMode __PACK__;
	unsigned char dns1[IP_ADDR_LEN], dns2[IP_ADDR_LEN], dns3[IP_ADDR_LEN] __PACK__;
	unsigned char dhcps[IP_ADDR_LEN] __PACK__;
	unsigned char dhcpMode __PACK__; // 0 - None, 1 - DHCP Relay, 2 - DHCP Server
	unsigned short pppIdleTime __PACK__;
	unsigned char pppConnectType __PACK__;

	unsigned char adslConnectionMode __PACK__;
	unsigned char adslEncapMode __PACK__;
	unsigned short adslMode __PACK__;	// 1: ANSI T1.413, 2: G.dmt, 3: multi-mode, 4: ADSL2, 8: AnnexL, 16: ADSL2+
	unsigned char adslOlr __PACK__;	// adsl capability, 0: disable 1: bitswap 3: SRA & bitswap
	unsigned char ripEnabled __PACK__;
	unsigned char ripVer __PACK__;	// rip version. 0: v1, 1: v2, 2: v1 compatibility
#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
	unsigned char ospfEnabled __PACK__;	//ql_xu-- enable ospf
#endif

	unsigned char atmLoopback __PACK__;
	unsigned char atmMode __PACK__;
	unsigned char atmVcSwitch  __PACK__;
	unsigned char atmMac1[MAC_ADDR_LEN] __PACK__ ;
	unsigned char atmMac2[MAC_ADDR_LEN] __PACK__ ;
	unsigned char atmVcAutoSearch __PACK__;		// the very first pvc auto-search
	
	unsigned char ipfOutAction __PACK__; // 0 - Deny, 1 - Allow
	unsigned char ipfInAction __PACK__; // 0 - Deny, 1 - Allow
	unsigned char macfOutAction __PACK__; // 0 - Deny, 1 - Allow
	unsigned char macfInAction __PACK__; // 0 - Deny, 1 - Allow
	unsigned char portFwEnabled __PACK__;
#ifdef NATIP_FORWARDING
	unsigned char ipFwEnabled __PACK__;
#endif
	unsigned char dmzEnabled __PACK__;
	unsigned char dmzHost[IP_ADDR_LEN] __PACK__; // DMZ host
#if defined(CONFIG_USER_SNMPD_SNMPD_V2CTRAP) || defined(_CWMP_MIB_)
	unsigned char snmpSysDescr[SNMP_STRING_LEN] __PACK__;
#endif //defined(CONFIG_USER_SNMPD_SNMPD_V2CTRAP) || defined(_CWMP_MIB_)
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
	unsigned char snmpSysContact[SNMP_STRING_LEN] __PACK__;	
	unsigned char snmpSysLocation[SNMP_STRING_LEN] __PACK__;
	unsigned char snmpSysObjectID[SNMP_STRING_LEN] __PACK__;
	unsigned char snmpCommunityRO[SNMP_STRING_LEN] __PACK__;
	unsigned char snmpCommunityRW[SNMP_STRING_LEN] __PACK__;
	//unsigned char snmpTrapIpAddr[MAC_ADDR_LEN] __PACK__ ; // MAC address of LAN port in used
	unsigned char snmpTrapIpAddr[IP_ADDR_LEN] __PACK__ ; // MAC address of LAN port in used
#endif
	unsigned char snmpSysName[SNMP_STRING_LEN] __PACK__;
	
	// Kao
	unsigned short brctlAgeingTime __PACK__;
	unsigned char brctlStp __PACK__; // Spanning tree protocol flag, 0 - disabled, 1 - enabled
	unsigned char mpMode __PACK__; // multi-port admin status: port-mapping, vlan or ipqos
	unsigned char qosDomain __PACK__;	// default qos doamin: ip procedence bits or 802.1p
#ifdef QOS_DIFFSERV
	unsigned char qosDiffServ __PACK__;
	unsigned char phbClass __PACK__;	// PHB class - 1:AF1; 2:AF2; 3:AF3; 4:AF4; 5:EF
#endif
	/*ql:20081114 START: new IP QoS*/
#ifdef NEW_IP_QOS_SUPPORT
	unsigned int qosUprate __PACK__;
	unsigned char  qosPolicy __PACK__;// 1-gred; 0-other
	unsigned short TotalBandWidth __PACK__;
	unsigned char TotalBandWidthEn __PACK__; //ql- enable total bandwidth limit
	unsigned char qosmode __PACK__;// 1:qos priority  2:tc shaping
#endif
	/*ql:20081114 END*/
#ifdef CONFIG_USER_IGMPPROXY
	unsigned char igmpProxy __PACK__; // IGMP proxy flag, 0 - disabled, 1 - enabled
	unsigned char igmpProxyItf __PACK__; // IGMP proxy interface index
#endif
#ifdef IP_PASSTHROUGH
	unsigned char ipptItf __PACK__; // IP passthrough interface index
	unsigned int ipptLTime __PACK__; // IP passthrough max lease time in seconds
	unsigned char ipptLanacc __PACK__;	// enable LAN access
#endif
	unsigned char spcEnable __PACK__;	// enable single PC mode
	unsigned char spcIPType __PACK__;	// private IP or IP passthrough
	unsigned char aclcapability __PACK__;   // ACL capability flag, 0 - disabled, 1 - enabled. Mason Yu
	unsigned char connlimit	__PACK__;	//nat session number limit, 0 - disabled, 1 - enabled. ql_xu
#ifdef URL_BLOCKING_SUPPORT	
	unsigned char urlcapability __PACK__;   // URL capability flag, 0 - disabled, 1 - enabled. Mason Yu ; 2 for url allow
#endif
//#if defined(ZTE_GENERAL_ROUTER_SC) || defined(TIME_ZONE)
	unsigned char ntpEnabled __PACK__; // ntp client enabled
	unsigned char ntpServerId __PACK__; // ntp Server Index
	unsigned char ntpTimeZone[8] __PACK__; // ntp  Time Zone 
	unsigned char ntpServerIp1[IP_ADDR_LEN] __PACK__; // ntp  server ip address
	unsigned char ntpServerIp2[IP_ADDR_LEN] __PACK__; // ntp server ip address
	unsigned char ntpServerHost1[MAX_NAME_LEN] __PACK__; // ntp server host address
	unsigned char ntpServerHost2[MAX_NAME_LEN] __PACK__; // ntp server host address
//#endif
	unsigned char upnp __PACK__; 		// UPNP flag, 0 - disabled, 1 - enabled
	unsigned char upnpExtItf __PACK__;   	// UPNP Binded WAN interface index
	//unsigned char upnpNat __PACK__;	// UPNP NAT Traversal, 0 - disabled, 1-enabled
#ifdef DOMAIN_BLOCKING_SUPPORT	
	unsigned char domainblkcap __PACK__;    // Domain capability flag, 0 - disabled, 1 - enabled. Mason Yu
#endif
#ifdef CONFIG_IGMP_FORBID
         unsigned char igmpforbidEnable __PACK__;//igmp forbid capability flag,0 -disabled, 1 -enabled.alex_huang
#endif


#ifdef WLAN_SUPPORT
	// WLAN stuffs
	unsigned char ssid[MAX_SSID_LEN]__PACK__ ; // SSID
	unsigned char channel __PACK__ ;// current channel
//	unsigned char elanMacAddr[6] __PACK__ ; // Ethernet Lan MAC address
	unsigned char wlanMacAddr[6] __PACK__ ; // WLAN MAC address
	unsigned char wep __PACK__ ; // WEP flag, 0 - disabled, 1 - 64bits, 2 128 bits
	unsigned char wep64Key1[WEP64_KEY_LEN] __PACK__ ;
	unsigned char wep64Key2[WEP64_KEY_LEN] __PACK__ ;
	unsigned char wep64Key3[WEP64_KEY_LEN] __PACK__ ;
	unsigned char wep64Key4[WEP64_KEY_LEN] __PACK__ ;
	unsigned char wep128Key1[WEP128_KEY_LEN] __PACK__ ;
	unsigned char wep128Key2[WEP128_KEY_LEN] __PACK__ ;
	unsigned char wep128Key3[WEP128_KEY_LEN] __PACK__ ;
	unsigned char wep128Key4[WEP128_KEY_LEN] __PACK__ ;
	unsigned char wepDefaultKey __PACK__ ;
	unsigned char wepKeyType __PACK__ ;
	unsigned short fragThreshold __PACK__ ;
	unsigned short rtsThreshold __PACK__ ;
	unsigned short supportedRates __PACK__ ;
	unsigned short basicRates __PACK__ ;
	unsigned short beaconInterval __PACK__ ;
	unsigned char preambleType __PACK__; // preamble type, 0 - long preamble, 1 - short preamble
	unsigned char authType __PACK__; // authentication type, 0 - open-system, 1 - shared-key, 2 - both
#ifdef WLAN_ACL
	unsigned char acEnabled __PACK__; // enable/disable WLAN access control
#endif

	unsigned char hiddenSSID __PACK__ ;
	unsigned char wlanDisabled __PACK__; // enabled/disabled wlan interface
	unsigned char aliasName[MAX_NAME_LEN] __PACK__; // device logical name
	unsigned char txPower __PACK__; // TxPower 15/30/60 mW. Mason Yu
	unsigned long inactivityTime __PACK__; // wlan client inactivity time
	unsigned char rateAdaptiveEnabled __PACK__; // enable/disable rate adaptive
	unsigned char dtimPeriod __PACK__; // DTIM period
	unsigned char wlanMode __PACK__; // wireless mode - AP, Ethernet bridge 
	unsigned char networkType __PACK__; // adhoc or Infrastructure
#ifdef WLAN_IAPP
	unsigned char iappDisabled __PACK__; // disable IAPP
#endif

#ifdef WLAN_WPA
	unsigned char encrypt __PACK__; // encrypt type, defined as ENCRYPT_t
	unsigned char enableSuppNonWpa __PACK__; // enable/disable nonWPA client support
	unsigned char suppNonWpa __PACK__; // which kind of non-wpa client is supported (wep/1x)
	unsigned char wpaAuth __PACK__; // WPA authentication type (auto or psk)
	unsigned char wpaCipher __PACK__; // WPA unicast cipher suite
	unsigned char wpaPSK[MAX_PSK_LEN+1] __PACK__; // WPA pre-shared key
	unsigned long wpaGroupRekeyTime __PACK__; // group key rekey time in second
	unsigned char rsIpAddr[4] __PACK__; // radius server IP address
	unsigned short rsPort __PACK__; // radius server port number
	unsigned char rsPassword[MAX_PSK_LEN+1] __PACK__; // radius server password
	unsigned char enable1X __PACK__; // enable/disable 802.1x
	unsigned char wpaPSKFormat __PACK__; // PSK format 0 - passphrase, 1 - hex
	unsigned char accountRsEnabled __PACK__; // enable/disable accounting server
	unsigned char accountRsIpAddr[4] __PACK__; // accounting radius server IP address
	unsigned short accountRsPort __PACK__; // accounting radius server port number
	unsigned char accountRsPassword[MAX_RS_PASS_LEN] __PACK__; // accounting radius server password
	unsigned char accountRsUpdateEnabled __PACK__; // enable/disable accounting server update
	unsigned short accountRsUpdateDelay __PACK__; // account server update delay time in sec
	unsigned char macAuthEnabled __PACK__; // mac authentication enabled/disabled
	unsigned char rsMaxRetry __PACK__; // radius server max try
	unsigned short rsIntervalTime __PACK__; // radius server timeout
	unsigned char accountRsMaxRetry __PACK__; // accounting radius server max try
	unsigned short accountRsIntervalTime __PACK__; // accounting radius server timeout
	unsigned char wpa2Cipher __PACK__; // wpa2 Unicast cipher
#endif

#ifdef WLAN_WDS
	unsigned char wdsEnabled __PACK__; // wds enable/disable
	unsigned char wdsNum __PACK__; // number of wds entry existed
	WDS_T wdsArray[MAX_WDS_NUM] __PACK__; // wds array
	unsigned char wdsWep __PACK__ ; // WEP flag, 0 - disabled, 1 - 64bits, 2 128 bits
	unsigned char wdsWep64Key1[WEP64_KEY_LEN] __PACK__ ;
	unsigned char wdsWep64Key2[WEP64_KEY_LEN] __PACK__ ;
	unsigned char wdsWep64Key3[WEP64_KEY_LEN] __PACK__ ;
	unsigned char wdsWep64Key4[WEP64_KEY_LEN] __PACK__ ;
	unsigned char wdsWep128Key1[WEP128_KEY_LEN] __PACK__ ;
	unsigned char wdsWep128Key2[WEP128_KEY_LEN] __PACK__ ;
	unsigned char wdsWep128Key3[WEP128_KEY_LEN] __PACK__ ;
	unsigned char wdsWep128Key4[WEP128_KEY_LEN] __PACK__ ;
	unsigned char wdsWepDefaultKey __PACK__ ;
	unsigned char wdsWepKeyType __PACK__ ;
#endif

	unsigned char wlanPrivacyChk __PACK__; // enable/disable wlan privacy check
	unsigned char blockRelay __PACK__; // block/un-block the relay between wireless client
	unsigned char BlockEth2Wir __PACK__; // block/un-block ethernet to wireless
	unsigned char maccloneEnabled __PACK__; // enable NAT2.5 MAC Clone
	unsigned char itfGroup;		// wlan0 interface group

	#ifdef CONFIG_USB_RTL8192SU_SOFTAP  		//add by yq_zhou 2.10
	unsigned char wlanProtectionDisabled __PACK__;  
	unsigned char wlanAggregation __PACK__;
	unsigned char wlanShortGIEnabled __PACK__;
	unsigned char wlanBlockRelay __PACK__;
	unsigned char wlanContrlBand __PACK__;
	unsigned char wlanChannelWidth __PACK__;
	#endif 

#ifdef WLAN_WEB_REDIRECT //jiunming,web_redirect
	unsigned char WlanWebRedirURL[MAX_URL_LEN] __PACK__;     //url
#endif	
#ifdef WLAN_8185AG
	unsigned char wlanBand __PACK__; // wlan band, bit0-11B, bit1-11G, bit2-11A
#ifdef CONFIG_USB_RTL8192SU_SOFTAP
	unsigned int fixedTxRate __PACK__;
#else
	unsigned short fixedTxRate __PACK__; // fixed wlan tx rate, used when rate adaptive is disabled
#endif	
#ifdef CONFIG_USB_RTL8187SU_SOFTAP	
	unsigned short mlcstRate __PACK__;	//cathy, for multicast rate setting, 0:auto, bit 0-11 setting is same as fixedTxRate
#endif	
#endif
#ifdef WLAN_QoS
	unsigned char wlanqos	__PACK__;	//wlan QoS(WMM) switch
#endif
	unsigned char wlanDIG	__PACK__;	//wlan DIG (Dynamic Initial Gain) enable flag
	unsigned char wlanBcnAdvtisement	__PACK__; //wlan Beacon Advertisement
#endif
	// Jenny, system log
	unsigned char syslogslevel	__PACK__;	// system log log level
	unsigned char syslogdlevel	__PACK__;	// system log display level
#ifdef SYSLOG_REMOTE_LOG
	unsigned char syslogmode	__PACK__;	// system log mode
	unsigned char syslogServerIpAddr[IP_ADDR_LEN] __PACK__;     // remote syslog server IP address
	unsigned short syslogServerPort __PACK__; // remote syslog server port number
#endif
	// Added by Mason Yu for write superUser into Current Setting
	unsigned char suserName[MAX_NAME_LEN] __PACK__;     // user name
	unsigned char suserPassword[MAX_NAME_LEN] __PACK__; // user assword
	unsigned char adslTone[MAX_ADSL_TONE] __PACK__;      // ADSL Tone
	
	unsigned char DirectBridgeMode __PACK__; // ioctl for direct bridge mode, jiunming
	unsigned char adslHighInp __PACK__; // adsl high INP flag
	
#ifdef AUTO_PROVISIONING
	unsigned char configFileVersion[MAX_VER_LEN] __PACK__;     // Jenny, configuration file version
	unsigned char httpServerIpAddr[IP_ADDR_LEN] __PACK__;     // Jenny, http server IP for auto-provisioning
#endif
	unsigned char pppoeACMacAddr[MAC_ADDR_LEN] __PACK__ ;	// Jenny, PPPoE AC MAC address
	unsigned short pppoeSession __PACK__;	// Jenny,identifier for PPPoE session
	
#ifdef _CWMP_MIB_ /*jiunming, mib for cwmp-tr069*/
	unsigned char	cwmp_ProvisioningCode[64] __PACK__;
	unsigned char	cwmp_ACSURL[256+1] __PACK__;
	unsigned char	cwmp_ACSUserName[256+1] __PACK__;
	unsigned char	cwmp_ACSPassword[256+1] __PACK__;
	unsigned char	cwmp_InformEnable __PACK__;
	unsigned int	cwmp_InformInterval __PACK__;
	unsigned int	cwmp_InformTime __PACK__;
	unsigned char	cwmp_ConnReqUserName[256+1] __PACK__;
	unsigned char	cwmp_ConnReqPassword[256+1] __PACK__;
	unsigned char	cwmp_UpgradesManaged __PACK__;
	unsigned char	cwmp_LANConfPassword[64] __PACK__;
	unsigned char	cwmp_SerialNumber[64] __PACK__;
	unsigned char	cwmp_DHCP_ServerConf __PACK__;
	unsigned char	cwmp_LAN_IPIFEnable __PACK__;
	unsigned char	cwmp_LAN_EthIFEnable __PACK__;
	unsigned char	cwmp_WLAN_BasicEncry __PACK__; /*0:none, 1:Wep*/
	unsigned char	cwmp_WLAN_WPAEncry __PACK__; /*0:tkip, 1:aes, 2:tkip&aes*/
	unsigned char	cwmp_DL_CommandKey[32+1] __PACK__;
	unsigned int	cwmp_DL_StartTime __PACK__;
	unsigned int	cwmp_DL_CompleteTime __PACK__;
	unsigned int	cwmp_DL_FaultCode __PACK__;
	unsigned int	cwmp_Inform_EventCode __PACK__;
	unsigned char	cwmp_RB_CommandKey[32+1] __PACK__;
	unsigned char	cwmp_ACS_ParameterKey[32+1] __PACK__;
	unsigned char   cwmp_CERT_Password[64+1] __PACK__;
	unsigned char	cwmp_Flag __PACK__;
	unsigned char	cwmp_SI_CommandKey[32+1] __PACK__;
#ifdef _PRMT_USERINTERFACE_
	unsigned char	 UIF_PW_Required __PACK__;
	unsigned char	 UIF_PW_User_Sel __PACK__;
	unsigned char	 UIF_Upgrade __PACK__;
	unsigned int	 UIF_WarrantyDate __PACK__;
	unsigned char	 UIF_AutoUpdateServer[256] __PACK__;
	unsigned char	 UIF_UserUpdateServer[256] __PACK__;
#endif //_PRMT_USERINTERFACE_
#ifdef  _PRMT_X_CT_COM_IPTV_
	unsigned char	cwmp_CT_IPTV_IGMPEnable __PACK__;
	unsigned int	cwmp_CT_IPTV_STBNumber __PACK__;
#endif  //_PRMT_X_CT_COM_IPTV_
#ifdef _PRMT_X_CT_COM_RECON_
	unsigned char	cwmp_CT_ReCon_Enable __PACK__;
#endif //_PRMT_X_CT_COM_RECON_
#ifdef _PRMT_X_CT_COM_MWBAND_
	unsigned int	cwmp_CT_MWBAND_Mode __PACK__;
	unsigned char	cwmp_CT_MWBAND_STB_Enable __PACK__;
	unsigned char	cwmp_CT_MWBAND_CMR_Enable __PACK__;
	unsigned char	cwmp_CT_MWBAND_PC_Enable __PACK__;
	unsigned char	cwmp_CT_MWBAND_PHN_Enable __PACK__;
	unsigned int	cwmp_CT_MWBAND_Number __PACK__;
	unsigned int	cwmp_CT_MWBAND_STB_Num __PACK__;
	unsigned int	cwmp_CT_MWBAND_CMR_Num __PACK__;
	unsigned int	cwmp_CT_MWBAND_PC_Num __PACK__;
	unsigned int	cwmp_CT_MWBAND_PHN_Num __PACK__;
#endif
#ifdef _PRMT_X_CT_COM_PORTALMNT_
	unsigned char	cwmp_CT_PM_Enable __PACK__;
	unsigned char	cwmp_CT_PM_URL4PC[256] __PACK__;
	unsigned char	cwmp_CT_PM_URL4STB[256] __PACK__;
	unsigned char	cwmp_CT_PM_URL4MOBILE[256] __PACK__;
#endif

#if defined(_PRMT_X_CT_COM_DHCP_)||defined(IP_BASED_CLIENT_TYPE)
#if 1
	unsigned char	cwmp_CT_STB_MinAddr[IP_ADDR_LEN] __PACK__;
	unsigned char	cwmp_CT_STB_MaxAddr[IP_ADDR_LEN] __PACK__;
	unsigned char	cwmp_CT_PHN_MinAddr[IP_ADDR_LEN] __PACK__;
	unsigned char	cwmp_CT_PHN_MaxAddr[IP_ADDR_LEN] __PACK__;
	unsigned char	cwmp_CT_CMR_MinAddr[IP_ADDR_LEN] __PACK__;
	unsigned char	cwmp_CT_CMR_MaxAddr[IP_ADDR_LEN] __PACK__;
	unsigned char	cwmp_CT_PC_MinAddr[IP_ADDR_LEN] __PACK__;
	unsigned char	cwmp_CT_PC_MaxAddr[IP_ADDR_LEN] __PACK__;
	unsigned char	cwmp_CT_HGW_MinAddr[IP_ADDR_LEN] __PACK__;
	unsigned char	cwmp_CT_HGW_MaxAddr[IP_ADDR_LEN] __PACK__;
#else    
	unsigned char	cwmp_CT_STB_MinAddr __PACK__;
	unsigned char	cwmp_CT_STB_MaxAddr __PACK__;
	unsigned char	cwmp_CT_PHN_MinAddr __PACK__;
	unsigned char	cwmp_CT_PHN_MaxAddr __PACK__;
	unsigned char	cwmp_CT_CMR_MinAddr __PACK__;
	unsigned char	cwmp_CT_CMR_MaxAddr __PACK__;
	unsigned char	cwmp_CT_PC_MinAddr __PACK__;
	unsigned char	cwmp_CT_PC_MaxAddr __PACK__;
	unsigned char	cwmp_CT_HGW_MinAddr __PACK__;
	unsigned char	cwmp_CT_HGW_MaxAddr __PACK__;
#endif
#endif
#ifdef _PRMT_X_CT_COM_VPDN_
	unsigned char	cwmp_CT_VPDN_Enable __PACK__;
#endif //_PRMT_X_CT_COM_VPDN_
#ifdef _PRMT_X_CT_COM_MONITOR_
	unsigned char	cwmp_CT_MNT_Enable __PACK__;
	unsigned int	cwmp_CT_MNT_Number __PACK__;
#endif //_PRMT_X_CT_COM_MONITOR_
	unsigned char	cwmp_ACS_KickURL[64] __PACK__;
	unsigned char	cwmp_ACS_DownloadURL[64] __PACK__;
	unsigned int 	cwmp_ConnReqPort __PACK__;
	unsigned char 	cwmp_ConnReqPath[32] __PACK__;	
	unsigned char	cwmp_Flag2 __PACK__;
#ifdef _PRMT_TR143_
	unsigned char	tr143_udpecho_enable __PACK__;
	unsigned char	tr143_udpecho_itftype __PACK__;
	unsigned char	tr143_udpecho_srcip[4] __PACK__;
	unsigned short	tr143_udpecho_port __PACK__;
	unsigned char	tr143_udpecho_plus __PACK__;
#endif //_PRMT_TR143_
#else /*_CWMP_MIB_*/
#if defined(_PRMT_X_CT_COM_DHCP_)||defined(IP_BASED_CLIENT_TYPE)
#if 1
	unsigned char	cwmp_CT_STB_MinAddr[IP_ADDR_LEN] __PACK__;
	unsigned char	cwmp_CT_STB_MaxAddr[IP_ADDR_LEN] __PACK__;
	unsigned char	cwmp_CT_PHN_MinAddr[IP_ADDR_LEN] __PACK__;
	unsigned char	cwmp_CT_PHN_MaxAddr[IP_ADDR_LEN] __PACK__;
	unsigned char	cwmp_CT_CMR_MinAddr[IP_ADDR_LEN] __PACK__;
	unsigned char	cwmp_CT_CMR_MaxAddr[IP_ADDR_LEN] __PACK__;
	unsigned char	cwmp_CT_PC_MinAddr[IP_ADDR_LEN] __PACK__;
	unsigned char	cwmp_CT_PC_MaxAddr[IP_ADDR_LEN] __PACK__;
	unsigned char	cwmp_CT_HGW_MinAddr[IP_ADDR_LEN] __PACK__;
	unsigned char	cwmp_CT_HGW_MaxAddr[IP_ADDR_LEN] __PACK__;
#else  
	unsigned char	cwmp_CT_STB_MinAddr __PACK__;
	unsigned char	cwmp_CT_STB_MaxAddr __PACK__;
	unsigned char	cwmp_CT_PHN_MinAddr __PACK__;
	unsigned char	cwmp_CT_PHN_MaxAddr __PACK__;
	unsigned char	cwmp_CT_CMR_MinAddr __PACK__;
	unsigned char	cwmp_CT_CMR_MaxAddr __PACK__;
	unsigned char	cwmp_CT_PC_MinAddr __PACK__;
	unsigned char	cwmp_CT_PC_MaxAddr __PACK__;
	unsigned char	cwmp_CT_HGW_MinAddr __PACK__;
	unsigned char	cwmp_CT_HGW_MaxAddr __PACK__;
#endif
#endif
#endif
	unsigned char	syslog __PACK__; 
	unsigned int	maxmsglen __PACK__; 
	unsigned int	adsldbg __PACK__; 
	
#ifdef DOS_SUPPORT
	unsigned int dos_enable __PACK__;
	unsigned int dos_syssyn_flood __PACK__;
	unsigned int dos_sysfin_flood __PACK__;
	unsigned int dos_sysudp_flood __PACK__;
	unsigned int dos_sysicmp_flood __PACK__;
	unsigned int dos_pipsyn_flood __PACK__;
	unsigned int dos_pipfin_flood __PACK__;
	unsigned int dos_pipudp_flood __PACK__;
	unsigned int dos_pipicmp_flood __PACK__;
	unsigned int dos_block_time __PACK__;
#endif
	unsigned char	eth_mac_ctrl __PACK__;
	unsigned char	wlan_mac_ctrl __PACK__;
	unsigned char 	dhcp_GatewayAddr[IP_ADDR_LEN] __PACK__;
	#ifdef CONFIG_WIFI_SIMPLE_CONFIG// WPS def WIFI_SIMPLE_CONFIG
	#define PIN_LEN                                 8
	unsigned char wscDisable __PACK__;
	unsigned char wscMethod __PACK__;
	unsigned char wscConfigured __PACK__;
	unsigned char wscPin[PIN_LEN+1] __PACK__;
	unsigned char wscAuth __PACK__;
	unsigned char wscEnc __PACK__;
	unsigned char wscManualEnabled __PACK__;
	unsigned char wscUpnpEnabled __PACK__;
	unsigned char wscRegistrarEnabled __PACK__;	
	unsigned char wscSsid[MAX_SSID_LEN] __PACK__ ;
	unsigned char wscPsk[MAX_PSK_LEN+1] __PACK__;
	unsigned char wscConfigByExtReg __PACK__;
	#endif

#ifdef WLAN_MBSSID	
	unsigned char itfGroupVap0;		// wlan0-vap0 interface group
	unsigned char itfGroupVap1;		// wlan0-vap1 interface group
	unsigned char itfGroupVap2;		// wlan0-vap2 interface group
	unsigned char itfGroupVap3;		// wlan0-vap3 interface group
#endif
#ifdef CTC_TELECOM_ACCOUNT
	unsigned char CTCAccountEnable __PACK__;
#endif //CTC_TELECOM_ACCOUNT
//added by xl_yue
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	unsigned char WlanCountryOrArea __PACK__;
	unsigned char WlanRegDomain __PACK__;
#endif

//alex
#ifdef CONFIG_8021P_PRIO
#define NUM_PKT_PRIO 8
unsigned char set8021p_prio[NUM_PKT_PRIO] __PACK__;
#ifdef NEW_IP_QOS_SUPPORT
unsigned char setpred_prio[NUM_PKT_PRIO] __PACK__;
#endif
#endif
//add by ramen
#ifdef CONFIG_IP_NF_ALG_ONOFF
#ifdef CONFIG_IP_NF_FTP
unsigned char ipAlgFTP;
#endif
#ifdef CONFIG_IP_NF_H323
unsigned char ipAlgH323;
#endif
#ifdef CONFIG_IP_NF_IRC
unsigned char ipAlgIRC;
#endif
#ifdef CONFIG_IP_NF_RTSP
unsigned char ipAlgRTSP;
#endif
#ifdef CONFIG_IP_NF_QUAKE3
unsigned char ipAlgQUAKE3;
#endif
#ifdef CONFIG_IP_NF_CUSEEME
unsigned char ipAlgCUSEEME;
#endif
#ifdef CONFIG_IP_NF_L2TP
unsigned char ipAlgL2TP;
#endif
#ifdef CONFIG_IP_NF_IPSEC
unsigned char ipAlgIPSEC;
#endif
#ifdef CONFIG_IP_NF_SIP
unsigned char ipAlgSIP;
#endif
#ifdef CONFIG_IP_NF_PPTP
unsigned char ipAlgPPTP;
#endif
#endif
//add by ramen
#ifdef DNS_BIND_PVC_SUPPORT

unsigned char dns1PvcEnable;
unsigned char dns1PvcIfIndex;
unsigned char dns2PvcIfIndex;
unsigned char dns3PvcIfIndex;
#endif
//add by ramen
#ifdef QOS_SPEED_LIMIT_SUPPORT
unsigned int upbandwidth;
#endif

	unsigned char addressMapType __PACK__; // 
	unsigned char lsip[IP_ADDR_LEN] __PACK__;
	unsigned char leip[IP_ADDR_LEN] __PACK__;
	unsigned char gsip[IP_ADDR_LEN] __PACK__;
	unsigned char geip[IP_ADDR_LEN] __PACK__;
//ql
#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
	unsigned short rfTime __PACK__;
#endif
#ifdef MIB_WLAN_RESTART_SUPPORT
	unsigned char restartwlan;
#endif
	unsigned char snmpautorun;
	unsigned char cwmpautorun;

#ifndef CONFIG_EXT_SWITCH 
#ifdef ELAN_LINK_MODE_INTRENAL_PHY
	unsigned char ethMode; 
#endif
#endif 

#ifdef SYSLOG_SUPPORT
#ifdef SEND_LOG
	unsigned char logServer[IP_ADDR_LEN] __PACK__;
	unsigned char logNmae[MAX_NAME_LEN] __PACK__;     // user name
	unsigned char logPassword[MAX_NAME_LEN] __PACK__; // user assword
#endif	
#endif

//#ifdef QSETUP_WEB_REDIRECT
//	unsigned char startRedirect __PACK__;	// 1: redirect rule set
//#endif
        	unsigned char snmpdEnabled __PACK__;    // 0: disable, 1:enable
#ifdef TCP_UDP_CONN_LIMIT
	unsigned char connlimitEn __PACK__;	//connection limit enable, 0: disable, 1: enable	
	unsigned int connlimitUdp __PACK__;		//udp port numbers per user
	unsigned int connlimitTcp __PACK__;		//tcpp port numbers per user.
#endif

#ifdef WEB_REDIRECT_BY_MAC
	unsigned char WebRedirByMACURL[MAX_URL_LEN] __PACK__;     //url
	unsigned int landingPageTime __PACK__; // Time Interval of Landing Page in seconds
#endif	

#ifdef CONFIG_USB_ETH
	unsigned char usbeth_itfgrp __PACK__;		// usb0 interface group
#endif //CONFIG_USB_ETH
#ifdef CONFIG_RTL867X_COMBO_PORTMAPPING
	unsigned char ethitfGroup  __PACK__; 
#endif

//#ifdef DHCPS_POOL_COMPLETE_IP
	unsigned char dhcpSubnetMask[IP_ADDR_LEN] __PACK__;
//#endif

#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
	unsigned char captivePortalEnable __PACK__;
	char captivePortalURL[MAX_URL_LEN] __PACK__;
#endif

#ifdef WLAN_QoS
	unsigned char wlanqos_apsd	__PACK__;	//wlan QoS_APSD(WMM) switch
#endif

} MIB_T, *MIB_Tp;
#ifdef DNS_BIND_PVC_SUPPORT
#define ADDDNSROUTE 0
#define DELDNSROUTE 1
#endif
#ifdef CONFIG_WIFI_SIMPLE_CONFIG // WPS WIFI_SIMPLE_CONFIG
//enum { CONFIG_METHOD_ETH=0x2, CONFIG_METHOD_PIN=0x4, CONFIG_METHOD_PBC=0x80 };
enum { WSC_ENCRYPT_NONE=1, WSC_ENCRYPT_WEP=2, WSC_ENCRYPT_TKIP=4, WSC_ENCRYPT_AES=8, WSC_ENCRYPT_TKIPAES=12 };
enum { WSC_AUTH_OPEN=1, WSC_AUTH_WPAPSK=2, WSC_AUTH_SHARED=4, WSC_AUTH_WPA=8, WSC_AUTH_WPA2=0x10, WSC_AUTH_WPA2PSK=0x20, WSC_AUTH_WPA2PSKMIXED=0x22 };
#endif

typedef struct hw_config_setting {
	// Supervisor of web server account
	unsigned char superName[MAX_NAME_LEN] __PACK__ ; // supervisor name
	unsigned char superPassword[MAX_NAME_LEN] __PACK__; // supervisor assword
	unsigned char bootMode __PACK__; // 0 - last config, 1 - default config, 2 - upgrade config	
	unsigned char elanMacAddr[MAC_ADDR_LEN] __PACK__ ; // MAC address of ELAN port in used
	unsigned char wlanMacAddr[MAC_ADDR_LEN] __PACK__ ; // MAC address of WLAN port in used
//#if WLAN_SUPPORT
//#ifdef WLAN_8185AG
	unsigned char txPowerCCK[MAX_CHAN_NUM] __PACK__; // CCK Tx power for each channel
	unsigned char txPowerOFDM[MAX_CHAN_NUM] __PACK__; // OFDM Tx power for each channel
//#else	
	//unsigned char txPower[MAX_CHAN_NUM] __PACK__; // Tx power for each channel
//#endif
	unsigned char regDomain __PACK__; // regulation domain
	unsigned char rfType __PACK__; // RF module type
	unsigned char antDiversity __PACK__; // rx antenna diversity on/off
	unsigned char txAnt __PACK__; // select tx antenna, 0 - A, 1 - B
	unsigned char csThreshold __PACK__;
	unsigned char ccaMode __PACK__;	// 0, 1, 2
	unsigned char phyType __PACK__; // for Philip RF module only (0 - analog, 1 - digital)
	unsigned char ledType __PACK__; // LED type, see LED_TYPE_T for definition
//#endif // of WLAN_SUPPORT
	unsigned char	byte_test __PACK__;
	unsigned short word_test __PACK__;
	unsigned int dword_test __PACK__;
	int	int_test1 __PACK__;
	int	int_test2 __PACK__;	
//#ifdef WLAN_SUPPORT
	unsigned char wifi_support __PACK__;
//#endif
	unsigned char	serialNumber[64] __PACK__;
} HW_MIB_T, *HW_MIB_Tp;

typedef struct chain_record_header {
	unsigned short id __PACK__;
	unsigned int len __PACK__;
} MIB_CHAIN_RECORD_HDR_T, *MIB_CHAIN_RECORD_HDR_Tp;

typedef struct chain_entry {
	struct chain_entry	*pNext;
	unsigned char* pValue;
} MIB_CHAIN_ENTRY_T, *MIB_CHAIN_ENTRY_Tp;


/*
 * Flash File System 
 */
#define DEFAULT_SETTING_MIN_LEN		sizeof(MIB_T)
#define DEFAULT_SETTING_MAX_LEN		0x1000
#define CURRENT_SETTING_MIN_LEN		sizeof(MIB_T)
#ifndef COMPRESS_CURRENT_SETTING
#ifdef VOIP_SUPPORT
#define CURRENT_SETTING_MAX_LEN		0x4000
#else
#define CURRENT_SETTING_MAX_LEN		0x2000
#endif
#else
#define CURRENT_SETTING_MAX_LEN		0x4000
#define CURRENT_SETTING_MAX_REAL_LEN	0x2000
#define CURRENT_SETTING_LEN			0x4000
#endif
#define HW_SETTING_MIN_LEN		sizeof(HW_MIB_T)
#define HW_SETTING_MAX_LEN		0x1000

#ifdef EMBED
#define FLASH_DEVICE_NAME		("/dev/mtd")
#ifdef CONFIG_SPANSION_16M_FLASH
#define FLASH_BLOCK_SIZE		0x20000		// 128KB block
#define CODE_IMAGE_OFFSET		0x20000
//---
#define DEFAULT_SETTING_OFFSET		0x4000
//efine DEFAULT_SETTING_OFFSET		0x340000
#define HW_SETTING_OFFSET		DEFAULT_SETTING_OFFSET + DEFAULT_SETTING_MAX_LEN
#define CURRENT_SETTING_OFFSET		0x6000
//#define CURRENT_SETTING_OFFSET		0x360000
#define WEB_PAGE_OFFSET			0x1E0000
//#define WEB_PAGE_OFFSET			0x3E0000
#else
//#define FLASH_BLOCK_SIZE		0x10000		// 64KB block
#define FLASH_BLOCK_SIZE		0x1000		// 4KB block
#define CODE_IMAGE_OFFSET		0x10000
#define DEFAULT_SETTING_OFFSET		0x4000
#define HW_SETTING_OFFSET		DEFAULT_SETTING_OFFSET + DEFAULT_SETTING_MAX_LEN
#ifdef VOIP_SUPPORT
#define CURRENT_SETTING_OFFSET                0x3a0000
#else
#define CURRENT_SETTING_OFFSET		0x6000
#endif
#define WEB_PAGE_OFFSET			0x1E0000
#endif





#else
#define FLASH_DEVICE_NAME		("setting.bin")
#define HW_SETTING_OFFSET		0
#define DEFAULT_SETTING_OFFSET		HW_SETTING_OFFSET + HW_SETTING_MAX_LEN
#define CURRENT_SETTING_OFFSET		DEFAULT_SETTING_OFFSET + DEFAULT_SETTING_MAX_LEN
#define WEB_PAGE_OFFSET			CURRENT_SETTING_OFFSET + CURRENT_SETTING_MAX_LEN
#define CODE_IMAGE_OFFSET		WEB_PAGE_OFFSET + 0x10000
#endif





/*
 * Chain Record MIB struct
 */
typedef struct ipportfilter_entry {
	unsigned char action __PACK__; // 0 - Deny, 1 - Allow
	//unsigned char ipAddr[IP_ADDR_LEN] __PACK__;
	unsigned char srcIp[IP_ADDR_LEN] __PACK__;
	unsigned char dstIp[IP_ADDR_LEN] __PACK__;
	unsigned char smaskbit __PACK__;
	unsigned char dmaskbit __PACK__;
	//unsigned short fromPort __PACK__;
	unsigned short srcPortFrom __PACK__;
	unsigned short dstPortFrom __PACK__;
	//unsigned short toPort __PACK__;
	unsigned short srcPortTo __PACK__;
	unsigned short dstPortTo __PACK__;
	unsigned char dir __PACK__;
	//unsigned char portType __PACK__;
	unsigned char protoType __PACK__;
	//unsigned char comment[COMMENT_LEN] __PACK__;
} MIB_CE_IP_PORT_FILTER_T, *MIB_CE_IP_PORT_FILTER_Tp;
 

 /*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
#define DHCP_OPT_VAL_LEN 20

enum e_DHCPOpUsedFor
{
	eUsedFor_DHCPServer = 1,
	eUsedFor_DHCPServer_ServingPool,
	eUsedFor_DHCPClient_Sent,
	eUsedFor_DHCPClient_Req
};
	
typedef struct dhcpoption_entry {
	unsigned char enable __PACK__; 
	unsigned char usedFor __PACK__;
	unsigned int order __PACK__;
	unsigned int tag __PACK__;
	unsigned char len __PACK__;//ql add: recored the len of value
	unsigned char value[DHCP_OPT_VAL_LEN] __PACK__;
	unsigned char ifIndex __PACK__;
	unsigned int dhcpOptInstNum __PACK__;
	unsigned int dhcpConSPInstNum __PACK__;
} MIB_CE_DHCP_OPTION_T, *MIB_CE_DHCP_OPTION_Tp;
#endif
/*ping_zhang:20080919 END*/

typedef struct portfw_entry {
	//unsigned char action __PACK__; // 0 - Deny, 1 - Allow
	unsigned char ipAddr[IP_ADDR_LEN] __PACK__; /*TR-069:internalclient*/
	unsigned short fromPort __PACK__; /*TR-069:internalport*/
	unsigned short toPort __PACK__; /*TR-069:internalport*/
	unsigned char protoType __PACK__;/*TR-069:protocol*/
	unsigned char comment[COMMENT_LEN] __PACK__;
//#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
	unsigned char ifIndex __PACK__;/*0xff: no specific, interface name, refer to "struct atmvc_entry"*/
	unsigned char enable __PACK__;
	unsigned int  leaseduration __PACK__;/*0:static*/
	unsigned char remotehost[IP_ADDR_LEN] __PACK__;
//	unsigned int  externalport __PACK__;
	unsigned int  externalfromport __PACK__;
	unsigned int  externaltoport __PACK__;
	//unsigned int  internalport __PACK__;
	//unsigned char protocol __PACK__;/*0:tcp, 1:udp*/
	//unsigned char internalclient[IP_ADDR_LEN] __PACK__;
	unsigned int  InstanceNum __PACK__;
//#endif
} MIB_CE_PORT_FW_T, *MIB_CE_PORT_FW_Tp;

#ifdef PORT_FORWARD_ADVANCE
typedef struct ipfw_advance_entry {	
	unsigned char ipAddr[IP_ADDR_LEN] __PACK__; 	
	unsigned char ifIndex __PACK__;/*0xff: no specific, interface name, refer to "struct atmvc_entry"*/
	unsigned char gategory __PACK__;
	unsigned int rule __PACK__;
} MIB_CE_PORT_FW_ADVANCE_T, *MIB_CE_PORT_FW_ADVANCE_Tp;
#endif

#ifdef ZTE_GENERAL_ROUTER_SC
typedef struct vtlsvr_entry {
	unsigned char svrIpAddr[IP_ADDR_LEN] __PACK__;	//server ip
	unsigned char svrName[16] __PACK__;	//server name
	unsigned short wanStartPort __PACK__;		//wan comm start port
	unsigned short wanEndPort __PACK__;	//wan comm end port
	unsigned short svrStartPort __PACK__;		//local server start port
	unsigned short svrEndPort __PACK__;	//local server end port
	unsigned char protoType __PACK__;	//protocol(TCP/UDP)
}MIB_CE_VTL_SVR_T, *MIB_CE_VTL_SVR_Tp;
#endif
#ifdef NATIP_FORWARDING
typedef struct ipfw_entry {
	unsigned char action __PACK__; // 0 - Deny, 1 - Allow
	unsigned char local_ip[IP_ADDR_LEN] __PACK__;
	unsigned char remote_ip[IP_ADDR_LEN] __PACK__;
} MIB_CE_IP_FW_T, *MIB_CE_IP_FW_Tp;
#endif

#ifdef PORT_TRIGGERING
#define GAMING_MAX_RANGE 32
typedef struct porttrg_entry {
	unsigned char	name[32] __PACK__;		// name
	unsigned char	ip[IP_ADDR_LEN] __PACK__;
	unsigned char	tcpRange[GAMING_MAX_RANGE] __PACK__;
	unsigned char	udpRange[GAMING_MAX_RANGE] __PACK__;
	unsigned char	enable __PACK__;			// enable
} MIB_CE_PORT_TRG_T, *MIB_CE_PORT_TRG_Tp;
#endif

typedef struct macfilter_entry {
	unsigned char action __PACK__; // 0 - Deny, 1 - Allow
	//unsigned char macAddr[MAC_ADDR_LEN] __PACK__;
	unsigned char srcMac[MAC_ADDR_LEN] __PACK__;
	unsigned char dstMac[MAC_ADDR_LEN] __PACK__;
	unsigned char comment[COMMENT_LEN] __PACK__;
	// Added by Mason Yu for Incoming MAC filtering
	unsigned char dir __PACK__;
} MIB_CE_MAC_FILTER_T, *MIB_CE_MAC_FILTER_Tp;

typedef struct atmvc_entry {
	unsigned char ifIndex __PACK__;	// high nibble for PPP, low nibble for mpoa
	unsigned char vpi __PACK__;
	unsigned char qos __PACK__;
	unsigned short vci __PACK__;
	unsigned short pcr __PACK__;
	unsigned short scr __PACK__;
	unsigned short mbs __PACK__;
	unsigned int cdvt __PACK__;
	unsigned char encap __PACK__;
	unsigned char napt __PACK__;
	unsigned char cmode __PACK__;
	unsigned char brmode __PACK__;	// 0: transparent bridging, 1: PPPoE bridging
#ifdef CONFIG_GUI_WEB
	unsigned char pppUsername[P_MAX_NAME_LEN] __PACK__;
	unsigned char pppPassword[P_MAX_NAME_LEN] __PACK__;
#else
	unsigned char pppUsername[MAX_PPP_NAME_LEN+1] __PACK__;
	unsigned char pppPassword[MAX_NAME_LEN] __PACK__;
#endif
	unsigned char pppAuth __PACK__;	// 0:AUTO, 1:PAP, 2:CHAP
	unsigned char pppACName[MAX_NAME_LEN] __PACK__;
	unsigned char pppCtype __PACK__;
	unsigned short pppIdleTime __PACK__;
	unsigned char ipDhcp __PACK__;
	unsigned char rip __PACK__;
	unsigned char ipAddr[IP_ADDR_LEN] __PACK__;
	unsigned char remoteIpAddr[IP_ADDR_LEN] __PACK__;
	unsigned char dgw __PACK__;
	unsigned int mtu __PACK__;
	unsigned char enable __PACK__;
	unsigned char netMask[IP_ADDR_LEN] __PACK__;	// Jenny; Subnet mask
	unsigned char ipunnumbered __PACK__;	// Jenny, unnumbered(1)
#if defined( CONFIG_EXT_SWITCH) || defined(CONFIG_RTL867X_COMBO_PORTMAPPING)
	// used for VLAN mapping
	unsigned char vlan __PACK__;
	unsigned short vid __PACK__;
	unsigned short vprio __PACK__;	// 802.1p priority bits
	unsigned char vpass __PACK__;	// vlan passthrough
	// used for interface group
	unsigned char itfGroup;
#endif
        unsigned long cpePppIfIndex;   // Mason Yu. Remote Management
        unsigned long cpeIpIndex;      // Mason Yu. Remote Management
        
#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
	unsigned char connDisable __PACK__; //0:enable, 1:disable
	unsigned int ConDevInstNum __PACK__;
	unsigned int ConIPInstNum __PACK__;
	unsigned int ConPPPInstNum __PACK__;
	unsigned short autoDisTime __PACK__;	// Jenny, TR-069 PPP AutoDisconnectTime
	unsigned short warnDisDelay __PACK__;	// Jenny, TR-069 PPP WarnDisconnectDelay
	unsigned char pppServiceName[MAX_NAME_LEN] __PACK__;	// Jenny, TR-069 PPPoEServiceName
	unsigned char WanName[MAX_NAME_LEN] __PACK__;	//Name of this wan connection
#ifdef _PRMT_TR143_
	unsigned char TR143UDPEchoItf __PACK__;
#endif //_PRMT_TR143_
#ifdef _PRMT_X_CT_COM_PPPOEv2_
	unsigned char PPPoEProxyEnable __PACK__;
	unsigned int  PPPoEProxyMaxUser __PACK__;
#endif //_PRMT_X_CT_COM_PPPOEv2_
#ifdef _PRMT_X_CT_COM_WANEXT_
	unsigned short ServiceList __PACK__;
#endif //_PRMT_X_CT_COM_WANEXT_
#endif //_CWMP_MIB_
#ifdef CTC_WAN_NAME
	unsigned int applicationtype __PACK__;  //TR069, INTERNET, TR069_INTERNET, Other
	//char applicationname[MAX_NAME_LEN] __PACK__;
#endif
#ifdef CONFIG_SPPPD_STATICIP
	unsigned char pppIp __PACK__;	// Jenny, static PPPoE
#endif

} MIB_CE_ATM_VC_T, *MIB_CE_ATM_VC_Tp;

/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
#define RTF_EMPTY 0x0001 /*empty route new added by tr069*/
#endif
/*ping_zhang:20081217 END*/
typedef struct iproute_entry {
	unsigned char destID[IP_ADDR_LEN] __PACK__; // TR-069: DestIP
	unsigned char netMask[IP_ADDR_LEN] __PACK__; // TR-069: DestMask
	unsigned char nextHop[IP_ADDR_LEN] __PACK__; // TR-069: GatewayIP
//#ifdef _CWMP_MIB_ /*jiunming, mib for cwmp-tr069*/
	unsigned char Enable __PACK__;
	unsigned char Type __PACK__; /*0:network, 1:host, 2:default*/
	unsigned char SourceIP[IP_ADDR_LEN] __PACK__;
	unsigned char SourceMask[IP_ADDR_LEN] __PACK__;
	unsigned char ifIndex __PACK__; // 0xff: no specific interface
	int	      FWMetric __PACK__;
	unsigned int  InstanceNum __PACK__;
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	unsigned char Flags __PACK__;
#endif
/*ping_zhang:20081217 END*/
//#endif
} MIB_CE_IP_ROUTE_T, *MIB_CE_IP_ROUTE_Tp;


typedef struct aclip_entry {
#ifdef ACL_IP_RANGE
	unsigned char startipAddr[IP_ADDR_LEN] __PACK__;
	unsigned char endipAddr[IP_ADDR_LEN] __PACK__;
#endif
	unsigned char ipAddr[IP_ADDR_LEN] __PACK__;
	unsigned char maskbit __PACK__;
	unsigned char Enabled __PACK__;   
	unsigned char Interface __PACK__;   	
} MIB_CE_ACL_IP_T, *MIB_CE_ACL_IP_Tp;

//ql_xu add:
#ifdef MAC_ACL
typedef struct aclmac_entry {
	unsigned char macAddr[MAC_ADDR_LEN] __PACK__;
	unsigned char Enabled __PACK__;   
	unsigned char Interface __PACK__;   	
} MIB_CE_ACL_MAC_T, *MIB_CE_ACL_MAC_Tp;
#endif
#ifdef NAT_CONN_LIMIT
typedef struct connlimit_entry {
	unsigned char ipAddr[IP_ADDR_LEN] __PACK__;
	unsigned char Enabled __PACK__;
	unsigned int connNum __PACK__;
} MIB_CE_CONN_LIMIT_T, *MIB_CE_CONN_LIMIT_Tp;
#endif
#ifdef TCP_UDP_CONN_LIMIT
typedef struct tcp_udp_connlimit_entry {
	unsigned char ipAddr[IP_ADDR_LEN] __PACK__;
	unsigned char Enabled __PACK__;
	unsigned char protocol __PACK__;	//0: TCP, 1:UDP
	unsigned int connNum __PACK__;
} MIB_CE_TCP_UDP_CONN_LIMIT_T, *MIB_CE_TCP_UDP_CONN_LIMIT_Tp;
#endif // TCP_UDP_CONN_LIMIT

typedef struct multi_addr_mapping_entry {
	unsigned char addressMapType __PACK__; // 
	unsigned char lsip[IP_ADDR_LEN] __PACK__;
	unsigned char leip[IP_ADDR_LEN] __PACK__;
	unsigned char gsip[IP_ADDR_LEN] __PACK__;
	unsigned char geip[IP_ADDR_LEN] __PACK__;
} MIB_CE_MULTI_ADDR_MAPPING_LIMIT_T, *MIB_CE_MULTI_ADDR_MAPPING_LIMIT_Tp;

#ifdef URL_BLOCKING_SUPPORT
typedef struct urlfqdn_entry {
	unsigned char fqdn[MAX_URL_LENGTH] __PACK__;	
} MIB_CE_URL_FQDN_T, *MIB_CE_URL_FQDN_Tp;
#endif

#ifdef URL_ALLOWING_SUPPORT
typedef struct urlalwfqdn_entry {
	unsigned char fqdn[MAX_URL_LENGTH] __PACK__;	
} MIB_CE_URL_ALLOW_FQDN_T, *MIB_CE_URL_ALLOW_FQDN_Tp;
#endif

#ifdef URL_BLOCKING_SUPPORT
typedef struct keywdfilter_entry {
	unsigned char keyword[MAX_KEYWD_LENGTH] __PACK__;	
} MIB_CE_KEYWD_FILTER_T, *MIB_CE_KEYWD_FILTER_Tp;
#endif

#ifdef DOMAIN_BLOCKING_SUPPORT
typedef struct domainblk_entry {
	unsigned char domain[MAX_DOMAIN_LENGTH] __PACK__;	
} MIB_CE_DOMAIN_BLOCKING_T, *MIB_CE_DOMAIN_BLOCKING_Tp;
#endif

typedef struct rip_entry {
	unsigned char ifIndex __PACK__;	
	unsigned char receiveMode __PACK__;   
	unsigned char sendMode __PACK__;   	
} MIB_CE_RIP_T, *MIB_CE_RIP_Tp;

#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
typedef struct ospf_entry {
	unsigned char ipAddr[IP_ADDR_LEN] __PACK__;
	unsigned char netMask[IP_ADDR_LEN] __PACK__;
}MIB_CE_OSPF_T, *MIB_CE_OSPF_Tp;
#endif


#ifdef WLAN_MBSSID
typedef struct mbssid_entry {
	unsigned char idx;              // 0:root AP, 1: vap0, 2:vap1, 3:vap2, 4: vap3
	unsigned char encrypt __PACK__; // encrypt type, defined as ENCRYPT_t
	unsigned char enable1X __PACK__; // enable/disable 802.1x
	unsigned char wep __PACK__ ; // WEP flag, 0 - disabled, 1 - 64bits, 2 128 bits
	unsigned char wpaAuth __PACK__; // WPA authentication type (auto or psk)
	unsigned char wpaPSKFormat __PACK__; // PSK format 0 - passphrase, 1 - hex
	unsigned char wpaPSK[MAX_PSK_LEN+1] __PACK__; // WPA pre-shared key
	unsigned short rsPort __PACK__; // radius server port number
	unsigned char rsIpAddr[IP_ADDR_LEN] __PACK__; // radius server IP address
	unsigned char rsPassword[MAX_PSK_LEN+1] __PACK__; // radius server password
	unsigned char wlanDisabled __PACK__; // enabled/disabled wlan(include VAP0~3) interface. 0:enable, 1:disable
	unsigned char ssid[MAX_SSID_LEN]__PACK__ ; // SSID
	unsigned char authType __PACK__; // authentication type, 0 - open-system, 1 - shared-key, 2 - both		
#ifdef _CWMP_MIB_ /*jiunming, mib for cwmp-tr069*/
	/*for root ap, ssid and authType is not used => use MIB_WLAN_SSID and MIB_WLAN_AUTH_TYPE*/
	unsigned char cwmp_WLAN_BasicEncry __PACK__; /*0:none, 1:Wep*/
#endif
//added by xl_yue:for supporting WPA(AES) and WPA(TKIP)
#ifdef ENABLE_WPAAES_WPA2TKIP
	unsigned char unicastCipher __PACK__;	// 1: TKIP, 2: AES
	unsigned char wpa2UnicastCipher __PACK__;	// 1: TKIP, 2: AES
#endif
	unsigned char bcnAdvtisement __PACK__; //0: ap does not send out beacons, 1: ap sends out beacons 
	unsigned char hidessid __PACK__; //0: beacon include the SSID name, 1: beacon does not include the SSID name
} MIB_CE_MBSSIB_T, *MIB_CE_MBSSIB_Tp;

typedef struct mbssid_wep_entry {
	unsigned char wep64Key1[WEP64_KEY_LEN+1] __PACK__ ;    // 3131313131: key is 11111
	unsigned char wep64Key2[WEP64_KEY_LEN+1] __PACK__ ;    // 3232323232: key is 22222
	unsigned char wep64Key3[WEP64_KEY_LEN+1] __PACK__ ;    // 3333333333: key is 33333	 
	unsigned char wep64Key4[WEP64_KEY_LEN+1] __PACK__ ;    // 3434343434: key is 44444
	unsigned char wep128Key1[WEP128_KEY_LEN+1] __PACK__ ;
	unsigned char wep128Key2[WEP128_KEY_LEN+1] __PACK__ ;
	unsigned char wep128Key3[WEP128_KEY_LEN+1] __PACK__ ;
	unsigned char wep128Key4[WEP128_KEY_LEN+1] __PACK__ ;
	unsigned char wepDefaultKey __PACK__ ;                  // 0:key1, 1:key2, 2:key3, 3:key4
	unsigned char wepKeyType __PACK__ ;                     // 1:Hex, 0:ASCII	
} MIB_CE_MBSSIB_WEP_T, *MIB_CE_MBSSIB_WEP_Tp;
#endif

#ifdef WLAN_SUPPORT
#ifdef WLAN_ACL
typedef struct wlac_entry {
	unsigned char macAddr[MAC_ADDR_LEN] __PACK__;
	//unsigned char comment[COMMENT_LEN] __PACK__;
} MIB_CE_WLAN_AC_T, *MIB_CE_WLAN_AC_Tp;
#endif
#endif

#ifdef CONFIG_RTL867X_COMBO_PORTMAPPING
typedef struct swport_entry {
	// used for Ethernet to PVC mapping
	unsigned char pvcItf;
	unsigned char itfGroup;
	// used for VLAN config
	unsigned char pvid;
	unsigned char egressTagAction;
	// used for Link Mode setting
	unsigned char linkMode;	// 10/100 half/full
} MIB_CE_SW_PORT_T, *MIB_CE_SW_PORT_Tp;

#endif
#ifdef CONFIG_EXT_SWITCH
typedef struct swport_entry {
	// used for Ethernet to PVC mapping
	unsigned char pvcItf;
	unsigned char itfGroup;
	// used for VLAN config
	unsigned char pvid;
	unsigned char egressTagAction;
	// used for Link Mode setting
	unsigned char linkMode;	// 10/100 half/full
} MIB_CE_SW_PORT_T, *MIB_CE_SW_PORT_Tp;

typedef struct vlan_entry {
	// | resvd | vc7 vc6 vc5 vc4 | vc3 vc2 vc1 vc0 | ...
	//  vap3 | vap2 vap1 vap0 wlan0 | resvd | LAN4 LAN3 LAN2 LAN1
	//unsigned char member;
	unsigned int member;
	unsigned short tag;
	unsigned char dhcps;
} MIB_CE_VLAN_T, *MIB_CE_VLAN_Tp;
#endif

//ql 20081119 for traffic shaping
#ifdef NEW_IP_QOS_SUPPORT
typedef struct ipqos_tc_entry {
    unsigned char  entryid     __PACK__;// id of this rules
    unsigned char  ifIndex     __PACK__;
    unsigned int   srcip       __PACK__;
    unsigned char  smaskbits   __PACK__;//source subnetmask bit number,default 0
    unsigned int   dstip       __PACK__;
    unsigned char  dmaskbits   __PACK__;
    unsigned short sport       __PACK__;
    unsigned short dport       __PACK__;
    unsigned char  protoType   __PACK__;//0-none, 1-ICMP, 2-TCP, 3-UDP, 4-TCP/UDP
    unsigned int   limitSpeed  __PACK__;
}MIB_CE_IP_TC_T, *MIB_CE_IP_TC_Tp;
#endif

typedef struct ipqos_entry {
	unsigned char sip[IP_ADDR_LEN] __PACK__;
	unsigned char smaskbit __PACK__;
	unsigned short sPort __PACK__;
	unsigned char dip[IP_ADDR_LEN] __PACK__;
	unsigned char dmaskbit __PACK__;
	unsigned short dPort __PACK__;
	unsigned char protoType __PACK__;
	unsigned char phyPort __PACK__;
//#ifdef NEW_IP_QOS_SUPPORT
#if defined(NEW_IP_QOS_SUPPORT) || defined(QOS_DIFFSERV)
	unsigned char qosDscp __PACK__;//ql stream based on DSCP
#endif
#ifdef NEW_IP_QOS_SUPPORT
	unsigned char vlan1p __PACK__;	//ql stream based on 802.1p
#endif
	unsigned char outif __PACK__;	// outbound interface
	unsigned char prior __PACK__;	// assign to priority queue
#ifdef QOS_DSCP
	unsigned char dscp __PACK__;	// Jenny, DSCP enable flag: 1: DSCP, 0: TOS
#endif
	unsigned char m_ipprio __PACK__;	// mark IP precedence
	unsigned char m_iptos __PACK__;	// mark IP Type of Service
	unsigned char m_1p __PACK__;	// mark 802.1p: 0: none, 1: prio 0, 2: prio 1, ...
#ifdef NEW_IP_QOS_SUPPORT
	unsigned char m_dscp __PACK__;//ql
#endif
	unsigned char enable __PACK__;
#ifdef _CWMP_MIB_ /*jiunming, mib for cwmp-tr069*/
	unsigned int  InstanceNum __PACK__;
#endif
	
#ifdef QOS_SPEED_LIMIT_SUPPORT
	unsigned char limitSpeedEnabled;
	unsigned char limitSpeedRank;//rank more little,speed more high
#endif
#ifdef QOS_DIFFSERV
	unsigned char ifIndex __PACK__;
	unsigned char enDiffserv __PACK__;	// for diffserv chain if this flag is set to 1
	unsigned short totalBandwidth __PACK__;
	unsigned short htbRate __PACK__;	// bandwidth division rate for HTB
	unsigned int latency  __PACK__;
	unsigned int limitSpeed  __PACK__;	// police rate limit
	unsigned char policing __PACK__;	// 1:drop; 2:continue
#endif
} MIB_CE_IP_QOS_T, *MIB_CE_IP_QOS_Tp;
#ifdef QOS_SPEED_LIMIT_SUPPORT
typedef struct ipqos_speedrank_entry{
	unsigned char index;
	unsigned char prior;
	unsigned char speed;
	unsigned char count;
}MIB_CE_IP_QOS_SPEEDRANK_T,MIB_CE_IP_QOS_SPEEDRANK_Tp;
#endif
typedef struct acc_entry {
	unsigned char telnet __PACK__;
	unsigned char ftp __PACK__;
	unsigned char tftp __PACK__;
	unsigned char web __PACK__;
	unsigned char snmp __PACK__;
	unsigned char ssh __PACK__;
	unsigned char icmp __PACK__;
	unsigned char nop __PACK__; // added for alignment
	unsigned short telnet_port __PACK__;
	unsigned short web_port __PACK__;
	unsigned short ftp_port __PACK__;
	unsigned char https __PACK__;
	unsigned short https_port __PACK__;
//add by ramen for zte531b pvc's rmt acc.
#ifdef ZTE_GENERAL_ROUTER_SC
	unsigned char ifIndex __PACK__;	// high nibble for PPP, low nibble for mpoa
	unsigned char vpi __PACK__;
	unsigned short vci __PACK__;
	unsigned char protocol __PACK__;	
#endif
} MIB_CE_ACC_T, *MIB_CE_ACC_Tp;

#ifdef _CWMP_MIB_ /*jiunming, mib for cwmp-tr069*/
#ifdef WLAN_SUPPORT
typedef struct cwmp_psk {
	unsigned char index __PACK__;
	unsigned char presharedkey[64+1] __PACK__;
	unsigned char keypassphrase[63+1] __PACK__;
} CWMP_PSK_T, *CWMP_PSK_Tp;
#endif /*WLAN_SUPPORT*/
#ifdef _PRMT_X_CT_COM_PPPOE_
typedef struct cwmp_ct_pppoe {
	unsigned char Enable __PACK__;
	unsigned char ifIndex __PACK__;
	unsigned int  MAXUser __PACK__;
	unsigned int  InstanceNum __PACK__;
} CWMP_CT_PPPOE_T, *CWMP_CT_PPPOE_Tp;
#endif //_PRMT_X_CT_COM_PPPOE_
#endif /*_CWMP_MIB_*/

// Added by Mason Yu
typedef struct macBaseDhcp_entry {	
	unsigned char macAddr_Dhcp[MAC_ADDR_LEN] __PACK__;
	unsigned char ipAddr_Dhcp[IP_ADDR_LEN] __PACK__;		
} MIB_CE_MAC_BASE_DHCP_T, *MIB_CE_MAC_BASE_DHCP_Tp;

// add for auto-pvc-search
typedef struct AutoPvcSearch_entry{
	unsigned short vpi __PACK__;
	unsigned int vci __PACK__;
} MIB_AUTO_PVC_SEARCH_T, *MIB_AUTO_PVC_SEARCH_Tp;

// Added by Mason Yu
typedef struct ddns_entry {
	unsigned char provider[10] __PACK__;
	unsigned char hostname[35] __PACK__;	
	unsigned char interface[10] __PACK__;
	unsigned char username[35] __PACK__;
	unsigned char password[35] __PACK__; 
	unsigned char email[35] __PACK__;    // username
	unsigned char key[35] __PACK__;      // password
	unsigned char Enabled __PACK__;
 	unsigned int  InstanceNum __PACK__;
 	unsigned short ServicePort __PACK__;

} MIB_CE_DDNS_T, *MIB_CE_DDNS_Tp;

// Jenny, add for PPPoE session information
typedef struct pppoeSession_entry {
	unsigned char ifNo __PACK__;
	unsigned char vpi __PACK__;
	unsigned short vci __PACK__;
	unsigned char acMac[MAC_ADDR_LEN] __PACK__;
	unsigned short sessionId __PACK__;
} MIB_CE_PPPOE_SESSION_T, *MIB_CE_PPPOE_SESSION_Tp;

#ifdef ACCOUNT_CONFIG
// Jenny, add for user account information
typedef struct accountConfig_entry {
	unsigned char userName[MAX_NAME_LEN] __PACK__;	// user name
	unsigned char userPassword[MAX_NAME_LEN] __PACK__;	// user password
	unsigned char privilege __PACK__;	// account privilege, refer to ACC_PRIV_T
} MIB_CE_ACCOUNT_CONFIG_T, *MIB_CE_ACCOUNT_CONFIG_Tp;
#endif

#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
//////ql_xu def
//
typedef struct fc_entry {
	unsigned char ipAddr[IP_ADDR_LEN] __PACK__;
	unsigned char netMask[IP_ADDR_LEN] __PACK__;
	unsigned char lanIp2[IP_ADDR_LEN] __PACK__;
	unsigned char subnetMask2[IP_ADDR_LEN] __PACK__;
	unsigned char dhcpStartIp[IP_ADDR_LEN] __PACK__;
	unsigned char dhcpEndIp[IP_ADDR_LEN] __PACK__;
	unsigned int day __PACK__;
	unsigned int hour __PACK__;
	unsigned int min __PACK__;
	char encap_tmp __PACK__;
	char enable_ip2 __PACK__;
	char enable_dhcp __PACK__;
	char ip_changed __PACK__;
	/* next for pppoe/pppoA */
	//unsigned char wanIpAddr[IP_ADDR_LEN] __PACK__;
	/* mer mode */
	unsigned char dns1[IP_ADDR_LEN] __PACK__;
	unsigned char dns2[IP_ADDR_LEN] __PACK__;
	char dnsMode __PACK__;
	//char pppWanIpMode __PACK__;
}TMP_FC_T;

extern MIB_CE_ATM_VC_T fcEntry;
extern TMP_FC_T fcEntry2;
extern int store_day;
extern int store_hour;
extern int store_minute;
#endif

#ifdef LAYER7_FILTER_SUPPORT //star: for layer7 filter
#define MAX_APP_NAME  20
typedef struct layer7_entry{
	unsigned char appname[MAX_APP_NAME];
}LAYER7_FILTER_T,*LAYER7_FILTER_Tp;

#endif

/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
#define OPTION_LEN   32
#define GENERAL_LEN  64
typedef struct dhcp_serving_pool {
	//general
	unsigned char enable __PACK__;
	unsigned int poolorder __PACK__;
	unsigned char poolname[MAX_NAME_LEN] __PACK__;
	//criterion
	unsigned char sourceinterface  __PACK__;
	unsigned char vendorclass[OPTION_LEN] __PACK__;
	unsigned char vendorclassflag __PACK__;
	unsigned char clientid[OPTION_LEN] __PACK__;
	unsigned char clientidflag __PACK__;
	unsigned char userclass[OPTION_LEN] __PACK__;
	unsigned char userclassflag __PACK__;
	unsigned char chaddr[MAC_ADDR_LEN] __PACK__;
	unsigned char chaddrmask[MAC_ADDR_LEN] __PACK__;
	unsigned char chaddrflag __PACK__;
	//config
	unsigned char localserved __PACK__;
	unsigned char startaddr[IP_ADDR_LEN] __PACK__;
	unsigned char endaddr[IP_ADDR_LEN] __PACK__;
	unsigned char subnetmask[IP_ADDR_LEN] __PACK__;
	unsigned char iprouter[IP_ADDR_LEN] __PACK__;
	unsigned char dnsserver1[IP_ADDR_LEN]  __PACK__;
	unsigned char dnsserver2[IP_ADDR_LEN]  __PACK__;
	unsigned char dnsserver3[IP_ADDR_LEN]  __PACK__;
	unsigned char domainname[GENERAL_LEN] __PACK__;
	int leasetime  __PACK__;
	unsigned char dhcprelayip[IP_ADDR_LEN] __PACK__;
	unsigned char dnsservermode  __PACK__;
	
#ifdef _CWMP_MIB_
	unsigned int  InstanceNum __PACK__;
#endif	
	
}DHCPS_SERVING_POOL_T,*DHCPS_SERVING_POOL_Tp;
#endif
/*ping_zhang:20080919 END*/


extern BOOT_TYPE_T __boot_mode;

#ifdef PARENTAL_CTRL
#define MAX_PARENTCTRL_USER_LEN 32
#define MAX_PARENTCTRL_USER_NUM 16
#define SUNDAY 		(1<<0)
#define MONDAY 		(1<<1)
#define TUESDAY 		(1<<2)
#define WEDNESSDAY (1<<3)
#define THURSDAY 	(1<<4)
#define FRIDAY 		(1<<5)
#define SATURDAY 	(1<<6)
typedef struct parentctrl_entry {
	unsigned char username[MAX_PARENTCTRL_USER_LEN] __PACK__; 
	unsigned char mac[MAC_ADDR_LEN] __PACK__;
	unsigned char controlled_day;
	unsigned char start_hr;
	unsigned char start_min;
	unsigned char end_hr;
	unsigned char end_min;
	unsigned char cur_state __PACK__;
} MIB_PARENT_CTRL_T, *MIB_PARENT_CTRL_Tp;

#endif


#ifdef WEB_REDIRECT_BY_MAC
#define MAX_WEB_REDIR_BY_MAC		16
#define WEB_REDIR_BY_MAC_PORT		18080
typedef struct WebRedirByMAC_entry{
	unsigned char mac[MAC_ADDR_LEN] __PACK__;
} MIB_WEB_REDIR_BY_MAC_T, *MIB_WEB_REDIR_BY_MAC_Tp;
#endif

#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
#define MAX_ALLOWED_LIST 500
#define CAPTIVEPORTAL_PORT 18182
#define CP_MASK_DONOT_CARE 0xFF

typedef struct CaptivePortalAllowedList_entry
{
	unsigned char ip_addr[IP_ADDR_LEN];
	unsigned char mask;
} CWMP_CAPTIVEPORTAL_ALLOWED_LIST_T, *CWMP_CAPTIVEPORTAL_ALLOWED_LIST_Tp;
#endif

#ifdef WLAN_QoS
typedef struct wlan_qos_entry {
	unsigned int txop __PACK__;
	unsigned int ecwmax __PACK__;
	unsigned int ecwmin __PACK__;	
	unsigned int aifsn __PACK__;
	unsigned int ack __PACK__;	
} MIB_WLAN_QOS_T, *MIB_WLAN_QOS_Tp;
#endif // WLAN_QoS


/* ------------------------------------------------------------
 * MIB API
 * ------------------------------------------------------------ */
int mib_update_from_raw(unsigned char* ptr, int len); /* Write the specified setting to flash, this function will also check the length and checksum */ 
int mib_read_to_raw(CONFIG_DATA_T data_type, unsigned char* ptr, int len); /* Load flash setting to the specified pointer */
int _mib_update(CONFIG_DATA_T data_type); /* Update RAM setting to flash */
int mib_read_header(CONFIG_DATA_T data_type, PARAM_HEADER_Tp pHeader); /* Load flash header */
int _mib_load(CONFIG_DATA_T data_type); /* Load flash setting to RAM */
int mib_load_table(CONFIG_DATA_T data_type); /* Load flash setting of mib_table to RAM */
int mib_load_chain(CONFIG_DATA_T data_type); /* Load flash setting of mib_chain to RAM */
int mib_reset(CONFIG_DATA_T data_type); /* Reset to default */
int mib_update_firmware(unsigned char* ptr, int len); /* Update Firmware */

int mib_init(void); /* Initialize */
int _mib_get(int id, void *value); /* get mib value */
int _mib_set(int id, void *value); /* set mib value */
#ifdef INCLUDE_DEFAULT_VALUE
int mib_init_mib_with_program_default(CONFIG_DATA_T data_type, int action);
#endif

unsigned int _mib_chain_total(int id); /* get chain record size */
void _mib_chain_clear(int id); /* clear chain record */
int _mib_chain_add(int id, unsigned char* ptr); /* add chain record */
int _mib_chain_delete(int id, unsigned int recordNum); /* delete the specified chain record */
unsigned char* _mib_chain_get(int id, unsigned int recordNum); /* get the specified chain record */
// for message logging
int _mib_chain_update(int id, unsigned char* ptr, unsigned int recordNum); /* log updating the specified chain record */
#ifdef WLAN_SUPPORT
//1/20/06' hrchen, for WLAN enable/disable check
int wlan_is_up(void);
int update_wlan_disable(unsigned char disabled);
#endif
void itfcfg(char *if_name, int up_flag);
#define ACC_TELNET_PORT 61025
#define ACC_HTTP_PORT 61080
#define ACC_FTP_PORT 61021
//#include "sysconfig.h"
#endif // INCLUDE_MIB_H

