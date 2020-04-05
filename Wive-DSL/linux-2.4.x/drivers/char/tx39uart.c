/***********************************************************************************
$RCSfile: tx39uart.c,v $
Subject : UART Device Driver
$Date: 2003/08/18 05:43:16 $
$Author: kaohj $
$Log: tx39uart.c,v $
Revision 1.1.1.1  2003/08/18 05:43:16  kaohj
initial import into CVS

Revision 1.11  2002/05/16 05:35:40  colinlee
DEL: remove WRITE_TX39REG(UART_DISTXD, OFF_UARTA_CTL1); in tx39_initialize

Revision 1.10  2002/05/14 09:27:36  colinlee
MOD: fix TX39_CLEAN_UARTA_INT written by vincent

Revision 1.9  2002/04/23 03:39:25  vincent
MOD: Don't touch UARTB when clean UARTA interrupt

Revision 1.8  2002/04/16 03:26:07  vincent
DEL: remove tags of CVS uptate contradictory

Revision 1.7  2002/04/16 02:39:46  vincent
ADD: add tx39uart_use_count variable to deside to free_irq or not
ADD: add TX39_CLEAN_UARTA_INT in tx39_initialize to fix bug when 1st open.

Revision 1.6  2002/04/10 02:34:52  colinlee
ADD: reset UART DMA counter when opening
DEL: remove MOD_IN_USE when closing

Revision 1.5  2002/03/14 08:24:16  colinlee
MOD: replace printk with DEBUGINFO and add #ifdef GV_CONFIG_DEBUG_MESSAG
ADD: set MFIO 23,24 to standard function at console_init, because bootrom DISABLE UART

Revision 1.4  2002/03/11 08:28:27  colinlee
DEL: remove some LED_control

Revision 1.3  2002/03/11 02:34:00  vincent
ADD: add current_baud variable and enable change_speed hardware control marked by Fanky

Revision 1.2  2002/03/07 01:20:05  colinlee
DEL: move DisableUARTA() to usb820d.c

Revision 1.1.1.1  2002/02/06 08:45:32  colinlee
import

Revision 1.3  2002/01/25 09:05:25  colin
*** empty log message ***

Revision 1.2  2002/01/09 01:24:37  colin
add: inmport CVS information

************************************************************************************/


#define SERIAL_INLINE

#include <linux/config.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>
#include <linux/serial_reg.h>
#include <linux/major.h>
#include <linux/string.h>
#include <linux/fcntl.h>
#include <linux/ptrace.h>
#include <linux/ioport.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/serialP.h>
#include <linux/delay.h>
#if 1
#include <linux/console.h>
#endif

#include <asm/system.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/bitops.h>
#include <asm/serial.h>

#include "tx39.h"
#include "tsbref.h"
//#include <asm/tsbref/systemmap.h>
//#include <asm/tsbref/deviceinfo.h>

//#include "u820.h"



#ifdef SERIAL_INLINE
#define _INLINE_ inline
#endif
	
//#define  GV_CONFIG_DEBUG_MESSAGE 1

#ifdef  GV_CONFIG_DEBUG_MESSAGE
#define DEBUGINFO(a) printk a
#else
#define DEBUGINFO(a)
#endif

static char *serial_name = "TX39 UART driver";
static char *serial_version = "0.04";

static DECLARE_TASK_QUEUE(tq_serial);

static struct tty_driver serial_driver, callout_driver;
static int serial_refcount;
static int tx39uart_use_count = 0;

/* number of characters left in xmit buffer before we ask for more */
#define WAKEUP_CHARS 256

/*
 * IRQ_timeout		- How long the timeout should be for each IRQ
 * 				should be after the IRQ has been active.
 */

static struct async_struct *IRQ_ports[NR_IRQS];

static int IRQ_timeout[NR_IRQS];
#ifdef CONFIG_TX39UART_CONSOLE
static struct console sercons;
#endif

//static unsigned detect_uart_irq (struct serial_state * state);
static void change_speed(struct async_struct *info, struct termios *old_termios);
static void tx39uart_wait_until_sent(struct tty_struct *tty, int timeout);


#define NR_PORTS	1

#define RS_ISR_PASS_LIMIT 256

static struct tty_struct *serial_table[NR_PORTS];
static struct termios *serial_termios[NR_PORTS];
static struct termios *serial_termios_locked[NR_PORTS];

#ifndef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#endif

static struct serial_state tx39uart_table[] = {
        {0, (1843200/16), 0x100, 1, STD_COM_FLAGS}  /* ttyS0 */
};

/*
 * tmp_buf is used as a temporary buffer by serial_write.  We need to
 * lock it in case the copy_from_user blocks while swapping in a page,
 * and some other program tries to do a serial write at the same time.
 * Since the lock will only come under contention when the system is
 * swapping and available memory is low, it makes sense to share one
 * buffer across all the serial ports, since it significantly saves
 * memory if large numbers of serial ports are open.
 */
static unsigned char *tmp_buf;
static unsigned char *dma_buf;
int dma_buf_pos = 0;
int current_baud = 115200;	//vincent for uartopen_set()
static DECLARE_MUTEX(tmp_buf_sem);

/*
 *  Macro for TX39 serial
 */

#define	TX39_PARITY_MASK	(UART_EVENPARITY | UART_ENPARITY)
#define	TX39_PARITY_ODD		(UART_ENPARITY)
#define	TX39_PARITY_EVEN	(UART_EVENPARITY | UART_ENPARITY)
#define	TX39_PARITY_NONE	0

#define	TX39_BIT_MASK		(UART_BIT_7)
#define	TX39_BIT_7		(UART_BIT_7)
#define	TX39_BIT_8		0	

#define	TX39_STOP_MASK		(UART_TWOSTOP)
#define	TX39_STOP_1		0
#define	TX39_STOP_2		(UART_TWOSTOP)


#define TX39_GETCHAR        (READ_TX39REG(OFF_UARTA_RXDATA) & UART_RXDATAMASK)
#define TX39_PUTCHAR(c)     (WRITE_TX39REG(UART_TXDATA(c), OFF_UARTA_TXDATA))

#define TX39_ENABLE_TXINTR  OR_TX39REG(INTR_UARTATXINT, OFF_INTR_ENABLE2)
#define TX39_DISABLE_TXINTR AND_TX39REG(~INTR_UARTATXINT, OFF_INTR_ENABLE2) 
#define TX39_ENABLE_RXINTR  OR_TX39REG(INTR_UARTARXINT, OFF_INTR_ENABLE2)
#define TX39_DISABLE_RXINTR AND_TX39REG(~INTR_UARTARXINT, OFF_INTR_ENABLE2) 
#define TX39_ENABLE_MIO18NEG OR_TX39REG(INTR_MFIONEGINT(18),OFF_INTR_ENABLE4)
#define TX39_ENABLE_MIO18POS OR_TX39REG(INTR_MFIOPOSINT(18),OFF_INTR_ENABLE3)
#define TX39_DISABLE_MIO18NEG AND_TX39REG(~INTR_MFIONEGINT(18),OFF_INTR_ENABLE4)
#define TX39_DISABLE_MIO18POS AND_TX39REG(~INTR_MFIOPOSINT(18),OFF_INTR_ENABLE3)
#define TX39_CLEAN_UARTA_INT  WRITE_TX39REG(0xffc00000,OFF_INTR_CLEAR2)

/* Added by Fanky */
#define TX39_MIO_RTS          0x00000020
#define TX39_MIO_CTS          0x00040000
#define TX39_IOCTL_FLOW       0x5496  
#define TX39_FLOW_HW          0x01
#define TX39_FLOW_SW          0x02     


struct _modem_signal {
    unsigned int addr;		/* address of MODEM SIGNAL */
    unsigned int mask;		/* mask of data pointed addr */
    unsigned int value;		/* signal is valid if masked data equal to value */
};

#define	IDX_RTS	0
#define	IDX_CTS	1
#define	IDX_DTR 2
#define	IDX_DSR	3
#define	IDX_DCD	4
#define	IDX_RI	5

#ifdef HARD_FLOW 
static struct _modem_signal modem_signal_3912[] = {
    {OFF_IO_CTL,	IO_IODOUT_3912(SER_RTS), IO_IODOUT_3912(SER_RTS)},
    {OFF_IO_CTL,	IO_IODIN_3912(SER_CTS), IO_IODIN_3912(SER_CTS)},
    {OFF_IO_MFIOOUT,	IO_MFIOOUT(SER_DTR), IO_MFIOOUT(SER_DTR)},
    {OFF_IO_MFIOIN,	IO_MFIOIN(SER_DSR), IO_MFIOIN(SER_DSR)},
    {OFF_IO_MFIOIN,	IO_MFIOIN(SER_DCD), IO_MFIOIN(SER_DCD)},
    {OFF_IO_MFIOIN,	IO_MFIOIN(SER_RI), IO_MFIOIN(SER_RI)}
};

static struct _modem_signal modem_signal_3922[] = {
    {OFF_IO_INOUT,	IO_IODOUT_3922(SER_RTS), IO_IODOUT_3922(SER_RTS)},
    {OFF_IO_INOUT,	IO_IODIN_3922(SER_CTS), IO_IODIN_3922(SER_CTS)},
    {OFF_IO_MFIOOUT,	IO_MFIOOUT(SER_DTR), IO_MFIOOUT(SER_DTR)},
    {OFF_IO_MFIOIN,	IO_MFIOIN(SER_DSR), IO_MFIOIN(SER_DSR)},
    {OFF_IO_MFIOIN,	IO_MFIOIN(SER_DCD), IO_MFIOIN(SER_DCD)},
    {OFF_IO_MFIOIN,	IO_MFIOIN(SER_RI), IO_MFIOIN(SER_RI)}
};
#endif

struct _baudtbl {
    int number;
    int bnumber;
    unsigned int value;
};

static struct _baudtbl baudtbl1 [] = {		/* this table is for TX3912-75 */
    {115200, B115200,   UART_B115200_3728},     // colin : 0604
    {57600, B57600, UART_B57600_3728},
    {38400, B38400, UART_B38400_3728},
    {19200, B19200, UART_B19200_3728},
    {9600, B9600, UART_B9600_3728},
    {4800, B4800, UART_B4800_3728},
    {2400, B2400, UART_B2400_3728},
    {1200, B1200, UART_B1200_3728},
    {600, B600, UART_B600_3728},
    {300, B300, UART_B300_3728},
    {0, B0, 0}
};

#if	0
static struct _baudtbl baudtbl2 [] = {		/* this table is for TX3912-92, TX3922 */
    {115200, B115200,	UART_B115200_9216},
    {57600, B57600, UART_B57600_9216},
    {38400, B38400, UART_B38400_9216},
    {19200, B19200, UART_B19200_9216},
    {9600, B9600, UART_B9600_9216},
    {4800, B4800, UART_B4800_9216},
    {2400, B2400, UART_B2400_9216},
    {1200, B1200, UART_B1200_9216},
    {600, B600, UART_B600_9216},
    {300, B300, UART_B300_9216},
    {0,	B0, 0}
};
#endif

struct _int_info {
    int addr;
    int mask;
};

#define MAXINFO		8
#define SER_MASK	(UARTA_ERR | INTR_UARTATXINT | INTR_UARTARXINT)

#ifdef HARD_DLOW
static struct _int_info tx3912_int_info [MAXINFO] = {
    {OFF_INTR_ENABLE2, SER_MASK},
    {OFF_INTR_ENABLE3, SER_MFIOPOS},
    {OFF_INTR_ENABLE4, SER_MFIONEG},
    {OFF_INTR_ENABLE5, SER_GIO_3912},
    {0, 0}
};

static struct _int_info tx3922_int_info [MAXINFO] = {
    {OFF_INTR_ENABLE2, SER_MASK},
    {OFF_INTR_ENABLE3, SER_MFIOPOS},
    {OFF_INTR_ENABLE4, SER_MFIONEG},
    {OFF_INTR_ENABLE8, SER_GIO_3912},
    {0, 0}
};
#endif

#define	CLOCK_CONFIG0	( CLOCK_CSERDIV(0) | CLOCK_ENCSERCLK | CLOCK_ENUARTACLK )
//#define	CLOCK_CONFIG1	( CLOCK_CSERSEL | CLOCK_CSERDIV(2) | CLOCK_ENCSERCLK | CLOCK_ENUARTACLK )
//#define CLOCK_CONFIG1   ( CLOCK_CSERSEL | CLOCK_CSERDIV(0) | CLOCK_ENCSERCLK | CLOCK_ENUARTACLK )
//MM:20010920
#define CLOCK_CONFIG1   ( CLOCK_CSERSEL | CLOCK_CSERDIV(2) | CLOCK_ENCSERCLK | CLOCK_ENUARTACLK )
// colin : 0604

#define UART_ENABLE 1		// colin : 20011220
int uart_enable=1;		// colin : 20011224

#ifdef HARD_FLOW 
static struct machine_tbl {
    int cpu_type;		/* CPU type */
    unsigned int clock_config;		/* Clock Register variable */
    unsigned int baudmask;		/* Baud Rate counter mask */
    struct _baudtbl *baudtable;	/* Baud rate Table */
    struct _modem_signal *signaltable; /* modem signal definition */
    struct _int_info *inttable;
} cputypes[] = {
    {MACH_GVPDA_TX3911_55, CLOCK_CONFIG0, UART_BAUDMASK_18432, baudtbl1, modem_signal_3912, tx3912_int_info},
    {MACH_GVPDA_TX3911_58, CLOCK_CONFIG1, UART_BAUDMASK_3686, baudtbl1, modem_signal_3912, tx3912_int_info}
};
#else
static struct machine_tbl {
    int cpu_type;		/* CPU type */
    unsigned int clock_config;		/* Clock Register variable */
    unsigned int baudmask;		/* Baud Rate counter mask */
    struct _baudtbl *baudtable;	/* Baud rate Table */
    struct _modem_signal *signaltable; /* modem signal definition */
    struct _int_info *inttable;
} cputypes[] = {
//    {MACH_GVPDA_TX3911_55, CLOCK_CONFIG1, UART_BAUDMASK_3686, baudtbl1, modem_signal_3912, tx3912_int_info},
 //   {MACH_GVPDA_TX3911_58, CLOCK_CONFIG1, UART_BAUDMASK_3686, baudtbl1, modem_signal_3912, tx3912_int_info}
    {0/*MACH_GVPDA_TX3911_55*/, CLOCK_CONFIG0, UART_BAUDMASK_18432, baudtbl1, NULL, NULL},
//    {MACH_GVPDA_TX3911_58, CLOCK_CONFIG1, UART_BAUDMASK_3686, baudtbl1, NULL, NULL}
    {1/*MACH_GVPDA_TX3911_58*/, CLOCK_CONFIG1, UART_BAUDMASK_3728, baudtbl1, NULL, NULL}//colin:0604
};
#endif

#define	SUPPORT_CPUS	(sizeof(cputypes)/sizeof(cputypes[0]))

#define	TSBREF_DEFAULT_BAUD	115200
//#define	TSBREF_DEFAULT_BAUD	9600	

static struct machine_tbl	*machine_info = NULL;
//static void tx39_initialize(int baud, int parity, int bits, int stop);
void tx39_initialize(int baud, int parity, int bits, int stop);
static struct _baudtbl *getbaudvalbynum(int reqbaud, int fallbackbaud);
static int def_cflags = 0;
static void UART_DMA_INIT(void);
static unsigned char DMA_TX39_GETCHAR(void);
static int  dma_do_next(void);

/*
 * ------------------------------------------------------------
 * tx39uart_stop() and tx39uart_start()
 *
 * This routines are called before setting or resetting tty->stopped.
 * They enable or disable transmitter interrupts, as necessary.
 * ------------------------------------------------------------
 */
static void tx39uart_stop(struct tty_struct *tty)
{
	DEBUGINFO(("tx39uart_stop called\n"));

}

static void tx39uart_start(struct tty_struct *tty)
{
	struct async_struct *info = (struct async_struct *)tty->driver_data;
	unsigned long flags;

	DEBUGINFO(("tx39uart_start called\n"));
	
	save_flags(flags); cli();
	if (info->xmit.head != info->xmit.tail && info->xmit.buf && !(info->IER & UART_IER_THRI)) {
		info->IER |= UART_IER_THRI;
		TX39_ENABLE_TXINTR;
	}
	restore_flags(flags);	
}

/*
 * ----------------------------------------------------------------------
 *
 * Here starts the interrupt handling routines.  All of the following
 * subroutines are declared as inline and are folded into
 * tx39uart_interrupt().  They were separated out for readability's sake.
 *
 * Note: tx39uart_interrupt() is a "fast" interrupt, which means that it
 * runs with interrupts turned off.  People who may want to modify
 * tx39uart_interrupt() should try to keep the interrupt handler as fast as
 * possible.  After you are done making modifications, it is not a bad
 * idea to do:
 * 
 * gcc -S -DKERNEL -Wall -Wstrict-prototypes -O6 -fomit-frame-pointer serial.c
 *
 * and look at the resulting assemble code in serial.s.
 *
 * 				- Ted Ts'o (tytso@mit.edu), 7-Mar-93
 * -----------------------------------------------------------------------
 */

/*
 * This routine is used by the interrupt handler to schedule
 * processing in the software interrupt portion of the driver.
 */
static _INLINE_ void tx39uart_sched_event(struct async_struct *info,
				  int event)
{
	info->event |= 1 << event;
	queue_task(&info->tqueue, &tq_serial);
	mark_bh(SERIAL_BH);		// colin : 20010626
}

static unsigned char DMA_TX39_GETCHAR(void)
{
   int i;
   
  
   
   i = dma_buf_pos;
   if (dma_buf_pos == 255 )
        dma_buf_pos = 0;
   else 
        dma_buf_pos++;

   
   //printk("=>%x %x %c\n",dma_buf_pos,READ_TX39REG(OFF_UARTA_DMA_COUNT),dma_buf[i]);
   return dma_buf[i] ;     


}

static int dma_do_next(void)
{
   int dma_remain_count;
    
   dma_remain_count = READ_TX39REG(OFF_UARTA_DMA_COUNT) & 0x0000ffff;
   if (dma_buf_pos != dma_remain_count)
      return 1;

        return 0;
}

static _INLINE_ void receive_chars(struct async_struct *info,
				 unsigned long *status)
{
	struct tty_struct *tty = info->tty;
	unsigned char ch;
	struct	async_icount *icount;
        int ignored = 0;
        int status_read = 0;
	static unsigned int nbrk=0;		// colin : 20011207
       
 
	icount = &info->state->icount;

#define BUF_THRESHOLD 0	
#ifdef  GV_CONFIG_DEBUG_MESSAGE
printk("UARTA ctrl1 reg = 0x%08x\n", READ_TX39REG(OFF_UARTA_CTL1));
printk("<<%d>>\n", dma_buf_pos);
#endif

	do {
#ifdef  GV_CONFIG_DEBUG_MESSAGE
printk("[%d],[%d],", dma_buf_pos, READ_TX39REG(OFF_UARTA_DMA_COUNT) & 0x0000ffff);
#endif
                
                WRITE_TX39REG( INTR_UARTARXINT, OFF_INTR_CLEAR2 ); 
                                   
	        ch = DMA_TX39_GETCHAR();

#ifdef  GV_CONFIG_DEBUG_MESSAGE
		printk("0x%x(%c),",ch, ch);
#endif
                status_read = 0;
                if (tty->flip.count + BUF_THRESHOLD  >= TTY_FLIPBUF_SIZE)
                {
                        DEBUGINFO(("tx39uart.c : tty receive buffer full.\n"));
                        break;
                } 
                
                *tty->flip.flag_buf_ptr = 0;
                *tty->flip.char_buf_ptr = ch;
                icount->rx++;
              


                if ( *status & ( INTR_UARTABREAKINT |INTR_UARTAPARITYERRINT|
                            INTR_UARTAFRAMEERRINT | INTR_UARTARXOVERRUNINT ))
                {
                   if (*status&INTR_UARTABREAKINT){
                        icount->brk++;
			nbrk++;

	/*	
                        *status &= ~(INTR_UARTAPARITYERRINT |
                                    INTR_UARTAFRAMEERRINT);	// ??????
	*/	
                        status_read |= UART_LSR_BI;
                        DEBUGINFO(("receive BREAK\n"));
                        WRITE_TX39REG(INTR_UARTABREAKINT,OFF_INTR_CLEAR2);
#if 0
			if (nbrk > 100)		// colin : 20011207
			{
OR_TX39REG(IO_MFIOOUT(5), OFF_IO_MFIOOUT);
				AND_TX39REG(~(INTR_UARTARXINT|INTR_UARTARXOVERRUNINT|
			INTR_UARTAFRAMEERRINT|INTR_UARTABREAKINT|INTR_UARTAPARITYERRINT),OFF_INTR_ENABLE2);
				nbrk = 0;
			}
#endif
                   }else if (*status&INTR_UARTAPARITYERRINT){
                        icount->parity++;
                        status_read |= UART_LSR_PE;
                        DEBUGINFO(("receive PE\n")); 
                        WRITE_TX39REG(INTR_UARTAPARITYERRINT,OFF_INTR_CLEAR2);
                   }else if (*status&INTR_UARTAFRAMEERRINT){
                        icount->frame++;
                        status_read |= UART_LSR_FE;
                        DEBUGINFO(("receive FE\n"));
                        WRITE_TX39REG(INTR_UARTAFRAMEERRINT,OFF_INTR_CLEAR2);
                   }else if (*status&INTR_UARTARXOVERRUNINT){
                        icount->overrun++;
                        status_read |= UART_LSR_OE;
                        DEBUGINFO(("receive OVERRUN\n"));
                        DEBUGINFO((" %x \n",*status ));
                        WRITE_TX39REG(INTR_UARTARXOVERRUNINT,OFF_INTR_CLEAR2); 
                   } 
                
                   if (status_read & info->ignore_status_mask) 
                   {
			if (++ignored > 100)
				break;
			goto ignore_char;
		   }
		   status_read &= info->read_status_mask;
                   if (status_read & (UART_LSR_BI) ) {

			*tty->flip.flag_buf_ptr = TTY_BREAK;
			if (info->flags & ASYNC_SAK)
				do_SAK(tty);
		   } else if (status_read & UART_LSR_PE )
				*tty->flip.flag_buf_ptr = TTY_PARITY;
		   else if (status_read & UART_LSR_FE )
				*tty->flip.flag_buf_ptr = TTY_FRAME;

                   if (status_read & UART_LSR_OE) {
                       if (tty->flip.count < TTY_FLIPBUF_SIZE) {
					tty->flip.count++;
					tty->flip.flag_buf_ptr++;
					tty->flip.char_buf_ptr++;
					*tty->flip.flag_buf_ptr = TTY_OVERRUN;
	               }
		   }

                } /* upper if */
                
             
                
                if ( I_IXON(tty) )
                {
                  if ( ch == /*STOP_CHAR(tty)*/ 19 ){
                        info->tty->stopped = 1;  
                        //TX39_DISABLE_TXINTR;
                         
                  }else if ( ch == /*START_CHAR(tty)*/ 17 ){
                        info->tty->stopped = 0;
                        //TX39_ENABLE_TXINTR;               
                        
                  }
                }
                
                      
		tty->flip.flag_buf_ptr++;
		tty->flip.char_buf_ptr++;
		tty->flip.count++;	
             
ignore_char:

               *status = READ_TX39REG( OFF_INTR_STATUS2) & READ_TX39REG(OFF_INTR_ENABLE2);      		
	} while ( (*status & INTR_UARTARXINT) || dma_do_next() );



	tty_flip_buffer_push(tty);
}

static _INLINE_ void transmit_chars(struct async_struct *info, int *intr_done)
{
	int count;
    unsigned long flags; 

	if (info->x_char) {
		WRITE_TX39REG(INTR_UARTATXINT, OFF_INTR_CLEAR2);
		WRITE_TX39REG(UART_TXDATA(info->x_char), OFF_UARTA_TXDATA);
		info->state->icount.tx++;
		info->x_char = 0;
                //printk(" Xon/Xoff : X char %x\n",info->x_char); 
		if (intr_done)
			*intr_done = 0;
		return;
	}
	
	if ((CIRC_CNT(info->xmit.head, info->xmit.tail, SERIAL_XMIT_SIZE) <= 0) || info->tty->stopped ||
	    info->tty->hw_stopped) {
                 
		info->IER &= ~UART_IER_THRI;
		TX39_DISABLE_TXINTR;
		return;
	}


	count = info->xmit_fifo_size;
	
	do {
                save_flags(flags); cli(); 
                /*if ( READ_TX39REG(OFF_INTR_STATUS3) & 
                     READ_TX39REG(OFF_INTR_ENABLE3) & IO_MFIOSEL(18) )
                {
                        WRITE_TX39REG(IO_MFIOSEL(18) , OFF_INTR_CLEAR3);
                        TX39_DISABLE_TXINTR;
                        info->tty->hw_stopped = 1;
                        restore_flags(flags); 
                        return;
                }*/
                
		WRITE_TX39REG(INTR_UARTATXINT, OFF_INTR_CLEAR2);
		WRITE_TX39REG(UART_TXDATA(info->xmit.buf[info->xmit.tail++]), OFF_UARTA_TXDATA);
		info->xmit.tail = info->xmit.tail & (SERIAL_XMIT_SIZE-1);
		info->state->icount.tx++;
                restore_flags(flags);

	} while (--count > 0 && !info->tty->hw_stopped );
	
	if (CIRC_CNT(info->xmit.head, info->xmit.tail, SERIAL_XMIT_SIZE) < WAKEUP_CHARS)
		tx39uart_sched_event(info, RS_EVENT_WRITE_WAKEUP);

#ifdef SERIAL_DEBUG_INTR
	printk("THRE...");
#endif
	if (intr_done)
		*intr_done = 0;

	if (CIRC_CNT(info->xmit.head, info->xmit.tail, SERIAL_XMIT_SIZE) <= 0) {
		info->IER &= ~UART_IER_THRI;
		TX39_DISABLE_TXINTR;
	}
}

static _INLINE_ void check_modem_status(struct async_struct *info)
{
	DEBUGINFO(("check_modem_status\n"));
}


/*
 * This is the serial driver's interrupt routine for a single port
 */
//static void tx39uart_interrupt_single(int irq, void *dev_id, struct pt_regs * regs)
void tx39uart_interrupt_single(int irq, void *dev_id, struct pt_regs * regs) // colin : pwr_save
{
//	int status_CTSH,status_CTSL;
        unsigned long status_en2;
	int pass_counter = 0;	
	struct async_struct * info;
        unsigned long flags;

//tx39_xxx_write("xx",2);
#if 0 
	printk("tx39uart_interrupt_single\n");
#endif

	info = 	IRQ_ports[irq];

	if (!info || !info->tty)
		return;	

	do {
            /* Add by Fanky ; check CTS line status */
		status_en2 = READ_TX39REG(OFF_INTR_STATUS2) &
			READ_TX39REG(OFF_INTR_ENABLE2);
/*             
                if ( info->tty->termios->c_cflag & CRTSCTS ){ 
                     status_CTSH = READ_TX39REG(OFF_INTR_STATUS3) &
                             READ_TX39REG(OFF_INTR_ENABLE3)& IO_MFIOSEL(18);
                     status_CTSL = READ_TX39REG(OFF_INTR_STATUS4) & 
                             READ_TX39REG(OFF_INTR_ENABLE4)& IO_MFIOSEL(18);
                }
                else 
                {
                  status_CTSH = status_CTSL =0;                
                }

                if (status_CTSH )
                {
                       WRITE_TX39REG(IO_MFIOSEL(18),OFF_INTR_CLEAR3);
                       info->tty->hw_stopped = 1;
                }    
                if (status_CTSL)
                {
                       WRITE_TX39REG(IO_MFIOSEL(18),OFF_INTR_CLEAR4);
                       info->tty->hw_stopped = 0;
                       TX39_ENABLE_TXINTR;
                       tx39uart_sched_event(info, RS_EVENT_WRITE_WAKEUP);

                }  
 */
		if (status_en2 & INTR_UARTATXINT) {
                        
			transmit_chars(info, 0);
                       
		}

		if (status_en2 & INTR_UARTARXINT){
                        save_flags(flags); cli(); 
			receive_chars(info, &status_en2);
                        restore_flags(flags);
                } 
		
		if (pass_counter++ > RS_ISR_PASS_LIMIT)
			break;
		
	} while (READ_TX39REG(OFF_INTR_STATUS2) & (INTR_UARTARXINT|INTR_UARTATXINT));
	info->last_active = jiffies;
}

/*
 * -------------------------------------------------------------------
 * Here ends the serial interrupt routines.
 * -------------------------------------------------------------------
 */

/*
 * This routine is used to handle the "bottom half" processing for the
 * serial driver, known also the "software interrupt" processing.
 * This processing is done at the kernel interrupt level, after the
 * tx39uart_interrupt() has returned, BUT WITH INTERRUPTS TURNED ON.  This
 * is where time-consuming activities which can not be done in the
 * interrupt driver proper are done; the interrupt driver schedules
 * them using tx39uart_sched_event(), and they get done here.
 */
static void do_serial_bh(void)
{
	run_task_queue(&tq_serial);
}

static void do_softint(void *private_)
{
	struct async_struct	*info = (struct async_struct *) private_;
	struct tty_struct	*tty;
	
	tty = info->tty;
	if (!tty)
		return;

	if (test_and_clear_bit(RS_EVENT_WRITE_WAKEUP, &info->event)) {
		if ((tty->flags & (1 << TTY_DO_WRITE_WAKEUP)) &&
		    tty->ldisc.write_wakeup)
			(tty->ldisc.write_wakeup)(tty);
		wake_up_interruptible(&tty->write_wait);
	}
}

/*
 * ---------------------------------------------------------------
 * Low level utility subroutines for the serial driver:  routines to
 * figure out the appropriate timeout for an interrupt chain, routines
 * to initialize and startup a serial port, and routines to shutdown a
 * serial port.  Useful stuff like that.
 * ---------------------------------------------------------------
 */

#if 0		// colin: 20020314  not used
/*
 * This routine figures out the correct timeout for a particular IRQ.
 * It uses the smallest timeout of all of the serial ports in a
 * particular interrupt chain.  Now only used for IRQ 0....
 */
static void figure_IRQ_timeout(int irq)
{
	struct	async_struct	*info;
	int	timeout = 60*HZ;	/* 60 seconds === a long time :-) */

	info = IRQ_ports[irq];
	if (!info) {
		IRQ_timeout[irq] = 60*HZ;
		return;
	}
	while (info) {
		if (info->timeout < timeout)
			timeout = info->timeout;
		info = info->next_port;
	}
	if (!irq)
		timeout = timeout / 2;
	IRQ_timeout[irq] = timeout ? timeout : 1;
}
#endif
static int startup(struct async_struct * info)
{
	unsigned long flags;
	int	retval=0;
	void (*handler)(int, void *, struct pt_regs *);
	struct serial_state *state= info->state;
	unsigned long page;

	page = get_free_page(GFP_KERNEL);
	if (!page)
		return -ENOMEM;

	save_flags(flags); cli();

	if (info->flags & ASYNC_INITIALIZED) {
		free_page(page);
		goto errout;
	}

	/* XXX */
	
	state->type = 1;
	
	if (!state->port || !state->type) {
		if (info->tty)
			set_bit(TTY_IO_ERROR, &info->tty->flags);
		printk("port err\n");
		free_page(page);
		goto errout;
	}
	if (info->xmit.buf)
		free_page(page);
	else
		info->xmit.buf = (unsigned char *) page;
	handler = tx39uart_interrupt_single;
	
	retval = request_irq(1, handler, SA_INTERRUPT, "serial", NULL);

#if 0	// colin : 20020403
	LED_control(1,1);
	printk("INT2 = 0x%08lx\n",READ_TX39REG(OFF_INTR_ENABLE2));
	printk("retval = 0x%x\n",retval);
#endif
	IRQ_ports[1] = info;

	info->flags |= ASYNC_INITIALIZED;
	
	restore_flags(flags);
	return 0;
	
errout:
	restore_flags(flags);
	return retval;
	
}

/*
 * This routine will shutdown a serial port; interrupts are disabled, and
 * DTR is dropped if the hangup on close termio flag is on.
 */
static void shutdown(struct async_struct * info)
{
        unsigned long   flags;

        if (!(info->flags & ASYNC_INITIALIZED))
                return;

#ifdef SERIAL_DEBUG_OPEN  
        printk("Shutting down serial port %d (irq %d)....", info->line,
               info->irq);
#endif
                           
        save_flags(flags); cli(); /* Disable interrupts */

        if (info->xmit.buf) {  
                free_page((unsigned long) info->xmit.buf);
                info->xmit.buf = 0;
        }              
       
        if (info->tty)
                set_bit(TTY_IO_ERROR, &info->tty->flags);

        info->flags &= ~ASYNC_INITIALIZED;
        restore_flags(flags);
}

/*
 * This routine is called to set the UART divisor registers to match
 * the specified baud rate for a serial port.
 */
static void change_speed(struct async_struct *info, struct termios *old_termios)
{
    unsigned int flags;
    unsigned int cflag, ctl1;
    struct _baudtbl *baudinfo;
    int bits = 0, baud, quot;

    if (!info->tty || !info->tty->termios)
	return;
    cflag = info->tty->termios->c_cflag;
  
    /*
      TX39 does not support 5 bits/char and 6 bits/char, so ignore them.
     */
    ctl1 = READ_TX39REG(OFF_UARTA_CTL1);

    switch(cflag & CSIZE) {
    case CS5: 	break;
    case CS6:	break;
    case CS7:	
	ctl1 &= ~TX39_BIT_MASK; ctl1 |= TX39_BIT_7; bits = 9; break;
    case CS8:
	ctl1 &= ~TX39_BIT_MASK; ctl1 |= TX39_BIT_8; bits = 10; break;
    default:	break;
    }
    
    if(cflag & CSTOPB) {
	ctl1 &= ~TX39_STOP_MASK; 
	ctl1 |= TX39_STOP_2;
	bits++;
    }

    switch(cflag & (PARENB | PARODD)) {
    case (PARENB | PARODD):
	ctl1 &= ~TX39_PARITY_MASK; ctl1 |= TX39_PARITY_ODD; bits++; break;
    case PARENB:
	ctl1 &= ~TX39_PARITY_MASK; ctl1 |= TX39_PARITY_EVEN; bits++; break;
    default:
	ctl1 &= ~TX39_PARITY_MASK; ctl1 |= TX39_PARITY_NONE; break;
    }

    baud = tty_get_baud_rate(info->tty);
    if ((baudinfo = getbaudvalbynum(baud, 0)) == NULL) {
	/* restore old termios if exist */
	if (old_termios) {
	    info->tty->termios->c_cflag &= ~CBAUD;
	    info->tty->termios->c_cflag |= (old_termios->c_cflag & CBAUD);
	    baud = tty_get_baud_rate(info->tty);
	    baudinfo = getbaudvalbynum(baud, 0);
	}
    }
    /* 
       As a last resort, if baud is not exist, default to TSBDEF_DEFAULT_BAUD
    */
    if (baudinfo == NULL) {
	baud = TSBREF_DEFAULT_BAUD;
	baudinfo = getbaudvalbynum(baud, 0);
    }
    info->quot = quot = baudinfo->value & machine_info->baudmask;
    info->timeout = ((info->xmit_fifo_size * HZ * bits)/baud);
    info->timeout += HZ/50;
   
    current_baud = baud;   //vincent  
 
    /*info->read_status_mask = UART_LSR_OE;*/
    //OR_TX39REG(INTR_UARTARXOVERRUNINT ,OFF_INTR_ENABLE2); 

    if (I_INPCK(info->tty)){
	info->read_status_mask |= UART_LSR_FE | UART_LSR_PE;
        //OR_TX39REG((INTR_UARTAFRAMEERRINT|INTR_UARTAPARITYERRINT),OFF_INTR_ENABLE2); 
    }
    if (I_BRKINT(info->tty) || I_PARMRK(info->tty)){
	info->read_status_mask |= UART_LSR_BI;
        //OR_TX39REG( INTR_UARTABREAKINT ,OFF_INTR_ENABLE2);  
    }
    /* Add by Fanky */
    info->ignore_status_mask = 0;
    if (I_IGNPAR(info->tty))
	info->ignore_status_mask |= UART_LSR_PE | UART_LSR_FE;
    if (I_IGNBRK(info->tty)) {
	info->ignore_status_mask |= UART_LSR_BI;
	/*
	 * If we're ignore parity and break indicators, ignore 
	 * overruns too.  (For real raw support).
	 */
	if (I_IGNPAR(info->tty))
	      info->ignore_status_mask |= UART_LSR_OE;
	}

    /* supporting hardflow : pending */
    /* Modified by Fanky */
        save_flags(flags); cli();
        WRITE_TX39REG(ctl1, OFF_UARTA_CTL1);
        WRITE_TX39REG(quot, OFF_UARTA_CTL2);
        restore_flags(flags);

    
    DEBUGINFO(("Serial: change_speed is called: cflags = %o\n", cflag));
}

static void tx39uart_put_char(struct tty_struct *tty, unsigned char ch)
{
#if UART_ENABLE
	struct async_struct *info = (struct async_struct *)tty->driver_data;
	unsigned long flags;

if (uart_enable)
{
#if 0	
	printk("Serial: put_char is called\n");
#endif

	if (!tty || !info->xmit.buf)
		return;

	save_flags(flags); cli();

	if (CIRC_CNT(info->xmit.head, info->xmit.tail, SERIAL_XMIT_SIZE) >= SERIAL_XMIT_SIZE - 1) {
		restore_flags(flags);
		return;
	}

	info->xmit.buf[info->xmit.head++] = ch;
	info->xmit.head &= SERIAL_XMIT_SIZE-1;
	restore_flags(flags);
}
#endif
}

static void tx39uart_flush_chars(struct tty_struct *tty)
{
	struct async_struct *info = (struct async_struct *)tty->driver_data;
	unsigned long flags;	

#if 0	
	printk("Serial: tx39uart_flush_chars is called\n");
#endif
	
	if (info->xmit.head == info->xmit.tail || tty->stopped || tty->hw_stopped ||
	    !info->xmit.buf)
		return;

	save_flags(flags); cli();
	info->IER |= UART_IER_THRI;
	TX39_ENABLE_TXINTR;
	restore_flags(flags);

}

static int tx39uart_write(struct tty_struct * tty, int from_user,
		    const unsigned char *buf, int count)
{
	int	c, ret = 0;	
	struct async_struct *info = (struct async_struct *)tty->driver_data;
	unsigned long flags;	

#if 0
	printk("Serial: tx39uart_write is called\n");
#endif
	
	if (!tty || !info->xmit.buf || !tmp_buf)
		return 0;

	save_flags(flags);
	if (from_user) {
		down(&tmp_buf_sem);
		while (1) {
			c = MIN(count,
				MIN(SERIAL_XMIT_SIZE - CIRC_CNT(info->xmit.head, info->xmit.tail, SERIAL_XMIT_SIZE) - 1,
				    SERIAL_XMIT_SIZE - info->xmit.head));
			if (c <= 0)
				break;

			c -= copy_from_user(tmp_buf, buf, c);
			if (!c) {
				if (!ret)
					ret = -EFAULT;
				break;
			}
			cli();
			c = MIN(c, MIN(SERIAL_XMIT_SIZE - CIRC_CNT(info->xmit.head, info->xmit.tail, SERIAL_XMIT_SIZE) - 1,
				       SERIAL_XMIT_SIZE - info->xmit.head));
			memcpy(info->xmit.buf + info->xmit.head, tmp_buf, c);
			info->xmit.head = ((info->xmit.head + c) &
					   (SERIAL_XMIT_SIZE-1));
			restore_flags(flags);
			buf += c;
			count -= c;
			ret += c;
		}
		up(&tmp_buf_sem);
	} else {
		while (1) {
			cli();		
			c = MIN(count,
				MIN(SERIAL_XMIT_SIZE - CIRC_CNT(info->xmit.head, info->xmit.tail, SERIAL_XMIT_SIZE) - 1,
				    SERIAL_XMIT_SIZE - info->xmit.head));
			if (c <= 0) {
				restore_flags(flags);
				break;
			}
			memcpy(info->xmit.buf + info->xmit.head, buf, c);
			info->xmit.head = ((info->xmit.head + c) & (SERIAL_XMIT_SIZE-1));
			restore_flags(flags);
			buf += c;
			count -= c;
			ret += c;
		}
	}
	
	if (CIRC_CNT(info->xmit.head, info->xmit.tail, SERIAL_XMIT_SIZE) && !tty->stopped && !tty->hw_stopped &&
	    !(info->IER & UART_IER_THRI)) {
		info->IER |= UART_IER_THRI;
		TX39_ENABLE_TXINTR;
	}
	return ret;
}

static int tx39uart_write_room(struct tty_struct *tty)
{
	struct async_struct *info = (struct async_struct *)tty->driver_data;
	int	ret;
	
	ret = SERIAL_XMIT_SIZE - CIRC_CNT(info->xmit.head, info->xmit.tail, SERIAL_XMIT_SIZE) - 1;
	
	if (ret < 0)
		ret = 0;
	return ret;
	
}

static int tx39uart_chars_in_buffer(struct tty_struct *tty)
{
	struct async_struct *info = (struct async_struct *)tty->driver_data;
	
	return CIRC_CNT(info->xmit.head, info->xmit.tail, SERIAL_XMIT_SIZE);
}

static void tx39uart_flush_buffer(struct tty_struct *tty)
{
	struct async_struct *info = (struct async_struct *)tty->driver_data;
	unsigned long flags;
	
	save_flags(flags); cli();
	info->xmit.head = info->xmit.tail = 0;
	restore_flags(flags);
	wake_up_interruptible(&tty->write_wait);
	if ((tty->flags & (1 << TTY_DO_WRITE_WAKEUP)) &&
	    tty->ldisc.write_wakeup)
		(tty->ldisc.write_wakeup)(tty);	
}

/*
 * This function is used to send a high-priority XON/XOFF character to
 * the device
 */
static void tx39uart_send_xchar(struct tty_struct *tty, char ch)
{
	struct async_struct *info = (struct async_struct *)tty->driver_data;

	info->x_char = ch;
	if (ch) {
		/* Make sure transmit interrupts are on */
		info->IER |= UART_IER_THRI;
		TX39_ENABLE_TXINTR;
	}
}

/*
 * ------------------------------------------------------------
 * tx39uart_throttle()
 * 
 * This routine is called by the upper-layer tty layer to signal that
 * incoming characters should be throttled.
 * ------------------------------------------------------------
 */
static void tx39uart_throttle(struct tty_struct * tty)
{
        //unsigned long flags;

	DEBUGINFO(("Serial: tx39uart_throttle is called\n"));

        if (I_IXOFF(tty)) 
        {
            tx39uart_send_xchar(tty, /*STOP_CHAR(tty)*/19);

        }
        if (tty->termios->c_cflag & CRTSCTS)
        {     /*
               * save_flags(flags); cli();
               * WRITE_TX39REG(IO_MFIOOUT(5),OFF_IO_MFIOOUT);
               * restore_flags(flags);
               */
        } 
}

static void tx39uart_unthrottle(struct tty_struct * tty)
{
  struct async_struct *info = (struct async_struct *)tty->driver_data;
  //unsigned long flags;

	DEBUGINFO(("Serial: tx39uart_unthrottle is called\n"));

        if (I_IXOFF(tty))
        {
	     if (info->x_char)
		info->x_char = 0;
	     else
		tx39uart_send_xchar(tty, /*START_CHAR(tty)*/17);
	}
        
        if (tty->termios->c_cflag & CRTSCTS)
        {
            /*
             * save_flags(flags); cli();
             * AND_TX39REG(~(IO_MFIOOUT(5)),OFF_IO_MFIOOUT);	
             * restore_flags(flags);
             */
        }  

}

/*
 * ------------------------------------------------------------
 * tx39uart_ioctl() and friends
 * ------------------------------------------------------------
 */

static int get_serial_info(struct async_struct * info,
			   struct serial_struct * retinfo)
{
	struct serial_struct tmp;
	struct serial_state *state = info->state;
   
	if (!retinfo)
		return -EFAULT;
	memset(&tmp, 0, sizeof(tmp));
	tmp.type = state->type;
	tmp.line = state->line;
	tmp.port = state->port;
	tmp.irq = state->irq;
	tmp.flags = state->flags;
	tmp.xmit_fifo_size = state->xmit_fifo_size;
	tmp.baud_base = state->baud_base;
	tmp.close_delay = state->close_delay;
	tmp.closing_wait = state->closing_wait;
	tmp.custom_divisor = state->custom_divisor;
	tmp.hub6 = state->hub6;
	if (copy_to_user(retinfo,&tmp,sizeof(*retinfo)))
		return -EFAULT;
	return 0;
}

static int set_serial_info(struct async_struct * info,
			   struct serial_struct * new_info)
{

	DEBUGINFO(("set_serial_info is called\n"));
	
	return 0;
}


/*
 * get_lsr_info - get line status register info
 *
 * Purpose: Let user call ioctl() to get info when the UART physically
 * 	    is emptied.  On bus types like RS485, the transmitter must
 * 	    release the bus after transmitting. This must be done when
 * 	    the transmit shift register is empty, not be done when the
 * 	    transmit holding register is empty.  This functionality
 * 	    allows an RS485 driver to be written in user space. 
 */
static int get_lsr_info(struct async_struct * info, unsigned int *value)
{
	DEBUGINFO(("get_lsr_info is called\n"));
	return 0;
}


static int get_modem_info(struct async_struct * info, unsigned int *value)
{
	DEBUGINFO(("get_modem_info is called\n"));
	return 0;
}

static int set_modem_info(struct async_struct * info, unsigned int cmd,
			  unsigned int *value)
{
	DEBUGINFO(("set_modem_info is called\n"));
	return 0;
}

/*
 * tx39uart_break() --- routine which turns the break handling on or off
 */
static void tx39uart_break(struct tty_struct *tty, int break_state)
{
	DEBUGINFO(("Serial: tx39uart_break is called\n"));
}


static int tx39uart_ioctl(struct tty_struct *tty, struct file * file,
		    unsigned int cmd, unsigned long arg)
{
  struct async_struct *info = (struct async_struct *)tty->driver_data;    

	if ((cmd != TIOCGSERIAL) && (cmd != TIOCSSERIAL) &&
	    (cmd != TIOCSERCONFIG) && (cmd != TIOCSERGSTRUCT) &&
	    (cmd != TIOCMIWAIT) && (cmd != TIOCGICOUNT)) {
		if (tty->flags & (1 << TTY_IO_ERROR))
		    return -EIO;
	}

	switch (cmd) {
		case TIOCMGET:
			DEBUGINFO(("TIOCMGET\n"));
			return 0;
		case TIOCMBIS:
		case TIOCMBIC:
		case TIOCMSET:
			DEBUGINFO(("TIOCMBIS\n"));			
			return 0;
			
		case TIOCGSERIAL:
			DEBUGINFO(("TIOCGSERIAL\n"));
                        return get_serial_info(info,
					       (struct serial_struct *) arg);			
			
			
		case TIOCSSERIAL:
			DEBUGINFO(("TIOCSSSERIAL\n"));
			return 0;

		case TIOCSERCONFIG:
			DEBUGINFO(("TIOCSERCONFIG\n"));
			return 0;

		case TIOCSERGETLSR: /* Get line status register */
			DEBUGINFO(("TIOCSERGETLSR\n"));			
			return 0;

		case TIOCSERGSTRUCT:
			DEBUGINFO(("TIOCSERGSTRUCT\n"));
			return 0;
				
		/*
		 * Wait for any of the 4 modem inputs (DCD,RI,DSR,CTS) to change
		 * - mask passed in arg for lines of interest
 		 *   (use |'ed TIOCM_RNG/DSR/CD/CTS for masking)
		 * Caller should use TIOCGICOUNT to see which one it was
		 */
		case TIOCMIWAIT:
			DEBUGINFO(("TIOCFICOUNT\n"));
			return 0;


		/* 
		 * Get counter of input serial line interrupts (DCD,RI,DSR,CTS)
		 * Return: write counters to the user passed counter struct
		 * NB: both 1->0 and 0->1 transitions are counted except for
		 *     RI where only 0->1 is counted.
		 */
		case TIOCGICOUNT:
			DEBUGINFO(("TIOCFICOUNT\n"));
			return 0;

		case TIOCSERGWILD:
		case TIOCSERSWILD:
			/* "setserial -W" is called in Debian boot */
			DEBUGINFO( ("TIOCSER?WILD ioctl obsolete, ignored.\n"));
			return 0;

		default:
			return -ENOIOCTLCMD;
		}
	
	return 0;	

}

static void tx39uart_set_termios(struct tty_struct *tty, struct termios *old_termios)
{
	struct async_struct *info = (struct async_struct *)tty->driver_data;
        unsigned long flags;
         
	if (tty->termios->c_cflag == old_termios->c_cflag)
	  return;
        
        
   
	change_speed(info, NULL);

        /* Add by Fanky */
        save_flags(flags); cli();
        if (tty->termios->c_cflag & CRTSCTS)
        {
          DEBUGINFO(("CTS/RTS flow control enable.\n"));
          TX39_ENABLE_MIO18POS;
          TX39_ENABLE_MIO18NEG;
        }
        else
        {
          DEBUGINFO(("CTS/RTS flow control disable.\n"));
          TX39_DISABLE_MIO18POS; 
          TX39_DISABLE_MIO18NEG; 
        }
        restore_flags(flags);
 
	if ((old_termios->c_cflag & CRTSCTS) &&
	    !(tty->termios->c_cflag & CRTSCTS)) {
		tty->hw_stopped = 0;
		tx39uart_start(tty);
	}
}

/*
 * ------------------------------------------------------------
 * tx39uart_close()
 * 
 * This routine is called when the serial port gets closed.  First, we
 * wait for the last remaining data to be sent.  Then, we unlink its
 * async structure from the interrupt chain if necessary, and we free
 * that IRQ if nothing is left in the chain.
 * ------------------------------------------------------------
 */
static void tx39uart_close(struct tty_struct *tty, struct file * filp)
{
	struct async_struct * info = (struct async_struct *)tty->driver_data;
	struct serial_state *state;
	unsigned long flags;

	state = info->state;
	
	save_flags(flags); cli();
	
	if (tty_hung_up_p(filp)) {
		MOD_DEC_USE_COUNT;
		tx39uart_use_count--;
		restore_flags(flags);
		return;
	}
	
#ifdef SERIAL_DEBUG_OPEN
	printk("tx39uart_close ttys%d, count = %d\n", info->line, state->count);
#endif
	if ((tty->count == 1) && (state->count != 1)) {
		/*
		 * Uh, oh.  tty->count is 1, which means that the tty
		 * structure will be freed.  state->count should always
		 * be one in these conditions.  If it's greater than
		 * one, we've got real problems, since it means the
		 * serial port won't be shutdown.
		 */
		printk("tx39uart_close: bad serial port count; tty->count is 1, "
		       "state->count is %d\n", state->count);
		state->count = 1;
	}
	if (--state->count < 0) {
		printk("tx39uart_close: bad serial port count for ttys%d: %d\n",
		       info->line, state->count);
		state->count = 0;
	}
	if (state->count) {
		MOD_DEC_USE_COUNT;
		tx39uart_use_count--;
		restore_flags(flags);
		return;
	}
	info->flags |= ASYNC_CLOSING;
	/*
	 * Save the termios structure, since this port may have
	 * separate termios for callout and dialin.
	 */
	if (info->flags & ASYNC_NORMAL_ACTIVE)
		info->state->normal_termios = *tty->termios;
	if (info->flags & ASYNC_CALLOUT_ACTIVE)
		info->state->callout_termios = *tty->termios;
	/*
	 * Now we wait for the transmit buffer to clear; and we notify 
	 * the line discipline to only process XON/XOFF characters.
	 */
	tty->closing = 1;
	if (info->closing_wait != ASYNC_CLOSING_WAIT_NONE)
		tty_wait_until_sent(tty, info->closing_wait);
	/*
	 * At this point we stop accepting input.  To do this, we
	 * disable the receive line status interrupts, and tell the
	 * interrupt driver to stop checking the data ready bit in the
	 * line status register.
	 */
	info->IER &= ~UART_IER_RLSI;
	info->read_status_mask &= ~UART_LSR_DR;
	if (info->flags & ASYNC_INITIALIZED) {
		/*serial_out(info, UART_IER, info->IER);*/
		/*
		 * Before we drop DTR, make sure the UART transmitter
		 * has completely drained; this is especially
		 * important if there is a transmit FIFO!
		 */
		tx39uart_wait_until_sent(tty, info->timeout);
	}
	shutdown(info);
	if (tty->driver.flush_buffer)
		tty->driver.flush_buffer(tty);
	if (tty->ldisc.flush_buffer)
		tty->ldisc.flush_buffer(tty);
	tty->closing = 0;
	info->event = 0;
	info->tty = 0;
	if (info->blocked_open) {
		if (info->close_delay) {
			current->state = TASK_INTERRUPTIBLE;
			schedule_timeout(info->close_delay);
		}
		wake_up_interruptible(&info->open_wait);
	}
	info->flags &= ~(ASYNC_NORMAL_ACTIVE|ASYNC_CALLOUT_ACTIVE|
			 ASYNC_CLOSING);
	//wake_up_interruptible(&info->close_wait);
	MOD_DEC_USE_COUNT;
	tx39uart_use_count--;
//        if (!MOD_IN_USE)	//problem !! it not work
	if( tx39uart_use_count <=0)	//vincent : we use this instead.
            free_irq(1,NULL);
	restore_flags(flags);
}

/*
 * tx39uart_wait_until_sent() --- wait until the transmitter is empty
 */
static void tx39uart_wait_until_sent(struct tty_struct *tty, int timeout)
{
	struct async_struct * info = (struct async_struct *)tty->driver_data;
	unsigned long orig_jiffies, char_time;
	
	if (info->state->type == PORT_UNKNOWN)
		return;

	if (info->xmit_fifo_size == 0)
		return; /* Just in case.... */

	orig_jiffies = jiffies;
	/*
	 * Set the check interval to be 1/5 of the estimated time to
	 * send a single character, and make it at least 1.  The check
	 * interval should also be less than the timeout.
	 * 
	 * Note: we have to use pretty tight timings here to satisfy
	 * the NIST-PCTS.
	 */
	char_time = (info->timeout - HZ/50) / info->xmit_fifo_size;
	char_time = char_time / 5;
	if (char_time == 0)
		char_time = 1;
	if (timeout)
	  char_time = MIN(char_time, timeout);
	
	while (!(READ_TX39REG(OFF_UARTA_CTL1) & UART_EMPTY)) {
		
		current->state = TASK_INTERRUPTIBLE;
		current->counter = 0;	/* make us low-priority */
		schedule_timeout(char_time);
		if (signal_pending(current))
			break;
		if (timeout && ((orig_jiffies + timeout) < jiffies))
			break;
	}
	
	current->state = TASK_RUNNING;
}

/*
 * tx39uart_hangup() --- called by tty_hangup() when a hangup is signaled.
 */
static void tx39uart_hangup(struct tty_struct *tty)
{
	DEBUGINFO(("Serial: tx39uart_hangup is called\n"));
}

/*
 * ------------------------------------------------------------
 * tx39uart_open() and friends
 * ------------------------------------------------------------
 */
static int block_til_ready(struct tty_struct *tty, struct file * filp,
			   struct async_struct *info)
{
	DEBUGINFO(("block_till_ready is called"));
	return 0;
}

static int get_async_struct(int line, struct async_struct **ret_info)
{

	struct async_struct *info;
	struct serial_state *sstate;

	sstate = tx39uart_table + line;
	sstate->count++;
	if (sstate->info) {
		*ret_info = sstate->info;
		return 0;
	}
	
	info = kmalloc(sizeof(struct async_struct), GFP_KERNEL);
	if (!info) {
		sstate->count--;
		return -ENOMEM;
	}
	memset(info, 0, sizeof(struct async_struct));
	info->magic = SERIAL_MAGIC;
	info->port = sstate->port;
	info->flags = sstate->flags;
	info->xmit_fifo_size = sstate->xmit_fifo_size;
	info->line = line;
	info->tqueue.routine = do_softint;
	info->tqueue.data = info;
	info->state = sstate;
	if (sstate->info) {
		kfree(info);
		*ret_info = sstate->info;
		return 0;
	}
	*ret_info = sstate->info = info;
	return 0;	
}

/*
 * This routine is called whenever a serial port is opened.  It
 * enables interrupts for a serial port, linking in its async structure into
 * the IRQ chain.   It also performs the serial-specific
 * initialization for the tty structure.
 */
static int tx39uart_open(struct tty_struct *tty, struct file * filp)
{
	struct async_struct	*info;
	int 			retval, line;
	unsigned long		page;

#if 0		// move to tty_io.c tty_open
#ifndef CONFIG_SERIAL_CONSOLE		// colin : 20010117
	if ((retval = uartopen_check()) < 0)
		return retval;
#endif
#endif

	MOD_INC_USE_COUNT;
	tx39uart_use_count++;
	line = MINOR(tty->device) - tty->driver.minor_start;
#if    	0
	printk("tx39uart_open %x : %x : %x\n", MINOR(tty->device),
			tty->driver.minor_start, line);
#endif

	retval = get_async_struct(line, &info);

	if (retval) {
		MOD_DEC_USE_COUNT;
		tx39uart_use_count--;
		return retval;
	}

	tty->driver_data = info;
	info->tty = tty;
	
        info->tty->low_latency = (info->flags & ASYNC_LOW_LATENCY) ? 1 : 0;
	retval = startup(info);


	if (!tmp_buf) {
		page = get_free_page(GFP_KERNEL);
		if (!page) {
			/* MOD_DEC_USE_COUNT; "info->tty" will cause this? */
			return -ENOMEM;
		}
		if (tmp_buf)
			free_page(page);
		else
			tmp_buf = (unsigned char *) page;
	}
	
#ifdef CONFIG_TX39UART_CONSOLE
     
	if (sercons.cflag && sercons.index == line) {
		tty->termios->c_cflag = sercons.cflag;
		sercons.cflag = 0;
		change_speed(info, NULL);
	}
#endif
        /* Add by Fanky 2001-5-16*/
       if ((info->state->count == 1) &&
	    (info->flags & ASYNC_SPLIT_TERMIOS)) {
		if (tty->driver.subtype == SERIAL_TYPE_NORMAL)
			*tty->termios = info->state->normal_termios;
		else 
			*tty->termios = info->state->callout_termios;
		
	}  

        //change_speed(info, NULL);
        info->session = current->session;
	info->pgrp = current->pgrp;
        /* Add by Fanky ; reset DMA buffer 2001-5-16 */
	/* modify by colin : 20020403 */
        dma_buf_pos = READ_TX39REG(OFF_UARTA_DMA_COUNT) & 0x0000ffff;
        WRITE_TX39REG(INTR_UARTARXINT, OFF_INTR_CLEAR2);
       
	return retval;
}

/*
 * /proc fs routines....
 */

static inline int line_info(char *buf, struct serial_state *state)
{
	DEBUGINFO(("line_info called\n"));
	return 0;
}

int tx39uart_read_proc(char *page, char **start, off_t off, int count,
		 int *eof, void *data)
{
	DEBUGINFO(("tx39uart_read_proc called\n"));
	return 0;
}

/*
 * ---------------------------------------------------------------------
 * tx39uart_init() and friends
 *
 * tx39uart_init() is called at boot-time to initialize the serial driver.
 * ---------------------------------------------------------------------
 */

/*
 * This routine prints out the appropriate serial driver version
 * number, and identifies which options were configured into this
 * driver.
 */
static _INLINE_ void show_serial_version(void)
{
 	printk(KERN_INFO "%s version %s\n", serial_name, serial_version);
}

static unsigned char __dma_buffer[256];

static void UART_DMA_INIT(void)
{

 unsigned long dma_address;

     dma_buf =(unsigned char *)__dma_buffer;
     
     if (dma_buf == NULL )
           printk("tx39uart : Fail of UART DMA allocation.\n");
     dma_address = virt_to_bus(dma_buf);
         
     WRITE_TX39REG(dma_address ,OFF_UARTA_DMA_CTL1);
     WRITE_TX39REG(255,OFF_UARTA_DMA_CTL2);

}

/*
 * The serial driver boot-time initialization code!
 */
//__initfunc(int tx39uart_init(void))
int __init rs_init(void)
{
	int i;
	struct serial_state * state;
	extern void atomwide_serial_init (void);
	extern void dualsp_serial_init (void);
	//void (*handler)(int, void *, struct pt_regs *);
    UART_DMA_INIT();
	tx39_initialize(TSBREF_DEFAULT_BAUD, TX39_PARITY_NONE, TX39_BIT_8, TX39_STOP_1);

//#ifndef CONFIG_SERIAL_CONSOLE		// colin : 20020306
//	DisableUARTA();
//#endif
	
#if 0	
	/*
	 *	The interrupt of the serial console port
	 *	can't be shared.
	 */
	if (sercons.flags & CON_CONSDEV) {
		for(i = 0; i < NR_PORTS; i++)
			if (i != sercons.index &&
			    tx39uart_table[i].irq == tx39uart_table[sercons.index].irq)
				tx39uart_table[i].irq = 0;
	}
#endif	

	init_bh(SERIAL_BH, do_serial_bh);	// colin :  20010626
	
	show_serial_version();

	/* Initialize the tty_driver structure */
	
	memset(&serial_driver, 0, sizeof(struct tty_driver));
	serial_driver.magic = TTY_DRIVER_MAGIC;
	serial_driver.driver_name = "serial";
#ifdef	CONFIG_SERIAL_CONSOLE
	serial_driver.name = "ttyS";
#else
	serial_driver.name = "ttyG";	// DJH
#endif
	serial_driver.major = TTY_MAJOR;
#ifdef	CONFIG_SERIAL_CONSOLE
	serial_driver.minor_start = 64;
#else
	//serial_driver.minor_start = 72;	// DJH
	serial_driver.minor_start = 127;	// colin
#endif
	serial_driver.num = NR_PORTS;
	serial_driver.type = TTY_DRIVER_TYPE_SERIAL;
	serial_driver.subtype = SERIAL_TYPE_NORMAL;
	serial_driver.init_termios = tty_std_termios;
	serial_driver.init_termios.c_cflag =
	    def_cflags;
	serial_driver.flags = TTY_DRIVER_REAL_RAW;
	serial_driver.refcount = &serial_refcount;
	serial_driver.table = serial_table;
	serial_driver.termios = serial_termios;
	serial_driver.termios_locked = serial_termios_locked;

	serial_driver.open = tx39uart_open;
	serial_driver.close = tx39uart_close;
	serial_driver.write = tx39uart_write;
	serial_driver.put_char = tx39uart_put_char;
	serial_driver.flush_chars = tx39uart_flush_chars;
	serial_driver.write_room = tx39uart_write_room;
	serial_driver.chars_in_buffer = tx39uart_chars_in_buffer;
	serial_driver.flush_buffer = tx39uart_flush_buffer;
	serial_driver.ioctl = tx39uart_ioctl;
	serial_driver.throttle = tx39uart_throttle;
	serial_driver.unthrottle = tx39uart_unthrottle;
	serial_driver.send_xchar = tx39uart_send_xchar;
	serial_driver.set_termios = tx39uart_set_termios;
	serial_driver.stop = tx39uart_stop;
	serial_driver.start = tx39uart_start;
	serial_driver.hangup = tx39uart_hangup;
	serial_driver.break_ctl = tx39uart_break;
	serial_driver.wait_until_sent = tx39uart_wait_until_sent;
	serial_driver.read_proc = tx39uart_read_proc;

	if (tty_register_driver(&serial_driver))
		panic("Couldn't register serial driver\n");

	TX39_ENABLE_RXINTR;
	for (i = 0, state = tx39uart_table; i < NR_PORTS; i++,state++) {
		state->magic = SSTATE_MAGIC;
		state->line = i;
		state->type = PORT_UNKNOWN;
		state->custom_divisor = 0;
		state->close_delay = 5*HZ/10;
		state->closing_wait = 30*HZ;
		state->callout_termios = callout_driver.init_termios;
		state->normal_termios = serial_driver.init_termios;
		state->icount.cts = state->icount.dsr = 
			state->icount.rng = state->icount.dcd = 0;
		state->icount.rx = state->icount.tx = 0;
		state->icount.frame = state->icount.parity = 0;
		state->icount.overrun = state->icount.brk = 0;
		state->irq = irq_cannonicalize(state->irq);
		if (check_region(state->port,8))
			continue;
	}
      
	return 0;
}

static void setup_machine_info(void) 
{
    struct machine_tbl *p;
    unsigned int csr;

    if (machine_info == NULL) {
#if 0    	
		for (p = cputypes; p < &cputypes[SUPPORT_CPUS]; p++) {
	    	if (p->cpu_type == mips_machtype) machine_info = p;
		}
#endif
		if (machine_info == NULL) machine_info = &cputypes[0];

		printk("setup_machine_info:Cpu type is %d\n", machine_info - cputypes);

		csr = READ_TX39REG(OFF_CLOCK_CTL);
		csr |= machine_info->clock_config;
		WRITE_TX39REG( csr, OFF_CLOCK_CTL );
    } else {
		printk("setup_machine_info:Already setup for Cpu type is %d\n", machine_info - cputypes);	
    }
}
static struct _baudtbl *getbaudvalbynum(int reqbaud, int fallbackbaud) 
{
    struct _baudtbl *preq = NULL, *pfallback = NULL, *p;

    while (pfallback == NULL) {
	for(p = baudtbl1; p->number != 0; p++) {
	    if(p->number == reqbaud) preq = p;
	    if(p->number == fallbackbaud) pfallback = p;
	}
	if (fallbackbaud == 0 || pfallback != NULL) break;
	fallbackbaud = 9600;	/* this value must exist in baudtable */
    }
    return (preq != NULL)?preq:pfallback;

}

void tx39_initialize(int baud, int parity, int bits, int stop)
{
	unsigned int csr;
        //unsigned int miosel,miodir;
	struct _baudtbl *baudinfo;
	
	/* get machine information */
      
	DEBUGINFO(("KERNEL : tx39_initialize......\n"));
	setup_machine_info();

	/* TX39 Internal UART setup */
	
	WRITE_TX39REG(UART_DISTXD, OFF_UARTA_CTL1);

	/* All disable interrupt about serial */
//	WRITE_TX39REG(0, OFF_INTR_ENABLE2);
	AND_TX39REG( ~ (0xffc00000), OFF_INTR_ENABLE2);
TX39_CLEAN_UARTA_INT;
	csr = READ_TX39REG( OFF_UARTA_CTL1 ); /* Read current serial status */
	csr &= ~(TX39_PARITY_MASK | TX39_BIT_MASK | TX39_STOP_MASK); 
	csr |= (parity | bits | stop);	/* setup new parity, word size, stop bits */
	WRITE_TX39REG( csr, OFF_UARTA_CTL1 );

        /* Add by Fanky ;Set h/w flow control */
        /* initialize MFIO registers */
        /* CTS : MIOSEL[18] ; RTS : MIOSEL[5] */
        /*
         *miosel = READ_TX39REG( OFF_IO_MFIOSEL);
         *WRITE_TX39REG( TX39_MIO_RTS|TX39_MIO_CTS|miosel,OFF_IO_MFIOSEL);
         *miodir = READ_TX39REG( OFF_IO_MFIODIREC);
         *WRITE_TX39REG( (~TX39_MIO_CTS & miodir)|TX39_MIO_RTS,OFF_IO_MFIODIREC);
         */

	/* 
	 *  Set baud rate 
	 */
	baudinfo = getbaudvalbynum(baud, TSBREF_DEFAULT_BAUD);
	if (def_cflags == 0) {
	    def_cflags = baudinfo->bnumber;

	    switch(parity) {
	    case TX39_PARITY_ODD: 
		def_cflags |= PARENB | PARODD; break;
	    case TX39_PARITY_EVEN:
		def_cflags |= PARENB; break;
	    case TX39_PARITY_NONE:
	    default:
			break;/* nothing to do */
	    }

	    switch(bits) {
	    case TX39_BIT_7:
		def_cflags |= CS7; break;
	    case TX39_BIT_8:
	    default:
		def_cflags |= CS8; break;
	    }

	    switch(stop) {
	    case TX39_STOP_2:
		def_cflags |= CSTOPB; break;
	    case TX39_STOP_1:
	    default:
			break;/* do nothing */
	    }
	}
	baud = baudinfo->value & machine_info->baudmask;
        
        OR_TX39REG(UART_ENDMARX,OFF_UARTA_CTL1);
        OR_TX39REG(UART_ENDMALOOP,OFF_UARTA_CTL1);
        
	WRITE_TX39REG(baud, OFF_UARTA_CTL2);

	/* Starting Serial Drivers */

	csr = READ_TX39REG( OFF_UARTA_CTL1 );
	csr &= (~UART_DISTXD);
	WRITE_TX39REG( csr, OFF_UARTA_CTL1 );
	csr |= UART_ENUART ;
	WRITE_TX39REG( csr, OFF_UARTA_CTL1 );
        
	//printk("OFF_UARTA_CTL1 = 0x%08lx\n",READ_TX39REG(OFF_UARTA_CTL1));
	//printk("OFF_UARTA_CTL2 = 0x%08lx\n",READ_TX39REG(OFF_UARTA_CTL2));

        
}

/*
 * ------------------------------------------------------------
 * Serial console driver
 * ------------------------------------------------------------
 */

/*
 *	Wait for transmitter & holding register to empty
 */
static inline void wait_for_xmitr(struct async_struct *info)
{
	unsigned int tmout = 1000000;
	unsigned int intr_status;
	
	do {
		intr_status = READ_TX39REG(OFF_INTR_STATUS2);
		if (--tmout == 0) break;
	} while (!(intr_status & INTR_UARTATXINT));
}

/*
 *	Print a string to the serial port trying not to disturb
 *	any possible real use of the port...
 */
static void tx39_console_write(struct console *co, const char *s,
				unsigned count)
{
#if UART_ENABLE
	struct serial_state *ser;
	unsigned i;
	struct async_struct scr_info; /* serial_{in,out} because HUB6 */

if (uart_enable)
{
        //printk("tx39_console_write(). %s\n",s);		// colin : BUG....
#if 0        
	ser = tx39uart_table + co->index;
	scr_info.magic = SERIAL_MAGIC;
	scr_info.port = ser->port;
	scr_info.flags = ser->flags;
#endif	
	/*
	 *	Now, do each character
	 */
	for (i = 0; i < count; i++, s++) {
		wait_for_xmitr(&scr_info);
		/*
		 *	Send the character out.
		 *	If a LF, also do CR...
		 */
		
		WRITE_TX39REG( INTR_UARTATXINT, OFF_INTR_CLEAR2 );
		WRITE_TX39REG( UART_TXDATA(*s), OFF_UARTA_TXDATA );
		
		if (*s == 10) {
			wait_for_xmitr(&scr_info);
			WRITE_TX39REG( INTR_UARTATXINT, OFF_INTR_CLEAR2 );
			WRITE_TX39REG( UART_TXDATA(13), OFF_UARTA_TXDATA );
		}
	}
	/*
	 *	Finally, Wait for transmitter & holding register to empty
	 * 	and restore the IER
	 */
	wait_for_xmitr(&scr_info);
}
#endif
}

/*
 *	Receive character from the serial port
 */
static int tx39_console_wait_key(struct console *co)
{
	DEBUGINFO(("serial_console_wait_key\n"));

	return 0;
}

static kdev_t tx39_console_device(struct console *c)
{
	return MKDEV(TTY_MAJOR, 64);
}

/*
 *	Setup initial baud/bits/parity. We do two things here:
 *	- construct a cflag setting for the first tx39uart_open()
 *	- initialize the serial port
 *	Return non-zero if we didn't find a serial port.
 */
static __inline int tx39_console_setup(struct console *co, char *options)
{
    /*
      option passed line description for console, like this
      baud parity bit
      baud : [0-9]*
      parity: 'o' |'O'| 'e' | 'E'	('o' means parity odd, 'e' means parity even)
      bits : [0-9]
     */
    int baud=TSBREF_DEFAULT_BAUD, parity=TX39_PARITY_NONE, bits=TX39_BIT_8, stopbits;
    char *s;

    printk("console_setup param:%s\n", options);

    if (options) {
	baud = simple_strtoul(options, NULL, 10);
	s = options;
	while(*s >= '0' && *s <= '9')
	    s++;
	if (*s)
	    parity = *s++;
	if (*s)
	    bits   = *s - '0';

	switch(parity) {
	case 'o':
	case 'O':
	    parity = TX39_PARITY_ODD; break;
	case 'e':
	case 'E':
	    parity = TX39_PARITY_EVEN; break;
	default:
	    parity = TX39_PARITY_NONE; break;
	}

	switch(bits) {
	case 7: 
	    bits = TX39_BIT_7; break;
	case 8:
	default:
	    bits = TX39_BIT_8; break;
	}
    }

    stopbits = TX39_STOP_1; 
    tx39_initialize(baud, parity, bits, stopbits);
	
    return 0;
}

static struct console tx39uart_cons = {
	name:	"ttyS",
	write:	tx39_console_write,
	read:	NULL,
	device:	tx39_console_device,
	setup:	tx39_console_setup,
	flags:	0,
	index:	CON_PRINTBUFFER,
	cflag:	-1,
	next:	0,
};

extern struct console dbg_console;
/*
 *	Register console.
 */
//__initfunc (long tx39_console_init(long kmem_start, long kmem_end))
long __init tx39_serial_console_init(long kmem_start, long kmem_end)
{
	/* colin : 20020314  because boot disable uart */
#ifdef CONFIG_SERIAL_CONSOLE
        AND_TX39REG(~(IO_MFIOOUT(23)|IO_MFIOOUT(24)), OFF_IO_MFIODIREC);
        AND_TX39REG(~(IO_MFIOSEL(23)|IO_MFIOSEL(24)),OFF_IO_MFIOSEL);
#endif

	printk("serial_console_init\n");
	//unregister_console(&dbg_console);
	register_console(&tx39uart_cons);
	return kmem_start;
}

#ifndef	CONFIG_SERIAL_CONSOLE	// DJH : add for debug
void tx39_xxx_write(const char *s, unsigned count)
{
	for(; count; count--)
	{
		if( *s == '\n' )
			uart_putc('\r');
		uart_putc(*s++);
	}
}
#endif
