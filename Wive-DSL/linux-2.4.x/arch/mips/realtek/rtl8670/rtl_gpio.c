/*
 * FILE NAME rtl_gpio.c
 *
 * BRIEF MODULE DESCRIPTION
 *  GPIO For Flash Reload Default
 *
 *  Author: jimmylin@realtek.com.tw
 *
 * Copyright 2005 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE	LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 */



#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/reboot.h>
#include <linux/kmod.h>
#include <linux/proc_fs.h>

// Mason Yu
#if defined(CONFIG_RTL8670)
#include "lx4180.h"
#else // 8671
#include "lx5280.h"
#endif
#include "gpio.h"

#ifdef CONFIG_RTL8672
#define PROBE_NULL	0
#define PROBE_ACTIVE	1
#define PROBE_RESET	2
#define PROBE_RELOAD	3

#ifdef CONFIG_GPIO_LED_TR068

#include <linux/notifier.h>
#include <linux/if.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include "led-generic.h"
#include "pushbutton.h"

// static int wan_ipif_counter = 0;
// Mason Yu. Add proc file for push button.
static char default_flag='0';
static char reboot_flag='0';

#ifdef CONFIG_WLAN_ON_OFF_BUTTON
//cathy
static char wlan_onoff_flag='0';
#endif

void tr068_internet_led(char state);

void internet_led_check(void) {
	struct net_device *dev;
	struct in_device *in_dev;
	
	read_lock(&dev_base_lock);
	for (dev = dev_base; dev; dev = dev->next) {
		//printk("check(1) %s, %x, %x\n", dev->name, dev->flags, dev->priv_flags);
		if (!(dev->flags & IFF_UP))
			continue;
			
		if ((!(dev->priv_flags & IFF_DOMAIN_WAN)) && (!(dev->flags & IFF_POINTOPOINT)))
			continue;
		in_dev = dev->ip_ptr;
		//in_dev = in_dev_get(dev);
		//printk("check(2) %s, %p\n", dev->name, in_dev);
		if (!in_dev)
			continue;

		for_ifa(in_dev) {
			// at least one WAN IP exist.
		tr068_internet_led('1');
			read_unlock(&dev_base_lock);
			return;
		} endfor_ifa(in_dev);
	}
	read_unlock(&dev_base_lock);
	tr068_internet_led('0');
}
	

static int if_inetaddr_event(struct notifier_block *this, unsigned long event, void *ifa)
{
	//printk("%s(%p, %d, %p)\n", __FUNCTION__, this, event, ifa);
	internet_led_check();
		return NOTIFY_DONE;
	}


static struct notifier_block if_addr_notifier = {
	.notifier_call = if_inetaddr_event,
};

static struct notifier_block if_dev_notifier = {
	.notifier_call = if_inetaddr_event,
};



static char power_flag='2';
static char internet_flag='0';

static int power_read_proc(char *page, char **start, off_t off,
                     int count, int *eof, void *data)
{
      int len;
      len = sprintf(page, "%c\n", power_flag);
      if (len <= off+count) *eof = 1;
          *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
          return len;

}
static int power_write_proc(struct file *file, const char *buffer,
                      unsigned long count, void *data)
{	
	if (buffer && !copy_from_user(&power_flag, buffer, sizeof(power_flag))) {  
		switch(power_flag) {
		case '0':
			led_off(LED_POWER_RED);
			led_on(LED_POWER_GREEN);
			break;
		case '1':
			led_off(LED_POWER_RED);
			led_off(LED_POWER_GREEN);
			break;
		case '2':
			led_off(LED_POWER_GREEN);
			led_on(LED_POWER_RED);
			break;
		}
		return count;
	}
	return -EFAULT;
}

void tr068_internet_led(char state) {
	if (internet_flag==state)
		return;	
	
	switch(state) {
	case '0':
		led_off(LED_INTERNET_GREEN);
		led_off(LED_INTERNET_RED);
		tr068_internet_led_stop();
		break;
	case '1':
		led_on(LED_INTERNET_GREEN);
		led_off(LED_INTERNET_RED);
		tr068_internet_led_start();
		break;
	case '2':
		led_off(LED_INTERNET_GREEN);
		led_on(LED_INTERNET_RED);
		tr068_internet_led_stop();
		break;
	// test only
	/*
	case '3': 
		tr068_internet_traffic();
		printk("%u\n", traffic_count);
	*/

	default:
		return;
	}

	internet_flag = state;
}



static int internet_read_proc(char *page, char **start, off_t off,
                     int count, int *eof, void *data)
{
      int len;
      len = sprintf(page, "%c\n", internet_flag);
      if (len <= off+count) *eof = 1;
          *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
          return len;

}
static int internet_write_proc(struct file *file, const char *buffer,
                      unsigned long count, void *data)
{	
	char input;
	if (buffer && !copy_from_user(&input, buffer, sizeof(input))) {  
		tr068_internet_led(input);
		return count;
	}
	return -EFAULT;
}

// Mason Yu. Add proc file for push button. Start.
static int default_read_proc(char *page, char **start, off_t off,
                     int count, int *eof, void *data)
{
      int len;
      len = sprintf(page, "%c\n", default_flag);
      if (len <= off+count) *eof = 1;
          *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
          return len;

}

static int default_write_proc(struct file *file, const char *buffer,
                      unsigned long count, void *data)
{
      if (count < 2)
         return -EFAULT;
      if (buffer && !copy_from_user(&default_flag, buffer, 1)) {
         return count;
         }
      return -EFAULT;
}


static int reboot_read_proc(char *page, char **start, off_t off,
                     int count, int *eof, void *data)
{
      int len;
      len = sprintf(page, "%c\n", reboot_flag);
      if (len <= off+count) *eof = 1;
          *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
          return len;

}

static int reboot_write_proc(struct file *file, const char *buffer,
                      unsigned long count, void *data)
{
      if (count < 2)
         return -EFAULT;
      if (buffer && !copy_from_user(&reboot_flag, buffer, 1)) {
         return count;
         }
      return -EFAULT;
}
// Mason Yu. Add proc file for push button. End.

#ifdef CONFIG_WLAN_ON_OFF_BUTTON
//cathy, for WIFISW button
static int wlan_onoff_read_proc(char *page, char **start, off_t off,
                     int count, int *eof, void *data)
{
      int len;
      len = sprintf(page, "%c\n", wlan_onoff_flag);
      if (len <= off+count) *eof = 1;
          *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
          return len;

}

static int wlan_onoff_write_proc(struct file *file, const char *buffer,
                      unsigned long count, void *data)
{
      if (buffer && !copy_from_user(&wlan_onoff_flag, buffer, 1)) {
         return count;
      }
      return -EFAULT;
}
#endif

// Mason Yu. Add proc file for push button.
static void pb_reset_event(int event) {
	static unsigned int pb_counter = 0;
	static int pb_state = PROBE_NULL;

	switch(pb_state) {
	case PROBE_NULL:
		if (event) {
			pb_state = PROBE_ACTIVE;
			pb_counter ++;
		}
		break;
	case PROBE_ACTIVE:
		if (event) {
			pb_counter++;
			// Mason Yu
			/*
			if (pb_counter >= 5) {				
				kernel_restart(NULL);
			}
			*/			
		} else {
			// Mason Yu
			pb_state = PROBE_NULL;			
			if (pb_counter < 2)
			{				
				printk("Push Button do nothing.\n");			
			}
			else if (pb_counter >= 5)	
			{
				//reload default
			        default_flag='1';	
			        printk("Set default_flag = '1'\n");
				

			}			
			else//2<=probe_counter<5
			{				
				reboot_flag='1';
				printk("Set reboot_flag = '1'\n");				
			}
			pb_counter = 0;
		}
		break;
	}
}

#ifdef CONFIG_WLAN_ON_OFF_BUTTON
//cathy, if pressed less than 4 seconds, do on-off switch
static void pb_wlan_event(int event) {
	static unsigned int pb_counter = 0;
	static int pb_state = PROBE_NULL;

	switch(pb_state) {
	case PROBE_NULL:
		if (event) {
			pb_state = PROBE_ACTIVE;
			pb_counter ++;
		}
		break;
	case PROBE_ACTIVE:
		if (event) {
			pb_counter++;
		} else {
			pb_state = PROBE_NULL;			
			if (pb_counter < 4)
			{
				wlan_onoff_flag='1';
				printk("wifi on-off switch\n");			
			}
			pb_counter = 0;
		}
		break;
	}
}
#endif

static void rtl_gpio_timer(unsigned long data) {
	struct timer_list *timer = (struct timer_list *)data;
	pb_reset_event(  pb_is_pushed(PB_RESET) );
#ifdef CONFIG_WLAN_ON_OFF_BUTTON	
	pb_wlan_event(  pb_is_pushed(PB_WIFISW) );
#endif
	mod_timer(timer, jiffies + 100);
}

static struct timer_list probe_timer;

static int __init rtl_gpio_init(void)
{
	int ret = 0;
	struct proc_dir_entry *entry=NULL;
	
	printk("Realtek GPIO Driver for Flash Reload Default\n");

	register_inetaddr_notifier(&if_addr_notifier);
	register_netdevice_notifier(&if_dev_notifier);

	// diagnostic..
	//led_test();	

	entry = create_proc_entry("power_flag", 0644, NULL);
	if (entry) {
		entry->read_proc=power_read_proc;
		entry->write_proc=power_write_proc;
		entry->owner 	  = NULL;
		entry->mode 	  = S_IFREG | S_IRUGO;
		entry->uid 	  = 0;
		entry->gid 	  = 0;
		entry->size 	  = 37;
	}	

	entry = create_proc_entry("internet_flag", 0644, NULL);
	if (entry) {
		entry->read_proc=internet_read_proc;
		entry->write_proc=internet_write_proc;
		entry->owner 	  = NULL;
		entry->mode 	  = S_IFREG | S_IRUGO;
		entry->uid 	  = 0;
		entry->gid 	  = 0;
		entry->size 	  = 37;
	}

	/*
	entry = create_proc_entry("led", 0644, NULL);
		if (entry) {
			entry->read_proc=led_read_proc;
			entry->write_proc=led_write_proc;
			entry->owner	  = NULL;
			entry->mode 	  = S_IFREG | S_IRUGO;
			entry->uid	  = 0;
			entry->gid	  = 0;
			entry->size 	  = 37;
		}	
	*/
	
	// Mason Yu. Add proc file for push button. Start.
	entry = create_proc_entry("load_default", 0, NULL);
        if (entry) {
                entry->read_proc=default_read_proc;
	        entry->write_proc=default_write_proc;
	}

	entry = create_proc_entry("load_reboot", 0, NULL);
        if (entry) {
                entry->read_proc=reboot_read_proc;
	        entry->write_proc=reboot_write_proc;
	}
	// Mason Yu. Add proc file for push button. End.
#ifdef CONFIG_WLAN_ON_OFF_BUTTON
	//cathy, add for WIFISW button
	entry = create_proc_entry("wlan_onoff", 0, NULL);
        if (entry) {
                entry->read_proc=wlan_onoff_read_proc;
	        entry->write_proc=wlan_onoff_write_proc;
	}
#endif	
	//led_tr068_init();
	init_timer (&probe_timer);
	probe_timer.expires = jiffies + 100;
	probe_timer.data = (unsigned long)&probe_timer;
	probe_timer.function = &rtl_gpio_timer;
	mod_timer(&probe_timer, jiffies + 100);

//cleanup:

	return ret;
}

static void __exit rtl_gpio_exit(void)
{
	printk("Unload Realtek GPIO Driver \n");
	//del_timer_sync(&probe_timer);
}

module_init(rtl_gpio_init);
module_exit(rtl_gpio_exit);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("GPIO driver for Reload default");

#endif

#else

//#define CONFIG_WIFI_SIMPLE_CONFIG

#if defined(RESET_BUTTON)

#define RESET_BTN_PIN		22
#define WPS_LED     23

#define PBC_BUTTON 11 // B3

//#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
#ifdef CONFIG_WLAN_ON_OFF_BUTTON
#define WLAN_RESET   19
#endif
#ifdef CONFIG_WIFI_SIMPLE_CONFIG
//cathy
static unsigned char wps_wlan_restart=0;
#endif

#define PROBE_NULL		0
#define PROBE_ACTIVE	1
#define PROBE_RESET		2
#define PROBE_RELOAD	3


//#define  GPIO_DEBUG
#ifdef GPIO_DEBUG
/* note: prints function name for you */
#  define DPRINTK(fmt, args...) printk("%s: " fmt, __FUNCTION__ , ## args)
#else
#  define DPRINTK(fmt, args...)
#endif

static struct timer_list probe_timer;
static unsigned int    probe_counter;
static unsigned int    probe_state;
//#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
#ifdef CONFIG_WLAN_ON_OFF_BUTTON
static unsigned int    wlan_state=0;
#endif

static char default_flag='0';
static char reboot_flag='0';
//#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
#ifdef CONFIG_WLAN_ON_OFF_BUTTON
static char wlan_flag='0';
#endif
//#ifdef TR068_POWER_LED
static char power_flag='2';
//#endif

/******  HARDWARE DEPENDENT SECTION ====> ******/

#ifdef CONFIG_NET_WIRELESS
#ifndef CONFIG_WIFI_SIMPLE_CONFIG
void wps_led_set(int isOn) {
	return;
}
#else
static void rtl8185_wps_led(int isOn) {
	unsigned char val;

	struct net_device *dev = dev_get_by_name(WLAN_NAME);
	if (0 == dev) {
		printk("dev %s not found\n", WLAN_NAME);
		return;
	}

	rtk8185_reg_write(dev, 2, 0x80, rtk8185_reg_read(dev, 2, 0x80) | (1 << 10));
	rtk8185_reg_write(dev, 2, 0x82, rtk8185_reg_read(dev, 2, 0x82) & (~(1 << 10)));
	rtk8185_reg_write(dev, 2, 0x84, rtk8185_reg_read(dev, 2, 0x84) | (1 << 10));

	
	val = rtk8185_reg_read(dev, 1, 0x91);
	if (isOn) {
		val = val | 0x02;
	} else {
		val = val & (~0x02);
	}
	rtk8185_reg_write(dev, 1, 0x91, val);
	
}

void wps_led_set(int isOn) {
	//printk("wps led %s\n", isOn ? "on" : "off");
#if (BOARD_TYPE == BIG_MODEL_WPS)
	if (isOn) 
		gpioClear(7);
	else
		gpioSet(7);
#elif (BOARD_TYPE == ALPHA_MODEL_WPS)
	rtl8185_wps_led(isOn);
#endif	
}

static int IsPushButtonPressed() {
#if (BOARD_TYPE == BIG_MODEL_WPS)

	gpioConfig(PBC_BUTTON, GPIO_FUNC_INPUT);
	if (gpioRead(PBC_BUTTON) == 1)
		return 1;
	else
		return 0;
	
#elif (BOARD_TYPE == ALPHA_MODEL_WPS)

	unsigned char val;

	struct net_device *dev = dev_get_by_name(WLAN_NAME);
	if (0 == dev) {
		printk("dev %s not found\n", WLAN_NAME);
		return;
	}


	rtk8185_reg_write(dev, 2, 0x80, rtk8185_reg_read(dev, 2, 0x80) | (1 << 10));
	rtk8185_reg_write(dev, 2, 0x82, rtk8185_reg_read(dev, 2, 0x82) & (~(1 << 10)));
	rtk8185_reg_write(dev, 2, 0x84, rtk8185_reg_read(dev, 2, 0x84) | (1 << 10));
		
	val = rtk8185_reg_read(dev, 1, 0x91);
	rtk8185_reg_write(dev, 1, 0x91, val & ~(1 << 2));

	val = rtk8185_reg_read(dev, 1, 0x92);

	// it is low active
	if (val & 0x04)
		return 0;
	else
		return 1;
#else
	return 0;
#endif
}

#endif // CONFIG_WIFI_SIMPLE_CONFIG

/****** <=====  HARDWARE DEPENDENT SECTION ******/

#ifdef CONFIG_WIFI_SIMPLE_CONFIG
static unsigned int		AutoCfg_LED_Blink;
static unsigned int		AutoCfg_LED_Toggle;

static void autoconfig_gpio_init(void)
{	
	REG32(GPIO_PBDIR) = REG32(GPIO_PBDIR) | (1<<WPS_LED);	
	wps_led_set(0);
}


static void autoconfig_gpio_off(void)
{
	//RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) | (1 << AUTOCFG_LED_PIN)));
	wps_led_set(0);
	AutoCfg_LED_Blink = 0;
}


static void autoconfig_gpio_on(void)
{
	//RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1 << AUTOCFG_LED_PIN))));
	wps_led_set(1);
	AutoCfg_LED_Blink = 0;
}


static void autoconfig_gpio_blink(void)
{
	//RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1 << AUTOCFG_LED_PIN))));
	wps_led_set(1);
	AutoCfg_LED_Blink = 1;
	AutoCfg_LED_Toggle = 1;
}


static int gpio_read_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	int len;
	char flag;

//cathy
	if(wps_wlan_restart) {
		flag = '2';
	}
	else if (IsPushButtonPressed()) {
		flag = '1';
	} else {
		flag = '0';
	}

	len = sprintf(page, "%c\n", flag);


	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len > count) len = count;
	if (len < 0) len = 0;
	
	return len;
}


static int gpio_write_proc(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	char flag;

	if (count < 2)
		return -EFAULT;

	DPRINTK("file: %08x, buffer: %s, count: %lu, data: %08x\n",
		(unsigned int)file, buffer, count, (unsigned int)data);
	if (buffer && !copy_from_user(&flag, buffer, 1)) {
		if (flag == 'E')
			autoconfig_gpio_init();
		else if (flag == '0'){
			wps_wlan_restart = 0;
			autoconfig_gpio_off();
		}
		else if (flag == '1')
			autoconfig_gpio_on();
		else if (flag == '2')
			autoconfig_gpio_blink();
		//cathy
		else if (flag == 'R') 
			wps_wlan_restart = 1;	
#ifdef CONFIG_RTL8186_TR			
		else if (flag == '3') // update flash in special case		
			flash_write_flag = 0x8000;				
		else if (flag == '4') // enable system led blinking 
			system_led_blink = 1;	
		else if (flag == '5') // disable system led blinking 
			system_led_blink = 0;			
		else if (flag == '6')
			autoconfig_gpio_blink_quick();
		else if (flag == '7') // disable system led 
			system_led_blink = 2;								
#endif			

#ifdef CONFIG_RTL8186_KB
		else if (flag == '3')
			disable_power_led_blink=1;		
		else if (flag == '4')
			disable_power_led_blink=0;			
#ifdef CONFIG_SQUASHFS
		else if (flag == '5') {		
			disable_power_led_blink=3; // set power-led off
		}
#endif			
#endif			
		else
			{}

		return count;
	}
	else
		return -EFAULT;
}
#endif
#endif // of CONFIG_NET_WIRELESS


//#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
#ifdef GPIO_LED_TR068_POWER
struct timer_list FlashPoserLedTimer;
void flashpowerled()
 {
	static int CtrlFlag = 0;
	
	FlashPoserLedTimer.expires = jiffies+HZ/2;
	gpio_red_power(CtrlFlag);

	if(power_flag == '1')
	{
		CtrlFlag = (CtrlFlag == 1)?0:1;

		FlashPoserLedTimer.function=(void (*)(unsigned long))flashpowerled;
		mod_timer(&FlashPoserLedTimer, FlashPoserLedTimer.expires);	
	}
 }
#endif

static void rtl_gpio_timer(unsigned long data)
{
	unsigned int pressed=1;	

//#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
#ifdef CONFIG_WLAN_ON_OFF_BUTTON
	unsigned int wlan_pressed=0;

	if ( ( REG32(GPIO_PADAT) & (1 << WLAN_RESET) ) ){		//PA3 is high level default
		wlan_pressed = 0;
	}
	else
	{		
		wlan_pressed = 1;		
	}


//	printk("\nkernel:wlan_flag=%c,wlan_pressed=%d\n",wlan_flag,wlan_pressed);
	if(wlan_pressed)
	{
		printk("\nwlan_press=1\n");
		if(wlan_flag == '0')
			wlan_flag = '1';
	}

#endif
	
	// Mason Yu
	if ( ( REG32(GPIO_PADAT) & (1 << RESET_BTN_PIN) ) ){		//PA6 is high level default
	//if ( ( REG32(GPIO_PBDAT) & (1 << RESET_BTN_PIN) ) ){		//PB3 is high level default
		pressed = 0;
		//printk("high level\n");
	}
	else
	{		
		pressed = 1;		
		//printk("low level\n");
	}

	if (probe_state == PROBE_NULL)
	{
		if (pressed)
		{
			probe_state = PROBE_ACTIVE;
			probe_counter++;			
		}
		else {			
			probe_counter = 0;
		}	
	}else if (probe_state == PROBE_ACTIVE)
	{
		if (pressed)
		{			
			probe_counter++;			
			#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
			if(probe_counter>=6)
			    {
			  		    power_flag='2';
			}
			#endif
		}
		else
		{
#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
			if (probe_counter >= 3)	
			{
#define NEW_HARDWARE_RESET	
//now we assume the flash is AMD type, and current config space is 0xbfc06000...
//jim 20071010
#ifdef NEW_HARDWARE_RESET			
				//reload default
				 void (*restart)();
				unsigned short gimr;
				volatile unsigned short read1;
				volatile unsigned short read2;
				restart = (void (*)())(0xbfc00000);
				//erase current configuration space...
#define AM29LVXXX_COMMAND_ADDR1(x)        ((volatile unsigned short *) ((x)+ 0x555 * 2))
#define AM29LVXXX_COMMAND_ADDR2(x)         ((volatile  unsigned short *) ((x) + 0x2AA * 2))
#define AM29LVXXX_COMMAND1              0xAA
#define AM29LVXXX_COMMAND2              0x55
#define AM29LVXXX_SECTOR_ERASE_CMD1     0x80
#define AM29LVXXX_SECTOR_ERASE_CMD2     0x30
#define AM29LVXXX_PROGRAM_CMD           0xA0
#define CURRENT_CONFIG_SPACE   (0XBFC06000)
//#define REG16(reg)	(*(volatile unsigned short *)(reg))
				gimr=REG16(GIMR);
				REG16(GIMR)=0; //close all interrupt....
				*AM29LVXXX_COMMAND_ADDR1(0xbfc00000) = AM29LVXXX_COMMAND1;
            			*AM29LVXXX_COMMAND_ADDR2(0xbfc00000) = AM29LVXXX_COMMAND2;
            			*AM29LVXXX_COMMAND_ADDR1(0xbfc00000) = AM29LVXXX_SECTOR_ERASE_CMD1;
            			*AM29LVXXX_COMMAND_ADDR1(0xbfc00000) = AM29LVXXX_COMMAND1;
            			*AM29LVXXX_COMMAND_ADDR2(0xbfc00000) = AM29LVXXX_COMMAND2;
            			*(volatile unsigned short*)(0xbfc06000) = AM29LVXXX_SECTOR_ERASE_CMD2;
				do 
				{  // wait until it stop toggle.
					
       				     read1 = *(volatile unsigned short*)(CURRENT_CONFIG_SPACE);
      					      read2 = *(volatile unsigned short*)(CURRENT_CONFIG_SPACE);
       			} while (read1 != read2);

				{     
					unsigned int tickCount;
					unsigned short pat;
		 			tickCount=0x0fffffff; 
	       			while (!((pat = *(volatile unsigned short*)(CURRENT_CONFIG_SPACE)) & 0x80))
	 				{
            
						if(!(tickCount--))
	 	    				{ 
	 	    	  				break;
	 	    				}
			
	 				}
	 	
        			}
				//REG16(GIMR)=gimr; //restore all interupt mask....
				{
					printk("Watchdog Reset!\n");
	    	    	    	    	//reset nic
	    	    	    	    	*((char*)0xB980003B) = 0x01;

#ifdef CONFIG_NET_WIRELESS
	    	    	    	    	//reset wlan
	    	    	    	    	*((char*)0xBD300037) = 0x04;
#endif	    

	    	    	    	    	//reset sar
	    	    	    	    	extern void Reset_Sar(void);
	    	    	    	    	Reset_Sar();
	    	    	    	    	Idle(200000);
	    				#include "lx5280.h"
	    				#define WDTKICK (WDTCLR|OVSEL_13)  //command to kick watchdog
	    	    	    	    	REG32(WDTCNR) = WDTKICK;  //start hw watchdog
	    	    	    	    	while (1);  //wait until reboot	
				}
				//reboot...
				restart();
#endif				
				
				
			        default_flag='1';	
			        printk("Set default_flag = '1'\n");
				//hardware restart!!!
				
	//			kernel_thread(reset_flash_default, (void *)1, SIGCHLD);
				return;
			}else
			{
				probe_state = PROBE_NULL;
				probe_counter = 0;	
			}
#else
			if (probe_counter < 2)
			{
				probe_state = PROBE_NULL;
				probe_counter = 0;				
			}
			else if (probe_counter >= 5)	
			{
				//reload default
			        default_flag='1';	
			        printk("Set default_flag = '1'\n");
	//			kernel_thread(reset_flash_default, (void *)1, SIGCHLD);
				return;

			}			
			else//2<=probe_counter<5
			{
				printk("Set reboot_flag = '1'\n");
				reboot_flag='1';
				//kill_proc(1,SIGTERM,1);
		//		kernel_thread(reset_flash_default, 0, SIGCHLD);
				return;
			}
#endif			
		}
	}
//#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
#ifdef GPIO_LED_TR068_POWER
	static int flashflag=0;
	if(power_flag == '0')
	{
		flashflag = 0;
		gpio_red_power(0);
		gpio_green_power(1);
	}
	else if(power_flag == '2')
	{
		flashflag = 0;
		gpio_red_power(1);
		gpio_green_power(0);
	}
	else
	{
		gpio_green_power(0);
		if(flashflag == 0){
			flashflag = 1;
			flashpowerled();
		}
	}

#endif

#ifdef CONFIG_NET_WIRELESS
#ifdef CONFIG_WIFI_SIMPLE_CONFIG
	if (AutoCfg_LED_Blink==1)
	{
		if (AutoCfg_LED_Toggle) {
			//RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) | (1 << AUTOCFG_LED_PIN)));
			wps_led_set(0);
		}
		else {
			//RTL_W32(RTL_GPIO_PABDATA, (RTL_R32(RTL_GPIO_PABDATA) & (~(1 << AUTOCFG_LED_PIN))));
			wps_led_set(1);
		}
		AutoCfg_LED_Toggle = AutoCfg_LED_Toggle? 0 : 1;
	}
#endif
#endif // of CONFIG_NET_WIRELESS
#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
	mod_timer(&probe_timer, jiffies + 50);
#else
	mod_timer(&probe_timer, jiffies + 100);
#endif
}



static int default_read_proc(char *page, char **start, off_t off,
                     int count, int *eof, void *data)
{
      int len;
      len = sprintf(page, "%c\n", default_flag);
      if (len <= off+count) *eof = 1;
          *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
          return len;

}

static int default_write_proc(struct file *file, const char *buffer,
                      unsigned long count, void *data)
{
      if (count < 2)
         return -EFAULT;
      if (buffer && !copy_from_user(&default_flag, buffer, 1)) {
         return count;
         }
      return -EFAULT;
}


static int reboot_read_proc(char *page, char **start, off_t off,
                     int count, int *eof, void *data)
{
      int len;
      len = sprintf(page, "%c\n", reboot_flag);
      if (len <= off+count) *eof = 1;
          *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
          return len;

}

static int reboot_write_proc(struct file *file, const char *buffer,
                      unsigned long count, void *data)
{
      if (count < 2)
         return -EFAULT;
      if (buffer && !copy_from_user(&reboot_flag, buffer, 1)) {
         return count;
         }
      return -EFAULT;
}

//#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
#ifdef CONFIG_WLAN_ON_OFF_BUTTON
static int wlan_read_proc(char *page, char **start, off_t off,
                     int count, int *eof, void *data)
{
      int len;
      len = sprintf(page, "%c\n", wlan_flag);
      if (len <= off+count) *eof = 1;
          *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
          return len;

}
static int wlan_write_proc(struct file *file, const char *buffer,
                      unsigned long count, void *data)
{
	
      if (buffer && !copy_from_user(&wlan_flag, buffer, count)) {  
         return count;
         }
      return -EFAULT;
}
#endif

static int power_read_proc(char *page, char **start, off_t off,
                     int count, int *eof, void *data)
{
      int len;
      len = sprintf(page, "%c\n", power_flag);
      if (len <= off+count) *eof = 1;
          *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
          return len;

}

static int power_write_proc(struct file *file, const char *buffer,
                      unsigned long count, void *data)
{
	
      if (buffer && !copy_from_user(&power_flag, buffer, count)) {  
         return count;
         }
      return -EFAULT;
}

int __init rtl_gpio_init(void)
{
	struct proc_dir_entry *res=NULL;

	printk("Realtek GPIO Driver for Flash Reload Default\n");
	
	// Modified by Mason Yu
	// Set GPIOA pin 6 as input pin for reset button	
	if ( (1 << RESET_BTN_PIN) == (REG32(GPIO_PADIR) & (1 << RESET_BTN_PIN)) ){
	//if ( (1 << RESET_BTN_PIN) == (REG32(GPIO_PBDIR) & (1 << RESET_BTN_PIN)) ){
		//printk("***** Set GPIO pin 6 as input bin\n");
		REG32(GPIO_PADIR) = REG32(GPIO_PADIR) - (1 << RESET_BTN_PIN);
		//REG32(GPIO_PBDIR) = REG32(GPIO_PBDIR) - (1 << RESET_BTN_PIN);
	}
//#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
#ifdef CONFIG_WLAN_ON_OFF_BUTTON
	if ( (1 << WLAN_RESET) == (REG32(GPIO_PADIR) & (1 << WLAN_RESET)) ){
		//printk("***** Set GPIO A3 as input bin\n");
		REG32(GPIO_PADIR) = REG32(GPIO_PADIR) - (1 << WLAN_RESET);
	}
#endif

	res = create_proc_entry("load_default", 0, NULL);
        if (res) {
                res->read_proc=default_read_proc;
	        res->write_proc=default_write_proc;
	}

	res = create_proc_entry("load_reboot", 0, NULL);
        if (res) {
                res->read_proc=reboot_read_proc;
	        res->write_proc=reboot_write_proc;
	}
//#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
#ifdef CONFIG_WLAN_ON_OFF_BUTTON
	res = create_proc_entry("wlan_onoff", 0644, NULL);
        if (res) {
                res->read_proc=wlan_read_proc;
	        res->write_proc=wlan_write_proc;
		res->owner 	  = NULL;
		res->mode 	  = S_IFREG | S_IRUGO;
		res->uid 	  = 0;
		res->gid 	  = 0;
		res->size 	  = 37;
	}
#endif

//#ifdef TR068_POWER_LED
	res = create_proc_entry("power_flag", 0644, NULL);
        if (res) {
                res->read_proc=power_read_proc;
	        res->write_proc=power_write_proc;
		res->owner 	  = NULL;
		res->mode 	  = S_IFREG | S_IRUGO;
		res->uid 	  = 0;
		res->gid 	  = 0;
		res->size 	  = 37;
	}
//#endif

#ifdef CONFIG_NET_WIRELESS
#ifdef CONFIG_WIFI_SIMPLE_CONFIG	
	res = create_proc_entry("gpio", 0, NULL);
	if (res) {
		res->read_proc = gpio_read_proc;
		res->write_proc = gpio_write_proc;
	}
	else {
		printk("Realtek GPIO Driver, create proc failed!\n");
	}
#endif        
#endif // of CONFIG_NET_WIRELESS

	init_timer (&probe_timer);
	probe_counter = 0;
	probe_state = PROBE_NULL;
#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
	probe_timer.expires = jiffies + 50;
#else
	probe_timer.expires = jiffies + 100;
#endif
	probe_timer.data = (unsigned long)NULL;
	probe_timer.function = &rtl_gpio_timer;
#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)
	mod_timer(&probe_timer, jiffies + 50);
#else
	mod_timer(&probe_timer, jiffies + 100);
#endif
	return 0;
}


static void __exit rtl_gpio_exit(void)
{
	printk("Unload Realtek GPIO Driver \n");
	del_timer_sync(&probe_timer);
}


module_exit(rtl_gpio_exit);
module_init(rtl_gpio_init);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("GPIO driver for Reload default");

#endif
#endif
