#include <linux/config.h>
#include <linux/sched.h>	/* jiffies is defined */
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/string.h>
#include "../include/rtk_voip.h"
#include "spi.h"

static int cur_channel = 0;//for slic.
extern unsigned long volatile jiffies;

#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226
extern unsigned char wideband_mode_ctrl;
#endif

/*==================== FOR RTL8186 ==================*/

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
static int cur_channel_1 = 1; //for daa. chiminer
static rtl8186_spi_dev_t dev[CHANNEL];

/************************************* Set GPIO Pin to SPI ***********************************************************/
/*
@func int32 | _rtl8186_spi_init | Initialize SPI device
@parm rtl8186_spi_dev_t* | pDev | Structure to store device information
@parm uint32 | gpioSCLK | GPIO ID of SCLK
@parm uint32 | gpioCS_ | GPIO ID of CS_
@parm uint32 | gpioSDI | GPIO ID of SDI
@parm uint32 | gpioSDO | GPIO ID of SDO
@parm uint32 | maxSpeed | how fast SPI driver can generate the SCLK signal (unit: HZ)
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
static int32 _rtl8186_spi_init( rtl8186_spi_dev_t* pDev, uint32 gpioSCLK, uint32 gpioCS_, uint32 gpioSDO, uint32 gpioSDI, int32 maxSpeed )
{
	
	pDev->gpioSCLK = gpioSCLK;
	pDev->gpioCS_ = gpioCS_;
	pDev->gpioSDI = gpioSDI;
	pDev->gpioSDO = gpioSDO;
	//pDev->SClkDelayLoop = GetSysClockRate() / maxSpeed;	//@@
	pDev->SClkDelayLoop = SysClock / maxSpeed;
	/*rtlglue_printf("GetSysClockRate()=%d\n",GetSysClockRate());*/

	_rtl8186_initGpioPin( gpioSCLK, GPIO_DIR_OUT, GPIO_INT_DISABLE );
	_rtl8186_initGpioPin( gpioCS_, GPIO_DIR_OUT, GPIO_INT_DISABLE );
	_rtl8186_initGpioPin( gpioSDI, GPIO_DIR_IN, GPIO_INT_DISABLE );
	_rtl8186_initGpioPin( gpioSDO, GPIO_DIR_OUT, GPIO_INT_DISABLE );
	
	return SUCCESS;
}


/*
@func int32 | _rtl8186_spi_rawRead | Read several bits from SPI
@parm rtl8186_spi_dev_t* | pDev | Structure containing device information
@parm uint32* | pData | Pointer to store data
@parm uint32 | bits | Number bits of data wanted to read
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
static int32 _rtl8186_spi_rawRead( rtl8186_spi_dev_t* pDev, void* pData, int32 bits )
{
	uint32 delayLoop;
	uint8* pch = pData;
	
	*pch = 0;

	if ( pData == NULL ) return FAILED;
	
	_rtl8186_setGpioDataBit( pDev->gpioCS_, 1 ); /* raise the CS_ */
	_rtl8186_setGpioDataBit( pDev->gpioSCLK, 1 ); /* raise the SCLK */
	for( delayLoop = pDev->SClkDelayLoop; delayLoop > 0 ; delayLoop-- ); /* delay for a while */

	_rtl8186_setGpioDataBit( pDev->gpioCS_, 0 ); /* fall down the CS_ */
	for( delayLoop = pDev->SClkDelayLoop; delayLoop > 0 ; delayLoop-- ); /* delay for a while */

	for( bits--; bits >= 0; bits-- )
	{
		uint32 buf;

		_rtl8186_setGpioDataBit( pDev->gpioSCLK, 0 ); /* fall down the SCLK */
		for( delayLoop = pDev->SClkDelayLoop; delayLoop > 0 ; delayLoop-- ); /* delay for a while */

		pch[bits/8] &= ~((uint8)1<<(bits&0x7));
		_rtl8186_getGpioDataBit( pDev->gpioSDI, &buf );
		pch[bits/8] |= buf?((uint8)1<<(bits&0x7)):0;
		
		_rtl8186_setGpioDataBit( pDev->gpioSCLK, 1 ); /* raising up the SCLK */
		for( delayLoop = pDev->SClkDelayLoop; delayLoop > 0 ; delayLoop-- ); /* delay for a while */
	}	
	
	_rtl8186_setGpioDataBit( pDev->gpioCS_, 1 ); /* raise the CS_ */

	
	return SUCCESS;
}


/*
@func int32 | _rtl8186_spi_rawWrite | Write several bits from SPI
@parm rtl8186_spi_dev_t* | pDev | Structure containing device information
@parm uint32* | pData | Pointer to data
@parm uint32 | bits | Number bits of data wanting to write
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/



static int32 _rtl8186_spi_rawWrite( rtl8186_spi_dev_t* pDev, void* pData, int32 bits )
{
	uint32 delayLoop;
	uint8* pch = pData;

	if ( pData == NULL ) return FAILED;
	
	_rtl8186_setGpioDataBit( pDev->gpioCS_, 1 ); /* raise the CS_ */
	_rtl8186_setGpioDataBit( pDev->gpioSCLK, 1 ); /* raise the SCLK */
	for( delayLoop = pDev->SClkDelayLoop; delayLoop > 0 ; delayLoop-- ); /* delay for a while */

	_rtl8186_setGpioDataBit( pDev->gpioCS_, 0 ); /* fall down the CS_ */
	for( delayLoop = pDev->SClkDelayLoop; delayLoop > 0 ; delayLoop-- ); /* delay for a while */

	for( bits-- ; bits >= 0; bits-- )
	{
		_rtl8186_setGpioDataBit( pDev->gpioSDO, (pch[bits/8]&((uint32)1<<(bits&0x7)))?1:0 );
		_rtl8186_setGpioDataBit( pDev->gpioSCLK, 0 ); /* fall down the SCLK */
		for( delayLoop = pDev->SClkDelayLoop; delayLoop > 0 ; delayLoop-- ); /* delay for a while */
		_rtl8186_setGpioDataBit( pDev->gpioSCLK, 1 ); /* raising up the SCLK */
		for( delayLoop = pDev->SClkDelayLoop; delayLoop > 0 ; delayLoop-- ); /* delay for a while */
	}	
	
	_rtl8186_setGpioDataBit( pDev->gpioCS_, 1 ); /* raise the CS_ */
	
	
	return SUCCESS;
}


/*
@func int32 | _rtl8186_spi_exit | Called when a SPI device is released
@parm rtl8186_spi_dev_t* | pDev | Structure containing device information
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
int32 _rtl8186_spi_exit( rtl8186_spi_dev_t* pDev )
{
	return SUCCESS;
}
 

/************************* SPI API ****************************************/




//------------------------------------------

void init_channel(int channel, uint32 pin_cs, uint32 pin_reset, uint32 pin_clk, uint32 pin_do, uint32 pin_di)
{
	int i;
	_rtl8186_initGpioPin(pin_reset, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(pin_cs, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(pin_clk, GPIO_DIR_IN, GPIO_INT_DISABLE);
	#if speed_booting_rating
		for(i=0;i<50000;i++);
	#else
		cyg_thread_delay(15);
	#endif
	_rtl8186_setGpioDataBit(pin_cs, 1); 	/* CS_ preset to high state*/
	_rtl8186_setGpioDataBit(pin_reset, 1);	// reset high
	#if speed_booting_rating
		for(i=0;i<50000;i++);
	#else
		cyg_thread_delay(15);
	#endif
	_rtl8186_setGpioDataBit(pin_reset, 0);	// set reset low, PCLK and FS id present and stable.
	#if speed_booting_rating
		for(i=0;i<50000;i++);
	#else
		cyg_thread_delay(15);
	#endif
	_rtl8186_setGpioDataBit(pin_reset, 1);	// release reset
	#if speed_booting_rating
		for(i=0;i<50000;i++);
	#else
		cyg_thread_delay(15);
	#endif				// wait more than 100ms 
	
		 
	_rtl8186_spi_init(&dev[channel], pin_clk, pin_cs, pin_do, pin_di, Maxsped);	

}

//------------------------------------------
void init_spi(int ch_spi)
{
	
	printk("( GPIO %s )  ", GPIO );
	
	if (ch_spi == 0)
	{
		printk("for SLIC[%d]...", ch_spi);
		init_channel(0, PIN_CS1, PIN_RESET1, PIN_CLK, PIN_DO, PIN_DI);
		
		printk("=> OK !\n");
	}
	else if (ch_spi == 1)
	{
		printk("for DAA[%d]...", ch_spi);
		init_channel(1, PIN_CS3_DAA, PIN_RESET3_DAA, PIN_CLK_DAA, PIN_DO_DAA, PIN_DI_DAA);
		printk("=> OK !\n");
	}
	else 
	{
		printk("No GPIO Pin assign for any device -> Can't initialize any device\n");
		
	}
	
	
}

/************************* Read & Write Legerity Register API ***********************************/

static void read_legerity_spi(unsigned int reg, Le88xxx *Legerity)
{
	uint8 buf,i,j=0;
	
	
	//choice which slic enable
	buf = 0x4A;
	_rtl8186_spi_rawWrite(&dev[cur_channel], &buf, 8);
#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226
	if (slic_order == 0) 
		buf = 0x01;	
	else if (slic_order == 1)
		buf = 0x02;
#else
	if (slic_order == 0) {
		if (wideband_mode_ctrl == 0)
			buf = 0x01;
		else if (wideband_mode_ctrl == 1)
			buf = 0x21;		
	} else if (slic_order == 1) {
		if (wideband_mode_ctrl == 0)
			buf = 0x02;
		else if (wideband_mode_ctrl == 1)
			buf = 0x22;	
	}		
#endif	
	_rtl8186_spi_rawWrite(&dev[cur_channel], &buf, 8);	
	
	
	//Some registers only can be written. 
	if ((reg == 0x02) || (reg == 0x04) || (reg == 0x06)) {
		printk("For Le88xxx, this register only can be written.\n");
		return;
	}
	
	//decide how many time to call _rtl8186_spi_rawRead()
	if ((reg == 0x86) || (reg == 0x8A))
		i = 14;
	else if (reg == 0xD2)
		i = 11;
	else if (reg == 0x88)
		i = 12;
	else if (reg == 0x98)
		i = 10;
	else if (reg == 0xD4)		
		i = 8;
	else if (reg == 0x9A)
		i = 5;		
	else if ((reg == 0xD0) || (reg == 0xC4) || (reg == 0xE0) || (reg == 0xEE) || (reg == 0xF2))
		i = 4;
	else if (reg == 0xE4)
		i = 3;
	else if ((reg == 0x4D) || (reg == 0x4F) || (reg == 0x6C) || (reg == 0x73) || (reg == 0x80)  \
		|| (reg == 0x96) || (reg == 0xC6) || (reg == 0xCD)|| (reg == 0x82))
		i = 2;
	else
		i = 1;	
	
	//(reg+1) means reading this register.  	 
	if (reg == 0x4D || reg == 0x4F || reg == 0x73 || reg == 0xCD) {
		buf = reg;
		_rtl8186_spi_rawWrite(&dev[cur_channel], &buf, 8);
	} else {
		buf = reg + 1;
		_rtl8186_spi_rawWrite(&dev[cur_channel], &buf, 8);		
	}
	
	
	//read data from register
	for (j=0;j<i;j++)
		_rtl8186_spi_rawRead(&dev[cur_channel], &(Legerity->byte1)+j, 8);
			
	return;
}	

static void write_legerity_spi(unsigned int reg, Le88xxx *data)
{
	uint8 buf,i,j=0;
	
	
	//choice which slic enable
	buf = 0x4A;
	_rtl8186_spi_rawWrite(&dev[cur_channel], &buf, 8);
#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226
	if (slic_order == 0) 
		buf = 0x01;	
	else if (slic_order == 1)
		buf = 0x02;
#else
	if (slic_order == 0) {
		if (wideband_mode_ctrl == 0)
			buf = 0x01;
		else if (wideband_mode_ctrl == 1)
			buf = 0x21;		
	} else if (slic_order == 1) {
		if (wideband_mode_ctrl == 0)
			buf = 0x02;
		else if (wideband_mode_ctrl == 1)
			buf = 0x22;	
	}		
#endif
	_rtl8186_spi_rawWrite(&dev[cur_channel], &buf, 8);
		
	//Some registers only can be read. 
	if ((reg == 0x73) || (reg == 0xCD) || (reg == 0x4D) || (reg == 0x4F)) {
		printk("For Le88xxx, this register only can be read.\n");
		return;
	}
	
	//decide how many time to call _rtl8186_spi_rawWrite()
	if ((reg == 0x86) || (reg == 0x8A))
		i = 14;
	else if (reg == 0xD2)
		i = 11;
	else if (reg == 0x88)
		i = 12;
	else if (reg == 0x98)
		i = 10;
	else if (reg == 0xD4)		
		i = 8;
	else if (reg == 0x9A)
		i = 5;		
	else if ((reg == 0xD0) || (reg == 0xC4) || (reg == 0xE0) || (reg == 0xEE) || (reg == 0xF2))
		i = 4;
	else if (reg == 0xE4)
		i = 3;
	else if ((reg == 0x6C) || (reg == 0x80) || (reg == 0x82) || (reg == 0x96) || (reg == 0xC6))
		i = 2;
	else if ((reg == 0x02) || (reg == 0x04))
		i = 0;
	else
		i = 1;
			
	buf = reg;
	_rtl8186_spi_rawWrite(&dev[cur_channel], &buf, 8);

	//write data into register
	for (j=0;j<i;j++)
		_rtl8186_spi_rawWrite(&dev[cur_channel], &(data->byte1)+j, 8);
		
}


void readLegerityReg(unsigned char address, Le88xxx *Legerity)
{	

	read_legerity_spi(address, Legerity);
}


void writeLegerityReg(unsigned char address, Le88xxx *data)
{

	write_legerity_spi(address, data);
}

/**********Read and Write Silab slic direct register******************/

static unsigned char read_spi(unsigned int reg)
{
	uint8 buf;
	unsigned long flags;
		
	save_flags(flags); cli();
		
	buf = slic_order;
	_rtl8186_spi_rawWrite(&dev[cur_channel], &buf, 8);
	
	buf = reg | 0x80;
	_rtl8186_spi_rawWrite(&dev[cur_channel], &buf, 8);
		
	_rtl8186_spi_rawRead(&dev[cur_channel], &buf, 8);
	
	restore_flags(flags);
	
	return buf;
}	

static void write_spi(unsigned int reg, unsigned int data)
{
	uint8 buf;
	unsigned long flags;
		
	save_flags(flags); cli();
		
	buf = slic_order;
	_rtl8186_spi_rawWrite(&dev[cur_channel], &buf, 8);
	
	buf = reg;
	_rtl8186_spi_rawWrite(&dev[cur_channel], &buf, 8);

	buf = data;
	_rtl8186_spi_rawWrite(&dev[cur_channel], &buf, 8);
	
	restore_flags(flags);
}



/*****************************************************************************/

/*********************************************************/
//The following function is used to DAA(Si3050 Si3018/19).
//Because of differential SPI interface, people must call 
//those function when they want to initialize DAA. 
//chiminer 12_19_2005
/*********************************************************/

static unsigned char read_spi_daa(unsigned int reg)
{
	uint8 buf;
	
	buf = 0x60;
	_rtl8186_spi_rawWrite(&dev[cur_channel_1], &buf, 8);
	
	buf = reg;	
	_rtl8186_spi_rawWrite(&dev[cur_channel_1], &buf, 8);
		
	_rtl8186_spi_rawRead(&dev[cur_channel_1], &buf, 8);
	return buf;
}	

static void write_spi_daa(unsigned int reg, unsigned int data)
{
	uint8 buf;
	
	buf = 0x20;
	_rtl8186_spi_rawWrite(&dev[cur_channel_1], &buf, 8);
	
	buf = reg;
	_rtl8186_spi_rawWrite(&dev[cur_channel_1], &buf, 8);

	buf = data;
	_rtl8186_spi_rawWrite(&dev[cur_channel_1], &buf, 8);
}

unsigned char readDAAReg(unsigned char address)
{	
#if chiminer_daa_spi_debug		
	printk("read DirectReg %d...\n", address);
#endif
	return read_spi_daa(address);
}


void writeDAAReg(unsigned char address, unsigned char data)
{
#if chiminer_daa_spi_debug				
	printk("write 0x%x to DirectReg %d...\n", data, address);
#endif
	write_spi_daa(address, data);
}


#endif//CONFIG_RTK_VOIP_DRIVERS_PCM8186


/*==================== FOR RTL865x ==================*/

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
static rtl8651_spi_dev_t dev[PCM_CH_NUM];
#if 0
static rtl8651_spi_dev_t dev[CHANNEL];
#endif
/*
@func int32 | _rtl865x_spi_init | Initialize SPI device
@parm rtl8651_spi_dev_t* | pDev | Structure to store device information
@parm uint32 | gpioSCLK | GPIO ID of SCLK
@parm uint32 | gpioCS_ | GPIO ID of CS_
@parm uint32 | gpioSDI | GPIO ID of SDI
@parm uint32 | gpioSDO | GPIO ID of SDO
@parm uint32 | maxSpeed | how fast SPI driver can generate the SCLK signal (unit: HZ)
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
int32 _rtl865x_spi_init( rtl8651_spi_dev_t* pDev, uint32 gpioSCLK, uint32 gpioCS_, uint32 gpioSDI, uint32 gpioSDO, int32 maxSpeed )
{
	pDev->gpioSCLK = gpioSCLK;
	pDev->gpioCS_ = gpioCS_;
	pDev->gpioSDI = gpioSDI;
	pDev->gpioSDO = gpioSDO;
//	pDev->SClkDelayLoop = 0;//GetSysClockRate() / maxSpeed;
	pDev->SClkDelayLoop = 0;

	/*rtlglue_printf("GetSysClockRate()=%d\n",GetSysClockRate());*/

	_rtl865x_initGpioPin( gpioSCLK, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE );
	_rtl865x_initGpioPin( gpioCS_, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE );
	_rtl865x_initGpioPin( gpioSDI, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE );
	_rtl865x_initGpioPin( gpioSDO, GPIO_PERI_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE );
	
	return SUCCESS;
}


/*
@func int32 | _rtl865x_spi_rawRead | Read several bits from SPI
@parm rtl8651_spi_dev_t* | pDev | Structure containing device information
@parm uint32* | pData | Pointer to store data
@parm uint32 | bits | Number bits of data wanted to read
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
int32 _rtl865x_spi_rawRead( rtl8651_spi_dev_t* pDev, void* pData, int32 bits )
{
	uint32 delayLoop;
	uint8* pch = pData;
	
	if ( pData == NULL ) return FAILED;
	
	_rtl865x_setGpioDataBit( pDev->gpioCS_, 1 ); /* raise the CS_ */
	_rtl865x_setGpioDataBit( pDev->gpioSCLK, 1 ); /* raise the SCLK */
	for( delayLoop = pDev->SClkDelayLoop; delayLoop > 0 ; delayLoop-- ); /* delay for a while */

	_rtl865x_setGpioDataBit( pDev->gpioCS_, 0 ); /* fall down the CS_ */
	for( delayLoop = pDev->SClkDelayLoop; delayLoop > 0 ; delayLoop-- ); /* delay for a while */

	for( bits--; bits >= 0; bits-- )
	{
		uint32 buf;

		_rtl865x_setGpioDataBit( pDev->gpioSCLK, 0 ); /* fall down the SCLK */
		for( delayLoop = pDev->SClkDelayLoop; delayLoop > 0 ; delayLoop-- ); /* delay for a while */

		pch[bits/8] &= ~((uint8)1<<(bits&0x7));
		_rtl865x_getGpioDataBit( pDev->gpioSDO, &buf );
		pch[bits/8] |= buf?((uint8)1<<(bits&0x7)):0;
		
		_rtl865x_setGpioDataBit( pDev->gpioSCLK, 1 ); /* raising up the SCLK */
		for( delayLoop = pDev->SClkDelayLoop; delayLoop > 0 ; delayLoop-- ); /* delay for a while */
	}	
	
	_rtl865x_setGpioDataBit( pDev->gpioCS_, 1 ); /* raise the CS_ */
	
	return SUCCESS;
}


/*
@func int32 | _rtl865x_spi_rawWrite | Write several bits from SPI
@parm rtl8651_spi_dev_t* | pDev | Structure containing device information
@parm uint32* | pData | Pointer to data
@parm uint32 | bits | Number bits of data wanting to write
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
int32 _rtl865x_spi_rawWrite( rtl8651_spi_dev_t* pDev, void* pData, int32 bits )
{
	uint32 delayLoop;
	uint8* pch = pData;
	
	if ( pData == NULL ) return FAILED;
	
	_rtl865x_setGpioDataBit( pDev->gpioCS_, 1 ); /* raise the CS_ */
	_rtl865x_setGpioDataBit( pDev->gpioSCLK, 1 ); /* raise the SCLK */
	for( delayLoop = pDev->SClkDelayLoop; delayLoop > 0 ; delayLoop-- ); /* delay for a while */

	_rtl865x_setGpioDataBit( pDev->gpioCS_, 0 ); /* fall down the CS_ */
	for( delayLoop = pDev->SClkDelayLoop; delayLoop > 0 ; delayLoop-- ); /* delay for a while */

	for( bits-- ; bits >= 0; bits-- )
	{
		_rtl865x_setGpioDataBit( pDev->gpioSDI, (pch[bits/8]&((uint32)1<<(bits&0x7)))?1:0 );
		_rtl865x_setGpioDataBit( pDev->gpioSCLK, 0 ); /* fall down the SCLK */
		for( delayLoop = pDev->SClkDelayLoop; delayLoop > 0 ; delayLoop-- ); /* delay for a while */
		_rtl865x_setGpioDataBit( pDev->gpioSCLK, 1 ); /* raising up the SCLK */
		for( delayLoop = pDev->SClkDelayLoop; delayLoop > 0 ; delayLoop-- ); /* delay for a while */
	}
	
	_rtl865x_setGpioDataBit( pDev->gpioCS_, 1 ); /* raise the CS_ */
	
	return SUCCESS;
}


/*
@func int32 | _rtl865x_spi_exit | Called when a SPI device is released
@parm rtl8651_spi_dev_t* | pDev | Structure containing device information
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
int32 _rtl865x_spi_exit( rtl8651_spi_dev_t* pDev )
{
	return SUCCESS;
}

static uint8 read_spi(uint32 reg)
{
	uint8 buf;
	unsigned long flags;
	save_flags(flags); cli();	
	buf = reg | 0x80;
#if 0
	_rtl865x_spi_rawWrite(&dev[0], &buf, 8);
	_rtl865x_spi_rawRead(&dev[0], &buf, 8);
#else
	_rtl865x_spi_rawWrite(&dev[cur_channel], &buf, 8);
	_rtl865x_spi_rawRead(&dev[cur_channel], &buf, 8);
#endif
	restore_flags(flags);
	return buf;
}

static void write_spi(uint32 reg, uint32 data)
{
	uint8 buf;
	unsigned long flags;
	save_flags(flags); cli();
	buf = reg;
	_rtl865x_spi_rawWrite(&dev[cur_channel], &buf, 8);

	buf = data;
	_rtl865x_spi_rawWrite(&dev[cur_channel], &buf, 8);
	restore_flags(flags);
}

void init_channel(int channel, uint32 pin_cs, uint32 pin_reset, uint32 pin_clk, uint32 pin_do, uint32 pin_di, uint32 pin_led)
{
	int i;
	_rtl865x_initGpioPin(pin_cs,    GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl865x_initGpioPin(pin_reset, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);

	#if speed_booting_rating
		for(i=0;i<50000000;i++);
	#else
	cyg_thread_delay(15);
	#endif
	_rtl865x_setGpioDataBit(pin_cs, 1); /* CS_ */
	_rtl865x_setGpioDataBit(pin_reset, 1);
	#if speed_booting_rating
		for(i=0;i<5000000;i++);
	#else
	cyg_thread_delay(15);
	#endif
	
	_rtl865x_setGpioDataBit(pin_reset, 0);
	#if speed_booting_rating
		for(i=0;i<5000000;i++);
	#else
		cyg_thread_delay(15);
	#endif						/* wait more than 100ms */
	_rtl865x_setGpioDataBit(pin_reset, 1);
	#if speed_booting_rating
		for(i=0;i<5000000;i++);
	#else
	cyg_thread_delay(15);
	#endif
	_rtl865x_spi_init(&dev[channel], pin_clk, pin_cs, pin_do, pin_di, 10000);	

	/* jason++ 2005/11/15 add for led support */	
	_rtl865x_initGpioPin(pin_led, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl865x_setGpioDataBit(pin_led, 1);	// turn off during the initial time
}

void init_spi(int ch_spi)
{
	static unsigned int relay_flag = 0;
	if(relay_flag == 0)
		_rtl865x_initGpioPin(PIN_RELAY, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);

	/*Slic 0*/
	if(ch_spi==0)
	{
		printk("Init gpio for spi.\n");
		init_channel(0, PIN_CS1, PIN_RESET1, PIN_CLK, PIN_DO, PIN_DI, PIN_LED1);
	}
	/*Slic 0*/
	if(ch_spi==1)
		init_channel(1, PIN_CS2, PIN_RESET2, PIN_CLK, PIN_DO, PIN_DI, PIN_LED2);

	if(relay_flag == 0)
		_rtl865x_setGpioDataBit(PIN_RELAY, 1);
	relay_flag = 1;

}

void set_865x_slic_channel(int channel)
{
	cur_channel = channel;
}

/************************* Read & Write Legerity Register API ***********************************/

static void read_legerity_spi(unsigned int reg, Le88xxx *Legerity)
{
	uint8 buf,i,j=0;
	
	//choice which slic enable
	buf = 0x4A;
	_rtl865x_spi_rawWrite(&dev[cur_channel], &buf, 8);
#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226
	if (slic_order == 0) 
		buf = 0x01;	
	else if (slic_order == 1)
		buf = 0x02;
#else
	if (slic_order == 0) {
		if (wideband_mode_ctrl == 0)
			buf = 0x01;
		else if (wideband_mode_ctrl == 1)
			buf = 0x21;		
	} else if (slic_order == 1) {
		if (wideband_mode_ctrl == 0)
			buf = 0x02;
		else if (wideband_mode_ctrl == 1)
			buf = 0x22;	
	}		
#endif
	_rtl865x_spi_rawWrite(&dev[cur_channel], &buf, 8);	
		
	//Some registers only can be written. 
	if ((reg == 0x02) || (reg == 0x04) || (reg == 0x06)) {
		printk("For Le88xxx, this register only can be written.\n");
		return;
	}
	
	//decide how many time to call _rtl865x_spi_rawRead()
	if ((reg == 0x86) || (reg == 0x8A))
		i = 14;
	else if (reg == 0xD2)
		i = 11;
	else if (reg == 0x88)
		i = 12;
	else if (reg == 0x98)
		i = 10;
	else if (reg == 0xD4)		
		i = 8;
	else if (reg == 0x9A)
		i = 5;		
	else if ((reg == 0xD0) || (reg == 0xC4) || (reg == 0xE0) || (reg == 0xEE) || (reg == 0xF2))
		i = 4;
	else if (reg == 0xE4)
		i = 3;
	else if ((reg == 0x4D) || (reg == 0x4F) || (reg == 0x6C) || (reg == 0x73) || (reg == 0x80)  \
		|| (reg == 0x96) || (reg == 0xC6) || (reg == 0xCD)|| (reg == 0x82))
		i = 2;
	else
		i = 1;	
	
	//(reg+1) means reading this register.  	 
	if (reg == 0x4D || reg == 0x4F || reg == 0x73 || reg == 0xCD) {
		buf = reg;
		_rtl865x_spi_rawWrite(&dev[cur_channel], &buf, 8);
	} else {
		buf = reg + 1;
		_rtl865x_spi_rawWrite(&dev[cur_channel], &buf, 8);		
	}
		
	//read data from register
	for (j=0;j<i;j++)
		_rtl865x_spi_rawRead(&dev[cur_channel], &(Legerity->byte1)+j, 8);
			
	return;
}	

static void write_legerity_spi(unsigned int reg, Le88xxx *data)
{
	uint8 buf,i,j=0;
	
	
	//choice which slic enable
	buf = 0x4A;
	_rtl865x_spi_rawWrite(&dev[cur_channel], &buf, 8);
#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226
	if (slic_order == 0) 
		buf = 0x01;	
	else if (slic_order == 1)
		buf = 0x02;
#else
	if (slic_order == 0) {
		if (wideband_mode_ctrl == 0)
			buf = 0x01;
		else if (wideband_mode_ctrl == 1)
			buf = 0x21;		
	} else if (slic_order == 1) {
		if (wideband_mode_ctrl == 0)
			buf = 0x02;
		else if (wideband_mode_ctrl == 1)
			buf = 0x22;	
	}		
#endif
	_rtl865x_spi_rawWrite(&dev[cur_channel], &buf, 8);
		
	//Some registers only can be read. 
	if ((reg == 0x73) || (reg == 0xCD) || (reg == 0x4D) || (reg == 0x4F)) {
		printk("For Le88xxx, this register only can be read.\n");
		return;
	}
	
	//decide how many time to call _rtl865x_spi_rawWrite()
	if ((reg == 0x86) || (reg == 0x8A))
		i = 14;
	else if (reg == 0xD2)
		i = 11;
	else if (reg == 0x88)
		i = 12;
	else if (reg == 0x98)
		i = 10;
	else if (reg == 0xD4)		
		i = 8;
	else if (reg == 0x9A)
		i = 5;		
	else if ((reg == 0xD0) || (reg == 0xC4) || (reg == 0xE0) || (reg == 0xEE) || (reg == 0xF2))
		i = 4;
	else if (reg == 0xE4)
		i = 3;
	else if ((reg == 0x6C) || (reg == 0x80) || (reg == 0x82) || (reg == 0x96) || (reg == 0xC6))
		i = 2;
	else if ((reg == 0x02) || (reg == 0x04))
		i = 0;
	else
		i = 1;
			
	buf = reg;
	_rtl865x_spi_rawWrite(&dev[cur_channel], &buf, 8);

	//write data into register
	for (j=0;j<i;j++)
		_rtl865x_spi_rawWrite(&dev[cur_channel], &(data->byte1)+j, 8);
		
}


void readLegerityReg(unsigned char address, Le88xxx *Legerity)
{	

	read_legerity_spi(address, Legerity);
}


void writeLegerityReg(unsigned char address, Le88xxx *data)
{

	write_legerity_spi(address, data);
}


#endif/*CONFIG_RTK_VOIP_DRIVERS_PCM8651*/

/*==================== FOR RTL867x ==================*/

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8671
static int cur_channel_1 = 1; //for daa. chiminer
static rtl867x_spi_dev_t dev[CHANNEL];

/************************************* Set GPIO Pin to SPI ***********************************************************/
/*
@func int32 | _rtl867x_spi_init | Initialize SPI device
@parm rtl867x_spi_dev_t* | pDev | Structure to store device information
@parm uint32 | gpioSCLK | GPIO ID of SCLK
@parm uint32 | gpioCS_ | GPIO ID of CS_
@parm uint32 | gpioSDI | GPIO ID of SDI
@parm uint32 | gpioSDO | GPIO ID of SDO
@parm uint32 | maxSpeed | how fast SPI driver can generate the SCLK signal (unit: HZ)
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
static int32 _rtl867x_spi_init( rtl867x_spi_dev_t* pDev, uint32 gpioSCLK, uint32 gpioCS_, uint32 gpioSDO, uint32 gpioSDI, int32 maxSpeed )
{
	
	pDev->gpioSCLK = gpioSCLK;
	pDev->gpioCS_ = gpioCS_;
	pDev->gpioSDI = gpioSDI;
	pDev->gpioSDO = gpioSDO;
	//pDev->SClkDelayLoop = GetSysClockRate() / maxSpeed;	//@@
	pDev->SClkDelayLoop = SysClock / maxSpeed;
	/*rtlglue_printf("GetSysClockRate()=%d\n",GetSysClockRate());*/

	_rtl867x_initGpioPin( gpioSCLK, GPIO_DIR_OUT, GPIO_INT_DISABLE );
	_rtl867x_initGpioPin( gpioCS_, GPIO_DIR_OUT, GPIO_INT_DISABLE );
	_rtl867x_initGpioPin( gpioSDI, GPIO_DIR_IN, GPIO_INT_DISABLE );
	_rtl867x_initGpioPin( gpioSDO, GPIO_DIR_OUT, GPIO_INT_DISABLE );
	
	return SUCCESS;
}


/*
@func int32 | _rtl867x_spi_rawRead | Read several bits from SPI
@parm rtl867x_spi_dev_t* | pDev | Structure containing device information
@parm uint32* | pData | Pointer to store data
@parm uint32 | bits | Number bits of data wanted to read
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
static int32 _rtl867x_spi_rawRead( rtl867x_spi_dev_t* pDev, void* pData, int32 bits )
{
	uint32 delayLoop;
	uint8* pch = pData;
	
	*pch = 0;

	if ( pData == NULL ) return FAILED;
#ifdef CONFIG_RTL8671
// added by eric for pull high another cs
	if (pDev->gpioCS_ == PIN_CS1)
		_rtl867x_setGpioDataBit( PIN_CS3_DAA, 1 ); /* raise the CS_ */
	else if (pDev->gpioCS_ == PIN_CS3_DAA)
		_rtl867x_setGpioDataBit( PIN_CS1, 1 ); /* raise the CS_ */
	else
		printk("\r\n8671: error");
#endif
	_rtl867x_setGpioDataBit( pDev->gpioCS_, 1 ); /* raise the CS_ */
	_rtl867x_setGpioDataBit( pDev->gpioSCLK, 1 ); /* raise the SCLK */
	for( delayLoop = pDev->SClkDelayLoop; delayLoop > 0 ; delayLoop-- ); /* delay for a while */

	_rtl867x_setGpioDataBit( pDev->gpioCS_, 0 ); /* fall down the CS_ */
	for( delayLoop = pDev->SClkDelayLoop; delayLoop > 0 ; delayLoop-- ); /* delay for a while */

	for( bits--; bits >= 0; bits-- )
	{
		uint32 buf;

		_rtl867x_setGpioDataBit( pDev->gpioSCLK, 0 ); /* fall down the SCLK */
		for( delayLoop = pDev->SClkDelayLoop; delayLoop > 0 ; delayLoop-- ); /* delay for a while */

		pch[bits/8] &= ~((uint8)1<<(bits&0x7));
		_rtl867x_getGpioDataBit( pDev->gpioSDI, &buf );
		pch[bits/8] |= buf?((uint8)1<<(bits&0x7)):0;
		
		_rtl867x_setGpioDataBit( pDev->gpioSCLK, 1 ); /* raising up the SCLK */
		for( delayLoop = pDev->SClkDelayLoop; delayLoop > 0 ; delayLoop-- ); /* delay for a while */
	}	
	
	_rtl867x_setGpioDataBit( pDev->gpioCS_, 1 ); /* raise the CS_ */

	
	
	return SUCCESS;
}


/*
@func int32 | _rtl867x_spi_rawWrite | Write several bits from SPI
@parm rtl867x_spi_dev_t* | pDev | Structure containing device information
@parm uint32* | pData | Pointer to data
@parm uint32 | bits | Number bits of data wanting to write
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/



static int32 _rtl867x_spi_rawWrite( rtl867x_spi_dev_t* pDev, void* pData, int32 bits )
{
	uint32 delayLoop;
	uint8* pch = pData;

	if ( pData == NULL ) return FAILED;
#ifdef CONFIG_RTL8671
// added by eric for pull high another cs
	if (pDev->gpioCS_ == PIN_CS1)
		_rtl867x_setGpioDataBit( PIN_CS3_DAA, 1 ); /* raise the CS_ */
	else if (pDev->gpioCS_ == PIN_CS3_DAA)
		_rtl867x_setGpioDataBit( PIN_CS1, 1 ); /* raise the CS_ */
	else
		printk("\r\n8671: error");
#endif
	_rtl867x_setGpioDataBit( pDev->gpioCS_, 1 ); /* raise the CS_ */
	_rtl867x_setGpioDataBit( pDev->gpioSCLK, 1 ); /* raise the SCLK */
	for( delayLoop = pDev->SClkDelayLoop; delayLoop > 0 ; delayLoop-- ); /* delay for a while */

	_rtl867x_setGpioDataBit( pDev->gpioCS_, 0 ); /* fall down the CS_ */
	for( delayLoop = pDev->SClkDelayLoop; delayLoop > 0 ; delayLoop-- ); /* delay for a while */

	for( bits-- ; bits >= 0; bits-- )
	{
		_rtl867x_setGpioDataBit( pDev->gpioSDO, (pch[bits/8]&((uint32)1<<(bits&0x7)))?1:0 );
		_rtl867x_setGpioDataBit( pDev->gpioSCLK, 0 ); /* fall down the SCLK */
		for( delayLoop = pDev->SClkDelayLoop; delayLoop > 0 ; delayLoop-- ); /* delay for a while */
		_rtl867x_setGpioDataBit( pDev->gpioSCLK, 1 ); /* raising up the SCLK */
		for( delayLoop = pDev->SClkDelayLoop; delayLoop > 0 ; delayLoop-- ); /* delay for a while */
	}	
	
	_rtl867x_setGpioDataBit( pDev->gpioCS_, 1 ); /* raise the CS_ */

	
	
	return SUCCESS;
}


/*
@func int32 | _rtl867x_spi_exit | Called when a SPI device is released
@parm rtl867x_spi_dev_t* | pDev | Structure containing device information
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
int32 _rtl867x_spi_exit( rtl867x_spi_dev_t* pDev )
{
	return SUCCESS;
}
 

/************************* SPI API ****************************************/




//------------------------------------------

void init_channel(int channel, uint32 pin_cs, uint32 pin_reset, uint32 pin_clk, uint32 pin_do, uint32 pin_di)
{
	int i;
	static unsigned char inital = 0;
	_rtl867x_initGpioPin(pin_reset, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl867x_initGpioPin(pin_cs, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	
	if ((channel == 0) || (inital == 0)) {
	#if speed_booting_rating
		for(i=0;i<50000;i++);
	#else
		cyg_thread_delay(15);
	#endif
	_rtl867x_setGpioDataBit(pin_clk, 1); 	/* SPI_CLK should be high when reset*/
	_rtl867x_setGpioDataBit(pin_cs, 1); 	/* CS_ preset to high state*/
	_rtl867x_setGpioDataBit(pin_reset, 1);	// reset high
	#if speed_booting_rating
		for(i=0;i<50000;i++);
	#else
		cyg_thread_delay(15);
	#endif
	_rtl867x_setGpioDataBit(pin_reset, 0);	// set reset low, PCLK and FS id present and stable.
	#if speed_booting_rating
		for(i=0;i<50000;i++);
	#else
		cyg_thread_delay(15);
	#endif
	_rtl867x_setGpioDataBit(pin_reset, 1);	// release reset
	#if speed_booting_rating
		for(i=0;i<50000;i++);
	#else
		cyg_thread_delay(15);
	#endif				// wait more than 100ms 
		inital++;
	}
		 
	_rtl867x_spi_init(&dev[channel], pin_clk, pin_cs, pin_do, pin_di, Maxsped);	

}

//------------------------------------------
void init_spi(int ch_spi)
{
	
	printk("( GPIO %s )  ", GPIO );
	
	if (ch_spi == 0)
	{
		printk("for SLIC[%d]...", ch_spi);
		init_channel(0, PIN_CS1, PIN_RESET1, PIN_CLK, PIN_DO, PIN_DI);
		
		printk("=> OK !\n");
	}
	else if (ch_spi == 1)
	{
		printk("for DAA[%d]...", ch_spi);
		init_channel(1, PIN_CS3_DAA, PIN_RESET3_DAA, PIN_CLK_DAA, PIN_DO_DAA, PIN_DI_DAA);
		printk("=> OK !\n");
	}
	else 
	{
		printk("No GPIO Pin assign for any device -> Can't initialize any device\n");
		
	}
	
	
}

/************************* Read & Write Legerity Register API ***********************************/

static void read_legerity_spi(unsigned int reg, Le88xxx *Legerity)
{
	uint8 buf,i,j=0;
	
	//choice which slic enable
	buf = 0x4A;
	_rtl867x_spi_rawWrite(&dev[cur_channel], &buf, 8);
#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226
	if (slic_order == 0) 
		buf = 0x01;	
	else if (slic_order == 1)
		buf = 0x02;
#else
	if (slic_order == 0) {
		if (wideband_mode_ctrl == 0)
			buf = 0x01;
		else if (wideband_mode_ctrl == 1)
			buf = 0x21;		
	} else if (slic_order == 1) {
		if (wideband_mode_ctrl == 0)
			buf = 0x02;
		else if (wideband_mode_ctrl == 1)
			buf = 0x22;	
	}		
#endif
	_rtl867x_spi_rawWrite(&dev[cur_channel], &buf, 8);	
		
	//Some registers only can be written. 
	if ((reg == 0x02) || (reg == 0x04) || (reg == 0x06)) {
		printk("For Le88xxx, this register only can be written.\n");
		return;
	}
	
	//decide how many time to call _rtl867x_spi_rawRead()
	if ((reg == 0x86) || (reg == 0x8A))
		i = 14;
	else if (reg == 0xD2)
		i = 11;
	else if (reg == 0x88)
		i = 12;
	else if (reg == 0x98)
		i = 10;
	else if (reg == 0xD4)		
		i = 8;
	else if (reg == 0x9A)
		i = 5;		
	else if ((reg == 0xD0) || (reg == 0xC4) || (reg == 0xE0) || (reg == 0xEE) || (reg == 0xF2))
		i = 4;
	else if (reg == 0xE4)
		i = 3;
	else if ((reg == 0x4D) || (reg == 0x4F) || (reg == 0x6C) || (reg == 0x73) || (reg == 0x80)  \
		|| (reg == 0x96) || (reg == 0xC6) || (reg == 0xCD)|| (reg == 0x82))
		i = 2;
	else
		i = 1;	
	
	//(reg+1) means reading this register.  	 
	if (reg == 0x4D || reg == 0x4F || reg == 0x73 || reg == 0xCD) {
		buf = reg;
		_rtl867x_spi_rawWrite(&dev[cur_channel], &buf, 8);
	} else {
		buf = reg + 1;
		_rtl867x_spi_rawWrite(&dev[cur_channel], &buf, 8);		
	}
	
	//read data from register
	for (j=0;j<i;j++)
		_rtl867x_spi_rawRead(&dev[cur_channel], &(Legerity->byte1)+j, 8);
			
	return;
}	

static void write_legerity_spi(unsigned int reg, Le88xxx *data)
{
	uint8 buf,i,j=0;
	
	
	#if 1 //0:  for slic3215 test
	//choice which slic enable
	buf = 0x4A;
	_rtl867x_spi_rawWrite(&dev[cur_channel], &buf, 8);
#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226
	if (slic_order == 0) 
		buf = 0x01;	
	else if (slic_order == 1)
		buf = 0x02;
#else
	if (slic_order == 0) {
		if (wideband_mode_ctrl == 0)
			buf = 0x01;
		else if (wideband_mode_ctrl == 1)
			buf = 0x21;		
	} else if (slic_order == 1) {
		if (wideband_mode_ctrl == 0)
			buf = 0x02;
		else if (wideband_mode_ctrl == 1)
			buf = 0x22;	
	}		
#endif
	_rtl867x_spi_rawWrite(&dev[cur_channel], &buf, 8);
	#endif
	
	//Some registers only can be read. 
	if ((reg == 0x73) || (reg == 0xCD) || (reg == 0x4D) || (reg == 0x4F)) {
		printk("For Le88xxx, this register only can be read.\n");
		return;
	}
	
	//decide how many time to call _rtl867x_spi_rawWrite()
	if ((reg == 0x86) || (reg == 0x8A))
		i = 14;
	else if (reg == 0xD2)
		i = 11;
	else if (reg == 0x88)
		i = 12;
	else if (reg == 0x98)
		i = 10;
	else if (reg == 0xD4)		
		i = 8;
	else if (reg == 0x9A)
		i = 5;		
	else if ((reg == 0xD0) || (reg == 0xC4) || (reg == 0xE0) || (reg == 0xEE) || (reg == 0xF2))
		i = 4;
	else if (reg == 0xE4)
		i = 3;
	else if ((reg == 0x6C) || (reg == 0x80) || (reg == 0x82) || (reg == 0x96) || (reg == 0xC6))
		i = 2;
	else if ((reg == 0x02) || (reg == 0x04))
		i = 0;
	else
		i = 1;
			
	buf = reg;
	_rtl867x_spi_rawWrite(&dev[cur_channel], &buf, 8);

	//write data into register
	for (j=0;j<i;j++)
		_rtl867x_spi_rawWrite(&dev[cur_channel], &(data->byte1)+j, 8);
		
}


void readLegerityReg(unsigned char address, Le88xxx *Legerity)
{	

	read_legerity_spi(address, Legerity);
}


void writeLegerityReg(unsigned char address, Le88xxx *data)
{

	write_legerity_spi(address, data);
}

/**********Read and Write Silab slic direct register******************/

static unsigned char read_spi(unsigned int reg)
{
	uint8 buf;
	unsigned long flags;
		
	save_flags(flags); cli();
		
	buf = slic_order;
	_rtl867x_spi_rawWrite(&dev[cur_channel], &buf, 8);
	
	buf = reg | 0x80;
	_rtl867x_spi_rawWrite(&dev[cur_channel], &buf, 8);
		
	_rtl867x_spi_rawRead(&dev[cur_channel], &buf, 8);
	
	restore_flags(flags);
	
	return buf;
}	

static void write_spi(unsigned int reg, unsigned int data)
{
	uint8 buf;
	unsigned long flags;
		
	save_flags(flags); cli();
		
	buf = slic_order;
	_rtl867x_spi_rawWrite(&dev[cur_channel], &buf, 8);
	
	buf = reg;
	_rtl867x_spi_rawWrite(&dev[cur_channel], &buf, 8);

	buf = data;
	_rtl867x_spi_rawWrite(&dev[cur_channel], &buf, 8);
	
	restore_flags(flags);
}



/*****************************************************************************/

/*********************************************************/
//The following function is used to DAA(Si3050 Si3018/19).
//Because of differential SPI interface, people must call 
//those function when they want to initialize DAA. 
//chiminer 12_19_2005
/*********************************************************/

static unsigned char read_spi_daa(unsigned int reg)
{
	uint8 buf;

	unsigned long flags;
		
	save_flags(flags); cli();
	
	buf = 0x60;
	_rtl867x_spi_rawWrite(&dev[cur_channel_1], &buf, 8);
	
	buf = reg;	
	_rtl867x_spi_rawWrite(&dev[cur_channel_1], &buf, 8);
		
	_rtl867x_spi_rawRead(&dev[cur_channel_1], &buf, 8);

	restore_flags(flags);
	return buf;
}	

static void write_spi_daa(unsigned int reg, unsigned int data)
{
	uint8 buf;
	unsigned long flags;
		
	save_flags(flags); cli();
	
	buf = 0x20;
	_rtl867x_spi_rawWrite(&dev[cur_channel_1], &buf, 8);
	
	buf = reg;
	_rtl867x_spi_rawWrite(&dev[cur_channel_1], &buf, 8);

	buf = data;
	_rtl867x_spi_rawWrite(&dev[cur_channel_1], &buf, 8);
	restore_flags(flags);
}

unsigned char readDAAReg(unsigned char address)
{	
#if chiminer_daa_spi_debug		
	printk("read DirectReg %d...\n", address);
#endif
	return read_spi_daa(address);
}


void writeDAAReg(unsigned char address, unsigned char data)
{
#if chiminer_daa_spi_debug				
	printk("write 0x%x to DirectReg %d...\n", data, address);
#endif
	write_spi_daa(address, data);
}


#endif//CONFIG_RTK_VOIP_DRIVERS_PCM8671


unsigned char readDirectReg(unsigned char address)
{	
	return read_spi(address);
}


void writeDirectReg(unsigned char address, unsigned char data)
{

	write_spi(address, data);
}

unsigned short readIndirectReg(unsigned char address)
{ 
	waitForIndirectReg();
	 
	writeDirectReg(30,address); 
	waitForIndirectReg();
	return ( readDirectReg(28) | (readDirectReg (29))<<8);
}


void writeIndirectReg(unsigned char address, unsigned short data)
{
	waitForIndirectReg();
	writeDirectReg(28,(unsigned char)(data & 0xFF));
	writeDirectReg(29,(unsigned char)((data & 0xFF00)>>8));
	writeDirectReg(30,address);
}

void waitForIndirectReg(void)
{
	while (readDirectReg(31));
}

void cyg_thread_delay(int delay)
{
	unsigned long t1 = jiffies+delay;

	while(jiffies < t1);

}


/*******************************************************************
* Note: The following IndirectReg related function calls are only for Si321x used. *
********************************************************************/



static unsigned char read_spi_nodaisy(unsigned int reg)
{
	uint8 buf;
	
	buf = reg | 0x80;
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
	_rtl865x_spi_rawWrite(&dev[cur_channel], &buf, 8);
	_rtl865x_spi_rawRead(&dev[cur_channel], &buf, 8);
#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8186 
	_rtl8186_spi_rawWrite(&dev[cur_channel], &buf, 8);
	_rtl8186_spi_rawRead(&dev[cur_channel], &buf, 8);
#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8671
	_rtl867x_spi_rawWrite(&dev[cur_channel], &buf, 8);
	_rtl867x_spi_rawRead(&dev[cur_channel], &buf, 8);
#endif	
	return buf;
}

unsigned char readDirectReg_nodaisy(unsigned char address)
{
	return read_spi_nodaisy(address);
}

static void write_spi_nodaisy(unsigned int reg, unsigned int data)
{
	uint8 buf;
	
	buf = reg;
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
	_rtl865x_spi_rawWrite(&dev[cur_channel], &buf, 8);
	buf = data;
	_rtl865x_spi_rawWrite(&dev[cur_channel], &buf, 8);
#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8186
	_rtl8186_spi_rawWrite(&dev[cur_channel], &buf, 8);
	buf = data;
	_rtl8186_spi_rawWrite(&dev[cur_channel], &buf, 8);
#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8671
	_rtl867x_spi_rawWrite(&dev[cur_channel], &buf, 8);
	buf = data;
	_rtl867x_spi_rawWrite(&dev[cur_channel], &buf, 8);
#endif	
}

void writeDirectReg_nodaisy(unsigned char address, unsigned char data)
{
	write_spi_nodaisy(address, data);
}


/**************Winbond slic W682388 ******************/
/****************Write and read API*******************/
int WriteReg(unsigned char Address, unsigned char Data)
{
	const unsigned char cmd = 0x04;
	unsigned char buf;
	unsigned short cmd_address;
	
	cmd_address = (Address<<8) | cmd;
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
	_rtl865x_spi_rawWrite(&dev[cur_channel], &cmd_address, 16);
#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8186	
	_rtl8186_spi_rawWrite(&dev[cur_channel], &cmd_address, 16);
#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8671
	_rtl867x_spi_rawWrite(&dev[cur_channel], &cmd_address, 16);
#endif	
	
	buf = Data;
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
	_rtl865x_spi_rawWrite(&dev[cur_channel], &buf, 8);
#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8186	
	_rtl8186_spi_rawWrite(&dev[cur_channel], &buf, 8);
#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8671
	_rtl867x_spi_rawWrite(&dev[cur_channel], &buf, 8);
#endif	

	return 0;
}

unsigned char ReadReg(unsigned char Address)
{
	const unsigned char cmd = 0x84;
	unsigned char buf;
	unsigned short cmd_address;
	
	cmd_address = (Address<<8) | cmd;
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
	_rtl865x_spi_rawWrite(&dev[cur_channel], &cmd_address, 16);
#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8186	
	_rtl8186_spi_rawWrite(&dev[cur_channel], &cmd_address, 16);
#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8671
	_rtl867x_spi_rawWrite(&dev[cur_channel], &cmd_address, 16);
#endif	

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
	_rtl865x_spi_rawRead(&dev[cur_channel], &buf, 8);
#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8186	
	_rtl8186_spi_rawRead(&dev[cur_channel], &buf, 8);
#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8671
	_rtl867x_spi_rawRead(&dev[cur_channel], &buf, 8);
#endif
	
	return buf;

}


