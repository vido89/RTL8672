#include <linux/timer.h>
#include <linux/sched.h>
#include "led.h"

/* =================== For V210 EV Board LED Control ======================== */

static void fxo_led_blinking(unsigned long data)
{
	static unsigned int gpio_on_off = 0;	//0: off, 1: on.
	
	if (gpio_on_off == 1)
	{
#ifdef _V210_EV_BOARD_	
		GPAB_DATA &= 0xfffffffb;
#endif
#ifdef _V210_Ed_
		GPAB_DATA &= 0xfffffff7;
#endif		
	}
	else
	{
#ifdef _V210_EV_BOARD_	
		GPAB_DATA |= 0x04;	
#endif
#ifdef _V210_Ed_
		GPAB_DATA |= 0x08;	
#endif
	}
	gpio_on_off ^= 1;
        fxo_led_timer.expires = jiffies + led_blinking_frequency;
        add_timer(&fxo_led_timer);
	return;
}
static void fxo_led_on(unsigned long data)
{
#ifdef _V210_EV_BOARD_	
	GPAB_DATA &= 0xfffffffb;
#endif	
#ifdef _V210_Ed_
	GPAB_DATA &= 0xfffffff7;
#endif	
        fxo_led_timer.expires = jiffies + 1;
        add_timer(&fxo_led_timer);
	return;
}
void FXO_LED_STATE(unsigned int state)
{
	
	switch (state) {
	case 0:	//off 
                del_timer_sync(&fxo_led_timer);
#ifdef _V210_EV_BOARD_	
		GPAB_DATA |= 0x04;
#endif		
#ifdef _V210_Ed_	
		GPAB_DATA |= 0x08;
#endif		
		break;
#if 0
	case 1:	//on
                del_timer_sync(&fxo_led_timer);
		GPAB_DATA &= 0xfffffffb;
		break;
#else
	case 1:	//on
                del_timer_sync(&fxo_led_timer);
                init_timer(&fxo_led_timer);
                fxo_led_timer.expires = jiffies + 1;
                fxo_led_timer.function = fxo_led_on;
                add_timer(&fxo_led_timer);
		break;
#endif		
	case 2:	//blinking
                del_timer_sync(&fxo_led_timer);
                init_timer(&fxo_led_timer);
                fxo_led_timer.expires = jiffies + led_blinking_frequency;
                fxo_led_timer.function = fxo_led_blinking;
                add_timer(&fxo_led_timer);
		break;
	case 3:
		printk("GPAB_DATA=%x\n",GPAB_DATA);
		break;
	default:
		printk("Wrong FXO LED state\n");
		break;			
	}
	return;
}

static void fxs_one_led_blinking(unsigned long data)
{
	static unsigned int gpio_on_off = 0;	//0: off, 1: on.
	
	if (gpio_on_off == 1)
		GPAB_DATA &= 0xfffffff7;
	else
		GPAB_DATA |= 0x08;	
	gpio_on_off ^= 1;
	fxs_one_led_timer.expires = jiffies + led_blinking_frequency;
	add_timer(&fxs_one_led_timer);
	return;
}
void FXS_ONE_LED_STATE(unsigned int state)
{
	
	switch (state) {
	case 0:	//off 
		del_timer_sync(&fxs_one_led_timer);
		GPAB_DATA |= 0x08;
		break;
	case 1:	//on
		del_timer_sync(&fxs_one_led_timer);
		GPAB_DATA &= 0xfffffff7;
		break;
	case 2:	//blinking
		del_timer_sync(&fxs_one_led_timer);
		init_timer(&fxs_one_led_timer);
		fxs_one_led_timer.expires = jiffies + led_blinking_frequency;
		fxs_one_led_timer.function = fxs_one_led_blinking;
		add_timer(&fxs_one_led_timer);
		break;
	default:
		printk("Wrong FXS LED state\n");
		break;			
	}
	return;
}

static void fxs_led_blinking(unsigned long data)
{
	static unsigned int gpio_on_off = 0;	//0: off, 1: on.
	
	if (gpio_on_off == 1)
	#ifdef _V210_EV_BOARD_
		GPAB_DATA &= 0xffffffef;
	#endif
	#ifdef _V210_Ed_
		GPAB_DATA &= 0xfffffffb;
	#endif	
	else
	#ifdef _V210_EV_BOARD_
		GPAB_DATA |= 0x10;
	#endif
	#ifdef _V210_Ed_
		GPAB_DATA |= 0x04;
	#endif			
	gpio_on_off ^= 1;
	fxs_led_timer.expires = jiffies + led_blinking_frequency;
	add_timer(&fxs_led_timer);
	return;
}
void FXS_LED_STATE(unsigned int state)
{
	
	switch (state) {
	case 0:	//off 
		del_timer_sync(&fxs_led_timer);
		#ifdef _V210_EV_BOARD_
		GPAB_DATA |= 0x10;
		#endif
		#ifdef _V210_Ed_
		GPAB_DATA |= 0x04;
		#endif	
		break;
	case 1:	//on
		del_timer_sync(&fxs_led_timer);
		#ifdef _V210_EV_BOARD_
		GPAB_DATA &= 0xffffffef;
		#endif
		#ifdef _V210_Ed_
		GPAB_DATA &= 0xfffffffb;
		#endif	
		break;
	case 2:	//blinking
		del_timer_sync(&fxs_led_timer);
		init_timer(&fxs_led_timer);
		fxs_led_timer.expires = jiffies + led_blinking_frequency;
		fxs_led_timer.function = fxs_led_blinking;
		add_timer(&fxs_led_timer);
		break;
	default:
		printk("Wrong FXS1 LED state\n");
		break;			
	}
	return;
}

static void sip_led_blinking(unsigned long data)
{
	static unsigned int gpio_on_off = 0;	//0: off, 1: on.
	
	if (gpio_on_off == 1)
		GPAB_DATA &= 0xffffffdf;
	else
		GPAB_DATA |= 0x20;	
	gpio_on_off ^= 1;
	sip_led_timer.expires = jiffies + led_blinking_frequency;
	add_timer(&sip_led_timer);
	return;
}
void SIP_LED_STATE(unsigned int state)
{
	
	switch (state) {
	case 0:	//off 
		del_timer_sync(&sip_led_timer);
		GPAB_DATA |= 0x20;
		break;
	case 1:	//on
		del_timer_sync(&sip_led_timer);
		GPAB_DATA &= 0xffffffdf;
		break;
	case 2:	//blinking
		del_timer_sync(&sip_led_timer);
		init_timer(&sip_led_timer);
		sip_led_timer.expires = jiffies + led_blinking_frequency;
		sip_led_timer.function = sip_led_blinking;
		add_timer(&sip_led_timer);
		break;
	default:
		printk("Wrong SIP LED state\n");
		break;			
	}
	return;
}

void fxs_led_state(unsigned int chid, unsigned int state)
{
	if (chid == 0)
		FXS_LED_STATE(state);
	else if (chid == 1)
		FXS_ONE_LED_STATE(state);
	else
		printk("%s-%d: undefined chid = %d\n", chid);
}

void LED_Init(void)
{
	GPAB_DIR = GPAB_DIR | 0x3c;

	fxs_led_state(0, 0);
#if SLIC_CH_NUM == 2	
	fxs_led_state(1, 0);
#endif	
	FXO_LED_STATE(1);	
}


/* =========================================================================== */
