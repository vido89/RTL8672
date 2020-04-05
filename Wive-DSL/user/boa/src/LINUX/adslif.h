/* obcif.h: A RealCom ADSL driver for uClinux. */
/*
	This is for the RTL8670 ADSL router.
*/

#ifndef OBCIF_H
#define OBCIF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <rtk/options.h>

typedef struct OBCIF_ARG {
  int argsize;
  int arg;
}obcif_arg;

// ioctl command called by protocols (system-independent)
#define RLCM_IOC_MAGIC	(('R'+'L'+'C'+'M'+'a'+'d'+'s'+'l') << 8)  //0x2fe00
#define RLCM_PHY_ENABLE_MODEM			(RLCM_IOC_MAGIC + 1)
#define RLCM_PHY_DISABLE_MODEM			(RLCM_IOC_MAGIC + 2)
#define RLCM_GET_DRIVER_VERSION			(RLCM_IOC_MAGIC + 3)
#define RLCM_GET_DRIVER_BUILD			(RLCM_IOC_MAGIC + 4)
#define RLCM_MODEM_RETRAIN			(RLCM_IOC_MAGIC + 5)
#define RLCM_GET_REHS_COUNT			(RLCM_IOC_MAGIC + 6)
#define RLCM_GET_CHANNEL_SNR			(RLCM_IOC_MAGIC + 7)
#define RLCM_GET_AVERAGE_SNR			(RLCM_IOC_MAGIC + 8)
#define RLCM_GET_SNR_MARGIN			(RLCM_IOC_MAGIC + 9)
#define RLCM_REPORT_MODEM_STATE			(RLCM_IOC_MAGIC +10)
#define RLCM_REPORT_PM_DATA			(RLCM_IOC_MAGIC +11)
#define RLCM_MODEM_NEAR_END_ID_REQ		(RLCM_IOC_MAGIC +12)
#define RLCM_MODEM_FAR_END_ID_REQ		(RLCM_IOC_MAGIC +13)
#define RLCM_MODEM_NEAR_END_LINE_DATA_REQ	(RLCM_IOC_MAGIC +14)
#define RLCM_MODEM_FAR_END_LINE_DATA_REQ	(RLCM_IOC_MAGIC +15)
#define RLCM_MODEM_NEAR_END_FAST_CH_DATA_REQ	(RLCM_IOC_MAGIC +16)
#define RLCM_MODEM_NEAR_END_INT_CH_DATA_REQ	(RLCM_IOC_MAGIC +17)
#define RLCM_MODEM_FAR_END_FAST_CH_DATA_REQ	(RLCM_IOC_MAGIC +18)
#define RLCM_MODEM_FAR_END_INT_CH_DATA_REQ	(RLCM_IOC_MAGIC +19)
#define RLCM_SET_ADSL_MODE			(RLCM_IOC_MAGIC +20)
#define RLCM_GET_ADSL_MODE			(RLCM_IOC_MAGIC +21)
#define RLCM_GET_LOSS_DATA			(RLCM_IOC_MAGIC +22)
#define RLCM_GET_LINK_SPEED			(RLCM_IOC_MAGIC +23)
#define RLCM_GET_CHANNEL_MODE			(RLCM_IOC_MAGIC +24)
#define RLCM_GET_LOOP_ATT			(RLCM_IOC_MAGIC +25)
#define RLCM_INC_TX_POWER			(RLCM_IOC_MAGIC +26)
#define RLCM_TUNE_PERF				(RLCM_IOC_MAGIC +27)
#define RLCM_ENABLE_BIT_SWAP			(RLCM_IOC_MAGIC +28)
#define RLCM_DISABLE_BIT_SWAP			(RLCM_IOC_MAGIC +29)
#define RLCM_ENABLE_PILOT_RELOCATION		(RLCM_IOC_MAGIC +30)
#define RLCM_DISABLE_PILOT_RELOCATION		(RLCM_IOC_MAGIC +31)
#define RLCM_ENABLE_TRELLIS			(RLCM_IOC_MAGIC +32)
#define RLCM_DISABLE_TRELLIS			(RLCM_IOC_MAGIC +33)
#define RLCM_SET_VENDOR_ID			(RLCM_IOC_MAGIC +34)
#define RLCM_MODEM_READ_CONFIG                  (RLCM_IOC_MAGIC +35)
#define RLCM_MODEM_WRITE_CONFIG                 (RLCM_IOC_MAGIC +36)
#define RLCM_DEBUG_MODE				(RLCM_IOC_MAGIC +37)
#define RLCM_TEST_PSD				(RLCM_IOC_MAGIC +38)
#define RLCM_GET_ADSL_TIME			(RLCM_IOC_MAGIC +39)
#define RLCM_PHY_START_MODEM			(RLCM_IOC_MAGIC +40)
#define RLCM_ENABLE_ADSL_LOG			(RLCM_IOC_MAGIC +41)
#define RLCM_DISABLE_ADSL_LOG			(RLCM_IOC_MAGIC +42)
//added command
#define RLCM_GET_VENDOR_ID			(RLCM_IOC_MAGIC +43)
#define RLCM_GET_TX_POWER			(RLCM_IOC_MAGIC +44)
#define RLCM_GET_PERF_VALUE			(RLCM_IOC_MAGIC +45)
#define RLCM_GET_15MIN_LOSS_DATA		(RLCM_IOC_MAGIC +46)
#define RLCM_GET_1DAY_LOSS_DATA			(RLCM_IOC_MAGIC +47)
#define RLCM_GET_CHANNEL_BITLOAD		(RLCM_IOC_MAGIC +48)
//for MIB TRAP set
#define RLCM_GET_TRAP_THRESHOLD			(RLCM_IOC_MAGIC +49)
#define RLCM_SET_TRAP_THRESHOLD			(RLCM_IOC_MAGIC +50)
#define RLCM_15MIN_WAIT_TRAP			(RLCM_IOC_MAGIC +51)
//for ATM test
#define RLCM_ENABLE_ATM_LOOPBACK		(RLCM_IOC_MAGIC +52)
#define RLCM_DISABLE_ATM_LOOPBACK		(RLCM_IOC_MAGIC +53)
//new command 9/19/03'
#define RLCM_MSGMODE				(RLCM_IOC_MAGIC +54)
//new command 1/30/04' for API list
#define RLCM_CMD_API				(RLCM_IOC_MAGIC +55)
#define RLCM_GET_CURRENT_LOSS_DATA		(RLCM_IOC_MAGIC +56)
#define RLCM_GET_CHANNEL_BH			(RLCM_IOC_MAGIC +57)
//new command 4/20/04', for MIB trap information get
#define RLCM_GET_TRAP_15MIN_LOSS_DATA		(RLCM_IOC_MAGIC +58)
//tylo, 8671 new command
#define RLCM_SEND_DYING_GASP				(RLCM_IOC_MAGIC +59)
//#define RLCM_READ_D						(RLCM_IOC_MAGIC +60)
//#define RLCM_WRITE_D						(RLCM_IOC_MAGIC +61)
#define RLCM_TEST_HW						(RLCM_IOC_MAGIC +62)
#define RLCM_SET_LOOPBACK					(RLCM_IOC_MAGIC +63)
#define RLCM_INIT_ADSL_MODE				(RLCM_IOC_MAGIC +64)
#define RLCM_ENABLE_POM_ACCESS			(RLCM_IOC_MAGIC +65)
#define RLCM_DISABLE_POM_ACCESS			(RLCM_IOC_MAGIC +66)
#define RLCM_MASK_TONE						(RLCM_IOC_MAGIC +67)
#define RLCM_GET_CAPABILITY				(RLCM_IOC_MAGIC +68)
#define RLCM_VERIFY_HW						(RLCM_IOC_MAGIC +69)
#define RLCM_TRIG_OLR_TYPE					(RLCM_IOC_MAGIC +70)
#define RLCM_TRIG_OLR_TYPE1				(RLCM_IOC_MAGIC +71)
#define RLCM_ENABLE_AEQ					(RLCM_IOC_MAGIC +72)
#define RLCM_ENABLE_HPF					(RLCM_IOC_MAGIC +73)
#define RLCM_SET_HPF_FC					(RLCM_IOC_MAGIC +74)
// new_hibrid
#define RLCM_SET_HYBRID				(RLCM_IOC_MAGIC +75)
#define RLCM_SET_RX_GAIN					(RLCM_IOC_MAGIC +76)
#define RLCM_SET_AFE_REG					(RLCM_IOC_MAGIC +77)
#define RLCM_SET_FOBASE					(RLCM_IOC_MAGIC +78)
//yaru + for webdata
#define RLCM_SET_XDSL_MODE				(RLCM_IOC_MAGIC +79)
#define RLCM_GET_SHOWTIME_XDSL_MODE		(RLCM_IOC_MAGIC +80)
#define RLCM_GET_XDSL_MODE				(RLCM_IOC_MAGIC +81)
#define RLCM_SET_OLR_TYPE					(RLCM_IOC_MAGIC +82)
#define RLCM_GET_OLR_TYPE 					(RLCM_IOC_MAGIC +83)
#define RLCM_GET_LINE_RATE					(RLCM_IOC_MAGIC +84)
#define RLCM_GET_DS_ERROR_COUNT			(RLCM_IOC_MAGIC +85)
#define RLCM_GET_US_ERROR_COUNT			(RLCM_IOC_MAGIC +86)
#define RLCM_GET_DIAG_QLN					(RLCM_IOC_MAGIC +87)
#define RLCM_GET_DIAG_HLOG				(RLCM_IOC_MAGIC +88)
#define RLCM_GET_DIAG_SNR					(RLCM_IOC_MAGIC +89)
#define RLCM_GET_DS_PMS_PARAM1			(RLCM_IOC_MAGIC +90)
#define RLCM_GET_US_PMS_PARAM1			(RLCM_IOC_MAGIC +91)
#define RLCM_SET_ANNEX_L					(RLCM_IOC_MAGIC +92)
#define RLCM_GET_ANNEX_L					(RLCM_IOC_MAGIC +93)
#define RLCM_GET_LINK_POWER_STATE		(RLCM_IOC_MAGIC +94)
#define RLCM_GET_ATT_RATE			(RLCM_IOC_MAGIC +95)
#define RLCM_LOADCARRIERMASK			(RLCM_IOC_MAGIC +96)
#define RLCM_SET_ANNEX_M			(RLCM_IOC_MAGIC +97)
#define RLCM_GET_ANNEX_M			(RLCM_IOC_MAGIC +98)
#define RLCM_SET_8671_REV			(RLCM_IOC_MAGIC +99)
#define RLCM_GET_8671_REV			(RLCM_IOC_MAGIC +100)
#define RLCM_SET_HIGH_INP			(RLCM_IOC_MAGIC +101)
#define RLCM_GET_HIGH_INP			(RLCM_IOC_MAGIC +102)
#define RLCM_GET_LD_STATE			(RLCM_IOC_MAGIC +103)
#define RLCM_SET_ANNEX_B			(RLCM_IOC_MAGIC +104)
#define RLCM_GET_ANNEX_B			(RLCM_IOC_MAGIC +105)
//for TR069
#define RLCM_GET_DSL_STAT_SHOWTIME		(RLCM_IOC_MAGIC +106)
#define RLCM_GET_DSL_STAT_TOTAL			(RLCM_IOC_MAGIC +107)
#define RLCM_GET_DSL_PSD			(RLCM_IOC_MAGIC +108)
#define RLCM_GET_DSL_ORHERS			(RLCM_IOC_MAGIC +109)
#define RLCM_GET_DSL_GI				(RLCM_IOC_MAGIC +110)
// for Telefonica
#define RLCM_SET_ADSL_LAST_OPMode		(RLCM_IOC_MAGIC +111)
#define RLCM_SET_ADSL_PMS_CONFIG		(RLCM_IOC_MAGIC +112)
//Lupin, for TR069
#define RLCM_GET_ADSL2WAN_IFCFG			(RLCM_IOC_MAGIC +113)
#define RLCM_ENABLE_DIAGNOSTIC			(RLCM_IOC_MAGIC +114)




#define GET_LOADCARRIERMASK_SIZE (sizeof(unsigned char)*64)
/*ioctl argument size or struct*/
//for RLCM_GET_DRIVER_VERSION
#define RLCM_DRIVER_VERSION_SIZE	10

//for RLCM_GET_DRIVER_BUILD
#define RLCM_DRIVER_BUILD_SIZE		14

//for RLCM_GET_REHS_COUNT
#define RLCM_GET_REHS_COUNT_SIZE	4  //(sizeof(reHandshakeCount))

//for RLCM_GET_CHANNEL_BITLOAD
#define RLCM_GET_CHANNEL_BITLOAD_SIZE	(sizeof(short)*256)  //(sizeof(gChannelBitLoad))

//for RLCM_GET_CHANNEL_SNR
#define RLCM_GET_CHANNEL_SNR_SIZE	(sizeof(short)*256)  //(sizeof(gChannelSNR))

//for RLCM_GET_AVERAGE_SNR
#define RLCM_GET_AVERAGE_SNR_SIZE	4  //(sizeof(gAverageSNR))

//for RLCM_GET_SNR_MARGIN
#define RLCM_GET_SNR_MARGIN_SIZE	8  //(sizeof(gitex_EstimatedSNRMargin))

//for RLCM_REPORT_MODEM_STATE
#define MODEM_STATE_IDLE                0
#define MODEM_STATE_L3                  1
#define MODEM_STATE_LISTENING           2
#define MODEM_STATE_ACTIVATING          3
#define MODEM_STATE_Ghs_HANDSHAKING     4
#define MODEM_STATE_ANSI_HANDSHAKING    5
#define MODEM_STATE_INITIALIZING        6
#define MODEM_STATE_RESTARTING          7
#define MODEM_STATE_FAST_RETRAIN        8
#define MODEM_STATE_SHOWTIME_L0         9
#define MODEM_STATE_SHOWTIME_LQ         10
#define MODEM_STATE_SHOWTIME_L1         11
#define MODEM_STATE_EXCHANGE            12

//for RLCM_REPORT_PM_DATA
typedef struct {
  unsigned short  FecNotInterleaved;
  unsigned short  FecInterleaved;
  unsigned short  CrcNotInterleaved;
  unsigned short  CrcInterleaved;
  unsigned short  HecNotInterleaved;
  unsigned short  HecInterleaved;
  unsigned short  TotalCellCountInterleaved;
  unsigned short  TotalCellCountNotInterleaved;
  unsigned short  ActiveCellCountInterleaved;
  unsigned short  ActiveCellCountNotInterleaved;
  unsigned short  BERInterleaved;
  unsigned short  BERNotInterleaved;
} Modem_def_counters;

typedef struct {
  Modem_def_counters  near_end;
  Modem_def_counters   far_end;
} Modem_def_counter_set;
#define RLCM_REPORT_PM_DATA_SIZE	sizeof(Modem_def_counter_set) 


/*for 	RLCM_MODEM_NEAR_END_ID_REQ
	RLCM_MODEM_FAR_END_ID_REQ
*/
typedef struct   { 
  unsigned char   countryCode; 
  unsigned char   reserved; 
  unsigned long   vendorCode; 
  unsigned short  vendorSpecific; 
} RLCM_ITU_VendorId; 

typedef struct { 
  RLCM_ITU_VendorId    ITU_VendorId; 
  unsigned char   ITU_StandardRevisionNbr; 
  unsigned short  ANSI_ETSI_VendorId; 
  unsigned char   ANSI_ETSI_VendorRevisionNbr; 
  unsigned char   ANSI_ETSI_StandardRevisionNbr; 
  unsigned long   ALC_ManagementInfo; 
} Modem_Identification; 
#define RLCM_MODEM_ID_REQ_SIZE	sizeof(Modem_Identification) 

//for RLCM_MODEM_NEAR_END_LINE_DATA_REQ
typedef struct {
  unsigned short relCapacityOccupationDnstr;
  // Mason Yu Output Power Error
  //signed char	 noiseMarginDnstr;
  signed short	 noiseMarginDnstr;
  signed char    outputPowerUpstr;
  unsigned char  attenuationDnstr;
  unsigned long operationalMode;
} Modem_NearEndLineOperData;
#define RLCM_MODEM_NEAR_END_LINE_DATA_REQ_SIZE sizeof(Modem_NearEndLineOperData)

//for RLCM_MODEM_FAR_END_LINE_DATA_REQ
typedef struct {
  unsigned short relCapacityOccupationUpstr;
  // Mason Yu Output Power Error
  //signed char	 noiseMarginUpstr;
  signed short	 noiseMarginUpstr;
  signed char	 outputPowerDnstr;
  unsigned char	 attenuationUpstr;
  unsigned char  carrierLoad[256];
} Modem_FarEndLineOperData;
#define RLCM_MODEM_FAR_END_LINE_DATA_REQ_SIZE sizeof(Modem_FarEndLineOperData)

/*for 	RLCM_MODEM_NEAR_END_FAST_CH_DATA_REQ
	RLCM_MODEM_NEAR_END_INT_CH_DATA_REQ
	RLCM_MODEM_FAR_END_FAST_CH_DATA_REQ
	RLCM_MODEM_FAR_END_INT_CH_DATA_REQ
*/
#define RLCM_MODEM_CH_DATA_REQ_SIZE	sizeof(int)

/*for RLCM_SET_ADSL_MODE & RLCM_GET_ADSL_MODE & RLCM_PHY_START_MODEM
If you select multi-mode value (that is, ANSI | G_DMT | G_LITE or ANSI | G_DMT
or ANSI | G_LITE or G_DMT | G_LITE), the handshaking priority will be G_DMT first,
then G_LITE, finally ANSI.
*/
#define ADSL_MODE_ANSI		1	//ANSI T1.413 issue 2 mode
#define ADSL_MODE_G_DMT		2	//ITU G.dmt mode
#define ADSL_MODE_G_LITE	8	//ITU G.lite mode

//for RLCM_GET_LOSS_DATA, RLCM_GET_15MIN_LOSS_DATA, RLCM_GET_1DAY_LOSS_DATA
//
typedef struct {
	unsigned long	LossOfFrame_NE;		//NELOF
	unsigned long	LossOfFrame_FE;		//FELOF
	unsigned long	LossOfSignal_NE;	//NELOS
	unsigned long	LossOfSignal_FE;	//FELOS
	unsigned long	LossOfPower_NE;		//NELPR
	unsigned long	LossOfPower_FE;		//FELPR
	unsigned long	LCD_Fast_NE;		//NELCD_f
	unsigned long	LCD_Fast_FE;		//FELCD_f
	unsigned long	LCD_Interleaved_NE;	//NELCD_f
	unsigned long	LCD_Interleaved_FE;	//FELCD_f
	unsigned long   CrcCounter_Fast_NE;	//CRC error count
	unsigned long   CrcCounter_Interleaved_NE; //CRC error count
	unsigned long   ESs_NE; //Error second count
} Modem_LossData;
#define RLCM_GET_LOSS_DATA_SIZE 	sizeof(Modem_LossData)

//for RLCM_GET_LINK_SPEED
typedef struct {
	unsigned long	upstreamRate;
	unsigned long	downstreamRate;
} Modem_LinkSpeed;
#define RLCM_GET_LINK_SPEED_SIZE 	sizeof(Modem_LinkSpeed)

//for RLCM_GET_CHANNEL_MODE
#define	CH_MODE_FAST		1
#define	CH_MODE_INTERLEAVE	2

//for RLCM_GET_LOOP_ATT
typedef struct {
	unsigned short	upstreamAtt;
	unsigned short	downstreamAtt;
} Modem_AvgLoopAttenuation;
#define RLCM_GET_LOOP_ATT_SIZE 	sizeof(Modem_AvgLoopAttenuation)

//for RLCM_MODEM_READ_CONFIG, RLCM_MODEM_WRITE_CONFIG
typedef struct {
	int HandshakeMode;
        int TxPower;
        int PerfTuning;
        int AtuC_VendorID;
        int BitSwapEnable;
        int PilotRelocationEnable;
        int TrellisEnable;
} Modem_Config;
#define RLCM_MODEM_CONFIG_SIZE 	sizeof(Modem_Config)


//for RLCM_TEST_PSD, other value is normal operation
#define TEST_SEND_QUIET		1	//send Quiet
#define TEST_SEND_REVERB1	2	//send Reverb1

//for RLCM_GET_TRAP_THRESHOLD
#define RLCM_TRAP_THRESHOLD_SIZE 	sizeof(Modem_LossData)

//for RLCM_15MIN_WAIT_TRAP
#define RLCM_WAIT_TRAP_ES_NE	0	//wait 15min ATUR Error second trap

//yaru for TR069
typedef struct {
	unsigned int	ReceiveBlocks;
	unsigned int	TransmitBlocks;
	unsigned int	CellDelin;
	unsigned int	LinkRetain;
	unsigned int	InitErrors;
	unsigned int	InitTimeouts;
	unsigned int	LOF;
	unsigned int	ES;
	unsigned int	SES;
	unsigned int	FEC;
	unsigned int	AtucFEC;
	unsigned int	HEC;
	unsigned int	AtucHEC;
	unsigned int	CRC;
	unsigned int	AtucCRC;
} Modem_DSLConfigStatus;	//yaru TR069, copy T_DSLConfigStatus


//Modem_ADSL2WANConfig.LinkEncapSupported/LinkEncapRequested/LinkEncapUsed:
enum
{
	LE_G_992_3_ANNEX_K_ATM=0,
	LE_G_992_3_ANNEX_K_PTM,
	LE_G_993_2_ANNEX_K_ATM,
	LE_G_993_2_ANNEX_K_PTM,
	LE_G_994_1,
	
	LE_END /*the last one*/
};
//Modem_ADSL2WANConfig.StandardsSuported/StandardUsed:
enum
{
	STD_G_992_1_Annex_A=0,
	STD_G_992_1_Annex_B,
	STD_G_992_1_Annex_C,
	STD_T1_413,
	STD_T1_413i2,
	STD_ETSI_101_388,
	STD_G_992_2,
	STD_G_992_3_Annex_A,
	STD_G_992_3_Annex_B,
	STD_G_992_3_Annex_C,
	
	STD_G_992_3_Annex_I,
	STD_G_992_3_Annex_J,
	STD_G_992_3_Annex_L,
	STD_G_992_3_Annex_M,
	STD_G_992_4,
	STD_G_992_5_Annex_A,
	STD_G_992_5_Annex_B,
	STD_G_992_5_Annex_C,
	STD_G_992_5_Annex_I,
	STD_G_992_5_Annex_J,
	
	STD_G_992_5_Annex_M,
	STD_G_993_1,
	STD_G_993_1_Annex_A,
	STD_G_993_2_Annex_A,
	STD_G_993_2_Annex_B,
	STD_G_993_1_Annex_C,
	
	STD_END /*the last one*/
};

typedef struct {
	unsigned int LinkEncapSupported;
	unsigned int LinkEncapRequested;
	unsigned int LinkEncapUsed;
	unsigned int StandardsSuported;
	unsigned int StandardUsed;		
} Modem_ADSL2WANConfig; //Lupin, tr069

#define TR069_ADSL2WANCFG_SIZE sizeof(Modem_ADSL2WANConfig)
#define TR069_STAT_SIZE sizeof(Modem_DSLConfigStatus)
#define TR069_DIAG_GI_SIZE (sizeof(unsigned short)*(TONE_RANGE+1)))
#define TR069_DSL_PSD_SIZE (sizeof(int)*2)
#define TR069_DSL_OTHER_SIZE (sizeof(unsigned int)*3)
//end for TR069

#ifdef FIELD_TRY_SAFE_MODE
#define RLCM_GET_SAFEMODE_CTRL  (RLCM_IOC_MAGIC +115)
#define RLCM_SET_SAFEMODE_CTRL  (RLCM_IOC_MAGIC +116)

typedef struct {
	int FieldTrySafeMode;
        int FieldTryTestPSDTimes;
        int FieldTryCtrlIn;
        char SafeModeNote[20];
  } SafeModeData;
#define SAFEMODE_DATA_SIZE sizeof(SafeModeData)
#endif

//additional size for RLCM_GET_DIAG_HLOG ioctl, sizeof(short)*(MAX_DSL_TONE*3+HLOG_ADDITIONAL_SIZE)
#define HLOG_ADDITIONAL_SIZE	13

//use RLCM_ENABLE_DIAGNOSTIC to start dsl diag. (instead of RLCM_DEBUG_MODE ioctl (mode=41))
#define _USE_NEW_IOCTL_FOR_DSLDIAG_	1

#ifdef __cplusplus
}
#endif

#endif // OBCIF_H

