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
         * init profiler
         */
	la t0, rlx_prof_init
	jalr ra, t0
	nop

        /*
         * set counting mode
         */
	la t0, rlx_prof_set_cp3_ctrl
        li a0, CP3_PERFMODE1
	jalr ra, t0
	nop

        /*
         * start profiler
         */
	la t0, rlx_prof_start
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
  /*
   * save current return value
   */
  move s0, a0

  /*
   * stop profiler
   */
  la t0, rlx_prof_stop
  jalr ra,t0
  nop

  /*
   * save profiler output
   */
  la t0, rlx_prof_save_result
  jalr ra,t0
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
