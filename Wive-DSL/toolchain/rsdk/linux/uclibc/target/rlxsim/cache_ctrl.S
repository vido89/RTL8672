# =============================================================================
# This file include the functions to control cache behavior.
# =============================================================================

#include "include/regdef.h"
#define CCTL_IInval   0x00000002
#define CCTL_DInval   0x00000001
#define CCTL_DWBInval 0x00000100
    .set  	noat
    .set  	noreorder

    .text

# =============================================================================
# void flush_cache()
# =============================================================================
    .globl	flush_cache
    .ent 	flush_cache
flush_cache:
    mfc0  at, C0_CCTL

    # Invalidate ICache
    andi  at, (~(CCTL_IInval)) & 0xffff
    mtc0  at, C0_CCTL
    or    at, CCTL_IInval
    mtc0  at, C0_CCTL

    # Writeback and Invalidate DCache
    andi  at, (~(CCTL_DWBInval)) & 0xffff
    mtc0  at, C0_CCTL
    or    at, CCTL_DWBInval
    mtc0  at, C0_CCTL

    # Invalidate DCache for not writeback cache cpu
    andi  at, (~(CCTL_DInval)) & 0xffff
    mtc0  at, C0_CCTL
    or    at, CCTL_DInval
    mtc0  at, C0_CCTL

    j     ra
    nop
    .end flush_cache
