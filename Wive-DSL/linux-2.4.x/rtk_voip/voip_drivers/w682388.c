////////////////////////////////////////////////////////////////////////
//
// slic.c
//
// Driver interface routines for the w682388 Programmable CODEC
//
// Copyright 2005, Winbond Electronics Corp
//
// NOTEs: 
//
/////////////////////////////////////////////////////////////////////////
#include <linux/config.h>
#include <linux/sched.h>	/* jiffies is defined */
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/string.h> 
#include "w682388.h"

/////////////////////////////////////////////////////////////////////////
// local definitions
/////////////////////////////////////////////////////////////////////////
typedef enum {OPEN =0, ON_HOOK, ACTIVE_MODE, ONHOOKTX, RINGING} SlicLineStates;
typedef enum { OSC1_CH1,OSC2_CH1, OSC1_CH2, OSC2_CH2 } Oscillators; 
typedef enum { DTMF_697 = 0, DTMF_770, DTMF_852, DTMF_941,  DTMF_1209, DTMF_1336, DTMF_1477, DTMF_1633}DTMFtones;

// Initialization data for the Winbond W682388 CODEC
BYTE InitData[247] = {

/*0*/	3,		0,		15,		15,		0,		0,		23,		0,		254,	1,		0,		0,		0,		0,		0,		0,
/*10*/	0,		0,		3,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,
/*20*/	0,		0,		17,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,
/*30*/	0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,
/*40*/	0,		0,		0,		0,		0,		7,		0,		0,		0,		0,		40/*127*/,	8/*127*/,	162/*125*/,	20,		0,		10,
/*50*/	10,		4,		4,		114,	114,	16,		16,		242,	242,	0,		9/*15*/,		2/*7*/,		26,		0,		4,		4,
/*60*/	245,	19,		8,		50,		100,	100,	0,		83,		83,		0,		0,		0,		0,		0,		0,		0,
/*70*/	0,		0,		0,		0,		0,		0,		0,		213,	0,		0,		0,		213,	0,		50,		50,		0,
/*80*/	3,		3,		3,		3,		3,		13,		13,		3,		3,		3,		3,		107,	4,		3,		3,		3,
/*90*/	3,		3,		13,		13,		3,		3,		3,		3,		0,		66,		2,		2,		0,		66,		2,		2,
/*A0*/	192,	192,	192,	255,	127,	97,		137,	97,		0,		0,		0,		0,		0,		0,		3,		9,
/*B0*/	9,		0,		0,		0x89/*119*/,	119,	119,	0x9B/*166*/,	0xAB/*201*/,	0,		0,		0,		0,		0,		0,		0,		0,
/*C0*/	0,		0,		0,		0,		221,	13,		0,		0,		221,	13,		0,		0,		108,	126,	0,		0,
/*D0*/	108,	126,	0,		0,		64,		6,		0,		0,		64,		6,		0,		0,		128,	12,		0,		0,
/*E0*/	128,	12,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,
/*F0*/	0,		68,		0,		0,		68,		0,		0 };


Tone DTMFtone[8] = 
{
	{0x6D4B, 0x0D09,0x1F40,0x0000},		// 697
	{0x694C, 0x0E3D,0x1F40,0x0000},		// 770	
	{0x6465, 0x0F89,0x1F40,0x0000},		// 852
	{0x5E9A, 0x10DE,0x1F40,0x0000},		// 941  
	{0x4A80, 0x2437,0x1F40,0x0000},		// 1209
	{0x3FC4, 0x269D,0x1F40,0x0000},		// 1336
	{0x331C, 0x28D5,0x1F40,0x0000},		// 1477
	{0x2462, 0x2aB3,0x1F40,0x0000}		// 1633
};

/////////////////////////////////////////////////////////////////////////
// local globals
/////////////////////////////////////////////////////////////////////////

SlicLineState LineState;

static void delay(int counts);		//Generic delay Replace with System specific routine
static void PLL_status(void);

static void PLL_status()
{
	unsigned char status;
	status = ReadReg(W682388REG_PLLS);
	status = (status & 0x1F);
	if (status == 0x0F)
		printk("PCLK is 2.048MHz locked.\n");
	else
		printk("PCLK is not correctly.\n"); 	
	return;
}

void W682388_OnHookLineReversal(BYTE chid, BYTE bReversal)
{
	if (chid == 0) {
		if (bReversal == 0)	//forward O_H_T
			WriteReg(W682388REG_LSC1,0x02);
		else	//Reverse O_H_T	
			WriteReg(W682388REG_LSC1,0x06);
	} else if (chid == 1) {
		if (bReversal == 0)	//forward O_H_T
			WriteReg(W682388REG_LSC2,0x02);
		else	//Reverse O_H_T	
			WriteReg(W682388REG_LSC2,0x06);
	}
	
	return;
}

/////////////////////////////////////////////////////////////////////////
// SlicInit 
// Initializes the w682388 hardware
/////////////////////////////////////////////////////////////////////////
WORD SlicInit()
{
	 
	  //BurstWrite(W682388REG_PCMC, 247, &InitData[0]);			//Load Cal Setup Data beginning at Address 0x00
	int i;
	for (i=0;i<247;i++)
		WriteReg(i,InitData[i]);
	  
	  PLL_status();	//check PCLK.
	  
	  // switch on Both DC-DC converters
	  WriteReg(W682388REG_PON, W682388REG_PON_FULL_ON);	
	  
	  //set 600Ohm impedance to ch0 and ch1
	  SetImpedance(0,W682388IMPEDANCE_600OHM);
	  SetImpedance(1,W682388IMPEDANCE_600OHM);

      // Setup Line Settings to Initial Values (OHV, CMV, VOV)
	  //Ch1
      WriteReg(W682388REG_OHVC1, W682388LINE_48V);//0x19);//
      WriteReg(W682388REG_CMVC1, W682388LINE_4_5V);//2);//
      WriteReg(W682388REG_VOVC1, W682388LINE_9V);
	  //Ch2
      WriteReg(W682388REG_OHVC2, W682388LINE_48V);
      WriteReg(W682388REG_CMVC2, W682388LINE_4_5V);//0);//
      WriteReg(W682388REG_VOVC2, W682388LINE_9V);

      // setup DC-DC converter (PWM, switching delay)
      WriteReg(W682388REG_PWMT, W682388DCDC_PWMT);
      WriteReg(W682388REG_DDCC, W682388DCDC_DDCC);

	  W682388Calibrate();
	  // set PCM TX/RX slots. Current setup CH1 = Slot 0, CH2 = slot 1
      // Need to set this up per the application requirements

	  //CH1 PCM Settings
      WriteReg(W682388REG_TCH, 0);
      WriteReg(W682388REG_TTSC1L, 1);
      WriteReg(W682388REG_RTSC1L, 1); 

	  //CH2PCM Settings
      WriteReg(W682388REG_TCH, 0);
      WriteReg(W682388REG_TTSC2L, 9);
      WriteReg(W682388REG_RTSC2L, 9);

      // enable PCM, u-law, 8bit, not GCI, tristate on positive edge 
	  WriteReg(W682388REG_PCMC , 0x43);
	  
	  //setup Ring cadence on:2 Sec,cadence off:2 Sec,amplitude: 78 Volt,frequency: 20 Hz
	  SetupRing(W682388_RING_FREQUENCY,W682388_RING_AMPLITUDE,2,2,0);
	  SetupRing(W682388_RING_FREQUENCY,W682388_RING_AMPLITUDE,2,2,1);
	  
      // Turn On Tracking (if desired)

	  SetVoltageTracking(0, TRUE);
 	  SetVoltageTracking(1, TRUE);
      
      // enable and clear all slic interrupt sources

      WriteReg(W682388REG_IE1C1, 0);//0xff);  //Enables
      WriteReg(W682388REG_IE2C1, 0);//0xff);
      WriteReg(W682388REG_IE1C2, 0);//0xff);   
      WriteReg(W682388REG_IE2C2, 0);//0xff);
      WriteReg(W682388REG_IE3,   0);//0x07);

      WriteReg(W682388REG_INT1C1, 0xff); // Clear Status	
      WriteReg(W682388REG_INT2C1, 0xff);
      WriteReg(W682388REG_INT1C2, 0xff);	 	
      WriteReg(W682388REG_INT2C2, 0xff);
      WriteReg(W682388REG_INT3,   0x07);


      // power up line
      SetLineState(1, ON_HOOK);
      SetLineState(0, ON_HOOK);
      
      printk("W682388 init. ok!\n");

  return NOERROR;
}

/////////////////////////////////////////////////////////////////////////
// GetHookState
// Get current hook status
/////////////////////////////////////////////////////////////////////////
BYTE GetHookState(BYTE Channel)
{
  BYTE State;

  if(Channel)
  {
		State = ReadReg(W682388REG_RTLCC2) & 0x01;
		LineState.OnHookCH2 = State;
  }

  else
  {
		State = ReadReg(W682388REG_RTLCC1) & 0x01; 		
		LineState.OnHookCH1 = State;
  }

  return State;
}

/////////////////////////////////////////////////////////////////////////
// SetLineState
// Controls the line state of the channel		
/////////////////////////////////////////////////////////////////////////
WORD SetLineState(BYTE Channel, BYTE State)
{
  BYTE LineFeed;
  BYTE Forward = 1;	//default set to Forward

  switch(State) {
	  case ACTIVE_MODE:
	    {

	      if(Channel)
		     Forward=LineState.BattPolarityCH2;
		  else
		     Forward=LineState.BattPolarityCH1;
      
	      // normal operation mode
	      LineFeed=Forward?W682388LINEFEED_FORWARDACTIVE:              
	        W682388LINEFEED_REVERSEACTIVE;                                          
	    }
	    break;
    
	  case ON_HOOK:
	    {
	      if(Channel)
		     Forward=LineState.BattPolarityCH2;
		  else
		     Forward=LineState.BattPolarityCH1;
		     
		 LineFeed=W682388LINEFEED_FORWARDIDLE;              
	             
	    }
	    break;
    
	  case ONHOOKTX:
	    {
	      if(Channel)
		     Forward=LineState.BattPolarityCH2;
		  else
		     Forward=LineState.BattPolarityCH1;
     
	      // on hook transmission mode
	      LineFeed=Forward?W682388LINEFEED_FORWARDONHOOKTX:           
	        W682388LINEFEED_REVERSEONHOOKTX;                                     
	    }
	    break;
    
	  case RINGING:
	    {
	      // ringing
	      LineFeed=W682388LINEFEED_RINGING;                                             
	    }
	    break;
     
      case OPEN:
      	{
      	  //open
      	  LineFeed=W682388LINEFEED_OPEN;
      	}
     }    
     
	  if(Channel)
		WriteReg(W682388REG_LSC2,LineFeed); 
	 else 
		WriteReg(W682388REG_LSC1,LineFeed); 
  return 0;
}

/////////////////////////////////////////////////////////////////////////
// SetVoltageTracking
// Selects tracking mode for DC-DC converter	 
/////////////////////////////////////////////////////////////////////////
WORD SetVoltageTracking(BYTE Channel, BYTE Track)
{
  BYTE data;

  if(Channel)
  {
	data = (ReadReg(W682388REG_VOVC2)) &0x0f;  
	WriteReg(W682388REG_VOVC2, (BYTE)(data |(Track?W682388VOV_TRACK_ON:W682388VOV_TRACK_OFF)));
  }
  else
  {
	data =(ReadReg(W682388REG_VOVC1)) &0x0f;  
	WriteReg(W682388REG_VOVC1, (BYTE)(data |(Track?W682388VOV_TRACK_ON:W682388VOV_TRACK_OFF)));
  }


  return 0;
}

/////////////////////////////////////////////////////////////////////////
// SetImpedance
// Selects Impedance for both channels	
//		Both channels are set the same 
/////////////////////////////////////////////////////////////////////////
WORD SetImpedance(BYTE ch, BYTE Country)
{
	if (ch == 0) {
		WriteReg(W682388REG_IM1C1,Country); 
		WriteReg(W682388REG_IM2C1,(BYTE)(Country & W682388REG_LCC_Disable)); 
	} else if (ch == 1) {
		WriteReg(W682388REG_IM1C2,Country); 
		WriteReg(W682388REG_IM2C2,(BYTE)(Country & W682388REG_LCC_Disable)); 
	}

  return 0;
}

/////////////////////////////////////////////////////////////////////////
// BatteryPolarity
// Sets battery polarity for line
/////////////////////////////////////////////////////////////////////////
WORD BatteryPolarity(BYTE Channel, BYTE Polarity)
{
  BYTE data;

  if(Channel)
  {

	  	data = (ReadReg(W682388REG_OHVC2)) &0x3F;
	  	
	  	// check if polarity has changed on CH2
		WriteReg(W682388REG_OHVC2, (BYTE)(data |(Polarity?W682388OHV_SB_ON:W682388OHV_SB_OFF))); 	  	
		// save new polarity
		LineState.BattPolarityCH2=Polarity;

		// update line state
		SetLineState(Channel, ACTIVE_MODE);

  }
  else
  {
	  	data = (ReadReg(W682388REG_OHVC1)) &0x3F;
	  	
	  	// check if polarity has changed on CH1
		WriteReg(W682388REG_OHVC1, (BYTE)(data |(Polarity?W682388OHV_SB_ON:W682388OHV_SB_OFF))); 	  	
		// save new polarity
		LineState.BattPolarityCH1=Polarity;

		// update line state
		SetLineState(Channel, ACTIVE_MODE);
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////
//  SetupRing
//  Sets up ring frequency, Amplitude and Cadence
////////////////////////////////////////////////////////////////////////
WORD SetupRing (WORD Frequency, WORD Amplitude, WORD OnTime, WORD OffTime, BYTE Channel)
{
	BYTE OscCntl; 

		if(Channel == 0)
		{
		    OscCntl = ReadReg(W682388REG_OSNC);
		    //WriteReg(W682388REG_OSNC,OscCntl|0x02);						//Setup Oscillator Control

			WriteReg(W682388REG_OS2C1ICL,(BYTE)((Amplitude & 0xff)));		//Set Amplitude
			WriteReg(W682388REG_OS2C1ICH,(BYTE)((Amplitude>>8)& 0xff));

			WriteReg(W682388REG_OS2C1CL,(BYTE)((Frequency & 0xff)));		//Set Frequency
			WriteReg(W682388REG_OS2C1CH,(BYTE)((Frequency>>8)& 0xff));
			
			OnTime *= 800;
			WriteReg(W682388REG_OS2C1ATL,(BYTE)((OnTime & 0xff)));			//Set Active timer (On Time)
			WriteReg(W682388REG_OS2C1ATH,(BYTE)((OnTime>>8)& 0xff));
			
			OffTime *= 800;
			WriteReg(W682388REG_OS2C1ITL,(BYTE)((OffTime & 0xff)));		//Set Inactive timer  (Off Time)
			WriteReg(W682388REG_OS2C1ITH,(BYTE)((OffTime>>8)& 0xff));													

		}

		if(Channel == 1)
		{
			OscCntl = ReadReg(W682388REG_OSNC);
			//WriteReg(W682388REG_OSNC,OscCntl|0x08);							//Setup Oscillator Control

			WriteReg(W682388REG_OS2C2ICL,(BYTE)((Amplitude & 0xff)));		//Set Amplitude
			WriteReg(W682388REG_OS2C2ICH,(BYTE)((Amplitude>>8)& 0xff));

			WriteReg(W682388REG_OS2C2CL,(BYTE)((Frequency & 0xff)));		//Set Frequency
			WriteReg(W682388REG_OS2C2CH,(BYTE)((Frequency>>8)& 0xff));
			
			OnTime *= 800;
			WriteReg(W682388REG_OS2C2ATL,(BYTE)((OnTime & 0xff)));			//Set Active timer (On Time)
			WriteReg(W682388REG_OS2C2ATH,(BYTE)((OnTime>>8)& 0xff));
			
			OffTime *= 800;
			WriteReg(W682388REG_OS2C2ITL,(BYTE)((OffTime & 0xff)));		//Set Inactive timer  (Off Time)
			WriteReg(W682388REG_OS2C2ITH,(BYTE)((OffTime>>8)& 0xff));																								
			
		}


	return 0;
}


////////////////////////////////////////////////////////////////////////
//  GenerateTones
//  Sets up Tone frequency, Amplitude and Cadence
////////////////////////////////////////////////////////////////////////
WORD GenerateTones (WORD Frequency, WORD Amplitude, WORD OnTime, WORD OffTime, BYTE Oscillator)
{
	BYTE OscCntl; 

		OscCntl = ReadReg(W682388REG_OSNC);						//Get Oscillator Control
		

		if(Oscillator == OSC1_CH1)
		{
			OscCntl |= 0x01;							//Set Oscillator Enable

			WriteReg(W682388REG_OS1C1ICL,(BYTE)((Amplitude & 0xff)));		//Set Amplitude
			WriteReg(W682388REG_OS1C1ICH,(BYTE)((Amplitude>>8)& 0xff));
			
			WriteReg(W682388REG_OS1C1CL,(BYTE)((Frequency & 0xff)));		//Set Frequency
			WriteReg(W682388REG_OS1C1CH,(BYTE)((Frequency>>8)& 0xff));
			
			WriteReg(W682388REG_OS1C1ATL,(BYTE)((OnTime & 0xff)));			//Set Active timer (On Time)
			WriteReg(W682388REG_OS1C1ATH,(BYTE)((OnTime>>8)& 0xff));

			WriteReg(W682388REG_OS1C1ITL,(BYTE)((OffTime & 0xff)));		//Set Inactive timer  (Off Time)
			WriteReg(W682388REG_OS1C1ITH,(BYTE)((OffTime>>8)& 0xff));											
		}

		
		if(Oscillator == OSC2_CH1)
		{
		    OscCntl |= 0x02;							//Setup Oscillator Enable

			WriteReg(W682388REG_OS2C1ICL,(BYTE)((Amplitude & 0xff)));		//Set Amplitude
			WriteReg(W682388REG_OS2C1ICH,(BYTE)((Amplitude>>8)& 0xff));

			WriteReg(W682388REG_OS2C1CL,(BYTE)((Frequency & 0xff)));		//Set Frequency
			WriteReg(W682388REG_OS2C1CH,(BYTE)((Frequency>>8)& 0xff));
			
			WriteReg(W682388REG_OS2C1ATL,(BYTE)((OnTime & 0xff)));		    //Set Active timer (On Time)
			WriteReg(W682388REG_OS1C1ATH,(BYTE)((OnTime>>8)& 0xff));

			WriteReg(W682388REG_OS2C1ITL,(BYTE)((OffTime & 0xff)));		//Set Inactive timer  (Off Time)
			WriteReg(W682388REG_OS2C1ITH,(BYTE)((OffTime>>8)& 0xff));													

		}

	
		if(Oscillator == OSC1_CH2)
		{
		    OscCntl |= 0x04;							//Setup Oscillator Enable
			
			WriteReg(W682388REG_OS1C2ICL,(BYTE)((Amplitude & 0xff)));		//Set Amplitude
			WriteReg(W682388REG_OS1C2ICH,(BYTE)((Amplitude>>8)& 0xff));

			WriteReg(W682388REG_OS1C2CL,(BYTE)((Frequency & 0xff)));		//Set Frequency
			WriteReg(W682388REG_OS1C2CH,(BYTE)((Frequency>>8)& 0xff));
			
			WriteReg(W682388REG_OS1C2ATL,(BYTE)((OnTime & 0xff)));			//Set Active timer (On Time)
			WriteReg(W682388REG_OS1C2ATH,(BYTE)((OnTime>>8)& 0xff));

			WriteReg(W682388REG_OS1C2ITL,(BYTE)((OffTime & 0xff)));		//Set Inactive timer  (Off Time)
			WriteReg(W682388REG_OS1C2ITH,(BYTE)((OffTime>>8)& 0xff));													
											
		}

	
		if(Oscillator == OSC2_CH2)
		{
		    OscCntl |= 0x08;							//Setup Oscillator Enable

			WriteReg(W682388REG_OS2C2ICL,(BYTE)((Amplitude & 0xff)));		//Set Amplitude
			WriteReg(W682388REG_OS2C2ICH,(BYTE)((Amplitude>>8)& 0xff));

			WriteReg(W682388REG_OS2C2CL,(BYTE)((Frequency & 0xff)));		//Set Frequency
			WriteReg(W682388REG_OS2C2CH,(BYTE)((Frequency>>8)& 0xff));
			
			WriteReg(W682388REG_OS2C2ATL,(BYTE)((OnTime & 0xff)));			//Set Active timer (On Time)
			WriteReg(W682388REG_OS1C2ATH,(BYTE)((OnTime>>8)& 0xff));

			WriteReg(W682388REG_OS2C2ITL,(BYTE)((OffTime & 0xff)));		//Set Inactive timer  (Off Time)
			WriteReg(W682388REG_OS2C2ITH,(BYTE)((OffTime>>8)& 0xff));													
												
			}

		WriteReg(W682388REG_OSNC,OscCntl);							//Update Oscillator Control

	return 0;
}


/////////////////////////////////////////////////////////////////////////
// GenerateMeterPulse
// This routine generates a metering pulse for a line	
// 
//		Inputs:  Frequency, Amplitude, cadence and channel
//
/////////////////////////////////////////////////////////////////////////
typedef enum  {Ch1, Ch2, BOTH} Channels;
WORD GenerateMeterPulse(WORD Frequency, WORD Amplitude, WORD OnTime, WORD OffTime, BYTE Channel)
{

BYTE Rmpc;
	
	Rmpc = ReadReg(W682388REG_RMPC);

	if(Channel == Ch1)
			Rmpc |= 0x05;
	else if (Channel == Ch2)
			Rmpc |= 0x09;
	else if (Channel == BOTH)
			Rmpc |= 0x0D;

	WriteReg(W682388REG_MPATL,(BYTE)(OnTime & 0xff));				//Set Active Timer
	WriteReg(W682388REG_MPATH,(BYTE)((OnTime >>8)& 0xff));
	
	WriteReg(W682388REG_MPITL,(BYTE)(OffTime & 0xff));				//Set InActive Timer
	WriteReg(W682388REG_MPITH,(BYTE)((OffTime >>8)& 0xff));
	
	WriteReg(W682388REG_MPCL,(BYTE)(Frequency & 0xff));				//Set Frequency
	WriteReg(W682388REG_MPCH,(BYTE)((Frequency >>8)& 0xff));
								
	WriteReg(W682388REG_MPICL,(BYTE)(Amplitude & 0xff));			//Set Amplitude
	WriteReg(W682388REG_MPICH,(BYTE)((Amplitude >>8)& 0xff));	
											
	WriteReg(W682388REG_MPADS,0x08);								//Metering Attack Rate
	
	WriteReg(W682388REG_MPMX,0x40);
	
	WriteReg(W682388REG_RMPC,Rmpc);									//Enabale Pulse Metering
	
	return 0;  

}
/////////////////////////////////////////////////////////////////////////
// FSKSendData
// This routine manages FSK tranmission using the FIFO.
// FSK transmit data is send by writing to the (FSKTD) register LSB first.  
// Control of the process is done by moitoring the FSK Status (FSKS) register.
//
//		Inputs:  Buffer and Length 
//
/////////////////////////////////////////////////////////////////////////
WORD FSKSendData(WORD NumBytes, BYTE Buffer[])
{
BYTE status, count;
 
	//Load the FSK_LCR reg
	WriteReg(W682388REG_FSKLCR,Buffer[0]);

	//Load the FSK_FCR reg
	WriteReg(W682388REG_FSKC,Buffer[1]);

	count=2;
	while(NumBytes-2)
	{
		//Get FSK Status
		status = ReadReg(W682388REG_FSKS);

		while (status & 0x04){					//If FIFO Full, wait until empty
			status = ReadReg(W682388REG_FSKS);

		}
		//Send Data
		WriteReg(W682388REG_FSKTD,Buffer[count]);

		count++;
		
		NumBytes--;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////
// SendCallerID
// This routine Sends a Type I, Caller ID Mesaage. It should be called 	
//		during the OnHook Transmission state, between rings.
//		(Note: that the delay is only a placeholder. You must implement
//			  an accurate timer to do thid properly.)
//
//		Inputs:  Frequency, Amplitude, Cadence and Channel
//
/////////////////////////////////////////////////////////////////////////
WORD SendCallerID (BYTE Channel)
{
BYTE FSKControlRegister = 0, FSKLevelRegister = 0, rmpc;

BYTE FSKString[20]= {
	0x0f,0x89,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55};
	
BYTE CallerIDData[68] ={   
	0x0f,0x89,0x80, 0x2B ,0x01,0x08,0x30,0x32,0x30,0x31,0x30,0x39,0x35,0x31,0x02,0x0A,0x34,0x30,
	0x38,0x35,0x35,0x35,0x31,0x32,0x31,0x32,0x07,0x13,0x57,0x69,0x6E,0x62,0x6F,0x6E,0x64,0x20,
	0x45,0x6C,0x65,0x63,0x74,0x72,0x6F,0x6E,0x69,0x63,0x73,0x27 };

		
			delay(500);							//Delay 500 milliseconds
			//Replace the delay above 
			// with a timer native to your system.

		rmpc = ReadReg(W682388REG_RMPC);  		
		
		if(Channel)
			rmpc |= 0x10;
		else 
			rmpc &= ~0x10; 	
			
		WriteReg(W682388REG_RMPC,rmpc);  
		
		FSKLevelRegister = 0x0f;			//Set FSK Level

		FSKControlRegister |= 0x01;			//FSK enable in case it's not on
		FSKControlRegister |= 0x08;			//FSK TX On in case it's not on
		FSKControlRegister |= 0x80;			//Make sure format is packaged	 
   
		FSKString[0] = FSKLevelRegister;
		FSKString[1] = FSKControlRegister;	
		
		CallerIDData[0] = FSKLevelRegister;
		CallerIDData[1] = FSKControlRegister;
		
		//enable fsk

			delay(10);						//Delay 10 milliseconds
			//Replace the delay above 
			// with a timer native to your system.
		
		FSKSendData(20,FSKString);

			delay(280);						//Delay 280 milliseconds
			//Replace the delay above 
			// with a timer native to your system.
		
		FSKSendData(48,CallerIDData);
		
			delay(500);						//Delay 500 milliseconds
			//Replace the delay above 
			// with a timer native to your system.

		//disable fsk
		WriteReg(W682388REG_FSKC,0x00);	 

	return 0;
}

/////////////////////////////////////////////////////////////////////////
// DumpRegisters
// Prints out all registers
/////////////////////////////////////////////////////////////////////////
void DumpRegisters(void)
{
  BYTE Addr, Data;

  printk("Register dump for w682388 \n");
  printk("All Registers\n ");
  for (Addr=0;Addr<=246;Addr++) {                                              
    Data = ReadReg(Addr);
    printk("Address (%x) = %x\n ", Addr,Data);
  }
  printk("*** End Dump ***");
  printk("\n\n");
  
}

//////////////////////////////////////////////////////////////////////////
//FXS ring
void WINBOND_FXS_ring(ring_struct *ring)
{
	if (ring->ring_set == 1)  //ring on 
		SetLineState(ring->CH,RINGING);
	else if (ring->ring_set == 0)  //ring off
		SetLineState(ring->CH,ON_HOOK);
		
	return;
}

//////////////////////////////////////////////////////////////////////////
//check FXS ring state
int Check_WINBOND_FXS_ring(ring_struct *rings)
{
	int ringer; //0: ring off, 1: ring on
	
	if (rings->CH == 0) {
		if ((ReadReg(0x46)&0x40) == 0x40)
			ringer = 1;
		else
			ringer = 0;
	} else if (rings->CH == 1) {
		if ((ReadReg(0x47)&0x40) == 0x40)
			ringer = 1;
		else
			ringer = 0;	
	}
	
	return ringer;
}

//////////////////////////////////////////////////////////////////////////
// average 8 reads
//////////////////////////////////////////////////////////////////////////
BYTE ReadSPIByte8(BYTE Addr)
{		//average 8 reads
WORD IAcc=0;
	IAcc+= ReadReg(Addr);
	IAcc+= ReadReg(Addr);
	IAcc+= ReadReg(Addr);
	IAcc+= ReadReg(Addr);
	IAcc+= ReadReg(Addr);
	IAcc+= ReadReg(Addr);
	IAcc+= ReadReg(Addr);
	IAcc+= ReadReg(Addr);
	return IAcc/8;
}

///////////////////////////////////////////////////////////////////////////
// abs
//  Dummy Absolute value Calculation.  
//  Note: If available use the Absolute value function local to your system.
///////////////////////////////////////////////////////////////////////////
WORD abs(WORD oData)
{
  WORD lReturn ;
  lReturn = (oData<0)?-oData:oData;
  return  lReturn;   
}

/////////////////////////////////////////////////////////////////////////
// AdjustLineState
// Sets up Line for calibration
/////////////////////////////////////////////////////////////////////////
WORD AdjustLineState(BYTE LineState)
{
BYTE CurrentLineState;
WORD looper;

	WriteReg (W682388REG_LSC1,LineState);				//set line state open
	WriteReg (W682388REG_LSC2,LineState);				//set line state open
	for (looper=0;looper<10000;looper++);			//some delay

//now check
	CurrentLineState = ReadReg(W682388REG_LSC1);		//read current line state
//need to add error breakout
	if ((CurrentLineState>>4) != LineState){		//if not open try one more time
		//need set to open again
		WriteReg (W682388REG_LSC1,LineState);			//set line state open
		WriteReg (W682388REG_LSC2,LineState);			//set line state open
		for (looper=0;looper<10000;looper++);		//some delay
		CurrentLineState = ReadReg(W682388REG_LSC1);
		if ((CurrentLineState>>4) != LineState)
			return 0;
	}
	CurrentLineState = ReadReg(W682388REG_LSC2);		//read current line state
//need to add error breakout
	if ((CurrentLineState>>4) != LineState){		//if not open try one more time
		//need set to open again
		WriteReg(W682388REG_LSC2,LineState);			//set line state open
		for (looper=0;looper<10000;looper++);		//some delay
		CurrentLineState = ReadReg(W682388REG_LSC2);
		if ((CurrentLineState>>4) != LineState)
			return 0;
	}
	return 1;


}
void W682388Calibrate(void)
{
BYTE UCAcc1,UCAcc2,vbg,vbg1,temp,min,minval;
unsigned int looper,looper2;
	printk("W682388 start calibration........\n");
	
	WriteReg(W682388REG_CAL1,0x77);					//setup defaults
	WriteReg(W682388REG_CAL4,0x99);					//setup defaults
	WriteReg(W682388REG_CAL5,0x99);					//setup defaults
	WriteReg(W682388REG_TDCOFFS,0x0);					//setup defaults
	WriteReg(W682388REG_RDCOFFS,0x0);					//setup defaults
	WriteReg(W682388REG_ETCC1,0x13);					//setup defaults
	WriteReg(W682388REG_ETCC2,0x13);					//setup defaults
	WriteReg(W682388REG_IM2C1,0x30);					//setup defaults
	WriteReg(W682388REG_IM2C2,0x30);					//setup defaults
	WriteReg(W682388REG_IQT3OFFS,0x3f);				//setup defaults
	WriteReg(W682388REG_IQR3OFFS,0x3f);				//setup defaults

	if (!AdjustLineState(0x00))
		return;

	minval = 0xff;
	WriteReg(W682388REG_CC,0x08);						//enable bandgap cal
	for (looper=0;looper<8;looper++){
		WriteReg(W682388REG_CC,(BYTE)(0x08|looper));
		for (looper2=0;looper2<10000;looper2++);	//some delay
		temp = ReadReg(W682388REG_TEMP);				//read temp sense
		vbg = ReadReg(W682388REG_VBG);					//read VBG
		if (abs(temp-vbg)<minval){
			minval = abs(temp-vbg);
			min = looper;
		}
		if (temp == vbg)
			break;
	}
	if (min < 4)			//don't use 1,2,or 3
		min = 0;
	WriteReg(W682388REG_CC,(BYTE)(min & ~0x08));				//course trim is set


	
//now set to reverse on-hook trans for CAL1	
	UCAcc2 = ReadReg(W682388REG_OHVC2);				//save polarity
	if (UCAcc2 & 0x40)
		goto ReverseCal;

//now set to forward on-hook trans for CAL1	
	if (!AdjustLineState(0x02))
		return;
	
	for (looper=0;looper<20000000;looper++);			//wait ~200ms
	WriteReg(W682388REG_ETCC1,0x33);					//bypass some filters to speed it up
	WriteReg(W682388REG_ETCC2,0x33);
//now in forward on-hook transmision state

//first set CAL1 & CAL5 VBAT trim
	vbg = ReadReg(W682388REG_CAL1);
	vbg1 = vbg &0x0f;
	for (looper=0;looper<60000;looper++);
	UCAcc1  = ReadSPIByte8(W682388REG_QR3IC1);
	UCAcc2 = ReadSPIByte8(W682388REG_QR1IC1);
//check to see if Q3 already greater than Q1
	if (UCAcc1 > UCAcc2){
		WriteReg(W682388REG_CAL1,(BYTE)((vbg&0xf0)|0x04));		//set CAL1 ch1 to 4
		WriteReg(W682388REG_CAL5,(BYTE)((vbg&0xf0)|6));		//set CAL5 ch1 to 6
		UCAcc1  = ReadReg(W682388REG_CAL4);
		WriteReg(W682388REG_CAL4,(BYTE)((UCAcc1&0xf0)|6));		//set CAL4 ch1 to 6
		vbg1 = 0;
		for (looper=0;looper<10000;looper++);
		UCAcc1  = ReadSPIByte8(W682388REG_QR3IC1);
		UCAcc2 = ReadSPIByte8(W682388REG_QR1IC1);
	}

	while ((UCAcc2>=UCAcc1)&(vbg1 != 0x0d)){		//go until QR3 > QR1
		vbg1 += 1;
		WriteReg(W682388REG_CAL1,(BYTE)((vbg&0xf0)|vbg1));
		WriteReg(W682388REG_CAL5,(BYTE)(((vbg&0xf0)|vbg1)+2));
		for (looper=0;looper<60000;looper++);
		UCAcc1 = ReadSPIByte8(W682388REG_QR3IC1);
		UCAcc2 = ReadSPIByte8(W682388REG_QR1IC1);
		vbg = ReadReg(W682388REG_CAL1);
		vbg1 = vbg &0x0f;

	}
	vbg = ReadReg(W682388REG_CAL1);
	vbg1 = vbg>>4;
	UCAcc1 = ReadSPIByte8(W682388REG_QR3IC2);
	UCAcc2 = ReadSPIByte8(W682388REG_QR1IC2);
//check to see if Q3 already greater than Q1
	if (UCAcc1 > UCAcc2){
		WriteReg(W682388REG_CAL1,(BYTE)((vbg&0x0f)|0x40));		//set CAL1 ch2 to 0
		UCAcc1  = ReadReg(W682388REG_CAL5);
		WriteReg(W682388REG_CAL5,(BYTE)((UCAcc1&0x0f)|0x60));	//set CAL5 ch2 to 2
		UCAcc1  = ReadReg(W682388REG_CAL4);
		WriteReg(W682388REG_CAL4,(BYTE)((UCAcc1&0x0f)|0x60));	//set CAL4 ch2 to 2
		vbg1 = 0;
		for (looper=0;looper<10000;looper++);
		UCAcc1  = ReadSPIByte8(W682388REG_QR3IC2);
		UCAcc2 = ReadSPIByte8(W682388REG_QR1IC2);
	}
	while ((UCAcc2>=UCAcc1)&(vbg1 != 0x0d)){		//go until QR3 > QR1
		vbg1 += 1;
		WriteReg(W682388REG_CAL1,(BYTE)((vbg&0x0f)|vbg1<<4));
		UCAcc1  = ReadReg(W682388REG_CAL5);
		WriteReg(W682388REG_CAL5,(BYTE)((UCAcc1&0x0f)|(vbg1+2)<<4));
		for (looper=0;looper<60000;looper++);
		UCAcc1 = ReadSPIByte8(W682388REG_QR3IC2);
		UCAcc2 = ReadSPIByte8(W682388REG_QR1IC2);
		vbg = ReadReg(W682388REG_CAL1);
		vbg1 = vbg>>4;
	}
//now set CAL4
	vbg = ReadReg(W682388REG_CAL4);
	vbg1 = vbg &0x0f;
	UCAcc1 = ReadSPIByte8(W682388REG_QT3IC1);
	UCAcc2 = ReadSPIByte8(W682388REG_QT1IC1);
	while ((UCAcc2>=UCAcc1)&(vbg1 != 0x0f)){		//go until QR3 > QR1
		vbg1 += 1;
		WriteReg(W682388REG_CAL4,(BYTE)((vbg&0xf0)|vbg1));
		for (looper=0;looper<60000;looper++);
		UCAcc1 = ReadReg(W682388REG_QT3IC1);
		UCAcc2 = ReadReg(W682388REG_QT1IC1);
		vbg = ReadReg(W682388REG_CAL4);
		vbg1 = vbg &0x0f;
	}
	vbg = ReadReg(W682388REG_CAL4);
	vbg1 = vbg>>4;
	UCAcc1 = ReadSPIByte8(W682388REG_QT3IC2);
	UCAcc2 = ReadSPIByte8(W682388REG_QT1IC2);
	while ((UCAcc2>=UCAcc1)&(vbg1 != 0x0f)){		//go until QR3 > QR1
		vbg1 += 1;
		WriteReg(W682388REG_CAL4,(BYTE)((vbg&0x0f)|vbg1<<4));
		for (looper=0;looper<60000;looper++);
		UCAcc1 = ReadSPIByte8(W682388REG_QT3IC2);
		UCAcc2 = ReadSPIByte8(W682388REG_QT1IC2);
		vbg = ReadReg(W682388REG_CAL4);
		vbg1 = vbg>>4;
	}
	goto CalFinished;								//we be finished

ReverseCal:
	WriteReg(W682388REG_ETCC1,0x13);					//bypass some filters to speed it up
	WriteReg(W682388REG_ETCC2,0x13);
	
	
//now reverse on-hook transmition for CAL4
//now set to forward on-hook trans for CAL1	
	if (!AdjustLineState(0x06))
		return;
	
	for (looper=0;looper<20000000;looper++);			//wait ~200mS
	WriteReg(W682388REG_ETCC1,0x33);					//bypass some filters to speed it up
	WriteReg(W682388REG_ETCC2,0x33);

	vbg = ReadReg(W682388REG_CAL1);
	vbg1 = vbg &0x0f;
	UCAcc1 = ReadSPIByte8(W682388REG_QT3IC1);
	UCAcc2 = ReadSPIByte8(W682388REG_QT1IC1);
//check to see if Q3 already greater than Q1
	if (UCAcc1 > UCAcc2){
		WriteReg(W682388REG_CAL1,(BYTE)((vbg&0xf0)|0x04));		//set CAL1 ch1 to 0
		WriteReg(W682388REG_CAL4,(BYTE)((vbg&0xf0)|6));		//set CAL4 ch1 to 2
		UCAcc1  = ReadReg(W682388REG_CAL5);
		WriteReg(W682388REG_CAL5,(BYTE)((UCAcc1&0xf0)|6));		//set CAL5 ch1 to 2
		vbg1 = 0;
		for (looper=0;looper<10000;looper++);
		UCAcc1  = ReadSPIByte8(W682388REG_QT3IC1);
		UCAcc2 = ReadSPIByte8(W682388REG_QT1IC1);
	}

	while ((UCAcc2>=UCAcc1)&(vbg1 != 0x0d)){		//go until QR3 > QR1
		vbg1 += 1;
		WriteReg(W682388REG_CAL1,(BYTE)((vbg&0xf0)|vbg1));
		WriteReg(W682388REG_CAL4,(BYTE)(((vbg&0xf0)|vbg1)+2));
		for (looper=0;looper<60000;looper++);
		UCAcc1 = ReadReg(W682388REG_QT3IC1);
		UCAcc2 = ReadReg(W682388REG_QT1IC1);
		vbg = ReadReg(W682388REG_CAL1);
		vbg1 = vbg &0x0f;
	}
	vbg = ReadReg(W682388REG_CAL1);
	vbg1 = vbg>>4;
	UCAcc1 = ReadSPIByte8(W682388REG_QT3IC2);
	UCAcc2 = ReadSPIByte8(W682388REG_QT1IC2);
//check to see if Q3 already greater than Q1
	if (UCAcc1 > UCAcc2){
		WriteReg(W682388REG_CAL1,(BYTE)((vbg&0x0f)|0x40));		//set CAL1 ch2 to 0
		UCAcc1  = ReadReg(W682388REG_CAL4);
		WriteReg(W682388REG_CAL4,(BYTE)((UCAcc1&0x0f)|0x60));	//set CAL4 ch2 to 2
		UCAcc1  = ReadReg(W682388REG_CAL5);
		WriteReg(W682388REG_CAL5,(BYTE)((UCAcc1&0x0f)|0x60));	//set CAL5 ch2 to 2
		vbg1 = 0;
		for (looper=0;looper<10000;looper++);
		UCAcc1  = ReadSPIByte8(W682388REG_QT3IC2);
		UCAcc2 = ReadSPIByte8(W682388REG_QT1IC2);
	}
	while ((UCAcc2>=UCAcc1)&(vbg1 != 0x0d)){		//go until QR3 > QR1
		vbg1 += 1;
		WriteReg(W682388REG_CAL1,(BYTE)((vbg&0x0f)|vbg1<<4));
		UCAcc1  = ReadReg(W682388REG_CAL4);
		WriteReg(W682388REG_CAL4,(BYTE)((UCAcc1&0x0f)|(vbg1+2)<<4));
		for (looper=0;looper<60000;looper++);
		UCAcc1 = ReadSPIByte8(W682388REG_QT3IC2);
		UCAcc2 = ReadSPIByte8(W682388REG_QT1IC2);
		vbg = ReadReg(W682388REG_CAL1);
		vbg1 = vbg>>4;
	}

//now set CAL5
	vbg = ReadReg(W682388REG_CAL5);
	vbg1 = vbg &0x0f;
	UCAcc1 = ReadSPIByte8(W682388REG_QR3IC1);
	UCAcc2 = ReadSPIByte8(W682388REG_QR1IC1);
	while ((UCAcc2>=UCAcc1)&(vbg != 0x0f)){			//go until QR3 > QR1
		vbg1 += 1;
		WriteReg(W682388REG_CAL5,(BYTE)((vbg&0xf0)|vbg1));
		for (looper=0;looper<60000;looper++);
		UCAcc1 = ReadReg(W682388REG_QR3IC1);
		UCAcc2 = ReadReg(W682388REG_QR1IC1);
		vbg = ReadReg(W682388REG_CAL5);
		vbg1 = vbg &0x0f;
	}
	vbg = ReadReg(W682388REG_CAL5);
	vbg1 = vbg>>4;
	UCAcc1 = ReadSPIByte8(W682388REG_QR3IC2);
	UCAcc2 = ReadSPIByte8(W682388REG_QR1IC2);
	while ((UCAcc2>=UCAcc1)&(vbg1 != 0x0f)){		//go until QR3 > QR1
		vbg1 += 1;
		WriteReg(W682388REG_CAL5,(BYTE)((vbg&0x0f)|vbg1<<4));
		for (looper=0;looper<60000;looper++);
		UCAcc1 = ReadSPIByte8(W682388REG_QR3IC2);
		UCAcc2 = ReadSPIByte8(W682388REG_QR1IC2);
		vbg = ReadReg(W682388REG_CAL5);
		vbg1 = vbg>>4;
	}


CalFinished:
//set back to open
	if (!AdjustLineState(0x00))
		return;
	WriteReg(W682388REG_ETCC1,0x13);
	WriteReg(W682388REG_ETCC2,0x13);
	printk("Finish W682388 calibration\n");
}
/////////////////////////////////////////////////////////////////////////
// delay
// This is a fake delay for CID illustration purposes.	
// 
//		Replace this with a delay that is native to your system
//
/////////////////////////////////////////////////////////////////////////
static void delay(int count)
{

	while(count--)
	{
		;;;;  
	}

}

/////////////////////////////////////////////////////////////////////////
// This routine Generates DTMF using the internal Oscillators.
/////////////////////////////////////////////////////////////////////////
WORD GenerateDTMF ( Tone* DTMFLow, Tone* DTMFHigh, BYTE Channel)
{
	BYTE OscCntl; 

		OscCntl = ReadReg(W682388REG_OSNC);						//Get Oscillator Control

		if(Channel == 1)
		{

			OscCntl |= 0x03;							//Enable Channel Oscillators 1 & 2
			
			//Osc1 1 Channel 1

			WriteReg(W682388REG_OS1C1ICL,(BYTE)((DTMFLow->Amplitude & 0xff)));		//Set Amplitude
			WriteReg(W682388REG_OS1C1ICH,(BYTE)((DTMFLow->Amplitude>>8)& 0xff));
			
			WriteReg(W682388REG_OS1C1CL,(BYTE)((DTMFLow->Frequency & 0xff)));		//Set Frequency
			WriteReg(W682388REG_OS1C1CH,(BYTE)((DTMFLow->Frequency>>8)& 0xff));
			
			WriteReg(W682388REG_OS1C1ATL,(BYTE)((DTMFLow->OnTime & 0xff)));			//Set Active timer (On Time)
			WriteReg(W682388REG_OS1C1ATH,(BYTE)((DTMFLow->OnTime>>8)& 0xff));

			WriteReg(W682388REG_OS1C1ITL,(BYTE)((DTMFLow->OffTime & 0xff)));		//Set Inactive timer  (Off Time)
			WriteReg(W682388REG_OS1C1ITH,(BYTE)((DTMFLow->OffTime>>8)& 0xff));											
		
 			//Osc2 Channel 1  

			WriteReg(W682388REG_OS2C1ICL,(BYTE)((DTMFHigh->Amplitude & 0xff)));		//Set Amplitude
			WriteReg(W682388REG_OS2C1ICH,(BYTE)((DTMFHigh->Amplitude>>8)& 0xff));

			WriteReg(W682388REG_OS2C1CL,(BYTE)((DTMFHigh->Frequency & 0xff)));		//Set Frequency
			WriteReg(W682388REG_OS2C1CH,(BYTE)((DTMFHigh->Frequency>>8)& 0xff));
			
			WriteReg(W682388REG_OS2C1ATL,(BYTE)((DTMFHigh->OnTime & 0xff)));		    //Set Active timer (On Time)
			WriteReg(W682388REG_OS1C1ATH,(BYTE)((DTMFHigh->OnTime>>8)& 0xff));

			WriteReg(W682388REG_OS2C1ITL,(BYTE)((DTMFHigh->OffTime & 0xff)));		//Set Inactive timer  (Off Time)
			WriteReg(W682388REG_OS2C1ITH,(BYTE)((DTMFHigh->OffTime>>8)& 0xff));													

		}

	
		if(Channel == 2)
		{

			OscCntl |= 0x0C;							//Enable Channel2 Oscillators 1 & 2
 			
 			//Osc1 Channel 2  

			WriteReg(W682388REG_OS1C2ICL,(BYTE)((DTMFLow->Amplitude & 0xff)));		//Set Amplitude
			WriteReg(W682388REG_OS1C2ICH,(BYTE)((DTMFLow->Amplitude>>8)& 0xff));

			WriteReg(W682388REG_OS1C2CL,(BYTE)((DTMFLow->Frequency & 0xff)));		//Set Frequency
			WriteReg(W682388REG_OS1C2CH,(BYTE)((DTMFLow->Frequency>>8)& 0xff));
			
			WriteReg(W682388REG_OS1C2ATL,(BYTE)((DTMFLow->OnTime & 0xff)));			//Set Active timer (On Time)
			WriteReg(W682388REG_OS1C2ATH,(BYTE)((DTMFLow->OnTime>>8)& 0xff));

			WriteReg(W682388REG_OS1C2ITL,(BYTE)((DTMFLow->OffTime & 0xff)));		//Set Inactive timer  (Off Time)
			WriteReg(W682388REG_OS1C2ITH,(BYTE)((DTMFLow->OffTime>>8)& 0xff));													

 			//Osc2 Channel 2   

			WriteReg(W682388REG_OS2C2ICL,(BYTE)((DTMFHigh->Amplitude & 0xff)));		//Set Amplitude
			WriteReg(W682388REG_OS2C2ICH,(BYTE)((DTMFHigh->Amplitude>>8)& 0xff));

			WriteReg(W682388REG_OS2C2CL,(BYTE)((DTMFHigh->Frequency & 0xff)));		//Set Frequency
			WriteReg(W682388REG_OS2C2CH,(BYTE)((DTMFHigh->Frequency>>8)& 0xff));
			
			WriteReg(W682388REG_OS2C2ATL,(BYTE)((DTMFHigh->OnTime & 0xff)));			//Set Active timer (On Time)
			WriteReg(W682388REG_OS1C2ATH,(BYTE)((DTMFHigh->OnTime>>8)& 0xff));

			WriteReg(W682388REG_OS2C2ITL,(BYTE)((DTMFHigh->OffTime & 0xff)));		//Set Inactive timer  (Off Time)
			WriteReg(W682388REG_OS2C2ITH,(BYTE)((DTMFHigh->OffTime>>8)& 0xff));													
												
		}

		WriteReg(W682388REG_OSNC,OscCntl);							//Update Oscillator Control

	return 0;
}

///////////////////////////// END OF FILE ///////////////////////////////
