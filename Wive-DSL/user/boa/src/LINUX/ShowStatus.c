#include <sys/ioctl.h>
#include <netinet/if_ether.h>
#include <net/if.h>
#include "adsl_drv.h"	
#include <sys/types.h>
#include <unistd.h>

#include "utility.h"
#define RUNFILE "/var/run/ShowStatus_pid"

void StartShowStatusThread (void)
{
	char strbuf[64];

	while(1) {
		getAdslInfo(ADSL_GET_STATE, strbuf, sizeof(strbuf));
		printf("DSL Operational Status : %s\n", strbuf);
		
		sleep(2);	
	}
}

void main(int argc, char *argv[])
{
	pid_t s_pid;
	FILE *filep;
	
	s_pid   = getpid();
	filep = fopen(RUNFILE,"w+");

	fprintf(filep, "%d", s_pid);
	fclose(filep);
	
	StartShowStatusThread();
}
