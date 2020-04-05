/*
 *      Web server handler routines for wlan stuffs
 *
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *
 *      $Id: fmwlan.c,v 1.49 2009/02/13 13:33:30 cathy Exp $
 *
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "../webs.h"
#include "mib.h"
#include "webform.h"
#include "utility.h"
//xl_yue add
#include "../defs.h"
//static SS_STATUS_Tp pStatus=NULL;
#ifdef WLAN_MBSSID
//const char *WLANAPIF[] = {
//	"wlan0", "wlan0-vap0", "wlan0-vap1", "wlan0-vap2", "wlan0-vap3", 0
//};
extern const char *WLANAPIF[];
#endif

#ifdef CONFIG_USB_RTL8192SU_SOFTAP
WLAN_RATE_T rate_11n_table_20M_LONG[]={
	{MCS0, 	"6.5"},
	{MCS1, 	"13"},
	{MCS2, 	"19.5"},
	{MCS3, 	"26"},
	{MCS4, 	"39"},
	{MCS5, 	"52"},
	{MCS6, 	"58.5"},
	{MCS7, 	"65"},
	{MCS8, 	"13"},
	{MCS9, 	"26"},
	{MCS10, 	"39"},
	{MCS11, 	"52"},
	{MCS12, 	"78"},
	{MCS13, 	"104"},
	{MCS14, 	"117"},
	{MCS15, 	"130"},
	{0}
};
WLAN_RATE_T rate_11n_table_20M_SHORT[]={
	{MCS0, 	"7.2"},
	{MCS1, 	"14.4"},
	{MCS2, 	"21.7"},
	{MCS3, 	"28.9"},
	{MCS4, 	"43.3"},
	{MCS5, 	"57.8"},
	{MCS6, 	"65"},
	{MCS7, 	"72.2"},
	{MCS8, 	"14.444"},
	{MCS9, 	"28.889"},
	{MCS10, 	"43.333"},
	{MCS11, 	"57.778"},
	{MCS12, 	"86.667"},
	{MCS13, 	"115.556"},
	{MCS14, 	"130"},
	{MCS15, 	"144.444"},
	{0}
};
WLAN_RATE_T rate_11n_table_40M_LONG[]={
	{MCS0, 	"13.5"},
	{MCS1, 	"27"},
	{MCS2, 	"40.5"},
	{MCS3, 	"54"},
	{MCS4, 	"81"},
	{MCS5, 	"108"},
	{MCS6, 	"121.5"},
	{MCS7, 	"135"},
	{MCS8, 	"27"},
	{MCS9, 	"54"},
	{MCS10, 	"81"},
	{MCS11, 	"108"},
	{MCS12, 	"162"},
	{MCS13, 	"216"},
	{MCS14, 	"243"},
	{MCS15, 	"270"},
	{0}
};
WLAN_RATE_T rate_11n_table_40M_SHORT[]={
	{MCS0, 	"15"},
	{MCS1, 	"30"},
	{MCS2, 	"45"},
	{MCS3, 	"60"},
	{MCS4, 	"90"},
	{MCS5, 	"120"},
	{MCS6, 	"135"},
	{MCS7, 	"150"},
	{MCS8, 	"30"},
	{MCS9, 	"60"},
	{MCS10, 	"90"},
	{MCS11, 	"120"},
	{MCS12, 	"180"},
	{MCS13, 	"240"},
	{MCS14, 	"270"},
	{MCS15, 	"300"},
	{0}
};
WLAN_RATE_T tx_fixed_rate[]={
	{1, "1"},
	{(1<<1), 	"2"},
	{(1<<2), 	"5.5"},
	{(1<<3), 	"11"},
	{(1<<4), 	"6"},
	{(1<<5), 	"9"},
	{(1<<6), 	"12"},
	{(1<<7), 	"18"},
	{(1<<8), 	"24"},
	{(1<<9), 	"36"},
	{(1<<10), 	"48"},
	{(1<<11), 	"54"},
	{(1<<12), 	"MCS0"},
	{(1<<13), 	"MCS1"},
	{(1<<14), 	"MCS2"},
	{(1<<15), 	"MCS3"},
	{(1<<16), 	"MCS4"},
	{(1<<17), 	"MCS5"},
	{(1<<18), 	"MCS6"},
	{(1<<19), 	"MCS7"},
	{(1<<20), 	"MCS8"},
	{(1<<21), 	"MCS9"},
	{(1<<22), 	"MCS10"},
	{(1<<23), 	"MCS11"},
	{(1<<24), 	"MCS12"},
	{(1<<25), 	"MCS13"},
	{(1<<26), 	"MCS14"},
	{(1<<27), 	"MCS15"},
	{0}
};
#endif



/////////////////////////////////////////////////////////////////////////////
#ifndef NO_ACTION
static void run_script(int mode)
{
	int pid;
	char tmpBuf[100];
	pid = fork();
       	if (pid)
               	waitpid(pid, NULL, 0);
       	else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _CONFIG_SCRIPT_PROG);
#ifdef HOME_GATEWAY
		execl( tmpBuf, _CONFIG_SCRIPT_PROG, "gw", "bridge", NULL);
#else
		execl( tmpBuf, _CONFIG_SCRIPT_PROG, "ap", "bridge", NULL);
#endif
               	exit(1);
	}
}
#endif

/////////////////////////////////////////////////////////////////////////////
static inline int isAllStar(char *data)
{
//xl_yue: for 531b, all star is a valid key,and wep key is displayed on webpage
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	return 0;
#else
	int i;
	for (i=0; i<strlen(data); i++) {
		if (data[i] != '*')
			return 0;
	}
	return 1;
#endif
}

/////////////////////////////////////////////////////////////////////////////
void formWlanSetup(webs_t wp, char_t *path, char_t *query)
{
   	char_t *submitUrl, *strSSID, *strChan, *strDisabled, *strVal;
	char_t vChar, chan, disabled, mode=-1;
	NETWORK_TYPE_T net;
	char tmpBuf[100];
	int flags;
//xl_yue
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	char_t *strAuth, *strRate, *strHiddenSSID,*strDomain;
	AUTH_TYPE_T authType;
	unsigned short uShort;
	char_t Domain;
	int value;
#endif

	strDisabled = websGetVar(wp, T("wlanDisabled"), T(""));
	
	if ( !gstrcmp(strDisabled, T("ON")) )
		disabled = 1;
	else
		disabled = 0;
	update_wlan_disable(disabled);

	if (getInFlags("wlan0", &flags) == 1) {
		if (disabled)
			flags &= ~IFF_UP;
		else
			flags |= IFF_UP;
		
		setInFlags("wlan0", flags);
	}

	if ( mib_set( MIB_WLAN_DISABLED, (void *)&disabled) == 0) {
		strcpy(tmpBuf, T(strDisbWlanErr));
		goto setErr_wlan;
	}

	if ( disabled )
		goto setwlan_ret;
	
	// Added by Mason Yu for TxPower
	strVal = websGetVar(wp, T("txpower"), T(""));
	if ( strVal[0] ) {
//modified by xl_yue
//#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
#ifdef WLAN_TX_POWER_DISPLAY
		if (strVal[0] < '0' || strVal[0] > '4') {
			strcpy(tmpBuf, T(strInvdTxPower));
			goto setErr_wlan;
		}
#else
		if (strVal[0]!= '0' && strVal[0]!= '1' && strVal[0]!= '2') {
			strcpy(tmpBuf, T(strInvdTxPower));
			goto setErr_wlan;
		}
#endif
		mode = strVal[0] - '0';	

#if 0		
		printf("mode=%d\n", mode);
		if (mode == 0){
			printf("TxPower=15mW\n");
		}else if (mode == 1) {
			printf("TxPower=30mW\n");
		}else if (mode == 2) {
			printf("TxPower=60mW\n");
		}
#endif		
		
		if ( mib_set( MIB_TX_POWER, (void *)&mode) == 0) {
   			strcpy(tmpBuf, T(strSetMIBTXPOWErr));
			goto setErr_wlan;
		}		
		
	}
	
	strVal = websGetVar(wp, T("mode"), T(""));
	if ( strVal[0] ) {
		if (strVal[0]!= '0' && strVal[0]!= '1' && strVal[0]!= '2' && strVal[0]!= '3') {
			strcpy(tmpBuf, T(strInvdMode));
			goto setErr_wlan;
		}
		mode = strVal[0] - '0';

#ifdef WLAN_CLIENT
		if (mode == CLIENT_MODE) {
			ENCRYPT_T encrypt;
			
              		mib_get( MIB_WLAN_ENCRYPT,  (void *)&vChar);
              		encrypt = (ENCRYPT_T)vChar;
			if (encrypt == ENCRYPT_WPA) {
				mib_get( MIB_WLAN_WPA_AUTH, (void *)&vChar);
				if (vChar & 1) { // radius
					strcpy(tmpBuf, T(strSetWPAWarn));
					goto setErr_wlan;
				}
			}
			else if (encrypt == ENCRYPT_WEP) {
				mib_get( MIB_WLAN_ENABLE_1X, (void *)&vChar);
				if (vChar & 1) {
					strcpy(tmpBuf, T(strSetWEPWarn));
					goto setErr_wlan;
				}
			}
		}
#endif

		if ( mib_set( MIB_WLAN_MODE, (void *)&mode) == 0) {
			strcpy(tmpBuf, T(strSetMIBWLANMODEErr));
			goto setErr_wlan;
		}
	}

   	strSSID = websGetVar(wp, T("ssid"), T(""));
	if ( strSSID[0] ) {
		if ( mib_set(MIB_WLAN_SSID, (void *)strSSID) == 0) {
   	 			strcpy(tmpBuf, T(strSetSSIDErr));
				goto setErr_wlan;
		}
	}
	else if ( mode == 1 && !strSSID[0] ) { // client and NULL SSID
		if ( mib_set(MIB_WLAN_SSID, (void *)strSSID) == 0) {
   	 			strcpy(tmpBuf, T(strSetSSIDErr));
				goto setErr_wlan;
		}
	}

#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	//added by xl_yue: set regDomain for supporting to select channel accoriding to countries or areas
	strDomain =  websGetVar(wp, T("domain"), T(""));
	if(strDomain[0]){
		if(!string_to_dec(strDomain, &value)){
			strcpy(tmpBuf, T(strSetCOUNTRYAREAErr));
			goto setErr_wlan;
		}
			
		Domain = value;
		vChar = wlCountryRegList[Domain].regDomain;

		if ( mib_set(MIB_WLAN_COUNTRY_AREA, (void *)&Domain) == 0) {
			strcpy(tmpBuf, T(strSetCOUNTRYAREAErr));
			goto setErr_wlan;
		}

		if ( mib_set(MIB_WLAN_REG_DOMAIN, (void *)&vChar) == 0) {
			strcpy(tmpBuf, T(strSetREGDOMAINErr));
			goto setErr_wlan;
		}
	}
#endif

	strChan = websGetVar(wp, T("chan"), T(""));
	if ( strChan[0] ) {
		errno=0;
		chan = strtol( strChan, (char **)NULL, 10);
		if (errno) {
   			strcpy(tmpBuf, T(strInvdChanNum));
			goto setErr_wlan;
		}
		if ( mib_set( MIB_WLAN_CHAN_NUM, (void *)&chan) == 0) {
			strcpy(tmpBuf, T(strSetChanErr));
			goto setErr_wlan;
		}
	}

	strVal = websGetVar(wp, T("type"), T(""));
	if (strVal[0]) {
		if (strVal[0]!= '0' && strVal[0]!= '1') {
			strcpy(tmpBuf, T(strInvdNetType));
			goto setErr_wlan;
		}
		if (strVal[0] == '0')
			net = INFRASTRUCTURE;
		else
			net = ADHOC;
		vChar = (char_t)net;
		if ( mib_set(MIB_WLAN_NETWORK_TYPE, (void *)&vChar) == 0) {
			strcpy(tmpBuf, T(strSetMIBWLANTYPEErr));
			goto setErr_wlan;
		}
	}

//xl_yue: translocate from advance setting to basic_setting  for ZTE531B BRIDGE
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	strAuth = websGetVar(wp, T("authType"), T(""));
	if (strAuth[0]) {
		if ( !gstrcmp(strAuth, T("open")))
			authType = AUTH_OPEN;
		else if ( !gstrcmp(strAuth, T("shared")))
			authType = AUTH_SHARED;
		else if ( !gstrcmp(strAuth, T("both")))
			authType = AUTH_BOTH;
		else {
			strcpy(tmpBuf, T(strInvdAuthType));
			goto setErr_wlan;
		}
		vChar = (char_t)authType;
		if ( mib_set(MIB_WLAN_AUTH_TYPE, (void *)&vChar) == 0) {
			strcpy(tmpBuf, T(strSetAuthTypeErr));
			goto setErr_wlan;
		}
	}

	// set tx rate
	strRate = websGetVar(wp, T("txRate"), T(""));
	if ( strRate[0] ) {
		if ( strRate[0] == '0' ) { // auto
			vChar = 1;
			if ( mib_set(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&vChar) == 0) {
				strcpy(tmpBuf, T(strSetAdapRateErr));
				goto setErr_wlan;
			}
		}
		else  {
			vChar = 0;
			if ( mib_set(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&vChar) == 0) {
				strcpy(tmpBuf, T(strSetAdapRateErr));
				goto setErr_wlan;
			}
#ifdef WLAN_8185AG
#ifdef CONFIG_USB_RTL8192SU_SOFTAP
			{
				unsigned int uInt;
				uInt = atoi(strRate);
				uInt = 1 << (uInt-1);
				if ( mib_set(MIB_WLAN_FIX_RATE, (void *)&uInt) == 0) {
					strcpy(tmpBuf, T(strSetFixdRateErr));
					goto setErr_wlan;
				}
			}
#else
			uShort = atoi(strRate);
			uShort = 1 << (uShort-1);
			if ( mib_set(MIB_WLAN_FIX_RATE, (void *)&uShort) == 0) {
				strcpy(tmpBuf, T(strSetFixdRateErr));
				goto setErr_wlan;
			}
#endif			
#if 0
			strRate = websGetVar(wp, T("basicrates"), T(""));
			if ( strRate[0] ) {
				uShort = atoi(strRate);
				if ( mib_set(MIB_WLAN_BASIC_RATE, (void *)&uShort) == 0) {
					strcpy(tmpBuf, T(strSetBaseRateErr));
					goto setErr_wlan;
				}
			}

			strRate = websGetVar(wp, T("operrates"), T(""));
			if ( strRate[0] ) {
				uShort = atoi(strRate);
				if ( mib_set(MIB_WLAN_SUPPORTED_RATE, (void *)&uShort) == 0) {
					strcpy(tmpBuf, T(strSetOperRateErr));
					goto setErr_wlan;
				}
			}
#endif
#else
			if (strRate[0] == '1' )  // 11M
				uShort = TX_RATE_11M;
			else if (strRate[0] == '2' ) // 5.5M
				uShort = TX_RATE_5M;
			else if (strRate[0] == '3' ) // 2M
				uShort = TX_RATE_2M;
			else if (strRate[0] == '4' ) // 1M
				uShort = TX_RATE_1M;
			else {
				strcpy(tmpBuf, T(strInvdTxRate));
				goto setErr_wlan;
			}
			if ( mib_set(MIB_WLAN_SUPPORTED_RATE, (void *)&uShort) == 0) {
				strcpy(tmpBuf, T(strSetOperRateErr));
				goto setErr_wlan;
			}
			if ( mib_set(MIB_WLAN_BASIC_RATE, (void *)&uShort) == 0) {
				strcpy(tmpBuf, T(strSetBaseRateErr));
				goto setErr_wlan;
			}
#endif
		}
	}
	else { // set rate in operate, basic sperately
#ifdef WIFI_TEST
		vChar = 0;
		uShort = 0;
		// disable rate adaptive
		if ( mib_set(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&vChar) == 0) {
			strcpy(tmpBuf, T(strSetAdapRateErr));
			goto setErr_wlan;
			}
#if 0
		strRate = websGetVar(wp, T("operRate1M"), T(""));
		if ( !gstrcmp(strRate, T("1M")))
			uShort |= TX_RATE_1M;
		strRate = websGetVar(wp, T("operRate2M"), T(""));
		if ( !gstrcmp(strRate, T("2M")))
			uShort |= TX_RATE_2M;
		strRate = websGetVar(wp, T("operRate5M"), T(""));
		if ( !gstrcmp(strRate, T("5M")))
			uShort |= TX_RATE_5M;
		strRate = websGetVar(wp, T("operRate11M"), T(""));
		if ( !gstrcmp(strRate, T("11M")))
			uShort |= TX_RATE_11M;
		strRate = websGetVar(wp, T("operRate6M"), T(""));
		if ( !gstrcmp(strRate, T("6M")))
			uShort |= TX_RATE_6M;
		strRate = websGetVar(wp, T("operRate9M"), T(""));
		if ( !gstrcmp(strRate, T("9M")))
			uShort |= TX_RATE_9M;
		strRate = websGetVar(wp, T("operRate12M"), T(""));
		if ( !gstrcmp(strRate, T("12M")))
			uShort |= TX_RATE_12M;
		strRate = websGetVar(wp, T("operRate18M"), T(""));
		if ( !gstrcmp(strRate, T("18M")))
			uShort |= TX_RATE_18M;			
		strRate = websGetVar(wp, T("operRate24M"), T(""));
		if ( !gstrcmp(strRate, T("24M")))
			uShort |= TX_RATE_24M;			
		strRate = websGetVar(wp, T("operRate36M"), T(""));
		if ( !gstrcmp(strRate, T("36M")))
			uShort |= TX_RATE_36M;			
		strRate = websGetVar(wp, T("operRate48M"), T(""));
		if ( !gstrcmp(strRate, T("48M")))
			uShort |= TX_RATE_48M;			
		strRate = websGetVar(wp, T("operRate54M"), T(""));
		if ( !gstrcmp(strRate, T("54M")))
			uShort |= TX_RATE_54M;
		if ( mib_set(MIB_WLAN_SUPPORTED_RATE, (void *)&uShort) == 0) {
			strcpy(tmpBuf, T("Set Tx operation rate failed!"));
			goto setErr_wlan;
		}

		// set basic tx rate
		uShort = 0;
		strRate = websGetVar(wp, T("basicRate1M"), T(""));
		if ( !gstrcmp(strRate, T("1M")))
			uShort |= TX_RATE_1M;
		strRate = websGetVar(wp, T("basicRate2M"), T(""));
		if ( !gstrcmp(strRate, T("2M")))
			uShort |= TX_RATE_2M;
		strRate = websGetVar(wp, T("basicRate5M"), T(""));
		if ( !gstrcmp(strRate, T("5M")))
			uShort |= TX_RATE_5M;
		strRate = websGetVar(wp, T("basicRate11M"), T(""));
		if ( !gstrcmp(strRate, T("11M")))
			uShort |= TX_RATE_11M;
		strRate = websGetVar(wp, T("basicRate6M"), T(""));
		if ( !gstrcmp(strRate, T("6M")))
			uShort |= TX_RATE_6M;
		strRate = websGetVar(wp, T("basicRate9M"), T(""));
		if ( !gstrcmp(strRate, T("9M")))
			uShort |= TX_RATE_9M;
		strRate = websGetVar(wp, T("basicRate12M"), T(""));
		if ( !gstrcmp(strRate, T("12M")))
			uShort |= TX_RATE_12M;
		strRate = websGetVar(wp, T("basicRate18M"), T(""));
		if ( !gstrcmp(strRate, T("18M")))
			uShort |= TX_RATE_18M;			
		strRate = websGetVar(wp, T("basicRate24M"), T(""));
		if ( !gstrcmp(strRate, T("24M")))
			uShort |= TX_RATE_24M;			
		strRate = websGetVar(wp, T("basicRate36M"), T(""));
		if ( !gstrcmp(strRate, T("36M")))
			uShort |= TX_RATE_36M;			
		strRate = websGetVar(wp, T("basicRate48M"), T(""));
		if ( !gstrcmp(strRate, T("48M")))
			uShort |= TX_RATE_48M;			
		strRate = websGetVar(wp, T("basicRate54M"), T(""));
		if ( !gstrcmp(strRate, T("54M")))
			uShort |= TX_RATE_54M;			
		if ( mib_set(MIB_WLAN_BASIC_RATE, (void *)&uShort) == 0) {
			strcpy(tmpBuf, T("Set Tx basic rate failed!"));
			goto setErr_wlan;
		}
#endif
#endif // of WIFI_TEST
	}

	// set hidden SSID
	strHiddenSSID = websGetVar(wp, T("hiddenSSID"), T(""));
	if (strHiddenSSID[0]) {
		if (!gstrcmp(strHiddenSSID, T("no")))
			vChar = 0;
		else if (!gstrcmp(strHiddenSSID, T("yes")))
			vChar = 1;
		else {
			strcpy(tmpBuf, T(strInvdBrodSSID));
			goto setErr_wlan;
		}
		if ( mib_set(MIB_WLAN_HIDDEN_SSID, (void *)&vChar) == 0) {
			strcpy(tmpBuf, T(strSetBrodSSIDErr));
			goto setErr_wlan;
		}
	}

#endif


#ifdef WLAN_8185AG
{	char_t *strRate;
	unsigned short val;

	strVal = websGetVar(wp, T("band"), T(""));
	if ( strVal[0] ) {
#if 0
		if (strVal[0]!= '0' && strVal[0]!= '1' && strVal[0]!= '2' && strVal[0]!= '3') {
  			strcpy(tmpBuf, T("Invalid band value!"));
			goto setErr_wlan;
		}
#endif
#ifdef CONFIG_USB_RTL8192SU_SOFTAP
		mode = atoi(strVal);
		mode++;
#else
		mode = strVal[0] - '0' + 1;
#endif
		if ( mib_set( MIB_WLAN_BAND, (void *)&mode) == 0) {
			strcpy(tmpBuf, T(strSetBandErr));
			goto setErr_wlan;
		}
	}

	strRate = websGetVar(wp, T("basicrates"), T(""));
	if ( strRate[0] ) {
		val = atoi(strRate);
		if ( mib_set(MIB_WLAN_BASIC_RATE, (void *)&val) == 0) {
			strcpy(tmpBuf, T(strSetBaseRateErr));
			goto setErr_wlan;
		}
	}

	strRate = websGetVar(wp, T("operrates"), T(""));
	if ( strRate[0] ) {
		val = atoi(strRate);
		if ( mib_set(MIB_WLAN_SUPPORTED_RATE, (void *)&val) == 0) {
			strcpy(tmpBuf, T(strSetOperRateErr));
			goto setErr_wlan;
		}
	}
}
#endif

	strVal = websGetVar(wp, T("chanwid"), T(""));            //add by yq_zhou 2.10
	if ( strVal[0] ) {
		mode = strVal[0] - '0';
#ifdef CONFIG_USB_RTL8192SU_SOFTAP
		if ( mib_set( MIB_WLAN_CHANNEL_WIDTH, (void *)&mode) == 0) {   
			strcpy(tmpBuf, T(strSetChanWidthErr)); 
			goto setErr_wlan;
		}
#endif
	}
	
	strVal = websGetVar(wp, T("ctlband"), T(""));            //add by yq_zhou 2.10
	if ( strVal[0] ) {
		mode = strVal[0] - '0';
#ifdef CONFIG_USB_RTL8192SU_SOFTAP
		if ( mib_set( MIB_WLAN_CONTROL_BAND, (void *)&mode) == 0) {
			strcpy(tmpBuf, T(strSetCtlBandErr));
			goto setErr_wlan;
		}
#endif
	}
setwlan_ret:
//#ifdef APPLY_CHANGE		
	
//#endif
//	mib_update(CURRENT_SETTING);

#ifndef NO_ACTION
	run_script(mode);
#endif

#ifdef CONFIG_WIFI_SIMPLE_CONFIG//WPS def WIFI_SIMPLE_CONFIG
	{
	int ret;
	char_t *wepKey;
        sprintf(tmpBuf, "wps_clear_configure_by_reg%d", 0);
        wepKey = websGetVar(wp, tmpBuf, NULL);
        ret = 0;
        if (wepKey && wepKey[0])
                ret = atoi(wepKey);
        update_wps_configured(ret);
	}
#endif

	config_WLAN(ACT_RESTART);	

	submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page
	OK_MSG(submitUrl);

	return;

setErr_wlan:
	ERR_MSG(tmpBuf);
}

/////////////////////////////////////////////////////////////////////////////
void formWep(webs_t wp, char_t *path, char_t *query)
{
   	char_t *submitUrl, *wepKey;
   	char_t *strKeyLen, *strFormat, *strKeyId, *strEnabled, *strVal;
   	char_t vChar;
	char tmpBuf[100], key[30];
	int enabled, keyLen, ret, i;
	WEP_T wep;
#ifdef WLAN_MBSSID	
	MIB_CE_MBSSIB_WEP_T Entry;
	MIB_CE_MBSSIB_T EntryWPA;
#endif
	
	// Added by Mason Yu
#ifdef WLAN_MBSSID
	strVal = websGetVar(wp, T("wepSSID"), T(""));
	if (strVal[0]) {
		if ( !gstrcmp(strVal, T("root"))) {
			i = 0;
		}
		else if ( !gstrcmp(strVal, T("vap0"))) {
			i = 1;
		}
		else if ( !gstrcmp(strVal, T("vap1"))) {
			i = 2;
		}
		else if ( !gstrcmp(strVal, T("vap2"))) {
			i = 3;
		}
		else if ( !gstrcmp(strVal, T("vap3"))) {
			i = 4;
		}
		else {	
			strcpy(tmpBuf, T(strNotSuptSSIDType));				
			goto setErr_wep;
		}
		
	} else {
		strcpy(tmpBuf, T(strNoSSIDTypeErr));				
		goto setErr_wep;
	}	
	
	if (!mib_chain_get(MIB_MBSSIB_WEP_TBL, i, (void *)&Entry)) {  		
  		strcpy(tmpBuf, T(strGetMBSSIBWEPTBLErr));				
		goto setErr_wep;		
	}
	// Added by Mason Yu. End
	/*	
	printf("Entry.wep64Key1=%s\n", Entry.wep64Key1);
	printf("Entry.wep64Key2=%s\n", Entry.wep64Key2);
	printf("Entry.wep64Key3=%s\n", Entry.wep64Key3);
	printf("Entry.wep64Key4=%s\n", Entry.wep64Key4);
	printf("Entry.wep128Key1=%s\n", Entry.wep128Key1);
	printf("Entry.wep128Key2=%s\n", Entry.wep128Key2);
	printf("Entry.wep128Key3=%s\n", Entry.wep128Key3);
	printf("Entry.wep128Key4=%s\n", Entry.wep128Key4);	
	printf("Entry.wepDefaultKey=%d\n", Entry.wepDefaultKey);
	printf("Entry.wepKeyType=%d\n", Entry.wepKeyType);
	*/
#endif
	
	strEnabled = websGetVar(wp, T("wepEnabled"), T(""));
	if ( !gstrcmp(strEnabled, T("ON")))
		enabled = 1;
	else
		enabled = 0;

	if ( enabled ) {
		strKeyLen = websGetVar(wp, T("length"), T(""));
		if (!strKeyLen[0]) {
 			strcpy(tmpBuf, T(strKeyLenMustExist));
			goto setErr_wep;
		}
		if (strKeyLen[0]!='1' && strKeyLen[0]!='2') {
 			strcpy(tmpBuf, T(strInvdKeyLen));
			goto setErr_wep;
		}
		if (strKeyLen[0] == '1')
			wep = WEP64;
		else
			wep = WEP128;
	}
	else
		wep = WEP_DISABLED;
	
	vChar = (char_t)wep;

	#ifdef WLAN_MBSSID
	if (0 != i) {
		if (!mib_chain_get(MIB_MBSSIB_TBL, i, (void *)&EntryWPA)) {  		
  			strcpy(tmpBuf, T(strGetMBSSIBTBLErr));				
			goto setErr_wep;		
		}
		EntryWPA.wep = vChar;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&EntryWPA, i);
	} else 		
	#endif
	if ( mib_set( MIB_WLAN_WEP, (void *)&vChar) == 0) {
  		strcpy(tmpBuf, T(strSetWLANWEPErr));
		goto setErr_wep;
	}	

#ifdef WLAN_WPA
{	ENCRYPT_T encrypt=ENCRYPT_WEP;

	if (wep == WEP_DISABLED)
		encrypt = ENCRYPT_DISABLED;
	
	vChar = (char_t)encrypt;

	#ifdef WLAN_MBSSID
	if (0 != i) {
		if (!mib_chain_get(MIB_MBSSIB_TBL, i, (void *)&EntryWPA)) {  		
  			strcpy(tmpBuf, T(strGetMBSSIBTBLErr));				
			goto setErr_wep;		
		}
		EntryWPA.encrypt = vChar;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&EntryWPA, i);
	} else 		
	#endif
	if (mib_set( MIB_WLAN_ENCRYPT, (void *)&vChar) == 0) {
  		strcpy(tmpBuf, T(strSetWLANENCRYPTErr));
		goto setErr_wep;
	}	
}
#endif

	if (wep == WEP_DISABLED)
		goto setwep_ret;

	strFormat = websGetVar(wp, T("format"), T(""));	
	if (!strFormat[0]) {
 		strcpy(tmpBuf, T(strKeyTypeMustExist));
		goto setErr_wep;
	}

	if (strFormat[0]!='1' && strFormat[0]!='2') {
		strcpy(tmpBuf, T(strInvdKeyType));
		goto setErr_wep;
	}

	vChar = (char_t)(strFormat[0] - '0' - 1);

	#ifdef WLAN_MBSSID
	if (0 != i) {
		Entry.wepKeyType = vChar;
	} else 		
	#endif
	if ( mib_set( MIB_WLAN_WEP_KEY_TYPE, (void *)&vChar) == 0) {
  		strcpy(tmpBuf, T(strSetWepKeyTypeErr));
		goto setErr_wep;
	}	

	if (wep == WEP64) {
		if (strFormat[0]=='1')
			keyLen = WEP64_KEY_LEN;
		else
			keyLen = WEP64_KEY_LEN*2;
	}
	else {
		if (strFormat[0]=='1')
			keyLen = WEP128_KEY_LEN;
		else
			keyLen = WEP128_KEY_LEN*2;
	}

	strKeyId = websGetVar(wp, T("defaultTxKeyId"), T(""));
	if ( strKeyId[0] ) {
		if ( strKeyId[0]!='1' && strKeyId[0]!='2' && strKeyId[0]!='3' && strKeyId[0]!='4' ) {
	 		strcpy(tmpBuf, T(strInvdDeftKey));
   			goto setErr_wep;
		}
		vChar = (char_t)(strKeyId[0] - '0' - 1);

		#ifdef WLAN_MBSSID
		if (0 != i) {
			Entry.wepDefaultKey = vChar;		
		} else 		
		#endif
		if ( !mib_set( MIB_WLAN_WEP_DEFAULT_KEY, (void *)&vChar ) ) {
	 		strcpy(tmpBuf, T(strSetDeftKeyErr));
   			goto setErr_wep;
		}		
	}

	wepKey = websGetVar(wp, T("key1"), T(""));
	if  (wepKey[0]) {
		if (strlen(wepKey) != keyLen) {
			strcpy(tmpBuf, T(strInvdKey1Len));
			goto setErr_wep;
		}
		if ( !isAllStar(wepKey) ) {
			if (strFormat[0] == '1') // ascii
				strcpy(key, wepKey);
			else { // hex
				if ( !string_to_hex(wepKey, key, keyLen)) {
	   				strcpy(tmpBuf, T(strInvdWEPKey1));
					goto setErr_wep;
				}
			}
			if (wep == WEP64) {

				#ifdef WLAN_MBSSID
				if (0 != i) {
					memcpy(Entry.wep64Key1,key,WEP64_KEY_LEN);
				} else 		
				#endif
				ret=mib_set(MIB_WLAN_WEP64_KEY1, (void *)key);				
			}
			else {

				#ifdef WLAN_MBSSID
				if (0 != i) {
					memcpy(Entry.wep128Key1,key,WEP128_KEY_LEN);
				} else 		
				#endif
				ret=mib_set(MIB_WLAN_WEP128_KEY1, (void *)key);				
			}


			#ifdef WLAN_MBSSID
			if (0 != i) {				
			} else 					
			#endif
			if (!ret) {
	 			strcpy(tmpBuf, T(strSetWEPKey1Err));
				goto setErr_wep;
			}			
		}
	}

	wepKey = websGetVar(wp, T("key2"), T(""));	
	if  (wepKey[0]) {
		if (strlen(wepKey) != keyLen) {
			strcpy(tmpBuf, T(strInvdKey2Len));
			goto setErr_wep;
		}
		if ( !isAllStar(wepKey) ) {
			if (strFormat[0] == '1') { // ascii
				strcpy(key, wepKey);				
			}
			else { // hex
				if ( !string_to_hex(wepKey, key, keyLen)) {
	   				strcpy(tmpBuf, T(strInvdWEPKey2));
   					goto setErr_wep;
				}
			}
			if (wep == WEP64){

				#ifdef WLAN_MBSSID
				if (0 != i) {				
					memcpy(Entry.wep64Key2,key,WEP64_KEY_LEN);
				} else 					
				#endif
				ret=mib_set(MIB_WLAN_WEP64_KEY2, (void *)key);				
			}
			else {
				#ifdef WLAN_MBSSID
				if (0 != i) {				
					memcpy(Entry.wep128Key2,key,WEP128_KEY_LEN);
				} else 					
				#endif
				ret=mib_set(MIB_WLAN_WEP128_KEY2, (void *)key);				
			}

			#ifdef WLAN_MBSSID
			if (0 != i) {				
				
			} else 					
			#endif
			if (!ret) {
	 			strcpy(tmpBuf, T(strSetWEPKey2Err));
				goto setErr_wep;
			}			
		}
	}

	wepKey = websGetVar(wp, T("key3"), T(""));
	if  (wepKey[0]) {
		if (strlen(wepKey) != keyLen) {
			strcpy(tmpBuf, T(strInvdKey3Len));
			goto setErr_wep;
		}
		if ( !isAllStar(wepKey) ) {
			if (strFormat[0] == '1') // ascii
				strcpy(key, wepKey);
			else { // hex
				if ( !string_to_hex(wepKey, key, keyLen)) {
	   				strcpy(tmpBuf, T(strInvdWEPKey3));
   					goto setErr_wep;
				}
			}
			if (wep == WEP64) {
				#ifdef WLAN_MBSSID
				if (0 != i) {				
					memcpy(Entry.wep64Key3,key,WEP64_KEY_LEN);	
				} else 					
				#endif
				ret=mib_set(MIB_WLAN_WEP64_KEY3, (void *)key);				
			}
			else {				
				#ifdef WLAN_MBSSID
				if (0 != i) {				
					memcpy(Entry.wep128Key3,key,WEP128_KEY_LEN);
				} else 					
				#endif
				ret=mib_set(MIB_WLAN_WEP128_KEY3, (void *)key);				
			}

			#ifdef WLAN_MBSSID
			if (0 != i) {				
				
			} else 					
			#endif
			if (!ret) {
	 			strcpy(tmpBuf, T(strSetWEPKey3Err));
				goto setErr_wep;
			}			
		}
	}

	wepKey = websGetVar(wp, T("key4"), T(""));
	if  (wepKey[0]) {
		if (strlen(wepKey) != keyLen) {
			strcpy(tmpBuf, T(strInvdKey4Len));
			goto setErr_wep;
		}
		if ( !isAllStar(wepKey) ) {
			if (strFormat[0] == '1') // ascii
				strcpy(key, wepKey);
			else { // hex
				if ( !string_to_hex(wepKey, key, keyLen)) {
	   				strcpy(tmpBuf, T(strInvdWEPKey4));
   					goto setErr_wep;
				}
			}
			if (wep == WEP64) {				
				#ifdef WLAN_MBSSID
				if (0 != i) {				
					memcpy(Entry.wep64Key4,key,WEP64_KEY_LEN);	
				} else 					
				#endif
				ret=mib_set(MIB_WLAN_WEP64_KEY4, (void *)key);				
			}
			else {				
				#ifdef WLAN_MBSSID
				if (0 != i) {				
					memcpy(Entry.wep128Key4,key,WEP128_KEY_LEN);
				} else 					
				#endif
				ret=mib_set(MIB_WLAN_WEP128_KEY4, (void *)key);				
			}


			#ifdef WLAN_MBSSID
			if (0 != i) {				
				
			} else 					
			#endif
			if (!ret) {
	 			strcpy(tmpBuf, T(strSetWEPKey4Err));
				goto setErr_wep;
			}			
		}
	}	

#ifdef WLAN_MBSSID	
	mib_chain_update(MIB_MBSSIB_WEP_TBL, (void *)&Entry, i);
	
	sleep(5);
	if (!mib_chain_get(MIB_MBSSIB_WEP_TBL, i, (void *)&Entry)) {
  		
  		strcpy(tmpBuf, T(strGetMBSSIBWEPTLBUpdtErr));				
		goto setErr_wep;		
	}
	/*	
	printf("MIB_MBSSIB_WEP_TBL Updated\n");
	printf("Entry.wep64Key1=%s\n", Entry.wep64Key1);
	printf("Entry.wep64Key2=%s\n", Entry.wep64Key2);
	printf("Entry.wep64Key3=%s\n", Entry.wep64Key3);
	printf("Entry.wep64Key4=%s\n", Entry.wep64Key4);
	printf("Entry.wep128Key1=%s\n", Entry.wep128Key1);
	printf("Entry.wep128Key2=%s\n", Entry.wep128Key2);
	printf("Entry.wep128Key3=%s\n", Entry.wep128Key3);
	printf("Entry.wep128Key4=%s\n", Entry.wep128Key4);	
	printf("Entry.wepDefaultKey=%d\n", Entry.wepDefaultKey);
	printf("Entry.wepKeyType=%d\n", Entry.wepKeyType);
	*/
#endif
	
setwep_ret:
//	mib_update(CURRENT_SETTING);
	
#ifndef NO_ACTION
	run_script(-1);
#endif
#ifdef CONFIG_WIFI_SIMPLE_CONFIG // WPS
	fprintf(stderr, "WEP WPS Processing\n");

        sprintf(tmpBuf, "wps_clear_configure_by_reg%d", 0);
        wepKey = websGetVar(wp, tmpBuf, NULL);
        ret = 0;
        if (wepKey && wepKey[0])
                ret = atoi(wepKey);
        update_wps_configured(ret);

#endif

	config_WLAN(ACT_RESTART);

	submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page
	OK_MSG(submitUrl);

	return;

setErr_wep:
	ERR_MSG(tmpBuf);
}

#ifdef WLAN_WPA
/////////////////////////////////////////////////////////////////////////////
void formWlEncrypt(webs_t wp, char_t *path, char_t *query)
{
   	char_t *submitUrl, *strEncrypt, *strVal;
   	char_t vChar;
	char tmpBuf[100];
	ENCRYPT_T encrypt;
	int enableRS=0, intVal, getPSK=0, len;
	unsigned long reKeyTime;
	SUPP_NONWAP_T suppNonWPA;
	struct in_addr inIp;
#ifdef WLAN_MBSSID
	MIB_CE_MBSSIB_T Entry;
	int i;
#endif
			
#ifdef WLAN_MBSSID
	strVal = websGetVar(wp, T("wpaSSID"), T(""));

	if (strVal[0]) {
		if ( !gstrcmp(strVal, T("root"))) {
			i = 0;
		}
		else if ( !gstrcmp(strVal, T("vap0"))) {
			i = 1;
		}
		else if ( !gstrcmp(strVal, T("vap1"))) {
			i = 2;
		}
		else if ( !gstrcmp(strVal, T("vap2"))) {
			i = 3;
		}
		else if ( !gstrcmp(strVal, T("vap3"))) {
			i = 4;
		}
		else {	
			strcpy(tmpBuf, T(strNotSuptSSIDType));				
			goto setErr_encrypt;
		}
		
	} else {
		strcpy(tmpBuf, T(strNoSSIDTypeErr));				
		goto setErr_encrypt;
	}	
	
	if (!mib_chain_get(MIB_MBSSIB_TBL, i, (void *)&Entry)) {  		
  		strcpy(tmpBuf, T(strGetMBSSIBTBLErr));				
		goto setErr_encrypt;		
	}
	// Added by Mason Yu. End
	/*
	printf("Entry.idx=%d\n", Entry.idx);	
	printf("Entry.encrypt=%d\n", Entry.encrypt);
	printf("Entry.enable1X=%d\n", Entry.enable1X);
	printf("Entry.wep=%d\n", Entry.wep);
	printf("Entry.wpaAuth=%d\n", Entry.wpaAuth);
	printf("Entry.wpaPSKFormat=%d\n", Entry.wpaPSKFormat);
	printf("Entry.wpaPSK=%s\n", Entry.wpaPSK);
	printf("Entry.rsPort=%d\n", Entry.rsPort);
	printf("Entry.rsIpAddr=0x%x\n", *((unsigned long *)Entry.rsIpAddr));
	printf("Entry.rsPassword=%s\n", Entry.rsPassword);
	*/
#endif
	
   	strEncrypt = websGetVar(wp, T("method"), T(""));
	if (!strEncrypt[0]) {
 		strcpy(tmpBuf, T(strNoEncryptionErr));
		goto setErr_encrypt;
	}
	encrypt = (ENCRYPT_T) strEncrypt[0] - '0';
//xl_yue
#ifdef ENABLE_WPAAES_WPA2TKIP
	if(encrypt == ENCRY_WPA_TKIP|| encrypt == ENCRY_WPA_AES){
		if(encrypt == ENCRY_WPA_TKIP)
			vChar = WPA_CIPHER_TKIP;
		else 
			vChar = WPA_CIPHER_AES;
		
		encrypt = ENCRYPT_WPA;
		
#ifdef WLAN_MBSSID
		if (0 != i) {
			Entry.unicastCipher = vChar;
		} else 		
#endif
		if (mib_set( MIB_WLAN_WPA_CIPHER_SUITE, (void *)&vChar) == 0) {
  			strcpy(tmpBuf, T(strSetWLANENCRYPTErr));
			goto setErr_encrypt;
		}	
	}else if(encrypt == ENCRY_WPA2_AES|| encrypt == ENCRY_WPA2_TKIP){
		if(encrypt == ENCRY_WPA2_TKIP)
			vChar = WPA_CIPHER_TKIP;
		else 
			vChar = WPA_CIPHER_AES;
		encrypt = ENCRYPT_WPA2;

#ifdef WLAN_MBSSID
		if (0 != i) {
			Entry.wpa2UnicastCipher = vChar;
		} else 		
#endif
		if (mib_set( MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&vChar) == 0) {
  			strcpy(tmpBuf, T(strSetWLANENCRYPTErr));
			goto setErr_encrypt;
		}	
	}else if(encrypt == ENCRYPT_WPA2_MIXED){//mixed mode onley support wpa/tkip and wpa2/aes
#ifdef WLAN_MBSSID
		if (0 != i) {
			Entry.unicastCipher = WPA_CIPHER_TKIP;
			Entry.wpa2UnicastCipher = WPA_CIPHER_AES;
		} else 		
#endif
		{
			vChar = WPA_CIPHER_TKIP;
			if (mib_set( MIB_WLAN_WPA_CIPHER_SUITE, (void *)&vChar) == 0) {
  				strcpy(tmpBuf, T(strSetWLANENCRYPTErr));
				goto setErr_encrypt;
			}	

			vChar = WPA_CIPHER_AES;
			if (mib_set( MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&vChar) == 0) {
  				strcpy(tmpBuf, T(strSetWLANENCRYPTErr));
				goto setErr_encrypt;
			}
		}
	}
#endif
	if (encrypt!=ENCRYPT_DISABLED && encrypt!=ENCRYPT_WEP && encrypt!=ENCRYPT_WPA
		&& encrypt != ENCRYPT_WPA2 && encrypt != ENCRYPT_WPA2_MIXED){
		strcpy(tmpBuf, T(strInvdEncryptErr));
		goto setErr_encrypt;
	}
	vChar = (char_t)encrypt;

	#ifdef WLAN_MBSSID
	if (0 != i) {
		Entry.encrypt = vChar;
	} else 		
	#endif
	if (mib_set( MIB_WLAN_ENCRYPT, (void *)&vChar) == 0) {
  		strcpy(tmpBuf, T(strSetWLANENCRYPTErr));
		goto setErr_encrypt;
	}	
	
	if (encrypt == ENCRYPT_DISABLED || encrypt == ENCRYPT_WEP) {

#ifdef WLAN_1x
		strVal = websGetVar(wp, T("use1x"), T(""));
		if ( !gstrcmp(strVal, T("ON"))) {
			mib_get( MIB_WLAN_MODE, (void *)&vChar);
			if (vChar) { // not AP mode
				strcpy(tmpBuf, T(strSet8021xWarning));
				goto setErr_encrypt;
			}
			vChar = 1;
			enableRS = 1;
		}
		else
			vChar = 0;

		#ifdef WLAN_MBSSID
		if (0 != i) {
			Entry.enable1X = vChar;
		} else 		
		#endif
		if ( mib_set( MIB_WLAN_ENABLE_1X, (void *)&vChar) == 0) {
  			strcpy(tmpBuf, T(strSet1xEnabErr));
			goto setErr_encrypt;
		}
				
#endif

		if (encrypt == ENCRYPT_WEP) {
	 		WEP_T wep;

			#ifdef WLAN_MBSSID
			if (0 != i) {
				vChar = Entry.wep;
			} else 		
			#endif
			if ( !mib_get( MIB_WLAN_WEP,  (void *)&vChar) ) {
				strcpy(tmpBuf, T(strGetWLANWEPErr));
				goto setErr_encrypt;
			}			
			
			wep = (WEP_T)vChar;
			if (wep == WEP_DISABLED) {
				//wep = WEP64;
				vChar = (char_t)WEP64;

				#ifdef WLAN_MBSSID
				if (0 != i) {
					Entry.wep = vChar;
				} else 		
				#endif
				if ( mib_set( MIB_WLAN_WEP, (void *)&vChar) == 0) {
		  			strcpy(tmpBuf, T(strSetWLANWEPErr));
					goto setErr_encrypt;
				}				
			}
		}
#if 0
		else {
			strVal = websGetVar(wp, T("useMacAuth"), T(""));
			if ( !gstrcmp(strVal, T("ON"))) {
				vChar = 1;
				enableRS = 1;
			}
			else
				vChar = 0;
			if ( mib_set( MIB_WLAN_ENABLE_MAC_AUTH, (void *)&vChar) == 0) {
  				strcpy(tmpBuf, T("Set MIB_WLAN_ENABLE_MAC_AUTH MIB error!"));
				goto setErr_encrypt;
			}
		}
#endif
	}
	else {

#ifdef WLAN_1x
		// WPA authentication
		vChar = 0;

		#ifdef WLAN_MBSSID
		if (0 != i) {
			Entry.enable1X = vChar;
		} else 		
		#endif
		if ( mib_set( MIB_WLAN_ENABLE_1X, (void *)&vChar) == 0) {
  			strcpy(tmpBuf, T(strSet1xEnabErr));
			goto setErr_encrypt;
		}		
		
		strVal = websGetVar(wp, T("wpaAuth"), T(""));
		if (strVal[0]) {
			if ( !gstrcmp(strVal, T("eap"))) {
				mib_get( MIB_WLAN_MODE, (void *)&vChar);
				if (vChar) { // not AP mode
					strcpy(tmpBuf, T(strSetWPARADIUSWarn));
					goto setErr_encrypt;
				}
				vChar = WPA_AUTH_AUTO;
				enableRS = 1;
			}
			else if ( !gstrcmp(strVal, T("psk"))) {
				vChar = WPA_AUTH_PSK;
				getPSK = 1;
			}
			else {
				strcpy(tmpBuf, T(strInvdWPAAuthValue));
				goto setErr_encrypt;
			}

			#ifdef WLAN_MBSSID
			if (0 != i) {
				Entry.wpaAuth = vChar;
			} else 		
			#endif
			if ( mib_set(MIB_WLAN_WPA_AUTH, (void *)&vChar) == 0) {
				strcpy(tmpBuf, T(strSetAUTHTYPEErr));
				goto setErr_encrypt;
			}			
		}
#endif

#if 0
		// support nonWPA client
 		strVal = websGetVar(wp, T("nonWpaSupp"), T(""));
		if ( !gstrcmp(strVal, T("ON")))
			vChar = 1;
		else
			vChar = 0;
		if ( mib_set( MIB_WLAN_ENABLE_SUPP_NONWPA, (void *)&vChar) == 0) {
  			strcpy(tmpBuf, T("Set MIB_WLAN_ENABLE_SUPP_NONWPA mib error!"));
			goto setErr_encrypt;
		}
		if ( vChar ) {
			suppNonWPA = SUPP_NONWPA_NONE;
			strVal = websGetVar(wp, T("nonWpaWep"), T(""));
			if ( !gstrcmp(strVal, T("ON")))
				suppNonWPA |= SUPP_NONWPA_WEP;

			strVal = websGetVar(wp, T("nonWpa1x"), T(""));
			if ( !gstrcmp(strVal, T("ON"))) {
				suppNonWPA |= SUPP_NONWPA_1X;
				enableRS = 1;
			}
			
			vChar = (char_t)suppNonWPA;
			if ( mib_set( MIB_WLAN_SUPP_NONWPA, (void *)&vChar) == 0) {
  				strcpy(tmpBuf, T("Set MIB_WLAN_SUPP_NONWPA mib error!"));
				goto setErr_encrypt;
			}
		}

		// cipher suite		
		// sc_yang write the ciphersuite according to  encrypt for wpa
		// wpa mixed mode is not implemented yet.
		vChar = 0 ;
		if( (encrypt ==  ENCRYPT_WPA) || (encrypt == ENCRYPT_WPA2_MIXED) )
			vChar =   WPA_CIPHER_TKIP ;
		if ( mib_set(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&vChar) == 0) {
				strcpy(tmpBuf, T("Set MIB_WLAN_WPA_CIPHER_SUITE failed!"));
				goto setErr_encrypt;
		}
		//set wpa2UniCipher  for wpa2
		// wpa2 mixed mode is not implemented yet.
		vChar = 0 ;
		if( (encrypt ==  ENCRYPT_WPA2) || (encrypt == ENCRYPT_WPA2_MIXED) )
			vChar =   WPA_CIPHER_AES ;
		if ( mib_set(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&vChar) == 0) {
				strcpy(tmpBuf, T("Set MIB_WLAN_WPA2_UNICIPHER failed!"));
				goto setErr_encrypt;
		}
#endif

		// Kaohj, user pre-shared key for WPA
		//getPSK = 1;
		// pre-shared key
		if ( getPSK ) {
			char_t oldFormat;
			int oldPskLen;

   			strVal = websGetVar(wp, T("pskFormat"), T(""));
			if (!strVal[0]) {
	 			strcpy(tmpBuf, T(strNoPSKFormat));
				goto setErr_encrypt;
			}
			vChar = strVal[0] - '0';
			if (vChar != 0 && vChar != 1) {
	 			strcpy(tmpBuf, T(strInvdPSKFormat));
				goto setErr_encrypt;
			}

			// remember current psk format and length to compare to default case "****"
			#ifdef WLAN_MBSSID
			if (0 != i) {
				oldFormat = Entry.wpaPSKFormat;
				strcpy(tmpBuf, Entry.wpaPSK);
				oldPskLen = strlen(Entry.wpaPSK);
			} else 		
			#endif
			{
				mib_get(MIB_WLAN_WPA_PSK_FORMAT, (void *)&oldFormat);
				mib_get(MIB_WLAN_WPA_PSK, (void *)tmpBuf);
				oldPskLen = strlen(tmpBuf);
			}			

			strVal = websGetVar(wp, T("pskValue"), T(""));
			len = strlen(strVal);			
//xl_yue: "*******" is a valid key for 531b, and key is display on webpage
#if !(defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC))
			{
				int tmp;
				printf("len= %d oldPskLen= %d oldFormat=%d vChar=%d\n", len, oldPskLen, oldFormat, vChar);
				if (oldFormat == vChar && len == oldPskLen ) {
					for (tmp=0; tmp<len; tmp++) {
						if ( strVal[tmp] != '*' )
							break;
					}
					if (tmp == len)
						goto rekey_time;
					}
			}
#endif		
			#ifdef WLAN_MBSSID
			if (0 != i) {
				Entry.wpaPSKFormat = vChar;
			} else 		
			#endif
			if ( mib_set(MIB_WLAN_WPA_PSK_FORMAT, (void *)&vChar) == 0) {
				strcpy(tmpBuf, T(strSetWPAPSKFMATErr));
				goto setErr_encrypt;
			}			

			if (vChar==1) { // hex
				if (len!=MAX_PSK_LEN || !string_to_hex(strVal, tmpBuf, MAX_PSK_LEN)) {
	 				strcpy(tmpBuf, T(strInvdPSKValue));
					goto setErr_encrypt;
				}
			}
			else { // passphras
				if (len==0 || len > (MAX_PSK_LEN) ) {
	 				strcpy(tmpBuf, T(strInvdPSKValue));
					goto setErr_encrypt;
				}
			}

			#ifdef WLAN_MBSSID
			if (0 != i) {
				strcpy(Entry.wpaPSK, strVal);
			} else 		
			#endif
			{
				if ( !mib_set(MIB_WLAN_WPA_PSK, (void *)strVal)) {
					strcpy(tmpBuf, T(strSetWPAPSKErr));
					goto setErr_encrypt;
				}
			}			
		}
rekey_time:
		// group key rekey time
		reKeyTime = 0;
		strVal = websGetVar(wp, T("groupKeyTimeDay"), T(""));
		if (strVal[0]) {
			if ( !string_to_dec(strVal, &intVal) ) {
				strcpy(tmpBuf, T(strInvdRekeyDay));
				goto setErr_encrypt;
			}
			reKeyTime += intVal*86400;
		}
		strVal = websGetVar(wp, T("groupKeyTimeHr"), T(""));
		if (strVal[0]) {
			if ( !string_to_dec(strVal, &intVal) ) {
				strcpy(tmpBuf, T(strInvdRekeyHr));
				goto setErr_encrypt;
			}
			reKeyTime += intVal*3600;
		}
		strVal = websGetVar(wp, T("groupKeyTimeMin"), T(""));
		if (strVal[0]) {
			if ( !string_to_dec(strVal, &intVal) ) {
				strcpy(tmpBuf, T(strInvdRekeyMin));
				goto setErr_encrypt;
			}
			reKeyTime += intVal*60;
		}

		strVal = websGetVar(wp, T("groupKeyTimeSec"), T(""));
		if (strVal[0]) {
			if ( !string_to_dec(strVal, &intVal) ) {
				strcpy(tmpBuf, T(strInvdRekeySec));
				goto setErr_encrypt;
			}
			reKeyTime += intVal;
		}
		if (reKeyTime) {
			if ( !mib_set(MIB_WLAN_WPA_GROUP_REKEY_TIME, (void *)&reKeyTime)) {
				strcpy(tmpBuf, T(strSetREKEYTIMEErr));
				goto setErr_encrypt;
			}
		}
	}

#ifdef WLAN_1x
	if (enableRS == 1) { // if 1x enabled, get RADIUS server info
		unsigned short uShort;
		
		strVal = websGetVar(wp, T("radiusPort"), T(""));
		if (!strVal[0]) {
			strcpy(tmpBuf, T("No RS port number!"));
			goto setErr_encrypt;
		}
		if (!string_to_dec(strVal, &intVal) || intVal<=0 || intVal>65535) {
			strcpy(tmpBuf, T(strInvdRSPortNum));
			goto setErr_encrypt;
		}
		uShort = (unsigned short)intVal;

		#ifdef WLAN_MBSSID
		if (0 != i) {
			Entry.rsPort = uShort;
		} else 		
		#endif
		if ( !mib_set(MIB_WLAN_RS_PORT, (void *)&uShort)) {
			strcpy(tmpBuf, T(strSetRSPortErr));
			goto setErr_encrypt;
		}		
		
		strVal = websGetVar(wp, T("radiusIP"), T(""));
		if (!strVal[0]) {
			strcpy(tmpBuf, T(strNoIPAddr));
			goto setErr_encrypt;
		}
		if ( !inet_aton(strVal, &inIp) ) {
			strcpy(tmpBuf, T(strInvdRSIPValue));
			goto setErr_encrypt;
		}

		#ifdef WLAN_MBSSID
		if (0 != i) {
			*((unsigned long *)Entry.rsIpAddr) = inIp.s_addr;
		} else 		
		#endif
		if ( !mib_set(MIB_WLAN_RS_IP, (void *)&inIp)) {
			strcpy(tmpBuf, T(strSetIPAddrErr));
			goto setErr_encrypt;
		}		
		
		strVal = websGetVar(wp, T("radiusPass"), T(""));
		if (strlen(strVal) > (MAX_PSK_LEN) ) {
			strcpy(tmpBuf, T(strRSPwdTooLong));
			goto setErr_encrypt;
		}

		#ifdef WLAN_MBSSID
		if (0 != i) {
			strcpy(Entry.rsPassword, strVal);
		} else 		
		#endif
		if ( !mib_set(MIB_WLAN_RS_PASSWORD, (void *)strVal)) {
			strcpy(tmpBuf, T(strSetRSPwdErr));
			goto setErr_encrypt;
		}		

		strVal = websGetVar(wp, T("radiusRetry"), T(""));
		if (strVal[0]) {
			if ( !string_to_dec(strVal, &intVal) ) {
				strcpy(tmpBuf, T(strInvdRSRetry));
				goto setErr_encrypt;
			}
			vChar = (char_t)intVal;
			if ( !mib_set(MIB_WLAN_RS_RETRY, (void *)&vChar)) {
				strcpy(tmpBuf, T(strSetRSRETRYErr));
				goto setErr_encrypt;
			}
		}
		strVal = websGetVar(wp, T("radiusTime"), T(""));
		if (strVal[0]) {
			if ( !string_to_dec(strVal, &intVal) ) {
				strcpy(tmpBuf, T(strInvdRSTime));
				goto setErr_encrypt;
			}
			uShort = (unsigned short)intVal;
			if ( !mib_set(MIB_WLAN_RS_INTERVAL_TIME, (void *)&uShort)) {
				strcpy(tmpBuf, T(strSetRSINTVLTIMEErr));
				goto setErr_encrypt;
			}
		}
#if 0
		strVal = websGetVar(wp, T("useAccount"), T(""));
		if ( !gstrcmp(strVal, T("ON")))
			intVal = 1;
		else
			intVal = 0;
		if ( mib_set( MIB_WLAN_ACCOUNT_RS_ENABLED, (void *)&intVal) == 0) {
  			strcpy(tmpBuf, T("Set MIB_WLAN_ACCOUNT_RS_ENABLED mib error!"));
			goto setErr_encrypt;
		}
		if (intVal == 0)
			goto get_wepkey;

		strVal = websGetVar(wp, T("accountPort"), T(""));
		if (!strVal[0]) {
			strcpy(tmpBuf, T("No account RS port number!"));
			goto setErr_encrypt;
		}
		if (!string_to_dec(strVal, &intVal) || intVal<=0 || intVal>65535) {
			strcpy(tmpBuf, T("Error! Invalid value of account RS port number."));
			goto setErr_encrypt;
		}
		if ( !mib_set(MIB_WLAN_ACCOUNT_RS_PORT, (void *)&intVal)) {
			strcpy(tmpBuf, T("Set account RS port error!"));
			goto setErr_encrypt;
		}
		strVal = websGetVar(wp, T("accountIP"), T(""));
		if (!strVal[0]) {
			strcpy(tmpBuf, T("No account RS IP address!"));
			goto setErr_encrypt;
		}
		if ( !inet_aton(strVal, &inIp) ) {
			strcpy(tmpBuf, T("Invalid account RS IP-address value!"));
			goto setErr_encrypt;
		}
		if ( !mib_set(MIB_WLAN_ACCOUNT_RS_IP, (void *)&inIp)) {
			strcpy(tmpBuf, T("Set account RS IP-address error!"));
			goto setErr_encrypt;
		}
		strVal = websGetVar(wp, T("accountPass"), T(""));
		if (strlen(strVal) > (MAX_RS_PASS_LEN -1) ) {
			strcpy(tmpBuf, T("Account RS password length too long!"));
			goto setErr_encrypt;
		}
		if ( !mib_set(MIB_WLAN_ACCOUNT_RS_PASSWORD, (void *)strVal)) {
			strcpy(tmpBuf, T("Set account RS password error!"));
			goto setErr_encrypt;
		}
		strVal = websGetVar(wp, T("accountRetry"), T(""));
		if (strVal[0]) {
			if ( !string_to_dec(strVal, &intVal) ) {
				strcpy(tmpBuf, T("Invalid account RS retry value!"));
				goto setErr_encrypt;
			}
			if ( !mib_set(MIB_WLAN_ACCOUNT_RS_RETRY, (void *)&intVal)) {
				strcpy(tmpBuf, T("Set MIB_WLAN_ACCOUNT_RS_RETRY error!"));
				goto setErr_encrypt;
			}
		}
		strVal = websGetVar(wp, T("accountTime"), T(""));
		if (strVal[0]) {
			if ( !string_to_dec(strVal, &intVal) ) {
				strcpy(tmpBuf, T("Invalid account RS time value!"));
				goto setErr_encrypt;
			}
			if ( !mib_set(MIB_WLAN_ACCOUNT_RS_INTERVAL_TIME, (void *)&intVal)) {
				strcpy(tmpBuf, T("Set MIB_WLAN_ACCOUNT_RS_INTERVAL_TIME error!"));
				goto setErr_encrypt;
			}
		}
		strVal = websGetVar(wp, T("accountUpdateEnabled"), T(""));
		if ( !gstrcmp(strVal, T("ON")))
			intVal = 1;
		else
			intVal = 0;
		if ( mib_set( MIB_WLAN_ACCOUNT_UPDATE_ENABLED, (void *)&intVal) == 0) {
			strcpy(tmpBuf, T("Set MIB_WLAN_ACCOUNT_UPDATE_ENABLED mib error!"));
			goto setErr_encrypt;
		}
		strVal = websGetVar(wp, T("accountUpdateTime"), T(""));
		if (strVal[0]) {
			if ( !string_to_dec(strVal, &intVal) ) {
				strcpy(tmpBuf, T("Error! Invalid value of update time"));
				goto setErr_encrypt;
			}
			if ( !mib_set(MIB_WLAN_ACCOUNT_UPDATE_DELAY, (void *)&intVal)) {
				strcpy(tmpBuf, T("Set MIB_WLAN_ACCOUNT_UPDATE_DELAY mib error!"));
				goto setErr_encrypt;
			}
		}
#endif

get_wepkey:
		// get 802.1x WEP key length
		strVal = websGetVar(wp, T("wepKeyLen"), T(""));
		if (strVal[0]) {
			if ( !gstrcmp(strVal, T("wep64")))
				vChar = WEP64;
			else if ( !gstrcmp(strVal, T("wep128")))
				vChar = WEP128;
			else {
				strcpy(tmpBuf, T(strInvdWepKeyLen));
				goto setErr_encrypt;
			}

			#ifdef WLAN_MBSSID
			if (0 != i) {
				Entry.wep = vChar;
			} else 		
			#endif
			if ( mib_set(MIB_WLAN_WEP, (void *)&vChar) == 0) {
				strcpy(tmpBuf, T(strSetMIBWLANWEPErr));
				goto setErr_encrypt;
			}			
		}
	}
#endif

	// Added by Mason Yu
#ifdef WLAN_MBSSID
	mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, i);
	
	sleep(5);
	if (!mib_chain_get(MIB_MBSSIB_TBL, i, (void *)&Entry)) {
  		
  		strcpy(tmpBuf, T(strGetMBSSIBTBLUpdtErr));				
		goto setErr_encrypt;		
	}
	/*
	printf("MIB_MBSSIB_TBL updated\n");
	printf("Entry.idx=%d\n", Entry.idx);
	printf("Entry.encrypt=%d\n", Entry.encrypt);
	printf("Entry.enable1X=%d\n", Entry.enable1X);
	printf("Entry.wep=%d\n", Entry.wep);
	printf("Entry.wpaAuth=%d\n", Entry.wpaAuth);
	printf("Entry.wpaPSKFormat=%d\n", Entry.wpaPSKFormat);
	printf("Entry.wpaPSK=%s\n", Entry.wpaPSK);
	printf("Entry.rsPort=%d\n", Entry.rsPort);
	printf("Entry.rsIpAddr=0x%x\n", *((unsigned long *)Entry.rsIpAddr));
	printf("Entry.rsPassword=%s\n", Entry.rsPassword);
	*/
#endif
	
//	mib_update(CURRENT_SETTING);
set_OK:
	
#ifndef NO_ACTION
	run_script(-1);
#endif
#ifdef CONFIG_WIFI_SIMPLE_CONFIG //WPS
	//fprintf(stderr, "WPA WPS Configure\n");
	sprintf(tmpBuf, "wps_clear_configure_by_reg%d", 0);
        strVal = websGetVar(wp, tmpBuf, NULL);
        intVal = 0;
        if (strVal && strVal[0])
                intVal = atoi(strVal);
        update_wps_configured(intVal);
#endif

	config_WLAN(ACT_RESTART);

	submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page
	OK_MSG(submitUrl);

	return;

setErr_encrypt:
	ERR_MSG(tmpBuf);
}
#endif // WLAN_WPA
#ifdef WLAN_ACL

int wlShowAcList(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0, entryNum, i;
	MIB_CE_WLAN_AC_T Entry;
	char tmpBuf[100];
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	entryNum = mib_chain_total(MIB_WLAN_AC_TBL);	
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_WLAN_AC_TBL, i, (void *)&Entry))
		{
  			websError(wp, 400, T("Get chain record error!\n"));
			return -1;
		}
		nBytesSent += websWrite(wp, T("<tr>"
      	"<td align=left width=\"40%%\"  bgcolor=\"#808080\"><font size=\"2\"><b>MAC 地址</b></font></td>\n"));
		snprintf(tmpBuf, 100, T("%02x-%02x-%02x-%02x-%02x-%02x"),
			Entry.macAddr[0], Entry.macAddr[1], Entry.macAddr[2],
			Entry.macAddr[3], Entry.macAddr[4], Entry.macAddr[5]);

		nBytesSent += websWrite(wp, T("<td align=left width=\"60%%\"  bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td></tr>\n"),tmpBuf);
	}
#endif
	return nBytesSent;
}

#endif

#ifdef WLAN_ACL
/////////////////////////////////////////////////////////////////////////////
int wlAcList(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0, entryNum, i;
	MIB_CE_WLAN_AC_T Entry;
	char tmpBuf[100];
	
	entryNum = mib_chain_total(MIB_WLAN_AC_TBL);

	nBytesSent += websWrite(wp, T("<tr>"
      	"<td align=center width=\"45%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td></tr>\n"),strMACAddr,strSelect);
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_WLAN_AC_TBL, i, (void *)&Entry))
		{
  			websError(wp, 400, T("Get chain record error!\n"));
			return -1;
		}
#if defined(ZTE_531B_BRIDGE_SC)||defined(ZTE_GENERAL_ROUTER_SC)	
snprintf(tmpBuf, 100, T("%02x-%02x-%02x-%02x-%02x-%02x"),
			Entry.macAddr[0], Entry.macAddr[1], Entry.macAddr[2],
			Entry.macAddr[3], Entry.macAddr[4], Entry.macAddr[5]);
#else
		snprintf(tmpBuf, 100, T("%02x:%02x:%02x:%02x:%02x:%02x"),
			Entry.macAddr[0], Entry.macAddr[1], Entry.macAddr[2],
			Entry.macAddr[3], Entry.macAddr[4], Entry.macAddr[5]);
#endif

		nBytesSent += websWrite(wp, T("<tr>"
			"<td align=center width=\"45%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
       			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n"),
				tmpBuf, i);
	}
	return nBytesSent;
}

/////////////////////////////////////////////////////////////////////////////
void formWlAc(webs_t wp, char_t *path, char_t *query)
{
	char_t *strAddMac, *strDelMac, *strDelAllMac, *strVal, *submitUrl, *strEnabled;
	char tmpBuf[100];
	char_t vChar;
	int entryNum, i, enabled;
	MIB_CE_WLAN_AC_T macEntry;
	MIB_CE_WLAN_AC_T Entry;
//xl_yue
//#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	char_t * strSetMode;
	strSetMode = websGetVar(wp, T("setFilterMode"), T(""));
//#endif
	strAddMac = websGetVar(wp, T("addFilterMac"), T(""));
	strDelMac = websGetVar(wp, T("deleteSelFilterMac"), T(""));
	strDelAllMac = websGetVar(wp, T("deleteAllFilterMac"), T(""));
	strEnabled = websGetVar(wp, T("wlanAcEnabled"), T(""));

//xl_yue: access control mode is set independently from adding MAC for 531b
//#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	if (strSetMode[0]) {
		vChar = strEnabled[0] - '0';

		if ( mib_set( MIB_WLAN_AC_ENABLED, (void *)&vChar) == 0) {
  			strcpy(tmpBuf, T(strEnabAccCtlErr));
			goto setErr_ac;
		}
		goto setac_ret;
	}
//#endif

//added by xl_yue: acltbl can not be modified in disabled status
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	mib_get( MIB_WLAN_AC_ENABLED, (void *)&vChar);
	if (!vChar) {
		strcpy(tmpBuf, T(strModifyAcltblErr));
		goto setErr_ac;
	}
#endif

	if (strAddMac[0]) {
		int intVal;
		/*
		if ( !gstrcmp(strEnabled, T("ON")))
			vChar = 1;
		else
			vChar = 0;
		*/
//xl_yue
#if 0
//#if !(defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC))
		vChar = strEnabled[0] - '0';

		if ( mib_set( MIB_WLAN_AC_ENABLED, (void *)&vChar) == 0) {
  			strcpy(tmpBuf, T(strEnabAccCtlErr));
			goto setErr_ac;
		}
#endif
		strVal = websGetVar(wp, T("mac"), T(""));
		if ( !strVal[0] ) {
//			strcpy(tmpBuf, T("Error! No mac address to set."));
			goto setac_ret;
		}

//added by xl_yue: kick of "-" in string
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		for(i=0;i<17;i++){
			if((i+1)%3 != 0)
				strVal[i-(i+1)/3] = strVal[i];
		}
		strVal[12] = '\0';
#endif

		if (strlen(strVal)!=12 || !string_to_hex(strVal, macEntry.macAddr, 12)) {
			strcpy(tmpBuf, T(strInvdMACAddr));
			goto setErr_ac;
		}
		if (!isValidMacAddr(macEntry.macAddr)) {
			strcpy(tmpBuf, T(strInvdMACAddr));
			goto setErr_ac;
		}

/*
		strVal = websGetVar(wp, T("comment"), T(""));
		if ( strVal[0] ) {
			if (strlen(strVal) > COMMENT_LEN-1) {
				strcpy(tmpBuf, T("Error! Comment length too long."));
				goto setErr_ac;
			}
			strcpy(macEntry.comment, strVal);
		}
		else
			macEntry.comment[0] = '\0';
*/

		entryNum = mib_chain_total(MIB_WLAN_AC_TBL);
		if ( entryNum >= MAX_WLAN_AC_NUM ) {
			strcpy(tmpBuf, T(strAddAcErrForFull));
			goto setErr_ac;
		}

		// set to MIB. Check if entry exists
		for (i=0; i<entryNum; i++) {
			if (!mib_chain_get(MIB_WLAN_AC_TBL, i, (void *)&Entry))
			{
	  			websError(wp, 400, T("Get chain record error!\n"));
				return;
			}
			
			if (!memcmp(macEntry.macAddr, Entry.macAddr, 6))
			{
				strcpy(tmpBuf, T(strMACInList));
				goto setErr_ac;
			}
		}
		
		intVal = mib_chain_add(MIB_WLAN_AC_TBL, (unsigned char*)&macEntry);
		if (intVal == 0) {
			strcpy(tmpBuf, T(strAddListErr));
			goto setErr_ac;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, T(strTableFull));
			goto setErr_ac;
		}
	}

	/* Delete entry */
	if (strDelMac[0]) {
		unsigned int deleted = 0;
		entryNum = mib_chain_total(MIB_WLAN_AC_TBL);
		for (i=entryNum; i>0; i--) {
			snprintf(tmpBuf, 20, "select%d", i-1);
			strVal = websGetVar(wp, tmpBuf, T(""));
			
			if ( !gstrcmp(strVal, T("ON")) ) {

				deleted ++;
				if(mib_chain_delete(MIB_WLAN_AC_TBL, i-1) != 1) {
					strcpy(tmpBuf, T(strDelListErr));
					goto setErr_ac;
				}
			}
		}
		if (deleted <= 0) {
			strcpy(tmpBuf, T("There is no item selected to delete!"));
			goto setErr_ac;
		}
	}

	/* Delete all entry */
	if ( strDelAllMac[0]) {
		mib_chain_clear(MIB_WLAN_AC_TBL); /* clear chain record */
	}

setac_ret:
//#ifdef APPLY_CHANGE
	config_WLAN(ACT_RESTART);
//#endif
//	mib_update(CURRENT_SETTING);

#ifndef NO_ACTION
	run_script(-1);
#endif

	submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page
	OK_MSG( submitUrl );
  	return;

setErr_ac:
	ERR_MSG(tmpBuf);
}
#endif

/////////////////////////////////////////////////////////////////////////////
//check wlan status and set web menu
// parm: 1 -> javascript support
//	 0 -> no js
//xl_yue: translocate to fmmenucreate.c or fmmenucreate_user.c
#if 0
int wlanMenu(int eid, webs_t wp, int argc, char_t **argv)
{
#ifdef WLAN_SUPPORT
	char *parm;
	
	if (ejArgs(argc, argv, T("%s"), &parm) < 1) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}
	
	//if (wlan_is_up()) {
		if ( parm[0] == '1' ) {
			websWrite(wp, T("menu.addItem(\"Wireless\");"
				"wlan = new MTMenu();"
				"wlan.addItem(\"Basic Settings\", \"/admin/wlbasic.asp\", \"\", \"Setup wireless basic configuration\");"
				"wlan.addItem(\"Advanced Settings\", \"/admin/wladvanced.asp\", \"\", \"Setup wireless advanced configuration\");"
#ifdef WLAN_MBSSID
				"wlan.addItem(\"Security\", \"/admin/wlwpa_mbssid.asp\", \"\", \"Setup wireless security\");"));				
#else
				"wlan.addItem(\"Security\", \"/admin/wlwpa.asp\", \"\", \"Setup wireless security\");"));			
#endif

#ifdef WLAN_ACL
			websWrite(wp, T("wlan.addItem(\"Access Control\", \"/admin/wlactrl.asp\", \"\", \"Setup access control list for wireless clients\");"));
#endif
#ifdef WLAN_WDS
			websWrite(wp, T("wlan.addItem(\"WDS\", \"/admin/wlwds.asp\", \"\", \"WDS Settings\");"));
#endif
#ifdef WLAN_CLIENT
			websWrite(wp, T("wlan.addItem(\"Site Survey\", \"/admin/wlsurvey.asp\", \"\", \"Wireless Site Survey\");"));
#endif

#ifdef CONFIG_WIFI_SIMPLE_CONFIG//WPS def WLAN_CLIENT
			websWrite(wp, T("wlan.addItem(\"WPS\", \"/admin/wlwps.asp\", \"\", \"Wireless Protected Setup\");"));
#endif

#ifdef WLAN_MBSSID
			websWrite(wp, T("wlan.addItem(\"MBSSID\", \"/admin/wlmbssid.asp\", \"\", \"Wireless Multiple BSSID Setting\");"));
#endif

			websWrite(wp, T("menu.makeLastSubmenu(wlan);"));
		}
		else {

			websWrite(wp, T("<tr><td><b>Wireless</b></td></tr>\n"
				"<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlbasic.asp\" target=\"view\">Basic Settings</a></td></tr>\n"
				"<tr><td>&nbsp;&nbsp;<a href=\"/admin/wladvanced.asp\" target=\"view\">Advanced Settings</a></td></tr>"
#ifdef WLAN_MBSSID
				"<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlwpa_mbssid.asp\" target=\"view\">Security</a></td></tr>\n"));				
#else
				"<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlwpa.asp\" target=\"view\">Security</a></td></tr>\n"));		
#endif

#ifdef WLAN_ACL
			websWrite(wp, T("<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlactrl.asp\" target=\"view\">Access Control</a></td></tr>\n"));
#endif
#ifdef WLAN_WDS
			websWrite(wp, T("<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlwds.asp\" target=\"view\">WDS Settings</a></td></tr>\n"));
#endif
#ifdef WLAN_CLIENT
			websWrite(wp, T("<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlsurvey.asp\" target=\"view\">Wireless Site Survey</a></td></tr>\n"));
#endif

#ifdef CONFIG_WIFI_SIMPLE_CONFIG // WPS
			websWrite(wp, T("<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlwps.asp\" target=\"view\">Wireless Protected Setup</a></td></tr>\n"));
#endif
#ifdef WLAN_MBSSID
			websWrite(wp, T("<tr><td>&nbsp;&nbsp;<a href=\"/admin/wlmbssid.asp\" target=\"view\">Wireless Multiple BSSID Setting</a></td></tr>\n"));
#endif
		}
	//};
#endif
	return 0;
}
#endif

// Added by Mason Yu
#ifdef WLAN_MBSSID
int wlmbssid_asp(int eid, webs_t wp, int argc, char_t **argv)
{
	char	*string_0[] = {	"Vap0", "Vap1", "Vap2", "Vap3" };
    	char	*string_1[] = { "En_vap0", "En_vap1", "En_vap2", "En_vap3"}; //En_vap(WlanCardIdx)(VAPIdx)
    	char	*string_2[] = { "ssid_v0", "ssid_v1", "ssid_v2", "ssid_v3"};     //ssid_v(WlanCardIdx)(VAPIdx)
    	int	cntLoop, cntwlancard;
    	//uint8	totalWlanCards;

    	//totalWlanCards = pRomeCfgParam->wlaninterCfgParam.totalWlanCards;
    
	websWrite(wp,
			"<form method=\"get\" action=\"/goform/asp_setWlanMBS\" name=userform>\n"\			
			"<BR>\n"
	);

	//for (cntwlancard=0; cntwlancard<totalWlanCards; cntwlancard++)
	{
			
			websWrite(wp,
				"<table cellSpacing=1 cellPadding=2 border=1>\n"\
				"<tr><td bgColor=bbccff>Wireless Card </td></tr></table>\n"\				
				"<table cellSpacing=1 cellPadding=2 border=0>\n"\
				"<tr>\n"
				);
			
			
			for(cntLoop=0; cntLoop<MAX_WLAN_VAP; cntLoop++)
			{
				websWrite(wp,
					"<tr>\n"\
					"<td bgColor=#ddeeff>%s</td>\n",
					string_0[cntLoop]
				);
				/*
				if (pRomeCfgParam->wlanCfgParam[cntwlancard].enable)
				{
					websWrite(wp,
						"<td bgColor=#ddeeff><input type=checkbox name=%s value=1 %s onClick=\"onload_func();\">Enable</td>\n",
						string_1[cntwlancard*4+cntLoop], pRomeCfgParam->wlanCfgParam[cntwlancard].enable_vap[cntLoop]?"checked":""
					);
				}
				else
				{
					websWrite(wp,
						"<td bgColor=#ddeeff><input type=checkbox name=%s disabled value=1 %s onClick=\"onload_func();\">Enable</td>\n",
						string_1[cntwlancard*4+cntLoop], pRomeCfgParam->wlanCfgParam[cntwlancard].enable_vap[cntLoop]?"checked":""
					);
				}
				*/
				websWrite(wp,
						"<td bgColor=#ddeeff><input type=checkbox name=%s value=1 %s onClick=\"onload_func();\">Enable</td>\n",
						string_1[cntLoop], "checked"
					);

				websWrite(wp,
					"<td bgColor=#aaddff>SSID</td>\n"\
					"<td bgColor=#ddeeff><input type=text name=%s size=16 maxlength=16 value=%s></td>\n"\
					"</tr>\n",
					string_2[cntLoop], "CTC-1q2w"
		            );
			}		
	}

	websWrite(wp,
			"<tr>\n"\
			"<td colspan=2 align=center>\n"\
			"<input type=submit value=Apply>\n"\
	        	"<input type=reset value=Reset>\n"\
			"</td>\n"\
			"</tr>\n"\
			"</table> </form>\n"
	);
}

void formWlanMBSSID(webs_t wp, char_t *path, char_t *query)
{	
	char_t	*str, *submitUrl;
	MIB_CE_MBSSIB_T Entry;
	char tmpBuf[100], en_vap[10];
	int i;
	AUTH_TYPE_T authType;
#ifndef NO_ACTION
	int pid;
#endif
	
	for (i=0; i<4; i++) {
		sprintf(en_vap, "En_vap%d", i);
		str = websGetVar(wp, T(en_vap), T(""));
		if ( str && str[0] ) {	// enable		
			if (!mib_chain_get(MIB_MBSSIB_TBL, i+1, (void *)&Entry)) {  		
  				strcpy(tmpBuf, T(strGetMBSSIBTBLErr));				
				goto setErr_mbssid;		
			}
			
			Entry.wlanDisabled = 0;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, i+1);		
		} else {		// disable	
			if (!mib_chain_get(MIB_MBSSIB_TBL, i+1, (void *)&Entry)) {  		
  				strcpy(tmpBuf, T(strGetMBSSIBTBLErr));				
				goto setErr_mbssid;		
			}
			
			Entry.wlanDisabled = 1;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, i+1);		
		}
	}
	
	for (i=0; i<4; i++) {
		sprintf(en_vap, "ssid_v%d", i);
		str = websGetVar(wp, T(en_vap), T(""));
		if ( str ) {			
			if (!mib_chain_get(MIB_MBSSIB_TBL, i+1, (void *)&Entry)) {  		
  				strcpy(tmpBuf, T(strGetVAPMBSSIBTBLErr));				
				goto setErr_mbssid;		
			}			
			
			strcpy(Entry.ssid, str);			
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, i+1);		
		} 
	}
	
	for (i=0; i<4; i++) {
		sprintf(en_vap, "authType%d", i);	
		str = websGetVar(wp, T(en_vap), T(""));
		if (str[0]) {
			if (!mib_chain_get(MIB_MBSSIB_TBL, i+1, (void *)&Entry)) {  		
  				strcpy(tmpBuf, T(strGetVAPMBSSIBTBLErr));				
				goto setErr_mbssid;		
			}
			
			if ( !gstrcmp(str, T("open")))
				authType = AUTH_OPEN;
			else if ( !gstrcmp(str, T("shared")))
				authType = AUTH_SHARED;
			else if ( !gstrcmp(str, T("both")))
				authType = AUTH_BOTH;
			else {
				strcpy(tmpBuf, T(strInvdAuthType));
				goto setErr_mbssid;
			}
			
			Entry.authType = authType;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, i+1);		
		}
	}	

	config_WLAN(ACT_RESTART);
#ifndef NO_ACTION
	pid = fork();
        if (pid)
                waitpid(pid, NULL, 0);
        else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _CONFIG_SCRIPT_PROG);
#ifdef HOME_GATEWAY
		execl( tmpBuf, _CONFIG_SCRIPT_PROG, "gw", "bridge", NULL);
#else
		execl( tmpBuf, _CONFIG_SCRIPT_PROG, "ap", "bridge", NULL);
#endif
                exit(1);
        }
#endif

	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	OK_MSG(submitUrl);
  	return;

setErr_mbssid:
	ERR_MSG(tmpBuf);
}


// Added by Mason Yu
int postSSID(int eid, webs_t wp, int argc, char_t **argv)
{	
	char_t *name;
	MIB_CE_MBSSIB_T Entry;
	char strbuf[MAX_PSK_LEN+1], strbuf2[20];
	int len;
	int i;
	unsigned char vChar;
	
	if (ejArgs(argc, argv, T("%s"), &name) < 1) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}	
	
	if ( !strcmp(name, T("root")) ){		
		i = 0;		
		
	}
	else if ( !strcmp(name, T("vap0")) ) {		
		i = 1;		
	}
	else if ( !strcmp(name, T("vap1")) ) {		
		i = 2;		
	}
	else if ( !strcmp(name, T("vap2")) ) {		
		i = 3;		
	}
	else if ( !strcmp(name, T("vap3")) ) {		
		i = 4;		
	}
	else {
		printf("Not support this SSID!\n");
		return 1;
	}

	//fprintf(stderr, "******* 0 *******\n");
	if (0==i) {
		//fprintf(stderr, "******* 1 *******\n");
		MBSSID_GetRootEntry(&Entry);
	} else {
		if (!mib_chain_get(MIB_MBSSIB_TBL, i, (void *)&Entry)) {  		
	  		printf("Error! Get MIB_MBSSIB_TBL(Root) error.\n");
  			return 1;					
		}
	}
	
	// wpaPSK
	for (len=0; len<strlen(Entry.wpaPSK); len++)
		strbuf[len]='*';
	strbuf[len]='\0';
	
	// RsIp
	if ( ((struct in_addr *)Entry.rsIpAddr)->s_addr == INADDR_NONE ) {
		sprintf(strbuf2, "%s", "");				
	} else {	
		sprintf(strbuf2, "%s", inet_ntoa(*((struct in_addr *)Entry.rsIpAddr)));
	}

	vChar = Entry.encrypt;
#ifdef ENABLE_WPAAES_WPA2TKIP
	if(Entry.encrypt == ENCRYPT_WPA){
		if(Entry.unicastCipher == WPA_CIPHER_TKIP)
			vChar = ENCRY_WPA_TKIP;
		else
			vChar = ENCRY_WPA_AES;
	}else if(Entry.encrypt == ENCRYPT_WPA2){
		if(Entry.wpa2UnicastCipher == WPA_CIPHER_TKIP)
			vChar = ENCRY_WPA2_TKIP;
		else
			vChar = ENCRY_WPA2_AES;
	}
#endif

	if ( i == 0 ) {
		websWrite(wp, T(
		"checked onClick=\"postSecurity(%d, %d, %d, %d, %d, '%s', %d, '%s', '%s','%s')\""
		"\n"),
//xl_yue
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		vChar, Entry.enable1X, Entry.wep, Entry.wpaAuth, Entry.wpaPSKFormat, Entry.wpaPSK, Entry.rsPort, strbuf2, Entry.rsPassword,name);
#else
		vChar, Entry.enable1X, Entry.wep, Entry.wpaAuth, Entry.wpaPSKFormat, strbuf, Entry.rsPort, strbuf2, Entry.rsPassword,name);
#endif
	} else {
		websWrite(wp, T(
		"onClick=\"postSecurity(%d, %d, %d, %d, %d, '%s', %d, '%s', '%s','%s')\""
		"\n"),
//xl_yue
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		vChar, Entry.enable1X, Entry.wep, Entry.wpaAuth, Entry.wpaPSKFormat, Entry.wpaPSK, Entry.rsPort, strbuf2, Entry.rsPassword,name);
#else
		vChar, Entry.enable1X, Entry.wep, Entry.wpaAuth, Entry.wpaPSKFormat, strbuf, Entry.rsPort, strbuf2, Entry.rsPassword,name);
#endif
	}
		
	return 0;
}

int postSSIDWEP(int eid, webs_t wp, int argc, char_t **argv)
{	
	char_t *name;
	MIB_CE_MBSSIB_T EntryWPA;
	MIB_CE_MBSSIB_WEP_T Entry;
	char strbuf[20], strbuf2[20];
	int len;
	int i;
	
	if (ejArgs(argc, argv, T("%s"), &name) < 1) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}	

	if ( !strcmp(name, T("root")) ){		
		i = 0;			
	}
	else if ( !strcmp(name, T("vap0")) ) {		
		i = 1;		
	}
	else if ( !strcmp(name, T("vap1")) ) {		
		i = 2;		
	}
	else if ( !strcmp(name, T("vap2")) ) {		
		i = 3;		
	}
	else if ( !strcmp(name, T("vap3")) ) {		
		i = 4;		
	}
	else {
		printf("Not support this SSID!\n");
		return 1;
	}

	if (0==i) {
		MBSSID_GetRootEntry(&EntryWPA);
		MBSSID_GetRootEntryWEP(&Entry);
	} else {
		if (!mib_chain_get(MIB_MBSSIB_WEP_TBL, i, (void *)&Entry)) {  		
  			printf("Error! Get MIB_MBSSIB_TBL(Root) error.\n");
  			return 1;					
		}	
	
		if (!mib_chain_get(MIB_MBSSIB_TBL, i, (void *)&EntryWPA)) {  		
	  		printf("Error! Get MIB_MBSSIB_TBL(Root) error.\n");
  			return 1;					
		}
	}
	
	
	if (EntryWPA.wep == 0)
		EntryWPA.wep = 1;
	
	/*	
	websWrite(wp, "%s.formWep.length.value = %d;\n", DOCUMENT, vChar);
	mib_get( MIB_WLAN_WEP_KEY_TYPE, (void *)&vChar );
	websWrite(wp, "%s.formWep.format.value = %d;\n", DOCUMENT, vChar+1);
	mib_get( MIB_WLAN_WEP_DEFAULT_KEY, (void *)&vChar );
	websWrite(wp, "%s.formWep.defaultTxKeyId.value = %d;\n", DOCUMENT, vChar+1);	
	*/
	
//added by xl_yue
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	websWrite(wp, T("onClick=\"postWEP(%d, %d, %d, %d)\";\n"),
			EntryWPA.wep, Entry.wepKeyType+1, Entry.wepDefaultKey+1, i);
#else  
	websWrite(wp, T(
	"onClick=\"postWEP(%d, %d, %d)\";\n"),
	EntryWPA.wep, Entry.wepKeyType+1, Entry.wepDefaultKey+1);
#endif
				
	return 0;
}

int checkSSID(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t *name;
	MIB_CE_MBSSIB_T Entry;
	//char strbuf[20], strbuf2[20];
	//int len;
	int i;
	
	if (ejArgs(argc, argv, T("%s"), &name) < 1) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}	
	
	if ( !strcmp(name, T("vap0")) ){		
		i = 1;		
		
	}
	else if ( !strcmp(name, T("vap1")) ) {		
		i = 2;		
	}
	else if ( !strcmp(name, T("vap2")) ) {		
		i = 3;		
	}
	else if ( !strcmp(name, T("vap3")) ) {		
		i = 4;		
	}	
	else {
		printf("Not support this VAP!\n");
		return 1;
	}

	if (0==i) {
		MBSSID_GetRootEntry(&Entry);
	} else {
		if (!mib_chain_get(MIB_MBSSIB_TBL, i, (void *)&Entry)) {  		
	  		printf("Error! Get MIB_MBSSIB_TBL(Root) error.\n");
  			return 1;					
		}
	}
	
	if ( Entry.wlanDisabled == 0 ) {		
		websWrite(wp, "checked");
	} else {		
		websWrite(wp, "");
	}
}


int SSIDStr(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t *name;
	MIB_CE_MBSSIB_T Entry;
	//char strbuf[20], strbuf2[20];
	//int len;
	int i;
	
	if (ejArgs(argc, argv, T("%s"), &name) < 1) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}	
	
	if ( !strcmp(name, T("vap0")) ){		
		i = 1;		
		
	}
	else if ( !strcmp(name, T("vap1")) ) {		
		i = 2;		
		
	}
	else if ( !strcmp(name, T("vap2")) ) {		
		i = 3;		
	}
	else if ( !strcmp(name, T("vap3")) ) {		
		i = 4;		
	}	
	else {
		printf("SSIDStr: Not support this VAP!\n");
		return 1;
	}
	if (0==i) {
		MBSSID_GetRootEntry(&Entry);
	} else {
		if (!mib_chain_get(MIB_MBSSIB_TBL, i, (void *)&Entry)) {  		
	  		printf("Error! Get MIB_MBSSIB_TBL(SSIDStr) error.\n");
  			return 1;					
		}	
	}
	
	websWrite(wp, "%s", Entry.ssid);
	
}
#endif

#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
//xl_yue
int LinkMBSSIDPage(int eid, webs_t wp, int argc, char_t **argv)
{
	char *parm;
	
	if (ejArgs(argc, argv, T("%s"), &parm) < 1) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}
	
#ifdef WLAN_MBSSID
	websWrite(wp, 
 		 "<tr>"\
 		 "<td width=\"26%%\"><font size=2><b>虚拟SSID:</b></td>"\
     		 "<td width=\"74%%\"><font size=2><input type=\"button\" value=\"设置虚拟SSID\" name=\"setVSSID\" onClick=\"setVSSIDClick('/admin/wlmbssid_sc.asp')\">"\
  		 "</td></tr>\n");
#endif

}
#endif


/////////////////////////////////////////////////////////////////////////////
//check wlan status and set checkbox
int wlanStatus(int eid, webs_t wp, int argc, char_t **argv)
{
#ifdef WLAN_SUPPORT
	if (wlan_is_up())
	    //websWrite(wp, T("\"OFF\" enabled"));
	    websWrite(wp, T("\"OFF\""));
	else 
	    //websWrite(wp, T("\"ON\" checked disabled"));
	    websWrite(wp, T("\"ON\" checked"));
	return 0;
#endif
}
/////////////////////////////////////////////////////////////////////////////
void formAdvanceSetup(webs_t wp, char_t *path, char_t *query)
{
	char_t *submitUrl, *strAuth, *strFragTh, *strRtsTh, *strBeacon, *strPreamble;
	char_t *strRate, *strHiddenSSID, *strDtim, *strIapp, *strBlock;
	char_t *strProtection, *strAggregation, *strShortGIO;
	char_t vChar;
	unsigned short uShort;
	AUTH_TYPE_T authType;
	PREAMBLE_T preamble;
	int val;
	char tmpBuf[100];

//xl_yue: translocate to basic_setting   for ZTE531B BRIDGE
#if !(defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC))
	strAuth = websGetVar(wp, T("authType"), T(""));
	if (strAuth[0]) {
		if ( !gstrcmp(strAuth, T("open")))
			authType = AUTH_OPEN;
		else if ( !gstrcmp(strAuth, T("shared")))
			authType = AUTH_SHARED;
		else if ( !gstrcmp(strAuth, T("both")))
			authType = AUTH_BOTH;
		else {
			strcpy(tmpBuf, T(strInvdAuthType));
			goto setErr_advance;
		}
		vChar = (char_t)authType;
		if ( mib_set(MIB_WLAN_AUTH_TYPE, (void *)&vChar) == 0) {
			strcpy(tmpBuf, T(strSetAuthTypeErr));
			goto setErr_advance;
		}
	}
#endif
	strFragTh = websGetVar(wp, T("fragThreshold"), T(""));
	if (strFragTh[0]) {
		if ( !string_to_dec(strFragTh, &val) || val<256 || val>2346) {
			strcpy(tmpBuf, T(strFragThreshold));
			goto setErr_advance;
		}
		uShort = (unsigned short)val;
		if ( mib_set(MIB_WLAN_FRAG_THRESHOLD, (void *)&uShort) == 0) {
			strcpy(tmpBuf, T(strSetFragThreErr));
			goto setErr_advance;
		}
	}
	strRtsTh = websGetVar(wp, T("rtsThreshold"), T(""));
	if (strRtsTh[0]) {
		if ( !string_to_dec(strRtsTh, &val) || val<0 || val>2347) {
			strcpy(tmpBuf, T(strRTSThreshold));
			goto setErr_advance;
		}
		uShort = (unsigned short)val;
		if ( mib_set(MIB_WLAN_RTS_THRESHOLD, (void *)&uShort) == 0) {
			strcpy(tmpBuf, T(strSetRTSThreErr));
			goto setErr_advance;
		}
	}

	strBeacon = websGetVar(wp, T("beaconInterval"), T(""));
	if (strBeacon[0]) {
		if ( !string_to_dec(strBeacon, &val) || val<20 || val>1024) {
			strcpy(tmpBuf, T(strInvdBeaconIntv));
			goto setErr_advance;
		}
		uShort = (unsigned short)val;
		if ( mib_set(MIB_WLAN_BEACON_INTERVAL, (void *)&uShort) == 0) {
			strcpy(tmpBuf, T(strSetBeaconIntvErr));
			goto setErr_advance;
		}
	}

//xl_yue: translocate to basic_setting  for ZTE531B BRIDGE
#if !(defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC))
	// set tx rate
	strRate = websGetVar(wp, T("txRate"), T(""));
	if ( strRate[0] ) {
		if ( strRate[0] == '0' ) { // auto
			vChar = 1;
			if ( mib_set(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&vChar) == 0) {
				strcpy(tmpBuf, T(strSetAdapRateErr));
				goto setErr_advance;
			}
		}
		else  {
			vChar = 0;
			if ( mib_set(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&vChar) == 0) {
				strcpy(tmpBuf, T(strSetAdapRateErr));
				goto setErr_advance;
			}
#ifdef WLAN_8185AG
#ifdef CONFIG_USB_RTL8192SU_SOFTAP
			{
				unsigned int uInt;
				uInt = atoi(strRate);
				uInt = 1 << (uInt-1);
				if ( mib_set(MIB_WLAN_FIX_RATE, (void *)&uInt) == 0) {
					strcpy(tmpBuf, T(strSetFixdRateErr));
					goto setErr_advance;
				}
			}
#else
			uShort = atoi(strRate);
			uShort = 1 << (uShort-1);
			if ( mib_set(MIB_WLAN_FIX_RATE, (void *)&uShort) == 0) {
				strcpy(tmpBuf, T(strSetFixdRateErr));
				goto setErr_advance;
			}
#endif			
			strRate = websGetVar(wp, T("basicrates"), T(""));
			if ( strRate[0] ) {
				uShort = atoi(strRate);
				if ( mib_set(MIB_WLAN_BASIC_RATE, (void *)&uShort) == 0) {
					strcpy(tmpBuf, T(strSetBaseRateErr));
					goto setErr_advance;
				}
			}

			strRate = websGetVar(wp, T("operrates"), T(""));
			if ( strRate[0] ) {
				uShort = atoi(strRate);
				if ( mib_set(MIB_WLAN_SUPPORTED_RATE, (void *)&uShort) == 0) {
					strcpy(tmpBuf, T(strSetOperRateErr));
					goto setErr_advance;
				}
			}
#else

			if (strRate[0] == '1' )  // 11M
				uShort = TX_RATE_11M;
			else if (strRate[0] == '2' ) // 5.5M
				uShort = TX_RATE_5M;
			else if (strRate[0] == '3' ) // 2M
				uShort = TX_RATE_2M;
			else if (strRate[0] == '4' ) // 1M
				uShort = TX_RATE_1M;
			else {
				strcpy(tmpBuf, T(strInvdTxRate));
				goto setErr_advance;
			}
			if ( mib_set(MIB_WLAN_SUPPORTED_RATE, (void *)&uShort) == 0) {
				strcpy(tmpBuf, T(strSetOperRateErr));
				goto setErr_advance;
			}
			if ( mib_set(MIB_WLAN_BASIC_RATE, (void *)&uShort) == 0) {
				strcpy(tmpBuf, T(strSetBaseRateErr));
				goto setErr_advance;
			}
#endif
		}
	}
	else { // set rate in operate, basic sperately
#ifdef WIFI_TEST
		vChar = 0;
		uShort = 0;
		// disable rate adaptive
		if ( mib_set(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&vChar) == 0) {
			strcpy(tmpBuf, T(strSetAdapRateErr));
			goto setErr_advance;
			}

#if 0
		strRate = websGetVar(wp, T("operRate1M"), T(""));
		if ( !gstrcmp(strRate, T("1M")))
			uShort |= TX_RATE_1M;
		strRate = websGetVar(wp, T("operRate2M"), T(""));
		if ( !gstrcmp(strRate, T("2M")))
			uShort |= TX_RATE_2M;
		strRate = websGetVar(wp, T("operRate5M"), T(""));
		if ( !gstrcmp(strRate, T("5M")))
			uShort |= TX_RATE_5M;
		strRate = websGetVar(wp, T("operRate11M"), T(""));
		if ( !gstrcmp(strRate, T("11M")))
			uShort |= TX_RATE_11M;
		strRate = websGetVar(wp, T("operRate6M"), T(""));
		if ( !gstrcmp(strRate, T("6M")))
			uShort |= TX_RATE_6M;
		strRate = websGetVar(wp, T("operRate9M"), T(""));
		if ( !gstrcmp(strRate, T("9M")))
			uShort |= TX_RATE_9M;
		strRate = websGetVar(wp, T("operRate12M"), T(""));
		if ( !gstrcmp(strRate, T("12M")))
			uShort |= TX_RATE_12M;
		strRate = websGetVar(wp, T("operRate18M"), T(""));
		if ( !gstrcmp(strRate, T("18M")))
			uShort |= TX_RATE_18M;			
		strRate = websGetVar(wp, T("operRate24M"), T(""));
		if ( !gstrcmp(strRate, T("24M")))
			uShort |= TX_RATE_24M;			
		strRate = websGetVar(wp, T("operRate36M"), T(""));
		if ( !gstrcmp(strRate, T("36M")))
			uShort |= TX_RATE_36M;			
		strRate = websGetVar(wp, T("operRate48M"), T(""));
		if ( !gstrcmp(strRate, T("48M")))
			uShort |= TX_RATE_48M;			
		strRate = websGetVar(wp, T("operRate54M"), T(""));
		if ( !gstrcmp(strRate, T("54M")))
			uShort |= TX_RATE_54M;
		if ( mib_set(MIB_WLAN_SUPPORTED_RATE, (void *)&uShort) == 0) {
			strcpy(tmpBuf, T("Set Tx operation rate failed!"));
			goto setErr_advance;
		}

		// set basic tx rate
		uShort = 0;
		strRate = websGetVar(wp, T("basicRate1M"), T(""));
		if ( !gstrcmp(strRate, T("1M")))
			uShort |= TX_RATE_1M;
		strRate = websGetVar(wp, T("basicRate2M"), T(""));
		if ( !gstrcmp(strRate, T("2M")))
			uShort |= TX_RATE_2M;
		strRate = websGetVar(wp, T("basicRate5M"), T(""));
		if ( !gstrcmp(strRate, T("5M")))
			uShort |= TX_RATE_5M;
		strRate = websGetVar(wp, T("basicRate11M"), T(""));
		if ( !gstrcmp(strRate, T("11M")))
			uShort |= TX_RATE_11M;
		strRate = websGetVar(wp, T("basicRate6M"), T(""));
		if ( !gstrcmp(strRate, T("6M")))
			uShort |= TX_RATE_6M;
		strRate = websGetVar(wp, T("basicRate9M"), T(""));
		if ( !gstrcmp(strRate, T("9M")))
			uShort |= TX_RATE_9M;
		strRate = websGetVar(wp, T("basicRate12M"), T(""));
		if ( !gstrcmp(strRate, T("12M")))
			uShort |= TX_RATE_12M;
		strRate = websGetVar(wp, T("basicRate18M"), T(""));
		if ( !gstrcmp(strRate, T("18M")))
			uShort |= TX_RATE_18M;			
		strRate = websGetVar(wp, T("basicRate24M"), T(""));
		if ( !gstrcmp(strRate, T("24M")))
			uShort |= TX_RATE_24M;			
		strRate = websGetVar(wp, T("basicRate36M"), T(""));
		if ( !gstrcmp(strRate, T("36M")))
			uShort |= TX_RATE_36M;			
		strRate = websGetVar(wp, T("basicRate48M"), T(""));
		if ( !gstrcmp(strRate, T("48M")))
			uShort |= TX_RATE_48M;			
		strRate = websGetVar(wp, T("basicRate54M"), T(""));
		if ( !gstrcmp(strRate, T("54M")))
			uShort |= TX_RATE_54M;			
		if ( mib_set(MIB_WLAN_BASIC_RATE, (void *)&uShort) == 0) {
			strcpy(tmpBuf, T("Set Tx basic rate failed!"));
			goto setErr_advance;
		}
#endif
#endif // of WIFI_TEST
	}
#endif

	// set preamble
	strPreamble = websGetVar(wp, T("preamble"), T(""));
	if (strPreamble[0]) {
		if (!gstrcmp(strPreamble, T("long")))
			preamble = LONG_PREAMBLE;
		else if (!gstrcmp(strPreamble, T("short")))
			preamble = SHORT_PREAMBLE;
//modified by xl_yue
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		else if (!gstrcmp(strPreamble, T("auto")))
			preamble = AUTO_PREAMBLE;
#endif
		else {
			strcpy(tmpBuf, T(strInvdPreamble));
			goto setErr_advance;
		}
		vChar = (char_t)preamble;
		if ( mib_set(MIB_WLAN_PREAMBLE_TYPE, (void *)&vChar) == 0) {
			strcpy(tmpBuf, T(strSetPreambleErr));
			goto setErr_advance;
		}
	}

//xl_yue: translocate to basic_setting  for ZTE531B BRIDGE
#if !(defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC))
	// set hidden SSID
	strHiddenSSID = websGetVar(wp, T("hiddenSSID"), T(""));
	if (strHiddenSSID[0]) {
		if (!gstrcmp(strHiddenSSID, T("no")))
			vChar = 0;
		else if (!gstrcmp(strHiddenSSID, T("yes")))
			vChar = 1;
		else {
			strcpy(tmpBuf, T(strInvdBrodSSID));
			goto setErr_advance;
		}
		if ( mib_set(MIB_WLAN_HIDDEN_SSID, (void *)&vChar) == 0) {
			strcpy(tmpBuf, T(strSetBrodSSIDErr));
			goto setErr_advance;
		}
	}
#endif

#ifdef CONFIG_USB_RTL8192SU_SOFTAP           // add by yq_zhou 2.10
	strProtection = websGetVar(wp, T("protection"), T("")); 
	if (strProtection[0]){   
		if (!gstrcmp(strProtection,T("yes")))
			vChar = 0;
		else if (!gstrcmp(strProtection,T("no")))
			vChar = 1;
		else{
			strcpy(tmpBuf, T(strInvdProtection)); 
			goto setErr_advance;
		}
		if (mib_set(MIB_WLAN_PROTECTION_DISABLED,(void *)&vChar) ==0){
			strcpy(tmpBuf, T(strSetProtectionErr));
			goto setErr_advance;
		}
	}

	strAggregation = websGetVar(wp, T("aggregation"), T(""));  
	if (strAggregation[0]){  
		if (!gstrcmp(strAggregation,T("enable")))
			vChar = 1;
		else if (!gstrcmp(strAggregation,T("disable")))
			vChar = 0;
		else{
			strcpy(tmpBuf, T(strInvdAggregation));
			goto setErr_advance;
		}
		if (mib_set(MIB_WLAN_AGGREGATION,(void *)&vChar) ==0){
			strcpy(tmpBuf, T(strSetAggregationErr));
			goto setErr_advance;
		}
	}

	strShortGIO = websGetVar(wp, T("shortGI0"), T(""));  
	if (strShortGIO[0]){   
		if (!gstrcmp(strShortGIO,T("on")))
			vChar = 1;
		else if (!gstrcmp(strShortGIO,T("off")))
			vChar = 0;
		else{
			strcpy(tmpBuf, T(strInvdShortGI0));
			goto setErr_advance;
		}
		if (mib_set(MIB_WLAN_SHORTGI_ENABLED,(void *)&vChar) ==0){
			strcpy(tmpBuf, T(strSetShortGI0Err));
			goto setErr_advance;
		}
	}
#endif

	strDtim = websGetVar(wp, T("dtimPeriod"), T(""));
	if (strDtim[0]) {
		if ( !string_to_dec(strDtim, &val) || val<1 || val>255) {
			strcpy(tmpBuf, T(strInvdDTIMPerd));
			goto setErr_advance;
		}
		vChar = (char_t)val;
		if ( mib_set(MIB_WLAN_DTIM_PERIOD, (void *)&vChar) == 0) {
			strcpy(tmpBuf, T(strSetDTIMErr));
			goto setErr_advance;
		}
	}

#ifdef WLAN_IAPP
	strIapp = websGetVar(wp, T("iapp"), T(""));
	if (strIapp[0]) {
		if (!gstrcmp(strIapp, T("no")))
			vChar = 1;
		else if (!gstrcmp(strIapp, T("yes")))
			vChar = 0;
		else {
			strcpy(tmpBuf, T(strInvdIAPP));
			goto setErr_advance;
		}
		if ( mib_set(MIB_WLAN_IAPP_DISABLED, (void *)&vChar) == 0) {
			strcpy(tmpBuf, T(strMIBIAPPDISBErr));
			goto setErr_advance;
		}
	}
#endif

	// set block-relay
	strBlock = websGetVar(wp, T("block"), T(""));
	if (strBlock[0]) {
		if (strBlock[0] == '0')
			vChar = 0;
		else // '1'
			vChar = 1;
		if ( mib_set(MIB_WLAN_BLOCK_RELAY, (void *)&vChar) == 0) {
			strcpy(tmpBuf, T(strSetRelayBlockErr));
			goto setErr_advance;
		}
	}

#ifdef WLAN_QoS
	strBlock = websGetVar(wp, T("WmmEnabled"), T(""));
	if (strBlock[0]) {
		if (strBlock[0] == '0')
			vChar = 0;
		else // '1'
			vChar = 1;
		if ( mib_set(MIB_WLAN_QoS, (void *)&vChar) == 0) {
			strcpy(tmpBuf, T(strSetWlanWMMErr));
			goto setErr_advance;
		}
	}
#endif
//#ifdef APPLY_CHANGE
	config_WLAN(ACT_RESTART);
//#endif
//	mib_update(CURRENT_SETTING);

#ifndef NO_ACTION
	run_script(-1);
#endif

	submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page

//	websRedirect(wp, submitUrl);
	OK_MSG(submitUrl);
  	return;

setErr_advance:
	ERR_MSG(tmpBuf);
}

/////////////////////////////////////////////////////////////////////////////
int wirelessClientList(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0, i,j,found=0;
	WLAN_STA_INFO_Tp pInfo;
	char *buff;
//add by ramen for filtering the repeat WLAN client.
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
        char macbuffer[5*MAX_STA_NUM][20]={0};
	int index=0,m=0;
#endif
//xl_yue
#ifndef WLAN_MBSSID
	buff = calloc(1, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1));
	if ( buff == 0 ) {
		printf("Allocate buffer failed!\n");
		return 0;
	}
	if ( getWlStaInfo(WLANIF,  (WLAN_STA_INFO_Tp)buff ) < 0 ) {
		printf("Read wlan sta info failed!\n");
		free(buff);
		return 0;
	}
#else
	for(j=0;j<5;j++)
	{
		buff = calloc(1, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1));
		if ( buff == 0 ) {
			printf("Allocate buffer failed!\n");
			return 0;
		}

		if ( getWlStaInfo(WLANAPIF[j],  (WLAN_STA_INFO_Tp)buff ) < 0 ) {
			printf("Read wlan sta info failed!\n");
			free(buff);
			return 0;
		}
#endif

	for (i=1; i<=MAX_STA_NUM; i++) {
		pInfo = (WLAN_STA_INFO_Tp)&buff[i*sizeof(WLAN_STA_INFO_T)];
		if (pInfo->aid && (pInfo->flag & STA_INFO_FLAG_ASOC)) {
#ifdef CONFIG_USB_RTL8192SU_SOFTAP
			char txrate[20];
			int rateid=0;
			
			if((pInfo->txOperaRates & 0x80) != 0x80){	
				if(pInfo->txOperaRates%2){
					sprintf(txrate, "%d%s",pInfo->txOperaRates/2, ".5"); 
				}else{
					sprintf(txrate, "%d",pInfo->txOperaRates/2); 
				}
			}else{
				if((pInfo->ht_info & 0x1)==0){ //20M
					if((pInfo->ht_info & 0x2)==0){//long
						for(rateid=0; rateid<16;rateid++){
							if(rate_11n_table_20M_LONG[rateid].id == pInfo->txOperaRates){
								sprintf(txrate, "%s", rate_11n_table_20M_LONG[rateid].rate);
								break;
							}
						}
					}else if((pInfo->ht_info & 0x2)==0x2){//short
						for(rateid=0; rateid<16;rateid++){
							if(rate_11n_table_20M_SHORT[rateid].id == pInfo->txOperaRates){
								sprintf(txrate, "%s", rate_11n_table_20M_SHORT[rateid].rate);
								break;
							}
						}
					}
				}else if((pInfo->ht_info & 0x1)==0x1){//40M
					if((pInfo->ht_info & 0x2)==0){//long
						
						for(rateid=0; rateid<16;rateid++){
							if(rate_11n_table_40M_LONG[rateid].id == pInfo->txOperaRates){
								sprintf(txrate, "%s", rate_11n_table_40M_LONG[rateid].rate);
								break;
							}
						}
					}else if((pInfo->ht_info & 0x2)==0x2){//short
						for(rateid=0; rateid<16;rateid++){
							if(rate_11n_table_40M_SHORT[rateid].id == pInfo->txOperaRates){
								sprintf(txrate, "%s", rate_11n_table_40M_SHORT[rateid].rate);
								break;
							}
						}
					}
				}
				
			}
#endif
			
/*
printf("***%02x:%02x:%02x:%02x:%02x:%02x,%d,%d,%d%s,%s,%d, j=%d\n",pInfo->addr[0],pInfo->addr[1],pInfo->addr[2],pInfo->addr[3],pInfo->addr[4],pInfo->addr[5],
			pInfo->tx_packets, pInfo->rx_packets,
			pInfo->txOperaRates/2, ((pInfo->txOperaRates%2) ? ".5" : "" ),
			( (pInfo->flag & STA_INFO_FLAG_ASLEEP) ? "yes" : "no"),
			pInfo->expired_time/100,j);
*/
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
                        char tmpbuff[10];
			sprintf(tmpbuff,"%02x-%02x-%02x-%02x-%02x-%02x",pInfo->addr[0],pInfo->addr[1],pInfo->addr[2],pInfo->addr[3],pInfo->addr[4],pInfo->addr[5]);
			for(m=0;m<index;m++)
				{
				  if(!strncmp(tmpbuff,macbuffer[m],17))//found
				  	{
				  	  goto nextvalidclient;
				  	}
				  }
#endif
			nBytesSent += websWrite(wp,
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
			T("<tr bgcolor=#b7b7b7><td><font size=2>%02x-%02x-%02x-%02x-%02x-%02x</td>"
#else
	   		T("<tr bgcolor=#b7b7b7><td><font size=2>%02x:%02x:%02x:%02x:%02x:%02x</td>"
#endif
			"<td><font size=2>%d</td>"
	     		"<td><font size=2>%d</td>"
#ifdef CONFIG_USB_RTL8192SU_SOFTAP
			"<td><font size=2>%s</td>"
#else
			"<td><font size=2>%d%s</td>"
#endif			
			"<td><font size=2>%s</td>"
			"<td><font size=2>%d</td></tr>"),
			pInfo->addr[0],pInfo->addr[1],pInfo->addr[2],pInfo->addr[3],pInfo->addr[4],pInfo->addr[5],
			pInfo->tx_packets, pInfo->rx_packets,
#ifdef CONFIG_USB_RTL8192SU_SOFTAP
			txrate,
#else
			pInfo->txOperaRates/2, ((pInfo->txOperaRates%2) ? ".5" : "" ),
#endif			
			( (pInfo->flag & STA_INFO_FLAG_ASLEEP) ? "yes" : "no"),
			pInfo->expired_time/100 );
			found++;
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		sprintf(macbuffer[index++],"%02x-%02x-%02x-%02x-%02x-%02x",pInfo->addr[0],pInfo->addr[1],pInfo->addr[2],pInfo->addr[3],pInfo->addr[4],pInfo->addr[5]);
nextvalidclient:
#endif
		}
	}
//xl_yue	
#ifdef WLAN_MBSSID
	free(buff);
	}
#endif

	if (found == 0) {
		nBytesSent += websWrite(wp,
	   		T("<tr bgcolor=#b7b7b7><td><font size=2>None</td>"
			"<td><font size=2>---</td>"
	     		"<td><font size=2>---</td>"
			"<td><font size=2>---</td>"
			"<td><font size=2>---</td>"
			"<td><font size=2>---</td></tr>"));
	}
//xl_yue
#ifndef WLAN_MBSSID
	free(buff);
#endif
	return nBytesSent;
}

/////////////////////////////////////////////////////////////////////////////
void formWirelessTbl(webs_t wp, char_t *path, char_t *query)
{
	char_t *submitUrl;

	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
}

#ifdef WLAN_CLIENT
static SS_STATUS_Tp pStatus=NULL;
/////////////////////////////////////////////////////////////////////////////
void formWlSiteSurvey(webs_t wp, char_t *path, char_t *query)
{
 	char_t *submitUrl, *refresh, *connect, *strSel;
	int status, idx;
	unsigned char res, *pMsg=NULL;
	int wait_time;
	char tmpBuf[100];

	submitUrl = websGetVar(wp, T("submit-url"), T(""));

	refresh = websGetVar(wp, T("refresh"), T(""));
	if ( refresh[0] ) {
		// issue scan request
		wait_time = 0;
		while (1) {
			if ( getWlSiteSurveyRequest(WLANIF,  &status) < 0 ) {
				strcpy(tmpBuf, T("Site-survey request failed!"));
				goto ss_err;
			}
			if (status != 0) {	// not ready
				if (wait_time++ > 5) {
					strcpy(tmpBuf, T("scan request timeout!"));
					goto ss_err;
				}
				sleep(1);
			}
			else
				break;
		}

		// wait until scan completely
		wait_time = 0;
		while (1) {
			res = 1;	// only request request status
			if ( getWlSiteSurveyResult(WLANIF, (SS_STATUS_Tp)&res) < 0 ) {
				strcpy(tmpBuf, T("Read site-survey status failed!"));
				free(pStatus);
				pStatus = NULL;
				goto ss_err;
			}
			if (res == 0xff) {   // in progress
				if (wait_time++ > 10) {
					strcpy(tmpBuf, T("scan timeout!"));
					free(pStatus);
					pStatus = NULL;
					goto ss_err;
				}
				sleep(1);
			}
			else
				break;
		}

		if (submitUrl[0])
			websRedirect(wp, submitUrl);

		return;
	}

	connect = websGetVar(wp, T("connect"), T(""));
	if ( connect[0] ) {
		strSel = websGetVar(wp, T("select"), T(""));
		if (strSel[0]) {
			unsigned char res;
			NETWORK_TYPE_T net;
			int chan;

			if (pStatus == NULL) {
				strcpy(tmpBuf, T("Please refresh again!"));
				goto ss_err;

			}
			sscanf(strSel, "sel%d", &idx);
			if ( idx >= pStatus->number ) { // invalid index
				strcpy(tmpBuf, T("Connect failed!"));
				goto ss_err;
			}

			// Set SSID, network type to MIB
			memcpy(tmpBuf, pStatus->bssdb[idx].bdSsIdBuf, pStatus->bssdb[idx].bdSsId.Length);
			tmpBuf[pStatus->bssdb[idx].bdSsId.Length] = '\0';
			if ( mib_set(MIB_WLAN_SSID, (void *)tmpBuf) == 0) {
				strcpy(tmpBuf, T("Set SSID error!"));
				goto ss_err;
			}

			if ( pStatus->bssdb[idx].bdCap & cESS )
				net = INFRASTRUCTURE;
			else
				net = ADHOC;
			
			if ( mib_set(MIB_WLAN_NETWORK_TYPE, (void *)&net) == 0) {
				strcpy(tmpBuf, T("Set MIB_WLAN_NETWORK_TYPE failed!"));
				goto ss_err;
			}

			if (net == ADHOC) {
				chan = pStatus->bssdb[idx].ChannelNumber;
				if ( mib_set( MIB_WLAN_CHAN_NUM, (void *)&chan) == 0) {
   					strcpy(tmpBuf, T("Set channel number error!"));
					goto ss_err;
				}
			}

//			mib_update(CURRENT_SETTING);

			res = idx;
			wait_time = 0;
			while (1) {
				if ( getWlJoinRequest(WLANIF, &pStatus->bssdb[idx], &res) < 0 ) {
					strcpy(tmpBuf, T("Join request failed!"));
					goto ss_err;
				}
				if ( res == 1 ) { // wait
					if (wait_time++ > 5) {
						strcpy(tmpBuf, T("connect-request timeout!"));
						goto ss_err;
					}
					sleep(1);
					continue;
				}
				break;
			}

			if ( res == 2 ) // invalid index
				pMsg = "Connect failed!";
			else {
				wait_time = 0;
				while (1) {
					if ( getWlJoinResult(WLANIF, &res) < 0 ) {
						strcpy(tmpBuf, T("Get Join result failed!"));
						goto ss_err;
					}
					if ( res != 0xff ) { // completed
						if (wait_time++ > 10) {
							strcpy(tmpBuf, T("connect timeout!"));
							goto ss_err;
						}
						break;
					}
					sleep(1);
				}

				if ( res!=STATE_Bss && res!=STATE_Ibss_Idle && res!=STATE_Ibss_Active )
					pMsg = "Connect failed!";
				else
					pMsg = "Connect successfully!";
			}
			OK_MSG1(pMsg, submitUrl);
		}
	}
	return;

ss_err:
	ERR_MSG(tmpBuf);
}

/////////////////////////////////////////////////////////////////////////////
int wlSiteSurveyTbl(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0, i;
	BssDscr *pBss;
	char tmpBuf[100], ssidbuf[40];
	WLAN_MODE_T mode;
	unsigned char mib_mode;
	bss_info bss;
	if (pStatus==NULL) {
		pStatus = calloc(1, sizeof(SS_STATUS_T));
		if ( pStatus == NULL ) {
			printf("Allocate buffer failed!\n");
			return 0;
		}
	}

	pStatus->number = 0; // request BSS DB

	if ( getWlSiteSurveyResult(WLANIF, pStatus) < 0 ) {
		ERR_MSG("Read site-survey status failed!");
		free(pStatus);
		pStatus = NULL;
		return 0;
	}

	if ( !mib_get( MIB_WLAN_MODE, (void *)&mib_mode) ) {
		printf("Get MIB_WLAN_MODE MIB failed!");
		return 0;
	}
	mode=mib_mode;
	if ( getWlBssInfo(WLANIF, &bss) < 0) {
		printf("Get bssinfo failed!");
		return 0;
	}

	nBytesSent += websWrite(wp, T("<tr>"
	"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b>SSID</b></font></td>\n"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>BSSID</b></font></td>\n"
	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Channel</b></font></td>\n"
      	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Type</b></font></td>\n"
      	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Encrypt</b></font></td>\n"
	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Signal</b></font></td>\n"));
	if ( mode == CLIENT_MODE )
		nBytesSent += websWrite(wp, T("<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n"));
	else
		nBytesSent += websWrite(wp, T("</tr>\n"));

	for (i=0; i<pStatus->number && pStatus->number!=0xff; i++) {
		pBss = &pStatus->bssdb[i];
		snprintf(tmpBuf, 200, T("%02x:%02x:%02x:%02x:%02x:%02x"),
			pBss->bdBssId[0], pBss->bdBssId[1], pBss->bdBssId[2],
			pBss->bdBssId[3], pBss->bdBssId[4], pBss->bdBssId[5]);

		memcpy(ssidbuf, pBss->bdSsIdBuf, pBss->bdSsId.Length);
		ssidbuf[pBss->bdSsId.Length] = '\0';

		nBytesSent += websWrite(wp, T("<tr>"
			"<td align=left width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%d</td>\n"
      			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%d</td>\n"),
			ssidbuf, tmpBuf, pBss->ChannelNumber,
			((pBss->bdCap & cIBSS) ? "Ad hoc" : "AP"),
			((pBss->bdCap & cPrivacy) ? "yes" : "no"), pBss->rssi);

		if ( mode == CLIENT_MODE )
			nBytesSent += websWrite(wp,
			T("<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name="
			"\"select\" value=\"sel%d\" onClick=\"enableConnect()\"></td></tr>\n"), i);
		else
			nBytesSent += websWrite(wp, T("</tr>\n"));
	}

	return nBytesSent;
}
#endif	// of WLAN_CLIENT


#ifdef WLAN_WDS
/////////////////////////////////////////////////////////////////////////////
void formWlWds(webs_t wp, char_t *path, char_t *query)
{
	char_t *strAddMac, *strDelMac, *strDelAllMac, *strVal, *submitUrl, *strEnabled, *strSet;
	char tmpBuf[100];
	int  i,idx;
	WDS_T macEntry;
	WDS_T Entry;
	unsigned char entryNum,enabled,delNum=0;

	strSet = websGetVar(wp, T("wdsSet"), T(""));
	strAddMac = websGetVar(wp, T("addWdsMac"), T(""));
	strDelMac = websGetVar(wp, T("deleteSelWdsMac"), T(""));
	strDelAllMac = websGetVar(wp, T("deleteAllWdsMac"), T(""));
	strEnabled = websGetVar(wp, T("wlanWdsEnabled"), T(""));

	if (strSet[0]) {
		if (!gstrcmp(strEnabled, T("ON"))){
			enabled = 1;
		}
		else
			enabled = 0;
		if (mib_set( MIB_WLAN_WDS_ENABLED, (void *)&enabled) == 0) {
  			strcpy(tmpBuf, T(strSetEnableErr));
			goto setErr_wds;
		}
	}

	if (strAddMac[0]) {
		int intVal;
		/*if ( !gstrcmp(strEnabled, T("ON"))){
			enabled = 1;
		}
		else
			enabled = 0;
		if ( mib_set( MIB_WLAN_WDS_ENABLED, (void *)&enabled) == 0) {
  			strcpy(tmpBuf, T(strSetEnableErr));
			goto setErr_wds;
		}*/
		strVal = websGetVar(wp, T("mac"), T(""));
		if ( !strVal[0] )
			goto setWds_ret;

//added by xl_yue: kick of "-" in string
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		for(i=0;i<17;i++){
			if((i+1)%3 != 0)
				strVal[i-(i+1)/3] = strVal[i];
		}
		strVal[12] = '\0';
#endif

		if (strlen(strVal)!=12 || !string_to_hex(strVal, macEntry.macAddr, 12)) {
			strcpy(tmpBuf, T(strInvdMACAddr));
			goto setErr_wds;
		}
		if (!isValidMacAddr(macEntry.macAddr)) {
			strcpy(tmpBuf, T(strInvdMACAddr));
			goto setErr_wds;
		}

		strVal = websGetVar(wp, T("comment"), T(""));
		if ( strVal[0] ) {
			if (strlen(strVal) > COMMENT_LEN-1) {
				strcpy(tmpBuf, T(strCommentTooLong));
				goto setErr_wds;
			}
			strcpy(macEntry.comment, strVal);
		}
		else
			macEntry.comment[0] = '\0';

		if ( !mib_get(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, T(strGetEntryNumErr));
			goto setErr_wds;
		}
		if ( (entryNum + 1) > MAX_WDS_NUM) {
			strcpy(tmpBuf, T(strErrForTablFull));
			goto setErr_wds;
		}

		// Jenny added, set to MIB. Check if entry exists
		for (i=0; i<entryNum; i++) {
			if (!mib_chain_get(MIB_WDS_TBL, i, (void *)&Entry)) {
	  			websError(wp, 400, T("Get chain record error!\n"));
				return;
			}
			if (!memcmp(macEntry.macAddr, Entry.macAddr, 6)) {
				strcpy(tmpBuf, T(strMACInList));
				goto setErr_wds;
			}
		}

		// set to MIB. try to delete it first to avoid duplicate case
		//mib_set(MIB_WLAN_WDS_DEL, (void *)&macEntry);
		intVal = mib_chain_add(MIB_WDS_TBL, (void *)&macEntry);
		if (intVal == 0) {
			strcpy(tmpBuf, T(strAddEntryErr));
			goto setErr_wds;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, T(strTableFull));
			goto setErr_wds;
		}
		entryNum++;
		if ( !mib_set(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, T(strGetEntryNumErr));
			goto setErr_wds;
		}
	}

	/* Delete entry */
	delNum=0;
	if (strDelMac[0]) {
		if ( !mib_get(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, T(strGetEntryNumErr));
			goto setErr_wds;
		}
		for (i=0; i<entryNum; i++) {

			idx = entryNum-i-1;			
			snprintf(tmpBuf, 20, "select%d", idx);
			strVal = websGetVar(wp, tmpBuf, T(""));				
			
			if ( !gstrcmp(strVal, T("ON")) ) {	
				if(mib_chain_delete(MIB_WDS_TBL, idx) != 1) {					
					strcpy(tmpBuf, T(strDelRecordErr));
				}
				delNum++;
			}
		}
		entryNum-=delNum; 
		if ( !mib_set(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, T(strGetEntryNumErr));
			goto setErr_wds;
		}
		if (delNum <= 0) {
			strcpy(tmpBuf, T("There is no item selected to delete!"));
			goto setErr_wds;
		}
	}

	/* Delete all entry */
	if ( strDelAllMac[0]) {
		mib_chain_clear(MIB_WDS_TBL); /* clear chain record */

		entryNum=0;
		if ( !mib_set(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, T(strGetEntryNumErr));
			goto setErr_wds;
		}

	}

setWds_ret:
//#ifdef APPLY_CHANGE
	config_WLAN(ACT_RESTART);	
//#endif
//	mib_update(CURRENT_SETTING);

#ifndef NO_ACTION
	run_script(-1);
#endif

	submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page
	OK_MSG( submitUrl );
  	return;

setErr_wds:
	ERR_MSG(tmpBuf);
}


/////////////////////////////////////////////////////////////////////////////
int wlWdsList(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0, i;
	WDS_T entry;
	char tmpBuf[100];
	unsigned char entryNum;
	WDS_T Entry;

	if ( !mib_get(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
  		websError(wp, 400, T("Get table entry error!\n"));
		return -1;
	}
//modified by xl_yue
	nBytesSent += websWrite(wp, T("<tr>"
      	"<td align=center width=\"45%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td>\n"
      	"<td align=center width=\"35%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td></tr>\n"),strMACAddr,strWdsComment,strSelect);

	for (i=0; i<entryNum; i++) {
		*((char *)&entry) = (char)i;
		if (!mib_chain_get(MIB_WDS_TBL, i, (void *)&Entry)) {
  			websError(wp, 400, errGetEntry);
			return -1;
		}
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		snprintf(tmpBuf, 100, T("%02x-%02x-%02x-%02x-%02x-%02x"),
			Entry.macAddr[0], Entry.macAddr[1], Entry.macAddr[2],
			Entry.macAddr[3], Entry.macAddr[4], Entry.macAddr[5]);
#else
		snprintf(tmpBuf, 100, T("%02x:%02x:%02x:%02x:%02x:%02x"),
			Entry.macAddr[0], Entry.macAddr[1], Entry.macAddr[2],
			Entry.macAddr[3], Entry.macAddr[4], Entry.macAddr[5]);
#endif
		nBytesSent += websWrite(wp, T("<tr>"
			"<td align=center width=\"45%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"35%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
       			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n"),
				tmpBuf, Entry.comment, i);
	}
	return nBytesSent;
}


/////////////////////////////////////////////////////////////////////////////
void formWdsWep(webs_t wp, char_t *path, char_t *query)
{
   	char_t *submitUrl, *wepKey;
   	char_t *strKeyLen, *strFormat, *strKeyId, *strEnabled;
	char tmpBuf[100], key[30];
	int enabled, keyLen, ret, i;
	WEP_T wep;

	enabled = 1;
	if ( mib_set( MIB_WLAN_WDS_ENABLED, (void *)&enabled) == 0) {
  		strcpy(tmpBuf, T("Set enabled flag error!"));
		goto setErr_wdsWep;
	}

	strEnabled = websGetVar(wp, T("wepEnabled"), T(""));
	if ( !gstrcmp(strEnabled, T("ON")))
		enabled = 1;
	else
		enabled = 0;

	if ( enabled ) {
		strKeyLen = websGetVar(wp, T("length"), T(""));
		if (!strKeyLen[0]) {
 			strcpy(tmpBuf, T("Key length must exist!"));
			goto setErr_wdsWep;
		}
		if (strKeyLen[0]!='1' && strKeyLen[0]!='2') {
 			strcpy(tmpBuf, T("Invalid key length value!"));
			goto setErr_wdsWep;
		}
		if (strKeyLen[0] == '1')
			wep = WEP64;
		else
			wep = WEP128;
	}
	else
		wep = WEP_DISABLED;

	if ( mib_set( MIB_WLAN_WDS_WEP, (void *)&wep) == 0) {
  		strcpy(tmpBuf, T("Set WEP MIB error!"));
		goto setErr_wdsWep;
	}

	if (wep == WEP_DISABLED)
		goto setWdsWep_ret;

	strFormat = websGetVar(wp, T("format"), T(""));
	if (!strFormat[0]) {
 		strcpy(tmpBuf, T("Key type must exist!"));
		goto setErr_wdsWep;
	}

	if (strFormat[0]!='1' && strFormat[0]!='2') {
		strcpy(tmpBuf, T("Invalid key type value!"));
		goto setErr_wdsWep;
	}

	i = strFormat[0] - '0' - 1;
	if ( mib_set( MIB_WLAN_WDS_WEP_KEY_TYPE, (void *)&i) == 0) {
  		strcpy(tmpBuf, T("Set WEP key type error!"));
		goto setErr_wdsWep;
	}

	if (wep == WEP64) {
		if (strFormat[0]=='1')
			keyLen = WEP64_KEY_LEN;
		else
			keyLen = WEP64_KEY_LEN*2;
	}
	else {
		if (strFormat[0]=='1')
			keyLen = WEP128_KEY_LEN;
		else
			keyLen = WEP128_KEY_LEN*2;
	}

	strKeyId = websGetVar(wp, T("defaultTxKeyId"), T(""));
	if ( strKeyId[0] ) {
		if ( strKeyId[0]!='1' && strKeyId[0]!='2' && strKeyId[0]!='3' && strKeyId[0]!='4' ) {
	 		strcpy(tmpBuf, T("Invalid default tx key id!"));
   			goto setErr_wdsWep;
		}
		i = strKeyId[0] - '0' - 1;
		if ( !mib_set( MIB_WLAN_WDS_WEP_DEFAULT_KEY, (void *)&i ) ) {
	 		strcpy(tmpBuf, T("Set default tx key id error!"));
   			goto setErr_wdsWep;
		}
	}

	wepKey = websGetVar(wp, T("key1"), T(""));
	if  (wepKey[0]) {
		if (strlen(wepKey) != keyLen) {
			strcpy(tmpBuf, T("Invalid key 1 length!"));
			goto setErr_wdsWep;
		}
		if ( !isAllStar(wepKey) ) {
			if (strFormat[0] == '1') // ascii
				strcpy(key, wepKey);
			else { // hex
				if ( !string_to_hex(wepKey, key, keyLen)) {
	   				strcpy(tmpBuf, T("Invalid wep-key1 value!"));
					goto setErr_wdsWep;
				}
			}
			if (wep == WEP64)
				ret=mib_set(MIB_WLAN_WDS_WEP64_KEY1, (void *)key);
			else
				ret=mib_set(MIB_WLAN_WDS_WEP128_KEY1, (void *)key);
			if (!ret) {
	 			strcpy(tmpBuf, T("Set wep-key1 error!"));
				goto setErr_wdsWep;
			}
		}
	}
	wepKey = websGetVar(wp, T("key2"), T(""));
	if  (wepKey[0]) {
		if (strlen(wepKey) != keyLen) {
			strcpy(tmpBuf, T("Invalid key 2 length!"));
			goto setErr_wdsWep;
		}
		if ( !isAllStar(wepKey) ) {
			if (strFormat[0] == '1') // ascii
				strcpy(key, wepKey);
			else { // hex
				if ( !string_to_hex(wepKey, key, keyLen)) {
	   				strcpy(tmpBuf, T("Invalid wep-key2 value!"));
   					goto setErr_wdsWep;
				}
			}
			if (wep == WEP64)
				ret=mib_set(MIB_WLAN_WDS_WEP64_KEY2, (void *)key);
			else
				ret=mib_set(MIB_WLAN_WDS_WEP128_KEY2, (void *)key);
			if (!ret) {
	 			strcpy(tmpBuf, T("Set wep-key2 error!"));
				goto setErr_wdsWep;
			}
		}
	}

	wepKey = websGetVar(wp, T("key3"), T(""));
	if  (wepKey[0]) {
		if (strlen(wepKey) != keyLen) {
			strcpy(tmpBuf, T("Invalid key 3 length!"));
			goto setErr_wdsWep;
		}
		if ( !isAllStar(wepKey) ) {
			if (strFormat[0] == '1') // ascii
				strcpy(key, wepKey);
			else { // hex
				if ( !string_to_hex(wepKey, key, keyLen)) {
	   				strcpy(tmpBuf, T("Invalid wep-key3 value!"));
   					goto setErr_wdsWep;
				}
			}
			if (wep == WEP64)
				ret=mib_set(MIB_WLAN_WDS_WEP64_KEY3, (void *)key);
			else
				ret=mib_set(MIB_WLAN_WDS_WEP128_KEY3, (void *)key);
			if (!ret) {
	 			strcpy(tmpBuf, T("Set wep-key3 error!"));
				goto setErr_wdsWep;
			}
		}
	}

	wepKey = websGetVar(wp, T("key4"), T(""));
	if  (wepKey[0]) {
		if (strlen(wepKey) != keyLen) {
			strcpy(tmpBuf, T("Invalid key 1 length!"));
			goto setErr_wdsWep;
		}
		if ( !isAllStar(wepKey) ) {
			if (strFormat[0] == '1') // ascii
				strcpy(key, wepKey);
			else { // hex
				if ( !string_to_hex(wepKey, key, keyLen)) {
	   				strcpy(tmpBuf, T("Invalid wep-key4 value!"));
   					goto setErr_wdsWep;
				}
			}
			if (wep == WEP64)
				ret=mib_set(MIB_WLAN_WDS_WEP64_KEY4, (void *)key);
			else
				ret=mib_set(MIB_WLAN_WDS_WEP128_KEY4, (void *)key);
			if (!ret) {
	 			strcpy(tmpBuf, T("Set wep-key4 error!"));
				goto setErr_wdsWep;
			}
		}
	}

setWdsWep_ret:
//	mib_update(CURRENT_SETTING);

#ifndef NO_ACTION
	run_script(-1);
#endif

	submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page
	OK_MSG(submitUrl);

	return;

setErr_wdsWep:
	ERR_MSG(tmpBuf);
}


/////////////////////////////////////////////////////////////////////////////
int wdsList(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0, i;
	WDS_INFO_Tp pInfo;
	char *buff;

	buff = calloc(1, sizeof(WDS_INFO_T)*MAX_STA_NUM);
	if ( buff == 0 ) {
		printf("Allocate buffer failed!\n");
		return 0;
	}

	if ( getWdsInfo(WLANIF, buff) < 0 ) {
		printf("Read wlan sta info failed!\n");
		return 0;
	}

	for (i=0; i<MAX_WDS_NUM; i++) {
		pInfo = (WDS_INFO_Tp)&buff[i*sizeof(WDS_INFO_T)];

		if (pInfo->state == STATE_WDS_EMPTY)
			break;

		nBytesSent += websWrite(wp,
	   		"<tr bgcolor=#b7b7b7><td><font size=2>%02x:%02x:%02x:%02x:%02x:%02x</td>"
			"<td><font size=2>%d</td>"
	     		"<td><font size=2>%d</td>"
			"<td><font size=2>%d</td>"
			"<td><font size=2>%d%s</td>",
			pInfo->addr[0],pInfo->addr[1],pInfo->addr[2],pInfo->addr[3],pInfo->addr[4],pInfo->addr[5],
			pInfo->tx_packets, pInfo->tx_errors, pInfo->rx_packets,
			pInfo->txOperaRate/2, ((pInfo->txOperaRate%2) ? ".5" : "" ));
	}

	free(buff);

	return nBytesSent;
}
#endif // WLAN_WDS

#ifdef CONFIG_WIFI_SIMPLE_CONFIG // WPS
#define _WSC_DAEMON_PROG       T("/bin/wscd")
#define WLAN_IF  T("wlan0")
#define OK_MSG1(msg, url) { \
        websHeader(wp); \
        websWrite(wp, T("<body><blockquote><h4>%s</h4>\n"), msg); \
        if (url) websWrite(wp, T("<form><input type=button value=\"  OK  \" OnClick=window.location.replace(\"%s\")></form></blockquote></body>"), url);\
        else websWrite(wp, T("<form><input type=button value=\"  OK  \" OnClick=window.close()></form></blockquote></body>"));\
        websFooter(wp); \
        websDone(wp, 200); \
}

#define OK_MSG2(msg, msg1, url) { \
        char tmp[200]; \
        sprintf(tmp, msg, msg1); \
        OK_MSG1(tmp, url); \
}
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
#define START_PBC_MSG \
        "成功开始PBC!<br><br>" \
        "你需要在2分钟内在 %s 端运行WPS."
#define START_PIN_MSG \
        "成功开始PIN!<br><br>" \
        "你需要在2分钟内在 %s 端运行WPS."
#define SET_PIN_MSG \
        "客户端PIN设置成功!<br><br>" \
        "你需要在2分钟内在client端运行WPS."
#else
#define START_PBC_MSG \
        "Start PBC successfully!<br><br>" \
        "You have to run Wi-Fi Protected Setup in %s within 2 minutes."
#define START_PIN_MSG \
        "Start PIN successfully!<br><br>" \
        "You have to run Wi-Fi Protected Setup in %s within 2 minutes."
#define SET_PIN_MSG \
        "Applied client's PIN successfully!<br><br>" \
        "You have to run Wi-Fi Protected Setup in client within 2 minutes."
#endif
#define MIB_GET(id, val) do { \
		if (0==mib_get(id, (void *)val)) { \
		} \
	} while (0)

static void update_wps_configured(int reset_flag)
{
	char is_configured, encrypt1, encrypt2, auth, disabled, iVal, mode, format;
	char ssid1[100], ssid2[100];
	unsigned char tmpbuf[100];

	//fprintf(stderr, "update_wps_configured(%d)\n", reset_flag);
	
	MIB_GET(MIB_WSC_CONFIGURED, (void *)&is_configured);
	MIB_GET(MIB_WLAN_MODE, (void *)&mode);	
	
	if (!is_configured && mode == AP_MODE) { 
		MIB_GET(MIB_WLAN_SSID, (void *)ssid1);
		mib_getDef(MIB_WLAN_SSID, (void *)ssid2);
		//fprintf(stderr, "ssid=%s defssid=%s\n", ssid1, ssid2);
		#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		if (strcmp(ssid1, ssid2) && (strlen(ssid2) != 0)) {
		#else		
		if (strcmp(ssid1, ssid2)) {
		#endif
			is_configured = 1;
			mib_set(MIB_WSC_CONFIGURED, (void *)&is_configured);

			MIB_GET(MIB_WSC_CONFIG_BY_EXT_REG, (void *)&iVal);		
			if (is_configured && iVal == 0) {
				iVal = 1;
				mib_set(MIB_WSC_MANUAL_ENABLED, (void *)&iVal);				
			}		
			//return;
		}
		
		MIB_GET(MIB_WLAN_ENCRYPT, (void *)&encrypt1);
		mib_getDef(MIB_WLAN_ENCRYPT, (void *)&encrypt2);

		if (encrypt1 != encrypt2) {
			is_configured = 1;
			mib_set(MIB_WSC_CONFIGURED, (void *)&is_configured);
		}		
	}

	MIB_GET(MIB_WSC_DISABLE, (void *)&disabled);
	if (disabled)
		return;
	
	MIB_GET(MIB_WLAN_ENCRYPT, (void *)&encrypt1);
	if (encrypt1 == ENCRYPT_DISABLED) {
		auth = WSC_AUTH_OPEN;
		encrypt2 = WSC_ENCRYPT_NONE;
	}
	else if (encrypt1 == ENCRYPT_WEP) {
		auth = WSC_AUTH_OPEN;
		encrypt2 = WSC_ENCRYPT_WEP;		
	}
	else if (encrypt1 == ENCRYPT_WPA) {
		auth = WSC_AUTH_WPAPSK;
		MIB_GET(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&encrypt1);
		if (encrypt1 == WPA_CIPHER_TKIP)
			encrypt2 = WSC_ENCRYPT_TKIP;		
		else if (encrypt1 == WPA_CIPHER_AES)
			encrypt2 = WSC_ENCRYPT_AES;		
		else 
			encrypt2 = WSC_ENCRYPT_TKIPAES;				
	}
	else if (encrypt1 == ENCRYPT_WPA2) {
		auth = WSC_AUTH_WPA2PSK;
		MIB_GET(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&encrypt1);
		if (encrypt1 == WPA_CIPHER_TKIP)
			encrypt2 = WSC_ENCRYPT_TKIP;		
		else if (encrypt1 == WPA_CIPHER_AES)
			encrypt2 = WSC_ENCRYPT_AES;		
		else 
			encrypt2 = WSC_ENCRYPT_TKIPAES;				
	}
	else {
		auth = WSC_AUTH_WPA2PSKMIXED;
		encrypt2 = WSC_ENCRYPT_TKIPAES;			
	}
	mib_set(MIB_WSC_AUTH, (void *)&auth);	
	mib_set(MIB_WSC_ENC, (void *)&encrypt2);

	MIB_GET(MIB_WLAN_ENCRYPT, (void *)&encrypt1);
	if (encrypt1 == ENCRYPT_WPA || encrypt1 == ENCRYPT_WPA2 || encrypt1 == ENCRYPT_WPA2_MIXED) {
		MIB_GET(MIB_WLAN_WPA_AUTH, (void *)&format);
		if (format & 2) { // PSK
			MIB_GET(MIB_WLAN_WPA_PSK, (void *)tmpbuf);
			mib_set(MIB_WSC_PSK, (void *)tmpbuf);					
		}		
	}
	if (reset_flag) {
		reset_flag = 0;
		mib_set(MIB_WSC_CONFIG_BY_EXT_REG, (void *)&reset_flag);		
	}	

	MIB_GET(MIB_WSC_CONFIG_BY_EXT_REG, (void *)&iVal);		
	if (is_configured && iVal == 0) {
		iVal = 1;
		mib_set(MIB_WSC_MANUAL_ENABLED, (void *)&iVal);				
	}	
	iVal = 0;
	if (mode == AP_MODE || mode == AP_WDS_MODE) {
		if (encrypt1 == ENCRYPT_WEP || encrypt1 == ENCRYPT_DISABLED) {
			MIB_GET(MIB_WLAN_ENABLE_1X, (void *)&encrypt2);
			if (encrypt2)
				iVal = 1;
		}
		else {
			MIB_GET(MIB_WLAN_WPA_AUTH, (void *)&encrypt2);
			if (encrypt2 == WPA_AUTH_AUTO)
				iVal = 1;
		}
	}
	else if (mode == CLIENT_MODE || mode == AP_WDS_MODE)
		iVal = 1;
	if (iVal) {
		iVal = 0;
		mib_set(MIB_WSC_MANUAL_ENABLED, (void *)&iVal);		
		mib_set(MIB_WSC_CONFIGURED, (void *)&iVal);				
		mib_set(MIB_WSC_CONFIG_BY_EXT_REG, (void *)&iVal);	
	}	
}


void formWsc(webs_t wp, char_t *path, char_t *query)
{
	char_t *strVal, *submitUrl;
	char intVal;
	char tmpbuf[200];
//	int mode;
	unsigned char mode;

	mib_get(MIB_WLAN_MODE, (void *)&mode);	
	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	strVal = websGetVar(wp, T("triggerPBC"), T(""));
	if (strVal[0]) {
		mib_get(MIB_WSC_DISABLE, (void *)&intVal);
		if (intVal) {
			intVal = 0;
			mib_set(MIB_WSC_DISABLE, (void *)&intVal);
			mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);	// update to flash	
			system("echo 1 > /var/wps_start_pbc");
#ifndef NO_ACTION
			run_init_script("bridge");
#endif			
		}
		else {		
//#ifndef NO_ACTION		
			//sprintf(tmpbuf, "%s -sig_pbc", _WSC_DAEMON_PROG);
			//system(tmpbuf);
			va_cmd(_WSC_DAEMON_PROG, 1, 1, "-sig_pbc");
//#endif
		}
		OK_MSG2(START_PBC_MSG, ((mode==AP_MODE) ? "client" : "AP"), submitUrl);
		return;
	}

	strVal = websGetVar(wp, T("triggerPIN"), T(""));
	if (strVal[0]) {
		int local_pin_changed = 0;		
		strVal = websGetVar(wp, T("localPin"), T(""));
		if (strVal[0]) {
			mib_get(MIB_WSC_PIN, (void *)tmpbuf);
			if (strcmp(tmpbuf, strVal)) {
				mib_set(MIB_WSC_PIN, (void *)strVal);
				local_pin_changed = 1;				
			}			
		}		
		mib_get(MIB_WSC_DISABLE, (void *)&intVal);
		if (intVal) {
			char localpin[100];
			intVal = 0;			
			mib_set(MIB_WSC_DISABLE, (void *)&intVal);
			mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);	// update to flash	
			system("echo 1 > /var/wps_start_pin");

#ifndef NO_ACTION
			if (local_pin_changed) {
				mib_get(MIB_WSC_PIN, (void *)localpin);
				sprintf(tmpbuf, "echo %s > /var/wps_local_pin", localpin);
				system(tmpbuf);
			}
			run_init_script("bridge");			
#endif			
		}
		else {		
//#ifndef NO_ACTION		
			if (local_pin_changed) {
				system("echo 1 > /var/wps_start_pin");
				
				mib_update(CURRENT_SETTING,CONFIG_MIB_ALL);					
				//run_init_script("bridge");
			}
			else {
				sprintf(tmpbuf, "%s -sig_start", _WSC_DAEMON_PROG);
				system(tmpbuf);
			}			
//#endif
		}
		OK_MSG2(START_PIN_MSG, ((mode==AP_MODE) ? "client" : "AP"), submitUrl);
		return;
	}
	
	strVal = websGetVar(wp, T("setPIN"), T(""));
	if (strVal[0]) {		
		strVal = websGetVar(wp, T("peerPin"), T(""));
		if (strVal[0]) {
			mib_get(MIB_WSC_DISABLE, (void *)&intVal);
			if (intVal) {
				intVal = 0;
				mib_set(MIB_WSC_DISABLE, (void *)&intVal);
				mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);	

				sprintf(tmpbuf, "echo %s > /var/wps_peer_pin", strVal);
				system(tmpbuf);

#ifndef NO_ACTION
				run_init_script("bridge");
#endif					
			}
			else {			
//#ifndef NO_ACTION
				//sprintf(tmpbuf, "iwpriv %s set_mib pin=%s", WLAN_IF, strVal);
				//system(tmpbuf);
				sprintf(tmpbuf, "pin=%s", strVal);
				va_cmd("/bin/iwpriv", 3, 1, WLAN_IF, "set_mib", tmpbuf);
//#endif
			}
			OK_MSG1(SET_PIN_MSG, submitUrl);			
			return;
		}
	}

	strVal = websGetVar(wp, T("disableWPS"), T(""));
	if ( !gstrcmp(strVal, T("ON")))
		intVal = 1;
	else
		intVal = 0;
	mib_set(MIB_WSC_DISABLE, (void *)&intVal);

#ifdef REVERSE_WPS_LED
//xl_yue:support wps_led for zte531b
	sprintf(tmpbuf, "echo %d > /proc/gpio",!(intVal));
	system(tmpbuf);
#endif	
	strVal = websGetVar(wp, T("localPin"), T(""));
	if (strVal[0])
		mib_set(MIB_WSC_PIN, (void *)strVal);

//	update_wps_configured(0);
	config_WLAN(ACT_RESTART);	
	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);	// update to flash
	
#ifndef NO_ACTION
	run_init_script("bridge");
#endif

	OK_MSG(submitUrl);
}

#endif




