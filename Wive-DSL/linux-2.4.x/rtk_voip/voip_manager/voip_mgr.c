#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/if_ether.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/spinlock.h>
#include <asm/unaligned.h>
#include <linux/netfilter.h>

#include "../include/type.h"
#include "../voip_rx/rtk_trap.h"
#include "voip_feature.h"



extern struct nf_sockopt_ops voip_mgr_sockopts;
extern int dsp_init_first;

#define USE_DTMF_FIFO

#ifndef USE_DTMF_FIFO
static char latest_input[4]={'Z', 'Z', 'Z', 'Z'};

int dtmf_in(uint32 ch_id, char input)
{
	if(input != 'Z') { //slince
	  latest_input[ch_id] = input;	
	  //printk("k:%c ", latest_input[ch_id]);
	}
	return 0;
}

int dtmf_out(uint32 ch_id, char * output)
{
	*output = latest_input[ch_id];	
	latest_input[ch_id] = 'Z';
	return 0;
}
#else  // kenny: DTMF FIFO
#define DTMF_PCM_FIFO_SIZE 10
static char latest_input[4][DTMF_PCM_FIFO_SIZE]={ {'Z', 'Z', 'Z', 'Z', 'Z', 'Z', 'Z', 'Z', 'Z', 'Z'},{'Z', 'Z', 'Z', 'Z', 'Z', 'Z', 'Z', 'Z', 'Z', 'Z'}, {'Z', 'Z', 'Z', 'Z', 'Z', 'Z', 'Z', 'Z', 'Z', 'Z'}, {'Z', 'Z', 'Z', 'Z', 'Z', 'Z', 'Z', 'Z', 'Z', 'Z'}};
static char dtmf_wp[4]={0,0,0,0},dtmf_rp[4]={0,0,0,0};

int dtmf_in(uint32 ch_id, char input)
{
	if(input != 'Z') { //slince
                if ((dtmf_wp[ch_id]+1)%DTMF_PCM_FIFO_SIZE != dtmf_rp[ch_id]) {
	  		latest_input[ch_id][dtmf_wp[ch_id]] = input;
                        dtmf_wp[ch_id] = (dtmf_wp[ch_id]+1) % DTMF_PCM_FIFO_SIZE;   
	  	} else { // FIFI overflow
	  		printk("********* DTMF FIFO overflow ********\n");
	  	}
		//printk("k:%c ", latest_input[ch_id]);
	}
	return 0;
}

int dtmf_out(uint32 ch_id, char * output)
{
	if ( dtmf_wp[ch_id] == dtmf_rp[ch_id]) { 
		// FIFO empty
		*output = 'Z';
	} else {
		*output = latest_input[ch_id][dtmf_rp[ch_id]];		
                dtmf_rp[ch_id] = (dtmf_rp[ch_id]+1) % DTMF_PCM_FIFO_SIZE;   
	}
//	latest_input[ch_id] = 'Z';
	return 0;
}

int flush_dtmf_fifo(uint32 ch_id)
{
	int i;
	for (i=0; i< DTMF_PCM_FIFO_SIZE; i++)
		latest_input[ch_id][i] = 0;
}
#endif


#define USE_HOOK_FIFO

#ifdef USE_HOOK_FIFO
#define HOOK_PCM_FIFO_SIZE 10
static char hook_fifo[MAX_SLIC_CH_NUM][HOOK_PCM_FIFO_SIZE] = {0};
static char hook_wp[MAX_SLIC_CH_NUM]={0}, hook_rp[MAX_SLIC_CH_NUM]={0};
static char prev_out[MAX_SLIC_CH_NUM]={0};

int hook_in(uint32 ch_id, char input)
{
	if(input != 5)	/* not equal to PHONE_UNKNOWN, then put in to fifo */
	{ 
                if ((hook_wp[ch_id]+1)%HOOK_PCM_FIFO_SIZE != hook_rp[ch_id]) 
		{
	  		hook_fifo[ch_id][hook_wp[ch_id]] = input;
                        hook_wp[ch_id] = (hook_wp[ch_id]+1) % HOOK_PCM_FIFO_SIZE;
	  		//printk("hook_wp=%d\n", hook_wp[ch_id]);	
	  	}
	  	else 
	  	{ 
	  		printk("********* HOOK FIFO overflow ********\n");
	  	}
		//printk("k:%c ", hook_fifo[ch_id]);
	}
	return 0;
}

int hook_out(uint32 ch_id, char * output)
{
	if ( hook_wp[ch_id] == hook_rp[ch_id]) // FIFO empty
	{ 
		if (prev_out[ch_id] == 0 )	/* PHONE_ON_HOOK */
			*output = 3;		/* PHONE_STILL_ON_HOOK */
		else if (prev_out[ch_id] == 2 ) /* PHONE_FLASH_HOOK */
			*output = 4;		/* PHONE_STILL_OFF_HOOK */
		else if (prev_out[ch_id] == 1 )	/* PHONE_OFF_HOOK */
			*output = 4;		/* PHONE_STILL_OFF_HOOK */
	} 
	else 
	{
		*output = hook_fifo[ch_id][hook_rp[ch_id]];
		prev_out[ch_id] = hook_fifo[ch_id][hook_rp[ch_id]];
                hook_rp[ch_id] = (hook_rp[ch_id]+1) % HOOK_PCM_FIFO_SIZE;           
		//printk("hook_rp=%d\n", hook_rp[ch_id]);	
	}
//	hook_fifo[ch_id] = 'Z';
	return 0;
}

int flush_hook_fifo(uint32 ch_id)
{
	int i;
	for (i=0; i< HOOK_PCM_FIFO_SIZE; i++)
		hook_fifo[ch_id][i] = 0;
}
#endif


#define ENERGY_FIFO_SIZE 10
static char eng_fifo[PCM_CH_NUM][ENERGY_FIFO_SIZE]={0};
static char eng_wp[PCM_CH_NUM]={0},eng_rp[PCM_CH_NUM]={0};

int energy_in(uint32 ch_id, char input)
{

	if ((eng_wp[ch_id]+1)%ENERGY_FIFO_SIZE != eng_rp[ch_id]) 
	{
		eng_fifo[ch_id][eng_wp[ch_id]] = input;
		eng_wp[ch_id] = (eng_wp[ch_id]+1) % ENERGY_FIFO_SIZE;   
	} 
	else
	{
		printk("********* ENERGY FIFO overflow ********\n");
	}
		//printk("k:%c ", latest_input[ch_id]);

	return 0;
}

int energy_out(uint32 ch_id)
{
	int output;
	
	if ( eng_wp[ch_id] == eng_rp[ch_id])
	{ 
		// FIFO empty
		return -1;
	} 
	else 
	{
		output = eng_fifo[ch_id][eng_rp[ch_id]];		
                eng_rp[ch_id] = (eng_rp[ch_id]+1) % ENERGY_FIFO_SIZE;   
                
                return output;
	}
}

#ifdef AUDIOCODES_VOIP
#define FAX_MODEM_PCM_FIFO_SIZE 5
static char fax_modem_fifo[4][FAX_MODEM_PCM_FIFO_SIZE] = {0};
static char fax_modem_wp[4]={0,0,0,0}, fax_modem_rp[4]={0,0,0,0};

int fax_modem_in(uint32 ch_id, char input)
{
	if(input == 1)
	{ 
                if ((fax_modem_wp[ch_id]+1)%FAX_MODEM_PCM_FIFO_SIZE != fax_modem_rp[ch_id]) 
		{
	  		fax_modem_fifo[ch_id][fax_modem_wp[ch_id]] = input;
                        fax_modem_wp[ch_id] = (fax_modem_wp[ch_id]+1) % FAX_MODEM_PCM_FIFO_SIZE;
	  		//printk("fax_modem_wp=%d\n", fax_modem_wp[ch_id]);	
	  	}
	  	else 
	  	{ 
	  		printk("********* FAX/MODEM FIFO overflow ********\n");
	  	}
		//printk("k:%c ", fax_modem_fifo[ch_id]);
	}
	return 0;
}

int fax_modem_out(uint32 ch_id)
{
	int output;
	
	if ( fax_modem_wp[ch_id] == fax_modem_rp[ch_id]) // FIFO empty
	{ 
		output = 0;
		//printk("output = %d\n", output);
	} 
	else 
	{
		output = fax_modem_fifo[ch_id][fax_modem_rp[ch_id]];
                fax_modem_rp[ch_id] = (fax_modem_rp[ch_id]+1) % FAX_MODEM_PCM_FIFO_SIZE;           
		//printk("fax_modem_rp=%d\n", fax_modem_rp[ch_id]);	
		//printk("output = %d\n", output);
	}
	return output;
}

int flush_fax_modem_fifo(uint32 ch_id)
{
	int i;
	for (i=0; i< FAX_MODEM_PCM_FIFO_SIZE; i++)
		fax_modem_fifo[ch_id][i] = 0;
}
#endif

/****************************************************************/
static int __init rtk_voip_mgr_init_module(void)
{
  printk("=============RTK VOIP SUITE=============\n"); 
  printk("INITIAL VOIP MANAGER PROGRAM\n");
  //printk("===========================================\n");     
  if(nf_register_sockopt(&voip_mgr_sockopts))
   {
    printk(" voip_mgr cannot register sockopt \n");
    return -1;
   }
   extern unsigned int gVoipFeature;
   gVoipFeature = RTK_VOIP_FEATURE;
   BOOT_MSG("Get RTK VoIP Feature.\n");
#ifdef CONFIG_RTK_VOIP_MODULE
	pcmctrl_init();
	rtk_trap_init_module();	
#endif  
#if ! defined (AUDIOCODES_VOIP) 
  rtk_voip_dsp_init();
  dsp_init_first = 1;
#endif

  return 0;
} 

/****************************************************************/
static void __exit rtk_voip_mgr_cleanup_module(void)
{
  printk("=============RTK VOIP SUITE============\n"); 
  printk("Remove VOIP MANAGER PROGRAM\n");
  printk("===========================================\n");     
  nf_unregister_sockopt(&voip_mgr_sockopts);
#if ! defined (AUDIOCODES_VOIP)
  rtk_voip_dsp_exit();
#endif
#ifdef CONFIG_RTK_VOIP_MODULE
	rtk_trap_cleanup_module();
	pcmctrl_cleanup();  
#endif
}

/****************************************************************/
module_init(rtk_voip_mgr_init_module);
module_exit(rtk_voip_mgr_cleanup_module);

