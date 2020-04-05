#ifndef RTK_TRAP_H
#define RTK_TRAP_H

#include <linux/skbuff.h> /* for struct sk_buff */
#include <linux/in.h> /* for struct sockaddr_in*/

#include "../include/type.h"
#include "../voip_manager/voip_mgr_netfilter.h"

#ifdef CONFIG_RTK_VOIP_SRTP
#include "srtp.h"
#endif

enum 
{
	RTK_TRAP_ACCEPT=0,	//done
	RTK_TRAP_CONTINUE,	//continue to do this packet
	RTK_TRAP_DROP,		//drop the packet
	RTK_TRAP_NONE		//send to upper layer
};

struct RTK_TRAP_profile
{
	Tuint32 ip_src_addr;
	Tuint32 ip_dst_addr;
	Tuint16 udp_src_port;
	Tuint16 udp_dst_port;
	Tuint8 protocol; 	//tcp or udp
	Tuint8 c_id;
	Tuint32 m_id;
	Tuint32 s_id;
	struct socket *serv_sock;
	struct sockaddr_in serv;
	Tint32 (*rtk_trap_callback)(Tuint8 ch_id, Tuint32 media_type, void *ptr_data, Tuint32 data_len, Tuint32 flags); 
	struct net_device *dev;
	struct RTK_TRAP_profile *next;	
#ifdef CONFIG_RTK_VOIP_SRTP

	// NOTE: Only "SUPPORT_RTCP disabled" is implemented now.
	
	Tuint32 crypto_type; // cipher type/key_len and auth type/key_len/tag_len
	Tuint8 inline_key[30]; //16-byte (128 bit) master key with a 14-byte salting key
	srtp_policy_t tx_policy;
	srtp_policy_t rx_policy;	
	srtp_ctx_t *tx_srtp_ctx;
	srtp_ctx_t *rx_srtp_ctx;
#endif

#ifdef SUPPORT_VOICE_QOS
	Tuint8 tos;	
#endif
};
//extern Tint32 rtk_trap_register(TstVoipMgrSession *stVoipMgrSession, Tuint8 ch_id, Tint32 m_id, Tint32(*callback)(Tuint8 ch_id, Tint32 m_id, void *ptr_data, Tuint32 data_len, Tuint32 flags));
extern Tint32 rtk_trap_register(TstVoipMgrSession *stVoipMgrSession, Tuint8 c_id, Tuint32 m_id, Tuint32 s_id, Tint32(*callback)(Tuint8 ch_id, Tint32 m_id, void *ptr_data, Tuint32 data_len, Tuint32 flags));

//extern Tint32 rtk_trap_unregister(Tuint8 ch_id);
extern Tint32 rtk_trap_unregister(Tuint32 s_id);
extern int get_filter(Tuint8 ch_id, struct RTK_TRAP_profile *myptr);

#endif
