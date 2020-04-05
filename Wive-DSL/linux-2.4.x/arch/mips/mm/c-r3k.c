/*
 * r2300.c: R2000 and R3000 specific mmu/cache code.
 *
 * Copyright (C) 1996 David S. Miller (dm@engr.sgi.com)
 *
 * with a lot of changes to make this thing work for R3000s
 * Tx39XX R4k style caches added. HK
 * Copyright (C) 1998, 1999, 2000 Harald Koerfgen
 * Copyright (C) 1998 Gleb Raiko & Vladimir Roganov
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>

#include <asm/page.h>
#include <asm/pgtable.h>
#include <asm/mmu_context.h>
#include <asm/system.h>
#include <asm/isadep.h>
#include <asm/io.h>
#include <asm/wbflush.h>
#include <asm/bootinfo.h>
#include <asm/cpu.h>

static unsigned long icache_size, dcache_size;		/* Size in bytes */
static unsigned long icache_lsize, dcache_lsize;	/* Size in bytes */

#undef DEBUG_CACHE
#ifdef CONFIG_RTL8672
#define read_c0_status()	__read_32bit_c0_register($12, 0)
#define __read_32bit_c0_register(source, sel)				\
({ int __res;								\
	if (sel == 0)							\
		__asm__ __volatile__(					\
			"mfc0\t%0, " #source "\n\t"			\
			: "=r" (__res));				\
	else								\
		__asm__ __volatile__(					\
			".set\tmips32\n\t"				\
			"mfc0\t%0, " #source ", " #sel "\n\t"		\
			".set\tmips0\n\t"				\
			: "=r" (__res));				\
	__res;								\
})
#define write_c0_status(val)	__write_32bit_c0_register($12, 0, val)
#define __write_32bit_c0_register(register, sel, value)			\
do {									\
	if (sel == 0)							\
		__asm__ __volatile__(					\
			"mtc0\t%z0, " #register "\n\t"			\
			: : "Jr" ((unsigned int)(value)));		\
	else								\
		__asm__ __volatile__(					\
			".set\tmips32\n\t"				\
			"mtc0\t%z0, " #register ", " #sel "\n\t"	\
			".set\tmips0"					\
			: : "Jr" ((unsigned int)(value)));		\
} while (0)

#endif
unsigned long __init r3k_cache_size(unsigned long ca_flags)
{
#if defined(CONFIG_RTL8672)
    if( (ca_flags&(ST0_ISC|ST0_SWC)== ST0_ISC))
            return 8<<10;
    else
            return 16<<10;
#else
	unsigned long flags, status, dummy, size;
	volatile unsigned long *p;

	return 0;
	p = (volatile unsigned long *) KSEG0;

	flags = read_32bit_cp0_register(CP0_STATUS);

	/* isolate cache space */
	write_32bit_cp0_register(CP0_STATUS, (ca_flags|flags)&~ST0_IEC);

	*p = 0xa5a55a5a;
	dummy = *p;
	status = read_32bit_cp0_register(CP0_STATUS);

	if (dummy != 0xa5a55a5a || (status & ST0_CM)) {
		size = 0;
	} else {
		for (size = 128; size <= 0x40000; size <<= 1)
			*(p + size) = 0;
		*p = -1;
		for (size = 128; 
		     (size <= 0x40000) && (*(p + size) == 0); 
		     size <<= 1)
			;
		if (size > 0x40000)
			size = 0;
	}

	write_32bit_cp0_register(CP0_STATUS, flags);

	return size * sizeof(*p);
#endif
}

unsigned long __init r3k_cache_lsize(unsigned long ca_flags)
{
	unsigned long flags, status, lsize, i;
	volatile unsigned long *p;
#if defined(CONFIG_RTL8672)

    return 16;
#else
	return 0;
	p = (volatile unsigned long *) KSEG0;

	flags = read_32bit_cp0_register(CP0_STATUS);

	/* isolate cache space */
	write_32bit_cp0_register(CP0_STATUS, (ca_flags|flags)&~ST0_IEC);

	for (i = 0; i < 128; i++)
		*(p + i) = 0;
	*(volatile unsigned char *)p = 0;
	for (lsize = 1; lsize < 128; lsize <<= 1) {
		*(p + lsize);
		status = read_32bit_cp0_register(CP0_STATUS);
		if (!(status & ST0_CM))
			break;
	}
	for (i = 0; i < 128; i += lsize)
		*(volatile unsigned char *)(p + i) = 0;

	write_32bit_cp0_register(CP0_STATUS, flags);

	return lsize * sizeof(*p);
#endif    
}

static void __init r3k_probe_cache(void)
{
#if defined(CONFIG_RTL8672)
	dcache_size = 8 << 10;
	dcache_lsize = 16;
	icache_size = 16 << 10;
	icache_lsize = 16;
#else
	dcache_size = 4*1024;
	dcache_lsize = 16;
	icache_size = 4*1024;
	icache_lsize = 16;
#endif
}

static void r3k_flush_icache_range(unsigned long start, unsigned long end)
{
#if defined(CONFIG_RTL8672)
	int flags = read_32bit_cp0_register(CP0_STATUS);
	write_32bit_cp0_register(CP0_STATUS, (flags)&~ST0_IEC);
#endif
	/*Invalidate I-Cache*/
	__asm__ volatile(
		"mtc0 $0,$20\n\t"
		"nop\n\t"
		"li $8,2\n\t"
		"mtc0 $8,$20\n\t"
		"nop\n\t"
		"nop\n\t"
		"mtc0 $0,$20\n\t"
		"nop"
		: /* no output */
		: /* no input */
			);
#if defined(CONFIG_RTL8672)
	write_32bit_cp0_register(CP0_STATUS, (flags)&~ST0_IEC);
#endif
#if 0
	lx4180_writeCacheCtrl(0);
	lx4180_writeCacheCtrl(2);
	lx4180_writeCacheCtrl(0);

	unsigned long size, i, flags;
	volatile unsigned char *p;


	//rupert
    volatile unsigned int reg;
    save_flags(flags);cli();
    reg=read_c0_xcontext();
    __asm__ volatile("nop");
    __asm__ volatile("nop");
    write_c0_xcontext((reg | 0x2)); //wirte '1' to bit 0, 1
    __asm__ volatile("nop");
    __asm__ volatile("nop");
    write_c0_xcontext((reg &(~0x2))); //write '0' to bit 0,1
    __asm__ volatile("nop");
    __asm__ volatile("nop");
    write_c0_xcontext((reg | 0x2)); //wirte '1' to bit 0, 1
    __asm__ volatile("nop");
    __asm__ volatile("nop");
    restore_flags(flags);
	return;




	size = end - start;
	if (size > icache_size || KSEGX(start) != KSEG0) {
		start = KSEG0;
		size = icache_size;
	}
	p = (char *)start;

	flags = read_32bit_cp0_register(CP0_STATUS);

	/* isolate cache space */
	write_32bit_cp0_register(CP0_STATUS, (ST0_ISC|ST0_SWC|flags)&~ST0_IEC);

	for (i = 0; i < size; i += 0x080) {
		asm ( 	"sb\t$0, 0x000(%0)\n\t"
			"sb\t$0, 0x004(%0)\n\t"
			"sb\t$0, 0x008(%0)\n\t"
			"sb\t$0, 0x00c(%0)\n\t"
			"sb\t$0, 0x010(%0)\n\t"
			"sb\t$0, 0x014(%0)\n\t"
			"sb\t$0, 0x018(%0)\n\t"
			"sb\t$0, 0x01c(%0)\n\t"
		 	"sb\t$0, 0x020(%0)\n\t"
			"sb\t$0, 0x024(%0)\n\t"
			"sb\t$0, 0x028(%0)\n\t"
			"sb\t$0, 0x02c(%0)\n\t"
			"sb\t$0, 0x030(%0)\n\t"
			"sb\t$0, 0x034(%0)\n\t"
			"sb\t$0, 0x038(%0)\n\t"
			"sb\t$0, 0x03c(%0)\n\t"
			"sb\t$0, 0x040(%0)\n\t"
			"sb\t$0, 0x044(%0)\n\t"
			"sb\t$0, 0x048(%0)\n\t"
			"sb\t$0, 0x04c(%0)\n\t"
			"sb\t$0, 0x050(%0)\n\t"
			"sb\t$0, 0x054(%0)\n\t"
			"sb\t$0, 0x058(%0)\n\t"
			"sb\t$0, 0x05c(%0)\n\t"
		 	"sb\t$0, 0x060(%0)\n\t"
			"sb\t$0, 0x064(%0)\n\t"
			"sb\t$0, 0x068(%0)\n\t"
			"sb\t$0, 0x06c(%0)\n\t"
			"sb\t$0, 0x070(%0)\n\t"
			"sb\t$0, 0x074(%0)\n\t"
			"sb\t$0, 0x078(%0)\n\t"
			"sb\t$0, 0x07c(%0)\n\t"
			: : "r" (p) );
		p += 0x080;
	}

	write_32bit_cp0_register(CP0_STATUS, flags);
#endif	
}

#ifdef CONFIG_RTL8672
static void r3k_flush_dcache_range(unsigned long start, unsigned long end)
{
	/*Invalidate D-Cache*/
#if defined(CONFIG_RTL8672)

#if 1
	/* Flush D-Cache using its range */
	unsigned char *p;
	unsigned int size;
	unsigned int flags;
	unsigned int i;

	size = end - start;
	size += 16; //tylo, flush one more cache line for CPU write-buffer issue

	/* correctness check : flush all if any parameter is illegal */
	if (	( KSEGX(start) == KUSEG ) ||
		( size >= dcache_size )		)
	{
		/*
		 *	ghhuang
		 *		=> For Realtek Lextra CPU,
		 *		   the cache would NOT be flushed only if the Address to-be-flushed
		 *		   is the EXPLICIT address ( which is really stored in that cache line ).
		 *		   For the aliasd addresses, the cache entry would NOT be flushed even
		 *		   it matchs same cache-index.
		 *
		 *		   => This is different from traditional MIPS-based CPU's configuration.
		 *		      So if we want to flush ALL-cache entries, we would need to use "mtc0"
		 *		      instruction instead of simply modifying the "size" to "dcache_size"
		 *		      and "start" to "KSEG0".
		 *
		 */
		flags = read_c0_status();

		/* isolate cache space */
		write_c0_status( (flags) &~ ST0_IEC );
		__asm__ volatile(
			"mtc0 $0,$20\n\t"
			"nop\n\t"
			"li $8,512\n\t"
			"mtc0 $8,$20\n\t"
			"nop\n\t"
			"nop\n\t"
			"mtc0 $0,$20\n\t"
			"nop"
			: /* no output */
			: /* no input */
				);
		write_c0_status(flags);
	} else
	{
		/* Start to isolate cache space */
		p = (char *)(PHYSADDR(start) | K0BASE);

		flags = read_c0_status();

		/* isolate cache space */
		write_c0_status( (flags) &~ ST0_IEC );

		for (i = 0; i < size; i += 0x040)
		{
			asm volatile ( 	
				"cache 0x15, 0x000(%0)\n\t"
			 	"cache 0x15, 0x010(%0)\n\t"
			 	"cache 0x15, 0x020(%0)\n\t"
			 	"cache 0x15, 0x030(%0)\n\t"
				:		/* No output registers */
				:"r"(p)		/* input : 'p' as %0 */
				);
			p += 0x040;
		}
		write_c0_status(flags);
	}
#else
	__asm__ volatile(
		"mtc0 $0,$20\n\t"
		"nop\n\t"
		"li $8,512\n\t"
		"mtc0 $8,$20\n\t"
		"nop\n\t"
		"nop\n\t"
		"mtc0 $0,$20\n\t"
		"nop"
		: /* no output */
		: /* no input */
			);
#endif

#if 0 /* Reset Prefetch Buffer If Necessary */
   *(volatile unsigned int *) 0xB8001010 = (*(volatile unsigned int *) 0xB8001010) | 0x10000;
#endif

#else
	__asm__ volatile(
		"mtc0 $0,$20\n\t"
		"nop\n\t"
		"li $8,1\n\t"
		"mtc0 $8,$20\n\t"
		"nop\n\t"
		"nop\n\t"
		"mtc0 $0,$20\n\t"
		"nop"
		: /* no output */
		: /* no input */
			);
#endif

}

#else
static void r3k_flush_dcache_range(unsigned long start, unsigned long end)
{
	/*Invalidate I-Cache*/
#ifdef CONFIG_CPU_LX5280
	__asm__ volatile(
		"mtc0 $0,$20\n\t"
		"nop\n\t"
		"li $8,1\n\t"
		"mtc0 $8,$20\n\t"
		"nop\n\t"
		"nop\n\t"
		"mtc0 $0,$20\n\t"
		"nop"
		: /* no output */
		: /* no input */
			);
#else
	__asm__ volatile(
		"mtc0 $0,$20\n\t"
		"nop\n\t"
		"li $8,512\n\t"
		"mtc0 $8,$20\n\t"
		"nop\n\t"
		"nop\n\t"
		"mtc0 $0,$20\n\t"
		"nop"
		: /* no output */
		: /* no input */
			);
#endif

#if 0
	/*Invalidate D-Cache*/
	lx4180_writeCacheCtrl(0);
	lx4180_writeCacheCtrl(1);
	lx4180_writeCacheCtrl(0);
	return;

	unsigned long size, i, flags;
	volatile unsigned char *p;
    volatile unsigned int reg;
    save_flags(flags);cli();
    reg=read_c0_xcontext();
    __asm__ volatile("nop");
    __asm__ volatile("nop");
    __asm__ volatile("nop");
    write_c0_xcontext((reg & (~0x1))); //write '0' to bit 0,1
    __asm__ volatile("nop");
    write_c0_xcontext((reg | (0x1))); //write '0' to bit 0,1
    __asm__ volatile("nop");
    write_c0_xcontext((reg & (~0x1))); //write '0' to bit 0,1
    __asm__ volatile("nop");
    restore_flags(flags);
	return;




	size = end - start;
	if (size > dcache_size || KSEGX(start) != KSEG0) {
		start = KSEG0;
		size = dcache_size;
	}
	p = (char *)start;

	flags = read_32bit_cp0_register(CP0_STATUS);

	/* isolate cache space */
	write_32bit_cp0_register(CP0_STATUS, (ST0_ISC|flags)&~ST0_IEC);

	for (i = 0; i < size; i += 0x080) {
		asm ( 	"sb\t$0, 0x000(%0)\n\t"
			"sb\t$0, 0x004(%0)\n\t"
			"sb\t$0, 0x008(%0)\n\t"
			"sb\t$0, 0x00c(%0)\n\t"
		 	"sb\t$0, 0x010(%0)\n\t"
			"sb\t$0, 0x014(%0)\n\t"
			"sb\t$0, 0x018(%0)\n\t"
			"sb\t$0, 0x01c(%0)\n\t"
		 	"sb\t$0, 0x020(%0)\n\t"
			"sb\t$0, 0x024(%0)\n\t"
			"sb\t$0, 0x028(%0)\n\t"
			"sb\t$0, 0x02c(%0)\n\t"
		 	"sb\t$0, 0x030(%0)\n\t"
			"sb\t$0, 0x034(%0)\n\t"
			"sb\t$0, 0x038(%0)\n\t"
			"sb\t$0, 0x03c(%0)\n\t"
		 	"sb\t$0, 0x040(%0)\n\t"
			"sb\t$0, 0x044(%0)\n\t"
			"sb\t$0, 0x048(%0)\n\t"
			"sb\t$0, 0x04c(%0)\n\t"
		 	"sb\t$0, 0x050(%0)\n\t"
			"sb\t$0, 0x054(%0)\n\t"
			"sb\t$0, 0x058(%0)\n\t"
			"sb\t$0, 0x05c(%0)\n\t"
		 	"sb\t$0, 0x060(%0)\n\t"
			"sb\t$0, 0x064(%0)\n\t"
			"sb\t$0, 0x068(%0)\n\t"
			"sb\t$0, 0x06c(%0)\n\t"
		 	"sb\t$0, 0x070(%0)\n\t"
			"sb\t$0, 0x074(%0)\n\t"
			"sb\t$0, 0x078(%0)\n\t"
			"sb\t$0, 0x07c(%0)\n\t"
			: : "r" (p) );
		p += 0x080;
	}

	write_32bit_cp0_register(CP0_STATUS,flags);
#endif	
}
#endif

static inline unsigned long get_phys_page (unsigned long addr,
					   struct mm_struct *mm)
{
	pgd_t *pgd;
	pmd_t *pmd;
	pte_t *pte;
	unsigned long physpage;

	pgd = pgd_offset(mm, addr);
	pmd = pmd_offset(pgd, addr);
	pte = pte_offset(pmd, addr);

	if ((physpage = pte_val(*pte)) & _PAGE_VALID)
		return KSEG0ADDR(physpage & PAGE_MASK);

	return 0;
}

static inline void r3k_flush_cache_all(void)
{
}
 
static inline void r3k___flush_cache_all(void)
{
	r3k_flush_dcache_range(KSEG0, KSEG0 + dcache_size);
	r3k_flush_icache_range(KSEG0, KSEG0 + icache_size);
}

static void r3k_flush_cache_mm(struct mm_struct *mm)
{
}

static void r3k_flush_cache_range(struct mm_struct *mm, unsigned long start,
				  unsigned long end)
{
}

static void r3k_flush_cache_page(struct vm_area_struct *vma,
				   unsigned long page)
{
}

static void r3k_flush_page_to_ram(struct page * page)
{
	/*
	 * Nothing to be done
	 */
}

static void r3k_flush_icache_page(struct vm_area_struct *vma, struct page *page)
{
	struct mm_struct *mm = vma->vm_mm;
	unsigned long physpage;

	if (mm->context == 0)
		return;

	if (!(vma->vm_flags & VM_EXEC))
		return;

#ifdef DEBUG_CACHE
	printk("cpage[%d,%08lx]", (int)mm->context, page);
#endif

	physpage = (unsigned long) page_address(page);
	if (physpage)
		r3k_flush_icache_range(physpage, physpage + PAGE_SIZE);
}

static void r3k_flush_cache_sigtramp(unsigned long addr)
{
	unsigned long flags;

	save_and_cli(flags);
    
	r3k___flush_cache_all();
	restore_flags(flags);
	return;


#ifdef DEBUG_CACHE
	printk("csigtramp[%08lx]", addr);
#endif

	flags = read_32bit_cp0_register(CP0_STATUS);

	write_32bit_cp0_register(CP0_STATUS, flags&~ST0_IEC);

	/* Fill the TLB to avoid an exception with caches isolated. */
	asm ( 	"lw\t$0, 0x000(%0)\n\t"
		"lw\t$0, 0x004(%0)\n\t"
		: : "r" (addr) );

	write_32bit_cp0_register(CP0_STATUS, (ST0_ISC|ST0_SWC|flags)&~ST0_IEC);

	asm ( 	"sb\t$0, 0x000(%0)\n\t"
		"sb\t$0, 0x004(%0)\n\t"
		: : "r" (addr) );

	write_32bit_cp0_register(CP0_STATUS, flags);
}

static void r3k_dma_cache_wback_inv(unsigned long start, unsigned long size)
{
	wbflush();
	r3k_flush_dcache_range(start, start + size);
}

void __init ld_mmu_r23000(void)
{
	unsigned long config;

	_clear_page = r3k_clear_page;
	_copy_page = r3k_copy_page;

	r3k_probe_cache();
	r3k_flush_icache_range(0,0);

	_flush_cache_all = r3k_flush_cache_all;
	___flush_cache_all = r3k___flush_cache_all;
	_flush_cache_mm = r3k_flush_cache_mm;
	_flush_cache_range = r3k_flush_cache_range;
	_flush_cache_page = r3k_flush_cache_page;
	_flush_cache_sigtramp = r3k_flush_cache_sigtramp;
	_flush_page_to_ram = r3k_flush_page_to_ram;
	_flush_icache_page = r3k_flush_icache_page;
	_flush_icache_range = r3k_flush_icache_range;

	_dma_cache_wback_inv = r3k_dma_cache_wback_inv;

	printk("Primary instruction cache %dkb, linesize %d bytes\n",
		(int) (icache_size >> 10), (int) icache_lsize);
	printk("Primary data cache %dkb, linesize %d bytes\n",
		(int) (dcache_size >> 10), (int) dcache_lsize);

}
