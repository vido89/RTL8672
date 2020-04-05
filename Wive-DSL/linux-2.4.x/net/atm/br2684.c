/*
Experimental ethernet netdevice using ATM AAL5 as underlying carrier
(RFC1483 obsoleted by RFC2684) for Linux 2.4
Author: Marcell GAL, 2000, XDSL Ltd, Hungary
*/

#include <linux/module.h>
#include <linux/config.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <linux/rtnetlink.h>
#include <linux/ip.h>
#include <asm/uaccess.h>
#include <net/arp.h>

#include <linux/atmbr2684.h>

#include "ipcommon.h"
#ifdef	CONFIG_RTL867X_IPTABLES_FAST_PATH
#include "../ipv4/fastpath/fastpath_core.h"
int pppoe_fastpath(struct sk_buff *skb);
#endif

/*
 * Define this to use a version of the code which interacts with the higher
 * layers in a more intellegent way, by always reserving enough space for
 * our header at the begining of the packet.  However, there may still be
 * some problems with programs like tcpdump.  In 2.5 we'll sort out what
 * we need to do to get this perfect.  For now we just will copy the packet
 * if we need space for the header
 */
/* #define FASTER_VERSION */

#if 0
//#define DPRINTK(format, args...) printk(KERN_DEBUG "br2684: " format, ##args)
#define DPRINTK(format, args...) printk("br2684: " format, ##args)
#else
#define DPRINTK(format, args...)
#endif

//#ifdef SKB_DEBUG
#if 0
static void skb_debug(const struct sk_buff *skb)
{
#define NUM2PRINT 50
	char buf[NUM2PRINT * 3 + 1];	/* 3 chars per byte */
	int i = 0;
	for (i = 0; i < skb->len && i < NUM2PRINT; i++) {
		sprintf(buf + i * 3, "%2.2x ", 0xff & skb->data[i]);
	}
	//printk(KERN_DEBUG "br2684: skb: %s\n", buf);
	printk("br2684: skb: %s\n", buf);
}
#else
#define skb_debug(skb)	do {} while (0)
#endif

static unsigned char llc_oui_pid_pad[] =
    { 0xAA, 0xAA, 0x03, 0x00, 0x80, 0xC2, 0x00, 0x07, 0x00, 0x00 };

static unsigned char llc_oui_pid_pad_ip[] =
    { 0xAA, 0xAA, 0x03, 0x00, 0x00, 0x00, 0x08, 0x00 };

#define PADLEN	(2)

enum br2684_encaps {
	e_vc  = BR2684_ENCAPS_VC,
	e_llc = BR2684_ENCAPS_LLC,
	r_vc  = BR2684_ENCAPS_VC_IP,
	r_llc = BR2684_ENCAPS_LLC_IP,
};

struct br2684_vcc {
	struct atm_vcc  *atmvcc;
	struct br2684_dev *brdev;
	/* keep old push,pop functions for chaining */
	void (*old_push)(struct atm_vcc *vcc,struct sk_buff *skb);
	/* void (*old_pop)(struct atm_vcc *vcc,struct sk_buff *skb); */
	enum br2684_encaps encaps;
	struct list_head brvccs;
#ifdef CONFIG_ATM_BR2684_IPFILTER
	struct br2684_filter filter;
#endif /* CONFIG_ATM_BR2684_IPFILTER */
#ifndef FASTER_VERSION
	unsigned copies_needed, copies_failed;
#endif /* FASTER_VERSION */
};

struct br2684_dev {
	struct net_device net_dev;
	struct list_head br2684_devs;
	int number;
	struct list_head brvccs; /* one device <=> one vcc (before xmas) */
	struct net_device_stats stats;
	int mac_was_set;
};

/*
 * This lock should be held for writing any time the list of devices or
 * their attached vcc's could be altered.  It should be held for reading
 * any time these are being queried.  Note that we sometimes need to
 * do read-locking under interrupt context, so write locking must block
 * the current CPU's interrupts
 */
static rwlock_t devs_lock = RW_LOCK_UNLOCKED;

static LIST_HEAD(br2684_devs);

static inline struct br2684_dev *BRPRIV(const struct net_device *net_dev)
{
	return (struct br2684_dev *) ((char *) (net_dev) -
	    (unsigned long) (&((struct br2684_dev *) 0)->net_dev));
}

static inline struct br2684_dev *list_entry_brdev(const struct list_head *le)
{
	return list_entry(le, struct br2684_dev, br2684_devs);
}

static inline struct br2684_vcc *BR2684_VCC(const struct atm_vcc *atmvcc)
{
	return (struct br2684_vcc *) (atmvcc->user_back);
}

static inline struct br2684_vcc *list_entry_brvcc(const struct list_head *le)
{
	return list_entry(le, struct br2684_vcc, brvccs);
}

/* Caller should hold read_lock(&devs_lock) */
static struct br2684_dev *br2684_find_dev(const struct br2684_if_spec *s)
{
	struct list_head *lh;
	struct br2684_dev *brdev;
	switch (s->method) {
	case BR2684_FIND_BYNUM:
		list_for_each(lh, &br2684_devs) {
			brdev = list_entry_brdev(lh);
			if (brdev->number == s->spec.devnum)
				return brdev;
		}
		break;
	case BR2684_FIND_BYIFNAME:
		list_for_each(lh, &br2684_devs) {
			brdev = list_entry_brdev(lh);
			if (!strncmp(brdev->net_dev.name, s->spec.ifname,
			    sizeof brdev->net_dev.name))
				return brdev;
		}
		break;
	}
	return NULL;
}

/*
 * Send a packet out a particular vcc.  Not to useful right now, but paves
 * the way for multiple vcc's per itf.  Returns true if we can send,
 * otherwise false
 */
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
static int br2684_xmit_vcc(struct sk_buff *skb, struct br2684_dev *brdev,
	struct br2684_vcc *brvcc)
{
	struct atm_vcc *atmvcc;
	int minheadroom;
#ifdef FASTER_VERSION
	if (brvcc->encaps == e_llc)
		memcpy(skb_push(skb, 8), llc_oui_pid_pad, 8);
	/* last 2 bytes of llc_oui_pid_pad are managed by header routines;
	   yes, you got it: 8 + 2 = sizeof(llc_oui_pid_pad)
	 */
#else

	switch(brvcc->encaps) {
	case e_llc:
		minheadroom = 10;
		if (skb_headroom(skb) < minheadroom) {
			struct sk_buff *skb2 = skb_realloc_headroom(skb, minheadroom);
			brvcc->copies_needed++;
			dev_kfree_skb(skb);
			if (skb2 == NULL) {
				brvcc->copies_failed++;
				return 0;
			}
			skb = skb2;
		}
		// Kaohj --- add padding for Ethernet packet less than 60
		if (skb->len < 60) {
			int put_len;
			struct sk_buff 	*tmp_skb;
			
			put_len = 60-skb->len;
			if (put_len > skb_tailroom(skb)) {
				if ((tmp_skb = dev_alloc_skb(128)) == NULL) {
					/* Free dropped skb */
					printk("\nbr2684 alloc skb failed!\n");
					dev_kfree_skb(skb); /* should not be here */
					return 0;
				}
				skb_reserve(tmp_skb, minheadroom);
				memcpy(skb_put(tmp_skb, skb->len), skb->data, skb->len);
				dev_kfree_skb(skb);
				skb = tmp_skb;
			}
			memset(skb->data+skb->len, 0, put_len);
			skb_put(skb, put_len);
		}
		skb_push(skb, minheadroom);
		memcpy(skb->data, llc_oui_pid_pad, 10);
		break;
	case e_vc:
		minheadroom = 2;
		if (skb_headroom(skb) < minheadroom) {
			struct sk_buff *skb2 = skb_realloc_headroom(skb, minheadroom);
			brvcc->copies_needed++;
			dev_kfree_skb(skb);
			if (skb2 == NULL) {
				brvcc->copies_failed++;
				return 0;
			}
			skb = skb2;
		}
		// Kaohj --- add padding for Ethernet packet less than 60
		if (skb->len < 60) {
			int put_len;
			struct sk_buff 	*tmp_skb;
			
			put_len = 60-skb->len;
			if (put_len > skb_tailroom(skb)) {
				if ((tmp_skb = dev_alloc_skb(128)) == NULL) {
					/* Free dropped skb */
					printk("\nbr2684 alloc skb failed!\n");
					dev_kfree_skb(skb); /* should not be here */
					return 0;
				}
				skb_reserve(tmp_skb, minheadroom);
				memcpy(skb_put(tmp_skb, skb->len), skb->data, skb->len);
				dev_kfree_skb(skb);
				skb = tmp_skb;
			}
			memset(skb->data+skb->len, 0, put_len);
			skb_put(skb, put_len);
			
		}
		skb_push(skb, minheadroom);
		memset(skb->data, 0, 2);
		break;	
	case r_llc:
		minheadroom = 6;
		skb_pull(skb, minheadroom);
		memcpy(skb->data, llc_oui_pid_pad_ip, 6);
		break;
	case r_vc:
		// Mason Yu. Do not add ether type (0x0800) in the front of Paket.
		// Just add IP header in the front of the packet.
		//minheadroom = 12;
		minheadroom = 14;	
		skb_pull(skb, minheadroom);
		break;
	default:
		dev_kfree_skb(skb);
		return 0;
	}
		

#endif /* FASTER_VERSION */
	skb_debug(skb);

	ATM_SKB(skb)->vcc = atmvcc = brvcc->atmvcc;
	DPRINTK("atm_skb(%p)->vcc(%p)->dev(%p)\n", skb, atmvcc, atmvcc->dev);
	if (!atm_may_send(atmvcc, skb->truesize)) {
		/* we free this here for now, because we cannot know in a higher 
			layer whether the skb point it supplied wasn't freed yet.
			now, it always is.
		*/

		dev_kfree_skb(skb);
		return 0;
	}
	atomic_add(skb->truesize, &atmvcc->tx_inuse);
	ATM_SKB(skb)->iovcnt = 0;
	ATM_SKB(skb)->atm_options = atmvcc->atm_options;
	brdev->stats.tx_packets++;
	brdev->stats.tx_bytes += skb->len;
	atmvcc->send(atmvcc, skb);
	return 1;
}

static inline struct br2684_vcc *pick_outgoing_vcc(struct sk_buff *skb,
	struct br2684_dev *brdev)
{
	return list_empty(&brdev->brvccs) ? NULL :
	    list_entry_brvcc(brdev->brvccs.next); /* 1 vcc/dev right now */
}

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
static int br2684_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct br2684_dev *brdev = BRPRIV(dev);
	struct br2684_vcc *brvcc;

	DPRINTK("br2684_start_xmit, skb->dst=%p\n", skb->dst);
	read_lock(&devs_lock);
	brvcc = pick_outgoing_vcc(skb, brdev);
	if (brvcc == NULL) {
		DPRINTK("no vcc attached to dev %s\n", dev->name);
		brdev->stats.tx_errors++;
		brdev->stats.tx_carrier_errors++;
		/* netif_stop_queue(dev); */
		dev_kfree_skb(skb);
		read_unlock(&devs_lock);
		// Modified bu Mason Yu. It will free 2 SKB for qdisc_restart().
		//return -EUNATCH;
		return 0;
	}
	if (!br2684_xmit_vcc(skb, brdev, brvcc)) {
		/*
		 * We should probably use netif_*_queue() here, but that
		 * involves added complication.  We need to walk before
		 * we can run
		 */
		/* don't free here! this pointer might be no longer valid!
		dev_kfree_skb(skb);
		*/
		brdev->stats.tx_errors++;
		brdev->stats.tx_fifo_errors++;
	}
	read_unlock(&devs_lock);
	return 0;
}

static struct net_device_stats *br2684_get_stats(struct net_device *dev)
{
	DPRINTK("br2684_get_stats\n");
	return &BRPRIV(dev)->stats;
}

#ifdef FASTER_VERSION
/*
 * These mirror eth_header and eth_header_cache.  They are not usually
 * exported for use in modules, so we grab them from net_device
 * after ether_setup() is done with it.  Bit of a hack.
 */
static int (*my_eth_header)(struct sk_buff *, struct net_device *,
	unsigned short, void *, void *, unsigned);
static int (*my_eth_header_cache)(struct neighbour *, struct hh_cache *);

static int
br2684_header(struct sk_buff *skb, struct net_device *dev,
	      unsigned short type, void *daddr, void *saddr, unsigned len)
{
	u16 *pad_before_eth;
	int t = my_eth_header(skb, dev, type, daddr, saddr, len);
	if (t > 0) {
		pad_before_eth = (u16 *) skb_push(skb, 2);
		*pad_before_eth = 0;
		return dev->hard_header_len;	/* or return 16; ? */
	} else
		return t;
}

static int
br2684_header_cache(struct neighbour *neigh, struct hh_cache *hh)
{
/* hh_data is 16 bytes long. if encaps is ether-llc we need 24, so
xmit will add the additional header part in that case */
	u16 *pad_before_eth = (u16 *)(hh->hh_data);
	int t = my_eth_header_cache(neigh, hh);
	DPRINTK("br2684_header_cache, neigh=%p, hh_cache=%p\n", neigh, hh);
	if (t < 0)
		return t;
	else {
		*pad_before_eth = 0;
		hh->hh_len = PADLEN + ETH_HLEN;
	}
	return 0;
}

/*
 * This is similar to eth_type_trans, which cannot be used because of
 * our dev->hard_header_len
 */
static inline unsigned short br_type_trans(struct sk_buff *skb,
					       struct net_device *dev)
{
	struct ethhdr *eth;
	unsigned char *rawp;
	eth = skb->mac.ethernet;

	if (*eth->h_dest & 1) {
		if (memcmp(eth->h_dest, dev->broadcast, ETH_ALEN) == 0)
			skb->pkt_type = PACKET_BROADCAST;
		else
			skb->pkt_type = PACKET_MULTICAST;
	}

	else if (memcmp(eth->h_dest, dev->dev_addr, ETH_ALEN))
		skb->pkt_type = PACKET_OTHERHOST;

	if (ntohs(eth->h_proto) >= 1536)
		return eth->h_proto;

	rawp = skb->data;

	/*
	 * This is a magic hack to spot IPX packets. Older Novell breaks
	 * the protocol design and runs IPX over 802.3 without an 802.2 LLC
	 * layer. We look for FFFF which isn't a used 802.2 SSAP/DSAP. This
	 * won't work for fault tolerant netware but does for the rest.
	 */
	if (*(unsigned short *) rawp == 0xFFFF)
		return htons(ETH_P_802_3);

	/*
	 * Real 802.2 LLC
	 */
	return htons(ETH_P_802_2);
}
#endif /* FASTER_VERSION */

/*
 * We remember when the MAC gets set, so we don't override it later with
 * the ESI of the ATM card of the first VC
 */
static int (*my_eth_mac_addr)(struct net_device *, void *);
static int br2684_mac_addr(struct net_device *dev, void *p)
{
	int err = my_eth_mac_addr(dev, p);
	if (!err)
		BRPRIV(dev)->mac_was_set = 1;
	return err;
}

#ifdef CONFIG_ATM_BR2684_IPFILTER
/* this IOCTL is experimental. */
static int br2684_setfilt(struct atm_vcc *atmvcc, unsigned long arg)
{
	struct br2684_vcc *brvcc;
	struct br2684_filter_set fs;

	if (copy_from_user(&fs, (void *) arg, sizeof fs))
		return -EFAULT;
	if (fs.ifspec.method != BR2684_FIND_BYNOTHING) {
		/*
		 * This is really a per-vcc thing, but we can also search
		 * by device
		 */
		struct br2684_dev *brdev;
		read_lock(&devs_lock);
		brdev = br2684_find_dev(&fs.ifspec);
		if (brdev == NULL || list_empty(&brdev->brvccs) ||
		    brdev->brvccs.next != brdev->brvccs.prev)  /* >1 VCC */
			brvcc = NULL;
		else
			brvcc = list_entry_brvcc(brdev->brvccs.next);
		read_unlock(&devs_lock);
		if (brvcc == NULL)
			return -ESRCH;
	} else
		brvcc = BR2684_VCC(atmvcc);
	memcpy(&brvcc->filter, &fs.filter, sizeof(brvcc->filter));
	return 0;
}

/* Returns 1 if packet should be dropped */
static inline int
packet_fails_filter(u16 type, struct br2684_vcc *brvcc, struct sk_buff *skb)
{
	if (brvcc->filter.netmask == 0)
		return 0;			/* no filter in place */
	if (type == __constant_htons(ETH_P_IP) &&
	    (((struct iphdr *) (skb->data))->daddr & brvcc->filter.
	     netmask) == brvcc->filter.prefix)
		return 0;
	if (type == __constant_htons(ETH_P_ARP))
		return 0;
	/* TODO: we should probably filter ARPs too.. don't want to have
	 *   them returning values that don't make sense, or is that ok?
	 */
	return 1;		/* drop */
}
#endif /* CONFIG_ATM_BR2684_IPFILTER */

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
static void br2684_close_vcc(struct br2684_vcc *brvcc)
{
	DPRINTK("removing VCC %p from dev %p\n", brvcc, brvcc->brdev);
	write_lock_irq(&devs_lock);
	list_del(&brvcc->brvccs);
	write_unlock_irq(&devs_lock);
	brvcc->atmvcc->user_back = NULL;	/* what about vcc->recvq ??? */
	brvcc->old_push(brvcc->atmvcc, NULL);	/* pass on the bad news */
	kfree(brvcc);
	MOD_DEC_USE_COUNT;
}

extern int fp_on;
/* when AAL5 PDU comes in: */
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
static void br2684_push(struct atm_vcc *atmvcc, struct sk_buff *skb)
{
	struct br2684_vcc *brvcc = BR2684_VCC(atmvcc);
	struct br2684_dev *brdev = brvcc->brdev;
	int plen = sizeof(llc_oui_pid_pad) + ETH_HLEN;

	DPRINTK("br2684_push\n");

	if (skb == NULL) {	/* skb==NULL means VCC is being destroyed */
		br2684_close_vcc(brvcc);
		if (list_empty(&brdev->brvccs)) {
			read_lock(&devs_lock);
			list_del(&brdev->br2684_devs);
			read_unlock(&devs_lock);
			unregister_netdev(&brdev->net_dev);
			kfree(brdev);
		}
		return;
	}

	skb_debug(skb);
	atm_return(atmvcc, skb->truesize);
	DPRINTK("skb from brdev %p\n", brdev);
	DPRINTK("brdev %p encaps = %d\n", brdev, brvcc->encaps);
	switch(brvcc->encaps) {
	case e_llc:
		/* let us waste some time for checking the encapsulation.
		   Note, that only 7 char is checked so frames with a valid FCS
		   are also accepted (but FCS is not checked of course) */
		if (memcmp(skb->data, llc_oui_pid_pad, 7)) {
			brdev->stats.rx_errors++;
			dev_kfree_skb(skb);
			return;
		}

		if (skb->len < plen) {
			brdev->stats.rx_errors++;
			dev_kfree_skb(skb);	/* dev_ not needed? */
			return;
		}

#ifdef FASTER_VERSION
	/* FIXME: tcpdump shows that pointer to mac header is 2 bytes earlier,
	   than should be. What else should I set? */
		skb_pull(skb, plen);
		skb->mac.raw = ((char *) (skb->data)) - ETH_HLEN;
		skb->pkt_type = PACKET_HOST;
#ifdef CONFIG_BR2684_FAST_TRANS
		skb->protocol = ((u16 *) skb->data)[-1];
#else				/* some protocols might require this: */
		skb->protocol = br_type_trans(skb, &brdev->net_dev);
#endif /* CONFIG_BR2684_FAST_TRANS */
#else
		skb_pull(skb, plen - ETH_HLEN);
		skb->protocol = eth_type_trans(skb, &brdev->net_dev);
#endif /* FASTER_VERSION */
		
		break;
	case e_vc:
		/* first 2 chars should be 0 */
		if (*((u16 *) (skb->data)) != 0) {
			brdev->stats.rx_errors++;
			dev_kfree_skb(skb);
			return;
		}

		plen = PADLEN + ETH_HLEN;	/* pad, dstmac,srcmac, ethtype */

		if (skb->len < plen) {
			brdev->stats.rx_errors++;
			dev_kfree_skb(skb);	/* dev_ not needed? */
			return;
		}
		skb_pull(skb, plen - ETH_HLEN);
		skb->protocol = eth_type_trans(skb, &brdev->net_dev);

		break;
	case r_llc:
		plen = 8;
		skb_pull(skb, plen);
		//cathy, assign skb->mac.raw for ip_finish_output3
		skb->mac.raw = ((char *) (skb->data)) - ETH_HLEN;
		skb->protocol = ETH_P_IP;
		break;
	case r_vc:
		plen = 0;	/* pad, dstmac,srcmac, ethtype */
		//cathy, assign skb->mac.raw for ip_finish_output3
		skb->mac.raw = ((char *) (skb->data)) - ETH_HLEN;
		skb->protocol = ETH_P_IP;
		break;
	default:	
		dev_kfree_skb(skb);
		return;
	}

#ifdef CONFIG_ATM_BR2684_IPFILTER
	if (packet_fails_filter(skb->protocol, brvcc, skb)) {
		brdev->stats.rx_dropped++;
		dev_kfree_skb(skb);
		return;
	}
#endif /* CONFIG_ATM_BR2684_IPFILTER */
	skb->dev = &brdev->net_dev;
	// Kaohj --- keep the from_device info
	skb->from_dev = skb->dev;
	ATM_SKB(skb)->vcc = atmvcc;	/* needed ? */
	DPRINTK("received packet's protocol: %04x\n", ntohs(skb->protocol));
	skb_debug(skb);
	if (!(brdev->net_dev.flags & IFF_UP)) { /* sigh, interface is down */
		brdev->stats.rx_dropped++;
		dev_kfree_skb(skb);
		return;
	}
	brdev->stats.rx_packets++;
	brdev->stats.rx_bytes += skb->len;

//#define CONFIG_RTL867X_IPTABLES_FAST_PATH
#ifdef CONFIG_RTL867X_IPTABLES_FAST_PATH
#ifdef CONFIG_RTK_VOIP_RX_INPUT_QUEUE
	//extern unsigned char voip_inputQ;
	//extern unsigned int voip_qlen;

	if (skb->protocol == ETH_P_IP){	
		/*voip packets*/
		if (enter_rtk_voip_trap(skb) == 1){			
			return;
	        }
	}
#endif
	// Modified by Mason Yu. FastPath can not be disabled.
	if(fp_on){ 		
		skb->nh.raw=skb->data;
		// Mason Yu for DiffServ.
		// Because we have NAT and DiffServ use tc's u32 classfier, it must use orignal Src IP to parser tc rule.
		memcpy(&(skb->srcIP), skb->nh.raw+12, 4);		
		
                //cathy, pppoe fastpath
//		if( (skb->protocol == 0x8864) && (skb->data[8]== 0x45) ){		//pppoe
		if (skb->protocol == ETH_P_PPP_SES) {		// PPPoE session, protocol 0x8864
			if (((u16 *)skb->data)[3] == 0x0021)	// pppoe data (IP protocol)
				if (pppoe_fastpath(skb)==1){					
					return;
		                }
		}
		else if (skb->protocol == ETH_P_IP)	{	// Only IP packet for fast path
			if (FastPath_Enter(skb)== 1) {
				//printk("in fast path!\n");
				return;
			}
		}
	}	
#endif
#ifdef CONFIG_RTK_VOIP_RX_INPUT_QUEUE
	if (skb_is_voip(skb)) 
	{ 
		netif_rx_voip(skb);			
	}
	else 
#endif
	if (netif_rx(skb) == NET_RX_DROP)
		brdev->stats.rx_dropped++;
}

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
static int br2684_regvcc(struct atm_vcc *atmvcc, unsigned long arg)
{
/* assign a vcc to a dev
Note: we do not have explicit unassign, but look at _push()
*/
	int err;
	struct br2684_vcc *brvcc;
	struct sk_buff_head copy;
	struct sk_buff *skb;
	struct br2684_dev *brdev;
	struct atm_backend_br2684 be;

	MOD_INC_USE_COUNT;
	if (copy_from_user(&be, (void *) arg, sizeof be)) {
		MOD_DEC_USE_COUNT;
		return -EFAULT;
	}
	write_lock_irq(&devs_lock);
	brdev = br2684_find_dev(&be.ifspec);
	if (brdev == NULL) {
		printk(KERN_ERR
		    "br2684: tried to attach to non-existant device\n");
		err = -ENXIO;
		goto error;
	}
	if (atmvcc->push == NULL) {
		err = -EBADFD;
		goto error;
	}
	if (!list_empty(&brdev->brvccs)) {	/* Only 1 VCC/dev right now */
		err = -EEXIST;
		goto error;
	}

	if (be.fcs_in != BR2684_FCSIN_NO || be.fcs_out != BR2684_FCSOUT_NO ||
	    be.fcs_auto || be.has_vpiid || be.send_padding || 
	    (be.encaps != BR2684_ENCAPS_VC && be.encaps != BR2684_ENCAPS_LLC &&
	    be.encaps != BR2684_ENCAPS_VC_IP && be.encaps != BR2684_ENCAPS_LLC_IP) ||
	    be.min_size != 0) {
		err = -EINVAL;
		goto error;
	}
	brvcc = kmalloc(sizeof(struct br2684_vcc), GFP_KERNEL);
	if (!brvcc) {
		err = -ENOMEM;
		goto error;
	}
	memset(brvcc, 0, sizeof(struct br2684_vcc));
	DPRINTK("br2684_regvcc vcc=%p, encaps=%d, brvcc=%p\n", atmvcc, be.encaps,
		brvcc);
	if (list_empty(&brdev->brvccs) && !brdev->mac_was_set) {
		unsigned char *esi = atmvcc->dev->esi;
		if (esi[0] | esi[1] | esi[2] | esi[3] | esi[4] | esi[5])
			memcpy(brdev->net_dev.dev_addr, esi,
			    brdev->net_dev.addr_len);
		else
			brdev->net_dev.dev_addr[2] = 1;
	}
	list_add(&brvcc->brvccs, &brdev->brvccs);
	write_unlock_irq(&devs_lock);
	brvcc->brdev = brdev;
	brvcc->atmvcc = atmvcc;
	atmvcc->user_back = brvcc;
	brvcc->encaps = (enum br2684_encaps) be.encaps;
	brvcc->old_push = atmvcc->push;
	barrier();
	atmvcc->push = br2684_push;
	skb_queue_head_init(&copy);
	skb_migrate(&atmvcc->recvq, &copy);
	// Kaohj -- notify encapsulation to sar driver
	fops_get (atmvcc->dev->ops);
	if (atmvcc->dev->ops->ioctl) {
		struct SAR_IOCTL_CFG 	cfg;
		
		cfg.vpi = atmvcc->vpi;
		cfg.vci = atmvcc->vci;
		switch(be.encaps) {
			case BR2684_ENCAPS_VC:
				cfg.rfc = RFC1483_BRIDGED;
				cfg.framing = VC_MUX;
				break;
			case BR2684_ENCAPS_LLC:
				cfg.rfc = RFC1483_BRIDGED;
				cfg.framing = LLC_SNAP;
				break;
			case BR2684_ENCAPS_VC_IP:
				cfg.rfc = RFC1483_ROUTED;
				cfg.framing = VC_MUX;
				break;
			case BR2684_ENCAPS_LLC_IP:
				cfg.rfc = RFC1483_ROUTED;
				cfg.framing = LLC_SNAP;
				break;
			default:
				break;
		}
		err = atmvcc->dev->ops->ioctl(atmvcc->dev, SAR_SET_ENCAPS, &cfg);
		if (err) {
			fops_put (atmvcc->dev->ops);
			printk("WARNNING!! Set encapsulation failed!\n");
			//return err;
		}
	}
	while ((skb = skb_dequeue(&copy))) {
		BRPRIV(skb->dev)->stats.rx_bytes -= skb->len;
		BRPRIV(skb->dev)->stats.rx_packets--;
		br2684_push(atmvcc, skb);
	}
	return 0;
    error:
	write_unlock_irq(&devs_lock);
	MOD_DEC_USE_COUNT;
	return err;
}

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
static int br2684_create(struct atm_vcc *atmvcc, unsigned long arg)
{
	int err;
	struct br2684_dev *brdev;
	struct atm_newif_br2684 ni;

	DPRINTK("br2684_create\n");
	/*
	 * We track module use by vcc's NOT the devices they're on.  We're
	 * protected here against module death by the kernel_lock, but if
	 * we need to sleep we should make sure that the module doesn't
	 * disappear under us.
	 */
	MOD_INC_USE_COUNT;
	if (copy_from_user(&ni, (void *) arg, sizeof ni)) {
		MOD_DEC_USE_COUNT;
		return -EFAULT;
	}
	if (ni.media != BR2684_MEDIA_ETHERNET || ni.mtu != 1500) {
		MOD_DEC_USE_COUNT;
		return -EINVAL;
	}
	if ((brdev = kmalloc(sizeof(struct br2684_dev), GFP_KERNEL)) == NULL) {
		MOD_DEC_USE_COUNT;
		return -ENOMEM;
	}
	memset(brdev, 0, sizeof(struct br2684_dev));
	INIT_LIST_HEAD(&brdev->brvccs);

	write_lock_irq(&devs_lock);
	brdev->number = list_empty(&br2684_devs) ? 1 :
	    list_entry_brdev(br2684_devs.prev)->number + 1;
	list_add_tail(&brdev->br2684_devs, &br2684_devs);
	write_unlock_irq(&devs_lock);

	if (ni.ifname[0] != '\0') {
		memcpy(brdev->net_dev.name, ni.ifname,
		    sizeof(brdev->net_dev.name));
		brdev->net_dev.name[sizeof(brdev->net_dev.name) - 1] = '\0';
	} else
		sprintf(brdev->net_dev.name, "nas%d", brdev->number);
	DPRINTK("registered netdev %s\n", brdev->net_dev.name);
	ether_setup(&brdev->net_dev);
	brdev->mac_was_set = 0;
#ifdef FASTER_VERSION
	my_eth_header = brdev->net_dev.hard_header;
	brdev->net_dev.hard_header = br2684_header;
	my_eth_header_cache = brdev->net_dev.hard_header_cache;
	brdev->net_dev.hard_header_cache = br2684_header_cache;
	brdev->net_dev.hard_header_len = sizeof(llc_oui_pid_pad) + ETH_HLEN;	/* 10 + 14 */
#endif
	my_eth_mac_addr = brdev->net_dev.set_mac_address;
	brdev->net_dev.set_mac_address = br2684_mac_addr;
	brdev->net_dev.hard_start_xmit = br2684_start_xmit;
	brdev->net_dev.get_stats = br2684_get_stats;
	// Kaohj, used to avoid bridging between WAN connections
	brdev->net_dev.priv_flags = IFF_DOMAIN_WAN;

	// Kaohj -- put net_device into atm_vcc
	atmvcc->net_dev = (void *)&brdev->net_dev;
	/* open, stop, do_ioctl ? */
	err = register_netdev(&brdev->net_dev);
	
	MOD_DEC_USE_COUNT;
	if (err < 0) {
		printk(KERN_ERR "br2684_create: register_netdev failed\n");
		write_lock_irq(&devs_lock);
		list_del(&brdev->br2684_devs);
		write_unlock_irq(&devs_lock);
		kfree(brdev);
		return err;
	}
	return 0;
}


// Added by Mason Yu for PPPoE PassThrough
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
static int br2684_setbrpppoe(struct atm_vcc *atmvcc, unsigned long arg)
{
	
	int err;	
	struct br2684_dev *brdev;
	struct atm_backend_br2684 be;	   
    
	MOD_INC_USE_COUNT;
	if (copy_from_user(&be, (void *) arg, sizeof be)) {
		MOD_DEC_USE_COUNT;
		return -EFAULT;
	}	
			
	write_lock_irq(&devs_lock);
	brdev = br2684_find_dev(&be.ifspec);
	if (brdev == NULL) {
		printk(KERN_ERR
		    "br2684: tried to attach to non-existant device(br2684_setbrpppoe)\n");
		err = -ENXIO;
		goto error;
	}
	
	
	brdev->net_dev.brpppoe = be.brpppoe;
	write_unlock_irq(&devs_lock);
	
	//printk("brdev->net_dev.brpppoe = %d\n", brdev->net_dev.brpppoe);
	return 0;
  error:
	write_unlock_irq(&devs_lock);
	MOD_DEC_USE_COUNT;
	return err;	
    
}	

#ifdef CONFIG_EXT_SWITCH
// Kaohj
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
static int br2684_setvlan(struct atm_vcc *atmvcc, unsigned long arg)
{
	struct atm_backend_br2684 be;

	MOD_INC_USE_COUNT;
	if (copy_from_user(&be, (void *) arg, sizeof be)) {
		MOD_DEC_USE_COUNT;
		return -EFAULT;
	}
	write_lock_irq(&devs_lock);
	//printk("br2684_setvlan(%d/%d): vlan=%d, vid=%d\n",
	// atmvcc->vpi, atmvcc->vci, be.vlan, be.vid);
	atmvcc->pvcvlan.vlan = be.vlan_tag.vlan;
	atmvcc->pvcvlan.vid = be.vlan_tag.vid;
	atmvcc->pvcvlan.vlan_prio = be.vlan_tag.vlan_prio;
	atmvcc->pvcvlan.vlan_pass = be.vlan_tag.vlan_pass;
	write_unlock_irq(&devs_lock);
	return 0;
}

// Kaohj --- defined at re8670.c
extern int bitmap_virt2phy(int mbr);

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
static int br2684_setitfgrp(struct atm_vcc *atmvcc, unsigned long arg)
{
	struct atm_backend_br2684 be;

	MOD_INC_USE_COUNT;
	if (copy_from_user(&be, (void *) arg, sizeof be)) {
		MOD_DEC_USE_COUNT;
		return -EFAULT;
	}
	write_lock_irq(&devs_lock);
	memcpy(&atmvcc->ifgrp, &be.ifgrp, sizeof(atmvcc->ifgrp));
	// Kaohj -- translate bitmap of virtual port to that of phy id(port)
	atmvcc->ifgrp.member = bitmap_virt2phy(atmvcc->ifgrp.member);
	//printk("br2684_setitfgrp(%d/%d): group=%d, member=0x%x\n",
	// atmvcc->vpi, atmvcc->vci, atmvcc->ifgrp.flag, atmvcc->ifgrp.member);
	write_unlock_irq(&devs_lock);
	return 0;
}
#endif

/*
 * This handles ioctls actually performed on our vcc - we must return
 * -ENOIOCTLCMD for any unrecognized ioctl
 */
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
static int br2684_ioctl(struct atm_vcc *atmvcc, unsigned int cmd,
	unsigned long arg)
{
	int err;	
	switch(cmd) {
	case ATM_SETBACKEND:
	case ATM_NEWBACKENDIF:{	
		atm_backend_t b;
		MOD_INC_USE_COUNT;
		err = get_user(b, (atm_backend_t *) arg);
		MOD_DEC_USE_COUNT;
		if (err)
			return -EFAULT;
		if (b != ATM_BACKEND_BR2684)
			return -ENOIOCTLCMD;
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		if (cmd == ATM_SETBACKEND)					
			return br2684_regvcc(atmvcc, arg); 		
		else		    			
		    return br2684_create(atmvcc, arg);	 	   
        }
	// Added by Mason Yu for PPPoE PassThrough    
	case ATM_SETBRIDGEPPPOE:
		return br2684_setbrpppoe(atmvcc, arg);
#ifdef CONFIG_EXT_SWITCH
	case ATM_SETVLAN:
		return br2684_setvlan(atmvcc, arg);
	case ATM_SETITFGRP:
		return br2684_setitfgrp(atmvcc, arg);
#endif
#ifdef CONFIG_ATM_BR2684_IPFILTER
	case BR2684_SETFILT:
		if (atmvcc->push != br2684_push)
			return -ENOIOCTLCMD;
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		MOD_INC_USE_COUNT;
		err = br2684_setfilt(atmvcc, arg);
		MOD_DEC_USE_COUNT;
		return err;
#endif /* CONFIG_ATM_BR2684_IPFILTER */
	}
	return -ENOIOCTLCMD;
}

/* Never put more than 256 bytes in at once */
static int br2684_proc_engine(loff_t pos, char *buf)
{
	struct list_head *lhd, *lhc;
	struct br2684_dev *brdev;
	struct br2684_vcc *brvcc;
	list_for_each(lhd, &br2684_devs) {
		brdev = list_entry_brdev(lhd);
		if (pos-- == 0)
			return sprintf(buf, "dev %.16s: num=%d, mac=%02X:%02X:"
			    "%02X:%02X:%02X:%02X (%s)\n", brdev->net_dev.name,
			    brdev->number,
			    brdev->net_dev.dev_addr[0],
			    brdev->net_dev.dev_addr[1],
			    brdev->net_dev.dev_addr[2],
			    brdev->net_dev.dev_addr[3],
			    brdev->net_dev.dev_addr[4],
			    brdev->net_dev.dev_addr[5],
			    brdev->mac_was_set ? "set" : "auto");
		list_for_each(lhc, &brdev->brvccs) {
			brvcc = list_entry_brvcc(lhc);
			if (pos-- == 0)
//				return sprintf(buf, "  vcc %d.%d.%d: encaps=%s"
				return sprintf(buf, "  vcc %d.%d.%d: encaps=%s %d"		// Jenny, for WAN status determination
#ifndef FASTER_VERSION
				    ", failed copies %u/%u"
#endif /* FASTER_VERSION */
				    "\n", brvcc->atmvcc->dev->number,
				    brvcc->atmvcc->vpi, brvcc->atmvcc->vci,
//				    (brvcc->encaps == e_llc) ? "LLC" : "VC"
				    (brvcc->encaps == e_llc) ? "LLC" : "VC", brvcc->encaps
#ifndef FASTER_VERSION
				    , brvcc->copies_failed
				    , brvcc->copies_needed
#endif /* FASTER_VERSION */
				    );
#ifdef CONFIG_ATM_BR2684_IPFILTER
#define b1(var, byte)	((u8 *) &brvcc->filter.var)[byte]
#define bs(var)		b1(var, 0), b1(var, 1), b1(var, 2), b1(var, 3)
			if (brvcc->filter.netmask != 0 && pos-- == 0)
				return sprintf(buf, "    filter=%d.%d.%d.%d/"
				    "%d.%d.%d.%d\n", bs(prefix), bs(netmask));
#undef bs
#undef b1
#endif /* CONFIG_ATM_BR2684_IPFILTER */
		}
	}
	return 0;
}

static ssize_t br2684_proc_read(struct file *file, char *buf, size_t count,
	loff_t *pos)
{
	unsigned long page;
	int len = 0, x, left;
	page = get_free_page(GFP_KERNEL);
	if (!page)
		return -ENOMEM;
	left = PAGE_SIZE - 256;
	if (count < left)
		left = count;
	read_lock(&devs_lock);
	for (;;) {
		x = br2684_proc_engine(*pos, &((char *) page)[len]);
		if (x == 0)
			break;
		if (x > left)
			/*
			 * This should only happen if the user passed in
			 * a "count" too small for even one line
			 */
			x = -EINVAL;
		if (x < 0) {
			len = x;
			break;
		}
		len += x;
		left -= x;
		(*pos)++;
		if (left < 256)
			break;
	}
	read_unlock(&devs_lock);
	if (len > 0 && copy_to_user(buf, (char *) page, len))
		len = -EFAULT;
	free_page(page);
	return len;
}

static struct file_operations br2684_proc_operations = {
	read: br2684_proc_read,
};

extern struct proc_dir_entry *atm_proc_root;	/* from proc.c */

extern int (*br2684_ioctl_hook)(struct atm_vcc *, unsigned int, unsigned long);

/* the following avoids some spurious warnings from the compiler */
#define UNUSED __attribute__((unused))

static int __init UNUSED br2684_init(void)
{
	struct proc_dir_entry *p;
DPRINTK("br2684_init()\n");
	if ((p = create_proc_entry("br2684", 0, atm_proc_root)) == NULL)
		return -ENOMEM;
	p->proc_fops = &br2684_proc_operations;
	br2684_ioctl_hook = br2684_ioctl;
	return 0;
}

static void __exit UNUSED br2684_exit(void)
{
	struct br2684_dev *brdev;
	br2684_ioctl_hook = NULL;
	remove_proc_entry("br2684", atm_proc_root);
	while (!list_empty(&br2684_devs)) {
		brdev = list_entry_brdev(br2684_devs.next);
		unregister_netdev(&brdev->net_dev);
		list_del(&brdev->br2684_devs);
		kfree(brdev);
	}
}

module_init(br2684_init);
module_exit(br2684_exit);

MODULE_AUTHOR("Marcell GAL");
MODULE_DESCRIPTION("RFC2684 bridged protocols over ATM/AAL5");
MODULE_LICENSE("GPL");
