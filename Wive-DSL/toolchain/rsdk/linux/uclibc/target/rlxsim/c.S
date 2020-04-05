/*
 * Realtek Semiconductor Corp.
 *
 * c.S:
 */
#define UART_DATA_ADDRESS    0xbc00000c

#define zero	$0
#define AT	$1
#define v0 	$2
#define v1	$3

#define a0	$4
#define a1	$5
#define a2	$6
#define	a3	$7

#if #abi(n32) || #abi(n64) || #abi(64)
#define a4	$8
#define a5	$9
#define a6	$10
#define	a7	$11
#define ta0	$8	/* alias for $a4 */
#define ta1	$9	/* alias for $a5 */
#define ta2	$10	/* alias for $a6 */
#define ta3	$11	/* alias for $a7 */
#else
#define t0	$8
#define t1	$9
#define t2	$10
#define t3	$11
#define ta0	$12	/* alias for $t4 */
#define ta1	$13	/* alias for $t5 */
#define ta2	$14	/* alias for $t6 */
#define ta3	$15	/* alias for $t7 */
#endif

#define t4	$12
#define t5	$13
#define t6	$14
#define t7	$15

#define s0	$16
#define s1	$17
#define s2	$18
#define s3	$19
#define s4	$20
#define s5	$21
#define s6	$22
#define s7	$23
#define s8	$30		/* == fp */

#define t8	$24
#define t9	$25
#define k0	$26
#define k1	$27

#define gp	$28

#define sp	$29
#define fp	$30
#define ra	$31

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
	syscall 1
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
