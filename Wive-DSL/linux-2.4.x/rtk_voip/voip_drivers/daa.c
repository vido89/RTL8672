#include "daa.h"
#include <linux/config.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/string.h>
#include "rtk_voip.h"
#include <asm/delay.h>

extern void writeDAAReg(unsigned int address, unsigned int data);
extern unsigned char readDAAReg(unsigned int address);
extern void cyg_thread_delay(int delay);

//---static function prototype-----------------------------//
static void Resistor_ADC_calibration(void);
static void device_revision(void);
static void tran_rec_full_scale(unsigned char scale);
static unsigned char system_line_connect(void);
static void ring_validation_DAA(unsigned char reg22,unsigned char reg23,unsigned char reg24);
static void caller_ID_pass(void);
//---------------------------------------------------------//

/*********** Select the Si3018 or Si3019 Country Specific register 16,26,30 and 31 *******/
#if 0
	#define _Si3018_CountryReg_
#else
	#define _Si3019_CountryReg_
#endif
/*****************************************************************/

/***************  PULSE DIALING ************************/
//For pulse dialing used, the value of FOH bit of reg31 must be set to mini vlaue.
//The resistor calibration (RCALD bit of reg25) and auto-calibration (CALD bit of reg17)
//must be disabled.
#define pulse_dialing 1 //0: pulse_dialing ,1: tone(DTMF)_dialing
/*********************************************/

/********************************************************************************/
//for caller ID pass through.It must be set in on-hook transmission state on slic.
#ifdef FXO_CALLER_ID_DET
#define CALLER_ID 1	//0: disable caller ID. 1: enable caller ID
#else
#define CALLER_ID 0
#endif
/********************************************************************************/

/******only for Si3019 used**********/
#define _Si3019_used_ 0
/************************************/



static void Resistor_ADC_calibration() {
	unsigned int reg17,i; 
	
	//disable resistor calibration
	//writeDAAReg(25,	readDAAReg(25)|0xa0);
	while((readDAAReg(25)&0x80)); //auto resistor calibration.
	//reg17 = readDAAReg(17) | 0x20;//disable auto ADC_calibration.CALD
	//writeDAAReg(17,reg17);
	reg17 = readDAAReg(17) | 0x40;//enable manual ADC_calibration.MCAL	
	writeDAAReg(17,reg17);
	//The duration of the calibration needs 256ms, then toggles 1 to 0.
	for (i=0;i<23000000;i++);
	reg17 = readDAAReg(17) & 0xbf;//No ADC_calibration.MCAL
	writeDAAReg(17,reg17);
	return;
	
}

static void device_revision() {
	unsigned int reg11,reg13;
	unsigned char temp;
	//char *version[]={"A","B","C","D"};
	reg11 = readDAAReg(11);	
	reg13 = readDAAReg(13);
	//verify the revision of Si3050
	temp = reg11 & 0x0f;
	switch (temp) {
		case 0:
			printk("System-side Si3050A ");
			break;	
		case 2:
			printk("System-side Si3050B ");
			break;
		case 3:
			printk("System-side Si3050C ");
			break;
		case 4:
			printk("System-side Si3050D ");
			break;
		default:
			printk("error vision");
	}
	//verify line-side device Si3018 or Si3019
	temp = (reg11 & 0xf0)>>4;
	if (temp == 1)
		printk(",Line-side Si3018");
	else if (temp == 3)
		printk(",Line-side Si3019");
	else
		printk(",Line-side ?");
	//verify the revision of Si3018/Si3019
	temp = (reg13>>2) & 0x0f;
	switch (temp) {
		case 1:
			printk("A\n");
			break;	
		case 2:
			printk("B\n");
			break;
		case 3:
			printk("C\n");
			break;
		case 4:
			printk("D\n");
			break;
		case 5:
			printk("E\n");
			break;
		case 6:
			printk("F\n");
			break;
		default:
			printk("error vision");
	}
	printk("\n");
	return;
}

static void tran_rec_full_scale(unsigned char scale) {
	
	if (scale == 1) {
		writeDAAReg(31,(readDAAReg(31) | 0x80)); //Transmit/receive full scale x1
		writeDAAReg(26,(readDAAReg(26) | 0xc0)); //set DCV[1:0]=3,MINI[1:0]=0
	} else if (scale == 2) {
		writeDAAReg(30,(readDAAReg(30) | 0x10)); //Transmit/receive full scale x2
		writeDAAReg(26,(readDAAReg(26) | 0xc0)); //set DCV[1:0]=3,MINI[1:0]=0    
	} else
		printk("Error full_scale ratio.\n");
	
	return;
	
}

static unsigned char system_line_connect(void) {
	
	if ((readDAAReg(12)>>6))
		return 1; //DAA system-side and line-side connect ok;
	else
		return 0; //DAA system-side and line-side connect fail;
}

void country_specific_termination(unsigned char ohs,unsigned char ohs2,unsigned char rz,
		unsigned char rt,unsigned char ilim,unsigned char dcv,unsigned char mini,unsigned char acim) {
	unsigned char reg16,reg26,reg30,reg31;	
	
	reg16 = readDAAReg(16) | (ohs<<6) | (rz<<1) | rt;
	writeDAAReg(16,reg16); //setup OHS,RZ and RT of register 16.
	
	reg26 = readDAAReg(26) | (ilim<<1) | (dcv<<6) | (mini<<4);	
	writeDAAReg(26,reg26); //setup LILM,DCV,and MINI of register 26.
	
	reg30 = readDAAReg(30) | acim;
	writeDAAReg(30,reg30); //setup ACIM of register 30.
	
	reg31 = readDAAReg(31) | (ohs2<<3);
	writeDAAReg(31,reg31); //setup OHS2 of register 31.
	printk("The setting of country specific registers is ok.\n");
	return;		
}

void DAA_hybrid_coeff(unsigned char acim,unsigned char coeff1,unsigned char coeff2,unsigned char coeff3,
		unsigned char coeff4,unsigned char coeff5,unsigned char coeff6,unsigned char coeff7,unsigned char coeff8) {
	unsigned char reg30;
	
	reg30 = readDAAReg(30) | acim;
	writeDAAReg(30,reg30); //setup ACIM of register 30.
	
	writeDAAReg(45,coeff1);//setup Hybrid coeff1 of register 45.	
	writeDAAReg(46,coeff2);//setup Hybrid coeff2 of register 46.
	writeDAAReg(47,coeff3);//setup Hybrid coeff3 of register 47.
	writeDAAReg(48,coeff4);//setup Hybrid coeff4 of register 48.
	writeDAAReg(49,coeff5);//setup Hybrid coeff5 of register 49.
	writeDAAReg(50,coeff6);//setup Hybrid coeff6 of register 50.
	writeDAAReg(51,coeff7);//setup Hybrid coeff7 of register 51.
	writeDAAReg(52,coeff8);//setup Hybrid coeff8 of register 52.
	return;			
} 

void DAA_Tx_Gain_ctrl(unsigned char tga2,unsigned char txg2,unsigned char tga3,unsigned char txg3) {
	unsigned char reg38,reg40;
	
	reg38 = (tga2<<4) | txg2; //tga2 = 0 means gaining up, tga2 = 1 means attenuating
	writeDAAReg(38,reg38);			   //txg2 = 0 means 0dB, txg2 = 1 means 1dB,...txg2 = 15 means 15dB
	
	reg40 = (tga3<<4) | txg3; //tga3 = 0 means gaining up, tga3 = 1 means attenuating
	writeDAAReg(40,reg40);			   //txg3 = 0 means 0dB, txg3 = 1 means 0.1dB,...txg3 = 15 means 0.15dB
	
	return;
}

void DAA_Rx_Gain_ctrl(unsigned char rga2,unsigned char rxg2,unsigned char rga3,unsigned char rxg3) {
	unsigned char reg39,reg41;
	
	reg39 = (rga2<<4) | rxg2; //rga2 = 0 means gaining up, rga2 = 1 means attenuating
	writeDAAReg(39,reg39);			   //rxg2 = 0 means 0dB, rxg2 = 1 means 1dB,...rxg2 = 15 means 15dB
	
	reg41 = (rga3<<4) | rxg3; //rga3 = 0 means gaining up, rga3 = 1 means attenuating
	writeDAAReg(41,reg41);			   //rxg3 = 0 means 0dB, rxg3 = 1 means 0.1dB,...rxg3 = 15 means 0.15dB
	
	return;
}

void DAA_Tx_Gain_Web(unsigned char gain) {
	static unsigned char gain_before = 10;
	
	if ( gain >= 7) 
		DAA_Tx_Gain_ctrl(0,(gain-7),0,0);
	else 
		DAA_Tx_Gain_ctrl(1,(7-gain),0,0);	
	gain_before = gain;
	return;	
}	

void DAA_Rx_Gain_Web(unsigned char gain) {
	static unsigned char gain_before = 10;
	
	if ( gain >= 7) 
		DAA_Rx_Gain_ctrl(0,(gain-7),0,0);
	else 
		DAA_Rx_Gain_ctrl(1,(7-gain),0,0);		
	gain_before = gain;
	return;	
}	

unsigned char going_off_hook(void) 
{
	unsigned int i;
	unsigned char reg31;
	
	if (!(system_line_connect())) {
		printk("system-side and line-side not connect\n");
	#if 0	
		writeDAAReg(6,0x10);//re-powerup line-side device.Set PDL bit of reg6
		for (i=0;i<100000;i++); //delay 1ms
		writeDAAReg(6,0x00);//clear PDL bit of reg6
	#else
		daa_init_all(0);
	#endif
		//return;
	}

	/* Daa must going off-hook, and then we can judge line is connected or not. */
	writeDAAReg(5,readDAAReg(5)|0x01); //set OH bit of register 5 to go off-hook.

	udelay(1500) ;
	if (!(readDAAReg(12)&0x1f) && (readDAAReg(19)&0x02)) 
	{
		writeDAAReg(5,readDAAReg(5)&0xfe); //set OH bit of register 5 to on-hook.
		printk("phone line not connect\n");
		return 0xff;	
	}

#if 0 //0: Don't check PSTN Line is used or not.
	/* Daa must going on-hook, and then we can judge line is busy or not. */	
	writeDAAReg(5,readDAAReg(5)&0xfe); //set OH bit of register 5 to on-hook.
	for (i=0;i<70000;i++); //delay 250us
	
	
	/* line busy ~ 12V(0x0c) , otherwise >= 48V(0x30) */
	if (readDAAReg(29) < 0x1a)  /* 0x1a ~ 26V */
	{
		printk("phone line busy\n");
		return 0xff; 
	}
	else
	{
		writeDAAReg(5,readDAAReg(5)|0x01); //set OH bit of register 5 to go off-hook.
		for (i=0;i<70000;i++); //delay 250us
	}
#endif

	while(!(readDAAReg(5)&0x01)); //Ensure line-side is going off-hook.
	//The following is transients to settle line-side device.Default delay 128ms.
	#if pulse_dialing
	reg31 = 1;//0:512ms 1:128ms 2:64ms 3:8ms.
	#else
	reg31 = 3;//0:512ms 1:128ms 2:64ms 3:8ms.
	#endif
	reg31 = readDAAReg(31) | (reg31<<5);
	writeDAAReg(31,reg31);
	for (i=0;i<11500000;i++); //delay 128ms
	#if pulse_dialing
	Resistor_ADC_calibration(); //calibration
	#endif
	return 1; //going off-hook ok
	 	
}

void on_hook_daa() {
	writeDAAReg(5,readDAAReg(5)&0xfe); //set OH bit of register 5 to on-hook.
	return;
}

static void caller_ID_pass() {
	writeDAAReg(5,readDAAReg(5)|0x08); //set ONHM bit of register 5 to enable caller ID detection.
	return;
}

//using ring_detection_daa() by while() polling. 
unsigned char ring_detection_DAA(void) {
	
	#if _Si3019_used_
		if (readDAAReg(29) == 0) {
			printk("phone line not connect\n");
			return 2;//Line not connect.	
	}
	#endif 
	//ring_validation_DAA(INIT_R22,INIT_R23,INIT_R24);
	if (readDAAReg(5) == 0xff)
		daa_init_all(0);
	#if 1
	#if CALLER_ID
	if (readDAAReg(5)&0x04)
		return 1; //Ringing now.
	else
		return 0; //Not ring state.
	#else
	if ((readDAAReg(5)&0x04))// || (readDAAReg(5)&0x20) || (readDAAReg(5)&0x40))//ring signal postive //ring signal negative 
		return 1; //Ringing now.
	else
		return 0; //Not ring state.
	#endif
	#else
	unsigned char temp=1;
	while (temp) {
		if ((readDAAReg(5)&0x04)) {
			printk("Ringing now\n");
			temp = 0;
		}	
		else	{
			//printk("NO Ringing \n");
			temp = 1;
		}
	}
	return 1; //Ringing now.
	#endif
}

static void ring_validation_DAA(unsigned char reg22,unsigned char reg23,unsigned char reg24) {
	writeDAAReg(22,reg22); //setup RDLY and RMX of register 22.
	if (readDAAReg(22) != reg22)
		printk("The reg22 of DAA isn't correct.\n");
	writeDAAReg(23,reg23); //setup RDLY,RTO and RCC of register 23.
	if (readDAAReg(23) != reg23)
		printk("The reg23 of DAA isn't correct.\n");
	writeDAAReg(24,reg24); //setup RAS of register 24.
	if (readDAAReg(24) != reg24)
		printk("The reg24 of DAA isn't correct.\n");
	writeDAAReg(18,INIT_R18); //set ring detector full-wave rectifier enable.RFWE of register 18.
	if (readDAAReg(18) != INIT_R18)
		printk("The reg18 of DAA isn't correct.\n");
	writeDAAReg(24,(readDAAReg(24)|0x80));	//enable RNGV of register 24.
	printk("The validation of incoming ring is ok.\n");
	return;
}

void daa_init_all(int ch) {
	//Call init_spi() to reset DAA before call this.
	printk("-----------------DAA init. start-------------------\n"); 
	writeDAAReg(3,INIT_R3);//disable all interrupt.Default should be disable. 
	writeDAAReg(7,INIT_R7);//setup sample rate 8kHz.
	writeDAAReg(17,(readDAAReg(17)|0x20));//disable auto ADC_calibration.CALD
	#if !pulse_dialing
	writeDAAReg(25,(readDAAReg(25)|0x20));//disable resistor calibration.RCALD
	#endif
	writeDAAReg(25,(readDAAReg(25)&0xdf));//setup resistor_calibration. reg25.It may not be written.
	writeDAAReg(34,INIT_R34);//setup PCM transmit slot(for 8186 PCM channel 1 time slot 2)
	writeDAAReg(35,INIT_R35);//setup PCM transmit slot(for 8186 PCM channel 1 time slot 2)
	writeDAAReg(36,INIT_R36);//setup PCM receive slot(for 8186 PCM channel 1 time slot 2)
	writeDAAReg(37,INIT_R37);//setup PCM receive slot(for 8186 PCM channel 1 time slot 2)
	writeDAAReg(33,INIT_R33);//enable PCM,use u-law.
	
	writeDAAReg(42,INIT_R42);//disable GCI interface.
	
	country_specific_termination(0,0,0,0,0,3,0,0);//setup DC and AC Termination.Default Taiwan.
	
	ring_validation_DAA(INIT_R22,INIT_R23,INIT_R24);//setup ring detection threshold.
	
	//DAA_hybrid_coeff(0,255,247,255,4,253,2,0,0);//set hybrid coefficient from reg45 to reg52.
	
	writeDAAReg(6,0x00);//powerup line-side device.
	printk("DAA init. ok\n");
	#if CALLER_ID
	caller_ID_pass();
	#endif
	
	device_revision();//display DAA revision.
	return; 
		
}
#if 0
//for dial out or ring incoming
	
	for (i=0;i<10000;i++) {
	unsigned char temp;
	do {
		if (readDirectReg(68)&0x01)  {//detect on-hook or off-hook.
			temp = 1;
			//printk("temp=%d\n",temp);
		} else if (ring_detection_DAA()) {//detect ring on or off.
			temp = 2;
			//printk("temp=%d\n",temp);
		} else if (!ring_detection_DAA()) {
			temp = 0;
			writeDirectReg(64, 0x01);
		} else {
			temp = 0;
			//printk("temp=%d\n",temp);
		}	
	
	} while (temp == 0); 
		
	if (temp == 1) { //for dial out
		printk("dial out\n");
		going_off_hook();
		while ((readDirectReg(68)&0x01));  //put the hook down
		on_hook_daa();
		printk("end dialing\n");
	}
	
	if (temp == 2) { //for ring incoming
		writeDirectReg(64, 0x04);
		
		for (;;) {
									
			//if ((readDirectReg(64)&0x20) || (readDirectReg(64)&0x40) || (readDirectReg(64)&0x10)) {
				
			if ((readDirectReg(68)&0x01)) {  //detect on-hook or off-hook.
				printk("ring incoming\n");
				going_off_hook();
				while ((readDirectReg(68)&0x01));  //put the hook down
				on_hook_daa();
				printk("end ring incoming\n");
				break;	
				
			} else {
				break;
			}
			
		
		}
		printk("out of for-loop\n");
		
		#if 0
		while (!(readDirectReg(68)&0x01)); //take the hook on
		going_off_hook();
		while ((readDirectReg(68)&0x01));  //put the hook down
		on_hook_daa();
		printk("end ring incoming\n");
		#endif
		
		
	}
	}
	#if 0 //for dial out
	while (!(readDirectReg(68)&0x01)); //take the hook on
	going_off_hook();
	while ((readDirectReg(68)&0x01));  //put the hook down
	on_hook_daa();
	#endif
	
	#if 0 //for ring incoming
	for (i=0;i<20;i++) {
	ring_detection_DAA();
	writeDirectReg(64, 0x04);
	going_off_hook();
	while ((readDirectReg(68)&0x01));
	//WaitKey();
	on_hook_daa();
	printk("Ring incoming\n");
	printk("end\n");
	}
	#endif
#endif

#if 0
	//for dial out or ring incoming
	unsigned char temp;
	do {
		if ((readDirectReg(68)&0x01))  {//detect on-hook or off-hook.dial out
			printk("off-hook\n");
			printk("going_off_hook()=%d\n",going_off_hook());
			//cyg_thread_delay(200);
				
			while ((readDirectReg(68)&0x01));  //put the hook down
			on_hook_daa();
			printk("on-hook\n");
			
			//printk("temp=%d\n",temp);
		} else if (ring_detection_DAA() == 1) {//detect ring on or off.
			//printk("incoming\n");
			
			writeDirectReg(64, 0x04);
			if ((readDirectReg(68)&0x01)) {  //detect on-hook or off-hook.
				printk("ring incoming\n");
				going_off_hook();
				while ((readDirectReg(68)&0x01));  //put the hook down
				on_hook_daa();
				printk("end ring incoming\n");
			}
			
			//printk("temp=%d\n",temp);
		} else if (!ring_detection_DAA()) {
			//cyg_thread_delay(10);
			temp = 0;
			writeDirectReg(64, 0x01);
		} else {
			temp = 0;
			//printk("temp=%d\n",temp);
		}	
	
	} while (temp == 0); 
		
	#if 0 //for dial out
	while (!(readDirectReg(68)&0x01)); //take the hook on
	printk("off-hook\n");
	going_off_hook();
	//for pulse dialing
	for (i=0;i<11;i++)
		pulse_dialing_tone(pulse_dialing_number());
	
	while ((readDirectReg(68)&0x01));  //put the hook down
	on_hook_daa();
	printk("on-hook\n");
	#endif
	
	#if 0 //for ring incoming
	for (i=0;i<20;i++) {
	ring_detection_DAA();
	writeDirectReg(64, 0x04);
	going_off_hook();
	while ((readDirectReg(68)&0x01));
	//WaitKey();
	on_hook_daa();
	printk("Ring incoming\n");
	printk("end\n");
	}
	#endif
#endif

#if 0
//dial_number count
unsigned char pulse_dialing_number(void) {
	unsigned char count,dial_number=0;
	unsigned int break_for_number=0,hold_time_long=0,hold_time_long_one=0;	
	#if 1
	while ((readDirectReg(68)&0x01));//when bit0 of reg68 equals to 0,break while loop.
	#else
	for (;;) {
		if (!(readDirectReg(68)&0x01)) {//when bit0 of reg68 equals to 0,break for loop.
				break;
			} else	
				hold_time_long++;
		if (hold_time_long >= 1500)
			return 11;
	}
	#endif	
	for (count=0;count<10;count++) {
		
		for (;;) {
			if (!(readDirectReg(68)&0x01)) {//when bit0 of reg68 equals to 0,break for loop.
				break;
			} else	
				break_for_number++;
			//printk("break_for_number=%d\n",break_for_number);
			if (break_for_number >= 220)
				break;	
		}
		//printk("break_for_number=%d\n",break_for_number);
		if (break_for_number >= 220)
			break;
		//printk("first\n");
		#if 0
		while (!(readDirectReg(68)&0x01));//when bit0 of reg68 equals to 1,break while loop.	
		#else
		for (;;) {
			if ((readDirectReg(68)&0x01)) {//when bit0 of reg68 equals to 1,break for loop.
				break;
			} else	
				hold_time_long_one++;
			//printk("break_for_number=%d\n",break_for_number);
			if (hold_time_long_one >= 550)
				return 11;	
		}
		#endif
		dial_number++;
		
		//printk("second\n");
		//break_for_number=0;	
	}	
		 
	return dial_number;
}
//1 means 1 pulse,2 means 2 pulse.....,0 means 10 pulse.
void pulse_dialing_tone(unsigned char dial_number) {
	unsigned char i;
	unsigned int j;
	if ((dial_number > 0) && (dial_number < 11)) {//when dial_number is btw. 1 and 10, it can be worked.
		for (i=0;i<dial_number;i++) {
			
			writeDAAReg(5,readDAAReg(5)&0xfe); //set OH bit of register 5 to go on-hook.
			for (j=0;j<5000000;j++); //presist 50~60ms
			writeDAAReg(5,readDAAReg(5)|0x01); //set OH bit of register 5 to go off-hook.
			for (j=0;j<5000000;j++); //presist 50~40ms
			
			
		}
		
		printk("dial_number=%d\n",dial_number);
	}
	return;	
}	
#endif

