
/* ====================================================================
 * The Vovida Software License, Version 1.0 
 * 
 * Copyright (c) 2000 Vovida Networks, Inc.  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 
 * 3. The names "VOCAL", "Vovida Open Communication Application Library",
 *    and "Vovida Open Communication Application Library (VOCAL)" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact vocal@vovida.org.
 *
 * 4. Products derived from this software may not be called "VOCAL", nor
 *    may "VOCAL" appear in their name, without prior written
 *    permission of Vovida Networks, Inc.
 * 
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL VOVIDA
 * NETWORKS, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT DAMAGES
 * IN EXCESS OF $1,000, NOR FOR ANY INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 * 
 * ====================================================================
 * 
 * This software consists of voluntary contributions made by Vovida
 * Networks, Inc. and many individuals on behalf of Vovida Networks,
 * Inc.  For more information on Vovida Networks, Inc., please see
 * <http://www.vovida.org/>.
 *
 */

//static const char* const RtcpReceiver_cxx_Version =
//    "$Id: RtcpReceiver.c,v 1.2 2007/09/28 14:02:46 eric Exp $";

/* Kao
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
*/

/*#include <assert.h>
#include <time.h>
#include <sys/types.h>*/
/* Kao
#include "vtypes.h"
*/
/*#include <unistd.h>
#include <string.h>
#include <map>*/

/* Kao
// networking
#include <sys/types.h>
#include <sys/socket.h>
*/
/*#include <netinet/in.h> */

#ifdef DEBUG_LOG
#include "cpLog.h"
#endif
/* Kao
#include "vsock.hxx"
*/
#include <linux/string.h>

#include "rtpTypes.h"
#include "rtpTools.h"
#include "NtpTime.h"
#include "Rtp.h"
#include "Rtcp.h"
#include "types.h"
#include "myTypes.h"
/*#include <debug.h>*/

#ifdef SUPPORT_RTCP

RtcpPacket RTCP_RX_DEC[MAX_SESS_NUM][RTCP_RX_DEC_NUM];
/* static variable*/
static int rtcp_cur_get[MAX_SESS_NUM];

RtcpReceiver RtcpRxInfo[MAX_SESS_NUM];

RtpSessionState sessionState[MAX_SESS_NUM];
RtpSessionError sessionError[MAX_SESS_NUM];

/* ----------------------------------------------------------------- */
/* --- rtpcReceiver Constructor ------------------------------------ */
/* ----------------------------------------------------------------- */

void RtcpRx_Init (void)
{
	uint32 sid;
	for(sid=0; sid<SESS_NUM; sid++)
	{
		RtcpRx_InitByID(sid);
    }
}

void RtcpRx_InitByID (uint32 sid)
{
    RtcpReceiver* pRxInfo = NULL;
    RtpTranInfo *info;
    int i;

	if(sid >= SESS_NUM)
		return;

	pRxInfo = &RtcpRxInfo[sid];

	pRxInfo->packetReceived = 0;

	pRxInfo->accumOneWayDelay = 0;
	pRxInfo->avgOneWayDelay = 0;
	pRxInfo->accumRoundTripDelay = 0;
    pRxInfo->avgRoundTripDelay = 0;
	pRxInfo->tranInfoCnt = 0;

/*	info = pRxInfo->tranInfoList;*/
	for(i=0; i<MAX_TRANINFO_LIST; i++)
	{
		info = &pRxInfo->tranInfoList[i];
		info->balloc = 0;
	    	info->ssrc = 0;
/*		info += sizeof(RtpTranInfo);*/
/*	    	info++;*/
	}

	rtcp_cur_get[sid] = 0;
}

#if 0
void RtcpRx_Close (void)
{
    // must remove each transmitter block and each SDES info
    map < RtpSrc, RtpTranInfo* > ::iterator s = tranInfoList.begin();
    while (s != tranInfoList.end())
    {
        removeTranInfo((s->second)->ssrc);
        s = tranInfoList.begin();
    }
    //cpLog(LOG_DEBUG_STACK, "RTCP: Receiver removed");
}
#endif

RtcpReceiver* RtcpRx_getInfo (uint32 sid)
{
    RtcpReceiver* pRxInfo = NULL;

	if(sid >= SESS_NUM)
		return NULL;

	pRxInfo = &RtcpRxInfo[sid];
	return pRxInfo;
}

/* --- receive packet functions ------------------------------------ */


RtcpPacket* RtcpRx_getPacket (uint32 sid)
{
	int *cur_get = NULL;

	if(sid >= SESS_NUM)
		return NULL;

	cur_get = &rtcp_cur_get[sid];
	
    /* create packet */
	RtcpPacket* p = &RTCP_RX_DEC[sid][*cur_get];

	if(p->packetState == PKT_FREE)
		return NULL;
	
	//printk("cur_get=%d\n", *cur_get);
	
	(*cur_get)++;
	*cur_get &= (RTCP_RX_DEC_NUM - 1);
	

	/*	RtcpPkt_Init(p);*/
	p->packetAlloc = PACKETSIZE;
	p->unusedSize = p->packetAlloc;
	/*    RtcpPkt_setTotalUsage(p, len);*/

    // check packet
	/*    if (!RtcpRx_isValid(p))
	{
		p->packetState = PKT_FREE;
		p = NULL;
		return NULL;
	}*/
    return p;
}

int RtcpRx_freePacket(RtcpPacket* p)
{
	if(p == NULL)
	{
		//printk("p=NULL, Can't Free Packet\n");
		return -1;
	}

	if(p->packetState == PKT_FREE)
	{
		//printk("Packet is Free, Can't Free Packet again\n");
		return -1;
	}

	p->packetState = PKT_FREE;
	
	//printk("Free Packet\n");
	//printk("\n");

	return 0;
}

int RtcpRx_isValid (RtcpPacket* p)
{
    char* begin = (char*) (RtcpPkt_getPacketData(p));
    char* end = (char*) (begin + RtcpPkt_getTotalUsage(p));
    RtcpHeader* middle = (RtcpHeader*) (begin);

    // check if known payload type for first packet
    if (middle->type != rtcpTypeSR && middle->type != rtcpTypeRR)
        return 0;

    // check padbyte
    if (middle->padding)
        return 0;

    // check header lengths
    while (begin < end && (int)middle->version == RTP_VERSION)
    {
        begin += _mul32((ntohs(middle->length) + 1), sizeof(RtpSrc));
        middle = (RtcpHeader*) (begin);
    }

    if (begin != end)
        return 0;

    // exit with success
#ifdef DEBUG_LOG
    cpLog(LOG_DEBUG_STACK, "RTCP packet is valid");
#endif
    //    cout << "RTCP packet is valid" << endl;
    return 1;
}

int RtcpRx_receiveRTCP (uint32 sid)
{
        extern RtpSessionState sessionState[MAX_SESS_NUM];
        extern RtpSessionError sessionError[MAX_SESS_NUM];
	RtcpPacket* p;
	RtcpPacket* p1;

	if(sid >= SESS_NUM)
	{
		printk("sid >= SESS_NUM, return !\n");
		return -1;
	}	

    if ( !( sessionState[sid] == rtp_session_sendrecv
            || sessionState[sid] == rtp_session_recvonly ) )
    {
        p1 = RtcpRx_getPacket(sid);
        
        if (p1) 
        	RtcpRx_freePacket(p1);
        	
        sessionError[sid] = session_wrongState;

	//printk("RTCP stack can't receive. Wrong state"); 

        return -1;
    }

    // generate compound packet
    p = RtcpRx_getPacket(sid);

	if(p == NULL)
		return -1;
		
	//printk("RtcpRx-1\n");

    int ret = 0;

    // read compound packet
    if (RtcpRx_readRTCP(sid, p) == 1)
    {
       ret = 1;
    }
    //printk("R%d\n", sid);

    RtcpRx_freePacket(p);
    
    return ret;
}


int RtcpRx_readRTCP (uint32 sid, RtcpPacket* p)
{

    char* begin = (char*) (RtcpPkt_getPacketData(p));
    char* end = (char*) (begin + RtcpPkt_getTotalUsage(p));
    RtcpHeader* middle = NULL;
    int ret = 0;

    while (begin < end)
    {
        middle = (RtcpHeader*) (begin);
        switch (middle->type)
        {
            case (rtcpTypeSR):
            case (rtcpTypeRR):
            	RtcpRx_readSR_H (sid, middle);
            	break;
            case (rtcpTypeSDES):
                RtcpRx_readSDES_H (sid, middle);
            	break;
            case (rtcpTypeBYE):
            	if ( RtcpRx_readBYE_H (sid, middle) == 0)
                {
                	ret = 1;
                }
            	break;
            case (rtcpTypeAPP):
            	RtcpRx_readAPP_H (sid, middle);
            	break;
            default:
#ifdef DEBUG_LOG
            cpLog (LOG_ERR, "RTCP: Unknown RTCP type");
#endif
            	break;
        }
/*        begin += _mul32((ntohs(middle->length) + 1), sizeof(u_int32_t));*/
		begin += _mul32((middle->length + 1), sizeof(u_int32_t));
    }
    return ret;
}



RtcpHeader* RtcpRx_findRTCP (RtcpPacket* p, RtcpType type)
{
    char* begin = (char*) (RtcpPkt_getPacketData(p));
    char* end = (char*) (begin + RtcpPkt_getTotalUsage(p));
    RtcpHeader* middle = NULL;

    while (begin < end)
    {
        middle = (RtcpHeader*) (begin);
        if (type == (RtcpType) (middle->type))
            return middle;
        begin += _mul32((ntohs(middle->length) + 1), sizeof(u_int32_t));
    }

    // packet type not found
#ifdef DEBUG_LOG
    cpLog (LOG_ERR, "RTCP: Type found here: %d", (int)type);
#endif
    return NULL;
}



/* --- Read SR RTCP packet ----------------------------------------- */

int RtcpRx_readSR (uint32 sid, RtcpPacket* p)
{
    RtcpHeader* head = RtcpRx_findRTCP (p, rtcpTypeSR);
    if (head == NULL) head = RtcpRx_findRTCP (p, rtcpTypeRR);
    if (head == NULL) return -1;

    RtcpRx_readSR_H (sid, head);

    // read next RR packet if found
    // future: - ?

    return 0;
}

/* read SR by Header*/
void RtcpRx_readSR_H (uint32 sid, RtcpHeader* head)
{
    char* middle = NULL;
    RtcpReceiver* pRxInfo = NULL;

	NtpTime nowNtp, nowNtp1;
	NtpTime thenNtp;
	int i;

	if(sid >= SESS_NUM)
		return;

	pRxInfo = &RtcpRxInfo[sid];

    Ntp_getTime(&nowNtp);

    // read SR block
    if (head->type == rtcpTypeSR)
    {
		RtcpSender* senderBlock = (RtcpSender*)((char*)head + sizeof(RtcpHeader));
/*        RtpTranInfo* s = RtcpRx_findTranInfo(sid, ntohl(senderBlock->ssrc)); */

		RtpTranInfo* s = RtcpRx_findTranInfo(sid, senderBlock->ssrc);

/*		s->lastSRTimestamp = (ntohl(senderBlock->ntpTimeSec) << 16 | ntohl(senderBlock->ntpTimeFrac) >> 16); */
		s->lastSRTimestamp = ((senderBlock->ntpTimeSec) << 16 | (senderBlock->ntpTimeFrac) >> 16);
/*        s->recvLastSRTimestamp = nowNtp; */

		Ntp_cpy(&s->recvLastSRTimestamp, &nowNtp);

        //printSR (senderBlock);  // - debug

        pRxInfo->packetReceived++;

/*		NtpTime thenNtp ( ntohl(senderBlock->ntpTimeSec), ntohl(senderBlock->ntpTimeFrac) ); */
		Ntp_TimeInit(&thenNtp, senderBlock->ntpTimeSec, senderBlock->ntpTimeFrac);

/*      pRxInfo->accumOneWayDelay += (nowNtp - thenNtp); */
        pRxInfo->accumOneWayDelay += NTP_sub(&nowNtp, &thenNtp);

        pRxInfo->avgOneWayDelay = _idiv32(pRxInfo->accumOneWayDelay, pRxInfo->packetReceived);

        middle = (char*)head + sizeof(RtcpHeader) + sizeof(RtcpSender);
    }
    else
    {
        // move over blank SR header
        middle = (char*)head + sizeof(RtcpHeader);

        // move over the ssrc of packet sender
        RtpSrc* sender = (RtpSrc*) (middle);
        RtpSrc ssrc;

/*        ssrc = ntohl(*sender); */
		ssrc = *sender;
        middle += sizeof(RtpSrc);

        pRxInfo->packetReceived++;
    }

    // read RR blocks
    RtcpReport* block = (RtcpReport*) (middle);
    for (i = head->count; i > 0; i--)
    {
        //printRR (block);  // - debug

        // - ? what these are if the count is more than 1??
/*        NtpTime thenNtp (ntohl(block->lastSRTimeStamp) >> 16, ntohl(block->lastSRTimeStamp) << 16 ); */
		Ntp_TimeInit(&thenNtp, (block->lastSRTimeStamp) >> 16, (block->lastSRTimeStamp) << 16);

/*        NtpTime nowNtp1 (nowNtp.getSeconds() & 0x0000FFFF, nowNtp.getFractional() & 0xFFFF0000); */
		Ntp_TimeInit(&nowNtp1, nowNtp.seconds & 0x0000FFFF, nowNtp.fractional & 0xFFFF0000);
/*        pRxInfo->accumRoundTripDelay += ((nowNtp1 - thenNtp) - ntohl(block->lastSRDelay)); */
		pRxInfo->accumRoundTripDelay += (NTP_sub(&nowNtp1, &thenNtp) - (block->lastSRDelay));

        pRxInfo->avgRoundTripDelay = _idiv32(pRxInfo->accumRoundTripDelay, pRxInfo->packetReceived);

        ++block;
    }

    // handle profile specific extensions
    // - ?
}

/* --- Read SDES packet -------------------------------------------- */

int RtcpRx_readSDES (uint32 sid, RtcpPacket* p)
{
    RtcpHeader* head = RtcpRx_findRTCP (p, rtcpTypeSDES);
    if (head == NULL) return -1;

    RtcpRx_readSDES_H (sid, head);

    // read next SDES packet if found
    // future: - ?

    return 0;
}


void RtcpRx_readSDES_H (uint32 sid, RtcpHeader* head)
{
	int i;
    char* begin = (char*) ((char*)head + sizeof(RtcpHeader));
    RtcpChunk* middle = (RtcpChunk*) (begin);

    RtcpSDESItem* item = NULL;
    RtcpSDESItem* nextitem = NULL;
    RtpSrc ssrc;

    for (i = head->count; i > 0; i--)
    {
/*		ssrc = ntohl(middle->ssrc); */
		ssrc = middle->ssrc;

        for (item = &(middle->startOfItem); item->type; item = nextitem)
        {
            RtcpRx_addSDESItem(sid, ssrc, item);
            nextitem = (RtcpSDESItem*)((char*)item + sizeof(RtcpSDESItem) - 1 + item->length);
        }

        middle = (RtcpChunk*) (item);
    }
}



void RtcpRx_addSDESItem (uint32 sid, RtpSrc ssrc, RtcpSDESItem* item)
{
    RtpTranInfo* s = RtcpRx_findTranInfo(sid, ssrc);

    switch (item->type)
    {
        case rtcpSdesCname:
			strncpy ((s->SDESInfo).cname, &(item->startOfText), item->length + 1);
			break;
        case rtcpSdesName:
			strncpy ((s->SDESInfo).name, &(item->startOfText), item->length + 1);
			break;
        case rtcpSdesEmail:
			strncpy ((s->SDESInfo).email, &(item->startOfText), item->length + 1);
			break;
        case rtcpSdesPhone:
			strncpy ((s->SDESInfo).phone, &(item->startOfText), item->length + 1);
			break;
        case rtcpSdesLoc:
			strncpy ((s->SDESInfo).loc, &(item->startOfText), item->length + 1);
			break;
        case rtcpSdesTool:
			strncpy ((s->SDESInfo).tool, &(item->startOfText), item->length + 1);
			break;
        case rtcpSdesNote:
			strncpy ((s->SDESInfo).note, &(item->startOfText), item->length + 1);
			break;
        case rtcpSdesPriv:
			// future: not implemented
        default:
#ifdef DEBUG_LOG
			cpLog (LOG_ERR, "RtcpReceiver: SDES type unknown");
#endif
			break;
    }

    /*
    // - debug
    cerr <<"Update "<<src<<" with "<< (int) item->type <<" "<< (int) item->length;
    char output [255];
    memset (output, 0, 255);
    strncpy (output, &(item->startOfText), item->length+1);
    cerr << endl <<output<<endl;
    cerr <<"_SDES_";
    */
}



/* --- Read BYE packet --------------------------------------------- */

int RtcpRx_readBYE (uint32 sid, RtcpPacket* p)
{
    RtcpHeader* head = RtcpRx_findRTCP (p, rtcpTypeBYE);
    if (head == NULL) return -1;

    RtcpRx_readBYE_H (sid, head);

    // read next BYE packet if found
    // future: - ?

    return 0;
}


int RtcpRx_readBYE_H (uint32 sid, RtcpHeader* head)
{

	int i;
    //    char* end = reinterpret_cast<char*>
    //                ((char*)head + sizeof(RtpSrc) * (ntohs(head->length) + 1));
    RtpSrc* src = (RtpSrc*)((char*)head + sizeof(RtcpHeader));


    for (i = head->count; i > 0; i--)
    {
#ifdef DEBUG_LOG
        cpLog( LOG_DEBUG_STACK, "readRtcpBye for %d", ntohl(*src) );
#endif
        //       cerr << "readRtcpBye for " << ntohl(*src) << endl;
/*		RtcpRx_removeTranInfo (sid, ntohl(*src++)); */
		RtcpRx_removeTranInfo (sid, *src++, 0);
	}

    return 0;
}



/* --- Read APP packet --------------------------------------------- */

int RtcpRx_readAPP (uint32 sid, RtcpPacket* p)
{
    RtcpHeader* head = RtcpRx_findRTCP (p, rtcpTypeAPP);
    if (head == NULL) return -1;

    RtcpRx_readAPP_H (sid, head);

    // read next APP packet if found
    // future: - ?

    return 0;
}


void RtcpRx_readAPP_H (uint32 sid, RtcpHeader* head)
{
    // future: not implemented
    assert (0);
}


/* --- known transmitter list functions ---------------------------- */

RtpTranInfo* RtcpRx_addTranInfo (uint32 sid, RtpSrc ssrc, RtpReceiver* recv)
{
	int nResult = 0;
    RtpTranInfo s;
	RtpTranInfo* info;

	if (recv) assert (ssrc == recv->ssrc);

	s.recv = recv;
    s.ssrc = ssrc;
    s.expectedPrior = 0;
    s.receivedPrior = 0;

	info = &s;

	nResult = RtcpRx_addTranFinal (sid, info);
    if (nResult == 1)
    {
        info = RtcpRx_findTranInfo (sid, ssrc);
        assert (info->recv == NULL);  // - ?
        info->recv = recv;
    }
    else if(nResult == -1)
    	return NULL;
    return info;
}

int RtcpRx_addTranFinal (uint32 sid, RtpTranInfo* s)
{
	RtcpReceiver* pRxInfo;
	RtpTranInfo* info;
	int i;

	if(sid >= SESS_NUM)
		return -1;
	pRxInfo = &RtcpRxInfo[sid];

	for(i=0; i<MAX_TRANINFO_LIST; i++)
	{
		info = &pRxInfo->tranInfoList[i];
		if(info->ssrc == s->ssrc)	/* transmitter already in listing */
			return 1;
	}

	for(i=0; i<MAX_TRANINFO_LIST; i++)
	{
		info = &pRxInfo->tranInfoList[i];
		if(info->balloc == 0)
			break;
	}

	if(i == MAX_TRANINFO_LIST)
		return -1;

	info->balloc = 1;
	info->recv =  s->recv;
	info->ssrc = s->ssrc;
	info->expectedPrior = s->expectedPrior;
	info->receivedPrior = s->receivedPrior;
	pRxInfo->tranInfoCnt++;
	
	s = info;
	printk("RTCP(%d): Transmitter add: %u, cnt: %u\n", sid, s->ssrc, pRxInfo->tranInfoCnt);
    return 0;
}



int RtcpRx_removeTranInfo (uint32 sid, RtpSrc ssrc, int flag)
{
	RtcpReceiver* pRxInfo;
	RtpTranInfo* info, *preInfo;
	int i;

	if(sid >= SESS_NUM)
		return -1;
	pRxInfo = &RtcpRxInfo[sid];

/*	info = pRxInfo->tranInfoList; */
	for(i=0; i<MAX_TRANINFO_LIST; i++)
	{
		info = &pRxInfo->tranInfoList[i];
		if(info->ssrc == ssrc)
			break;
/*		info += sizeof(RtpTranInfo); */
	}

    if (i == MAX_TRANINFO_LIST)
    {
        /* ssrc not found */
        assert (0);
    }

    /* remove from RTP stack */
    if (!flag)
        removeSource(sid, info->ssrc, 1);
    info->balloc = 0;
	info->recv = NULL;

    /* printk("RTCP: done removing\n"); */
	for(i=i+1; i<MAX_TRANINFO_LIST; i++)
	{
		preInfo = info;
		info = &pRxInfo->tranInfoList[i];
/*		info += sizeof(RtpTranInfo);*/
/*		info++;*/
		if(info->balloc != 0)
		{
			preInfo->balloc = info->balloc;
			preInfo->recv = info->recv;
			info->balloc = 0;
			info->recv = NULL;
		}
	}
	pRxInfo->tranInfoCnt--;

    /* printk ("RTCP: Transmitter removed: %d", ssrc); */
    return 0;
}



RtpTranInfo* RtcpRx_findTranInfo (uint32 sid, RtpSrc ssrc)
{
	RtcpReceiver* pRxInfo = NULL;
    RtpTranInfo* info = NULL;
    RtpTranInfo* p = NULL;
    int i;

   	if(sid >= SESS_NUM)
		return NULL;
	pRxInfo = &RtcpRxInfo[sid];
	
/*	info = pRxInfo->tranInfoList; */
	for(i=0; i<MAX_TRANINFO_LIST; i++)
	{
		info = &pRxInfo->tranInfoList[i];
		if(info->ssrc == ssrc)
			break;
	}

    if (i == MAX_TRANINFO_LIST)        // receiver not found, so add it
    {
        info = RtcpRx_addTranInfo(sid, ssrc, NULL);
    }
	else if(info->balloc == 0)
	{
		p = pRxInfo->tranInfoList;
		for(i=0; i<MAX_TRANINFO_LIST; i++)
		{
			if(p->balloc == 0)
				break;
			p++;
		}

		if(i == MAX_TRANINFO_LIST)
			printk("Find tran wrong!\n");

		if(p != info)
		{
			p->ssrc = info->ssrc;
			p->expectedPrior = info->expectedPrior;
			p->receivedPrior = info->receivedPrior;
			pRxInfo->tranInfoCnt++;
			info->ssrc = 0;
			info = p;
		}
		info->balloc = 1;
	}

    return info;
}


RtpTranInfo* RtcpRx_getTranInfoList (uint32 sid, int index)
{
	RtcpReceiver* pRxInfo = NULL;
    RtpTranInfo* info = NULL;
/*    int i; */

   	if(sid >= SESS_NUM)
		return NULL;
		
	if(index >= MAX_TRANINFO_LIST)
		return NULL;
	pRxInfo = &RtcpRxInfo[sid];

/*	info = pRxInfo->tranInfoList; */

/*    for (i = 0; i < index; i++)
        ++info; */
	info = &pRxInfo->tranInfoList[index];

    return info;
}

int RtcpRx_getTranInfoCount(uint32 sid)
{
	RtcpReceiver* pRxInfo = &RtcpRxInfo[sid];
    return pRxInfo->tranInfoCnt;
}

int RtcpRx_getPort (uint32 sid)
{
	RtcpReceiver* pRxInfo = &RtcpRxInfo[sid];
    return pRxInfo->localPort;
}

/* get the data for latency */
int RtcpRx_getAvgOneWayDelay(uint32 sid)
{
	RtcpReceiver* pRxInfo = &RtcpRxInfo[sid];
	return pRxInfo->avgOneWayDelay;
}
int RtcpRx_getAvgRoundTripDelay(uint32 sid)
{
	RtcpReceiver* pRxInfo = &RtcpRxInfo[sid];
	return pRxInfo->avgRoundTripDelay;
}

#endif /* SUPPORT_RTCP */

