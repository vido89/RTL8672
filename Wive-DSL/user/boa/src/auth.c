/*
 *  Boa, an http server
 *  Copyright (C) 1995 Paul Phillips <psp@well.com>
 *  Authorization "module" (c) 1998,99 Martin Hinner <martin@tdp.cz>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 1, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

extern char *mgmtUserName(void);
extern char *mgmtPassword(void);

#include <stdio.h>
#include <fcntl.h>
#ifdef __UC_LIBC__
#include <unistd.h>
#else
#include <crypt.h>
#endif
#include "syslog.h"
#include "md5.h"
#include "boa.h"
#ifdef SHADOW_AUTH
#include <shadow.h>
#endif
#ifdef EMBED
#include <sys/types.h>
#include <pwd.h>
#include <config/autoconf.h>
#endif

#ifdef SECURITY_COUNTS
#include "../../login/logcnt.c"
#endif

#ifdef EMBED
// Added by Mason Yu for 2 level web page
#include "./LINUX/mib.h"

// Added by Mason Yu
//extern char usName[MAX_NAME_LEN];
#endif

//add by xl_yue
#ifdef ACCOUNT_LOGIN_CONTROL
/*
extern char suName[MAX_NAME_LEN];
extern struct account_info su_account;
extern struct account_info us_account;
extern time_t time_counter;
extern struct errlogin_entry * errlogin_list;
*/
#define MAX_LOG_NUM 3
#endif

#ifdef USE_AUTH

//#define DBG_DIGEST 1

// Added by Mason Yu
#ifdef BOA_UNAUTH_FILE
#ifndef CONFIG_GUI_WEB
#define MAX_UNAUTH_FILE 22
#else
#define MAX_UNAUTH_FILE 72
#endif
char *UNAUTHFILE[MAX_UNAUTH_FILE] = {
	"/",
	"/admin/status.asp",
	"/admin/share.js",
	"/share.js",
	"/status.asp",
	"/goform/admin/formStatus",
#ifndef CONFIG_GUI_WEB
	"/admin/title.html",
	"/code.asp",
	"/admin/code_user.asp",
	"/admin/graphics/topbar.png",
	"/menu-images/menu_root.gif",
	"/menu-images/menu_tee.gif",
	"/menu-images/menu_link_default.gif",
	"/menu-images/menu_folder_closed.gif",
	"/menu-images/menu_bar.gif",
	"/menu-images/menu_corner.gif",
	"/admin/menu-images/menu_root.gif",
	"/admin/menu-images/menu_tee.gif",
	"/admin/menu-images/menu_link_default.gif",
	"/admin/menu-images/menu_folder_closed.gif",
	"/admin/menu-images/menu_bar.gif",
	"/admin/menu-images/menu_corner.gif",
#else
	"/admin/title.asp",
	"/admin/status_pl.asp",
	"/blank.html",
	"/admin/css.css",
	"/admin/stm31.js",
	"/status_pl.asp",
	"/qsetup.asp",
	"/qlogin.asp",
	"/plogin.asp",
	"/popup.asp",
	"/notconnection.asp",
	"/conn-fail.asp",
	"/diag-test.asp",
	"/stats.asp",
	"/advn/adsl-statis.asp",
	"/css.css",
	"/style.css",
	"/css_connfail.css",
	"/css_qlogin.css",
	"/images/spacer.gif",
	"/images/config-bg.gif",
	"/images/config-login-admin.gif",
	"/images/config-login-lock.gif",
	"/images/config-content-btm.gif",
	"/images/config-content-top.gif",
	"/images/config-content-bg.gif",
	"/images/config-content-titlebarbg.gif",
	"/images/config-tabtop-bg.gif",
	"/images/btn-commitreboot.gif",
	"/images/conn-bg.gif",
	"/images/conn-btn-abort.gif",
	"/images/conn-btn-abort-over.gif",
	"/images/conn-btn-again.gif",
	"/images/conn-btn-again-over.gif",
	"/images/conn-dialog-bg.gif",
	"/images/conn-dialog-bgbtm.gif",
	"/images/conn-dialog-top.gif",
	"/images/connect_fail.gif",
	"/images/conn-notice-bg.gif",
	"/images/conn-prolink-logo.png",
	"/images/conn-top-bg.gif",
	"/admin/images/bg1.gif",
	"/admin/images/bg3.gif",
	"/admin/images/logo.gif",
	"/admin/images/spacer.gif",
	"/admin/images/bg4.gif",
	"/admin/images/img-status-b.gif",
	"/admin/images/img-status-a.gif",
	"/admin/images/img-setup-b.gif",
	"/admin/images/img-setup-a.gif",
	"/admin/images/img-lan-b.gif",
	"/admin/images/img-lan-a.gif",
	"/admin/images/img-internet-b.gif",
	"/admin/images/img-internet-a.gif",
	"/admin/images/img-firewall-b.gif",
	"/admin/images/img-firewall-a.gif",
	"/admin/images/img-advance-b.gif",
	"/admin/images/img-advance-a.gif",
	"/admin/images/img-admin-b.gif",
	"/admin/images/img-admin-a.gif",
	"/admin/images/img-wireless-b.gif",
	"/admin/images/img-wireless-a.gif",
	"/admin/images/config-content-titlebarbg.gif",
	"/goform/formPPPAuth",
	"/goform/formStats",
	"/goform/formDiagTest"
#endif
};
#endif

struct _auth_dir_ {
	char *directory;
	FILE *authfile;
	int dir_len;
#if SUPPORT_AUTH_DIGEST
	int authtype; // 0:basic, 1:digest
	char *realm;
#endif
	struct _auth_dir_ *next;
};

typedef struct _auth_dir_ auth_dir;

static auth_dir *auth_hashtable [AUTH_HASHTABLE_SIZE];


// simple function to strip leading space and ending space/LF/CR
// Magician (2007.12.27): Modify to trim all white spaces, and solve some possible secure problems.
char *trim(char *input)
{
	char *tmp, *ret;
	int i, len;

	tmp = input;

	len = strlen(input);
	for( i = 0; i < len; i++ )  // Trim leading spaces.
	{
		if(isspace(*tmp))
			tmp++;
		else
			break;
	}

	len = strlen(tmp);
	for( i = len - 1; i >= 0; i-- )  // Trim trailing spaces.
	{
		if( isspace(tmp[i]) )
			tmp[i] = '\0';
		else
			break;
	}

	return tmp;
}

int istrimed(char chr, char *trimchr)
{
	int i, len;

	if(isspace(chr))
		return 1;

	len = strlen(trimchr);
	for( i = 0; i < len; i++ )
		if( trimchr[i] == chr )
			return 1;

	return 0;
}

//Magician 2007/12/27: Extended for trim function.
char *trimEx(char *input, char *tmchr)
{
	char *tmp, *ret;
	int i, len;

	tmp = input;

	len = strlen(input);
	for( i = 0; i < len; i++ )  // Trim leading spaces.
	{
		if(istrimed(*tmp, tmchr))
			tmp++;
		else
			break;
	}

	len = strlen(tmp);
	for( i = len - 1; i >= 0; i-- )  // Trim trailing spaces.
	{
		if( istrimed(tmp[i], tmchr) )
			tmp[i] = '\0';
		else
			break;
	}

	return tmp;
}

#if SUPPORT_AUTH_DIGEST

//static struct http_session *digest_session, digest_session0;
static struct http_session session_array[HTTP_SESSION_MAX]; // support 2 session only

static struct http_session * http_session_get() {
	int i;
	for (i=0; i< HTTP_SESSION_MAX; i++) {
		if (session_array[i].in_use)
			continue;

		session_array[i].in_use = 1;
		return &session_array[i];
	}
	return 0;
}

static void http_session_free(struct http_session *s) {
	s->in_use = 0;
}

#define soap_random rand()
static void http_da_calc_nonce(char nonce[HTTP_DA_NONCELEN])
{
  static short count = 0xCA53;
  sprintf(nonce, "%8.8x%4.4hx%8.8x", (int)time(NULL), count++, soap_random);
}

static void http_da_calc_opaque(char opaque[HTTP_DA_OPAQUELEN])
{
  sprintf(opaque, "%8.8x", soap_random);
}

static void s2hex(const unsigned char *src, char *dst, int len) {
	int i;
	for (i=0; i < len; i++) {
		sprintf(dst, "%02x", src[i]);
		dst += 2;
	}
	*dst = 0;
}

void CalcDigestMD5(char * pszUsername,
		 char * pszRealm,
		 char * pszPassword,
		 char * pszMethod,
		 char * pszDigestUri,
		 char * pszNonce,
		 char * pszNC,
		 char * pszCNonce,
		 char * pszQop,
		 char * pszMD5Alg,
		 char * pszResult) {
		 
	struct MD5Context md5;
	char szHA1[33], szHA2[33];
	unsigned char bDigest[16];
	int iPos;

	MD5Init(&md5);
	MD5Update(&md5, (unsigned char*)pszUsername, strlen(pszUsername));
	MD5Update(&md5, (unsigned char*)":", 1);
	MD5Update(&md5, (unsigned char*)pszRealm, strlen(pszRealm));
	MD5Update(&md5, (unsigned char*)":", 1);
	MD5Update(&md5, (unsigned char*)pszPassword, strlen(pszPassword));
	if( *pszMD5Alg && strcasecmp(pszMD5Alg, "MD5") ) {
		MD5Update(&md5, (unsigned char*)":", 1);
		MD5Update(&md5, (unsigned char*)pszNonce, strlen(pszNonce));
		MD5Update(&md5, (unsigned char*)":", 1);
		MD5Update(&md5, (unsigned char*)pszCNonce, strlen(pszCNonce));
		MD5Final(bDigest, &md5);

		for( iPos = 0; iPos < 16; iPos++ )
			sprintf(szHA1 + iPos * 2, "%02x", bDigest[iPos]);

		MD5Init(&md5);
		MD5Update(&md5, (unsigned char*)szHA1, 32);
	}
	MD5Final(bDigest, &md5);
	for( iPos = 0; iPos < 16; iPos++ )
		sprintf(szHA1 + iPos * 2, "%02x", bDigest[iPos]);

	MD5Init(&md5);
	MD5Update(&md5, (unsigned char*)pszMethod, strlen(pszMethod));
	MD5Update(&md5, (unsigned char*)":", 1);
	MD5Update(&md5, (unsigned char*)pszDigestUri, strlen(pszDigestUri));
	if( *pszQop && strcasecmp(pszQop, "auth") ) {
		MD5Update(&md5, (unsigned char*)":", 1);
		MD5Update(&md5, (unsigned char*)"d41d8cd98f00b204e9800998ecf8427e", 32);
	}
	MD5Final(bDigest, &md5);
	for( iPos = 0; iPos < 16; iPos++ )
		sprintf(szHA2 + iPos * 2, "%02x", bDigest[iPos]);

	MD5Init(&md5);
	MD5Update(&md5, (unsigned char*)szHA1, 32);
	MD5Update(&md5, (unsigned char*)":", 1);
	MD5Update(&md5, (unsigned char*)pszNonce, strlen(pszNonce));
	MD5Update(&md5, (unsigned char*)":", 1);
	MD5Update(&md5, (unsigned char*)pszNC, strlen(pszNC));
	MD5Update(&md5, (unsigned char*)":", 1);
	MD5Update(&md5, (unsigned char*)pszCNonce, strlen(pszCNonce));
	MD5Update(&md5, (unsigned char*)":", 1);
	MD5Update(&md5, (unsigned char*)pszQop, strlen(pszQop));
	MD5Update(&md5, (unsigned char*)":", 1);
	MD5Update(&md5, (unsigned char*)szHA2, 32);
	MD5Final(bDigest, &md5);

	for( iPos = 0; iPos < 16; iPos++ )
		sprintf(pszResult + iPos * 2, "%02x", bDigest[iPos]);
}

void http_da_session_cleanup()
{
	struct http_session *s;
	time_t now = time(NULL);

	//MUTEX_LOCK(http_da_session_lock);
	int i;
	for (i = 0; i < HTTP_SESSION_MAX; i++) {
		s = &session_array[i];

		if (!s->in_use)
			continue;

		// not expired yet.
		if (s->modified + HTTP_DA_EXPIRY_TIME > now)
			continue;

		http_session_free(s);
	}
  //MUTEX_UNLOCK(http_da_session_lock);
}

void http_da_session_hardclean() {
    struct http_session *s;
    int i;
    
    for (i = 0; i < HTTP_SESSION_MAX; i++) {
    	s = &session_array[i];
    	http_session_free(s);
    	s->modified = 0;
    }
}

static struct http_session * http_da_session_start(const char *realm, const char *nonce, const char *opaque)
{

	struct http_session *session;
	time_t now = time(NULL);
	static int count = 0;

	if((count++ % 10) == 0) /* don't do this all the time to improve efficiency */
 		http_da_session_cleanup();

  	//MUTEX_LOCK(http_da_session_lock);

  	session = http_session_get();
	if (session)
  {
  	//session->next = http_da_session;
		session->modified = now;
		if (nonce)
			strncpy(session->nonce, nonce, sizeof(session->nonce));
		else
			http_da_calc_nonce(session->nonce);

		if (opaque)
			strncpy(session->opaque, opaque, sizeof(session->opaque));
		else
			http_da_calc_opaque(session->opaque);

		strncpy(session->realm,realm, sizeof(session->realm));
    		session->ncount = 0;
    		//http_da_session = session;
  	}

	return session;
  	//MUTEX_UNLOCK(http_da_session_lock);
}

static struct http_session * http_da_session_update(const char *realm, const char *nonce, const char *opaque, const char *cnonce, const char *ncount)
{
	int i;
	struct http_session *s;
#if VERIFY_OPAQUE
  	if (!realm || !nonce || !opaque || !cnonce || !ncount)
    		return 0;
#else
  	if (!realm || !nonce || !cnonce || !ncount)
    		return 0;
#endif
  //MUTEX_LOCK(http_da_session_lock);
	for (i = 0; i < HTTP_SESSION_MAX; i++)
	{
		s = &session_array[i];
    if (!s->in_use)
      continue;

		#if DBG_DIGEST
    	fprintf(stderr, "session nonce=%s; client resend nonce=%s\n", s->nonce, nonce);
		#endif

		#if VERIFY_OPAQUE
   		if (!strcmp(s->realm, realm) && !strcmp(s->nonce, nonce) && !strcmp(s->opaque, opaque))
   			break;
 		#else
   		if (!strcmp(s->realm, realm) && !strcmp(s->nonce, nonce))
   			break;
 		#endif
	}

	#if DBG_DIGEST
		fprintf(stderr, "session ncount=%d; client ncount=%d\n", s->ncount, strtoul(ncount, NULL, 16));
	#endif

	if (i < HTTP_SESSION_MAX)
	{
		unsigned long nc = strtoul(ncount, NULL, 16);

// Magician 01/14/2008
/* For non-sequential sending by FireFox, ignore ncount number comparison.
		if (s->ncount >= nc)
		{
			s->modified = 0;
			http_session_free(s);
		}
		else
		{
*/
			s->ncount = nc;
			s->modified = time(NULL);
//		}
		}
	else
		return 0;

	return s;
}

static void _auth_find_value(const char *input, const char * const token, char **ppVal, int *pSize)
{
	char del1[] = ",", del2[] = "=", *tmp, *tok, *value, *toks[11];
	char tmchr[] = "\"';`,";
	int i = 0;
	
	

	tmp = strdup(strstr(input, "username"));

	for( tok = strtok(tmp, del1); tok != NULL && i < 10; tok = strtok(NULL, del1), i++ )
		toks[i] = tok;

	toks[i] = NULL;

	for( i = 0; i < 11; i++ )
	{
		if( !toks[i] )
			break;

		if((tok = strtok(toks[i], del2)) && (value = strtok(NULL, del1)) && !strcmp(trim(tok), token))
		{
			value = trimEx(value, tmchr);
			*ppVal = value;
			*pSize = strlen(value);
			break;
		}
	}

	free(tmp);
}


// return length of the value. -1 if token is not in input.
static int auth_find_value(const char *input, const char * const token, char *buf, int len) {
	char *pc = NULL;
	int tmp;
	memset(buf, 0, len);
	_auth_find_value(input, token, &pc, &tmp);
	if (pc) {
		strncpy(buf, pc, tmp);
		return tmp;
	}
	return -1;
}


#endif

/*
 * Name: get_auth_hash_value
 *
 * Description: adds the ASCII values of the file letters
 * and mods by the hashtable size to get the hash value
 */
inline int get_auth_hash_value(char *dir)
{
#ifdef EMBED
	return 0;
#else
	unsigned int hash = 0;
	unsigned int index = 0;
	unsigned char c;

	hash = dir[index++];
	while ((c = dir[index++]) && c != '/')
		hash += (unsigned int) c;

	return hash % AUTH_HASHTABLE_SIZE;
#endif
}

/*
 * Name: auth_add
 *
 * Description: adds
 */
void * auth_add(char *directory, char *file)
{
	auth_dir *new_a, *old;

	old = auth_hashtable[get_auth_hash_value(directory)];
	while (old)
	{
		if (!strcmp(directory, old->directory))
			return 0;
		old = old->next;
	}

	new_a = (auth_dir *)malloc(sizeof(auth_dir));
	/* success of this call will be checked later... */
	new_a->authfile = fopen(file, "rt");
	new_a->directory = strdup(directory);
	new_a->dir_len = strlen(directory);
	new_a->next = auth_hashtable [get_auth_hash_value(directory)];
	auth_hashtable [get_auth_hash_value(directory)] = new_a;

	return (void *)new_a;
}

#if SUPPORT_AUTH_DIGEST
void auth_add_digest(char *directory, char *file, char *realm) {

	auth_dir *dir;
	dir = (auth_dir *)auth_add(directory,file);
	if (dir) {
		dir->authtype = HTTP_AUTH_DIGEST;
		dir->realm = strdup(realm);

		fprintf(stderr, "Added Digest: %s, %s, %s\n", directory, file, realm);
	}
}
#endif

void auth_check()
{
	int hash;
	auth_dir *cur;

	for (hash=0;hash<AUTH_HASHTABLE_SIZE;hash++)
	{
  	cur = auth_hashtable [hash];
	  while (cur)
		{
			if (!cur->authfile)
			{
				log_error_time();
				fprintf(stderr,"Authentication password file for %s not found!\n",
						cur->directory);
			}
			cur = cur->next;
		}
	}
}

#ifdef LDAP_HACK
#include <lber.h>
#include <ldap.h>
#include <sg_configdd.h>
#include <sg_confighelper.h>
#include <sg_users.h>

/* Return a positive, negative or not possible result to the LDAP auth for
 * the specified user.
 */
static int ldap_auth(const char *const user, const char *const pswd) {
	static time_t last;
	static char *prev_user;
	LDAP *ld;
	int ldap_ver, r;
	char f[256];
	ConfigHandleType *c = config_load(amazon_ldap_config_dd);

	/* Don't repeat query too often if the user name hasn't changed */
	if (last && prev_user &&
			time(NULL) < (last + config_get_int(c, AMAZON_LDAP_CACHET)) &&
			strcmp(prev_user, user) == 0) {
		config_free(c);
		last = time(NULL);
		return 1;
	}
	if (prev_user != NULL)   { free(prev_user);	prev_user = NULL;   }
	last = 0;

	if ((ld = ldap_init(config_get_string(c, AMAZON_LDAP_HOST),
			config_get_int(c, AMAZON_LDAP_PORT))) == NULL) {
		syslog(LOG_ERR, "unable to initialise LDAP");
		config_free(c);
		return 0;
	}
	if (ldap_set_option(ld, LDAP_OPT_REFERRALS, LDAP_OPT_OFF) != LDAP_SUCCESS) {
		syslog(LOG_ERR, "unable to set LDAP referrals off");
		config_free(c);
		ldap_unbind(ld);
		return 0;
	}
	ldap_ver = config_get_int(c, AMAZON_LDAP_VERSION);
	if (ldap_ver > 0 && ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &ldap_ver) != LDAP_SUCCESS) {
		syslog(LOG_ERR, "unable to set LDAP version %d", ldap_ver);
		config_free(c);
		ldap_unbind(ld);
		return 0;
	}
	snprintf(f, sizeof f, config_get_string(c, AMAZON_LDAP_BIND_DN), user);
	r = ldap_simple_bind_s(ld, f, pswd);
	if (r != LDAP_SUCCESS) {
		syslog(LOG_ERR, "unable to connect to LDAP (%s)", ldap_err2string(r));
		config_free(c);
		ldap_unbind(ld);
		return (r==LDAP_INVALID_CREDENTIALS)?-1:0;
	}
	config_free(c);
	ldap_unbind(ld);
	/* Caching timing details for next time through */
	prev_user = strdup(user);
	last = time(NULL);
	return 1;
}
static unsigned char ldap_succ;
#endif



/*
 * Name: auth_check_userpass
 *
 * Description: Checks user's password. Returns 0 when sucessful and password
 * 	is ok, else returns nonzero; As one-way function is used RSA's MD5 w/
 *  BASE64 encoding.
#ifdef EMBED
 * On embedded environments we use crypt(), instead of MD5.
#endif
 */
int auth_check_userpass(char *user, char *pass, FILE *authfile, request * req)
{
#ifdef LDAP_HACK

	/* Yeah, code before declarations will fail on older compilers... */
	switch (ldap_auth(user, pass)) {
	case -1:	ldap_succ = 0;				return 1;
	case 0:		ldap_succ = strcmp(user, "root")?0:1;	break;
	case 1:
		ldap_succ = 1;
		if (start_user_update(0) == 0)
			done_user_update(set_user_password(user, pass, 0)==0?1:0);
		return 0;
	}
#endif
#ifdef SHADOW_AUTH
	struct spwd *sp;

	sp = getspnam(user);
	if (!sp)
		return 2;

	if (!strcmp(crypt(pass, sp->sp_pwdp), sp->sp_pwdp))
		return 0;

#else

//#ifndef EMBED
#if 1

#ifdef EMBED
	char temps[0x100],*pwd;
	struct MD5Context mc;
 	unsigned char final[16];
	char encoded_passwd[0x40];
	// Added by Mason Yu (2 leval)
	FILE *fp;
	char disabled;

   	/* Encode password ('pass') using one-way function and then use base64
	encoding. */

	// Added by Mason Yu(2 level)
	//if ( strcmp("user", user)==0 && strcmp(req->request_uri, "/")==0 ) {

	// Added by Mason Yu for boa memory leak
	// User access web every time, the boa will auth once. And use strdup to allocate memory for directory_index.
	// So We should free the memory space of older directory_index to avoid memory leak.
	if (directory_index) free(directory_index);

	if ( strcmp(usName, user)==0
#ifdef ACCOUNT_CONFIG
		|| getAccPriv(user) == (int)PRIV_USER
#endif
	) {
		directory_index = strdup("/admin/index_user.html");

		if (strcmp(req->request_uri, "/")==0)
		{
			#ifdef WLAN_SUPPORT
		        // Get WLAN Interface status
				if(mib_get(MIB_WLAN_DISABLED, (void *)&disabled) == 0)
				{
					printf("Get WLAN interface flag error!\n");
					return 1;
				}

				if (disabled == 1)
				{
					printf("WLAN Interface is disabled !\n");
					return 1;
				}
				else
				{
			#endif
					fp = fopen("/var/boaUser.passwd", "r");
					authfile = fp;
			#ifdef WLAN_SUPPORT
				}
			#endif
		}
	}

	else
		directory_index = strdup("index.html");


	MD5Init(&mc);
	{
	//char *pass="admin";
	MD5Update(&mc, pass, strlen(pass));
	}
	MD5Final(final, &mc);
	strcpy(encoded_passwd,"$1$");
	base64encode(final, encoded_passwd+3, 16);

	DBG(printf("auth_check_userpass(%s,%s,...);\n",user,pass);)

	fseek(authfile, 0, SEEK_SET);
	while (fgets(temps,0x100,authfile))
	{
		if (temps[strlen(temps)-1]=='\n')
			temps[strlen(temps)-1] = 0;
		pwd = strchr(temps,':');
		if (pwd)
		{
			*pwd++=0;
			if (!strcmp(temps,user))
			{
				if (!strcmp(pwd,encoded_passwd)) {
					// Added by Mason Yu for web page via serverhost
					if ((strcmp(req->request_uri, "/")==0) && (!strcmp(usName, user)
					#ifdef ACCOUNT_CONFIG
						|| getAccPriv(user) == (int)PRIV_USER
					#endif
					))
						fclose(fp);
					return 0;
				}
			} else {
				// Modified by Mason Yu for multi user with passwd file.
				continue;
				//return 2;
			}
		}
	}

	// Added by Mason Yu for web page via serverhost
	if ((strcmp(req->request_uri, "/")==0) && (!strcmp(usName, user)
	#ifdef ACCOUNT_CONFIG
		|| getAccPriv(user) == (int)PRIV_USER
	#endif
	))
		fclose(fp);
#else
//printf("user=%s, pass=%s name=%s pass2=%s\n",user,pass,mgmtUserName(),mgmtPassword());
	//if((strcmp(mgmtUserName(),user)==0)&&(strcmp(mgmtPassword(),pass)==0)) return 0;
	//else return 2;
	return 0;
#endif

#else
	struct passwd *pwp;

	pwp = getpwnam(user);
	if (pwp != NULL) {
		if (strcmp(crypt(pass, pwp->pw_passwd), pwp->pw_passwd) == 0)
			return 0;
	} else
		return 2;
#endif	/* ! EMBED */
#endif	/* SHADOW_AUTH */
	return 1;
}

#ifdef ACCOUNT_LOGIN_CONTROL
//added by xl_yue
struct errlogin_entry * scan_errlog_list(request * req)
{
	struct errlogin_entry * perrlog;

	for(perrlog = errlogin_list;perrlog;perrlog = perrlog->next){
		if(!strcmp(perrlog->remote_ip_addr, req->remote_ip_addr))
			break;
	}

	return perrlog;
}
//added by xl_yue
int check_errlogin_list(request * req)
{
	struct errlogin_entry * perrlog;
	struct errlogin_entry * berrlog = NULL;

	for(perrlog = errlogin_list;perrlog;berrlog = perrlog,perrlog = perrlog->next){
		if(!strcmp(perrlog->remote_ip_addr, req->remote_ip_addr))
			break;
	}

	if(!perrlog)
		return 0;

	if(perrlog->login_count >= MAX_LOG_NUM){
		return -1;
	}
	else{//unlist and free
		if(perrlog == errlogin_list)
			errlogin_list = perrlog->next;
		else
			berrlog->next = perrlog->next;

		free(perrlog);
	}
	return 0;
}
#endif

static void Send_Unauthorized(auth_dir *current, request *req) {
#if SUPPORT_AUTH_DIGEST
	if (current->authtype == HTTP_AUTH_DIGEST) {
		struct http_session *s;
		if ((s = http_da_session_start(current->realm, 0, 0)) != NULL) {

			send_r_unauthorized_digest(req, s);
			return;
		}
		// no more session available
		fprintf(stderr, "No more session available\n");
		send_r_bad_request(req);
	}
	else
#endif
	send_r_unauthorized(req,server_name);
}

int auth_authorize(request * req)
{
	int i, denied = 1;
	auth_dir *current;
 	int hash;
	char *pwd, *method;
#ifndef ACCOUNT_LOGIN_CONTROL
	static char current_client[20];
#endif
	char auth_userpass[0x80];
	//added by xl_yue
#ifdef ACCOUNT_LOGIN_CONTROL
	struct account_info * current_account_info = NULL;
	struct errlogin_entry * err_login;
	int do_succeed_log = 0;
#endif

//xl_yue
#ifdef USE_LOGINWEB_OF_SERVER
	struct user_info * pUser_info;
#ifdef LOGIN_ERR_TIMES_LIMITED
	struct errlogin_entry * pErrlog_entry = NULL;
#endif
#endif

  DBG(printf("auth_authorize\n");)

	// Mason Yu
	//printf("auth_authorize(1): req->request_uri=%s\n", req->request_uri);
#ifdef BOA_UNAUTH_FILE
	for (i=0; i<MAX_UNAUTH_FILE; i++) {
		if ( strcmp(req->request_uri, UNAUTHFILE[i]) == 0 ) {
#ifndef CONFIG_GUI_WEB
			directory_index = strdup("/admin/index_user.html");
#endif
			//printf("This file need not auth!!\n");
			return 1;
		}
	}
#endif

	hash = get_auth_hash_value(req->request_uri);
  current = auth_hashtable[hash];

  while (current) {
  		if (!memcmp(req->request_uri, current->directory,
								current->dir_len)) {
			if (current->directory[current->dir_len - 1] != '/' &&
				        req->request_uri[current->dir_len] != '/' &&
								req->request_uri[current->dir_len] != '\0') {
				break;
			}
//xl_yue add
#ifndef USE_LOGINWEB_OF_SERVER
			if (req->authorization)
			{
				if (current->authfile==0)
				{
					send_r_error(req);
					return 0;
				}

			#if SUPPORT_AUTH_DIGEST
				if (!strncasecmp(req->authorization,"Digest ",7))
				{
					char username[33],realm[34],nonce[33],qop[33],alg[16];
					char cnonce[64],response[34],nc[33],opaque[33];
					char myresponse[33];
					struct http_session *session;

					auth_find_value(req->authorization + 7, "username", username, sizeof(username));
					auth_find_value(req->authorization + 7, "realm", realm, sizeof(realm));
					auth_find_value(req->authorization + 7, "nonce", nonce, sizeof(nonce));
					auth_find_value(req->authorization + 7, "qop", qop, sizeof(qop));
					auth_find_value(req->authorization + 7, "cnonce", cnonce, sizeof(cnonce));
					auth_find_value(req->authorization + 7, "response", response, sizeof(response));
					auth_find_value(req->authorization + 7, "opaque", opaque, sizeof(opaque));
					auth_find_value(req->authorization + 7, "nc", nc, sizeof(nc));
					auth_find_value(req->authorization + 7, "algorithm", alg, sizeof(alg));

					strncpy(auth_userpass, username, sizeof(auth_userpass) - 1);
					auth_userpass[sizeof(auth_userpass) - 1] = 0;

					#if DBG_DIGEST
						char uri[256];
						auth_find_value(req->authorization + 7, "uri", uri, sizeof(uri));
						fprintf(stderr,"Client inputs:\n");
						fprintf(stderr,"  username=%s\n", username);
						fprintf(stderr,"  realm=%s\n", realm);
						fprintf(stderr,"  nonce=%s\n", nonce);
						fprintf(stderr,"  qop=%s\n", qop);
						fprintf(stderr,"  cnonce=%s\n", cnonce);
						fprintf(stderr,"  response=%s\n", response);
						fprintf(stderr,"  opaque=%s\n", opaque);
						fprintf(stderr,"  uri=%s\n", uri);
						fprintf(stderr,"  nc=%s\n", nc);
						fprintf(stderr,"  alg=%s\n", alg);
					#endif

					if((session = http_da_session_update(realm, nonce, opaque, cnonce, nc)))
					{
						char tmpStr[64];
						char *str;

						if(!strcmp(username, usName))
						{
							fclose(current->authfile);
							current->authfile = fopen("/var/DigestUser.passwd", "r");

							if (directory_index)
								free(directory_index);

							directory_index = strdup("/admin/index_user.html");
						}
						else if(!strcmp(username, suName))
						{
							fclose(current->authfile);
							current->authfile = fopen("/var/DigestSuper.passwd", "r");

							if (directory_index)
								free(directory_index);

							directory_index = strdup("index.html");
						}

						#if DBG_DIGEST
							fprintf(stderr, "DEBUG: enter http_da_session_update session\n");
						#endif

						fseek(current->authfile, 0, SEEK_SET);
						while (fgets(tmpStr, sizeof(tmpStr), current->authfile))
						{
							str = trim(tmpStr);

							#if DBG_DIGEST
								fprintf(stderr, "DEBUG: tmpStr(username)=%s\n", tmpStr);
							#endif

							if (!strcmp(username, tmpStr))
							{
								fgets(tmpStr, sizeof(tmpStr),current->authfile);
								str = trim(tmpStr);

								#if DBG_DIGEST
									fprintf(stderr, "DEBUG: tmpStr(password)=%s\n", tmpStr);
								#endif

								switch(req->method)
								{
									case M_GET:
										method = strdup("GET");
										break;
									case M_HEAD:
										method = strdup("HEAD");
										break;
									case M_PUT:
										method = strdup("PUT");
										break;
									case M_POST:
										method = strdup("POST");
										break;
									case M_DELETE:
										method = strdup("DELETE");
										break;
									case M_LINK:
										method = strdup("LINK");
										break;
									case M_UNLINK:
										method = strdup("UNLINK");
										break;
									default:
										method = strdup("");
								}
								
								CalcDigestMD5(username, session->realm, str, method, req->request_uri, session->nonce, nc, cnonce, qop, alg, myresponse);

								#if DBG_DIGEST
									fprintf(stderr, "DEBUG:   reponse=%s\n", response);
									fprintf(stderr, "DEBUG: myreponse=%s\n\n", myresponse);
								#endif

								free(method);

								if(!strcmp(response, myresponse))
								{
									denied = 0;
									break;
								}
							}
							else
								fgets(tmpStr, sizeof(tmpStr),current->authfile);
						}
					}
				}
				else
				{
			#endif  // SUPPORT_AUTH_DIGEST

					if (strncasecmp(req->authorization,"Basic ",6))
					{
						syslog(LOG_ERR, "Can only handle Basic auth\n");
						send_r_bad_request(req);
						return 0;
					}

					#if SUPPORT_AUTH_DIGEST
					if (current->authtype == HTTP_AUTH_DIGEST)
					{
						Send_Unauthorized(current, req);
						return 0;
					}
					#endif

					base64decode(auth_userpass, req->authorization+6, sizeof(auth_userpass));

					if ( (pwd = strchr(auth_userpass,':')) == 0 )
					{
						syslog(LOG_ERR, "No user:pass in Basic auth\n");
						send_r_bad_request(req);
						return 0;
					}

					*pwd++=0;
					// Modified by Mason Yu
					//denied = auth_check_userpass(auth_userpass,pwd,current->authfile);

					denied = auth_check_userpass(auth_userpass,pwd,current->authfile, req);
			#if SUPPORT_AUTH_DIGEST
				} // digest check
			#endif
#ifdef SECURITY_COUNTS
				if (strncmp(get_mime_type(req->request_uri),"image/",6))
					access__attempted(denied, auth_userpass);
#endif

#ifdef ACCOUNT_LOGIN_CONTROL
				//added by xl_yue
				if(!denied){
					//check if this ip in errlogin list
					if(check_errlogin_list(req)){
						denied = 5;
						goto pass_check;
					}

					//for user
					if (strcmp(usName, auth_userpass)==0){
						current_account_info = &us_account;
//						printf("select us_account\n");
					}
					//for super user
					else if(strcmp(suName, auth_userpass)==0){
						current_account_info = &su_account;
//						printf("select su_account\n");
					}
					else{
						current_account_info = NULL;
//						printf("select none account\n");
					}

					if(current_account_info){
						if (strcmp(current_account_info->remote_ip_addr, req->remote_ip_addr) == 0){
//							printf("the same ip\n");
							if(current_account_info->account_timeout){			//need login again for no acction for 5 minutes
								current_account_info->account_timeout = 0;
								current_account_info->last_time = time_counter;
								denied = 3;
//								printf("account timeout\n");
							}else{
//								printf("account not timeout\n");
								current_account_info->last_time = time_counter;
								current_account_info->account_busy = 1;
								//req->paccount_info = current_account_info;
								//current_account_info->refcnt++;
							}
						}else{
//							printf("not same ip\n");
							//only one user can login using the same account at the same time
							if(current_account_info->account_busy){
//								printf("account busy\n");
								denied = 4;
							}else{
//								printf("account not busy\n");
								current_account_info->account_timeout = 0;
								current_account_info->last_time = time_counter;
								current_account_info->account_busy = 1;
								do_succeed_log = 1;
								strncpy(current_account_info->remote_ip_addr, req->remote_ip_addr, sizeof(current_account_info->remote_ip_addr));
								//req->paccount_info = current_account_info;
								//current_account_info->refcnt++;
							}
						}
					}
				}else{
					/*login error three times,should wait 1 minutes */
					if((err_login = scan_errlog_list(req)) == NULL){
						err_login = (struct errlogin_entry*) malloc(sizeof(struct errlogin_entry));
						if (!err_login)
							die(OUT_OF_MEMORY);
						err_login->login_count = 0;
						strncpy(err_login->remote_ip_addr, req->remote_ip_addr, sizeof(err_login->remote_ip_addr));
						//list err_login to errlogin_list
						err_login->next = errlogin_list;
						errlogin_list = err_login;
					}
					if((++(err_login->login_count)) >= MAX_LOG_NUM){
						//if have login,then exit
						if(!strcmp(su_account.remote_ip_addr, req->remote_ip_addr)){
							su_account.account_timeout = 0;
							su_account.account_busy = 0;
							su_account.remote_ip_addr[0] = '\0';
						}
						if(!strcmp(us_account.remote_ip_addr, req->remote_ip_addr)){
							us_account.account_timeout = 0;
							us_account.account_busy = 0;
							us_account.remote_ip_addr[0] = '\0';
						}

						if(err_login->login_count == MAX_LOG_NUM)
							err_login->last_time = time_counter;

						denied = 5;
					}else{
						err_login->last_time = time_counter;
					}
				}
#endif

pass_check:
				if (denied) {
					switch (denied) {
						case 1:
							syslog(LOG_ERR, "Authentication attempt failed for %s from %s because: Bad Password\n", auth_userpass, req->remote_ip_addr);
							#if DBG_DIGEST
								fprintf(stderr, "DEBUG %d: Authentication attempt failed for %s from %s because: Bad Password\n", denied, auth_userpass, req->remote_ip_addr);
							#endif
							break;
						case 2:
							syslog(LOG_ERR, "Authentication attempt failed for %s from %s because: Invalid Username\n",	auth_userpass, req->remote_ip_addr);
							#if DBG_DIGEST
								fprintf(stderr, "DEBUG %d: Authentication attempt failed for %s from %s because: Invalid Username\n",	denied, auth_userpass, req->remote_ip_addr);
							#endif
							break;
#ifdef ACCOUNT_LOGIN_CONTROL
						//added by xl_yue
						case 3:
							syslog(LOG_ERR, "Authentication attempt failed for %s from %s because:login has timeouted \n", auth_userpass, req->remote_ip_addr);
							#if DBG_DIGEST
								fprintf(stderr, "DEBUG %d: Authentication attempt failed for %s from %s because:login has timeouted \n", denied, auth_userpass, req->remote_ip_addr);
							#endif
							break;
						//added by xl_yue
						case 4:
							syslog(LOG_ERR, "Authentication attempt failed for %s from %s because: another has logined\n", auth_userpass, req->remote_ip_addr);
							#if DBG_DIGEST
								fprintf(stderr, "DEBUG %d: Authentication attempt failed for %s from %s because: another has logined\n", denied, auth_userpass, req->remote_ip_addr);
							#endif
							send_r_forbidden(req);
							return 0;
							break;
						//added by xl_yue
						case 5:
							syslog(LOG_ERR, "Authentication attempt failed for %s from %s because: have logined error for three times\n", auth_userpass, req->remote_ip_addr);
							#if DBG_DIGEST
								fprintf(stderr, "DEBUG %d: Authentication attempt failed for %s from %s because: have logined error for three times\n", denied, auth_userpass, req->remote_ip_addr);
							#endif
							send_r_forbidden(req);
							return 0;
							break;
#endif
						}
#ifndef ACCOUNT_LOGIN_CONTROL
					bzero(current_client, sizeof(current_client));
#endif
					Send_Unauthorized(current, req);
					return 0;


				}
#ifdef ACCOUNT_LOGIN_CONTROL
				if(do_succeed_log){
					syslog(LOG_INFO, "Authentication successful for %s from %s\n", auth_userpass, req->remote_ip_addr);
				}
#else
				if (strcmp(current_client, req->remote_ip_addr) != 0) {
					strncpy(current_client, req->remote_ip_addr, sizeof(current_client));
					syslog(LOG_INFO, "Authentication successful for %s from %s\n", auth_userpass, req->remote_ip_addr);
				}
#endif
				/* Copy user's name to request structure */
#ifdef LDAP_HACK
				if (!ldap_succ) {
					strcpy(req->user, "noldap");
					syslog(LOG_INFO, "Access granted as noldap");
				} else
#endif
				strncpy(req->user, auth_userpass, 15);
				req->user[15] = '\0';
				return 1;
			}else
			{
				/* No credentials were supplied. Tell them that some are required */
#ifdef ACCOUNT_LOGIN_CONTROL
				//added by xl_yue:if have login error for three times, forbid logining until 1 minute later
				if((err_login = scan_errlog_list(req)) != NULL){
					if(err_login->login_count >= MAX_LOG_NUM){
						send_r_forbidden(req);
						return 0;
					}
				}
#endif
				Send_Unauthorized(current, req);
				return 0;
			}
#else
//xl_yue
				pUser_info = search_login_list(req);
				if(pUser_info){
					//user account can not access admin account directory
					if((strcmp(req->request_uri,"/")) && (!strcmp(current->directory,"/")) && (!strcmp(pUser_info->directory,"/admin/index_user.html"))){
						send_r_forbidden(req);
						return 0;
					}

					directory_index = pUser_info->directory;
					pUser_info->last_time = time_counter;
					return 1;
				}else{
#ifdef LOGIN_ERR_TIMES_LIMITED
					pErrlog_entry = search_errlog_list(req);
					if(pErrlog_entry){
						if(pErrlog_entry->login_count >= MAX_LOGIN_NUM){
							send_r_forbidden(req);
							return 0;
						}
					}
#endif
					if(!strcmp(req->request_uri,"/admin/login.asp")
						|| !strcmp(req->request_uri,"/code.asp")
						|| !strcmp(req->request_uri,"/admin/code_user.asp")
						|| !strcmp(req->request_uri,"/goform/admin/formLogin")
						|| !strcmp(req->request_uri,"/admin/LoginFiles/realtek.jpg")
						|| !strcmp(req->request_uri,"/admin/LoginFiles/locker.gif"))
						return 1;

					directory_index = NULL;

					if(strcmp(req->request_uri,"/")){
/*
						req->response_status = R_FORBIDDEN;
						req_write(req, "<HTML><HEAD><TITLE>Forbidden</TITLE></HEAD>\n <BODY><H1>Forbidden </H1>\n");
						req_write(req, " Your requested URL is forbidden to access, maybe because you have not logined.\n");
 						req_write(req, " Please close this window and reopen your web browser to login.\n</BODY></HTML>\n");
						req_flush(req);
*/
						req->buffer_end=0;
						req->response_status = R_MOVED_PERM;
						req_write(req, "<HTML><HEAD><TITLE>Login</TITLE></HEAD>\n"
									 "<BODY><BLOCKQUOTE>\n <h2><font color=\"#0000FF\">Login</font></h2>"
									 "You have not logined,please\n");
						req_write(req, "<A HREF=\"/admin/login.asp\" target=_top>login</A>.\n</BLOCKQUOTE></BODY></HTML>\n");
						req_flush(req);
						return 0;
					}

					websRedirect(req, "/admin/login.asp");
					return 0;
				}
#endif
		}
	  current = current->next;
  }

	return 1;
}

void dump_auth(void)
{
	int i;
	auth_dir *temp;

	for (i = 0; i < AUTH_HASHTABLE_SIZE; ++i) {
		if (auth_hashtable[i]) {
			temp = auth_hashtable[i];
			while (temp) {
				auth_dir *temp_next;

				if (temp->directory)
					free(temp->directory);
				if (temp->authfile)
					fclose(temp->authfile);
				temp_next = temp->next;
				free(temp);
				temp = temp_next;
			}
			auth_hashtable[i] = NULL;
		}
	}
}

#endif
