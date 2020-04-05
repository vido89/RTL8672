#ifndef _PRMT_CTCOM_H_
#define _PRMT_CTCOM_H_

#include "prmt_igd.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _PRMT_X_CT_COM_ACCOUNT_
extern struct CWMP_LEAF tCTAccountLeaf[];
int getCTAccount(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCTAccount(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif /*_PRMT_X_CT_COM_ACCOUNT_*/

#ifdef _PRMT_X_CT_COM_ALG_
extern struct CWMP_LEAF tXCTCOMALGLeaf[];
int getXCTCOMALG(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setXCTCOMALG(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif //_PRMT_X_CT_COM_ALG_

#ifdef _PRMT_X_CT_COM_RECON_
extern struct CWMP_LEAF tCT_ReConLeaf[];
int getCT_ReCon(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_ReCon(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif //_PRMT_X_CT_COM_RECON_

#ifdef _PRMT_X_CT_COM_PORTALMNT_
extern struct CWMP_LEAF tCT_PortalMNTLeaf[];
int getCT_PortalMNT(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_PortalMNT(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif //_PRMT_X_CT_COM_PORTALMNT_

#ifdef _PRMT_X_CT_COM_SRVMNG_
extern struct CWMP_LEAF tCTServiceLeaf[];
int getCTService(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCTService(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif /*_PRMT_X_CT_COM_SRVMNG_*/


#ifdef _PRMT_X_CT_COM_SYSLOG_
extern struct CWMP_LEAF tCT_SyslogLeaf[];
int getCT_Syslog(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_Syslog(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif //_PRMT_X_CT_COM_SYSLOG_

#ifdef CONFIG_MIDDLEWARE
extern struct CWMP_LEAF tCT_MiddlewareMgtLeaf[];
int getCT_MiddlewareMgt(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_MiddlewareMgt(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif //CONFIG_MIDDLEWARE
	
#ifdef _PRMT_X_CT_COM_IPTV_
extern struct CWMP_LEAF tCT_IPTVLeaf[];
int getCT_IPTV(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_IPTV(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif

#ifdef _PRMT_X_CT_COM_MONITOR_
extern struct CWMP_LEAF tCT_MONITORLeaf[];
int getCT_MONITOR(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_MONITOR(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif

#ifdef _PRMT_X_CT_COM_VPDN_
extern struct CWMP_LEAF tCT_VPDNLeaf[];
int getCT_VPDN(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_VPDN(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif

#ifdef _PRMT_X_CT_COM_MWBAND_
extern struct CWMP_LEAF tCT_MWBANDLeaf[];
int getCT_MWBAND(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setCT_MWBAND(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif

#ifdef __cplusplus
}
#endif
#endif /*_PRMT_TIME_H_*/
