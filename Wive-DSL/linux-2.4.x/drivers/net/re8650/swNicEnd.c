/* rtl8650End.c - RealTek RTL81x9 Fast Ethernet interface header
*
* $Author: kaohj $
*
* $Log: swNicEnd.c,v $
* Revision 1.1.1.1  2003/08/18 05:43:22  kaohj
* initial import into CVS
*
* Revision 1.4  2003/05/23 06:02:56  elvis
* 1. use "rtl_mbuf.h" for ASIC mbuf data structure
* 2. adjust RTL8650_ONLY directives
*
* Revision 1.3  2003/05/19 09:52:49  elvis
* regular update
*
* Revision 1.2  2003/05/14 03:03:10  kckao
* Add I-RAM initialize function
*
* Revision 1.1  2003/05/12 09:57:38  kckao
* moved from csp
*
* ---------------------------------------------------------------
*/
/* Copyright 1984-2000 Wind River Systems, Inc. */
/*
modification history
--------------------
01c,28jan00,dgy  Updated comments that refered to the wrong files.
01b,28jan00,dgy  Changed ryl81x9CsrReadByte to use the correct SYS_IN
				 macro.
01a,29oct99,dgy  created
*/

/*

INCLUDES:
end.h endLib.h etherMultiLib.h rtl8650End.h

SEE ALSO: muxLib, endLib, netBufLib
.pG "Writing and Enhanced Network Driver"

*/

#define ERROR -1



//#define RTL8650_ONLY
//#define L3_ACCELERATION
//#define L4_ACCELERATION

#define MAX_PORT_NUM     	5
#define RTL8650_LOAD_COUNT	8

#include "rtl_types.h"
#ifdef RTL8650_ONLY
#include "asicRegs.h"
#include "swCore.h"
#else 
#include "rtl8651_asicRegs.h"
#include "rtl8651_tblAsicDrv.h"
#include "rtl8651_tblDrv.h"
#endif

#ifdef m_next		
	#undef m_next
	#undef m_len
	#undef m_data
	#undef m_type
	#undef m_flags
	#undef m_pkthdr
	#undef m_ext
#endif
#include "rtl_mbuf.h"

#define RTL_DEV_NAME 			"vl"	/* device name */
#define RTL_DEV_NAME_LEN 		16	

#define ICU_NIC					6
#define TX_DESC_NUM				64
#define RX_DESC_NUM				32
/*
#define DESC_OWNED_BIT			0x00000001
*/
#define DESC_WRAP_BIT			0x00000002
#define UNCACHE_MASK			0x20000000

#define RTL_FLG_PROMISCUOUS_FLAG     0x01
#define RTL_FLG_MEM_ALLOC_FLAG       0x02
#define RTL_FLG_MODE_MEM_IO_MAP	     0x04   /* device registers memory mapped */
#define RTL_FLG_POLLING              0x08	/* polling flag */

#define RTL_MIN_FBUF     100    /* Minimum size of the first buffer in a */

#define IS_BROADCAST(mac)	((mac[0]&mac[1]&mac[2]&mac[3]&mac[4]&mac[5]) == 0xFF)
#define IS_ARP(len)		(len[12]==0x08 && len[13]==0x06)

/* The definition of the driver control structure */

typedef struct rtl_device
    {	
	UINT32		unit;			/* unit index */
    UINT32      flags;			/* Our local flags */
    UINT32		offset;
	UINT32		peripheral;		/* index of interrupt peripheral */
	UINT32		source;			/* index of IRQ source */
	
	UINT32   	vid;            /* VLAN index */
	UINT32   	memberPort;     /* member port mask */
	UINT32   	portNums;     	/* number of member ports */
	UINT32   	mtu;            /* layer 3 mtu */
    UCHAR		enetAddr[6];	/* ethernet address */
	} RTL8650END_DEVICE;

/* Configuration items */
#define ETHERMTU			1500
#define ENET_HDR_REAL_SIZ	14
/*#define RTL_BUFSIZ      (ETHERMTU + ENET_HDR_REAL_SIZ + 6) */
#define RTL_BUFSIZ		 2048
#define RTL_RMD_RLEN     5       /* ring size as a power of 2 -- 32 RMD's */ /*vicadd*/
#define RTL_SPEED        10000000

typedef struct rtl_mBuf 		SNIC_MBLK;
typedef struct rtl_pktHdr	SNIC_PBLK;


/***** LOCAL DEFINITIONS *****/

LOCAL UINT32	txBDhead;	/* index for system to release buffer */
LOCAL UINT32	txBDtail;	/* index for system to set buf to BD */
LOCAL UINT32	rxBDhead;	/* index for system to set buf to BD */
LOCAL UINT32	rxMBDhead;	/* index for system to set mbuf */

LOCAL UINT32	swRxBDc[RX_DESC_NUM];
LOCAL UINT32	swRxMBDc[RX_DESC_NUM];
LOCAL UINT32	swTxBDc[TX_DESC_NUM];
LOCAL SNIC_PBLK	swRxPblkc[RX_DESC_NUM];
LOCAL SNIC_MBLK	swRxMblkc[RX_DESC_NUM];
LOCAL SNIC_PBLK	swTxPblkc[TX_DESC_NUM];
LOCAL SNIC_MBLK	swTxMblkc[TX_DESC_NUM];

LOCAL UINT32	*swRxBD;
LOCAL UINT32	*swRxMBD;
LOCAL UINT32	*swTxBD;
LOCAL SNIC_PBLK	*swRxPblk;
LOCAL SNIC_MBLK	*swRxMblk;
LOCAL SNIC_PBLK	*swTxPblk;
LOCAL SNIC_MBLK	*swTxMblk;

#define SW_INT_MASK	(RX_ERR_IP | TX_ERR_IP | TX_ALL_DONE_IP | RX_DONE_IP)
LOCAL int swMemInitiated = 0;
LOCAL int swStarted = 0;

uint32 linkChangePendingCount = 0;

/* interface which mapping to VLAN */
RTL8650END_DEVICE *pSwIf[RTL8650_LOAD_COUNT];
/* netPool for all port in switch core */


/*#define DEBUG_PACKET*/

/* network buffers configuration */


/***** LOCALS *****/

/* forward static functions */

LOCAL STATUS	rtl8650Reset ();
LOCAL void	    rtl8650Int 			(RTL8650END_DEVICE *);
LOCAL void		rtl8650HandleRecvInt(RTL8650END_DEVICE *);
LOCAL void		rtl8650HandleTxInt	(RTL8650END_DEVICE *);
LOCAL void		rtl8650Restart 		(RTL8650END_DEVICE *);
LOCAL STATUS 	rtl8650RestartSetup (RTL8650END_DEVICE *);
LOCAL void		rtl8650Config 		(RTL8650END_DEVICE *);
LOCAL void		rtl8650AddrFilterSet(RTL8650END_DEVICE *);
LOCAL STATUS 	_rtl8650DrvSend(RTL8650END_DEVICE *, char *,int,unsigned int);
				
/* END Specific interfaces. */


/* This is the only externally visible interface. */


LOCAL STATUS	rtl8650Start 		(RTL8650END_DEVICE *);
LOCAL STATUS	rtl8650Stop 		(RTL8650END_DEVICE *);
LOCAL STATUS	rtl8650Unload 		(RTL8650END_DEVICE *);


/* 
 * Define the device function table.  This is static across all driver
 * instances.
 */

void swNic_resetDescriptors(void)
{
    /* Disable Tx/Rx and reset all descriptors */
    REG32(CPUICR) &= ~(TXCMD | RXCMD);
    return;
}

LOCAL void _rtl8650IramEnable(void)
	{
    __asm__ volatile (	"       .set noreorder\n"
    					"		mtc0	$0, $20\n"
    					"       nop\n"
    					"       nop\n"
    					"       li		$8,0x00000003\n"
    					"       mtc0	$8, $20\n"
    					"       nop\n"
    					"       nop\n"

    					"       mfc0	$8,$12\n"
    					"       nop\n"
    					"       nop\n"
    					"       or		$8,0x80000000\n"
    					"       mtc0	$8,$12\n"
    					"       nop\n"
    					"       nop\n"

    					"       la		$8,1f\n"
                        "       la		$9,0x0ffffc00\n"
                        "       and		$8,$8,$9\n"
                        "       mtc3	$8,$0\n"
                        "       nop\n"
                        "       nop\n"
                        "       addiu	$8,$8,0x1FFF\n"
                        "       mtc3	$8,$1\n"
                        "       nop\n"
                        "       nop\n"

                        "       mtc0	$0, $20\n"
                        "       nop\n"
                        "       nop\n"
                        "       .set reorder\n"
                        "1:"
						);

	}

/* this function conpensate the sw recv dma limite */
LOCAL void swNic_shiftDataForAlign(char *pData, int len)
{
short *pNewOff = (short*)((UINT32)pData & 0xFFFFFFFC);
short *pOldOff = (short*)pData;
	if(pNewOff == pData)
		return;
	len = (len+1)/2;
	while(len--)
	{
		pOldOff[len] = pNewOff[len];
	}
}

/*******************************************************************************
*
* rtl8650Int - handle controller interrupt
*
* This routine is called at interrupt level in response to an interrupt from
* the controller.
*
* RETURNS: N/A.
*/
int pktout_cnt = 0;
int mbufout_cnt = 0;

LOCAL void rtl8650Int
    (
    RTL8650END_DEVICE  *pDrvCtrl
    )
    {
    UINT32        stat;
	int int_count=0;
	int RecvFlag = FALSE;
		
	/*
	if((GISR & NIC_IP) == 0)
		 return;
	*/

	/* Disable controller interrupts. */
	/*REG32(CPUIIMR) = 0x00;*/

	for (;;)
		{
   		/* Read the interrupt status register */
    	stat = REG32(CPUIISR);
    	/* clear interrupts, */
		REG32(CPUIISR) = stat;

    	/* Check if a valid Interrupt has been set */
		if ((stat & (INTPENDING_NIC_MASK|LINK_CHANG_IP)) == 0)
			break;

		if(stat & PKTHDR_DESC_RUNOUT_IP)
			pktout_cnt++;
		if(stat & MBUF_DESC_RUNOUT_IP)
			mbufout_cnt++;
		/* Check for Receive Complete */
		if (stat & RX_DONE_IP)
			{
			}

		/* Check for transmit Interrupt */
		if (stat & TX_DONE_IP)
			{
        	rtl8650HandleTxInt (pDrvCtrl);
			}

		/* Check and handle link change interrupt */
		if (stat & LINK_CHANG_IP) {
			linkChangePendingCount++;
		}

		if(int_count++>5)
			break;	
		}
    /*REG32(CPUIIMR) = INTPENDING_NIC_MASK;*/

	}



/*******************************************************************************
*
* rtl8650HandleTxInt - task level interrupt service for tx packets
*
* This routine is called by the interrupt service routine to do any 
* message transmission processing.
*
* RETURNS: N/A.
*/

LOCAL void rtl8650HandleTxInt
    (
    RTL8650END_DEVICE *pDrvCtrl
    )
	{
	char *pBuf;
	/*printf("rt tx int");*/
	while((swTxBD[txBDhead] & DESC_OWNED_BIT) == 0 && \
				txBDhead != txBDtail)
		{
		/* freebuffer */
		pBuf = (char*)swTxMblk[txBDhead].m_data;
		pBuf -= pDrvCtrl->offset;
		swTxPblk[txBDhead].ph_len = 0;
		swTxMblk[txBDhead].m_data = 0;
		swTxMblk[txBDhead].m_len = 0;
		swTxMblk[txBDhead].m_flags = 0;
		txBDhead++;
		txBDhead %= TX_DESC_NUM;
		}
	}


/*******************************************************************************
*
* rtl8650HandleRecvInt - task level interrupt service for input packets
*
* This routine is called by the interrupt service routine to do any 
* message received processing.
*
* RETURNS: N/A.
*/


LOCAL void rtl8650HandleRecvInt
    (
    RTL8650END_DEVICE *pDrvCtrl
    )
    {

    }


STATUS _rtl8650DrvSend
	(
    RTL8650END_DEVICE	*pDrvCtrl,	/* device ptr */
    char	*pBuf,
    int len,
    unsigned int	portlist
	)
{
    char*       pBuf2;
    int         s;	
	int			i;


	swTxMblk[txBDtail].m_next = 0;
	swTxMblk[txBDtail].m_len = len;
	swTxMblk[txBDtail].m_flags = 0x9c;
	swTxMblk[txBDtail].m_data = (char*)((uint32)pBuf2 | UNCACHE_MASK);
	/* trigger to send */
	swTxPblk[txBDtail].ph_len = len;
	swTxPblk[txBDtail].ph_flags = 0x8800;
	/* get output ports */
	swTxPblk[txBDtail].ph_portlist = portlist;
	/**/
	(UINT32)swTxBD[txBDtail] |= DESC_OWNED_BIT;
	REG32(CPUICR) |= TXFD;
	/* advance one */
	txBDtail++;
	txBDtail %= TX_DESC_NUM;
 
	
	return (0);
}

/***********************************************************************
*
* rtl8650Send - the driver send routine
*
* RETURNS: 0 or ERROR.
*/
 
LOCAL STATUS rtl8650Send
    (
    RTL8650END_DEVICE	*pDrvCtrl,	/* device ptr */
    char	*pMblk		/* data to send */
    )
    {
    int         len = 0;
    char*       pBuf;
    char*		pBuf2;
    int         s;	


	swTxMblk[txBDtail].m_next = 0;
	swTxMblk[txBDtail].m_len = len;
	swTxMblk[txBDtail].m_flags = 0x9c;
	swTxMblk[txBDtail].m_data = (char*)((uint32)pBuf | UNCACHE_MASK);
	/* trigger to send */
	swTxPblk[txBDtail].ph_len = len;
	swTxPblk[txBDtail].ph_flags = 0x8800;
	/* get output ports */
	if(pDrvCtrl->portNums>1)
#ifndef RTL8650_ONLY /* Use high-level driver */
		swTxPblk[txBDtail].ph_portlist = pDrvCtrl->memberPort & asicL2_output(pBuf);
#else
		swTxPblk[txBDtail].ph_portlist = pDrvCtrl->memberPort & swCore_layer2TableGetPortByMac(pBuf);
#endif		
	else
		swTxPblk[txBDtail].ph_portlist = pDrvCtrl->memberPort;
	/**/
	(UINT32)swTxBD[txBDtail] |= DESC_OWNED_BIT;
	REG32(CPUICR) |= TXFD;
	/* advance one */
	txBDtail++;
	txBDtail %= TX_DESC_NUM;

    return (0);
 
    }  


/*******************************************************************************
*
* rtl8650EndLoad - initialize the driver and device
*
* This routine initializes the driver and the device to the operational state.
* All of the device-specific parameters are passed in <initString>, which
* expects a string of the following format:
*
* <unit>:<vlan_id>:<member_port>:<mtu>:<mac>
*
* This routine can be called in two modes. If it is called with an empty but
* allocated string, it places the name of this device (that is, "rtl") into 
* the <initString> and returns 0.
*
* If the string is allocated and not empty, the routine attempts to load
* the driver using the values specified in the string.
*
* RETURNS: An END object pointer, or NULL on error, or 0 and the name of the
* device if the <initString> was NULL.
*/


/*******************************************************************************
*
* rtl8650InitMem - initialize memory for the device
*
* Using data in the control structure, setup and initialize the memory
* areas needed.  If the memory address is not already specified, then allocate
* cache safe memory.
*
* RETURNS: 0 or ERROR.
*
*/

LOCAL STATUS rtl8650InitMem
    (
    RTL8650END_DEVICE * 	pDrvCtrl	/* device to be initialized */
    )
{ 
	int ix;
	swRxBD = (UINT32*)((UINT32)swRxBDc|UNCACHE_MASK);
	swRxMBD = (UINT32*)((UINT32)swRxMBDc|UNCACHE_MASK);
	swTxBD = (UINT32*)((UINT32)swTxBDc|UNCACHE_MASK);
	swRxPblk = (SNIC_PBLK*)((UINT32)swRxPblkc|UNCACHE_MASK);
	swRxMblk = (SNIC_MBLK*)((UINT32)swRxMblkc|UNCACHE_MASK);
	swTxPblk = (SNIC_PBLK*)((UINT32)swTxPblkc|UNCACHE_MASK);
	swTxMblk = (SNIC_MBLK*)((UINT32)swTxMblkc|UNCACHE_MASK);
	/* init xmt BD */
	for(ix=0;ix<TX_DESC_NUM;ix++)
	{
		swTxBD[ix] = ((UINT32)&swTxPblk[ix] | UNCACHE_MASK);
		swTxPblk[ix].ph_mbuf = (void*)((UINT32)&swTxMblk[ix] | UNCACHE_MASK);
		swTxMblk[ix].m_pkthdr = (void*)((UINT32)&swTxPblk[ix] | UNCACHE_MASK);
	}
	(UINT32)swTxBD[TX_DESC_NUM-1] |= DESC_WRAP_BIT;

	txBDhead = txBDtail = 0;

	return 0;
    }

/*******************************************************************************
*
* rtl8650Start - start the device
*
* This function initializes the device and calls BSP functions to connect
* interrupts and start the device running in interrupt mode.
*
* The complement of this routine is rtl8650Stop.  Once a unit is reset by
* rtl8650Stop, it may be re-initialized to a running state by this routine.
*
* RETURNS: 0 if successful, otherwise ERROR
*/

LOCAL STATUS rtl8650Start		
    (
    RTL8650END_DEVICE *	pDrvCtrl
    )
    {

	/*printf("rt start\n");    	*/
	/* init NIC */

	if(swStarted)
		return 0;

    /* Enable Rx & Tx. Config bus burst size and mbuf size. */
    REG32(CPUICR) = TXCMD | RXCMD | BUSBURST_256WORDS | MBUF_2048BYTES;


    /* Enable interrupts */
    /*
    REG32(CPUIIMR) |= (RX_ERR_IE | TX_ERR_IE | PKTHDR_DESC_RUNOUT_IE | MBUF_DESC_RUNOUT_IE | 
            TX_ALL_DONE_IE | RX_DONE_IE);
    */
    REG32(CPUIIMR) = (INTPENDING_NIC_MASK|LINK_CHANG_IP);
	swStarted = 1;
	
    return (0);
    }



	  
/*******************************************************************************
*
* rtl8650Ioctl - interface ioctl procedure
*
* Process an interface ioctl request.
*
* This routine implements the network interface control functions.
* It handles EIOCSADDR, EIOCGADDR, EIOCSFLAGS, EIOCGFLAGS, EIOCMULTIADD,
* EIOCMULTIDRTL, EIOCMULTIGET, EIOCPOLLSTART, EIOCPOLLSTOP, EIOCGMIB2 commands.
*
* RETURNS: 0 if successful, otherwise EINVAL.
*/

LOCAL int rtl8650Ioctl
    (
    RTL8650END_DEVICE *	pDrvCtrl,
    int		cmd
    )
    {
    int error = 0;

    return (error);
    }



/*******************************************************************************
*
* rtl8650Reset - Reset the chip
*
* This routine does a soft reset on the chip.
*
* RETURNS: 0, Always. 
*/

LOCAL STATUS rtl8650Reset
    (
    RTL8650END_DEVICE *	pDrvCtrl	/* pointer to device control structure */
    )
    {
    /* issue the reset command */
	/*NIC_CNR |= RESET;*/

    return 0;
    }
   

/*******************************************************************************
*
* rtl8650Stop - stop the device
*
* This routine marks the interface as down and resets the device.  This
* includes disabling interrupts, stopping the transmitter and receiver.
*
* The complement of this routine is rtl8650Start.  Once a unit is
* stop in this routine, it may be re-initialized to a running state by
* rtl8650Start.
*
* RETURNS: 0 or ERROR
*/

LOCAL STATUS rtl8650Stop
    (
    RTL8650END_DEVICE *	pDrvCtrl
    )
    {

	/* Disable interrupts by clearing the interrupt mask. */
    REG32(CPUIIMR) = 0x00;

	/* Stop the chip's Tx and Rx DMA processes. */
    REG32(CPUICR) &= ~(TXCMD | RXCMD);

	return 0;
    }



/******************************************************************************
*
* rtl8650AddrFilterSet - set the address filter for multicast addresses
*
* This routine goes through all of the multicast addresses on the list
* of addresses (added with the rtl8650AddrAdd() routine) and sets the
* device's filter correctly.
*
* NOMANUAL
*/
#if 0
LOCAL void rtl8650AddrFilterSet
    (
    RTL8650END_DEVICE *pDrvCtrl
    )
    {

    ETHER_MULTI* 	pCurr;
    rtl_ib*			pIb;
    u_char*			pCp;
    u_char 			c;
    u_long 			crc;
    u_long 			carry;
	int				i, j;


    pIb = pDrvCtrl->ib;

    RTL_ADDRF_CLEAR (pIb);

    pCurr = END_MULTI_LST_FIRST (&pDrvCtrl->end);

    while (pCurr != NULL)
		{

		pCp = (unsigned char *)&pCurr->addr;
		crc = 0xFFFFFFFF; /* initial value */

		for (i = 0; i < 6; i++) 
			{
			c = *(pCp + i);
			for (j = 0; j < 8; j++) 
				{
				carry = ((crc & 0x80000000) ? 1 : 0) ^ (c & 0x01);
				crc <<= 1;
				c >>= 1;
				if (carry)
					crc = (crc ^ 0x04c11db6) | carry;
				}
			}

		/* Just want the 6 most significant bits. */

		crc = crc >> 26;

		/* Turn on the corresponding bit in the filter. */

		RTL_ADDRF_SET (pIb, crc);

		pCurr = END_MULTI_LST_NEXT(pCurr);
		}
    }
#endif

/*******************************************************************************
*
* rtl8650RestartSetup - setup memory descriptors and turn on chip
*
* Initializes all the shared memory structures and turns on the chip.
*/

LOCAL STATUS rtl8650RestartSetup
    (
    RTL8650END_DEVICE *pDrvCtrl
    )
    {

    /* reset the device */

    rtl8650Reset (pDrvCtrl);

    /* reconfigure the device */

    rtl8650Config (pDrvCtrl);

    return (0); 
    }
     
/*******************************************************************************
*
* rtl8650Restart - restart the device after a fatal error
*
* This routine takes care of all the messy details of a restart.  The device
* is reset and re-initialized.  The driver state is re-synchronized.
*/

LOCAL void rtl8650Restart
    (
    RTL8650END_DEVICE *pDrvCtrl
    )
    {

    rtl8650Reset (pDrvCtrl);

    rtl8650RestartSetup (pDrvCtrl);

    /* set the flags to indicate readiness */

    }


/******************************************************************************
*
* rtl8650Config - reconfigure the interface under us.
*
* Reconfigure the interface setting promiscuous mode, and changing the
* multicast interface list.
*
* NOMANUAL
*/

LOCAL void rtl8650Config
    (
    RTL8650END_DEVICE *pDrvCtrl
    )
    {

    /* Set promiscuous mode if it's asked for. */


    }

/*******************************************************************************
*
* rtl8650Unload - unload a driver from the system
*
* This routine deallocates lists, and free allocated memory.
*
* RETURNS: 0, always.
*/

LOCAL STATUS rtl8650Unload
    (
    RTL8650END_DEVICE *	pDrvCtrl
    )
    {


    return (0);
    }


#ifndef RTL8650_ONLY /* use highe level driver */
unsigned int asicL2_output(char *pPDU)
{
	char cpu, srcBlk, isStatic;
	unsigned int i, mbr, ageSec;
	ether_addr_t Mac;
	unsigned char *mac = (unsigned char *)pPDU;
	unsigned int l2hash = mac[0]^mac[1]^mac[2]^mac[3]^mac[4]^mac[5];
	
	for(i=0; i<RTL8651_L2TBL_COLUMN; i++) {
		/*
		if (rtl8651_getAsicL2Table(l2hash, i, &Mac, &cpu, &srcBlk, &isStatic, &mbr, &ageSec, NULL) == FAILED)
			continue;*/
		if(Mac.octet[5]==mac[5])
			break;
	}
	if(i==RTL8651_L2TBL_COLUMN)
		return 0xffffffff;
	else
		return(mbr);
}
#ifdef L3_ACCELERATION
int asicL3AddArp(char *pPDU, int unit, unsigned int port)
{
	char ifName[RTL_DEV_NAME_LEN];
	ether_addr_t *mac = (ether_addr_t *)&pPDU[22];
	ipaddr_t *ipAddr = (ipaddr_t*)&pPDU[28];

	sprintf(ifName, "vl%d", unit);	
	rtl8651_addArp(*ipAddr, mac, ifName, port);
}
#endif
#endif
