/*
 * msgq.h -- System V Message Queue Framework Header
 * --- By Kaohj
 */

#ifndef _h_MSGQ
#define _h_MSGQ 1

#include <sys/types.h>
#ifdef EMBED
#include <config/autoconf.h>
#else
#include "../../../config/autoconf.h"
#endif
//ql
#include "LINUX/options.h"

#ifdef VOIP_SUPPORT
#define MAX_SEND_SIZE	(7*1024)
#else /*VOIP_SUPPORT*/
#define MAX_SEND_SIZE	4096
#endif /*VOIP_SUPPORT*/
#define BOA_MAX_SEND_SIZE	5000

#define MQ_CREATE	0
#define MQ_GET		1
#define MSG_SUCC	0
#define MSG_FAIL	1
#define MSG_MIB_LOCKED	2
#define KILL_PROCESS_OFF 0
#define KILL_PROCESS_ON  1

/* Command Type */
#define		CMD_START		0
#define		CMD_MIB_LOCK		1
#define		CMD_MIB_UNLOCK		2
#define		CMD_MIB_UPDATE		3
#define		CMD_MIB_GET		4
#define		CMD_MIB_SET		5
#define		CMD_MIB_RELOAD		6
#define		CMD_MIB_SIZE		7
#define		CMD_MIB_INFO_ID		8
#define		CMD_MIB_INFO_INDEX	9
#define		CMD_MIB_BACKUP		10
#define		CMD_MIB_RESTORE		11
#define		CMD_MIB_GET_DEFAULT	12
#define		CMD_MIB_INFO_TOTAL   13
#define		CMD_CHAIN_TOTAL		15
#define		CMD_CHAIN_GET		16
#define		CMD_CHAIN_ADD		17
#define		CMD_CHAIN_DELETE	18
#define		CMD_CHAIN_CLEAR		19
#define		CMD_CHAIN_UPDATE	20
#define		CMD_CHAIN_INFO_ID	21
#define		CMD_CHAIN_INFO_INDEX	22
#define		CMD_CHAIN_INFO_NAME	23
#define		CMD_CHAIN_DESC_ID	24

#define		CMD_REBOOT		30
#define		CMD_GET_BOOT_MODE	31
#define		CMD_UPLOAD		32
#define		CMD_KILLPROC		33
#define 	CMD_CHECK_IMAGE    	34
#ifdef CONFIG_USER_DDNS
#define 	CMD_DDNS_CTRL		35
#endif
#define		CMD_UPGRADE_MODE	36

#ifdef CONFIG_USER_PPPOE_PROXY
#define           CMD_ADD_POLICY_TABLE	37
#define           CMD_DEL_POLICY_TABLE	38
#define           CMD_ADD_POLICY_RULE	39
#define           CMD_DEL_POLICY_RULE	40
#define           CMD_NO_ADSLLINK_PPP	41
#define           CMD_POLL_SWITCH_PORT	42
#endif
//#ifdef	RESERVE_KEY_SETTING
#define		CMD_MIB_RETRIVE_TABLE	43
#define		CMD_MIB_RETRIVE_CHAIN	44
#define		CMD_MIB_CLEAR		45
//#endif
#define		CMD_START_AUTOHUNT	47
#define		CMD_FILE2XML		48
#define		CMD_XML2FILE		49
#define		CMD_MIB_SET_FLASH	50
#define		CMD_CHAIN_ADD_FLASH	51
#define		CMD_END			52



typedef struct msgInfo {
	int	cmd;
	int	arg1;
	int	arg2;
	char	mtext[MAX_SEND_SIZE];
} MSG_T;

struct mymsgbuf {
        long mtype;			// Message type
        long request;			// Request ID/Status code
	MSG_T msg;
};

typedef struct twoFile {
	char fromName[32];
	char toName[32];
} MSGFile_T;

typedef struct boaMsgInfo {
	int	cmd;
	int	arg1;
	int	arg2;
	char	mtext[BOA_MAX_SEND_SIZE];
} BOA_MSG_T;

struct boamsgbuf {
        long mtype;			// Message type
        long request;			// Request ID/Status code
	BOA_MSG_T msg;
};
extern void send_message_boa(int qid, long type, long req, BOA_MSG_T *msg);

extern int	open_queue( key_t keyval, int flag );
extern void	send_message(int qid, long type, long req, MSG_T *msg);
extern void	read_message(int qid, struct mymsgbuf *qbuf, long type);
extern int	peek_message(int qid, long type);
extern void	remove_queue(int qid);

#endif /* _h_MSGQ */
