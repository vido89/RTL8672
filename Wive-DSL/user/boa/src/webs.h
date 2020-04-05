#ifndef _WEBS_H_
#define _WEBS_H_


#include "asp_page.h"
#include <arpa/inet.h>
#include <time.h>
#include <errno.h>
#include	<sys/param.h>
#include <sys/stat.h>
#define gatoi		atoi
#define gstrcmp	strcmp
#define gstrchr		strchr
#define gstrtok		strtok
#define gstrlen   strlen
#define gstrcpy   strcpy
#define bstrdup(B_ARGS, s) bstrdupNoBalloc(s)
#define char_t char
#define gisspace	isspace

//typedef request* webs_t;
#define webs_t request *
typedef short bool_t;

#define E_L					T(__FILE__), __LINE__
#define B_L				T(__FILE__), __LINE__
#define	E_ASSERT			0x1			/* Assertion error */
#define	E_LOG				0x2			/* Log error to log file */
#define	E_USER				0x3			/* Error that must be displayed */

int websWrite(request* wp, char_t *fmt, ...);
extern char *websGetVar(request *req, char *var, char *defaultGetValue);


/*
 *	Privilege Masks
 */
#define PRIV_NONE	0x00
#define PRIV_READ	0x01
#define PRIV_WRITE	0x02
#define PRIV_ADMIN	0x04

typedef enum {
	AM_NONE = 0,
	AM_FULL,
	AM_BASIC,
	AM_DIGEST,
	AM_INVALID
} accessMeth_t;

/* Add by Dick Tam, for firmware update */
#define MAX_FIRMWARE_FILE_SIZE				(2097152)	// 2M
#define WEB_UPLOAD_PAGE_SIZE					(32768-512)		// 32 KB, malloc header size is 8
//#define WEB_UPLOAD_PAGE_SIZE					(16384-512)		// 16 KB
#define MAX_UPLOAD_BLOCK_NUM					(MAX_FIRMWARE_FILE_SIZE/WEB_UPLOAD_PAGE_SIZE)
#define REMOTE_PASS_LIFE	1200	// seconds
#endif // _WEBS_H_


#define MAX_SEND_SIZE	80
#define MQ_CREATE	0
#define MQ_GET		1
#define MSG_SUCC	0
#define MSG_FAIL	1

struct mymsgbuf {
        long mtype;			// Message type
        long request;			// Request ID/Status code
        char mtext[MAX_SEND_SIZE];	// Client/server information
};
