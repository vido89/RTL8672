/*
 * main.c -- Main program for the GoAhead WebServer (LINUX version)
 *
 * Copyright (c) GoAhead Software Inc., 1995-2000. All Rights Reserved.
 *
 * See the file "license.txt" for usage and redistribution license requirements
 *
 * $Id: main.c,v 1.14 2008/07/17 09:37:52 kaohj Exp $
 */

/******************************** Description *********************************/

/*
 *	Main program for for the GoAhead WebServer. This is a demonstration
 *	main program to initialize and configure the web server.
 */

/********************************* Includes ***********************************/

#include	"../wsIntrn.h"
#include	<signal.h>
#include	<unistd.h> 
#include	<sys/types.h>
#include	<sys/wait.h>
#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../../uClibc/include/linux/autoconf.h"
#endif

#ifdef WEBS_SSL_SUPPORT
#include	"../websSSL.h"
#endif

#ifdef USER_MANAGEMENT_SUPPORT
void	formDefineUserMgmt(void);
#endif

/* Add by Dick Tam */
#include "webform.h"
#include "mib.h"
#include "utility.h"
#include "../msgq.h"
/* Add by Dick Tam end */

/*********************************** Locals ***********************************/
/*
 *	Change configuration here
 */

#ifdef EMBED
static char_t		*rootWeb = T("/web");			/* Root web directory */
#else
static char_t		*rootWeb = T("web");			/* Root web directory */
#endif
static char_t		*password = T("");				/* Security password */
static int			port = 80;						/* Server port */
static int			retries = 5;					/* Server port retries */
static int			finished;						/* Finished flag */

/****************************** Forward Declarations **************************/

static int 	initWebs();
static int 	initMsgQ();
static void	term(int);

/* Mark by Dick Tam */
/*
static int	aspTest(int eid, webs_t wp, int argc, char_t **argv);
static void formTest(webs_t wp, char_t *path, char_t *query);
*/
/* Mark by Dick Tam end */
static int  websHomePageHandler(webs_t wp, char_t *urlPrefix, char_t *webDir,
				int arg, char_t *url, char_t *path, char_t *query);
extern void defaultErrorHandler(int etype, char_t *msg);
extern void defaultTraceHandler(int level, char_t *buf);
#ifdef B_STATS
static void printMemStats(int handle, char_t *fmt, ...);
static void memLeaks();
#endif

const char * const BRIDGE_IF = T("br0");
const char * const ELAN_IF = T("eth0");
const char * const WLAN_IF = T("wlan0");

#define RUNFILE "/var/run/webs.pid"

/*********************************** Code *************************************/
/*
 *	Main -- entry point from LINUX
 */

int main(int argc, char** argv)
{
	int   msgqueue_id, pid;
	struct mymsgbuf qbuf;
	FILE *pidfile;
	
#if 0
#ifdef EMBED
	extern int flash_main(int argc, char** argv);
	extern int genscript_main(int argc, char** argv);

	// handle flash UI link
	if ( !strcmp(argv[0], "flash") ) {	
		flash_main(argc, argv);
		return 0;
	}
	else if ( !strcmp(argv[0], "genscript") ) {	
		genscript_main(argc, argv);
		return 0;
	}
#endif	
#endif

/*
 *	Initialize the memory allocator. Allow use of malloc and start 
 *	with a 60K heap.  For each page request approx 8KB is allocated.
 *	60KB allows for several concurrent page requests.  If more space
 *	is required, malloc will be used for the overflow.
 */
	bopen(NULL, (60 * 1024), B_USE_MALLOC);
	signal(SIGPIPE, SIG_IGN);
	// Kaohj, install signal handlers
	signal(SIGTERM, term);
	signal(SIGINT, term);

/*
 *	Write pidfile
 */
 	pid = (int)getpid();
	if ((pidfile = fopen(RUNFILE, "w"))) {
		fprintf(pidfile, "%d\n", pid);
		fclose(pidfile);
	}
	
/*
 *	Initialize the web server
 */
	if (initWebs() < 0) {
		return -1;
	}

// Kaohj
/*
 *	Initialize the System V message queue for IPC
 */
#ifdef EMBED
	if ((msgqueue_id=initMsgQ()) < 0) {
		return -1;
	}
#endif

#ifdef WEBS_SSL_SUPPORT
	websSSLOpen();
#endif

/*
 *	Basic event loop. SocketReady returns true when a socket is ready for
 *	service. SocketSelect will block until an event occurs. SocketProcess
 *	will actually do the servicing.
 */
	while (!finished) {
		if (socketReady(-1) || socketSelect(-1, 1000)) {
			socketProcess(-1);
		}
		if (peek_message(msgqueue_id, pid)) {
			read_message(msgqueue_id, &qbuf, pid);
			msgProcess(&qbuf);
			send_message(msgqueue_id, qbuf.mtype, qbuf.request, qbuf.mtext);
		}
		websCgiCleanup();
		emfSchedProcess();
	}

#ifdef WEBS_SSL_SUPPORT
	websSSLClose();
#endif

#ifdef USER_MANAGEMENT_SUPPORT
	umClose();
#endif

	// Kaohj
	unlink(RUNFILE);
	remove_queue(msgqueue_id);
/*
 *	Close the socket module, report memory leaks and close the memory allocator
 */
	websCloseServer();
	socketClose();
#ifdef B_STATS
	memLeaks();
#endif
	bclose();
	return 0;
}

/******************************************************************************/
/*
 *	Initialize the web server.
 */

static int initWebs()
{
	struct hostent	*hp;
	struct in_addr	intaddr;
	char			host[128], dir[128], webdir[128];
	char			*cp;
	char_t			wbuf[128];
#ifdef EMBED
	int noIp = FALSE;	// david
#endif

/*
 *	Initialize the socket subsystem
 */
	socketOpen();

#ifdef USER_MANAGEMENT_SUPPORT
/*
 *	Initialize the User Management database
 */
	umOpen();
	umRestore(T("umconfig.txt"));
#endif

/*
 *	Define the local Ip address, host name, default home page and the 
 *	root web directory.
 */
	if (gethostname(host, sizeof(host)) < 0) {
		printf("[webs] Can't get hostname!\n");
		error(E_L, E_LOG, T("Can't get hostname"));
		return -1;
	}
	
/*
 *	Dick Modified 
 *	use getInAddr to replace gethostbyname in ucLinux
 */
#ifdef EMBED
	if ( !getInAddr(BRIDGE_IF, IP_ADDR, (void *)&intaddr) ) {
		if ( !getInAddr(ELAN_IF, IP_ADDR, (void *)&intaddr) ) {
			printf("[webs] Can't get IP address!\n");
			error(E_L, E_LOG, T("Can't get IP address!"));
			noIp = TRUE;

			cp = inet_ntoa(intaddr);
			printf("[webs] IP address = %s\n",cp);
		}
	}
#else
	if ((hp = gethostbyname(host)) == NULL) {
		error(E_L, E_LOG, T("Can't get host address"));
		return -1;
	}
/*
 *	Dick Modified 
 */

	memcpy((char *) &intaddr, (char *) hp->h_addr_list[0],
		(size_t) hp->h_length);
#endif

#ifdef EMBED
/*
 *	Init MIB
 */
	if ( mib_init() == 0 ) {
		printf("[webs] Initialize MIB failed!\n");
		error(E_L, E_LOG, T("Initialize MIB failed!\n"));
		return -1;
	}


/*
 *	Set ../web as the root web. Modify this to suit your needs
 */
	sprintf(webdir, "%s", rootWeb);
#else
	getcwd(dir, sizeof(dir)); 
	if ((cp = strrchr(dir, '/'))) {
		*cp = '\0';
	}
	sprintf(webdir, "%s/%s", dir, rootWeb);
#endif	

/*
 *	Configure the web server options before opening the web server
 */
	websSetDefaultDir(webdir);
#ifdef EMBED
	if ( !noIp ) { // david
		cp = inet_ntoa(intaddr);
		printf("[webs] IP addr =%s\n",cp);
		ascToUni(wbuf, cp, min(strlen(cp) + 1, sizeof(wbuf)));
		websSetIpaddr(wbuf);
	}
#else
	cp = inet_ntoa(intaddr);
	ascToUni(wbuf, cp, min(strlen(cp) + 1, sizeof(wbuf)));
	websSetIpaddr(wbuf);
#endif
	ascToUni(wbuf, host, min(strlen(host) + 1, sizeof(wbuf)));
	websSetHost(wbuf);

/*
 *	Configure the web server options before opening the web server
 */
	websSetDefaultPage(T("default.asp"));
	websSetPassword(password);

/* 
 *	Open the web server on the given port. If that port is taken, try
 *	the next sequential port for up to "retries" attempts.
 */
	websOpenServer(port, retries);

/*
 * 	First create the URL handlers. Note: handlers are called in sorted order
 *	with the longest path handler examined first. Here we define the security 
 *	handler, forms handler and the default web page handler.
 */
	websUrlHandlerDefine(T(""), NULL, 0, websSecurityHandler, 
		WEBS_HANDLER_FIRST);
	websUrlHandlerDefine(T("/goform"), NULL, 0, websFormHandler, 0);
	websUrlHandlerDefine(T("/cgi-bin"), NULL, 0, websCgiHandler, 0);
	websUrlHandlerDefine(T(""), NULL, 0, websDefaultHandler, 
		WEBS_HANDLER_LAST); 

/*
 *	Init user management
 */
	set_user_profile();

/*
 *	ASP script procedures and form functions.
 */
	websAspDefine(T("getInfo"), getInfo);
	websAspDefine(T("getIndex"), getIndex);

	websAspDefine(T("portFwList"), portFwList);
	websAspDefine(T("ipPortFilterList"), ipPortFilterList);
	websAspDefine(T("macFilterList"), macFilterList);

	websAspDefine(T("atmVcList"), atmVcList);	
	websAspDefine(T("atmVcList2"), atmVcList2);	
	websAspDefine(T("wanConfList"), wanConfList);	

	websFormDefine(T("formTcpipLanSetup"), formTcpipLanSetup);	// TCP/IP Lan Setting Form

	websFormDefine(T("formPortFw"), formPortFw);					// Firewall Porting forwarding Setting Form
	websFormDefine(T("formFilter"), formFilter);					// Firewall IP, Port, Mac Filter Setting Form
	websFormDefine(T("formDMZ"), formDMZ);						// Firewall DMZ Setting Form

	websFormDefine(T("formPasswordSetup"), formPasswordSetup);	// Management Password Setting Form
	websFormDefine(T("formUpload"), formUpload);					// Management Upload Firmware Setting Form
	websFormDefine(T("formSaveConfig"), formSaveConfig);			// Management Upload/Download Configuration Setting Form
	websFormDefine(T("formSnmpConfig"), formSnmpConfig);			// SNMP Configuration Setting Form
	websFormDefine(T("formAdslDrv"), formAdslDrv);				// ADSL Driver Setting Form
	websFormDefine(T("formStats"), formStats);				// Packet Statistics Form
	websFormDefine(T("formRconfig"), formRconfig);				// Remote Configuration Form
	
	websAspDefine(T("adslDrvSnrTblGraph"), adslDrvSnrTblGraph);
	websAspDefine(T("adslDrvSnrTblList"), adslDrvSnrTblList);
	websAspDefine(T("adslDrvBitloadTblGraph"), adslDrvBitloadTblGraph);
	websAspDefine(T("adslDrvBitloadTblList"), adslDrvBitloadTblList);
	websAspDefine(T("memDump"), memDump);	
	websAspDefine(T("pktStatsList"), pktStatsList);	

	websFormDefine(T("formStatus"), formStatus);			// Status Form
	websFormDefine(T("formStatusModify"), formDate);

	websFormDefine(T("formWanAtm"), formAtmVc);			// Atm VC Configuration Setting Form
	websFormDefine(T("formWanAdsl"), formAdsl);			// ADSL Configuration Setting Form
	websFormDefine(T("formPPPEdit"), formPPPEdit);			// PPP interface Configuration Form
	websFormDefine(T("formIPEdit"), formIPEdit);			// IP interface Configuration Form
	websFormDefine(T("formBrEdit"), formBrEdit);			// Bridged interface Configuration Form
	
	websFormDefine(T("formBridging"), formBridge);			// Bridge Configuration Form
	websAspDefine(T("bridgeFdbList"), bridgeFdbList);
	websFormDefine(T("formRefleshFdbTbl"), formRefleshFdbTbl);

	websFormDefine(T("formRouting"), formRoute);			// Routing Configuration Form
	websAspDefine(T("showStaticRoute"), showStaticRoute);	
	websAspDefine(T("ShowDefaultGateway"), ShowDefaultGateway);	// Jenny, for DEFAULT_GATEWAY_V2	
	websAspDefine(T("ShowRouteInf"), ShowRouteInf);
	websAspDefine(T("GetDefaultGateway"), GetDefaultGateway);
	websFormDefine(T("formRefleshRouteTbl"), formRefleshRouteTbl);
	websAspDefine(T("routeList"), routeList);	

	websFormDefine(T("formDhcpServer"), formDhcpd);	// DHCP Server Setting Form
	websAspDefine(T("dhcpClientList"), dhcpClientList);
	websFormDefine(T("formReflashClientTbl"), formReflashClientTbl);
	
	websFormDefine(T("formDNS"), formDns);			// DNS Configuration Form
	websFormDefine(T("formDhcrelay"), formDhcrelay);	// DHCPRelay Configuration Form
	websFormDefine(T("formPing"), formPing);		// Ping diagnostic Form
	websFormDefine(T("formOAMLB"), formOamLb);		// OAM Loopback diagnostic Form
#ifdef FINISH_MAINTENANCE_SUPPORT
	websFormDefine(T("formFinishMaintenance"), formFinishMaintenance);		// xl_yue added,inform itms
#endif
//added by xl_yue
#ifdef USE_LOGINWEB_OF_SERVER
	websFormDefine(T("formLogin"), formLogin);		// xl_yue added,
	websFormDefine(T("formLogout"), formLogout);		// xl_yue added,
#endif

#ifdef ACCOUNT_LOGIN_CONTROL
	websFormDefine(T("formAdminLogout"), formAdminLogout);		// xl_yue added
	websFormDefine(T("formUserLogout"), formUserLogout);		// xl_yue added
#endif
	websFormDefine(T("formReboot"), formReboot);		// Commit/reboot Form
	websFormDefine(T("formDhcpMode"), formDhcpMode);	// DHCP Mode Configuration Form
	
	websFormDefine(T("formIgmproxy"), formIgmproxy);	// IGMP Configuration Form
	websAspDefine(T("if_wan_list"), ifwanList);

	websFormDefine(T("formRip"), formRip);			// RIP Configuration Form
	websAspDefine(T("if_router_list"), ifrouterList);
	websAspDefine(T("showRipIf"), showRipIf);	
	
	websFormDefine(T("formACL"), formACL);                  // ACL Configuration Form
	websAspDefine(T("showACLTable"), showACLTable);
	
	websFormDefine(T("formOthers"), formOthers);	// Others advance Configuration Form
#ifdef AUTO_PROVISIONING
	websFormDefine(T("formAutoProvision"), formAutoProvision);	// Auto-Rovisioning Configuration Form
#endif
	
#ifdef CONFIG_EXT_SWITCH
	websFormDefine(T("formMpMode"), formMpMode);	// Eth-to-pvc mapping Form
	websFormDefine(T("formEth2Pvc"), formEth2pvc);	// Eth-to-pvc mapping Form
	websAspDefine(T("eth2pvcPost"), eth2pvcPost);
	websFormDefine(T("formVlanCfg"), formVlan);	// Vlan Configuration Form
	websAspDefine(T("vlanPost"), vlanPost);
	websFormDefine(T("formIPQoS"), formQos);	// IP QoS Form
	websAspDefine(T("ipQosList"), qosList);
#endif

	websFormDefine(T("formSAC"), formAcc);	// Services Access Control
	websAspDefine(T("accPost"), accPost);

	websAspDefine(T("autopvcStatus"), autopvcStatus);	// auto-pvc search	
	websAspDefine(T("showPVCList"), showPVCList);	// auto-pvc search		
	websAspDefine(T("ShowAutoPVC"), ShowAutoPVC);	// auto-pvc search		
	websAspDefine(T("ShowApplicationMode"), ShowApplicationMode);	// China Telecom jim...	
	websAspDefine(T("ShowChannelMode"), ShowChannelMode);	// China Telecom jim...	
	websAspDefine(T("ShowPPPIPSettings"), ShowPPPIPSettings);	// China Telecom jim...	
	websAspDefine(T("ShowNAPTSetting"), ShowNAPTSetting);	// China Telecom jim...	
	

#ifdef WLAN_SUPPORT
	websFormDefine(T("formWlanSetup"), formWlanSetup);
	websFormDefine(T("formWep"), formWep);
	websAspDefine(T("wlAcList"), wlAcList);
	websAspDefine(T("wirelessClientList"), wirelessClientList);
	websFormDefine(T("formWirelessTbl"), formWirelessTbl);

	websFormDefine(T("formWlAc"), formWlAc);
	websFormDefine(T("formAdvanceSetup"), formAdvanceSetup);

#ifdef WLAN_WPA
	websFormDefine(T("formWlEncrypt"), formWlEncrypt);
#endif

#endif

#ifdef DIAGNOSTIC_TEST
	websFormDefine(T("formDiagTest"), formDiagTest);	// Diagnostic test
	websAspDefine(T("lanTest"), lanTest);	// Ethernet LAN connection test
	websAspDefine(T("adslTest"), adslTest);	// ADSL service provider connection test
	websAspDefine(T("internetTest"), internetTest);	// Internet service provider connection test
#endif
#ifdef DOS_SUPPORT
	websFormDefine(T("formDosCfg"), formDosCfg);
#endif
/*
 *	Create the Form handlers for the User Management pages
 */
#ifdef USER_MANAGEMENT_SUPPORT
	formDefineUserMgmt();
#endif

/*
 *	Create a handler for the default home page
 */
	websUrlHandlerDefine(T("/"), NULL, 0, websHomePageHandler, 0); 
	printf("[webs] initWebs completed\n");
	return 0;
}

// Kaohj
/******************************************************************************/
/*
 *	Initialize the message queue for System V IPC.
 *	Return message queue identifier on success, -1 on failure
 */

static int initMsgQ()
{
	key_t key;
	int   queue_id;
	
	/* Create unique key via call to ftok() */
	key = ftok("/bin/init", 'k');
	
	if ((queue_id = open_queue(key, MQ_CREATE)) == -1) {
		perror("open_queue");
		return -1;
	}
	
	//printf("queue_id=%d\n", queue_id);
	return queue_id;
}

/******************************************************************************/
/*
 *	Catch SIGTERM signal and SIGINT signal (del/^C).
 */

static void term(int signum)
{
	finished = 1;
}

/******************************************************************************/
/*
 *	Test Javascript binding for ASP. This will be invoked when "aspTest" is
 *	embedded in an ASP page. See web/asp.asp for usage. Set browser to 
 *	"localhost/asp.asp" to test.
 */

/* Mark by Dick Tam */
/*
static int aspTest(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t	*name, *address;

	if (ejArgs(argc, argv, T("%s %s"), &name, &address) < 2) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}
	return websWrite(wp, T("Name: %s, Address %s"), name, address);
}
*/
/* Mark by Dick Tam end */

/******************************************************************************/
/*
 *	Test form for posted data (in-memory CGI). This will be called when the
 *	form in web/forms.asp is invoked. Set browser to "localhost/forms.asp" to test.
 */

/* Mark by Dick Tam */
/*
static void formTest(webs_t wp, char_t *path, char_t *query)
{
	char_t	*name, *address;

	name = websGetVar(wp, T("name"), T("Joe Smith")); 
	address = websGetVar(wp, T("address"), T("1212 Milky Way Ave.")); 

	websHeader(wp);
	websWrite(wp, T("<body><h2>Name: %s, Address: %s</h2>\n"), name, address);
	websFooter(wp);
	websDone(wp, 200);
}
*/
/* Mark by Dick Tam end */

/******************************************************************************/
/*
 *	Home page handler
 */

static int websHomePageHandler(webs_t wp, char_t *urlPrefix, char_t *webDir,
	int arg, char_t *url, char_t *path, char_t *query)
{
/*
 *	If the empty or "/" URL is invoked, redirect default URLs to the home page
 */
	if (*url == '\0' || gstrcmp(url, T("/")) == 0) {
		websRedirect(wp, T("home.asp"));
		return 1;
	}
	return 0;
}

/******************************************************************************/
/*
 *	Default error handler.  The developer should insert code to handle
 *	error messages in the desired manner.
 */

void defaultErrorHandler(int etype, char_t *msg)
{
#if 0
	write(1, msg, gstrlen(msg));
#endif
}

/******************************************************************************/
/*
 *	Trace log. Customize this function to log trace output
 */

void defaultTraceHandler(int level, char_t *buf)
{
/*
 *	The following code would write all trace regardless of level
 *	to stdout.
 */
#if 0
	if (buf) {
		write(1, buf, gstrlen(buf));
	}
#endif
}

/******************************************************************************/
/*
 *	Returns a pointer to an allocated qualified unique temporary file name.
 *	This filename must eventually be deleted with bfree();
 */

char_t *websGetCgiCommName()
{
	char_t	*pname1, *pname2;

// david, remove warning ---------
//	pname1 = tempnam(NULL, T("cgi"));
pname1=malloc(40);
sprintf(pname1, "%sXXXXXX",  T("cgi"));
mkstemp(pname1);
//--------------------------------

	pname2 = bstrdup(B_L, pname1);
	free(pname1);
	return pname2;
}

/******************************************************************************/
/*
 *	Launch the CGI process and return a handle to it.
 */

int websLaunchCgiProc(char_t *cgiPath, char_t **argp, char_t **envp,
					  char_t *stdIn, char_t *stdOut)
{
	int	pid, fdin, fdout, hstdin, hstdout, rc;

	fdin = fdout = hstdin = hstdout = rc = -1; 
	if ((fdin = open(stdIn, O_RDWR | O_CREAT, 0666)) < 0 ||
		(fdout = open(stdOut, O_RDWR | O_CREAT, 0666)) < 0 ||
		(hstdin = dup(0)) == -1 ||
		(hstdout = dup(1)) == -1 ||
		dup2(fdin, 0) == -1 ||
		dup2(fdout, 1) == -1) {
		goto DONE;
	}
		
 	rc = pid = fork();
 	if (pid == 0) {
/*
 *		if pid == 0, then we are in the child process
 */
		if (execve(cgiPath, argp, envp) == -1) {
			printf("content-type: text/html\n\n"
				"Execution of cgi process failed\n");
		}
		exit (0);
	} 

DONE:
	if (hstdout >= 0) {
		dup2(hstdout, 1);
	}
	if (hstdin >= 0) {
		dup2(hstdin, 0);
	}
	if (fdout >= 0) {
		close(fdout);
	}
	if (fdin >= 0) {
		close(fdin);
	}
	return rc;
}

/******************************************************************************/
/*
 *	Check the CGI process.  Return 0 if it does not exist; non 0 if it does.
 */

int websCheckCgiProc(int handle)
{
/*
 *	Check to see if the CGI child process has terminated or not yet.  
 */
	if (waitpid(handle, NULL, WNOHANG) == handle) {
		return 0;
	} else {
		return 1;
	}
}

/******************************************************************************/

#ifdef B_STATS
static void memLeaks() 
{
	int		fd;

	if ((fd = gopen(T("leak.txt"), O_CREAT | O_TRUNC | O_WRONLY)) >= 0) {
		bstats(fd, printMemStats);
		close(fd);
	}
}

/******************************************************************************/
/*
 *	Print memory usage / leaks
 */

static void printMemStats(int handle, char_t *fmt, ...)
{
	va_list		args;
	char_t		buf[256];

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);
	write(handle, buf, strlen(buf));
}
#endif

/******************************************************************************/
