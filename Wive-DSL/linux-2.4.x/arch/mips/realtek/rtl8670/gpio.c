/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2003  
* All rights reserved.
* 
* Abstract: GPIO driver source code.
*
* ---------------------------------------------------------------
*/

#include <linux/config.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/mman.h>
#include <linux/ioctl.h>
#include <linux/fd.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/byteorder.h>
#include <asm/mach-realtek/rtl8672/platform.h>
#include "gpio.h"

//#define CONFIG_E8BGPIO 1
extern int g_internet_up;
extern int g_ppp_up;
void  gpio_led_set(void);

int led0enable=1; //for rtl8185 driver
static int GPIOdataReg=0;

/*
Check BD800000 to identify which GPIO pins can be used
*/
unsigned int get_GPIOMask(void)
{
	unsigned int portMask=0xFFFFFFFF;

#if  !defined(CONFIG_EXT_SWITCH)&& !defined(CONFIG_GPIO_LED_CHT_E8B)
	//portMask &= ~(GPIO_PB3|GPIO_PB4|GPIO_PB5|GPIO_PB6|GPIO_PB7);  //disable B3-B7
#endif

	return portMask;
}

/*
Config one GPIO pin. Release 1 only support output function
number and PIN location map:
Pin	num
PB7	15
:	:
PB0	8
PA7	7
:	:
PA0	0
*/
void gpioConfig (int gpio_num, int gpio_func)
{
  unsigned int useablePin;
  unsigned int mask;
  
	//printk( "<<<<<<<<<enter gpioConfig(gpio_num:%d, gpio_func:%d)\n", gpio_num, gpio_func );
	
	if ((gpio_num>31)||(gpio_num<0)) return;
	 
	useablePin = get_GPIOMask();
	mask=1<<gpio_num;
	if ((useablePin&mask)==0) {  //GPIO pins is shared by other modules
		printk("GPIO config Error! PIN %d is used by a hardware module\n",gpio_num);
		return;
	};

    	//mask=1<<gpio_num;    
	if (GPIO_FUNC_INPUT == gpio_func)
		REG32(GPIO_PABCD_DIR) = REG32(GPIO_PABCD_DIR)&(~mask);
	else
        	REG32(GPIO_PABCD_DIR) = REG32(GPIO_PABCD_DIR)|mask;
}

/*set GPIO pins on*/
void gpioSet(int gpio_num)
{
  unsigned int portMask=0;
	unsigned int pins;
 
	//printk( "<<<<<<<<<enter gpioSet( gpio_num:%d )\n", gpio_num );  
	if ((gpio_num>31)||(gpio_num<0)) return;
    
	pins = 1<<gpio_num;
	portMask = get_GPIOMask();
	pins &= portMask;  //mask out disable pins
	if (pins==0) return;  //no pins to set    
    
	GPIOdataReg |= pins;  //set pins
	//write out
	REG32(GPIO_PABCD_DAT) = GPIOdataReg;
}

/*set GPIO pins off*/
void gpioClear(int gpio_num)
{
  unsigned int portMask=0;
	unsigned int pins;

//	printk( "<<<<<<<<<enter gpioClear( gpio_num:%d )\n", gpio_num );      
	if ((gpio_num>31)||(gpio_num<0)) return;
    
	pins = 1<<gpio_num;
	portMask = get_GPIOMask();
	pins &= portMask;  //mask out disable pins
	if (pins==0) return;  //no pins to reset    
        
	GPIOdataReg &= ~pins;  //reset pins
	//write out
	REG32(GPIO_PABCD_DAT) = GPIOdataReg;
}


int gpioRead(int gpio_num) 
{
	unsigned int val;
	if ((gpio_num>31)||(gpio_num<0)) 
		return 0;

	val = REG32(GPIO_PABCD_DAT);
	if (val & (1 << gpio_num))
		return 1;
	else
		return 0;		
}


// Added by Mason Yu for New map LED
void gpioHandshaking(int flag)
{
	#ifdef GPIO_LED_ADSL_HS
	gpioConfig(ADSL_LED, GPIO_FUNC_OUTPUT);		

	// on
	if ( flag == 1) {
		gpioClear(ADSL_LED);
		//gpioSet(ADSL_LED);
		return;
	}	
	
	// off
	if ( flag == 0) {
		gpioSet(ADSL_LED); 
		//gpioClear(ADSL_LED);
		return;
	}
	#elif   defined(CONFIG_GPIO_LED_CHT_E8B)
	      gpio_set_dsl_link(flag);
	#else
	return;
	#endif

}

// Added by Mason Yu for New map LED
void gpioACT(int flag)
{
	#ifdef GPIO_LED_ADSL_ACT
	gpioConfig(ADSL_ACT_LED, GPIO_FUNC_OUTPUT);
	
	// on
	if ( flag == 1) {
		gpioClear(ADSL_ACT_LED);
		return;
	}	
	
	// off
	if ( flag == 0) {
		gpioSet(ADSL_ACT_LED); 
		return;
	}
	#else
	return;
	#endif

}

// Added by Mason Yu for New map LED
void gpioAlarm(int flag)
{
	#ifdef GPIO_LED_ADSL_ALARM
	gpioConfig(ALARM_LED, GPIO_FUNC_OUTPUT);
	
	// on
	if ( flag == 1) {
		gpioClear(ALARM_LED);
		return;
	}	
	
	// off
	if ( flag == 0) {
		gpioSet(ALARM_LED); 
		return;
	}
	#else
	return;
	#endif

}

#if defined(CONFIG_STD_LED)
void gpio_LED_PPP(int flag)
{
	#ifdef GPIO_LED_PPP
	gpioConfig(PPP_LED, GPIO_FUNC_OUTPUT);
	
	// on
	if ( flag == 1) {
		gpioClear(PPP_LED);
		return;
	}	
	
	// off
	if ( flag == 0) {
		gpioSet(PPP_LED);
		return;
	}
	#else
	return;
	#endif
}
#elif defined(CONFIG_GPIO_LED_CHT_E8B)
void gpio_LED_PPP(int flag)
{
   gpio_set_ppp_g(flag);

}


#endif
void gpio_internet(int flag)
{
//	gpioAlarm(flag);

	#ifdef GPIO_LED_TR068_INTERNET
	gpioConfig(INTERNET_LED, GPIO_FUNC_OUTPUT);
	
	// on
	if ( flag == 1) {
		gpioClear(INTERNET_LED);
		return;
	}	
	
	// off
	if ( flag == 0) {
		gpioSet(INTERNET_LED);
		return;
	}
	#elif defined(CONFIG_GPIO_LED_CHT_E8B)
                  
          gpio_set_ppp_g(flag);
	
	#else
	return;
	#endif
}

int __init gpio_init(void)
{
#ifndef CONFIG_JTAG_USABLE
	unsigned int sicr;

	sicr = REG32(MISC_PINMUX);	
	if( sicr&0x80000000 )
	{
		printk( "<<<<<<<disable GPIO JTAG function.\n" );
		REG32(MISC_PINMUX) = sicr&0x7fffffff;
	}
#endif	
	//init GPIO_PABCD_CNR=0
	REG32(GPIO_PABCD_CNR) = 0x0;
	//init GPIO_PABCD_PTYPE=0
	REG32(GPIO_PABCD_PTYPE) = 0x0;


#ifdef CONFIG_GPIO_LED_CHT_E8B	
         E8GPIO_init();	 
	  gpioHandshaking(0);
         gpio_internet(0);
#endif

	return 0;
  
}


#ifdef CONFIG_GPIO_LED_CHT_E8B
// In the charge of LED blinking control timer
struct timer_list FlashLedTimer;

void FlashLED(unsigned long status)
{
	static int CtrlFlag = 0;

	{
	switch((unsigned char)status){
		case C_AMSW_IDLE:
			FlashLedTimer.expires=jiffies+HZ/2;
			gpioHandshaking(CtrlFlag);
			gpio_internet(0);

			break;
		case C_AMSW_L3:			
			gpioHandshaking(1);
			gpio_internet(0);
		
			break;
		case C_AMSW_ACTIVATING:
			FlashLedTimer.expires=jiffies+HZ/2;
		
			gpioHandshaking(CtrlFlag);
			gpio_internet(0);
			break;
		case C_AMSW_INITIALIZING:			
			FlashLedTimer.expires=jiffies+HZ/4;
			gpioHandshaking(CtrlFlag);
			gpio_internet(0);
		
			break;
		case C_AMSW_SHOWTIME_L0:	
			FlashLedTimer.expires=jiffies+HZ/4;

		
			// Added by Mason Yu for PPP ACT LED
//			printk("\ng_internet_up=%d\n",g_internet_up);

			if(IsTrafficOnAtm()) {		
				if(g_internet_up == 1)
					gpio_internet(CtrlFlag);
				else
					gpio_internet(0);
				#ifdef GPIO_LED_PPP_BLK
				if ( g_ppp_up >= 1)				
					gpio_LED_PPP(!CtrlFlag);
				#endif
			}			
			else {	
				if(g_internet_up == 1)
				{
					CtrlFlag = 1;
					gpio_internet(CtrlFlag);
				
				}
				else
					gpio_internet(0);
				#ifdef GPIO_LED_PPP_BLK
				if ( g_ppp_up >= 1)				
					gpio_LED_PPP(1);
				#endif
			}		
			 gpioHandshaking(1);
                      break;
	   
		case C_AMSW_END_OF_LD:
			
			gpioHandshaking(0);	
			gpio_internet(0);
			break;
	}

	if(CtrlFlag)
		CtrlFlag = 0;
	else
		CtrlFlag = 1;	

	FlashLedTimer.data = status;	
	FlashLedTimer.function=(void (*)(unsigned long))FlashLED;
	mod_timer(&FlashLedTimer, FlashLedTimer.expires);	

	}
	
}	

/*
 * ADSL_state : Called by DSP driver while ADSL status changing
 */

void ADSL_state(unsigned char LEDstate)
{
	static unsigned char LastStatus = 255;

	if(LastStatus != LEDstate){
		LastStatus = LEDstate;
		FlashLED((unsigned long)LEDstate);
	}
}	
#endif

//if 128 PIN board 
//PA5 PA6 PA7 PB0 PB1 PB2 PB3 PB4 PB5 PB6 PB7
//0        1    2     3     4     5    6      7      8    9    10
//74164 used

//if 208 PIN board
#define LED_CLK    14    //GPIO_B_6
#define LED_DATA  15    //GPIO_B_7


#define POWER_GREEN    0
#define POWER_RED      1
#define PPP_GREEN      2
#define PPP_RED        3               
#define LED_DSL_LINK   4 

static int   power_red_on;
static int   power_green_on;
static int   ppp_red_on;
static int   ppp_green_on;
static int   led_dsl_link_on;
static int   led_wps_g_on=0;
static int   led_wps_r_on=0;
static int   led_wps_y_on=0;

void gpio_set_power_g(int flag)
{
    if(flag) //on  
       power_green_on = 1;
    else 
       power_green_on = 0;		
    gpio_led_set();
}

void gpio_set_power_r(int flag)
{
    if(flag)
       power_red_on = 1;
    else
       power_red_on = 0;		 
    gpio_led_set();
}

void gpio_set_ppp_g(int flag)
{                
     if(flag)
	 ppp_green_on = 1;
     else
	 ppp_green_on = 0;
     gpio_led_set();	 
     	 
}

void gpio_set_ppp_r(int flag)
{
     if(flag)
	 ppp_red_on = 1;
     else
	 ppp_red_on = 0;
     gpio_led_set();	 
     	 
}

void gpio_set_dsl_link(int flag)
{
     if(flag)
	led_dsl_link_on  = 1;
     else
	 led_dsl_link_on = 0;
     gpio_led_set();	 
     	 
}
void  gpio_led_set(void) {
	gpioConfig(LED_DATA, GPIO_FUNC_OUTPUT);
	gpioConfig(LED_CLK, GPIO_FUNC_OUTPUT);

   
       if(led_wps_y_on){
	 	gpioClear(LED_DATA);	
       }
	else {
		gpioSet(LED_DATA);
	}
	
	gpioClear(LED_CLK);  
	gpioSet(LED_CLK); 	   //clock 1
	if(led_wps_r_on)
	{
          gpioClear(LED_DATA);
	}
	else 
	{
		gpioSet(LED_DATA);
	}
	gpioClear(LED_CLK)	;
	gpioSet(LED_CLK);     //clock  2  

	if(led_wps_g_on){
            gpioClear(LED_DATA);
	}	
	else {
         gpioSet(LED_DATA);       	
	}

	gpioClear(LED_CLK)	;  
	gpioSet(LED_CLK);     //clock  3
	if(ppp_red_on)
	{
             gpioClear(LED_DATA);

	}
	else
	{
             gpioSet(LED_DATA);

	}
	gpioClear(LED_CLK)	;
	gpioSet(LED_CLK);     //clock  4
	if(ppp_green_on){
         gpioClear(LED_DATA);   
	}
	else
	{
            gpioSet(LED_DATA);

	}
	gpioClear(LED_CLK)	;  
	gpioSet(LED_CLK);     //clock   5 
	if(led_dsl_link_on)
	{
           gpioClear(LED_DATA);

	}
	else 
	{
	  gpioSet(LED_DATA);
	}
	  gpioClear(LED_CLK)	;
	  gpioSet(LED_CLK);     //clock   6
	if(power_red_on)
	{
	  gpioClear(LED_DATA);
	}
	else
	{
	    
	   gpioSet(LED_DATA);
	}
	gpioClear(LED_CLK)	;
	gpioSet(LED_CLK);     //clock    7
	if(power_green_on)
	{
              gpioClear(LED_DATA);
	
	}
	else
	{
             gpioSet(LED_DATA);
	}
	gpioClear(LED_CLK)	;
	gpioSet(LED_CLK);     //clock     8 
     


}
#ifdef CONFIG_GPIO_LED_CHT_E8B 
void E8GPIO_init()
{

	init_timer(&FlashLedTimer);	

}
#endif

static void __exit gpio_exit (void)
{
}

module_init(gpio_init);
module_exit(gpio_exit);
