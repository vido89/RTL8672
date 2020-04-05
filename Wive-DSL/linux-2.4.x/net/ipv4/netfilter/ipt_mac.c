/* Kernel module to match MAC address parameters. */
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/if_ether.h>

#include <linux/netfilter_ipv4/ipt_mac.h>
#include <linux/netfilter_ipv4/ip_tables.h>

static int
match(const struct sk_buff *skb,
      const struct net_device *in,
      const struct net_device *out,
      const void *matchinfo,
      int offset,
      const void *hdr,
      u_int16_t datalen,
      int *hotdrop)
{
    const struct ipt_mac_info *info = matchinfo;
    int i, invert;
    unsigned char info_addr[ETH_ALEN];
    unsigned char pkt_addr[ETH_ALEN];

	if (info->flags & MAC_SRC) {
		invert = !!(info->flags & MAC_SRC_INV);
		if (info->flags & SRC_MASK) { // with src-mask
			for (i=0; i<ETH_ALEN; i++) {
				info_addr[i] = (info->srcaddr[i] & info->srcmask[i]);
				pkt_addr[i] = (skb->mac.ethernet->h_source[i] & info->srcmask[i]);
			}
			if (skb->mac.raw < skb->head
				|| (skb->mac.raw + ETH_HLEN) > skb->data
				|| ((memcmp(pkt_addr, info_addr, ETH_ALEN)
				!= 0) ^ invert))
				return 0;
		}
		else {
			if (skb->mac.raw < skb->head
				|| (skb->mac.raw + ETH_HLEN) > skb->data
				|| ((memcmp(skb->mac.ethernet->h_source, info->srcaddr, ETH_ALEN)
				!= 0) ^ invert))
				return 0;
		}
	}
	if (info->flags & MAC_DST) {
		invert = !!(info->flags & MAC_DST_INV);
		if (info->flags & DST_MASK) { // with dst-mask
			for (i=0; i<ETH_ALEN; i++) {
				info_addr[i] = (info->dstaddr[i] & info->dstmask[i]);
				pkt_addr[i] = (skb->mac.ethernet->h_dest[i] & info->dstmask[i]);
			}
			if (skb->mac.raw < skb->head
				|| (skb->mac.raw + ETH_HLEN) > skb->data
				|| ((memcmp(pkt_addr, info_addr, ETH_ALEN)
				!= 0) ^ invert))
				return 0;
		}
		else {
			if (skb->mac.raw < skb->head
				|| (skb->mac.raw + ETH_HLEN) > skb->data
				|| ((memcmp(skb->mac.ethernet->h_dest, info->dstaddr, ETH_ALEN)
				!= 0) ^ invert))
				return 0;
		}
	}
	return 1;
#if 0
    // Modified by Mason Yu for dst MAC
    if (   ( info->srcaddr[ETH_ALEN-1] == 0 && info->srcaddr[ETH_ALEN-2] == 0 && info->srcaddr[ETH_ALEN-3] == 0
          && info->srcaddr[ETH_ALEN-4] == 0 && info->srcaddr[ETH_ALEN-5] == 0 && info->srcaddr[ETH_ALEN-6] == 0  )
       ||  ( info->dstaddr[ETH_ALEN-1] == 0 && info->dstaddr[ETH_ALEN-2] == 0 && info->dstaddr[ETH_ALEN-3] == 0
          && info->dstaddr[ETH_ALEN-4] == 0 && info->dstaddr[ETH_ALEN-5] == 0 && info->dstaddr[ETH_ALEN-6] == 0  ) ) {
    	/* Is mac pointer valid? */
    	return (skb->mac.raw >= skb->head
	    && (skb->mac.raw + ETH_HLEN) <= skb->data
	    /* If so, compare... */
	    && (((memcmp(skb->mac.ethernet->h_source, info->srcaddr, ETH_ALEN) 
		== 0) ^ info->invert) ||
	     // Added by Mason Yu for dst MAC
	     ((memcmp(skb->mac.ethernet->h_dest, info->dstaddr, ETH_ALEN)
		== 0) ^ info->invert))
	     );	
    	
    }else {
    	/* Is mac pointer valid? */
    	return (skb->mac.raw >= skb->head
	    && (skb->mac.raw + ETH_HLEN) <= skb->data
	    /* If so, compare... */
	    && ((memcmp(skb->mac.ethernet->h_source, info->srcaddr, ETH_ALEN)
		== 0) ^ info->invert)
	    // Added by Mason Yu for dst MAC
	    && ((memcmp(skb->mac.ethernet->h_dest, info->dstaddr, ETH_ALEN)
		== 0) ^ info->invert)
	     );	
    }	
#endif	
    
#if 0    
    /* Is mac pointer valid? */
    return (skb->mac.raw >= skb->head
	    && (skb->mac.raw + ETH_HLEN) <= skb->data
	    /* If so, compare... */
	    && ((memcmp(skb->mac.ethernet->h_source, info->srcaddr, ETH_ALEN)
		== 0) ^ info->invert)	    
	     );
#endif    	     
	     
}

static int
ipt_mac_checkentry(const char *tablename,
		   const struct ipt_ip *ip,
		   void *matchinfo,
		   unsigned int matchsize,
		   unsigned int hook_mask)
{
	/* FORWARD isn't always valid, but it's nice to be able to do --RR */
	if (hook_mask
	    & ~((1 << NF_IP_PRE_ROUTING) | (1 << NF_IP_LOCAL_IN)
		| (1 << NF_IP_FORWARD))) {
		printk("ipt_mac: only valid for PRE_ROUTING, LOCAL_IN or FORWARD.\n");
		return 0;
	}

	if (matchsize != IPT_ALIGN(sizeof(struct ipt_mac_info)))
		return 0;

	return 1;
}

static struct ipt_match mac_match
= { { NULL, NULL }, "mac", &match, &ipt_mac_checkentry, NULL, THIS_MODULE };

static int __init init(void)
{
	return ipt_register_match(&mac_match);
}

static void __exit fini(void)
{
	ipt_unregister_match(&mac_match);
}

module_init(init);
module_exit(fini);
MODULE_LICENSE("GPL");
