#ifndef PORT_H_
#define PORT_H_

#define uint32 unsigned int
#define char_t char
//typedef request* webs_t;
#define B_ARGS_DEC		char_t *file, int line
#define webs_t request *

extern int getInfo(int eid, request* wp, int argc, char_t  **argv);
extern int getIndex(int eid, request* wp, int argc, char_t **argv);
extern int portFwList(int eid, request* wp, int argc, char_t **argv);
extern int ipPortFilterList(int eid, request* wp, int argc, char_t **argv);
extern int macFilterList(int eid, request* wp, int argc, char_t **argv);
extern int atmVcList(int eid, request* wp, int argc, char_t **argv);
extern int atmVcList2(int eid, request* wp, int argc, char_t **argv);
extern int wanConfList(int eid, request* wp, int argc, char_t **argv);

/* Routines exported in fmtcpip.c */
extern void formTcpipLanSetup(webs_t wp, char_t *path, char_t *query);

/* Routines exported in fmfwall.c */
extern void formPortFw(webs_t wp, char_t *path, char_t *query);
extern void formFilter(webs_t wp, char_t *path, char_t *query);
extern void formDMZ(webs_t wp, char_t *path, char_t *query);
extern int portFwList(int eid, webs_t wp, int argc, char_t **argv);
extern int ipPortFilterList(int eid, webs_t wp, int argc, char_t **argv);
extern int macFilterList(int eid, webs_t wp, int argc, char_t **argv);

/* Routines exported in fmmgmt.c */
extern void formPasswordSetup(webs_t wp, char_t *path, char_t *query);
extern void formUpload(webs_t wp, char_t * path, char_t * query);
extern void formSaveConfig(webs_t wp, char_t *path, char_t *query);
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
extern void formSnmpConfig(webs_t wp, char_t *path, char_t *query);
#endif
extern void formAdslDrv(webs_t wp, char_t *path, char_t *query);
extern void formStats(webs_t wp, char_t *path, char_t *query);
extern void formRconfig(webs_t wp, char_t *path, char_t *query);
extern int adslDrvSnrTblGraph(int eid, webs_t wp, int argc, char_t **argv);
extern int adslDrvSnrTblList(int eid, webs_t wp, int argc, char_t **argv);
extern int adslDrvBitloadTblGraph(int eid, webs_t wp, int argc, char_t **argv);
extern int adslDrvBitloadTblList(int eid, webs_t wp, int argc, char_t **argv);
extern int memDump(int eid, webs_t wp, int argc, char_t **argv);
extern int pktStatsList(int eid, webs_t wp, int argc, char_t **argv);

#endif

