/*
 * sysconfig.c --- main file for configuration server API
 * --- By Kaohj
 */

#include "sysconfig.h"
#include "msgq.h"
#include "mibtbl.h"
#include <stdio.h>
#include <signal.h>
#ifdef EMBED
#include <config/autoconf.h>
#else
#include "../../../../config/autoconf.h"
#endif

#define CONF_SERVER_PIDFILE	"/var/run/configd.pid"

static void sendcmd(struct mymsgbuf* qbuf)
{
	key_t key;
	int   qid, cpid, spid;
	FILE *spidfile;
	
#ifdef EMBED
	/* Create unique key via call to ftok() */
	key = ftok("/bin/init", 'k');
	if ((qid = open_queue(key, MQ_GET)) == -1) {
		perror("open_queue");
		return;
	}
	
	// get client pid
	cpid = (int)getpid();
	
	// get server pid
	if ((spidfile = fopen(CONF_SERVER_PIDFILE, "r"))) {
		fscanf(spidfile, "%d\n", &spid);
		fclose(spidfile);
	}
	else
		printf("server pidfile not exists\n");
	
	send_message(qid, spid, cpid, &qbuf->msg);
	while (!peek_message(qid, cpid));
	read_message(qid, qbuf, cpid);
#else
	memset(qbuf, 0, sizeof(struct mymsgbuf));
	qbuf->request = MSG_SUCC;
	return;
#endif	
}

int mib_lock()
{
        struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int k;
	int ret=1;
	
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_MIB_LOCK;
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("mib lock failed\n");
		ret = 0;
	}
	
	return ret;
}

int mib_unlock()
{
        struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int k;
	int ret=1;
	
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_MIB_UNLOCK;
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("mib lock failed\n");
		ret = 0;
	}
	
	return ret;
}

int mib_update(CONFIG_DATA_T type, CONFIG_MIB_T flag)
{
        struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int k;
	int ret=1;
	char vChar;
	
#ifdef CLOSE_ITF_BEFORE_WRITE
#ifdef WLAN_SUPPORT
	vChar=0;
	mib_get(MIB_WLAN_DISABLED, (void *)&vChar);
	if (!vChar)
		stopwlan();
#endif
#endif /* CLOSE_ITF_BEFORE_WRITE*/
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_MIB_UPDATE;
	mymsg->arg1=(int)type;
	mymsg->arg2=(int)flag;
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("mib update failed\n");
		ret = 0;
	}
	
#ifdef CLOSE_ITF_BEFORE_WRITE
#ifdef WLAN_SUPPORT
	if (!vChar)
		startWLan();
#endif //WLAN_SUPPORT
#endif /* CLOSE_ITF_BEFORE_WRITE*/
	return ret;
}

int mib_load(CONFIG_DATA_T type, CONFIG_MIB_T flag)
{
        struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int k;
	int ret=1;
	
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_MIB_RELOAD;
	mymsg->arg1=(int)type;
	mymsg->arg2=(int)flag;
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("mib reload failed\n");
		ret = 0;
	}
	
	return ret;
}

int mib_get(int id, void *value)
{
        struct mymsgbuf qbuf;
	MSG_T *mymsg;
	mib_table_entry_T info;
	int size;
	int ret=1;
        
        if (!mib_info_id(id, &info))
        	return 0;
        
        size = info.size;
        
        mymsg = &qbuf.msg;
        mymsg->cmd = CMD_MIB_GET;
	mymsg->arg1=id;
	sendcmd(&qbuf);
	if (qbuf.request == MSG_SUCC) {
		memcpy(value, mymsg->mtext, size);
		ret = 1;
	}
	else {
		printf("get request failed\n");
		ret = 0;
	}
	return ret;
}

int mib_set(int id, void *value)
{
        struct mymsgbuf qbuf;
	MSG_T *mymsg;
	mib_table_entry_T info;
	int size;
	int ret=1;
        
        if (!mib_info_id(id, &info))
        	return 0;
        
        size = info.size;

        mymsg = &qbuf.msg;
        mymsg->cmd = CMD_MIB_SET;
	mymsg->arg1=id;
	
	memcpy(mymsg->mtext, value, size);
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("set request failed\n");
		ret = 0;
	}
	return ret;
}

int mib_set_flash(int id, void *value)
{
	struct mymsgbuf qbuf;
	MSG_T *mymsg;
	mib_table_entry_T info;
	int size;
	int ret=1;
	char vChar;
	
	if (!mib_info_id(id, &info))
		return 0;
        
#ifdef CLOSE_ITF_BEFORE_WRITE
#ifdef WLAN_SUPPORT
	vChar=0;
	mib_get(MIB_WLAN_DISABLED, (void *)&vChar);
	if (!vChar)
		stopwlan();
#endif
#endif /* CLOSE_ITF_BEFORE_WRITE*/
	size = info.size;

	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_MIB_SET_FLASH;
	mymsg->arg1=id;
	
	memcpy(mymsg->mtext, value, size);
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("set request failed\n");
		ret = 0;
	}
#ifdef CLOSE_ITF_BEFORE_WRITE
#ifdef WLAN_SUPPORT
	if (!vChar)
		startWLan();
#endif //WLAN_SUPPORT
#endif /* CLOSE_ITF_BEFORE_WRITE*/
	return ret;
}

//#ifdef MSG_WLAN_RESTART_SUPPORT
//static void sendCmd2Boa(struct mymsgbuf* qbuf)
static void sendCmd2Boa(struct boamsgbuf* qbuf)
{
	key_t key;
	int   qid, cpid, spid;
	FILE *spidfile;
	
	/* Create unique key via call to ftok() */
	key = ftok("/bin/init", 'w');
	if ((qid = open_queue(key, MQ_GET)) == -1) {
		perror("open_queue");
		printf("open boa_msg_queue error\n");
		return;
	}
	
	// get client pid
	cpid = (int)getpid();
	
	// get server pid
	if ((spidfile = fopen("/var/run/boa.pid", "r"))) {
		fscanf(spidfile, "%d\n", &spid);
		fclose(spidfile);
	}
	else
		printf("server pidfile not exists\n");
	
//	send_message(qid, spid, cpid, &qbuf->msg);
	send_message_boa(qid, spid, cpid, &qbuf->msg);

//	while (!peek_message(qid, cpid));
//	read_message(qid, qbuf, cpid);
}

int sendMsg2Boa(int cmd, void *value, unsigned int length)
{
//       struct mymsgbuf qbuf;
//	MSG_T *mymsg;
	struct boamsgbuf qbuf;
	BOA_MSG_T *mymsg;
	int ret=1;

        mymsg = &qbuf.msg;
        mymsg->cmd = cmd;
        if (value)
        	memcpy(mymsg->mtext, value, length);
	sendCmd2Boa(&qbuf);
	return ret;
}
//#endif

int mib_info_id(int id, mib_table_entry_T *info)
{
        struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int k;
	int ret=1;
	
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_MIB_INFO_ID;
	mymsg->arg1=id;
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("get mib info id failed\n");
		ret = 0;
	}
	else
		memcpy((void *)info, (void *)mymsg->mtext, sizeof(mib_table_entry_T));
	
	return ret;
}

int mib_getDef(int id, void *value)
{
        struct mymsgbuf qbuf;
	MSG_T *mymsg;
	mib_table_entry_T info;
	int size;
	int ret=1;
        
        if (!mib_info_id(id, &info))
        	return 0;
        
        size = info.size;
        
        mymsg = &qbuf.msg;
        mymsg->cmd = CMD_MIB_GET_DEFAULT;
	mymsg->arg1=id;
	sendcmd(&qbuf);
	if (qbuf.request == MSG_SUCC) {
		memcpy(value, mymsg->mtext, size);
		ret = 1;
	}
	else {
		printf("get request failed\n");
		ret = 0;
	}
	return ret;
}

int mib_info_index(int index, mib_table_entry_T *info)
{
        struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int k;
	int ret=1;
	
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_MIB_INFO_INDEX;
	mymsg->arg1=index;
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("get mib info index %d failed\n",index);
		ret = 0;
	}
	else
		memcpy((void *)info, (void *)mymsg->mtext, sizeof(mib_table_entry_T));
	
	return ret;
}

// Apply Star Zhang's fast load
int mib_info_total()
{
	struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int k;
	int ret=1;
	
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_MIB_INFO_TOTAL;
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("get mib size failed\n");
		ret = 0;
	}
	else
		ret = qbuf.msg.arg1;
	
	return ret;
}
// The end of fast load

/* 
 * type:
 * CONFIG_MIB_ALL:   all mib setting (table and chain)
 * CONFIG_MIB_TABLE: mib table
 * CONFIG_MIB_CHAIN: mib_chain
 */
int mib_backup(CONFIG_MIB_T type)
{
        struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int k;
	int ret=1;
	
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_MIB_BACKUP;
	mymsg->arg1=(int)type;
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("mib backup table failed\n");
		ret = 0;
	}else
		printf("mib backup table success\n");
	
	return ret;
}

//added by ql
#ifdef	RESERVE_KEY_SETTING
int mib_retrive_table(int id)
{
        struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int k;
	int ret=1;
	
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_MIB_RETRIVE_TABLE;
	mymsg->arg1= id;
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("mib retrieve table failed\n");
		ret = 0;
	}
	
	return ret;
}
int mib_retrive_chain(int id)
{
        struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int k;
	int ret=1;
	
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_MIB_RETRIVE_CHAIN;
	mymsg->arg1= id;
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("mib retrieve chain failed\n");
		ret = 0;
	}
	
	return ret;
}
int mib_clear(CONFIG_MIB_T type)
{
        struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int k;
	int ret=1;
	
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_MIB_CLEAR;
	mymsg->arg1= (int)type;;
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("mib backup table failed\n");
		ret = 0;
	}
	
	return ret;
}
#endif
/* 
 * type:
 * CONFIG_MIB_ALL:   all mib setting (table and chain)
 * CONFIG_MIB_TABLE: mib table
 * CONFIG_MIB_CHAIN: mib_chain
 */
int mib_restore(CONFIG_MIB_T type)
{
        struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int k;
	int ret=1;
	
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_MIB_RESTORE;
	mymsg->arg1=(int)type;
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("mib restore table failed\n");
		ret = 0;
	}
	
	return ret;
}

int mib_chain_total(int id)
{
        struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int k;
	int ret=1;
	
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_CHAIN_TOTAL;
	mymsg->arg1=id;
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("get total failed\n");
		ret = 0;
	}
	else
		ret = qbuf.msg.arg1;
	
	return ret;
}

int mib_chain_get(int id, unsigned int recordNum, void *value)
{
        struct mymsgbuf qbuf;
	MSG_T *mymsg;
	mib_chain_record_table_entry_T info;
	int ret;
	int size;
	
	if (!mib_chain_info_id(id, &info))
		return 0;
	
	size = info.per_record_size;
	if (size >= MAX_SEND_SIZE) {
		printf("chain_get: chain record size(%d) overflow (max. %d).\n", size, MAX_SEND_SIZE);
		return 0;
	}
	
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_CHAIN_GET;
	mymsg->arg1=id;
	sprintf(mymsg->mtext, "%d", recordNum);
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("get chain failed\n");
		ret = 0;
	}
	else {
		memcpy(value, mymsg->mtext, size);
		ret = 1;
	}
	return ret;
}

/*
 * 0  : add fail
 * -1 : table full
 * 1  : successful
 */
int mib_chain_add(int id, void* ptr)
{
        struct mymsgbuf qbuf;
	MSG_T *mymsg;
	mib_chain_record_table_entry_T info;
	int ret;
	int size;
	
	ret = 1;
	if (!mib_chain_info_id(id, &info))
		return 0;
	
	size = mib_chain_total(id);
	if (info.table_size != -1 && size >= info.table_size)
		return -1;
	size = info.per_record_size;
	if (size >= MAX_SEND_SIZE) {
		printf("chain_add: chain record size(%d) overflow (max. %d).\n", size, MAX_SEND_SIZE);
		return 0;
	}
	
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_CHAIN_ADD;
	mymsg->arg1=id;
	memcpy(mymsg->mtext, ptr, size);
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("add chain failed\n");
		ret = 0;
	}
	
	return ret;
}

int mib_chain_delete(int id, unsigned int recordNum)
{
        struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int ret;
	
	ret = 1;
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_CHAIN_DELETE;
	mymsg->arg1=id;
	sprintf(mymsg->mtext, "%d", recordNum);
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("delete chain failed\n");
		ret = 0;
	}
	
	return ret;
}

int mib_chain_clear(int id)
{
        struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int ret;
	
	ret = 1;
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_CHAIN_CLEAR;
	mymsg->arg1=id;
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("clear chain failed\n");
		ret = 0;
	}
	
	return ret;
}

int mib_chain_update(int id, void* ptr, unsigned int recordNum)
{
        struct mymsgbuf qbuf;
	MSG_T *mymsg;
	mib_chain_record_table_entry_T info;
	int ret;
	int size;
	
	ret = 1;
	if (!mib_chain_info_id(id, &info))
		return 0;
	
	size = info.per_record_size;
	if (size >= MAX_SEND_SIZE) {
		printf("chain_update: chain record size(%d) overflow (max. %d).\n", size, MAX_SEND_SIZE);
		return 0;
	}
	
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_CHAIN_UPDATE;
	mymsg->arg1=id;
	mymsg->arg2=recordNum;
	memcpy(mymsg->mtext, ptr, size);
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("update chain failed\n");
		ret = 0;
	}
	
	return ret;
}

int mib_chain_info_id(int id, mib_chain_record_table_entry_T *info)
{
        struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int ret=1;
	
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_CHAIN_INFO_ID;
	mymsg->arg1=id;
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("get mib chain info id failed\n");
		ret = 0;
	}
	else
		memcpy((void *)info, (void *)mymsg->mtext, sizeof(mib_chain_record_table_entry_T));
	
	return ret;
}

int mib_chain_info_index(int index, mib_chain_record_table_entry_T *info)
{
        struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int ret=1;
	
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_CHAIN_INFO_INDEX;
	mymsg->arg1=index;
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("get mib chain info index failed\n");
		ret = 0;
	}
	else
		memcpy((void *)info, (void *)mymsg->mtext, sizeof(mib_chain_record_table_entry_T));
	
	return ret;
}

int mib_chain_info_name(char *name, mib_chain_record_table_entry_T *info)
{
	struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int ret=1;
	
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_CHAIN_INFO_NAME;
	strncpy(mymsg->mtext, name, MAX_SEND_SIZE);
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("get mib chain info name failed\n");
		ret = 0;
	}
	else
		memcpy((void *)info, (void *)mymsg->mtext, sizeof(mib_chain_record_table_entry_T));
	
	return ret;
}

int mib_chain_desc_id(int id, mib_chain_member_entry_T *desc)
{
	struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int ret=1;
	
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_CHAIN_DESC_ID;
	mymsg->arg1=id;
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("get mib chain desc id failed\n");
		ret = 0;
	}
	else
	{
		memcpy((void *)desc, (void *)mymsg->mtext, sizeof(mib_chain_member_entry_T)*mymsg->arg1);
	}
	
	return ret;
}

int cmd_reboot()
{
       struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int ret=1;
	
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_REBOOT;
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("reboot failed\n");
		ret = 0;
	}
	
	return ret;
}

int mib_get_bootmode()
{
        struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int ret=1;
	
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_GET_BOOT_MODE;
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("get mib bootmode failed\n");
		ret = 0;
	}
	else
		ret = qbuf.msg.arg1;
	
	return ret;
}

int cmd_killproc(unsigned int mask)
{
        struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int ret=1;
	
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_KILLPROC;
	mymsg->arg1=mask;
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("kill processes failed\n");
		ret = 0;
	}
	
	return ret;
}

int cmd_upload(const char *fname, int offset)
{
        struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int ret=1;
	
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_UPLOAD;
	mymsg->arg1=offset;
	strncpy(mymsg->mtext, fname, MAX_SEND_SIZE-1);
	mymsg->mtext[MAX_SEND_SIZE-1] = '\0';
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("firmware upgrade failed\n");
		ret = 0;
	}
	
	return ret;
}

int cmd_upgrade_mode(int mode)
{
	struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int ret=1;
	
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_UPGRADE_MODE;
	mymsg->arg1=mode;
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("fail to change to upgrade mode !\n");
		ret = 0;
	}
	
	return ret;
}

int cmd_check_image(const char *fname, int offset) 
{
	
	struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int ret=1;
	
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_CHECK_IMAGE;
	mymsg->arg1=offset;
	strncpy(mymsg->mtext, fname, MAX_SEND_SIZE-1);
	mymsg->mtext[MAX_SEND_SIZE-1] = '\0';
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("firmware upgrade failed\n");
		ret = 0;
	}
	
	return ret;
}

int cmd_start_autohunt()
{
	struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int ret=1;
	
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_START_AUTOHUNT;
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("start autohunt failed\n");
		ret = 0;
	}
	
	return ret;
}

// Aded by Mason Yu
#ifdef CONFIG_USER_DDNS
int cmd_ddnsctrl(const char *ifname)
{
        struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int ret=1;
	
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_DDNS_CTRL;
	strncpy(mymsg->mtext, ifname, MAX_SEND_SIZE-1);
	mymsg->mtext[MAX_SEND_SIZE-1] = '\0';
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("Start DDNSCtrl failed\n");
		ret = 0;
	}
	
	return ret;
}
#endif

// Kaohj -- Translating xml file
/*
 * Translate file (fname) to xml (xname)
 * fname: file name of the [encrypted] file (getting from outside world)
 * xname: file name of the xml-formatted file for local process
 */
int cmd_file2xml(const char *fname, const char *xname)
{
        struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int ret=1;
	MSGFile_T *pFile;
	
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_FILE2XML;
	pFile = (MSGFile_T *)&mymsg->mtext[0];
	strncpy(pFile->fromName, fname, 32);
	pFile->fromName[31] = '\0';
	strncpy(pFile->toName, xname, 32);
	pFile->toName[31] = '\0';
	mymsg->mtext[MAX_SEND_SIZE-1] = '\0';
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("File to XML failed\n");
		ret = 0;
	}
	
	return ret;
}

/*
 * Translate xml (xname) to file (fname)
 * xname: file name of the local-generated xml-formatted file
 * fname: file name of the [encrypted] file (to be transferred)
 */
int cmd_xml2file(const char *xname, const char *fname)
{
        struct mymsgbuf qbuf;
	MSG_T *mymsg;
	int ret=1;
	MSGFile_T *pFile;
	
	mymsg = &qbuf.msg;
	mymsg->cmd = CMD_XML2FILE;
	pFile = (MSGFile_T *)&mymsg->mtext[0];
	strncpy(pFile->fromName, xname, 32);
	pFile->fromName[31] = '\0';
	strncpy(pFile->toName, fname, 32);
	pFile->toName[31] = '\0';
	mymsg->mtext[MAX_SEND_SIZE-1] = '\0';
	sendcmd(&qbuf);
	if (qbuf.request != MSG_SUCC) {
		printf("XML to File failed\n");
		ret = 0;
	}
	
	return ret;
}


