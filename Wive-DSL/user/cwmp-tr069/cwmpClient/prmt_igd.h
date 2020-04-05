#ifndef _PRMT_IGD_H_
#define _PRMT_IGD_H_

#include <linux/config.h>
#include <config/autoconf.h>

#include "libcwmp.h"
#include "cwmp_porting.h"

#include <rtk/options.h>
#include <rtk/sysconfig.h>
#include "adsl_drv.h"
#include "utility.h"
#include "prmt_apply.h"
#ifdef CONFIG_MIDDLEWARE
#include <rtk/midwaredefs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LEAF tLANConfigSecurityLeaf[];
extern struct CWMP_LEAF tIGDLeaf[];
extern struct CWMP_NODE tIGDObject[];
extern struct CWMP_NODE tROOT[];
#ifdef CONFIG_MIDDLEWARE
extern struct CWMP_NODE mw_tROOT[];
#endif

int getIGD(char *name, struct CWMP_LEAF *entity, int *type, void **data);

int getLANConfSec(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setLANConfSec(char *name, struct CWMP_LEAF *entity, int type, void *data);

#ifdef _PRMT_X_CT_COM_DATATYPE
int changestring2int(void* dataaddr, int righttype, int wrongtype, int* tmpint, unsigned int* tmpuint, int* tmpbool);
#endif

#ifdef __cplusplus
}
#endif

#endif /*_PRMT_IGD_H_*/
