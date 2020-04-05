/*
 *      Adsl Driver Interface
 *      Authors: Dick Tam	<dicktam@realtek.com.tw>
 *
 */

#ifndef _INCLUDE_ADSL_DRV_H
#define _INCLUDE_ADSL_DRV_H

#define ADSL_DRV_RETURN_LEN	512

typedef enum {
	ADSL_GET_SNR_DS,
	ADSL_GET_SNR_US,
	ADSL_GET_VERSION,
	ADSL_GET_MODE,
	ADSL_GET_STATE,
	ADSL_GET_POWER_DS,
	ADSL_GET_POWER_US,
	ADSL_GET_ATTRATE_DS,
	ADSL_GET_ATTRATE_US,
	ADSL_GET_RATE_DS,
	ADSL_GET_RATE_US,
	ADSL_GET_LATENCY,
	//ADSL_GET_LATENCY_DS,
	//ADSL_GET_LATENCY_US,
	ADSL_GET_LPATT_DS,
	ADSL_GET_LPATT_US,
	ADSL_GET_TRELLIS,
	ADSL_GET_POWER_LEVEL,
	ADSL_GET_K_DS,
	ADSL_GET_K_US,
	ADSL_GET_R_DS,
	ADSL_GET_R_US,
	ADSL_GET_S_DS,
	ADSL_GET_S_US,
	ADSL_GET_D_DS,
	ADSL_GET_D_US,
	ADSL_GET_DELAY_DS,
	ADSL_GET_DELAY_US,
	ADSL_GET_CRC_DS,
	ADSL_GET_FEC_DS,
	ADSL_GET_ES_DS,
	ADSL_GET_SES_DS,
	ADSL_GET_UAS_DS,
	ADSL_GET_CRC_US,
	ADSL_GET_FEC_US,
	ADSL_GET_ES_US,
	ADSL_GET_SES_US,
	ADSL_GET_UAS_US
} ADSL_GET_ID;
        
typedef struct {
	char *cmd;
	ADSL_GET_ID id;
} adsl_drv_get_cmd;

typedef struct {
	char *cmd;
	char *value;
} adsl_drv_set_cmd;

extern int getAdslDrvInfo(char *name, char *buff, int len);
extern int getAdslInfo(ADSL_GET_ID id, char *buf, int len);


#endif
