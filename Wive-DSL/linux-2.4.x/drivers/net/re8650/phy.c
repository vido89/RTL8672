/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /usr/local/dslrepos/uClinux-dist/linux-2.4.x/drivers/net/re8650/phy.c,v 1.1.1.1 2003/08/18 05:43:22 kaohj Exp $
*
* Abstract: Phy access driver source code.
*
* $Author: kaohj $
*
* $Log: phy.c,v $
* Revision 1.1.1.1  2003/08/18 05:43:22  kaohj
* initial import into CVS
*
* Revision 1.1  2003/05/12 09:57:38  kckao
* moved from csp
*
* Revision 1.1.1.2  2003/03/31 06:29:37  elvis
* no message
*
* Revision 1.1.1.1  2003/03/31 06:09:26  elvis
* no message
*
* Revision 1.1  2003/03/04 13:36:49  danwu
* init
*
* ---------------------------------------------------------------
*/

#if 0
#include <sys/rtl_types.h>
#include "board.h"
#include <csp/rtl8650/ethernet/phy.h>
#endif

#include "rtl_types.h"
#include "asicRegs.h"
#include "phy.h"



uint32 phy_readReg(uint32 port, uint32 regnum)
{
    ASSERT_CSP( port < MAX_PORT_NUMBER );
    ASSERT_CSP( regnum <= PHY_ANLP_REG );
    
    return REG32(PHY_BASE + (port << 5) + (regnum << 2));
}

int32 phy_writeReg(uint32 port, uint32 regnum, uint32 value)
{
    ASSERT_CSP( port < MAX_PORT_NUMBER );
    ASSERT_CSP( regnum <= PHY_ANLP_REG );
    
    /* Wait for command ready */
    while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE );
    
    REG32(TCR0) = value;
    
    /* Fill address */
    REG32(SWTAA) = PHY_BASE + (port << 5) + (regnum << 2);
        
    /* Activate add command */
    REG32(SWTACR) = ACTION_START | CMD_FORCE;
    
    /* Wait for command done */
    while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE );
    
    /* Check status */
    if ( (REG32(SWTASR) & TABSTS_MASK) == TABSTS_SUCCESS )
        return 0;
        
    /* There might be something wrong */
    ASSERT_CSP( 0 );
}
