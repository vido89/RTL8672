/* re8650.c: A Linux Ethernet driver for the RealTek 8670 chips. */
/*
	Copyright 2001,2002 Jeff Garzik <jgarzik@mandrakesoft.com>

	Copyright (C) 2001, 2002 David S. Miller (davem@redhat.com) [tg3.c]
	Copyright (C) 2000, 2001 David S. Miller (davem@redhat.com) [sungem.c]
	Copyright 2001 Manfred Spraul				    [natsemi.c]
	Copyright 1999-2001 by Donald Becker.			    [natsemi.c]
       	Written 1997-2001 by Donald Becker.			    [8139too.c]
	Copyright 1998-2001 by Jes Sorensen, <jes@trained-monkey.org>. [acenic.c]

	This software may be used and distributed according to the terms of
	the GNU General Public License (GPL), incorporated herein by reference.
	Drivers based on or derived from this code fall under the GPL and must
	retain the authorship, copyright and license notice.  This file is not
	a complete program and may only be used when the entire operating
	system is licensed under the GPL.

	See the file COPYING in this distribution for more information.

	TODO, in rough priority order:
	* dev->tx_timeout
	* LinkChg interrupt
	* Support forcing media type with a module parameter,
	  like dl2k.c/sundance.c
	* Implement PCI suspend/resume
	* Constants (module parms?) for Rx work limit
	* support 64-bit PCI DMA
	* Complete reset on PciErr
	* Consider Rx interrupt mitigation using TimerIntr
	* Implement 8139C+ statistics dump; maybe not...
	  h/w stats can be reset only by software reset
	* Tx checksumming
	* Handle netif_rx return value
	* ETHTOOL_GREGS, ETHTOOL_[GS]WOL,
	* Investigate using skb->priority with h/w VLAN priority
	* Investigate using High Priority Tx Queue with skb->priority
	* Adjust Rx FIFO threshold and Max Rx DMA burst on Rx FIFO error
	* Adjust Tx FIFO threshold and Max Tx DMA burst on Tx FIFO error
	* Implement Tx software interrupt mitigation via
	  Tx descriptor bit
	* The real minimum of CP_MIN_MTU is 4 bytes.  However,
	  for this to be supported, one must(?) turn on packet padding.

 */

#define DRV_NAME		"8139cp"
#define DRV_VERSION		"0.0.7"
#define DRV_RELDATE		"Feb 27, 2002"


#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/compiler.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/if_vlan.h>
#include <linux/crc32.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/rtl8650/interrupt.h>
#include <linux/slab.h>
#include "rtl_types.h"
#include "rtl_mbuf.h"
#include "rtl8651_asicRegs.h"
#include "swCore.h"


#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
#define CP_VLAN_TAG_USED 1
#define CP_VLAN_TX_TAG(tx_desc,vlan_tag_value) \
	do { (tx_desc)->opts2 = (vlan_tag_value); } while (0)
#else
#define CP_VLAN_TAG_USED 0
#define CP_VLAN_TX_TAG(tx_desc,vlan_tag_value) \
	do { (tx_desc)->opts2 = 0; } while (0)
#endif

/* These identify the driver base version and may not be removed. */
static char version[] __devinitdata =
KERN_INFO DRV_NAME " Ethernet driver v" DRV_VERSION " (" DRV_RELDATE ")\n";

MODULE_AUTHOR("Rupert Chang <rupert@mail.realtek.com.tw>");
MODULE_DESCRIPTION("RealTek RTL-8670 series 10/100 Ethernet driver");
MODULE_LICENSE("GPL");

static int debug = -1;
MODULE_PARM (debug, "i");
MODULE_PARM_DESC (debug, "re8650 bitmapped message enable number");

/* Maximum number of multicast addresses to filter (vs. Rx-all-multicast).
   The RTL chips use a 64 element hash table based on the Ethernet CRC.  */
static int multicast_filter_limit = 32;
MODULE_PARM (multicast_filter_limit, "i");
MODULE_PARM_DESC (multicast_filter_limit, "8139cp maximum number of filtered multicast addresses");

#define PFX			DRV_NAME ": "
#define CP_DEF_MSG_ENABLE	(NETIF_MSG_DRV		| \
				 NETIF_MSG_PROBE 	| \
				 NETIF_MSG_LINK)
#define CP_REGS_SIZE		(0xff + 1)
#define RE8650_RX_RING_SIZE	16	
#define RE8650_TX_RING_SIZE	32	
#define DESC_ALIGN		0x4
#define UNCACHE_MASK		0xa0000000
#define UNCACHE(x)		((u32)x|UNCACHE_MASK)
struct rtl_pktHdr		txPktBuff[RE8650_TX_RING_SIZE];
struct rtl_mBuf			txMblkBuff[RE8650_TX_RING_SIZE];
struct rtl_pktHdr		rxPktBuff[RE8650_RX_RING_SIZE];
struct rtl_mBuf			rxMblkBuff[RE8650_RX_RING_SIZE];
#define	PKTBUF_SIZE 			RE8650_TX_RING_SIZE*sizeof(struct 		
#define RE8650_RXRING_BYTES	( (sizeof(DMA_INFO) * (RE8650_RX_RING_SIZE+1)) + DESC_ALIGN)

#define RE8650_TXRING_BYTES	( (sizeof(DMA_INFO) * (RE8650_TX_RING_SIZE+1)) + DESC_ALIGN)
		


#define NEXT_TX(N)		(((N) + 1) & (RE8650_TX_RING_SIZE - 1))


#define NEXT_TX(N)		(((N) + 1) & (RE8650_TX_RING_SIZE - 1))
#define NEXT_RX(N)		(((N) + 1) & (RE8650_RX_RING_SIZE - 1))
#define TX_BUFFS_AVAIL(CP)					\
	(((CP)->tx_tail <= (CP)->tx_head) ?			\
	  (CP)->tx_tail + (RE8650_TX_RING_SIZE - 1) - (CP)->tx_head :	\
	  (CP)->tx_tail - (CP)->tx_head - 1)


#define PKT_BUF_SZ		1536	/* Size of each temporary Rx buffer.*/
#define RX_OFFSET		2

/* The following settings are log_2(bytes)-4:  0 == 16 bytes .. 6==1024, 7==end of packet. */

/* Time in jiffies before concluding the transmitter is hung. */
#define TX_TIMEOUT		(6*HZ)

/* hardware minimum and maximum for a single frame's data payload */
#define CP_MIN_MTU		60	/* TODO: allow lower, but pad */
#define CP_MAX_MTU		4096
#define	LOOPBACK	1


enum PHY_REGS{
	TXFCE= 1<<7,
	RXFCE=1 <<6,
	SP100=1<<3,
	LINK=1<<2,
	TXPF=1<<1,
	RXPF=1<<0,
	FORCE_TX = 1<<5

};
	
typedef	struct rtl_mBuf *RTL_MP;	
typedef struct dma_info
{
		u32	opts1;
}DMA_INFO;

struct ring_info {
	struct sk_buff		*skb;
	unsigned		frag;
	struct rtl_pktHdr	*PktHdr;
	struct rtl_mBuf		*Mblk;
};

struct cp_extra_stats {
	unsigned long		rx_frags;
	unsigned long 		tx_timeouts;
	unsigned long		mbufout_cnt;
	unsigned long		pktrunt;
	unsigned long		linkchg;
};

struct re_private {
	unsigned		tx_head;
	unsigned		tx_tail;
	unsigned		tx_lqhead;
	unsigned		tx_lqtail;
	unsigned		rx_tail;
	u32			vid;            /* VLAN index */
	u32			memberPort;     /* member port mask */
	u32			portNums;     	/* number of member ports */

	void			*regs;
	struct net_device	*dev;
	spinlock_t		lock;
	DMA_INFO		*rxPktDesc;
	DMA_INFO		*rxMblkDesc;
	DMA_INFO		*txPktDesc;
	struct rtl_pktHdr	*txPktHdr[RE8650_TX_RING_SIZE]; 
	struct rtl_mBuf		*txMBlk[RE8650_TX_RING_SIZE]; 
	struct ring_info	tx_skb[RE8650_TX_RING_SIZE];
	struct ring_info	rx_skb[RE8650_RX_RING_SIZE];
	unsigned		rx_buf_sz;

#if CP_VLAN_TAG_USED
	struct vlan_group	*vlgrp;
#endif

	u32			msg_enable;

	struct net_device_stats net_stats;
	struct cp_extra_stats	cp_stats;

	struct pci_dev		*pdev;
	u32			rx_config;

	struct sk_buff		*frag_skb;
	unsigned		dropping_frag : 1;
	struct mii_if_info	mii_if;
};
struct re_private *reDev;
static void __re8650_set_rx_mode (struct net_device *dev);
static void re8650_tx (struct re_private *cp);
static void re8650_clean_rings (struct re_private *cp);
static void re8650_tx_timeout (struct net_device *dev);
static int re8650_start_xmit1(struct sk_buff *skb, struct net_device *dev);
int vlan_config(struct re_private *cp);
int port_config(struct re_private *cp);
int swCore_init(void);

int	port_config(struct re_private *cp)
{
	int i;
	rtl_auto_nego_ability_t	ab;
	
	ab.capableFlowCtrl = 1;
	ab.capable100MFull = ab.capable100MHalf = 0;
	ab.capable10MFull = ab.capable10MHalf = 1;
	for (i=0;i<MAX_PORT_NUMBER;i++)
	{
		swCore_portSetAutoNegociationAbility(i,&ab);
		swCore_portEnableAutoNegociation(i);
		swCore_portRestartAutoNegociation(i);

	}
}



int	vlan_config(struct re_private *cp)
{
	rtl_vlan_param_t	vp;
	int	i;
	vp.mtu = 1522;
	vp.memberPort = 0x3f;
	cp->memberPort = 0x3f;
	vp.egressUntag = 0x3f;
	memcpy((char*)&vp.gMac, (char*)cp->dev->dev_addr, 6);
	if(swCore_vlanCreate(8,&vp)){
		printk("creating vlan %d fails\n", cp->vid);
		return -1;
	}
#if 0
	//swCore_vlanDestroy(cp->vid);
	if(swCore_vlanCreate(cp->vid,&vp)){
		printk("creating vlan %d fails\n", cp->vid);
		return -1;
	}
	for(i=0; i<MAX_PORT_NUMBER; i++)
	{
		if(cp->memberPort & (1<<i))
		{
			swCore_vlanSetPVid(i, cp->vid);
		}
	}
#endif
	vlanTable_setStpStatusOfAllPorts(8, RTL_STP_FORWARD);
}


static inline void re8650_set_rxbufsize (struct re_private *cp)
{
	unsigned int mtu = cp->dev->mtu;
	
	if (mtu > ETH_DATA_LEN)
		/* MTU + ethernet header + FCS + optional VLAN tag */
		cp->rx_buf_sz = mtu + ETH_HLEN + 8;
	else
		cp->rx_buf_sz = PKT_BUF_SZ;
}

static void re8650_rx_skb (struct re_private *cp, struct sk_buff *skb)
{
	skb->protocol = eth_type_trans (skb, cp->dev);

	cp->net_stats.rx_packets++;
	cp->net_stats.rx_bytes += skb->len;
	cp->dev->last_rx = jiffies;

#if CP_VLAN_TAG_USED
	sd
	if (cp->vlgrp && (desc->opts2 & RxVlanTagged)) {
		vlan_hwaccel_rx(skb, cp->vlgrp, desc->opts2 & 0xffff);
	} else
#endif
#if LOOPBACK
		if(re8650_start_xmit1(skb,cp->dev))
		{
			cp->net_stats.tx_dropped++;
			dev_kfree_skb_irq(skb);
		}
#else
		netif_rx(skb);
#endif
}

static void re8650_rx_err_acct (struct re_private *cp, unsigned rx_tail,
			    u32 status, u32 len)
{
}

static void re8650_rx_frag (struct re_private *cp, unsigned rx_tail,
			struct sk_buff *skb, u32 status, u32 len)
{
}

static inline unsigned int re8650_rx_csum_ok (u32 status)
{
	return 0;
}

static void re8650_rx (struct re_private *cp)
{
	unsigned rx_tail = cp->rx_tail;
	struct sk_buff *skb, *new_skb;
	unsigned rx_work = 100;
	u16	len;
	u32	status;
	unsigned buflen;
	struct 	 ring_info	*ring;
	while (rx_work--) {
		ring = &cp->rx_skb[rx_tail];
		skb = ring->skb;
		if (!skb)
			BUG();
		status = cp->rxPktDesc[rx_tail].opts1;
		if( status & DESC_OWNED_BIT)
			break;
		len = (ring->PktHdr->ph_len&0x1fff)-4;
		if (netif_msg_rx_status(cp))
			printk(KERN_DEBUG "%s: rx slot %d status 0x%x len %d\n",
			       cp->dev->name, rx_tail,status, len);
		buflen = cp->rx_buf_sz + RX_OFFSET;
		new_skb = dev_alloc_skb (buflen);
		if (!new_skb) {
			cp->net_stats.rx_dropped++;
			goto rx_next;
		}
		if ((u32)new_skb->data &0x3)
			printk(KERN_DEBUG "new_skb->data unaligment %8x\n",(u32)new_skb->data);

		new_skb->dev = cp->dev;
		//dma_cache_wback_inv((unsigned long)skb->data, len);
		
		skb_put(skb, len);
		skb->len = len;
		if (skb->len != len)
			printk("skb->len %d len %d \n",skb->len,len);
		re8650_rx_skb(cp, skb);
		
		ring->skb = new_skb;

rx_next:
		
		ring->PktHdr->ph_mbuf	= (void*)UNCACHE(ring->Mblk);
		ring->PktHdr->ph_len 	= 0x0;
		ring->PktHdr->ph_flags	= 0x9000;
		
		ring->Mblk->m_next = 0;
		ring->Mblk->m_pkthdr = (void*)UNCACHE(ring->PktHdr);
		ring->Mblk->m_flags = 0x9c;
		ring->Mblk->m_len = 0x0;
		ring->Mblk->m_data = (void*)UNCACHE(new_skb->data);
		if (rx_tail == (RE8650_RX_RING_SIZE - 1))
		{
			cp->rxMblkDesc[rx_tail].opts1 |= UNCACHE(ring->Mblk)|DESC_OWNED_BIT|DESC_WRAP_BIT;
			cp->rxPktDesc[rx_tail].opts1 |= DESC_OWNED_BIT|DESC_WRAP_BIT; 
		}
		else
		{
			cp->rxMblkDesc[rx_tail].opts1 |= UNCACHE(ring->Mblk)|DESC_OWNED_BIT;
			cp->rxPktDesc[rx_tail].opts1 |= DESC_OWNED_BIT; 
		}
		rx_tail = NEXT_RX(rx_tail);
	}
	cp->rx_tail = rx_tail;
}

static void re8650_interrupt (int irq, void *dev_instance, struct pt_regs *regs)
{
	struct net_device *dev = dev_instance;
	struct re_private *cp = dev->priv;
	u16 status;
    	u32        stat;
	int int_count=0;
	int RecvFlag = FALSE;
		

	/* Disable controller interrupts. */

   	/* Read the interrupt status register */
    	stat = REG32(CPUIISR);
    	/* clear interrupts, */
    	/* Check if a valid Interrupt has been set */
	if(stat & PKTHDR_DESC_RUNOUT_IP)
	{
    		REG32(CPUIIMR) |= PKTHDR_DESC_RUNOUT_IE;
		re8650_rx(cp);
		cp->cp_stats.pktrunt++;
	}
	if(stat & MBUF_DESC_RUNOUT_IP)
	{
		re8650_rx(cp);
		cp->cp_stats.mbufout_cnt++;
	}
	/* Check for Receive Complete */
	if (stat & RX_DONE_IP)
	{
		re8650_rx(cp);
	}
	if (stat & RX_ERR_IP)
	{
		cp->net_stats.rx_errors++;
	}
	/* Check for transmit Interrupt */
	if (stat & TX_DONE_IP)
	{
        	re8650_tx(cp);
	}
	/* Check and handle link change interrupt */
	if (stat & LINK_CHANG_IP) 
	{
		cp->cp_stats.linkchg++;
	}
	REG32(CPUIISR) = stat;
	spin_unlock(&cp->lock);
}

static void re8650_tx (struct re_private *cp)
{
	unsigned tx_head = cp->tx_head;
	unsigned tx_tail = cp->tx_tail;
	u32	status;

	while (tx_tail != tx_head) {
		struct sk_buff *skb;
		status  = cp->txPktDesc[tx_tail].opts1;
		if (status & DESC_OWNED_BIT)
			break;

		 cp->txPktDesc[tx_tail].opts1=0x0;
		skb = cp->tx_skb[tx_tail].skb;
		if (!skb)
			BUG();
		cp->net_stats.tx_packets++;
		cp->net_stats.tx_bytes += skb->len;
		if (netif_msg_tx_done(cp))
			printk(KERN_DEBUG "%s: tx done, slot %d\n", cp->dev->name, tx_tail);
		dev_kfree_skb_irq(skb);
		cp->tx_skb[tx_tail].skb = NULL;
		tx_tail = NEXT_TX(tx_tail);
	}

	cp->tx_tail = tx_tail;

	if (netif_queue_stopped(cp->dev) && (TX_BUFFS_AVAIL(cp) > (MAX_SKB_FRAGS + 1)))
		netif_wake_queue(cp->dev);

}

static int re8650_start_xmit1(struct sk_buff *skb, struct net_device *dev)
{
	
	struct re_private *cp = dev->priv;
	struct rtl_pktHdr 	*pktHdr;	
	struct rtl_mBuf		*Mblk;
	unsigned entry;

	if (TX_BUFFS_AVAIL(cp) <= (skb_shinfo(skb)->nr_frags + 1)) {
		printk("%s: Tx Ring full\n",
		       dev->name);
		return 1;
	}
	entry = cp->tx_head;
	if (skb_shinfo(skb)->nr_frags == 0) {
		u32 len;

		len = ETH_ZLEN < skb->len ? skb->len : ETH_ZLEN;
		len +=4;
		cp->tx_skb[entry].skb = skb;
		cp->tx_skb[entry].frag = 0 ;

		Mblk = cp->txMBlk[entry];
		pktHdr = cp->txPktHdr[entry];
		Mblk->m_next = 0;
		Mblk->m_len = len;
		Mblk->m_flags = 0x9c;
		Mblk->m_data = (char*)UNCACHE(skb->data);
		//pktHdr->ph_portlist = cp->memberPort&swCore_layer2TableGetPortByMac(skb->data);
		pktHdr->ph_portlist = 0xf;
		pktHdr->ph_len = len;
		pktHdr->ph_flags = 0x8800;
		if (entry==(RE8650_TX_RING_SIZE-1))
		cp->txPktDesc[entry].opts1 = DESC_OWNED_BIT|
			DESC_WRAP_BIT |UNCACHE(cp->txPktHdr[entry]); 
		else
			cp->txPktDesc[entry].opts1 = DESC_OWNED_BIT|
				UNCACHE(cp->txPktHdr[entry]); 

		entry = NEXT_TX(entry);
	} else {
		for(;;);
		/* We must give this initial chunk to the device last.
		 * Otherwise we could race with the device.
		 */

	}
	cp->tx_head = entry;
	REG32(CPUICR) |= TXFD;
	dev->trans_start = jiffies;
	return 0;


}
static int re8650_start_xmit (struct sk_buff *skb, struct net_device *dev)
{
	struct re_private *cp = dev->priv;
	struct rtl_pktHdr 	*pktHdr;	
	struct rtl_mBuf		*Mblk;
	unsigned entry;
#if CP_VLAN_TAG_USED
	u32 vlan_tag = 0;
#endif

	spin_lock_irq(&cp->lock);
	if (TX_BUFFS_AVAIL(cp) <= (skb_shinfo(skb)->nr_frags + 1)) {
		netif_stop_queue(dev);
		spin_unlock_irq(&cp->lock);
		printk("%s: Tx Ring full\n",
		       dev->name);
		return 1;
	}
	entry = cp->tx_head;
	if (skb_shinfo(skb)->nr_frags == 0) {
		u32 len;

		len = ETH_ZLEN < skb->len ? skb->len : ETH_ZLEN;
		len +=4;
		cp->tx_skb[entry].skb = skb;
		cp->tx_skb[entry].frag = 0 ;

		Mblk = cp->txMBlk[entry];
		pktHdr = cp->txPktHdr[entry];
		Mblk->m_next = 0;
		Mblk->m_len = len;
		Mblk->m_flags = 0x9c;
		Mblk->m_data = (char*)UNCACHE(skb->data);
		//pktHdr->ph_portlist = cp->memberPort&swCore_layer2TableGetPortByMac(skb->data);
		pktHdr->ph_portlist = 0xf;
		pktHdr->ph_len = len;
		pktHdr->ph_flags = 0x8800;
		if (entry==(RE8650_TX_RING_SIZE-1))
		cp->txPktDesc[entry].opts1 = DESC_OWNED_BIT|
			DESC_WRAP_BIT |UNCACHE(cp->txPktHdr[entry]); 
		else
			cp->txPktDesc[entry].opts1 = DESC_OWNED_BIT|
				UNCACHE(cp->txPktHdr[entry]); 

		entry = NEXT_TX(entry);
	} else {
		for(;;);
		/* We must give this initial chunk to the device last.
		 * Otherwise we could race with the device.
		 */

	}
	cp->tx_head = entry;
	if (netif_msg_tx_queued(cp))
		printk(KERN_DEBUG "%s: tx queued, slot %d, skblen %d\n",
		       dev->name, entry, skb->len);

	spin_unlock_irq(&cp->lock);

	REG32(CPUICR) |= TXFD;
	
	
	dev->trans_start = jiffies;

	return 0;

}

/* Set or clear the multicast filter for this adaptor.
   This routine is not state sensitive and need not be SMP locked. */

static void __re8650_set_rx_mode (struct net_device *dev)
{
	/*struct re_private *cp = dev->priv;*/
	u32 mc_filter[2];	/* Multicast hash filter */
	int mode,i;
	/*u32 tmp;*/


	printk("%s: %s %d Still Unimplemented !!!!!!!\n",__FILE__,__FUNCTION__,__LINE__);
	return ;
	/* Note: do not reorder, GCC is clever about common statements. */
	if (dev->flags & IFF_PROMISC) {
		/* Unconditionally log net taps. */
		printk (KERN_NOTICE "%s: Promiscuous mode enabled.\n",
			dev->name);
		mc_filter[1] = mc_filter[0] = 0xffffffff;
	} else if ((dev->mc_count > multicast_filter_limit)
		   || (dev->flags & IFF_ALLMULTI)) {
		/* Too many to filter perfectly -- accept all multicasts. */
		mc_filter[1] = mc_filter[0] = 0xffffffff;
	} else {
		struct dev_mc_list *mclist;
		mc_filter[1] = mc_filter[0] = 0;
		for (i = 0, mclist = dev->mc_list; mclist && i < dev->mc_count;
		     i++, mclist = mclist->next) {
			int bit_nr = ether_crc(ETH_ALEN, mclist->dmi_addr) >> 26;

			mc_filter[bit_nr >> 5] |= 1 << (bit_nr & 31);
		}
	}

	/* We can safely update without stopping the chip. */
}

static void re8650_set_rx_mode (struct net_device *dev)
{
	unsigned long flags;
	struct re_private *cp = dev->priv;

	spin_lock_irqsave (&cp->lock, flags);
	__re8650_set_rx_mode(dev);
	spin_unlock_irqrestore (&cp->lock, flags);
}

static void __re8650_get_stats(struct re_private *cp)
{
	/* XXX implement */
}

static struct net_device_stats *re8650_get_stats(struct net_device *dev)
{
	struct re_private *cp = dev->priv;
	int i;
	/* The chip only need report frame silently dropped. */
	spin_lock_irq(&cp->lock);
 	if (netif_running(dev) && netif_device_present(dev))
 		__re8650_get_stats(cp);
	spin_unlock_irq(&cp->lock);

	return &cp->net_stats;
}

static void re8650_stop_hw (struct re_private *cp)
{


	/* Disable interrupts by clearing the interrupt mask. */
    	REG32(CPUIIMR) = 0x00;
	/* Stop the chip's Tx and Rx DMA processes. */
    	REG32(CPUICR) &= ~(TXCMD | RXCMD);
	synchronize_irq();
	udelay(10);

	cp->rx_tail = 0;
	cp->tx_head = cp->tx_tail = 0;
}

static void re8650_reset_hw (struct re_private *cp)
{
	unsigned work = 1000;
   	REG32(CPUICR) |= SOFTRST;
	printk(KERN_ERR "%s: hardware reset timeout\n", cp->dev->name);
}

static inline void re8650_start_hw (struct re_private *cp)
{
	

#if 0
	cpw8(Cmd, RxOn | TxOn);
	cpw16(CpCmd, PCIMulRW | RxChkSum | CpRxOn | CpTxOn);
#endif
}

static void re8650_init_hw (struct re_private *cp)
{
	struct net_device *dev = cp->dev;
	u8 status;

	//re8650_reset_hw(cp);
    	/* Fill Tx packet header FDP */
    	REG32(CPUTPDCR) = (u32)cp->txPktDesc;
	/* Fill Rx packet header FDP */
    	REG32(CPURPDCR) = (u32)cp->rxPktDesc;
   	/* Fill Rx mbuf FDP */
   	REG32(CPURMDCR) = (u32)cp->rxMblkDesc;
   	REG32(CPUICR) = TXCMD | RXCMD | BUSBURST_32WORDS | MBUF_2048BYTES;
    	REG32(CPUIIMR) |= (RX_ERR_IE | TX_ERR_IE | PKTHDR_DESC_RUNOUT_IE | MBUF_DESC_RUNOUT_IE | 
            TX_ALL_DONE_IE | RX_DONE_IE);
}

static int re8650_refill_rx (struct re_private *cp)
{
	unsigned i;

	for (i = 0; i < RE8650_RX_RING_SIZE; i++) {
		struct sk_buff *skb;
		struct	ring_info *ring;
		skb = dev_alloc_skb(cp->rx_buf_sz + RX_OFFSET);
		if (!skb)
			goto err_out;

		skb->dev = cp->dev;
		ring = &cp->rx_skb[i];
		ring->skb = skb;
		if ((u32)skb->data &0x3)
			printk(KERN_DEBUG "skb->data unaligment %8x\n",(u32)skb->data);
		ring->Mblk = &rxMblkBuff[i];
		ring->PktHdr = &rxPktBuff[i];
		ring->PktHdr->ph_mbuf	= (void*)UNCACHE(ring->Mblk);
		ring->PktHdr->ph_len 	= 0x0;
		ring->PktHdr->ph_flags	= 0x9000;
		ring->Mblk->m_next = 0;
		ring->Mblk->m_pkthdr = (void*)UNCACHE(ring->PktHdr);
		ring->Mblk->m_flags = 0x9c;
		ring->Mblk->m_len = 0x0;
		ring->Mblk->m_data = (void*)UNCACHE(skb->data);
		cp->rxPktDesc[i].opts1 = UNCACHE(ring->PktHdr)|DESC_OWNED_BIT;
		cp->rxMblkDesc[i].opts1 = UNCACHE(ring->Mblk)|DESC_OWNED_BIT;
 
	}
	cp->rxPktDesc[RE8650_RX_RING_SIZE-1].opts1 |= DESC_WRAP_BIT;
	cp->rxMblkDesc[RE8650_RX_RING_SIZE-1].opts1 |=DESC_WRAP_BIT;

	return 0;

err_out:
	re8650_clean_rings(cp);
	return -ENOMEM;
}

static void re8650_tx_timeout (struct net_device *dev)
{
	struct re_private *cp = dev->priv;
	
	cp->cp_stats.tx_timeouts++;

	spin_lock_irq(&cp->lock);
	re8650_tx(cp);
	spin_unlock_irq(&cp->lock);
	netif_wake_queue(cp->dev);
	
}
static int re8650_init_rings (struct re_private *cp)
{
	int i;
	struct rtl_pktHdr 	*pktHdr;	
	struct rtl_mBuf		*Mblk;

	
	for(i=0;i<RE8650_TX_RING_SIZE;i++)
	{
		cp->txPktDesc[i].opts1 = UNCACHE(&cp->txPktHdr[i]); 
		cp->txPktHdr[i] = (struct rtl_pktHdr*) UNCACHE(&txPktBuff[i]);
		cp->txMBlk[i] = (struct rtl_mBuf*)UNCACHE(&txMblkBuff[i]);
		pktHdr = cp->txPktHdr[i];
		Mblk = cp->txMBlk[i];
		pktHdr->ph_mbuf = (void*)UNCACHE(cp->txMBlk[i]);
		Mblk->m_pkthdr = (void*)UNCACHE(cp->txPktHdr[i]);
	}
	cp->txPktDesc[RE8650_TX_RING_SIZE-1].opts1 |= DESC_WRAP_BIT;
	return re8650_refill_rx (cp);
}

static int re8650_alloc_rings (struct re_private *cp)
{
	void*	pBuf;
	if (cp!= reDev )	 for(;;);
	pBuf = kmalloc(RE8650_RXRING_BYTES,GFP_KERNEL);
	if (!pBuf)
		goto ErrMem;
	memset(pBuf, 0, RE8650_RXRING_BYTES);
	//pBuf = (void*)( (u32)(pBuf + DESC_ALIGN-1) &  ~(DESC_ALIGN -1) ) ;
	cp->rxPktDesc = (DMA_INFO*)((u32)(pBuf) | UNCACHE_MASK);
	pBuf = kmalloc(RE8650_RXRING_BYTES,GFP_KERNEL);
	if (!pBuf)
		goto ErrMem;
	memset(pBuf, 0, RE8650_RXRING_BYTES);
	cp->rxMblkDesc = (DMA_INFO*)((u32)(pBuf) | UNCACHE_MASK);


	pBuf= kmalloc(RE8650_TXRING_BYTES, GFP_KERNEL);
	if (!pBuf)
		goto ErrMem;
	memset(pBuf, 0, RE8650_TXRING_BYTES);
	cp->txPktDesc = (DMA_INFO*)((u32)(pBuf) | UNCACHE_MASK);
	
	return re8650_init_rings(cp);
ErrMem:
	if (cp->rxPktDesc)
		kfree(cp->rxPktDesc);
	if (cp->rxMblkDesc)
		kfree(cp->rxMblkDesc);

	return -ENOMEM;

	
}

static void re8650_clean_rings (struct re_private *cp)
{
	unsigned i;


	for (i = 0; i < RE8650_RX_RING_SIZE; i++) {
		if (cp->rx_skb[i].skb) {
			dev_kfree_skb(cp->rx_skb[i].skb);
		}
	}

	for (i = 0; i < RE8650_TX_RING_SIZE; i++) {
		if (cp->tx_skb[i].skb) {
			struct sk_buff *skb = cp->tx_skb[i].skb;
			dev_kfree_skb(skb);
			cp->net_stats.tx_dropped++;
		}
	}

	memset(&cp->rx_skb, 0, sizeof(struct ring_info) * RE8650_RX_RING_SIZE);
	memset(&cp->tx_skb, 0, sizeof(struct ring_info) * RE8650_TX_RING_SIZE);
}

static void re8650_free_rings (struct re_private *cp)
{
	re8650_clean_rings(cp);
}

static int re8650_open (struct net_device *dev)
{
	struct re_private *cp = dev->priv;
	u_int32_t portmask, type;
	uint16 fid, sid;

	int rc;

	if (netif_msg_ifup(cp))
		printk(KERN_DEBUG "%s: enabling interface\n", dev->name);
	re8650_reset_hw(cp);	
	swCore_init();
#if 0
	rtl8651_tblDrvInit(NULL);
	fid=0; sid=0;
	rtl8651_addSpanningTreeInstance(sid);
	rtl8651_setSpanningTreeInstanceProtocolWorking(sid, FALSE);
	rtl8651_addFilterDatabase(fid);
	rtl8651_specifyFilterDatabaseSpanningTreeInstance(fid, sid);
	type = RTL8651_FORWARD_MAC;
	mac.octet[0] = 0xFF;mac.octet[1] = 0xFF;mac.octet[2] = 0xFF;mac.octet[3] = 0xFF;mac.octet[4] = 0xFF;mac.octet[5] = 0xFF;
		portmask=0xFFFFFFFF;
		rtl8651_addFilterDatabaseEntry(fid, &mac,type,portmask);/*tblDrv filter-database 0 add ff:ff:ff:ff:ff:ff forward 0xffffffff*/
#endif
	re8650_set_rxbufsize(cp);	/* set new rx buf size */
	rc = re8650_alloc_rings(cp);
	if (rc)
		return rc;

	re8650_init_hw(cp);
	//port_config(cp);

	vlan_config(cp);
	rc = request_irq(dev->irq, re8650_interrupt, SA_INTERRUPT, dev->name, dev);
	if (rc)
		goto err_out_hw;

	netif_start_queue(dev);

	return 0;

err_out_hw:
	re8650_stop_hw(cp);
	re8650_free_rings(cp);
	return rc;
}

static int re8650_close (struct net_device *dev)
{
	struct re_private *cp = dev->priv;

	if (netif_msg_ifdown(cp))
		printk(KERN_DEBUG "%s: disabling interface\n", dev->name);

	netif_stop_queue(dev);
	re8650_stop_hw(cp);
	free_irq(dev->irq, dev);
	re8650_free_rings(cp);
	return 0;
}

static int re8650_change_mtu(struct net_device *dev, int new_mtu)
{
	struct re_private *cp = dev->priv;
	int rc;

	/* check for invalid MTU, according to hardware limits */
	if (new_mtu < CP_MIN_MTU || new_mtu > CP_MAX_MTU)
		return -EINVAL;

	/* if network interface not up, no need for complexity */
	if (!netif_running(dev)) {
		dev->mtu = new_mtu;
		re8650_set_rxbufsize(cp);	/* set new rx buf size */
		return 0;
	}

	spin_lock_irq(&cp->lock);

	re8650_stop_hw(cp);			/* stop h/w and free rings */
	re8650_clean_rings(cp);

	dev->mtu = new_mtu;
	re8650_set_rxbufsize(cp);		/* set new rx buf size */

	rc = re8650_init_rings(cp);		/* realloc and restart h/w */
	re8650_start_hw(cp);
	spin_unlock_irq(&cp->lock);

	return rc;
}



static int re8650_ethtool_ioctl (struct re_private *cp, void *useraddr)
{
	u32 ethcmd;

	/* dev_ioctl() in ../../net/core/dev.c has already checked
	   capable(CAP_NET_ADMIN), so don't bother with that here.  */

	if (get_user(ethcmd, (u32 *)useraddr))
		return -EFAULT;

	switch (ethcmd) {

	case ETHTOOL_GDRVINFO: {
		struct ethtool_drvinfo info = { ETHTOOL_GDRVINFO };
		strcpy (info.driver, DRV_NAME);
		strcpy (info.version, DRV_VERSION);
		strcpy (info.bus_info, cp->pdev->slot_name);
		if (copy_to_user (useraddr, &info, sizeof (info)))
			return -EFAULT;
		return 0;
	}

	/* get settings */
	case ETHTOOL_GSET: {
		struct ethtool_cmd ecmd = { ETHTOOL_GSET };
		spin_lock_irq(&cp->lock);
		spin_unlock_irq(&cp->lock);
		if (copy_to_user(useraddr, &ecmd, sizeof(ecmd)))
			return -EFAULT;
		return 0;
	}
	/* set settings */
	case ETHTOOL_SSET: {
		int r;
		struct ethtool_cmd ecmd;
		if (copy_from_user(&ecmd, useraddr, sizeof(ecmd)))
			return -EFAULT;
		spin_lock_irq(&cp->lock);
		spin_unlock_irq(&cp->lock);
		return r;
	}
	/* restart autonegotiation */
	/* get link status */
	case ETHTOOL_GLINK: {
		struct ethtool_value edata = {ETHTOOL_GLINK};
		if (copy_to_user(useraddr, &edata, sizeof(edata)))
			return -EFAULT;
		return 0;
	}

	/* get message-level */
	case ETHTOOL_GMSGLVL: {
		struct ethtool_value edata = {ETHTOOL_GMSGLVL};
		edata.data = cp->msg_enable;
		if (copy_to_user(useraddr, &edata, sizeof(edata)))
			return -EFAULT;
		return 0;
	}
	/* set message-level */
	case ETHTOOL_SMSGLVL: {
		struct ethtool_value edata;
		if (copy_from_user(&edata, useraddr, sizeof(edata)))
			return -EFAULT;
		cp->msg_enable = edata.data;
		return 0;
	}

	default:
		break;
	}

	return -EOPNOTSUPP;
}


static int re8650_ioctl (struct net_device *dev, struct ifreq *rq, int cmd)
{
	struct re_private *cp = dev->priv;
	int rc = 0;

	if (!netif_running(dev))
		return -EINVAL;

	switch (cmd) {
	case SIOCETHTOOL:
		return re8650_ethtool_ioctl(cp, (void *) rq->ifr_data);

	default:
		rc = -EOPNOTSUPP;
		break;
	}

	return rc;
}

#if CP_VLAN_TAG_USED
static void cp_vlan_rx_register(struct net_device *dev, struct vlan_group *grp)
{
	struct re_private *cp = dev->priv;

	spin_lock_irq(&cp->lock);
	cp->vlgrp = grp;
	cpw16(CpCmd, cpr16(CpCmd) | RxVlanOn);
	spin_unlock_irq(&cp->lock);
}

static void cp_vlan_rx_kill_vid(struct net_device *dev, unsigned short vid)
{
	struct re_private *cp = dev->priv;

	spin_lock_irq(&cp->lock);
	cpw16(CpCmd, cpr16(CpCmd) & ~RxVlanOn);
	if (cp->vlgrp)
		cp->vlgrp->vlan_devices[vid] = NULL;
	spin_unlock_irq(&cp->lock);
}
#endif

/* Serial EEPROM section. */

/*  EEPROM_Ctrl bits. */
#define EE_SHIFT_CLK	0x04	/* EEPROM shift clock. */
#define EE_CS			0x08	/* EEPROM chip select. */
#define EE_DATA_WRITE	0x02	/* EEPROM chip data in. */
#define EE_WRITE_0		0x00
#define EE_WRITE_1		0x02
#define EE_DATA_READ	0x01	/* EEPROM chip data out. */
#define EE_ENB			(0x80 | EE_CS)

/* Delay between EEPROM clock transitions.
   No extra delay is needed with 33Mhz PCI, but 66Mhz may change this.
 */

#define eeprom_delay()	readl(ee_addr)

/* The EEPROM commands include the alway-set leading bit. */
#define EE_WRITE_CMD	(5)
#define EE_READ_CMD		(6)
#define EE_ERASE_CMD	(7)



static void __devexit cp_remove_one (struct pci_dev *pdev)
{
	struct net_device *dev = pci_get_drvdata(pdev);
	struct re_private *cp = dev->priv;

	if (!dev)
		BUG();
	unregister_netdev(dev);
	iounmap(cp->regs);
	kfree(dev);
}


int __init re8650_probe (void)
{
#ifdef MODULE
	printk("%s", version);
#endif

	struct net_device *dev;
	struct re_private *cp;
	int rc;
	void *regs=(void*)0xb9800000;
	unsigned addr_len, i;

#ifndef MODULE
	static int version_printed;
	if (version_printed++ == 0)
		printk("%s", version);
#endif

	dev = alloc_etherdev(sizeof(struct re_private));
	
	if (!dev)
		return -ENOMEM;
	SET_MODULE_OWNER(dev);
	cp = dev->priv;
	memset(cp,0,sizeof(*cp));
	reDev = (struct re_private*)dev->priv;
	cp->dev = dev;
	spin_lock_init (&cp->lock);
	
	
	dev->base_addr = (unsigned long) regs;
	cp->regs = regs;

	re8650_stop_hw(cp);

	/* read MAC address from EEPROM */
	dev->dev_addr[0] = 0;
	dev->dev_addr[1] = 0xc0;
	dev->dev_addr[2] = 0x50;
	dev->dev_addr[3] = 0x80;
	dev->dev_addr[4] = 0xd0;
	dev->dev_addr[5] = 0xdd;

	dev->open = re8650_open;
	dev->stop = re8650_close;
	dev->set_multicast_list = re8650_set_rx_mode;
	dev->hard_start_xmit = re8650_start_xmit;
	dev->get_stats = re8650_get_stats;
	dev->do_ioctl = re8650_ioctl;
#if 1 
	dev->tx_timeout = re8650_tx_timeout;
	dev->watchdog_timeo = TX_TIMEOUT;
#endif
#ifdef CP_TX_CHECKSUM
	sd
	dev->features |= NETIF_F_SG | NETIF_F_IP_CSUM;
#endif
#if CP_VLAN_TAG_USED
	dev->features |= NETIF_F_HW_VLAN_TX | NETIF_F_HW_VLAN_RX;
	dev->vlan_rx_register = cp_vlan_rx_register;
	dev->vlan_rx_kill_vid = cp_vlan_rx_kill_vid;
#endif

	dev->irq = ICU_NIC;

	rc = register_netdev(dev);
	if (rc)
		goto err_out_iomap;

	printk (KERN_INFO "%s: %s at 0x%lx, "
		"%02x:%02x:%02x:%02x:%02x:%02x, "
		"IRQ %d\n",
		dev->name,
		"RTL-8139C+",
		dev->base_addr,
		dev->dev_addr[0], dev->dev_addr[1],
		dev->dev_addr[2], dev->dev_addr[3],
		dev->dev_addr[4], dev->dev_addr[5],
		dev->irq);


	/*
	 * Looks like this is necessary to deal with on all architectures,
	 * even this %$#%$# N440BX Intel based thing doesn't get it right.
	 * Ie. having two NICs in the machine, one will have the cache
	 * line set at boot time, the other will not.
	 */
	return 0;

err_out_iomap:
	iounmap(regs);
	kfree(dev);
	return -1 ;


	
}

static void __exit re8650_exit (void)
{
}

module_init(re8650_probe);
module_exit(re8650_exit);
