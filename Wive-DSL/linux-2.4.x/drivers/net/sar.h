#define uClinux

#ifdef vxWorks
#include	"re8670.h"
#endif

#ifdef uClinux
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
#endif

/*--------------------------------
	Global Compile Definitions
---------------------------------*/

#define	ATM_OAM
#define	ATM_OAM_TEST
#define 	ENA_OAM_CH
#undef	SAR_CRC_GEN
#undef	LoopBack_Test	/* Enable Loopback Mode */

#ifdef uClinux
#undef	FAST_ROUTE_Test
#undef	Performance_Tune
// Bottom-Half use
#define SAR_BH   20        // 0-17 were reserved. max=32
#endif



/*--------------------------------
	Global Variable and Definitions
---------------------------------*/

#ifdef vxWorks
#define 	SAR_DEV_NAME 		"sar"	/* device name */
#define 	SAR_DEV_NAME_LEN 	3
#define	SAR_POLLING		1

// JONAH_DEBUG +
extern RE8670_END_DEVICE *reend_device;
extern STATUS re8670Send
    (
    RE8670_END_DEVICE           *pDrvCtrl,           /* device ptr */
    M_BLK_ID                             pMblk               /* data tosend */
    );
#endif

#ifdef uClinux
#undef	DISABLE_STAT
#define	TRUE	0
#define	FALSE	1
#endif

#ifdef vxWorks
#define	uint8	UINT8
#define	uint16	UINT16
#define	uint32	UINT32
#define	int8		INT8
#define	int16	INT16
#define	int32	INT32
#endif

#ifdef uClinux
#define	uint8	u8
#define	uint16	u16
#define	uint32	u32
#define	int8		signed char
#define	int16	signed short
#define	int32	signed long

#define	UINT8	uint8	
#define	UINT16	uint16	
#define	UINT32	uint32	
#define	INT8	int8		
#define	INT16	int16	
#define	INT32	int32	
#define	UCHAR	uint8
#define	BOOL	uint8
#endif


#undef	_LITTLE_ENDIAN


#define 	sar_reg(offset)  (*(volatile UINT32 *)((UINT32)SAR_ADDR + offset))
#define 	reg(address)  (*(volatile UINT32 *)((UINT32)address))

#define 	VC_CREATED			1
#define 	VC_NOT_CREATED	0

#ifndef vxWorks
#define	UNCACHE_MASK	0xA0000000
#endif

#define	DESC_ALIGN		256
#define	Uncache_Mask	0xA0000000


#define 	CRC10_POLY			0x633		/* Generation polynomial of CRC10 */
#define 	CRC32_POLY			0x04c11db7L	/* Generation polynomial of CRC328 */
#define 	VER_CRC32_POLY	0xc704dd7bL	/* Verification polynomial of CRC10 */

#define	SAR_TX_Buffer_Size	2048
#define	SAR_RX_Buffer_Size	2048

#define	SAR_OAM_Buffer_Size	64

#define 	SAR_RX_DESC_NUM	64
#define 	SAR_TX_DESC_NUM	64

#define	RING_SIZE			64	

#define	CRC32_Size			4
#define	Max_Packet_Size	1518
#define	Min_Packet_Size		64

#define	RDATHR		7
#define	RDATimer	1

#define	SAR_MAX_Process_DESC	64
#define	NIC_MAX_Process_DESC	64

#undef 	OAM_CH
#define	OAM_CH_NO		16
#define	Max_Header_Size	16
#define	Enable_VC_CNT	16


extern UCHAR	*FrameHeader_1483[4];
extern UCHAR	*FrameHeader_1577[2];
extern int8	FrameHeaderSize_1483[4];
extern int8	FrameHeaderSize_1577[2];

enum RFC_MODE{	
	RFC1483_BRIDGED, 	/* Ethernet over ATM (bridged) */
	RFC1483_ROUTED,	/* Ethernet over ATM (routed IP) */
	RFC1577,			/* IP over ATM */
	RFC2364,			/* PPP over ATM */
	RFC2516			/* PPP over Ethernet */
};

enum FRAMING_MODE{
	LLC_SNAP,
	VC_MUX
};




/*--------------------------------
		Register Address
---------------------------------*/
#define 	SAR_ADDR		0xB8300000

/* Channel */
#define	TV_Ctrl_Addr	SAR_ADDR+0x00000000
#define	TV_FDP_Addr	SAR_ADDR+0x00000004
#define	TV_SCR_Addr	SAR_ADDR+0x00000008
#define	TV_HDR_Addr	SAR_ADDR+0x0000000C

#define	TO_Ctrl_Addr	SAR_ADDR+0x00000100
#define	TO_FDP_Addr	SAR_ADDR+0x00000104

#define	RV_Ctrl_Addr	SAR_ADDR+0x00001000
#define	RV_FDP_Addr	SAR_ADDR+0x00001004
#define	RV_CKS_Addr	SAR_ADDR+0x00001008
#define	RV_HDR_Addr	SAR_ADDR+0x0000100C

#define	RO_Ctrl_Addr	SAR_ADDR+0x00001100
#define	RO_FDP_Addr	SAR_ADDR+0x00001104

/* Control */
#define	SAR_CNFG_Addr	 SAR_ADDR + 0x00002000
#define	SAR_STS_Addr	 SAR_ADDR + 0x00002004
#define	SAR_TBEI_Addr	 SAR_ADDR + 0x00002010
#define	SAR_TDFI_Addr	 SAR_ADDR + 0x00002014
#define	SAR_RDAI_Addr	 SAR_ADDR + 0x00002018
#define	SAR_RBFI_Addr	 SAR_ADDR + 0x0000201C
#define	SAR_RTOI_Addr	 SAR_ADDR + 0x0000200C

/* Debug */
#define	Test_Reg_0		0xB8303000
#define	Test_Reg_1		0xB8303010

/*--------------------------------
	Channel Control Definition
---------------------------------*/

#define	CHEN			1<<31
#define	SSL_Offset		19
#define	QoS_UBR		0
#define	QoS_CBR		1
#define	QoS_nrtVBR		2
#define	QoS_rtVBR		3
#define	TBEC			1<<25
#define	CDOI_Offset		18
#define	CDOI_Mask		0x00FC0000
#define 	SCR_Offset		16
#define	MBS_Offset		8
#define	GFC_Offset		28
#define	VPI_Offset		20
#define	VCI_Offset		4
	
#define	AAL5			1<<30
#define	ATRLREN		1<<29
#define	CFD				1<<25
#define	RBFC			1<<24

#define 	FFCN_Offset		16
#define	WIEN			1<<8
#define	IEPEN_Offset	6

#define	DNOAMEN		1<<30

/*--------------------------------
	SAR Control Definition
---------------------------------*/
#define	SAREN				1<<31
#define	UFLB				1<<30
#define	RDATHR_Offset		24
#define	RDATIMER_Offset	16
#define	TBEOMSK			1<<14
#define	TDFOMSK			1<<13
#define	QCLKSEL				1<<12
#define	QCLKOFFSET_Offset	8
#define	IERBF				1<<6
#define	IERDA				1<<5
#define	IERTO				1<<4
#define	IETBE				1<<3
#define	IETDF				1<<2

#define	RTO					1<<4
#define	RBF					1<<3
#define	RDA					1<<2
#define	TDF					1<<1
#define	TBE					1<<0


/*--------------------------------
	uClinux Related Definitions
---------------------------------*/

/* Time in jiffies before concluding the transmitter is hung. */
#ifdef uClinux
#define TX_TIMEOUT		(6*HZ)
#endif


/*--------------------------------
	OAM Related Definition
---------------------------------*/

#ifdef ATM_OAM
typedef struct tATMOAMLBReq {
	unsigned long 		vpi;			// vpi (input)
	unsigned long 		vci;			// vci (input)
	unsigned char 	Scope;		// segment-to-segment(0)/end-to-end(1) (input)
	unsigned char 	Channel;	// ATM0/ATM1 (input)
	unsigned char 	LocID[16];	// location ID (input)
	unsigned char 	SrcID[16];	// source ID (input)
	unsigned long		Tag;		// message tag (output for start, input for stop)
} ATMOAMLBReq;

#endif

/*--------------------------------
	Descriptor Structures 
---------------------------------*/
#define	OWN_32			1<<31

#define	OWN			1<<15
#define	EOR				1<<14
#define	FS				1<<13
#define	LS				1<<12
#define	ATMPORT_FAST	1<<11
#define	ATMPORT_SLOW	0
#define	TRLREN			1<<10
#define	ETHNT_OFFSET	4
#define	ETHNT_OFFSET_MSK	0x03F0
	#define	No_Offset		0x3F
	#define	Zero_Offset		0x00
#define	PTI_Offset_Tx	2
#define	CLP				1<<0
#define	LEN_Mask		0x0FFF
#define	CRC10EN		1<<10
#define	CRC32Err		1<<10
#define	LENErr			1<<9
#define	IPCErr			1<<8
#define 	PPI_Offset		6
#define	FRGI			1<<5
#define	WII				1<<4
#define	PTI_Offset_Rx	1
#define	isOAM			1<<8
#define	OAMType_Offset	6		

typedef struct packet_desc_struct {
  uint32 offset0;
  uint32 offset4;
  uint32 offset8;
  uint32 offsetC;
}packet_desc_t;

typedef struct {
	UINT16	CMD;
	UINT16	LEN;
	UINT32	START_ADDR;
	UINT16	TRLR;
	UINT16	RSVD0;
	UINT32	RSVD1;
} TV_CMD_DESC;

typedef struct {
	UINT16	STS;
	UINT16	LEN;
	UINT32	START_ADDR;
	UINT32	RSVD0;
	UINT32	RSVD1;
} TV_STS_DESC;

typedef struct {
	UINT16	CMD;
	UINT16	LEN;
	UINT32	START_ADDR;
	UINT32	RSVD0;
	UINT32	RSVD1;
} TO_CMD_DESC;

typedef struct {
	UINT16	STS;
	UINT16	LEN;
	UINT32	START_ADDR;
	UINT32	RSVD0;
	UINT32	RSVD1;
} TO_STS_DESC;

typedef struct {
	UINT16	CMD;
	UINT16	LEN;
	UINT32	START_ADDR;
	UINT32	RSVD0;
	UINT32	RSVD1;
} RV_CMD_DESC;


typedef struct {
	UINT16	STS;
	UINT16	LEN;
	UINT32	START_ADDR;
	UINT16	TRLR;
	UINT16 	BPC_LENGTH;
	UINT16	RSVD;
	UINT16	TUCACC;
} RV_STS_DESC;

typedef struct {
	UINT16	CMD;
	UINT16	LEN;
	UINT32	START_ADDR;
	UINT32	RSVD0;
	UINT32	RSVD1;
} RO_CMD_DESC;

typedef struct {
	UINT16	STS;
	UINT16	LEN;
	UINT32	START_ADDR;
	UINT16	BCC;
	UINT16	RSVD0;
	UINT32	RSVD1;
} RO_STS_DESC;

/*--------------------------------
		VC Structures
---------------------------------*/
typedef struct {
	UINT32	CtlSts;
	UINT32	FDP;
	UINT32	SCR;
	UINT32	HDR;
	UINT32	CtlSts_Addr;
	UINT32	FDP_Addr;
	UINT32	SCR_Addr;
	UINT32	HDR_Addr;
	UINT32	SegmentCRC;
	UINT16	buffer_size;
	INT8		Ether_Offset_Value;
	UINT8	desc_pf;		/* Free Buffer pointer */
	UINT8	desc_pc;		/* SAR CDOI pointer */
	UINT8	desc_pw;		/* Software Write Buffer Pointer */

} TV_Channel;

typedef struct {
	UINT32	CtlSts;
	UINT32	FDP;
	UINT32	CKS;
	UINT32	HDR;
	UINT32	CtlSts_Addr;
	UINT32	FDP_Addr;
	UINT32	CKS_Addr;
	UINT32	HDR_Addr;
	UINT32	SegmentCRC;
	UINT32	buffer_size;
	UINT8	desc_pr;		/* Software Read Buffer Pointer */
	UINT8	desc_pc;		/* SAR CDOI pointer */
	UINT8	desc_pa;		/* Allocate Buffer Pointer */
} RV_Channel;

/*--------------------------------
		Statistics
---------------------------------*/
typedef struct ch_stat{

	/* RX program flow related */
	INT32	rcv_cnt;			/* count of Receive Functiont calls */
	INT32	rcv_ok;			/* packet return count */
	
	/* RX data statistics related */
	INT32	rx_desc_cnt;	/* count of descriptors that receive Functiont walks through */
	INT32	rx_byte_cnt;	/* received byte count */
	INT32	rx_pkt_cnt;
	INT32	rx_pkt_fail;

	INT32	rx_FS_cnt;		/* count of FS desc */
	INT32	rx_LS_cnt;		/* count of LS desc */
	INT32	rx_desc_ok_cnt;	/* rx ok descriptors */
	INT32 	rx_oam_count;	/* rx oam cell count */
	INT32	rx_buf_alloc;	/* rx buffer allocated */
	INT32	rx_buf_lack;		/* lack of rx buffer */
	INT32	rx_desc_fail;	/* rx desc errors */
	INT32	rx_crc_error;	/* rx AAL5 CRC error count */
	INT32	rx_lenb_error;	/* rx packet length too large */
	INT32	rx_lens_error;	/* rx packet length too small */

	INT32	rx_netif_cnt;	/* count for netif calls */
	INT32	RBF_cnt;

	/* TX program flow related */
	INT32	send_cnt;		/* count of Send function calls */
	INT32	send_ok;		/* count of Send function ok */
	INT32	send_fail;		/* count of Send function errors */
	INT32	send_desc_full;	/* descriptors full */
	INT32	send_desc_lack;	/* not enough free descriptors */

	/* TX data statistics related */
	INT32	tx_desc_ok_cnt;	/* tx ok descriptors (successfully transmitted) */
	INT32	tx_buf_free;		/* tx buffer freed */
	INT32	tx_pkt_ok_cnt;	/* tx ok packets (successfully transmitted)*/
	INT32	tx_pkt_fail_cnt;	/* tx fail packets (send into descritprtor ring failed) */
	INT32	tx_byte_cnt;		/* send byte count (send into descriptor ring) */
	
}ch_stat;

typedef struct Traffic_Manage{
	UINT32	tick_now;
	INT32	cell_cnt;	
	INT8		Type;
	UINT16	PCR;
	UINT16	SCR;
	UINT8	MBS;
	UINT8	CRD;
	UINT32	CDVT;
}Traffic_Manage;


typedef struct 
{	
	int8				ch_no;
	uint8			vpi;
	uint16			vci;
	int				rfc;
	int				framing;
	int				created;
	int				loopback;
	uint8			MAC[6];
	Traffic_Manage	QoS;
	ch_stat			stat;


	TV_Channel	TV;
	RV_Channel	RV;

	BOOL		TBE_Flag;

	UINT32		rx_buf[64];		/* 64 Descriptors per channel  to record corresponding rx skb*/
	UINT32		tx_buf[64];		/* 64 Descriptors per channel  to record corresponding tx skb*/
	
	UINT32		skb_pool[64];	/* 64 skb pool for rx descriptors */
	UINT8		skb_pool_put;
	UINT8		skb_pool_get;

	/* statistics */
	INT32		RBF_cnt;		
	INT32		TBE_cnt;
	INT32		TDF_cnt;
	INT32		RDA_cnt;

	TV_CMD_DESC	*TV_Desc;	
	RV_STS_DESC	*RV_Desc;	

	TO_CMD_DESC	*TO_Desc;	
	RO_STS_DESC	*RO_Desc;	


}sar_atm_vcc;

/*--------------------------------
	IO Control Parameters
---------------------------------*/

/* IOCTLs */
struct SAR_IOCTL_CFG {
	int8				ch_no;
	uint8			vpi;					//vpi number
	uint16			vci;					//vci number
	int				rfc;
	int				framing;
	int				created;
	int				loopback;	
	uint8			MAC[6];
	Traffic_Manage	QoS;
	ch_stat			stat;
};

// ioctl command called by system & user space applications
#define SAR_GET_MODEMSTATE		SIOCDEVPRIVATE
#define SAR_GET_STATS			(SIOCDEVPRIVATE+1)
#define SAR_ENABLE			(SIOCDEVPRIVATE+2)
#define SAR_DISABLE			(SIOCDEVPRIVATE+3)
#define SAR_GET_CONFIG 			(SIOCDEVPRIVATE+4)
#define SAR_SET_CONFIG 			(SIOCDEVPRIVATE+5)
#define SAR_ATM_OAM_SET_ID		(SIOCDEVPRIVATE+6)
#define SAR_ATM_OAM_START		(SIOCDEVPRIVATE+7)
#define SAR_ATM_OAM_STOP		(SIOCDEVPRIVATE+8)
#define SAR_ATM_OAM_STATUS		(SIOCDEVPRIVATE+9)
#define SAR_ATM_OAM_STATUS_FE		(SIOCDEVPRIVATE+10)
#define SAR_ATM_OAM_RPT_LB		(SIOCDEVPRIVATE+11)
#define SAR_ATM_OAM_STOP_LB		(SIOCDEVPRIVATE+12)
#define SAR_CREATE_VC			(SIOCDEVPRIVATE+13)
#define SAR_DELETE_VC			(SIOCDEVPRIVATE+14)
#define SAR_ENABLE_UTOPIA		(SIOCDEVPRIVATE+15)
#define	SAR_UTOPIA_FAST			(SIOCDEVPRIVATE+16)
#define	SAR_UTOPIA_SLOW			(SIOCDEVPRIVATE+17)
#define	SAR_SETMAC			(SIOCDEVPRIVATE+18)
//#define SAR_ATM_OAM_LB_AGAIN		(SIOCDEVPRIVATE+)

/*--------------------------------
	SAR END Device Definition
---------------------------------*/
typedef struct rtl_sar_device
{	

#ifdef vxWorks
	END_OBJ		end;			/* The class we inherit from. */

	UINT8		ProcessRcv;
	int			unit;			/* unit number */
	char*       	pMclMem;        	/* ptr to MBLK/CL_BLK memory */
	UINT32      	flags;			/* Our local flags */
	CL_POOL_ID	pClPoolId;
	CL_POOL_ID	pClPoolId2;
	UCHAR		enetAddr[6];		/* ethernet address */
	UINT16		mBlkCount;      	/* MBLK count */
	UINT16		clCount;        	/* Extra Cluster count */
	M_BLK_ID	pollMblk;       	/* Used in polling mode only*/
	M_BLK_ID	RxPktHdrMblk;	/* Record FS descriptor (MBlk) */
	M_BLK_ID	RxCurHdrMblk;
	UINT16		pktlen;

#endif

#ifdef uClinux
	/* uClinux Related */
	struct net_device		*dev;
	spinlock_t			lock;
	dma_addr_t			ring_dma;
	struct net_device_stats	net_stats;
	struct pci_dev		*pdev;
	struct sk_buff		*frag_skb;
	struct sk_buff		*currskb;
	struct mii_if_info		mii_if;
	void					*regs;
	u32					msg_enable;
	
	/* Main Sar structure */
	UINT8		ProcessRcv;
	int			unit;			/* unit number */
	UINT32      	flags;			/* Our local flags */
	UCHAR		enetAddr[6];		/* ethernet address */
	UINT16		pktlen;
#endif

	char*		pTVDescBuf;        /* ptr to device descriptor memory (allocated) */
	char*		pTODescBuf;        /* ptr to device descriptor memory (allocated) */
	char*		pRVDescBuf;        /* ptr to device descriptor memory (allocated) */
	char*		pRODescBuf;        /* ptr to device descriptor memory (allocated) */

	TV_CMD_DESC	*TVDesc;	/* ptr to device descriptor memory (after masked, actually used) */
	RV_STS_DESC	*RVDesc;	/* ptr to device descriptor memory (after masked, actually used) */
	TO_CMD_DESC	*TODesc;	/* ptr to device descriptor memory (after masked, actually used) */	
	RO_STS_DESC	*RODesc;	/* ptr to device descriptor memory (after masked, actually used) */

	sar_atm_vcc		vcc[17];

	UINT32	CNFG_Reg	;	
	UINT32	STS_Reg	;	
	UINT32	TDFI_Reg	;	
	UINT32	TBEI_Reg	;	
	UINT32	RTOI_Reg	;	
	UINT32	RDAI_Reg	;	
	UINT32	RBFI_Reg	;	
	
	UINT32	tx_channel_on;	/* indicate which tx channel is enabled */
	UINT32	rx_channel_on;	/* indicate which rx channel is enabled */
	UINT16	atmport;
	int		QoS_Test;
#ifdef vxWorks
} SAR_END_DEVICE;
SAR_END_DEVICE	*sar_dev;
static SEM_ID		SarRxJobSem = NULL;
#endif

#ifdef uClinux
}sar_private;
extern sar_private	*sar_dev;
#endif

/*--------------------------------
		Macros
---------------------------------*/


#define	GetTxCDOI(pDrvCtrl, ch_no) \
		((int8)((REG32(pDrvCtrl->vcc[ch_no].TV.CtlSts_Addr)&CDOI_Mask)>>CDOI_Offset)&0x3F)

#define	GetRxCDOI(pDrvCtrl, ch_no) \
		((int8)((REG32(pDrvCtrl->vcc[ch_no].RV.CtlSts_Addr)&CDOI_Mask)>>CDOI_Offset)&0x3F)

#define 	sar_reg(offset)  (*(volatile UINT32 *)((UINT32)SAR_ADDR + offset))
#define 	reg(address)  (*(volatile UINT32 *)((UINT32)address))

#ifndef vxWorks
#define	REG32(reg) (*(volatile uint32 *)(reg))
#define	REG16(reg) (*(volatile uint16 *)(reg))
#define	REG8(reg) (*(volatile uint8 *)(reg))
#define	DRV_ENTER	{};//printk(KERN_DEBUG "---->Enter %s: %d  \n",__FUNCTION__,__LINE__)
#define	DRV_LEAVE	{};//printk(KERN_DEBUG "<----Leave %s: %d  \n",__FUNCTION__,__LINE__)
#endif

/*--------------------------------
	Functions Declarations
---------------------------------*/

void SetCRCTbl(void );
void GenCRC10Table(void);
void Enable_Sachem_Loopback(void);
void Enable_Sachem_Utopia(void);
void Set_RDA(int8 RdaThr, int8 RdaTimer);

void Enable_IERBF(void);
void Disable_IERBF(void);
void Enable_IERDA(void);
void Disable_IERDA(void);
void Enable_IERTO(void);
void Disable_IERTO(void);
void Enable_IETBE(void);
void Disable_IETBE(void);
void Enable_IETDF(void);
void Disable_IETDF(void);


void Enable_SAR(void);
void Disable_SAR(void);
void Enable_LoopBack(void);
void Disable_LoopBack(void);
void Enable_ATRLREN(int8 i);
void Disable_ATRLREN(int8 i);
void Enable_tx_ch(int8 i);
void Enable_rx_ch(int8 i);
void Disable_tx_ch(int8 i);
void Disable_rx_ch(int8 i);
void Enable_AAL5(int8 i);
void Enable_Raw(int8 i);

void Clear_TBE(int8 i, int8 CDOI);
void Clear_RBF(int8 i, int8 CDOI);
void Cell_Forced_Dropped(int8 i);
void SetQoS(int8 i, int8 QoS);
void SetPCR(int8 i, uint16 PCR);
void	SetSCR(int8 i, uint16 SCR);
void	SetMBS(int8 i, uint8 MBS);
uint8 GetCRD(int8 i);
void	SetVpiVci(uint8 VPI, uint16 VCI, int8 ch_no);	
uint32 ReadD_ (uint32 address);
void WriteD_ (uint32 address,  uint32 data);
void Set1 (uint32 address,  int8 index );
void Reset1 (uint32 address,  int8 index );
uint8 Test1 (uint32 address,  int8 index );
int32 S2i(uint8 * str_P);
void Dump(uint32 Buffer, int32 size);
void Search(uint32 pattern);
void Idle(int32 period);
void Reset_Sar(void);

void Enable_Word_Insert(int8 i);
void Disable_Word_Insert(int8 i);

void Enable_Drop_NonOAM(int8 i);
void Disble_Drop_NonOAM(int8 i);

void Set_QoS_Ext(void);
#ifdef vxWorks
BOOL ClearRxBuffer(SAR_END_DEVICE *pDrvCtrl, int8 ch_no);
BOOL ClearTxBuffer(SAR_END_DEVICE *pDrvCtrl, int8 ch_no);
void AllocVcBuff(SAR_END_DEVICE *cp, int8 ch_no);
void CreateVC(SAR_END_DEVICE *cp, struct SAR_IOCTL_CFG *cfg);
void	DeleteVC(SAR_END_DEVICE *cp, struct SAR_IOCTL_CFG *cfg);
void	flush_tx_desc(SAR_END_DEVICE *cp);
void Alloc_desc(SAR_END_DEVICE *pDrvCtrl);
void Free_desc(SAR_END_DEVICE *pDrvCtrl);
void Init_reg(SAR_END_DEVICE *pDrvCtrl);
#endif
#ifdef uClinux
BOOL ClearRxBuffer(sar_private *pDrvCtrl, int8 ch_no);
BOOL ClearTxBuffer(sar_private *pDrvCtrl, int8 ch_no);
void AllocVcBuff(sar_private *cp, int8 ch_no);
void CreateVC(sar_private *cp, struct SAR_IOCTL_CFG *cfg);
void	DeleteVC(sar_private *cp, struct SAR_IOCTL_CFG *cfg);
void Alloc_desc(sar_private *pDrvCtrl);
void Free_desc(sar_private *pDrvCtrl);
void Init_reg(sar_private *pDrvCtrl);
void	flush_tx_desc(sar_private *cp);
#endif

#ifdef LoopBack_Test
INT16 Lying_Engine(uint8 *buf, INT16 len);
void exchange_mac(uint8 *buf);
#endif

/////////////////////////////////////////////for OAM
#ifdef ATM_OAM
////////////////////////////////////////////////////////////////////////
//																		
//	Module Name			: $Workfile: AtmOAM.h $
//	Date					: $Date: 2004/04/23 12:08:13 $
//	Author				: Dick Tam, ITEX Network Group
//	$NoKeyword:$
//
//	Copyright 2003, Realtek Semiconductor Inc.
//
////////////////////////////////////////////////////////////////////////

#if !defined(__ATM_OAM_H__)
#define __ATM_OAM_H__

#define RTOS							VXWORKS

#if RTOS == VXWORKS
#include <linux/time.h>
#endif

#undef TRUE
#undef FALSE
#define TRUE							1
#define FALSE						0

#define OAM_CELL_SIZE				52

// OAM type & function type, 6th octet
#define	FAULT_AIS							0x10		// fault management
#define	FAULT_RDI							0x11
#define	FAULT_CC							0x14
#define   FAULT_LB							0x18
#define	PERFORMANCE_FPM					0x20		// performance management
#define	PERFORMANCE_BACKWARD_REPORT	0x21
#define   APS_GROUP_PROTECT				0x50		// APS coordinatin protocol
#define   APS_INDIVIDUAL_PROTECT			0x51
#define   ACT_DEACT_FPM_BR					0x80		// activation /deactivation
#define	ACT_DEACT_CC						0x81
#define	ACT_DEACT_FPM						0x82
#define  SYSTEM_MANAGEMENT				0xF0		// system management



/* OAM FM LB */
// OAM loopback timer interval
#define OAM_LB_WAIT_TIME					5000		// 5 sec
// OAM loopback state
#define FAULT_LB_IDLE						0
#define FAULT_LB_WAITING					1
#define FAULT_LB_STOP						2
// OAM loopback field size
#define OAM_LB_INDICATION_SIZE			1
#define OAM_LB_TAG_SIZE					4
#define OAM_LB_LLID_SIZE					16
#define OAM_LB_SRCID_SIZE					16
#define OAM_LB_UNUSE						8

// OAM cell format
#define OAM_FORMAT_H1						0													// OAM cell type and function
#define OAM_FORMAT_H2						1													// OAM cell type and function
#define OAM_FORMAT_H3						2													// OAM cell type and function
#define OAM_FORMAT_H4						3													// OAM cell type and function
#define OAM_FORMAT_TYPE					4													// OAM cell type and function

// OAM loopback cell format
#define OAM_FORMAT_LB_INDICATION			OAM_FORMAT_TYPE+1									// loopback indication
#define OAM_FORMAT_LB_TAG					OAM_FORMAT_LB_INDICATION+OAM_LB_INDICATION_SIZE	// correlation tag
#define OAM_FORMAT_LB_LLID				OAM_FORMAT_LB_TAG+OAM_LB_TAG_SIZE				// llid
#define OAM_FORMAT_LB_SID					OAM_FORMAT_LB_LLID+OAM_LB_LLID_SIZE				// source id
#define OAM_FORMAT_LB_UNUSE				OAM_FORMAT_LB_SID+OAM_LB_SRCID_SIZE				// source id

#define OAM_FORMAT_CRC10					52								// OAM cell CRC-16

#define LB_ALL_NODE_SIZE					6					// LB Location ID: 0 (all replies)
#define LB_TABLE_SIZE						6					// LB table size for TMN


/* OAM F5 */
typedef struct OAMF5Timer
{
#if RTOS == VXWORKS
	struct timer_list timer;
#endif

	void  *pOAM;						// OAMF5 struct pointer
	unsigned int OAMFunctionType;			// octect 6
	unsigned int OAMUserData;				// FM -- LB, index of OAMF5LBList
	
	unsigned short oneShot;				// one shot or periodic timer
	unsigned short occupied;				// occupied or empty
	unsigned long ulTimeout;				// timeout period, ms
} OAMF5Timer;

/* OAM FM LB */
typedef struct tOAMF5_LB_INFO {
	unsigned long	tag;								// LB correlation tag
	unsigned char	locationID[OAM_LB_LLID_SIZE];	// LB location ID
	unsigned char	channel;						// ATM0 (default) or ATM1
	char			scope;							// segment-to-segment (0) or end-to-end (1)
	unsigned long	count;							// Rx LB cell counters but it's generated by this CP.
	long			status;							// current status: idle or waiting
	long			all0_status;						// all 0 status for lccation ID: 0 only
	OAMF5Timer	timer;
	unsigned long	timestamp;						// timestamp of LB start
	unsigned long rtt;								// round-trip time of LB response
	unsigned char	cell[OAM_CELL_SIZE];
} OAMF5_LB_INFO;

typedef struct tOAMF5_LBRX_INFO {
	unsigned char	locationID[OAM_LB_LLID_SIZE];	// LB location ID
	unsigned long	sur_des_count;					// Rx LB cell counters but it's generated by other CP
	long			status;							// current status: empty or occupy
} OAMF5_LBRX_INFO;

typedef struct OAMF5 {
	unsigned short		VPI;												// vpi
	unsigned short		VCI;												// vci

	/* OAM FM -- LB */
	long			OAMF5lFaultLBState;										// loopback state
	unsigned long	OAMF5ulFaultLBTag;										// loopback correlation tag
	unsigned char	OAMF5ucCPID[OAM_LB_SRCID_SIZE];						// connection point ID

	// the first LB_ALL_NODE_SIZE is reserved for location ID: 0 (all replies)
	// the rest of list is used for other location ID
	// it's used by TMN on Tx only
	OAMF5_LB_INFO		OAMF5LBList[LB_ALL_NODE_SIZE+LB_TABLE_SIZE];

	// This is only for OAM RX LB used
	OAMF5_LBRX_INFO 	LBRXList[LB_ALL_NODE_SIZE];

	unsigned long	OAMF5ulFaultLBTxGoodCell;			// transmit good loopback cells
	unsigned long	OAMF5ulFaultLBRxGoodCell;			// receive good loopback cells

} OAMF5;


/* OAM F4 */
#define OAMF4Timer OAMF5Timer
#define OAMF4_LB_INFO OAMF5_LB_INFO
#define OAMF4_LBRX_INFO OAMF5_LBRX_INFO

typedef struct OAMF4 {
	unsigned short		VPI;												// vpi
	unsigned short		VCI;												// vci

	/* OAM FM -- LB */
	long			OAMF4lFaultLBState;										// loopback state
	unsigned long	OAMF4ulFaultLBTag;										// loopback correlation tag
	unsigned char	OAMF4ucCPID[OAM_LB_SRCID_SIZE];						// connection point ID

	// the first LB_ALL_NODE_SIZE is reserved for location ID: 0 (all replies)
	// the rest of list is used for other location ID
	// it's used by TMN on Tx only
	OAMF4_LB_INFO		OAMF4LBList[LB_ALL_NODE_SIZE+LB_TABLE_SIZE];

	// This is only for OAM RX LB used
	OAMF4_LBRX_INFO 	LBRXList[LB_ALL_NODE_SIZE];

	unsigned long	OAMF4ulFaultLBTxGoodCell;			// transmit good loopback cells
	unsigned long	OAMF4ulFaultLBRxGoodCell;			// receive good loopback cells

} OAMF4;



/* OAM F5, global structure */
extern OAMF5 OAMF5_info;
extern OAMF4 OAMF4_info;

/* OAM F5 */
extern void OAMF5Init(unsigned short vpi, unsigned short vci, OAMF5 *pOAMF5);
extern int OAMF5SetVpiVci(unsigned short vpi, unsigned short vci, OAMF5 *pOAMF5);
extern int OAMF5TMNSetCPID(unsigned char ucLocationType, unsigned char *pCPID, OAMF5 *pOAMF5);
extern int OAMF5TMNTxLBStart(unsigned char Scope, unsigned char *pLocID, unsigned long *Tag, OAMF5 *pOAMF5);
extern int OAMF5TMNTxLBStop(unsigned long Tag, OAMF5 *pOAMF5);
extern void OAMRxF5Cell(unsigned char *pOamCell, OAMF5 *pOAMF5);

/* OAM F4 */
extern void OAMF4Init(unsigned short vpi, unsigned short vci, OAMF4 *pOAMF4);
extern int OAMF4SetVpiVci(unsigned short vpi, unsigned short vci, OAMF4 *pOAMF4);
extern int OAMF4TMNSetCPID(unsigned char ucLocationType, unsigned char *pCPID, OAMF4 *pOAMF4);
extern int OAMF4TMNTxLBStart(unsigned char Scope, unsigned char *pLocID, unsigned long *Tag, OAMF4 *pOAMF4);
extern int OAMF4TMNTxLBStop(unsigned long Tag, OAMF4 *pOAMF4);
extern void OAMRxF4Cell(unsigned char *pOamCell, OAMF4 *pOAMF4);




/* ATM OAM Interface */
typedef struct tATMOAMLBID 
{
	unsigned long vpi;			// Near End vpi (input) 
	unsigned long vci;			// Near End vci (input) 
	unsigned char LocID[16];	// Near End connection point ID (input) 
} ATMOAMLBID;

typedef struct tATMOAMLBState 
{
	unsigned long vpi;				// Near End vpi (input) 
	unsigned long vci;				// Near End vci (input) 
	unsigned char LocID[6][16];		// location ID (output)
	unsigned long Tag;			// message tag (input)
	unsigned long count[6];		// statistic counter (output)
	unsigned long rtt[6];			// round-trip time (output)
	long	           status[6];			// state (output: waiting(1)/stop(2) )
}ATMOAMLBState;

typedef struct tATMOAMLBRXState
{
	unsigned long vpi;				// Near End vpi (input) 
	unsigned long vci;				// Near End vci (input) 
	unsigned char	LocID[6][16]; 	// location ID 
	unsigned long	count[6];        	// statistic counter (output)
	long		status[6];       		// state (output: empty (0) /occupy(1))
} ATMOAMLBRXState;

typedef void (*VOIDFUNCPTR) (unsigned long);  /* ptr to function returning void, for timer dunction */

#endif

#endif
