/* re8601.c: A Linux Ethernet driver for the RealTek 8601 chips. */
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
#include <linux/slab.h>
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
MODULE_DESCRIPTION("RealTek RTL-8601 series 10/100 Ethernet driver");
MODULE_LICENSE("GPL");

static int debug = -1;
MODULE_PARM (debug, "i");
MODULE_PARM_DESC (debug, "re8601 bitmapped message enable number");

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
#define RE8601_RX_RING_SIZE	64
#define RE8601_TX_RING_SIZE	32
#define DESC_ALIGN		0x100
#define UNCACHE_MASK		0xa0000000

#define RE8601_RXRING_BYTES	( (sizeof(struct dma_desc) * (RE8601_RX_RING_SIZE+1)) + DESC_ALIGN)

#define RE8601_TXRING_BYTES	( (sizeof(struct dma_desc) * (RE8601_TX_RING_SIZE+1)) + DESC_ALIGN)
		


#define NEXT_TX(N)		(((N) + 1) & (RE8601_TX_RING_SIZE - 1))
#define NEXT_RX(N)		(((N) + 1) & (RE8601_RX_RING_SIZE - 1))
#define TX_HQBUFFS_AVAIL(CP)					\
	(((CP)->tx_hqtail <= (CP)->tx_hqhead) ?			\
	  (CP)->tx_hqtail + (RE8601_TX_RING_SIZE - 1) - (CP)->tx_hqhead :	\
	  (CP)->tx_hqtail - (CP)->tx_hqhead - 1)

#define PKT_BUF_SZ		1536	/* Size of each temporary Rx buffer.*/
#define RX_OFFSET		2

/* The following settings are log_2(bytes)-4:  0 == 16 bytes .. 6==1024, 7==end of packet. */

/* Time in jiffies before concluding the transmitter is hung. */
#define TX_TIMEOUT		(6*HZ)

/* hardware minimum and maximum for a single frame's data payload */
#define CP_MIN_MTU		60	/* TODO: allow lower, but pad */
#define CP_MAX_MTU		4096

#if 0
#define RTL_W32(reg, value)			(*(volatile u32*)(ioaddr+reg)) = (u32)value
#define RTL_W16(reg, value)			(*(volatile u16*)(ioaddr+reg)) = (u16)value
#define RTL_W8(reg, value)			(*(volatile u8*)(ioaddr+reg)) = (u8)value
#define RTL_R32(reg)				(*(volatile u32*)(ioaddr+reg))
#define RTL_R16(reg)				(*(volatile u16*)(ioaddr+reg))
#define RTL_R8(reg)					(*(volatile u8*)(ioaddr+reg))
#else
#define ETHBASE	0xbd012000
#define RTL_W32(reg, value)			(*(volatile u32*)(ETHBASE+reg)) = (u32)value
#define RTL_W16(reg, value)			(*(volatile u16*)(ETHBASE+reg)) = (u16)value
#define RTL_W8(reg, value)			(*(volatile u8*)(ETHBASE+reg)) = (u8)value
#define RTL_R32(reg)				(*(volatile u32*)(ETHBASE+reg))
#define RTL_R16(reg)				(*(volatile u16*)(ETHBASE+reg))
#define RTL_R8(reg)					(*(volatile u8*)(ETHBASE+reg))
#endif



enum PHY_REGS{
	TXFCE= 1<<7,
	RXFCE=1 <<6,
	SP100=1<<3,
	LINK=1<<2,
	TXPF=1<<1,
	RXPF=1<<0,
	FORCE_TX = 1<<5

};
	
enum {
	/* NIC register offsets */
	CNR1 = 0,
	IDR0 = 0,			/* Ethernet ID */
	IDR1 = 0x1,			/* Ethernet ID */
	IDR2 = 0x2,			/* Ethernet ID */
	IDR3 = 0x3,			/* Ethernet ID */
	IDR4 = 0x4,			/* Ethernet ID */
	IDR5 = 0x5,			/* Ethernet ID */
	MAR0 = 0x8,			/* Multicast register */
	MAR1 = 0x9,
	MAR2 = 0xa,
	MAR3 = 0xb,
	MAR4 = 0xc,
	MAR5 = 0xd,
	MAR6 = 0xe,
	MAR7 = 0xf,
	TXOKCNT=0x10,
	RXOKCNT=0x12,
	TXERR = 0x14,
	RXERRR = 0x16,
	MISSPKT = 0x18,
	FAE=0x1A,
	TX1COL = 0x1c,
	TXMCOL=0x1e,
	RXOKPHY=0x20,
	RXOKBRD=0x22,
	RXOKMUL=0x24,
	TXABT=0x26,
	TXUNDRN=0x28,
	TRSR=0x34,
	CMD=0x00,
	ID0=0x04,
	ID1=0x08,
	TSAD=0x14,
	RSAD=0x18,
	IMTR=0x1c,
	IMR=0x20,
	ISR=0x24,
	TMF0=0x28,
	TMF1=0x2c,
	TMF2=0x30,
	TMF3=0x34,
	MII=0x38,
	CNR2=0x3c,
	TCR=0x40,
	RCR=0x44,
	MSR=0x58,
	MIIAR=0x5C,
	TxFDP1=0x1300,
	TxCDO1=0x1304,
	TXCPO1=0x1308,
	TXPSA1=0x130A,
	TXCPA1=0x130C,
	LastTxCDO1=0x1310,
	TXPAGECNT1=0x1312,
	Tx1ScratchDes=0x1350,
	TxFDP2=0x1380,
	TxCDO2=0x1384,
	TXCPO2=0x1388,
	TXPSA2=0x138A,
	TXCPA2=0x138C,
	LASTTXCDO2=0x1390,
	TXPAGECNT2=0x1392,
	Tx2ScratchDes=0x13A0,
	RxFDP=0x13F0,
	RxCDO=0x13F4,
	RxRingSize=0x13F6,
	RxCPO=0x13F8,
	RxPSA=0x13FA,
	RXPLEN=0x1403,
	//LASTRXCDO=0x1402,
	RXPFDP=0x1404,
	RXPAGECNT=0x1408,
	RXSCRATCHDES=0x1410,
	EthrntRxCPU_Des_Num=0x1430,
	EthrntRxCPU_Des_Wrap =	0x1431,
	Rx_Pse_Des_Thres = 	0x1432,	
	
	IO_CMD = 0x1434,
	RX_IntNum_Shift = 0x8,             /// ????
	TX_OWN = (1<<5),
	RX_OWN = (1<<4),
	MII_RE = (1<<3),
	MII_TE = (1<<2),
	TX_FNL = (1<1),
	TX_FNH = (1),
	/*TX_START= MII_RE | MII_TE | TX_FNL | TX_FNH,
	TX_START = 0x8113c,*/
	RXMII = MII_RE,
	TXMII= MII_TE,


	


};

enum RE8601_STATUS_REGS
{
	/* Tx and Rx status descriptors */
	DescOwn		= (1 << 31), /* Descriptor is owned by NIC */
	RingEnd		= (1 << 30), /* End of descriptor ring */
	FirstFrag	= (1 << 29), /* First segment of a packet */
	LastFrag	= (1 << 28), /* Final segment of a packet */
	TxError		= (1 << 23), /* Tx error summary */
	RxError		= (1 << 20), /* Rx error summary */
	IPCS		= (1 << 18), /* Calculate IP checksum */
	UDPCS		= (1 << 17), /* Calculate UDP/IP checksum */
	TCPCS		= (1 << 16), /* Calculate TCP/IP checksum */
	TxVlanTag	= (1 << 17), /* Add VLAN tag */
	RxVlanTagged	= (1 << 16), /* Rx VLAN tag available */
	IPFail		= (1 << 15), /* IP checksum failed */
	UDPFail		= (1 << 14), /* UDP/IP checksum failed */
	TCPFail		= (1 << 13), /* TCP/IP checksum failed */
	NormalTxPoll	= (1 << 6),  /* One or more normal Tx packets to send */
	PID1		= (1 << 17), /* 2 protocol id bits:  0==non-IP, */
	PID0		= (1 << 16), /* 1==UDP/IP, 2==TCP/IP, 3==IP */
	RxProtoTCP	= 1,
	RxProtoUDP	= 2,
	RxProtoIP	= 3,
	TxFIFOUnder	= (1 << 25), /* Tx FIFO underrun */
	TxOWC		= (1 << 22), /* Tx Out-of-window collision */
	TxLinkFail	= (1 << 21), /* Link failed during Tx of packet */
	TxMaxCol	= (1 << 20), /* Tx aborted due to excessive collisions */
	TxColCntShift	= 16,	     /* Shift, to get 4-bit Tx collision cnt */
	TxColCntMask	= 0x01 | 0x02 | 0x04 | 0x08, /* 4-bit collision count */
	RxErrFrame	= (1 << 27), /* Rx frame alignment error */
	RxMcast		= (1 << 26), /* Rx multicast packet rcv'd */
	RxErrCRC	= (1 << 18), /* Rx CRC error */
	RxErrRunt	= (1 << 19), /* Rx error, packet < 64 bytes */
	RWT		= (1 << 21), /* Rx  */
	E8023		= (1 << 22), /* Receive Ethernet 802.3 packet  */
	TxCRC		= (1 <<23),
	
	RxVlanOn	= (1 << 2),  /* Rx VLAN de-tagging enable */
	RxChkSum	= (1 << 1),  /* Rx checksum offload enable */		

};

enum RE8601_THRESHOLD_REGS{
	THVAL		= 2,
	RINGSIZE	= 2,

	LOOPBACK	= (0x3 << 8),
 	AcceptErr	= 0x100,     /* Accept packets with CRC errors */
	AcceptRunt	= 0x04,	     /* Accept runt (<64 bytes) packets */
	AcceptBroadcast	= 0x80,	     /* Accept broadcast packets */
	AcceptMulticast	= 0x40,	     /* Accept multicast packets */
	AcceptMyPhys	= 0x20,	     /* Accept pkts with our MAC as dest */
	AcceptAllPhys	= 0x10,	     /* Accept all pkts w/ physical dest */
	AcceptAll = AcceptBroadcast | AcceptMulticast | AcceptMyPhys |  AcceptAllPhys | AcceptErr | AcceptRunt,
	AcceptNoAllPhys = AcceptBroadcast | AcceptMulticast | AcceptMyPhys | AcceptErr | AcceptRunt,
	AcceptNoBroad = AcceptMulticast |AcceptMyPhys |  AcceptAllPhys | AcceptErr | AcceptRunt,
	AcceptNoMulti =  AcceptMyPhys |  AcceptAllPhys | AcceptErr | AcceptRunt,
	NoErrAccept = AcceptBroadcast | AcceptMulticast | AcceptMyPhys |  AcceptAllPhys,
	
};




enum RE8601_ISR_REGS{

	SW_INT 		= (1 <<0),
	TX_EMPTY	= (1 << 1),
	LINK_CHG	= (1 <<	9),
	TX_ERR		= (1 << 7),
	TX_OK		= (1 << 5),
	RX_EMPTY	= (1 << 3),
	RX_FIFOOVR	=(1 << 4),
	RX_ERR		=(1 << 8),
	RUNT_ERR 	=(1<<19),
	RX_OK		= (1 << 6),


};

enum RE8601_IOCMD_REG
{
	RX_MIT 		= 3,
	RX_TIMER 	= 1,
	RX_FIFO 	= 2,
	TX_FIFO		= 1,
	TX_MIT		= 3,	
	TX_POLL		= 1 << 0,
	CMD_CONFIG = 0x3c | RX_MIT << 8 | RX_FIFO << 11 |  RX_TIMER << 13 | TX_MIT << 16 | TX_FIFO<<19,
};

enum RE8601_MII_REGS{

	MMIMODE		= 0x00200000,
	PHYAD		= 0x01,
	PHYAD_SH	= 16,
	DISNWAY		= 0x00000010,
	LINK_CTRL	= 0x00008000,
	DPLX_CTRL	= 0x00004000,
	SPD_CTRL	= 0x00002000,
	FL_CTRL		= 0x00001000,
};

/* Type match filter setting */
#define TP0			0x0800
#define TP1			0x8100
#define TP2			0x0806
#define TP3			0x8860
#define TFMSK0			0xffff
#define TFMSK1			0xffff
#define TFMSK2			0xffff
#define TFMSK3			0xfff0
#define TFMSK_SH		16

/* Control register 1 setting */
#define RXBLEN					0x2
#define TXBLEN					0x4
#define RXBLEN_SH				30
#define TXBLEN_SH				26
#define TDFN					0x01000000
#define RESET					0x00100000
#define RX_ENABLE				0x00080000
#define TX_ENABLE				0x00040000
#define UNTAG_ENABLE				0x00004000
#define CNR_CMD					0x000f20a8

/* Control register 2 setting */
#define TXPON					0x8000
#define TXPOFF					0x4000
#define PTMASK					0x0
#define PTMASK_SH				8
#define IFG					0x3
#define IFG_SH					4

/* Mitigation setting */
#define MIT_TXTIMER				0x0
#define MIT_TXNUM				0x0
#define MIT_RXTIMER				0x0
#define MIT_RXNUM				0x0

/*
static const u32 re8601_intr_mask =
	RX_OK | RX_ERR | RX_EMPTY | RX_FIFOOVR |
	TX_OK | TX_ERR | TX_EMPTY;
*/
static const u32 re8601_intr_mask =
	RX_OK | RX_EMPTY | RX_FIFOOVR |
	TX_OK;

typedef struct dma_desc {
	u32		opts1;
	u32		opts2;
	u32		addr;
	u32		opts3;

}DMA_DESC;


struct ring_info {
	struct sk_buff		*skb;
	dma_addr_t		mapping;
	unsigned		frag;
};

struct cp_extra_stats {
	unsigned long		rx_frags;
	unsigned long tx_timeouts;
};

struct re_private {
	unsigned		tx_hqhead;
	unsigned		tx_hqtail;
	unsigned		tx_lqhead;
	unsigned		tx_lqtail;
	unsigned		rx_tail;

	void			*regs;
	struct net_device	*dev;
	spinlock_t		lock;

	DMA_DESC		*rx_ring;
	
	DMA_DESC		*tx_hqring;
	DMA_DESC		*tx_lqring;
	struct ring_info	tx_skb[RE8601_TX_RING_SIZE];
	struct ring_info	rx_skb[RE8601_RX_RING_SIZE];
	unsigned		rx_buf_sz;
	dma_addr_t		ring_dma;

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
	char*			rxdesc_buf;
	char*			txdesc_buf;
	struct mii_if_info	mii_if;
};
struct re_private *reDev;
static void __re8601_set_rx_mode (struct net_device *dev);
static void re8601_tx (struct re_private *cp);
static void re8601_clean_rings (struct re_private *cp);
static void re8601_tx_timeout (struct net_device *dev);

// Kao
static void eth_set_mac_address(struct net_device *dev, unsigned char* enaddr)
{
	unsigned int mac0, mac1;

#ifdef ETH_DRV_DEBUG
	// debug
	{
		static int s_nCount = 0;
		int i;
		diag_printf("Set MAC address [%d]: ", ++s_nCount);
		for (i = 0; i < 6; i++)
			diag_printf("%02x%c", enaddr[i], i != 5 ? ':' : '\n');
	}
#endif
	
	// backup MAC address
	memcpy(dev->dev_addr, enaddr, 6);

	// write the address to NIC
	mac0 = enaddr[0] | enaddr[1] << 8 | enaddr[2] << 16 | enaddr[3] << 24;
	mac1 = enaddr[4] | enaddr[5] << 8;
	
	RTL_W32(ID0, mac0);
	RTL_W32(ID1, mac1);
}

static inline void re8601_set_rxbufsize (struct re_private *cp)
{
	unsigned int mtu = cp->dev->mtu;
	
	if (mtu > ETH_DATA_LEN)
		/* MTU + ethernet header + FCS + optional VLAN tag */
		cp->rx_buf_sz = mtu + ETH_HLEN + 8;
	else
		cp->rx_buf_sz = PKT_BUF_SZ;
}

static inline void re8601_rx_skb (struct re_private *cp, struct sk_buff *skb,
			      DMA_DESC *desc)
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
		netif_rx(skb);
}

static void re8601_rx_err_acct (struct re_private *cp, unsigned rx_tail,
			    u32 status, u32 len)
{
	if (netif_msg_rx_err (cp))
		printk (KERN_DEBUG
			"%s: rx err, slot %d status 0x%x len %d\n",
			cp->dev->name, rx_tail, status, len);
	cp->net_stats.rx_errors++;
	if (status & RxErrFrame)
		cp->net_stats.rx_frame_errors++;
	if (status & RxErrCRC)
		cp->net_stats.rx_crc_errors++;
	if (status & RxErrRunt)
		cp->net_stats.rx_length_errors++;
}

static void re8601_rx_frag (struct re_private *cp, unsigned rx_tail,
			struct sk_buff *skb, u32 status, u32 len)
{
	struct sk_buff *copy_skb, *frag_skb = cp->frag_skb;
	unsigned orig_len = frag_skb ? frag_skb->len : 0;
	unsigned target_len = orig_len + len;
	unsigned first_frag = status & FirstFrag;
	unsigned last_frag = status & LastFrag;

	if (netif_msg_rx_status (cp))
		printk (KERN_DEBUG "%s: rx %s%sfrag, slot %d status 0x%x len %d\n",
			cp->dev->name,
			cp->dropping_frag ? "dropping " : "",
			first_frag ? "first " :
			last_frag ? "last " : "",
			rx_tail, status, len);

	cp->cp_stats.rx_frags++;

	if (!frag_skb && !first_frag)
		cp->dropping_frag = 1;
	if (cp->dropping_frag)
		goto drop_frag;

	copy_skb = dev_alloc_skb (target_len + RX_OFFSET);
	if (!copy_skb) {
		printk(KERN_WARNING "%s: rx slot %d alloc failed\n",
		       cp->dev->name, rx_tail);

		cp->dropping_frag = 1;
drop_frag:
		if (frag_skb) {
			dev_kfree_skb_irq(frag_skb);
			cp->frag_skb = NULL;
		}
		if (last_frag) {
			cp->net_stats.rx_dropped++;
			cp->dropping_frag = 0;
		}
		return;
	}

	copy_skb->dev = cp->dev;
	// Kao, 8601 doesn't do 2 bytes offset
	//skb_reserve(copy_skb, RX_OFFSET);
	skb_put(copy_skb, target_len);
	if (frag_skb) {
		memcpy(copy_skb->data, frag_skb->data, orig_len);
		printk("frag fragdata=\n%s\n", frag_skb->data);
		dev_kfree_skb_irq(frag_skb);
	}
#if 0
	pci_dma_sync_single(cp->pdev, cp->rx_skb[rx_tail].mapping,
			    len, PCI_DMA_FROMDEVICE);
#endif
	memcpy(copy_skb->data + orig_len, skb->data, len);
	printk("frag data=\n%s\n", skb->data);

	copy_skb->ip_summed = CHECKSUM_NONE;

	if (last_frag) {
		if (status & (RxError)) {
			re8601_rx_err_acct(cp, rx_tail, status, len);
			dev_kfree_skb_irq(copy_skb);
		} else
			re8601_rx_skb(cp, copy_skb, &cp->rx_ring[rx_tail]);
		cp->frag_skb = NULL;
	} else {
		cp->frag_skb = copy_skb;
	}
}

static inline unsigned int re8601_rx_csum_ok (u32 status)
{
	unsigned int protocol = (status >> 16) & 0x3;
	
	if (likely((protocol == RxProtoTCP) && (!(status & TCPFail))))
		return 1;
	else if ((protocol == RxProtoUDP) && (!(status & UDPFail)))
		return 1;
	else if ((protocol == RxProtoIP) && (!(status & IPFail)))
		return 1;
	return 0;
}

static void re8601_rx (struct re_private *cp)
{
	unsigned rx_tail = cp->rx_tail;
	unsigned rx_work = 100;

	while (rx_work--) {
		u32 status, len;
		dma_addr_t mapping;
		struct sk_buff *skb, *new_skb;
		DMA_DESC *desc;
		unsigned buflen;

		skb = cp->rx_skb[rx_tail].skb;
		if (!skb)
			BUG();

		desc = &cp->rx_ring[rx_tail];
		status = desc->opts1;
		if (status & DescOwn)
			break;

		len = (status & 0x0fff) - 4;
		/*
		printk("rxlen=%d, desc#=%d\n", len, rx_tail);
		int *du = (int *)skb->data;
		int i;
		printk("skbdata=\n");
		for (i=0; i<10; i++)
		{
			printk("%8x", du[i]);
		}
		printk("\n");
		*/
		
		mapping = cp->rx_skb[rx_tail].mapping;

		if ((status & (FirstFrag | LastFrag)) != (FirstFrag | LastFrag)) {
			re8601_rx_frag(cp, rx_tail, skb, status, len);
			goto rx_next;
		}

		if (status & (RxError)) {
			re8601_rx_err_acct(cp, rx_tail, status, len);
			goto rx_next;
		}

		if (netif_msg_rx_status(cp))
			printk(KERN_DEBUG "%s: rx slot %d status 0x%x len %d\n",
			       cp->dev->name, rx_tail, status, len);

		buflen = cp->rx_buf_sz + RX_OFFSET;
		new_skb = dev_alloc_skb (buflen);
		if (!new_skb) {
			cp->net_stats.rx_dropped++;
			goto rx_next;
		}
		if ((u32)new_skb->data &0x3)
			printk(KERN_DEBUG "new_skb->data unaligment %8x\n",(u32)new_skb->data);

		/*skb_reserve(new_skb, RX_OFFSET);*/
		new_skb->dev = cp->dev;
#if 0
		pci_unmap_single(cp->pdev, mapping,
				 buflen, PCI_DMA_FROMDEVICE);
#endif
		
		/* Handle checksum offloading for incoming packets. */
		if (re8601_rx_csum_ok(status))
			skb->ip_summed = CHECKSUM_UNNECESSARY;
		else
			skb->ip_summed = CHECKSUM_NONE;

		// KaO, 8601 doesn't do the two bytes offset
		//skb_reserve(skb, RX_OFFSET);
		//dma_cache_wback_inv((unsigned long)skb->data, len);
		skb_put(skb, len);
		mapping =
		cp->rx_skb[rx_tail].mapping = (u32)new_skb->tail | UNCACHE_MASK;
#if 0
			pci_map_single(cp->pdev, new_skb->tail,

 					buflen, PCI_DMA_FROMDEVICE);
#endif
		re8601_rx_skb(cp, skb, desc);
		
		cp->rx_skb[rx_tail].skb = new_skb;
		cp->rx_ring[rx_tail].addr = mapping;

rx_next:
		if (rx_tail == (RE8601_RX_RING_SIZE - 1))
			desc->opts1 = (DescOwn | RingEnd |
						  cp->rx_buf_sz);
		else
			desc->opts1 = (DescOwn | cp->rx_buf_sz);
		cp->rx_ring[rx_tail].opts2 = 0;
		rx_tail = NEXT_RX(rx_tail);
	}

	if (!rx_work)
		printk(KERN_WARNING "%s: rx work limit reached\n", cp->dev->name);

	cp->rx_tail = rx_tail;
}

static void re8601_interrupt (int irq, void *dev_instance, struct pt_regs *regs)
{
	struct net_device *dev = dev_instance;
	struct re_private *cp = dev->priv;
	u32 status;

	status = RTL_R32(ISR);
	if (!status || (status == 0xFFFF))
		return;

	if (netif_msg_intr(cp))
		printk(KERN_DEBUG "%s: intr, status %04x cmd %02x \n",
		        dev->name, status, RTL_R8(CMD));

	spin_lock(&cp->lock);

	if (status & (RX_OK | RX_ERR | RX_EMPTY | RX_FIFOOVR))
		re8601_rx(cp);
	if (status & (TX_OK | TX_ERR | TX_EMPTY | SW_INT))
		re8601_tx(cp);

	RTL_W32(ISR,status);


	spin_unlock(&cp->lock);
}

static void re8601_tx (struct re_private *cp)
{
	unsigned tx_head = cp->tx_hqhead;
	unsigned tx_tail = cp->tx_hqtail;

	while (tx_tail != tx_head) {
		struct sk_buff *skb;
		u32 status;
		rmb();
		status = (cp->tx_hqring[tx_tail].opts1);
		if (status & DescOwn)
			break;
		if (status & TxError)
		{
			cp->net_stats.tx_errors++;
			if (status & TxFIFOUnder)
				cp->net_stats.tx_fifo_errors++;
		}
		skb = cp->tx_skb[tx_tail].skb;
		cp->tx_hqring[tx_tail].addr =0xaa550000;
		cp->tx_hqring[tx_tail].opts1 =0x0;
		
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

	cp->tx_hqtail = tx_tail;

	if (netif_queue_stopped(cp->dev) && (TX_HQBUFFS_AVAIL(cp) > (MAX_SKB_FRAGS + 1)))
		netif_wake_queue(cp->dev);
}

static int re8601_start_xmit (struct sk_buff *skb, struct net_device *dev)
{
	struct re_private *cp = dev->priv;
	unsigned entry;
	u32 eor;
#if CP_VLAN_TAG_USED
	u32 vlan_tag = 0;
#endif

	spin_lock_irq(&cp->lock);

	/* This is a hard error, log it. */
	if (TX_HQBUFFS_AVAIL(cp) <= (skb_shinfo(skb)->nr_frags + 1)) {
		netif_stop_queue(dev);
		spin_unlock_irq(&cp->lock);
		printk(KERN_ERR PFX "%s: BUG! Tx Ring full when queue awake!\n",
		       dev->name);
		return 1;
	}

#if CP_VLAN_TAG_USED
	if (cp->vlgrp && vlan_tx_tag_present(skb))
		vlan_tag = TxVlanTag | vlan_tx_tag_get(skb);
#endif
	if ( (skb->len>1514) )
		for(;;)
		{
			printk("error tx len = %d \n",skb->len);
		}
		
		
	entry = cp->tx_hqhead;
	eor = (entry == (RE8601_TX_RING_SIZE - 1)) ? RingEnd : 0;
	if (skb_shinfo(skb)->nr_frags == 0) {
		DMA_DESC  *txd = &cp->tx_hqring[entry];
		u32 mapping, len;

		len = ETH_ZLEN < skb->len ? skb->len : ETH_ZLEN;
		eor = (entry == (RE8601_TX_RING_SIZE - 1)) ? RingEnd : 0;
		CP_VLAN_TX_TAG(txd, vlan_tag);
		mapping = (u32)skb->data|UNCACHE_MASK;
		txd->addr = (mapping);
		wmb();
		cp->tx_skb[entry].skb = skb;
		cp->tx_skb[entry].mapping = mapping;
		cp->tx_skb[entry].frag = 0;

		txd->opts1 = (eor | len | DescOwn | FirstFrag |
			LastFrag | TxCRC);
		wmb();

		entry = NEXT_TX(entry);
	} else {
		DMA_DESC *txd;
		u32 first_len, first_mapping;
		int frag, first_entry = entry;

		/* We must give this initial chunk to the device last.
		 * Otherwise we could race with the device.
		 */
		first_len = skb->len - skb->data_len;
		/*first_mapping = pci_map_single(cp->pdev, skb->data,*
					       first_len, PCI_DMA_TODEddVICE);*/
		first_mapping = (u32)skb->data|UNCACHE_MASK;
		cp->tx_skb[entry].skb = skb;
		cp->tx_skb[entry].mapping = first_mapping;
		cp->tx_skb[entry].frag = 1;
		entry = NEXT_TX(entry);

		for (frag = 0; frag < skb_shinfo(skb)->nr_frags; frag++) {
			skb_frag_t *this_frag = &skb_shinfo(skb)->frags[frag];
			u32 len, mapping;
			u32 ctrl;

			len = this_frag->size;
			/*mapping = pci_map_single(cp->pdev,
						 ((void *) page_address(this_frag->page) +
						  this_frag->page_offset),
						 len, PCI_DMA_TODEVICE);*/
			mapping = (u32)this_frag->page_offset|UNCACHE_MASK;

			eor = (entry == (RE8601_TX_RING_SIZE - 1)) ? RingEnd : 0;
			ctrl = eor | len | DescOwn | TxCRC;
			if (frag == skb_shinfo(skb)->nr_frags - 1)
				ctrl |= LastFrag;

			txd = &cp->tx_hqring[entry];
			CP_VLAN_TX_TAG(txd, vlan_tag);
			txd->addr = (mapping);
			wmb();

			txd->opts1 = (ctrl);
			wmb();
			cp->tx_skb[entry].skb = skb;
			cp->tx_skb[entry].mapping = mapping;
			cp->tx_skb[entry].frag = frag + 2;
			entry = NEXT_TX(entry);
		}

		txd = &cp->tx_hqring[first_entry];
		CP_VLAN_TX_TAG(txd, vlan_tag);
		
		txd->addr = (first_mapping);
		
		wmb();
		
		eor = (first_entry == (RE8601_TX_RING_SIZE - 1)) ? RingEnd : 0;
		txd->opts1 = (first_len | FirstFrag | DescOwn|TxCRC|eor);
		
		wmb();
	}
	cp->tx_hqhead = entry;
	if (netif_msg_tx_queued(cp))
		printk(KERN_DEBUG "%s: tx queued, slot %d, skblen %d\n",
		       dev->name, entry, skb->len);
	if (TX_HQBUFFS_AVAIL(cp) <= (MAX_SKB_FRAGS + 1))
		netif_stop_queue(dev);

	spin_unlock_irq(&cp->lock);
	
//	RTL_W32(IO_CMD,CMD_CONFIG | TX_POLL);
	RTL_W32(CNR1, TDFN|(RXBLEN<<RXBLEN_SH)|(TXBLEN<<TXBLEN_SH)|CNR_CMD);

	dev->trans_start = jiffies;

	return 0;
}

/* Set or clear the multicast filter for this adaptor.
   This routine is not state sensitive and need not be SMP locked. */

static void __re8601_set_rx_mode (struct net_device *dev)
{
	/*struct re_private *cp = dev->priv;*/
	u32 mc_filter[2];	/* Multicast hash filter */
	int i, rx_mode;
	/*u32 tmp;*/


	printk("%s: %s %d Still Unimplemented !!!!!!!\n",__FILE__,__FUNCTION__,__LINE__);
	return ;
	/* Note: do not reorder, GCC is clever about common statements. */
	if (dev->flags & IFF_PROMISC) {
		/* Unconditionally log net taps. */
		printk (KERN_NOTICE "%s: Promiscuous mode enabled.\n",
			dev->name);
		rx_mode =
		    AcceptBroadcast | AcceptMulticast | AcceptMyPhys |
		    AcceptAllPhys;
		mc_filter[1] = mc_filter[0] = 0xffffffff;
	} else if ((dev->mc_count > multicast_filter_limit)
		   || (dev->flags & IFF_ALLMULTI)) {
		/* Too many to filter perfectly -- accept all multicasts. */
		rx_mode = AcceptBroadcast | AcceptMulticast | AcceptMyPhys;
		mc_filter[1] = mc_filter[0] = 0xffffffff;
	} else {
		struct dev_mc_list *mclist;
		rx_mode = AcceptBroadcast | AcceptMyPhys;
		mc_filter[1] = mc_filter[0] = 0;
		for (i = 0, mclist = dev->mc_list; mclist && i < dev->mc_count;
		     i++, mclist = mclist->next) {
			int bit_nr = ether_crc(ETH_ALEN, mclist->dmi_addr) >> 26;

			mc_filter[bit_nr >> 5] |= 1 << (bit_nr & 31);
			rx_mode |= AcceptMulticast;
		}
	}

	/* We can safely update without stopping the chip. */
}

static void re8601_set_rx_mode (struct net_device *dev)
{
	unsigned long flags;
	struct re_private *cp = dev->priv;

	spin_lock_irqsave (&cp->lock, flags);
	__re8601_set_rx_mode(dev);
	spin_unlock_irqrestore (&cp->lock, flags);
}

static void __re8601_get_stats(struct re_private *cp)
{
	/* XXX implement */
}

static struct net_device_stats *re8601_get_stats(struct net_device *dev)
{
	struct re_private *cp = dev->priv;

	/* The chip only need report frame silently dropped. */
	spin_lock_irq(&cp->lock);
 	if (netif_running(dev) && netif_device_present(dev))
 		__re8601_get_stats(cp);
	spin_unlock_irq(&cp->lock);

	return &cp->net_stats;
}

static void re8601_stop_hw (struct re_private *cp)
{
//	RTL_W32(IO_CMD,0); /* timer  rx int 1 packets*/
	RTL_W32(IMR, 0);
	RTL_W32(ISR, 0xffff);
	synchronize_irq();
	udelay(10);

	cp->rx_tail = 0;
	cp->tx_hqhead = cp->tx_hqtail = 0;
}

static void re8601_reset_hw (struct re_private *cp)
{
	unsigned work = 1000;
	unsigned long rs = 1 << 20;

   	RTL_W32(CNR1,rs);	 /* Reset */	
	while (work--) {
		if (!(RTL_R32(CNR1) & rs))
			return;
		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(10);
	}
   	RTL_W32(CNR1,rs);	 /* Reset */	
	while (work--) {
		if (!(RTL_R32(CNR1) & rs))
			return;
		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(10);
	}


   	RTL_W32(CNR1,1 << 13);	 /* checksum */	

	printk(KERN_ERR "%s: hardware reset timeout\n", cp->dev->name);
}

static inline void re8601_start_hw (struct re_private *cp)
{
	
	RTL_W32(IO_CMD,CMD_CONFIG); /* timer  rx int 1 packets*/
#if 0
	cpw8(Cmd, RxOn | TxOn);
	cpw16(CpCmd, PCIMulRW | RxChkSum | CpRxOn | CpTxOn);
#endif
}

static void re8601_init_hw (struct re_private *cp)
{
	struct net_device *dev = cp->dev;
//	u8 status;

	re8601_reset_hw(cp);

	RTL_W32(IMTR, (MIT_TXTIMER<<16)|(MIT_TXNUM<<8)|(MIT_RXTIMER<<4)|(MIT_RXNUM));
	RTL_W32(ISR,0xffffffff);
	RTL_W32(IMR,re8601_intr_mask);   	
//	RTL_W32(RxFDP,(u32)(cp->rx_ring));	
#if DEBUG
	writecheck = RTL_R32(RxFDP);	
	if (writecheck != ( (u32)cp->rx_ring ))
		for (;;);

#endif	
//	RTL_W16(RxCDO,0);
//	RTL_W32(TxFDP1,(u32)(cp->tx_hqring));	
#if DEBUG
	writecheck = RTL_R32(TxFDP1);	
	if (writecheck != ( (u32)cp->tx_hqring))
		for (;;);
#endif
//	RTL_W16(TxCDO1,0);	
//	RTL_W32(TxFDP2,(u32)cp->tx_lqring);	
//	RTL_W16(TxCDO2,0);
	// packet type filter setting
	RTL_W32(TMF0, TP0|(TFMSK0<<TFMSK_SH));
	RTL_W32(TMF1, TP1|(TFMSK1<<TFMSK_SH));
	RTL_W32(TMF2, TP2|(TFMSK2<<TFMSK_SH));
	RTL_W32(TMF3, TP3|(TFMSK3<<TFMSK_SH));

	RTL_W32(MII, MMIMODE|DISNWAY|LINK_CTRL|DPLX_CTRL|SPD_CTRL|FL_CTRL);
	RTL_W32(CNR2, 0x00000000|(PTMASK<<PTMASK_SH)|(IFG<<IFG_SH));
	RTL_W32(TSAD, (u32)(cp->tx_hqring)&(0x1fffffff));
	RTL_W32(RSAD, (u32)(cp->rx_ring)&(0x1fffffff));
	RTL_W32(CNR1, (RXBLEN<<RXBLEN_SH)|(TXBLEN<<TXBLEN_SH)|CNR_CMD);
//	RTL_W8(Rx_Pse_Des_Thres,THVAL);
//	RTL_W8(EthrntRxCPU_Des_Num,RE8601_RX_RING_SIZE-1);
//	RTL_W8(RxRingSize,RINGSIZE);	
//	status = RTL_R8(MSR);
//	status = status & ~(TXFCE | RXFCE | FORCE_TX);
//	RTL_W8(MSR,status);
//	re8601_start_hw(cp);

	__re8601_set_rx_mode(dev);
#if 0
	cpw8_f (Cfg9346, Cfg9346_Unlock);

	/* Restore our idea of the MAC address. */
	cpw32_f (MAC0 + 0, cpu_to_le32 (*(u32 *) (dev->dev_addr + 0)));
	cpw32_f (MAC0 + 4, cpu_to_le32 (*(u32 *) (dev->dev_addr + 4)));

	cpw8(TxThresh, 0x06); /* XXX convert magic num to a constant */
	cpw32_f (TxConfig, IFG | (TX_DMA_BURST << TxDMAShift));

	cpw8(Config1, cpr8(Config1) | DriverLoaded | PMEnable);
	cpw8(Config3, PARMEnable); /* disables magic packet and WOL */
	cpw8(Config5, cpr8(Config5) & PMEStatus); /* disables more WOL stuff */

	cpw32_f(HiTxRingAddr, 0);
	cpw32_f(HiTxRingAddr + 4, 0);
	cpw32_f(OldRxBufAddr, 0);
	cpw32_f(OldTSD0, 0);
	cpw32_f(OldTSD0 + 4, 0);
	cpw32_f(OldTSD0 + 8, 0);
	cpw32_f(OldTSD0 + 12, 0);

	cpw32_f(RxRingAddr, cp->ring_dma);
	cpw32_f(RxRingAddr + 4, 0);
	cpw32_f(TxRingAddr, cp->ring_dma + (sizeof(struct cp_desc) * CP_RX_RING_SIZE));
	cpw32_f(TxRingAddr + 4, 0);

	cpw16(MultiIntr, 0);

	cpw16(IntrMask, cp_intr_mask);

	cpw8_f (Cfg9346, Cfg9346_Lock);
#endif
}

static int re8601_refill_rx (struct re_private *cp)
{
	unsigned i;

	for (i = 0; i < RE8601_RX_RING_SIZE; i++) {
		struct sk_buff *skb;

		skb = dev_alloc_skb(cp->rx_buf_sz + RX_OFFSET);
		if (!skb)
			goto err_out;

		skb->dev = cp->dev;
#if 0
		cp->rx_skb[i].mapping = pci_map_single(cp->pdev,
			skb->tail, cp->rx_buf_sz, PCI_DMA_FROMDEVICE);
#endif
		cp->rx_skb[i].skb = skb;
		cp->rx_skb[i].frag = 0;
		if ((u32)skb->data &0x3)
			printk(KERN_DEBUG "skb->data unaligment %8x\n",(u32)skb->data);
		
		cp->rx_ring[i].addr = (u32)skb->data|UNCACHE_MASK;
		if (i == (RE8601_RX_RING_SIZE - 1))
			cp->rx_ring[i].opts1 = (DescOwn | RingEnd | cp->rx_buf_sz);
		else
			cp->rx_ring[i].opts1 =(DescOwn | cp->rx_buf_sz);
		cp->rx_ring[i].opts2 = 0;
	}

	return 0;

err_out:
	re8601_clean_rings(cp);
	return -ENOMEM;
}

static void re8601_tx_timeout (struct net_device *dev)
{
	struct re_private *cp = dev->priv;
	unsigned tx_head = cp->tx_hqhead;
	unsigned tx_tail = cp->tx_hqtail;
	
	cp->cp_stats.tx_timeouts++;

	spin_lock_irq(&cp->lock);
	while (tx_tail != tx_head) {
		struct sk_buff *skb;
		u32 status;
		rmb();
		status = (cp->tx_hqring[tx_tail].opts1);
		if (status & DescOwn)
			break;
		skb = cp->tx_skb[tx_tail].skb;
		if (!skb)
			BUG();
		cp->net_stats.tx_packets++;
		cp->net_stats.tx_bytes += skb->len;
		dev_kfree_skb(skb);
		cp->tx_skb[tx_tail].skb = NULL;
		tx_tail = NEXT_TX(tx_tail);
	}

	cp->tx_hqtail = tx_tail;

	spin_unlock_irq(&cp->lock);
	if (netif_queue_stopped(cp->dev))
		netif_wake_queue(cp->dev);
	
}
static int re8601_init_rings (struct re_private *cp)
{
	memset(cp->tx_hqring, 0, sizeof(DMA_DESC) * RE8601_TX_RING_SIZE);
	memset(cp->rx_ring, 0, sizeof(DMA_DESC) * RE8601_RX_RING_SIZE);
	cp->rx_tail = 0;
	cp->tx_hqhead = cp->tx_hqtail = 0;

	return re8601_refill_rx (cp);
}

static int re8601_alloc_rings (struct re_private *cp)
{
	/*cp->rx_ring = pci_alloc_consistent(cp->pdev, CP_RING_BYTES, &cp->ring_dma);*/
	void*	pBuf;
	
	
	
	pBuf = kmalloc(RE8601_RXRING_BYTES,GFP_KERNEL);
	if (!pBuf)
		goto ErrMem;
	cp->rxdesc_buf = pBuf;
	memset(pBuf, 0, RE8601_RXRING_BYTES);
	
	pBuf = (void*)( (u32)(pBuf + DESC_ALIGN-1) &  ~(DESC_ALIGN -1) ) ;
	cp->rx_ring = (DMA_DESC*)((u32)(pBuf) | UNCACHE_MASK);


	pBuf= kmalloc(RE8601_TXRING_BYTES, GFP_KERNEL);
	if (!pBuf)
		goto ErrMem;
	cp->txdesc_buf = pBuf;
	memset(pBuf, 0, RE8601_TXRING_BYTES);
	pBuf = (void*)( (u32)(pBuf + DESC_ALIGN-1) &  ~(DESC_ALIGN -1) ) ;
	cp->tx_hqring = (DMA_DESC*)((u32)(pBuf) | UNCACHE_MASK);

	return re8601_init_rings(cp);

ErrMem:
	if (cp->rxdesc_buf)
		kfree(cp->rxdesc_buf);
	if (cp->txdesc_buf)
		kfree(cp->txdesc_buf);
	return -ENOMEM;

	
}

static void re8601_clean_rings (struct re_private *cp)
{
	unsigned i;


	for (i = 0; i < RE8601_RX_RING_SIZE; i++) {
		if (cp->rx_skb[i].skb) {
			dev_kfree_skb(cp->rx_skb[i].skb);
		}
	}

	for (i = 0; i < RE8601_TX_RING_SIZE; i++) {
		if (cp->tx_skb[i].skb) {
			struct sk_buff *skb = cp->tx_skb[i].skb;
			dev_kfree_skb(skb);
			cp->net_stats.tx_dropped++;
		}
	}

	memset(&cp->rx_skb, 0, sizeof(struct ring_info) * RE8601_RX_RING_SIZE);
	memset(&cp->tx_skb, 0, sizeof(struct ring_info) * RE8601_TX_RING_SIZE);
}

static void re8601_free_rings (struct re_private *cp)
{
	re8601_clean_rings(cp);
	/*pci_free_consistent(cp->pdev, CP_RING_BYTES, cp->rx_ring, cp->ring_dma);*/
	kfree(cp->rxdesc_buf);
	kfree(cp->txdesc_buf);
	
	cp->rx_ring = NULL;
	cp->tx_hqring = NULL;
}

static int re8601_open (struct net_device *dev)
{
	struct re_private *cp = dev->priv;
	int rc;

	if (netif_msg_ifup(cp))
		printk(KERN_DEBUG "%s: enabling interface\n", dev->name);

	re8601_set_rxbufsize(cp);	/* set new rx buf size */
	rc = re8601_alloc_rings(cp);
	if (rc)
		return rc;

	re8601_init_hw(cp);

//	rc = request_irq(dev->irq, re8601_interrupt, 0, dev->name, dev);
	rc = request_irq(dev->irq, re8601_interrupt, SA_INTERRUPT, dev->name, dev);
	if (rc)
		goto err_out_hw;

	netif_start_queue(dev);

	return 0;

err_out_hw:
	re8601_stop_hw(cp);
	re8601_free_rings(cp);
	return rc;
}

static int re8601_close (struct net_device *dev)
{
	struct re_private *cp = dev->priv;

	if (netif_msg_ifdown(cp))
		printk(KERN_DEBUG "%s: disabling interface\n", dev->name);

	netif_stop_queue(dev);
	re8601_stop_hw(cp);
	free_irq(dev->irq, dev);
	re8601_free_rings(cp);
	return 0;
}

static int re8601_change_mtu(struct net_device *dev, int new_mtu)
{
	struct re_private *cp = dev->priv;
	int rc;

	/* check for invalid MTU, according to hardware limits */
	if (new_mtu < CP_MIN_MTU || new_mtu > CP_MAX_MTU)
		return -EINVAL;

	/* if network interface not up, no need for complexity */
	if (!netif_running(dev)) {
		dev->mtu = new_mtu;
		re8601_set_rxbufsize(cp);	/* set new rx buf size */
		return 0;
	}

	spin_lock_irq(&cp->lock);

	re8601_stop_hw(cp);			/* stop h/w and free rings */
	re8601_clean_rings(cp);

	dev->mtu = new_mtu;
	re8601_set_rxbufsize(cp);		/* set new rx buf size */

	rc = re8601_init_rings(cp);		/* realloc and restart h/w */
	re8601_start_hw(cp);
	spin_unlock_irq(&cp->lock);

	return rc;
}
#if 0
static char mii_2_8139_map[8] = {
	BasicModeCtrl,
	BasicModeStatus,
	0,
	0,
	NWayAdvert,
	NWayLPAR,
	NWayExpansion,
	0
};
static int mdio_read(struct net_device *dev, int phy_id, int location)
{
	struct re_private *cp = dev->priv;

	return location < 8 && mii_2_8139_map[location] ?
	       readw(cp->regs + mii_2_8139_map[location]) : 0;
}


static void mdio_write(struct net_device *dev, int phy_id, int location,
		       int value)
{
	struct re_private *cp = dev->priv;

	if (location == 0) {
		cpw8(Cfg9346, Cfg9346_Unlock);
		cpw16(BasicModeCtrl, value);
		cpw8(Cfg9346, Cfg9346_Lock);
	} else if (location < 8 && mii_2_8139_map[location])
		cpw16(mii_2_8139_map[location], value);
}

#endif
static int re8601_ethtool_ioctl (struct re_private *cp, void *useraddr)
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
		mii_ethtool_gset(&cp->mii_if, &ecmd);
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
		r = mii_ethtool_sset(&cp->mii_if, &ecmd);
		spin_unlock_irq(&cp->lock);
		return r;
	}
	/* restart autonegotiation */
	case ETHTOOL_NWAY_RST: {
		return mii_nway_restart(&cp->mii_if);
	}
	/* get link status */
	case ETHTOOL_GLINK: {
		struct ethtool_value edata = {ETHTOOL_GLINK};
		edata.data = mii_link_ok(&cp->mii_if);
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


static int re8601_ioctl (struct net_device *dev, struct ifreq *rq, int cmd)
{
	struct re_private *cp = dev->priv;
	int rc = 0;

	if (!netif_running(dev))
		return -EINVAL;

	switch (cmd) {
	case SIOCETHTOOL:
		return re8601_ethtool_ioctl(cp, (void *) rq->ifr_data);

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

static int __devinit read_eeprom (void *ioaddr, int location, int addr_len)
{

	return 0;
}


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


int __init re8601_probe (void)
{
#ifdef MODULE
	printk("%s", version);
#endif

	struct net_device *dev;
	struct re_private *cp;
	int rc;
	void *regs=(void*)0xbd012000;
	unsigned addr_len, i;
	unsigned char eth_addr[6];

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
	reDev = (struct re_private*)dev->priv;
	cp->dev = dev;
	spin_lock_init (&cp->lock);
	
#if 0
	cp->mii_if.dev = dev;
	cp->mii_if.mdio_read = mdio_read;
	cp->mii_if.mdio_write = mdio_write;
	cp->mii_if.phy_id = CP_INTERNAL_PHY;
#endif
	
	dev->base_addr = (unsigned long) regs;
	cp->regs = regs;

	re8601_stop_hw(cp);

	/* read MAC address from EEPROM */
	for (i = 0; i < 6; i++)
	{
		eth_addr[i] = i * 2;
	}
	
	eth_set_mac_address(dev, eth_addr);
	
	/*
	for (i = 0; i < 3; i++)
		((u16 *) (dev->dev_addr))[i] = i * 2;
	*/

	dev->open = re8601_open;
	dev->stop = re8601_close;
	dev->set_multicast_list = re8601_set_rx_mode;
	dev->hard_start_xmit = re8601_start_xmit;
	dev->get_stats = re8601_get_stats;
	dev->do_ioctl = re8601_ioctl;
	/*dev->change_mtu = re8601_change_mtu;*/
#if 1 
	dev->tx_timeout = re8601_tx_timeout;
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

	dev->irq = 4;

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

static void __exit re8601_exit (void)
{
}

module_init(re8601_probe);
module_exit(re8601_exit);
