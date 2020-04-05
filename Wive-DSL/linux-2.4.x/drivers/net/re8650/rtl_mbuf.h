
/*
* Copyright c                  Realtek Semiconductor Corporation, 2002
* All rights reserved.
*
* Program : The mbuf module header file
* Abstract :
* Author : David Chun-Feng Liu (cfliu@realtek.com.tw)
* $Id: rtl_mbuf.h,v 1.1.1.1 2003/08/18 05:43:22 kaohj Exp $
* $Log: rtl_mbuf.h,v $
* Revision 1.1.1.1  2003/08/18 05:43:22  kaohj
* initial import into CVS
*
* Revision 1.1  2003/05/23 06:05:53  elvis
* initial version
*
* Revision 1.69  2003/05/09 11:06:31  cfliu
* Add comments and some code for 8651B
* define new flag bits for 8651B
*
* Revision 1.68  2003/05/06 08:10:44  cfliu
* Redefine ph_proto field in pktHdr structure. Define bit fields to avoid manual shift operations.
*
* Revision 1.67  2003/05/02 09:00:22  cfliu
* Add mBuf_mutexLock/UnLock. used when  POSIX interface not included in release package.
*
* Revision 1.66  2003/04/17 11:40:26  cfliu
* Rename some symbols...
*
* Revision 1.65  2003/04/16 10:30:36  cfliu
* Global renaming for mbuf API and symbols
*
* Revision 1.64  2003/03/11 05:49:05  hiwu
* add FRAG_MBUF_DONE
*
* Revision 1.63  2003/02/20 07:46:22  cfliu
* Remove mbuf type MT_CONTROL so MT_TYPES=2 now
*
* Revision 1.62  2003/02/18 10:21:58  cfliu
* Move hostOffsetMac into datalink union. Delete datalinkTx union
*
* Revision 1.61  2003/02/18 03:18:23  hiwu
* add NAT_MBUF_DONE
*
* Revision 1.60  2003/02/10 01:36:17  hiwu
* change IPSEC_DONE to IPSEC_MBUF_DONE
* change L2TP_DONE  to L2TP_MBUF_DONE
* change PPTP_DONE to PPTP_MBUF_DONE
*
* Revision 1.59  2003/01/28 13:35:26  hiwu
* remove
* #define PROCESS_NAT  0x01
* #define PROCESS_L2TP 0x02
* #define PROCESS_IPSEC 0x04
* #define PROCESS_PPTP 0x08
* #define PROCESS_FORWARD_IN 0x10
* #define PROCESS_FORWARD_OUT 0x20
*
* Revision 1.58  2003/01/14 08:42:07  cfliu
* Remove unused pkthdr bits PKT_LASTFRAG, PKT_FRAG, and PKT_INETHOLD
*
* Revision 1.57  2002/12/31 09:38:29  hiwu
* add
* #define PROCESS_NAT  0x01
* #define PROCESS_L2TP 0x02
* #define PROCESS_IPSEC 0x04
* #define PROCESS_PPTP 0x08
* #define PROCESS_FORWARD_IN 0x10
* #define PROCESS_FORWARD_OUT 0x20
* #define m_process_flags	m_reserved[1]
*
* Revision 1.56  2002/11/06 02:36:54  elvis
* add struct datalinkTx for pkthdr
*
* Revision 1.55  2002/11/05 09:28:46  cfliu
* Add ALLOCPKTHDR for m_cloneMbuChain and m_dupMufChain
* Update document for mBuf_adjHead,mBuf_trimHead,mBuf_trimTail,mBuf_cloneMbufChain,m_dupMbufChian,
* mBuf_clonePacket,mBuf_dupPacket
* Remove m_checkSum16
*
* Revision 1.54  2002/11/01 08:52:14  cfliu
* Fix compile warnings on Greenhill compiler
*
* Revision 1.53  2002/10/25 10:31:14  cfliu
* Add comment for mBuf_copyToMbuf()
*
* Revision 1.52  2002/10/23 04:31:13  cfliu
* Change m_data, m_extbuf datatype from int8* to uint8*
* Add mBuf_getBMjmpTable & mBuf_BMpatternMatch to support pattern matching in mbuf
*
* Revision 1.51  2002/10/15 06:45:14  cfliu
* Fix mBuf_driverFreePkthdr prototype problem
*
* Revision 1.50  2002/10/14 08:05:29  cfliu
* Update mbuf documentation
*
* Revision 1.49  2002/10/14 03:01:04  hiwu
* remove M_ZERO
*
* Revision 1.48  2002/10/12 07:21:36  cfliu
* Update m_driverXXX APIs
*
* Revision 1.47  2002/10/12 06:29:34  henryCho
* Move header next field to driver reserved field
*
* Revision 1.46  2002/10/12 05:39:15  cfliu
* Move ph_nextHdr to the first word of pkthdr. That is, share the same 4-byte space with
* ph_nextfree and ph_muf.
*
* Revision 1.45  2002/10/11 03:25:45  cfliu
* Add document for m_clone{Packet|MbufChain} and m_dup{Packet|MbufChain}
*
* Revision 1.44  2002/10/11 03:15:43  elvis
* change trunk to etherChannel
*
* Revision 1.43  2002/10/11 02:45:23  hiwu
* add #define M_ZERO
* add mCloneMbufChain()
* add mDupMbufChain()
*
* Revision 1.42  2002/10/06 09:19:26  jzchen
* Add driver field
*
* Revision 1.41  2002/10/01 05:15:52  henryCho
* Change proto-type
*
* Revision 1.39  2002/09/26 05:29:22  henryCho
* Modify callback field type of pkthdr structure.
*
* Revision 1.37  2002/09/24 01:31:37  henryCho
* Modify M_EOR definition.
*
* Revision 1.36  2002/09/20 07:35:58  elvis
* add mBuf_padding()
*
* Revision 1.35  2002/09/13 06:33:43  cfliu
* Add ph_portIndex in pkthdr so trunking port id can be passed to L4
*
* Revision 1.34  2002/08/23 06:04:20  danwu
* Add field definitions of struct rtl_pktHdr.
*
* Revision 1.33  2002/08/21 10:19:53  hiwu
* #define m_aux_mem_index 	m_reserved[0]
* for IPSEC support
*
* Revision 1.32  2002/08/21 06:41:14  danwu
* Add pkthdr field defintions.
*
* Revision 1.31  2002/08/21 05:28:54  cfliu
* Remove all _iMbuf*() functions and remove all m_getPkt*() functions to mbuf.h.
* m_getPkt*() functions are now declared as macros.
* Add documentation for all exported macros.
*
* Revision 1.30  2002/08/20 05:51:30  danwu
* Change to RTL_EXTERN_INLINE & RTL_STATIC_INLINE macros.
*
* Revision 1.28  2002/08/20 01:41:49  danwu
* Change inline to __inline__.
*
* Revision 1.27  2002/08/15 04:27:52  hiwu
* remove ph_ch_ports
*
* Revision 1.26  2002/08/13 04:38:28  cfliu
* Remove MHLEN and MCLBYTES, update mBuf_pullup API document.
*
* Revision 1.25  2002/08/12 11:31:54  cfliu
* Revise all Flags in all structures and removed unused ones.
*
* Revision 1.24  2002/08/12 05:50:29  hiwu
* merge ph_callback in pkthdr
*
* Revision 1.23  2002/08/05 08:42:33  cfliu
* Update mBuf_trimHead & mBuf_trimTail documentation.
*
* Revision 1.22  2002/08/02 02:48:59  cfliu
* Add M_GETNEWMBUF, MBUF_RESERVE, and changed MBUF_ALIGN
*
* Revision 1.21  2002/07/26 01:49:56  cfliu
* Removed mbuf #defines such as NMBUFS,... and add new fields in struct rtl_mBufStatus
*
* Revision 1.20  2002/07/24 05:53:40  hiwu
* remove m_procflags
* change rcvch_ports to ports
* change rcvch_index to ch_index
*
* Revision 1.19  2002/07/02 09:00:46  cfliu
* Remove unused #defines
*
* Revision 1.18  2002/06/28 06:24:54  hiwu
* remove nested comment that make compiler parser error
*
* Revision 1.17  2002/06/28 05:45:33  hiwu
*
* add	 m_procflags  //// for nat, l2tp, pptp, ipsec
* add #define	NAT_PASS 0x1
* add #define	FIREWALL_PASS 0x02
* add #define	IPSEC_PASS 0x04
* add #define	L2TP_PASS 0x08
* remark int8      m_reserved[2]
*
* Revision 1.15  2002/06/19 09:37:18  cfliu
* Remove some unused macro definitions and some mbuf types.
*
* Revision 1.14  2002/06/13 09:06:41  cfliu
* Add msgLog support
*
* Revision 1.13  2002/06/12 09:31:13  cfliu
* Modified to allow variable cluster size specified by user during runtime.
* However, those constants defined in section one shoule NOT be used anymore.
*
* Revision 1.11  2002/06/05 03:43:27  cfliu
* Add mbstat structure and a new function mBuf_getBufStat() for user to get mbuf module status
*
* Revision 1.10  2002/05/22 05:44:12  hiwu
* add
* #define ph_proto_header		u.network.header
* #define	ph_nextpkt_hdr		u.network.nextpkthdr
*
* Revision 1.9  2002/05/16 05:22:03  cfliu
* 1.Redesign pkthdr structure. ASIC irrevelant fields in pkthdr are now grouped with union structure so these
* fields may be reused by different software modules.
* 2.m_getNextpkt() and m_setNextpkt() removed.
*
*
* Revision 1.5  2002/05/15 08:40:57  cfliu
* Add mBuf_freePkthdr() so pkthdr could be freed alone.
* Note this is not the normal way to free a pkthdr attached with mbuf chains.
*
* Revision 1.4  2002/05/15 05:59:57  cfliu
* 1.Add m_getPkthdr() so pkthdrs could be allocated alone
* 2.Remove unused MBUF_WAIT flag.
*
* Revision 1.3  2002/05/13 11:33:12  hiwu
* make the codes like FreeBSD4.4 as possible
*
* Revision 1.2  2002/04/29 10:07:35  hiwu
* add  " *ph_rcvif" and  "ph_rcvch_index"
*
* Revision 1.13  2002/04/29 07:56:02  hiwu
* add "ph_revif " and "ph_rcvch_index" in mbuf structure
*
* Revision 1.9  2002/04/16 07:59:19  cfliu
* modify type of struct rtl_pktHdr: ph_nextpkt
*
* Revision 1.8  2002/04/11 10:19:16  cfliu
* Add m_checkSum16() and declare mBuf_get/setXXX functions as inline function.
*
* Revision 1.7  2002/04/10 14:35:34  cfliu
* add APIs to set/get pkthdr structure.
*
* Revision 1.6  2002/04/08 08:11:51  hiwu
* mark void     *ph_rcvif;
* add   int32     ph_rcvif_index;
*
* Revision 1.5  2002/04/04 10:47:29  cfliu
* Use 'indent' tool to unify coding style....
* The options used could be found on /product/cygtest/.indent.pro
*
* Revision 1.2  2002/03/21 05:43:50  cfliu
* Primitive data types changed to uint32, uint16, ...etc.
*
*/
#ifndef _MBUF_H_
#define	_MBUF_H_

#include"rtl_types.h"

/*********************************************************************************
	SECTION 1:		mbuf module default settings
**********************************************************************************/
/*********************************************************************************
	SECTION 2:		mbuf module data structure definitions
**********************************************************************************/
#define 	BUF_FREE			0x00   /* Buffer is Free  */
#define 	BUF_USED			0x80   /* Buffer is occupied */
#define 	BUF_ASICHOLD		0x80   /* Buffer is hold by ASIC */
#define 	BUF_DRIVERHOLD 	0xc0	   /* Buffer is hold by driver */

/*@struct m_buf | The mbuf header associated with each cluster */
struct rtl_mBuf
{
	/*
	   m_next and m_pkthhr should be at the first two elements. _mget, _pMbufChainCopy relies on this
	 */
	struct rtl_mBuf *m_next;
		#define m_nextfree	m_next;
	struct rtl_pktHdr *m_pkthdr;		   /* Points to the pkthdr structure */

	uint16    m_len;				   /* data bytes used in this cluster */
	int8      m_flags;				   /*   mbuf flags; see below */
		#define 	MBUF_FREE			BUF_FREE   /* Free. Not occupied. should be on free list   */
		#define	MBUF_USED			BUF_USED	   /* Buffer is occupied */
		#define	MBUF_EXT			0x10		   /* has associated with an external cluster, this is always set. */
		#define   MBUF_PKTHDR		0x08		   /* is the 1st mbuf of this packet */
		#define 	MBUF_EOR			0x04		   /* is the last mbuf of this packet. Set only by ASIC*/
		/*
		   (TBD) Any more flags?
		 */

	int8      m_type;				   /*  CHANGED: Type of data of this mbuf. Shrinks to 1 byte.  */
		/*
		   These are traditional 4BSD mbuf types
		 */
		#define 	MBUFTYPE_FREE		0x00		   /*Free, unused buffer */
		#define	MBUFTYPE_DATA		0x01			   /*dynamic (data) allocation */
		#define	MBUFTYPES		0x02		   /*Total number of mbuf types for mbtypes[] */

	uint8     *m_data;				   /*  location of data in the cluster */
	/*
	   In our project, all mbufs store data in clusters. So embed m_ext structure in m_hdr
	 */
	uint8     *m_extbuf;				   /* start of buffer*/
	void      (*m_extfree) (uint8 *, uint32);	/*  cluster free routine */
	void      (*m_extref) (uint8 *, uint32);	/*  cluster reference counting function */
	uint16    m_extsize;			   /* sizeof the cluster */
	
	uint8 m_process_flags;
		/* bit-wise */
		#define NAT_MBUF_DONE 0x01
		#define IPSEC_MBUF_DONE 0x02
		#define L2TP_MBUF_DONE 0x04
		#define PPTP_MBUF_DONE 0x08
		#define FRAG_MBUF_DONE 0x10 /* mbuf contain ip fragment packet */

	int8      m_reserved[1];	

};


struct ifnet;

/*@struct rtl_pktHdr |  pkthdr records packet specific information. Each pkthdr is exactly 32 bytes.
 first 20 bytes are for ASIC, the rest 12 bytes are for driver and software usage.
 */
struct rtl_pktHdr
{
	/*
	   This union should be at the first element. _mget relies on this
	 */
	/*Byte 0-3 */
	union
	{
		struct rtl_pktHdr *pkthdr_next;	   /*  next pkthdr in free list */
		struct rtl_mBuf *mbuf_first;	   /*  1st mbuf of this pkt */
	}PKTHDRNXT;
	#define	ph_nextfree		PKTHDRNXT.pkthdr_next
	#define	ph_mbuf		PKTHDRNXT.mbuf_first

	/*Byte 4-7*/
	uint16    ph_len;				   /*   total packet length */
	uint16    ph_flags;				   /*  NEW:Packet header status bits */
		#define PKTHDR_FREE			(BUF_FREE << 8)	/* Free. Not occupied. should be on free list   */
		#define PKTHDR_USED			(BUF_USED << 8)
		#define PKTHDR_DRIVERHOLD 	(BUF_DRIVERHOLD<<8)	/* Hold by driver */
		#define PKTHDR_CPU_OWNED   0x4000
		#define PKT_INCOMING		0x1000	   /* Incoming: packet is incoming */

		#define PKT_OUTGOING		0x0800	   /*  Outgoing: packet is outgoing */
		#define PKTHDR_RESERVED1	0x0400		/* Reserved bit */
		#define PKTHDR_RESERVED2	0x0200		/* Reserved bit */
		#define PKT_BCAST			0x0100	   /*send/received as link-level broadcast  */

		#define PKT_MCAST			0x0080	   /*send/received as link-level multicast   */
#ifdef RTL8651B
		#define PKTHDR_HWLOOKUP	0x0040	   /*valid when ph_extPortList!=0. 1:Hardware table lookup assistance*/
		#define PKTHDR_BRIDGING		0x0060	   /*when PKTHDR_HWLOOKUP is on. 1: Hardware assist to do L2 bridging only, 0:hardware assist to do NAPT*/
#endif
		#define PKTHDR_HPRIORITY    	0x0010     /* High priority */

		#define PKTHDR_PPPOE_AUTOADD    0x0008     /* PPPoE header auto-add */
		#define PKTHDR_VLAN_AUTOADD     0x0004     /* VLAN tag auto-add */
		#define CSUM_IP_OK			0x0002	   /* Incoming: IP header cksum has checked */
		#define CSUM_L4_OK			0x0001	   /*Incoming:TCP/UDP/ICMP cksum checked */
		#define CSUM_IP				0x0002	    /* Outgoing: IP header cksum offload to ASIC*/
		#define CSUM_L4		    		0x0001	   /*Outgoing:TCP/UDP/ICMP cksum offload to ASIC*/

	/*Byte 8-11*/
//	uint16    ph_proto;	
#if 0 /* _LITTLE_ENDIAN */
	uint16	ph_reserved2:4;
	uint16	ph_vlanIdx:3;
	uint16	ph_pppoeIdx:3;
	uint16	ph_pppeTagged:1;
	uint16	ph_LLCTagged:1;
	uint16	ph_vlanTagged:1;
	uint16	ph_proto:3;
#else
	uint16	ph_proto:3;
	uint16	ph_vlanTagged:1;
	uint16	ph_LLCTagged:1;
	uint16	ph_pppeTagged:1;
	uint16	ph_pppoeIdx:3;
	uint16	ph_vlanIdx:3;
	uint16	ph_reserved2:4;
#endif
		#define PKTHDR_ETHERNET      0
		#define PKTHDR_IP               	2
		#define PKTHDR_ICMP             	3
		#define PKTHDR_IGMP             	4
		#define PKTHDR_TCP              	5
		#define PKTHDR_UDP              	6

	uint16    ph_reason;			   /*   NEW: (TBD) indicates why the packet is received by CPU */
	/*
	   (TBD) any more flags ??
	 */

	/*Byte 12-15*/
#ifdef RTL8651B
	uint16	ph_extPortList;		/*extension port list. meaningful only to driver. */
#else
	uint16	ph_reserved3;	/*reserved, must be zero */
#endif
	uint16    ph_portlist;			   /*  Physical Incoming port id / Physical Outgoing Port list */


	/*Byte 16-19: Reserved for ASIC */
	uint32	ph_reserved;

	/*Byte 20-31: Interpreted by software, layer dependent.
	   Be careful don't use more than 12 bytes.
	*/
	union{

		struct {
			void    (*callback)(void *, uint32);		/* for driver call back function */
			struct rtl_pktHdr *nextHdr;	/*  next packet in queue/record */
			uint32  mbufCount;          /* for mbuf flow control */
		} driver;

		struct {
			uint32	portIndx;		   /*Input: logical port index, assigned in etherChannel_input().  */
									/*Output: logical port mask  */
			uint32	ch_index;	   /* channel index, assigned in vlan_input() */
			uint32	hostMacOffset;	/* offset from the base host mac address, used by the caller of vlan_output */
		}datalink;

		struct {
			struct ifnet   *rcvif;      /*  rcv interface , TBD: should be casted to (struct ifnet *) */
			void	*header;			/* pointer to packet header of ip packet. for ip reassembly */
			struct rtl_pktHdr *nextpkthdr;	/*  next packet in queue/record */
		}network;

		struct {
			struct rtl_pktHdr *nextpkt;	  /*  Moved from mbuf. next packet in queue/record */
			int32 reserved;
			void *ctrl;		/*attach control info here. Currently only UDP use this field for incoming packets */

		} transport;
	}u;

#define ph_callback	u.driver.callback
#define ph_mbufcount	u.driver.mbufCount
#define ph_nextHdr	u.driver.nextHdr
#define ph_portIndex	u.datalink.portIndx
#define ph_ch_index		u.datalink.ch_index
#define ph_hostMacOffset	u.datalink.hostMacOffset
#define ph_rcvif			u.network.rcvif
#define ph_proto_header	u.network.header
#define ph_nextpkt_hdr		u.network.nextpkthdr
#define ph_nextpkt		u.transport.nextpkt
#define ph_ctrl			u.transport.ctrl
};

struct rtl_mBufStatus
{
	uint32 	m_totalmbufs;	//Total mbufs allocated during initialization
	uint32	m_totalclusters;	//Total clusters allocated during initialization
	uint32	m_totalpkthdrs;	//Total pkthdrs allocated during initialization
	uint32    m_freembufs;	/* free mbufs in pool now*/
	uint32    m_freeclusters;	/* free clusters in  pool now*/
	uint32	m_freepkthdrs;	/* free pkthdrs in pool now*/
	uint32    m_msize;		/* length of an mbuf */
	uint32    m_mclbytes;		/* length of an mbuf cluster */
	uint32 	m_pkthdrsize;	/* length of an pkthdr */

	uint32    m_wait;			/* times waited for space, includes mbuf, pkthdr and cluster */
};


/*********************************************************************************
	SECTION 3:		mbuf module exported variables, symbols and macros
**********************************************************************************/
#define	MBUF_COPYALL		1000000000	   /* length for m_copy to copy all */
#define	MBUF_WAITOK		0x01
#define	MBUF_DONTWAIT	0x02	   /* Don't wait if there is no buffer available */
#define	MBUF_ONLY	0x04	   /* Don't allocate a cluster in mBuf_get */
#define	MBUF_ALLOCPKTHDR	0x08	   /* Allocate a packet header with mbuf chain in mBuf_getm, mBuf_cloneMbufChain, mBuf_dupMbufChain*/
#define	MBUF_GETNEWMBUF 0x10		   /* In mBuf_prepend, alloate new mbufs directly */
#define MBUF_CHECKPKTHDR(m)	((m)&&(ISSET((m)->m_flags, MBUF_USED) && ((m)->m_pkthdr))?1:0)
#define MBUF_GETPKTHDRFIELD16(field)  (*((uint16 *)(field)))
#define MBUF_SETPKTHDRFIELD16(field, value)	*((uint16 *)(field)) = (value)

//The size of each cluster.
extern uint32 m_clusterSize;

/*********************************************************************************
	SECTION 4:		mbuf module exported API prototype
**********************************************************************************/

/* mbuf module exported APIs */

/*	@doc MBUF_API

	@module mbuf.h - Mbuf module API documentation	|
	This document illustrates the API interface of the mbuf module.
	@normal Chun-Feng Liu (cfliu@realtek.com.tw) <date>

	Copyright <cp>2001 Realtek<tm> Semiconductor Cooperation, All Rights Reserved.

 	@head3 List of Symbols |
 	Here is a list of all functions and variables in this module.

 	@index | MBUF_API
*/
extern int32 mBuf_init(uint32, uint32, uint32, uint32, uint32);			   //was tunable_mbinit()

/*
@func void	| mBuf_init	| mbuf module initialization
@parm uint32 	| mbufs		| Number of mbufs
@parm uint32 	| clusters	| Number of clusters
@parm uint32 	| pkthdrs		| Number of packet headers
@parm uint32		| clusterSize	| Size of each cluster
@parm uint32		| msgLogId	| Debuging message log list id
@rdesc None
@rvalue SUCCESS		| 	The mbuf module and its memory pool is initiated as requested
@rvalue FAILED	| 	Failed to initialize the mbuf module.
@comm mbuf module initialization. Allocate mbuf and related structure pools. Value for <p clusterSize> must be power of 2
 */


extern int32 mBuf_getBufStat(struct rtl_mBufStatus *mbs);
/*
@func  int32	| mBuf_getBufStat	| Returns current status of the mbuf module
@parm struct rtl_mBufStatus *	| mbs	| A pointer to the mbstat structure for mBuf_getBufStat() to fill in
@rdesc Returns current status of the mbuf module
@rvalue <p FAILED>	| 	Failed to get mbuf module status. Maybe mbs is NULL or mbuf module not yet initiated.
@rvalue <p SUCCESS>	| 	Mbuf module status is returned with the mbstat structure given.
 */



extern int32 mBuf_leadingSpace(struct rtl_mBuf *m);

/*
@func int32	| mBuf_leadingSpace	| Calculate the number of leading free data bytes.
@parm struct rtl_mBuf * 	| m		| Pointer to the mbuf chain.
@rdesc Returns the number of free leading space
@rvalue n		| 	The number of free leading data bytes in cluster.
@comm
Calculate the number of leading free data bytes.
@xref  <c mBuf_trailingSpace>

 */


extern int32 mBuf_trailingSpace(struct rtl_mBuf *m);

/*
@func int32	| m_trailinspace	| Calculate the number of trailing free data bytes.
@parm struct rtl_mBuf * 	| m		| Pointer to the mbuf chain.
@rdesc Returns the number of free trailing space
@rvalue n		| 	The number of free trailing data bytes in cluster.
@comm
Calculate the number of trailing free data bytes.
@xref <c mBuf_leadingSpace>

 */


extern int32 mBuf_clusterIsWritable(struct rtl_mBuf *m);

/*
@func int32	| mBuf_clusterIsWritable	| Determine whether <p m>'s cluster is writable or not.
@parm struct rtl_mBuf * 	| m		| Pointer to an mbuf.
@rdesc Returns TRUE or FALSE
@rvalue TRUE		| 	The cluster is writable.
@rvalue FALSE	| 	The cluster is not writable.
@comm
Determine whether <p m>'s cluster is writable or not. mbufs allocated due to mBuf_clonePacket, mBuf_split, mBuf_cloneMbufChain
should not modify its cluster becoz it is not the owner of these clusters.
 */

extern   uint32 mBuf_getPktlen(struct rtl_mBuf *m);
/*
@func uint32	| mBuf_getPktlen	| Get total number of data bytes in the packet which <p m> belongs to
@parm struct rtl_mBuf *	|m		| Indicate the packet
@rdesc Returns data length of the packet
@rvalue -1		| 	Failed.
@rvalue <p length>| 	The actual length of  packet
@comm
Get total number of data bytes in the packet which <p m> belongs to
@xref <c MBUF_GETLEN>
*/

extern struct rtl_mBuf *mBuf_data2Mbuf(int8 * x);  //was dtom

/*
@func struct rtl_mBuf *	| mBuf_data2Mbuf	| Given data address <p x>, return mbuf address <p m>
@parm int8  * 	|x		| Data address
@rdesc Returns mbuf address <p m> which owns the cluster block where <p x> resides.
@rvalue <p NULL>		| 	Can't find the address. The data address <p x> given might be within a zombie cluster whose owning mbuf has already been freed.
@rvalue <p m>		| 	The address of owning mbuf
@comm
Finds the mbuf address of given data address <p x>
@devnote  Original BSD code define this as a dtom macro. In our implmentation, we make it a function.
@ex The following example demonstrates how to use this function |
	struct rtl_mBuf *n;

	n = mBuf_data2Mbuf(x);
	if (n != NULL) {
		printf("There are %d bytes in the cluster", n->m_len);
	}
  @xref  <c MBUF2DATAPTR>
*/



extern struct rtl_mBuf *mBuf_get(int32 how, int32 type, uint32 Nbuf);

/*
@func struct rtl_mBuf *	| mBuf_get	| mbuf space allocation routines.
@parm int32			|how	| <p MBUF_WAITOK>, <p MBUF_DONTWAIT>, and/or <p MBUF_ONLY>
@parm int32			|type	| The type of mbuf requested. only <p MT_DATA> now.
@parm uint32 			| Nbuf	| The number of mbufs requesting.

@rdesc Returns the address of allocated mbuf head
@rvalue <p NULL>		| 	Can't allocate all  <p Nbuf>s at once.
@rvalue <p n>		| 	The address of first mbuf allocated. All <p Nbuf>s have been linked together.
@comm
Get <p Nbuf> mbufs and clusters. All mbufs are chained via the <p m_next> pointer inside each mbuf. Note that memory content in all allocated clusters are NOT initialized.

If <p how> has MBUF_DONTWAIT bit set, and we can't get all <p Nbuf>s immediately, return NULL.

If <p how> has MBUF_WAITOK bit set, and we can't get all <p Nbuf>s right away, block waiting until our request is satisfied

If <p how> has MBUF_ONLY bit set, then no clusters would be allocated. Only mbufs are allocated. Can be used with MBUF_DONTWAIT or MBUF_WAITOK

@devnote
1.Side effect: <p mBuf_get> wakes up any sleeping threads if they are block waiting for free mbufs or clusters.

@ex The following example demonstrates how to use this function |
	struct rtl_mBuf *m;
	m= mBuf_get( MBUF_DONTWAIT, MT_DATA, 5);  //allocate 5 MT_DATA-typed mbufs and clusters without blocking.
	if (!m)
		return NULL;

  @xref
  <c mBuf_getCleared> ,  <c mBuf_getm>, <c mBuf_driverGet>
 */


extern struct rtl_mBuf *mBuf_getCleared(int32 how, int32 type, uint32 Nbuf);

/*
@func struct rtl_mBuf *	| mBuf_getCleared| Same as mBuf_get, but initialize all clusters to zero.
@parm int32			|how	| <p MBUF_WAIT>, <p MBUF_DONTWAIT> and/or <p MBUF_ONLY>
@parm int32			|type	| The type of mbuf requested. only <p MT_DATA> now.
@parm uint32 			| Nbuf	| The number of mbufs requesting.

@rdesc Returns the address of allocated mbuf head
@rvalue <p NULL>		| 	Can't allocate all  <p Nbuf>s at once.
@rvalue <p n>		| 	The address of first mbuf allocated. All <p Nbuf>s have been linked together and cleared to 0.
@comm
Same as <p mget>(). However, content in all mbufs and clusters have been cleared to 0.
@ex The following example demonstrates how to use this function |
	register struct rtl_mBuf *m;
	m= mBuf_getCleared(MBUF_DONTWAIT | MBUF_ONLY, MT_DATA, 5);  //allocate 5 MT_DATA-typed mbufs without blocking.
	if (!m)
		return NULL;

  @xref
  <c mBuf_get>,  <c mBuf_getm>, <c mBuf_driverGet>
 */


extern uint32 mBuf_driverGet(uint32 Nmbuf,struct rtl_mBuf **ppFirstMbuf, struct rtl_mBuf **ppLastMbuf);
/*
@func uint32	| mBuf_driverGet| Driver specific. Get multiple MT_DATA-typed mbufs without blocking
@parm uint32	|Nmbuf	| Number of mbufs requesting
@parm struct rtl_mBuf **	|ppFirstMbuf	| Returns a pointer which points to  the first mbuf allocated.
@parm struct rtl_mBuf **	| ppLastMbuf	| Returns a pointer which points to  the last mbuf allocated.
@rdesc Returns number of mbufs successfully allocated.
@comm
An optimized mBuf_get() for driver. <p ppFirstMbuf> and <p ppLastMbuf> should not be NULL!!
mbuf and cluster not initialized to zero. Only required fields in mBuf_driverGet have been properly filled.

@ex The following example demonstrates how to use this function |
	struct rtl_mBuf * localTempMbufHead,  * localTempMbufTail;
	int32 allocated;
	requested = 5;
	allocated= mBuf_driverGet(requested, &localTempMbufHead, &localTempMbufTail);
	if (allocated!=requested)
		printf("Can't allocate all 5 mbufs in driver\n")

  @xref
  <c mBuf_get>,  <c mBuf_getm>, <c mBuf_getCleared>
 */

extern struct rtl_mBuf *mBuf_getm(struct rtl_mBuf *m, uint32 len, int32 how, int32 type);

/*
@func struct rtl_mBuf *	| mBuf_getm| allocate <p len>-worth of  mbuf clusters and return a pointer to the top
of the allocated chain.
@parm struct rtl_mBuf * 	| m		| Pointer to an mbuf chain. Could be NULL.
@parm uint32			|len		| Number of data bytes requesting.
@parm int32			|how	| <p MBUF_WAIT> or <p MBUF_DONTWAIT> and/or <p M_ALLOCPKTHDR>.
@parm int32			|type	| Type of mbuf requested. only <p MT_DATA> now.
@rdesc Returns return a pointer to the head of the allocated chain.
@rvalue <p NULL>		| 	Allocation failed.
@rvalue <p n>		| 	The memory address of first mbuf allocated.
@comm
If <p m> is non-null, then it is an mbuf chain to which we want <p len> bytes worth of clusters and their associating mbufs be attached, and so
if allocation is successful, a pointer to m is returned. i.e <p n> = <p m>)

If <p m> is null,  <p len> bytes worth of clusters and their associating mbufs would be allocated.

If <p how> has M_ALLOCPKTHDR bit set, <p mBuf_getm> also allocates a pkthdr with the mbuf chain.
If <p how> has MBUF_DONTWAIT bit set, and we can't get a pkthdr immediately, return NULL.
If <p how> has MBUF_WAITOK bit set, and we can't get a pkthdr right away, block waiting until our request is satisfied.

M_BUFONLY can't be used with mBuf_getm, in this case, you should use mBuf_get() and calculate how many mbufs you need on your own.

@ex The following example demonstrates how to use this function |
	m = mBuf_getm(m, size, MBUF_WAITOK, MT_DATA);
	if (m == NULL)
		return ENOBUFS;

  @xref
  <c mBuf_get> , <c mBuf_driverGet>, <c mBuf_getCleared>
 */


extern struct rtl_mBuf *mBuf_getPkthdr(struct rtl_mBuf *m, int32 how);
/*
@func struct rtl_mBuf *	| mBuf_getPkthdr| Allocate a packet header for mbuf chain <p m>
@parm struct rtl_mBuf * 	| m		| Pointer to the mbuf chain.
@parm int32			|how	| <p MBUF_WAIT> or <p MBUF_DONTWAIT>

@rdesc Returns the address of mbuf <p m> that holds a packet header.
@rvalue <p NULL>		| 	Can't allocate a  pkthdr for mbuf <p m>.
@rvalue <p m>		| 	A pkthdr is allocated to mbuf <p m>.
@comm

Given an mbuf <p m>, allocate a packet header (pkthdr) structure for <p m> and makes <p m> the owner of this pkthdr.
If there is already a pkthdr owned by some mbuf after m on the same mbuf chain, m becomes the owner of that pkthdr without getting a new one. Corresponding pkthdr fields are modified accordingly.

If <p how> has MBUF_DONTWAIT bit set, and we can't get a pkthdr immediately, return NULL.
If <p how> has MBUF_WAITOK bit set, and we can't get a pkthdr right away, block waiting until our request is satisfied.


 mBuf_getPkthdr may wake up other sleeping threads waiting for pkthdrs if any

@ex The following example demonstrates how to use this function |
	if (!mBuf_getPkthdr(pNewMbuf, MBUF_DONTWAIT))	//get a pkthdr
		goto nospace;
  @xref
  <c mBuf_driverGetPkthdr>
 */

extern uint32 mBuf_driverGetPkthdr(uint32 Npkthdr,struct rtl_pktHdr **ppHeadPkthdr, struct rtl_pktHdr **ppTailPkthdr);
/*
@func uint32	| mBuf_driverGetPkthdr| Driver specific. Get multiple pkthdrs without blocking
@parm uint32	|Npkthdr	| Number of pkthdrs requesting
@parm struct rtl_pktHdr **	|ppHeadPkthdr	| Returns a pointer which points to  the first pkthdr allocated.
@parm struct rtl_pktHdr **	| ppTailPkthdr	| Returns a pointer which points to  the last pkthdr allocated.
@rdesc Returns number of pkthdrs successfully allocated.
@comm
This function is dedicated for driver.  <p ppHeadPkthdr> and <p ppTailPkthdr> should not be NULL!!

@ex The following example demonstrates how to use this function |
	struct rtl_pktHdr * localTempPkthdrHead,  * localTempPkthdrTail;
	int32 allocated;
	requested = 5;
	allocated= mBuf_driverGetPkthdr(requested, &localTempPkthdrHead, &localTempPkthdrTail);
	if (allocated!=requested)
		printf("Can't allocate all 5 mbufs in driver\n")

  @xref
  <c mBuf_getPkthdr>
 */

extern struct rtl_mBuf *mBuf_freeOne(struct rtl_mBuf *m);

/*
@func struct rtl_mBuf *	| mBuf_freeOne	| Free a single mbuf <p m> and associated cluster storage.
@parm struct rtl_mBuf * 	| m		|  Pointer to an mbuf chain.
@rdesc Returns address of next mbuf if any
@rvalue <p NULL>		| 	<p m> has been freed and there isn't any successing mbufs.
@rvalue <p n>		| 	The address of next mbuf after <p m>.
@comm
 Free a single mbuf and associated external storage. Place the successor, if any, in <p n>.
 Note that <p m> is a value-result parameter. If the mbuf <p m> is freed, <p m> will point
 to next mbuf, if any, on return.

@ex The following example demonstrates how to use this function |
	struct rtl_mBuf *n;
	n = mBuf_freeOne(m, MBUF_WAIT);

@xref
  <c mBuf_freeMbufChain> , <c mBuf_driverFreeMbufChain>, <c mBuf_freePkthdr>, <c mBuf_driverFreePkthdr>
 */

extern uint32 mBuf_freeMbufChain(register struct rtl_mBuf *m);

/*
@func struct rtl_mBuf *	| mBuf_freeMbufChain| Free the whole mbuf chain started from <p m>
@parm struct rtl_mBuf * 	| m		| Pointer to the mbuf chain.
@rdesc Returns number of mbufs being freed in mbuf chain <p m>.
@rvalue <p n>		| 	The number of mbufs being freed. If <p n> is 0, <p m> is not freed.
@comm
 Free the whole mbuf chain starting from <p m>.

@devnote  Return type changed from void to int32 to provide more information.
@ex The following example demonstrates how to use this function |
	struct rtl_mBuf *m;
	int32 n;
	m = mBuf_get(MBUF_DONTWAIT, MT_DATA, 5);
	n = mBuf_freeMbufChain(m, MBUF_WAIT);
	if (n!=5){
		//Do error handling...
	}
@xref
  <c mBuf_freeOne> , <c mBuf_driverFreeMbufChain>, <c mBuf_freePkthdr>, <c mBuf_driverFreePkthdr>
 */

extern uint32 mBuf_driverFreeMbufChain(struct rtl_mBuf *pFirstMbuf);
/*
@func uint32	| mBuf_driverFreeMbufChain| Free the whole mbuf chain started from <p m>
@parm struct rtl_mBuf * 	| m		| Pointer to the mbuf chain.
@rdesc Returns number of mbufs being freed in mbuf chain <p m>.
@rvalue <p n>		| 	The number of mbufs being freed. If <p n> is 0, <p m> is not freed.
@comm
 Free the whole mbuf chain starting from <p m>.  To be used only in driver.
@ex The following example demonstrates how to use this function |
	struct rtl_mBuf * localTempMbufHead,  * localTempMbufTail;
	int32 allocated;
	requested = 5;
	allocated= mBuf_driverGet(requested, &localTempMbufHead, &localTempMbufTail);
	if (allocated==requested){
		n = mBuf_driverFreeMbufChain(localTempMbufHead);
		if(n==allocated)
			printf("All mbufs successfully freed in driver\n");
	}
@xref
  <c mBuf_freeOne> , <c mBuf_driverFreeMbufChain>, <c mBuf_freePkthdr>, <c mBuf_driverFreePkthdr>
 */



extern void mBuf_freePkthdr(struct rtl_pktHdr *ph);
/*
@func struct rtl_pktHdr *	| mBuf_freePkthdr| Free a pkthdr alone.
@parm struct rtl_pktHdr *	|ph	| The pkthdr to be freed.
@rdesc No return value
@comm  Free a pkthdr alone. Callers should be aware that this is NOT the normal way to free a pkthdr with mbufs attached.
You should use <p mBuf_freeOne> or <p mBuf_freeMbufChain> to free pkthdrs attached with mbuf chains. This function is usd once only in TCP module.
@devnote
Caller of this function should remove the links between mbufs and pkthdrs on their own before <p mBuf_freePkthdr> is called.
@xref  <c mBuf_freeOne> , <c mBuf_freeMbufChain>, <c mBuf_driverFreeMbufChain>, <c mBuf_driverFreePkthdr>
 */

uint32 mBuf_driverFreePkthdr(struct rtl_pktHdr *ph, uint32 Npkthdr, struct rtl_pktHdr **ppHeadPkthdr);
/*
@func uint32	| mBuf_driverGetPkthdr| Driver specific. Free multiple pkthdrs without blocking
@parm struct rtl_pktHdr *	|ph	| The first pkthdr to be freed
@parm uint32	|Npkthdr	| Number of pkthdrs to be freed
@parm struct rtl_pktHdr **	|ppHeadPkthdr	| Returns next un-freed pkthdr, if any.
@rdesc Returns number of pkthdrs successfully freed.
@comm
This function is dedicated for driver.  <p ph> should not be NULL!!

 @xref
   <c mBuf_freeOne> , <c mBuf_freeMbufChain>, <c mBuf_driverFreeMbufChain>, <c mBuf_freePkthdr>
 */

extern struct rtl_mBuf *mBuf_adjHead(struct rtl_mBuf *, uint32 req_len);
/*
@func struct rtl_mBuf *	| mBuf_adjHead	| Remove <p req_len> bytes of data from head the mbuf chain pointed to by <p m>.
@parm struct rtl_mBuf * 	| m		| Pointer to an mbuf chain. <p m> doesn't need to be the first mbuf in chain.
@parm int32			| req_len	| Number of data bytes to be removed
@rdesc m_adj returns the address of first mbuf after trimming data.
@rvalue <p NULL>	| 	Can't adjust length of mbuf. <p m> might 1)be NULL 2)have no clusters.
@rvalue <p m>		| 	When success, the first mbuf <p m> where user can read/write data is returned.
@comm
Remove <p req_len> bytes of data from the head of mbuf chain  <p m>. If user removed all
data bytes in the whole packet, <p m> is returned.

@devnote  If any clusters, after m_adj, is totally unused (ie. m_len=0),
the m_data pointer would be reset to m_extbuf and the mbuf would NOT be freed.

@xref
  <c mBuf_adjTail>,   <c mBuf_trimHead>,  <c mBuf_trimTail>
 */

extern struct rtl_mBuf *mBuf_adjTail(struct rtl_mBuf *, uint32 req_len);
/*
@func struct rtl_mBuf *	| mBuf_adjTail	| Remove <p req_len> bytes of data from tail of the mbuf chain pointed to by <p m>.
@parm struct rtl_mBuf * 	| m		| Pointer to an mbuf chain. <p m> doesn't need to be the first mbuf in chain.
@parm int32			| req_len	| Number of data bytes to be removed
@rdesc m_adj returns the address of mbuf which contains the last data byte.
@rvalue <p NULL>	| 	Can't adjust length of mbuf. <p m> might 1)be NULL 2)have no clusters or <p req_len> is 0
@rvalue <p m>		| 	When success, the address of the mbuf which hold the last data byte is returned.
@comm
Remove <p req_len> bytes of data from the tail of mbuf chain  <p m>. If user removed all
data bytes in the whole packet, <p m> is returned.

@devnote  If any clusters, after m_adj, is totally unused (ie. m_len=0),
the m_data pointer would be reset to m_extbuf and the mbuf would NOT be freed.

@xref
  <c mBuf_adjHead>,   <c mBuf_trimHead>,  <c mBuf_trimTail>
 */

extern struct rtl_mBuf *mBuf_trimHead(struct rtl_mBuf *, uint32 req_len);
/*
@func struct rtl_mBuf *	| mBuf_trimHead	| Same as mBuf_adjHead, but also frees unused mbufs and clusters
@parm struct rtl_mBuf * 	| m		| Pointer to an mbuf chain. <p m> doesn't need to be the first mbuf in chain.
@parm int32			| req_len	| Number of data bytes to be trimmed from head.
@rdesc mBuf_trimHead returns the address of first mbuf after trimming.
@rvalue <p NULL>	| 	Can't adjust length of mbuf or the whole mbuf chain is trimmed and freed.
@rvalue <p m>		| 	When success, the first mbuf <p m> where user can read/write data is returned.
@comm
Same as mBuf_adjHead, but also frees unused mbufs and clusters

@devnote  mBuf_trimHead is implemented with mBuf_split and mBuf_freeMbufChain. It first splits the mbuf
to two mbuf chains from indicated position and frees the first one.
There is a possible risk that mBuf_trimHead may fail when the mbuf chain <p m> consumes all mbufs and no free mbufs
is available for mBuf_split to work correctly.

@xref
  <c mBuf_adjHead>,   <c mBuf_adjTail>,  <c mBuf_trimTail>
 */

extern struct rtl_mBuf *mBuf_trimTail(struct rtl_mBuf *, uint32 req_len);
/*
@func struct rtl_mBuf *	| mBuf_trimTail	| Same as mBuf_adjTail, but also frees unused mbufs and clusters
@parm struct rtl_mBuf * 	| m		| Pointer to an mbuf chain. <p m> doesn't need to be the first mbuf in chain.
@parm int32			| req_len	| Number of data bytes to be trimmed from tail.
@rdesc mBuf_trimTail returns the address <p m>.
@rvalue <p NULL>	| 	Can't adjust length of mbuf or the whole mbuf chain is trimmed and freed.
@rvalue <p m>		| 	When success, returns the first mbuf <p m> with data.
@comm
Same as mBuf_adjTail, but also frees unused mbufs and clusters

@devnote  mBuf_trimTail is implemented with mBuf_split and mBuf_freeMbufChain. It first splits the mbuf
to two mbuf chains from indicated position and frees the latter one.
There is a possible risk that mBuf_trimTail may fail when the mbuf chain <p m> consumes all mbufs and no free mbufs
is available for mBuf_split to work correctly.

@xref
  <c mBuf_adjHead>,   <c mBuf_adjTail>,  <c mBuf_trimTail>
*/



extern int32 mBuf_copyToMbuf(struct rtl_mBuf *, uint32 offset, uint32 len, int8 *cp);

/*
@func int32			| mBuf_copyToMbuf	| Copy <p len> bytes of data from user's buffer <p cp> to mbuf chain <p m>, started form offset <p offset>.
@parm struct rtl_mBuf * 	| m		| Address of the mbuf chain.
@parm uint32			|offset	| Starting byte to be copied in mbuf chain <p m>. Start from 0
@parm uint32			|len		| Number of bytes to be copied from <p cp>
@parm int8 * 		|cp		| Address of user's buffer.
@rdesc Returns number of bytes successfully copied
@rvalue <p -1>		| 	Failed.
@rvalue <p n>		| 	<p n> bytes have been copied from <p cp> into indicated mbuf chain <p m>
@comm
Copy <p len> bytes from user's buffer <p cp>, to the indicated mbuf chain
<p m> beginning from the <p offset>-th data byte in mbuf chain.
If there aren't at least 'offset' bytes in mbuf chain, -1 is returned.
mBuf_copyToMbuf() extends mbuf chain if neccessary.

@ex The following example demonstrates how to use this function |
	#define SIZE	100
	int32 i;
	int8 my_buffer[SIZE];
	for(i=0; i<SIZE; i++)
		my_buffer[i] = i;
	if (100!=mBuf_copyToMbuf(m, 10, 100, my_buffer))	//copy 100 bytes from my_buffer to mbuf m, started from the 10th bytes in mbuf
		printf("Can't copy all data!");
  @xref
  <c mBuf_copyToUserBuffer>
 */



extern int32 mBuf_copyToUserBuffer(struct rtl_mBuf *m, uint32 off, uint32 len, int8 * cp);

/*
@func int32			| mBuf_copyToUserBuffer	| Copy some data from an mbuf chain <p m> to user's buffer <p cp>.
@parm struct rtl_mBuf * 	| m		| Pointer to an mbuf chain.
@parm uint32			|offset	| The number of starting data byte in mbuf chain <p m>. Start from 0.
@parm uint32			|len		| The number of data bytes to be copied. If <p len>=M_COPYALL, then copy all data till the end of mbuf chain.
@parm int8 *			|cp		| User specified buffer address.
@rdesc Returns number of bytes successfully received and copied to user's buffer
@rvalue <p -1>		| 	Failed.
@rvalue <p n>		| 	<p n> bytes have been copied from <p m> into indicated buffer <p cp> successfully.
@comm
Copy data from an mbuf chain starting from the <p offset>-th data byte,
continuing for <p len> bytes, into the indicated buffer <p cp>. User should be sure that there is enough free space in
the specified buffer <p cp>.

@ex The following example demonstrates how to use this function |
	int8 my_buffer[100];
	mBuf_copyToUserBuffer(m, 10, 100, my_buffer); //copy 100 bytes from the 10-th byte in mbuf to user's cp buffer.

  @xref
  <c mBuf_copyToMbuf>
 */

extern struct rtl_mBuf *mBuf_cloneMbufChain(struct rtl_mBuf *pThisMbuf, int32 iOffset,
							int32 iLength, int32 iWait);

/*
@func struct rtl_mBuf *	| mBuf_cloneMbufChain	| Clone a part of mbuf chain <p m>
@parm struct rtl_mBuf * 	| m		| Pointer to an mbuf chain.
@parm int32			|offset	| The number of starting data byte in mbuf chain <p m>. Start from 0
@parm int32			|len		| The number of data bytes to be cloned. If <p len>=M_COPYALL, then clone all data till the end of mbuf chain.
@parm int32			|how	| <p MBUF_WAIT> or <p MBUF_DONTWAIT> and/or <p M_ALLOCPKTHDR>
@rdesc Returns address of the new, cloned mbuf chain <p n>.
@rvalue <p NULL>		| 	Can't clone mbuf chain <p m>.
@rvalue <p n>		| 	The address of cloned mbuf chain <p n>. Data in <p n> is read only.
@comm
Make a clone of an mbuf chain starting from <p offset>-th byte from the beginning,
continuing for <p len> bytes.  If <p len> is M_COPYALL, then clone to end of the whole mbuf chain. (You may choose to use the optimized mBuf_clonePacket() in this case)
The <p how> parameter is a choice of MBUF_WAIT or MBUF_DONTWAIT and/or M_ALLOCPKTHDR by the caller.
If M_ALLOCPKTHDR flag is given, a new pkthdr would be allocated for duplicated packet. 

Note that the clone is read-only, The new mbuf chain <p n> only shares cluster data with <p m>.
<p n> can only read cluster data, but not write. <p m> still owns write priviledge.

@ex The following example demonstrates how to use this function |
	struct rtl_mBuf *n;

	n = mBuf_cloneMbufChain(m, 20, M_COPYALL, MBUF_WAIT);
	if (n != NULL) {
		// do following processing
	}
  @xref
  <c mBuf_clonePacket>, <c mBuf_dupPacket>, <c mBuf_cloneMbufChain>
 */

extern struct rtl_mBuf *mBuf_dupMbufChain(struct rtl_mBuf *pMbufChain, int32 iOffset, int32 iLength,  int32 flag);

/*
@func struct rtl_mBuf *	| mBuf_dupMbufChain	| Duplicate a part of mbuf chain <p m>
@parm struct rtl_mBuf * 	| m		| Pointer to an mbuf chain.
@parm int32			|offset	| The number of starting data byte in mbuf chain <p m>. Start from 0
@parm int32			|len		| The number of data bytes to be cloned. If <p len>=M_COPYALL, then duplicate all data till the end of mbuf chain.
@parm int32			|how	| <p MBUF_WAIT> or <p MBUF_DONTWAIT> and/or <p M_ALLOCPKTHDR>
@rdesc Returns address of the new, duplicated mbuf chain <p n>.
@rvalue <p NULL>		| 	Can't duplicate mbuf chain <p m>.
@rvalue <p n>		| 	The address of duplicated mbuf chain <p n>. Data in <p n> is writable.
@comm
Duplicate an mbuf chain starting from <p offset>-th byte from the beginning,
continuing for <p len> bytes.  If <p len> is M_COPYALL, then duplicate to end of the whole mbuf chain. (You may choose to use the optimized mBuf_dupPacket() in this case)
The <p how> parameter is a choice of MBUF_WAIT or MBUF_DONTWAIT and/or M_ALLOCPKTHDR by the caller.
If M_ALLOCPKTHDR flag is given, a new pkthdr would be allocated for duplicated packet. 

Note that the duplicated mbuf is writable..

@ex The following example demonstrates how to use this function |
	struct rtl_mBuf *n;

	n = mBuf_dupMbufChain(m, 20, M_COPYALL, MBUF_WAIT);
	if (n != NULL) {
		// do following processing
	}
  @xref
  <c mBuf_clonePacket>, <c mBuf_dupPacket>, <c mBuf_cloneMbufChain>
 */



extern struct rtl_mBuf *mBuf_clonePacket(struct rtl_mBuf *pMbuf, int32 iHow);

/*
@func struct rtl_mBuf *	| mBuf_clonePacket	| Clone the entire packet <p m>
@parm struct rtl_mBuf * 	| m		| Pointer to an mbuf chain.
@parm int32			|how	| <p MBUF_WAIT> or <p MBUF_DONTWAIT>
@rdesc Returns address of the new mbuf chain <p n>.
@rvalue <p NULL>		| 	Can't clone mbuf chain <p m>.
@rvalue <p n>		| 	The address of copied mbuf chain <p n>. Data in <p n> is read only.
@comm
Clone an entire mbuf chain <p m>, including the packet header (which must be present).
An optimization of the common case `mBuf_cloneMbufChain(m, 0, M_COPYALL, how)'.
The new cloned packet always allocates a new pkthdr.
Note that the copy is read-only, because clusters are not copied, only their reference counts are incremented.

@ex The following example demonstrates how to use this function |
	struct rtl_mBuf *n;
	n = mBuf_clonePacket(m, MBUF_WAIT);
	if (n != NULL) {
		// do following processing
	}
  @xref
  <c mBuf_dupPacket>, <c mBuf_cloneMbufChain>, <c mBuf_dupMbufChain>
 */

extern struct rtl_mBuf *mBuf_dupPacket(struct rtl_mBuf *pMbuf, int32 iHow);

/*
@func struct rtl_mBuf *	| mBuf_dupPacket	| Duplicate an mbuf chain <p m>, including its data,  into a completely new chain <p n>
@parm struct rtl_mBuf * 	| m		| Pointer to an mbuf chain.
@parm int32			|how	| <p MBUF_WAIT> or <p MBUF_DONTWAIT>
@rdesc Returns address of the new mbuf chain <p n>.
@rvalue <p NULL>		| 	Can't duplicate mbuf chain <p m>.
@rvalue <p n>		| 	The address of copied mbuf chain <p n>. Data in <p n> is writable.
@comm
Duplicate an mbuf chain <p m> into a completely new chain <p n>, including
copying data in any <p m>'s mbuf clusters. 
The new duplicated packet always allocates a new pkthdr.
Use this instead of mBuf_clonePacket() when you need a writable copy of an mbuf chain.

@ex The following example demonstrates how to use this function |
	struct rtl_mBuf *n;
	n = mBuf_dupPacket(m, MBUF_WAIT);
	if (n != NULL) {
		// do following processing
	}
  @xref
  <c mBuf_clonePacket> , <c mBuf_copyToUserBuffer>, <c mBuf_cloneMbufChain>
 */





extern struct rtl_mBuf *mBuf_prepend(struct rtl_mBuf *m, uint32 plen, int32 how);

/*
@func struct rtl_mBuf *	| mBuf_prepend	| Arrange to prepend space of size <p plen> to mbuf <p m>.
@parm struct rtl_mBuf * 	| m		| Pointer to an mbuf chain.
@parm uint32			|plen	| Number of bytes to be prepended before <p m>
@parm int32			|how	| <p MBUF_WAIT>, <p MBUF_DONTWAIT>, <p M_GETNEWMBUF>
@rdesc Returns address of the first mbuf.
@rvalue <p NULL>		| 	Can't allocate memory or parameter problem.
@rvalue <p n>		| 	The address of first mbuf after prepending.
@comm
Arrange to prepend space of size <p plen> to mbuf <p m>.
 If new mbufs must be allocated, <p how> specifies whether to wait or not. When user calls mBuf_prepend, the
 data length recorded in mbufs and pkthdr is incremented accordingly.

By default, mBuf_prepend uses any leading free space before allocating any new clusters.
But if <p M_GETNEWMBUF> is set, mBuf_prepend allocates new mbufs directly and leaves any leading free buffer space as is.

If <p how> has MBUF_DONTWAIT bit set, and we can't get all buffers immediately, return NULL.
If <p how> has MBUF_WAITOK bit set, and we can't get all buffers right away, block waiting until our request is satisfied.

M_BUFONLY can't be used with mBuf_prepend.

@ex The following example demonstrates how to use this function |
	m0 = mBuf_prepend(m0, PPP_HDRLEN, MBUF_DONTWAIT | M_GETNEWMBUF);
	if (m0 == 0) {
	    error = ENOBUFS;
	    goto bad;
	}
  @xref <c mBuf_cat>, <c mBuf_padding>

 */

extern struct rtl_mBuf *mBuf_padding(struct rtl_mBuf *m, uint32 plen, int32 how);
/*
@func struct rtl_mBuf *	| mBuf_padding	| Arrange to append space of size <p plen> to mbuf <p m>.
@parm struct rtl_mBuf * 	| m		| Pointer to an mbuf chain.
@parm uint32			|plen	| Number of bytes to append at <p m>
@parm int32			|how	| <p MBUF_WAIT> or <p MBUF_DONTWAIT>
@rdesc Returns address of the last mbuf padded.
@rvalue <p NULL>		| 	Can't allocate memory or parameter problem.
@rvalue <p n>		| 	The address of last mbuf after padding
@comm
Arrange to append space of size <p plen> to mbuf <p m>.
 If new mbufs must be allocated, <p how> specifies whether to wait or not. When user calls mBuf_padding, the
 data length recorded in mbufs and pkthdr are incremented accordingly.

 M_BUFONLY can't be used with mBuf_padding.
@devnote  API CHANGED:  If <p how> is MBUF_DONTWAIT and allocation fails, NULL is returned.
Original mbuf <p m> is intact.

@ex The following example demonstrates how to use this function |
	m0 = mBuf_padding(m, (ETHER_MIN_LEN-ETHER_CRC_LEN-len), MBUF_DONTWAIT);
	if (m0 == 0) {
	    error = ENOBUFS;
	    goto bad;
	}
@xref <c mBuf_split>, <c mBuf_cat>, <c mBuf_getm>, <c mBuf_prepend>, <c mBuf_split>


 */


extern struct rtl_mBuf *mBuf_cat(struct rtl_mBuf *m, struct rtl_mBuf *n);

/*
@func struct rtl_mBuf *	| mBuf_cat	| Concatenate mbuf chain <p n> to <p m>.
@parm struct rtl_mBuf * 	| m		| mbuf chain to be appended.
@parm struct rtl_mBuf * 	| n		| mbuf chain to be appended after <p m>
@rdesc Returns the address of <p m> or NULL if failed.
@rvalue <p NULL>		| 	Can't concatenate two mbuf chain
@rvalue <p m>		| 	When success, the address of <p m> is returned..
@comm
Concatenate mbuf chain <p n> to <p m>.
Both chains must be of the same type (e.g. <p MT_DATA>).
Total packet length of <p m> would be adjusted accordingly. However, <p n>'s control header, if any, would be freed
@xref   <c mBuf_split>, <c mBuf_padding>, <p mBuf_getm>

 */


extern int32 mBuf_pullup(struct rtl_mBuf *, int32);

/*
@func int32		| mBuf_pullup| 	Rearange an mbuf chain so that <p len> bytes are contiguous
 								 in <p m>'s first cluster.
@parm struct rtl_mBuf * 	| m	| Pointer to an mbuf chain.
@parm 	int32			|len	| The number of databytes requested to put in <p m>'s first cluster. <p len> should not exceed the size of a cluster buffer (256 bytes).
@rdesc	Returns total data bytes in the first mbuf cluster.
@rvalue <p n>		| 	The number of data bytes in <p m>'s first cluster. If <p n> is less than <p len>,
						that means <p mBuf_pullup> can't pull up all <p len> bytes requested because there isn't
						so much data in mbuf chain.
@comm  Original BSD4.4 note: Rearange an mbuf chain so that <p len> bytes are contiguous and in <p m>'s cluster.
Successing mbufs are adjusted accordingly. If the return value <p n> is smaller than requested <p len>,
maybe there isn't enough data bytes in the mbuf chain <p m>, or maybe the requested value <p len> exceeds
the maximum capacity for a single cluster.

Our implementation note:
(cfliu 2002.02.19)This function is important in traditional BSD networking stack because
original mbuf can't do dtom() if data is saved in cluster. This is not the
case here since we offer a back pointer from cluster to mbuf via cltag_mbuf field.
However, this function is still useful if we want to collect continous databytes from later clusters
to the specified mbuf's cluster. The maximum number of  <p len> should be less than a cluster
can hold (ie. len less than or equal to m_clusterSIze)

(2002.08.11) If 1) requested length <p len> is smaller than m_clusterSize, and 2) there are enough data in this mbuf chain. However, the
trailing space in <p m> is not large enough, <p mBuf_pullup> will move m's data upward first and then copy requested data from latter clusters

@devnote  This function is different from traditional BSD mbuf code. When pull up fails, the mbuf chain will not be freed,
and the return value has been changed to an integer instead of a pointer to mbuf structure.

@ex The following example demonstrates how to use this function |
	int32 n;

	n = mBuf_pullup(m, 128);
	if (n < 128) {
		//There isn't so much data in mbuf chain.
	}
  @xref
  <c m_pulldown>
 */


struct rtl_mBuf *mBuf_split(register struct rtl_mBuf *m0, uint32 len0, int32 wait);

/*@func struct rtl_mBuf * | mBuf_split | Partition an mbuf chain in two pieces. Data is cloned
  @parm struct rtl_mBuf * 	| m    	| Pointer to an mbuf chain
  @parm 	int32			| len    	| The number of last data byte to remain in original mbuf chain <p m>
  @parm  int32 			| wait   	| <p MBUF_DONTWAIT> or <p MBUF_WAIT>
  @rdesc	Returns a pointer to the splited mbuf chain <p n>
  @rvalue NULL		| Can't split the mbuf chain
  @rvalue <p n>		| Success. <p n> is the address of second mbuf chain. It holds all data bytes after the <p len>-th byte in <p m>.
  @comm  Partition an mbuf chain <p m> into two pieces, returning the tail mbuf chain <p n>. In case of failure, it returns NULL and
 attempts to restore the chain to its original state. Data in clusters  are NOT copied, but cloned only.

  @ex The following example demonstrates how to use this function |
	struct rtl_mBuf *n;

	n = mBuf_split(m, off, MBUF_DONTWAIT);
	if (n == NULL) {
		goto bad;
	}
  @xref
  <c mBuf_cat>, <c mBuf_padding>, <p mBuf_getm>

 */

void mBuf_getBMjmpTable(uint8 *pat,  uint32 *jump_tbl,uint32 patLen);
int32 mBuf_BMpatternMatch(struct rtl_mBuf *m, uint32 len, uint8 *delimiter, uint32 delimitLen, uint32 *jmp_tbl);

#define MBUF_GETLEN(m)	 ((m)? ((m)->m_len : -1)
/*
@func MACRO	| MBUF_GETLEN	| Get total number of data bytes in the mbuf <p m>
@parm struct rtl_mBuf *	|m		| Indicate the packet
@rdesc Returns data length of the mbuf
@rvalue -1		| 	Failed.
@rvalue <p length>| 	The actual length of data in <p m>
@comm
 Get total number of data bytes in the mbuf <p m>
@xref  <c mBuf_getPktlen>
*/


#define MBUF_SET_PKTHDRFLAGS(m, Flags) do{\
	assert((m));\
	assert(ISSET((m)->m_flags, MBUF_USED));\
	assert((m)->m_pkthdr);\
	MBUF_SETPKTHDRFIELD16(((memaddr *)&(m)->m_pkthdr->ph_flags), (Flags));\
 }while(0)


/*
@func MACRO	| MBUF_SET_PKTHDRFLAGS	| Set packet specific flags for ASIC processing
@parm struct rtl_mBuf *	|m		| Indicate the packet
@parm uint32	|Flags		| flag to be set
@rdesc None
@comm
Set packet specific flags for ASIC processing. This overwrites original flag value. If you are adding flag bits rather than reseting it,
remember to save its old value first

@xref  <c MBUF_GET_PKTHDRFLAGS>
*/


#define MBUF_GET_PKTHDRFLAGS(m)	(MBUF_CHECKPKTHDR((m))? MBUF_GETPKTHDRFIELD16((memaddr *)(&m->m_pkthdr->ph_flags)):-1)
/*
@func MACRO	| MBUF_GET_PKTHDRFLAGS	| Get packet specific flags set by ASIC
@parm struct rtl_mBuf *	|m		| Indicate the packet
@rdesc Returns the result of execution
@rvalue -1		| 	Failed. <p m> might have no pkthdrs.
@rvalue FLAGS	| 	Returns the flags.
@comm
Get packet specific flags set by ASIC

@xref  <c MBUF_SET_PKTHDRFLAGS>
*/

#define MBUF_SET_PORTLIST(m,Portlist)  do{\
	assert((m));\
	assert(ISSET((m)->m_flags, MBUF_USED));\
	assert((m)->m_pkthdr);\
	(m)->m_pkthdr->ph_portlist = (Portlist);\
}while(0)

/*
@func MACRO	| MBUF_SET_PORTLIST	| Set outgoing port list
@parm struct rtl_mBuf *	|m		| Indicate the packet
@parm uint32	|Portlist		| port list
@rdesc None
@comm  Set outgoing port list
@xref  <c MBUF_GET_PORTLIST>
*/

#define MBUF_GET_PORTLIST(m) (MBUF_CHECKPKTHDR((m))? ((m)->m_pkthdr->ph_portlist):-1)
/*
@func MACRO	| MBUF_GET_PORTLIST	| get incoming port list
@parm struct rtl_mBuf *	|m		| Indicate the packet
@rdesc Returns the result of execution
@rvalue 0		| 	Failed. <p m> might have no pkthdrs.
@rvalue Portlist	| 	Returns incoming portlist.
@comm
 get incoming port list

@xref  <c MBUF_SET_PORTLIST>
*/


#define MBUF_SET_TRAPREASON(m, Reason)	do{\
	assert((m));\
	assert(ISSET((m)->m_flags, MBUF_USED));\
	assert((m)->m_pkthdr);\
	MBUF_SETPKTHDRFIELD16(((memaddr *)&(m)->m_pkthdr->ph_reason), (Reason));\
}while(0)

/*
@func MACRO	| MBUF_SET_TRAPREASON	| Set packet trapping reason for ASIC processing
@parm struct rtl_mBuf *	|m		| Indicate the packet
@parm uint16	|Reason		| Trapping reason
@rdesc None
@comm
Set packet trapping reason for ASIC processing

@xref  <c MBUF_GET_TRAPREASON>
*/

#define MBUF_GET_TRAPREASON(m)	(MBUF_CHECKPKTHDR((m))? MBUF_GETPKTHDRFIELD16((memaddr *)(&m->m_pkthdr->ph_reason)):-1)
/*
@func MACRO	| MBUF_GET_TRAPREASON	| Get packet trapping reason set by ASIC
@parm struct rtl_mBuf *	|m		| Indicate the packet
@rdesc Returns the result of execution
@rvalue -1		| 	Failed. <p m> might have no pkthdrs.
@rvalue Reason	| 	Returns trap reason
@comm
Get packet trapping reason set by ASIC

@xref  <c MBUF_SET_TRAPREASON>
*/

#define	MBUF2DATAPTR(m, t)	((t)((m)->m_data))
/*
@func MACRO	| MBUF2DATAPTR	| Given mbuf pointer 'm', returns address of m->m_data, and convert it to type 't'
@parm struct rtl_mBuf *	|m		| Pointer to an mbuf
@parm TYPE	|t		| A type to be casted
@rdesc Address of m->m_data
@comm
Given mbuf pointer 'm', returns address of m->m_data, and convert it to type 't'
@xref  <c mBuf_data2Mbuf>
*/


#define	MBUF_ALIGN(m, len) do {	\
	assert((m)->m_len==0);\
	assert(len>0);\
	assert(((m)->m_extsize - (len))>0);\
	(m)->m_data += ( (m)->m_extsize - (len)) & ~(sizeof(memaddr) - 1);	\
} while (0)

/*
@func MACRO	| MBUF_ALIGN	| Align the m->m_data pointer from end of the cluster, for 'len' bytes.
@parm struct rtl_mBuf *	|m		| Pointer to an mbuf
@parm uint32	|len		| Size to be aligned.
@rdesc None
@comm
Set the m_data pointer of a NEWLY-allocated cluster  to place
an object of the specified size at the end of the mbuf, longword aligned.
@xref  <c MBUF_RESERVE>
*/


#define	MBUF_RESERVE(m, len) do {	\
	assert((m)->m_len==0);\
	assert(len>=0);\
	(m)->m_data += ( ((len) > (m)->m_extsize)? (m)->m_extsize : len);	\
} while (0)
/*
@func MACRO	| MBUF_RESERVE	| Reserve 'len' bytes from the beginning of the newly allocated cluster
@parm struct rtl_mBuf *	|m		| Pointer to an mbuf
@parm uint32	|len		| Size to be aligned.
@rdesc None
@comm
Forward move the m_data pointer of a NEWLY-allocated cluster  for 'len' bytes.
m_data won't move beyond m_extbuf + m_extsize, make sure 'len' you give is a reasonable value
@xref  <c MBUF_ALIGN>
*/

#endif			   /* !_MBUF_H_ */
