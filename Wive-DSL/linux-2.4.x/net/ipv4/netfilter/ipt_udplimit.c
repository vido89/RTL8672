/*
 * netfilter module to limit the number of parallel udp
 * connections per IP address.
 * Jan Du Caju <Jan.DuCaju@kuleuven.net>
 *
 * based on ...
 * iplimit (in fact a shameless copy ;-)
 *   (c) 2000 Gerd Knorr <kraxel@bytesex.org>
 *   Nov 2002: Martin Bene <martin.bene@icomedias.com>:
 *		only ignore TIME_WAIT or gone connections
 *
 *
 * Kernel module to match connection tracking information.
 * GPL (C) 1999  Rusty Russell (rusty@rustcorp.com.au).
 */
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/list.h>
#include <linux/netfilter_ipv4/ip_conntrack.h>
#include <linux/netfilter_ipv4/ip_conntrack_core.h>
#include <linux/netfilter_ipv4/ip_tables.h>
//#include <linux/netfilter_ipv4/ipt_iplimit.h>

#define DEBUG 0

MODULE_LICENSE("GPL");


//Import from connlimt.h and some modification
struct ipt_iplimit_data;

struct ipt_iplimit_info {
	int limit;
	int inverse;
	u_int32_t mask;
	struct ipt_iplimit_data *data;
};



/* we'll save the tuples of all connections we care about */
struct ipt_iplimit_conn
{
        struct list_head list;
	struct ip_conntrack_tuple tuple;
};

struct ipt_iplimit_data {
	spinlock_t lock;
	struct list_head iphash[256];
};

static int ipt_iphash(u_int32_t addr)
{
	int hash;

	hash  =  addr        & 0xff;
	hash ^= (addr >>  8) & 0xff;
	hash ^= (addr >> 16) & 0xff;
	hash ^= (addr >> 24) & 0xff;
	return hash;
}

static int count_them(struct ipt_iplimit_data *data,
		      u_int32_t addr, u_int32_t mask,
		      struct ip_conntrack *ct)
{
	int addit = 1, matches = 0;
	struct ip_conntrack_tuple tuple;
	struct ip_conntrack_tuple_hash *found;
	struct ipt_iplimit_conn *conn;
	struct list_head *hash,*lh;

	spin_lock(&data->lock);
	tuple = ct->tuplehash[0].tuple;
	hash = &data->iphash[ipt_iphash(addr & mask)];

	/* check the saved connections */
	for (lh = hash->next; lh != hash; lh = lh->next) {
		conn = list_entry(lh,struct ipt_iplimit_conn,list);
		found = ip_conntrack_find_get(&conn->tuple,ct);
		if (0 == memcmp(&conn->tuple,&tuple,sizeof(tuple)) &&
		    found != NULL) {
			addit = 0;
		}
#if DEBUG
		printk("ipt_udplimit [%d]: src=%u.%u.%u.%u:%d dst=%u.%u.%u.%u:%d\n",
		       ipt_iphash(addr & mask),
		       NIPQUAD(conn->tuple.src.ip), ntohs(conn->tuple.src.u.udp.port),
		       NIPQUAD(conn->tuple.dst.ip), ntohs(conn->tuple.dst.u.udp.port));
#endif
		if (NULL == found) {
			/* this one is gone */
			lh = lh->prev;
			list_del(lh->next);
			kfree(conn);
			continue;
		}
		if ((addr & mask) == (conn->tuple.src.ip & mask)) {
			/* same source IP address -> be counted! */
			matches++;
		}
		nf_conntrack_put(&found->ctrack->infos[0]);
	}
	if (addit) {
		/* save the new connection in our list */
#if DEBUG
		printk("ipt_udplimit [%d]: src=%u.%u.%u.%u:%d dst=%u.%u.%u.%u:%d new\n",
		       ipt_iphash(addr & mask),
		       NIPQUAD(tuple.src.ip), ntohs(tuple.src.u.tcp.port),
		       NIPQUAD(tuple.dst.ip), ntohs(tuple.dst.u.tcp.port));
#endif
		conn = kmalloc(sizeof(*conn),GFP_ATOMIC);
		if (NULL == conn)
			return -1;
		memset(conn,0,sizeof(*conn));
		INIT_LIST_HEAD(&conn->list);
		conn->tuple = tuple;
		list_add(&conn->list,hash);
		matches++;
	}
	spin_unlock(&data->lock);
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
	const struct ipt_iplimit_info *info = matchinfo;
	int connections, match;
	struct ip_conntrack *ct;
	enum ip_conntrack_info ctinfo;

	ct = ip_conntrack_get((struct sk_buff *)skb, &ctinfo);
	if (NULL == ct) {
		printk("ipt_udplimit: Oops: invalid ct state ?\n");
		*hotdrop = 1;
		return 0;
	}
	connections = count_them(info->data,skb->nh.iph->saddr,info->mask,ct);
	if (-1 == connections) {
		printk("ipt_udplimit: Hmm, kmalloc failed :-(\n");
		*hotdrop = 1; /* let's free some memory :-) */
		return 0;
	}
        match = (info->inverse) ? (connections <= info->limit) : (connections > info->limit);
#if DEBUG
	printk("ipt_udplimit: src=%u.%u.%u.%u mask=%u.%u.%u.%u "
	       "connections=%d limit=%d match=%s\n",
	       NIPQUAD(skb->nh.iph->saddr), NIPQUAD(info->mask),
	       connections, info->limit, match ? "yes" : "no");
#endif

	return match;
}

static int check(const char *tablename,
		 const struct ipt_ip *ip,
		 void *matchinfo,
		 unsigned int matchsize,
		 unsigned int hook_mask)
{
	struct ipt_iplimit_info *info = matchinfo;
	int i;

	/* verify size */
	if (matchsize != IPT_ALIGN(sizeof(struct ipt_iplimit_info)))
		return 0;

	/* refuse anything but udp */
	if (ip->proto != IPPROTO_UDP)
		return 0;

	/* init private data */
	info->data = kmalloc(sizeof(struct ipt_iplimit_data),GFP_KERNEL);
	spin_lock_init(&(info->data->lock));
	for (i = 0; i < 256; i++)
		INIT_LIST_HEAD(&(info->data->iphash[i]));
	
	return 1;
}

static void destroy(void *matchinfo, unsigned int matchinfosize)
{
	struct ipt_iplimit_info *info = matchinfo;
	struct ipt_iplimit_conn *conn;
	struct list_head *hash;
	int i;

	/* cleanup */
	for (i = 0; i < 256; i++) {
		hash = &(info->data->iphash[i]);
		while (hash != hash->next) {
			conn = list_entry(hash->next,struct ipt_iplimit_conn,list);
			list_del(hash->next);
			kfree(conn);
		}
	}
	kfree(info->data);
}

static struct ipt_match udplimit_match
= { { NULL, NULL }, "udplimit", &match, &check, &destroy, THIS_MODULE };

static int __init init(void)
{
	/* NULL if ip_conntrack not a module */
	if (ip_conntrack_module)
		__MOD_INC_USE_COUNT(ip_conntrack_module);
	return ipt_register_match(&udplimit_match);
}

static void __exit fini(void)
{
	ipt_unregister_match(&udplimit_match);
	if (ip_conntrack_module)
		__MOD_DEC_USE_COUNT(ip_conntrack_module);
}

module_init(init);
module_exit(fini);
