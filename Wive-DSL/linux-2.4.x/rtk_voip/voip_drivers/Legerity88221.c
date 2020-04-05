#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/delay.h>  	// udelay
#include "spi.h"
#include "Legerity88221.h"
#include "fsk.h"
#include "../include/rtk_voip.h"

#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226
unsigned char wideband_mode_ctrl = 0;
#endif

//---------------------static function prototype-------------------------------//
static void Legerity_soft_reset(unsigned char slic_id);
static void Legerity_time_slot(unsigned char slic_id, Le88xxx *data, unsigned char wri_re);
static void Legerity_clock_slot( Le88xxx *data, unsigned char wri_re);
static void Legerity_product_revision(void);
static void Legerity_GX_filter_coeff(unsigned char slic_id, Le88xxx *data, unsigned char wri_re);
static void Legerity_GR_filter_coeff(unsigned char slic_id, Le88xxx *data, unsigned char wri_re);
static void Legerity_B_FIR_filter_coeff(unsigned char slic_id, Le88xxx *data, unsigned char wri_re);
static void Legerity_B_IIR_filter_coeff(unsigned char slic_id, Le88xxx *data, unsigned char wri_re);
static void Legerity_X_filter_coeff(unsigned char slic_id, Le88xxx *data, unsigned char wri_re);
static void Legerity_R_filter_coeff(unsigned char slic_id, Le88xxx *data, unsigned char wri_re);
static void Legerity_Z_FIR_filter_coeff(unsigned char slic_id, Le88xxx *data, unsigned char wri_re);
static void Legerity_Z_IIR_filter_coeff(unsigned char slic_id, Le88xxx *data, unsigned char wri_re);
static void Legerity_DISN(unsigned char slic_id, Le88xxx *data, unsigned char wri_re);
static void Legerity_metering_coeff(unsigned char slic_id, Le88xxx *data, unsigned char wri_re);
static void Legerity_generator_A_B_Bias_coeff(unsigned char slic_id, Le88xxx *data, unsigned char wri_re);
static void Legerity_generator_C_D_coeff(unsigned char slic_id, Le88xxx *data, unsigned char wri_re);
static void Legerity_generator_enable_bit(unsigned char slic_id, Le88xxx *data, unsigned char wri_re);
static void Legerity_generator_disable_bit(unsigned char slic_id, Le88xxx *data);
static void Legerity_cadence_timer_coeff(unsigned char slic_id, Le88xxx *data, unsigned char wri_re);
static void Legerity_interrupt_mask_bit(unsigned char slic_id, unsigned char whichone, unsigned char mask_unmask);
static void Legerity_switching_regulator_coeff( Le88xxx *data, unsigned char wri_re);
static void Legerity_switching_regulator_control( Le88xxx *data, unsigned char wri_re);

static unsigned char checkSum( char * string );
static void Legerity_DTMF_coeff(unsigned char slic_id, char data);
void Legerity_OnHookLineReversal(unsigned char slic_id, unsigned char bReversal); // added for caller id 
//-----------------------------------------------------------------------------//
	
//The ability of this function equals to hardware reset(pull RST^).
void Legerity_hard_reset() {
	Le88xxx data;
	
	writeLegerityReg( 4, &data);
	
	return;	
}

static void Legerity_soft_reset(unsigned char slic_id) {
	Le88xxx data;
	
	slic_order = slic_id;
	writeLegerityReg( 2, &data);
	return;	
}

//Timing slot setting for transmitting and receiving.wri_re :0-write, 1-read.
static void Legerity_time_slot(unsigned char slic_id, Le88xxx *data, unsigned char wri_re) {
	
	slic_order = slic_id;
	if (!wri_re) {
		writeLegerityReg( 0x40, data);//transmit time slot
		writeLegerityReg( 0x42, data);//receive time slot
		
	} else {	
		readLegerityReg( 0x40, data);//transmit time slot
#ifdef _Legerity_debug_
		printk("transmit and receive time slot=0x%x\n",(data->byte1)&0x7f);
#endif		
	}	
	return;	
}

//Data transmit edge and transmit/receive clock slot. wri_re :0-write, 1-read.
//data.byte1=0x40;
static void Legerity_clock_slot( Le88xxx *data, unsigned char wri_re) {
	
	
	if (!wri_re) {
		writeLegerityReg( 0x44, data);
		
	} else {
		readLegerityReg( 0x44, data);
#ifdef _Legerity_debug_
		printk("transmit and receive clock slot=0x%x\n",(data->byte1)&0x7f);
#endif		
	}
	return;		
}	

//Tx,Rx voice gain. wri_re :0-write, 1-read.
//data.byte1=0x00;
void Legerity_tx_rx_gain(unsigned char slic_id, Le88xxx *data, unsigned char wri_re) {
	
	slic_order = slic_id;
	if (!wri_re) {
		writeLegerityReg( 0x50, data);
			
	} else {
		readLegerityReg( 0x50, data);
#ifdef _Legerity_debug_
		printk("TX and RX gain=0x%x\n",(data->byte1)&0x78);
#endif		
	}
	return;		
}

void Legerity_TX_slic_gain(unsigned char slic_id, unsigned char txgain)
{
	Le88xxx data;
	unsigned long flags;
			
	save_flags(flags); cli();
	if ((txgain >= 0) && (txgain <=2)) {
		readLegerityReg( 0x50, &data);
		data.byte1 &= 0xCF;
		data.byte1 |= 0x10;
		writeLegerityReg( 0x50, &data);
	} else if ((txgain >= 3) && (txgain <=7)) {
		readLegerityReg( 0x50, &data);
		data.byte1 &= 0xCF;
		data.byte1 |= 0x00;
		writeLegerityReg( 0x50, &data);
	} else if ((txgain >= 8) && (txgain <=10)) {
		readLegerityReg( 0x50, &data);
		data.byte1 &= 0xCF;
		data.byte1 |= 0x20;
		writeLegerityReg( 0x50, &data);
	}		
	restore_flags(flags);
	return;
}

void Legerity_RX_slic_gain(unsigned char slic_id, unsigned char rxgain)
{
	Le88xxx data;
	unsigned long flags;

        save_flags(flags); cli();
	if (rxgain <= 4) {
		readLegerityReg( 0x50, &data);
		data.byte1 &= 0xBF;
		data.byte1 |= 0x00;
		writeLegerityReg( 0x50, &data);
	} else {
		readLegerityReg( 0x50, &data);
		data.byte1 &= 0xBF;
		data.byte1 |= 0x40;
		writeLegerityReg( 0x50, &data);
	}
	restore_flags(flags);
	return;
}
		
//Product revision number
static void Legerity_product_revision(void) {
	Le88xxx data;
	readLegerityReg( 0x73, &data);
	
	if (data.byte2 == 0xb2)
		printk("Legerity Le88221 ");
	else if(data.byte2 == 0xb6)
		printk("Legerity Le88241 ");
	else if(data.byte2 == 0xb3)
		printk("Legerity Le88226 ");
	else if(data.byte2 == 0xb7)
		printk("Legerity Le88246 ");
	else
		printk("Unknow device ");
			
	printk("Revision %d.\n",data.byte1*100);
			
	return;	
}

//GX filter coefficient setting. wri_re :0-write, 1-read.
//All filter can be read in programed filter enabled mode.
//
static void Legerity_GX_filter_coeff(unsigned char slic_id, Le88xxx *data, unsigned char wri_re) {
	Le88xxx data_temp;
	
	slic_order = slic_id;
	
	readLegerityReg( 0x60, &data_temp);
	data_temp.byte1 |= 0x10;	//programed GX filter enabled 
	writeLegerityReg( 0x60, &data_temp);
	
	if (!wri_re) {
		writeLegerityReg( 0x80, data);
	} else {	
		readLegerityReg( 0x80, data);
#ifdef _Legerity_debug_
		printk("GX filter coefficient: \n");
		printk("Data byte1=0x%x ,Data byte2=0x%x\n",data->byte1, data->byte2);
#endif		
	}
	
#ifdef _default_filter_value_
	readLegerityReg( 0x60, &data_temp);
	data_temp.byte1 &= 0xEF;	//default GX filter enabled 
	writeLegerityReg( 0x60, &data_temp);		 	
#endif
	
	return;
}	

//GR filter coefficient setting. wri_re :0-write, 1-read.
//All filter can be read in programed filter enabled mode.
//
static void Legerity_GR_filter_coeff(unsigned char slic_id, Le88xxx *data, unsigned char wri_re) {
	Le88xxx data_temp;
	
	slic_order = slic_id;
	
	readLegerityReg( 0x60, &data_temp);
	data_temp.byte1 |= 0x20;	//programed GR filter enabled 
	writeLegerityReg( 0x60, &data_temp);
	
	if (!wri_re) {
		writeLegerityReg( 0x82, data);
	} else {		
		readLegerityReg( 0x82, data);
#ifdef _Legerity_debug_
		printk("GR filter coefficient: \n");
		printk("Data byte1=0x%x ,Data byte2=0x%x\n",data->byte1, data->byte2);
#endif
	}
	
#ifdef _default_filter_value_	
	readLegerityReg( 0x60, &data_temp);
	data_temp.byte1 &= 0xDF;	//default GR filter enabled 
	writeLegerityReg( 0x60, &data_temp);
#endif
			 	
	return;
}

//B FIR filter coefficient setting. wri_re :0-write, 1-read.
//All filter can be read in programed filter enabled mode.
//
static void Legerity_B_FIR_filter_coeff(unsigned char slic_id, Le88xxx *data, unsigned char wri_re) {
	Le88xxx data_temp;
	
	slic_order = slic_id;
	
	readLegerityReg( 0x60, &data_temp);
	data_temp.byte1 |= 0x01;	//programed B filter enabled 
	writeLegerityReg( 0x60, &data_temp);
	
	if (!wri_re) { 
		writeLegerityReg( 0x86, data);
	} else {		
		readLegerityReg( 0x86, data);
#ifdef _Legerity_debug_
		printk("B FIR filter coefficient: \n");
		int i;
		for (i=0;i<7;i++)
			printk("Data byte%d=0x%x, Data byte%d=0x%x\n",2*i+1,*(&(data->byte1)+i*2),2*i+2,*(&(data->byte1)+i*2+1));
#endif	
	}
		
#ifdef _default_filter_value_
	readLegerityReg( 0x60, &data_temp);
	data_temp.byte1 &= 0xFE;	//default B filter enabled 
	writeLegerityReg( 0x60, &data_temp);
#endif
			 	
	return;
}

//B IIR filter coefficient setting. wri_re :0-write, 1-read.
//All filter can be read in programed filter enabled mode.
//
static void Legerity_B_IIR_filter_coeff(unsigned char slic_id, Le88xxx *data, unsigned char wri_re) {
	Le88xxx data_temp;
	
	slic_order = slic_id;
	
	readLegerityReg( 0x60, &data_temp);
	data_temp.byte1 |= 0x01;	//programed B filter enabled 
	writeLegerityReg( 0x60, &data_temp);
	
	if (!wri_re) {
		writeLegerityReg( 0x96, data);
	} else {		
		readLegerityReg( 0x96, data);
#ifdef _Legerity_debug_
		printk("B IIR filter coefficient: \n");
		printk("Data byte1=0x%x ,Data byte2=0x%x\n",data->byte1, data->byte2);
#endif
	}
		
#ifdef _default_filter_value_
	readLegerityReg( 0x60, &data_temp);
	data_temp.byte1 &= 0xFE;	//default B filter enabled 
	writeLegerityReg( 0x60, &data_temp);
#endif
			 	
	return;
}

//X filter coefficient setting. wri_re :0-write, 1-read.
//All filter can be read in programed filter enabled mode.
//
static void Legerity_X_filter_coeff(unsigned char slic_id, Le88xxx *data, unsigned char wri_re) {
	Le88xxx data_temp;
	
	slic_order = slic_id;
	
	readLegerityReg( 0x60, &data_temp);
	data_temp.byte1 |= 0x08;	//programed X filter enabled 
	writeLegerityReg( 0x60, &data_temp);
	
	if (!wri_re) { 
		writeLegerityReg( 0x88, data);
	} else {	
		readLegerityReg( 0x88, data);
#ifdef _Legerity_debug_
		printk("X filter coefficient: \n");
		int i;
		for (i=0;i<6;i++)
			printk("Data byte%d=0x%x, Data byte%d=0x%x\n",2*i+1,*(&(data->byte1)+i*2),2*i+2,*(&(data->byte1)+i*2+1));
#endif
	}
		
#ifdef _default_filter_value_
	readLegerityReg( 0x60, &data_temp);
	data_temp.byte1 &= 0xF7;	//default X filter enabled 
	writeLegerityReg( 0x60, &data_temp);
#endif
			 	
	return;
}	

//R filter coefficient setting. wri_re :0-write, 1-read.
//All filter can be read in programed filter enabled mode.
//
static void Legerity_R_filter_coeff(unsigned char slic_id, Le88xxx *data, unsigned char wri_re) {
	Le88xxx data_temp;
	
	slic_order = slic_id;
	
	readLegerityReg( 0x60, &data_temp);
	data_temp.byte1 |= 0x04;	//programed R filter enabled 
	writeLegerityReg( 0x60, &data_temp);
		
	if (!wri_re) {
		writeLegerityReg( 0x8A, data);
	} else {		
		readLegerityReg( 0x8A, data);
#ifdef _Legerity_debug_
		printk("R filter coefficient: \n");
		int i;
		for (i=0;i<7;i++)
			printk("Data byte%d=0x%x, Data byte%d=0x%x\n",2*i+1,*(&(data->byte1)+i*2),2*i+2,*(&(data->byte1)+i*2+1));
#endif
	}
		
#ifdef _default_filter_value_
	readLegerityReg( 0x60, &data_temp);
	data_temp.byte1 &= 0xFB;	//default R filter enabled 
	writeLegerityReg( 0x60, &data_temp);
#endif
			 	
	return;
}

//Z FIR filter coefficient setting. wri_re :0-write, 1-read.
//All filter can be read in programed filter enabled mode.
//
static void Legerity_Z_FIR_filter_coeff(unsigned char slic_id, Le88xxx *data, unsigned char wri_re) {
	Le88xxx data_temp;
	
	slic_order = slic_id;
	
	readLegerityReg( 0x60, &data_temp);
	data_temp.byte1 |= 0x02;	//programed Z filter enabled 
	writeLegerityReg( 0x60, &data_temp);
	
	if (!wri_re) { 
		writeLegerityReg( 0x98, data);
	} else {		
		readLegerityReg( 0x98, data);
#ifdef _Legerity_debug_
		printk("Z FIR filter coefficient: \n");
		int i;
		for (i=0;i<5;i++)
			printk("Data byte%d=0x%x, Data byte%d=0x%x\n",2*i+1,*(&(data->byte1)+i*2),2*i+2,*(&(data->byte1)+i*2+1));
#endif	
	}
		
#ifdef _default_filter_value_
	readLegerityReg( 0x60, &data_temp);
	data_temp.byte1 &= 0xFD;	//default Z filter enabled 
	writeLegerityReg( 0x60, &data_temp);
#endif
			 	
	return;
}

//Z IIR filter coefficient setting. wri_re :0-write, 1-read.
//All filter can be read in programed filter enabled mode.
//
static void Legerity_Z_IIR_filter_coeff(unsigned char slic_id, Le88xxx *data, unsigned char wri_re) {
	Le88xxx data_temp;
	
	slic_order = slic_id;
	
	readLegerityReg( 0x60, &data_temp);
	data_temp.byte1 |= 0x02;	//programed Z filter enabled 
	writeLegerityReg( 0x60, &data_temp);
	
	if (!wri_re) {
		writeLegerityReg( 0x9A, data);
	} else {		
		readLegerityReg( 0x9A, data);
#ifdef _Legerity_debug_
		printk("Z IIR filter coefficient: \n");
		int i;
		for (i=0;i<2;i++)
			printk("Data byte%d=0x%x, Data byte%d=0x%x\n",2*i+1,*(&(data->byte1)+i*2),2*i+2,*(&(data->byte1)+i*2+1));
			printk("Data byte5=0x%x\n",data->byte5);
#endif	
	}
		
#ifdef _default_filter_value_
	readLegerityReg( 0x60, &data_temp);
	data_temp.byte1 &= 0xFD;	//default Z filter enabled 
	writeLegerityReg( 0x60, &data_temp);
#endif
			 	
	return;
}

//Digital impedance scaling network setting. wri_re :0-write, 1-read.
//
static void Legerity_DISN(unsigned char slic_id, Le88xxx *data, unsigned char wri_re) {
	
	slic_order = slic_id;
	
	if (!wri_re) {
		writeLegerityReg( 0xCA, data);
	} else {
		readLegerityReg( 0xCA, data);
#ifdef _Legerity_debug_
		printk("DISN=0x%x\n",data->byte1);
#endif		
	}
			
	return;
}	

//Metering parameter setting. wri_re: 0-write, 1-read.
//This function is just parameter setting. METR of 56h must be set to enable meter. 
//
static void Legerity_metering_coeff(unsigned char slic_id, Le88xxx *data, unsigned char wri_re) {
	
	slic_order = slic_id;
	
	if (!wri_re) {
		writeLegerityReg( 0xD0, data);
	} else {
		readLegerityReg( 0xD0, data);
#ifdef _Legerity_debug_
		printk("Metering parameter: \n");
		int i;
		for (i=0;i<2;i++)
			printk("Data byte%d=0x%x, Data byte%d=0x%x\n",2*i+1,*(&(data->byte1)+i*2),2*i+2,*(&(data->byte1)+i*2+1));
#endif	
	}	
	printk("Set METR of 56h to enable meter.\n");		
	return;
}

//Signal generator A, B and Bias parameter setting. wri_re: 0-write, 1-read.
//This function is just parameter setting. EGBIAD, EGA and EGB of DEh must set to enable generator.
//Or SGCAD of DEh must to set to enable cadence.
//
static void Legerity_generator_A_B_Bias_coeff(unsigned char slic_id, Le88xxx *data, unsigned char wri_re) {
	
	slic_order = slic_id;
	
	if (!wri_re) {
		writeLegerityReg( 0xD2, data);
	} else {
		readLegerityReg( 0xD2, data);
#ifdef _Legerity_debug_
		printk("Signal generator A, B and Bias parameter: \n");
		int i;
		for (i=0;i<5;i++)
			printk("Data byte%d=0x%x, Data byte%d=0x%x\n",2*i+1,*(&(data->byte1)+i*2),2*i+2,*(&(data->byte1)+i*2+1));
			printk("Data byte11=0x%x\n",data->byte11);
#endif
	}	
	printk("Set EGBIAS, EGA and EGB of DEh to enable signal generator.\n");		
	return;
}

//Signal generator C and D parameter setting. wri_re: 0-write, 1-read.
//This function is just parameter setting. EGC and EGD of DEh must set to enable generator.
//Or CIDDIS of EAh must to set to enable Caller ID.
//
static void Legerity_generator_C_D_coeff(unsigned char slic_id, Le88xxx *data, unsigned char wri_re) {
	
	slic_order = slic_id;
	
	if (!wri_re) {
		writeLegerityReg( 0xD4, data);
	} else {
		readLegerityReg( 0xD4, data);
#ifdef _Legerity_debug_
		printk("Signal generator C and D parameter: \n");
		int i;
		for (i=0;i<4;i++)
			printk("Data byte%d=0x%x, Data byte%d=0x%x\n",2*i+1,*(&(data->byte1)+i*2),2*i+2,*(&(data->byte1)+i*2+1));
#endif
	}	
	printk("Set EGC and EGD of DEh to enable signal generator.\n");		
	return;
}

//Signal generator enable bit for generator A, B, C, D and Bias. wri_re: 0-write, 1-read.
//SGCAD, EGBIAD, EGA, EGB, EGC and EGD of DEh.
static void Legerity_generator_enable_bit(unsigned char slic_id, Le88xxx *data, unsigned char wri_re) {
	Le88xxx data_temp;
	
	slic_order = slic_id;
	
	if (!wri_re) {
		readLegerityReg( 0xDE, &data_temp);//read original value
#ifdef	_Legerity_debug_
		printk("Enable bit of DE/DFh before writting =%x\n",data_temp.byte1);	
#endif		
		data_temp.byte1 |= data->byte1;
		writeLegerityReg( 0xDE, &data_temp);
#ifdef	_Legerity_debug_
		printk("Enable bit of DE/DFh after writting =%x\n",data_temp.byte1);	
#endif		
	} else {
		readLegerityReg( 0xDE, data);
#ifdef	_Legerity_debug_
		printk("Generator mask bit of DE/DFh =%x\n",data->byte1);
#endif				
	}		

	return;
}

//Signal generator disable bit for generator A, B, C, D and Bias.
//SGCAD, EGBIAD, EGA, EGB, EGC and EGD of DEh.
static void Legerity_generator_disable_bit(unsigned char slic_id, Le88xxx *data) {
	Le88xxx data_temp;
	
	slic_order = slic_id;
	
	
		readLegerityReg( 0xDE, &data_temp);//read original value
#ifdef	_Legerity_debug_
		printk("Disable bit of DE/DFh before writting =%x\n",data_temp.byte1);	
#endif		
		data_temp.byte1 &= ~(data->byte1);
		writeLegerityReg( 0xDE, &data_temp);
#ifdef	_Legerity_debug_
		printk("Disable bit of DE/DFh after writting =%x\n",data_temp.byte1);	
#endif		
	
	return;
}

//Cadence timer on/off setting. wri_re: 0-write, 1-read.
//Set SGCAD of DEh to enable cadence on.
static void Legerity_cadence_timer_coeff(unsigned char slic_id, Le88xxx *data, unsigned char wri_re) {
		
	slic_order = slic_id;	
	
	if (!wri_re) {
		writeLegerityReg( 0xE0, data);
	} else {
		readLegerityReg( 0xE0, data);
#ifdef _Legerity_debug_
		printk("Cadence timer parameter: \n");
		int i;
		for (i=0;i<2;i++)
			printk("Data byte%d=0x%x, Data byte%d=0x%x\n",2*i+1,*(&(data->byte1)+i*2),2*i+2,*(&(data->byte1)+i*2+1));
#endif	
	}
	return;
}		
			
	

//Caller ID checksum
static unsigned char checkSum( char * string ) {
	int i =0;
	unsigned char sum=0;
	while (string[i] !=0){
        	sum += string[i++];
        }

	return -sum ;
}


//Switching regulator parameter setting. wri_re : 0-write, 1-read.
//default value 00 05 80
static void Legerity_switching_regulator_coeff( Le88xxx *data, unsigned char wri_re) {

	if (!wri_re) {
		writeLegerityReg( 0xE4, data);
	} else {
		readLegerityReg( 0xE4, data);
#ifdef _Legerity_debug_
		printk("Switching regulator parameter: \n");
		printk("Data byte1=0x%x, Data byte2=0x%x\n",data->byte1,data->byte2);
		printk("Data byte3=0x%x\n",data->byte3);
#endif	
	}
			 
	return;
}

//Switching regulator control. wri_re : 0-write, 1-read.
//default value 07
static void Legerity_switching_regulator_control( Le88xxx *data, unsigned char wri_re) {

	if (!wri_re) {
		writeLegerityReg( 0xE6, data);
	} else {
		readLegerityReg( 0xE6, data);
#ifdef _Legerity_debug_
		printk("Switching regulator control=0x%x\n",data->byte1);
#endif	
	}		 
	return;
}

//Ring state.
int Legerity_FXS_ring(ring_struct *slic_id) {
	Le88xxx data;
	
	unsigned long flags;

	save_flags(flags); cli();

	slic_order = slic_id->CH;	
	if (slic_id->ring_set == 1) {	/*Ring on*/
		//set SGCAD on.
		readLegerityReg( 0xDE, &data);
		data.byte1 |= 0x80;
		writeLegerityReg( 0xDE, &data); 
		//set ring state
		readLegerityReg( 0x56, &data);
		data.byte1 = (data.byte1&0xF0) | 0x07;
		writeLegerityReg( 0x56, &data);
	} else if (slic_id->ring_set == 0) {	/*Ring off*/
		//set SGCAD off.
		readLegerityReg( 0xDE, &data);
		data.byte1 &= 0x7F;
		writeLegerityReg( 0xDE, &data); 
		//set on-hook state
		readLegerityReg( 0x56, &data);
		data.byte1 = (data.byte1&0xF0) | 0x04;
		writeLegerityReg( 0x56, &data);
	} 
	restore_flags(flags);

	return 0xff;
	
}

int Check_Legerity_FXS_ring(ring_struct *ring)		
{
	unsigned long flags;
	int ringer; //0: ring off, 1: ring on
	Le88xxx data;

	save_flags(flags); cli();
	
	slic_order = ring->CH;

	readLegerityReg( 0xDE, &data);
	
	//printk("%x\n", data.byte1&0x0F);
	
	if ((data.byte1&0x0F) == 1)
		ringer = 1;
	else
		ringer = 0;
		
	restore_flags(flags);

	return ringer;
}

//System state setting manual. wri_re: 0-write, 1-read.
unsigned char Legerity_system_state(unsigned char slic_id, unsigned char state, unsigned char wri_re){
	Le88xxx data;
	
	slic_order = slic_id;
	
	if (!wri_re) {
		switch (state) {
			case DISCONNECT:
			case TIP_OPEN:
			case RING_OPEN:
			case LOWER_POWER_IDLE:
			case SHUTDOWN:
				{
				//set ACT off
				readLegerityReg( 0x56, &data);
				data.byte1 &= 0xDF;
				//set state
				data.byte1 = (data.byte1&0xF0) | (state&0x0F);
				writeLegerityReg( 0x56, &data);
				}
				break;
			case ACTIVE_LOW_BATTERY:
			case ACTIVE_MID_BATTERY:
			case LOW_GAIN:
				{
				//set ACT on 
				readLegerityReg( 0x56, &data);
				data.byte1 |= 0x20;
				//set state
				data.byte1 = (data.byte1&0xF0) | (state&0x0F);
				writeLegerityReg( 0x56, &data);	
				}	
				break;
			case RINGING:
				{
				//set SGCAD on.
				readLegerityReg( 0xDE, &data);
				data.byte1 |= 0x80;
				writeLegerityReg( 0xDE, &data);
				//set ACT on 
				readLegerityReg( 0x56, &data);
				data.byte1 |= 0x20;
				//set state
				data.byte1 = (data.byte1&0xF0) | (state&0x0F);
				writeLegerityReg( 0x56, &data);	
				}		
				break;
			default:
				printk("No such system state.\n");
				break;
		}		
		return 0xFF;
	} else {	
		readLegerityReg( 0x56, &data);
		data.byte1 &= 0x0F;
#ifdef	_Legerity_debug_
		switch (data.byte1) {
			case DISCONNECT:
				printk("DISCONNECT state.\n");
				break;
			case TIP_OPEN:
				printk("TIP_OPEN state.\n");
				break;
			case RING_OPEN:
				printk("RING_OPEN state.\n");
				break;
			case ACTIVE_LOW_BATTERY:
				printk("ACTIVE_LOW_BATTERY state.\n");
				break;
			case LOWER_POWER_IDLE:
				printk("LOWER_POWER_IDLE state.\n");
				break;
			case RINGING:
				printk("RINGING state.\n");
				break;
			case LOW_GAIN:
				printk("LOW_GAIN state.\n");
				break;
			case ACTIVE_MID_BATTERY:
				printk("ACTIVE_MID_BATTERY state.\n");
				break;
			case SHUTDOWN:							
				printk("SHUTDOWN state.\n");
				break;
			default:
				printk("Unknow state.\n");
				break;	
		}	
		
#endif		
		return data.byte1;
	}	
		
}		

//On-hook or off-hook switch detection. 0: on-hook, 1: off-hook.
//Using this function befor unmasking MHOOK of 6C/6Dh.
unsigned char Legerity_HOOK_state(hook_struck *hook) {
	Le88xxx data;
	
	slic_order = hook->CH;
	if ((slic_order < 0) || (slic_order > 1)) {
		printk("No such device.\n");
		return 0xFF;
	}	 
	
	readLegerityReg( 0x4F, &data);
	if (slic_order == 0)
		hook->hook_status = data.byte1&0x01;
	else if (slic_order == 1) 
		hook->hook_status = data.byte2&0x01;
	
	return 0xFF;
}	


		
//Interrupt mask bit mask or unmask. mask_unmask: 0-mask, 1-unmask.
static void Legerity_interrupt_mask_bit(unsigned char slic_id, unsigned char whichone, unsigned char mask_unmask) {
	Le88xxx data;
		
	slic_order = slic_id;
	readLegerityReg( 0x6C, &data);
	if (mask_unmask == 0) { //unmask interrupt bit.
#ifdef	_Legerity_debug_
		printk("Befor unmask data.byte1=0x%x, data.byte2=0x%x\n",data.byte1,data.byte2);
#endif		
		switch (whichone) {
			case MCFAIL:
				data.byte1 &= ~whichone;
				break;
			case MOCALMY_Z:
				{
				data.byte1 &= ~whichone;
				data.byte2 &= ~whichone;	
				}
				break;
			case MTEMPA:
			case MIO:
			case MCAD:
			case MCID:
			case MGNK:
			case MHOOK:
				{
				if (slic_id == 0)
					data.byte1 &= ~whichone;
				else if (slic_id == 1)
					data.byte2 &= ~whichone;	
				}
				break;
			default:
				printk("Error unmask bit\n");
				break;	
		} //end switch	
#ifdef	_Legerity_debug_
		printk("After unmask data.byte1=0x%x, data.byte2=0x%x\n",data.byte1,data.byte2);
#endif			
	} else { //mask interrupt bit
#ifdef	_Legerity_debug_
		printk("Befor mask data.byte1=0x%x, data.byte2=0x%x\n",data.byte1,data.byte2);
#endif	
		switch (whichone) {
			case MCFAIL:
				data.byte1 |= whichone;
				break;
			case MOCALMY_Z:
				{
				data.byte1 |= whichone;
				data.byte2 |= whichone;	
				}
				break;
			case MTEMPA:
			case MIO:
			case MCAD:
			case MCID:
			case MGNK:
			case MHOOK:
				{
				if (slic_id == 0)
					data.byte1 |= whichone;
				else if (slic_id == 1)
					data.byte2 |= whichone;	
				}
				break;
			default:
				printk("Error mask bit\n");
				break;	
		} //end switch	
#ifdef	_Legerity_debug_
		printk("After mask data.byte1=0x%x, data.byte2=0x%x\n",data.byte1,data.byte2);
#endif
	}
	writeLegerityReg( 0x6C, &data);
	
	return;
}		
	
void Legerity_slic_init_all(unsigned char slic_id) {
	Le88xxx data;
	slic_order = slic_id;
	
	
	//choice which slic enable
	if (slic_id == 0) 
		data.byte1 = 0x01;	
	else if (slic_id == 1)
		data.byte1 = 0x02;
	writeLegerityReg( 0x4A, &data); /*Writting this register TWICE in the first time*/ 
	writeLegerityReg( 0x4A, &data);
	
	Legerity_product_revision();
	
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226
	//0-> don't support Wideband , 1-> support Wideband
	Legerity_Wideband_ctrl(0);
#endif	
	//set pclk = 2.048MHz, interrupt:open-drain, No PCM Signaling Mode
	data.byte1 = 0x82;
	writeLegerityReg( 0x46, &data);
	
	//write switching regular
	data.byte1 = S_R_Byte1;data.byte2 = S_R_Byte2;data.byte3 = S_R_Byte3;
	Legerity_switching_regulator_coeff( &data, 0);
		
	//write switching regular init. mode
	data.byte1 = S_R_Ctrl;
	Legerity_switching_regulator_control( &data, 0);
	
	//write operating condition
	data.byte1 = 0x00;
	writeLegerityReg( 0x70, &data);
	
#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226
	//set compander. u-law
	data.byte1 = 0x40;
	writeLegerityReg( 0x60, &data);
	
	//set time slot
	if (slic_id == 0) {//time slot 0
		data.byte1 = 0x00;
		Legerity_time_slot(slic_id, &data, 0);
	} else if (slic_id == 1) {//time slot 1
		data.byte1 = 0x01;
		Legerity_time_slot(slic_id, &data, 0);
	} else {
		printk("No such device.\n");
		return;
	}
#else
	if (wideband_mode_ctrl == 0) {
		//set compander. u-law
		data.byte1 = 0x40;
		writeLegerityReg( 0x60, &data);
		
		//set time slot
		if (slic_id == 0) {//time slot 0
			data.byte1 = 0x00;
			Legerity_time_slot(slic_id, &data, 0);
		} else if (slic_id == 1) {//time slot 1
			data.byte1 = 0x01;
			Legerity_time_slot(slic_id, &data, 0);
		} else {
			printk("No such device.\n");
			return;
		}
	} else if (wideband_mode_ctrl == 1) {
		//set compander. linear mode
		data.byte1 = 0x80;
		writeLegerityReg( 0x60, &data);
		
		//set time slot
		if (slic_id == 0) {//time slot 0
			data.byte1 = 0x00;
			Legerity_time_slot(slic_id, &data, 0);
		} else if (slic_id == 1) {//time slot 1
			data.byte1 = 0x02;
			Legerity_time_slot(slic_id, &data, 0);
		} else {
			printk("No such device.\n");
			return;
		}
	}
#endif
	
	//set clock slot.--------------should check
	data.byte1 = 0x40;
	Legerity_clock_slot( &data, 0);
				
	//set state auto switching self.
	data.byte1 = 0x20;
	writeLegerityReg( 0x68, &data);
			
	/*write loop supervision parameter*/
	data.byte1 = L_S_Byte1;data.byte2 = L_S_Byte2;data.byte3 = L_S_Byte3;data.byte4 = L_S_Byte4;
	writeLegerityReg( 0xC2, &data);
	
	/*write DC feed parameter*/
	data.byte1 = DC_FEED_Byte1;data.byte2 = DC_FEED_Byte2;
	writeLegerityReg( 0xC6, &data);
	
	//write the frequency and amplitude of generator A, B and Bias 
	//frqA: 20Hz, ampA: 75V. frqB:0 Hz, ampB: 0V. Bias: 0V.
	data.byte1 = OTHER_FEATURE;data.byte2 = BIAS>>8;data.byte3 = BIAS;data.byte4 = FRQA>>8;
	data.byte5 = FRQA;data.byte6 = AMPA>>8;data.byte7 = AMPA;data.byte8 = FRQB>>8;
	data.byte9 = FRQB;data.byte10 = AMPB>>8;data.byte11 = AMPB;
	Legerity_generator_A_B_Bias_coeff(slic_id, &data, 0);
	
	//write the frequency and amplitude of generator C and D.For caller ID (FSK)
	//frqC: 2200Hz, ampC: -7dBm0. frqD: 1200Hz, ampD: -7dBm0.
	data.byte1 = FRQC>>8;data.byte2 = FRQC;data.byte3 = AMPC>>8;data.byte4 = AMPC;
	data.byte5 = FRQD>>8;data.byte6 = FRQD;data.byte7 = AMPD>>8;data.byte8 = AMPD;
	Legerity_generator_C_D_coeff(slic_id, &data, 0);
	
	//write cadence timer coeff. Cadence on: 2sec, cadence off: 2sec.
	data.byte1 = CADON>>8;data.byte2 = CADON;data.byte3 = CADOFF>>8;data.byte4 = CADOFF;
	Legerity_cadence_timer_coeff(slic_id, &data, 0);
	
	//write TX and RX gain.
	data.byte1 = 0x00;
	Legerity_tx_rx_gain(slic_id, &data, 0);
	
	//write R filter.----------should check
#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226	
	data.byte1 = RCm46_36;data.byte2 = RCm26_16;data.byte3 = RCm40_30;data.byte4 = RCm20_10;
	data.byte5 = RCm41_31;data.byte6 = RCm21_11;data.byte7 = RCm42_32;data.byte8 = RCm22_12;
	data.byte9 = RCm43_33;data.byte10 = RCm23_13;data.byte11 = RCm44_34;data.byte12 = RCm24_14;
	data.byte13 = RCm45_35;data.byte14 = RCm25_15;
#else
	if (wideband_mode_ctrl == 0) {
	data.byte1 = RCm46_36;data.byte2 = RCm26_16;data.byte3 = RCm40_30;data.byte4 = RCm20_10;
	data.byte5 = RCm41_31;data.byte6 = RCm21_11;data.byte7 = RCm42_32;data.byte8 = RCm22_12;
	data.byte9 = RCm43_33;data.byte10 = RCm23_13;data.byte11 = RCm44_34;data.byte12 = RCm24_14;
	data.byte13 = RCm45_35;data.byte14 = RCm25_15;
	} else if (wideband_mode_ctrl == 1) {
		data.byte1 = RCm46_36_w;data.byte2 = RCm26_16_w;data.byte3 = RCm40_30_w;data.byte4 = RCm20_10_w;
		data.byte5 = RCm41_31_w;data.byte6 = RCm21_11_w;data.byte7 = RCm42_32_w;data.byte8 = RCm22_12_w;
		data.byte9 = RCm43_33_w;data.byte10 = RCm23_13_w;data.byte11 = RCm44_34_w;data.byte12 = RCm24_14_w;
		data.byte13 = RCm45_35_w;data.byte14 = RCm25_15_w;
	}
#endif	
	Legerity_R_filter_coeff(slic_id, &data, 0);
	
	//write GR filter.----------should check
#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226	
	data.byte1 = GRCm40_30;data.byte2 = GRCm20_10;
#else
	if (wideband_mode_ctrl == 0) {
		data.byte1 = GRCm40_30;data.byte2 = GRCm20_10;
	} else if (wideband_mode_ctrl == 1) {
		data.byte1 = GRCm40_30_w;data.byte2 = GRCm20_10_w;
	}
#endif	
	Legerity_GR_filter_coeff(slic_id, &data, 0);
		
	//write X filer.-----------should check
#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226
	data.byte1 = XCm40_30;data.byte2 = XCm20_10;data.byte3 = XCm41_31;data.byte4 = XCm21_11;
	data.byte5 = XCm42_32;data.byte6 = XCm22_12;data.byte7 = XCm43_33;data.byte8 = XCm23_13;
	data.byte9 = XCm44_34;data.byte10 = XCm24_14;data.byte11 = XCm45_35;data.byte12 = XCm25_15;
#else
	if (wideband_mode_ctrl == 0) {
		data.byte1 = XCm40_30;data.byte2 = XCm20_10;data.byte3 = XCm41_31;data.byte4 = XCm21_11;
		data.byte5 = XCm42_32;data.byte6 = XCm22_12;data.byte7 = XCm43_33;data.byte8 = XCm23_13;
		data.byte9 = XCm44_34;data.byte10 = XCm24_14;data.byte11 = XCm45_35;data.byte12 = XCm25_15;
	} else if (wideband_mode_ctrl == 1) {
		data.byte1 = XCm40_30_w;data.byte2 = XCm20_10_w;data.byte3 = XCm41_31_w;data.byte4 = XCm21_11_w;
		data.byte5 = XCm42_32_w;data.byte6 = XCm22_12_w;data.byte7 = XCm43_33_w;data.byte8 = XCm23_13_w;
		data.byte9 = XCm44_34_w;data.byte10 = XCm24_14_w;data.byte11 = XCm45_35_w;data.byte12 = XCm25_15_w;
	}
#endif
	Legerity_X_filter_coeff(slic_id, &data, 0);
	
	//write GX filter.----------should check
#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226	
	data.byte1 = GXCm40_30;data.byte2 = GXCm20_10;
#else
	if (wideband_mode_ctrl == 0) {
	data.byte1 = GXCm40_30;data.byte2 = GXCm20_10;
	} else if (wideband_mode_ctrl == 1) {
		data.byte1 = GXCm40_30_w;data.byte2 = GXCm20_10_w;
	}
#endif	
	Legerity_GX_filter_coeff(slic_id, &data, 0);
	
	//write B FIR filter.For echo cancellation -------should check
#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226	
	data.byte1 = BCm32_22;data.byte2 = BCm12_33;data.byte3 = BCm23_13;data.byte4 = BCm34_24;
	data.byte5 = BCm14_35;data.byte6 = BCm25_15;data.byte7 = BCm36_26;data.byte8 = BCm16_37;
	data.byte9 = BCm27_17;data.byte10 = BCm38_28;data.byte11 = BCm18_39;data.byte12 = BCm29_19;
	data.byte13 = BCm310_210;data.byte14 = BCm110;
#else
	if (wideband_mode_ctrl == 0) {
		data.byte1 = BCm32_22;data.byte2 = BCm12_33;data.byte3 = BCm23_13;data.byte4 = BCm34_24;
		data.byte5 = BCm14_35;data.byte6 = BCm25_15;data.byte7 = BCm36_26;data.byte8 = BCm16_37;
		data.byte9 = BCm27_17;data.byte10 = BCm38_28;data.byte11 = BCm18_39;data.byte12 = BCm29_19;
		data.byte13 = BCm310_210;data.byte14 = BCm110;
	} else if (wideband_mode_ctrl == 1) {
		data.byte1 = BCm32_22_w;data.byte2 = BCm12_33_w;data.byte3 = BCm23_13_w;data.byte4 = BCm34_24_w;
		data.byte5 = BCm14_35_w;data.byte6 = BCm25_15_w;data.byte7 = BCm36_26_w;data.byte8 = BCm16_37_w;
		data.byte9 = BCm27_17_w;data.byte10 = BCm38_28_w;data.byte11 = BCm18_39_w;data.byte12 = BCm29_19_w;
		data.byte13 = BCm310_210_w;data.byte14 = BCm110_w;
	}
#endif	
	Legerity_B_FIR_filter_coeff(slic_id, &data, 0);
	
	//write B IIR filter.For echo cancellation -------should check
#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226	
	data.byte1 = BCm411_311;data.byte2 = BCm211_111;
#else
	if (wideband_mode_ctrl == 0) {
	data.byte1 = BCm411_311;data.byte2 = BCm211_111;
	} else if (wideband_mode_ctrl == 1) {
		data.byte1 = BCm411_311_w;data.byte2 = BCm211_111_w;
	}
#endif	
	Legerity_B_IIR_filter_coeff(slic_id, &data, 0);
	
	//write Z FIR filter.For impedance matching -------should check
#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226	
	data.byte1 = ZCm40_30;data.byte2 = ZCm20_10;data.byte3 = ZCm41_31;data.byte4 = ZCm21_11;
	data.byte5 = ZCm42_32;data.byte6 = ZCm22_12;data.byte7 = ZCm43_33;data.byte8 = ZCm23_13;
	data.byte9 = ZCm44_34;data.byte10 = ZCm24_14;
#else
	if (wideband_mode_ctrl == 0) {
	data.byte1 = ZCm40_30;data.byte2 = ZCm20_10;data.byte3 = ZCm41_31;data.byte4 = ZCm21_11;
	data.byte5 = ZCm42_32;data.byte6 = ZCm22_12;data.byte7 = ZCm43_33;data.byte8 = ZCm23_13;
	data.byte9 = ZCm44_34;data.byte10 = ZCm24_14;
	} else if (wideband_mode_ctrl == 1) {
		data.byte1 = ZCm40_30_w;data.byte2 = ZCm20_10_w;data.byte3 = ZCm41_31_w;data.byte4 = ZCm21_11_w;
		data.byte5 = ZCm42_32_w;data.byte6 = ZCm22_12_w;data.byte7 = ZCm43_33_w;data.byte8 = ZCm23_13_w;
		data.byte9 = ZCm44_34_w;data.byte10 = ZCm24_14_w;
	}
#endif	
	Legerity_Z_FIR_filter_coeff(slic_id, &data, 0);
	
	//write Z IIR filter.For impedance matching -------should check
#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226	
	data.byte1 = ZCm45_35;data.byte2 = ZCm25_15;data.byte3 = ZCm26_16;data.byte4 = ZCm47_37;
	data.byte5 = ZCm27_17;
#else
	if (wideband_mode_ctrl == 0) {
	data.byte1 = ZCm45_35;data.byte2 = ZCm25_15;data.byte3 = ZCm26_16;data.byte4 = ZCm47_37;
	data.byte5 = ZCm27_17;
	} else if (wideband_mode_ctrl == 1) {
		data.byte1 = ZCm45_35_w;data.byte2 = ZCm25_15_w;data.byte3 = ZCm26_16_w;data.byte4 = ZCm47_37_w;
		data.byte5 = ZCm27_17_w;
	}
#endif	
	Legerity_Z_IIR_filter_coeff(slic_id, &data, 0);
	
	//write DISN filter. For impedance matching --------should check
#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226	
	data.byte1 = DISN;
#else
	if (wideband_mode_ctrl == 0)
	data.byte1 = DISN;
	else if (wideband_mode_ctrl == 1)
		data.byte1 = DISN_w;
#endif		
	Legerity_DISN(slic_id, &data, 0);
	
	//unmask Hook state bit. MHOOK of 6C/6Dh
	Legerity_interrupt_mask_bit(slic_id, MHOOK, 0);
	
	//set system state to disconnect state for other state changing.
	Legerity_system_state(slic_id, DISCONNECT, 0);
	
	//set system state to on-hook state.
	Legerity_system_state(slic_id, LOWER_POWER_IDLE, 0);
	
	return;
}

//General purpose IO direction setting. IO1_dir and IO2_dir : 0-input, 1-output.
void Legerity_GPIO_dir_set(unsigned char slic_id, unsigned char IO1_dir, unsigned char IO2_dir) {
	Le88xxx data;
	
	slic_order = slic_id;
	if ((IO1_dir > 1) || (IO2_dir > 1)) {
		printk("IO_dir only be 0:input or 1:output.\n");
		return;
	}	
	data.byte1 = ((IO2_dir & 0x01)<<2) | (IO1_dir & 0x01); 
	writeLegerityReg( 0x54, &data); 
	
	return;
}

//General purpose IO data. IO1_data and IO2_data : 0-pull low, 1-pull high.
void Legerity_GPIO_data(unsigned char slic_id, unsigned char IO1_data, unsigned char IO2_data) {
	Le88xxx data;
	
	slic_order = slic_id;
	if ((IO1_data > 1) || (IO2_data > 1)) {
		printk("IO_data only be 0:low or 1:high.\n");
		return;
	}
	data.byte1 = ((IO2_data & 0x01)<<1) | (IO1_data & 0x01); 
	writeLegerityReg( 0x52, &data);
	
	return;
}

/*DTMF caller ID generator C and D coefficient***********/
static void Legerity_DTMF_coeff(unsigned char slic_id, char data)
{
	Le88xxx data_temp;
	slic_order = slic_id;	
	/*disable generator C and D.*/
	data_temp.byte1 = 0x8C;
	Legerity_generator_disable_bit(slic_id, &data_temp);
	
	switch (data) {
		case '1':
			{
			data_temp.byte1 = DTMF_1>>24;data_temp.byte2 = DTMF_1>>16;
			data_temp.byte3 = AMP>>8;data_temp.byte4 = AMP;	
			data_temp.byte5 = DTMF_1>>8;data_temp.byte6 = DTMF_1;
			data_temp.byte7 = AMP>>8;data_temp.byte8 = AMP;
			}
			break;
		case '2':	
			{
			data_temp.byte1 = DTMF_2>>24;data_temp.byte2 = DTMF_2>>16;
			data_temp.byte3 = AMP>>8;data_temp.byte4 = AMP;	
			data_temp.byte5 = DTMF_2>>8;data_temp.byte6 = DTMF_2;
			data_temp.byte7 = AMP>>8;data_temp.byte8 = AMP;
			}
			break;
		case '3':	
			{
			data_temp.byte1 = DTMF_3>>24;data_temp.byte2 = DTMF_3>>16;
			data_temp.byte3 = AMP>>8;data_temp.byte4 = AMP;	
			data_temp.byte5 = DTMF_3>>8;data_temp.byte6 = DTMF_3;
			data_temp.byte7 = AMP>>8;data_temp.byte8 = AMP;
			}
			break;
		case '4':	
			{
			data_temp.byte1 = DTMF_4>>24;data_temp.byte2 = DTMF_4>>16;
			data_temp.byte3 = AMP>>8;data_temp.byte4 = AMP;	
			data_temp.byte5 = DTMF_4>>8;data_temp.byte6 = DTMF_4;
			data_temp.byte7 = AMP>>8;data_temp.byte8 = AMP;
			}
			break;
		case '5':	
			{
			data_temp.byte1 = DTMF_5>>24;data_temp.byte2 = DTMF_5>>16;
			data_temp.byte3 = AMP>>8;data_temp.byte4 = AMP;	
			data_temp.byte5 = DTMF_5>>8;data_temp.byte6 = DTMF_5;
			data_temp.byte7 = AMP>>8;data_temp.byte8 = AMP;
			}
			break;
		case '6':	
			{
			data_temp.byte1 = DTMF_6>>24;data_temp.byte2 = DTMF_6>>16;
			data_temp.byte3 = AMP>>8;data_temp.byte4 = AMP;	
			data_temp.byte5 = DTMF_6>>8;data_temp.byte6 = DTMF_6;
			data_temp.byte7 = AMP>>8;data_temp.byte8 = AMP;
			}
			break;
		case '7':	
			{
			data_temp.byte1 = DTMF_7>>24;data_temp.byte2 = DTMF_7>>16;
			data_temp.byte3 = AMP>>8;data_temp.byte4 = AMP;	
			data_temp.byte5 = DTMF_7>>8;data_temp.byte6 = DTMF_7;
			data_temp.byte7 = AMP>>8;data_temp.byte8 = AMP;
			}
			break;					
		case '8':	
			{
			data_temp.byte1 = DTMF_8>>24;data_temp.byte2 = DTMF_8>>16;
			data_temp.byte3 = AMP>>8;data_temp.byte4 = AMP;	
			data_temp.byte5 = DTMF_8>>8;data_temp.byte6 = DTMF_8;
			data_temp.byte7 = AMP>>8;data_temp.byte8 = AMP;
			}
			break;
		case '9':	
			{
			data_temp.byte1 = DTMF_9>>24;data_temp.byte2 = DTMF_9>>16;
			data_temp.byte3 = AMP>>8;data_temp.byte4 = AMP;	
			data_temp.byte5 = DTMF_9>>8;data_temp.byte6 = DTMF_9;
			data_temp.byte7 = AMP>>8;data_temp.byte8 = AMP;
			}
			break;
		case '0':	
			{
			data_temp.byte1 = DTMF_0>>24;data_temp.byte2 = DTMF_0>>16;
			data_temp.byte3 = AMP>>8;data_temp.byte4 = AMP;	
			data_temp.byte5 = DTMF_0>>8;data_temp.byte6 = DTMF_0;
			data_temp.byte7 = AMP>>8;data_temp.byte8 = AMP;
			}
			break;
		case 'A':	
			{
			data_temp.byte1 = DTMF_A>>24;data_temp.byte2 = DTMF_A>>16;
			data_temp.byte3 = AMP>>8;data_temp.byte4 = AMP;	
			data_temp.byte5 = DTMF_A>>8;data_temp.byte6 = DTMF_A;
			data_temp.byte7 = AMP>>8;data_temp.byte8 = AMP;
			}
			break;
		case 'B':	
			{
			data_temp.byte1 = DTMF_B>>24;data_temp.byte2 = DTMF_B>>16;
			data_temp.byte3 = AMP>>8;data_temp.byte4 = AMP;	
			data_temp.byte5 = DTMF_B>>8;data_temp.byte6 = DTMF_B;
			data_temp.byte7 = AMP>>8;data_temp.byte8 = AMP;
			}
			break;
		case 'C':	
			{
			data_temp.byte1 = DTMF_C>>24;data_temp.byte2 = DTMF_C>>16;
			data_temp.byte3 = AMP>>8;data_temp.byte4 = AMP;	
			data_temp.byte5 = DTMF_C>>8;data_temp.byte6 = DTMF_C;
			data_temp.byte7 = AMP>>8;data_temp.byte8 = AMP;
			}
			break;					
		case 'D':	
			{
			data_temp.byte1 = DTMF_D>>24;data_temp.byte2 = DTMF_D>>16;
			data_temp.byte3 = AMP>>8;data_temp.byte4 = AMP;	
			data_temp.byte5 = DTMF_D>>8;data_temp.byte6 = DTMF_D;
			data_temp.byte7 = AMP>>8;data_temp.byte8 = AMP;
			}
			break;
		default:
			printk("No such DTMF frequency.\n");
			break;
	}/*end switch*/	
	Legerity_generator_C_D_coeff(slic_id, &data_temp, 0);
	
	/*enable generator C and D .*/
	data_temp.byte1 = 0x0C;
	Legerity_generator_enable_bit(slic_id, &data_temp, 0);
	return;
}

void Legerity_Set_ONHOOK_Trans(unsigned char slic_id)
{
	slic_order = slic_id;
	/*force slic into ACTIVE_MID_BATTERY state(on hook transmission).*/
	Legerity_system_state( slic_id, ACTIVE_MID_BATTERY, 0);
}
	
/*DTMF caller ID main function.**************************/
void Legerity_DTMF_CallerID_main(unsigned char slic_id, char *data)
{
	Le88xxx data_temp;
	int i;
	slic_order = slic_id;
	
	/*force slic into ACTIVE_MID_BATTERY state(on hook transmission).*/
	Legerity_system_state( slic_id, ACTIVE_MID_BATTERY, 0);
	
	/*Start code*/
	Legerity_DTMF_coeff(slic_id,'D');
	
	/*digital transmission sequence*/
	i = 0;
	while (data[i] != 0) 
		Legerity_DTMF_coeff(slic_id, data[i++]);
	
	/*End code*/
	Legerity_DTMF_coeff(slic_id,'C');
	 
	/*Disable generator C and D*/
	data_temp.byte1 = 0x0C;
	Legerity_generator_disable_bit(slic_id, &data_temp);

	return;	
}


void SLIC_Set_Ring_Cadence_Legerity(unsigned char slic__order, unsigned short on_msec, unsigned short off_msec)
{
	Le88xxx data;
	//write cadence timer coeff.
	data.byte1 = (on_msec/5)>>8;
	data.byte2 = (on_msec/5);
	data.byte3 = (off_msec/5)>>8;
	data.byte4 = (off_msec/5);
	Legerity_cadence_timer_coeff(slic__order, &data, 0);
}


TstLe88221Impendance imp_tbl[] = 
{
	//USA (600ohm)
	{0xEA, {0xBA, 0xEB, 0x2A, 0x2C, 0xB5, 0x25, 0xAA, 0x24, 0x2C, 0x3D}, {0xAA, 0xBA, 0x27, 0x9F, 0x01}, {0x2D, 0x01, 0x2B, 0xB0, 0x5A, 0x33, 0x24, 0x5C, 0x35, 0xA4, 0x5A, 0x3D, 0x33, 0xB6}, {0x3A, 0x10, 0x3D, 0x3D, 0xB2, 0xA7, 0x6B, 0xA5, 0x2A, 0xCE, 0x2A, 0x8F}, {0xA8, 0x71}, {0xA9, 0xF0}, 0x00, {0x2A, 0x42, 0x22, 0x4B, 0x1C, 0xA3, 0xA8, 0xFF, 0x8F, 0xAA, 0xF5, 0x9F, 0xBA, 0xF0}, {0x2E, 0x01}},
	//UK (300ohm + 1000ohm//220nF )
	{0xD8, {0xA8, 0xF1, 0xA4, 0xA1, 0x24, 0x34, 0xA2, 0xCB, 0x22, 0x97}, {0x2E, 0xB3, 0xB5, 0x2E, 0x01}, {0xA2, 0xD0, 0x4C, 0x10, 0xA4, 0xA8, 0x22, 0xA9, 0xF5, 0xB3, 0xAA, 0xAC, 0x43, 0x27}, {0x26, 0xD0, 0x2A, 0x52, 0xC3, 0xA9, 0xCA, 0xA2, 0xC3, 0xAB, 0xB4, 0x55}, {0xA8, 0x71}, {0xA9, 0xF0}, 0x00, {0xA3, 0x5B, 0x32, 0x2C, 0x12, 0x22, 0xEB, 0x2A, 0x23, 0xAA, 0x3A, 0x24, 0x2B, 0x40}, {0xD4, 0xA0}},
	//AUSTRALIA (220ohm + 820ohm//120nF)
	{0xDD, {0xF3, 0xA0, 0xAA, 0x62, 0x22, 0xBA, 0x34, 0x3A, 0xA2, 0xA2}, {0x2A, 0xA2, 0xA7, 0x9F, 0x01}, {0x2D, 0x01, 0xB4, 0x10, 0xAA, 0x38, 0x2A, 0xA2, 0xAB, 0x7E, 0xAA, 0xAE, 0xAA, 0x87}, {0xAE, 0x50, 0xBE, 0x2D, 0xA4, 0xBC, 0xCC, 0x8F, 0x5A, 0x45, 0x2A, 0xAD}, {0xA8, 0x71}, {0xA9, 0xF0}, 0x00, {0x33, 0x53, 0x32, 0xA4, 0x1A, 0xA1, 0xAB, 0x2B, 0xB3, 0xAB, 0x43, 0xE5, 0xA5, 0x60}, {0x5F, 0xC1}},
	//HK (use USA)
	{0xEA, {0xBA, 0xEB, 0x2A, 0x2C, 0xB5, 0x25, 0xAA, 0x24, 0x2C, 0x3D}, {0xAA, 0xBA, 0x27, 0x9F, 0x01}, {0x2D, 0x01, 0x2B, 0xB0, 0x5A, 0x33, 0x24, 0x5C, 0x35, 0xA4, 0x5A, 0x3D, 0x33, 0xB6}, {0x3A, 0x10, 0x3D, 0x3D, 0xB2, 0xA7, 0x6B, 0xA5, 0x2A, 0xCE, 0x2A, 0x8F}, {0xA8, 0x71}, {0xA9, 0xF0}, 0x00, {0x2A, 0x42, 0x22, 0x4B, 0x1C, 0xA3, 0xA8, 0xFF, 0x8F, 0xAA, 0xF5, 0x9F, 0xBA, 0xF0}, {0x2E, 0x01}},
	//Japan (600ohm + 1000nF)
	{0xEA, {0x4B, 0xA2, 0x2B, 0x27, 0xAD, 0xBB, 0x4C, 0xAC, 0x2A, 0x22}, {0x2A, 0xA3, 0xA2, 0x32, 0xD0}, {0xAA, 0xB0, 0xDA, 0xB0, 0x72, 0x4E, 0x22, 0x2D, 0x22, 0x2D, 0xA2, 0xAC, 0xB2, 0x3F}, {0x2A, 0x20, 0x7C, 0xA3, 0x2A, 0xAA, 0x2B, 0x54, 0xA2, 0x2B, 0xBA, 0x87}, {0xA8, 0x71}, {0xA9, 0xF0}, 0x00, {0xDA, 0x52, 0x22, 0x9F, 0x1A, 0xA2, 0x23, 0x4B, 0xA3, 0x22, 0x4A, 0xA4, 0x5C, 0x40}, {0x32, 0xD0}},
	//Sweden (200ohm + 1000ohm//100nF)
	{0xE4, {0x98, 0x90, 0x53, 0xC2, 0x2C, 0x4A, 0xA2, 0x2A, 0xA4, 0xC2}, {0x98, 0xA2, 0x97, 0x9F, 0x01}, {0x2D, 0xD0, 0xAD, 0x10, 0x3A, 0x28, 0x2A, 0x22, 0x26, 0xAD, 0x52, 0x6D, 0xBA, 0x37}, {0xAA, 0xC0, 0x52, 0xCD, 0x2D, 0xAC, 0x6A, 0x8F, 0xDA, 0x25, 0xA8, 0xFD}, {0xA8, 0x71}, {0xA9, 0xF0}, 0x00, {0x23, 0x52, 0x22, 0xA2, 0x1A, 0xB1, 0xAA, 0x2A, 0x44, 0xA2, 0x62, 0x27, 0xC8, 0xF0}, {0x2E, 0x01}},
	//Germany (220ohm + 820ohm//115nF)
	{0xDD, {0xA4, 0xA0, 0x2B, 0xE2, 0x2E, 0xCA, 0x2D, 0x2A, 0x4A, 0xA2}, {0x98, 0xA2, 0x97, 0x9F, 0x01}, {0x2D, 0xD0, 0xF5, 0x10, 0xBB, 0x38, 0x2A, 0xE2, 0xBA, 0xAD, 0xB8, 0x7E, 0x23, 0x97}, {0xC4, 0x50, 0xA2, 0xAC, 0x2A, 0xAC, 0xBA, 0xA7, 0xA9, 0xF5, 0x32, 0xBD}, {0xA8, 0x71}, {0xA9, 0xF0}, 0x00, {0x23, 0x52, 0x32, 0x44, 0x1A, 0xA1, 0xBB, 0x2A, 0xA3, 0xAA, 0x4A, 0xA5, 0xFB, 0x60}, {0xDB, 0xA1}},
	//France(215ohm + 1000ohm//137nF)
	{0xD9, {0x2A, 0xA0, 0x33, 0xA1, 0xB2, 0xCA, 0x97, 0xA9, 0xAC, 0xB2}, {0x2A, 0xA2, 0x37, 0x1F, 0x01}, {0x72, 0xD0, 0x33, 0x10, 0x24, 0x38, 0x26, 0xA5, 0xBD, 0x33, 0xAC, 0xAB, 0xDC, 0xA5}, {0xAA, 0x60, 0xA2, 0xBC, 0xA2, 0xCC, 0xAE, 0x87, 0xBB, 0x35, 0x22, 0xBD}, {0xA8, 0x71}, {0xA9, 0xF0}, 0x00, {0x2A, 0x4F, 0x22, 0x36, 0x12, 0xA1, 0xD3, 0x32, 0x25, 0x42, 0x7A, 0x87, 0xD8, 0xF0}, {0x2E, 0x01}},
	//TR57 (use USA)
	{0xEA, {0xBA, 0xEB, 0x2A, 0x2C, 0xB5, 0x25, 0xAA, 0x24, 0x2C, 0x3D}, {0xAA, 0xBA, 0x27, 0x9F, 0x01}, {0x2D, 0x01, 0x2B, 0xB0, 0x5A, 0x33, 0x24, 0x5C, 0x35, 0xA4, 0x5A, 0x3D, 0x33, 0xB6}, {0x3A, 0x10, 0x3D, 0x3D, 0xB2, 0xA7, 0x6B, 0xA5, 0x2A, 0xCE, 0x2A, 0x8F}, {0xA8, 0x71}, {0xA9, 0xF0}, 0x00, {0x2A, 0x42, 0x22, 0x4B, 0x1C, 0xA3, 0xA8, 0xFF, 0x8F, 0xAA, 0xF5, 0x9F, 0xBA, 0xF0}, {0x2E, 0x01}},
	//Belgium (150ohm + 830ohm//72nF)
	{0xE6, {0x73, 0x21, 0x23, 0x23, 0x4E, 0x5A, 0xB3, 0x3A, 0xAA, 0xB2}, {0x98, 0x24, 0x97, 0x9F, 0x01}, {0x2D, 0x01, 0xBA, 0x10, 0x2A, 0x48, 0xA2, 0xB1, 0xD2, 0x4B, 0x33, 0xA5, 0x3C, 0xAF}, {0xB9, 0xF0, 0x32, 0xAE, 0xCA, 0xAC, 0xCA, 0x8F, 0x44, 0x25, 0x23, 0x3D}, {0xA8, 0x71}, {0xA9, 0xF0}, 0x00, {0x2A, 0x42, 0x22, 0x24, 0x1A, 0x22, 0x97, 0x4B, 0x3F, 0x2A, 0xFA, 0x87, 0x29, 0xF0}, {0x2E, 0x01}},
	//Finland (270ohm + 910ohm//120nF)
	{0xE8, {0x3A, 0x41, 0x6D, 0x42, 0x24, 0xCB, 0x2A, 0xCA, 0x32, 0xD3}, {0x98, 0xA2, 0x97, 0x9F, 0x01}, {0x2D, 0x01, 0xAA, 0x10, 0xAB, 0xC8, 0xB3, 0xA3, 0x22, 0xA5, 0x2E, 0x2E, 0x3B, 0x97}, {0xAC, 0xD0, 0x42, 0x3D, 0x54, 0xAC, 0xAE, 0x87, 0x3A, 0xA5, 0x35, 0x3E}, {0xA8, 0x71}, {0xA9, 0xF0}, 0x00, {0xC2, 0x54, 0x22, 0xA2, 0x13, 0xC1, 0x33, 0x22, 0x23, 0xDA, 0x3A, 0xB4, 0xD3, 0x50}, {0xA2, 0x41}},
	//Italy (180ohm + 630ohm//60nF)
	{0xE9, {0x32, 0xA1, 0xA5, 0x73, 0x33, 0xCB, 0xA2, 0x2B, 0x2B, 0xA3}, {0x87, 0xCB, 0x97, 0x9F, 0x01}, {0xDD, 0x01, 0xA3, 0x30, 0xBA, 0xA9, 0xB3, 0x5C, 0xEB, 0xA2, 0x2C, 0x4B, 0x26, 0x25}, {0x3A, 0x30, 0x4A, 0x8F, 0xB3, 0xAC, 0x2A, 0x8F, 0xCA, 0x55, 0xAB, 0xAD}, {0xA8, 0x71}, {0xA9, 0xF0}, 0x00, {0xA2, 0x52, 0x22, 0xA3, 0x1D, 0x32, 0xAC, 0x42, 0x5F, 0x42, 0xF3, 0xAF, 0xAA, 0xF0}, {0x2E, 0x01}},
	//China (200ohm + 680ohm//100nF)
	{0xDA, {0xA2, 0xA0, 0x2A, 0xA2, 0xA8, 0xFA, 0xAA, 0x2A, 0x24, 0xB2}, {0x98, 0x53, 0x97, 0x9F, 0x01}, {0x1D, 0x01, 0x2C, 0x10, 0x3A, 0xC8, 0xBA, 0xC2, 0xF2, 0xBE, 0x2D, 0xBD, 0x9F, 0xB7}, {0xBA, 0x30, 0x62, 0x3D, 0x32, 0xBC, 0xAC, 0xA7, 0xCE, 0x45, 0x39, 0xFD}, {0xA8, 0x71}, {0xA9, 0xF0}, 0x00, {0xA3, 0x5B, 0x32, 0x97, 0x13, 0xA1, 0x62, 0x3A, 0xA4, 0x24, 0x64, 0xA7, 0xA8, 0xF0}, {0x2E, 0x01}}
	
};

void Legerity_Set_Impendance(unsigned char slic_id, unsigned short country, unsigned short impd)
{
	Le88xxx data;
	
	//write DISN filter. For impedance matching 
 	data.byte1 = imp_tbl[country]._DISN;
 	Legerity_DISN(slic_id, &data, 0);
 	
 	//write Z FIR filter.For impedance matching 
	data.byte1 = imp_tbl[country]._ZFIR[0];	data.byte2 = imp_tbl[country]._ZFIR[1];
	data.byte3 = imp_tbl[country]._ZFIR[2];	data.byte4 = imp_tbl[country]._ZFIR[3];
 	data.byte5 = imp_tbl[country]._ZFIR[4];	data.byte6 = imp_tbl[country]._ZFIR[5];
 	data.byte7 = imp_tbl[country]._ZFIR[6];	data.byte8 = imp_tbl[country]._ZFIR[7];
 	data.byte9 = imp_tbl[country]._ZFIR[8];	data.byte10 = imp_tbl[country]._ZFIR[9];
 	Legerity_Z_FIR_filter_coeff(slic_id, &data, 0);
 	
 	//write Z IIR filter.For impedance matching 
 	data.byte1 = imp_tbl[country]._ZIIR[0];	data.byte2 = imp_tbl[country]._ZIIR[1];
 	data.byte3 = imp_tbl[country]._ZIIR[2];	data.byte4 = imp_tbl[country]._ZIIR[3];
 	data.byte5 = imp_tbl[country]._ZIIR[4];
 	Legerity_Z_IIR_filter_coeff(slic_id, &data, 0);
 	
 	 //write R filter.
	data.byte1 = imp_tbl[country]._RF[0];	data.byte2 = imp_tbl[country]._RF[1];
	data.byte3 = imp_tbl[country]._RF[2];	data.byte4 = imp_tbl[country]._RF[3];
 	data.byte5 = imp_tbl[country]._RF[4];	data.byte6 = imp_tbl[country]._RF[5];
 	data.byte7 = imp_tbl[country]._RF[6];	data.byte8 = imp_tbl[country]._RF[7];
 	data.byte9 = imp_tbl[country]._RF[8];	data.byte10 = imp_tbl[country]._RF[9];
 	data.byte11 = imp_tbl[country]._RF[10];	data.byte12 = imp_tbl[country]._RF[11];
 	data.byte13 = imp_tbl[country]._RF[12];	data.byte14 = imp_tbl[country]._RF[13];
 	Legerity_R_filter_coeff(slic_id, &data, 0);
 	
 	 //write X filer.
 	data.byte1 = imp_tbl[country]._XF[0];	data.byte2 = imp_tbl[country]._XF[1];
 	data.byte3 = imp_tbl[country]._XF[2];	data.byte4 = imp_tbl[country]._XF[3];
 	data.byte5 = imp_tbl[country]._XF[4];	data.byte6 = imp_tbl[country]._XF[5];
 	data.byte7 = imp_tbl[country]._XF[6];	data.byte8 = imp_tbl[country]._XF[7];
 	data.byte9 = imp_tbl[country]._XF[8];	data.byte10 = imp_tbl[country]._XF[9];
 	data.byte11 = imp_tbl[country]._XF[10];	data.byte12 = imp_tbl[country]._XF[11];
 	Legerity_X_filter_coeff(slic_id, &data, 0);
 	
 	 //write GR filter.
 	data.byte1 = imp_tbl[country]._GR[0];	data.byte2 = imp_tbl[country]._GR[1];
 	Legerity_GR_filter_coeff(slic_id, &data, 0);
 	
	 //write GX filter.
 	data.byte1 = imp_tbl[country]._GX[0];	data.byte2 = imp_tbl[country]._GX[1];
 	Legerity_GX_filter_coeff(slic_id, &data, 0);
 	
 	//write TX and RX gain.
 	data.byte1 = imp_tbl[country]._ANLG;
 	Legerity_tx_rx_gain(slic_id, &data, 0);
 	
 	 //write B FIR filter.For echo cancellation 
 	data.byte1 = imp_tbl[country]._BFIR[0];	data.byte2 = imp_tbl[country]._BFIR[1];
 	data.byte3 = imp_tbl[country]._BFIR[2];	data.byte4 = imp_tbl[country]._BFIR[3];
 	data.byte5 = imp_tbl[country]._BFIR[4];	data.byte6 = imp_tbl[country]._BFIR[5];
 	data.byte7 = imp_tbl[country]._BFIR[6];	data.byte8 = imp_tbl[country]._BFIR[7];
 	data.byte9 = imp_tbl[country]._BFIR[8];	data.byte10 = imp_tbl[country]._BFIR[9];
 	data.byte11 = imp_tbl[country]._BFIR[10];	data.byte12 = imp_tbl[country]._BFIR[11];
 	data.byte13 = imp_tbl[country]._BFIR[12];	data.byte14 = imp_tbl[country]._BFIR[13];
 	Legerity_B_FIR_filter_coeff(slic_id, &data, 0);
 
 	//write B IIR filter.For echo cancellation 
 	data.byte1 = imp_tbl[country]._BIIR[0];	data.byte2 = imp_tbl[country]._BIIR[1];
 	Legerity_B_IIR_filter_coeff(slic_id, &data, 0);
	
}

void SLIC_Set_Impendance_Legerity(unsigned char slic__order, unsigned short country, unsigned short impd)
{
	unsigned long flags;
	save_flags(flags); cli();

	switch (country)
	{
		case COUNTRY_USA:
		case COUNTRY_AUSTRALIA:
		case COUNTRY_GR:
		case COUNTRY_JP:
		case COUNTRY_CN:
		case COUNTRY_UK:
		case COUNTRY_SE:
		case COUNTRY_FR:
		case COUNTRY_BE:
		case COUNTRY_FL:
		case COUNTRY_IT:
			Legerity_Set_Impendance(slic__order, country, impd);
			break;
			
		case COUNTRY_HK:
		case COUNTRY_TR:
			Legerity_Set_Impendance(slic__order, country, impd);
			PRINT_MSG(" Not support impedance of this country. Set to default SLIC impedance 600 ohm.\n");
			break;
		default:
			Legerity_Set_Impendance(slic__order, country, impd);
			PRINT_MSG(" Not support impedance of this country. Set to default SLIC impedance 600 ohm.\n");
			break;
	}
	
	restore_flags(flags);
}

void SendNTTCAR_Legerity(int chid)
{
	Le88xxx data[3],r_data;
	
	int protect_cnt = 0;
	
	int i, b_num[]={0xD2, 0xDE, 0xE0};	//back up ring setting register
	Le88xxx b_data[3];
			



	slic_order = chid;

	//printk("slic_order=%d",slic_order);

	/************** backup the register ***************/
	for ( i=0;i<3;i++)
		readLegerityReg( b_num[i], &b_data[i]);

	//0xD2 ring voltage / freq seting. ring 17HZ. 75vrms = 106.5V
	data[0].byte1 = 0;
	data[0].byte2 = 0;
	data[0].byte3 = 0;
	data[0].byte4 = 0;
	data[0].byte5 = 0x2E;
	data[0].byte6 = 0x57;
	data[0].byte7 = 0xF4;
	data[0].byte8 = 0;
	data[0].byte9 = 0;
	data[0].byte10 = 0;
	data[0].byte11 = 0;
	
	//0xDE  signal Generator control , enable ring cadence
	data[1].byte1 = 0x81;
	
	//0xE0 ring cadence timer 0.5s on, 0.5s off
	data[2].byte1 = 0;
	data[2].byte2 = 0x64;
	data[2].byte3 = 0;
	data[2].byte4 = 0x64;
	/*********** To Create Short Ring *****************/
	for ( i=0;i<3;i++)
		writeLegerityReg( (unsigned char)b_num[i], &data[i]);


	//set ring state
	readLegerityReg( 0x56, &r_data);
	r_data.byte1 = (r_data.byte1&0xF0) | 0x07;
	writeLegerityReg( 0x56, &r_data);

	mdelay(100); // delay for a while


	/*********** Check Phone Hook State ***************/

	readLegerityReg( 0x4F, &r_data);
	if(chid==0)
	{
		if (!(r_data.byte1&0x01)) // if phone on-hook
			while(!(r_data.byte1&0x01))  //wait phone off-hook atuomatically
			{
				if (protect_cnt == 30)	// wait 6 sec, if no off-hook-> break to prevent watch dog reset.
					break;
				mdelay(200);
				protect_cnt ++;
				readLegerityReg( 0x4F, &r_data);
			}
	}
	else
	{
		if (!(r_data.byte2&0x01)) // if phone on-hook
			while(!(r_data.byte2&0x01))  //wait phone off-hook atuomatically
			{
				if (protect_cnt == 30)	// wait 6 sec, if no off-hook-> break to prevent watch dog reset.
					break;
				mdelay(200);
				protect_cnt ++;
				readLegerityReg( 0x4F, &r_data);
			}
	
	}
	/******* Set Reverse On-Hook Transmission *********/		
	
	Legerity_OnHookLineReversal(slic_order, 1); // if phone off-hook, set Reverse On-Hook Transmission
	
	/************** restore the register ***************/
	for ( i=0;i<3;i++)
		writeLegerityReg( (unsigned char)b_num[i], &b_data[i]);


	
	//printk("end_ntt,protect_cnt=%d\n",protect_cnt);
}

void Legerity_OnHookLineReversal(unsigned char slic_id, unsigned char bReversal)
{
	Le88xxx data_temp;

	slic_order = slic_id;
	readLegerityReg( 0x56, &data_temp);
	//mdelay(10);

	if(bReversal)
	{
		data_temp.byte1 |= 0x10;
	}
	else
	{		
		data_temp.byte1 &= 0xEF;
	}
	writeLegerityReg( 0x56, &data_temp);
	Legerity_Set_ONHOOK_Trans(slic_id);

	return;
}

#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226
//select:0-> Normal Codec mode, 1-> Wideband Codec mode
void Legerity_Wideband_ctrl(unsigned char select)
{
	
	wideband_mode_ctrl = select;
	if (select == 0) 		
		printk("Don't support Wideband mode\n");
	else if (select == 1)	
		printk("Support Wideband mode\n");

	return;
}
/* Register 0x70 Bit6 for Cutoff Receive Path
        0: Receive path connected
        1: Receive path cutoff
*/
 
void Legerity_slic_set_rx_pcm(int chid, int enable)//PCM TX connect to SLIC Rx
{
        int reg_val;
        Le88xxx data;
        unsigned long flags;
 
        save_flags(flags); cli();
 
        slic_order = chid;
 
        readLegerityReg(0x70, &data);
        //printk("reg 0x70 val = 0x%x\n", data.byte1);
 
        if (enable == 0)
        {
                data.byte1 = data.byte1 | 0x40;
                writeLegerityReg( 0x70, &data);
        }
        else if (enable == 1)
        {
                data.byte1 = data.byte1 & 0xBF;
                writeLegerityReg( 0x70, &data);
        }
 
        //readLegerityReg(0x70, &data);
        //printk("reg 0x70 val = 0x%x\n", data.byte1);
 
        restore_flags(flags);
}

/* Register 0x70 Bit7 for Cutoff Receive Path
        0: Transmit path connected
        1: Transmit path cut off
*/
void Legerity_slic_set_tx_pcm(int chid, int enable)//PCM RX connect to SLIC Tx
{
        int reg_val;
        Le88xxx data;
        unsigned long flags;
 
        save_flags(flags); cli();
 
        slic_order = chid;
 
        readLegerityReg(0x70, &data);
        //printk("reg 0x70 val = 0x%x\n", data.byte1);
 
        if (enable == 0)
        {
                data.byte1 = data.byte1 | 0x80;
                writeLegerityReg( 0x70, &data);
        }
        else if (enable == 1)
        {
                data.byte1 = data.byte1 & 0x7F;
                writeLegerityReg( 0x70, &data);
        }
 
        //readLegerityReg(0x70, &data);
        //printk("reg 0x70 val = 0x%x\n", data.byte1);
 
        restore_flags(flags);
}


#endif
