
/*-- System inlcude files --*/
#include <net/if.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"

#ifdef ADDRESS_MAPPING
#ifdef MULTI_ADDRESS_MAPPING

int showMultAddrMappingTable(int eid, webs_t wp, int argc, char_t * * argv)
{
	int 						nBytesSent=0;
	unsigned int 				entryNum, i;
	MIB_CE_MULTI_ADDR_MAPPING_LIMIT_T 	Entry;
	struct in_addr 			dest;
	char 					lsip[16], leip[16],gsip[16],geip[16];
		
	entryNum = mib_chain_total(MULTI_ADDRESS_MAPPING_LIMIT_TBL);

	nBytesSent += websWrite(wp, 
		T("<tr><td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td>"
		"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Local Start IP</b></font></td>"
		"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Local End IP</b></font></td>"
		"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Global Start IP</b></font></td>"
		"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Global End IP</b></font></td></tr>"));
	
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MULTI_ADDRESS_MAPPING_LIMIT_TBL, i, (void *)&Entry))
		{
			websError(wp, 400, T("Get chain record error!\n"));
			return;
		}

		if (*(unsigned long *)Entry.lsip)
		{
			dest.s_addr = *(unsigned long *)Entry.lsip;
			strcpy(lsip, inet_ntoa(dest));
		}
		else 
			strcpy(lsip, "-");

		if (*(unsigned long *)Entry.leip)
		{
			dest.s_addr = *(unsigned long *)Entry.leip;
			strcpy(leip, inet_ntoa(dest));
		}
		else 
			strcpy(leip, "-");

		if (*(unsigned long *)Entry.gsip)		
		{
			dest.s_addr = *(unsigned long *)Entry.gsip;
			strcpy(gsip, inet_ntoa(dest));
		}
		else 
			strcpy(gsip, "-");

		if (*(unsigned long *)Entry.geip)			
		{
			dest.s_addr = *(unsigned long *)Entry.geip;
			strcpy(geip, inet_ntoa(dest));
		}
		else
			strcpy(geip, "-");


		nBytesSent += websWrite(wp, T("<tr>"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\" ><font size=\"2\"><b>%s</b></font></td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"			
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>"
			"</tr>\n"),
			i, lsip,leip,gsip, geip);

	}	
	return 0;
}

void formAddressMap(webs_t wp, char_t *path, char_t *query)
{
	char							*str, *submitUrl;
	char 						tmpBuf[100];
	struct sockaddr 				conn_ip;
	unsigned char 				connlimitEn;
	MIB_CE_MULTI_ADDR_MAPPING_LIMIT_T	entry;
	int							tmpi;
	unsigned char 				vChar;
	

/* Delete entry */
#if 1
	str = websGetVar(wp, T("deleteSelAddressMap"), T(""));
	if (str[0])
	{
		unsigned int i;
		unsigned int idx;
		unsigned int totalEntry = mib_chain_total(MULTI_ADDRESS_MAPPING_LIMIT_TBL); /* get chain record size */
		unsigned char *strVal;

		config_AddressMap(ACT_STOP);
		for (i=0; i<totalEntry; i++) {

			idx = totalEntry-i-1;			
			snprintf(tmpBuf, 20, "select%d", idx);

			strVal = websGetVar(wp, tmpBuf, T(""));
			if ( !gstrcmp(strVal, T("ON")) ) {						
		
				if(mib_chain_delete(MULTI_ADDRESS_MAPPING_LIMIT_TBL, idx) != 1) {
					strcpy(tmpBuf, T("Delete chain record error!"));
					goto setErr_route;
				}
			}
		}		

		goto setEffect_route;
	}
/*delete all entries*/
	str = websGetVar(wp, T("deleteAllAddressMap"), T(""));
	if (str[0])
	{
		config_AddressMap(ACT_STOP);
		mib_chain_clear(MULTI_ADDRESS_MAPPING_LIMIT_TBL); /* clear chain record */
		goto setEffect_route;
	}
#endif 		
// Add
	str = websGetVar(wp, T("add"), T(""));
	if (str[0]) {
		int mibtotal,i, intVal;
		memset(&entry, 0, sizeof(MIB_CE_MULTI_ADDR_MAPPING_LIMIT_T));
		//addressMapType
		str = websGetVar(wp, T("addressMapType"), T(""));			
		entry.addressMapType = str[0]- '0';
		//ip
		str = websGetVar(wp, T("lsip"), T(""));	
		if (str[0]){
			struct in_addr curIpAddr, curSubnet;
			if ( !inet_aton(str, (struct in_addr *)&entry.lsip) ) {
				strcpy(tmpBuf, T(strIPAddresserror));
				goto setErr_route;
			}				

		}
		//lsip
		str = websGetVar(wp, T("lsip"), T(""));	
		if (str[0]){
			struct in_addr curIpAddr, curSubnet;
			if ( !inet_aton(str, (struct in_addr *)&entry.lsip) ) {
				strcpy(tmpBuf, T(strIPAddresserror));
				goto setErr_route;
			}				
		}
		//leip
		str = websGetVar(wp, T("leip"), T(""));	
		if (str[0]){
			struct in_addr curIpAddr, curSubnet;
			if ( !inet_aton(str, (struct in_addr *)&entry.leip) ) {
				strcpy(tmpBuf, T(strIPAddresserror));
				goto setErr_route;
			}				
		}
		//gsip
		str = websGetVar(wp, T("gsip"), T(""));	
		if (str[0]){
			struct in_addr curIpAddr, curSubnet;
			if ( !inet_aton(str, (struct in_addr *)&entry.gsip) ) {
				strcpy(tmpBuf, T(strIPAddresserror));
				goto setErr_route;
			}				
		}
		//geip
		str = websGetVar(wp, T("geip"), T(""));	
		if (str[0]){
			struct in_addr curIpAddr, curSubnet;
			if ( !inet_aton(str, (struct in_addr *)&entry.geip) ) {
				strcpy(tmpBuf, T(strIPAddresserror));
				goto setErr_route;
			}				
		}

		// check of max enties = 16
		mibtotal = mib_chain_total(MULTI_ADDRESS_MAPPING_LIMIT_TBL);
		if (mibtotal >= 16){
			strcpy(tmpBuf, T("Maximun rules exceeded!"));
				goto setErr_route;
		}

		// delete all NAT Rules
		config_AddressMap(ACT_STOP);
		
		intVal = mib_chain_add(MULTI_ADDRESS_MAPPING_LIMIT_TBL, (void *)&entry);
		if (intVal == 0) {
			//websWrite(wp, T("%s"), strAddChainerror);
			//return;
			strcpy(tmpBuf, T(strAddChainerror));
			goto setErr_route;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, T(strTableFull));
			goto setErr_route;
		}
		else 
			goto setEffect_route;
	}
setOk_route:
	/* upgdate to flash */
	//	mib_update(CURRENT_SETTING);
	//connlimitflag= 1;
	
	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	if (submitUrl[0])
		websRedirect(wp, submitUrl);
	else
		websDone(wp, 200);
	return;

setEffect_route:
	config_AddressMap(ACT_START);
	submitUrl = websGetVar(wp, T("submit-url"), T(""));
	OK_MSG(submitUrl);
	return;

setErr_route:
	ERR_MSG(tmpBuf);	

} 


#else //!MULTI_ADDRESS_MAPPING
void formAddressMap(webs_t wp, char_t *path, char_t *query)
{
	char_t	*str, *submitUrl;
	char tmpBuf[100];
	char_t vChar;
	ADSMAP_T mapType;
	
	str = websGetVar(wp, T("save"), T(""));
	if (str[0]) {
		struct in_addr lsip, leip, gsip, geip;
		
		str = websGetVar(wp, T("addressMapType"), T(""));
		if (!str[0]) {
 			strcpy(tmpBuf, T("Error! No address Map Type!"));
			goto setErr_addressMap;
		}
		
		// delete all NAT Rules
		config_AddressMap(ACT_STOP);
		
		// Mason Yu on True
		mapType = (ADSMAP_T) str[0] - '0';
		if (mapType!=ADSMAP_NONE && mapType!=ADSMAP_ONE_TO_ONE && mapType!=ADSMAP_MANY_TO_ONE
			&& mapType != ADSMAP_MANY_TO_MANY && mapType != ADSMAP_ONE_TO_MANY) {
			strcpy(tmpBuf, T("Invalid Address Map Type." ));
			goto setErr_addressMap;
		}
		vChar = (char_t)mapType;		
		
		// Set MIB_ADDRESS_MAP_TYPE
		if (mib_set( MIB_ADDRESS_MAP_TYPE, (void *)&vChar) == 0) {
  			strcpy(tmpBuf, T("Set MIB_ADDRESS_MAP_TYPE error."));
			goto setErr_addressMap;
		}
		
		// Set MIB_LOCAL_START_IP
		str = websGetVar(wp, T("lsip"), T(""));
		if ( str[0] ) {
			if ( !inet_aton(str, &lsip) ) {
				strcpy(tmpBuf, T("Invalid Local Start IP address value!"));
				goto setErr_addressMap;
			}

			if ( !mib_set(MIB_LOCAL_START_IP, (void *)&lsip)) {
	  			strcpy(tmpBuf, T("Set  MIB_LOCAL_START_IP MIB error!"));
				goto setErr_addressMap;
			}
		}
		
		// Set MIB_LOCAL_END_IP
		str = websGetVar(wp, T("leip"), T(""));
		if ( str[0] ) {
			if ( !inet_aton(str, &leip) ) {
				strcpy(tmpBuf, T("Invalid Local End IP address value!"));
				goto setErr_addressMap;
			}

			if ( !mib_set(MIB_LOCAL_END_IP, (void *)&leip)) {
	  			strcpy(tmpBuf, T("Set  MIB_LOCAL_END_IP MIB error!"));
				goto setErr_addressMap;
			}
		}
		
		// Set MIB_GLOBAL_START_IP
		str = websGetVar(wp, T("gsip"), T(""));
		if ( str[0] ) {
			if ( !inet_aton(str, &gsip) ) {
				strcpy(tmpBuf, T("Invalid Global Start IP address value!"));
				goto setErr_addressMap;
			}

			if ( !mib_set(MIB_GLOBAL_START_IP, (void *)&gsip)) {
	  			strcpy(tmpBuf, T("Set  MIB_GLOBAL_START_IP MIB error!"));
				goto setErr_addressMap;
			}
		}
		
		// Set MIB_GLOBAL_END_IP
		str = websGetVar(wp, T("geip"), T(""));
		if ( str[0] ) {
			if ( !inet_aton(str, &geip) ) {
				strcpy(tmpBuf, T("Invalid Global End IP address value!"));
				goto setErr_addressMap;
			}

			if ( !mib_set(MIB_GLOBAL_END_IP, (void *)&geip)) {
	  			strcpy(tmpBuf, T("Set  MIB_GLOBAL_END_IP MIB error!"));
				goto setErr_addressMap;
			}
		}				
	}	
	
setOk_addressMap:
	/* upgdate to flash */
//	mib_update(CURRENT_SETTING);
	// Setup all NAT Rules
	config_AddressMap(ACT_START);
	
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
	OK_MSG(submitUrl);
  	return;

setErr_addressMap:
	ERR_MSG(tmpBuf);
}
#endif //end of !MULTI_ADDRESS_MAPPING
#endif
