/*
 *      AP MIB binary<->text convertion tool
 *
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *
 */

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include "cvcfg.h"
#ifndef WIN32
	#include <unistd.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#else
	#include <Winsock2.h>
	#include <io.h>
#endif

// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT
#include "voip_flash.h"
#include "voip_flash_mib.h"
#endif

#define SPACE			(' ')
#define EOL			('\n')
#define TAB			('\x9')

// MIB value, id mapping table
typedef struct _mib_table_ {
	int id;
	char *name;
} mib_table_T;

#ifdef WIN32
typedef unsigned long in_addr_t;
extern int inet_aton(const char *cp, struct in_addr *addr);
#endif

//////////////////////////////////////////////////////////////////////////////
static int _is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}

#if 0
// Validate digit
static int _isdigit(char c)
{
    return ((c >= '0') && (c <= '9'));
}
#endif

//////////////////////////////////////////////////////////////////////////////
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

//////////////////////////////////////////////////////////////////////////////
static char *getVal(char *value, char **p)
{
	int len=0;

	while (*value == ' ' ) value++;

	*p = value;

	while (*value && *value!=',') {
		value++;
		len++;
	}

	if ( !len ) {
		*p = NULL;
		return NULL;
	}

	if ( *value == 0)
		return NULL;

	*value = 0;
	value++;

	return value;
}

////////////////////////////////////////////////////////////////////////////
#define WRITE_LINE(format, prefix, name, value) { \
	sprintf(line, format, prefix, name, value); \
	fwrite(line, 1, strlen(line), fp); \
}

/////////////////////////////////////////////////////////////////////////////
static int acNum;

#if defined(_MESH_ENABLE_) && defined(_MESH_ACL_ENABLE_)
static int meshAclNum;
#endif

#ifdef HOME_GATEWAY
static int macFilterNum, portFilterNum, ipFilterNum, portFwNum, triggerPortNum;

#if defined(GW_QOS_ENGINE) || defined(QOS_BY_BANDWIDTH)
static int qosRuleNum;
#endif
#endif

#ifdef HOME_GATEWAY
#ifdef VPN_SUPPORT
static int  ipsecTunnelNum;
#endif
#endif
static int wdsNum;
static int disable_hwsetting=0;

/* Local function decalaration */
static void showHelp(void);
static int getToken(char *line, char *value, int *def, int *hwtbl, int *idx);
static int set_mib(struct all_config *pConfig, int id, void *value, int def, int hw, int idx);
static void swap_mib_word_value(APMIB_Tp pMib);
static void getVal2(char *value, char **p1, char **p2);
#ifdef HOME_GATEWAY
static void getVal3(char *value, char **p1, char **p2, char **p3);
static void getVal4(char *value, char **p1, char **p2, char **p3, char **p4);
static void getVal5(char *value, char **p1, char **p2, char **p3, char **p4, char **p5);
static void getVal7(char *value, char **p1, char **p2, char **p3, char **p4, char **p5, char **p6, char **p7);
#endif
#ifdef HOME_GATEWAY
#ifdef VPN_SUPPORT
static void getVal24(char *value, char **p1, char **p2, char **p3, char **p4, char **p5, char **p6, char **p7,\
        char **p8, char **p9, char **p10, char **p11, char **p12, char **p13, char **p14, char **p15, char **p16,\
	char **p17, char **p18, char **p19, char **p20, char **p21, char **p22, char **p23, char **p24);
//#define IPSECTUNNEL_FORMAT ("%d, %d, %s, %d, %s, %d, %d, %s , %d, %s, %d, %d, %d, %d, %d, %d, %s, %d, %d, %d, %lu, %lu, %d, %s, %s, %s")
#define IPSECTUNNEL_FORMAT ("%d, %d, %s, %d, %s, %d, %d, %s , %d, %s, %d, %d,  %d, %d,  %s, %d, %d, %d, %lu, %lu, %d, %s, %s, %s")
#endif
#endif
/////////////////////////////////////////////////////////////////////////////
static char *copyright="Copyright (c) Realtek Semiconductor Corp., 2003-2006. All Rights Reserved.";
static char *version="1.2";

////////////////////////////////////////////////////////////////////////////////
static void showHelp(void)
{
	printf("\nRTL865x config file converting tool [%s].\n", 
#ifdef HOME_GATEWAY
		"gw"
#else
		"ap"
#endif
		);

	printf("%s Ver %s.\n\n", copyright, version);
	printf("Usage: arguments...\n");
	printf("  Where arguments are:\n");
	printf("    -in input_filename, specify the input filename.\n");
	printf("    -ot out_filename, generate a output filename in text mode.\n");
	printf("    -ogu out_filename, generate a output filename for target mode [update].\n");
	printf("    -ogg out_filename, generate a output filename for target mode [upgrade].\n");
	printf("    -ogf out_filename, generate a output filename for target mode [force].\n");
	printf("    -op out_filename, generate a output filename PC mode.\n");
	printf("    -no_hw, do not to generate hw setting.\n");
	printf("    -r generate raw configutation with pad.\n\n");	
}

////////////////////////////////////////////////////////////////////////////////
int checkFileType(char *filename)
{
	int fh=0;
	struct stat status;
	char *buf=NULL, *ptr;
	int flag=0, len;
	PARAM_HEADER_Tp pHeader;

	if ( stat(filename, &status) < 0 )
		goto back;

	buf = malloc(status.st_size);

	if ( buf == NULL)
		goto back;
#ifdef WIN32
	fh = open(filename, O_RDONLY|O_BINARY);
#else
	fh = open(filename, O_RDONLY);
#endif
	if ( fh == -1 )
		goto back;

	lseek(fh, 0L, SEEK_SET);
	if ( read(fh, buf, status.st_size) != status.st_size)
		goto back;

	// Assume PC config file
	flag = PC_MODE;
//	len = HW_SETTING_OFFSET;
	len = 0;
	pHeader = (PARAM_HEADER_Tp)&buf[len];
	if ( (int)(pHeader->len+sizeof(PARAM_HEADER_T)) > status.st_size )
		goto check_target;

	ptr = &buf[len+sizeof(PARAM_HEADER_T)];
	if ( !CHECKSUM_OK(ptr, pHeader->len) )
		goto check_target;
	if (memcmp(pHeader->signature, HW_SETTING_HEADER_TAG, TAG_LEN))
		goto check_target;
	flag |= HS_TYPE;

	len = DEFAULT_SETTING_OFFSET;
	pHeader = (PARAM_HEADER_Tp)&buf[len];
	ptr = &buf[len+sizeof(PARAM_HEADER_T)];
	if ( !CHECKSUM_OK(ptr, pHeader->len) )
		goto check_target;

	if (memcmp(pHeader->signature, DEFAULT_SETTING_HEADER_TAG, TAG_LEN))
		goto check_target;
	flag |= DS_TYPE;

	len = CURRENT_SETTING_OFFSET;
	pHeader = (PARAM_HEADER_Tp)&buf[len];
	if ( !CHECKSUM_OK(ptr, pHeader->len) )
		goto check_target;
	if (memcmp(pHeader->signature, CURRENT_SETTING_HEADER_TAG, TAG_LEN))
		goto check_target;
	flag |= CS_TYPE;
	goto back;

check_target:
	// Assume target mode
	flag = TARGET_MODE;
// rock: fix wild pointer problem
#ifdef VOIP_SUPPORT
#else
	pHeader = (PARAM_HEADER_Tp)buf;
	pHeader->len = WORD_SWAP(pHeader->len);
#endif
	len = 0;
	while (len < status.st_size) {
#ifdef VOIP_SUPPORT
		pHeader = (PARAM_HEADER_Tp)&buf[len];
		pHeader->len = WORD_SWAP(pHeader->len);
#endif
		if ( (int)(pHeader->len+sizeof(PARAM_HEADER_T)) <= status.st_size ) {
			ptr = &buf[len+sizeof(PARAM_HEADER_T)];
			DECODE_DATA(ptr, pHeader->len);
			if ( CHECKSUM_OK(ptr, pHeader->len) ) {
				if (!memcmp(pHeader->signature, HW_SETTING_HEADER_TAG, TAG_LEN) ||
					!memcmp(pHeader->signature, HW_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) ||
					!memcmp(pHeader->signature, HW_SETTING_HEADER_FORCE_TAG, TAG_LEN) )
					flag |= HS_TYPE;
				else if (!memcmp(pHeader->signature, CURRENT_SETTING_HEADER_TAG, TAG_LEN) ||
						!memcmp(pHeader->signature, CURRENT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) ||
						!memcmp(pHeader->signature, CURRENT_SETTING_HEADER_FORCE_TAG, TAG_LEN) )
					flag |= CS_TYPE;
				else if (!memcmp(pHeader->signature, DEFAULT_SETTING_HEADER_TAG, TAG_LEN) ||
						!memcmp(pHeader->signature, DEFAULT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) ||
						!memcmp(pHeader->signature, DEFAULT_SETTING_HEADER_FORCE_TAG, TAG_LEN) )
					flag |= DS_TYPE;
				len = len + sizeof(PARAM_HEADER_T) + pHeader->len;
#ifdef VOIP_SUPPORT
#else
				pHeader = (PARAM_HEADER_Tp)&buf[len];
				pHeader->len = WORD_SWAP(pHeader->len);
#endif
			}
			else
				break;
		}
		else
			break;
	}
	if ( !(flag & TYPE_MASK) )
		flag = TXT_MODE;	// if no config found, set as txt mode
back:
	if (fh >0)
		close(fh);
	if (buf)
		free(buf);

	return (flag);
}

////////////////////////////////////////////////////////////////////////////////
int parseBinConfig(int type, char *filename, struct all_config *pMib)
{
	int fh, ver;
	char *data;
	PARAM_HEADER_T header;
#ifdef WIN32
	fh = open(filename, O_RDONLY|O_BINARY);
#else
	fh = open(filename, O_RDONLY);
#endif
	if ( fh == -1 )
		return -1;

	lseek(fh, 0, SEEK_SET);

	// Read hw setting
	if ( (type & TYPE_MASK) & HS_TYPE ) {
		if ((type & MODE_MASK) == PC_MODE)
			lseek(fh, HW_SETTING_OFFSET, SEEK_SET);

		if ( read(fh, &header, sizeof(PARAM_HEADER_T)) != sizeof(PARAM_HEADER_T)) {
			printf("Read hs header failed!\n");
			close(fh);
			return -1;
		}
		if ((type & MODE_MASK) == TARGET_MODE)
			header.len = WORD_SWAP(header.len);

		if (header.len > sizeof(HW_SETTING_T) + 100) {
			printf("hs length too large!");
			close(fh);
			return -1;
		}
		if ( read(fh, (char *)&pMib->hwmib, header.len) != header.len) {
			printf("Read hw mib failed!\n");
			close(fh);
			return -1;
		}
		data = (char *)&pMib->hwmib;

		if ((type & MODE_MASK) == TARGET_MODE) {
			DECODE_DATA(data, header.len);
			swap_mib_word_value((APMIB_Tp)data);
		}

		if ( CHECKSUM_OK(data, header.len) == 0) {
			printf("hs checksum error!\n");
			close(fh);
			return -1;
		}
		pMib->hwmib_exist = 1;
		sscanf(&header.signature[TAG_LEN], "%02d", &ver);
		pMib->hwmib_ver = ver;
		pMib->hwmib_len = header.len;

	}

	// Read default setting
	if ( (type & TYPE_MASK) & DS_TYPE ) {
		if ((type & MODE_MASK) == PC_MODE)
			lseek(fh, DEFAULT_SETTING_OFFSET, SEEK_SET);

		if ( read(fh, &header, sizeof(PARAM_HEADER_T)) != sizeof(PARAM_HEADER_T)) {
			printf("Read ds header failed!\n");
			close(fh);
			return -1;
		}
		if ((type & MODE_MASK) == TARGET_MODE)
			header.len = WORD_SWAP(header.len);

		if (header.len > sizeof(APMIB_T) + 100) {
			printf("ds length too large!");
			close(fh);
			return -1;
		}
		if ( read(fh, (char *)&pMib->dsmib, header.len) != header.len) {
			printf("Read ds mib failed!\n");
			close(fh);
			return -1;
		}
		data = (char *)&pMib->dsmib;

		if ((type & MODE_MASK) == TARGET_MODE) {
			DECODE_DATA(data, header.len);
			swap_mib_word_value((APMIB_Tp)data);
		}

		if ( CHECKSUM_OK(data, header.len) == 0) {
			printf("ds checksum error!\n");
			close(fh);
			return -1;
		}
		pMib->dsmib_exist = 1;
		sscanf(&header.signature[TAG_LEN], "%02d", &ver);
		pMib->dsmib_ver = ver;
		pMib->dsmib_len = header.len;
	}

	// Read current setting
	if ( (type & TYPE_MASK) & CS_TYPE ) {
		if ((type & MODE_MASK) == PC_MODE)
			lseek(fh, CURRENT_SETTING_OFFSET, SEEK_SET);

		if ( read(fh, &header, sizeof(PARAM_HEADER_T)) != sizeof(PARAM_HEADER_T)) {
			printf("Read cs header failed!\n");
			close(fh);
			return -1;
		}
		if ((type & MODE_MASK) == TARGET_MODE)
			header.len = WORD_SWAP(header.len);

		if (header.len > sizeof(APMIB_T) + 100) {
			printf("cs length too large!");
			close(fh);
			return -1;
		}
		if ( read(fh, (char *)&pMib->csmib, header.len) != header.len) {
			printf("Read cs mib failed!\n");
			close(fh);
			return -1;
		}
		data = (char *)&pMib->csmib;

		if ((type & MODE_MASK) == TARGET_MODE) {
			DECODE_DATA(data, header.len);
			swap_mib_word_value((APMIB_Tp)data);
		}

		if ( CHECKSUM_OK(data, header.len) == 0) {
			printf("cs checksum error!\n");
			close(fh);
			return -1;
		}
		pMib->csmib_exist = 1;
		sscanf(&header.signature[TAG_LEN], "%02d", &ver);
		pMib->csmib_ver = ver;
		pMib->csmib_len = header.len;
	}

	close(fh);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
int parseTxtConfig(char *filename, struct all_config *pMib)
{
	char line[700], value[700];
	FILE *fp;
	int id, def_flag, hw_tbl, wlan_inx, vwlan_inx;

	fp = fopen(filename, "r");
	if ( fp == NULL )
		return -1;

	acNum = 0;
	
#if defined(_MESH_ENABLE_) && defined(_MESH_ACL_ENABLE_)
	meshAclNum = 0;
#endif

#ifdef HOME_GATEWAY
	portFilterNum = ipFilterNum = macFilterNum = portFwNum = triggerPortNum = 0;

#if defined(GW_QOS_ENGINE) || defined(QOS_BY_BANDWIDTH)
	qosRuleNum = 0;
#endif
#endif

#ifdef HOME_GATEWAY
#ifdef VPN_SUPPORT
	ipsecTunnelNum = 0 ;
#endif
#endif
	wdsNum = 0;

	while ( fgets(line, 700, fp) ) {

// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT
{
		int mode;
		char name[200];

		mode = voip_mibline_from(line, name, value);
		if (mode == VOIP_CURRENT_SETTING && name[0])
		{
			voip_mib_read(&pMib->csmib.voipCfgParam, name, value);
			continue;
		}
		else if (mode == VOIP_DEFAULT_SETTING)
		{
			voip_mib_read(&pMib->dsmib.voipCfgParam, name, value);
			continue;
		}
}
#endif

		id = getToken(line, value, &def_flag, &hw_tbl, &wlan_inx, &vwlan_inx);
		if ( id == 0 )
			continue;

		if ( set_mib(pMib, id, value, def_flag, hw_tbl, wlan_inx, vwlan_inx) < 0) {
			printf("Parse MIB [%d] error!\n", id );
			fclose(fp);
			return -1;
		}
	}

	fclose(fp);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
static int getToken(char *line, char *value, int *def_flag, int *hw_tbl, int *wlan_inx, int *vwlan_inx)
{
	char *ptr=line, *p1;
	char token[300]={0};
	int len=0, idx;
	mib_table_entry_T *pTbl=mib_table;

	*def_flag = 0;
	*hw_tbl = 0;
	*wlan_inx = -1;
	*vwlan_inx = 0;

	if ( *ptr == ';' )	// comments
		return 0;

	// get token
	while (*ptr && *ptr!=EOL && *ptr!=';') {
		if ( *ptr == '=' ) {
			memcpy(token, line, len);

			// delete ending space
			for (idx=len-1; idx>=0; idx--) {
				if (token[idx]!= SPACE && token[idx]!= TAB)
					break;
			}
			token[idx+1] = '\0';
			ptr++;
			break;
		}
		ptr++;
		len++;
	}
	if ( !token[0] )
		return 0;

	// get value
	len=0;
	while (*ptr == SPACE ) ptr++; // delete space

	p1 = ptr;
	while ( *ptr && *ptr!=EOL && *ptr!=';' && *ptr!='\r') {
		ptr++;
		len++;
	}
	memcpy(value, p1, len );
	value[len] = '\0';

	// delete ending space
	for (idx=len-1; idx>=0; idx--) {
		if (value[idx]!= SPACE && value[idx]!= TAB)
			break;
	}
	value[idx+1] = '\0';

	// delete brace " "
	if (strlen(value)>0 && value[0]=='"' && value[strlen(value)-1]=='"') {
		char tmp1[100];		
		memcpy(tmp1, &value[1], strlen(value)-2);		
		tmp1[strlen(value)-2] = '\0';
		strcpy(value, tmp1);	
	}
	
	p1 = token;
	if ( !memcmp(token, "DEF_", 4)) {
		p1 = &token[4];
		if (!memcmp(p1, "WLAN", 4) && p1[5]=='_') {
			*wlan_inx = p1[4] - '0';
			if (*wlan_inx >=  NUM_WLAN_INTERFACE) {
				printf("Invalid wlan interface number!\n");
				return 0;
			}
#ifdef MBSSID
			if (!memcmp(&p1[6], "VAP", 3) && p1[10]=='_') {
				*vwlan_inx = p1[9] - '0';
				if (*vwlan_inx >=  NUM_VWLAN_INTERFACE) {
					printf("Invalid virtual wlan interface number!\n");
					return 0;
				}
				p1 += 11;
				*vwlan_inx += 1;
			}
#ifdef UNIVERSAL_REPEATER
			else if (!memcmp(&p1[6], "VXD", 3) && p1[9]=='_') {
				*vwlan_inx = NUM_VWLAN_INTERFACE;				
				p1 += 10;
			}
#endif		
			else
#endif
			p1 += 6;
			pTbl = mib_wlan_table;
		}
		else
			pTbl = mib_table;
		*def_flag = 1;
	}
	else if ( !memcmp(token, "HW_", 3)) {
		p1 = &token[3];
		if (!memcmp(p1, "WLAN", 4) && p1[5]=='_') {
			*wlan_inx = p1[4] - '0';
			if (*wlan_inx >=  NUM_WLAN_INTERFACE) {
				printf("Invalid wlan interface number!\n");
				return 0;
			}
			p1 += 6;
			pTbl = hwmib_wlan_table;
		}
		else
			pTbl = hwmib_table;
		*hw_tbl = 1;
	}
	else { // current setting
		if (!memcmp(p1, "WLAN", 4) && p1[5]=='_') {
			*wlan_inx = p1[4] - '0';
			if (*wlan_inx >=  NUM_WLAN_INTERFACE) {
				printf("Invalid wlan interface number!\n");
				return 0;
			}
#ifdef MBSSID
			if (!memcmp(&p1[6], "VAP", 3) && p1[10]=='_') {
				*vwlan_inx = p1[9] - '0';
				if (*vwlan_inx >=  NUM_VWLAN_INTERFACE) {
					printf("Invalid virtual wlan interface number!\n");
					return 0;
				}
				p1 += 11;
				*vwlan_inx += 1;
			}

#ifdef UNIVERSAL_REPEATER
			else if (!memcmp(&p1[6], "VXD", 3) && p1[9] == '_') {
				*vwlan_inx = NUM_VWLAN_INTERFACE;
				p1 += 10;
			}
#endif				
			else
#endif
			p1 += 6;
			pTbl = mib_wlan_table;
		}
	}

	idx = 0;
	while (pTbl[idx].id) {
		if (!strcmp(pTbl[idx].name, p1))
			return pTbl[idx].id;
		idx++;
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
static int set_mib(struct all_config *pConfig, int id, void *value, int def_mib, int hw_tbl, int idx, int v_idx)
{
	unsigned char key[180];
	char *p1, *p2;
#ifdef HOME_GATEWAY
	char *p3, *p4, *p5, *p6, *p7;
#if defined(GW_QOS_ENGINE) || defined(VPN_SUPPORT)
	char *p8, *p9, *p10, *p11, *p12;
#endif

#ifdef VPN_SUPPORT
	char *p13, *p14, *p15, *p16, *p17, *p18, *p19, *p20, *p21, *p22, *p23, *p24;
#endif
#endif
	struct in_addr inAddr;
	int i, j, k, int_val, max_chan;
	MACFILTER_Tp pWlAc;
	WDS_Tp pWds;

#ifdef HOME_GATEWAY
	PORTFW_Tp pPortFw;
	PORTFILTER_Tp pPortFilter;
	IPFILTER_Tp pIpFilter;
	MACFILTER_Tp pMacFilter;
	TRIGGERPORT_Tp pTriggerPort;

#ifdef GW_QOS_ENGINE
	QOS_Tp pQos;
#endif

#ifdef QOS_BY_BANDWIDTH
	IPQOS_Tp pQos;
#endif

#ifdef VPN_SUPPORT
	IPSECTUNNEL_Tp pIpsecTunnel;
#endif
#endif
	mib_table_entry_T *pTbl;
	char *pMib;

	if (def_mib) {
		if (idx >= 0) {
			pMib = (char *)&pConfig->dsmib.wlan[idx][v_idx]; //for debug sean
			pTbl = mib_wlan_table;
		}
		else {
			pMib = (char *)&pConfig->dsmib;
			pTbl = mib_table;
		}
		pConfig->dsmib_exist = 1;
	}
	else if (hw_tbl) {
		if (idx >= 0) {
			pMib = (char *)&pConfig->hwmib.wlan[idx];
			pTbl = hwmib_wlan_table;
		}
		else {
			pMib = (char *)&pConfig->hwmib;
			pTbl = hwmib_table;
		}
		pConfig->hwmib_exist = 1;
	}
	else {
		if (idx >= 0) {
			pMib = (char *)&pConfig->csmib.wlan[idx][v_idx];
			pTbl = mib_wlan_table;
		}
		else {
			pMib = (char *)&pConfig->csmib;
			pTbl = mib_table;
		}
		pConfig->csmib_exist = 1;
	}


	for (i=0; pTbl[i].id; i++) {
		if ( pTbl[i].id == id )
			break;
	}
	if ( pTbl[i].id == 0 )
		return -1;

	switch (pTbl[i].type) {
	case BYTE_T:
		*((unsigned char *)(((long)pMib) + pTbl[i].offset)) = (unsigned char)atoi(value);
		break;

	case WORD_T:
		*((unsigned short *)(((long)pMib) + pTbl[i].offset)) = (unsigned short)atoi(value);
		break;

	case STRING_T:
		if ( (int)(strlen(value)+1) > pTbl[i].size )
			return 0;
		strcpy((char *)(((long)pMib) + pTbl[i].offset), (char *)value);
		break;

	case BYTE5_T:
		if (strlen(value)!=10 || !string_to_hex(value, key, 10))
			return -1;
		memcpy((unsigned char *)(((long)pMib) + pTbl[i].offset), key, 5);
		break;

	case BYTE6_T:
		if (strlen(value)!=12 || !string_to_hex(value, key, 12))
			return -1;
		memcpy((unsigned char *)(((long)pMib) + pTbl[i].offset), key, 6);
		break;

	case BYTE13_T:
		if (strlen(value)!=26 || !string_to_hex(value, key, 26))
			return -1;
		memcpy((unsigned char *)(((long)pMib) + pTbl[i].offset), key, 13);
		break;

	case DWORD_T:
		*((unsigned long *)(((long)pMib) + pTbl[i].offset)) = (unsigned long)atoi(value);
		break;

	case IA_T:
		if ( !inet_aton(value, &inAddr) )
			return -1;
		memcpy((unsigned char *)(((long)pMib) + pTbl[i].offset), (unsigned char *)&inAddr,  4);
		break;

	case BYTE_ARRAY_T:
		max_chan = (pTbl[i].id == MIB_HW_TX_POWER_CCK)? MAX_CCK_CHAN_NUM:MAX_OFDM_CHAN_NUM;
		if ( pTbl[i].id == MIB_HW_TX_POWER_CCK || pTbl[i].id == MIB_HW_TX_POWER_OFDM ) {
			p1 = (char *)value;
			for (k=0, j=0; k<max_chan ; k++) {
				if ( !sscanf(&p1[j], "%d", &int_val) ) {
					printf("invalid value!\n");
					return -1;
				}
				key[k] = (unsigned char)int_val;
				while ( p1[++j] ) {
					if ( p1[j]==' ') {
						j++;
						break;
					}
				}
			}
			memcpy((unsigned char *)(((long)pMib) + pTbl[i].offset), (unsigned char *)key, pTbl[i].size );
		}
		break;

	case WLAC_ARRAY_T:
		getVal2((char *)value, &p1, &p2);
		if (p1 == NULL) {
			printf("Invalid WLAC in argument!\n");
			break;
		}
		if (strlen(p1)!=12 || !string_to_hex(p1, key, 12))
			return -1;

		pWlAc = (MACFILTER_Tp)(((long)pMib)+pTbl[i].offset+acNum*sizeof(MACFILTER_T));
		memcpy(pWlAc->macAddr, key, 6);
		if (p2 != NULL )
			strcpy(pWlAc->comment, p2);
		acNum++;
		break;

#if defined(_MESH_ENABLE_) && defined(_MESH_ACL_ENABLE_) // below code copy above ACL code
	case MESH_ACL_ARRAY_T:
		getVal2((char *)value, &p1, &p2);
		if (p1 == NULL) {
			printf("Invalid Mesh ACL in argument!\n");
			break;
		}
		if (strlen(p1)!=12 || !string_to_hex(p1, key, 12))
			return -1;

		pWlAc = (MACFILTER_Tp)(((long)pMib)+pTbl[i].offset+meshAclNum*sizeof(MACFILTER_T));
		memcpy(pWlAc->macAddr, key, 6);
		if (p2 != NULL )
			strcpy(pWlAc->comment, p2);
		meshAclNum++;
		break;
#endif

	case WDS_ARRAY_T:
		getVal2((char *)value, &p1, &p2);
		if (p1 == NULL) {
			printf("Invalid WDS in argument!\n");
			break;
		}
		if (strlen(p1)!=12 || !string_to_hex(p1, key, 12))
			return -1;

		pWds = (WDS_Tp)(((long)pMib)+pTbl[i].offset+wdsNum*sizeof(WDS_T));
		memcpy(pWds->macAddr, key, 6);
		if (p2 != NULL )
			strcpy(pWds->comment, p2);
		wdsNum++;
		break;


#ifdef HOME_GATEWAY
	case MACFILTER_ARRAY_T:
		getVal2((char *)value, &p1, &p2);
		if (p1 == NULL) {
			printf("Invalid MACFILTER in argument!\n");
			break;
		}
		if (strlen(p1)!=12 || !string_to_hex(p1, key, 12))
			return -1;

		pMacFilter = (MACFILTER_Tp)(((long)pMib)+pTbl[i].offset+macFilterNum*sizeof(MACFILTER_T));
		memcpy(pMacFilter->macAddr, key, 6);
		if (p2 != NULL )
			strcpy(pMacFilter->comment, p2);
		macFilterNum++;
		break;

	case PORTFW_ARRAY_T:
		getVal5((char *)value, &p1, &p2, &p3, &p4, &p5);
		if (p1 == NULL || p2 == NULL || p3 == NULL || p4 == NULL ) {
			printf("Invalid PORTFW arguments!\n");
			break;
		}
		if ( !inet_aton(p1, &inAddr) )
			return -1;

		pPortFw = (PORTFW_Tp)(((long)pMib)+pTbl[i].offset+portFwNum*sizeof(PORTFW_T));
		memcpy(pPortFw->ipAddr, (unsigned char *)&inAddr, 4);
		pPortFw->fromPort = (unsigned short)atoi(p2);
		pPortFw->toPort = (unsigned short)atoi(p3);
		pPortFw->protoType = (unsigned char)atoi(p4);
		if ( p5 )
			strcpy( pPortFw->comment, p5 );
		portFwNum++;
		break;

	case IPFILTER_ARRAY_T:
		getVal3((char *)value, &p1, &p2, &p3);
		if (p1 == NULL || p2 == NULL) {
			printf("Invalid IPFILTER arguments!\n");
			break;
		}
		if ( !inet_aton(p1, &inAddr) )
			return -1;
		pIpFilter = (IPFILTER_Tp)(((long)pMib)+pTbl[i].offset+ipFilterNum*sizeof(IPFILTER_T));
		memcpy(pIpFilter->ipAddr, (unsigned char *)&inAddr, 4);
		pIpFilter->protoType = (unsigned char)atoi(p2);
		if ( p3 )
			strcpy( pIpFilter->comment, p3 );
		ipFilterNum++;
		break;

	case PORTFILTER_ARRAY_T:
		getVal4((char *)value, &p1, &p2, &p3, &p4);
		if (p1 == NULL || p2 == NULL || p3 == NULL) {
			printf("Invalid PORTFILTER arguments!\n");
			break;
		}
		pPortFilter = (PORTFILTER_Tp)(((long)pMib)+pTbl[i].offset+portFilterNum*sizeof(PORTFILTER_T));
		pPortFilter->fromPort = (unsigned short)atoi(p1);
		pPortFilter->toPort = (unsigned short)atoi(p2);
		pPortFilter->protoType = (unsigned char)atoi(p3);
		if ( p4 )
			strcpy( pPortFilter->comment, p4 );
		portFilterNum++;
		break;

	case TRIGGERPORT_ARRAY_T:
		getVal7((char *)value, &p1, &p2, &p3, &p4, &p5, &p6, &p7);
		if (p1 == NULL || p2 == NULL || p3 == NULL || p4 == NULL || p5 == NULL || p6 == NULL) {
			printf("Invalid TRIGGERPORT arguments!\n");
			break;
		}
		pTriggerPort = (TRIGGERPORT_Tp)(((long)pMib)+pTbl[i].offset+triggerPortNum*sizeof(TRIGGERPORT_T));
		pTriggerPort->tri_fromPort = (unsigned short)atoi(p1);
		pTriggerPort->tri_toPort = (unsigned short)atoi(p2);
		pTriggerPort->tri_protoType = (unsigned char)atoi(p3);
		pTriggerPort->inc_fromPort = (unsigned short)atoi(p4);
		pTriggerPort->inc_toPort = (unsigned short)atoi(p5);
		pTriggerPort->inc_protoType = (unsigned char)atoi(p6);
		if ( p7 )
			strcpy( pTriggerPort->comment, p7);
		triggerPortNum++;
		break;

#ifdef GW_QOS_ENGINE
	case QOS_ARRAY_T:
		getVal12((char *)value, &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9, &p10, &p11, &p12);
		if (p1 == NULL || p2 == NULL || p3 == NULL || p4 == NULL || p5 == NULL || p6 == NULL || p7 == NULL ||
		    p8 == NULL || p9 == NULL || p10 == NULL || p11 == NULL || p12 == NULL ) {
			printf("Invalid QoS arguments!\n");
			break;
		}
		pQos = (QOS_Tp)(((long)pMib)+pTbl[i].offset+qosRuleNum*sizeof(QOS_T));
		pQos->enabled = (unsigned char)atoi(p1);
		pQos->priority = (unsigned char)atoi(p2);
		pQos->protocol = (unsigned short)atoi(p3);
		if ( !inet_aton(p4, &inAddr) )
			return -1;
		memcpy(pQos->local_ip_start, (unsigned char *)&inAddr, 4);
		if ( !inet_aton(p5, &inAddr) )
			return -1;
		memcpy(pQos->local_ip_end, (unsigned char *)&inAddr, 4);
        
		pQos->local_port_start = (unsigned short)atoi(p6);
		pQos->local_port_end = (unsigned short)atoi(p7);
		if ( !inet_aton(p8, &inAddr) )
			return -1;
		memcpy(pQos->remote_ip_start, (unsigned char *)&inAddr, 4);
		if ( !inet_aton(p9, &inAddr) )
			return -1;
		memcpy(pQos->remote_ip_end, (unsigned char *)&inAddr, 4);

		pQos->remote_port_start = (unsigned short)atoi(p10);
		pQos->remote_port_end = (unsigned short)atoi(p11);
        	strcpy( pQos->entry_name, p12 );
		qosRuleNum++;
		break;
#endif
#ifdef VPN_SUPPORT
	case IPSECTUNNEL_ARRAY_T:
		getVal24((char *)value, &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9, &p10, &p11, &p12, &p13
		, &p14, &p15, &p16, &p17, &p18, &p19, &p20, &p21 , &p22, &p23, &p24);
		if (p1 == NULL || p2 == NULL || p3 == NULL || p4 == NULL || p5 == NULL || p6 == NULL || p7 == NULL ||\
		p9 == NULL || p10 == NULL || p11 == NULL || p12 == NULL || p13 == NULL || p14 == NULL || p15 == NULL\
		|| p16 == NULL || p17 == NULL || p18 == NULL || p19 == NULL || p20 == NULL || p21 == NULL|| p22 == NULL
		|| p23 == NULL|| p24 == NULL)
		{
			printf("Invalid Ipsec Tunnel arguments!\n");
			break;
		}
		pIpsecTunnel= (IPSECTUNNEL_Tp)(((long)pMib)+pTbl[i].offset+ipsecTunnelNum*sizeof(IPSECTUNNEL_T));
		pIpsecTunnel->tunnelId= (unsigned char)atoi(p1);
		pIpsecTunnel->enable= (unsigned char)atoi(p2);
		strcpy(pIpsecTunnel->connName,p3);
		pIpsecTunnel->lcType= (unsigned char)atoi(p4);

                if ( !inet_aton(p5, &inAddr) )
		                        return -1;
		memcpy(pIpsecTunnel->lc_ipAddr , (unsigned char *)&inAddr, 4);

		pIpsecTunnel->lc_maskLen= atoi(p6);

		pIpsecTunnel->rtType= (unsigned char)atoi(p7);
		if ( !inet_aton(p8, &inAddr) )
		                        return -1;
		memcpy(pIpsecTunnel->rt_ipAddr , (unsigned char *)&inAddr, 4);

		pIpsecTunnel->rt_maskLen= atoi(p9);

		if ( !inet_aton(p10, &inAddr) )
		                        return -1;
		memcpy(pIpsecTunnel->rt_gwAddr , (unsigned char *)&inAddr, 4);
		pIpsecTunnel->keyMode= (unsigned char)atoi(p11);
		pIpsecTunnel->conType= (unsigned char)atoi(p12);
		pIpsecTunnel->espEncr= (unsigned char)atoi(p14);
		pIpsecTunnel->espAuth= (unsigned char)atoi(p15);

		if(p15)
			strcpy(pIpsecTunnel->psKey, p15);

		pIpsecTunnel->ikeEncr= (unsigned char)atoi(p16);
		pIpsecTunnel->ikeAuth= (unsigned char)atoi(p17);
		pIpsecTunnel->ikeKeyGroup= (unsigned char)atoi(p18);
		pIpsecTunnel->ikeLifeTime = strtol(p19,(char **)NULL, 10);

		pIpsecTunnel->ipsecLifeTime = strtol(p20,(char **)NULL, 10);
		pIpsecTunnel->ipsecPfs= (unsigned char)atoi(p21);

		if(p22)
			strcpy(pIpsecTunnel->spi,p22);
		if(p23)
			strcpy(pIpsecTunnel->encrKey, p23);
		if(p24)
			strcpy(pIpsecTunnel->authKey, p24);
		ipsecTunnelNum++;
		break;

#endif
#endif

	default:
		return -1;
	}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////
int generateBinFile(int type, char *filename, int flag, struct all_config *allConfig)
{	
	int fh;
	char *ptr, *pHsTag, *pDsTag, *pCsTag;
	PARAM_HEADER_T header;
	unsigned char checksum;
	int mib_len, section_len;

	if (type == PC_MODE) {
		char *buf;
		buf=calloc(1, 0x6000);
		if ( buf == NULL ) {
			printf("Allocate buffer failed!\n");
			return -1;
		}
		fh = open(filename, O_RDWR|O_CREAT|O_TRUNC);
		write(fh, buf, 0x6000);
		free(buf);

		pHsTag = HW_SETTING_HEADER_TAG;

		pDsTag = DEFAULT_SETTING_HEADER_TAG;
		pCsTag = CURRENT_SETTING_HEADER_TAG;
	}
	else {
#ifdef WIN32
		_chmod(filename, S_IREAD|S_IWRITE);
		fh = open(filename, O_RDWR|O_CREAT|O_TRUNC|O_BINARY);
#else
		chmod(filename, S_IREAD|S_IWRITE);
		fh = open(filename, O_RDWR|O_CREAT|O_TRUNC);
#endif
		if ((flag & 0x7f) == 0) {  // upgrade
			pHsTag = HW_SETTING_HEADER_UPGRADE_TAG;
			pDsTag = DEFAULT_SETTING_HEADER_UPGRADE_TAG;
			pCsTag = CURRENT_SETTING_HEADER_UPGRADE_TAG;
		}
		else if ((flag & 0x7f) == 1) { // update
			pHsTag = HW_SETTING_HEADER_TAG;
			pDsTag = DEFAULT_SETTING_HEADER_TAG;
			pCsTag = CURRENT_SETTING_HEADER_TAG;
		}
		else {
			pHsTag = HW_SETTING_HEADER_FORCE_TAG;
			pDsTag = DEFAULT_SETTING_HEADER_FORCE_TAG;
			pCsTag = CURRENT_SETTING_HEADER_FORCE_TAG;
		}
	}

	if ( fh == -1 )
		return -1;

	lseek(fh, 0, SEEK_SET);

	if (allConfig->hwmib_exist && !disable_hwsetting) {
		if (allConfig->hwmib_ver == 0) {
			sprintf(header.signature, "%s%02d", pHsTag, HW_SETTING_VER);
			header.len = sizeof(HW_SETTING_T) + sizeof(checksum);
		}
		else {
			sprintf(header.signature, "%s%02d", pHsTag, allConfig->hwmib_ver);
			header.len = allConfig->hwmib_len;
		}
		mib_len = header.len;
		ptr = (char *)&allConfig->hwmib;
		checksum = CHECKSUM(ptr, header.len-1);
		ptr[header.len-1] = checksum;
		if ( type == PC_MODE )
			lseek(fh, HW_SETTING_OFFSET, SEEK_SET);
		else {
			if ( !(flag & RAW_TYPE) ) {
   				ENCODE_DATA(ptr, header.len);
			}
			header.len = WORD_SWAP(header.len);
		}
		write(fh, &header, sizeof(header));
		write(fh, ptr, mib_len);
		if ( flag & RAW_TYPE ) { // raw type, pad 0
			section_len = HW_SETTING_SECTOR_LEN - sizeof(header) - mib_len;
			while (section_len-- >0) {
				write(fh, "\x0", 1);
			}
		}
	}

	if (allConfig->dsmib_exist) {
		if (allConfig->dsmib_ver == 0) {
			sprintf(header.signature, "%s%02d", pDsTag, DEFAULT_SETTING_VER);
			header.len = sizeof(APMIB_T) + sizeof(checksum);
		}
		else {
			sprintf(header.signature, "%s%02d", pDsTag, allConfig->dsmib_ver);
			header.len = allConfig->dsmib_len;
		}
		mib_len = header.len;
		ptr = (char *)&allConfig->dsmib;
		checksum = CHECKSUM(ptr, header.len-1);
		ptr[header.len-1] = checksum;
		if ( type == PC_MODE )
			lseek(fh, DEFAULT_SETTING_OFFSET, SEEK_SET);
		else {
			swap_mib_word_value(&allConfig->dsmib);
			if ( !(flag&RAW_TYPE) ) {
				ENCODE_DATA(ptr, header.len);
			}
			header.len = WORD_SWAP(header.len);
		}
		write(fh, &header, sizeof(header));
		write(fh, ptr, mib_len);

		if ( flag & RAW_TYPE ) { // raw type, pad 0
			section_len = DEFAULT_SETTING_SECTOR_LEN - sizeof(header) - mib_len;
			while (section_len-- >0) {
				write(fh, "\x0", 1);
			}
		}
	}
	if (allConfig->csmib_exist) {
		if (allConfig->csmib_ver == 0) {
			sprintf(header.signature, "%s%02d", pCsTag, CURRENT_SETTING_VER);
			header.len = sizeof(APMIB_T) + sizeof(checksum);
		}
		else {
			sprintf(header.signature, "%s%02d", pCsTag, allConfig->csmib_ver);
			header.len = allConfig->csmib_len;
		}
		mib_len = header.len;
		ptr = (char *)&allConfig->csmib;
		checksum = CHECKSUM(ptr, header.len-1);
		ptr[header.len-1] = checksum;
		if ( type == PC_MODE )
			lseek(fh, CURRENT_SETTING_OFFSET, SEEK_SET);
		else {
			swap_mib_word_value(&allConfig->csmib);
			if (!(flag&RAW_TYPE)) {
				ENCODE_DATA(ptr, header.len);
			}
			header.len = WORD_SWAP(header.len);
		}
		write(fh, &header, sizeof(header));
		write(fh, ptr, mib_len);

		if ( flag & RAW_TYPE ) { // raw type, pad 0
			section_len = CURRENT_SETTING_SECTOR_LEN - sizeof(header) - mib_len;
			while (section_len-- >0) {
				write(fh, "\x0", 1);
			}
		}
	}

	close(fh);

#ifdef WIN32
	_chmod(filename, S_IREAD);
#else
	chmod(filename,  DEFFILEMODE);
#endif

	return 0;
}

int generateTxtFile(char *filename, struct all_config mib_config)
{
	unsigned char *p;
	FILE *fp;
	char buf[500], line[300], prefix[20];
	int i, j;
	MACFILTER_Tp pWlAc;
	WDS_Tp pWds;
	int max_chan_num;

#ifdef HOME_GATEWAY
	PORTFW_Tp pPortFw;
	PORTFILTER_Tp pPortFilter;
	IPFILTER_Tp pIpFilter;
	MACFILTER_Tp pMacFilter;
	TRIGGERPORT_Tp pTriggerPort;

#ifdef GW_QOS_ENGINE
	QOS_Tp pQos;
	char LipS[20], LipE[20], RipS[20], RipE[20];
#endif

#ifdef QOS_BY_BANDWIDTH
	IPQOS_Tp pQos;
	char LipS[20], LipE[20];
#endif

#ifdef VPN_SUPPORT
	IPSECTUNNEL_Tp pIpsecTunnel;
	char strLcIp[20], strRtIp[20], strRtGw[20];
#endif
#endif
	int idx=0, wlan_inx, vwlan_inx;
	mib_table_entry_T *pTbl;
	char *pMib;
	APMIB_Tp pApMib;

	fp = fopen(filename, "w");
	if ( fp == NULL)
		return -1;

next_config:
	wlan_inx = 0;
	vwlan_inx = 0;
	if ( ++idx>6) {

// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT
		fflush(fp);
		if (mib_config.csmib_exist)
			voip_mibtbl_write(&mib_config.csmib.voipCfgParam, fileno(fp), VOIP_CURRENT_SETTING);
		if (mib_config.dsmib_exist)
			voip_mibtbl_write(&mib_config.dsmib.voipCfgParam, fileno(fp), VOIP_DEFAULT_SETTING);
#endif

		fclose(fp);
		return 0;
	}
next_wlan:
	if (idx == 1) {
		if ( !mib_config.hwmib_exist )
			goto next_config;
		pTbl = hwmib_table;
		pMib = (char *)&mib_config.hwmib;
		strcpy(prefix, "HW_");
	}
	else if (idx == 2) {
		if ( !mib_config.hwmib_exist )
			goto next_config;
		pTbl = hwmib_wlan_table;
		pMib = (char *)&mib_config.hwmib.wlan[wlan_inx];
		sprintf(prefix, "HW_WLAN%d_", wlan_inx);
	}
	else if (idx == 3) {
		if ( !mib_config.dsmib_exist )
			goto next_config;
		pTbl = mib_table;
		pMib = (char *)&mib_config.dsmib;
		pApMib = &mib_config.dsmib;
		strcpy(prefix, "DEF_");
	}
	else if (idx == 4) {
		if ( !mib_config.dsmib_exist )
			goto next_config;
		pTbl = mib_wlan_table;
		pMib = (char *)&mib_config.dsmib.wlan[wlan_inx][vwlan_inx];
		pApMib = &mib_config.dsmib;
#ifdef MBSSID
		if (vwlan_inx > 0)
			sprintf(prefix, "DEF_WLAN%d_VAP%d_", wlan_inx, vwlan_inx-1);
		else
#endif
		sprintf(prefix, "DEF_WLAN%d_", wlan_inx);
	}
	else if (idx == 5) {
		if ( !mib_config.csmib_exist )
			goto next_config;
		pTbl = mib_table;
		pMib = (char *)&mib_config.csmib;
		pApMib = &mib_config.csmib;
		prefix[0] = '\0';
	}
	else if (idx == 6) {
		if ( !mib_config.csmib_exist )
			goto next_config;
		pTbl = mib_wlan_table;
		pMib = (char *)&mib_config.csmib.wlan[wlan_inx][vwlan_inx];
		pApMib = &mib_config.csmib;
#ifdef MBSSID
		if (vwlan_inx > 0)
			sprintf(prefix, "WLAN%d_VAP%d_", wlan_inx, vwlan_inx-1);
		else
#endif
		sprintf(prefix, "WLAN%d_", wlan_inx);
	}

	for (i=0; pTbl[i].id; i++) {
		switch (pTbl[i].type) {
		case BYTE_T:
#ifdef MBSSID
			if (pTbl[i].id == MIB_WSC_DISABLE && vwlan_inx > 0) {
				WRITE_LINE("%s%s=%d\n", prefix, pTbl[i].name, 1);
			}
			else
#endif
			WRITE_LINE("%s%s=%d\n", prefix, pTbl[i].name,
				*((unsigned char *)(((long)pMib) + pTbl[i].offset)));
			break;

		case WORD_T:
			WRITE_LINE("%s%s=%d\n", prefix, pTbl[i].name,
				*((unsigned short *)(((long)pMib) + pTbl[i].offset)));
			break;

		case STRING_T:
			WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name,
				(char *)(((long)pMib) + pTbl[i].offset));
			break;

		case BYTE5_T:
			p = (unsigned char *)(((long)pMib) + pTbl[i].offset);
			sprintf(buf, "%02x%02x%02x%02x%02x", p[0], p[1], p[2], p[3], p[4]);
			WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			break;

		case BYTE6_T:
			p = (unsigned char *)(((long)pMib) + pTbl[i].offset);
			sprintf(buf, "%02x%02x%02x%02x%02x%02x", p[0], p[1], p[2], p[3], p[4], p[5]);
			WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			break;

		case BYTE13_T:
			p = (unsigned char *)(((long)pMib) + pTbl[i].offset);
			sprintf(buf, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
				p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12]);
			WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			break;

		case DWORD_T:
			WRITE_LINE("%s%s=%d\n", prefix, pTbl[i].name,
				*((unsigned int *)(((long)pMib) + pTbl[i].offset)));
			break;

		case IA_T:
			WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name,
				inet_ntoa(*((struct in_addr *)(((long)pMib) + pTbl[i].offset))));
			break;

		case BYTE_ARRAY_T:
			if ( pTbl[i].id == MIB_HW_TX_POWER_CCK || pTbl[i].id == MIB_HW_TX_POWER_OFDM ) {
				max_chan_num = (pTbl[i].id == MIB_HW_TX_POWER_CCK)? MAX_CCK_CHAN_NUM: MAX_OFDM_CHAN_NUM;
				buf[0] = '\0';
				p = (unsigned char *)(((long)pMib) + pTbl[i].offset);
				for (j=0; j<max_chan_num; j++) {
					sprintf(line, "%d", *(p+ j) );
					if ( j+1 != max_chan_num)
						strcat(line, " ");
					strcat(buf, line);
				}
				WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			}
			break;


		case WLAC_ARRAY_T:
			for (j=0; j<pApMib->wlan[wlan_inx][vwlan_inx].acNum; j++) {
				pWlAc = (MACFILTER_Tp)(((long)pMib) + pTbl[i].offset + j*sizeof(MACFILTER_T));
				sprintf(buf, "%02x%02x%02x%02x%02x%02x", pWlAc->macAddr[0], pWlAc->macAddr[1],
					pWlAc->macAddr[2], pWlAc->macAddr[3], pWlAc->macAddr[4], pWlAc->macAddr[5]);
				if (strlen(pWlAc->comment)) {
					strcat(buf, ", ");
					strcat(buf, pWlAc->comment);
				}
				WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			}
			break;

#if defined(_MESH_ENABLE_) && defined(_MESH_ACL_ENABLE_) // below code copy above ACL code
		case MESH_ACL_ARRAY_T:
			for (j=0; j<pApMib->meshAclNum; j++) {
				pWlAc = (MACFILTER_Tp)(((long)pMib) + pTbl[i].offset + j*sizeof(MACFILTER_T));
				sprintf(buf, "%02x%02x%02x%02x%02x%02x", pWlAc->macAddr[0], pWlAc->macAddr[1],
					pWlAc->macAddr[2], pWlAc->macAddr[3], pWlAc->macAddr[4], pWlAc->macAddr[5]);
				if (strlen(pWlAc->comment)) {
					strcat(buf, ", ");
					strcat(buf, pWlAc->comment);
				}
				WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			}
			break;
#endif

		case WDS_ARRAY_T:
			for (j=0; j<pApMib->wlan[wlan_inx][vwlan_inx].wdsNum; j++) {
				pWds = (WDS_Tp)(((long)pMib) + pTbl[i].offset + j*sizeof(WDS_T));
				sprintf(buf, "%02x%02x%02x%02x%02x%02x", pWds->macAddr[0], pWds->macAddr[1],
					pWds->macAddr[2], pWds->macAddr[3], pWds->macAddr[4], pWds->macAddr[5]);
				if (strlen(pWds->comment)) {
					strcat(buf, ", ");
					strcat(buf, pWds->comment);
				}
				WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			}
			break;

#ifdef HOME_GATEWAY
		case PORTFW_ARRAY_T:
			for (j=0; j<pApMib->portFwNum; j++) {
				pPortFw = (PORTFW_Tp)(((long)pMib) + pTbl[i].offset + j*sizeof(PORTFW_T));
				sprintf(buf, "%s, %d, %d, %d", inet_ntoa(*((struct in_addr *)pPortFw->ipAddr)),
					pPortFw->fromPort, pPortFw->toPort, pPortFw->protoType);
				if (strlen(pPortFw->comment)) {
					strcat(buf, ", ");
					strcat(buf, pPortFw->comment);
				}
				WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			}
			break;

		case PORTFILTER_ARRAY_T:
			for (j=0; j<pApMib->portFilterNum; j++) {
				pPortFilter = (PORTFILTER_Tp)(((long)pMib) + pTbl[i].offset + j*sizeof(PORTFILTER_T));
				sprintf(buf, "%d, %d, %d", pPortFilter->fromPort,
					pPortFilter->toPort, pPortFilter->protoType);
				if (strlen(pPortFilter->comment)) {
					strcat(buf, ", ");
					strcat(buf, pPortFilter->comment);
				}
				WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			}
			break;

		case IPFILTER_ARRAY_T:
			for (j=0; j<pApMib->ipFilterNum; j++) {
				pIpFilter = (IPFILTER_Tp)(((long)pMib) + pTbl[i].offset + j*sizeof(IPFILTER_T));
				sprintf(buf, "%s, %d", inet_ntoa(*((struct in_addr *)pIpFilter->ipAddr)), pIpFilter->protoType);
				if (strlen(pIpFilter->comment)) {
					strcat(buf, ", ");
					strcat(buf, pIpFilter->comment);
				}
				WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			}
			break;

		case MACFILTER_ARRAY_T:
			for (j=0; j<pApMib->macFilterNum; j++) {
				pMacFilter = (MACFILTER_Tp)(((long)pMib) + pTbl[i].offset + j*sizeof(MACFILTER_T));
				sprintf(buf, "%02x%02x%02x%02x%02x%02x", pMacFilter->macAddr[0], pMacFilter->macAddr[1],
					pMacFilter->macAddr[2], pMacFilter->macAddr[3], pMacFilter->macAddr[4], pMacFilter->macAddr[5]);
				if (strlen(pMacFilter->comment)) {
					strcat(buf, ", ");
					strcat(buf, pMacFilter->comment);
				}
				WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			}
			break;

		case TRIGGERPORT_ARRAY_T:
			for (j=0; j<pApMib->triggerPortNum; j++) {
				pTriggerPort = (TRIGGERPORT_Tp)(((long)pMib) + pTbl[i].offset + j*sizeof(TRIGGERPORT_T));
				sprintf(buf, "%d, %d, %d, %d, %d, %d",
					pTriggerPort->tri_fromPort,pTriggerPort->tri_toPort, pTriggerPort->tri_protoType,
					pTriggerPort->inc_fromPort,pTriggerPort->inc_toPort, pTriggerPort->inc_protoType);
				if (strlen(pTriggerPort->comment)) {
					strcat(buf, ", ");
					strcat(buf, pTriggerPort->comment);
				}
				WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			}
			break;

#ifdef GW_QOS_ENGINE
		case QOS_ARRAY_T:
			for (j=0; j<pApMib->qosRuleNum; j++) {
				pQos = (QOS_Tp)(((long)pMib) + pTbl[i].offset + j*sizeof(QOS_T));
                        strcpy(LipS, inet_ntoa(*((struct in_addr*)pQos->local_ip_start)));
                        strcpy(LipE, inet_ntoa(*((struct in_addr*)pQos->local_ip_end)));
                        strcpy(RipS, inet_ntoa(*((struct in_addr*)pQos->remote_ip_start)));
                        strcpy(RipE, inet_ntoa(*((struct in_addr*)pQos->remote_ip_end)));
				sprintf(buf, QOS_FORMAT, pQos->enabled,
                                pQos->priority, pQos->protocol, 
                                LipS, LipE, 
                                pQos->local_port_start, pQos->local_port_end, 
                                RipS, RipE,
					pQos->remote_port_start, pQos->remote_port_end, pQos->entry_name );
				
				WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			}
			break;
#endif

#ifdef QOS_BY_BANDWIDTH
		case QOS_ARRAY_T:
			for (j=0; j<pApMib->qosRuleNum; j++) {
				pQos = (IPQOS_Tp)(((long)pMib) + pTbl[i].offset + j*sizeof(IPQOS_T));
                        strcpy(LipS, inet_ntoa(*((struct in_addr*)pQos->local_ip_start)));
                        strcpy(LipE, inet_ntoa(*((struct in_addr*)pQos->local_ip_end)));
				sprintf(buf, QOS_FORMAT, pQos->enabled,
                                pQos->mac[0],pQos->mac[1],pQos->mac[2],pQos->mac[3],pQos->mac[4],pQos->mac[5],
                                pQos->mode, LipS, LipE, 
                                pQos->bandwidth, 
					pQos->entry_name );
				
				WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			}
			break;
#endif
#ifdef VPN_SUPPORT
		case IPSECTUNNEL_ARRAY_T:

			for (j=0; j<pApMib->ipsecTunnelNum; j++) {
				pIpsecTunnel= (IPSECTUNNEL_Tp)(((long)pMib) + pTbl[i].offset + j*sizeof(IPSECTUNNEL_T));
				strcpy(strLcIp, inet_ntoa(*((struct in_addr*) pIpsecTunnel->lc_ipAddr)));
				strcpy(strRtIp, inet_ntoa(*((struct in_addr*) pIpsecTunnel->rt_ipAddr)));
				strcpy(strRtGw, inet_ntoa(*((struct in_addr*) pIpsecTunnel->rt_gwAddr)));


				sprintf(buf, IPSECTUNNEL_FORMAT, pIpsecTunnel->tunnelId, pIpsecTunnel->enable,
				pIpsecTunnel->connName, pIpsecTunnel->lcType, strLcIp, pIpsecTunnel->lc_maskLen,
				pIpsecTunnel->rtType, strRtIp, pIpsecTunnel->rt_maskLen,
				strRtGw, pIpsecTunnel->keyMode,
				pIpsecTunnel->conType, pIpsecTunnel->espEncr, pIpsecTunnel->espAuth,
				pIpsecTunnel->psKey, pIpsecTunnel->ikeEncr, pIpsecTunnel->ikeAuth, pIpsecTunnel->ikeKeyGroup,
				pIpsecTunnel->ikeLifeTime, pIpsecTunnel->ipsecLifeTime, pIpsecTunnel->ipsecPfs,
				pIpsecTunnel->spi, pIpsecTunnel->encrKey, pIpsecTunnel->authKey
				);

				WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			}
			break;
#endif
		case URLFILTER_ARRAY_T:
			break;
#endif
		default:
			printf("set mib with invalid type[%d, %s]!\n", pTbl[i].type, pTbl[i].name);
			break;
		}
	}

	if (idx == 2 || idx == 4 || idx == 6) {
#ifdef MBSSID
		if (idx == 4 || idx == 6) {
			if (++vwlan_inx <= NUM_VWLAN_INTERFACE)
				goto next_wlan;
			else
				vwlan_inx = 0;
		}
#endif
		if (++wlan_inx < NUM_WLAN_INTERFACE)
			goto next_wlan;
	}

	goto next_config;
}

////////////////////////////////////////////////////////////////////////////////
static void swap_mib_word_value(APMIB_Tp pMib)
{
	int i, j;

//for debug sean
	for (i=0; i<NUM_WLAN_INTERFACE; i++) 
		for (j=0; j<(NUM_VWLAN_INTERFACE+1); j++) 
	{
		pMib->wlan[i][j].fragThreshold = WORD_SWAP(pMib->wlan[i][j].fragThreshold);
		pMib->wlan[i][j].rtsThreshold = WORD_SWAP(pMib->wlan[i][j].rtsThreshold);
		pMib->wlan[i][j].supportedRates = WORD_SWAP(pMib->wlan[i][j].supportedRates);
		pMib->wlan[i][j].basicRates = WORD_SWAP(pMib->wlan[i][j].basicRates);
		pMib->wlan[i][j].beaconInterval = WORD_SWAP(pMib->wlan[i][j].beaconInterval);
		pMib->wlan[i][j].inactivityTime = DWORD_SWAP(pMib->wlan[i][j].inactivityTime);
		pMib->wlan[i][j].wpaGroupRekeyTime = DWORD_SWAP(pMib->wlan[i][j].wpaGroupRekeyTime);
		pMib->wlan[i][j].rsPort = WORD_SWAP(pMib->wlan[i][j].rsPort);
		pMib->wlan[i][j].rsIntervalTime = WORD_SWAP(pMib->wlan[i][j].rsIntervalTime);
		pMib->wlan[i][j].fixedTxRate = WORD_SWAP(pMib->wlan[i][j].fixedTxRate);
	}

#ifdef	_MESH_ENABLE_
	pMib->meshMaxNumOfNeighbors =WORD_SWAP(pMib->meshMaxNumOfNeighbors) ;
//	pMib->meshDefaultTTL    = WORD_SWAP(pMib->meshDefaultTTL) ;
//	pMib->meshHelloInterval = WORD_SWAP(pMib->meshHelloInterval) ;
#endif

#ifdef HOME_GATEWAY
	pMib->pppIdleTime = WORD_SWAP(pMib->pppIdleTime);
	pMib->pptpMtuSize = WORD_SWAP(pMib->pptpMtuSize);
	for (i=0; i<pMib->portFwNum; i++) {
		pMib->portFwArray[i].fromPort = WORD_SWAP(pMib->portFwArray[i].fromPort);
		pMib->portFwArray[i].toPort = WORD_SWAP(pMib->portFwArray[i].toPort);
	}
	for (i=0; i<pMib->portFilterNum; i++) {
		pMib->portFilterArray[i].fromPort = WORD_SWAP(pMib->portFilterArray[i].fromPort);
		pMib->portFilterArray[i].toPort = WORD_SWAP(pMib->portFilterArray[i].toPort);
	}
	for (i=0; i<pMib->triggerPortNum; i++) {
		pMib->triggerPortArray[i].tri_fromPort = WORD_SWAP(pMib->triggerPortArray[i].tri_fromPort);
		pMib->triggerPortArray[i].tri_toPort = WORD_SWAP(pMib->triggerPortArray[i].tri_toPort);
		pMib->triggerPortArray[i].inc_fromPort = WORD_SWAP(pMib->triggerPortArray[i].inc_fromPort);
		pMib->triggerPortArray[i].inc_toPort = WORD_SWAP(pMib->triggerPortArray[i].inc_toPort);
	}
	pMib->pppMtuSize = WORD_SWAP(pMib->pppMtuSize);
	pMib->fixedIpMtuSize = WORD_SWAP(pMib->fixedIpMtuSize);
	pMib->dhcpMtuSize = WORD_SWAP(pMib->dhcpMtuSize);
	pMib->l2tpMtuSize = WORD_SWAP(pMib->l2tpMtuSize); /* keith: add l2tp support. 20080519 */
	pMib->l2tpIdleTime = WORD_SWAP(pMib->l2tpIdleTime); /* keith: add l2tp support. 20080519 */
	
	pMib->pptpIdleTime = WORD_SWAP(pMib->pptpIdleTime); /* brad: add pptp support. 20081125 */
#ifdef GW_QOS_ENGINE
	pMib->qosManualUplinkSpeed = DWORD_SWAP(pMib->qosManualUplinkSpeed);	

	for (i=0; i<pMib->qosRuleNum; i++) {
		pMib->qosRuleArray[i].protocol = WORD_SWAP(pMib->qosRuleArray[i].protocol);
		pMib->qosRuleArray[i].local_port_start = WORD_SWAP(pMib->qosRuleArray[i].local_port_start);
		pMib->qosRuleArray[i].local_port_end = WORD_SWAP(pMib->qosRuleArray[i].local_port_end);
		pMib->qosRuleArray[i].remote_port_start = WORD_SWAP(pMib->qosRuleArray[i].remote_port_start);
		pMib->qosRuleArray[i].remote_port_end = WORD_SWAP(pMib->qosRuleArray[i].remote_port_end);
	}
#endif

#ifdef QOS_BY_BANDWIDTH
	pMib->qosManualUplinkSpeed = DWORD_SWAP(pMib->qosManualUplinkSpeed);	

	for (i=0; i<pMib->qosRuleNum; i++) {
		pMib->qosRuleArray[i].bandwidth = DWORD_SWAP(pMib->qosRuleArray[i].bandwidth);
	}
#endif
	
#ifdef  VPN_SUPPORT
        for (i=0; i<pMib->ipsecTunnelNum; i++) {
                pMib->ipsecTunnelArray[i].ipsecLifeTime = WORD_SWAP(pMib->ipsecTunnelArray[i].ipsecLifeTime);
                pMib->ipsecTunnelArray[i].ikeLifeTime = WORD_SWAP(pMib->ipsecTunnelArray[i].ikeLifeTime);
        }
#endif
#ifdef DOS_SUPPORT
	pMib->dosEnabled = DWORD_SWAP(pMib->dosEnabled);	
	pMib->syssynFlood = WORD_SWAP(pMib->syssynFlood);	
	pMib->sysfinFlood = WORD_SWAP(pMib->sysfinFlood);	
	pMib->sysudpFlood = WORD_SWAP(pMib->sysudpFlood);	
	pMib->sysicmpFlood = WORD_SWAP(pMib->sysicmpFlood);	
	pMib->pipsynFlood = WORD_SWAP(pMib->pipsynFlood);	
	pMib->pipfinFlood = WORD_SWAP(pMib->pipfinFlood);	
	pMib->pipudpFlood = WORD_SWAP(pMib->pipudpFlood);	
	pMib->pipicmpFlood = WORD_SWAP(pMib->pipicmpFlood);	
	pMib->blockTime = WORD_SWAP(pMib->blockTime);
#endif
#endif

// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT
	voip_mibtbl_swap_value(&pMib->voipCfgParam);
#endif
}

////////////////////////////////////////////////////////////////////////////////
static void getVal2(char *value, char **p1, char **p2)
{
	value = getVal(value, p1);
	if ( value )
		getVal(value, p2);
	else
		*p2 = NULL;
}

#ifdef HOME_GATEWAY
////////////////////////////////////////////////////////////////////////////////
static void getVal3(char *value, char **p1, char **p2, char **p3)
{
	*p1 = *p2 = *p3 = NULL;

	value = getVal(value, p1);
	if ( !value )
		return;
	value = getVal(value, p2);
	if ( !value )
		return;
	getVal(value, p3);
}

////////////////////////////////////////////////////////////////////////////////
static void getVal4(char *value, char **p1, char **p2, char **p3, char **p4)
{
	*p1 = *p2 = *p3 = *p4 = NULL;

	value = getVal(value, p1);
	if ( !value )
		return;
	value = getVal(value, p2);
	if ( !value )
		return;
	value = getVal(value, p3);
	if ( !value )
		return;
	getVal(value, p4);
}

////////////////////////////////////////////////////////////////////////////////
static void getVal5(char *value, char **p1, char **p2, char **p3, char **p4, char **p5)
{
	*p1 = *p2 = *p3 = *p4 = *p5 = NULL;

	value = getVal(value, p1);
	if ( !value )
		return;
	value = getVal(value, p2);
	if ( !value )
		return;
	value = getVal(value, p3);
	if ( !value )
		return;
	value = getVal(value, p4);
	if ( !value )
		return;
	getVal(value, p5);
}

////////////////////////////////////////////////////////////////////////////////
static void getVal7(char *value, char **p1, char **p2, char **p3, char **p4, \
	char **p5, char **p6, char **p7)
{
	*p1 = *p2 = *p3 = *p4 = *p5 = *p6 = *p7 = NULL;

	value = getVal(value, p1);
	if ( !value )
		return;
	value = getVal(value, p2);
	if ( !value )
		return;
	value = getVal(value, p3);
	if ( !value )
		return;
	value = getVal(value, p4);
	if ( !value )
		return;
	value = getVal(value, p5);
	if ( !value )
		return;
	value = getVal(value, p6);
	if ( !value )
		return;
	value = getVal(value, p7);
}
#ifdef GW_QOS_ENGINE
////////////////////////////////////////////////////////////////////////////////
void getVal12(char *value, char **p1, char **p2, char **p3, char **p4, char **p5, 
	char **p6, char **p7, char **p8, char **p9, char **p10, char **p11, char **p12)
{
	*p1 = *p2 = *p3 = *p4 = *p5 = *p6 = *p7 = NULL;
    *p8 = *p9 = *p10 = *p11 = *p12 = NULL;
	value = getVal(value, p1);
	if ( !value )
		return;
	value = getVal(value, p2);
	if ( !value )
		return;
	value = getVal(value, p3);
	if ( !value )
		return;
	value = getVal(value, p4);
	if ( !value )
		return;
	value = getVal(value, p5);
	if ( !value )
		return;
	value = getVal(value, p6);
	if ( !value )
		return;
	value = getVal(value, p7);
	if ( !value )
		return;
	value = getVal(value, p8);
	if ( !value )
		return;
	value = getVal(value, p9);
	if ( !value )
		return;
	value = getVal(value, p10);
	if ( !value )
		return;
	value = getVal(value, p11);
	if ( !value )
		return;
	value = getVal(value, p12);
}
#endif
#ifdef VPN_SUPPORT
////////////////////////////////////////////////////////////////////////////////
static void getVal24(char *value, char **p1, char **p2, char **p3, char **p4, char **p5, char **p6, char **p7,\
	char **p8, char **p9, char **p10, char **p11, char **p12, char **p13, char **p14, char **p15, char **p16,\
	char **p17, char **p18, char **p19, char **p20, char **p21, char **p22, char **p23, char **p24)
{
	*p1 = *p2 = *p3 = *p4 = *p5 = *p6 = *p7 = *p8 = *p9 = *p10 = *p11 = *p12\
	= *p13 = *p14 = *p15 = *p16 = *p17 = *p18 = *p19 = *p20 = *p21 = *p22 = *p23 = *p24  = NULL;

	value = getVal(value, p1);
	if ( !value )
		return;
	value = getVal(value, p2);
	if ( !value )
		return;
	value = getVal(value, p3);
	if ( !value )
		return;
	value = getVal(value, p4);
	if ( !value )
		return;
	value = getVal(value, p5);
	if ( !value )
		return;
	value = getVal(value, p6);
	if ( !value )
		return;
	value = getVal(value, p7);
	if ( !value )
		return;
	value = getVal(value, p8);
	if ( !value )
		return;
	value = getVal(value, p9);
	if ( !value )
		return;
	value = getVal(value, p10);
	if ( !value )
		return;
	value = getVal(value, p11);
	if ( !value )
		return;
	value = getVal(value, p12);
	if ( !value )
		return;
	value = getVal(value, p13);
	if ( !value )
		return;
	value = getVal(value, p14);
	if ( !value )
		return;
	value = getVal(value, p15);
	if ( !value )
		return;
	value = getVal(value, p16);
	if ( !value )
		return;
	value = getVal(value, p17);
	if ( !value )
		return;
	value = getVal(value, p18);
	if ( !value )
		return;
	value = getVal(value, p19);
	if ( !value )
		return;
	value = getVal(value, p20);
	if ( !value )
		return;
	value = getVal(value, p21);
	if ( !value )
		return;
	value = getVal(value, p22);
	if ( !value )
		return;
	value = getVal(value, p23);
	if ( !value )
		return;
	value = getVal(value, p24);
	if ( !value )
		return;
}
#endif
#endif

#ifdef WIN32
/* 
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 */
int inet_aton(register const char *cp, struct in_addr *addr)
{
	register in_addr_t val;
	register int base, n;
	register char c;
	unsigned int parts[4];
	register unsigned int *pp = parts;

	c = *cp;
	for (;;) {
		/*
		 * Collect number up to ``.''.
		 * Values are specified as for C:
		 * 0x=hex, 0=octal, isdigit=decimal.
		 */
		if (!isdigit(c))
			return (0);
		val = 0; base = 10;
		if (c == '0') {
			c = *++cp;
			if (c == 'x' || c == 'X')
				base = 16, c = *++cp;
			else
				base = 8;
		}
		for (;;) {
			if (isascii(c) && isdigit(c)) {
				val = (val * base) + (c - '0');
				c = *++cp;
			} else if (base == 16 && isascii(c) && isxdigit(c)) {
				val = (val << 4) |
					(c + 10 - (islower(c) ? 'a' : 'A'));
				c = *++cp;
			} else
				break;
		}
		if (c == '.') {
			/*
			 * Internet format:
			 *	a.b.c.d
			 *	a.b.c	(with c treated as 16 bits)
			 *	a.b	(with b treated as 24 bits)
			 */
			if (pp >= parts + 3)
				return (0);
			*pp++ = val;
			c = *++cp;
		} else
			break;
	}
	/*
	 * Check for trailing characters.
	 */
	if (c != '\0' && (!isascii(c) || !isspace(c)))
		return (0);
	/*
	 * Concoct the address according to
	 * the number of parts specified.
	 */
	n = pp - parts + 1;
	switch (n) {

	case 0:
		return (0);		/* initial nondigit */

	case 1:				/* a -- 32 bits */

// david ----------
		val <<=24;
//-----------------
		
		break;

	case 2:				/* a.b -- 8.24 bits */

// david ----------
		val <<=16;
//-----------------
		if ((val > 0xffffff) || (parts[0] > 0xff))
			return (0);
		val |= parts[0] << 24;
		break;

	case 3:				/* a.b.c -- 8.8.16 bits */
// david ----------
		val <<=8;
//-----------------

		if ((val > 0xffff) || (parts[0] > 0xff) || (parts[1] > 0xff))
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16);
		break;

	case 4:				/* a.b.c.d -- 8.8.8.8 bits */
		if ((val > 0xff) || (parts[0] > 0xff) || (parts[1] > 0xff) || (parts[2] > 0xff))
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
		break;
	}
	if (addr)
		addr->s_addr = htonl(val);
	return (1);
}
#endif
