/*
 *      Web server handler routines for management (password, save config, f/w update)
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *      Authors: Dick Tam	<dicktam@realtek.com.tw>
 *
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sysinfo.h>
#include <signal.h>
#include <net/if.h>
#include <stdint.h>
#include <linux/atm.h>
#include <linux/atmdev.h>

#include "../webs.h"
#include "../um.h"
#include "mib.h"
#include "webform.h"
#include "adsl_drv.h"
#include "utility.h"
#include "rtl_flashdrv.h"
#include <semaphore.h>

//xl_yue
#ifdef USE_LOGINWEB_OF_SERVER
#include <syslog.h>
#include "boa.h"
#endif

//ql_xu add
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>

// Mason Yu
#ifdef EMBED
#include <linux/config.h>
#include <linux/sockios.h>	//cathy
#else
#include "../../../../uClibc/include/linux/autoconf.h"
#endif


//xl_yue
#ifdef ACCOUNT_LOGIN_CONTROL
#include <syslog.h>
#include "boa.h"
#endif
#include "../defs.h"

#define DEFAULT_GROUP		T("administrators")
#define ACCESS_URL			T("/")
#define _PATH_PROCNET_DEV	"/proc/net/dev"
#define MAX_DSL_TONE		512
///ql_xu add
#define _PPPOE_CONF			"/var/ppp/pppoe.conf"
#define _PPPOA_CONF			"/var/ppp/pppoa.conf"
#define _PPP_CONF			"/var/ppp/ppp.conf"
#define _PROC_NET_ATM_CLIP	"/proc/net/atm/pvc"
#define _PROC_NET_ATM_BR	"/proc/net/atm/br2684"

static int srandomCalled = 0;
char g_rUserName[MAX_NAME_LEN];
char g_rUserPass[MAX_NAME_LEN];
bool_t g_remoteUpdate;

short *snr;
short *qln;
short *hlog;
static int diagflag=1;
#define XML_CONFIG

// Added by Mason Yu
extern char *g_post_file_name;
extern int g_startPos;
extern sem_t semSave;
extern int g_filesize;
#ifdef ENABLE_SIGNATURE_ADV
extern int upgrade;
#endif
extern int g_upgrade_firmware;
#ifdef USE_LOGINWEB_OF_SERVER
// Mason Yu on True
unsigned char g_login_username[MAX_NAME_LEN];
#endif

#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
#define REBOOT_MSG(url) { \
	websHeader(wp); \
	websWrite(wp, T("<body><blockquote><h5>如果您改变了配置，要使配置生效请重新启动系统!" \
                "<BR><form><input type=button value=\"  确定  \" OnClick=window.location.replace(\"%s\")></form></blockquote></body>"), url);\
	websFooter(wp); \
	websDone(wp, 200); \
}
#endif

static void write_etcPassword()
{
	FILE *fp;
	char userName[MAX_NAME_LEN], userPass[MAX_NAME_LEN];
	char *xpass;
#ifdef ACCOUNT_CONFIG
	MIB_CE_ACCOUNT_CONFIG_T entry;
	unsigned int totalEntry;
#endif
	int i;

	fp = fopen("/var/passwd", "w+");
#ifdef ACCOUNT_CONFIG
	totalEntry = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL); /* get chain record size */
	for (i = 0; i < totalEntry; i++) {
		if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, i, (void *)&entry)) {
			printf("ERROR: Get account configuration information from MIB database failed.\n");
			return;
		}
		strcpy(userName, entry.userName);
		strcpy(userPass, entry.userPassword);
		xpass = crypt(userPass, "$1$");
		if (xpass) {
			//#ifdef CONFIG_USER_MENU_CLI
			#ifdef CONFIG_USER_CLI
			if (entry.privilege == (unsigned char)PRIV_ROOT)
				fprintf(fp, "%s:%s:0:0::/tmp:/bin/cli\n", userName, xpass);
			else
				fprintf(fp, "%s:%s:1:0::/tmp:/bin/cli\n", userName, xpass);
			#else
			if (entry.privilege == (unsigned char)PRIV_ROOT)
				fprintf(fp, "%s:%s:0:0::/tmp:/bin/sh\n", userName, xpass);
			else
				fprintf(fp, "%s:%s:1:0::/tmp:/bin/sh\n", userName, xpass);
			#endif
		}
	}
#endif
	mib_get( MIB_SUSER_NAME, (void *)userName );
	mib_get( MIB_SUSER_PASSWORD, (void *)userPass );
	xpass = crypt(userPass, "$1$");
	if (xpass)
//#ifdef CONFIG_USER_MENU_CLI
#ifdef CONFIG_USER_CLI
		fprintf(fp, "%s:%s:0:0::/tmp:/bin/cli\n", userName, xpass);
#else
		fprintf(fp, "%s:%s:0:0::/tmp:/bin/sh\n", userName, xpass);
#endif
		
	// Added by Mason Yu for others user	
//	mib_get( MIB_SUPER_NAME, (void *)userName );
//	mib_get( MIB_SUPER_PASSWORD, (void *)userPass );
//	xpass = crypt(userPass, "$1$");
//	if (xpass)
//#ifdef CONFIG_USER_MENU_CLI
//#ifdef CONFIG_USER_CLI
//		fprintf(fp, "%s:%s:0:0::/tmp:/bin/cli\n", userName, xpass);
//#else
//		fprintf(fp, "%s:%s:0:0::/tmp:/bin/sh\n", userName, xpass);
//#endif

			
//	mib_get( MIB_USER_NAME, (void *)userName );
//	if (userName[0]) {
//		mib_get( MIB_USER_PASSWORD, (void *)userPass );				
//		xpass = crypt(userPass, "$1$");
//		if (xpass)
//#ifdef CONFIG_USER_MENU_CLI
//#ifdef CONFIG_USER_CLI
//			fprintf(fp, "%s:%s:1:0::/tmp:/bin/cli\n", userName, xpass);
//#else
//			fprintf(fp, "%s:%s:1:0::/tmp:/bin/sh\n", userName, xpass);
//#endif
//	}	
	fclose(fp);
	chmod("/var/tmp", 0x1fd);	// let owner and group have write access
}

#ifdef ACCOUNT_CONFIG
extern char suName[MAX_NAME_LEN];
extern char usName[MAX_NAME_LEN];
// Jenny, user account configuration
/////////////////////////////////////////////////////////////////////////////
int accountList(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;

	unsigned int entryNum, i;
	MIB_CE_ACCOUNT_CONFIG_T Entry;
	char	*priv;
	char upasswd[MAX_NAME_LEN];

	nBytesSent += websWrite(wp, T("<tr><font size=2>"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=2>Select</td>\n"
	"<td align=center width=\"50%%\" bgcolor=\"#808080\"><font size=2>User Name</td>\n"
	"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=2>Privilege</td></font></tr>\n"));

	/*if (!mib_get(MIB_SUSER_PASSWORD, (void *)upasswd)) {
		printf("ERROR: Get superuser password from MIB database failed.\n");
		return;
	}*/
	nBytesSent += websWrite(wp, T("<tr>"
	"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"select\""
//	" value=\"s0\" onClick=\"postEntry('%s', %d, '%s')\"></td>\n"),
//	suName, PRIV_ROOT, upasswd);
	" value=\"s0\" onClick=\"postEntry('%s', %d)\"></td>\n"),
	suName, PRIV_ROOT);
	nBytesSent += websWrite(wp, T(
	"<td align=center width=\"50%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
	"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>Admin</b></font></td></tr>\n"),
	suName);

	/*if (!mib_get(MIB_USER_PASSWORD, (void *)upasswd)) {
		printf("ERROR: Get user password from MIB database failed.\n");
		return;
	}*/
	nBytesSent += websWrite(wp, T("<tr>"
	"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"select\""
//	" value=\"s1\" onClick=\"postEntry('%s', %d, '%s')\"></td>\n"),
//	usName, PRIV_USER, upasswd);
	" value=\"s1\" onClick=\"postEntry('%s', %d)\"></td>\n"),
	usName, PRIV_USER);
	nBytesSent += websWrite(wp, T(
	"<td align=center width=\"50%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
	"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>User</b></font></td></tr>\n"),
	usName);

	entryNum = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL);
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, i, (void *)&Entry)) {
  			websError(wp, 400, T(strGetChainerror));
			return -1;
		}

		priv = 0;
		if (Entry.privilege == PRIV_ROOT)
			priv = "Admin";
		else if (Entry.privilege == PRIV_ENG)
			priv = "Support";
		else if (Entry.privilege == PRIV_USER)
			priv = "User";

		nBytesSent += websWrite(wp, T("<tr>"
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"select\""
//		" value=\"s%d\" onClick=\"postEntry('%s', %d, '%s')\"></td>\n"),
//		i+2, Entry.userName, Entry.privilege, Entry.userPassword);
		" value=\"s%d\" onClick=\"postEntry('%s', %d)\"></td>\n"),
		i+2, Entry.userName, Entry.privilege);
		nBytesSent += websWrite(wp, T(
		"<td align=center width=\"50%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td></tr>\n"),
		Entry.userName, priv);
	}
	
	return nBytesSent;
}

void formAccountConfig(webs_t wp, char_t *path, char_t *query)
{
	char_t *str, *strUser, *submitUrl, *strOldPassword, *strPassword, *strConfPassword, *strPriv;
	MIB_CE_ACCOUNT_CONFIG_T entry, Entry;
	char tmpBuf[100];
	strUser = websGetVar(wp, T("username"), T(""));
	strPriv = websGetVar(wp, T("privilege"), T(""));
	strOldPassword = websGetVar(wp, T("oldpass"), T(""));
	strPassword = websGetVar(wp, T("newpass"), T(""));
	strConfPassword = websGetVar(wp, T("confpass"), T(""));
	/* Retrieve next page URL */
	submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page
	int totalEntry, i, selected;

	// Delete
	str = websGetVar(wp, T("deluser"), T(""));
	if (str[0]) {
		unsigned int i;
		unsigned int idx;
		totalEntry = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL); /* get chain record size */
		str = websGetVar(wp, T("select"), T(""));
		
		if (str[0]) {
			if (!strncmp(str, "s0", 2) || !strncmp(str, "s1", 2)) {
				strcpy(tmpBuf, T("Sorry, the account cannot be deleted!"));
				goto setErr_user;
			}
			for (i=0; i<totalEntry; i++) {
				idx = totalEntry - i + 1;
				snprintf(tmpBuf, 4, "s%d", idx);
				if (!gstrcmp(str, T(tmpBuf))) {
					/* get the specified chain record */
					if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, idx - 2, (void *)&Entry)) {
						strcpy(tmpBuf, errGetEntry);
						goto setErr_user;
					}
					// delete from chain record
					if(mib_chain_delete(MIB_ACCOUNT_CONFIG_TBL, idx - 2) != 1) {
						strcpy(tmpBuf, T("Delete chain record error!"));
						goto setErr_user;
					}
					goto setOk_user;
				}
			}
		}
		else {
			strcpy(tmpBuf, T("There is no item selected to delete!"));
			goto setErr_user;
		}
	}

	if (!strUser[0]) {
		strcpy(tmpBuf, T(strUserNameempty));
		goto setErr_user;
	}
	else {
		strncpy(entry.userName, strUser, MAX_NAME_LEN-1);
		entry.userName[MAX_NAME_LEN-1] = '\0';
		//entry.userName[MAX_NAME_LEN] = '\0';
	}

	if (!strPassword[0]) {
		strcpy(tmpBuf, T(WARNING_EMPTY_NEW_PASSWORD));
		goto setErr_user;
	}
	else {
		strncpy(entry.userPassword, strPassword, MAX_NAME_LEN-1);
		entry.userPassword[MAX_NAME_LEN-1] = '\0';
		//entry.userPassword[MAX_NAME_LEN] = '\0';
	}

	if (!strConfPassword[0]) {
		strcpy(tmpBuf, T(WARNING_EMPTY_CONFIRMED_PASSWORD));
		goto setErr_user;
	}

	if (strcmp(strPassword, strConfPassword) != 0 ) {
		strcpy(tmpBuf, T(WARNING_UNMATCHED_PASSWORD));
		goto setErr_user;
	}

	if (strPriv[0])
		entry.privilege = strPriv[0] - '0';

	totalEntry = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL); /* get chain record size */
	// Add
	str = websGetVar(wp, T("adduser"), T(""));		
	if (str[0]) {
		int intVal;
		/* Check if user name exists */
		if (strcmp(suName, strUser) == 0 || strcmp(usName, strUser) == 0) {
			strcpy(tmpBuf, T("ERROR: user already exists!"));
			goto setErr_user;
		}
		for (i=0; i<totalEntry; i++) {
			if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, i, (void *)&Entry)) {
  				websError(wp, 400, T(strGetChainerror));
				return;
			}

			if (strcmp(Entry.userName, strUser) == 0) {
				strcpy(tmpBuf, T("ERROR: user already exists!"));
				goto setErr_user;
			}
		}

		intVal = mib_chain_add(MIB_ACCOUNT_CONFIG_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			strcpy(tmpBuf, T(strAddChainerror));
			goto setErr_user;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, T(strTableFull));
			goto setErr_user;
		}
	}

	// Modify
	str = websGetVar(wp, T("modify"), T(""));
	if (str[0]) {
		selected = -1;
		str = websGetVar(wp, T("select"), T(""));
		if (str[0]) {
			for (i=0; i<totalEntry+2; i++) {
				snprintf(tmpBuf, 4, "s%d", i);
				if (!gstrcmp(str, T(tmpBuf))) {
					selected = i;
					break;
				}
			}
			if (selected >= 2) {
				if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, selected - 2, (void *)&Entry)) {
					strcpy(tmpBuf, T(strGetChainerror));
					goto setErr_user;
				}
				if (strcmp(Entry.userPassword, strOldPassword) != 0) {
					strcpy(tmpBuf, T(WARNING_WRONG_PASSWORD));
					goto setErr_user;
				}
				mib_chain_update(MIB_ACCOUNT_CONFIG_TBL, (void *)&entry, selected - 2);
			}
			else if (selected == 0) {
				if (!mib_get(MIB_SUSER_PASSWORD, (void *)tmpBuf)) {
					strcpy(tmpBuf, T(WARNING_GET_PASSWORD));
					goto setErr_user;
				}
				if (strcmp(tmpBuf, strOldPassword) != 0) {
					strcpy(tmpBuf, T(WARNING_WRONG_PASSWORD));
					goto setErr_user;
				} else if (!mib_set(MIB_SUSER_PASSWORD, (void *)strPassword)) {
					strcpy(tmpBuf, T(WARNING_SET_PASSWORD));
					goto setErr_user;
				}
				if (!mib_set(MIB_SUSER_NAME, (void *)strUser)) {
					strcpy(tmpBuf, T("ERROR: Set Super user name to MIB database failed."));
					goto setErr_user;
				}
				mib_get(MIB_SUSER_NAME, (void *)suName);
			}
			else if (selected == 1) {
				if (!mib_get(MIB_USER_PASSWORD, (void *)tmpBuf)) {
					strcpy(tmpBuf, T(WARNING_GET_PASSWORD));
					goto setErr_user;
				}
				if (strcmp(tmpBuf, strOldPassword) != 0) {
					strcpy(tmpBuf, T(WARNING_WRONG_PASSWORD));
					goto setErr_user;
				} else if (!mib_set(MIB_USER_PASSWORD, (void *)strPassword)) {
					strcpy(tmpBuf, T(WARNING_SET_PASSWORD));
					goto setErr_user;
				}
				if (!mib_set(MIB_USER_NAME, (void *)strUser)) {
					strcpy(tmpBuf, T("ERROR: Set user name to MIB database failed."));
					goto setErr_user;
				}
				mib_get(MIB_USER_NAME, (void *)usName);
			}
		}
	}

setOk_user:
#ifdef EMBED
	// for take effect on real time
	writePasswdFile();
	write_etcPassword();	// Jenny
#endif


	OK_MSG(submitUrl);
	return;

setErr_user:
	OK_MSG1(tmpBuf, submitUrl);
}
#endif

/////////////////////////////////////////////////////////////////////////////
void formPasswordSetup(webs_t wp, char_t *path, char_t *query)
{
	char_t *str, *submitUrl, *strPassword, *strOldPassword, *strConfPassword;
	char tmpBuf[100];
	char userName[MAX_NAME_LEN];
	
	str = websGetVar(wp, T("userMode"), T(""));		
	//strUser = websGetVar(wp, T("username"), T(""));
	strOldPassword = websGetVar(wp, T("oldpass"), T(""));
	strPassword = websGetVar(wp, T("newpass"), T(""));
	strConfPassword = websGetVar(wp, T("confpass"), T(""));
	
	if ( !strOldPassword[0] ) {
		strcpy(tmpBuf, T(WARNING_EMPTY_OLD_PASSWORD));
		goto setErr_pass;
	}
	
	if ( !strPassword[0] ) {
		strcpy(tmpBuf, T(WARNING_EMPTY_NEW_PASSWORD));
		goto setErr_pass;
	}	

	if ( !strConfPassword[0] ) {
		strcpy(tmpBuf, T(WARNING_EMPTY_CONFIRMED_PASSWORD));
		goto setErr_pass;
	}

	if (strcmp(strPassword, strConfPassword) != 0 ) {
		strcpy(tmpBuf, T(WARNING_UNMATCHED_PASSWORD));
		goto setErr_pass;
	}
		
#if 0
	if ( strUser[0] ) {
		/* Check if user name is the same as supervisor name */
		if ( !mib_get(MIB_SUPER_NAME, (void *)tmpBuf)) {
			strcpy(tmpBuf, T("ERROR: Get supervisor name MIB error!"));
			goto setErr_pass;
		}
		if ( !strcmp(strUser, tmpBuf)) {
			strcpy(tmpBuf, T("ERROR: Cannot use the same user name as supervisor."));
			goto setErr_pass;
		}
	}
#endif	
	// Commented by Mason Yu
#if 0
	// delete original user
	mib_get( MIB_USER_NAME, (void *)userName );
	if (userName[0]) {
		if ( umDeleteUser(userName) ) {
			printf("ERROR: Unable to delete user account (user=%s).\n", userName);
			return;
		}
		mib_get( MIB_SUPER_NAME, (void *)userName );
		if ( umDeleteUser(userName) ) {
			printf("ERROR: Unable to delete user account (user=%s).\n", userName);
			return;
		}
	}
#endif
	
	if (str[0]) {
		if ( str[0] == '0' ) {       // superuser ==> cht
			if ( !mib_get(MIB_SUSER_PASSWORD, (void *)tmpBuf)) {
				strcpy(tmpBuf, T(WARNING_GET_PASSWORD));
				goto setErr_pass;
			}
			
			if ( strcmp(tmpBuf, strOldPassword) != 0 ) {
				strcpy(tmpBuf, T(WARNING_WRONG_PASSWORD));
				goto setErr_pass;
			}else if ( !mib_set(MIB_SUSER_PASSWORD, (void *)strPassword) ) {							
				strcpy(tmpBuf, T(WARNING_SET_PASSWORD));
				goto setErr_pass;
			}
		}	
		else if ( str[0] == '1' ) {  // normal user ==> user
			if ( !mib_get(MIB_USER_PASSWORD, (void *)tmpBuf)) {
				strcpy(tmpBuf, T(WARNING_GET_PASSWORD));
				goto setErr_pass;
			}
			
			if ( strcmp(tmpBuf, strOldPassword) != 0 ) {
				strcpy(tmpBuf, T(WARNING_WRONG_PASSWORD));
				goto setErr_pass;
			}else if ( !mib_set(MIB_USER_PASSWORD, (void *)strPassword) ) {							
				strcpy(tmpBuf, T(WARNING_SET_PASSWORD));
				goto setErr_pass;
			}	
		}		
		else {			
			strcpy(tmpBuf, T(WARNING_WRONG_USER));			
			goto setErr_pass;
		}
	}

#if 0	
	/* Set user account to MIB */
	if ( !mib_set(MIB_USER_NAME, (void *)strUser) ) {
		strcpy(tmpBuf, T("ERROR: Set user name to MIB database failed."));
		goto setErr_pass;
	}
#endif

#ifdef EMBED
	// Added by Mason Yu for take effect on real time
	writePasswdFile();
	write_etcPassword();	// Jenny
#endif
	
	/*
	if (mib_update(HW_SETTING) == 0) {
		printf("Warning : Commit hs fail(formPasswordSetup()) !\n");				
	}
	*/

	/* upgdate to flash */
//	mib_update(CURRENT_SETTING);

	/* Init user management */
	// Commented By Mason Yu
	//set_user_profile();
	
	/* Retrieve next page URL */
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
	#endif
	submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page

	OK_MSG(submitUrl);
	return;

setErr_pass:
	ERR_MSG(tmpBuf);
}

// Added by Mason Yu for 2 level web page
/////////////////////////////////////////////////////////////////////////////
void formUserPasswordSetup(webs_t wp, char_t *path, char_t *query)
{
	char_t *str, *submitUrl, *strPassword, *strOldPassword, *strConfPassword;
	char tmpBuf[100];
	char userName[MAX_NAME_LEN];
#ifdef ACCOUNT_CONFIG
	MIB_CE_ACCOUNT_CONFIG_T Entry;
	int totalEntry, i, selected = -1;
#endif
	
	//str = websGetVar(wp, T("userMode"), T(""));		
	//strUser = websGetVar(wp, T("username"), T(""));
	strOldPassword = websGetVar(wp, T("oldpass"), T(""));
	strPassword = websGetVar(wp, T("newpass"), T(""));
	strConfPassword = websGetVar(wp, T("confpass"), T(""));
	
	if ( !strOldPassword[0] ) {
		strcpy(tmpBuf, T("ERROR: Old Password cannot be empty."));
		goto setErr_pass;
	}
	
	if ( !strPassword[0] ) {
		strcpy(tmpBuf, T("ERROR: New Password cannot be empty."));
		goto setErr_pass;
	}	

	if ( !strConfPassword[0] ) {
		strcpy(tmpBuf, T("ERROR: Confirmed Password cannot be empty."));
		goto setErr_pass;
	}

	if (strcmp(strPassword, strConfPassword) != 0 ) {
		strcpy(tmpBuf, T("ERROR: New Password is not the same as Confirmed Password."));
		goto setErr_pass;
	}
		
	
#ifdef ACCOUNT_CONFIG
	totalEntry = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL);
	for (i=0; i<totalEntry; i++) {
		if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, i, (void *)&Entry))
			continue;
		if (Entry.privilege == (unsigned char)PRIV_ROOT)
			continue;
		#ifdef USE_LOGINWEB_OF_SERVER
		if(!strcmp(g_login_username, Entry.userName))
		#else
		if (strcmp(wp->user, Entry.userName) == 0)
		#endif
		{
			selected = i;
			break;
		}
	}
	if (selected != -1) {
		if (strcmp(Entry.userPassword, strOldPassword) != 0) {
			strcpy(tmpBuf, T(WARNING_WRONG_PASSWORD));
			goto setErr_pass;
		} else {
			strncpy(Entry.userPassword, strPassword, MAX_NAME_LEN-1);
			Entry.userPassword[MAX_NAME_LEN-1] = '\0';
			//Entry.userPassword[MAX_NAME_LEN] = '\0';
		}
		Entry.privilege = (unsigned char)getAccPriv(Entry.userName);
		mib_chain_update(MIB_ACCOUNT_CONFIG_TBL, (void *)&Entry, selected);
	}
	else {
#endif
	if ( !mib_get(MIB_USER_PASSWORD, (void *)tmpBuf)) {
		strcpy(tmpBuf, T("ERROR: Get user password MIB error!"));
		goto setErr_pass;
	}
	
	if ( strcmp(tmpBuf, strOldPassword) != 0 ) {
		strcpy(tmpBuf, T("ERROR: Input Old user password error!"));
		goto setErr_pass;
	}else if ( !mib_set(MIB_USER_PASSWORD, (void *)strPassword) ) {							
		strcpy(tmpBuf, T("ERROR: Set user password to MIB database failed."));
		goto setErr_pass;
	}	
#ifdef ACCOUNT_CONFIG
	}
#endif
	
#ifdef EMBED
	// Added by Mason Yu for take effect on real time
	writePasswdFile();
	write_etcPassword();	// Jenny
#endif

	/*
	if (mib_update(HW_SETTING) == 0) {
		printf("Warning : Commit hs fail(formPasswordSetup()) !\n");				
	}
	*/

	/* upgdate to flash */
//	mib_update(CURRENT_SETTING);

	/* Init user management */
	// Commented By Mason Yu
	//set_user_profile();
	
	/* Retrieve next page URL */
	submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page

	OK_MSG(submitUrl);
	return;

setErr_pass:
	ERR_MSG(tmpBuf);
}

////////////////////////////////////////////////////////////////////
void set_user_profile(void)
{
	char superName[MAX_NAME_LEN], superPass[MAX_NAME_LEN];
	char userName[MAX_NAME_LEN], userPass[MAX_NAME_LEN];
	char dport[10];
//	char_t *user, *nextUser, *group;
	
	/* first time load, get mib */
	if ( !mib_get( MIB_SUPER_NAME, (void *)superName ) ||
		!mib_get( MIB_SUSER_PASSWORD, (void *)superPass ) ||
			!mib_get( MIB_USER_NAME, (void *)userName ) ||
				!mib_get( MIB_USER_PASSWORD, (void *)userPass ) ) {
		error(E_L, E_LOG, T("Get user account MIB failed"));
		return;
	}

	/* Delete all user account belonging to DEFAULT_GROUP */
	/*
	user = umGetFirstUser();
	while (user) {
//		printf("umDeleteUser (user=%s).\n", user);
		nextUser = umGetNextUser(user);
		group = umGetUserGroup(user);
		if (gstrcmp(DEFAULT_GROUP, group) == 0) {
			if ( umDeleteUser(user) ) {
				printf("ERROR: Unable to delete user account (user=%s).\n", user);
				return;
			}
		}
		
		user = nextUser;
		//user = umGetFirstUser();
	}
	*/

	umDeleteAccessLimit(ACCESS_URL);
	umDeleteGroup(DEFAULT_GROUP);

	if ( userName[0] ) {
		/* Create supervisor */
		if ( !umGroupExists(DEFAULT_GROUP) )
			if ( umAddGroup(DEFAULT_GROUP, (short)PRIV_ADMIN, AM_BASIC, FALSE, FALSE) ) {
				error(E_L, E_LOG, T("ERROR: Unable to add group."));
				return;
			}
		if ( !umAccessLimitExists(ACCESS_URL) ) {
			if ( umAddAccessLimit(ACCESS_URL, AM_FULL, (short)0, DEFAULT_GROUP) ) {
				error(E_L, E_LOG, T("ERROR: Unable to add access limit."));
				return;
			}
		}

		/* Create user */
		if ( umAddUser(superName, superPass, DEFAULT_GROUP, FALSE, FALSE) ) {
			error(E_L, E_LOG, T("ERROR: Unable to add supervisor account."));
			return;
		}			

		/* Create user */
		if ( umAddUser(userName, userPass, DEFAULT_GROUP, FALSE, FALSE) ) {
			error(E_L, E_LOG, T("ERROR: Unable to add user account."));
			return;
		}
	}
	else {
		if (g_remoteConfig) {	// remote config not allowed
			char ipaddr[20], tmpStr[5];
			
			if (g_rUserName[0]) {
				if ( umDeleteUser(g_rUserName) ) {
					printf("ERROR: Unable to delete user account (user=%s).\n", g_rUserName);
					return;
				}
				g_rUserName[0] = '\0';
			}
			
			mib_get(MIB_ADSL_LAN_IP, (void *)tmpStr);
			strncpy(ipaddr, inet_ntoa(*((struct in_addr *)tmpStr)), 16);
			ipaddr[15] = '\0';
			snprintf(ipaddr, 20, "%s:80", ipaddr);
			sprintf(dport, "%d", g_remoteAccessPort);
			// iptables -D INPUT -i ! br0 -p TCP --dport 80 -j ACCEPT
			va_cmd(IPTABLES, 11, 1, (char *)FW_DEL, (char *)FW_INPUT, ARG_I,
			"!", LANIF, "-p", ARG_TCP, FW_DPORT, "80", "-j", (char *)FW_ACCEPT);
			// iptables -t nat -D PREROUTING -i ! $LAN_IF -p TCP --dport 51003 -j DNAT --to-destination ipaddr:80
			va_cmd(IPTABLES, 15, 1, "-t", "nat",
						(char *)FW_DEL,	"PREROUTING",
						(char *)ARG_I, "!", (char *)LANIF,
						"-p", (char *)ARG_TCP,
						(char *)FW_DPORT, dport, "-j",
						"DNAT", "--to-destination", ipaddr);
			g_remoteConfig = 0;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// search token szKey from string szString
// if find, return its value, else return null
char* SearchKeyValue(char* szString, char* szKey)
{
	char *szDuplicate;
	char *key, *lp, *cp, *value;
	
	//duplicate the string, avoid the original string to be modefied
	szDuplicate = bstrdup(B_L, szString);

	for (lp = szDuplicate ; lp && *lp; ) 
	{
		cp = lp;
		if ((lp = gstrchr(lp, ';')) != NULL) 
		{
			lp++;
		}

		if ((key = gstrtok(cp, T("= \t;"))) == NULL) 
		{
			continue;
		}

		if ((value = gstrtok(NULL, T(";"))) == NULL) 
		{
			value = T("");
		}

		while (gisspace(*value)) 
		{
			value++;
		}

		if(strcmp(key, szKey) == 0)
		{
			return value;
		}
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// find key szKey form string szString
// start from nStart to nEnd of szString
// if found, return the first index of the matched string
// if not found, return -1
static int FindKeyIndex(char *szKey, char * szString, int nStart, int nEnd)
{
	int nKeyLen = strlen(szKey);
	char *szSearch = szString + nStart;
	char *szSearchEnd = szString + nEnd;
	int nIndex = 0;
	bool bFind = false;
	while(szSearch != szSearchEnd)
	{
		if(memcmp(szSearch, szKey, nKeyLen) ==0)
		{
			bFind = true;
			break;
		}
		else
		{
			nIndex++;
			szSearch++;
		}
	}

	if(bFind == true)
		return (nIndex + nStart);
	else
		return -1;
	
}

#if 0
void releaseUploadMem(webs_t wp)
{
//	unsigned char* tempPtr = wp->queryNext[firmware_page_idx];
	int counter;

//	printf("query 0 (nFirst %d) %02x-%02x-%02x-%02x\n", nFirst, wp->query[nFirst], wp->query[nFirst+1], wp->query[nFirst+2], wp->query[nFirst+3]);
//	printf("Last %02x-%02x-%02x-%02x\n", tempPtr[nLast-4], tempPtr[nLast-3], tempPtr[nLast-2], tempPtr[nLast-1]);

	for(counter=0; counter<MAX_UPLOAD_BLOCK_NUM; counter++)
	{
		if(wp->queryNext[counter])
		{
			unsigned char* queryPtr = wp->queryNext[counter];
//			printf("free %d, %08x, %02x-%02x-%02x-%02x\n", counter, (unsigned int)wp->queryNext[counter] , queryPtr[0], queryPtr[1], queryPtr[2], queryPtr[3]);
			free(wp->queryNext[counter]);
			wp->queryNext[counter] = NULL;				
		}
	}
}
#endif
	
#ifdef EMBED
int flashdrv_filewrite(FILE *fp,int size,void  *dstP) //Brian
{
	int i;
	int nWritten; /* completed bytes */
	//volatile unsigned char data[4096];
	volatile unsigned short *dstAddr;
	unsigned char *block;
	nWritten = 0;
	dstAddr = dstP;
	
	block = malloc(FLASH_BLOCK_SIZE);
	if(!block)
	return 1;
	
	// Mason Yu
	flashdrv_init();
	
	while ( nWritten < size )
	{
		int nRead;
		int writebyte;

		/* fill buffer with file */
		for( i = 0, nRead = 0; 
		     i < FLASH_BLOCK_SIZE && ( i < (size-nWritten) );
		     i++, nRead++ )
		{
			block[i] = fgetc(fp);
		}
		
		printf("flashWrite --> %08x (len %d)\n", dstAddr, nRead);
		//flash_write( (void*)block, (void*)dstAddr, nRead );
		//flashdrv_updateImg( (void*)block, (void*)(FLASH_BASE+dstAddr), nRead );
		writebyte = flashdrv_updateImg( (void*)block, (void*)(FLASH_BASE+dstAddr), nRead );
		printf("flashdrv_filewrite:  len=%d writebyte=%d\n", nRead, writebyte);

		dstAddr += nRead >> 1;
		nWritten += nRead;
	}
	
	free(block);
	return 0;
}
#endif


static int isValidImageFile(FILE *fp) {
	IMGHDR imgHdr;
	unsigned int csum;
	int size, remain, nRead, block;
	unsigned char buf[64];
#if defined(ZTE_GENERAL_ROUTER_SC) || defined(ZTE_531B_BRIDGE_SC)
	SIGHDR sigHdr;
	unsigned int hdrChksum;
	int i;
#endif
	/*ql: 20080729 START: check image key according to IC version*/
#ifdef MULTI_IC_SUPPORT
	unsigned int key;
#endif
	/*ql: 20080729 END*/

#if defined(ENABLE_SIGNATURE)
	//ql_xu add: check if the img signature is right
	memset(&sigHdr, 0, sizeof(SIGHDR));
	if (1 != fread(&sigHdr, sizeof(sigHdr), 1, fp)) {
		printf("failed to read signature header\n");
		goto ERROR1;
	}
#endif
	if (1!=fread(&imgHdr, sizeof(imgHdr), 1, fp)) {
		printf("Failed to read header\n");
		goto ERROR1;
	}
#ifndef ENABLE_SIGNATURE_ADV
#ifdef ENABLE_SIGNATURE
	printf("sig len: %d\n", sigHdr.sigLen);
	if (sigHdr.sigLen > SIG_LEN) {
		printf("signature length error\n");
		goto ERROR1;
	}
	for (i=0; i<sigHdr.sigLen; i++)
		sigHdr.sigStr[i] = sigHdr.sigStr[i] - 10;
	if (strcmp(sigHdr.sigStr, SIGNATURE)) {
		printf("signature error\n");
		goto ERROR1;
	}

	hdrChksum = sigHdr.chksum;
	hdrChksum = ipchksum(&imgHdr, sizeof(imgHdr), hdrChksum);
	if (hdrChksum) {
		printf("Checksum failed, size=%d, csum=%04xh\n", sigHdr.sigLen, hdrChksum);
		goto ERROR1;
	}
#endif
#endif

	/*ql: 20080729 START: get sachem version, determine the IC version and then get correct img key.*/
#ifdef MULTI_IC_SUPPORT
	key = getImgKey();
	
	if ((key != (imgHdr.key & key)) || (((imgHdr.key>>28)&0xf) != ((key>>28)&0xf))) {
		printf("Unknown header\n");
		goto ERROR1;
	}
#else
	if (imgHdr.key != APPLICATION_IMAGE) {
		printf("Unknown header\n");
		goto ERROR1;
	}
#endif
	/*ql: 20080729 END*/

	csum = imgHdr.chksum;
	size = imgHdr.length;
	remain = size;

	while (remain > 0) {
		block = (remain > sizeof(buf)) ? sizeof(buf) : remain;		
		nRead = fread(buf, 1, block, fp);
		if (nRead <= 0) {
			printf("read too short (remain=%d, block=%d)\n", remain, block);
			goto ERROR1;
		}
		remain -= nRead;
		csum = ipchksum(buf, nRead,csum);
	}
	
	if (csum) {
		printf("Checksum failed, size=%d, csum=%04xh\n", size, csum);
		goto ERROR1;
	}

	return 1;
ERROR1:
	return 0;
}


// find the start and end of the upload file.
static FILE * _uploadGet(request *wp, unsigned int *startPos, unsigned *endPos) {
   
   FILE *fp=NULL;	
	struct stat statbuf;	
	unsigned char c, *buf; 

	if (wp->method == M_POST)
	{
	   fstat(wp->post_data_fd, &statbuf);
		lseek(wp->post_data_fd, SEEK_SET, 0);
      
		printf("file size=%d\n",statbuf.st_size);
		fp=fopen(wp->post_file_name,"rb");
		if(fp==NULL) goto error;
	}
	else goto error;

   
   //printf("_uploadGet\n");
   do
	{
		if(feof(fp))
		{
			printf("Cannot find start of file\n");
			goto error;
		}
		c= fgetc(fp);
		if (c!=0xd)
			continue;
		c= fgetc(fp);
		if (c!=0xa)
			continue;
		c= fgetc(fp);
		if (c!=0xd)
			continue;
		c= fgetc(fp);
		if (c!=0xa)
			continue;
		break;
	}while(1);
	(*startPos)=ftell(fp);

   if(fseek(fp,statbuf.st_size-0x200,SEEK_SET)<0) 
      goto error;
	do
	{
		if(feof(fp))
		{
			printf("Cannot find end of file\n");
			goto error;
		}
		c= fgetc(fp);
		if (c!=0xd)
			continue;
		c= fgetc(fp);
		if (c!=0xa)
			continue;
		c= fgetc(fp);
		if (c!='-')
			continue;
		c= fgetc(fp);
		if (c!='-')
			continue;
		break;
	}while(1);
	(*endPos)=ftell(fp);

   return fp;
error:
   return NULL;
}

#ifdef WEB_UPGRADE
// Added by Mason Yu
void displayUploadMessage(webs_t wp, int status)
{
	
	//printf("Popout web page\n");	
	websHeader(wp);
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	websWrite(wp, T("<META HTTP-EQUIV=Refresh CONTENT=\"60; URL=/status_sc.asp\">\n"));
      #else
      websWrite(wp, T("<META HTTP-EQUIV=Refresh CONTENT=\"60; URL=/status.asp\">\n"));
      #endif
   	websWrite(wp, T("<body><blockquote><h4>\n"));
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	if (status == ERROR_FILESIZE)    	
   		websWrite(wp, T("%s</h4>\n"), "软件升级失败 ! (文件太大)");
   	else if (status == ERROR_FORMAT)
   		websWrite(wp, T("%s</h4>\n"), "软件升级失败 ! (文件格式错误)");   
#else
   	if (status == ERROR_FILESIZE)    	
   		websWrite(wp, T("%s</h4>\n"), "Upgrade Firmware failed ! (file size exceeded)");
   	else if (status == ERROR_FORMAT)
   		websWrite(wp, T("%s</h4>\n"), "Upgrade Firmware failed ! (file format error)");   	
#endif	
   	websWrite(wp, T("%s<br><br>\n"), rebootWord0);
   	websWrite(wp, T("%s\n"), rebootWord2);
   	websWrite(wp, T("</blockquote></body>"));
   	websFooter(wp);
	websDone(wp, 200);
	
#ifdef EMBED
	// wake up the reboot thread
      	semSave.__sem_value=SEM_REBOOT;
	sem_post(&semSave);
#endif
}	


#ifdef UPGRADE_V1
///////////////////////////////////////////////////////////////////////////////
void formUpload(webs_t wp, char_t * path, char_t * query)
{
   	unsigned int startPos,endPos, nLen;
   	char tmpBuf[100]/*, *submitUrl*/;
	FILE	*fp=NULL;
	char buff[256], tmp1[20], tmp2[20], *ends;
	int mtdSize;

#ifdef CLOSE_OTHER_PROCESS
        printf("SYNC!!!...\n");
        sync();
        usleep(5);
#endif

	/* find the start and end positive of run time image */
   	tmpBuf[0] = '\0';
	//printf("\nTry to get file size of new firmware\n"); 	

	
#ifdef ENABLE_SIGNATURE_ADV
	if (upgrade != 2) {//signature Err
		strcpy(tmpBuf, T(FILEWITHWRONGSIG));
		goto fail;
	}
#endif

	// Mason Yu
	//if ( g_filesize >= MAX_UPLOAD_FILESIZE) {
	// Jenny, check MTD block size
	if (!(fp=fopen("/proc/mtd", "r")))
		printf("/proc/mtd not exists.\n");
	else {
		fgets(buff, sizeof(buff), fp);
		while (fgets(buff, sizeof(buff), fp) != NULL) {
			if (sscanf(buff, "%*s%s%*s%s", tmp1, tmp2) != 2) {
				printf("Unsuported MTD partition format\n");
				break;
			}
			else
				mtdSize = strtol(tmp1, &ends, 16);
			if (strcmp(tmp2, "\"rootfs\"") == 0)
				break;
		}
		fclose(fp);
	}
	if (g_filesize >= mtdSize) {
		displayUploadMessage(wp, ERROR_FILESIZE);
		//goto end;
		goto fail;
	}

   	if ( (fp = _uploadGet(wp, &startPos, &endPos)) == NULL){
      		strcpy(tmpBuf, T(FILEOPENFAILED));
      		goto fail;	
   	}
	
	/* check header and checksum of this image */
	printf("endPos=%u startPos=%u\n",endPos,startPos);
   	nLen = endPos - startPos - 4;

   #ifdef EMBED
	// write to flash   
	{
		//unsigned char *block;
      	//int i, nRead, nWrite, rv;		
      	int rv;
	fseek(fp, startPos, SEEK_SET); // seek to the data star
	if (!isValidImageFile(fp)) {
		// Mason Yu
		printf("Incorrect image file\n");
		displayUploadMessage(wp, ERROR_FORMAT);
		goto end;
		//goto fail;
	}
	
	fseek(fp, startPos+sizeof(IMGHDR), SEEK_SET); // seek to the data star 
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	//jim luo
	int writeflashtime;
	Modem_LinkSpeed vLs;
	vLs.upstreamRate=0;
	if (adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs,RLCM_GET_LINK_SPEED_SIZE) && vLs.upstreamRate != 0)
		writeflashtime=g_filesize/17400; //star: flash can wirte about 17k in 1 sec with the adsl line up
	else
		writeflashtime=g_filesize/21000;//star: flash can wirte about 21k in 1 sec with the adsl line down
        websWrite(wp, T("<html><head><style>\n" \
        "#cntdwn{ border-color: white;border-width: 0px;font-size: 12pt;color: red;text-align:left; font-weight:bold; font-family: Courier;}\n" \
        "</style><script language=javascript>\n" \
        "var h=(%d+70);\n" \
        "function stop() { clearTimeout(id); }\n"\
	 "function start() { h--; if (h >= 70) { frm.time.value = h; frm.textname.value='  软件升级中，请等待...'; id=setTimeout(\"start()\",1000); }\n" \
        "if (h >= 0 && h < 70) { frm.time.value = h; frm.textname.value='  升级成功，系统重启中...'; id=setTimeout(\"start()\",1000); }\n" \
        "if (h == 0) { window.open(\"/status_sc.asp\",target=\"view\"); }}\n" \
        "</script></head><body bgcolor=white  onLoad=\"start();\" onUnload=\"stop();\">" \
        "<form name=frm><B><font color=red><INPUT TYPE=text NAME=textname size=40 id=\"cntdwn\">\n" \
        "<INPUT TYPE=text NAME=time size=5 id=\"cntdwn\"></form></body></html>") ,writeflashtime);    
#else
        // Added by Mason Yu  
        websWrite(wp, T("<html><head><style>\n" \
        "#cntdwn{ border-color: white;border-width: 0px;font-size: 12pt;color: red;text-align:left; font-weight:bold; font-family: Courier;}\n" \
        "</style><script language=javascript>\n" \
        "var h=120;\n" \
        "function stop() { clearTimeout(id); }\n"\
        "function start() { h--; if (h >= 55) { frm.time.value = h; frm.textname.value='Firmware upgrading, Please wait ...'; id=setTimeout(\"start()\",1000); }\n" \
        "if (h >= 0 && h < 55) { frm.time.value = h; frm.textname.value='System restarting, Please wait ...'; id=setTimeout(\"start()\",1000); }\n" \
        "if (h == 0) { window.open(\"/status.asp\",target=\"view\"); }}\n" \
        "</script></head><body bgcolor=white  onLoad=\"start();\" onUnload=\"stop();\">" \
        "<blockquote><form name=frm><B><font color=red><INPUT TYPE=text NAME=textname size=40 id=\"cntdwn\">\n" \
        // for tiscali
        //"<INPUT TYPE=text NAME=time size=5 id=\"cntdwn\"></form></body></html>") );    
        "<INPUT TYPE=text NAME=time size=5 id=\"cntdwn\"></font></form>") );
        websWrite(wp, T(
        "<h4>Please note do NOT power off the device during the upload because it may crash the system.</h4>\n"
        "</blockquote></body></html>"));
#endif 
	// Save file for upgrade Firmware
	g_upgrade_firmware=TRUE;

	// wake up the Upgrade/reboot thread
      	semSave.__sem_value = SEM_UPGRADE;
      	g_post_file_name = wp->post_file_name;
      	g_startPos = startPos;
	sem_post(&semSave);	
		 	
   }
   #endif
   
end:   
	//OK_MSG("/upload.asp");   
   return;

fail:

#ifdef ENABLE_SIGNATURE_ADV
	remove(wp->post_file_name);
	#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	OK_MSG1(tmpBuf, "/admin/upload_sc.asp");
	#else
	OK_MSG1(tmpBuf, "/upload.asp");
	#endif
#else
	//ERR_MSG(tmpBuf);
	#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	OK_MSG1(tmpBuf, "/admin/upload_sc.asp");
	#else
	OK_MSG1(tmpBuf, "/upload.asp");
	#endif
#endif
	return;
}
#else
#if defined(UPGRADE_V2) || defined(UPGRADE_V3)
void formUpload(webs_t wp, char_t * path, char_t * query)
{
   	char *str;
   	int action;
	FILE *fp;
   	
	str = websGetVar(wp, T("act"), T(""));
	if (str[0]) {
		// replace by mini web
		action = str[0] - '0';
	}
	else
		return;
	
#ifdef EMBED
	if (action==1) {
		// disable ip forwarding
		if (fp = fopen(PROC_IPFORWARD, "w"))
		{
			fprintf(fp, "0\n");
			fclose(fp);
		}
	      	semSave.__sem_value = SEM_UPGRADE_2;
		sem_post(&semSave);
	}
	else
	if (action==2) { // UPGRADE_V3 --- reboot to upgrade mode
		char bm = (char)BOOT_UPGRADE;
		// save to indicate the upgrade mode
		mib_set(MIB_BOOT_MODE, (void *)&bm);
		mib_update(HW_SETTING, CONFIG_MIB_ALL);
		websHeader(wp);
   		websWrite(wp, T("<body onLoad=\"top.location='/toupgrade.html';\">\n"));
   		websWrite(wp, T("</body >\n"));
   		websFooter(wp);
		//OK_MSG1("The system is restarting ...\n", NULL);
		semSave.__sem_value=SEM_REBOOT;
		sem_post(&semSave);
	}
#endif
	
}
#endif // of UPGRADE_V2
#endif // of UPGRADE_V1
#endif // of WEB_UPGRADE

#if 0
//return 0:OK, other:fail
int call_cmd(const char *filename, int num, int dowait, ...)
{
	va_list ap;
	char *s;
	char *argv[24];
	int status=0, st, k;
	pid_t pid, wpid;
	
	va_start(ap, dowait);
	
	for (k=0; k<num; k++)
	{
		s = va_arg(ap, char *);
		argv[k+1] = s;
	}
	
	argv[k+1] = NULL;
	if((pid = vfork()) == 0) {
		/* the child */
		char *env[3];
		
		signal(SIGINT, SIG_IGN);
		argv[0] = (char *)filename;
		env[0] = "PATH=/bin:/usr/bin:/etc:/sbin:/usr/sbin";
		env[1] = NULL;

		execve(filename, argv, env);

		printf("exec %s failed\n", filename);
		_exit(2);
	} else if(pid > 0) {
		if (!dowait)
			status = 0;
		else {
			/* parent, wait till rc process dies before spawning */
			while ((wpid = wait(&st)) != pid)
				if (wpid == -1 && errno == ECHILD) { /* see wait(2) manpage */
					break;
				}
		}
	} else if(pid < 0) {
		printf("fork of %s failed\n", filename);
		status = -1;
	}
	if (wpid>0)
		if (WIFEXITED(st))
			status = WEXITSTATUS(st);
//			printf("Child exited with RC=%d\n",WEXITSTATUS(st));
	va_end(ap);
	
	return status;
}
#endif
///////////////////////////////////////////////////////////////////////////////
/*
 *	Tag: load, Value:Upload - upload configuration file
 *    Tag: save, Value:Save... - save configuration file
 *    Tag: reset, Value:Rest - reset configuration to default
 */

#include <semaphore.h>
extern sem_t semSave;
//#define MAX_CONFIG_FILESIZE 200000
void formSaveConfig(webs_t wp, char_t *path, char_t *query)
{
	char_t *strRequest;
	unsigned int maxFileSector;

	char tmpBuf[100], *submitUrl;

	CONFIG_DATA_T action_type = UNKNOWN_SETTING;

	wp->buffer_end=0; // clear header
   	tmpBuf[0] = '\0';
	
	if (g_filesize > MIN_UPLOAD_FILESIZE) {
		websHeader(wp);
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		websWrite(wp, T("<META HTTP-EQUIV=Refresh CONTENT=\"60; URL=/status_sc.asp\">\n"));
		websWrite(wp, T("<body><blockquote><h4>\n"));
		websWrite(wp, T("配置文件恢复失败! 文件太大!<br>"));
          #else
            	websWrite(wp, T("<META HTTP-EQUIV=Refresh CONTENT=\"60; URL=/status.asp\">\n"));
	    	websWrite(wp, T("<body><blockquote><h4>\n"));
	    	websWrite(wp, T("Restore settings from config file failed! Uploaded file size out of constraint!<br>"));
          #endif	
   		websWrite(wp, T("%s</h4>\n"), rebootWord0);
		websWrite(wp, T("<br>%s\n"), rebootWord2);
		websWrite(wp, T("</blockquote></body>"));
		websFooter(wp);
		websDone(wp, 200);
		goto end;
	}
	else if (g_filesize >= MAX_CONFIG_FILESIZE) {
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		strcpy(tmpBuf, T("配置文件恢复失败! 上传文件太大!\n"));
#else
		strcpy(tmpBuf, T("ERROR: Restore Config file failed! Uploaded file size out of constraint!\n"));
#endif
		goto setErr_pass;
	}
	
	strRequest = websGetVar(wp, T("save_ds"), T(""));
	if (strRequest[0])
	{
		action_type = DEFAULT_SETTING;
		maxFileSector = DEFAULT_SETTING_MAX_LEN/10;
	}

	strRequest = websGetVar(wp, T("save_cs"), T(""));
	if (strRequest[0])
	{
		action_type = CURRENT_SETTING;
		maxFileSector = CURRENT_SETTING_MAX_LEN/10;
	}

	strRequest = websGetVar(wp, T("save_hs"), T(""));
	if (strRequest[0])
	{
		action_type = HW_SETTING;
		maxFileSector = HW_SETTING_MAX_LEN/10;
	}

	if (action_type != UNKNOWN_SETTING)
	{	// save configuration file
		PARAM_HEADER_T header;
		unsigned char *ptr;
		unsigned int fileSize,filelen;
		unsigned int fileSector;

#ifdef XML_CONFIG
		FILE *fp;
		int ret=-1;
		//create config file
		ret = call_cmd("/bin/CreatexmlConfig", 0, 1);
	#ifdef XOR_ENCRYPT
		xor_encrypt("/tmp/config.xml", "/tmp/config_xor.xml");
	#endif
#else
		unsigned char *buf;

		if(mib_read_header(action_type, &header) != 1)
		{
			error(E_L, E_LOG, T("ERROR: Flash read fail"));
			strcpy(tmpBuf, T("ERROR: Flash read fail."));
			goto setErr_pass;
		}
		
		fileSize = sizeof(PARAM_HEADER_T) + header.len;
		buf = malloc(fileSize);
		if ( buf == NULL ) {
			strcpy(tmpBuf, "Allocate buffer failed!");
			goto setErr_pass;
		}

		printf("fileSize=%d\n",fileSize);
		if(mib_read_to_raw(action_type, buf, fileSize) != 1)
		{
			free(buf);
			
			error(E_L, E_LOG, T("ERROR: Flash read fail"));
			strcpy(tmpBuf, T("ERROR: Flash read fail."));
			goto setErr_pass;
		}
#endif
		websWrite(wp, "HTTP/1.0 200 OK\n");
		websWrite(wp, "Content-Type: application/octet-stream;\n");

		if(action_type == CURRENT_SETTING)
#ifdef XML_CONFIG
		websWrite(wp, "Content-Disposition: attachment;filename=\"config.xml\" \n");
#else
		websWrite(wp, "Content-Disposition: attachment;filename=\"adsl-config.bin\" \n");
#endif
		else if(action_type == DEFAULT_SETTING)
		websWrite(wp, "Content-Disposition: attachment;filename=\"adsl-config-ds.bin\" \n");
		else if(action_type == HW_SETTING)
		websWrite(wp, "Content-Disposition: attachment;filename=\"adsl-config-hs.bin\" \n");
#ifdef 	SERVER_SSL
		// IE bug, we can't sent file with no-cache through https
#else 
		websWrite(wp, "Pragma: no-cache\n");
		websWrite(wp, "Cache-Control: no-cache\n");
#endif		
		websWrite(wp, "\n");

#ifdef XML_CONFIG
	#ifdef XOR_ENCRYPT
		fp=fopen("/tmp/config_xor.xml","r");
	#else
		fp=fopen("/tmp/config.xml","r");
	#endif
		//decide the file size	
		fseek(fp, 0, SEEK_END);
		filelen = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		fileSize=filelen;
	#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		if(fileSize>0)
		{	//generate keyword to configfile,if bridge
		#ifdef ZTE_531B_BRIDGE_SC
			websWriteDataNonBlock(wp, &ZTE_Bridge_Config_Keyword, 4);
		#else
			websWriteDataNonBlock(wp, &ZTE_Router_Config_Keyword, 4);
		#endif
		}
	#endif
		while(fileSize>0)
		{
			char buf[filelen*5];
			int nRead;
			
//			printf("write %d %d %08x\n",maxFileSector, fileSize, (unsigned int )ptr);
			fileSector = (fileSize>maxFileSector)?maxFileSector:fileSize;
			nRead = fread((void *)buf, 1, fileSector, fp);
	#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
			ZTE_Encrypt(buf, buf, 26, nRead);
	#endif
			websWriteDataNonBlock(wp, buf, nRead);
			
			fileSize -= fileSector;
			ptr += fileSector;
			//wrong free....
			free(buf);
 			//sleep(1);
		}
		fclose(fp);
#else
		ptr = buf;
	#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		if(fileSize>0)
		{	//generate keyword to configfile,if bridge
		#ifdef ZTE_531B_BRIDGE_SC
			websWriteDataNonBlock(wp, &ZTE_Bridge_Config_Keyword, 4);
		#else
			websWriteDataNonBlock(wp, &ZTE_Router_Config_Keyword, 4);
		#endif
		}
	#endif
		while(fileSize>0)
		{
//			printf("write %d %d %08x\n",maxFileSector, fileSize, (unsigned int )ptr);
			fileSector = (fileSize>maxFileSector)?maxFileSector:fileSize;
			websWriteDataNonBlock(wp, ptr, fileSector);
			
			fileSize -= fileSector;
			ptr += fileSector;
 			sleep(1);
		}
		free(buf);
#endif		
		//websDone(wp, 200);
		//OK_MSG("/saveconf.asp");		
		return;
	}

	strRequest = websGetVar(wp, T("reset"), T(""));
	if (strRequest[0])
	{	// reset configuration to default
		char_t *submitUrl=0;

#if 0
		if(mib_reset(CURRENT_SETTING) == 0)
		{
			error(E_L, E_LOG, T("ERROR: reset fail."));
			strcpy(tmpBuf, T("ERROR: reset fail."));
			goto setErr_pass;
		}
#endif
		
		//submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page

		//OK_MSG(submitUrl);
		// Modified by Mason Yu. for not use default setting	
		/*	
		mib_load(DEFAULT_SETTING, CONFIG_MIB_ALL);
		formReboot(wp, NULL, NULL);	
		*/
		
		// Jenny, add reboot messages when reset to default
		websHeader(wp);
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	          websWrite(wp, T("<META HTTP-EQUIV=Refresh CONTENT=\"60; URL=/status_sc.asp\">\n"));
                 #else
                websWrite(wp, T("<META HTTP-EQUIV=Refresh CONTENT=\"60; URL=/status.asp\">\n"));
                 #endif	
		websWrite(wp, T("<body><blockquote><h4>\n"));
	   	websWrite(wp, T("%s</h4>\n"), rebootWord0);
   		websWrite(wp, T("%s<br><br>\n"), rebootWord1);
	   	websWrite(wp, T("%s\n"), rebootWord2);
   		websWrite(wp, T("</blockquote></body>"));
	   	websFooter(wp);
		websDone(wp, 200);

#ifdef EMBED
		va_cmd("/bin/flash", 2, 1, "default", "cs");
		semSave.__sem_value=SEM_REBOOT;
		
		/* Save and reboot the system */
		// wake up the Save/reboot thread
		sem_post(&semSave);
#endif
		return;
	}

#ifdef XML_CONFIG
	{
		FILE	*fp=NULL,*fp_input;
		unsigned char *buf;
		unsigned int startPos,endPos,nLen,nRead;
		int ret=-1;		
		
		if ((fp = _uploadGet(wp, &startPos, &endPos)) == NULL) {
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
			strcpy(tmpBuf, T("错误 :请选择文件"));
#else
			strcpy(tmpBuf, T("ERROR: find the start and end of the upload file failed!"));
#endif
         		goto setErr_pass;
		}

		/* check header and checksum of this image */
		//printf("endPos=%u startPos=%u\n",endPos,startPos);
      		nLen = endPos - startPos - 4;
      		//printf("filesize is %d\n", nLen);
      		buf = malloc(nLen);
      		if (!buf) {
	      		fclose(fp);
         		goto setErr_pass;
      		}

      		fseek(fp, startPos, SEEK_SET);
      		nRead = fread((void *)buf, 1, nLen, fp);
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		// ZTE 531B  Bridge need the ability to disable wrong config file to open the router mode. then we should encrypt the 
		// file and indicate if bridge or router config file.
		//check keyword
	#ifdef ZTE_531B_BRIDGE_SC
		if(!memcmp(buf, &ZTE_Bridge_Config_Keyword, 4))
	#else
		if(!memcmp(buf, &ZTE_Router_Config_Keyword, 4))
	#endif
		{
			memmove(buf, buf+4, nRead-4);
			*(buf+nRead-4)=0; //add terminate .
			ZTE_Deencrypt(buf, buf, 26, nRead-4);
			printf("Config File: \n%s", buf);
		}else
		{
			//destroy config file
			memset(buf, 0x12, nRead-4);
		}
		
#endif
      		fclose(fp);
      		if (nRead != nLen)
         		printf("Read %d bytes, expect %d bytes\n", nRead, nLen);
	
      		//printf("write to %d bytes from %08x\n", nLen, buf);
     		
	#ifdef XOR_ENCRYPT
		fp_input=fopen("/tmp/config_xor.xml","w");
	#else
      		fp_input=fopen("/tmp/config.xml","w");
	#endif
      		if (!fp_input)
			printf("Get config file fail!\n");
	#ifdef XOR_ENCRYPT
		fwrite((void *)buf, 1, nLen, fp_input);
	#else
      		fprintf(fp_input,buf);
	#endif
      		printf("create file config.xml\n");
      		free(buf);
      		fclose(fp_input);
	#ifdef XOR_ENCRYPT
		xor_encrypt("/tmp/config_xor.xml", "/tmp/config.xml");
		unlink("/tmp/config_xor.xml");
	#endif
		ret = call_cmd("/bin/LoadxmlConfig", 0, 1);
		
		if (ret == 0) {
			websHeader(wp);
				#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	                   websWrite(wp, T("<META HTTP-EQUIV=Refresh CONTENT=\"60; URL=/status_sc.asp\">\n"));
                #else
			websWrite(wp, T("<META HTTP-EQUIV=Refresh CONTENT=\"60; URL=/status.asp\">\n"));
                #endif
   			websWrite(wp, T("<body><blockquote><h4>\n"));
			 #if defined( ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
			websWrite(wp, T("成功恢复系统配置! \n<br>"));
			#else
	   		websWrite(wp, T("Restore settings from config file successful! \n<br>"));
			#endif
	   		websWrite(wp, T("%s</h4>\n"), rebootWord0);
   			websWrite(wp, T("%s<br><br>\n"), rebootWord1);
   			websWrite(wp, T("%s\n"), rebootWord2);
	   		websWrite(wp, T("</blockquote></body>"));
   			websFooter(wp);
			websDone(wp, 200);
			sem_post(&semSave);
		}
		else {
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
			strcpy(tmpBuf, T("错误:系统配置恢复失败,文件有误 !"));
#else
			strcpy(tmpBuf, T("ERROR: Restore Config file failed! Invalid config file!"));
#endif
			goto setErr_pass;
		}
	}
#else
   if (1)
   {
      unsigned int startPos,endPos,nLen,nRead;
	   FILE	*fp=NULL;	
	   //struct stat statbuf;	
	   unsigned char *buf; 

	   /* find the start and end positive of run time image */
	   printf("\nTry to get file size of new firmware\n");
      
      if ((fp = _uploadGet(wp, &startPos, &endPos)) == NULL)
         goto setErr_pass;

	   /* check header and checksum of this image */
	   printf("endPos=%u startPos=%u\n",endPos,startPos);

      nLen = endPos - startPos - 4;
      //printf("filesize is %d\n", nLen);
      buf = malloc(nLen);
      if (!buf)
         goto setErr_pass;

      fseek(fp, startPos, SEEK_SET);
      nRead = fread((void *)buf, 1, nLen, fp);
      if (nRead != nLen)
         printf("Read %d bytes, expect %d bytes\n", nRead, nLen);

      printf("write to %d bytes from %08x\n", nLen, buf);
      if(mib_update_from_raw(buf, nLen) != 1)
		{
			strcpy(tmpBuf, "Flash error!");
			free(buf);
			goto setErr_pass;
		}
      //submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page
      //printf("submitted val: %s\n", submitUrl);
		OK_MSG("/saveconf.asp");				
      free(buf);
   }
#endif

#if 0
	// should be upload configuration file
	if(1)
	{
		char_t *content_type = 0;
		char_t *boundary = 0;

		int nFirst, nLine, nLast, nLen;

		unsigned char *ptr;
		
		unsigned char *buf = 0;

		content_type = websGetVar(wp, T("CONTENT_TYPE"), T(""));
		if(content_type != NULL)
		{
			boundary = SearchKeyValue(content_type, "boundary");
		}

		if(content_type == 0 || boundary == 0)
		{
			error(E_L, E_LOG, T("ERROR: No content_type or No boundary!"));
			strcpy(tmpBuf, T("ERROR: Unable to add access limit."));
			goto setErr_pass;
		}

		// search for first boundary keyword
		 nFirst = FindKeyIndex(boundary, query, 0, wp->lenPostData);
		 if(nFirst == -1)
		 {
			error(E_L, E_LOG, T("ERROR: Http Data do not have first boundary!!"));
			strcpy(tmpBuf, T("ERROR: Http Data do not have first boundary!!"));
			goto setErr_pass;
		 }
		 nFirst += (strlen(boundary) + 2); //0D 0A

		// search for first empty line
		 nLine =  FindKeyIndex("\r\n", query, nFirst, wp->lenPostData);
		 while(nLine != nFirst)
		 {
			nFirst = nLine + 2;
			nLine =  FindKeyIndex("\r\n", query, nFirst, wp->lenPostData);
		 }
		 nFirst = nLine + 2; //0D 0A

		// search for second empty line
		 nLast = FindKeyIndex(boundary, query, nFirst, wp->lenPostData);
		 if(nLast == -1)
		 {
			error(E_L, E_LOG, T("ERROR: Http Data do not have second boundary!!"));
			strcpy(tmpBuf, T("ERROR: Http Data do not have second boundary!!"));
			goto setErr_pass;
		 }
		 nLast = nLast -2;	// boundary --

		nLen = nLast - nFirst -2; //0D 0A

		printf("Upload Configuration File, File Size = %d\n",nLen);

		ptr = (unsigned char *)(query + nFirst);			// point to start of File

		buf = malloc(nLen);
		if ( buf == NULL ) {
			strcpy(tmpBuf, "Allocate buffer failed!");
			goto setErr_pass;
		}
		memcpy(buf, ptr, nLen);

		if(mib_update_from_raw(buf, nLen) != 1)
		{
			strcpy(tmpBuf, "Flash error!");
			free(buf);
			goto setErr_pass;
		}

		free(buf);

		if(!mib_init())
		{
			strcpy(tmpBuf, "Error: Flash read fail!");
			free(buf);
			goto setErr_pass;
		}
		
		// search for submit-url
		 nFirst = FindKeyIndex("submit-url", query, nLast, wp->lenPostData);
		 if(nFirst != -1)
		 {
		 	nFirst += 10 + 1 + 4; //  submit-url + " + \r\n\r\n
		 	nLast = FindKeyIndex("\r\n", query, nFirst, wp->lenPostData);
		 	if((nLast != -1) && (nLast > nFirst))
		 	{
				memset((char *)tmpBuf, 0x00, 100);
				memcpy((char *)tmpBuf,(char *)(query + nFirst),nLast-nFirst);
				OK_MSG(tmpBuf);
				return;
		 	}
		 }

	}


	printf("fail!\n");
#endif
   return;
 	
 setErr_pass:
 		//ql-- should assign the URL to jump to.
		//ERR_MSG(tmpBuf);
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		OK_MSG1(tmpBuf, "/admin/saveconf_sc.asp");
#else
		OK_MSG1(tmpBuf, "/admin/saveconf.asp");
#endif
		return;

 end:
	sem_post(&semSave);
 	return;
}

#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
void formSnmpConfig(webs_t wp, char_t *path, char_t *query)
{
	char_t *str, *submitUrl;
	struct in_addr trap_ip;
	static char tmpBuf[100];

//star: for take effect
	unsigned int snmpchangeflag=0;
	unsigned char vChar;
	char_t origstr[128];
	unsigned char origip[16];
	unsigned char snmpVal, oid_snmpVal;

#ifdef ZTE_GENERAL_ROUTER_SC //star: for snmp on/off
	str = websGetVar(wp, T("onsnmp"), T(""));
	if ( str[0] ) {
		FILE *fp;
		fp=fopen("/var/run/snmpd.pid","r");
		if(fp!=NULL){
			strcpy(tmpBuf, T("snmp客户端程序已经在运行!"));
			close(fp);
			goto setErr_pass;
		}
		vChar=1;
		mib_set( MIB_SNMP_AUTORUN, (void *)&vChar);
		if (-1==restart_snmp(1)){
			strcpy(tmpBuf, T("snmp客户端程序开启失败!"));
			goto setErr_pass;
		}
		goto end_snmp;
	}

	str = websGetVar(wp, T("offsnmp"), T(""));
	if ( str[0] ) {
		restart_snmp(0);
		vChar=0;
		mib_set( MIB_SNMP_AUTORUN, (void *)&vChar);
		goto end_snmp;
	}
#endif
	
	// Enable/Disable SNMPD
	str = websGetVar(wp, T("save"), T(""));
	if (str[0]) {
		str = websGetVar(wp, T("snmp_enable"), T(""));
		if (str[0]) {
			if (str[0] == '0')
				snmpVal = 0;
			else
				snmpVal = 1;
			
			mib_get(MIB_SNMPD_ENABLE, (void *)&oid_snmpVal);
			if ( oid_snmpVal != snmpVal ) {
				snmpchangeflag = 1;
			}
			
			if ( !mib_set(MIB_SNMPD_ENABLE, (void *)&snmpVal)) {
				strcpy(tmpBuf, T("formSnmpConfig: set MIB_SNMPD_ENABLE fail!"));
				goto setErr_pass;
			}
		}		
 	}
	
#if !defined(ZTE_531B_BRIDGE_SC) && !defined(ZTE_GENERAL_ROUTER_SC)
	str = websGetVar(wp, T("snmpSysDescr"), T(""));
	if (str[0]) {
		mib_get(MIB_SNMP_SYS_DESCR, (void*)origstr);
		if(strcmp(origstr,str)!=0)
			snmpchangeflag = 1;
		if ( !mib_set(MIB_SNMP_SYS_DESCR, (void *)str)) {
			strcpy(tmpBuf, T("Set snmpSysDescr mib error!"));
			goto setErr_pass;
		}
	}

	str = websGetVar(wp, T("snmpSysContact"), T(""));
	if (str[0]) {
		mib_get(MIB_SNMP_SYS_CONTACT, (void*)origstr);
		if(strcmp(origstr,str)!=0)
			snmpchangeflag = 1;
		if ( !mib_set(MIB_SNMP_SYS_CONTACT, (void *)str)) {
			strcpy(tmpBuf, T("Set snmpSysContact mib error!"));
			goto setErr_pass;
		}
	}

	str = websGetVar(wp, T("snmpSysName"), T(""));
	if (str[0]) {
		mib_get(MIB_SNMP_SYS_NAME, (void*)origstr);
		if(strcmp(origstr,str)!=0)
			snmpchangeflag = 1;
		if ( !mib_set(MIB_SNMP_SYS_NAME, (void *)str)) {
			strcpy(tmpBuf, T("Set snmpSysName mib error!"));
			goto setErr_pass;
		}
	}

	str = websGetVar(wp, T("snmpSysLocation"), T(""));
	if (str[0]) {
		mib_get(MIB_SNMP_SYS_LOCATION, (void*)origstr);
		if(strcmp(origstr,str)!=0)
			snmpchangeflag = 1;
		if ( !mib_set(MIB_SNMP_SYS_LOCATION, (void *)str)) {
			strcpy(tmpBuf, T("Set snmpSysLocation mib error!"));
			goto setErr_pass;
		}
	}

	str = websGetVar(wp, T("snmpSysObjectID"), T(""));
	if (str[0]) {
		mib_get(MIB_SNMP_SYS_OID, (void*)origstr);
		if(strcmp(origstr,str)!=0)
			snmpchangeflag = 1;
		if ( !mib_set(MIB_SNMP_SYS_OID, (void *)str)) {
			strcpy(tmpBuf, T("Set snmpSysObjectID mib error!"));
			goto setErr_pass;
		}
	}
#endif
	
	str = websGetVar(wp, T("snmpCommunityRO"), T(""));
	if (str[0]) {
		mib_get(MIB_SNMP_COMM_RO, (void*)origstr);

		if(strcmp(origstr,str)!=0)
			snmpchangeflag = 1;
		if ( !mib_set(MIB_SNMP_COMM_RO, (void *)str)) {
			strcpy(tmpBuf, T(strSetcommunityROerror));
			goto setErr_pass;
		}
	}
	str = websGetVar(wp, T("snmpCommunityRW"), T(""));
	if (str[0]) {
		mib_get(MIB_SNMP_COMM_RW, (void*)origstr);
		if(strcmp(origstr,str)!=0)
			snmpchangeflag = 1;
		if ( !mib_set(MIB_SNMP_COMM_RW, (void *)str)) {
			strcpy(tmpBuf, T(strSetcommunityRWerror));
			goto setErr_pass;
		}
	}
	str = websGetVar(wp, T("snmpTrapIpAddr"), T(""));
	if ( str[0] ) {
		if ( !inet_aton(str, &trap_ip) ) {
			strcpy(tmpBuf, T(strInvalTrapIp));
			goto setErr_pass;
		}
		mib_get(MIB_SNMP_TRAP_IP, (void*)&origip);
		if(((struct in_addr*)origip)->s_addr != trap_ip.s_addr)
			snmpchangeflag = 1;
		if ( !mib_set(MIB_SNMP_TRAP_IP, (void *)&trap_ip)) {
			strcpy(tmpBuf, T(strSetTrapIperror));
			goto setErr_pass;
		}
	}

	/* upgdate to flash */
//	mib_update(CURRENT_SETTING);
	if(snmpchangeflag == 1)
	{
#ifndef ZTE_GENERAL_ROUTER_SC
// Kaohj
#if 0
#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
			/*to avoid the dead when wrriting the flash*/
		itfcfg("sar", 0);
		itfcfg("eth0", 0);
#ifdef WLAN_SUPPORT
		itfcfg("wlan0", 0);
#endif
#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
		mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);

#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		itfcfg("sar", 1);
		itfcfg("eth0", 1);
#ifdef WLAN_SUPPORT
		vChar=0;
		mib_get( MIB_WLAN_DISABLED, (void *)&vChar);
		if( vChar==0 )	itfcfg("wlan0", 1);
#endif //WLAN_SUPPORT
#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
#endif // of #if 0
//		RECONNECT_MSG(strIp);
//		req_flush(wp);
#if defined(APPLY_CHANGE)
		if ( snmpVal == 1 )       // on
			restart_snmp(1);
		else 
			restart_snmp(0);  // off
#endif				
		submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page
		OK_MSG(submitUrl);
#else
		submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page
		strcpy(tmpBuf, T("修改成功，如果要生效，请关闭再启动SNMP客户端!"));
		OK_MSG1(tmpBuf,submitUrl);
#endif
		return;
	}

	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
  	return;

#ifdef ZTE_GENERAL_ROUTER_SC
end_snmp:	
	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	if (submitUrl[0]){
		submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page
		strcpy(tmpBuf, T("设置成功!"));
		OK_MSG1(tmpBuf,submitUrl);
	}
	else
		websDone(wp, 200);
  	return;
#endif	
 setErr_pass:
	ERR_MSG(tmpBuf);	
}
#endif

#if 0 // DSL driver page
adsl_drv_set_cmd adsl_drv_set_cmd_list[] =
{
	{"adsl-drv-modem-start", "adsl-start-mode"},
	{"adsl-drv-modem-enable", NULL},
	{"adsl-drv-modem-disable", NULL},
	{"adsl-drv-retrain", NULL},
	{"adsl-drv-mode", "adsl-mode"},
	{"adsl-drv-txpower", "adsl-txpower"},
	{"adsl-drv-tuneper", "adsl-tuneper"},
	{"adsl-drv-bitswap-enable", NULL},
	{"adsl-drv-bitswap-disable", NULL},
	{"adsl-drv-pilotrelocat-enable", NULL},
	{"adsl-drv-pilotrelocat-disable", NULL},
	{"adsl-drv-trellis-enable", NULL},
	{"adsl-drv-trellis-disable", NULL},
	{"adsl-drv-vendorid", "adsl-vendorid"},
	{"adsl-drv-debugmode", "adsl-debugmode"},
	{"adsl-drv-testpsd", "adsl-testpsd"},
	{"adsl-drv-log-enable", NULL},
	{"adsl-drv-log-disable", NULL},

	{NULL, NULL}	
};


void formAdslDrv(webs_t wp, char_t *path, char_t *query)
{
	char_t *cmd, *str, *submitUrl;
	char tmpBuf[100];
	int idx;

	for (idx=0; adsl_drv_set_cmd_list[idx].cmd != NULL; idx++) {

		cmd = websGetVar(wp, adsl_drv_set_cmd_list[idx].cmd, T(""));
		if (cmd[0]) {
			str = NULL;
			if(adsl_drv_set_cmd_list[idx].value != NULL) {
				str = websGetVar(wp, adsl_drv_set_cmd_list[idx].value, T(""));
			}
			
			if(!setAdslDrvInfo(adsl_drv_set_cmd_list[idx].cmd, str)) {
				snprintf(tmpBuf, 100, "Set %s error!", adsl_drv_set_cmd_list[idx].cmd);
				goto setErr_pass;
			}
		}

	}

	submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page
	OK_MSG(submitUrl);
  	return;

 setErr_pass:
	ERR_MSG(tmpBuf);	
}
#endif


void formSetAdslTone(webs_t wp, char_t *path, char_t *query)
{
	char_t *submitUrl;
	short mode;
	int i, chan;
	char tmpBuf[100];
	char_t *strVal;
	unsigned char tone[64];	  // Added by Mason Yu for correct Tone Mib Type
	char_t *strApply, *strMaskAll, *strUnmaskAll;
	
	memset(tone, 0, sizeof(tone));
	
	strApply = websGetVar(wp, T("apply"), T(""));
	strMaskAll = websGetVar(wp, T("maskAll"), T(""));
	strUnmaskAll = websGetVar(wp, T("unmaskAll"), T(""));
	
	
	// get the channel number
	mib_get(MIB_ADSL_MODE, (void *)&mode);
	if (mode & ADSL_MODE_ADSL2P)
		chan = 512;	// ADSL2+
	else
		chan = 256;	// ADSL, ADSL2
	
	if (strApply[0]) {		
		for (i=0; i<chan; i++) {
			snprintf(tmpBuf, 20, "select%d", i);
			strVal = websGetVar(wp, tmpBuf, T(""));
				
				if ( !gstrcmp(strVal, T("ON")) ) {
					//tone[i/8] = tone[i/8] | (1 << (i%8));
					tone[i/8] = (tone[i/8] << 1) | 1 ;
				}else {
					//tone[i/8] = tone[i/8] | (0 << (i%8));
					tone[i/8] = (tone[i/8] << 1) | 0 ;
				}
		}
//#ifdef APPLY_CHANGE
		// set Tone mask
		adsl_drv_get(RLCM_LOADCARRIERMASK, (void *)tone, GET_LOADCARRIERMASK_SIZE);
//#endif
		goto setOk_tone;	
	}



	if (strMaskAll[0]) {
		for (i=0; i<chan; i++) {
			//tone[i/8] = tone[i/8] | (1 << (i%8));	
			tone[i/8] = (tone[i/8] << 1) | 1 ;
		}
		goto setOk_tone;
	}
	
	
	if (strUnmaskAll[0]) {
		for (i=0; i<chan; i++) {
			//tone[i/8] = tone[i/8] | (0 << (i%8));
			tone[i/8] = (tone[i/8] << 1) | 0 ;
		}
		goto setOk_tone;
	}

setOk_tone:		

	if ( !mib_set(MIB_ADSL_TONE, (void *)tone) ) {							
		strcpy(tmpBuf, T("ERROR: Set ADSL Tone to MIB database failed."));
		goto setErr_tone;
	}

// Kaohj
#if 0
	unsigned char vChar;
#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		/*to avoid the dead when wrriting the flash*/
		itfcfg("sar", 0);
		itfcfg("eth0", 0);
#ifdef WLAN_SUPPORT
		itfcfg("wlan0", 0);
#endif
#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
		mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);

#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		itfcfg("sar", 1);
		 itfcfg("eth0", 1);
#ifdef WLAN_SUPPORT
		vChar=0;
		mib_get( MIB_WLAN_DISABLED, (void *)&vChar);
		if( vChar==0 )	itfcfg("wlan0", 1);
#endif //WLAN_SUPPORT
#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
#endif // of #if 0
	
	submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page

	OK_MSG(submitUrl);

  	return;

setErr_tone:
	ERR_MSG(tmpBuf);
  	
}

void formSetAdsl(webs_t wp, char_t *path, char_t *query)
{
	char_t *submitUrl;
	char_t *strVal;
	char_t olr;
	short mode;
	int xmode;
	
	mib_get(MIB_ADSL_MODE, (void *)&mode);
	mode &= ADSL_MODE_ANXB;
	
	
	strVal = websGetVar(wp, T("glite"), T(""));
	if (strVal[0]=='1')
		mode |= ADSL_MODE_GLITE;
	strVal = websGetVar(wp, T("t1413"), T(""));
	if (strVal[0]=='1')
		mode |= ADSL_MODE_T1413;
	strVal = websGetVar(wp, T("gdmt"), T(""));
	if (strVal[0]=='1')
		mode |= ADSL_MODE_GDMT;
	strVal = websGetVar(wp, T("adsl2"), T(""));
	if (strVal[0]=='1')
		mode |= ADSL_MODE_ADSL2;
	strVal = websGetVar(wp, T("anxl"), T(""));
	if (strVal[0]=='1')
		mode |= ADSL_MODE_ANXL;
	strVal = websGetVar(wp, T("anxm"), T(""));
	if (strVal[0]=='1')
		mode |= ADSL_MODE_ANXM;

        strVal = websGetVar(wp, T("anxb"), T(""));
        if (strVal[0]=='1') mode |= ADSL_MODE_ANXB;
        else mode &= ~ADSL_MODE_ANXB;
                        
    	strVal = websGetVar(wp, T("adsl2p"), T(""));
	if (strVal[0]=='1')
		mode |= ADSL_MODE_ADSL2P;
	
	mib_set(MIB_ADSL_MODE, (void *)&mode);
	
	// OLR type
	olr = 0;
	strVal = websGetVar(wp, T("bswap"), T(""));
	if (strVal[0]=='1')
		olr |= 1;
	
	strVal = websGetVar(wp, T("sra"), T(""));
	if (strVal[0]=='1')
		olr |= 2;
	
	mib_set(MIB_ADSL_OLR, (void *)&olr);

// Kaohj
#if 0
	unsigned char vChar;
#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		/*to avoid the dead when wrriting the flash*/
		itfcfg("sar", 0);
		itfcfg("eth0", 0);
#ifdef WLAN_SUPPORT
		itfcfg("wlan0", 0);
#endif
#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
		mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);

#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		itfcfg("sar", 1);
		 itfcfg("eth0", 1);
#ifdef WLAN_SUPPORT
		vChar=0;
		mib_get( MIB_WLAN_DISABLED, (void *)&vChar);
		if( vChar==0 )	itfcfg("wlan0", 1);
#endif //WLAN_SUPPORT
#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
#endif // of #if 0


	//added by xlyue
//	syslog(LOG_INFO, "ADSL Setting -- ADSL_MODE: 0x%x, ADSL_ORL: 0x%x\n",mode,olr);

	
#ifdef APPLY_CHANGE
	if (mode & ADSL_MODE_ANXL)	// Annex L
		xmode = 3; // Wide-Band & Narrow-Band Mode
	else
		xmode = 0;
	adsl_drv_get(RLCM_SET_ANNEX_L, (void *)&xmode, 4);
	
	if (mode & ADSL_MODE_ANXM)	// Annex M
		xmode = 1;
	else
		xmode = 0;
	adsl_drv_get(RLCM_SET_ANNEX_M, (void *)&xmode, 4);

        if (mode & ADSL_MODE_ANXB)      // Annex B
                xmode = 1;
        else
                xmode = 0;
                
        adsl_drv_get(RLCM_SET_ANNEX_B, (void *)&xmode, 4);
                                                        
    	xmode = (int)olr;
	if (xmode == 2)// SRA (should include bitswap)
		xmode = 3;
	adsl_drv_get(RLCM_SET_OLR_TYPE, (void *)&xmode, 4);
	
	xmode=0;
	if (mode & (ADSL_MODE_GLITE|ADSL_MODE_T1413|ADSL_MODE_GDMT))
		xmode |= 1; // ADSL1
	if (mode & ADSL_MODE_ADSL2)
		xmode |= 2; // ADSL2
	if (mode & ADSL_MODE_ADSL2P)
		xmode |= 4; // ADSL2+
	adsl_drv_get(RLCM_SET_XDSL_MODE, (void *)&xmode, 4);
	
	xmode = mode & (ADSL_MODE_GLITE|ADSL_MODE_T1413|ADSL_MODE_GDMT); //  1: ansi, 2: g.dmt, 8:g.lite
	adsl_drv_get(RLCM_SET_ADSL_MODE, (void *)&xmode, 4);
	
	//adsl_drv_get(RLCM_MODEM_RETRAIN, NULL, 0);
#endif

	submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page

	OK_MSG(submitUrl);

  	return;

}

void formDiagAdsl(webs_t wp, char_t *path, char_t *query)
{
	char_t *str, *submitUrl;
	int mode;

//	str = websGetVar(wp, T("start"), T(""));
	// Display diagnostic messages
	websHeader(wp);
#if  defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	websWrite(wp, T("<META HTTP-EQUIV=Refresh CONTENT=\"180; URL=/adv/adsl-diag-ch.asp?act=1\">\n"));
#else
		websWrite(wp, T("<META HTTP-EQUIV=Refresh CONTENT=\"180; URL=/adv/adsl-diag.asp?act=1\">\n"));
#endif
   	websWrite(wp, T("<body><blockquote><br><br>\n"));
   	websWrite(wp, T("%s<br>\n"), Tadsl_diag_wait);
   	websWrite(wp, T("</blockquote></body>"));
   	websFooter(wp);
	websDone(wp, 200);
	
//	if (str[0]) {
		// start diagnose here
#ifdef _USE_NEW_IOCTL_FOR_DSLDIAG_
		//fprintf( stderr, "use RLCM_ENABLE_DIAGNOSTIC to start dsldiag\n" );
		mode=0;
		adsl_drv_get(RLCM_ENABLE_DIAGNOSTIC, (void *)&mode, sizeof(int));//Lupin
#else
		mode = 41;
		adsl_drv_get(RLCM_DEBUG_MODE, (void *)&mode, sizeof(int));
#endif
		adsl_drv_get(RLCM_MODEM_RETRAIN, NULL, 0);
//	}
	//submitUrl = websGetVar(wp, T("submit-url"), T(""));
	//if (submitUrl[0])
	//	websRedirect(wp, submitUrl);
}

void formStatAdsl(webs_t wp, char_t *path, char_t *query)
{
	char_t *submitUrl;

	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
}

#if 0
int adslDrvSnrTblGraph(int eid, webs_t wp, int argc, char_t **argv)
{
	short tbl[256];
	int len;
	short max, min;

	len = getAdslDrvSnrTbl((void *)tbl);
	if(len > 0) {
		int idx;
		int nBytesSent=0;

		max = Max(tbl, len);
		min = Min(tbl, len);

		nBytesSent += websWrite(wp, T("var test=screen.width; \n"));
		nBytesSent += websWrite(wp, T("var g = new Graph(400,300); \n"));

		nBytesSent += websWrite(wp, T("g.addRow(%d"), tbl[0]);
		for(idx=1;idx<len;idx++) {
			nBytesSent += websWrite(wp, T(",%d"), tbl[idx]);
		}

		nBytesSent += websWrite(wp, T("); \n"));


		if(min != max) {
			nBytesSent += websWrite(wp, T("g.scale = %d; \n"), (int) ((max - min)/4) );
		} else {
			nBytesSent += websWrite(wp, T("g.scale = 1; \n"));
		}

		nBytesSent += websWrite(wp, T("g.title = \"ADSL Channel SNR\"; \n"));
		nBytesSent += websWrite(wp, T("g.xLabel = \"Channel\"; \n"));
		nBytesSent += websWrite(wp, T("g.yLabel = \"SNR\"; \n"));
		nBytesSent += websWrite(wp, T("g.build(); \n"));

		return nBytesSent;
	}

	return 0;
}

int adslDrvSnrTblList(int eid, webs_t wp, int argc, char_t **argv)
{
	short tbl[256];
	int len;

	len = getAdslDrvSnrTbl((void *)tbl);
	if(len > 0) {
		int idx=0;
		int row, rowIdx;
		int nBytesSent=0;

		row = (int) (len - len%10)/10;

		for(rowIdx=0;rowIdx<row; rowIdx++) {
			if((rowIdx % 2) == 1)
				nBytesSent += websWrite(wp, T("<tr bgcolor=\"#EEEEEE\"> \n"));
			else			
				nBytesSent += websWrite(wp, T("<tr bgcolor=\"#DDDDDD\"> \n"));
			
			nBytesSent += websWrite(wp, T("<td width=20%%><font size=3><b>[%03d~%03d]</b></td>\n"), idx, (idx+10));
			nBytesSent += websWrite(wp, T("<td width=20%%><font size=3>%d</td>\n"), tbl[idx]); idx++;
			nBytesSent += websWrite(wp, T("<td width=20%%><font size=3>%d</td>\n"), tbl[idx]); idx++;
			nBytesSent += websWrite(wp, T("<td width=20%%><font size=3>%d</td>\n"), tbl[idx]); idx++;
			nBytesSent += websWrite(wp, T("<td width=20%%><font size=3>%d</td>\n"), tbl[idx]); idx++;
			nBytesSent += websWrite(wp, T("<td width=20%%><font size=3>%d</td>\n"), tbl[idx]); idx++;
			nBytesSent += websWrite(wp, T("<td width=20%%><font size=3>%d</td>\n"), tbl[idx]); idx++;
			nBytesSent += websWrite(wp, T("<td width=20%%><font size=3>%d</td>\n"), tbl[idx]); idx++;
			nBytesSent += websWrite(wp, T("<td width=20%%><font size=3>%d</td>\n"), tbl[idx]); idx++;
			nBytesSent += websWrite(wp, T("<td width=20%%><font size=3>%d</td>\n"), tbl[idx]); idx++;
			nBytesSent += websWrite(wp, T("<td width=20%%><font size=3>%d</td>\n"), tbl[idx]); idx++;
			nBytesSent += websWrite(wp, T("</tr> \n"));
		}

		if((rowIdx % 2) == 1)
			nBytesSent += websWrite(wp, T("<tr bgcolor=\"#EEEEEE\"> \n"));
		else			
			nBytesSent += websWrite(wp, T("<tr bgcolor=\"#DDDDDD\"> \n"));
		nBytesSent += websWrite(wp, T("<td width=20%%><font size=3><b>[%03d~%03d]</b></td>\n"), idx, (len-1));
		for(;idx<len;idx++) {
			nBytesSent += websWrite(wp, T("<td width=20%%><font size=3>%d</td>\n"), tbl[idx]);
		}
		nBytesSent += websWrite(wp, T("</tr> \n"));

		return nBytesSent;
	}

	return 0;
}

int adslDrvBitloadTblGraph(int eid, webs_t wp, int argc, char_t **argv)
{
	short tbl[256];
	int len;
	short max, min;

	len = getAdslDrvBitLoadTbl((void *)tbl);
		
	if(len > 0) {
		int idx;
		int nBytesSent=0;

		max = Max(tbl, len);
		min = Min(tbl, len);

		nBytesSent += websWrite(wp, T("var test=screen.width; \n"));
		nBytesSent += websWrite(wp, T("var g = new Graph(400,300); \n"));

		nBytesSent += websWrite(wp, T("g.addRow(%d"), tbl[0]);
		for(idx=1;idx<len;idx++) {
			nBytesSent += websWrite(wp, T(",%d"), tbl[idx]);
		}

		nBytesSent += websWrite(wp, T("); \n"));

		if(min != max) {
			nBytesSent += websWrite(wp, T("g.scale = %d; \n"), (int) ((max - min)/4) );
		} else {
			nBytesSent += websWrite(wp, T("g.scale = 1; \n"));
		}

		nBytesSent += websWrite(wp, T("g.title = \"ADSL Bit Loading\"; \n"));
		nBytesSent += websWrite(wp, T("g.xLabel = \"Channel\"; \n"));
		nBytesSent += websWrite(wp, T("g.yLabel = \"Bit Loading\"; \n"));
		nBytesSent += websWrite(wp, T("g.build(); \n"));

		return nBytesSent;
	}

	return 0;
}

int adslDrvBitloadTblList(int eid, webs_t wp, int argc, char_t **argv)
{
	short tbl[256];
	int len;

	len = getAdslDrvBitLoadTbl((void *)tbl);
	if(len > 0) {
		int idx=0;
		int row, rowIdx;
		int nBytesSent=0;

		row = (int) (len - len%10)/10;

		for(rowIdx=0;rowIdx<row; rowIdx++) {
			if((rowIdx % 2) == 1)
				nBytesSent += websWrite(wp, T("<tr bgcolor=\"#EEEEEE\"> \n"));
			else			
				nBytesSent += websWrite(wp, T("<tr bgcolor=\"#DDDDDD\"> \n"));
			
			nBytesSent += websWrite(wp, T("<td width=20%%><font size=3><b>[%03d~%03d]</b></td>\n"), idx, (idx+10));
			nBytesSent += websWrite(wp, T("<td width=20%%><font size=3>%d</td>\n"), tbl[idx]); idx++;
			nBytesSent += websWrite(wp, T("<td width=20%%><font size=3>%d</td>\n"), tbl[idx]); idx++;
			nBytesSent += websWrite(wp, T("<td width=20%%><font size=3>%d</td>\n"), tbl[idx]); idx++;
			nBytesSent += websWrite(wp, T("<td width=20%%><font size=3>%d</td>\n"), tbl[idx]); idx++;
			nBytesSent += websWrite(wp, T("<td width=20%%><font size=3>%d</td>\n"), tbl[idx]); idx++;
			nBytesSent += websWrite(wp, T("<td width=20%%><font size=3>%d</td>\n"), tbl[idx]); idx++;
			nBytesSent += websWrite(wp, T("<td width=20%%><font size=3>%d</td>\n"), tbl[idx]); idx++;
			nBytesSent += websWrite(wp, T("<td width=20%%><font size=3>%d</td>\n"), tbl[idx]); idx++;
			nBytesSent += websWrite(wp, T("<td width=20%%><font size=3>%d</td>\n"), tbl[idx]); idx++;
			nBytesSent += websWrite(wp, T("<td width=20%%><font size=3>%d</td>\n"), tbl[idx]); idx++;
			nBytesSent += websWrite(wp, T("</tr> \n"));
		}

		if((rowIdx % 2) == 1)
			nBytesSent += websWrite(wp, T("<tr bgcolor=\"#EEEEEE\"> \n"));
		else			
			nBytesSent += websWrite(wp, T("<tr bgcolor=\"#DDDDDD\"> \n"));
		nBytesSent += websWrite(wp, T("<td width=20%%><font size=3><b>[%03d~%03d]</b></td>\n"), idx, (len-1));
		for(;idx<len;idx++) {
			nBytesSent += websWrite(wp, T("<td width=20%%><font size=3>%d</td>\n"), tbl[idx]);
		}
		nBytesSent += websWrite(wp, T("</tr> \n"));

		return nBytesSent;
	}

	return 0;
}
#endif

int adslToneDiagTbl(int eid, webs_t wp, int argc, char_t **argv)
{
	char mode;
	static int chan=256;
	int nBytesSent=0;
	char str[16], hlin_ds[32], hlin_us[32];
	char latt_ds[16], latt_us[16], satt_ds[16], satt_us[16];
	char snr_ds[16], snr_us[16], attr_ds[16], attr_us[16];
	char txpw_ds[16], txpw_us[16];
	char *act;
	int ldstate;
	
	act = websGetVar(wp, T("act"), T(""));
	if (act && act[0]=='1') {
		adsl_drv_get(RLCM_GET_LD_STATE, (void *)&ldstate, 4);
		if (ldstate != 0)
			nBytesSent += websWrite(wp, T("<tr>\n<b><font color='green'>%s</b></tr>\n"), Tadsl_diag_suc);
		else
			nBytesSent += websWrite(wp, T("<tr>\n<b><font color='red'>%s</b></tr>\n"), Tadsl_diag_fail);
	}
	// get the channel number
	if(adsl_drv_get(RLCM_GET_SHOWTIME_XDSL_MODE, (void *)&mode, 1)) {
		//ramen to clear the first 3 bit
		mode&=0x1F;
		if (mode < 5) //adsl1/adsl2
			chan = 256;
		else
			chan = 512;
	}

	nBytesSent += websWrite(wp, T("<tr bgcolor=#f0f0f0>\n<th align=left bgcolor=#c0c0c0 width=\"120\"></th>\n"));
	nBytesSent += websWrite(wp, T("<th width=\"100\"><font size=2>%s</th><th width=\"100\"><font size=2>%s</th>\n</tr>\n"), Tdownstream, Tupstream);

	#if 0
	nBytesSent += websWrite(wp, T("<tr bgcolor=#f0f0f0>\n<th align=left bgcolor=#c0c0c0>Number of SubCarrier</th>\n"));
	nBytesSent += websWrite(wp, T("<td align=center>%d</td>\n"), chan);
	nBytesSent += websWrite(wp, T("<td align=center>%d</td>\n</tr>\n"), chan);
	#endif

	//hlog = malloc(sizeof(short)*(chan*3+HLOG_ADDITIONAL_SIZE));
	hlog = malloc(sizeof(short)*(MAX_DSL_TONE*3+HLOG_ADDITIONAL_SIZE));
	//if (adsl_drv_get(RLCM_GET_DIAG_HLOG, (void *)hlog, sizeof(short)*(chan*3+HLOG_ADDITIONAL_SIZE))) {
	if (adsl_drv_get(RLCM_GET_DIAG_HLOG, (void *)hlog, sizeof(short)*(MAX_DSL_TONE*3+HLOG_ADDITIONAL_SIZE))) {
		// Hlinear scale
		snprintf(hlin_ds, 16, "%d", (unsigned short)hlog[chan*3+1]);
		snprintf(hlin_us, 16, "%d", (unsigned short)hlog[chan*3]);
		// loop Attenuation
		snprintf(latt_ds, 16, "%d.%d", hlog[chan*3+3]/10, abs(hlog[chan*3+3]%10));
		snprintf(latt_us, 16, "%d.%d", hlog[chan*3+2]/10, abs(hlog[chan*3+2]%10));
		// signal Attenuation
		snprintf(satt_ds, 16, "%d.%d", hlog[chan*3+5]/10, abs(hlog[chan*3+5]%10));
		snprintf(satt_us, 16, "%d.%d", hlog[chan*3+4]/10, abs(hlog[chan*3+4]%10));
		// SNR Margin
		snprintf(snr_ds, 16, "%d.%d", hlog[chan*3+7]/10, abs(hlog[chan*3+7]%10));
		snprintf(snr_us, 16, "%d.%d", hlog[chan*3+6]/10, abs(hlog[chan*3+6]%10));
		// Attainable Rate
		snprintf(attr_ds, 16, "%d", hlog[chan*3+9]);
		snprintf(attr_us, 16, "%d", hlog[chan*3+8]);
		// tx power
		snprintf(txpw_ds, 16, "%d.%d", hlog[chan*3+11]/10, abs(hlog[chan*3+11]%10));
		snprintf(txpw_us, 16, "%d.%d", hlog[chan*3+10]/10, abs(hlog[chan*3+10]%10));
	}
	else {
		hlin_ds[0] = hlin_us[0] = latt_ds[0] = latt_us[0] = 0;
		satt_ds[0] = satt_us[0] = snr_ds[0] = snr_us[0] = 0;
		attr_ds[0] = attr_us[0] = txpw_ds[0] = txpw_us[0] = 0;
	}
		
	nBytesSent += websWrite(wp, T("<tr bgcolor=#f0f0f0>\n<th align=left bgcolor=#c0c0c0><font size=2>%s</th>\n"), Thlin_scale);
	nBytesSent += websWrite(wp, T("<td align=center><font size=2>%s</font></td>\n"), hlin_ds);
	nBytesSent += websWrite(wp, T("<td align=center><font size=2>%s</font></td>\n</tr>\n"), hlin_us);
	
	nBytesSent += websWrite(wp, T("<tr bgcolor=#f0f0f0>\n<th align=left bgcolor=#c0c0c0><font size=2>%s(dB)</th>\n"), Tloop_annu);
	nBytesSent += websWrite(wp, T("<td align=center><font size=2>%s</font></td>\n"), latt_ds);
	nBytesSent += websWrite(wp, T("<td align=center><font size=2>%s</font></td>\n</tr>\n"), latt_us);
	
	nBytesSent += websWrite(wp, T("<tr bgcolor=#f0f0f0>\n<th align=left bgcolor=#c0c0c0><font size=2>%s(dB)</th>\n"), Tsig_annu);
	nBytesSent += websWrite(wp, T("<td align=center><font size=2>%s</font></td>\n"), satt_ds);
	nBytesSent += websWrite(wp, T("<td align=center><font size=2>%s</font></td>\n</tr>\n"), satt_us);
	
	nBytesSent += websWrite(wp, T("<tr bgcolor=#f0f0f0>\n<th align=left bgcolor=#c0c0c0><font size=2>%s(dB)</th>\n"), Tsnr_marg);
	nBytesSent += websWrite(wp, T("<td align=center><font size=2>%s</font></td>\n"), snr_ds);
	nBytesSent += websWrite(wp, T("<td align=center><font size=2>%s</font></td>\n</tr>\n"), snr_us);
	
	nBytesSent += websWrite(wp, T("<tr bgcolor=#f0f0f0>\n<th align=left bgcolor=#c0c0c0><font size=2>%s(Kbps)</th>\n"), Tattain_rate);
	nBytesSent += websWrite(wp, T("<td align=center><font size=2>%s</font></td>\n"), attr_ds);
	nBytesSent += websWrite(wp, T("<td align=center><font size=2>%s</font></td>\n</tr>\n"), attr_us);
	
	nBytesSent += websWrite(wp, T("<tr bgcolor=#f0f0f0>\n<th align=left bgcolor=#c0c0c0><font size=2>%s(dBm)</th>\n"), Tout_power);
	nBytesSent += websWrite(wp, T("<td align=center><font size=2>%s</font></td>\n"), txpw_ds);
	nBytesSent += websWrite(wp, T("<td align=center><font size=2>%s</font></td>\n</tr>\n"), txpw_us);
	free(hlog);
	
	return nBytesSent;
}
	
int adslToneDiagList(int eid, webs_t wp, int argc, char_t **argv)
{
	char mode;
	static int chan=256;
	int i;
	int nBytesSent=0;
	int ival, intp, fp;
	char str[16];
	
	// get the channel number
	if(adsl_drv_get(RLCM_GET_SHOWTIME_XDSL_MODE, (void *)&mode, 1)) {
		//ramen to clear the first 3 bit.
		mode&=0x1f;
		if (mode < 5) //adsl1/adsl2
			chan = 256;
		else
			chan = 512;
	}
	
	intp=fp=0;
	str[0] = 0;
	
	/*
	snr = malloc(sizeof(short)*chan);
	qln = malloc(sizeof(short)*chan);
	hlog = malloc(sizeof(short)*(chan*3+HLOG_ADDITIONAL_SIZE));
	*/
	snr = malloc(sizeof(short)*MAX_DSL_TONE);
	qln = malloc(sizeof(short)*MAX_DSL_TONE);
	hlog = malloc(sizeof(short)*(MAX_DSL_TONE*3+HLOG_ADDITIONAL_SIZE));
	
	/*
	if (adsl_drv_get(RLCM_GET_DIAG_HLOG, (void *)hlog, sizeof(short)*(chan*3+HLOG_ADDITIONAL_SIZE))) {
		adsl_drv_get(RLCM_GET_DIAG_SNR, (void *)snr, sizeof(short)*chan);
		adsl_drv_get(RLCM_GET_DIAG_QLN, (void *)qln, sizeof(short)*chan);
		diagflag = 1;
	}
	*/
	if (adsl_drv_get(RLCM_GET_DIAG_HLOG, (void *)hlog, sizeof(short)*(MAX_DSL_TONE*3+HLOG_ADDITIONAL_SIZE))) {
		adsl_drv_get(RLCM_GET_DIAG_SNR, (void *)snr, sizeof(short)*MAX_DSL_TONE);
		adsl_drv_get(RLCM_GET_DIAG_QLN, (void *)qln, sizeof(short)*MAX_DSL_TONE);
		diagflag = 1;
	}
	else
		diagflag = 0;
	
	nBytesSent += websWrite(wp, T("<tr>\n<th width=15%% bgcolor=#c0c0c0><font size=2>%s</font></th>\n"), Ttone_num);
	nBytesSent += websWrite(wp, T("<th width=15%% bgcolor=#c0c0c0><font size=2>H.Real</font></th>\n"));
	nBytesSent += websWrite(wp, T("<th width=15%% bgcolor=#c0c0c0><font size=2>H.Image</font></th>\n"));
	nBytesSent += websWrite(wp, T("<th width=15%% bgcolor=#c0c0c0><font size=2>SNR</font></th>\n"));
	nBytesSent += websWrite(wp, T("<th width=15%% bgcolor=#c0c0c0><font size=2>QLN</font></th>\n"));
	nBytesSent += websWrite(wp, T("<th width=15%% bgcolor=#c0c0c0><font size=2>Hlog</font></th>\n</tr>\n"));
	
	for (i = 0; i < chan; i++) {
		
		nBytesSent += websWrite(wp, T("<tr>\n<th bgcolor=#c0c0c0><font size=2>%d</font></th>\n"), i);
		// H.Real = H.Real*Hlin.Scale/32768
		if (diagflag) {
			/*
			if (i <= 31) { // upstream
				fp=(hlog[i+chan]*hlog[chan*3]*100)/32768;
			}
			else { // downstream
				fp=(hlog[i+chan]*hlog[chan*3+1]*100)/32768;
			}
			*/
			intp = hlog[i+chan]/1000;
			fp = abs(hlog[i+chan]%1000);
			if (fp<0) {
				fp = -fp;
				if (intp == 0)
					snprintf(str, 16, "-0.%03d", fp);
				else
					snprintf(str, 16, "%d.%03d", intp, fp);
			}
			else
				snprintf(str, 16, "%d.%03d", intp, fp);
		}
		
		nBytesSent += websWrite(wp, T("<td align=center bgcolor=#f0f0f0><font size=2>%s</font></td>\n"), str);
		// H.Image = H.Image*Hlin.Scale/32768
		if (diagflag) {
			/*
			if (i <= 31) { // upstream
				fp=(hlog[i+chan*2]*hlog[chan*3]*100)/32768;
			}
			else { // downstream
				fp=(hlog[i+chan*2]*hlog[chan*3+1]*100)/32768;
			}
			*/
			intp = hlog[i+chan*2]/1000;
			fp = abs(hlog[i+chan*2]%1000);
			if (fp<0) {
				fp = -fp;
				if (intp == 0)
					snprintf(str, 16, "-0.%03d", fp);
				else
					snprintf(str, 16, "%d.%03d", intp, fp);
			}
			else
				snprintf(str, 16, "%d.%03d", intp, fp);
		}
		
		nBytesSent += websWrite(wp, T("<td align=center bgcolor=#f0f0f0><font size=2>%s</font></td>\n"), str);
		//nBytesSent += websWrite(wp, T("<td align=center bgcolor=#f0f0f0>%d</td>\n"), 0);
		// snr = -32+(snr/2)
		if (diagflag) {
			/*
			ival = snr[i] * 5; // *10/2
			intp = ival/10-32;
			fp = ival % 10;
			*/
			intp = snr[i]/10;
			fp = abs(snr[i]%10);
			snprintf(str, 16, "%d.%d", intp, fp);
		}
		nBytesSent += websWrite(wp, T("<td align=center bgcolor=#f0f0f0><font size=2>%s</font></td>\n"), str);
		// qln = -23-(qln/2)
		if (diagflag) {
			/*
			ival = qln[i] * 5;
			intp = ival/10+23;
			fp = ival % 10;
			*/
			intp = qln[i]/10;
			fp = abs(qln[i]%10);
			if (fp<0) {
				if (intp != 0)
					snprintf(str, 16, "%d.%d", intp, -fp);
				else
					snprintf(str, 16, "-%d.%d", intp, -fp);
			}
			else
				snprintf(str, 16, "%d.%d", intp, fp);
		}
		nBytesSent += websWrite(wp, T("<td align=center bgcolor=#f0f0f0><font size=2>%s</font></td>\n"), str);
		// hlog = 6-(hlog/10)
		if (diagflag) {
			/*
			ival = hlog[i]/10;
			if (ival >= 6) {// negative value
				intp = ival - 6;
				fp = hlog[i] % 10;
				nBytesSent += websWrite(wp, T("<td align=center bgcolor=#f0f0f0>-%d.%d</td>\n</tr>\n"), intp, fp);
			}
			else { //positive value
				intp = 6- ival;
				ival = hlog[i] % 10;
				if (ival != 0)
					intp--;
				fp = 10 - ival;
				nBytesSent += websWrite(wp, T("<td align=center bgcolor=#f0f0f0>%d.%d</td>\n</tr>\n"), intp, fp);
			}
			*/
			intp = hlog[i]/10;
			fp = abs(hlog[i]%10);
			if (fp<0) {
				if (intp != 0)
					snprintf(str, 16, "%d.%d", intp, -fp);
				else
					snprintf(str, 16, "-%d.%d", intp, -fp);
			}
			else
				snprintf(str, 16, "%d.%d", intp, fp);
		}
		/*
		else
			nBytesSent += websWrite(wp, T("<td align=center bgcolor=#f0f0f0>%d.%d</td>\n</tr>\n"), intp, fp);
		*/
		nBytesSent += websWrite(wp, T("<td align=center bgcolor=#f0f0f0><font size=2>%s</font></td>\n</tr>\n"), str);
	}
	
	free(snr);
	free(qln);
	free(hlog);
	
	return nBytesSent;
}


int adslToneConfDiagList(int eid, webs_t wp, int argc, char_t **argv)
{
	short mode;
	int i, chan;
	int nBytesSent=0;
	unsigned char tone[64];    // Added by Mason Yu for correct Tone Mib Type
	int onbit;
	
	
	memset(tone, 0, sizeof(tone));
	
	// get the channel number
	mib_get(MIB_ADSL_MODE, (void *)&mode);
	if (mode & ADSL_MODE_ADSL2P)
		chan = 512;	// ADSL2+
	else
		chan = 256;	// ADSL, ADSL2

	if ( !mib_get(MIB_ADSL_TONE, (void *)tone)) {
		printf(strGetToneerror);		
	}

#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	nBytesSent += websWrite(wp, T("<tr>\n<th width=25%% bgcolor=#808080><font size=\"2\">载波编号</font></th>\n"));
	nBytesSent += websWrite(wp, T("<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>选择</b></font></td></tr>\n"));
#else
	nBytesSent += websWrite(wp, T("<tr>\n<th width=25%% bgcolor=#c0c0c0>Tone Number</th>\n"));
	nBytesSent += websWrite(wp, T("<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n"));
#endif
	
	
	
	for (i = 0; i < chan; i++) {
		//onbit =(tone[i/8] >> (i%8) ) & 0x01;
		onbit =(tone[i/8] >> (7-(i%8)) ) & 0x01;	
		
		nBytesSent += websWrite(wp, T("<tr>\n<th bgcolor=#c0c0c0>%d</th>\n"), i);			
		if (onbit == 1)
			nBytesSent += websWrite(wp, T("<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\" checked></td></tr>\n"), i);		
		else 
			nBytesSent += websWrite(wp, T("<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n"), i);		
		
	}
	
	return nBytesSent;
}


//cathy
void DSLuptime(int eid, webs_t wp, int argc, char_t **argv)
{
	Modem_LinkSpeed vLs;
	unsigned char adslflag;
	struct sysinfo info, *up;
	int updays, uphours, upminutes, upsec;
	
	// check for xDSL link
	if (!adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE) || vLs.upstreamRate == 0)
		adslflag = 0;
	else
		adslflag = 1;

	if( adslflag ) {
		up = updateLinkTime(0);
		sysinfo(&info);
		info.uptime = info.uptime - up->uptime;
		updays = (int) info.uptime / (60*60*24);
		if (updays)
			websWrite (wp, T ("%d day%s,&nbsp;"), updays, (updays != 1) ? "s" : "");
		upsec = (int) info.uptime % 60;
		upminutes = (int) info.uptime / 60;
		uphours = (upminutes / 60) % 24;
		upminutes %= 60;
		websWrite (wp, T ("%02d:%02d:%02d </td></tr>\n"), uphours, upminutes, upsec);
	}
	else {
		websWrite(wp, T("&nbsp;"));
	}
}

              

/////////////////////////////////////////////////////////////////////////////
void formStats(webs_t wp, char_t *path, char_t *query)
{
	char_t *strValue, *submitUrl;

	strValue = websGetVar(wp, T("reset"), T(""));	//cathy, reset stats
	if(strValue[0]) {
		if(strValue[0] - '0') {	//reset
#ifdef EMBED	
			int skfd;
			struct ifreq ifr;
			if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
				perror("socket error");
			}
			else {
				if (ioctl(skfd, SIOCRESETSTAT, &ifr) < 0) {
					printf("ioctl SIOCRESETSTAT error\n");
				}
				close(skfd);
			}
#endif
		}
	}


	submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page

	if (submitUrl[0])
		websRedirect(wp, submitUrl);
}

static char *get_name(char *name, char *p)
{
    while (isspace(*p))
	p++;
    while (*p) {
	if (isspace(*p))
	    break;
	if (*p == ':') {	/* could be an alias */
	    char *dot = p, *dotname = name;
	    *name++ = *p++;
	    while (isdigit(*p))
		*name++ = *p++;
	    if (*p != ':') {	/* it wasn't, backup */
		p = dot;
		name = dotname;
	    }
	    if (*p == '\0')
		return NULL;
	    p++;
	    break;
	}
	*name++ = *p++;
    }
    *name++ = '\0';
    return p;
}

// Dump the Memory usage
int memDump(int eid, webs_t wp, int argc, char_t **argv)
{
	struct sysinfo info;
	
	sysinfo(&info);
	info.mem_unit*=1024;
	info.totalram/=info.mem_unit;
	info.freeram/=info.mem_unit;
	
	websWrite(wp, T("<font size=2><b>Memory Usage:\n"));
	websWrite(wp, T("<br>Total: %13ld kB&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Free: %13ld kB\n"),
		info.totalram, info.freeram);
	return 0;
}
	
// List the packet statistics for all interfaces at web page.
int pktStatsList(int eid, webs_t wp, int argc, char_t **argv)
{
	FILE *fh;
#if defined(CTC_WAN_NAME)
	//ql_xu add: for auto generate wan name
	FILE *fp;
	int entryNum;
	int mibcnt;
	MIB_CE_ATM_VC_T Entry;
#endif
#ifdef EMBED
	int skfd, i;
	struct atmif_sioc mysio;
	struct SAR_IOCTL_CFG cfg;
	struct ch_stat stat;
#endif
	char buf[512];
	unsigned long rx_pkt, rx_err, rx_drop, tx_pkt, tx_err, tx_drop;
	
#ifdef CTC_WAN_NAME
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
#endif
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	websWrite(wp, T("<tr><font size=1>"
	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=2>端口名称</td>\n"
	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=2>接收包数</td>\n"
	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=2>接收错包数</td>\n"
	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=2>接收丢包数</td>\n"
	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=2>发送包数</td>\n"
	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=2>发送错包数</td>\n"
	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=2>发送丢包数</td></font></tr>\n"));
#else
websWrite(wp, T("<tr><font size=1>"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">Interface</td>\n"
	"<td align=center width=\"15%%\" bgcolor=\"#808080\">Rx pkt</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">Rx err</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">Rx drop</td>\n"
	"<td align=center width=\"15%%\" bgcolor=\"#808080\">Tx pkt</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">Tx err</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">Tx drop</td></font></tr>\n"));
#endif
	// Ethernet statistics
	fh = fopen(_PATH_PROCNET_DEV, "r");
	if (!fh) {
		fprintf(stderr, "Warning: cannot open %s (%s).\n",
			_PATH_PROCNET_DEV, strerror(errno));
		return;
	}
	fgets(buf, sizeof buf, fh);	/* eat line */
	fgets(buf, sizeof buf, fh);
	while (fgets(buf, sizeof buf, fh)) {
		char *s, name[IFNAMSIZ];
		int islan;
		s = get_name(name, buf);    
		islan=0;
		if (!strcmp("eth0", name)) {
			sscanf(s,
			"%*lu %lu %lu %lu %*lu %*lu %*lu %*lu %*lu %lu %lu %lu %*lu %*lu %*lu %*lu",
			&rx_pkt, &rx_err, &rx_drop, &tx_pkt, &tx_err, &tx_drop);
			islan=1;
			websWrite(wp, T("<tr><td align=center width=\"8%%\" "
				"bgcolor=\"#C0C0C0\"><font size=\"2\"><b>eth0</b></font></td>\n"));
		}
#ifdef CONFIG_USB_ETH
		if (!strcmp(USBETHIF, name)) {
			sscanf(s,
			"%*lu %lu %lu %lu %*lu %*lu %*lu %*lu %*lu %lu %lu %lu %*lu %*lu %*lu %*lu",
			&rx_pkt, &rx_err, &rx_drop, &tx_pkt, &tx_err, &tx_drop);
			islan=1;
			websWrite(wp, T("<tr><td align=center width=\"8%%\" "
				"bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"), USBETHIF );
		}
#endif //CONFIG_USB_ETH
#ifdef WLAN_SUPPORT
		else if (!strcmp("wlan0", name)) {
			if (wlan_is_up()) {
				sscanf(s,
				"%*lu %lu %lu %lu %*lu %*lu %*lu %*lu %*lu %lu %lu %lu %*lu %*lu %*lu %*lu",
				&rx_pkt, &rx_err, &rx_drop, &tx_pkt, &tx_err, &tx_drop);
				islan=1;
				websWrite(wp, T("<tr><td align=center width=\"8%%\" "
					"bgcolor=\"#C0C0C0\"><font size=\"2\"><b>wlan0</b></font></td>\n"));
			};
		}
#endif
		if (islan)
			websWrite(wp, T(
			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td></tr>\n"),
			rx_pkt, rx_err, rx_drop, tx_pkt, tx_err, tx_drop);
	}
	
	fclose(fh);
	
#ifdef EMBED
	// pvc statistics
	if((skfd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0){
		perror("socket open error");
		return;
	}
	
	mysio.number = 0;

	for (i=0; i < MAX_VC_NUM; i++)
	{
		
		cfg.ch_no = i;
		mysio.arg = (void *)&cfg;
		if(ioctl(skfd, ATM_SAR_GETSTAT, &mysio)<0)
		{
			(void)close(skfd);
			return;
		}

		if (cfg.created == 0)
			continue;	

#ifdef CTC_WAN_NAME
		//check mib chain to see the channel mode of the current channel.
		int found=0;
		for(mibcnt=0; mibcnt<entryNum; mibcnt++)
		{
			if (!mib_chain_get(MIB_ATM_VC_TBL, mibcnt, (void *)&Entry))
			{
				printf("get MIB chain error\n");
				continue;
			}
			
			if(Entry.enable==0)
				continue;
			
			if(Entry.vpi==cfg.vpi && Entry.vci==cfg.vci)
			{
				//found entry in mibs, get channel mode
				cfg.rfc = Entry.cmode;
				{
					char applicationname[30];
					char username[MAX_NAME_LEN];
					char password[MAX_NAME_LEN];
					char interfacename[MAX_NAME_LEN];
					char pppIdx=0;
#ifdef _CWMP_MIB_ 
					char setname[30];
					memset(setname, 0, 30);
#endif
					memset(applicationname, 0, sizeof(applicationname));
					memset(interfacename, 0, sizeof(interfacename));
					if(cfg.rfc == ADSL_BR1483)
					{
						//bridge mode
#ifdef _CWMP_MIB_ 
						if(*(Entry.WanName))
							strcpy(setname, Entry.WanName);
#endif
						setWanName(applicationname, Entry.applicationtype);
						strcat(applicationname, "B_");
					} 
					else { 
						if(cfg.rfc == ADSL_PPPoE || cfg.rfc == ADSL_PPPoA )
						{
#ifdef _CWMP_MIB_ 
							if(*(Entry.WanName))
								strcpy(setname, Entry.WanName);
#endif
							snprintf(interfacename, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));
							fh = fopen(_PATH_PROCNET_DEV, "r");
							if (!fh) {
										fprintf(stderr, "Warning: cannot open %s (%s).\n",
											_PATH_PROCNET_DEV, strerror(errno));
										return;
							}
							fgets(buf, sizeof buf, fh);	/* eat line */
							fgets(buf, sizeof buf, fh);
							while (fgets(buf, sizeof buf, fh)) {
										char *s, ifName[IFNAMSIZ];
										char name[30];
										
										s = get_name(name, buf);    
										if (!strcmp(interfacename, name)) {
											sscanf(s,
												"%*lu %lu %lu %lu %*lu %*lu %*lu %*lu %*lu %lu %lu %lu %*lu %*lu %*lu %*lu",
												&rx_pkt, &rx_err, &rx_drop, &tx_pkt, &tx_err, &tx_drop);

											//display
											memset(ifName, 0, 30);
											getWanName(&Entry,ifName);
#ifdef _CWMP_MIB_
											if(*setname)
												strcpy(ifName, setname);
#endif
											websWrite(wp, T("<tr>"
												"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
												"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
												"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
												"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
												"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
												"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
												"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td></tr>\n"),
												ifName, rx_pkt, rx_err, rx_drop, tx_pkt, tx_err, tx_drop);
										}
							}		
							fclose(fh);
							found++;
							continue;
						} else {	//rt1483 rt1577
#ifdef _CWMP_MIB_ 
							if(*(Entry.WanName))
								strcpy(setname, Entry.WanName);
#endif
							setWanName(applicationname, Entry.applicationtype);
						}
						strcat(applicationname, "R_");
					}
					sprintf(applicationname, "%s%d_%d", applicationname, cfg.vpi, cfg.vci);
#ifdef _CWMP_MIB_ 
					if(*setname)
						strcpy(applicationname, setname);
#endif
				
					websWrite(wp, T("<tr>"
						"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
						"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
						"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
						"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
						"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
						"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
						"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td></tr>\n"),
						applicationname, cfg.stat.rx_pkt_cnt, cfg.stat.rx_pkt_fail,
						cfg.stat.rx_crc_error, cfg.stat.tx_desc_ok_cnt,
						cfg.stat.tx_pkt_fail_cnt, cfg.stat.send_desc_lack);
					found++;
					break;
				}
			}
		}
		if(found==0)
		{
			printf("in pktStatsList: not found mib entry for this interface\n");
			//set to default application name....
		}
		
		
#else
		websWrite(wp, T("<tr>"
		"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%u_%u</b></font></td>\n"
		"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
		"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
		"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
		"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
		"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
		"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td></tr>\n"),
		cfg.vpi, cfg.vci, cfg.stat.rx_pkt_cnt, cfg.stat.rx_pkt_fail,
		cfg.stat.rx_crc_error, cfg.stat.tx_desc_ok_cnt,
		cfg.stat.tx_pkt_fail_cnt, cfg.stat.send_desc_lack);
#endif
	}
	(void)close(skfd);
#endif
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
void formRconfig(webs_t wp, char_t *path, char_t *query)
{
	char_t *strValue, *strSubmit, *uid, *upwd;
	char tmpBuf[100];
	char ipaddr[20], dport[10];
	char userName[MAX_NAME_LEN];
	
	strSubmit = websGetVar(wp, T("yes"), T(""));
	mib_get(MIB_ADSL_LAN_IP, (void *)tmpBuf);
	strncpy(ipaddr, inet_ntoa(*((struct in_addr *)tmpBuf)), 16);
	ipaddr[15] = '\0';
	snprintf(ipaddr, 20, "%s:80", ipaddr);
	
	// enable
	if (strSubmit[0]) {
		mib_get( MIB_USER_NAME, (void *)userName );
		if (userName[0] == '\0') {
			strcpy(tmpBuf, T("PROHIBIT: Administrator's password not set!"));
			goto setErr_rconf;
		}
			
		strValue = websGetVar(wp, T("writable"), T(""));
		if (strValue[0])	// allow update
			g_remoteUpdate = TRUE;
		else	// read only
			g_remoteUpdate = FALSE;
		
		strValue = websGetVar(wp, T("portFlag"), T(""));
		if (strValue[0]) {	// default port 51003
			g_remoteAccessPort = 51003;
		}
		else {	// use randomly selected port
			if (!srandomCalled) {
				srandom(time(0));
				srandomCalled = 1;
			}
			g_remoteAccessPort = 50000 + (random()&0x00000fff);
		}
		
		sprintf(dport, "%d", g_remoteAccessPort);
		
		uid = websGetVar(wp, T("uid"), T(""));
		upwd = websGetVar(wp, T("pwd"), T(""));
		if (uid[0] != '\0' && upwd[0] != '\0') {
			/* Create user */
			if ( umAddUser(uid, upwd, DEFAULT_GROUP, FALSE, FALSE) ) {
				error(E_L, E_LOG, T("ERROR: Unable to add user account."));
				return;
			}
			else {
				strcpy(g_rUserName, uid);
				strcpy(g_rUserPass, upwd);
				// The remote access session MUST be started
				// within REMOTE_PASS_LIFE seconds.
				g_rexpire = time(0) + REMOTE_PASS_LIFE;
				g_rSessionStart = FALSE;
			}
		}
		else {
			g_rUserName[0] = '\0';
			g_rUserPass[0] = '\0';
		}
			
		// iptables -D INPUT -j block
		va_cmd(IPTABLES, 4, 1, (char *)FW_DEL, (char *)FW_INPUT, "-j", "block");
		// iptables -A INPUT -i ! br0 -p TCP --dport 80 -j ACCEPT
		va_cmd(IPTABLES, 11, 1, (char *)FW_ADD, (char *)FW_INPUT, ARG_I,
		"!", LANIF, "-p", ARG_TCP, FW_DPORT, "80", "-j", (char *)FW_ACCEPT);
		// iptables -A INPUT -j block
		va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_INPUT, "-j", "block");
		
		// iptables -t nat -A PREROUTING -i ! $LAN_IF -p TCP --dport 51003 -j DNAT --to-destination ipaddr:80
		va_cmd(IPTABLES, 15, 1, "-t", "nat",
					(char *)FW_ADD,	"PREROUTING",
					(char *)ARG_I, "!", (char *)LANIF,
					"-p", (char *)ARG_TCP,
					(char *)FW_DPORT, dport, "-j",
					"DNAT", "--to-destination", ipaddr);
		g_remoteConfig = 1;
	}
	
	strSubmit = websGetVar(wp, T("no"), T(""));
	// disable
	if (strSubmit[0]) {
		sprintf(dport, "%d", g_remoteAccessPort);
		
		// delete original user
		if (g_rUserName[0]) {
			if ( umDeleteUser(g_rUserName) ) {
				printf("ERROR: Unable to delete user account (user=%s).\n", g_rUserName);
				return;
			}
			g_rUserName[0] = '\0';
		}
		
		// iptables -D INPUT -i ! br0 -p TCP --dport 80 -j ACCEPT
		va_cmd(IPTABLES, 11, 1, (char *)FW_DEL, (char *)FW_INPUT, ARG_I,
		"!", LANIF, "-p", ARG_TCP, FW_DPORT, "80", "-j", (char *)FW_ACCEPT);
		// iptables -t nat -D PREROUTING -i ! $LAN_IF -p TCP --dport 51003 -j DNAT --to-destination ipaddr:80
		va_cmd(IPTABLES, 15, 1, "-t", "nat",
					(char *)FW_DEL,	"PREROUTING",
					(char *)ARG_I, "!", (char *)LANIF,
					"-p", (char *)ARG_TCP,
					(char *)FW_DPORT, dport, "-j",
					"DNAT", "--to-destination", ipaddr);
		g_remoteConfig = 0;
		g_remoteUpdate = FALSE;
	}
	
	strSubmit = websGetVar(wp, T("submit-url"), T(""));   // hidden page
	websRedirect(wp, strSubmit);
	return;
	
setErr_rconf:
	ERR_MSG(tmpBuf);
}

//for rc4 encryption
/****************************************/
/* rc4_encrypt()                        */
/****************************************/
#define RC4_INT unsigned int
#define MAX_MESSAGE_LENGTH 2048
unsigned char en_cipherstream[MAX_MESSAGE_LENGTH+1];
void xor_block(int length, unsigned char *a, unsigned char *b, unsigned char *out)
{
    int i;
    for (i=0;i<length; i++)
    {
        out[i] = a[i] ^ b[i];
    }
}
static __inline__ void swap(unsigned char *a, unsigned char *b)
{
    unsigned char tmp;

    tmp = *a;
    *a = *b;
    *b = tmp;
}
void rc4(
            unsigned char *key,
            int key_length,
            int cipherstream_length,
            unsigned char *cipherstream)
{
    int i, j, x;

    unsigned char s[256];
    unsigned char k[256];

    /* Create Key Stream */
    for (i=0; i<256; i++)
        k[i] = key[i % key_length];
    
    /* Initialize SBOX */
    for (j=0; j<256; j++)
        s[j] = j;
    
    /* Seed the SBOX */
    i = 0;
    for (j=0; j<256; j++)
    {
        i = (i + s[j] + k[j]) & 255;
        swap(&s[j], &s[i]);
    }

    /* Generate the cipherstream */
    j = 0;
    i = 0;

    for (x=0; x<cipherstream_length; x++)
    {
        j = (j + 1) & 255;
        i = (i + s[j]) & 255;
        swap(&s[j], &s[i]);
        cipherstream[x] = s[(s[j] + s[i]) & 255];
    };
}

void rc4_encrypt(
            unsigned char *key,
            int key_length,
            unsigned char *data,
            int data_length,
            unsigned char *ciphertext)
{

    rc4(key, key_length, data_length, en_cipherstream);
    xor_block(data_length, en_cipherstream, data, ciphertext);
}

int rc4_encry()
{
	// for rc4 ecncryption
	FILE *fp_in, *fp_out;
	char ciphertext[256];
	unsigned char rc4_key[256];
	int key_length;
	char LINE[256];
	int data_len;
	
	rc4_key[0] = 0;
	strcpy(rc4_key, "dkskpplwejgstyujfglkgjiruyo09trpvs32");
	key_length = strlen(rc4_key);
	rc4_key[key_length] = '\0';

	fp_in = fopen("/var/log/messages", "r");
	fp_out = fopen("/tmp/log.txt", "w");
	while (!feof(fp_in)) {
		data_len = fread(LINE, sizeof(char), 255, fp_in);
		LINE[data_len] = 0;
		//encryption
		rc4_encrypt(rc4_key, key_length, LINE, data_len, ciphertext);
		ciphertext[data_len] = 0;
		fwrite(ciphertext, sizeof(char), data_len, fp_out);		
	}
	fclose(fp_out);
	fclose(fp_in);

	return 0;
}


#define _PATH_SYSCMD_LOG "/tmp/syscmd.log"
unsigned char first_time=1;
unsigned char dbg_enable=0;
void formSysCmd(webs_t wp, char_t *path, char_t *query)
{
	char_t  *submitUrl, *sysCmd,*strVal,*strRequest;
	unsigned char adsldbg;
	unsigned int maxFileSector;
//#ifndef NO_ACTION
	char_t tmpBuf[100];
//#endif

	if(first_time==1){
		mib_get(MIB_ADSL_DEBUG, (void *)&adsldbg);
		if(adsldbg==1)
			dbg_enable=1;
		first_time=0;
	}
	submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page
	sysCmd = websGetVar(wp, T("sysCmd"), T(""));   // hidden page

	strRequest = websGetVar(wp, T("save"), T(""));
	if (strRequest[0])
	{
		PARAM_HEADER_T header;
		unsigned char *ptr;
		unsigned int fileSize,filelen;
		unsigned int fileSector;

		wp->buffer_end=0; // clear header
		FILE *fp;
		//create config file
		rc4_encry();
		websWrite(wp, "HTTP/1.0 200 OK\n");
		websWrite(wp, "Content-Type: application/octet-stream;\n");

		websWrite(wp, "Content-Disposition: attachment;filename=\"log\" \n");

		
		websWrite(wp, "Pragma: no-cache\n");
		websWrite(wp, "Cache-Control: no-cache\n");
		websWrite(wp, "\n");

		fp=fopen("/tmp/log.txt","r");
		//decide the file size	
		fseek(fp, 0, SEEK_END);
		filelen = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		fileSize=filelen;

		while(fileSize>0)
		{
			char buf[0x1000];
			int nRead;
			maxFileSector=0x500;
//			printf("write %d %d %08x\n",maxFileSector, fileSize, (unsigned int )ptr);
			fileSector = (fileSize>maxFileSector)?maxFileSector:fileSize;
			nRead = fread((void *)buf, 1, fileSector, fp);
			buf[nRead]=0;
			websWriteDataNonBlock(wp, buf, nRead);
			//printf("%s",buf);
			fileSize -= fileSector;
			ptr += fileSector;
			free(buf);
 			//sleep(1);
		}
		fclose(fp);
		//websDone(wp, 200);
		//OK_MSG("/saveconf.asp");
		return;
	}
//#ifndef NO_ACTION
	if(sysCmd[0]){
		snprintf(tmpBuf, 100, "%s 2>&1 > %s",sysCmd,  _PATH_SYSCMD_LOG);
		system(tmpBuf);
	}
//#endif


	strVal = websGetVar(wp, T("adsldbg"), T(""));
	if(strVal[0]!=0){
		if (strVal[0]) {
			if (strVal[0] == '0'){
				adsldbg = 0;
			}
			else{
				adsldbg = 1;
			}
			if ( !mib_set(MIB_ADSL_DEBUG, (void *)&adsldbg)) {
				strcpy(tmpBuf, T("Set LOG Capability error!"));
			}
		}
	}

	websRedirect(wp, submitUrl);
	return;
}

int sysCmdLog(int eid, webs_t wp, int argc, char_t **argv)
{
        FILE *fp;
	char  buf[150];
	int nBytesSent=0;

        fp = fopen(_PATH_SYSCMD_LOG, "r");
        if ( fp == NULL )
                goto err1;
        while(fgets(buf,150,fp)){
		nBytesSent += websWrite(wp, T("%s"), buf);
        }
	fclose(fp);
	unlink(_PATH_SYSCMD_LOG);
err1:
	return nBytesSent;
}
int lanSetting(int eid, webs_t wp, int argc, char_t **argv)
{
char *parm;
	
	if (ejArgs(argc, argv, T("%s"), &parm) < 1) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}
 if( parm[0] == '1' ){
 	websWrite(wp, T("menu.addItem(\"Lan Setting\");"
			"lansetting = new MTMenu();"
			"lansetting.addItem(\"LAN Interface\", \"tcpiplan.asp\", \"\", \"LAN Interface Configuration\");"
			"lansetting.addItem(\"DHCP Mode\", \"dhcpmode.asp\", \"\", \"DHCP Mode Configuration\");"
			"lansetting.addItem(\"DHCP Server\", \"dhcpd.asp\", \"\", \"DHCP Server Configuration\");"
			"lansetting.addItem(\"DHCP Relay\", \"dhcrelay.asp\", \"\", \"DHCP Relay Configuration\");"));
	websWrite(wp, T("menu.makeLastSubmenu(lansetting);"));
 	}
 else
  {
  websWrite(wp, T("<tr><td><b>Lan Setting</b></td></tr>\n"
                           "<tr><td>&nbsp;&nbsp;<a href=\"tcpiplan.asp\" target=\"view\">LAN Interface</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"dhcpmode.asp\" target=\"view\">DHCP Mode</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"dhcpd.asp\" target=\"view\">DHCP Server</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"dhcrelay.asp\" target=\"view\">DHCP Relay</a></td></tr>\n"
			));
 	}
 
   return 0;
}

//xl_yue: translocate to fmmenucreate.c
#if 0
int srvMenu(int eid, webs_t wp, int argc, char_t **argv)
{
	char *parm;
	
	if (ejArgs(argc, argv, T("%s"), &parm) < 1) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}
	
	if ( parm[0] == '1' ) {
		websWrite(wp, T("menu.addItem(\"Services\");"
			"service = new MTMenu();"
			"service.addItem(\"DHCP Mode\", \"dhcpmode.asp\", \"\", \"DHCP Mode Configuration\");"
			"service.addItem(\"DHCP Server\", \"dhcpd.asp\", \"\", \"DHCP Server Configuration\");"
			"service.addItem(\"DHCP Relay\", \"dhcrelay.asp\", \"\", \"DHCP Relay Configuration\");"));
		websWrite(wp, T("service.addItem(\"DNS\");"
			"dns = new MTMenu();"
			"dns.addItem(\"DNS Server\", \"dns.asp\", \"\", \"DNS Configuration\");"
#ifdef CONFIG_USER_DDNS			
			"dns.addItem(\"Dynamic DNS\", \"ddns.asp\", \"\", \"DDNS Configuration\");"
#endif			
			"service.makeLastSubmenu(dns);"));
		websWrite(wp, T("service.addItem(\"Firewall\");"
			"firewall = new MTMenu();"
			"firewall.addItem(\"IP/Port Filtering\", \"fw-ipportfilter.asp\", \"\", \"Setup IP/Port filering\");"
#ifdef MAC_FILTER
			"firewall.addItem(\"MAC Filtering\", \"/admin/fw-macfilter.asp\", \"\", \"Setup MAC filering\");"
#endif
			"firewall.addItem(\"Port Forwarding\", \"fw-portfw.asp\", \"\", \"Setup port-forwarding\");"));
#ifdef URL_BLOCKING_SUPPORT
		websWrite(wp, T("firewall.addItem(\"URL Blocking\", \"url_blocking.asp\", \"\", \"URL Blocking Setting\");"));			
#endif	
#ifdef DOMAIN_BLOCKING_SUPPORT	
		websWrite(wp, T("firewall.addItem(\"Domain Blocking\", \"domainblk.asp\", \"\", \"Domain Blocking Setting\");"));		
#endif
#ifdef NATIP_FORWARDING
		websWrite(wp, T("firewall.addItem(\"NAT IP Forwarding\", \"fw-ipfw.asp\", \"\", \"Setup NAT IP Mapping\");"));
#endif
#ifdef PORT_TRIGGERING
		websWrite(wp, T("firewall.addItem(\"Port Triggering\", \"gaming.asp\", \"\", \"Setup Port Triggering\");"));
#endif
		websWrite(wp, T("firewall.addItem(\"DMZ\", \"fw-dmz.asp\", \"\", \"Setup DMZ\");"
			"service.makeLastSubmenu(firewall);"
#ifdef CONFIG_USER_IGMPPROXY
			"service.addItem(\"IGMP Proxy\", \"igmproxy.asp\", \"\", \"IGMP Proxy Configuration\");"
#endif
//#ifdef CONFIG_USER_UPNPD
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
			"service.addItem(\"UPnP\", \"upnp.asp\", \"\", \"UPnP Configuration\");"			
#endif	
#ifdef CONFIG_USER_ROUTED_ROUTED
			"service.addItem(\"RIP\", \"rip.asp\", \"\", \"RIP Configuration\");"
#endif
			"menu.makeLastSubmenu(service);"));
		websWrite(wp, T("menu.makeLastSubmenu(service);"));
	}
	else {
		websWrite(wp, T("<tr><td><b>Services</b></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"dhcpmode.asp\" target=\"view\">DHCP Mode</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"dhcpd.asp\" target=\"view\">DHCP Server</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"dhcrelay.asp\" target=\"view\">DHCP Relay</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"dns.asp\" target=\"view\">DNS</a></td></tr>\n"
#ifdef CONFIG_USER_DDNS			
			"<tr><td>&nbsp;&nbsp;<a href=\"ddns.asp\" target=\"view\">Dynamic DNS</a></td></tr>\n"
#endif
			));
		websWrite(wp, T("<tr><td>&nbsp;&nbsp;<b>Firewall</b></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"fw-ipportfilter.asp\" target=\"view\">IP/Port Filtering</a></td></tr>\n"
#ifdef MAC_FILTER
			"<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"/admin/fw-macfilter.asp\" target=\"view\">MAC Filtering</a></td></tr>\n"
#endif
			"<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"fw-portfw.asp\" target=\"view\">Port Forwarding</a></td></tr>\n"));
#ifdef URL_BLOCKING_SUPPORT
		websWrite(wp, T("<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"url_blocking.asp\" target=\"view\">URL Blocking</a></td></tr>\n"));
#endif
#ifdef DOMAIN_BLOCKING_SUPPORT		
		websWrite(wp, T("<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"domainblk.asp\" target=\"view\">Domain Blocking</a></td></tr>\n"));
#endif
#ifdef NATIP_FORWARDING
		websWrite(wp, T("<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"fw-ipfw.asp\" target=\"view\">NAT IP Forwarding</a></td></tr>\n"));
#endif
#ifdef PORT_TRIGGERING
		websWrite(wp, T("<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"gaming.asp\" target=\"view\">Port Triggering</a></td></tr>\n"));
#endif
		websWrite(wp, T("<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"fw-dmz.asp\" target=\"view\">DMZ</a></td></tr>"
#ifdef CONFIG_USER_IGMPPROXY
			"<tr><td>&nbsp;&nbsp;<a href=\"igmproxy.asp\" target=\"view\">IGMP Proxy</a></td></tr>"
#endif
//#ifdef CONFIG_USER_UPNPD
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
			"<tr><td>&nbsp;&nbsp;<a href=\"upnp.asp\" target=\"view\">UPnP</a></td></tr>"
#endif
#ifdef CONFIG_USER_ROUTED_ROUTED
			"<tr><td>&nbsp;&nbsp;<a href=\"rip.asp\" target=\"view\">RIP</a></td></tr>"
#endif
		));
	}
	return 0;
}
#endif

// Admin web menu
// parm: 1 -> javascript support
//	 0 -> no js
//xl_yue: transloacte to fmmenucreate.c
#if 0
int adminMenu(int eid, webs_t wp, int argc, char_t **argv)
{
	char *parm;
	
	if (ejArgs(argc, argv, T("%s"), &parm) < 1) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}
	
	if ( parm[0] == '1' ) {
		websWrite(wp, T("menu.addItem(\"Admin\");"
			"admin = new MTMenu();"
#ifdef FINISH_MAINTENANCE_SUPPORT			
			//added by xl_yue
			"admin.addItem(\"Finish Maintenance\", \"/finishmaintenance.asp\", \"\", \"Finish Maintenance\");"
#endif			
			"admin.addItem(\"Commit/Reboot\", \"/admin/reboot.asp\", \"\", \"Commit/reboot the system\");"
#ifdef CONFIG_SAVE_RESTORE			
			"admin.addItem(\"Backup/Restore\", \"/admin/saveconf.asp\", \"\", \"Backup/restore current settings\");"
#endif	
#ifdef ACCOUNT_LOGIN_CONTROL
			//added by xl_yue
			"admin.addItem(\"Logout\", \"/admin/adminlogout.asp\", \"\", \"Logout\");"
#endif			

#ifdef SYSLOG_SUPPORT
			"admin.addItem(\"System Log\", \"syslog.asp\", \"\", \"Show system log\");"
#endif
#ifdef DOS_SUPPORT
			"admin.addItem(\"DOS\", \"dos.asp\", \"\", \"Denial of service\");"
#endif
			"admin.addItem(\"Password\", \"password.asp\", \"\", \"Setup access password\");"));
		websWrite(wp, T(
#ifdef WEB_UPGRADE
#ifdef UPGRADE_V1
			"admin.addItem(\"Upgrade Firmware\", \"/admin/upload.asp\", \"\", \"Update firmware image\");"
#else
#ifdef UPGRADE_V2
			"admin.addItem(\"Upgrade Firmware\", \"/admin/upload2.asp\", \"\", \"Update firmware image\");"
#endif // of UPGRADE_V2
#ifdef UPGRADE_V3
			"admin.addItem(\"Upgrade Firmware\", \"/admin/upload3.asp\", \"\", \"Update firmware image\");"
#endif // of UPGRADE_V3
#endif // of UPGRADE_V1
#endif // of WEB_UPGRADE
			/*
			"admin.addItem(\"Remote Config\", \"rconfig.asp\", \"\", \"Remote configuration from WAN side\");"
			*/
#ifdef IP_ACL
			"admin.addItem(\"ACL Config\", \"/admin/acl.asp\", \"\", \"ACL Setting\");"
#endif
#ifdef TIME_ZONE
			"admin.addItem(\"Time Zone\", \"ntp.asp\", \"\", \"Setup Time Zone\");"
#endif
#ifdef AUTO_PROVISIONING
			"admin.addItem(\"Auto-Provisionning\", \"autoprovision.asp\", \"\", \"Auto-Provisioning Configuration\");"
#endif
#ifdef _CWMP_MIB_
			"admin.addItem(\"TR-069 Config\", \"tr069config.asp\", \"\", \"TR-069 Configuration\");"
#endif
//added by xl_yue
#ifdef USE_LOGINWEB_OF_SERVER
//			"admin.addItem(\"Login\", \"/admin/login.asp\", \"\", \"Login\");"
			"admin.addItem(\"Logout\", \"/admin/logout.asp\", \"\", \"Logout\");"
#endif

			));
		websWrite(wp, T("menu.makeLastSubmenu(admin);"));
	}
	else {
		websWrite(wp, T("<tr><td><b>Admin</b></td></tr>\n"
#ifdef FINISH_MAINTENANCE_SUPPORT
			//added by xl_yue
			"<tr><td>&nbsp;&nbsp;<a href=\"/finishmaintenance.asp\" target=\"view\">Finish Maintenance</a></td></tr>\n"
#endif			
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/reboot.asp\" target=\"view\">Commit/Reboot</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/saveconf.asp\" target=\"view\">Backup/Restore</a></td></tr>\n"

#ifdef ACCOUNT_LOGIN_CONTROL
			//added by xl_yue
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/adminlogout.asp\" target=\"view\">Logout</a></td></tr>\n"
#endif			

#ifdef SYSLOG_SUPPORT
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/syslog.asp\" target=\"view\">System log</a></td></tr>\n"
#endif
#ifdef DOS_SUPPORT
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/dos.asp\" target=\"view\">DOS</a></td></tr>\n"
#endif
			"<tr><td>&nbsp;&nbsp;<a href=\"password.asp\" target=\"view\">Password</a></td></tr>\n"));
		websWrite(wp, T(
#ifdef WEB_UPGRADE
#ifdef UPGRADE_V1
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/upload.asp\" target=\"view\">Upgrade Firmware</a></td></tr>\n"
#else
#ifdef UPGRADE_V2
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/upload2.asp\" target=\"view\">Upgrade Firmware</a></td></tr>\n"
#endif // of UPGRADE_V2
#ifdef UPGRADE_V3
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/upload3.asp\" target=\"view\">Upgrade Firmware</a></td></tr>\n"
#endif // of UPGRADE_V3
#endif // of UPGRADE_V1
#endif // of WEB_UPGRADE
/*
			"<tr><td>&nbsp;&nbsp;<a href=\"rconfig.asp\" target=\"view\">Remote Config</a></td></tr>\n"
*/
#ifdef IP_ACL
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/acl.asp\" target=\"view\">ACL Config</a></td></tr>\n"
#endif
#ifdef TIME_ZONE
			"<tr><td>&nbsp;&nbsp;<a href=\"ntp.asp\" target=\"view\">Time Zone Setting</a></td></tr>\n"
#endif
#ifdef AUTO_PROVISIONING
			"<tr><td>&nbsp;&nbsp;<a href=\"autoprovision.asp\" target=\"view\">Auto-Provisioning Configuration</a></td></tr>\n"
#endif
#ifdef _CWMP_MIB_
			"<tr><td>&nbsp;&nbsp;<a href=\"tr069config.asp\" target=\"view\">TR-069 Config</a></td></tr>\n"
#endif

//added by xl_yue
#ifdef USE_LOGINWEB_OF_SERVER
//			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/login.asp\" target=\"view\">Login</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/logout.asp\" target=\"view\">Logout</a></td></tr>\n"
#endif

			));
	}
	return 0;
}
#endif

//xl_yue add
#ifdef WEB_MENU_USE_MTM
#if !defined(ZTE_531b_BRIDGE_SC) && !defined(ZTE_GENERAL_ROUTER_SC)
int userAddAdminMenu(int eid, webs_t wp, int argc, char_t **argv)
{
	char *parm;
	
	if (ejArgs(argc, argv, T("%s"), &parm) < 1) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}
#ifdef ACCOUNT_LOGIN_CONTROL
	if ( parm[0] == '1' ) {
		websWrite(wp, T(
//			"admin.addItem(\"Logout\", \"/admin/userlogout.asp\", \"\", \"Logout\");"
			"admin.addItem(\"Logout\", \"userlogout.asp\", \"\", \"Logout\");"
		));
	}
	else {
		websWrite(wp, T(
//			"<tr><td>&nbsp;&nbsp;<a href=\"/admin/userlogout.asp\" target=\"view\">Logout</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"userlogout.asp\" target=\"view\">Logout</a></td></tr>\n"
		));
	}
#endif

#ifdef USE_LOGINWEB_OF_SERVER
	if ( parm[0] == '1' ) {
		websWrite(wp, T(
//			"admin.addItem(\"Login\", \"login.asp\", \"\", \"Login\");"
			"admin.addItem(\"Logout\", \"logout.asp\", \"\", \"Logout\");"
		));
	}
	else {
		websWrite(wp, T(
//			"<tr><td>&nbsp;&nbsp;<a href=\"login.asp\" target=\"view\">Login</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"logout.asp\" target=\"view\">Logout</a></td></tr>\n"
		));
	}
#endif

	return 0;
}
#endif
#endif

//xl_yue: translocate to fmmenucreate.c
#if 0
int diagMenu(int eid, webs_t wp, int argc, char_t **argv)
{
	char *parm;
	
	if (ejArgs(argc, argv, T("%s"), &parm) < 1) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}
	
	if ( parm[0] == '1' ) {
		websWrite(wp, T("menu.addItem(\"Diagnostic\");"
			"diag = new MTMenu();"
			"diag.addItem(\"Ping\", \"ping.asp\", \"\", \"Ping test\");"
			"diag.addItem(\"ATM Loopback\", \"oamloopback.asp\", \"\", \"ATM OAM loopback test\");"
			"diag.addItem(\"ADSL\", \"/adv/adsl-diag.asp\", \"\", \"ADSL Tone Diagnostics\");"
#ifdef DIAGNOSTIC_TEST
			"diag.addItem(\"Diagnostic Test\", \"diag-test.asp\", \"\", \"Diagnostic test\");"
#endif
		));
		websWrite(wp, T("menu.makeLastSubmenu(diag);"));
	}
	else {
		websWrite(wp, T("<tr><td><b>Diagnostic</b></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"ping.asp\" target=\"view\">Ping</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"oamloopback.asp\" target=\"view\">ATM Loopback</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"/adv/adsl-diag.asp\" target=\"view\">ADSL</a></td></tr>\n"
#ifdef DIAGNOSTIC_TEST
			"<tr><td>&nbsp;&nbsp;<a href=\"diag-test.asp\" target=\"view\">Diagnostic Test</a></td></tr>\n"
#endif
		));
	}
	return 0;
}
#endif

static int process_msg(char *msg, int is_wlan_only)
{
	char *p1, *p2;
	
	p2 = strstr(p1, "wlan");	
	if (p2 && p2[5]==':')  
		memcpy(p1, p2, strlen(p2)+1);
	else {
		if (is_wlan_only)
			return 0;
		
		p2 = strstr(p1, "klogd: ");
		if (p2 == NULL)
			return 0;
		memcpy(p1, p2+7, strlen(p2)-7+1);
	}
	return 1;
}

#ifdef FINISH_MAINTENANCE_SUPPORT
//added by xl_yue for supporting inform ITMS after finishing maintenance
void formFinishMaintenance(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str, *submitUrl;
	pid_t tr069_pid;

	str = websGetVar(wp, T("finish"), T(""));
	if (str[0]) {
		// signal tr069 to inform ITMS that maintenance is finished
		tr069_pid = read_pid("/var/run/cwmp.pid");
		if ( tr069_pid > 0)
			kill(tr069_pid, SIGUSR1);
		else 
			goto setErr_Signal;
	}

	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	OK_MSG1("OK:start to inform ITMS that maintenance is over!",submitUrl);
  	return;

setErr_Signal:
	ERR_MSG("ERROR:can not find TR069 pcocess!");

}
#endif

//added by xl_yue
#ifdef USE_LOGINWEB_OF_SERVER

#ifdef USE_BASE64_MD5_PASSWD
void calPasswdMD5(char *pass, char *passMD5);
#endif

void formLogin(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str,*username,*password, *submitUrl;
	char	suPasswd[MAX_NAME_LEN], usPasswd[MAX_NAME_LEN];
#ifdef USE_BASE64_MD5_PASSWD
	char md5_pass[32];
#endif
	struct user_info * pUser_info;
#ifdef LOGIN_ERR_TIMES_LIMITED
	struct errlogin_entry * pErrlog_entry = NULL;
#endif
#ifdef CTC_TELECOM_ACCOUNT
	unsigned char vChar;
#endif
	char usrName[MAX_NAME_LEN];
	char supName[MAX_NAME_LEN];
	//xl_yue:1:bad password;2:invalid user;3:login error for three times,forbidden;4:other has login;
	int denied = 1;		
#ifdef ACCOUNT_CONFIG
	MIB_CE_ACCOUNT_CONFIG_T Entry;
	int totalEntry, i;
#endif

	str = websGetVar(wp, T("save"), T(""));
	if (str[0]) {
		pUser_info = search_login_list(wp);
		if(pUser_info){
			denied = 5;
			goto setErr_Signal;
		}
		username = websGetVar(wp, T("username"), T(""));
		// Mason Yu on True
		//printf("username=%s\n", username);
		strcpy(g_login_username, username);
		if (!username[0] ) {
			denied = 2;
			goto setErr_Signal;
		}
		password = websGetVar(wp, T("password"), T(""));
		if (!password[0] ) {
			denied = 1;
			goto setErr_Signal;
		}
	}else{
		denied = 10;
		goto setErr_Signal;
	}

#ifdef ACCOUNT_CONFIG
	totalEntry = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL);
	for (i=0; i<totalEntry; i++) {
		if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, i, (void *)&Entry)) {
			denied = 10;
			goto setErr_Signal;
		}
		if (Entry.privilege == (unsigned char)PRIV_ROOT)
			strcpy(supName, Entry.userName);
		else
			strcpy(usrName, Entry.userName);
		if (strcmp(username, Entry.userName) == 0) {
#ifdef USE_BASE64_MD5_PASSWD
			calPasswdMD5(Entry.userPassword, md5_pass);
			if (strcmp(password, md5_pass))
#else
			if (strcmp(password,Entry.userPassword))
#endif
			{
				denied = 1;
				goto setErr_Signal;
			}
			denied = 0;
			goto pass_check;
		}
	}
#endif
	if ( !mib_get(MIB_USER_NAME, (void *)usrName) ) {							
		denied = 10;
		goto setErr_Signal;
	}

	if(strcmp(usrName, username)==0){
		if ( !mib_get(MIB_USER_PASSWORD, (void *)usPasswd) ) {							
			denied = 10;
			goto setErr_Signal;
		}
#ifdef USE_BASE64_MD5_PASSWD
		calPasswdMD5(usPasswd, md5_pass);
		if(strcmp(password,md5_pass))
#else
		if(strcmp(password,usPasswd))
#endif
		{
			denied = 1;
			goto setErr_Signal;
		}
		denied = 0;
		goto pass_check;
	}

#ifdef CTC_TELECOM_ACCOUNT
	if(!mib_get(MIB_CTC_ACCOUNT_ENABLE, (void *)&vChar)){
		denied = 10;
		goto setErr_Signal;
	}
	if(!vChar){ 
		denied = 2;
		goto setErr_Signal;
	}
#endif

	if ( !mib_get(MIB_SUSER_NAME, (void *)supName) ) {							
		denied = 10;
		goto setErr_Signal;
	}

	if(strcmp(supName, username)==0){
		if ( !mib_get(MIB_SUSER_PASSWORD, (void *)suPasswd) ){							
			denied = 10;
			goto setErr_Signal;
		}
#ifdef USE_BASE64_MD5_PASSWD
		calPasswdMD5(suPasswd, md5_pass);
		if(strcmp(password,md5_pass))
#else
		if(strcmp(password,suPasswd))
#endif
		{
			denied = 1;
			goto setErr_Signal;
		}
		denied = 0;
		goto pass_check;
	}

	if(denied){
		denied = 2;
		goto setErr_Signal;
	}

pass_check:

#ifdef ONE_USER_LIMITED
	if(!strcmp(usrName, username) && usStatus.busy){
		if(strcmp(usStatus.remote_ip_addr, wp->remote_ip_addr)){
			denied = 4;
			goto setErr_Signal;
		}
	}else if(!strcmp(supName, username) && suStatus.busy){
		if(strcmp(suStatus.remote_ip_addr, wp->remote_ip_addr)){
			denied = 4;
			goto setErr_Signal;
		}
	}
#endif

	pUser_info = search_login_list(wp);
	if(!pUser_info){
		pUser_info = malloc(sizeof(struct user_info));
		pUser_info->last_time = time_counter;
//		pUser_info->login_status = STATUS_LOGIN;
		strncpy(pUser_info->remote_ip_addr, wp->remote_ip_addr, sizeof(pUser_info->remote_ip_addr));
		if(strcmp(usrName, username)==0){
			pUser_info->directory = strdup("/admin/index_user.html");
#ifdef ONE_USER_LIMITED
			pUser_info->paccount = &usStatus;
			pUser_info->paccount->busy = 1;
			strncpy(pUser_info->paccount->remote_ip_addr, wp->remote_ip_addr, sizeof(pUser_info->paccount->remote_ip_addr));
#endif
		}
		else{
			pUser_info->directory = strdup("/index.html");
#ifdef ONE_USER_LIMITED
			pUser_info->paccount = &suStatus;
			pUser_info->paccount->busy = 1;
			strncpy(pUser_info->paccount->remote_ip_addr, wp->remote_ip_addr, sizeof(pUser_info->paccount->remote_ip_addr));
#endif
		}
		//list it to user_login_list
		pUser_info->next = user_login_list;
		user_login_list = pUser_info;

		syslog(LOG_INFO, "login successful for %s from %s\n", username, wp->remote_ip_addr);
	}else{
//		if(pUser_info->login_status != STATUS_FORBIDDEN){
			pUser_info->last_time = time_counter;
//			pUser_info->login_status = STATUS_LOGIN;
//		}
	}

#ifdef LOGIN_ERR_TIMES_LIMITED
	free_from_errlog_list(wp);
#endif

	websRedirect(wp, "/");
	
//	submitUrl = websGetVar(wp, T("submit-url"), T(""));
//	OK_MSG1("OK:login successfully!",submitUrl);
  	return;

setErr_Signal:

#ifdef LOGIN_ERR_TIMES_LIMITED
	if(denied == 1 || denied == 2){
		pErrlog_entry = search_errlog_list(wp);
		if(pErrlog_entry){
			pErrlog_entry->last_time = time_counter;
			pErrlog_entry->login_count++;
			if(pErrlog_entry->login_count >= MAX_LOGIN_NUM)
				denied = 3;
		}else{
			pErrlog_entry = malloc(sizeof(struct errlogin_entry));
			pErrlog_entry->last_time = time_counter;
			pErrlog_entry->login_count = 1;
			strncpy(pErrlog_entry->remote_ip_addr, wp->remote_ip_addr, sizeof(pErrlog_entry->remote_ip_addr));
			pErrlog_entry->next = errlogin_list;
			errlogin_list = pErrlog_entry;
		}
	}
#endif
	
	switch(denied){
		case 1:
			ERR_MSG2("ERROR:bad password!");
			syslog(LOG_ERR, "login error from %s for bad password \n",wp->remote_ip_addr);
			break;
		case 2:
			ERR_MSG2("ERROR:invalid username!");
			syslog(LOG_ERR, "login error from %s for invalid username \n",wp->remote_ip_addr);
			break;
#ifdef LOGIN_ERR_TIMES_LIMITED
		case 3:
			ERR_MSG2("ERROR:you have logined error three times, please relogin 1 minute later!");
			syslog(LOG_ERR, "login error from %s for having logined error three times \n",wp->remote_ip_addr);
			break;
#endif
#ifdef ONE_USER_LIMITED
		case 4:
			ERR_MSG2("ERROR:another user have logined in using this account!only one user can login using this account at the same time!");
			syslog(LOG_ERR, "login error from %s for using the same account with another user at the same time\n",wp->remote_ip_addr);
			break;
#endif
		case 5:
			ERR_MSG2("ERROR:you have logined! please logout at first and then login!");
			syslog(LOG_ERR, "login error from %s for having logined\n",wp->remote_ip_addr);
			break;
		default:
			ERR_MSG2("ERROR:web authentication error!please close this window and reopen your web browser to login!");
			syslog(LOG_ERR, "web authentication error!\n");
			break;
		}
}

void formLogout(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str, *submitUrl;
	struct user_info * pUser_info;
	
	str = websGetVar(wp, T("save"), T(""));
	if (str[0]) {
		if(!free_from_login_list(wp)){
			syslog(LOG_ERR, "logout error from %s\n",wp->remote_ip_addr);
			goto setErr_Signal;
		}
		syslog(LOG_INFO, "logout successful from %s\n",wp->remote_ip_addr);
	}

	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	OK_MSG1("OK:logout successfully!",submitUrl);
  	return;

setErr_Signal:
	ERR_MSG("ERROR:logout failed!perhaps you have logouted!");

}

int passwd2xmit(int eid, webs_t wp, int argc, char_t **argv)
{
#ifdef USE_BASE64_MD5_PASSWD
	websWrite(wp, "document.cmlogin.password.value = b64_md5(document.cmlogin.password.value);");
#endif
}

#endif


#ifdef ACCOUNT_LOGIN_CONTROL
//added by xl_yue
/*
extern struct account_info su_account;
extern char suName[MAX_NAME_LEN];
*/
void formAdminLogout(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str, *submitUrl;

	str = websGetVar(wp, T("adminlogout"), T(""));
	if (str[0]) {
		su_account.account_busy = 0;
		su_account.account_timeout = 1;
		syslog(LOG_INFO, "Account: %s logout from %s\n", suName, su_account.remote_ip_addr);
	}
	else
		goto err_logout;
	
	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	OK_MSG1("Logout successfully!",submitUrl);
  	return;
	
err_logout:
	ERR_MSG("Logout error!");
}

//added by xl_yue
/*
extern struct account_info us_account;
extern char usName[MAX_NAME_LEN];
*/
void formUserLogout(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str, *submitUrl;

	str = websGetVar(wp, T("userlogout"), T(""));
	if (str[0]) {
		us_account.account_busy = 0;
		us_account.account_timeout = 1;
		syslog(LOG_INFO, "Account: %s logout from %s\n", usName, us_account.remote_ip_addr);
	}
	else
		goto err_logout;
	
	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	OK_MSG1("Logout successfully!",submitUrl);
  	return;
	
err_logout:
	ERR_MSG("Logout error!");
}

#endif


#ifdef SYSLOG_SUPPORT
#if 0
int sysLogList(int eid, webs_t wp, int argc, char_t **argv)
{
	FILE *fp;
	char  buf[150];
	int nBytesSent=0;
	int enabled;
	unsigned char vChar;

	if(first_time==1){
		mib_get(MIB_ADSL_DEBUG, (void *)&vChar);
		if(vChar==1)
			dbg_enable=1;
		first_time=0;
	}

	if(dbg_enable==0){
		fp = fopen("/var/log/messages.old", "r");
		if (fp) {
			while (fgets(buf, 150, fp))
				nBytesSent += websWrite(wp, T("%s"), buf);
			fclose(fp);
		}
		fp = fopen("/var/log/messages", "r");
		if (fp == NULL){
			//printf("read log file fail!\n");
			goto err1;
		}
       	 //fgets(buf,150,fp);
		while(fgets(buf,150,fp)){
			nBytesSent += websWrite(wp, T("%s"), buf);
		}

		fclose(fp);
	}
err1:
	return nBytesSent;
}
#endif
int sysLogList(int eid, webs_t wp, int argc, char_t **argv)
{
	FILE *fp;
	char buff[256], *facstr, *pristr, *timestr, *tmp, *msgstr;
	int nBytesSent=0;
	int enabled;
	unsigned char vChar;
	int pri;

	if (first_time==1) {
		mib_get(MIB_ADSL_DEBUG, (void *)&vChar);
		if(vChar==1)
			dbg_enable=1;
		first_time=0;
	}

	if (dbg_enable==0) {
		nBytesSent += websWrite(wp, T("<tr>"
			"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Date/Time</b></font></td>\n"
			"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Facility</b></font></td>\n"
			"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Severity</b></font></td>\n"
			"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Message</b></font></td></tr>\n"));

		fp = fopen("/var/log/messages.old", "r");
		if (fp) {
			fgets(buff, sizeof(buff), fp);
			while (fgets(buff, sizeof(buff), fp) != NULL) {
				tmp = strtok(buff, "|");
				timestr = strtok(NULL, "|");
				facstr = strtok(NULL, ".");
				pristr = strtok(NULL, "|");
				msgstr = strtok(NULL, "\n");
				mib_get(MIB_SYSLOG_DISPLAY_LEVEL, (void *)&vChar);
				pri = atoi(tmp);
				if((pri & 0x07) <= vChar)
					nBytesSent += websWrite(wp, T("<tr>"
						"<td bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
						"<td bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
						"<td bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
						"<td bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td></tr>\n"), timestr, facstr, pristr, msgstr);
			}
			fclose(fp);
		}

		if (!(fp = fopen("/var/log/messages", "r"))) {
			//printf("Error: cannot open /var/log/messages - continuing...\n");
			goto err1;
		}
		fgets(buff, sizeof(buff), fp);
		while (fgets(buff, sizeof(buff), fp) != NULL) {
			tmp = strtok(buff, "|");
			timestr = strtok(NULL, "|");
			facstr = strtok(NULL, ".");
			pristr = strtok(NULL, "|");
			msgstr = strtok(NULL, "\n");
			mib_get(MIB_SYSLOG_DISPLAY_LEVEL, (void *)&vChar);
			pri = atoi(tmp);
			if((pri & 0x07) <= vChar)
				nBytesSent += websWrite(wp, T("<tr>"
					"<td bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
					"<td bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
					"<td bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
					"<td bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td></tr>\n"), timestr, facstr, pristr, msgstr);
		}
		fclose(fp);
	}
err1:
	return nBytesSent;
}

static void saveLogFile(webs_t wp, FILE *fp)
{
	unsigned char *ptr;
	unsigned int fileSize,filelen;
	unsigned int fileSector;
	unsigned int maxFileSector;

	//decide the file size	
	fseek(fp, 0, SEEK_END);
	filelen = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fileSize=filelen;

	while (fileSize>0) {
		char buf[0x100];
		maxFileSector = 0x50;
		int nRead;

		fileSector = (fileSize > maxFileSector) ? maxFileSector : fileSize;
		nRead = fread((void *)buf, 1, fileSector, fp);

		websWriteDataNonBlock(wp, buf, nRead);

		fileSize -= fileSector;
		ptr += fileSector;
	}
}

void formSysLog(webs_t wp, char_t *path, char_t *query)
{
	char_t  *submitUrl,*strVal,*str;
	unsigned char logcap;
	char tmpBuf[100];
	struct in_addr inIp;

	//max msg. length
	str = websGetVar(wp, T("maxloglen"), T(""));
	if (str[0]) {
		unsigned int len;
		len = (unsigned int) strtol(str, (char**)NULL, 10);
		if ( mib_set(MIB_MAXLOGLEN, (void *)&len) == 0) {
			strcpy(tmpBuf, T("Set max msg. length MIB error!"));
			goto formSysLog_err;
		}
	}
	
	// Set ACL Capability
	strVal = websGetVar(wp, T("apply"), T(""));
	if (strVal[0]) {
		struct in_addr inIpAddr;
		strVal = websGetVar(wp, T("logcap"), T(""));
		if (strVal[0]) {
			if (strVal[0] == '0')
				logcap = 0;
			else
				logcap = 1;
			if ( !mib_set(MIB_SYSLOG, (void *)&logcap)) {
				strcpy(tmpBuf, T("Set LOG Capability error!"));
				goto formSysLog_err;
			}
		}
		
#ifdef WEB_ENABLE_PPP_DEBUG
		strVal = websGetVar(wp, T("pppcap"), T(""));
		if (strVal[0]) {
			unsigned char pppcap;
			struct data_to_pass_st msg;
			pppcap = strVal[0] - '0';
			snprintf(msg.data, BUF_SIZE, "spppctl syslog %d", pppcap);
			write_to_pppd(&msg);
		}
#endif
	
		strVal = websGetVar(wp, T("levelLog"), T(""));
		if (strVal[0]) {
			logcap = strVal[0] - '0' ;
			if (mib_set(MIB_SYSLOG_LOG_LEVEL, (void *)&logcap) == 0) {
				strcpy(tmpBuf, T("Set Syslog Log Level error!"));
				goto formSysLog_err;
			}
		}
		strVal = websGetVar(wp, T("levelDisplay"), T(""));
		if (strVal[0]) {
			logcap = strVal[0] - '0' ;
			if (mib_set(MIB_SYSLOG_DISPLAY_LEVEL, (void *)&logcap) == 0) {
				strcpy(tmpBuf, T("Set Syslog Display Level error!"));
				goto formSysLog_err;
			}
		}
#ifdef SYSLOG_REMOTE_LOG
		strVal = websGetVar(wp, T("logMode"), T(""));
		if (strVal[0]) {
			logcap = strVal[0] - '0' ;
			if (mib_set(MIB_SYSLOG_MODE, (void *)&logcap) == 0) {
				strcpy(tmpBuf, T("Set Syslog Display Level error!"));
				goto formSysLog_err;
			}
		}
		strVal = websGetVar(wp, T("logAddr"), T(""));
		if (strVal[0]) {
			if (!inet_aton(strVal, &inIpAddr)) {
				strcpy(tmpBuf, T(strWrongIP));
				goto formSysLog_err;
			}
			if (!mib_set(MIB_SYSLOG_SERVER_IP, (void *)&inIpAddr)) {
				strcpy(tmpBuf, T(strSetIPerror));
				goto formSysLog_err;
			}
		}
		strVal = websGetVar(wp, T("logPort"), T(""));
		if (strVal && strVal[0] ) { 			
			unsigned short intVal = atoi(strVal);
			if (!mib_set(MIB_SYSLOG_SERVER_PORT, (void *)&intVal)) {
				strcpy(tmpBuf, T(Tsvr_port));
				goto formSysLog_err;
			}
		}
#endif
 	}
	
	// Set Log Server IP
#ifdef SEND_LOG
	strVal = websGetVar(wp, T("ip"), T(""));
	if (strVal[0]) {
		if ( !inet_aton(strVal, &inIp) ) {
			strcpy(tmpBuf, T(strWrongIP));
			goto formSysLog_err;
		}
		
		if ( !mib_set( MIB_LOG_SERVER_IP, (void *)&inIp)) {
			strcpy(tmpBuf, T("Set LOG Server IP error!"));
			goto formSysLog_err;
		}				
 	}
	
	// Set User name for Log Server
	strVal = websGetVar(wp, T("username"), T(""));
	if (strVal[0]) {				
		if ( !mib_set(MIB_LOG_SERVER_NAME, (void *)strVal)) {
			strcpy(tmpBuf, T("Set user name for LOG Server IP error!"));
			goto formSysLog_err;
		}
	}
	
	// Set Passeord for Log Server
	strVal = websGetVar(wp, T("passwd"), T(""));
	if (strVal[0]) {		
		if ( !mib_set(MIB_LOG_SERVER_PASSWORD, (void *)strVal)) {
			strcpy(tmpBuf, T("Set user name for LOG Server IP error!"));
			goto formSysLog_err;
		}
	}
#endif

	// Save log File
	strVal = websGetVar(wp, T("save_log"), T(""));
	if (strVal[0])
	{
		/*unsigned char *ptr;
		unsigned int fileSize,filelen;
		unsigned int fileSector;
		unsigned int maxFileSector;*/
		FILE *fp, *fp2;
		
		fp=fopen("/var/log/messages","r");
		if ( fp == NULL ) {
			strcpy(tmpBuf, T("System Log not exists!"));
			goto formSysLog_err;
		}
		
		wp->buffer_end=0; // clear header
		websWrite(wp, "HTTP/1.0 200 OK\n");
		websWrite(wp, "Content-Type: application/octet-stream;\n");	
		websWrite(wp, "Content-Disposition: attachment;filename=\"messages.txt\" \n");	
		websWrite(wp, "Pragma: no-cache\n");
		websWrite(wp, "Cache-Control: no-cache\n");
		websWrite(wp, "\n");	
		
		fp2 = fopen("/var/log/messages.old", "r");
		if (fp2) {
			saveLogFile(wp, fp2);
			fclose(fp2);
		}
		/*
		//decide the file size	
		fseek(fp, 0, SEEK_END);
		filelen = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		fileSize=filelen;
	
		while(fileSize>0)
		{
			char buf[0x100];
			maxFileSector=0x50;
			int nRead;
			
//			printf("write %d %d %08x\n",maxFileSector, fileSize, (unsigned int )ptr);
			fileSector = (fileSize>maxFileSector)?maxFileSector:fileSize;
			//fileSector = fileSize;
			nRead = fread((void *)buf, 1, fileSector, fp);
	
			websWriteDataNonBlock(wp, buf, nRead);
			
			fileSize -= fileSector;
			ptr += fileSector;
			//wrong free....
			//free(buf);
 			//sleep(1);
		}
		*/
		saveLogFile(wp, fp);
		fclose(fp);
		return;
		
	}
	
	// Clear log file
	str = websGetVar(wp, T("clear_log"), T(""));
	if (str[0]) {
		unlink("/var/log/messages.old");
		unlink("/var/log/messages");
		submitUrl = websGetVar(wp, T("submit-url"), T(""));
		if (submitUrl[0])
			websRedirect(wp, submitUrl);
		else
			websDone(wp, 200);
	  	return;
	}

formSysLog_end:
#if defined(APPLY_CHANGE)
	// Mason Yu. Take effect in real time.
	stopLog();
	startLog();
#endif
	submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	OK_MSG1(strSetOkToCommitReboot, submitUrl);
#else
	OK_MSG(submitUrl);
#endif
	return;
formSysLog_err:
	ERR_MSG(tmpBuf);
}

#ifdef WEB_ENABLE_PPP_DEBUG
void ShowPPPSyslog(int eid, webs_t wp, int argc, char_t **argv)
{
	websWrite(wp, T("<tr>\n\t<td width=\"25%%\"><font size=2><b>Show PPP Debug Message&nbsp;:</b></td>\n"));
	websWrite(wp, T("\t<td width=\"30%%\"><font size=2>\n"));
	websWrite(wp, T("\t\t<input type=\"radio\" value=\"0\" name=\"pppcap\">Disable&nbsp;&nbsp;"));
	websWrite(wp, T("\n\t\t<input type=\"radio\" value=\"1\" name=\"pppcap\">Enable"));
	websWrite(wp, T("\n\t</td>\n</tr>\n"));
}
#endif

void RemoteSyslog(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t *name;
	
	if (ejArgs(argc, argv, T("%s"), &name) < 1) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}

	if (!strncmp(name, T("syslog-mode"), 11)) {
#ifdef SYSLOG_REMOTE_LOG
		websWrite(wp, T("<tr>\n\t<td><font size=2><b>Mode&nbsp;:</b></td>\n"));
		websWrite(wp, T("\t<td><select name='logMode' size=\"1\" onChange='cbClick(this)'>\n"));
		checkWrite(eid, wp, argc, argv);
#else
		websWrite(wp, T("<input type=\"hidden\" name=\"logMode\">\n"));
#endif
	}

	if (!strncmp(name, T("server-info"), 11)) {
#ifdef SYSLOG_REMOTE_LOG
		websWrite(wp, T("\n\t</select></td>\n</tr>\n"
				"\t<td><font size=2><b>Server IP Address&nbsp;:</b></td>\n"
				"\t<td><input type='text' name='logAddr' maxlength=\"15\"></td>\n"
				"</tr>\n<tr>\n"
				"\t<td><font size=2><b>Server UDP Port&nbsp;:</b></td>\n"
				"\t<td><input type='text' name='logPort' maxlength=\"15\"></td>\n"
				"</tr>\n"));
#else
		websWrite(wp, T("<input type=\"hidden\" name=\"logAddr\">\n"));
		websWrite(wp, T("<input type=\"hidden\" name=\"logPort\">\n"));
#endif
	}
	
	if (!strncmp(name, T("check-ip"), 8)) {
#ifdef SYSLOG_REMOTE_LOG
		websWrite(wp, T("\tif (document.forms[0].logAddr.disabled == false && !checkIP(document.formSysLog.logAddr))\n"));
		websWrite(wp, T("\t\treturn false;\n"));
#endif
	}
}
#endif	// of SYSLOG_SUPPORT

#ifdef TIME_ZONE
void formNtp(webs_t wp, char_t *path, char_t *query)
{
	char_t *submitUrl,*strVal, *tmpStr;
	char tmpBuf[100];
	char enabled=0, ntpServerIdx;
	struct in_addr ipAddr ;
	
	strVal = websGetVar(wp, T("save"), T(""));   

	if(strVal[0]){
		struct tm tm_time;
		time_t tm;
		
		tm_time.tm_isdst = -1;  /* Be sure to recheck dst. */
		strVal = websGetVar(wp, T("year"), T(""));	
		tm_time.tm_year = atoi(strVal) - 1900;
		strVal = websGetVar(wp, T("month"), T(""));	
		tm_time.tm_mon = atoi(strVal)-1;
		strVal = websGetVar(wp, T("day"), T(""));	
		tm_time.tm_mday = atoi(strVal);
		strVal = websGetVar(wp, T("hour"), T(""));	
		tm_time.tm_hour = atoi(strVal);
		strVal = websGetVar(wp, T("minute"), T(""));	
		tm_time.tm_min = atoi(strVal);
		strVal = websGetVar(wp, T("second"), T(""));	
		tm_time.tm_sec = atoi(strVal);
		tm = mktime(&tm_time);
		if(tm < 0){
			sprintf(tmpBuf, "set Time Error\n");
			goto setErr_end;
		}
		if(stime(&tm) < 0){
			sprintf(tmpBuf, "set Time Error\n");
			goto setErr_end;
		}

		tmpStr = websGetVar(wp, T("timeZone"), T(""));  
		if(tmpStr[0]){
			FILE *fp;
			
			if ( mib_set(MIB_NTP_TIMEZONE, (void *)tmpStr) == 0) {
					strcpy(tmpBuf, T("Set Time Zone error!"));
				goto setErr_end;
			}
			strVal = strstr(tmpStr, " ");
			if (strVal != NULL)
				strVal[0] = '\0';
			snprintf(tmpBuf, 16, "GMT%s", tmpStr);
			//setenv("TZ", tmpBuf, 1);
			if ((fp = fopen("/etc/TZ", "w")) != NULL) {
				fprintf(fp, "%s\n", tmpBuf);
				fclose(fp);
			}
		}

		tmpStr = websGetVar(wp, T("enabled"), T(""));
		if(!strcmp(tmpStr, "ON"))
			enabled = 1 ;
		else 
			enabled = 0 ;
		if ( mib_set( MIB_NTP_ENABLED, (void *)&enabled) == 0) {
			strcpy(tmpBuf, T("Set enabled flag error!"));
			goto setErr_end;
		}
		
		tmpStr = websGetVar(wp, T("ntpServerId"), T(""));  
		if(tmpStr[0]){
			ntpServerIdx = tmpStr[0] - '0' ;
			if ( mib_set(MIB_NTP_SERVER_ID, (void *)&ntpServerIdx) == 0) {
				strcpy(tmpBuf, T("Set Time Zone error!"));
				goto setErr_end;
			}
		}
		
		tmpStr = websGetVar(wp, T("ntpServerIp1"), T(""));  
		if(tmpStr[0]){
			inet_aton(tmpStr, &ipAddr);
			if ( mib_set(MIB_NTP_SERVER_IP1, (void *)&ipAddr) == 0) {
				strcpy(tmpBuf, T("Set NTP server error!"));
				goto setErr_end;
			} 
		}
		
		tmpStr = websGetVar(wp, T("ntpServerIp2"), T(""));  
		if(tmpStr[0]){
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#if 0
			inet_aton(tmpStr, &ipAddr);
			if ( mib_set(MIB_NTP_SERVER_IP2,(void *) &ipAddr ) == 0) {
				strcpy(tmpBuf, T("Set NTP server IP error!"));
				goto setErr_end;
			}
#else
			if ( mib_set(MIB_NTP_SERVER_HOST2, (void *)tmpStr) == 0) {
				strcpy(tmpBuf, T("Set NTP server IP error!"));
				goto setErr_end;
			}
#endif
/*ping_zhang:20081217 END*/
			
		}
	}
	if (enabled == 0)		
		goto  set_ntp_end;
	
set_ntp_end:
#if defined(APPLY_CHANGE)
	if (enabled) {
		stopNTP();
		startNTP();
	} else {
		stopNTP();
	}
#endif
	submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page
	
	OK_MSG(submitUrl);
	return;
	
setErr_end:
	ERR_MSG(tmpBuf);
}
#endif

#ifdef DOS_SUPPORT
void formDosCfg(webs_t wp, char_t *path, char_t *query)
{
	char_t	*submitUrl, *tmpStr;
	char	tmpBuf[100];
	unsigned int	floodCount=0,blockTimer=0;
	unsigned int	enabled = 0;

	submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page

	mib_get(MIB_DOS_ENABLED, (void *)&enabled);

	tmpStr = websGetVar(wp, T("dosEnabled"), T(""));
	if(!strcmp(tmpStr, "ON")) {
		enabled |= 1;

		tmpStr = websGetVar(wp, T("sysfloodSYN"), T(""));
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 2;
			tmpStr = websGetVar(wp, T("sysfloodSYNcount"), T(""));
			string_to_dec(tmpStr,&floodCount);
			if ( mib_set(MIB_DOS_SYSSYN_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, T(strSetDosSYSSYNFLOODErr));
				goto setErr;
			}
		}
		else{
			enabled &= ~2;
		}
		tmpStr = websGetVar(wp, T("sysfloodFIN"), T(""));
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 4;
			tmpStr = websGetVar(wp, T("sysfloodFINcount"), T(""));
			string_to_dec(tmpStr,&floodCount);
			if ( mib_set(MIB_DOS_SYSFIN_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, T(strSetDosSYSFINFLOODErr));
				goto setErr;
			}
		}
		else{
			enabled &= ~4;
		}
		tmpStr = websGetVar(wp, T("sysfloodUDP"), T(""));
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 8;
			tmpStr = websGetVar(wp, T("sysfloodUDPcount"), T(""));
			string_to_dec(tmpStr,&floodCount);
			if ( mib_set(MIB_DOS_SYSUDP_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, T(strSetDosSYSUDPFLOODErr));
				goto setErr;
			}
		}
		else{
			enabled &= ~8;
		}
		tmpStr = websGetVar(wp, T("sysfloodICMP"), T(""));
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x10;
			tmpStr = websGetVar(wp, T("sysfloodICMPcount"), T(""));
			string_to_dec(tmpStr,&floodCount);
			if ( mib_set(MIB_DOS_SYSICMP_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, T(strSetDosSYSICMPFLOODErr));
				goto setErr;
			}
		}
		else{
			enabled &= ~0x10;
		}
		tmpStr = websGetVar(wp, T("ipfloodSYN"), T(""));
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x20;
			tmpStr = websGetVar(wp, T("ipfloodSYNcount"), T(""));
			string_to_dec(tmpStr,&floodCount);
			if ( mib_set(MIB_DOS_PIPSYN_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, T(strSetDosPIPSYNFLOODErr));
				goto setErr;
			}
		}
		else{
			enabled &= ~0x20;
		}
		tmpStr = websGetVar(wp, T("ipfloodFIN"), T(""));
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x40;
			tmpStr = websGetVar(wp, T("ipfloodFINcount"), T(""));
			string_to_dec(tmpStr,&floodCount);
			if ( mib_set(MIB_DOS_PIPFIN_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, T(strSetDosPIPFINFLOODErr));
				goto setErr;
			}
		}
		else{
			enabled &= ~0x40;
		}
		tmpStr = websGetVar(wp, T("ipfloodUDP"), T(""));
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x80;
			tmpStr = websGetVar(wp, T("ipfloodUDPcount"), T(""));
			string_to_dec(tmpStr,&floodCount);
			if ( mib_set(MIB_DOS_PIPUDP_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, T(strSetDosPIPUDPFLOODErr));
				goto setErr;
			}
		}
		else{
			enabled &= ~0x80;
		}
		tmpStr = websGetVar(wp, T("ipfloodICMP"), T(""));
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x100;
			tmpStr = websGetVar(wp, T("ipfloodICMPcount"), T(""));
			string_to_dec(tmpStr,&floodCount);
			if ( mib_set(MIB_DOS_PIPICMP_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, T(strSetDosPIPICMPFLOODErr));
				goto setErr;
			}
		}
		else{
			enabled &= ~0x100;
		}
		tmpStr = websGetVar(wp, T("TCPUDPPortScan"), T(""));
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x200;

			tmpStr = websGetVar(wp, T("portscanSensi"), T(""));
			if( tmpStr[0]=='1' ) {
				enabled |= 0x800000;
			}
			else{
				enabled &= ~0x800000;
			}
		}
		else{
			enabled &= ~0x200;
		}
		tmpStr = websGetVar(wp, T("ICMPSmurfEnabled"), T(""));
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x400;
		}
		else{
			enabled &= ~0x400;
		}
		tmpStr = websGetVar(wp, T("IPLandEnabled"), T(""));
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x800;
		}
		else{
			enabled &= ~0x800;
		}
		tmpStr = websGetVar(wp, T("IPSpoofEnabled"), T(""));
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x1000;
		}
		else{
			enabled &= ~0x1000;
		}
		tmpStr = websGetVar(wp, T("IPTearDropEnabled"), T(""));
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x2000;
		}
		else{
			enabled &= ~0x2000;
		}
		tmpStr = websGetVar(wp, T("PingOfDeathEnabled"), T(""));
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x4000;
		}
		else{
			enabled &= ~0x4000;
		}
		tmpStr = websGetVar(wp, T("TCPScanEnabled"), T(""));
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x8000;
		}
		else{
			enabled &= ~0x8000;
		}
		tmpStr = websGetVar(wp, T("TCPSynWithDataEnabled"), T(""));
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x10000;
		}
		else{
			enabled &= ~0x10000;
		}
		tmpStr = websGetVar(wp, T("UDPBombEnabled"), T(""));
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x20000;
		}
		else{
			enabled &= ~0x20000;
		}
		tmpStr = websGetVar(wp, T("UDPEchoChargenEnabled"), T(""));
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x40000;
		}
		else{
			enabled &= ~0x40000;
		}
		tmpStr = websGetVar(wp, T("sourceIPblock"), T(""));
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x400000;
			tmpStr = websGetVar(wp, T("IPblockTime"), T(""));
			string_to_dec(tmpStr,&blockTimer);
			if ( mib_set(MIB_DOS_BLOCK_TIME, (void *)&blockTimer) == 0) {
				strcpy(tmpBuf, T(strSetDosIPBlockTimeErr));
				goto setErr;
			}
		}
		else{
			enabled &= ~0x400000;
		}
	}
	else
		enabled = 0;

	if ( mib_set(MIB_DOS_ENABLED, (void *)&enabled) == 0) {
		strcpy(tmpBuf, T(strSetDosEnableErr));
		goto setErr;
	}

	//apmib_update(CURRENT_SETTING);
#if defined(APPLY_CHANGE)
	setup_dos_protection();
#endif
	
#ifndef NO_ACTION
	run_init_script("all");
#endif

	submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	strcpy(tmpBuf, T(strSetOkToCommitReboot));
	OK_MSG1(tmpBuf,submitUrl);
#else
	OK_MSG(submitUrl);
#endif
	return;

setErr:
	ERR_MSG(tmpBuf);
}
#endif

#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
void ZTESoftwareVersion(int eid, webs_t wp, int argc, char_t * * argv)
{
	websWrite(wp, T(ZTESOFTWAREVERSION));
}
#endif

#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
void formRefresh(webs_t wp, char_t *path, char_t *query)
{
	char_t *submitUrl, *tmpStr;
	unsigned int time;
	unsigned short truetime;
	tmpStr = websGetVar(wp, T("interval"), T(""));
	sscanf(tmpStr, "%ld", &time);
	truetime=(unsigned int)time;
	mib_set(MIB_REFRESH_TIME, (void *)&truetime);
	
	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
}
#endif

#ifdef WEB_REDIRECT_BY_MAC
void formLanding(webs_t wp, char_t *path, char_t *query)
{
	char_t *submitUrl, *strLTime;
	unsigned int uLTime;		

	strLTime = websGetVar(wp, T("interval"), T(""));		
	if ( strLTime[0] ) {
		sscanf(strLTime, "%u", &uLTime);
	}
	
	mib_set(MIB_WEB_REDIR_BY_MAC_INTERVAL, (void *)&uLTime);
	
	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	if (submitUrl[0])
		//websRedirect(wp, submitUrl);
		OK_MSG(submitUrl);
	return;
}
#endif
