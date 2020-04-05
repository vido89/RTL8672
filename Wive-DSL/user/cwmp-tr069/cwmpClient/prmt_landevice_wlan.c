#include "prmt_landevice_wlan.h"
#include "prmt_landevice.h"
#include <rtk/utility.h>
#ifdef WLAN_SUPPORT

#define STR63ZERO	"000000000000000000000000000000000000000000000000000000000000000"
#define STR64ZERO	"0000000000000000000000000000000000000000000000000000000000000000"
#define	WLANUPDATETIME	90


char		gLocationDescription[4096]={0};
char		gWLANAssociations[ sizeof(WLAN_STA_INFO_T)*(MAX_STA_NUM+1) ];
time_t		gWLANAssUpdateTime=0;
unsigned int	gWLANTotalClients=0;
int		gWLANIDForAssInfo=-1; /*-1:no info, 0:wlan0, 1:wlan0-vap0, 2:wlan-vap1.....*/

char *wlan_name[ WLAN_IF_NUM ]=
	{ 
	"wlan0"
#ifdef WLAN_MBSSID
	, "wlan0-vap0", "wlan0-vap1", "wlan0-vap2", "wlan0-vap3" 
#endif
	};


static int _is_hex(char c);
static int string_to_hex(char *string, unsigned char *key, int len);

int updateWLANAssociations(void);
int loadWLANAssInfoByInstNum( unsigned int instnum );
int getWLANSTAINFO(int id, WLAN_STA_INFO_T *info);
int getRateStr( unsigned short rate, char *buf );
int setRateStr( char *buf, unsigned short *rate );
int getIPbyMAC( char *mac, char *ip );

unsigned int getWLANConfInstNum( char *name );
unsigned int getWEPInstNum( char *name );
unsigned int getAssDevInstNum( char *name );
unsigned int getPreSharedKeyInstNum( char *name );


struct CWMP_OP tPreSharedKeyEntityLeafOP = { getPreSharedKeyEntity, setPreSharedKeyEntity };
struct CWMP_PRMT tPreSharedKeyEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"PreSharedKey",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tPreSharedKeyEntityLeafOP},
{"KeyPassphrase",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tPreSharedKeyEntityLeafOP},
{"AssociatedDeviceMACAddress",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tPreSharedKeyEntityLeafOP}
};
enum ePreSharedKeyEntityLeaf
{
	ePreSharedKey,
	eKeyPassphrase,
	ePreAssociatedDeviceMACAddress
};
struct CWMP_LEAF tPreSharedKeyEntityLeaf[] =
{
{ &tPreSharedKeyEntityLeafInfo[ePreSharedKey] },
{ &tPreSharedKeyEntityLeafInfo[eKeyPassphrase] },
{ &tPreSharedKeyEntityLeafInfo[ePreAssociatedDeviceMACAddress] },
{ NULL }
};


struct CWMP_PRMT tPreSharedKeyObjectInfo[] =
{
/*(name,	type,		flag,		op)*/
{"1",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"2",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"3",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"4",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"5",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"6",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"7",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"8",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"9",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"10",		eCWMP_tOBJECT,	CWMP_READ,	NULL}
};
enum ePreSharedKeyObject
{
	ePreSharedKey1,
	ePreSharedKey2,
	ePreSharedKey3,
	ePreSharedKey4,
	ePreSharedKey5,
	ePreSharedKey6,
	ePreSharedKey7,
	ePreSharedKey8,
	ePreSharedKey9,
	ePreSharedKey10
};
struct CWMP_NODE tPreSharedKeyObject[] =
{
/*info,  					leaf,				next)*/
{&tPreSharedKeyObjectInfo[ePreSharedKey1],	tPreSharedKeyEntityLeaf,	NULL},
{&tPreSharedKeyObjectInfo[ePreSharedKey2],	tPreSharedKeyEntityLeaf,	NULL},
{&tPreSharedKeyObjectInfo[ePreSharedKey3],	tPreSharedKeyEntityLeaf,	NULL},
{&tPreSharedKeyObjectInfo[ePreSharedKey4],	tPreSharedKeyEntityLeaf,	NULL},
{&tPreSharedKeyObjectInfo[ePreSharedKey5],	tPreSharedKeyEntityLeaf,	NULL},
{&tPreSharedKeyObjectInfo[ePreSharedKey6],	tPreSharedKeyEntityLeaf,	NULL},
{&tPreSharedKeyObjectInfo[ePreSharedKey7],	tPreSharedKeyEntityLeaf,	NULL},
{&tPreSharedKeyObjectInfo[ePreSharedKey8],	tPreSharedKeyEntityLeaf,	NULL},
{&tPreSharedKeyObjectInfo[ePreSharedKey9],	tPreSharedKeyEntityLeaf,	NULL},
{&tPreSharedKeyObjectInfo[ePreSharedKey10],	tPreSharedKeyEntityLeaf,	NULL},
{NULL,						NULL,				NULL}
};


struct CWMP_OP tWEPKeyEntityLeafOP = { getWEPKeyEntity, setWEPKeyEntity };
struct CWMP_PRMT tWEPKeyEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"WEPKey",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWEPKeyEntityLeafOP}
};
enum eWEPKeyEntityLeaf
{
	eWEPKey
};
struct CWMP_LEAF tWEPKeyEntityLeaf[] =
{
{ &tWEPKeyEntityLeafInfo[eWEPKey] },
{ NULL }
};


struct CWMP_PRMT tWEPKeyObjectInfo[] =
{
/*(name,	type,		flag,		op)*/
{"1",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"2",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"3",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"4",		eCWMP_tOBJECT,	CWMP_READ,	NULL}
};
enum eWEPKeyObject
{
	eWEP1,
	eWEP2,
	eWEP3,
	eWEP4
};
struct CWMP_NODE tWEPKeyObject[] =
{
/*info,  			leaf,			next)*/
{&tWEPKeyObjectInfo[eWEP1],	tWEPKeyEntityLeaf,	NULL},
{&tWEPKeyObjectInfo[eWEP2],	tWEPKeyEntityLeaf,	NULL},
{&tWEPKeyObjectInfo[eWEP3],	tWEPKeyEntityLeaf,	NULL},
{&tWEPKeyObjectInfo[eWEP4],	tWEPKeyEntityLeaf,	NULL},
{NULL,				NULL,			NULL}
};


struct CWMP_OP tAscDeviceEntityLeafOP = { getAscDeviceEntity,NULL };
struct CWMP_PRMT tAscDeviceEntityLeafInfo[] =
{
/*(name,				type,		flag,				op)*/
{"AssociatedDeviceMACAddress",		eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tAscDeviceEntityLeafOP},
{"AssociatedDeviceIPAddress",		eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tAscDeviceEntityLeafOP},
{"AssociatedDeviceAuthenticationState",	eCWMP_tBOOLEAN,	CWMP_READ|CWMP_DENY_ACT,	&tAscDeviceEntityLeafOP},
{"LastRequestedUnicastCipher",		eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tAscDeviceEntityLeafOP},
{"LastRequestedMulticastCipher",	eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tAscDeviceEntityLeafOP},
{"LastPMKId",				eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tAscDeviceEntityLeafOP}
};
enum eAscDeviceEntityLeaf
{
	eAssociatedDeviceMACAddress,
	eAssociatedDeviceIPAddress,
	eAssociatedDeviceAuthenticationState,
	eLastRequestedUnicastCipher,
	eLastRequestedMulticastCipher,
	eLastPMKId
};
struct CWMP_LEAF tAscDeviceEntityLeaf[] =
{
{ &tAscDeviceEntityLeafInfo[eAssociatedDeviceMACAddress] },
{ &tAscDeviceEntityLeafInfo[eAssociatedDeviceIPAddress] },
{ &tAscDeviceEntityLeafInfo[eAssociatedDeviceAuthenticationState] },
{ &tAscDeviceEntityLeafInfo[eLastRequestedUnicastCipher] },
{ &tAscDeviceEntityLeafInfo[eLastRequestedMulticastCipher] },
{ &tAscDeviceEntityLeafInfo[eLastPMKId] },
{ NULL }
};


struct CWMP_PRMT tAscDeviceObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_LNKLIST,	NULL}
};
enum eAscDeviceObject
{
	eAscDevice0
};
struct CWMP_LINKNODE tAscDeviceObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tAscDeviceObjectInfo[eAscDevice0],	tAscDeviceEntityLeaf,	NULL,		NULL,			0}
};



struct CWMP_OP tWLANConfEntityLeafOP = { getWLANConf, setWLANConf };
struct CWMP_PRMT tWLANConfEntityLeafInfo[] =
{
/*(name,				type,		flag,			op)*/
{"Enable",				eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"Status",				eCWMP_tSTRING,	CWMP_READ,		&tWLANConfEntityLeafOP},
{"BSSID",				eCWMP_tSTRING,	CWMP_READ,		&tWLANConfEntityLeafOP},
{"MaxBitRate",				eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"Channel",				eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"SSID",				eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"BeaconType",				eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
#ifdef MAC_FILTER
{"MACAddressControlEnabled",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
#endif /*MAC_FILTER*/
{"Standard",				eCWMP_tSTRING,	CWMP_READ,		&tWLANConfEntityLeafOP},
{"WEPKeyIndex",				eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"KeyPassphrase",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
#ifdef CONFIG_BOA_WEB_E8B_CH	//cathy
{"WEPEncryptionLevel",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,		&tWLANConfEntityLeafOP},
#else
{"WEPEncryptionLevel",			eCWMP_tSTRING,	CWMP_READ,		&tWLANConfEntityLeafOP},
#endif
{"BasicEncryptionModes",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"BasicAuthenticationMode",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"WPAEncryptionModes",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"WPAAuthenticationMode",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"IEEE11iEncryptionModes",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"IEEE11iAuthenticationMode",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"PossibleChannels",			eCWMP_tSTRING,	CWMP_READ,		&tWLANConfEntityLeafOP},
{"BasicDataTransmitRates",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"OperationalDataTransmitRates",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"PossibleDataTransmitRates",		eCWMP_tSTRING,	CWMP_READ,		&tWLANConfEntityLeafOP},
/*InsecureOOBAccessEnabled*/
{"BeaconAdvertisementEnabled",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"SSIDAdvertisementEnabled",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP}, /*version 1.4*/
{"RadioEnabled",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"AutoRateFallBackEnabled",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"LocationDescription",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
/*RequlatoryDomain*/
/*TotalPSKFailures*/
/*TotalIntegrityFailures*/
{"ChannelsInUse",			eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,&tWLANConfEntityLeafOP},
{"DeviceOperationMode",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
/*DistanceFromRoot*/
/*PeerBSSID*/
{"AuthenticationServiceMode",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"TotalBytesSent",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWLANConfEntityLeafOP},
{"TotalBytesReceived",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWLANConfEntityLeafOP},
{"TotalPacketsSent",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWLANConfEntityLeafOP},
{"TotalPacketsReceived",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWLANConfEntityLeafOP},
#ifdef _PRMT_X_CT_COM_WLAN_
{"X_CT-COM_SSIDHide",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"X_CT-COM_Powerlevel",			eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"X_CT-COM_PowerValue",			eCWMP_tUINT,	CWMP_READ,		&tWLANConfEntityLeafOP},
{"X_CT-COM_APModuleEnable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,		&tWLANConfEntityLeafOP},
#endif //_PRMT_X_CT_COM_WLAN_
{"TotalAssociations",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWLANConfEntityLeafOP}
};
enum eWLANConfEntityLeafInfo
{
	eWL_Enable,
	eWL_Status,
	eWL_BSSID,
	eWL_MaxBitRate,
	eWL_Channel,
	eWL_SSID,
	eWL_BeaconType,
#ifdef MAC_FILTER
	eWL_MACAddressControlEnabled,
#endif
	eWL_Standard,
	eWL_WEPKeyIndex,
	eWL_KeyPassphrase,
	eWL_WEPEncryptionLevel,
	eWL_BasicEncryptionModes,
	eWL_BasicAuthenticationMode,
	eWL_WPAEncryptionModes,
	eWL_WPAAuthenticationMode,
	eWL_IEEE11iEncryptionModes,
	eWL_IEEE11iAuthenticationMode,
	eWL_PossibleChannels,
	eWL_BasicDataTransmitRates,
	eWL_OperationalDataTransmitRates,
	eWL_PossibleDataTransmitRates,
	/*InsecureOOBAccessEnabled*/
	eWL_BeaconAdvertisementEnabled,
	eWL_SSIDAdvertisementEnabled,
	eWL_RadioEnabled,
	eWL_AutoRateFallBackEnabled,
	eWL_LocationDescription,
	/*RequlatoryDomain*/
	/*TotalPSKFailures*/
	/*TotalIntegrityFailures*/
	eWL_ChannelsInUse,
	eWL_DeviceOperationMode,
	/*DistanceFromRoot*/
	/*PeerBSSID*/
	eWL_AuthenticationServiceMode,
	eWL_TotalBytesSent,
	eWL_TotalBytesReceived,
	eWL_TotalPacketsSent,
	eWL_TotalPacketsReceived,
#ifdef _PRMT_X_CT_COM_WLAN_
	eWL_X_CTCOM_SSIDHide,
	eWL_X_CTCOM_Powerlevel,
	eWL_X_CTCOM_PowerValue,
	eWL_X_CTCOM_APModuleEnable,
#endif //_PRMT_X_CT_COM_WLAN_
	eWL_TotalAssociations	
};
struct CWMP_LEAF tWLANConfEntityLeaf[] =
{
{ &tWLANConfEntityLeafInfo[eWL_Enable] },
{ &tWLANConfEntityLeafInfo[eWL_Status] },
{ &tWLANConfEntityLeafInfo[eWL_BSSID] },
{ &tWLANConfEntityLeafInfo[eWL_MaxBitRate] },
{ &tWLANConfEntityLeafInfo[eWL_Channel] },
{ &tWLANConfEntityLeafInfo[eWL_SSID] },
{ &tWLANConfEntityLeafInfo[eWL_BeaconType] },
#ifdef MAC_FILTER
{ &tWLANConfEntityLeafInfo[eWL_MACAddressControlEnabled] },
#endif
{ &tWLANConfEntityLeafInfo[eWL_Standard] },
{ &tWLANConfEntityLeafInfo[eWL_WEPKeyIndex] },
{ &tWLANConfEntityLeafInfo[eWL_KeyPassphrase] },
{ &tWLANConfEntityLeafInfo[eWL_WEPEncryptionLevel] },
{ &tWLANConfEntityLeafInfo[eWL_BasicEncryptionModes] },
{ &tWLANConfEntityLeafInfo[eWL_BasicAuthenticationMode] },
{ &tWLANConfEntityLeafInfo[eWL_WPAEncryptionModes] },
{ &tWLANConfEntityLeafInfo[eWL_WPAAuthenticationMode] },
{ &tWLANConfEntityLeafInfo[eWL_IEEE11iEncryptionModes] },
{ &tWLANConfEntityLeafInfo[eWL_IEEE11iAuthenticationMode] },
{ &tWLANConfEntityLeafInfo[eWL_PossibleChannels] },
{ &tWLANConfEntityLeafInfo[eWL_BasicDataTransmitRates] },
{ &tWLANConfEntityLeafInfo[eWL_OperationalDataTransmitRates] },
{ &tWLANConfEntityLeafInfo[eWL_PossibleDataTransmitRates] },
/*InsecureOOBAccessEnabled*/
{ &tWLANConfEntityLeafInfo[eWL_BeaconAdvertisementEnabled] },
{ &tWLANConfEntityLeafInfo[eWL_SSIDAdvertisementEnabled] },
{ &tWLANConfEntityLeafInfo[eWL_RadioEnabled] },
{ &tWLANConfEntityLeafInfo[eWL_AutoRateFallBackEnabled] },
{ &tWLANConfEntityLeafInfo[eWL_LocationDescription] },
/*RequlatoryDomain*/
/*TotalPSKFailures*/
/*TotalIntegrityFailures*/
{ &tWLANConfEntityLeafInfo[eWL_ChannelsInUse] },
{ &tWLANConfEntityLeafInfo[eWL_DeviceOperationMode] },
/*DistanceFromRoot*/
/*PeerBSSID*/
{ &tWLANConfEntityLeafInfo[eWL_AuthenticationServiceMode] },
{ &tWLANConfEntityLeafInfo[eWL_TotalBytesSent] },
{ &tWLANConfEntityLeafInfo[eWL_TotalBytesReceived] },
{ &tWLANConfEntityLeafInfo[eWL_TotalPacketsSent] },
{ &tWLANConfEntityLeafInfo[eWL_TotalPacketsReceived] },
#ifdef _PRMT_X_CT_COM_WLAN_
{ &tWLANConfEntityLeafInfo[eWL_X_CTCOM_SSIDHide] },
{ &tWLANConfEntityLeafInfo[eWL_X_CTCOM_Powerlevel] },
{ &tWLANConfEntityLeafInfo[eWL_X_CTCOM_PowerValue] },
{ &tWLANConfEntityLeafInfo[eWL_X_CTCOM_APModuleEnable] },
#endif //_PRMT_X_CT_COM_WLAN_
{ &tWLANConfEntityLeafInfo[eWL_TotalAssociations] },	
{ NULL }
};

struct CWMP_OP tWLAN_AssociatedDevice_OP = { NULL, objAscDevice };
struct CWMP_PRMT tWLANConfEntityObjectInfo[] =
{
/*(name,				type,		flag,			op)*/
{"AssociatedDevice",			eCWMP_tOBJECT,	CWMP_READ,		&tWLAN_AssociatedDevice_OP},
{"WEPKey",				eCWMP_tOBJECT,	CWMP_READ,		NULL},
{"PreSharedKey",			eCWMP_tOBJECT,	CWMP_READ,		NULL}
};
enum eWLANConfEntityObject
{
	eWLAN_AssociatedDevice,
	eWLAN_WEPKey,
	eWLAN_PreSharedKey
};
struct CWMP_NODE tWLANConfEntityObject[] =
{
/*info,  						leaf,	next)*/
{&tWLANConfEntityObjectInfo[eWLAN_AssociatedDevice],	NULL,	NULL},
{&tWLANConfEntityObjectInfo[eWLAN_WEPKey],		NULL,	tWEPKeyObject},
{&tWLANConfEntityObjectInfo[eWLAN_PreSharedKey],	NULL,	tPreSharedKeyObject},
{NULL,							NULL,	NULL}
};



struct CWMP_PRMT tWLANConfigObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"1",				eCWMP_tOBJECT,	CWMP_READ,		NULL},
#ifdef WLAN_MBSSID
{"2",				eCWMP_tOBJECT,	CWMP_READ,		NULL},
{"3",				eCWMP_tOBJECT,	CWMP_READ,		NULL},
{"4",				eCWMP_tOBJECT,	CWMP_READ,		NULL},
{"5",				eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
};
enum eWLANConfigObject
{
	eWLAN1,
#ifdef WLAN_MBSSID
	eWLAN2,
	eWLAN3,
	eWLAN4,
	eWLAN5
#endif
};
struct CWMP_NODE tWLANConfigObject[] =
{
/*info,  			leaf,			node)*/
{ &tWLANConfigObjectInfo[eWLAN1],tWLANConfEntityLeaf,	tWLANConfEntityObject},	
#ifdef WLAN_MBSSID
{ &tWLANConfigObjectInfo[eWLAN2],tWLANConfEntityLeaf,	tWLANConfEntityObject},	
{ &tWLANConfigObjectInfo[eWLAN3],tWLANConfEntityLeaf,	tWLANConfEntityObject},	
{ &tWLANConfigObjectInfo[eWLAN4],tWLANConfEntityLeaf,	tWLANConfEntityObject},	
{ &tWLANConfigObjectInfo[eWLAN5],tWLANConfEntityLeaf,	tWLANConfEntityObject},	
#endif
{ NULL,				NULL,			NULL}
};

#ifdef CTCOM_WLAN_REQ
struct CWMP_PRMT tWLANObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eWLANObject
{
	eWLAN0
};
struct CWMP_LINKNODE tWLANObject[] =
{
/*info,  				leaf,			next,				sibling,		instnum)*/
{&tWLANObjectInfo[eWLAN0],	tWLANConfEntityLeaf,	tWLANConfEntityObject,		NULL,			0}
};
#endif

int getPreSharedKeyEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char		*lastname = entity->info->name;
	int		id=0,chain_id=0;
	CWMP_PSK_T	*pEntry=NULL, psk_entity;
	char		*tok;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	id = getPreSharedKeyInstNum( name );
	if( (id<1) || (id>10) ) return ERR_9007;

	chain_id = getPSKChainId( id );
	if( chain_id >= 0 )
	{
		pEntry = &psk_entity;
		if( !mib_chain_get(CWMP_PSK_TBL, chain_id, (void*)pEntry ) )
			pEntry = NULL;
	}else
		pEntry = NULL;


	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "PreSharedKey" )==0 )
	{
#if 1
		*data = strdup( "" ); /*return an empty string*/
#else
		if(pEntry)
			*data = strdup( pEntry->presharedkey );
		else
			*data = strdup( STR64ZERO );
#endif	
	}else if( strcmp( lastname, "KeyPassphrase" )==0 )
	{
#if 1
		*data = strdup( "" ); /*return an empty string*/
#else
		if(pEntry)
			*data = strdup( pEntry->keypassphrase );
		else
			*data = strdup( STR63ZERO );
#endif
	}else if( strcmp( lastname, "AssociatedDeviceMACAddress" )==0 )
	{
		*data = strdup( "" );
	}else{
		return ERR_9005;
	}
	return 0;
}

int setPreSharedKeyEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char		*lastname = entity->info->name;
	int		id=0, chain_id=0;
	CWMP_PSK_T	*pEntry=NULL, psk_entity;
	char		*tok;
	char		*buf=data;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	id = getPreSharedKeyInstNum( name );
	if( (id<1) || (id>10) ) return ERR_9007;

	chain_id = getPSKChainId( id );
	if( chain_id >= 0 )
	{
		pEntry = &psk_entity;
		if( !mib_chain_get(CWMP_PSK_TBL, chain_id, (void*)pEntry ) )
			pEntry = NULL;
	}else
		pEntry = NULL;

	if( strcmp( lastname, "PreSharedKey" )==0 )
	{
		int i,len;
		if( buf==NULL ) return ERR_9007;
		
		len = strlen(buf);
		if( len==0 || len>64 ) return ERR_9007;
#ifndef CONFIG_BOA_WEB_E8B_CH	//cathy			
		for( i=0; i<len; i++ )
			if( _is_hex(buf[i])==0 ) return ERR_9007;
#endif			
		if( pEntry )
		{
			strcpy( pEntry->presharedkey, buf );
			mib_chain_update(CWMP_PSK_TBL, (char *)pEntry, chain_id );
		}else{
			CWMP_PSK_T new_psk;
			new_psk.index=id;
			strcpy( new_psk.presharedkey, buf );
			strcpy( new_psk.keypassphrase, STR63ZERO );
			mib_chain_add( CWMP_PSK_TBL, (unsigned char*)&new_psk );
		}

		if( id==1 ) //also update MIB_WLAN_WPA_PSK
		{
			unsigned char pskfmt;
#ifdef CONFIG_BOA_WEB_E8B_CH	//cathy			
			pskfmt = 0; //0:Passphrase,   1:hex
#else
			pskfmt = 1;
#endif
#ifdef WLAN_MBSSID
			unsigned int wlaninst=0;
			MIB_CE_MBSSIB_T Entry;

			wlaninst = getWLANConfInstNum( name );
			if( wlaninst<1 || wlaninst>5 )	return ERR_9007;
			if(wlaninst != 1){
				if (!mib_chain_get(MIB_MBSSIB_TBL, wlaninst-1, (void *)&Entry)) return ERR_9002;
				strcpy( Entry.wpaPSK, buf );
				Entry.wpaPSKFormat = pskfmt;
				mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
			}else
#endif		
			{
				mib_set(MIB_WLAN_WPA_PSK, (void *)buf);
				mib_set(MIB_WLAN_WPA_PSK_FORMAT, (void *)&pskfmt);
			}
		}
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "KeyPassphrase" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( (strlen(buf)<8) || (strlen(buf)>63) ) return ERR_9007;
		if( pEntry )
		{
			strcpy( pEntry->keypassphrase, buf );
			mib_chain_update(CWMP_PSK_TBL, (char *)pEntry, chain_id);
		}else{
			CWMP_PSK_T new_psk;
			new_psk.index=id;
			strcpy( new_psk.presharedkey, STR64ZERO );
			strcpy( new_psk.keypassphrase, buf );
			mib_chain_add( CWMP_PSK_TBL, (unsigned char*)&new_psk );
		}
		
		if( id==1 ) //also update MIB_WLAN_WPA_PSK
		{
			unsigned char pskfmt;
			pskfmt = 0; //0:Passphrase,   1:hex
#ifdef WLAN_MBSSID
			unsigned int wlaninst=0;
			MIB_CE_MBSSIB_T Entry;

			wlaninst = getWLANConfInstNum( name );
			if( wlaninst<1 || wlaninst>5 )	return ERR_9007;
			if(wlaninst != 1){
				if (!mib_chain_get(MIB_MBSSIB_TBL, wlaninst-1, (void *)&Entry)) return ERR_9002;
				strcpy( Entry.wpaPSK, buf );
				Entry.wpaPSKFormat = pskfmt;
				mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
			}else
#endif		
			{
				mib_set(MIB_WLAN_WPA_PSK, (void *)buf);
				mib_set(MIB_WLAN_WPA_PSK_FORMAT, (void *)&pskfmt);
			}
		}
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "AssociatedDeviceMACAddress" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)!=0 ) return ERR_9001;
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int getWEPKeyEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	int	keyid=0;
	char	*tok;
	unsigned int wlaninst=0,wepinst=0;;
#ifdef WLAN_MBSSID
	MIB_CE_MBSSIB_WEP_T Entry;
#endif
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	wlaninst = getWLANConfInstNum( name );
#ifdef WLAN_MBSSID
	if( wlaninst<1 || wlaninst>5 )	return ERR_9007;
#else
	if( wlaninst!=1 ) return ERR_9007;
#endif
	wepinst = getWEPInstNum( name );
	if( wepinst<1 || wepinst>4 )	return ERR_9007;
#ifdef WLAN_MBSSID
	if (!mib_chain_get(MIB_MBSSIB_WEP_TBL, wlaninst-1, (void *)&Entry)) return ERR_9002;
#endif


	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "WEPKey" )==0 )
	{
#if 1
		*data = strdup( "" ); /*return an empty string*/
#else

		unsigned char	hex_key[32],ascii_key[32];
		unsigned char keyfmt=WEP64;
		//0:disable, 1:64, 2:128
#ifdef WLAN_MBSSID
		{
			MIB_CE_MBSSIB_T EntryWPA;
			if (!mib_chain_get(MIB_MBSSIB_TBL, wlaninst-1, (void *)&EntryWPA)) return ERR_9002;
			keyfmt = EntryWPA.wep;
		}
#else
		mib_get( MIB_WLAN_WEP, (void *)&keyfmt);
#endif				
		keyid = wepinst;
		switch( keyid )
		{
		case 1:
			if(keyfmt!=WEP128)
#ifdef WLAN_MBSSID
				//strcpy(ascii_key, Entry.wep64Key1);
				memcpy( ascii_key, Entry.wep64Key1, sizeof(Entry.wep64Key1) );
#else
				mib_get(MIB_WLAN_WEP64_KEY1, (void *)ascii_key);
#endif
			else
#ifdef WLAN_MBSSID
				//strcpy(ascii_key, Entry.wep128Key1);	
				memcpy( ascii_key, Entry.wep128Key1, sizeof(Entry.wep128Key1) );
#else
				mib_get(MIB_WLAN_WEP128_KEY1, (void *)ascii_key);
#endif
			break;
		case 2:
			if(keyfmt!=WEP128)
#ifdef WLAN_MBSSID
				//strcpy(ascii_key, Entry.wep64Key2);
				memcpy( ascii_key, Entry.wep64Key2, sizeof(Entry.wep64Key2) );
#else
				mib_get(MIB_WLAN_WEP64_KEY2, (void *)ascii_key);
#endif
			else
#ifdef WLAN_MBSSID
				//strcpy(ascii_key, Entry.wep128Key2);	
				memcpy( ascii_key, Entry.wep128Key2, sizeof(Entry.wep128Key2) );
#else
				mib_get(MIB_WLAN_WEP128_KEY2, (void *)ascii_key);
#endif
			break;
		case 3:
			if(keyfmt!=WEP128)
#ifdef WLAN_MBSSID
				//strcpy(ascii_key, Entry.wep64Key3);
				memcpy( ascii_key, Entry.wep64Key3, sizeof(Entry.wep64Key3) );
#else
				mib_get(MIB_WLAN_WEP64_KEY3, (void *)ascii_key);
#endif
			else
#ifdef WLAN_MBSSID
				//strcpy(ascii_key, Entry.wep128Key3);	
				memcpy( ascii_key, Entry.wep128Key3, sizeof(Entry.wep128Key3) );
#else
				mib_get(MIB_WLAN_WEP128_KEY3, (void *)ascii_key);
#endif
			break;
		case 4:
			if(keyfmt!=WEP128)
#ifdef WLAN_MBSSID
				//strcpy(ascii_key, Entry.wep64Key4);
				memcpy( ascii_key, Entry.wep64Key4, sizeof(Entry.wep64Key4) );
#else
				mib_get(MIB_WLAN_WEP64_KEY4, (void *)ascii_key);
#endif
			else
#ifdef WLAN_MBSSID
				//strcpy(ascii_key, Entry.wep128Key4);	
				memcpy( ascii_key, Entry.wep128Key4, sizeof(Entry.wep128Key4) );
#else
				mib_get(MIB_WLAN_WEP128_KEY4, (void *)ascii_key);
#endif
			break;
		default:
			return ERR_9005;
		}
		if( keyfmt!=WEP128 )
			sprintf( hex_key, "%02x%02x%02x%02x%02x", 
				ascii_key[0], ascii_key[1], ascii_key[2], ascii_key[3], ascii_key[4] );
		else
			sprintf( hex_key, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
				ascii_key[0], ascii_key[1], ascii_key[2], ascii_key[3], ascii_key[4],
				ascii_key[5], ascii_key[6], ascii_key[7], ascii_key[8], ascii_key[9],
				ascii_key[10], ascii_key[11], ascii_key[12] );

		//for debug purpose, just show the key (no matter what it's type is?)
		*data = strdup( hex_key );
#endif
	}else{
		return ERR_9005;
	}
	return 0;
}

int setWEPKeyEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned int wlaninst=0,wepinst=0;;
#ifdef WLAN_MBSSID
	MIB_CE_MBSSIB_WEP_T Entry;
#endif
	
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	wlaninst = getWLANConfInstNum( name );
#ifdef WLAN_MBSSID
	if( wlaninst<1 || wlaninst>5 )	return ERR_9007;
#else
	if( wlaninst!=1 ) return ERR_9007;
#endif
	wepinst = getWEPInstNum( name );
	if( wepinst<1 || wepinst>4 )	return ERR_9007;
#ifdef WLAN_MBSSID
	if (!mib_chain_get(MIB_MBSSIB_WEP_TBL, wlaninst-1, (void *)&Entry)) return ERR_9002;
#endif

	if( strcmp( lastname, "WEPKey" )==0 )
	{
		unsigned char ascii_key[32], *tok;
		int len=0, keyid;
		unsigned char keyfmt,key_type;

		if( buf==NULL ) return ERR_9007;
		len = strlen(buf);
		if( (len!=10) && (len!=26) ) return ERR_9007;
		memset( ascii_key, 0, sizeof(ascii_key) );
		if(!string_to_hex(buf, ascii_key, len)) return ERR_9007;

		//key format==>0:disable, 1:64, 2:128
		keyfmt = (len==10)?WEP64:WEP128;
#ifdef WLAN_MBSSID
		if(wlaninst != 1)
		{
			MIB_CE_MBSSIB_T EntryWPA;
			if (!mib_chain_get(MIB_MBSSIB_TBL, wlaninst-1, (void *)&EntryWPA)) return ERR_9002;
			EntryWPA.wep = keyfmt;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&EntryWPA, wlaninst-1);
		}else
#endif
		mib_set( MIB_WLAN_WEP, (void *)&keyfmt);

		//key type==>KEY_ASCII:ascii, KEY_HEX:hex, tr-069 always uses the hex format.
		key_type = KEY_HEX; 
#ifdef WLAN_MBSSID
		if(wlaninst != 1){
			Entry.wepKeyType = key_type;
		}else
#endif
		mib_set( MIB_WLAN_WEP_KEY_TYPE, (void *)&key_type);

		keyid = wepinst;
		switch( keyid )
		{
		case 1:
			if(keyfmt==WEP64)
#ifdef WLAN_MBSSID
				if(wlaninst != 1){
					//strcpy(Entry.wep64Key1, ascii_key);	
					memcpy( Entry.wep64Key1, ascii_key, sizeof(Entry.wep64Key1) );
				}else
#endif
				mib_set(MIB_WLAN_WEP64_KEY1, (void *)ascii_key);

			else
#ifdef WLAN_MBSSID
				if(wlaninst != 1){
					//strcpy(Entry.wep128Key1, ascii_key);	
					memcpy( Entry.wep128Key1, ascii_key, sizeof(Entry.wep128Key1) );
				}else
#endif
				mib_set(MIB_WLAN_WEP128_KEY1, (void *)ascii_key);

			break;
		case 2:
			if(keyfmt==WEP64)
#ifdef WLAN_MBSSID
				if(wlaninst != 1){
					//strcpy(Entry.wep64Key2, ascii_key);	
					memcpy( Entry.wep64Key2, ascii_key, sizeof(Entry.wep64Key2) );
				}else
#endif
				mib_set(MIB_WLAN_WEP64_KEY2, (void *)ascii_key);

			else
#ifdef WLAN_MBSSID
				if(wlaninst != 1){
					//strcpy(Entry.wep128Key2, ascii_key);	
					memcpy( Entry.wep128Key2, ascii_key, sizeof(Entry.wep128Key2) );
				}else
#endif
				mib_set(MIB_WLAN_WEP128_KEY2, (void *)ascii_key);

			break;
		case 3:
			if(keyfmt==WEP64)
#ifdef WLAN_MBSSID
				if(wlaninst != 1){
					//strcpy(Entry.wep64Key3, ascii_key);	
					memcpy( Entry.wep64Key3, ascii_key, sizeof(Entry.wep64Key3) );
				}else
#endif
				mib_set(MIB_WLAN_WEP64_KEY3, (void *)ascii_key);

			else
#ifdef WLAN_MBSSID
				if(wlaninst != 1){
					//strcpy(Entry.wep128Key3, ascii_key);	
					memcpy( Entry.wep128Key3, ascii_key, sizeof(Entry.wep128Key3) );
				}else
#endif
				mib_set(MIB_WLAN_WEP128_KEY3, (void *)ascii_key);

			break;
		case 4:
			if(keyfmt==WEP64)
#ifdef WLAN_MBSSID
				if(wlaninst != 1){
					//strcpy(Entry.wep64Key4, ascii_key);	
					memcpy( Entry.wep64Key4, ascii_key, sizeof(Entry.wep64Key4) );
				}else
#endif
				mib_set(MIB_WLAN_WEP64_KEY4, (void *)ascii_key);

			else
#ifdef WLAN_MBSSID
				if(wlaninst != 1){
					//strcpy(Entry.wep128Key4, ascii_key);	
					memcpy( Entry.wep128Key4, ascii_key, sizeof(Entry.wep128Key4) );
				}else
#endif
				mib_set(MIB_WLAN_WEP128_KEY4, (void *)ascii_key);

			break;
		default:
			return ERR_9005;		
		}		

#ifdef WLAN_MBSSID
		mib_chain_update(MIB_MBSSIB_WEP_TBL, (void *)&Entry, wlaninst-1);
#endif
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int getAscDeviceEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char		*lastname = entity->info->name;
	unsigned int	device_id=0;
	WLAN_STA_INFO_T info;
	char		*tok=NULL;
	unsigned int wlaninst=0;

	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	wlaninst = getWLANConfInstNum( name );
#ifdef WLAN_MBSSID
	if( wlaninst<1 || wlaninst>5 )	return ERR_9007;
#else
	if( wlaninst!=1 ) return ERR_9007;
#endif

	if( loadWLANAssInfoByInstNum(wlaninst)<0 ) return ERR_9002;
	device_id = getAssDevInstNum(name);
	if( device_id<1 || device_id>gWLANTotalClients ) return ERR_9005;
	if( getWLANSTAINFO( device_id-1, &info )<0 ) return ERR_9002;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "AssociatedDeviceMACAddress" )==0 )
	{
		char buf[32];
		sprintf( buf, "%02x:%02x:%02x:%02x:%02x:%02x",
				info.addr[0],info.addr[1],info.addr[2],
				info.addr[3],info.addr[4],info.addr[5] );
		*data = strdup( buf );
	}else if( strcmp( lastname, "AssociatedDeviceIPAddress" )==0 )
	{
		char aip[32]="",amac[32]="";
		sprintf( amac, "%02x:%02x:%02x:%02x:%02x:%02x",
				info.addr[0],info.addr[1],info.addr[2],
				info.addr[3],info.addr[4],info.addr[5] );
		if( getIPbyMAC( amac, aip ) < 0 )
			*data = strdup( "" );
		else
			*data = strdup( aip );	
	}else if( strcmp( lastname, "AssociatedDeviceAuthenticationState" )==0 )
	{
		int i = ((info.flag & STA_INFO_FLAG_ASOC)==STA_INFO_FLAG_ASOC);
		*data = intdup( i );
	}else if( strcmp( lastname, "LastRequestedUnicastCipher" )==0 )
	{
		*data = strdup( "" );
	}else if( strcmp( lastname, "LastRequestedMulticastCipher" )==0 )
	{
		*data = strdup( "" );
	}else if( strcmp( lastname, "LastPMKId" )==0 )
	{
		*data = strdup( "" );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int objAscDevice(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);fflush(NULL);
	unsigned int wlaninst=0;
		
	wlaninst = getWLANConfInstNum( name );
	if( wlaninst<1 || wlaninst>WLAN_IF_NUM ) return ERR_9007;

	switch( type )
	{
	case eCWMP_tINITOBJ:
	     {
		struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;
		
		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
		
		*c=NULL;
		loadWLANAssInfoByInstNum(wlaninst);
		if(gWLANTotalClients>0)
			return create_Object( c, tAscDeviceObject, sizeof(tAscDeviceObject), gWLANTotalClients, 1 );
		
		return 0;
		break;
	     }
	case eCWMP_tUPDATEOBJ:
	     {
	     	unsigned int num,i;
	     	struct CWMP_LINKNODE *old_table;

		loadWLANAssInfoByInstNum(wlaninst);
		num = gWLANTotalClients;
	     	old_table = (struct CWMP_LINKNODE*)entity->next;
	     	entity->next = NULL;
	     	for( i=0; i<num;i++ )
	     	{
	     		struct CWMP_LINKNODE *remove_entity=NULL;

			remove_entity = remove_SiblingEntity( &old_table, i+1 );
			if( remove_entity!=NULL )
			{
				add_SiblingEntity( (struct CWMP_LINKNODE**)&entity->next, remove_entity );
			}else{ 
				unsigned int InstNum=i+1;
				add_Object( name, (struct CWMP_LINKNODE**)&entity->next,  tAscDeviceObject, sizeof(tAscDeviceObject), &InstNum );
			}
	     	}
	     	if( old_table )
	     		destroy_ParameterTable( (struct CWMP_NODE*)old_table );
		return 0;
	     	break;
	     }
	}

	return -1;
}

#ifdef CTCOM_WLAN_REQ
#define WLANENTRYNUM   5
int objWLANConfiguration(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	unsigned int num=0,i,maxnum=0;
	
	unsigned int wlaninst=0;
	MIB_CE_MBSSIB_T Entry;
	unsigned int total;
	unsigned char mssidmap=1;

	total = mib_chain_total(MIB_MBSSIB_TBL);
//	printf("\ntotal=%d\n",total);
	for(i=1;i<total;i++){
		if (!mib_chain_get(MIB_MBSSIB_TBL, i, (void *)&Entry))
			continue;
//		printf("\nEntry.instnum=%d\n",Entry.instnum);
		if(Entry.instnum==(i+1))
			mssidmap|=(1<<i);
	}
//	printf("\nmssidmap=%x\n",mssidmap);
//	printf("\nname=%s\n",name);

	//CWMPDBG( 1, ( stderr, "<%s:%d>name:%s(action:%d)\n", __FUNCTION__, __LINE__, name,type ) );
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);fflush(NULL);
	if( (name==NULL) || (entity==NULL) ) return -1;

	switch( type )
	{
	case eCWMP_tINITOBJ:
	     {
		struct CWMP_LINKNODE **ptable = (struct CWMP_LINKNODE **)data;
		
		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		for(i=0;i<WLANENTRYNUM;i++){
			if(mssidmap&(1<<i)!=0)
				if( create_Object( ptable, tWLANObject, sizeof(tWLANObject), 1, (i+1)) < 0 )
						return -1;
		}
		return 0;
	     	break;
	     }
	case eCWMP_tADDOBJ:
	     {
	     	int ret;
	     	
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		for(i=1;i<WLANENTRYNUM;i++){
			if((mssidmap&(1<<i))==0)
				break;
		}
//		printf("\nfree num=%d\n",i);
		if(i==WLANENTRYNUM)	return ERR_9004;
	
		*(unsigned int*)data=i+1;
			
		ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tWLANObject, sizeof(tWLANObject), data );
		if( ret >= 0 && i > 0)
		{
			mib_chain_get(MIB_MBSSIB_TBL, i, (void *)&Entry);
			Entry.instnum = (i+1);
			mib_chain_update( MIB_MBSSIB_TBL, (void*)&Entry, i );
		}

		return ret;
	     	break;
	     }
	case eCWMP_tDELOBJ:
	     {
	     	int ret;
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

//		printf("\ndata=%d\n",*(int*)data);
		num = mib_chain_total( MIB_MBSSIB_TBL );
		for( i=1; i<num;i++ )
		{
			if( !mib_chain_get( MIB_MBSSIB_TBL, i, (void*)&Entry ))
				continue;
			if(Entry.instnum == *(int*)data)
				break;
		}//for
		if(i==num) return ERR_9005;
	
		ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
		if( ret==0 )
		{
			Entry.instnum=0;
			Entry.wlanDisabled=1;
			mib_chain_update( MIB_MBSSIB_TBL, (void*)&Entry, i );
			ret=1;
		}
		return ret;
	     	break;
	     }
	case eCWMP_tUPDATEOBJ:
	     {
		struct CWMP_LINKNODE *old_table;

		//CWMPDBG( 1, ( stderr, "<%s:%d>\n", __FUNCTION__, __LINE__ ) );
	     	old_table = (struct CWMP_LINKNODE *)entity->next;
	     	entity->next = NULL;
	     	for( i=0; i<WLANENTRYNUM;i++ )
	     	{
	     		struct CWMP_LINKNODE *remove_entity=NULL;
			MIB_CE_MBSSIB_T *p,ssid_entity;

			if(i!=0){
				p = &ssid_entity;
				if( !mib_chain_get( MIB_MBSSIB_TBL, i, (void*)p ))
					continue;
					
				if(p->instnum!=(i+1))  continue;
	     		}
			
			if( (i==0)||(p->instnum==(i+1)))	
			{
				unsigned int num=1;
				if(i!=0)
					num=p->instnum;
				remove_entity = remove_SiblingEntity( &old_table, num );
				if( remove_entity!=NULL )
				{
					add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
				}else{					
					add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tWLANObject, sizeof(tWLANObject), &num );
				}	
		     	}
		}
	     	if( old_table )
	     		destroy_ParameterTable( (struct CWMP_NODE *)old_table );
	     	return 0;
	     }
	}
	return -1;


}
#endif

int getWLANConf(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned long bs=0,br=0,ps=0,pr=0;
	unsigned char buf[256]="";
	unsigned char vChar=0;
	unsigned int vUint=0;
	unsigned int wlaninst=0;
#ifdef WLAN_MBSSID
	MIB_CE_MBSSIB_T Entry;
#endif
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	wlaninst = getWLANConfInstNum( name );
#ifdef WLAN_MBSSID
	if( wlaninst<1 || wlaninst>5 )	return ERR_9007;
	if (!mib_chain_get(MIB_MBSSIB_TBL, wlaninst-1, (void *)&Entry)) return ERR_9002;
#else
	if( wlaninst!=1 ) return ERR_9007;
#endif


	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
#ifdef WLAN_MBSSID
	   if(wlaninst!=1)
		vChar = Entry.wlanDisabled;
	   else
#endif
		mib_get( MIB_WLAN_DISABLED, (void *)&vChar);

		*data = booldup( (vChar==0) );
	}else if( strcmp( lastname, "Status" )==0 )
	{
		int flags=0;
		if( getInFlags(wlan_name[wlaninst-1], &flags)==1 )
		{
			if (flags & IFF_UP)
				*data = strdup( "Up" );
			else
				*data = strdup( "Disabled" );
		}else
			*data = strdup( "Error" );
	}else if( strcmp( lastname, "BSSID" )==0 )
	{
		unsigned char macadd[MAC_ADDR_LEN];
		mib_get(MIB_ELAN_MAC_ADDR, (void *)macadd);
		sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", macadd[0], macadd[1],
			macadd[2], macadd[3], macadd[4], macadd[5]+wlaninst-1);
		*data=strdup(buf);
	}else if( strcmp( lastname, "MaxBitRate" )==0 )
	{
		mib_get(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&vChar);
		if( vChar==1 )
			*data=strdup( "Auto" );
		else
		{
			mib_get( MIB_WLAN_BAND, (void *)&vChar);
			if( vChar==1 ) //2.4 GHz (B)
				*data = strdup( "11" );
			else if( vChar==2 )//2.4 GHz (G)
				*data = strdup( "54" );
			else if( vChar==3 )//2.4 GHz (B+G)
				*data = strdup( "54" );
			else /*0, wifi_g==4, or wifi_bg==5?????*/
				*data = strdup( "" );//return ERR_9002;
		}
	}else if( strcmp( lastname, "Channel" )==0 )
	{
		mib_get( MIB_WLAN_CHAN_NUM, (void *)&vChar);
		*data = uintdup( (unsigned int)vChar );
	}else if( strcmp( lastname, "SSID" )==0 )
	{
	     /*for root ap, ssid and authType is not used => use MIB_WLAN_SSID and MIB_WLAN_AUTH_TYPE*/
#ifdef WLAN_MBSSID
	    if(wlaninst!=1)
		strcpy( buf, Entry.ssid );
	    else
#endif
		getMIB2Str(MIB_WLAN_SSID, buf);

		*data = strdup( buf );
	}else if( strcmp( lastname, "BeaconType" )==0 )
	{
#ifdef CONFIG_BOA_WEB_E8B_CH	//cathy
#ifdef WLAN_MBSSID
		if(wlaninst!=1)
			vChar = Entry.encrypt;
		else			
#endif		
			mib_get( MIB_WLAN_ENCRYPT, (void *)&vChar);
		if(vChar==ENCRYPT_DISABLED)
			*data = strdup( "None" );
		else if(vChar==ENCRYPT_WEP)
			*data = strdup( "Basic" );
#else
	
#ifdef WLAN_MBSSID
	    if(wlaninst!=1)
		vChar = Entry.wlanDisabled;
	    else
#endif
		mib_get( MIB_WLAN_DISABLED, (void *)&vChar);

		if(vChar==1) //disabled, so no beacon type
		{
			*data = strdup( "None" );
			return 0;
		}
		
#ifdef WLAN_MBSSID
		if(wlaninst!=1)
			vChar = Entry.encrypt;
		else
#endif
		mib_get( MIB_WLAN_ENCRYPT, (void *)&vChar);

		if( (vChar==ENCRYPT_WEP) || (vChar==ENCRYPT_DISABLED) )
			*data = strdup( "Basic" );
#endif		
		else if( vChar==ENCRYPT_WPA )
			*data = strdup( "WPA" );
		else if( vChar==ENCRYPT_WPA2 ) /*IEEE 802.11i*/
			*data = strdup( "11i" );
		else if( vChar==ENCRYPT_WPA2_MIXED ) /*WPA & WPA2*/
			*data = strdup( "WPAand11i" );
		else
			return ERR_9002;
#ifdef MAC_FILTER
	}else if( strcmp( lastname, "MACAddressControlEnabled" )==0 )
	{
		mib_get(MIB_WLAN_MAC_CTRL, (void *)&vChar);
		*data = booldup( vChar!=0 );
#endif /*MAC_FILTER*/
	}else if( strcmp( lastname, "Standard" )==0 )
	{		
		mib_get( MIB_WLAN_BAND, (void *)&vChar);
		if( vChar==1 ) //2.4 GHz (B)
			*data = strdup( "b" );
		else if( vChar==2 )//2.4 GHz (G)
			*data = strdup( "g" );
		else if( vChar==3 )//2.4 GHz (B+G)
			*data = strdup( "b,g" );
		else /*0, wifi_g==4, or wifi_bg==5?????*/
			*data = strdup( "" ); //return ERR_9002;
	}else if( strcmp( lastname, "WEPKeyIndex" )==0 )
	{
#ifdef WLAN_MBSSID
		MIB_CE_MBSSIB_WEP_T WEPEntry;
		if(wlaninst != 1){
			if (!mib_chain_get(MIB_MBSSIB_WEP_TBL, wlaninst-1, (void *)&WEPEntry)) return ERR_9002;
			vChar = WEPEntry.wepDefaultKey;
		}else
#endif
		mib_get( MIB_WLAN_WEP_DEFAULT_KEY, (void *)&vChar );

		vChar = vChar + 1;//mib's wepid is from 0 to 3
		*data = uintdup( (unsigned int)vChar );
	}else if( strcmp( lastname, "KeyPassphrase" )==0 )
	{
#if 1
		*data = strdup( "" ); /*return an empty string*/
#else
#ifdef WLAN_MBSSID
		memcpy( buf, Entry.wpaPSK, sizeof(Entry.wpaPSK) );
#else
		mib_get(MIB_WLAN_WPA_PSK, (void *)buf);
#endif //WLAN_MBSSID
		*data = strdup( buf );
#endif
	}else if( strcmp( lastname, "WEPEncryptionLevel" )==0 )
	{
#ifdef CONFIG_BOA_WEB_E8B_CH	//cathy
			//0:disable, 1:64, 2:128
#ifdef WLAN_MBSSID
		if(wlaninst != 1)
			vChar = Entry.wep;
		else
#endif
		mib_get( MIB_WLAN_WEP, (void *)&vChar);
		if(vChar == 0)
			*data = strdup( "Disabled" );
		else if (vChar == 1)
			*data = strdup( "40-bit" );
		else
			*data = strdup( "104-bit" );
#else
		*data = strdup( "Disabled,40-bit,104-bit" );
#endif
	}else if( strcmp( lastname, "BasicEncryptionModes" )==0 )
	{
#ifdef WLAN_MBSSID
		if(wlaninst != 1){
			vChar = Entry.encrypt;
		}else
#endif
		mib_get( MIB_WLAN_ENCRYPT, (void *)&vChar);

		if( vChar==ENCRYPT_WEP )
			*data = strdup( "WEPEncryption" );
		else if( vChar==ENCRYPT_DISABLED )
			*data = strdup( "None" );
		else
		{
#ifdef WLAN_MBSSID
			if(wlaninst != 1){
				vChar = Entry.cwmp_WLAN_BasicEncry;
			}else
#endif
			mib_get( CWMP_WLAN_BASICENCRY, (void *)&vChar);

			if( vChar==1 )
				*data = strdup( "WEPEncryption" );
			else
				*data = strdup( "None" );
		}
	}else if( strcmp( lastname, "BasicAuthenticationMode" )==0 )
	{
#ifdef CONFIG_BOA_WEB_E8B_CH	//cathy
#ifdef WLAN_MBSSID
		if((wlaninst != 1))
			vChar = Entry.authType;
		else
#endif
		mib_get(MIB_WLAN_AUTH_TYPE, (void *)&vChar);
		if(vChar==0)
			*data = strdup( "OpenSystem" );
		else if(vChar==1)
			*data = strdup( "SharedKey" );
		else
			*data = strdup( "Both" );
#else
#ifdef WLAN_MBSSID
		if((wlaninst != 1))
			vChar = Entry.enable1X;
		else
#endif
		mib_get( MIB_WLAN_ENABLE_1X, (void *)&vChar);

		if(vChar)
			*data = strdup( "EAPAuthentication" );
		else
			*data = strdup( "None" );
#endif		
	}else if( strcmp( lastname, "WPAEncryptionModes" )==0 )
	{
#ifdef ENABLE_WPAAES_WPA2TKIP	
#ifdef WLAN_MBSSID
		if(wlaninst != 1)
			vChar = Entry.unicastCipher;
		else
#endif
#endif
		mib_get( MIB_WLAN_WPA_CIPHER_SUITE, (void *)&vChar);
		if( vChar==WPA_CIPHER_TKIP )
			*data = strdup( "TKIPEncryption" );
		else if( vChar==WPA_CIPHER_AES )
			*data = strdup( "AESEncryption" );
		else
			return ERR_9002;
	}else if( strcmp( lastname, "WPAAuthenticationMode" )==0 )
	{
#ifdef WLAN_MBSSID
		if(wlaninst != 1)
			vChar = Entry.wpaAuth;
		else
#endif
		mib_get( MIB_WLAN_WPA_AUTH, (void *)&vChar);

		if(vChar==WPA_AUTH_PSK)
			*data = strdup( "PSKAuthentication" );
		else
			*data = strdup( "EAPAuthentication" );
	}else if( strcmp( lastname, "IEEE11iEncryptionModes" )==0 )
	{
#ifdef ENABLE_WPAAES_WPA2TKIP	
#ifdef WLAN_MBSSID
		if(wlaninst != 1)
			vChar = Entry.wpa2UnicastCipher;
		else
#endif
#endif
		mib_get( MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&vChar);
		if( vChar==WPA_CIPHER_TKIP )
			*data = strdup( "TKIPEncryption" );
		else if( vChar==WPA_CIPHER_AES )
			*data = strdup( "AESEncryption" );
		else
			return ERR_9002;
	}else if( strcmp( lastname, "IEEE11iAuthenticationMode" )==0 )
	{
#ifdef WLAN_MBSSID
		if(wlaninst != 1)
			vChar = Entry.wpaAuth;
		else
#endif
		mib_get( MIB_WLAN_WPA_AUTH, (void *)&vChar);

		if(vChar==WPA_AUTH_PSK)
			*data = strdup( "PSKAuthentication" );
		else
			*data = strdup( "EAPAuthentication" );
	}else if( strcmp( lastname, "PossibleChannels" )==0 )
	{
		unsigned char dm;
		mib_get( MIB_HW_REG_DOMAIN, (void *)&dm);
		switch(dm)
		{
		case 1: //1-11
		case 2:
			*data = strdup("1-11");
			break;
		case 3: //1-13
			*data = strdup("1-13");
			break;
		case 4: //10-11
			*data = strdup("10-11");
			break;
		case 5: //10-13
			*data = strdup("10-13");
			break;
		case 6: //1-14
			*data = strdup("1-14");
			break;
		default:
			return ERR_9002;
		}
	}else if( strcmp( lastname, "BasicDataTransmitRates" )==0 )
	{
		unsigned short uShort;
		mib_get(MIB_WLAN_BASIC_RATE, (void *)&uShort);
		getRateStr( uShort, buf );
		*data = strdup( buf );		
	}else if( strcmp( lastname, "OperationalDataTransmitRates" )==0 )
	{
		unsigned short uShort;
		mib_get(MIB_WLAN_SUPPORTED_RATE, (void *)&uShort);
		getRateStr( uShort, buf );
		*data = strdup( buf );		
	}else if( strcmp( lastname, "PossibleDataTransmitRates" )==0 )
	{
		unsigned short uShort;
		mib_get(MIB_WLAN_SUPPORTED_RATE, (void *)&uShort);
		getRateStr( uShort, buf );
		*data = strdup( buf );
	}else if( strcmp( lastname, "BeaconAdvertisementEnabled" )==0 )
	{
#ifdef WLAN_MBSSID
	    if(wlaninst!=1)
		vChar = Entry.bcnAdvtisement;
	    else
#endif
		mib_get( MIB_WLAN_BEACON_ADVERTISEMENT, (void *)&vChar);
		*data = booldup( (vChar!=0) );
	}else if( strcmp( lastname, "SSIDAdvertisementEnabled" )==0 )
	{
#ifdef WLAN_MBSSID
	    if(wlaninst!=1)
		vChar = Entry.hidessid;
	    else
#endif		
		mib_get( MIB_WLAN_HIDDEN_SSID, (void *)&vChar);
		*data = booldup( (vChar==0) );
	}else if( strcmp( lastname, "RadioEnabled" )==0 )
	{
#ifdef WLAN_MBSSID
	    if(wlaninst!=1)
		vChar = Entry.wlanDisabled;
	    else
#endif
		mib_get( MIB_WLAN_DISABLED, (void *)&vChar);

		*data = booldup( (vChar==0) );
	}else if( strcmp( lastname, "AutoRateFallBackEnabled" )==0 )
	{
		mib_get(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&vChar);
		*data = booldup( (vChar==1) );
	}else if( strcmp( lastname, "LocationDescription" )==0 )
	{
		*data = strdup( gLocationDescription );
	}else if( strcmp( lastname, "ChannelsInUse" )==0 )
	{
		mib_get( MIB_WLAN_CHAN_NUM, (void *)&vChar);
		sprintf( buf, "%u", vChar );
		*data = strdup( buf );
	}else if( strcmp( lastname, "DeviceOperationMode" )==0 )
	{
		mib_get(MIB_WLAN_MODE, (void *)&vChar);
		if(vChar==AP_MODE)
			*data = strdup( "InfrastructureAccessPoint" );
		else if(vChar==CLIENT_MODE)
			*data = strdup( "WirelessStation" );
		else if(vChar==WDS_MODE)
			*data = strdup( "WirelessBridge" );
		else /*WirelessRepeater or others*/
			return ERR_9002;
	}else if( strcmp( lastname, "AuthenticationServiceMode" )==0 )
	{
		*data = strdup( "None" );
	}else if( strcmp( lastname, "TotalBytesSent" )==0 )
	{
		if( getInterfaceStat( wlan_name[wlaninst-1], &bs, &br, &ps, &pr ) < 0 )
			return -1;
		*data = uintdup( bs );
	}else if( strcmp( lastname, "TotalBytesReceived" )==0 )
	{
		if( getInterfaceStat( wlan_name[wlaninst-1], &bs, &br, &ps, &pr ) < 0 )
			return -1;
		*data = uintdup( br );
	}else if( strcmp( lastname, "TotalPacketsSent" )==0 )
	{
		if( getInterfaceStat( wlan_name[wlaninst-1], &bs, &br, &ps, &pr ) < 0 )
			return -1;
		*data = uintdup( ps );
	}else if( strcmp( lastname, "TotalPacketsReceived" )==0 )
	{
		if( getInterfaceStat( wlan_name[wlaninst-1], &bs, &br, &ps, &pr ) < 0 )
			return -1;
		*data = uintdup( pr );
#ifdef _PRMT_X_CT_COM_WLAN_
	}else if( strcmp( lastname, "X_CT-COM_SSIDHide" )==0 )
	{
#ifdef WLAN_MBSSID
	    if(wlaninst!=1)
		vChar = Entry.hidessid;
	    else
#endif	
		mib_get( MIB_WLAN_HIDDEN_SSID, (void *)&vChar);
		*data = booldup( (vChar!=0) );
	}else if( strcmp( lastname, "X_CT-COM_Powerlevel" )==0 )
	{	//MIB_TX_POWER=> 0:15mW, 1:30mW, 2:60mW
		mib_get( MIB_TX_POWER, (void *)&vChar);
//added by xl_yue: 
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC) || defined(CONFIG_BOA_WEB_E8B_CH)
		if(vChar < 5)
			vUint = vChar + 1;
		else
			return ERR_9002;
#else
		if( vChar==0 ) 
			vUint = 3;
		else if( vChar==1 ) 
			vUint = 2;
		else if( vChar==2 ) 
			vUint = 1;
		else 
			return ERR_9002;
#endif
		*data = uintdup( vUint );
	}else if( strcmp( lastname, "X_CT-COM_PowerValue" )==0 )
	{	//mW-to-dBm Power Conversion =>  dBm = 10*log(mW)
		mib_get( MIB_TX_POWER, (void *)&vChar);
//added by xl_yue: 
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC) || defined(CONFIG_BOA_WEB_E8B_CH)
		if( vChar==0 ) 
			vUint=18;/*100%*/
		else if( vChar==1 ) 
			vUint=17;/*80%*/
		else if( vChar==2 ) 
			vUint=15;/*50%*/
		else if( vChar==3 ) 
			vUint=12;/*25%*/
		else if( vChar==4)
			vUint=8;/*10%*/
		else
			return ERR_9002;
#else
		if( vChar==0 ) 
			vUint=12;/*15mW*/
		else if( vChar==1 ) 
			vUint=15;/*30mW*/
		else if( vChar==2 ) 
			vUint=18;/*60mW*/
		else 
			return ERR_9002;
#endif
		*data = uintdup( vUint );
	}else if(strcmp(lastname,"X_CT-COM_APModuleEnable")==0){
		mib_get( MIB_WLAN_DISABLED, (void *)&vChar);
		*data = booldup( (vChar==0) );
#endif //#ifdef _PRMT_X_CT_COM_WLAN_
	}else if( strcmp( lastname, "TotalAssociations" )==0 )
	{
		if( loadWLANAssInfoByInstNum(wlaninst)< 0 )
			*data = uintdup( 0 );
		else
			*data = uintdup( gWLANTotalClients );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setWLANConf(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char	vChar=0;
	unsigned int wlaninst=0;
#ifdef WLAN_MBSSID
	MIB_CE_MBSSIB_T Entry;
#endif
	
	if( (name==NULL) || (entity==NULL)) return -1;
#ifdef _PRMT_X_CT_COM_DATATYPE
	int tmpint=0;
	unsigned int tmpuint=0;
	int tmpbool=0;
	if(changestring2int(data,entity->info->type,type,&tmpint,&tmpuint,&tmpbool)<0)
		return ERR_9006;
#else
	if( entity->info->type!=type ) return ERR_9006;
#endif
	if( data==NULL ) return ERR_9007;


	wlaninst = getWLANConfInstNum( name );
#ifdef WLAN_MBSSID
	if( wlaninst<1 || wlaninst>5 )	return ERR_9007;
	if (!mib_chain_get(MIB_MBSSIB_TBL, wlaninst-1, (void *)&Entry)) return ERR_9002;
#else
	if( wlaninst!=1 ) return ERR_9007;
#endif

	if( strcmp( lastname, "Enable" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?1:0;
#ifdef WLAN_MBSSID
	   if( wlaninst!=1 )
	   {
		Entry.wlanDisabled = vChar;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
	   }else
#endif
		mib_set(MIB_WLAN_DISABLED, (void *)&vChar);

#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "MaxBitRate" )==0 )
	{
		if( strlen(buf)==0 ) return ERR_9007;
		if( strcmp(buf,"Auto")!=0 ) return ERR_9001;
		vChar = 1;
		mib_set(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&vChar);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "Channel" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *i = &tmpuint;		
#else
		unsigned int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		//check if an valid channel depending on MIB_HW_REG_DOMAIN??
		if(*i!=0) //0:auto
		{
			unsigned char dm;
			int valid = 0;
			
			mib_get( MIB_HW_REG_DOMAIN, (void *)&dm);
			switch(dm)
			{
			case 1: //1-11
			case 2:
				if( (*i>=1) && (*i<=11) ) valid = 1;
				break;
			case 3: //1-13
				if( (*i>=1) && (*i<=13) ) valid = 1;
				break;
			case 4: //10-11
				if( (*i>=10) && (*i<=11) ) valid = 1;
				break;
			case 5: //10-13
				if( (*i>=10) && (*i<=11) ) valid = 1;
				break;
			case 6: //1-14
				if( (*i>=1) && (*i<=14) ) valid = 1;
				break;
			}
			if( valid==0 ) return ERR_9007;			
		}
		vChar = *i;
		mib_set( MIB_WLAN_CHAN_NUM, (void *)&vChar);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "SSID" )==0 )
	{
		//MIB_WLAN_SSID
		if( buf==NULL ) return ERR_9007;
		if( (strlen(buf)==0) || (strlen(buf)>=MAX_SSID_LEN) ) return ERR_9007;
#ifdef WLAN_MBSSID
	if( wlaninst!=1 )
	{
		strcpy( Entry.ssid, buf );
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
	}else
#endif
		mib_set( MIB_WLAN_SSID, buf );

#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "BeaconType" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( strcmp( buf, "None" )==0 )
		{
#ifdef CONFIG_BOA_WEB_E8B_CH	//cathy
			vChar = ENCRYPT_DISABLED;
#ifdef WLAN_MBSSID
			if(wlaninst != 1){
				Entry.encrypt = vChar;
				Entry.authType = vChar;
				Entry.cwmp_WLAN_BasicEncry = vChar;
				mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
			}else {
#endif	
				mib_set( MIB_WLAN_ENCRYPT, (void *)&vChar);	
				mib_set( MIB_WLAN_AUTH_TYPE, (void *)&vChar);	//open mode
				mib_set( CWMP_WLAN_BASICENCRY, (void *)&vChar);
#ifdef WLAN_MBSSID
			}
#endif
			
#else
			vChar = 1;
#ifdef WLAN_MBSSID
		    if( wlaninst!=1 )
		    {
			Entry.wlanDisabled = vChar;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
		    }else
#endif
			mib_set( MIB_WLAN_DISABLED, (void *)&vChar);
#endif	
		}else if( strcmp( buf, "Basic" )==0 )
		{
#ifdef CONFIG_BOA_WEB_E8B_CH	//cathy
			vChar = ENCRYPT_WEP;
#ifdef WLAN_MBSSID
			if( wlaninst!=1 )
			{
				Entry.encrypt = vChar;
				Entry.cwmp_WLAN_BasicEncry = 1;	//basic:wep encryption
				//mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
			}else {				
#endif	
				mib_set( MIB_WLAN_ENCRYPT, (void *)&vChar);
				vChar = 1;	//basic:wep encryption
				mib_set( CWMP_WLAN_BASICENCRY, (void *)&vChar);
#ifdef WLAN_MBSSID
			}
#endif			

			//0:disable, 1:64, 2:128
#ifdef WLAN_MBSSID
			if(wlaninst != 1)
				vChar = Entry.wep;
			else
#endif
			mib_get( MIB_WLAN_WEP, (void *)&vChar);

			if(vChar==0)
			{
				vChar=WEP64;
#ifdef WLAN_MBSSID
				if(wlaninst != 1){
					Entry.wep = vChar;
					mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
				}else
#endif
				mib_set( MIB_WLAN_WEP, (void *)&vChar);

			}
			vChar = 2;	//both open and shared mode
#ifdef WLAN_MBSSID
			if(wlaninst!=1)
			{
				Entry.authType = vChar;
				mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
			}else
#endif
				mib_set(MIB_WLAN_AUTH_TYPE, (void *)&vChar);
#else		
#ifdef WLAN_MBSSID
			if(wlaninst != 1)
				vChar = Entry.encrypt;
			else
#endif
			mib_get( MIB_WLAN_ENCRYPT, (void *)&vChar);

			if( (vChar==ENCRYPT_WPA)||(vChar==ENCRYPT_WPA2)||(vChar==ENCRYPT_WPA2_MIXED) )
			{
				unsigned char vch=0;
#ifdef WLAN_MBSSID
				if(wlaninst != 1)
					vch = Entry.cwmp_WLAN_BasicEncry;
				else
#endif
				mib_get( CWMP_WLAN_BASICENCRY, (void *)&vch);

				if( vch==1 )//basic:wep encryption
				{
					//0:disable, 1:64, 2:128
#ifdef WLAN_MBSSID
					if(wlaninst != 1)
						vChar = Entry.wep;
					else
#endif
					mib_get( MIB_WLAN_WEP, (void *)&vChar);

					if(vChar==0)
					{
						vChar=WEP64;
#ifdef WLAN_MBSSID
						if(wlaninst != 1){
							Entry.wep = vChar;
							//mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
						}else
#endif
						mib_set( MIB_WLAN_WEP, (void *)&vChar);

					}

					vChar = ENCRYPT_WEP;
				}else //basic:none encryption
					vChar = ENCRYPT_DISABLED;
#ifdef WLAN_MBSSID
				if(wlaninst != 1){
					Entry.encrypt = vChar;
					mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
				}else
#endif									
				mib_set( MIB_WLAN_ENCRYPT, (void *)&vChar);					

			}
#endif			
		}else if( strcmp( buf, "WPA" )==0 )
		{
				vChar = ENCRYPT_WPA;
#ifdef WLAN_MBSSID
				if(wlaninst != 1){
					Entry.encrypt = vChar;
					Entry.authType = 0;	//open mode
					mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
				}else {
#endif									
				mib_set( MIB_WLAN_ENCRYPT, (void *)&vChar);
				vChar = 0;
				mib_set(MIB_WLAN_AUTH_TYPE, (void *)&vChar);
#ifdef WLAN_MBSSID
				}
#endif
		}else if( strcmp( buf, "11i" )==0 )
		{
				vChar = ENCRYPT_WPA2;
#ifdef WLAN_MBSSID
				if(wlaninst != 1){
					Entry.encrypt = vChar;
					Entry.authType = 0;	//open mode
					mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
				}else {
#endif									
				mib_set( MIB_WLAN_ENCRYPT, (void *)&vChar);
				vChar = 0;
				mib_set(MIB_WLAN_AUTH_TYPE, (void *)&vChar);
#ifdef WLAN_MBSSID
				}
#endif
		}else if( strcmp( buf, "WPAand11i" )==0 )
		{
				vChar = ENCRYPT_WPA2_MIXED;
#ifdef WLAN_MBSSID
				if(wlaninst != 1){
					Entry.encrypt = vChar;
					Entry.authType = 0;	//open mode
					mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
				}else {
#endif									
				mib_set( MIB_WLAN_ENCRYPT, (void *)&vChar);
				vChar = 0;
				mib_set(MIB_WLAN_AUTH_TYPE, (void *)&vChar);
#ifdef WLAN_MBSSID
				}
#endif				
		}else
			return ERR_9007;
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
#ifdef MAC_FILTER
	}else if( strcmp( lastname, "MACAddressControlEnabled" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vChar = (*i==1)?1:0;
		mib_set( MIB_WLAN_MAC_CTRL, (void *)&vChar);
		{
			unsigned char eth_mac_ctrl=0,mac_out_dft=1;
			mib_get(MIB_ETH_MAC_CTRL, (void *)&eth_mac_ctrl);
			if( vChar==1 || eth_mac_ctrl==1 )
				mac_out_dft=0;//0:deny, 1:allow			
			mib_set(MIB_MACF_OUT_ACTION, (void *)&mac_out_dft);
		}
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_MACFILTER, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
#endif /*MAC_FILTER*/
	}else if( strcmp( lastname, "WEPKeyIndex" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *i = &tmpuint;		
#else
		unsigned int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		if( (*i<1) || (*i>4) ) return ERR_9007;
		vChar = (unsigned char)*i;
		vChar = vChar - 1; //mib's wepid is from 0 to 3
#ifdef WLAN_MBSSID
		if(wlaninst != 1){
			MIB_CE_MBSSIB_WEP_T WEPEntry;
			if (!mib_chain_get(MIB_MBSSIB_WEP_TBL, wlaninst-1, (void *)&WEPEntry)) return ERR_9002;
			WEPEntry.wepDefaultKey = vChar;
			mib_chain_update(MIB_MBSSIB_WEP_TBL, (void *)&WEPEntry, wlaninst-1);
		}else
#endif
		mib_set( MIB_WLAN_WEP_DEFAULT_KEY, (void *)&vChar );

#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "KeyPassphrase" )==0 )
	{
		unsigned char pskfmt;
		if( buf==NULL ) return ERR_9007;
		if( (strlen(buf)<8) || (strlen(buf)>63) ) return ERR_9007;
		pskfmt = 0; //0:Passphrase,   1:hex
#ifdef WLAN_MBSSID
		if(wlaninst != 1){
			strcpy( Entry.wpaPSK, buf );
			Entry.wpaPSKFormat = pskfmt;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
		}else
#endif
		{
			mib_set(MIB_WLAN_WPA_PSK, (void *)buf);
			mib_set(MIB_WLAN_WPA_PSK_FORMAT, (void *)&pskfmt);
		}
		
		{
			int chain_id;
			CWMP_PSK_T	*pEntry=NULL, psk_entity;
			
			chain_id = getPSKChainId( 1 ); //default presharedkey
			if( chain_id >= 0 )
			{
				pEntry = &psk_entity;
				if( mib_chain_get(CWMP_PSK_TBL, chain_id, (void*)pEntry ) )
				{
					strcpy( pEntry->keypassphrase, buf );
					mib_chain_update(CWMP_PSK_TBL, (char *)pEntry, chain_id);
				}
			}
		}
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}
#ifdef CONFIG_BOA_WEB_E8B_CH	//cathy
	else if( strcmp( lastname, "WEPEncryptionLevel" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( strcmp( buf, "Disabled" )==0 )
			vChar = WEP_DISABLED;
		else if( strcmp( buf, "40-bit" )==0 )
			vChar = WEP64;
		else if( strcmp( buf, "104-bit" )==0 )
			vChar = WEP128;
		else return ERR_9007;
#ifdef WLAN_MBSSID
		if(wlaninst != 1){
			Entry.wep = vChar;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
		}
		else
#endif
		mib_set( MIB_WLAN_WEP, (void *)&vChar);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}
#endif
	else if( strcmp( lastname, "BasicEncryptionModes" )==0 )
	{
		unsigned char c_mode;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;

#ifdef WLAN_MBSSID
		if(wlaninst != 1)
			c_mode = Entry.encrypt;
		else
#endif		
		mib_get( MIB_WLAN_ENCRYPT, (void *)&c_mode);

		if( strcmp( buf, "WEPEncryption" )==0 )
		{
			if( c_mode==ENCRYPT_DISABLED )
			{
				vChar = ENCRYPT_WEP;
#ifdef WLAN_MBSSID
				if(wlaninst != 1){
					Entry.encrypt = vChar;
					if(Entry.wep==WEP_DISABLED)
						Entry.wep=WEP64;
					//mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
				}else
#endif			
				{
					mib_set( MIB_WLAN_ENCRYPT, (void *)&vChar);
					{
						unsigned char c_key=0;
						mib_get( MIB_WLAN_WEP, (void *)&c_key);
						if( c_key==WEP_DISABLED )
						{
							c_key=WEP64;
							mib_set( MIB_WLAN_WEP, (void *)&c_key);
						}
					}
				}

			}
			
			vChar=1;
#ifdef WLAN_MBSSID
			if(wlaninst != 1){
				Entry.cwmp_WLAN_BasicEncry = vChar;
				mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
			}else
#endif									
			mib_set( CWMP_WLAN_BASICENCRY, (void *)&vChar);

		}else if( strcmp( buf, "None" )==0 )
		{
			if( c_mode==ENCRYPT_WEP )
			{
				vChar = ENCRYPT_DISABLED;
#ifdef WLAN_MBSSID
				if(wlaninst != 1){
					Entry.encrypt = vChar;
					//mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
				}else
#endif									
				mib_set( MIB_WLAN_ENCRYPT, (void *)&vChar);

			}
			
			vChar=0;
#ifdef WLAN_MBSSID
			if(wlaninst != 1){
				Entry.cwmp_WLAN_BasicEncry = vChar;
				mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
			}else
#endif									
			mib_set( CWMP_WLAN_BASICENCRY, (void *)&vChar);

		}else
			return ERR_9007;
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "BasicAuthenticationMode" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
#ifdef CONFIG_BOA_WEB_E8B_CH	//cathy
		if( strcmp( buf, "OpenSystem")==0 )
			vChar=0;
		else if( strcmp( buf, "SharedKey")==0 )
			vChar=1;
		else if( strcmp( buf, "Both")==0 )
			vChar=2;
		else
#else			
		if( strcmp( buf, "None")==0 )
			vChar=0;
		else if( strcmp( buf, "EAPAuthentication")==0 )
			vChar=1;
		else		
#endif
			return ERR_9007;

#ifndef CONFIG_BOA_WEB_E8B_CH	//cathy
#ifdef WLAN_MBSSID
		if(wlaninst != 1){
			Entry.enable1X = vChar;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
		}else
#endif		
			mib_set( MIB_WLAN_ENABLE_1X, (void *)&vChar);
		vChar=AUTH_BOTH;
#endif

#ifdef WLAN_MBSSID
		if(wlaninst!=1)
		{
			Entry.authType = vChar;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
		}else
#endif
			mib_set(MIB_WLAN_AUTH_TYPE, (void *)&vChar);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "WPAEncryptionModes" )==0 )
	{
		unsigned char c_mode;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;

		/*in current web, wpa only supports tkip*/
		if( strcmp( buf, "TKIPEncryption" )==0 )
			vChar=WPA_CIPHER_TKIP;
#ifdef ENABLE_WPAAES_WPA2TKIP
		else if( strcmp( buf, "AESEncryption" )==0 )
			vChar=WPA_CIPHER_AES;
#endif
		else
			return ERR_9001;
#ifdef ENABLE_WPAAES_WPA2TKIP		
#ifdef WLAN_MBSSID
		if(wlaninst!=1)
		{
			Entry.unicastCipher = vChar;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
		}else
#endif
#endif
		mib_set( MIB_WLAN_WPA_CIPHER_SUITE, (void *)&vChar);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "WPAAuthenticationMode" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( strcmp( buf, "PSKAuthentication")==0 )
			vChar=WPA_AUTH_PSK;
		else if( strcmp( buf, "EAPAuthentication")==0 )
			vChar=WPA_AUTH_AUTO;
		else
			return ERR_9001;

#ifdef WLAN_MBSSID
		if(wlaninst != 1){
			Entry.wpaAuth = vChar;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
		}else
#endif		
		mib_set( MIB_WLAN_WPA_AUTH, (void *)&vChar);

#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "IEEE11iEncryptionModes" )==0 )
	{
		unsigned char c_mode;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;

		/*in current web, wpa2 only supports aes*/
		if( strcmp( buf, "AESEncryption" )==0 )
			vChar=WPA_CIPHER_AES;
#ifdef ENABLE_WPAAES_WPA2TKIP
		else if( strcmp( buf, "TKIPEncryption" )==0 )
			vChar=WPA_CIPHER_TKIP;
#endif
		else
			return ERR_9001;
#ifdef ENABLE_WPAAES_WPA2TKIP
#ifdef WLAN_MBSSID
		if(wlaninst!=1)
		{
			Entry.wpa2UnicastCipher = vChar;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
		}else
#endif
#endif
		mib_set( MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&vChar);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "IEEE11iAuthenticationMode" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( strcmp( buf, "PSKAuthentication")==0 )
			vChar=WPA_AUTH_PSK;
		else if( strcmp( buf, "EAPAuthentication")==0 )
			vChar=WPA_AUTH_AUTO;
		else
			return ERR_9001;

#ifdef WLAN_MBSSID
		if(wlaninst != 1){
			Entry.wpaAuth = vChar;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
		}else
#endif					
		mib_set( MIB_WLAN_WPA_AUTH, (void *)&vChar);

#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "BasicDataTransmitRates" )==0 )
	{
		unsigned short uShort;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( setRateStr( buf, &uShort )<0 ) return ERR_9007;
		mib_set(MIB_WLAN_BASIC_RATE, (void *)&uShort);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "OperationalDataTransmitRates" )==0 )
	{
		unsigned short uShort;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( setRateStr( buf, &uShort )<0 ) return ERR_9007;
		mib_set(MIB_WLAN_SUPPORTED_RATE, (void *)&uShort);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "BeaconAdvertisementEnabled" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
#ifdef WLAN_MBSSID
	   if( wlaninst!=1 )
	   {
		Entry.bcnAdvtisement = vChar;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
	   }else
#endif
		mib_set( MIB_WLAN_BEACON_ADVERTISEMENT, (void *)&vChar);

#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "SSIDAdvertisementEnabled" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?1:0;
#ifdef WLAN_MBSSID
	   if( wlaninst!=1 )
	   {
		Entry.hidessid = vChar;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
	   }else
#endif
		mib_set( MIB_WLAN_HIDDEN_SSID, (void *)&vChar);

#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "RadioEnabled" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?1:0;
#ifdef WLAN_MBSSID
	   if( wlaninst!=1 )
	   {
		Entry.wlanDisabled = vChar;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
	   }else
#endif
		mib_set(MIB_WLAN_DISABLED, (void *)&vChar);

#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "AutoRateFallBackEnabled" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		if( i==NULL ) return ERR_9007;
		vChar = (*i==1)?1:0;
		mib_set(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&vChar);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "LocationDescription" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 )
			strcpy( gLocationDescription, "" );
		else if( strlen(buf)<4096 )
			strcpy( gLocationDescription, buf );
		else 
			return ERR_9007;
	}else if( strcmp( lastname, "DeviceOperationMode" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strcmp(buf, "InfrastructureAccessPoint" )==0 )
			vChar=AP_MODE;
		else if( strcmp(buf, "WirelessStation" )==0 )
#ifdef WLAN_CLIENT
			vChar=CLIENT_MODE;
#else
			return ERR_9001;
#endif
		else if( strcmp(buf, "WirelessBridge" )==0 )
			vChar=WDS_MODE;
		else
			return ERR_9007;
		
		mib_set(MIB_WLAN_MODE, (void *)&vChar);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "AuthenticationServiceMode" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strcmp(buf, "None" )!=0 ) return ERR_9001;
#ifdef _PRMT_X_CT_COM_WLAN_
	}else if( strcmp( lastname, "X_CT-COM_SSIDHide" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif	
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
#ifdef WLAN_MBSSID
	   if( wlaninst!=1 )
	   {
		Entry.hidessid = vChar;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
	   }else
#endif
		mib_set(MIB_WLAN_HIDDEN_SSID, (void *)&vChar);

#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if( strcmp( lastname, "X_CT-COM_Powerlevel" )==0 )
	{
#ifdef _PRMT_X_CT_COM_DATATYPE
		unsigned int *i = &tmpuint;		
#else
		unsigned int *i = data;
#endif	
		if( i==NULL ) return ERR_9007;
		if( *i<0 || *i>5 ) return ERR_9007;
//added by xl_yue: 
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC) || defined(CONFIG_BOA_WEB_E8B_CH)
		if((*i) >= 1 && (*i) <= 5)
			vChar = (*i) - 1;
		else
			return ERR_9001;
#else
		if( *i==1 )
			vChar = 2;
		else if( *i==2 )
			vChar = 1;
		else if( *i==3 )
			vChar = 0;
		else //*i==4,5
			return ERR_9001;
#endif
		mib_set(MIB_TX_POWER, (void *)&vChar);
#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif
	}else if(strcmp(lastname,"X_CT-COM_APModuleEnable")==0){
#ifdef _PRMT_X_CT_COM_DATATYPE
		int *i = &tmpbool;		
#else
		int *i = data;
#endif
		
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?1:0;
		mib_set(MIB_WLAN_DISABLED, (void *)&vChar);

#ifdef _CWMP_APPLY_
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 1;
#endif

#endif //#ifdef _PRMT_X_CT_COM_WLAN_
	}else{
		return ERR_9005;
	}
	
	return 0;
}

/**************************************************************************************/
/* utility functions*/
/**************************************************************************************/
extern unsigned int getInstNum( char *name, char *objname );
unsigned int getWLANConfInstNum( char *name )
{
	return getInstNum( name, "WLANConfiguration" );
}

unsigned int getWEPInstNum( char *name )
{
	return getInstNum( name, "WEPKey" );
}

unsigned int getAssDevInstNum( char *name )
{
	return getInstNum( name, "AssociatedDevice" );
}

unsigned int getPreSharedKeyInstNum( char *name )
{
	return getInstNum( name, "PreSharedKey" );
}


char WLANASSFILE[] = "/tmp/stainfo";
int updateWLANAssociations( void )
{
	int i;
	time_t c_time=0;
	
	c_time = time(NULL);
	if( c_time >= gWLANAssUpdateTime+WLANUPDATETIME )
	{
//		if(gWLANAssociations==NULL)
//		{
//			gWLANAssociations = malloc( sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1));
//			if(gWLANAssociations==NULL) return -1;
//		}


		for(i=0;i<WLAN_IF_NUM;i++)
		{
			char filename[32];
			FILE *fp=NULL;
			int  has_info;
			
			has_info=1;
			memset( gWLANAssociations, 0, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1) );	
			if ( getWlStaInfo( (char*)wlan_name[i],  (WLAN_STA_INFO_T *)gWLANAssociations ) < 0 )
			{
				memset( gWLANAssociations, 0, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1) );
				has_info=0;
			}

#if 0
{
			int j;
			int found=0;
			for (j=1; j<=MAX_STA_NUM; j++)
			{
				WLAN_STA_INFO_T *pInfo;
				pInfo = (WLAN_STA_INFO_T*)&gWLANAssociations[j*sizeof(WLAN_STA_INFO_T)];
				if (pInfo->aid && (pInfo->flag & STA_INFO_FLAG_ASOC))
					found++;
			}
			fprintf( stderr, "wlan_name:%s, found stations:%d\n ", wlan_name[i], found );
}
#endif
			
			sprintf( filename, "%s.%s", WLANASSFILE, wlan_name[i] );
			fp=fopen( filename, "wb" );
			if(fp)
			{
				if(has_info)
				{
					fwrite( gWLANAssociations, 1, sizeof(WLAN_STA_INFO_T)*(MAX_STA_NUM+1), fp );
				}
				fclose(fp);
			}	
		}
#if 0
{
		//test wireless association clients
		srand( (unsigned int)time(NULL) );
		for (i=1; i<=MAX_STA_NUM; i++)
		{
			int rnum;
			
			pInfo = (WLAN_STA_INFO_T*)&gWLANAssociations[i*sizeof(WLAN_STA_INFO_T)];
			if( (rand() % 4 )==2 )
			{
				pInfo->flag =  pInfo->flag | STA_INFO_FLAG_ASOC;
				pInfo->aid=i;
				pInfo->addr[0]= (unsigned int)i;
				fprintf( stderr, "Add one wlanassociation client: mac=%02x\n", pInfo->addr[0] );
			}
			
		}
}
#endif	
		
		gWLANAssUpdateTime = c_time;
	}
	return 0;
}


int loadWLANAssInfoByInstNum( unsigned int instnum )
{
	char filename[32];
	FILE *fp=NULL;
	int  wlanid,found=0;
	
	if( instnum==0 || instnum>WLAN_IF_NUM ) return -1;

	if( updateWLANAssociations()< 0 )
	{
		gWLANIDForAssInfo = -1;
		memset( gWLANAssociations, 0, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1) );
		gWLANTotalClients=0;
		return -1;
	}

	wlanid = instnum-1;
	if( gWLANIDForAssInfo==wlanid ) return 0;
	
	gWLANIDForAssInfo = -1;
	memset( gWLANAssociations, 0, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1) );
	gWLANTotalClients=0;
	
	sprintf( filename, "%s.%s", WLANASSFILE, wlan_name[wlanid] );
	fp=fopen( filename, "rb" );
	if(fp)
	{
		int i;
		WLAN_STA_INFO_T *pInfo;
		
		fread( gWLANAssociations,  1, sizeof(WLAN_STA_INFO_T)*(MAX_STA_NUM+1), fp );	
		fclose(fp);
		
		for (i=1; i<=MAX_STA_NUM; i++)
		{
			pInfo = (WLAN_STA_INFO_T*)&gWLANAssociations[i*sizeof(WLAN_STA_INFO_T)];
			if (pInfo->aid && (pInfo->flag & STA_INFO_FLAG_ASOC))
				found++;
		}				
		gWLANTotalClients = found;
	}
	
	return 0;
}


int getWLANSTAINFO(int id, WLAN_STA_INFO_T *info)
{
	WLAN_STA_INFO_T* pInfo;
	int found=-1, i;
	//id starts from 0,1,2...
	if( (id<0) || (id>=gWLANTotalClients) || (info==NULL) ) return -1;
	
	for (i=1; i<=MAX_STA_NUM; i++)
	{
		pInfo = (WLAN_STA_INFO_T*)&gWLANAssociations[i*sizeof(WLAN_STA_INFO_T)];
		if (pInfo->aid && (pInfo->flag & STA_INFO_FLAG_ASOC))
		{
			found++;
			if(found==id) break;
		}
	}
	if( i>MAX_STA_NUM ) return -1;
	
	memcpy( info, pInfo, sizeof(WLAN_STA_INFO_T) );
	return 0;
}

/*copy from mib.c, because it defines with "static" */
static int _is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}

static int string_to_hex(char *string, unsigned char *key, int len)
{
	char tmpBuf[4];
	int idx, ii=0;
	for (idx=0; idx<len; idx+=2) {
		tmpBuf[0] = string[idx];
		tmpBuf[1] = string[idx+1];
		tmpBuf[2] = 0;
		if ( !_is_hex(tmpBuf[0]) || !_is_hex(tmpBuf[1]))
			return 0;

		key[ii++] = (unsigned char) strtol(tmpBuf, (char**)NULL, 16);
	}
	return 1;
}

int getPSKChainId( int index )
{
	CWMP_PSK_T	*pEntry=NULL,psk_entity;
	int total=0, i=0;
	
	total = mib_chain_total( CWMP_PSK_TBL );
	if( total<=0 ) return -1;
	
	for( i=0; i<total; i++)
	{
		pEntry = &psk_entity;
		if( mib_chain_get(CWMP_PSK_TBL, i, (void*)pEntry ) )
			if( pEntry->index==index ) break;
	}
	if( i==total ) return -1;
	
	return i;
}

int getRateStr( unsigned short rate, char *buf )
{
	int len;
	if( buf==NULL ) return -1;

	buf[0]=0;
	if( rate & TX_RATE_1M )
		strcat( buf, "1," );
	if( rate & TX_RATE_2M )
		strcat( buf, "2," );
	if( rate & TX_RATE_5M )
		strcat( buf, "5.5," );
	if( rate & TX_RATE_11M )
		strcat( buf, "11," );
	if( rate & TX_RATE_6M )
		strcat( buf, "6," );
	if( rate & TX_RATE_9M )
		strcat( buf, "9," );
	if( rate & TX_RATE_12M )
		strcat( buf, "12," );
	if( rate & TX_RATE_18M )
		strcat( buf, "18," );
	if( rate & TX_RATE_24M )
		strcat( buf, "24," );
	if( rate & TX_RATE_36M )
		strcat( buf, "36," );
	if( rate & TX_RATE_48M )
		strcat( buf, "48," );
	if( rate & TX_RATE_54M )
		strcat( buf, "54," );
		
	len = strlen(buf);
	if( len>1 )
		buf[len-1]=0;
	return 0;
}

int setRateStr( char *buf, unsigned short *rate )
{
	if( (rate!=NULL) && (buf!=NULL) )
	{
		char *tok;
		
		*rate=0;
		tok = strtok(buf,", \n\r");
		while(tok)
		{
			if( strcmp( tok, "1" )==0 )
				*rate = *rate | TX_RATE_1M;
			else if( strcmp( tok, "2" )==0 )
				*rate = *rate | TX_RATE_2M;
			else if( strcmp( tok, "5.5" )==0 )
				*rate = *rate | TX_RATE_5M;
			else if( strcmp( tok, "11" )==0 )
				*rate = *rate | TX_RATE_11M;
			else if( strcmp( tok, "6" )==0 )
				*rate = *rate | TX_RATE_6M;
			else if( strcmp( tok, "9" )==0 )
				*rate = *rate | TX_RATE_9M;
			else if( strcmp( tok, "12" )==0 )
				*rate = *rate | TX_RATE_12M;
			else if( strcmp( tok, "18" )==0 )
				*rate = *rate | TX_RATE_18M;
			else if( strcmp( tok, "24" )==0 )
				*rate = *rate | TX_RATE_24M;
			else if( strcmp( tok, "36" )==0 )
				*rate = *rate | TX_RATE_36M;
			else if( strcmp( tok, "48" )==0 )
				*rate = *rate | TX_RATE_48M;
			else if( strcmp( tok, "54" )==0 )
				*rate = *rate | TX_RATE_54M;
			else{
				*rate=0;
				return -1;
			}
							
			tok = strtok(NULL,", \n\r");
		}		
		return 0;
	}
	
	return -1;
}


int getIPbyMAC( char *mac, char *ip )
{
	int	ret=-1;
	FILE 	*fh;
	char 	buf[128];

	if( (mac==NULL) || (ip==NULL) )	return ret;
	ip[0]=0;
	
	fh = fopen("/proc/net/arp", "r");
	if (!fh) return ret;
	
	fgets(buf, sizeof buf, fh);	/* eat line */
	//fprintf( stderr, "%s\n", buf );
	while (fgets(buf, sizeof buf, fh))
	{
		char cip[32],cmac[32];
		
		//fprintf( stderr, "%s\n", buf );
		//format: IP address       HW type     Flags       HW address            Mask     Device
		if( sscanf(buf,"%s %*s %*s %s %*s %*s", cip,cmac)!=2 )
			continue;
			
		//fprintf( stderr, "mac:%s, cmac:%s, cip:%s\n", mac, cmac, cip );
		if( strcasecmp( mac, cmac )==0 )
		{
			strcpy( ip, cip );
			ret=0;
			break;
		}
	}
	fclose(fh);
	return ret;
}
#endif /*#ifdef WLAN_SUPPORT*/
