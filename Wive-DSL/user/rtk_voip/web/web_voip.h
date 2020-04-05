#ifndef __WEB_VOIP_H
#define __WEB_VOIP_H

#include "voip_flash.h"
#include "voip_types.h"
#include "voip_flash_mib.h"
#include "voip_flash_tool.h"

#if CONFIG_RTK_VOIP_PACKAGE_865X
	#include "asp_page.h"
	#include "boa.h"
#else
	#include "webs.h"
#endif

#ifdef __mips__
#if CONFIG_RTK_VOIP_PACKAGE_865X
#define VOIP_CONFIG_PATH	"/www/config_voip.dat"
#elif CONFIG_RTK_VOIP_PACKAGE_867X
#define VOIP_CONFIG_PATH	"/var/config_voip.dat"
#else
#define VOIP_CONFIG_PATH	"/web/config_voip.dat"
#endif
#else
#define VOIP_CONFIG_PATH	"../web/config_voip.dat"
#endif

#if CONFIG_RTK_VOIP_PACKAGE_865X
#define gstrcmp strcmp
#endif

#if CONFIG_RTK_VOIP_PACKAGE_865X
typedef char char_t;
typedef struct request *webs_t;
#endif

/* To show the register status on Web page. */
#define _PATH_TMP_STATUS	"/tmp/status"

// init web functions
int web_voip_init();
int web_restart_solar();

// config function
int web_voip_saveConfig();

#if CONFIG_RTK_VOIP_PACKAGE_865X
int asp_voip_getInfo(webs_t wp, int argc, char_t **argv);
int asp_voip_GeneralGet(webs_t wp, int argc, char_t **argv);
int asp_voip_DialPlanGet( webs_t wp, int argc, char_t **argv);
int asp_voip_ToneGet(webs_t wp, int argc, char_t **argv);
int asp_voip_RingGet(webs_t wp, int argc, char_t **argv);
int asp_voip_OtherGet(webs_t wp, int argc, char_t **argv);
int asp_voip_ConfigGet(webs_t wp, int argc, char_t **argv);
int asp_voip_FxoGet(webs_t wp, int argc, char_t **argv);
#else
int asp_voip_getInfo(int eid, webs_t wp, int argc, char_t **argv);
int asp_voip_GeneralGet(int ejid, webs_t wp, int argc, char_t **argv);
int asp_voip_DialPlanGet(int ejid, webs_t wp, int argc, char_t **argv);
int asp_voip_ToneGet(int ejid, webs_t wp, int argc, char_t **argv);
int asp_voip_RingGet(int ejid, webs_t wp, int argc, char_t **argv);
int asp_voip_OtherGet(int ejid, webs_t wp, int argc, char_t **argv);
int asp_voip_ConfigGet(int ejid, webs_t wp, int argc, char_t **argv);
int asp_voip_FxoGet(int ejid, webs_t wp, int argc, char_t **argv);
#endif

void asp_voip_GeneralSet(webs_t wp, char_t *path, char_t *query);
void asp_voip_DialPlanSet(webs_t wp, char_t *path, char_t *query);
void asp_voip_ToneSet(webs_t wp, char_t *path, char_t *query);
void asp_voip_RingSet(webs_t wp, char_t *path, char_t *query);
void asp_voip_OtherSet(webs_t wp, char_t *path, char_t *query);
void asp_voip_ConfigSet(webs_t wp, char_t *path, char_t *query);
void asp_voip_ConfigSave(webs_t wp, char_t *path, char_t *query);
void asp_voip_ConfigLoad(webs_t wp, char_t *path, char_t *query);
void asp_voip_IvrReqSet(webs_t wp, char_t *path, char_t *query);
void asp_voip_FxoSet(webs_t wp, char_t *path, char_t *query);

#endif
