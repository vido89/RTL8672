/*****************************************************************************/

/*
 *	login.c -- simple login program.
 *
 *	(C) Copyright 1999-2001, Greg Ungerer (gerg@snapgear.com).
 * 	(C) Copyright 2001, SnapGear Inc. (www.snapgear.com) 
 * 	(C) Copyright 2000, Lineo Inc. (www.lineo.com) 
 *
 *	Made some changes and additions Nick Brok (nick@nbrok.iaehv.nl).
 */

/*****************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <getopt.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <sys/utsname.h>
// Kaohj
#include <sys/signal.h>
#include <config/autoconf.h>  
#ifndef __UC_LIBC__
#include <crypt.h>
#endif
#ifdef OLD_CONFIG_PASSWORDS
#include <crypt_old.h>
#endif
#include <sys/types.h>
#include <pwd.h>
#include <syslog.h>

#include <rtk/options.h>
#include <rtk/mib.h>
#include <rtk/sysconfig.h>

/*****************************************************************************/

//xl_yue
#define	TELNET_LOGIN_CONTROL
//xl_yue
#ifdef	TELNET_LOGIN_CONTROL
#define TELNET_MAX_LOGIN 3	//telnet max can login error 2 times.when login error for three times,downlink telent
#endif
/* Delay bad password exit.
 * 
 * This doesn't really accomplish anything I guess..
 * as other connections can be made in the meantime.. and
 * someone attempting a brute force attack could kill their
 * connection if a delay is detected etc.
 *
 * -m2 (20000201)
 */
#define DELAY_EXIT	1

/*****************************************************************************/

char *version = "v1.0.2";

char usernamebuf[128];

/*****************************************************************************/

static inline char *getrealpass(const char *user) {
	struct passwd *pwp;
	
	pwp = getpwnam(user);
	if (pwp == NULL)
		return NULL;
	return pwp->pw_passwd;
}

static inline char *getrealshell(const char *user) {
	struct passwd *pwp;
	
	pwp = getpwnam(user);
	if (pwp == NULL)
		return NULL;
	return pwp->pw_shell;
}

/*****************************************************************************/

int main(int argc, char *argv[])
{
	char	*user;
	char	*realpwd, *realshell, *gotpwd, *cpwd;
	char *host;
	int flag;
	int persist;
	int	i;
#ifdef TELNET_LOGIN_CONTROL
	//added by xl_yue
	int login_num = 0;
#endif
    // Kaohj
    host=0; persist=0;
    while ((flag = getopt(argc, argv, "h:p")) != EOF) {
        switch (flag) {
        case 'h':
            host = optarg;
            break;
        // Kaohj added
        case 'p': // for persistence login (usually use for console login)
            signal(SIGINT, SIG_IGN);
            persist = 1;
            break;
        default:
			fprintf(stderr,
			"login [OPTION]... [username]\n"
			"\nBegin a new session on the system\n\n"
			"Options:\n"
			"\t-h\t\tName of the remote host for this login.\n"
			);
        }
    }

	chdir("/");
	
	// Kaohj
	while (1) {
	if (optind < argc) {
		user = argv[optind];
	} else {
		printf("login: ");
		fflush(stdout);
		if (fgets(usernamebuf, sizeof(usernamebuf), stdin) == NULL)
			exit(0);
		user = strchr(usernamebuf, '\n');
		*user = '\0';
		user = &usernamebuf[0];
		cpwd = strchr(user, '\n');
		if (cpwd != NULL)
			*cpwd = '\0';
	}

	gotpwd = getpass("Password: ");
	realpwd = getrealpass(user);
	// Kaohj added, 2005/12/14
#ifndef TELNET_CLI
	realshell = getrealshell(user);
#else
	realshell = "cli";
#endif
	if (gotpwd && realpwd) {
		int good = 0;

		openlog("login", 0, LOG_AUTHPRIV);

		cpwd = crypt(gotpwd, realpwd);
		if (strcmp(cpwd, realpwd) == 0) 
			good++;
			
		if (good) {
			syslog(LOG_INFO, "Authentication successful for %s from %s\n",
					user, host ? host : "unknown");
#ifdef EMBED
			// Kaohj added, 2005/12/14
			if (realshell) {
				#ifdef EMBED
				FILE *fp;
				if (!persist) {
					// Kaohj --- check daemon
					fp = fopen("/var/run/cli.pid", "r");
					if (fp) {
						// Allow only one CLI process
						printf("CLI busy !!\n");
						fclose(fp);
						return(0);
					}
				}
#endif
				if (strstr(realshell, "cli")) {
					if (persist)
						execlp(realshell, realshell, "-c", "-u", user, NULL);
					else
						execlp(realshell, realshell, "-u", user, NULL);
				}
				else
					execlp(realshell, realshell, NULL);
			}
			else
				execlp("sh", "sh", NULL);
#else
			execlp("sh", "sh", "-t", NULL);
#endif
		} else {
			syslog(LOG_ERR, "Authentication attempt failed for %s from %s\n",
					user, host ? host : "unknown");
			sleep(DELAY_EXIT);
		}
	}
	// Kaohj
	if (!persist)

	//added by xl_yue
#ifdef TELNET_LOGIN_CONTROL
		if((++login_num) < TELNET_MAX_LOGIN) {
			printf("Login incorrect\n\n");
			continue;
		}
		else
#endif
			break;
	} // end of while (1)

	return(0);
}

/*****************************************************************************/
