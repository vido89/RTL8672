/* decide the irq block assignment */
#define	LX4180_NUM_CPU_IRQ	8
#define	LX4180_NUM_SYS_IRQ	32

#define	LX4180-_IRQ_BASE		0

/* CPU interrupts */

/* 
   IP0 - Software interrupt 
   IP1 - Software interrupt 
   IP2 - All but battery, high speed modem, and real time clock 
   IP3 - RTC Long1 (system timer) 
   IP4 - RTC Long2 
   IP5 - High Speed Modem (unused on VR4181) 
   IP6 - Unused 
   IP7 - Timer interrupt from CPO_COMPARE 
*/

#define LX4180_IRQ_SW1       (VR4181_CPU_IRQ_BASE + 0)  
#define LX4180_IRQ_SW2       (VR4181_CPU_IRQ_BASE + 1)  
#define LX4180_IRQ_INT0      (VR4181_CPU_IRQ_BASE + 2)
#define LX4180_IRQ_INT1      (VR4181_CPU_IRQ_BASE + 3)
#define LX4180_IRQ_INT2      (VR4181_CPU_IRQ_BASE + 4)
#define LX4180_IRQ_INT3      (VR4181_CPU_IRQ_BASE + 5)
#define LX4180_IRQ_INT4      (VR4181_CPU_IRQ_BASE + 6)
#define LX4180_IRQ_TIMER     (VR4181_CPU_IRQ_BASE + 7)


/* Cascaded from VR4181_IRQ_INT0 (ICU mapped interrupts) */

/* 
   IP2 - same as VR4181_IRQ_INT1
   IP8 - This is a cascade to GPIO IRQ's. Do not use.
   IP16 - same as VR4181_IRQ_INT2
   IP18 - CompactFlash
*/
