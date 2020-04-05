/*
 *	msgparser.c -- Parser for an well-formed message
 */

#include "../msgq.h"
#include "mibtbl.h"
#include "utility.h"
#include <stdio.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <signal.h>
#include <semaphore.h>
#ifdef EMBED
#include <config/autoconf.h>
#else
#include "../../../../config/autoconf.h"
#endif

#define MAX_ARGS	3
#define MAX_ARG_LEN	32

extern unsigned char * __mib_get_mib_tbl(CONFIG_DATA_T data_type);

static int parse_token(char *buf, char argv[MAX_ARGS][MAX_ARG_LEN+1]);
//static void cfg_get(int argc, char argv[MAX_ARGS][MAX_ARG_LEN+1], struct mymsgbuf *qbuf);
static void cfg_mib_get(struct mymsgbuf *qbuf);
//static void cfg_set(int argc, char argv[MAX_ARGS][MAX_ARG_LEN+1], struct mymsgbuf *qbuf);
static void cfg_mib_set(struct mymsgbuf *qbuf);
static void cfg_mib_set_flash(struct mymsgbuf *qbuf);
static void cfg_mib_info_id(struct mymsgbuf *qbuf);
static void cfg_mib_info_index(struct mymsgbuf *qbuf);
static void cfg_mib_info_total(struct mymsgbuf *qbuf);  // For Star Zhang's fast load
static void cfg_mib_backup(struct mymsgbuf *qbuf);
static void cfg_mib_restore(struct mymsgbuf *qbuf);
static void cfg_mib_get_default(struct mymsgbuf *qbuf);
/*
static void cfg_mib_size(struct mymsgbuf *qbuf);
static void cfg_mib_type(struct mymsgbuf *qbuf);
*/
static void cfg_chain_total(struct mymsgbuf *qbuf);
static void cfg_chain_get(struct mymsgbuf *qbuf);
static void cfg_chain_add(struct mymsgbuf *qbuf);
static void cfg_chain_delete(struct mymsgbuf *qbuf);
static void cfg_chain_clear(struct mymsgbuf *qbuf);
static void cfg_chain_update(struct mymsgbuf *qbuf);
static void cfg_chain_info_id(struct mymsgbuf *qbuf);
static void cfg_chain_info_index(struct mymsgbuf *qbuf);
static void cfg_chain_info_name(struct mymsgbuf *qbuf);
static void cfg_chain_desc_id(struct mymsgbuf *qbuf);

static void cfg_mib_lock(struct mymsgbuf *qbuf);
static void cfg_mib_unlock(struct mymsgbuf *qbuf);
static void cfg_mib_update(struct mymsgbuf *qbuf);
static void cfg_mib_reload(struct mymsgbuf *qbuf);
#ifdef EMBED
static void cfg_reboot(struct mymsgbuf *qbuf);
static void cfg_get_bootmode(struct mymsgbuf *qbuf);
static void cfg_upload(struct mymsgbuf *qbuf);
static void cfg_killprocess(struct mymsgbuf *qbuf);
static void cfg_check_image(struct mymsgbuf *qbuf);
#ifdef AUTO_PVC_SEARCH_AUTOHUNT
static void cfg_start_autohunt(struct mymsgbuf *qbuf);
#endif
#ifdef CONFIG_USER_DDNS
static void cfg_ddns_ctrl(struct mymsgbuf *qbuf);
#endif
static void cfg_upgrade_mode(struct mymsgbuf *qbuf);
#endif
static void cfg_file2xml(struct mymsgbuf *qbuf);
static void cfg_xml2file(struct mymsgbuf *qbuf);
///added by ql
static void cfg_retrieve_table(struct mymsgbuf *qbuf);
static void cfg_retrieve_chain(struct mymsgbuf *qbuf);
static void cfg_clear_mib(struct mymsgbuf *qbuf);
#ifdef  CONFIG_USER_PPPOE_PROXY
static void cfg_add_policy_rule(struct mymsgbuf * qbuf);
static void cfg_del_policy_rule(struct mymsgbuf * qbuf);
static void cfg_add_policy_table(struct mymsgbuf * qbuf);
static void cfg_del_policy_table(struct mymsgbuf * qbuf);
static void cfg_noadsllink_ppp(struct mymsgbuf *qbuf);
#endif
struct command
{
	int	needs_arg;
	int	cmd;
	void	(*func)(struct mymsgbuf *qbuf);
};

volatile int __mib_lock = 0;
MIB_T table_backup;
unsigned char *chain_backup;
unsigned int backupChainSize = 0;

static struct command commands[] = {
	{1, CMD_MIB_GET, cfg_mib_get},
	{1, CMD_MIB_SET, cfg_mib_set},
	{1, CMD_MIB_SET_FLASH, cfg_mib_set_flash},
	{1, CMD_MIB_INFO_ID, cfg_mib_info_id},
	{1, CMD_MIB_INFO_INDEX, cfg_mib_info_index},
	{1, CMD_MIB_INFO_TOTAL, cfg_mib_info_total},  // For Star Zhang's fast load
	{1, CMD_MIB_BACKUP, cfg_mib_backup},
	{1, CMD_MIB_RESTORE, cfg_mib_restore},
	{1, CMD_MIB_GET_DEFAULT, cfg_mib_get_default},
	/*
	{1, CMD_MIB_SIZE, cfg_mib_size},
	{1, CMD_MIB_TYPE, cfg_mib_type},
	*/
	{1, CMD_CHAIN_TOTAL, cfg_chain_total},
	{1, CMD_CHAIN_GET, cfg_chain_get},
	{1, CMD_CHAIN_ADD, cfg_chain_add},
	{1, CMD_CHAIN_DELETE, cfg_chain_delete},
	{1, CMD_CHAIN_CLEAR, cfg_chain_clear},
	{1, CMD_CHAIN_UPDATE, cfg_chain_update},
	{1, CMD_CHAIN_INFO_ID, cfg_chain_info_id},
	{1, CMD_CHAIN_INFO_INDEX, cfg_chain_info_index},
	{1, CMD_CHAIN_INFO_NAME, cfg_chain_info_name},
	{1, CMD_CHAIN_DESC_ID, cfg_chain_desc_id},
	/*
	{1, CMD_CHAIN_SIZE, cfg_chain_size},
	*/
	{1, CMD_MIB_LOCK, cfg_mib_lock},
	{1, CMD_MIB_UNLOCK, cfg_mib_unlock},
	{1, CMD_MIB_UPDATE, cfg_mib_update},
	{1, CMD_MIB_RELOAD, cfg_mib_reload},
#ifdef EMBED
	{1, CMD_REBOOT, cfg_reboot},
	{1, CMD_GET_BOOT_MODE, cfg_get_bootmode},
	{1, CMD_UPLOAD, cfg_upload},
	{1, CMD_KILLPROC, cfg_killprocess},
	{1, CMD_CHECK_IMAGE, cfg_check_image },
#ifdef AUTO_PVC_SEARCH_AUTOHUNT
	{1, CMD_START_AUTOHUNT, cfg_start_autohunt},
#endif
#ifdef CONFIG_USER_DDNS	
	{1, CMD_DDNS_CTRL, cfg_ddns_ctrl },
#endif	
	{1, CMD_UPGRADE_MODE, cfg_upgrade_mode},
	{1, CMD_FILE2XML, cfg_file2xml},
	{1, CMD_XML2FILE, cfg_xml2file},
#endif	
#ifdef CONFIG_USER_PPPOE_PROXY
         {1, CMD_ADD_POLICY_RULE,	cfg_add_policy_rule}, 
        {1, CMD_DEL_POLICY_RULE,	cfg_del_policy_rule}, 
        {1, CMD_ADD_POLICY_TABLE,	cfg_add_policy_table}, 
          {1, CMD_DEL_POLICY_TABLE,	cfg_del_policy_table}, 
         {1, CMD_NO_ADSLLINK_PPP,	cfg_noadsllink_ppp}, 
 #endif
#ifdef	RESERVE_KEY_SETTING
	{1, CMD_MIB_RETRIVE_TABLE, cfg_retrieve_table},
	{1, CMD_MIB_RETRIVE_CHAIN, cfg_retrieve_chain},
	{1, CMD_MIB_CLEAR,	cfg_clear_mib},
#endif
	{0, 0, NULL}
};

int msgProcess(struct mymsgbuf *qbuf)
{
  	int argc, c;
	char argv[MAX_ARGS][MAX_ARG_LEN+1];
	
	// response message type should be the client request magic number
	qbuf->mtype = qbuf->request;
	/*
	if ((argc=parse_token(qbuf->mtext, argv)) == 0)
		return 0;
	
	for(c=0; commands[c].name!=NULL; c++) {
		if(!strcmp(argv[0], commands[c].name)) {
			argc--;
			if(argc >= commands[c].num_string_arg)
				commands[c].func(argc, (char **)(&argv[1]), qbuf);
			break;
		}
	}
	*/
	for (c=0; commands[c].cmd!=0; c++) {
		if (qbuf->msg.cmd == commands[c].cmd) {
			commands[c].func(qbuf);
			break;
		}
	}
	return 0;
}

/******************************************************************************/
/*
 *	Token Parser -- parse tokens seperated by spaces on buf
 *	Return: number of tokens been parsed
 */


static void cfg_mib_lock(struct mymsgbuf *qbuf)
{
	__mib_lock = 1;
	qbuf->request = MSG_SUCC;
}

static void cfg_mib_unlock(struct mymsgbuf *qbuf)
{
	__mib_lock = 0;
	qbuf->request = MSG_SUCC;
}

static void cfg_mib_update(struct mymsgbuf *qbuf)
{
	CONFIG_DATA_T data_type;
	CONFIG_MIB_T flag;
	char vChar;
	qbuf->request = MSG_FAIL;
	
	data_type = (CONFIG_DATA_T)qbuf->msg.arg1;
	flag = (CONFIG_MIB_T)qbuf->msg.arg2;
	
#ifdef CLOSE_ITF_BEFORE_WRITE
	/*to avoid the dead when wrriting the flash*/
	itfcfg("sar", 0);
	itfcfg("eth0", 0);
#ifdef WLAN_SUPPORT
	vChar=0;
	_mib_get(MIB_WLAN_DISABLED, (void *)&vChar);
	//if (!vChar)
	//	stopwlan();
#endif
#endif /* CLOSE_ITF_BEFORE_WRITE*/
	if (data_type == CURRENT_SETTING) {
		if (flag == CONFIG_MIB_ALL) {
			if(_mib_update(data_type)!=0)
				qbuf->request = MSG_SUCC;
		}
		else if (flag == CONFIG_MIB_TABLE) {
			PARAM_HEADER_T header;
			unsigned int total_size, table_size;
			unsigned char *buf, *ptr;
			unsigned char *pMibTbl;
			
			if(__mib_header_read(data_type, &header) != 1)
				return;
			total_size = sizeof(PARAM_HEADER_T) + header.len;
			buf = (unsigned char *)malloc(total_size);
			if (buf == NULL)
				return;
			if(mib_read_to_raw(data_type, buf, total_size) != 1) {
				free(buf);
				return;
			}
			ptr = buf + sizeof(PARAM_HEADER_T);
			// update the mib table only
			pMibTbl = __mib_get_mib_tbl(data_type);
			memcpy(ptr, pMibTbl, sizeof(MIB_T));
			__mib_content_encod_check(data_type, &header, ptr);
			// update header
			memcpy(buf, (unsigned char*)&header, sizeof(PARAM_HEADER_T));
			
			if(mib_update_from_raw(buf, total_size) != 1) {
				free(buf);
				return;
			}
			free(buf);
			qbuf->request = MSG_SUCC;
		}
		else { // not support currently, Jenny added  
				//jim we should check the size to make sure of no-exceeded flash range....
				//jim this will called by pppoe.c /pppoe_session_info();
			PARAM_HEADER_T header;
			unsigned int chainRecordSize, mibTblSize, totalSize;
			unsigned char *buf, *ptr;
			unsigned char* pVarLenTable = NULL;
		    	//printf("%s line %d\n", __FUNCTION__, __LINE__);
			if(__mib_header_read(data_type, &header) != 1)
				return;
		    	//printf("%s line %d\n", __FUNCTION__, __LINE__);
			mibTblSize = __mib_content_min_size(data_type);
			chainRecordSize = __mib_chain_all_table_size(data_type);
			header.len = chainRecordSize + mibTblSize;
			totalSize = sizeof(PARAM_HEADER_T) + header.len;
			buf = (unsigned char *)malloc(totalSize);
		    	//printf("%s line %d Totalsize=%d\n", __FUNCTION__, __LINE__, totalSize);
			if (buf == NULL)
				return;
			//jim
			if(totalSize > __mib_content_max_size(data_type))
			{
				printf("too large config paras to store! abadon!\n");
				return;
			}
		    	//printf("%s line %d\n", __FUNCTION__, __LINE__);
			if(mib_read_to_raw(data_type, buf, totalSize) != 1) {
				free(buf);
		    	//printf("%s line %d\n", __FUNCTION__, __LINE__);
				return;
			}
			ptr = &buf[sizeof(PARAM_HEADER_T)];	// point to start of MIB data 
		    	//printf("%s line %d chainRecordSize=%d\n", __FUNCTION__, __LINE__, chainRecordSize);
			// update the chain record only
			if (chainRecordSize > 0) {
				pVarLenTable = &ptr[mibTblSize];	// point to start of variable length MIB data 
				if(__mib_chain_record_content_encod(data_type, pVarLenTable, chainRecordSize) != 1) {
					free(buf);
		    	//printf("%s line %d\n", __FUNCTION__, __LINE__);
					return;
				}
			}
			__mib_content_encod_check(data_type, &header, ptr);
			// update header
			memcpy(buf, (unsigned char*)&header, sizeof(PARAM_HEADER_T));

		    	//printf("%s line %d\n", __FUNCTION__, __LINE__);
			if(mib_update_from_raw(buf, totalSize) != 1) {
				free(buf);
		    	//printf("%s line %d\n", __FUNCTION__, __LINE__);
				return;
			}
			qbuf->request = MSG_SUCC;
		    	//printf("%s line %d\n", __FUNCTION__, __LINE__);
			free(buf);
		}
	}
	else {
		if(_mib_update(data_type)!=0)
			qbuf->request = MSG_SUCC;
	}
#ifdef CLOSE_ITF_BEFORE_WRITE
	itfcfg("sar", 1);
	itfcfg("eth0", 1);
#endif /* CLOSE_ITF_BEFORE_WRITE*/
}

static void cfg_mib_get(struct mymsgbuf *qbuf)
{
	int id;
	unsigned int *value;
	
	qbuf->request = MSG_FAIL;
	
	if(_mib_get(qbuf->msg.arg1, (void *)qbuf->msg.mtext)!=0)
		qbuf->request = MSG_SUCC;
	value = (unsigned int *)qbuf->msg.mtext;	
}



static void cfg_mib_get_default(struct mymsgbuf *qbuf)
{
	int id;
	unsigned int *value;
	
	qbuf->request = MSG_FAIL;
	
	if(_mib_getDef(qbuf->msg.arg1, (void *)qbuf->msg.mtext)!=0)
		qbuf->request = MSG_SUCC;
	value = (unsigned int *)qbuf->msg.mtext;
}

static void cfg_mib_set(struct mymsgbuf *qbuf)
{
	qbuf->request = MSG_FAIL;
	
	if (__mib_lock) {
		qbuf->request = MSG_MIB_LOCKED;
		return;
	}
	
	if(_mib_set(qbuf->msg.arg1, (void *)qbuf->msg.mtext)!=0)
		qbuf->request = MSG_SUCC;
}

static void cfg_mib_set_flash(struct mymsgbuf *qbuf)
{
	struct mymsgbuf myqbuf;
	
	qbuf->request = MSG_FAIL;
	
	if (__mib_lock) {
		qbuf->request = MSG_MIB_LOCKED;
		return;
	}

	_mib_set(qbuf->msg.arg1, (void *)qbuf->msg.mtext); // set to the current table
	myqbuf.msg.arg1 = CONFIG_MIB_TABLE;
	cfg_mib_backup(&myqbuf);	// backup current MIB table into system
	myqbuf.msg.arg1 = CURRENT_SETTING;
	myqbuf.msg.arg2 = CONFIG_MIB_TABLE;
	cfg_mib_reload(&myqbuf);	//get table setting from flash
	_mib_set(qbuf->msg.arg1, (void *)qbuf->msg.mtext); // set to the flash table
#ifdef CLOSE_ITF_BEFORE_WRITE
	itfcfg("eth0", 0);
	itfcfg("sar", 0);
#endif /* CLOSE_ITF_BEFORE_WRITE*/
	
	myqbuf.msg.arg1 = CURRENT_SETTING;
	myqbuf.msg.arg2 = CONFIG_MIB_TABLE;
	cfg_mib_update(&myqbuf);

	myqbuf.msg.arg1 = CONFIG_MIB_TABLE;
	cfg_mib_restore(&myqbuf);	// restore backup MIB table
#ifdef CLOSE_ITF_BEFORE_WRITE
	itfcfg("eth0",1);
	itfcfg("sar",1);
#endif /* CLOSE_ITF_BEFORE_WRITE*/
	qbuf->request = MSG_SUCC;
}

static void cfg_mib_info_id(struct mymsgbuf *qbuf)
{
	int k;
	
	qbuf->request = MSG_FAIL;
	
	for (k=0; mib_table[k].id; k++) {
		if (mib_table[k].id == qbuf->msg.arg1)
			break;
	}
	
	if (mib_table[k].id == 0)
		return;
	
	memcpy((void *)qbuf->msg.mtext, (void *)&mib_table[k], sizeof(mib_table_entry_T));
	qbuf->request = MSG_SUCC;
}

static void cfg_mib_info_index(struct mymsgbuf *qbuf)
{
	int total;
	int i;
	
	qbuf->request = MSG_FAIL;
	
	for (i=0; mib_table[i].id; i++);
	total = i+1;
	if (qbuf->msg.arg1>total)
		return;
	
	memcpy((void *)qbuf->msg.mtext, (void *)&mib_table[qbuf->msg.arg1], sizeof(mib_table_entry_T));
	qbuf->request = MSG_SUCC;
}

// Apply Star Zhang's fast load
static void cfg_mib_info_total(struct mymsgbuf *qbuf)
{
	int total;
	int i;
	
	qbuf->request = MSG_FAIL;
	
	for (i=0; mib_table[i].id; i++);
		 total = i+1;
	qbuf->msg.arg1=total;

	qbuf->request = MSG_SUCC;
}
// The end of fast load

static void cfg_mib_backup(struct mymsgbuf *qbuf)
{
	CONFIG_MIB_T type;
	unsigned char *pMibTbl;
	
	qbuf->request = MSG_FAIL;
	type = (CONFIG_MIB_T)qbuf->msg.arg1;
	
//	if (type == CONFIG_MIB_ALL || type == CONFIG_MIB_TABLE) {
	if (type == CONFIG_MIB_TABLE) {
		pMibTbl = __mib_get_mib_tbl(CURRENT_SETTING);
		memcpy(&table_backup, pMibTbl, sizeof(MIB_T));  //save setting
	}
	else if (type == CONFIG_MIB_CHAIN){
		unsigned char* pVarLenTable = NULL;
		PARAM_HEADER_T header;
		unsigned int chainRecordSize, mibTblSize, totalSize;
		unsigned char *buf, *ptr;

		if (chain_backup)	 free(chain_backup);
		if(__mib_header_read(CURRENT_SETTING, &header) != 1)
			return;
		mibTblSize = __mib_content_min_size(CURRENT_SETTING);
		backupChainSize = __mib_chain_all_table_size(CURRENT_SETTING);
		header.len = backupChainSize + mibTblSize;
		totalSize = sizeof(PARAM_HEADER_T) + header.len;
		buf = (unsigned char *)malloc(totalSize);
		if (buf == NULL)
			return;
		if(mib_read_to_raw(CURRENT_SETTING, buf, totalSize) != 1) {
			free(buf);
			return;
		}
		ptr = &buf[sizeof(PARAM_HEADER_T)];
		if (backupChainSize > 0) {
			pVarLenTable = &ptr[mibTblSize];
			if(__mib_chain_record_content_encod(CURRENT_SETTING, pVarLenTable, backupChainSize) != 1) {
				free(buf);
				return;
			}
		}
		chain_backup = (unsigned char *)malloc(backupChainSize);
		memcpy(chain_backup, pVarLenTable, backupChainSize);  //save MIB chain setting
		free(buf);
//		return;
	}
	else if (type == CONFIG_MIB_ALL){
		unsigned char* pVarLenTable = NULL;
		PARAM_HEADER_T header;
		unsigned int chainRecordSize, mibTblSize, totalSize;
		unsigned char *buf, *ptr;

		if (chain_backup)	 free(chain_backup);
		if(__mib_header_read(CURRENT_SETTING, &header) != 1)
			return;
		mibTblSize = __mib_content_min_size(CURRENT_SETTING);
		backupChainSize = __mib_chain_all_table_size(CURRENT_SETTING);
		header.len = backupChainSize + mibTblSize;
		totalSize = sizeof(PARAM_HEADER_T) + header.len;
		buf = (unsigned char *)malloc(totalSize);
		if (buf == NULL)
			return;
		if(mib_read_to_raw(CURRENT_SETTING, buf, totalSize) != 1) {
			free(buf);
			return;
		}
		ptr = &buf[sizeof(PARAM_HEADER_T)];
		pMibTbl = __mib_get_mib_tbl(CURRENT_SETTING);
		memcpy(&table_backup, pMibTbl, sizeof(MIB_T));  //save MIB table setting
		if (backupChainSize > 0) {
			pVarLenTable = &ptr[mibTblSize];
			if(__mib_chain_record_content_encod(CURRENT_SETTING, pVarLenTable, backupChainSize) != 1) {
				free(buf);
		return;
			}
		}
		chain_backup = (unsigned char *)malloc(backupChainSize);
		memcpy(chain_backup, pVarLenTable, backupChainSize);  //save MIB chain setting
		free(buf);
//		return;
	}
	else
		return;
	
	qbuf->request = MSG_SUCC;
}

//added by ql
#ifdef	RESERVE_KEY_SETTING
static void cfg_retrieve_table(struct mymsgbuf *qbuf)
{
	int id;

	qbuf->request = MSG_FAIL;
	id = qbuf->msg.arg1;

	mib_table_record_retrive(id);

	qbuf->request = MSG_SUCC;
}
static void cfg_retrieve_chain(struct mymsgbuf *qbuf)
{
	int id;

	qbuf->request = MSG_FAIL;
	id = qbuf->msg.arg1;

	mib_chain_record_retrive(id);

	qbuf->request = MSG_SUCC;
}
static void cfg_clear_mib(struct mymsgbuf *qbuf)
{
	CONFIG_MIB_T type;

	qbuf->request = MSG_FAIL;
	type = (CONFIG_MIB_T)qbuf->msg.arg1;

	mib__record_clear(type);

	qbuf->request = MSG_SUCC;
}
#endif

static void cfg_mib_restore(struct mymsgbuf *qbuf)
{
	CONFIG_MIB_T type;
	unsigned char *pMibTbl;
	
	qbuf->request = MSG_FAIL;
	type = (CONFIG_MIB_T)qbuf->msg.arg1;
	
//	if (type == CONFIG_MIB_ALL || type == CONFIG_MIB_TABLE) {
	if (type == CONFIG_MIB_TABLE) {
		pMibTbl = __mib_get_mib_tbl(CURRENT_SETTING);
		memcpy(pMibTbl, &table_backup, sizeof(MIB_T));  //restore setting
	}
	else if (type == CONFIG_MIB_CHAIN){
		unsigned char* pVarLenTable = NULL;
		PARAM_HEADER_T header;
		unsigned int mibTblSize, totalSize;
		unsigned char *buf, *ptr;

		if(__mib_header_read(CURRENT_SETTING, &header) != 1)
			return;
		mibTblSize = __mib_content_min_size(CURRENT_SETTING);
		header.len = backupChainSize + mibTblSize;
		totalSize = sizeof(PARAM_HEADER_T) + header.len;
		buf = (unsigned char *)malloc(totalSize);
		if (buf == NULL)
			return;
		if(__mib_file_read(CURRENT_SETTING, buf, totalSize) != 1) {
			free(buf);
			return;
		}
		ptr = &buf[sizeof(PARAM_HEADER_T)];
		pMibTbl = __mib_get_mib_tbl(CURRENT_SETTING);
		memcpy(ptr, pMibTbl, sizeof(MIB_T));
		__mib_chain_all_table_clear(CURRENT_SETTING);
		if(backupChainSize > 0)
		{
			pVarLenTable = &ptr[mibTblSize];	// point to start of variable length MIB data 
			memcpy(pVarLenTable, chain_backup, backupChainSize);  //restore MIB chain setting
			// parse variable length MIB data
			if( __mib_chain_record_content_decod(pVarLenTable, backupChainSize) != 1)
			{
				free(buf);
				return;
			}
		}
		memcpy(buf, (unsigned char*)&header, sizeof(PARAM_HEADER_T));
		free(buf);
//		return;
	}
	else if (type == CONFIG_MIB_ALL){
		unsigned char* pVarLenTable = NULL;
		PARAM_HEADER_T header;
		unsigned int mibTblSize, totalSize;
		unsigned char *buf, *ptr;

		if(__mib_header_read(CURRENT_SETTING, &header) != 1)
			return;
		mibTblSize = __mib_content_min_size(CURRENT_SETTING);
		header.len = backupChainSize + mibTblSize;
		totalSize = sizeof(PARAM_HEADER_T) + header.len;
		buf = (unsigned char *)malloc(totalSize);
		if (buf == NULL)
			return;
		if(__mib_file_read(CURRENT_SETTING, buf, totalSize) != 1) {
			free(buf);
			return;
		}
		ptr = &buf[sizeof(PARAM_HEADER_T)];
		pMibTbl = __mib_get_mib_tbl(CURRENT_SETTING);
		memcpy(pMibTbl, &table_backup, sizeof(MIB_T));  //restore setting
		//memcpy(ptr, &table_backup, sizeof(MIB_T));  //restore MIB table setting
		__mib_chain_all_table_clear(CURRENT_SETTING);
		if(backupChainSize > 0)
		{
			pVarLenTable = &ptr[mibTblSize];	// point to start of variable length MIB data 
			memcpy(pVarLenTable, chain_backup, backupChainSize);  //restore MIB chain setting
			// parse variable length MIB data
			if( __mib_chain_record_content_decod(pVarLenTable, backupChainSize) != 1)
			{
				free(buf);
				return;
			}
		}
		memcpy(buf, (unsigned char*)&header, sizeof(PARAM_HEADER_T));
		free(buf);
//		return;
	}
	else
		return;
	
	qbuf->request = MSG_SUCC;
}

static void cfg_chain_total(struct mymsgbuf *qbuf)
{
	qbuf->request = MSG_FAIL;
	
	qbuf->msg.arg1 = _mib_chain_total(qbuf->msg.arg1);
	qbuf->request = MSG_SUCC;
}

static void cfg_chain_get(struct mymsgbuf *qbuf)
{
	int index, entryNo;
	void *pEntry;
	
	qbuf->request = MSG_FAIL;
	index = __mib_chain_mib2tbl_id(qbuf->msg.arg1);
	if (index == -1)
		return;
	
	entryNo = atoi(qbuf->msg.mtext);
	pEntry = _mib_chain_get(qbuf->msg.arg1, entryNo);
	if (pEntry) {
		memcpy(qbuf->msg.mtext, pEntry, mib_chain_record_table[index].per_record_size);
		qbuf->request = MSG_SUCC;
	}
	else
		qbuf->request = MSG_FAIL;
}

static void cfg_chain_add(struct mymsgbuf *qbuf)
{
	qbuf->request = MSG_FAIL;
	
	if (__mib_lock) {
		qbuf->request = MSG_MIB_LOCKED;
		return;
	}
	
	if (_mib_chain_add(qbuf->msg.arg1, qbuf->msg.mtext)) {
		qbuf->request = MSG_SUCC;
	}
	else
		qbuf->request = MSG_FAIL;
}

static void cfg_chain_delete(struct mymsgbuf *qbuf)
{
	int entryNo;
	
	qbuf->request = MSG_FAIL;
	entryNo = atoi(qbuf->msg.mtext);
	
	if (__mib_lock) {
		qbuf->request = MSG_MIB_LOCKED;
		return;
	}
	
	if (_mib_chain_delete(qbuf->msg.arg1, entryNo)) {
		qbuf->request = MSG_SUCC;
	}
	else
		qbuf->request = MSG_FAIL;
}

static void cfg_chain_clear(struct mymsgbuf *qbuf)
{
	qbuf->request = MSG_FAIL;
	
	if (__mib_lock) {
		qbuf->request = MSG_MIB_LOCKED;
		return;
	}
	
	_mib_chain_clear(qbuf->msg.arg1);
	qbuf->request = MSG_SUCC;
}

static void cfg_chain_update(struct mymsgbuf *qbuf)
{
	int index;
	void *pEntry;
	
	qbuf->request = MSG_FAIL;
	
	if (__mib_lock) {
		qbuf->request = MSG_MIB_LOCKED;
		return;
	}
	
	index = __mib_chain_mib2tbl_id(qbuf->msg.arg1);
	if (index == -1)
		return;
	pEntry = _mib_chain_get(qbuf->msg.arg1, qbuf->msg.arg2);
	if (pEntry)
		memcpy(pEntry, qbuf->msg.mtext, mib_chain_record_table[index].per_record_size);
	else
		return;
	if (_mib_chain_update(qbuf->msg.arg1, qbuf->msg.mtext, qbuf->msg.arg2)) {
		qbuf->request = MSG_SUCC;
	}
	else
		qbuf->request = MSG_FAIL;
}

static void cfg_chain_info_id(struct mymsgbuf *qbuf)
{
	int index;
	
	qbuf->request = MSG_FAIL;
	
	index = __mib_chain_mib2tbl_id(qbuf->msg.arg1);
	if (index == -1)
		return;
	
	memcpy((void *)qbuf->msg.mtext, (void *)&mib_chain_record_table[index], sizeof(mib_chain_record_table_entry_T));
	qbuf->request = MSG_SUCC;
}

static void cfg_chain_info_index(struct mymsgbuf *qbuf)
{
	int total;
	int i;
	
	qbuf->request = MSG_FAIL;
	
	for (i=0; mib_chain_record_table[i].id; i++);
	total = i+1;
	if (qbuf->msg.arg1>total)
		return;
	
	memcpy((void *)qbuf->msg.mtext, (void *)&mib_chain_record_table[qbuf->msg.arg1], sizeof(mib_chain_record_table_entry_T));
	qbuf->request = MSG_SUCC;
}

static void cfg_chain_info_name(struct mymsgbuf *qbuf)
{
	int total;
	int i;
	
	qbuf->request = MSG_FAIL;
	
	for (i=0; mib_chain_record_table[i].id; i++) {
		if (!strcmp(mib_chain_record_table[i].name, qbuf->msg.mtext))
			break;
	}
	
	if (mib_chain_record_table[i].id == 0)
		return; // not found
	
	memcpy((void *)qbuf->msg.mtext, (void *)&mib_chain_record_table[i], sizeof(mib_chain_record_table_entry_T));
	qbuf->request = MSG_SUCC;
}

static void cfg_chain_desc_id(struct mymsgbuf *qbuf)
{
	int index;
	mib_chain_member_entry_T *rec_desc;
	int i, size, copylen;
	
	qbuf->request = MSG_FAIL;
	
	index = __mib_chain_mib2tbl_id(qbuf->msg.arg1);
	if (index == -1)
		return;
	
	rec_desc = mib_chain_record_table[index].record_desc;
	if (rec_desc == 0)
		return;
	// get total size of the record descriptor
	i = 0; size = 0;
	while (rec_desc[i++].name[0] != 0)
		size++;
	
	copylen = sizeof(mib_chain_member_entry_T)*size;
	if (copylen > MAX_SEND_SIZE)
		return;
	qbuf->msg.arg1 = size;
	memcpy((void *)qbuf->msg.mtext, (void *)rec_desc, copylen);
	qbuf->request = MSG_SUCC;
}

/*
 *	reload hs	---	reload hardware setting
 *	reload cs	---	reload current setting
 *	reload ds	---	reload default setting
 */
static void cfg_mib_reload(struct mymsgbuf *qbuf)
{
	CONFIG_DATA_T data_type;
	CONFIG_MIB_T flag;
	qbuf->request = MSG_FAIL;
	
	if (__mib_lock) {
		qbuf->request = MSG_MIB_LOCKED;
		return;
	}
	
	data_type = (CONFIG_DATA_T)qbuf->msg.arg1;
	flag = (CONFIG_MIB_T)qbuf->msg.arg2;
	
	if (data_type == CURRENT_SETTING) {
		if (flag == CONFIG_MIB_ALL) {
			if(_mib_load(data_type)!=0)
				qbuf->request = MSG_SUCC;
		}
		else if (flag == CONFIG_MIB_TABLE) {
			if (mib_load_table(data_type)!=0)
				qbuf->request = MSG_SUCC;
		}
		else { // not support currently, Jenny added
			if (mib_load_chain(data_type)!=0)
				qbuf->request = MSG_SUCC;
		}
	}
	else {
		if(_mib_load(data_type)!=0)
			qbuf->request = MSG_SUCC;
	}
}



#ifdef CONFIG_USER_PPPOE_PROXY
#define IPPOLICY_ROUTE_TABLE_OFFSET   10
static void cfg_add_policy_table (struct mymsgbuf *qbuf){

    printf("cfg_add_policy_table,  wan unit =%d \n",qbuf->msg.arg1);

     int wan_unit ;
     pppoe_proxy pp_cmd;
     char wan_pppif[6];
     char wan_unitif[6];
     char  lan_pppif[6];
     wan_unit = qbuf->msg.arg1; 
     snprintf(wan_unitif, 6, "%u",IPPOLICY_ROUTE_TABLE_OFFSET+wan_unit);
     snprintf(wan_pppif, 6, "ppp%u",wan_unit);
     va_cmd("/bin/ip",7,1,"route","add","default","dev",wan_pppif,"table",wan_unitif);
      pp_cmd.wan_unit = wan_unit;
      pp_cmd.cmd = PPPOE_GET_LAN_UNIT;
      ppp_proxy_ioctl( &pp_cmd,  SIOCPPPOEPROXY); 
      
	 snprintf(lan_pppif, 6, "ppp%u",pp_cmd.lan_unit); 
	 va_cmd("/bin/ip",6,1,"rule","add","iif",lan_pppif,"table",wan_unitif);
	 
    qbuf->request = MSG_SUCC;
}

static void cfg_del_policy_table(struct mymsgbuf *qbuf){
	   


     int wan_unit ;
     char wan_pppif[6];
     char wan_unitif[6];
     wan_unit = qbuf->msg.arg1; 
     snprintf(wan_unitif, 6, "%u",IPPOLICY_ROUTE_TABLE_OFFSET+wan_unit);
     snprintf(wan_pppif, 6, "ppp%u",wan_unit);
     
     va_cmd("/bin/ip",7,1,"route","del","default","dev",wan_pppif,"table",wan_unitif);
         
//	  va_cmd("/bin/ip",6,1,"rule","add","iif",pppif,"table",wan_pppif);
   
    qbuf->request = MSG_SUCC;
   


}

static void cfg_del_policy_rule(struct mymsgbuf *qbuf){
	


     int wan_unit ;
     int lan_unit;
     char wan_pppif[6];
     char  lan_pppif[6];

     lan_unit  = qbuf->msg.arg1;
     wan_unit = qbuf->msg.arg2; 
     snprintf(wan_pppif, 6, "%u",IPPOLICY_ROUTE_TABLE_OFFSET+wan_unit);
     snprintf(lan_pppif, 6, "ppp%u",lan_unit);
         
     va_cmd("/bin/ip",6,1,"rule","del","iif",lan_pppif,"table",wan_pppif);
   
    qbuf->request = MSG_SUCC;  



}


static void cfg_add_policy_rule(struct mymsgbuf *qbuf){


     int wan_unit ;
     int lan_unit;
     char wan_pppif[6];
     char  lan_pppif[6];

     lan_unit  = qbuf->msg.arg1;
     wan_unit = qbuf->msg.arg2; 
     snprintf(wan_pppif, 6, "%u",IPPOLICY_ROUTE_TABLE_OFFSET+wan_unit);
     snprintf(lan_pppif, 6, "ppp%u",lan_unit);
         
     va_cmd("/bin/ip",6,1,"rule","add","iif",lan_pppif,"table",wan_pppif);
   
    qbuf->request = MSG_SUCC; 	  


}




static void cfg_noadsllink_ppp(struct mymsgbuf *qbuf)
{
     struct data_to_pass_st spppd_msg;
     int unit ;
     int share_num;
     int i;
     pppoe_proxy pp_cmd;
     char wanif[6];
     char wan_pppif[6];
     unit = qbuf->msg.arg1; 
     char lanif[6];

		  snprintf(wanif, 6, "%u", IPPOLICY_ROUTE_TABLE_OFFSET+unit);
                snprintf(wan_pppif, 6, "ppp%u", unit);
                pp_cmd.cmd = PPPOE_SHARE_NUM;   
                pp_cmd.wan_unit =  unit;     
                ppp_proxy_ioctl( &pp_cmd,  SIOCPPPOEPROXY);
		 
		  
		  if((share_num=pp_cmd.share_num)==1)
		  	{
		  	    
			    pp_cmd.cmd= PPPOE_DEL_SINGLE_UNIT;
			    pp_cmd.wan_unit =  unit;
			     
		           ppp_proxy_ioctl( &pp_cmd,  SIOCPPPOEPROXY);
			    
			   
			    kill(pp_cmd.pid,SIGTERM);
			  
			    snprintf(lanif, 6, "ppp%u", pp_cmd.lan_unit);
			    
			    va_cmd("/bin/ip",6,1,"rule","del","iif",lanif,"table", wanif);
	                  va_cmd("/bin/ip",7,1,"route","del","default","dev",wan_pppif,"table",wanif);
		  	}
               else 
               	{
               	   
               	    pp_cmd.cmd = PPPOE_DEL_SHARED_UNIT;
			    pp_cmd.share_num = share_num;
			    pp_cmd.wan_unit =  unit;
			    ppp_proxy_ioctl( &pp_cmd,  SIOCPPPOEPROXY);
		
			    for(i=0 ;i < MAXSHARENUM;i++)
			    {
			       if(pp_cmd.share_lan_unit[i]!=-1)
				  {
                            snprintf(lanif, 6, "ppp%u",pp_cmd.share_lan_unit[i]);
				va_cmd("/bin/ip",6,1,"rule","del","iif",lanif,"table", wanif);
				kill(pp_cmd.share_pid[i],SIGTERM);
			       }
			    }
				
               	}
                   
                   snprintf(spppd_msg.data, BUF_SIZE, "spppctl del %u proxy 0",unit);

                  write_to_pppd(&spppd_msg); 

		 
            qbuf->request = MSG_SUCC;

}
#endif
#ifdef EMBED

static void cfg_reboot(struct mymsgbuf *qbuf)
{
	/*
	va_cmd(IFCONFIG, 2, 1, (char *)ELANIF, "down");
	va_cmd(IFCONFIG, 2, 1, (char *)WLANIF, "down");
	va_cmd("/bin/sarctl",1,1,"disable");
	*/
	va_cmd("/bin/adslctrl",1,1,"disablemodemline");
	itfcfg("sar", 0);
	itfcfg("eth0", 0);
	itfcfg("wlan0", 0);
	
	/* reboot the system */
	reboot(RB_AUTOBOOT);
	//va_cmd("/bin/reboot", 0, 1);
	qbuf->request = MSG_SUCC;
}

static void cfg_get_bootmode(struct mymsgbuf *qbuf)
{
	qbuf->msg.arg1 = __boot_mode;
	qbuf->request = MSG_SUCC;
}
/*
 *	upload <filename>	---	upload firmware
 */
// Brian --- for debug
//FILE *myfp;
//const char ftpFile[]="/var/msglog";
//const char killFile[]="/var/killlog";
struct pidStruc {
	PID_SHIFT_T shift;
	char *pidFile;
};

static struct pidStruc killProcessFile[]=
{
	{PID_DNSMASQ, "dnsmasq.pid"},
	{PID_SNMPD, "snmpd.pid"},
	{PID_WEB, "boa.pid"},
	{PID_CLI, "cli.pid"},
	{PID_DHCPD, "udhcpd.pid"},
	{PID_DHCPRELAY, "dhcrelay.pid"},
//	{PID_INETD, "inetd.pid"},
	{PID_TELNETD, "telnetd.pid"},
	{PID_FTPD, "ftpd.pid"},
	{PID_TFTPD, "tftpd.pid"},
	{PID_SSHD, "dropbear.pid"},
	{PID_SYSLOGD, "syslogd.pid"},
	{PID_KLOGD, "klogd.pid"},
	{PID_IGMPPROXY, "igmp_pid"},
	{PID_RIPD, "routed.pid"},
	{PID_SNTPD, "vsntp.pid"},
	{PID_MPOAD, "mpoad.pid"},
	{PID_SPPPD, "spppd.pid"},
	{PID_UPNPD, "linuxigd.pid"},
	{PID_UPDATEDD, "updatedd.pid"},
	{PID_CWMP, "cwmp.pid"}, /*tr069/cwmpClient pid,jiunming*/
	{-1, 0}
};
/*

static char *killProcessFile[]=
{
   "dnsmasq.pid", // dns
   "snmpd.pid",
   "boa.pid",
   "cli.pid",
   0
};
*/

#define CONF_PIDFILE_PATH  "/var/run"
#define SIGKILL		 9
#define SIGTERM		15
int g_killprocess=KILL_PROCESS_OFF;
static void cfg_killprocess(struct mymsgbuf *qbuf)
{
	int  spid, index;
	FILE *spidfile;
	unsigned int pidMask, thisPid;
	
	qbuf->request = MSG_FAIL;
	
	if (chdir (CONF_PIDFILE_PATH) < 0){
		return;
	}
	pidMask = (unsigned int)qbuf->msg.arg1;
	
	for( index=0; killProcessFile[index].pidFile; index++ )
	{
		// check for process mask
		thisPid = 1 << killProcessFile[index].shift;
		if (!(thisPid & pidMask))
			continue;
		if ((spidfile = fopen( killProcessFile[index].pidFile, "r"))) {
			// Mason Yu
			if ( strcmp("updatedd.pid", killProcessFile[index].pidFile) == 0 ) {				
				g_killprocess = KILL_PROCESS_ON;							
			} 	 
				
			fscanf(spidfile, "%d\n", &spid);
			fclose(spidfile);
			printf("kill=%s spid=%d\n", killProcessFile[index].pidFile, spid);
			//fprintf(myfp, "cfg_killprocess: kill=%s spid=%d\n", killProcessFile[index].pidFile, spid);
			//fflush(myfp);			
			kill( spid, SIGTERM);			
		}
	}
	
	qbuf->request = MSG_SUCC;
}

#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#define SIOCETHTEST_SAVED 0x89a1

struct arg{
	unsigned char cmd;
	unsigned int cmd2;
	unsigned int cmd3;
	unsigned int cmd4;
}pass_arg_saved;

void reboot_by_watchdog()
{
	struct ifreq	ifr;
  	int fd=0;
	
	fd = socket(AF_INET, SOCK_DGRAM, 0);
  	if(fd< 0){
		printf("Saved: Watchdog control fail!\n");
		goto fail;
  	}
	strncpy(ifr.ifr_name, "eth0", 16);
	ifr.ifr_data = (void *)&pass_arg_saved;
	
	pass_arg_saved.cmd=14;  //watchdog command
    	pass_arg_saved.cmd2=2;
    	pass_arg_saved.cmd3=0;    	
    	
	return ioctl(fd, SIOCETHTEST_SAVED, &ifr);
	
fail:
	return 1;
}

void stop_processes(void)
{

	//printf("upgrade: killing tasks...\n");
	
	kill(1, SIGTSTP);		/* Stop init from reforking tasks */
	kill(1, SIGSTOP);		
	kill(2, SIGSTOP);		
	kill(3, SIGSTOP);		
	kill(4, SIGSTOP);		
	kill(5, SIGSTOP);		
	kill(6, SIGSTOP);		
	//atexit(restartinit);		/* If exit prematurely, restart init */
	sync();

	
}

static void cfg_upload(struct mymsgbuf *qbuf)
{
	int rv, offset;
	FILE	*fp=NULL;		
	struct stat st;
	int fsize;
	struct mymsgbuf myqbuf;
	IMGHDR imgHdr; 
	/*ql:20080729 START: check image key validity.*/
#ifdef MULTI_IC_SUPPORT
	unsigned int key;
#endif
	/*ql:20080729 END*/

#ifndef DEMOBOARD_1_1_2
//#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
//#ifdef ZTE_LED_REQUEST
// Kaohj -- TR068 Power LED
//star: for ZTE power LED request	
	FILE *fp1;
	unsigned char power_flag;
	fp1 = fopen("/proc/power_flag","w");
	if(fp1)
	{
		
		power_flag = '1';
		fwrite(&power_flag,1,1,fp1);
		fclose(fp1);
	}
//#endif
#endif
	
	itfcfg("eth0", 0);
	itfcfg("sar", 0);
	itfcfg("wlan0", 0);
	
#ifdef CONFIG_8M_SDRAM
	stop_processes();
	myqbuf.msg.arg1 = NET_PID;
	cfg_killprocess(&myqbuf);
#endif // of CONFIG_8M_SDRAM
	
	qbuf->request = MSG_FAIL;
	//myfp = fopen (ftpFile, "w+");
	//fprintf(myfp, "cfg_upload: argv[0]=%s argv[1]=%s filelen=%d\n", argv[0], argv[1], filelen);
	//fflush(myfp);
	// file offset
	offset = qbuf->msg.arg1;
#if defined(ENABLE_SIGNATURE)
	offset += sizeof(SIGHDR);
#endif

	
	if ((fp = fopen (qbuf->msg.mtext, "rb")) == NULL) {
		printf("File %s open fail\n", qbuf->msg.mtext);
		return;
	}
	
	if (fstat (fileno(fp), &st) < 0) {
		fclose(fp);
		return;
	}
	
	if (fseek(fp, offset, SEEK_SET)==-1) {
		fclose(fp);
		return;
	}
	
	fsize = st.st_size-offset;
	if (fsize <= 0) {
		fclose(fp);
		return;
	}

	
	/*ql:20080729 START: check the validity of image, just check key*/
#ifndef MULTI_IC_SUPPORT
	{
		// simple check for image header. Making it backward compatible for now.
		// Andrew
		if ((1==fread(&imgHdr, sizeof(imgHdr), 1, fp)) &&
			(APPLICATION_IMAGE == imgHdr.key)) {
			//fseek(fp, sizeof(IMGHDR), SEEK_SET);
			//fsize = st.st_size - sizeof(IMGHDR);
			fsize -= sizeof(IMGHDR);
		} else {
			//fseek(fp, 0, SEEK_SET);
			fseek(fp, offset, SEEK_SET);
		}		
	}
#else
	// simple check for image header. Making it backward compatible for now.
	// Andrew
	key = getImgKey();
	if ((1==fread(&imgHdr, sizeof(imgHdr), 1, fp)) &&
		((key == (imgHdr.key & key)) && (((imgHdr.key>>28)&0xf) == ((key>>28)&0xf)))) {
		//fseek(fp, sizeof(IMGHDR), SEEK_SET);
		//fsize = st.st_size - sizeof(IMGHDR);
		printf("img with header!\n");
		fsize -= sizeof(IMGHDR);
	} else {
		//fseek(fp, 0, SEEK_SET);
		printf("img without header!\n");
		fseek(fp, offset, SEEK_SET);
	}
#endif
	/*ql:20080729 END*/
	
	//fprintf(myfp, "cfg_upload: fp=0x%x\n", fp);
	//fflush(myfp);
	printf("filesize=%d\n", fsize);
	rv = flashdrv_filewrite(fp,fsize,CODE_IMAGE_OFFSET);
	// Kaohj -- re-init fs
	if ((fp = fopen ("/proc/fs/fs_init", "w")) == NULL) {
		printf("open fs_init error\n");
	}else{
		fprintf(fp, "1\n");
		fclose(fp);
	}
	
	//printf("flash write = %d\n", rv);
	if (rv) {
	   printf("flash error!\n");
	   return;
	}	
	
	// Mason Yu
	//reboot_by_watchdog();
	
	printf("flash write completed !!\n");
	unlink(qbuf->msg.mtext);
	qbuf->request = MSG_SUCC;
	printf("The system is restarting ...\n");
	usleep(5000);
	//restart();
	reboot(RB_AUTOBOOT);
	exit(0); /* Shrug */
}

static void cfg_upgrade_mode(struct mymsgbuf *qbuf)
{
	int  spid;
	FILE *spidfile;
	struct mymsgbuf myqbuf;
	
	//printf("change to upgrade mode\n");
	if (chdir (CONF_PIDFILE_PATH) < 0){
		return;
	}
	if ((spidfile = fopen( "boa.pid", "r"))) {
		fscanf(spidfile, "%d\n", &spid);
		fclose(spidfile);
		printf("kill=%s, spid=%d\n", "boa", spid);
		kill( spid, SIGTERM);			
	}
	va_cmd("/bin/httpd",0,0);
	qbuf->request = MSG_SUCC;
}

static void cfg_check_image(struct mymsgbuf *qbuf) 
{
	FILE	*fp=NULL;
	IMGHDR imgHdr;
	unsigned int csum;
	int nRead, total = 0;
	unsigned char buf[64];
	int offset;
#if defined(ENABLE_SIGNATURE)
	SIGHDR sigHdr;
	int i;
	unsigned int hdrChksum;
#endif
	/*ql:20080729 START: check image key validity.*/
#ifdef MULTI_IC_SUPPORT
	unsigned int key;
#endif
	/*ql:20080729 END*/

	qbuf->request = MSG_FAIL;
	offset = qbuf->msg.arg1;
	
	if ((fp = fopen (qbuf->msg.mtext, "rb")) == NULL) {
		printf("File %s open fail\n", qbuf->msg.mtext);
		return;
	}

	if (fseek(fp, offset, SEEK_SET)==-1) {
		//jim should delete below fclose, otherwise will be closed twice...
		//fclose(fp);
		goto ERROR;
	}
#if defined(ENABLE_SIGNATURE)
//ql add to check if the image is right.
	memset(&sigHdr, 0, sizeof(sigHdr));
	if (1 != fread(&sigHdr, sizeof(sigHdr), 1, fp)) {
		printf("failed to read signature\n");
		goto ERROR;
	}
#endif
	
	if (1!=fread(&imgHdr, sizeof(imgHdr), 1, fp)) {
		printf("Failed to read header\n");
		goto ERROR;
	}
#ifndef ENABLE_SIGNATURE_ADV
#ifdef ENABLE_SIGNATURE
	if (sigHdr.sigLen > SIG_LEN) {
		printf("signature length error\n");
		goto ERROR;
	}
	for (i=0; i<sigHdr.sigLen; i++)
		sigHdr.sigStr[i] = sigHdr.sigStr[i] - 10;
	if (strcmp(sigHdr.sigStr, SIGNATURE)) {
		printf("signature error\n");
		goto ERROR;
	}

	hdrChksum = sigHdr.chksum;
	hdrChksum = ipchksum(&imgHdr, sizeof(imgHdr), hdrChksum);
	if (hdrChksum) {
		printf("Checksum failed, size=%d, csum=%04xh\n", sigHdr.sigLen, hdrChksum);
		goto ERROR;
	}
#endif
#endif

	/*ql:20080729 START: check the validity of image, just check key*/
#ifndef MULTI_IC_SUPPORT
	if (imgHdr.key != APPLICATION_IMAGE) {
		printf("Unknown header\n");
		goto ERROR;
	}
#else
	key = getImgKey();
	
	if ((key != (imgHdr.key & key)) || (((imgHdr.key>>28)&0xf) != ((key>>28)&0xf))) {
		printf("img key error!\n");
		goto ERROR;
	}
#endif
	/*ql:20080729 END*/

	csum = imgHdr.chksum;
	while (nRead = fread(buf, 1, sizeof(buf), fp)) {
		total += nRead;
		csum = ipchksum(buf, nRead,csum);
	}

	if (csum) {
		printf("Checksum failed, size=%d, csum=%04xh\n", total, csum);
		goto ERROR;
	}
	qbuf->request = MSG_SUCC;

ERROR:
	fclose(fp);
	return;
}

#ifdef AUTO_PVC_SEARCH_AUTOHUNT
#define MAX_PVC_SEARCH_PAIRS 16
static void cfg_start_autohunt(struct mymsgbuf *qbuf) 
{
	FILE *fp;

	MIB_AUTO_PVC_SEARCH_Tp entryP;
	//MIB_AUTO_PVC_SEARCH_T Entry;
	unsigned int entryNum,i;
	unsigned char tmp[12], tmpBuf[MAX_PVC_SEARCH_PAIRS*12];

	entryNum = _mib_chain_total(MIB_AUTO_PVC_SEARCH_TBL);
	memset(tmpBuf, 0, sizeof(tmpBuf));
	for(i=0;i<entryNum; i++) {
		memset(tmp, 0, 12);
		entryP = _mib_chain_get(MIB_AUTO_PVC_SEARCH_TBL, i);
		if (!entryP)
			continue;
		//if (!_mib_chain_get(MIB_AUTO_PVC_SEARCH_TBL, i, (void *)&Entry))
		//	continue;
		sprintf(tmp,"(%d %d)", entryP->vpi, entryP->vci); 
		strcat(tmpBuf, tmp);
		
	}
	//printf("StartSarAutoPvcSearch: inform SAR %s\n", tmpBuf);
	

	if (fp=fopen("/proc/AUTO_PVC_SEARCH", "w") ) 
	{				
		fprintf(fp, "1%s\n", tmpBuf);	//write pvc list stored in flash to SAR driver
//		printf("StartSarAutoPvcSearch: Inform SAR driver to start auto-pvc-search\n");

		fclose(fp);
	} else {
		printf("Open /proc/AUTO_PVC_SEARCH failed! Can't start SAR driver doing auto-pvc-search\n");
	}
	
	qbuf->request = MSG_SUCC;
}
#endif

// Added by Mason Yu
#ifdef CONFIG_USER_DDNS
extern sem_t semDdnsc;
char g_ddns_ifname[10];

static void cfg_ddns_ctrl(struct mymsgbuf *qbuf)
{	
	strcpy(g_ddns_ifname, qbuf->msg.mtext);	
	sem_post(&semDdnsc);
	qbuf->request = MSG_SUCC;
	return;
}
#endif

// Kaohj -- transform received file to xml file
const char tmpxml[] = "/tmp/tmpdecryxor.xml";
const char xml_name[] = "/tmp/config.xml";
static void cfg_file2xml(struct mymsgbuf *qbuf)
{
	FILE	*fpin=NULL;
	char LINE[256], str[64], str2[64];   
	struct mymsgbuf myqbuf;
	MSGFile_T *pFile;
	
	qbuf->request = MSG_FAIL;
	
	pFile = (MSGFile_T *)&qbuf->msg.mtext[0];
	#ifdef XOR_ENCRYPT
	rename(pFile->fromName, tmpxml);
	xor_encrypt((char *)tmpxml, pFile->toName);
	unlink(tmpxml);
	#else
	if (strcmp(pFile->fromName, pFile->toName))
		rename(pFile->fromName, pFile->toName);
	#endif
	
	if ((fpin = fopen (xml_name, "rb")) == NULL)
		return;
	
	fseek(fpin, 0, SEEK_SET);
	fgets(LINE, sizeof(LINE), fpin);
	fclose(fpin);
	sprintf(str, "%s\n", CONFIG_HEADER);
	// Support Dos Format
	sprintf(str2, "%s\r\n", CONFIG_HEADER);	                 
	if (strcmp(LINE, str) && strcmp(LINE, str2)) { // header error  
		unlink(xml_name);
		return;
	}
	
	qbuf->request = MSG_SUCC;
}

// Kaohj -- transform xml file to backup file
static void cfg_xml2file(struct mymsgbuf *qbuf)
{
	struct mymsgbuf myqbuf;
	MSGFile_T *pFile;
	
	qbuf->request = MSG_FAIL;
	
	pFile = (MSGFile_T *)&qbuf->msg.mtext[0];
	#ifdef XOR_ENCRYPT
	rename(pFile->fromName, tmpxml);
	xor_encrypt((char *)tmpxml, pFile->toName);
	unlink(tmpxml);
	#else
	if (strcmp(pFile->fromName, pFile->toName))
		rename(pFile->fromName, pFile->toName);
	#endif
	
	qbuf->request = MSG_SUCC;
}

#endif

