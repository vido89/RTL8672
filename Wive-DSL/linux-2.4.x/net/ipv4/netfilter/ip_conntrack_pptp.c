/* PPTP extension for IP connection tracking. 
 * Brian Kuschak <bkuschak@yahoo.com> with some help from
 * Galen Hazelwood <galenh@esoft.com>.
 *
 * Adapted from John Hardin's <jhardin@impsec.org> 2.2.x 
 * PPTP Masquerade patch.
 *
 * Masquerading for PPTP (Point to Point Tunneling Protocol).
 * PPTP is a a protocol for creating virtual private networks.
 * It is a specification defined by Microsoft and some vendors
 * working with Microsoft.  PPTP is built on top of a modified
 * version of the Internet Generic Routing Encapsulation Protocol.
 * GRE is defined in RFC 1701 and RFC 1702.  Documentation of
 * PPTP can be found on the Microsoft web site.
 *
 * Copyright (c) 1997-1998 Gordon Chaffee
 */

#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/ip.h>
#include <net/checksum.h>
#include <net/tcp.h>
#include <linux/skbuff.h>
#include <linux/proc_fs.h>

#include <linux/netfilter_ipv4/lockhelp.h>
#include <linux/netfilter_ipv4/ip_conntrack_helper.h>
#include <linux/netfilter_ipv4/ip_conntrack_pptp.h>
#include <linux/netfilter_ipv4/ip_conntrack_protocol.h>

#ifdef CONFIG_IP_NF_PPTP_DEBUG
#define DEBUGP printk
#define PRINTK_GRE_HDR printk_gre_hdr
#define PRINTK_PPTP_HDR printk_pptp_hdr
#define DEBUG_DUMP_TUPLE DUMP_TUPLE
#else
#define DEBUGP(format, args...)
#define PRINTK_GRE_HDR(from, iph, greh)
#define PRINTK_PPTP_HDR(from, iph, pptph)
#define DEBUG_DUMP_TUPLE(t)
#endif

DECLARE_LOCK(ip_pptp_lock);
struct module *ip_conntrack_pptp = THIS_MODULE;

LIST_HEAD(gre_list);

struct tuple_table {
	struct list_head		list;
	struct ip_conntrack_tuple	tuple;
	struct ip_conntrack		*master;
};

#ifdef CONFIG_IP_NF_PPTP_DEBUG
/* PptpControlMessageType names */
static const char *strMName[] = {
	"UNKNOWN_MESSAGE",
	"START_SESSION_REQUEST",
	"START_SESSION_REPLY",
	"STOP_SESSION_REQUEST",
	"STOP_SESSION_REPLY",
	"ECHO_REQUEST",
	"ECHO_REPLY",
	"OUT_CALL_REQUEST",
	"OUT_CALL_REPLY",
	"IN_CALL_REQUEST",
	"IN_CALL_REPLY",
	"IN_CALL_CONNECTED",
	"CALL_CLEAR_REQUEST",
	"CALL_DISCONNECT_NOTIFY",
	"WAN_ERROR_NOTIFY",
	"SET_LINK_INFO"
};

static void
printk_pptp_hdr(char *from_txt, const struct iphdr *iph, const struct pptp_pkt_hdr *pptph)
{
	struct PptpControlHeader	*ctlh;
	__u16				msg;
        union {
                void				*rawreq;
                struct PptpOutCallRequest       *ocreq;
                struct PptpOutCallReply         *ocack;
                struct PptpInCallRequest        *icreq;
                struct PptpInCallReply          *icack;
		struct PptpClearCallRequest	*clrreq;
                struct PptpCallDisconnectNotify *disc;
                struct PptpWanErrorNotify       *wanerr;
                struct PptpSetLinkInfo          *setlink;
        } pptpReq;

	printk("%s", from_txt);
        printk("%d.%d.%d.%d -> ", NIPQUAD(iph->saddr));
        printk("%d.%d.%d.%d ", NIPQUAD(iph->daddr));
	printk("LEN=%d TY=%d MC=%X", ntohs(pptph->packetLength),
		ntohs(pptph->packetType), ntohl(pptph->magicCookie));

	if (ntohs(pptph->packetType) == PPTP_CONTROL_PACKET) {
		ctlh = (struct PptpControlHeader *) ((char *)pptph + sizeof(struct pptp_pkt_hdr));
		pptpReq.rawreq = (void *) ((char*) ctlh + sizeof(struct PptpControlHeader));

		/* todo call id */
		msg = htons(ctlh->messageType);
		switch(msg)
		{
			case PPTP_OUT_CALL_REQUEST:
				printk(" CID=%d", ntohs(pptpReq.ocreq->callID));
				break;
			case PPTP_IN_CALL_REQUEST:
				printk(" CID=%d", ntohs(pptpReq.icreq->callID));
				break;	
			case PPTP_OUT_CALL_REPLY:
				printk(" CID=%d PCID=%d", ntohs(pptpReq.ocack->callID),
					ntohs(pptpReq.ocack->peersCallID));
				break;
			case PPTP_WAN_ERROR_NOTIFY:
				printk(" PCID=%d", ntohs(pptpReq.wanerr->peersCallID));
				break;
	
			case PPTP_SET_LINK_INFO:
				printk(" PCID=%d", ntohs(pptpReq.setlink->peersCallID));
				break;
	
			case PPTP_CALL_DISCONNECT_NOTIFY:
				printk(" CID=%d", ntohs(pptpReq.disc->callID));
		}
		printk(" CTL=%s", (msg <= PPTP_MSG_MAX)? strMName[msg]:strMName[0]);
	}

	printk("\n");
}

void
printk_gre_hdr(char *from_txt, const struct iphdr *iph, const struct pptp_gre_hdr *greh)
{

	printk("%s GRE: ", from_txt);
        printk("%d.%d.%d.%d -> ", NIPQUAD(iph->saddr));
        printk("%d.%d.%d.%d ", NIPQUAD(iph->daddr));
	printk("PR=%X LEN=%d CID=%d", ntohs(greh->protocol),
		ntohs(greh->payload_len), ntohs(greh->call_id));

	printk("\n");
}
#endif


/*
 *	Store this tuple so we can lookup the peer call id later.
 */

static struct ip_conntrack_tuple *
put_gre_tuple(	__u32 s_addr, __u32 d_addr, __u16 call_id, __u16 peer_call_id, 
		struct ip_conntrack *master)
{
	struct list_head	*l;
        struct tuple_table	*tt;

	if((tt = kmalloc(sizeof(struct tuple_table), GFP_ATOMIC)) == NULL)
	{
		DEBUGP("put_gre_tuple: out of memory\n");
		return NULL;
	}
	tt->tuple.src.ip = s_addr;
	tt->tuple.dst.ip = d_addr;
	tt->tuple.src.u.gre.call_id = call_id;
	tt->tuple.dst.u.gre.peer_call_id = peer_call_id;
	tt->tuple.dst.protonum = IPPROTO_GRE;
	INIT_LIST_HEAD(&tt->list);
	tt->master = master;

        //hash = hash_key(IPPROTO_GRE, d_addr, call_id);
	//l = &gre_table[hash];
	l = &gre_list;
	list_add(&tt->list, l);

	DEBUGP("put_gre_tuple(): Master=%p ", master);
	DEBUG_DUMP_TUPLE(&(tt->tuple));
        return (&tt->tuple);
}

/*
 *	Hunt the list to see if we have an entry for this tuple
 */

static struct ip_conntrack_tuple *
get_gre_tuple(	__u32 s_addr, __u32 d_addr, __u16 call_id, 
		struct ip_conntrack **master)
{
	struct list_head		*l, *e;
        struct tuple_table		*tt;
	struct ip_conntrack_tuple	*t;

        //hash = hash_key(IPPROTO_GRE, d_addr, call_id);
	//l = &gre_table[hash];
	l = &gre_list;
	for (e=l->next; e!=l; e=e->next) {
		tt = list_entry(e, struct tuple_table, list);
		t = &tt->tuple;
		
                if (t->src.ip == s_addr &&
                    t->dst.ip == d_addr &&
                    t->src.u.gre.call_id == call_id) {
		    	if(master)
		    		*master = tt->master;
			return t;
		}
        }

	DEBUGP("get_gre_tuple(): FAILED to lookup tuple: ");
        DEBUGP("%d.%d.%d.%d -> ", NIPQUAD(s_addr));
        DEBUGP("%d.%d.%d.%d ", NIPQUAD(d_addr));
        DEBUGP("CID=%d\n", call_id);
        return NULL;
}

/*
 *	Remove the selected tuple from the list
 */

static void
clear_gre_tuples(struct ip_conntrack *master)
{
	struct list_head		*l, *e;
        struct tuple_table		*tt;
	struct ip_conntrack_tuple	*t;
	int				found = 0;

        //hash = hash_key(IPPROTO_GRE, d_addr, call_id);
	//l = &gre_table[hash];
	l = &gre_list;
	for (e=l->next; e!=l; e=e->next) {
		tt = list_entry(e, struct tuple_table, list);
		t = &tt->tuple;
		
		if(tt->master == master) {
			DEBUGP("clear_gre_tuple(): master=%p ", master);
			DEBUG_DUMP_TUPLE(&(tt->tuple));
			list_del(e);
			kfree(tt);
			found = 1;
		}
        }
	if(!found)
	{
		DEBUGP("clear_gre_tuple(): FAILED to delete tuple: ");
		DEBUGP("master = %p\n", master);
	}
}


static int gre_pkt_to_tuple(const void *datah, size_t datalen,
			    struct ip_conntrack_tuple *tuple)
{
	const struct pptp_gre_hdr *hdr = datah;

	/* Forward direction is easy */
	tuple->src.u.gre.call_id = hdr->call_id;
	tuple->dst.u.all = 0;
	return 1;
}

static int gre_invert_tuple(struct ip_conntrack_tuple *tuple,
			    const struct ip_conntrack_tuple *orig)
{
	struct ip_conntrack_tuple *t;

	/* A response is harder to figure, lookup in list */
	if((t = get_gre_tuple(orig->src.ip, orig->dst.ip, orig->src.u.gre.call_id, NULL)))
	{
		tuple->src.u.gre.call_id = t->dst.u.gre.peer_call_id;
		tuple->dst.u.all = 0;
		return 1;
	}
	DEBUGP("Couldn't find reponse to ");
	DEBUG_DUMP_TUPLE(orig);
	return -1;
}

/* Print out the per-protocol part of the tuple. */
static unsigned int gre_print_tuple(char *buffer,
				    const struct ip_conntrack_tuple *tuple)
{
	return sprintf(buffer, "call_id=%hu ",
		       ntohs(tuple->src.u.gre.call_id));
}

/* Print out the private part of the conntrack. */
static unsigned int gre_print_conntrack(char *buffer,
					const struct ip_conntrack *conntrack)
{
	return 0;
}

/* Returns verdict for packet, or -1 for invalid. */
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
static int gre_packet(struct ip_conntrack *conntrack,
		      struct iphdr *iph, size_t len,
		      enum ip_conntrack_info ctinfo)
{
#ifdef CONFIG_IP_NF_PPTP_DEBUG
	struct pptp_gre_hdr *greh = (struct pptp_gre_hdr *)((u_int32_t *)iph + iph->ihl);
#endif

	/* 
	 *	If we've seen traffic both ways, this is a connected GRE stream.
	 * 	Extend timeout. 
	 */
	if (conntrack->status & IPS_SEEN_REPLY) {
		ip_ct_refresh(conntrack, GRE_CONNECTED_TIMEOUT);
		/* Also, more likely to be important, and not a probe */
		set_bit(IPS_ASSURED_BIT, &conntrack->status);
	} else
		ip_ct_refresh(conntrack, GRE_TIMEOUT);

	DEBUGP("CT=%lx, Master=%lx, DIR=%s ", (unsigned long) conntrack, 
		(unsigned long) conntrack->help.ct_pptp_info.master,
		(ctinfo >= IP_CT_IS_REPLY ? "reply   " : "original"));
	PRINTK_GRE_HDR("", iph, greh);
	return NF_ACCEPT;
}

/* Called when a new connection for this protocol found. */
static int gre_new(struct ip_conntrack *conntrack,
			     struct iphdr *iph, size_t len)
{
	struct pptp_gre_hdr *greh = (struct pptp_gre_hdr *)((u_int32_t *)iph + iph->ihl);
	struct ip_conntrack *master;
	struct ip_conntrack_tuple *t;

	/* 
	 *	we only get here if we added a inverse tuple for this packet, meaning
	 *	we expected it.  set the master of this connection 
	 */
	if(!(t = get_gre_tuple(iph->saddr, iph->daddr, greh->call_id, &master)))
	{
		DEBUGP("gre_new: Unexpected - don't have a tuple for this packet!\n");
		return 0;
	}

	/* 	 link to our master, add ourself to master's child list. 
	 */
	conntrack->help.ct_pptp_info.master = master;
	INIT_LIST_HEAD(&conntrack->help.ct_pptp_info.list);
	list_add(&conntrack->help.ct_pptp_info.list, &master->help.ct_pptp_info.list);

	/*
	 * 	normally we see first packet from masqed client to server.  if the
	 *	server sends first, we need to adjust the expected response.
	 */
	if(iph->saddr != master->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip)
	{
		conntrack->nat.help.pptp_info.serv_to_client = 1;

		/* original src = pptp serv
		 * original dst = gateway
		 * reply src = client
		 * reply dst = pptp serv
		 */
		conntrack->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip =
			master->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip;
	}
	//nf_conntrack_get(&master->master);

	DEBUGP("CT=%lx, Master=%lx, DIR=new ", (unsigned long) conntrack, (unsigned long) master);
	PRINTK_GRE_HDR("", iph, greh);

	return (GRE_TIMEOUT);
}

struct ip_conntrack_protocol ip_conntrack_protocol_gre =
{ { NULL, NULL }, IPPROTO_GRE, "gre",
    gre_pkt_to_tuple, gre_invert_tuple, gre_print_tuple, gre_print_conntrack,
    gre_packet, gre_new, NULL };

/* 
 *	look for inbound control packets from server through masq gateway to masqed client
 */

static int ip_inbound_pptp_tcp(const struct iphdr *iph, struct ip_conntrack *ct,
				enum ip_conntrack_info ctinfo)
{
	struct PptpControlHeader	*ctlh;
        union {
                void				*rawreq;
                struct PptpOutCallRequest       *ocreq;
                struct PptpOutCallReply         *ocack;
                struct PptpInCallRequest        *icreq;
                struct PptpInCallReply          *icack;
		struct PptpClearCallRequest	*clrreq;
                struct PptpCallDisconnectNotify *disc;
                struct PptpWanErrorNotify       *wanerr;
                struct PptpSetLinkInfo          *setlink;
        } pptpReq;
	__u16				msg, *cid, *pcid;
	struct tcphdr 		*tcph = (void *)iph + iph->ihl * 4;
	const char 		*data = (const char *)tcph + tcph->doff * 4;
	struct pptp_pkt_hdr 	*pptph = (struct pptp_pkt_hdr *) data;
	int dir = CTINFO2DIR(ctinfo);

	DEBUGP("inbound_pptp_tcp(): CT=%lx, ", (unsigned long) ct);
	PRINTK_PPTP_HDR("", iph, pptph);

	ctlh = (struct PptpControlHeader *) ((char *) pptph + sizeof(struct pptp_pkt_hdr));
	pptpReq.rawreq = (void *) ((char*) ctlh + sizeof(struct PptpControlHeader));
	switch (msg = htons(ctlh->messageType)) {
		case PPTP_OUT_CALL_REPLY:
			/* server responding to masq'd client */
			cid = &pptpReq.ocack->callID;
			pcid = &pptpReq.ocack->peersCallID;
			break;

		case PPTP_IN_CALL_REPLY:
			/* server responding to masq'd client */
			cid = &pptpReq.icack->callID;
			pcid = &pptpReq.icack->peersCallID;
			break;

		case PPTP_WAN_ERROR_NOTIFY:
			/* server notifying masq'd client */
			/* no need to alter conntrack */
			return 0;

		case PPTP_SET_LINK_INFO:
			/* server notifying masq'd client */
			/* no need to alter conntrack */
			return 0;

		case PPTP_CALL_DISCONNECT_NOTIFY:
			/* server notifying masq'd client */
			/* expire this connection */
			ip_ct_refresh(ct, (30*HZ));
			clear_gre_tuples(ct);
			return 0;

		default:
			DEBUGP("UNKNOWN inbound packet: ");
			DEBUGP("%s (TY=%d)\n", (msg <= PPTP_MSG_MAX)? strMName[msg] : strMName[0], msg);
			/* fall through */

		case PPTP_ECHO_REPLY:
		case PPTP_START_SESSION_REQUEST:
		case PPTP_START_SESSION_REPLY:
		case PPTP_STOP_SESSION_REQUEST:
		case PPTP_STOP_SESSION_REPLY:
		case PPTP_ECHO_REQUEST:
			/* no need to alter conntrack */
			return 0;
	}

	LOCK_BH(&ip_pptp_lock);

	/* info for conntrack/NAT */
	ct->help.ct_pptp_info.pptp_magic = PPTP_TCP_PORT;		/* our magic number */
	ct->help.ct_pptp_info.orig_call_id = *cid;
	ct->help.ct_pptp_info.peer_call_id = *pcid;
	INIT_LIST_HEAD(&ct->help.ct_pptp_info.list);

	/* tuple for GRE packets (from server to masqed client)
	 * Here src = pptp server, dst = ppp addr 
	 * !dir: src = masq client, dst = pptp server 
	 */

	/*	
	 *	masq client <--> pptp serv 
	 *	new connection replaces any old ones.
	 */
	
	/*
	 * 	populate our lists for peer call ID lookup
	 */
	put_gre_tuple(ct->tuplehash[!dir].tuple.src.ip, ct->tuplehash[!dir].tuple.dst.ip, *cid, *pcid, ct);
	put_gre_tuple(ct->tuplehash[!dir].tuple.dst.ip, ct->tuplehash[!dir].tuple.src.ip, *pcid, *cid, ct);
	put_gre_tuple(ct->tuplehash[dir].tuple.src.ip, ct->tuplehash[dir].tuple.dst.ip, *pcid, *cid, ct);
	put_gre_tuple(ct->tuplehash[dir].tuple.dst.ip, ct->tuplehash[dir].tuple.src.ip, *cid, *pcid, ct);

	if(ip_conntrack_protocol_register(&ip_conntrack_protocol_gre) == 0)
		DEBUGP("pptp: registered conntrack protocol GRE!\n");
	else
		DEBUGP("pptp: failed to register conntrack protocol GRE... (maybe it's already registered?)\n");

	UNLOCK_BH(&ip_pptp_lock);

	return 0;
}

/* 
 * 	Look for outbound control packets from masqed client through masq gateway to server
 */

static int ip_outbound_pptp_tcp(const struct iphdr *iph, struct ip_conntrack *ct,
				enum ip_conntrack_info ctinfo)
{
	struct PptpControlHeader	*ctlh;
        union {
                void				*rawreq;
                struct PptpOutCallRequest       *ocreq;
                struct PptpOutCallReply         *ocack;
                struct PptpInCallRequest        *icreq;
                struct PptpInCallReply          *icack;
		struct PptpClearCallRequest	*clrreq;
                struct PptpCallDisconnectNotify *disc;
                struct PptpWanErrorNotify       *wanerr;
                struct PptpSetLinkInfo          *setlink;
        } pptpReq;
	__u16				msg, *cid;
	struct tcphdr 		*tcph = (void *)iph + iph->ihl * 4;
	const char 		*data = (const char *)tcph + tcph->doff * 4;
	struct pptp_pkt_hdr 	*pptph = (struct pptp_pkt_hdr *) data;

	DEBUGP("outbound_pptp_tcp(): CT=%lx, ", (unsigned long) ct);
	PRINTK_PPTP_HDR("", iph, pptph);

	ctlh = (struct PptpControlHeader *) ((char *) pptph + sizeof(struct pptp_pkt_hdr));
	pptpReq.rawreq = (void *) ((char*) ctlh + sizeof(struct PptpControlHeader));
	switch (msg = htons(ctlh->messageType)) {
		case PPTP_OUT_CALL_REQUEST:
			/* masq'd client initiating connection to server */
			cid = &pptpReq.ocreq->callID;
			break;		/* create conntrack and get CID */

		case PPTP_IN_CALL_REQUEST:
			/* masq'd client initiating connection to server */
			cid = &pptpReq.icreq->callID;
			break;		/* create conntrack and get CID */

		case PPTP_CALL_CLEAR_REQUEST:
			/* masq'd client sending to server */
			/* no need to alter conntrack */
			return 0;

		case PPTP_CALL_DISCONNECT_NOTIFY:
			/* masq'd client notifying server */
			/* expire this connection */
			ip_ct_refresh(ct, (30*HZ));
			clear_gre_tuples(ct);
			return 0;

		default:
			DEBUGP("UNKNOWN outbound packet: ");
			DEBUGP("%s (TY=%d)\n", (msg <= PPTP_MSG_MAX)? strMName[msg]:strMName[0], msg);
			/* fall through */

		case PPTP_SET_LINK_INFO:
		case PPTP_START_SESSION_REQUEST:
		case PPTP_START_SESSION_REPLY:
		case PPTP_STOP_SESSION_REQUEST:
		case PPTP_STOP_SESSION_REPLY:
		case PPTP_ECHO_REPLY:
		case PPTP_ECHO_REQUEST:
			/* no need to alter conntrack */
			return 0;
	}


	/* Info for NAT */
	DEBUGP("ip_outbound_pptp_tcp: original client call id: %d\n", *cid);
	ct->nat.help.pptp_info.call_id = *cid;

	DEBUGP("ip_outbound_pptp_tcp(): ");
	DEBUGP("%s, CT=%lx, CID=%d\n", strMName[msg], (unsigned long) ct,
		(cid ? ntohs(*cid) : 0));

	return NF_ACCEPT;
}

static void delete_connection(struct ip_conntrack *ct, enum ip_conntrack_info ctinfo)
{
	clear_gre_tuples(ct);

	ip_ct_refresh(ct, 5*HZ);
	//nf_conntrack_put(&ct->master);

#if 0
	/* 	expire the GRE connection
	 */
	l = &ct->help.ct_pptp_info.list;
	for (e=l->next; e!=l; e=e->next) {
		ct_gre = list_entry(e, struct ip_conntrack, help.ct_pptp_info.list);
		if(!ct_gre)
		{
			DEBUGP("What - NULL ct_gre!\n");
			continue;
		}
		list_del(&ct_gre->help.ct_pptp_info.list);

		ip_ct_refresh(ct_gre, 5*HZ);
		if (del_timer(&ct_gre->timeout))
	        	ct_gre->timeout.function((unsigned long)ct_gre);
		else
			DEBUGP("Couldn't delete GRE timer!\n");
	}
#endif	
}


/* FIXME: This should be in userspace.  Later. */
static int help(const struct iphdr *iph, size_t len,
		struct ip_conntrack *ct,
		enum ip_conntrack_info ctinfo)
{
	/* tcplen not negative guaranteed by ip_conntrack_tcp.c */
	struct tcphdr *tcph = (void *)iph + iph->ihl * 4;
	unsigned int tcplen = len - iph->ihl * 4;
//	int dir = CTINFO2DIR(ctinfo);
	const char *data = (const char *)tcph + tcph->doff * 4;
	struct pptp_pkt_hdr *pptph = (struct pptp_pkt_hdr *) data;


	/* Until there's been traffic both ways, don't look in packets. */
	if (ctinfo != IP_CT_ESTABLISHED
	    && ctinfo != IP_CT_ESTABLISHED+IP_CT_IS_REPLY) {
		DEBUGP("pptp: Conntrackinfo = %u\n", ctinfo);
		return NF_ACCEPT;
	}

	/* If we get a FIN or RST, this connection's going down, and so is */
	/* the GRE tunnel. Deal. */
	if (tcph->rst || tcph->fin) {
		DEBUGP("pptp: bringing down gre connection.\n");
		delete_connection(ct, ctinfo);
	}
	
	/* Not whole TCP header? */
	if (tcplen < sizeof(struct tcphdr) || tcplen < tcph->doff*4) {
		DEBUGP("pptp: tcplen = %u\n", (unsigned)tcplen);
		return NF_ACCEPT;
	}

	/* Checksum invalid?  Ignore. */
	/* FIXME: Source route IP option packets --RR */
	if (tcp_v4_check(tcph, tcplen, iph->saddr, iph->daddr,
			 csum_partial((char *)tcph, tcplen, 0))) {
		DEBUGP("pptp_help: bad csum: %p %u %u.%u.%u.%u %u.%u.%u.%u\n",
		       tcph, tcplen, NIPQUAD(iph->saddr),
		       NIPQUAD(iph->daddr));

		/* WHAT?!?  Win2k seems to send OUT_CALL_REQ packets with bogus checksums...
	 	 */
		/* return NF_ACCEPT; */
	}

	/* verify we have data (i.e. pptph points before end of packet) */
	if ((void *) pptph >= (void *) ((iph) + len)) {
		DEBUGP("pptp_help(): no TCP data in pkt\n");
		return NF_ACCEPT;
	}

	/* if it's not a control message we can't do anything with it */
        if (ntohs(pptph->packetType) != PPTP_CONTROL_PACKET ||
	    ntohl(pptph->magicCookie) != PPTP_MAGIC_COOKIE) {
		DEBUGP("pptp_help(): not a control pkt: type=0x%x, cookie=0x%lx\n",
			ntohs(pptph->packetType), ntohl(pptph->magicCookie));
		return NF_ACCEPT;
	}

	if(ctinfo >= IP_CT_IS_REPLY)
		ip_inbound_pptp_tcp(iph, ct, ctinfo);
	else
		ip_outbound_pptp_tcp(iph, ct, ctinfo);

	return NF_ACCEPT;
}

static struct ip_conntrack_helper pptp_out = 
// Commented by Mason Yu
#if 0
                   	{ { NULL, NULL },
						{ { 0, { .tcp ={ __constant_htons(PPTP_TCP_PORT)} } },
						{ 0, { 0 }, IPPROTO_TCP } },
						{ { 0, { .tcp = {0xFFFF }} },
						{ 0, { 0 }, 0xFFFF } },
						help };
#endif
                     { { NULL, NULL },
	                   "pptp",					/* name */
	                   IP_CT_HELPER_F_REUSE_EXPECT,			/* flags */
	                   THIS_MODULE,					/* module */
	                   2,						/* max_expected */
	                   240,						/* timeout */
	                   { { 0, { __constant_htons(PPTP_TCP_PORT) } },	/* tuple */
	                     { 0, { 0 }, IPPROTO_TCP } },
	                   { { 0, { 0xFFFF } },				/* mask */
	                     { 0, { 0 }, 0xFFFF } },
	                   help					/* helper */
	                 };
/* Some stuff for the /proc filesystem.
 */
struct proc_dir_entry *pptp_proc_entry = NULL;

static int pptp_proc_read(char *page, char **start, off_t off,
			       int count, int *eof, void *data)
{
	char *out = page;
	struct list_head	*l, *e;
        struct tuple_table	*tt;
        struct ip_conntrack_tuple *t;
	int len;

	out += sprintf (out, "GRE tuple list\n");

	l = &gre_list;
	for (e=l->next; e!=l; e=e->next) 
	{
		tt = list_entry(e, struct tuple_table, list);
		t = &tt->tuple;
		
		out += sprintf(out, "tuple %p: %u %u.%u.%u.%u CID:%hu -> %u.%u.%u.%u.  PCID=%hu, Master=%p\n",
				t, t->dst.protonum, 
				NIPQUAD(t->src.ip), ntohs(t->src.u.all), 
				NIPQUAD(t->dst.ip), ntohs(t->dst.u.gre.peer_call_id),
				tt->master);
        }

	len = out - page;
	len -= off;
	if (len < count) {
		*eof = 1;
		if (len <= 0)
			return 0;
	} else
		len = count;

	*start = page + off;

	return len;
}

static void pptp_proc_create( void )
{
	pptp_proc_entry = create_proc_entry("pptp", S_IFREG|S_IRUGO|S_IWUSR, &proc_root);
	if (pptp_proc_entry == NULL) {
		printk("pptp.c: unable to initialise /proc/pptp\n");
		return;
	}
	pptp_proc_entry->data = (void*)NULL;
	pptp_proc_entry->read_proc = pptp_proc_read;
}
static void pptp_algfini();
#ifdef CONFIG_IP_NF_ALG_ONOFF
static unsigned char ginitflag=0;
unsigned char pptp_algonoff= 0;
#define PROCFS_NAME "pptp_algonoff"
static struct proc_dir_entry* FP_Proc_pptp_algonoff;
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
int pptp_proc_debug_write(struct file *file, const char *buffer,
                      unsigned long count, void *data)
{
         char proc_buffer[count];    
    /* write data to the buffer */
    memset(proc_buffer, 0, sizeof(proc_buffer));
    if ( copy_from_user(proc_buffer, buffer, count) ) {
        return -EFAULT;
    }

    if(proc_buffer[0]=='1'){
	if(!pptp_algonoff)
         {
         DEBUGP("*****Enable pptp ALG function!*****\n");
	pptp_alginit();
	pptp_algonoff=1;
	}
     
    }
    else if(proc_buffer[0]=='0'){
	if(pptp_algonoff)
        {
        DEBUGP("*****Disable pptp ALG function!*****\n");
       	pptp_algfini();
	pptp_algonoff=0;
	}
    }
    else
        printk("Error setting!\n");
    return -1;
}
int pptp_proc_debug_read(struct file *file, const char *buffer,
                      unsigned long count, void *data)
{
         if(pptp_algonoff==1)
        printk("pptp ALG ON!\n");
    if(pptp_algonoff==0)
        printk("pptp ALG OFF!\n");
    return -1;
}
int pptp_Alg_OnOff_init()
{

	FP_Proc_pptp_algonoff= create_proc_entry(PROCFS_NAME, 0644, &proc_root);
    if (FP_Proc_pptp_algonoff == NULL) {
        remove_proc_entry(PROCFS_NAME, &proc_root);
        printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
            PROCFS_NAME);
        return -ENOMEM;
    }
    FP_Proc_pptp_algonoff->read_proc  = (read_proc_t *)pptp_proc_debug_read;
    FP_Proc_pptp_algonoff->write_proc  = (write_proc_t *)pptp_proc_debug_write;
    //FP_Proc_Debug->owner       = THIS_MODULE;
    FP_Proc_pptp_algonoff->mode       = S_IFREG | S_IRUGO;
    FP_Proc_pptp_algonoff->uid       = 0;
    FP_Proc_pptp_algonoff->gid       = 0;
    FP_Proc_pptp_algonoff->size       = 37;  
 DEBUGP("/proc/%s created\n", PROCFS_NAME);
   
   ginitflag=1;
    return 0;

}
#endif
static int __init init(void)
{
	#ifdef CONFIG_IP_NF_ALG_ONOFF	
	pptp_Alg_OnOff_init();	
	#endif
	return pptp_alginit();
}

static void __exit fini(void)
{
	pptp_algfini();
}
static int pptp_alginit()
{
	int err;
	printk("PPTP netfilter connection tracking: ");
	if((err = ip_conntrack_helper_register(&pptp_out)))
		printk("register failed!\n");
	else
		printk("registered\n");
	pptp_proc_create();
	#ifdef CONFIG_IP_NF_ALG_ONOFF
	DEBUGP("load pptp ALG mould sucessfully!\n");
	pptp_algonoff=1;	
	#endif
	return err;
}
static void pptp_algfini()
{
	struct list_head		*l, *e;
	struct tuple_table		*tt;

	remove_proc_entry("pptp", &proc_root);

	LOCK_BH(&ip_pptp_lock);
	l = &gre_list;
	for (e=l->next; e!=l; e=e->next) {
		tt = list_entry(e, struct tuple_table, list);
		
		DEBUGP("clear_gre_tuple(): master=%p ", tt->master);
		DEBUG_DUMP_TUPLE(&(tt->tuple));
		list_del(e);
		kfree(tt);
        }
	UNLOCK_BH(&ip_pptp_lock);

	ip_conntrack_helper_unregister(&pptp_out);
	#ifdef CONFIG_IP_NF_ALG_ONOFF
	pptp_algonoff=0;
	#endif
}
EXPORT_SYMBOL(ip_pptp_lock);
EXPORT_SYMBOL(ip_conntrack_pptp);

module_init(init);
module_exit(fini);
