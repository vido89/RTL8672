/*
 * Copyright c                Realtek Semiconductor Corporation, 2002
 * All rights reserved.                                                    
 * 
 * Abstract:
 *
 *   GPIO driver header file for export include.
 *
 */

#ifndef _GPIO_H_
#define _GPIO_H_


enum GPIO_DEF {
	GPIO_A_0 = 0, GPIO_A_1, GPIO_A_2, GPIO_A_3, GPIO_A_4, GPIO_A_5, GPIO_A_6, GPIO_A_7, 
	GPIO_B_0 = 8, GPIO_B_1, GPIO_B_2, GPIO_B_3, GPIO_B_4, GPIO_B_5, GPIO_B_6, GPIO_B_7, 
	GPIO_C_0 = 16, GPIO_C_1, GPIO_C_2, GPIO_C_3, GPIO_C_4, GPIO_C_5, GPIO_C_6, GPIO_C_7, 
	GPIO_D_0 = 24, GPIO_D_1, GPIO_D_2, GPIO_D_3, GPIO_D_4, GPIO_D_5, GPIO_D_6, GPIO_D_7
};

/*port A pin*/
#define GPIO_PA7	0x00000080
#define GPIO_PA6	0x00000040
#define GPIO_PA5	0x00000020
#define GPIO_PA4	0x00000010
#define GPIO_PA3	0x00000008
#define GPIO_PA2	0x00000004
#define GPIO_PA1	0x00000002
#define GPIO_PA0	0x00000001
/*port B pin*/
#define GPIO_PB7	0x00008000
#define GPIO_PB6	0x00004000
#define GPIO_PB5	0x00002000
#define GPIO_PB4	0x00001000
#define GPIO_PB3	0x00000800
#define GPIO_PB2	0x00000400
#define GPIO_PB1	0x00000200
#define GPIO_PB0	0x00000100
/*port C pin*/
#define GPIO_PC7	0x00800000
#define GPIO_PC6	0x00400000
#define GPIO_PC5	0x00200000
#define GPIO_PC4	0x00100000
#define GPIO_PC3	0x00080000
#define GPIO_PC2	0x00040000
#define GPIO_PC1	0x00020000
#define GPIO_PC0	0x00010000
/*port D pin*/
#define GPIO_PD7	0x80000000
#define GPIO_PD6	0x40000000
#define GPIO_PD5	0x20000000
#define GPIO_PD4	0x10000000
#define GPIO_PD3	0x08000000
#define GPIO_PD2	0x04000000
#define GPIO_PD1	0x02000000
#define GPIO_PD0	0x01000000

#undef	GPIO_RESET_BUTTON
// #define GPIO_RESET_BUTTON       1
// #define	GPIO_LED_ADSL_HS	1
#undef	GPIO_LED_ADSL_HS
//#define	GPIO_LED_ADSL_ACT	1
#undef	GPIO_LED_ADSL_ACT
//#define	GPIO_LED_ADSL_ALARM	1
#undef	GPIO_LED_ADSL_ALARM
#undef	GPIO_LED_PPP		

/*real use of GPIO pin
Pin #   0   1 ...   7   8 ...  15
GPIO  PA0 PA1 ... PA7 PB0 ... PB7
*/
#ifdef CONFIG_EXT_SWITCH
// GPIO to simulate MDC/MDIO
// Note: the MDC and MDIO must use the same GPIO port (A or B) in current
// 	software design
#define GPIO_MDC	12	// PB4, 4+8
#define GPIO_MDIO	11	// PB3, 3+8
#endif

#if defined(CONFIG_RTL8670)
#define ADSL_LED	0

// Added by Mason Yu for PPP LED
#elif defined(CONFIG_STD_LED)
/* bitmap of GPIO pin as gpio function
Bit #   0   1 ...   7   8 ...  15
GPIO  PB0 PB1 ... PB7 PA0 ... PA7
*/
#define GPIO_FUNC	0x7820

#define RESET_BUTTON    14
#define ADSL_LED	13
#define ADSL_ACT_LED	11
#define ALARM_LED	12
#define PPP_LED		5
#define PCI_RST		10
#elif  defined(CONFIG_GPIO_LED_CHT_E8B)
#define PPP_RED
#define PPP_GREEN
#define LED_DSL_LINK 
#define LED_WPS_G
#define LED_WPS_R
#define LED_WPS_Y
#define RESET_BUTTON    10

#else // 8672
#define GPIO_FUNC	0xe000


#define ADSL_LED	15
#define ADSL_ACT_LED	14
#define ALARM_LED	13
#endif


//WPS MODE
#define GENERIC_MODEL_WPS	0
// Andrew. This define will use GPIO B3(in) for PBC button, and B7(out) for LED
#define BIG_MODEL_WPS 		1
#define ALPHA_MODEL_WPS		2
#define E8B_MODEL_WPS		3
#define BOARD_TYPE 		E8B_MODEL_WPS

/*port function definition*/
#define GPIO_FUNC_INPUT 	0x0001  /*data input*/
#define GPIO_FUNC_OUTPUT 	0x0002	/*data output*/
#define GPIO_FUNC_IRQ_FALL 	0x0003	/*falling edge IRQ*/
#define GPIO_FUNC_IRQ_RISE 	0x0004	/*rising edge IRQ*/
#define GPIO_FUNC_IRQ_LEVEL 	0x0005	/*level trigger IRQ*/

extern void gpioConfig (int gpio_num, int gpio_func);
extern void gpioSet(int gpio_num);
extern void gpioClear(int gpio_num);
extern int gpioRead(int gpio_num);
extern void gpioHandshaking(int flag);
extern void gpioACT(int flag);
extern void gpioAlarm(int flag);
extern void gpio_LED_PPP(int flag);

/* ADSL status */

#endif  /* _GPIO_H_ */
