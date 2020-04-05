/*
 * Flash mapping for BCM947XX boards
 *
 * Copyright (C) 2001 Broadcom Corporation
 *
 * $Id: rtl865x_flash.c,v 1.5 2008/10/22 09:41:23 tylo Exp $
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include <linux/config.h>


#define WINDOW_ADDR 0xbfc00000
#define WINDOW_SIZE 0x200000
#define BUSWIDTH 2

static struct mtd_info *rtl865x_mtd;

__u8 rtl865x_map_read8(struct map_info *map, unsigned long ofs)
{
	//printk("enter %s %d\n",__FILE__,__LINE__);
	return __raw_readb(map->map_priv_1 + ofs);
}

__u16 rtl865x_map_read16(struct map_info *map, unsigned long ofs)
{
	//printk("enter %s %d\n",__FILE__,__LINE__);
	return __raw_readw(map->map_priv_1 + ofs);
}

__u32 rtl865x_map_read32(struct map_info *map, unsigned long ofs)
{
	//printk("enter %s %d\n",__FILE__,__LINE__);
	return __raw_readl(map->map_priv_1 + ofs);
}

void rtl865x_map_copy_from(struct map_info *map, void *to, unsigned long from, ssize_t len)
{
	//printk("enter to %x from  %x len %d\n",to, map->map_priv_1+from , len);
	//11/15/05' hrchen, change the size to fit file systems block size if use different fs
	//4096 for cramfs, 1024 for squashfs
	if (from>0x10000)
	    memcpy(to, map->map_priv_1 + from, (len<=1024)?len:1024);//len);
	else
	    memcpy(to, map->map_priv_1 + from, (len<=4096)?len:4096);//len);
	//printk("enter %s %d\n", __FILE__,__LINE__);

}

void rtl865x_map_write8(struct map_info *map, __u8 d, unsigned long adr)
{
	__raw_writeb(d, map->map_priv_1 + adr);
	mb();
}

void rtl865x_map_write16(struct map_info *map, __u16 d, unsigned long adr)
{
	__raw_writew(d, map->map_priv_1 + adr);
	mb();
}

void rtl865x_map_write32(struct map_info *map, __u32 d, unsigned long adr)
{
	__raw_writel(d, map->map_priv_1 + adr);
	mb();
}

void rtl865x_map_copy_to(struct map_info *map, unsigned long to, const void *from, ssize_t len)
{
	//printk("enter %s %d\n",__FILE__,__LINE__);
	memcpy_toio(map->map_priv_1 + to, from, len);
}

struct map_info rtl865x_map = {
	name: "Physically mapped flash",
	size: WINDOW_SIZE,
	buswidth: BUSWIDTH,
	read8: rtl865x_map_read8,
	read16: rtl865x_map_read16,
	read32: rtl865x_map_read32,
	copy_from: rtl865x_map_copy_from,
	write8: rtl865x_map_write8,
	write16: rtl865x_map_write16,
	write32: rtl865x_map_write32,
	copy_to: rtl865x_map_copy_to
};

//10/17/05' hrchen, kernel is removed
static struct mtd_partition rtl865x_parts[] = {
	{ name: "boot",	offset:  0 ,size:  0x10000, mask_flags: 0 },
#ifdef CONFIG_4M_FLASH
#ifdef CONFIG_64KB_CONF_PART
	{ name: "rootfs",offset:  0x10000, size:0x3E0000,mask_flags:0},
	{ name: "config",offset: 0x3F0000, size: 0x10000,mask_flags:0},
#else
	{ name: "rootfs",offset:  0x10000, size:0x3F0000,mask_flags:0},
#endif
#else
#ifdef CONFIG_64KB_CONF_PART
	{ name: "rootfs",offset:  0x10000, size:0x1E0000,mask_flags:0},
	{ name: "config",offset: 0x1F0000, size: 0x10000,mask_flags:0},
#else
	{ name: "rootfs",offset:  0x10000, size:0x1F0000,mask_flags:0},
#endif
#endif
};


#if LINUX_VERSION_CODE < 0x20212 && defined(MODULE)
#define init_rtl865x_map init_module
#define cleanup_rtl865x_map cleanup_module
#endif

mod_init_t init_rtl865x_map(void)
{
       	printk(KERN_NOTICE "RTL flash device: %x at %x\n", WINDOW_SIZE, WINDOW_ADDR);
	rtl865x_map.map_priv_1 =WINDOW_ADDR;

	if (!rtl865x_map.map_priv_1) {
		printk("Failed to ioremap\n");
		return -EIO;
	}
#ifdef CONFIG_RTL8672_SPI_FLASH
	printk(KERN_NOTICE "RTL flash device probe spi.\n");
	rtl865x_mtd = do_map_probe("spi_probe", &rtl865x_map);
#else
       	printk(KERN_NOTICE "RTL flash device probe cfi.\n");
	rtl865x_mtd = do_map_probe("cfi_probe", &rtl865x_map);
#endif
	if (rtl865x_mtd) {
       	    printk(KERN_NOTICE "RTL flash device add map.\n");
	    rtl865x_mtd->module = THIS_MODULE;
	    add_mtd_partitions(rtl865x_mtd, rtl865x_parts, sizeof(rtl865x_parts)/sizeof(rtl865x_parts[0]));
	    return 0;
	}

       	printk(KERN_NOTICE "RTL flash device map Ok.\n");
	iounmap((void *)rtl865x_map.map_priv_1);
	return -ENXIO;
}

mod_exit_t cleanup_rtl865x_map(void)
{
	if (rtl865x_mtd) {
		del_mtd_partitions(rtl865x_mtd);
		map_destroy(rtl865x_mtd);
	}
	if (rtl865x_map.map_priv_1) {
		iounmap((void *)rtl865x_map.map_priv_1);
		rtl865x_map.map_priv_1 = 0;
	}
}

MODULE_LICENSE("GPL");
module_init(init_rtl865x_map);
module_exit(cleanup_rtl865x_map);
