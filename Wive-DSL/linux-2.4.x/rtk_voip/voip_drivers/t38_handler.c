#include <linux/config.h>
#include <linux/signal.h>
#include <linux/netdevice.h>
#include <asm/io.h>
#include "rtk_voip.h"
#include "rtl_types.h"
#include "t38_handler.h"
#include "pcm_interface.h"

#ifdef T38_STAND_ALONE_HANDLER

#ifdef REDUCE_PCM_FIFO_MEMCPY
extern uint32* pRxBufTmp; // For reducing memcpy
extern uint32* pTxBufTmp;
#endif

t38_running_state_t t38RunningState[MAX_SLIC_CH_NUM];

int32 PCM_handler_T38( unsigned int chid )
{
	/* 
	 * Make sure ( t38RunningState[ chid ] == T38_START ) 
	 */
	unsigned long flags;

	/************ T.38 encode & ecode **********************/
#ifdef REDUCE_PCM_FIFO_MEMCPY
	T38_API_EncodeDecodeProcessAndDoSend( chid, pRxBufTmp, pTxBufTmp );
#else
	???
#endif

#ifdef SUPPORT_PCM_FIFO
	//printk("%d ", tx_fifo_cnt_w[chid]);
	save_flags(flags); cli();
#ifdef REDUCE_PCM_FIFO_MEMCPY
	if (pcm_write_tx_fifo_done(chid))
#else
	if (pcm_write_tx_fifo(chid, &TxBufTmp[chid][0]))
#endif
	{
		printk("TF(T.38)\n");
	}
	restore_flags(flags);
#endif

	return 0;
}


#endif /* T38_RUNNING_ON_ISR */

