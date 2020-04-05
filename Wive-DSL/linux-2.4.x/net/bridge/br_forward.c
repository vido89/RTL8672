/*
 *	Forwarding decision
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_forward.c,v 1.18 2008/11/27 07:55:12 ql Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/skbuff.h>
#include <linux/if_bridge.h>
#include <linux/netfilter_bridge.h>
#include "br_private.h"
#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
#include <linux/imq.h>
#include <linux/netfilter_ipv4.h>
#endif

// Kaohj
//#define RTK_8305

static inline int should_deliver(struct net_bridge_port *p, struct sk_buff *skb)
{
	if (skb->dev == p->dev ||
	    p->state != BR_STATE_FORWARDING ||
	    // Kaohj, not bridged between WAN connections [TR-068, I-122]
	    // not bridged between virtual ELAN, we have only one NIC port
	    ((skb->dev->priv_flags & IFF_DOMAIN_WAN) &&
	    (p->dev->priv_flags & IFF_DOMAIN_WAN)) ||
	    ((skb->dev->priv_flags & IFF_DOMAIN_ELAN) &&
	    (p->dev->priv_flags & IFF_DOMAIN_ELAN)))
		return 0;
	
	// Kaohj --- check for wireless bridge blocking
	//printk("wlan_blocking=%d\n", p->br->wlan_blocking);
	if (p->br->wlan_blocking != WLAN_BLOCK_DISABLE) {
		if (p->br->wlan_blocking == WLAN_BLOCK_ALL) {
			if ((skb->dev->priv_flags & IFF_DOMAIN_WLAN) ||
				(p->dev->priv_flags & IFF_DOMAIN_WLAN))
				return 0;
		}
		else if (p->br->wlan_blocking == WLAN_BLOCK_ELAN) {
			if (((skb->dev->priv_flags & IFF_DOMAIN_WLAN) ||
				(p->dev->priv_flags & IFF_DOMAIN_WLAN)) &&
				((skb->dev->priv_flags & IFF_DOMAIN_ELAN) ||
				(p->dev->priv_flags & IFF_DOMAIN_ELAN)))
				return 0;
		}
		else if (p->br->wlan_blocking == WLAN_BLOCK_WAN) {
			if (((skb->dev->priv_flags & IFF_DOMAIN_WLAN) ||
				(p->dev->priv_flags & IFF_DOMAIN_WLAN)) &&
				((skb->dev->priv_flags & IFF_DOMAIN_WAN) ||
				(p->dev->priv_flags & IFF_DOMAIN_WAN)))
				return 0;
		}
	}

#ifdef MULTICAST_FILTER
	if ((p->br->fltr_portlist_num) &&
		!memcmp(skb->mac.ethernet->h_dest, "\x01\x00\x5e", 3))
	{
		int i, filter = 0;
		unsigned short port = *((unsigned short *)&(skb->data[22]));
		unsigned short frag_offset = *((unsigned short *)&(skb->data[6]));
		unsigned long x;

		if((frag_offset & 0x1fff) ==0){
			for (i=0; i<p->br->fltr_portlist_num; i++) {
				if (port == p->br->fltr_portlist[i]) {
					filter = 1;
					break;
				}
			}
		}

		x = skb->mac.ethernet->h_dest[3] ^ skb->mac.ethernet->h_dest[4] ^ skb->mac.ethernet->h_dest[5];
		x = x & (MLCST_MAC_ENTRY - 1);

		if (!strcmp(p->dev->name, "wlan0") ||
			!strcmp(p->dev->name, "wlan1") ||
			!strcmp(p->dev->name, "wlan2"))
		{
			if (filter) {
				if (p->br->fltr_maclist[x][3] == 0) {
					memcpy(&(p->br->fltr_maclist[x][0]), &(skb->mac.ethernet->h_dest[3]), 3);
					p->br->fltr_maclist[x][3] = 1;
				}
				return 0;
			}
			else {
				if ((p->br->fltr_maclist[x][3] != 0) &&
					!memcmp(&(p->br->fltr_maclist[x][0]), &(skb->mac.ethernet->h_dest[3]), 3)){
					return 0;
				}
				else
					return 1;
			}
		}
		else
			return 1;
	}
	else
		return 1;
#else
	return 1;
#endif
}

#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
extern int enable_imq;
/*
*ql_xu: bridge packet will not go through IP_POST_ROUTING chain, now we need qos on bridge&route,
* so we have to redirect bridge packet to IMQ.
*/
int br_dev_queue_xmit(struct sk_buff *skb)
{
	dev_queue_xmit(skb);

	return 0;
}
#endif

int br_dev_queue_push_xmit(struct sk_buff *skb)
{
#ifdef CONFIG_NETFILTER
	nf_bridge_maybe_copy_header(skb);
#endif
	skb_push(skb, ETH_HLEN);

#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
	/*
	* ql_xu: for upstream bridge packet, all redirect to IMQ; while downstream transmit directly.
	*/
	if (enable_imq) {
		if (skb->dev->priv_flags==IFF_DOMAIN_WAN) {
			skb->imq_br 	= 1;
			skb->imq_flags |= IMQ_F_ENQUEUE;
			//skb->nfcache |= NFC_ALTERED;
			
			NF_HOOK_THRESH(PF_INET, NF_IP_POST_ROUTING, skb, NULL, skb->dev, 
				br_dev_queue_xmit, NF_IP_PRI_IMQ_ENQ);
		} else
	dev_queue_xmit(skb);
	} else
		dev_queue_xmit(skb);
#else
	dev_queue_xmit(skb);
#endif

	return 0;
}

int br_forward_finish(struct sk_buff *skb)
{
	NF_HOOK(PF_BRIDGE, NF_BR_POST_ROUTING, skb, NULL, skb->dev,
			br_dev_queue_push_xmit);

	return 0;
}

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
static void __br_deliver(struct net_bridge_port *to, struct sk_buff *skb)
{
	skb->dev = to->dev;
	// Kaohj, bridged PPPoE
	if (to->dev->brpppoe && !(skb->mac.ethernet->h_proto == 0x8863 ||
		skb->mac.ethernet->h_proto == 0x8864)) {
		kfree_skb(skb);
		return;
	}
#ifdef CONFIG_NETFILTER_DEBUG
	skb->nf_debug = 0;
#endif
	NF_HOOK(PF_BRIDGE, NF_BR_LOCAL_OUT, skb, NULL, skb->dev,
			br_forward_finish);
}

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
static void __br_forward(struct net_bridge_port *to, struct sk_buff *skb)
{
	struct net_device *indev;

	// Kaohj, bridged PPPoE
	if (to->dev->brpppoe && !(skb->mac.ethernet->h_proto == 0x8863 ||
		skb->mac.ethernet->h_proto == 0x8864)) {
		kfree_skb(skb);
		return;
	}
	indev = skb->dev;
	skb->dev = to->dev;

	NF_HOOK(PF_BRIDGE, NF_BR_FORWARD, skb, indev, skb->dev,
			br_forward_finish);
}

/* called under bridge lock */
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
void br_deliver(struct net_bridge_port *to, struct sk_buff *skb)
{
	if (should_deliver(to, skb)) {
		__br_deliver(to, skb);
		return;
	}

	kfree_skb(skb);
}

/* called under bridge lock */
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
void br_forward(struct net_bridge_port *to, struct sk_buff *skb)
{
	if (should_deliver(to, skb)) {
		__br_forward(to, skb);
		return;
	}

	kfree_skb(skb);
}

/* called under bridge lock */
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
static void br_flood(struct net_bridge *br, struct sk_buff *skb, int clone,
	void (*__packet_hook)(struct net_bridge_port *p, struct sk_buff *skb))
{
	struct net_bridge_port *p;
	struct net_bridge_port *prev;
#ifdef RTK_8305
	// Kaohj, send to eth port only once
	int eth_been_sent;
#endif

	if (clone) {
		struct sk_buff *skb2;

		if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
			br->statistics.tx_dropped++;
			return;
		}

		skb = skb2;
	}

	prev = NULL;

	// Kaohj
#ifdef RTK_8305
	eth_been_sent = 0;
#endif
	p = br->port_list;
	while (p != NULL) {
		// Kaohj --- block multicast traffic from pvc to pvc
		// from_dev: put at br2684.c->br2684_push() to keep the device info we are arriving on
		// For routing case. The multicast source comes from pvc0(routing interface) and will
		// be flooded to pvc1(bridging interface).
		// Purpose: preventing multicast traffic from WAN (pvc) to another WAN (pvc)
		if (skb->from_dev && (skb->from_dev->priv_flags &
		 p->dev->priv_flags & IFF_DOMAIN_WAN)) {
		 	p = p->next;
		 	continue;
		}
#ifdef RTK_8305
		// Kaohj, send to eth port once only (in 8305 case)
		if (p->dev->priv_flags == IFF_DOMAIN_ELAN) {
			if (eth_been_sent) {
				p = p->next;
				continue;
			}
			else
				eth_been_sent = 1;
		}
#endif
		
		if (should_deliver(p, skb)) {
			if (prev != NULL) {
				struct sk_buff *skb2;

				if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
					br->statistics.tx_dropped++;
					kfree_skb(skb);
					return;
				}

				__packet_hook(prev, skb2);
			}

			prev = p;
		}

		p = p->next;
	}

	if (prev != NULL) {
		__packet_hook(prev, skb);
		return;
	}

	kfree_skb(skb);
}

/* called under bridge lock */
void br_flood_deliver(struct net_bridge *br, struct sk_buff *skb, int clone)
{
	br_flood(br, skb, clone, __br_deliver);
}

/* called under bridge lock */
void br_flood_forward(struct net_bridge *br, struct sk_buff *skb, int clone)
{
	br_flood(br, skb, clone, __br_forward);
}
