/*
 * Copyright (c) 1988, 1989, 1990, 1991, 1992, 1993, 1994, 1995, 1996, 1997
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * @(#) $Header: /tcpdump/master/tcpdump/interface.h,v 1.149 2001/01/02 22:47:06 guy Exp $ (LBL)
 */

#ifndef tcpdump_interface_h
#define tcpdump_interface_h

#ifdef HAVE_OS_PROTO_H
#include "os-proto.h"
#endif
#include <sys/types.h>
#include <sys/time.h>

#ifndef HAVE___ATTRIBUTE__
#define __attribute__(x)
#endif

/* snprintf et al */

#include <stdarg.h>

#if !defined(HAVE_SNPRINTF)
int snprintf (char *str, size_t sz, const char *format, ...)
     __attribute__ ((format (printf, 3, 4)));
#endif

#if !defined(HAVE_VSNPRINTF)
int vsnprintf (char *str, size_t sz, const char *format, va_list ap)
     __attribute__((format (printf, 3, 0)));
#endif

#ifndef HAVE_STRLCAT
extern size_t strlcat (char *, const char *, size_t);
#endif
#ifndef HAVE_STRLCPY
extern size_t strlcpy (char *, const char *, size_t);
#endif

struct tok {
	int v;			/* value */
	char *s;		/* string */
};

extern int aflag;		/* translate network and broadcast addresses */
extern int dflag;		/* print filter code */
extern int eflag;		/* print ethernet header */
extern int fflag;		/* don't translate "foreign" IP address */
extern int nflag;		/* leave addresses as numbers */
extern int Nflag;		/* remove domains from printed host names */
extern int qflag;		/* quick (shorter) output */
extern int Rflag;		/* print sequence # field in AH/ESP*/
extern int sflag;		/* use the libsmi to translate OIDs */
extern int Sflag;		/* print raw TCP sequence numbers */
extern int tflag;		/* print packet arrival time */
extern int uflag;		/* Print undecoded NFS handles */
extern int vflag;		/* verbose */
extern int xflag;		/* print packet in hex */
extern int Xflag;		/* print packet in hex/ascii */

extern char *espsecret;

extern int packettype;		/* as specified by -T */
#define PT_VAT		1	/* Visual Audio Tool */
#define PT_WB		2	/* distributed White Board */
#define PT_RPC		3	/* Remote Procedure Call */
#define PT_RTP		4	/* Real-Time Applications protocol */
#define PT_RTCP		5	/* Real-Time Applications control protocol */
#define PT_SNMP		6	/* Simple Network Management Protocol */
#define PT_CNFP		7	/* Cisco NetFlow protocol */

#ifndef min
#define min(a,b) ((a)>(b)?(b):(a))
#endif
#ifndef max
#define max(a,b) ((b)>(a)?(b):(a))
#endif

#ifndef INET6
/*
 * The default snapshot length.  This value allows most printers to print
 * useful information while keeping the amount of unwanted data down.
 * In particular, it allows for an ethernet header, tcp/ip header, and
 * 14 bytes of data (assuming no ip options).
 */
#define DEFAULT_SNAPLEN 68
#else
#define DEFAULT_SNAPLEN 96
#endif

#ifndef BIG_ENDIAN
#define BIG_ENDIAN 4321
#define LITTLE_ENDIAN 1234
#endif

#define ESRC(ep) ((ep)->ether_shost)
#define EDST(ep) ((ep)->ether_dhost)

#ifndef NTOHL
#define NTOHL(x)	(x) = ntohl(x)
#define NTOHS(x)	(x) = ntohs(x)
#define HTONL(x)	(x) = htonl(x)
#define HTONS(x)	(x) = htons(x)
#endif
#endif

extern char *program_name;	/* used to generate self-identifying messages */

extern int32_t thiszone;	/* seconds offset from gmt to local time */

extern int snaplen;
/* global pointers to beginning and end of current packet (during printing) */
extern const u_char *packetp;
extern const u_char *snapend;

/* True if  "l" bytes of "var" were captured */
#define TTEST2(var, l) ((u_char *)&(var) <= snapend - (l))

/* True if "var" was captured */
#define TTEST(var) TTEST2(var, sizeof(var))

/* Bail if "l" bytes of "var" were not captured */
#define TCHECK2(var, l) if (!TTEST2(var, l)) goto trunc

/* Bail if "var" was not captured */
#define TCHECK(var) TCHECK2(var, sizeof(var))

struct timeval;

extern void ts_print(const struct timeval *);
extern void relts_print(int);

extern int fn_print(const u_char *, const u_char *);
extern int fn_printn(const u_char *, u_int, const u_char *);
extern const char *tok2str(const struct tok *, const char *, int);
extern char *dnaddr_string(u_short);

extern void wrapup(int);

extern void error(const char *, ...)
    __attribute__((noreturn, format (printf, 1, 2)));
extern void warning(const char *, ...) __attribute__ ((format (printf, 1, 2)));

extern char *read_infile(char *);
extern char *copy_argv(char **);

extern void safeputchar(int);
extern void safeputs(const char *);

extern char *isonsap_string(const u_char *);
extern char *llcsap_string(u_char);
extern char *protoid_string(const u_char *);
extern char *dnname_string(u_short);
extern char *dnnum_string(u_short);

/* The printer routines. */

struct pcap_pkthdr;

extern void ascii_print_with_offset(const u_char *, u_int, u_int);    
extern void ascii_print(const u_char *, u_int);    
extern void hex_print_with_offset(const u_char *, u_int, u_int);    
extern void telnet_print(const u_char *, u_int);    
extern void hex_print(const u_char *, u_int);    
extern int ether_encap_print(u_short, const u_char *, u_int, u_int, u_short *);
extern int llc_print(const u_char *, u_int, u_int, const u_char *,
	const u_char *, u_short *);
extern void aarp_print(const u_char *, u_int);
extern void arp_print(const u_char *, u_int, u_int);
extern void atalk_print(const u_char *, u_int);
extern void atm_if_print(u_char *, const struct pcap_pkthdr *, const u_char *);
extern void bootp_print(const u_char *, u_int, u_short, u_short);
extern void bgp_print(const u_char *, int);
extern void bxxp_print(const u_char *, u_int);
extern void cnfp_print(const u_char *cp, u_int len, const u_char *bp);
extern void decnet_print(const u_char *, u_int, u_int);
extern void default_print(const u_char *, u_int);
extern void default_print_unaligned(const u_char *, u_int);
extern void dvmrp_print(const u_char *, u_int);
extern void egp_print(const u_char *, u_int, const u_char *);
extern void ether_if_print(u_char *, const struct pcap_pkthdr *,
	const u_char *);
extern void token_if_print(u_char *, const struct pcap_pkthdr *,
	const u_char *);
extern void fddi_if_print(u_char *, const struct pcap_pkthdr *, const u_char *);
extern void gre_print(const u_char *, u_int);
extern void icmp_print(const u_char *, u_int, const u_char *);
extern void igmp_print(const u_char *, u_int, const u_char *);
extern void igrp_print(const u_char *, u_int, const u_char *);
extern void ip_print(const u_char *, u_int);
extern void ipN_print(const u_char *, u_int);
extern void ipx_print(const u_char *, u_int);
extern void isoclns_print(const u_char *, u_int, u_int, const u_char *,
	const u_char *);
extern void krb_print(const u_char *, u_int);
extern void llap_print(const u_char *, u_int);
extern void nfsreply_print(const u_char *, u_int, const u_char *);
extern void nfsreq_print(const u_char *, u_int, const u_char *);
extern void ns_print(const u_char *, u_int);
extern void ntp_print(const u_char *, u_int);
extern void null_if_print(u_char *, const struct pcap_pkthdr *, const u_char *);
extern void ospf_print(const u_char *, u_int, const u_char *);
extern void pimv1_print(const u_char *, u_int);
extern void cisco_autorp_print(const u_char *, u_int);
extern void mobile_print(const u_char *, u_int);
extern void pim_print(const u_char *, u_int);
extern void pppoe_print(const u_char *, u_int);
extern void ppp_print(register const u_char *, u_int);
extern void ppp_if_print(u_char *, const struct pcap_pkthdr *, const u_char *);
extern void ppp_hdlc_if_print(u_char *, const struct pcap_pkthdr *,
	const u_char *);
extern void ppp_bsdos_if_print(u_char *, const struct pcap_pkthdr *,
	const u_char *);
extern int vjc_print(register const char *, register u_int, u_short);
extern void raw_if_print(u_char *, const struct pcap_pkthdr *, const u_char *);
extern void rip_print(const u_char *, u_int);
extern void sl_if_print(u_char *, const struct pcap_pkthdr *, const u_char *);
extern void lane_if_print(u_char *, const struct pcap_pkthdr *,const u_char *);
extern void cip_if_print(u_char *, const struct pcap_pkthdr *,const u_char *);
extern void sl_bsdos_if_print(u_char *, const struct pcap_pkthdr *,
    const u_char *);
extern void chdlc_if_print(u_char *, const struct pcap_pkthdr *,
    const u_char *);
extern void sll_if_print(u_char *, const struct pcap_pkthdr *, const u_char *);
extern void snmp_print(const u_char *, u_int);
//extern void sunrpcrequest_print(const u_char *, u_int, const u_char *);
extern void tcp_print(const u_char *, u_int, const u_char *, int);
extern void tftp_print(const u_char *, u_int);
extern void timed_print(const u_char *, u_int);
extern void udp_print(const u_char *, u_int, const u_char *, int);
extern void wb_print(const void *, u_int);
extern int ah_print(register const u_char *, register const u_char *);
extern int esp_print(register const u_char *, register const u_char *, int *);
extern void isakmp_print(const u_char *, u_int, const u_char *);
extern int ipcomp_print(register const u_char *, register const u_char *, int *);
extern void rx_print(register const u_char *, int, int, int, u_char *);
extern void netbeui_print(u_short, const u_char *, const u_char *);
extern void ipx_netbios_print(const u_char *, const u_char *);
extern void nbt_tcp_print(const u_char *, int);
extern void nbt_udp137_print(const u_char *data, int);
extern void nbt_udp138_print(const u_char *data, int);
extern char *smb_errstr(int, int);
extern void print_data(const unsigned char *, int);
extern void l2tp_print(const u_char *, u_int);
extern void lcp_print(const u_char *, u_int);
extern void vrrp_print(const u_char *bp, u_int len, int ttl);
extern void cdp_print(const u_char *p, u_int length, u_int caplen,
		      const u_char *esrc, const u_char *edst);
extern void stp_print(const u_char *p, u_int length);
extern void radius_print(const u_char *, u_int);

#ifdef INET6
extern void ip6_print(const u_char *, int);
extern void ip6_opt_print(const u_char *, int);
extern int hbhopt_print(const u_char *);
extern int dstopt_print(const u_char *);
extern int frag6_print(const u_char *, const u_char *);
extern void icmp6_print(const u_char *, const u_char *);
extern void ripng_print(const u_char *, int);
extern int rt6_print(const u_char *, const u_char *);
extern void ospf6_print(const u_char *, u_int);
extern void dhcp6_print(const u_char *, u_int, u_int16_t, u_int16_t);
#endif /*INET6*/
extern u_short in_cksum(const u_short *addr, register int len, u_short csum);
