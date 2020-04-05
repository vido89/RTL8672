#include "prmt_services.h"
#ifdef CONFIG_USER_RTK_VOIP
#include "prmt_voiceservice.h"
#endif
#ifdef _PRMT_X_CT_EXT_ENABLE_
#include "prmt_ctcom.h"
#endif //_PRMT_X_CT_EXT_ENABLE_
#ifdef _PRMT_SERVICES_


#ifdef CONFIG_USER_RTK_VOIP
struct CWMP_OP tSRV_VoiceService_OP = { NULL, objVoiceService };
#endif

/*********tServicesObject*************************************************************************************************/
struct CWMP_PRMT tServicesObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
#ifdef _PRMT_X_CT_COM_IPTV_
{"X_CT-COM_IPTV",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
#ifdef _PRMT_X_CT_COM_MWBAND_
{"X_CT-COM_MWBAND",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
#ifdef _PRMT_X_CT_COM_MONITOR_
{"X_CT-COM_Monitor",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
#ifdef _PRMT_X_CT_COM_VPDN_
{"X_CT-COM_VPDN",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
#ifdef CONFIG_USER_RTK_VOIP
{"VoiceService",		eCWMP_tOBJECT,	CWMP_READ,	&tSRV_VoiceService_OP},
#endif
};
enum eServicesObject
{
#ifdef _PRMT_X_CT_COM_IPTV_
	eSX_CTCOM_IPTV,
#endif
#ifdef _PRMT_X_CT_COM_MWBAND_
	eSX_CTCOM_MWBAND,
#endif
#ifdef _PRMT_X_CT_COM_MONITOR_
	eSX_CTCOM_MONITOR,
#endif
#ifdef _PRMT_X_CT_COM_VPDN_
	eSX_CTCOM_VPDN,
#endif
#ifdef CONFIG_USER_RTK_VOIP
	eSX_VoiceService,
#endif
	eSX_END  /*the last one*/
};

struct CWMP_NODE tServicesObject[] =
{
/*info,  				leaf,			next)*/
#ifdef _PRMT_X_CT_COM_IPTV_
{&tServicesObjectInfo[eSX_CTCOM_IPTV],	tCT_IPTVLeaf,		NULL},
#endif
#ifdef _PRMT_X_CT_COM_MWBAND_
{&tServicesObjectInfo[eSX_CTCOM_MWBAND],tCT_MWBANDLeaf,		NULL},
#endif
#ifdef _PRMT_X_CT_COM_MONITOR_
{&tServicesObjectInfo[eSX_CTCOM_MONITOR],tCT_MONITORLeaf,	NULL},
#endif
#ifdef _PRMT_X_CT_COM_VPDN_
{&tServicesObjectInfo[eSX_CTCOM_VPDN],	tCT_VPDNLeaf,		NULL},
#endif
#ifdef CONFIG_USER_RTK_VOIP
{&tServicesObjectInfo[eSX_VoiceService],	NULL,		NULL},
#endif
{NULL,					NULL,			NULL}	
};
/*********end tServicesObject*************************************************************************************************/










#endif /*_PRMT_SERVICES_*/
