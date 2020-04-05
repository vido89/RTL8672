#include <linux/config.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <asm/ptrace.h>
#include <asm/rtl8670/interrupt.h>
#if defined(CONFIG_RTL8670)
#include "lx4180.h"
#else // 8671
#include "lx5280.h"
#endif

/* Jonah +2 for hrchen's code */
#if defined(CONFIG_RTL8670)
#define OBC_USE_TIMER1
#endif

//ql
#include "../../../../user/boa/src/LINUX/options.h"

extern void timer1_isr(void);


extern struct irqaction timer_irq;
void enable_lx4180_irq(int irq);
void disable_lx4180_irq(int irq);
extern int eth_poll;

static void rtl8670_timer_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{ 
	/* Jonah -3 +all for hrchen's code*/
	//REG32(TCIR) |= TCIR_TC0IP;
	///* call the generic one */
	//do_timer(regs);

	  unsigned int status=REG32(TCIR);
	
			REG32(TCIR) |= status;	/*clear IRQ pending*/
			if (status&TCIR_TC0IP) {  /*timer0 IRQ*/
				/* call the generic one */
				do_timer(regs);
			};
	
#ifdef OBC_USE_TIMER1   
			if (status&TCIR_TC1IP) {  /*timer1 IRQ*/
				/*wake up time out routine in OBC driver*/
				timer1_isr();
			};
#endif
    
    //for ATM QoS Interpolation
    extern void ATM_QoS_Interpolate(void);
    ATM_QoS_Interpolate();
    
	//7/13/06' hrchen, for watchdog monitor
    extern int watchdog_enable;
    extern void kick_watchdog(void);
	if (watchdog_enable)
        kick_watchdog();
	// Kaohj
#ifdef CONFIG_RE8670
	if (eth_poll) { // in eth poll mode; schedule eth rx for each timer tick
		eth_poll_schedule();
	}
#endif
}

void  setup_timer (void)
{

	REG32(CDBR)=(DIVISOR-1) << DIVF_OFFSET;
    	REG32(TC0DATA) = (((CPU_CLOCK_RATE/DIVISOR)/TICK_FREQ)+1) << TCD_OFFSET;	
#ifndef UPSTREAM_TRAFFIC_CTL
	REG32(TCCNR) = TC0EN | TC0MODE_TIMER;
#endif
	REG32(TCIR)=TCIR_TC0IE;

/* Jonah +5 for hrchen's code */
#ifdef OBC_USE_TIMER1   
			//enable timer1 for OBC
			REG32(TCIR)|=TCIR_TC1IE;
#endif
	//celar timer IRQ
	REG32(TCIR) |= (TCIR_TC0IP|TCIR_TC1IP);

#ifdef UPSTREAM_TRAFFIC_CTL
	REG32(CDBR)=(DIVISOR-1) << DIVF_OFFSET;
	REG32(TC1DATA) = (((CPU_CLOCK_RATE/DIVISOR)*8)+1) << TCD_OFFSET;
	REG32(TCCNR) = TC0EN | TC0MODE_TIMER | TC1EN | TC1MODE_TIMER;
	//REG32(TCIR) |= TCIR_TC1IE;
#endif

	
	REG32(IRR2)=0;
#ifndef CONFIG_RTL8672
	REG8(ILR)=0;
#endif
	timer_irq.handler = rtl8670_timer_interrupt;
#ifdef CONFIG_RTL8672
	setup_irq(23, &timer_irq);
#else
	setup_irq(1, &timer_irq);
#endif
	enable_lx4180_irq(1);
}
#ifdef CONFIG_RTL8672
void enable_lx4180_irq(int irq)
{
	switch (irq)
	{

#if defined (CONFIG_RTL8672) && defined (CONFIG_USB)
		case 21:	//usb host IP
			REG32(GISR) = USB_H_IS ;
			REG32(IRR1) |=USB_H_ILEV << USB_H_IPS;
			REG32(GIMR) |= USB_H_IM ;
			
		case 20:	//usb OTG IP
			REG32(GISR) = USB_D_IS ;
			REG32(IRR1) |=USB_D_ILEV << USB_D_IPS;
			REG32(GIMR) |= USB_D_IM ;
#endif		

		case 19:
//tylo, GIMR and GISR in 8672 is 4 BYTES
			REG32(GISR) = UART_IS ;
			REG32(IRR1) |=UART_ILEV << UART_IPS;
			REG32(GIMR) |= UART_IM ;
			break;

		case 1:
			REG32(GISR) = Timer_IS;
			REG32(IRR1) |= TICK_ILEV << Timer_IPS;
			REG32(GIMR) |= Timer_IM;
			break;
			
		case 6:
			REG32(GISR) = SAR_IS ; 
			REG32(IRR3) |=SAR_ILEV << SAR_IPS;
			REG32(GIMR) |= SAR_IM ;
			break;

		case 5:
			REG32(GISR) = Ethernet_IS ;
			REG32(IRR3) |=Ethernet_ILEV << Ethernet_IPS;
			REG32(GIMR) |= Ethernet_IM ;
			break;

		case 7:
			REG32(GISR) = DMT_IS ;
			REG32(IRR3) |=DMT_ILEV << DMT_IPS;
			REG32(GIMR) |= DMT_IM ;
			break;

#if defined(CONFIG_RTL8670) && defined (CONFIG_USB)
		case 6:
			REG32(GISR) = USB_IS ;
			REG32(IRR1) |=USB_ILEV << USB_IPS;
			REG32(GIMR) |= USB_IM ;
			break;
#endif			
#if 0
		case 7:
			REG32(GISR) = PCI_IS ;
			REG32(IRR1) |=PCI_ILEV << PCI_IPS;
			REG32(GIMR) |= PCI_IM ;
			break;
#endif

	}
}

void disable_lx4180_irq(int irq)
{
	switch (irq)
	{
		case 1:
			REG32(GISR) = Timer_IS;
			REG32(IRR1) |= TICK_ILEV << Timer_IPS;
			REG32(GIMR) &= ~Timer_IM;
			break;
			
		case 6:
			REG32(GISR) = SAR_IS ;
			REG32(IRR1) |=SAR_ILEV << SAR_IPS;
			REG32(GIMR) &= ~SAR_IM ;
			break;

		case 5:
			REG32(GISR) = Ethernet_IS ;
			REG32(IRR1) |=Ethernet_ILEV << Ethernet_IPS;
			REG32(GIMR) &= ~Ethernet_IM ;
			break;
			
		case 7:
			REG32(GISR) = DMT_IS ;
			REG32(IRR1) |=DMT_ILEV << DMT_IPS;
			REG32(GIMR) &= ~DMT_IM ;
			break;
			
#if defined(CONFIG_RTL8670) && defined (CONFIG_USB)
		case 6:
			REG16(GISR) = USB_IS ;
			REG32(IRR1) |=USB_ILEV << USB_IPS;
			REG16(GIMR) &= ~USB_IM ;
			break;
#endif
	}
}

#else
void enable_lx4180_irq(int irq)
{
	switch (irq)
	{
		case 0:
			REG16(GISR) = UART_IS ;
			REG32(IRR1) |=UART_ILEV << UART_IPS;
			REG16(GIMR) |= UART_IM ;
			break;

		case 1:
			REG16(GISR) = Timer_IS;
			REG32(IRR1) |= TICK_ILEV << Timer_IPS;
			REG16(GIMR) |= Timer_IM;
			break;
			
		case 3:
			REG16(GISR) = SAR_IS ;
			REG32(IRR1) |=SAR_ILEV << SAR_IPS;
			REG16(GIMR) |= SAR_IM ;
			break;

		case 4:
			REG16(GISR) = Ethernet_IS ;
			REG32(IRR1) |=Ethernet_ILEV << Ethernet_IPS;
			REG16(GIMR) |= Ethernet_IM ;
			break;
			
		case 5:
			REG16(GISR) = DMT_IS ;
			REG32(IRR1) |=DMT_ILEV << DMT_IPS;
			REG16(GIMR) |= DMT_IM ;
			break;
			
#if defined(CONFIG_RTL8670) && defined (CONFIG_USB)
		case 6:
			REG16(GISR) = USB_IS ;
			REG32(IRR1) |=USB_ILEV << USB_IPS;
			REG16(GIMR) |= USB_IM ;
			break;
#endif			

		case 7:
			REG16(GISR) = PCI_IS ;
			REG32(IRR1) |=PCI_ILEV << PCI_IPS;
			REG16(GIMR) |= PCI_IM ;
			break;
#ifdef CONFIG_RTK_VOIP
		case 10:
			REG16(GISR) = PCM_IS ;
			REG32(IRR2) |=PCM_ILEV << PCM_IPS;
			REG16(GIMR) |= PCM_IM ;
			break;
#endif
	}
}

void disable_lx4180_irq(int irq)
{
	switch (irq)
	{
		case 0:
			REG16(GISR) = UART_IS ;
			REG32(IRR1) |=UART_ILEV << UART_IPS;
			REG16(GIMR) &= ~UART_IM ;
			break;

		case 1:
			REG16(GISR) = Timer_IS;
			REG32(IRR1) |= TICK_ILEV << Timer_IPS;
			REG16(GIMR) &= ~Timer_IM;
			break;
			
		case 3:
			REG16(GISR) = SAR_IS ;
			REG32(IRR1) |=SAR_ILEV << SAR_IPS;
			REG16(GIMR) &= ~SAR_IM ;
			break;

		case 4:
			REG16(GISR) = Ethernet_IS ;
			REG32(IRR1) |=Ethernet_ILEV << Ethernet_IPS;
			REG16(GIMR) &= ~Ethernet_IM ;
			break;
			
		case 5:
			REG16(GISR) = DMT_IS ;
			REG32(IRR1) |=DMT_ILEV << DMT_IPS;
			REG16(GIMR) &= ~DMT_IM ;
			break;
			
#if defined(CONFIG_RTL8670) && defined (CONFIG_USB)
		case 6:
			REG16(GISR) = USB_IS ;
			REG32(IRR1) |=USB_ILEV << USB_IPS;
			REG16(GIMR) &= ~USB_IM ;
			break;
#endif
			
		case 7:
			REG16(GISR) = PCI_IS ;
			REG32(IRR1) |=PCI_ILEV << PCI_IPS;
			REG16(GIMR) &= ~PCI_IM ;
			break;
#ifdef CONFIG_RTK_VOIP
		case 10:
			REG16(GISR) = PCM_IS ;
			REG32(IRR2) |=PCM_ILEV << PCM_IPS;
			REG16(GIMR) &= ~PCM_IM ;
			break;
#endif
	}
}
#endif
#ifdef CONFIG_RTL8671
#include "../../../drivers/atm/ra8670.h"

extern struct tasklet_struct *sar_rx_tasklets;
extern struct tasklet_struct *eth_rx_tasklets;
extern void Disable_SAR(sar_private *cp);
int orig_rxmode;
#ifdef CONFIG_RTL8672
#define ETHBASE 0xb8018000
#else
#define ETHBASE	0xB9800000
#endif
#define RTL_W32(reg, value)			(*(volatile u32*)(ETHBASE+reg)) = (u32)value
#define RTL_R32(reg)				(*(volatile u32*)(ETHBASE+reg))
#define RCR		0x44
#define ACCEPT_BROADCAST	0x08
#define ACCEPT_MYPHYS		0x02

atomic_t activeFlag=ATOMIC_INIT(1);
//disable devices IRQ and their tasklet, called by mm_task()
void SuspendDevices(void)
{
	int rx_mode;
	if (!test_and_clear_bit(0, &activeFlag))
	    return;
	if (sar_rx_tasklets) tasklet_disable(sar_rx_tasklets);
	//if (eth_rx_tasklets) tasklet_disable(eth_rx_tasklets);
	
	//stop SAR
	disable_lx4180_irq(3);
	//stop NIC
#ifdef CONFIG_RTL8672
	disable_lx4180_irq(5);
#else
	disable_lx4180_irq(4);
#endif
	// save RCR
	orig_rxmode = RTL_R32(RCR);
	// Kaohj -- accept broadcast any myphy only
	rx_mode = (ACCEPT_BROADCAST | ACCEPT_MYPHYS);
	RTL_W32(RCR, rx_mode);
	// transfer to poll mode for rate control
#ifdef CONFIG_RE8670
	eth_poll = 1;
#endif	
	//stop PCI
#ifndef CONFIG_RTL8672
	disable_lx4180_irq(7);
#endif
	Disable_SAR(sar_dev);
    
}

//enable devices IRQ and their tasklet, called by mm_task()
void ResumeDevices(void)
{
	if (test_and_set_bit(0, &activeFlag))
	    return;
	if (sar_rx_tasklets) tasklet_enable(sar_rx_tasklets);
	//if (eth_rx_tasklets) tasklet_enable(eth_rx_tasklets);
	
	//start SAR
	enable_lx4180_irq(3);
	//start NIC
	// Kaohj -- return to interrupt mode
#ifdef CONFIG_RE8670
	eth_poll = 0;
#endif
#ifdef CONFIG_RTL8672
	enable_lx4180_irq(5);
#else
	enable_lx4180_irq(4);
#endif
	// restore RCR
	RTL_W32(RCR, orig_rxmode);
	
	//start PCI
#ifndef CONFIG_RTL8672
	enable_lx4180_irq(7);
#endif
}
#endif

/* !!! JONAH + from hrchen's timer.c */
#ifdef OBC_USE_TIMER1	

/*
call from OBC driver to get rest msec of timer1 before timeout
*/
unsigned long get_timer1_rest_ms(void)
{
    return (REG32(TC1CNT)>>TCD_OFFSET)/(CPU_CLOCK_RATE/(DIVISOR*1000));
}

/*
call from OBC driver to start timer1
*/
void start_timer1(unsigned long timer_ms)
{
  unsigned long timer_data = timer_ms*(CPU_CLOCK_RATE/(DIVISOR*1000));  /*1ms value for timer1 data register*/
  
    REG32(TCCNR) &= (~TC1EN);  /*reset counter*/
    if (timer_data==0)
	timer_data = 3;
    REG32(TC1DATA) = (timer_data-1) << TCD_OFFSET;	/*set data*/
    REG32(TCCNR) |= (TC1EN | TC1MODE_COUNTER);  /*start count down*/
}

/*
call from OBC driver to stop timer1
*/
void stop_timer1(void)
{
    REG32(TCCNR) &= (~TC1EN);  /*disable counter*/
    //celar timer1 IRQ
    REG32(TCIR) |= TCIR_TC1IP;
}
#endif

