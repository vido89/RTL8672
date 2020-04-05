
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <paths.h>
#include <signal.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/param.h>
#ifdef EMBED
#include <config/autoconf.h>
#include "./LINUX/options.h"
#else
#include "../../../../config/autoconf.h"
#endif

/* SYSLOG_NAMES defined to pull some extra junk from syslog.h */
#define SYSLOG_NAMES
#include <sys/syslog.h>
#include <sys/uio.h>
#include <sys/stat.h>

#define FALSE   ((int) 0)
#define TRUE    ((int) 1)
/* Path for the file where all log messages are written */
#define __LOG_FILE "/var/log/messages"

/* Path to the unix socket */
static char lfile[MAXPATHLEN];

static char *logFilePath = __LOG_FILE;
#ifdef EMBED
static int logFileMaxSize = 16384;
#endif

static int logLevel = LOG_DEBUG;
/* interval between marks in seconds */
#ifdef EMBED
static int MarkInterval = 0;
#else
static int MarkInterval = 20 * 60;
#endif

/* localhost's name */
// Kaohj
//static char LocalHostName[64];

#ifdef SYSLOG_REMOTE_LOG
#include <netinet/in.h>

struct RemoteLogInfo {
	int fd;					/* udp socket for logging to remote host */
	char *hostname;			/* where do we log? */
	int port;				/* what port to log to? */
	int filterlevel;		/* (7 - LOG_xxx) only logs messages at priority LOG_xxx and higher.
	                         * 0 = log all. 8 = log none
							 */
	int enable;
};

#ifdef CONFIG_USER_MGMT_MGMT
#define NUM_REMOTE_HOSTS 2
#else
#define NUM_REMOTE_HOSTS 1
#endif

static struct RemoteLogInfo remote_log_info[NUM_REMOTE_HOSTS] = {
  { -1, 0, 514, LOG_DEBUG, 0 }
};

static int remote_logging_initialised = FALSE;
static int remote_logging = FALSE;
static int local_logging = FALSE;
static int init_RemoteLog(void);
#endif

#define MAXLINE         1024            /* maximum line length */

/* try to open up the specified device */
int device_open(char *device, int mode)
{
	int m, f, fd = -1;

	m = mode | O_NONBLOCK;

	/* Retry up to 5 times */
	for (f = 0; f < 5; f++)
		if ((fd = open(device, m, 0600)) >= 0)
			break;
	if (fd < 0)
		return fd;
	/* Reset original flags. */
	if (m != mode)
		fcntl(fd, F_SETFL, mode);
	return fd;
}

void perror_msg_and_die(const char *s, ...)
{
	va_list p;
	int err = errno;

	va_start(p, s);
	//vperror_msg(s, p);
	if (s==0) s="";
	fflush(stdout);
	vfprintf(stderr, s, p);
	if (*s) s = ": ";
	fprintf(stderr, "%s%s\n", s, strerror(err));
	va_end(p);
	exit(EXIT_FAILURE);
}
static void message (char *fmt, ...)
{
	int fd;
	struct flock fl;
	va_list arguments;

	fl.l_whence = SEEK_SET;
	fl.l_start  = 0;
	fl.l_len    = 1;

	if ((fd = device_open (logFilePath,
						   O_WRONLY | O_CREAT | O_NOCTTY | O_APPEND |
						   O_NONBLOCK)) >= 0) {
		fl.l_type = F_WRLCK;
		fcntl (fd, F_SETLKW, &fl);
		va_start (arguments, fmt);
		vdprintf (fd, fmt, arguments);
		va_end (arguments);
		fl.l_type = F_UNLCK;
		fcntl (fd, F_SETLKW, &fl);
#ifdef EMBED
		{
			struct stat st;
			char buf[128];

			if (fstat(fd, &st) != -1 && st.st_size >= logFileMaxSize) {
				snprintf(buf, sizeof(buf), "%s.old", logFilePath);
				rename(logFilePath, buf);
			}
		}
#endif
		close (fd);
	} else {
		/* Always send console messages to /dev/console so people will see them. */
		if ((fd = device_open (_PATH_CONSOLE,
							   O_WRONLY | O_NOCTTY | O_NONBLOCK)) >= 0) {
			va_start (arguments, fmt);
			vdprintf (fd, fmt, arguments);
			va_end (arguments);
			close (fd);
		} else {
			fprintf (stderr, "Bummer, can't print: ");
			va_start (arguments, fmt);
			vfprintf (stderr, fmt, arguments);
			fflush (stderr);
			va_end (arguments);
		}
	}
}

static void logMessage (int pri, char *msg)
{
	time_t now;
	char *timestamp;
	//static char res[20] = "";
	static char res[20] = "", logpri[20] = "";
// Kaohj
///*
	CODE *c_pri, *c_fac;

	if (pri != 0) {
		for (c_fac = facilitynames;
				c_fac->c_name && !(c_fac->c_val == LOG_FAC(pri) << 3); c_fac++);
		for (c_pri = prioritynames;
				c_pri->c_name && !(c_pri->c_val == LOG_PRI(pri)); c_pri++);
// Jenny
/*
		if (c_fac->c_name == NULL || c_pri->c_name == NULL)
			snprintf(res, sizeof(res), "<%d>", pri);
		else
			snprintf(res, sizeof(res), "%s.%s", c_fac->c_name, c_pri->c_name);
	}
*/
		if (c_fac->c_name == NULL || c_pri->c_name == NULL)
			snprintf(logpri, sizeof(res), "<%d>", pri);
		else
			snprintf(logpri, sizeof(res), "%s.%s", c_fac->c_name, c_pri->c_name);
	}

	time(&now);
	if (strlen(msg) < 21 || msg[3] != ' ' || msg[6] != ' ' ||
			msg[9] != ':' || msg[12] != ':' || msg[15] != ' ') {
		time(&now);
		timestamp = ctime(&now) + 4;
		timestamp[20] = '\0';
	} else {
		timestamp = msg;
		timestamp[20] = '\0';
		msg += 21;
	}

	/* todo: supress duplicates */

#ifdef SYSLOG_REMOTE_LOG
	/* send message to remote logger */
	if (!remote_logging_initialised) {
		if (init_RemoteLog() == 0)
		  remote_logging_initialised = TRUE;
	}
	if (remote_logging) {
		int i;
		static const int IOV_COUNT = 2;
		struct iovec iov[IOV_COUNT];
		struct iovec *v = iov;

		memset(&res, 0, sizeof(res));
		snprintf(res, sizeof(res), "<%d>", pri);
		v->iov_base = res ;
		v->iov_len = strlen(res);
		v++;

		v->iov_base = msg;
		v->iov_len = strlen(msg);		   

		for (i = 0; i < NUM_REMOTE_HOSTS; i++) {
			if (!remote_log_info[i].enable || remote_log_info[i].fd == -1) {
				continue;
			}
			if (7 - LOG_PRI(pri) < remote_log_info[i].filterlevel) {
				continue;
			}
			writev_retry:
			if ( -1 == writev(remote_log_info[i].fd, iov, IOV_COUNT)) {
				if (errno == EINTR) goto writev_retry;
				message("%d|%s|%s|slogd: cannot write to remote file handle on " 
						"%s:%d - %d\n", pri, timestamp, logpri, remote_log_info[i].hostname, remote_log_info[i].port, errno);
				close(remote_log_info[i].fd);
				remote_log_info[i].fd = -1;
				remote_logging_initialised = FALSE;
			}
		}
	}
	if (local_logging == TRUE)
#endif
		/* now spew out the message to wherever it is supposed to go */
#ifdef EMBED
		//message("<%d> %s %s\n", pri, timestamp, msg);
		message("%d|%s|%s|%s\n", pri, timestamp, logpri, msg);
#else
		// Kaohj
		//message("%s %s %s %s\n", timestamp, LocalHostName, res, msg);
		message("%s %s %s\n", timestamp, LocalHostName, res, msg);
#endif
}

// Kaohj -- write pid file
#ifdef EMBED
static char syslogd_pidfile[] = "/var/run/syslogd.pid";
static void log_pid()
{
	FILE *f;
	pid_t pid;
	char *pidfile = syslogd_pidfile;

	pid = getpid();
	if((f = fopen(pidfile, "w")) == NULL)
		return;
	fprintf(f, "%d\n", pid);
	fclose(f);
}
#endif

static void quit_signal(int sig)
{
	logMessage(LOG_SYSLOG | LOG_INFO, "System log daemon exiting.");
	unlink(lfile);

#ifdef EMBED
	unlink(syslogd_pidfile);
#endif
	exit(TRUE);
}

static void domark(int sig)
{
	if (MarkInterval > 0) {
		logMessage(LOG_SYSLOG | LOG_INFO, "-- MARK --");
		alarm(MarkInterval);
	}
}

/* This must be a #define, since when DODEBUG and BUFFERS_GO_IN_BSS are
 * enabled, we otherwise get a "storage size isn't constant error. */
static int serveConnection (char* tmpbuf, int n_read)
{
	int    pri_set = 0;
	char  *p = tmpbuf;

	/* SOCK_DGRAM messages do not have the terminating NUL,  add it */
	if (n_read > 0)
		tmpbuf[n_read] = '\0';

	while (p < tmpbuf + n_read) {

		int           pri = (LOG_USER | LOG_NOTICE);
		char          line[ MAXLINE + 1 ];
		char         *q = line;

		while (q < &line[ sizeof (line) - 1 ]) {
			if (!pri_set && *p == '<') {
			/* Parse the magic priority number. */
				pri = 0;
				while (isdigit (*(++p))) {
					pri = 10 * pri + (*p - '0');
				}
				if((pri&0x07) > logLevel)
					return n_read;
				if (pri & ~(LOG_FACMASK | LOG_PRIMASK)){
					pri = (LOG_USER | LOG_NOTICE);
				}
				pri_set = 1;
			} else if (*p == '\0') {
				pri_set = 0;
				*q = *p++;
				break;
			} else if (*p == '\n') {
				*q++ = ' ';
			} else if (iscntrl(*p) && (*p < 0177)) {
				*q++ = '^';
				*q++ = *p ^ 0100;
			} else {
				*q++ = *p;
			}
			p++;
		}
		*q = '\0';
		p++;
		/* Now log it */
		if (q > line)
			logMessage (pri, line);
	}
	return n_read;
}

#ifdef SYSLOG_REMOTE_LOG
static void verror_msg(const char *s, va_list p)
{
	fflush(stdout);
	fprintf(stderr, "slogd: ");
	vfprintf(stderr, s, p);
}

static void herror_msg_and_die(const char *s, ...)
{
	va_list p;

	va_start(p, s);
	if (s == 0)
		s = "";
	verror_msg(s, p);
	if (*s)
		fputs(": ", stderr);
	herror("");
	va_end(p);
	exit(EXIT_FAILURE);
}

static struct hostent *xgethostbyname(const char *name)
{
	struct hostent *retval;

	if ((retval = gethostbyname(name)) == NULL)
		herror_msg_and_die("%s", name);

	return retval;
}

static void error_msg_and_die(const char *s, ...)
{
	va_list p;

	va_start(p, s);
	verror_msg(s, p);
	va_end(p);
	putc('\n', stderr);
	exit(EXIT_FAILURE);
}

static int init_RemoteLog (void)
{
	int i;
	struct sockaddr_in remoteaddr;
	struct hostent *hostinfo;
	int len = sizeof(remoteaddr);

	remote_logging = FALSE;

	for (i = 0; i < NUM_REMOTE_HOSTS; i++) {
		if (!remote_log_info[i].enable || remote_log_info[i].hostname == 0 ||
			remote_log_info[i].port == 0 || remote_log_info[i].filterlevel > 7) {
			continue;
		}
		memset(&remoteaddr, 0, len);

		remote_log_info[i].fd = socket(AF_INET, SOCK_DGRAM, 0);

		if (remote_log_info[i].fd < 0) {
			error_msg_and_die("slogd: cannot create socket");
		}

		hostinfo = xgethostbyname(remote_log_info[i].hostname);
		remoteaddr.sin_family = AF_INET;
		remoteaddr.sin_addr = *(struct in_addr *) *hostinfo->h_addr_list;
		remoteaddr.sin_port = htons(remote_log_info[i].port);

		/* 
			 Since we are using UDP sockets, connect just sets the default host and port 
			 for future operations
		*/
		if (0 != (connect(remote_log_info[i].fd, (struct sockaddr *) &remoteaddr, len))) {
			//printf("cannot connect to remote host %s:%d", remote_log_info[i].hostname, remote_log_info[i].port);
			close(remote_log_info[i].fd);
			remote_log_info[i].fd = -1;
			return -1;	
		}
		remote_logging = TRUE;
	}
	return 0;
}
#endif

static void doSyslogd (void)
{
	struct sockaddr_un sunx;
	socklen_t addrLength;

	int sock_fd;
	fd_set fds;
	char buf[128];

	/* Set up signal handlers. */
	signal (SIGINT,  quit_signal);
	signal (SIGTERM, quit_signal);
	signal (SIGQUIT, quit_signal);
	signal (SIGHUP,  SIG_IGN);
	signal (SIGCHLD,  SIG_IGN);
#ifdef SIGCLD
	signal (SIGCLD,  SIG_IGN);
#endif
	signal (SIGALRM, domark);
	alarm (MarkInterval);

	/* Create the syslog file so realpath() can work. */
	if (realpath (_PATH_LOG, lfile) != NULL)
		unlink (lfile);

	memset (&sunx, 0, sizeof (sunx));
	sunx.sun_family = AF_UNIX;
	strncpy (sunx.sun_path, lfile, sizeof (sunx.sun_path));
	if ((sock_fd = socket (AF_UNIX, SOCK_DGRAM, 0)) < 0)
		perror_msg_and_die ("Couldn't get file descriptor for socket " _PATH_LOG);

	addrLength = sizeof (sunx.sun_family) + strlen (sunx.sun_path);
	if (bind(sock_fd, (struct sockaddr *) &sunx, addrLength) < 0)
		perror_msg_and_die ("Could not connect to socket " _PATH_LOG);

	if (chmod (lfile, 0666) < 0)
		perror_msg_and_die ("Could not set permission on " _PATH_LOG);



	// Kaohj
	//logMessage (LOG_SYSLOG | LOG_INFO, "syslogd started: " BB_BANNER);
	logMessage (LOG_SYSLOG | LOG_INFO, "syslogd started");
	// Kaohj -- add symbolic link /var/tmp/messages and /var/tmp/messages.old
	// that can be seen by the remote login user
	snprintf(buf, sizeof(buf), "%s.old", logFilePath);
	if (symlink(logFilePath, "/var/tmp/messages")==-1)
		printf("link fail: /var/tmp/messages -> %s\n", logFilePath);
	if (symlink(buf, "/var/tmp/messages.old")==-1)
		printf("link fail: /var/tmp/messages.old -> %s\n", buf);

	for (;;) {

		FD_ZERO (&fds);
		FD_SET (sock_fd, &fds);

		if (select (sock_fd+1, &fds, NULL, NULL, NULL) < 0) {
			if (errno == EINTR) {
				/* alarm may have happened. */
				continue;
			}
			perror_msg_and_die ("select error");
		}

		if (FD_ISSET (sock_fd, &fds)) {
		       int   i;
		       // Kaohj
		       //RESERVE_BB_BUFFER(tmpbuf, BUFSIZ + 1);
		       char tmpbuf[BUFSIZ + 1];

		       memset(tmpbuf, '\0', BUFSIZ+1);
		       if ( (i = recv(sock_fd, tmpbuf, BUFSIZ, 0)) > 0) {
			       serveConnection(tmpbuf, i);
		       } else {
			       perror_msg_and_die ("UNIX socket error");
		       }
		       // Kaohj
		       //RELEASE_BB_BUFFER (tmpbuf);
		}/* FD_ISSET() */
	} /* for main loop */
}

int main(int argc, char **argv)
{
	int opt;
#if ! defined(__uClinux__)
	int doFork = TRUE;
#endif
	char *p;

	/* do normal option parsing */
	//while ((opt = getopt(argc, argv, "s:m:nO:R:LC")) > 0) {
	while ((opt = getopt(argc, argv, "s:m:l:nO:R:LC")) > 0) {
		switch (opt) {
			case 's':
				logFileMaxSize=atoi(optarg);
				break;
			case 'm':
				MarkInterval = atoi(optarg) * 60;
				break;
			case 'l':
				logLevel = atoi(optarg);
				break;
			case 'n':
#if ! defined(__uClinux__)
				doFork = FALSE;
#endif
				break;
#ifdef SYSLOG_REMOTE_LOG
			case 'R':
				remote_log_info[0].hostname = strdup(optarg);
				if ( (p = strchr(remote_log_info[0].hostname, ':'))){
					remote_log_info[0].port = atoi(p+1);
					*p = '\0';
				}
				remote_log_info[0].enable = 1;
				remote_log_info[0].filterlevel = 7 - logLevel;
				remote_logging = TRUE;
				break;
			case 'L':
				local_logging = TRUE;
				break;
#endif
			// Kaohj
			/*
			case 'O':
				logFilePath = xstrdup(optarg);
				break;
			default:
				show_usage();
			*/
		}
	}


	/* Store away localhost's name before the fork */
// Kaohj
/*
	gethostname(LocalHostName, sizeof(LocalHostName));
	if ((p = strchr(LocalHostName, '.'))) {
		*p++ = '\0';
	}
*/

	umask(0);

#if ! defined(__uClinux__)
	if (doFork == TRUE) {
		if (daemon(0, 1) < 0)
			perror_msg_and_die("daemon");
	}
#endif
#ifdef EMBED
	log_pid();
#endif
	doSyslogd();

	return EXIT_SUCCESS;
}


