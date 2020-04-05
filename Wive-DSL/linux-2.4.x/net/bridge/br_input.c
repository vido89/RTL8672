/*
 *	Handle incoming frames
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_input.c,v 1.15 2008/02/28 09:16:11 alex Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/if_bridge.h>
#include <linux/netfilter_bridge.h>
#include "br_private.h"
#ifdef CONFIG_PPPOE_PROXY
#include <linux/config.h>
#include <linux/ppp_defs.h>
#include  <linux/if_ppp.h>
#endif

unsigned char bridge_ula[6] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x00 };

static int br_pass_frame_up_finish(struct sk_buff *skb)
{
#ifdef CONFIG_NETFILTER_DEBUG
	skb->nf_debug = 0;
#endif
	netif_rx(skb);

	return 0;
}

static void br_pass_frame_up(struct net_bridge *br, struct sk_buff *skb)
{
	struct net_device *indev;

	br->statistics.rx_packets++;
	br->statistics.rx_bytes += skb->len;

	// Kaohj -- added for ip_tables.c checking (checking interfaces under br0)
	if (skb->switch_port == 0)
		skb->switch_port = skb->dev->name;
	indev = skb->dev;
	skb->dev = &br->dev;
	skb->pkt_type = PACKET_HOST;
	skb_push(skb, ETH_HLEN);
	skb->protocol = eth_type_trans(skb, &br->dev);

	NF_HOOK(PF_BRIDGE, NF_BR_LOCAL_IN, skb, indev, NULL,
			br_pass_frame_up_finish);
}

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
int br_handle_frame_finish(struct sk_buff *skb)
{
	struct net_bridge *br;
	unsigned char *dest;
	struct net_bridge_fdb_entry *dst;
	struct net_bridge_port *p;
	int passedup;

	dest = skb->mac.ethernet->h_dest;

	p = skb->dev->br_port;
	if (p == NULL)
		goto err_nolock;

	br = p->br;
	read_lock(&br->lock);
	if (skb->dev->br_port == NULL)
		goto err;

	passedup = 0;
	if (br->dev.flags & IFF_PROMISC) {
		struct sk_buff *skb2;

		skb2 = skb_clone(skb, GFP_ATOMIC);
		if (skb2 != NULL) {
			passedup = 1;
			br_pass_frame_up(br, skb2);
		}
	}

	if (dest[0] & 1) {
#ifdef  CONFIG_PPPOE_PROXY
   if(pppoe_proxy_enabled)
    {
    if(!((skb->mac.ethernet->h_proto==0x8863||skb->mac.ethernet->h_proto==0x8864)&&(skb->mac.ethernet->h_dest[0]==0xFF&&
       skb->mac.ethernet->h_dest[1]==0xFF&&skb->mac.ethernet->h_dest[2]==0xFF&&skb->mac.ethernet->h_dest[3]==0xFF&&
       skb->mac.ethernet->h_dest[4]==0xFF&&skb->mac.ethernet->h_dest[5]==0xFF)&&(skb->data[1]==0x09)))			
	br_flood_forward(br, skb, !passedup);
   }
   else 
        br_flood_forward(br, skb, !passedup);	   
#else

       br_flood_forward(br, skb, !passedup);
#endif

if(!passedup){
#ifdef MULTICAST_FILTER
			if ((br->fltr_portlist_num) &&
				!memcmp(dest, "\x01\x00\x5e", 3))
			{
				int i, pass_up = 1;
				unsigned short port = *((unsigned short *)&(skb->data[22]));
				unsigned short frag_offset = *((unsigned short *)&(skb->data[6]));
				unsigned long x;

				if((frag_offset & 0x1fff) ==0){
					for (i=0; i<p->br->fltr_portlist_num; i++) {
						if (port == p->br->fltr_portlist[i]) {
							pass_up = 0;
							break;
						}
					}
				}

				x = dest[3] ^ dest[4] ^ dest[5];
				x = x & (MLCST_MAC_ENTRY - 1);

				if (pass_up) {
					if ((br->fltr_maclist[x][3] != 0) &&
						!memcmp(&(br->fltr_maclist[x][0]), &(dest[3]), 3))
						kfree_skb(skb);
					else if(skb->dev->priv_flags == IFF_DOMAIN_WAN && dest[0]!=0xff)
						kfree_skb(skb);
					else
						br_pass_frame_up(br, skb);
				}
				else {
					kfree_skb(skb);
					if (br->fltr_maclist[x][3] == 0) {
						memcpy(&(br->fltr_maclist[x][0]), &(dest[3]), 3);
						br->fltr_maclist[x][3] = 1;
					}
				}
			}
			else if(skb->dev->priv_flags == IFF_DOMAIN_WAN && dest[0]!=0xff)
				kfree_skb(skb);
			else
				br_pass_frame_up(br, skb);
		}
#else
			if(skb->dev->priv_flags == IFF_DOMAIN_WAN && dest[0]!=0xff)
				kfree_skb(skb);
			else
				br_pass_frame_up(br, skb);
		}
#endif
		goto out;
	}

	dst = br_fdb_get(br, dest);
	if (dst != NULL && dst->is_local) {
		if (!passedup)
			br_pass_frame_up(br, skb);
		else
			kfree_skb(skb);
		br_fdb_put(dst);
		goto out;
	}

	if (dst != NULL) {
		br_forward(dst->dst, skb);
		br_fdb_put(dst);
		goto out;
	}

	br_flood_forward(br, skb, 0);

out:
	read_unlock(&br->lock);
	return 0;

err:
	read_unlock(&br->lock);
err_nolock:
	kfree_skb(skb);
	return 0;
}

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
int br_handle_frame(struct sk_buff *skb)
{
	struct net_bridge *br;
	unsigned char *dest;
	struct net_bridge_port *p;

	dest = skb->mac.ethernet->h_dest;

	p = skb->dev->br_port;
	if (p == NULL)
		goto err_nolock;

	br = p->br;
	read_lock(&br->lock);
	if (skb->dev->br_port == NULL)
		goto err;

	if (!(br->dev.flags & IFF_UP) ||
	    p->state == BR_STATE_DISABLED)
		goto err;

	if (skb->mac.ethernet->h_source[0] & 1)
		goto err;

        /* Pause frames shouldn't be passed up by driver anyway */
        if (skb->protocol == htons(ETH_P_PAUSE))
                       goto err_nolock;

	if (p->state == BR_STATE_LEARNING ||
	    p->state == BR_STATE_FORWARDING)
		// Kao added, if only one port, not learning ...
		if (br->port_list != NULL && br->port_list->next != NULL) {
			//10/26/05' hrchen, add sMAC check to avoid MAC conflict bug
			if (memcmp(skb->mac.ethernet->h_source, skb->dev->dev_addr, skb->dev->addr_len))
				br_fdb_insert(br, p, skb->mac.ethernet->h_source, 0);
		}

	if (br->stp_enabled &&
	    !memcmp(dest, bridge_ula, 5) &&
	    !(dest[5] & 0xF0))
		goto handle_special_frame;

	if (p->state == BR_STATE_FORWARDING) {
 		if (br_should_route_hook && br_should_route_hook(&skb)) {
 			read_unlock(&br->lock);
 			return -1;
 		}
 
 		if (!memcmp(p->br->dev.dev_addr, dest, ETH_ALEN))
 			skb->pkt_type = PACKET_HOST;

#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE) || defined(CONFIG_EXT_SWITCH) || defined(CONFIG_EXT_SWITCH_MODULE)
                 if(eth_hdr(skb)->h_proto == htons(ETH_P_8021Q)) // Block vlan packet
                     {
                         read_unlock(p->br->lock);
                       return 0;                       // Forward to normal path
                     }
#endif 
		NF_HOOK(PF_BRIDGE, NF_BR_PRE_ROUTING, skb, skb->dev, NULL,
			br_handle_frame_finish);
		read_unlock(&br->lock);
		return 0;
	}

err:
	read_unlock(&br->lock);
err_nolock:
	kfree_skb(skb);
	return 0;

handle_special_frame:
	if (!dest[5]) {
		br_stp_handle_bpdu(skb);
		read_unlock(&br->lock);
		return 0;
	}

	read_unlock(&br->lock);
	kfree_skb(skb);
 	return 0;
}
