/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /usr/local/dslrepos/uClinux-dist/linux-2.4.x/drivers/net/re8650/phy.h,v 1.1.1.1 2003/08/18 05:43:22 kaohj Exp $
*
* Abstract: PHY access header file.
*
* $Author: kaohj $
*
* $Log: phy.h,v $
* Revision 1.1.1.1  2003/08/18 05:43:22  kaohj
* initial import into CVS
*
* Revision 1.1  2003/05/12 09:56:40  kckao
* moved from csp
*
* Revision 1.1.1.2  2003/03/31 06:29:37  elvis
* no message
*
* Revision 1.1.1.1  2003/03/31 06:09:26  elvis
* no message
*
* Revision 1.1  2003/03/04 13:50:03  danwu
* init
*
* ---------------------------------------------------------------
*/

#ifndef _PHY_H_
#define _PHY_H_



#define PHY_CTRL_REG            0
#define PHY_STS_REG             1
#define PHY_ID1_REG             2
#define PHY_ID2_REG             3
#define PHY_ANADV_REG           4
#define PHY_ANLP_REG            5


uint32 phy_readReg(uint32 port, uint32 regnum);
int32 phy_writeReg(uint32 port, uint32 regnum, uint32 value);



#endif   /* _PHY_H_ */

