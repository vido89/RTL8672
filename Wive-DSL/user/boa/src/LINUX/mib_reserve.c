#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include <arpa/inet.h>
#include <netinet/in.h>

/* for open(), lseek() */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "mib.h"
#include "mibtbl.h"
#include "mib_reserve.h"

#ifdef WEB_DEBUG_MSG
#define	TRACE	printf
#else
#define	TRACE
#endif

//ql add for key parameter reserving.
extern MIB_T table_backup;
extern unsigned char *chain_backup;
extern unsigned int backupChainSize;

int mib_table_record_retrive(int id) /* get mib value from backup mib Info*/
{
	int i;
	char value[256+1];

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

	pMibTbl = (unsigned char *)&table_backup;
	memset(value, 0, 256+1);
	
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
	_mib_set(id, value);
	
	return 1;
}

int mib_chain_record_retrive(int id)
{
	unsigned int idx = 0;
	unsigned char *ptr = chain_backup;
	unsigned int len = backupChainSize;

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

		if ( mib_chain_record_table[i].id != id )
		{
			idx += pCRHeader->len;
			continue;
		}
		
		numOfRecord = pCRHeader->len / mib_chain_record_table[i].per_record_size;

		//remove existing record
		__mib_chain_clear(mib_chain_record_table[i].id);

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
		return 1;
	}	

	return 1;
}

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

int mib__record_clear(CONFIG_MIB_T type)
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
	printf("mib_init_mib_with_program_default in\n");

	__mib_init_mib_header();

	pMibTbl = __mib_get_mib_tbl(CURRENT_SETTING);
	if(pMibTbl == 0)
	{
		TRACE("Error: __mib_get_mib_tbl fail!\n");
		return 0;
	}

	pHeader = __mib_get_mib_header(CURRENT_SETTING);
	if(pHeader == 0)
	{
		TRACE("Error: __mib_get_mib_header fail!\n");
		return 0;
	}

	mib_table_data_type = CURRENT_SETTING;

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
}
//end ql
