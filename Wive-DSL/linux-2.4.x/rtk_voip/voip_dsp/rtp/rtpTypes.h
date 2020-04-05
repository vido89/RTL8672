#ifndef RTPTYPES_H
#define RTPTYPES_H

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

//static const char* const rtpTypes_h_Version =
//    "$Id: rtpTypes.h,v 1.2 2007/09/28 14:02:46 eric Exp $";

//#include <sys/types.h>
//#include <unistd.h>
//#ifdef KBUILD_BASENAME
#include <linux/types.h>
//#else
//#include <sys/types.h>
//#endif

#include "myTypes.h"
#include "voip_params.h"
#include "NtpTime.h"
//typedef unsigned short		u_int16_t;
//typedef unsigned int		u_int32_t;


#define _idiv32(x, y)		((x) / (y))
#define _imul32(x, y)		((x) * (y))
#define _imod32(x, y)		((x) % (y))


/// Version of this RTP, always 2
//const int RTP_VERSION = 2;
#define RTP_VERSION 2

#define RTP_SEQ_MOD (1<<16)
//const int RTP_SEQ_MOD = 1 << 16;

/// Maximum UDP packet size, 8129
//const int RTP_MTU = 8129;

/// 32-bit sequence number
typedef u_int16_t RtpSeqNumber;

/// Middle 32-bit of NTP
typedef u_int32_t RtpTime;

/// 32-bit source number
typedef u_int32_t RtpSrc;


/* ----------------------------------------------------------------- */
/* --- RTP Structures ---------------------------------------------- */
/* ----------------------------------------------------------------- */

typedef void* RtpPacketPtr;
typedef void* RtpReceiverPtr;
typedef void* RtpTransmitterPtr;

struct stRtpReceiver;


/// RTP packet header
struct RtpHeaderStruct
{
#ifdef __LITTLE_ENDIAN
u_int32_t count:
    4;
u_int32_t extension:
    1;
u_int32_t padding:
    1;
u_int32_t version:
    2;
u_int32_t type:
    7;
u_int32_t marker:
    1;
#else //__BIG_ENDIAN
    /// protocal version
u_int32_t version:
    2;
    /// padding flag - for encryption
u_int32_t padding:
    1;
    /// header extension flag
u_int32_t extension:
    1;
    /// csrc count
u_int32_t count:
    4;
    /// marker bit - for profile
u_int32_t marker:
    1;
    /// payload type
u_int32_t type:
    7;
#endif

    /// sequence number of this packet
u_int32_t sequence :
    16;
    /// timestamp of this packet
    RtpTime timestamp;
    /// source of packet
    RtpSrc ssrc;
    /// list of contributing sources
    RtpSrc startOfCsrc;
};
typedef struct RtpHeaderStruct RtpHeader;


// Transmitter errors
typedef enum
{
    tran_success = 0
} RtpTransmitterError;


// Receiver errors
typedef enum
{
    recv_success = 0,
    recv_bufferEmpty = 20,
    recv_lostPacket = 21
} RtpReceiverError;

// Operation mode
typedef enum
{
    rtprecv_normal,
    rtprecv_droppacket
} RtpReceiverMode;

typedef enum
{
    rtptran_normal,
    rtptran_droppacket
} RtpTransmitMode;

typedef enum
{
	rtpCN_withCodec = 0,
	rtpCN_withAll
} RtpTranCNMode;

#if 1
/* ----------------------------------------------------------------- */
/* --- RTCP Structures --------------------------------------------- */
/* ----------------------------------------------------------------- */

typedef void* RtcpPacketPtr;
typedef void* RtcpReceiverPtr;
typedef void* RtcpTransmitterPtr;


// Supported RTCP types
typedef enum
{
    rtcpTypeSR = 200,
    rtcpTypeRR = 201,
    rtcpTypeSDES = 202,
    rtcpTypeBYE = 203,
    rtcpTypeAPP = 204             // not implemented
} RtcpType;


// Supported SDES types
typedef enum
{
    rtcpSdesEnd = 0,
    rtcpSdesCname = 1,
    rtcpSdesName = 2,
    rtcpSdesEmail = 3,
    rtcpSdesPhone = 4,
    rtcpSdesLoc = 5,
    rtcpSdesTool = 6,
    rtcpSdesNote = 7,
    rtcpSdesPriv = 8              // not implemented
} RtcpSDESType;



/// RTCP header
struct RtcpHeaderStruct
{
#ifdef __LITTLE_ENDIAN
u_int32_t count:
    5;
u_int32_t padding:
    1;
u_int32_t version:
    2;
#else //__BIG_ENDIAN
/// protocal version
u_int32_t version:
    2;
    /// padding flag
u_int32_t padding:
    1;
    /// depending on packet type
u_int32_t count:
    5;
#endif
    /// type of RTCP packet
u_int32_t type:
    8;
    /// lenght of RTCP packet in octlets minus 1
u_int32_t length:
    16;
};
typedef struct RtcpHeaderStruct RtcpHeader;


/// report block
struct RtcpReportStruct
{
    /// source being reported
    RtpSrc ssrc;

    /* endian problem here? - kle */
    /// fraction lost since last report
u_int32_t fracLost:
    8;
    /// cumulative packet lost - signed
    unsigned char cumLost[3];

    /// number of cycles
u_int32_t recvCycles:
    16;
    /// last seq number received
u_int32_t lastSeqRecv:
    16;
    /// interval jitter
    u_int32_t jitter;
    /// last SR packet received from ssrc
    u_int32_t lastSRTimeStamp;
    /// delay since last SR packet
    u_int32_t lastSRDelay;
};
typedef struct RtcpReportStruct RtcpReport;


/// sender info
struct RtcpSenderStruct
{
    /// source of sender
    RtpSrc ssrc;
    /// seconds of NTP
    RtpTime ntpTimeSec;
    /// fractional seconds of NTP
    RtpTime ntpTimeFrac;
    /// transmitter RTP timestamp
    RtpTime rtpTime;
    /// number of packets sent
    u_int32_t packetCount;
    /// number of octlets sent
    u_int32_t octetCount;
};
typedef struct RtcpSenderStruct RtcpSender;


/// bye reason item
struct RtcpByeStruct
{
    /// lenght of text
    u_int8_t length;
    /// reason for leaving, not null-term
    char startOfText;
};
typedef struct RtcpByeStruct RtcpBye;


/// source descrp item
struct RtcpSDESItemStruct
{
    /// type of description
    u_int8_t type;
    /// lenght of item
    u_int8_t length;
    /// text description not null-term
    char startOfText;
};
typedef struct RtcpSDESItemStruct RtcpSDESItem;


/// source descrp chunk
struct RtcpChunkStruct
{
    /// source being described
    RtpSrc ssrc;
    /// list of SDES information, ending with rtcpSdesEnd
    RtcpSDESItem startOfItem;
};
typedef struct RtcpChunkStruct RtcpChunk;


/// SDES information
struct SDESdataStruct
{
    /// CNAME for this source
    char cname [256];
    /// NAME for this source
    char name [256];
    /// EMAIL for this source
    char email [256];
    /// PHONE for this source
    char phone [256];
    /// LOC for this source
    char loc [256];
    /// TOOL for this source
    char tool [256];
    /// NOTE for this source
    char note [256];
};
typedef struct SDESdataStruct SDESdata;

#endif

#if 1
/// receiver information
struct RtpTranInfoStruct
{
	int balloc;

    /// SSRC number for recv
    RtpSrc ssrc;

    struct stRtpReceiver* recv;  /* pointer to receiver for specific information */
    /// number packets expected in last interval
    int expectedPrior;
    /// number of packets actually received in last RTCP interval
    int receivedPrior;

    /// SDES information
    SDESdata SDESInfo;

    /// LSR timestamp which will be one of the fields of the next SR sent out
    u_int32_t lastSRTimestamp;

    /// receiveing time of the last SR received
    NtpTime recvLastSRTimestamp;

};
typedef struct RtpTranInfoStruct RtpTranInfo;
#endif



/* ----------------------------------------------------------------- */
/* --- RTP Session ------------------------------------------------- */
/* ----------------------------------------------------------------- */




// Session errors
typedef enum
{
    session_success = 0,
    session_wrongState = 20
} RtpSessionError;


/* ----------------------------------------------------------------- */
/* --- RTP Events -------------------------------------------------- */
/* ----------------------------------------------------------------- */


struct RtpEventDTMFRFC2833Struct
{
u_int32_t event:
    8;
#ifdef __LITTLE_ENDIAN
u_int32_t volume:
    6;
u_int32_t reserved:
    1;
u_int32_t edge:
    1;
#else
u_int32_t edge:
    1;
u_int32_t reserved:
    1;
u_int32_t volume:
    6;
#endif
u_int32_t duration:
    16;
};
typedef struct RtpEventDTMFRFC2833Struct RtpEventDTMFRFC2833;


struct RtpEventDTMFCiscoRtp
{
u_int32_t sequence:
    8;
#if __BYTE_ORDER == __LITTLE_ENDIAN
u_int32_t level:
    5;
u_int32_t reserved0:
    3;
u_int32_t edge:
    5;
u_int32_t digitType:
    3;
u_int32_t digitCode:
    5;
u_int32_t reserved1:
    3;
#elif __BYTE_ORDER == __BIG_ENDIAN
u_int32_t reserved0:
    3;
u_int32_t level:
    5;
u_int32_t digitType:
    3;
u_int32_t edge:
    5;
u_int32_t reserved1:
    3;
u_int32_t digitCode:
    5;
#else
#error "Problem in <endian.h>"
#endif
};
typedef struct RtpEventDTMFCiscoRtpStruct RtpEventCiscoRtp;
        
enum DTMFEvent
{
    DTMFEventNULL = -1,
    DTMFEventDigit0,
    DTMFEventDigit1,
    DTMFEventDigit2,
    DTMFEventDigit3,
    DTMFEventDigit4,
    DTMFEventDigit5,
    DTMFEventDigit6,
    DTMFEventDigit7,
    DTMFEventDigit8,
    DTMFEventDigit9,
    DTMFEventDigitStar,
    DTMFEventDigitHash
};

enum KeyEvent
{
    KeyEventNULL,
    KeyEventOn,
    KeyEventEdge,
    KeyEventOff
};


/* ----------------------------------------------------------------- */
/* --- RTP and RTCP Receiver and Transmitter Struct ---------------- */
/* ----------------------------------------------------------------- */
#define MAX_TRANINFO_LIST 2	//16	/* I think that each seesion has only one SSRC */

///Struct to transmit RTCP packets
typedef struct stRtcpReceiver
{
	/* list of known sources */
/*	map < RtpSrc, RtpTranInfo* > tranInfoList;*/
	struct RtpTranInfoStruct tranInfoList[MAX_TRANINFO_LIST];
	int tranInfoCnt;

	/* my UDP stack */
/*	UdpStack* myStack;*/
	int localPort;
	int remotePort;

	int packetReceived;

	int accumOneWayDelay;
	int avgOneWayDelay;

	int accumRoundTripDelay;
	int avgRoundTripDelay;

}RtcpReceiver;

typedef struct stRtpReceiver
{
	RtpSeqNumber prevSeqRecv;		/* previous sequence number received */
	RtpSeqNumber prevSeqPlay;		/* previous sequence numer played */
	BOOL sourceSet;					/* source found flag */
	RtpSrc ssrc;					/* SRC number for this source */
	BOOL probationSet;				/* probation set flag */
	RtpSrc srcProbation;			/* wouldn't listen to this source */
	int probation;					/* probation, 0 source valid */
	RtpSeqNumber seedSeq;			/* inital seqence number */

#if 1
	NtpTime seedNtpTime;			/* inital NTP timestamp */
	RtpTime seedRtpTime;        	/* inital RTP timestamp */
#endif

#if 0
	RtpTime sampleRate;				/* rtp interval */
	int baseSampleRate;				/* payload specific sample rate */
	RtpPacket* prevPacket;			/* previous packet */
	NtpTime gotime;					/* next packet play time */
#endif

	int packetReceived;				/* number of packets received */
	int payloadReceived;			/* number of bytes of payload received */

#if 0
	RtpTime prevRtpTime;			/* last RtpTime play */
	NtpTime prevNtpTime;        	/* last NtpTime play */
	RtpSeqNumber prevSeqRecv;		/* previous sequence number received */
	RtpSeqNumber prevSeqPlay;		/* previous sequence numer played */
#endif

	int recvCycles;					/* number of received sequence number cycles */
	int playCycles;					/* number of played sequence number cycles */

#if 1
	int transit;					/* relative transmission time for prev packet */
	int jitter;						/* estimated jitter */
	int jitterTime;					/* jitter time in ms time */
	/* map<RtpSeqNumber, RtpPacket*> jitterBuffer; */ /* jitter buffer */
#endif

	RtcpReceiver* rtcpRecv;			/* additional SDES and RTCP information */
	int bitRate;					/* payload specific sample rate */
	RtpPayloadType payloadFormat; 	/* format of payload for stack */
	int pktSampleSize;				/* number of samples per RTP packet on api (typical 160 or 240) */
	int payloadSize;				/* payload sizes */

#if 0
	int jitterSeed;					/* inital jitter size */
#endif
	RtpReceiverMode recvOpmode;
}RtpReceiver;

typedef struct stRtpTransmitter
{
	RtpSrc ssrc;					/* local SRC number */
	NtpTime seedNtpTime;			/* inital NTP time */
	RtpTime seedRtpTime;			/* inital RTP time */
	RtpTime prevRtpTime;			/* RTP time of previous packet */
	NtpTime prevNtpTime;			/* NTP time of previous packet */
	RtpSeqNumber prevSequence;		/* previous packet's sequence */
	int packetSent;					/* numbers of packet sent */
	int payloadSent;				/* total bytes of payload sent */
	RtpPayloadType payloadFormat;	/* format of payload for stack */
	int pktSampleSize;				/* number of samples per RTP packet on api (typical 160 or 240) */
	int payloadSize;				/* payload sizes */
	BOOL markerOnce;				/* marker once */
	RtpTransmitMode tranOpmode;
	int bitRate;					/* number of bytes per sample  (typical 1 or 2, used for endian conversion) */

}RtpTransmitter;

///Struct to transmit RTCP packets
typedef struct stRtcpTransmitter
{

	/* Next time to submit RTCP packet */
	NtpTime nextInterval;

	/** Transmitter SDES information
	 *  data stored as null-term strings
	**/
	SDESdata* SDESInfo;
	SDESdata SDESdataBuf[1];

	RtpTransmitter* tran;

	RtpReceiver* recv;

	RtcpReceiver* rtcpRecv;

	/* my UDP stack */
/*	UdpStack* myStack; */
	int localPort;
	int remotePort;

/*	NetworkAddress remoteAddr; */
}RtcpTransmitter;


#endif // RTPTYPES_H
