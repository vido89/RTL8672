/* linux/net/inet/arp.c
 *
 * Version:	$Id: arp.c,v 1.9 2007/09/27 01:42:13 yachang Exp $
 *
 * Copyright (C) 1994 by Florian  La Roche
 *
 * This module implements the Address Resolution Protocol ARP (RFC 826),
 * which is used to convert IP addresses (or in the future maybe other
 * high-level addresses) into a low-level hardware address (like an Ethernet
 * address).
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 * Fixes:
 *		Alan Cox	:	Removed the Ethernet assumptions in 
 *					Florian's code
 *		Alan Cox	:	Fixed some small errors in the ARP 
 *					logic
 *		Alan Cox	:	Allow >4K in /proc
 *		Alan Cox	:	Make ARP add its own protocol entry
 *		Ross Martin     :       Rewrote arp_rcv() and arp_get_info()
 *		Stephen Henson	:	Add AX25 support to arp_get_info()
 *		Alan Cox	:	Drop data when a device is downed.
 *		Alan Cox	:	Use init_timer().
 *		Alan Cox	:	Double lock fixes.
 *		Martin Seine	:	Move the arphdr structure
 *					to if_arp.h for compatibility.
 *					with BSD based programs.
 *		Andrew Tridgell :       Added ARP netmask code and
 *					re-arranged proxy handling.
 *		Alan Cox	:	Changed to use notifiers.
 *		Niibe Yutaka	:	Reply for this device or proxies only.
 *		Alan Cox	:	Don't proxy across hardware types!
 *		Jonathan Naylor :	Added support for NET/ROM.
 *		Mike Shaver     :       RFC1122 checks.
 *		Jonathan Naylor :	Only lookup the hardware address for
 *					the correct hardware type.
 *		Germano Caronni	:	Assorted subtle races.
 *		Craig Schlenter :	Don't modify permanent entry 
 *					during arp_rcv.
 *		Russ Nelson	:	Tidied up a few bits.
 *		Alexey Kuznetsov:	Major changes to caching and behaviour,
 *					eg intelligent arp probing and 
 *					generation
 *					of host down events.
 *		Alan Cox	:	Missing unlock in device events.
 *		Eckes		:	ARP ioctl control errors.
 *		Alexey Kuznetsov:	Arp free fix.
 *		Manuel Rodriguez:	Gratuitous ARP.
 *              Jonathan Layes  :       Added arpd support through kerneld 
 *                                      message queue (960314)
 *		Mike Shaver	:	/proc/sys/net/ipv4/arp_* support
 *		Mike McLagan    :	Routing by source
 *		Stuart Cheshire	:	Metricom and grat arp fixes
 *					*** FOR 2.1 clean this up ***
 *		Lawrence V. Stefani: (08/12/96) Added FDDI support.
 *		Alan Cox 	:	Took the AP1000 nasty FDDI hack and
 *					folded into the mainstream FDDI code.
 *					Ack spit, Linus how did you allow that
 *					one in...
 *		Jes Sorensen	:	Make FDDI work again in 2.1.x and
 *					clean up the APFDDI & gen. FDDI bits.
 *		Alexey Kuznetsov:	new arp state machine;
 *					now it is in net/core/neighbour.c.
 *		Krzysztof Halasa:	Added Frame Relay ARP support.
 */

#include <linux/types.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/config.h>
#include <linux/socket.h>
#include <linux/sockios.h>
#include <linux/errno.h>
#include <linux/in.h>
#include <linux/mm.h>
#include <linux/inet.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/fddidevice.h>
#include <linux/if_arp.h>
#include <linux/trdevice.h>
#include <linux/skbuff.h>
#include <linux/proc_fs.h>
#include <linux/stat.h>
#include <linux/init.h>
#ifdef CONFIG_SYSCTL
#include <linux/sysctl.h>
#endif

#include <net/ip.h>
#include <net/icmp.h>
#include <net/route.h>
#include <net/protocol.h>
#include <net/tcp.h>
#include <net/sock.h>
#include <net/arp.h>
#if defined(CONFIG_AX25) || defined(CONFIG_AX25_MODULE)
#include <net/ax25.h>
#if defined(CONFIG_NETROM) || defined(CONFIG_NETROM_MODULE)
#include <net/netrom.h>
#endif
#endif
#ifdef CONFIG_ATM_CLIP
#include <net/atmclip.h>
#endif

#include <asm/system.h>
#include <asm/uaccess.h>

#include <linux/netfilter_arp.h>
#ifdef	CONFIG_RTL867X_IPTABLES_FAST_PATH
#include "fastpath/fastpath_core.h"
#endif


/*
 	temp macro defined by jim to debug...
*/
// China Telecom spec.
#ifdef CONFIG_CTC_E8_CLIENT_LIMIT
enum DeviceType
{
	CTC_Computer=0,
	CTC_Carema
};
enum CTCIdentity
{
	NEWONE=0,
	NORMALONE
};



#define CTC_BLOCKED 1
#define CTC_UNBLOCKED 0
#define CTC_DETECTED 1
#define CTC_UNDETECTED 0
struct ClientsDetect{
	struct ClientsDetect *next;
	u32 ip;
	u32 detected; //0, waitting to be replied, 1, detected.
	enum CTCIdentity identity;	// 0, new clients ready to become normal members, 1, normal menber.
	u32 blocked;   //0, unblocked, 1, blocked
	enum DeviceType type; //device type : Computer or Camera .. determined by dhcp server.
};
static struct proc_dir_entry *clientspath;
#define MAXNUM    256          //
#define CLIENT_POLLING_INTERVAL  4       // 4 seconds to polling the results
#define CLIENT_UPDATE_AFTER        1       //After 1 second since polled, check the detect results.
#define PROCFS_NAME    "ClientsMonitor"
static char procfs_buffer[32] = {'0',  '0'};
static struct ClientsDetect *clients_to_detect=NULL;   
static struct ClientsDetect *clients_type_table=NULL;   // all possilbe exist or unexist clients' types are stored here, mantained by user process.
static unsigned int client_detect_flag;
static struct timer_list  timer_client_polling;
static struct timer_list  timer_client_update;
static int flagLimitOnAll=1;   //switch mirror to InternetGatewayDevice.Services.X_CT-COM_MWBAND.Enalbe
static int limitOnAll=1;			//mirror to InternetGatewayDevice.Services.X_CT-COM_MWBAND.TotalTerminalNumber.
static int computerLimitEnable=1; //mirror to InternetGatewayDevice.Services.X_CT-COM_MWBAND.ComputerRestrictEnable.
static int limitOnComputer=2;  //mirror to InternetGatewayDevice.Services.X_CT-COM_MWBAND.ComputerRestrictNumber
static int cameraLimitEnable=1;//mirror to InternetGatewayDevice.Services.X_CT-COM_MWBAND.CameraRestrictEnable.
static int limitOnCamera=1;	//mirror to InternetGatewayDevice.Services.X_CT-COM_MWBAND.CameraRestrictNumber.
static int clients_limit_proc_pid=0; // used
// used to buffer the output strings 
static unsigned char out_buffer_base[1024];
static unsigned char *cur_out_buffer=out_buffer_base;
static int flag_signal=0; // if set we will send signal to user process.
static int lock_clientlist=0;  //
static int insertClient(struct ClientsDetect ** clientslist, u32 ip, 
		enum DeviceType devtype, enum CTCIdentity identity, u32 detected, u32 blocked);
static struct ClientsDetect * clientInList(struct ClientsDetect * clientslist, u32 ip);
#endif

/*
 *	Interface to generic neighbour cache.
 */
static u32 arp_hash(const void *pkey, const struct net_device *dev);
static int arp_constructor(struct neighbour *neigh);
static void arp_solicit(struct neighbour *neigh, struct sk_buff *skb);
static void arp_error_report(struct neighbour *neigh, struct sk_buff *skb);
static void parp_redo(struct sk_buff *skb);

static struct neigh_ops arp_generic_ops = {
	family:			AF_INET,
	solicit:		arp_solicit,
	error_report:		arp_error_report,
	output:			neigh_resolve_output,
	connected_output:	neigh_connected_output,
	hh_output:		dev_queue_xmit,
	queue_xmit:		dev_queue_xmit,
};

static struct neigh_ops arp_hh_ops = {
	family:			AF_INET,
	solicit:		arp_solicit,
	error_report:		arp_error_report,
	output:			neigh_resolve_output,
	connected_output:	neigh_resolve_output,
	hh_output:		dev_queue_xmit,
	queue_xmit:		dev_queue_xmit,
};

static struct neigh_ops arp_direct_ops = {
	family:			AF_INET,
	output:			dev_queue_xmit,
	connected_output:	dev_queue_xmit,
	hh_output:		dev_queue_xmit,
	queue_xmit:		dev_queue_xmit,
};

struct neigh_ops arp_broken_ops = {
	family:			AF_INET,
	solicit:		arp_solicit,
	error_report:		arp_error_report,
	output:			neigh_compat_output,
	connected_output:	neigh_compat_output,
	hh_output:		dev_queue_xmit,
	queue_xmit:		dev_queue_xmit,
};

struct neigh_table arp_tbl = {
	family:		AF_INET,
	entry_size:	sizeof(struct neighbour) + 4,
	key_len:	4,
	hash:		arp_hash,
	constructor:	arp_constructor,
	proxy_redo:	parp_redo,
	id:		"arp_cache",
	parms: {
		tbl:			&arp_tbl,
		base_reachable_time:	30 * HZ,
		retrans_time:		1 * HZ,
		gc_staletime:		60 * HZ,
		reachable_time:		30 * HZ,
		delay_probe_time:	5 * HZ,
		queue_len:		3,
		ucast_probes:		3,
		mcast_probes:		3,
		anycast_delay:		1 * HZ,
		proxy_delay:		(8 * HZ) / 10,
		proxy_qlen:		64,
		locktime:		1 * HZ,
	},
	gc_interval:	30 * HZ,
	gc_thresh1:	128,
	gc_thresh2:	512,
	gc_thresh3:	1024,
};

int arp_mc_map(u32 addr, u8 *haddr, struct net_device *dev, int dir)
{
	switch (dev->type) {
	case ARPHRD_ETHER:
	case ARPHRD_FDDI:
	case ARPHRD_IEEE802:
		ip_eth_mc_map(addr, haddr);
		return 0; 
	case ARPHRD_IEEE802_TR:
		ip_tr_mc_map(addr, haddr);
		return 0;
	default:
		if (dir) {
			memcpy(haddr, dev->broadcast, dev->addr_len);
			return 0;
		}
	}
	return -EINVAL;
}


static u32 arp_hash(const void *pkey, const struct net_device *dev)
{
	u32 hash_val;

	hash_val = *(u32*)pkey;
	hash_val ^= (hash_val>>16);
	hash_val ^= hash_val>>8;
	hash_val ^= hash_val>>3;
	hash_val = (hash_val^dev->ifindex)&NEIGH_HASHMASK;

	return hash_val;
}

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
static int arp_constructor(struct neighbour *neigh)
{
	u32 addr = *(u32*)neigh->primary_key;
	struct net_device *dev = neigh->dev;
	struct in_device *in_dev = in_dev_get(dev);

	if (in_dev == NULL)
		return -EINVAL;

	neigh->type = inet_addr_type(addr);
	if (in_dev->arp_parms)
		neigh->parms = in_dev->arp_parms;

	in_dev_put(in_dev);

	if (dev->hard_header == NULL) {
		neigh->nud_state = NUD_NOARP;
		neigh->ops = &arp_direct_ops;
		neigh->output = neigh->ops->queue_xmit;
	} else {
		/* Good devices (checked by reading texts, but only Ethernet is
		   tested)

		   ARPHRD_ETHER: (ethernet, apfddi)
		   ARPHRD_FDDI: (fddi)
		   ARPHRD_IEEE802: (tr)
		   ARPHRD_METRICOM: (strip)
		   ARPHRD_ARCNET:
		   etc. etc. etc.

		   ARPHRD_IPDDP will also work, if author repairs it.
		   I did not it, because this driver does not work even
		   in old paradigm.
		 */

#if 1
		/* So... these "amateur" devices are hopeless.
		   The only thing, that I can say now:
		   It is very sad that we need to keep ugly obsolete
		   code to make them happy.

		   They should be moved to more reasonable state, now
		   they use rebuild_header INSTEAD OF hard_start_xmit!!!
		   Besides that, they are sort of out of date
		   (a lot of redundant clones/copies, useless in 2.1),
		   I wonder why people believe that they work.
		 */
		switch (dev->type) {
		default:
			break;
		case ARPHRD_ROSE:	
#if defined(CONFIG_AX25) || defined(CONFIG_AX25_MODULE)
		case ARPHRD_AX25:
#if defined(CONFIG_NETROM) || defined(CONFIG_NETROM_MODULE)
		case ARPHRD_NETROM:
#endif
			neigh->ops = &arp_broken_ops;
			neigh->output = neigh->ops->output;
			return 0;
#endif
		;}
#endif
		if (neigh->type == RTN_MULTICAST) {
			neigh->nud_state = NUD_NOARP;
			arp_mc_map(addr, neigh->ha, dev, 1);
		} else if (dev->flags&(IFF_NOARP|IFF_LOOPBACK)) {
			neigh->nud_state = NUD_NOARP;
			memcpy(neigh->ha, dev->dev_addr, dev->addr_len);
		} else if (neigh->type == RTN_BROADCAST || dev->flags&IFF_POINTOPOINT) {
			neigh->nud_state = NUD_NOARP;
			memcpy(neigh->ha, dev->broadcast, dev->addr_len);
		}
		if (dev->hard_header_cache)
			neigh->ops = &arp_hh_ops;
		else
			neigh->ops = &arp_generic_ops;
		if (neigh->nud_state&NUD_VALID)
			neigh->output = neigh->ops->connected_output;
		else
			neigh->output = neigh->ops->output;
	}
	return 0;
}

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
static void arp_error_report(struct neighbour *neigh, struct sk_buff *skb)
{
	dst_link_failure(skb);
	kfree_skb(skb);
}

static void arp_solicit(struct neighbour *neigh, struct sk_buff *skb)
{
	u32 saddr;
	u8  *dst_ha = NULL;
	struct net_device *dev = neigh->dev;
	u32 target = *(u32*)neigh->primary_key;
	int probes = atomic_read(&neigh->probes);

	if (skb && inet_addr_type(skb->nh.iph->saddr) == RTN_LOCAL)
		saddr = skb->nh.iph->saddr;
	else
		saddr = inet_select_addr(dev, target, RT_SCOPE_LINK);

	if ((probes -= neigh->parms->ucast_probes) < 0) {
		if (!(neigh->nud_state&NUD_VALID))
			printk(KERN_DEBUG "trying to ucast probe in NUD_INVALID\n");
		dst_ha = neigh->ha;
		read_lock_bh(&neigh->lock);
	} else if ((probes -= neigh->parms->app_probes) < 0) {
#ifdef CONFIG_ARPD
		neigh_app_ns(neigh);
#endif
		return;
	}

	arp_send(ARPOP_REQUEST, ETH_P_ARP, target, dev, saddr,
		 dst_ha, dev->dev_addr, NULL);
	if (dst_ha)
		read_unlock_bh(&neigh->lock);
}

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
static int arp_filter(__u32 sip, __u32 tip, struct net_device *dev)
{
	struct rtable *rt;
	int flag = 0; 
	/*unsigned long now; */

	if (ip_route_output(&rt, sip, tip, 0, 0) < 0) 
		return 1;
	if (rt->u.dst.dev != dev) { 
		NET_INC_STATS_BH(ArpFilter);
		flag = 1;
	} 
	ip_rt_put(rt); 
	return flag; 
} 

/* OBSOLETE FUNCTIONS */

/*
 *	Find an arp mapping in the cache. If not found, post a request.
 *
 *	It is very UGLY routine: it DOES NOT use skb->dst->neighbour,
 *	even if it exists. It is supposed that skb->dev was mangled
 *	by a virtual device (eql, shaper). Nobody but broken devices
 *	is allowed to use this function, it is scheduled to be removed. --ANK
 */

static int arp_set_predefined(int addr_hint, unsigned char * haddr, u32 paddr, struct net_device * dev)
{
	switch (addr_hint) {
	case RTN_LOCAL:
		printk(KERN_DEBUG "ARP: arp called for own IP address\n");
		memcpy(haddr, dev->dev_addr, dev->addr_len);
		return 1;
	case RTN_MULTICAST:
		arp_mc_map(paddr, haddr, dev, 1);
		return 1;
	case RTN_BROADCAST:
		memcpy(haddr, dev->broadcast, dev->addr_len);
		return 1;
	}
	return 0;
}


#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
int arp_find(unsigned char *haddr, struct sk_buff *skb)
{
	struct net_device *dev = skb->dev;
	u32 paddr;
	struct neighbour *n;

	if (!skb->dst) {
		printk(KERN_DEBUG "arp_find is called with dst==NULL\n");
		kfree_skb(skb);
		return 1;
	}

	paddr = ((struct rtable*)skb->dst)->rt_gateway;

	if (arp_set_predefined(inet_addr_type(paddr), haddr, paddr, dev))
		return 0;

	n = __neigh_lookup(&arp_tbl, &paddr, dev, 1);

	if (n) {
		n->used = jiffies;
		if (n->nud_state&NUD_VALID || neigh_event_send(n, skb) == 0) {
			read_lock_bh(&n->lock);
 			memcpy(haddr, n->ha, dev->addr_len);
			read_unlock_bh(&n->lock);
			neigh_release(n);
			return 0;
		}
		neigh_release(n);
	} else
		kfree_skb(skb);
	return 1;
}

/* END OF OBSOLETE FUNCTIONS */

int arp_bind_neighbour(struct dst_entry *dst)
{
	struct net_device *dev = dst->dev;
	struct neighbour *n = dst->neighbour;

	if (dev == NULL)
		return -EINVAL;
	if (n == NULL) {
		u32 nexthop = ((struct rtable*)dst)->rt_gateway;
#ifdef CONFIG_ATM_CLIP
		if ((dev->flags&(IFF_LOOPBACK|IFF_POINTOPOINT)) && dev->type != ARPHRD_ATM)	// Jenny
#else
		if (dev->flags&(IFF_LOOPBACK|IFF_POINTOPOINT))
#endif
			nexthop = 0;
		n = __neigh_lookup_errno(
#ifdef CONFIG_ATM_CLIP
		    dev->type == ARPHRD_ATM ? &clip_tbl :
#endif
		    &arp_tbl, &nexthop, dev);
		if (IS_ERR(n))
			return PTR_ERR(n);
		dst->neighbour = n;
	}
	return 0;
}

/*
 * Check if we can use proxy ARP for this path
 */

static inline int arp_fwd_proxy(struct in_device *in_dev, struct rtable *rt)
{
	struct in_device *out_dev;
	int imi, omi = -1;

	if (!IN_DEV_PROXY_ARP(in_dev))
		return 0;

	if ((imi = IN_DEV_MEDIUM_ID(in_dev)) == 0)
		return 1;
	if (imi == -1)
		return 0;

	/* place to check for proxy_arp for routes */

	if ((out_dev = in_dev_get(rt->u.dst.dev)) != NULL) {
		omi = IN_DEV_MEDIUM_ID(out_dev);
		in_dev_put(out_dev);
	}
	return (omi != imi && omi != -1);
}

/*
 *	Interface to link layer: send routine and receive handler.
 */

/*
 *	Create and send an arp packet. If (dest_hw == NULL), we create a broadcast
 *	message.
 */

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
void arp_send(int type, int ptype, u32 dest_ip, 
	      struct net_device *dev, u32 src_ip, 
	      unsigned char *dest_hw, unsigned char *src_hw,
	      unsigned char *target_hw)
{
	struct sk_buff *skb;
	struct arphdr *arp;
	unsigned char *arp_ptr;

	/*
	 *	No arp on this interface.
	 */
	
	if (dev->flags&IFF_NOARP)
		return;

	/*
	 *	Allocate a buffer
	 */
	
	skb = alloc_skb(sizeof(struct arphdr)+ 2*(dev->addr_len+4)
				+ dev->hard_header_len + 15, GFP_ATOMIC);
	if (skb == NULL)
		return;

	skb_reserve(skb, (dev->hard_header_len+15)&~15);
	skb->nh.raw = skb->data;
	arp = (struct arphdr *) skb_put(skb,sizeof(struct arphdr) + 2*(dev->addr_len+4));
	skb->dev = dev;
	skb->protocol = __constant_htons (ETH_P_ARP);
	if (src_hw == NULL)
		src_hw = dev->dev_addr;
	if (dest_hw == NULL)
		dest_hw = dev->broadcast;

	/*
	 *	Fill the device header for the ARP frame
	 */
	if (dev->hard_header &&
	    dev->hard_header(skb,dev,ptype,dest_hw,src_hw,skb->len) < 0)
		goto out;

	/*
	 * Fill out the arp protocol part.
	 *
	 * The arp hardware type should match the device type, except for FDDI,
	 * which (according to RFC 1390) should always equal 1 (Ethernet).
	 */
	/*
	 *	Exceptions everywhere. AX.25 uses the AX.25 PID value not the
	 *	DIX code for the protocol. Make these device structure fields.
	 */
	switch (dev->type) {
	default:
		arp->ar_hrd = htons(dev->type);
		arp->ar_pro = __constant_htons(ETH_P_IP);
		break;

#if defined(CONFIG_AX25) || defined(CONFIG_AX25_MODULE)
	case ARPHRD_AX25:
		arp->ar_hrd = __constant_htons(ARPHRD_AX25);
		arp->ar_pro = __constant_htons(AX25_P_IP);
		break;

#if defined(CONFIG_NETROM) || defined(CONFIG_NETROM_MODULE)
	case ARPHRD_NETROM:
		arp->ar_hrd = __constant_htons(ARPHRD_NETROM);
		arp->ar_pro = __constant_htons(AX25_P_IP);
		break;
#endif
#endif

#ifdef CONFIG_FDDI
	case ARPHRD_FDDI:
		arp->ar_hrd = __constant_htons(ARPHRD_ETHER);
		arp->ar_pro = __constant_htons(ETH_P_IP);
		break;
#endif
#ifdef CONFIG_TR
	case ARPHRD_IEEE802_TR:
		arp->ar_hrd = __constant_htons(ARPHRD_IEEE802);
		arp->ar_pro = __constant_htons(ETH_P_IP);
		break;
#endif
	}

	arp->ar_hln = dev->addr_len;
	arp->ar_pln = 4;
	arp->ar_op = htons(type);

	arp_ptr=(unsigned char *)(arp+1);

	memcpy(arp_ptr, src_hw, dev->addr_len);
	arp_ptr+=dev->addr_len;
	memcpy(arp_ptr, &src_ip,4);
	arp_ptr+=4;
	if (target_hw != NULL)
		memcpy(arp_ptr, target_hw, dev->addr_len);
	else
		memset(arp_ptr, 0, dev->addr_len);
	arp_ptr+=dev->addr_len;
	memcpy(arp_ptr, &dest_ip, 4);

	/* Send it off, maybe filter it using firewalling first.  */
	NF_HOOK(NF_ARP, NF_ARP_OUT, skb, NULL, dev, dev_queue_xmit);
	return;

out:
	kfree_skb(skb);
}

static void parp_redo(struct sk_buff *skb)
{
	arp_rcv(skb, skb->dev, NULL);
}

/*
 *	Process an arp request.
 */

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
int arp_process(struct sk_buff *skb)
{
	struct net_device *dev = skb->dev;
	struct in_device *in_dev = in_dev_get(dev);
	struct arphdr *arp;
	unsigned char *arp_ptr;
	struct rtable *rt;
	unsigned char *sha, *tha;
	u32 sip, tip;
	u16 dev_type = dev->type;
	int addr_type;
	struct neighbour *n;

	/* arp_rcv below verifies the ARP header, verifies the device
	 * is ARP'able, and linearizes the SKB (if needed).
	 */

	if (in_dev == NULL)
		goto out;

	arp = skb->nh.arph;
	arp_ptr= (unsigned char *)(arp+1);

	switch (dev_type) {
	default:	
		if (arp->ar_pro != __constant_htons(ETH_P_IP))
			goto out;
		if (htons(dev_type) != arp->ar_hrd)
			goto out;
		break;
#ifdef CONFIG_NET_ETHERNET
	case ARPHRD_ETHER:
		/*
		 * ETHERNET devices will accept ARP hardware types of either
		 * 1 (Ethernet) or 6 (IEEE 802.2).
		 */
		if (arp->ar_hrd != __constant_htons(ARPHRD_ETHER) &&
		    arp->ar_hrd != __constant_htons(ARPHRD_IEEE802))
			goto out;
		if (arp->ar_pro != __constant_htons(ETH_P_IP))
			goto out;
		break;
#endif
#ifdef CONFIG_TR
	case ARPHRD_IEEE802_TR:
		/*
		 * Token ring devices will accept ARP hardware types of either
		 * 1 (Ethernet) or 6 (IEEE 802.2).
		 */
		if (arp->ar_hrd != __constant_htons(ARPHRD_ETHER) &&
		    arp->ar_hrd != __constant_htons(ARPHRD_IEEE802))
			goto out;
		if (arp->ar_pro != __constant_htons(ETH_P_IP))
			goto out;
		break;
#endif
#ifdef CONFIG_FDDI
	case ARPHRD_FDDI:
		/*
		 * According to RFC 1390, FDDI devices should accept ARP hardware types
		 * of 1 (Ethernet).  However, to be more robust, we'll accept hardware
		 * types of either 1 (Ethernet) or 6 (IEEE 802.2).
		 */
		if (arp->ar_hrd != __constant_htons(ARPHRD_ETHER) &&
		    arp->ar_hrd != __constant_htons(ARPHRD_IEEE802))
			goto out;
		if (arp->ar_pro != __constant_htons(ETH_P_IP))
			goto out;
		break;
#endif
#ifdef CONFIG_NET_FC
	case ARPHRD_IEEE802:
		/*
		 * According to RFC 2625, Fibre Channel devices (which are IEEE
		 * 802 devices) should accept ARP hardware types of 6 (IEEE 802)
		 * and 1 (Ethernet).
		 */
		if (arp->ar_hrd != __constant_htons(ARPHRD_ETHER) &&
		    arp->ar_hrd != __constant_htons(ARPHRD_IEEE802))
			goto out;
		if (arp->ar_pro != __constant_htons(ETH_P_IP))
			goto out;
		break;
#endif
#if defined(CONFIG_AX25) || defined(CONFIG_AX25_MODULE)
	case ARPHRD_AX25:
		if (arp->ar_pro != __constant_htons(AX25_P_IP))
			goto out;
		if (arp->ar_hrd != __constant_htons(ARPHRD_AX25))
			goto out;
		break;
#if defined(CONFIG_NETROM) || defined(CONFIG_NETROM_MODULE)
	case ARPHRD_NETROM:
		if (arp->ar_pro != __constant_htons(AX25_P_IP))
			goto out;
		if (arp->ar_hrd != __constant_htons(ARPHRD_NETROM))
			goto out;
		break;
#endif
#endif
	}

	/* Understand only these message types */

	if (arp->ar_op != __constant_htons(ARPOP_REPLY) &&
	    arp->ar_op != __constant_htons(ARPOP_REQUEST))
		goto out;

/*
 *	Extract fields
 */
	sha=arp_ptr;
	arp_ptr += dev->addr_len;
	memcpy(&sip, arp_ptr, 4);
	arp_ptr += 4;
	tha=arp_ptr;
	arp_ptr += dev->addr_len;
	memcpy(&tip, arp_ptr, 4);
/* 
 *	Check for bad requests for 127.x.x.x and requests for multicast
 *	addresses.  If this is one such, delete it.
 */
	if (LOOPBACK(tip) || MULTICAST(tip))
		goto out;

/*
 *     Special case: We must set Frame Relay source Q.922 address
 */
	if (dev_type == ARPHRD_DLCI)
		sha = dev->broadcast;

/*
 *  Process entry.  The idea here is we want to send a reply if it is a
 *  request for us or if it is a request for someone else that we hold
 *  a proxy for.  We want to add an entry to our cache if it is a reply
 *  to us or if it is a request for our address.  
 *  (The assumption for this last is that if someone is requesting our 
 *  address, they are probably intending to talk to us, so it saves time 
 *  if we cache their address.  Their address is also probably not in 
 *  our cache, since ours is not in their cache.)
 * 
 *  Putting this another way, we only care about replies if they are to
 *  us, in which case we add them to the cache.  For requests, we care
 *  about those for us and those for our proxies.  We reply to both,
 *  and in the case of requests for us we add the requester to the arp 
 *  cache.
 */

	/* Special case: IPv4 duplicate address detection packet (RFC2131) */
	if (sip == 0) {
		if (arp->ar_op == __constant_htons(ARPOP_REQUEST) &&
		    inet_addr_type(tip) == RTN_LOCAL)
			arp_send(ARPOP_REPLY,ETH_P_ARP,tip,dev,tip,sha,dev->dev_addr,dev->dev_addr);
		goto out;
	}

	if (arp->ar_op == __constant_htons(ARPOP_REQUEST) &&
	    ip_route_input(skb, tip, sip, 0, dev) == 0) {

		rt = (struct rtable*)skb->dst;
		addr_type = rt->rt_type;

		if (addr_type == RTN_LOCAL) {
			n = neigh_event_ns(&arp_tbl, sha, &sip, dev);
			if (n) {
				int dont_send = 0;
				if (IN_DEV_ARPFILTER(in_dev))
					dont_send |= arp_filter(sip,tip,dev); 				
				if (!dont_send)
					{ 
					//add by ramen to filter the ARP request that's tip isn't belong to the device which received the arp packet
                    			struct in_ifaddr *dev_ifaddr;
                   			 for(dev_ifaddr=in_dev->ifa_list;dev_ifaddr;dev_ifaddr=dev_ifaddr->ifa_next)
			                        {
			                            if(tip==dev_ifaddr->ifa_local) //judge the requested ip is euqal to the device's ip                     
			                            arp_send(ARPOP_REPLY,ETH_P_ARP,sip,dev,tip,sha,dev->dev_addr,sha);
			                        }
					}

				neigh_release(n);
			}
			goto out;
		} else if (IN_DEV_FORWARD(in_dev)) {
			if ((rt->rt_flags&RTCF_DNAT) ||
			    (addr_type == RTN_UNICAST  && rt->u.dst.dev != dev &&
			     (arp_fwd_proxy(in_dev, rt) || pneigh_lookup(&arp_tbl, &tip, dev, 0)))) {
				n = neigh_event_ns(&arp_tbl, sha, &sip, dev);
				if (n)
					neigh_release(n);

				if (skb->stamp.tv_sec == 0 ||
				    skb->pkt_type == PACKET_HOST ||
				    in_dev->arp_parms->proxy_delay == 0) {
					arp_send(ARPOP_REPLY,ETH_P_ARP,sip,dev,tip,sha,dev->dev_addr,sha);
				} else {
					pneigh_enqueue(&arp_tbl, in_dev->arp_parms, skb);
					in_dev_put(in_dev);
					return 0;
				}
				goto out;
			}
		}
	}

	/* Update our ARP tables */

	n = __neigh_lookup(&arp_tbl, &sip, dev, 0);

#ifdef CONFIG_IP_ACCEPT_UNSOLICITED_ARP
	/* Unsolicited ARP is not accepted by default.
	   It is possible, that this option should be enabled for some
	   devices (strip is candidate)
	 */
	if (n == NULL &&
	    arp->ar_op == __constant_htons(ARPOP_REPLY) &&
	    inet_addr_type(sip) == RTN_UNICAST)
		n = __neigh_lookup(&arp_tbl, &sip, dev, -1);
#endif

	if (n) {
		int state = NUD_REACHABLE;
		int override = 0;

		/* If several different ARP replies follows back-to-back,
		   use the FIRST one. It is possible, if several proxy
		   agents are active. Taking the first reply prevents
		   arp trashing and chooses the fastest router.
		 */
		if (jiffies - n->updated >= n->parms->locktime)
			override = 1;

		/* Broadcast replies and request packets
		   do not assert neighbour reachability.
		 */
		if (arp->ar_op != __constant_htons(ARPOP_REPLY) ||
		    skb->pkt_type != PACKET_HOST)
			state = NUD_STALE;
		neigh_update(n, sha, state, override, 1);
		neigh_release(n);
#ifdef CONFIG_CTC_E8_CLIENT_LIMIT
		if((arp->ar_op == __constant_htons(ARPOP_REPLY))&&(skb->dev==dev_get_by_name("br0")))
		{
			//unnecessary
			/*
			//copy send ip address to sip..
 			u32 sip;
			struct net_device *dev;
			struct ClientsDetect * localClientsList;
			memcpy(&sip, (unsigned char*)(&(arp[1]))+arp->ar_hln, 4);
			dev=dev_get_by_name("br0");
			localClientsList=clients_to_detect;
			//send arp request to detect if all the clients are still here.......
			while(localClientsList)
			{
				u32 saddr;
				if(!localClientsList->ip)
					break;
				saddr = inet_select_addr(dev, localClientsList->ip, RT_SCOPE_LINK);
				if(saddr)
					arp_send(ARPOP_REQUEST, ETH_P_ARP, localClientsList->ip, dev, saddr,
		 				NULL, dev->dev_addr, NULL);
				else
				{
					printk("no ip found on device\n");
				}
				localClientsList=localClientsList->next;
			}			
			dev_put(dev);
			insertClient(clients_to_detect,  sip);
			// 2 seconds later, we will check the results in 
			mod_timer(&timer_client_polling, jiffies+CLIENT_POLLING_INTERVAL*HZ);
			*/
		}
#endif
	}

#ifdef CONFIG_CTC_E8_CLIENT_LIMIT
//added by jim luo
	if((arp->ar_op == __constant_htons(ARPOP_REPLY))&&(skb->dev==dev_get_by_name("br0")))
 	{
 		//copy send ip address to sip..
 		u32 sip;
		struct ClientsDetect *client;
		memcpy(&sip, (unsigned char*)(&(arp[1]))+arp->ar_hln, 4);
		
		//printk("dectecing client with arp %08x\n", sip);
		if((client=clientInList(clients_to_detect, sip))==NULL) //client is not in list
		{
			printk("new client detected\n");
			insertClient(&clients_to_detect, sip, CTC_Computer, NEWONE, CTC_DETECTED, CTC_UNBLOCKED);
		}else
		{
			//set detected flag...
			//printk("set detected flag, clients 0x%08x re detected \n", sip);
			client->detected=CTC_DETECTED;
		}
 	}
#endif

out:
	if (in_dev)
		in_dev_put(in_dev);
	kfree_skb(skb);
	return 0;
}


/*
 *	Receive an arp request from the device layer.
 */

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
int arp_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt)
{
	struct arphdr *arp = skb->nh.arph;

	if (arp->ar_hln != dev->addr_len ||
	    dev->flags & IFF_NOARP ||
	    skb->pkt_type == PACKET_OTHERHOST ||
	    skb->pkt_type == PACKET_LOOPBACK ||
	    arp->ar_pln != 4)
		goto freeskb;

	if ((skb = skb_share_check(skb, GFP_ATOMIC)) == NULL)
		goto out_of_mem;

	if (skb_is_nonlinear(skb)) {
		if (skb_linearize(skb, GFP_ATOMIC) != 0)
			goto freeskb;
	}

	return NF_HOOK(NF_ARP, NF_ARP_IN, skb, dev, NULL, arp_process);

freeskb:
	kfree_skb(skb);
out_of_mem:
	return 0;
}

/*
 *	User level interface (ioctl, /proc)
 */

/*
 *	Set (create) an ARP cache entry.
 */

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
int arp_req_set(struct arpreq *r, struct net_device * dev)
{
	u32 ip = ((struct sockaddr_in *) &r->arp_pa)->sin_addr.s_addr;
	struct neighbour *neigh;
	int err;

	if (r->arp_flags&ATF_PUBL) {
		u32 mask = ((struct sockaddr_in *) &r->arp_netmask)->sin_addr.s_addr;
		if (mask && mask != 0xFFFFFFFF)
			return -EINVAL;
		if (!dev && (r->arp_flags & ATF_COM)) {
			dev = dev_getbyhwaddr(r->arp_ha.sa_family, r->arp_ha.sa_data);
			if (!dev)
				return -ENODEV;
		}
		if (mask) {
			if (pneigh_lookup(&arp_tbl, &ip, dev, 1) == NULL)
				return -ENOBUFS;
			return 0;
		}
		if (dev == NULL) {
			ipv4_devconf.proxy_arp = 1;
			return 0;
		}
		if (__in_dev_get(dev)) {
			__in_dev_get(dev)->cnf.proxy_arp = 1;
			return 0;
		}
		return -ENXIO;
	}

	if (r->arp_flags & ATF_PERM)
		r->arp_flags |= ATF_COM;
	if (dev == NULL) {
		struct rtable * rt;
		if ((err = ip_route_output(&rt, ip, 0, RTO_ONLINK, 0)) != 0)
			return err;
		dev = rt->u.dst.dev;
		ip_rt_put(rt);
		if (!dev)
			return -EINVAL;
	}
	if (r->arp_ha.sa_family != dev->type)	
		return -EINVAL;

	neigh = __neigh_lookup_errno(&arp_tbl, &ip, dev);
	err = PTR_ERR(neigh);
	if (!IS_ERR(neigh)) {
		unsigned state = NUD_STALE;
		if (r->arp_flags & ATF_PERM)
			state = NUD_PERMANENT;
		err = neigh_update(neigh, (r->arp_flags&ATF_COM) ?
				   r->arp_ha.sa_data : NULL, state, 1, 0);
		neigh_release(neigh);
	}
	return err;
}

static unsigned arp_state_to_flags(struct neighbour *neigh)
{
	unsigned flags = 0;
	if (neigh->nud_state&NUD_PERMANENT)
		flags = ATF_PERM|ATF_COM;
	else if (neigh->nud_state&NUD_VALID)
		flags = ATF_COM;
	return flags;
}

/*
 *	Get an ARP cache entry.
 */

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
static int arp_req_get(struct arpreq *r, struct net_device *dev)
{
	u32 ip = ((struct sockaddr_in *) &r->arp_pa)->sin_addr.s_addr;
	struct neighbour *neigh;
	int err = -ENXIO;

	neigh = neigh_lookup(&arp_tbl, &ip, dev);
	if (neigh) {
		read_lock_bh(&neigh->lock);
		memcpy(r->arp_ha.sa_data, neigh->ha, dev->addr_len);
		r->arp_flags = arp_state_to_flags(neigh);
		read_unlock_bh(&neigh->lock);
		r->arp_ha.sa_family = dev->type;
		strncpy(r->arp_dev, dev->name, sizeof(r->arp_dev));
		neigh_release(neigh);
		err = 0;
	}
	return err;
}

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
int arp_req_delete(struct arpreq *r, struct net_device * dev)
{
	int err;
	u32 ip = ((struct sockaddr_in *)&r->arp_pa)->sin_addr.s_addr;
	struct neighbour *neigh;

	if (r->arp_flags & ATF_PUBL) {
		u32 mask = ((struct sockaddr_in *) &r->arp_netmask)->sin_addr.s_addr;
		if (mask == 0xFFFFFFFF)
			return pneigh_delete(&arp_tbl, &ip, dev);
		if (mask == 0) {
			if (dev == NULL) {
				ipv4_devconf.proxy_arp = 0;
				return 0;
			}
			if (__in_dev_get(dev)) {
				__in_dev_get(dev)->cnf.proxy_arp = 0;
				return 0;
			}
			return -ENXIO;
		}
		return -EINVAL;
	}

	if (dev == NULL) {
		struct rtable * rt;
		if ((err = ip_route_output(&rt, ip, 0, RTO_ONLINK, 0)) != 0)
			return err;
		dev = rt->u.dst.dev;
		ip_rt_put(rt);
		if (!dev)
			return -EINVAL;
	}
	err = -ENXIO;
	neigh = neigh_lookup(&arp_tbl, &ip, dev);
	if (neigh) {
		if (neigh->nud_state&~NUD_NOARP)
			err = neigh_update(neigh, NULL, NUD_FAILED, 1, 0);
		neigh_release(neigh);
	}
	return err;
}

/*
 *	Handle an ARP layer I/O control request.
 */

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
int arp_ioctl(unsigned int cmd, void *arg)
{
	int err;
	struct arpreq r;
	struct net_device * dev = NULL;

	switch(cmd) {
		case SIOCDARP:
		case SIOCSARP:
			if (!capable(CAP_NET_ADMIN))
				return -EPERM;
		case SIOCGARP:
			err = copy_from_user(&r, arg, sizeof(struct arpreq));
			if (err)
				return -EFAULT;
			break;
		default:
			return -EINVAL;
	}

	if (r.arp_pa.sa_family != AF_INET)
		return -EPFNOSUPPORT;

	if (!(r.arp_flags & ATF_PUBL) &&
	    (r.arp_flags & (ATF_NETMASK|ATF_DONTPUB)))
		return -EINVAL;
	if (!(r.arp_flags & ATF_NETMASK))
		((struct sockaddr_in *)&r.arp_netmask)->sin_addr.s_addr=__constant_htonl(0xFFFFFFFFUL);

	rtnl_lock();
	if (r.arp_dev[0]) {
		err = -ENODEV;
		if ((dev = __dev_get_by_name(r.arp_dev)) == NULL)
			goto out;

		/* Mmmm... It is wrong... ARPHRD_NETROM==0 */
		if (!r.arp_ha.sa_family)
			r.arp_ha.sa_family = dev->type;
		err = -EINVAL;
		if ((r.arp_flags & ATF_COM) && r.arp_ha.sa_family != dev->type)
			goto out;
	} else if (cmd == SIOCGARP) {
		err = -ENODEV;
		goto out;
	}

	switch(cmd) {
	case SIOCDARP:
	        err = arp_req_delete(&r, dev);
		break;
	case SIOCSARP:
		err = arp_req_set(&r, dev);
		break;
	case SIOCGARP:
		err = arp_req_get(&r, dev);
		if (!err && copy_to_user(arg, &r, sizeof(r)))
			err = -EFAULT;
		break;
	}
out:
	rtnl_unlock();
	return err;
}

/*
 *	Write the contents of the ARP cache to a PROCfs file.
 */
#ifndef CONFIG_PROC_FS
static int arp_get_info(char *buffer, char **start, off_t offset, int length) { return 0; }
#else
#if defined(CONFIG_AX25) || defined(CONFIG_AX25_MODULE)
static char *ax2asc2(ax25_address *a, char *buf);
#endif
#define HBUFFERLEN 30

static int arp_get_info(char *buffer, char **start, off_t offset, int length)
{
	int len=0;
	off_t pos=0;
	int size;
	char hbuffer[HBUFFERLEN];
	int i,j,k;
	const char hexbuf[] =  "0123456789ABCDEF";
#ifdef CONFIG_CTC_E8_CLIENT_LIMIT
	size = sprintf(buffer,"IP address       HW type   Flags   HW address          Mask  Device  Aging\n");
#else
	size = sprintf(buffer,"IP address       HW type   Flags   HW address          Mask  Device\n");
#endif
	pos+=size;
	len+=size;

	for(i=0; i<=NEIGH_HASHMASK; i++) {
		struct neighbour *n;
		read_lock_bh(&arp_tbl.lock);
		for (n=arp_tbl.hash_buckets[i]; n; n=n->next) {
			struct net_device *dev = n->dev;
			int hatype = dev->type;

			/* Do not confuse users "arp -a" with magic entries */
			if (!(n->nud_state&~NUD_NOARP))
				continue;

			read_lock(&n->lock);

/*
 *	Convert hardware address to XX:XX:XX:XX ... form.
 */
#if defined(CONFIG_AX25) || defined(CONFIG_AX25_MODULE)
			if (hatype == ARPHRD_AX25 || hatype == ARPHRD_NETROM)
				ax2asc2((ax25_address *)n->ha, hbuffer);
			else {
#endif
			for (k=0,j=0;k<HBUFFERLEN-3 && j<dev->addr_len;j++) {
				hbuffer[k++]=hexbuf[(n->ha[j]>>4)&15 ];
				hbuffer[k++]=hexbuf[n->ha[j]&15     ];
				hbuffer[k++]=':';
			}
			hbuffer[--k]=0;

#if defined(CONFIG_AX25) || defined(CONFIG_AX25_MODULE)
		}
#endif

			{
				char tbuf[16];
				sprintf(tbuf, "%u.%u.%u.%u", NIPQUAD(*(u32*)n->primary_key));
#ifdef CONFIG_CTC_E8_CLIENT_LIMIT
				size = sprintf(buffer+len, "%-16s 0x%-8x0x%-6x%s"
							"   *     %s       %u\n",
					tbuf,
					hatype,
					arp_state_to_flags(n), 
					hbuffer,
					dev->name, (jiffies-n->updated));
#else
				size = sprintf(buffer+len, "%-16s 0x%-8x0x%-6x%s"
							"   *     %s\n",
					tbuf,
					hatype,
					arp_state_to_flags(n), 
					hbuffer,
					dev->name);
#endif
			}

			read_unlock(&n->lock);

			len += size;
			pos += size;
		  
			if (pos <= offset)
				len=0;
			if (pos >= offset+length) {
				read_unlock_bh(&arp_tbl.lock);
 				goto done;
			}
		}
		read_unlock_bh(&arp_tbl.lock);
	}

	for (i=0; i<=PNEIGH_HASHMASK; i++) {
		struct pneigh_entry *n;
		for (n=arp_tbl.phash_buckets[i]; n; n=n->next) {
			struct net_device *dev = n->dev;
			int hatype = dev ? dev->type : 0;

			{
				char tbuf[16];
				sprintf(tbuf, "%u.%u.%u.%u", NIPQUAD(*(u32*)n->key));
#ifdef CONFIG_CTC_E8_CLIENT_LIMIT
				size = sprintf(buffer+len, "%-16s 0x%-8x0x%-6x%s"
							"   *     %s        *\n",
					tbuf,
					hatype,
 					ATF_PUBL|ATF_PERM,
					"00:00:00:00:00:00",
					dev ? dev->name : "*");
#else
				size = sprintf(buffer+len, "%-16s 0x%-8x0x%-6x%s"
							"   *     %s\n",
					tbuf,
					hatype,
 					ATF_PUBL|ATF_PERM,
					"00:00:00:00:00:00",
					dev ? dev->name : "*");
#endif
			}

			len += size;
			pos += size;
		  
			if (pos <= offset)
				len=0;
			if (pos >= offset+length)
				goto done;
		}
	}

done:
  
	*start = buffer+len-(pos-offset);	/* Start of wanted data */
	len = pos-offset;			/* Start slop */
	if (len>length)
		len = length;			/* Ending slop */
	if (len<0)
		len = 0;
	return len;
}
#endif

/* Note, that it is not on notifier chain.
   It is necessary, that this routine was called after route cache will be
   flushed.
 */
void arp_ifdown(struct net_device *dev)
{
	neigh_ifdown(&arp_tbl, dev);
}


/*
 *	Called once on startup.
 */

static struct packet_type arp_packet_type = {
	type:	__constant_htons(ETH_P_ARP),
	func:	arp_rcv,
	data:	(void*) 1, /* understand shared skbs */
};

#ifdef CONFIG_CTC_E8_CLIENT_LIMIT
/*
static void SendSig2Pid(void)
{
	struct task_struct *p;
	struct siginfo info;

	info.si_signo = sig;
	info.si_errno = 0;
	info.si_code = SI_USER;
	info.si_pid = current->pid;
	info.si_uid = current->uid;
	printk("send signal to user process pid=%d, info.si_pid=%d\n", clients_limit_proc_pid,);
	p = find_task_by_pid(clients_limit_proc_pid);
	send_sig_info(sig, &info, p);
}*/
static void deleteArp(u32 ip)
{
	struct neighbour *neigh;
	struct net_device *dev;
	int err;
	if(!ip)
		return ;
	dev=dev_get_by_name("br0");
	neigh = neigh_lookup(&arp_tbl, &ip, dev);
	if (neigh) {
		if (neigh->nud_state&~NUD_NOARP)
			err = neigh_update(neigh, NULL, NUD_FAILED, 1, 0);
		neigh_release(neigh);
	}
	dev_put(dev);
	printk("arp deleted\n");
	return;
}

static int typeChanged(u32 ip, enum DeviceType  curtype, enum DeviceType * newtype)

{
	struct ClientsDetect * cur_ptr_type_list=clients_type_table;
	while(cur_ptr_type_list && ip != cur_ptr_type_list->ip)
	{
		cur_ptr_type_list=cur_ptr_type_list->next;
	}
	if(cur_ptr_type_list) //founded match...
	{

		if(curtype!=cur_ptr_type_list->type)
		{
			*newtype=cur_ptr_type_list->type;
			return 1;
		}else
		return 0;
	}
	else
		return 0;
}
static int updateTypeTable(u32 ip, enum DeviceType devtype)
{
	struct ClientsDetect * cur_ptr_type_list;
	cur_ptr_type_list=clients_type_table;
	while(cur_ptr_type_list && cur_ptr_type_list->ip && cur_ptr_type_list->ip!=ip)
	{
		cur_ptr_type_list=cur_ptr_type_list->next;
	}
	if(cur_ptr_type_list)
	{		//found....
		cur_ptr_type_list->type=devtype;
	}else
	{		//not found or table empty
		cur_ptr_type_list=kmalloc(sizeof(struct ClientsDetect), GFP_KERNEL);
		if(cur_ptr_type_list==NULL)
		{
			printk("clients_type_table NULL, not enough mem\n");
			return -1;
		}
		memset(cur_ptr_type_list, 0, sizeof(struct ClientsDetect));
		cur_ptr_type_list->ip=ip;
		cur_ptr_type_list->type=devtype;
		cur_ptr_type_list->next=clients_type_table;
		clients_type_table=cur_ptr_type_list;
	}
}
static int updateDeviceType(struct ClientsDetect * clientslist, struct ClientsDetect * typelist)
{
	struct ClientsDetect * cur_ptr_type_list;
	while(clientslist)
	{
		cur_ptr_type_list=typelist;
		while(cur_ptr_type_list && clientslist->ip != cur_ptr_type_list->ip)
		{
			cur_ptr_type_list=cur_ptr_type_list->next;
		}
		if(cur_ptr_type_list) //founded match...
			clientslist->type=cur_ptr_type_list->type;
		clientslist=clientslist->next;
	}
	return 0;
}
static struct ClientsDetect * getClient(struct ClientsDetect * clientslist,  enum DeviceType type, enum CTCIdentity identity, int detected, int blocked)
{
	while(clientslist)
	{
		if(clientslist->ip && (clientslist->type==type) 
				&& (clientslist->identity==identity) && (clientslist->detected==detected)
				&& (clientslist->blocked==blocked))
			return clientslist;
		clientslist=clientslist->next;
	}
	return NULL;
}
static int blockClients(struct ClientsDetect *clients,enum DeviceType type, enum CTCIdentity identity, int maxnum)
{
	int num=0;
	while(clients)
	{
		if(!clients->blocked)
		{
			if(clients->type==type && clients->identity==identity && maxnum)
			{
				maxnum--;
				clients->blocked=CTC_BLOCKED;
				clients->identity=NORMALONE; //
				cur_out_buffer+=sprintf(cur_out_buffer, "/bin/iptables -I FORWARD -s %d.%d.%d.%d -j DROP\n", 
										clients->ip>>24&0xFF, clients->ip>>16&0xFF, 
										clients->ip>>8&0xFF, clients->ip>>0&0xFF);
				if((unsigned int)cur_out_buffer> (unsigned int)( out_buffer_base+sizeof(out_buffer_base)))
				{
					printk("blockClients:out buffer overflow\n");
					cur_out_buffer=out_buffer_base;
					return NULL;
				}
				//we will  signal to trigger user demean to process iptables at proper time.
				flag_signal=1;
				num++;
			}
		}
		clients=clients->next;
	}
	return num; //new blocked num
}
static int allowClients(struct ClientsDetect *clients,enum DeviceType type, enum CTCIdentity identity, int maxnum)
{
	int num=0;
	while(clients)
	{
		if(clients->identity==NEWONE  && maxnum && identity==NEWONE)
		{
			clients->identity=NORMALONE;
			clients->blocked=0;
			maxnum--;
			num++;
		}else if(clients->blocked) //now all is normalone...
		{
			if(clients->type==type && clients->identity==identity && maxnum)
			{
				maxnum--;
				clients->identity=NORMALONE;
				clients->blocked=0;
				cur_out_buffer+=sprintf(cur_out_buffer, "/bin/iptables -D FORWARD -s %d.%d.%d.%d -j DROP\n", 
										clients->ip>>24&0xFF, clients->ip>>16&0xFF, 
										clients->ip>>8&0xFF, clients->ip>>0&0xFF);
				if((unsigned int)cur_out_buffer> (unsigned int)( out_buffer_base+sizeof(out_buffer_base)))
				{
					printk("allowClients:out buffer overflow\n");
					cur_out_buffer=out_buffer_base;
					return NULL;
				}
				//we will  signal to trigger user demean to process iptables at proper time.
				flag_signal=1;
				num++;
			}
		}
		clients=clients->next;
	}
	return num; //new blocked num
}
static struct ClientsDetect * removeClients(struct ClientsDetect ** ptrclientslist,  enum DeviceType type, enum CTCIdentity identity, int detected, int blocked)
{
	struct ClientsDetect * prevClient=NULL;
	struct ClientsDetect * clientslist=NULL;
	if(ptrclientslist)
		clientslist=*ptrclientslist;
	while(clientslist)
	{
		if( (clientslist->type==type) 
				&& (clientslist->identity==identity) && (clientslist->detected==detected)
				&& (clientslist->blocked==blocked))
		{
			if(prevClient)
				prevClient->next=clientslist->next;
			else
				*ptrclientslist=clientslist->next;
			//delete arp info...
			deleteArp(clientslist->ip);
			// remove block rules from iptables...
			// add the command to buffer to tell user to do ipfilter. 
			if(blocked) // if blocked in iptables, we will unblock it
			{
				cur_out_buffer+=sprintf(cur_out_buffer, "/bin/iptables -D FORWARD -s %d.%d.%d.%d -j DROP\n", 
										clientslist->ip>>24&0xFF, clientslist->ip>>16&0xFF, 
										clientslist->ip>>8&0xFF, clientslist->ip>>0&0xFF);
				if((unsigned int)cur_out_buffer> (unsigned int)( out_buffer_base+sizeof(out_buffer_base)))
				{
					printk("out buffer overflow\n");
					cur_out_buffer=out_buffer_base;
					return NULL;
				}
				//we will  signal to trigger user demean to process iptables at proper time.
				flag_signal=1;
			}
			
			//?????? how to 
			kfree(clientslist);
			if(prevClient)
				clientslist=prevClient->next;
			else
				clientslist=*ptrclientslist;
		}else
		{
			prevClient=clientslist;	
			clientslist=clientslist->next;
		}
	}
	return NULL;
}
static int getClientNum(struct ClientsDetect * clientslist,  enum DeviceType type, enum CTCIdentity identity, int detected, int blocked)
{
	int num=0;
	while(clientslist)
	{
		if(clientslist->ip && (clientslist->type==type) 
				&& (clientslist->identity==identity) && (clientslist->detected==detected)
				&& (clientslist->blocked==blocked))
			num++;
		clientslist=clientslist->next;
	}
	return num;
}
static void clients_polling_handler(unsigned long dummy)
{
	struct net_device *dev;
	struct ClientsDetect * localClientsList;
	//printk("polling_handler 1\n");
	dev=dev_get_by_name("br0");
	if(!dev)
		goto back ;
	localClientsList=clients_to_detect;
	//printk("polling_handler 2 dev=%08x, clientlist=%08x\n", dev,localClientsList);	
	//send arp request to detect if all the clients are still here.......
	while(localClientsList)
	{
		u32 saddr;
		if(!localClientsList->ip)
			break;
		//reset detected flag...
		localClientsList->detected=CTC_UNDETECTED;
		saddr = inet_select_addr(dev, localClientsList->ip, RT_SCOPE_LINK);
		//printk("polling handler: sdddr is gotten");
		if(saddr)
		{
			//printk("polling handler: ready to send one arp request\n");
			arp_send(ARPOP_REQUEST, ETH_P_ARP, localClientsList->ip, dev, saddr,
					NULL, dev->dev_addr, NULL);
			//printk("polling handler one arp request sent out\n ");
		}
		else
		{
			//printk("no ip found on device\n");
		}
		localClientsList=localClientsList->next;
	}			
	dev_put(dev);
	//printk("polling handler 3:   all arp requests sent out\n ");
back:	
	//restart polling timer;
	mod_timer(&timer_client_polling, jiffies+CLIENT_POLLING_INTERVAL*HZ);
	//we will check the detect results after CLIENT_UPDATE_AFTER seconds.
	mod_timer(&timer_client_update, jiffies+CLIENT_UPDATE_AFTER*HZ);
	//printk("polling handler 4 arp requests sent out\n ");
}
static void clients_update_handler(unsigned long dummy)
{
	struct ClientsDetect * clients;
	int pcNumsAllowedDetected=0;
	int pcNumsBlockedDetected=0;
	int pcNumsAllowedUndetected=0;
	int pcNumsBlockedUndetected=0;
	
	
	int cameraNumsAllowedDetected=0;	
	int cameraNumsBlockedDetected=0;
	int cameraNumsAllowedUndetected=0;
	int cameraNumsBlockedUndetected=0;
	//printk("clients_update_handler restarted\n ");
	// check all the detection results.
	clients=clients_to_detect;
	if(!clients)
		return;
	if(lock_clientlist==1)
		return;
	lock_clientlist=1; // lock the client list structure.
	pcNumsAllowedDetected=getClientNum(clients, CTC_Computer, NORMALONE, 1, 0);
	pcNumsAllowedUndetected=getClientNum(clients, CTC_Computer, NORMALONE, 0, 0);
	pcNumsBlockedDetected=getClientNum(clients, CTC_Computer, NORMALONE, 1, 1);
	pcNumsBlockedUndetected=getClientNum(clients, CTC_Computer, NORMALONE, 0, 1);
	cameraNumsAllowedDetected=getClientNum(clients, CTC_Carema, NORMALONE, 1, 0);
	cameraNumsAllowedUndetected=getClientNum(clients, CTC_Carema, NORMALONE, 0, 0);
	cameraNumsBlockedDetected=getClientNum(clients, CTC_Carema, NORMALONE, 1, 1);
	cameraNumsBlockedUndetected=getClientNum(clients, CTC_Carema, NORMALONE, 0, 1);
	if(flagLimitOnAll == 1)
	{     //since we dont care of what the device type is ... when device type changed, no more action to be taken...
		updateDeviceType(clients_to_detect, clients_type_table);
		//printk("device type updated\n");
		if((pcNumsBlockedUndetected+cameraNumsBlockedUndetected))
		{
			//unblock the unexist devices from iptables and remove the arp entry in arp cache... 
			removeClients(&clients_to_detect, CTC_Computer, NORMALONE, 0, 1);
			removeClients(&clients_to_detect, CTC_Carema, NORMALONE, 0, 1);
			clients=clients_to_detect;
			//printk("some blocked deviced undetected\n");
			
		}
		if((pcNumsAllowedUndetected+cameraNumsAllowedUndetected))
		{
			//remove the undetected clients from tables and clear the arp cache.... 
			removeClients(&clients_to_detect, CTC_Computer, NORMALONE, 0, 0);
			removeClients(&clients_to_detect, CTC_Carema, NORMALONE, 0, 0);
			clients=clients_to_detect;
			//printk("some allowed device undetected\n");
			
		}
		if((pcNumsAllowedDetected+cameraNumsAllowedDetected)>=limitOnAll)
		{
			int blockedNum=pcNumsAllowedDetected+cameraNumsAllowedDetected-limitOnAll;
			//Camera is first blocked...
			blockClients(clients,CTC_Carema, NEWONE, MAXNUM);			
			blockClients(clients,CTC_Computer, NEWONE, MAXNUM);
			blockedNum-=blockClients(clients,CTC_Carema, NORMALONE, blockedNum);	
			blockClients(clients,CTC_Computer, NORMALONE, blockedNum);
			
		}
		if((pcNumsAllowedDetected+cameraNumsAllowedDetected)<limitOnAll)
		{
			int canAllowedNum=limitOnAll-pcNumsAllowedDetected-cameraNumsAllowedDetected;
			int allowedNum=0;
			allowedNum=allowClients(clients, CTC_Computer, NEWONE, canAllowedNum);
			canAllowedNum-=allowedNum;
			if(canAllowedNum)
			{
				allowedNum=allowClients(clients, CTC_Carema, NEWONE, canAllowedNum);
				canAllowedNum-=allowedNum;
				if(canAllowedNum)
				{
					allowedNum=allowClients(clients, CTC_Computer, NORMALONE, canAllowedNum);
					canAllowedNum-=allowedNum;
					if(canAllowedNum)
					{
						allowedNum=allowClients(clients, CTC_Carema, NORMALONE, canAllowedNum);
						canAllowedNum-=allowedNum;
					}
				}
			}
		}
		//now user will polling the command to execute.
		/*
		if(flag_signal)
		{	
			struct task_struct *p;
			struct siginfo info;

			info.si_signo = sig;
			info.si_errno = 0;
			info.si_code = SI_USER;
			info.si_pid = current->pid;
			info.si_uid = current->uid;
			printk("send signal to user process pid=%d, info.si_pid=%d\n", clients_limit_proc_pid,);
			p = find_task_by_pid(clients_limit_proc_pid);
			send_sig_info(sig, &info, p);
		}*/
	}else if(flagLimitOnAll == 2)
	{
		// Camera and Computer has different limits on each.....
		enum DeviceType newtype;
		//first process Computer devices
		clients=clients_to_detect;
		//remove the undetected clients from table, 
		//and unblocking the bloced undetected clients 
		//and clear arp cache.
		if(computerLimitEnable)
		{
			removeClients(&clients_to_detect, CTC_Computer, NORMALONE, CTC_UNDETECTED, CTC_BLOCKED);
			removeClients(&clients_to_detect, CTC_Computer, NORMALONE, CTC_UNDETECTED, CTC_UNBLOCKED);	
		}
		if(cameraLimitEnable)
		{
			removeClients(&clients_to_detect, CTC_Carema, NORMALONE, CTC_UNDETECTED, CTC_BLOCKED);
			removeClients(&clients_to_detect, CTC_Carema, NORMALONE, CTC_UNDETECTED, CTC_UNBLOCKED);
		}
		// now the clients in list is all detected
		//first check if device type changed.
		clients=clients_to_detect;
		while(clients)
		{
			if(!clients->ip)
			{
				printk("error in clients update handler\n");
				break;
			}
			if(typeChanged(clients->ip , clients->type, &newtype))
			{
				printk("type change to %d of client 0x%08x\n", newtype, clients->ip);
				if(clients->blocked==CTC_UNBLOCKED)
				{	//directly change type... the blocking or not will be determied in next steps.
					clients->type=newtype;
				}else
				{
					//firstly, leave blocking state temporary, it may be reblocked in next steps...
					cur_out_buffer+=sprintf(cur_out_buffer, "/bin/iptables -D FORWARD -s %d.%d.%d.%d -j DROP\n", 
										clients->ip>>24&0xFF, clients->ip>>16&0xFF, 
										clients->ip>>8&0xFF, clients->ip>>0&0xFF);
					clients->blocked=0;
					if((unsigned int)cur_out_buffer> (unsigned int)( out_buffer_base+sizeof(out_buffer_base)))
					{
						printk("leave blocking:out buffer overflow\n");
						cur_out_buffer=out_buffer_base;
						lock_clientlist=0; // unlock the client list structure.
						return NULL;
					}
					//we will  signal to trigger user demean to process iptables at proper time.
					flag_signal=1;
					clients->blocked=1;
					clients->type=newtype;
				}
			}
			clients=clients->next;
		}
		clients=clients_to_detect;
		///calculate the number of detected Computersize...
		pcNumsAllowedDetected=getClientNum(clients, CTC_Computer, NORMALONE, CTC_DETECTED, CTC_UNBLOCKED);
		pcNumsBlockedDetected=getClientNum(clients, CTC_Computer, NORMALONE, CTC_DETECTED, CTC_BLOCKED);
		if(computerLimitEnable&& pcNumsAllowedDetected>=limitOnComputer)
		{
			blockClients(clients,CTC_Computer, NEWONE, MAXNUM);
			blockClients(clients,CTC_Computer, NORMALONE, pcNumsAllowedDetected-limitOnComputer); 
		}else
		{
			int canAllowedNum=MAXNUM;//=limitOnComputer-pcNumsAllowedDetected;
			int allowedNum=0;
			if(computerLimitEnable)
				canAllowedNum=limitOnComputer-pcNumsAllowedDetected;
			allowedNum=allowClients(clients, CTC_Computer, NEWONE, canAllowedNum);
			canAllowedNum-=allowedNum;
			if(canAllowedNum)
			{
				allowClients(clients, CTC_Computer, NORMALONE, canAllowedNum);
			}
		}
		//////calculate the number of detected Cameras numbers...
		cameraNumsAllowedDetected=getClientNum(clients, CTC_Carema, NORMALONE,  CTC_DETECTED, CTC_UNBLOCKED);
		cameraNumsBlockedDetected=getClientNum(clients, CTC_Carema, NORMALONE, CTC_DETECTED, CTC_BLOCKED);
		if(cameraLimitEnable && cameraNumsAllowedDetected>=limitOnCamera)
		{
			blockClients(clients,CTC_Carema, NEWONE, MAXNUM);
			blockClients(clients,CTC_Carema, NORMALONE, cameraNumsAllowedDetected-limitOnComputer); 
		}else
		{
			int canAllowedNum=MAXNUM;//=limitOnCamera-cameraNumsAllowedDetected;
			int allowedNum=0;
			if(cameraLimitEnable)
				canAllowedNum=limitOnCamera-cameraNumsAllowedDetected;
			allowedNum=allowClients(clients, CTC_Carema, NEWONE, canAllowedNum);
			canAllowedNum-=allowedNum;
			if(canAllowedNum)
			{
				allowClients(clients, CTC_Carema, NORMALONE, canAllowedNum);
			}
		}
	}
		
	lock_clientlist=0; // unlock the client list structure.
}
static struct ClientsDetect * clientInList(struct ClientsDetect * clientslist, u32 ip)
{
	while(clientslist)
	{
		if(clientslist->ip==ip)
			return clientslist;
		clientslist=clientslist->next;
	}
	return NULL;
}
static int insertClient(struct ClientsDetect ** clientslist, u32 ip, 
		enum DeviceType devtype, enum CTCIdentity identity, u32 detected, u32 blocked)	
{
	struct ClientsDetect *newclient;
	if(lock_clientlist==1)
		return;
	lock_clientlist=1;
	if(!(newclient=clientInList(*clientslist, ip)))
	{
		newclient=kmalloc(sizeof(struct ClientsDetect), GFP_KERNEL);
		if(newclient==NULL)
		{
			lock_clientlist=0;
			return -1;
		}
		newclient->next=*clientslist;
		*clientslist=newclient;	
		newclient->blocked=0;
	}else	
	{
		newclient->blocked=blocked;
	}
	newclient->ip=ip;
	newclient->identity=identity;
	newclient->detected=detected;
//	newclient->blocked=blocked;
	newclient->type=devtype;
	//if(*clientslist!=NULL)
	//	newclient->next=*clientslist;
	//*clientslist=newclient;
	lock_clientlist=0;
	return 0;
}
/*
static int destroyClientsList(struct ClientsDetect ** clientslist)
{
	while(*clientslist)
	{
		struct ClientsDetect *client;
		client=*clientslist;
		*clientslist=(*clientslist)->next;
		kfree(client);
	}
	return 0;
}*/
/*
static int updateClient(struct ClientsDetect *clientslist, u32 ip)
{
	if((!clientslist) ||(!ip))
		return -1;
	while(clientslist)
	{
		struct ClientsDetect *client;
		client=clientslist;
		clientslist=(clientslist)->next;
		if(client->ip==ip)
		{
			client->detected=1; // detected...
			return 0;
		}
	}
	return -1;
}
*/

//user will read out the iptables commands to execute 
static int procfile_read(char *buffer,
	      char **buffer_location,
	      off_t offset, int buffer_length, int *eof, void *data)
{

	int procfs_buffer_size;
	int total_size=0;
	unsigned char procfs_buffer[15]; // ip format xxx.xxx.xxx.xxx
	struct ClientsDetect * localclientslist;
	char * local_buffer;
	localclientslist=clients_to_detect;	
	local_buffer=buffer;
	///////////////////////////
	//debug
	/*
	printk("procfile_read\n");
	printk("localclientslist=%08x, offset=%d\n", localclientslist, offset);
	while(localclientslist)
	{
		printk("localclients %08x\n", localclientslist->ip);
		localclientslist=localclientslist->next;
	}
	localclientslist=clients_to_detect;	*/
	/////////////////////////////
	memcpy(buffer, out_buffer_base, (unsigned)cur_out_buffer-(unsigned)out_buffer_base);
	total_size=(unsigned)cur_out_buffer-(unsigned)out_buffer_base;
	if (total_size <= offset+buffer_length) *eof = 1;
	*buffer_location = buffer + offset;
	total_size -= offset;
	if (total_size>buffer_length) total_size = buffer_length;
	if (total_size<0) total_size = 0;
	memset(out_buffer_base, 0, sizeof(out_buffer_base));
	cur_out_buffer=out_buffer_base;
	
	return total_size;
}


/*
		
 */
static int procfile_write(struct file *file, const char *buffer, unsigned long count,
		   void *data)
{
	unsigned char proc_buffer[1024];
	unsigned char *local_buffer_ptr=proc_buffer;
	char * stringptr;
	unsigned int temp_flag;
	int i=0;
	int size;
	struct net_device *netdev;
	u32 saddr;
	enum DeviceType devicetype;
	printk("proc_write count=%d\n", count);
	if(count>1024)
		return -EFAULT;
	//printk("procfile wirte: 2\n");
	memset(proc_buffer, 0, sizeof(proc_buffer));
	if(copy_from_user(local_buffer_ptr, buffer, count))
		return -EFAULT;
	//printk("procfile wirte: 2.2 count=%d\n", count);
	if(0)
	{
			int tmpcnt=0;
			while(tmpcnt<count)
			{
				printk("%c", local_buffer_ptr[tmpcnt]);
				tmpcnt++;
			}
			printk("\n");
	}
	//printk("procfile wirte: 3\n");	
	if(*local_buffer_ptr==0)
	{
		//printk("wrong file format in arp.c\n");
		return count;
	}
		//printk("procfile wirte: 5\n");
	// we pase out the possilbe command from user to change the limitnumber;
	//		"flagLimitOnAll 1(ON)/0(OFF)\n"
	//        	"limitOnAll 2\n"
	//		"limitOnComputer 3\n"
	//		"limitOnCamera 2\n"
	if(stringptr=strstr(local_buffer_ptr, "flagLimitOnAll" ))
	{
		stringptr+=sizeof("flagLimitOnAll");
		sscanf(stringptr, " %d\n", &flagLimitOnAll);
		printk("flaglimitonall changed %d\n", flagLimitOnAll);
	}
	if(stringptr=strstr(local_buffer_ptr, "limitOnAll" ))
	{
		stringptr+=sizeof("limitOnAll");
		sscanf(stringptr, " %d\n", &limitOnAll);
		printk("limitOnAll changed %d\n", limitOnAll);
	}
	if(stringptr=strstr(local_buffer_ptr, "computerLimitEnable" ))
	{
		stringptr+=sizeof("computerLimitEnable");
		sscanf(stringptr, " %d\n", &computerLimitEnable);
		printk("computerLimitEnable state: %d\n", computerLimitEnable);
	}
	if(stringptr=strstr(local_buffer_ptr, "limitOnComputer" ))
	{
		stringptr+=sizeof("limitOnComputer");
		sscanf(stringptr, " %d\n", &limitOnComputer);
		printk("limitOnComputer changed %d\n", limitOnComputer);
	}
	if(stringptr=strstr(local_buffer_ptr, "cameraLimitEnable" ))
	{
		stringptr+=sizeof("cameraLimitEnable");
		sscanf(stringptr, " %d\n", &cameraLimitEnable);
		printk("cameraLimitEnable state: %d\n", cameraLimitEnable);
	}	
	if(stringptr=strstr(local_buffer_ptr, "limitOnCamera" ))
	{
		stringptr+=sizeof("limitOnCamera");
		sscanf(stringptr, " %d\n", &limitOnCamera);
		printk("limitOnCamera changed %d\n", limitOnCamera);
	}
	// the rest are ips and types...
	// the string begins with "IP              Types\n"
	if(stringptr=strstr(local_buffer_ptr, "IP              Types\n" ))
	{
		stringptr+=sizeof("IP              Types\n")-1;
		printk("ip list contained in proc write\n");
		//jim luo get the IPs want to be detected.....
		for(i=0; i<256; i++)
		{
			int ret;
			u32 ipparts[4];
			u32 ip;
			ret=sscanf(stringptr, "%d.%d.%d.%d %d\n", 
							&ipparts[0],&ipparts[1],&ipparts[2],&ipparts[3], &devicetype);		
			//printk("ret=%d\n", ret);
			if(ret <= 0)
				break;
			ip= (ipparts[0]<<24) |(ipparts[1]<<16) |(ipparts[2]<<8) |(ipparts[3]);
			printk("ip=%08x\n", ip);
			//insert or update the device type in type table.
			updateTypeTable(ip, devicetype);
			//printk("arp.c procfile_write ip value %08x\n", ip);
			while((*stringptr != '\n') && (*stringptr != 0))
			{
				stringptr++;
			}
			if(*stringptr==0)
				break;
			stringptr++;
		}
	}else
	{
		printk("no ip list contained in proc write\n");
		return count;
	}
	
	//printk("procfile write: clients_to_detect=%08x\n", clients_to_detect);
	return count;
};

#endif


#ifdef CONFIG_DSL_CODESWAP
void __SWAP ARP_Fastpath_Init() {
	int i;
	for(i=0; i<=NEIGH_HASHMASK; i++) {
		struct neighbour *n;
		read_lock_bh(&arp_tbl.lock);
		for (n=arp_tbl.hash_buckets[i]; n; n=n->next) {
			//struct net_device *dev = n->dev;
			//int hatype = dev->type;

			/* Do not confuse users "arp -a" with magic entries */
			if (!(n->nud_state&~NUD_NOARP))
				continue;

			read_lock(&n->lock);

			/*

			n->primary_key; // IP addr
			n->ha; // mac addr
			n->flags;

			printk(KERN_INFO "IP: %08x, %d  Mac: %02x%02x-%02x%02x-%02x%02x\n",
				*(u32*)(n->primary_key), arp_state_to_flags(n), 
				n->ha[0],n->ha[1],n->ha[2],
				n->ha[3],n->ha[4],n->ha[5]);

			*/
			// check arp_state_to_flags(n), if 0 then don't addArp
#ifdef CONFIG_RTL867X_IPTABLES_FAST_PATH
			if (0!=arp_state_to_flags(n))
				rtl865x_addArp(*(u32*)(n->primary_key), n->ha, ARP_NONE);
#endif
			read_unlock(&n->lock);
		}
		read_unlock_bh(&arp_tbl.lock);
	}


	
}

#endif

void __init arp_init (void)
{
	neigh_table_init(&arp_tbl);

	dev_add_pack(&arp_packet_type);

	proc_net_create ("arp", 0, arp_get_info);

#ifdef CONFIG_SYSCTL
	neigh_sysctl_register(NULL, &arp_tbl.parms, NET_IPV4, NET_IPV4_NEIGH, "ipv4");
#endif

//added by jim luo
#ifdef CONFIG_CTC_E8_CLIENT_LIMIT
clientspath= create_proc_entry(PROCFS_NAME, 0644, NULL);
	if (clientspath == NULL) {
		remove_proc_entry(PROCFS_NAME, &proc_root);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
			PROCFS_NAME);
		return -ENOMEM;
	}

	clientspath->read_proc  = procfile_read;
	clientspath->write_proc  =  procfile_write;
	clientspath->owner 	  = NULL;
	clientspath->mode 	  = S_IFREG | S_IRUGO;
	clientspath->uid 	  = 0;
	clientspath->gid 	  = 0;
	clientspath->size 	  = 37;

	printk(KERN_INFO "/proc/%s created\n", PROCFS_NAME);	


	// init timers
	//polling timer 
	// we will stay idle for 10 seconds to let all process up....
	init_timer(&timer_client_polling);
	timer_client_polling.expires = jiffies+10*HZ;
	timer_client_polling.function =clients_polling_handler;
	add_timer(&timer_client_polling);

	init_timer(&timer_client_update);
	timer_client_update.expires = jiffies;
	timer_client_update.function =clients_update_handler;
	add_timer(&timer_client_update);
#endif
}


#ifdef CONFIG_PROC_FS
#if defined(CONFIG_AX25) || defined(CONFIG_AX25_MODULE)

/*
 *	ax25 -> ASCII conversion
 */
char *ax2asc2(ax25_address *a, char *buf)
{
	char c, *s;
	int n;

	for (n = 0, s = buf; n < 6; n++) {
		c = (a->ax25_call[n] >> 1) & 0x7F;

		if (c != ' ') *s++ = c;
	}
	
	*s++ = '-';

	if ((n = ((a->ax25_call[6] >> 1) & 0x0F)) > 9) {
		*s++ = '1';
		n -= 10;
	}
	
	*s++ = n + '0';
	*s++ = '\0';

	if (*buf == '\0' || *buf == '-')
	   return "*";

	return buf;

}

#endif
#endif
