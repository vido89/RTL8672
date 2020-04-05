#ifndef	__INTDEF_H
#define	__INTDEF_H

// 2001.02.01 updated from glchen
#include "config.h"

#define	_INTNUM_DECERR		15		// DECODE
#define	_INTNUM_EXCEPTION	14		//
#define	_INTNUM_TIMER1		13		//
#define	_INTNUM_TIMER0		12		//
#define	_INTNUM_PIC_END		11		//
#define	_INTNUM_FIELD_END	10		//
#define	_INTNUM_H_DEVICE_INT	9		// 
#define	_INTNUM_H_PIO_INT	8		//
#ifdef DVD728
#define	_INTNUM_UART0_INT	6		// UART0
#define	_INTNUM_UART1_INT	7		// UART1
#else
#define	_INTNUM_UART0_INT	7		// UART0
#define	_INTNUM_UART1_INT	6		// UART1
#endif
#define	_INTNUM_X_UNMAP_FLAG	5		// IOP un-map
#define	_INTNUM_UNMAP_ERROR	4		// RISC un-map
#define	_INTNUM_RISC_INT3	3		// ext. INT3
#define	_INTNUM_RISC_INT2	2		// ext. INT2
#define	_INTNUM_RISC_INT1	1		// ext. INT1
#define	_INTNUM_RISC_INT0	0		// ext. INT0

#define INTR_DECERR		(1<<_INTNUM_DECERR)
#define	INTR_EXCEPTION		(1<<_INTNUM_EXCEPTION)
#define INTR_TIMER1             (1<<_INTNUM_TIMER1)
#define INTR_TIMER0             (1<<_INTNUM_TIMER0)
#define INTR_PIC_END            (1<<_INTNUM_PIC_END)
#define INTR_FIELD_END          (1<<_INTNUM_FIELD_END)
#define INTR_H_DEVICE_INT       (1<<_INTNUM_H_DEVICE_INT)
#define INTR_H_PIO_INT          (1<<_INTNUM_H_PIO_INT)
#define INTR_UART0_INT		(1<<_INTNUM_UART0_INT)
#define INTR_UART1_INT		(1<<_INTNUM_UART1_INT)
#define INTR_X_UNMAP_FLAG	(1<<_INTNUM_X_UNMAP_FLAG)
#define INTR_UNMAP_ERROR	(1<<_INTNUM_UNMAP_ERROR)
#define INTR_RISC_INT3		(1<<_INTNUM_RISC_INT3)
#define INTR_RISC_INT2		(1<<_INTNUM_RISC_INT2)
#define INTR_RISC_INT1		(1<<_INTNUM_RISC_INT1)
#define INTR_RISC_INT0		(1<<_INTNUM_RISC_INT0)

#define	INTR_USED_MASK		INTR_MASK

#endif/*__INTDEF_H*/
