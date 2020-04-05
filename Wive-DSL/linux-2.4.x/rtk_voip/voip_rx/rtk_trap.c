/**************************************************
 * Realtek VoIP suite
 * Bruce (kfchang@realtek.com.tw)
 * ***********************************************/



#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/if_ether.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/inet.h>

#include <asm/uaccess.h>

#include "rtk_voip.h"
#include "rtk_trap.h"
#include "../include/type.h"
//#include "../voip_manager/voip_mgr_netfilter.h"

#ifdef CONFIG_RTK_VOIP_SRTP
#include "cp3_profile.h"
#include "srtp.h"
#endif

#ifdef T38_STAND_ALONE_HANDLER
#include "../voip_drivers/t38_handler.h"
#endif

struct RTK_TRAP_profile *filter[MAX_RTP_TRAP_SESSION]={0};

#define VOIP_RX_IN_UDP // move rtk_trap() from /net/core/dev.c to /net/ipv4/udp.c

#if defined(VOIP_RX_IN_UDP)
int (*rtk_trap_hook)(struct sk_buff *skb) = NULL;
#endif

//int (*rtk_trap_hook)(struct sk_buff *skb) = NULL;
extern int (*rtk_trap_hook)(struct sk_buff *skb);
struct RTK_TRAP_profile *header;

#ifndef AUDIOCODES_VOIP
uint32 nRxRtpStatsCountByte[MAX_SLIC_CH_NUM];
uint32 nRxRtpStatsCountPacket[MAX_SLIC_CH_NUM];
#endif

#ifdef CONFIG_RTK_VOIP_SRTP
err_status_t status;

// For kernel SRTP test only
char key[30] = { 0, 1, 2, 3, 4, 5, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,  \
		20, 21, 22, 23, 24, 25, 26, 27, 28, 29};
void srtp_polity_init(struct RTK_TRAP_profile *ptr)
{
	// TX
	crypto_policy_set_rtp_default(&ptr->tx_policy.rtp);
	crypto_policy_set_rtcp_default(&ptr->tx_policy.rtcp);
	ptr->tx_policy.ssrc.type  = ssrc_any_outbound;
	ptr->tx_policy.key  = (uint8_t *) key;
	
	ptr->tx_policy.next = NULL;
	ptr->tx_policy.rtp.sec_serv = sec_serv_conf | sec_serv_auth;
	ptr->tx_policy.rtcp.sec_serv = sec_serv_none;  /* we don't do RTCP anyway */
	status = srtp_create(&ptr->tx_srtp_ctx, &ptr->tx_policy);
    	if (status) {
		PRINT_MSG("error: srtp_create() failed with code %d\n",status);
	}	

	// RX
	crypto_policy_set_rtp_default(&ptr->rx_policy.rtp);
	crypto_policy_set_rtcp_default(&ptr->rx_policy.rtcp);
	ptr->rx_policy.ssrc.type  = ssrc_any_inbound;
	ptr->rx_policy.key  = (uint8_t *) key;
	
	ptr->rx_policy.next = NULL;
	ptr->rx_policy.rtp.sec_serv = sec_serv_conf | sec_serv_auth;
	ptr->rx_policy.rtcp.sec_serv = sec_serv_none;  /* we don't do RTCP anyway */
	status = srtp_create(&ptr->rx_srtp_ctx, &ptr->rx_policy);
    	if (status) {
		PRINT_MSG("error: srtp_create() failed with code %d\n",status);
	}	

}
#endif /* CONFIG_RTK_VOIP_SRTP */

Tint32 rtk_trap_register(TstVoipMgrSession *stVoipMgrSession, Tuint8 c_id, Tuint32 m_id, Tuint32 s_id, Tint32(*callback)(Tuint8 ch_id, Tint32 m_id, void *ptr_data, Tuint32 data_len, Tuint32 flags))
{

	struct RTK_TRAP_profile *ptr;
#if 0
	PRINT_MSG("ip_src_addr = %d\n", stVoipMgrSession->ip_src_addr);
	PRINT_MSG("ip_dst_addr = %d\n", stVoipMgrSession->ip_dst_addr);
	PRINT_MSG("udp_src_port = %d\n", stVoipMgrSession->udp_src_port);
	PRINT_MSG("udp_dst_port = %d\n", stVoipMgrSession->udp_dst_port);
	PRINT_MSG("protocol = %d\n", stVoipMgrSession->protocol);
#endif
#ifdef REG_INIT_SOCKET //for UDP socket send
	TstUDPSession stUDPSession;
	stUDPSession.ip_src_addr = stVoipMgrSession->ip_src_addr;
	stUDPSession.ip_dst_addr = stVoipMgrSession->ip_dst_addr;
	stUDPSession.udp_src_port= stVoipMgrSession->udp_src_port;
	stUDPSession.udp_dst_port= stVoipMgrSession->udp_dst_port;
	udp_socket_establish( &stUDPSession);

#endif
	
#ifndef AUDIOCODES_VOIP
	if(s_id <0 || s_id > 2*SESS_NUM){
		PRINT_MSG("s_id %d non-support now!(support 0~3)\n", s_id);
		return -1;
	}
#else
	if(s_id <0 || s_id > SESS_NUM){
		PRINT_MSG("s_id %d non-support now!(support 0~%d)\n", s_id, SESS_NUM-1);
		return -1;
	}
#endif
	if(filter[s_id]!=0) {
		PRINT_MSG("s_id %d in used, please unregister first\n", s_id);
		return -1;
	}

	if(header == NULL)
	{
		PRINT_MSG("path 0\n");
		header=(struct RTK_TRAP_profile *) kmalloc(sizeof(struct RTK_TRAP_profile), GFP_ATOMIC);	
		if(header == NULL)
		{
			PRINT_MSG("RTK TRAP allocate memory error !!\n");
			return -1;
		}
		header->ip_src_addr = stVoipMgrSession->ip_src_addr;
		header->ip_dst_addr = stVoipMgrSession->ip_dst_addr;
		header->udp_src_port = stVoipMgrSession->udp_src_port;
		header->udp_dst_port = stVoipMgrSession->udp_dst_port;
		header->protocol = stVoipMgrSession->protocol;
#ifdef SUPPORT_VOICE_QOS
		header->tos = stVoipMgrSession->tos;
#endif
		header->c_id = c_id;
		header->m_id = m_id;
		header->s_id = s_id;
		header->rtk_trap_callback=(void *)callback;
#ifdef CONFIG_RTK_VOIP_SRTP
		srtp_polity_init(header);
#endif		
		header->next=NULL;
		ptr = header;
	}	
	else
	{
		PRINT_MSG("path 1\n");
		ptr=header;

		while(ptr!=NULL && ptr->next!=NULL)
		{
			ptr=ptr->next;	
		}

		ptr->next=(struct RTK_TRAP_profile *)kmalloc(sizeof(struct RTK_TRAP_profile), GFP_ATOMIC);	
		if(ptr->next == NULL) 
		{
			PRINT_MSG("RTK TRAP allocate memory error !!\n");
			return -1;
		}
		ptr->next->ip_src_addr = stVoipMgrSession->ip_src_addr;
		ptr->next->ip_dst_addr = stVoipMgrSession->ip_dst_addr;
		ptr->next->udp_src_port = stVoipMgrSession->udp_src_port;
		ptr->next->udp_dst_port = stVoipMgrSession->udp_dst_port;
		ptr->next->protocol = stVoipMgrSession->protocol;
#ifdef SUPPORT_VOICE_QOS
		ptr->next->tos = stVoipMgrSession->tos;
#endif
		ptr->next->c_id= c_id;
		ptr->next->m_id= m_id;
		ptr->next->s_id= s_id;
#ifdef CONFIG_RTK_VOIP_SRTP
		srtp_polity_init(ptr->next);
#endif		
		ptr->next->rtk_trap_callback=(void *)callback;
		ptr->next->next=NULL;

	}
	filter[s_id] = ptr;
	PRINT_MSG("establish complete\n");
	return 0;
}


Tint32 rtk_trap_unregister(Tuint32 s_id)
{
	struct RTK_TRAP_profile *ptr, *ptr1;

	ptr=header;
	ptr1=header;

#ifndef AUDIOCODES_VOIP
	if(s_id <0 || s_id > 2*SESS_NUM){
		PRINT_MSG("s_id %d non-support now!(support 0~3)\n", s_id);
		return -1;
	}
#else
	if(s_id <0 || s_id > SESS_NUM){
		PRINT_MSG("s_id %d non-support now!(support 0~%d)\n", s_id, SESS_NUM-1);
		return -1;
	}
#endif

	if(filter[s_id]==0) {
		PRINT_MSG("s_id %d non-used now, can't unregister\n", s_id);
		return -1;
	}
	while(ptr != NULL) {
		if(ptr->s_id==s_id) {
			filter[s_id]=0;
			if(ptr!=header) {
				ptr1->next=ptr->next;
			} else {
				header=header->next;
			}
#ifdef CONFIG_RTK_VOIP_SRTP
			status = srtp_dealloc(ptr->rx_srtp_ctx);
		    	if (status) {
				PRINT_MSG("error: srtp_dealloc() failed with code %d\n",status);
			}	
			status = srtp_dealloc(ptr->tx_srtp_ctx);
		    	if (status) {
				PRINT_MSG("error: srtp_dealloc() failed with code %d\n",status);
			}	
#endif			
				kfree(ptr);
			return 0;
		}
		ptr1 = ptr;
		ptr = ptr->next;
	}
	PRINT_MSG("no found\n");
	return -1;
}

int get_filter(Tuint8 ch_id, struct RTK_TRAP_profile *myptr)
{
	struct RTK_TRAP_profile *ptr;

        ptr=header;
	while( ptr !=NULL )
        {
                if(ptr->c_id!=ch_id){
                  ptr = ptr->next;
                }
		myptr = ptr;
                return 0;
        }
	return -1;
}

inline int rtk_trap_callback_trap( const struct RTK_TRAP_profile *ptr, 
								   const struct sk_buff *skb )
{
#ifdef T38_STAND_ALONE_HANDLER
	if( t38RunningState[ ptr->c_id ] == T38_START ) {
		T38_API_PutPacket( ptr->c_id, skb->data + sizeof(struct udphdr), ((skb->nh.iph)->tot_len) - 28 );
		
		return 1;
	}
#endif /* T38_STAND_ALONE_HANDLER */

	return 0;
}

/****************************************************************/
int rtk_trap(struct sk_buff *skb)
{
	struct RTK_TRAP_profile *ptr;
	//struct preprocess_result *rst;
	//struct net_device *rx_dev;

	Tuint32 src_ip;
       	Tuint32 dst_ip;
	Tuint16 src_port;
	Tuint16 dst_port;
	Tuint8 proto;
	//Tuint16 tmp;

	//PRINT_MSG("skb enter rtk_trap= %x\n",skb);
#ifndef VOIP_RX_IN_UDP
	skb->mac.ethernet=((void *)skb->data -14);
#endif	
	//rx_dev = skb->dev;
	ptr = header;
	while(ptr!=NULL) 
	{
	  //PRINT_MSG("packet enter filter\n");
#ifndef VOIP_RX_IN_UDP
	  tmp = ntohs((skb->mac.ethernet)->h_proto);
	  if ( 0x0800 != tmp ) {
		  //PRINT_MSG("h_proto = %x\n",tmp);
	    break;
	  }

	  skb->nh.iph=(void *)((skb->data));
#endif	  
	  proto = (skb->nh.iph)->protocol;
	  src_ip = ((skb->nh.iph)->saddr);
	  dst_ip = ((skb->nh.iph)->daddr);

	  if ( src_ip != ptr->ip_src_addr ||
	       dst_ip != ptr->ip_dst_addr ||
	       proto != ptr->protocol
	     )
	  {
	    //PRINT_MSG("src_ip = %x\n",src_ip);
	    //PRINT_MSG("dst_ip = %x\n",dst_ip);
	    //PRINT_MSG("proto = %x\n",proto);
	    ptr=ptr->next;
	    continue;
	  }

	  skb->h.uh=(struct udphdr *)((char *)(skb->nh.iph)+20);
	  src_port =(Tuint16) ((skb->h.uh)->source);
	  dst_port =(Tuint16) ((skb->h.uh)->dest);
#ifndef RTP_SRC_PORT_CHECK
	  if ( dst_port != ptr->udp_dst_port )
#else /*RTP_SRC_PORT_CHECK*/
	  if ( src_port != ptr->udp_src_port ||
	       dst_port != ptr->udp_dst_port 
	     )
#endif /*RTP_SRC_PORT_CHECK*/
	  {
	    //PRINT_MSG("src_port = %x\n",src_port);
	    //PRINT_MSG("dst_port = %x\n",dst_port);
	    //PRINT_MSG("src_port = %x\n",ptr->udp_src_port);
	    ptr=ptr->next;
	    continue;
	  }
	  
#ifndef AUDIOCODES_VOIP
	  nRxRtpStatsCountByte[ ptr->c_id ] += ((skb->nh.iph)->tot_len) - 28;	/* UDP payload is excluded */
	  nRxRtpStatsCountPacket[ ptr->c_id ] ++;
#endif
	  
	  //PRINT_MSG("RTK TRAP catch one packet!!\n");
	  //rst = (struct preprocess_result *)preprocess(skb);
	  //PRINT_MSG("CH = %x\n");
	  //ptr->rtk_trap_callback(CH, 0, rst->ptr_data, rst->data_len, 0);
#if 1
	  /* thlin: use sid as input to call-back function, so the call-back function DSP_pktRx() in dsp_main.c gets sid instead of mid.*/
#ifdef VOIP_RX_IN_UDP	  
  #ifdef CONFIG_RTK_VOIP_SRTP
	  void *srtp_hdr = skb->data + sizeof(struct udphdr);
	  int pkt_octet_len = ((skb->nh.iph)->tot_len) - 28;
    #ifdef FEATURE_COP3_PROFILE	  
	  unsigned long flags;
	  save_flags(flags); cli();
	  ProfileEnterPoint(PROFILE_INDEX_TEMP0);
    #endif	  
	  /* apply srtp */
	  status = srtp_unprotect(ptr->rx_srtp_ctx,
			srtp_hdr, &pkt_octet_len);
    #ifdef FEATURE_COP3_PROFILE	  
	  ProfileExitPoint(PROFILE_INDEX_TEMP0);
	  ProfileDump(PROFILE_INDEX_TEMP0, PROFILE_INDEX_TEMP0,1000);
	  restore_flags(flags);
    #endif	  
	  if (status) {
	    PRINT_MSG("error: srtp unprotection failed with code %d%s\n", status,
		    status == err_status_replay_fail ? " (replay check failed)" :
		    status == err_status_auth_fail ? " (auth check failed)" : "");	  
	  } else {	
    #if ! defined (AUDIOCODES_VOIP)		
	    if( !rtk_trap_callback_trap( ptr, skb ) )  ??? bad packet ptr/size??
	 		ptr->rtk_trap_callback(ptr->c_id, ptr->s_id, srtp_hdr, pkt_octet_len, 0);
    #else
	    if( !rtk_trap_callback_trap( ptr, skb ) )  ??? bad packet ptr/size??
	 	ptr->rtk_trap_callback(ptr->c_id, ptr->m_id, srtp_hdr, pkt_octet_len, 0); //TODO yg
    #endif
	  }
  #else
    #if ! defined (AUDIOCODES_VOIP)
	  if( !rtk_trap_callback_trap( ptr, skb ) )
	  	ptr->rtk_trap_callback(ptr->c_id, ptr->s_id, skb->data + sizeof(struct udphdr), ((skb->nh.iph)->tot_len) - 28, 0);
    #else
	  if( !rtk_trap_callback_trap( ptr, skb ) )
	  	ptr->rtk_trap_callback(ptr->c_id, ptr->m_id, skb->data + sizeof(struct udphdr), ((skb->nh.iph)->tot_len) - 28, 0); //TODO yg
    #endif
  #endif 
#else
    #if ! defined (AUDIOCODES_VOIP)
	  if( !rtk_trap_callback_trap( ptr, skb ) )  ??? bad packet ptr/size??
	    ptr->rtk_trap_callback(ptr->c_id, ptr->s_id, skb->data + 28, ((skb->nh.iph)->tot_len) - 28, 0);
    #else
	  if( !rtk_trap_callback_trap( ptr, skb ) )  ??? bad packet ptr/size??
	  	ptr->rtk_trap_callback(ptr->c_id, ptr->m_id, skb->data + 28, ((skb->nh.iph)->tot_len) - 28, 0); //TODO yg
    #endif
#endif	  
#else
	  if( !rtk_trap_callback_trap( ptr, skb ) )  ??? bad packet ptr/size??
	  	ptr->rtk_trap_callback(ptr->c_id, ptr->m_id, skb->data + 28, ((skb->nh.iph)->tot_len) - 28, 0);
#endif
	  skb_unlink(skb);
	  kfree_skb(skb);
	  //dev_put(rx_dev);	
	  return RTK_TRAP_ACCEPT;
	}
	return RTK_TRAP_NONE;	
}	


#ifndef AUDIOCODES_VOIP
void ResetRtpStatsCount( uint32 chid )
{
        extern uint32 nTxRtpStatsCountByte[MAX_SLIC_CH_NUM];
        extern uint32 nTxRtpStatsCountPacket[MAX_SLIC_CH_NUM];

        extern uint32 nRxRtpStatsLostPacket[MAX_SLIC_CH_NUM];

	if( chid >= SLIC_CH_NUM )
		return;

	/* Rx RTP statistics */
	nRxRtpStatsCountByte[ chid ] = 0;
	nRxRtpStatsCountPacket[ chid ] = 0;

	/* Tx RTP statistics */
	nTxRtpStatsCountByte[ chid ] = 0;
	nTxRtpStatsCountPacket[ chid ] = 0;

	/* Rx RTP lost */
	nRxRtpStatsLostPacket[ chid ] = 0;
}
#endif

int __init rtk_trap_init_module(void)
{
	int i;
	
#if 0
	PRINT_MSG("============= RTK VoIP SUITE ============\n");	
	PRINT_MSG("INITIAL RTP TRAP\n");
	PRINT_MSG("=========================================\n");    	
#endif
	//PRINT_MSG("1.rtk_trap_hook = %x\n", rtk_trap_hook);
	rtk_trap_hook = rtk_trap;
	header = NULL;

	extern int (*udp_rtk_trap_hook)(struct sk_buff *skb);
	extern void **udp_rtk_trap_profile_header;
	udp_rtk_trap_hook = rtk_trap;
	udp_rtk_trap_profile_header = &header;	
	
	//PRINT_MSG("2. rtk_trap_hook = %x\n", rtk_trap_hook);

#ifdef CONFIG_RTK_VOIP_SRTP

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
#ifndef CONFIG_IPSEC
	rtl8186_crypto_init(); 	
#endif	
#endif	

  	/* initialize srtp library */
  	status = srtp_init();
  	if (status) {
    		PRINT_MSG("error: srtp initialization failed with error code %d\n", status);
	}	
#endif	

#ifndef AUDIOCODES_VOIP
	for( i = 0; i < SLIC_CH_NUM; i ++ )
		ResetRtpStatsCount( i );
#endif

	return 0;
}	


void __exit rtk_trap_cleanup_module(void)
{
	rtk_trap_hook = NULL;
	header = NULL;

	PRINT_MSG("============= RTK VoIP SUITE ============\n");	
        PRINT_MSG("Remove RTK TRAP\n");
        PRINT_MSG("=========================================\n");          

}

#ifdef CONFIG_RTK_VOIP_RX_INPUT_QUEUE
#define MAX_PORTS 5 
unsigned short voip_rtp_port[MAX_PORTS] = {0,0,0,0};
unsigned char voip_inputQ = 0;

void set_input_rtp_port(unsigned short rtport,unsigned short rtcport, unsigned char channel)
{
	if (channel > 1) 
		return;

	printk("\r\n*** [VoIP Input Queue]RTP port %d channel :%d *****",rtport, channel);
	voip_rtp_port[channel*2] = rtport;
	voip_rtp_port[channel*2+1] = rtcport;
	voip_inputQ = 1;
}
void unset_input_rtp_port(unsigned char channel)
{
	if (channel > 1) 
		return;
	voip_rtp_port[channel*2] =  0;
	voip_rtp_port[channel*2+1] = 0;
	if ((voip_rtp_port[0] == 0) && (voip_rtp_port[2] == 0))
		voip_inputQ = 0;
}

/*
	This is function to improve VoIP quality in RTL8671.
	Check the destination port and put it into voip_iput_queue.
*/
int skb_is_voip( struct sk_buff *skb)
{
	unsigned short dport ;

	if (!voip_inputQ) return 0;

	memcpy(&dport, skb->data+22, 2);
	if ((dport == voip_rtp_port[0]) || (dport == voip_rtp_port[1])
		||(dport == voip_rtp_port[2])||(dport == voip_rtp_port[3]))
		return  1;
	else 
		return 0;

}

/* 
	move the receive function at br2684.c
*/
int enter_rtk_voip_trap( struct sk_buff *skb)
{
	unsigned char *ptr;
	if (skb_is_voip(skb)){
		//note : IP header only. pppoe needs fixed up
		skb->mac.ethernet=((void *)skb->data -14);
		skb->nh.iph=(void *)((skb->data));
		skb_pull(skb, sizeof(struct iphdr));
		if (rtk_trap(skb) == 0)
			return 1; //drop
		else 
		{
			printk("\r\n pkt:");
			for (ptr = skb->data; ptr < skb->data+64; ptr++)
				printk("%02x ", *ptr);
			return 0; //continue processing
		}
	}
	return 0;	//continue processing!
}

#endif

#ifndef CONFIG_RTK_VOIP_MODULE
module_init(rtk_trap_init_module);
module_exit(rtk_trap_cleanup_module);
#endif
