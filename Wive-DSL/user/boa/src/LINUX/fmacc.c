/*
 *      Web server handler routines for service access control
 *
 */

/*-- System inlcude files --*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/wait.h>

#include "../webs.h"
#include "webform.h"
#include "mib.h"

#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../../uClibc/include/linux/autoconf.h"
#endif

/////////////////////////////////////////////////////////////////////////////
#ifdef ZTE_GENERAL_ROUTER_SC
void formAcc(webs_t wp, char_t *path, char_t *query)
{
	char_t *submitUrl, *strVal;
	char tmpBuf[100];
	unsigned int i;
	unsigned char vChar;
	unsigned int  uPort;
#ifndef NO_ACTION
	int pid;
#endif
	
//#ifdef APPLY_CHANGE
	// remove entries
	//filter_set_remote_access(0);
//#endif
	/*if (!mib_chain_get(MIB_ACC_TBL, 0, (void *)&Entry)) {
		memset(&entry, '\0', sizeof(MIB_CE_ACC_T));
		mib_chain_add(MIB_ACC_TBL, (unsigned char*)&entry);
		mib_chain_get(MIB_ACC_TBL, 0, (void *)&Entry);
	}
	
	memset(&Entry, '\0', sizeof(MIB_CE_ACC_T));*/
	filter_set_remote_access(0);
	//mib_chain_clear(MIB_ACC_TBL);
	   char retrieveStr[64]={0};	 
	   MIB_CE_ACC_T Entry;	
	   MIB_CE_ATM_VC_T pvcEntry;
	   int interfaceIndex;
	   //get selected pvc
	  strVal = websGetVar(wp,"interfaceName", T(""));
	   interfaceIndex=strVal[0]-'0';
	  /*if(!(mib_chain_get(MIB_ACC_TBL,interfaceIndex,&Entry))&&!(mib_chain_get(MIB_ATM_VC_TBL,interfaceIndex,&pvcEntry)))
	  	{
	  	printf("get MIB_ATM_VC_TBL error!\n");
	  	return;
	  } */
	   if(!(mib_chain_get(MIB_ACC_TBL,interfaceIndex,&Entry)))
	  	{
	  	printf("get MIB_ATM_VC_TBL error!\n");
	  	return;
	  } 
	  assignDefaultValue(&Entry);
	//  copyPvcEntryToAccEntry(&Entry,&pvcEntry);
	  #ifdef CONFIG_USER_TELNETD_TELNETD
	   //telnet 	  
	   strVal = websGetVar(wp,"w_telnet", T(""));
	  if (strVal[0]=='1')
	{
		Entry.telnet |= 0x01;
		uPort = 0;
		strVal = websGetVar(wp, T("w_telnet_port"), T(""));
		if (strVal[0]) {
			sscanf(strVal, "%u", &uPort);
			Entry.telnet_port = uPort;
		} 
		if (!Entry.telnet_port)
			Entry.telnet_port = 23;
	  }
	  #endif
	  //ftp
	#ifdef CONFIG_USER_FTPD_FTPD
	strVal = websGetVar(wp, "w_ftp", T(""));
	if (strVal[0]=='1')
		{		
		Entry.ftp |= 0x01;		
		uPort = 0;
		strVal = websGetVar(wp, T("w_ftp_port"), T(""));
		if (strVal[0]) {
		sscanf(strVal, "%u", &uPort);
		Entry.ftp_port = uPort;
		} 
		if (!Entry.ftp_port)
		 Entry.ftp_port = 21;
	}
	#endif
	// tftp
	#ifdef CONFIG_USER_TFTPD_TFTPD
	strVal = websGetVar(wp, "w_tftp", T(""));
	if (strVal[0]=='1')
		Entry.tftp |= 0x01;
	#endif
	// web
	strVal = websGetVar(wp, "w_web", T(""));
	if (strVal[0]=='1')
	{
		Entry.web |= 0x01;
		uPort = 0;
		strVal = websGetVar(wp, T("w_web_port"), T(""));
		if (strVal[0]) {
			sscanf(strVal, "%u", &uPort);
			Entry.web_port = uPort;
		} 
		if (!Entry.web_port)
		Entry.web_port = 80;
	}	
	#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
	// snmp	
	strVal = websGetVar(wp, "w_snmp", T(""));
	if (strVal[0]=='1')
		Entry.snmp |= 0x01;	
	#endif
	// ssh
	#ifdef CONFIG_USER_SSH_DROPBEAR
	strVal = websGetVar(wp, "w_ssh", T(""));
	if (strVal[0]=='1')
		Entry.ssh |= 0x01;
	#endif
	// icmp
	// LAN side access is always enabled
	strVal = websGetVar(wp, "w_icmp", T(""));
	if (strVal[0]=='1')
		Entry.icmp |= 0x01;
	mib_chain_update(MIB_ACC_TBL, (void *)&Entry,interfaceIndex);	
		
setOk_acc:
	// log message	
//#ifdef APPLY_CHANGE
	// Apply to changes
	filter_set_remote_access(1);
//#endif
//	mib_update(CURRENT_SETTING);

#ifndef NO_ACTION
	pid = fork();
        if (pid) {
	      	waitpid(pid, NULL, 0);
	}
        else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _FIREWALL_SCRIPT_PROG);
		execl( tmpBuf, _FIREWALL_SCRIPT_PROG, NULL);
               	exit(1);
        }
#endif

	submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
  	return;

setErr_acc:
	ERR_MSG(tmpBuf);
}
#else
void formAcc(webs_t wp, char_t *path, char_t *query)
{
	char_t *submitUrl, *strVal;
	char tmpBuf[100];
	unsigned int i;
	MIB_CE_ACC_T Entry;
	MIB_CE_ACC_T entry;
	unsigned char vChar;
	unsigned int  uPort;
#ifndef NO_ACTION
	int pid;
#endif
	
//#ifdef APPLY_CHANGE
	// remove entries
	filter_set_remote_access(0);
//#endif
	if (!mib_chain_get(MIB_ACC_TBL, 0, (void *)&Entry)) {
		memset(&entry, '\0', sizeof(MIB_CE_ACC_T));
		mib_chain_add(MIB_ACC_TBL, (unsigned char*)&entry);
		mib_chain_get(MIB_ACC_TBL, 0, (void *)&Entry);
	}
	
	memset(&Entry, '\0', sizeof(MIB_CE_ACC_T));
	
	// telnet
	#ifdef CONFIG_USER_TELNETD_TELNETD
	strVal = websGetVar(wp, T("l_telnet"), T(""));
	if (strVal[0]=='1')
		Entry.telnet |= 0x02;
	strVal = websGetVar(wp, T("w_telnet"), T(""));
	if (strVal[0]=='1')
		Entry.telnet |= 0x01;

	uPort = 0;
	strVal = websGetVar(wp, T("w_telnet_port"), T(""));
	if (strVal[0]) {
		sscanf(strVal, "%u", &uPort);
		Entry.telnet_port = uPort;
	} 
	if (!Entry.telnet_port)
		Entry.telnet_port = 23;
	#endif
	
	// ftp
	#ifdef CONFIG_USER_FTPD_FTPD
	strVal = websGetVar(wp, T("l_ftp"), T(""));
	if (strVal[0]=='1')
		Entry.ftp |= 0x02;
	strVal = websGetVar(wp, T("w_ftp"), T(""));
	if (strVal[0]=='1')
		Entry.ftp |= 0x01;

	uPort = 0;
	strVal = websGetVar(wp, T("w_ftp_port"), T(""));
	if (strVal[0]) {
		sscanf(strVal, "%u", &uPort);
		Entry.ftp_port = uPort;
	} 
	if (!Entry.ftp_port)
		Entry.ftp_port = 21;
        #endif
	
	// tftp
	#ifdef CONFIG_USER_TFTPD_TFTPD
	strVal = websGetVar(wp, T("l_tftp"), T(""));
	if (strVal[0]=='1')
		Entry.tftp |= 0x02;
	strVal = websGetVar(wp, T("w_tftp"), T(""));
	if (strVal[0]=='1')
		Entry.tftp |= 0x01;
	#endif
	
	// web
	strVal = websGetVar(wp, T("l_web"), T(""));	
	if (strVal[0]=='1')
		Entry.web |= 0x02;
	strVal = websGetVar(wp, T("w_web"), T(""));
	if (strVal[0]=='1')
		Entry.web |= 0x01;
	uPort = 0;
	strVal = websGetVar(wp, T("w_web_port"), T(""));
	if (strVal[0]) {
		sscanf(strVal, "%u", &uPort);
		Entry.web_port = uPort;
	} 
	if (!Entry.web_port)
		Entry.web_port = 80;
	
	//https
	#ifdef CONFIG_USER_BOA_WITH_SSL
	strVal = websGetVar(wp, T("l_https"), T(""));
	printf("get from web :%s",strVal);
	if (strVal[0]=='1')
		Entry.https |= 0x02;
	strVal = websGetVar(wp, T("w_https"), T(""));
	printf("get from web :%s",strVal);
	if (strVal[0]=='1')
		Entry.https |= 0x01;	
	uPort = 0;
	strVal = websGetVar(wp, T("w_https_port"), T(""));
	printf("get from web :%s",strVal);
	if (strVal[0]) {
		sscanf(strVal, "%u", &uPort);
		Entry.https_port = uPort;
	} 
	if (!Entry.https_port)
		Entry.https_port = 443;
	#endif //end of CONFIG_USER_BOA_WITH_SSL
	
	// snmp	
	#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
	strVal = websGetVar(wp, T("l_snmp"), T(""));
	if (strVal[0]=='1')
		Entry.snmp |= 0x02;
	strVal = websGetVar(wp, T("w_snmp"), T(""));
	if (strVal[0]=='1')
		Entry.snmp |= 0x01;	
	#endif
	
	// ssh
	#ifdef CONFIG_USER_SSH_DROPBEAR
	strVal = websGetVar(wp, T("l_ssh"), T(""));
	if (strVal[0]=='1')
		Entry.ssh |= 0x02;
	strVal = websGetVar(wp, T("w_ssh"), T(""));
	if (strVal[0]=='1')
		Entry.ssh |= 0x01;
	#endif
	
	// icmp
	// LAN side access is always enabled
	strVal = websGetVar(wp, T("w_icmp"), T(""));
	if (strVal[0]=='1')
		Entry.icmp |= 0x01;
	
	
setOk_acc:
	// log message
	mib_chain_update(MIB_ACC_TBL, (void *)&Entry, 0);
//#ifdef APPLY_CHANGE
	// Apply to changes
	filter_set_remote_access(1);
//#endif
//	mib_update(CURRENT_SETTING);

#ifndef NO_ACTION
	pid = fork();
        if (pid) {
	      	waitpid(pid, NULL, 0);
	}
        else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _FIREWALL_SCRIPT_PROG);
		execl( tmpBuf, _FIREWALL_SCRIPT_PROG, NULL);
               	exit(1);
        }
#endif

	submitUrl = websGetVar(wp, T("submit-url"), T(""));   // hidden page
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
  	return;

setErr_acc:
	ERR_MSG(tmpBuf);
}
#endif
// Post the services access control configuration at web page.
// 
int accPost(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;
	unsigned int i, k;
	MIB_CE_ACC_T Entry;
	char strDocAcc[]="document.acc.";
	#ifdef ZTE_GENERAL_ROUTER_SC
	int accEntryNums=mib_chain_total(MIB_ACC_TBL);
	int pvcEntryIndex=0;
	for(pvcEntryIndex=0;pvcEntryIndex<accEntryNums;pvcEntryIndex++)
		{
		memset(&Entry,'\0',sizeof(MIB_CE_ACC_T));
		if (!mib_chain_get(MIB_ACC_TBL, pvcEntryIndex, (void *)&Entry))
			{
			printf("get MIB_ACC_TBL error!\n");
			return 0;
			}
		if(Entry.protocol != ADSL_BR1483)
		{
		#ifdef CONFIG_USER_TELNETD_TELNETD
			if (Entry.telnet & 0x01)
			nBytesSent += websWrite(wp, T("%sw_telnet.checked=true;\n"), strDocAcc);
		#endif
		#ifdef CONFIG_USER_FTPD_FTPD
			if (Entry.ftp & 0x01)
			nBytesSent += websWrite(wp, T("%sw_ftp.checked=true;\n"), strDocAcc);
		#endif
		#ifdef CONFIG_USER_TFTPD_TFTPD
			if (Entry.tftp & 0x01)
			nBytesSent += websWrite(wp, T("%sw_tftp.checked=true;\n"), strDocAcc);
		#endif
			if (Entry.web & 0x01)
			nBytesSent += websWrite(wp, T("%sw_web.checked=true;\n"), strDocAcc);
		#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
			if (Entry.snmp & 0x01)
			nBytesSent += websWrite(wp, T("%sw_snmp.checked=true;\n"), strDocAcc);
		#endif
		#ifdef CONFIG_USER_SSH_DROPEAR
			if (Entry.ssh & 0x01)
			nBytesSent += websWrite(wp, T("%sw_ssh.checked=true;\n"), strDocAcc);
		#endif
			if (Entry.icmp & 0x01)
			nBytesSent += websWrite(wp, T("%sw_icmp.checked=true;\n"), strDocAcc);
			nBytesSent += websWrite(wp, T("%sw_telnet_port.value=%d;\n"), strDocAcc, Entry.telnet_port);
		        nBytesSent += websWrite(wp, T("%sw_ftp_port.value=%d;\n"), strDocAcc, Entry.ftp_port);		
		        nBytesSent += websWrite(wp, T("%sw_web_port.value=%d;\n"), strDocAcc, Entry.web_port);
			nBytesSent += websWrite(wp,T("document.getElementById(\"accSetting\").style.display=\"\";"));
			break;
		}		
	
		}
	#else
	if (!mib_chain_get(MIB_ACC_TBL, 0, (void *)&Entry))
		return 0;
	#ifdef CONFIG_USER_TELNETD_TELNETD
	if (Entry.telnet & 0x02)
		nBytesSent += websWrite(wp, T("%sl_telnet.checked=true;\n"), strDocAcc);	
	if (Entry.telnet & 0x01)
		nBytesSent += websWrite(wp, T("%sw_telnet.checked=true;\n"), strDocAcc);
	#endif
	#ifdef CONFIG_USER_FTPD_FTPD
	if (Entry.ftp & 0x02)
		nBytesSent += websWrite(wp, T("%sl_ftp.checked=true;\n"), strDocAcc);
	if (Entry.ftp & 0x01)
		nBytesSent += websWrite(wp, T("%sw_ftp.checked=true;\n"), strDocAcc);
	#endif
	#ifdef CONFIG_USER_TFTPD_TFTPD
	if (Entry.tftp & 0x02)
		nBytesSent += websWrite(wp, T("%sl_tftp.checked=true;\n"), strDocAcc);
	if (Entry.tftp & 0x01)
		nBytesSent += websWrite(wp, T("%sw_tftp.checked=true;\n"), strDocAcc);
	#endif
	if (Entry.web & 0x02)
		nBytesSent += websWrite(wp, T("%sl_web.checked=true;\n"), strDocAcc);
	if (Entry.web & 0x01)
		nBytesSent += websWrite(wp, T("%sw_web.checked=true;\n"), strDocAcc);
	#ifdef CONFIG_USER_BOA_WITH_SSL
	if (Entry.https & 0x02)
		nBytesSent += websWrite(wp, T("%sl_https.checked=true;\n"), strDocAcc);
	if (Entry.https & 0x01)
		nBytesSent += websWrite(wp, T("%sw_https.checked=true;\n"), strDocAcc);
	#endif //end of CONFIG_USER_BOA_WITH_SSL			
	#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
	if (Entry.snmp & 0x02)
		nBytesSent += websWrite(wp, T("%sl_snmp.checked=true;\n"), strDocAcc);
	if (Entry.snmp & 0x01)
		nBytesSent += websWrite(wp, T("%sw_snmp.checked=true;\n"), strDocAcc);
	#endif
	#ifdef CONFIG_USER_SSH_DROPBEAR
	if (Entry.ssh & 0x02)
		nBytesSent += websWrite(wp, T("%sl_ssh.checked=true;\n"), strDocAcc);
	if (Entry.ssh & 0x01)
		nBytesSent += websWrite(wp, T("%sw_ssh.checked=true;\n"), strDocAcc);
	#endif
	// LAN side access is always enabled
	if (Entry.icmp & 0x01)
		nBytesSent += websWrite(wp, T("%sw_icmp.checked=true;\n"), strDocAcc);
	#ifdef CONFIG_USER_TELNETD_TELNETD	
	nBytesSent += websWrite(wp, T("%sw_telnet_port.value=%d;\n"), strDocAcc, Entry.telnet_port);
	#endif
	#ifdef CONFIG_USER_FTPD_FTPD
	nBytesSent += websWrite(wp, T("%sw_ftp_port.value=%d;\n"), strDocAcc, Entry.ftp_port);		
	#endif
	nBytesSent += websWrite(wp, T("%sw_web_port.value=%d;\n"), strDocAcc, Entry.web_port);	
	#endif
	#ifdef CONFIG_USER_BOA_WITH_SSL
	nBytesSent += websWrite(wp, T("%sw_https_port.value=%d;\n"), strDocAcc, Entry.https_port);
	#endif
	return nBytesSent;
}
int accItem(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;
#ifdef CONFIG_USER_TELNETD_TELNETD
nBytesSent += websWrite(wp, T("<tr>\n"
		"<td align=left class=tdkind>TELNET</td>\n")
		T("<td align=left><input type=checkbox name=l_telnet value=1></td>\n")
		T("<td align=center><input type=checkbox name=w_telnet value=1></td>\n"
		"<td align=left><input type=text size=5 name=w_telnet_port></td>\n</tr>\n"));
#endif
	
#ifdef CONFIG_USER_FTPD_FTPD
	nBytesSent += websWrite(wp, T("<tr>\n"
		"<td align=left class=tdkind>FTP</td>\n")
// Mason Yu. Need to check
		T("<td align=left><input type=checkbox name=l_ftp value=1></td>\n")
		T("<td align=center><input type=checkbox name=w_ftp value=1></td>\n"
		"<td align=left><input type=text size=5 name=w_ftp_port></td>\n</tr>\n"));
#endif

#ifdef CONFIG_USER_TFTPD_TFTPD
	nBytesSent += websWrite(wp, T("<tr>\n"
		"<td align=left class=tdkind>TFTP</td>\n")
		T("<td align=left><input type=checkbox name=l_tftp value=1></td>\n")
		T("<td align=center><input type=checkbox name=w_tftp value=1></td>\n</tr>\n"));

#endif
	
	nBytesSent += websWrite(wp, T("<tr>\n"
		"<td align=left class=tdkind>HTTP</td>")
		T("<td align=left><input type=checkbox name=l_web value=1></td>\n")
		T("<td align=center><input type=checkbox name=w_web value=1></td>\n"
		"<td align=left><input type=text size=5 name=w_web_port></td>\n</tr>\n"));

#ifdef CONFIG_USER_BOA_WITH_SSL
		nBytesSent += websWrite(wp, T("<tr>\n"
		"<td align=left class=tdkind>HTTPS</td>\n"
		"<td align=left><input type=checkbox name=l_https value=1></td>\n"
		"<td align=center><input type=checkbox name=w_https value=1></td>\n"
		"<td align=left><input type=text size=5 name=w_https_port></td>\n</tr>\n"));
#endif //end of CONFIG_USER_BOA_WITH_SSL
	
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
	nBytesSent += websWrite(wp, T("<tr>\n"
		"<td align=left class=tdkind>SNMP</td>\n")
		T("<td align=left><input type=checkbox name=l_snmp value=1></td>\n")
		T("<td align=center><input type=checkbox name=w_snmp value=1></td>\n</tr>\n"));
#endif
	
#ifdef CONFIG_USER_SSH_DROPBEAR
	nBytesSent += websWrite(wp, T("<tr>\n"
		"<td align=left class=tdkind>SSH</td>\n")
		T("<td align=left><input type=checkbox name=l_ssh value=1></td>\n")
		T("<td align=center><input type=checkbox name=w_ssh value=1></td>\n</tr>\n"));
#endif

	
	nBytesSent += websWrite(wp, T("<tr>\n"
		"<td align=left class=tdkind>PING</td>\n")
		T("<td align=left><input type=checkbox name=l_icmp checked disabled></td>\n")
		T("<td align=center><input type=checkbox name=w_icmp value=1></td>\n</tr>\n"));

	
	return nBytesSent;
}


