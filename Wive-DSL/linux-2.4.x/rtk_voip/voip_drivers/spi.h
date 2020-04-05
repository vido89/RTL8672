/*
* Copyright c                  Realtek Semiconductor Corporation, 2006  
* All rights reserved.
*/

#ifndef _SPI_H_
#define _SPI_H_

#include "gpio.h"

/* common structure */
//#define CHANNEL 1 //original for one channel slic
#define CHANNEL 2 //one for slic and another for daa. chiminer

struct rtl_spi_dev_s
{
	uint32 gpioSCLK;
	uint32 gpioCS_;
	uint32 gpioSDI;
	uint32 gpioSDO;
	uint32 SClkDelayLoop;
};

typedef struct rtl_spi_dev_s rtl8651_spi_dev_t;
typedef struct rtl_spi_dev_s rtl8186_spi_dev_t;
typedef struct rtl_spi_dev_s rtl867x_spi_dev_t;
extern unsigned char slic_order;
void cyg_thread_delay(int delay);
void waitForIndirectReg(void);
unsigned char readDirectReg(unsigned char address);
void writeDirectReg(unsigned char address, unsigned char data);
void writeIndirectReg(unsigned char address, unsigned short data);
void init_spi(int ch_spi);
unsigned short readIndirectReg(unsigned char address);
//--chiminer 06-7-27-----------//
#define speed_booting_rating 1
//-----------------------------//

//--chiminer 2006-5-11----------
//The global variable is used to access the data of registers of Le88221.
//After calling read function, this global variable is valid.
//After writting data into this global variable, call the write function. 
typedef struct Le88xxx_register {
	unsigned char byte1;
	unsigned char byte2;
	unsigned char byte3;
	unsigned char byte4;
	unsigned char byte5;
	unsigned char byte6;
	unsigned char byte7;
	unsigned char byte8;
	unsigned char byte9;
	unsigned char byte10;
	unsigned char byte11;
	unsigned char byte12;
	unsigned char byte13;
	unsigned char byte14;	
} Le88xxx;

/*********************  Function Prototype in spi.c  ***********************/
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221 
/****The following is used to Legerity slic*****/
void readLegerityReg(unsigned char address, Le88xxx *data);
void writeLegerityReg(unsigned char address, Le88xxx *data);
#endif

#if defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210 ) || defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215 )
/****The following is used to Silicon Lab. slic and DAA*****/
unsigned char readDirectReg_nodaisy(unsigned char address);
void writeDirectReg_nodaisy(unsigned char address, unsigned char data);
unsigned char readDAAReg(unsigned char address);
void writeDAAReg(unsigned char address, unsigned char data);
#endif
/***************************************************************************/



/*==================== FOR RTL8186 ==================*/

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186

//------spi clock rate----------
//-- thlin + 05-08-01 ----
#define SysClock 180000000   // uint in Hz
#define Maxsped 10000000
//------------------------



#ifndef SUCCESS
#define SUCCESS 	0
#endif
#ifndef FAILED
#define FAILED 		-1
#endif





#endif//CONFIG_RTK_VOIP_DRIVERS_PCM8186


/*==================== FOR RTL865x ==================*/

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651


void set_865x_slic_channel(int channel);

int32 _rtl865x_spi_init( rtl8651_spi_dev_t* pDev, uint32 gpioSCLK, uint32 gpioCS_, uint32 gpioSDI, uint32 gpioSDO, int32 maxSpeed );
int32 _rtl865x_spi_rawRead( rtl8651_spi_dev_t* pDev, void* data, int32 bits );
int32 _rtl865x_spi_rawWrite( rtl8651_spi_dev_t* pDev, void* data, int32 bits );
int32 _rtl865x_spi_exit( rtl8651_spi_dev_t* pDev );


#endif//CONFIG_RTK_VOIP_DRIVERS_PCM8651


/*==================== FOR RTL867x ==================*/

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8671

//------spi clock rate----------
#define SysClock 180000000   // uint in Hz
#define Maxsped 10000000
//------------------------

#ifndef SUCCESS
#define SUCCESS 	0
#endif
#ifndef FAILED
#define FAILED 		-1
#endif

#endif//CONFIG_RTK_VOIP_DRIVERS_PCM8671


#endif //_SPI_H_
