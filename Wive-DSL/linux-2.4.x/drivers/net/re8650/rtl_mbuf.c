
/*
* Copyright c                  Realtek Semiconductor Corporation, 2002
* All rights reserved.
*
* Program : The mbuf module implementation
* Abstract :
* Author : David Chun-Feng Liu (cfliu@realtek.com.tw)
* $Log: rtl_mbuf.c,v $
* Revision 1.1.1.1  2003/08/18 05:43:22  kaohj
* initial import into CVS
*
* Revision 1.1  2003/05/23 06:05:53  elvis
* initial version
*
* Revision 1.79  2003/05/22 08:45:28  cfliu
* use osglue_mbufMutexLock,Unlock
* Rename mBuf, pktHdr ==> rtl_mBuf, rtl_pktHdr
*
* Revision 1.78  2003/05/02 09:00:33  cfliu
* no message
*
* Revision 1.77  2003/05/02 08:47:42  cfliu
* define HAVE_POSIX_PTHREAD & HAVE_SYNCH_MODULE. If the former is undefined, user
* should provide his own mutex lock functions. If the latter is undefined, MBUF_WAIT
* is not supported since there is no semaphore post/wakeup mechanism.
* Replace all mutex lock/unlocks with mBuf_mutexLock/mBuf_mutexUnLock
*
* Revision 1.76  2003/04/16 10:28:54  cfliu
* Global renaming for mbuf API and symbols
*
* Revision 1.75  2003/04/01 10:26:10  cfliu
* Workaround a possible(??) bug in mBuf_prepend, I don't think it's a bug in mbuf module.
*
* Revision 1.74  2003/02/27 14:28:05  hiwu
* remove syntax error
*
* Revision 1.73  2003/02/27 10:24:03  cfliu
* use _mbufMutex to replace all splnet() calls. mbuf module doesn't share splnet with protocol stack anymore
*
* Revision 1.72  2003/01/21 05:56:21  cfliu
* Remove redundant header file inclusion...still need closer look on them..
*
* Revision 1.71  2002/12/24 08:01:42  cfliu
* Remove nasty assertion in _pvGetFreeBuffer and _vReturnFreeBuffer
*
* Revision 1.70  2002/12/24 07:56:53  cfliu
* Wrap all splimp()...splx() calls out of function body if that function is called within another mbuf API to eliminate unneccessary spinlock()'s
*
* Revision 1.69  2002/12/10 11:47:59  cfliu
* Add mBuf_init(). mBuf_init() is used only in init.c
*
* Revision 1.68  2002/11/29 06:25:55  cfliu
* Fix several bugs regarding mBuf_freeMbufChain()
*
* Revision 1.67  2002/11/28 05:15:34  cfliu
* Fix GCC 3.2 compile warnings
*
* Revision 1.66  2002/11/07 04:08:24  cfliu
* no message
*
* Revision 1.65  2002/11/05 09:24:57  cfliu
* Add protection to avoid multiple initialization
* Add _vFreePkthdr
* Max mbuf chain length is 65535, not 65536 since it overflows ph_len (16 bits)
* mBuf_prepend, mBuf_padding accepts 0 byte 'plen'
* Enforce max mbuf chain size limit on mBuf_copyToMbuf, mBuf_cat,
* Protection from adjusting too much for mBuf_adjHead and mBuf_adjTail. Excesss request bytes ignored and returns first mbuf.
* It's ok to trim an mbuf chain with plen > pktlen, the whole mbuf chain is trimmed.
* Remove m_checkSum16 since we never use it till now.
* Remove all NDEBUG compile flag for memcpy()
*
* Revision 1.64  2002/11/01 08:51:10  cfliu
* Fix compile warnings on Greenhill compiler
* Constrain each mbuf chain to max 64K.
*
* Revision 1.63  2002/10/30 03:35:41  elvis
* clear padding bytes for mBuf_padding()
*
* Revision 1.62  2002/10/25 10:30:04  cfliu
* Bugfix for mBuf_copyToMbuf()
*
* Revision 1.61  2002/10/23 04:26:48  cfliu
* Add pattern matching feature. Given a pattern string, mBuf_BMpatternMatch() use Boyer-Moore algorithm and lookup content
* in mbuf chain for first occurance. Required by SO_RCVDELIMITER socket option.
*
* Revision 1.60  2002/10/15 06:44:47  cfliu
* Fix mBuf_driverFreePkthdr prototype problem
*
* Revision 1.59  2002/10/14 10:41:02  cfliu
* Fix a typo bug in mBuf_freeOne()
*
* Revision 1.58  2002/10/14 08:07:13  cfliu
* Update mbuf documentation, remove redundant code, add more comments...
*
* Revision 1.57  2002/10/12 09:07:46  henryCho
* Open driver mbuf api protection
*
* Revision 1.56  2002/10/12 07:21:17  cfliu
* Update m_driverXXX APIs
*
* Revision 1.55  2002/10/12 06:28:56  henryCho
* Move header next field to driver reserved field
*
* Revision 1.54  2002/10/12 06:11:13  cfliu
* no message
*
* Revision 1.53  2002/10/12 05:38:06  cfliu
* Remove m_XXXIntr() and add new m_driverXXX APIs.
* m_driverXXX APIs are for driver use only and splimp() is accquired inside them.
*
* Revision 1.52  2002/10/11 08:27:55  cfliu
* User 'register' modifier for mBuf_get, mBuf_freeOne, m_getPkthdr functions...
*
* Revision 1.51  2002/10/09 11:00:18  cfliu
* Rename m_copym as mBuf_cloneMbufChain, m_dup as mBuf_dupPacket, m_copypacket as mBuf_clonePacket
* Add mBuf_dupMbufChain.
*
* Revision 1.50  2002/10/09 06:08:18  cfliu
* replace all printf() with clePrintf()
*
* Revision 1.49  2002/10/09 04:57:19  cfliu
* Replace all cle_printf() with clePrintf().
* Via reent_getCleUserId(), clePrintf() knows which user id to print output string
*
* Revision 1.48  2002/10/01 05:15:12  henryCho
* Change proto-type
*
* Revision 1.47  2002/09/26 05:24:07  henryCho
* Enable APIs for interrupt.
*
* Revision 1.46  2002/09/26 01:45:58  danwu
* Use UNCACHED_MALLOC() to allocate pkthdr and mbuf pools.
*
* Revision 1.45  2002/09/24 04:23:47  cfliu
* Use POSIX condition variable to implement broadcast channel for functions such as select()
* synch API: synch_reserveChannel(), synch_sleep() prototype changed, if CHANNEL_BROADCAST flags is
* set when being called, the corresponding channel would, when signalled, wake up all
* waiting threads rather than only one.
*
* Revision 1.44  2002/09/20 07:36:38  elvis
* add mBuf_padding()
*
* Revision 1.43  2002/08/28 11:20:06  cfliu
* mBuf_adjHead() returns NULL (not input parameter 'mp' anymore)  for  both req_len>=pktlen cases.
*
* Revision 1.42  2002/08/28 07:29:53  cfliu
* Fix a bug in mBuf_pullup. When iLen == pkt length, function should return directly.
*
* Revision 1.41  2002/08/26 03:37:08  cfliu
* Remove uncritical msgLog_log()'s
*
* Revision 1.40  2002/08/21 05:28:18  cfliu
* Remove all _iMbuf*() functions and remove all m_getPkt*() functions to mbuf.h.
* m_getPkt*() functions are now declared as macros.
*
* Revision 1.39  2002/08/20 05:49:16  danwu
* Change to RTL_EXTERN_INLINE & RTL_STATIC_INLINE macros.
*
* Revision 1.38  2002/08/20 02:23:34  cfliu
* Remove extern qualifier from inline function prototype
*
* Revision 1.37  2002/08/19 11:39:35  danwu
* Change inline to __inline__.
*
* Revision 1.36  2002/08/15 03:42:43  hiwu
* fix mBuf_pullup leading space size in-correct
*
* Revision 1.35  2002/08/12 14:15:30  hiwu
* add mBuf_pullup base on leading space
*
* Revision 1.34  2002/08/12 11:28:46  cfliu
* Remove if_XXX.c
*
* Revision 1.32  2002/08/05 08:41:31  cfliu
* Fix bugs in mBuf_trimHead and mBuf_trimTail. Test code also updated and tested (Test 8 and Test 9 in mbuf_test.c)
*
* Revision 1.30  2002/08/02 02:49:54  cfliu
* Fix m_getMbuflen and add M_GETNEWMBUF flag to mBuf_prepend
*
* Revision 1.29  2002/08/01 09:02:05  cfliu
* Remove memSet and memCopy.  memSet and memCopy no longer in source tree now
*
* Revision 1.28  2002/07/29 02:22:15  cfliu
* Fixed a bug in mBuf_prepend, test code also updated.
*
* Revision 1.27  2002/07/27 14:02:36  cfliu
* Fixed several bugs in:
* 1. m_copym: a)forgot to set pNewMbuf->m_extsize, check of mBuf_get(...)!=NULL, should be ==
* 2. mBuf_prepend: Failed when prepending an mbuf before an mbuf with no leading space and has a pkthdr.
*
* Revision 1.26  2002/07/26 11:09:26  cfliu
* Update mBuf_freePkthdr. TCP now use this function to remove pkthdr before appending segment data onto recv buffer.
*
* Revision 1.25  2002/07/26 01:46:57  cfliu
* struct rtl_mBufStatus changed.
*
* Revision 1.24  2002/07/23 08:59:31  cfliu
* Fix bug in mBuf_copyToMbuf when 'len' > ph_len
*
* Revision 1.21  2002/07/19 06:27:49  cfliu
* Replace all memCopy calls with memcpy in standard library
*
* Revision 1.20  2002/07/03 09:19:58  cfliu
* Removed all standard header files from source code. They would be included by <core/types.h>-><rtl_depend.h>
*
* Revision 1.19  2002/06/24 01:18:28  cfliu
* fix a bug in mBuf_getPkthdr: when mBuf_getPkthdr is called with an mbuf chain 'm', pkt length is not calculated accurately.
*
* Revision 1.18  2002/06/19 09:36:43  cfliu
* Add assertion to verify that each mbuf API call is passed with an in-use mbuf.
*
* Revision 1.14  2002/06/12 09:22:18  cfliu
* Mbuf now supports variable cluster length. Cluster size is given by user during mBuf_init
* A new parameter "clusterSize" is added for mBuf_init
*
* Revision 1.13  2002/06/11 04:19:59  cfliu
* BSD tsleep/wakeup is not used anymore. Replaced with synch_XXX APIs
*
* Revision 1.12  2002/06/07 14:06:46  cfliu
* Add mBuf_get, m_getPkthdr, mBuf_freeMbufChain routines for ISR (Currently inactive)
*
* Revision 1.10  2002/06/05 03:42:55  cfliu
* Add mbstat structure and a new function m_getBufStat() for user to get mbuf module status
*
* Revision 1.9  2002/05/27 07:01:03  cfliu
* handle tsleep() API definition change issue
*
* Revision 1.8  2002/05/24 07:57:58  hiwu
* fix m_copym() always return zero pointer
*
* Revision 1.6  2002/05/16 05:21:54  cfliu
* 1.Redesign pkthdr structure. ASIC irrevelant fields in pkthdr are now grouped with union structure so these
* fields may be reused by different software modules.
* 2.m_getNextpkt() and m_setNextpkt() removed.
*
* Revision 1.5  2002/05/15 08:40:45  cfliu
* Add mBuf_freePkthdr() so pkthdr could be freed alone.
* Note this is not the normal way to free a pkthdr attached with mbuf chains.
*
* Revision 1.4  2002/05/15 05:59:15  cfliu
* Add m_getPkthdr() so pkthdrs could be allocated alone
*
* Revision 1.2  2002/04/29 10:08:49  hiwu
* remove interfacec set and get  api
*
* Revision 1.16  2002/04/25 03:57:02  cfliu
* Modifies mBuf_init() initialization code
*
* Revision 1.14  2002/04/19 09:49:01  cfliu
* A major update. m_adj is separated to 4 functions: mBuf_adjHead, mBuf_adjTail, mBuf_trimHead, and mBuf_trimTail.
* mBuf_prepend is modified so data pointer moves and total packet length increases. Other changes including
* mBuf_get/setXXX APIs have thir prototypes changed. mbuf_test.c is rewritten and organized to make adding
* new testing scenarios more easily.
*
* Revision 1.11  2002/04/16 07:58:49  cfliu
* modify m_setNextPkt, m_getNextPkt
*
* Revision 1.10  2002/04/12 08:57:40  cfliu
* merge m_adj and m_trim. mBuf_prepend will now adjust m_data to reflect new data space accquired.
* However, these functions need undergo extensive test and I will clean up mbuf_test.c as soon as possible.
* This is NOT a stable release.
*
* Revision 1.9  2002/04/11 10:18:59  cfliu
* Add m_checkSum16() and declare mBuf_get/setXXX functions as inline function.
*
* Revision 1.8  2002/04/10 14:35:19  cfliu
* add APIs to set/get pkthdr structure.
*
* Revision 1.7  2002/04/04 10:47:28  cfliu
* Use 'indent' tool to unify coding style....
* The options used could be found on /product/cygtest/.indent.pro
*
* Revision 1.5  2002/03/30 04:26:48  cfliu
* TODO: m_trim not yet tested....
*
* Revision 1.4  2002/03/29 13:28:18  cfliu
* Update code which used renamed functions in memutils.c
*
* Revision 1.2  2002/03/21 05:29:18  cfliu
* All primitive data types replaces with uint32, uint16, ...etc
*
* -------------------------------------------------------
*/
//#define HAVE_POSIX_PTHREAD  //undefine it if POSIX interface not in release package


#include "assert.h"
#include "rtl_types.h"
#include "rtl_mbuf.h"
#include "osglue.h"
#ifdef HAVE_SYNCH_MODULE
#include <core/synch.h>
#endif
/*TODO: 1. Support not yet provided if users want to allocate their own clusters
  *		2. Check what may happen for zombie clusters.
  *	Support:
  *		Test  mBuf_get/m_getPkthdr/m_freemfor ISR.
  */

/*********************************************************************************
	Section 1:	Exported public global variables
**********************************************************************************/
uint32 m_clusterSize;	//Size of each cluster
/*********************************************************************************
	Section 2:	Internal structures and variables, see local mbuf_local.h
**********************************************************************************/
#include "rtl_mbuf_local.h"

static int32 _mbInited=0;

/*********************************************************************************
	Section 3:	Exported API prototype: Defined in mbuf.h
**********************************************************************************/
/*********************************************************************************
	Section 4:	Internal function prototype
**********************************************************************************/
static int32 _iTryGet(uint32 * piAvailableBuffers, uint32 iRequestedBuffers,  uint32 * piWaitID, int32 iHow, uint32 * piHaveWaited);
static uint32 _iFreeMbufChain(struct rtl_mBuf *pFirstMbuf, uint32 iNum);
static void _vWakeup(uint32 * piWaitID);
static void _vInitBufferPools(void *pBufPool, int32 iSize, int32 iNum);
RTL_STATIC_INLINE  void _vFreePkthdr(struct rtl_pktHdr *ph);
RTL_STATIC_INLINE uint8 _iGetClusterDataByte(struct rtl_mBuf *m, uint32 offset, uint32 *offsetTbl, uint32 *lenTbl, uint32 mbufs);
RTL_STATIC_INLINE void _vReturnFreeBuffer(void *pvFreeBuffer,  struct poolstat *pBufPoolStat);
RTL_STATIC_INLINE void *_pvGetFreeBuffer(struct poolstat *pBufPoolStat);
static struct rtl_mBuf *_pCopyMbufChain(struct rtl_mBuf *pMbufChain, int32 iOffset, int32 iLength,	 int32 flag);
static struct rtl_mBuf *_pCopyPacket(struct rtl_mBuf *pMbuf, uint32 iHow);
/*********************************************************************************
	Section 5:	Exported API implementation
**********************************************************************************/
#define UNCACHED_MALLOC(x)  (void *) (0x20000000 | (uint32) malloc(x))

int32 isPowerOf2(uint32 number, uint32 *exponent){
	uint32 size, bits;	
	if(exponent){
		//user is not interested in the exponent
		for(bits=0, size=1; bits < (sizeof(uint32)<<3);bits++){
			if(number==size)
				break;
			else
				size = size << 1;
		}
		if(bits == sizeof(uint32)*8 )
			return FALSE;
		*exponent = bits;
		return TRUE;
	}else if(((number-1)&number)==0)
		return TRUE;
	 return FALSE;
}



/* Initialize all mbuf module internal structures and variables. See mbuf.h */
int32 mBuf_init(uint32 nmbufs, uint32 nmbclusters, uint32 nmpkthdrs, uint32 clusterSize, memaddr logId)
{
	uint32 exponent;

	if ( _mbInited!=0)
		return FAILED;


	osglue_mbufMutexLock();

	_iTotalMbufs = nmbufs;
	_iTotalClusters = nmbclusters;
	_iTotalPkthdrs = nmpkthdrs;
	_mbufMsgLogId = logId;

	//clusterSize must be power of 2
	if((isPowerOf2(clusterSize, &exponent)==FALSE)||(clusterSize < sizeof(struct mcluster_tag)))
		return FAILED;

	m_clusterSize = clusterSize;
	m_clusterShift = exponent;

	_MbufPool= UNCACHED_MALLOC(_iTotalMbufs *sizeof(struct rtl_mBuf));	/* mbuf pool */
	_ClusterDataPool= UNCACHED_MALLOC(_iTotalClusters * clusterSize); /* cluster data */
	_ClusterTag= malloc(_iTotalClusters * sizeof(struct mcluster_tag)); /* cluster data */
	_PkthdrPool= UNCACHED_MALLOC(_iTotalPkthdrs * sizeof(struct rtl_pktHdr)); /* cluster data */

	if(!_MbufPool || !_ClusterDataPool || !_ClusterDataPool || !_PkthdrPool)
		return FAILED;

	/*
	   Clears content and chain neighboring elements. Own bits set to BUF_FREE(0x00) implicitly
	 */
	_vInitBufferPools(_MbufPool, sizeof(struct rtl_mBuf), _iTotalMbufs);
	_vInitBufferPools(_ClusterTag, sizeof(struct mcluster_tag),
					  _iTotalClusters);
	_vInitBufferPools(_PkthdrPool, sizeof(struct rtl_pktHdr), _iTotalPkthdrs);

	/*
	   Initialize Pool status variables
	 */
	memset((void *) &_MbufStatus, (uint8) 0,   sizeof(struct poolstat));
	memset((void *) &_ClStatus, (uint8) 0,   sizeof(struct poolstat));
	memset((void *) &_PHStatus, (uint8) 0,   sizeof(struct poolstat));
	/*
	   Initialize Data pool
	 */
	memset((void *) _ClusterDataPool, (uint8) 0,
		   _iTotalClusters * m_clusterSize);

	_MbufStatus.pvAddr = _MbufStatus.pvFreelist = _MbufPool;
	_MbufStatus.iTotalSize = _MbufStatus.iFreebufs = _iTotalMbufs;

	_ClStatus.pvAddr = _ClStatus.pvFreelist = _ClusterTag;
	_ClStatus.iTotalSize = _ClStatus.iFreebufs = _iTotalClusters;

	_PHStatus.pvAddr = _PHStatus.pvFreelist = _PkthdrPool;
	_PHStatus.iTotalSize = _PHStatus.iFreebufs = _iTotalPkthdrs;


	memset((void *) &_Statistics, (uint8) 0, sizeof(struct rtl_mBufStatus));
	_Statistics.m_msize = sizeof(struct rtl_mBuf);	   /* length of an mbuf */
	_Statistics.m_pkthdrsize = sizeof(struct rtl_pktHdr);
	_Statistics.m_mclbytes=m_clusterSize;
	_mbInited = 1;
	osglue_mbufMutexUnlock();
	return SUCCESS;
}

/* Get mbuf module buffer pool status , see mbuf.h*/
int32 mBuf_getBufStat(struct rtl_mBufStatus *mbs){
	if (!mbs || _mbInited!=1)
		return FAILED;
	mbs->m_totalmbufs=_iTotalMbufs;
	mbs->m_totalclusters=_iTotalClusters;
	mbs->m_totalpkthdrs=_iTotalPkthdrs;
	mbs->m_freembufs=  _MbufStatus.iFreebufs;	   /* mbufs obtained from page pool */
	mbs->m_freeclusters=_ClStatus.iFreebufs;	   /* clusters obtained from page pool */
	mbs->m_freepkthdrs=_PHStatus.iFreebufs;	   /* number of free pkthdrs */
	mbs->m_msize = _Statistics.m_msize;		   /* length of an mbuf */
	mbs->m_mclbytes=_Statistics.m_mclbytes;	   /* length of an mbuf cluster */
	mbs->m_pkthdrsize= _Statistics.m_pkthdrsize;   /* length of an pkthdr */
	mbs->m_wait= _Statistics.m_wait;	  /* times waited for space, includes mbuf, pkthdr and cluster */
	return SUCCESS;
}

/*
 * Compute the amount of space available
 * before the current start of data in a cluster.
 */
int32 mBuf_leadingSpace(struct rtl_mBuf * m)
{
	int32 retval;

	assert(m &&	ISSET(m->m_flags, MBUF_USED) && (m->m_flags & MBUF_EXT));
	retval = (int32)(m->m_data - m->m_extbuf);

	return retval;
}

/*
 * Compute the amount of space available
 * after the end of data in an cluster.
 */
int32 mBuf_trailingSpace(struct rtl_mBuf * m)
{
	int32 retval;

	assert(m && ISSET(m->m_flags, MBUF_USED) &&(m->m_flags & MBUF_EXT));
	retval =  (( (memaddr)m->m_extbuf + m->m_extsize) - ((memaddr)m->m_data + m->m_len));

	return retval;

}

/*
 * Check if we can write to the cluster. Only the owner of this cluster can write.
 */
int32 mBuf_clusterIsWritable(struct rtl_mBuf * m)
{
	int32 retval;

	assert(m && ISSET(m->m_flags, MBUF_USED));
	if (CLDataAddr2TagAddr(m->m_extbuf)->cltag_mbuf == m)
		retval = (CLDataAddr2TagAddr(m->m_extbuf)->cltag_refcnt == 1);
	else
		retval = 0;

	return retval;

}

/*Get packet length. see mbuf.h */
static uint32 _m_getPktlen(struct rtl_mBuf *mp){
	struct rtl_mBuf *m=mp;
	uint32 pktlen=0;
	assert(mp);
	assert(ISSET(m->m_flags, MBUF_USED));

	//calculate total pktlen
	if(m->m_pkthdr)	//If there is a packet header, use ph_len directly
		pktlen = m->m_pkthdr->ph_len;
	else{
		pktlen = 0;	//since we don't assume pkthdr exists, we calculate pktlen manually via 'pktlen'
		for (;m;)		//find the last mbuf and pktlen pktlen
		{
			pktlen += m->m_len;
			if (m->m_next == (struct rtl_mBuf *) 0){
				m=mp;
				break;
			}
			m = m->m_next;
		}
	}
	return pktlen;
}

uint32 mBuf_getPktlen(struct rtl_mBuf *mp){
	uint32 ret;
	osglue_mbufMutexLock();
	//we need lock becoz we will traverse linked list.
	ret = _m_getPktlen(mp);
	osglue_mbufMutexUnlock();
	return ret;
}

// Given data addr(Always in cluster) -->return mbuf addr
struct rtl_mBuf *mBuf_data2Mbuf(int8 * x)
{
	struct rtl_mBuf *retval;
	assert(x);
	retval = CLDataAddr2TagAddr(x)->cltag_mbuf;
	return retval;
}


static struct rtl_mBuf *_m_get(int32 how, int32 type, uint32 Nbuf)
{
	register int32      iIndex=0, iHow;
	register struct rtl_mBuf *pThisMbuf, *pFirstMbuf, *pPrevMbuf;
	register struct mcluster_tag *pThisClusterTag;
	uint32 iMbufWaited;
	uint32 iClusterWaited;
	assert(how);
	assert(Nbuf > 0);

	pThisMbuf = pFirstMbuf = pPrevMbuf = NULL;
	pThisClusterTag = NULL;
	iMbufWaited = iClusterWaited = FALSE;

	iHow = how;

	//see if we can get all mbufs. Max mbuf chain size is 64K.
	//Note: An mbuf chain can NEVER grow up to 65536 bytes, this overflows ph_len.
	if ((Nbuf>_iTotalMbufs)||(Nbuf*m_clusterSize>65536))
		goto out;
	if(_iTryGet(&_MbufStatus.iFreebufs, Nbuf, &_MbufStatus.iWaiting, iHow,&iMbufWaited) == FALSE)
		goto out;

	//Do we need clusters? see if we can get all clusters
	if (ISCLEARED(iHow, MBUF_ONLY))	{

		//see if we can get all clusters
		if ((Nbuf>_iTotalClusters)||
			(_iTryGet(&_ClStatus.iFreebufs, Nbuf, &_ClStatus.iWaiting, iHow,&iClusterWaited) == FALSE))
				goto out;
	}
	CLEARBITS(iHow, MBUF_WAITOK);	   //Clear wait bit.

	/* We now have enough mbufs and clusters(if MBUF_ONLY isn't set)  */

	if (ISSET(iHow, MBUF_ONLY)){	//user want mbuf only. Maybe it would be used to reference other allocated clusters or maybe user wishes to manage his own cluster pool
		for (iIndex = 0; iIndex < Nbuf; iIndex++){		   // Initialize all Nbuf mbufs
			assert(_MbufStatus.pvFreelist);
			pThisMbuf = (struct rtl_mBuf *) _pvGetFreeBuffer(&_MbufStatus);
			assert(pThisMbuf);
			memset((void *) pThisMbuf, (uint8) 0, sizeof(struct rtl_mBuf));	//Initialize mbuf to 0
			CLEARBITS(pThisMbuf->m_flags, MBUF_EXT);
			SETBITS(pThisMbuf->m_flags, MBUF_USED);
			pThisMbuf->m_type = type;
			pThisMbuf->m_next = NULL;
			_iMbufTypes[type]++;

			if (iIndex == 0)
				pFirstMbuf = pThisMbuf;	/* Return this pointer to user */
			else
				pPrevMbuf->m_next = pThisMbuf;	/*Chain together */
			pPrevMbuf = pThisMbuf;
		}
	}else{
		//Normal case, user wants both an mbuf and a cluster
		for (iIndex = 0; iIndex < Nbuf; iIndex++)
		{							   // Initialize all Nbuf mbufs
			assert(_MbufStatus.pvFreelist);
			pThisMbuf = (struct rtl_mBuf *) _pvGetFreeBuffer(&_MbufStatus);
			assert(_ClStatus.pvFreelist);
			pThisClusterTag =	(struct mcluster_tag *) _pvGetFreeBuffer(&_ClStatus);
			assert(pThisMbuf && pThisClusterTag);

			//Initializing mbuf*/
			memset((void *) pThisMbuf, (uint8) 0, sizeof(struct rtl_mBuf));
			SETBITS(pThisMbuf->m_flags, MBUF_EXT | MBUF_USED);
			pThisMbuf->m_type = type;
			pThisMbuf->m_next = NULL;
			_iMbufTypes[type]++;

			pThisMbuf->m_extbuf = pThisMbuf->m_data =
				(uint8 *) CLTagAddr2DataAddr(pThisClusterTag);
			pThisMbuf->m_len = 0;
			pThisMbuf->m_extsize = m_clusterSize;

			/*initialize corresponding cluster tag, cluster is NOT cleared to save time*/
			pThisClusterTag->cltag_refcnt = 1;
			SETBITS(pThisClusterTag->cltag_flags, CLUSTER_USED);
			pThisClusterTag->cltag_mbuf = pThisMbuf;	/* points back to pThisMbuf */

			if (iIndex == 0)
				pFirstMbuf = pThisMbuf;	/* Return this pointer to user */
			else
				pPrevMbuf->m_next = pThisMbuf;	/*Chain together */
			pPrevMbuf = pThisMbuf;

		}
	}

	/*
	   See if we can wakeup another one
	 */
	if (iClusterWaited == TRUE && _ClStatus.pvFreelist)	{
		_Statistics.m_wait++;
		_vWakeup(&_ClStatus.iWaiting); /* wakeup another one */
	}
	else if (iMbufWaited == TRUE && _MbufStatus.pvFreelist){
		_Statistics.m_wait++;
		_vWakeup(&_MbufStatus.iWaiting);	/* wakeup another one */
	}

out:

	if (iIndex == Nbuf)
		return (pFirstMbuf);		   //Return success only when all requested buffers allocated

	return NULL;					   //Return Fail
}

/*Allocate mbuf and clusters. See document in mbuf.h*/
 struct rtl_mBuf *mBuf_get(int32 how, int32 type, uint32 Nbuf){
	struct rtl_mBuf *m;
	osglue_mbufMutexLock();
	m=_m_get( how,  type,  Nbuf);
	osglue_mbufMutexUnlock();
	return m;
}


/*Same as mBuf_get,but initialize all clusters to zero */
struct rtl_mBuf *mBuf_getCleared(int32 how, int32 type, uint32 Nbuf)
{
	register struct rtl_mBuf *m, *n;
	int32     i = 0;

	assert(Nbuf);
	osglue_mbufMutexLock();
	m = _m_get(how, type, Nbuf);
	if (m){
		assert(ISSET(m->m_flags, MBUF_USED));

		if(ISSET(how, MBUF_ONLY))
			goto out;

		//Clear the content in clusters allocated.
		assert(ISCLEARED(how, MBUF_ONLY));
		for (n = m, i = 0; i < Nbuf; i++, n = n->m_next)
			memset(MBUF2DATAPTR(n, int8 *), 0, m->m_extsize);
	}
out:
	osglue_mbufMutexUnlock();
	return (m);
}


/*Given an mbuf, allocate a pkthdr for it. See document in mbuf.h */
static struct rtl_mBuf *_m_gethdr(struct rtl_mBuf *pMbuf, int32 how){
	int32 iLen = 0;
	struct rtl_mBuf *pThisMbuf = NULL;
	struct rtl_pktHdr *pPkthdr = NULL;
	uint32 PkthdrWaited;

	assert(how &&pMbuf);
	assert(ISCLEARED(pMbuf->m_flags, MBUF_PKTHDR));

	iLen = pMbuf->m_len;
	// Should there be an mbuf in given mbuf chain who holds a pkthdr, rob the pkthdr's ownership and makes pMbuf the owner.
	for (pThisMbuf = pMbuf->m_next; pThisMbuf; pThisMbuf = pThisMbuf->m_next)
	{
		//find a pkthdr owner if there is one
		if (ISSET(pThisMbuf->m_flags, MBUF_PKTHDR)){
			assert(ISSET(pPkthdr->ph_flags, PKTHDR_USED));
			assert(pPkthdr->ph_mbuf == pThisMbuf);
			pPkthdr = pThisMbuf->m_pkthdr;
			pMbuf->m_pkthdr = pPkthdr;
			pPkthdr->ph_mbuf = pMbuf;
			pPkthdr->ph_len += iLen;   //increment packet length
			SETBITS(pMbuf->m_flags, MBUF_PKTHDR);	// make pBuf the pkthdr owner
			for (; pThisMbuf && (pThisMbuf->m_next != NULL); pThisMbuf = pThisMbuf->m_next)
				CLEARBITS(pThisMbuf->m_flags, MBUF_PKTHDR);	//clear the rested mbufs' PKTHDR bit if exist
			return pMbuf;
		}
		iLen += pThisMbuf->m_len;
	}


	/*
	   No mbufs already have a pkthdr, so we need to allocate one.
	 */

	//see if we can get a pkthdr
	if ((!_PHStatus.pvFreelist) &&
		(_iTryGet(&_PHStatus.iFreebufs, 1, &_PHStatus.iWaiting, how, &PkthdrWaited) == FALSE))
			goto out;

	/*
	   Yes we can...
	 */
	assert(_PHStatus.pvFreelist);
	pPkthdr = (struct rtl_pktHdr *) _pvGetFreeBuffer(&_PHStatus);
	assert(pPkthdr);
	memset((void *) pPkthdr, (uint8) 0, sizeof(struct rtl_pktHdr));	//initialize pkthdr to 0
	SETBITS(pPkthdr->ph_flags, PKTHDR_USED);
	pPkthdr->ph_mbuf = pMbuf;
	pPkthdr->ph_len += iLen;
	pMbuf->m_pkthdr = pPkthdr;
	SETBITS(pMbuf->m_flags, MBUF_PKTHDR);	/* Now we are the head of this mbuf chain */

	//change all mbuf's m_pkthdr pointer
	for (pThisMbuf = pMbuf->m_next; pThisMbuf;pThisMbuf = pThisMbuf->m_next)
		pThisMbuf->m_pkthdr = pPkthdr;

	if (PkthdrWaited == TRUE && _PHStatus.pvFreelist){
		_Statistics.m_wait++;
		_vWakeup(&_PHStatus.iWaiting);	/* wakeup another one */
	}

out:
	if (pPkthdr)
		return pMbuf;				   //return success if a pkthdr is allocated
	else
		return NULL;
}


 struct rtl_mBuf *mBuf_getPkthdr(struct rtl_mBuf *pMbuf, int32 how){
 	struct rtl_mBuf *m;
	osglue_mbufMutexLock();
	m = _m_gethdr( pMbuf,  how);
	osglue_mbufMutexUnlock();
	return m;
	
 }


/*Free a pkthdr alone. This is usd in TCP, when appending incoming packet to TCP receive buffer and the pkthdr attached should be
removed.  See document in mbuf.h*/
void mBuf_freePkthdr(struct rtl_pktHdr *ph){
	osglue_mbufMutexLock();
	_vFreePkthdr(ph);
	osglue_mbufMutexUnlock();
}


/* This will allocate len-worth of  mbuf clusters and return a pointer to the top
 * of the allocated chain. See document in mbuf.h
 */
struct rtl_mBuf *mBuf_getm(struct rtl_mBuf *m, uint32 len, int32 how, int32 type){
	register struct rtl_mBuf *pMbuf = m, *mtail = NULL, *p=NULL;
	register int32     trailing_len = 0, Nbuf = 0;

	assert(how);
	assert(len);

	//Exceed maximum mbuf chain limit?
	if(len> 65535)
		return pMbuf;

	osglue_mbufMutexLock();

	if (m != NULL){	   //we are given an mbuf chain,  so we will attach new mbufs after given mbuf chain.
		assert(ISSET(m->m_flags, MBUF_USED));
		for (mtail = m; mtail->m_next != NULL; mtail = mtail->m_next) ;
		trailing_len = mBuf_trailingSpace(mtail);
		//mtail would be used later to concatenate old mbuf chain and new mbuf chain
		if (trailing_len >= len)	   // Trailing space is large enough, No need to allocate new mbufs
			goto out;
		len -= trailing_len;
	}

	//Calculate how many mbufs we need
	Nbuf = ((len - 1) / m_clusterSize) + 1;
	assert(Nbuf > 0);

	CLEARBITS(how, MBUF_ONLY);

	//Caution:pMbuf is reused here
	pMbuf = _m_get(how, type, Nbuf);

	if (!pMbuf)
		goto out;

	if (m != NULL)	{
		if(m->m_pkthdr){//if m has a pkthdr, so do all new mbufs
			for (p = pMbuf; p; p = p->m_next)
				p->m_pkthdr = m->m_pkthdr;
		}
		mtail->m_next = pMbuf;
		pMbuf = m;
	}else{
		//We only allocate a pkthdr when user says he wants one and
		//user didn't give us an mbuf to chain with m
		if (ISSET(how, MBUF_ALLOCPKTHDR) && (pMbuf != _m_gethdr(pMbuf, how))){
			_iFreeMbufChain(m, MBUF_FREEALL);
			pMbuf = NULL;
			goto out;
		}
		for (p = pMbuf; p; p = p->m_next)
			p->m_pkthdr = pMbuf->m_pkthdr;
	}
out:
	osglue_mbufMutexUnlock();
	return pMbuf;

}


/* Arrange to prepend space of size plen to mbuf m and adjust data pointers
 * If a new mbuf must be allocated, how specifies whether to wait.
 *  See document in mbuf.h
 */
struct rtl_mBuf *mBuf_prepend(struct rtl_mBuf *m, uint32 plen, int32 how){
	struct rtl_mBuf *pMbuf=m, *mm;
	int32     leading_len, Nbuf, start;
	int32	count=0;


	assert(m);
	assert(ISSET(m->m_flags, MBUF_USED));

	osglue_mbufMutexLock();
	//By default, m_prepedn uses any leading free space before allocating any new clusters.
	//But if M_GETNEWMBUF is set, mBuf_prepend allocates new mbufs directly.
	if (ISSET(how, MBUF_GETNEWMBUF))
		leading_len = 0;
	else
		leading_len = mBuf_leadingSpace(m);

	//Does leading free space size satisfies user's request?
	if (leading_len >= plen){
		m->m_data -= plen;
		m->m_len += plen;
		if(m->m_pkthdr)
			m->m_pkthdr->ph_len +=plen;
		assert(pMbuf==m);
		goto out;
	}
	if(leading_len){//Bug!! FIXME: 2003.4.1 (cfliu) if we remove this if(leading_len).... on QA board, 
				// then m->m_data would be set as 0x1!!! strange, strange, strange...
		//this mbuf is not large enough to hold all requesting 'plen' bytes...
		m->m_data -= leading_len;	//Move data pointer of 'm' to the first byte
		m->m_len += leading_len;	//Use up all available space in this cluster
		if(m->m_pkthdr)
			m->m_pkthdr->ph_len += leading_len;
		plen -= leading_len;
	}
	count = leading_len;


	//now allocate new clusters and mbufs
	Nbuf = ((plen - 1) / m_clusterSize) + 1;
	start = (Nbuf*m_clusterSize)-plen;

	CLEARBITS(how, MBUF_ONLY);

	//Caution: pMbuf is reused here..
	pMbuf = _m_get(how, m->m_type, Nbuf);
	if (!pMbuf)
		goto out;

	assert(pMbuf->m_data == pMbuf->m_extbuf);

	//find the last new mbuf and set pkthdr fields for all new mbufs if required
	for (mm = pMbuf; mm; mm = mm->m_next){
		//for all mbufs, use up all cluster space.
		assert(mm->m_len==0);
		if(mm==pMbuf){
			pMbuf->m_data += start;
			pMbuf->m_len = m->m_extsize - start; 	//Move data pointer of the first new mbuf
			count+= pMbuf->m_len;
		}else{
			mm->m_len = mm->m_extsize;//each mbuf except the first one is full-sized
			count+=mm->m_extsize;
		}
		if(m->m_pkthdr){	//if old mbuf chain has a pkthdr, all new mbufs aware it.
			mm->m_pkthdr = m->m_pkthdr;
			m->m_pkthdr->ph_len+=mm->m_len;
		}
		if(mm->m_next==NULL){
			mm->m_next = m;			//chain last new mbuf with first old mbuf
			if(m->m_pkthdr){
				SETBITS(pMbuf->m_flags, MBUF_PKTHDR);
				m->m_pkthdr->ph_mbuf = pMbuf;		//pkthdr now points to first new mbuf
				CLEARBITS(m->m_flags, MBUF_PKTHDR);
				assert(count==plen+leading_len);
				//m->m_pkthdr->ph_len += plen;	//adjust packet length
			}
			break;
		}
	}
out:
	osglue_mbufMutexUnlock();
	return pMbuf;
}


/*
 * Arrange to append space of size plen to mbuf m
 * See document in mbuf.h
 */
struct rtl_mBuf *mBuf_padding(struct rtl_mBuf *m, uint32 plen, int32 how){
	struct rtl_mBuf *pLastMbuf=NULL, *pLastMbufNext, *pNewMbuf, *mm;
	int32     trailing_len, Nbuf, end;
	int32	count=0;


	assert(m);
	assert(ISSET(m->m_flags, MBUF_USED));

	osglue_mbufMutexLock();
	// find the last mbuf in the list
	for(pLastMbuf=m, pLastMbufNext=m->m_next; pLastMbufNext; pLastMbuf=pLastMbuf->m_next, pLastMbufNext=pLastMbufNext->m_next); //No problem for ;

	trailing_len = mBuf_trailingSpace(pLastMbuf);

	//Does the last mbuf satisfies request?
	if (trailing_len >= plen){
		memset((void *)(pLastMbuf->m_data+pLastMbuf->m_len), 0, plen); // clear padding bytes
		pLastMbuf->m_len += plen;
		if(m->m_pkthdr)
			m->m_pkthdr->ph_len +=plen;
		goto out;
	}

	//this mbuf is not large enough to hold all requested 'plen' bytes
	memset((void *)(pLastMbuf->m_data+pLastMbuf->m_len), 0, trailing_len); // clear padding bytes
	pLastMbuf->m_len += trailing_len;	//Use up all available trailing space in this cluster
	if(m->m_pkthdr)
		m->m_pkthdr->ph_len += plen;
	count = trailing_len;

	plen -= trailing_len;

	//now allocate new clusters and mbufs
	Nbuf = ((plen - 1) / m_clusterSize) + 1;
	end = (Nbuf*m_clusterSize)-plen;

	CLEARBITS(how, MBUF_ONLY);
	pNewMbuf = _m_get(how, m->m_type, Nbuf);
	if (!pNewMbuf){
		osglue_mbufMutexUnlock();
		return NULL;
	}

	assert(pNewMbuf->m_data == pNewMbuf->m_extbuf);
	// append the newly allocated mbuf to the original list
	pLastMbuf->m_next = pNewMbuf;

	//find the last new mbuf and set pkthdr fields for all new mbufs if required
	for (mm = pNewMbuf; mm; mm = mm->m_next)
	{
		//for all mbufs, use up all cluster space.
		assert(mm->m_len==0);

		if(m->m_pkthdr){	//if old mbuf chain has a pkthdr, all new mbufs aware it.
			mm->m_pkthdr = m->m_pkthdr;
		}
		if(mm->m_next != NULL) {
			mm->m_len = mm->m_extsize;//each mbuf except the first one is full-sized
			memset((void *)mm->m_data, 0, mm->m_len); // clear padding bytes
			count+=mm->m_len;
		} else {/* last mbuf */
			mm->m_len = mm->m_extsize-end;
			memset((void *)mm->m_data, 0, mm->m_len); // clear padding bytes
			count+=mm->m_len;
			assert(count==plen);
			pLastMbuf = mm;
			break;
		}
	}
out:
	osglue_mbufMutexUnlock();
	return pLastMbuf;
}



/* Free a single mbuf and associated external storage. Place the successor, if any, in n.  See document in mbuf.h*/

struct rtl_mBuf *mBuf_freeOne(struct rtl_mBuf *m)
{

	struct rtl_mBuf *n = NULL;
#ifndef NDEBUG
	int32 retval;
#endif

	osglue_mbufMutexLock();
	assert(m);
	assert(ISSET(m->m_flags, MBUF_USED));
	if (m->m_next)
		n = m->m_next;
#ifndef NDEBUG
	retval = _iFreeMbufChain(m, 0);/* We might not be able to free this....Might need to check. */
	assert(retval==1);
#else
	 _iFreeMbufChain(m, 0);
#endif
	osglue_mbufMutexUnlock();
	return n;
}


/* Rewritten.   mBuf_freeMbufChain: Free the whole mbuf chain. Notice user may pass only a part of the mbuf chain to us */
uint32 mBuf_freeMbufChain(register struct rtl_mBuf * m){
	int32     i = 0;

	assert(m);
	assert(ISSET(m->m_flags, MBUF_USED));

	osglue_mbufMutexLock();
	i = _iFreeMbufChain(m, MBUF_FREEALL);
	osglue_mbufMutexUnlock();
	if (i > 0)
		return i;
	else
		return 0;

}


///////////////////////////////////////////////////////

/*
 * Mbuffer utility routines.
 */

/*
 * Copy data from a buffer back into the indicated mbuf chain,
 * starting "off" bytes in mbuf chain from the beginning, extending the mbuf
 * chain if necessary.
 */
int32 mBuf_copyToMbuf(struct rtl_mBuf * m0,  uint32 offset, register uint32 len,  int8 * cp)
{
	 int32 mlen;
	 struct rtl_mBuf *m = m0, *n;
	 uint32 off = offset;
	int32     totlen = 0;


	assert(m);
	if(mBuf_clusterIsWritable(m)==0)	//We are not the owner or the cluster is cloned by other mbuf.
		return -1;

	osglue_mbufMutexLock();
	assert(ISSET(m->m_flags, MBUF_USED));
	mlen = m->m_len;
	while (off > mlen){/*Find from which mbuf we start copy */
		off -= mlen;
		totlen += mlen;
		if (m->m_next == 0){
			if(totlen < offset) //We have reached the end f mbuf chain but  we don't have that much data in mbuf chain!!!
				return -1;
		}
		m = m->m_next;
	}
 	while (len > 0){
 		int32 maxLen;
		assert(ISSET(m->m_flags, MBUF_EXT));
		if (m->m_len)
			mlen= min(mBuf_trailingSpace(m)+m->m_len - off, len);
		else  //in this case, off= m_len=0
			mlen = min(m->m_extsize, len);

		//Enforce max mbuf chain size limit.
		maxLen = totlen + m->m_len + mlen;
		if(maxLen > 65535)
			mlen= 65535-totlen-m->m_len;

		memcpy( (void *) (off + MBUF2DATAPTR(m, int8 *)),(void *) cp,(uint32) mlen);
		cp += mlen;
		len -= mlen;
		mlen += off;
		off = 0;
		totlen += mlen;
		if(m->m_len < mlen+off)
			m->m_len = mlen+off;

		if (len == 0 || maxLen>65535)
			break;
		if (m->m_next == 0){
			n = _m_get(MBUF_DONTWAIT, m->m_type, 1);
			if (n == 0)
				break;
			n->m_len = min(m_clusterSize, len);
			m->m_next = n;
		}
		m = m->m_next;
	}

  	if (((m = m0)->m_flags & MBUF_PKTHDR)&& (m->m_pkthdr->ph_len < totlen))
		m->m_pkthdr->ph_len = totlen;
	osglue_mbufMutexUnlock();
	return totlen;
}


//Jump table used by Boyer-Moore pattern search algorithm
void mBuf_getBMjmpTable(uint8 *pat,  uint32 *jump_tbl,uint32 patLen){
	uint32 i;
	for(i=0; i<256; i++)
		jump_tbl[i]= patLen;
	for(i=0; i< patLen-1; i++)
		jump_tbl[(uint32) pat[i]]= patLen -i -1;
}

//Find in the first 'len' bytes of mbuf chain 'm' (mat cross cluster), find first occurance of
//pattern 'delimiter', which is 'delimitLen' bytes long.
//If pattern matched, return the position found(start from 0), otherwise return -1.
//Assumption: Assume there are 'len' bytes on chain. Don't check it.
//len: Lenght of 'mem'
//delimiter: pattern to be matched
//delimitLen: length of 'pat' and 'jmp_tbl'
//jmp_tbl: jump table buffer, should be 256 entries long
int32 mBuf_BMpatternMatch(struct rtl_mBuf *m, uint32 len, uint8 *delimiter, uint32 delimitLen, uint32 *jmp_tbl){
	int32 retval= -1;
	//Boyer-Moore string searching algorithm.

	assert(m && len && delimiter && delimitLen && jmp_tbl);
	osglue_mbufMutexLock();
	if(m->m_data + len <= m->m_extbuf + m->m_extsize){
		//Fast path, all 'len' bytes within a single cluster.
		int32 i,j,k;
		uint8 *mem= (uint8 *)m->m_data;
		for(i=delimitLen -1; i < len; ){  //compute downward
			for(j=delimitLen-1, k=i; j>=0 && (mem[k]==delimiter[j]); k--, j--); //no problem for ;
			if (j<0){	//delimiter exhausted?
				retval = k+1;	//yes, pattern found at (k+2)-th byte
				goto out;
			}else
				i+=jmp_tbl[(uint32)mem[i]];	//no, update using jmp_tbl
		}
	}else{

		//Slow path...may cross cluster
		struct rtl_mBuf *p;
		int32 i,j,k;
		uint32 offsetTbl[16], lenTbl[16],mbufs, num=0;

		//Remember last byte data offset of each cluster.
		for(mbufs=0, p=m; p; p=p->m_next, mbufs++){
			offsetTbl[mbufs]=num+p->m_len;
			lenTbl[mbufs]=p->m_len;
			num +=p->m_len;
		}

		for(i=delimitLen -1; i < len; ){  //compute downward
			for(j=delimitLen-1, k=i; j>=0 && _iGetClusterDataByte(m, k,offsetTbl,lenTbl,mbufs) ==delimiter[j]; k--, j--); //no problem for ;
			if (j<0){	//delimiter exhausted?
				retval = k+1;	//yes, pattern found at (k+1)-th byte
				goto out;
			}else
				i+=jmp_tbl[(uint32)_iGetClusterDataByte(m, i, offsetTbl,lenTbl,mbufs)];	//no, update using jmp_tbl
		}
	}
out:
	osglue_mbufMutexUnlock();
	return retval;
}


/* Not modified...Minor revision only.
 * Copy data from an mbuf chain starting "off" (Offset starts from 0)bytes from the beginning,
 * continuing for "len" bytes, into the indicated buffer.
 */
int32 mBuf_copyToUserBuffer(struct rtl_mBuf * m, uint32 off, uint32 len, int8 * cp)
{
	register uint32 count;
	register uint32 copied = 0;


	osglue_mbufMutexLock();
	while (off > 0)
	{
		assert(m != NULL);
		assert(ISSET(m->m_flags, MBUF_USED));
		if (off < m->m_len)
			break;
		off -= m->m_len;
		m = m->m_next;
	}
	while (len > 0)
	{
		assert(m != NULL);
		assert(ISSET(m->m_flags, MBUF_USED));
		count = min(m->m_len - off, len);
		memcpy( cp,MBUF2DATAPTR(m, int8 *) + off, count);
		len -= count;
		copied += count;
		cp += count;
		off = 0;
		m = m->m_next;
	}
	osglue_mbufMutexUnlock();
	return copied;
}


/* Rewritten
 * Make a clone of an mbuf chain starting "iOffset"(begin from 0) bytes from the beginning,
 * continuing for "iLength" bytes.  The wait parameter is a choice of MBUF_WAITOK/MBUF_DONTWAIT from caller.
 * If len is M_COPYALL, clone to end of mbuf chain. (In this case, you can use mBuf_clonePacket() instead)
 *
 * Note that the copy is read-only, because clusters are not copied,
 * only their reference counts are incremented.
 */
struct rtl_mBuf *mBuf_cloneMbufChain(struct rtl_mBuf *pMbufChain, int32 iOffset, int32 iLength,int32 flag){
	struct rtl_mBuf *m;
	osglue_mbufMutexLock();
	m =  _pCopyMbufChain(pMbufChain,  iOffset,  iLength, flag | MBUF_ONLY);
	osglue_mbufMutexUnlock();
	return m;
}

/* Rewritten
 * Duplicate an mbuf chain starting "iOffset"(begin from 0) bytes from the beginning,
 * continuing for "iLength" bytes.  The wait parameter is a choice of MBUF_WAITOK/MBUF_DONTWAIT from caller.
 * If len is M_COPYALL, duplicate to end of mbuf chain. (In this case, you can use mBuf_dupPacket() instead)
 *
 * Returned mubf chain is writable.
 */

struct rtl_mBuf *mBuf_dupMbufChain(struct rtl_mBuf *pMbufChain, int32 iOffset, int32 iLength,  int32 flag){
	struct rtl_mBuf *m;
	osglue_mbufMutexLock();
	m =  _pCopyMbufChain( pMbufChain,  iOffset,  iLength, flag);
	osglue_mbufMutexUnlock();
	return m;
}

/* Rewritten
 * "CLONE"  ENTIRE packet, including header (which must be present).
 * An optimization of the common case `mBuf_cloneMbufChain(m, 0, M_COPYALL, how)'.
 * Note that the copy is read-only, because clusters are not copied,
 * only their reference counts are incremented.
 */
struct rtl_mBuf *mBuf_clonePacket(struct rtl_mBuf *pMbuf, int32 iHow){
	struct rtl_mBuf *m;
	osglue_mbufMutexLock();
	m =  _pCopyPacket(pMbuf, iHow | MBUF_ONLY);
	osglue_mbufMutexUnlock();
	return m;
}


/* Rewritten
 * "DUPLICATE" entire packet into a completely new chain, including
 * copying any mbufs, pkthdrs, and clusters.  An optimization of the common case
 * mBuf_dupMbufChain(). Use this instead of mBuf_clonePacket() when
 * you need a writable copy of an mbuf chain.
 */
struct rtl_mBuf *mBuf_dupPacket(struct rtl_mBuf *pMbuf, int32 iHow){
	struct rtl_mBuf *m;
	osglue_mbufMutexLock();
	m =  _pCopyPacket(pMbuf, iHow);
	osglue_mbufMutexUnlock();
	return m;
}



/*
 * Concatenate mbuf chain pTailMbuf to pHeadMbuf. If pTailMbuf has a pkthdr, it would be freed.
 * Both chains must be of the same type (e.g. MT_DATA).
 */

struct rtl_mBuf *mBuf_cat(struct rtl_mBuf *pHeadMbuf, struct rtl_mBuf *pTailMbuf)
{
	struct rtl_mBuf *pHeadLastMbuf = pHeadMbuf;
	struct rtl_mBuf *pMbuf;
	uint32    iTailLength = 0, pktlen=0;

	assert(pTailMbuf && pHeadMbuf);
	assert(ISSET(pHeadMbuf->m_flags, MBUF_USED));
	assert(ISSET(pTailMbuf->m_flags, MBUF_USED));

	osglue_mbufMutexLock();
	if ((pTailMbuf->m_type != pHeadMbuf->m_type)){
		pHeadMbuf = NULL;
		goto out;
	}

	while (pHeadLastMbuf->m_next){
		pktlen+= pHeadLastMbuf->m_len;
		pHeadLastMbuf = pHeadLastMbuf->m_next;
	}
	pktlen+=pHeadLastMbuf->m_len;
	
	//Exceed max mbuf chain size.
	if((_m_getPktlen(pTailMbuf)+pktlen)>=65536){
		pHeadMbuf=NULL;
		goto out;
	}

	pHeadLastMbuf->m_next = pTailMbuf;

	//free pTailMbuf's pkthdr if there is one
	if ISSET(pTailMbuf->m_flags, MBUF_PKTHDR){
		assert(pTailMbuf->m_pkthdr && ISSET(pTailMbuf->m_pkthdr->ph_flags, PKTHDR_USED));
		iTailLength = pTailMbuf->m_pkthdr->ph_len;
		_vFreePkthdr(pTailMbuf->m_pkthdr);
		pTailMbuf->m_pkthdr=0;
		CLEARBITS(pTailMbuf->m_flags, MBUF_PKTHDR);
	}

	if ISSET(pHeadMbuf->m_flags, MBUF_PKTHDR){
		assert(pHeadMbuf->m_pkthdr
			   && ISSET(pHeadMbuf->m_pkthdr->ph_flags, PKTHDR_USED));
		if (iTailLength > 0)
			pHeadMbuf->m_pkthdr->ph_len += iTailLength;
		pMbuf = pTailMbuf;
		while (pMbuf)	{
			CLEARBITS(pMbuf->m_flags, MBUF_PKTHDR);
			pMbuf->m_pkthdr = pHeadMbuf->m_pkthdr;	//all trailing mbuf's m_pkthdr points to pHeadMbuf's pkthdr
			pMbuf = pMbuf->m_next;
		}
	}

out:
	osglue_mbufMutexUnlock();
	return pHeadMbuf;
}

struct rtl_mBuf *mBuf_adjHead(struct rtl_mBuf *mp, uint32 req_len){
	 uint32 len = req_len;
	 struct rtl_mBuf *m=mp;
	 uint32 adjusted=0, toomuch=0;


	assert(mp);
	assert(ISSET(m->m_flags, MBUF_USED));

	if(ISCLEARED(m->m_flags, MBUF_EXT))
		return NULL;

	osglue_mbufMutexLock();

	while (m != NULL && len > 0)	{
		assert(m && (m->m_flags & MBUF_EXT));
		if (m->m_len <= len){		//The whole mbuf should be adjusted or trimmed
			len -= m->m_len;
			adjusted+= m->m_len;
			m->m_len = 0;		   //clusters are not freed even it contains no data
			m->m_data = m->m_extbuf; //reset the m_data pointer
			m = m->m_next;
		}else{
			m->m_len -= len;
			adjusted+= len;

			if(m->m_len==0)
				m->m_data = m->m_extbuf;
			else
				m->m_data += len;
			len = 0;
			assert(m->m_len>0 );
			break;
		}
	}
	if(len>0)
		toomuch=1;
	if (mp->m_pkthdr)
		mp->m_pkthdr->ph_len -= adjusted;
	osglue_mbufMutexUnlock();
	if(toomuch)
		return mp;
	else
		return m;
}

struct rtl_mBuf *mBuf_adjTail(struct rtl_mBuf *mp, uint32 req_len){
	 uint32 len = req_len;
	 struct rtl_mBuf *m=mp, *n=NULL;
	 uint32 pktlen, drop;
	 uint32 adjusted=0;


	assert(mp);
	assert(req_len);
	assert(ISSET(m->m_flags, MBUF_USED));

	if(ISCLEARED(m->m_flags, MBUF_EXT)){
		return NULL;
	}

	osglue_mbufMutexLock();
	//calculate total pktlen
	pktlen = _m_getPktlen(mp);

	if(len>pktlen)
		len = pktlen;
	drop = pktlen - len;	//we will start dropping data from the #drop -th byte


	// 'pktlen' is the length of packet.
	// 'len' is the number of daya bytes to be dropped
	// 'drop' downcounts how far is the first byte to drop
	// 'm' is the first mbuf in packet,

	/*
	 * Find the mbuf with first byte of data to be trimmed, adjust its length,
	 * and toss data from remaining mbufs on chain.
	 */

	m = mp;		//start from first mbuf
	n = NULL;
	adjusted=0;
	for (; m; n=m, m = m->m_next)
	{
		assert(m && (m->m_flags & MBUF_EXT));
		if (m->m_len >= drop)			//the first mbuf to trim.
		{
			adjusted+= (m->m_len - drop);
			if (drop==0){
				assert(m->m_extbuf);
				m->m_data = m->m_extbuf;
			}
			m->m_len = drop;		//m->m_data + m->m_len is the last used byte
			break;
		}
		drop -= m->m_len;			//The first byte to trim is 'drop+1' byte away from here.
	}

	// 'm' is now the first mbuf which contains first data byte.
	//Now we clear all mbufs after 'm'  starting from 'n'

	n = m->m_next;
	if(!n && (m->m_len==0)){	//No more mbufs after m and m is empty. Return first mbuf to user
		m = mp;
		goto done;
	}

	while (n){
		adjusted+= n->m_len;
		n->m_len = 0;
		n->m_data = n->m_extbuf;
		n = n->m_next;
	}

done:
	if (mp->m_pkthdr)
		mp->m_pkthdr->ph_len -= adjusted;

	osglue_mbufMutexUnlock();
	return m;
}




/*
 * Partition an mbuf chain in two pieces, returning the tail --
 * all but the first len0 bytes.  In case of failure, it returns NULL and
 * attempts to restore the chain to its original state.
 *  The chain is NOT copied, but cloned.
 */
static struct rtl_mBuf *_m_split(register struct rtl_mBuf *m0, uint32 len0, int32 wait){

	register struct rtl_mBuf *m = NULL, *n = NULL;
	uint32  len = len0, remain = 0;


	assert(len);
	assert(m0);
	assert(ISSET(m0->m_flags, MBUF_USED));

	osglue_mbufMutexLock();
	for (m = m0; m && len > m->m_len; m = m->m_next){  //find from which mbuf to split
		assert(ISSET(m->m_flags, MBUF_EXT));
		len -= m->m_len;
	}
	if (m == 0)	   //len0 is greater than the number of all data we have, can't split....
		goto out;

	assert(m->m_len >= len);
	remain = m->m_len - len;		   // "remain" bytes in this mbuf are to be copied to new mbuf


	if (ISSET(m0->m_flags, MBUF_PKTHDR))	{
		if (remain == 0)	{							   //we split exactly at the cluster boundary
			if (!m->m_next)
				goto out;//can't split at the last byte
			n = m->m_next;
			n->m_pkthdr = NULL;
		}else if ((n = _m_get(wait | MBUF_ONLY, m0->m_type, 1)) == NULL)	//the split point is not at the boundary of cluster
			goto out;

		if (!_m_gethdr(n, wait)){
			_iFreeMbufChain(n, MBUF_FREEALL);
			n=NULL;
			goto out;
		}


		//Copy pkthdr content. skip  first 4-bytes to avoid overwriting ph_mbuf pointer */
		memcpy((void *) (((int32 *) n->m_pkthdr) + 1), (void *) (((int32 *) m0->m_pkthdr) + 1),sizeof(struct rtl_pktHdr) - 4);

		n->m_pkthdr->ph_len = m0->m_pkthdr->ph_len - len0;	//adjust mbuf chain length.
		m0->m_pkthdr->ph_len = len0;
		assert(ISSET(m->m_flags, MBUF_EXT));
		goto extpacket;

	}else if (remain == 0){	 
		n = m->m_next;
		m->m_next = 0;				   //terminate the m0 mbuf chain
		goto out;
	}else{	   //no pkthdr, get a new mbuf to store trailing data
		m->m_len -= remain;
		if ((n = _m_get(wait | MBUF_ONLY, m0->m_type, 1)) == NULL)
			goto out;
	}

  extpacket:
	SETBITS(n->m_flags, MBUF_EXT);
	if (remain != 0)	{
		n->m_extbuf = m->m_extbuf;
		n->m_extsize = m->m_extsize;
		if (!m->m_extref)
			CLDataAddr2TagAddr(m->m_extbuf)->cltag_refcnt++;
		else
			(*(m->m_extref)) (m->m_extbuf, m->m_extsize);
		//m->m_extsize = 0; /* For Accounting XXXXXX danger cfliu:Why this line?????*/
		n->m_data = m->m_data + len;
		n->m_len = remain;
		n->m_next = m->m_next;
		m->m_len = len;
	}
	m->m_next = 0;
out:
	osglue_mbufMutexUnlock();
	return n;
}

struct rtl_mBuf *mBuf_split(register struct rtl_mBuf *m0, uint32 len0, int32 wait){
	struct rtl_mBuf *m;
	osglue_mbufMutexLock();
	m =  _m_split( m0, len0, wait);
	osglue_mbufMutexUnlock();
	return m;
}


//although using mBuf_split has some overhead and side effect (one extra mbuf and pkthdr may be allocated
//and freed immediately), and is less efficient, however, we don't need extra code here.
extern struct rtl_mBuf *mBuf_trimHead(struct rtl_mBuf *mp, uint32 req_len){
	struct rtl_mBuf *n=NULL;

	assert(mp);
	assert(req_len);
	assert(ISSET(mp->m_flags, MBUF_USED));
	osglue_mbufMutexLock();
	if(_m_getPktlen(mp)<= req_len){
		_iFreeMbufChain(mp,MBUF_FREEALL);
	}else{
		n = _m_split(mp, req_len, MBUF_DONTWAIT);
		if(_iFreeMbufChain(mp,MBUF_FREEALL)<1)
			;//DEBUGMSG(( "mBuf_trimHead failed"));
	}
	osglue_mbufMutexUnlock();
	return n;

}

//although using mBuf_split has some overhead and side effect (one extra mbuf and pkthdr may be allocated
//and freed immediately), and is less efficient, however, we don't need extra code here.
struct rtl_mBuf *mBuf_trimTail(struct rtl_mBuf *mp, uint32 req_len){
	struct rtl_mBuf *n;
	int32 len;

	assert(mp);
	assert(req_len);
	assert(ISSET(mp->m_flags, MBUF_USED));

	osglue_mbufMutexLock();
	len = _m_getPktlen(mp);

	if(len==0){
		mp=NULL;
		goto out;
	}else
		len -= req_len;

	if(len <= 0){ //trim too much
		_iFreeMbufChain(mp,MBUF_FREEALL);
		mp=NULL;
		goto out;

	}

	n = _m_split(mp, len, MBUF_DONTWAIT);
	assert(n);
	if(_iFreeMbufChain(n, MBUF_FREEALL)<1){
		;//DEBUGMSG(( "mBuf_trimTail failed2"));
		mp=NULL;
	}
	//find the last mbuf which contains last data byte.
out:
	osglue_mbufMutexUnlock();
	return mp;
}


/*
 *	cfliu: This function is important in traditional BSD networking stack because
 *		 original mbuf can't do dtom() if data is saved in cluster. This is not the
 *		case here since we offer a back pointer from cluster to mbuf via cltag_mbuf field.
 *		However, this function is still useful if we want to collect continous databytes from later clusters
 *		to the specified mbuf's cluster.
 *
 * 		We won't pullup more data bytes than a cluster can hold (ie. iLen <=m_clusterSize)
 */
int32 mBuf_pullup(register struct rtl_mBuf * pMbuf, int32 iLen)
{
	int32     iBytesLeft = 0, iRestSpace = 0, iCount = 0;
	uint8     *pChar = NULL;

	struct rtl_mBuf *pThisMbuf = NULL;

	assert(pMbuf && (iLen > 0));
	assert(ISSET(pMbuf->m_flags, MBUF_USED));

	osglue_mbufMutexLock();

	if (ISCLEARED(pMbuf->m_flags, MBUF_EXT) || (!pMbuf->m_data))	//may be just a mbuf without clusters
		goto quit;

	if ((iBytesLeft = iLen) <= pMbuf->m_len)	// no need to pull up.
		goto quit;

	if ((pMbuf->m_pkthdr) && (pMbuf->m_pkthdr->ph_len < iBytesLeft))	//not enough data to be pulled up
		goto quit;

	//mbuf chain has enough data for us to pull up.

	iRestSpace = mBuf_trailingSpace(pMbuf);

	iBytesLeft -= pMbuf->m_len;

	if (iRestSpace >= iBytesLeft)
	{								   // Cluster is large enough
		pThisMbuf = pMbuf->m_next;	   //first mbuf to check
		assert(pThisMbuf != NULL);
		pChar = pMbuf->m_data + pMbuf->m_len;	//where first byte of data should be copied to
		while (iBytesLeft > 0)
		{
			while (pThisMbuf->m_len == 0)
				pThisMbuf = pThisMbuf->m_next;
			assert(pThisMbuf != NULL);
			iCount = min(pThisMbuf->m_len, iBytesLeft);
			if (!memcpy( pChar, pThisMbuf->m_data,iCount))
				goto quit;

			pThisMbuf->m_len -= iCount;	//length shrinks
			pThisMbuf->m_data += iCount;	//starting address goes down

			pMbuf->m_len += iCount;
			pChar += iCount;

			iBytesLeft -= iCount;
		}
	}
	else {
	////
	/* hiwu : pullup based on leading space*/
		uint8 *cp1, *cp2;
		int32 ii = 0;
		iRestSpace = mBuf_leadingSpace(pMbuf);
		if (iRestSpace >= iBytesLeft){ // Cluster is large enough
		pThisMbuf = pMbuf->m_next;	   //first mbuf to check
		assert(pThisMbuf != NULL);

		cp1 = pMbuf->m_data;
		pMbuf->m_data -= iBytesLeft;
		cp2 = pMbuf->m_data;
		for(ii = 0 ; ii < pMbuf->m_len ; ii++){
			*cp2 = *cp1;
			 cp2++;
			 cp1++;
		}

		pChar = pMbuf->m_data + pMbuf->m_len;	//where first byte of data should be copied to


		while (iBytesLeft > 0)
		{
			while (pThisMbuf->m_len == 0)
				pThisMbuf = pThisMbuf->m_next;
			assert(pThisMbuf != NULL);
			iCount = min(pThisMbuf->m_len, iBytesLeft);
			if (!memcpy( pChar, pThisMbuf->m_data,iCount))
				goto quit;

			pThisMbuf->m_len -= iCount;	//length shrinks
			pThisMbuf->m_data += iCount;	//starting address goes down

			pMbuf->m_len += iCount;
			pChar += iCount;

			iBytesLeft -= iCount;
		}
		}

	}

  quit:
	osglue_mbufMutexUnlock();
	return pMbuf->m_len;
}

//////// mbuf APIs dedicated for drivers //////

//user want allocate pkthdrs without mbufs.
//mBuf_driverGetPkthdr returns success only when all requested pkthdr could be get.
//User should be responsible for managing the pointers linking pkthdr and mbufs. Otherwise,
//orphan pkthdrs may never be freed.
//Assumption: Caller should be responsible that pHeadPkthdr and pTailPkthdr are always non-NULL!!
uint32 mBuf_driverGetPkthdr(uint32 Npkthdr, struct rtl_pktHdr **ppFirstPkthdr, struct rtl_pktHdr **ppTailPkthdr){
	//see if we can get all mbufs
	register int32 iIndex;
	register struct rtl_pktHdr *pThisPkthdr=NULL,*pPrevPkthdr=NULL;
	osglue_mbufMutexLock();

	Npkthdr = min(Npkthdr, _PHStatus.iFreebufs);

	if (Npkthdr==0){
		osglue_mbufMutexUnlock();
		return 0;
	}

	/*Yes, we have enough free pkthdrs... */
	for (iIndex = 0; iIndex < Npkthdr; iIndex++){	// Initialize all Npkthdr pkthdrs
		assert(_PHStatus.pvFreelist);
		pThisPkthdr = _PHStatus.pvFreelist;
		_PHStatus.iFreebufs--;
		//Assumption: Pointers are 32-bits,
		_PHStatus.pvFreelist = (uint32 *) (*((uint32 *) pThisPkthdr));

		//assert(pThisPkthdr);
		//memset((void *) pThisPkthdr, (uint8) 0, sizeof(struct rtl_pktHdr));	//Do we really need to Initialize pkthdr to 0 at driver???
		pThisPkthdr->ph_flags= PKTHDR_USED|PKTHDR_DRIVERHOLD;
		pThisPkthdr->ph_nextHdr=NULL;
		pThisPkthdr->ph_len=0;

		if (iIndex != 0)
			pPrevPkthdr->ph_nextHdr = pThisPkthdr;	/*Chain together */
		else
			*ppFirstPkthdr = pThisPkthdr;	/* Return this pointer to user */
		pPrevPkthdr = pThisPkthdr;
	}
	assert(iIndex==Npkthdr);
	osglue_mbufMutexUnlock();
	*ppTailPkthdr = pThisPkthdr;
	return (iIndex);		   //Return success only when all requested buffers allocated
}



/*Free a pkthdr alone. Callers should be aware that this is NOT the normal way to free a pkthdr with
mbufs attached. Callers should use  mBuf_freeOne()or  mBuf_freeMbufChain() to free pkthdrs attached with mbuf chains.
Caller of this function should remove the links between mbufs and pkthdrs on their own before
mBuf_freePkthdr() is called.*/
uint32 mBuf_driverFreePkthdr(struct rtl_pktHdr *ph, uint32 Npkthdr, struct rtl_pktHdr **ppHeadPkthdr)
{
	register struct rtl_pktHdr *pThisPkthdr = ph, *pNextPkthdr = NULL;
	register uint32     iFreed = 0;
	osglue_mbufMutexLock();
	while (iFreed<Npkthdr && pThisPkthdr && ISSET(pThisPkthdr->ph_flags, PKTHDR_USED))
	{
		//assert(ph->ph_flags & PKTHDR_USED);
		CLEARBITS(ph->ph_flags, PKTHDR_USED);
		//Assumption: 1. Pointers are 32-bits,   2.1st field of pBufPoolStat points to next free Buffer as defined in mbuf.h
		*((uint32 *) pThisPkthdr) = (uint32) _PHStatus.pvFreelist;	/* i.e.   p->next = Freelist */
		_PHStatus.pvFreelist = pThisPkthdr;	/*i.e.  Freelist = p */
		_PHStatus.iFreebufs++;
		pNextPkthdr = pThisPkthdr->ph_nextHdr;
		iFreed++;
		pThisPkthdr = pNextPkthdr;
	}
	if(ppHeadPkthdr)
		*ppHeadPkthdr = pNextPkthdr;
	osglue_mbufMutexUnlock();
	return iFreed;
}



//user want get 'Nmbuf' pair of mbuf-cluster.
//mbuf and cluster not initialized to zero.
//Assumption: Caller should be responsible that pFirstMbuf and pTailMbuf are always non-NULL!!
uint32 mBuf_driverGet(uint32 Nmbuf,struct rtl_mBuf **ppFirstMbuf, struct rtl_mBuf **ppTailMbuf)
{
	//see if we can get all mbufs
	register uint32 iIndex;
	register struct rtl_mBuf *pThisMbuf=NULL,*pPrevMbuf=NULL;
	register struct mcluster_tag *pThisClusterTag=NULL;
	osglue_mbufMutexLock();
	Nmbuf = min(Nmbuf, _MbufStatus.iFreebufs);
	Nmbuf = min(Nmbuf, _ClStatus.iFreebufs);
	if (Nmbuf==0){
		osglue_mbufMutexUnlock();
		return 0;
	}

	/*Yes, we have enough free mbuf and cluster pair... */
	for (iIndex = 0; iIndex < Nmbuf; iIndex++){	// Initialize all Npkthdr pkthdrs
		//Assumption: Pointers are 32-bits,

		/*Get an mbuf*/
		assert(_MbufStatus.pvFreelist);
		pThisMbuf = _MbufStatus.pvFreelist;
		_MbufStatus.iFreebufs--;
		_MbufStatus.pvFreelist = (uint32 *) (*((uint32 *) pThisMbuf));

		/*Get a cluster*/
		assert(_ClStatus.pvFreelist);
		pThisClusterTag = _ClStatus.pvFreelist;
		_ClStatus.iFreebufs--;
		_ClStatus.pvFreelist = (uint32 *) (*((uint32 *) pThisClusterTag));
		pThisMbuf->m_next = NULL;
		pThisMbuf->m_pkthdr= 0;
		pThisMbuf->m_len = 0;
		pThisMbuf->m_flags =  MBUF_EXT | MBUF_USED;
		pThisMbuf->m_type = MBUFTYPE_DATA;
		_iMbufTypes[MBUFTYPE_DATA]++;
		pThisMbuf->m_extbuf = pThisMbuf->m_data =
			(uint8 *) CLTagAddr2DataAddr(pThisClusterTag);
		pThisMbuf->m_extsize = _Statistics.m_mclbytes;

		pThisClusterTag->cltag_mbuf = pThisMbuf;	/* points back to pThisMbuf */
		pThisClusterTag->cltag_flags =  CLUSTER_USED;
		pThisClusterTag->cltag_refcnt = 1;

		if (iIndex != 0)
			pPrevMbuf->m_next = pThisMbuf;	/*Chain together */
		else
			*ppFirstMbuf = pThisMbuf;	/* Return this pointer to user */
		pPrevMbuf = pThisMbuf;
	}
	assert(iIndex==Nmbuf);
	osglue_mbufMutexUnlock();
	*ppTailMbuf = pThisMbuf;
	return (iIndex);		   //Return success only when all requested buffers allocated
}


/*Free the whole mbuf chain*/
//user want free 'Nmbuf' pairs of mbuf-clusters started from 'pFirstMbuf',
//Returns the number of pairs of mbuf-cluster freed and if we haven't free the whole chain, return
//Next usable mbuf via 'pHeadMbuf'
uint32 mBuf_driverFreeMbufChain(struct rtl_mBuf *pFirstMbuf)
{
	register struct rtl_mBuf *pThisMbuf = pFirstMbuf, *pNextMbuf = NULL;
	register struct mcluster_tag *pThisClusterTag = NULL;
	register uint32     iFreed = 0;
	osglue_mbufMutexLock();
	while (pThisMbuf && ISSET(pThisMbuf->m_flags, MBUF_USED))
	{
		//Free the cluster */
		if (pThisMbuf->m_flags & MBUF_EXT)
		{
			pThisClusterTag = CLDataAddr2TagAddr(pThisMbuf->m_data);

			pThisClusterTag->cltag_refcnt--;
			if (pThisClusterTag->cltag_refcnt == 0)
			{					   // The last referee should  free the cluster
				CLEARBITS(pThisClusterTag->cltag_flags, CLUSTER_USED);

				//Assumption: 1. Pointers are 32-bits,   2.1st field of pBufPoolStat points to next free Buffer as defined in mbuf.h
				*((uint32 *) pThisClusterTag) = (uint32) _ClStatus.pvFreelist;	/* i.e.   p->next = Freelist */
				_ClStatus.pvFreelist = pThisClusterTag;	/*i.e.  Freelist = p */
				_ClStatus.iFreebufs++;
			}
			else if (pThisClusterTag->cltag_mbuf == pThisMbuf)
			{
				//The owner is quiting and cluster becomes zombie.
				//TODO: Checks what may happen if zombie cluster happens.....
				// 1. m_dtom() won't work for zombie clusters
				pThisClusterTag->cltag_mbuf = NULL;
			}
			// All other referees just silently quit.
		}

		if (pThisMbuf->m_pkthdr)
		{
			if (pThisMbuf->m_flags & MBUF_PKTHDR)
			{
				CLEARBITS(pThisMbuf->m_pkthdr->ph_flags, PKTHDR_USED);
				*((uint32 *) pThisMbuf->m_pkthdr) = (uint32) _PHStatus.pvFreelist;	/* i.e.   p->next = Freelist */
				_PHStatus.pvFreelist = pThisMbuf->m_pkthdr;	/*i.e.  Freelist = p */
				_PHStatus.iFreebufs++;
				pThisMbuf->m_pkthdr = NULL;
				CLEARBITS(pThisMbuf->m_flags, MBUF_PKTHDR);
			}
			else					   /* user wanna free all buffers, so don't worry about pkt length... */
				pThisMbuf->m_pkthdr = NULL;
		}

		/*
		   Free the mbuf itself
		 */
		_iMbufTypes[(int32) MBUFTYPE_DATA]--;
		pThisMbuf->m_flags = MBUF_FREE;
		pNextMbuf = pThisMbuf->m_next;
		*((uint32 *) pThisMbuf) = (uint32) _MbufStatus.pvFreelist;	/* i.e.   p->next = Freelist */
		_MbufStatus.pvFreelist = pThisMbuf;	/*i.e.  Freelist = p */
		_MbufStatus.iFreebufs++;
		iFreed++;
		pThisMbuf = pNextMbuf;
	}
	osglue_mbufMutexUnlock();
	return iFreed;
}


/*********************************************************************************
	Section 6:	Internal function implementation
**********************************************************************************/


/*
 * 	Function:	_vWakeup
 *	Input:		piWaitID		waiting channel to wakeup
 *	Returns:	None
 *	Note:
 * 		Wake up the next instance (if any)  which is waiting in the specified wait channel.
 * 		This should be called with mbuf mutex.
 *
 * 		m_{get | gethdr } routines would call is in order to wake another sleep
 * 		instance faster.
 */

static void _vWakeup(uint32 * piWaitID)
{
	//Assume mbuf mutex already accquired.
	assert(piWaitID);

#ifdef HAVE_SYNCH_MODULE
	if (piWaitID){
		(*piWaitID)--;
		synch_wakeup(piWaitID);
	}
#endif
	return;
}

RTL_STATIC_INLINE  void _vFreePkthdr(struct rtl_pktHdr *ph){
		assert(ph->ph_flags & PKTHDR_USED);
		CLEARBITS(ph->ph_flags, PKTHDR_USED);
		_vReturnFreeBuffer(ph, &_PHStatus);	/* free the pkthdr */
}

//In mbuf chain 'm', get the 'offset'-th data byte.
//'offsetTbl' stores the accumulated offset of each cluster in chain. 'mbufs' is the total number of mbufs in chain
//For example, m0-m3 are chained together, m0-m3 has 100,200,300,400 databyte bytes, so 'mbufs'=4
//and offsetTbl={100,300,600,1000}
RTL_STATIC_INLINE uint8 _iGetClusterDataByte(struct rtl_mBuf *m, uint32 offset, uint32 *offsetTbl, uint32 *lenTbl, uint32 mbufs){
	uint32 i, len=offset;
	struct rtl_mBuf *p;
	for(i=0, p=m; i<mbufs && offset>offsetTbl[i]; len-=lenTbl[i], p=p->m_next, i++);  //no problem for ;
	return (uint8)p->m_data[len];
}

/*
 * 	Function:	_iTryGet
 *	Input:		piAvailableBuffers		How many resource instances still available
 *				iRequestedBuffers	How many resource instances requested
 *				piWaitID		The wait channel
 *				iHow 		If resources aren't available, is it ok to wait?
 *				have_Waited	When returns, tell caller whether we have waited via tsleep
 *	Returns: 		Whether requested resources are avilable
 *	Note:
 * 			Should be called with mbuf mutex
 */

static int32 _iTryGet(register uint32 * piAvailableBuffers, register uint32 iRequestedBuffers,
					register uint32 * piWaitID, register int32 iHow, register uint32 * piHaveWaited)
{
	//Assume mbuf mutex already accquired.
	int32     iWaited = FALSE;

	assert(piAvailableBuffers && (iRequestedBuffers > 0) && (piWaitID)
		   && (iHow) && (piHaveWaited));

#ifndef HAVE_SYNCH_MODULE
	//if synch module not available, ignore iHow flag.
	if(*piAvailableBuffers < iRequestedBuffers){
		*piHaveWaited = iWaited;
		return FALSE;
	}
#else
	while (*piAvailableBuffers < iRequestedBuffers)	{

		//no enough buffers now and user doesn't want to wait, return fail
		if (ISSET(iHow , MBUF_DONTWAIT)){
			*piHaveWaited = iWaited;
			return FALSE;   // we don't have enough buffer and user doesn't want to wait
		}

		//User can wait. Wait until we are satisfied.
		(*piWaitID)++;
		osglue_mbufMutexUnlock();
		//wait forever. spinlock must be freed, after tsleep, spinlock must be reaccquired.
		synch_sleep((int8 * *)piWaitID,  0 ,0);
		(*piWaitID)--;
		iWaited = TRUE;
		osglue_mbufMutexLock();
	}
#endif

	*piHaveWaited = iWaited;
	return TRUE;
}



/*
 * 	Function:	_vInitBufferPools
 *	Input:		p		pointer to starting address of pool
 *				size		size of each element in bytes
 *				num		Number of elements in this pool
 *	Returns: 		NULL
 *	Description:
 *		Clear memory content to 0 and chain all elements together. We assume that the first field of each pool
 *		 is a 32-bit pointer to point to next element in the pool.
 */

static void _vInitBufferPools(void *pBufPool, int32 iSize, int32 iNum)
{
	//Assume mbuf mutex already accquired.
	int32     i;
	int8     *pBuf;

	assert(pBufPool);
	assert(iSize && iNum);
	pBuf = (int8 *) pBufPool;

	memset((void *) pBuf, (uint8) 0, iSize * iNum);	//Initialize to 0

	//Assumption: Pointers are 32-bits,
	for (i = 0; i < (iNum - 1); i++, pBuf += iSize)
		*((uint32 *) pBuf) = (uint32) (pBuf + iSize);	//chain neighboring bufers together via next pointer aligned to 1st field
}


/*
 * 	Function:	_pvGetFreeBuffer
 *	Input:
 *				pool		from which pool
 *	Returns: 		p = the first free buffer address
 *	Description:
 *		Get the first free buffer from specified pool and adjust freebuf count variable.
 *		Should be invoked with mbuf mutex hold
 */

RTL_STATIC_INLINE void *_pvGetFreeBuffer(struct poolstat *pBufPoolStat)
{
	//Assume mbuf mutex already accquired.
	void     *pvFreeBuffer;
	assert(pBufPoolStat);

#if 0
#ifndef NDEBUG
	if (pBufPoolStat->pvAddr == _MbufPool)
		assert(((uint32) pBufPoolStat->pvFreelist >=
				(uint32) pBufPoolStat->pvAddr)
			   && ((uint32) pBufPoolStat->pvFreelist <
				   (((uint32) pBufPoolStat->pvAddr) +
					sizeof(struct rtl_mBuf) * _iTotalMbufs)));
	if (pBufPoolStat->pvAddr == _ClusterTag)
		assert(((uint32) pBufPoolStat->pvFreelist >=
				(uint32) pBufPoolStat->pvAddr)
			   && ((uint32) pBufPoolStat->pvFreelist <
				   (((uint32) pBufPoolStat->pvAddr) +
					sizeof(struct mcluster_tag) * _iTotalClusters)));
	if (pBufPoolStat->pvAddr == _PkthdrPool)
		assert(((uint32) pBufPoolStat->pvFreelist >=
				(uint32) pBufPoolStat->pvAddr)
			   && ((uint32) pBufPoolStat->pvFreelist <
				   (((uint32) pBufPoolStat->pvAddr) +
					sizeof(struct rtl_pktHdr) * _iTotalPkthdrs)));

#endif
#endif
	pvFreeBuffer = pBufPoolStat->pvFreelist;
	pBufPoolStat->iFreebufs--;

	//Assumption: Pointers are 32-bits,
	pBufPoolStat->pvFreelist = (uint32 *) (*((uint32 *) pvFreeBuffer));
	return pvFreeBuffer;
};

/*
 * 	Function:	_vReturnFreeBuffer
 *	Input:		pvFreeBuffer		pointer to freed buffer
 *				pBufPoolStat		to which pool
 *	Returns: 		None
 *	Description:
 *		Return buffer p to pool
 */

RTL_STATIC_INLINE void _vReturnFreeBuffer(register void *pvFreeBuffer,
										  register struct poolstat *pBufPoolStat)
{
	//Assume mbuf mutex already accquired.
	assert(pvFreeBuffer);
	assert(pBufPoolStat);
#if 0	
#ifndef NDEBUG
	if (pBufPoolStat->pvAddr == _MbufPool)
		assert(((uint32) pvFreeBuffer >= (uint32) pBufPoolStat->pvAddr) &&
			   ((uint32) pvFreeBuffer <
				(((uint32) pBufPoolStat->pvAddr) +
				 sizeof(struct rtl_mBuf) * _iTotalMbufs)));
	if (pBufPoolStat->pvAddr == _ClusterTag)
		assert(((uint32) pvFreeBuffer >= (uint32) pBufPoolStat->pvAddr) &&
			   ((uint32) pvFreeBuffer <
				(((uint32) pBufPoolStat->pvAddr) +
				 sizeof(struct mcluster_tag) * _iTotalClusters)));
	if (pBufPoolStat->pvAddr == _PkthdrPool)
		assert(((uint32) pvFreeBuffer >= (uint32) pBufPoolStat->pvAddr) &&
			   ((uint32) pvFreeBuffer <
				(((uint32) pBufPoolStat->pvAddr) +
				 sizeof(struct rtl_pktHdr) * _iTotalPkthdrs)));

#endif
#endif
	//Assumption: 1. Pointers are 32-bits,   2.1st field of pBufPoolStat points to next free Buffer as defined in mbuf.h
	*((uint32 *) pvFreeBuffer) = (uint32) pBufPoolStat->pvFreelist;	/* i.e.   p->next = Freelist */
	pBufPoolStat->pvFreelist = pvFreeBuffer;	/*i.e.  Freelist = p */
	pBufPoolStat->iFreebufs++;
}


/*Free iNum mbufs from mbuf chain m*/
uint32 _iFreeMbufChain(struct rtl_mBuf *pFirstMbuf, uint32 iFreeAll)
{
	register struct rtl_mBuf *pThisMbuf = pFirstMbuf, *pNextMbuf = NULL;
	register struct mcluster_tag *pThisClusterTag = NULL;
	register uint32     iFreed = 0;
	register uint32     iFreedPkthdr = 0, iFreedCLusters = 0;

	assert(pFirstMbuf && ISCLEARED(pFirstMbuf->m_flags, MBUF_FREE));

	
	while (pThisMbuf && ISSET(pThisMbuf->m_flags, MBUF_USED))
	{

		//Free the cluster */
		if (pThisMbuf->m_flags & MBUF_EXT){
			if (pThisMbuf->m_extfree != NULL){						   /* None of my business. Call registered free function */
				(*pThisMbuf->m_extfree) (pThisMbuf->m_extbuf,
										 pThisMbuf->m_extsize);
			}else{						   /* I am responsible to free this cluster if required */
				pThisClusterTag = CLDataAddr2TagAddr(pThisMbuf->m_data);
				assert(pThisClusterTag);
				assert(pThisClusterTag->cltag_flags & CLUSTER_USED);

				pThisClusterTag->cltag_refcnt--;
				if (pThisClusterTag->cltag_refcnt == 0){   // The last referee should  free the cluster
					CLEARBITS(pThisClusterTag->cltag_flags, CLUSTER_USED);
					_vReturnFreeBuffer(pThisClusterTag, &_ClStatus);
					iFreedCLusters++;
				}else if (pThisClusterTag->cltag_mbuf == pThisMbuf){
					//The owner is quiting and cluster becomes zombie.
					//TODO: Checks what may happen if zombie cluster happens.....
					// 1. m_dtom() won't work for zombie clusters
					pThisClusterTag->cltag_mbuf = NULL;
				}
				// All other referees just silently quit.
			}
		}


		/*
		   Free the pkthdr if exists
		 */
		if (pThisMbuf->m_pkthdr){
			if (pThisMbuf->m_flags & MBUF_PKTHDR){
				assert(pThisMbuf->m_pkthdr->ph_mbuf == pThisMbuf);
				/*
				   We are freeing the head of mbuf chain, so we need to either
				   adjust the pointer or free the pkthdr
				 */

				if ((pThisMbuf->m_next) && !iFreeAll)	{
					pThisMbuf->m_pkthdr->ph_mbuf = pThisMbuf->m_next;
					assert(pThisMbuf->m_len);
					pThisMbuf->m_pkthdr->ph_len -= pThisMbuf->m_len;	/*decrease packet length */
					SETBITS(pThisMbuf->m_next->m_flags, MBUF_PKTHDR);	/* next mbuf is the head now */
				}else{
					assert(pThisMbuf->m_pkthdr->ph_flags & PKTHDR_USED);
					CLEARBITS(pThisMbuf->m_pkthdr->ph_flags, PKTHDR_USED);
					_vReturnFreeBuffer(pThisMbuf->m_pkthdr, &_PHStatus);	/* free the pkthdr */
					iFreedPkthdr++;
				}
				pThisMbuf->m_pkthdr = NULL;
				CLEARBITS(pThisMbuf->m_flags, MBUF_PKTHDR);
			}else if (!iFreeAll)		   //Free this mbuf only
				pThisMbuf->m_pkthdr->ph_len -= pThisMbuf->m_len;	/*decrease packet length */
			else					   /* user wanna free all buffers, so don't worry about pkt length... */
				pThisMbuf->m_pkthdr = NULL;
		}

		/*
		   Free the mbuf itself
		 */
		_iMbufTypes[(int32) pThisMbuf->m_type]--;
		pThisMbuf->m_flags = MBUF_FREE;
		pNextMbuf = pThisMbuf->m_next;

		_vReturnFreeBuffer(pThisMbuf, &_MbufStatus);	/* free the mbuf itself */

		iFreed++;

		if (iFreeAll)
			pThisMbuf = pNextMbuf;
		else
			break;
	}

	if (iFreedCLusters&&_ClStatus.iWaiting)
		_vWakeup(&_ClStatus.iWaiting);
	if (iFreedPkthdr&&_PHStatus.iWaiting)
		_vWakeup(&_PHStatus.iWaiting);
	if(_MbufStatus.iWaiting)
		_vWakeup(&_MbufStatus.iWaiting);

	pFirstMbuf = pNextMbuf;

	return iFreed;
}


/* Duplicate or Clone a packet.
 *  Depends on the MBUF_ONLY bit. If it is set, we clone. otherwise, we duplicate
 */
static struct rtl_mBuf *_pCopyPacket(struct rtl_mBuf *pMbuf, uint32 iHow)
{
	struct rtl_mBuf *pFirstMbuf = NULL, *pThisMbuf = NULL, *pNewMbuf =
		NULL, *pPrevNewMbuf = NULL;
	struct rtl_pktHdr *pNewPkthdr = NULL;
	uint32    iCopied = 0;

	assert(pMbuf);
	assert(iHow);
	pThisMbuf = pMbuf;

	while (pThisMbuf)
	{
		assert(ISSET(pThisMbuf->m_flags, MBUF_USED));
		pNewMbuf = _m_get(iHow, pThisMbuf->m_type, 1);	// get one buffer pNewMbuf

		if (!pNewMbuf)
			goto nospace;

		if (pPrevNewMbuf)
			pPrevNewMbuf->m_next = pNewMbuf;	//chain new mbuf onto the chain

		pPrevNewMbuf = pNewMbuf;	   //save the pointer so we can chain it later

		if (!pFirstMbuf && ISSET(pThisMbuf->m_flags, MBUF_PKTHDR))
		{							   //for the first mbuf)
			pFirstMbuf = pNewMbuf;
			if (!_m_gethdr(pFirstMbuf, iHow))	//get a pkthdr
				goto nospace;
			assert(pFirstMbuf->m_pkthdr);
			pNewPkthdr = pFirstMbuf->m_pkthdr;
			assert(pNewPkthdr->ph_mbuf == pFirstMbuf);

			//Copy pkthdr content. skip  first 4-bytes to avoid overwriting ph_mbuf pointer */
			memcpy
				((void *) (((int32 *) pNewPkthdr) + 1),
				  (void *) (((int32 *) pThisMbuf->m_pkthdr) + 1),
				 sizeof(struct rtl_pktHdr) - 4);

			assert(pFirstMbuf->m_pkthdr);
			pNewPkthdr = pFirstMbuf->m_pkthdr;
			assert(pNewPkthdr->ph_mbuf == pFirstMbuf);
			assert(pThisMbuf->m_pkthdr->ph_mbuf == pMbuf);
		}

		/*
		   Copy the mbuf content
		 */
		if ISSET
			(iHow, MBUF_ONLY)
		{							   /* Clusters would be cloned */
			memcpy
				((void *) (((int32 *) pNewMbuf) + 1),
				  (void *) (((int32 *) pThisMbuf) + 1),
				  sizeof(struct rtl_mBuf) - 4);
		}
		else
		{							   /* Have its own cluster */
			//be careful don't destroy m_extbuf, m_data,etc.
			pNewMbuf->m_len = pThisMbuf->m_len;
			pNewMbuf->m_flags = pThisMbuf->m_flags;
			pNewMbuf->m_type = pThisMbuf->m_type;
		}

		// Restore new pkthdr pointer
		if (pNewPkthdr)
			pNewMbuf->m_pkthdr = pNewPkthdr;

#ifndef NDEBUG
		if (pThisMbuf->m_pkthdr)
			assert(pThisMbuf->m_pkthdr->ph_mbuf == pMbuf);
		if (pNewMbuf->m_pkthdr)
			assert(pNewMbuf->m_pkthdr->ph_mbuf == pFirstMbuf);
#endif

		assert(ISSET(pThisMbuf->m_flags, MBUF_EXT));
		assert(ISSET(pNewMbuf->m_flags, MBUF_EXT));

		if ISSET
			(iHow, MBUF_ONLY)
		{							   /* Cloning */
			/*
			   Increase reference count
			 */
			if (!pThisMbuf->m_extref)
			{
				//TODO: Be careful to check  _iDataAddrToClusterIndex may be null if cluster is zombie
				CLDataAddr2TagAddr(pThisMbuf->m_extbuf)->cltag_refcnt++;
			}
			else
				(*(pThisMbuf->m_extref)) (pThisMbuf->m_extbuf,
										  pThisMbuf->m_extsize);
		}
		else
		{							   //Copy cluster content to new buffer
			memcpy
				( (void *) pNewMbuf->m_data,(void *) pThisMbuf->m_data,
				 pThisMbuf->m_len);
		}

		iCopied++;
		pThisMbuf = pThisMbuf->m_next;
	}
	assert(pFirstMbuf);

	return pFirstMbuf;

  nospace:
  	if(pFirstMbuf)
		_iFreeMbufChain(pFirstMbuf, MBUF_FREEALL);

	return NULL;
}

/* Duplicate or Clone an mbuf chain.
 *  Depends on the MBUF_ONLY bit. If it is set, we clone. otherwise, we duplicate
 */
static struct rtl_mBuf *_pCopyMbufChain(struct rtl_mBuf *pMbufChain, int32 iOffset, int32 iLength,	 int32 flag)
{
	register struct rtl_mBuf *pNewMbuf, *pThisMbuf, **ppNewMbuf;
	int32     off = iOffset;
	struct rtl_mBuf *pTopMbuf;
	struct rtl_pktHdr *copiedNewHdr=NULL;
	int32     copyhdr = 0;

	pThisMbuf = pMbufChain;

	assert(pThisMbuf && ISSET(pThisMbuf->m_flags, MBUF_USED));
	assert(iLength > 0 && off >= 0);

	if (ISSET(flag, MBUF_ALLOCPKTHDR))
		copyhdr = 1;				   //so we will clone a mbuf chain with its pkthdr

	if (pThisMbuf->m_len == 0){		   //User gave us a mbuf chain without any data.....
		return NULL;
	}

	while (off > 0)
	{								   /* Find from which mbuf we start cloning */
		assert(pThisMbuf && pThisMbuf->m_len);
		if (off < pThisMbuf->m_len)
			break;
		off -= pThisMbuf->m_len;
		pThisMbuf = pThisMbuf->m_next;
	}

	//We don't have so many databytes in chain now. Can't clone from indicated offset
	if (pThisMbuf->m_len < off){
		return NULL;
	}

	ppNewMbuf = &pTopMbuf;
	pTopMbuf = 0;

	while ((iLength > 0) && (pThisMbuf))
	{
		if(ISSET(flag, MBUF_ONLY))  //clone
			pNewMbuf = _m_get(MBUF_ONLY | flag, pThisMbuf->m_type, 1);	// get one buffer pNewMbuf
		else  //duplicate
			pNewMbuf = _m_get(flag, pThisMbuf->m_type, 1);	// get one buffer pNewMbuf and a cluster

		if (pNewMbuf == 0)
			goto nospace;

		*ppNewMbuf = pNewMbuf;

		if (copyhdr){	 //copy pkthdr only for the first mbuf
			if (!_m_gethdr(pNewMbuf, flag))	//get a pkthdr
				goto nospace;
			//copy the whole pkthdr header except the 1st word(4 bytes) to avoid destroy ph_mbuf pointer
			memcpy
				((void *) (((int32 *) pNewMbuf->m_pkthdr) + 1),
				  (void *) (((int32 *) pThisMbuf->m_pkthdr) + 1),
				 sizeof(struct rtl_pktHdr) - 4);

			pNewMbuf->m_flags = pThisMbuf->m_flags;	//copy flags
			SETBITS(pNewMbuf->m_flags, MBUF_PKTHDR);
			if (iLength == MBUF_COPYALL)
				pNewMbuf->m_pkthdr->ph_len -= iOffset;
			else
				pNewMbuf->m_pkthdr->ph_len = iLength;
			copiedNewHdr = pNewMbuf->m_pkthdr;
			copyhdr = 0;
		}

		pNewMbuf->m_len = min(iLength, pThisMbuf->m_len - off);

		if(ISSET(flag, MBUF_ONLY)){
			//Cloning...
			assert(ISSET(pThisMbuf->m_flags, MBUF_EXT));
			pNewMbuf->m_data = pThisMbuf->m_data + off;

			if (!pThisMbuf->m_extref){
				CLDataAddr2TagAddr(pThisMbuf->m_extbuf)->cltag_refcnt++;
			}else
				(*(pThisMbuf->m_extref)) (pThisMbuf->m_extbuf,
										  pThisMbuf->m_extsize);
			pNewMbuf->m_extbuf = pThisMbuf->m_extbuf;
			pNewMbuf->m_extsize = pThisMbuf->m_extsize;
			SETBITS(pNewMbuf->m_flags, MBUF_EXT);
		}else{
			//Duplicate...
			assert(ISSET(pNewMbuf->m_flags, MBUF_EXT));
			assert(CLDataAddr2TagAddr(pThisMbuf->m_extbuf)->cltag_refcnt==1);
			memcpy(pNewMbuf->m_data , pThisMbuf->m_data + off, pNewMbuf->m_len);
		}
		if(copiedNewHdr)
			pNewMbuf->m_pkthdr = copiedNewHdr;

		iLength -= pNewMbuf->m_len;	   /*decrement data bytes to be copied */
		off = 0;
		pThisMbuf = pThisMbuf->m_next;
		ppNewMbuf = &pNewMbuf->m_next;
	}

	return (pTopMbuf);
  nospace:
	if (pTopMbuf)
		_iFreeMbufChain(pTopMbuf, MBUF_FREEALL);
	return (0);
}



