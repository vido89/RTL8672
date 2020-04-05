#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <asm/processor.h>
#include <linux/sched.h>
#include <asm/mach-realtek/rtl8672/platform.h>
#include <linux/init.h>
#include "led-generic.h"
#include "gpio.h"
static struct led_struct led_flash_dsl;
static struct led_struct led_internet_act;

void tr068_internet_traffic(void) {
	led_act_touch(&led_internet_act);
}


void tr068_internet_led_start(void) {
	//printk("%s\n", __FUNCTION__);
	led_act_start(&led_internet_act, LED_INTERNET_GREEN, HZ / 20, 10);
	//led_flash_start(&led_internet_act, tr069_internet_flash_func);
}


void tr068_internet_led_stop(void) {
	//printk("%s\n", __FUNCTION__);
	led_act_stop(&led_internet_act);
	//led_internet_act._counter = 0;
	//del_timer(&(led_internet_act.timer));
}

char adsl_showtime = 0;

void ADSL_state(unsigned char LEDstate)
{
	static unsigned char LastStatus = 255;
	static char prev_adsl_showtime = 0;
	if(LastStatus == LEDstate)
		return;
		
	LastStatus = LEDstate;
	adsl_showtime = 0;
	switch((unsigned char)LEDstate){
		case C_AMSW_IDLE:
		case C_AMSW_L3:
		case C_AMSW_ACTIVATING:
		case C_AMSW_END_OF_LD:
			led_off(LED_INTERNET_GREEN);
			led_off(LED_INTERNET_RED);
			led_flash_start(&led_flash_dsl, LED_DSL, HZ / 2);
			break;
					
		case C_AMSW_INITIALIZING:			
			led_flash_start(&led_flash_dsl, LED_DSL, HZ / 4);		
			break;
			
		case C_AMSW_SHOWTIME_L0:
			led_flash_stop(&led_flash_dsl); 
			led_on(LED_DSL);
			adsl_showtime = 1;
			break;
	}	
	if (prev_adsl_showtime != adsl_showtime) {
		extern void internet_led_check(void);
		prev_adsl_showtime = adsl_showtime;
		//printk("%s %d\n", __func__, __LINE__);
		internet_led_check();
		#if 0
		if (adsl_showtime)
			blocking_notifier_call_chain(&dsl_chain,
						NETDEV_UP, 0);
		else
			blocking_notifier_call_chain(&dsl_chain,
						NETDEV_DOWN, 0);
		#endif
	}
}	



static int __init led_tr068_init(void) {
	
	//led_flash_dsl.led = LED_DSL;			
	//led_flash_dsl.state = 0; // off
	
	led_internet_act.led = LED_INTERNET_GREEN;
	led_internet_act.act_state = 1; // Default is ON
	led_internet_act.cycle = HZ /20;
	led_internet_act.backlog = 10;

	
	return 0;
}

static void __exit led_tr068_exit(void) {
}


module_init(led_tr068_init);
module_exit(led_tr068_exit);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("GPIO driver for Reload default");

