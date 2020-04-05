#include <linux/interrupt.h>
#include "rtk_voip.h"
#include "virtual_daa.h"

char relay_2_PSTN_flag[MAX_SLIC_CH_NUM]={0};     /* 1: relay to PSTN , 0: relay to SLIC */

// for virtual DAA.GPIOD_14 for on_hook or off_hook detect, GPIOD_13 for ring incoming detect.
// GPIOD_12 for hook det of second relay

// set the virtual-daa-used gpio pin direction to i/p.
void virtual_daa_init()
{
	
#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA_2_RELAY_SUPPORT
	GPCD_DIR = GPCD_DIR & 0xFFFF8FFF;	
#else
	GPCD_DIR = GPCD_DIR & 0xFFFF9FFF;
#endif
	//printk("Virtual DAA Init.\n");
}

//1: off-hook, 0: on-hook
unsigned char virtual_daa_hook_detect(unsigned char relay_id)
{
	if (relay_id == 0)
	{
		if (GPCD_DATA&0x40000000) //GPIOD_14
		{
			return ON_HOOK;
		}
		else
		{
			return OFF_HOOK;
		}
	}
#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA_2_RELAY_SUPPORT
	else if (relay_id == 1)
	{
		if (GPCD_DATA&0x10000000) //GPIOD_12
		{
			return ON_HOOK;
		}
		else
		{
			return OFF_HOOK;
		}
	}
#endif
}
	 
//1: ring incoming, 0: no ring incoming
static unsigned char virtual_daa_ring_incoming_detect() //GPIO_D bit13: i/p
{	
	int i;
	
	if (!(GPCD_DATA&0x20000000)) 
	{
		return RING_INCOMING;				
	
	}

	return NO_RING;	
}
	

#define CIR_SIZE	100
#define DAA_RING_THS	30
char cir_res[CIR_SIZE] = {0};
int cir_idx = 0;
int summation = 0;

unsigned char virtual_daa_ring_det(void)
{
	summation -= cir_res[cir_idx];
	
	cir_res[cir_idx] = virtual_daa_ring_incoming_detect();
	
	summation += cir_res[cir_idx];
	cir_idx++;
	
	if (cir_idx >= CIR_SIZE)
		cir_idx = 0;
#if 0
	if (summation != 0)	
		printk("%d\n", summation);
#endif
	if (summation < 0)
		printk("error: virtual_daa_ring_det() \n");
	
	/* TH: When DAA incomimg ring, summation can reach around 55 */
	if (summation > DAA_RING_THS)
	{
		return RING_INCOMING;
	}
	else
	{
		return NO_RING;
	}
}

//1: pull high, 0: pull low
// relay_id should match to ch_id. i.e. each pcm channel can relay to PSTN.
char virtual_daa_relay_switch(unsigned char relay_id, unsigned char state)	//realy_id=0: GPIO_D bit6: o/p
{
	unsigned long flags;
	save_flags(flags); cli();
	if (state == RELAY_PSTN)	/* pull low */
	{	
		if (relay_id == 0)
		{
			if (relay_2_PSTN_flag[0] == 0)
			{
				GPCD_DATA = (GPCD_DATA>>16) & 0xffbf;
				printk("--- Switch relay %d to PSTN ---\n", relay_id);
				
				return RELAY_SUCCESS;
			}
			else
				printk("--- Relay %d has been switch to PSTN ---\n", relay_id);
			
			
		}
		else if (relay_id == 1)
		{
		
#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA_2_RELAY_SUPPORT

			if (relay_2_PSTN_flag[1] == 0)
			{
				GPAB_DATA = GPAB_DATA & 0xffdf;
				printk("--- Switch relay %d to PSTN ---\n", relay_id);
				
				return RELAY_SUCCESS;
			}
			else
				printk("--- Relay %d has been switch to PSTN ---\n", relay_id);
#else
			printk("** ERROR **: no relay for pcm channel %d.\n", relay_id);
			
			return RELAY_FAIL;
#endif
		}
		else
		{
			printk("** ERROR **: no relay for pcm channel %d.\n", relay_id);
			
			return RELAY_FAIL;
		}
	}
	else if (state == RELAY_SLIC)	/* pull high */
	{
		if (relay_id == 0)
		{
			if (relay_2_PSTN_flag[0] == 1)
			{
				GPCD_DATA = (GPCD_DATA>>16) | 0x40;
				printk("--- Switch relay %d to SLIC ---\n", relay_id);
				
				return RELAY_SUCCESS;
			}
			else
				printk("--- Relay %d has been switch to SLIC ---\n", relay_id);
			
		}
		else if (relay_id == 1)
		{
		
#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA_2_RELAY_SUPPORT

			if (relay_2_PSTN_flag[1] == 1)
			{
				GPAB_DATA = GPAB_DATA | 0x20;
				printk("--- Switch relay %d to SLIC ---\n", relay_id);
				
				return RELAY_SUCCESS;
			}
			else
				printk("--- Relay %d has been switch to SLIC ---\n", relay_id);
#else
			printk("** ERROR **: no relay for pcm channel %d.\n", relay_id);
			
			return RELAY_FAIL;
#endif
		}
		else
		{
			printk("** ERROR **: no relay for pcm channel %d.\n", relay_id);
			
			return RELAY_FAIL;
		}
	}
	else
	{ 
		printk("virtual_daa_relay_switch: no such state\n");	
		
		return RELAY_FAIL;
	}
	
	restore_flags(flags);
}	



