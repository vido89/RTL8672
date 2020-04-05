/*
 *      Web server handler routines for shaper stuff
 *
 */


/*-- System inlcude files --*/
#include <net/if.h>
#include <signal.h>
#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../../uClibc/include/linux/autoconf.h"
#endif
#include "options.h"

#ifdef CONFIG_SHAPER
/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"

void initSHAPERSetupPage(webs_t wp)
{
}

void formSHAPERSetup(webs_t wp, char_t *path, char_t *query)
{
    char_t  *str, *submitUrl;

    str = websGetVar(wp, T("shaper_up"), T(""));
    if (!mib_set(MIB_TOTAL_BANDWIDTH_UP, (void *)str)) {
        printf("Set shaper_up mib error!");
    }else{
	va_cmd("/bin/flash", 3, 1, "set",  "TOTAL_BANDWIDTH_UP", str);	
#ifdef WEB_DEBUG_MSG
        printf("shaper_up value : %s\n", str);
#endif
    }

    str = websGetVar(wp, T("shaper_limit_up"), T(""));
    if (!mib_set(MIB_TOTAL_BANDWIDTH_LIMIT_UP, (void *)str)) {
        printf("Set shaper_limit_up mib error!");
    }else{
	va_cmd("/bin/flash", 3, 1, "set",  "TOTAL_BANDWIDTH_LIMIT_UP", str);	
#ifdef WEB_DEBUG_MSG
        printf("shaper_limit_up value : %s\n", str);
#endif
    }

    str = websGetVar(wp, T("shaper_down"), T(""));
    if (!mib_set(MIB_TOTAL_BANDWIDTH_DOWN, (void *)str)) {
        printf("Set shaper_down mib error!");
    }else{
	va_cmd("/bin/flash", 3, 1, "set",  "TOTAL_BANDWIDTH_DOWN", str);	
#ifdef WEB_DEBUG_MSG
        printf("shaper_down value : %s\n", str);
#endif
    }

    str = websGetVar(wp, T("shaper_limit_down"), T(""));
    if (!mib_set(MIB_TOTAL_BANDWIDTH_LIMIT_DOWN, (void *)str)) {
        printf("Set shaper_limit_down mib error!");
    }else{
	va_cmd("/bin/flash", 3, 1, "set",  "TOTAL_BANDWIDTH_LIMIT_DOWN", str);	
#ifdef WEB_DEBUG_MSG
        printf("shaper_limit_down value : %s\n", str);
#endif
    }

    str = websGetVar(wp, T("shaper_high_prio_ports"), T(""));
    if (!mib_set(MIB_HIGH_PRIO_PORTS, (void *)str)) {
        printf("Set shaper_high_prio_ports mib error!");
    }else{
	va_cmd("/bin/flash", 3, 1, "set",  "HIGH_PRIO_PORTS", str);	
#ifdef WEB_DEBUG_MSG
        printf("shaper_high_prio_ports value : %s\n", str);
#endif
    }

    str = websGetVar(wp, T("shaper_low_prio_ports"), T(""));
    if (!mib_set(MIB_LOW_PRIO_PORTS, (void *)str)) {
        printf("Set shaper_low_prio_ports mib error!");
    }else{
	va_cmd("/bin/flash", 3, 1, "set",  "LOW_PRIO_PORTS", str);	
#ifdef WEB_DEBUG_MSG
        printf("shaper_high_prio_ports value : %s\n", str);
#endif
    }

	
    str = websGetVar(wp, T("shaper_ip2p_enable"), T(""));
    if (!mib_set(MIB_SHAPER_IP2P_ENABLE, (void *)str)) {
        printf("Set shaper_ip2p_enable mib error!");
    }else{
	va_cmd("/bin/flash", 3, 1, "set",  "SHAPER_IP2P_ENABLE", str);	
#ifdef WEB_DEBUG_MSG
        printf("shaper_ip2p_enable value : %s\n", str);
#endif
}

    str = websGetVar(wp, T("shaper_l7_enable"), T(""));
    if (!mib_set(MIB_SHAPER_L7_ENABLE, (void *)str)) {
        printf("Set shaper_l7_enable mib error!");
    }else{
	va_cmd("/bin/flash", 3, 1, "set",  "SHAPER_L7_ENABLE", str);	
#ifdef WEB_DEBUG_MSG
        printf("shaper_l7_enable value : %s\n", str);
#endif
}

    str = websGetVar(wp, T("shaper_in_enable"), T(""));
    if (!mib_set(MIB_SHAPER_IN_ENABLE, (void *)str)) {
        printf("Set shaper_in_enable mib error!");
    }else{
	va_cmd("/bin/flash", 3, 1, "set",  "SHAPER_IN_ENABLE", str);	
#ifdef WEB_DEBUG_MSG
        printf("shaper_in_enable value : %s\n", str);
#endif
}

    str = websGetVar(wp, T("shaper_enable"), T(""));
    if (!mib_set(MIB_SHAPER_ENABLE, (void *)str)) {
        printf("Set shaper_enable mib error!");
    }else{
	va_cmd("/bin/flash", 3, 1, "set",  "SHAPER_ENABLE", str);	
#ifdef WEB_DEBUG_MSG
        printf("shaper_enable value : %s\n", str);
#endif
	printf("Call shaper helper!\n");
        va_cmd("/bin/service", 3, 0, "shaper", "restart", "&");
        va_cmd("/bin/service", 3, 0, "iptables", "start", "&");
    }

    submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page
    if (submitUrl[0])
            websRedirect(wp, submitUrl);
            else
            websDone(wp, 200);
    return;
}
#endif
