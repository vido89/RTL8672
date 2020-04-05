/*
 * Realtek Semiconductor Corp.
 *
 * c.S:
 */
#include <regdef.h>
#include "rlx_library.h"

#define UART_DATA_ADDRESS    0xbc00000c

.set nomips16

	.globl	_stack_base
_stack_base:
	.rept 512*1024
	.byte 0
	.endr

	.balign 16
	.globl	_stack 
_stack:
	.rept 8
	.long 0
	.endr

	.data
	.globl my_argv
my_argv:
	.rept 16
	.byte 0
	.endr

	.data
	.globl my_arg_str
my_arg_str:
	.rept 256
	.byte 0
	.endr

	.comm cp3_counter,32

	.text
	
	.globl start
	.globl _exit
	

	.ent start
start:
	/*
	 * Initialize COP3 counters
   */
	la t0, rlx_cp3_init
	jalr ra, t0
	nop
	la t0, rlx_cp3_start
	li a0, CP3_PERFMODE1
	jalr ra, t0
	nop
	
	/*
	** Initialize data.
	*/
	la	v0, _edata
	/*
	** Align pointer.
	*/
	add	v0, 3
	srl	v0, 2
	sll	v0, 2
	la	v1, end
loop0:	sw	zero, 0x0(v0)
	sw	zero, 0x4(v0)
	sw	zero, 0x8(v0)
	sw	zero, 0xc(v0)
	addu	v0, 16
	blt	v0, v1, loop0

	/*
	** Initialize the global data pointer.
	*/
	la	gp, _gp
    la  sp, _stack

	/*
	** Every good C program has a main.
	*/
#if defined(__mips16)
	jalx	main
#else
	jal	main
#endif
	nop
	move	a0, v0
	.end start
	
	.ent _exit
_exit:
  move s0, a0
  la t0, rlx_cp3_get_counters
  la a0, cp3_counter
  jalr ra, t0
  nop
  la t0, rlx_cp3_print_counters
  li a0, CP3_PERFMODE1
  la a1, cp3_counter
  jalr ra, t0
  nop
  move a0, s0
  syscall 1
.end _exit 

	/*
	** PutC
	** Instruction used to perform character output
	** from programs running during simulation.
	*/
	.globl	__PutCharacter
	.ent	__PutCharacter
__PutCharacter:
 	li	a0, UART_DATA_ADDRESS
 	sw	a1, 0(a0)
	j	ra	# Return
	.end	__PutCharacter

.comm   __errno,4
