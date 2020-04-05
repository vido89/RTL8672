/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /usr/local/dslrepos/uClinux-dist/linux-2.4.x/drivers/net/re8650/swUtil.c,v 1.1.1.1 2003/08/18 05:43:22 kaohj Exp $
*
* Abstract: Utilities for switch core drivers.
*
* $Author: kaohj $
*
* $Log: swUtil.c,v $
* Revision 1.1.1.1  2003/08/18 05:43:22  kaohj
* initial import into CVS
*
* Revision 1.1  2003/05/12 09:55:54  kckao
* moved from csp
*
* Revision 1.3  2003/05/05 02:09:41  kckao
* Modify for OS independent
*
* Revision 1.2  2003/03/13 10:56:17  cfliu
* Move ASSERT_CSP to core/types.h
*
* Revision 1.1  2002/11/22 10:42:37  danwu
* init.
*
* ---------------------------------------------------------------
*/

#include "rtl_types.h"



int32 s2Mac(macaddr_t *mac_P, int8 *str_P)
{
    uint16  val;
    uint32  count;
    
    ASSERT_CSP( mac_P );
    ASSERT_CSP( str_P );
    
    bzero(mac_P, sizeof(macaddr_t));
    
    if ( str_P[0] == '0' && (str_P[1] == 'x' || str_P[1] == 'X') )
        str_P += 2;
	
    if ( *str_P == '\0' )
        return -1;
	
	count = 1;
    for (val = 0; count <= 12; str_P++)
    {
        if ( '0' <= *str_P && *str_P <= '9' )
        {
            val <<= 4;
            val += *str_P - '0';
        }
        else if ( 'a' <= *str_P && *str_P <= 'f' )
        {
            val <<= 4;
            val +=  *str_P - 'a' + 10;
        }
        else if ( 'A' <= *str_P && *str_P <= 'F' )
        {
            val <<= 4;
            val +=  *str_P - 'A' + 10;
        }
        else
            continue;
        
        if ( count % 4 == 0 )
        {
            if ( count / 4 == 1 )
            {
                mac_P->mac47_32 = val;
                val = 0;
            }
            else if ( count / 4 == 2 )
            {
                mac_P->mac31_16 = val;
                val = 0;
            }
        }
        
        count++;
    }
    
    mac_P->mac15_0 = val;
    
    return 0;
}

int32 s2IP(ipaddr_t *ip_P, int8 * str_P)
{
    uint32  val;
    uint32  count = 1;
    
    ASSERT_CSP( ip_P );
    ASSERT_CSP( str_P );
    
    *ip_P = 0;
    
    for (val = 0; *str_P; str_P++)
    {
        if ( '0' <= *str_P && *str_P <= '9' )
        {
            val *= 10;
            val += *str_P - '0';
        }
        else if ( *str_P == '.' )
        {
            *ip_P |= (val & 0xff) << (32 - 8 * count);
            
            val = 0;
            count++;
        }
        else
            break;
    }
    *ip_P |= (val & 0xff);
    
    return 0;
}

uint32 s2IPMask(int8 * str_P)
{
    uint32  val;
    uint32  ipMask;
    uint32  count = 1;
    
    ASSERT_CSP( str_P );
    
    ipMask = 0;
    
    for (val = 0; ; str_P++)
    {
        if ( '0' <= *str_P && *str_P <= '9' )
        {
            val *= 10;
            val += *str_P - '0';
        }
        else if (*str_P == '.')
        {
            if (val == 255)
                ipMask += 8;
            else
                break;
            
            val = 0;
            count++;
        }
        else
            break;
    }
    if (val == 255)
        ipMask += 8;
    else
    {
        if (val < 128);
        else if ((val >= 128) && (val < 192))
            ipMask += 1;
        else if ((val >= 192) && (val < 224))
            ipMask += 2;
        else if ((val >= 224) && (val < 240))
            ipMask += 3;
        else if ((val >= 240) && (val < 248))
            ipMask += 4;
        else if ((val >= 248) && (val < 252))
            ipMask += 5;
        else if ((val >= 252) && (val < 254))
            ipMask += 6;
        else if (val == 254)
            ipMask += 7;
    }
    
    return ipMask;
}

int32 bs2i(int8 * str_P)
{
    uint32  val;
	
    for (val = 0; *str_P; str_P++)
    {
        val <<= 1;
        if ( '0' <= *str_P && *str_P <= '1' )
            val += *str_P - '0';
        else if ( *str_P == ' ' )
            val >>= 1;
        else
            break;
    }
    
    return val;
}

int32 s2i(int8 * str_P)
{
    uint32  val;
    
    if ( (str_P[0] == '0') && (str_P[1] == 'x') )
    {
        str_P += 2;
        for (val = 0; *str_P; str_P++)
        {
            val *= 16;
            if ( '0' <= *str_P && *str_P <= '9' )
                val += *str_P - '0';
            else if ( 'a' <= *str_P && *str_P <= 'f' )
                val += *str_P - 'a' + 10;
            else if ( 'A' <= *str_P && *str_P <= 'F' )
                val += *str_P - 'A' + 10;
            else
                break;
        }
    }
    else
    {
        for (val = 0; *str_P; str_P++)
        {
            val *= 10;
            if ( '0' <= *str_P && *str_P <= '9' )
                val += *str_P - '0';
            else
                break;
        }
    }
    
    return val;
}

uint32 s2gidx(int8 * str_P)
{
    uint32  val;
	
    for (val = 0; *str_P; str_P++)
    {
        val <<= 3;
        if ( '0' <= *str_P && *str_P <= '7' )
            val += *str_P - '0';
        else if ( *str_P == ' ' )
            val >>= 3;
        else
            break;
    }
    
    return val;
}
