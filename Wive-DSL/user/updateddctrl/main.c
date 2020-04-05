
#include <stdio.h>
#include <net/if.h>
#include <netdb.h>
#include <string.h>
//#include "../boa/src/LINUX/mib.h"
//#include "../boa/src/LINUX/utility.h"
#include <rtk/sysconfig.h>
#include <config/autoconf.h>

char g_InterfaceName[10];

int main (int argc, char** argv)	
{	   
	unsigned int entryNum, i;
	char InterfaceName[10];
	char account[70];
	MIB_CE_DDNS_T Entry;	
	
	//printf("updateddctrl start .....\n");	
   	
	if (argc != 2)
   	{
      		printf("Usage: updateddctrl InterfaceName>\n");
      		printf("Example: updateddctrl ppp0\n");
      		printf("Example: updateddctrl br0\n");
      		exit(0);
   	}
	
	// Save the interface names	
	strcpy(InterfaceName, argv[1]);		

	//init. mib
	/*
	if ( mib_init() == 0 ) {
		printf("[upnpctrl] Initialize MIB failed!\n");
		return;
	}
	*/	
	
	cmd_ddnsctrl(InterfaceName);			
	return 1;
}
