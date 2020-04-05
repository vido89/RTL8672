
/* 
 * files.c -- DHCP server file manipulation *
 * Rewrite by Russ Dill <Russ.Dill@asu.edu> July 2001
 */
 
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <netdb.h>

#include "debug.h"
#include "dhcpd.h"
#include "options.h"
#include "leases.h"
#include "files.h"

/* on these functions, make sure you datatype matches */
static int read_ip(char *line, void *arg)
{
	struct in_addr *addr = arg;
	struct hostent *host;
	int retval = 1;

	if (!inet_aton(line, addr)) {
		if ((host = gethostbyname(line))) 
			addr->s_addr = *((unsigned long *) host->h_addr_list[0]);
		else retval = 0;
	}
	return retval;
}


static int read_str(char *line, void *arg)
{
	char **dest = arg;
	
	if (*dest) free(*dest);
	*dest = strdup(line);
	
	return 1;
}


static int read_u32(char *line, void *arg)
{
	u_int32_t *dest = arg;
	char *endptr;
	*dest = strtoul(line, &endptr, 0);
	return endptr[0] == '\0';
}


static int read_yn(char *line, void *arg)
{
	char *dest = arg;
	int retval = 1;

	if (!strcasecmp("yes", line))
		*dest = 1;				
	else if (!strcasecmp("no", line))
		*dest = 0;				
	else retval = 0;
	
	return retval;
}

#ifdef _CWMP_TR111_
//arg's format is like "3561 4 00e0d4 5 0000-0001 6 IGD"
//enterprise-number: 3561(ADSL Forum)
//sub-option code: 4(GatewayManufacturerOUI)
//sub-option code: 5(GatewaySerialNumber)
//sub-option code: 6(GatewayProductClass)
static int read_opt125(char *line, void *arg, int *len)
{
        char *dest = arg;
        int retval = 0;
        int *ent_num=(int*)&dest[0];
        char *pStart, *pEnd, *pNext;
        char *pOptData;
        unsigned short OptDataLen=0;

	//LOG(LOG_ERR,  "<%s:%d>line:%s\n", __FUNCTION__, __LINE__, line  );
        if( line==NULL || arg==NULL || len==NULL ) return retval;
	//LOG(LOG_ERR,  "%d", __LINE__  );
        *len = 0;

        //get enterprise-number;
        pStart=line;
        pEnd=strstr( pStart, " " );
	//LOG(LOG_ERR,  "%d", __LINE__  );
        if( pEnd==NULL ) return retval;
	//LOG(LOG_ERR,  "%d", __LINE__  );
        *pEnd = 0;
        pNext = pEnd+1;
        //LOG(LOG_INFO, "pStart:%s\n", pStart);
        *ent_num = htonl( atoi( pStart ) );
        *len+=4;

        pStart=pNext;
        pOptData=&dest[5];
        while( (pStart!=NULL) && strlen(pStart) )
        {
                unsigned char sub_code;
                unsigned char *sub_data;
                unsigned char sub_len;

                //get sub code
                pEnd=strstr( pStart, " " );
                if( pEnd==NULL ) break;
                *pEnd = 0;
                pNext = pEnd+1;
                //LOG(LOG_INFO, "pStart:%s\n", pStart);
                sub_code = atoi(pStart);
                //LOG(LOG_INFO, "sub_code:%u\n",sub_code );

                //get sub_data
                pStart=pNext;
                pEnd=strstr( pStart, " " );
                if( (pEnd==NULL) && (strlen(pStart)==0) ) break;
                if(pEnd)
                {
                        *pEnd = 0;
                        pNext = pEnd+1;
                }else
                        pNext = NULL;
                //LOG(LOG_INFO, "pStart:%s\n", pStart);
                sub_data = pStart;
                sub_len = strlen(pStart);
                //LOG(LOG_INFO, "sub_len:%u\n", sub_len);

                pOptData[0]=sub_code;
                pOptData[1]=sub_len;
                memcpy( &pOptData[2], sub_data, sub_len );
                pOptData = pOptData+sub_len+2;
                OptDataLen = OptDataLen+sub_len+2;

                pStart=pNext;
        }

        if( *ent_num && OptDataLen)
        {
                dest[4] = OptDataLen;
                *len = *len + 1 + OptDataLen;
                retval = 1;
        }

        return retval;
}
#endif

/* read a dhcp option and add it to opt_list */
static int read_opt(char *line, void *arg)
{
	struct option_set **opt_list = arg;
	char *opt, *val, *endptr;
	struct dhcp_option *option = NULL;
	int retval = 0, length = 0;
	char buffer[255];
	u_int16_t result_u16;
	u_int32_t result_u32;
	int i;

	if (!(opt = strtok(line, " \t="))) return 0;

	for (i = 0; options[i].code; i++)
		if (!strcmp(options[i].name, opt))
			option = &(options[i]);
		
	if (!option) return 0;
	
	do {
#ifdef _CWMP_TR111_
		if( option->code==0x7d )
			val = line + strlen(opt)+1;
		else
#endif
		val = strtok(NULL, ", \t");
		if (val) {
			length = option_lengths[option->flags & TYPE_MASK];
			retval = 0;
			switch (option->flags & TYPE_MASK) {
			case OPTION_IP:
				retval = read_ip(val, buffer);
				break;
			case OPTION_IP_PAIR:
				retval = read_ip(val, buffer);
				if (!(val = strtok(NULL, ", \t/-"))) retval = 0;
				if (retval) retval = read_ip(val, buffer + 4);
				break;
			case OPTION_STRING:
				length = strlen(val);
				if (length > 0) {
					if (length > 254) length = 254;
					memcpy(buffer, val, length);
					retval = 1;
				}
				break;
			case OPTION_BOOLEAN:
				retval = read_yn(val, buffer);
				break;
			case OPTION_U8:
				buffer[0] = strtoul(val, &endptr, 0);
				retval = (endptr[0] == '\0');
				break;
			case OPTION_U16:
				result_u16 = htons(strtoul(val, &endptr, 0));
				memcpy(buffer, &result_u16, 2);
				retval = (endptr[0] == '\0');
				break;
			case OPTION_S16:
				result_u16 = htons(strtol(val, &endptr, 0));
				memcpy(buffer, &result_u16, 2);
				retval = (endptr[0] == '\0');
				break;
			case OPTION_U32:
				result_u32 = htonl(strtoul(val, &endptr, 0));
				memcpy(buffer, &result_u32, 4);
				retval = (endptr[0] == '\0');
				break;
			case OPTION_S32:
				result_u32 = htonl(strtol(val, &endptr, 0));	
				memcpy(buffer, &result_u32, 4);
				retval = (endptr[0] == '\0');
				break;
#ifdef _CWMP_TR111_
			case OPTION_125:
				retval = read_opt125( val, buffer, &length );
				break;
#endif
			default:
				break;
			}
			if (retval) 
				attach_option(opt_list, option, buffer, length);
		};
	} while (val && retval && option->flags & OPTION_LIST);
	return retval;
}


static struct config_keyword keywords[] = {
	/* keyword[14]	handler   variable address		default[20] */
	{"start",	read_ip,  &(server_config.start),	"192.168.1.20"},
	{"end",		read_ip,  &(server_config.end),		"192.168.1.254"},
#ifdef IP_BASED_CLIENT_TYPE
	{"pcstart",	read_ip,  &(server_config.pcstart),	"192.168.1.20"},
	{"pcend",		read_ip,  &(server_config.pcend),		"192.168.1.29"},
	{"cmrstart",	read_ip,  &(server_config.cmrstart),	"192.168.1.30"},
	{"cmrend",		read_ip,  &(server_config.cmrend),		"192.168.1.39"},
	{"stbstart",	read_ip,  &(server_config.stbstart),	"192.168.1.40"},
	{"stbend",		read_ip,  &(server_config.stbend),		"192.168.1.49"},
	{"phnstart",	read_ip,  &(server_config.phnstart),	"192.168.1.50"},
	{"phnend",		read_ip,  &(server_config.phnend),		"192.168.1.59"},
	{"hgwstart",	read_ip,  &(server_config.hgwstart),	"192.168.1.60"},
	{"hgwend",		read_ip,  &(server_config.hgwend),		"192.168.1.69"},
#endif
	{"interface",	read_str, &(server_config.interface),	"eth0"},
	{"option",	read_opt, &(server_config.options),	""},
	{"opt",		read_opt, &(server_config.options),	""},
	{"max_leases",	read_u32, &(server_config.max_leases),	"254"},
	{"remaining",	read_yn,  &(server_config.remaining),	"yes"},
	{"ippt",	    read_yn,  &(server_config.ippt),	    "no"},   // Added by Mason Yu for Half Bridge
	{"ipptlt",	    read_u32, &(server_config.ipptlt),	"600"},      // Addeded by Mason Yu for Half Bridge
	{"auto_time",	read_u32, &(server_config.auto_time),	"7200"},
	{"decline_time",read_u32, &(server_config.decline_time),"3600"},
	{"conflict_time",read_u32,&(server_config.conflict_time),"3600"},
	{"offer_time",	read_u32, &(server_config.offer_time),	"60"},
	{"min_lease",	read_u32, &(server_config.min_lease),	"60"},
//	{"lease_file",	read_str, &(server_config.lease_file),	"/var/lib/misc/udhcpd.leases"},
	{"pidfile",	read_str, &(server_config.pidfile),	"/var/run/udhcpd.pid"},
	{"lease_file",	read_str, &(server_config.lease_file),	"/var/udhcpd/udhcpd.leases"},
//	{"pidfile",	read_str, &(server_config.pidfile),	"/var/udhcpd/udhcpd.pid"},
	{"notify_file", read_str, &(server_config.notify_file),	""},
	{"siaddr",	read_ip,  &(server_config.siaddr),	"0.0.0.0"},
	{"sname",	read_str, &(server_config.sname),	""},
	{"boot_file",	read_str, &(server_config.boot_file),	""},
	{"force_portal_file",	read_str, &(server_config.force_portal_file), "/proc/fp_dhcp"},
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	{"poolname",	read_str, &(server_config.poolname),	""},
	{"cwmpinstnum",	read_u32, &(server_config.cwmpinstnum),	"0"},
	{"poolorder",	read_u32, &(server_config.poolorder),	"0"},
	{"sourceinterface",	read_u32, &(server_config.sourceinterface),	"0"},
	{"vendorclass",	read_str, &(server_config.vendorclass),	""},
	{"vendorclassflag",	read_u32, &(server_config.vendorclassflag),	"0"},
	{"clientid",	read_str, &(server_config.clientid),	""},
	{"clientidflag",	read_u32, &(server_config.clientidflag),	"0"},
	{"userclass",	read_str, &(server_config.userclass),	""},
	{"userclassflag",	read_u32, &(server_config.userclassflag),	"0"},
	{"chaddr",	read_str, &(server_config.chaddr),	""},
	{"chaddrmask",	read_str, &(server_config.chaddrmask),	""},
	{"chaddrflag",	read_u32, &(server_config.chaddrflag),	"0"},
#endif
	/*ADDME: static lease */
	{"",		NULL, 	  NULL,				""}
};

#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
void add_serving_pool(struct server_config_t* pserving_pool )
{
	struct server_config_t* ptmp=p_serverpool_config;
	struct server_config_t* ptmpnext;
	int found=0;

	if(ptmp==NULL){
		p_serverpool_config=pserving_pool;
	}
	else{
		while(ptmp!=NULL){
			if((pserving_pool->poolorder) >= (ptmp->poolorder)){
				if((ptmp->next)!=NULL){
					if((pserving_pool->poolorder) <= (ptmp->next->poolorder))
						found=1;
				}else
					found=1;
			}
			if(found==1){
				ptmpnext=ptmp->next;
				ptmp->next=pserving_pool;
				pserving_pool->next=ptmpnext;
				break;
			}

			ptmp=ptmp->next;
		}
	}
}
#endif
int read_config(char *file)
{
	FILE *in;
	char buffer[80], orig[80], *token, *line;
	int i;
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	char poolname[30+1]={0};
	char poolflag=0;
	struct server_config_t *serving_pool_config=NULL;
#endif


	for (i = 0; strlen(keywords[i].keyword); i++)
		if (strlen(keywords[i].def))
			keywords[i].handler(keywords[i].def, keywords[i].var);

	if (!(in = fopen(file, "r"))) {
		LOG(LOG_ERR, "unable to open config file: %s", file);
		return 0;
	}
	
	while (fgets(buffer, 80, in)) {
		if (strchr(buffer, '\n')) *(strchr(buffer, '\n')) = '\0';
		strncpy(orig, buffer, 80);
		if (strchr(buffer, '#')) *(strchr(buffer, '#')) = '\0';
		token = buffer + strspn(buffer, " \t");
		if (*token == '\0') continue;
		line = token + strcspn(token, " \t=");
		if (*line == '\0') continue;
		*line = '\0';
		line++;
		
		/* eat leading whitespace */
		line = line + strspn(line, " \t=");
		/* eat trailing whitespace */
		for (i = strlen(line); i > 0 && isspace(line[i - 1]); i--);
		line[i] = '\0';
		
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
		if(!strcmp(token,"poolname")){
			strncpy(poolname,line,30);
			printf("\ndhcpd server read configfile:poolname=%s\n",poolname);
		}
		if(!strcmp(token,"poolend")){
			poolflag=0;
			poolname[0]=0;
			if(serving_pool_config!=NULL)
				add_serving_pool(serving_pool_config);
			serving_pool_config=NULL;
			continue;
		}
		if(poolname[0]!=0){
				if(poolflag==0){
					serving_pool_config=malloc(sizeof(struct server_config_t));
					memset(serving_pool_config,0,sizeof(struct server_config_t));
					poolflag=1;
					for (i = 0; strlen(keywords[i].keyword); i++)
						if (strlen(keywords[i].def))
							keywords[i].handler(keywords[i].def,(void*)((int)serving_pool_config+((int)(keywords[i].var)-(int)(&server_config))));
				}
				if(poolflag==1){
					if(serving_pool_config==NULL)
						continue;
					for (i = 0; strlen(keywords[i].keyword); i++){
						if (!strcasecmp(token, keywords[i].keyword)){
							if (!keywords[i].handler(line, (void*)((int)serving_pool_config+((int)(keywords[i].var)-(int)(&server_config))))) {
								LOG(LOG_ERR, "unable to parse '%s'", orig);
								/* reset back to the default value */
								keywords[i].handler(keywords[i].def,(void*)((int)serving_pool_config+((int)(keywords[i].var)-(int)(&server_config))));
							}
						}
					}
				}
		}
#else
		for (i = 0; strlen(keywords[i].keyword); i++)
			if (!strcasecmp(token, keywords[i].keyword))
				if (!keywords[i].handler(line, keywords[i].var)) {
					LOG(LOG_ERR, "unable to parse '%s'", orig);
					/* reset back to the default value */
					keywords[i].handler(keywords[i].def, keywords[i].var);
				}
#endif
	}
	fclose(in);
	return 1;
}


void write_leases(void)
{
	FILE *fp;
	unsigned int i;
	char buf[255];
	time_t curr = time(0);
	unsigned long lease_time;
	
	if (!(fp = fopen(server_config.lease_file, "w"))) {
		LOG(LOG_ERR, "Unable to open %s for writing", server_config.lease_file);
		return;
	}
	
	for (i = 0; i < server_config.max_leases; i++) {
		if (leases[i].yiaddr != 0) {
			if (server_config.remaining) {
				if (lease_expired(&(leases[i])))
					lease_time = 0;
				else lease_time = leases[i].expires - curr;
			} else lease_time = leases[i].expires;
			if(leases[i].expires == 0xffffffff) //star add: for infinity web display
				lease_time = 0xffffffff;
			lease_time = htonl(lease_time);
			fwrite(leases[i].chaddr, 16, 1, fp);
			fwrite(&(leases[i].yiaddr), 4, 1, fp);
			fwrite(&lease_time, 4, 1, fp);
		}
	}
	fclose(fp);
	
	if (server_config.notify_file) {
		sprintf(buf, "%s %s", server_config.notify_file, server_config.lease_file);
		system(buf);
	}
}


void read_leases(char *file)
{
	FILE *fp;
	unsigned int i = 0;
	struct dhcpOfferedAddr lease;
	
	if (!(fp = fopen(file, "r"))) {
		LOG(LOG_ERR, "Unable to open %s for reading", file);
		return;
	}
	
	while (i < server_config.max_leases && (fread(&lease, sizeof lease, 1, fp) == 1)) {
		/* ADDME: is it a static lease */
		if (lease.yiaddr >= server_config.start && lease.yiaddr <= server_config.end) {
			lease.expires = ntohl(lease.expires);
			if ((!server_config.remaining)&&lease.expires!=0xffffffff) lease.expires -= time(0);
			if (!(add_lease(lease.chaddr, lease.yiaddr, lease.expires))) {
				LOG(LOG_WARNING, "Too many leases while loading %s\n", file);
				break;
			}				
			i++;
		}
	}
	DEBUG(LOG_INFO, "Read %d leases", i);
	fclose(fp);
}

#if 0
void update_force_portal()
{
	FILE *fp;
	unsigned long i;

	if (server_config.force_portal_file)
    {
    	if (NULL == (fp = fopen(server_config.force_portal_file, "w")))
        {
    		/* LOG(LOG_ERR, "Unable to open %s for writing", server_config.force_portal_file); */
    		return;
    	}

    	for (i = 0; i < server_config.max_leases; i++)
        {
    		if ((leases[i].yiaddr != 0)
                && (!lease_expired(&(leases[i]))))
            {
                fprintf(fp, "device: %u: %02x-%02x-%02x-%02x-%02x-%02x\n", 
                        leases[i].stClientInfo.ulDevice - 1, 
                        leases[i].chaddr[0], leases[i].chaddr[1], leases[i].chaddr[2], 
                        leases[i].chaddr[3], leases[i].chaddr[4], leases[i].chaddr[5]);
    		}
    	}
        
        fclose(fp);
	}

    return;
}
#else
void update_force_portal(struct dhcpOfferedAddr *pstLease)
{
	FILE *fp;

	if (server_config.force_portal_file)
    {
    	if (NULL == (fp = fopen(server_config.force_portal_file, "w")))
        {
    		/* LOG(LOG_ERR, "Unable to open %s for writing", server_config.force_portal_file); */
    		return;
    	}

        fprintf(fp, "device: %u: %02x-%02x-%02x-%02x-%02x-%02x\n", 
                pstLease->stClientInfo.ulDevice - 1, 
                pstLease->chaddr[0], pstLease->chaddr[1], pstLease->chaddr[2], 
                pstLease->chaddr[3], pstLease->chaddr[4], pstLease->chaddr[5]);
        
        fclose(fp);
	}

    return;
}
#endif

void remove_force_portal()
{
	FILE *fp;
    
	if (server_config.force_portal_file)
    {
        /* Empte force portal file */
    	if (NULL == (fp = fopen(server_config.force_portal_file, "w")))
        {
    		/* LOG(LOG_ERR, "Unable to open %s for writing", server_config.force_portal_file); */
    	}
        else
        {
            fclose(fp);
        }
        
        /* remove(server_config.force_portal_file); */
	}

    return;
}

#ifdef CONFIG_CTC_E8_CLIENT_LIMIT
void update_client_limit(struct dhcpOfferedAddr *pstLease)
{
	FILE *fp;
    struct dhcp_ctc_client_info *pstClientInfo = (struct dhcp_ctc_client_info *)(pstLease->stClientInfo.pvDHCPClientData);

    if (NULL == pstClientInfo)
    {
        return;
    }
    
	if (NULL == (fp = fopen(CLIENT_LIMIT_DEV_FILE, "w")))
    {
		/* LOG(LOG_ERR, "Unable to open %s for writing", CLIENT_LIMIT_DEV_FILE); */
		return;
	}

    fprintf(fp, "device: %u: %02x-%02x-%02x-%02x-%02x-%02x\n", 
            pstClientInfo->iCategory, 
            pstLease->chaddr[0], pstLease->chaddr[1], pstLease->chaddr[2], 
            pstLease->chaddr[3], pstLease->chaddr[4], pstLease->chaddr[5]);
    
    fclose(fp);

    return;
}
#endif
