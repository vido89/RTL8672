/*
 *  arch/mips/philips/nino/prom.c
 *
 *  Copyright (C) 2001 Steven J. Hill (sjhill@realitydiluted.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *  
 *  Early initialization code for the Philips Nino
 */
#include <linux/config.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <asm/bootinfo.h>
#include <asm/addrspace.h>
#include <asm/page.h>
#include <asm/system.h>

char arcs_cmdline[CL_SIZE];

#ifdef CONFIG_FB_TX3912
extern unsigned long tx3912fb_paddr;
extern unsigned long tx3912fb_vaddr;
extern unsigned long tx3912fb_size;
#endif

const char *get_system_type(void)
{
	return "Realtek RTL867x";
}

unsigned short totall_system_memory_detected;
unsigned long detect_ram_sequence[4];

/* Do basic initialization */
void __init prom_init(int argc, char **argv, unsigned long magic, int *prom_vec)
{
	unsigned long mem_size,reg_mem;
	int cnt=1;
        unsigned long mempos;
	unsigned long memmeg;
	unsigned short save_dword;
	unsigned long flags;

#ifdef CONFIG_MTD
    //10/17/05' hrchen, move cramfs to mtdblock1
	strcpy(arcs_cmdline, "nosmp ramdisk_size=0 reserve=0 root=/dev/mtdblock1");
#endif
	mips_machgroup = MACH_GROUP_LEXRA;
	mips_machtype = MACH_UNKNOWN;
	save_flags(flags);cli(); 
//Maximum RAM 
#ifdef CONFIG_8M_SDRAM
	reg_mem = 8; 
#else
	reg_mem = 64; 
#endif
	detect_ram_sequence[0] = reg_mem;
	detect_ram_sequence[1] = reg_mem;
//Test to be sure in RAM capacity
	for(memmeg=4;memmeg<reg_mem;memmeg+=4){
	
	    mempos = 0xa0000000L + memmeg * 0x100000;
	           
	    save_dword = *(volatile unsigned short *)mempos;
	    
	    *(volatile unsigned short *)mempos = (unsigned short)0xABCD;
	    
	    if (*(volatile unsigned short *)mempos != (unsigned short)0xABCD){
		*(volatile unsigned short *)mempos = save_dword;
		break;
	    } 
	    
	    *(volatile unsigned short *)mempos = (unsigned short)0xDCBA;
	    
	    if (*(volatile unsigned short *)mempos != (unsigned short)0xDCBA){
		*(volatile unsigned short *)mempos = save_dword;
		break;
	    } 
	    
	    *(volatile unsigned short *)mempos = save_dword;
	}

#ifdef CONFIG_8M_SDRAM
	printk("Static 8MB RAM SIZE \n");
	memmeg = 8;
#else
	printk("Auto detect RAM SIZE \n");
#endif	
	mem_size = memmeg << 20;

#ifndef CONFIG_8M_SDRAM
	//workaround for non correct memory sizes detect for some devices
	if (reg_mem != memmeg) 
	    {
	    printk("Workaround for: \n");
	    //workaround for 16bit 16Mbyte RAM devices
	    if (reg_mem == 32 && memmeg == 20) 
		{
		printk("Bad 16RAM 16Bit devices \n");
		reg_mem  = 16;
		memmeg 	 = 16;
		mem_size = 16777216;
		}
	    }
#endif
	detect_ram_sequence[2] = memmeg;
	detect_ram_sequence[3] = mem_size;
	
       restore_flags(flags); 
       totall_system_memory_detected = memmeg;
       add_memory_region(0, mem_size, BOOT_MEM_RAM); 
}

void __init prom_free_prom_memory (void)
{
}
