
#include <asm/mach-realtek/rtl8672/platform.h>

extern unsigned char rtl8186_gpio_async;

#if defined(CONFIG_RTK_VOIP_GPIO_IPP_100)
#define LCM_GPIO_PIN	1		//0: GPIO_B/D/F used ,1: GPIO_A/C/E used
#define WM8510_GPIO_PIN		0	//0: GPIO_B/D/F used ,1: GPIO_A/C/E used
#define KEYPAD_GPIO_PIN		1	//0: GPIO_B/D/F used ,1: GPIO_A/C/E used
#define LED_GPIO_PIN		0	//0: GPIO_B/D/F used ,1: GPIO_A/C/E used
#elif defined(CONFIG_RTK_VOIP_GPIO_IPP_101)
#define LCM_GPIO_PIN	1		//0: GPIO_B/D/F used ,1: GPIO_A/C/E used
#define WM8510_GPIO_PIN		1	//0: GPIO_B/D/F used ,1: GPIO_A/C/E used
#define KEYPAD_GPIO_PIN		1	//0: GPIO_B/D/F used ,1: GPIO_A/C/E used
#define LED_GPIO_PIN		1	//0: GPIO_B/D/F used ,1: GPIO_A/C/E used
#endif

//-----------------------------
#if 1	//0: kernel used(udelay), 1: test program used(for(;;)) 
#define __test_program_used__
#else
#define __kernel_used__
#endif
//---------------------- LCM --------------------------------------------
#define _LCM_PIN_
#define RATING_FACTOR	2	//Adjust LCM read and write rating.
//cli() protection for kernel used. Don't define MACRO LCM_CLI_PROTECT in test program.
//#define LCM_CLI_PROTECT

//----------------------- WM8510 codec ----------------------------------------------
#define I2C_RATING_FACTOR	10	//Adjust I2C read and write rating.
//There is an I2C protocal.
#define _I2C_WM8510_ 	//GPIO pin
//cli() protection for kernel used. Don't define MACRO _WM8510_CLI_PROTECT in test program.
//#define _WM8510_CLI_PROTECT

//----------------------- keypad -------------------------------------------
#define _KEYPAD_PIN_	//GPIO pin
#define KEYPAD_RATING_FACTOR	4	//debounce check
#define KEYPAD_ROW_NUMBER	6	//input 
#define KEYPAD_COLUMN_NUMBER	7	//output
//The GPIO's interrupt of 8186 is edge-trigger. So when u want to use interrupt mode, 
//PLEASE CHOOSE MACRO KEYPAD_INTERRUPT_ENABLE
#if 1		
#define INPUT_PULL_HIGH_RESISTOR	//input pull-high resistor.for interrupt mode or polling mode
#define CHECK_FLAG	0
#else		
#define INPUT_PULL_LOW_RESISTOR		//input pull-low resistor.for interrupt mode or polling mode 
#define CHECK_FLAG	1
#endif
#if 1		//enable interrupt
#define KEYPAD_INTERRUPT_ENABLE
#else		//disable interrupt
#define KEYPAD_INTERRUPT_DISABLE
#endif
//-----------------------------------------------------------------------------

//----------------------- tri-keypad -------------------------------------------
#define _TRI_KEYPAD_PIN_	//GPIO pin
#define TRI_KEYPAD_RATING_FACTOR	1	//debounce check
#define TRI_KEYPAD_ROW_NUMBER	10	//input/output
#define TRI_CHECK_FLAG	0		//interrupt pin :pull high
#define _POLLING_INTERRUPT_ 0	//_POLLING_INTERRUPT_:0 -> polling mode, _POLLING_INTERRUPT_:1 ->interrupt mode
#if _POLLING_INTERRUPT_
#define _TRI_KEYPAD_INTERRUPT_
#else
#define _TRI_KEYPAD_POLLING_
#include <linux/init.h>
#include <linux/sched.h>
#endif
//-----------------------------------------------------------------------------

//------------------------- LED_74HC164 -----------------------------------------
#define _LED_74HC164_	//GPIO pin
#define LED_74HC164_RATING_FACTOR	2	//scanning frequency
#define LED_CLK_FACTOR		1	//clk period
#define LED_MAX_POSITIVE_CLK		10	//max positive clk
#define LED_NUMBER		16	//amount of led
#define LED_POLARITY	 1	//1->led common-anode, 0->led common-cathode
//#include <linux/init.h>
//#include <linux/sched.h>
//-------------------------------------------------------------------------------

#ifndef _BASE_GPIO_H_
#define _BASE_GPIO_H_

/********************* LCM data struct *************************************/

struct rtl8186_lcm_dev_s
{
	unsigned int reg_select;	//output
	unsigned int enable;		//output
	unsigned int read_write;	//output
	unsigned int DB7;		//input or output
	unsigned int DB6;		//input or output
	unsigned int DB5;		//input or output
	unsigned int DB4;		//input or output
	/*unsigned int DB3;		//input or output
	unsigned int DB2;		//input or output
	unsigned int DB1;		//input or output
	unsigned int DB0;		//input or output
	*/
};

typedef struct rtl8186_lcm_dev_s rtl8186_lcm_dev_t;

enum REGISTER_SELECT
{
	INSTRUCT_REGISTER = 0,
	DATA_REGISTER,	
};

enum BUSY_FLAG
{
	READ_FLAG = 0,
	NOT_READ_FLAG,
};

/*******************************************************************/

/*********************** I2C data struct ********************************************/
struct rtl8186_i2c_dev_s
{
	//unsigned int i2c_reset;		//output
	unsigned int sclk;		//output
	unsigned int sdio;		//input or output	
};	

typedef struct rtl8186_i2c_dev_s rtl8186_i2c_dev_t;
/*******************************************************************/

/************************ KEYPAD data struct *******************************************/
struct rtl8186_keypad_dev_s
{
	unsigned int row0;		//input
	unsigned int row1;		//input
	unsigned int row2;		//input
	unsigned int row3;		//input
	unsigned int row4;		//input
	unsigned int row5;		//input
	unsigned int column0;		//output
	unsigned int column1;		//output
	unsigned int column2;		//output
	unsigned int column3;		//output
	unsigned int column4;		//output
	unsigned int column5;		//output
	unsigned int column6;		//output
};	

typedef struct rtl8186_keypad_dev_s rtl8186_keypad_dev_t;
/*******************************************************************/

/************************ TRI_KEYPAD data struct *******************************************/
struct rtl8186_tri_keypad_dev_s
{
	unsigned int tri_row0;		//input/output
	unsigned int tri_row1;		//input/output
	unsigned int tri_row2;		//input/output
	unsigned int tri_row3;		//input/output
	unsigned int tri_row4;		//input/output
	unsigned int tri_row5;		//input/output
	unsigned int tri_row6;		//input/output
	unsigned int tri_row7;		//input/output
	unsigned int tri_row8;		//input/output
	unsigned int tri_row9;		//input/output
	
	unsigned int tri_interrupt;		//input	
};	

typedef struct rtl8186_tri_keypad_dev_s rtl8186_tri_keypad_dev_t;
/*******************************************************************/
	
/************************ LED_74HC164 data struct *************************/
struct rtl8186_led_74hc164_dev_s
{
	unsigned int a;		//output 
	unsigned int clk;		//output
};

typedef struct rtl8186_led_74hc164_dev_s rtl8186_led_74hc164_dev_t;
/**************************************************************************/

#ifndef SUCCESS
#define SUCCESS 	0
#endif
#ifndef FAILED
#define FAILED 		-1
#endif

/*********************  Function Prototype in base_gpio.c  ***********************/
void init_lcm_gpio(int lcm_gpio);
unsigned char read_lcm_IR(void);
void write_lcm_IR(unsigned char data,enum BUSY_FLAG bf_flag,unsigned char bit_interface);
unsigned char read_lcm_DR(void);
void write_lcm_DR(unsigned char data,enum BUSY_FLAG bf_flag,unsigned char bit_interface);

void init_i2c_gpio(void);
unsigned short int read_WM8510(unsigned char Reg);
void write_WM8510(unsigned char Reg,unsigned short int data);

void init_keypad_gpio(void);
void keyscan(void);

void init_tri_keypad_gpio(void);
void tri_keyscan(void);

void init_led_74hc164_gpio(void);
void led_shower(unsigned short led_on);

unsigned char iphone_hook_detect();
/******** GPIO define ********/

#define _GPIO_DEBUG_ 0		// 0 --> 5, more debug messages, when the num larger. 

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
#define GPIO_ID(port,pin) ((unsigned int)port<<16|(unsigned int)pin)

/* This is reversed macro. */
#define GPIO_PORT(id) (id>>16)
#define GPIO_PIN(id) (id&0xffff)


#ifdef _LCM_PIN_  
#if defined(CONFIG_RTK_VOIP_GPIO_IPP_100)
#if 0
#define GPIO_LCM "C"
#define PIN_RS		GPIO_ID(GPIO_PORT_C,6)	//output
#define PIN_ENABLE	GPIO_ID(GPIO_PORT_C,7)	//output
#define PIN_R_W_	GPIO_ID(GPIO_PORT_C,8)	//output
#define PIN_DB7		GPIO_ID(GPIO_PORT_C,9)	//output or input
#define PIN_DB6		GPIO_ID(GPIO_PORT_C,10)	//output or input
#define PIN_DB5		GPIO_ID(GPIO_PORT_C,11)	//output or input
#define PIN_DB4		GPIO_ID(GPIO_PORT_C,12)	//output or input
#else
#define GPIO_LCM "E"
#define PIN_RS		GPIO_ID(GPIO_PORT_E,6)	//output
#define PIN_ENABLE	GPIO_ID(GPIO_PORT_E,4)	//output
#define PIN_R_W_	GPIO_ID(GPIO_PORT_C,15)	//output
#define PIN_DB7		GPIO_ID(GPIO_PORT_E,0)	//output or input
#define PIN_DB6		GPIO_ID(GPIO_PORT_E,1)	//output or input
#define PIN_DB5		GPIO_ID(GPIO_PORT_E,2)	//output or input
#define PIN_DB4		GPIO_ID(GPIO_PORT_E,3)	//output or input
#endif
#elif defined(CONFIG_RTK_VOIP_GPIO_IPP_101)
#define GPIO_LCM "ACE"
#define PIN_RS		GPIO_ID(GPIO_PORT_E,6)	//output
#define PIN_ENABLE	GPIO_ID(GPIO_PORT_E,4)	//output
#define PIN_R_W_	GPIO_ID(GPIO_PORT_C,15)	//output
#define PIN_DB7		GPIO_ID(GPIO_PORT_E,0)	//output or input
#define PIN_DB6		GPIO_ID(GPIO_PORT_A,9)	//output or input
#define PIN_DB5		GPIO_ID(GPIO_PORT_A,5)	//output or input
#define PIN_DB4		GPIO_ID(GPIO_PORT_A,4)	//output or input
#endif
#endif

#ifdef _I2C_WM8510_
#if defined(CONFIG_RTK_VOIP_GPIO_IPP_100)
#define GPIO_I2C "D"
#define SCLK_PIN	GPIO_ID(GPIO_PORT_D,10)	//output
#define SDIO_PIN	GPIO_ID(GPIO_PORT_D,6)	//output or input
#elif defined(CONFIG_RTK_VOIP_GPIO_IPP_101)
#define GPIO_I2C "C"
#define SCLK_PIN	GPIO_ID(GPIO_PORT_C,13)	//output
#define SDIO_PIN	GPIO_ID(GPIO_PORT_C,14)	//output or input
#endif
#endif

#ifdef _KEYPAD_PIN_
#define GPIO_KEYPAD "C"
#define KEYPAD_ROW_0		GPIO_ID(GPIO_PORT_C,7)	//input
#define KEYPAD_ROW_1		GPIO_ID(GPIO_PORT_C,8)	//input
#define KEYPAD_ROW_2		GPIO_ID(GPIO_PORT_C,9)	//input
#define KEYPAD_ROW_3		GPIO_ID(GPIO_PORT_C,10)	//input
#define KEYPAD_ROW_4		GPIO_ID(GPIO_PORT_C,11)	//input
#define KEYPAD_ROW_5		GPIO_ID(GPIO_PORT_C,12)	//input
#define KEYPAD_COLUMN_0		GPIO_ID(GPIO_PORT_C,0)	//output
#define KEYPAD_COLUMN_1		GPIO_ID(GPIO_PORT_C,1)	//output
#define KEYPAD_COLUMN_2		GPIO_ID(GPIO_PORT_C,2)	//output
#define KEYPAD_COLUMN_3		GPIO_ID(GPIO_PORT_C,3)	//output
#define KEYPAD_COLUMN_4		GPIO_ID(GPIO_PORT_C,4)	//output
#define KEYPAD_COLUMN_5		GPIO_ID(GPIO_PORT_C,5)	//output
#define KEYPAD_COLUMN_6		GPIO_ID(GPIO_PORT_C,6)	//output
#endif

#ifdef _TRI_KEYPAD_PIN_
#if defined(CONFIG_RTK_VOIP_GPIO_IPP_100)
#if 0
#define GPIO_TRI_KEYPAD "C"
#define TRI_KEYPAD_ROW_0		GPIO_ID(GPIO_PORT_C,6)	//input/output
#define TRI_KEYPAD_ROW_1		GPIO_ID(GPIO_PORT_C,7)	//input/output
#define TRI_KEYPAD_ROW_2		GPIO_ID(GPIO_PORT_C,8)	//input/output
#define TRI_KEYPAD_ROW_3		GPIO_ID(GPIO_PORT_C,9)	//input/output
#define TRI_KEYPAD_ROW_4		GPIO_ID(GPIO_PORT_C,10)	//input/output
#define TRI_KEYPAD_ROW_5		GPIO_ID(GPIO_PORT_C,11)	//input/output
#define TRI_KEYPAD_ROW_6		GPIO_ID(GPIO_PORT_C,12)	//input/output
#define TRI_KEYPAD_ROW_7		GPIO_ID(GPIO_PORT_C,13)	//input/output
#define TRI_KEYPAD_ROW_8		GPIO_ID(GPIO_PORT_C,14)	//input/output
#define TRI_KEYPAD_ROW_9		GPIO_ID(GPIO_PORT_C,15)	//input/output
#define TRI_INTERRUPT			GPIO_ID(GPIO_PORT_C,5)	//input
#else
#define GPIO_TRI_KEYPAD "A"
#define TRI_KEYPAD_ROW_0		GPIO_ID(GPIO_PORT_C,13)//GPIO_ID(GPIO_PORT_E,0)	//input/output
#define TRI_KEYPAD_ROW_1		GPIO_ID(GPIO_PORT_A,8)	//input/output
#define TRI_KEYPAD_ROW_2		GPIO_ID(GPIO_PORT_A,7)	//input/output
#define TRI_KEYPAD_ROW_3		GPIO_ID(GPIO_PORT_A,6)	//input/output
#define TRI_KEYPAD_ROW_4		GPIO_ID(GPIO_PORT_C,14)//GPIO_ID(GPIO_PORT_E,1)	//input/output
#define TRI_KEYPAD_ROW_5		GPIO_ID(GPIO_PORT_C,12)//GPIO_ID(GPIO_PORT_E,2)	//input/output
#define TRI_KEYPAD_ROW_6		GPIO_ID(GPIO_PORT_A,3)	//input/output
#define TRI_KEYPAD_ROW_7		GPIO_ID(GPIO_PORT_A,2)	//input/output
#define TRI_KEYPAD_ROW_8		GPIO_ID(GPIO_PORT_A,1)	//input/output
#define TRI_KEYPAD_ROW_9		GPIO_ID(GPIO_PORT_A,0)	//input/output
#define TRI_INTERRUPT			GPIO_ID(GPIO_PORT_D,0)	//input
#endif
#elif defined(CONFIG_RTK_VOIP_GPIO_IPP_101)
#define GPIO_TRI_KEYPAD "C"
#define TRI_KEYPAD_ROW_0		GPIO_ID(GPIO_PORT_C,0)	//input/output
#define TRI_KEYPAD_ROW_1		GPIO_ID(GPIO_PORT_C,1)	//input/output
#define TRI_KEYPAD_ROW_2		GPIO_ID(GPIO_PORT_C,2)	//input/output
#define TRI_KEYPAD_ROW_3		GPIO_ID(GPIO_PORT_C,3)	//input/output
#define TRI_KEYPAD_ROW_4		GPIO_ID(GPIO_PORT_C,4)	//input/output
#define TRI_KEYPAD_ROW_5		GPIO_ID(GPIO_PORT_C,5)	//input/output
#define TRI_KEYPAD_ROW_6		GPIO_ID(GPIO_PORT_C,6)	//input/output
#define TRI_KEYPAD_ROW_7		GPIO_ID(GPIO_PORT_C,7)	//input/output
#define TRI_KEYPAD_ROW_8		GPIO_ID(GPIO_PORT_C,8)	//input/output
#define TRI_KEYPAD_ROW_9		GPIO_ID(GPIO_PORT_C,9)	//input/output
#define TRI_INTERRUPT			GPIO_ID(GPIO_PORT_C,10)	//input
#endif
#endif

#ifdef _LED_74HC164_
#if defined(CONFIG_RTK_VOIP_GPIO_IPP_100)
#define GPIO_LED_74HC164 "D"
#define LED_74HC164_A			GPIO_ID(GPIO_PORT_D,8)	//output
#define LED_74HC164_CLK			GPIO_ID(GPIO_PORT_D,7)	//output
#elif defined(CONFIG_RTK_VOIP_GPIO_IPP_101)
#define GPIO_LED_74HC164 "A"
#define LED_74HC164_A			GPIO_ID(GPIO_PORT_A,3)	//output
#define LED_74HC164_CLK			GPIO_ID(GPIO_PORT_A,2)	//output
#endif
#endif

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

enum GPIO_FUNC	
{
	GPIO_FUNC_DIRECTION,
	GPIO_FUNC_DATA,
	GPIO_FUNC_INTERRUPT_STATUS,
	GPIO_FUNC_INTERRUPT_ENABLE,
	GPIO_FUNC_MAX,
};

/* Defined the gpio function ("Direction", "Data", "ISR", "IMR") of each GPIO port of RTL8186. */
/* Also define the read/write start bit of each gpio function.*/

//******************************************* Direction

static unsigned int regGpioDirectionRead[] =
{
	GPABDIR, 	/* Port A */
	GPABDIR, 	/* Port B */
	GPCDDIR, 	/* Port C */
	GPCDDIR, 	/* Port D */
	GPEFDIR,  	/* Port E */
	GPEFDIR,		/* Port F */
	GPGDIR,		/* Port G */
};

static unsigned int bitStartGpioDirectionRead[] =
{
	0, 			/* Port A */
	16, 			/* Port B */
	0,  			/* Port C */
	16, 			/* Port D */
	0, 			/* Port E */
	0, 			/* Port F */	// according to modified Spec. 
	0, 			/* Port G */
};

static unsigned int regGpioDirectionWrite[] =
{
	GPABDIR, 	/* Port A */
	GPABDIR, 	/* Port B */
	GPCDDIR, 	/* Port C */
	GPCDDIR, 	/* Port D */
	GPEFDIR,  	/* Port E */
	GPEFDIR,		/* Port F */
	GPGDIR,		/* Port G */
};

static unsigned int bitStartGpioDirectionWrite[] =
{
	0, 			/* Port A */
	0, 			/* Port B */
	0,  			/* Port C */
	0, 			/* Port D */
	0, 			/* Port E */
	0, 			/* Port F */
	0, 			/* Port G */
};

//******************************************* Data

static unsigned int regGpioDataRead[] =
{
	GPABDATA, 	/* Port A */
	GPABDATA, 	/* Port B */
	GPCDDATA, 	/* Port C */
	GPCDDATA,  	/* Port D */
	GPEFDATA,  	/* Port E */
	GPEFDATA,	/* Port F */
	GPGDATA,	/* Port G */
};

static unsigned int bitStartGpioDataRead[] =
{
	0, 			/* Port A */
	16, 			/* Port B */
	0,  			/* Port C */
	16, 			/* Port D */
	0, 			/* Port E */
	16, 			/* Port F */
	0, 			/* Port G */
};

static unsigned int regGpioDataWrite[] =
{
	GPABDATA, 	/* Port A */
	GPABDATA, 	/* Port B */
	GPCDDATA, 	/* Port C */
	GPCDDATA,  	/* Port D */
	GPEFDATA,  	/* Port E */
	GPEFDATA,	/* Port F */
	GPGDATA,	/* Port G */
};

static unsigned int bitStartGpioDataWrite[] =
{
	0, 			/* Port A */
	0, 			/* Port B */
	0,  			/* Port C */
	0, 			/* Port D */
	0, 			/* Port E */
	0, 			/* Port F */
	0, 			/* Port G */
};

//******************************************* ISR

static unsigned int regGpioInterruptStatusRead[] =
{
	GPABISR, 	/* Port A */
	GPABISR, 	/* Port B */
	GPCDISR, 	/* Port C */
	GPCDISR,  	/* Port D */
	GPEFISR,  	/* Port E */
	GPEFISR,		/* Port F */
	GPGISR,		/* Port G */
};

static unsigned int bitStartGpioInterruptStatusRead[] =
{
	0, 			/* Port A */
	16, 			/* Port B */
	0,  			/* Port C */
	16, 			/* Port D */
	0, 			/* Port E */
	16, 			/* Port F */
	0, 			/* Port G */
};

static unsigned int regGpioInterruptStatusWrite[] =
{
	GPABISR, 	/* Port A */
	GPABISR, 	/* Port B */
	GPCDISR, 	/* Port C */
	GPCDISR,  	/* Port D */
	GPEFISR,  	/* Port E */
	GPEFISR,		/* Port F */
	GPGISR,		/* Port G */
};

static unsigned int bitStartGpioInterruptStatusWrite[] =
{
	0, 			/* Port A */
	0, 			/* Port B */
	0,  			/* Port C */
	0, 			/* Port D */
	0, 			/* Port E */
	0, 			/* Port F */
	0, 			/* Port G */
};

//******************************************* IMR

static unsigned int regGpioInterruptEnableRead[] =
{
	GPABIMR,	/* Port A */
	GPABIMR,	/* Port B */
	GPCDIMR, 	/* Port C */
	GPCDIMR,	/* Port D */
	GPEFIMR,		/* Port E */
	GPEFIMR,		/* Port F */
	GPGIMR,		/* Port G */
};

static unsigned int bitStartGpioInterruptEnableRead[] =
{
	0, 			/* Port A */
	16,  			/* Port B */
	0,			/* Port C */
	16, 			/* Port D */
	0,  			/* Port E */
	16, 			/* Port F */
	0,  			/* Port G */
};

static unsigned int regGpioInterruptEnableWrite[] =
{
	GPABIMR,	/* Port A */
	GPABIMR,	/* Port B */
	GPCDIMR, 	/* Port C */
	GPCDIMR,	/* Port D */
	GPEFIMR,		/* Port E */
	GPEFIMR,		/* Port F */
	GPGIMR,		/* Port G */
};

static unsigned int bitStartGpioInterruptEnableWrite[] =
{
	0, 			/* Port A */
	0,  			/* Port B */
	0,			/* Port C */
	0, 			/* Port D */
	0,  			/* Port E */
	0, 			/* Port F */
	0,  			/* Port G */
};


#endif	//_BASE_GPIO_H_
