#ifndef _DAA_H_
#define _DAA_H_

#define DAA_NOT_USE	0
#define DAA_USE		1
#define DAA_USE_HOLD	2

/* DAA Register Initial Value */
#define	INIT_R1	0x00	//	Control 1
#define	INIT_R2	0x03	//	Control 2
#define	INIT_R3	0x00	//	Interrupt Mask
#define	INIT_R4	0x00	//	Interrupt Source
#define	INIT_R5	0x00	//	DAA Control 1
//#define	INIT_R6		//	DAA Control 2
#define	INIT_R7	0x00	//8kHz	Sample Rate Control
#define	INIT_R10 0x00	//	DAA Control 3 .Digitial data loopback 
//#define	INIT_R11	//	System-Side and Line-Side Device Revision. Read only 
//#define	INIT_R12	//	Line-Side Device Status. Read only 
//#define	INIT_R13	//	Line-Side Device Revision. Read only 
#define	INIT_R14 0x00		//	DAA Control 4. Ring detect polarity
#define	INIT_R15 0x00		//	TX/RX Gain Control 1.Transmit mute and receive mute.
//#define	INIT_R16		//	International Control 1
#define	INIT_R17 0x00		//	International Control 2
#define	INIT_R18 0x02		//	International Control 3. for ring detection used.
//#define	INIT_R19		//	International Control 4. Read only
#define	INIT_R20 0x00		//	Call Progress RX Attenuation. Mute
#define	INIT_R21 0x00		//	Call Progress TX Attenuation. Mute
#define	INIT_R22 0x96		//	Ring Validation Control 1
#define	INIT_R23 0x2d		//	Ring Validation Control 2
#define	INIT_R24 0x99		//	Ring Validation Control 3
//#define	INIT_R25		//	Resistor Calibration
//#define	INIT_R26		//	DC Termination Control,FCC=0xc0,TBR21=0xc2
//#define	INIT_R28		//	Loop Current Status. Read only
//#define	INIT_R29		//	Line Voltage Status. Read only
//#define	INIT_R30		//	AC Termination Control
//#define	INIT_R31		//	DAA Control 5
#define	INIT_R32	0x03	//	Ground Start Control //Don't need 
#define	INIT_R33	0x28	//	PCM/SPI Mode Select.Set reg34~37 before set reg33
#define	INIT_R34	0x11	//	PCM Transmit Start Count - Low Byte
#define	INIT_R35	0x00	//	PCM Transmit Start Count - High Byte
#define	INIT_R36	0x11	//	PCM Receive Start count - Low Byte
#define	INIT_R37	0x00	//	PCM Receive Start count - High Byte
//#define	INIT_R38		//	TX Gain Control 2
//#define	INIT_R39		//	RX Gain Control 2
//#define	INIT_R40		//	TX Gain Control 3
//#define	INIT_R41		//	RX Gain Control 3
#define	INIT_R42	0x00	//	GCI Control
#define	INIT_R43	0x00	//	Line Current/Voltage Threshold Interrupt
#define	INIT_R44	0x00	//	Line Current/Voltage Threshold Interrupt Control
#define	INIT_R45	0x00	//	Programmable Hybrid Register 1
#define	INIT_R46	0x00	//	Programmable Hybrid Register 2
#define	INIT_R47	0x00	//	Programmable Hybrid Register 3
#define	INIT_R48	0x00	//	Programmable Hybrid Register 4
#define	INIT_R49	0x00	//	Programmable Hybrid Register 5
#define	INIT_R50	0x00	//	Programmable Hybrid Register 6
#define	INIT_R51	0x00	//	Programmable Hybrid Register 7
#define	INIT_R52	0x00	//	Programmable Hybrid Register 8
#define	INIT_R59	0x00	//	Spark Quenching Control




void daa_init_all(int ch);
void country_specific_termination(unsigned char ohs,unsigned char ohs2,unsigned char rz,
		unsigned char rt,unsigned char ilim,unsigned char dcv,unsigned char mini,unsigned char acim);
unsigned char going_off_hook(void);
void on_hook_daa(void);
unsigned char ring_detection_DAA(void);
void DAA_hybrid_coeff(unsigned char acim,unsigned char coeff1,unsigned char coeff2,unsigned char coeff3,
		unsigned char coeff4,unsigned char coeff5,unsigned char coeff6,unsigned char coeff7,unsigned char coeff8);
void DAA_Tx_Gain_ctrl(unsigned char tga2,unsigned char txg2,unsigned char tga3,unsigned char txg3);
void DAA_Rx_Gain_ctrl(unsigned char rga2,unsigned char rxg2,unsigned char rga3,unsigned char rxg3);
void DAA_Tx_Gain_Web(unsigned char gain);
void DAA_Rx_Gain_Web(unsigned char gain);

#endif //_DAA_H_
