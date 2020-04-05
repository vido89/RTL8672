/*
 * Save system configuration to file
 */
#include<stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "mibtbl.h"
#include "utility.h"

#define error -1
#define FMT_RAW		1
#define FMT_XML		2

FILE *fp;
char default_path[] = "/tmp/config.xml";
char buffer[4096];
char chainEntry[1024];

void print_name_value(char *name, void *addr, TYPE_T type, int size)
{
	char svalue[512], tmp[16];
	unsigned char *pchar;
	unsigned int vUInt;
	int	vInt, i;
	
	switch (type) {
		case IA_T:
			strcpy(svalue, inet_ntoa(*((struct in_addr *)addr)));
			if ( ((struct in_addr *)addr)->s_addr == INADDR_NONE )
				svalue[0] = '\0';
			else
				strcpy(svalue, inet_ntoa(*((struct in_addr *)addr)));
			break;
		case STRING_T:
			strcpy(svalue, (char *)addr);
			printf("size=%d, %02x%02x%02x%02x%02x%02x%02x%02x\n", size, svalue[0], svalue[1], svalue[2], svalue[3], svalue[4], svalue[5], svalue[6], svalue[7]);
			break;
		case BYTE_T:
			vUInt = (unsigned int) (*(unsigned char *)addr);
			sprintf(svalue,"%u",vUInt);
			break;
		case WORD_T:
			vUInt = (unsigned int) (*(unsigned short *)addr);
			sprintf(svalue,"%u",vUInt);
			break;
		case INTEGER_T:
			vInt = *((int *)addr);
			sprintf(svalue,"%d",vInt);
			break;
		case DWORD_T:
			vUInt = (unsigned int) (*(unsigned int *)addr);
			sprintf(svalue,"%u",vUInt);
			break;
		case BYTE5_T:
			pchar = (unsigned char *)addr;
			sprintf(svalue, "%02x%02x%02x%02x%02x", 
				pchar[0], pchar[1], pchar[2], pchar[3], pchar[4]);
			break;
		case BYTE6_T:
			pchar = (unsigned char *)addr;
			sprintf(svalue, "%02x%02x%02x%02x%02x%02x", 
				pchar[0], pchar[1],pchar[2], pchar[3], pchar[4], pchar[5]);
			break;
		case BYTE13_T:
			pchar = (unsigned char *)addr;
			sprintf(svalue, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
				pchar[0], pchar[1],pchar[2], pchar[3], pchar[4], pchar[5],
				pchar[6], pchar[7],pchar[8], pchar[9], pchar[10], pchar[11], pchar[12]);
			break;
		case BYTE_ARRAY_T:
			pchar = (unsigned char *)addr;
			for (i=0; i<size; i++)
			{
				sprintf(tmp, "%d", pchar[i]);
				if (i != (size-1))
					strcat(tmp, ",");
				strcat(svalue, tmp);
			}
			break;
		default:
			printf("Unknown data type !\n");
	}
	
	fprintf(fp,"<Value Name=\"%s\" Value=\"%s\"/>\n", name, svalue);
	printf("<Value Name=\"%s\" Value=\"%s\"/>\n", name, svalue);
}

void show_usage()
{
	fprintf(stderr,	"Usage: saveconfig [ -f filename ] [ -t raw/xml ]\n");
}

int main(int argc, char **argv)
{
	int i, k;
	int opt;
	char userfile[64];
	char *loadfile;
	int filefmt;
	mib_table_entry_T info;
	mib_chain_record_table_entry_T chainInfo;
	mib_chain_member_entry_T *rec_desc;
	
	loadfile = default_path;
	filefmt = FMT_XML;
	/* do normal option parsing */
	while ((opt = getopt(argc, argv, "f:t:")) > 0) {
		switch (opt) {
			case 'f':
				strncpy(userfile, optarg, 64);
				userfile[63] = '\0';
				loadfile = userfile;
				break;
			case 't':
				if (!strcmp("raw", optarg))
					filefmt = FMT_RAW;
				else if (!strcmp("xml", optarg))
					filefmt = FMT_XML;
				else {
					show_usage();
					return error;
				}
				break;
			default:
				show_usage();
				return error;
		}
	}
	
	fp=fopen(loadfile,"w");
	fprintf(fp,"%s\n", CONFIG_HEADER);
	printf("%s\n", CONFIG_HEADER);
	i=0;
	
	// MIB Table
	while (1) {
		if (!mib_info_index(i++, &info))
			return error;
		
		if (info.id == 0)
			break;		
		//printf("get table entry %s\n", info.name);
		mib_get(info.id, (void *)buffer);
		print_name_value(info.name, buffer, info.type, info.size);
	}
	
	//MIB chain record
	i = 0;
	while (1) {
		unsigned int entryNum, index, offset;
		void *value;
		
		if (!mib_chain_info_index(i++, &chainInfo))
			return error;
		
		if (chainInfo.id == 0)
			break;
		
		entryNum = mib_chain_total(chainInfo.id);
		if (mib_chain_desc_id(chainInfo.id, (void *)&buffer)==0) {
			printf("Empty MIB chain %s descriptor !\n", chainInfo.name);
			continue;
		}
			
		rec_desc = (mib_chain_member_entry_T *)&buffer[0];
		printf("chain entry %d # %d\n", chainInfo.id, entryNum);
		
		if (entryNum == 0) {
			fprintf(fp,"<chain chainName=\"%s\">\n", chainInfo.name);
			printf("<chain chainName=\"%s\">\n", chainInfo.name);
			fprintf(fp,"</chain>\n");
			printf("</chain>\n");
		}
		else {
			for(index=0;index<entryNum;index++){
				mib_chain_get(chainInfo.id, index, (void *)&chainEntry[0]);
				fprintf(fp,"<chain chainName=\"%s\">\n", chainInfo.name);
				printf("<chain chainName=\"%s\">\n", chainInfo.name);
				k = 0; offset = 0;
				while (rec_desc[k].name[0] != 0) {
					print_name_value(rec_desc[k].name, chainEntry+offset, rec_desc[k].type, rec_desc[k].size);
					offset+=rec_desc[k++].size;
				}
				fprintf(fp,"</chain>\n");
				printf("</chain>\n");
			}
		}
	}
	
	fprintf(fp,"%s\n", CONFIG_TRAILER);
	printf("%s\n", CONFIG_TRAILER);
	fclose(fp);
	return 0;
}

