/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 *
 * Tom St Denis, tomstdenis@gmail.com, http://libtomcrypt.org
 */
#include "tomcrypt.h"

/**
   @file burn_stack.c
   Burn stack, Tom St Denis
*/

/**
   Burn some stack memory
   @param len amount of stack to burn in bytes
*/
void burn_stack(unsigned long len)
{
   unsigned char buf[32];
   zeromem(buf, sizeof(buf));
   if (len > (unsigned long)sizeof(buf))
      burn_stack(len - sizeof(buf));
}



/* $Source: /usr/local/dslrepos/uClinux-dist/user/dropbear-0.48.1/libtomcrypt/src/misc/burn_stack.c,v $ */
/* $Revision: 1.1 $ */
/* $Date: 2006/06/08 13:44:05 $ */
