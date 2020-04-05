/*
 *      Web server handler routines for diagnostic tests
 *
 */

#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#include "debug.h"
#include "options.h"
#include <net/if.h>
#include <linux/if_bridge.h>
#include <stdio.h>
#include <sys/signal.h>
#include <linux/atm.h>
#include <linux/atmdev.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <net/route.h>
#include	<netdb.h>
#include "../defs.h"

#ifdef DIAGNOSTIC_TEST
#if   !defined(ZTE_531B_BRIDGE_SC) && !defined(ZTE_GENERAL_ROUTER_SC)
static const char R_PASS[] = " color='green'><b>PASS";
static const char R_FAIL[] = " color='red'><b>FAIL";
#else
static const char R_PASS[] = " color='green'><b>Í¨¹ý";
static const char R_FAIL[] = " color='red'><b>Ê§°Ü";
#endif
static int cmode = 0;
int eth=0, adslflag=0, pppserver=0, auth=0, ipup=0, lb5s=0, lb5e=0, lb4s=0, lb4e=0, dgw=0, pdns=0;

#if 0
static struct sockaddr_in pingaddr;
static int pingsock = -1;
static long ntransmitted = 0, nreceived = 0, nrepeats = 0;
static int myid = 0;
/* common routines */
static int create_icmp_socket(void)
{
	struct protoent *proto;
	int sock;

	proto = getprotobyname("icmp");
	/* if getprotobyname failed, just silently force
	 * proto->p_proto to have the correct value for "icmp" */
	if ((sock = socket(AF_INET, SOCK_RAW,
			(proto ? proto->p_proto : 1))) < 0) {        /* 1 == ICMP */
		printf("cannot create raw socket\n");
	}

	/* drop root privs if running setuid */
//	setuid(getuid());

	return sock;
}

static int in_cksum(unsigned short *buf, int sz)
{
	int nleft = sz;
	int sum = 0;
	unsigned short *w = buf;
	unsigned short ans = 0;

	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1) {
		*(unsigned char *) (&ans) = *(unsigned char *) w;
		sum += ans;
	}

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	ans = ~sum;
	return (ans);
}

static void pingfinal()
{
	finished = 1;
}

static void sendping()
{
	struct icmp *pkt;
	int c;
	char packet[DEFDATALEN + 8];

	pkt = (struct icmp *) packet;
	pkt->icmp_type = ICMP_ECHO;
	pkt->icmp_code = 0;
	pkt->icmp_cksum = 0;
	pkt->icmp_seq = ntransmitted++;
	pkt->icmp_id = myid;
	pkt->icmp_cksum = in_cksum((unsigned short *) pkt, sizeof(packet));

	c = sendto(pingsock, packet, sizeof(packet), 0,
			   (struct sockaddr *) &pingaddr, sizeof(struct sockaddr_in));
	
	if (c < 0 || c != sizeof(packet)) {		
		ntransmitted--;
		finished = 1;
		printf("sock: sendto fail !");
		return;
	}
	
	signal(SIGALRM, sendping);
	if (ntransmitted < PINGCOUNT) {
		alarm(PINGINTERVAL);
	} else {	/* done, wait for the last ping to come back */
		signal(SIGALRM, pingfinal);
		alarm(MAXWAIT);
	}
}

int testPing(char *pingString)
{
	char tmpBuf[100];
	struct hostent *h;
	struct icmp *pkt;
	struct iphdr *iphdr;
	char packet[DEFDATALEN + 8];
	int rcvdseq, ret=0;
	fd_set rset;
	struct timeval tv;
	
	if ((pingsock = create_icmp_socket()) < 0)
		printf("ping: socket create error");		
	
	memset(&pingaddr, 0, sizeof(struct sockaddr_in));
	pingaddr.sin_family = AF_INET;
	
	if ((h = gethostbyname(pingString)) == NULL)
		printf("ping: Get HostbyName Error\n");
	
	if (h->h_addrtype != AF_INET)
		printf("unknown address type; only AF_INET is currently supported.\n");
	
	memcpy(&pingaddr.sin_addr, h->h_addr, sizeof(pingaddr.sin_addr));
	
	printf("PING %s (%s): %d data bytes\n", h->h_name,
		inet_ntoa(*(struct in_addr *) &pingaddr.sin_addr.s_addr), DEFDATALEN);
        
	myid = getpid() & 0xFFFF;
	ntransmitted = nreceived = nrepeats = 0;
	finished = 0;
	rcvdseq = ntransmitted-1;
	FD_ZERO(&rset);
	FD_SET(pingsock, &rset);
	/* start the ping's going ... */
	sendping();

	/* listen for replies */
	while (1) {
		struct sockaddr_in from;
		socklen_t fromlen = (socklen_t) sizeof(from);
		int c, hlen, dupflag;
		
		if (finished)
			break;
		
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		
		if (select(pingsock + 1, &rset, NULL, NULL, &tv) > 0) {
			if ((c = recvfrom(pingsock, packet, sizeof(packet), 0, (struct sockaddr *) &from, &fromlen)) < 0) {
				if (errno == EINTR)
					continue;
				
				printf("sock: recvfrom fail !");
				continue;
			}
		}
		else // timeout or error
			continue;
		
		if (c < DEFDATALEN + ICMP_MINLEN)
			continue;
		
		iphdr = (struct iphdr *) packet;
		hlen = iphdr->ihl << 2;
		pkt = (struct icmp *) (packet + hlen);	/* skip ip hdr */
		if (pkt->icmp_id != myid)
			continue;
		if (pkt->icmp_type == ICMP_ECHOREPLY) {
			++nreceived;
			if (pkt->icmp_seq == rcvdseq) {
				// duplicate
				++nrepeats;
				--nreceived;
				dupflag = 1;
			} else {
				rcvdseq = pkt->icmp_seq;
				dupflag = 0;
				if (nreceived < PINGCOUNT)
				// reply received, send another immediately
					sendping();
			}
			printf("%d bytes from %s: icmp_seq=%u", c,
				   inet_ntoa(*(struct in_addr *) &from.sin_addr.s_addr),
				   pkt->icmp_seq);
			if (dupflag)
				printf(" (DUP!)");
			printf("\n");
		}
		if (nreceived >= PINGCOUNT) {
			ret = 1;
			break;
		}
	}
	close(pingsock);	

	printf("\n--- ping statistics ---\n");
	printf("%ld packets transmitted, ", ntransmitted);
	printf("%ld packets received, ", nreceived);
	if (nrepeats)
		printf("%ld duplicates, ", nrepeats);
	printf("\n");
	return ret;
}
#endif

static void processDiagTest(webs_t wp)
{
	int inf=-1, i, pppif;
	MIB_CE_ATM_VC_T Entry;
	unsigned int entryNum;
	FILE *fp;
	char buff[10], ifname[6];
	MIB_CE_ATM_VC_Tp pEntry;
	
	Modem_LinkSpeed vLs;

	if (!adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE) || vLs.upstreamRate == 0)
		adslflag = 0;
	else
		adslflag = 1;

	if (fp = fopen("/tmp/diaginf", "r")) {
		fscanf(fp, "%d", &inf);
		fclose(fp);
	}
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);

	for (i=0;i<entryNum;i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			websError(wp, 400, T("Get chain record error!\n"));
			exit(-1);
		}
		if (Entry.enable == 0)
			continue;
		if (inf == -1)
			inf = Entry.ifIndex;
		if (Entry.ifIndex == inf) {
			if (Entry.cmode != ADSL_BR1483) {
				struct in_addr inAddr;
				int flags;
				cmode = 1;
				if (PPP_INDEX(Entry.ifIndex) != 0x0f) {	// PPP Interface
					int pppflag;
					cmode = 2;
					sprintf(ifname, "ppp%d", PPP_INDEX(Entry.ifIndex));
					if (fp = fopen("/tmp/ppp_diag_log", "r")) {
						while (fgets(buff, sizeof(buff), fp) != NULL) {
							sscanf(buff, "%d:%d", &pppif, &pppflag);
							if (pppif == PPP_INDEX(Entry.ifIndex))
									break;
						}
						fclose(fp);
					}
					switch(pppflag)
					{
						case 1:
							pppserver = 1;
							auth = ipup = 0;
							break;
						case 2:
							pppserver = auth = 1;
							ipup = 0;
							break;
						case 3:
							pppserver = auth = ipup = 1;
							break;
						case 0:
						default:
							pppserver = auth = ipup = 0;
							break;
					}
				}
				else
					sprintf(ifname, "vc%u", VC_INDEX(Entry.ifIndex));
			}
			else
				cmode = 0;
			break;
		}
	}
	pEntry = &Entry;
	if (adslflag) {
		lb5s = testOAMLookback(pEntry, 0, 5);
		lb5e = testOAMLookback(pEntry, 1, 5);
		lb4s = testOAMLookback(pEntry, 0, 4);
		lb4e = testOAMLookback(pEntry, 1, 4);
		if (cmode > 0) {
			char pingaddr[16];
			memset(pingaddr, 0x00, 16);
			if (defaultGWAddr(pingaddr))
				dgw = 0;
			else
//				dgw = testPing(pingaddr);
				dgw = utilping(pingaddr);
			memset(pingaddr, 0x00, 16);
			if (pdnsAddr(pingaddr))
				pdns = 0;
			else
//				pdns = testPing(pingaddr);
				pdns = utilping(pingaddr);
		}
	}
}

int startflag = 0;
void formDiagTest(webs_t wp, char_t *path, char_t *query)
{
	char_t *strSubmit, *submitUrl;
	unsigned char wan_itf;
	FILE *fp;

	strSubmit = websGetVar(wp, T("wan_if"), T(""));
	if (strSubmit[0]) {
		wan_itf = (unsigned char)atoi(strSubmit);
		fp = fopen("/tmp/diaginf", "w");
		fprintf(fp, "%d", wan_itf);
		fclose(fp);
	}

//	strSubmit = websGetVar(wp, T("start"), T(""));
//	if (strSubmit[0]) {
		// start diagnostic test here
		startflag = 1;
//	}

	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
  	return;
}

// Test Ethernet LAN connection table
int lanTest(int eid, webs_t wp, int argc, char_t **argv)
{
	int fd, nBytesSent=0, flags;
	int mii_reg, i;
	struct ifreq ifrq;
	ushort *data = (ushort *)(&ifrq.ifr_data);
	unsigned int *data32 = (unsigned int *)(&ifrq.ifr_data);
	unsigned phy_id=1;
	unsigned char new_ioctl_nums = 0;
	ushort mii_val[32];

	if (startflag) {
		strncpy(ifrq.ifr_name, ELANIF, sizeof(ifrq.ifr_name));
		ifrq.ifr_name[ sizeof(ifrq.ifr_name)-1] = 0;
		eth = getLinkStatus(&ifrq);
		nBytesSent += websWrite(wp, T("<tr><td width=100%% colspan=\"2\" bgcolor=\"#808080\">"
		"<font color=\"#FFFFFF\" size=2><b>%s</b></font></td></tr>"), Tlan_conn_chk);
		nBytesSent += websWrite(wp, T("<tr>"
		"<td width=\"80%%\" bgcolor=\"#DDDDDD\"><font size=2><b>%s</b></td>\n"
		"<td width=\"20%%\" bgcolor=\"#EEEEEE\"><font size=2%s</td></tr>\n"), Ttest_eth_conn, (eth)?R_PASS: R_FAIL);
	}
	return nBytesSent;
}

// Test ADSL service provider connection table
int adslTest(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;
	adslflag = pppserver = auth = ipup = lb5s = lb5e = lb4s = lb4e = dgw = pdns=0;
	if (startflag) {
		processDiagTest(wp);
		nBytesSent += websWrite(wp, T("<tr><td width=100%% colspan=\"2\" bgcolor=\"#808080\">"
		"<font color=\"#FFFFFF\" size=2><b>%s</b></font></td></tr>"), Tadsl_conn_chk);
		nBytesSent += websWrite(wp, T("<tr>"
		"<td width=80%% bgcolor=#DDDDDD><font size=2><b>%s</td>"
		"<td width=20%% bgcolor=#EEEEEE><font size=2%s</td></tr>\n"), Ttest_adsl_syn, (adslflag)?R_PASS: R_FAIL);
		nBytesSent += websWrite(wp, T("<tr>"
		"<td width=80%% bgcolor=#DDDDDD><font size=2><b>%s</b></td>"
		"<td width=20%% bgcolor=#EEEEEE><font size=2%s</td></tr>\n"), Ttest_oam_f5_seg, (lb5s)?R_PASS: R_FAIL);
		nBytesSent += websWrite(wp, T("<tr>"
		"<td width=80%% bgcolor=#DDDDDD><font size=2><b>%s</b></td>"
		"<td width=20%% bgcolor=#EEEEEE><font size=2%s</td></tr>\n"), Ttest_oam_f5_end, (lb5e)?R_PASS: R_FAIL);
		nBytesSent += websWrite(wp, T("<tr>"
		"<td width=80%% bgcolor=#DDDDDD><font size=2><b>%s</b></td>"
		"<td width=20%% bgcolor=#EEEEEE><font size=2%s</td></tr>\n"), Ttest_oam_f4_seg, (lb4s)?R_PASS: R_FAIL);
		nBytesSent += websWrite(wp, T("<tr>"
		"<td width=80%% bgcolor=#DDDDDD><font size=2><b>%s</b></td>"
		"<td width=20%% bgcolor=#EEEEEE><font size=2%s</td></tr>\n"), Ttest_oam_f4_end, (lb4e)?R_PASS: R_FAIL);
	}
	return nBytesSent;
}

// Test Internet service provider connection table
int internetTest(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;
	unsigned int entryNum;
	
	if (startflag) {
		if (cmode > 0) {
			nBytesSent += websWrite(wp, T("<tr><td width=100%% colspan=\"2\" bgcolor=\"#808080\">"
			"<font color=\"#FFFFFF\" size=2><b>%s</b></font></td></tr>"), Tint_conn_chk);
			if (cmode == 2) {
				nBytesSent += websWrite(wp, T("<tr>"
				"<td width=80%% bgcolor=#DDDDDD><font size=2><b>%s</b></td>"
				"<td width=20%% bgcolor=#EEEEEE><font size=2%s</td></tr>\n"), Ttest_ppps_conn, (pppserver)?R_PASS: R_FAIL);
				nBytesSent += websWrite(wp, T("<tr>"
				"<td width=80%% bgcolor=#DDDDDD><font size=2><b>%s</td>"
				"<td width=20%% bgcolor=#EEEEEE><font size=2%s</td></tr>\n"), Ttest_auth, (auth==1)?R_PASS: R_FAIL);
				nBytesSent += websWrite(wp, T("<tr>"
				"<td width=80%% bgcolor=#DDDDDD><font size=2><b>%s</b></td>"
				"<td width=20%% bgcolor=#EEEEEE><font size=2%s</td></tr>\n"), Ttest_assigned_ip, (ipup)?R_PASS: R_FAIL);
			}
			nBytesSent += websWrite(wp, T("<tr>"
			"<td width=80%% bgcolor=#DDDDDD><font size=2><b>%s</b></td>"
			"<td width=20%% bgcolor=#EEEEEE><font size=2%s</td></tr>\n"), Tping_def_gw, (dgw)?R_PASS: R_FAIL);
			nBytesSent += websWrite(wp, T("<tr>"
			"<td width=80%% bgcolor=#DDDDDD><font size=2><b>%s</b></td>"
			"<td width=20%% bgcolor=#EEEEEE><font size=2%s</td></tr>\n"), Tping_pri_dnss, (pdns)?R_PASS: R_FAIL);
		}
		startflag = 0;
	}
	return nBytesSent;
}
#endif
