/* sar.c - RealTek rtl8670 sar interface header */

#define DRV_NAME		"sar"
#define DRV_VERSION		"0.0.1"
#define DRV_RELDATE		"Jun 17, 2003"


#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/compiler.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/if_vlan.h>
#include <linux/crc32.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include "sar.h"


sar_private	*sar_dev;
struct net_device *sar_net_dev;
extern struct net_device *eth_net_dev;


UCHAR SarMacAddr[6]={0x00, 0x00, 0x00, 0x04, 0x05, 0x06};

UCHAR LLCHdr802_3[10]		= {0xAA, 0xAA, 0x03, 0x00, 0x80, 0xC2, 0x00, 0x07, 0x00, 0x00};
UCHAR VCMuxHdr802_3[2]		= {0x00, 0x00};
UCHAR LLCHdrRoutedIP[8]		= {0xAA, 0xAA, 0x03, 0x00, 0x00, 0x00, 0x08, 0x00};
UCHAR VCMuxHdrRoutedIP[1]	= {0x00}; 
UCHAR ARP_1577[8]			= {0xAA, 0xAA, 0x03, 0x00, 0x00, 0x00, 0x08, 0x06};
UCHAR DATA_1577[8]			= {0xAA, 0xAA, 0x03, 0x00, 0x00, 0x00, 0x08, 0x00};

UCHAR *FrameHeader_1483[4]={
			LLCHdr802_3, 		/* RFC1483_BR_LLC, LLC/SNAP for 802.3 */
			VCMuxHdr802_3, 	/* RFC1483_BR_VCMux, VC Mux for 802.3 */
			LLCHdrRoutedIP,		/* RFC1483_RT_LLC, LLC/SNAP for routed ip*/
			VCMuxHdrRoutedIP	/* RFC1483_RT_VCMux, VC Mux for routed ip should be NULL encap */
		};
		
UCHAR *FrameHeader_1577[2]={
			ARP_1577, 		/* for Arp Packet */
			DATA_1577		/* for Data Packet */
			};	

int8	FrameHeaderSize_1483[4]={0x0A, 0x02, 0x08, 0x00};
int8	FrameHeaderSize_1577[2]={0x08, 0x08};

/* These identify the driver base version and may not be removed. */
//static char version[] __devinitdata =
//KERN_INFO DRV_NAME " sar driver v" DRV_VERSION " (" DRV_RELDATE ")\n";

MODULE_AUTHOR("Jonah Chen <jonah@realtek.com.tw>");
MODULE_DESCRIPTION("RealTek RTL-8670 series SAR driver");
MODULE_LICENSE("GPL");

//static int debug = -1;
MODULE_PARM (debug, "i");
MODULE_PARM_DESC (debug, "sar bitmapped message enable number");

/* Maximum number of multicast addresses to filter (vs. Rx-all-multicast).
   The RTL chips use a 64 element hash table based on the Ethernet CRC.  */
//static int multicast_filter_limit = 32;
MODULE_PARM (multicast_filter_limit, "i");
MODULE_PARM_DESC (multicast_filter_limit, "8139cp maximum number of filtered multicast addresses");

#define PFX			DRV_NAME ": "


//static void sar_tx (sar_private *cp);
//static void sar_clean_rings (sar_private *cp);


#ifdef ATM_OAM
void InitOAM(unsigned short Vpi, unsigned short Vci);
#endif

//add for ADSL_LED blink check
int sar_traffic(void)
{
  static int last_sar_rx_num=0;
  static int last_sar_tx_num=0;
  sar_private *cp = sar_net_dev->priv;
  int rtn_num=0;
  
    if (last_sar_rx_num!=cp->net_stats.rx_bytes) {
    	last_sar_rx_num = cp->net_stats.rx_bytes;
    	rtn_num=1;
    }
    if (last_sar_tx_num!=cp->net_stats.tx_bytes) {
    	last_sar_tx_num = cp->net_stats.tx_bytes;
    	rtn_num=1;
    }
    return rtn_num;
}


static BOOL sar_rx (sar_private *cp, uint8 ch_no)
{
	DRV_ENTER;

	int8 		i=0,  j=0, k=0, encap_mode;
	int16	total_len;
	BOOL	rc= TRUE, reuse=FALSE;
	int		status;
	RV_STS_DESC	*RVDesc=NULL;;

	struct sk_buff 	*skb=NULL, *tmpskb=NULL;

	/* Now For 1483 only */
	encap_mode = cp->vcc[ch_no].rfc*2+cp->vcc[ch_no].framing;

	k=GetRxCDOI(cp, ch_no);
	j=cp->vcc[ch_no].RV.desc_pr;

	cp->vcc[ch_no].stat.rcv_cnt++;
	
	/* working on descriptors */
	while(((j!=k)||(!(cp->RVDesc[ch_no*RING_SIZE+j].STS&OWN)))&&(i<SAR_MAX_Process_DESC)){
	
		/* this descriptor is still own by hardware, exit this loop */
		// JONAH_DEBUG open
		if(cp->RVDesc[ch_no*RING_SIZE+j].STS&OWN)
			break;
		

		if(ch_no==OAM_CH_NO){	
			cp->vcc[ch_no].stat.rx_oam_count++;
			RVDesc = (RV_STS_DESC*)&cp->RODesc[j];
			//goto Error_Desc;
		} else {
		
			tmpskb = dev_alloc_skb(SAR_RX_Buffer_Size);
			if (tmpskb==NULL){
				cp->vcc[ch_no].stat.rx_buf_lack++;
				mark_bh(SAR_BH);
				reuse=TRUE;
				goto Error_Desc;
			} else {
				reuse=FALSE;
				cp->vcc[ch_no].stat.rx_buf_alloc++;			
			}
		
			/* Handle this Descriptor */
			RVDesc = &cp->RVDesc[ch_no*RING_SIZE+j];
			skb=(struct sk_buff *)cp->vcc[ch_no].rx_buf[j];
		
			if(skb==(struct sk_buff *)NULL) {
				goto Next_Desc;
			}
		};
		
		if((RVDesc->STS&(FS|LS))==(FS|LS)){	/* One descriptor contains one complete packet */
			cp->vcc[ch_no].stat.rx_FS_cnt++;
			cp->vcc[ch_no].stat.rx_LS_cnt++;
			cp->vcc[ch_no].stat.rx_desc_cnt++;

			if(ch_no==OAM_CH_NO){				
				#ifdef ATM_OAM
					int oam_type;
					status = RVDesc->STS;
					if(status&0x0100){
						oam_type=(status&0x00C0)>>6;
						if((oam_type == 0)||(oam_type == 1))
						{	// OAM F4 cell
							OAMRxF4Cell((unsigned char *)RVDesc->START_ADDR, &OAMF4_info);									
						}
						else if((oam_type == 2)||(oam_type == 3))
						{	// OAM F5 cell
							OAMRxF5Cell((unsigned char *)RVDesc->START_ADDR, &OAMF5_info);
						}
					};
					 
					goto Next_Desc;
				#else
					goto Next_Desc;
				#endif
					
			}
			
			if(RVDesc->STS&LENErr)
				total_len = RVDesc->BPC_LENGTH - FrameHeaderSize_1483[encap_mode];
			else 
				total_len = (RVDesc->LEN&LEN_Mask) - FrameHeaderSize_1483[encap_mode];
			
			
			if (total_len > 1518){
				cp->vcc[ch_no].stat.rx_lenb_error++;
    				goto Error_Desc;
    			}

			if(total_len <= 0){
				cp->vcc[ch_no].stat.rx_lens_error++;
				goto Error_Desc;
			}

			if(RVDesc->STS&CRC32Err){
				cp->vcc[ch_no].stat.rx_crc_error++;
				goto Error_Desc;
			}

			/* Handle Word Insertion */
			if(RVDesc->STS&WII){
				skb_reserve(skb, 2);
				total_len-=2;
			}
			skb_reserve(skb, FrameHeaderSize_1483[encap_mode]);
				
		#ifdef LoopBack_Test
			total_len=Lying_Engine(skb->data, total_len);
			/*exchange_mac(skb->data);*/
		#endif	

			skb_put(skb, total_len);

		#ifdef FAST_ROUTE_Test
			skb->dev = eth_net_dev;
			skb->pkt_type = PACKET_FASTROUTE;
		#else
			skb->protocol = eth_type_trans (skb, cp->dev);			
			skb->dev = cp->dev;
		#endif
			
			skb->ip_summed = CHECKSUM_UNNECESSARY;
		
			cp->dev->last_rx = jiffies;
			
			status=netif_rx(skb);
			
			if(status!=NET_RX_DROP)
				cp->vcc[ch_no].stat.rx_netif_cnt ++;
			
			cp->vcc[ch_no].stat.rx_byte_cnt += total_len;
			cp->net_stats.rx_bytes +=total_len;
			/*goto Next_Desc;*/
		}else {
			goto	Error_Desc;
		}
#if 0
		}else if (RVDesc->STS&FS){	/* First Segment */
			cp->vcc[ch_no].stat.rx_FS_cnt++;
			cp->vcc[ch_no].stat.rx_desc_cnt++;


			/* Handle this descriptor and 1483 Header */
			pRxMblk->mBlkHdr.mData += FrameHeaderSize_1483[encap_mode];
	   		pRxMblk->mBlkHdr.mLen  = RVDesc->LEN- FrameHeaderSize_1483[encap_mode];
    			pRxMblk->mBlkHdr.mFlags |= M_PKTHDR;    		    			

			/* Handle Word Insertion */
			if(RVDesc->STS&WII){
				pRxMblk->mBlkHdr.mData += 2;
				pRxMblk->mBlkHdr.mLen -= 2;
			}

			/* Record first Mblk information */
    			cp->RxPktHdrMblk=pRxMblk;				
			cp->pktlen = pRxMblk->mBlkHdr.mLen;

			cp->vcc[ch_no].stat.rx_byte_cnt += pRxMblk->mBlkHdr.mLen;
			
			goto Next_Desc;
			
		}else if (RVDesc->STS&LS){	/* Last Segment */
			cp->vcc[ch_no].stat.rx_LS_cnt++;
			cp->vcc[ch_no].stat.rx_desc_cnt++;

			/* No FS but LS received */
			if (cp->RxPktHdrMblk==NULL)	goto Error_Desc;

			/* if CRC32 of AAL5 ERROR(some cell maybe dropped), drop packet */
			if(RVDesc->STS&CRC32Err){
				cp->vcc[ch_no].stat.rx_crc_error++;
				goto Error_Desc;
			}

			/* Packet length too small */
			/* if(RVDesc->LEN<(Min_Packet_Size+FrameHeaderSize_1483[encap_mode]-CRC32_Size)){
				cp->vcc[ch_no].stat.rx_lens_error++;
				goto Error_Desc;
			}*/

			/* LENGTH ERROR, corret the length */
			cp->pktlen += RVDesc->LEN;
			len_diff = 0;
			if(RVDesc->STS&LENErr){
				len_diff= cp->pktlen - RVDesc->BPC_LENGTH;
				cp->pktlen = RVDesc->BPC_LENGTH;
			}else if (!RVDesc->STS&CRC32Err) {
				if(cp->pktlen != RVDesc->BPC_LENGTH)
					goto Error_Desc;	
			}

			/* handle this Desciptor */			
	   		pRxMblk->mBlkHdr.mLen = RVDesc->LEN-len_diff;
    			cp->RxCurHdrMblk->mBlkHdr.mNext=pRxMblk;
    			pRxMblk->mBlkHdr.mNext=NULL;

			/* Packet length too large */
			if(cp->pktlen>(Max_Packet_Size-CRC32_Size)){
				cp->vcc[ch_no].stat.rx_lenb_error++;
				goto Error_Desc;
			}		

    			cp->RxPktHdrMblk->mBlkPktHdr.len=cp->pktlen;
			END_RCV_RTN_CALL(&cp->end, cp->RxPktHdrMblk);		   
			cp->RxPktHdrMblk=NULL;

			cp->vcc[ch_no].stat.rx_byte_cnt += pRxMblk->mBlkHdr.mLen;

		}else {	/* Intermediate Segment */
			cp->vcc[ch_no].stat.rx_desc_cnt++;

			/* No FS but Intermediate received */
			if (cp->RxPktHdrMblk==NULL)	goto Error_Desc;

			/* handle this Desciptor */
			pRxMblk->mBlkHdr.mLen  = RVDesc->LEN;
			cp->RxCurHdrMblk->mBlkHdr.mNext=pRxMblk;
			cp->pktlen += RVDesc->LEN;

			cp->vcc[ch_no].stat.rx_byte_cnt += RVDesc->LEN;
		}

#endif

		cp->currskb=skb;			

		cp->vcc[ch_no].stat.rx_desc_ok_cnt++;
		cp->vcc[ch_no].stat.rx_pkt_cnt++;
		cp->net_stats.rx_packets ++;

		/* mask EOR and STS, clear OWN, FS, LS */
		RVDesc->STS &= 0x4FFF;
		goto Next_Desc;			

	Error_Desc:
		cp->vcc[ch_no].stat.rx_desc_fail++;
		cp->vcc[ch_no].stat.rx_pkt_fail++;
		cp->net_stats.rx_errors ++;
		cp->net_stats.rx_dropped++;
		RVDesc->STS &= 0x4FFF;
		if(reuse!=TRUE)
			dev_kfree_skb_irq(skb);
		

	Next_Desc:	
		/* Restore LEN and CMD field */
		if(ch_no==OAM_CH_NO)	RVDesc->LEN=SAR_OAM_Buffer_Size;
		else			RVDesc->LEN=SAR_RX_Buffer_Size;

		if(ch_no==OAM_CH_NO){	// reuse	
			RVDesc->STS= OWN |(RVDesc->STS&EOR);
		}else {
			/* Fill Descriptor */
			if(reuse==TRUE){
				RVDesc->STS= OWN |(RVDesc->STS&EOR);			
				cp->vcc[ch_no].rx_buf[j]=(UINT32)skb;
			}else if(tmpskb!=(struct sk_buff *)NULL){
				RVDesc->START_ADDR=(UINT32)tmpskb->data|Uncache_Mask;
				RVDesc->STS= OWN |(RVDesc->STS&EOR);
				cp->vcc[ch_no].rx_buf[j]=(UINT32)tmpskb;
			};
		}

		/* Next Descriptor */
		j= (1 + j)%SAR_RX_DESC_NUM;		
		k= GetRxCDOI(cp, ch_no);
		i++;


	}
	
	/* Restore Descriptor Index */
	cp->vcc[ch_no].RV.desc_pr = j;
	cp->ProcessRcv = 0;
	cp->vcc[ch_no].stat.rcv_ok++;


	return rc;	

}

static void sar_rx_bh(void){
	int8	ch_no;
	
	// JONAH_Prom +1
	sar_dev->RDAI_Reg=reg(SAR_RDAI_Addr);
	for(ch_no=0;ch_no<17;ch_no++){	
		if(!(sar_dev->RDAI_Reg&((UINT32)0x00000001<<ch_no))) continue;
		sar_rx(sar_dev, ch_no);
		sar_dev->vcc[ch_no].RDA_cnt++;
	}
	sar_dev->RDAI_Reg = 0;	

	/* handle RBF */
	if((sar_dev->RBFI_Reg=reg(SAR_RBFI_Addr))!=0){
		ch_no=0;
		while(!(sar_dev->RBFI_Reg&0x00000001)){
			ch_no++;
			sar_dev->RBFI_Reg=sar_dev->RBFI_Reg>>1;
		}
		sar_rx(sar_dev, ch_no);
		Clear_RBF(ch_no, GetRxCDOI(sar_dev, ch_no));		
	}
	
	return;     
}


static void sar_interrupt (int irq, void *dev_instance, struct pt_regs *regs)
{
	DRV_ENTER;

	struct net_device *dev = dev_instance;
	sar_private *cp = dev->priv;
	INT8		ch_no;
	
	cp->STS_Reg=reg(SAR_STS_Addr);

	if (netif_msg_intr(cp))
		printk(KERN_DEBUG "%s: intr, SAR Status Reg %08x \n",
		        dev->name, cp->STS_Reg);

	spin_lock(&cp->lock);
	
	/* RBF */
	if((cp->RBFI_Reg=reg(SAR_RBFI_Addr))!=0){
		ch_no=0;
		while(!(cp->RBFI_Reg&0x00000001)){
			ch_no++;
			cp->RBFI_Reg=cp->RBFI_Reg>>1;
		}


		mark_bh(SAR_BH);

		/* handle RX buffer first */
		//rc = sar_rx(cp, ch_no);

		//cdoi= GetRxCDOI(cp, ch_no);
		/* Then clear RBF */
		//if(rc==TRUE)	Clear_RBF(ch_no, cdoi);
		//else			Cell_Forced_Dropped(ch_no);
		
		cp->vcc[ch_no].RBF_cnt++;
	}

	/* RDA */
	// JONAH_Prom -1+1
	//if((cp->RDAI_Reg=reg(SAR_RDAI_Addr))!=0){
	if(cp->STS_Reg&RDA){
		mark_bh(SAR_BH);
	}

	/* TDF */
	if((cp->TDFI_Reg=reg(SAR_TDFI_Addr))!=0){
		// JONAH_DEBUG
		if (netif_queue_stopped(cp->dev))
			netif_wake_queue(cp->dev);
		
		for(ch_no=0;ch_no<17;ch_no++){
			if(!(cp->TDFI_Reg&((UINT32)0x00000001<<ch_no)))	continue;
			ClearTxBuffer(cp, ch_no);

			cp->vcc[ch_no].TDF_cnt++;
		}
	}

	/* TBE */
	if((cp->TBEI_Reg=reg(SAR_TBEI_Addr)!=0)){
		for(ch_no=0;ch_no<17;ch_no++){
			if(!(cp->TBEI_Reg&((UINT32)0x00000001<<ch_no)))	continue;
			cp->vcc[ch_no].TBE_cnt++;
			cp->vcc[ch_no].TBE_Flag=TRUE;
		}		
	}

	spin_unlock(&cp->lock);
}



static int 	sar_start_xmit (struct sk_buff *skb, struct net_device *dev)
{
	DRV_ENTER;

	sar_private *cp = dev->priv;	
	INT8				ch_no=0, CRC_Gen_offset, encap_mode ;
	INT16 			i=0, j=0, fragNum=0, avail_desc=0, total_len;
	UINT8			currCDOI,  *tmp_mac;
	UINT16			CMD=0;
	struct sk_buff 	*tmpskb;
	TV_CMD_DESC	*TVDesc;
	BOOL			insert_desc = FALSE;


	#if 0
		skb->dev = eth_net_dev;
		skb->pkt_type = PACKET_FASTROUTE;
		skb->ip_summed = CHECKSUM_UNNECESSARY;	
		cp->dev->last_rx = jiffies;
		netif_rx(skb);
		return 0;
	#endif


	spin_lock_irq(&cp->lock);

	/* !!! compare MAC and get correct VC (Ch_no) */
	if(cp->QoS_Test==TRUE){
		tmp_mac = (UINT8 *)skb->data;
		if(!memcmp(cp->vcc[0].MAC, tmp_mac, 6))
			ch_no=0;
		else if(!memcmp(cp->vcc[1].MAC, tmp_mac, 6))
			ch_no=1;
		else{ 
			printk(KERN_ERR" QoS Test MAC error\n");
			printk("skb MAC \n");
			for(i=0;i<6;i++)
				printk(KERN_ERR"%02X:", tmp_mac[5-i]);			
			printk("vcc 0 mac\n");
			for(i=0;i<6;i++)
				printk(KERN_ERR"%02X:", cp->vcc[0].MAC[5-i]);			
			printk("vcc 1 mac\n");
			for(i=0;i<6;i++)
				printk(KERN_ERR"%02X:", cp->vcc[1].MAC[5-i]);			
			goto DropPacket;
		}	
	} else
		ch_no=0;
	
	cp->vcc[ch_no].stat.send_cnt++;

	/* Clear transmitted descriptors */
	ClearTxBuffer(cp, ch_no);
	
	/* Now For 1483 only */
	encap_mode = cp->vcc[ch_no].rfc*2+cp->vcc[ch_no].framing;

	/*if no descriptor available, clear TBE if needed */
	if(((((cp->vcc[ch_no].TV.desc_pw+1)%SAR_TX_DESC_NUM)==cp->vcc[ch_no].TV.desc_pf)&&	
		(cp->TVDesc[ch_no*RING_SIZE+cp->vcc[ch_no].TV.desc_pw].CMD&OWN)!=0)){

		cp->vcc[ch_no].stat.send_desc_full++;	/* descriptors full*/
		/* get current CDOI */
		currCDOI=GetTxCDOI(cp, ch_no);

		/* if TBE occurs and current descriptor is own by hardware (that means last time TBE occurs, 
			and the desriptors has been refilled), clear TBE and make this channel be scheduled again */
		if ((reg(SAR_TBEI_Addr)&(0x00000001<<ch_no))&&((cp->TVDesc[ch_no*RING_SIZE+currCDOI].CMD)&OWN))
			Clear_TBE(ch_no, currCDOI);
		//printk(KERN_ERR" no descriptor available\n");
		
		goto DropPacket;
	}

	/* get fragment number */
	fragNum = skb_shinfo(skb)->nr_frags;
	if(fragNum ==0)	fragNum=1;
	avail_desc = (cp->vcc[ch_no].TV.desc_pf- 1 - cp->vcc[ch_no].TV.desc_pw + SAR_TX_DESC_NUM)%SAR_TX_DESC_NUM;
	/* if not enough descriptors, drop this packet */
	if (avail_desc < fragNum) 
	{		
		/*printk(KERN_ERR " SAR Tx Fail need:%d, availble: %d,desc_pf %d,desc_pw %d\n",
			fragNum+1, avail_desc, cp->vcc[ch_no].TV.desc_pf, cp->vcc[ch_no].TV.desc_pw);*/
		cp->vcc[ch_no].stat.send_desc_lack++;
		//printk(KERN_ERR" avail_desc %d <= fragNum %d\n", avail_desc, fragNum);
		goto DropPacket;
	}    

	CRC_Gen_offset=4;
	total_len = skb->len;
	tmpskb = skb; 

	if((skb->data - skb->head)<FrameHeaderSize_1483[encap_mode])
		insert_desc = TRUE;

	if(insert_desc == TRUE){
		/* fill descriptors with pMBlk, insert a new descriptor for 1483 header */
		// JONAH_DEBUG
		//for(i=0;i<(fragNum+2);i++)
		for(i=0;i<(fragNum+1);i++)
		{
	
			/* Get Current Descriptor Index */
			j= (cp->vcc[ch_no].TV.desc_pw+i)%SAR_RX_DESC_NUM;
	
			/* if this Descriptor is OWN by SAR, skip it. (we need 2 descs)*/
			if((cp->TVDesc[ch_no*RING_SIZE+j].CMD&OWN)||(cp->TVDesc[ch_no*RING_SIZE+(j+1)%SAR_RX_DESC_NUM].CMD&OWN)){
				//printk(KERN_ERR"Next Desc Own by SAR\n");
				goto DropPacket;
			} 	
	
			/* Handle this descriptor */
			TVDesc=&cp->TVDesc[ch_no*RING_SIZE+j];
			CMD=0;
			
			if(i==0){	
				/* fill 1483 header if first descriptor*/
				TVDesc->START_ADDR = (UINT32)(FrameHeader_1483[encap_mode])|Uncache_Mask;
				TVDesc->LEN = (FrameHeaderSize_1483[encap_mode])&LEN_Mask;		
			} else if (i==1){
				/* first of the fragment */
				if(fragNum==0){
					TVDesc->START_ADDR=(u32)skb->data|Uncache_Mask;
					TVDesc->LEN=skb->len&LEN_Mask;
					cp->vcc[ch_no].tx_buf[j]=(UINT32)skb;		
				} else {
					TVDesc->START_ADDR=(u32)skb->data|Uncache_Mask;
					TVDesc->LEN=(skb->len - skb->data_len)&LEN_Mask;
					cp->vcc[ch_no].tx_buf[j]=(UINT32)skb;		
				}
			}else {
				/* get next frag */
					skb_frag_t *this_frag = &skb_shinfo(skb)->frags[i-1];
				TVDesc->START_ADDR=(u32)this_frag->page_offset|Uncache_Mask;
				TVDesc->LEN=this_frag->size;
				cp->vcc[ch_no].tx_buf[j]=(UINT32)skb;
			}
						

			// JONAH_DEBUG
			//printk(KERN_ERR"sar tx %d\n", TVDesc->LEN);
			
			/* Set FS LS */
			if(i==0)			CMD |= FS;
			if(i==fragNum)	CMD |= LS;



			CMD = CMD 
					|TRLREN		/* Enable AAL5 Trailer*/
					|cp->atmport/*use ATMPORT 1 */
					|CLP		/* Set CLP */
					|OWN		/* set owner */
					|(((uint16)cp->vcc[ch_no].TV.Ether_Offset_Value<<ETHNT_OFFSET)
						&ETHNT_OFFSET_MSK);	/* enable Ethernet CRC32 generation and append*/
	
			if(j==(SAR_TX_DESC_NUM-1))		
				CMD |= EOR;
		
			/* save the CMD to Descriptor */
			TVDesc->CMD=CMD;
	
		}
		// JONAH_DEBUG
		//cp->vcc[ch_no].TV.desc_pw = (cp->vcc[ch_no].TV.desc_pw + fragNum + 2)%SAR_TX_DESC_NUM;
		cp->vcc[ch_no].TV.desc_pw = (cp->vcc[ch_no].TV.desc_pw + fragNum + 1)%SAR_TX_DESC_NUM;

	}else {

		fragNum =0;		
		for(i=0;i<(fragNum+1);i++)
		{
	
			/* Get Current Descriptor Index */
			j= (cp->vcc[ch_no].TV.desc_pw+i)%SAR_RX_DESC_NUM;
	
			/* if this Descriptor is OWN by SAR, skip it. */	
			if(cp->TVDesc[ch_no*RING_SIZE+j].CMD&OWN){
				goto DropPacket;
			}	
	
			/* Handle this descriptor */
			TVDesc=&cp->TVDesc[ch_no*RING_SIZE+j];
			CMD=0;
			
			if (i==0){/* first of the fragment */
				
				/* check if the reserved area is enought */
				if((skb->data - skb->head)<FrameHeaderSize_1483[encap_mode])	{
					//printk(KERN_ERR"  SKBuff reserved size error, reserved size =  %d\n",(skb->data - skb->head));
					goto DropPacket;
				} else {
					skb->data-=FrameHeaderSize_1483[encap_mode];
				}
				
				memcpy(skb->data, FrameHeader_1483[encap_mode], FrameHeaderSize_1483[encap_mode]);
			
				TVDesc->START_ADDR=(u32)skb->data|Uncache_Mask;
				cp->vcc[ch_no].tx_buf[j]=(UINT32)skb;
				
				skb->len+=FrameHeaderSize_1483[encap_mode];
			
				if(fragNum==0) /* only 1 fragment */
					TVDesc->LEN=skb->len&LEN_Mask;
				else  /* there are more  fragments */
					TVDesc->LEN=(skb->len - skb->data_len)&LEN_Mask;	
				
			}else {
				/* get next frag */
				skb_frag_t *this_frag = &skb_shinfo(skb)->frags[i-1];
				TVDesc->START_ADDR=(u32)this_frag->page_offset|Uncache_Mask;
				TVDesc->LEN=this_frag->size;
				cp->vcc[ch_no].tx_buf[j]=(UINT32)skb;
			}
			
			// JONAH_DEBUG
			//printk(KERN_ERR"sar tx 2 %d\n", TVDesc->LEN);
			
			/* Set FS LS */
			if(i==0)			CMD |= FS;
			if(i==fragNum)	CMD |= LS;

			
			CMD = CMD 
					|TRLREN /* Enable AAL5 Trailer*/
					|cp->atmport/*use ATMPORT 1 */
					|CLP	/* Set CLP */
					|OWN	/* set owner */
					|(((uint16)cp->vcc[ch_no].TV.Ether_Offset_Value<<ETHNT_OFFSET)
						&ETHNT_OFFSET_MSK); /* enable Ethernet CRC32 generation and append*/

	
			if(j==(SAR_TX_DESC_NUM-1))		
				CMD |= EOR;
	
			/* save the CMD to Descriptor */
			TVDesc->CMD=CMD;
	
		}
	
		cp->vcc[ch_no].TV.desc_pw = (cp->vcc[ch_no].TV.desc_pw + fragNum + 1)%SAR_TX_DESC_NUM;
	
	}

	
	currCDOI = GetTxCDOI(cp, ch_no);
	/* if TBE occurs and the current descriptor is owned by hardware, re-schedule this channel */
	if ((reg(SAR_TBEI_Addr)&(0x00000001<<ch_no))&&((cp->TVDesc[ch_no*RING_SIZE+currCDOI].CMD)&OWN))
		Clear_TBE(ch_no, currCDOI);

	/* Clear transmitted descriptors AGAIN */
	ClearTxBuffer(cp, ch_no);

	cp->vcc[ch_no].stat.send_ok++;
	
	spin_unlock_irq(&cp->lock);	
	DRV_LEAVE;
	return 0;

DropPacket:
	//printk(KERN_ERR"tx packet dropped, desc_num = %d\n",avail_desc);
	dev_kfree_skb_irq(skb);
	//dev_kfree_skb(skb);
	cp->vcc[ch_no].stat.send_fail++;
	if(insert_desc == TRUE)
		cp->vcc[ch_no].stat.tx_pkt_fail_cnt+=fragNum;
	else
		cp->vcc[ch_no].stat.tx_pkt_fail_cnt+=(fragNum+1);
	cp->net_stats.tx_errors++;
	cp->net_stats.tx_dropped++;
	//netif_stop_queue(dev);
	spin_unlock_irq(&cp->lock);	
	DRV_LEAVE;
	return 0;

}

static void sar_set_rx_mode (struct net_device *dev)
{
	DRV_ENTER;

	printk(KERN_DEBUG "sar_set_rx_mode: called.\n");

	if (dev==NULL) { printk(KERN_ERR "sar_set_rx_mode: dev is NULL!\n"); return; }
	if (dev->flags & IFF_PROMISC)
	{
		printk(KERN_DEBUG "%s: Setting promiscuous mode.\n", dev->name);
	}
	else if ((dev->mc_list) || (dev->flags & IFF_ALLMULTI))
	{
		printk(KERN_DEBUG "%s: All multicast list.\n", dev->name);
	}

	DRV_LEAVE;

}



static struct net_device_stats *sar_get_stats(struct net_device *dev)
{
	DRV_ENTER;

	sar_private *cp = dev->priv;

	if (cp==NULL) { printk("net_device_stats: cp is NULL!\n"); return NULL;}

	printk(KERN_DEBUG "rlcm_get_stats: called.\n");

	DRV_LEAVE;
	return &cp->net_stats;

}

static void sar_stop_hw (sar_private *cp)
{
	DRV_ENTER;


	int i;
	/* clear Config Register */
	Disable_SAR();

	synchronize_irq();
	udelay(10);

	/* set all desc point to 0 */
	for(i=0;i<Enable_VC_CNT;i++){
		cp->vcc[i].TV.desc_pf = 0;
		cp->vcc[i].TV.desc_pc = 0;
		cp->vcc[i].TV.desc_pw = 0;
		cp->vcc[i].RV.desc_pr = 0;
		cp->vcc[i].RV.desc_pc = 0;
		cp->vcc[i].RV.desc_pa = 0;
	}

	remove_bh(SAR_BH);
	/* ??? !!! free all memory */

	DRV_LEAVE;
}


static void sar_init_hw (sar_private *cp)
{
	DRV_ENTER;

	struct SAR_IOCTL_CFG	cfg;
	

	/* ---> SAR Entire Module Related <--- */
	Reset_Sar();
	Init_reg(cp);	/* set corresponding register address */
	SetCRCTbl();
	GenCRC10Table();

	Enable_SAR();

	/* Enable/Disable Loopback test mode */
#ifdef LoopBack_Test
	/*Enable_LoopBack(); */
	Disable_LoopBack(); 
	Enable_Sachem_Loopback();
#else
	Disable_LoopBack();
	//REG16(0xb8000c20)=0x0600;
	//Enable_Sachem_Utopia();
	//Enable_Sachem_Loopback();
#endif

	/* Select Qos Clock from External clock */
	/*Set_QoS_Int();*/		/* internal */
	Set_QoS_Ext();		/* external */

	

	/* ---> Individual Chanel Related <--- */
	/* Create one VC first */
	/*cfg.ch_no		= 0;
	cfg.vpi 		= 5;
	cfg.vci		= 35;
	cfg.rfc		= RFC1483_BRIDGED;
	cfg.framing	= LLC_SNAP;
	//cfg.loopback	= FALSE;
	cfg.QoS.Type	= QoS_UBR;
	cfg.QoS.PCR	= 0x1DFF;
	cfg.QoS.SCR	= 0;
	cfg.QoS.MBS	= 128;
	cfg.QoS.CRD	= 0;
	cfg.QoS.CDVT	= 0;
	CreateVC(cp, &cfg);*/

	cp->atmport = ATMPORT_FAST;
	cp->QoS_Test= FALSE;


	#ifdef ENA_OAM_CH
		AllocVcBuff(cp, 16);	
		Enable_Drop_NonOAM(OAM_CH_NO);
		Enable_rx_ch(OAM_CH_NO);
		Enable_tx_ch(OAM_CH_NO);
	#endif
	
	#ifdef ATM_OAM
		//remember to change the VPI/VCI setting
		OAMF5Init(0, 35, &OAMF5_info);
		OAMF4Init(0, 0, &OAMF4_info);
	#endif



	/* ---> Interrupt Related <--- */
	Enable_IERDA();
	Set_RDA(RDATHR, RDATimer);
	/* polling TBE */
	Disable_IETBE();
	/* Enable_IETBE(); */
	/* Enable_IETDF();*/
	Enable_IERBF();

	init_bh(SAR_BH, sar_rx_bh);
	DRV_LEAVE;

}

#if 0
static int sar_refill_rx (sar_private *cp)
{
	DRV_ENTER;

	#if 0
	unsigned i;

	for (i = 0; i < SAR_RX_RING_SIZE; i++) {
		struct sk_buff *skb;

		skb = dev_alloc_skb(cp->rx_buf_sz + RX_OFFSET);
		if (!skb)
			goto err_out;

		skb->dev = cp->dev;
#if 0
		cp->rx_skb[i].mapping = pci_map_single(cp->pdev,
			skb->tail, cp->rx_buf_sz, PCI_DMA_FROMDEVICE);
#endif
		cp->rx_skb[i].skb = skb;
		cp->rx_skb[i].frag = 0;
		if ((u32)skb->data &0x3)
			printk(KERN_DEBUG "skb->data unaligment %8x\n",(u32)skb->data);
		
		cp->rx_ring[i].addr = (u32)skb->data|Uncache_Mask;
		if (i == (SAR_RX_RING_SIZE - 1))
			cp->rx_ring[i].opts1 = (DescOwn | RingEnd | cp->rx_buf_sz);
		else
			cp->rx_ring[i].opts1 =(DescOwn | cp->rx_buf_sz);
		cp->rx_ring[i].opts2 = 0;
	}

	return 0;

err_out:
	sar_clean_rings(cp);
	#endif
	DRV_LEAVE;
	return -ENOMEM;
}

static int sar_init_rings (sar_private *cp)
{
	DRV_ENTER;

	#if 0
	memset(cp->tx_hqring, 0, sizeof(DMA_DESC) * SAR_TX_RING_SIZE);
	memset(cp->rx_ring, 0, sizeof(DMA_DESC) * SAR_RX_RING_SIZE);
	cp->rx_tail = 0;
	cp->tx_hqhead = cp->tx_hqtail = 0;

	#endif
	DRV_LEAVE;
	return sar_refill_rx (cp);
}

static int sar_alloc_rings (sar_private *cp)
{
	DRV_ENTER;

	#if 0
	/*cp->rx_ring = pci_alloc_consistent(cp->pdev, CP_RING_BYTES, &cp->ring_dma);*/
	void*	pBuf;
	
	
	
	pBuf = kmalloc(SAR_RXRING_BYTES,GFP_KERNEL);
	if (!pBuf)
		goto ErrMem;
	cp->rxdesc_buf = pBuf;
	memset(pBuf, 0, SAR_RXRING_BYTES);
	
	pBuf = (void*)( (u32)(pBuf + Desc_Align-1) &  ~(Desc_Align -1) ) ;
	cp->rx_ring = (DMA_DESC*)((u32)(pBuf) | Uncache_Mask);


	pBuf= kmalloc(SAR_TXRING_BYTES, GFP_KERNEL);
	if (!pBuf)
		goto ErrMem;
	cp->txdesc_buf = pBuf;
	memset(pBuf, 0, SAR_TXRING_BYTES);
	pBuf = (void*)( (u32)(pBuf + Desc_Align-1) &  ~(Desc_Align -1) ) ;
	cp->tx_hqring = (DMA_DESC*)((u32)(pBuf) | Uncache_Mask);

	return sar_init_rings(cp);

ErrMem:
	if (cp->rxdesc_buf)
		kfree(cp->rxdesc_buf);
	if (cp->txdesc_buf)
		kfree(cp->txdesc_buf);
	#endif
	DRV_LEAVE;
	return -ENOMEM;
	
}

static void sar_clean_rings (sar_private *cp)
{
	DRV_ENTER;

	#if 0
	unsigned i;


	for (i = 0; i < SAR_RX_RING_SIZE; i++) {
		if (cp->rx_skb[i].skb) {
			dev_kfree_skb(cp->rx_skb[i].skb);
		}
	}

	for (i = 0; i < SAR_TX_RING_SIZE; i++) {
		if (cp->tx_skb[i].skb) {
			struct sk_buff *skb = cp->tx_skb[i].skb;
			dev_kfree_skb(skb);
			cp->net_stats.tx_dropped++;
		}
	}

	memset(&cp->rx_skb, 0, sizeof(struct ring_info) * SAR_RX_RING_SIZE);
	memset(&cp->tx_skb, 0, sizeof(struct ring_info) * SAR_TX_RING_SIZE);
	#endif
	DRV_LEAVE;
}

static void sar_free_rings (sar_private *cp)
{
	DRV_ENTER;

	#if 0
	sar_clean_rings(cp);
	/*pci_free_consistent(cp->pdev, CP_RING_BYTES, cp->rx_ring, cp->ring_dma);*/
	kfree(cp->rxdesc_buf);
	kfree(cp->txdesc_buf);
	
	cp->rx_ring = NULL;
	cp->tx_hqring = NULL;
	#endif
	DRV_LEAVE;
}
#endif

static int sar_open (struct net_device *dev)
{
	DRV_ENTER;

	sar_private *cp = dev->priv;
	int rc;

	if (netif_msg_ifup(cp))
		printk(KERN_DEBUG "%s: enabling interface\n", dev->name);

	Alloc_desc(cp);	/* alloocate memory for descriptors */
	
	sar_init_hw(cp);

	rc = request_irq(dev->irq, sar_interrupt, SA_INTERRUPT, dev->name, dev);
	if (rc)
		goto err_out_hw;

	netif_start_queue(dev);

	DRV_LEAVE;
	return 0;

err_out_hw:
	sar_stop_hw(cp);
	/* free all buffers */
	Free_desc(cp);
	DRV_LEAVE;
	return rc;

}

static int sar_close (struct net_device *dev)
{
	DRV_ENTER;

	sar_private *cp = dev->priv;

	if (netif_msg_ifdown(cp))
		printk(KERN_DEBUG "%s: disabling interface\n", dev->name);

	netif_stop_queue(dev);
	sar_stop_hw(cp);
	free_irq(dev->irq, dev);
	/*sar_free_rings(cp);*/
	DRV_LEAVE;
	return 0;
}



extern int StartAtmOAMLoopBack(ATMOAMLBReq *pATMOAMLBReq);
extern int SetAtmOAMCpid(ATMOAMLBID *pATMOAMLBID);
extern int StopAtmOAMLoopBack(ATMOAMLBReq *pATMOAMLBReq);
extern int GetAtmOAMLoopBackStatus(ATMOAMLBState *pATMOAMLBState);
extern int GetAtmOAMLoopBackStatusFE(ATMOAMLBRXState *pATMOAMLBRXState);
extern void OAMF5ContiLbTest(ATMOAMLBReq *pucPtr);
extern void OAMF5StopLbTest(void);

static int sar_ioctl (struct net_device *dev, struct ifreq *rq, int cmd)
{
	DRV_ENTER;
	sar_private 	*cp = (sar_private *)dev->priv;
	char 		*data = (char *)rq->ifr_data;
	int8			ch_no;
	//BOOL		rc = FALSE;
	//int			i;
	
#ifdef ATM_OAM
	/*OAMF4 *pOAMF4;
	OAMF5 *pOAMF5;*/
#endif
	
	if (dev==NULL) { printk("sar_ioctl: dev is NULL!\n"); return -1;}
		
	if (cp==NULL) { printk("sar_ioctl: cp is NULL!\n"); return -2;}

	printk(KERN_DEBUG "sar_ioctl: called. cmd=0x%x, rq->ifr_name=%s\n", cmd, rq->ifr_name);

	switch (cmd) {

		case SAR_GET_STATS:
			{
	
				printk(KERN_DEBUG "sar_ioctl: SAR_GET_STATS called.\n");
				struct SAR_IOCTL_CFG	*get_cfg = (struct SAR_IOCTL_CFG	*)data;
				ch_no = get_cfg->ch_no;
				if (copy_to_user(&(get_cfg->stat),	&(cp->vcc[ch_no].stat), sizeof(ch_stat)))
					return -EFAULT;
		
				printk(KERN_DEBUG "sar_ioctl: SAR_GET_STATS done.\n");
				break;
			}

		case SAR_ENABLE:
			{
	
				printk(KERN_DEBUG "sar_ioctl: SAR_ENABLE called.\n");
				Enable_SAR();
				printk(KERN_DEBUG "sar_ioctl: SAR_ENABLE done.\n");
				break;
			}
	

		case SAR_DISABLE:
			{
				printk(KERN_DEBUG "sar_ioctl: SAR_DISABLE called.\n");
				Disable_SAR();
				printk(KERN_DEBUG "sar_ioctl: SAR_DISABLE done.\n");
				break;
			}

	
		case SAR_GET_CONFIG:
			{
				struct SAR_IOCTL_CFG	*get_cfg = (struct SAR_IOCTL_CFG	*)data;
				
				printk(KERN_DEBUG "sar_ioctl: SAR_GET_CONFIG called.\n");

				ch_no = get_cfg->ch_no;
				get_cfg->ch_no=ch_no;
				get_cfg->vpi = cp->vcc[ch_no].vpi;
				get_cfg->vci = cp->vcc[ch_no].vci;
				get_cfg->rfc = cp->vcc[ch_no].rfc;
				get_cfg->framing = cp->vcc[ch_no].framing;
				get_cfg->created = cp->vcc[ch_no].created;
				//get_cfg->loopback = cp->vcc[ch_no].loopback;

				memcpy(&get_cfg->QoS, &cp->vcc[ch_no].QoS, sizeof(Traffic_Manage));
				memcpy(&get_cfg->stat, &cp->vcc[ch_no].stat, sizeof(ch_stat));
	
				printk(KERN_DEBUG "sar_ioctl: RLCM_GET_CONFIG done.\n");
				break;
			}
	
		case SAR_SET_CONFIG:
			{
				struct SAR_IOCTL_CFG 	cfg;
				printk(KERN_DEBUG "sar_ioctl: SAR_SET_CONFIG called.\n");

				if (copy_from_user(&cfg, (struct SAR_IOCTL_CFG *)data, sizeof(struct SAR_IOCTL_CFG)))
					return -EFAULT;

				ch_no = cfg.ch_no;

				if(cp->vcc[ch_no].created !=FALSE)	return -EFAULT;

				CreateVC(cp, &cfg);
	
#ifdef ATM_OAM
				//OAMF5Init(0, 0, &OAMF5_info);
				//OAMF4Init(0, 0, &OAMF4_info);
#endif
				printk(KERN_DEBUG "sar_ioctl: SAR_SET_CONFIG done.\n");
				break;
			}


	
#ifdef ATM_OAM
		case SAR_ATM_OAM_SET_ID:
			{
				ATMOAMLBID pATMOAMLBID;
	
				if (copy_from_user(&pATMOAMLBID, (ATMOAMLBID *)data, sizeof(ATMOAMLBID)))
					return -EFAULT;
				printk("SAR_ATM_OAM_SET_ID: Vpi = %d, Vci = %d, ID = %s\n", (int)pATMOAMLBID.vpi, (int)pATMOAMLBID.vci, pATMOAMLBID.LocID);

				if(SetAtmOAMCpid(&pATMOAMLBID) == 0)
				{
					return -EFAULT; 
				}
				break;
			}	
		case SAR_ATM_OAM_START:
			{
				ATMOAMLBReq pucPtr;
				//unsigned char i;
	
				if (copy_from_user(&pucPtr, (ATMOAMLBReq *)data, sizeof(ATMOAMLBReq)))
					return -EFAULT;
	
				printk("SAR_ATM_OAM_START: Vpi = %d, Vci = %d\n", (int)pucPtr.vpi, (int)pucPtr.vci);

				if(StartAtmOAMLoopBack(&pucPtr) == 0)
				{
					return -EFAULT; 
				}
				else
				{
					printk("SAR_ATM_OAM_START: Tag is %d\n", (int)pucPtr.Tag);
				}
				break;
			}

		case SAR_ATM_OAM_STOP:
			{
				ATMOAMLBReq pucPtr;
		
				if (copy_from_user(&pucPtr, (ATMOAMLBReq *)data, sizeof(ATMOAMLBReq)))
					return -EFAULT;
				if(StopAtmOAMLoopBack(&pucPtr) == 0)
				{
					return -EFAULT; 
				}
				else
				{
					printk("SAR_ATM_OAM_STOP: Tag is %d\n", (int)pucPtr.Tag);
				}
				break;
			}
					
		case SAR_ATM_OAM_STATUS:
			{
				//OAMF5_LB_INFO *pLBInfo;
				//OAMF5_LBRX_INFO *pLBRXInfo;
				ATMOAMLBState pucPtr;
				//unsigned char i, j;
	
				if (copy_from_user(&pucPtr, (ATMOAMLBState *)data, sizeof(ATMOAMLBState)))
					return -EFAULT;
		
				printk("SAR_ATM_OAM_STATUS: Vpi = %d, Vci = %d\n",(int)pucPtr.vpi, (int)pucPtr.vci);
				if(GetAtmOAMLoopBackStatus(&pucPtr) == 0)
				{
					printk("SAR_ATM_OAM_STATUS: Tag %d NOT found!\n", (int)pucPtr.Tag);
					return -EFAULT; 
				}
				else
				{
					int i, j;
					printk("SAR_ATM_OAM_STATUS: Tag %d\n", (int)pucPtr.Tag);

					for(i=0;i<6;i++)
					{
						if(pucPtr.status[i] == FAULT_LB_WAITING)
						{
							printk("status Waiting\n");
						}
						else if(pucPtr.status[i] == FAULT_LB_STOP)
						{
							printk("status Stop, count %u, rtt %u, LLID ", (int)pucPtr.count[i], (int)pucPtr.rtt[i]);
							for (j = 0; j < OAM_LB_LLID_SIZE; j++)
								printk("%x", pucPtr.LocID[i][j]);
							printk("\n");
						}
					}
				}
				break;
			}
		case SAR_ATM_OAM_STATUS_FE:
			{
				ATMOAMLBRXState pucPtr;

				if (copy_from_user(&pucPtr, (ATMOAMLBRXState *)data, sizeof(ATMOAMLBRXState)))
					return -EFAULT;

				if(GetAtmOAMLoopBackStatusFE(&pucPtr) == 0)
				{
					printk("SAR_ATM_OAM_STATUS_FE: NOT found!\n");
					return -EFAULT; 
				}
				else
				{
					//int i, j;
					printk("SAR_ATM_OAM_STATUS_FE: \n");
				}
				break;
			}
		case SAR_ATM_OAM_RPT_LB:
			{
				ATMOAMLBReq pucPtr;
				//unsigned char i;
	
				if (copy_from_user(&pucPtr, (ATMOAMLBReq *)data, sizeof(ATMOAMLBReq)))
					return -EFAULT;
	
				printk("SAR_ATM_OAM_START: Vpi = %d, Vci = %d\n", (int)pucPtr.vpi, (int)pucPtr.vci);

				OAMF5ContiLbTest(&pucPtr);
				break;
			}
		case SAR_ATM_OAM_STOP_LB:
			{
				OAMF5StopLbTest();
				break;
			}
#endif

		
		case SAR_CREATE_VC:
			{
				struct SAR_IOCTL_CFG 	cfg;
	
				printk(KERN_DEBUG "sar_ioctl: SAR_CREATE_VC called.\n");

				if (copy_from_user(&cfg, (struct SAR_IOCTL_CFG *)data, sizeof(struct SAR_IOCTL_CFG)))
					return -EFAULT;
				
				CreateVC(cp, &cfg);
				printk(KERN_DEBUG "sar_ioctl: SAR_CREATE_VC done.\n");
				break;
			}

		case SAR_DELETE_VC:
			{
	
				struct SAR_IOCTL_CFG	cfg;	
				printk(KERN_DEBUG "sar_ioctl: SAR_DELETE_VC called.\n");

				if (copy_from_user(&cfg, (struct SAR_IOCTL_CFG *)data, sizeof(struct SAR_IOCTL_CFG)))
					return -EFAULT;

				ch_no = cfg.ch_no;

				if(cp->vcc[ch_no].created !=VC_CREATED)	return -EFAULT;
				
				DeleteVC(cp, &cfg);
	
				//cfg.created = FALSE;
				
				printk(KERN_DEBUG "sar_ioctl: SAR_DELETE_VC done.\n");
				break;
			}
		case SAR_SETMAC:
			{	
				struct SAR_IOCTL_CFG cfg;
				printk(KERN_DEBUG "sar_ioctl: SAR_SETMAC called.\n");
				if (copy_from_user(&cfg, (struct SAR_IOCTL_CFG *)data, sizeof(struct SAR_IOCTL_CFG)))
					return -EFAULT;
				ch_no=cfg.ch_no;
				memcpy(cp->vcc[ch_no].MAC, cfg.MAC, 6);
				//for(i=0;i<6;i++)
				//		"%02X ", cp->vcc[ch_no].MAC[i]);
					
				cp->QoS_Test=TRUE;
				printk(KERN_DEBUG "sar_ioctl: SAR_SETMAC done.\n");
				
				break;

			}
			
		case SAR_ENABLE_UTOPIA:
			{
	
				printk(KERN_DEBUG "sar_ioctl: SAR_ENABLE_UTOPIA called.\n");

				REG16(0xB8000c44)= (uint16)0xBE03;
				
				printk(KERN_DEBUG "sar_ioctl: SAR_ENABLE_UTOPIA done.\n");
				break;
			}		

		case SAR_UTOPIA_FAST:
			{
	
				printk(KERN_DEBUG "sar_ioctl: SAR_UTOPIA_FAST called.\n");

				cp->atmport = ATMPORT_FAST;
				flush_tx_desc(cp);
				
				printk(KERN_DEBUG "sar_ioctl: SAR_UTOPIA_FAST done.\n");
				break;
			}		

		case SAR_UTOPIA_SLOW:
			{
	
				printk(KERN_DEBUG "sar_ioctl: SAR_UTOPIA_SLOW called.\n");

				cp->atmport = ATMPORT_SLOW;
				flush_tx_desc(cp);
				
				printk(KERN_DEBUG "sar_ioctl: SAR_UTOPIA_SLOW done.\n");
				break;
			}		

	
		default:
			printk(KERN_DEBUG "sar_ioctl: not support ioctl command (0x%x)\n", cmd);
			return -EOPNOTSUPP;

	}

	DRV_LEAVE;
	return 0;
}

#if 1
/*
for auto switch data mode,
FAST mode is 1, INTERLEAVED mode is 2
hrchen 4/1/04'
*/
static void  Reset_desc(sar_private *cp)
{
	INT32	size;

	/* TV Channels : 16 byte x 64 descriptors x 16 channel, +256 for 256 byte alignment */
	size=sizeof(TV_CMD_DESC)*RING_SIZE*Enable_VC_CNT+DESC_ALIGN; 
	memset(cp->TVDesc, 0 , size);

	/* RV Channels : 16 byte x 64 descriptors x 16 channel, +256 for 256 byte alignment */
	size=sizeof(RV_CMD_DESC)*RING_SIZE*Enable_VC_CNT+DESC_ALIGN; 
	memset(cp->RVDesc, 0 , size);

	/* TO Channel : 16 byte x 64 descriptors x 1 channel, +256 for 256 byte alignment */
	size=sizeof(TO_CMD_DESC)*RING_SIZE+DESC_ALIGN; 
	memset(cp->TODesc, 0 ,size);

	/* RO Channel : 16 byte x 64 descriptors x 1 channel, +256 for 256 byte alignment */
	size=sizeof(RO_CMD_DESC)*RING_SIZE+DESC_ALIGN; 
	memset(cp->RODesc, 0 , size);
}

static int sar_quick_open (struct net_device *dev)
{
	sar_private *cp = dev->priv;
	int rc;

	Reset_desc(cp);  /* reset descriptor memory*/
	
	/* ---> SAR Entire Module Related <--- */
	Reset_Sar();
	Init_reg(cp);	/* set corresponding register address */
	Enable_SAR();
	/* Select Qos Clock from External clock */
	/*Set_QoS_Int();*/		/* internal */
	Set_QoS_Ext();		/* external */

	#ifdef ENA_OAM_CH
		//AllocVcBuff(cp, 16);	  //OAM bfr is created at start, no need to create again
		Enable_Drop_NonOAM(OAM_CH_NO);
		Enable_rx_ch(OAM_CH_NO);
		Enable_tx_ch(OAM_CH_NO);
	#endif



	/* ---> Interrupt Related <--- */
	Enable_IERDA();
	Set_RDA(RDATHR, RDATimer);
	/* polling TBE */
	Disable_IETBE();
	/* Enable_IETBE(); */
	/* Enable_IETDF();*/
	Enable_IERBF();

	//rc = request_irq(dev->irq, sar_interrupt, SA_INTERRUPT, dev->name, dev);
	//if (rc)
	//	goto err_out_hw;


	return 0;
}

static int sar_quick_close (struct net_device *dev)
{
	sar_private *cp = dev->priv;

	if (netif_msg_ifdown(cp))
		printk(KERN_DEBUG "%s: disabling interface\n", dev->name);

	netif_stop_queue(dev);

	/* clear Config Register */
	Disable_SAR();

	synchronize_irq();
	udelay(10);
	//remove_bh(SAR_BH);
	
	//free_irq(dev->irq, dev);

	return 0;
}

void set_atm_data_mode(int mode)
{
  int io_cmd=0;
  struct ifreq rq;
  struct SAR_IOCTL_CFG vc_array[17];
  int i;
  sar_private *cp = sar_net_dev->priv;
  
    if (mode==1) io_cmd = SAR_UTOPIA_FAST;
    else if (mode==2) io_cmd = SAR_UTOPIA_SLOW;
    if (io_cmd!=0) {
        memset(&rq, 0, sizeof(struct ifreq));
        memset(vc_array, 0, sizeof(vc_array));
        strcpy(rq.ifr_name, "adsl");
        sar_quick_close(sar_net_dev);
        for (i=0;i<16;i++) {
            vc_array[i].ch_no = i;
            rq.ifr_data = (char*)&vc_array[i];
            sar_ioctl(sar_net_dev, &rq, SAR_GET_CONFIG);
            if(vc_array[i].created == VC_CREATED) {
                DeleteVC(cp, &vc_array[i]);  //free tx/rx PVC skb OAM memory
                vc_array[i].created = VC_CREATED;
            };
        };
        
        //reopen
        sar_ioctl(sar_net_dev, &rq, io_cmd);
        sar_quick_open(sar_net_dev);
        for (i=0;i<16;i++) {
            if(vc_array[i].created == VC_CREATED) {
                vc_array[i].ch_no = i;
                vc_array[i].created = VC_NOT_CREATED;
                rq.ifr_data = (char*)&vc_array[i];
            	sar_ioctl(sar_net_dev, &rq, SAR_SET_CONFIG);
            }
        };
	if (netif_msg_ifup(cp))
		printk(KERN_DEBUG "%s: enabling interface\n", sar_net_dev->name);
	netif_start_queue(sar_net_dev);
    };
}
#endif



int __init sar_probe (void)
{

	struct net_device *dev;
	sar_private *cp;
	int rc;

	DRV_ENTER;

	dev = alloc_etherdev(sizeof(sar_private));
	if (!dev)
		return -ENOMEM;
	SET_MODULE_OWNER(dev);
	cp = dev->priv;
	sar_dev = (sar_private *)cp;
	sar_net_dev = dev;
	cp->dev = dev;
	spin_lock_init (&cp->lock);
	
	dev->base_addr = (unsigned long) SAR_ADDR;
	cp->regs = (void *)SAR_ADDR;

	sar_stop_hw(cp);

	memcpy((u8 *)dev->dev_addr, SarMacAddr, 6);

	dev->open 			= sar_open;
	dev->stop 			= sar_close;
	dev->set_multicast_list 	= sar_set_rx_mode;
	dev->hard_start_xmit 	= sar_start_xmit;
	dev->get_stats 		= sar_get_stats;
	dev->do_ioctl 		= sar_ioctl;

	/* !!! irq 3 is used at this point due to rupert's ISR dispatch method */
	dev->irq = 3;

	rc = register_netdev(dev);
	if (rc)
		goto err_out_iomap;

	printk (KERN_INFO "%s: %s at 0x%lx, "
		"%02x:%02x:%02x:%02x:%02x:%02x, "
		"IRQ %d\n",
		dev->name,
		"RTL8670 SAR",
		dev->base_addr,
		dev->dev_addr[0], dev->dev_addr[1],
		dev->dev_addr[2], dev->dev_addr[3],
		dev->dev_addr[4], dev->dev_addr[5],
		dev->irq);


	/*
	 * Looks like this is necessary to deal with on all architectures,
	 * even this %$#%$# N440BX Intel based thing doesn't get it right.
	 * Ie. having two NICs in the machine, one will have the cache
	 * line set at boot time, the other will not.
	 */
	return 0;

err_out_iomap:
	iounmap((void *)SAR_ADDR);
	kfree(dev);
	return -1 ;

}

static void __exit sar_exit (void)
{
}

///////////////////////////////OAM related
#ifdef ATM_OAM
int OAMFSendACell(unsigned char *pCell){

	INT16 			j=0, fragNum=1, avail_desc=0;
	UINT8			currCDOI;
	UINT16			CMD=0;
	TV_CMD_DESC	*TVDesc;
		
	//END_TX_SEM_TAKE (&sar_end->end, WAIT_FOREVER);

	sar_dev->vcc[OAM_CH_NO].stat.send_cnt++;

	/* Clear transmitted descriptors */
	ClearTxBuffer(sar_dev, OAM_CH_NO);


	{
		static int oam_tx_counter = 0;
		oam_tx_counter++;
		if(oam_tx_counter%10000 == 1)
			printk("OAM tx %d\n",oam_tx_counter);
	}

#ifdef Do_Make
	/*if no descriptor available, clear TBE if needed */
	if(((((sar_dev->vcc[OAM_CH_NO].TV.desc_pw+1)%SAR_TX_DESC_NUM)==sar_dev->vcc[ch_no].TV.desc_pf)&&	
		(sar_dev->TODesc[sar_dev->vcc[OAM_CH_NO].TV.desc_pw].CMD&OWN)!=0)){

		sar_dev->vcc[OAM_CH_NO].stat.send_desc_full++;	/* descriptors full*/
		
		/* get current CDOI */
		currCDOI=GetTxCDOI(sar_dev,OAM_CH_NO);

		/* if TBE occurs and current descriptor is own by hardware (that means last time TBE occurs, 
			and the desriptors has been refilled), clear TBE and make this channel be scheduled again */
		if ((reg(SAR_TBEI_Addr)&(0x00000001<<OAM_CH_NO))&&((sar_dev->TODesc[currCDOI].CMD)&OWN))
			Clear_TBE(OAM_CH_NO, currCDOI);

		goto DropPacket;
	}


	/* if not enough descriptors, drop this packet */
	avail_desc = (sar_dev->vcc[OAM_CH_NO].TV.desc_pf- 1 - sar_dev->vcc[OAM_CH_NO].TV.desc_pw + SAR_TX_DESC_NUM)%SAR_TX_DESC_NUM;
	if (avail_desc <= fragNum) 
	{		
		sar_dev->vcc[OAM_CH_NO].stat.send_desc_lack++;		
		goto DropPacket;
	}    
#endif

	/* Get Current Descriptor Index */
	j= sar_dev->vcc[OAM_CH_NO].TV.desc_pw;

	/* if this Descriptor is OWN by SAR*/
	if(sar_dev->TODesc[j].CMD&OWN){
		goto Drop_OAM_Cell;
	} 	

	/* Handle this descriptor */
	TVDesc=(TV_CMD_DESC *)(&sar_dev->TODesc[j]);
	CMD=0;
		
	/* fill 1483 header if first descriptor*/
	TVDesc->START_ADDR = (uint32)pCell|UNCACHE_MASK;
	TVDesc->LEN = 52;		
	sar_dev->vcc[OAM_CH_NO].tx_buf[j]=(UINT32)NULL;		
		
	/* Set FS LS */
	CMD |= (FS|LS);
		
	CMD = CMD 
			|CRC10EN	/* Enable CRC10 Generate*/
			|sar_dev->atmport  	/*use ATMPORT FAST */
			|OWN;	/* set owner */

	if(j==(SAR_TX_DESC_NUM-1))		
		CMD |= EOR;
	
	/* save the CMD to Descriptor */
	TVDesc->CMD=CMD;

	sar_dev->vcc[OAM_CH_NO].TV.desc_pw = (sar_dev->vcc[OAM_CH_NO].TV.desc_pw + 1)%SAR_TX_DESC_NUM;

	#ifndef No_CDOI
	currCDOI = GetTxCDOI(sar_dev, OAM_CH_NO);
	#endif

	/* if TBE occurs and the current descriptor is owned by hardware, re-schedule this channel */
	if ((reg(SAR_TBEI_Addr)&(0x00000001<<OAM_CH_NO))&&((sar_dev->TODesc[currCDOI].CMD)&OWN))
		#ifndef No_CDOI
			Clear_TBE(OAM_CH_NO, currCDOI);
		#else
			REG32(sar_dev->vcc[OAM_CH_NO].TV.CtlSts_Addr)|=TBEC;
		#endif
		

	/* Clear transmitted descriptors AGAIN */
	ClearTxBuffer(sar_dev, OAM_CH_NO);
	kfree(pCell);
	sar_dev->vcc[OAM_CH_NO].stat.send_ok++;
	sar_dev->vcc[OAM_CH_NO].stat.tx_byte_cnt += 53;
//	END_TX_SEM_GIVE (&sar_end->end);
	return 1;

Drop_OAM_Cell:
	printk("  tx OAM Cell dropped, available descriptors = %d\n",avail_desc);
	kfree(pCell);
	sar_dev->vcc[OAM_CH_NO].stat.send_fail++;
	sar_dev->vcc[OAM_CH_NO].stat.tx_pkt_fail_cnt+=fragNum;
	//END_TX_SEM_GIVE (&sar_end->end);
	return 0;

}


/*void  OAMFReceiveCell(unsigned int type, unsigned char *pCell){

	unsigned char *pcell2;

	pcell2 = (unsigned char *)malloc(SAR_OAM_Buffer_Size);
	memcpy(pcell2, pCell, SAR_OAM_Buffer_Size);
	OAMFSendACell(pcell2);
	return;

}*/
#endif

module_init(sar_probe);
module_exit(sar_exit);

