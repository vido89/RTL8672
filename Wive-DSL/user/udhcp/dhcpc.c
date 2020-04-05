/* dhcpc.c
 *
 * udhcp DHCP client
 *
 * Russ Dill <Russ.Dill@asu.edu> July 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
 
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/file.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>

#include "dhcpd.h"
#include "dhcpc.h"
#include "options.h"
#include "clientpacket.h"
#include "packet.h"
#include "script.h"
#include "socket.h"
#include "debug.h"
#include "pidfile.h"

/* Dick Tam, 2003-05-16, Microsoft AUTO IP procedure */
#include "arpping.h"
#include <stdlib.h>
//add by ramen
#include <rtk/utility.h>

static int state;

/* Dick Tam, 2003-05-16, Microsoft AUTO IP procedure */
static unsigned long requested_ip = 0; /* = 0 */
static unsigned long router_align = 0;

static unsigned long server_addr;
static unsigned long timeout;
static int packet_num; /* = 0 */
static int fd = -1;
static int signal_pipe[2];

#define LISTEN_NONE 0
#define LISTEN_KERNEL 1
#define LISTEN_RAW 2
static int listen_mode;

#define DEFAULT_SCRIPT	"/usr/share/udhcpc/default.script"

struct client_config_t client_config = {
	/* Default options. */
	abort_if_no_lease: 0,
	foreground: 0,
	quit_after_lease: 0,
	background_if_no_lease: 0,
	microsoft_auto_ip_enable: 0,		/* Dick Tam, 2003-05-16, Microsoft AUTO IP procedure */
	interface: "eth0",
	pidfile: NULL,
	script: DEFAULT_SCRIPT,
	clientid: NULL,
	hostname: NULL,
#ifdef DHCP_OPTION_125
	vi_vendorSpec: NULL,
#endif
	ifindex: 0,
	arp: "\0\0\0\0\0\0",		/* appease gcc-3.0 */
};

#ifdef DHCP_OPTION_125
unsigned char vi_enterprise_data[] = {
	0x01 , 0x06 , 0x30 , 0x30 , 0x30,
	0x31 , 0x30 , 0x32 , 0x02 , 0x10 , 0x30 , 0x30 , 0x30 , 0x31 , 0x30 , 0x32 , 0x2d , 0x34 , 0x32 , 0x38 , 0x32,
	0x38 , 0x38 , 0x38 , 0x32 , 0x39 , 0x03 , 0x11 , 0x43 , 0x44 , 0x52 , 0x6f , 0x75 , 0x74 , 0x65 , 0x72 , 0x20,
	0x56 , 0x6f , 0x49 , 0x50 , 0x20 , 0x41 , 0x54 , 0x41 };
#endif
/*ql:20080926 START: initial MIB_DHCP_CLIENT_OPTION_TBL*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
unsigned char wan_ifIndex;
#endif
/*ql:20080926 END*/

#ifndef IN_BUSYBOX
static void __attribute__ ((noreturn)) show_usage(void)
{
	printf(
"Usage: udhcpc [OPTIONS]\n\n"
"  -c, --clientid=CLIENTID         Client identifier\n"
"  -H, --hostname=HOSTNAME         Client hostname\n"
"  -h                              Alias for -H\n"
"  -f, --foreground                Do not fork after getting lease\n"
"  -b, --background                Fork to background if lease cannot be\n"
"                                  immediately negotiated.\n"
"  -i, --interface=INTERFACE       Interface to use (default: eth0)\n"
"  -n, --now                       Exit with failure if lease cannot be\n"
"                                  immediately negotiated.\n"
"  -p, --pidfile=file              Store process ID of daemon in file\n"
"  -q, --quit                      Quit after obtaining lease\n"
"  -r, --request=IP                IP address to request (default: none)\n"
"  -s, --script=file               Run file at dhcp events (default:\n"
"                                  " DEFAULT_SCRIPT ")\n"
"  -v, --version                   Display version\n"
"  -a, --auto IP                   Microsoft AUTO IP enable\n"			/* Dick Tam, 2003-05-16, Microsoft AUTO IP procedure */
	);
	exit(0);
}
#else
extern void show_usage(void) __attribute__ ((noreturn));
#endif


/* just a little helper */
static void change_mode(int new_mode)
{
	DEBUG(LOG_INFO, "entering %s listen mode",
		new_mode ? (new_mode == 1 ? "kernel" : "raw") : "none");
	if (fd >= 0) close(fd);
	fd = -1;
	listen_mode = new_mode;
}


/* perform a renew */
static void perform_renew(void)
{
	LOG(LOG_INFO, "Performing a DHCP renew");
	switch (state) {
	case BOUND:
		change_mode(LISTEN_KERNEL);
	case RENEWING:
	case REBINDING:
		state = RENEW_REQUESTED;
		break;
	case RENEW_REQUESTED: /* impatient are we? fine, square 1 */
		run_script(NULL, "deconfig");
	case REQUESTING:
	case RELEASED:
		change_mode(LISTEN_RAW);
		state = INIT_SELECTING;
		break;
	case INIT_SELECTING:
		break;			/* Dick Tam, 2003-05-16, in order to eliminate compile warning */
	}

	/* start things over */
	packet_num = 0;

	/* Kill any timeouts because the user wants this to hurry along */
	timeout = 0;
}


/* perform a release */
static void perform_release(void)
{
	char buffer[16];
	struct in_addr temp_addr;

	/* send release packet */
	if (state == BOUND || state == RENEWING || state == REBINDING) {
		temp_addr.s_addr = server_addr;
		sprintf(buffer, "%s", inet_ntoa(temp_addr));
		temp_addr.s_addr = requested_ip;
		LOG(LOG_INFO, "Unicasting a release of %s to %s", 
				inet_ntoa(temp_addr), buffer);
		send_release(server_addr, requested_ip); /* unicast */
		run_script(NULL, "deconfig");
	}
	LOG(LOG_INFO, "Entering released state");

	change_mode(LISTEN_NONE);
	state = RELEASED;
	timeout = 0x7fffffff;
}


/* Exit and cleanup */
static void exit_client(int retval)
{
	pidfile_delete(client_config.pidfile);
	/*ql:20080926 START: when stop process, clear mib tbl*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	//stopDhcpc();
#endif
	/*ql:20080926 END*/
	CLOSE_LOG();
	exit(retval);
}


/* Signal handler */
static void signal_handler(int sig)
{
	if (send(signal_pipe[1], &sig, sizeof(sig), MSG_DONTWAIT) < 0) {
		LOG(LOG_ERR, "Could not send signal: %s",
			strerror(errno));
	}
}


static void background(void)
{
#if 0
	int pid_fd;

	pid_fd = pidfile_acquire(client_config.pidfile); /* hold lock during fork. */
	while (pid_fd >= 0 && pid_fd < 3) pid_fd = dup(pid_fd); /* don't let daemon close it */
	if (daemon(0, 0) == -1) {
		perror("fork");
		exit_client(1);
	}
	client_config.foreground = 1; /* Do not fork again. */
	client_config.background_if_no_lease = 0;
	pidfile_write_release(pid_fd);
#endif	
}

/* Dick Tam, 2003-05-16, Microsoft AUTO IP procedure */
static struct in_addr rt_microsoft_auto_ip_generate(unsigned char * mac, unsigned int count)
{
	unsigned long seed;
	unsigned char addr[4];
	struct in_addr temp;
	unsigned int i;

	seed = mac[0];
	seed = (seed << 2) ^ mac[1];
	seed = (seed << 2) ^ mac[2];
	seed = (seed << 2) ^ mac[3];
	seed = (seed << 2) ^ mac[4];
	seed = (seed << 2) ^ mac[5];

	srandom(seed);

	for(i=0;i<=count;i++)
	{
		seed = (unsigned long) random();
	}

	// network byte order
	addr[0] = 0xA9;	// 169
	addr[1] = 0xFE;	// 254
	addr[2] = (seed >> 8) & 0xFF;
	addr[3] = seed & 0xFF;

	if((addr[3] == 0x00) || (addr[3] == 0xFE) || (addr[3] == 0xFF))
	{
		// X.X.X.0, X.X.X.255, X.X.X.254 is NOT allowed
		addr[3] = 0x01;
	}

	memcpy(&seed, addr, 4);
	temp.s_addr  =seed;

	return temp;
}

static int rt_microsoft_auto_ip_start(struct in_addr *pResult)
{
	unsigned int i;
	struct in_addr addr;

	for(i=0;i<10;i++)
	{
		addr = rt_microsoft_auto_ip_generate(client_config.arp, i);

		if(arpping(addr.s_addr, 0, client_config.arp, client_config.interface) != 0)
		{	// ARP no response
			*pResult = addr;
			return 0;
		}		
	}

	return -1;
}

#ifdef DHCP_OPTION_125
// Kaohj --- calculate VI Vendor-Specific option data
static void get_vendorSpec()
{
	int data_len, option_len;
	struct vendor_info_t *vendor_data;
	
	if (client_config.vi_vendorSpec) free(client_config.vi_vendorSpec);
	// data-len
	data_len = sizeof(vi_enterprise_data);
	// option-len
	option_len = data_len+4+1;	// enterprise-number(4)+data-len(1)
	client_config.vi_vendorSpec = xmalloc(option_len + 2); // option-code+option-len
	client_config.vi_vendorSpec[OPT_CODE] = DHCP_VI_VENSPEC;
	client_config.vi_vendorSpec[OPT_LEN] = option_len;
	vendor_data = (struct vendor_info_t *)&client_config.vi_vendorSpec[OPT_DATA];
	vendor_data->ent_num = htonl(3561);
	vendor_data->data_len = data_len;
	memcpy(client_config.vi_vendorSpec + OPT_DATA+4+1, vi_enterprise_data, data_len);
}
#endif

#ifdef COMBINED_BINARY
int udhcpc_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	unsigned char *temp, *message;
	unsigned long t1 = 0, t2 = 0, xid = 0;
	unsigned long start = 0, lease;
	fd_set rfds;
	int retval;
	struct timeval tv;
	int c, len;
	struct dhcpMessage packet;
	struct in_addr temp_addr;
	int pid_fd;
	time_t now;
	int max_fd;
	int sig;
	int tmp;

	static struct option arg_options[] = {
		{"clientid",	required_argument,	0, 'c'},
		{"foreground",	no_argument,		0, 'f'},
		{"background",	no_argument,		0, 'b'},
		{"hostname",	required_argument,	0, 'H'},
		{"hostname",    required_argument,      0, 'h'},
		{"interface",	required_argument,	0, 'i'},
		{"now", 	no_argument,		0, 'n'},
		{"pidfile",	required_argument,	0, 'p'},
		{"quit",	no_argument,		0, 'q'},
		{"request",	required_argument,	0, 'r'},
		{"script",	required_argument,	0, 's'},
		/*ql:20080926 START: initial MIB_DHCP_CLIENT_OPTION_TBL*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
		{"wanifIndex", required_argument,	0, 'w'},
#endif
		/*ql:20080926 END*/
		{"version",	no_argument,		0, 'v'},
		{"help",	no_argument,		0, '?'},
		{"autoip",	no_argument,		0, 'a'},		/* Dick Tam, 2003-05-16, Microsoft AUTO IP procedure */
		{0, 0, 0, 0}
	};

	/* get options */
	while (1) {
		int option_index = 0;
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
		c = getopt_long(argc, argv, "c:fbH:h:i:np:qr:s:w:v:a", arg_options, &option_index);		/* Dick Tam, 2003-05-16, Microsoft AUTO IP procedure */
#else
		c = getopt_long(argc, argv, "c:fbH:h:i:np:qr:s:v:a", arg_options, &option_index);		/* Dick Tam, 2003-05-16, Microsoft AUTO IP procedure */
#endif
		if (c == -1) break;
		
		switch (c) {
		case 'c':
			len = strlen(optarg) > 255 ? 255 : strlen(optarg);
			if (client_config.clientid) free(client_config.clientid);
			client_config.clientid = xmalloc(len + 2);
			client_config.clientid[OPT_CODE] = DHCP_CLIENT_ID;
			client_config.clientid[OPT_LEN] = len;
			client_config.clientid[OPT_DATA] = '\0';
			strncpy(client_config.clientid + OPT_DATA, optarg, len);
			break;
		case 'f':
			client_config.foreground = 1;
			break;
		case 'b':
			client_config.background_if_no_lease = 1;
			break;
		case 'h':
		case 'H':
			len = strlen(optarg) > 255 ? 255 : strlen(optarg);
			if (client_config.hostname) free(client_config.hostname);
			client_config.hostname = xmalloc(len + 2);
			client_config.hostname[OPT_CODE] = DHCP_HOST_NAME;
			client_config.hostname[OPT_LEN] = len;
			strncpy(client_config.hostname + 2, optarg, len);
			break;
		case 'i':
			client_config.interface =  optarg;
			break;
		case 'n':
			client_config.abort_if_no_lease = 1;
			break;
		case 'p':
			client_config.pidfile = optarg;
			break;
		case 'q':
			client_config.quit_after_lease = 1;
			break;
		case 'r':
			requested_ip = inet_addr(optarg);
			break;
		case 's':
			client_config.script = optarg;
			break;
		case 'v':
			printf("udhcpcd, version %s\n\n", VERSION);
			exit_client(0);
			break;
		/* Dick Tam, 2003-05-16, Microsoft AUTO IP procedure */	
		case 'a':
			client_config.microsoft_auto_ip_enable = 1;
			break;
		/*ql:20080926 START: initial MIB_DHCP_CLIENT_OPTION_TBL*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
		case 'w':
			tmp = atoi(optarg);
			wan_ifIndex = (unsigned char)tmp;
			break;
#endif
		/*ql:20080926 END*/
		default:
			show_usage();
		}
	}

	// Add by Dick
	// No daemon DHCP client, only foreground for ucLinux
	client_config.foreground = 1;

	OPEN_LOG("udhcpc");
	LOG(LOG_INFO, "udhcp client (v%s) started", VERSION);

	pid_fd = pidfile_acquire(client_config.pidfile);
	pidfile_write_release(pid_fd);

	if (read_interface(client_config.interface, &client_config.ifindex, 
			   NULL, client_config.arp) < 0)
		exit_client(1);
		
	if (!client_config.clientid) {
		client_config.clientid = xmalloc(6 + 3);
		client_config.clientid[OPT_CODE] = DHCP_CLIENT_ID;
		client_config.clientid[OPT_LEN] = 7;
		client_config.clientid[OPT_DATA] = 1;
		memcpy(client_config.clientid + 3, client_config.arp, 6);
	}

#ifdef DHCP_OPTION_125
	get_vendorSpec();
#endif
	/*ql:20080926 START: initial MIB_DHCP_CLIENT_OPTION_TBL*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	initialDhcpcOptionTbl();
#endif
	/*ql:20080926 END*/
	
	/*ql:20080925 START: modify entry in MIB_DHCP_CLIENT_OPTION_TBL*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	unsigned char clientid[10];
	clientid[0] = 1;
	memcpy(clientid+1, client_config.arp, 6);
	updateDhcpcOptionTbl(DHCP_CLIENT_ID, clientid, 7);
#endif
	/*ql:20080925 END*/

	/* setup signal handlers */
	socketpair(AF_UNIX, SOCK_STREAM, 0, signal_pipe);
	signal(SIGUSR1, signal_handler);
	signal(SIGUSR2, signal_handler);
	signal(SIGTERM, signal_handler);
	
	state = INIT_SELECTING;
	run_script(NULL, "deconfig");
	change_mode(LISTEN_RAW);

	for (;;) {

		tv.tv_sec = timeout - time(0);
		tv.tv_usec = 0;
		FD_ZERO(&rfds);

		if (listen_mode != LISTEN_NONE && fd < 0) {
			if (listen_mode == LISTEN_KERNEL)
				fd = listen_socket(INADDR_ANY, CLIENT_PORT, client_config.interface);
			else
				fd = raw_socket(client_config.ifindex);
			if (fd < 0) {
				LOG(LOG_ERR, "FATAL: couldn't listen on socket, %s", strerror(errno));
				exit_client(0);
			}
		}
		if (fd >= 0) FD_SET(fd, &rfds);
		FD_SET(signal_pipe[0], &rfds);		

		if (tv.tv_sec > 0) {
			DEBUG(LOG_INFO, "Waiting on select...\n");
			max_fd = signal_pipe[0] > fd ? signal_pipe[0] : fd;
			retval = select(max_fd + 1, &rfds, NULL, NULL, &tv);
		} else retval = 0; /* If we already timed out, fall through */

		now = time(0);
		if (retval == 0) {
			/* timeout dropped to zero */
			switch (state) {
			case INIT_SELECTING:
				if (packet_num < 3) {
					if (packet_num == 0)
						xid = random_xid();

					/* send discover packet */
					send_discover(xid, requested_ip); /* broadcast */
					
					timeout = now + ((packet_num == 2) ? 4 : 2);
					packet_num++;
				} else {
					/* Dick Tam, 2003-05-16, Microsoft AUTO IP procedure */
					struct in_addr auto_addr;
				
					if(client_config.microsoft_auto_ip_enable && 
						(requested_ip != 0) && 	(router_align != 0) &&
						(arpping(requested_ip, 0, client_config.arp, client_config.interface) != 0) &&
						(arpping(router_align, requested_ip, client_config.arp, client_config.interface) == 0)) {
						// Microsoft AUTO IP Procedure success, with valid lease
						struct dhcpMessage packet;
						auto_addr.s_addr = requested_ip;
						
						rt_prepare_microsoft_auto_ip_dhcpack(&packet, auto_addr.s_addr);
						run_script(&packet, "bound");
							
						LOG(LOG_INFO, "Microsoft AUTO IP success with valid lease %s",inet_ntoa(auto_addr));

						if (client_config.quit_after_lease) 
							exit_client(0);
						if (!client_config.foreground)
							background();

						/* wait to try again */
						packet_num = 0;
#ifdef FAST_LEASE_DEBUG
						timeout = now + 30;
#else
						timeout = now + 300;
#endif
					} else if(client_config.microsoft_auto_ip_enable &&
						(rt_microsoft_auto_ip_start(&auto_addr) != -1)){
						// Microsoft AUTO IP Procedure success
						struct dhcpMessage packet;

						rt_prepare_microsoft_auto_ip_dhcpack(&packet, auto_addr.s_addr);
						run_script(&packet, "bound");
							
						LOG(LOG_INFO, "Microsoft AUTO IP success %s",inet_ntoa(auto_addr));

						if (client_config.quit_after_lease) 
							exit_client(0);
						if (!client_config.foreground)
							background();

						router_align = 0;
						requested_ip = auto_addr.s_addr;
							
						/* wait to try again */
						packet_num = 0;
#ifdef FAST_LEASE_DEBUG
						timeout = now + 30;
#else
						timeout = now + 300;
#endif						
					} else {	// non-AUTO IP case
						if (client_config.background_if_no_lease) {
							LOG(LOG_INFO, "No lease, forking to background.");
							background();
						} else if (client_config.abort_if_no_lease) {
							LOG(LOG_INFO, "No lease, failing.");
							exit_client(1);
					  	}
						/* wait to try again */
						packet_num = 0;
#ifdef FAST_LEASE_DEBUG
						timeout = now + 30;
#else
//						timeout = now + 60;
						timeout = now + 30;
#endif
//						LOG(LOG_INFO, "No lease, wait 60 second to retry.");
						LOG(LOG_INFO, "No lease, wait 30 second to retry.");
					}
				}
				break;
			case RENEW_REQUESTED:
			case REQUESTING:
				if (packet_num < 3) {
					/* send request packet */
					if (state == RENEW_REQUESTED)
						send_renew(xid, server_addr, requested_ip); /* unicast */
					else send_selecting(xid, server_addr, requested_ip); /* broadcast */
					
					timeout = now + ((packet_num == 2) ? 10 : 2);
					packet_num++;
				} else {
					/* timed out, go back to init state */
					if (state == RENEW_REQUESTED) run_script(NULL, "deconfig");
					state = INIT_SELECTING;
					timeout = now;
					packet_num = 0;
					change_mode(LISTEN_RAW);
				}
				break;
			case BOUND:
				/* Lease is starting to run out, time to enter renewing state */
				state = RENEWING;
				change_mode(LISTEN_KERNEL);
				DEBUG(LOG_INFO, "Entering renew state");
				/* fall right through */
			case RENEWING:
				/* Either set a new T1, or enter REBINDING state */
				if ((t2 - t1) <= (lease / 14400 + 1)) {
					/* timed out, enter rebinding state */
					state = REBINDING;
					timeout = now + (t2 - t1);
					DEBUG(LOG_INFO, "Entering rebinding state");
				} else {
					/* send a request packet */
					send_renew(xid, server_addr, requested_ip); /* unicast */
					
					t1 = (t2 - t1) / 2 + t1;
					timeout = t1 + start;
				}
				break;
			case REBINDING:
				/* Either set a new T2, or enter INIT state */
				if ((lease - t2) <= (lease / 14400 + 1)) {
					/* timed out, enter init state */
					state = INIT_SELECTING;
					LOG(LOG_INFO, "Lease lost, entering init state");
					run_script(NULL, "deconfig");
					timeout = now;
					packet_num = 0;
					change_mode(LISTEN_RAW);
				} else {
					/* send a request packet */
					send_renew(xid, 0, requested_ip); /* broadcast */

					t2 = (lease - t2) / 2 + t2;
					timeout = t2 + start;
				}
				break;
			case RELEASED:
				/* yah, I know, *you* say it would never happen */
				timeout = 0x7fffffff;
				break;
			}
		} else if (retval > 0 && listen_mode != LISTEN_NONE && FD_ISSET(fd, &rfds)) {
			/* a packet is ready, read it */
			
			if (listen_mode == LISTEN_KERNEL)
				len = get_packet(&packet, fd);
			else len = get_raw_packet(&packet, fd);
			
			if (len == -1 && errno != EINTR) {
				DEBUG(LOG_INFO, "error on read, %s, reopening socket", strerror(errno));
				change_mode(listen_mode); /* just close and reopen */
			}
			if (len < 0) continue;
			
			if (packet.xid != xid) {
				DEBUG(LOG_INFO, "Ignoring XID %lx (our xid is %lx)",
					(unsigned long) packet.xid, xid);
				continue;
			}
			
			if ((message = get_option(&packet, DHCP_MESSAGE_TYPE)) == NULL) {
				DEBUG(LOG_ERR, "couldnt get option from packet -- ignoring");
				continue;
			}
			
			switch (state) {
			case INIT_SELECTING:
				/* Must be a DHCPOFFER to one of our xid's */
				if (*message == DHCPOFFER) {
					if ((temp = get_option(&packet, DHCP_SERVER_ID))) {
						memcpy(&server_addr, temp, 4);
						xid = packet.xid;
						requested_ip = packet.yiaddr;
						
						/* enter requesting state */
						state = REQUESTING;
						timeout = now;
						packet_num = 0;
					} else {
						DEBUG(LOG_ERR, "No server ID in message");
					}
				}
				break;
			case RENEW_REQUESTED:
			case REQUESTING:
			case RENEWING:
			case REBINDING:
				if (*message == DHCPACK) {
					if (!(temp = get_option(&packet, DHCP_LEASE_TIME))) {
						LOG(LOG_ERR, "No lease time with ACK, using 1 hour lease");
						lease = 60 * 60;
					} else {
						memcpy(&lease, temp, 4);
						lease = ntohl(lease);
					}

#ifdef FAST_LEASE_DEBUG
					lease = 30;
#endif						
					/* enter bound state */
					t1 = lease / 2;
					
					/* little fixed point for n * .875 */
					t2 = (lease * 0x7) >> 3;
					temp_addr.s_addr = packet.yiaddr;
					LOG(LOG_INFO, "Lease of %s obtained, lease time %ld", 
						inet_ntoa(temp_addr), lease);
					start = now;
					timeout = t1 + start;
					requested_ip = packet.yiaddr;

					/*ql: 20080926 START: get request option value*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
					updateReqDhcpcOptionValue(&packet);
#endif
					/*ql: 20080926 END*/
					
					/* Dick Tam, 2003-05-16, Microsoft AUTO IP procedure */
					// store router IP for Microsoft AUTO IP Procedure
					if((temp = get_option(&packet, DHCP_ROUTER)))
						memcpy(&router_align, temp, 4);
					#ifdef CONFIG_TR069_DNS_ISOLATION
					 set_tr069_dns_isolation(1);
					#endif
					run_script(&packet,((state == RENEWING || state == REBINDING) ? "renew" : "bound"));

					state = BOUND;
					change_mode(LISTEN_NONE);
					if (client_config.quit_after_lease) 
						exit_client(0);
					if (!client_config.foreground)
						background();
					//add by ramen for the dns bind pvc
					DnsBindPvcRoute(1);//delete the route about dns bind pvc
					DnsBindPvcRoute(0);//add the route about dns bind pvc
					#ifdef CONFIG_TR069_DNS_ISOLATION
				       set_tr069_dns_isolation(0);
					#endif

				} else if (*message == DHCPNAK) {
					/* return to init state */
					LOG(LOG_INFO, "Received DHCP NAK");
					run_script(&packet, "nak");
					if (state != REQUESTING)
						run_script(NULL, "deconfig");
					state = INIT_SELECTING;
					timeout = now;
					requested_ip = 0;
					packet_num = 0;
					change_mode(LISTEN_RAW);
					sleep(3); /* avoid excessive network traffic */
				}
				break;
			/* case BOUND, RELEASED: - ignore all packets */
			}	
		} else if (retval > 0 && FD_ISSET(signal_pipe[0], &rfds)) {
			if (read(signal_pipe[0], &sig, sizeof(sig)) < 0) {
				DEBUG(LOG_ERR, "Could not read signal: %s", 
					strerror(errno));
				continue; /* probably just EINTR */
			}
			switch (sig) {
			case SIGUSR1: 
				perform_renew();
				break;
			case SIGUSR2:
				perform_release();
				break;
			case SIGTERM:
				LOG(LOG_INFO, "Received SIGTERM");
				exit_client(0);
			}
		} else if (retval == -1 && errno == EINTR) {
			/* a signal was caught */		
		} else {
			/* An error occured */
			DEBUG(LOG_ERR, "Error on select");
		}
		
	}
	return 0;
}

