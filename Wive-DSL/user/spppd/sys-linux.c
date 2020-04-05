
#include "if_sppp.h"
#include "pppoe.h"
#include "pppoa.h"
#include "signal.h"
#include <sys/file.h>
#include <rtk/utility.h>
#include <config/autoconf.h>
#include <rtk/sysconfig.h>
#ifdef CONFIG_MIDDLEWARE
#include <rtk/midwaredefs.h>
#endif
#define N_SPPP	8

// Added by Mason Yu. Access internet fail.
static unsigned long GetIP=0;

extern int in_pppoe_disc;
extern unsigned int auth_fail_counter[N_SPPP];
int ppp_dev_fd[N_SPPP];

fd_set in_fds;		/* set of fds that wait_input waits for */
int max_in_fd;		/* highest fd set in in_fds */

struct sppp *dod_sp = NULL;
//int dod_flag = 1;

#define _PATH_PROCNET_DEV               "/proc/net/dev"

/* We can get an EIO error on an ioctl if the modem has hung up */
#define ok_error(num) ((num)==EIO)

/*
 * SET_SA_FAMILY - set the sa_family field of a struct sockaddr,
 * if it exists.
 */

#define SET_SA_FAMILY(addr, family)			\
    memset ((char *) &(addr), '\0', sizeof(addr));	\
    addr.sa_family = (family);

#define TIMEOUT(fun, arg1, arg2, handle) 	timeout(fun,arg1,arg2, &handle)



/********************************************************************
 *
 * sifup - Config the interface up and enable IP packets to pass.
 */

int sifup (struct sppp *sp)
{
    struct ifreq ifr;

    memset (&ifr, '\0', sizeof (ifr));
    strlcpy(ifr.ifr_name, sp->if_name, sizeof (ifr.ifr_name));
    if (ioctl(sp->sock_fd, SIOCGIFFLAGS, (caddr_t) &ifr) < 0) {
		if (! ok_error (errno))
	    	printf("ioctl (SIOCGIFFLAGS): %m(%d)", errno);
		return 0;
    }

    ifr.ifr_flags |= (IFF_UP | IFF_POINTOPOINT);
    if (ioctl(sp->sock_fd, SIOCSIFFLAGS, (caddr_t) &ifr) < 0) {
		if (! ok_error (errno))
	    	printf("ioctl(SIOCSIFFLAGS): %m(%d)", errno);
		return 0;
    }
    return 1;
}

/********************************************************************
 *
 * sifdown - Config the interface down and disable IP.
 */

int sifdown (struct sppp *sp)
{
    struct ifreq ifr;

    memset (&ifr, '\0', sizeof (ifr));
    strlcpy(ifr.ifr_name, sp->if_name, sizeof (ifr.ifr_name));
    if (ioctl(sp->sock_fd, SIOCGIFFLAGS, (caddr_t) &ifr) < 0) {
	if (! ok_error (errno))
	    printf("ioctl (SIOCGIFFLAGS): %m(%d)", errno);
	return 0;
    }

    ifr.ifr_flags &= ~IFF_UP;
    ifr.ifr_flags |= IFF_POINTOPOINT;
    if (ioctl(sp->sock_fd, SIOCSIFFLAGS, (caddr_t) &ifr) < 0) {
	if (! ok_error (errno))
	    printf("ioctl(SIOCSIFFLAGS): %m(%d)", errno);
	return 0;
    }
    return 1;
}

/********************************************************************
 *
 * sifaddr - Config the interface IP addresses and netmask.
 */

int sifaddr (struct sppp *sp)
{
    struct ifreq   ifr; 
    struct rtentry rt;
    u_long	net_mask;
    // Added by Mason Yu
    u_long 	alais_ip;
#ifdef AUTO_PPPOE_ROUTE
    unsigned char dgw;
#endif     
    
    memset (&ifr, '\0', sizeof (ifr));
    memset (&rt,  '\0', sizeof (rt));
    
    SET_SA_FAMILY (ifr.ifr_addr,    AF_INET); 
    SET_SA_FAMILY (ifr.ifr_dstaddr, AF_INET); 
    SET_SA_FAMILY (ifr.ifr_netmask, AF_INET); 

    strlcpy (ifr.ifr_name, sp->if_name, sizeof (ifr.ifr_name));
    // Added by Mason Yu for Half Bridge
    // Write gloable variables
    if (sp->ippt_flag ==1) {
       FILE *fp;      
       
       fp = fopen("/tmp/PPPHalfBridge", "w+");
       if (fp) {
       	  fwrite( &sp->ipcp.myipaddr, 4, 1, fp);
       	   
       	  // Modified by Mason Yu
       	  alais_ip = sp->ipcp.myipaddr + 1 ;
       	  //*(((char *)(&alais_ip)) + 3) = 254;       	  
       	  fwrite( &alais_ip, 4, 1, fp);
       	  
       	  // Added by Mason Yu. Access internet fail.
       	  fwrite( &sp->ipcp.primary_dns, 4, 1, fp);    	  
       	  fclose(fp);
       }	
    }
    
	syslog(LOG_INFO, "spppd: %s: IP address %s\n", sp->if_name, inet_ntoa(sp->ipcp.myipaddr));
	syslog(LOG_INFO, "spppd: %s: gateway %s\n", sp->if_name, inet_ntoa(sp->ipcp.hisipaddr));
	
    
/*
 *  Set our IP address 
 */
    // Modified by Mason Yu for Half Bridge     
    if (sp->ippt_flag )  {  
//        ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr.s_addr = 0x0a010101;                     
        ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr.s_addr = 0x0a000001;	// Jenny, for IP passthrough determination temporarily
    }else  
         ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr.s_addr = sp->ipcp.myipaddr;
    
    if (ioctl(sp->sock_fd, SIOCSIFADDR, (caddr_t) &ifr) < 0) {
		if (errno != EEXIST) {
		    if (! ok_error (errno))
			printf("ioctl(SIOCSIFADDR): %m(%d)", errno);
		}
    	else {
	    	printf("ioctl(SIOCSIFADDR): Address already exists");
		}
        return (0);
    }
/*
 *  Set the gateway address
 */
    // Modified by Mason Yu for Half Bridge
    if (sp->ippt_flag){           
//         ((struct sockaddr_in *) &ifr.ifr_dstaddr)->sin_addr.s_addr = 0x0a010101;
        ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr.s_addr = 0x0a000002;	// Jenny, for IP passthrough determination temporarily
    } else  {         
         ((struct sockaddr_in *) &ifr.ifr_dstaddr)->sin_addr.s_addr = sp->ipcp.hisipaddr;
    }
    
    if (ioctl(sp->sock_fd, SIOCSIFDSTADDR, (caddr_t) &ifr) < 0) {
	if (! ok_error (errno))
	    printf("ioctl(SIOCSIFDSTADDR): %m(%d)", errno); 
	return (0);
    } 
/*
 *  Set the netmask.
 *  For recent kernels, force the netmask to 255.255.255.255.
 */
	/* for kernel version > 2.1.16 */
	net_mask = ~0L;
	/* */		
    if (net_mask != 0) {
	((struct sockaddr_in *) &ifr.ifr_netmask)->sin_addr.s_addr = net_mask;
	if (ioctl(sp->sock_fd, SIOCSIFNETMASK, (caddr_t) &ifr) < 0) {
	    if (! ok_error (errno))
		printf("ioctl(SIOCSIFNETMASK): %m(%d)", errno); 
	    return (0);
	} 
    }
    
    	
	// Modifieded by Mason Yu for PPP Half Bridge		
	// Half Bridge rule
	if (sp->ippt_flag ==1) {                         
             
               int hisip[4];               
               char br0_alias_cmd[60];               
               unsigned long net, mask;
      	     
               // Modified by Mason Yu               
               hisip[0] = (alais_ip >> 24 ) & 0xff;
               hisip[1] = (alais_ip >> 16 ) & 0xff;
               hisip[2] = (alais_ip >> 8 ) & 0xff;
               hisip[3] = (alais_ip ) & 0xff;
               	
               /* Set br0 alias br0:1  */
               sprintf(br0_alias_cmd, "ifconfig %s %d.%d.%d.%d", (char*)LAN_IPPT, hisip[0], hisip[1], hisip[2], hisip[3]);
               system(br0_alias_cmd);
               
               	/* Add route for Public IP */
               	if (IN_CLASSA(alais_ip)) {
			net = alais_ip & IN_CLASSA_NET;
			mask = IN_CLASSA_NET;
		}
		else if (IN_CLASSB(alais_ip)) {			
			net = alais_ip & IN_CLASSB_NET;
			mask = IN_CLASSB_NET;
		}
		else {
			net = alais_ip & IN_CLASSC_NET;
			mask = IN_CLASSC_NET;  
		}
		            
               	sprintf(br0_alias_cmd, "route del -net %d.%d.%d.%d netmask %d.%d.%d.%d", 
               		*((unsigned char *)(&net)), *((unsigned char *)(&net)+1), *((unsigned char *)(&net)+2), *((unsigned char *)(&net)+3),
               		*((unsigned char *)(&mask)), *((unsigned char *)(&mask)+1), *((unsigned char *)(&mask)+2), *((unsigned char *)(&mask)+3)
               		 );               
               	system(br0_alias_cmd);
               	
               	sprintf(br0_alias_cmd, "route add -host %d.%d.%d.%d dev br0", 
               		(int)(sp->ipcp.myipaddr>>24)&0xff, (int)(sp->ipcp.myipaddr>>16)&0xff, (int)(sp->ipcp.myipaddr>>8)&0xff, (int)(sp->ipcp.myipaddr)&0xff);
               	system(br0_alias_cmd);
	}
    	
	/*
 	*  Add default route
 	*/
#ifdef AUTO_PPPOE_ROUTE
	mib_get( MIB_ADSL_WAN_DGW_ITF, (void *)&dgw);	// Jenny, check default gateway
//	if((dgw == DGW_AUTO && sp->over == SPPP_PPPOE) || sp->dgw)	// Jenny, if set to auto(0xef)
	if(dgw == DGW_AUTO || sp->dgw)	// Jenny, if set to auto(0xef)
#else
	if(sp->dgw)
#endif
	{		
		  	
          if (sp->ippt_flag ==1) {
          	char default_route_cmd[40];              	
		/* Set dafault route */   
#ifdef DEFAULT_GATEWAY_V2
			if (ifExistedDGW() == 1) {	// Jenny, delete existed default gateway first
				sprintf(default_route_cmd, "route del default");	
				system(default_route_cmd);
			}
#endif
               	sprintf(default_route_cmd, "route add default ppp%d", sp->if_unit);    
               	system(default_route_cmd);        	
          }
	  else if (sp->ippt_flag==2){
		      char default_route_cmd[40];         
                       /* Set dafault route */   
#ifdef DEFAULT_GATEWAY_V2
			if (ifExistedDGW() == 1) {  // Jenny, delete existed default gateway first
				sprintf(default_route_cmd, "route del default");    
				system(default_route_cmd);
			}
#endif
                    sprintf(default_route_cmd, "route add default ppp%d", sp->if_unit);    
                    system(default_route_cmd);        

	  }
	  else {
		       // Normal rule
			char default_route_cmd[40];		   
			/* Set dafault route */   
#ifdef DEFAULT_GATEWAY_V2
			if (ifExistedDGW() == 1) {	// Jenny, delete existed default gateway first
				sprintf(default_route_cmd, "route del default");	
				system(default_route_cmd);
			}
#endif
			sprintf(default_route_cmd, "route add default ppp%d", sp->if_unit);	
			system(default_route_cmd); 	   
#if 0
		       rt.rt_dev = sp->if_name;
              
		       SET_SA_FAMILY (rt.rt_dst,     AF_INET);
    	       SET_SA_FAMILY (rt.rt_genmask, AF_INET);
		       SET_SA_FAMILY (rt.rt_gateway, AF_INET);
		       ((struct sockaddr_in *) &rt.rt_dst)->sin_addr.s_addr  = 0L;
    	       ((struct sockaddr_in *) &rt.rt_genmask)->sin_addr.s_addr = 0L;
		       ((struct sockaddr_in *) &rt.rt_gateway)->sin_addr.s_addr = 0L;
		       ioctl(sp->sock_fd, SIOCDELRT, &rt);
              
		       SET_SA_FAMILY (rt.rt_dst,     AF_INET);
    	       SET_SA_FAMILY (rt.rt_genmask, AF_INET);
		       SET_SA_FAMILY (rt.rt_gateway, AF_INET);
		       ((struct sockaddr_in *) &rt.rt_dst)->sin_addr.s_addr     = 0L;
    	       ((struct sockaddr_in *) &rt.rt_genmask)->sin_addr.s_addr = 0L;
		       ((struct sockaddr_in *) &rt.rt_gateway)->sin_addr.s_addr = sp->ipcp.hisipaddr;
		       rt.rt_flags = RTF_UP | RTF_GATEWAY;
		       if (ioctl(sp->sock_fd, SIOCADDRT, &rt) < 0) {
	           	if (! ok_error (errno))
		       		printf("ioctl(SIOCADDRT) device route: %m(%d)", errno);
	           	return (0);
		       }
#endif
	  }
	  syslog(LOG_INFO, "spppd: route add default %s", sp->if_name);    
    }
    
    // Added by Mason Yu for HALF Bridge
    // Modified IP Packet filter by iptables command    
    if (sp->ippt_flag) {
    	int myip[4];               
        //char del_cmd[60];
        char del_cmd[256];
        //char append_cmd[80];        
        char append_cmd[256];        
        
        // Added by Mason Yu. Access internet fail.    
	if ( sp->previous_ippt_myip != 0 ) {
		/* Delete IP Packet rule */   
        	//sprintf(del_cmd, "iptables -t nat -D POSTROUTING 1");
        	sprintf(del_cmd, "iptables -t nat -D POSTROUTING -o ppp%d -j SNAT --to-source %d.%d.%d.%d", sp->if_unit, 
        		(sp->previous_ippt_myip >> 24)&0xff, (sp->previous_ippt_myip >> 16)&0xff, (sp->previous_ippt_myip >> 8)&0xff, sp->previous_ippt_myip&0xff );        
        	system(del_cmd);
        } else {
        	/* Delete IP packet rule(MASQUERADE)  */
        	sprintf(del_cmd, "iptables -t nat -D POSTROUTING -o ppp%d -j MASQUERADE", sp->if_unit);
        	system(del_cmd);
        }
               	
        myip[0] = (sp->ipcp.myipaddr >> 24 ) & 0xff;
        myip[1] = (sp->ipcp.myipaddr >> 16 ) & 0xff;
        myip[2] = (sp->ipcp.myipaddr >> 8 ) & 0xff;
        myip[3] = (sp->ipcp.myipaddr ) & 0xff;  
        
        // If the src IP is public IP, the packet need not to do SNAT
        //sprintf(append_cmd, "iptables -t nat -I POSTROUTING 1 -o ppp%d -s %d.%d.%d.%d -j RETURN", sp->if_unit, myip[0], myip[1], myip[2], myip[3]);    
        //system(append_cmd);
        
        /* Append IP Packet rule */   
        sprintf(append_cmd, "iptables -t nat -I POSTROUTING 1 -o ppp%d -j SNAT --to-source %d.%d.%d.%d", sp->if_unit, myip[0], myip[1], myip[2], myip[3]);    
        system(append_cmd); 
        	
        sp->previous_ippt_myip = sp->ipcp.myipaddr;
    }
	
    
    return 1;
}

/********************************************************************
 *
 * cifaddr - Clear the interface IP addresses, and delete routes
 * through the interface if possible.
 */

int cifaddr (struct sppp *sp)
{
    struct rtentry rt;

	/* for kernel version < 2.1.16 */
	#if 0
    {
/*
 *  Delete the route through the device
 */
	memset (&rt, '\0', sizeof (rt));

	SET_SA_FAMILY (rt.rt_dst,     AF_INET);
	SET_SA_FAMILY (rt.rt_gateway, AF_INET);
	rt.rt_dev = sp->if_name;

	((struct sockaddr_in *) &rt.rt_gateway)->sin_addr.s_addr = 0;
	((struct sockaddr_in *) &rt.rt_dst)->sin_addr.s_addr     = sp->ipcp.hisipaddr;
	rt.rt_flags = RTF_UP | RTF_HOST;

	/* for kernel version > 2.1.0 */
	{
	    SET_SA_FAMILY (rt.rt_genmask, AF_INET);
	    ((struct sockaddr_in *) &rt.rt_genmask)->sin_addr.s_addr = -1L;
	}

	if (ioctl(sp->sock_fd, SIOCDELRT, &rt) < 0 && errno != ESRCH) {
	    if (! ok_error (errno))
			printf("ioctl(SIOCDELRT) device route: %m(%d)", errno);
	    return (0);
	}
    }
    #endif
    
    return 1;
}

/*
 * Set the MTU and other parameters for the ppp device
 */
int sifmtu(struct sppp *sp)
{
    struct ifreq   ifr;
    
    memset (&ifr, '\0', sizeof (ifr));
    strlcpy(ifr.ifr_name, sp->if_name, sizeof (ifr.ifr_name));
    ifr.ifr_mtu = (sp->lcp.mru <= sp->lcp.their_mru ? sp->lcp.mru : sp->lcp.their_mru);
	
    if (ioctl(sp->sock_fd, SIOCSIFMTU, (caddr_t) &ifr) < 0)
		printf("ioctl(SIOCSIFMTU): %m(%d)", errno);
	return 1;
}

struct user_net_device_stats {
    unsigned long long rx_packets;	/* total packets received       */
    unsigned long long tx_packets;	/* total packets transmitted    */
    unsigned long long rx_bytes;	/* total bytes received         */
    unsigned long long tx_bytes;	/* total bytes transmitted      */
    unsigned long rx_errors;	/* bad packets received         */
    unsigned long tx_errors;	/* packet transmit problems     */
    unsigned long rx_dropped;	/* no space in linux buffers    */
    unsigned long tx_dropped;	/* no space available in linux  */
    unsigned long rx_multicast;	/* multicast packets received   */
    unsigned long rx_compressed;
    unsigned long tx_compressed;
    unsigned long collisions;

    /* detailed rx_errors: */
    unsigned long rx_length_errors;
    unsigned long rx_over_errors;	/* receiver ring buff overflow  */
    unsigned long rx_crc_errors;	/* recved pkt with crc error    */
    unsigned long rx_frame_errors;	/* recv'd frame alignment error */
    unsigned long rx_fifo_errors;	/* recv'r fifo overrun          */
    unsigned long rx_missed_errors;	/* receiver missed packet     */
    /* detailed tx_errors */
    unsigned long tx_aborted_errors;
    unsigned long tx_carrier_errors;
    unsigned long tx_fifo_errors;
    unsigned long tx_heartbeat_errors;
    unsigned long tx_window_errors;
};

static char *get_name(char *name, char *p)
{
    while (isspace(*p))
	p++;
    while (*p) {
	if (isspace(*p))
	    break;
	if (*p == ':') {	/* could be an alias */
	    char *dot = p, *dotname = name;
	    *name++ = *p++;
	    while (isdigit(*p))
		*name++ = *p++;
	    if (*p != ':') {	/* it wasn't, backup */
		p = dot;
		name = dotname;
	    }
	    if (*p == '\0')
		return NULL;
	    p++;
	    break;
	}
	*name++ = *p++;
    }
    *name++ = '\0';
    return p;
}


int chk_ifxmtrcv(struct sppp *sp)
{
int retval = 0;
struct user_net_device_stats	if_stats;
char buf[512];
FILE *fh = fopen(_PATH_PROCNET_DEV, "r");
	if(!fh)
		return 0;
    fgets(buf, sizeof buf, fh);	/* eat line */
    fgets(buf, sizeof buf, fh);
	
    while (fgets(buf, sizeof buf, fh)) {
	char *s, name[IFNAMSIZ];
		s = get_name(name, buf);    
		if (strcmp(sp->if_name,name))
			continue;
		sscanf(s,
			"%Lu %Lu %lu %lu %lu %lu %lu %lu %Lu %Lu %lu %lu %lu %lu %lu %lu",
	       &if_stats.rx_bytes,
	       &if_stats.rx_packets,
	       &if_stats.rx_errors,
	       &if_stats.rx_dropped,
	       &if_stats.rx_fifo_errors,
	       &if_stats.rx_frame_errors,
	       &if_stats.rx_compressed,
	       &if_stats.rx_multicast,

	       &if_stats.tx_bytes,
	       &if_stats.tx_packets,
	       &if_stats.tx_errors,
	       &if_stats.tx_dropped,
	       &if_stats.tx_fifo_errors,
	       &if_stats.collisions,
	       &if_stats.tx_carrier_errors,
	       &if_stats.tx_compressed);

		if(sp->pp_last_sent == if_stats.tx_packets) {
			// Mason Yu
			//sp->idletime += 10;
			sp->idletime += 1;  // 1 means 20 seconds
			
			if(sp->idletime >= sp->timeout) {
				sp->idletime = 0;
				retval = -1;
			}
		} 
		else {
			sp->idletime = 0;
		}
		sp->pp_last_sent = if_stats.tx_packets;
		sp->pp_last_recv = if_stats.rx_packets;
	       
		fclose(fh);
			
		return retval;
    }
	
    if (ferror(fh)) {
		perror(_PATH_PROCNET_DEV);
    }

	fclose(fh);	
	return retval;
}

#ifdef CONFIG_MIDDLEWARE
int getMgtPVCIndex()
{
	MIB_CE_ATM_VC_T Entry,*pEntry;
	int ret=-1;
	unsigned int i,num;

	pEntry = &Entry;
	
	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ) )
			continue;

		if(pEntry->cmode != ADSL_PPPoE){
			continue;
		}

		if(pEntry->ServiceList & X_CT_SRV_TR069)
		{
			ret = (pEntry->ifIndex) >> 4;
			break;
		}
	}	

	return ret;
}
#if 0
static int sendMsg2MidProcess(struct mwMsg * pMsg)
{
	int spid;
	FILE * spidfile;
	int msgid;
	
	msgid = msgget((key_t)1357,  0666);
	if(msgid <= 0){
		fprintf(stdout,"get cwmp msgqueue error!\n");
		return -1;
	}

	/* get midware interface pid*/
	if ((spidfile = fopen(CWMP_MIDPROC_RUNFILE, "r"))) {
		fscanf(spidfile, "%d\n", &spid);
		fclose(spidfile);
	}else{
		fprintf(stdout,"midware interface pidfile not exists in %s\n",__FUNCTION__);
		return -1;
	}

	pMsg->msg_type = spid;
	pMsg->msg_datatype = MSG_MIDWARE;
	if(msgsnd(msgid, (void *)pMsg, MW_MSG_SIZE, 0) < 0){
		fprintf(stdout,"send message to midwareintf error!\n");
		return -1;
	}
	
	return 0;
}

void sendInformKeyParaMsg2MidProcess()
{
	struct mwMsg sendMsg;
	char * sendBuf = sendMsg.msg_data;
	
	*(sendBuf) = OP_informKeyPara;
	sendMsg2MidProcess(&sendMsg);
}
#endif
#endif

int ppp_if_up(struct sppp *sp)
{
	   FILE *fp;
	  char buff[32];
	if(!sifup(sp)) {
		sppp_last_connection_error(sp->if_unit, ERROR_NOT_ENABLED_FOR_INTERNET);
		return 0;
	}
	if(!sifaddr(sp)) {
		sppp_last_connection_error(sp->if_unit, ERROR_IP_CONFIGURATION);
		return 0;
	}
	if(!sifmtu(sp))
		return 0;

	/* change DNS */
	if(sp->ipcp.primary_dns || sp->ipcp.second_dns) {
#ifdef CONFIG_TR069_DNS_ISOLATION
               if(sp->isTr069_interface)
	      {
	             set_tr069_dns_isolation(1);
		      fp = fopen("/var/resolv_tr069.conf", "w");	      
	      }	       
                 
	       else
#endif		   	
	      {	
		       fp = fopen("/var/ppp/resolv.conf", "w");
	      } 	
		
		if(sp->ipcp.primary_dns) {
			sprintf(buff, "nameserver %s\n", inet_ntoa(sp->ipcp.primary_dns));
			fputs(buff, fp);
			syslog(LOG_INFO, "spppd: %s: Primary DNS %s", sp->if_name, inet_ntoa(sp->ipcp.primary_dns));
		}
		if(sp->ipcp.second_dns) {
			sprintf(buff, "nameserver %s\n", inet_ntoa(sp->ipcp.second_dns));
			fputs(buff, fp);
			syslog(LOG_INFO, "spppd: %s: Secondary DNS %s", sp->if_name, inet_ntoa(sp->ipcp.second_dns));
		}
		fclose(fp);
#ifdef CONFIG_TR069_DNS_ISOLATION
//set tr069 dns 
              //set_tr069_dns_isolation(1);
              set_tr069_dns_isolation(0);
#endif		
	      	
#ifdef CONFIG_MIDDLEWARE
		if(sp->if_unit == getMgtPVCIndex())
		{
			/*save dns for management channel pvc*/ 
			fp = 	fopen(MGT_DNS_FILE, "w");
			memset(buff,0,32);
			if(sp->ipcp.primary_dns) {
				sprintf(buff, "%s\n", inet_ntoa(sp->ipcp.primary_dns));
				fputs(buff, fp);
			}
			if(sp->ipcp.second_dns) {
				sprintf(buff, "%s\n", inet_ntoa(sp->ipcp.second_dns));
				fputs(buff, fp);
			}
			fclose(fp);
		}
		//sendInformKeyParaMsg2MidProcess();
#endif
	}
	ppp_status();
	sppp_last_connection_error(sp->if_unit, ERROR_NONE);
	
	return 1;
}

int ppp_down_flag[N_SPPP];	// Added by Jenny, connect/disconnect manually flag

// Jenny, check authentication restart timeout
void authTimeout(void *cookie)
{
	struct sppp *sp = (struct sppp *)cookie;
	auth_fail_counter[sp->unit] = 0;
	ppp_down(sp->if_unit);
	ppp_up(sp->if_unit);
}
#ifdef CONFIG_USER_PPPOE_PROXY
void linkTimeout(void *cookie)
{
  
     struct sppp *sp = (struct sppp *)cookie;
     if(adsl_PADT_flag == 0)
	 	no_adsl_link_wan_ppp(sp->unit);

} 
#endif
int ppp_if_down(struct sppp *sp)
{

	if(!cifaddr(sp))
		return 0;
	if(!sifdown(sp))
		return 0;

	if(sp->over == SPPP_PPPOE)
		pppoeDelete(sp);
	// Kaohj
	//else if(sp->over == SPPP_PPPOATM)
	//	pppoatm_close(sp->dev);

	stop_ppp(sp);
//	sppp_detach(sp);
	//ppp_status();
//	sppp_attach(sp);
	sppp_if_down(sp);	// Jenny
	ppp_status();
#ifdef DIAGNOSTIC_TEST
	sppp_diag_log(sp->if_unit, 0);	// Jenny, diagnostic test log init
#endif
	/* Jenny, Authentication failed max counter, set timer */
	if (auth_fail_counter[sp->unit] >= 3) {
		TIMEOUT(authTimeout, (void *)sp, 120, sp->auth_ch);
		return 0;
	}
	/* dial on demand */
	if (!dial_setup(sp))
		return 0;
	/* direct dial */
//	if(sp->over == SPPP_PPPOE || !ppp_down_flag[sp->unit])	// Jenny
	if(!ppp_down_flag[sp->unit])	// Jenny
		if(!start_ppp(sp))
			return 0;
		
	return 1;
}

/*
 * add_fd - add an fd to the set that wait_input waits for.
 */
void add_fd(int fd)
{
    FD_SET(fd, &in_fds);
    if (fd > max_in_fd)
	max_in_fd = fd;
}

/*
 * remove_fd - remove an fd from the set that wait_input waits for.
 */
void remove_fd(int fd)
{
    FD_CLR(fd, &in_fds);
}

/********************************************************************
 *
 * output - Output PPP packet.
 */

void output (struct sppp *sp, unsigned char *p, int len)
{

    if (write(sp->fd, p, len) < 0) {
        if (errno == EWOULDBLOCK || errno == ENOBUFS
            || errno == ENXIO || errno == EIO || errno == EINTR)
            printf("write: warning: %m (%d)", errno);
        else
            printf("write: %m (%d)", errno);
    }
}

extern struct sppp *spppq;

int get_input()
{
	struct sppp *sp;
	int nr;
	int len = 1500;
	unsigned char buf[1536];

	sp = spppq;
	
	while(sp) {
		// Jenny
		if (sp->fd < 0) {
			sp = sp->pp_next;
			continue;
		}
		// Kaohj
		if (!FD_ISSET(sp->fd, &in_fds)) {
			sp = sp->pp_next;
			continue;
		}
		if (sp->fd >= 0) {
			nr = read(sp->fd, buf, len);
			if (nr < 0 && errno != EWOULDBLOCK && errno != EIO && errno != EINTR)
				printf("read: %m");
			if (nr < 0 && errno == ENXIO)
				break;
		}
	    if (nr < 0 && sp->if_unit >= 0) {
    	    /* N.B. we read ppp_fd first since LCP packets come in there. */
        	nr = read(sp->dev_fd, buf, len);
        	if (nr < 0 && errno != EWOULDBLOCK && errno != EIO && errno != EINTR)
            	printf("read /dev/ppp: %m");
        	if (nr < 0 && errno == ENXIO)
            	break;
    	} 
    	if(nr>0)
	    	sppp_input(sp, buf, nr);
	    sp = sp->pp_next;
    }
   	return 0;
}


int sppp_recv(void)
{
	fd_set in = in_fds;
	struct timeval tv;
	int ret;
	

	tv.tv_sec = 0;
	tv.tv_usec = 10000;

	ret = select(max_in_fd+1, &in, NULL, NULL, &tv);

	if(ret>0)
		get_input();
	return 0;
}

int stop_ppp(struct sppp *sp)
{
	// Kaohj
//#if 0
	/* disconnect channel to PPP interface */
	if (ioctl(sp->fd, PPPIOCDISCONN, &sp->if_unit) < 0) {
		printf("Couldn't disconnect PPP unit %d: %m", sp->if_unit);
	}
	if (ioctl(sp->fd, PPPIOCDETACH, &sp->chindex) < 0) {
		printf("Couldn't detach channel %d: %m", sp->chindex);
	}
//#endif

	if(sp->dev_fd >= 0)
		remove_fd(sp->dev_fd);
	if(sp->fd>=0)
		remove_fd(sp->fd);		
	if(sp->fd>=0) {
		close(sp->fd);		
		sp->fd = -1;	// Jenny
	}
	return 0;
}

// Jenny, set to 1 when press "connect" button
int ppp_up_flag[N_SPPP] = {0};

extern struct callout idle_ch;
// Kaohj
int start_ppp_real(struct sppp *sp)
{
	int flags;
//	struct sppp *tmp_sp = spppq;
	
	ppp_up_flag[sp->if_unit] = 0;

#ifdef _CWMP_MIB_
       ppp_pending_flag[sp->if_unit] = 0;
#endif
	if (sp->fd < 0) {
		printf("open ppp fd error!!\n");
		return -1;
	}
				
	/* get channel number */
	if (ioctl(sp->fd, PPPIOCGCHAN, &sp->chindex) == -1) {
	   	printf("%s: Couldn't get channel number: %d\n", sp->if_name, sp->chindex);
		return -1;
	}

	sp->fd = open("/dev/ppp", O_RDWR);
	if (sp->fd < 0) {
	   	printf("Couldn't reopen /dev/ppp\n");
		return -1;
	}

	/* attach PPP interface to exist channel */
	if (ioctl(sp->fd, PPPIOCATTCHAN, &sp->chindex) < 0) {
	   	printf("Couldn't attach to channel %d\n", sp->chindex);
		return -1;
	}

	flags = fcntl(sp->fd, F_GETFL);
	if (flags == -1 || fcntl(sp->fd, F_SETFL, flags | O_NONBLOCK) == -1)
	   	printf("Couldn't set /dev/ppp (channel) to nonblock\n");

	/* connect channel to PPP interface */
	if (ioctl(sp->fd, PPPIOCCONNECT, &sp->if_unit) < 0) {
		printf("Couldn't attach to PPP unit %d\n", sp->if_unit);
		return -1;
	}

	add_fd(sp->dev_fd);
	add_fd(sp->fd);

	sp->pp_con = ppp_if_up;
	sp->pp_tlf = ppp_if_down;

	/* set LCP restart timer timeout */
	sp->lcp.timeout = 3;
	/* set ipcp parameter */
#ifdef CONFIG_SPPPD_STATICIP
	if(sp->staticip == 0) 
		sp->ipcp.myipaddr = 0;
#else
	sp->ipcp.myipaddr = 0;
#endif
	sp->ipcp.hisipaddr = 0;
	sp->ipcp.primary_dns = 0;
	sp->ipcp.second_dns = 0;
	
	/* */
		
	if (ppp_term_flag)
		sp->pp_up(sp);
	if (sp->timeout)	// Jenny
		TIMEOUT(sppp_idle, 0, 1, idle_ch);
	return 0;
}

int start_ppp(struct sppp *sp)
{
int flags;

	if(sp->over == SPPP_PPPOATM) {
		// Kaohj
		struct pppoa_param_s *poar=sp->dev;
		if (poar->fd<=0)
			sp->fd = pppoatm_init(sp->dev);
		else
			sp->fd = poar->fd;
		//sp->fd = pppoatm_init(sp->dev);
		return(start_ppp_real(sp));
	}
	else 
	if(sp->over == SPPP_PPPOE) {
		if (sp->fd < 0)	// Jenny
			sp->fd = pppoe_client_init(sp);
		/*
		if (pppoe_client_connect(sp) == -1) {
			in_pppoe_disc = 0;
			return 0;	// pppoe deleted
		}
		in_pppoe_disc = 0;
		*/
	}
	else
		return -1;
			
#if 0
	if(sp->fd<0) {
		printf("open ppp fd error!!\n");
		return -1;
	}
				
	/* get channel number */
	if (ioctl(sp->fd, PPPIOCGCHAN, &sp->chindex) == -1) {
	   	printf("Couldn't get channel number: %d\n", sp->chindex);
		return -1;
	}
	
	
	sp->fd = open("/dev/ppp", O_RDWR);
	if (sp->fd < 0) {
	   	printf("Couldn't reopen /dev/ppp\n");
		return -1;
	}

	/* attach PPP interface to exist channel */
	if (ioctl(sp->fd, PPPIOCATTCHAN, &sp->chindex) < 0) {
	   	printf("Couldn't attach to channel %d\n", sp->chindex);
		return -1;
	}

	flags = fcntl(sp->fd, F_GETFL);
	if (flags == -1 || fcntl(sp->fd, F_SETFL, flags | O_NONBLOCK) == -1)
	   	printf("Couldn't set /dev/ppp (channel) to nonblock\n");

	/* connect channel to PPP interface */
    if (ioctl(sp->fd, PPPIOCCONNECT, &sp->if_unit) < 0) {
		printf("Couldn't attach to PPP unit %d\n", sp->if_unit);
		return -1;
	}

	add_fd(sp->dev_fd);
	add_fd(sp->fd);

	sp->pp_con = ppp_if_up;
	sp->pp_tlf = ppp_if_down;

	/* set LCP restart timer timeout */
	sp->lcp.timeout = 3;
	/* set ipcp parameter */
	sp->ipcp.myipaddr = 0;
	sp->ipcp.hisipaddr = 0;
	sp->ipcp.primary_dns = 0;
	sp->ipcp.second_dns = 0;
	
	/* */
		
	sp->pp_up(sp);
#endif
	return 0;
}

int clear_dgw(void)
{
struct sppp	*sp = spppq;
	
	while(sp) {
		sp->dgw = 0;
	    sp = sp->pp_next;
	}
	return 0;
}

int del_ppp(int unit)
{
	struct sppp *sp;
	unsigned int flags;
	
	sp = spppq;
	while(sp) {
		if(sp->unit == unit) {
			sp->pp_down(sp);
			if(sp->over == SPPP_PPPOE)
				pppoeDelete(sp);
			else if(sp->over == SPPP_PPPOATM) {
				if (sp->dev)
					pppoatm_close(sp->dev);
			}

			auth_fail_counter[sp->unit] = 0;
			ppp_down_flag[sp->unit] = 0;
			ppp_up_flag[sp->if_unit] = 0;
			disc_counter[sp->if_unit] = 0;
			pre_PADT_flag[sp->if_unit] = 0;
			sendPADR_flag[sp->if_unit] = 0;
			ioctl(ppp_dev_fd[sp->if_unit], PPPIOCGFLAGS, (caddr_t)&flags);
			if (flags & SC_LOOP_TRAFFIC)
				flags &= ~SC_LOOP_TRAFFIC;
			ioctl(ppp_dev_fd[sp->if_unit], (caddr_t)PPPIOCSFLAGS, &flags);
			
			/* disconnect channel to PPP interface */
			if (sp->fd >= 0) {
				if (ioctl(sp->fd, PPPIOCDISCONN, &sp->if_unit) < 0)
					printf("Couldn't disconnect PPP unit %d: %m", sp->if_unit);
				if (ioctl(sp->fd, PPPIOCDETACH, &sp->chindex) < 0)
					printf("Couldn't detach channel %d: %m", sp->chindex);
			}

			close(sp->sock_fd);
			sp->sock_fd = -1;

			if(sp->dev_fd >= 0)
				remove_fd(sp->dev_fd);
			if(sp->fd>=0)
				remove_fd(sp->fd);	//star
			if(sp->fd>=0) {
				close(sp->fd);
				sp->fd = -1;	// Jenny
			}

			sppp_detach(sp);

#ifdef CONFIG_USER_PPPOE_PROXY

			if(sp->enable_pppoe_proxy)
                    {   
                       
                        no_adsl_link_wan_ppp(sp->unit);
		          
                     }
#endif

			ppp_status();

			if(sp->dev)
				free(sp->dev);
			free(sp);
			return 0;
		}
		sp = sp->pp_next;		
	}
	return -1;	
}


void dial_start(int signum)
{
	unsigned int flags;
	printf("process dial signal %x\n", signum);
	if (dod_sp) {
		if(dod_sp->over == 1) {
			if(dod_sp->pp_lowerp)
				return;
		}
		if(dod_sp->pp_phase != PHASE_DEAD)
			return;
		ioctl(ppp_dev_fd[dod_sp->if_unit], PPPIOCGFLAGS, (caddr_t)&flags);
		flags &= ~SC_LOOP_TRAFFIC;
		ioctl(ppp_dev_fd[dod_sp->if_unit], (caddr_t)PPPIOCSFLAGS, &flags);
		if (!ppp_down_flag[dod_sp->unit])
			start_ppp(dod_sp);
	}
}

int sdifflag(struct sppp *sp)
{
	struct ifreq ifr;

	memset (&ifr, '\0', sizeof (ifr));
	strlcpy(ifr.ifr_name, sp->if_name, sizeof (ifr.ifr_name));
	if (ioctl(sp->sock_fd, SIOCGIFFLAGS, (caddr_t) &ifr) < 0) {
		if (! ok_error (errno))
		printf("ioctl (SIOCGIFFLAGS): %m(%d)", errno);
		return 0;
	}
	ifr.ifr_flags &= ~IFF_POINTOPOINT;
	ifr.ifr_flags |= IFF_UP;
	if (ioctl(sp->sock_fd, SIOCSIFFLAGS, (caddr_t) &ifr) < 0) {
		if (! ok_error (errno))
		printf("ioctl(SIOCSIFFLAGS): %m(%d)", errno);
		return 0;
	}
	return 1;
}

int sdialifaddr(struct sppp *sp)
{
	struct ifreq ifr;
	u_long net_mask;
	char default_route_cmd[40];

	memset (&ifr, '\0', sizeof (ifr));

	SET_SA_FAMILY (ifr.ifr_addr,    AF_INET);
	SET_SA_FAMILY (ifr.ifr_dstaddr, AF_INET);
	SET_SA_FAMILY (ifr.ifr_netmask, AF_INET);

	strlcpy (ifr.ifr_name, sp->if_name, sizeof (ifr.ifr_name));
	/* Set our IP address */
	((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr.s_addr = 0x40404040;
	if (ioctl(sp->sock_fd, SIOCSIFADDR, (caddr_t) &ifr) < 0) {
		if (errno != EEXIST) {
			if (!ok_error (errno))
			printf("ioctl(SIOCSIFADDR): %m(%d)", errno);
		}
		else {
			printf("ioctl(SIOCSIFADDR): Address already exists");
		}
		return (0);
	}
	/* Set the gateway address */
	((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr.s_addr = 0x40404040;
	if (ioctl(sp->sock_fd, SIOCSIFDSTADDR, (caddr_t) &ifr) < 0) {
		if (!ok_error (errno))
			printf("ioctl(SIOCSIFDSTADDR): %m(%d)", errno);
		return (0);
	}
	/* Set the netmask. For recent kernels, force the netmask to 255.255.255.255. */
	/* for kernel version > 2.1.16 */
	net_mask = ~0L;
	if (net_mask != 0) {
		((struct sockaddr_in *) &ifr.ifr_netmask)->sin_addr.s_addr = net_mask;
		if (ioctl(sp->sock_fd, SIOCSIFNETMASK, (caddr_t) &ifr) < 0) {
			if (!ok_error (errno))
				printf("ioctl(SIOCSIFNETMASK): %m(%d)", errno);
			return (0);
		}
	}
	/* Set dafault route */
	sprintf(default_route_cmd, "route add default ppp%d", sp->if_unit);
	system(default_route_cmd);

	return 1;
}

int dial_setup(struct sppp *sp)
{
	if (sp->timeout) {
		int ppid = (int)getpid();
		unsigned int flags;

		ioctl(ppp_dev_fd[sp->if_unit], PPPIOCSTIMEOUT, &ppid);
		ioctl(ppp_dev_fd[sp->if_unit], PPPIOCGFLAGS, (caddr_t)&flags);
		flags |= SC_LOOP_TRAFFIC;
		ioctl(ppp_dev_fd[sp->if_unit], (caddr_t)PPPIOCSFLAGS, &flags);
		dod_sp = sp;
		printf("set sp timeout %d\n", sp->timeout);
		sifup(sp);
		sdialifaddr(sp);
		sdifflag(sp);
		system("echo '0'>/proc/internet_flag");
/*		if (dod_flag == 1) {	// dial-on-demand setup, used by dns triggering for the first time
			FILE *fp = fopen("/tmp/ppp_dod_flag", "w");
			fprintf(fp, "%d", dod_flag);
			fclose(fp);
			dod_flag = 0;
		}*/
		return 0;
	}
	return -1;
}

int add_ppp(int unit, struct spppreq *spr)
{
struct sppp *sp;
//struct pppoe_param_s *poer;
//struct pppoa_param_s *poar;


int flags;
#ifdef AUTO_PPPOE_ROUTE
unsigned char dgw;
#endif

	del_ppp(unit);
	sp = malloc(sizeof(struct sppp));
	if(!sp)
		return -1;
	memset (sp, 0, sizeof (struct sppp));
        

	sp->up_flag = 1;
	sp->unit = unit;
	sp->mode = spr->ppp.mode;
//alex_huang for pppoe_proxy
#ifdef CONFIG_USER_PPPOE_PROXY
        sp->enable_pppoe_proxy=spr->ppp.enable_pppoe_proxy; 
       if(sp->enable_pppoe_proxy)
       	{
       	   TIMEOUT(linkTimeout, (void *)sp, 100, sp->link_ch);
       	}
#endif   
#ifdef CONFIG_TR069_DNS_ISOLATION
        sp->isTr069_interface = spr->ppp.isTr069_interface;
#endif
	//if(spr->ppp.dgw) {
#ifdef AUTO_PPPOE_ROUTE
	mib_get( MIB_ADSL_WAN_DGW_ITF, (void *)&dgw);	// Jenny, check default gateway
	if(dgw == DGW_AUTO || spr->ppp.dgw)	// Jenny, if set to auto(0xef)
#else
	if(spr->ppp.dgw)
#endif
	{
		/* clear all other interface dgw and set this one */
		clear_dgw();
		sp->dgw = 1;
		sp->timeout = spr->ppp.timeout;
	}
	else {
		sp->dgw = 0;
		sp->timeout = 0;
	}

	// Added by Jenny for keepalive setting
	if (spr->ppp.diska)
		sp->diska = 1;
	else
		sp->diska = 0;

#ifdef ZTE_GENERAL_ROUTER_SC
         if(spr->ppp.ippt_flag)
		sp->ippt_flag = spr->ppp.ippt_flag;		
	   else
		sp->ippt_flag = 0;
#else
	 // Added by Mason Yu for Half Bridge
	if(spr->ppp.ippt_flag) {
		sp->ippt_flag = 1;		
	}
	else
		sp->ippt_flag = 0;
#endif
	  
	/* set debug flag */
	sp->debug = spr->ppp.debug;
	
	/* setup device */
	sp->over = spr->ppp.over;
	if(sp->over == SPPP_PPPOATM) {
		sp->dev = malloc(sizeof(struct pppoa_param_s));
		if(!sp->dev)
			goto err;
		memcpy(sp->dev, spr->dev, sizeof(struct pppoa_param_s));
	}
	else 
	if(sp->over == SPPP_PPPOE) {
		sp->dev = malloc(sizeof(struct pppoe_param_s));
		if(!sp->dev)
			goto err;
		memcpy(sp->dev, spr->dev, sizeof(struct pppoe_param_s));
	}
	else
		goto err;

	/* mtu option */
	sp->lcp.mru = spr->ppp.lcp.mru;
	
	/* setting/changing myauth */
	// Kaohj -- user setting for auth
	sp->myauth.proto = spr->ppp.myauth.proto;
	sp->myauth.aumode = spr->ppp.myauth.proto;	// Jenny
	//sp->myauth.proto = PPP_PAP;
	strcpy(sp->myauth.name, spr->ppp.myauth.name);
	strcpy(sp->myauth.secret, spr->ppp.myauth.secret);

	/* setting/changing hisauth */
	bzero((char*)&sp->hisauth, sizeof sp->hisauth);

	/* assign interface unit */
	sp->dev_fd = ppp_dev_fd[unit];
	sp->if_unit = unit;
	sprintf(sp->if_name, "ppp%d", sp->if_unit);
	strcpy(spr->ppp.if_name, sp->if_name);
	sp->fd = -1;	// Jenny
#ifdef _CWMP_MIB_
	sp->autoDisconnectTime = spr->ppp.autoDisconnectTime;
	sp->warnDisconnectDelay = spr->ppp.warnDisconnectDelay;
#endif
#ifdef CONFIG_SPPPD_STATICIP
        //alex_huang for static ip setting
	if(spr->ppp.staticip == 1) {	
		sp->ipcp.myipaddr = spr->ppp.ipcp.myipaddr;
		sp->staticip = 1;
	}
#endif
    /* Get an internet socket for doing socket ioctls. */
   	sp->sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
   	if (sp->sock_fd < 0) {
		if ( ! ok_error ( errno ))
    		printf("Couldn't create IP socket: %m(%d)", errno);
   	}

	sppp_attach(sp);
	/* update ppp status file */
	ppp_status();
#ifdef DIAGNOSTIC_TEST
	sppp_diag_log(sp->if_unit, 0);	// Jenny, diagnostic test log init
#endif
	sppp_last_connection_error(sp->if_unit, ERROR_UNKNOWN);	// Jenny, last connection error init
	
	/* dial on demand */
	if (!dial_setup(sp))
		return 0;
	/* direct dial */
//	printf("add_ppp(%s)\n", sp->if_name);
	if(!start_ppp(sp))
		return 0;
	
err:
	printf("add_ppp() error!!\n");
	if(sp->fd>=0)
	{
		close(sp->fd);
		sp->fd = -1;	// Jenny
	}
	if(sp->sock_fd>=0) {
		close(sp->sock_fd);
		sp->sock_fd = -1;
	}
	if(sp->dev)	// Jenny
		free(sp->dev);
	free(sp);
	return -1;	
}

// Jenny, del ppp & add ppp for PPPoE discovery timeout several times
void reconnect_ppp(int unit)
{
	struct sppp *sp;
	struct spppreq *spr;
	
	sp = spppq;
	while(sp) {
		if(sp->unit == unit) {
			if (sp->over == SPPP_PPPOATM)
				return;
			disc_counter[unit] = 0;
			// backup original ppp parameters
			spr = malloc(sizeof(struct spppreq));
			memset(spr, 0, sizeof(struct spppreq));
			spr->ppp.unit = sp->unit;
			spr->ppp.mode = sp->mode;
			spr->ppp.dgw = sp->dgw;
			spr->ppp.timeout = sp->timeout;
			spr->ppp.diska = sp->diska;
			spr->ppp.ippt_flag = sp->ippt_flag;		
			spr->ppp.debug = sp->debug;
			spr->ppp.over = sp->over;
			spr->ppp.lcp.mru = sp->lcp.mru;
#ifdef _CWMP_MIB_
			spr->ppp.autoDisconnectTime = sp->autoDisconnectTime;
			spr->ppp.warnDisconnectDelay = sp->warnDisconnectDelay;
#endif
#ifdef CONFIG_USER_PPPOE_PROXY
                         spr->ppp.enable_pppoe_proxy=sp->enable_pppoe_proxy; 
#endif
#ifdef CONFIG_SPPPD_STATICIP
			if(sp->staticip == 1) {
				spr->ppp.staticip = sp->staticip;
				spr->ppp.ipcp.myipaddr = sp->ipcp.myipaddr;
			}
#endif
			spr->ppp.myauth.proto = sp->myauth.proto;
			strcpy(spr->ppp.myauth.name, sp->myauth.name);
			strcpy(spr->ppp.myauth.secret, sp->myauth.secret);
			spr->dev = malloc(sizeof(struct pppoe_param_s));
			memcpy(spr->dev, sp->dev, sizeof(struct pppoe_param_s));
//			del_ppp(unit);
			add_ppp(unit, spr);
			free(spr->dev);
			free(spr);
		}
		sp = sp->pp_next;		
	}
}

extern struct spppreq sprt;
extern struct pppoe_param_s poert;
extern struct pppoa_param_s poart;


int show_ppp(int unit)
{
struct sppp *sp;
struct pppoe_param_s *poer;
struct pppoa_param_s *poar;
char *encaps_str[] = {"VCMUX", "LLC"};
char *qos_str[] = {"none", "UBR", "CBR", "VBR"};
char *aal_str[] = {"AAL0", "AAL5"};

	sp = spppq;
	while(sp) {
		if(sp->unit == unit) {
			sprt.dev = sp->dev;
			sprt.ppp.unit = unit;
			sprt.ppp.over = sp->over;
			sprt.ppp.mode = sp->mode;
			sprt.ppp.pp_phase = sp->pp_phase;
			strcpy(sprt.ppp.if_name, sp->if_name);
			memcpy(&sprt.ppp.lcp, &sp->lcp, sizeof(struct slcp));
			memcpy(&sprt.ppp.ipcp, &sp->ipcp, sizeof(struct sipcp));
			memcpy(&sprt.ppp.myauth, &sp->myauth, sizeof(struct sauth));
			memcpy(&sprt.ppp.hisauth, &sp->hisauth, sizeof(struct sauth));
			if(sprt.ppp.over == 0) {
				memcpy(&poart, sprt.dev, sizeof poart);
			}
			if(sprt.ppp.over == 1) {
				memcpy(&poert, sprt.dev, sizeof poert);
			}
			
			#if 0
			//////////////////////////////////////////////
			printf("%s\n", sprt.ppp.if_name);
			if(sprt.ppp.over == 0) {
				poar = sprt.dev;	
				printf("PPPoATM PVC = %d.%d, %s, %s, Qos = %s \n", poar->addr.sap_addr.vpi, poar->addr.sap_addr.vci, \
					aal_str[poar->qos.aal],
					encaps_str[poar->encaps],
					qos_str[poar->qos.txtp.traffic_class]);
			}
			if(sprt.ppp.over == 1) {
				poer = sprt.dev;
				printf("PPPoE %s, phase = %d\n", poer->dev_name, poer->DiscoveryState);
				printf("MAC = %02X%02X%02X%02X%02X%02X, Server MAC = %02X%02X%02X%02X%02X%02X\n",
					poer->MyEthAddr[0], poer->MyEthAddr[1], poer->MyEthAddr[2], poer->MyEthAddr[3], poer->MyEthAddr[4], poer->MyEthAddr[5], 
					poer->ACEthAddr[0], poer->ACEthAddr[1], poer->ACEthAddr[2], poer->ACEthAddr[3], poer->ACEthAddr[4], poer->ACEthAddr[5]);
			}
			
			printf("PPP PHASE = %d\n", sprt.ppp.pp_phase);
			printf("username = %s\n", sprt.ppp.myauth.name);
			printf("password = %s\n", sprt.ppp.myauth.secret);
			printf("MRU = %d\n", sprt.ppp.lcp.mru);
			printf("mode = %d\n", sprt.ppp.mode);
			#endif
			
			break;
		}
		sp = sp->pp_next;
	}
}


int init_ppp_unit(void)
{
	int i;
	int flags;
	int if_unit;

	for(i=0; i<N_SPPP; i++) {
		ppp_dev_fd[i] = open("/dev/ppp", O_RDWR);
   		if (ppp_dev_fd[i] < 0)
   			printf("Couldn't open /dev/ppp: %m");
		flags = fcntl(ppp_dev_fd[i], F_GETFL);
		if (flags == -1 || fcntl(ppp_dev_fd[i], F_SETFL, flags | O_NONBLOCK) == -1)
			printf("Couldn't set /dev/ppp to nonblock: %m");
		/* creates a new PPP interface and make /dev/ppp own the interface */
		if_unit = -1;
		if(ioctl(ppp_dev_fd[i], PPPIOCNEWUNIT, &if_unit) < 0) {
			printf("Couldn't create new ppp unit: %m");
		}
	}

    FD_ZERO(&in_fds);
    max_in_fd = 0;

}	


#if 0
#include <netdb.h>
int pingsock = -1;
int myid = 0;
static int create_icmp_socket(void)
{
	struct protoent *proto;
	int sock;

	proto = getprotobyname("icmp");
	/* if getprotobyname failed, just silently force
	 * proto->p_proto to have the correct value for "icmp" */
	if ((sock = socket(AF_INET, SOCK_RAW, (proto ? proto->p_proto : 1))) < 0)        /* 1 == ICMP */
		printf("cannot create raw socket\n");
	return sock;
}
#endif

#if defined(SPPPD_NO_IDLE) || defined(QSETUP_WEB_REDIRECT)
#define PPPOE_ITF_CHECK
#endif
int ppp_status(void)
{
struct sppp *sp;
struct pppoa_param_s *poar;
struct pppoe_param_s *poer;
PPPOE_DRV_CTRL *p;
char buff[256];
char sbuf[20], tbuf[20];
char *encaps_str[] = {"VCMUX", "LLC"};
char *qos_str[] = {"none", "UBR", "CBR", "VBR"};
char *aal_str[] = {"AAL0", "AAL5"};
FILE *fp;
struct flock flpoe, flpoa;
int fdpoe, fdpoa;
#ifdef PPPOE_ITF_CHECK
int pppoe_no = -1, flags;
struct in_addr inAddr;
int startRedirect = 0;
#endif

	signal(SIGUSR2, SIG_IGN);
	// Jenny, file locking
	fdpoe = open("/var/ppp/pppoe.conf", O_RDWR);
	fdpoa = open("/var/ppp/pppoa.conf", O_RDWR);
	if ((fdpoe != -1) && (fdpoe != -1)) {
		flpoe.l_type = flpoa.l_type = F_WRLCK;
		flpoe.l_whence = flpoa.l_whence = SEEK_SET;
		flpoe.l_start = flpoa.l_start = 0;
		flpoe.l_len = flpoa.l_len = 0;
		flpoe.l_pid = flpoa.l_pid = getpid();
		if (fcntl(fdpoe, F_SETLKW, &flpoe) == -1)
			printf("pppoe write lock failed\n");
		if (fcntl(fdpoa, F_SETLKW, &flpoa) == -1)
			printf("pppoa write lock failed\n");
	}
	fp = fopen("/var/ppp/ppp.conf", "w");

	sprintf(buff, "%-5s%-6s%-8s%-3s%-15s%-16s%16s%6s\n", \
		"if", "dev", "dev_v", "gw", "phase", "username", "password", "MRU");
	fputs(buff, fp);
			
#ifdef PPPOE_ITF_CHECK
	if (spppq)
		pppoe_no = 0;
#endif
	
	sp = spppq;
	while(sp) {
		if(sp->over == SPPP_PPPOATM) {
			poar = sp->dev;
			sprintf(sbuf, "%d.%d", poar->addr.sap_addr.vpi, poar->addr.sap_addr.vci);	
		}
		if(sp->over == SPPP_PPPOE) {
			poer = sp->dev;
			sprintf(sbuf, "%s", poer->dev_name);
		}

		sprintf(buff, "%-5s%-6s%-8s%-3d%-15s%-16s%16s%6d\n", \
			sp->if_name, (sp->over ? "PPPoE" : "PPPoA"), \
			sbuf, sp->dgw, sppp_phase_name(sp->pp_phase), \
			sp->myauth.name, sp->myauth.secret, \
			(sp->lcp.mru <= sp->lcp.their_mru ? sp->lcp.mru : sp->lcp.their_mru));
		fputs(buff, fp);
#ifdef PPPOE_ITF_CHECK
		if (getInFlags(sp->if_name, &flags) == 1) {
			if ((flags & IFF_UP) && (getInAddr( sp->if_name, IP_ADDR, (void *)&inAddr) == 1))
				pppoe_no ++;
		}
#endif
		sp = sp->pp_next;
	}
	fclose(fp);


	fp = fopen("/var/ppp/pppoe.conf", "w");

	sprintf(buff, "%-8s%-8s%-15s %-12s %-12s %-17s %-17s\n", \
		"if", "dev", "phase", "MAC", "AC_MAC", "uptime", "totaluptime");
	fputs(buff, fp);

	sp = spppq;
	while(sp) {
		if(sp->over == SPPP_PPPOE) {
			poer = sp->dev;
			p = sp->pp_lowerp;
			sbuf[0] = '\0';
			tbuf[0] = '\0';
			sppp_uptime(sp, sbuf, tbuf);
			if(p) {
				sprintf(buff, "%-8s%-8s%-15s %02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X %-17s %-17s\n", \
					sp->if_name, p->name, _pppoe_state_name(p->DiscoveryState), \
					p->MyEthAddr[0], p->MyEthAddr[1], p->MyEthAddr[2], p->MyEthAddr[3], p->MyEthAddr[4], p->MyEthAddr[5], \
					p->ACEthAddr[0], p->ACEthAddr[1], p->ACEthAddr[2], p->ACEthAddr[3], p->ACEthAddr[4], p->ACEthAddr[5], sbuf, tbuf);
			}
			else {
				sprintf(buff, "%-8s%-8s%-15s %02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X %-17s %-17s\n", \
					sp->if_name, poer->dev_name, "Idle", \
					poer->MyEthAddr[0], poer->MyEthAddr[1], poer->MyEthAddr[2], poer->MyEthAddr[3], poer->MyEthAddr[4], poer->MyEthAddr[5], \
					poer->ACEthAddr[0], poer->ACEthAddr[1], poer->ACEthAddr[2], poer->ACEthAddr[3], poer->ACEthAddr[4], poer->ACEthAddr[5], sbuf, tbuf);
			}	
			fputs(buff, fp);
		}
		sp = sp->pp_next;
	}
	fclose(fp);

	fp = fopen("/var/ppp/pppoa.conf", "w");

	sprintf(buff, "%-5s%-8s%-7s%-7s%-5s%-6s%-6s%-6s%-17s%-17s\n", \
		"if", "dev", "class", "encaps", "qos", "pcr", "scr", "mbs", "uptime", "totaluptime");
	fputs(buff, fp);
			
	sp = spppq;
	while(sp) {
		if(sp->over == SPPP_PPPOATM) {
			poar = sp->dev;
			sbuf[0] = '\0';
			tbuf[0] = '\0';
			sppp_uptime(sp, sbuf, tbuf);
			sprintf(buff, "%-5s%-3d%-5d%-7s%-7s%-5s%-6d%-6d%-6d%-17s%-17s\n", \
				sp->if_name, poar->addr.sap_addr.vpi, poar->addr.sap_addr.vci, \
				poar->qos.aal==ATM_AAL0?aal_str[0]:aal_str[1], \
				encaps_str[poar->encaps], \
				qos_str[poar->qos.txtp.traffic_class], \
				poar->qos.txtp.pcr, \
				poar->qos.txtp.scr, \
				poar->qos.txtp.mbs, sbuf, tbuf);
			fputs(buff, fp);
		}
		sp = sp->pp_next;
	}
	fclose(fp);

	// Jenny, file unlocking
	if ((fdpoe != -1) && (fdpoe != -1)) {
		flpoe.l_type = flpoa.l_type = F_UNLCK;
		if (fcntl(fdpoe, F_SETLK, &flpoe) == -1)
			printf("pppoe write unlock failed\n");
		if (fcntl(fdpoa, F_SETLK, &flpoa) == -1)
			printf("pppoa write unlock failed\n");
		close(fdpoe);
		close(fdpoa);
	}

#ifdef PPPOE_ITF_CHECK
	if ((fp = fopen("/tmp/QSetup", "r"))) {
		fscanf(fp, "%d\n", &startRedirect);
		fclose(fp);
	}
	if (pppoe_no == 0) {
		if (!startRedirect) {
#ifdef QSETUP_WEB_REDIRECT
			start_qsetup_redirect();
#endif
#if 0
			if (pingsock > 0) {
				close(pingsock);
				pingsock = -1;
			}
#endif
		}
	}
	else {
		if (startRedirect) {
#ifdef QSETUP_WEB_REDIRECT
			stop_qsetup_redirect();
#endif
#if 0
			if (pingsock < 0)
				if ((pingsock = create_icmp_socket()) < 0)
					printf("ping: socket create error");		
			myid = getpid() & 0xFFFF;
#endif
		}
	}
#endif

	signal(SIGUSR2, ppp_status);
	return 0;	
}

// Jenny, for PPP connecting manually
int ppp_new(int unit, struct spppreq *spr)
{
	struct sppp *sp;

	del_ppp(unit);
	sp = malloc(sizeof(struct sppp));
	if(!sp)
		return -1;
	memset (sp, 0, sizeof (struct sppp));

	sp->up_flag = 1;
	sp->unit = unit;
	sp->mode = spr->ppp.mode;
	if(spr->ppp.dgw) {
		clear_dgw();
		sp->dgw = 1;
		sp->timeout = spr->ppp.timeout;
	}
	else {
		sp->dgw = 0;
		sp->timeout = 0;
	}

        if(spr->ppp.ippt_flag)
		sp->ippt_flag = spr->ppp.ippt_flag;		
	 else
		sp->ippt_flag = 0;

       
	if (spr->ppp.diska)
		sp->diska = 1;
	else
		sp->diska = 0;
	sp->debug = spr->ppp.debug;
	sp->over = spr->ppp.over;
	if(sp->over == SPPP_PPPOATM) {
		sp->dev = malloc(sizeof(struct pppoa_param_s));
		if(!sp->dev)
			goto err;
		memcpy(sp->dev, spr->dev, sizeof(struct pppoa_param_s));
	}
	else 
	if(sp->over == SPPP_PPPOE) {
		sp->dev = malloc(sizeof(struct pppoe_param_s));
		if(!sp->dev)
			goto err;
		memcpy(sp->dev, spr->dev, sizeof(struct pppoe_param_s));
	}
	else
		goto err;

	sp->lcp.mru = spr->ppp.lcp.mru;
	sp->myauth.proto = spr->ppp.myauth.proto;
	strcpy(sp->myauth.name, spr->ppp.myauth.name);
	strcpy(sp->myauth.secret, spr->ppp.myauth.secret);
	bzero((char*)&sp->hisauth, sizeof sp->hisauth);
	sp->dev_fd = ppp_dev_fd[unit];
	sp->if_unit = unit;
	sprintf(sp->if_name, "ppp%d", sp->if_unit);
	strcpy(spr->ppp.if_name, sp->if_name);
	sp->fd = -1;
#ifdef _CWMP_MIB_
	sp->autoDisconnectTime = spr->ppp.autoDisconnectTime;
	sp->warnDisconnectDelay = spr->ppp.warnDisconnectDelay;
#endif
#ifdef CONFIG_SPPPD_STATICIP
        //Jenny, static ip setting
	if(spr->ppp.staticip == 1) {	
		sp->ipcp.myipaddr = spr->ppp.ipcp.myipaddr;
		sp->staticip = 1;
	}
#endif

   	sp->sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
   	if (sp->sock_fd < 0) {
		if ( ! ok_error ( errno ))
    		printf("Couldn't create IP socket: %m(%d)", errno);
   	}

	sppp_attach(sp);
	if(sp->over == SPPP_PPPOE) {
		if (sp->fd < 0)
			sp->fd = pppoe_client_init(sp);
		pppoeDelete(sp);
		sp->fd = -1;
	}
	else if(sp->over == SPPP_PPPOATM) {
		struct pppoa_param_s *poar=sp->dev;
		if (poar->fd <= 0)
			sp->fd = pppoatm_init(sp->dev);
		else
			sp->fd = poar->fd;
		ppp_down_flag[sp->unit] = 1;
		//start_ppp(sp);
		//ppp_down(sp->unit);
	}
	ppp_status();
#ifdef DIAGNOSTIC_TEST
	sppp_diag_log(sp->if_unit, 0);	// Jenny, diagnostic test log init
#endif
	sppp_last_connection_error(sp->if_unit, ERROR_UNKNOWN);	// Jenny, last connection error init
//	printf("%s connection manually OK!!\n", sp->if_name);
	pppoe_up_log(unit, 1);
	return 1;
	
err:
	printf("new ppp() error!!\n");
	if(sp->fd>=0)
	{
		close(sp->fd);
		sp->fd = -1;
	}
	if(sp->sock_fd>=0) {
		close(sp->sock_fd);
		sp->sock_fd = -1;
	}
	if(sp->dev)
		free(sp->dev);
	free(sp);
	return -1;	
}

// Jenny, for PPP connecting manually
int ppp_up(int unit)
{
	struct sppp *sp;
	PPPOE_DRV_CTRL *p;
	
	if (ppp_up_flag[unit])
		return 1;
//	ppp_up_flag[unit] = 1;
	sp = spppq;
	while(sp) {
		if(sp->unit == unit) {
			if (sp->pp_phase != PHASE_DEAD)
				return 1;
			ppp_up_flag[unit] = 1;
//			ppp_down(unit);
			if(sp->over == SPPP_PPPOE)
				p = sp->pp_lowerp;
			ppp_down_flag[sp->unit] = 0;
			break;
		}
		sp = sp->pp_next;
	}
	
	dial_setup(sp);
	if(!start_ppp(sp))
		return 0;
	
	printf("!!%s up error!!\n", sp->if_name);
	return -1;	
}

// Jenny, for PPP disconnecting manually
int ppp_down(int unit)
{
	struct sppp *sp;
	struct pppoa_param_s *poar;
	
	sp = spppq;
	while(sp) {
		if(sp->unit == unit) {
			if (sp->pp_phase == PHASE_DEAD)
				return 1;
			ppp_down_flag[sp->unit] = 1;
			sp->pp_down(sp);
			if(sp->over == SPPP_PPPOATM) {
				poar = sp->dev;
				remove_fd(poar->fd);
				remove_fd(sp->dev_fd);
			}
//			stop_ppp(sp);
			ppp_status();
//			printf("%s down OK!!\n", sp->if_name);
			ppp_up_flag[sp->if_unit] = 0;
			return 0;
		}
		sp = sp->pp_next;		
	}
	
	printf("!!%s down error!!\n", sp->if_name);
	return -1;	
}


// Mason YU, for PPP connecting manually of Remote Manage
int rm_pppoe_test(int unit, struct spppreq *spr)
{
	struct sppp *sp;
	struct pppoa_param_s *poar;
	struct pppoe_param_s *poer;
	PPPOE_DRV_CTRL *p;
	
	sp = spppq;
	while(sp) {
		if(sp->unit == unit) {
			if (sp->pp_phase == PHASE_NETWORK)
				return 1;
			if(sp->over == SPPP_PPPOATM) {
				poar = sp->dev;
				ppp_down_flag[sp->unit] = 0;
			}
			if(sp->over == SPPP_PPPOE) {
				poer = sp->dev;
				p = sp->pp_lowerp;
				if (poer->DiscoveryState == PPPOE_STATE_SESSION)
					return 1;
			}
			break;
		}
		sp = sp->pp_next;
	}
	
	// Change username ans password
	strcpy(sp->myauth.name, spr->ppp.myauth.name);
	strcpy(sp->myauth.secret, spr->ppp.myauth.secret);
	
	dial_setup(sp);
	if(!start_ppp(sp))
		return 0;
	
	printf("!!%s up error!!\n", sp->if_name);
	return -1;	
}

//int debug_set(int unit)
int debug_set(int unit, struct spppreq *spr)
{
	struct sppp *sp;

	sp = spppq;
	while(sp) {
		if(sp->unit == unit) {
			/*if (sp->debug == 1)
				sp->debug = 0;
			else
				sp->debug = 1;*/
			sp->debug = spr->ppp.debug;
			sppp_debug_log(sp->unit, sp->debug);
			return 0;
		}
		sp = sp->pp_next;
	}
	return 0;
}

#ifdef PPP_QUICK_CONNECT
int ppp_quick_connect(int unit, unsigned char *name, unsigned char *passwd)
{
	struct sppp *sp;
	sp = spppq;
	while(sp) {
		if(sp->unit == unit)
			break;
		sp = sp->pp_next;
	}
	strcpy(sp->myauth.name, name);
	strcpy(sp->myauth.secret, passwd);
	reconnect_ppp(unit);
	return 0;
}
#endif

#ifdef ENABLE_PPP_SYSLOG
void syslog_set(int dbg)
{
	FILE *fp;
	if (fp = fopen(PPP_SYSLOG, "w")) {
		fprintf(fp, "%d", dbg);
		fclose(fp);
	}
}
#endif

