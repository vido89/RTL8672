#include <sys/reboot.h>

#include <semaphore.h>
#include "utility.h"

sem_t semSave;
char *g_post_file_name;
int g_startPos;

// Save/reboot thread startup function
void saveD (void)
{
	// wait for user save/reboot
	sem_wait(&semSave);
	
	// Jenny, disconnect PPP before rebooting
	stopPPP();
	sleep(2);
#if (defined(CONFIG_RTL867X_NETLOG)  && defined (CONFIG_USER_NETLOGGER_SUPPORT))
           va_cmd("/bin/netlogger",1,1,"disable");
	   sleep(1);
#endif

#ifdef EMBED
	// Modified by Mason Yu
	if ( semSave.__sem_value == SEM_REBOOT ) {
		sleep(3);		
		cmd_reboot();		
	}
#if defined (ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
	if ( semSave.__sem_value == SEM_DEFAULT) {
		va_cmd("/bin/flash", 2, 1, "default", "cs");
		sleep(3);		
		cmd_reboot();		
	}
#endif
#ifdef WEB_UPGRADE
#ifdef UPGRADE_V1
	else if ( semSave.__sem_value == SEM_UPGRADE ){	
		sleep(3);								
		cmd_upload(g_post_file_name, g_startPos);		
		
		/* reboot the system */
		//reboot(RB_AUTOBOOT);			
		//cmd_reboot();					
	}
#endif
#ifdef UPGRADE_V2
	else if (semSave.__sem_value == SEM_UPGRADE_2) {
		cmd_upgrade_mode(0);
	}
#endif
#endif
	else {
		/* upgdate to flash */
		mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
		// Kaohj
		va_cmd("/bin/adslctrl",1,1,"disablemodemline");
		sleep(1);	
		sync();
		va_cmd("/bin/sarctl",1,1,"disable");
		va_cmd(IFCONFIG, 2, 1, (char *)ELANIF, "down");
		va_cmd(IFCONFIG, 2, 1, (char *)WLANIF, "down");
		/* reboot the system */
		reboot(RB_AUTOBOOT);
		//va_cmd("/bin/reboot", 0, 1);
	}
#endif
}

void startSaved()
{
	pthread_t ptSaveId;
	if (pthread_create( &ptSaveId, 0, &saveD, 0 )!=0)
		return;
}
