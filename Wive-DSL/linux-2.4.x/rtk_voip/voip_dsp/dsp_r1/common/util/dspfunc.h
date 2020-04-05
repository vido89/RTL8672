#ifndef _DSP_FUNC_H_
#define _DSP_FUNC_H_

#include "typedef.h"

extern Word16 tabpow[33];
extern Word16 tablog[33];
extern Word16 tabsqr[49];

/*-------------------------------*
 * Mathematic functions.         *
 *-------------------------------*/

Word32 Inv_sqrt(   /* (o) Q30 : output value   (range: 0<=val<1)           */
  Word32 L_x       /* (i) Q0  : input value    (range: 0<=val<=7fffffff)   */
);

void Log2(
  Word32 L_x,       /* (i) Q0 : input value                                 */
  Word16 *exponent, /* (o) Q0 : Integer part of Log2.   (range: 0<=val<=30) */
  Word16 *fraction  /* (o) Q15: Fractionnal part of Log2. (range: 0<=val<1) */
);

Word32 Pow2(        /* (o) Q0  : result       (range: 0<=val<=0x7fffffff) */
  Word16 exponent,  /* (i) Q0  : Integer part.      (range: 0<=val<=30)   */
  Word16 fraction   /* (i) Q15 : Fractionnal part.  (range: 0.0<=val<1.0) */
);

#endif /* _DSP_FUNC_H_ */

