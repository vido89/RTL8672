#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/ip.h>
#include <net/checksum.h>
#include <net/tcp.h>
#include <net/udp.h>

#include <linux/netfilter_ipv4/lockhelp.h>
#include <linux/netfilter_ipv4/ip_conntrack_helper.h>
#include <linux/netfilter_ipv4/ip_conntrack_irc.h>
#include <linux/netfilter_ipv4/ip_conntrack_pptp.h>
#include <linux/netfilter_ipv4/ip_conntrack_protocol.h>
struct isakmphdr {
	__u16	source;
	__u16	dest;
	__u16	len;
	__u16	chksum;
	__u64	icookie;
	__u64	rcookie;
};


#define IPPROTO_ESP 0x32

#define MAX_PORTS 8
static int ports[MAX_PORTS];
static int ports_n_c = 0;
extern char ipsec_flag;

unsigned long ip_ct_esp_timeout = 30*HZ;
unsigned long ip_ct_esp_timeout_stream = 180*HZ;

static int esp_pkt_to_tuple(const void *datah, size_t datalen,
			    struct ip_conntrack_tuple *tuple)
{
	const struct udphdr *hdr = datah;

	tuple->src.u.all = hdr->source;
	tuple->dst.u.all = hdr->dest;

	return 1;
}

static int esp_invert_tuple(struct ip_conntrack_tuple *tuple,
			    const struct ip_conntrack_tuple *orig)
{
	tuple->src.u.all = orig->dst.u.all;
	tuple->dst.u.all = orig->src.u.all;
	return 1;
}

/* Print out the per-protocol part of the tuple. */
static unsigned int esp_print_tuple(char *buffer,
				    const struct ip_conntrack_tuple *tuple)
{
	return sprintf(buffer, "sport=%hu dport=%hu ",
		       ntohs(tuple->src.u.all),
		       ntohs(tuple->dst.u.all));
}

/* Print out the private part of the conntrack. */
static unsigned int esp_print_conntrack(char *buffer,
					const struct ip_conntrack *conntrack)
{
	return 0;
}

/* Returns verdict for packet, and may modify conntracktype */
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
static int esp_packet(struct ip_conntrack *conntrack,
		      struct iphdr *iph, size_t len,
		      enum ip_conntrack_info conntrackinfo)
{
	/* If we've seen traffic both ways, this is some kind of UDP
	   stream.  Extend timeout. */
	if (test_bit(IPS_SEEN_REPLY_BIT, &conntrack->status)) {
		ip_ct_refresh(conntrack, ip_ct_esp_timeout_stream);
		/* Also, more likely to be important, and not a probe */
		set_bit(IPS_ASSURED_BIT, &conntrack->status);
	} else
		ip_ct_refresh(conntrack, ip_ct_esp_timeout);

	return NF_ACCEPT;
}

/* Called when a new connection for this protocol found. */
static int esp_new(struct ip_conntrack *conntrack,
			     struct iphdr *iph, size_t len)
{
	return 1;
}
struct ip_conntrack_protocol ip_conntrack_protocol_esp
= { { NULL, NULL }, IPPROTO_ESP, "esp",
    esp_pkt_to_tuple, esp_invert_tuple, esp_print_tuple, esp_print_conntrack,
    esp_packet, esp_new, NULL, NULL, NULL };




/* FIXME: This should be in userspace.  Later. */
static int help(const struct iphdr *iph, size_t len,
		struct ip_conntrack *ct, enum ip_conntrack_info ctinfo)

{
	if(ipsec_flag == '1')
		/* tcplen not negative guaranteed by ip_conntrack_tcp.c */
		ip_conntrack_protocol_register(&ip_conntrack_protocol_esp);

	
	
	return NF_ACCEPT;

}

static struct ip_conntrack_helper ipsec_helpers[MAX_PORTS];

static void fini(void);
static void ipsec_alginit(void);
#ifdef CONFIG_IP_NF_ALG_ONOFF
static unsigned char ginitflag=0;
unsigned char ipsec_algonoff= 0;
#define PROCFS_NAME "ipsec_algonoff"
static struct proc_dir_entry* FP_Proc_ipsec_algonoff;
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
int ipsec_proc_debug_write(struct file *file, const char *buffer,
                      unsigned long count, void *data)
{
         char proc_buffer[count];    
    /* write data to the buffer */
    memset(proc_buffer, 0, sizeof(proc_buffer));
    if ( copy_from_user(proc_buffer, buffer, count) ) {
        return -EFAULT;
    }

    if(proc_buffer[0]=='1'){
	if(!ipsec_algonoff)
        {
     
     	ipsec_alginit();
	ipsec_algonoff=1;
	}
    }
    else if(proc_buffer[0]=='0'){
	if(ipsec_algonoff)
        {
      
       	fini();
	ipsec_algonoff=0;
	}
    }
    else
        printk("Error setting!\n");
    return -1;
}
int ipsec_proc_debug_read(struct file *file, const char *buffer,
                      unsigned long count, void *data)
{
         if(ipsec_algonoff==1)
        printk("ipsec ALG ON!\n");
    if(ipsec_algonoff==0)
        printk("ipsec ALG OFF!\n");
    return -1;
}
int  ipsec_Alg_OnOff_init()
{

	FP_Proc_ipsec_algonoff= create_proc_entry(PROCFS_NAME, 0644, &proc_root);
    if (FP_Proc_ipsec_algonoff == NULL) {
        remove_proc_entry(PROCFS_NAME, &proc_root);
        printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
            PROCFS_NAME);
        return -ENOMEM;
    }
    FP_Proc_ipsec_algonoff->read_proc  = (read_proc_t *)ipsec_proc_debug_read;
    FP_Proc_ipsec_algonoff->write_proc  = (write_proc_t *)ipsec_proc_debug_write;
    //FP_Proc_Debug->owner       = THIS_MODULE;
    FP_Proc_ipsec_algonoff->mode       = S_IFREG | S_IRUGO;
    FP_Proc_ipsec_algonoff->uid       = 0;
    FP_Proc_ipsec_algonoff->gid       = 0;
    FP_Proc_ipsec_algonoff->size       = 37;  
 
   ginitflag=1;
    return 0;

}
#endif
static void ipsec_alginit(void)
{
int i, ret;
#ifdef CONFIG_IP_NF_ALG_ONOFF
ports_n_c=0;
#endif
	/* If no port given, default to standard irc port */
	if (ports[0] == 0)
		ports[0] = 500;

	for (i = 0; (i < MAX_PORTS) && ports[i]; i++) {
		memset(&ipsec_helpers[i], 0,
		       sizeof(struct ip_conntrack_helper));
		ipsec_helpers[i].tuple.src.u.udp.port = htons(ports[i]);
		ipsec_helpers[i].tuple.dst.protonum = IPPROTO_UDP;
		ipsec_helpers[i].mask.src.u.udp.port = 0xFFFF;
		ipsec_helpers[i].mask.dst.protonum = 0xFFFF;
		ipsec_helpers[i].help = help;


		ret = ip_conntrack_helper_register(&ipsec_helpers[i]);

		if (ret) {
			printk("ip_conntrack_ipsec: ERROR registering port %d\n",
				ports[i]);
			fini();
			return -EBUSY;
		}
		ports_n_c++;
	}
	
	return 0;
}
static int __init init(void)
{
#ifdef CONFIG_IP_NF_ALG_ONOFF
	ipsec_Alg_OnOff_init();
#endif
	ipsec_alginit();
#ifdef CONFIG_IP_NF_ALG_ONOFF
	ipsec_algonoff=1;	
#endif
	return 0;
}
/* This function is intentionally _NOT_ defined as __exit, because 
 * it is needed by the init function */
static void fini(void)
{
	int i;
	for (i = 0; (i < MAX_PORTS) && ports[i]; i++) {
		ip_conntrack_helper_unregister(&ipsec_helpers[i]);
	}
	
}

module_init(init);
module_exit(fini);
