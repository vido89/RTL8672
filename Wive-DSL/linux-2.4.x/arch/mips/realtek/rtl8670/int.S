/*
*  int_handler.S
*/

#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/stackframe.h>

		.text
		.set	noreorder
		.set	noat
		.align	5
		.extern sim_handle_interrupt
				
		NESTED(sim_handle_int, PT_SIZE, sp)
		SAVE_ALL
		CLI
		
		jal		sim_handle_interrupt
		move	a0, sp
		
		j		ret_from_irq
		nop
		
		END(sim_handle_int)
		
		.globl	set_cp0_status
set_cp0_status:
		mfc0	a2, CP0_STATUS
		nop
		nop
		nop
		or		a2,a2,a1
		mtc0	a2, CP0_STATUS
		jr		ra
		nop
		
/* Jonah + for hrchen's code*/
        .globl  get_cp0_cause
        .ent    get_cp0_cause
get_cp0_cause:
                mfc0    v0, CP0_CAUSE
                jr              ra
                nop
        .end get_cp0_cause

