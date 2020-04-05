#ifndef __ASM_MACH_KERNEL_ENTRY_INIT_H
#define __ASM_MACH_KERNEL_ENTRY_INIT_H

	.macro  kernel_entry_setup
        .set    push
        .set    noreorder
        
        
	#--- initialize and start COP3
        mfc0	$8,$12
        nop
        or		$8,0x80000000
        mtc0	$8,$12
        nop
        nop
        
        
        # IRAM off
	mtc0	$0, $20 # CCTL
	nop
	nop
	li		$8,0x00000020 
	mtc0	$8, $20
	nop
	nop

	
        # DRAM off
	mtc0	$0, $20 # CCTL
	nop
	nop
	li		$8,0x00000800 
	mtc0	$8, $20
	nop
	nop	
	

	#--- invalidate the icache and dcache with a 0->1 transition
	mtc0	$0, $20 # CCTL
	nop
	nop
	li		$8,0x00000003 # Invalid ICACHE and DCACHE
	mtc0	$8, $20
	nop
	nop

#if 1
	#--- load iram base and top
	la		$8,__iram
	la		$9,0x0ffffc00
	and		$8,$8,$9
	mtc3	$8,$0	# IW bas
	nop
	nop
	addiu	$8,$8,(CONFIG_IMEM_SIZE-1)
	mtc3	$8,$1	# IW top
	nop
	nop

	#--- Refill the IRAM with a 0->1 transition
	mtc0	$0, $20 # CCTL
	nop
	nop
	li		$8,0x00000010 # IRAM Fill
	mtc0	$8, $20
	nop
	nop

	
#ifdef CONFIG_USB_RTL8192SU_SOFTAP	
	#--- load dram base and top
	la		$8,__dram
	la		$9,0x0ffffc00
	and		$8,$8,$9
	mtc3	$8,$4	# DW bas
	nop
	nop
	addiu	$8,$8,0x3fff
	mtc3	$8,$5	# DW top
	nop
	nop
	
	# DRAM on
	mtc0	$0, $20 # CCTL
	nop
	nop
	li		$8,0x00000400
	mtc0	$8, $20
	nop
	nop
#endif		
#endif

	.set	pop
	.endm
        
#endif