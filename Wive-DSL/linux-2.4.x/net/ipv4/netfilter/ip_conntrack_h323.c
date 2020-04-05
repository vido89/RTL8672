/* 
 * H.323 'brute force' extension for H.323 connection tracking. 
 * Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>
 *
 * Based on ip_masq_h323.c for 2.2 kernels from CoRiTel, Sofia project.
 * (http://www.coritel.it/projects/sofia/nat/)
 * Uses Sampsa Ranta's excellent idea on using expectfn to 'bind'
 * the unregistered helpers to the conntrack entries.
 */


#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/ip.h>
#include <net/checksum.h>
#include <net/tcp.h>

#include <linux/netfilter_ipv4/lockhelp.h>
#include <linux/netfilter_ipv4/ip_conntrack.h>
#include <linux/netfilter_ipv4/ip_conntrack_core.h>
#include <linux/netfilter_ipv4/ip_conntrack_helper.h>
#include <linux/netfilter_ipv4/ip_conntrack_tuple.h>
#include <linux/netfilter_ipv4/ip_conntrack_h323.h>
#include <linux/proc_fs.h>

MODULE_AUTHOR("Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>");
MODULE_DESCRIPTION("H.323 'brute force' connection tracking module");
MODULE_LICENSE("GPL");

DECLARE_LOCK(ip_h323_lock);
struct module *ip_conntrack_h323 = THIS_MODULE;

#if 0
#define DEBUGP printk
#else
#define DEBUGP(format, args...)
#endif

/* FIXME: This should be in userspace.  Later. */
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
static int h245_help(const struct iphdr *iph, size_t len,
		     struct ip_conntrack *ct,
		     enum ip_conntrack_info ctinfo)
{
	struct tcphdr *tcph = (void *)iph + iph->ihl * 4;
	unsigned char *data = (unsigned char *) tcph + tcph->doff * 4;
	unsigned char *data_limit;
	u_int32_t tcplen = len - iph->ihl * 4;
	u_int32_t datalen = tcplen - tcph->doff * 4;
	int dir = CTINFO2DIR(ctinfo);
	struct ip_ct_h225_master *info = &ct->help.ct_h225_info;
	struct ip_conntrack_expect expect, *exp = &expect;
	struct ip_ct_h225_expect *exp_info = &exp->help.exp_h225_info;
	u_int16_t data_port;
	u_int32_t data_ip;
	unsigned int i;

	DEBUGP("ct_h245_help: help entered %u.%u.%u.%u:%u->%u.%u.%u.%u:%u\n",
		NIPQUAD(iph->saddr), ntohs(tcph->source),
		NIPQUAD(iph->daddr), ntohs(tcph->dest));

	/* Can't track connections formed before we registered */
	if (!info)
		return NF_ACCEPT;
		
	/* Until there's been traffic both ways, don't look in packets. */
	if (ctinfo != IP_CT_ESTABLISHED
	    && ctinfo != IP_CT_ESTABLISHED + IP_CT_IS_REPLY) {
		DEBUGP("ct_h245_help: Conntrackinfo = %u\n", ctinfo);
		return NF_ACCEPT;
	}

	/* Not whole TCP header or too short packet? */
	if (tcplen < sizeof(struct tcphdr) || tcplen < tcph->doff * 4 + 5) {
		DEBUGP("ct_h245_help: tcplen = %u\n", (unsigned)tcplen);
		return NF_ACCEPT;
	}

	/* Checksum invalid?  Ignore. */
	/* FIXME: Source route IP option packets --RR */
	if (tcp_v4_check(tcph, tcplen, iph->saddr, iph->daddr,
			      csum_partial((char *)tcph, tcplen, 0))) {
		DEBUGP("ct_h245_help: bad csum: %p %u %u.%u.%u.%u %u.%u.%u.%u\n",
		       tcph, tcplen, NIPQUAD(iph->saddr),
		       NIPQUAD(iph->daddr));
		return NF_ACCEPT;
	}

	data_limit = (unsigned char *) data + datalen;
	/* bytes: 0123   45
	          ipadrr port */
	for (i = 0; data < (data_limit - 5); data++, i++) {
		data_ip = *((u_int32_t *)data);
		if (data_ip == iph->saddr) {
			data_port = *((u_int16_t *)(data + 4));
			memset(&expect, 0, sizeof(expect));
			/* update the H.225 info */
			DEBUGP("ct_h245_help: new RTCP/RTP requested %u.%u.%u.%u:->%u.%u.%u.%u:%u\n",
				NIPQUAD(ct->tuplehash[!dir].tuple.src.ip),
				NIPQUAD(iph->saddr), ntohs(data_port));
			LOCK_BH(&ip_h323_lock);
			info->is_h225 = H225_PORT + 1;
			exp_info->port = data_port;
			exp_info->dir = dir;
			exp_info->offset = i;

			exp->seq = ntohl(tcph->seq) + i;
		    
			exp->tuple = ((struct ip_conntrack_tuple)
				{ { ct->tuplehash[!dir].tuple.src.ip,
				    { 0 } },
				  { data_ip,
				    { .tcp = { data_port } },
				    IPPROTO_UDP }});
			exp->mask = ((struct ip_conntrack_tuple)
				{ { 0xFFFFFFFF, { 0 } },
				  { 0xFFFFFFFF, { .tcp = { 0xFFFF } }, 0xFFFF }});
	
			exp->expectfn = NULL;
			
			/* Ignore failure; should only happen with NAT */
			ip_conntrack_expect_related(ct, exp);

			UNLOCK_BH(&ip_h323_lock);
		}
	}

	return NF_ACCEPT;

}

/* H.245 helper is not registered! */
static struct ip_conntrack_helper h245 = 
	{ { NULL, NULL },
          "H.245",				/* name */
          IP_CT_HELPER_F_REUSE_EXPECT,		/* flags */
          NULL,					/* module */
          8,					/* max_ expected */
          240,					/* timeout */
          { { 0, { 0 } },			/* tuple */
            { 0, { 0 }, IPPROTO_TCP } },
          { { 0, { 0xFFFF } },			/* mask */
            { 0, { 0 }, 0xFFFF } },
          h245_help				/* helper */
	};

static int h225_expect(struct ip_conntrack *ct)
{
	WRITE_LOCK(&ip_conntrack_lock);
	ct->helper = &h245;
	DEBUGP("h225_expect: helper for %p added\n", ct);
	WRITE_UNLOCK(&ip_conntrack_lock);
	
	return NF_ACCEPT;	/* unused */
}

/* FIXME: This should be in userspace.  Later. */
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
static int h225_help(const struct iphdr *iph, size_t len,
		     struct ip_conntrack *ct,
		     enum ip_conntrack_info ctinfo)
{
	struct tcphdr *tcph = (void *)iph + iph->ihl * 4;
	unsigned char *data = (unsigned char *) tcph + tcph->doff * 4;
	unsigned char *data_limit;
	u_int32_t tcplen = len - iph->ihl * 4;
	u_int32_t datalen = tcplen - tcph->doff * 4;
	int dir = CTINFO2DIR(ctinfo);
	struct ip_ct_h225_master *info = &ct->help.ct_h225_info;
	struct ip_conntrack_expect expect, *exp = &expect;
	struct ip_ct_h225_expect *exp_info = &exp->help.exp_h225_info;
	u_int16_t data_port;
	u_int32_t data_ip;
	unsigned int i;
	
	DEBUGP("ct_h225_help: help entered %u.%u.%u.%u:%u->%u.%u.%u.%u:%u\n",
		NIPQUAD(iph->saddr), ntohs(tcph->source),
		NIPQUAD(iph->daddr), ntohs(tcph->dest));

	/* Can't track connections formed before we registered */
	if (!info)
		return NF_ACCEPT;

	/* Until there's been traffic both ways, don't look in packets. */
	if (ctinfo != IP_CT_ESTABLISHED
	    && ctinfo != IP_CT_ESTABLISHED + IP_CT_IS_REPLY) {
		DEBUGP("ct_h225_help: Conntrackinfo = %u\n", ctinfo);
		return NF_ACCEPT;
	}

	/* Not whole TCP header or too short packet? */
	if (tcplen < sizeof(struct tcphdr) || tcplen < tcph->doff * 4 + 5) {
		DEBUGP("ct_h225_help: tcplen = %u\n", (unsigned)tcplen);
		return NF_ACCEPT;
	}

	/* Checksum invalid?  Ignore. */
	/* FIXME: Source route IP option packets --RR */
	if (tcp_v4_check(tcph, tcplen, iph->saddr, iph->daddr,
			      csum_partial((char *)tcph, tcplen, 0))) {
		DEBUGP("ct_h225_help: bad csum: %p %u %u.%u.%u.%u %u.%u.%u.%u\n",
		       tcph, tcplen, NIPQUAD(iph->saddr),
		       NIPQUAD(iph->daddr));
		return NF_ACCEPT;
	}
	
	data_limit = (unsigned char *) data + datalen;
	/* bytes: 0123   45
	          ipadrr port */
	for (i = 0; data < (data_limit - 5); data++, i++) {
		// Kaohj
		#if 1
		// Kaohj -- prevent from unaligned access
		unsigned char *tmp = (unsigned char *)&data_ip;
		tmp[0] = data[0];
		tmp[1] = data[1];
		tmp[2] = data[2];
		tmp[3] = data[3];
		#else
		data_ip = *((u_int32_t *)data);
		#endif
		if (data_ip == iph->saddr) {
			data_port = *((u_int16_t *)(data + 4));
			if (data_port == tcph->source) {
				/* Signal address */
				DEBUGP("ct_h225_help: sourceCallSignalAddress from %u.%u.%u.%u\n",
					NIPQUAD(iph->saddr));
				/* Update the H.225 info so that NAT can mangle the address/port
				   even when we have no expected connection! */
#ifdef CONFIG_IP_NF_NAT_NEEDED
				LOCK_BH(&ip_h323_lock);
				info->dir = dir;
				info->seq[IP_CT_DIR_ORIGINAL] = ntohl(tcph->seq) + i;
				info->offset[IP_CT_DIR_ORIGINAL] = i;
				UNLOCK_BH(&ip_h323_lock);
#endif
			} else {
				memset(&expect, 0, sizeof(expect));

				/* update the H.225 info */
				LOCK_BH(&ip_h323_lock);
				info->is_h225 = H225_PORT;
				exp_info->port = data_port;
				exp_info->dir = dir;
				exp_info->offset = i;

				exp->seq = ntohl(tcph->seq) + i;

				exp->tuple = ((struct ip_conntrack_tuple)
					{ { ct->tuplehash[!dir].tuple.src.ip,
					    { 0 } },
					  { data_ip,
					    { .tcp = { data_port } },
					    IPPROTO_TCP }});
				exp->mask = ((struct ip_conntrack_tuple)
					{ { 0xFFFFFFFF, { 0 } },
					  { 0xFFFFFFFF, { .tcp = { 0xFFFF } }, 0xFFFF }});
	
				exp->expectfn = h225_expect;
				
				/* Ignore failure */
				ip_conntrack_expect_related(ct, exp);

				DEBUGP("ct_h225_help: new H.245 requested %u.%u.%u.%u->%u.%u.%u.%u:%u\n",
					NIPQUAD(ct->tuplehash[!dir].tuple.src.ip),
					NIPQUAD(iph->saddr), ntohs(data_port));

				UNLOCK_BH(&ip_h323_lock);
                	}  
#ifdef CONFIG_IP_NF_NAT_NEEDED
		} else if (data_ip == iph->daddr) {
			data_port = *((u_int16_t *)(data + 4));
			if (data_port == tcph->dest) {
				/* Signal address */
				DEBUGP("ct_h225_help: destCallSignalAddress %u.%u.%u.%u\n",
					NIPQUAD(iph->daddr));
				/* Update the H.225 info so that NAT can mangle the address/port
				   even when we have no expected connection! */
				LOCK_BH(&ip_h323_lock);
				info->dir = dir;
				info->seq[IP_CT_DIR_REPLY] = ntohl(tcph->seq) + i;
				info->offset[IP_CT_DIR_REPLY] = i;
				UNLOCK_BH(&ip_h323_lock);
			}
#endif
		}
	}

	return NF_ACCEPT;

}

static struct ip_conntrack_helper h225 = 
	{ { NULL, NULL },
	  "H.225",					/* name */
	  IP_CT_HELPER_F_REUSE_EXPECT,			/* flags */
	  THIS_MODULE,					/* module */
	  2,						/* max_expected */
	  240,						/* timeout */
	  { { 0, { __constant_htons(H225_PORT) } },	/* tuple */
	    { 0, { 0 }, IPPROTO_TCP } },
	  { { 0, { 0xFFFF } },				/* mask */
	    { 0, { 0 }, 0xFFFF } },
	  h225_help					/* helper */
	};
#ifdef CONFIG_IP_NF_ALG_ONOFF
static unsigned char ginitflag=0;
unsigned char h323_algonoff= 0;
#define PROCFS_NAME "h323_algonoff"
static struct proc_dir_entry* FP_Proc_h323_algonoff;
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
int h323_proc_debug_write(struct file *file, const char *buffer,
                      unsigned long count, void *data)
{
         char proc_buffer[count];    
    /* write data to the buffer */
    memset(proc_buffer, 0, sizeof(proc_buffer));
    if ( copy_from_user(proc_buffer, buffer, count) ) {
        return -EFAULT;
    }

    if(proc_buffer[0]=='1'){
	if(!h323_algonoff)
        	{
        	ip_conntrack_helper_register(&h225);
	DEBUGP("load h323 ALG mould sucessfully!\n");
	h323_algonoff=1;
	}
    }
    else if(proc_buffer[0]=='0'){
	if(h323_algonoff)
       	{
       	ip_conntrack_helper_unregister(&h225);
	DEBUGP("unload h323 ALG mould sucessfully!\n");
	h323_algonoff=0;
	}
    }
    else
        printk("Error setting!\n");
    return -1;
}
int h323_proc_debug_read(struct file *file, const char *buffer,
                      unsigned long count, void *data)
{
         if(h323_algonoff==1)
        printk("h323 ALG ON!\n");
    if(h323_algonoff==0)
        printk("h323 ALG OFF!\n");
    return -1;
}
int  h323_Alg_OnOff_init()
{

	FP_Proc_h323_algonoff= create_proc_entry(PROCFS_NAME, 0644, &proc_root);
    if (FP_Proc_h323_algonoff == NULL) {
        remove_proc_entry(PROCFS_NAME, &proc_root);
        printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",  PROCFS_NAME);
        return -ENOMEM;
    }
    FP_Proc_h323_algonoff->read_proc  = (read_proc_t *)h323_proc_debug_read;
    FP_Proc_h323_algonoff->write_proc  = (write_proc_t *)h323_proc_debug_write;
    //FP_Proc_Debug->owner       = THIS_MODULE;
    FP_Proc_h323_algonoff->mode       = S_IFREG | S_IRUGO;
    FP_Proc_h323_algonoff->uid       = 0;
    FP_Proc_h323_algonoff->gid       = 0;
    FP_Proc_h323_algonoff->size       = 37;
    DEBUGP("/proc/%s created\n", PROCFS_NAME);
   ginitflag=1;
    return 0;

}
#endif


static int __init init(void)
{
	
	int ret= ip_conntrack_helper_register(&h225);
	#ifdef CONFIG_IP_NF_ALG_ONOFF
	DEBUGP("load h323 ALG mould sucessfully!\n");
	h323_Alg_OnOff_init();
	h323_algonoff=1;	
	#endif
	return ret;
}

static void __exit fini(void)
{
	/* Unregister H.225 helper */	
	ip_conntrack_helper_unregister(&h225);
	#ifdef CONFIG_IP_NF_ALG_ONOFF
	DEBUGP("unload h323 ALG mould sucessfully!\n");
	h323_algonoff=0;
	#endif
}

EXPORT_SYMBOL(ip_h323_lock);

module_init(init);
module_exit(fini);
