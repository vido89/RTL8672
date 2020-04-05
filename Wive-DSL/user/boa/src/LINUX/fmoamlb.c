/*
 *      Web server handler routines for OAM Loopback diagnostic stuffs
 *
 */


/*-- System inlcude files --*/
#include <sys/socket.h>
#include <sys/signal.h>
#include <stdint.h>
#include <linux/atm.h>
#include <linux/atmdev.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "../defs.h"

#define MAXWAIT		5

static int finished = 0;

/* common routines */

static void lbTimeout()
{
	finished = 1;
}

///////////////////////////////////////////////////////////////////
void formOamLb(webs_t wp, char_t *path, char_t *query)
{
#ifdef EMBED
	char_t	*str, *submitUrl;
	char tmpBuf[100];
	int	skfd,i,j, entryNum;
	struct atmif_sioc mysio;
	ATMOAMLBReq lbReq;
	ATMOAMLBState lbState;
	unsigned int uVc;
	int curidx, len;
	char_t *tmpStr;
	unsigned char *tmpValue;
	MIB_CE_ATM_VC_T Entry;
	
#ifndef NO_ACTION
	int pid;
#endif

	if((skfd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0){
		perror("socket open error");
		exit(1);
	}
	
	memset(&lbReq, 0, sizeof(ATMOAMLBReq));
	memset(&lbState, 0, sizeof(ATMOAMLBState));
	str = websGetVar(wp, T("oam_flow"), T(""));
	if (str[0] == '0')
		lbReq.Scope = 0;	// Segment
	else if (str[0] == '1')
		lbReq.Scope = 1;	// End-to-End
	
	str = websGetVar(wp, T("oam_VC"), T(""));
	if (str==NULL) {
		strcpy(tmpBuf, T(Tno_conn));
		goto setErr_oamlb;
	}
	
	//get PVC number
	sscanf(str,"%u",&uVc);
	if (!mib_chain_get(MIB_ATM_VC_TBL, uVc, (void *)&Entry))
	{
		strcpy(tmpBuf, T("PVC configuration error!"));
		goto setErr_oamlb;
	}
	lbReq.vpi = Entry.vpi;
	lbReq.vci = Entry.vci;
	str = websGetVar(wp, T("oam_llid"), T(""));
	// convert max of 32 hex decimal string into its 16 octets value
	len = strlen(str);
	curidx = 16;
	for (i=0; i<32; i+=2)
	{
		// Loopback Location ID
		curidx--;
		tmpValue = (unsigned char *)&lbReq.LocID[curidx];
		if (len > 0)
		{
			len -= 2;
			if (len < 0)
				len = 0;
			tmpStr = str + len;
			*tmpValue = strtoul(tmpStr, 0, 16);
			*tmpStr='\0';
		}
		else
			*tmpValue = 0;
	}
	
//	printf("Loopback Location ID:\n");
//	for (i=0; i<16; i++)
//		printf("%.02x", lbReq.LocID[i]);
//	printf("\n");
	
	mysio.number = 0;	// ATM interface number
	mysio.arg = (void *)&lbReq;
	// Start the loopback test
	if (ioctl(skfd, ATM_OAM_LB_START, &mysio)<0) {
		strcpy(tmpBuf, "ioctl: ATM_OAM_LB_START failed !");
		close(skfd);
		goto setErr_oamlb;
	}
	
	finished = 0;
	signal(SIGALRM, lbTimeout);
	alarm(MAXWAIT);
	// Query the loopback status
	mysio.arg = (void *)&lbState;
	lbState.vpi = Entry.vpi;
	lbState.vci = Entry.vci;
	lbState.Tag = lbReq.Tag;
	
	while (1)
	{
		if (finished)
			break;	// break for timeout
		
		if (ioctl(skfd, ATM_OAM_LB_STATUS, &mysio)<0) {
			strcpy(tmpBuf, "ioctl: ATM_OAM_LB_STATUS failed !");
			mysio.arg = (void *)&lbReq;
			ioctl(skfd, ATM_OAM_LB_STOP, &mysio);
			close(skfd);
			goto setErr_oamlb;
		}
		
		if (lbState.count[0] > 0)
		{
			break;	// break for loopback success
		}
	}
		
	mysio.arg = (void *)&lbReq;
	// Stop the loopback test
	if (ioctl(skfd, ATM_OAM_LB_STOP, &mysio)<0) {
		strcpy(tmpBuf, "ioctl: ATM_OAM_LB_STOP failed !");
		close(skfd);
		goto setErr_oamlb;
	}
	close(skfd);


#ifndef NO_ACTION
	pid = fork();
        if (pid)
                waitpid(pid, NULL, 0);
        else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _CONFIG_SCRIPT_PROG);
#ifdef HOME_GATEWAY
		execl( tmpBuf, _CONFIG_SCRIPT_PROG, "gw", "bridge", NULL);
#else
		execl( tmpBuf, _CONFIG_SCRIPT_PROG, "ap", "bridge", NULL);
#endif
                exit(1);
        }
#endif

	websHeader(wp);
	if (!finished)
	{
		websWrite(wp, T("<body><blockquote><h4><font color='green'>\"%s\"<br><br>"), Trecv_cell_suc);
		printf("\n--- Loopback cell received successfully ---\n");
	}
	else
	{
		websWrite(wp, T("<body><blockquote><h4><font color='red'>%s<br><br>"), Trecv_cell_fail);
		printf("\n--- Loopback failed ---\n");
	}
	websWrite(wp, T("</h4>\n"));
	printf("\n");
	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	websWrite(wp, T("<form><input type=button value=\"%s\" OnClick=window.location.replace(\"%s\")></form></blockquote></body>"), Tback, submitUrl);
	websFooter(wp);
	websDone(wp, 200);
	
  	return;

setErr_oamlb:
	ERR_MSG(tmpBuf);
#endif
}

int oamSelectList(int eid, webs_t wp, int argc, char_t **argv)
{
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	int first_write=0;

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	
	websWrite(wp, T("<BR>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"));
	for (i=0;i<entryNum;i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			websError(wp, 400, T("Get chain record error!\n"));
			return -1;
		}
		
		if (Entry.enable == 0)
			continue;

		if (first_write==0) {
			first_write=1;
		    websWrite(wp, T("<input type=\"radio\" value=\"%d\" name=\"oam_VC\" checked>%d/%d\n"),
				i, Entry.vpi, Entry.vci);			
		} else 
		    websWrite(wp, T("<input type=\"radio\" value=\"%d\" name=\"oam_VC\">%d/%d\n"),
				i, Entry.vpi, Entry.vci);
	}
	return 0;
}

