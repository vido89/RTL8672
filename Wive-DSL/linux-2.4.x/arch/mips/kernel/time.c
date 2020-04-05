/*
 * Copyright 2001 MontaVista Software Inc.
 * Author: Jun Sun, jsun@mvista.com or jsun@junsun.net
 *
 * Common time service routines for MIPS machines. See 
 * Documents/MIPS/README.txt. 
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */
#include <linux/config.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/param.h>
#include <linux/time.h>
#include <linux/smp.h>
#include <linux/kernel_stat.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>

#include <asm/bootinfo.h>
#include <asm/compiler.h>
#include <asm/cpu.h>
#include <asm/time.h>
#include <asm/hardirq.h>
#include <asm/div64.h>

/* This is for machines which generate the exact clock. */
#define USECS_PER_JIFFY (1000000/HZ)
#define USECS_PER_JIFFY_FRAC ((u32)((1000000ULL << 32) / HZ))

/*
 * forward reference
 */
extern rwlock_t xtime_lock;
extern volatile unsigned long wall_jiffies;
static unsigned long dummy_gettimeoffset(void)
{
	return 0;
}
unsigned long (*gettimeoffset)(void) = dummy_gettimeoffset;

/*
 * By default we provide the null RTC ops
 */
static unsigned long null_rtc_get_time(void)
{
	return mktime(2000, 1, 1, 0, 0, 0);
}


/*
 * timeofday services, for syscalls.
 */
void do_gettimeofday(struct timeval *tv)
{
	unsigned long flags;
	unsigned long usec, sec;

	read_lock_irqsave(&xtime_lock, flags);
	usec = gettimeoffset();
	{
		unsigned long lost = jiffies - wall_jiffies;

		if (lost)
			usec += lost * USECS_PER_JIFFY;
	}
	sec = xtime.tv_sec;
	usec += xtime.tv_usec;
	read_unlock_irqrestore(&xtime_lock, flags);

	/* usec may have gone up a lot: be safe */
	while (usec >= 1000000) {
		usec -= 1000000;
		sec++;
	}

	tv->tv_sec = sec;
	tv->tv_usec = usec;

}

void do_settimeofday(struct timeval *tv)
{
	write_lock_irq(&xtime_lock);
	/* This is revolting. We need to set the xtime.tv_usec
	 * correctly. However, the value in this location is
	 * is value at the last tick.
	 * Discover what correction gettimeofday
	 * would have done, and then undo it!
	 */
	tv->tv_usec -= gettimeoffset();
	tv->tv_usec -= (jiffies - wall_jiffies) * USECS_PER_JIFFY;

	while (tv->tv_usec < 0) {
		tv->tv_usec += 1000000;
		tv->tv_sec--;
	}

	xtime = *tv;
	time_adjust = 0;		/* stop active adjtime() */
	time_status |= STA_UNSYNC;
	time_maxerror = NTP_PHASE_LIMIT;
	time_esterror = NTP_PHASE_LIMIT;
	write_unlock_irq(&xtime_lock);

}


/*
 * Gettimeoffset routines.  These routines returns the time duration
 * since last timer interrupt in usecs.
 *
 * If the exact CPU counter frequency is known, use fixed_rate_gettimeoffset.
 * Otherwise use calibrate_gettimeoffset()
 *
 * If the CPU does not have counter register all, you can either supply
 * your own gettimeoffset() routine, or use null_gettimeoffset() routines,
 * which gives the same resolution as HZ.
 */


/* This is for machines which generate the exact clock. */
#define USECS_PER_JIFFY (1000000/HZ)

/* usecs per counter cycle, shifted to left by 32 bits */
static unsigned int sll32_usecs_per_cycle=0;

/* how many counter cycles in a jiffy */
static unsigned long cycles_per_jiffy=0;

/* Cycle counter value at the previous timer interrupt.. */
static unsigned int timerhi, timerlo;

/* last time when xtime and rtc are sync'ed up */
static long last_rtc_update;

/* the function pointer to one of the gettimeoffset funcs*/
unsigned long (*do_gettimeoffset)(void) = null_gettimeoffset;

unsigned long null_gettimeoffset(void)
{
	return 0;
}

unsigned long fixed_rate_gettimeoffset(void)
{
	u32 count;
	unsigned long res;

	/* Get last timer tick in absolute kernel time */
	count = read_32bit_cp0_register(CP0_COUNT);

	/* .. relative to previous jiffy (32 bits is enough) */
	count -= timerlo;

#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)
	__asm__("multu	%1,%2"
		: "=h" (res)
		: "r" (count), "r" (sll32_usecs_per_cycle)
		: "lo", GCC_REG_ACCUM);
#else
	__asm__("multu\t%1,%2\n\t"
	        "mfhi\t%0"
	        :"=r" (res)
	        :"r" (count),
	         "r" (sll32_usecs_per_cycle));
#endif

	/*
	 * Due to possible jiffies inconsistencies, we need to check
	 * the result so that we'll get a timer that is monotonic.
	 */
	if (res >= USECS_PER_JIFFY)
		res = USECS_PER_JIFFY-1;

	return res;
}

/*
 * Cached "1/(clocks per usec)*2^32" value.
 * It has to be recalculated once each jiffy.
 */
static unsigned long cached_quotient;

/* Last jiffy when calibrate_divXX_gettimeoffset() was called. */
static unsigned long last_jiffies = 0;


/*
 * This is copied from dec/time.c:do_ioasic_gettimeoffset() by Mercij.
 */
unsigned long calibrate_div32_gettimeoffset(void)
{
	u32 count;
	unsigned long res, tmp;
	unsigned long quotient;

	tmp = jiffies;

	quotient = cached_quotient;

	if (last_jiffies != tmp) {
		last_jiffies = tmp;
		if (last_jiffies != 0) {
			unsigned long r0;
			do_div64_32(r0, timerhi, timerlo, tmp);
			do_div64_32(quotient, USECS_PER_JIFFY,
			            USECS_PER_JIFFY_FRAC, r0);
			cached_quotient = quotient;
		}
	}

	/* Get last timer tick in absolute kernel time */
	count = read_32bit_cp0_register(CP0_COUNT);

	/* .. relative to previous jiffy (32 bits is enough) */
	count -= timerlo;

#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)
	__asm__("multu  %1,%2"
		: "=h" (res)
		: "r" (count), "r" (quotient)
		: "lo", GCC_REG_ACCUM);
#else
	__asm__("multu  %2,%3"
	        : "=l" (tmp), "=h" (res)
	        : "r" (count), "r" (quotient));
#endif

	/*
	 * Due to possible jiffies inconsistencies, we need to check
	 * the result so that we'll get a timer that is monotonic.
	 */
	if (res >= USECS_PER_JIFFY)
		res = USECS_PER_JIFFY - 1;

	return res;
}

unsigned long calibrate_div64_gettimeoffset(void)
{
	u32 count;
	unsigned long res, tmp;
	unsigned long quotient;

	tmp = jiffies;

	quotient = cached_quotient;

#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)
	if (last_jiffies != tmp) {
		last_jiffies = tmp;
		if (last_jiffies) {
			unsigned long r0;
			__asm__(".set	push\n\t"
				".set	mips3\n\t"
				"lwu	%0,%3\n\t"
				"dsll32	%1,%2,0\n\t"
				"or	%1,%1,%0\n\t"
				"ddivu	$0,%1,%4\n\t"
				"mflo	%1\n\t"
				"dsll32	%0,%5,0\n\t"
				"or	%0,%0,%6\n\t"
				"ddivu	$0,%0,%1\n\t"
				"mflo	%0\n\t"
				".set	pop"
				: "=&r" (quotient), "=&r" (r0)
				: "r" (timerhi), "m" (timerlo),
				  "r" (tmp), "r" (USECS_PER_JIFFY),
				  "r" (USECS_PER_JIFFY_FRAC)
				: "hi", "lo", GCC_REG_ACCUM);
			cached_quotient = quotient;
		}
	}
#else
	if (tmp && last_jiffies != tmp) {
		last_jiffies = tmp;
		__asm__(".set\tnoreorder\n\t"
	        ".set\tnoat\n\t"
	        ".set\tmips3\n\t"
	        "lwu\t%0,%2\n\t"
	        "dsll32\t$1,%1,0\n\t"
	        "or\t$1,$1,%0\n\t"
	        "ddivu\t$0,$1,%3\n\t"
	        "mflo\t$1\n\t"
	        "dsll32\t%0,%4,0\n\t"
	        "nop\n\t"
	        "ddivu\t$0,%0,$1\n\t"
	        "mflo\t%0\n\t"
	        ".set\tmips0\n\t"
	        ".set\tat\n\t"
	        ".set\treorder"
	        :"=&r" (quotient)
	        :"r" (timerhi),
	         "m" (timerlo),
	         "r" (tmp),
	         "r" (USECS_PER_JIFFY));
	        cached_quotient = quotient;
	}
#endif

	/* Get last timer tick in absolute kernel time */
	count = read_32bit_cp0_register(CP0_COUNT);

	/* .. relative to previous jiffy (32 bits is enough) */
	count -= timerlo;

#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)
	__asm__("multu	%1,%2"
		: "=h" (res)
		: "r" (count), "r" (quotient)
		: "lo", GCC_REG_ACCUM);
#else
	__asm__("multu\t%1,%2\n\t"
	        "mfhi\t%0"
	        :"=r" (res)
	        :"r" (count),
	         "r" (quotient));
#endif

	/*
	 * Due to possible jiffies inconsistencies, we need to check
	 * the result so that we'll get a timer that is monotonic.
	 */
	if (res >= USECS_PER_JIFFY)
		res = USECS_PER_JIFFY-1;

	return res;
}


/*
 * local_timer_interrupt() does profiling and process accounting
 * on a per-CPU basis.  
 *
 * In UP mode, it is invoked from the (global) timer_interrupt.  
 *
 * In SMP mode, it might invoked by per-CPU timer interrupt, or
 * a broadcasted inter-processor interrupt which itself is triggered
 * by the global timer interrupt.
 */
#if 0
void local_timer_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	if (!user_mode(regs)) {
		if (prof_buffer && current->pid) {
			extern int _stext;
			unsigned long pc = regs->cp0_epc;

			pc -= (unsigned long) &_stext;
			pc >>= prof_shift;
			/*
			 * Dont ignore out-of-bounds pc values silently,
			 * put them into the last histogram slot, so if
			 * present, they will show up as a sharp peak.
			 */
			if (pc > prof_len-1)
			pc = prof_len-1;
			atomic_inc((atomic_t *)&prof_buffer[pc]);
		}
	}

#ifdef CONFIG_SMP
	/* in UP mode, update_process_times() is invoked by do_timer() */
	update_process_times(user_mode(regs));
#endif
}
/*
 * high-level timer interrupt service routines.  This function
 * is set as irqaction->handler and is invoked through do_IRQ.
 */
void timer_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	do_timer(regs);

}
asmlinkage void ll_timer_interrupt(int irq, struct pt_regs *regs)
{
	int cpu = smp_processor_id();

	irq_enter(cpu, irq);
	kstat.irqs[cpu][irq]++;

	/* we keep interrupt disabled all the time */
	timer_interrupt(irq, NULL, regs);
	
	irq_exit(cpu, irq);

	if (softirq_pending(cpu))
		do_softirq();
}

asmlinkage void ll_local_timer_interrupt(int irq, struct pt_regs *regs)
{
	int cpu = smp_processor_id();

	irq_enter(cpu, irq);
	kstat.irqs[cpu][irq]++;

	/* we keep interrupt disabled all the time */
	local_timer_interrupt(irq, NULL, regs);
	
	irq_exit(cpu, irq);

	if (softirq_pending(cpu))
		do_softirq();
}
#endif
/*
 * time_init() - it does the following things.
 *
 * 1) board_time_init() - 
 * 	a) (optional) set up RTC routines, 
 *      b) (optional) calibrate and set the mips_counter_frequency
 *	    (only needed if you intended to use fixed_rate_gettimeoffset
 *	     or use cpu counter as timer interrupt source)
 * 2) setup xtime based on rtc_get_time().
 * 3) choose a appropriate gettimeoffset routine.
 * 4) calculate a couple of cached variables for later usage
 * 5) board_timer_setup() - 
 *	a) (optional) over-write any choices made above by time_init().
 *	b) machine specific code should setup the timer irqaction.
 *	c) enable the timer interrupt
 */ 


static int dummy_set_rtc(void)
{
	return 0;
}

/*
 * hook for setting the RTC's idea of the current time.
 */
int (*set_rtc)(void) = dummy_set_rtc;

struct irqaction timer_irq = {
	name: "timer",
	flags:SA_INTERRUPT,
};
void __init time_init(void)
{
	xtime.tv_sec = 0;
	xtime.tv_usec = 0;
	setup_timer();
}

#define FEBRUARY		2
#define STARTOFTIME		1970
#define SECDAY			86400L
#define SECYR			(SECDAY * 365)
#define leapyear(year)		((year) % 4 == 0)
#define days_in_year(a)		(leapyear(a) ? 366 : 365)
#define days_in_month(a)	(month_days[(a) - 1])

static int month_days[12] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};
#if 0
void to_tm(unsigned long tim, struct rtc_time * tm)
{
	long hms, day, gday;
	int i;

	gday = day = tim / SECDAY;
	hms = tim % SECDAY;

	/* Hours, minutes, seconds are easy */
	tm->tm_hour = hms / 3600;
	tm->tm_min = (hms % 3600) / 60;
	tm->tm_sec = (hms % 3600) % 60;

	/* Number of years in days */
	for (i = STARTOFTIME; day >= days_in_year(i); i++)
	day -= days_in_year(i);
	tm->tm_year = i;

	/* Number of months in days left */
	if (leapyear(tm->tm_year))
	days_in_month(FEBRUARY) = 29;
	for (i = 1; day >= days_in_month(i); i++)
	day -= days_in_month(i);
	days_in_month(FEBRUARY) = 28;
	tm->tm_mon = i-1;	/* tm_mon starts from 0 to 11 */

	/* Days are what is left over (+1) from all that. */
	tm->tm_mday = day + 1;

	/*
	 * Determine the day of week
	 */
	tm->tm_wday = (gday + 4) % 7; /* 1970/1/1 was Thursday */
}
#endif
