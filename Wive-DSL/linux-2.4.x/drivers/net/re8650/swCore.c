/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /usr/local/dslrepos/uClinux-dist/linux-2.4.x/drivers/net/re8650/swCore.c,v 1.1.1.1 2003/08/18 05:43:22 kaohj Exp $
*
* Abstract: Switch core driver source code.
*
* $Author: kaohj $
*
* $Log: swCore.c,v $
* Revision 1.1.1.1  2003/08/18 05:43:22  kaohj
* initial import into CVS
*
* Revision 1.2  2003/05/13 04:45:10  orlando
* add code to switch between 10M/100M.
*
* Revision 1.1  2003/05/12 09:56:18  kckao
* moved from csp
*
* Revision 1.5  2003/05/09 12:15:25  kckao
* remove L2/L3 register
*
* Revision 1.4  2003/05/06 08:11:46  elvis
* Disable flow control for port0 ~ 5
*
* Revision 1.3  2003/05/05 02:09:54  kckao
* Modify for OS independent
*
* Revision 1.26  2003/03/13 10:56:18  cfliu
* Move ASSERT_CSP to core/types.h
*
* Revision 1.25  2003/03/04 13:36:16  danwu
* no message
*
* Revision 1.24  2003/02/20 03:07:22  jzchen
* Change counter api to macro
*
* Revision 1.23  2003/01/07 07:55:03  danwu
* While initialization, reset rx/tx and descriptors before full reset.
*
* Revision 1.22  2002/12/30 05:45:22  elvis
* Add timing-probing macros on targets
*
* Revision 1.21  2002/12/19 01:32:56  elvis
* comment out the vlan-creation codes
*
* Revision 1.20  2002/12/11 08:21:14  elvis
* Comment out the initializing sw-reset code, may require
* work-arounds
*
* Revision 1.19  2002/12/11 07:08:08  elvis
* 1. remove L3/L4 init code
* 2. use swCore APIs to setup two vlans
*
* Revision 1.18  2002/11/29 09:57:18  elvis
* CPU can receive all broadcast packets (temporarily),
* adjust later
*
* Revision 1.17  2002/11/22 10:43:17  danwu
* Reconstruct table drivers.
*
* Revision 1.16  2002/11/19 10:17:51  henryCho
* Modify interrupt registration to support daisy-chain interrupt.
*
* Revision 1.15  2002/11/11 02:07:03  danwu
* Move mii_allocateBuf() to boot.c.
*
* Revision 1.14  2002/10/23 03:15:54  danwu
* no message
*
* Revision 1.13  2002/10/05 10:13:23  jzchen
* Re-construct driver style update
*
* Revision 1.12  2002/09/26 05:27:03  henryCho
* no message
*
* Revision 1.11  2002/09/26 01:48:40  danwu
* Allow flexible descriptor number and cluster size.
*
* Revision 1.10  2002/09/17 00:58:10  danwu
* no message
*
* Revision 1.9  2002/09/13 06:15:17  danwu
* Change ip table isN2One to isOne2One.
*
* Revision 1.8  2002/09/13 06:01:29  henryCho
* no message
*
* Revision 1.7  2002/09/10 07:12:15  danwu
* Invoke mii_allocateBuf().
* Debug s2i().
*
* Revision 1.6  2002/09/04 10:05:18  danwu
* Add Henry's init code.
*
* Revision 1.5  2002/09/04 02:30:11  danwu
* no message
*
* Revision 1.4  2002/09/03 05:19:20  danwu
* Add s2i.
*
* Revision 1.3  2002/08/28 11:19:27  danwu
* Add collision control codes. (in progress)
*
* Revision 1.2  2002/08/23 08:59:22  danwu
* no message
*
* Revision 1.1  2002/08/20 05:59:20  danwu
* Create.
*
* ---------------------------------------------------------------
*/

#include "rtl_types.h"
#include "rtl_errno.h"
#include "asicRegs.h"
#include "swCore.h"
#include "phy.h"



/* STATIC VARIABLE DECLARATIONS
 */

typedef uint32 sem_t;
static int32                kernel_inited = 0;
static sem_t                semVlan;


/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
static void     swCore_intHandler(void);



/* MACRO DEFINITIONS 
 */



int32 swCore_init()
{
    /* Perform full-reset for sw-core */ 
    REG32(MACCR) |=  FULL_RST;
    
    /* Disable NIC Tx/Rx and reset all descriptors */
    REG32(CPUICR) &= ~(TXCMD | RXCMD);
    
    /* Check BIST until all packet buffers are reset */
    while (REG32(BISTCR) != BIST_READY_PATTERN)
    	/*tick_Delay10ms(1); */
    
    /* Install interrupt handler */
#if 0	/* moved to NIC driver */
    int_Register(SW_ILEV, SWIE, SWIRS_OFFSET, swCore_intHandler);
#endif
    
    /* Enable back pressure and PHY */
    REG32(MACCR) = EN_BACK_PRESSURE | EN_PHY_P4 | EN_PHY_P3 | EN_PHY_P2 | 
                    EN_PHY_P1 | EN_PHY_P0;
    
    /* Enable VLAN ingress filtering */
    REG32(SWTMCR) = EN_VLAN_INGRESS_FILTER;
    
    /* Setup protocol trapping functionality */
    REG32(PTRAPCR) = EN_ARP_TRAP | EN_RARP_TRAP | EN_PPPOE_TRAP | EN_IGMP_TRAP | 
	                    EN_DHCP_TRAP1 | EN_DHCP_TRAP2 | EN_OSPF_TRAP | EN_RIP_TRAP;
    
    /* Enable L2 lookup engine and spanning tree functionality */
    REG32(MSCR) = EN_L2 | EN_STP;
    
    /* Initialize MIB counters */
    REG32(MIB_CONTROL) = IN_COUNTER_RESTART | OUT_COUNTER_RESTART;
    
    /* Start normal TX and RX */
    REG32(BISTCR) |= TRXRDY;
    
    /* Setup flow control functionality */
    REG32(FCRTH) = (0x18 << IN_Q_PER_PORT_BUF_FC_THH_OFFSET) | 
                    (0x0c << IN_Q_PER_PORT_BUF_FC_THL_OFFSET) | 
                    (0x18 << OUT_Q_PER_PORT_BUF_FC_THH_OFFSET) | 
                    (0x0c << OUT_Q_PER_PORT_BUF_FC_THL_OFFSET);
    REG32(FCPTR) = (0x006c << IN_Q_PTH_OFFSET) | 
                    (0x006c << OUT_Q_PTH_OFFSET);
/*    
    REG32(FCREN) = EN_INQ_FC_CPU | EN_INQ_FC_5 | EN_INQ_FC_4 | EN_INQ_FC_3 | EN_INQ_FC_2 | 
                    EN_INQ_FC_1 | EN_INQ_FC_0 | EN_OUTQ_FC_CPU | EN_OUTQ_FC_5 | EN_OUTQ_FC_4 | 
                    EN_OUTQ_FC_3 | EN_OUTQ_FC_2 | EN_OUTQ_FC_1 | EN_OUTQ_FC_0 | EN_MDC_MDIO_FC | 
                    (0x1f0 << DSC_TH_OFFSET);
*/ 
    REG32(FCREN) = EN_INQ_FC_CPU  | EN_OUTQ_FC_CPU  | EN_MDC_MDIO_FC | 
                    (0x1f0 << DSC_TH_OFFSET);
    /* Initialize PHY */
    REG32(MACCR) |= (EN_PHY_P4 | EN_PHY_P3 | EN_PHY_P2 | EN_PHY_P1 | EN_PHY_P0);
    
    /* Workaround: PHY registers must be read out once to ensure behavior correctness */
    {
    uint32  val = 
                REG32(PORT0_PHY_CONTROL) | REG32(PORT0_PHY_STATUS) | 
                REG32(PORT0_PHY_IDENTIFIER_1) | REG32(PORT0_PHY_IDENTIFIER_2) | 
                REG32(PORT0_PHY_AUTONEGO_ADVERTISEMENT) | 
                REG32(PORT0_PHY_AUTONEGO_LINK_PARTNER_ABILITY) | 
                REG32(PORT1_PHY_CONTROL) | REG32(PORT1_PHY_STATUS) | 
                REG32(PORT1_PHY_IDENTIFIER_1) | REG32(PORT1_PHY_IDENTIFIER_2) | 
                REG32(PORT1_PHY_AUTONEGO_ADVERTISEMENT) | 
                REG32(PORT1_PHY_AUTONEGO_LINK_PARTNER_ABILITY) | 
                REG32(PORT2_PHY_CONTROL) | REG32(PORT2_PHY_STATUS) | 
                REG32(PORT2_PHY_IDENTIFIER_1) | REG32(PORT2_PHY_IDENTIFIER_2) | 
                REG32(PORT2_PHY_AUTONEGO_ADVERTISEMENT) | 
                REG32(PORT2_PHY_AUTONEGO_LINK_PARTNER_ABILITY) | 
                REG32(PORT3_PHY_CONTROL) | REG32(PORT3_PHY_STATUS) | 
                REG32(PORT3_PHY_IDENTIFIER_1) | REG32(PORT3_PHY_IDENTIFIER_2) | 
                REG32(PORT3_PHY_AUTONEGO_ADVERTISEMENT) | 
                REG32(PORT3_PHY_AUTONEGO_LINK_PARTNER_ABILITY) | 
                REG32(PORT4_PHY_CONTROL) | REG32(PORT4_PHY_STATUS) | 
                REG32(PORT4_PHY_IDENTIFIER_1) | REG32(PORT4_PHY_IDENTIFIER_2) | 
                REG32(PORT4_PHY_AUTONEGO_ADVERTISEMENT) | 
                REG32(PORT4_PHY_AUTONEGO_LINK_PARTNER_ABILITY);
                
                for (val=0; val<5; val++)
    			{
        			phy_writeReg(val, PHY_ANADV_REG, 
#if 1
/*  Use 100 Mbps */	
                    phy_readReg(val, PHY_ANADV_REG) | CAPABLE_100BASE_TX_FD 
                                                    | CAPABLE_100BASE_TX_HD 
                                                    | CAPABLE_10BASE_TX_FD 
                                                    | CAPABLE_10BASE_TX_HD);
#else                                                    
/*  Use 10 Mbps */

                    phy_readReg(val, PHY_ANADV_REG) & ~CAPABLE_100BASE_TX_FD 
                                                    & ~CAPABLE_100BASE_TX_HD 
                                                    | CAPABLE_10BASE_TX_FD 
                                                    | CAPABLE_10BASE_TX_HD);
#endif                                                    
        			phy_writeReg(val, PHY_CTRL_REG, phy_readReg(val, PHY_CTRL_REG) | RESTART_AUTONEGO);
        			
    			}
    }

    
    /* Enable interrupt */
#if 0	/* move to NIC driver */
    if ( SW_ILEV < getIlev() )
  	    setIlev(SW_ILEV);
#endif
    
    return 0;
}

#if 0	/* move to NIC driver */
static void swCore_intHandler(void)
{
    int32  intPending;
    
    /* Read interrupt control register */
    intPending = REG32(CPUIISR);
    
    /* Filter masked interrupt */
    intPending &= REG32(CPUIIMR);
    
    /* Check and handle NIC interrupts */
    if (intPending & INTPENDING_NIC_MASK)
        swNic_intHandler(intPending);
        
    /* Check and handle link change interrupt */
    if (intPending & LINK_CHANG_IP)
        /* Handle link change here */;
    
    /* Check and handle software interrupt */
    if (intPending & SW_INT_IP)
        /* Handle software interrupt here */;
}
#endif


int32 swCore_vlanSetPVid(uint32 portNum, uint32 pvid)
{
    if ( portNum >= MAX_PORT_NUMBER )
        return EINVAL;
    
    REG32(PVCR) = (REG32(PVCR) & ~(PVID_MASK << (3 * portNum))) | ((pvid & 7) << (3 * portNum));
    
    return 0;
}

int32 swCore_vlanGetPVid(uint32 portNum, uint32 *pvid_P)
{
    if ( portNum >= MAX_PORT_NUMBER )
        return EINVAL;
    
    return vlanTable_getVidByIndex((REG32(PVCR) >> (3 * portNum)) & PVID_MASK, pvid_P);
}

int32 swCore_staticMacAddrAdd(rtl_l2_param_t * param_P)
{
    uint32          eidx;
    
    ASSERT_CSP( param_P );
    
    param_P->isStatic = 1;
    param_P->nxtHostFlag = 0;
    
    return l2SwitchTable_add(&eidx, param_P);
}

int32 swCore_staticMacAddrRemove(rtl_l2_param_t * param_P)
{
    uint32          eidx;
    
    ASSERT_CSP( param_P );
    
    if ( (l2SwitchTable_getInformationByMac(&eidx, param_P) == ENOENT) || 
            !param_P->isStatic )
        return ENOENT;
    else
        l2SwitchTable_remove(&eidx, &param_P->mac);
    return 0;
}

int32 swCore_layer2TableGetInformation(uint32 eidx, rtl_l2_param_t * param_P)
{
    ASSERT_CSP( param_P );
    
    return l2SwitchTable_getInformation(eidx, param_P);
}

int32 swCore_layer2TableGetInformationByMac(rtl_l2_param_t * param_P)
{
    uint32          eidx;
    
    ASSERT_CSP( param_P );
    
    return l2SwitchTable_getInformationByMac(&eidx, param_P);
}

int32 swCore_counterGetMemberPort(uint32 *portList_P)
{
    if (portList_P == NULL)
        return EINVAL;
    
    *portList_P = (REG32(MIB_CONTROL) >> PORT_FOR_COUNTING_OFFSET) & ALL_PORT_MASK;
    
    return 0;
}

int32 swCore_counterSetMemberPort(uint32 portList)
{
    uint32      regValue;
    
    if (portList > ALL_PORT_MASK)
        return EINVAL;
    
    regValue = REG32(MIB_CONTROL);
    regValue &= ~(ALL_PORT_MASK << PORT_FOR_COUNTING_OFFSET);
    regValue |= (portList << PORT_FOR_COUNTING_OFFSET);
    REG32(MIB_CONTROL) = regValue;
    
    REG32(MIB_CONTROL) = IN_COUNTER_RESTART | OUT_COUNTER_RESTART;
    
    return 0;
}

int32 swCore_counterGetIngress(rtl_ingress_counter_t *counters_P)
{
    uint32      *counter32p;
    uint32      counterReg;
    
    if (counters_P == NULL)
        return EINVAL;
    
    counter32p = (uint32 *) counters_P;
    counterReg = ETHER_STATS_OCTETS;
    
    while (counterReg <= DOT3_IN_PAUSE_FRAMES)
    {
        *counter32p = REG32(counterReg);
        
        counter32p++;
        counterReg += 4;
    }
    
    return 0;
}

int32 swCore_counterGetEgress(rtl_egress_counter_t *counters_P)
{
    uint32      *counter32p;
    uint32      counterReg;
    
    if (counters_P == NULL)
        return EINVAL;
    
    counter32p = (uint32 *) counters_P;
    counterReg = IF_OUT_OCTETS;
    
    while (counterReg <= DOT3_OUT_PAUSE_FRAMES)
    {
        *counter32p = REG32(counterReg);
        
        counter32p++;
        counterReg += 4;
    }
    
    return 0;
}

int32 swCore_portSetSpeedDuplex(uint32 portNum, uint32 speedDuplex)
{
    if ( portNum >= MAX_PORT_NUMBER )
        return EINVAL;
        
    switch (speedDuplex)
    {
        case RTL_PORT_100M_FD:
            return phy_writeReg(portNum, PHY_CTRL_REG, phy_readReg(portNum, PHY_CTRL_REG)
                            | SPEED_SELECT_100M | SELECT_FULL_DUPLEX & ~ENABLE_AUTONEGO);
        case RTL_PORT_100M_HD:
            return phy_writeReg(portNum, PHY_CTRL_REG, phy_readReg(portNum, PHY_CTRL_REG)
                            | SPEED_SELECT_100M & ~SELECT_FULL_DUPLEX & ~ENABLE_AUTONEGO);
        case RTL_PORT_10M_FD:
            return phy_writeReg(portNum, PHY_CTRL_REG, phy_readReg(portNum, PHY_CTRL_REG)
                            & ~SPEED_SELECT_100M | SELECT_FULL_DUPLEX & ~ENABLE_AUTONEGO);
        case RTL_PORT_10M_HD:
            return phy_writeReg(portNum, PHY_CTRL_REG, phy_readReg(portNum, PHY_CTRL_REG)
                            & ~SPEED_SELECT_100M & ~SELECT_FULL_DUPLEX & ~ENABLE_AUTONEGO);
        default:
            return EINVAL;
    }
}

int32 swCore_portSetAutoNegociationAbility(uint32 portNum, rtl_auto_nego_ability_t *anAbility_P)
{
    uint32 anadv_reg;
    
    ASSERT_CSP( anAbility_P );
    
    if ( portNum >= MAX_PORT_NUMBER )
        return EINVAL;
    
    anadv_reg = phy_readReg(portNum, PHY_ANADV_REG);
    
    if ( anAbility_P->capableFlowCtrl )
        anadv_reg |= CAPABLE_PAUSE;
    else
        anadv_reg &= ~CAPABLE_PAUSE;
    if ( anAbility_P->capable100MFull )
        anadv_reg |= CAPABLE_100BASE_TX_FD;
    else
        anadv_reg &= ~CAPABLE_100BASE_TX_FD;
    if ( anAbility_P->capable100MHalf )
        anadv_reg |= CAPABLE_100BASE_TX_HD;
    else
        anadv_reg &= ~CAPABLE_100BASE_TX_HD;
    if ( anAbility_P->capable10MFull )
        anadv_reg |= CAPABLE_10BASE_TX_FD;
    else
        anadv_reg &= ~CAPABLE_10BASE_TX_FD;
    if ( anAbility_P->capable10MHalf )
        anadv_reg |= CAPABLE_10BASE_TX_HD;
    else
        anadv_reg &= ~CAPABLE_10BASE_TX_HD;
        
    return phy_writeReg(portNum, PHY_ANADV_REG, anadv_reg);
}

int32 swCore_portEnableAutoNegociation(uint32 portNum)
{
    if ( portNum >= MAX_PORT_NUMBER )
        return EINVAL;
        
    return phy_writeReg(portNum, PHY_CTRL_REG, 
                            phy_readReg(portNum, PHY_CTRL_REG) | ENABLE_AUTONEGO);
}

int32 swCore_portDisableAutoNegociation(uint32 portNum)
{
    if ( portNum >= MAX_PORT_NUMBER )
        return EINVAL;
        
    return phy_writeReg(portNum, PHY_CTRL_REG, 
                            phy_readReg(portNum, PHY_CTRL_REG) & ~ENABLE_AUTONEGO);
}

int32 swCore_portRestartAutoNegociation(uint32 portNum)
{
    if ( portNum >= MAX_PORT_NUMBER )
        return EINVAL;
        
    return phy_writeReg(portNum, PHY_CTRL_REG, 
                            phy_readReg(portNum, PHY_CTRL_REG) | RESTART_AUTONEGO);
}

int32 swCore_portSetLoopback(uint32 portNum)
{
    if ( portNum >= MAX_PORT_NUMBER )
        return EINVAL;
        
    return phy_writeReg(portNum, PHY_CTRL_REG, 
                            phy_readReg(portNum, PHY_CTRL_REG) | ENABLE_LOOPBACK);
}

int32 swCore_portResetLoopback(uint32 portNum)
{
    if ( portNum >= MAX_PORT_NUMBER )
        return EINVAL;
        
    return phy_writeReg(portNum, PHY_CTRL_REG, 
                            phy_readReg(portNum, PHY_CTRL_REG) & ~ENABLE_LOOPBACK);
}

int32 swCore_portGetStatus(uint32 portNum, rtl_port_status_t *portStatus_P)
{
    uint32 regval;
    
    ASSERT_CSP( portStatus_P );
    
    if ( portNum >= MAX_PORT_NUMBER )
        return EINVAL;
        
    regval = phy_readReg(portNum, PHY_CTRL_REG);
    portStatus_P->enAutoNego = regval & ENABLE_AUTONEGO;
    portStatus_P->enSpeed100M = regval & SPEED_SELECT_100M;
    portStatus_P->enFullDuplex = regval & SELECT_FULL_DUPLEX;
    portStatus_P->enLoopback = regval & ENABLE_LOOPBACK;
    
    regval = phy_readReg(portNum, PHY_STS_REG);
    portStatus_P->linkEstablished = regval & STS_LINK_ESTABLISHED;
    portStatus_P->autoNegoCompleted = regval & STS_AUTONEGO_COMPLETE;
    portStatus_P->remoteFault = regval & STS_REMOTE_FAULT;
    if ( regval & STS_CAPABLE_100BASE_TX_FD )
        portStatus_P->speedDuplex = RTL_PORT_100M_FD;
    else if ( regval & STS_CAPABLE_100BASE_TX_HD )
        portStatus_P->speedDuplex = RTL_PORT_100M_HD;
    else if ( regval & STS_CAPABLE_10BASE_TX_FD )
        portStatus_P->speedDuplex = RTL_PORT_10M_FD;
    else if ( regval & STS_CAPABLE_10BASE_TX_HD )
        portStatus_P->speedDuplex = RTL_PORT_10M_HD;
        
    regval = phy_readReg(portNum, PHY_ANADV_REG);
    portStatus_P->autoNegoAbility.capableFlowCtrl = regval & CAPABLE_PAUSE;
    portStatus_P->autoNegoAbility.capable100MFull = regval & CAPABLE_100BASE_TX_FD;
    portStatus_P->autoNegoAbility.capable100MHalf = regval & CAPABLE_100BASE_TX_HD;
    portStatus_P->autoNegoAbility.capable10MFull = regval & CAPABLE_10BASE_TX_FD;
    portStatus_P->autoNegoAbility.capable10MHalf = regval & CAPABLE_10BASE_TX_HD;
                    
    regval = phy_readReg(portNum, PHY_ANLP_REG);
    portStatus_P->linkPartnerAutoNegoAbility.capableFlowCtrl = regval & CAPABLE_PAUSE;
    portStatus_P->linkPartnerAutoNegoAbility.capable100MFull = regval & CAPABLE_100BASE_TX_FD;
    portStatus_P->linkPartnerAutoNegoAbility.capable100MHalf = regval & CAPABLE_100BASE_TX_HD;
    portStatus_P->linkPartnerAutoNegoAbility.capable10MFull = regval & CAPABLE_10BASE_TX_FD;
    portStatus_P->linkPartnerAutoNegoAbility.capable10MHalf = regval & CAPABLE_10BASE_TX_HD;
    
    return 0;
}
