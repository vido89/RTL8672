#include "include/regdef.h"

#define UART_DATA_ADDRESS    0xbc00000c

	.globl	_stack_base
_stack_base:
	.rept 4096
	.byte 0
	.endr

	.balign 16
	.globl	_stack 
_stack:
	.rept 8
	.long 0
	.endr

	.text
	
	.globl start
	.globl _exit
	

	.ent start
start:

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
	jal	main
	nop
	move	a0, v0
	.end start
	
	.ent _exit
_exit:
	move	v0, a0

stop_loop:
 	b	stop_loop
    nop
	.end	_exit

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
