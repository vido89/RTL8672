/*
 *  arch/mips/realtek/rtl8670/pci.c
 * 
 * Enumerate and enable PCI devices
 *
 * Note: Due to PCI problem that two slot enabled will bring us the abnormal 
 *	 content of PCI configuration space, we enable only one device on 
 * 	 slot 0 right now.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * History:
 * 2007/06/05 SH	Modify to support only one PCI device on slot 0.
 *
 */ 
 
#include <linux/config.h>
#include <linux/ioport.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/paccess.h>
#include <asm/pci_channel.h>

#include "pci.h"

#if defined(CONFIG_RTL8670)
#include "lx4180.h"
#else // 8671
#include "lx5280.h"
#endif

int rtl8671_isLinuxIncompliantEndianMode=1;

#undef DEBUG
//#define DEBUG	
#ifdef DEBUG
#define DBG(x...) printk(x)
#else
#define DBG(x...)
#endif

/*
 * RTL8671 supports two PCI slots 
 */
#define PCI_SLOT_NUMBER 1 // Only support one device now

typedef struct BASE_ADDRESS_STRUCT{
  u32 Address;
  enum type_s {
    IO_SPACE=0x80,
    MEM_SPACE
  }Type;
  enum width_s {
    WIDTH32,
    WIDTH64
  }Width;
  u32 Prefetch;
  u32 Size;
}base_address_s;

typedef struct PCI_CONFIG_SPACE_STRUCT{
  u32 Config_Base;  //config space base address
  enum status_s{
    ENABLE,
    DISABLE
  }Status;  //device is enable or disable
  u32 Vendor_Device_ID;
  u8  Revision_ID;
  u32 Class_Code;
  u8  Header_Type;
  base_address_s BAR[6];
  u32 SubSystemVendor_ID;
}pci_config_s;


static pci_config_s *pci_slot[PCI_SLOT_NUMBER][8];

static u32 rtlpci_read_config_endian_free(u32 address)
{		
	if(!rtl8671_isLinuxIncompliantEndianMode)
	{
		return le32_to_cpu(REG32(address));		
	}
	return REG32(address);
}

static void rtlpci_write_config_endian_free(u32 address,u32 value)
{	

	if(!rtl8671_isLinuxIncompliantEndianMode)
		REG32(address)=le32_to_cpu(value);	
	else
		REG32(address)=value;
}


/* 
 * pci_base_address_fix_up:
 *    Make sure the base address is correct before PCI driver scan bus.
 *    Called by driver/pci.c pci_do_scan_bus().
 */
void pci_base_address_fix_up(void)
{
	unsigned int i,j,k;
	for(i=0;i<PCI_SLOT_NUMBER;i++) {
		for(j=0;j<8;j++) {
			for(k=0;k<6;k++) {
				if(pci_slot[i][j]==NULL)
					continue;
				if(pci_slot[i][j]->BAR[k].Address!=0){
DBG("%s: write %x to 0x%x\n", __func__, (pci_slot[i][j]->BAR[k].Address)&0x1fffffff, (pci_slot[i][j]->Config_Base+PCI_CONFIG_BASE0+(4*k)));
					REG32((pci_slot[i][j]->Config_Base+PCI_CONFIG_BASE0+(4*k))) = (pci_slot[i][j]->BAR[k].Address)&0x1fffffff;
					
				}					
			}
		}
	}
}

/* scan_resource: 
 *   Scan the resource needed by PCI device including mapping method, 
 *   base address and mapping range.
 */
void scan_resource(pci_config_s *pci_config, int slot_num, int dev_function, u32 config_base)
{
    int i;
    u32 BaseAddr, data;
		
    BaseAddr = config_base+PCI_CONFIG_BASE0;	

    for (i=0;i<6;i++) {  //detect resource usage
    	rtlpci_write_config_endian_free(BaseAddr,0xffffffff);
    	data = rtlpci_read_config_endian_free(BaseAddr);
    	    	
    	if (data!=0) {  //resource request exist
    	    int j;
    	    if (data&1) {  //IO space
    	    
    	    	pci_config->BAR[i].Type = IO_SPACE;
    	    	    	    	
  	        //scan resource size
			pci_config->BAR[i].Size=((~(data&PCI_BASE_ADDRESS_IO_MASK))+1)&0xffff;
  	        printk("IO Space %i, data=0x%x size=0x%x \n",i,data,pci_config->BAR[i].Size);
  	  
    	    } else {  //Memory space
    	    	
    	    	pci_config->BAR[i].Type = MEM_SPACE;
    	    	//bus width
    	    	if ((data&0x0006)==4) pci_config->BAR[i].Width = WIDTH64; //bus width 64
    	    	else pci_config->BAR[i].Width = WIDTH32;  //bus width 32
    	    	//prefetchable
    	    	if (data&0x0008) pci_config->BAR[i].Prefetch = 1; //prefetchable
    	    	else pci_config->BAR[i].Prefetch = 0;  //no prefetch
  	        //scan resource size
  	        if (pci_config->BAR[i].Width==WIDTH32) {
  	          for (j=4;j<32;j++)
  	            if (data&(1<<j)) break;
  	          if (j<32) pci_config->BAR[i].Size = 1<<j;
  	          else  pci_config->BAR[i].Size = 0;  	          
  	        } else //width64 is not support
  	          {
  	          	pci_config->BAR[i].Size = 0;
  	          }
  	        printk("Memory Space %i data=0x%x size=0x%x\n",i,data,pci_config->BAR[i].Size);	
    	    }
    	} else {  //no resource
    	    memset(&(pci_config->BAR[i]), 0, sizeof(base_address_s));    		
    	}
	BaseAddr += 4;  //next base address
    }
    
}

/* 
 * rtl_pci_scan_slot:
 *   scan the PCI bus, save config information into pci_slot,
 *   return the number of PCI functions 
 */
static int rtl_pci_scan_slot(void)
{
	u32 config_base, data, vendor_device;
	int i, dev_function;
	int dev_num = 0;
	u16 device,vendor,last_ok_func;

	for ( i=0 ; i<PCI_SLOT_NUMBER ; i++ )
	{  //probe only one pci slots

		/* PCI timing issue work around  */
   	   	config_base = 0;
   	   	while(config_base < 3000000)	{
   	   		config_base++;
   	   	}   

		switch(i)
		{
			case 0:      	   	       	   	
			config_base = PCI_SLOT0_CONFIG_BASE;           
			break;

			case 1:
			config_base = PCI_SLOT1_CONFIG_BASE;
			break;

			default:
				return 0;
		}

		dev_function=0;

		vendor_device = rtlpci_read_config_endian_free(config_base+PCI_CONFIG_VENDER_DEV);

		vendor=vendor_device&0xffff;
		device=vendor_device>>16;

		last_ok_func=0;

		while (dev_function<8)
		{  //pci device exist

			{
				/* support following vendor list */
				switch(vendor)
				{
					case 0x17a0: //Genesys
						if(!((dev_function==0)||(dev_function==3)) )
						{
							goto next_func;
						}
						printk("Found Genesys USB 2.0 PCI Card!, function=%d!\n",dev_function);
						//REG32(config_base+0x50)&=(0xffffffef);
						break;

					case PCI_VENDOR_ID_AL: //ALi
						if(dev_function>3)
						{
							goto next_func;
						}
						printk("Found ALi USB 2.0 PCI Card, function=%d!\n",dev_function);			      	
						break;
		
					case PCI_VENDOR_ID_VIA: //0x1106
						if(dev_function>2)
						{
							goto next_func;
						}
						printk("Found VIA USB 2.0 PCI Card!, function=%d!\n",dev_function);			      	
						break;

					case PCI_VENDOR_ID_PHILIPS: //0x1106
						if(dev_function>2)
						{
							goto next_func;
						}
						printk("Found PHILIPS USB 2.0 PCI Card!, function=%d!\n",dev_function);			      	
						break;

					case PCI_VENDOR_ID_NEC: //0x1033
						if(dev_function>2)
						{
							goto next_func;
						}
						printk("Found NEC USB 2.0 PCI Card, function=%d!\n",dev_function);			      	
						break;

					case PCI_VENDOR_ID_REALTEK:
						if(dev_function>0) ////assume realtek's chip only have one function.
						{
							if((device==0x8139)||(device==0x8185)||(device==0x8188))
							{
								goto next_func;
							}
						}

						if((device==0x8139)||(device==0x8185)||(device==0x8188))
							printk("Found Realtek %x PCI Card, function=%d!\n",device,dev_function);
						break;

					case 0x168c: // Atheros
						if(dev_function>0) //assume atheros's chip only have one function.
						{
							//if(device==0x0013)
							{
								goto next_func;
							}
						}
						if(device==0x0013)			
							printk("Found Atheros AR5212 802.11a/b/g PCI Card, function=%d!\n",dev_function);
						else if(device==0x0012)
							printk("Found Atheros AR5211 802.11a/b PCI Card, function=%d!\n",dev_function);				
						else
							printk("Found Atheros unknow PCI Card, function=%d!\n",dev_function);				
						break;
					case 0x13f6: // C-media
						REG32(0xbc805038)|=0x80000000;	//enable 16bit mode
					case 0x17cb: // Airgo
						if(dev_function>0) 
						{
							goto next_func;
						}
						break;

					case PCI_VENDOR_ID_PROMISE: //0x105a
						if(dev_function>0)
						{
next_func:
							dev_function++;
							config_base += 256;
							continue;
						}
						printk("Found Promise IDE PCI Card, function=%d!\n",dev_function);
						break;

					default:
						printk("Error device and vendor ID, go on the next slot PCI scan\n");
						goto error_device;
						break;
				}
			}

			if (	(0==rtlpci_read_config_endian_free(config_base+PCI_CONFIG_VENDER_DEV)) ||
				((rtlpci_read_config_endian_free(config_base+PCI_CONFIG_VENDER_DEV)&0xffff)==0xffff))
			{
				dev_function++;
				config_base += 256;  //next function's config base
				continue;
			}



			pci_slot[i][dev_function] = kmalloc(sizeof(pci_config_s),GFP_KERNEL);
			pci_slot[i][dev_function]->Config_Base = config_base;
			pci_slot[i][dev_function]->Status = DISABLE;
			pci_slot[i][dev_function]->Vendor_Device_ID = rtlpci_read_config_endian_free(config_base+PCI_CONFIG_VENDER_DEV);
			data = rtlpci_read_config_endian_free(config_base+PCI_CONFIG_CLASS_REVISION);
			pci_slot[i][dev_function]->Revision_ID = data&0x00FF;
			pci_slot[i][dev_function]->Class_Code = data>>8;
			pci_slot[i][dev_function]->Header_Type = (rtlpci_read_config_endian_free(config_base+PCI_CONFIG_CACHE)>>16)&0x000000FF;
			pci_slot[i][dev_function]->SubSystemVendor_ID = rtlpci_read_config_endian_free(config_base+PCI_CONFIG_SUBSYSTEMVENDOR);
			scan_resource(pci_slot[i][dev_function], i, dev_function, config_base);  //probe resource request

			printk("PCI device exists: slot %d function %d VendorID %x DeviceID %x %x\n", i, dev_function,           	
			pci_slot[i][dev_function]->Vendor_Device_ID&0x0000FFFF,
			pci_slot[i][dev_function]->Vendor_Device_ID>>16,config_base);
			dev_num++;
			dev_function++;
			////config_base += dev_function*64*4;  //next function's config base
			config_base += 256;  //next function's config base
//			if (!(pci_slot[i][0]->Header_Type&0x80)) break;  //single function card
			if (dev_function>=8) break;  //only 8 functions allow in a PCI card  
		}      
error_device:
#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)
		while(0);
#endif
	}	

    return dev_num;
}

/* sort resource by its size */
void bubble_sort(int space_size[PCI_SLOT_NUMBER*8*6][2], int num)
{
  int i, j;
  int tmp_swap[2];
  
    for (i=0;i<num-1;i++) {
    	for (j=i;j<num-1;j++) {
          if (space_size[j][1]<space_size[j+1][1]) {
            tmp_swap[0] = space_size[j][0];
            tmp_swap[1] = space_size[j][1];
            space_size[j][0] = space_size[j+1][0];
            space_size[j][1] = space_size[j+1][1];
            space_size[j+1][0] = tmp_swap[0];
            space_size[j+1][1] = tmp_swap[1];
          }
        }
    }
}


/*
 * PCI software reset
 */
static void rtl_pci_reset(void){
#if 0 // Two PCI devices can not work together right now
	/* Due to NXP USB host only works on PCI clock 31Mhz~33Mhz */
	*((volatile unsigned long *) 0xb9c04000) = 0x38063a29;	// config PCI clock to 31.14MHZ
	mdelay(10);
	*((volatile unsigned long *) 0xb9c04000) = 0xb8063a29;
	printk("Reset PCI ..\n");
#else
        *((volatile unsigned long *) RT8670_SICR_BASE) &= ~(1<<31);
        mdelay(10);
        *((volatile unsigned long *) RT8670_SICR_BASE) |= (1<<31);
        printk("Reset PCI ..0605(Only PCI slot0 enable)\n");
#endif
}

static int rtlpci_read_config_byte(struct pci_dev *dev, int where, u8 *value)
{
	u32 tmp;
	u32 addr;
	u32 shift=3-(where & 0x3);	
	

	if(PCI_FUNC(dev->devfn) >= 8) return PCIBIOS_FUNC_NOT_SUPPORTED;
	if(PCI_SLOT(dev->devfn) >= PCI_SLOT_NUMBER) return PCIBIOS_FUNC_NOT_SUPPORTED;

	if(pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)] == NULL)
		return PCIBIOS_FUNC_NOT_SUPPORTED;

	if(pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)]->Status != ENABLE)
		return PCIBIOS_FUNC_NOT_SUPPORTED;
			
	addr = pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)]->Config_Base;
	
	if(addr == 0) return PCIBIOS_FUNC_NOT_SUPPORTED;
	
	if (dev->bus->number != 0)
	{
		*value = 0;
		return PCIBIOS_SUCCESSFUL;
	}

	

	if(rtl8671_isLinuxIncompliantEndianMode)
	{
		addr |= (where&~0x3);
		tmp = REG32(addr);
		DBG("%s devfn %x addr %x value %x\n", __func__, dev->devfn, addr, tmp);					
		for (addr=0;addr<(3-shift);addr++)
			tmp = tmp >>8;
		*value = (u8)tmp;
	}
	else
	{
		//*value = REG8(addr+where);
		addr |= (where&0xfffffffc);
		tmp = REG32(addr);
		DBG("%s devfn %x addr %x value %x\n", __func__, dev->devfn, addr, tmp);			
		for (addr=0;addr<shift;addr++)
			tmp = tmp >>8;
		*value = (u8)tmp;		
	}

	return PCIBIOS_SUCCESSFUL;
}

static int rtlpci_read_config_word(struct pci_dev *dev, int where, u16 *value)
{
	
	u32 tmp,addr;

	if(PCI_FUNC(dev->devfn) >= 8) return PCIBIOS_FUNC_NOT_SUPPORTED;
	if(PCI_SLOT(dev->devfn) >= PCI_SLOT_NUMBER) return PCIBIOS_FUNC_NOT_SUPPORTED;

	if(pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)] == NULL) 
		return PCIBIOS_FUNC_NOT_SUPPORTED;

	if(pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)]->Status != ENABLE)
		return PCIBIOS_FUNC_NOT_SUPPORTED;

	addr = pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)]->Config_Base;
	
	if(addr == 0) return PCIBIOS_FUNC_NOT_SUPPORTED;

	if (dev->bus->number != 0)
	{
		*value = 0;
		return PCIBIOS_SUCCESSFUL;
	}

	addr |= (where&~0x3);	

	if(rtl8671_isLinuxIncompliantEndianMode)
	{
		tmp=REG32(addr);
		DBG("%s devfn %x addr %x value %x\n", __func__, dev->devfn, addr, tmp);					
		if (where&0x2)
			*value = (u16)(tmp>>16);
		else
			*value = (u16)(tmp);
	}
	else
	{
		tmp=REG32(addr);
		DBG("%s devfn %x addr %x value %x\n", __func__, dev->devfn, addr, tmp);					
		if(where&0x2)
			*value=le16_to_cpu((u16)(tmp));
		else
			*value=le16_to_cpu((u16)(tmp>>16));
	}

	return PCIBIOS_SUCCESSFUL;
}

static int rtlpci_read_config_dword(struct pci_dev *dev, int where, u32 *value)
{
	u32 addr;


	if(PCI_FUNC(dev->devfn) >= 8) return PCIBIOS_FUNC_NOT_SUPPORTED;

	if(PCI_SLOT(dev->devfn) >= PCI_SLOT_NUMBER) return PCIBIOS_FUNC_NOT_SUPPORTED;

	if(pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)] == NULL)
		return PCIBIOS_FUNC_NOT_SUPPORTED;
		
	if(pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)]->Status != ENABLE)
		return PCIBIOS_FUNC_NOT_SUPPORTED;		

	addr = pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)]->Config_Base;
	

	if(addr == 0) return PCIBIOS_FUNC_NOT_SUPPORTED;
		
	if (dev->bus->number != 0)
	{
		*value=0;
		return PCIBIOS_SUCCESSFUL;
	}

	if(rtl8671_isLinuxIncompliantEndianMode)
	{
		DBG("%s devfn %x addr %x value %x \n", __func__, dev->devfn, addr+where, REG32(addr+where));				
		*value = REG32(addr+where);		
	}
	else
	{
		DBG("%s devfn %x addr %x value %x\n", __func__, dev->devfn, addr+where, le32_to_cpu(REG32(addr+where)));					
		*value = le32_to_cpu(REG32(addr+where));		
	}
	return PCIBIOS_SUCCESSFUL;
}


static int rtlpci_write_config_byte(struct pci_dev *dev, int where, u8 value)
{
	u32 tmp;
	u32 addr;
	u32 shift=(where & 0x3);	
	
	
	if(PCI_FUNC(dev->devfn) >= 8) return PCIBIOS_FUNC_NOT_SUPPORTED;
	if(PCI_SLOT(dev->devfn) >= PCI_SLOT_NUMBER) return PCIBIOS_FUNC_NOT_SUPPORTED;
		
	if(pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)] == NULL)
		return PCIBIOS_FUNC_NOT_SUPPORTED;
				
	if(pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)]->Status != ENABLE)
		return PCIBIOS_FUNC_NOT_SUPPORTED;				
				
	addr = pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)]->Config_Base;
	if(addr == 0) return PCIBIOS_FUNC_NOT_SUPPORTED;

	if (dev->bus->number != 0)		
		return PCIBIOS_SUCCESSFUL;
	

	
	if(rtl8671_isLinuxIncompliantEndianMode)
	{
		u32 newval=(u32)value,mask=0xff;
		addr |= (where&0xfffffffc);
		DBG("%s devfn %x addr %x value %x\n", __func__, dev->devfn, addr, newval);
		tmp = REG32(addr);
		newval <<= (8*(shift));
		mask <<= (8*(shift));
		REG32(addr)=(tmp&(~mask))|newval;	

	}
	else
	{
		//*value = REG8(addr+where);
		u32 newval=(u32)value,mask=0xff;
		addr |= (where&0xfffffffc);
		DBG("%s devfn %x addr %x value %x\n", __func__, dev->devfn, addr, newval);		
		tmp = REG32(addr);
		newval <<= (8*(3-shift));
		mask <<= (8*(3-shift));
		REG32(addr)=(tmp&(~mask))|newval;
	}

	return PCIBIOS_SUCCESSFUL;
}

static int rtlpci_write_config_word(struct pci_dev *dev, int where, u16 value)
{
	u32 addr;
	
	if(PCI_FUNC(dev->devfn) >= 8) return PCIBIOS_FUNC_NOT_SUPPORTED;
	if(PCI_SLOT(dev->devfn) >= PCI_SLOT_NUMBER) return PCIBIOS_FUNC_NOT_SUPPORTED;	
	
	if(pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)] == NULL)
		return PCIBIOS_FUNC_NOT_SUPPORTED;

	if(pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)]->Status != ENABLE)
		return PCIBIOS_FUNC_NOT_SUPPORTED;
				
	addr = pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)]->Config_Base;
	
	if(addr == 0) return PCIBIOS_FUNC_NOT_SUPPORTED;
	
	if (dev->bus->number != 0)		
		return PCIBIOS_SUCCESSFUL;

	if(rtl8671_isLinuxIncompliantEndianMode)
	{
		u32 tmp;
		addr |= (where&0xfffffffc);
		DBG("%s devfn %x addr %x value %x\n", __func__, dev->devfn, addr, value);		
		tmp = REG32(addr);
		if(where&2)			
			REG32(addr)=(tmp&0xffff)|(((u32)(value)<<16));			
		else
			REG32(addr)=(tmp&0xffff0000)|((u32)(value));
	}
	else
	{
		u32 tmp;
		addr |= (where&0xfffffffc);
		DBG("%s devfn %x addr %x value %x\n", __func__, dev->devfn, addr, value);		
		tmp = REG32(addr);
		if(where&2)			
			REG32(addr)=(tmp&0xffff0000)|((u32)(le16_to_cpu(value)));			
		else
			REG32(addr)=(tmp&0xffff)|((u32)((le16_to_cpu(value))<<16));
			
	}	
	return PCIBIOS_SUCCESSFUL;
}

static int rtlpci_write_config_dword(struct pci_dev *dev, int where, u32 value)
{
	u32 addr;

	if(PCI_FUNC(dev->devfn) >= 8) return PCIBIOS_FUNC_NOT_SUPPORTED;
	if(PCI_SLOT(dev->devfn) >= PCI_SLOT_NUMBER) return PCIBIOS_FUNC_NOT_SUPPORTED;
	
	if(pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)] == NULL)
		return PCIBIOS_FUNC_NOT_SUPPORTED;
		
	if(pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)]->Status != ENABLE)
		return PCIBIOS_FUNC_NOT_SUPPORTED;		
				
	addr = pci_slot[PCI_SLOT(dev->devfn)][PCI_FUNC(dev->devfn)]->Config_Base;
	
	if(addr == 0) return PCIBIOS_FUNC_NOT_SUPPORTED;
	
	if (dev->bus->number != 0)		
		return PCIBIOS_SUCCESSFUL;


	if(rtl8671_isLinuxIncompliantEndianMode)
	{
		DBG("%s devfn %x addr %x value %x\n", __func__, dev->devfn, addr+where, value);	
		REG32(addr+where)=value;		
	}
	else
	{
		DBG("%s devfn %x addr %x value %x\n", __func__, dev->devfn, addr+where, le32_to_cpu(value));		
		REG32(addr+where)=le32_to_cpu(value);
	}

	return PCIBIOS_SUCCESSFUL;
}


static struct pci_ops pcibios_ops = {
	rtlpci_read_config_byte,
	rtlpci_read_config_word,
	rtlpci_read_config_dword,
	rtlpci_write_config_byte,
	rtlpci_write_config_word,
	rtlpci_write_config_dword
};


#define PCI_IO_START      0x1d200000
#define PCI_IO_END        0x1d2fffff
#define PCI_MEM_START     0x1d300000
#define PCI_MEM_END       0x1d3fffff




static struct resource pci_io_resource = {
	"pci IO space", 
	PCI_IO_START,
	PCI_IO_END,
	IORESOURCE_IO
};

static struct resource pci_mem_resource = {
	"pci memory space", 
	PCI_MEM_START,
	PCI_MEM_END,
	IORESOURCE_MEM
};


struct pci_channel mips_pci_channels[] = {
	{&pcibios_ops, &pci_io_resource, &pci_mem_resource, 0, 1},
	{(struct pci_ops *) NULL, (struct resource *) NULL,
	 (struct resource *) NULL, (int) NULL, (int) NULL}
};


int pcibios_enable_resources(struct pci_dev *dev)
{
	return 0;
}

unsigned int pcibios_assign_all_busses(void)
{
	return 0;
}


void __init pcibios_fixup_resources(struct pci_dev *dev)
{
	/* nothing to do here */
}



void __init pcibios_fixup(void)
{
	/* nothing to do here */
}

void __init pcibios_fixup_irqs(void)
{
    struct pci_dev *dev;
#if 1
	pci_for_each_dev(dev) {
		dev->irq = 7; // fix irq
	}
#endif
}

/*
 * rtl_pci_assign_resource:	
 *   Assign memory location to MEM & IO space
 *
 *   return 0:OK, else:fail 
 */
static int rtl_pci_assign_resource(void)
{
  	int i, slot, func, BARnum;
  	int mem_space_size[PCI_SLOT_NUMBER*8*6][2]; //[0]:store device index, [1]:store resource size
  	int io_space_size[PCI_SLOT_NUMBER*8*6][2]; //[0]:store device index, [1]:store resource size
  	int mem_idx, io_idx, total_size, tmp;
  	u16 config_command[PCI_SLOT_NUMBER][8];  	//record config space resource usage, 
  
	memset(mem_space_size, 0, sizeof(mem_space_size));
	memset(io_space_size, 0, sizeof(io_space_size));
	memset(config_command, 0, sizeof(config_command));
    
	//collect resource
	mem_idx = io_idx =0;
	for (slot=0;slot<PCI_SLOT_NUMBER;slot++)
	{
    	
		if (pci_slot[slot][0]==0) continue;  //this slot is null      
		if (pci_slot[slot][0]->Vendor_Device_ID==0) continue;  //this slot is null      

		for (func=0;func<8;func++)
		{
			if (pci_slot[slot][func]==0) continue;

			pci_slot[slot][func]->Status = ENABLE;

			for (BARnum=0;BARnum<6;BARnum++)
			{
				if (pci_slot[slot][func]->BAR[BARnum].Type==MEM_SPACE)
				{  //memory space
					printk("memory mapping BAnum=%d slot=%d func=%d\n",BARnum,slot,func);
					config_command[slot][func] |= CMD_MEM_SPACE;  //this device use Memory
					mem_space_size[mem_idx][0] = (slot<<16)|(func<<8)|(BARnum<<0);
					mem_space_size[mem_idx][1] = pci_slot[slot][func]->BAR[BARnum].Size;
					mem_idx++;
				} else if (pci_slot[slot][func]->BAR[BARnum].Type==IO_SPACE)
				{  //io space
					printk("io mapping BAnum=%d slot=%d func=%d\n",BARnum,slot,func);
					config_command[slot][func] |= CMD_IO_SPACE;  //this device use IO
					io_space_size[io_idx][0] = (slot<<16)|(func<<8)|(BARnum<<0);
					io_space_size[io_idx][1] = pci_slot[slot][func]->BAR[BARnum].Size;
					io_idx++;
				}
	      			else
	      			{
//					printk("unknow mapping BAnum=%d slot=%d func=%d\n",BARnum,slot,func);
				}
			}  //for (BARnum=0;BARnum<6;BARnum++) 
		}  //for (func=0;func<8;func++)
	} //for (slot=0;slot<PCI_SLOT_NUMBER;slot++)

    //sort by size
    if (mem_idx>1) bubble_sort(mem_space_size, mem_idx);
    if (io_idx>1)  bubble_sort(io_space_size, io_idx);   
    
    //check mem total size
    total_size = 0;
    for (i=0;i<mem_idx;i++) {  	
    	tmp = mem_space_size[i][1]-1;
        total_size = (total_size+tmp)&(~tmp);     
        total_size = total_size + mem_space_size[i][1];
    }
    if (total_size>PCI_MEM_SPACE_SIZE) {
	printk("lack of memory map space resource \n");
	return -1;  //lack of memory space resource
    }

    //check io total size
    total_size = 0;
    for (i=0;i<io_idx;i++) {     	
    	tmp = io_space_size[i][1]-1;
        total_size = (total_size+tmp)&(~tmp);     
        total_size = total_size + io_space_size[i][1];
    }
    
    if (total_size>PCI_IO_SPACE_SIZE) 
    {
	printk("lack of io map space resource\n");
		return -2;  //lack of io space resource
    }
    

    //assign memory space
    total_size = 0;
    for (i=0;i<mem_idx;i++) {
    	unsigned int config_base;     	
    	tmp = mem_space_size[i][1]-1;
       total_size = (total_size+tmp)&(~tmp);        
       tmp = mem_space_size[i][0];
		
        //assign to struct
        if (pci_slot[(tmp>>16)][(tmp>>8)&0x00FF]->BAR[tmp&0x00FF].Type!= MEM_SPACE)
        	continue;
        
        pci_slot[(tmp>>16)][(tmp>>8)&0x00FF]->BAR[tmp&0x00FF].Address = PCI_SLOT_MEM_BASE+total_size;
     	switch((tmp>>16)){
     	case 0:
     		config_base = PCI_SLOT0_CONFIG_BASE;
     		break;
     	case 1:
     		config_base = PCI_SLOT1_CONFIG_BASE;
     		break;
     	default:
     		panic("PCI slot assign error");
    	}        
        //get BAR address and assign to PCI device
        tmp = config_base   //SLOT
              +((tmp>>8)&0x00FF)*64*4					//function
              +((tmp&0x00FF)*4+PCI_CONFIG_BASE0);			//BAR bumber

        rtlpci_write_config_endian_free(tmp,(PCI_SLOT_MEM_BASE+total_size)&0x1FFFFFFF);
        printk("assign mem base %x~%x at %x size=%d\n",(PCI_SLOT_MEM_BASE+total_size)&0x1FFFFFFF,((PCI_SLOT_MEM_BASE+total_size)&0x1FFFFFFF)+mem_space_size[i][1]-1,tmp,mem_space_size[i][1]);
	
        total_size = total_size + mem_space_size[i][1];  //next address
    }
    //assign IO space
    total_size = 0;
    for (i=0;i<io_idx;i++) {
    	unsigned int config_base;    	
    	tmp = io_space_size[i][1]-1;
        total_size = (total_size+tmp)&(~tmp);        
        tmp = io_space_size[i][0];
        //assign to struct
        if (pci_slot[(tmp>>16)][(tmp>>8)&0x00FF]->BAR[tmp&0x00FF].Type!= IO_SPACE)
        	continue;

        pci_slot[(tmp>>16)][(tmp>>8)&0x00FF]->BAR[tmp&0x00FF].Address = PCI_SLOT_IO_BASE+total_size;
        //get BAR address and assign to PCI device
     	switch((tmp>>16)){
     	case 0:
     		config_base = PCI_SLOT0_CONFIG_BASE;
     		break;
     	case 1:
     		config_base = PCI_SLOT1_CONFIG_BASE;
     		break;
     	default:
     		panic("PCI slot assign error");
    	}
        tmp = config_base   //SLOT
              +((tmp>>8)&0x00FF)*64*4					//function
              +((tmp&0x00FF)*4+PCI_CONFIG_BASE0);			//BAR bumber
        
	 rtlpci_write_config_endian_free(tmp,(PCI_SLOT_IO_BASE+total_size)&0x1FFFFFFF);
	 
	printk("assign I/O base %x~%x at %x size=%d\n",(PCI_SLOT_IO_BASE+total_size)&0x1FFFFFFF,((PCI_SLOT_IO_BASE+total_size)&0x1FFFFFFF)+io_space_size[i][1]-1,tmp,io_space_size[i][1]);
	total_size += io_space_size[i][1];  //next address	
       
        
    }
    
    //enable device
    for (slot=0;slot<PCI_SLOT_NUMBER;slot++) {
    	
      if (pci_slot[slot][0]==0) continue;  //this slot is null      
      if (pci_slot[slot][0]->Vendor_Device_ID==0) continue;  //this slot is null      
      for (func=0;func<8;func++) {
      	unsigned int config_base;
        if (pci_slot[slot][func]==0) continue;  //this slot:function is null      
        if (pci_slot[slot][func]->Vendor_Device_ID==0) continue;  //this slot:function is null      
        //get config base address
     	switch(slot){
     	case 0:
     		config_base = PCI_SLOT0_CONFIG_BASE;
     		break;
     	case 1:
     		config_base = PCI_SLOT1_CONFIG_BASE;
     		break;
     	default:
     		panic("PCI slot assign error");
    	}        
        
        tmp = config_base   //SLOT
              +func*64*4;   //function

        //printk("1 REG32(tmp+PCI_CONFIG_INT_LINE) = %x\n",REG32(tmp+PCI_CONFIG_INT_LINE));

	//enable cache line size, lantancy
	//printk("(REG32(tmp+PCI_CONFIG_CACHE)&0xFFFF0000)|0x2004=%x\n",(REG32(tmp+PCI_CONFIG_CACHE)&0xFFFF0000)|0x2004);
	rtlpci_write_config_endian_free(tmp+PCI_CONFIG_CACHE,(rtlpci_read_config_endian_free(tmp+PCI_CONFIG_CACHE)&0xFFFF0000)|0x2004);

	
  	// work around pci configuration timing issue
       	config_base = 0;	       	
       	while(config_base < 1000){
       	config_base++;
       	} 

	//Execute PCI commands
	if(!rtl8671_isLinuxIncompliantEndianMode){
		u32 command_status=rtlpci_read_config_endian_free(tmp+PCI_CONFIG_COMMAND);
		command_status=(config_command[slot][func]|CMD_BUS_MASTER|CMD_PARITY_ERROR_RESPONSE)|(command_status&0xffff0000);
		rtlpci_write_config_endian_free(tmp+PCI_CONFIG_COMMAND,command_status);		
	}
	else
	{
       	REG16(tmp+PCI_CONFIG_COMMAND) = config_command[slot][func]
        	|CMD_BUS_MASTER|CMD_PARITY_ERROR_RESPONSE;//|CMD_WRITE_AND_INVALIDATE;
	}


      }
    }

    return 0;
}

/* 
 * rtl_pci_init: 
 *   Called by drivers/pci.c.
 *   Scan pci buses, assign Memory & IO space to PCI card, 
 *   and enable the PCI devices
 *   rtn 0:ok, else:fail 
 */
int rtl_pci_init(void)
{
  	int function_num;

	rtl_pci_reset();
	    
	memset(pci_slot, 0, 4*PCI_SLOT_NUMBER*8);

	function_num = rtl_pci_scan_slot();

    if (function_num==0) {
    	printk("No PCI device exist!!\n");
    	return -1;
    }
	
    //auto assign resource
    if (rtl_pci_assign_resource()) {
    	printk("PCI Resource assignment failed!\n");
    	return -2;
    }

    printk("Find Total %d PCI functions\n", function_num);
    return 0;
      
}

