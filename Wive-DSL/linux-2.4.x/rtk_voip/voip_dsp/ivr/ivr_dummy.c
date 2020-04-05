#include "rtk_voip.h"
#include "rtl_types.h"

#if ! defined (CONFIG_RTK_VOIP_G7231) || defined (AUDIOCODES_VOIP)
void InitializeIvr723Player( unsigned int chid, void *pIvrPlay )
{
}

unsigned int PutDataIntoIvr723Player( void *pIvrPlay, 
								   const unsigned char *pData, 
								   unsigned int nCount )
{
	return 0;
}

int RunIvr723Player( unsigned int chid, void *pIvrPlay, const Word16 **ppOut )
{
	return 0;
}

unsigned int GetPredictionPeriodOfG72363( const void *pIvrPlay )
{
	return 0;
}
#endif /* !CONFIG_RTK_VOIP_G7231 */

