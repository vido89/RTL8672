/*
 *      Web server handler routines for DNS stuffs
 *
 */


/*-- System inlcude files --*/
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/route.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"

void formDns(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str, *submitUrl;
	char tmpBuf[100];
	char vChar;
	int dns_changed=0;
#ifndef NO_ACTION
	int pid;
#endif
#ifdef DNS_BIND_PVC_SUPPORT
	DnsBindPvcRoute(DELDNSROUTE);
#endif
	str = websGetVar(wp, T("dnsMode"), T(""));
	if ( str[0] ) {
		DNS_TYPE_T dns, dns_old;
		
		if (!strcmp(str, T("dnsAuto")))
			dns = DNS_AUTO;
		else if (!strcmp(str, T("dnsManual")))
			dns = DNS_MANUAL;
		else {
			strcpy(tmpBuf, T(Tinvalid_DNS_mode));
			goto setErr_dns;
		}

		if ( !mib_get(MIB_ADSL_WAN_DNS_MODE, (void *)&vChar)) {
	  		strcpy(tmpBuf, T(TDNS_mib_get_error));
			goto setErr_dns;
		}
		dns_old = (DNS_TYPE_T)vChar;
		if (dns != dns_old)
			dns_changed = 1;
		//jim strict check dns changed....
		if ( !mib_get(MIB_ADSL_WAN_DNS1, (void *)&vChar)) {
	  		strcpy(tmpBuf, T(TDNS_mib_get_error));
			goto setErr_dns;
		}

		// Set DNS to MIB
		vChar = (unsigned char) dns;
		if ( !mib_set(MIB_ADSL_WAN_DNS_MODE, (void *)&vChar)) {
	  		strcpy(tmpBuf, T(TDNS_mib_set_error));
			goto setErr_dns;
		}

		if ( dns == DNS_MANUAL ) {
			struct in_addr dns1, dns2, dns3;
			//jim 
			struct in_addr dns1_old, dns2_old, dns3_old;
			dns1.s_addr=0;
            dns2.s_addr=0;
            dns3.s_addr=0;
            // If DHCP server is enabled in LAN, update dhcpd.conf
			str = websGetVar(wp, T("dns1"), T(""));
			if ( str[0] ) {
				if ( !inet_aton(str, &dns1) ) {
					strcpy(tmpBuf, T(Tinvalid_DNS_address));
					goto setErr_dns;
				}
				//jim 
				if ( !mib_get(MIB_ADSL_WAN_DNS1, (void *)&dns1_old)) {
	  				strcpy(tmpBuf, T(TDNS_mib_get_error));
					goto setErr_dns;
				}
				if(dns1.s_addr!=dns1_old.s_addr)
					dns_changed = 1;
				
				if ( !mib_set(MIB_ADSL_WAN_DNS1, (void *)&dns1)) {
	  				strcpy(tmpBuf, T(TDNS_mib_set_error));
					goto setErr_dns;
				}
				#ifdef DNS_BIND_PVC_SUPPORT
				str = websGetVar(wp, T("enableDnsBind"), T(""));
				if(str[0]){//dns bind enabled
				   unsigned char enableDnsBind=1;
				   if(!mib_set(MIB_DNS_BIND_PVC_ENABLE,(void *)&enableDnsBind))
				   	{
				   	printf("set MIB_DNS_BIND_PVC_ENABLE error!\n");
					strcpy(tmpBuf, T(TDNS_mib_set_error));
							goto setErr_dns;
				   	}
				    str=websGetVar(wp,T("dnspvc1"),T(""));
				    printf("dns1 interface %d\n",atoi(str));
				    unsigned char dns1itfIndex=( unsigned char )atoi(str);
				if ( !mib_set(MIB_DNS_BIND_PVC1, (void *)&dns1itfIndex)) {
			  				strcpy(tmpBuf, T(TDNS_mib_set_error));
							goto setErr_dns;
					}
				}else{
				   unsigned char enableDnsBind=0;
				   if(!mib_set(MIB_DNS_BIND_PVC_ENABLE,&enableDnsBind))
				   	{
				   	printf("set MIB_DNS_BIND_PVC_ENABLE error!\n");
					strcpy(tmpBuf, T(TDNS_mib_set_error));
							goto setErr_dns;
				   	}
					}
				#endif
			}
			else {
				goto setOk_dns;
			}
			str = websGetVar(wp, T("dns2"), T(""));
			if ( str[0] ) {
				if ( !inet_aton(str, &dns2) ) {
					strcpy(tmpBuf, T(Tinvalid_DNS_address));
					goto setErr_dns;
				}
				//jim 
				if ( !mib_get(MIB_ADSL_WAN_DNS2, (void *)&dns2_old)) {
	  				strcpy(tmpBuf, T(TDNS_mib_get_error));
					goto setErr_dns;
				}
				if(dns2.s_addr!=dns2_old.s_addr)
					dns_changed = 1;
				
				if ( !mib_set(MIB_ADSL_WAN_DNS2, (void *)&dns2)) {
	  				strcpy(tmpBuf, T(TDNS_mib_set_error));
					goto setErr_dns;
				}
				#ifdef DNS_BIND_PVC_SUPPORT
				str = websGetVar(wp, T("enableDnsBind"), T(""));
				if(str[0]){//dns bind enabled
				    str=websGetVar(wp,T("dnspvc2"),T(""));
				    printf("dns2 interface %d\n",atoi(str));
				    unsigned char dns2itfIndex=( unsigned char )atoi(str);
				if ( !mib_set(MIB_DNS_BIND_PVC2, (void *)&dns2itfIndex)) {
			  				strcpy(tmpBuf, T(TDNS_mib_set_error));
							goto setErr_dns;
					}
				}
				#endif
			}
			else {
				//jim 
				if ( !mib_get(MIB_ADSL_WAN_DNS2, (void *)&dns2_old)) {
	  				strcpy(tmpBuf, T(TDNS_mib_get_error));
					goto setErr_dns;
				}
				if(dns2.s_addr!=dns2_old.s_addr)
					dns_changed = 1;
				
				if ( !mib_set(MIB_ADSL_WAN_DNS2, (void *)&dns2)) {
	  				strcpy(tmpBuf, T(TDNS_mib_set_error));
					goto setErr_dns;
				}
			}
			str = websGetVar(wp, T("dns3"), T(""));
			if ( str[0] ) {
				if ( !inet_aton(str, &dns3) ) {
					strcpy(tmpBuf, T(Tinvalid_DNS_address));
					goto setErr_dns;
				}
				//jim 
				if ( !mib_get(MIB_ADSL_WAN_DNS3, (void *)&dns3_old)) {
	  				strcpy(tmpBuf, T(TDNS_mib_get_error));
					goto setErr_dns;
				}
				if(dns3.s_addr!=dns3_old.s_addr)
					dns_changed = 1;
				if ( !mib_set(MIB_ADSL_WAN_DNS3, (void *)&dns3)) {
	  				strcpy(tmpBuf, T(TDNS_mib_set_error));
					goto setErr_dns;
				}
				#ifdef DNS_BIND_PVC_SUPPORT
				str = websGetVar(wp, T("enableDnsBind"), T(""));
				if(str[0]){//dns bind enabled
				    str=websGetVar(wp,T("dnspvc3"),T(""));
				    printf("dns3 interface %d\n",atoi(str));
				    unsigned char dns3itfIndex=( unsigned char )atoi(str);
				if ( !mib_set(MIB_DNS_BIND_PVC3, (void *)&dns3itfIndex)) {
			  				strcpy(tmpBuf, T(TDNS_mib_set_error));
							goto setErr_dns;
					}
				}
				#endif
			}
			else {
				//jim 
				if ( !mib_get(MIB_ADSL_WAN_DNS3, (void *)&dns3_old)) {
	  				strcpy(tmpBuf, T(TDNS_mib_get_error));
					goto setErr_dns;
				}
				if(dns3.s_addr!=dns3_old.s_addr)
					dns_changed = 1;
				
				if ( !mib_set(MIB_ADSL_WAN_DNS3, (void *)&dns3)) {
	  				strcpy(tmpBuf, T(TDNS_mib_set_error));
					goto setErr_dns;
				}
			}
		#ifdef DNS_BIND_PVC_SUPPORT
		 DnsBindPvcRoute(ADDDNSROUTE);			
		#endif
		}
	}

setOk_dns:
	/* upgdate to flash */
//	mib_update(CURRENT_SETTING);

#ifndef NO_ACTION
	pid = fork();
        if (pid)
                waitpid(pid, NULL, 0);
        else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _CONFIG_SCRIPT_PROG);
#ifdef HOME_GATEWAY
		execl( tmpBuf, _CONFIG_SCRIPT_PROG, "gw", "bridge", NULL);
#else
		execl( tmpBuf, _CONFIG_SCRIPT_PROG, "ap", "bridge", NULL);
#endif
                exit(1);
        }
#endif

#if defined(APPLY_CHANGE) || defined(ZTE_GENERAL_ROUTER_SC)
	if(dns_changed)
	{
		#if defined(ZTE_GENERAL_ROUTER_SC)
		mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
		#endif
		//jim valid immediately....		
		restart_dnsrelay();
	}
#endif
	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	OK_MSG(submitUrl);
  	return;

setErr_dns:
	ERR_MSG(tmpBuf);
}

