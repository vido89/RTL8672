#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include "led-generic.h"
#include "pushbutton.h"
#include "gpio.h"
#include <asm/mach-realtek/rtl8672/platform.h>

#define LOW_ACTIVE	1
#if LOW_ACTIVE

#define GPIO_SET(w, op)  do { \
	gpioConfig(w, GPIO_FUNC_OUTPUT); \
	if (LED_ON==op) gpioClear(w); \
	else gpioSet(w); \
} while (0);

#define GPIO_READ(w) (!gpioRead(w))
		
#else

#define GPIO_SET(w, op)  do { \
	gpioConfig(w, GPIO_FUNC_OUTPUT); \
	if (LED_ON==op) gpioSet(w); \
	else gpioClear(w); \
} while (0);

#define GPIO_READ(w) (gpioRead(w))

#endif

static void board_03_handle_set(int which, int op) {
	
	//printk("%s: led %d op %d\n", __FUNCTION__, which, op);
	switch (which) {
	case LED_POWER_GREEN:
		    //printk("%s: RTL8671B power led green op %d\n", __FUNCTION__, op);
		    GPIO_SET(GPIO_B_6, op);
		break;
	case LED_POWER_RED:
		    //printk("%s: RTL8671B power led red op %d\n", __FUNCTION__, op);
		    GPIO_SET(GPIO_D_2, op);
		break;
	case LED_DSL:		
		//printk("%s:led %s op %d\n", __FUNCTION__, LED_DSL, op);			
		GPIO_SET(GPIO_A_5, op);
		break;
	case LED_INTERNET_GREEN:
		//printk("%s:led %s op %d\n", __FUNCTION__, LED_INTERNET_GREEN, op);			
		GPIO_SET(GPIO_B_2, op);
		break;
	case LED_INTERNET_RED:
		//printk("%s:led %s op %d\n", __FUNCTION__, LED_INTERNET_RED, op);			
		GPIO_SET(GPIO_B_5, op);
		break;
		
	case LED_PPP_GREEN:
		//printk("%s:led %s op %d\n", __FUNCTION__, LED_PPP_GREEN, op);				
		GPIO_SET(GPIO_B_2, op);
		break;
	case LED_PPP_RED:
		//printk("%s:led %s op %d\n", __FUNCTION__, LED_PPP_RED, op);					
		GPIO_SET(GPIO_B_5, op);
		break;		
#ifdef CONFIG_NET_RADIO
	case LED_WPS_GREEN:
		//printk("%s:led %s op %d\n", __FUNCTION__, LED_WPS_GREEN, op);					
		GPIO_SET(GPIO_D_3, op);		
		break;
	case LED_WPS_RED:
		//printk("%s:led %s op %d\n", __FUNCTION__, LED_WPS_RED, op);				
		GPIO_SET(GPIO_D_4, op);		
		break;
	case LED_WPS_YELLOW:
		//printk("%s:led %s op %d\n", __FUNCTION__, LED_WPS_YELLOW, op);					
		GPIO_SET(GPIO_D_6, op);		
		break;
#endif				
	default:
		led_handle_set(which, op);
	}
}

static void board_03_handle_init(void) {
	board_03_handle_set(LED_POWER_GREEN, LED_OFF);
	board_03_handle_set(LED_POWER_RED, LED_ON);
	board_03_handle_set(LED_DSL, LED_OFF);
	board_03_handle_set(LED_INTERNET_GREEN, LED_OFF);
	board_03_handle_set(LED_INTERNET_RED, LED_OFF);
#ifdef CONFIG_NET_RADIO
	board_03_handle_set(LED_WPS_GREEN, LED_OFF);
	board_03_handle_set(LED_WPS_RED, LED_OFF);
	board_03_handle_set(LED_WPS_YELLOW, LED_OFF);
#endif
};

static struct led_operations board_03_operation = {
	.name = "board_03",
	.handle_init = board_03_handle_init,
	.handle_set = board_03_handle_set,
};


static void board_03_pb_init(void) {
};

static int board_03_pb_is_pushed(int which) {
	switch(which) {
		
	case PB_RESET:
		//printk("PB: %d\n", GPIO_READ(GPIO_A_6));
		return GPIO_READ(GPIO_A_6);
#ifdef CONFIG_NET_RADIO
	case PB_WIFISW:
		if( IS_6028A ) {
			return GPIO_READ(GPIO_D_2);
		}
		else if( IS_6028B || IS_6085 ) {
			return GPIO_READ(GPIO_A_7);
		}
	case PB_WPS:
		if( IS_6028A ) {
			return GPIO_READ(GPIO_A_7);
		}
		else if( IS_6028B || IS_6085 ) {
			return GPIO_READ(GPIO_B_7);
		}
#endif
	}
	return 0;
}

static struct pushbutton_operations board_03_pb_op = {
	.handle_init = board_03_pb_init,
	.handle_is_pushed = board_03_pb_is_pushed,
};

static int __init board_03_led_init(void) {
	if( IS_6028B || IS_6085 ) {
		REG32(0xb8003304) |= 0x001c0000; //enable GPIO D2, D3, D4, D6
	}
	led_register_operations(&board_03_operation);
	pb_register_operations(&board_03_pb_op);
	return 0;
}

static void __exit board_03_led_exit(void) {
}


module_init(board_03_led_init);
module_exit(board_03_led_exit);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("GPIO driver for Reload default");



