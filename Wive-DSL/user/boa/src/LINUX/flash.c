
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/atm.h>

/* for open(), lseek() */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* for opendir, readdir, kill */
#include <dirent.h>
#include <signal.h>

/* for ioctl */
#include <sys/ioctl.h>
#include <netinet/if_ether.h>
#include <net/if.h>
#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../../uClibc/include/linux/autoconf.h"
#endif

#include "mibtbl.h"

//#define FLASH_TEST

extern int flash_read(char *buf, int offset, int len);
extern int flash_write(char *buf, int offset, int len);
extern void __mib_init_mib_header(void);
extern int __mib_header_read(CONFIG_DATA_T data_type, PARAM_HEADER_Tp pHeader);
extern int __mib_header_check(CONFIG_DATA_T data_type, PARAM_HEADER_Tp pHeader);
extern int __mib_content_decod_check(CONFIG_DATA_T data_type, PARAM_HEADER_Tp pHeader, unsigned char* ptr);


int mib_tbl_get_by_name(char* name, char* arg, void *value);
int mib_get_by_name(char* name, void *value);
int mib_set_by_name(char* name, char *value);
void mib_get_all(void);
#ifdef CAN_RW_FILE	
int flash_read_to_file(CONFIG_DATA_T data_type, char * filename);
int flash_write_from_file(CONFIG_DATA_T data_type, char * filename);
#endif // #ifdef CAN_RW_FILE				
void unzip_web(char *file, char *dest_dir);

int getInAddr( char *interface, void *pAddr )
{
	struct ifreq ifr;
	int skfd, found=0;
	struct sockaddr_in *addr;

	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket");
		return found;
	}

	strcpy(ifr.ifr_name, interface);
	if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)
	{
		close(skfd);
		return (0);
	}

	if (ioctl(skfd, SIOCGIFADDR, &ifr) == 0) {
		addr = ((struct sockaddr_in *)&ifr.ifr_addr);
		*((struct in_addr *)pAddr) = *((struct in_addr *)&addr->sin_addr);
		found = 1;
	}

	close( skfd );
	return found;
}

static int __inline__ _is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}

static int __inline__ string_to_hex(char *string, unsigned char *key, int len)
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

/*
 * Show Flash File System Help
 */
static void showHelp(void)
{
	printf("Usage: flash cmd\n");
	printf("cmd:\n");
	printf("  get MIB-NAME \t\t\tget a specific mib from flash memory.\n");
	printf("  set MIB-NAME MIB-VALUE \tset a specific mib into flash memory.\n");
	printf("  all \t\t\t\tdump all flash parameters.\n");
#ifdef CAN_RW_FILE	
	printf("  wds FILENAME \t\t\twrite flash parameters to default settings.\n");
	printf("  wcs FILENAME \t\t\twrite flash parameters to current settings.\n");
	printf("  whs FILENAME \t\t\twrite flash parameters to hardware settings.\n");
	printf("  rds FILENAME \t\t\tread flash parameters from default settings.\n");
	printf("  rcs FILENAME \t\t\tread flash parameters from current settings.\n");
	printf("  rhs FILENAME \t\t\tread flash parameters from hardware settings.\n");
#endif // #ifdef CAN_RW_FILE				
#ifdef VOIP_SUPPORT
	printf(" voip \t\t\tvoip setings.\n");
#endif /*VOIP_SUPPORT*/
	printf("  reset \t\t\treset current setting to default.\n");
	printf("  default <cs/hs>\t\twrite program default value to flash.\n");
	printf("  dump <cs/hs>\t\t\tdump flash setting.\n");
	printf("  unzip web\t\t\tunzip web pages.\n\n");
}

// Kaohj --- close the interfaces before writing to flash
static void onoffItf(int on)
{
	if (on) {
		//system("ifconfig eth0 up");
		itfcfg("eth0", 1);
		//system("ifconfig wlan0 up");
		itfcfg("wlan0", 1);
		// SAR_ENABLE
		itfcfg("sar", 1);
	}
	else {
		// SAR disable
		itfcfg("sar", 0);
		//system("ifconfig eth0 down");
		itfcfg("eth0", 0);
		//system("ifconfig wlan0 down");
		itfcfg("wlan0", 0);
		sleep(2);
	}
}

int main(int argc, char** argv)
{
	int action=0;
	int argNum=1;
	char mib[100]={0};
	char mibvalue[256+1]={0};
#ifdef CAN_RW_FILE			
	CONFIG_DATA_T data_type;
#endif // #ifdef CAN_RW_FILE			
	
	// Kaohj
	flashdrv_init();
	
	if ( argc > 1 ) 
	{
		if ( !strcmp(argv[argNum], "info") ) {
			printf("CS offset = %x\n",CURRENT_SETTING_OFFSET);
			printf("DS offset = %x\n",DEFAULT_SETTING_OFFSET);
			printf("HS offset = %x\n",HW_SETTING_OFFSET);
			printf("WEB offset = %x\n",WEB_PAGE_OFFSET);
			printf("CODE offset = %x\n",CODE_IMAGE_OFFSET);

			return 0;
		}
		else if ( !strcmp(argv[argNum], "loop") ) {
			while(1)
			{
				usleep(1000);
			}
		}
		else if ( !strcmp(argv[argNum], "get") ) {
			action = 1;
			argNum++;
		}
		else if ( !strcmp(argv[argNum], "set") ) {
			action = 2;
			argNum++;
		}
		else if ( !strcmp(argv[argNum], "all") ) {
			action = 3;
		}
#ifdef CAN_RW_FILE			
		else if ( !strcmp(argv[argNum], "rds") ) {
			action = 4;
			argNum++;
			data_type = DEFAULT_SETTING;
		}
		else if ( !strcmp(argv[argNum], "rcs") ) {
			action = 5;			
			argNum++;
			data_type = CURRENT_SETTING;
		}
		else if ( !strcmp(argv[argNum], "rhs") ) {
			action = 6;
			argNum++;
			data_type = HW_SETTING;
		}
		else if ( !strcmp(argv[argNum], "wds") ) {
			action = 7;
			argNum++;
			data_type = DEFAULT_SETTING;
		}
		else if ( !strcmp(argv[argNum], "wcs") ) {
			action = 8;
			argNum++;
			data_type = CURRENT_SETTING;
		}
		else if ( !strcmp(argv[argNum], "whs") ) {
			action = 9;
			argNum++;
			data_type = HW_SETTING;
		}
#endif // #ifdef CAN_RW_FILE					
		else if ( !strcmp(argv[argNum], "reset") ) {
			action = 10;
		}
		else if ( !strcmp(argv[argNum], "default") ) {
			action = 11;
		}
		else if ( !strcmp(argv[argNum], "unzip") ) {
			action = 12;
			argNum++;
		}
		else if ( !strcmp(argv[argNum], "check") ) {
			action = 13;
		}
		else if ( !strcmp(argv[argNum], "clear") ) {
			action = 14;
		}
		else if ( !strcmp(argv[argNum], "kill") ) {
			action = 15;
			argNum++;
		}
		else if ( !strcmp(argv[argNum], "ip") ) {
			action = 16;
			argNum++;
		}
		else if ( !strcmp(argv[argNum], "dump") ) {
			action = 17;
 			argNum++;
		}
#ifdef FLASH_TEST
		// Kaohj, for test
		else if ( !strcmp(argv[argNum], "test") ) {
			action = 18;
		}
#endif
/*+++++add by Jack for VoIP project 20/03/07+++++*/
#ifdef VOIP_SUPPORT
		else if(!strcmp(argv[argNum], "voip")){
			if (argc >= 2) // have voip param
				return flash_voip_cmd(argc - 2, &argv[2]);
			else
				return flash_voip_cmd(0, NULL);
		}
	
#endif /*VOIP_SUPPORT*/
/*-----end-----*/
#if 0 //def CONFIG_WIFI_SIMPLE_CONFIG
		else if ( !strcmp(argv[argNum], "upd-wsc-conf") ) {
			return updateWscConf(argv[argNum+1], argv[argNum+2], 0);			
		}

		else if ( !strcmp(argv[argNum], "gen-pin") ) {
			return updateWscConf(0, 0, 1);			
		}		
		else if (!strcmp(argv[argNum], "-param_file")) {
			readFileSetParam(argv[argNum+1]);
			return 0;
			
		}
#endif // WIFI_SIMPLE_CONFIG
	}
	
	if(action==0)
	{
		showHelp();
		return 0;
	}

	if(action==10)
	{
		if(mib_reset(CURRENT_SETTING) != 0)
		{
			printf("reset current settings\n");
		}
		else
		{
			printf("reset current settings fail!\n");
		}
		return 0;
	}

	if(action==17)
	{
		PARAM_HEADER_T header;
		unsigned int fileSize, i;
		unsigned char *pFile=NULL;
		CONFIG_DATA_T data_type;
		int file_offset, table_size;
		char tdata[SIGNATURE_LEN+1];
	
		if (argc <=2) {
			printf("command error!\n");
			showHelp();
			return 0;
		}
		if ( !strcmp(argv[argNum], "ds") )
		{
			data_type = DEFAULT_SETTING;
			file_offset = DEFAULT_SETTING_OFFSET;
			table_size = sizeof(MIB_T);
		}
		else if ( !strcmp(argv[argNum], "cs") )
		{
			data_type = CURRENT_SETTING;
			file_offset = CURRENT_SETTING_OFFSET;
			table_size = sizeof(MIB_T);
		}
		else if ( !strcmp(argv[argNum], "hs") )
		{
			data_type = HW_SETTING;
			file_offset = HW_SETTING_OFFSET;
			table_size = sizeof(HW_MIB_T);
		}
		else
		{
			return 0;
		}

		if(__mib_header_read(data_type, &header) != 1)
			return 0;

		fileSize = header.len + sizeof(PARAM_HEADER_T);
		pFile = malloc(fileSize);
		if ( pFile == NULL )
			return 0;

		if(__mib_file_read(data_type, pFile, fileSize) != 1)
			return 0;

		printf("Header Length: %d\tData Length: %d (Table=%d, Chain=%d)\n",
			sizeof(PARAM_HEADER_T), header.len, table_size, header.len-table_size);
		for (i=0; i<SIGNATURE_LEN; i++)
			tdata[i] = pFile[i];
		tdata[SIGNATURE_LEN] = '\0';
		printf("Signature: %s\nVersion: %d\nChecksum: 0x%.2x\nLength: %d\n", tdata, header.version, header.checksum, header.len);
		printf("------------------ Header+Data ------------------------\n");
		for (i=0;i<fileSize;i+=16)
		{
			//printf("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n"
			printf("%.5x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x\n"
				,file_offset+i ,pFile[i], pFile[i+1], pFile[i+2], pFile[i+3], pFile[i+4], pFile[i+5], pFile[i+6], pFile[i+7], pFile[i+8], pFile[i+9], pFile[i+10], pFile[i+11], pFile[i+12], pFile[i+13], pFile[i+14], pFile[i+15]);
		}

		if(pFile) free(pFile);
		return 0;
	}
	
	if(action==11)
	{
		if (argc <=2) {
			printf("command error!\n");
			showHelp();
			return 0;
		}
#ifdef CLOSE_ITF_BEFORE_WRITE
		// close interfaces
		onoffItf(0);
#endif
		if ( !strcmp(argv[argNum+1], "ds") )
		{
			mib_init_mib_with_program_default(DEFAULT_SETTING, FLASH_DEFAULT_TO_ALL);
		}
		else if ( !strcmp(argv[argNum+1], "cs") )
		{
			mib_init_mib_with_program_default(CURRENT_SETTING, FLASH_DEFAULT_TO_ALL);
		}
		else if ( !strcmp(argv[argNum+1], "hs") )
		{
			mib_init_mib_with_program_default(HW_SETTING, FLASH_DEFAULT_TO_ALL);
		}
#ifdef CLOSE_ITF_BEFORE_WRITE
		// open interfaces
		onoffItf(1);
#endif

		return 0;
	}
	
	if(action==12)
	{
		if(argNum < argc)
		{
			if ( !strcmp(argv[argNum], "web") ) 
			{
#ifdef EMBED
				unzip_web(NULL,"/var/web/");
#else
				if((argc - argNum) == 2)
					unzip_web(argv[argNum],argv[argNum+1]);
				else if((argc - argNum) == 1)
					unzip_web(argv[argNum],"/home/unzip_web/");
				else			
					unzip_web("webpages.bin","/home/unzip_web/");
#endif				
			}
		}
		return 0;
	}

	if(action==13)
	{
		int i;
		PARAM_HEADER_T header;
		CONFIG_DATA_T data_type;
		unsigned char* pFlashBank;
		unsigned int fileSize = 0, reset_flash;
		unsigned char* pFile = NULL;
		unsigned char* pContent = NULL;
		// Kaohj, check for mib chain
		unsigned char* pVarLenTable = NULL;
		unsigned int contentMinSize = 0;
		unsigned int varLenTableSize = 0;

		for(i=0;i<3;i++)
		{
			if(i==0)
			{
				data_type = CURRENT_SETTING;
				pFlashBank = "CS_BANK";
			}
			else if(i==1)
			{
				data_type = DEFAULT_SETTING;
				pFlashBank = "DS_BANK";
				
				// Added by Mason Yu. Not use default seetting.
				continue;
			}
			else if(i==2)
			{
				data_type = HW_SETTING;
				pFlashBank = "HS_BANK";
			}
			reset_flash=0;

check_flash:
			if(__mib_header_read(data_type, &header) != 1)
				goto flash_check_error;

			if(__mib_header_check(data_type, &header) != 1)
				goto flash_check_error;

			fileSize = header.len + sizeof(PARAM_HEADER_T);
			pFile = malloc(fileSize);
			if ( pFile == NULL )
				goto error_return;

			if(__mib_file_read(data_type, pFile, fileSize) != 1)
				goto flash_check_error;

			pContent = &pFile[sizeof(PARAM_HEADER_T)];	// point to start of MIB data 

			if(__mib_content_decod_check(data_type, &header, pContent) != 1)
				goto flash_check_error;

			// Kaohj, check for mib chain
			contentMinSize = __mib_content_min_size(data_type);
			varLenTableSize = header.len - contentMinSize;
			if(varLenTableSize > 0)
			{
				pVarLenTable = &pContent[contentMinSize];	// point to start of variable length MIB data 
		
				// parse variable length MIB data
				if( __mib_chain_record_content_decod(pVarLenTable, varLenTableSize) != 1)
				{
					//printf("mib_chain_record decode fail !\n");
					goto flash_check_error;
				}
			}

			free(pFile);
			printf("%s ok! (minsize=%d, varsize=%d)\n", pFlashBank, contentMinSize, varLenTableSize);
			continue;
flash_check_error:
                        if (!reset_flash) {  //not reset yet
                            if (mib_init_mib_with_program_default(data_type, FLASH_DEFAULT_TO_AUGMENT)) {  //try to reset flash
                            	printf("%s check fail! Reset to default\n", pFlashBank);
                                reset_flash=1;
                                goto check_flash;  //check again
                            };
                        };
error_return:
			if(pFile) free(pFile);
			printf("%s fail!\n", pFlashBank);			
			return -1;
		}
	
		return 0;
	}

	if(action==14)
	{
		unsigned char buf[] = "CLEAR";

		flash_write(buf, CURRENT_SETTING_OFFSET, sizeof(buf));
		flash_write(buf, DEFAULT_SETTING_OFFSET, sizeof(buf));
		flash_write(buf, HW_SETTING_OFFSET, sizeof(buf));

		return 0;
	}

	if(action==15)
	{
		DIR * d;
		struct dirent * de;
		char *ext;
		char psbuf[256];
		int i, h, l;
		char* appName;

		if (argc < argNum)
			return 0;

		appName = argv[argNum];

		d = opendir("/proc");
		if (!d) {
			return 0;
		}

		while (de = readdir(d)) {

			for(i=0;i<strlen(de->d_name);i++)
				if (!isdigit(de->d_name[i]))
					goto next;

			sprintf(psbuf, "/proc/%s/stat", de->d_name);

			h = open(psbuf, O_RDONLY);

			if (h==-1)
				continue;
				
			l = read(h, psbuf, 255);
			if (l<=0) {
				close(h);
				continue;
			}

			psbuf[l] = '\0';
			psbuf[255] = '\0';

			ext = strrchr(psbuf, ')');
			ext[0] = '\0';

			ext = strrchr(psbuf, '(')+1;

			if ( !strcmp(appName, ext) ) {
				int pid;

				sscanf(de->d_name, "%d", &pid);

				if (kill(pid, SIGTERM) != 0) {
					printf( "Could not kill pid '%d' \n", pid);
				} else {
					printf("%s is Killed\n",appName);
				}

				return 0;
			}

			next:
				;
		}

		return 0;
	}

	if(action==16)
	{
		struct in_addr ipAddr;

		if (getInAddr( argv[argNum], (void *)&ipAddr) != 1)
			printf("IP=0.0.0.0\n");
		else
			printf("IP=%s\n", inet_ntoa(ipAddr));

		return 0;
	}
#ifdef FLASH_TEST
	// Kaohj, for test
	if(action==18)
	{
		int idx;
		volatile int i;
		
		printf("writing start at 0xbfc20000 ");
		for (idx=0; idx<=23; idx++) {
			// write start at offset 0x20000, each block 0x10000
			flash_write(0x80500000, 0x20000+idx*0x10000, 0x10000);
			printf(".");
			i=0;
			usleep(200000);
			//while(i<=0xfffff)
			//	i++;
		}
		printf("write Ok\n");
		return 0;
	}
#endif
	
	mib_init();

	if(action==1)
	{
		unsigned char buffer[512];
		while(argNum < argc)
		{
			memset(buffer, 0x00 , 512);
			sscanf(argv[argNum++], "%s", mib);

			if(mib_tbl_get_by_name(mib, argv[argNum], buffer) != 0)
			{
				argNum++;
				printf("%s\n",buffer);				
			}
			else if(mib_get_by_name(mib, buffer) != 0)
			{
				printf("%s=%s\n",mib,buffer);
			}
		}
	}

	if(action==2)
	{
		while((argNum + 1) < argc)
		{
			sscanf(argv[argNum++], "%s", mib);
			sscanf(argv[argNum++], "%s", mibvalue);

			if(mib_set_by_name(mib, mibvalue) != 0)
				printf("set %s=%s\n",mib,mibvalue);
			else
				printf("set %s=%s fail!\n",mib,mibvalue);			
		}

#ifdef CLOSE_ITF_BEFORE_WRITE
		// close interfaces
		onoffItf(0);
#endif
		if(_mib_update(CURRENT_SETTING) == 0)
			printf("CS Flash error! \n");

		if(_mib_update(HW_SETTING) == 0)
			printf("HS Flash error! \n");
#ifdef CLOSE_ITF_BEFORE_WRITE
		// open interfaces
		onoffItf(1);
#endif

	}

	if(action==3)
	{
		mib_get_all();
	}	

#ifdef CAN_RW_FILE			
	if((action>=4) && (action<=6))
	{
		if(argNum >= argc)
			return 0;
			
		sscanf(argv[argNum++], "%s", mibvalue);

		if(flash_read_to_file(data_type, mibvalue)==0)
		{
			printf("flash_read_to_file fail!\n");
		}			
	}	

	if((action>=7) && (action<=9))
	{
		if(argNum >= argc)
			return 0;
			
		sscanf(argv[argNum++], "%s", mibvalue);

		if(flash_write_from_file(data_type, mibvalue)==0)
		{
			printf("flash_write_from_file fail!\n");
		}			
	}	
#endif // #ifdef CAN_RW_FILE			



	return 0;
}


/*
 * Get MIB Table Value
 */
int mib_tbl_get_by_name(char* name, char* arg, void *value)
{
	int i;
	int id = -1;

	for (i=0; mib_chain_record_table[i].id; i++) {
		if ( !strcmp(mib_chain_record_table[i].name, name) )
		{
			id = mib_chain_record_table[i].id;
			break;
		}
	}
	
	if(id != -1)
	{
		unsigned int total = _mib_chain_total(id);
		unsigned int recordNum;

		if ( !strcmp("NUM", arg) )
		{
			sprintf(value,"%s_NUM=%u",name , total);
			return 1;			
		}

		sscanf(arg, "%u", &recordNum);

		if(recordNum < total)
		{
			if(id == MIB_ATM_VC_TBL)
			{
				char *pStr;
				MIB_CE_ATM_VC_Tp pEntry = (MIB_CE_ATM_VC_Tp) _mib_chain_get(MIB_ATM_VC_TBL,recordNum); /* get the specified chain record */
				if(pEntry) {
					char myip[16];
					strncpy(myip, inet_ntoa(*((struct in_addr *)pEntry->ipAddr)), 16);
					myip[15] = '\0';
					sprintf(value,"%s_IFINDEX=0x%x\n%s_VPI=%u\n%s_VCI=%u\n%s_QoS=%s\n%s_PCR=%u\n%s_SCR=%u\n%s_MBS=%u\n%s_CDVT=%u\n"
						,name , pEntry->ifIndex
						,name , pEntry->vpi
						,name , pEntry->vci
						,name , (pEntry->qos == 0)?"ubr":(pEntry->qos == 1)?"cbr":(pEntry->qos == 2)?"rt-vbr":"nrt-vbr"
						,name , pEntry->pcr
						,name , pEntry->scr
						,name , pEntry->mbs
						,name , pEntry->cdvt);
					pStr = (char *)value+strlen((char *)value);
					sprintf(pStr, "%s_ENCAP=%u\n%s_NAPT=%u\n%s_CMODE=%u\n%s_BRMODE=%u\n"
						"%s_PPPNAME=%s\n%s_PPPPWD=%s\n%s_PPPTYPE=%u\n"
						"%s_PPPIDLE=%u\n%s_DHCP=%u\n%s_RIP=%u\n"
						"%s_MYIP=%s\n%s_REMOTEIP=%s\n%s_ENABLE=%u"
						, name, pEntry->encap
						, name, pEntry->napt
						, name, pEntry->cmode
						, name, pEntry->brmode
						, name, pEntry->pppUsername
						, name, pEntry->pppPassword
						, name, pEntry->pppCtype
						, name, pEntry->pppIdleTime
						, name, pEntry->ipDhcp
						, name, pEntry->rip
						, name, myip
						, name, inet_ntoa(*((struct in_addr *)pEntry->remoteIpAddr))
						, name, pEntry->enable);
						
					return 1;
				}
			} 
			else if(id == MIB_PORT_FW_TBL)
			{
				MIB_CE_PORT_FW_Tp pEntry = (MIB_CE_PORT_FW_Tp) _mib_chain_get(MIB_PORT_FW_TBL,recordNum); /* get the specified chain record */
				if(pEntry) {
					sprintf(value,"%s_IP=%s\n%s_FROM=%u\n%s_TO=%u\n%s_PROTO=%u"
						,name , inet_ntoa(*((struct in_addr *)(pEntry->ipAddr)))
						,name , pEntry->fromPort
						,name , pEntry->toPort
						,name , pEntry->protoType);
						
					return 1;
				}
			}
			else if(id == MIB_MAC_FILTER_TBL)
			{
				MIB_CE_MAC_FILTER_Tp pEntry = (MIB_CE_MAC_FILTER_Tp) _mib_chain_get(MIB_MAC_FILTER_TBL,recordNum); /* get the specified chain record */
				if(pEntry) {
					sprintf(value,"%s_ACTION=%u\n%s_MAC=%02x:%02x:%02x:%02x:%02x:%02x"
						,name, pEntry->action
						,name 
						,pEntry->srcMac[0]
						,pEntry->srcMac[1]
						,pEntry->srcMac[2]
						,pEntry->srcMac[3]
						,pEntry->srcMac[4]
						,pEntry->srcMac[5]);
						
					return 1;
				}
			}
			else if(id == MIB_IP_PORT_FILTER_TBL)
			{
				MIB_CE_IP_PORT_FILTER_Tp pEntry = (MIB_CE_IP_PORT_FILTER_Tp) _mib_chain_get(MIB_IP_PORT_FILTER_TBL,recordNum); /* get the specified chain record */
				if(pEntry) {
					sprintf(value,"%s_ACTION=%u\n%s_PROTO=%u\n%s_SIP=%s\n%s_SMASK=%u\n%s_SFROM=%u\n%s_STO=%u\n"
							"%s_DIP=%s\n%s_DMASK=%u\n%s_DFROM=%u\n%s_DTO=%u"
						,name , pEntry->action
						,name , pEntry->protoType
						,name , inet_ntoa(*((struct in_addr *)(pEntry->srcIp)))
						,name , pEntry->smaskbit
						,name , pEntry->srcPortFrom
						,name , pEntry->srcPortTo
						,name , inet_ntoa(*((struct in_addr *)(pEntry->dstIp)))
						,name , pEntry->dmaskbit
						,name , pEntry->dstPortFrom
						,name , pEntry->dstPortTo);
						
					return 1;
				}
			}
#ifdef WLAN_SUPPORT
#ifdef WLAN_ACL
			else if(id == MIB_WLAN_AC_TBL)
			{
				MIB_CE_WLAN_AC_Tp pEntry = (MIB_CE_WLAN_AC_Tp) _mib_chain_get(MIB_WLAN_AC_TBL,recordNum); /* get the specified chain record */
				if(pEntry) {
					sprintf(value,"%s_MAC=%02x:%02x:%02x:%02x:%02x:%02x"
						,name 
						,pEntry->macAddr[0]
						,pEntry->macAddr[1]
						,pEntry->macAddr[2]
						,pEntry->macAddr[3]
						,pEntry->macAddr[4]
						,pEntry->macAddr[5]);
						
					return 1;
				}
			}
#endif
#endif
#ifdef CONFIG_EXT_SWITCH
			else if(id == MIB_SW_PORT_TBL)
			{
				MIB_CE_SW_PORT_Tp pEntry = (MIB_CE_SW_PORT_Tp) _mib_chain_get(MIB_SW_PORT_TBL,recordNum); /* get the specified chain record */
				if(pEntry) {
					sprintf(value,"%s_PVC=%u\n%s_PVID=%u\n%s_EGRESSTAG=%u"
						,name , pEntry->pvcItf
						,name , pEntry->pvid
						,name , pEntry->egressTagAction);
						
					return 1;
				}
			}
			else if(id == MIB_VLAN_TBL)
			{
				MIB_CE_VLAN_Tp pEntry = (MIB_CE_VLAN_Tp) _mib_chain_get(MIB_VLAN_TBL,recordNum); /* get the specified chain record */
				if(pEntry) {
					sprintf(value,"%s_MEMBER=0x%02x\n%s_TAG=%u\n"
						,name , pEntry->member
						,name , pEntry->tag);
						
					return 1;
				}
			}
#ifdef IP_QOS
			else if(id == MIB_IP_QOS_TBL)
			{
				MIB_CE_IP_QOS_Tp pEntry = (MIB_CE_IP_QOS_Tp) _mib_chain_get(MIB_IP_QOS_TBL,recordNum); /* get the specified chain record */
				if(pEntry) {
					sprintf(value,"%s_SRCIP=%s\n%s_SRCMASK=%u\n%s_SRCPORT=%u\n"
						"%s_DSTIP=%s\n%s_DSTMASK=%u\n%s_DSTPORT=%u\n"
						"%s_PROTO=%u\n%s_PHYPORT=%u\n%s_PRIOR=%u\n%s_OUTBOUND=%u\n"
						,name , inet_ntoa(*((struct in_addr *)pEntry->sip))
						,name , pEntry->smaskbit
						,name , pEntry->sPort
						,name , inet_ntoa(*((struct in_addr *)pEntry->dip))
						,name , pEntry->dmaskbit
						,name , pEntry->dPort
						,name , pEntry->protoType
						,name , pEntry->phyPort
						,name , pEntry->prior
						,name , pEntry->outif);
						
					return 1;
				}
			}
#endif	// of IP_QOS
#endif	// of CONFIG_EXT_SWITCH
			else if(id == MIB_PPPOE_SESSION_TBL)	// Jenny
			{
				MIB_CE_PPPOE_SESSION_Tp pEntry = (MIB_CE_PPPOE_SESSION_Tp) _mib_chain_get(MIB_PPPOE_SESSION_TBL, recordNum); /* get the specified chain record */
				if(pEntry) {
					sprintf(value,"%s_IFNO=%u\n%s_VPI=%u\n%s_VCI=%u\n%s_MAC=%02x:%02x:%02x:%02x:%02x:%02x\n%s_SESSIONID=%u\n"
						,name, pEntry->ifNo
						,name, pEntry->vpi
						,name, pEntry->vci
						,name 
						,pEntry->acMac[0]
						,pEntry->acMac[1]
						,pEntry->acMac[2]
						,pEntry->acMac[3]
						,pEntry->acMac[4]
						,pEntry->acMac[5]
						,name, pEntry->sessionId);
						
					return 1;
				}
			}
#ifdef ACCOUNT_CONFIG
			else if (id == MIB_ACCOUNT_CONFIG_TBL)	// Jenny
			{
				MIB_CE_ACCOUNT_CONFIG_Tp pEntry = (MIB_CE_ACCOUNT_CONFIG_Tp) _mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, recordNum); /* get the specified chain record */
				if(pEntry) {
					sprintf(value,"%s_USER=%s\n%s_PASSWD=%s\n%s_PRIV=%u\n"
						,name, pEntry->userName
						,name, pEntry->userPassword
						,name, pEntry->privilege);
					return 1;
				}
			}
#endif
		}
	}

	return 0;
}


/*
 * Get MIB Value
 */
int mib_get_by_name(char* name, void *value)	// get default value
{
	int i, idx;
	int id = -1;
	unsigned char tmp[100];

	for (i=0; mib_table[i].id; i++) {
		if ( !strcmp(mib_table[i].name, name) )
		{
			id = mib_table[i].id;
			idx = i;
			break;
		}
	}

	if(id != -1)
	{
		unsigned char buffer[256+1];
		unsigned int vUInt;
		int	vInt;
		memset(buffer, 0x00 , 256+1);
		if(_mib_get(id, (void *)buffer)!=0)
		{
			switch (mib_table[i].type) {
				case IA_T:
					strcpy(value, inet_ntoa(*((struct in_addr *)buffer)));
					return 1;

				case BYTE_T:
					vUInt = (unsigned int) (*(unsigned char *)buffer);
					sprintf(value,"%u",vUInt);
					return 1;
					
				case WORD_T:
					vUInt = (unsigned int) (*(unsigned short *)buffer);
					sprintf(value,"%u",vUInt);
					return 1;

				case DWORD_T:
					vUInt = (unsigned int) (*(unsigned int *)buffer);
					sprintf(value,"%u",vUInt);
					return 1;

				case INTEGER_T:
					vInt = *((int *)buffer);
					sprintf(value,"%d",vInt);
					return 1;

				case BYTE5_T:
					sprintf(value, "%02x%02x%02x%02x%02x", 
						buffer[0], buffer[1],buffer[2], buffer[3], buffer[4]);
					return 1;

				case BYTE6_T:
					sprintf(value, "%02x%02x%02x%02x%02x%02x", 
						buffer[0], buffer[1],buffer[2], buffer[3], buffer[4], buffer[5]);
					return 1;

				case BYTE13_T:
					sprintf(value, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
						buffer[0], buffer[1],buffer[2], buffer[3], buffer[4], buffer[5],
						buffer[6], buffer[7],buffer[8], buffer[9], buffer[10], buffer[11], buffer[12]);
					return 1;

				case BYTE_ARRAY_T:
					for (i=0; i<mib_table[idx].size; i++)
					{
						sprintf(tmp, "%d", buffer[i]);
						if (i != (mib_table[idx].size-1))
							strcat(tmp, ",");
						strcat(value, tmp);
					}
					return 1;
					
				case STRING_T:
					strcpy(value, buffer);
					return 1;
					
				default:
					return 0;
			}		
		}
	}

	return 0;
}

/*
 * Get all MIB Value
 */
void mib_get_all(void)
{
	int i, total;
	unsigned char buffer[256+1];

	total = 0;
	/* -------------------------------------------
	 *	mib chain
	 * -------------------------------------------*/
	printf("--------------------------------------------------------\n");
	printf("MIB Chain Information\n");
	printf("--------------------------------------------------------\n");
	printf("%4s %-22s %12s %14s\n", "ID", "Name", "size/record", "Max. Entries");
	for (i=0; mib_chain_record_table[i].id; i++) {
		printf("%4d %-22s %12d %14d\n", mib_chain_record_table[i].id,
			mib_chain_record_table[i].name,
			mib_chain_record_table[i].per_record_size,
			mib_chain_record_table[i].table_size);
	}
	/* -------------------------------------------
	 *	mib table
	 * -------------------------------------------*/
	printf("\n--------------------------------------------------------\n");
	printf("MIB Table Information\n");
	printf("--------------------------------------------------------\n");
	for (i=0; mib_table[i].id; i++) {
		memset(buffer, 0x00 , 256+1);
		if(mib_get_by_name(mib_table[i].name, buffer) != 0) {
			printf("%s=%s\n",mib_table[i].name,buffer);
			total += mib_table[i].size;
		}
	}
	printf("--------------------------------------------------------\n");
	printf("Total size: %d bytes\n", total);
}

/*
 * Set MIB Value
 */
int mib_set_by_name(char* name, char *value)	// get default value
{
	int i;
	int id = -1;

	for (i=0; mib_table[i].id; i++) {
		if ( !strcmp(mib_table[i].name, name) )
		{
			id = mib_table[i].id;
			break;
		}
	}

	if(id != -1)
	{
		unsigned char buffer[100];
		struct in_addr ipAddr;
		unsigned char vChar;
		unsigned short vShort;
		unsigned int vUInt;
		int vInt;
	
		switch (mib_table[i].type) {
			case IA_T:
				ipAddr.s_addr = inet_addr(value);
				return _mib_set(id, (void *)&ipAddr);

			case BYTE_T:
				sscanf(value,"%u",&vUInt);
				vChar = (unsigned char) vUInt;
				return _mib_set(id, (void *)&vChar);

			case WORD_T:
				sscanf(value,"%u",&vUInt);
				vShort = (unsigned short) vUInt;
				return _mib_set(id, (void *)&vShort);
				
			case DWORD_T:
				sscanf(value,"%u",&vUInt);
				return _mib_set(id, (void *)&vUInt);

			case INTEGER_T:
				sscanf(value,"%d",&vInt);
				return _mib_set(id, (void *)&vInt);

			case BYTE5_T:
				string_to_hex(value, buffer, 10);
				return _mib_set(id, (void *)&buffer);

			case BYTE6_T:
				string_to_hex(value, buffer, 12);
				return _mib_set(id, (void *)&buffer);

			case BYTE13_T:
				string_to_hex(value, buffer, 26);
				return _mib_set(id, (void *)&buffer);

			case STRING_T:
				return _mib_set(id, (void *)value);

			case BYTE_ARRAY_T:
				string_to_hex(value, buffer, 28);
				return _mib_set(id, (void *)&buffer);

			default:
				return 0;
		}
	}

	return 0;
}

#ifdef CAN_RW_FILE			
int flash_read_to_file(CONFIG_DATA_T data_type, char * filename)
{
	int len;
	unsigned char * buf;
	int fh;
	PARAM_HEADER_Tp pHeader = mib_get_mib_header(data_type);

	fh = open(filename, O_CREAT|O_RDWR);
	if ( fh == -1 )
	{
		printf("open file fail!\n");
		return 0;
	}
			
	len = sizeof(PARAM_HEADER_T) + pHeader->len;	// total len = Header + MIB data
	buf = malloc(len);

	if(buf == 0)
	{
		printf("allocate memory fail!\n");
		goto error;
	}

	switch(data_type)
	{
		case CURRENT_SETTING:
			if(flash_read(buf, CURRENT_SETTING_OFFSET, len) == 0)
			{
				printf("flash read error!\n");
				goto error;
			}
			break;
		case DEFAULT_SETTING:
			if(flash_read(buf, DEFAULT_SETTING_OFFSET, len) == 0)
			{
				printf("flash read error!\n");
				goto error;
			}
			break;
		case HW_SETTING:
			if(flash_read(buf, HW_SETTING_OFFSET, len) == 0)
			{
				printf("flash read error!\n");
				goto error;
			}
			break;
		default:
			goto error;
	}

	lseek(fh, 0, SEEK_SET);

	if ( write(fh, buf, len) != len)
	{
		printf("write file fail!\n");
		goto error;
	}

	close(fh);
	sync();

	free(buf);
	return 1;			
error:
	if(buf) free(buf);
	close(fh);
	return 0;			
}

int flash_write_from_file(CONFIG_DATA_T data_type, char * filename)
{
	int fh;
	int size;
	unsigned char * buf;
	unsigned char * ptr;
	PARAM_HEADER_Tp pFileHeader;
	PARAM_HEADER_Tp pHeader = mib_get_mib_header(data_type);

	fh = open(filename, O_RDONLY);
	if ( fh == -1 )
	{
		printf("open file fail!\n");
		return 0;
	}

	size = lseek(fh, 0, SEEK_END);
	lseek(fh, 0, SEEK_SET);

	buf = malloc(size);

	if(buf == 0)
	{
		printf("allocate memory fail!\n");
		goto error;
	}

	if ( read(fh, buf, size) != size)
	{
		printf("read file fail!\n");
		goto error;
	}

	pFileHeader = (PARAM_HEADER_Tp) buf;

	if ( memcmp(pFileHeader->signature, pHeader->signature, SIGNATURE_LEN) )
	{
		printf("file signature error!\n");
		goto error;
	}

	if(pFileHeader->len != pHeader->len)
	{
		printf("file size error!\n");
		goto error;
	}			

	ptr = &buf[sizeof(PARAM_HEADER_T)];			// point to start of MIB data
	DECODE_DATA(ptr,  pFileHeader->len);

	if(pFileHeader->checksum != CHECKSUM(ptr, pFileHeader->len))
	{
		printf("checksum error!\n");
		goto error;
	}			

	if(_mib_update_from_tbl(data_type, ptr) == 0)
	{
		printf("flash write error!\n");
		goto error;
	}

	free(buf);
	close(fh);
	return 1;
	
error:
	if(buf) free(buf);
	close(fh);
	return 0;	
}
#endif // #ifdef CAN_RW_FILE

static int getdir(char *fullname, char *path, int loop)
{
	char tmpBuf[100], *p, *p1;

	strcpy(tmpBuf, fullname);
	path[0] = '\0';

	p1 = tmpBuf;
	while (1) {
		if ((p=strchr(p1, '/'))) {
			if (--loop == 0) {
				*p = '\0';
				strcpy(path, tmpBuf);
				return 0;
			}
			p1 = ++p;
		}
		else
			break;
	}
	return -1;
}

void unzip_web(char *file, char *dest_dir)
{
	PARAM_HEADER_T header;
	char *buf=NULL;
	char tmpFile[100], tmpFile1[100], tmpBuf[100];
	int fh=0, i, loop, size;
	FILE_ENTRY_Tp pEntry;
	struct stat sbuf;

#ifdef EMBED
	if(file) {
		printf("No file name required !\n");
		return;
	}
		
	if ( flash_read((char *)&header, WEB_PAGE_OFFSET, sizeof(header)) == 0) {
		printf("Read web header failed!\n");
		return;
	}
#else
	if(!file) {
		printf("File name required !\n");
		return;
	}

	if ((fh = open(file, O_RDONLY)) < 0) {
		printf("Can't open file %s\n", file);
		return;
	}
	lseek(fh, 0L, SEEK_SET);
	if (read(fh, &header, sizeof(header)) != sizeof(header)) {
		printf("Read web header failed %s!\n", file);
		close(fh);
		return;
	}

	header.len = DWORD_SWAP(header.len);	
#endif

	if (memcmp(header.signature, WEB_SIGNATURE_TAG, SIGNATURE_LEN)) {
		printf("Invalid web image!\n");
		return;
	}

	buf = malloc(header.len);
	printf("%x %d\n",buf, header.len);
	if (buf == NULL) {
		printf("Allocate buffer failed!\n");
		return;
	}

#ifdef EMBED
	if ( flash_read(buf, WEB_PAGE_OFFSET+sizeof(header), header.len) == 0) {
		printf("Read web image failed!\n");
		goto unzip_web_error;
	}
#else
	if (read(fh, buf, header.len) != header.len) {
		printf("Read web image failed!\n");
		goto unzip_web_error;
	}
	close(fh);
#endif

	if ( CHECKSUM(buf, header.len) != header.checksum) {
		printf("Web image invalid!\n");
		goto unzip_web_error;
	}
	
	// save to a file
	strcpy(tmpFile, "/var/flashweb.gz");
	fh = open(tmpFile, O_RDWR|O_CREAT|O_TRUNC);
	if (fh == -1) {
		printf("Create output file error %s!\n", tmpFile );
		goto unzip_web_error;
	}
	if ( write(fh, buf, header.len) != header.len) {
		printf("write file error %s!\n", tmpFile);
		goto unzip_web_error;
	}
	close(fh);
	sync();

//	free with cause memory bug (mmap.c)
//	free(buf);	

	// decompress file
	sprintf(tmpFile1, "%sXXXXXX", tmpFile);
	mkstemp(tmpFile1);

	sprintf(tmpBuf, "gunzip -c %s > %s", tmpFile, tmpFile1);
	system(tmpBuf);

	unlink(tmpFile);

	if (stat(tmpFile1, &sbuf) != 0) {
		printf("Stat file error %s!\n", tmpFile1);
		goto unzip_web_error;
	}
	if (sbuf.st_size < sizeof(FILE_ENTRY_T) ) {
		sprintf(tmpBuf, "Invalid decompress file size %ld!\n", sbuf.st_size);
		printf(tmpBuf);
		unlink(tmpFile1);
		goto unzip_web_error;
	}

	buf = malloc(sbuf.st_size);
	printf("%x %d\n",buf, sbuf.st_size);
	if (buf == NULL) {
		sprintf(tmpBuf,"Allocate buffer failed %ld!\n", sbuf.st_size);
		printf(tmpBuf);
		goto unzip_web_error;
	}
	if ((fh = open(tmpFile1, O_RDONLY)) < 0) {
		printf("Can't open file %s\n", tmpFile1);
		goto unzip_web_error;
	}
	lseek(fh, 0L, SEEK_SET);
	if ( read(fh, buf, sbuf.st_size) != sbuf.st_size) {
		printf("Read file error %ld!\n", sbuf.st_size);
		goto unzip_web_error;
	}
	close(fh);
	sync();
	unlink(tmpFile1);
	size = sbuf.st_size;
	for (i=0; i<size; ) {
		pEntry = (FILE_ENTRY_Tp)&buf[i];

#ifndef EMBED
		pEntry->size = DWORD_SWAP(pEntry->size);
#endif

		strcpy(tmpFile, dest_dir);
		strcat(tmpFile, pEntry->name);

		loop = 0;
		while (1) {
			if (getdir(tmpFile, tmpBuf, ++loop) < 0)
				break;
			if (tmpBuf[0] && stat(tmpBuf, &sbuf) < 0) { // not exist
 				if ( mkdir(tmpBuf, S_IREAD|S_IWRITE) < 0) {
					printf("Create directory %s failed!\n", tmpBuf);
					goto unzip_web_error;
				}
			}
		}

		fh = open(tmpFile, O_RDWR|O_CREAT|O_TRUNC);
		if (fh == -1) {
			printf("Create output file error %s!\n", tmpFile );
			goto unzip_web_error;
		}

		if ( write(fh, &buf[i+sizeof(FILE_ENTRY_T)], pEntry->size) != pEntry->size ) {
			printf("Write file error %s, len=%ld!\n", tmpFile, pEntry->size);
			goto unzip_web_error;
		}
		close(fh);
		i += (pEntry->size + sizeof(FILE_ENTRY_T));
	}

unzip_web_error:
	if(buf) free(buf);
	return;
}


#ifdef CONFIG_WIFI_SIMPLE_CONFIG
/*
enum { 
	MODE_AP_UNCONFIG=1, 			// AP unconfigured (enrollee)
	MODE_CLIENT_UNCONFIG=2, 		// client unconfigured (enrollee) 
	MODE_CLIENT_CONFIG=3,			// client configured (registrar) 
	MODE_AP_PROXY=4, 			// AP configured (proxy)
	MODE_AP_PROXY_REGISTRAR=5		// AP configured (proxy and registrar)
};
*/

/*
#define WRITE_WSC_PARAM(dst, tmp, str, val) {	\	
	sprintf(tmp, str, val); \
	fprintf(stderr, "%s", tmp); \
	memcpy(dst, tmp, strlen(tmp)); \
	dst += strlen(tmp); \
}
*/
/*
static void convert_bin_to_str(unsigned char *bin, int len, char *out)
{
	int i;
	char tmpbuf[10];

	out[0] = '\0';

	for (i=0; i<len; i++) {
		sprintf(tmpbuf, "%02x", bin[i]);
		strcat(out, tmpbuf);
	}
}

static void convert_hex_to_ascii(unsigned long code, char *out)
{
	*out++ = '0' + ((code / 10000000) % 10);  
	*out++ = '0' + ((code / 1000000) % 10);
	*out++ = '0' + ((code / 100000) % 10);
	*out++ = '0' + ((code / 10000) % 10);
	*out++ = '0' + ((code / 1000) % 10);
	*out++ = '0' + ((code / 100) % 10);
	*out++ = '0' + ((code / 10) % 10);
	*out++ = '0' + ((code / 1) % 10);
	*out = '\0';
}

static int compute_pin_checksum(unsigned long int PIN)
{
	unsigned long int accum = 0;
	int digit;
	
	PIN *= 10;
	accum += 3 * ((PIN / 10000000) % 10); 	
	accum += 1 * ((PIN / 1000000) % 10);
	accum += 3 * ((PIN / 100000) % 10);
	accum += 1 * ((PIN / 10000) % 10); 
	accum += 3 * ((PIN / 1000) % 10); 
	accum += 1 * ((PIN / 100) % 10); 
	accum += 3 * ((PIN / 10) % 10);

	digit = (accum % 10);
	return (10 - digit) % 10;
}
*/
#if 0
static int updateWscConf(char *in, char *out, int genpin)
{
	int fh;
	struct stat status;
	char *buf, *ptr;
	unsigned char intVal2, is_client, is_config, is_registrar, is_wep=0, wep_key_type=0, wep_transmit_key=0;
	unsigned char intVal;
	char tmpbuf[100], tmp1[100];
	int len;
		
	if ( !mib_init()) {
		printf("Initialize AP MIB failed!\n");
		return -1;
	}

	mib_get(MIB_WSC_PIN, (void *)tmpbuf);
	if (genpin || !memcmp(tmpbuf, "\x0\x0\x0\x0\x0\x0\x0\x0", PIN_LEN)) {
		#include <sys/time.h>			
		struct timeval tod;
		unsigned long num;
		
		gettimeofday(&tod , NULL);

		mib_get(MIB_ELAN_MAC_ADDR/*MIB_HW_NIC0_ADDR*/, (void *)&tmp1);			
		tod.tv_sec += tmp1[4]+tmp1[5];		
		srand(tod.tv_sec);
		num = rand() % 10000000;
		num = num*10 + compute_pin_checksum(num);
		convert_hex_to_ascii((unsigned long)num, tmpbuf);

		mib_set(MIB_WSC_PIN, (void *)tmpbuf);
		mib_update(CURRENT_SETTING);		

		printf("Generated PIN = %s\n", tmpbuf);

		if (genpin)
			return 0;
	}

	if (stat(in, &status) < 0) {
		printf("stat() error [%s]!\n", in);
		return -1;
	}

	buf = malloc(status.st_size+2048);
	if (buf == NULL) {
		printf("malloc() error [%d]!\n", (int)status.st_size+2048);
		return -1;		
	}

	ptr = buf;
	mib_get(MIB_WLAN_MODE, (void *)&is_client);
	mib_get(MIB_WSC_CONFIGURED, (void *)&is_config);
	mib_get(MIB_WSC_REGISTRAR_ENABLED, (void *)&is_registrar);	
	if (is_client == CLIENT_MODE) {
		if (is_registrar)
			intVal = MODE_CLIENT_CONFIG;			
		else {
			if (!is_config)
				intVal = MODE_CLIENT_UNCONFIG;
			else
				intVal = MODE_CLIENT_CONFIG;
		}
	}
	else {
		is_registrar = 1; // always true in AP		
		if (!is_config)
			intVal = MODE_AP_UNCONFIG;
		else {
			if (is_registrar)
				intVal = MODE_AP_PROXY_REGISTRAR;
			else
				intVal = MODE_AP_PROXY;
		}		
	}
	WRITE_WSC_PARAM(ptr, tmpbuf, "mode = %d\n", intVal);

	if (is_client)
		intVal = 0;
	else
		mib_get(MIB_WSC_UPNP_ENABLED, (void *)&intVal);
	WRITE_WSC_PARAM(ptr, tmpbuf, "upnp = %d\n", intVal);

	intVal = 0;
	mib_get(MIB_WSC_METHOD, (void *)&intVal);
	//Ethernet(0x2)+Label(0x4)+PushButton(0x80) Bitwise OR
	if (intVal == 1) //Pin+Ethernet
		intVal = (CONFIG_METHOD_ETH | CONFIG_METHOD_PIN);
	else if (intVal == 2) //PBC+Ethernet
		intVal = (CONFIG_METHOD_ETH | CONFIG_METHOD_PBC);
	if (intVal == 3) //Pin+PBC+Ethernet
		intVal = (CONFIG_METHOD_ETH | CONFIG_METHOD_PIN | CONFIG_METHOD_PBC);
	WRITE_WSC_PARAM(ptr, tmpbuf, "config_method = %d\n", intVal);

	mib_get(MIB_WSC_AUTH, (void *)&intVal2);
	WRITE_WSC_PARAM(ptr, tmpbuf, "auth_type = %d\n", intVal2);

	mib_get(MIB_WSC_ENC, (void *)&intVal);
	WRITE_WSC_PARAM(ptr, tmpbuf, "encrypt_type = %d\n", intVal);
	if (intVal == WSC_ENCRYPT_WEP)
		is_wep = 1;

	if (is_client) {
		mib_get(MIB_WLAN_NETWORK_TYPE, (void *)&intVal);
		if (intVal == 0)
			intVal = 1;
		else
			intVal = 2;
	}
	else
		intVal = 1;
	WRITE_WSC_PARAM(ptr, tmpbuf, "connection_type = %d\n", intVal);

	mib_get(MIB_WSC_MANUAL_ENABLED, (void *)&intVal);
	WRITE_WSC_PARAM(ptr, tmpbuf, "manual_config = %d\n", intVal);

	if (is_wep) { // only allow WEP in none-MANUAL mode (configured by external registrar)
		mib_get(MIB_WLAN_ENCRYPT, (void *)&intVal);
		if (intVal != ENCRYPT_WEP) {
			printf("WEP mismatched between WPS and host system\n");
			free(buf);
			return -1;
		}
		mib_get(MIB_WLAN_WEP, (void *)&intVal);
		if (intVal <= WEP_DISABLED || intVal > WEP128) {
			printf("WEP encrypt length error\n");
			free(buf);
			return -1;
		}
		mib_get(MIB_WLAN_WEP_KEY_TYPE, (void *)&wep_key_type);
		mib_get(MIB_WLAN_WEP_DEFAULT_KEY, (void *)&wep_transmit_key);
		wep_transmit_key++;
		WRITE_WSC_PARAM(ptr, tmpbuf, "wep_transmit_key = %d\n", wep_transmit_key);
		if (intVal == WEP64) {
			mib_get(MIB_WLAN_WEP64_KEY1, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 5);
				tmp1[5] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 5, tmp1);
				tmp1[10] = '\0';
			}			
			WRITE_WSC_PARAM(ptr, tmpbuf, "network_key = %s\n", tmp1);

			mib_get(MIB_WLAN_WEP64_KEY2, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 5);
				tmp1[5] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 5, tmp1);
				tmp1[10] = '\0';
			}			
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key2 = %s\n", tmp1);

			mib_get(MIB_WLAN_WEP64_KEY3, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 5);
				tmp1[5] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 5, tmp1);
				tmp1[10] = '\0';
			}			
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key3 = %s\n", tmp1);


			mib_get(MIB_WLAN_WEP64_KEY4, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 5);
				tmp1[5] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 5, tmp1);
				tmp1[10] = '\0';
			}			
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key4 = %s\n", tmp1);
		}
		else {
			mib_get(MIB_WLAN_WEP128_KEY1, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 13);
				tmp1[13] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 13, tmp1);
				tmp1[26] = '\0';
			}
			WRITE_WSC_PARAM(ptr, tmpbuf, "network_key = %s\n", tmp1);


			mib_get(MIB_WLAN_WEP128_KEY2, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 13);
				tmp1[13] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 13, tmp1);
				tmp1[26] = '\0';
			}
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key2 = %s\n", tmp1);

			mib_get(MIB_WLAN_WEP128_KEY3, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 13);
				tmp1[13] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 13, tmp1);
				tmp1[26] = '\0';
			}
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key3 = %s\n", tmp1);

			mib_get(MIB_WLAN_WEP128_KEY4, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 13);
				tmp1[13] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 13, tmp1);
				tmp1[26] = '\0';
			}
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key4 = %s\n", tmp1);
		}
	}
	else {
		mib_get(MIB_WLAN_WPA_PSK, (void *)&tmp1);		
		WRITE_WSC_PARAM(ptr, tmpbuf, "network_key = %s\n", tmp1);
	}
		
	mib_get(MIB_WLAN_SSID, (void *)&tmp1);	
	WRITE_WSC_PARAM(ptr, tmpbuf, "ssid = %s\n", tmp1);	

#if 0	
//	}
//	else {			
		mib_get(MIB_WSC_PSK, (void *)&tmp1);
		WRITE_WSC_PARAM(ptr, tmpbuf, "network_key = %s\n", tmp1);		
		
		mib_get(MIB_WSC_SSID, (void *)&tmp1);
		WRITE_WSC_PARAM(ptr, tmpbuf, "ssid = %s\n", tmp1);
//	}
#endif

	mib_get(MIB_WSC_PIN, (void *)&tmp1);
	WRITE_WSC_PARAM(ptr, tmpbuf, "pin_code = %s\n", tmp1);

	mib_get(MIB_WLAN_CHAN_NUM, (void *)&intVal);
	if (intVal > 14)
		intVal = 2;
	else
		intVal = 1;
	WRITE_WSC_PARAM(ptr, tmpbuf, "rf_band = %d\n", intVal);

/*
	mib_get(MIB_HW_MODEL_NUM, (void *)&tmp1);	
	WRITE_WSC_PARAM(ptr, tmpbuf, "model_num = \"%s\"\n", tmp1);	

	mib_get(MIB_HW_SERIAL_NUM, (void *)&tmp1);	
	WRITE_WSC_PARAM(ptr, tmpbuf, "serial_num = \"%s\"\n", tmp1);	
*/
	mib_get(MIB_SNMP_SYS_NAME, (void *)&tmp1);	
	WRITE_WSC_PARAM(ptr, tmpbuf, "device_name = \"%s\"\n", tmp1);	

	len = (int)(((long)ptr)-((long)buf));
	
	fh = open(in, O_RDONLY);
	if (fh == -1) {
		printf("open() error [%s]!\n", in);
		return -1;
	}

	lseek(fh, 0L, SEEK_SET);
	if (read(fh, ptr, status.st_size) != status.st_size) {		
		printf("read() error [%s]!\n", in);
		return -1;	
	}
	close(fh);

	// search UUID field, replace last 12 char with hw mac address
	ptr = strstr(ptr, "uuid =");
	if (ptr) {
		char tmp2[100];
		mib_get(MIB_ELAN_MAC_ADDR/*MIB_HW_NIC0_ADDR*/, (void *)&tmp1);	
		convert_bin_to_str(tmp1, 6, tmp2);
		memcpy(ptr+27, tmp2, 12);		
	}

	fh = open(out, O_RDWR|O_CREAT|O_TRUNC);
	if (fh == -1) {
		printf("open() error [%s]!\n", out);
		return -1;
	}

	if (write(fh, buf, len+status.st_size) != len+status.st_size ) {
		printf("Write() file error [%s]!\n", out);
		return -1;
	}
	close(fh);
	free(buf);

	return 0;
}

#endif

/*
static void rtrim(char *input) {
	char *pc;

	pc = input + strlen(input) - 1;
	
	while (*pc) {
		if (pc < input)
			return ;
	
		if ((*pc == ' ') || (*pc == 0x0d) || (*pc == 0x0a)) {
			*pc = 0;						
	
		} else {
			return;
		}
		pc--;
		
	}	
}

static void readFileSetParam(char *file) {

	FILE *fp;
	char line[200], *name, *value;


	fp = fopen(file, "r");
	if (fp == NULL) {
		printf("read file [%s] failed!\n", file);
		return;
	}

	mib_init();

	while ( fgets(line, sizeof(line), fp) ) {
		if ((value = strchr(line, '=')) == 0) {
			printf("invalid context: %s\n", line);
			continue;
		}

		name = line;
		*value = 0;
		value++;
		rtrim(value);
		//fprintf(stderr, "(3, %s,%s)",name,value);
		mib_set_by_name(name, value);	
	}
	fclose(fp);
	onoffItf(0);
	if(_mib_update(CURRENT_SETTING) == 0)
		printf("CS Flash error! \n");
	if(_mib_update(HW_SETTING) == 0)
		printf("HS Flash error! \n");
	// open interfaces
	onoffItf(1);

	printf("Commit Setting and Reboot\n");
	system("/bin/reboot");
}
*/
#endif

