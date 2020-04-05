#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/kernel.h>
#include <net/tcp.h>
#include <linux/netfilter_ipv4/ip_nat.h>
#include <linux/netfilter_ipv4/ip_nat_helper.h>
#include <linux/netfilter_ipv4/ip_nat_rule.h>
#include <linux/netfilter_ipv4/ip_conntrack_helper.h>

#if 0
#define DEBUGP printk
#else
#define DEBUGP(format, args...)
#endif

// kaohj
#define ASSERT_READ_LOCK(x) MUST_BE_READ_LOCKED(&ip_conntrack_lock)
#define ASSERT_WRITE_LOCK(x) MUST_BE_WRITE_LOCKED(&ip_conntrack_lock)
#include <linux/netfilter_ipv4/ip_conntrack_core.h>
#include <linux/netfilter_ipv4/listhelp.h>

#define IPSEC_PORT 500
#define IPPROTO_ESP 0x32
#define  IPSEC_FREE 0
#define  IPSEC_USED 1
#define IPSEC_IDLE_TIME 600 
#define MaxSession 16 
struct isakmp_data_s {
	u_int64_t	icookie;	/* initiator cookie */
	u_int64_t	rcookie;	/* responder cookie */
	
	u_int32_t peer_ip;
	u_int32_t local_ip;
	u_int32_t alias_ip;
	
	u_int32_t	ospi;
	u_int32_t	ispi;
	
	u_int16_t state;
	u_int16_t idle_timer;
	// kaohj
	struct ip_conntrack	*pctrack;
};

struct timer_list ipsec_time;
static struct isakmp_data_s *_isakmpDb=NULL;
u_int32_t esp_addr=0;
u_int32_t esp_spi=0;
u_int16_t spi0;
u_int16_t spi1;
char ipsec_flag='1';

#define MAX_PORTS 8

static void _prepend_to_hash(struct ip_conntrack_tuple_hash *tuplehash, u_int32_t spi)
{
	unsigned int hash;
	u_int16_t	*pu16 = (u_int16_t *)&spi;
	struct ip_conntrack_tuple *tuple;
	// It's the first reply, associate to this connection
	tuple = &tuplehash->tuple;
	hash = (ntohl(tuple->src.ip + tuple->dst.ip
	 + tuple->src.u.all + tuple->dst.u.all
	 + tuple->dst.protonum)	+ ntohs(tuple->src.u.all))
	 % ip_conntrack_htable_size;
	LIST_DELETE(&ip_conntrack_hash[hash], tuplehash);
	tuple->src.u.all = pu16[0];
	tuple->dst.u.all = pu16[1];
	// re-hash this tuple
	hash = (ntohl(tuple->src.ip + tuple->dst.ip
	 + tuple->src.u.all + tuple->dst.u.all
	 + tuple->dst.protonum)	+ ntohs(tuple->src.u.all))
	 % ip_conntrack_htable_size;
	list_prepend(&ip_conntrack_hash[hash], tuplehash);
}

static struct isakmp_data_s*
_findEspIn(u_int32_t peer_ip,u_int32_t alias_ip,u_int32_t ispi){
	int i;
	struct isakmp_data_s *tp,*new_tp;
	tp = _isakmpDb;
	new_tp=NULL;
	
	for(i = 0 ; i < MaxSession ; i++)
	{
		if(tp->peer_ip == peer_ip &&
		   tp->alias_ip == alias_ip &&
		   tp->state == IPSEC_USED &&
		   // kaohj
		   //(tp->ispi & ispi) == tp->ispi )
		   tp->ispi == ispi )
		{	
			tp->idle_timer = 0;	   	
			// kaohj
			//tp->ispi = ispi;
		   	DEBUGP("Found session #%d in\n", i);
			return tp;
		}
		if(tp->peer_ip == peer_ip &&
		   tp->alias_ip == alias_ip &&
		   // kaohj
		   tp->ispi == 0 &&
		   tp->state == IPSEC_USED)
		{
			// kaohj
			DEBUGP("Refresh ESP session #%d on reply (new spi)\n", i);
			new_tp=tp;
	  	}
		tp++;
		
	}
	if(new_tp!=NULL)
	{
		// Here comes new spi
		new_tp->idle_timer = 0;	   	
		new_tp->ispi = ispi;
		// kaohj
		if (new_tp->pctrack != 0)
		{
			_prepend_to_hash(&new_tp->pctrack->tuplehash[IP_CT_DIR_REPLY], ispi);
		}
		else
			DEBUGP("should not be here!\n");
		return new_tp;
	}
	
	return NULL;
}


static struct isakmp_data_s*
_findEspOut(u_int32_t local_ip,u_int32_t peer_ip, u_int32_t ospi){
	int i;
	struct isakmp_data_s *tp;
	tp = _isakmpDb;
	
	for(i = 0 ; i < MaxSession ; i++){
		if(tp->peer_ip == peer_ip &&
		   tp->local_ip == local_ip &&
		   // kaohj
		   //(tp->ospi  & ospi ) == tp->ospi &&
		   tp->ospi == ospi &&
		   tp->state == IPSEC_USED ){	
		   	tp->idle_timer = 0;
		   	tp->ospi = ospi;
		   	DEBUGP("Found session #%d out\n", i);
		   	return tp;
		}
		tp++;
	}
	
	return NULL;
}



static struct isakmp_data_s*
_addEsp(u_int32_t local_ip,u_int32_t peer_ip,u_int32_t ospi){
	int i;
	struct isakmp_data_s *tp;
	tp = _isakmpDb;
	
	for(i = 0 ; i < MaxSession ; i++){
		if(tp->state == IPSEC_USED && tp->peer_ip == peer_ip
		   && tp->local_ip == local_ip){
			// Here comes new spi
			DEBUGP("New ESP session #%d out, spi=%x -> %x\n", i, tp->ospi, ospi);
			tp->idle_timer = 0;
			tp->ospi = ospi;
			tp->ispi = 0;
			return tp;
		}
		tp++;
	}
	return NULL;
}




/*********************************************************************************
* Routine Name :  _findIsakmpIn
* Description :
* Input :
* Output :
* Return :
* Note :
*        ThreadSafe: n
**********************************************************************************/
static struct isakmp_data_s*
_findIsakmpIn(u_int32_t peer_ip,u_int32_t alias_ip,u_int64_t icookie,u_int64_t rcookie){
	int i;
	struct isakmp_data_s *tp;
	// kaohj
	//u_int32_t *pu32;
	tp = _isakmpDb;
	
	for(i = 0 ; i < MaxSession ; i++){
		if(tp->peer_ip == peer_ip &&
		   tp->alias_ip == alias_ip &&
		   tp->icookie == icookie &&
		   tp->state == IPSEC_USED){
			tp->idle_timer = 0;
			tp->icookie = icookie;
			// kaohj
			//printk("find IKE in i=%d, local_ip=%x, ", i, tp->local_ip);
			//pu32 = (u_int32_t *)&icookie;
			//printk("icookie=%x, %x\n", pu32[0], pu32[1]);
			return tp;
		}
		tp++;
	}
	
	return NULL;
}


/*********************************************************************************
* Routine Name :  _findIsakmpOut
* Description :
* Input :
* Output :
* Return :
* Note :
*        ThreadSafe: n
**********************************************************************************/
static struct isakmp_data_s*
_findIsakmpOut(u_int32_t local_ip,u_int32_t peer_ip,u_int64_t icookie,u_int64_t rcookie){
	int i;
	struct isakmp_data_s *tp;
	tp = _isakmpDb;
	
	for(i = 0 ; i < MaxSession  ; i++){
		if(tp->peer_ip == peer_ip &&
		   tp->local_ip == local_ip &&
		   tp->icookie == icookie &&
		   tp->state == IPSEC_USED){
			// kaohj
			//printk("find IKE out i=%d, icookie=%x\n", i, icookie);
			return tp;
		}
		tp++;
	}
	
	return NULL;
}



/*********************************************************************************
* Routine Name :  _addIsakmp
* Description :
* Input :
* Output :
* Return :
* Note :
*        ThreadSafe: n
**********************************************************************************/
static struct isakmp_data_s*
_addIsakmp(u_int32_t local_ip,u_int32_t peer_ip,u_int32_t alias_ip,
		u_int64_t icookie,u_int64_t rcookie){
	int i;
	struct isakmp_data_s *tp;
	tp = _isakmpDb;
	// find the existed one
	for(i = 0 ; i < MaxSession  ; i++){
		if(tp->peer_ip == peer_ip &&
		   tp->local_ip == local_ip &&
		   tp->state == IPSEC_USED) {
		   	// session refresh
			tp->idle_timer = 0;
			tp->icookie = icookie;
			tp->rcookie = rcookie;
			//printk("Existed session #%d been found\n", i);
			return tp;
		}
		tp++;
	}
	
	tp = _isakmpDb;
	// if not already exists, find a new one
	for(i = 0 ; i < MaxSession  ; i++){
		if(tp->state == IPSEC_FREE){
			memset(tp,0,sizeof(struct isakmp_data_s));
			tp->idle_timer = 0;
			tp->peer_ip = peer_ip;
			tp->local_ip = local_ip;
			tp->alias_ip = alias_ip;
			tp->icookie = icookie;
			tp->rcookie = rcookie;
			tp->state = IPSEC_USED;
			// kaohj
			//printk("Free session #%d been found\n", i);
			return tp;
		}
		tp++;
	}
	return NULL;
}


#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
static unsigned int esp_help(struct ip_conntrack *ct,
			 struct ip_conntrack_expect *exp,
			 struct ip_nat_info *info,
			 enum ip_conntrack_info ctinfo,
			 unsigned int hooknum,
			 struct sk_buff **pskb)
{
	struct isakmp_data_s *tp;
	tp = _isakmpDb;
	struct isakmp_data_s *tb;
	struct iphdr *iph = (*pskb)->nh.iph;
	u_int32_t  *spi = (void *) iph + iph->ihl * 4;
	int dir = CTINFO2DIR(ctinfo);
	u_int32_t s_addr, d_addr,o_spi;
	o_spi= *spi;
	
	if(ipsec_flag=='0')
		return NF_ACCEPT;
	
	if (hooknum != 0)
		return NF_ACCEPT;
	
	s_addr=iph->saddr;
	d_addr=iph->daddr;
	DEBUGP("%x -------> %x, spi=%x\n", s_addr, d_addr, o_spi);
	
	//if (ctinfo < IP_CT_IS_REPLY)
	{	// original
		//printk("original\n");
		//printk("ori, %x, %x\n",
		// (int)(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all),
		// (int)(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all));
		//printk("rep, %x, %x\n",
		// (int)(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.all),
		// (int)(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all));
		tb = _findEspOut(ct->tuplehash[dir].tuple.src.ip, d_addr, o_spi );
		if(tb==NULL)
		{
        		tb=_addEsp(ct->tuplehash[dir].tuple.src.ip,d_addr,o_spi);
			if(tb != NULL)
			{
				// original SA been changed
				if (tb->pctrack != ct)
					tb->pctrack = ct;
				return NF_ACCEPT;
			}
			else
			{
				// maybe it's reply
				//printk("can not bind to session on original\n");
//				return NF_DROP;
			}
		}
		else
			return NF_ACCEPT;
	}
	
	//else // reply
	{	
		//printk("reply\n");
		tb = _findEspIn(ct->tuplehash[dir].tuple.src.ip, ct->tuplehash[dir].tuple.dst.ip, o_spi);
		if(tb!=NULL)
		{
			if (tb->pctrack != ct)
			{
				//printk("a new ct, reply should have been refreshed\n");
				iph->daddr=tb->local_ip;
				iph->check=0;
				iph->check=ip_fast_csum((unsigned char *)iph, iph->ihl);
			}
			
			//printk("reply to localip=%x\n", tb->local_ip);
		}
		else
		{
			//printk("can not bind to session on reply, drop it!\n");
			return NF_DROP;
		}
	}
	
	return NF_ACCEPT;
}
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
static unsigned int help(struct ip_conntrack *ct,
			 struct ip_conntrack_expect *exp,
			 struct ip_nat_info *info,
			 enum ip_conntrack_info ctinfo,
			 unsigned int hooknum, 
			 struct sk_buff **pskb)
{
	struct isakmp_data_s *tp;
	tp = _isakmpDb;
	struct iphdr *iph = (*pskb)->nh.iph;
	struct udphdr *udph = (void *) iph + iph->ihl * 4;
	int dir = CTINFO2DIR(ctinfo);
	u_int32_t s_addr, d_addr;
	u_int32_t *pu32;
	u_int64_t *dptr,icookie, rcookie;
	struct isakmp_data_s *tb;
	
	if(ipsec_flag=='0')
		return NF_ACCEPT;
	
	s_addr=iph->saddr;
	d_addr=iph->daddr;
	dptr = (u_int64_t *) ((void *) udph + sizeof(struct udphdr));
	icookie= dptr[0];
	rcookie= dptr[1];
	//printk("s_addr=%x, ", s_addr);
	//pu32 = (u_int32_t *)&icookie;
	//printk("icookie=%x, %x, \n", pu32[0], pu32[1]);
	//pu32 = (u_int32_t *)&rcookie;
	//printk("rcookie=%x, %x\n", pu32[0], pu32[1]);
       	

	// Mason Yu IKE Bug
	if( (hooknum==4 && rcookie==0 )|| 
	    (hooknum==4 && icookie==0))
	{
		tb = _findIsakmpOut(ct->tuplehash[dir].tuple.src.ip, d_addr, icookie, rcookie);
		
		if(tb==NULL)
		{			  
			_addIsakmp(ct->tuplehash[dir].tuple.src.ip,d_addr,s_addr,icookie,rcookie);
			return NF_ACCEPT;
		}
		else
		{
			//printk("drop the very first IKE\n");
			// Mason Yu IKE Bug 		
			//return NF_DROP;	// first packet, should not be found
		}
	}
	
	// Mason Yu IKE Bug
	if( (hooknum==0 && rcookie!=0) ||
	    (hooknum==0 && icookie==0) )
	{		
		if (ctinfo >= IP_CT_IS_REPLY)			
		{				
			// It's reply
			tb = _findIsakmpIn(ct->tuplehash[dir].tuple.src.ip, ct->tuplehash[dir].tuple.dst.ip, icookie, rcookie);
			if(tb!=NULL)
			{				
				iph->daddr=tb->local_ip;
				iph->check=0;
				iph->check=ip_fast_csum((unsigned char *)iph, iph->ihl);
				udph->check=0;
				udph->check=csum_partial((char *)udph,ntohs(udph->len),0);
				udph->check=csum_tcpudp_magic(iph->saddr,iph->daddr ,ntohs(udph->len),IPPROTO_UDP,udph->check);
				//printk("New local:%d.%d.%d.%d  IPcheck=%x UDPcheck=%x\n",NIPQUAD(d_addr),iph->check,udph->check);
			}
		}
		// Kaohj -- remove for checkpoint inter-operability
		#if 0
		else	// It's original
		{
			tb = _findIsakmpOut(ct->tuplehash[dir].tuple.src.ip, d_addr, icookie, rcookie);			
			
			// Mason Yu IKE Bug
			//if (tb == NULL)	
			if (tb == NULL && icookie!=0)
			{
				//printk("Not found IKE session, drop it\n");
				return NF_DROP;
			}	
			
		}
		#endif
	}
	
	return NF_ACCEPT;
}

static void check_timeout(unsigned long data)
{
	struct isakmp_data_s *tp;
	tp = _isakmpDb;
	int i;
	
	for(i=0; i < MaxSession ;i++)
	{
		if(tp == NULL || _isakmpDb == NULL)
			break;
		if(tp->state == IPSEC_FREE)
		{
			tp++;
			continue;
		}
		tp->idle_timer++;
		if(tp->idle_timer > IPSEC_IDLE_TIME)
		{
			tp->state = IPSEC_FREE;
		}
		tp++;
	}
	
	ipsec_time.expires=jiffies + 100;
	add_timer(&ipsec_time);
}
static int read_proc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	int len;
	
	len = sprintf(page, "%c\n", ipsec_flag);
	
	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count) len = count;
	if (len<0) len = 0;
	return len;
}
#include <linux/netfilter_ipv4/ip_conntrack_protocol.h>
extern struct ip_conntrack_protocol ip_conntrack_protocol_esp;
#ifdef CONFIG_RTL867X_KERNEL_MIPS16_NET
__NOMIPS16
#endif
static int write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	if (count < 2)
		return -EFAULT;
	
	if (buffer && !copy_from_user(&ipsec_flag, buffer, 1)) {
		if(ipsec_flag == '0' && (ip_conntrack_protocol_esp.list.prev != NULL 
		 || ip_conntrack_protocol_esp.list.next != NULL)){
			ip_conntrack_protocol_unregister(&ip_conntrack_protocol_esp);
			ip_conntrack_protocol_esp.list.next = NULL;
			ip_conntrack_protocol_esp.list.prev = NULL;
		}
		return count;
	}
	
	return -EFAULT;
}

// Modified by Mason Yu, This is the first different point from AP source code.
// Line 411(AP source code) : { { 0, { __constant_htons(500) } }
// Line 413(AP source code) : { { 0,{ 0xFFFF } }
static struct ip_nat_helper ip_nat_ipsec_helpers;
static struct ip_nat_helper ip_nat_ipsec_helpers = { { NULL, NULL},
	                            "ipsec",
				    IP_NAT_HELPER_F_ALWAYS,
				    THIS_MODULE,
				    { { 0, { .udp = { __constant_htons(500) } } },
			              { 0, { 0 }, IPPROTO_UDP } },
		                    { { 0, { .udp = { 0xFFFF } } } ,
		                      { 0, { 0 }, 0xFFFF } },
	                              help, NULL };

static struct ip_nat_helper esp = { { NULL, NULL},
	                            "esp",
				    IP_NAT_HELPER_F_ALWAYS,
				    THIS_MODULE,
				    { { 0, { 0 } },
			              { 0, { 0 }, IPPROTO_ESP } },
		                    { { 0, { 0 } },
		                      { 0, { 0 }, 0xFFFF } },
	                              esp_help, NULL};			      

/* This function is intentionally _NOT_ defined as  __exit, because
 * it is needed by init() */
static void fini(void)
{
	ip_nat_helper_unregister(&esp);
	ip_nat_helper_unregister(&ip_nat_ipsec_helpers);
}

static int __init init(void)
{
	int ret = 0;
	struct isakmp_data_s *tp;
	struct proc_dir_entry *res=create_proc_entry("nat_ipsec",0,NULL);
	
	if (res) {
		res->read_proc=read_proc;
		res->write_proc=write_proc;
	}
	
	tp = _isakmpDb;
	ret = ip_nat_helper_register(&ip_nat_ipsec_helpers);
	ret = ip_nat_helper_register(&esp);
	
	_isakmpDb = kmalloc(MaxSession*(sizeof(struct isakmp_data_s)),GFP_KERNEL);
	memset(_isakmpDb,0,MaxSession*(sizeof(struct isakmp_data_s)));
	
//	tp = _isakmpDb;
	init_timer(&ipsec_time);
	
	ipsec_time.expires=jiffies + 100;
	ipsec_time.data=(unsigned long)_isakmpDb;
	ipsec_time.function=&check_timeout;
	add_timer(&ipsec_time);
	
	return ret;
}

module_init(init);
module_exit(fini);
