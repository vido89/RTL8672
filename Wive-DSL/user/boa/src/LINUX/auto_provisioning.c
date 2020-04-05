#ifdef AUTO_PROVISIONING
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <stdarg.h>
#include <semaphore.h>
#include "adslif.h"

#define AUTO_PROVISIONING_INTERVAL	24 * 60 * 60	/* start auto-provisioning every 24 hours */

int do_cmd(const char *filename, char *argv [], int dowait)
{
	pid_t pid, wpid;
	int stat, st;
	
	if((pid = vfork()) == 0) {
		/* the child */
		char *env[3];
		
		signal(SIGINT, SIG_IGN);
		argv[0] = (char *)filename;
		env[0] = "PATH=/bin:/usr/bin:/etc:/sbin:/usr/sbin";
		env[1] = NULL;

		execve(filename, argv, env);

		printf("exec %s failed\n", filename);
		_exit(2);
	} else if(pid > 0) {
		if (!dowait)
			stat = 0;
		else {
			/* parent, wait till rc process dies before spawning */
			while ((wpid = wait(&stat)) != pid)
				if (wpid == -1 && errno == ECHILD) { /* see wait(2) manpage */
					stat = 0;
					break;
				}
		}
	} else if(pid < 0) {
		printf("fork of %s failed\n", filename);
		stat = -1;
	}
	//st = WEXITSTATUS(stat);
	return st;
}

void va_cmd(const char *cmd, int num, int dowait, ...)
{
	va_list ap;
	int k;
	char *s;
	char *argv[19];
	
	va_start(ap, dowait);
	
	for (k=0; k<num; k++)
	{
		s = va_arg(ap, char *);
		argv[k+1] = s;
	}
	
	argv[k+1] = NULL;
	do_cmd(cmd, argv, dowait);
	va_end(ap);
}

static char adslDevice[] = "/dev/adsl0";
static FILE* adslFp = NULL;
static char open_adsl_drv(void)
{
	if ((adslFp = fopen(adslDevice, "r")) == NULL) {
		printf("ERROR: failed to open %s, error(%s)\n",adslDevice, strerror(errno));
		return 0;
	};
	return 1;
}

static void close_adsl_drv(void)
{
	if(adslFp)
		fclose(adslFp);

	adslFp = NULL;
}

char adsl_drv_get(unsigned int id, void *rValue, unsigned int len)
{
#ifdef EMBED
	if(open_adsl_drv()) {
		obcif_arg	myarg;
	    	myarg.argsize = (int) len;
	    	myarg.arg = (int) (rValue);

		if (ioctl(fileno(adslFp), id, &myarg) < 0) {
			close_adsl_drv();
			return 0;
	       };

		close_adsl_drv();
		return 1;
	}
#endif
	return 0;
}

unsigned char serverIP[20];
// Jenny, autoprovisioning
static void auto_Provisioning(int sig)
{
	char buf[64];
	Modem_LinkSpeed vLs;
	vLs.upstreamRate=0;
	char firmwareUpdateRequest[] = "FirmwareUpdateRequest.xml";
	char url[256], output[256];
	int len;

	// wait until showtime
	while (!adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs,
		RLCM_GET_LINK_SPEED_SIZE) || vLs.upstreamRate == 0) {
		usleep(1000000);
	}
	usleep(1000000);
	printf("Start Autoprovisioning......\n");

	strcpy(url, serverIP);
	strcat(url, "/");
	strcat(url, firmwareUpdateRequest);
	len = strlen(url);
	url[len] = '\0';
	strcpy(output, "/tmp/");
	strcat(output, firmwareUpdateRequest);
	len = strlen(output);
	output[len] = '\0';
	
	va_cmd("/bin/wget", 5, 1, "--http-user=admin", "--http-passwd=system", url, "-O", output);
	va_cmd("/bin/parser", 1, 1, serverIP);
		
	signal(sig, auto_Provisioning);
	alarm(AUTO_PROVISIONING_INTERVAL);
}

int main(int argc, char **argv)
{
	sem_t sem;
	int len;

	strcpy(serverIP, argv[1]);
	len = strlen(serverIP);
	serverIP[len] = '\0';

	sem_init(&sem, 0, 0);
  
	/* Establish a handler for SIGALRM signals. */
	signal (SIGALRM, auto_Provisioning);

	/* Set an alarm to go off in a little while. */
	alarm (1);

	/* Infinite waiting */
	while(1){
		sem_wait(&sem);
	}

	return EXIT_SUCCESS;
}
#endif
