/* re8670.c: A Linux Ethernet driver for the RealTek 8670 chips. */
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
#include <asm/mach-realtek/rtl8672/platform.h>
#include "../../../user/boa/src/LINUX/options.h"
#ifdef CONFIG_EXT_SWITCH
#include "./rtl8306/Rtl8306_types.h"
#include "./rtl8306/Rtl8306_AsicDrv.h"
#ifdef CONFIG_RE8306_API
#include "./rtl8306/Rtl8306_Driver_s.h"
#include "./rtl8306/Rtl8306_Driver_sd.h"
#endif // of CONFIG_RE8306_API
#endif // of CONFIG_EXT_SWITCH
#include "re830x.h"

//jim luo 2007 06 18 fast bridge
#define ZTE_FAST_BRIDGE
#ifdef ZTE_FAST_BRIDGE
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_NET
__NOMIPS16
#endif
static void local_dev_kfree_skb(struct sk_buff *skb)
{
	dev_kfree_skb(skb);
}
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_NET
__NOMIPS16
#endif
static void local_dev_kfree_skb_irq(struct sk_buff *skb)
{
	dev_kfree_skb_irq(skb);
}
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_NET
__NOMIPS16
#endif
static void local_netif_wake_queue(struct net_device *dev)
{
	netif_wake_queue(dev);
}
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_NET
__NOMIPS16
#endif
static void local_netif_stop_queue(struct net_device *dev)
{
	netif_stop_queue(dev);
}
#endif
//#define WLANBR_SHORTCUT

#ifdef CONFIG_RTL8671
#include "../../arch/mips/realtek/rtl8670/lx5280.h"
#include "../../arch/mips/realtek/rtl8670/gpio.h"
#endif

// Added by Mason Yu for PPP ACT LED
int g_ppp_up = 0;
int g_internet_up = 0;
#ifdef CONFIG_EXT_SWITCH
void miiar_ReadBit(unsigned char phyad , unsigned char regad ,unsigned char bit,unsigned short *value);
void miiar_WriteBit(unsigned char phyad, unsigned char regad, unsigned char bit, unsigned char value);
void rtl8305sc_setVLANTagInsertRemove(char port, char option);
int rtl8305sc_setPower(char port,char enabled);
void rtl8305sc_setVLANTag1pProirity(char port, char option);
int rtl8305sc_setAsicPortVlanIndex(unsigned short port, unsigned short vlanIndex);
void IGMP_Snooping_config(void);
int rtl8305sc_setAsicVlanEnable(char enabled);
int rtl8305sc_setAsicVlanTagAware(char enabled);
extern void vc_ch_remapping(unsigned *member);
#endif
// kaohj -- bitmap of virtual ports vs bitmap of phy id
// Input:  bitmap of virtual ports
// Output: bitmap of phy id
int bitmap_virt2phy(int mbr);
void internal_miiar_write(unsigned char phyaddr,unsigned char regaddr,unsigned short value);
void miiar_write(unsigned char phyaddr,unsigned char regaddr,unsigned short value);
void miiar_read(unsigned char phyaddr,unsigned char regaddr,unsigned short *value);
static int re8670_start_xmit (struct sk_buff *skb, struct net_device *dev);

/* Jonah + for FASTROUTE */
struct net_device *eth_net_dev;
struct tasklet_struct *eth_rx_tasklets=NULL;
//extern struct net_device *sar_dev;
//#undef 	FAST_ROUTE_Test
#define 	FAST_ROUTE_Test
#ifdef FAST_ROUTE_Test
int fast_route_mode=1;
#endif
//#define	NIC_RX_BH	21
//#define	NIC_TX_BH	22
//extern  int sar_txx (struct sk_buff *skb);



#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
#define CP_VLAN_TAG_USED 1
#define CP_VLAN_TX_TAG(tx_desc,vlan_tag_value) \
	do { (tx_desc)->opts2 = (vlan_tag_value); } while (0)

#define cpw16(reg,val)  writew((val), (u32)cp->regs + (reg))
#define cpw16_f(reg,val) do {                   \
        writew((val), (u32)cp->regs + (reg));   \
        readw(cp->regs + (reg));                \
        } while (0)
#else
#define CP_VLAN_TAG_USED 0
#define CP_VLAN_TX_TAG(tx_desc,vlan_tag_value) \
	do { (tx_desc)->opts2 = 0; } while (0)
#endif

// Kaohj
int enable_IGMP_SNP=0;
int enable_ipqos=0;
//ql 20081120
int enable_imq=0;
int enable_port_mapping=0;
// Kaohj -- vlan-grouping
int enable_vlan_grouping=0;
//alex_huang 
int  wlan_igmp_tag = 0x1f; 
//alex_huang add for igmp forbidden
#ifdef CONFIG_IGMP_FORBID
#define   IGMP_FORBID              0x0a
int enable_IGMP_FORBID = 0;
 #endif
 
struct r8305_struc	rtl8305_info;
#ifdef CONFIG_EXT_SWITCH
int DropUnknownMulticast=1; //when IGMP snooping is enabled, 0:allow all multicast traffic  1:drop unknown multicast traffic
#define GPIO_SIMULATE
#ifdef GPIO_SIMULATE
void rtl8305s_smiRead(unsigned char phyad, unsigned char regad, unsigned short * data);
void rtl8305s_smiWrite(unsigned char phyad, unsigned char regad, unsigned short data);
#endif

int IGMP_SNP_registered=0;
int enable_virtual_port=0;

void select_page(unsigned int npage);
// Kaohj
#ifdef CONFIG_RE8306_API
void dump_8306_qos();
void reset_8306_counter();
void dump_8306_counter();
#endif // of CONFIG_RE8306_API
// Kaohj --- dump VLAN info (rtl8305_info)
void dump_vlan_info();
void set_8305(struct r8305_struc *info);


//cathy
#ifndef CONFIG_RTL8672
#define PABCDIR		(GPIO_MDC<=7?0x010:0x000)
#define PABCDAT		(GPIO_MDC<=7?0x014:0x004)
#else
#define PABCDIR		(GPIO_PABCD_DIR-GPIOCR_BASE)//jiunming, for 8672, (GPIO_MDC<=7?0x010:0x000)
#define PABCDAT		(GPIO_PABCD_DAT-GPIOCR_BASE)//jiunming, for 8672, (GPIO_MDC<=7?0x014:0x004)
#endif
// Kaohj
// virtual LAN port 0, 1, 2, 3 and 867x-nic to 8305(6) phy id
// ex, virt2phy[2]=3 ==> virtual port 2 is mapped to phy id 3
//     virt2phy[4]=0 ==> virtual port 867x-nic is mappped to phy id 0
#ifdef CONFIG_USBCLIENT
#define SWITCH_VPORT_TO_867X	5	// virtual LAN port against 867x-nic
#else
#define SWITCH_VPORT_TO_867X	4	// virtual LAN port against 867x-nic
#endif

#if 1
#ifdef CONFIG_RE8305
int virt2phy[SW_PORT_NUM+1]= {0, 1, 2, 3, 4};
#endif
#ifdef CONFIG_RE8306
//jim according to Kevin's advise, default cpu port is 4....
#ifdef CONFIG_USBCLIENT
int virt2phy[SW_PORT_NUM+1]= {0, 1, 2, 3, 4,6};
#else
int virt2phy[SW_PORT_NUM+1]= {0, 1, 2, 3, 4};
#endif
#endif
#else
int virt2phy[SW_PORT_NUM+1]= {1, 2, 3, 4, 0};
#endif
#endif  // CONFIG_EXT_SWITCH
/* These identify the driver base version and may not be removed. */
static char version[] __devinitdata =
KERN_INFO DRV_NAME " Ethernet driver v" DRV_VERSION " (" DRV_RELDATE ")\n";

MODULE_AUTHOR("Rupert Chang <rupert@mail.realtek.com.tw>");
MODULE_DESCRIPTION("RealTek RTL-8670 series 10/100 Ethernet driver");
MODULE_LICENSE("GPL");

#if 0
static int debug = -1;
#endif
MODULE_PARM (debug, "i");
MODULE_PARM_DESC (debug, "re8670 bitmapped message enable number");

/* Maximum number of multicast addresses to filter (vs. Rx-all-multicast).
   The RTL chips use a 64 element hash table based on the Ethernet CRC.  */
static int multicast_filter_limit = 32;
MODULE_PARM (multicast_filter_limit, "i");
MODULE_PARM_DESC (multicast_filter_limit, "8139cp maximum number of filtered multicast addresses");

#if defined(CONFIG_USB_RTL8192SU_SOFTAP) && !defined(CONFIG_RTL867X_PACKET_PROCESSOR)
#ifdef CONFIG_SKB_POOL_PREALLOC
#define RE8670_RX_RING_SIZE	256
#define RE8670_TX_RING_SIZE	256
#else
#define RE8670_RX_RING_SIZE	64//16~256 descriptors
#define RE8670_TX_RING_SIZE	128
#endif
#elif defined(CONFIG_RTL8192CD)
#define RE8670_RX_RING_SIZE	512
#define RE8670_TX_RING_SIZE	1024
#else
#define RE8670_RX_RING_SIZE	64//16~256 descriptors
#define RE8670_TX_RING_SIZE	64
#endif

#define PFX			DRV_NAME ": "
#define CP_DEF_MSG_ENABLE	(NETIF_MSG_DRV	|  NETIF_MSG_PROBE |  NETIF_MSG_LINK)
#define CP_REGS_SIZE		(0xff + 1)
#define DESC_ALIGN		0x100
#define UNCACHE_MASK		0xa0000000

#define RE8670_RXRING_BYTES	( (sizeof(struct dma_desc) * (RE8670_RX_RING_SIZE+1)) + DESC_ALIGN)

#define RE8670_TXRING_BYTES	( (sizeof(struct dma_desc) * (RE8670_TX_RING_SIZE+1)) + DESC_ALIGN)
		


#define NEXT_TX(N)		(((N) + 1) & (RE8670_TX_RING_SIZE - 1))
#define NEXT_RX(N)		(((N) + 1) & (RE8670_RX_RING_SIZE - 1))
#if 0
#define TX_HQBUFFS_AVAIL(CP)					\
	(((CP)->tx_hqtail <= (CP)->tx_hqhead) ?			\
	  (CP)->tx_hqtail + (RE8670_TX_RING_SIZE - 1) - (CP)->tx_hqhead :	\
	  (CP)->tx_hqtail - (CP)->tx_hqhead - 1)
#else
#define TX_HQBUFFS_AVAIL(CP)						\
	(((CP)->tx_hqtail - (CP)->tx_hqhead -1 + RE8670_TX_RING_SIZE) & (RE8670_TX_RING_SIZE -1))
#endif

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
#ifdef CONFIG_RTL8672
#define ETHBASE	0xB8018000 
#else
#define ETHBASE	0xB9800000
#endif
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
	CMD=0x3B,
	IMR=0x3C,
	ISR=0x3E,
	TCR=0x40,
	RCR=0x44,
	MSR=0x58,
	MIIAR=0x5C,
#if defined (CONFIG_VLAN_8021Q) || defined (CONFIG_VLAN_8021Q_MODULE)
        CpCmd=0xE0,    /* C+ Command register (C+ mode only) */
#endif        
#ifdef CONFIG_RTL8672 //cathy
	LEDCR=0x70,
#endif
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
	TX_FNL = (1<<1),
	TX_FNH = (1),
	/*TX_START= MII_RE | MII_TE | TX_FNL | TX_FNH,
	TX_START = 0x8113c,*/
	RXMII = MII_RE,
	TXMII= MII_TE,


	


};

enum RE8670_STATUS_REGS
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

enum RE8670_THRESHOLD_REGS{
#ifdef CONFIG_RTL8672
	TH_ON_VAL = 0x4,	//shlee flow control assert threshold: available desc <= 6
	TH_OFF_VAL= 0x8,	//shlee flow control de-assert threshold : available desc>=48
#else
	THVAL		= 2,
	RINGSIZE	= 2,
#endif
	LOOPBACK	= (0x3 << 8),
 	AcceptErr	= 0x20,	     /* Accept packets with CRC errors */
	AcceptRunt	= 0x10,	     /* Accept runt (<64 bytes) packets */
	AcceptBroadcast	= 0x08,	     /* Accept broadcast packets */
	AcceptMulticast	= 0x04,	     /* Accept multicast packets */
	AcceptMyPhys	= 0x02,	     /* Accept pkts with our MAC as dest */
	AcceptAllPhys	= 0x01,	     /* Accept all pkts w/ physical dest */
	AcceptAll = AcceptBroadcast | AcceptMulticast | AcceptMyPhys |  AcceptAllPhys | AcceptErr | AcceptRunt,
	AcceptNoBroad = AcceptMulticast |AcceptMyPhys |  AcceptAllPhys | AcceptErr | AcceptRunt,
	AcceptNoMulti =  AcceptMyPhys |  AcceptAllPhys | AcceptErr | AcceptRunt,
	NoErrAccept = AcceptBroadcast | AcceptMulticast | AcceptMyPhys,
	NoErrPromiscAccept = AcceptBroadcast | AcceptMulticast | AcceptMyPhys |  AcceptAllPhys,
	
};




enum RE8670_ISR_REGS{

	SW_INT 		= (1 <<10),
	TX_EMPTY	= (1 << 9),
	LINK_CHG	= (1 <<	8),
	TX_ERR		= (1 << 7),
	TX_OK		= (1 << 6),
	RX_EMPTY	= (1 << 5),
	RX_FIFOOVR	=(1 << 4),
	RX_ERR		=(1 << 2),
	RUNT_ERR 	=(1<<19),
	RX_OK		= (1 << 0),


};

enum RE8670_IOCMD_REG
{
	RX_MIT 		= 7,
	RX_TIMER 	= 1,
	RX_FIFO 	= 2,
	TX_FIFO		= 1,
	TX_MIT		= 7,	
	TX_POLL		= 1 << 0,
#ifdef CONFIG_RTL8672
	CMD_CONFIG = 0x40091030,
#else
	CMD_CONFIG = 0x3c | RX_MIT << 8 | RX_FIFO << 11 |  RX_TIMER << 13 | TX_MIT << 16 | TX_FIFO<<19,
#endif
};

typedef enum
{
	FLAG_WRITE		= (1<<31),
	FLAG_READ		= (0<<31),
	
	MII_PHY_ADDR_SHIFT	= 26, 
	MII_REG_ADDR_SHIFT	= 16,
	MII_DATA_SHIFT		= 0,
}MIIAR_MASK;

static const u32 re8670_intr_mask =
	RX_OK | RX_ERR | RX_EMPTY | RX_FIFOOVR |
	TX_EMPTY;
	//TX_OK | TX_ERR | TX_EMPTY;


typedef struct dma_desc {
	u32		opts1;
	u32		addr;
	u32		opts2;
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
	// JONAH
	unsigned long tx_cnt;
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
	struct ring_info	tx_skb[RE8670_TX_RING_SIZE];
	struct ring_info	rx_skb[RE8670_RX_RING_SIZE];
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
#if defined (CONFIG_VLAN_8021Q) || defined (CONFIG_VLAN_8021Q_MODULE)
	u16                     cpcmd;
#endif
	struct sk_buff		*frag_skb;
	unsigned		dropping_frag : 1;
	char*			rxdesc_buf;
	char*			txdesc_buf;
	struct mii_if_info	mii_if;
	//struct tq_struct	rx_task;
	//struct tq_struct	tx_task;
	struct tasklet_struct rx_tasklets;
	//struct tasklet_struct tx_tasklets;
};
//struct re_private *reDev;

// Kaohj
#ifdef CONFIG_EXT_SWITCH
struct net_device	*dev_sw[SW_PORT_NUM];
#endif

//for alignment issue
#define READWD(addr) ((unsigned char *)addr)[0]<<24 | ((unsigned char *)addr)[1]<<16 | ((unsigned char *)addr)[2]<<8 | ((unsigned char *)addr)[3]
#define READHWD(addr) ((unsigned char *)addr)[0]<<8 | ((unsigned char *)addr)[1]
#define VTAG2DESC(d) ( (((d) & 0x00ff)<<8) | (((d) & 0x0f00)>>8) )
//tylo, struct for IGMP snooping
#ifdef CONFIG_EXT_SWITCH
#include <net/checksum.h>
#ifdef CONFIG_USBCLIENT
#define MAX_IGMP_SNOOPING_GROUP	5
#else
#define MAX_IGMP_SNOOPING_GROUP	7
#endif
int REST_IGMP_SNOOPING_GROUP=MAX_IGMP_SNOOPING_GROUP;
int current_snooping_group=0;
struct IGMP_SNOOPING{
	//unsigned char VLAN_index;
	unsigned int multicast_addr;
	unsigned char member_port;
	unsigned char member_wlan;
	unsigned short VLANID;
	unsigned char tmp_member_port;
	unsigned char tmp_member_wlan;
	struct timer_list	snp_timer;
	unsigned char timer_running;
}snooping_table[MAX_IGMP_SNOOPING_GROUP];
#endif

static void __re8670_set_rx_mode (struct net_device *dev);
static void re8670_tx (struct re_private *cp);
static void re8670_clean_rings (struct re_private *cp);
static void re8670_tx_timeout (struct net_device *dev);

extern void enable_lx4180_irq(int irq);
extern void disable_lx4180_irq(int irq);


int re8670_rxskb_num=0;

#define RE8670_MAX_ALLOC_RXSKB_NUM 150
#define SKB_BUF_SIZE  1600

//for wifi-test
unsigned int iocmd_reg=CMD_CONFIG;

struct sk_buff *re8670_getAlloc(void);
struct sk_buff *re8670_getAlloc()
{	
struct sk_buff *skb=NULL;
	
//printk("skb=%x cb[0]=%d\n",(unsigned int)skb , skb->cb[0] );
    if ( re8670_rxskb_num < RE8670_MAX_ALLOC_RXSKB_NUM ) {
	//printk("free pre-alloc skb - unknow state...\n");
	skb = dev_alloc_skb(SKB_BUF_SIZE);
        if (skb!=NULL) {
            re8670_rxskb_num++;
            skb->src_port=IF_ETH;
            //printk("GetE %d\n", re8670_rxskb_num);
        };
    }
    return skb;
}

static inline void re8670_set_rxbufsize (struct re_private *cp)
{
	unsigned int mtu = cp->dev->mtu;
	
	if (mtu > ETH_DATA_LEN)
		/* MTU + ethernet header + FCS + optional VLAN tag */
		cp->rx_buf_sz = mtu + ETH_HLEN + 8;
	else
		cp->rx_buf_sz = PKT_BUF_SZ;
}

#if 0
/* flag:
 *	0 : Input
 *	1 : Output
 */
static void skb_debug(const struct sk_buff *skb, int enable, int flag)
{
	if (enable) {
		int i;
		if (!flag)
			printk("\nI: ");
		else
			printk("\nO: ");
		printk("eth len = %d", skb->len);
		for (i=0; i<skb->len && i<60; i++) {
			if (i%16==0)
				printk("\n");
			printk("%2.2x ", skb->data[i]);
		}
		printk("\n");
	}
}
#else
#define skb_debug(skb, enable, flag)	do {} while (0)
#endif

int rtl8305sc_setAsicVlan(unsigned short vlanIndex, unsigned short vid, unsigned short memberPortMask);
unsigned int handle_leave = 0, leave_query_count = 0;
unsigned char leave_port = 0;
unsigned int leave_multicast_addr;
int handle_IGMP_leave(int grp, struct sk_buff *skb);
void check_IGMP_snoop_rx(struct sk_buff *skb, int tag)
{
#ifdef CONFIG_EXT_SWITCH
int i, in_snp_table=0;	
unsigned char igmp_offset=0;	
unsigned char port;
unsigned int multicast_addr;
int isIgmpV3report = 0;
int isIgmpV3leave =0;

//tylo, for IGMP snooping
#define IP_TYPE_OFFSET				0x0c
#define IP_TYPE						0x0800
#define IP_HDLEN_OFFSET			0x0e
#define PROTOCOL_OFFSET			0x09
#define IGMP_PROTOCOL				0x02
#define IGMP_TYPE_REPORT_JOIN		0x16
#define IGMPV3_TYPE_REPORT_JOIN	0x22
#define IGMP_TYPE_REPORT_LEAVE	0x17
#define MULTICAST_ADDR_REPORT_OFFSET	0x04
#define MULTICAST_ADDR_REPORT_OFFSET_V3 0x0c
#define IGMPV3_GROUPTAG_OFFSET          0x08
#define IGMPV3_IS_INCLUDE               0x03
#define IGMPV3_IS_EXCLUDE               0x04  

	//check if incoming frame is multicast packet
	if((skb->pkt_type  != PACKET_MULTICAST)){
		return;
	}
	
	//check if incoming frame is IGMP packet
	if(((*(unsigned char *)(skb->data+PROTOCOL_OFFSET))==IGMP_PROTOCOL)){
		
		//retrieve the input port
#define TYPE_OFFSET_802_1Q		12
#define TYPE_802_1Q				0x8100
#define VID_OFFSET_802_1Q		14
//		port=tag;//(*(unsigned short *)(skb->data+VID_OFFSET_802_1Q)&0x0fff);
		//port = virt2phy[tag];	// Jenny
		if(tag == wlan_igmp_tag){
	          port =5;
	        }else{
		    for(i=0;i<SW_PORT_NUM;i++)
			if (tag == rtl8305_info.vlan[rtl8305_info.phy[virt2phy[i]].vlanIndex].vid)
				break;
		     port = virt2phy[i];
	      	 } 

		igmp_offset=((*(unsigned char *)(skb->data))&0x0f)*4;
		if((*(unsigned char *)(skb->data+igmp_offset))==IGMPV3_TYPE_REPORT_JOIN){
		    if(*(unsigned char *)(skb->data+igmp_offset+IGMPV3_GROUPTAG_OFFSET)==IGMPV3_IS_INCLUDE)
		    {
		       if((*(unsigned char *)(skb->data+igmp_offset+IGMPV3_GROUPTAG_OFFSET+1)== 0)&&(*(unsigned short *)(skb->data+igmp_offset+IGMPV3_GROUPTAG_OFFSET+2)== 0))
			       isIgmpV3leave=1;
		    }
		    //else if(*(unsigned char *)(skb->data+igmp_offset+IGMPV3_GROUPTAG_OFFSET)==IGMPV3_IS_EXCLUDE){
		    else if(*(unsigned char *)(skb->data+igmp_offset+IGMPV3_GROUPTAG_OFFSET)==IGMPV3_IS_EXCLUDE ||
		    		*(unsigned char *)(skb->data+igmp_offset+IGMPV3_GROUPTAG_OFFSET)==0x02){
			    if((*(unsigned char *)(skb->data+igmp_offset+IGMPV3_GROUPTAG_OFFSET+1)== 0)&&(*(unsigned short *)(skb->data+igmp_offset+IGMPV3_GROUPTAG_OFFSET+2)== 0))
			       isIgmpV3report=1;
		    }
			    
			multicast_addr=READWD(skb->data+igmp_offset+MULTICAST_ADDR_REPORT_OFFSET_V3);
		}
		else 
		    multicast_addr=READWD(skb->data+igmp_offset+MULTICAST_ADDR_REPORT_OFFSET);

		//join group
		if(((*(unsigned char *)(skb->data+igmp_offset))==IGMP_TYPE_REPORT_JOIN || isIgmpV3report )&&  REST_IGMP_SNOOPING_GROUP!=0){
			printk("get IGMP membership report  from port:%x  ",port);
			printk("multicast addr: %x\n",multicast_addr);
			in_snp_table=0;
			for(i=0;i<MAX_IGMP_SNOOPING_GROUP;i++){
				if(multicast_addr==snooping_table[i].multicast_addr){
					in_snp_table=1;
					break;
				}
			}
			if(in_snp_table){ //update snp table
				if(port!=5){ //port 5 for wlan
					snooping_table[i].tmp_member_port |= (1<<port);
					if((snooping_table[i].member_port & (1<<port))==0){
						snooping_table[i].member_port |= (1<<port);
						snooping_table[i].tmp_member_port |= 1<<port;
#ifdef CONFIG_USBCLIENT
                                        rtl8305sc_setAsicVlan(i+11,i+0x100,snooping_table[i].member_port);
#else
					rtl8305sc_setAsicVlan(i+9,i+0x100,snooping_table[i].member_port);

#endif
					}
				}
				else{
					snooping_table[i].tmp_member_wlan=1;
					snooping_table[i].member_wlan=1;
				}			
			}
			else{//find empty entry in IGMP_snooping table and create a vlan
				if(REST_IGMP_SNOOPING_GROUP>0){
					printk("create VLAN:"); 
					for(i=0;i<MAX_IGMP_SNOOPING_GROUP;i++){
						if(snooping_table[i].multicast_addr==0xc0000000)
							break;
					}
					REST_IGMP_SNOOPING_GROUP--;
					snooping_table[i].multicast_addr=multicast_addr;
					if(port!=5){
						snooping_table[i].member_port |= 1<<port;
						snooping_table[i].tmp_member_port |= 1<<port;
					}
					else{
						snooping_table[i].tmp_member_wlan=1;
						snooping_table[i].member_wlan=1;
					}
				//	printk("vlanindex:%x   vid:%x   member_port:%x       port:%x\n\n",i+11,i+0x101,snooping_table[i].member_port,port);
#ifdef CONFIG_USBCLIENT
                                   rtl8305sc_setAsicVlan(i+11,i+0x100,snooping_table[i].member_port);
#else
				   rtl8305sc_setAsicVlan(i+9,i+0x100,snooping_table[i].member_port);
#endif
				}
			}
			
		}
		//leave group
		if(((*(unsigned char *)(skb->data+igmp_offset))==IGMP_TYPE_REPORT_LEAVE)||isIgmpV3leave){
			printk("get IGMP membership leave  ");
			printk("multicast addr: %x\n",multicast_addr);
			for(i=0;i<MAX_IGMP_SNOOPING_GROUP;i++){
				if(multicast_addr==snooping_table[i].multicast_addr){
					handle_leave = 1;
					leave_port |= (1<<port);
					leave_multicast_addr = multicast_addr;
					handle_IGMP_leave(i, skb);
#if 0
					if(port!=5)
						snooping_table[i].member_port &= ~(1<<port);
					else
						snooping_table[i].member_wlan=0;
					if(snooping_table[i].member_port==0 && snooping_table[i].member_wlan==0){
						printk("remove this IGMP group:%x\n\n",multicast_addr);
						snooping_table[i].multicast_addr=0xc0000000;
						snooping_table[i].member_port =0;
						REST_IGMP_SNOOPING_GROUP++;
					}
					else{
						printk("remove member port:%x\n",port);
						
#ifdef CONFIG_USBCLIENT
                                      rtl8305sc_setAsicVlan(i+11,i+0x100,snooping_table[i].member_port);
#else
					    rtl8305sc_setAsicVlan(i+9,i+0x100,snooping_table[i].member_port);
#endif
					}
#endif
					break;
				}
			}
			printk("\n");
		}
	}
#endif
}

#ifdef CONFIG_EXT_SWITCH
void process_eth_vlan(struct sk_buff *skb, int *tag)
{
	//unsigned char *dest;
	//tylo, for port-mapping
	unsigned int source_port;
	//unsigned char tmp_mac[12];
	#define TYPE_OFFSET_802_1Q		12
	#define TYPE_802_1Q				0x8100
	#define VID_OFFSET_802_1Q		14
	
	source_port = SW_PORT_NUM;
	// save the vlan tag here
	//*(unsigned long *)&skb->cb[40] = desc->opts2;	
	//printk("*****vlan tag=%d\n", *tag);
	for(source_port=0;source_port<SW_PORT_NUM;source_port++)
		if (*tag == rtl8305_info.vlan[rtl8305_info.phy[virt2phy[source_port]].vlanIndex].vid)
			break;

	if (source_port < SW_PORT_NUM) {
	  
	    // Added by Mason Yu
	    // Save port name
	    //strcpy(skb->switch_port, dev_sw[source_port]->name);
	    skb->switch_port = (char *)dev_sw[source_port]->name;
	
	    // Port-mapping --- set the vlan_member
	    skb->vlan_member = rtl8305_info.vlan[rtl8305_info.phy[virt2phy[source_port]].vlanIndex].member;
	    // Kaohj -- for vlan-grouping, SW_PORT+cpu-port+system-lan-port
	    skb->pvid = rtl8305_info.phy[SW_PORT_NUM+1+virt2phy[source_port]].vlanIndex;
	    //printk("source_port=%d, pvid=%d\n", source_port, skb->pvid);
	     

          //user switch port to checked by the upper layer 	
#if 0 
  
	 // Kaohj --- mark the virtual port to be checked by the upper layer
	    if(((*(unsigned char *)(skb->data+PROTOCOL_OFFSET))==IGMP_PROTOCOL))
			skb->nfmark = source_port+1;
#endif

  #if 0 
  // not necessary since we don't register sw_port now
	    dest = skb->mac.ethernet->h_dest;
	    if (dest[0] & 1) {
		    struct sk_buff *skb2;
		
		    skb2 = skb_clone(skb, GFP_ATOMIC);
		    if (skb2!=NULL)
		        // find sw-port to do bridge
		        // todo ...
		        skb2->dev = dev_sw[source_port];		        
			    netif_rx(skb2);
	    }
	    else if (memcmp(skb->dev->dev_addr, dest, 6)) {
		    // not my eth addr, find sw-port to do bridge
		    // todo ...
		    skb->dev = dev_sw[source_port];
	    }
#endif
    };
}
#endif	// of CONFIG_EXT_SWITCH

//cathy
//fix tx fail problem under bidirectional routing mode with long packets
//since re8670_tx may be called by nic_tx2 and interrupt at the same time,
//tx_tail's value will be fetched incorrectly
atomic_t lock_tx_tail = ATOMIC_INIT(0);

__IRAM void nic_tx2(struct sk_buff *skb)
{
	struct re_private *cp = eth_net_dev->priv;
	skb->users.counter=1;
	atomic_set(&lock_tx_tail, 1);
	re8670_tx(cp);
	atomic_set(&lock_tx_tail, 0);
	re8670_start_xmit(skb,eth_net_dev);
}

#ifdef FAST_ROUTE_Test

extern void sar_tx(struct sk_buff *skb, unsigned int ch_no);
extern unsigned int find_mac(unsigned char *buf);	

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_NET
__NOMIPS16
#endif
void nic_tx(struct sk_buff *skb)
{
cli();	
	re8670_start_xmit(skb,eth_net_dev);
sti();	
}
unsigned int nic_rx_counter=0;
#define MAX_MAC_ADDR	8
#define	Enable_VC_CNT	8
typedef struct _mac_table_items
{
	unsigned int index;
	unsigned int enable;
	unsigned char mac[MAX_MAC_ADDR][6];
} mac_table_items;

extern mac_table_items  fast_route_ch_no[Enable_VC_CNT];
#endif	//FAST_ROUTE_Test

static inline void re8670_rx_skb (struct re_private *cp, struct sk_buff *skb,
			      DMA_DESC *desc)
{
#ifdef CONFIG_EXT_SWITCH
	int tag=0;
#endif
	skb_debug(skb, 1, 0);

#ifdef CONFIG_NET_WIRELESS
#ifdef WLANBR_SHORTCUT
	extern struct net_device *get_shortcut_dev(unsigned char *da);
	struct net_device *wlandev;
#endif
#endif

#ifdef FAST_ROUTE_Test
#if 0
	unsigned int ch_no;	

	if (fast_route_mode) {
		if (( (ch_no = find_mac(skb->data)) != -1) && fast_route_ch_no[ch_no].enable) {
			skb->dev = sar_dev;
			skb->pkt_type = PACKET_FASTROUTE;
			sar_tx(skb, ch_no);
			return;
		}
	}
#endif
#endif

	skb->dev = eth_net_dev;
	skb->protocol = eth_type_trans (skb, cp->dev);

#ifdef CONFIG_EXT_SWITCH
#if 1
	if (desc->opts2 & 0x10000) {
		tag = ((desc->opts2 & 0x0f)<<8); // VIDH
		tag |= ((desc->opts2 & 0xff00)>>8); // VIDL
#ifdef CONFIG_RE8305
		if (!enable_virtual_port)
#endif	
	        { // if virtual-port is enabled, then the vlan tag has been replaced by switch(830x)
			// marked for 802.1p based priority
			skb->nfmark = ((desc->opts2 & 0x00e0) >> 5) + 1;
		}
		
		//if(!enable_IGMP_SNP)
			process_eth_vlan(skb, &tag);
		
	}
#else	
	
	if (desc->opts2 & 0x10000) {
		if(!enable_IGMP_SNP)
			process_eth_vlan(skb, &tag);
		else{
			tag = ((desc->opts2 & 0x0f)<<8); // VIDH
			tag |= ((desc->opts2 & 0xff00)>>8); // VIDL
		}
	}
#endif	

	// Kaohj
	// Port-Mapping: vlan member is set at sar driver on receving packets
	//skb->vlan_member=0;
    if(enable_IGMP_SNP){
        check_IGMP_snoop_rx(skb, tag);
    }
#endif    

	cp->net_stats.rx_packets++;
	cp->net_stats.rx_bytes += skb->len;
	cp->dev->last_rx = jiffies;

#if CP_VLAN_TAG_USED
	if (cp->vlgrp && (desc->opts2 & RxVlanTagged)) {
		vlan_hwaccel_rx(skb, cp->vlgrp, desc->opts2 & 0xffff);
	} else
#endif
		skb->stamp = xtime;		
		//printk("re8670_rx_skb: skb->switch_port = %s\n", skb->switch_port);
#ifdef CONFIG_NET_WIRELESS
#ifdef WLANBR_SHORTCUT
		if ((cp->dev->br_port) &&			
			(wlandev = get_shortcut_dev(skb->mac.ethernet->h_dest)) != NULL) {
			if(netif_running(wlandev)){			
				skb_push(skb, ETH_HLEN);			
				memcpy(skb->data, skb->mac.ethernet, ETH_HLEN);			
				wlandev->hard_start_xmit(skb, wlandev);		
			}
		}		
		else		
#endif
#endif
#ifdef CONFIG_RTK_VOIP_RX_INPUT_QUEUE
	if (skb_is_voip(skb)) 
	{ 
		netif_rx_voip(skb);			
	}
	else 
#endif
	if (netif_rx(skb) == NET_RX_DROP)
		cp->net_stats.rx_dropped++;


}

static void re8670_rx_err_acct (struct re_private *cp, unsigned rx_tail,
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

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_NET
__NOMIPS16
#endif
static void re8670_rx_frag (struct re_private *cp, unsigned rx_tail,
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

	//copy_skb = dev_alloc_skb (target_len + RX_OFFSET);
	copy_skb = re8670_getAlloc();
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
	skb_reserve(copy_skb, RX_OFFSET);
	skb_put(copy_skb, target_len);
	if (frag_skb) {
		memcpy(copy_skb->data, frag_skb->data, orig_len);
		dev_kfree_skb_irq(frag_skb);
	}
#if 0
	pci_dma_sync_single(cp->pdev, cp->rx_skb[rx_tail].mapping,
			    len, PCI_DMA_FROMDEVICE);
#endif
	memcpy(copy_skb->data + orig_len, skb->data, len);

	copy_skb->ip_summed = CHECKSUM_NONE;

	if (last_frag) {
		if (status & (RxError)) {
			re8670_rx_err_acct(cp, rx_tail, status, len);
			dev_kfree_skb_irq(copy_skb);
		} else
			re8670_rx_skb(cp, copy_skb, &cp->rx_ring[rx_tail]);
		cp->frag_skb = NULL;
	} else {
		cp->frag_skb = copy_skb;
	}
}

static inline unsigned int re8670_rx_csum_ok (u32 status)
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

unsigned int eth_rx_count=0;
static unsigned char eth_close=0;
int eth_poll; // poll mode flag
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_NET
__NOMIPS16
#endif
void eth_poll_schedule()
{
	struct re_private *cp = eth_net_dev->priv;
	
	tasklet_hi_schedule(&cp->rx_tasklets);
}

static void re8670_rx (struct re_private *cp)
{
	unsigned rx_tail = cp->rx_tail;
	unsigned rx_work = 100;
	u16 status;

//__cli();
//scout('<');
	//protect eth rx while reboot
	if(eth_close == 1)
		return;
	// Kaohj
	if (eth_poll) // in poll mode
		rx_work = 2; // rx rate is rx_work * 100 pps (timer tick is 10ms)
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

		len = (status & 0x07ff) - 4;

		//mapping = cp->rx_skb[rx_tail].mapping;
#if 0//ql: in general, all pkts are send in one segment....?
		if ((status & (FirstFrag | LastFrag)) != (FirstFrag | LastFrag)) {
			re8670_rx_frag(cp, rx_tail, skb, status, len);
			goto rx_next;
		}
#endif
		if (status & (RxError)) {
			re8670_rx_err_acct(cp, rx_tail, status, len);
			goto rx_next;
		}

		///if (netif_msg_rx_status(cp))
		///	printk(KERN_DEBUG "%s: rx slot %d status 0x%x len %d\n",
		///	       cp->dev->name, rx_tail, status, len);

		buflen = cp->rx_buf_sz + RX_OFFSET;
		//new_skb = dev_alloc_skb (buflen);
		new_skb = re8670_getAlloc();
		if (!new_skb) {
			//cp->net_stats.rx_dropped++;
			// not enough memory, rx deferred
			//queue_task (&cp->rx_task, &tq_immediate);
			//mark_bh (IMMEDIATE_BH);
			//8/25/05' hrchen, disable RX_OK to prevent IRQ again
			//re8670_disable_rx();
			//printk("not enough memory, nic rx deferred\n");
			break;
			//goto rx_next;
		}
#ifdef CONFIG_RTL8672
		// Kaohj -- invalidate new_skb before NIC DMA
		dma_cache_wback_inv((unsigned long)new_skb->data, len);
#endif
		//if ((u32)new_skb->data &0x3)
		//	printk(KERN_DEBUG "new_skb->data unaligment %8x\n",(u32)new_skb->data);

		/*skb_reserve(new_skb, RX_OFFSET);*/
		//new_skb->dev = cp->dev;
#if 0
		pci_unmap_single(cp->pdev, mapping,
				 buflen, PCI_DMA_FROMDEVICE);
#endif
		
		/* Handle checksum offloading for incoming packets. */
		if (re8670_rx_csum_ok(status))
			skb->ip_summed = CHECKSUM_UNNECESSARY;
		else
			skb->ip_summed = CHECKSUM_NONE;

		skb_reserve(skb, RX_OFFSET);
		//dma_cache_wback_inv((unsigned long)skb->data, len);
		skb_put(skb, len);
		mapping =
		cp->rx_skb[rx_tail].mapping = (u32)new_skb->tail | UNCACHE_MASK;
#if 0
			pci_map_single(cp->pdev, new_skb->tail,

 					buflen, PCI_DMA_FROMDEVICE);
#endif
		eth_rx_count++;
		
	/*	
		printk("Rx: ");
		for (len=0; len < 32; len++)
			printk("%02x ", *(skb->data+len));
		printk("\n");
	*/	
		if(skb->data[12]==0x81 && skb->data[13]==0x00){
			//catch PRIO for WMM
			skb->cb[0]=skb->data[14]>>5;
		}
#ifdef CONFIG_RTL8672
	if( (	RTL_R16(RxCDO) - rx_tail + RE8670_RX_RING_SIZE)%RE8670_RX_RING_SIZE > 60)
		printk("Not enough descriptor ..(available desc no %d)\n", RE8670_RX_RING_SIZE - (RTL_R16(RxCDO) - rx_tail + RE8670_RX_RING_SIZE)%RE8670_RX_RING_SIZE);	

		RTL_R8(EthrntRxCPU_Des_Num)=rx_tail;	//flowctrl mechanism: driver need to maintain this pointer

#endif
		re8670_rx_skb(cp, skb, desc);
		
		cp->rx_skb[rx_tail].skb = new_skb;
		cp->rx_ring[rx_tail].addr = mapping;

rx_next:
		///cp->rx_ring[rx_tail].opts2 = 0;
		desc->opts1 = (DescOwn | cp->rx_buf_sz) | 
		    ((rx_tail == (RE8670_RX_RING_SIZE - 1))?RingEnd:0);
		rx_tail = NEXT_RX(rx_tail);
	}

//static int xxx = 0;
//if ( ((xxx++)%1000)==0) printk("re %d\n", 100-rx_work );
	//if (!rx_work) {
		//printk(KERN_WARNING "%s: rx work limit reached\n", cp->dev->name);
	//    tasklet_hi_schedule(&cp->rx_tasklets);
    //};

//scout('>');
	cp->rx_tail = rx_tail;
#ifdef CONFIG_RTL8671	
	//7/5/05' hrchen, NIC_IRQ will be disabled if RDU happen. Open NIC_IRQ here.
//__sti();
	// Kaohj
	if (eth_poll) { // in eth poll mode
		// clear ISR
		status = RTL_R16(ISR);
		RTL_W16(ISR,status);
	}
	else
#ifdef CONFIG_RTL8672
	REG32(GIMR)|=Ethernet_IM;
#else
	REG16(GIMR)|=Ethernet_IM;
#endif

	//enable_lx4180_irq(cp->dev->irq);
#endif

}

#if 0
static void nic_rx(void){
	re8670_rx(reDev);
	return;
}
#endif

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_NET
__NOMIPS16
#endif
void re8670_interrupt (int irq, void *dev_instance, struct pt_regs *regs)
{
	struct net_device *dev = dev_instance;
	struct re_private *cp = dev->priv;
	u16 status;

//scout('4');	
	if (!(status = RTL_R16(ISR)))
		return;

	//if (netif_msg_intr(cp))
	//	printk(KERN_DEBUG "%s: intr, status %04x cmd %02x \n",
	//	        dev->name, status, RTL_R8(CMD));

	//spin_lock(&cp->lock);

	//if (status & (TX_OK | TX_ERR | TX_EMPTY | SW_INT)) {
	if (status & TX_EMPTY) {
		// JONAH
		//mark_bh(NIC_TX_BH);
		//queue_task (&cp->tx_task, &tq_immediate);
		//mark_bh (IMMEDIATE_BH);
		//tasklet_schedule(&cp->tx_tasklets);
//scout('T');	

		//cathy
		if( !atomic_read(&lock_tx_tail) ) {
			re8670_tx(cp);
		}
	}

	if (status & (RX_OK | RX_ERR | RX_EMPTY | RX_FIFOOVR)) {
		// JONAH
#ifdef CONFIG_RTL8672
		REG32(GIMR)&=(~Ethernet_IM);
#else
		REG16(GIMR)&=(~Ethernet_IM);
#endif
		//disable_lx4180_irq(cp->dev->irq);
		//mark_bh(NIC_RX_BH);
		//queue_task (&cp->rx_task, &tq_immediate);
		//mark_bh (IMMEDIATE_BH);
//scout('R');	
		tasklet_hi_schedule(&cp->rx_tasklets);
		
		//7/5/05' hrchen, clear NIC ISR of 8671 will not clear RDU if no rx desc available.
		//The IRQ will never stop and RX_BH will never run and free rx desc.
		//Clear NIC GIMR here if RDU. nic_rx must reopen NIC_IM.
		//if (status&RX_EMPTY) {  //RDU happened, clear NIC_IM
		//};
	}

	RTL_W16(ISR,status);


	//spin_unlock(&cp->lock);
}

#if 0
static void nic_tx(void){
	re8670_tx(reDev);
	return;
}
#endif

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_NET
__NOMIPS16
#endif
__IRAM void re8670_tx (struct re_private *cp)
{
	//unsigned tx_head = cp->tx_hqhead;
	unsigned tx_tail = cp->tx_hqtail;
	u32 status;
	struct sk_buff *skb;
	
	if(eth_close == 1){
		return;
	}

	//while (tx_tail != tx_head) {
	while (!((status = (cp->tx_hqring[tx_tail].opts1))& DescOwn)) {
		//cp->tx_hqring[tx_tail].addr =0xaa550000;
		//cp->tx_hqring[tx_tail].opts1 =0x0;
		
	    skb = cp->tx_skb[tx_tail].skb;
		if (!skb)
		    break;
			//BUG();
		
		//rmb();
		//status = (cp->tx_hqring[tx_tail].opts1);
		//if (status & DescOwn)
		//	break;
		if (status & TxError)
		{
			cp->net_stats.tx_errors++;
			if (status & TxFIFOUnder)
				cp->net_stats.tx_fifo_errors++;
		}
		cp->net_stats.tx_packets++;
		cp->net_stats.tx_bytes += skb->len;
		//if (netif_msg_tx_done(cp))
		//	printk(KERN_DEBUG "%s: tx done, slot %d\n", cp->dev->name, tx_tail);
		if(skb->destructor==0)
			dev_kfree_skb(skb);
		else
#ifdef ZTE_FAST_BRIDGE
			local_dev_kfree_skb_irq(skb);
#else
			dev_kfree_skb_irq(skb);
#endif	
 		cp->tx_skb[tx_tail].skb = NULL;
		tx_tail = NEXT_TX(tx_tail);
	}

	cp->tx_hqtail = tx_tail;

#if 1
	if (netif_queue_stopped(cp->dev)/* && (TX_HQBUFFS_AVAIL(cp) > (MAX_SKB_FRAGS + 1))*/){
#ifdef ZTE_FAST_BRIDGE
		local_netif_wake_queue(cp->dev);
#else
		netif_wake_queue(cp->dev);
#endif
		printk("queue stopped\n");
	}
#endif
}

#ifdef CONFIG_EXT_SWITCH

struct timer_list seq_timer;
struct timer_arg timeout_arg;
//unsigned char skb_query_data[2048];
int timer_running=0;
//struct sk_buff skb_query;

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_NET
__NOMIPS16
#endif
//void snp_timeout_function(void)
void snp_timeout_function(struct sk_buff *skb)
{
	int idx;
	printk("snp_timeout\n");
	for(idx=0;idx<MAX_IGMP_SNOOPING_GROUP;idx++){
		if(snooping_table[idx].multicast_addr!=0xc0000000){
			printk("general query:update VLAN accroding to tmp_member_port\n");
			if(snooping_table[idx].tmp_member_port==0 && snooping_table[idx].tmp_member_wlan==0){
				if (handle_leave == 1) {
					if (leave_query_count > 0 && leave_query_count < 3) {
						snooping_table[idx].timer_running=0;
						snooping_table[idx].member_wlan=snooping_table[idx].tmp_member_wlan;
						timer_running=0;
						handle_IGMP_leave(idx, skb);
						dev_kfree_skb(skb);
						return;
					}
					leave_query_count = 0;
					handle_leave = 0;
					if (leave_port != 5)
						snooping_table[idx].member_port &= ~(1<<leave_port);
					else
						snooping_table[idx].member_wlan=0;
					if (snooping_table[idx].member_port==0 && snooping_table[idx].member_wlan==0){
						printk("remove this IGMP group:%x\n\n", snooping_table[idx].multicast_addr);
						snooping_table[idx].multicast_addr=0xc0000000;
						snooping_table[idx].member_port =0;
						REST_IGMP_SNOOPING_GROUP++;
					}
					else{
						printk("remove member port:%x\n", leave_port);
#ifdef CONFIG_USBCLIENT
						rtl8305sc_setAsicVlan(idx+11,idx+0x100,snooping_table[idx].member_port);
#else
						rtl8305sc_setAsicVlan(idx+9,idx+0x100,snooping_table[idx].member_port);
#endif
					}
				}
				printk("delete VLAN %d\n",idx+9);
				snooping_table[idx].member_port=0;
				snooping_table[idx].multicast_addr=0xc0000000;
				REST_IGMP_SNOOPING_GROUP++;
			}
			else{
				if (handle_leave == 1 && snooping_table[idx].multicast_addr == leave_multicast_addr) {
					unsigned char tmp_port;
					tmp_port = snooping_table[idx].member_port & (~leave_port);
					if (tmp_port == snooping_table[idx].tmp_member_port) {
						if (leave_query_count > 0 && leave_query_count < 3) {
							snooping_table[idx].timer_running=0;
							snooping_table[idx].member_wlan=snooping_table[idx].tmp_member_wlan;
							timer_running=0;
							handle_IGMP_leave(idx, skb);
							dev_kfree_skb(skb);
							return;
						}
						printk("remove member port:%x\n", leave_port);
						leave_query_count = 0;
						handle_leave = 0;
						leave_port = 0;
					}
					else {
						leave_query_count = 0;
						handle_leave = 0;
						leave_port = 0;
					}
				}
				printk("update VLAN %x        VID=%x\n",idx+9,snooping_table[idx].VLANID);
				if(snooping_table[idx].member_port!=snooping_table[idx].tmp_member_port)
#ifdef CONFIG_USBCLIENT				
					rtl8305sc_setAsicVlan(idx+11,idx+0x100,snooping_table[idx].tmp_member_port);
#else      
					rtl8305sc_setAsicVlan(idx+9,idx+0x100,snooping_table[idx].tmp_member_port);
#endif					
				snooping_table[idx].member_port=snooping_table[idx].tmp_member_port;
			}
			snooping_table[idx].timer_running=0;
			snooping_table[idx].member_wlan=snooping_table[idx].tmp_member_wlan;
		}
	}
	dev_kfree_skb(skb);
	timer_running=0;
}
struct timer_arg{
	unsigned char count;
	struct sk_buff *skb;
	struct sk_buff *old_skb;
	int expire;
};

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_NET
__NOMIPS16
#endif
void sequential_send(unsigned long arg){
	struct timer_arg *t_arg=(struct timer_arg *)arg;
	unsigned char multicast_insert[16];
	unsigned char insertVID[2];
	//int igmp_offset,testi;
	struct sk_buff *skb=t_arg->skb;
	struct re_private *cp = eth_net_dev->priv;
	//struct skb_shared_info skbshare;
	unsigned tmp_imr;
	struct sk_buff *skb2;

	tmp_imr=RTL_R16(IMR);
	if(tmp_imr==0x0)
		return;
	//skbshare.nr_frags=0;
	//skb->end=(unsigned char*)&skbshare;
	insertVID[0]=0x00; insertVID[1]=0x00;//TYPE of 802.1Q
	printk("sequential_send:%d   %d\n",t_arg->count,t_arg->expire);
	if(t_arg->count==SW_PORT_NUM){
		//add VID
		memcpy(multicast_insert,skb->data,12);
		multicast_insert[12]=0x81; multicast_insert[13]=0x00;

     insertVID[1]+=(t_arg->count+SW_PORT_NUM);

		memcpy(multicast_insert+14,insertVID,2);
		skb_push(skb,4);
		memcpy(skb->data, multicast_insert,16);

		//send out the packet
		spin_lock(&cp->lock);
		//skb->users.counter=2;
		//re8670_start_xmit(skb,eth_net_dev);
		skb2 = skb_clone(skb, GFP_ATOMIC);
		re8670_start_xmit(skb2, eth_net_dev);
		spin_unlock(&cp->lock);
		//set timer
		t_arg->count--;
		seq_timer.data=(unsigned long)t_arg;
		seq_timer.expires=jiffies+t_arg->expire*HZ;
		seq_timer.function=sequential_send;
		//printk("skb->user=%d   add timer..........expires=%x\n",skb->users,seq_timer.expires);
		add_timer(&seq_timer);
	}
	else if(t_arg->count!=0){
		skb->data[15]--;
		t_arg->count--;
		//skb_shinfo(skb)->nr_frags =0;
		//skb->users.counter=2;
		//re8670_start_xmit(skb,eth_net_dev);
		skb2 = skb_clone(skb, GFP_ATOMIC);
		re8670_start_xmit(skb2, eth_net_dev);
		//set timer
		seq_timer.data=(unsigned long)t_arg;
		seq_timer.expires=jiffies+t_arg->expire*HZ;
		seq_timer.function=sequential_send;
		//printk("skb->user=%d  add timer..........expires=%x\n",skb->users,seq_timer.expires);
		add_timer(&seq_timer);
	}
	else{
		//snp_timeout_function();
		snp_timeout_function(t_arg->old_skb);
		dev_kfree_skb(skb);
	}
}

#if 0
/* taken from TCP/IP Illustrated Vol. 2(1995) by Gary R. Wright and W. Richard Stevens. Page 236 */
static unsigned short iphdr_cksum_calc(struct iphdr *ip, int len)
{
	unsigned long sum = 0;		/* assume 32 bit long, 16 bit short */

	while (len > 1){
		sum += *((unsigned short*) ip)++;
		if(sum & 0x80000000)	/* if high order bit set, fold */
			sum = (sum & 0xFFFF) + (sum >> 16);
		len -= 2;
	}
	if (len)	/* take care of left over byte */
		sum += (unsigned short) *(unsigned char *)ip;
	while (sum>>16)
		sum = (sum & 0xFFFF) + (sum >> 16);
	return ~sum;
}
#endif

static char iphdr_data[20] = {0x45, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x40, 0x00,
	0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x01};
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_NET
__NOMIPS16
#endif
int handle_IGMP_leave(int grp, struct sk_buff *skb)
{
#define IP_HDLEN_OFFSET				0x0e
#define IP_HDR_DSTIP_OFFSET			0x1e
#define IP_HDR_SRCIP_OFFSET			0x1a
#define IGMP_TYPE_QUERY				0x11
#define MULTICAST_ADDR_REPORT_OFFSET	0x04
#define IGMP_PROTOCOL					0x02

	int igmp_offset;
	unsigned short tmp_csum;
	unsigned char iphdr_insert[20], dstmac[6];
	struct net_device *dev = eth_net_dev;
	struct ethhdr *eth;
	struct iphdr *iph;
	struct sk_buff *skb_query, *skb_backup;
	int *group_addr;
	short ip_check;

	// skb->data == IP packet; IGMP Leave received
	if (timer_running==0)
		if (snooping_table[grp].multicast_addr!=0xc0000000 && snooping_table[grp].timer_running==0) {
			if ((skb_backup = dev_alloc_skb(2048)) == NULL) {
				printk("\n%s alloc skb_backup failed!\n", __FUNCTION__);
				return 0;
			}
			memcpy(skb_put(skb_backup, skb->len),skb->data,skb->len);
			if ((skb_query = dev_alloc_skb(128)) == NULL) {
				printk("\n%s alloc skb failed!\n", __FUNCTION__);
				return 0;
			}
			// preparing IGMP Query
			skb_reserve(skb_query, 4); // for vlan tag
			skb_put(skb_query, ETH_HLEN+20+8); // eth+ip+igmp
			// skb_query->data == Ethernet packet
			igmp_offset=((*(unsigned char *)skb->data)&0x0f)*4;
			snooping_table[grp].tmp_member_port=0;
			snooping_table[grp].tmp_member_wlan=0;
			snooping_table[grp].timer_running=1;
			snooping_table[grp].snp_timer.expires=jiffies+(*(unsigned char *)(skb->data+igmp_offset+1))/10*HZ;
			snooping_table[grp].snp_timer.function=(void (*)(unsigned long))snp_timeout_function;
			snooping_table[grp].snp_timer.data=(unsigned long)grp;

			// handle mac header
			eth = (struct ethhdr *)skb_query->data;
			eth->h_proto = __constant_htons(ETH_P_IP);
			memcpy(eth->h_source, dev->dev_addr, dev->addr_len);
			dstmac[0] = 0x01; dstmac[1] = dstmac[3] = dstmac[4] = 0x00; dstmac[2] = 0x5e; dstmac[5] = 0x01;
			memcpy(eth->h_dest, dstmac, ETH_ALEN);
			// handle IP header
			memcpy(skb_query->data+ETH_HLEN, iphdr_data, 20);
			memcpy(skb_query->data+IP_HDR_SRCIP_OFFSET, skb->data+12, 4); // src IP
			skb_query->data[IP_HDR_SRCIP_OFFSET+3]++;
			skb_query->data[IP_HDR_DSTIP_OFFSET] = 0xe0; // dst IP: 224.0.0.1
			skb_query->data[IP_HDR_DSTIP_OFFSET+1] = 0;
			skb_query->data[IP_HDR_DSTIP_OFFSET+2] = 0;
			skb_query->data[IP_HDR_DSTIP_OFFSET+3] = 1;
			// handle IGMP Query message
			igmp_offset=ETH_HLEN+20;
			skb_query->data[igmp_offset] = IGMP_TYPE_QUERY;
			timeout_arg.expire = 1;
			//change IGMP max. response time
			skb_query->data[igmp_offset+1] = 0x01;
			//recompute IGMP checksum
			skb_query->data[igmp_offset+2] = 0;
			skb_query->data[igmp_offset+3] = 0;
			group_addr = (int *)&skb_query->data[igmp_offset+MULTICAST_ADDR_REPORT_OFFSET];
			*group_addr = snooping_table[grp].multicast_addr;
			tmp_csum = ip_compute_csum(skb_query->data+igmp_offset, 8);
			//set IGMP checksum
			skb_query->data[igmp_offset+2] = ((tmp_csum&0xff00)>>8);
			skb_query->data[igmp_offset+3] = (tmp_csum&0xff);
			skb_query->data[igmp_offset+MULTICAST_ADDR_REPORT_OFFSET+4] = '\0';
			skb_query->data[IP_HDR_DSTIP_OFFSET-6] = 0;
			skb_query->data[IP_HDR_DSTIP_OFFSET-5] = 0;
			// set IP checksum
			memcpy(iphdr_insert, skb_query->data+ETH_HLEN, 20);
			ip_check = ip_compute_csum(iphdr_insert, 20);
			(*(unsigned char *)(skb_query->data+IP_HDR_DSTIP_OFFSET-6)) = ((ip_check&0xff00)>>8);
			(*(unsigned char *)(skb_query->data+IP_HDR_DSTIP_OFFSET-5)) = (ip_check&0xff);

			timeout_arg.count = SW_PORT_NUM;
			timeout_arg.skb = skb_query;
			timeout_arg.old_skb = skb_backup;
			printk("\n\n\n");
			timer_running=1;
			leave_query_count ++;
			sequential_send((unsigned long)&timeout_arg);
		}
	return 1;
}

#endif

void re8670_mFrag_xmit (struct sk_buff *skb, struct re_private *cp, unsigned *entry
#ifdef CONFIG_EXT_SWITCH
	, u32 txOpts2
#endif
)
{
	u32 eor;
#if defined(CP_VLAN_TAG_USED) || defined (CONFIG_VLAN_8021Q) || defined (CONFIG_VLAN_8021Q_MODULE)
        u32 vlan_tag = 0;
#endif
		DMA_DESC *txd;
		u32 first_len, first_mapping;
		int frag, first_entry = *entry;
		u32 firstFragAddr;

		/* We must give this initial chunk to the device last.
		 * Otherwise we could race with the device.
		 */
		first_len = skb->len - skb->data_len;
		/*first_mapping = pci_map_single(cp->pdev, skb->data,*
					       first_len, PCI_DMA_TODEddVICE);*/
		firstFragAddr = (u32)skb->data;
		first_mapping = (u32)skb->data|UNCACHE_MASK;
		cp->tx_skb[*entry].skb = skb;
		cp->tx_skb[*entry].mapping = first_mapping;
		cp->tx_skb[*entry].frag = 1;
		*entry = NEXT_TX(*entry);

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

			eor = (*entry == (RE8670_TX_RING_SIZE - 1)) ? RingEnd : 0;
			ctrl = eor | len | DescOwn | TxCRC;
			if (frag == skb_shinfo(skb)->nr_frags - 1)
				ctrl |= LastFrag;

			txd = &cp->tx_hqring[*entry];
			CP_VLAN_TX_TAG(txd, vlan_tag);
#ifdef CONFIG_EXT_SWITCH
			if (txOpts2) {
				// add vlan tag
				txd->opts2 = txOpts2;
			}
#endif
			txd->addr = (mapping);
			wmb();
#ifdef CONFIG_RTL8672
			// Kaohj
			dma_cache_wback_inv((unsigned long)this_frag->page_offset, len);
#endif			
			txd->opts1 = (ctrl);
			wmb();
			cp->tx_skb[*entry].skb = skb;
			cp->tx_skb[*entry].mapping = mapping;
			cp->tx_skb[*entry].frag = frag + 2;
			*entry = NEXT_TX(*entry);
		}

		txd = &cp->tx_hqring[first_entry];
		CP_VLAN_TX_TAG(txd, vlan_tag);
		
		txd->addr = (first_mapping);
		
		wmb();
#ifdef CONFIG_RTL8672
		// Kaohj
		dma_cache_wback_inv((unsigned long)firstFragAddr, first_len);
#endif
		
		eor = (first_entry == (RE8670_TX_RING_SIZE - 1)) ? RingEnd : 0;
		txd->opts1 = (first_len | FirstFrag | DescOwn|TxCRC|eor);
		
		wmb();
}

#ifdef CONFIG_EXT_SWITCH
//return 0: pkt is IGMP query and already handled return to tx caller
//       1: not IGMP query, normal tx
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_NET
__NOMIPS16
#endif
int handle_IGMP_query(struct sk_buff *skb)
{
#define IP_TYPE_OFFSET				0x0c
#define IP_TYPE						0x0800
#define IP_HDLEN_OFFSET			0x0e
#define IGMP_PROTOCOL_OFFSET		0x17
#define IGMP_PROTOCOL				0x02
#define IGMP_TYPE_QUERY			0x11
#define MULTICAST_ADDR_REPORT_OFFSET	0x04

		int igmp_offset,testi;
		int i=0;
		unsigned short tmp_csum;
		// Kaohj
		char mrt;
		struct sk_buff *skb_query, *skb_backup;
		//unsigned char skb_query_data[2048];

		//check if incoming frame is multicast packet
		if((skb->data[0]!=0x01) || (skb->data[1]!=0x0) || ((skb->data[2]&0xfe)!=0x5e)){
			return 1;
		}		

		igmp_offset=((*(unsigned char *)(skb->data+IP_HDLEN_OFFSET))&0x0f)*4+14;
		if((READHWD(skb->data+IP_TYPE_OFFSET))==IP_TYPE && 
			((*(unsigned char *)(skb->data+IGMP_PROTOCOL_OFFSET))==IGMP_PROTOCOL) && (*(unsigned char *)(skb->data+igmp_offset))==IGMP_TYPE_QUERY){
			if( (*(unsigned char *)(skb->data+igmp_offset+MULTICAST_ADDR_REPORT_OFFSET))==0){
				//general query
				if(timer_running==0){
					for(i=0;i<MAX_IGMP_SNOOPING_GROUP;i++){
						if(snooping_table[i].multicast_addr!=0xc0000000 && snooping_table[i].timer_running==0){
							if ((skb_backup = dev_alloc_skb(2048)) == NULL) {
								printk("\n%s alloc skb_backup failed!\n", __FUNCTION__);
								return 0;
							}
							memcpy(skb_put(skb_backup, skb->len),skb->data,skb->len);
							snooping_table[i].tmp_member_port=0;
							snooping_table[i].tmp_member_wlan=0;
							snooping_table[i].timer_running=1;
							snooping_table[i].snp_timer.expires=jiffies+(*(unsigned char *)(skb->data+igmp_offset+1))/10*HZ;
							snooping_table[i].snp_timer.function=(void (*)(unsigned long))snp_timeout_function;
							snooping_table[i].snp_timer.data=(unsigned long)i;
							//skb_query.head=skb_query_data;
							//skb_query.data=skb_query_data+4;

							//get max. response time
							mrt = *(unsigned char *)(skb->data+igmp_offset+1);
							if(mrt/(10*4)==0){
								timeout_arg.expire=1;
							}
							else
								timeout_arg.expire=(*(unsigned char *)(skb->data+igmp_offset+1))/(10*4);
							//change IGMP max. response time
							if (mrt != 0) {// not IGMPv1 query
								mrt = mrt>>2;
								if (mrt==0)
									mrt = 1;
							}
							*(unsigned char *)(skb->data+igmp_offset+1)=mrt;
							//*(unsigned char *)(skb->data+igmp_offset+1)=*(unsigned char *)(skb->data+igmp_offset+1)>>2;
							//recompute IGMP checksum
							(*(unsigned char *)(skb->data+igmp_offset+2))=0;
							(*(unsigned char *)(skb->data+igmp_offset+3))=0;
							tmp_csum=ip_compute_csum(skb->data+igmp_offset,8);
							//set IGMP checksum
							(*(unsigned char *)(skb->data+igmp_offset+2))=((tmp_csum&0xff00)>>8);
							(*(unsigned char *)(skb->data+igmp_offset+3))=(tmp_csum&0xff);
							//memcpy(skb_query.data,skb->data,skb->len);
							//skb_query.len=skb->len;
							if ((skb_query = dev_alloc_skb(2048)) == NULL) {
								printk("\n%s alloc skb failed!\n", __FUNCTION__);
								return 0;
							}
							skb_reserve(skb_query, 4);
							memcpy(skb_put(skb_query, skb->len),skb->data,skb->len);
							//dev_kfree_skb(skb);

							timeout_arg.count=SW_PORT_NUM;
							//timeout_arg.skb=&skb_query;
							timeout_arg.skb=skb_query;
							timeout_arg.old_skb = skb_backup;
							if(timeout_arg.expire==0)//test
							for(testi=0;testi<0x40;testi++){
								if(testi%0x10==0 &&testi!=0)
									printk("\n");
								//printk("%x  ",skb_query.data[testi]);
								printk("%x  ",skb_query->data[testi]);
							}
							printk("\n\n\n");
							timer_running=1;
							sequential_send((unsigned long)&timeout_arg);
						}
						if(timer_running==1)
							break;
					}
				}
				if(i==MAX_IGMP_SNOOPING_GROUP){
					return 1;
				}
			}
			else
				return 1;
			
			return 0;
		}	
		return 1;
}
#endif

//return 0 for normal packet, 1 to drop multicast packet
int check_wlan_mcast_tx(struct sk_buff *skb)
{
#ifdef CONFIG_EXT_SWITCH
#define IP_HDR_DES_OFFSET		0x1e
//unsigned char multicast_insert[16];
    int i;
    unsigned int multicast_addr;	

		//check if incoming frame is multicast packet
		if((skb->data[0]!=0x01) || (skb->data[1]!=0x0) || ((skb->data[2]&0xfe)!=0x5e)){
			//bypass non-multicast pkt
			return 0;
		}		
		//insert VLANID for multicast packets
	//	printk("%x\n",*(unsigned int *)(skb->data+IP_HDR_DES_OFFSET));
		multicast_addr=READWD(skb->data+IP_HDR_DES_OFFSET);
		if(!((multicast_addr& 0xffffff00) == 0xe0000000)){
		//	printk("multi-cast traffic!  %x\n",*(unsigned int *)(skb->data+IP_HDR_DES_OFFSET));
			for(i=0;i<MAX_IGMP_SNOOPING_GROUP;i++){
				if(multicast_addr==snooping_table[i].multicast_addr){
					if(snooping_table[i].member_wlan==0)
						return 1;
					else
						return 0;
				}
			}
			
			if(skb->from_dev != NULL){
				if((skb->from_dev->priv_flags==IFF_DOMAIN_WAN) && DropUnknownMulticast)
					return 1;
			}
			else
				return 0;
		}
		return 0;
#else
	return 0;
#endif
}

#ifdef CONFIG_EXT_SWITCH
int check_IGMP_report(struct sk_buff *skb){
#define IP_TYPE_OFFSET				0x0c
#define IP_TYPE						0x0800
#define IP_HDLEN_OFFSET			0x0e
#define IGMP_PROTOCOL_OFFSET		0x17
#define IGMP_PROTOCOL				0x02
#define IGMP_TYPE_REPORT_JOIN		0x16
#define IGMPV3_TYPE_REPORT_JOIN	0x22
#define IGMP_TYPE_REPORT_LEAVE	0x17
#define MULTICAST_ADDR_REPORT_OFFSET	0x04

		int igmp_offset;
		unsigned short tmp_ip_type;
		//struct sk_buff *skb_query;
		//unsigned char skb_query_data[2048];
		
		//check if incoming frame is multicast packet
		if((skb->data[0]!=0x01) || (skb->data[1]!=0x0) || ((skb->data[2]&0xfe)!=0x5e)){
			//bypass non-multicast pkt
			return 0;
		}
		tmp_ip_type=READHWD(skb->data+IP_TYPE_OFFSET);
		igmp_offset=((*(unsigned char *)(skb->data+IP_HDLEN_OFFSET))&0x0f)*4+14;
		if((tmp_ip_type==IP_TYPE) && 
			((*(unsigned char *)(skb->data+IGMP_PROTOCOL_OFFSET))==IGMP_PROTOCOL) && (*(unsigned char *)(skb->data+igmp_offset))==IGMP_TYPE_REPORT_JOIN){
			return 1;
		}			
		return 0;
}

//tylo, return 0 if multicast address is not learned
int IGMP_snoop_tx(struct sk_buff *skb, DMA_DESC  *txd)
{
#define IP_HDR_DES_OFFSET		0x1e
//unsigned char multicast_insert[16];
    int i;
    unsigned int multicast_addr;	

    		//check if incoming frame is multicast packet
		if((skb->data[0]!=0x01) || (skb->data[1]!=0x0) || ((skb->data[2]&0xfe)!=0x5e)){
			//bypass non-multicast pkt
			return 1;
		}
		//with vlan tag general query
		if(skb->data[12]==0x81 && skb->data[13]==0x00){
		    multicast_addr=READWD(skb->data+IP_HDR_DES_OFFSET+4);
		}
		else multicast_addr=READWD(skb->data+IP_HDR_DES_OFFSET);
		//insert VLANID for multicast packets
		//printk("%x\n",*(unsigned int *)(skb->data+IP_HDR_DES_OFFSET));

		if(!((multicast_addr & 0xffffff00) == 0xe0000000)){
			//printk("multi-cast traffic!  %x\n",*(unsigned int *)(skb->data+IP_HDR_DES_OFFSET));
			for(i=0;i<MAX_IGMP_SNOOPING_GROUP;i++){
				if(multicast_addr==snooping_table[i].multicast_addr){
					u32	txOpts2;
					// set vlan id
					txOpts2 = VTAG2DESC(snooping_table[i].VLANID);
					txOpts2 |= 0x10000;
					txd->opts2 = txOpts2;
					/*memcpy(multicast_insert,skb->data,12);
					multicast_insert[12]=0x81; multicast_insert[13]=0x00;
					multicast_insert[14]=(snooping_table[i].VLANID >> 8 & 0x0f);
					multicast_insert[15]=(snooping_table[i].VLANID & 0xff);
					skb_push(skb,4);
					memcpy(skb->data, multicast_insert,16);

					//update txaddr and packet len
					txd->addr-=4;
					len+=4;*/
					return 1;
				}
			}
			return 0;
		}	
		return 1;
}


#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_NET
__NOMIPS16
#endif
int handle_IGMP_Snp(struct sk_buff *skb)
{
	//handle IGMP Query packets
	if (0==handle_IGMP_query(skb)){
		dev_kfree_skb(skb);
		return 0;  //an IGMP query and already processed.
	}
	if(check_IGMP_report(skb)==1){ //drop IGMP report from WLAN
		dev_kfree_skb(skb);
		return 0;
	}
	return 1;
}
#ifdef CONFIG_IGMP_FORBID

int  drop_IGMP(struct sk_buff *skb)
{
#define IP_TYPE_OFFSET				0x0c
#define IP_TYPE						0x0800
#define IGMP_PROTOCOL_OFFSET		0x17
#define IGMP_PROTOCOL				0x02
#define IGMP_TYPE_QUERY			       0x11
   
    //check if incoming frame is multicast packet
	if((skb->data[0]!=0x01) || (skb->data[1]!=0x0) || ((skb->data[2]&0xfe)!=0x5e)){
			return 1;
	}
	
        if((READHWD(skb->data+IP_TYPE_OFFSET))==IP_TYPE && ((*(unsigned char *)(skb->data+IGMP_PROTOCOL_OFFSET))==IGMP_PROTOCOL)) {
               dev_kfree_skb(skb);
                return 0;
	}
			
        return 1;
      
}
#endif

#endif

#ifdef CONFIG_PORT_MIRROR
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_NET
__NOMIPS16
#endif
void nic_tx_mirror (struct sk_buff *skb)
{
    struct net_device *dev = eth_net_dev;
    struct re_private *cp = dev->priv;
	unsigned entry;
	u32 eor;

	if (eth_close == 1)
	{
		dev_kfree_skb(skb);
		return;
	}

	spin_lock_irq(&cp->lock);
	re8670_tx(cp);

	if (TX_HQBUFFS_AVAIL(cp) <= 1)
	{
		spin_unlock_irq(&cp->lock);
		dev_kfree_skb(skb);
		return;
	}

	if ((skb->len>1518)) // with vlan tag
	{
		printk("error tx len = %d \n",skb->len);
		spin_unlock_irq(&cp->lock);
		dev_kfree_skb(skb);
		return;
	}
		
	entry = cp->tx_hqhead;
	eor = (entry == (RE8670_TX_RING_SIZE - 1)) ? RingEnd : 0;

	if (1) {
		DMA_DESC  *txd = &cp->tx_hqring[entry];
#ifdef CONFIG_RTL8672
		dma_cache_wback_inv((unsigned long)skb->data, skb->len);
#endif
		txd->addr = (u32)skb->data|UNCACHE_MASK;
		cp->tx_skb[entry].skb = skb;

		txd->opts1 = (eor | skb->len | DescOwn | FirstFrag |
			LastFrag | TxCRC);	
		wmb();
		
		entry = NEXT_TX(entry);
	}
	
	cp->tx_hqhead = entry;
	spin_unlock_irq(&cp->lock);

	RTL_W32(IO_CMD,iocmd_reg | TX_POLL);
	dev->trans_start = jiffies;
    
	return;
}
#endif

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_NET
__NOMIPS16
#endif
__IRAM int re8670_start_xmit (struct sk_buff *skb, struct net_device *dev)
{
	struct re_private *cp = dev->priv;
	unsigned entry;
	u32 eor;
#if 0
#ifdef CONFIG_EXT_SWITCH
	u32 txOpts2;
#endif
#endif
	     unsigned long flags;
#ifdef CONFIG_RTL867X_NETLOG
	     //unsigned long flags;
	    
	     local_irq_save(flags);
	      
	     re8670_tx( eth_net_dev->priv);
	        
	     local_irq_restore(flags);
//#endif
#else
	// Kaohj
	if (eth_poll) {
	     local_irq_save(flags);
	     re8670_tx( eth_net_dev->priv);
	     local_irq_restore(flags);
	}
#endif
		  
	//protect eth tx while reboot
#if 0
#ifdef FAST_ROUTE_Test
	// Suppose it is LLCHdr802_3, we remove it before trasmitting.
	if(skb->pkt_type == PACKET_FASTROUTE && skb->data[0]==0xaa){
		skb_pull(skb, 10);
	}
#endif
#endif
	if(eth_close == 1){
#ifdef ZTE_FAST_BRIDGE
		local_dev_kfree_skb(skb);
#else
		dev_kfree_skb(skb);
#endif
		cp->net_stats.tx_dropped++;
		return 0;
	}
//scout('{');


#ifdef CONFIG_EXT_SWITCH
#ifdef CONFIG_IGMP_FORBID
if(enable_IGMP_FORBID)
{
      
	if(0==drop_IGMP(skb)) {
		cp->net_stats.tx_dropped++;
		return 0;
	}
}
#endif
  if(enable_IGMP_SNP)
  {
      if(0==handle_IGMP_Snp(skb)) {
          cp->net_stats.tx_dropped++;
	  return 0;
      }
  }
 
#endif
	// JONAH
	cp->cp_stats.tx_cnt++;
	
#if CP_VLAN_TAG_USED
	u32 vlan_tag = 0;
#endif

	spin_lock_irq(&cp->lock);
	if (TX_HQBUFFS_AVAIL(cp) <= 1/*(skb_shinfo(skb)->nr_frags + 1)*/) {
#ifdef ZTE_FAST_BRIDGE
		local_netif_stop_queue(dev);
#else
		netif_stop_queue(dev);
#endif
		spin_unlock_irq(&cp->lock);
	/* This is a hard error, log it. */
//shlee         printk(KERN_ERR PFX "%s: BUG! Tx Ring full when queue awake!\n", dev->name);
                //printk("%s: BUG! Tx Ring full when queue awake!\n", __func__, dev->name);     //shlee 8672
                //re8670_tx(cp);        //shlee 8672 just test
#ifdef ZTE_FAST_BRIDGE
		local_dev_kfree_skb(skb);
#else
		dev_kfree_skb(skb);
#endif
                cp->net_stats.tx_dropped++;
                return 0;

	}

#if CP_VLAN_TAG_USED
	if (cp->vlgrp && vlan_tx_tag_present(skb))
		vlan_tag = TxVlanTag | vlan_tx_tag_get(skb);
#endif
	if ( (skb->len>1518) ) // with vlan tag
	{
		printk("error tx len = %d \n",skb->len);
		spin_unlock_irq(&cp->lock);
#ifdef ZTE_FAST_BRIDGE
		local_dev_kfree_skb(skb);
#else
		dev_kfree_skb(skb);
#endif
		cp->net_stats.tx_dropped++;
		return 0;
	}

#if 0		
#ifdef CONFIG_EXT_SWITCH
	txOpts2 = *(unsigned long *)&skb->cb[40];
	if (!(txOpts2 & 0x10000) || !dev->vlan)
		txOpts2 = 0;
#endif
#endif
		
	entry = cp->tx_hqhead;
	eor = (entry == (RE8670_TX_RING_SIZE - 1)) ? RingEnd : 0;
	if (1/*skb_shinfo(skb)->nr_frags == 0*/) {
		DMA_DESC  *txd = &cp->tx_hqring[entry];
		//u32 mapping, len;

		//len = ETH_ZLEN < skb->len ? skb->len : ETH_ZLEN;
		//eor = (entry == (RE8670_TX_RING_SIZE - 1)) ? RingEnd : 0;
		CP_VLAN_TX_TAG(txd, vlan_tag);
#if 0
#ifdef CONFIG_EXT_SWITCH
		if (txOpts2) {
			// add vlan tag
			txd->opts2 = txOpts2;
		}else
#endif
#endif
		// Kaohj
		#if 0 // deprecated; vlan tag will be processed in dev.c
		if((skb->vlan_passthrough & 0x81000000) == 0x81000000){ //vlan passthrough
				u32	tx_opts;
				// set vlan id
				tx_opts = VTAG2DESC(skb->vlan_passthrough & 0x0fff);
				tx_opts |= 0x10000;
				txd->opts2 = tx_opts;
		}
		#else
		if (enable_vlan_grouping) { // vlan tx
			u32	tx_opts;
			if (*((unsigned short *)&(skb->data[12])) != 0x8100 && skb->pvid) {
				// set vlan id
				tx_opts = VTAG2DESC(rtl8305_info.vlan[skb->pvid].vid & 0x0fff);
				tx_opts |= 0x10000;
				txd->opts2 = tx_opts;
			}
		}
		#endif
		//mapping = (u32)skb->data|UNCACHE_MASK;
		txd->addr = (u32)skb->data|UNCACHE_MASK;//(mapping);
		//wmb();
#ifdef CONFIG_EXT_SWITCH
		// Port-Mapping
        int should_deliver=2;
		if (skb->vlan_member!=0) {
			int k;
			should_deliver = 0;
			for (k=0; k<SW_PORT_NUM; k++) {
				//printk("%d: mbr=%x\n", k, rtl8305_info.vlan[k].member);
				if (skb->vlan_member == rtl8305_info.vlan[k].member) {
					u32	tx_opts;
					// set vlan id
					tx_opts = VTAG2DESC(rtl8305_info.vlan[k].vid);
					tx_opts |= 0x10000;
					txd->opts2 = tx_opts;
					//printk("k=%d, vid=%d\n", k, rtl8305_info.vlan[k].vid);
					should_deliver = 1;
					break;
				}
			}
			//printk("member=%x\n", skb->vlan_member);
			if (!should_deliver) {
				spin_unlock_irq(&cp->lock);
				dev_kfree_skb(skb);
				cp->net_stats.tx_dropped++;
				return 0;
			}
				
		}
		if(enable_IGMP_SNP){
			if(DropUnknownMulticast&&(should_deliver==2)&&(IGMP_snoop_tx(skb, txd)==0)){
				spin_unlock_irq(&cp->lock);
				dev_kfree_skb(skb);
				cp->net_stats.tx_dropped++;
				return 0;
			}
		}
#endif
#ifdef CONFIG_RTL8672
		// Kaohj --- invalidate DCache before NIC DMA
		dma_cache_wback_inv((unsigned long)skb->data, skb->len);
#endif

		cp->tx_skb[entry].skb = skb;
		//cp->tx_skb[entry].mapping = mapping;
		//cp->tx_skb[entry].frag = 0;

		txd->opts1 = (eor | /*len*/skb->len | DescOwn | FirstFrag |
			LastFrag | TxCRC);
#ifdef CONFIG_RTL8672
		dma_cache_wback_inv(txd, 16);
#endif
		wmb();
		entry = NEXT_TX(entry);
	}/* else {
        re8670_mFrag_xmit(skb, cp, &entry
#ifdef CONFIG_EXT_SWITCH
        , txOpts2
#endif        
        );
	}*/
	cp->tx_hqhead = entry;
#if 0
	if (netif_msg_tx_queued(cp))
		printk(KERN_DEBUG "%s: tx queued, slot %d, skblen %d\n",
		       dev->name, entry, skb->len);
	if (TX_HQBUFFS_AVAIL(cp) <= (MAX_SKB_FRAGS + 1))
#ifdef ZTE_FAST_BRIDGE
		local_netif_stop_queue(dev);
#else
		netif_stop_queue(dev);
#endif
#endif
	spin_unlock_irq(&cp->lock);

#if defined(CONFIG_CPU_RLX4181)||defined(CONFIG_CPU_RLX5181)
	//dcache write back, to make memory consist with cache before DMA
	dma_cache_wback_inv((unsigned long)skb->data, skb->len);
#endif
	RTL_W32(IO_CMD, iocmd_reg | TX_POLL);
	//printk("-------------------------------> iocmd_reg: 0x%x\n", iocmd_reg);
	dev->trans_start = jiffies;
//scout('}');		

	return 0;
}

/* Set or clear the multicast filter for this adaptor.
   This routine is not state sensitive and need not be SMP locked. */

static void __re8670_set_rx_mode (struct net_device *dev)
{
	/*struct re_private *cp = dev->priv;*/
	u32 mc_filter[2];	/* Multicast hash filter */
	int i, rx_mode;
	/*u32 tmp;*/


	// Kao
	//printk("%s: %s %d Still Unimplemented !!!!!!!\n",__FILE__,__FUNCTION__,__LINE__);
	//return ;
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
	// Kao, 2004/01/07
	RTL_W32(MAR0, mc_filter[0]);
	RTL_W32(MAR4, mc_filter[1]);
	RTL_W32(RCR, rx_mode);
}

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_NET
__NOMIPS16
#endif
static void re8670_set_rx_mode (struct net_device *dev)
{
	unsigned long flags;
	struct re_private *cp = dev->priv;
	// Kaohj
#ifdef CONFIG_EXT_SWITCH
	if (dev->base_addr == 0) {
		// not the root device
		return;
	}
#endif

	spin_lock_irqsave (&cp->lock, flags);
	__re8670_set_rx_mode(dev);
	spin_unlock_irqrestore (&cp->lock, flags);
}

// Kao, 2004/01/07
static int (*my_eth_mac_addr)(struct net_device *, void *);
static int re8670_set_mac_addr(struct net_device *dev, void *addr)
{
	u32 *hwaddr;
	int err;
#ifdef CONFIG_EXT_SWITCH
	int i;
	
	if (dev->base_addr == 0) {
		// not the root device
		printk("re_setmac, not root\n");
		return 0;
	}
	
	for (i = 0; i < SW_PORT_NUM; i++)
		my_eth_mac_addr(dev_sw[i], addr);
#endif
	err = my_eth_mac_addr(dev, addr);
	if (!err)
	{
		hwaddr = (u32 *)dev->dev_addr;
		RTL_W32(IDR0, *hwaddr);
		hwaddr = (u32 *)(dev->dev_addr+4);
		RTL_W32(IDR4, *hwaddr);
	}
	
	return err;
}

static void __re8670_get_stats(struct re_private *cp)
{
	/* XXX implement */
}

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_NET
__NOMIPS16
#endif
static struct net_device_stats *re8670_get_stats(struct net_device *dev)
{
	struct re_private *cp = dev->priv;

	/* The chip only need report frame silently dropped. */
	spin_lock_irq(&cp->lock);
 	if (netif_running(dev) && netif_device_present(dev))
 		__re8670_get_stats(cp);
	spin_unlock_irq(&cp->lock);

	return &cp->net_stats;
}

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_NET
__NOMIPS16
#endif
static void re8670_stop_hw (struct re_private *cp)
{
	RTL_W32(IO_CMD,0); /* timer  rx int 1 packets*/
	RTL_W16(IMR, 0);
	RTL_W16(ISR, 0xffff);
	synchronize_irq();
	udelay(10);

	cp->rx_tail = 0;
	cp->tx_hqhead = cp->tx_hqtail = 0;
}

static void re8670_reset_hw (struct re_private *cp)
{
	unsigned work = 1000;

   	RTL_W8(CMD,0x1);	 /* Reset */	
	while (work--) {
		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(10);
		if (!(RTL_R8(CMD) & 0x1))
			return;
	}
	
   	RTL_W8(CMD,0x1);	 /* Reset */	
   	work = 1000;
	while (work--) {
		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(10);
		if (!(RTL_R8(CMD) & 0x1))
			return;
	}


	// Kaohj -- should not be here
   	//RTL_W8(CMD,0x2);	 /* checksum */	

	printk(KERN_ERR "%s: hardware reset timeout\n", cp->dev->name);
}

static inline void re8670_start_hw (struct re_private *cp)
{
	RTL_W32(IO_CMD,iocmd_reg); /* timer  rx int 1 packets*/
#if 0
	cpw8(Cmd, RxOn | TxOn);
	cpw16(CpCmd, PCIMulRW | RxChkSum | CpRxOn | CpTxOn);
#endif
}

static void re8670_init_hw (struct re_private *cp)
{
	struct net_device *dev = cp->dev;
	u8 status;
	// Kao
	u32 *hwaddr, sicr;
	unsigned short regValue;
	unsigned int value;
#ifdef CONFIG_RE8306
	unsigned short tmpreg;
	
	if (virt2phy[SWITCH_VPORT_TO_867X] == 6) {
		miiar_read(6,22,&tmpreg);
		miiar_write(6,22,tmpreg|0x8000);
	}
#endif

	re8670_reset_hw(cp);
	
   	RTL_W8(CMD,0x2);	 /* checksum */	
	RTL_W16(ISR,0xffff);
	RTL_W16(IMR,re8670_intr_mask);   	
	RTL_W32(RxFDP,(u32)(cp->rx_ring));	
#if DEBUG
	writecheck = RTL_R32(RxFDP);	
	if (writecheck != ( (u32)cp->rx_ring ))
		for (;;);

#endif	
	RTL_W16(RxCDO,0);
	RTL_W32(TxFDP1,(u32)(cp->tx_hqring));	
#if DEBUG
	writecheck = RTL_R32(TxFDP1);	
	if (writecheck != ( (u32)cp->tx_hqring))
		for (;;);
#endif
	RTL_W16(TxCDO1,0);	
	RTL_W32(TxFDP2,(u32)cp->tx_lqring);
	RTL_W16(TxCDO2,0);
	// Kao
	//RTL_W32(RCR,AcceptAll);		
	RTL_W32(TCR,(u32)(0x0C00));
#ifdef CONFIG_RTL8672
	RTL_W8(EthrntRxCPU_Des_Wrap,TH_ON_VAL);	
	RTL_W8(Rx_Pse_Des_Thres,TH_OFF_VAL);	
	RTL_W8(EthrntRxCPU_Des_Num,RE8670_RX_RING_SIZE-1);	
	RTL_W8(RxRingSize,RE8670_RX_RING_SIZE-1);		
	status = RTL_R8(MSR);
	//status = status | (TXFCE|FORCE_TX|RXFCE);	// enable tx flowctrl
	RTL_W8(MSR,status);	
#else
	RTL_W8(Rx_Pse_Des_Thres,THVAL);
	RTL_W8(EthrntRxCPU_Des_Num,RE8670_RX_RING_SIZE-1);
	RTL_W8(RxRingSize,RINGSIZE);	
	status = RTL_R8(MSR);
	// Kaohj --- enable Ethernet RX Flow Control
	status = status & ~(TXFCE | RXFCE | FORCE_TX);
	status = status | RXFCE;
	RTL_W8(MSR,status);
#endif
	// Kao, set hw ID for physical match
	hwaddr = (u32 *)cp->dev->dev_addr;
	RTL_W32(IDR0, *hwaddr);
	hwaddr = (u32 *)(cp->dev->dev_addr+4);
	RTL_W32(IDR4, *hwaddr);
	// check if use internal phy
#ifdef CONFIG_RTL8671
        sicr = (*(volatile u32*)(0xB9C04000));	//tylo, 0xbd800000));
	if (!(sicr & 0x00000600))  //7/1/05' hrchen, for 8671 MII spec
#else
	sicr = (*(volatile u32*)(0xbd800000));
	if (!(sicr & 0x10))
#endif
	{
		// use internal PHY
		// Jenny, for internal PHY ethernet LED; set value 101=>Link+Act: On=Link, Off=No link, Flash=Tx or Rx activity
#ifndef CONFIG_RTL8672  //cathy
		miiar_read(1, 31, &regValue);
		regValue = ((regValue & 0xfff8) | 5);
		miiar_write(1, 31, regValue);
#else
	#ifndef CONFIG_RE8306 // andrew, configure Internal phy LED 
	RTL_W32(LEDCR, 0x47777);
	#endif
#endif  //cathy
	}
	//Set tx_queue_len=0 at start for memory save
	//if need increase by userspase
	dev->tx_queue_len = 0;
	re8670_start_hw(cp);
	__re8670_set_rx_mode(dev);

//NIC and Switch are forced to 100M FD if using external switch
//NIC forced to 100M FD
#ifndef CONFIG_CPU_PORT
#define CONFIG_CPU_PORT 4
#endif
#ifdef CONFIG_EXT_SWITCH
if(IS_6166 || IS_RLE0315){
	//NIC and Switch are forced to 100M FD if using external switch
        RTL_W32(MIIAR, 0x841f0000);
        mdelay(5);
        RTL_W32(MIIAR, 0x84002100);
        mdelay(5);
        //Switch forced to 100M FD
        select_page(0);
#ifdef AUTO_SEARCH_CPUPORT
        miiar_write(virt2phy[SW_PORT_NUM], 0, 0x2100);
#else
        miiar_write(CONFIG_CPU_PORT,0,0x2100);
#endif
        select_page(0);
} else {   //------------------------6028 6085 -------------------------//
        RTL_W32(MIIAR, 0x841f0000);
        mdelay(5);
        RTL_W32(MIIAR, 0x840405e1);
        mdelay(5);
        select_page(0);
#ifdef AUTO_SEARCH_CPUPORT
        miiar_write(virt2phy[SW_PORT_NUM], 4, 0x05e1);
        miiar_write(virt2phy[SW_PORT_NUM], 0, 0x1200);
#else
        miiar_write(CONFIG_CPU_PORT,4,0x05e1);
        miiar_write(CONFIG_CPU_PORT,0,0x1200);
#endif
        select_page(0);
        mdelay(5);
        RTL_W32(MIIAR, 0x84001200);
}
#endif
}

static int re8670_refill_rx (struct re_private *cp)
{
	unsigned i;

	for (i = 0; i < RE8670_RX_RING_SIZE; i++) {
		struct sk_buff *skb;

		skb = re8670_getAlloc();
		if (!skb)
			goto err_out;
#ifdef CONFIG_RTL8672
		// Kaohj --- invalidate DCache for uncachable usage
		//ql_xu
		dma_cache_wback_inv((unsigned long)skb->data, SKB_BUF_SIZE);
#endif

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
		if (i == (RE8670_RX_RING_SIZE - 1))
			cp->rx_ring[i].opts1 = (DescOwn | RingEnd | cp->rx_buf_sz);
		else
			cp->rx_ring[i].opts1 =(DescOwn | cp->rx_buf_sz);
		cp->rx_ring[i].opts2 = 0;
	}

	return 0;

err_out:
	re8670_clean_rings(cp);
	return -ENOMEM;
}

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_NET
__NOMIPS16
#endif
static void re8670_tx_timeout (struct net_device *dev)
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
static int re8670_init_rings (struct re_private *cp)
{
	memset(cp->tx_hqring, 0, sizeof(DMA_DESC) * RE8670_TX_RING_SIZE);
	memset(cp->rx_ring, 0, sizeof(DMA_DESC) * RE8670_RX_RING_SIZE);
	cp->rx_tail = 0;
	cp->tx_hqhead = cp->tx_hqtail = 0;

	return re8670_refill_rx (cp);
}

static int re8670_alloc_rings (struct re_private *cp)
{
	/*cp->rx_ring = pci_alloc_consistent(cp->pdev, CP_RING_BYTES, &cp->ring_dma);*/
	void*	pBuf;
	
	
	
	pBuf = kmalloc(RE8670_RXRING_BYTES,GFP_KERNEL);
	if (!pBuf)
		goto ErrMem;
#ifdef CONFIG_RTL8672
	// Kaohj -- invalidate DCache for uncachable usage
	dma_cache_wback_inv(pBuf, RE8670_RXRING_BYTES);
#endif
	cp->rxdesc_buf = pBuf;
	memset(pBuf, 0, RE8670_RXRING_BYTES);
	
	pBuf = (void*)( (u32)(pBuf + DESC_ALIGN-1) &  ~(DESC_ALIGN -1) ) ;
	cp->rx_ring = (DMA_DESC*)((u32)(pBuf) | UNCACHE_MASK);


	pBuf= kmalloc(RE8670_TXRING_BYTES, GFP_KERNEL);
	if (!pBuf)
		goto ErrMem;
#ifdef CONFIG_RTL8672
	// Kaohj -- invalidate DCache for uncachable usage
	dma_cache_wback_inv(pBuf, RE8670_TXRING_BYTES);
#endif
	cp->txdesc_buf = pBuf;
	memset(pBuf, 0, RE8670_TXRING_BYTES);
	pBuf = (void*)( (u32)(pBuf + DESC_ALIGN-1) &  ~(DESC_ALIGN -1) ) ;
	cp->tx_hqring = (DMA_DESC*)((u32)(pBuf) | UNCACHE_MASK);

	return re8670_init_rings(cp);

ErrMem:
	if (cp->rxdesc_buf)
		kfree(cp->rxdesc_buf);
	if (cp->txdesc_buf)
		kfree(cp->txdesc_buf);
	return -ENOMEM;

	
}

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_NET
__NOMIPS16
#endif
static void re8670_clean_rings (struct re_private *cp)
{
	unsigned i;


	for (i = 0; i < RE8670_RX_RING_SIZE; i++) {
		if (cp->rx_skb[i].skb) {
			dev_kfree_skb(cp->rx_skb[i].skb);
		}
	}

	for (i = 0; i < RE8670_TX_RING_SIZE; i++) {
		if (cp->tx_skb[i].skb) {
			struct sk_buff *skb = cp->tx_skb[i].skb;
			dev_kfree_skb(skb);
			cp->net_stats.tx_dropped++;
		}
	}

	memset(&cp->rx_skb, 0, sizeof(struct ring_info) * RE8670_RX_RING_SIZE);
	memset(&cp->tx_skb, 0, sizeof(struct ring_info) * RE8670_TX_RING_SIZE);
}

static void re8670_free_rings (struct re_private *cp)
{
	re8670_clean_rings(cp);
	/*pci_free_consistent(cp->pdev, CP_RING_BYTES, cp->rx_ring, cp->ring_dma);*/
	kfree(cp->rxdesc_buf);
	kfree(cp->txdesc_buf);
	
	cp->rx_ring = NULL;
	cp->tx_hqring = NULL;
}

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_NET
__NOMIPS16
#endif
static int re8670_open (struct net_device *dev)
{
	struct re_private *cp = dev->priv;
	int rc;

	// Kaohj
#ifdef CONFIG_EXT_SWITCH
	if (dev->base_addr == 0) {
		netif_start_queue(dev);
		return 0;
	}
#endif
	if (netif_msg_ifup(cp))
		printk(KERN_DEBUG "%s: enabling interface\n", dev->name);

	re8670_set_rxbufsize(cp);	/* set new rx buf size */
	rc = re8670_alloc_rings(cp);
	if (rc)
		return rc;

	re8670_init_hw(cp);
	// Kaohj
#ifdef CONFIG_EXT_SWITCH
	if (enable_virtual_port) {
		RTL_W8(CMD,RTL_R8(CMD)|0x04);	// VLAN de-tagging enabled
	}
#endif

//	rc = request_irq(dev->irq, re8670_interrupt, 0, dev->name, dev);
	rc = request_irq(dev->irq, re8670_interrupt, SA_INTERRUPT, dev->name, dev);
	if (rc)
		goto err_out_hw;

	netif_start_queue(dev);
	eth_close=0;

	return 0;

err_out_hw:
	re8670_stop_hw(cp);
	re8670_free_rings(cp);
	return rc;
}

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_NET
__NOMIPS16
#endif
static int re8670_close (struct net_device *dev)
{
	struct re_private *cp = dev->priv;

	// Kaohj
#ifdef CONFIG_EXT_SWITCH
	if (dev->base_addr == 0) {
		netif_stop_queue(dev);
		return 0;
	}
#endif
	eth_close=1;
	//if (eth_rx_tasklets) tasklet_disable(eth_rx_tasklets);
	if (netif_msg_ifdown(cp))
		printk(KERN_DEBUG "%s: disabling interface\n", dev->name);

	netif_stop_queue(dev);
	re8670_stop_hw(cp);
	free_irq(dev->irq, dev);
	re8670_free_rings(cp);
	return 0;
}

#if 0
static int re8670_change_mtu(struct net_device *dev, int new_mtu)
{
	struct re_private *cp = dev->priv;
	int rc;

	/* check for invalid MTU, according to hardware limits */
	if (new_mtu < CP_MIN_MTU || new_mtu > CP_MAX_MTU)
		return -EINVAL;

	/* if network interface not up, no need for complexity */
	if (!netif_running(dev)) {
		dev->mtu = new_mtu;
		re8670_set_rxbufsize(cp);	/* set new rx buf size */
		return 0;
	}

	spin_lock_irq(&cp->lock);

	re8670_stop_hw(cp);			/* stop h/w and free rings */
	re8670_clean_rings(cp);

	dev->mtu = new_mtu;
	re8670_set_rxbufsize(cp);		/* set new rx buf size */

	rc = re8670_init_rings(cp);		/* realloc and restart h/w */
	re8670_start_hw(cp);
	spin_unlock_irq(&cp->lock);

	return rc;
}

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
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_NET
__NOMIPS16
#endif
static int re8670_ethtool_ioctl (struct re_private *cp, void *useraddr)
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

// Kao added
unsigned short MII_RD(unsigned char PHY_ADDR, unsigned char REG_ADDR)
{
	volatile unsigned int value; // LINK_MODE_SUPPORT
	u32 sicr;
	unsigned short data;

#ifdef CONFIG_RTL8671
	sicr = (*(volatile u32*)(0xB9C04000));	//tylo, 0xbd800000));
	if (sicr & 0x00000600)  //7/1/05' hrchen, for 8671 MII spec
#else
	sicr = (*(volatile u32*)(0xbd800000));
	if (sicr & 0x10)
#endif
	{	// external phy
#ifdef CONFIG_EXT_SWITCH
		miiar_read(PHY_ADDR, REG_ADDR,&data);
#else
		// fix me
		data=0;
#endif
		return data;
	}
	else
	{
		//printk("MII_RD:%d,%d...", PHY_ADDR, REG_ADDR); // LINK_MODE_SUPPORT
		RTL_W32(MIIAR, FLAG_READ | (PHY_ADDR<<MII_PHY_ADDR_SHIFT) | (REG_ADDR <<MII_REG_ADDR_SHIFT));
		
		while (!(RTL_R32(MIIAR) & 0x80000000));
		
		value = RTL_R32(MIIAR);
		data = (unsigned short)(value & 0xffff);
		//printk("MII_RD: got %x,%x\n", value, data); // LINK_MODE_SUPPORT
	}
	
	return data;
}

// LINK_MODE_SUPPORT --->
static int mdio_read(struct net_device *dev, int phy_id, int location)
{
	struct re_private *cp = dev->priv;
	int ret; 
	ret = MII_RD(phy_id, location);
	//printk("mii_r: %d, %d = %xh\n", phy_id, location, ret);
	return ret;
}


static void mdio_write(struct net_device *dev, int phy_id, int location,
		       int value)
{
	struct re_private *cp = dev->priv;

	//printk("mii_w: %d, %d, %xh\n", phy_id, location, value); 
	internal_miiar_write(phy_id, location, value);
}
// <-- LINK_MODE_SUPPORT

// Mason Yu. For Set IPQOS
#define		SETIPQOS		0x01

/*
 * Structure used in SIOCSIPQOS request.
 */

struct ifIpQos
{
	int	cmd;
	char	enable;	
};

static void re8670_ipqos_ioctl(struct ifIpQos *req)
{
	int i;
	unsigned short regValue;
	
	switch(req->cmd) {
	
	case SETIPQOS:
		if (req->enable) {
			RTL_W8(CMD,RTL_R8(CMD)|0x04);	// VLAN de-tagging enabled
			enable_ipqos = 1;					
		}
		else {
			enable_ipqos = 0;
		}
		break;
	
	default: break;
	}
}
 


#ifdef CONFIG_EXT_SWITCH
#define  VLAN_ENABLE 0x01
#define  VLAN_SETINFO 0x02
#define  VLAN_SETPVIDX 0x03
#define  VLAN_SETTXTAG 0x04
#define  VLAN_DISABLE1PPRIORITY 0x05
#define	 VLAN_SETIGMPSNOOP	0x06
#define	 VLAN_SETPORTMAPPING	0x07
#define	 VLAN_SETIPQOS		0x08
#define	 VLAN_VIRTUAL_PORT	0x09
#define	 VLAN_SETVLANGROUPING	0x0a


#define  TAG_DCARE 0x03
#define  TAG_ADD  0x02
#define  TAG_REMOVE 0x01
#define  TAG_REPLACE 0x00
 
// Kaohj
/*
 * Structure used in SIOCSIFVLAN request.
 */
 
struct ifvlan
{
	int cmd;
	char enable;
	short vlanIdx;
	short vid;
	char	disable_priority;
	int member;
	int port;
	char txtag;
};

static void re8670_8305vlan_ioctl(struct ifvlan *req)
{
	int i;
	unsigned short regValue;
	
	switch(req->cmd) {
	case VLAN_ENABLE:
		// set into device
		for (i=0; i<SW_PORT_NUM; i++)
			dev_sw[i]->vlan = req->enable;
		//printk("enable: en=%d\n", req->enable);
		rtl8305_info.vlan_en = req->enable;
		rtl8305sc_setAsicVlanEnable(req->enable);
		break;
	case VLAN_SETINFO:
		//printk("info: vlanIdx=0x%x, vid=0x%x, mem=0x%x\n", req->vlanIdx, req->vid, req->member);
		rtl8305_info.vlan[req->vlanIdx].vid = req->vid;
		rtl8305_info.vlan[req->vlanIdx].member = bitmap_virt2phy(req->member);
		//printk("info: translate mbr to 0x%.08x\n", rtl8305_info.vlan[req->vlanIdx].member);
		rtl8305sc_setAsicVlan(req->vlanIdx,req->vid, rtl8305_info.vlan[req->vlanIdx].member);
		vc_ch_remapping(&(rtl8305_info.vlan[req->vlanIdx].member));
		break;
	case VLAN_SETPVIDX:
		// set vid into device
		if (req->port >=0 && req->port <= (SW_PORT_NUM-1)) { // to identify switch ports
			miiar_read(req->vlanIdx, 25, &regValue);
			dev_sw[req->port]->vid = (regValue & 0x0fff);
			//printk("pvidx: port=%d, vlanIdx=0x%x\n", req->port, req->vlanIdx);
			rtl8305_info.phy[virt2phy[req->port]].vlanIndex = req->vlanIdx;
			//printk("pidx: phy=%d, vlanidx=%d\n", virt2phy[req->port], req->vlanIdx);
			rtl8305sc_setAsicPortVlanIndex(virt2phy[req->port],req->vlanIdx);
		}
		else { // for vlan_groupping
			rtl8305_info.phy[SW_PORT_NUM+1+virt2phy[req->port-SW_PORT_NUM-1]].vlanIndex = req->vlanIdx;
		}
		break;
	case VLAN_SETTXTAG:
		// set into device
		// if it is for port 4 (867x NIC) ...
		if (req->port == SWITCH_VPORT_TO_867X) {
			/*
			if (req->txtag == TAG_REMOVE)
				RTL_W8(CMD,RTL_R8(CMD)|0x04);	// VLAN de-tagging enabled
			else if (req->txtag == TAG_DCARE)
				RTL_W8(CMD,RTL_R8(CMD)&0xfb);	// VLAN de-tagging disabled
			*/
		}
		else
			dev_sw[req->port]->egtag = req->txtag;
		//printk("txtag: port=%d, txtag=0x%x\n", req->port, req->txtag);
		rtl8305_info.phy[virt2phy[req->port]].egtag = req->txtag;
		rtl8305sc_setVLANTagInsertRemove(virt2phy[req->port],req->txtag);
		break;
	case VLAN_DISABLE1PPRIORITY:
		rtl8305sc_setVLANTag1pProirity(virt2phy[req->port], req->disable_priority);
		break;
	case VLAN_SETIGMPSNOOP:
		if (req->enable) {
			if (!IGMP_SNP_registered) {
				printk("***** Call IGMP_Snooping_config()\n");
				IGMP_Snooping_config();
			}
			enable_IGMP_SNP=1;
			enable_virtual_port=1;
			rtl8305_info.phy[virt2phy[SWITCH_VPORT_TO_867X]].egtag=TAG_REPLACE;
			for (i=0; i<SW_PORT_NUM; i++)
				rtl8305_info.phy[virt2phy[i]].egtag=TAG_REMOVE;
			set_8305(&rtl8305_info);
		}
		else
			enable_IGMP_SNP=0;
		break;
	case VLAN_SETPORTMAPPING:
		if (req->enable) {
			rtl8305_info.vlan_tag_aware=1;
			rtl8305_info.vlan_en=1;
			rtl8305sc_setAsicVlanTagAware(1); //enable tag aware
			//rtl8305sc_setAsicVlanEnable(1);	//VLAN enable
			enable_port_mapping = 1;
			for (i=0; i<SW_PORT_NUM; i++)
				rtl8305_info.phy[virt2phy[i]].egtag=TAG_REMOVE;
			set_8305(&rtl8305_info);
		}
		else {
			rtl8305_info.vlan_tag_aware=0;
			rtl8305_info.vlan_en=0;
			enable_port_mapping = 0;
		}
		break;
	case VLAN_SETVLANGROUPING:
		if (req->enable) {
			rtl8305_info.vlan_tag_aware=1;
			rtl8305_info.vlan_en=1;
			rtl8305sc_setAsicVlanTagAware(1); //enable tag aware
			enable_vlan_grouping = 1;
			for (i=0; i<SW_PORT_NUM; i++)
				rtl8305_info.phy[virt2phy[i]].egtag=TAG_REMOVE;
			set_8305(&rtl8305_info);
		}
		else {
			rtl8305_info.vlan_tag_aware=0;
			rtl8305_info.vlan_en=0;
			enable_vlan_grouping = 0;
			for (i=0; i<SW_PORT_NUM; i++)
				rtl8305_info.phy[virt2phy[i]].egtag=TAG_DCARE;
			set_8305(&rtl8305_info);
		}
		break;
	case VLAN_SETIPQOS:
		if (req->enable) {
			RTL_W8(CMD,RTL_R8(CMD)|0x04);	// VLAN de-tagging enabled
			enable_ipqos = 1;
		}
		else {
			enable_ipqos = 0;
		}
		break;
	case VLAN_VIRTUAL_PORT:
		if (req->enable) {
			RTL_W8(CMD,RTL_R8(CMD)|0x04);	// VLAN de-tagging enabled
			enable_virtual_port=1;
		}
		else {
			enable_virtual_port=0;
			rtl8305_info.phy[virt2phy[SWITCH_VPORT_TO_867X]].egtag = TAG_DCARE;	// don't care
			rtl8305sc_setVLANTagInsertRemove(virt2phy[SWITCH_VPORT_TO_867X],TAG_DCARE);
		}
		break;
 #ifdef CONFIG_IGMP_FORBID
	case  IGMP_FORBID:
		 if(req->enable)
		 	{
		 	   enable_IGMP_FORBID =0;
		 	}
		 else 
		 	{
			   enable_IGMP_FORBID =1;
		 	}
		 	
		 break;	
#endif
	default: break;
	}
}
#endif

//for ethctl
struct eth_arg{
	unsigned char cmd;
	unsigned int cmd2;
	unsigned int cmd3;
	unsigned int cmd4;
	unsigned int cmd5;
};
#define test_len 500
struct sk_buff testskb;
unsigned char testskbdata[test_len];
struct timer_list rxtest_timer;
unsigned char rxtest_running=0;
void show_rx(void){
	printk("ethernet rx count:%d\n",eth_rx_count);
	rxtest_running=0;
}

unsigned char led_control=0,mp_led_stop=0;
struct timer_list mpled_timer;
void led_flash(){
//cathy
#ifndef CONFIG_RTL8672
	unsigned char wlanreg;
	if(mp_led_stop==1)
		return;
	if(led_control==0){
		gpioClear(ADSL_LED);
		gpioClear(ADSL_ACT_LED);
		gpioClear(ALARM_LED);
		wlanreg=*(volatile unsigned char *)0xbd30005e;
		*(volatile unsigned char *)0xbd30005e= (wlanreg&(~(1<<4)));

		wlanreg=*(volatile unsigned char *)0xbd30005e;
		*(volatile unsigned char *)0xbd30005e= (wlanreg&(~(1<<5)));
		led_control=1;
	}
	else{
		gpioSet(ADSL_LED);
		gpioSet(ADSL_ACT_LED);
		gpioSet(ALARM_LED);
		wlanreg=*(volatile unsigned char *)0xbd30005e;
		*(volatile unsigned char *)0xbd30005e= (wlanreg|((1<<4)));

		wlanreg=*(volatile unsigned char *)0xbd30005e;
		*(volatile unsigned char *)0xbd30005e= (wlanreg|((1<<5)));
		led_control=0;
	}
	if(mp_led_stop==0){
		mpled_timer.expires=jiffies+HZ;
		mpled_timer.function=(void (*)(unsigned long))led_flash;
		add_timer(&mpled_timer);
	}
#endif //cathy
}
extern int AccessPOMLed;
void eth_ctl(struct eth_arg * arg){
	unsigned char cmd1,wlanreg;
	unsigned int cmd2,cmd3,i;
	unsigned short cmd4;
	unsigned int cmd5;
	struct skb_shared_info skbshare;
	
	cmd1=arg->cmd;
	cmd2=arg->cmd2;
	cmd3=arg->cmd3;
	cmd4=(unsigned short)arg->cmd4;
	cmd5 = arg->cmd5;
	


	switch(cmd1){
		case 1: //tx
			for(i=0;i<test_len;i++){
				testskbdata[i]=i%0xff;
			}
			skbshare.nr_frags=0;
			testskb.end=(unsigned char*)&skbshare;

			testskb.data=testskbdata;
			testskb.len=test_len;

			re8670_start_xmit(&testskb,  eth_net_dev);
			//mdelay(1);
			break;
		case 2://rx
			rxtest_timer.expires=jiffies+cmd2*HZ;
			rxtest_timer.function=(void (*)(unsigned long))show_rx;
			eth_rx_count=0;
			rxtest_running=1;
			add_timer(&rxtest_timer);
			break;
#ifdef CONFIG_EXT_SWITCH
		case 3://phy on
			rtl8305sc_setPower(cmd2, 1);
			break;

		case 4://phy off
			rtl8305sc_setPower(cmd2, 0);
			break;
#endif

		case 5://read phy
#ifdef CONFIG_EXT_SWITCH
			// Kaohj -- Select PHY Register page through configuring PHY 0 Register 16 [bit1 bit15]
			select_page(cmd4);
#endif
			miiar_read(cmd2,cmd3,&cmd4);
			printk("PHY%d.REG%d: 0x%04X\n", cmd2, cmd3, cmd4);
#ifdef CONFIG_EXT_SWITCH
			select_page(0);
#endif
			break;
			
		case 6://write phy
#ifdef CONFIG_EXT_SWITCH
			// Kaohj -- Select PHY Register page through configuring PHY 0 Register 16 [bit1 bit15]
			select_page(cmd5);
#endif
			miiar_write(cmd2,cmd3,cmd4);
			printk("wPHY%d.REG%d: 0x%04X\n", cmd2, cmd3, cmd4);
			miiar_read(cmd2,cmd3,&cmd4);
			printk("rPHY%d.REG%d: 0x%04X\n", cmd2, cmd3, cmd4);
#ifdef CONFIG_EXT_SWITCH
			select_page(0);
#endif
			break;
			
#ifdef CONFIG_EXT_SWITCH
//cathy
#ifndef CONFIG_RTL8672
		case 7://LED all on
			AccessPOMLed=0;
			//wlan led on
			*(volatile unsigned char *)0xbd300050=0xc0;//RTL_W8(_9346CR_, 0xc0); 

			wlanreg=*(volatile unsigned char *)0xbd300051;
			*(volatile unsigned char *)0xbd300051=((wlanreg&0xef)|0x10);

			wlanreg=*(volatile unsigned char *)0xbd30005e;
			*(volatile unsigned char *)0xbd30005e= (wlanreg&(~(1<<4)));

			wlanreg=*(volatile unsigned char *)0xbd30005e;
			*(volatile unsigned char *)0xbd30005e= (wlanreg&(~(1<<5)));
		    gpioConfig(ADSL_LED, GPIO_FUNC_OUTPUT);
		    gpioConfig(ADSL_ACT_LED, GPIO_FUNC_OUTPUT);
		    gpioConfig(ALARM_LED, GPIO_FUNC_OUTPUT);
		    gpioClear(ADSL_LED);
		    gpioClear(ADSL_ACT_LED);
		    gpioClear(ALARM_LED);
		    mp_led_stop=1;
		    led_control=1;
#endif //cathy
			break;
#endif
		case 8:
			//for wifi test
			//modify  RX_MIT
			if(cmd2==1) //wifi test
				iocmd_reg = 0x3c | 1 << 8 | RX_FIFO << 11 |  RX_TIMER << 13 | TX_MIT << 16 | TX_FIFO<<19;
			else
				iocmd_reg=CMD_CONFIG;
			printk("set iocmd_reg:%x\n",iocmd_reg);
			break;
		case 9:
//cathy
#ifndef CONFIG_RTL8672
			AccessPOMLed=0;
			*(volatile unsigned char *)0xbd300050=0xc0;//RTL_W8(_9346CR_, 0xc0); 

			wlanreg=*(volatile unsigned char *)0xbd300051;
			*(volatile unsigned char *)0xbd300051=((wlanreg&0xef)|0x10);
			gpioConfig(ADSL_LED, GPIO_FUNC_OUTPUT);
		    	gpioConfig(ADSL_ACT_LED, GPIO_FUNC_OUTPUT);
		    	gpioConfig(ALARM_LED, GPIO_FUNC_OUTPUT);

			mp_led_stop=0;
			mpled_timer.expires=jiffies+1*HZ;
			mpled_timer.function=(void (*)(unsigned long))led_flash;
			add_timer(&mpled_timer);
			break;
		case 10:  //GPIO test
		    	gpioConfig(cmd2, GPIO_FUNC_OUTPUT);
		    	if (cmd3 == 0)
		    		gpioClear(cmd2);
		    	else
		    		gpioSet(cmd2);
		    	break;
#endif
		// Added by Mason Yu for PPP LED		
		case 12:  //PPP on	
			g_ppp_up++;
#ifdef GPIO_LED_PPP
			gpio_LED_PPP(1);
#endif
			break;
		
		// Added by Mason Yu for PPP LED
		case 13:  //PPP off	
			g_ppp_up--;
			if (g_ppp_up<0)
				g_ppp_up = 0;
#ifdef GPIO_LED_PPP
			if (g_ppp_up == 0)
				gpio_LED_PPP(0);
#endif
			break;
			
		//7/13/06' hrchen, watchdog command
		case 14: {
            extern int update_watchdog_timer(void);
            extern void start_watchdog(int);
            extern void stop_watchdog(void);
            extern void get_watchdog_status(void);
            extern void set_watchdog_kick_time(int kick_sec);
            extern void set_watchdog_timeout_time(int kick_sec);
            switch (cmd2) {
            	case 1:  //update watchdog
            	  arg->cmd3=update_watchdog_timer();//for kick cycle time update in watchdog task
            	  break;
            	case 2:  //enable hw watchdog
            	  start_watchdog(cmd3);
            	  break;
            	case 3:  //disable hw watchdog
            	  stop_watchdog();
            	  break;
            	case 4:  //set watchdog kick time
            	  set_watchdog_kick_time(cmd3);
            	  break;
            	case 5:  //get watchdog status
            	  get_watchdog_status();
            	  break;
            	case 6:  //set watchdog timeout time
            	  set_watchdog_timeout_time(cmd3);
            	  break;
            };
            break;
        };
//star: for ZTE Router LED request
	  	case 15:
			g_internet_up = 1;
			break;

	  	case 16:
			g_internet_up = 0;
			break;

		case 21: // conntrack killall
			drop_all_conntrack();			
			break;
#if 0		
		case 22: // conntrack killtcpudp			
			drop_all_tcp_udp_conntrack();
			break;
		
		case 23: // conntrack killicmp			
			drop_all_icmp_conntrack();
			break;

		case 24: // conntrack killdns
			drop_all_dns_conntrack();
			break;
#endif
// Kaohj
#ifdef CONFIG_EXT_SWITCH
#ifdef CONFIG_RE8306_API
		case 26: // dump 8306 qos
			dump_8306_qos();
			break;
		case 27: // reset 8306 mib counters
			reset_8306_counter();
			break;
		case 28: // dump 8306 mib counters
			dump_8306_counter();
			break;
#endif
		case 29: // dump vlan info
			dump_vlan_info();
			break;
#endif	// of CONFIG_EXT_SWITCH
		default:
			printk("error cmd\n");
	}
}

static int re8670_ioctl (struct net_device *dev, struct ifreq *rq, int cmd)
{
	struct re_private *cp = dev->priv;
	int rc = 0;
	unsigned short *data = (unsigned short *)(&rq->ifr_data);
	u32 *data32 = (unsigned int *)(&rq->ifr_data);
	u32 sicr;
	u8 status;
	#ifdef CONFIG_EXT_SWITCH
	struct mii_ioctl_data *mii = (struct mii_data *)&rq->ifr_data;
	int port;
	#endif
	
	if (!netif_running(dev) && cmd!=SIOCETHTEST)
		return -EINVAL;

	switch (cmd) {
	case SIOCETHTOOL:
		return re8670_ethtool_ioctl(cp, (void *) rq->ifr_data);

	case SIOCGMIIPHY:
#ifdef CONFIG_RTL8672
#ifdef CONFIG_EXT_SWITCH
		if(1)
#else
		if(0)
#endif //CONFIG_EXT_SWITCH
#elif CONFIG_RTL8671
                sicr = (*(volatile u32*)(0xB9C04000));	//tylo, 0xbd800000));
		if (sicr & 0x00000600)  //7/1/05' hrchen, for 8671 MII spec
#else
		sicr = (*(volatile u32*)(0xbd800000));
		if (sicr & 0x10)
#endif
		{
			data32[1]=4;	// external phy address
//			printk("use MII\n");
#ifdef CONFIG_EXT_SWITCH
			for (port = 0; port < SW_PORT_NUM; port++) {
				if (dev_sw[port] == dev) {
					data32[1] = port;
					break;
				}
			}
#endif
		}
		else
		{
			data32[1]=1;	// internal phy address == 1
//			printk("internal phy\n");
		}
		
		break;
	case SIOCGMIIREG:
		//data[3] = MII_RD(data[0], data[1]);
		miiar_read(data[0], data[1], &data[3]);
//		printk("phyid=%d, miireg=%d\n", data[0], data[1]);
//		printk("data[3]=%X\n", data[3]);
		break;
	//tylo, for VLAN setting
	#ifdef CONFIG_EXT_SWITCH
	case SIOCS8305VLAN:
		re8670_8305vlan_ioctl((struct ifvlan *)rq->ifr_data);
		break;
	case SIOCSMIIREG:
		//miiar_write((unsigned char)mii->phy_id,(unsigned char)mii->reg_num, mii->val_in);
		miiar_write(virt2phy[mii->phy_id],(unsigned char)mii->reg_num, mii->val_in);
		break;
	#endif
	case SIOCDIRECTBR:  // ioctl for direct bridge mode, jiunming
#ifdef FAST_ROUTE_Test
		//printk( "\nSIOCDIRECTBR: old=%d new=%d", fast_route_mode, rq->ifr_ifru.ifru_ivalue );
		if( rq->ifr_ifru.ifru_ivalue==1 )
			fast_route_mode = 1;
		else if( rq->ifr_ifru.ifru_ivalue==0 )
			fast_route_mode = 0;
		else
#endif
			rc = -EINVAL;
		break;
	case SIOCGMEDIALS:	// Jenny, ioctl for media link status
		status = RTL_R8(MSR);
		if (status & (1 << 2))
			rc = 0;
		else
			rc = 1;
		break;
	case SIOCETHTEST:
		eth_ctl((struct eth_arg *)rq->ifr_data);
		break;
	// Mason Yu. For Set IPQOS
	case SIOCSIPQOS:		
	 	re8670_ipqos_ioctl((struct ifIpQos *)rq->ifr_data);
	 	break;
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
#if defined (CONFIG_VLAN_8021Q) || defined (CONFIG_VLAN_8021Q_MODULE)
	cp->cpcmd &= ~RxVlanOn; //enable vlan
	cpw16(CpCmd, cp->cpcmd);//enable vlan
#else
	cpw16(CpCmd, cpr16(CpCmd) | RxVlanOn);
#endif
	spin_unlock_irq(&cp->lock);
}

static void cp_vlan_rx_kill_vid(struct net_device *dev, unsigned short vid)
{
	struct re_private *cp = dev->priv;

	spin_lock_irq(&cp->lock);
#if defined (CONFIG_VLAN_8021Q) || defined (CONFIG_VLAN_8021Q_MODULE)
	cp->cpcmd &= ~RxVlanOn; //enable vlan
	cpw16(CpCmd, cp->cpcmd);//enable vlan
#else
	cpw16(CpCmd, cpr16(CpCmd) & ~RxVlanOn);
#endif
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

#if 0
static int __devinit read_eeprom (void *ioaddr, int location, int addr_len)
{

	return 0;
}
#endif

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

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_NET
__NOMIPS16
#endif
int __init re8670_probe(void)
{
#ifdef CONFIG_EXT_SWITCH
	char baseDevName[8];
#endif
#ifdef MODULE
	printk("%s", version);
#endif

	struct net_device *dev;
	struct re_private *cp;
	int rc;
#ifdef CONFIG_RTL8672
	void *regs=(void*)0xB8018000 ;
#else
	void *regs=(void*)0xb9800000;
#endif
	unsigned i;
	u32 sicr;
//CRC patch for 8306 B
#ifdef  CONFIG_EXT_SWITCH 
      asicVersionPara_t AsicVer;
    
        getAsicVersionInfo(&AsicVer);
	  
      if ((AsicVer.chipid == RTL8306_CHIPID) && 
        (AsicVer.vernum == RTL8306_VERNUM) && 
        (AsicVer.revision == 0x0)  )
    {
        miiar_write(2, 26,  0x0056);
    }
#ifdef CONFIG_RE8306

#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
	unsigned short regvalue1;
        unsigned short regvalue2;
        unsigned short regvalue3;
        unsigned short regvalue4;
	unsigned short regvalue5;
	unsigned short val;
	miiar_WriteBit(0,16,11,1);	
	miiar_ReadBit(0,19,10,&regvalue1);
	miiar_ReadBit(0,20,7,&regvalue2);
	miiar_ReadBit(0,20,6,&regvalue3);
	miiar_ReadBit(0,20,5,&regvalue4);
	miiar_ReadBit(0,20,4,&regvalue5);
	miiar_WriteBit(0,16,11,0);
	//printk("***phy0 19 bit 10 %x\n",regvalue1);
	//printk("***phy0 20 bit 7 %x\n",regvalue2);
	//printk("***phy0 20 bit 6 %x\n",regvalue3);
	//printk("***phy0 20 bit 5 %x\n",regvalue4);
	//printk("***phy0 20 bit 4 %x\n",regvalue5);

	//miiar_read(0, 20, &val);
	//printk("***val %x\n", val);
	printk("Probe Reg : %d%d%d%d%d\n", regvalue1, regvalue2, regvalue3, regvalue5, regvalue4);
	//jim there are some strange case, regvalue 5 (8306 pin99) is strange.. 0 or 1 undefined...
        if((regvalue1==1)&&(regvalue2==0)&&(regvalue3==1)/*&&(regvalue5==1)&&(regvalue4==1)*/)
	{
	   virt2phy[SWITCH_VPORT_TO_867X] =4 ;
	}
        	
	else if((regvalue1==0)&&(regvalue2==1)&&(regvalue3==0))
	{
	   virt2phy[SWITCH_VPORT_TO_867X] =6 ;
	}
	else 	
	   printk("unkown ext switch board\n");	
#endif	
#endif

#if 0
	unsigned short valx;
	miiar_WriteBit(0,16,11,1);	
	miiar_read(0, 20, &valx);
	printk("***val %x\n", val);
#endif

	// Kaohj -- check if the gpio jtag share pin is used as gpio function
#ifdef GPIO_SIMULATE
//cathy
#ifndef CONFIG_RTL8672
	i = (1<<GPIO_MDC) | (1<<GPIO_MDIO);
        sicr = (*(volatile u32*)(0xB9C04000));
        if (i&0x00f8) // disable GPIO jtag function (gpio is used as gpio function)
		(*(volatile u32*)(0xb9c04000)) = sicr&0xffffffef;
#endif //cathy
#endif
#endif // of CONFIG_EXT_SWITCH
	
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
	//reDev = (struct re_private*)dev->priv;
	
	/* Jonah + for FASTROUTE */
	eth_net_dev=dev;
	
	cp->dev = dev;
	spin_lock_init (&cp->lock);
	
#if 1 // LINK_MODE_SUPPORT
	cp->mii_if.dev = dev;
	cp->mii_if.mdio_read = mdio_read;
	cp->mii_if.mdio_write = mdio_write;
	cp->mii_if.phy_id = 1; // LINK_MODE_SUPPORT
#endif
	
	dev->base_addr = (unsigned long) regs;
	cp->regs = regs;

	re8670_stop_hw(cp);

	/* read MAC address from EEPROM */
	for (i = 0; i < 3; i++)
		((u16 *) (dev->dev_addr))[i] = i;

	dev->open = re8670_open;
	dev->stop = re8670_close;
	dev->set_multicast_list = re8670_set_rx_mode;
	dev->hard_start_xmit = re8670_start_xmit;
	dev->get_stats = re8670_get_stats;
	// Kao, 2004/01/07, enable set mac address
	my_eth_mac_addr = dev->set_mac_address;
	dev->set_mac_address = re8670_set_mac_addr;
	dev->do_ioctl = re8670_ioctl;
	/*dev->change_mtu = re8670_change_mtu;*/
#if 1 
	dev->tx_timeout = re8670_tx_timeout;
	dev->watchdog_timeo = TX_TIMEOUT;
#endif
#ifdef CP_TX_CHECKSUM
	dev->features |= NETIF_F_SG | NETIF_F_IP_CSUM;
#endif
#if CP_VLAN_TAG_USED
	dev->features |= NETIF_F_HW_VLAN_TX | NETIF_F_HW_VLAN_RX;
	dev->vlan_rx_register = cp_vlan_rx_register;
	dev->vlan_rx_kill_vid = cp_vlan_rx_kill_vid;
#endif

#ifdef CONFIG_RTL8672
	dev->irq = 5;
#else
	dev->irq = 4;
#endif
	// Kaohj
	dev->priv_flags = IFF_DOMAIN_ELAN;

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

	// JONAH
	//init_bh(NIC_RX_BH, nic_rx);
	//init_bh(NIC_TX_BH, nic_tx);
	// initialise bottom half
	//INIT_LIST_HEAD(&cp->rx_task.list);
	//cp->rx_task.sync = 0;
	//cp->rx_task.routine = (void (*)(void *)) re8670_rx;
	//cp->rx_task.data = cp;
	memset(&cp->rx_tasklets, 0, sizeof(struct tasklet_struct));
	cp->rx_tasklets.func=(void (*)(unsigned long))re8670_rx;
	cp->rx_tasklets.data=(unsigned long)cp;
	eth_rx_tasklets = &cp->rx_tasklets;
	
	//INIT_LIST_HEAD(&cp->tx_task.list);
	//cp->tx_task.sync = 0;
	//cp->tx_task.routine = (void (*)(void *)) re8670_tx;
	//cp->tx_task.data = cp;
	//memset(&cp->tx_tasklets, 0, sizeof(struct tasklet_struct));
	//cp->tx_tasklets.func=re8670_tx;
	//cp->tx_tasklets.data=cp;
	
	// Kao, check if use internal phy
#ifdef CONFIG_RTL8672
	if (1)
#elif CONFIG_RTL8671
        sicr = (*(volatile u32*)(0xB9C04000));	//tylo, 0xbd800000));
	if (!(sicr & 0x00000600))  //7/1/05' hrchen, for 8671 MII spec
#else
	sicr = (*(volatile u32*)(0xbd800000));
	if (!(sicr & 0x10))
#endif
	{
		// use internal PHY, set and restart the MII register
		// reset
		internal_miiar_write(1, 0, 0x8000);
		// 10/100/H/F flow control
		internal_miiar_write(1, 4, 0x05e1);
		// restart
		internal_miiar_write(1, 0, 0x1200);
	}
	//else{
#ifdef CONFIG_EXT_SWITCH
//cathy
#ifndef CONFIG_RTL8672
#ifdef GPIO_8305_RESET
#ifndef RE8305_RESET
#define RE8305_RESET		8	// GPIO A0
#endif
		// reset 8305
		gpioConfig(RE8305_RESET, GPIO_FUNC_OUTPUT);
		gpioClear(RE8305_RESET);
		mdelay(10);
		gpioSet(RE8305_RESET);
#endif	// of GPIO_8305_RESET
#endif //cathy		
		for (i=0;i<SW_PORT_NUM;i++) {
		    // reset
			miiar_write(i, 0, 0x8000);
			// 10/100/H/F flow control
			miiar_write(i, 4, 0x05e1);
			// restart
			miiar_write(i, 0, 0x1200);
		};
		// Kaohj --- port 4(CPU port, if applicable) MAC4 phyid=5, enable Ethernet flow control
		miiar_write(5, 4, 0x05e1);
		
		// Kaohj, Setup the default 8305 configuration
		// port		vlanIdx		Vlan	VID	Egress
		//---------------------------------------------------------
		// 0		0		A	1	don't care
		// 1		1		B	2	don't care
		// 2		2		C	3	don't care
		// 3		3		D	4	don't care
		// 4		4		E	5	don't care
		rtl8305_info.vlan_en = 0;
		// remove tag for port 0,1,2,3 and don't care for port 4
		for (i=0;i<SW_PORT_NUM;i++) {
			rtl8305_info.phy[virt2phy[i]].egtag=TAG_DCARE;
			rtl8305_info.phy[virt2phy[i]].vlanIndex=i;
			rtl8305_info.vlan[i].vid=i+1;
			// vlan members contain all ports
	#ifdef CONFIG_USBCLIENT
	         rtl8305_info.vlan[i].member=0x3f;
	#else
		  rtl8305_info.vlan[i].member=0x1f;	
	#endif
		}
		
		rtl8305_info.phy[virt2phy[SWITCH_VPORT_TO_867X]].egtag=TAG_DCARE;
		rtl8305_info.phy[virt2phy[SWITCH_VPORT_TO_867X]].vlanIndex=SWITCH_VPORT_TO_867X;
		rtl8305_info.vlan[SWITCH_VPORT_TO_867X].vid=SWITCH_VPORT_TO_867X+1;
 		
        #ifdef CONFIG_USBCLIENT
              rtl8305_info.vlan[SWITCH_VPORT_TO_867X].member=0x3f;
	 #else
              rtl8305_info.vlan[SWITCH_VPORT_TO_867X].member=0x1f;
	#endif	
		set_8305(&rtl8305_info);
		
		//disable 8305 flow control
		RTL_W32(MIIAR, 0x800401e1);
		do
		{
			//6/8/04' hrchen, compiler bugs will cause the code hang
			//for (i = 1; i < 1000; i++);
			mdelay(10);
		}
		while (RTL_R32(MIIAR) & 0x80000000);
		RTL_W32(MIIAR, 0x840401e1);
		do
		{
			//6/8/04' hrchen, compiler bugs will cause the code hang
			//for (i = 1; i < 1000; i++);
			mdelay(10);
		}
		while (RTL_R32(MIIAR) & 0x80000000);
		RTL_W32(MIIAR, 0x880401e1);
		do
		{
			//6/8/04' hrchen, compiler bugs will cause the code hang
			//for (i = 1; i < 1000; i++);
			mdelay(10);
		}
		while (RTL_R32(MIIAR) & 0x80000000);
		RTL_W32(MIIAR, 0x8c0401e1);
		do
		{
			//6/8/04' hrchen, compiler bugs will cause the code hang
			//for (i = 1; i < 1000; i++);
			mdelay(10);
		}
		while (RTL_R32(MIIAR) & 0x80000000);
#endif
	//}
#ifdef CONFIG_RTL8672
	// andrew, applying CRC patch
	RTL_W32(MIIAR, 0x841f0003);
	mdelay(5);
	RTL_W32(MIIAR, 0x84093871);
	mdelay(5);
	RTL_W32(MIIAR, 0x841f0000);
#endif
	// Kaohj
#ifdef CONFIG_EXT_SWITCH
	strcpy(baseDevName, dev->name);
	
	for (i=0; i<SW_PORT_NUM; i++) {
		dev = alloc_etherdev(0);
		if (!dev)
			return -ENOMEM;
		
		dev->open = re8670_open;
		dev->stop = re8670_close;
		dev->set_multicast_list = re8670_set_rx_mode;
		dev->hard_start_xmit = re8670_start_xmit;
		dev->get_stats = re8670_get_stats;
		dev->set_mac_address = re8670_set_mac_addr;
		dev->do_ioctl = re8670_ioctl;
		dev->priv_flags = IFF_DOMAIN_ELAN;
		dev->priv = cp;
		strcpy(dev->name, baseDevName);
		strcat(dev->name, "_sw%d");
		
		rc = register_netdev(dev);
		if (rc)
			goto err_out_iomap;
        	
		dev_sw[i] = dev;	// virtual sw-port
		printk (KERN_INFO "%s: %s at port %d\n",
			dev->name, "RTL-8305", i);
		// Kaohj
		//printk("sw-port %d, %x\n", i, dev);
	}
#endif	// CONFIG_EXT_SWITCH

#ifdef CONFIG_EXT_SWITCH
//tylo, init. IGMP snooping table
for(i=0;i<MAX_IGMP_SNOOPING_GROUP;i++){
	//snooping_table[i].VLAN_index=-1;
	snooping_table[i].multicast_addr=0xc0000000;
	snooping_table[i].member_port=0x0;
	snooping_table[i].member_wlan=0;
	snooping_table[i].tmp_member_port=0x0;
	snooping_table[i].tmp_member_wlan=0;
	snooping_table[i].VLANID=0x100+i;
	snooping_table[i].timer_running=0;
}
#endif
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

static void __exit re8670_exit (void)
{
}

module_init(re8670_probe);
module_exit(re8670_exit);

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_NET
__NOMIPS16
#endif
void internal_miiar_write(unsigned char phyaddr,unsigned char regaddr,unsigned short value){
	unsigned int tmp=0;
cli();
	tmp=1<<31 | (phyaddr&0x1f)<<26 | (regaddr&0x1f)<<16 | (value&0xffff);	
	RTL_W32(MIIAR,tmp);
	do
	{
		mdelay(20);
	}
	while (RTL_R32(MIIAR) & 0x80000000);
sti();	
}

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_NET
__NOMIPS16
#endif
void miiar_write(unsigned char phyaddr,unsigned char regaddr,unsigned short value){
cli();
#ifndef GPIO_SIMULATE
	internal_miiar_write(phyaddr,regaddr,value);
#else
	rtl8305s_smiWrite(phyaddr,regaddr,value);
#endif	
sti();	
}

#ifdef CONFIG_RTL867X_KERNEL_MIPS16_DRIVERS_NET
__NOMIPS16
#endif
void miiar_read(unsigned char phyaddr,unsigned char regaddr,unsigned short *value){
	unsigned int tmp=0;
cli();
#ifndef GPIO_SIMULATE
	tmp=(phyaddr&0x1f)<<26 | (regaddr&0x1f)<<16;
	RTL_W32(MIIAR,tmp);
	do
	{
		mdelay(20);
	}
	while (!(RTL_R32(MIIAR) & 0x80000000));
	*value=RTL_R32(MIIAR)&0xffff;
#else
	rtl8305s_smiRead(phyaddr,regaddr,value);
#endif	
sti();	
}

// Kaohj
/*
 *	pvid		group(vlan)
 *	0 (reserved for local-out skb)
 *	x(SW_PORT_NUM+1)0 (default)
 *	x+1		1
 *	x+2		2
 *	x+3		3
 *	x+4		4
 */
int get_vid(int pvid)
{
	if (pvid>=SW_PORT_NUM+1 && pvid<=(SW_PORT_NUM+1+4))
		return rtl8305_info.vlan[pvid].vid;
	return 0;
}

int get_pvid(int vid)
{
	int i;
	
	for (i=SW_PORT_NUM+1; i<=(SW_PORT_NUM+1+4); i++) {
		if (rtl8305_info.vlan[i].vid == vid)
			return i;
	}
	return 0;
}

#ifdef CONFIG_EXT_SWITCH
//tylo, for VLAN
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifdef GPIO_SIMULATE
//Using GPIO Port A Pin 6 to emulate MDIO and GPIO Port A Pin 7 to emulate MDC
//cathy
#ifndef CONFIG_RTL8672
#define REG32GPIO(reg)	*(volatile unsigned int*)(0xb9c01000+reg)
#define GPIO_SHIFT(x)	((x<=7?x:(x-8))+16)
#else
#define REG32GPIO(reg)	*(volatile unsigned int*)(GPIOCR_BASE+reg) //jiunming, for 8672, *(volatile unsigned int*)(0xb9c01000+reg)
#define GPIO_SHIFT(x)	(x)	//jiunming, for 8672,((x<=7?x:(x-8))+16)
#endif //cathy
/* Change clock to 1 */
void _rtl8305s_smiZBit(void) {
	unsigned int i;
	//REG32(PABCDIR) = (REG32(PABCDIR)& 0x3FFFFFFF) | 0x80000000;
	REG32GPIO(PABCDIR)= (REG32GPIO(PABCDIR)& ~((1<<GPIO_SHIFT(GPIO_MDC))|(1<<GPIO_SHIFT(GPIO_MDIO))))
	 | (1<<GPIO_SHIFT(GPIO_MDC));
	//REG32(PABCDAT) = (REG32(PABCDAT) & 0x3FFFFFFF);
	//REG32GPIO(PABCDAT) = (REG32GPIO(PABCDAT) & 0xFFe7FFFF);
	gpioClear(GPIO_MDC);//GPIO_PA4
	gpioClear(GPIO_MDIO);//GPIO_PA3
	for(i=0; i<25; i++);
}

/* Generate  1 -> 0 transition and sampled at 1 to 0 transition time */
void _rtl8305s_smiReadBit(unsigned char * data) {
	unsigned int i;
	//REG32(PABCDIR) = (REG32(PABCDIR)& 0x3FFFFFFF) | 0x80000000;
	REG32GPIO(PABCDIR)= (REG32GPIO(PABCDIR)& ~((1<<GPIO_SHIFT(GPIO_MDC))|(1<<GPIO_SHIFT(GPIO_MDIO))))
	 | (1<<GPIO_SHIFT(GPIO_MDC));
	//REG32(PABCDAT) = (REG32(PABCDAT) & 0x3FFFFFFF) | 0x80000000;
	//REG32GPIO(PABCDAT) = (REG32GPIO(PABCDAT) & 0xFFe7FFFF) | 0x00100000;
	gpioSet(GPIO_MDC); //GPIO_PA4
	gpioClear(GPIO_MDIO); //GPIO_PA3
	for(i=0; i<25; i++);
	//REG32(PABCDAT) = (REG32(PABCDAT) & 0x3FFFFFFF);
	//REG32GPIO(PABCDAT) = (REG32GPIO(PABCDAT) & 0xFFe7FFFF);	
	gpioClear(GPIO_MDC);//GPIO_PA4
	gpioClear(GPIO_MDIO);//GPIO_PA3	
	//*data = (REG32(PABCDAT) & 0x40000000)?1:0;
	*data = (REG32GPIO(PABCDAT) & (1<<GPIO_SHIFT(GPIO_MDIO)))?1:0;
}

/* Generate  0 -> 1 transition and put data ready during 0 to 1 whole period */
void _rtl8305s_smiWriteBit(unsigned char data) {
	unsigned int i;
	
	//REG32(PABCDIR) = REG32(PABCDIR) | 0xC0000000;
	REG32GPIO(PABCDIR) = REG32GPIO(PABCDIR) | (1<<GPIO_SHIFT(GPIO_MDC)) | (1<<GPIO_SHIFT(GPIO_MDIO));
	if(data) {/* Write 1 */
		//REG32(PABCDAT) = (REG32(PABCDAT) & 0x3FFFFFFF) | 0x40000000;
		//REG32GPIO(PABCDAT) = (REG32GPIO(PABCDAT) & 0xFFe7FFFF) | 0x00080000;
		gpioClear(GPIO_MDC);//GPIO_PA4
		gpioSet(GPIO_MDIO);//GPIO_PA3

		for(i=0; i<25; i++);
		//REG32(PABCDAT) = (REG32(PABCDAT) & 0x3FFFFFFF) | 0xC0000000;
		//REG32GPIO(PABCDAT) = (REG32GPIO(PABCDAT) & 0xFFe7FFFF) | 0x00180000;
		gpioSet(GPIO_MDC);//GPIO_PA4
		gpioSet(GPIO_MDIO);//GPIO_PA3
	} else {
		//REG32(PABCDAT) = (REG32(PABCDAT) & 0x3FFFFFFF);
		//REG32GPIO(PABCDAT) = (REG32GPIO(PABCDAT) & 0xFFe7FFFF);
		gpioClear(GPIO_MDC);//GPIO_PA4
		gpioClear(GPIO_MDIO);//GPIO_PA3
		for(i=0; i<25; i++);
		//REG32(PABCDAT) = (REG32(PABCDAT) & 0x3FFFFFFF) | 0x80000000;
		//REG32GPIO(PABCDAT) = (REG32GPIO(PABCDAT) & 0xFFe7FFFF) | 0x00100000;
		gpioSet(GPIO_MDC);//GPIO_PA4
		gpioClear(GPIO_MDIO);//GPIO_PA3
	}
}

/*
@func void | rtl8305s_smiRead | Read data from RTL8305SB/RTL8305S through MDC/MDIO interface
@parm uint8 | phyad | Specify the PHY to get information.
@parm uint8 | regad | Specify the Regist of PHY to get information. 
@parm uint16 * | data | The data get from RTL8305SB/RTL8305S.
@comm
Specify the PHY address and register address to get data from RTL8305SB/RTL8305S
Change MDIO to input state after write operation.
*/
void rtl8305s_smiRead(unsigned char phyad, unsigned char regad, unsigned short * data) {
	int i;
	unsigned char readBit;

	/* Configure port A pin 6, 7 to be GPIO and disable interrupts of these two pins */
	//REG32(PABCCNR) = REG32(PABCCNR) & 0x3FFFFFFF;
	//REG32(PABIMR) = REG32(PABIMR) & 0xFFFFFFF;
	/* 32 continuous 1 as preamble*/
	for(i=0; i<32; i++)
		_rtl8305s_smiWriteBit(1);
	/* ST: Start of Frame, <01>*/
	_rtl8305s_smiWriteBit(0);
	_rtl8305s_smiWriteBit(1);
	/* OP: Operation code, read is <10> */
	_rtl8305s_smiWriteBit(1);
	_rtl8305s_smiWriteBit(0);
	/* PHY Address */
	for(i=4; i>=0; i--) 
		_rtl8305s_smiWriteBit((phyad>>i)&0x1);
	/* Register Address */
	for(i=4; i>=0; i--) 
		_rtl8305s_smiWriteBit((regad>>i)&0x1);
	/* TA: Turnaround <z0> */
	_rtl8305s_smiZBit();
	_rtl8305s_smiReadBit(&readBit);
	/* Data */
	*data = 0;
	for(i=15; i>=0; i--) {
		_rtl8305s_smiReadBit(&readBit);
		*data = (*data<<1) | readBit;
	}
	_rtl8305s_smiZBit();
}

/*
@func void | rtl8305s_smiWrite | Write data to RTL8305SB/RTL8305SC through MDC/MDIO interface
@parm uint8 | phyad | Specify the PHY to put information.
@parm uint8 | regad | Specify the Regist of PHY to put information. 
@parm uint16 | data | The data put to RTL8305SB/RTL8305S.
@comm
Specify the PHY address and register address to put data to RTL8305SB/RTL8305S.
Change MDIO to input state after write operation.
*/
void rtl8305s_smiWrite(unsigned char phyad, unsigned char regad, unsigned short data) {
	int i;

	/* Configure port A pin 6, 7 to be GPIO and disable interrupts of these two pins */
	//REG32(PABCCNR) = REG32(PABCCNR) & 0x3FFFFFFF;
	//REG32(PABIMR) = REG32(PABIMR) & 0xFFFFFFF;
	/* 32 continuous 1 as preamble*/
	for(i=0; i<32; i++)
		_rtl8305s_smiWriteBit(1);
	/* ST: Start of Frame, <01>*/
	_rtl8305s_smiWriteBit(0);
	_rtl8305s_smiWriteBit(1);
	/* OP: Operation code, write is <01> */
	_rtl8305s_smiWriteBit(0);
	_rtl8305s_smiWriteBit(1);
	/* PHY Address */
	for(i=4; i>=0; i--) 
		_rtl8305s_smiWriteBit((phyad>>i)&0x1);
	/* Register Address */
	for(i=4; i>=0; i--) 
		_rtl8305s_smiWriteBit((regad>>i)&0x1);
	/* TA: Turnaround <10> */
	_rtl8305s_smiWriteBit(1);
	_rtl8305s_smiWriteBit(0);
	/* Data */
	for(i=15; i>=0; i--) 
		_rtl8305s_smiWriteBit((data>>i)&0x1);
	_rtl8305s_smiZBit();
}

#endif

// Kaohj
void select_page(unsigned int npage)
{
	unsigned short rdata;
	
	miiar_read(0, 16, &rdata);
	switch (npage) {
		case 0:
			miiar_write(0, 16, (rdata & 0x7FFF) | 0x0002);
			break;
		case 1:
			miiar_write(0, 16, rdata | 0x8002 );
			break;
		case 2:
			miiar_write(0, 16, rdata & 0x7FFD);
			break;
		case 3:
			miiar_write(0, 16, (rdata & 0xFFFD) | 0x8000);
			break;
		default: // default page 0
			miiar_write(0, 16, (rdata & 0x7FFF) | 0x0002);
	}
}

void set_8305_phy(struct r8305_struc *info)
{
	int i;
	
	for (i=0; i<=SW_PORT_NUM; i++) {
		rtl8305sc_setVLANTagInsertRemove(virt2phy[i], info->phy[virt2phy[i]].egtag);
		rtl8305sc_setAsicPortVlanIndex(virt2phy[i], info->phy[virt2phy[i]].vlanIndex);
	}
}

void set_8305_vlan(struct r8305_struc *info)
{
	int i;
	
	for (i=0; i<=SW_PORT_NUM; i++) {
		rtl8305sc_setAsicVlan(i, info->vlan[i].vid, info->vlan[i].member);
	}
}

void set_8305(struct r8305_struc *info)
{
	set_8305_vlan(info);
	set_8305_phy(info);
}

void miiar_ReadBit(unsigned char phyad , unsigned char regad ,unsigned char bit,unsigned short *value ){
       unsigned short regData;
       if(bit >= 16)
	       return ;
       miiar_read(phyad,regad,&regData);
       if(regData&(1<<bit))
        *value = 1;
       else 
	*value = 0;
       //return SUCCESS;
}

void miiar_WriteBit(unsigned char phyad, unsigned char regad, unsigned char bit, unsigned char value) {
	unsigned short regData;
	
	if(bit>=16)
		return;
	miiar_read(phyad, regad, &regData);
	if(value) 
		regData = regData | (1<<bit);
	else
		regData = regData & ~(1<<bit);
	miiar_write(phyad, regad, regData);
}

int rtl8305sc_setPower(char port,char enabled) {
	miiar_WriteBit(port, 0, 11, enabled==FALSE?1:0);
	return SUCCESS;
}

//Configure switch to be VLAN switch
int rtl8305sc_setAsicVlanEnable(char enabled) {
	miiar_WriteBit(0, 18, 8, enabled==FALSE?1:0);
	return SUCCESS;
}

//Configure switch to be VLAN tag awared
int rtl8305sc_setAsicVlanTagAware(char enabled) {
	miiar_WriteBit(0, 16, 10, enabled==FALSE?1:0);
	return SUCCESS;
}

//VLAN tag insert and remove
//11:do not insert or remove		10:add
//01:remove						00:remove and add new tags
void rtl8305sc_setVLANTagInsertRemove(char port, char option){
	unsigned short regValue;
	miiar_read(port,22,&regValue);
	regValue=(regValue&0xfffc)|option;  //add tag for frames outputed from port 1
	miiar_write(port,22,regValue);
}

//VLAN tag 802.1p priority disable
// 1: disable priority classification		0: enable priority classification
void rtl8305sc_setVLANTag1pProirity(char port, char option){
	unsigned short regValue;
	miiar_read(port,22,&regValue);
	regValue=(regValue&0xfbff)|((option&0x01)<<10);
	miiar_write(port,22,regValue);
}

int rtl8305sc_setAsicBroadcastInputDrop(char enabled) {
	miiar_WriteBit(0, 18, 13, enabled==TRUE?1:0);
	return SUCCESS;
}

//Configure switch VLAN ID and corresponding member ports
int rtl8305sc_setAsicVlan(unsigned short vlanIndex, unsigned short vid, unsigned short memberPortMask) {
	unsigned short regValue;

	switch(vlanIndex) {
		case 0:/* VLAN[A] */
		miiar_read(0, 25, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		miiar_write(0, 25, regValue);
		miiar_read(0, 24, &regValue);
		regValue = (regValue & 0xFFE0) | (memberPortMask & 0x3F);
		miiar_write(0, 24, regValue);
		break;
		case 1:/* VLAN[B] */
		miiar_read(1, 25, &regValue);		
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		miiar_write(1, 25, regValue);
		miiar_read(1, 24, &regValue);
		regValue = (regValue & 0xFFE0) | (memberPortMask & 0x3F);
		miiar_write(1, 24, regValue);
	
		break;
		case 2:/* VLAN[C] */
		miiar_read(2, 25, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		miiar_write(2, 25, regValue);
		miiar_read(2, 24, &regValue);
		regValue = (regValue & 0xFFE0) | (memberPortMask & 0x3F);
		miiar_write(2, 24, regValue);
		break;
		case 3:/* VLAN[D] */
		miiar_read(3, 25, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		miiar_write(3, 25, regValue);
		miiar_read(3, 24, &regValue);
		regValue = (regValue & 0xFFE0) | (memberPortMask & 0x3F);
		miiar_write(3, 24, regValue);
		break;
		case 4:/* VLAN[E] */
		miiar_read(4, 25, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		miiar_write(4, 25, regValue);
		miiar_read(4, 24, &regValue);
		regValue = (regValue & 0xFFE0) | (memberPortMask & 0x3F);
		miiar_write(4, 24, regValue);
		break;
		case 5:/* VLAN[F] */
		miiar_WriteBit(0, 16, 15, 1); /* Change to page 1 */
		miiar_write(0, 27, vid);
		miiar_write(0, 26, memberPortMask);
		break;
		case 6:/* VLAN[G] */
		miiar_WriteBit(0, 16, 15, 1); /* Change to page 1 */
		miiar_write(1, 27, vid);
		miiar_write(1, 26, memberPortMask);
		break;
		case 7:/* VLAN[H] */
		miiar_WriteBit(0, 16, 15, 1); /* Change to page 1 */
		miiar_write(2, 27, vid);
		miiar_write(2, 26, memberPortMask);
		break;
		case 8:/* VLAN[I] */
		miiar_WriteBit(0, 16, 15, 1); /* Change to page 1 */
		miiar_write(3, 27, vid);
		miiar_write(3, 26, memberPortMask);
		break;
		case 9:/* VLAN[J] */
		miiar_WriteBit(0, 16, 15, 1); /* Change to page 1 */
		miiar_write(4, 27, vid);
		miiar_write(4, 26, memberPortMask);
		break;
		case 10:/* VLAN[K] */
		miiar_WriteBit(0, 16, 15, 1); /* Change to page 1 */
		miiar_write(0, 29, vid);
		miiar_write(0, 28, memberPortMask);
		break;
		case 11:/* VLAN[L] */
		miiar_WriteBit(0, 16, 15, 1); /* Change to page 1 */
		miiar_write(1, 29, vid);
		miiar_write(1, 28, memberPortMask);
		break;
		case 12:/* VLAN[M] */
		miiar_WriteBit(0, 16, 15, 1); /* Change to page 1 */
		miiar_write(2, 29, vid);
		miiar_write(2, 28, memberPortMask);
		break;
		case 13:/* VLAN[N] */
		miiar_WriteBit(0, 16, 15, 1); /* Change to page 1 */
		miiar_write(3, 29, vid);
		miiar_write(3, 28, memberPortMask);
		break;
		case 14:/* VLAN[O] */
		miiar_WriteBit(0, 16, 15, 1); /* Change to page 1 */
		miiar_write(4, 29, vid);
		miiar_write(4, 28, memberPortMask);
		break;
		case 15:/* VLAN[P] */
		miiar_WriteBit(0, 16, 15, 1); /* Change to page 1 */
		miiar_write(0, 31, vid);
		miiar_write(0, 30, memberPortMask);
		break;
		default:
			return -1;
	}
	return SUCCESS;
}

//Configure switch port un-tagged packet belonged VLAN index
int rtl8305sc_setAsicPortVlanIndex(unsigned short port, unsigned short vlanIndex) {
	unsigned short regValue;

	if(port>=SWITCH_PORT_NUMBER)
		return -1;
	if(port==6){
		miiar_WriteBit(0, 16, 15, 1); /* Change to page 1 */
		miiar_read(0, 26, &regValue);	
		regValue = (regValue & 0xFFF) | (vlanIndex<<12);		
		miiar_write(0, 26, regValue);	
		miiar_WriteBit(0, 16, 15, 0); /* Change to page 0 */
	}
	else{
		miiar_read(port, 24, &regValue);
		regValue = (regValue & 0xFFF) | (vlanIndex<<12);
		miiar_write(port, 24, regValue);
	}
	return SUCCESS;
}
int getAsicVersionInfo(asicVersionPara_t *pAsicVer)
{
    unsigned short regval;

    /*get chip id*/
     miiar_read(4, 30, &regval );
     pAsicVer->chipid = (unsigned short)regval;
    /*get version number*/
     miiar_read(4, 31,  &regval);
    pAsicVer->vernum = (unsigned char )(regval & 0xFF);
    /* bit[8:9]*/
    miiar_read(4, 31,  &regval);
    regval = (regval & 0x300) >> 8;
    if (regval  == 0 || regval ==2)
        pAsicVer->series = RTL8306_S;
    else if (regval == 1)
        pAsicVer->series = RTL8306_SD;
    else if (regval == 3)
        pAsicVer->series = RTL8306_SDM;
    else 
        pAsicVer->series = 0xFF;
	
   
	
    miiar_WriteBit(0, 16, 11, 1);
    miiar_read(4, 26,  &regval);
    pAsicVer->revision = (regval & 0xE000) >> 13;
    miiar_WriteBit(0, 16, 11,  0);

    return 0;
}
#endif // of CONFIG_EXT_SWITCH

// Kaohj
/*
 * Translate bitmap of virtual port to that of phy port bit (corresponding bit)
 * Old Bit-map:  bit5  |  bit4  |  bit3  |  bit2  |  bit1  |  bit0
 *               wlan  | device |  lan3  |  lan2  |  lan1  |  lan0
 * New Bit-map:  bit9 | bit8 | bit7 | bit6 | bit5  |  bit4  |  bit3  |  bit2  |  bit1  |  bit0
 *               vap3 | vap2 | vap1 | vap0 | wlan  | device |  lan3  |  lan2  |  lan1  |  lan0
   add usb client bit 10 |  bit9 | bit8 | bit7     | bit6  | bit5   |  bit4       |  bit3  |  bit2  |  bit1  |  bit0
 *                     vap3 | vap2 | vap1 | vap0 | wlan  | device |lan4 (usb)| lan3  |  lan2  |  lan1  |  lan0
 */
int bitmap_virt2phy(int mbr)
{
#ifdef CONFIG_EXT_SWITCH
	int k, mask, phymap, phyid;
	
	// Mason Yu
	//phymap = mbr&0xffffffc0;
//	
#ifdef CONFIG_RE8306
phymap = mbr&0xfffff800;
#else
phymap = mbr&0xfffffc00;
#endif	
	// translate switch ports
	for (k=0; k<=SW_PORT_NUM; k++) {
		mask = mbr & (1<<k);
		if (mask) {
			phyid = virt2phy[k];
			if (phyid >= 6)
				phyid = 5;
			phymap |= (1 << phyid);
		}
	}
	
	// Added by Mason Yu
	// translate wlan ap ports
  #ifdef CONFIG_RE8306
	phymap |= (mbr&0x3c0);
   #else
	phymap |= (mbr&0x3e0);
   #endif	
	return phymap;
#else
	return 0;
#endif
}

#ifdef CONFIG_EXT_SWITCH
void IGMP_Snooping_config(void){
	int i;
#ifdef CONFIG_RE8306
	int shift;
#endif
	IGMP_SNP_registered = 1;
	rtl8305_info.vlan_tag_aware = 1;
	rtl8305sc_setAsicVlanTagAware(1);	//enable tag aware
	rtl8305sc_setAsicBroadcastInputDrop(1);	//Broadcast output drop
	rtl8305sc_setAsicVlanEnable(1);	//VLAN enable

	rtl8305_info.phy[SWITCH_VPORT_TO_867X].egtag=TAG_REPLACE; // replace
	set_8305(&rtl8305_info);
#if 0
	//remove tag for port 0,1,2,3 and add tag for port 4
	rtl8305sc_setVLANTagInsertRemove(0,1);
	rtl8305sc_setVLANTagInsertRemove(1,1);
	rtl8305sc_setVLANTagInsertRemove(2,1);
	rtl8305sc_setVLANTagInsertRemove(3,1);
	rtl8305sc_setVLANTagInsertRemove(4,2);
	
	// VID of VLAN A = 000, VLAN A member = Port 0, 1, 2, 3 and  4 (MII) 
	//Port 0 index to VLAN A
	rtl8305sc_setAsicVlan(0, 0, 0x1f);
	rtl8305sc_setAsicPortVlanIndex(0, 0);
		
	// VID of VLAN B = 001, VLAN B member = Port 0, 1, 2, 3 and  4 (MII) 
	//Port 1 index to VLAN B
	rtl8305sc_setAsicVlan(1, 1, 0x1f);
	rtl8305sc_setAsicPortVlanIndex(1, 1);

	
	// VID of VLAN C = 002, VLAN C member = Port 0, 1, 2, 3 and  4 (MII) 
	//Port 2 index to VLAN C
	rtl8305sc_setAsicVlan(2, 2, 0x1f);	
	rtl8305sc_setAsicPortVlanIndex(2, 2);
	
	// VID of VLAN D = 003, VLAN D member = Port 0, 1, 2, 3 and  4 (MII) 
	//Port 3 index to VLAN D
	rtl8305sc_setAsicVlan(3, 3, 0x1f);
	rtl8305sc_setAsicPortVlanIndex(3, 3);

	
	//Port 4 index to VLAN E
	rtl8305sc_setAsicVlan(4, 4, 0x1f);
	rtl8305sc_setAsicPortVlanIndex(4, 4);
#endif
	RTL_W8(CMD,RTL_R8(CMD)|0x04);	// VLAN de-tagging enabled

#ifdef CONFIG_RE8305
	for (i=0; i<SW_PORT_NUM; i++)	// Jenny
		rtl8305sc_setAsicVlan(i+SW_PORT_NUM+1, i+SW_PORT_NUM+1, (1 << virt2phy[i]) | (1 << virt2phy[SW_PORT_NUM]) |0x00);	
#if 0
	//VLAN F member = Port 0 and Port 4 (MII), VID of VLAN F = 005
	rtl8305sc_setAsicVlan(5, 5, 0x11);	
	//VLAN G member = Port 1 and Port 4 (MII), VID of VLAN F = 006
	rtl8305sc_setAsicVlan(6, 6, 0x12);	
	//VLAN H member = Port 2 and Port 4 (MII), VID of VLAN F = 007
	rtl8305sc_setAsicVlan(7, 7, 0x14);	
	//VLAN I member = Port 3 and Port 4 (MII), VID of VLAN F = 008
	rtl8305sc_setAsicVlan(8, 8, 0x18);
#endif
#endif

#ifdef CONFIG_RE8306
	shift = virt2phy[SWITCH_VPORT_TO_867X];
	// id 4, 5 for port 4; id 6 for port 5
	if (shift == 5 || shift == 6)
		shift--;
#ifdef CONFIG_USBCLIENT	
//VLAN F member = Port 0 and Port 4 (MII), VID of VLAN F = 005
		
	//VLAN G member = Port 1 and Port 4 (MII), VID of VLAN F = 006
	rtl8305sc_setAsicVlan(6, 6, 0x01|(1<<shift));	
	//VLAN H member = Port 2 and Port 4 (MII), VID of VLAN F = 007
	rtl8305sc_setAsicVlan(7, 7, 0x02|(1<<shift));	
	//VLAN I member = Port 3 and Port 4 (MII), VID of VLAN F = 008
	rtl8305sc_setAsicVlan(8, 8, 0x04|(1<<shift));
	rtl8305sc_setAsicVlan(9, 9, 0x08|(1<<shift));
	rtl8305sc_setAsicVlan(10, 10, 0x10|(1<<shift));
#else	
	//VLAN F member = Port 0 and Port 4 (MII), VID of VLAN F = 005
	rtl8305sc_setAsicVlan(5, 5, 0x01|(1<<shift));	
	//VLAN G member = Port 1 and Port 4 (MII), VID of VLAN F = 006
	rtl8305sc_setAsicVlan(6, 6, 0x02|(1<<shift));	
	//VLAN H member = Port 2 and Port 4 (MII), VID of VLAN F = 007
	rtl8305sc_setAsicVlan(7, 7, 0x04|(1<<shift));	
	//VLAN I member = Port 3 and Port 4 (MII), VID of VLAN F = 008
	rtl8305sc_setAsicVlan(8, 8, 0x08|(1<<shift));
#endif	
#endif


}
// Kaohj
#ifdef CONFIG_RE8306_API
void dump_8306_qos()
{
	uint32 enabled;
	uint32 ip, mask;
	uint32 port, queue, quemask, num, set, hisize, losize, level, prio, qid;
	uint32 burstsize;
	uint32 regValue, weight, rate, i;
	uint32 pktOn, pktOff, dscOn, dscOff, qentry[4];
	
	printk("[Qos bandwidth control configration]\n");
	printk("[Gloabal configration]\n");
	rtl8306_getAsicPhyReg(5, 28, 3, &regValue);
	regValue = regValue & 0xFF;
	printk("Leaky bucket Tick = %3d, ", regValue);
	rtl8306_getAsicPhyReg(5, 27, 3, &regValue);
	printk("Token = %3d, ", (regValue >> 8) & 0xFF);
	printk("LBTHR = %3d, ", regValue & 0xFF);
	rtl8306_getAsicPhyReg(5, 29, 3, &regValue);
	printk("Current LBCNT = 0X%3X\n", regValue & 0x7FF);
	rtl8306_getAsicQosRxRateGlobalControl(&hisize, &losize, &enabled);
	printk("Rx LB   Highsize=%2d*1KB, Lowsize=%2d*1KB, Preamble %s\n", hisize, losize, enabled == TRUE ? "Include":"Not Include");
	rtl8306_getAsicQosPortQueueNum(&num);
	printk("Port queue number -- %d queue\n", num);
	printk("[Tx queue bandwidth control]\n");		
	for (set = 0; set < 2; set ++) {
		printk("Queue configuration [set %d]\n", set);
		for (queue = 0; queue < 4; queue ++) {
			rtl8306_getAsicQosTxQueueWeight(queue, &weight, set);
			if ((queue == RTL8306_QUEUE2) || (queue == RTL8306_QUEUE3)) {
				rtl8306_getAsicQosTxQueueStrictPriority(queue, set, &enabled);
				rtl8306_getAsicQosTxQueueLeakyBucket(queue, set, &burstsize, &rate);
				printk("Q%d Weight=%3d, Burstsize=%2d*1KB, Rate=%4d*64Kbps, %8s strict PRISCH\n",
					   queue, weight, burstsize, rate, enabled == TRUE ? "Enabled":"Disabled");
			} else 
				printk("Q%d Weight=%3d\n", queue, weight);		
		}						
	}
	printk("[Per port bandwidth control]\n");
	printk("%-8s%-18s%-18s%-11s%-10s%-10s\n", "", "Rx Rate(*64kbps)", "Tx Rate(*64kbps)", "Sche mode", "Q3 LB", "Q2 LB");
	for (port = 0; port < RTL8306_PORT_NUMBER; port ++) {
		rtl8306_getAsicQosPortRate(port, &rate, RTL8306_PORT_RX, &enabled);
		printk("Port%-4d(%-4d,%-12s", port, rate, enabled == TRUE ? "Enabled)":"Disabled)");
		rtl8306_getAsicQosPortRate(port, &rate, RTL8306_PORT_TX, &enabled);
		printk("(%-4d,%-12s", rate, enabled == TRUE ? "Enabled)":"Disabled)");
		rtl8306_getAsicQosPortScheduleMode(port, &set, &quemask);
		printk("set%-8d%-10s%-10s\n", set, (quemask & 0x8) ? "Enabled":"Disabled", (quemask & 0x4) ? "Enabled":"Disabled");																	
	}

	/*dump qos priority assignment configuration*/
	printk("\n");
	printk("[Qos priority assignment ]\n");
	printk("Priority to QID mapping (PRI--QID):\n");
	for (prio = 0; prio <= RTL8306_PRIO3; prio++) {
		rtl8306_getAsicQosPrioritytoQIDMapping(prio, &qid);
		printk("P%d--Q%d, ", prio, qid);

	}
	printk("\n");
	rtl8306_getAsicQosPktPriorityAssign(RTL8306_ACL_PRIO, &level);
	printk("Different Priority type arbitration level:\nACL-BASED = %d, ", level);
	rtl8306_getAsicQosPktPriorityAssign(RTL8306_DSCP_PRIO, &level);	
	printk("DSCP-BASED = %d, ", level);
	rtl8306_getAsicQosPktPriorityAssign(RTL8306_1QBP_PRIO, &level);		
	printk("1Q_BASED = %d, ", level);
	rtl8306_getAsicQosPktPriorityAssign(RTL8306_PBP_PRIO, &level);		
	printk("Port-Based = %d\n", level);
	printk("1Q tag priority to 2-bit priority mapping(1Q-2bit)\n");
	for (i = 0; i <= RTL8306_1QTAG_PRIO7; i++) {
		rtl8306_getAsicQos1QtagPriorityto2bitPriority(i, &prio);
		printk("0x%1x-%1d, ", i, prio);	
	}
	printk("\n");
	printk("DSCP priority to 2-bit priority mapping(DSCP-2bit):\n");
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_EF, &prio);
	printk("DSCP_EF  -%d, ", prio);
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_AFL1, &prio);
	printk("DSCP_AFL1-%d, ", prio);		
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_AFM1, &prio);
	printk("DSCP_AFM1-%d, ", prio);	
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_AFH1, &prio);
	printk("DSCP_AFH1-%d\n", prio);		
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_AFL2, &prio);
	printk("DSCP_AFL2-%d, ", prio);		
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_AFM2, &prio);
	printk("DSCP_AFM2-%d, ", prio);		
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_AFH2, &prio);
	printk("DSCP_AFH2-%d, ", prio);		
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_AFL3, &prio);
	printk("DSCP_AFL3-%d\n", prio);		
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_AFM3, &prio);
	printk("DSCP_AFM3-%d, ", prio);		
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_AFH3, &prio);
	printk("DSCP_AFH3-%d, ", prio);		
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_AFL4, &prio);
	printk("DSCP_AFL4-%d, ", prio);		
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_AFM4, &prio);
	printk("DSCP_AFM4-%d\n", prio);		
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_AFH4, &prio);
	printk("DSCP_AFH4-%d, ", prio);		
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_NC, &prio);
	printk("DSCP_NC  -%d, ", prio);		
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_REG_PRI, &prio);
	printk("DSCP_REG -%d, ", prio);		
	rtl8306_getAsicQosDSCPBasedPriority(RTL8306_DSCP_BF, &prio);
	printk("DSCP_BF  -%d\n", prio);		
	rtl8306_getAsicQosDSCPUserAssignPriority(RTL8306_DSCP_USERA, &prio, &enabled);
	printk("User specify DSCP code A 0x%2x, %s\n", prio, enabled== TRUE ? "Enabled" : "Disabled");
	rtl8306_getAsicQosDSCPUserAssignPriority(RTL8306_DSCP_USERB, &prio, &enabled);
	printk("User specify DSCP code B 0x%2x, %s\n", prio, enabled== TRUE ? "Enabled" : "Disabled");	
	rtl8306_getAsicQosIPAddressPriority(&prio);
	printk("IP Address priority = %d\n", prio);
	rtl8306_getAsicQosIPAddress(RTL8306_IPADD_A, &ip, &mask, &enabled);
	printk("IP priority Address[A]--%8s, ip %d:%d:%d:%d, mask %d:%d:%d:%d\n",
		   enabled == TRUE ? "Enabled":"Disabled",
		   (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF,
		   (mask >> 24) & 0xFF, (mask >> 16) & 0xFF, (mask >> 8) & 0xFF, mask &0xFF);
	
	rtl8306_getAsicQosIPAddress(RTL8306_IPADD_B, &ip, &mask, &enabled);
	printk("IP priority Address[B]--%8s, ip %d:%d:%d:%d, mask %d:%d:%d:%d\n",
		   enabled == TRUE ? "Enabled":"Disabled",
		   (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF,
		   (mask >> 24) & 0xFF, (mask >> 16) & 0xFF, (mask >> 8) & 0xFF, mask &0xFF);

	printk("[Per Port Priority Configuration]\n");
	printk("%8s%-12s%-12s%-12s%-12s\n", "","DSCP-based", "1Q-based", "Port-based", "CPU TAG-based");
	for (port = 0 ; port < RTL8306_PORT_NUMBER; port ++) {
		rtl8306_getAsicQosPriorityEnable(port, RTL8306_DSCP_PRIO, &enabled);
		printk("Port%d   %-12s", port, enabled == TRUE ? "Enabled" :"Disabled");
		rtl8306_getAsicQosPriorityEnable(port, RTL8306_1QBP_PRIO, &enabled);
		rtl8306_getAsicQos1QBasedPriority(port, &prio);
		printk("%8s,%-3d ", enabled == TRUE ? "Enabled" :"Disabled", prio);
		rtl8306_getAsicQosPriorityEnable(port, RTL8306_PBP_PRIO, &enabled);
		rtl8306_getAsicQosPortBasedPriority(port, &prio);
		printk("%8s,%-3d", enabled == TRUE ? "Enabled" :"Disabled", prio);
		rtl8306_getAsicQosPriorityEnable(port, RTL8306_CPUTAG_PRIO, &enabled);
		printk("%12s\n", enabled == TRUE ? "Enabled" :"Disabled");
	}
	printk("\n");
	printk("[Flow control]\n");
	printk("%-5s%-9s%-9s%-9s%-9s\n", "", "Q0", "Q1", "Q2", "Q3");
	for (i = 0 ; i < 2; i ++) {
		printk("Set%-2d", i);
		for (queue = 0; queue < 4; queue ++) {
			rtl8306_getAsicQosQueueFlowControl(queue, i, &enabled);				
			printk("%-9s", enabled == TRUE ? "Enable" : "Disable");
		}
		printk("\n");
	}
	for (port = 0; port < 6; port ++) {
		rtl8306_getAsicQosPortFlowControlMode(port, &set);
		printk("P%d - Set%d, ", port, set);
	}
	printk("\n");
	rtl8306_getAsicQosSystemRxFlowControl(&enabled);
	printk("System Rx Flow control %s\n", enabled == TRUE ? "Enabled":"Disabled");															
	/*dump flow control threshold*/
	rtl8306_getAsicPhyReg(5, 30, 2, &regValue);
	qentry[0] = (regValue >> 8) & 0x3F;
	qentry[1] = regValue &0x3F;
	rtl8306_getAsicPhyReg(5, 31, 2, &regValue);
	qentry[2] = (regValue >> 8) & 0x3F;	
	qentry[3] = 128 - qentry[0] - qentry[1]- qentry[2];
	for (set = 0 ; set < 2; set ++) {
		printk ("QFL Set %d threshold:\n", set);
		printk("%-4s%-10s%-10s%-10s%-10s%-10sSet%d\n", "","pktOn", "pktOff", "dscOn", "dscOff", "entry", set);
		for (i = 0; i < 4; i ++)  {
			rtl8306_getAsicQosQueueFlowControlThr(i, RTL8306_FCO_QLEN, RTL8306_FCON, set, &pktOn, &enabled);
			rtl8306_getAsicQosQueueFlowControlThr(i, RTL8306_FCO_QLEN, RTL8306_FCOFF, set, &pktOff, &enabled);		
			rtl8306_getAsicQosQueueFlowControlThr(i, RTL8306_FCO_DSC, RTL8306_FCON, set,  &dscOn,  &enabled);
			rtl8306_getAsicQosQueueFlowControlThr(i, RTL8306_FCO_DSC, RTL8306_FCOFF, set, &dscOff, &enabled);
			printk("Q%-3d%-10d%-10d%-10d%-10d%-10d\n", i, pktOn, pktOff, dscOn, dscOff, qentry[i]);
		}

	}
	printk("%-8s%-10s%-10s\n", "", "dscOn", "dscOff");
	for (port = 0 ; port < 6; port ++) {
		rtl8306_getAsicQosPortFLowControlThr(port, &dscOn, &dscOff, RTL8306_PORT_TX);
		printk("Port%-4d%-10d%-10d\n", port,  dscOn, dscOff);			
	}			
}		
	
int32 rtl8306_getAsicQosQueueFlowControl(uint32 queue, uint32 set, uint32 *enabled)
{
	uint32 bitValue;
	
	if ((queue > RTL8306_QUEUE3) || (set > RTL8306_FCO_SET1) || (enabled == NULL))
		return FAILED;
	switch (queue) {
	case RTL8306_QUEUE0:
		if (set == RTL8306_FCO_SET0)
			rtl8306_getAsicPhyRegBit(5, 22, 6, 2, &bitValue);
		else 
			rtl8306_getAsicPhyRegBit(5, 28, 6, 2, &bitValue);			
		break;
	case RTL8306_QUEUE1:
		if (set == RTL8306_FCO_SET0)
			rtl8306_getAsicPhyRegBit(5, 22, 7, 2, &bitValue);
		else 
			rtl8306_getAsicPhyRegBit(5, 28, 7, 2, &bitValue);					
		break;
	case RTL8306_QUEUE2:
		if (set == RTL8306_FCO_SET0)
			rtl8306_getAsicPhyRegBit(5, 22, 8, 2, &bitValue);
		else 
			rtl8306_getAsicPhyRegBit(5, 28, 8, 2, &bitValue);					
		break;
	case RTL8306_QUEUE3:
		if (set == RTL8306_FCO_SET0)
			rtl8306_getAsicPhyRegBit(5, 22, 9, 2, &bitValue);
		else 
			rtl8306_getAsicPhyRegBit(5, 28, 9, 2, &bitValue);					
		break;
	default:
		return FAILED;		
	}			
	*enabled = (bitValue == 0 ? TRUE:FALSE	);	
	return SUCCESS;	
}

#define RTL8306_MIB_CNT1			0
#define RTL8306_MIB_CNT2			1
#define RTL8306_MIB_CNT3			2
#define RTL8306_MIB_CNT4			3
#define RTL8306_MIB_CNT5			4
#define RTL8306_MIB_RESET		0
#define RTL8306_MIB_START		1
void reset_8306_counter()
{
	int i;
	
     for (i=0;i<=5; i++)
            rtl8306_setAsicMibCounterReset(i, RTL8306_MIB_RESET);

      for (i=0;i<=5; i++)
            rtl8306_setAsicMibCounterReset(i, RTL8306_MIB_START);
}

void dump_8306_counter()
{
	uint32 port, unit, runit;
	uint32 value;
	
	for (port = 0; port < RTL8306_PORT_NUMBER; port ++) {
		printk("Port%d:\n", port);
		rtl8306_getAsicMibCounter(port, RTL8306_MIB_CNT1, &value);												
		rtl8306_getAsicMibCounterUnit(port, RTL8306_MIB_CNT1, &unit);
		printk("TX COUNT = %d (%s; ", value, unit == RTL8306_MIB_BYTE ? "byte)" : "pkt)");
		rtl8306_getAsicMibCounter(port, RTL8306_MIB_CNT2, &value);
		rtl8306_getAsicMibCounterUnit(port, RTL8306_MIB_CNT2, &unit);
		printk("RX COUNT = %d (%s; ", value, unit == RTL8306_MIB_BYTE ? "byte)" : "pkt)");
		rtl8306_getAsicMibCounter(port, RTL8306_MIB_CNT3, &value);
		printk("RX Drop = %d(pkt);\n", value);	
		rtl8306_getAsicMibCounter(port, RTL8306_MIB_CNT4, &value);
		printk("RX CRCERR =%d(pkt); ", value);
		rtl8306_getAsicMibCounter(port, RTL8306_MIB_CNT5, &value);
		printk("RX ERR =%d(pkt)\n", value);								
	}	
}

#endif // of CONFIG_RE8306_API

// Kaohj
void dump_vlan_info()
{
	int i;
	
	printk("phy num=%d; vlan num=%d, enabled=%d\n\n", SWITCH_PORT_NUMBER+4, VLAN_NUM, rtl8305_info.vlan_en);
	printk("phy info:\n");
	printk("phy     vlanIndex\n");
	for (i=0; i<SWITCH_PORT_NUMBER+4; i++)
		printk("%d      %d\n", i, rtl8305_info.phy[i].vlanIndex);
	printk("\nvlan info\n");
	printk("index    vid    member\n");
	for (i=0; i<VLAN_NUM; i++)
		printk("%d       %d     0x%x\n", i, rtl8305_info.vlan[i].vid, rtl8305_info.vlan[i].member);
	printk("\n");
}
#endif // of CONFIG_EXT_SWITCH

//tylo, for link status check, only can be used for internal phy
#define LINK_STATUS  1<<2
#define SPEED	1<<13
#define DUPLEX	1<<8
// index: 0: eth0; 1: eth0_sw0; 2: eth0_sw1; 3: eth0_sw2; 4: eth0_sw3
//return 0:not connected, 1:100/FULL, 2:100/HALF, 3:10/FULL, 410/HALF
int link_test(int index){
	unsigned int ret=0;
	unsigned short reg_value;
	
#ifdef CONFIG_EXT_SWITCH
	if (index == 0)
		return 1; // 100/FULL
	else if (index >= 1)
		index = index -1;
#else
	if (index == 0)
		index = 1;
	else if (index >= 1)
		return 0;
#endif
	//printk("index=%d\n", index);
	miiar_read(index, 1, &reg_value);
	if((reg_value & LINK_STATUS)==0){
		//printk("Not connected\n");
		return 0;
	}
	
	miiar_read(index, 0, &reg_value);
	//printk("reg_value=%x\n",reg_value);
	if(reg_value & SPEED){
		//100
		if(reg_value & DUPLEX){
			//FULL
			//printk("100/FULL\n");
			return 1;
		}
		else{
			//HALF
			//printk("100/HALF\n");
			return 2;
		}
			
	}
	else{
		//10
		if(reg_value & DUPLEX){
			//FULL
			//printk("10/FULL\n");
			return 3;
		}
		else{
			//HALF
			//printk("10/HALF\n");
			return 4;
		}
	}

}

