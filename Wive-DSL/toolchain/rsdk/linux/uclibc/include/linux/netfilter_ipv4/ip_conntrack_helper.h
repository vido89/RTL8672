/* IP connection tracking helpers. */
#ifndef _IP_CONNTRACK_HELPER_H
#define _IP_CONNTRACK_HELPER_H
#include <linux/netfilter_ipv4/ip_conntrack.h>

struct module;

struct ip_conntrack_helper
{	
	struct list_head list; 		/* Internal use. */

	const char *name;		/* name of the module */
	struct module *me;		/* pointer to self */
	unsigned int max_expected;	/* Maximum number of concurrent 
					 * expected connections */
	unsigned int timeout;		/* timeout for expecteds */

	/* Mask of things we will help (compared against server response) */
	struct ip_conntrack_tuple tuple;
	struct ip_conntrack_tuple mask;
	
	/* Function to call when data passes; return verdict, or -1 to
           invalidate. */
	int (*help)(struct sk_buff **pskb,
		    struct ip_conntrack *ct,
		    enum ip_conntrack_info conntrackinfo);

	void (*destroy)(struct ip_conntrack *ct);

	int (*to_nfattr)(struct sk_buff *skb, const struct ip_conntrack *ct);
};

extern int ip_conntrack_helper_register(struct ip_conntrack_helper *);
extern void ip_conntrack_helper_unregister(struct ip_conntrack_helper *);

/* Allocate space for an expectation: this is mandatory before calling 
   ip_conntrack_expect_related.  You will have to call put afterwards. */
extern struct ip_conntrack_expect *
ip_conntrack_expect_alloc(struct ip_conntrack *master);
extern void ip_conntrack_expect_put(struct ip_conntrack_expect *exp);

/* Add an expected connection: can have more than one per connection */
extern int ip_conntrack_expect_related(struct ip_conntrack_expect *exp);
extern void ip_conntrack_unexpect_related(struct ip_conntrack_expect *exp);
//add by ramen
#ifdef CONFIG_IP_NF_ALG_ONOFF
#define ALGONOFF_INIT(PROTO)\
static unsigned char ginitflag=0;\
extern int alginit_##PROTO();\
static unsigned char algonoff_##PROTO= 0;\
static struct proc_dir_entry* FP_Proc_algonoff_##PROTO;\
int proc_debug_write_##PROTO(struct file *file, const char *buffer, unsigned long count,	   void *data)\
{\
         char proc_buffer[count];\    
    memset(proc_buffer, 0, sizeof(proc_buffer));\
    if ( copy_from_user(proc_buffer, buffer, count) ) {\
        return -EFAULT;\
    }\
    if(proc_buffer[0]=='1'){\
	if(!algonoff_##PROTO)\
        	{     \
          printk("*****Enable %s ALG function!*****\n",#PROTO);\
	 algonoff_##PROTO=1;\
	}\
    }\
    else if(proc_buffer[0]=='0'){       \
	if(algonoff_##PROTO){\		
		printk("*****Disable %s ALG function!*****\n",#PROTO);\
		algonoff_##PROTO=0;\
		}\
    }\
    else\
        printk("Error setting!\n");\
    return -1;\
}\
int proc_debug_read_##PROTO(char *buffer,     char **buffer_location,     off_t offset, int buffer_length, int *eof, void *data)\
{\
	   if(algonoff_##PROTO==1)\
        		printk("%s ALG ON!\n",#PROTO);\
	    if(algonoff_##PROTO==0)\
        		printk("%s ALG OFF!\n",#PROTO);\
	    return -1;\
}\
int  Alg_OnOff_init_##PROTO()\
{\
	FP_Proc_algonoff_##PROTO= create_proc_entry(PROCFS_NAME,0644, &proc_root);\
    if (FP_Proc_algonoff_##PROTO == NULL) {\
        remove_proc_entry(PROCFS_NAME, &proc_root);\
        printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",\
            PROCFS_NAME);\
        return -ENOMEM;\
    }\
    FP_Proc_algonoff_##PROTO->read_proc  = (read_proc_t *)proc_debug_read_##PROTO;\
    FP_Proc_algonoff_##PROTO->write_proc  = (write_proc_t *)proc_debug_write_##PROTO;\
    FP_Proc_algonoff_##PROTO->owner       = THIS_MODULE;\
    FP_Proc_algonoff_##PROTO->mode       = S_IFREG | S_IRUGO;\
    FP_Proc_algonoff_##PROTO->uid       = 0;\
    FP_Proc_algonoff_##PROTO->gid       = 0;\
    FP_Proc_algonoff_##PROTO->size       = 4;    \
   ginitflag=1;\
    return 0;\
}
#endif
#endif /*_IP_CONNTRACK_HELPER_H*/
