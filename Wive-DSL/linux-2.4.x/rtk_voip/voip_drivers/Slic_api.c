#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include "Slic_api.h"
#include "si3210init.h"
#include "rtk_voip.h"
#include "spi.h"
#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA
#include "virtual_daa.h"
#endif
#ifdef AUDIOCODES_VOIP
#include "RTK_AC49xApi_Interface.h"
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_W682388
#include "w682388.h"
#endif
//#define PULSE_DIAL_DET

unsigned char slic_order;
static struct timer_list event_timer;
unsigned int flash_hook_time = 30; /* flash_hook_time * 10ms */
unsigned int flash_hook_min_time = 0; /* flash_hook_min_time * 10ms */
extern int hook_in(uint32 ch_id, char input);

/**** Digital gain compensation for SW DTMF detector 10log, normalized by 256 *******/
int16 tx_comp[10]={511, 456, 406, 362, 322, 287, 256, 228, 203, 181}; //-6dB to 3dB  
/************************************************************************************/

//======================= FSK CID =========================//
char fsk_cid_state[MAX_SLIC_CH_NUM]={0};                 // hc+ 1215 for FSK CID
volatile char fsk_alert_state[MAX_SLIC_CH_NUM]={0};      // hc+ 1229 for off hook FSK CID   
/* thlin: use volatitle for avoid problem in fsk_gen()-> while (fsk_alert_state[chid]){}; */
char fsk_alert_time[MAX_SLIC_CH_NUM]={0};                // sandro+ 2006/07/24 for off hook FSK CID
static char slic_choice_flag = 1;

extern int Hook_Polling_Silicon(hook_struck *hook, unsigned int flash_hook_duration);
extern unsigned char Hook_Polling_Legerity(hook_struck *hook, unsigned int flash_hook_duration);

void SLIC_Set_PCM_state(int chid, int enable)
{
#if defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210 ) || defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215 )

SetSi321xPCM(chid, enable);

#elif defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226)

Legerity_slic_set_tx_pcm(chid, enable);
Legerity_slic_set_rx_pcm(chid, enable);

#endif
}

void SLIC_Choice(unsigned char number) 
{
	if (slic_choice_flag == 1)
	{
#if defined (CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined (CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221)
		set_865x_slic_channel(number-1); /* Note: SLIC_LE88221 is 2 CH in one chip, so only use 1 set SPI interface */
#endif
	if (number > 0 && number <= SLIC_CH_NUM)
		slic_order = number;
	else
		printk("No such device of slic\n");
	}
}

void SLIC_Choice_Enable(void)
{
	slic_choice_flag = 1;
}

void SLIC_Choice_Disable(void)
{
	slic_choice_flag = 0;
}

unsigned char SLIC_Choice_Flag_Status(void)
{
	return slic_choice_flag;
}


void SLIC_reset(int CH, int codec_law, unsigned char slic_number)
{
	#if defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210 ) || defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215 )
		reset_SLIC(codec_law, slic_number);
		
	#elif defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226)
		Legerity_hard_reset();
		//Legerity_slic_init_all(slic_number);
		Legerity_GPIO_dir_set(0,1,1);
		Legerity_GPIO_dir_set(1,1,1);
		Legerity_GPIO_data(0,1,0);
		Legerity_GPIO_data(1,1,0);
		Legerity_slic_init_all(0);
		Legerity_slic_init_all(1);
	#elif defined CONFIG_RTK_VOIP_DRIVERS_SLIC_W682388
		SlicInit();
	#endif		
}	

/* mode: 0-type 1 (on-hook), 1-type 2(off-hook)*/
void CID_for_FSK_HW(unsigned int chid, char mode, unsigned char msg_type, char *str, char *str2, char *cid_name)
{
	unsigned long flags;		
#if ! defined (AUDIOCODES_VOIP)
	#if defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210 ) || defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215 )
		fsk_gen(chid, mode, msg_type, str, str2, cid_name);
	#elif defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226)
		save_flags(flags); cli();
		Legerity_FSK_CallerID_main(chid, str, str2, cid_name, mode, msg_type);//This Function only HW gen 2007-04-12
		restore_flags(flags);
	#endif		
#endif
}

void CID_for_FSK_SW(unsigned int chid, char mode, unsigned char msg_type, char *str, char *str2, char *cid_name)
{
#if ! defined (AUDIOCODES_VOIP)
	fsk_gen(chid, mode, msg_type, str, str2, cid_name);
#endif
}

void FXS_Ring(ring_struct *ring)
{
	#if defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210 ) || defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215 )
		Ring_FXS_Silicon(ring);
		
	#elif defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226)
		Legerity_FXS_ring(ring);
	
	#elif defined CONFIG_RTK_VOIP_DRIVERS_SLIC_W682388
		WINBOND_FXS_ring(ring);	
	#endif		
}	

int FXS_Check_Ring(ring_struct *ring)
{
	int ringer; //0: ring off, 1: ring on
	
	#if defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210 ) || defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215 )
		ringer = Check_Ring_FXS_Silicon(ring);
		
	#elif defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226)
		ringer = Check_Legerity_FXS_ring(ring);
	
	#elif defined CONFIG_RTK_VOIP_DRIVERS_SLIC_W682388
		ringer = Check_WINBOND_FXS_ring(ring);
			
	#endif		
	
	return ringer;	
}

void Set_SLIC_Tx_Gain(unsigned char chid, unsigned char tx_gain)
{
	#if defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210 ) || defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215 )
		SLIC_Set_Tx_Gain_Silicon(chid, tx_gain);
		
	#elif defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226)
		Legerity_TX_slic_gain(chid, tx_gain);
				
	#endif		
	
}

void Set_SLIC_Rx_Gain(unsigned char chid, unsigned char rx_gain)
{
	#if defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210 ) || defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215 )
		SLIC_Set_Rx_Gain_Silicon(chid, rx_gain);
		
	#elif defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226)
		Legerity_RX_slic_gain(chid, rx_gain);
				
	#endif		
	
}		
#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA
extern char relay_2_PSTN_flag[];
#endif

static unsigned int on_cnt[4] = {0};
static unsigned char pre_status[4]={0}, on_set[4]={0}, off_set[4]={0}, check_flash[4] = {1, 1, 1, 1};

/*
* on_cnt: count how many time "ON-HOOK" event happens.	
* on_set: if "ON-HOOK" event happens, on_set is set to 1.
* off_set: if "OFF-HOOK" event happens, off_set is set to 1.
* check_flash: if check_flash is 1, it means Hook_Polling_Silicon() need to detect FLASH HOOK event.
* pre_status: record previous hook status (only record 1: off-hook and 0: on-hook)
*/

/*

  PULSE_DIAL_PAUSE_TIME: the minimum dead time between adjacent dial pulse trains

*/
#ifdef PULSE_DIAL_DET

unsigned long connect_cnt[4];
unsigned long disconnect_cnt[4];
unsigned long pulse_cnt[4];

#define PULSE_DIAL_PAUSE_TIME 45

#endif

static unsigned int stop_poll[MAX_SLIC_CH_NUM] = {0}, stop_poll_cnt[MAX_SLIC_CH_NUM] = {0};

void SLIC_Hook_Polling(hook_struck *hook, unsigned int flash_hook_min_duration, unsigned int flash_hook_duration)
{
	
	unsigned char status;
	unsigned long flags;	
	
	save_flags(flags); cli();
		
#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA
	if (relay_2_PSTN_flag[hook->CH]==1)
	{
		status = virtual_daa_hook_detect(hook->CH); /* 1:off-hook  0:on-hook */
	}
	else
#endif
	{
	#if defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210 ) || defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215 )
	
	
		SLIC_Choice(hook->CH + 1);	 //choice slic1 or slic2. //chiminer 
		status = readDirectReg(68)&0x01; /* 1:off-hook  0:on-hook */

        if((fsk_spec_areas[hook->CH]&7)==FSK_NTT)
		{
			if(fsk_cid_state[hook->CH])     //when send caller id ignore the off hook event.
			{
				stop_poll[hook->CH] = 1;
			}

			if((stop_poll[hook->CH] == 1) && (status == 0)) //when NTT phone, off-hook -> on-hook, continue to poll.
			{
				stop_poll[hook->CH] = 0;
				stop_poll_cnt[hook->CH] = 0;
			}

			if(stop_poll[hook->CH] == 1)
			{
				status=pre_status[hook->CH];

				if (stop_poll_cnt[hook->CH]++ > 70)
				{
					stop_poll[hook->CH] = 0;
					stop_poll_cnt[hook->CH] = 0;
					PRINT_MSG("Force to start poll hook status for NTT\n");
				}
			}
		}
	
	#elif defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226)
	
		Le88xxx data;	
		slic_order = hook->CH;
		
		if ((slic_order < 0) || (slic_order > 1)) 
		{
			printk("No such device.\n");
			return 0xFF;
		}	
		readLegerityReg( 0x4F, &data);	

		if (slic_order == 0)
			status = data.byte1&0x01;
		else if (slic_order == 1)
			status = data.byte2&0x01;

        if((fsk_spec_areas[hook->CH]&7)==FSK_NTT)
		{
			if(fsk_cid_state[hook->CH])     //when send caller id ignore the off hook event.
			{
				stop_poll[hook->CH] = 1;
			}

			if((stop_poll[hook->CH] == 1) && (status == 0)) //when NTT phone, off-hook -> on-hook, continue to poll.
			{
				stop_poll[hook->CH] = 0;
			}

			if(stop_poll[hook->CH] == 1)
			{
				status=pre_status[hook->CH];

				if (stop_poll_cnt[hook->CH]++ > 70)
				{
					stop_poll[hook->CH] = 0;
					PRINT_MSG("Force to start poll hook status for NTT\n");
				}
			}
		}

	#elif defined CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
		status = iphone_hook_detect();
	#elif defined CONFIG_RTK_VOIP_DRIVERS_SLIC_W682388
		status = GetHookState(hook->CH);

        if((fsk_spec_areas[hook->CH]&7)==FSK_NTT)
		{
			if(fsk_cid_state[hook->CH])     //when send caller id ignore the off hook event.
			{
				stop_poll[hook->CH] = 1;
			}

			if((stop_poll[hook->CH] == 1) && (status == 0)) //when NTT phone, off-hook -> on-hook, continue to poll.
			{
				stop_poll[hook->CH] = 0;
			}

			if(stop_poll[hook->CH] == 1)
			{
				status=pre_status[hook->CH];

				if (stop_poll_cnt[hook->CH]++ > 70)
				{
					stop_poll[hook->CH] = 0;
					PRINT_MSG("Force to start poll hook status for NTT\n");
				}
			}
		}

	#endif	
	}
	restore_flags(flags);
	

	if(status)
	{
		/* on_cnt[] >= 10*flash_hook_min_duration ms and
		 * on_cnt[] < 10*flash_hook_duration ms,
		 * then flash event happen. 
		 */
		if (check_flash[hook->CH] == 1)
		{
			if (on_cnt[hook->CH] <= flash_hook_min_duration)
			{
				if (!(off_set[hook->CH]))
				{
					hook->hook_status = PHONE_OFF_HOOK;
					off_set[hook->CH] = 1;
					on_set[hook->CH] = 0;
					//printk("*** OFF ***\n");
				}
				else
				{
					hook->hook_status = PHONE_STILL_ON_HOOK;
					//printk("*** S-ON 1***\n");
				}
				
			}
			else if ((on_cnt[hook->CH] >= flash_hook_min_duration) && (on_cnt[hook->CH] < flash_hook_duration))
			{
				hook->hook_status = PHONE_FLASH_HOOK;
				//printk("*** FLASH ***\n");
			}
			else
			{
				if (!(off_set[hook->CH]))
				{
					hook->hook_status = PHONE_OFF_HOOK;
					off_set[hook->CH] = 1;
					on_set[hook->CH] = 0;
					//printk("*** OFF ***\n");
				}
				else
				{
					hook->hook_status = PHONE_STILL_ON_HOOK;
					//printk("*** S-ON 1***\n");
				}
				
			}
				
			check_flash[hook->CH] = 0;
		}	
		else
		{
			hook->hook_status = PHONE_STILL_OFF_HOOK;
			//printk("*** S-OFF 1***\n");
		}
			
		on_cnt[hook->CH] = 0;
#ifdef PULSE_DIAL_DET
		if (disconnect_cnt[hook->CH] != 0)
		{
			if (disconnect_cnt[hook->CH] < 8)
				pulse_cnt[hook->CH]++;
			disconnect_cnt[hook->CH]=0;
		}
		else
		{
			if (connect_cnt[hook->CH] > PULSE_DIAL_PAUSE_TIME)
			{
				if(pulse_cnt[hook->CH] != 0)
					printk("pu%d\n",pulse_cnt[hook->CH]);
				pulse_cnt[hook->CH] = 0;
			}
		}

		connect_cnt[hook->CH]++;
#endif
	}
	else
	{
		if (pre_status[hook->CH] == 1) 	/* prev = off-hook */
			check_flash[hook->CH] = 1;
		
		if (on_cnt[hook->CH] >= flash_hook_duration)
		{
			on_cnt[hook->CH] = flash_hook_duration; /* avoid on_cnt[] to overflow */
			
			if (on_set[hook->CH] == 0) 
			{
				hook->hook_status = PHONE_ON_HOOK;
				on_set[hook->CH] = 1;
				off_set[hook->CH] = 0;
				//printk("*** ON ***\n");
			}
			else
			{
				hook->hook_status = PHONE_STILL_ON_HOOK;
				//printk("*** S-ON 2***\n");
			}
		}
		else
		{
			hook->hook_status = PHONE_STILL_OFF_HOOK;
			//printk("*** S-OFF 2***\n");
		}
		
		//printk("%d\n", on_cnt[hook->CH]);
		on_cnt[hook->CH]++;
#ifdef PULSE_DIAL_DET
		disconnect_cnt[hook->CH]++;
		connect_cnt[hook->CH]=0;
#endif
	}
	
	pre_status[hook->CH] = status;
		
	return 0;
}


void Init_Hook_Polling(unsigned char CH)
{
	check_flash[CH] = 1;
	pre_status[CH] = 0; 
	on_cnt[CH] = 0;	
	on_set[CH] = 0; 
	off_set[CH] = 0;
	
}



void Event_Polling_Use_Timer(unsigned long data)
{
	int i;
        hook_struck hook_res[MAX_SLIC_CH_NUM];
	
#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA	
	virtual_daa_ring_det();		/* TH: use timer to polling daa incomimg ring */
#endif
	for (i=0; i< SLIC_CH_NUM; i++)
	{
		(&hook_res[i])->CH = i;
	
		SLIC_Hook_Polling(&hook_res[i], flash_hook_min_time, flash_hook_time);
		
		if ( (&hook_res[i])->hook_status == PHONE_ON_HOOK)
		{
			hook_in(i, PHONE_ON_HOOK);
			//printk("hook_in: ON\n");
		}
		else if( (&hook_res[i])->hook_status == PHONE_OFF_HOOK)
		{
			hook_in(i, PHONE_OFF_HOOK);
			//printk("hook_in: OFF\n");
		}
		else if( (&hook_res[i])->hook_status == PHONE_FLASH_HOOK)
		{
			hook_in(i, PHONE_FLASH_HOOK);
			//printk("hook_in: Flash\n");
		}
		else
			hook_in(i, PHONE_UNKNOWN);
	}

#ifdef AUDIOCODES_VOIP	
	/* Polling DTMF, Fax, Modem Events */
	extern int dtmf_in(uint32 ch_id, char input);
	extern int fax_modem_in(uint32 ch_id, char input);
	TeventDetectionResult event_res;
	
	event_res = RtkAc49xApiEventPolling();
	
	if (event_res.dtmf_digit != 'Z')
	{
		dtmf_in(event_res.channel, event_res.dtmf_digit);
	}
	
	if (event_res.bypass_flag == 1)
	{
		fax_modem_in(event_res.channel, event_res.bypass_flag);
	}
#endif	

	event_timer.expires = jiffies + 1;
	add_timer(&event_timer);
}

void Init_Event_Polling_Use_Timer(void)
{
	init_timer(&event_timer);
	event_timer.expires = jiffies + 1;
	//event_timer.data = 30;
	event_timer.function = Event_Polling_Use_Timer;
	add_timer(&event_timer);
	PRINT_MSG("Add Event Timer For Polling\n");
}

void Reinit_Event_Polling_Use_Timer(void)
{
	del_timer(&event_timer);
	Init_Event_Polling_Use_Timer();
}



void SLIC_Set_Ring_Cadence(unsigned char chid, unsigned short OnMsec, unsigned short OffMsec)
{
	#if defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210 ) || defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215 )
		SLIC_Set_Ring_Cadence_ON_Silicon(chid+1, OnMsec);
		SLIC_Set_Ring_Cadence_OFF_Silicon(chid+1, OffMsec);
		
	#elif defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226)
		SLIC_Set_Ring_Cadence_Legerity(chid, OnMsec, OffMsec);		
	#elif defined CONFIG_RTK_VOIP_DRIVERS_SLIC_W682388
		SetupRing(0x7E6C,0x1039,OnMsec,OffMsec,chid);
	#endif	
}

void SLIC_Set_Impendance(unsigned char chid, unsigned short country, unsigned short impd)
{
	#if defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210 ) || defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215 )
		SLIC_Set_Impendance_Silicon(chid+1, country, impd);
	#elif defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226)
		SLIC_Set_Impendance_Legerity(chid, country, impd);		
	#elif defined CONFIG_RTK_VOIP_DRIVERS_SLIC_W682388
		SetImpedance(chid, country);	//country ref:w682388.h(register 168/170 IMPED_SYNTH_CTRL)
	#endif
	PRINT_MSG("Set SLIC impedance according to the country...\n");
}

void SLIC_GenProcessTone(genTone_struct *gen_tone)
{
	#if defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210 ) || defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215 )
		GenProcessTone_Silicon(gen_tone);
		
	#elif defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226)
		//GenProcessTone_Legerity(gen_tone);
		
	#endif	
}



void OnHookLineReversal(int chid, unsigned char bReversal) //0: Forward On-Hook Transmission, 1: Reverse On-Hook Transmission
{
	#if defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210 ) || defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215 )
		OnHookLineReversal_Silicon(chid, bReversal);
	#elif defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226)
		Legerity_OnHookLineReversal(chid, bReversal);
	#elif defined CONFIG_RTK_VOIP_DRIVERS_SLIC_W682388	
		W682388_OnHookLineReversal(chid, bReversal);
	#endif
}

void SendNTTCAR(int chid)
{
	#if defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210 ) || defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215 )
		SendNTTCAR_Silicon(chid);
	#elif defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226)
		SendNTTCAR_Legerity(chid);
	#endif
}

void disableOscillators(void)
{
	#if defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210 ) || defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215 )
		disableOscillators_Silicon();
	#elif defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226)
		//disableOscillators_Legerity();
	#endif
}

#if defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210 ) || defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215 )
	static char cid_reg64, cid_reg64_prev;
#elif defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226)
	static Le88xxx data;
#endif

void SetOnHookTransmissionAndBackupRegister(int chid) // use for DTMF caller id
{
#if defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210 ) || defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215 )

	cid_reg64 = readDirectReg(64);

	if ( (cid_reg64 & 0x07) != 2 )  // force for DTMF CID display
	{
		cid_reg64_prev = cid_reg64; // record it
		PRINT_MSG("Reg64 = 0x%02x\n", cid_reg64);
		OnHookLineReversal(chid, 0); //Forward On-Hook Transmission
	}

#elif defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226)

	readLegerityReg( 0x56, &data);	//back up
	OnHookLineReversal(chid, 0); //Forward On-Hook Transmission

#endif
}

void RestoreBackupRegisterWhenSetOnHookTransmission(void) // use for DTMF caller id
{
#if defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210 ) || defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215 )

	writeDirectReg(64,cid_reg64_prev);

#elif defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226)

	writeLegerityReg( 0x56, &data);	// restore back-up value

#endif
}

#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221
#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88226
unsigned char Legerity_system_state(unsigned char slic_id, unsigned char state, unsigned char wri_re)
{
	return 0;
}
#endif
#endif

#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210
#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215
void fskInitialization (void) 
{
}
#endif
#endif

