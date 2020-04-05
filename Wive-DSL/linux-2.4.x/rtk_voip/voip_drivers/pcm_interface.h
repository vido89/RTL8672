/*
 *	Realtek RTL8186 PCM Controller Driver Header File
 *
 *	Author : thlin@realtek.com.tw
 *
 *	2005.09.05	
 *
 *	Copyright 2005 Realtek Semiconductor Corp.
 */

#ifndef _PCM_INTERFACE
#define _PCM_INTERFACE

#include <linux/config.h>
#include "rtk_voip.h"

#ifdef OPTIMIZATION
#define pcm_get_page_size(x) (PCM_PERIOD*PCM_PERIOD_10MS_SIZE)
#endif

/*******************  PCM FIFO interface ********************/
#ifdef SUPPORT_PCM_FIFO
/*
 * read/write one 160-byte pcm data from/to RX/TX FIFO
 * On sucess, 0 is returned. 
 * On error, -1 is returned. 
 */
int pcm_read_rx_fifo(unsigned int chid, void* dst);
int pcm_write_rx_fifo(unsigned int chid, void* src);
int pcm_read_tx_fifo(unsigned int chid, void* dst);
int pcm_write_tx_fifo(unsigned int chid, void* src);

int pcm_get_read_rx_fifo_addr(unsigned int chid, void** addr);
int pcm_read_rx_fifo_done(unsigned int chid);
int pcm_get_write_tx_fifo_addr(unsigned int chid, void** addr);
int pcm_write_tx_fifo_done(unsigned int chid);
#else
/*
 * get pcm RX/TX Buffer address
 */
unsigned char* pcm_get_rx_buf_addr(unsigned int chid);
unsigned char* pcm_get_tx_buf_addr(unsigned int chid);
#endif // SUPPORT_PCM_FIFO


/*******************  Function Prototype ********************/
void PCM_init(void);
void PCM_shutdown(void);
void PCM_restart(unsigned int chid);
void pcm_enableChan(unsigned int chid);
void pcm_disableChan(unsigned int chid);

/* Set Tx, Rx own bit to PCM Controller. pageindex = 0 ~ 1. */
void pcm_set_tx_own_bit(unsigned int chid, unsigned int pageindex);
void pcm_set_rx_own_bit(unsigned int chid, unsigned int pageindex);

/* clean interrupt pending bits */
void pcm_clean_isr(unsigned int statusval);

/* Get the Tx, Rx base address */
unsigned int pcm_get_tx_base_addr(unsigned int chid);
unsigned int pcm_get_rx_base_addr(unsigned int chid);

void pcm_enable(void);
void pcm_disable(void);

void pcm_tx_enable(unsigned int chid);
void pcm_tx_disable(unsigned int chid);
void pcm_rx_enable(unsigned int chid);
void pcm_rx_disable(unsigned int chid);

void pcm_imr_enable(unsigned int chid, unsigned char type);
void pcm_imr_disable(unsigned int chid, unsigned char type);
void print_pcm(void);
int voip_mgr_daa_off_hook(char chid);
void voip_mgr_daa_on_hook(char chid);
void pcm_channel_slot(void);
void pcm_set_tx_mute(unsigned char chid, int enable);
void pcm_set_rx_mute(unsigned char chid, int enable);
void pcm_Hold_DAA(unsigned char chid);
void pcm_UnHold_DAA(unsigned char chid);
/********************************************************************/
#ifdef CONFIG_RTK_SLIC_DAA_NEGOTIATION
extern unsigned int DAA_ISR_FLOW[];

#define DRIVER_FLOW_NORMAL		0
#define DRIVER_FLOW_PSTN_3WAY		1
#define DRIVER_FLOW_PSTN_FORWARD	2
#define DRIVER_FLOW_PSTN_COUPLE		3
#endif

#if 0 // use rtk_voip.h's SLIC_CH_NUM instead
#define PCM_DEVS 2	/* Set here to determine SLIC device number */
#endif
#define MAXCH	4

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
#define PCM_IRQ	13
#elif CONFIG_RTK_VOIP_DRIVERS_PCM8186
#define PCM_IRQ	7
#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8671
#define PCM_IRQ	10
#endif

#define Linear	0
#define A_law	1
#define U_law	2

#define TRUE	1
#define FALSE	0

#ifdef CONFIG_RTK_SLIC_DAA_NEGOTIATION
#define PCM_SAMPLE_DAA	80
#endif

/*  Interrupt Status Type */
#define TOK2DSP		0
#define ROK2DSP		1
#define TUB2DSP		2
#define RUB2DSP		3

/* Interrupt Mask Type */
#define P0OK		0
#define P1OK		1
#define TBUA		2
#define RBUA		3

/* Define the allocated memory size for PCM tx, rx buffer(2 page) */
#define PCM_SIZE_N		255					//page size = 320, 20ms
#define PCMPAGE_SIZE	(4*(PCM_SIZE_N + 1))	//one pages, 4(n + 1)
#define BUFFER_SIZE		(PCMPAGE_SIZE*2)	//two pages


#define pcm_outb(address, value)	writeb(value, address)
#define pcm_outw(address, value)	writew(value, address)
#define pcm_outl(address, value)	writel(value, address)

#define pcm_inb(address)		readb(address)
#define pcm_inw(address)		readw(address)
#define pcm_inl(address)		readl(address)

#ifndef BIT
#define BIT(x)	(1 << x)
#endif
	
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
#ifndef PCM_BASE
#define PCM_BASE (0xbd017000)
#endif
#elif CONFIG_RTK_VOIP_DRIVERS_PCM8186
#define PCM_BASE (0xbd280000)
#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8671
#define PCM_BASE (0xb8e08000)
#endif

#define PCMCR		(PCM_BASE + 0x00)	//Interface Control Register
#define PCMCHCNR	(PCM_BASE + 0x04)	//Channel specific Control Register
#define PCMTSR		(PCM_BASE + 0x08)	//Time Slot Assignment Register
#define PCMBSIZE	(PCM_BASE + 0x0C)	//Channels Buffer Size register
#define CH0TXBSA	(PCM_BASE + 0x10)	//Channel 0 TX buffer starting address pointer
#define CH1TXBSA	(PCM_BASE + 0x14)	//Channel 1 TX buffer starting address pointer
#define CH2TXBSA	(PCM_BASE + 0x18)	//Channel 2 TX buffer starting address pointer
#define CH3TXBSA	(PCM_BASE + 0x1C)	//Channel 3 TX buffer starting address pointer
#define CH0RXBSA	(PCM_BASE + 0x20)	//Channel 0 RX buffer starting address pointer
#define CH1RXBSA	(PCM_BASE + 0x24)	//Channel 1 RX buffer starting address pointer
#define CH2RXBSA	(PCM_BASE + 0x28)	//Channel 2 RX buffer starting address pointer
#define CH3RXBSA	(PCM_BASE + 0x2C)	//Channel 3 RX buffer starting address pointer
#define PCMIMR		(PCM_BASE + 0x30)	//Channels Interrupt Mask Register
#define PCMISR		(PCM_BASE + 0x34)	//Channels Interrupt Status Register


#define TX_BSA(channel)	(CH0TXBSA + 4*(channel))
#define RX_BSA(channel)	(CH0RXBSA + 4*(channel))

//PCMCR
#define PCME			BIT(12)
#define CKDIR		BIT(11)
#define PXDSE		BIT(10)
#define FSINV		BIT(9)

#define PCM_ENABLE		(PCME | CKDIR)


//PCMCHCNR
#define CH3RE		BIT(0)
#define CH3TE		BIT(1)
#define CH3UA		BIT(2)
#define C3CMPE		BIT(3)

#define CH2RE		BIT(8)
#define CH2TE		BIT(9)
#define CH2UA		BIT(10)
#define C2CMPE		BIT(11)

#define CH1RE		BIT(16)
#define CH1TE		BIT(17)
#define CH1UA		BIT(18)
#define C1CMPE		BIT(19)

#define CH0RE		BIT(24)
#define CH0TE		BIT(25)
#define CH0UA		BIT(26)
#define C0CMPE		BIT(27)
#define C0ILBE		BIT(28)

//PCMIMR
#define CH0P0OKIE 	BIT(15)
#define CH0P1OKIE 	BIT(14)
#define CH0TBUAIE 	BIT(13)
#define CH0RBUAIE 	BIT(12)

#define CH1P0OKIE 	BIT(11)
#define CH1P1OKIE 	BIT(10)
#define CH1TBUAIE 	BIT(9)
#define CH1RBUAIE 	BIT(8)

#define CH2P0OKIE 	BIT(7)
#define CH2P1OKIE 	BIT(6)
#define CH2TBUAIE 	BIT(5)
#define CH2RBUAIE 	BIT(4)

#define CH3P0OKIE 	BIT(3)
#define CH3P1OKIE 	BIT(2)
#define CH3TBUAIE 	BIT(1)
#define CH3RBUAIE 	BIT(0)

//PCMISR
#define CH3P1RBU	BIT(0)
#define CH3P0RBU	BIT(1)
#define CH3P1TBU	BIT(2)
#define CH3P0TBU	BIT(3)
#define CH3P1ROK	BIT(4)
#define CH3P0ROK	BIT(5)
#define CH3P1TOK	BIT(6)
#define CH3P0TOK	BIT(7)

#define CH2P1RBU	BIT(8)
#define CH2P0RBU	BIT(9)
#define CH2P1TBU	BIT(10)
#define CH2P0TBU	BIT(11)
#define CH2P1ROK	BIT(12)
#define CH2P0ROK	BIT(13)
#define CH2P1TOK	BIT(14)
#define CH2P0TOK	BIT(15)

#define CH1P1RBU	BIT(16)
#define CH1P0RBU	BIT(17)
#define CH1P1TBU	BIT(18)
#define CH1P0TBU	BIT(19)
#define CH1P1ROK	BIT(20)
#define CH1P0ROK	BIT(21)
#define CH1P1TOK	BIT(22)
#define CH1P0TOK	BIT(23)

#define CH0P1RBU	BIT(24)
#define CH0P0RBU	BIT(25)
#define CH0P1TBU	BIT(26)
#define CH0P0TBU	BIT(27)
#define CH0P1ROK	BIT(28)
#define CH0P0ROK	BIT(29)
#define CH0P1TOK	BIT(30)
#define CH0P0TOK	BIT(31)

//----------------------------------------------------

//#define BSIZE(channel, size)	((size << 24) >> (8*channel))	
#define BSIZE(channel, size)	( (size & 0xFF) << ((3-channel)*8))

#define P1RBU(channel)		(CH0P1RBU >> (8*channel))
#define P0RBU(channel)		(CH0P0RBU >> (8*channel))
#define P1TBU(channel)		(CH0P1TBU >> (8*channel))
#define P0TBU(channel)		(CH0P0TBU >> (8*channel))
#define P1ROK(channel)		(CH0P1ROK >> (8*channel))
#define P0ROK(channel)		(CH0P0ROK >> (8*channel))
#define P1TOK(channel)		(CH0P1TOK >> (8*channel))
#define P0TOK(channel)		(CH0P0TOK >> (8*channel))

#define ISR_MASK(channel)	(0xFF000000 >> (8*channel))
#define TOK_MASK(channel)	(0xC0000000 >> (8*channel))
#define ROK_MASK(channel)	(0x30000000 >> (8*channel))
#define TBU_MASK(channel)	( 0xC000000 >> (8*channel))
#define RBU_MASK(channel)	( 0x3000000 >> (8*channel))

#define CHxRE(channel)		(CH0RE   >> (8*channel))
#define CHxTE(channel)		(CH0TE   >> (8*channel))
#define CHxUA(channel)		(CH0UA   >> (8*channel))
#define CxCMPE(channel)		(C0CMPE  >> (8*channel))

#define CHxP0OKIE(channel)	(CH0P0OKIE >> (4*channel))
#define CHxP1OKIE(channel)	(CH0P1OKIE >> (4*channel))
#define CHxTBUAIE(channel)	(CH0TBUAIE >> (4*channel))
#define CHxRBUAIE(channel)	(CH0RBUAIE >> (4*channel))

#define NAME_SIZE	10
#define BOOL	unsigned char

struct pcm_priv{
        unsigned int chidannel;
	unsigned int txpindex;		//tx page index.
	unsigned int rxpindex;		//rx page index
	unsigned int rx_pagepointer;	//used for read. use this to move in the circular buffer page
	unsigned int tx_pagepointer;	//used for write.
	char name[NAME_SIZE];
	unsigned char* tx_buffer;	//buffer address of non-cached address
	unsigned char* rx_buffer;	//buffer address of non-cached address
	unsigned char* tx_allocated;	//buffer for tx - virtual address
	unsigned char* rx_allocated;	//buffer for rx - virtual address
	unsigned char* rx_circular;	//rx circular buffer.
	unsigned char* tx_circular;	//tx circular buffer.

	unsigned int rx_W;		//rx write index
	unsigned int rx_R;		//rx read index
	unsigned int tx_W;		//tx write index
	unsigned int tx_R;		//rx read index
	BOOL rx_overflow;		//rx circular buffer overflow.
        unsigned int chidCNRValue;        //a-law, i-law or linear mode. tx or rx enable.

	unsigned int mode;		//operation mode - linear, a-law or u-law
	
	wait_queue_head_t	wait_qr;
	wait_queue_head_t	wait_qw;

#ifdef USE_TASKLET
	struct tasklet_struct	rx_tasklet;
	struct tasklet_struct	tx_tasklet;
#endif

	//single-open
	spinlock_t	open_lock;
	unsigned int 	open_count;	//open count

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
	struct fasync_struct *pcm_fasync;    // asynch notification
#endif
};

//---------------------------------------------------------------------------------------------
//	Debug
//---------------------------------------------------------------------------------------------

//#define PCM_DEBUG

#undef PDBUG

#ifdef PCM_DEBUG
	#define PDBUG(fmt, args...) printk("-%s:" fmt, __FUNCTION__, ## args)
#else
	#define PDBUG(fmt, args...)
#endif


#define PERR(fmt, args...)	printk(KERN_ERR "PCM - " fmt, ## args)

#define PERR(fmt, args...)	printk(KERN_ERR "PCM - " fmt, ## args)





#endif
