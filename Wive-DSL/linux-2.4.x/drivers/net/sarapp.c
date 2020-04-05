#define uClinux

#ifdef vxWorks
#define END_MACROS
#include "vxWorks.h"
#include "wdLib.h"
#include "stdlib.h"
#include "taskLib.h"
#include "logLib.h"
#include "intLib.h"
#include "netLib.h"
#include "stdio.h"
#include "stdlib.h"
#include "sysLib.h"
#include "endLib.h"
#include "iv.h"
#include "memLib.h"
#include "semLib.h"
#include "cacheLib.h"
#include "sys/ioctl.h"
#include "etherLib.h"
#include "ioLib.h"
#include "net/protosw.h"
#include "sys/socket.h"
#include "errno.h"
#include "net/if.h"
#include "net/route.h"
#include "netinet/in.h"
#include "netinet/in_systm.h"
#include "netinet/in_var.h"
#include "netinet/ip.h"
#include "netinet/if_ether.h"
#include "net/if_subr.h"
#include "m2Lib.h"
#include "etherMultiLib.h"		/* multicast stuff. */
#include "end.h"			/* Common END structures. */
#include "netBufLib.h"
#include "muxLib.h"
#include "endLib.h"
#include "lstLib.h"			/* Needed to maintain protocol list. */
#include "interrupt.h"
#include "sar.h"
#endif 

#ifdef uClinux
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
#endif

#ifdef uClinux
#define malloc(x) kmalloc(x, GFP_KERNEL);
#define free kfree
#define printf printk
#define SAR_END_DEVICE sar_private
#define M_BLK_ID struct sk_buff *
#define netMblkClChainFree dev_kfree_skb_irq 
#define netTupleGet  dev_alloc_skb
#endif

/***** LOCAL DEFINITIONS *****/
UINT16	mantissa[0x10]=	{
	0x005, 0x016, 0x028, 0x03C, 
	0x051, 0x068, 0x080, 0x09B, 
	0x0B8, 0x0D8, 0x0FA, 0x120, 
	0x14B, 0x179, 0x1AE, 0x1E8};

UINT32	CRC32TBL[0x100];
UINT16	CRC10TBL[0x100];

/*--------------------------------
			Routines
---------------------------------*/

void  Alloc_desc(SAR_END_DEVICE *cp)
{
	UINT32	*tmp;
	INT32	size;

	/* TV Channels : 16 byte x 64 descriptors x 16 channel, +256 for 256 byte alignment */
	size=sizeof(TV_CMD_DESC)*RING_SIZE*Enable_VC_CNT+DESC_ALIGN; 
	tmp=malloc(size);	
	cp->pTVDescBuf=(char *)tmp;
	cp->TVDesc=(TV_CMD_DESC *) (((UINT32)((UINT32)tmp+DESC_ALIGN)&0x0FFFFF00)|UNCACHE_MASK);
	memset(cp->TVDesc, 0 , size);

	/* RV Channels : 16 byte x 64 descriptors x 16 channel, +256 for 256 byte alignment */
	size=sizeof(RV_CMD_DESC)*RING_SIZE*Enable_VC_CNT+DESC_ALIGN; 
	tmp=malloc(size);	
	cp->pRVDescBuf=(char *)tmp;
	cp->RVDesc=(RV_STS_DESC *) (((UINT32)((UINT32)tmp+DESC_ALIGN)&0xFFFFFF00)|UNCACHE_MASK );	
	memset(cp->RVDesc, 0 , size);

	/* TO Channel : 16 byte x 64 descriptors x 1 channel, +256 for 256 byte alignment */
	size=sizeof(TO_CMD_DESC)*RING_SIZE+DESC_ALIGN; 
	tmp=malloc(size);	
	cp->pTODescBuf=(char *)tmp;
	cp->TODesc=(TO_CMD_DESC *) (((UINT32)((UINT32)tmp+DESC_ALIGN)&0xFFFFFF00)|UNCACHE_MASK );	
	memset(cp->TODesc, 0 ,size);

	/* RO Channel : 16 byte x 64 descriptors x 1 channel, +256 for 256 byte alignment */
	size=sizeof(RO_CMD_DESC)*RING_SIZE+DESC_ALIGN; 
	tmp=malloc(size);	
	cp->pRODescBuf=(char *)tmp;
	cp->RODesc=(RO_STS_DESC *) (((UINT32)((UINT32)tmp+DESC_ALIGN)&0xFFFFFF00)|UNCACHE_MASK );	
	memset(cp->RODesc, 0 , size);
}

void  Free_desc(SAR_END_DEVICE *cp)
{
	if (cp->pTVDescBuf)		free(cp->pTVDescBuf);
	if (cp->pTODescBuf)		free(cp->pTODescBuf);
	if (cp->pRVDescBuf)		free(cp->pRVDescBuf);
	if (cp->pRODescBuf)		free(cp->pRODescBuf);
}


void Init_reg(SAR_END_DEVICE *cp)
{
	int i;
	sar_atm_vcc	*vcc;
	//vcc = malloc(sizeof(sar_atm_vcc)*17);

	/* Set Register Address */
	for (i=0;i<(Enable_VC_CNT+1);i++){

		vcc=&cp->vcc[i];

		memset(&vcc->TV, 0, sizeof(TV_Channel));
		memset(&vcc->RV, 0, sizeof(RV_Channel));
		memset(&vcc->stat, 0, sizeof(ch_stat));
		memset(&vcc->QoS, 0, sizeof(Traffic_Manage));

		vcc->TV.CtlSts_Addr	= TV_Ctrl_Addr + sizeof(TV_CMD_DESC)*i;
		vcc->TV.FDP_Addr		= TV_FDP_Addr + sizeof(TV_CMD_DESC)*i;
		vcc->TV.SCR_Addr	= TV_SCR_Addr + sizeof(TV_CMD_DESC)*i;
		vcc->TV.HDR_Addr	= TV_HDR_Addr + sizeof(TV_CMD_DESC)*i;

		vcc->RV.CtlSts_Addr	= RV_Ctrl_Addr + sizeof(RV_CMD_DESC)*i;
		vcc->RV.FDP_Addr		= RV_FDP_Addr + sizeof(RV_CMD_DESC)*i;
		vcc->RV.CKS_Addr	= RV_CKS_Addr + sizeof(RV_CMD_DESC)*i;
		vcc->RV.HDR_Addr	= RV_HDR_Addr + sizeof(RV_CMD_DESC)*i;

		vcc->TV.SegmentCRC 	= 0xFFFFFFFF;
		vcc->RV.SegmentCRC 	= 0xFFFFFFFF;		
		
		vcc->TBE_Flag	= TRUE;
		vcc->created 		= VC_NOT_CREATED;
	}
	
	cp->vcc[OAM_CH_NO].TV.CtlSts_Addr	= TO_Ctrl_Addr;
	cp->vcc[OAM_CH_NO].TV.FDP_Addr	= TO_FDP_Addr;	
	cp->vcc[OAM_CH_NO].RV.CtlSts_Addr	= RO_Ctrl_Addr;
	cp->vcc[OAM_CH_NO].RV.FDP_Addr	= RO_FDP_Addr;

	cp->CNFG_Reg	= 0x00000000;
	cp->STS_Reg	= 0x00000000;
	cp->TDFI_Reg	= 0x00000000;
	cp->TBEI_Reg	= 0x00000000;
	cp->RTOI_Reg	= 0x00000000;
	cp->RDAI_Reg	= 0x00000000;
	cp->RBFI_Reg	= 0x00000000;

	cp->tx_channel_on	=0x00000000;
	cp->rx_channel_on	=0x00000000;

	/* Assing FDP to SAR Register */
	for(i=0;i<16;i++){
		cp->vcc[i].TV.FDP		= (UINT32)(cp->TVDesc + i*RING_SIZE);
		cp->vcc[i].RV.FDP		= (UINT32)(cp->RVDesc + i*RING_SIZE);
		cp->vcc[i].TV_Desc	= (TV_CMD_DESC *)(cp->TVDesc + i*RING_SIZE);
		cp->vcc[i].RV_Desc	= (RV_STS_DESC *)(cp->RVDesc + i*RING_SIZE);
		
	}
		
	cp->vcc[16].TV.FDP		= (UINT32)cp->TODesc;
	cp->vcc[16].RV.FDP		= (UINT32)cp->RODesc;
	cp->vcc[16].TO_Desc		= (TO_CMD_DESC *)cp->TODesc;
	cp->vcc[16].RO_Desc		= (RO_STS_DESC *)cp->RODesc;
		
	reg(cp->vcc[0].TV.FDP_Addr)	= (UINT32)cp->TVDesc;
	reg(cp->vcc[0].RV.FDP_Addr)	= (UINT32)cp->RVDesc;
	reg(cp->vcc[16].TV.FDP_Addr)	= (UINT32)cp->TODesc;
	reg(cp->vcc[16].RV.FDP_Addr)	= (UINT32)cp->RODesc;
}

void SetCRCTbl(void )
{
	UINT32 	i,tmp;
	UINT8 	j;
	
	for (i=0;i<0x0100;i++)
	{
		tmp =  i<<0x18;
		for (j=0;j<8;j++){
			if ((tmp & 0x80000000)!=0)
				tmp = (tmp<<1)^CRC32_POLY;
			else
				tmp = tmp<<1;
		};
		CRC32TBL[i]=tmp ;
	};
}

void GenCRC10Table(void)
{
	UINT16	Crc10Accum;
	int 		i, j;

	for (i = 0; i < 256; i++) {
		Crc10Accum = ((UINT16) i<<2);
		for (j = 0; j < 8; j++) {
			if ((Crc10Accum <<= 1) & 0x400) 
				Crc10Accum ^= CRC10_POLY	;
		}
		CRC10TBL[i] = Crc10Accum;
	}	
}



void Enable_Sachem_Loopback(void){
	/*REG16(0xb8000c44)=0x4100;*/	/* in FPGA */
	Idle(200000);
	REG16(0xb8000c44)=0xBE03;		/* in ASIC */
	Idle(200000);
	REG16(0xb8000032)=0x1800;
	Idle(200000);
	REG16(0xb8000162)=0x0000;
	Idle(200000);
}

void Enable_Sachem_Utopia(void){
	Idle(200000);
	REG16(0xb8000c44)=0xBE03;		/* in ASIC */
	Idle(200000);
}




void Clear_TBE(int8 i, int8 CDOI)
{

	uint32 tmp=0;

	sar_dev->vcc[i].TV.CtlSts |=TBEC;
	sar_dev->vcc[i].TV.CtlSts &= 0xFF00FFFF;
	sar_dev->vcc[i].TV.CtlSts |= ((uint32)CDOI)<<CDOI_Offset ;
	tmp=sar_dev->vcc[i].TV.CtlSts;

	/* Next Time we may not want to clear TBE */
	sar_dev->vcc[i].TV.CtlSts &= (TBEC^0xFFFFFFFF);

	WriteD_(sar_dev->vcc[i].TV.CtlSts_Addr, tmp);
	return;
}


void Clear_RBF(int8 i, int8 CDOI)
{

	uint32 tmp=0;

	sar_dev->vcc[i].RV.CtlSts |=RBFC;
	sar_dev->vcc[i].RV.CtlSts &= 0xFF03FFFF;
	sar_dev->vcc[i].RV.CtlSts |= ((uint32)CDOI)<<CDOI_Offset ;
	tmp=sar_dev->vcc[i].RV.CtlSts;

	/* Next Time we may not want to clear RBF */
	sar_dev->vcc[i].RV.CtlSts &= (RBFC^0xFFFFFFFF);

	WriteD_(sar_dev->vcc[i].RV.CtlSts_Addr, tmp);
	return;
}


void Cell_Forced_Dropped(int8 i){
	uint32 tmp=0;

	sar_dev->vcc[i].RV.CtlSts |=CFD;
	sar_dev->vcc[i].RV.CtlSts |=RBFC;
		tmp=sar_dev->vcc[i].RV.CtlSts;

	/* Next Time we may not want to clear RBF and drop cell */
	sar_dev->vcc[i].RV.CtlSts &= (RBFC^0xFFFFFFFF);
	sar_dev->vcc[i].RV.CtlSts &= (CFD^0xFFFFFFFF);

	WriteD_(sar_dev->vcc[i].RV.CtlSts_Addr, tmp);

	return;
}




void Reset_Sar(void){
	reg(Test_Reg_0)=0x00000001;
	reg(Test_Reg_0)=0x00000000;
}

void Idle(int32 period){

	int32	i;

	for(i=0;i<(period);i++);
	return;
	
}

void Dump(uint32 Buffer, int32 size){

	int	k;	
	if(size%4)	size=size+4-(size%4);

	Buffer=Buffer&0xFFFFFFFC;
	
	if ((Buffer&0xF0000000)==0x80000000) Buffer |= UNCACHE_MASK;
	printf("Address  : Data");
	for(k=0;k<(size/4);k++){
		if ((k%4)==0) {
			printf ("\n");
			printf("%08X : ",Buffer+k*4);
		}		
		printf("%08X  ", reg(Buffer+k*4));
	}
	printf("\n");


}

void Search(uint32 pattern){

	uint32	i;

	printf("search pattern = 0x%08X\n", pattern);

	for(i=0;i<0x00FFFFFF;i=i+4){
		if(reg(0xA0000000+i)==pattern)	
			printf("--------->Match Address=0x%08X\n", 0xA0000000+i);
		if(!(i&0x000FFFFF))
			printf("Now Scan Address=0x%08X\n", 0xA0000000+i);
		
	}
	return;

}

void	SetVpiVci(uint8 VPI, uint16 VCI,int8  ch_no){

	uint32 HDR=0x00000000;
	HDR |=  ((((uint32) VPI)<< 20) | (((uint32) VCI) << 4));

	/* set VPI VCI value to corresponding channel, GFC is always set to 0 */
	sar_dev->vcc[ch_no].TV.HDR = HDR;
	WriteD_(sar_dev->vcc[ch_no].TV.HDR_Addr, sar_dev->vcc[ch_no].TV.HDR);
	sar_dev->vcc[ch_no].RV.HDR = HDR;
	WriteD_(sar_dev->vcc[ch_no].RV.HDR_Addr, sar_dev->vcc[ch_no].RV.HDR);
	sar_dev->vcc[ch_no].vpi = VPI;
	sar_dev->vcc[ch_no].vci = VCI;
	return;
}

void SetQoS(int8 i, int8 QoS)
{
	/* Set Scheduler Option */
	uint32	SSL = 0 ;
	
	SSL |= (((uint32) QoS)<< 29);
	sar_dev->vcc[i].QoS.Type=QoS;
	sar_dev->vcc[i].TV.CtlSts &= 0x8FFFFFFF;
	sar_dev->vcc[i].TV.CtlSts |= SSL;
	WriteD_(sar_dev->vcc[i].TV.CtlSts_Addr, sar_dev->vcc[i].TV.CtlSts);
	return;	

}

void	SetPCR(int8 i, uint16 PCR)
{
	/* Set Peak Cell Rate */
	sar_dev->vcc[i].QoS.PCR=PCR;
	sar_dev->vcc[i].TV.CtlSts &= 0xFFFF0000;
	sar_dev->vcc[i].TV.CtlSts |= (uint32)PCR;
	WriteD_(sar_dev->vcc[i].TV.CtlSts_Addr, sar_dev->vcc[i].TV.CtlSts);
	return;	
}



void	SetSCR(int8 i, uint16 SCR)
{
	/* Set Sustainable Cell Rate */
	sar_dev->vcc[i].QoS.SCR=SCR;
	sar_dev->vcc[i].TV.SCR &= 0x0000FFFF;
	sar_dev->vcc[i].TV.SCR |= ((uint32)SCR)<<16;
	WriteD_(sar_dev->vcc[i].TV.SCR_Addr, sar_dev->vcc[i].TV.SCR);
	return;	
}


void	SetMBS(int8 i, uint8 MBS)
{
	/* Set Max Burst Size */
	sar_dev->vcc[i].QoS.MBS=MBS;
	sar_dev->vcc[i].TV.SCR &= 0xFFFF00FF;
	sar_dev->vcc[i].TV.SCR |= ((uint32)MBS)<<8;
	WriteD_(sar_dev->vcc[i].TV.SCR_Addr, sar_dev->vcc[i].TV.SCR);
	return;	
}

uint8 GetCRD(int8 i)
{
	uint8	CRD;
	/* Set Cell Credit */
	sar_dev->vcc[i].TV.SCR=ReadD_(sar_dev->vcc[i].TV.SCR_Addr);
	CRD = (uint8)(sar_dev->vcc[i].TV.SCR&0x000000FF);
	sar_dev->vcc[i].QoS.CRD=CRD;
	return CRD;	
}



uint32 ReadD_ (uint32 address){
	address = address & 0xFFFFFFFC;
	if ((address&0xF0000000)==0x80000000) address |= UNCACHE_MASK;
	return reg(address);
}

void WriteD_ (uint32 address,  uint32 data){
	address = address & 0xFFFFFFFC;	
	if ((address&0xF0000000)==0x80000000) address |= UNCACHE_MASK;
	reg(address)=data;
	return;
	
}

void Set1 (uint32 address,  int8 index ){
	if ((address&0xF0000000)==0x80000000) address += 0x20000000;	
	address &= 0xFFFFFFFC;
	reg(address) = (reg(address)|(((uint32)0x00000001) << index));
	return;
}

void Reset1 (uint32 address,  int8 index ){
	if ((address&0xF0000000)==0x80000000) address += 0x20000000;	
	address &= 0x0FFFFFFC;
	reg(address) &= (0xFFFFFFFF^(((uint32)0x00000001) << index));
	return;
}

uint8 Test1 (uint32 address,  int8 index ){
	if ((address&0xF0000000)==0x80000000) address += 0x20000000;	
	address &= 0xFFFFFFFC;
	if(reg(address)&(((uint32)0x00000001) << index)) 	return TRUE;
	else 									return FALSE;
}

int32 S2i(uint8 * str_P)
{
    uint32  val;
    
    if ( (str_P[0] == '0') && (str_P[1] == 'x') )
    {
        str_P += 2;
        for (val = 0; *str_P; str_P++)
        {
            val *= 16;
            if ( '0' <= *str_P && *str_P <= '9' )
                val += *str_P - '0';
            else if ( 'a' <= *str_P && *str_P <= 'f' )
                val += *str_P - 'a' + 10;
            else if ( 'A' <= *str_P && *str_P <= 'F' )
                val += *str_P - 'A' + 10;
            else
                break;
        }
    }
    else
    {
        for (val = 0; *str_P; str_P++)
        {
            val *= 10;
            if ( '0' <= *str_P && *str_P <= '9' )
                val += *str_P - '0';
            else
                break;

        }
    }
    
    return val;
}


/*--------------------------------------------
	Enable/Disable Control routines used by commands
---------------------------------------------*/


void Enable_IERBF(void){
	sar_dev->CNFG_Reg |=IERBF;
	WriteD_(SAR_CNFG_Addr, sar_dev->CNFG_Reg);
	return;
}

void Disable_IERBF(void){
	sar_dev->CNFG_Reg &=(IERBF^0xFFFFFFFF);
	WriteD_(SAR_CNFG_Addr, sar_dev->CNFG_Reg);
	return;
}

void Set_RDA(int8 RdaThr, int8 RdaTimer){

	sar_dev->CNFG_Reg &= 0xF000FFFF;
	sar_dev->CNFG_Reg |= (((uint32)RdaThr)<<RDATHR_Offset)&0x07000000;
	sar_dev->CNFG_Reg |= (((uint32)RdaTimer)<<RDATIMER_Offset)&0x003F0000;
	WriteD_(SAR_CNFG_Addr, sar_dev->CNFG_Reg);
	return;
}

void Enable_IERDA(void){

	sar_dev->CNFG_Reg |=IERDA;
	WriteD_(SAR_CNFG_Addr, sar_dev->CNFG_Reg);
	return;
}


void Disable_IERDA(void){
	sar_dev->CNFG_Reg &=(IERDA^0xFFFFFFFF);
	WriteD_(SAR_CNFG_Addr, sar_dev->CNFG_Reg);
	return;
}

void Enable_IERTO(void){
	sar_dev->CNFG_Reg |=IERTO;
	WriteD_(SAR_CNFG_Addr, sar_dev->CNFG_Reg);
	return;
}

void Disable_IERTO(void){
	sar_dev->CNFG_Reg &=(IERTO^0xFFFFFFFF);
	WriteD_(SAR_CNFG_Addr, sar_dev->CNFG_Reg);
	return;
}

void Enable_IETBE(void){
	sar_dev->CNFG_Reg |=IETBE;
	WriteD_(SAR_CNFG_Addr, sar_dev->CNFG_Reg);
	return;
}

void Disable_IETBE(void){
	sar_dev->CNFG_Reg &=(IETBE^0xFFFFFFFF);
	WriteD_(SAR_CNFG_Addr, sar_dev->CNFG_Reg);
	return;
}

void Enable_IETDF(void){
	sar_dev->CNFG_Reg |=IETDF;
	WriteD_(SAR_CNFG_Addr, sar_dev->CNFG_Reg);
	return;
}

void Disable_IETDF(void){
	sar_dev->CNFG_Reg &=(IETDF^0xFFFFFFFF);
	WriteD_(SAR_CNFG_Addr, sar_dev->CNFG_Reg);
	return;
}

void Enable_LoopBack(void)
{
	sar_dev->CNFG_Reg |=UFLB;
	WriteD_(SAR_CNFG_Addr, sar_dev->CNFG_Reg);
	return;
}


void Disable_LoopBack(void)
{
	sar_dev->CNFG_Reg &=(UFLB^0xFFFFFFFF);
	WriteD_(SAR_CNFG_Addr, sar_dev->CNFG_Reg);
	return;
}

void Enable_SAR(void)
{
	sar_dev->CNFG_Reg |=SAREN;
	WriteD_(SAR_CNFG_Addr, sar_dev->CNFG_Reg);
	return;
}

void Disable_SAR(void)
{
	sar_dev->CNFG_Reg &=(SAREN^0xFFFFFFFF);
	WriteD_(SAR_CNFG_Addr, sar_dev->CNFG_Reg);
	return;
}

void Enable_ATRLREN(int8 i){
	sar_dev->vcc[i].RV.CtlSts |=ATRLREN;	
	WriteD_(sar_dev->vcc[i].RV.CtlSts_Addr, sar_dev->vcc[i].RV.CtlSts);
	return;
}

void Disable_ATRLREN(int8 i){
	sar_dev->vcc[i].RV.CtlSts &= (ATRLREN^0xFFFFFFFF);
	WriteD_(sar_dev->vcc[i].RV.CtlSts_Addr, sar_dev->vcc[i].RV.CtlSts);
	return;
}

void Enable_AAL5(int8 i)
{
	sar_dev->vcc[i].RV.CtlSts |=AAL5;
	WriteD_(sar_dev->vcc[i].RV.CtlSts_Addr, sar_dev->vcc[i].RV.CtlSts);
	return;
}


void Enable_Raw(int8 i)
{
	sar_dev->vcc[i].RV.CtlSts &=(AAL5^0xFFFFFFFF);
	WriteD_(sar_dev->vcc[i].RV.CtlSts_Addr, sar_dev->vcc[i].RV.CtlSts);
	return;
}

void Enable_tx_ch(int8 i)
{
	sar_dev->vcc[i].TV.CtlSts |=CHEN;
	sar_dev->tx_channel_on|=(1<<i);
	WriteD_(sar_dev->vcc[i].TV.CtlSts_Addr, sar_dev->vcc[i].TV.CtlSts);

	return;
}

void Enable_rx_ch(int8 i)
{
	sar_dev->vcc[i].RV.CtlSts |=CHEN;
	sar_dev->rx_channel_on|=(1<<i);
	WriteD_(sar_dev->vcc[i].RV.CtlSts_Addr, sar_dev->vcc[i].RV.CtlSts);
	return;
}

void Disable_tx_ch(int8 i)
{	
	sar_dev->vcc[i].TV.CtlSts &=(CHEN^0xFFFFFFFF);
	sar_dev->tx_channel_on&=((1<<i)^0xFFFFFFFF);
	WriteD_(sar_dev->vcc[i].TV.CtlSts_Addr, sar_dev->vcc[i].TV.CtlSts);
	return;
}

void Disable_rx_ch(int8 i)
{	
	sar_dev->vcc[i].RV.CtlSts &=(CHEN^0xFFFFFFFF);
	sar_dev->rx_channel_on&=((1<<i)^0xFFFFFFFF);
	WriteD_(sar_dev->vcc[i].RV.CtlSts_Addr, sar_dev->vcc[i].RV.CtlSts);
	return;
}



void Enable_Word_Insert(int8 i)
{	
	sar_dev->vcc[i].RV.CKS |=WIEN;
	WriteD_(sar_dev->vcc[i].RV.CKS_Addr, sar_dev->vcc[i].RV.CKS);
	return;
}


void Disable_Word_Insert(int8 i)
{	
	sar_dev->vcc[i].RV.CKS &= (WIEN^0xFFFFFFFF);
	WriteD_(sar_dev->vcc[i].RV.CKS_Addr, sar_dev->vcc[i].RV.CKS);
	return;
}



void Enable_Drop_NonOAM(int8 i)
{	
	sar_dev->vcc[i].RV.CtlSts |=DNOAMEN;
	WriteD_(sar_dev->vcc[i].RV.CtlSts_Addr, sar_dev->vcc[i].RV.CtlSts);
	return;
}


void Disable_Drop_NonOAM(int8 i)
{	
	sar_dev->vcc[i].RV.CtlSts &= (DNOAMEN ^ 0xFFFFFFFF);
	WriteD_(sar_dev->vcc[i].RV.CtlSts_Addr, sar_dev->vcc[i].RV.CtlSts);
	return;
}


void	Set_QoS_Int(void){
	sar_dev->CNFG_Reg &= (QCLKSEL^0xFFFFFFFF) ;
	WriteD_(SAR_CNFG_Addr, sar_dev->CNFG_Reg);
	return;
}


void	Set_QoS_Ext(void){
	sar_dev->CNFG_Reg |= QCLKSEL;
	WriteD_(SAR_CNFG_Addr, sar_dev->CNFG_Reg);
	return;
}

BOOL ClearRxBuffer(SAR_END_DEVICE *cp, int8 ch_no){

	int8				j, i;

	if(cp->vcc[ch_no].RV.desc_pr==cp->vcc[ch_no].RV.desc_pc){
		printf("channel-%d-Rx Buffer Bull with desc_pr == desc_pc\n",ch_no);
		return FALSE;
	}

	cp->vcc[ch_no].RV.desc_pc=cp->vcc[ch_no].RV.desc_pa;
	
	/*  Read Descriptor and Allocate the memory */
	j=cp->vcc[ch_no].RV.desc_pa;
	i=(cp->vcc[ch_no].RV.desc_pr+SAR_RX_DESC_NUM-1)%SAR_RX_DESC_NUM;
	
	while(j!=i){
		if(!(cp->RVDesc[ch_no*RING_SIZE+j].STS&OWN_32)){

			/* Free Memory for this descriptor */
			/* free(tmpRV_DESC->START_ADDR); */
				
			/* Allocate new buffer for this descriptor, and modify the address to non-cacheable area */
			/* cp->RVDesc[ch_no*RING_SIZE+j].START_ADDR=(uint32)(malloc(Rx_Side))|UNCACHE_MASK;*/
	
			/* Restore LEN and CMD field */
			cp->RVDesc[ch_no*RING_SIZE+j].LEN = SAR_RX_Buffer_Size;
			cp->RVDesc[ch_no*RING_SIZE+j].STS|=0x8000;
			cp->RVDesc[ch_no*RING_SIZE+j].STS&=0xC000; /* EOR and OWN bit */
			j++;
			j%=SAR_RX_DESC_NUM;
		} else break;
	}
	cp->vcc[ch_no].RV.desc_pa=j;	

	return TRUE;

}

BOOL ClearTxBuffer(SAR_END_DEVICE *cp, int8 ch_no){

	int16 	 		j=0;
	TV_CMD_DESC	*TVDesc;
	M_BLK_ID	pMblk;
	UINT32			encap_mode;

	j=cp->vcc[ch_no].TV.desc_pf;	
	cp->vcc[ch_no].TV.desc_pc=(uint8)((REG32(cp->vcc[ch_no].TV.CtlSts_Addr)&CDOI_Mask)>>18);
	
	/* Now For 1483 only */
	encap_mode = cp->vcc[ch_no].rfc*2+cp->vcc[ch_no].framing;

	TVDesc=&cp->TVDesc[ch_no*RING_SIZE+j];
	
	pMblk = (M_BLK_ID)cp->vcc[ch_no].tx_buf[j];

	while((j!=cp->vcc[ch_no].TV.desc_pw)&&(!(TVDesc->CMD&OWN))){

		cp->vcc[ch_no].stat.tx_desc_ok_cnt++;
		cp->vcc[ch_no].stat.tx_byte_cnt += TVDesc->LEN;
		#ifdef uClinux
		cp->net_stats.tx_bytes += TVDesc->LEN;
		#endif
		/* if mblk exists and this descriptor is not a 1483 header, free this mblk */
		if ((pMblk!=NULL)&&(TVDesc->START_ADDR!=(UINT32)(&FrameHeader_1483[encap_mode]))){
			netMblkClChainFree(pMblk);
			cp->vcc[ch_no].stat.tx_buf_free++;
		#ifdef uClinux
			cp->vcc[ch_no].tx_buf[j]=(UINT32)NULL;
		#endif
		}
		if(TVDesc->CMD&LS){/* one packet has been successfully transmmited */
			cp->vcc[ch_no].stat.tx_pkt_ok_cnt++;
		#ifdef uClinux
			cp->net_stats.tx_packets++;
		#endif
		}
		TVDesc->RSVD1=0x00000000;
		TVDesc->START_ADDR=0x00000000;
		j++;
		j%=SAR_TX_DESC_NUM;

		TVDesc=&cp->TVDesc[ch_no*RING_SIZE+j];
		pMblk = (M_BLK_ID)cp->vcc[ch_no].tx_buf[j];
	}
	cp->vcc[ch_no].TV.desc_pf=j;
	return TRUE;

}




void FreeVcBuff(SAR_END_DEVICE *cp, int8 ch_no){

	int				i;
	M_BLK_ID		pMblk;	
	

	if (ch_no>17)
		return;

	/* Free RX Descriptors */ 
	for(i=0;i<SAR_RX_DESC_NUM;i++){

		if(ch_no==OAM_CH_NO){

			if(cp->RVDesc[ch_no*RING_SIZE+i].START_ADDR !=(UINT32)NULL){
				#ifdef uClinux
				free((void *)(cp->RVDesc[ch_no*RING_SIZE+i].START_ADDR^0x20000000));
				#endif
				#ifdef vxWorks
				pMblk = (M_BLK_ID)cp->vcc[ch_no].rx_buf[i];
				netMblkClChainFree(pMblk);
				#endif
			}

			if(cp->TVDesc[ch_no*RING_SIZE+i].START_ADDR !=(UINT32)NULL){
				#ifdef uClinux
				free((void *)(cp->RVDesc[ch_no*RING_SIZE+i].START_ADDR^0x20000000));
				#endif
				#ifdef vcWorks
				pMblk = (M_BLK_ID)cp->vcc[ch_no].tx_buf[i];
				netMblkClChainFree(pMblk);
				#endif
			}
						
			/* Clear OWN and EOR bit */
			cp->RVDesc[ch_no*RING_SIZE+i].STS=0x0000;
			cp->TVDesc[ch_no*RING_SIZE+i].CMD=0x0000;

		}else{

			pMblk=(M_BLK_ID)cp->vcc[ch_no].rx_buf[i];
			if(pMblk!=(M_BLK_ID)NULL)
				netMblkClChainFree(pMblk);

			pMblk=(M_BLK_ID)cp->vcc[ch_no].tx_buf[i];
			if(pMblk!=(M_BLK_ID)NULL)
				netMblkClChainFree(pMblk);

			/* Clear this Mblk */
			cp->vcc[ch_no].rx_buf[i]=(UINT32)NULL;
			cp->vcc[ch_no].tx_buf[i]=(UINT32)NULL;

			cp->RVDesc[ch_no*RING_SIZE+i].STS=0x0000;
			cp->TVDesc[ch_no*RING_SIZE+i].CMD=0x0000;



		}
	}

	cp->vcc[ch_no].RV.desc_pr =0;
	cp->vcc[ch_no].RV.desc_pc =0;
	cp->vcc[ch_no].RV.desc_pa =0;
	cp->vcc[ch_no].TV.desc_pf =0;
	cp->vcc[ch_no].TV.desc_pc =0;
	cp->vcc[ch_no].TV.desc_pw =0;

	return;
}


void AllocVcBuff(SAR_END_DEVICE *cp, int8 ch_no){

	int				i;
	M_BLK_ID		pMblk;	

	/*  save buffer size to this channel (using same size) */
	if(ch_no==OAM_CH_NO)	cp->vcc[ch_no].TV.buffer_size=SAR_OAM_Buffer_Size;
	else						cp->vcc[ch_no].TV.buffer_size=SAR_TX_Buffer_Size;
	
	if(ch_no==OAM_CH_NO)	cp->vcc[ch_no].RV.buffer_size=SAR_OAM_Buffer_Size;
	else						cp->vcc[ch_no].RV.buffer_size=SAR_RX_Buffer_Size;
	
	if (ch_no>17)
		return;

	/*  initialize TX Descriptors: clear CMD and set EOR*/	
	for(i=0;i<SAR_TX_DESC_NUM;i++){
		if(ch_no==OAM_CH_NO){
		    if(i==(SAR_TX_DESC_NUM-1))		
			cp->TODesc[i].CMD=0x4000;
		    else	/* EOR */
			cp->TODesc[i].CMD=0x0000;
		} else {
		    if(i==(SAR_TX_DESC_NUM-1))		
			cp->TVDesc[ch_no*RING_SIZE+i].CMD=0x4000;
		    else	/* EOR */
			cp->TVDesc[ch_no*RING_SIZE+i].CMD=0x0000;
		};
				
	}

	/* initialize RX Descriptors */ 
	for(i=0;i<SAR_RX_DESC_NUM;i++){

		if(ch_no==OAM_CH_NO){
			#ifdef uClinux
			cp->RODesc[i].START_ADDR = (u32)malloc(SAR_OAM_Buffer_Size);
			#endif
			
			#ifdef vxWorks
 		   	pMblk = netTupleGet(cp->end.pNetPool, SAR_OAM_Buffer_Size, M_DONTWAIT, MT_DATA, FALSE);
			cp->RODesc[i].START_ADDR = (UINT32)pMblk->mBlkHdr.mData;
			#endif
			
			if(cp->RODesc[i].START_ADDR==(UINT32)NULL)		goto free_desc;
			
			cp->RODesc[i].START_ADDR |= UNCACHE_MASK;
			cp->RODesc[i].LEN = SAR_OAM_Buffer_Size;
			cp->vcc[ch_no].rx_buf[i]=(UINT32)pMblk;

			/* set OWN and EOR bit */
			if(i==(SAR_RX_DESC_NUM-1))
				cp->RODesc[i].STS=0xC000;
			else
				cp->RODesc[i].STS=0x8000;		
			
			cp->vcc[ch_no].RV.desc_pa ++;
			cp->vcc[ch_no].RV.desc_pa %= SAR_RX_DESC_NUM;				

		}else{
		
			/* Get a Buffer */
			#ifdef uClinux
			pMblk = netTupleGet(SAR_RX_Buffer_Size);
			#endif
			#ifdef vxWorks
		   	pMblk = netTupleGet(cp->end.pNetPool, SAR_RX_Buffer_Size, M_DONTWAIT, MT_DATA, FALSE);
			#endif
			
			if (pMblk==NULL)
			{
				printf("Error Could Not Get A Buffer..\n");
				goto free_desc;
			} else
				cp->vcc[ch_no].stat.rx_buf_alloc++;			
	
			/*  allocate memory for RX descriptors */
			cp->RVDesc[ch_no*RING_SIZE+i].LEN=SAR_RX_Buffer_Size;
			
			#ifdef uClinux
			cp->RVDesc[ch_no*RING_SIZE+i].START_ADDR=(UINT32)pMblk->data|UNCACHE_MASK;
			#endif
			#ifdef vxWorks
			cp->RVDesc[ch_no*RING_SIZE+i].START_ADDR=(UINT32)pMblk->mBlkHdr.mData|UNCACHE_MASK;
			#endif

			/* record this skb */
			cp->vcc[ch_no].rx_buf[i]=(UINT32)pMblk;

			/* set OWN and EOR bit */
			if(i==(SAR_RX_DESC_NUM-1))
				cp->RVDesc[ch_no*RING_SIZE+i].STS=0xC000;
			else
				cp->RVDesc[ch_no*RING_SIZE+i].STS=0x8000;		
			
			cp->vcc[ch_no].RV.desc_pa ++;
			cp->vcc[ch_no].RV.desc_pa %= SAR_RX_DESC_NUM;			
		}
	}

	if(cp->CNFG_Reg&IERBF)
		cp->vcc[ch_no].RV.desc_pa =(cp->vcc[ch_no].RV.desc_pa + SAR_RX_DESC_NUM -1) % SAR_RX_DESC_NUM;

	return;

free_desc:
	/* !!! free ring */
	FreeVcBuff(cp, ch_no);
	return;
	
}





void CreateVC(SAR_END_DEVICE *cp, struct SAR_IOCTL_CFG *cfg)
{

	int8	ch_no, encap_mode;
	
	ch_no = cfg->ch_no;

	cp->vcc[ch_no].ch_no		= cfg->ch_no;
	cp->vcc[ch_no].vpi		= cfg->vpi;
	cp->vcc[ch_no].vci		= cfg->vci;
	cp->vcc[ch_no].rfc		= cfg->rfc;
	cp->vcc[ch_no].framing	= cfg->framing;
	//cp->vcc[ch_no].loopback	= cfg->loopback;
	memcpy(&cp->vcc[ch_no].QoS, &cfg->QoS, sizeof(Traffic_Manage));	

	if(cp->vcc[ch_no].created == VC_NOT_CREATED){
		AllocVcBuff(cp, ch_no);
		memset(&cp->vcc[ch_no].stat, 0, sizeof(ch_stat));
	}

	SetVpiVci(cfg->vpi, cfg->vci, ch_no);	/* Set Vpi, Vci to Rx */


	Enable_AAL5(ch_no);
	Enable_ATRLREN(ch_no);

	SetQoS(ch_no, cfg->QoS.Type);			/* Set Scheduler Option */
	if (cfg->QoS.Type == QoS_UBR)  //default for UBR rate, hrchen 4/1/04'
	    SetPCR(ch_no, 7600);			/*  Set Peak Cell Rate */
	else
	    SetPCR(ch_no, cfg->QoS.PCR);			/*  Set Peak Cell Rate */
	SetSCR(ch_no, cfg->QoS.SCR);
	SetMBS(ch_no, cfg->QoS.MBS);
	

	/* Now For 1483 only */
	encap_mode = cp->vcc[ch_no].rfc*2+cp->vcc[ch_no].framing;

	/* if 1483 Bridge Mode, header(10)+Eth(14) = 24 makes the TCP/IP just on the 4-byte boundary,
		Word Insertion is not needed */
	if(cp->vcc[ch_no].rfc == RFC1483_BRIDGED)
		Disable_Word_Insert(ch_no);
	else if(cp->vcc[ch_no].rfc == RFC1483_ROUTED)
		Enable_Word_Insert(ch_no);

	
#ifndef SAR_CRC_GEN
		cp->vcc[ch_no].TV.Ether_Offset_Value=No_Offset;
#else
		cp->vcc[ch_no].TV.Ether_Offset_Value=FrameHeaderSize_1483[encap_mode];
#endif

	Enable_rx_ch(ch_no);
	Enable_tx_ch(ch_no);

	cp->vcc[ch_no].created = VC_CREATED;

	return;
}



void DeleteVC(SAR_END_DEVICE *cp, struct SAR_IOCTL_CFG *cfg){
	
	int8	ch_no;
	
	ch_no = cfg->ch_no;

	cp->vcc[ch_no].ch_no		= cfg->ch_no;
	//cp->vcc[ch_no].vpi		= cfg.vpi;
	//cp->vcc[ch_no].vci		= cfg.vci;
	//cp->vcc[ch_no].rfc		= cfg.rfc;
	//cp->vcc[ch_no].framing	= cfg.framing;
	//memcpy(&cp->vcc[ch_no].QoS, &cfg->QoS, sizeof(Traffic_Manage));	

	FreeVcBuff(cp, ch_no);


	//SetVpiVci(0, 0, ch_no);	/* Set Vpi, Vci to Rx */

	
	//Enable_AAL5(ch_no);
	//Disable_ATRLREN(ch_no);

	//SetQoS(ch_no, 0);			/* Set Scheduler Option */
	//SetPCR(ch_no, 0);			/*  Set Peak Cell Rate */


	/* Now For 1483 only */
	//encap_mode = cp->vcc[ch_no].rfc*2+cp->vcc[ch_no].framing;

	/* if 1483 Bridge Mode, header(10)+Eth(14) = 24 makes the TCP/IP just on the 4-byte boundary,
		Word Insertion is not needed */
	//if(cp->vcc[ch_no].rfc == RFC1483_BRIDGED)
	//	Disable_Word_Insert(ch_no);
	//else if(cp->vcc[ch_no].rfc == RFC1483_ROUTED)
	//	Enable_Word_Insert(ch_no);

	
//	#ifndef SAR_CRC_GEN
//		cp->vcc[ch_no].TV.Ether_Offset_Value=No_Offset;
//#else
//		cp->vcc[ch_no].TV.Ether_Offset_Value=FrameHeaderSize_1483[encap_mode];
//#endif

	Disable_rx_ch(ch_no);
	Disable_tx_ch(ch_no);

	cp->vcc[ch_no].created = VC_NOT_CREATED;

	return;
}


void	flush_tx_desc(SAR_END_DEVICE *cp){

	int	i,j;
	TV_CMD_DESC	*TVDesc;
	M_BLK_ID	pMblk;	
	
	for(i=0;i<16;i++){
		if(cp->vcc[i].created==VC_CREATED){
			for(j=0;j<SAR_RX_DESC_NUM;j++){
				TVDesc=&cp->TVDesc[i*RING_SIZE+j];
				pMblk = (M_BLK_ID)cp->vcc[i].tx_buf[j];
				if(pMblk!=(M_BLK_ID)NULL)
					netMblkClChainFree(pMblk);
				cp->vcc[i].tx_buf[j]=(UINT32)NULL;
				TVDesc->RSVD1=0x00000000;
				TVDesc->START_ADDR=0x00000000;
			}
			cp->vcc[i].stat.send_cnt=0;
			cp->vcc[i].stat.send_ok=0;
			cp->vcc[i].stat.send_fail=0;
			cp->vcc[i].stat.send_desc_lack=0;
			cp->vcc[i].stat.tx_pkt_ok_cnt=0;
			cp->vcc[i].stat.tx_pkt_fail_cnt=0;
			cp->vcc[i].stat.tx_byte_cnt=0;
			Disable_tx_ch(i);
			Enable_tx_ch(i);
		}
	}
}



#ifdef LoopBack_Test

uint16 ipcsum(uint16 *ptr, uint32 len, uint16 resid) {
	uint32 csum = resid;
	uint32 odd = 0;

	if(len & 1) odd = 1;
	len = len >> 1;
	
	for(;len > 0 ; len--,ptr++) 
		csum += ntohs(*ptr);

	if(odd)
		csum += (*((uint8 *)ptr) <<8) & 0xff00;

	/* take care of 1's complement */
	while(csum >> 16)
		csum = (csum & 0xffff) + (csum >> 16);

	if(csum == 0xffff)	csum = 0;

	return((uint16)csum);
}


void exchange_mac(uint8 *buf){

	char 	tmpMac[6];

	memcpy(tmpMac, buf, 6);
	memcpy(buf, buf+6, 6);
	memcpy(buf+6, tmpMac, 6);	
	return;

}

INT16 Lying_Engine(uint8 *buf, INT16 len){

	char 	broadcast[6]={0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	char		remoteMac[6]={0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
	char 	tmpMac[6], tempIPMac[10], tempIP[4];
	UINT16	*Type, *Opcode, *csum, *Frag, fragment;
	UINT8	*Protocol, *ICMP_Type;


		Type =(UINT16  *)(buf + 12);
		Opcode =(UINT16 *)(buf + 20);
		Protocol = (UINT8 *)(buf + 23);
		ICMP_Type = (UINT8 *)(buf + 34);		
		Frag= (UINT16 *)(buf+ 20);
		fragment = (UINT16)*Frag & 0x1FFF;
		
	if(!memcmp(buf, broadcast, 6)){	/* Broadcast Packet */
		
		if(!memcmp(buf+28, buf+38, 4))	/* Initial ARP, don't lie */
			return len;
		 
		if((*Type==0x0806)&&(*Opcode==0x0001)){ /* this is a ARP Request */
			/* handle Ehternet Header - Mac Address */	
			memcpy(tmpMac, buf+6, 6);
			memcpy(buf+6, remoteMac, 6);
			memcpy(buf, tmpMac, 6);

			/* change IP Opcode to reply */
			*Opcode = 0x0002;
			
			/* ARP IP and Mac Address */
			memcpy(tempIPMac, buf+22, 10);
			memcpy(tempIP, buf+38, 4);	
			memcpy(buf+32, tempIPMac, 10);
			memcpy(buf+22, remoteMac, 6);
			memcpy(buf+28, tempIP, 4);
			/* memset(buf+42, 0x20, 18); */
			return (len+18);
		}
		
	}else {

		if((*Type==0x0800)&&(*Protocol==0x01)&&(*ICMP_Type==0x08)){ /* this is a ICMP Ping Request */

			/* handle Ehternet Header - exchange Mac Address */	
			memcpy(tmpMac, buf, 6);
			memcpy(buf, buf+6, 6);
			memcpy(buf+6, tmpMac, 6);

			/* ip check sum */
			csum = (UINT16 *)(buf + 24);
			*csum = 0;
			*csum = (~ipcsum((UINT16 *)(buf+14), 20, 0));

			/* exchange IP Address */
			memcpy(tempIP, buf+26, 4);
			memcpy(buf+26, buf+30, 4);
			memcpy(buf+30, tempIP, 4);

			/* ICMP header */
			*ICMP_Type = 0x00;

			/* ICMP checksum */
			csum = (UINT16 *)(buf + 36);
			/* *csum = 0;*/
			*csum += 0x0800;/*(~ipcsum((UINT16 *)(buf+34), len-34, 0));*/
			
		}else if ((*Type==0x0800)&&(*Protocol==0x01)&&(fragment!=0)){ /* this is a ICMP Ping Request, fragment packet */

			/* handle Ehternet Header - exchange Mac Address */	
			memcpy(tmpMac, buf, 6);
			memcpy(buf, buf+6, 6);
			memcpy(buf+6, tmpMac, 6);

			/* ip check sum */
			csum = (UINT16 *)(buf + 24);
			*csum = 0;
			*csum = (~ipcsum((UINT16 *)(buf+14), 20, 0));

			/* exchange IP Address */
			memcpy(tempIP, buf+26, 4);
			memcpy(buf+26, buf+30, 4);
			memcpy(buf+30, tempIP, 4);

		}

		if(len<64)	len=64;
		return len;
	}
	/* exchange_mac(buf); */
	return len;
}

#endif

/////////////////////////////////////////////for OAM
#ifdef ATM_OAM

extern int OAMFSendACell(unsigned char *pCell);	/* OAM to SAR driver interface */

#define EMPTY	0
#define OCCUPY	1

#ifndef NULL
#define NULL		0
#endif

#define ATM_OAM_TRACE	printk
#define ATM_OAM_DEBUG

OAMF5 OAMF5_info;

static const unsigned char f5_llid_all_0[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static const unsigned char f5_llid_all_1[] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

/* OAM F5 */

unsigned long OAMF5GetTimeStamp(void)
{
#if RTOS == VXWORKS
	return (unsigned long)(jiffies*(1000/HZ));
#endif
}

int OAMF5StartTimer(OAMF5Timer* timer, VOIDFUNCPTR pFunc)
{
	if(timer->occupied == OCCUPY)
	{
		ATM_OAM_TRACE("OAMF5StartTimer (timer->occupied == OCCUPY)\n");
		return 0;
	}

	init_timer(&timer->timer);
        timer->timer.expires = jiffies + timer->ulTimeout;
        timer->timer.function = pFunc;
        timer->timer.data = (unsigned long)(timer);
	add_timer(&timer->timer);

	timer->occupied = OCCUPY;	
	ATM_OAM_DEBUG("OAMF5StartTimer ok\n");

	return 1;
}

int OAMF5StopTimer(OAMF5Timer* timer)
{
	if(timer->occupied == EMPTY)
	{
		ATM_OAM_TRACE("OAMF5StopTimer (timer->occupied == EMPTY)\n");
		return 0;
	}

	del_timer(&timer->timer);

	timer->occupied = EMPTY;	
	ATM_OAM_DEBUG("OAMF5StopTimer ok\n");

	return 1;
}

void OAMF5Init(unsigned short vpi, unsigned short vci, OAMF5 *pOAMF5)
{
	int i, j;
	unsigned char gfc = 0, clp = 0, ucPTI;

	pOAMF5->VPI = vpi;
	pOAMF5->VCI = vci;
	
	/* OAM FM --LB */
	pOAMF5->OAMF5lFaultLBState = FAULT_LB_IDLE;			// loopback state: idle
	pOAMF5->OAMF5ulFaultLBTag = 0;						// loopback correlation tag
	for (i = 0; i < OAM_LB_LLID_SIZE; i++) {
		pOAMF5->OAMF5ucCPID[i] = 0xFF;					// connection point ID
	}

	// initialize LB table for TMN transmission only
	for (i = 0; i < LB_ALL_NODE_SIZE+LB_TABLE_SIZE; i++) {
		pOAMF5->OAMF5LBList[i].tag = 0;
		for (j = 0; j < OAM_LB_LLID_SIZE; j++)
		{
			pOAMF5->OAMF5LBList[i].locationID[j] = 0;
			pOAMF5->LBRXList[i].locationID[j] = 0;
		}
		pOAMF5->OAMF5LBList[i].scope = 0;
		pOAMF5->OAMF5LBList[i].count = 0;
		pOAMF5->OAMF5LBList[i].status = FAULT_LB_IDLE;
		pOAMF5->LBRXList[i].status = EMPTY;
		pOAMF5->LBRXList[i].sur_des_count = 0;

		// initialize LB timer
		pOAMF5->OAMF5LBList[i].timer.pOAM = (void *)pOAMF5;
		pOAMF5->OAMF5LBList[i].timer.OAMFunctionType = FAULT_LB;
		pOAMF5->OAMF5LBList[i].timer.OAMUserData = i;
		pOAMF5->OAMF5LBList[i].timer.oneShot = 1;
		pOAMF5->OAMF5LBList[i].timer.occupied = EMPTY;
		pOAMF5->OAMF5LBList[i].timer.ulTimeout = OAM_LB_WAIT_TIME;		
	}

	pOAMF5->OAMF5ulFaultLBTxGoodCell = 0;
	pOAMF5->OAMF5ulFaultLBRxGoodCell = 0;

	ATM_OAM_DEBUG("ATM OAM F5 initialized.\n");
}

void OAMF5ChangeState(OAMF5 *pOAMF5)
{
	int i;
	OAMF5_LB_INFO *pLBInfo = NULL;

	// FM LB state
	for (i = 0, pLBInfo = &pOAMF5->OAMF5LBList[0]; i < LB_ALL_NODE_SIZE+LB_TABLE_SIZE; i++) {
		if(pLBInfo[i].status == FAULT_LB_WAITING)
			return;
		if (i == 0) i = LB_ALL_NODE_SIZE-1;
	}
	pOAMF5->OAMF5lFaultLBState = FAULT_LB_IDLE;
	
	ATM_OAM_DEBUG("OAMF5ChangeState >> IDLE\n");	
}

int OAMF5SetVpiVci(unsigned short vpi, unsigned short vci, OAMF5 *pOAMF5)
{
	if (pOAMF5->OAMF5lFaultLBState != FAULT_LB_IDLE) return 0;
	pOAMF5->VPI = vpi;
	pOAMF5->VCI = vci;
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
//
//		CAtmOAMF5::SetLBID()
//
//		Input		: ucIDType -- 0: all connection point with location ID enable
//								  1: specified by TNM (telecommunication network management)
//										country code  & network ID (4 bytes) + operator specific information (11 bytes)
//								  2: specified by TNM (telecommunication network management)
//										country code + network ID (4 bytes)
//								  3: specified by TNM (telecommunication network management)
//										partial NSAP based coding structure
//								  0x6A: no loopback
//								  0xFF: endpoint
//								  others: reserved for future use
//					  pDesID: pointer of the destination ID array
//					  pID: pointer of the specified location ID array
//
//		Output		: None
//
//		Author		: Network Group
//
//		History		:
//
//		Design Notes: Prepare loopback ID information for transmission path
//		I.610, Table 5, 10.2.3
//
/////////////////////////////////////////////////////////////////////////////

void OAMF5SetLBID(unsigned char ucIDType, unsigned char *pDesID, unsigned char *pID)
{
	int i;

	switch (ucIDType) {
	case 0:
		// all 0's: all CP with location ID option enable
	case 0x6A:
		// all 0x6A: not specify CP, no loopback is perform
	case 0xFF:
		// all 1's: enpoint
		for (i = 0; i < OAM_LB_LLID_SIZE; i++)
			pDesID[i] = ucIDType;
		break;
	case 2:
		pDesID[0] = ucIDType;
		// specified by telecommunication network management (TNM)
		for (i = 0; i < 4; i++)
			pDesID[i+1] = pID[i];
		for (; i < 15; i++)
			pDesID[i+1] = 0x6A;
		break;
	case 1:
	case 3:
		pDesID[0] = ucIDType;
		// specified by telecommunication network management (TNM)
		for (i = 0; i < OAM_LB_LLID_SIZE-1; i++)
			pDesID[i+1] = pID[i];
		break;
	}
}

int OAMF5TMNSetCPID(unsigned char ucLocationType, unsigned char *pCPID, OAMF5 *pOAMF5)
{
	if (pOAMF5->OAMF5lFaultLBState != FAULT_LB_IDLE) return 0;
	OAMF5SetLBID(ucLocationType, pOAMF5->OAMF5ucCPID, pCPID);
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
//
//		CAtmOAMF5::SendACell()
//
//		Input		: pointer of a cell, channel (ATM0/ATM1)
//
//		Output		: 0/1
//
//		Author		: Network Group
//
//		History		:
//
//		Design Notes: Send a OAM cell to queue
//
/////////////////////////////////////////////////////////////////////////////

int  OAMF5SendACell(unsigned char *pOamCell)
{
	unsigned char *dupCell;
	ATM_OAM_DEBUG("Into the OAMF5SendACell\n");

	dupCell = (unsigned char *) malloc(OAM_CELL_SIZE);

	if(dupCell == NULL)
	{
		ATM_OAM_TRACE("OAM malloc fail!\n");
		return 0;
	}
	
	memcpy(dupCell, pOamCell, OAM_CELL_SIZE);

#if 0
	{
		unsigned int i;
		for(i=0;i<52;i+=13)
		{
			ATM_OAM_DEBUG("Cell(%d) %x %x %x %x %x %x %x %x %x %x %x %x %x\n"
				,i
				,pOamCell[i] ,pOamCell[i+1] ,pOamCell[i+2] ,pOamCell[i+3] ,pOamCell[i+4] ,pOamCell[i+5]
				,pOamCell[i+6] ,pOamCell[i+7] ,pOamCell[i+8] ,pOamCell[i+9] ,pOamCell[i+10] ,pOamCell[i+11],pOamCell[i+12]);
		}
	}
#endif

	return OAMFSendACell(dupCell);
}

/////////////////////////////////////////////////////////////////////////////
//
//		CAtmOAMF5::GenLBCell()
//
//		Input		: pointer of OAMF5_LB_INFO
//
//		Output		: NONE
//
//		Author		: Network Group
//
//		History		:
//
//		Design Notes: Generate OAM fault management loopback cell for transmission
//						(excluding HEC and CRC10)
//
/////////////////////////////////////////////////////////////////////////////
void OAMF5GenLBCell(OAMF5_LB_INFO *pLBInfo, OAMF5 *pOAMF5)
{
	unsigned char ucPTI = (pLBInfo->scope)? 5 : 4;
	unsigned char gfc = 0, clp = 0;
	int i;

	ATM_OAM_DEBUG("Into the OAMF5GenLBCell\n");

	// generate cell header & payload
	pLBInfo->cell[OAM_FORMAT_H1] = (unsigned char) ((pOAMF5->VPI >> 4) & 0x0F) | ((gfc << 4) & 0xF0);
	pLBInfo->cell[OAM_FORMAT_H2] = (unsigned char) (((pOAMF5->VPI << 4) & 0x00F0) | (pOAMF5->VCI >> 12));
	pLBInfo->cell[OAM_FORMAT_H3] = (unsigned char) (pOAMF5->VCI >> 4);
	pLBInfo->cell[OAM_FORMAT_H4] = (unsigned char) ((pOAMF5->VCI << 4) & 0x00F0) | ((ucPTI << 1) & 0xE) | (clp & 1);

	pLBInfo->cell[OAM_FORMAT_TYPE] = FAULT_LB;	// OAM type & function
	pLBInfo->cell[OAM_FORMAT_LB_INDICATION] = 1;			// loopback indication bit
	pLBInfo->cell[OAM_FORMAT_LB_TAG] = (unsigned char)(pLBInfo->tag >> 24 & 0xFF);	// loopback correlation tag
	pLBInfo->cell[OAM_FORMAT_LB_TAG+1] = (unsigned char)(pLBInfo->tag >> 16 & 0xFF);	// loopback correlation tag
	pLBInfo->cell[OAM_FORMAT_LB_TAG+2] = (unsigned char)(pLBInfo->tag >> 8 & 0xFF);	// loopback correlation tag
	pLBInfo->cell[OAM_FORMAT_LB_TAG+3] = (unsigned char)(pLBInfo->tag & 0xFF);		// loopback correlation tag

	for (i = 0; i < OAM_LB_LLID_SIZE; i++) {
		pLBInfo->cell[OAM_FORMAT_LB_LLID+i] = pLBInfo->locationID[i];					// loopback location ID
		pLBInfo->cell[OAM_FORMAT_LB_SID+i] = pOAMF5->OAMF5ucCPID[i];					// loopback source ID
	}

	for (i = 0; i < OAM_LB_UNUSE; i++)
		pLBInfo->cell[OAM_FORMAT_LB_UNUSE+i] = 0x6A;

	ATM_OAM_DEBUG("Leave the OAMF5GenLBCell\n");	
}

OAMF5_LB_INFO *OAMF5FindLBInfo(unsigned long tag, OAMF5 *pOAMF5)
{
	int i;

	for (i = 0; i < LB_ALL_NODE_SIZE+LB_TABLE_SIZE; i++) {
		if (tag == pOAMF5->OAMF5LBList[i].tag && pOAMF5->OAMF5LBList[i].status != FAULT_LB_IDLE)
			return &(pOAMF5->OAMF5LBList[i]);
		if (i == 0) i = LB_ALL_NODE_SIZE-1;
	}
	return NULL;
}

OAMF5_LBRX_INFO *OAMF5FindLBRXInfo(OAMF5 *pOAMF5)
{
	return &(pOAMF5->LBRXList[0]);
}

void OAMF5TMNLBTimeoutFunc(struct timer_list *id, int arg)
{
	OAMF5Timer *pTimer;
	OAMF5 *pOAMF5;
	
	ATM_OAM_DEBUG("Into the OAMF5TMNLBTimeoutFunc %x\n", arg);

	pTimer = (OAMF5Timer*) arg;
	if(pTimer)
	{
		OAMF5_LB_INFO *pLBInfo = NULL;
		int i;
		pOAMF5 = pTimer->pOAM;
		pTimer->occupied = EMPTY;

		if(pTimer->OAMFunctionType != FAULT_LB)
		{
			ATM_OAM_TRACE("OAMF5TMNLBTimeoutFunc (pTimer->OAMFunctionType != FAULT_LB)\n");
			return;
		}

		pLBInfo = &pOAMF5->OAMF5LBList[pTimer->OAMUserData];

		// double check
		if(&(pLBInfo->timer) != pTimer)
		{
			ATM_OAM_TRACE("OAMF5TMNLBTimeoutFunc (&(pLBInfo->timer) != pTimer)\n");
			return;
		}

		// timestamp
		pLBInfo->rtt = OAMF5GetTimeStamp() - pLBInfo->timestamp;

		// return to idle state
		pLBInfo->status = FAULT_LB_STOP;
		OAMF5ChangeState(pOAMF5);
	}

	ATM_OAM_DEBUG("Leave the OAMF5TMNLBTimeoutFunc\n");	
}

/////////////////////////////////////////////////////////////////////////////
//
//		CAtmOAMF5::TMNTxLBStart()
//
//		Input		: Scope (1/0:end/segment), ATM channel, location ID
//
//		Output		: fail (0)/OK (1)
//
//		Author		: Network Group
//
//		History		:
//
//		Design Notes: Generate an OAM fault management loopback cell
//					  Send out to PHY layer
//					  Start waiting timer and adjust internal state
//					  return true -- cell is ready to be sent
//					  return false -- no cell available
//
/////////////////////////////////////////////////////////////////////////////
int OAMF5TMNTxLBStart(unsigned char Scope, unsigned char *pLocID, unsigned long *Tag, OAMF5 *pOAMF5)
{
	OAMF5_LB_INFO *pLBInfo = NULL;

	int i, j;

	ATM_OAM_DEBUG("Into the OAMF5TMNTxLBStart\n");

	// search for an empty OAMF5_LB_INFO from table
	if (!pLocID[0]) {
		if (pOAMF5->OAMF5LBList[0].status == FAULT_LB_WAITING) {
			ATM_OAM_TRACE("OAMF5TMNTxLBStart (pOAMF5->OAMF5LBList[0].status == FAULT_LB_WAITING)\n");
			return 0;
		}
		pLBInfo = &pOAMF5->OAMF5LBList[0];

		// clean up all-0 table
		for (i = 0; i < LB_ALL_NODE_SIZE; i++) {
			pLBInfo[i].all0_status = FAULT_LB_IDLE;
			pLBInfo[i].count = 0;
			pLBInfo[i].tag = 0;
			for (j = 0; j < OAM_LB_LLID_SIZE; j++)
				pLBInfo[i].locationID[j] = 0;
			pLBInfo[i].timestamp = 0;
			pLBInfo[i].rtt = 0;
		}
	} else {
		for (i = LB_ALL_NODE_SIZE; i < LB_ALL_NODE_SIZE+LB_TABLE_SIZE; i++) {
			if (pOAMF5->OAMF5LBList[i].status == FAULT_LB_IDLE) {
				pLBInfo = &pOAMF5->OAMF5LBList[i];
				break;
			}
		}

		// if no more IDLE list, pick a STOP list
		if(pLBInfo == NULL) {
			unsigned int timestamp = 0xFFFFFFFF;
			for (i = LB_ALL_NODE_SIZE; i < LB_ALL_NODE_SIZE+LB_TABLE_SIZE; i++) {
				if ((pOAMF5->OAMF5LBList[i].status == FAULT_LB_STOP) && (pOAMF5->OAMF5LBList[i].timestamp < timestamp)){
					pLBInfo = &pOAMF5->OAMF5LBList[i];
					timestamp = pOAMF5->OAMF5LBList[i].timestamp;
				}
			}
		}
	}

	if (pLBInfo == NULL){
		ATM_OAM_TRACE("OAMF5TMNTxLBStart (pLBInfo == NULL)\n");
		return 0;
	}

	// prepare LB information
//	AtomicSet(&(pLBInfo->status), FAULT_LB_WAITING);	// loopback state: waiting
	pLBInfo->status = FAULT_LB_WAITING;
//	AtomicIncrement((long *)&OAMF5ulFaultLBTag);
	pOAMF5->OAMF5ulFaultLBTag++;
	pLBInfo->tag = pOAMF5->OAMF5ulFaultLBTag-1;
	pLBInfo->scope = Scope;
	pLBInfo->count = 0;

	for (i = 0; i < OAM_LB_LLID_SIZE; i++)
		pLBInfo->locationID[i] = pLocID[i];
	
	OAMF5GenLBCell(pLBInfo, pOAMF5);					// generate OAM LB cell
	OAMF5SendACell(pLBInfo->cell);	// Send out a LB OAM cell
	pOAMF5->OAMF5ulFaultLBTxGoodCell++;

	// adjust internal state
	if (pOAMF5->OAMF5lFaultLBState != FAULT_LB_WAITING)
		pOAMF5->OAMF5lFaultLBState = FAULT_LB_WAITING;

	// timestamp
	pLBInfo->timestamp = OAMF5GetTimeStamp();
	pLBInfo->rtt = 0;
	
	// start timer
	if(OAMF5StartTimer(&(pLBInfo->timer), (VOIDFUNCPTR )OAMF5TMNLBTimeoutFunc) == 0)
	{
		pLBInfo->status = FAULT_LB_IDLE;
		OAMF5ChangeState(pOAMF5);
	}

	*Tag = pLBInfo->tag;

	ATM_OAM_DEBUG("Leave the OAMF5TMNTxLBStart\n");

	return 1;
}

/////////////////////////////////////////////////////////////////////////////
//
//		CAtmOAMF5::TMNTxLBStop()
//
//		Input		: Tag
//
//		Output		: fail (0)/OK (1)
//
//		Author		: Network Group
//
//		History		:
//
//		Design Notes: Stop generating an OAM fault management loopback cell
//					  Cancel timer and adjust internal state
//
/////////////////////////////////////////////////////////////////////////////
int OAMF5TMNTxLBStop(unsigned long Tag, OAMF5 *pOAMF5)
{
	OAMF5_LB_INFO *pLBInfo;
	OAMF5Timer *pLBTimer = NULL;
	OAMF5_LBRX_INFO *pLBRXInfo;

	int i, j;

	ATM_OAM_DEBUG("Into the OAMF5TMNTxLBStop\n");

	if ((pLBInfo = OAMF5FindLBInfo(Tag, pOAMF5)) == NULL) return 0;

	// clear OAM FM LB Rx List
	pLBRXInfo = &(pOAMF5->LBRXList[0]);
	for (i = 0; i < LB_ALL_NODE_SIZE; i++)
	{
		for (j = 0; j < OAM_LB_LLID_SIZE; j++)
		{
			pLBRXInfo[i].locationID[j] = 0;
		}

		pLBRXInfo[i].status = EMPTY;
		pLBRXInfo[i].sur_des_count = 0;
	}

	// stop timer
	OAMF5StopTimer(&(pLBInfo->timer));

	// return to idle state
	pLBInfo->status = FAULT_LB_IDLE;
	OAMF5ChangeState(pOAMF5);
	
	ATM_OAM_DEBUG("Out the OAMF5TMNTxLBStop\n");
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
//
//		CAtmOAMF5::ProcessRxLBCell()
//
//		Input		: pointer of a cell received
//
//		Output		: false/true (discard/loopback)
//
//		Author		: Network Group
//
//		History		:
//
//		Design Notes: Verify current loopback state
//					  Verify OAM data
//					  Please refer to figure C.1/I.610
//
/////////////////////////////////////////////////////////////////////////////
int OAMF5ProcessRxLBCell(unsigned char *pOamCell, OAMF5 *pOAMF5)
{
	unsigned long PTI = (pOamCell[OAM_FORMAT_H4] & 0xE);
	unsigned char ucLBIndicator = (pOamCell[OAM_FORMAT_LB_INDICATION] & 1), TempIndex = EMPTY;
	int i, j;
	// assumed CRC-10 is correct, ASIC will handle CRC-10

	ATM_OAM_DEBUG("Into the OAMF5ProcessRxLBCell\n");		

	// handle FM LB request	
	if ((PTI == 8 || PTI == 0xA) && ucLBIndicator) {
		// segment/end-to-end OAM F5 FM LB
		// it's segment endpoint or connection endpoint

		char Match1 = FALSE, Match2 = FALSE, Match3 = FALSE;		

		// verify LLID
		if (pOamCell[OAM_FORMAT_LB_LLID] == 0x6A)
		{
			ATM_OAM_DEBUG("OAMF5ProcessRxLBCell LLID 6A\n");
			return 0; // no loopback
		}

		if (memcmp(&pOamCell[OAM_FORMAT_LB_LLID], pOAMF5->OAMF5ucCPID, OAM_LB_LLID_SIZE) == 0)
			Match1 = TRUE;	// match connection point ID
		else if (memcmp(&pOamCell[OAM_FORMAT_LB_LLID], f5_llid_all_0, OAM_LB_LLID_SIZE) == 0)
			Match2 = TRUE;	// all connection point ID
		else if (memcmp(&pOamCell[OAM_FORMAT_LB_LLID], f5_llid_all_1, OAM_LB_LLID_SIZE) == 0)
			Match3 = TRUE;	// end-to-end loopback

		if (PTI == 0xA)		// is this end-to-end LB, ????
			Match2 = FALSE;

		if (Match1 || Match2 || Match3) { // Loop cell back
			char SidFound = FALSE;
			OAMF5_LBRX_INFO *pLBRXInfo;

			pLBRXInfo = &(pOAMF5->LBRXList[0]);
		
			// reset loopback indication		
			pOamCell[OAM_FORMAT_LB_INDICATION] &= 0xFE;
			// set LLID		
			memcpy(&pOamCell[OAM_FORMAT_LB_LLID], pOAMF5->OAMF5ucCPID, OAM_LB_LLID_SIZE);

			// save source ID
			// ITEX's implementation is save LLID, I think they are wrong. by Dick Tam
			for (i = 0; i < LB_ALL_NODE_SIZE; i++) {
				if((pLBRXInfo[i].status == OCCUPY) &&
					(memcmp(&pOamCell[OAM_FORMAT_LB_SID], pLBRXInfo[i].locationID, OAM_LB_SRCID_SIZE) == 0)) {

					SidFound = TRUE;
					pLBRXInfo[i].sur_des_count++;
					break;
				}					
			}

			if(!SidFound) {
				for (i = 0; i < LB_ALL_NODE_SIZE; i++) {
					if(pLBRXInfo[i].status == EMPTY) {
						pLBRXInfo[i].status = OCCUPY;
						memcpy(pLBRXInfo[i].locationID, &pOamCell[OAM_FORMAT_LB_SID],  OAM_LB_SRCID_SIZE);
						pLBRXInfo[i].sur_des_count++;
						break;
					}
				}
			}

			// debug message
			for (i = 0; i < LB_ALL_NODE_SIZE; i++)
			{
				if (pLBRXInfo[i].sur_des_count != 0)
				{
					ATM_OAM_DEBUG("The Reomte ID =");
					for (j = 0; j < OAM_LB_SRCID_SIZE; j++)
						ATM_OAM_DEBUG("%x", pLBRXInfo[i].locationID[j]);
					ATM_OAM_DEBUG(", The Rx Remote Count = %d\n", pLBRXInfo[i].sur_des_count);
				}
			}

			// Do not have to change cell header
			// cell is ready to be sent out at the moment
			return 1;
		}
	}

	// handle FM LB response	
	if (pOAMF5->OAMF5lFaultLBState == FAULT_LB_WAITING) {
		if ((PTI == 8 || PTI == 0xA) && (!ucLBIndicator)){
			OAMF5_LB_INFO *pLBInfo = NULL;
			unsigned long LBTag;
			
			// verify loopback source ID
			if (memcmp(&pOamCell[OAM_FORMAT_LB_SID], pOAMF5->OAMF5ucCPID, OAM_LB_SRCID_SIZE) != 0)
				return 0;

			// verify correlation tag value
			LBTag = (unsigned long)(pOamCell[OAM_FORMAT_LB_TAG]); 
			LBTag = (LBTag << 8) | ((unsigned long)(pOamCell[OAM_FORMAT_LB_TAG+1])); 
			LBTag = (LBTag << 8) | ((unsigned long)(pOamCell[OAM_FORMAT_LB_TAG+2])); 
			LBTag = (LBTag << 8) | ((unsigned long)(pOamCell[OAM_FORMAT_LB_TAG+3]));
				
			if ((pLBInfo = OAMF5FindLBInfo(LBTag, pOAMF5)) != NULL) {

				// loopback successful
				if (pLBInfo->status == FAULT_LB_WAITING) {

					if (pLBInfo == &pOAMF5->OAMF5LBList[0])
					{
						// all zero LB, don't stop timer
						// wait 5 second to collect all response

						// save LLID to pLBInfo->locationID, handle counter
						for (i = 0; i < LB_ALL_NODE_SIZE; i++)
						{
							if (pLBInfo[i].all0_status == FAULT_LB_WAITING) {

								// match LLID
								if (memcmp(&pOamCell[OAM_FORMAT_LB_LLID], pLBInfo[i].locationID, OAM_LB_LLID_SIZE) == 0)
								{
									pLBInfo[i].count++;
									return 0;
								}

							} else if (pLBInfo[i].all0_status == FAULT_LB_IDLE) {

								// save LLID
								pLBInfo[i].all0_status = FAULT_LB_WAITING;
								pLBInfo[i].count++;
								memcpy(pLBInfo[i].locationID, &pOamCell[OAM_FORMAT_LB_LLID],  OAM_LB_LLID_SIZE);

								// timestamp
								pLBInfo[i].rtt = OAMF5GetTimeStamp() - pLBInfo->timestamp;

								return 0;
							}
						}

						// more than LB_ALL_NODE_SIZE reponse, how to handle it ?

					} else {
						pLBInfo->count++;

						// timestamp
						pLBInfo->rtt = OAMF5GetTimeStamp() - pLBInfo->timestamp;

						// stop timer
						OAMF5StopTimer(&(pLBInfo->timer));					

						// save LLID
						memcpy(pLBInfo[i].locationID, &pOamCell[OAM_FORMAT_LB_LLID],  OAM_LB_LLID_SIZE);

						// return to idle state
						pLBInfo->status = FAULT_LB_STOP;
						OAMF5ChangeState(pOAMF5);
					}

					// Should tell TMN that LB successes.
					// TODO ... 
					// (or TMN may monitor Variable ulFaultLBRxGoodCell instead.)
				}
			}
		}
	}

	ATM_OAM_DEBUG("Leave the OAMF5ProcessRxLBCell\n");		

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//
//		OAMRxF5Cell
//
//		Input		: pointer of an OAM cell
//
//		Output		: None
//
//		Author		: Network Group
//
//		History		:
//
//		Design Notes: OAM F5 handler routine
//
/////////////////////////////////////////////////////////////////////////////
void OAMRxF5Cell(unsigned char *pOamCell, OAMF5 *pOAMF5)
{
	ATM_OAM_DEBUG("Into the RxOAMF5Cell %d\n", pOamCell[OAM_FORMAT_TYPE]);

#if 0
	{
		unsigned int i;
		for(i=0;i<52;i+=13)
		{
			ATM_OAM_DEBUG("Cell(%d) %x %x %x %x %x %x %x %x %x %x %x %x %x\n"
				,i
				,pOamCell[i] ,pOamCell[i+1] ,pOamCell[i+2] ,pOamCell[i+3] ,pOamCell[i+4] ,pOamCell[i+5]
				,pOamCell[i+6] ,pOamCell[i+7] ,pOamCell[i+8] ,pOamCell[i+9] ,pOamCell[i+10] ,pOamCell[i+11],pOamCell[i+12]);
		}
	}
#endif

 	switch (pOamCell[OAM_FORMAT_TYPE]) {

#if 0
	case FAULT_AIS:
	case FAULT_RDI:
	case FAULT_CC:
	case PERFORMANCE_FPM:
	case PERFORMANCE_BACKWARD_REPORT:
	case ACT_DEACT_FPM:
	case ACT_DEACT_FPM_BR:
	case ACT_DEACT_CC:
	case APS_GROUP_PROTECT:
	case APS_INDIVIDUAL_PROTECT:
	case SYSTEM_MANAGEMENT:
#endif


	case FAULT_LB:
		if  (pOAMF5 != NULL)
		{
			if (OAMF5ProcessRxLBCell(pOamCell, pOAMF5))
			{
				OAMF5SendACell(pOamCell);	// send LB cell out
				pOAMF5->OAMF5ulFaultLBTxGoodCell++;
			}
		}
		pOAMF5->OAMF5ulFaultLBRxGoodCell++;
		break;

	default:
		break;

	}

	ATM_OAM_DEBUG("Leave the OAMRxF5Cell\n");		
}



/* OAM F4 */
OAMF4 OAMF4_info;

#define OAMF4GetTimeStamp OAMF5GetTimeStamp
#define OAMF4StartTimer OAMF5StartTimer
#define OAMF4StopTimer OAMF5StopTimer
#define OAMF4SetLBID OAMF5SetLBID
#define OAMF4SendACell OAMF5SendACell

void OAMF4Init(unsigned short vpi, unsigned short vci, OAMF4 *pOAMF4)
{
	int i, j;
	unsigned char gfc = 0, clp = 0, ucPTI;

	pOAMF4->VPI = vpi;
	pOAMF4->VCI = vci;
	
	/* OAM FM --LB */
	pOAMF4->OAMF4lFaultLBState = FAULT_LB_IDLE;			// loopback state: idle
	pOAMF4->OAMF4ulFaultLBTag = 0;						// loopback correlation tag
	for (i = 0; i < OAM_LB_LLID_SIZE; i++) {
		pOAMF4->OAMF4ucCPID[i] = 0xFF;					// connection point ID
	}

	// initialize LB table for TMN transmission only
	for (i = 0; i < LB_ALL_NODE_SIZE+LB_TABLE_SIZE; i++) {
		pOAMF4->OAMF4LBList[i].tag = 0;
		for (j = 0; j < OAM_LB_LLID_SIZE; j++)
		{
			pOAMF4->OAMF4LBList[i].locationID[j] = 0;
			pOAMF4->LBRXList[i].locationID[j] = 0;
		}
		pOAMF4->OAMF4LBList[i].scope = 0;
		pOAMF4->OAMF4LBList[i].count = 0;
		pOAMF4->OAMF4LBList[i].status = FAULT_LB_IDLE;
		pOAMF4->LBRXList[i].status = EMPTY;
		pOAMF4->LBRXList[i].sur_des_count = 0;

		// initialize LB timer
		pOAMF4->OAMF4LBList[i].timer.pOAM = (void *)pOAMF4;
		pOAMF4->OAMF4LBList[i].timer.OAMFunctionType = FAULT_LB;
		pOAMF4->OAMF4LBList[i].timer.OAMUserData = i;
		pOAMF4->OAMF4LBList[i].timer.oneShot = 1;
		pOAMF4->OAMF4LBList[i].timer.occupied = EMPTY;
		pOAMF4->OAMF4LBList[i].timer.ulTimeout = OAM_LB_WAIT_TIME;		
	}

	pOAMF4->OAMF4ulFaultLBTxGoodCell = 0;
	pOAMF4->OAMF4ulFaultLBRxGoodCell = 0;

	ATM_OAM_DEBUG("ATM OAM F4 initialized.\n");
}

void OAMF4ChangeState(OAMF4 *pOAMF4)
{
	int i;
	OAMF4_LB_INFO *pLBInfo = NULL;

	// FM LB state
	for (i = 0, pLBInfo = &pOAMF4->OAMF4LBList[0]; i < LB_ALL_NODE_SIZE+LB_TABLE_SIZE; i++) {
		if(pLBInfo[i].status == FAULT_LB_WAITING)
			return;
		if (i == 0) i = LB_ALL_NODE_SIZE-1;
	}
	pOAMF4->OAMF4lFaultLBState = FAULT_LB_IDLE;
	
	ATM_OAM_DEBUG("OAMF4ChangeState >> IDLE\n");	
}

int OAMF4SetVpiVci(unsigned short vpi, unsigned short vci, OAMF4 *pOAMF4)
{
	if (pOAMF4->OAMF4lFaultLBState != FAULT_LB_IDLE) return 0;
	pOAMF4->VPI = vpi;
	pOAMF4->VCI = vci;
	return 1;
}


int OAMF4TMNSetCPID(unsigned char ucLocationType, unsigned char *pCPID, OAMF4 *pOAMF4)
{
	if (pOAMF4->OAMF4lFaultLBState != FAULT_LB_IDLE) return 0;
	OAMF5SetLBID(ucLocationType, pOAMF4->OAMF4ucCPID, pCPID);
	return 1;
}

void OAMF4GenLBCell(OAMF5_LB_INFO *pLBInfo, OAMF4 *pOAMF4)
{
	unsigned long VCI = (pLBInfo->scope)? 4 : 3;
	unsigned char gfc = 0, clp = 0, ucPTI = 0;
	int i;

	ATM_OAM_DEBUG("Into the OAMF4GenLBCell\n");

	// generate cell header & payload
	pLBInfo->cell[OAM_FORMAT_H1] = (unsigned char) ((pOAMF4->VPI >> 4) & 0x0F) | ((gfc << 4) & 0xF0);
	pLBInfo->cell[OAM_FORMAT_H2] = (unsigned char) (((pOAMF4->VPI << 4) & 0x00F0) | (VCI >> 12));
	pLBInfo->cell[OAM_FORMAT_H3] = (unsigned char) (VCI >> 4);
	pLBInfo->cell[OAM_FORMAT_H4] = (unsigned char) ((VCI << 4) & 0x00F0) | ((ucPTI << 1) & 0xE) | (clp & 1);

	pLBInfo->cell[OAM_FORMAT_TYPE] = FAULT_LB;	// OAM type & function
	pLBInfo->cell[OAM_FORMAT_LB_INDICATION] = 1;			// loopback indication bit
	pLBInfo->cell[OAM_FORMAT_LB_TAG] = (unsigned char)(pLBInfo->tag >> 24 & 0xFF);	// loopback correlation tag
	pLBInfo->cell[OAM_FORMAT_LB_TAG+1] = (unsigned char)(pLBInfo->tag >> 16 & 0xFF);	// loopback correlation tag
	pLBInfo->cell[OAM_FORMAT_LB_TAG+2] = (unsigned char)(pLBInfo->tag >> 8 & 0xFF);	// loopback correlation tag
	pLBInfo->cell[OAM_FORMAT_LB_TAG+3] = (unsigned char)(pLBInfo->tag & 0xFF);		// loopback correlation tag

	for (i = 0; i < OAM_LB_LLID_SIZE; i++) {
		pLBInfo->cell[OAM_FORMAT_LB_LLID+i] = pLBInfo->locationID[i];					// loopback location ID
		pLBInfo->cell[OAM_FORMAT_LB_SID+i] = pOAMF4->OAMF4ucCPID[i];					// loopback source ID
	}

	for (i = 0; i < OAM_LB_UNUSE; i++)
		pLBInfo->cell[OAM_FORMAT_LB_UNUSE+i] = 0x6A;

	ATM_OAM_DEBUG("Leave the OAMF4GenLBCell\n");	
}

OAMF4_LB_INFO *OAMF4FindLBInfo(unsigned long tag, OAMF4 *pOAMF4)
{
	int i;

	for (i = 0; i < LB_ALL_NODE_SIZE+LB_TABLE_SIZE; i++) {
		if (tag == pOAMF4->OAMF4LBList[i].tag && pOAMF4->OAMF4LBList[i].status != FAULT_LB_IDLE)
			return &(pOAMF4->OAMF4LBList[i]);
		if (i == 0) i = LB_ALL_NODE_SIZE-1;
	}
	return NULL;
}

OAMF4_LBRX_INFO *OAMF4FindLBRXInfo(OAMF4 *pOAMF4)
{
	return &(pOAMF4->LBRXList[0]);
}

void OAMF4TMNLBTimeoutFunc(struct timer_list *id, int arg)
{
	OAMF5Timer *pTimer;
	OAMF4 *pOAMF4;
	
	ATM_OAM_DEBUG("Into the OAMF4TMNLBTimeoutFunc %x\n", arg);

	pTimer = (OAMF4Timer*) arg;
	if(pTimer)
	{
		OAMF4_LB_INFO *pLBInfo = NULL;
		int i;
		pOAMF4 = pTimer->pOAM;
		pTimer->occupied = EMPTY;

		if(pTimer->OAMFunctionType != FAULT_LB)
		{
			ATM_OAM_TRACE("OAMF4TMNLBTimeoutFunc (pTimer->OAMFunctionType != FAULT_LB)\n");
			return;
		}

		pLBInfo = &pOAMF4->OAMF4LBList[pTimer->OAMUserData];

		// double check
		if(&(pLBInfo->timer) != pTimer)
		{
			ATM_OAM_TRACE("OAMF4TMNLBTimeoutFunc (&(pLBInfo->timer) != pTimer)\n");
			return;
		}

		// timestamp
		pLBInfo->rtt = OAMF4GetTimeStamp() - pLBInfo->timestamp;

		// return to idle state
		pLBInfo->status = FAULT_LB_STOP;
		OAMF4ChangeState(pOAMF4);
	}

	ATM_OAM_DEBUG("Leave the OAMF4TMNLBTimeoutFunc\n");	
}

int OAMF4TMNTxLBStart(unsigned char Scope, unsigned char *pLocID, unsigned long *Tag, OAMF4 *pOAMF4)
{
	OAMF5_LB_INFO *pLBInfo = NULL;

	int i, j;

	ATM_OAM_DEBUG("Into the OAMF4TMNTxLBStart\n");

	// search for an empty OAMF4_LB_INFO from table
	if (!pLocID[0]) {
		if (pOAMF4->OAMF4LBList[0].status == FAULT_LB_WAITING) {
			ATM_OAM_TRACE("OAMF4TMNTxLBStart (pOAMF4->OAMF4LBList[0].status == FAULT_LB_WAITING)\n");
			return 0;
		}
		pLBInfo = &pOAMF4->OAMF4LBList[0];

		// clean up all-0 table
		for (i = 0; i < LB_ALL_NODE_SIZE; i++) {
			pLBInfo[i].all0_status = FAULT_LB_IDLE;
			pLBInfo[i].count = 0;
			pLBInfo[i].tag = 0;
			for (j = 0; j < OAM_LB_LLID_SIZE; j++)
				pLBInfo[i].locationID[j] = 0;
			pLBInfo[i].timestamp = 0;
			pLBInfo[i].rtt = 0;
		}
	} else {
		for (i = LB_ALL_NODE_SIZE; i < LB_ALL_NODE_SIZE+LB_TABLE_SIZE; i++) {
			if (pOAMF4->OAMF4LBList[i].status == FAULT_LB_IDLE) {
				pLBInfo = &pOAMF4->OAMF4LBList[i];
				break;
			}
		}

		// if no more IDLE list, pick a STOP list
		if(pLBInfo == NULL) {
			unsigned int timestamp = 0xFFFFFFFF;
			for (i = LB_ALL_NODE_SIZE; i < LB_ALL_NODE_SIZE+LB_TABLE_SIZE; i++) {
				if ((pOAMF4->OAMF4LBList[i].status == FAULT_LB_STOP) && (pOAMF4->OAMF4LBList[i].timestamp < timestamp)){
					pLBInfo = &pOAMF4->OAMF4LBList[i];
					timestamp = pOAMF4->OAMF4LBList[i].timestamp;
				}
			}
		}
	}

	if (pLBInfo == NULL){
		ATM_OAM_TRACE("OAMF4TMNTxLBStart (pLBInfo == NULL)\n");
		return 0;
	}

	// prepare LB information
//	AtomicSet(&(pLBInfo->status), FAULT_LB_WAITING);	// loopback state: waiting
	pLBInfo->status = FAULT_LB_WAITING;
//	AtomicIncrement((long *)&OAMF5ulFaultLBTag);
	pOAMF4->OAMF4ulFaultLBTag++;
	pLBInfo->tag = pOAMF4->OAMF4ulFaultLBTag-1;
	pLBInfo->scope = Scope;
	pLBInfo->count = 0;

	for (i = 0; i < OAM_LB_LLID_SIZE; i++)
		pLBInfo->locationID[i] = pLocID[i];
	
	OAMF4GenLBCell(pLBInfo, pOAMF4);					// generate OAM LB cell
	OAMF4SendACell(pLBInfo->cell);	// Send out a LB OAM cell
	pOAMF4->OAMF4ulFaultLBTxGoodCell++;

	// adjust internal state
	if (pOAMF4->OAMF4lFaultLBState != FAULT_LB_WAITING)
		pOAMF4->OAMF4lFaultLBState = FAULT_LB_WAITING;

	// timestamp
	pLBInfo->timestamp = OAMF4GetTimeStamp();
	pLBInfo->rtt = 0;
	
	// start timer
	if(OAMF4StartTimer(&(pLBInfo->timer), (VOIDFUNCPTR )OAMF4TMNLBTimeoutFunc) == 0)
	{
		pLBInfo->status = FAULT_LB_IDLE;
		OAMF4ChangeState(pOAMF4);
	}

	*Tag = pLBInfo->tag;

	ATM_OAM_DEBUG("Leave the OAMF4TMNTxLBStart\n");

	return 1;
}

int OAMF4TMNTxLBStop(unsigned long Tag, OAMF4 *pOAMF4)
{
	OAMF4_LB_INFO *pLBInfo;
	OAMF4Timer *pLBTimer = NULL;
	OAMF4_LBRX_INFO *pLBRXInfo;

	int i, j;

	ATM_OAM_DEBUG("Into the OAMF4TMNTxLBStop\n");

	if ((pLBInfo = OAMF4FindLBInfo(Tag, pOAMF4)) == NULL) return 0;

	// clear OAM FM LB Rx List
	pLBRXInfo = &(pOAMF4->LBRXList[0]);
	for (i = 0; i < LB_ALL_NODE_SIZE; i++)
	{
		for (j = 0; j < OAM_LB_LLID_SIZE; j++)
		{
			pLBRXInfo[i].locationID[j] = 0;
		}

		pLBRXInfo[i].status = EMPTY;
		pLBRXInfo[i].sur_des_count = 0;
	}

	// stop timer
	OAMF4StopTimer(&(pLBInfo->timer));

	// return to idle state
	pLBInfo->status = FAULT_LB_IDLE;
	OAMF4ChangeState(pOAMF4);
	
	ATM_OAM_DEBUG("Out the OAMF4TMNTxLBStop\n");
	return 1;
}

unsigned short get_vci(unsigned char *pCell)
{
	return (((((unsigned short) pCell[1]) << 12) & (0xf000)) | ((((unsigned short) pCell[2]) << 4) & (0x0ff0))
				| ((((unsigned short) pCell[3]) >> 4) & (0x000f)));
}

int OAMF4ProcessRxLBCell(unsigned char *pOamCell, OAMF4 *pOAMF4)
{
	unsigned long VCI = get_vci(pOamCell);
	unsigned char ucLBIndicator = (pOamCell[OAM_FORMAT_LB_INDICATION] & 1), TempIndex = EMPTY;
	int i, j;
	// assumed CRC-10 is correct, ASIC will handle CRC-10

	ATM_OAM_DEBUG("Into the OAMF4ProcessRxLBCell\n");		

	// handle FM LB request	
	if ((VCI == 3 || VCI == 4) && ucLBIndicator){
		// segment/end-to-end OAM F5 FM LB
		// it's segment endpoint or connection endpoint

		char Match1 = FALSE, Match2 = FALSE, Match3 = FALSE;		

		// verify LLID
		if (pOamCell[OAM_FORMAT_LB_LLID] == 0x6A)
		{
			ATM_OAM_DEBUG("OAMF4ProcessRxLBCell LLID 6A\n");
			return 0; // no loopback
		}

		if (memcmp(&pOamCell[OAM_FORMAT_LB_LLID], pOAMF4->OAMF4ucCPID, OAM_LB_LLID_SIZE) == 0)
			Match1 = TRUE;	// match connection point ID
		else if (memcmp(&pOamCell[OAM_FORMAT_LB_LLID], f5_llid_all_0, OAM_LB_LLID_SIZE) == 0)
			Match2 = TRUE;	// all connection point ID
		else if (memcmp(&pOamCell[OAM_FORMAT_LB_LLID], f5_llid_all_1, OAM_LB_LLID_SIZE) == 0)
			Match3 = TRUE;	// end-to-end loopback

		if (VCI == 4)		// is this end-to-end LB
			Match2 = FALSE;

		if (Match1 || Match2 || Match3) { // Loop cell back
			char SidFound = FALSE;
			OAMF5_LBRX_INFO *pLBRXInfo;

			pLBRXInfo = &(pOAMF4->LBRXList[0]);
		
			// reset loopback indication		
			pOamCell[OAM_FORMAT_LB_INDICATION] &= 0xFE;
			// set LLID		
			memcpy(&pOamCell[OAM_FORMAT_LB_LLID], pOAMF4->OAMF4ucCPID, OAM_LB_LLID_SIZE);

			// save source ID
			// ITEX's implementation is save LLID, I think they are wrong. by Dick Tam
			for (i = 0; i < LB_ALL_NODE_SIZE; i++) {
				if((pLBRXInfo[i].status == OCCUPY) &&
					(memcmp(&pOamCell[OAM_FORMAT_LB_SID], pLBRXInfo[i].locationID, OAM_LB_SRCID_SIZE) == 0)) {

					SidFound = TRUE;
					pLBRXInfo[i].sur_des_count++;
					break;
				}					
			}

			if(!SidFound) {
				for (i = 0; i < LB_ALL_NODE_SIZE; i++) {
					if(pLBRXInfo[i].status == EMPTY) {
						pLBRXInfo[i].status = OCCUPY;
						memcpy(pLBRXInfo[i].locationID, &pOamCell[OAM_FORMAT_LB_SID],  OAM_LB_SRCID_SIZE);
						pLBRXInfo[i].sur_des_count++;
						break;
					}
				}
			}

			// debug message
			for (i = 0; i < LB_ALL_NODE_SIZE; i++)
			{
				if (pLBRXInfo[i].sur_des_count != 0)
				{
					ATM_OAM_DEBUG("The Reomte ID =");
					for (j = 0; j < OAM_LB_SRCID_SIZE; j++)
						ATM_OAM_DEBUG("%x", pLBRXInfo[i].locationID[j]);
					ATM_OAM_DEBUG(", The Rx Remote Count = %d\n", pLBRXInfo[i].sur_des_count);
				}
			}

			// Do not have to change cell header
			// cell is ready to be sent out at the moment
			return 1;
		}
	}

	// handle FM LB response	
	if (pOAMF4->OAMF4lFaultLBState == FAULT_LB_WAITING) {
		if ((VCI == 3 || VCI == 4) && (!ucLBIndicator)){
			OAMF4_LB_INFO *pLBInfo = NULL;
			unsigned long LBTag;
			
			// verify loopback source ID
			if (memcmp(&pOamCell[OAM_FORMAT_LB_SID], pOAMF4->OAMF4ucCPID, OAM_LB_SRCID_SIZE) != 0)
				return 0;

			// verify correlation tag value
			LBTag = (unsigned long)(pOamCell[OAM_FORMAT_LB_TAG]); 
			LBTag = (LBTag << 8) | ((unsigned long)(pOamCell[OAM_FORMAT_LB_TAG+1])); 
			LBTag = (LBTag << 8) | ((unsigned long)(pOamCell[OAM_FORMAT_LB_TAG+2])); 
			LBTag = (LBTag << 8) | ((unsigned long)(pOamCell[OAM_FORMAT_LB_TAG+3]));
				
			if ((pLBInfo = OAMF4FindLBInfo(LBTag, pOAMF4)) != NULL) {

				// loopback successful
				if (pLBInfo->status == FAULT_LB_WAITING) {

					if (pLBInfo == &pOAMF4->OAMF4LBList[0])
					{
						// all zero LB, don't stop timer
						// wait 5 second to collect all response

						// save LLID to pLBInfo->locationID, handle counter
						for (i = 0; i < LB_ALL_NODE_SIZE; i++)
						{
							if (pLBInfo[i].all0_status == FAULT_LB_WAITING) {

								// match LLID
								if (memcmp(&pOamCell[OAM_FORMAT_LB_LLID], pLBInfo[i].locationID, OAM_LB_LLID_SIZE) == 0)
								{
									pLBInfo[i].count++;
									return 0;
								}

							} else if (pLBInfo[i].all0_status == FAULT_LB_IDLE) {

								// save LLID
								pLBInfo[i].all0_status = FAULT_LB_WAITING;
								pLBInfo[i].count++;
								memcpy(pLBInfo[i].locationID, &pOamCell[OAM_FORMAT_LB_LLID],  OAM_LB_LLID_SIZE);

								// timestamp
								pLBInfo[i].rtt = OAMF4GetTimeStamp() - pLBInfo->timestamp;

								return 0;
							}
						}

						// more than LB_ALL_NODE_SIZE reponse, how to handle it ?

					} else {
						pLBInfo->count++;

						// timestamp
						pLBInfo->rtt = OAMF4GetTimeStamp() - pLBInfo->timestamp;

						// stop timer
						OAMF4StopTimer(&(pLBInfo->timer));					

						// save LLID
						memcpy(pLBInfo[i].locationID, &pOamCell[OAM_FORMAT_LB_LLID],  OAM_LB_LLID_SIZE);

						// return to idle state
						pLBInfo->status = FAULT_LB_STOP;
						OAMF4ChangeState(pOAMF4);
					}

					// Should tell TMN that LB successes.
					// TODO ... 
					// (or TMN may monitor Variable ulFaultLBRxGoodCell instead.)
				}
			}
		}
	}

	ATM_OAM_DEBUG("Leave the OAMF4ProcessRxLBCell\n");		

	return 0;
}

void OAMRxF4Cell(unsigned char *pOamCell, OAMF4 *pOAMF4)
{
	ATM_OAM_DEBUG("Into the RxOAMF4Cell %d\n", pOamCell[OAM_FORMAT_TYPE]);

#if 0
	{
		unsigned int i;
		for(i=0;i<52;i+=13)
		{
			ATM_OAM_DEBUG("Cell(%d) %x %x %x %x %x %x %x %x %x %x %x %x %x\n"
				,i
				,pOamCell[i] ,pOamCell[i+1] ,pOamCell[i+2] ,pOamCell[i+3] ,pOamCell[i+4] ,pOamCell[i+5]
				,pOamCell[i+6] ,pOamCell[i+7] ,pOamCell[i+8] ,pOamCell[i+9] ,pOamCell[i+10] ,pOamCell[i+11],pOamCell[i+12]);
		}
	}
#endif

 	switch (pOamCell[OAM_FORMAT_TYPE]) {

#if 0
	case FAULT_AIS:
	case FAULT_RDI:
	case FAULT_CC:
	case PERFORMANCE_FPM:
	case PERFORMANCE_BACKWARD_REPORT:
	case ACT_DEACT_FPM:
	case ACT_DEACT_FPM_BR:
	case ACT_DEACT_CC:
	case APS_GROUP_PROTECT:
	case APS_INDIVIDUAL_PROTECT:
	case SYSTEM_MANAGEMENT:
#endif


	case FAULT_LB:
		if  (pOAMF4 != NULL)
		{
			if (OAMF4ProcessRxLBCell(pOamCell, pOAMF4))
			{
				OAMF4SendACell(pOamCell);	// send LB cell out
				pOAMF4->OAMF4ulFaultLBTxGoodCell++;
			}
		}
		pOAMF4->OAMF4ulFaultLBRxGoodCell++;
		break;

	default:
		break;

	}

	ATM_OAM_DEBUG("Leave the OAMRxF4Cell\n");		
}








/////////////////////////////////////////////////////////////////////////////
//
//		SetAtmOAMCpid
//
//		Input		: pointer of ATMOAMLBID structure
//
//		Output		: fail (0)/OK (1)
//
//		Author		: Realtek CN SD2
//
/////////////////////////////////////////////////////////////////////////////
int SetAtmOAMCpid(ATMOAMLBID *pATMOAMLBID)
{
	/* F5 */
	if(OAMF5SetVpiVci(pATMOAMLBID->vpi, pATMOAMLBID->vci, &OAMF5_info) == 0)
		return 0;

	if(OAMF5TMNSetCPID(pATMOAMLBID->LocID[0], &(pATMOAMLBID->LocID[1]), &OAMF5_info) == 0)
		return 0;

	if(OAMF4SetVpiVci(pATMOAMLBID->vpi, pATMOAMLBID->vci, &OAMF4_info) == 0)
		return 0;

	if(OAMF4TMNSetCPID(pATMOAMLBID->LocID[0], &(pATMOAMLBID->LocID[1]), &OAMF4_info) == 0)
		return 0;

	return 1;
}

/////////////////////////////////////////////////////////////////////////////
//
//		StartAtmOAMLoopBack
//
//		Input		: pointer of ATMOAMLBReq structure
//
//		Output		: fail (0)/OK (1)
//
//		Author		: Realtek CN SD2
//
/////////////////////////////////////////////////////////////////////////////
int StartAtmOAMLoopBack(ATMOAMLBReq *pATMOAMLBReq)
{
	if((pATMOAMLBReq->vci == 3) || (pATMOAMLBReq->vci == 4))
	{	// F4 LB
		return OAMF4TMNTxLBStart(pATMOAMLBReq->Scope, pATMOAMLBReq->LocID, &(pATMOAMLBReq->Tag), &OAMF4_info);
	}
	else
	{	// F5 LB
		return OAMF5TMNTxLBStart(pATMOAMLBReq->Scope, pATMOAMLBReq->LocID, &(pATMOAMLBReq->Tag), &OAMF5_info);
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//		StopAtmOAMLoopBack
//
//		Input		: pointer of ATMOAMLBReq structure
//
//		Output		: fail (0)/OK (1)
//
//		Author		: Realtek CN SD2
//
/////////////////////////////////////////////////////////////////////////////
int StopAtmOAMLoopBack(ATMOAMLBReq *pATMOAMLBReq)
{
	if((pATMOAMLBReq->vci == 3) || (pATMOAMLBReq->vci == 4))
	{	// F4 LB
		return OAMF4TMNTxLBStop(pATMOAMLBReq->Tag, &OAMF4_info);
	}
	else
	{	// F5 LB
		return OAMF5TMNTxLBStop(pATMOAMLBReq->Tag, &OAMF5_info);
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//		GetAtmOAMLoopBackStatus
//
//		Input		: pointer of ATMOAMLBState structure
//
//		Output		: fail (0)/OK (1)
//
//		Author		: Realtek CN SD2
//
/////////////////////////////////////////////////////////////////////////////
int GetAtmOAMLoopBackStatus(ATMOAMLBState *pATMOAMLBState)
{
	if((pATMOAMLBState->vci == 3) || (pATMOAMLBState->vci == 4))
	{	// F4 LB
		OAMF4_LB_INFO* LBInfo = OAMF4FindLBInfo(pATMOAMLBState->Tag, &OAMF4_info);
		if(LBInfo == NULL)
		return 0;

		if(LBInfo == OAMF4_info.OAMF4LBList)
		{
			unsigned int i;
			for(i=0; i<6; i++)
			{
				memcpy(pATMOAMLBState->LocID[i], LBInfo[i].locationID, OAM_LB_LLID_SIZE);
				pATMOAMLBState->count[i] = LBInfo[i].count;
				pATMOAMLBState->rtt[i] = LBInfo[i].rtt;
				pATMOAMLBState->status[i] = LBInfo[i].all0_status;
			}
		}
		else
		{
			memset(pATMOAMLBState->LocID, 0x00, 6*16*sizeof(unsigned char));
			memset(pATMOAMLBState->count, 0x00, 6*sizeof(unsigned long));
			memset(pATMOAMLBState->rtt, 0x00, 6*sizeof(unsigned long));
			memset(pATMOAMLBState->status, 0x00, 6*sizeof(long));

			memcpy(pATMOAMLBState->LocID[0], LBInfo->locationID, OAM_LB_LLID_SIZE);
			pATMOAMLBState->count[0] = LBInfo->count;
			pATMOAMLBState->rtt[0] = LBInfo->rtt;
			pATMOAMLBState->status[0] = LBInfo->status;			
		}

		return 1;
	}
	else
	{	// F5 LB
		OAMF5_LB_INFO* LBInfo = OAMF5FindLBInfo(pATMOAMLBState->Tag, &OAMF5_info);
		if(LBInfo == NULL)
			return 0;

		if(LBInfo == OAMF5_info.OAMF5LBList)
		{
			unsigned int i;
			for(i=0; i<6; i++)
			{
				memcpy(pATMOAMLBState->LocID[i], LBInfo[i].locationID, OAM_LB_LLID_SIZE);
				pATMOAMLBState->count[i] = LBInfo[i].count;
				pATMOAMLBState->rtt[i] = LBInfo[i].rtt;
				pATMOAMLBState->status[i] = LBInfo[i].all0_status;
			}
		}
		else
		{
			memset(pATMOAMLBState->LocID, 0x00, 6*16*sizeof(unsigned char));
			memset(pATMOAMLBState->count, 0x00, 6*sizeof(unsigned long));
			memset(pATMOAMLBState->rtt, 0x00, 6*sizeof(unsigned long));
			memset(pATMOAMLBState->status, 0x00, 6*sizeof(long));

			memcpy(pATMOAMLBState->LocID[0], LBInfo->locationID, OAM_LB_LLID_SIZE);
			pATMOAMLBState->count[0] = LBInfo->count;
			pATMOAMLBState->rtt[0] = LBInfo->rtt;
			pATMOAMLBState->status[0] = LBInfo->status;			
		}

		return 1;
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//		GetAtmOAMLoopBackStatusFE
//
//		Input		: pointer of ATMOAMLBRXState structure
//
//		Output		: fail (0)/OK (1)
//
//		Author		: Realtek CN SD2
//
/////////////////////////////////////////////////////////////////////////////
int GetAtmOAMLoopBackStatusFE(ATMOAMLBRXState *pATMOAMLBRXState)
{
	if((pATMOAMLBRXState->vci == 3) || (pATMOAMLBRXState->vci == 4))
	{	// F4 LB
		unsigned int i;
		for(i=0; i<6; i++)
		{
			memcpy(pATMOAMLBRXState->LocID[i], OAMF4_info.LBRXList[i].locationID, OAM_LB_LLID_SIZE);
			pATMOAMLBRXState->count[i] = OAMF4_info.LBRXList[i].sur_des_count;
			pATMOAMLBRXState->status[i] = OAMF4_info.LBRXList[i].status;
		}

		return 1;
	}
	else
	{	// F5 LB
		unsigned int i;
		for(i=0; i<6; i++)
		{
			memcpy(pATMOAMLBRXState->LocID[i], OAMF5_info.LBRXList[i].locationID, OAM_LB_LLID_SIZE);
			pATMOAMLBRXState->count[i] = OAMF5_info.LBRXList[i].sur_des_count;
			pATMOAMLBRXState->status[i] = OAMF5_info.LBRXList[i].status;
		}

		return 1;
	}
}





#ifdef ATM_OAM_TEST
/* For Test only */

static unsigned long current_tag;

void OAMTestInit(void)
{
	ATMOAMLBID lbid;
	unsigned int i;

	lbid.vpi = 5;
	lbid.vci = 35;

	for(i=0; i<16; i++)
	{
		lbid.LocID[i] = i+1;
	}

	SetAtmOAMCpid(&lbid);
}

void OAMF4LbTest(void)
{
	ATMOAMLBReq req;

	req.vpi = 0;
	req.vci = 3;
	req.Scope = 1;	
	req.LocID[0] = 0x86;
	req.LocID[1] = 0x70;
	req.LocID[2] = 0x86;
	req.LocID[3] = 0x70;
	req.LocID[4] = 0x86;
	req.LocID[5] = 0x70;
	req.LocID[6] = 0x86;
	req.LocID[7] = 0x70;
	req.LocID[8] = 0x86;
	req.LocID[9] = 0x70;
	req.LocID[10] = 0x86;
	req.LocID[11] = 0x70;
	req.LocID[12] = 0x86;
	req.LocID[13] = 0x70;
	req.LocID[14] = 0x86;
	req.LocID[15] = 0x70;

	StartAtmOAMLoopBack(&req);

	current_tag = req.Tag;

	ATM_OAM_DEBUG("Tag %u\n", req.Tag);
}

//0=>fail, 1=>success
int OAMF5LbTest(ATMOAMLBReq *pucPtr)
{
	if (0==StartAtmOAMLoopBack(pucPtr)) {
	    printk("OAMF5LbTest no responce! Test stop.\n");
	    return 0;
	};

	current_tag = pucPtr->Tag;

	ATM_OAM_DEBUG("Tag %u\n", pucPtr->Tag);
	return 1;
}

void OAMF5StopLbTest(void);

static OAMF5Timer oamf5lbtest_timer;
void OAMF5LbTestTimer(void *arg)
{
	ATMOAMLBReq *pucPtr;

	pucPtr = (ATMOAMLBReq *)arg;

	if (0==OAMF5LbTest(pucPtr)) {
    	    OAMF5StopLbTest();
	    return;
	};

	// start timer for next OAM Tx
        init_timer (&oamf5lbtest_timer.timer);
        oamf5lbtest_timer.timer.expires = jiffies + oamf5lbtest_timer.ulTimeout;
        oamf5lbtest_timer.timer.function = OAMF5LbTestTimer;
        oamf5lbtest_timer.timer.data = (unsigned long)(arg);
	add_timer(&oamf5lbtest_timer.timer);

	oamf5lbtest_timer.occupied = OCCUPY;
}

void OAMF5TMNAllStop(OAMF5 *pOAMF5)
{
	int i;

	for (i = 0; i < LB_ALL_NODE_SIZE+LB_TABLE_SIZE; i++) {
		if (pOAMF5->OAMF5LBList[i].status != FAULT_LB_IDLE) {
			// stop timer
			OAMF5StopTimer(&(pOAMF5->OAMF5LBList[i].timer));
		};
		if (i == 0) i = LB_ALL_NODE_SIZE-1;
	}
	return;
}

static ATMOAMLBReq gblpucPtr;
void OAMF5ContiLbTest(ATMOAMLBReq *pucPtr)
{
	if(oamf5lbtest_timer.occupied == OCCUPY)
	{
		ATM_OAM_TRACE("OAMF5ContiLbTest (oamf5lbtest_timer.occupied == OCCUPY)\n");
		return;
	}
	oamf5lbtest_timer.ulTimeout = (300 * HZ) / 1000;
	memcpy(&gblpucPtr, pucPtr, sizeof(ATMOAMLBReq));
        OAMF5TMNAllStop(&OAMF5_info);
	OAMF5Init(pucPtr->vpi, pucPtr->vci, &OAMF5_info);	
	OAMF5LbTestTimer(&gblpucPtr);
}

void OAMF5StopLbTest(void)
{
    if(oamf5lbtest_timer.occupied == EMPTY)
	return;
    OAMF5TMNAllStop(&OAMF5_info);
    del_timer(&oamf5lbtest_timer.timer);
    oamf5lbtest_timer.occupied = EMPTY;
}

#if 0

void OAMF4LbTestTimer(struct timer_list *id, int arg)
{
	struct itimerspec time_value;

	if (timer_create(CLOCK_REALTIME, NULL, &oamf5lbtest_timer) == -1)
	{
		ATM_OAM_DEBUG("OAMTest8Timer timer_create Fail\n");
		return;
	}

	if (timer_connect(oamf5lbtest_timer, (VOIDFUNCPTR)OAMF4LbTestTimer, (int) NULL) == -1)
	{
		ATM_OAM_DEBUG("OAMTest8Timer timer_connect Fail\n");
		return;
	}

	bzero((char*)&time_value, sizeof(struct itimerspec));
	time_value.it_value.tv_sec = 1;
	time_value.it_value.tv_nsec = 0;

	if (timer_settime(oamf5lbtest_timer, 0, &time_value, NULL) == -1)
	{
		ATM_OAM_DEBUG("OAMTest8Timer  timer_settime Fail\n");
		return;
	}

	OAMF4LbTest();
}

void OAMF4ContiLbTest(void)
{
	OAMF4LbTestTimer(0, 0);
}

void OAMF5Dump(void)
{	// ITEX_ATM_OAM_STATUS

	int i, j, k;
	OAMF5_LB_INFO *pLBInfo = NULL;
	OAMF5_LBRX_INFO *pLBRXInfo = NULL;

	for (k = 0; k < LB_ALL_NODE_SIZE+LB_TABLE_SIZE; k++) 
	{
		pLBInfo = &(OAMF5_info.OAMF5LBList[k]);

		ATM_OAM_TRACE("[FM LB Info] Tag:%d, status:%d, count:%d, timestamp:%u ,rtt:%u, LLID:"
			,pLBInfo->tag
			,pLBInfo->status
			,pLBInfo->count
			,pLBInfo->timestamp
			,pLBInfo->rtt);


		for (j = 0; j < OAM_LB_LLID_SIZE; j++)
			ATM_OAM_TRACE("%x", pLBInfo->locationID[j]);
		ATM_OAM_TRACE("\n");
	}

	if ((pLBRXInfo = OAMF5FindLBRXInfo(&OAMF5_info)) != NULL) 
		{
			for (i = 0; i < LB_ALL_NODE_SIZE; i++)
			{
			if (pLBRXInfo[i].sur_des_count != 0)
				{
				ATM_OAM_TRACE("The Reomte ID =");
					for (j = 0; j < OAM_LB_LLID_SIZE; j++)
					ATM_OAM_TRACE("%x", pLBRXInfo[i].locationID[j]);
				ATM_OAM_TRACE(", The Rx Remote Count = %d\n", pLBRXInfo[i].sur_des_count);
				}
			}
			}
		}

void OAMF4Dump(void)
{	// ITEX_ATM_OAM_STATUS

	int i, j, k;
	OAMF4_LB_INFO *pLBInfo = NULL;
	OAMF4_LBRX_INFO *pLBRXInfo = NULL;

	for (k = 0; k < LB_ALL_NODE_SIZE+LB_TABLE_SIZE; k++) 
	{
		pLBInfo = &(OAMF4_info.OAMF4LBList[k]);

		ATM_OAM_TRACE("[FM LB Info] Tag:%d, status:%d, count:%d, timestamp:%u ,rtt:%u, LLID:"
			,pLBInfo->tag
			,pLBInfo->status
			,pLBInfo->count
			,pLBInfo->timestamp
			,pLBInfo->rtt);

		for (j = 0; j < OAM_LB_LLID_SIZE; j++)
			ATM_OAM_TRACE("%x", pLBInfo->locationID[j]);
		ATM_OAM_TRACE("\n");
	}

	if ((pLBRXInfo = OAMF4FindLBRXInfo(&OAMF4_info)) != NULL) 
	{
		for (i = 0; i < LB_ALL_NODE_SIZE; i++)
		{
			if (pLBRXInfo[i].sur_des_count != 0)
			{
				ATM_OAM_TRACE("The Reomte ID =");
				for (j = 0; j < OAM_LB_LLID_SIZE; j++)
					ATM_OAM_TRACE("%x", pLBRXInfo[i].locationID[j]);
				ATM_OAM_TRACE(", The Rx Remote Count = %d\n", pLBRXInfo[i].sur_des_count);
			}
		}
	}
}
#endif

#endif // ATM_OAM_TEST


#endif // ATM_OAM

