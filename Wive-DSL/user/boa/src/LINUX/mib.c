/*
 *      Routines to handle MIB operation
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *      Authors: Dick Tam	<dicktam@realtek.com.tw>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include <arpa/inet.h>
#include <netinet/in.h>

/* for open(), lseek() */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>

#ifdef EMBED
#include <getopt.h>
#endif


#include "mib.h"
#include "mibtbl.h"

#ifdef EMBED
#include "rtl_flashdrv.h"			// ucLinux flash driver
#include "rtl_flashdrv_api.h"		// ucLinux flash driver
#endif

#ifdef WEB_DEBUG_MSG
#define	TRACE	printf
#else
#define	TRACE
#endif

static MIB_T cs_mib;
static HW_MIB_T hs_mib;

static PARAM_HEADER_T cs_header;
static PARAM_HEADER_T ds_header;
static PARAM_HEADER_T hs_header;

unsigned char cs_valid = 0;
unsigned char ds_valid = 0;
unsigned char hs_valid = 0;

BOOT_TYPE_T __boot_mode;


/*
 * Utility functions
 */
#ifdef INCLUDE_DEFAULT_VALUE
static int _is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}

static int string_to_hex(char *string, unsigned char *key, int len)
{
	char tmpBuf[4];
	int idx, ii=0;
	for (idx=0; idx<len; idx+=2) {
		tmpBuf[0] = string[idx];
		tmpBuf[1] = string[idx+1];
		tmpBuf[2] = 0;
		if ( !_is_hex(tmpBuf[0]) || !_is_hex(tmpBuf[1]))
			return 0;

		key[ii++] = (unsigned char) strtol(tmpBuf, (char**)NULL, 16);
	}
	return 1;
}
#endif







extern int Encode(unsigned char *ucInput, unsigned int inLen, unsigned char *ucOutput);
extern int Decode(unsigned char *ucInput, unsigned int inLen, unsigned char *ucOutput);

/* ------------------------------------------------------------
 * Flash File System Utility functions
 * ------------------------------------------------------------ */
/*
 * Flash read function
 */
int __mib_flash_read(char *buf, int offset, int len)
{
#ifdef EMBED
	void *pDst = (void*)(FLASH_BASE + offset);

	TRACE("Flash read from %x len=%d\n",offset,len);
#if defined(COMPRESS_CURRENT_SETTING)
	///ql add: to compress current setting
	if ( offset == CURRENT_SETTING_OFFSET ) {//compress current setting
		// large memory demand on stack is not a good idea. jim
		//char compFile[CURRENT_SETTING_MAX_REAL_LEN];
		//char expFile[CURRENT_SETTING_LEN];
		//we assume the compress rate is 10:1
		int zipRate=10;
		char *compFile, *expFile;
		unsigned int expandLen=0, compLen=0;
				
		flashdrv_read(&compLen, pDst, 4);
		//printf("compressed read len:%d\n", compLen);

		if ( (compLen > 0) && (compLen <= CURRENT_SETTING_MAX_REAL_LEN) ) {
			compFile=malloc(compLen+4);
			if(compFile==NULL)
				return 0;
			expFile=malloc(zipRate*compLen);
			if(expFile==NULL)
			{
				free(compFile);
				return 0;
			}

			flashdrv_read(compFile, pDst, compLen+4);

			expandLen = Decode(compFile+4, compLen, expFile);
			//printf("expandLen read len: %d\n", expandLen);

			memcpy(buf, expFile, len);
			free(expFile);
			free(compFile);
			return 1;
		} else {
			return 0;
		}
	} else //don't compress
		if((flashdrv_read(buf, pDst, (uint32)len)) == 0)
			return 1;
#else
	if((flashdrv_read(buf, pDst, (uint32)len)) == 0)
		return 1;
#endif

	return 0;
#else

	int fh;
	int ok=1;

	fh = open(FLASH_DEVICE_NAME, O_RDWR);
	if ( fh == -1 )
		return 0;

	lseek(fh, offset, SEEK_SET);

	if ( read(fh, buf, len) != len)
		ok = 0;

	close(fh);

	TRACE("Flash read from %x len=%d\n",offset,len);
	return ok;
#endif
}

/*
 * Flash write function
 */
int __mib_flash_write(char *buf, int offset, int len)
{
#ifdef EMBED
	void *pDst = (void*)(FLASH_BASE + offset);

	TRACE("Flash write to %x len=%d\n",offset,len);
#if defined(COMPRESS_CURRENT_SETTING)
	if ( offset == CURRENT_SETTING_OFFSET ) {//compress current setting
		///first 4 bytes indicates the len of compressed setting
		char compFile[CURRENT_SETTING_MAX_REAL_LEN];
		int compLen;

		//printf("original write len: %d\n", len);
		
		//va_cmd("/bin/gzip", 2, 1, "-f", "/var/Tmp");
		memset(compFile, 0, CURRENT_SETTING_MAX_REAL_LEN);
		compLen = Encode(buf, len, compFile+4);

		memcpy(compFile, &compLen, 4);

		//printf("compress write len: %d\n", compLen);

		//write to flash
		compLen += 4;
		if (compLen > CURRENT_SETTING_MAX_REAL_LEN)
			return 0;
		if((flashdrv_updateImg(compFile, pDst, compLen) == 0)) {
			//printf("write success!\n");
			return 1;
		}
	} else {//don't compress
		//if (len > CURRENT_SETTING_MAX_REAL_LEN)
		//	return 0;
		if((flashdrv_updateImg(buf, pDst, (uint32)len)) == 0)
			return 1;
	}
#else
	if((flashdrv_updateImg(buf, pDst, (uint32)len)) == 0)
		return 1;
#endif

	return 0;
#else
	int fh;

	fh = open(FLASH_DEVICE_NAME, O_RDWR);

	if ( fh == -1 )
		fh = open(FLASH_DEVICE_NAME, O_CREAT|O_RDWR);

	if ( fh == -1 )
	{
		TRACE("Error: flash_write open fail! (Dev = %s)\n",FLASH_DEVICE_NAME);
		return 0;
	}

	lseek(fh, offset, SEEK_SET);

	if ( write(fh, buf, len) != len)
	{
		TRACE("Error: flash_write write fail!\n");
		return 0;
	}

	close(fh);
	sync();

	TRACE("Flash write to %x len=%d\n",offset,len);
	return 1;
#endif
}

/*
 * Write file into flash
 */
int __mib_file_write(CONFIG_DATA_T data_type, unsigned char* ptr, int len)
{
	switch(data_type)
	{
		case CURRENT_SETTING:
			return __mib_flash_write(ptr, CURRENT_SETTING_OFFSET, len);
		case DEFAULT_SETTING:
			return __mib_flash_write(ptr, DEFAULT_SETTING_OFFSET, len);
		case HW_SETTING:
			return __mib_flash_write(ptr, HW_SETTING_OFFSET, len);
		default:
			return 0;
	}
}

/*
 * Read file from flash
 */
int __mib_file_read(CONFIG_DATA_T data_type, unsigned char* ptr, int len)
{
	switch(data_type)
	{
		case CURRENT_SETTING:
			return __mib_flash_read(ptr, CURRENT_SETTING_OFFSET, len);
		case DEFAULT_SETTING:
			return __mib_flash_read(ptr, DEFAULT_SETTING_OFFSET, len);
		case HW_SETTING:
			return __mib_flash_read(ptr, HW_SETTING_OFFSET, len);
		default:
			return 0;
	}
}










/* ------------------------------------------------------------
 * MIB Chain Record Utility functions
 * ------------------------------------------------------------ */
int __mib_chain_mib2tbl_id(int id)
{
	int i;
	// search chain record mib table
	for (i=0; mib_chain_record_table[i].id; i++) {
		if ( mib_chain_record_table[i].id == id )
		{
			break;
		}
	}

	if ( mib_chain_record_table[i].id != id )		// ID NOT found
	{
		TRACE("chain record id(%d) NOT found!\n",id);
		return -1;
	}

	return i;
}

void __mib_chain_print(int id)
{
	int i = __mib_chain_mib2tbl_id(id);
	MIB_CHAIN_ENTRY_Tp pChain = NULL;

	if(i == -1)
		return;

	pChain = mib_chain_record_table[i].pChainEntry;	
	while(pChain != NULL)
	{
		TRACE("chain %s Entry=%08x Value=%08x Next=%08x \n", mib_chain_record_table[i].name, (unsigned int)pChain, (unsigned int)pChain->pValue, (unsigned int)pChain->pNext);

		pChain = pChain->pNext;
	}

}

 unsigned int __mib_chain_total(int id)
{
	int i = __mib_chain_mib2tbl_id(id);
	unsigned int rValue = 0;
	MIB_CHAIN_ENTRY_Tp pChain = NULL;

	if(i == -1)
		return 0;

	pChain = mib_chain_record_table[i].pChainEntry;	
	while(pChain != NULL)
	{
		pChain = pChain->pNext;
		rValue++;		
	}

	return rValue;
}

void __mib_chain_clear(int id)
{
	int i = __mib_chain_mib2tbl_id(id);
	MIB_CHAIN_ENTRY_Tp pChain = NULL;
	MIB_CHAIN_ENTRY_Tp pTemp = NULL;

	if(i == -1)
		return;

	pChain = mib_chain_record_table[i].pChainEntry;	
	while(pChain != NULL)
	{
		TRACE("chain clear Entry=%08x Value=%08x\n", (unsigned int)pChain, (unsigned int)pChain->pValue);
		// log message
		//syslog(LOG_INFO, "mib_chain_clear: %s", mib_chain_record_table[i].name);
		
		pTemp = pChain->pNext;
		free(pChain->pValue);
		free(pChain);
		pChain = pTemp;
	}
	mib_chain_record_table[i].pChainEntry = NULL;
}

int __mib_chain_add(int id, unsigned char* ptr)
{
	int i = __mib_chain_mib2tbl_id(id);
	// for log message
	int k = 0;
	MIB_CHAIN_ENTRY_Tp pNew = NULL;
	MIB_CHAIN_ENTRY_Tp pLast = NULL;

	if(i == -1)
		return 0;

	pNew = malloc(sizeof(MIB_CHAIN_ENTRY_T));
	if(pNew == NULL)
		return 0;

	pNew->pValue = malloc(mib_chain_record_table[i].per_record_size);
	if(pNew->pValue == NULL)
	{
		free(pNew);
		return 0;
	}

	TRACE("chain add Entry=%08x Value=%08x\n", (unsigned int)pNew, (unsigned int)pNew->pValue);

	memcpy(pNew->pValue, ptr, mib_chain_record_table[i].per_record_size);
	pNew->pNext = NULL;

	if(mib_chain_record_table[i].pChainEntry == NULL) {
		mib_chain_record_table[i].pChainEntry = pNew;
	} else {
		pLast = mib_chain_record_table[i].pChainEntry;
		k++;
		while(pLast->pNext != NULL) {
			pLast = pLast->pNext;
			k++;
		}

		pLast->pNext = pNew;
	}
	
	// log message
	//syslog(LOG_INFO, "mib_chain_add: %s on entry %d", mib_chain_record_table[i].name, k);
	return 1;
}

int __mib_chain_delete(int id, unsigned int recordNum)
{
	int i = __mib_chain_mib2tbl_id(id);
	MIB_CHAIN_ENTRY_Tp pLast = NULL;
	MIB_CHAIN_ENTRY_Tp pCurrent = NULL;

	if(i == -1)
		return 0;

	if(recordNum >= __mib_chain_total(id))
	{
		TRACE("can not delete record(%d)! max record(%d)\n",recordNum,__mib_chain_total(id));
		return 0;
	}

	
	// log message
	//syslog(LOG_INFO, "mib_chain_delete: %s on entry %d", mib_chain_record_table[i].name, recordNum);
	if(recordNum==0)
	{
		pCurrent = mib_chain_record_table[i].pChainEntry;
		mib_chain_record_table[i].pChainEntry = pCurrent->pNext;
		goto ok;
	}

	pLast = pCurrent = mib_chain_record_table[i].pChainEntry;	
	while(recordNum>0)
	{
		recordNum--;
		pLast = pCurrent;
		pCurrent = pCurrent->pNext;
	}

	pLast->pNext = pCurrent->pNext;
ok:
	TRACE("chain delete Entry=%08x Value=%08x\n", (unsigned int)pCurrent, (unsigned int)pCurrent->pValue);	
	
	free(pCurrent->pValue);
	free(pCurrent);
	return 1;	
}

unsigned char* __mib_chain_get(int id, unsigned int recordNum)
{
	int i = __mib_chain_mib2tbl_id(id);
	MIB_CHAIN_ENTRY_Tp pCurrent = NULL;

	if(i == -1)
		return 0;

	if(recordNum >= __mib_chain_total(id))
	{
		TRACE("can not get record(%d)! max record(%d)\n",recordNum,__mib_chain_total(id));
		return 0;
	}

	pCurrent = mib_chain_record_table[i].pChainEntry;	
	while(recordNum>0)
	{
		recordNum--;
		pCurrent = pCurrent->pNext;
	}

	return pCurrent->pValue;
}

unsigned int __mib_chain_all_table_size(CONFIG_DATA_T data_type)
{
	unsigned int size = 0;
	unsigned int chainSize;
	int i;
	
	for (i=0; mib_chain_record_table[i].id; i++) {
		if(mib_chain_record_table[i].mib_type == data_type)
		{
			chainSize = __mib_chain_total(mib_chain_record_table[i].id);
			if(chainSize > 0)
			{
				size += chainSize*mib_chain_record_table[i].per_record_size + sizeof(MIB_CHAIN_RECORD_HDR_T);
			}
		}
	}

	return size;
}

void __mib_chain_all_table_clear(CONFIG_DATA_T data_type)
{
	int i;
	
	for (i=0; mib_chain_record_table[i].id; i++) {
		if(mib_chain_record_table[i].mib_type == data_type)
		{
			__mib_chain_clear(mib_chain_record_table[i].id);
		}
	}

}








/* ------------------------------------------------------------
 * MIB Table Utility functions
 * ------------------------------------------------------------ */
unsigned char * __mib_get_mib_tbl(CONFIG_DATA_T data_type)
{
	switch(data_type)
	{
		case DEFAULT_SETTING:
		case CURRENT_SETTING:
			return (unsigned char *) &cs_mib;;
		case HW_SETTING:
			return (unsigned char *) &hs_mib;;
		default:
			return NULL;
	}
}

PARAM_HEADER_Tp __mib_get_mib_header(CONFIG_DATA_T data_type)
{
	switch(data_type)
	{
		case CURRENT_SETTING:
			return &cs_header;;
		case DEFAULT_SETTING:
			return &ds_header;;
		case HW_SETTING:
			return &hs_header;;
		default:
			return NULL;
	}
}

void __mib_init_mib_header(void)
{
	PARAM_HEADER_Tp pHeader;

	pHeader = __mib_get_mib_header(CURRENT_SETTING);
	memcpy(pHeader->signature, CS_CONF_SETTING_SIGNATURE_TAG, SIGNATURE_LEN);
	pHeader->version = FLASH_FILE_SYSTEM_VERSION;
	pHeader->checksum = 0xFF;
	pHeader->len = sizeof(MIB_T);

	pHeader = __mib_get_mib_header(DEFAULT_SETTING);
	memcpy(pHeader->signature, DS_CONF_SETTING_SIGNATURE_TAG, SIGNATURE_LEN);
	pHeader->version = FLASH_FILE_SYSTEM_VERSION;
	pHeader->checksum = 0xFF;
	pHeader->len = sizeof(MIB_T);

	pHeader = __mib_get_mib_header(HW_SETTING);
	memcpy(pHeader->signature, HS_CONF_SETTING_SIGNATURE_TAG, SIGNATURE_LEN);
	pHeader->version = FLASH_FILE_SYSTEM_VERSION;
	pHeader->checksum = 0xFF;
	pHeader->len = sizeof(HW_MIB_T);	
}

 unsigned int __mib_content_min_size(CONFIG_DATA_T data_type)
{
	switch(data_type)
	{
		case CURRENT_SETTING:
			return CURRENT_SETTING_MIN_LEN;
		case DEFAULT_SETTING:
			return DEFAULT_SETTING_MIN_LEN;
		case HW_SETTING:
			return HW_SETTING_MIN_LEN;
		default:
			return 0;
	}
}

 unsigned int __mib_content_max_size(CONFIG_DATA_T data_type)
{
	switch(data_type)
	{
		case CURRENT_SETTING:
			return CURRENT_SETTING_MAX_LEN;
		case DEFAULT_SETTING:
			return DEFAULT_SETTING_MAX_LEN;
		case HW_SETTING:
			return HW_SETTING_MAX_LEN;
		default:
			return 0;
	}
}
 
/*
 * Read header from flash
 */
int __mib_header_read(CONFIG_DATA_T data_type, PARAM_HEADER_Tp pHeader)
{
	return __mib_file_read(data_type, (unsigned char*)pHeader, sizeof(PARAM_HEADER_T));
}

/*
 * Check header
 */
int __mib_header_check(CONFIG_DATA_T data_type, PARAM_HEADER_Tp pHeader)
{
	switch(data_type)
	{
		case CURRENT_SETTING:
			if( memcmp(pHeader->signature, CS_CONF_SETTING_SIGNATURE_TAG, SIGNATURE_LEN) != 0)
			{
				TRACE("header signature error! \n");
				return 0;
			}
			
			if((pHeader->len < CURRENT_SETTING_MIN_LEN) ||(pHeader->len > CURRENT_SETTING_MAX_LEN))
			{
				TRACE("header len error! %d (%d,%d)\n",pHeader->len, CURRENT_SETTING_MIN_LEN, CURRENT_SETTING_MAX_LEN);
				return 0;
			}

			return 1;
		case DEFAULT_SETTING:
			if( memcmp(pHeader->signature, DS_CONF_SETTING_SIGNATURE_TAG, SIGNATURE_LEN) != 0 )
			{
				TRACE("header signature error! \n");
				return 0;
			}
			
			if((pHeader->len < DEFAULT_SETTING_MIN_LEN) ||(pHeader->len > DEFAULT_SETTING_MAX_LEN))
			{
				TRACE("header len error! %d (%d,%d)\n",pHeader->len, DEFAULT_SETTING_MIN_LEN, DEFAULT_SETTING_MAX_LEN);
				return 0;
			}

			return 1;
		case HW_SETTING:
			if( memcmp(pHeader->signature, HS_CONF_SETTING_SIGNATURE_TAG, SIGNATURE_LEN) != 0 )
			{
				TRACE("header signature error! \n");
				return 0;
			}
			
			if((pHeader->len < HW_SETTING_MIN_LEN) ||(pHeader->len > HW_SETTING_MAX_LEN))
			{
				TRACE("header len error! %d (%d,%d)\n",pHeader->len, HW_SETTING_MIN_LEN, HW_SETTING_MAX_LEN);
				return 0;
			}

			return 1;
		default:
			return 0;
	}
}

/*
 * Decode content and verify checksum
 */
int __mib_content_decod_check(CONFIG_DATA_T data_type, PARAM_HEADER_Tp pHeader, unsigned char* ptr)
{
	DECODE_DATA(ptr,  pHeader->len);

	if(pHeader->checksum != CHECKSUM(ptr, pHeader->len))
	{
		return 0;
	}

	return 1;
}

/*
 * Encode content and do checksum
 */
void __mib_content_encod_check(CONFIG_DATA_T data_type, PARAM_HEADER_Tp pHeader, unsigned char* ptr)
{	
	pHeader->checksum = CHECKSUM(ptr, pHeader->len);
	ENCODE_DATA(ptr,  pHeader->len);
}

int __mib_chain_record_content_decod(unsigned char* ptr, unsigned int len)
{
	unsigned int idx = 0;

	while(idx < len)
	{
		int i;
		int numOfRecord;
		MIB_CHAIN_RECORD_HDR_Tp pCRHeader = (MIB_CHAIN_RECORD_HDR_Tp) (ptr + idx);

		idx += sizeof(MIB_CHAIN_RECORD_HDR_T);

		// search chain record mib table
		for (i=0; mib_chain_record_table[i].id; i++) {
			if ( mib_chain_record_table[i].id == pCRHeader->id )
			{
				break;
			}
		}
		
		if ( mib_chain_record_table[i].id == 0 )		// ID NOT found
		{
			TRACE("chain record id(%d) NOT found!\n",pCRHeader->id);
			return 0;
		}

		if((idx + pCRHeader->len) > len)	// check record size
		{
			TRACE("invalid chain record size! Header len(%u), len(%u)\n",pCRHeader->len, len - idx);
			return 0;
		}

		if((pCRHeader->len % mib_chain_record_table[i].per_record_size) != 0)		// check record size
		{
			TRACE("invalid chain record size! len(%d), record size(%d)\n",pCRHeader->len, mib_chain_record_table[i].per_record_size);
			return 0;
		}

		numOfRecord = pCRHeader->len / mib_chain_record_table[i].per_record_size;

//		TRACE("chain record decod %s, %d record\n",mib_chain_record_table[i].name, numOfRecord);
		while(numOfRecord > 0)
		{
			if(__mib_chain_add(pCRHeader->id, ptr+idx) != 1)
			{
				TRACE("add chain record fail!\n");
				return 0;
			}
			
			numOfRecord--;
			idx += mib_chain_record_table[i].per_record_size;
		}		
	}	

	return 1;
}

int __mib_chain_record_content_encod(CONFIG_DATA_T data_type, unsigned char* ptr, unsigned int len)
{
	int i;
	unsigned int idx = 0;
	unsigned int chainSize;
	unsigned int entrySize;
	MIB_CHAIN_RECORD_HDR_Tp pCRHeader = NULL;

	for (i=0; mib_chain_record_table[i].id; i++) {
		if(mib_chain_record_table[i].mib_type == data_type)
		{
			chainSize = __mib_chain_total(mib_chain_record_table[i].id);
			if(chainSize > 0)
			{
				unsigned int currentRecord=0;
				MIB_CHAIN_ENTRY_Tp pChain = NULL;

				entrySize = chainSize*mib_chain_record_table[i].per_record_size + sizeof(MIB_CHAIN_RECORD_HDR_T);
				if((entrySize + idx) > len)
					return 0;

				// prepare chain record header
				pCRHeader = (MIB_CHAIN_RECORD_HDR_Tp) (ptr + idx);
				pCRHeader->id =  mib_chain_record_table[i].id;
				pCRHeader->len = chainSize*mib_chain_record_table[i].per_record_size;
				idx+=sizeof(MIB_CHAIN_RECORD_HDR_T);

				pChain = mib_chain_record_table[i].pChainEntry;
						
				while(pChain != NULL)
				{
					memcpy(ptr+idx, pChain->pValue, mib_chain_record_table[i].per_record_size);
					pChain = pChain->pNext;

					idx+=mib_chain_record_table[i].per_record_size;
					currentRecord++;
				}

				if(currentRecord!=chainSize)
				{
					TRACE("(currentRecord!=chainSize) \n");
					return 0;
				}
			}
		}
	}

	if(len!=idx)
	{
		TRACE("(len!=idx) \n");
		return 0;
	}

	return 1;
}

// flag:
//	0: all mib setting (table and chain)
//	1: mib table
//	2: mib chain
int __mib_content_read(CONFIG_DATA_T data_type, CONFIG_MIB_T flag)
{
	PARAM_HEADER_T header;
	PARAM_HEADER_Tp pHeader = NULL;
	unsigned char* pMibTbl = NULL;

	unsigned char* pFile = NULL;
	unsigned char* pContent = NULL;
	unsigned char* pVarLenTable = NULL;

	unsigned int fileSize = 0;
	unsigned int contentMinSize = 0;
	unsigned int varLenTableSize = 0;

	if(__mib_header_read(data_type, &header) != 1)
	{
		TRACE("(__mib_header_read(data_type, &header) != 1)  \n");
		goto error;
	}

	if(__mib_header_check(data_type, &header) != 1)
	{
		TRACE("(__mib_header_check(data_type, &header) != 1)  \n");
		goto error;
	}

	fileSize = header.len + sizeof(PARAM_HEADER_T);

	pFile = malloc(fileSize);
	if ( pFile == NULL )
	{
		TRACE("( pFile == NULL )  \n");
		goto error;
	}
	
	if(__mib_file_read(data_type, pFile, fileSize) != 1)
	{
		TRACE("(__mib_file_read(data_type, pFile, fileSize) != 1)  \n");
		goto error;
	}

	pContent = &pFile[sizeof(PARAM_HEADER_T)];	// point to start of MIB data 

	if(__mib_content_decod_check(data_type, &header, pContent) != 1)
	{
		TRACE("(__mib_content_decod_check(data_type, &header, pContent) != 1) \n ");
		goto error;
	}

	contentMinSize = __mib_content_min_size(data_type);

	// save header and content
	pHeader = __mib_get_mib_header(data_type);
	*pHeader = header;

	if (flag == CONFIG_MIB_ALL || flag == CONFIG_MIB_TABLE) {
		pMibTbl = __mib_get_mib_tbl(data_type);
		memcpy(pMibTbl, pContent, contentMinSize);
	}
	
	if (flag == CONFIG_MIB_TABLE) { // mib table only
		free(pFile);
		return 1;
	}
	
	// Kaohj, default and current setting share the same memory,
	// and the mib_chain_record_table is for CURRENT_SETTING
	//__mib_chain_all_table_clear(data_type);
	__mib_chain_all_table_clear(CURRENT_SETTING);
	varLenTableSize = header.len - contentMinSize;
	if(varLenTableSize > 0)
	{
		pVarLenTable = &pContent[contentMinSize];	// point to start of variable length MIB data 

		// parse variable length MIB data
		if( __mib_chain_record_content_decod(pVarLenTable, varLenTableSize) != 1)
		{
			TRACE("( __mib_chain_record_content_decod(pVarLenTable, varLenTableSize) != 1)  \n");
			goto error;
		}
	}

	free(pFile);
	return 1;
error:
	if(pFile) free(pFile);
	TRACE("__mib_content_read fail!\n");
	return 0;
}

unsigned int __mib_content_size(CONFIG_DATA_T data_type)
{
	unsigned int chainRecordSize, mibTblSize;

	mibTblSize = __mib_content_min_size(data_type);
	chainRecordSize = __mib_chain_all_table_size(data_type);

	return (chainRecordSize + mibTblSize + sizeof(PARAM_HEADER_T));
}

int __mib_content_write_to_raw(CONFIG_DATA_T data_type, unsigned char* buf, unsigned int len)
{
	unsigned int chainRecordSize, mibTblSize, totalSize;
	PARAM_HEADER_Tp pHeader = __mib_get_mib_header(data_type);

	if(buf == 0)
		goto error;

	unsigned char* pFile = buf;
	unsigned char* pContent = NULL;
	unsigned char* pVarLenTable = NULL;

	unsigned char* pMibTbl = NULL;

	mibTblSize = __mib_content_min_size(data_type);
	chainRecordSize = __mib_chain_all_table_size(data_type);

	pHeader->len = chainRecordSize + mibTblSize;
	totalSize = pHeader->len + sizeof(PARAM_HEADER_T);

	if(totalSize != len)
		goto error;

	// check size
	if(totalSize > __mib_content_max_size(data_type))
	{
		TRACE("__mib_content_write Total size(%d) > Max size(%d)\n",totalSize ,__mib_content_max_size(data_type));
		goto error;
	}
	
	pContent = &pFile[sizeof(PARAM_HEADER_T)];	// point to start of MIB data 
	pMibTbl = __mib_get_mib_tbl(data_type);
	memcpy(pContent, pMibTbl, mibTblSize);

	if(chainRecordSize>0)
	{
		pVarLenTable = &pContent[mibTblSize];	// point to start of variable length MIB data 

		if(__mib_chain_record_content_encod(data_type, pVarLenTable, chainRecordSize) != 1)
			goto error;
	}	

	__mib_content_encod_check(data_type, pHeader, pContent);

	// copy header
	memcpy(pFile, (unsigned char*)pHeader, sizeof(PARAM_HEADER_T));
	return 1;
		
error:
	TRACE("__mib_content_write_to_raw fail!\n");
	return 0;
}

int __mib_content_write(CONFIG_DATA_T data_type)
{
	unsigned int totalSize;
	unsigned char* pFile = NULL;

#ifdef EMBED
	if(data_type == HW_SETTING) {	// ds, hs will share the same flash block
		PARAM_HEADER_T header;
		unsigned int totalSize;
		unsigned char update = 1;
		unsigned int hs_size;
		totalSize = __mib_content_max_size(HW_SETTING) + __mib_content_max_size(DEFAULT_SETTING);

		if(__mib_header_read(DEFAULT_SETTING, &header) != 1)
		{
			TRACE("Error:  __mib_header_read fail \n");
			update = 0;
		}

		if(__mib_header_check(DEFAULT_SETTING, &header) != 1)
		{
			TRACE("Error:  __mib_header_check fail \n");
			update = 0;
		}

		pFile = malloc(totalSize);
		if ( pFile == NULL )
		{
			TRACE("Error:  malloc fail \n");
			goto error;
		}

		if(update)
		{
			__mib_content_read_to_raw(DEFAULT_SETTING, pFile, header.len + sizeof(PARAM_HEADER_T));
		}

		hs_size = __mib_content_size(HW_SETTING);
		if(__mib_content_write_to_raw(HW_SETTING, &pFile[__mib_content_max_size(DEFAULT_SETTING)], hs_size) != 1)
		{
			TRACE("Error:  __mib_content_write_to_raw fail \n");
			goto error;
		}

		if(__mib_file_write(DEFAULT_SETTING, pFile, totalSize) != 1)
		{
			TRACE("Error:  __mib_file_write fail \n");
			goto error;
		}
	} 
	else if (data_type == DEFAULT_SETTING) {	// ds, hs will share the same flash block
		PARAM_HEADER_T header;
		unsigned int totalSize;
		unsigned char update = 1;
		unsigned int ds_size;
		totalSize = __mib_content_max_size(HW_SETTING) + __mib_content_max_size(DEFAULT_SETTING);

		if(__mib_header_read(HW_SETTING, &header) != 1)
		{
			TRACE("Error:  __mib_header_read fail \n");
			update = 0;
		}

		if(__mib_header_check(HW_SETTING, &header) != 1)
		{
			TRACE("Error:  __mib_header_check fail \n");
			update = 0;
		}

		pFile = malloc(totalSize);
		if ( pFile == NULL )
		{
			TRACE("Error:  malloc fail \n");
			goto error;
		}

		if(update)
		{
			__mib_content_read_to_raw(HW_SETTING, &pFile[__mib_content_max_size(DEFAULT_SETTING)], header.len + sizeof(PARAM_HEADER_T));
		}

		ds_size = __mib_content_size(DEFAULT_SETTING);
		if(__mib_content_write_to_raw(DEFAULT_SETTING, pFile, ds_size) != 1)
		{
			TRACE("Error:  __mib_content_write_to_raw fail \n");
			goto error;
		}

		if(__mib_file_write(DEFAULT_SETTING, pFile, totalSize) != 1)
		{
			TRACE("Error:  __mib_file_write fail \n");
			goto error;
		}
	} else
#endif
	{
		totalSize = __mib_content_size(data_type);

		pFile = malloc(totalSize);
		if ( pFile == NULL )
		{
			TRACE("Error:  malloc fail \n");
			goto error;
		}

		if(__mib_content_write_to_raw(data_type, pFile, totalSize) != 1)
		{
			TRACE("Error:  __mib_content_write_to_raw fail \n");
			goto error;
		}

		if(__mib_file_write(data_type, pFile, totalSize) != 1)
		{
			TRACE("Error:  __mib_file_write fail \n");
			goto error;
		}
	}

	free(pFile);
	return 1;
		
error:
	if(pFile) free(pFile);
	TRACE("__mib_content_write fail!\n");
	return 0;
}

int __mib_content_read_to_raw(CONFIG_DATA_T data_type, unsigned char* ptr, int len)
{
	if(__mib_file_read(data_type, ptr, len) != 1)
	{
		return 0;
	}

	return 1;
}

int __mib_content_write_from_raw(unsigned char* ptr, int len)
{
	PARAM_HEADER_Tp pHeader = (PARAM_HEADER_Tp) ptr;
	CONFIG_DATA_T data_type;

	if(len < sizeof(PARAM_HEADER_T))
		return 0;

	if ( !memcmp(pHeader->signature, CS_CONF_SETTING_SIGNATURE_TAG, SIGNATURE_LEN) )
	{
		data_type = CURRENT_SETTING;
	}
	else if ( !memcmp(pHeader->signature, DS_CONF_SETTING_SIGNATURE_TAG, SIGNATURE_LEN))
	{
		data_type = DEFAULT_SETTING;
	}
	else if ( !memcmp(pHeader->signature, HS_CONF_SETTING_SIGNATURE_TAG, SIGNATURE_LEN))
	{
		data_type = HW_SETTING;
	}
	else
		return 0;

	if(__mib_header_check(data_type, pHeader) != 1)
	{
		TRACE("Error:  __mib_header_read fail \n");
		return 0;
	}

	if(len != (pHeader->len + sizeof(PARAM_HEADER_T)))
	{
		TRACE("Error:  len incorrect \n");
		return 0;
	}

#ifdef EMBED
	if(data_type == HW_SETTING) {	// ds, hs will share the same flash block
		PARAM_HEADER_T header;
		unsigned int totalSize;
		unsigned char *pFile;
		unsigned char update = 1;
		totalSize = __mib_content_max_size(HW_SETTING) + __mib_content_max_size(DEFAULT_SETTING);

		if(__mib_header_read(DEFAULT_SETTING, &header) != 1)
		{
			TRACE("Error:  __mib_header_read fail \n");
			update = 0;
		}

		if(__mib_header_check(DEFAULT_SETTING, &header) != 1)
		{
			TRACE("Error:  __mib_header_check fail \n");
			update = 0;
		}

		pFile = malloc(totalSize);
		if ( pFile == NULL )
		{
			TRACE("Error:  malloc fail \n");
			return 0;
		}

		if(update)
		{
			__mib_content_read_to_raw(DEFAULT_SETTING, pFile, header.len + sizeof(PARAM_HEADER_T));
		}

		memcpy(&pFile[__mib_content_max_size(DEFAULT_SETTING)], ptr, len);

		if(__mib_file_write(DEFAULT_SETTING, pFile, totalSize) != 1)
		{
			TRACE("Error:  __mib_file_write fail \n");
			free(pFile);
			return 0;
		}

		free(pFile);
	} else if(data_type == DEFAULT_SETTING) {	// ds, hs will share the same flash block
		PARAM_HEADER_T header;
		unsigned int totalSize;
		unsigned char *pFile;
		unsigned char update = 1;
		totalSize = __mib_content_max_size(HW_SETTING) + __mib_content_max_size(DEFAULT_SETTING);

		if(__mib_header_read(HW_SETTING, &header) != 1)
		{
			TRACE("Error:  __mib_header_read fail \n");
			update = 0;
		}

		if(__mib_header_check(HW_SETTING, &header) != 1)
		{
			TRACE("Error:  __mib_header_check fail \n");
			update = 0;
		}

		pFile = malloc(totalSize);
		if ( pFile == NULL )
		{
			TRACE("Error:  malloc fail \n");
			return 0;
		}

		if(update)
		{
			__mib_content_read_to_raw(HW_SETTING, &pFile[__mib_content_max_size(DEFAULT_SETTING)], header.len + sizeof(PARAM_HEADER_T));
		}

		memcpy(pFile, ptr, len);

		if(__mib_file_write(DEFAULT_SETTING, pFile, totalSize) != 1)
		{
			TRACE("Error:  __mib_file_write fail \n");
			free(pFile);
			return 0;
		}

		free(pFile);
	} else 
#endif
	{
		if(__mib_file_write(data_type, ptr, len) != 1)
			return 0;
	}
	
	return 1;
}










/* ------------------------------------------------------------
 * Flash File System API -- Flash utility
 * ------------------------------------------------------------ */
int flash_read(char *buf, int offset, int len) /* raw flash read, without protection */
{
	return __mib_flash_read(buf, offset, len);
}

int flash_write(char *buf, int offset, int len) /* raw flash write, without protection */
{
	return __mib_flash_write(buf, offset, len);
}

int mib_update_from_raw(unsigned char* ptr, int len) /* Write the specified setting to flash, this function will also check the length and checksum */
{
	return __mib_content_write_from_raw(ptr, len);
}

int mib_read_to_raw(CONFIG_DATA_T data_type, unsigned char* ptr, int len) /* Load flash setting to the specified pointer */
{
	return __mib_content_read_to_raw(data_type, ptr, len);
}

int _mib_update(CONFIG_DATA_T data_type) /* Update RAM setting to flash */
{
	return __mib_content_write(data_type);
}

int mib_read_header(CONFIG_DATA_T data_type, PARAM_HEADER_Tp pHeader) /* Load flash header */
{
	return __mib_header_read(data_type, pHeader);	
}

int _mib_load(CONFIG_DATA_T data_type) /* Load flash setting to RAM */
{
	return __mib_content_read(data_type, CONFIG_MIB_ALL);
}

// load mib table (not including mib chain)
int mib_load_table(CONFIG_DATA_T data_type) /* Load flash setting(mib table only) to RAM */
{
	return __mib_content_read(data_type, CONFIG_MIB_TABLE);
}

// Jenny, load mib chain (not including mib table)
int mib_load_chain(CONFIG_DATA_T data_type) /* Load flash setting(mib chain only) to RAM */
{
	return __mib_content_read(data_type, CONFIG_MIB_CHAIN);
}

int mib_reset(CONFIG_DATA_T data_type) /* Reset to default */
{
	PARAM_HEADER_T header;
	PARAM_HEADER_Tp pHeader;
	unsigned char *pFile;
	unsigned int size;	

	if(data_type != CURRENT_SETTING)
		return 0;

	if(__mib_header_read(DEFAULT_SETTING, &header) != 1)
	{
		TRACE("Error:  __mib_header_read fail \n");
		return 0;
	}

	if(__mib_header_check(DEFAULT_SETTING, &header) != 1)
	{
		TRACE("Error:  __mib_header_check fail \n");
		// not support default setting now
		//return 0;
	}

	size = header.len + sizeof(PARAM_HEADER_T);
	pFile = malloc(size);
	if ( pFile == NULL )
	{
		TRACE("Error:  malloc fail \n");
		return 0;
	}

	if(__mib_content_read_to_raw(DEFAULT_SETTING, pFile, size) != 1)
	{
		TRACE("Error:  __mib_content_read_to_raw fail \n");
		free(pFile);
		return 0;
	}

	pHeader = (PARAM_HEADER_Tp)pFile;
	memcpy(pHeader->signature, CS_CONF_SETTING_SIGNATURE_TAG, SIGNATURE_LEN);

	if(__mib_file_write(CURRENT_SETTING, pFile, size) != 1)
	{
		TRACE("Error:  __mib_file_write fail \n");
		free(pFile);
		return 0;
	}

	free(pFile);
	return 1;
}

int mib_update_firmware(unsigned char* ptr, int len) /* Update Firmware */
{
	return __mib_flash_write(ptr, CODE_IMAGE_OFFSET, len);
}










/* ------------------------------------------------------------
 * Flash File System API -- MIB Table Utility
 * ------------------------------------------------------------ */
#ifdef INCLUDE_DEFAULT_VALUE

#if defined(KEEP_CRITICAL_CURRENT_SETTING) || defined(KEEP_CRITICAL_HW_SETTING)
// -----------------
// try to restore the user setting in the flash configuration file
void keep_critical_settings(CONFIG_DATA_T data_type, unsigned char *pContent, int len)
{
	unsigned char *pMibTbl;
	int idx;
	pMibTbl = __mib_get_mib_tbl(data_type);
#ifdef 	KEEP_CRITICAL_HW_SETTING
	if (data_type == HW_SETTING) {
		//i think if HW setting size 
		//73+50+11+...
		if(len < 123 ) //no wlan set...
			len=73; //only keep mac address & superuser info...
		//else wlan setting kept...
		if(len > HW_SETTING_MIN_LEN)
			len=HW_SETTING_MIN_LEN;
	}
#endif
#ifdef KEEP_CRITICAL_CURRENT_SETTING
	if (data_type == CURRENT_SETTING) {
		// check the CS version
		//printf("flash config ver=%d, prog config ver=%d\n", *pContent, *pMibTbl);
		if (*pMibTbl != *pContent)
			return;
		
		printf("Restoring the user config ...\n");
		if(len > CURRENT_SETTING_MIN_LEN)
			len=CURRENT_SETTING_MIN_LEN;
	}
#endif
	memcpy(pMibTbl, pContent, len);
	return;
}
#endif

// action: 0-> reset to default
//         1-> try to keep the original (if valid) and program others to default
//return 0:fail, 1:ok
int mib_init_mib_with_program_default(CONFIG_DATA_T data_type, int action)
{
	unsigned char * pMibTbl;
	PARAM_HEADER_Tp pHeader;

	int idx;

	int i;
	unsigned char ch;
	unsigned short wd;
	unsigned int dwd;
	unsigned char buffer[64];
	struct in_addr addr;
	CONFIG_DATA_T mib_table_data_type;

	TRACE("Load from program default!\n");

	__mib_init_mib_header();

	pMibTbl = __mib_get_mib_tbl(data_type);
	if(pMibTbl == 0)
	{
		TRACE("Error: __mib_get_mib_tbl fail!\n");
		return 0;
	}

	pHeader = __mib_get_mib_header(data_type);
	if(pHeader == 0)
	{
		TRACE("Error: __mib_get_mib_header fail!\n");
		return 0;
	}

	if(data_type == DEFAULT_SETTING)
		mib_table_data_type = CURRENT_SETTING;
	else
		mib_table_data_type = data_type;	

	memset(pMibTbl,0x00,pHeader->len);


	for (idx=0; mib_table[idx].id; idx++) {

		if(mib_table_data_type != mib_table[idx].mib_type)
			continue;

		if(mib_table[idx].defaultValue == 0)
			continue;

//		TRACE("mib_init %s=%s, type=%d\n",mib_table[idx].name, mib_table[idx].defaultValue, data_type);

		switch (mib_table[idx].type) {
		case BYTE_T:
			sscanf(mib_table[idx].defaultValue,"%u",&dwd);
			ch = (unsigned char) dwd;
			memcpy( ((char *)pMibTbl) + mib_table[idx].offset, &ch, 1);
			break;

		case WORD_T:
			sscanf(mib_table[idx].defaultValue,"%u",&dwd);
			wd = (unsigned short) dwd;
			memcpy( ((char *)pMibTbl) + mib_table[idx].offset, &wd, 2);
			break;

		case DWORD_T:
			sscanf(mib_table[idx].defaultValue,"%u",&dwd);
			memcpy( ((char *)pMibTbl) + mib_table[idx].offset, &dwd, 4);
			break;

		case INTEGER_T:
			sscanf(mib_table[idx].defaultValue,"%d",&i);
			memcpy( ((char *)pMibTbl) + mib_table[idx].offset, &i, 4);
			break;

		case STRING_T:
			if ( strlen(mib_table[idx].defaultValue) < mib_table[idx].size )
			{
				strcpy((char *)(((long)pMibTbl) + mib_table[idx].offset), (char *)mib_table[idx].defaultValue);
			}			
			break;

		case BYTE5_T:
			string_to_hex((char *)mib_table[idx].defaultValue, buffer, 10);
			memcpy((unsigned char *)(((long)pMibTbl) + mib_table[idx].offset), (unsigned char *)buffer, 5);
			break;

		case BYTE6_T:
			string_to_hex((char *)mib_table[idx].defaultValue, buffer, 12);
			memcpy((unsigned char *)(((long)pMibTbl) + mib_table[idx].offset), (unsigned char *)buffer, 6);
			break;

		case BYTE13_T:
			string_to_hex((char *)mib_table[idx].defaultValue, buffer, 26);
			memcpy((unsigned char *)(((long)pMibTbl) + mib_table[idx].offset), (unsigned char *)buffer, 13);
			break;

		case BYTE_ARRAY_T:
			for (i=0; i<mib_table[idx].size; i++)
			{
				int val;
				sscanf(&mib_table[idx].defaultValue[i*3],"%d %*d",&val);
				*(((char *)pMibTbl) + mib_table[idx].offset + i) = (char)val;
			}
			break;
			
		case IA_T:
			addr.s_addr = inet_addr(mib_table[idx].defaultValue);
			memcpy((unsigned char *)(((long)pMibTbl) + mib_table[idx].offset), (unsigned char *)(&addr),  4);
			break;

		default:
			break;
		}

	}

	__mib_chain_all_table_clear(mib_table_data_type);
	
	for (idx=0; mib_chain_record_table[idx].id; idx++) {

		TRACE("list %d %d %s %d %x %x\n"
			, mib_chain_record_table[idx].id
			, mib_chain_record_table[idx].mib_type
			, mib_chain_record_table[idx].name
			, mib_chain_record_table[idx].per_record_size
			, mib_chain_record_table[idx].pChainEntry
			, mib_chain_record_table[idx].defaultValue);

		if(mib_chain_record_table[idx].defaultValue != 0) {
			int size = mib_chain_record_table[idx].defaultSize / mib_chain_record_table[idx].per_record_size;
			unsigned char *entry = mib_chain_record_table[idx].defaultValue;
			
			TRACE("mib_init %s %x %d\n"
				, mib_chain_record_table[idx].name
				, mib_chain_record_table[idx].defaultValue
				, mib_chain_record_table[idx].defaultSize);
			
			for(i=0; i< size; i++, entry += mib_chain_record_table[idx].per_record_size)
			{
				_mib_chain_add(mib_chain_record_table[idx].id, entry);						
			}
		}				
	}

	if(data_type == DEFAULT_SETTING)
	{
		unsigned int  size;
		unsigned char *pFile;

		size = __mib_content_size(CURRENT_SETTING);
		pFile = malloc(size);
		if ( pFile == NULL )
		{
			TRACE("Error: malloc fail\n");
			return 0;
		}

		if(__mib_content_write_to_raw(CURRENT_SETTING, pFile, size) != 1)
		{
			TRACE("Error: __mib_content_write_to_raw fail\n");
			return 0;
		}

		// change header signature
		pHeader = (PARAM_HEADER_Tp) pFile;
		memcpy(pHeader->signature, DS_CONF_SETTING_SIGNATURE_TAG, SIGNATURE_LEN);

		if(__mib_content_write_from_raw(pFile, size) != 1)
		{
			TRACE("Error: __mib_file_write fail\n");
			return 0;
		}

		free(pFile);
	}
	else
	{
		unsigned int  size;
		unsigned char *pFile;
#if defined(KEEP_CRITICAL_CURRENT_SETTING) || defined(KEEP_CRITICAL_HW_SETTING)
		if (data_type == CURRENT_SETTING) {
			pFile = CS_CONF_SETTING_SIGNATURE_TAG;
			size = CURRENT_SETTING_MAX_LEN;
		}
		if (data_type == HW_SETTING) {
			pFile = HS_CONF_SETTING_SIGNATURE_TAG;
			size = HW_SETTING_MAX_LEN;
		}
		if(action == FLASH_DEFAULT_TO_AUGMENT && (0 
#ifdef KEEP_CRITICAL_CURRENT_SETTING
		|| (data_type == CURRENT_SETTING)
#endif
#ifdef KEEP_CRITICAL_HW_SETTING
		 || (data_type == HW_SETTING)
#endif
		))
		{
			PARAM_HEADER_T s_header;
			PARAM_HEADER_Tp pHeader=&s_header;
			
			//read (hw or current) setting header from flash...
			__mib_header_read(data_type, &s_header);
			if( ( memcmp(pHeader->signature, pFile, SIGNATURE_LEN) == 0 )
							&& (pHeader->len < size) )
			{
				// it seems the previous hadredware setting is valid... we do more strict check , checksum
				int len;
				unsigned char * user_setting;
				len=pHeader->len+sizeof(PARAM_HEADER_T);
				user_setting=malloc(len);
				
				if(user_setting)
				{
					unsigned char* pContent;
					__mib_file_read(data_type, user_setting, len);
					pContent = &user_setting[sizeof(PARAM_HEADER_T)];	// point to start of MIB data 
					if(__mib_content_decod_check(data_type, &s_header, pContent))
					{
						//checksum is ok, the older setting is integrity...we can restore some critical settings..
						keep_critical_settings(data_type, pContent, s_header.len);
					}
					free(user_setting);					
				}
			}
			
		}
#endif
		size = __mib_content_size(data_type);
		pFile = malloc(size);
		if ( pFile == NULL )
		{
			TRACE("Error: malloc fail\n");
			return 0;
		}

		if(__mib_content_write_to_raw(data_type, pFile, size) != 1)
		{
			TRACE("Error: __mib_content_write_to_raw fail\n");
			return 0;
		}

		if(__mib_content_write_from_raw(pFile, size) != 1)
		{
			TRACE("Error: __mib_file_write fail\n");
			return 0;
		}

		free(pFile);
	}
#ifdef VOIP_SUPPORT
	{
		int totalEntry = 0, intVal;
		voipCfgParam_t *Entry;

		Entry = malloc(sizeof(voipCfgParam_t));
		printf("go to update voip default value\n");
		fprintf(stderr, "The total entry [00]%d\n",mib_chain_total(MIB_VOIP_CFG_TBL));
		if(mib_load(CURRENT_SETTING, CONFIG_MIB_CHAIN)){
			totalEntry = mib_chain_total(MIB_VOIP_CFG_TBL);
			printf("The total entry %d\n",totalEntry);
			if( totalEntry > 0 ){
				if(mib_chain_get(MIB_VOIP_CFG_TBL, 0, (void*)Entry)){
					flash_voip_default(Entry);
					//added by eric
					//printf("The voip version number is %d\n",Entry->version);
					//printf("the signature is %d\n",Entry->signature);
					//printf("the t.38 port is %d",Entry->ports[0].T38_port);
				}
				else
					fprintf(stderr, "get voip config fail.");
			}else{
				intVal = mib_chain_add(MIB_VOIP_CFG_TBL, (unsigned char*)Entry);
				if (intVal == 1)
				{
					//eric added for debugging!
					//fprintf(stderr, "!!! MIB_VOIP_CFG_TBL:%x\r\n", MIB_VOIP_CFG_TBL);
					//fprintf(stderr, "!!! Entry.version:%x\r\n", Entry->version);
					flash_voip_default(Entry);
					//fprintf(stderr,"the t.38 port is %d",Entry->ports[0].T38_port);
				}
				else if (intVal == -1)
					fprintf(stderr, "Error! VoIP Table Full.");
				else
					fprintf(stderr, "add new voip config fail.");
			}
			//added by eric
			//fprintf(stderr, "The total entry [11]%d\n",mib_chain_total(MIB_VOIP_CFG_TBL));
			if(!mib_chain_update(MIB_VOIP_CFG_TBL, (void *)Entry, 0))
				fprintf(stderr, "cannot update voip config!\n");
			fprintf(stderr, "The total entry [33]%d\n",mib_chain_total(MIB_VOIP_CFG_TBL));
			mib_update(CURRENT_SETTING,CONFIG_MIB_CHAIN);
		}else{
			fprintf(stderr, "load mib chain fail!\n");
		}
		totalEntry = mib_chain_total(MIB_VOIP_CFG_TBL);
		fprintf(stderr,"The total entry [22] %d\r\n after update  ",totalEntry);
		free(Entry);
	}
#endif /*VOIP_SUPPORT*/


	return 1;
}


int _mib_getDef(int id, char *buffer) {
	unsigned char * pMibTbl;
	PARAM_HEADER_Tp pHeader;

	int idx;

	int i;
	unsigned char ch;
	unsigned short wd;
	unsigned int dwd;
	struct in_addr addr;

	int ret = 1;
	
	for (idx=0; mib_table[idx].id; idx++) {

		//if(mib_table_data_type != mib_table[idx].mib_type)
		//	continue;

		if (id != mib_table[idx].id)
			continue;

		if(mib_table[idx].defaultValue == 0)
			continue;		

//		TRACE("mib_init %s=%s, type=%d\n",mib_table[idx].name, mib_table[idx].defaultValue, data_type);

		switch (mib_table[idx].type) {
		case BYTE_T:
			sscanf(mib_table[idx].defaultValue,"%u",&dwd);
			ch = (unsigned char) dwd;
			memcpy( buffer, &ch, 1);
			break;

		case WORD_T:
			sscanf(mib_table[idx].defaultValue,"%u",&dwd);
			wd = (unsigned short) dwd;
			memcpy( buffer, &wd, 2);
			break;

		case DWORD_T:
			sscanf(mib_table[idx].defaultValue,"%u",&dwd);
			memcpy( buffer, &dwd, 4);
			break;

		case INTEGER_T:
			sscanf(mib_table[idx].defaultValue,"%d",&i);
			memcpy( buffer, &i, 4);
			break;

		case STRING_T:
			if ( strlen(mib_table[idx].defaultValue) < mib_table[idx].size )
			{
				strcpy(buffer, (char *)mib_table[idx].defaultValue);
			}			
			break;

		case BYTE5_T:
			string_to_hex((char *)mib_table[idx].defaultValue, buffer, 10);
			memcpy(buffer, (unsigned char *)buffer, 5);
			break;

		case BYTE6_T:
			string_to_hex((char *)mib_table[idx].defaultValue, buffer, 12);
			memcpy(buffer, (unsigned char *)buffer, 6);
			break;

		case BYTE13_T:
			string_to_hex((char *)mib_table[idx].defaultValue, buffer, 26);
			memcpy(buffer, (unsigned char *)buffer, 13);
			break;

		case BYTE_ARRAY_T:
			for (i=0; i<mib_table[idx].size; i++)
			{
				int val;
				sscanf(&mib_table[idx].defaultValue[i*3],"%d %*d",&val);
				*(buffer + i) = (char)val;
			}
			break;
			
		case IA_T:
			addr.s_addr = inet_addr(mib_table[idx].defaultValue);
			memcpy(buffer, (unsigned char *)(&addr),  4);
			break;

		default:
			ret = 0;
			break;
		}

	}

	return ret;
}
#endif

int mib_init(void) /* Initialize */
{
	unsigned char vChar;
	
	// Kaohj added, auto-detect the flash
	flashdrv_init();
	__mib_init_mib_header();

	if(_mib_load(HW_SETTING))
		hs_valid = 1;
	// Commented by Mason Yu. for not use default setting	
	//if(_mib_load(DEFAULT_SETTING))
		ds_valid = 1;
	if(_mib_load(CURRENT_SETTING))
		cs_valid = 1;
	
	if (!hs_valid)
		return 0;
	// init cs mib
	if (_mib_get(MIB_BOOT_MODE, (void *)&vChar))
	{
		__boot_mode = (BOOT_TYPE_T)vChar;
		if ((__boot_mode == BOOT_LAST) || (__boot_mode == BOOT_UPGRADE))
		{
			if(!_mib_load(CURRENT_SETTING))
			{
				if(!_mib_load(DEFAULT_SETTING))
				{
					return 0;
				}
				else
				{
					_mib_update(CURRENT_SETTING);
				}
			}
		}
		else
		if (__boot_mode == BOOT_DEFAULT)
		{
			if(!_mib_load(DEFAULT_SETTING))
			{
				return 0;
			}
		}
		else
			return 0;
	}
	else
	{
		__boot_mode = BOOT_LAST;
		if(!_mib_load(CURRENT_SETTING))
		{
			if(!_mib_load(DEFAULT_SETTING))
			{
				return 0;
			}
			else
			{
				_mib_update(CURRENT_SETTING);
			}
		}
	}
	
	if (hs_valid && ds_valid && cs_valid)
		return 1;
	else
		return 0;
	
	return 1;
}

int _mib_get(int id, void *value) /* get mib value */
{
	int i;

	unsigned char * pMibTbl;
	mib_table_entry_T *pTbl = mib_table;

	// search current setting mib table
	for (i=0; mib_table[i].id; i++) {
		if ( mib_table[i].id == id )
		{
//			TRACE("mib_get %s\n",mib_table[i].name);
			break;
		}
	}

	if((mib_table[i].mib_type != CURRENT_SETTING) &&
		(mib_table[i].mib_type != HW_SETTING))
	{
		TRACE("mib_get id=%d unknown\n",id);
		return 0;
	}

	pMibTbl = __mib_get_mib_tbl(mib_table[i].mib_type);

	switch (pTbl[i].type) {
	case BYTE_T:
		memcpy((char *)value, ((char *)pMibTbl) + pTbl[i].offset, 1);
		break;

	case WORD_T:
		memcpy((char *)value, ((char *)pMibTbl) + pTbl[i].offset, 2);
		break;

	case DWORD_T:
		memcpy((char *)value, ((char *)pMibTbl) + pTbl[i].offset, 4);
		break;

	case INTEGER_T:
		memcpy((char *)value, ((char *)pMibTbl) + pTbl[i].offset, 4);
		break;

	case STRING_T:
		strcpy( (char *)value, (const char *)(((long)pMibTbl) + pTbl[i].offset) );
		break;

	case BYTE5_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), 5);
		break;

	case BYTE6_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), 6);
		break;

	case BYTE13_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), 13);
		break;
	
	case IA_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), 4);
		break;

	case BYTE_ARRAY_T:
		memcpy( (unsigned char *)value, (unsigned char *)(((long)pMibTbl) + pTbl[i].offset), pTbl[i].size);
		break;

	default:
		TRACE("mib_get fail!\n");
		return 0;
		
	}
	
	return 1;
}

int _mib_set(int id, void *value) /* set mib value */
{
	int i;
	unsigned char * pMibTbl;
	mib_table_entry_T *pTbl = mib_table;
	
	// search current setting mib table
	for (i=0; mib_table[i].id; i++) {
		if ( mib_table[i].id == id )
		{
//			TRACE("mib_set %s\n",mib_table[i].name);
			break;
		}
	}

	if((mib_table[i].mib_type != CURRENT_SETTING) &&
		(mib_table[i].mib_type != HW_SETTING))
	{
		return 0;
	}

	pMibTbl = __mib_get_mib_tbl(mib_table[i].mib_type);
	// log message
	{
		int k;
		char buf[64]; //sfstudio
		
		sprintf(buf, "0x");
		for (k=0; k<pTbl[i].size; k++)
			snprintf(buf, 64, "%s%.02x ", buf, *(((unsigned char *)pMibTbl)+pTbl[i].offset+k));
			//syslog(LOG_INFO, "mib_set: %s=%s%s", pTbl[i].name, buf, pTbl[i].size>=32?"...":"");
	}

	switch (pTbl[i].type) {
	case BYTE_T:
		memcpy( ((char *)pMibTbl) + pTbl[i].offset, value, 1);
		break;

	case WORD_T:
		memcpy( ((char *)pMibTbl) + pTbl[i].offset, value, 2);
		break;

	case DWORD_T:
		memcpy( ((char *)pMibTbl) + pTbl[i].offset, value, 4);
		break;

	case INTEGER_T:
		memcpy( ((char *)pMibTbl) + pTbl[i].offset, value, 4);
		break;

	case STRING_T:
		if ( strlen(value)+1 > pTbl[i].size )
			return 0;
		strcpy((char *)(((long)pMibTbl) + pTbl[i].offset), (char *)value);
		break;

	case BYTE5_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value, 5);
		break;

	case BYTE6_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value, 6);
		break;

	case BYTE13_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value, 13);
		break;

	case BYTE_ARRAY_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value, pTbl[i].size);
		break;

	case IA_T:
		memcpy((unsigned char *)(((long)pMibTbl) + pTbl[i].offset), (unsigned char *)value,  4);
		break;

	default:
		return 0;
	}

	return 1;
}

 unsigned int _mib_chain_total(int id) /* get chain record size */
 {
 	return __mib_chain_total(id);
 }

void _mib_chain_clear(int id) /* clear chain record */
 {
 	__mib_chain_clear(id);
 }

int _mib_chain_add(int id, unsigned char* ptr) /* add chain record */
 {
 	return __mib_chain_add(id, ptr);
 }

int _mib_chain_delete(int id, unsigned int recordNum) /* delete the specified chain record */
 {
 	return __mib_chain_delete(id, recordNum);
 }

unsigned char* _mib_chain_get(int id, unsigned int recordNum) /* get the specified chain record */
 {
 	return __mib_chain_get(id, recordNum);
 }

int _mib_chain_update(int id, unsigned char* ptr, unsigned int recordNum) /* log updating the specified chain record */
{
	int i = __mib_chain_mib2tbl_id(id);
	
	// log message
	//syslog(LOG_INFO, "mib_chain_update: %s on entry %d", mib_chain_record_table[i].name, recordNum);
	return 1;
}

