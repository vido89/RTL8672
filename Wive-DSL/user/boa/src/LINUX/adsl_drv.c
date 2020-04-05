/*
 *      Adsl Driver Interface
 *      Authors: Dick Tam	<dicktam@realtek.com.tw>
 *
 */

#include <string.h>
#include <stdlib.h>
#include <time.h>

// from hrchen adslctrl.c
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
// from hrchen adslctrl.c end

//for RLCM_GET_DS_PMS_PARAM1 & RLCM_GET_US_PMS_PARAM1
typedef struct {
	unsigned short	K;
	unsigned short	R;
	unsigned short	D;
	unsigned short	N_fec;
	unsigned short	Lp;
	unsigned short	S;
	unsigned short	Delay;
} Modem_PMSParm;

//for RLCM_GET_DS_ERROR_COUNT & RLCM_GET_US_ERROR_COUNT
typedef struct {
	unsigned long	crc;
	unsigned long	fec;
	unsigned long	es;
	unsigned long	ses;
	unsigned long	uas;
} Modem_MgmtCounter;

//for RLCM_GET_LINE_RATE
typedef struct {
	unsigned short	upstreamRate;
	unsigned short	downstreamRate;
} Modem_LineRate;

//for RLCM_GET_ATT_RATE
typedef struct {
	unsigned short	upstreamRate;
	unsigned short	downstreamRate;
} Modem_AttRate;

#include "adsl_drv.h"
#include "utility.h"
#include "mib.h"

static char rValueAdslDrv[ADSL_DRV_RETURN_LEN];
static char rGetFail[] = "n/a";

adsl_drv_get_cmd get_cmd_list[] = {
	{"snr-ds", ADSL_GET_SNR_DS},
	{"snr-us", ADSL_GET_SNR_US},
	{"version", ADSL_GET_VERSION},
	{"mode", ADSL_GET_MODE},
	{"state", ADSL_GET_STATE},
	{"power-ds", ADSL_GET_POWER_DS},
	{"power-us", ADSL_GET_POWER_US},
	{"attrate-ds", ADSL_GET_ATTRATE_DS},
	{"attrate-us", ADSL_GET_ATTRATE_US},
	{"rate-ds", ADSL_GET_RATE_DS},
	{"rate-us", ADSL_GET_RATE_US},
	{"latency", ADSL_GET_LATENCY},
	{"lpatt-ds", ADSL_GET_LPATT_DS},
	{"lpatt-us", ADSL_GET_LPATT_US},
	{"trellis", ADSL_GET_TRELLIS},
	{"pwlevel", ADSL_GET_POWER_LEVEL},
	{"pms-k-ds", ADSL_GET_K_DS},
	{"pms-k-us", ADSL_GET_K_US},
	{"pms-r-ds", ADSL_GET_R_DS},
	{"pms-r-us", ADSL_GET_R_US},
	{"pms-s-ds", ADSL_GET_S_DS},
	{"pms-s-us", ADSL_GET_S_US},
	{"pms-d-ds", ADSL_GET_D_DS},
	{"pms-d-us", ADSL_GET_D_US},
	{"pms-delay-ds", ADSL_GET_DELAY_DS},
	{"pms-delay-us", ADSL_GET_DELAY_US},
	{"fec-ds", ADSL_GET_FEC_DS},
	{"fec-us", ADSL_GET_FEC_US},
	{"crc-ds", ADSL_GET_CRC_DS},
	{"crc-us", ADSL_GET_CRC_US},
	{"es-ds", ADSL_GET_ES_DS},
	{"es-us", ADSL_GET_ES_US},
	{"ses-ds", ADSL_GET_SES_DS},
	{"ses-us", ADSL_GET_SES_US},
	{"uas-ds", ADSL_GET_UAS_DS},
	{"uas-us", ADSL_GET_UAS_US},
	{NULL, 0}
};

/*
 *	Get adsl string according to the id.
 *	The string is put into buf, and this function return the number of
 *	characters in the string (not including the trailing '\0')
 */
int getAdslInfo(ADSL_GET_ID id, char *buf, int len)
{
	int ret;
	int intVal[2];
	char chVal[2];
	Modem_NearEndLineOperData vNeld;
	Modem_FarEndLineOperData vFeld;
	Modem_AttRate vAr;
	Modem_LinkSpeed vLs;
	Modem_PMSParm pms;
	Modem_AvgLoopAttenuation vAla;
	Modem_Config vMc;
	Modem_MgmtCounter counter;
	
	ret = 0;
	
	memset(buf, 0x00, len);
	switch (id) {
		case ADSL_GET_SNR_DS:
			if(adsl_drv_get(RLCM_GET_SNR_MARGIN, intVal, RLCM_GET_SNR_MARGIN_SIZE)) {
				ret = snprintf(buf, len, "%d.%d", intVal[0]/10, intVal[0]%10);
			}
			break;
		case ADSL_GET_SNR_US:
			if(adsl_drv_get(RLCM_GET_SNR_MARGIN, intVal, RLCM_GET_SNR_MARGIN_SIZE)) {
				ret = snprintf(buf, len, "%d.%d", intVal[1]/10, intVal[1]%10);
			}
			break;
		case ADSL_GET_VERSION:
			if(adsl_drv_get(RLCM_GET_DRIVER_VERSION, (void *)rValueAdslDrv, RLCM_DRIVER_VERSION_SIZE)) {
				ret = snprintf(buf, len, "%s", rValueAdslDrv);
			}
			break;
		case ADSL_GET_MODE:
			if(adsl_drv_get(RLCM_GET_SHOWTIME_XDSL_MODE, (void *)&chVal[0], 1)) {
				//jim suit for annex mode append to showtime mode...
				unsigned int annexMode;
				char *ExpMode[]=
				{
					"",
					" AnnexB",
					" AnnexM",
					"",
					" AnnexL",
					0
				};
				annexMode=((unsigned char)chVal[0])>>5;
				if(annexMode >4)
					annexMode=0;
				chVal[0]&=0x1F;
				if (chVal[0] == 1) { // ADSL1
					adsl_drv_get(RLCM_GET_ADSL_MODE, (void *)&intVal[0], 4);
					if (intVal[0]==1)
						ret = snprintf(buf, len, "%s", "T1.413");
					if (intVal[0]==2)
						ret = snprintf(buf, len, "%s", "G.dmt");
				}
				else if (chVal[0] == 2)  // G.992.2 -- G.Lite
						ret = snprintf(buf, len, "%s", "G.Lite");
				else if (chVal[0] == 3)  // ADSL2
						ret = snprintf(buf, len, "%s", "ADSL2");
				else if (chVal[0] == 5)  // ADSL2+
						ret = snprintf(buf, len, "%s", "ADSL2+");
				//printf("annex mode =%s\n", ExpMode[annexMode]);
				strcat(buf, ExpMode[annexMode]);
			}
			break;
		// get string
		case ADSL_GET_STATE:
			if(adsl_drv_get(RLCM_REPORT_MODEM_STATE, (void *)buf, len)) {
				ret = strlen(buf);
			}
			else
				ret = snprintf(buf, len, "%s", "IDLE");
			break;
		case ADSL_GET_POWER_DS:
			if(adsl_drv_get(RLCM_MODEM_FAR_END_LINE_DATA_REQ, (void *)&vFeld, RLCM_MODEM_FAR_END_LINE_DATA_REQ_SIZE)) {
				ret = snprintf(buf, len, "%d.%d" ,vFeld.outputPowerDnstr/2, ((vFeld.outputPowerDnstr&1)*5)%10);
			}
			break;
		case ADSL_GET_POWER_US:
			if(adsl_drv_get(RLCM_MODEM_NEAR_END_LINE_DATA_REQ, (void *)&vNeld, RLCM_MODEM_NEAR_END_LINE_DATA_REQ_SIZE)) {
				ret = snprintf(buf, len, "%d.%d" ,vNeld.outputPowerUpstr/2, ((vNeld.outputPowerUpstr&1)*5)%10);
			}
			break;
		case ADSL_GET_ATTRATE_DS:
			if(adsl_drv_get(RLCM_GET_ATT_RATE, (void *)&vAr, sizeof(Modem_AttRate))) {
				ret = snprintf(buf, len, "%d"
					,vAr.downstreamRate);
			}
			break;
		case ADSL_GET_ATTRATE_US:
			if(adsl_drv_get(RLCM_GET_ATT_RATE, (void *)&vAr, sizeof(Modem_AttRate))) {
				ret = snprintf(buf, len, "%d"
					,vAr.upstreamRate);
			}
			break;
		case ADSL_GET_RATE_DS:
			if(adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE)) {
				ret = snprintf(buf, len, "%d"
					,vLs.downstreamRate);
			}
			break;
		case ADSL_GET_RATE_US:
			if(adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE)) {
				ret = snprintf(buf, len, "%d"
					,vLs.upstreamRate);
			}
			break;
		case ADSL_GET_LATENCY:
			if(adsl_drv_get(RLCM_GET_CHANNEL_MODE, (void *)&intVal[0], sizeof(int))) {
				if (intVal[0]==1)
					ret = snprintf(buf, len, "%s", "Fast");
				else if(intVal[0]==2)
					ret = snprintf(buf, len, "%s", "Interleave");
			}
			break;		
		case ADSL_GET_LPATT_DS:
			if(adsl_drv_get(RLCM_GET_LOOP_ATT, (void *)&vAla, RLCM_GET_LOOP_ATT_SIZE)) {
				ret = snprintf(buf, len, "%d.%d", vAla.downstreamAtt/2, (vAla.downstreamAtt*5)%10);
			}
			break;
		case ADSL_GET_LPATT_US:
			if(adsl_drv_get(RLCM_GET_LOOP_ATT, (void *)&vAla, RLCM_GET_LOOP_ATT_SIZE)) {
				ret = snprintf(buf, len, "%d.%d", vAla.upstreamAtt/2, (vAla.upstreamAtt*5)%10);
			}
			break;
		case ADSL_GET_TRELLIS:
			if(adsl_drv_get(RLCM_MODEM_READ_CONFIG, (void *)&vMc, RLCM_MODEM_CONFIG_SIZE)) {
				ret = snprintf(buf, len, "%s", (vMc.TrellisEnable==1)?"Enable":"Disable");
			}
			break;
		case ADSL_GET_POWER_LEVEL:
			if(adsl_drv_get(RLCM_GET_LINK_POWER_STATE, (void *)&chVal[0], 1)) {
				if (chVal[0] == 0)
					ret = snprintf(buf, len, "%s", "L0");
				else if (chVal[0] == 2)
					ret = snprintf(buf, len, "%s", "L2");
				else if (chVal[0] == 3)
					ret = snprintf(buf, len, "%s", "L3");
			}
			break;
		case ADSL_GET_K_DS:
		case ADSL_GET_R_DS:
		case ADSL_GET_S_DS:
		case ADSL_GET_D_DS:
		case ADSL_GET_DELAY_DS:
			memset((void *)&pms, 0, sizeof(pms));
			if(adsl_drv_get(RLCM_GET_DS_PMS_PARAM1, (void *)&pms, sizeof(pms))) {
				switch (id) {
					case ADSL_GET_K_DS:
						ret = snprintf(buf, len, "%d", pms.K);
						break;
					case ADSL_GET_R_DS:
						ret = snprintf(buf, len, "%d", pms.R);
						break;
					case ADSL_GET_S_DS:
						ret = snprintf(buf, len, "%d.%02d", pms.S/100, pms.S%100);
						break;
					case ADSL_GET_D_DS:
						ret = snprintf(buf, len, "%d", pms.D);
						break;
					case ADSL_GET_DELAY_DS:
						ret = snprintf(buf, len, "%d.%02d", pms.Delay/100, pms.Delay%100);
						break;
					default:break;
				}
			}
			break;
		case ADSL_GET_K_US:
		case ADSL_GET_R_US:
		case ADSL_GET_S_US:
		case ADSL_GET_D_US:
		case ADSL_GET_DELAY_US:
			memset((void *)&pms, 0, sizeof(pms));
			if(adsl_drv_get(RLCM_GET_US_PMS_PARAM1, (void *)&pms, sizeof(pms))) {
				switch (id) {
					case ADSL_GET_K_US:
						ret = snprintf(buf, len, "%d", pms.K);
						break;
					case ADSL_GET_R_US:
						ret = snprintf(buf, len, "%d", pms.R);
						break;
					case ADSL_GET_S_US:
						ret = snprintf(buf, len, "%d.%02d", pms.S/100, pms.S%100);
						break;
					case ADSL_GET_D_US:
						ret = snprintf(buf, len, "%d", pms.D);
						break;
					case ADSL_GET_DELAY_US:
						ret = snprintf(buf, len, "%d.%02d", pms.Delay/100, pms.Delay%100);
						break;
					default:break;
				}
			}
			break;
		case ADSL_GET_CRC_DS:
		case ADSL_GET_FEC_DS:
		case ADSL_GET_ES_DS:
		case ADSL_GET_SES_DS:
		case ADSL_GET_UAS_DS:
			memset((void *)&counter, 0, sizeof(counter));
			if(adsl_drv_get(RLCM_GET_DS_ERROR_COUNT, (void *)&counter, sizeof(counter))) {
				unsigned long *cnt;
				cnt = (unsigned long *)&counter;
				ret = snprintf(buf, len, "%d", cnt[id-ADSL_GET_CRC_DS]);
			}
			break;
		case ADSL_GET_CRC_US:
		case ADSL_GET_FEC_US:
		case ADSL_GET_ES_US:
		case ADSL_GET_SES_US:
		case ADSL_GET_UAS_US:
			memset((void *)&counter, 0, sizeof(counter));
			if(adsl_drv_get(RLCM_GET_US_ERROR_COUNT, (void *)&counter, sizeof(counter))) {
				//unsigned long *cnt;
				unsigned short *cnt;	// temporarily, should use long
				cnt = (unsigned short *)&counter;
				ret = snprintf(buf, len, "%d", cnt[id-ADSL_GET_CRC_US]);
			}
			break;
		default:
			break;
	}
	
	return ret;
}
		
		
int getAdslDrvInfo(char *name, char *buff, int len)
{
	int idx, length;
	
	length = 0;
	
	for (idx=0; get_cmd_list[idx].cmd != NULL; idx++) {
		if (!strcmp(name, get_cmd_list[idx].cmd)) {
			length=getAdslInfo(get_cmd_list[idx].id, buff, len);
			break;
		}
	}
	
	if (length == 0)
		length = snprintf(buff, len, "%s", BLANK);
	
	return length;
}



