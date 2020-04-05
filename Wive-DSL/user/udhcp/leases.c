/* 
 * leases.c -- tools to manage DHCP leases 
 * Russ Dill <Russ.Dill@asu.edu> July 2001
 */

#include <time.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "debug.h"
#include "dhcpd.h"
#include "files.h"
#include "options.h"
#include "leases.h"
#include "arpping.h"

unsigned char blank_chaddr[] = {[0 ... 15] = 0};

void lease_timer()
{
    int i, iUpdateForcePortal = 0;
    
	for (i = 0; i < server_config.max_leases; i++)
    {
		if ((leases[i].yiaddr != 0)
            && (lease_expired(&(leases[i]))))
        {
        	// Kaohj
#ifdef _CWMP_TR111_
		// Remove option 125 device when lease expired
		if (del_deviceId(leases[i].yiaddr))
			dump_deviceId();
#endif
            if (NULL != leases[i].stClientInfo.pfLeaseExpired)
            {
                leases[i].stClientInfo.pfLeaseExpired(&leases[i]);
            }

            if (FP_InvalidDevice != leases[i].stClientInfo.ulDevice)
            {
                iUpdateForcePortal = 1;
            }
            
            memset(&leases[i].stClientInfo, 0, sizeof(struct dhcpClientInfo));
		}
	}

    if (iUpdateForcePortal)
    {
        /* update_force_portal(); */
    }
    
    return;
}

void clear_all_lease()
{
    int i;
    
	for (i = 0; i < server_config.max_leases; i++)
    {
		if (leases[i].yiaddr != 0) 
        {
            if (NULL != leases[i].stClientInfo.pfLeaseExpired)
            {
                leases[i].stClientInfo.pfLeaseExpired(&leases[i]);
            }
		}
	}

    return;
}

void clear_one_lease(struct dhcpOfferedAddr *pstLease)
{
    int iUpdateForcePortal = 0;
    
    if (NULL != pstLease->stClientInfo.pfLeaseExpired)
    {
        pstLease->stClientInfo.pfLeaseExpired(pstLease);
    }

    if (FP_InvalidDevice != pstLease->stClientInfo.ulDevice)
    {
        iUpdateForcePortal = 1;
    }
    
    memset(pstLease, 0, sizeof(struct dhcpOfferedAddr));

    if (iUpdateForcePortal)
    {
        /* update_force_portal(); */
    }

    return;
}

/* clear every lease out that chaddr OR yiaddr matches and is nonzero */
void clear_lease(u_int8_t *chaddr, u_int32_t yiaddr)
{
	unsigned int i, j;
	
	for (j = 0; j < 16 && !chaddr[j]; j++);
	
	for (i = 0; i < server_config.max_leases; i++)
		if ((j != 16 && !memcmp(leases[i].chaddr, chaddr, 16)) ||
		    (yiaddr && leases[i].yiaddr == yiaddr)) {
			clear_one_lease(&(leases[i]));
		}
}


/* add a lease into the table, clearing out any old ones */
struct dhcpOfferedAddr *add_lease(u_int8_t *chaddr, u_int32_t yiaddr, unsigned long lease)
{
	struct dhcpOfferedAddr *oldest;
	
	/* clean out any old ones */
	clear_lease(chaddr, yiaddr);
		
	oldest = oldest_expired_lease();
	
	if (oldest) {
        clear_one_lease(oldest);
		memcpy(oldest->chaddr, chaddr, 16);
		oldest->yiaddr = yiaddr;
		// Mason Yu
		if ( lease == 0xffffffff ){		
			//printf("DHCPD Lease time is overflow!!\n");			
			oldest->expires = 0xffffffff;
		}else
			oldest->expires = time(0) + lease;
	}
	
	return oldest;
}


/* true if a lease has expired */
int lease_expired(struct dhcpOfferedAddr *lease)
{
	return (lease->expires < (unsigned long) time(0));
}	


/* Find the oldest expired lease, NULL if there are no expired leases */
struct dhcpOfferedAddr *oldest_expired_lease(void)
{
	struct dhcpOfferedAddr *oldest = NULL;
	unsigned long oldest_lease = time(0);
	unsigned int i;

	
	for (i = 0; i < server_config.max_leases; i++)
		if (oldest_lease > leases[i].expires) {
			oldest_lease = leases[i].expires;
			oldest = &(leases[i]);
		}
	return oldest;
		
}


/* Find the first lease that matches chaddr, NULL if no match */
struct dhcpOfferedAddr *find_lease_by_chaddr(u_int8_t *chaddr)
{
	unsigned int i;

	for (i = 0; i < server_config.max_leases; i++)
		if (!memcmp(leases[i].chaddr, chaddr, 16)) return &(leases[i]);
	
	return NULL;
}


/* Find the first lease that matches yiaddr, NULL is no match */
struct dhcpOfferedAddr *find_lease_by_yiaddr(u_int32_t yiaddr)
{
	unsigned int i;

	for (i = 0; i < server_config.max_leases; i++)
		if (leases[i].yiaddr == yiaddr) return &(leases[i]);
	
	return NULL;
}


/* find an assignable address, it check_expired is true, we check all the expired leases as well.
 * Maybe this should try expired leases by age... */
#ifdef IP_BASED_CLIENT_TYPE
u_int32_t find_address(int check_expired, enum DeviceType devicetype ) 
#else
u_int32_t find_address(int check_expired) 
#endif
{
	u_int32_t addr, ret;
	struct dhcpOfferedAddr *lease = NULL;	

#ifdef IP_BASED_CLIENT_TYPE
	u_int32_t addrend;
	switch(devicetype)
	{
		case CTC_Computer:
			addr = ntohl(server_config.pcstart);
			addrend = ntohl(server_config.pcend);
			break;
		case CTC_Camera:
			addr = ntohl(server_config.cmrstart);
			addrend = ntohl(server_config.cmrend);
			break;
		case CTC_STB:
			addr = ntohl(server_config.stbstart);
			addrend = ntohl(server_config.stbend);
			break;
		case CTC_PHONE:
			addr = ntohl(server_config.phnstart);
			addrend = ntohl(server_config.phnend);
			break;
		case CTC_HGW:
			addr = ntohl(server_config.hgwstart);
			addrend = ntohl(server_config.hgwend);
			break;            
		default:
			addr = ntohl(server_config.start);
			addrend = ntohl(server_config.end);
			break;
	}
	for (;addr <= addrend; addr++) {

		if(devicetype == CTC_UNKNOWN &&
			((addr>ntohl(server_config.pcstart)&&addr<ntohl(server_config.pcend))||
			   (addr>ntohl(server_config.cmrstart)&&addr<ntohl(server_config.cmrend))||
			   (addr>ntohl(server_config.stbstart)&&addr<ntohl(server_config.stbend))||
			   (addr>ntohl(server_config.phnstart)&&addr<ntohl(server_config.phnend))))
				continue;

		/* ie, 192.168.55.0 */
		if (!(addr & 0xFF)) continue;

		/* ie, 192.168.55.255 */
		if ((addr & 0xFF) == 0xFF) continue;
//jim added by star zhang
		if(find_Mac_by_IP(addr)) continue;// star add: for static ip based Mac

		/* lease is not taken */
		ret = htonl(addr);
		if ((!(lease = find_lease_by_yiaddr(ret)) ||

		     /* or it expired and we are checking for expired leases */
		     (check_expired  && lease_expired(lease))) &&

		     /* and it isn't on the network */
	    	     !check_ip(ret)) {
			return ret;
			break;
		}
	}
#else
	addr = ntohl(server_config.start); /* addr is in host order here */
	for (;addr <= ntohl(server_config.end); addr++) {

		/* ie, 192.168.55.0 */
		if (!(addr & 0xFF)) continue;

		/* ie, 192.168.55.255 */
		if ((addr & 0xFF) == 0xFF) continue;
//jim added by star zhang
		if(find_Mac_by_IP(addr)) continue;// star add: for static ip based Mac

		/* lease is not taken */
		ret = htonl(addr);
		if ((!(lease = find_lease_by_yiaddr(ret)) ||

		     /* or it expired and we are checking for expired leases */
		     (check_expired  && lease_expired(lease))) &&

		     /* and it isn't on the network */
	    	     !check_ip(ret)) {
			return ret;
			break;
		}
	}
#endif
	return 0;
}


/* check is an IP is taken, if it is, add it to the lease table */
int check_ip(u_int32_t addr)     
{
	struct in_addr temp;
	
	if (arpping(addr, server_config.server, server_config.arp, server_config.interface) == 0) {
		temp.s_addr = addr;
	 	LOG(LOG_INFO, "%s belongs to someone, reserving it for %ld seconds", 
	 		inet_ntoa(temp), server_config.conflict_time);
		add_lease(blank_chaddr, addr, server_config.conflict_time);
		return 1;
	} else return 0;
}

