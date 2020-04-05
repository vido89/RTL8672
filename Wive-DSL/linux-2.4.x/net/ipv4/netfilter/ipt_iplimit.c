/*
 * netfilter module to limit the number of parallel tcp
 * connections per IP address.
 * (c) 2000 Gerd Knorr <kraxel@bytesex.org>
 *
 * based on ...
 *
 * Kernel module to match connection tracking information.
 * GPL (C) 1999 Rusty Russell (rusty@rustcorp.com.au).
*/
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/list.h>
#include <linux/netfilter_ipv4/ip_conntrack.h>
#include <linux/netfilter_ipv4/ip_conntrack_core.h>
#include <linux/netfilter_ipv4/ip_conntrack_tcp.h>
#include <linux/netfilter_ipv4/ip_tables.h>

#define DEBUG 0

/* we'll save the tuples of all connections we care about */
struct ipt_iplimit_conn
{
 struct list_head list;
 struct ip_conntrack_tuple tuple;
};

static struct list_head ip_hash[256];

static int ipt_iphash(__u32 addr)
{
 int hash;

 hash = addr & 0xff;
 hash ^= (addr >> 8) & 0xff;
 hash ^= (addr >> 16) & 0xff;
 hash ^= (addr >> 24) & 0xff;
 return hash;
}

static int count_them(__u32 addr, struct ip_conntrack *ct)
{
	int hash = ipt_iphash(addr);
	int addit = 1, matches = 0;
	enum ip_conntrack_dir direction;
	struct ip_conntrack_tuple tuple;
	struct ip_conntrack_tuple_hash *found;
	struct ipt_iplimit_conn *conn;
	struct list_head *lh;

	/* pick the tuple we'll look at */
	for (direction = 0; direction < IP_CT_DIR_MAX; direction++) {
	if (ct->tuplehash[direction].tuple.src.ip == addr) {
		tuple = ct->tuplehash[direction].tuple;
		break;
	}
	}
	if (IP_CT_DIR_MAX == direction) {
		printk("ipt_iplimit: Huh? neither direction matches?\n");
		return 0;
	}

	/* check the saved connections */
	for (lh = ip_hash[hash].next; lh != &ip_hash[hash]; lh = lh->next) {
		conn = list_entry(lh,struct ipt_iplimit_conn,list);
		if (0 == memcmp(&conn->tuple,&tuple,sizeof(tuple))) {
			/* Just to be sure we have it only once in the list.
			We should'nt see tuples twice unless someone hooks this
			into a table without "-p tcp --syn" */
			addit = 0;
		}
		found = ip_conntrack_find_get(&conn->tuple,ct);
#if DEBUG
		printk("ipt_iplimit [%d]: src=%u.%u.%u.%u:%d dst=%u.%u.%u.%u:%d %s%s\n",hash,
			NIPQUAD(conn->tuple.src.ip), ntohs(conn->tuple.src.u.tcp.port),
			NIPQUAD(conn->tuple.dst.ip), ntohs(conn->tuple.dst.u.tcp.port),
			(NULL != found) ? "ok" : "gone",
			((NULL != found) && (found->ctrack->proto.tcp.state == 
			TCP_CONNTRACK_TIME_WAIT)) ? " (wait)" : "");
#endif
		if (NULL == found) {
			/* this one is gone */
			lh = lh->prev;
			list_del(lh->next);
			kfree(conn);
			continue;
		}
		if (found->ctrack->proto.tcp.state == TCP_CONNTRACK_TIME_WAIT) {
			/* we don't care about connections which are
			closed already -> ditch it */
			lh = lh->prev;
			list_del(lh->next);
			kfree(conn);
			nf_conntrack_put(&found->ctrack->infos[0]);
			continue;
		}
		if (addr == conn->tuple.src.ip) {
			/* same source IP address -> be counted! */
			matches++;
		}
		nf_conntrack_put(&found->ctrack->infos[0]);
	}
	if (addit) {
		/* save the new connection in our list */
#if DEBUG
		printk("ipt_iplimit [%d]: src=%u.%u.%u.%u:%d dst=%u.%u.%u.%u:%d new\n",hash,
			NIPQUAD(tuple.src.ip), ntohs(tuple.src.u.tcp.port),
			NIPQUAD(tuple.dst.ip), ntohs(tuple.dst.u.tcp.port));
#endif
		conn = kmalloc(sizeof(*conn),GFP_ATOMIC);
		if (NULL == conn) {
			printk("ipt_iplimit: Hmm, kmalloc failed :-(\n");
		} else {
			memset(conn,0,sizeof(*conn));
			INIT_LIST_HEAD(&conn->list);
			conn->tuple = tuple;
			list_add(&conn->list,&ip_hash[hash]);
		}
		matches++;
	}
	return matches;
}

static int
match(const struct sk_buff *skb,
	const struct net_device *in,
	const struct net_device *out,
	const void *matchinfo,
	int offset,
	const void *hdr,
	u_int16_t datalen,
	int *hotdrop)
{
	const int *limit = matchinfo;
	int connections;
	struct ip_conntrack *ct;
	enum ip_conntrack_info ctinfo;

	ct = ip_conntrack_get((struct sk_buff *)skb, &ctinfo);
	if (NULL == ct) {
		printk("ipt_iplimit: Oops: state is invalid\n");
		return 0;
	}
	connections = count_them(skb->nh.iph->saddr,ct);
#if DEBUG
	printk("ipt_iplimit: src=%u.%u.%u.%u connections=%d limit=%d\n",
		NIPQUAD(skb->nh.iph->saddr),connections,*limit);
#endif

	return connections > *limit;
}

static int check(const char *tablename,
	const struct ipt_ip *ip,
	void *matchinfo,
	unsigned int matchsize,
	unsigned int hook_mask)
{
#if 0
	if (matchsize != IPT_ALIGN(sizeof(struct ipt_state_info)))
		return 0;
#endif
	return 1;
}

static struct ipt_match iplimit_match
	= { { NULL, NULL }, "iplimit", &match, &check, NULL, THIS_MODULE };

static int __init init(void)
{
	int i;

	/* NULL if ip_conntrack not a module */
	if (ip_conntrack_module)
		__MOD_INC_USE_COUNT(ip_conntrack_module);
	for (i = 0; i < 256; i++)
		INIT_LIST_HEAD(&ip_hash[i]);
	return ipt_register_match(&iplimit_match);
}

static void __exit fini(void)
{
	ipt_unregister_match(&iplimit_match);
	if (ip_conntrack_module)
		__MOD_DEC_USE_COUNT(ip_conntrack_module);
}

module_init(init);
module_exit(fini);

