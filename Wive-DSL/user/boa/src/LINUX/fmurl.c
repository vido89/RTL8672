
/*
 *      Web server handler routines for URL stuffs
 *
 */


#include "options.h"
#ifdef URL_BLOCKING_SUPPORT

/*-- System inlcude files --*/
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/route.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#ifdef ZTE_GENERAL_ROUTER_SC
#define  URL_MAX_ENTRY  100
#define  KEY_MAX_ENTRY  100
#else
#define  URL_MAX_ENTRY  500
#define  KEY_MAX_ENTRY  500
#endif
///////////////////////////////////////////////////////////////////
void formURL(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str, *submitUrl, *strVal;
	char tmpBuf[100];				
	unsigned char urlcap;
#ifndef NO_ACTION
	int pid;
#endif

	// Set URL Capability
	str = websGetVar(wp, T("apply"), T(""));
	if (str[0]) {
		str = websGetVar(wp, T("urlcap"), T(""));
		if (str[0]) {
			if (str[0] == '0')
				urlcap = 0;
			else if(str[0] == '1')
				{
				urlcap = 1;
				}
#ifdef  URL_ALLOWING_SUPPORT		
			else if(str[0]=='2')
				{
				   urlcap=2 ;
				}
#endif
			if ( !mib_set(MIB_URL_CAPABILITY, (void *)&urlcap)) {
				strcpy(tmpBuf, T("Set URL Capability error!"));
				goto setErr_route;
			}
		}		
		goto  setOk_route;
 	}
	
	// Delete all FQDN
	str = websGetVar(wp, T("delFAllQDN"), T(""));
	if (str[0]) {
		mib_chain_clear(MIB_URL_FQDN_TBL); /* clear chain record */
		goto setOk_route;
	}

	/* Delete selected FQDN */
	str = websGetVar(wp, T("delFQDN"), T(""));
	if (str[0]) {
		unsigned int i;
		unsigned int idx;
		unsigned int totalEntry = mib_chain_total(MIB_URL_FQDN_TBL); /* get chain record size */
		unsigned int deleted = 0;

		for (i=0; i<totalEntry; i++) {

			idx = totalEntry-i-1;			
			snprintf(tmpBuf, 20, "select%d", idx);
			strVal = websGetVar(wp, tmpBuf, T(""));
			
			if ( !gstrcmp(strVal, T("ON")) ) {						
				deleted ++;
				if(mib_chain_delete(MIB_URL_FQDN_TBL, idx) != 1) {
					strcpy(tmpBuf, T("Delete chain record error!"));
					goto setErr_route;
				}
			}
		}		
		if (deleted <= 0) {
			strcpy(tmpBuf, T("There is no item selected to delete!"));
			goto setErr_route;
		}
		
		goto setOk_route;
	}

#if 0
	// Delete FQDN
	str = websGetVar(wp, T("delFQDN"), T(""));
	if (str[0]) {
		unsigned int i;
		unsigned int idx;
		unsigned int totalEntry = mib_chain_total(MIB_URL_FQDN_TBL); /* get chain record size */		
		
		str = websGetVar(wp, T("select"), T(""));		
		
		if (str[0]) {
			for (i=0; i<totalEntry; i++) {
				idx = totalEntry-i-1;			
				snprintf(tmpBuf, 4, "s%d", idx);
				
				if ( !gstrcmp(str, T(tmpBuf)) ) {					
					// delete from chain record
					if(mib_chain_delete(MIB_URL_FQDN_TBL, idx) != 1) {
						strcpy(tmpBuf, T("Delete URL chain record error!"));
						goto setErr_route;
					}					
				}
			} // end of for
		}		
		goto setOk_route;
	}
#endif

	// Add FQDN
	str = websGetVar(wp, T("addFQDN"), T(""));
	if (str[0]) {
		MIB_CE_URL_FQDN_T entry;		     
              int i, intVal;
              unsigned int totalEntry = mib_chain_total(MIB_URL_FQDN_TBL); /* get chain record size */	
              if((totalEntry+1)>(URL_MAX_ENTRY))
		{
		   strcpy(tmpBuf, TMaxUrl);
		   goto setErr_route;
		}
		str = websGetVar(wp, T("urlFQDN"), T(""));
//		printf("str = %s\n", str);
		for (i = 0 ; i< totalEntry;i++)	{  
		    if (!mib_chain_get(MIB_URL_FQDN_TBL, i, (void *)&entry)){
		       strcpy(tmpBuf, errGetEntry);
			goto setErr_route;
		    
		    }
		    if(!strcmp(entry.fqdn,str)){
		        strcpy(tmpBuf, TstrUrlExist );
		        goto setErr_route;
		    }
		}
					
	         
		// add into configuration (chain record)
		strcpy(entry.fqdn, str);		
		
		intVal = mib_chain_add(MIB_URL_FQDN_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			//websWrite(wp, T("%s"), "Error: Add URL chain record.");
			//return;
			strcpy(tmpBuf, T("Error: Add URL chain record."));
			goto setErr_route;
		}		
		else if (intVal == -1) {
			strcpy(tmpBuf, T(strTableFull));
			goto setErr_route;
		}		
		goto setOk_route;
	}
#ifdef URL_ALLOWING_SUPPORT
       //add allow fqdn 
       str = websGetVar(wp, T("addallowFQDN"), T(""));
	if (str[0]) {
		MIB_CE_URL_ALLOW_FQDN_T entry;		     
              int i, intVal ;
              unsigned int totalEntry = mib_chain_total(MIB_URL_ALLOW_FQDN_TBL); /* get chain record size */	
              if((totalEntry+1)>(URL_MAX_ENTRY))
		{
		   strcpy(tmpBuf, TMaxUrl);
		   goto setErr_route;
		}
		str = websGetVar(wp, T("urlFQDNALLOW"), T(""));
		
		for (i = 0 ; i< totalEntry;i++)	{  
		    if (!mib_chain_get(MIB_URL_ALLOW_FQDN_TBL, i, (void *)&entry)){
		       strcpy(tmpBuf, errGetEntry);
			goto setErr_route;
		    
		    }
		    if(!strcmp(entry.fqdn,str)){
		        strcpy(tmpBuf, TstrUrlExist );
		        goto setErr_route;
		    }
	}

	         
		// add into configuration (chain record)
		strcpy(entry.fqdn, str);		
		
		intVal = mib_chain_add(MIB_URL_ALLOW_FQDN_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			//websWrite(wp, T("%s"), "Error: Add URL chain record.");
			//return;
			strcpy(tmpBuf, T("Error: Add URL chain record."));
			goto setErr_route;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, T(strTableFull));
			goto setErr_route;
		}
	     goto setOk_route;
	}

       // Delete allowFQDN
	str = websGetVar(wp, T("delallowFQDN"), T(""));
	if (str[0]) {
		unsigned int i;
		unsigned int idx;
		unsigned int totalEntry = mib_chain_total(MIB_URL_ALLOW_FQDN_TBL); /* get chain record size */		
		
		str = websGetVar(wp, T("selectallow"), T(""));		
		
		if (str[0]) {
			printf("delallowFQDN\n");
			for (i=0; i<totalEntry; i++) {
				idx = totalEntry-i-1;			
				snprintf(tmpBuf, 4, "s%d", idx);
				
				if ( !gstrcmp(str, T(tmpBuf)) ) {					
					// delete from chain record
					if(mib_chain_delete(MIB_URL_ALLOW_FQDN_TBL, idx) != 1) {
						strcpy(tmpBuf, T("Delete URL chain record error!"));
						goto setErr_route;
					}					
				}
			} // end of for
		}		
		goto setOk_route;
	}

#endif	

	// Delete all Keyword
	str = websGetVar(wp, T("delAllKeywd"), T(""));
	if (str[0]) {
		mib_chain_clear(MIB_KEYWD_FILTER_TBL); /* clear chain record */
		goto setOk_route;
	}

	/* Delete selected Keyword */
	str = websGetVar(wp, T("delKeywd"), T(""));
	if (str[0]) {
		unsigned int i;
		unsigned int idx;
		unsigned int totalEntry = mib_chain_total(MIB_KEYWD_FILTER_TBL); /* get chain record size */
		unsigned int deleted = 0;

		for (i=0; i<totalEntry; i++) {

			idx = totalEntry-i-1;			
			snprintf(tmpBuf, 20, "select%d", idx);
			strVal = websGetVar(wp, tmpBuf, T(""));
			
			if ( !gstrcmp(strVal, T("ON")) ) {						
				deleted ++;
				if(mib_chain_delete(MIB_KEYWD_FILTER_TBL, idx) != 1) {
					strcpy(tmpBuf, T("Delete chain record error!"));
					goto setErr_route;
				}
			}
		}		
		if (deleted <= 0) {
			strcpy(tmpBuf, T("There is no item selected to delete!"));
			goto setErr_route;
		}
		
		goto setOk_route;
	}
#if 0
	// Delete Keyword
	str = websGetVar(wp, T("delKeywd"), T(""));
	if (str[0]) {
		unsigned int i;
		unsigned int idx;
		unsigned int totalEntry = mib_chain_total(MIB_KEYWD_FILTER_TBL); /* get chain record size */		
		
		str = websGetVar(wp, T("select"), T(""));		
		
		if (str[0]) {
			for (i=0; i<totalEntry; i++) {
				idx = totalEntry-i-1;			
				snprintf(tmpBuf, 4, "s%d", idx);
				
				if ( !gstrcmp(str, T(tmpBuf)) ) {					
					// delete from chain record
					if(mib_chain_delete(MIB_KEYWD_FILTER_TBL, idx) != 1) {
						strcpy(tmpBuf, T("Delete Keyword filter chain record error!"));
						goto setErr_route;
					}					
				}
			} // end of for
		}		
		goto setOk_route;
	}
#endif	

	// Add keyword
	str = websGetVar(wp, T("addKeywd"), T(""));
	if (str[0]) {
		MIB_CE_KEYWD_FILTER_T entry;	
		 int i, intVal;
              unsigned int totalEntry = mib_chain_total(MIB_KEYWD_FILTER_TBL); /* get chain record size */	
              if((totalEntry+1)>(KEY_MAX_ENTRY))
		{
		   strcpy(tmpBuf, TMaxKey);
		   goto setErr_route;
		}
		str = websGetVar(wp, T("Keywd"), T(""));
	//	printf("str = %s\n", str);
		for (i = 0 ; i< totalEntry;i++)	{  
		    if (!mib_chain_get(MIB_KEYWD_FILTER_TBL,i, (void *)&entry)){
		       strcpy(tmpBuf, errGetEntry);
			goto setErr_route;
		    
		    }
		    if(!strcmp(entry.keyword, str)){
		        strcpy(tmpBuf, TstrKeyExist);
		        goto setErr_route;
		    }
		}
		
		
		
		// add into configuration (chain record)
		strcpy(entry.keyword, str);		
		
		intVal = mib_chain_add(MIB_KEYWD_FILTER_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			//websWrite(wp, T("%s"), "Error: Add Keyword filtering chain record.");
			//return;
			strcpy(tmpBuf, T("Error: Add Keyword filtering chain record."));
			goto setErr_route;
		}		
		else if (intVal == -1) {
			strcpy(tmpBuf, T(strTableFull));
			goto setErr_route;
		}		
	}
setOk_route:
	/* upgdate to flash */
//	mib_update(CURRENT_SETTING);
#ifdef URL_ALLOWING_SUPPORT
       restart_url();
#else
	restart_urlblocking();
#endif 

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

	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
  	return;

setErr_route:
	ERR_MSG(tmpBuf);
}

int showURLTable(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;
	
	unsigned int entryNum, i;
	MIB_CE_URL_FQDN_T Entry;	
	
	entryNum = mib_chain_total(MIB_URL_FQDN_TBL);
#ifdef ZTE_GENERAL_ROUTER_SC	
	nBytesSent += websWrite(wp, T("<tr><font size=1>"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\">选择</td>\n"
	"<td align=center width=\"35%%\" bgcolor=\"#808080\">全域名</td></font></tr>\n"));
#else 
	nBytesSent += websWrite(wp, T("<tr><font size=1>"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\">Select</td>\n"
	"<td align=center width=\"35%%\" bgcolor=\"#808080\">FQDN</td></font></tr>\n"));

#endif

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_URL_FQDN_TBL, i, (void *)&Entry))
		{
  			websError(wp, 400, T("Get URL chain record error!\n"));
			return;
		}	
	
		nBytesSent += websWrite(wp, T("<tr>"
//		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"select\""
//		" value=\"s%d\"></td>\n"
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td>\n"
		"<td align=center width=\"35%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td></tr>\n"),
		i, Entry.fqdn);
	}
#ifdef ZTE_GENERAL_ROUTER_SC
	        nBytesSent += websWrite(wp, T("<input type=\"hidden\" value=%d name=\"urllistnumber\">"), i);
#endif

	
	return 0;
}
#ifdef URL_ALLOWING_SUPPORT
int showURLALLOWTable(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;
	
	unsigned int entryNum, i;
	MIB_CE_URL_ALLOW_FQDN_T Entry;	
	
	
	entryNum = mib_chain_total(MIB_URL_ALLOW_FQDN_TBL);
#ifdef ZTE_GENERAL_ROUTER_SC	
	nBytesSent += websWrite(wp, T("<tr><font size=1>"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\">选择</td>\n"
	"<td align=center width=\"35%%\" bgcolor=\"#808080\">全域名</td></font></tr>\n"));
#else 
	nBytesSent += websWrite(wp, T("<tr><font size=1>"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\">Select</td>\n"
	"<td align=center width=\"35%%\" bgcolor=\"#808080\">FQDN</td></font></tr>\n"));

#endif

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_URL_ALLOW_FQDN_TBL, i, (void *)&Entry))
		{
  			websError(wp, 400, T("Get URL chain record error!\n"));
			return;
		}	
	
		nBytesSent += websWrite(wp, T("<tr>"
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"selectallow\""
		" value=\"s%d\"></td>\n"
		"<td align=center width=\"35%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td></tr>\n"),
		i, Entry.fqdn);
	}
#ifdef ZTE_GENERAL_ROUTER_SC
	        nBytesSent += websWrite(wp, T("<input type=\"hidden\" value=%d name=\"urlallowlistnumber\">"), i);
#endif

	
	return 0;
}

#endif

int showKeywdTable(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;
	
	unsigned int entryNum, i;
	MIB_CE_KEYWD_FILTER_T Entry;		
	
	entryNum = mib_chain_total(MIB_KEYWD_FILTER_TBL);
#ifdef ZTE_GENERAL_ROUTER_SC	
	nBytesSent += websWrite(wp, T("<tr><font size=1>"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\">选择</td>\n"
	"<td align=center width=\"35%%\" bgcolor=\"#808080\">过滤关键词</td></font></tr>\n"));
#else
	nBytesSent += websWrite(wp, T("<tr><font size=1>"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\">Select</td>\n"
	"<td align=center width=\"35%%\" bgcolor=\"#808080\">Filtered Keyword</td></font></tr>\n"));

#endif
	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_KEYWD_FILTER_TBL, i, (void *)&Entry))
		{
  			websError(wp, 400, T("Get Keyword filter chain record error!\n"));
			return;
		}		
	
		nBytesSent += websWrite(wp, T("<tr>"
//		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"select\""
//		" value=\"s%d\"></td>\n"
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td>\n"
		"<td align=center width=\"35%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td></tr>\n"),
		i, Entry.keyword);
	}
#ifdef ZTE_GENERAL_ROUTER_SC
	      nBytesSent += websWrite(wp, T("<input type=\"hidden\" value=%d name=\"keylistnumber\">"), i);
#endif
	
	return 0;
}
#endif

