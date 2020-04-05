/*
 * sysconfig.h --- header file for configuration server
 * --- By Kaohj
 */


#ifndef INCLUDE_SYSCONFIG_H
#define INCLUDE_SYSCONFIG_H
#include "mib.h"
#include "mibtbl.h"
//#define		INCLUDE_DEFAULT_VALUE	1

int mib_lock(void); /* Update RAM setting to flash */
int mib_unlock(void); /* Update RAM setting to flash */
int mib_update(CONFIG_DATA_T data_type, CONFIG_MIB_T flag); /* Update RAM setting to flash */
int mib_load(CONFIG_DATA_T data_type, CONFIG_MIB_T flag); /* Load flash setting to RAM */
int mib_get(int id, void *value);
int mib_set(int id, void *value);
int mib_set_flash(int id, void *value); // set mib value into system and flash
int mib_info_id(int id, mib_table_entry_T *info);
int mib_info_index(int index, mib_table_entry_T *info);
int mib_backup(CONFIG_MIB_T flag); /* backup the running mib into the backup buffer */
int mib_restore(CONFIG_MIB_T flag); /* restore the backup buffer into the running mib */
//TYPE_T mib_type(int id);
int mib_chain_total(int id);
int mib_chain_get(int id, unsigned int recordNum, void *ptr); /* get the specified chain record */
int mib_chain_add(int id, void* ptr); /* add chain record */
int mib_chain_add_flash(int id, void* ptr); /* add chain record into system and flash*/
int mib_chain_delete(int id, unsigned int recordNum); /* delete the specified chain record */
int mib_chain_clear(int id); /* clear chain record */
// for message logging
int mib_chain_update(int id, void* ptr, unsigned int recordNum); /* log updating the specified chain record */
int mib_chain_info_id(int id, mib_chain_record_table_entry_T *info);
int mib_chain_info_index(int index, mib_chain_record_table_entry_T *info);
int mib_chain_info_name(char *name, mib_chain_record_table_entry_T *info);
int mib_chain_desc_id(int id, mib_chain_member_entry_T *desc);

int cmd_reboot(void);
int cmd_get_bootmode(void);
int cmd_killproc(unsigned int mask);
int cmd_upload(const char *filename, int offset);
int cmd_upgrade_mode(int mode);
int cmd_check_image(const char *filename, int offset);
int cmd_start_autohunt(void);
int cmd_file2xml(const char *filename, const char *xmlname);
int cmd_xml2file(const char *xmlname, const char *filename);
///added by ql
#ifdef	RESERVE_KEY_SETTING
int mib_retrive_table(int id);
int mib_retrive_chain(int id);
int mib_clear(CONFIG_MIB_T type);
#endif
//#ifdef MSG_WLAN_RESTART_SUPPORT
int sendMsg2Boa(int cmd, void *value, unsigned int length);
//#endif
#endif // INCLUDE_SYSCONFIG_H

