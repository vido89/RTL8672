/*
 * Load configuration file and update to the system
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "mibtbl.h"
#include "utility.h"

#define error -1
#define chain_record_number	(MIB_CHAIN_TBL_END-CHAIN_ENTRY_TBL_ID+1)

FILE *fp;
char default_path[] = "/tmp/config.xml";
char chain_updated[chain_record_number];
char LINE[256];
char buffer[4096];
char chainEntry[1024];
char header_str1[64], header_str2[64];
char trailer_str1[64], trailer_str2[64];
mib_table_entry_T *info;
int info_total;

static int get_line(char *s, int size)
{
	while (1) {
		if (!fgets(s,size,fp))
			return error;
		if (s[0] != '\n' && s[0] != '\r' && s[0] != '#')
			break;
	}
	
	//printf("get line: %s\n", s);
	return 0;
}

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

static int put_value(char *entry, TYPE_T type, char *vstr, int size)
{
	struct in_addr *ipAddr;
	unsigned char *vChar;
	unsigned short *vShort;
	unsigned int *vUInt;
	int *vInt;
	
	switch (type) {
		case IA_T:
			ipAddr = (struct in_addr *)entry;
			ipAddr->s_addr = inet_addr(vstr);
			break;
		case BYTE_T:
			vChar = (unsigned char *)entry;
			*vChar = (unsigned char)strtol(vstr, 0, 0);
			break;
		case WORD_T:
			vShort = (unsigned short *)entry;
			*vShort = (unsigned short)strtol(vstr, 0, 0);
			break;
		case DWORD_T:
			vUInt = (unsigned int *)entry;
			*vUInt = (unsigned int)strtol(vstr, 0, 0);
			break;
		case INTEGER_T:
			vInt = (int *)entry;
			*vInt = (int)strtol(vstr, 0, 0);
			break;
		case BYTE5_T:
			string_to_hex(vstr, entry, 10);
			break;
		case BYTE6_T:
			string_to_hex(vstr, entry, 12);
			break;
		case BYTE13_T:
			string_to_hex(vstr, entry, 26);
			break;
		case STRING_T:
			strcpy(entry, vstr);
			break;
		case BYTE_ARRAY_T:
			string_to_hex(vstr, entry, size*2);
			break;
		default:
			printf("Unknown data type !\n");
	}
	
	return 0;
}

static int table_setting(char *line)
{
	int i;
	char *ptoken;
	mib_table_entry_T info_entry;
	unsigned char buffer[128];
	struct in_addr ipAddr;
	unsigned char vChar;
	unsigned short vShort;
	unsigned int vUInt;
	int vInt;
	
	// get name
	ptoken = strtok(line, "\"");
	ptoken = strtok(0, "\"");
	//printf("table name=%s\n", ptoken);
	
	for(i=0; i<info_total; i++){
		if(!strcmp(((mib_table_entry_T*)(info+i))->name, ptoken)){
			memcpy(&info_entry,(mib_table_entry_T*)(info+i),sizeof(mib_table_entry_T));
			break;
		}
	}
	
	if(i>info_total)
		return error;
	
	// get value
	ptoken = strtok(0, "\"");
	ptoken = strtok(0, "\"");
	//printf("table value=%s\n", ptoken);
	
		switch (info_entry.type) {
			case IA_T:
				ipAddr.s_addr = inet_addr(ptoken);
				if (!mib_set(info_entry.id, (void *)&ipAddr)) {
					printf("Set MIB[%s] error!\n", info_entry.name);
					return error;
				}
				break;
			case BYTE_T:
				vChar = (unsigned char)strtol(ptoken, 0, 0);
				if (!mib_set(info_entry.id, (void *)&vChar)) {
					printf("Set MIB[%s] error!\n", info_entry.name);
					return error;
				}
				break;
			case WORD_T:
				vShort = (unsigned short)strtol(ptoken, 0, 0);
				if (!mib_set(info_entry.id, (void *)&vShort)) {
					printf("Set MIB[%s] error!\n", info_entry.name);
					return error;
				}
				break;
			case DWORD_T:
				//sscanf(str2,"%u",&vUInt);
				vUInt = (unsigned int)strtol(ptoken, 0, 0);
				if (!mib_set(info_entry.id, (void *)&vUInt)) {
					printf("Set MIB[%s] error!\n", info_entry.name);
					return error;
				}
				break;
			case INTEGER_T:
				//sscanf(str2,"%d",&vInt);
				vInt = (int)strtol(ptoken, 0, 0);
				if (!mib_set(info_entry.id, (void *)&vInt)) {
					printf("Set MIB[%s] error!\n", info_entry.name);
					return error;
				}
				break;
			case BYTE5_T:
				string_to_hex(ptoken, buffer, 10);
				if (!mib_set(info_entry.id, (void *)&buffer)) {
					printf("Set MIB[%s] error!\n", info_entry.name);
					return error;
				}
				break;
			case BYTE6_T:
				string_to_hex(ptoken, buffer, 12);
				if (!mib_set(info_entry.id, (void *)&buffer)) {
					printf("Set MIB[%s] error!\n", info_entry.name);
					return error;
				}
				break;
			case BYTE13_T:
				string_to_hex(ptoken, buffer, 26);
				if (!mib_set(info_entry.id, (void *)&buffer)) {
					printf("Set MIB[%s] error!\n", info_entry.name);
					return error;
				}
				break;
			case STRING_T:
				if (!mib_set(info_entry.id, (void *)ptoken)) {
					printf("Set MIB[%s] error!\n", info_entry.name);
					return error;
				}
				break;
			case BYTE_ARRAY_T:
				if (info_entry.id != MIB_ADSL_TONE) {
					string_to_hex(ptoken, buffer, 28);
					if (!mib_set(info_entry.id, (void *)&buffer)) {
						printf("Set MIB[%s] error!\n", info_entry.name);
						return error;
					}
				}
				break;
		}
	return 0;
}

static int chain_setting(char *line)
{
	int i, k;
	char *ptoken;
	mib_chain_record_table_entry_T chainInfo;
	mib_chain_member_entry_T *rec_desc;
	
	// get chain name
	ptoken = strtok(line, "\"");
	ptoken = strtok(0, "\"");
	printf("Chain name=%s\n", ptoken);
	// get chain info
	if (!mib_chain_info_name(ptoken, &chainInfo)) {
		printf("Invalid chain name: %s\n", ptoken);
		return error;
	}
	// get chain descriptor
	if (mib_chain_desc_id(chainInfo.id, (void *)&buffer)==0) {
		printf("Empty MIB chain %s descriptor !\n", chainInfo.name);
		rec_desc = 0;
	}
	else
		rec_desc = (mib_chain_member_entry_T *)&buffer[0];
	//clear orginal record
	if(chain_updated[chainInfo.id-CHAIN_ENTRY_TBL_ID]==0){
		mib_chain_clear(chainInfo.id);//clear chain record
		chain_updated[chainInfo.id-CHAIN_ENTRY_TBL_ID]=1;
	}
	
	while(!feof(fp)) {
		get_line(LINE, sizeof(LINE));
		// remove leading space
		i = 0; k = 0;
		while (LINE[i++]==' ')
			k++;
		if (!strncmp(&LINE[k], "</chain", 7))
			break; // end of chain
		// get name
		ptoken = strtok(&LINE[k], "\"");
		ptoken = strtok(0, "\"");
		printf("name=%s\n", ptoken);
		if (rec_desc == 0)
			continue;
		i = 0;
		while (rec_desc[i].name[0] != 0) {
			if (!strcmp(rec_desc[i].name, ptoken))
				break;
			i++;
		}
		if (rec_desc[i].name[0] == 0) {
			printf("Chain %s member %s not found !\n", chainInfo.name, ptoken);
			return error;
		}
		
		// get value
		ptoken = strtok(0, "\"");
		ptoken = strtok(0, "\"");
		printf("value=%s\n", ptoken);
		// put value
		put_value(&chainEntry[rec_desc[i].offset], rec_desc[i].type, ptoken, rec_desc[i].size);
	}
	
	if (rec_desc != 0)
		mib_chain_add(chainInfo.id, (void *)chainEntry);
	return 0;
}

static int update_setting(char *line)
{
	int i, k;
	char str[32];
	
	// remove leading space
	i = 0; k = 0;
	while (line[i++]==' ')
		k++;
	sscanf(line, "%s", str);
	//printf("str=%s\n", str);
	if (!strcmp(str, "<Value"))
		table_setting(&line[k]);
	else if (!strcmp(str, "<chain"))
		chain_setting(&line[k]);
	else
		printf("Unknown statement: %s\n", line);
	
	return 0;
}

void show_usage()
{
	fprintf(stderr,	"Usage: loadconfig [ -f filename ]\n");
}

int main(int argc, char **argv)
{
	int i;
	int opt;
	char userfile[64];
	char *loadfile;
	
	loadfile = default_path;
	/* do normal option parsing */
	while ((opt = getopt(argc, argv, "f:")) > 0) {
		switch (opt) {
			case 'f':
				strncpy(userfile, optarg, 64);
				userfile[63] = '\0';
				loadfile = userfile;
				break;
			default:
				show_usage();
				return error;
		}
	}
	
	/* initialize chain update flags */
	for(i=0;i<chain_record_number;i++){
		chain_updated[i]=0;
	}
	
	printf("Get user specific configuration file......\n\n");
	if (!(fp = fopen(loadfile, "r"))) {
		printf("User configuration file not exist: %s\n", loadfile);
		return error;
	}
	
	// generate header string
	sprintf(header_str1, "%s\n", CONFIG_HEADER);
	sprintf(header_str2, "%s\r\n", CONFIG_HEADER);
	// generate trailer string
	sprintf(trailer_str1, "%s\n", CONFIG_TRAILER);
	sprintf(trailer_str2, "%s\r\n", CONFIG_TRAILER);
	
	get_line(LINE, sizeof(LINE));
	if(strcmp(LINE, header_str1) && strcmp(LINE, header_str2)) {
		printf("Invalid config file!\n");
		fclose(fp);
		return error;
	}
	
	info_total = mib_info_total();
	info=(mib_table_entry_T *)malloc(sizeof(mib_table_entry_T)*info_total);
	
	for(i=0;i<info_total;i++){
		if(!mib_info_index(i,info+i))
			break;
	}
	
	if(i<info_total){
		free(info);
		printf("get mib info total entry error!\n");
		return error;
	}
	
	while(!feof(fp)) {
		get_line(LINE, sizeof(LINE));//get one line from the file
		if( (!strcmp(LINE, trailer_str1)) || !strcmp(LINE, trailer_str2))
			break; // end of configuration
		
		if(update_setting(LINE) < 0) {
			fclose(fp);
			printf("update setting fail!\n");
			free(info);
			return error;
		}
	}
	
	free(info);
	fclose(fp);
	printf("Restore settings from config file successful! \n");
	return 0;
}
