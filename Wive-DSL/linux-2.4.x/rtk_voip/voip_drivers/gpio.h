/*
* Copyright c                  Realtek Semiconductor Corporation, 2006  
* All rights reserved.
* 
* Program : GPIO Header File 
* Abstract : 
* Author :                
* 
*/

 
#ifndef __GPIO__
#define __GPIO__
#include <linux/config.h>
#include <asm/mach-realtek/rtl8672/platform.h>
#include "rtl_types.h"

/*==================== FOR RTL8186 EVB gpio pin==================*/
//It's for Z-version board used. GPIO_C used
#define CONFIG_RTK_VOIP_DRIVERS_PCM8186V_Z 0	//0: R-version board, 1: Z-version board
#if CONFIG_RTK_VOIP_DRIVERS_PCM8186V_Z
#define RTL8186PV_RELAY_USE_Z 1	//0: R-version board, 1: Z-version board
#else
#define RTL8186PV_RELAY_USE_Z 0	//0: R-version board, 1: Z-version board
#endif

#ifdef CONFIG_RTK_VOIP_GPIO_8186PV_V275
#define _chiminer_gpiod_fixed 0		//0: GPIO_C, 1: GPIO_D
#define GPIO "C"
#if !CONFIG_RTK_VOIP_DRIVERS_PCM8186V_Z //R-version board USED
/*slic used*/
#define PIN_RESET1	GPIO_ID(GPIO_PORT_C,0)
#define PIN_CS1		GPIO_ID(GPIO_PORT_C,2)	//output
#define PIN_CLK		GPIO_ID(GPIO_PORT_C,3)	//output
#define PIN_DI		GPIO_ID(GPIO_PORT_C,4) 	//input
#define PIN_DO		GPIO_ID(GPIO_PORT_C,6)	//output
/* DAA used. chiminer*/
#define PIN_RESET3_DAA		GPIO_ID(GPIO_PORT_C,10)  //output
#define PIN_CS3_DAA		GPIO_ID(GPIO_PORT_C,11)	//output
#define PIN_CLK_DAA		GPIO_ID(GPIO_PORT_C,12)	//output
#define PIN_DI_DAA		GPIO_ID(GPIO_PORT_C,13) 	//input
#define PIN_DO_DAA		GPIO_ID(GPIO_PORT_C,14)	//output

#else //Z-version board USED
/*slic used*/
#define PIN_RESET1	GPIO_ID(GPIO_PORT_C,1)
#define PIN_CLK		GPIO_ID(GPIO_PORT_C,2)	//output
#define PIN_CS1		GPIO_ID(GPIO_PORT_C,3)	//output
#define PIN_DI		GPIO_ID(GPIO_PORT_C,5) 	//input
#define PIN_DO		GPIO_ID(GPIO_PORT_C,4)	//output
/* DAA used. chiminer*/
#define PIN_RESET3_DAA		GPIO_ID(GPIO_PORT_C,6)  //output
#define PIN_CLK_DAA		GPIO_ID(GPIO_PORT_C,7)	//output
#define PIN_CS3_DAA		GPIO_ID(GPIO_PORT_C,8)	//output
#define PIN_DI_DAA		GPIO_ID(GPIO_PORT_C,10) 	//input
#define PIN_DO_DAA		GPIO_ID(GPIO_PORT_C,9)	//output
#endif //end CONFIG_RTK_VOIP_DRIVERS_PCM8186V_Z
#endif

#ifdef CONFIG_RTK_VOIP_GPIO_8186V_V100_V200_V210_C220
#define _chiminer_gpiod_fixed 1		//0: GPIO_C, 1: GPIO_D
#define GPIO "D"
/*slic used*/
#define PIN_RESET1	GPIO_ID(GPIO_PORT_D,7)
#define PIN_CS1		GPIO_ID(GPIO_PORT_D,10)	//output
#define PIN_CLK		GPIO_ID(GPIO_PORT_D,8)	//output
#define PIN_DI		GPIO_ID(GPIO_PORT_D,5) //input
#define PIN_DO		GPIO_ID(GPIO_PORT_D,0)	//output
/* DAA used. chiminer*/
#define PIN_RESET3_DAA		GPIO_ID(GPIO_PORT_D,13)  //output
#define PIN_CS3_DAA		GPIO_ID(GPIO_PORT_D,11)	//output
#define PIN_CLK_DAA		GPIO_ID(GPIO_PORT_D,12)	//output
#define PIN_DI_DAA		GPIO_ID(GPIO_PORT_D,9) 	//input
#define PIN_DO_DAA		GPIO_ID(GPIO_PORT_D,14)	//output
#endif

#ifdef CONFIG_RTK_VOIP_GPIO_IPP_100
#define _chiminer_gpiod_fixed 1		//0: GPIO_C, 1: GPIO_D
#define GPIO "D"
/*slic used*/
#define PIN_RESET1	GPIO_ID(GPIO_PORT_C,0)
#define PIN_CS1		GPIO_ID(GPIO_PORT_C,2)	//output
#define PIN_CLK		GPIO_ID(GPIO_PORT_C,3)	//output
#define PIN_DI		GPIO_ID(GPIO_PORT_C,4) 	//input
#define PIN_DO		GPIO_ID(GPIO_PORT_C,6)	//output
/* DAA used. chiminer*/
#define PIN_RESET3_DAA		GPIO_ID(GPIO_PORT_D,13)  //output
#define PIN_CS3_DAA		GPIO_ID(GPIO_PORT_D,11)	//output
#define PIN_CLK_DAA		GPIO_ID(GPIO_PORT_D,12)	//output
#define PIN_DI_DAA		GPIO_ID(GPIO_PORT_D,14) 	//input
#define PIN_DO_DAA		GPIO_ID(GPIO_PORT_D,9)	//output
#endif

#ifdef CONFIG_RTK_VOIP_GPIO_IPP_101
#define _chiminer_gpiod_fixed 0		//0: GPIO_C, 1: GPIO_D
#define GPIO "C"
/*slic used*/
#define PIN_RESET1	GPIO_ID(GPIO_PORT_C,0)
#define PIN_CS1		GPIO_ID(GPIO_PORT_C,2)	//output
#define PIN_CLK		GPIO_ID(GPIO_PORT_C,3)	//output
#define PIN_DI		GPIO_ID(GPIO_PORT_C,4) 	//input
#define PIN_DO		GPIO_ID(GPIO_PORT_C,6)	//output
/* DAA used. chiminer*/
#define PIN_RESET3_DAA		GPIO_ID(GPIO_PORT_A,1)  //output
#define PIN_CS3_DAA		GPIO_ID(GPIO_PORT_E,2)	//output
#define PIN_CLK_DAA		GPIO_ID(GPIO_PORT_E,1)	//output
#define PIN_DI_DAA		GPIO_ID(GPIO_PORT_E,3) 	//input
#define PIN_DO_DAA		GPIO_ID(GPIO_PORT_A,0)	//output
#endif
/*====================END FOR RTL8186 EVB gpio pin ==================*/

/*==================== FOR RTL865x EVB gpio pin ==================*/
//#define CONFIG_RTK_VOIP_DRIVERS_PCM8651_T	//if T-version board used
#ifdef CONFIG_RTK_VOIP_GPIO_8651B
#define GPIO "A"
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651_T

#define PIN_CS2		GPIO_ID(GPIO_PORT_C,0)
#define PIN_RESET2	GPIO_ID(GPIO_PORT_C,0)
#define PIN_RELAY	GPIO_ID(GPIO_PORT_E,0)

#define PIN_INT1	GPIO_ID(GPIO_PORT_E,1)
#define PIN_CS1		GPIO_ID(GPIO_PORT_E,6)
#define PIN_RESET1	GPIO_ID(GPIO_PORT_C,5)
#define PIN_CLK		GPIO_ID(GPIO_PORT_B,5)
#define PIN_DI		GPIO_ID(GPIO_PORT_B,6)
#define PIN_DO		GPIO_ID(GPIO_PORT_B,7)

#define PIN_LED1	GPIO_ID(GPIO_PORT_D,6)
#define PIN_LED2	GPIO_ID(GPIO_PORT_D,7)

#else

#define PIN_CS2		GPIO_ID(GPIO_PORT_C,0)
#define PIN_RESET2	GPIO_ID(GPIO_PORT_D,5)

#if 1
#define PIN_CS1		GPIO_ID(GPIO_PORT_E,3)
#define PIN_RESET1	GPIO_ID(GPIO_PORT_E,7)
#define PIN_INT1	GPIO_ID(GPIO_PORT_E,1)
#define PIN_CLK		GPIO_ID(GPIO_PORT_E,4)
#define PIN_DI		GPIO_ID(GPIO_PORT_E,6)
#define PIN_DO		GPIO_ID(GPIO_PORT_E,5)
#else
#define PIN_CS1		GPIO_ID(GPIO_PORT_D,3)
#define PIN_RESET1	GPIO_ID(GPIO_PORT_D,6)
#define PIN_INT1	GPIO_ID(GPIO_PORT_D,4)
#define PIN_CLK		GPIO_ID(GPIO_PORT_D,2)
#define PIN_DI		GPIO_ID(GPIO_PORT_D,1)
#define PIN_DO		GPIO_ID(GPIO_PORT_D,0)
#endif

#define PIN_INT2	GPIO_ID(GPIO_PORT_E,2)
#define PIN_RELAY	GPIO_ID(GPIO_PORT_E,0)

#define PIN_CS1_SiLab		GPIO_ID(GPIO_PORT_D,3)
#define PIN_RESET1_SiLab	GPIO_ID(GPIO_PORT_D,6)
#define PIN_INT1_SiLab		GPIO_ID(GPIO_PORT_D,4)
#define PIN_CLK_SiLab		GPIO_ID(GPIO_PORT_D,2)
#define PIN_DI_SiLab		GPIO_ID(GPIO_PORT_D,1)
#define PIN_DO_SiLab		GPIO_ID(GPIO_PORT_D,0)

#define PIN_LED1	GPIO_ID(GPIO_PORT_C,4)
#define PIN_LED2	GPIO_ID(GPIO_PORT_C,5)
#endif	//endif CONFIG_RTK_VOIP_DRIVERS_PCM8651_T
#endif

#ifdef CONFIG_RTK_VOIP_GPIO_8651C
#define GPIO "A"
/*slic used*/
#define PIN_RESET1	GPIO_ID(GPIO_PORT_A,6)  //output
#define PIN_CS1		GPIO_ID(GPIO_PORT_A,0)	//output
#define PIN_CLK		GPIO_ID(GPIO_PORT_A,1)	//output
#define PIN_DI		GPIO_ID(GPIO_PORT_A,3) 	//input
#define PIN_DO		GPIO_ID(GPIO_PORT_A,2)//5)	//output
/* DAA used*//*undefined*/
#define PIN_RESET3_DAA		GPIO_ID(GPIO_PORT_D,0)  //output
#define PIN_CS3_DAA		GPIO_ID(GPIO_PORT_D,1)	//output
#define PIN_CLK_DAA		GPIO_ID(GPIO_PORT_D,2)	//output
#define PIN_DI_DAA		GPIO_ID(GPIO_PORT_D,3) 	//input
#define PIN_DO_DAA		GPIO_ID(GPIO_PORT_D,4)	//output
#endif

/*====================END FOR RTL865x EVB gpio pin ==================*/

/*==================== FOR RTL867x EVB gpio pin ==================*/
#ifdef CONFIG_RTK_VOIP_GPIO_8671_QA_V1_1_V1_1_2_2
#define GPIO "AB"
/*slic used*/
#define PIN_RESET1	GPIO_ID(GPIO_PORT_B,3)  //output
#define PIN_CS1		GPIO_ID(GPIO_PORT_B,4)	//output
#define PIN_CLK		GPIO_ID(GPIO_PORT_B,5)	//output
#define PIN_DI		GPIO_ID(GPIO_PORT_B,6) 	//input
#define PIN_DO		GPIO_ID(GPIO_PORT_B,7)	//output
/* DAA used*/
#define PIN_RESET3_DAA		GPIO_ID(GPIO_PORT_A,3)  //output
#define PIN_CS3_DAA		GPIO_ID(GPIO_PORT_A,5)	//output
#define PIN_CLK_DAA		GPIO_ID(GPIO_PORT_A,4)	//output
#define PIN_DI_DAA		GPIO_ID(GPIO_PORT_B,7) 	//input
#define PIN_DO_DAA		GPIO_ID(GPIO_PORT_B,6)	//output
#endif

#ifdef CONFIG_RTK_VOIP_GPIO_8671_V1_2_EMI
#define GPIO "AB"
/*slic used*/
#define PIN_RESET1	GPIO_ID(GPIO_PORT_A,3)  //output
#define PIN_CS1		GPIO_ID(GPIO_PORT_A,4)	//output
#define PIN_CLK		GPIO_ID(GPIO_PORT_B,5)	//output
#define PIN_DI		GPIO_ID(GPIO_PORT_B,6) 	//input
#define PIN_DO		GPIO_ID(GPIO_PORT_B,7)	//output
/* DAA used*/
#define PIN_RESET3_DAA		GPIO_ID(GPIO_PORT_A,3)  //output
#define PIN_CS3_DAA		GPIO_ID(GPIO_PORT_A,5)	//output
#define PIN_CLK_DAA		GPIO_ID(GPIO_PORT_A,4)	//output
#define PIN_DI_DAA		GPIO_ID(GPIO_PORT_B,7) 	//input
#define PIN_DO_DAA		GPIO_ID(GPIO_PORT_B,6)	//output
#endif

#ifdef CONFIG_RTK_VOIP_GPIO_8671_V1_2
#define GPIO "AB"
#define USE_8671_GPIO_API	//unifiy the 8671 gpio api

#ifdef USE_8671_GPIO_API
/*port function definition from rtl8671/gpio.h*/
#define GPIO_FUNC_INPUT 	0x0001  /*data input*/
#define GPIO_FUNC_OUTPUT 	0x0002	/*data output*/
#define GPIO_FUNC_IRQ_FALL 	0x0003	/*falling edge IRQ*/
#define GPIO_FUNC_IRQ_RISE 	0x0004	/*rising edge IRQ*/
#define GPIO_FUNC_IRQ_LEVEL 	0x0005	/*level trigger IRQ*/

#define PIN_RESET1	11	//A3
#define PIN_CS1		5	//B5
#define PIN_CLK		12	//A4
#define PIN_DI		7	//B7
#define PIN_DO		6	//B6
/* DAA used*/
#define PIN_RESET3_DAA		11//A3	
#define PIN_CS3_DAA			13//A5
#define PIN_CLK_DAA			12//A4
#define PIN_DI_DAA			 7//B7
#define PIN_DO_DAA			 6//B6
#else 
/*slic used*/
#define PIN_RESET1	GPIO_ID(GPIO_PORT_A,3)  //output
#define PIN_CS1		GPIO_ID(GPIO_PORT_B,5)	//output
#define PIN_CLK		GPIO_ID(GPIO_PORT_A,4)	//output
#define PIN_DI		GPIO_ID(GPIO_PORT_B,7) 	//input
#define PIN_DO		GPIO_ID(GPIO_PORT_B,6)	//output
/* DAA used*/
#define PIN_RESET3_DAA		GPIO_ID(GPIO_PORT_A,3)  //output
#define PIN_CS3_DAA		GPIO_ID(GPIO_PORT_A,5)	//output
#define PIN_CLK_DAA		GPIO_ID(GPIO_PORT_A,4)	//output
#define PIN_DI_DAA		GPIO_ID(GPIO_PORT_B,7) 	//input
#define PIN_DO_DAA		GPIO_ID(GPIO_PORT_B,6)	//output
#endif
#endif

/*====================END FOR RTL867x EVB gpio pin ==================*/



/*==================== FOR RTL8186 ==================*/

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186

#define _GPIO_DEBUG_ 0


/******** GPIO define ********/


/* define GPIO port */
enum GPIO_PORT
{
	GPIO_PORT_A = 0,
	GPIO_PORT_B,
	GPIO_PORT_C,
	GPIO_PORT_D,
	GPIO_PORT_E,
	GPIO_PORT_F,
	GPIO_PORT_G,
	GPIO_PORT_MAX,
};


/* define GPIO direction */
enum GPIO_DIRECTION
{
	GPIO_DIR_IN = 0,
	GPIO_DIR_OUT =1,
};

/* define GPIO Interrupt Type */
enum GPIO_INTERRUPT_TYPE
{
	GPIO_INT_DISABLE = 0,
	GPIO_INT_ENABLE,

};

/*
 * Every pin of GPIO port can be mapped to a unique ID. All the access to a GPIO pin must use the ID.
 * This macro is used to map the port and pin into the ID.
 */
#define GPIO_ID(port,pin) ((uint32)port<<16|(uint32)pin)

/* This is reversed macro. */
#define GPIO_PORT(id) (id>>16)
#define GPIO_PIN(id) (id&0xffff)


/*************** Define RTL8186 GPIO Register Set ************************/

#define	GPABDATA	0xBD010120
#define	GPABDIR		0xBD010124
#define	GPABIMR		0xBD010128
#define	GPABISR		0xBD01012C
#define	GPCDDATA	0xBD010130
#define	GPCDDIR		0xBD010134
#define	GPCDIMR		0xBD010138
#define	GPCDISR		0xBD01013C
#define	GPEFDATA	0xBD010140
#define	GPEFDIR		0xBD010144
#define	GPEFIMR		0xBD010148
#define	GPEFISR		0xBD01014C
#define	GPGDATA		0xBD010150
#define	GPGDIR		0xBD010154
#define	GPGIMR		0xBD010158
#define	GPGISR		0xBD01015C

/*********************  Function Prototype in gpio.c  ***********************/
int32 _rtl8186_initGpioPin( uint32 gpioId, enum GPIO_DIRECTION direction, enum GPIO_INTERRUPT_TYPE interruptEnable );
int32 _rtl8186_getGpioDataBit( uint32 gpioId, uint32* pData );
int32 _rtl8186_setGpioDataBit( uint32 gpioId, uint32 data );

#endif//CONFIG_RTK_VOIP_DRIVERS_PCM8186



/*==================== FOR RTL865x ==================*/

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651

#include "asicRegs.h"

#define PDECNR			0xbd012070
//#define PDEDIR			0xbd012074
#define PDEDATA			0xbd012078

#define PECNR			(PDECNR+1)
#define PEDIR			(PDEDIR+1)
#define PEDATA			(PDEDATA+1)

/* define GPIO port */
enum GPIO_PORT
{
	GPIO_PORT_A = 0,
	GPIO_PORT_B,
	GPIO_PORT_C,
	GPIO_PORT_D,
	GPIO_PORT_E,
	GPIO_PORT_F,
	GPIO_PORT_G,
	GPIO_PORT_H,
	GPIO_PORT_I,
	GPIO_PORT_MAX,
};

/* define GPIO dedicate peripheral pin */
enum GPIO_PERIPHERAL
{
	GPIO_PERI_GPIO = 0,
	GPIO_PERI_TYPE0 = 0x2,
	GPIO_PERI_TYPE1 = 0x3,
};


/* define GPIO direction */
enum GPIO_DIRECTION
{
	GPIO_DIR_IN = 0,
	GPIO_DIR_OUT,
};

/* define GPIO Interrupt Type */
enum GPIO_INTERRUPT_TYPE
{
	GPIO_INT_DISABLE = 0,
	GPIO_INT_FALLING_EDGE,
	GPIO_INT_RISING_EDGE,
	GPIO_INT_BOTH_EDGE,
};

/*
 * Every pin of GPIO port can be mapped to a unique ID. All the access to a GPIO pin must use the ID.
 * This macro is used to map the port and pin into the ID.
 */
#define GPIO_ID(port,pin) ((uint32)port<<16|(uint32)pin)

/* This is reversed macro. */
#define GPIO_PORT(id) (id>>16)
#define GPIO_PIN(id) (id&0xffff)


int32 _rtl865x_initGpioPin(uint32 gpioId, enum GPIO_PERIPHERAL dedicate, 
                                           enum GPIO_DIRECTION, 
                                           enum GPIO_INTERRUPT_TYPE interruptEnable );
int32 _rtl865x_getGpioDataBit( uint32 gpioId, uint32* data );
int32 _rtl865x_setGpioDataBit( uint32 gpioId, uint32 data );
#if 0
int32 _rtl865x_fetchGpioInterruptStatus( uint32 gpioId, uint32* status );
int32 _rtl865x_clearGpioInterruptStatus( uint32 gpioId );
#endif


#endif//CONFIG_RTK_VOIP_DRIVERS_PCM8651


/*==================== FOR RTL867x ==================*/
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8671

#define _GPIO_DEBUG_ 0

/******** GPIO define ********/


/* define GPIO port */
enum GPIO_PORT
{
	GPIO_PORT_A = 0,
	GPIO_PORT_B,
	GPIO_PORT_MAX,
};

#ifdef USE_8671_GPIO_API
/* define GPIO direction */
enum GPIO_DIRECTION
{
	GPIO_DIR_IN = GPIO_FUNC_INPUT,
	GPIO_DIR_OUT =GPIO_FUNC_OUTPUT,
};

#else
/* define GPIO direction */
enum GPIO_DIRECTION
{
	GPIO_DIR_IN = 0,
	GPIO_DIR_OUT =1,
};
#endif
/* define GPIO Interrupt Type */
enum GPIO_INTERRUPT_TYPE
{
	GPIO_INT_DISABLE = 0,
	GPIO_INT_ENABLE,

};

/*
 * Every pin of GPIO port can be mapped to a unique ID. All the access to a GPIO pin must use the ID.
 * This macro is used to map the port and pin into the ID.
 */
#define GPIO_ID(port,pin) ((uint32)port<<16|(uint32)pin)

/* This is reversed macro. */
#define GPIO_PORT(id) (id>>16)
#define GPIO_PIN(id) (id&0xffff)


/*************** Define RTL67x GPIO Register Set ************************/

#define	GPADIR		0xB9C01000
#define	GPADATA		0xB9C01004
#define	GPAISR		0xB9C01008
#define	GPAIMR		0xB9C0100C
#define	GPBDIR		0xB9C01010
#define	GPBDATA		0xB9C01014
#define	GPBISR		0xB9C01018
#define	GPBIMR		0xB9C0101C
/**************************************************************************/
/* Register access macro (REG*()).*/
#define REG32(reg) 			(*((volatile uint32 *)(reg)))
#define REG16(reg) 			(*((volatile uint16 *)(reg)))
#define REG8(reg) 			(*((volatile uint8 *)(reg)))

/*********************  Function Prototype in gpio.c  ***********************/
int32 _rtl867x_initGpioPin( uint32 gpioId, enum GPIO_DIRECTION direction, enum GPIO_INTERRUPT_TYPE interruptEnable );
int32 _rtl867x_getGpioDataBit( uint32 gpioId, uint32* pData );
int32 _rtl867x_setGpioDataBit( uint32 gpioId, uint32 data );

#endif//CONFIG_RTK_VOIP_DRIVERS_PCM8671

#endif/*__GPIO__*/
