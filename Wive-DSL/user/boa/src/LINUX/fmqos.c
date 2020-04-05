/*
 *      Web server handler routines for IP QoS
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
#include "utility.h"

#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../../uClibc/include/linux/autoconf.h"
#endif

#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
#ifdef QOS_SPEED_LIMIT_SUPPORT
void mib_qos_speed_limit_check(MIB_CE_IP_QOS_Tp entry)
{
	int mibspeedindex=mib_qos_speed_limit_existed(entry->limitSpeedRank,entry->prior);
	MIB_CE_IP_QOS_SPEEDRANK_T qosSpeedEntry;
	mib_chain_get(MIB_QOS_SPEED_LIMIT,mibspeedindex,&qosSpeedEntry);
	qosSpeedEntry.count--;
	if(qosSpeedEntry.count==0)
		{
		printf("delete the entry:speed=%d,prior=%d\n",entry->limitSpeedRank,entry->prior);
		mib_chain_delete(MIB_QOS_SPEED_LIMIT,mibspeedindex);
		}
	else
		mib_chain_update(MIB_QOS_SPEED_LIMIT,&qosSpeedEntry,mibspeedindex);
}
#endif
/////////////////////////////////////////////////////////////////////////////
// Jenny, for checking duplicated IP QoS rule
int checkRule_ipqos(MIB_CE_IP_QOS_T qosEntry, unsigned char *sip, unsigned char *dip)
{
	int totalEntry, i;
	MIB_CE_IP_QOS_T Entry;
	char *temp;
	long nSip, nDip;

	totalEntry = mib_chain_total(MIB_IP_QOS_TBL);

	temp = inet_ntoa(*((struct in_addr *)sip));
	nSip = ntohl(inet_addr(temp));
	temp = inet_ntoa(*((struct in_addr *)dip));
	nDip = ntohl(inet_addr(temp));
	if (nSip == nDip && nSip != 0x0)
		return 0;	
		
	for (i=0; i<totalEntry; i++) {
		unsigned long v1, v2, pSip, pDip;
		int m;

		if (!mib_chain_get(MIB_IP_QOS_TBL, i, (void *)&Entry))
			return 0;
		temp[0] = '\0';
		temp = inet_ntoa(*((struct in_addr *)Entry.sip));
		v1 = ntohl(inet_addr(temp));
		v2 = 0xFFFFFFFFL;
		for (m=32; m>Entry.smaskbit; m--) {
			v2 <<= 1;
			v2 |= 0x80000000;
		}
		pSip = v1&v2;
		temp[0] = '\0';
		temp = inet_ntoa(*((struct in_addr *)Entry.dip));
		v1 = ntohl(inet_addr(temp));
		v2 = 0xFFFFFFFFL;
		for (m=32; m>Entry.dmaskbit; m--) {
			v2 <<= 1;
			v2 |= 0x80000000;
		}
		pDip = v1&v2;

		// If all parameters of Entry are all the same as mew rule, drop this new rule.
#ifdef QOS_DIFFSERV
		if (Entry.enDiffserv == qosEntry.enDiffserv
			&& Entry.m_ipprio == qosEntry.m_ipprio && Entry.m_iptos == qosEntry.m_iptos)
			return 2;
#endif
		if (nSip == pSip && nDip == pDip &&
			Entry.sPort == qosEntry.sPort && Entry.dPort == qosEntry.dPort &&
			Entry.protoType == qosEntry.protoType && Entry.phyPort == qosEntry.phyPort
//#ifdef NEW_IP_QOS_SUPPORT
#ifdef QOS_DSCP_MATCH
			&& Entry.qosDscp == qosEntry.qosDscp
#endif
			/* && Entry.vlan1p == qosEntry.vlan1p*/
//#endif
#ifdef QOS_DIFFSERV
			&& Entry.enDiffserv == qosEntry.enDiffserv
			&& Entry.m_ipprio == qosEntry.m_ipprio && Entry.m_iptos == qosEntry.m_iptos
			&& Entry.limitSpeed == qosEntry.limitSpeed && Entry.policing == qosEntry.policing
#endif
			)
				return 0;
	}
	return 1;
}

#ifdef NEW_IP_QOS_SUPPORT
#define PRINT_TRAFFICTL_RULE(pEntry) \
    printf("[TRAFFIC CONTROL]: entryid:%d, ifIndex:%d, srcip:%s, smaskbits:%d, dstip:%s, dmaskbits:%d,"  \
	   "sport:%d, dport%d, protoType:%d, limitspeed:%d\n",                                       \
	    pEntry->entryid, pEntry->ifIndex, inet_ntoa(*((struct in_addr*)&pEntry->srcip)),        \
	    pEntry->smaskbits, inet_ntoa(*((struct in_addr*)&pEntry->dstip)), pEntry->dmaskbits,    \
	    pEntry->sport, pEntry->dport, pEntry->protoType, pEntry->limitSpeed);

//star: to get ppp index for wanname
static int getpppindex(MIB_CE_ATM_VC_T *pEntry)
{
	int ret=-1;
	int mibtotal,i,num=0,totalnum=0;
	unsigned char pppindex,tmpindex;
	MIB_CE_ATM_VC_T Entry;

	if(pEntry->cmode != ADSL_PPPoE && pEntry->cmode != ADSL_PPPoA)
		return ret;
	
	pppindex = (pEntry->ifIndex >> 4) & 0x0f;
	if(pppindex == 0x0f)
		return ret;

	mibtotal = mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0;i<mibtotal;i++)
	{
		if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			continue;
		if(Entry.cmode != ADSL_PPPoE && Entry.cmode != ADSL_PPPoA)
			continue;
		tmpindex = (Entry.ifIndex >> 4) & 0x0f;
		if(tmpindex == 0x0f)
			continue;
		if(Entry.vpi == pEntry->vpi && Entry.vci == pEntry->vci)
		{
			totalnum++;
			if(tmpindex < pppindex)
				num++;
		}
	}

	if(totalnum > 1)
		ret = num;

	return ret;

}

static int generateWanInfName(MIB_CE_ATM_VC_T *entry, char* wanname)
{
	char vpistr[6];
	char vcistr[6];
	
	if(entry==NULL || wanname ==NULL)
		return -1;
	memset(vpistr, 0, sizeof(vpistr));
	memset(vcistr, 0, sizeof(vcistr));
	strcpy(wanname, "Internet_");

	if(entry->cmode == ADSL_BR1483)
		strcat(wanname, "B_");
	else
		strcat(wanname, "R_");
	sprintf(vpistr, "%d", entry->vpi);
	sprintf(vcistr, "%d", entry->vci);
	strcat(wanname, vpistr);
	strcat(wanname, "_");
	strcat(wanname, vcistr);
	//star: for multi-ppp in one pvc
	if(entry->cmode == ADSL_PPPoE || entry->cmode == ADSL_PPPoA)
	{	
		char pppindex[6];
		int intindex;
		intindex = getpppindex(entry);
		if(intindex != -1){
			snprintf(pppindex,6,"%u",intindex);
			strcat(wanname, "_");
			strcat(wanname, pppindex);
		}
	}
	
	return 0;
}

static int getWanInfName(MIB_CE_ATM_VC_T *pEntry, char* name)
{
	if(pEntry==NULL || name==NULL)
				return 0;
#ifdef _CWMP_MIB_	
	if(pEntry->WanName[0])
		strcpy(name, pEntry->WanName);
	else
#endif		
	{//if not set by ACS. then generate automaticly.  
		generateWanInfName(pEntry, name);
	}
	return 1;
}

char * getWanNameFromIfIndex(unsigned char ifIndex, char *wanname)
{
	MIB_CE_ATM_VC_T entry;
	int i, entryNum;

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<entryNum; i++)
	{
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entry))
			continue;
		if (entry.ifIndex == ifIndex)
			break;
	}
	if (i>=entryNum) {
		printf("not find matched interface!\n");
		return NULL;
	}

	getWanInfName(&entry, wanname);

	return wanname;
}

//ql: 20081117 add traffic shaping
int initTraffictlPage(int eid, webs_t wp, int argc, char_t **argv)
{
	MIB_CE_IP_TC_T entry;
	int entryNum = 0, i=0, nBytes = 0;
	char ifname[40];
	char sip[20], dip[20], *p = NULL;
	unsigned short total_bandwidth = 0;
	unsigned char totalBandWidthEn=0;

	entryNum = mib_chain_total(MIB_IP_QOS_TC_TBL);
	
	mib_get(MIB_TOTAL_BANDWIDTH, &total_bandwidth);
	mib_get(MIB_TOTAL_BANDWIDTH_LIMIT_EN, &totalBandWidthEn);
		
	nBytes += websWrite(wp, "totalBandWidthEn=%d;\n", totalBandWidthEn);
	if (totalBandWidthEn) {
		nBytes += websWrite(wp, "totalBandwidth=%d;\n", total_bandwidth);
	} else {
		nBytes += websWrite(wp, "totalBandwidth=0;\n");
	}

    for(;i<entryNum; i++)
	{
		if(!mib_chain_get(MIB_IP_QOS_TC_TBL, i, &entry))
		    continue;
		strncpy(sip, inet_ntoa(*((struct in_addr*)&entry.srcip)), 16);
		strncpy(dip, inet_ntoa(*((struct in_addr*)&entry.dstip)), 16);
		if(entry.srcip && entry.smaskbits)
		{
		    p = sip + strlen(sip);
		    snprintf(p,sizeof(sip)-strlen(sip), "/%d", entry.smaskbits );
		}
		if(entry.dstip && entry.dmaskbits)
		{
		    p = dip + strlen(dip);
		    snprintf(p,sizeof(dip)-strlen(dip), "/%d", entry.dmaskbits );
		}
		
		if (getWanNameFromIfIndex(entry.ifIndex, ifname)==NULL) {
			printf("fatal error! didn't find such interface\n");
			continue;
		}
		
		nBytes += websWrite(wp, "traffictlRules.push(new it_nr(%d, \"%s\", %d, %d, %d, \"%s\", \"%s\", %d));\n", 
			      entry.entryid, ifname, entry.protoType, 
			      entry.sport, entry.dport, sip, dip, entry.limitSpeed);
	}
    return nBytes;
}

#define TRAFFICTL_RULE_NUM_MAX 256
#define DELIM      '&'
#define SUBDELIM   '|'
#define SUBDELIM1  "|"

//Used to get subnet mask bit number 
static int getNetMaskBit(char* netmask)
{
    unsigned int bits = 0, mask = 0;
    int i=0, flag = 0;

    if(!netmask||strlen(netmask)>15)
	return 0;
    mask = inet_network(netmask);
    for(;i<32;i++)
    {
	if(mask&(0x80000000>>i)) {
	    if(flag)
		return 0;
	    else
		bits++;
	}
	else {
	    flag = 1;
	}
    }
    return bits;
}

static int parseArgs(char* action, MIB_CE_IP_TC_Tp pEntry)
{
    char* p = NULL, *tmp = NULL;
    int i = 0;
    
	//ifIndex
	tmp = strstr(action, "inf=");
	tmp += strlen("inf=");
	if (!tmp || *tmp==DELIM)
		return 1;
	else {
		pEntry->ifIndex = strtol(tmp, &p,0);
		if (*p != DELIM)
			return 1;
	}

    //protocol
    tmp =strstr(action, "proto=");
    tmp += strlen("proto=");
    if(!tmp||*tmp == DELIM)//not set protocol, set it to default,none
		pEntry->protoType = 0;
    else
    {
		pEntry->protoType = strtol(tmp, &p, 0);
		if(*p != DELIM)
	    	return 1;
    }

    //source ip
    tmp = strstr(action, "srcip=");
    tmp += strlen("srcip=");
    if(!tmp||*tmp == DELIM)//noet set, set default
		pEntry->srcip = 0;
    else
    {
		char sip[16]={0};
		struct in_addr addr;
		p = strchr(tmp, DELIM);
		if(p&&p-tmp>15)
			return 1;
		strncpy(sip, tmp, p-tmp);
		if(!inet_aton(sip, &addr))
			return 1;
		pEntry->srcip = addr.s_addr;
    }

    //source ip address netmask
    tmp = strstr(action, "srcnetmask=");
    tmp += strlen("srcnetmask=");
    if(!tmp||*tmp==DELIM)
		pEntry->smaskbits = 0;
    else
    {
		char smask[16]={0};
		p = strchr(tmp, DELIM);
		if(p&&p-tmp>15) return 1;
		strncpy(smask, tmp, p-tmp);
		pEntry->smaskbits = getNetMaskBit(smask);
    }

    //destination ip
    tmp = strstr(action, "dstip=");
    tmp += strlen("dstip=");
    if(!tmp||*tmp == DELIM)//noet set, set default
		pEntry->dstip = 0;
    else
    {
		char dip[16]={0};
		struct in_addr addr;
		p = strchr(tmp, DELIM);
		if(p&&p-tmp>15)
			return 1;
		strncpy(dip, tmp, p-tmp);
		if(!inet_aton(dip, &addr))
			return 1;
		pEntry->dstip = addr.s_addr;
    }

    //destination ip address netmask
    tmp = strstr(action, "dstnetmask=");
    tmp += strlen("dstnetmask=");
    if(!tmp||*tmp==DELIM)
		pEntry->dmaskbits = 0;
    else
    {
		char dmask[16]={0};
		p = strchr(tmp, DELIM);
		if(p&&p-tmp>15)
			return 1;
		strncpy(dmask, tmp, p-tmp);
		pEntry->dmaskbits = getNetMaskBit(dmask);

    }

    //source port
    tmp = strstr(action, "sport=");
    tmp += strlen("sport=");
    if(!tmp||*tmp==DELIM)
		pEntry->sport = 0;
    else
    {
		pEntry->sport = strtol(tmp, &p, 0);
		if(*p != DELIM)
			return 1;
    }

    //destination port
    tmp = strstr(action, "dport=");
    tmp += strlen("dport=");
    if(!tmp||*tmp==DELIM)
		pEntry->dport = 0;
    else
    {
		pEntry->dport = strtol(tmp, &p, 0);
		if(*p != DELIM)
	return 1;
}

    //upstream rate
    tmp = strstr(action, "uprate=");
    tmp += strlen("uprate=");
    if(!tmp||*tmp=='\0')
		pEntry->limitSpeed = 0;
    else
    {
		pEntry->limitSpeed = strtol(tmp, &p, 0);
		if(*p != '\0')
			return 1;
    }

    return 0;
}

void formQosShape(webs_t wp, char_t *path, char_t *query)
{
	char *action = NULL, *url = NULL;
	char *act1="applybandwidth";
	char *act2="applysetting";
	char *act3="addsetting";
	int entryNum = 0;

	entryNum = mib_chain_total(MIB_IP_QOS_TC_TBL);  
	action = websGetVar(wp, T("lst"), T("")); 
	
	if(action[0])
	{
		if( !strncmp(action, act1, strlen(act1)) )
		{//set total bandwidth
			unsigned short totalbandwidth = 0;
			unsigned char totalbandwidthEn=0;
			char* strbandwidth = NULL;
			strbandwidth = strstr(action, "bandwidth=");
			strbandwidth += strlen("bandwidth=");
			if(strbandwidth)//found it
			{
				totalbandwidth = atoi(strbandwidth);

				if (totalbandwidth) {
					totalbandwidthEn = 1;
				} else {
					totalbandwidthEn = 0;
				}
			}
			else {
				totalbandwidthEn = 0;
				totalbandwidth = 0;
			}
			
			mib_set(MIB_TOTAL_BANDWIDTH_LIMIT_EN, (void *)&totalbandwidthEn);
			mib_set(MIB_TOTAL_BANDWIDTH, (void*)&totalbandwidth);

			//take effect
			take_qos_effect();
		}
		else if( !strncmp(action, act2, strlen(act2)) )
		{//delete some
			int idlst[TRAFFICTL_RULE_NUM_MAX+1] = {0};
			char stridlst[256],err_msg[256], *p = NULL;
			MIB_CE_IP_TC_T entry;
			int  i=0, index=1;

			p = strstr(action, "id=");
			p += strlen("id=");
			if(*p == '\0') {//delete none
				goto done;
			}

			stridlst[0] = '\0';
			strncpy(stridlst, p, 256);

			//convert the id list, store them in idlst, 
			//you can delete most 10 rules at one time
			p = strtok(stridlst, SUBDELIM1);
			if(p) index = atoi(p);
			if(index>0&&index<=TRAFFICTL_RULE_NUM_MAX) idlst[index]=1;
			while((p = strtok(NULL, SUBDELIM1)) != NULL)
			{
				index = atoi(p);
				idlst[index]=1;
				if(index > TRAFFICTL_RULE_NUM_MAX )
				    break;
			}

			for(i=entryNum-1; i>=0; i--)
			{
				if(!mib_chain_get(MIB_IP_QOS_TC_TBL, i, &entry))
				    continue;

				if( 1 == idlst[entry.entryid]) //delete it
				{
		            //delete rules of  tc and iptables
				    if(1 != mib_chain_delete(MIB_IP_QOS_TC_TBL, i)) {
						snprintf(err_msg, 256, "Error happened when deleting rule %d", entry.entryid);
						ERR_MSG(err_msg);
						return;
				    }
				}
			}

done:
			take_qos_effect();
		}
		else if ( !strncmp(action, act3, strlen(act3)) )
		{
			MIB_CE_IP_TC_T entry;
			unsigned char map[TRAFFICTL_RULE_NUM_MAX+1]={0};
			int entryid = 1;
			int i = 0;
			
			if(entryNum>=TRAFFICTL_RULE_NUM_MAX)
			{
				ERR_MSG("Traffic controlling queue is full, you must delete some one!");
				return;
			}

			//allocate a free rule id for new entry
			for(;i<entryNum;i++)
			{
				if(!mib_chain_get(MIB_IP_QOS_TC_TBL, i, &entry))
				    continue;

				map[entry.entryid] = 1;
			}
			for(i=1;i<=TRAFFICTL_RULE_NUM_MAX;i++)
			{
				if(!map[i])
				{
					entryid = i;
					break;
				}
			}

			memset(&entry, 0, sizeof(MIB_CE_IP_TC_T));
			entry.entryid = entryid;

			if(parseArgs(action, &entry))
			{//some arguments are wrong
				ERR_MSG("Wrong setting is found!");
				return;
			}

			PRINT_TRAFFICTL_RULE((&entry));

			if(!mib_chain_add(MIB_IP_QOS_TC_TBL, &entry))
			{//adding mib setting is wrong 
				ERR_MSG("Cannot add setting into mib!");
				return;
			}
		}
	}
	//mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);

	//well, go back
	url = websGetVar(wp, T("submit-url"), T(""));
	if(url[0])
		websRedirect(wp, url);
	
	return;
}
#endif

void formQos(webs_t wp, char_t *path, char_t *query)
{
	char_t *submitUrl, *strAddQos, *strDelQos, *strVal, *strDelAllQos;
	char_t *strSAddr, *strDAddr, *strSport, *strDport;
	char tmpBuf[100];
	int intVal;
	unsigned int entryNum, totalEntry, i;
	MIB_CE_IP_QOS_T entry;
	struct in_addr curIpAddr, curSubnet;
	unsigned long v1, v2, v3, mask, mbit;
	unsigned char vChar, vChar_mode;
	int itfid, itfdomain;
#ifndef NO_ACTION
	int pid;
#endif
#ifdef CONFIG_8021P_PRIO
      	 char_t *strSet1p,*strSet1p_prio;
       	const char *nset1ptbl;
#ifdef NEW_IP_QOS_SUPPORT
	char_t *strSetPred, *strSetPred_prio;
	const char *nsetPredtbl;
#endif
	unsigned char m;
	unsigned char value[8];
#endif
	unsigned char mode=0;
	mib_get(MIB_MPMODE, (void *)&mode);
#ifdef IP_QOS//ql 20081117 START when NEW_IP_QOS_SUPPORT enable, dont need to stop IP QoS firstly.
	if (mode&0x02) stopIPQ();
#endif
	 
	strVal = websGetVar(wp, T("admin"), T(""));
	
	// enable/disable Qos
	if (strVal[0]) {
		// bitmap for virtual lan port function
		// Port Mapping: bit-0
		// QoS : bit-1
		
		//ql 20081119 START get qos policy
#ifdef NEW_IP_QOS_SUPPORT
		unsigned char policy;
		strVal = websGetVar(wp, T("qosPolicy"), T(""));
		if (strVal[0] == '1')
			policy = 1;
		else
			policy = 0;
		mib_set(MIB_QOS_POLICY, (void *)&policy);
#endif
		
		mib_get(MIB_MPMODE, (void *)&vChar_mode);
		strVal = websGetVar(wp, T("qosen"), T(""));
		if ( strVal[0] == '1' ) {
			vChar_mode |= 0x02;
#ifdef QOS_DIFFSERV
			vChar = 0;
			mib_set(MIB_QOS_DIFFSERV, (void *)&vChar);
#endif
		}
		else
			vChar_mode &= 0xfd;	
		mib_set(MIB_MPMODE, (void *)&vChar_mode);
      //if define re8305 cannot user 802.1p prio
#ifndef CONFIG_RE8305
		if ( strVal[0] == '1' ) {
			strVal = websGetVar(wp, T("qosdmn"), T(""));
			vChar = strVal[0] - '0';
			mib_set(MIB_QOS_DOMAIN, (void *)&vChar);
		}
#endif
#if defined(APPLY_CHANGE)
#ifdef IP_QOS
		if (vChar_mode&0x02) setupIPQ();
#else
#ifdef NEW_IP_QOS_SUPPORT
		take_qos_effect();
#endif
#endif
#endif
		submitUrl = websGetVar(wp, T("submit-url"), T(""));
             
		OK_MSG(submitUrl);
	  	return;
	}
#ifdef CONFIG_8021P_PRIO
	strSet1p= websGetVar(wp, T("set1p"), T(""));
       if (strSet1p[0]){
	   	if(mib_get(MIB_8021P_PROI, (void *)value)== 0)
		{
   			strcpy(tmpBuf, T("Get 802.1p Mib error"));
			goto setErr_qos;
		}        

	   	for(i=0;i<IPQOS_NUM_PKT_PRIO;i++)
	     {
	         nset1ptbl =set1ptable[i];
		  strSet1p_prio=websGetVar(wp, nset1ptbl, T(""));
		  if(strSet1p_prio[0]){
		  	vChar = strSet1p_prio[0] - '0';
			value[i]=vChar;		  
		  }
	     }
	     mib_set(MIB_8021P_PROI, (void *)value);
#if defined(APPLY_CHANGE)
#ifdef NEW_IP_QOS_SUPPORT
		take_qos_effect();
#endif
#endif
		submitUrl = websGetVar(wp, T("submit-url"), T(""));
		OK_MSG(submitUrl);
	  	return;
	}
	
#ifdef NEW_IP_QOS_SUPPORT
	strSetPred = websGetVar(wp, T("setpred"), T(""));
	if (strSetPred[0]){
		if (mib_get(MIB_PRED_PROI, (void *)value) == 0)
		{
			strcpy(tmpBuf, T("Get PRED Mib error"));
			goto setErr_qos;
		}

		for (i=0; i<IPQOS_NUM_PKT_PRIO; i++)
		{
			nsetPredtbl = setpredtable[i];
			strSetPred_prio = websGetVar(wp, nsetPredtbl, T(""));
			if (strSetPred_prio[0]){
				vChar = strSetPred_prio[0] - '0';
				value[i] = vChar;
			}
		}
		mib_set(MIB_PRED_PROI, (void *)value);
#if defined(APPLY_CHANGE)
#ifdef NEW_IP_QOS_SUPPORT
		take_qos_effect();
#endif
#endif
		submitUrl = websGetVar(wp, T("submit-url"), T(""));
		OK_MSG(submitUrl);
	  	return;
	}
#endif	
#endif
#ifdef QOS_SPEED_LIMIT_SUPPORT
          char *bandwidthctrl=websGetVar(wp,T("bandwidthctrl"),T(""));
	if(bandwidthctrl[0]){
		char * upbandwidth=websGetVar(wp,T("upbandwidth"),T(""));
		printf("upbandwidth=%s\n",upbandwidth);
		if ( !string_to_dec(upbandwidth, &intVal) ) {
					strcpy(tmpBuf, T(Tinvalid_pvc_bandwidth));
					goto setErr_qos;
				}
		unsigned short shortVal=(unsigned short)intVal;
		if(!mib_set(MIB_PVC_TOTAL_BANDWIDTH,&shortVal))
			goto setErr_qos;
		else goto setOk_qos;
		
	}		
#endif
	strAddQos = websGetVar(wp, T("addqos"), T(""));
	strDelQos = websGetVar(wp, T("delSel"), T(""));
	strDelAllQos = websGetVar(wp, T("delAll"), T(""));
	
	memset(&entry, 0, sizeof(entry));
	totalEntry = mib_chain_total(MIB_IP_QOS_TBL); /* get chain record size */
	
	/* Add new qos entry */
	if (strAddQos[0]) {
		int intVal;
		unsigned char smask[4], dmask[4], sip[4], dip[4];
		memset(sip, 0x0, 4);
		memset(dip, 0x0, 4);
		if (totalEntry >= MAX_QOS_RULE)
		{
			strcpy(tmpBuf, T(Texceed_max_rules));
			goto setErr_qos;
		}
		
#ifdef _CWMP_MIB_
		// enable
		entry.enable = 1;
#endif
#ifdef NEW_IP_QOS_SUPPORT
		//strVal = websGetVar(wp, T("s_m1p"), T(""));
		//entry.vlan1p = strVal[0] - '0';
		entry.vlan1p = 0;
		
#ifdef QOS_DSCP_MATCH
		strVal = websGetVar(wp, T("s_dscp"), T(""));
		if (strVal[0]) {
			entry.qosDscp = strtol(strVal, (char**)NULL, 0);
		} else
#endif
			entry.qosDscp = 0;
#endif
		// protocol type
		strVal = websGetVar(wp, T("prot"), T(""));
		entry.protoType = strVal[0] - '0';

		strSAddr = websGetVar(wp, T("sip"), T(""));
		strDAddr = websGetVar(wp, T("dip"), T(""));
		
		// Source address
		if (strSAddr[0]) {
			inet_aton(strSAddr, (struct in_addr *)&entry.sip);
			if ((entry.sip[0]=='\0') &&(entry.sip[1]=='\0') && (entry.sip[2]=='\0') && (entry.sip[3]=='\0')) {
				strcpy(tmpBuf, T(Tinvalid_source_ip));
				goto setErr_qos;
			}
			
			strSAddr = websGetVar(wp, T("smask"), T(""));
			if (strSAddr[0]) {
				if (!isValidNetmask(strSAddr, 1)) {
					strcpy(tmpBuf, T(Tinvalid_source_netmask));
					goto setErr_qos;
				}
				inet_aton(strSAddr, (struct in_addr *)smask);
				inet_aton(strSAddr, (struct in_addr *)&mask);
				if (mask==0) {
					strcpy(tmpBuf, T(Tinvalid_source_netmask));
					goto setErr_qos;
				}
				mask = htonl(mask);
				mbit=0; intVal=0;
				for (i=0; i<32; i++) {
					if (mask&0x80000000) {
						if (intVal) {
							strcpy(tmpBuf, T(Tinvalid_source_netmask));
							goto setErr_qos;
						}
						mbit++;
					}
					else
						intVal=1;
					mask <<= 1;
				}
				entry.smaskbit = mbit;
			}
			else {
				entry.smaskbit = 32;
				inet_aton(ARG_255x4, (struct in_addr *)smask);
			}

			// Jenny, for checking duplicated source address
			sip[0] = entry.sip[0] & smask[0];
			sip[1] = entry.sip[1] & smask[1];
			sip[2] = entry.sip[2] & smask[2];
			sip[3] = entry.sip[3] & smask[3];
		}
		
		// source port
		if (entry.protoType == PROTO_TCP || entry.protoType == PROTO_UDP) {
			strSport = websGetVar(wp, T("sport"), T(""));
			
			if (strSport[0]) {
				if ( !string_to_dec(strSport, &intVal) || intVal<1 || intVal>65535) {
					strcpy(tmpBuf, T(Tinvalid_source_port));
					goto setErr_qos;
				}
				entry.sPort = (unsigned short)intVal;
			}
		}

		// Destination address
		if (strDAddr[0]) {
			inet_aton(strDAddr, (struct in_addr *)&entry.dip);
			if ((entry.dip[0]=='\0') && (entry.dip[1]=='\0') && (entry.dip[2]=='\0') && (entry.dip[3]=='\0')) {
				strcpy(tmpBuf, T(Tinvalid_destination_ip));
				goto setErr_qos;
			}
			
			strDAddr = websGetVar(wp, T("dmask"), T(""));
			if (strDAddr[0]) {
				if (!isValidNetmask(strDAddr, 1)) {
					strcpy(tmpBuf, T(Tinvalid_destination_netmask));
					goto setErr_qos;
				}
				inet_aton(strDAddr, (struct in_addr *)dmask);
				inet_aton(strDAddr, (struct in_addr *)&mask);
				if (mask==0) {
					strcpy(tmpBuf, T(Tinvalid_destination_netmask));
					goto setErr_qos;
				}
				mask = htonl(mask);
				mbit=0; intVal=0;
				for (i=0; i<32; i++) {
					if (mask&0x80000000) {
						if (intVal) {
							strcpy(tmpBuf, T(Tinvalid_destination_netmask));
							goto setErr_qos;
						}
						mbit++;
					}
					else
						intVal=1;
					mask <<= 1;
				}
				entry.dmaskbit = mbit;
			}
			else {
				entry.dmaskbit = 32;
				inet_aton(ARG_255x4, (struct in_addr *)dmask);
			}

			// Jenny, for checking duplicated destination address
			dip[0] = entry.dip[0] & dmask[0];
			dip[1] = entry.dip[1] & dmask[1];
			dip[2] = entry.dip[2] & dmask[2];
			dip[3] = entry.dip[3] & dmask[3];
		}
		
		// destination port
		if (entry.protoType == PROTO_TCP || entry.protoType == PROTO_UDP) {
			strDport = websGetVar(wp, T("dport"), T(""));
		
			if (strDport[0]) {
				if ( !string_to_dec(strDport, &intVal) || intVal<1 || intVal>65535) {
					strcpy(tmpBuf, T(Tinvalid_destination_port));
					goto setErr_qos;
				}
				entry.dPort = (unsigned short)intVal;
			}
		}
		
		// physical port
		strVal = websGetVar(wp, T("phyport"), T(""));
		entry.phyPort = 0xff;
		itfid = atoi(strVal);
		if (itfid != 0) {
			itfdomain = IF_DOMAIN(itfid);
			itfid = itfid&0x0ff;
			if (itfdomain == DOMAIN_ELAN)
				entry.phyPort = itfid;
#ifdef WLAN_SUPPORT
			else if (itfdomain == DOMAIN_WLAN)
#if defined(ZTE_GENERAL_ROUTER_SC) && defined(WLAN_MBSSID)
			{
				entry.phyPort=5+itfid;
			}
#else
				entry.phyPort = 5;	// wlan0
#endif				
#endif
#ifdef CONFIG_USB_ETH
			else if (itfdomain == DOMAIN_ULAN)
				entry.phyPort = IFUSBETH_PHYNUM;	// usb0
#endif //CONFIG_USB_ETH
		}
		
		// outbound interface
		strVal = websGetVar(wp, T("out_if"), T(""));
		if (strVal[0]) {
			entry.outif = (unsigned char)atoi(strVal);
		}
		
		/*
		 *	Assign/Mark priority
		 */
		// outbound priority
		strVal = websGetVar(wp, T("prio"), T(""));
		entry.prior = (unsigned char)(strVal[0]-'0');
		
#ifdef QOS_DSCP
		// DSCP enable
		strVal = websGetVar(wp, T("dscpenable"), T(""));
		entry.dscp = (unsigned char)(strVal[0]-'0');
		if(entry.dscp == 0) {
#endif

		// IP precedence
		strVal = websGetVar(wp, T("ipprio"), T(""));
		entry.m_ipprio = (unsigned char)(strVal[0]-'0');
		
		// IP Type of Service
		strVal = websGetVar(wp, T("tos"), T(""));
		entry.m_iptos = atoi(strVal);
		
#ifdef QOS_DSCP
		}
		else if (entry.dscp == 1) {
			int dscp;
			// DSCP
			strVal = websGetVar(wp, T("dscp"), T(""));
			dscp = atoi(strVal);
#ifdef NEW_IP_QOS_SUPPORT
			entry.m_ipprio = 0;
			entry.m_iptos = 255;
			entry.m_dscp = dscp;
#else
			entry.m_ipprio = dscp >> 3;
			entry.m_iptos = (dscp & 0x07) >> 1;
#endif
		}
#endif

		// 802.1p
		strVal = websGetVar(wp, T("m1p"), T(""));
		entry.m_1p = (unsigned char)(strVal[0]-'0');
		
		// Check if this qos entry exists
		// ...
		#ifdef QOS_SPEED_LIMIT_SUPPORT
		//check whether limit speed
		
		strVal = websGetVar(wp,T("qosspeedenable"),T(""));
		if(!strcmp(strVal,"on"))
		{
		entry.limitSpeedEnabled=1;
		printf("entry.limitSpeedEnabled=%d\n",entry.limitSpeedEnabled);
		if(entry.limitSpeedEnabled)
			{
			//get limit speed rank
			strVal = websGetVar(wp,T("speedLimitRank"),T(""));
			if ( !string_to_dec(strVal, &intVal) || intVal<1 || intVal>1024) {
					strcpy(tmpBuf, T(Tinvalid_speed));
					goto setErr_qos;
				}
			entry.limitSpeedRank= (unsigned char)intVal;
			MIB_CE_IP_QOS_SPEEDRANK_T qosSpeedMib;
			qosSpeedMib.speed=(unsigned char)intVal;
			qosSpeedMib.prior=entry.prior;
			int mibspeedindex=mib_qos_speed_limit_existed(qosSpeedMib.speed,qosSpeedMib.prior);
			if(mibspeedindex==-1)//not existed
			{
				qosSpeedMib.count=1;
				qosSpeedMib.index=mib_chain_total(MIB_QOS_SPEED_LIMIT);
				intVal = mib_chain_add(MIB_QOS_SPEED_LIMIT, (unsigned char*)&qosSpeedMib);
				if (intVal == 0) {
					strcpy(tmpBuf, T(Tadd_chain_error));
					printf("chain_add MIB_QOS_SPEED_LIMIT error!\n");
					goto setErr_qos;
				}
				else if (intVal == -1) {
					strcpy(tmpBuf, T(strTableFull));
					printf("chain_add MIB_QOS_SPEED_LIMIT table full!\n");
					goto setErr_qos;
				}
			}
			else//existed
				{
					mib_chain_get(MIB_QOS_SPEED_LIMIT,mibspeedindex,&qosSpeedMib);
					qosSpeedMib.count++;
					mib_chain_update(MIB_QOS_SPEED_LIMIT,&qosSpeedMib,mibspeedindex);
				}
			}
		}		
		else entry.limitSpeedEnabled=0;
		#endif

		if (!checkRule_ipqos(entry, sip, dip)) {	// Jenny
			strcpy(tmpBuf, T(Tinvalid_rule));
			goto setErr_qos;
		}

		intVal = mib_chain_add(MIB_IP_QOS_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			strcpy(tmpBuf, T(Tadd_chain_error));
			goto setErr_qos;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, T(strTableFull));
			goto setErr_qos;
		}
	
	}




#ifdef ZTE_GENERAL_ROUTER_SC
		/* Delete entry */
	if (strDelQos[0])
	{
		unsigned int i;
		unsigned int idx;
		strVal=websGetVar(wp, T("select"), T("")); 
		if(strVal[0])
		{
			for (i=0; i<totalEntry; i++) {

				idx = totalEntry-i-1;			
				snprintf(tmpBuf, 20, "s%d", idx);
				if ( !gstrcmp(strVal, T(tmpBuf)) ) 
				{
#ifdef QOS_SPEED_LIMIT_SUPPORT
				MIB_CE_IP_QOS_T qosEntry;
				mib_chain_get(MIB_IP_QOS_TBL,idx,&qosEntry);				
				mib_qos_speed_limit_check(&qosEntry);
#endif
					if(mib_chain_delete(MIB_IP_QOS_TBL, idx) != 1) {
						strcpy(tmpBuf, T(Tdelete_chain_error));
						goto setErr_qos;
					}
					
				}
			}
		}	
		goto setOk_qos;
	}
#else
	/* Delete entry */
	if (strDelQos[0])
	{
		unsigned int i;
		unsigned int idx;
		unsigned int deleted = 0;

		for (i=0; i<totalEntry; i++) {

			idx = totalEntry-i-1;			
			snprintf(tmpBuf, 20, "s%d", idx);
			strVal = websGetVar(wp, tmpBuf, T(""));
			
			if (strVal[0] == '1') {
				deleted ++;
#ifdef QOS_SPEED_LIMIT_SUPPORT
				MIB_CE_IP_QOS_T qosEntry;
				mib_chain_get(MIB_IP_QOS_TBL,idx,&qosEntry);				
				mib_qos_speed_limit_check(&qosEntry);
#endif				
				if(mib_chain_delete(MIB_IP_QOS_TBL, idx) != 1) {
					strcpy(tmpBuf, T(Tdelete_chain_error));
					goto setErr_qos;
				}
			}
		}
		if (deleted <= 0) {
			strcpy(tmpBuf, T("There is no item selected to delete!"));
			goto setErr_qos;
		}
	
		goto setOk_qos;
	}
#endif //ZTE_GENERAL_ROUTER_SC
	/* Delete all entry */
	if ( strDelAllQos[0]) {
		mib_chain_clear(MIB_IP_QOS_TBL); /* clear chain record */
		#ifdef QOS_SPEED_LIMIT_SUPPORT
		mib_chain_clear(MIB_QOS_SPEED_LIMIT); /* clear chain record */
		#endif
		goto setOk_qos;
	}

setOk_qos:
//	mib_update(CURRENT_SETTING);

#if defined(APPLY_CHANGE)
	if (mode&0x02){
#ifdef IP_QOS
		//stopIPQ();
		setupIPQ();
#elif defined(NEW_IP_QOS_SUPPORT)
		take_qos_effect();
#endif
	}
#endif
			
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

setErr_qos:
	ERR_MSG(tmpBuf);
}
#endif	// of IP_QOS

// List all the available LAN side interface at web page.
// return: number of interface listed.
int iflanList(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;
	int ifnum=0;
	unsigned int i;
	char_t *name;
	struct itfInfo itfs[16];
	int type;
	
	if (ejArgs(argc, argv, T("%s"), &name) < 1) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}
	
	type = 0;
	if ( !strcmp(name, T("all")) )
#ifdef CONFIG_USB_ETH
		type = (DOMAIN_ELAN|DOMAIN_WLAN|DOMAIN_ULAN);
#else
		type = (DOMAIN_ELAN|DOMAIN_WLAN);
#endif //CONFIG_USB_ETH
	else if ( !strcmp(name, T("eth")) )
		type = DOMAIN_ELAN;	// eth interface
	else if ( !strcmp(name, T("wlan")) )
		type = DOMAIN_WLAN;	// wireless interface
#ifdef CONFIG_USB_ETH
	else if ( !strcmp(name, T("usb")) )
		type = DOMAIN_ULAN;	// usb eth interface
#endif //CONFIG_USB_ETH
	else
		type = DOMAIN_ELAN;	// default to eth
	
	ifnum = get_domain_ifinfo(itfs, 16, type);
	if (ifnum==0)
		return 0;
#ifdef ZTE_GENERAL_ROUTER_SC
	for (i=0; i<ifnum; i++) {
			nBytesSent += websWrite(wp, T("<option value=%u>%s</option>\n"),
			IF_ID(itfs[i].ifdomain, itfs[i].ifid), itfs[i].name);
	}
#else
	for (i=0; i<ifnum; i++) {
			nBytesSent += websWrite(wp, T("<option value=%u>%s</option>\n"),
			IF_ID(itfs[i].ifdomain, itfs[i].ifid), itfs[i].name);
	}
#endif	
	return nBytesSent;
}

#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
/////////////////////////////////////////////////////////////////////////////
int default_qos(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;
//alex
#ifdef CONFIG_RE8305
#ifdef NEW_IP_QOS_SUPPORT
        nBytesSent += websWrite(wp, T("<td><font size=2>Default QoS:&nbsp;</td>\n"));
	nBytesSent += websWrite(wp, T("<td><select name=qosdmn style=\"width:100px \">\n"));
#else
        nBytesSent += websWrite(wp, T("<td><font size=2><b>Default QoS:&nbsp;</b></td>\n"));
	nBytesSent += websWrite(wp, T("<td><select name=qosdmn>\n"));
#endif
	nBytesSent += websWrite(wp, T("<option value=%d>IP Pred</option>"
			"</select>\n</td>\n"), PRIO_IP);
	return nBytesSent;
#endif

#ifdef CONFIG_8021P_PRIO
#ifdef NEW_IP_QOS_SUPPORT
        nBytesSent += websWrite(wp, T("<td><font size=2>Default QoS:&nbsp;&nbsp;</td>\n"));
//	nBytesSent += websWrite(wp, T("<td><select name=qosdmn>\n"));
	nBytesSent += websWrite(wp, T("<td><select name=qosdmn style=\"width:100px \" onClick=\"return enable8021psetting()\">\n"));
#else
        nBytesSent += websWrite(wp, T("<td><font size=2><b>Default QoS:&nbsp;&nbsp;</b></td>\n"));
//	nBytesSent += websWrite(wp, T("<td><select name=qosdmn>\n"));
	nBytesSent += websWrite(wp, T("<td><select name=qosdmn onClick=\"return enable8021psetting()\">\n"));
#endif
	nBytesSent += websWrite(wp, T("<option value=%d>IP Pred</option>\n"),PRIO_IP);
	nBytesSent += websWrite(wp, T("<option value=%d>802.1p</option>\n"),PRIO_802_1p);
	nBytesSent +=  websWrite(wp,T("</select>\n</td>\n"));
        
	return nBytesSent;
#else
#ifdef NEW_IP_QOS_SUPPORT
	nBytesSent += websWrite(wp, T("<td><font size=2>Default QoS:&nbsp;</td>\n"));
	nBytesSent += websWrite(wp, T("<td><select name=qosdmn style=\"width:100px \">\n"));
#else
	nBytesSent += websWrite(wp, T("<td><font size=2><b>Default QoS:&nbsp;</b></td>\n"));
	nBytesSent += websWrite(wp, T("<td><select name=qosdmn>\n"));
#endif
	nBytesSent += websWrite(wp, T("<option value=%d>IP Pred</option>"
			"<option value=%d>802.1p</option>\n</select>\n</td>\n"), PRIO_IP, PRIO_802_1p);
	return nBytesSent;
#endif
}

/////////////////////////////////////////////////////////////////////////////
int qosList(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;

	unsigned int entryNum, i, k;
	MIB_CE_IP_QOS_T Entry;
	const char *type;
	char	*psip, *pdip, sip[20], dip[20];
	char sport[10], dport[10];
	const char *pPrio, *pIPrio, *pTos, *p1p;
#ifdef QOS_DSCP
	char pDscp[5];
#endif
#ifdef NEW_IP_QOS_SUPPORT
	char qosDscp[5], vlan1p[5];
#endif
	unsigned int mask, smask, dmask;
	char strPhy[]="LAN0", *pPhy;
	char wanif[40];
	//char_t  buffer[3];

	entryNum = mib_chain_total(MIB_IP_QOS_TBL);
#ifdef ZTE_GENERAL_ROUTER_SC

#ifdef IP_POLICY_ROUTING
	nBytesSent += websWrite(wp, T("<tr>"
	"<th bgcolor=\"#808080\" colspan=6><font size=\"2\">流量规则</font></th>\n"
	"<th bgcolor=\"#808080\" colspan=5><font size=\"2\">行为</font></th>\n"
	"<th bgcolor=\"#808080\" colspan=2><font size=\"2\"></font></th>\n"));
#else
	nBytesSent += websWrite(wp, T("<tr>"
	"<th bgcolor=\"#808080\" colspan=6><font size=\"2\">流量规则</font></th>\n"
	"<th bgcolor=\"#808080\" colspan=4><font size=\"2\">行为</font></th>\n"
	"<th bgcolor=\"#808080\" colspan=2><font size=\"2\"></font></th>\n"));
#endif
	nBytesSent += websWrite(wp, T("<tr>"
      	"<td align=center width=\"25%%\" bgcolor=\"#808080\"><font size=\"2\"><b>源 IP</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>源 Port</b></font></td>\n"
      	"<td align=center width=\"25%%\" bgcolor=\"#808080\"><font size=\"2\"><b>目的 IP</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>目的 Port</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>协议</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>物理端口</b></font></td>\n"));
      	nBytesSent += websWrite(wp, T(
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>优先级</b></font></td>\n"
      	#ifdef QOS_SPEED_LIMIT_SUPPORT
	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>限速</b></font></td>\n"
	#endif
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>IP Precd</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>IP ToS</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Wan 802.1p</b></font></td>\n"
#ifdef IP_POLICY_ROUTING
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>广域网接口</b></font></td>\n"
#endif
#ifdef _CWMP_MIB_
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>使能</b></font></td>\n"
#endif
      	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b>选择</b></font></td></tr>\n"));
#else //ZTE_GENERAL_ROUTER_SC
	nBytesSent += websWrite(wp, T("<tr>")
#ifdef _CWMP_MIB_
	T("<th bgcolor=\"#808080\" colspan=2><font size=\"2\"></font></th>\n")
#else
	T("<th bgcolor=\"#808080\" colspan=1><font size=\"2\"></font></th>\n")
#endif
#ifndef QOS_DSCP_MATCH
	T("<th bgcolor=\"#808080\" colspan=6><font size=\"2\">Traffic Classification Rules</font></th>\n"
#else
	T("<th bgcolor=\"#808080\" colspan=7><font size=\"2\">Traffic Classification Rules</font></th>\n"
#endif
#ifndef NEW_IP_QOS_SUPPORT
	"<th bgcolor=\"#808080\" colspan=7><font size=\"2\">Mark</font></th>\n"));
//	"<th bgcolor=\"#808080\" colspan=4><font size=\"2\">Mark</font></th>\n"
#else
#ifndef QOS_DSCP
	"<th bgcolor=\"#808080\" colspan=4><font size=\"2\">Mark</font></th>\n"));
#else
	"<th bgcolor=\"#808080\" colspan=5><font size=\"2\">Mark</font></th>\n"));
#endif
#endif
//	"<th bgcolor=\"#808080\" colspan=2><font size=\"2\"></font></th>\n"));
	nBytesSent += websWrite(wp, T("<tr>"
      	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td>\n")
#ifdef _CWMP_MIB_
      	T("<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Status</b></font></td>\n")
#endif
#ifdef NEW_IP_QOS_SUPPORT
      	T("<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Src IP</b></font></td>\n"
#else
      	T("<td align=center width=\"25%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Src IP</b></font></td>\n"
#endif
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Src Port</b></font></td>\n"
      	"<td align=center width=\"25%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Dst IP</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Dst Port</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Protocol</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Lan Port</b></font></td>\n"
#ifdef NEW_IP_QOS_SUPPORT
#ifdef QOS_DSCP_MATCH
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>DSCP</b></font></td>\n"
#endif
      	//"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>802.1p</b></font></td>\n"
#endif
		));
      	nBytesSent += websWrite(wp, T(
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Priority</b></font></td>\n")
      	#ifdef QOS_SPEED_LIMIT_SUPPORT
	T("<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Speed  Limit</b></font></td>\n")
	#endif
      	T("<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>IP Precd</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>IP ToS</b></font></td>\n")
#ifdef QOS_DSCP
      	T("<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>IP DSCP</b></font></td>\n")
#endif
      	T("<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Wan 802.1p</b></font></td>\n")
#ifdef IP_POLICY_ROUTING
      	T("<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Wan IF</b></font></td>\n")
#endif
//#ifdef _CWMP_MIB_
//      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Enable</b></font></td>\n"
//#endif
//      	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n"));
      	T("</tr>\n"));
#endif //ZTE_GENERAL_ROUTER_SC
	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_IP_QOS_TBL, i, (void *)&Entry))
		{
  			websError(wp, 400, T("Get chain record error!\n"));
			return -1;
		}

#ifdef QOS_DIFFSERV
		if (Entry.enDiffserv == 1) // Diffserv entry
			continue;
#endif

		// source ip
		psip = inet_ntoa(*((struct in_addr *)Entry.sip));
		if ( !strcmp(psip, "0.0.0.0"))
			psip = (char *)BLANK;
		else {
			if (Entry.smaskbit==0)
				snprintf(sip, 20, "%s", psip);
			else
				snprintf(sip, 20, "%s/%d", psip, Entry.smaskbit);
			psip = sip;
		}
		//snprintf(sip, 20, "%s/%d", inet_ntoa(*((struct in_addr *)Entry.sip)), Entry.smaskbit);
		// source port
		if (Entry.sPort == 0)
			strcpy(sport, BLANK);
		else
			snprintf(sport, 10, "%d", Entry.sPort);
		
		// destination ip
		pdip = inet_ntoa(*((struct in_addr *)Entry.dip));
		if ( !strcmp(pdip, "0.0.0.0"))
			pdip = (char *)BLANK;
		else {
			if (Entry.dmaskbit==0)
				snprintf(dip, 20, "%s", pdip);
			else
				snprintf(dip, 20, "%s/%d", pdip, Entry.dmaskbit);
			pdip = dip;
		}
		//snprintf(dip, 20, "%s/%d", inet_ntoa(*((struct in_addr *)Entry.dip)), Entry.dmaskbit);
		// destination port
		if (Entry.dPort == 0)
			strcpy(dport, BLANK);
		else
			snprintf(dport, 10, "%d", Entry.dPort);
		
		
		if ( Entry.protoType == PROTO_TCP )
			type = ARG_TCP;
		else if ( Entry.protoType == PROTO_UDP )
			type = ARG_UDP;
		else if ( Entry.protoType == PROTO_ICMP )
			type = ARG_ICMP;
		else
			type = BLANK;
		
#ifdef NEW_IP_QOS_SUPPORT
#ifdef QOS_DSCP_MATCH
		//DSCP
		if (Entry.qosDscp == 0)
			strcpy(qosDscp, BLANK);
		else
			snprintf(qosDscp, 5, "%d", Entry.qosDscp>>2);
#endif

		//802.1p
		if (Entry.vlan1p == 0)
			strcpy(vlan1p, BLANK);
		else
			snprintf(vlan1p, 5, "%d", Entry.vlan1p-1);
#endif

		// Assign Priority
		if (Entry.prior <= (IPQOS_NUM_PRIOQ-1))
			pPrio = prioLevel[Entry.prior];
		else
			// should not be here !!
			pPrio = prioLevel[2];
		
#ifdef QOS_DSCP
		if(Entry.dscp==0) {
			strcpy(pDscp, BLANK);
#endif
		if (Entry.m_ipprio == 0)
			pIPrio = BLANK;
		else
			pIPrio = n0to7[Entry.m_ipprio];
		
		if (Entry.m_iptos == 0xff)
			pTos = BLANK;
		else {
			int mask, i;
			
			mask = i = 1;
			while (i<=5) {
				if (Entry.m_iptos & mask)
					break;
				else {
					i++;
					mask<<=1;
				}
			}
			if (i>=6)
				i = 1;
			pTos = ipTos[i];
		}
#ifdef QOS_DSCP
		}
		else if (Entry.dscp == 1) {
			pIPrio = BLANK;
			pTos = BLANK;
#ifndef NEW_IP_QOS_SUPPORT
			if (Entry.m_ipprio == 0 && Entry.m_iptos == 0)
				strcpy(pDscp, "BE");
			else if (Entry.m_ipprio == 5 && Entry.m_iptos == 3)
				strcpy(pDscp, "EF");
			else 
				sprintf(pDscp, "AF%d%d", Entry.m_ipprio, Entry.m_iptos);
#else
			if (Entry.m_dscp == 0)
				strcpy(pDscp, BLANK);
			else
				snprintf(pDscp, 5, "%d", Entry.m_dscp>>2);
#endif
		}
#endif
		
		if (Entry.m_1p == 0)
			p1p = BLANK;
		else
			p1p = n0to7[Entry.m_1p];
		
		pPhy = strPhy;
		if (Entry.phyPort == 0xff)
			pPhy = (char *)BLANK;
#if (defined(CONFIG_EXT_SWITCH)  && defined(IP_QOS_VPORT))
//#ifdef CONFIG_EXT_SWITCH
		else if (Entry.phyPort < SW_PORT_NUM)
			strPhy[3] = '0' + virt2user[Entry.phyPort];
#else
		else if (Entry.phyPort == 0)
			strPhy[3] = '0';
#endif
#ifdef CONFIG_USB_ETH
		else if (Entry.phyPort == IFUSBETH_PHYNUM)
			pPhy = (char *)USBETHIF;
#endif //CONFIG_USB_ETH
#ifdef WLAN_SUPPORT
		else
#ifdef ZTE_GENERAL_ROUTER_SC
		{
			if(Entry.phyPort==5) //wlan0
				pPhy = (char *)WLANIF;
			else 	// 6,7,8,9 vap0,vap1 vap2 vap3
			{
				strncpy(strPhy, "vap0", 4);
				strPhy[3]='0'+ (Entry.phyPort-6);
			}
		}
#else
			pPhy = (char *)WLANIF;
#endif //ZTE_GENERAL_ROUTER_SC
		
#endif
#ifdef QOS_SPEED_LIMIT_SUPPORT
	  char qosSpeedLimit[8]={0};
	   if(Entry.limitSpeedEnabled) sprintf(qosSpeedLimit,"%d",Entry.limitSpeedRank);
	   else sprintf(qosSpeedLimit,"none");
#endif
		
		nBytesSent += websWrite(wp, T("<tr>")
#ifndef ZTE_GENERAL_ROUTER_SC
      			T("<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"s%d\" value=1></td>\n")
#ifdef _CWMP_MIB_
			T("<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n")
#endif
#endif
			T("<td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
     			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
#ifdef NEW_IP_QOS_SUPPORT
#ifdef QOS_DSCP_MATCH
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
#endif
     			//"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
#endif
     			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n")
     			#ifdef QOS_SPEED_LIMIT_SUPPORT
			T("<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n")
			#endif
     			T("<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
     			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n")
			#ifdef QOS_DSCP
     			T("<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n")
			#endif
     			T("<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"),
//			psip, sport, pdip, dport, type, pPhy, pPrio, 
#ifndef ZTE_GENERAL_ROUTER_SC
			i,
#ifdef _CWMP_MIB_
			(Entry.enable==0)?"Disable":"Enable",
#endif
			psip, sport, pdip, dport, type, pPhy, 
#ifdef NEW_IP_QOS_SUPPORT
#ifdef QOS_DSCP_MATCH
			qosDscp,
#endif
			/*vlan1p, */
#endif
			pPrio, 
#endif
			#ifdef QOS_SPEED_LIMIT_SUPPORT
			qosSpeedLimit,
			#endif
//			pIPrio, pTos, p1p);
			pIPrio, pTos,
			#ifdef QOS_DSCP
			pDscp,
			#endif
			p1p);
		
#ifdef IP_POLICY_ROUTING
		if (Entry.outif == 0xff) {
			wanif[0] = ' ';
			wanif[1] = 0;
		}
		else {

		#if defined(ZTE_GENERAL_ROUTER_SC) && defined(CTC_WAN_NAME)
			MIB_CE_ATM_VC_T vcEntry;
			memset(&vcEntry, 0, sizeof(vcEntry));
			if(0==getWanEntrybyindex(&vcEntry, Entry.outif))
			{
				//got it
				memset(wanif, 0, sizeof(wanif));
				getWanName(&vcEntry, wanif);
			}else
			{
				if (PPP_INDEX(Entry.outif) != 0x0f)
				{	// PPP interface
					snprintf(wanif, 8, "ppp%u", PPP_INDEX(Entry.outif));
     				}
     				else
				{	// vc interface
					snprintf(wanif, 8, "vc%u", VC_INDEX(Entry.outif));
     				}
			}
		#else
			if (PPP_INDEX(Entry.outif) != 0x0f)
			{	// PPP interface
				snprintf(wanif, 8, "ppp%u", PPP_INDEX(Entry.outif));
     			}
     			else
			{	// vc interface
				snprintf(wanif, 8, "vc%u", VC_INDEX(Entry.outif));
     			}
		#endif

		}
     		
		nBytesSent += websWrite(wp, T(
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"),
			 wanif);
#endif
#ifdef ZTE_GENERAL_ROUTER_SC
#ifdef _CWMP_MIB_
		nBytesSent += websWrite(wp, T(
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"),
			 (Entry.enable==0)?"Disable":"Enable" );
#endif
#endif
#ifndef ZTE_GENERAL_ROUTER_SC
//		nBytesSent += websWrite(wp, T(
//      			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"s%d\" value=1></td></tr>\n"),
//      			i);
      		nBytesSent += websWrite(wp, T("</tr>\n"));
#else
      		nBytesSent += websWrite(wp, T(
      			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"select\" value=\"s%d\"></td></tr>\n"),
      			i);
#endif
      			
	}
#ifdef ZTE_GENERAL_ROUTER_SC
	nBytesSent += websWrite(wp, T("<input type=\"hidden\" value=%d name=\"listnumber\">"), i);
#endif
	//snprintf(buffer, 3, "%u", entryNum);
	//ejSetResult(eid, buffer);
	return nBytesSent;
}
#endif	// of IP_QOS

int policy_route_outif(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	
#ifdef IP_POLICY_ROUTING
	#ifdef ZTE_GENERAL_ROUTER_SC
	nBytesSent += websWrite(wp, T(" <tr>\n"));
	nBytesSent += websWrite(wp, T(" \t<td align=left><font size=2>指定广域网接口:</td>\n"));
	nBytesSent += websWrite(wp, T("  \t<td align=left>\n\t\t<select name=out_if>\n"));
	nBytesSent += websWrite(wp, T(" \t\t<option value=255>无接口指定</option>\n"));
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			websError(wp, 400, T("Get chain record error!\n"));
			return -1;
		}
		
		if (Entry.enable == 0)
			continue;
		
		if (Entry.cmode == ADSL_BR1483)
			continue;
		#if 1
		if(1)
		{
			char wanif[40];
			memset(wanif, 0, sizeof(wanif));
			getWanName(&Entry, wanif);
			if (PPP_INDEX(Entry.ifIndex) != 0x0f)
			{	// PPP interface
				nBytesSent += websWrite(wp, T(" \t\t<option value=%u>%s</option>\n"),
					Entry.ifIndex, wanif);
			}
			else
			{	// vc interface
				nBytesSent += websWrite(wp, T(" \t\t<option value=%u>%s</option>\n"),
					Entry.ifIndex, wanif);
			}
			
			
		}
		#else
		if (PPP_INDEX(Entry.ifIndex) != 0x0f)
		{	// PPP interface
			nBytesSent += websWrite(wp, T(" \t\t<option value=%u>ppp%u</option>\n"),
				Entry.ifIndex, PPP_INDEX(Entry.ifIndex));
		}
		else
		{	// vc interface
			nBytesSent += websWrite(wp, T(" \t\t<option value=%u>vc%u</option>\n"),
				Entry.ifIndex, VC_INDEX(Entry.ifIndex));
		}
		#endif
	}
	
	nBytesSent += websWrite(wp, T("\t\t</select>\n\t</td>\n</tr>\n"));	
	#else
	nBytesSent += websWrite(wp, T("<tr>\n"));
#ifdef NEW_IP_QOS_SUPPORT
	nBytesSent += websWrite(wp, T("\t<td align=left><font size=2>Outbound Iterface:</td>\n"));
#else
	nBytesSent += websWrite(wp, T("\t<td align=left><font size=2><b>Outbound Iterface:</b></td>\n"));
#endif
	nBytesSent += websWrite(wp, T("\t<td align=left>\n\t\t<select name=out_if>\n"));
	nBytesSent += websWrite(wp, T("\t\t<option value=255></option>\n"));
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			websError(wp, 400, T("Get chain record error!\n"));
			return -1;
		}
		
		if (Entry.enable == 0)
			continue;
		
		if (Entry.cmode == ADSL_BR1483)
			continue;
		
		if (PPP_INDEX(Entry.ifIndex) != 0x0f)
		{	// PPP interface
			nBytesSent += websWrite(wp, T("\t\t<option value=%u>ppp%u</option>\n"),
				Entry.ifIndex, PPP_INDEX(Entry.ifIndex));
		}
		else
		{	// vc interface
			nBytesSent += websWrite(wp, T("\t\t<option value=%u>vc%u</option>\n"),
				Entry.ifIndex, VC_INDEX(Entry.ifIndex));
		}
	}
	
	nBytesSent += websWrite(wp, T("\t\t</select>\n\t</td>\n</tr>\n"));
	#endif
#endif
	return nBytesSent;
}

#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
int priority_outif(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0, i;
	const char *pPrio;
#ifdef ZTE_GENERAL_ROUTER_SC
#ifdef NEW_IP_QOS_SUPPORT
	nBytesSent += websWrite(wp, T("  \t<select name=prio style=\"width:150px \">\n"));
#else
	nBytesSent += websWrite(wp, T("  \t<select name=prio>\n"));
#endif
	pPrio = prioLevel[IPQOS_NUM_PRIOQ-1];
	nBytesSent += websWrite(wp, T(" \t\t<option value=%d>%s(最低)</option>\n"),  IPQOS_NUM_PRIOQ-1, pPrio);
	for (i=IPQOS_NUM_PRIOQ-2; i>=1; i--) {
		pPrio = prioLevel[i];
		nBytesSent += websWrite(wp, T("  \t\t<option value=%d>%s</option>\n"), i, pPrio);
	}
	pPrio = prioLevel[0];
	nBytesSent += websWrite(wp, T("  \t\t<option value=0>%s(最高)</option>\n"), pPrio);
	nBytesSent += websWrite(wp, T("  '\t</select>\n"));
#else
#ifdef NEW_IP_QOS_SUPPORT
	nBytesSent += websWrite(wp, T(" \t<select name=prio style=\"width:150px \">\n"));
#else
	nBytesSent += websWrite(wp, T(" \t<select name=prio>\n"));
#endif
	pPrio = prioLevel[IPQOS_NUM_PRIOQ-1];
nBytesSent += websWrite(wp, T("\t\t<option value=%d>%s(lowest)</option>\n"),  IPQOS_NUM_PRIOQ-1, pPrio);
	for (i=IPQOS_NUM_PRIOQ-2; i>=1; i--) {
		pPrio = prioLevel[i];
		nBytesSent += websWrite(wp, T("\t\t<option value=%d>%s</option>\n"), i, pPrio);
	}
	pPrio = prioLevel[0];
	nBytesSent += websWrite(wp, T(" \t\t<option value=0>%s(highest)</option>\n"), pPrio);
	nBytesSent += websWrite(wp, T("  \t<select>\n"));
#endif
	return nBytesSent;
}

int confDscp(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;

      nBytesSent += websWrite(wp, T( "<tr>\n"));
#ifdef NEW_IP_QOS_SUPPORT
#ifdef QOS_DSCP
      nBytesSent += websWrite(wp, T( "\t<td colspan=2><font size=2><input type=\"radio\" name=dscpenable value=0 onClick=\"return dscpClick()\">Type of Servie (TOS)&nbsp;&nbsp;</td></tr>\n"));
#endif
      nBytesSent += websWrite(wp, T( "\t<tr><td><font size=2>Precedence:&nbsp;&nbsp;</td><td><select name=ipprio style=\"width:150px \">\n"));
#else
#ifdef QOS_DSCP
      nBytesSent += websWrite(wp, T( "\t<td colspan=2><font size=2><b><input type=\"radio\" name=dscpenable value=0 onClick=\"return dscpClick()\">Type of Servie (TOS)&nbsp;&nbsp;</b></td>\n"));
#endif
      nBytesSent += websWrite(wp, T( "\t<td><font size=2><b>Precedence:&nbsp;&nbsp;</b><select name=ipprio>\n"));
#endif
      nBytesSent += websWrite(wp, T( "\t\t<option value=0></option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=1>0</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=2>1</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=3>2</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=4>3</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=5>4</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=6>5</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=7>6</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=8>7</option>\n"));
#ifdef NEW_IP_QOS_SUPPORT
      nBytesSent += websWrite(wp, T( "\t</select></td></tr>\n"));
      nBytesSent += websWrite(wp, T( "\t<tr><td><font size=2>TOS:&nbsp;&nbsp;</td><td><select name=tos style=\"width:150px \">\n"));
#else
      nBytesSent += websWrite(wp, T( "\t</select></td>\n"));
      nBytesSent += websWrite(wp, T( "\t<td><font size=2><b>TOS:&nbsp;&nbsp;</b><select name=tos>\n"));
#endif
      nBytesSent += websWrite(wp, T( "\t\t<option value=255></option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=0>Normal Service</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=2>Minimize Cost</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=4>Maximize Reliability</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=8>Maximize Throughput</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=16>Minimize Delay</option>\n"));
      nBytesSent += websWrite(wp, T( "\t</select></td>\n"));
      nBytesSent += websWrite(wp, T( "</tr>\n"));
#ifdef QOS_DSCP
      nBytesSent += websWrite(wp, T( "<tr>\n"));
#ifdef NEW_IP_QOS_SUPPORT
      nBytesSent += websWrite(wp, T( "\t<td colspan=2><font size=2><input type=\"radio\" name=dscpenable value=1 onClick=\"return dscpClick()\" checked>Diffserv Codepoint (DSCP)&nbsp;&nbsp;</td></tr>\n"));
      nBytesSent += websWrite(wp, T( "\t<tr><td><font size=2>DSCP tag:&nbsp;&nbsp;</td><td><select name=dscp style=\"width:150px \">\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=0> </option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=1>default(000000)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=57>AF13(001110)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=49>AF12(001100)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=41>AF11(001010)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=33>CS1(001000)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=89>AF23(010110)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=81>AF22(010100)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=73>AF21(010010)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=65>CS2(010000)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=121>AF33(011110)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=113>AF32(011100)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=105>AF31(011010)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=97>CS3(011000)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=153>AF43(100110)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=145>AF42(100100)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=137>AF41(100010)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=129>CS4(100000)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=185>EF(101110)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=161>CS5(101000)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=193>CS6(110000)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=225>CS7(111000)</option>\n"));
#else
      nBytesSent += websWrite(wp, T( "\t<td colspan=2><font size=2><b><input type=\"radio\" name=dscpenable value=1 onClick=\"return dscpClick()\" checked>Diffserv Codepoint (DSCP)&nbsp;&nbsp;</b></td>\n"));
      nBytesSent += websWrite(wp, T( "\t<td><font size=2><b>Value:&nbsp;&nbsp;</b><select name=dscp>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=0>BE PHB (000000)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=10>AF11 PHB (001010)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=12>AF12 PHB (001100)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=14>AF13 PHB (001110)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=18>AF21 PHB (010010)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=20>AF22 PHB (010100)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=22>AF23 PHB (010110)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=26>AF31 PHB (011010)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=28>AF32 PHB (011100)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=30>AF33 PHB (011110)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=34>AF41 PHB (100010)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=36>AF42 PHB (100100)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=38>AF43 PHB (100110)</option>\n"));
      nBytesSent += websWrite(wp, T( "\t\t<option value=46>EF PHB (101110)</option>\n"));
#endif
      nBytesSent += websWrite(wp, T( "\t</select></td>\n"));
      nBytesSent += websWrite(wp, T( "</tr>\n"));
#else
      nBytesSent += websWrite(wp, T( "<tr ID='dscphidden' style=\"display:none\">\n"));
      nBytesSent += websWrite(wp, T( "\t<td><input type=radio name=dscpenable checked></td>\n"));
      nBytesSent += websWrite(wp, T( "\t<td><input type=radio name=dscpenable ></td>\n"));
      nBytesSent += websWrite(wp, T( "\t<td><input type=hidden name=dscp></td>\n"));
      nBytesSent += websWrite(wp, T( "</tr>\n"));
      #if 0
      nBytesSent += websWrite(wp, T( "<tr>\n"));
      nBytesSent += websWrite(wp, T( "\t<td><input type=\"hidden\" value=\"0\" name=dscpenable></td>\n"));
      nBytesSent += websWrite(wp, T( "\t<td><input type=\"hidden\" value=\"0\" name=dscp></td>\n"));
      nBytesSent += websWrite(wp, T( "</tr>\n"));
      #endif
#endif
	return nBytesSent;
}

#ifdef QOS_DIFFSERV
void formDiffServ(webs_t wp, char_t *path, char_t *query)
{
	char_t *submitUrl, *strAddQos, *strVal, *strList, *strDelQos, *strPHBClass, *strHTBRate;
	char_t *strSAddr, *strDAddr, *strSport, *strDport, *strInf, *strBandwidth, *strLatency, *strDSCP, *strRate, *strPolicing, *strProt;
	char tmpBuf[100];
	int intVal, ret;
	unsigned int totalEntry, i, clearIdx;
	MIB_CE_IP_QOS_T entry;
	unsigned long mask, mbit;
	unsigned char vChar, vChar_mode = 0, vChar_class = 0, pClass;
#ifndef NO_ACTION
	int pid;
#endif

	mib_get(MIB_MPMODE, (void *)&vChar_mode);
	if (vChar_mode&0x02)
		stopIPQ();

	mib_get(MIB_DIFFSERV_PHBCLASS, (void *)&pClass);
	totalEntry = mib_chain_total(MIB_IP_QOS_TBL); /* get chain record size */
	for (i = 0; i < totalEntry; i ++) {
		if (!mib_chain_get(MIB_IP_QOS_TBL, i, (void *)&entry))
			continue;
		if (entry.enDiffserv == 0) // non-Diffserv entry
			continue;
		if (pClass == entry.m_ipprio) {
			clearIdx = i;
			break;
		}
	}

	strDelQos = websGetVar(wp, T("delSel"), T(""));
	/* Delete entry */
	if (strDelQos[0]) {
		unsigned int idx;
		unsigned int deleted = 0;

		for (i=0; i<totalEntry; i++) {
			idx = totalEntry-i-1;			
			snprintf(tmpBuf, 20, "s%d", idx);
			strVal = websGetVar(wp, tmpBuf, T(""));
			if (strVal[0] == '1') {
				deleted ++;
				cleanupDiffservRule(idx);
				if(mib_chain_delete(MIB_IP_QOS_TBL, idx) != 1) {
					strcpy(tmpBuf, T(Tdelete_chain_error));
					goto setErr_qos;
				}
			}
		}
		if (deleted <= 0) {
			strcpy(tmpBuf, T("There is no item selected to delete!"));
			goto setErr_qos;
		}
		goto setOk_qos;
	}

	strAddQos = websGetVar(wp, T("addqos"), T(""));
	if (strAddQos[0]) {
		// enable/disable DiffServ
		strVal = websGetVar(wp, T("qoscap"), T(""));
		vChar = strVal[0] - '0';
	}

	strPHBClass = websGetVar(wp, T("phbclass"), T(""));
	if (strPHBClass[0]) {
		// PHB Class
		vChar_class = strPHBClass[0] - '0';
	}

	if (vChar == 1) {
		unsigned char smask[4], dmask[4], sip[4], dip[4];

		//if (deleteDiffservEntry()) {
		//	strcpy(tmpBuf, T(Tdelete_chain_error));
		//	goto setErr_qos;
		//}

		// WAN interface
		strInf = websGetVar(wp, T("interface"), T(""));

		// Total Bandwidth Limit
		strBandwidth = websGetVar(wp, T("totalbandwidth"), T(""));

		// Class Rate Limit
		strHTBRate = websGetVar(wp, T("htbrate"), T(""));

		// Max Desired Latency
		strLatency = websGetVar(wp, T("latency"), T(""));

		strList = websGetVar(wp, T("phblst"), T(""));
		if (strList[0]) {
			char *token, *phblist;

			phblist = strdup(strList);
			while ((token = strtok(phblist, ",")) != NULL) {
				int phb;
				memset(&entry, 0, sizeof(entry));
				if (strInf[0])
					entry.ifIndex = (unsigned char)atoi(strInf);

				/* Add new qos entry */
				memset(sip, 0x0, 4);
				memset(dip, 0x0, 4);
				if (totalEntry >= MAX_QOS_RULE) {
					strcpy(tmpBuf, T(Texceed_max_rules));
					free(phblist);
					goto setErr_qos;
				}
				// Diffserv entry flag
				entry.enDiffserv = 1;
				// phb
				phb = atoi(token);
				entry.m_ipprio = phb >> 3;
				entry.m_iptos = (phb & 0x07) >> 1;
				if (strBandwidth[0])
					entry.totalBandwidth = strtol(strBandwidth, (char**)NULL, 0);
				if (strHTBRate[0])
					entry.htbRate = strtol(strHTBRate, (char**)NULL, 0);
				if (strLatency[0])
					entry.latency = strtol(strLatency, (char**)NULL, 0);

				// DSCP
#ifdef QOS_DSCP_MATCH
				snprintf(tmpBuf, 100, "m_dscp%s", token);
				strDSCP = websGetVar(wp, T(tmpBuf), T(""));
				if (strDSCP[0])
					entry.qosDscp = strtol(strDSCP, (char**)NULL, 0);
				else
#endif
					entry.qosDscp = 0;
				snprintf(tmpBuf, 100, "rate%s", token);
				strRate = websGetVar(wp, T(tmpBuf), T(""));
				// Police Rate
				if (strRate[0])
					entry.limitSpeed = strtol(strRate, (char**)NULL, 0);
				else
					entry.limitSpeed = 0;
				// Policing: drop/continue
				snprintf(tmpBuf, 100, "bhv%s", token);
				strPolicing = websGetVar(wp, T(tmpBuf), T(""));
				if (strPolicing[0])
					entry.policing = strPolicing[0] - '0';
				// protocol type
				snprintf(tmpBuf, 100, "prot%s", token);
				strProt = websGetVar(wp, T(tmpBuf), T(""));
				if (strProt[0])
					entry.protoType = strProt[0] - '0';

				snprintf(tmpBuf, 100, "sip%s", token);
				strSAddr = websGetVar(wp, T(tmpBuf), T(""));
				snprintf(tmpBuf, 100, "dip%s", token);
				strDAddr = websGetVar(wp, T(tmpBuf), T(""));
				// Source address
				if (strSAddr[0]) {
					inet_aton(strSAddr, (struct in_addr *)&entry.sip);
					if ((entry.sip[0]=='\0') &&(entry.sip[1]=='\0') && (entry.sip[2]=='\0') && (entry.sip[3]=='\0')) {
						strcpy(tmpBuf, T(Tinvalid_source_ip));
						free(phblist);
						goto setErr_qos;
					}
					snprintf(tmpBuf, 100, "smask%s", token);
					strSAddr = websGetVar(wp, T(tmpBuf), T(""));
					if (strSAddr[0]) {
						if (!isValidNetmask(strSAddr, 1)) {
							strcpy(tmpBuf, T(Tinvalid_source_netmask));
							free(phblist);
							goto setErr_qos;
						}
						inet_aton(strSAddr, (struct in_addr *)smask);
						inet_aton(strSAddr, (struct in_addr *)&mask);
						if (mask==0) {
							strcpy(tmpBuf, T(Tinvalid_source_netmask));
							free(phblist);
							goto setErr_qos;
						}
						mask = htonl(mask);
						mbit=0; intVal=0;
						for (i=0; i<32; i++) {
							if (mask&0x80000000) {
								if (intVal) {
									strcpy(tmpBuf, T(Tinvalid_source_netmask));
									free(phblist);
									goto setErr_qos;
								}
								mbit++;
							}
							else
								intVal=1;
							mask <<= 1;
						}
						entry.smaskbit = mbit;
					}
					else {
						entry.smaskbit = 32;
						inet_aton(ARG_255x4, (struct in_addr *)smask);
					}

					// Jenny, for checking duplicated source address
					sip[0] = entry.sip[0] & smask[0];
					sip[1] = entry.sip[1] & smask[1];
					sip[2] = entry.sip[2] & smask[2];
					sip[3] = entry.sip[3] & smask[3];
				}

				// source port
				if (entry.protoType == PROTO_TCP || entry.protoType == PROTO_UDP) {
					snprintf(tmpBuf, 100, "sport%s", token);
					strSport = websGetVar(wp, T(tmpBuf), T(""));
					if (strSport[0]) {
						if ( !string_to_dec(strSport, &intVal) || intVal<1 || intVal>65535) {
							strcpy(tmpBuf, T(Tinvalid_source_port));
							free(phblist);
							goto setErr_qos;
						}
						entry.sPort = (unsigned short)intVal;
					}
				}

				// Destination address
				if (strDAddr[0]) {
					inet_aton(strDAddr, (struct in_addr *)&entry.dip);
					if ((entry.dip[0]=='\0') && (entry.dip[1]=='\0') && (entry.dip[2]=='\0') && (entry.dip[3]=='\0')) {
						strcpy(tmpBuf, T(Tinvalid_destination_ip));
						free(phblist);
						goto setErr_qos;
					}
					snprintf(tmpBuf, 100, "dmask%s", token);
					strDAddr = websGetVar(wp, T(tmpBuf), T(""));
					if (strDAddr[0]) {
						if (!isValidNetmask(strDAddr, 1)) {
							strcpy(tmpBuf, T(Tinvalid_destination_netmask));
							free(phblist);
							goto setErr_qos;
						}
						inet_aton(strDAddr, (struct in_addr *)dmask);
						inet_aton(strDAddr, (struct in_addr *)&mask);
						if (mask==0) {
							strcpy(tmpBuf, T(Tinvalid_destination_netmask));
							free(phblist);
							goto setErr_qos;
						}
						mask = htonl(mask);
						mbit=0; intVal=0;
						for (i=0; i<32; i++) {
							if (mask&0x80000000) {
								if (intVal) {
									strcpy(tmpBuf, T(Tinvalid_destination_netmask));
									free(phblist);
									goto setErr_qos;
								}
								mbit++;
							}
							else
								intVal=1;
							mask <<= 1;
						}
						entry.dmaskbit = mbit;
					}
					else {
						entry.dmaskbit = 32;
						inet_aton(ARG_255x4, (struct in_addr *)dmask);
					}
					// Jenny, for checking duplicated destination address
					dip[0] = entry.dip[0] & dmask[0];
					dip[1] = entry.dip[1] & dmask[1];
					dip[2] = entry.dip[2] & dmask[2];
					dip[3] = entry.dip[3] & dmask[3];
				}

				// destination port
				if (entry.protoType == PROTO_TCP || entry.protoType == PROTO_UDP) {
					snprintf(tmpBuf, 100, "dport%s", token);
					strDport = websGetVar(wp, T(tmpBuf), T(""));
					if (strDport[0]) {
						if ( !string_to_dec(strDport, &intVal) || intVal<1 || intVal>65535) {
							strcpy(tmpBuf, T(Tinvalid_destination_port));
							free(phblist);
							goto setErr_qos;
						}
						entry.dPort = (unsigned short)intVal;
					}
				}

				// Check if this qos entry exists
				ret = checkRule_ipqos(entry, sip, dip);
				if (ret == 0) {
					strcpy(tmpBuf, T(Tinvalid_rule));
					free(phblist);
					goto setErr_qos;
				}
				else if (ret == 2) {
					if (vChar_class == 5)
						snprintf(tmpBuf, 100, "EF class existed! Please delete existed entry first!");
					else
						snprintf(tmpBuf, 100, "AF class %d existed! Please delete existed entry first!", vChar_class);
					free(phblist);
					goto setErr_qos;
				}
				mib_set(MIB_DIFFSERV_PHBCLASS, (void *)&vChar_class);
				intVal = mib_chain_add(MIB_IP_QOS_TBL, (unsigned char*)&entry);
				if (intVal == 0) {
					strcpy(tmpBuf, T(Tadd_chain_error));
					free(phblist);
					goto setErr_qos;
				}
				else if (intVal == -1) {
					strcpy(tmpBuf, T(strTableFull));
					free(phblist);
					goto setErr_qos;
				}
				phblist = 0;
			}
			free(phblist);
		}
		mib_set(MIB_QOS_DIFFSERV, (void *)&vChar);

#if defined(APPLY_CHANGE)
	cleanupDiffservRule(clearIdx);
	setupDiffServ();
#endif
	}
	else if (vChar == 0) {
		mib_set(MIB_QOS_DIFFSERV, (void *)&vChar);
		cleanupDiffservRule(clearIdx);
	}

setOk_qos:
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

setErr_qos:
	ERR_MSG(tmpBuf);
}

int diffservList(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;

	unsigned int entryNum, i;
	MIB_CE_IP_QOS_T Entry;
	const char *type;
	char phb[5], policing[10], rate[6];
	char	*psip, *pdip, sip[20], dip[20];
	char sport[10], dport[10];
	unsigned int mask, smask, dmask;
	char wanif[40];
#ifdef QOS_DSCP_MATCH
	char qosDscp[5];
#endif
	//unsigned char phbclass;

	//mib_get(MIB_DIFFSERV_PHBCLASS, (void *)&phbclass);
	nBytesSent += websWrite(wp, T("<tr>"
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td>\n"
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>PHB</b></font></td>\n"
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Class Rate</b></font></td>\n"
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Inf</b></font></td>\n"
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>BW</b></font></td>\n"));
	nBytesSent += websWrite(wp, T(
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Latency</b></font></td>\n"
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Src IP</b></font></td>\n"
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Src Port</b></font></td>\n"
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Dst IP</b></font></td>\n"
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Dst Port</b></font></td>\n"));
	nBytesSent += websWrite(wp, T(
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Protocol</b></font></td>\n"
#ifdef QOS_DSCP_MATCH
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>DSCP</b></font></td>\n"
#endif
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Rate</b></font></td>\n"
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Police</b></font></td>\n"
		"</tr>\n"
		));

	entryNum = mib_chain_total(MIB_IP_QOS_TBL);
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_IP_QOS_TBL, i, (void *)&Entry)) {
  			websError(wp, 400, T("Get chain record error!\n"));
			return -1;
		}

		if (Entry.enDiffserv == 0) // IP QoS entry
			continue;

		//if (Entry.m_ipprio != phbclass) // only show active PHB class
		//	continue;

		// PHB Group
		if (Entry.m_ipprio == 5 && Entry.m_iptos == 3)
			strcpy(phb, "EF");
		else 
			sprintf(phb, "AF%d%d", Entry.m_ipprio, Entry.m_iptos);

		if (PPP_INDEX(Entry.ifIndex) != 0x0f)	// PPP interface
			snprintf(wanif, 20, "ppp%u", PPP_INDEX(Entry.ifIndex));
		else		// vc interface
			snprintf(wanif, 20, "vc%u", VC_INDEX(Entry.ifIndex));

		// source ip
		psip = inet_ntoa(*((struct in_addr *)Entry.sip));
		if ( !strcmp(psip, "0.0.0.0"))
			psip = (char *)BLANK;
		else {
			if (Entry.smaskbit==0)
				snprintf(sip, 20, "%s", psip);
			else
				snprintf(sip, 20, "%s/%d", psip, Entry.smaskbit);
			psip = sip;
		}
		// source port
		if (Entry.sPort == 0)
			strcpy(sport, BLANK);
		else
			snprintf(sport, 10, "%d", Entry.sPort);

		// destination ip
		pdip = inet_ntoa(*((struct in_addr *)Entry.dip));
		if ( !strcmp(pdip, "0.0.0.0"))
			pdip = (char *)BLANK;
		else {
			if (Entry.dmaskbit==0)
				snprintf(dip, 20, "%s", pdip);
			else
				snprintf(dip, 20, "%s/%d", pdip, Entry.dmaskbit);
			pdip = dip;
		}
		// destination port
		if (Entry.dPort == 0)
			strcpy(dport, BLANK);
		else
			snprintf(dport, 10, "%d", Entry.dPort);

		if (Entry.protoType == PROTO_TCP)
			type = ARG_TCP;
		else if (Entry.protoType == PROTO_UDP)
			type = ARG_UDP;
		else if (Entry.protoType == PROTO_ICMP)
			type = ARG_ICMP;
		else
			type = BLANK;

#ifdef QOS_DSCP_MATCH
		//DSCP
		if (Entry.qosDscp == 0)
			strcpy(qosDscp, BLANK);
		else
			snprintf(qosDscp, 5, "%d", Entry.qosDscp>>2);
#endif
		
		if (Entry.limitSpeed == 0) {
			strcpy(rate, BLANK);
			strcpy(policing, BLANK);
		}
		else {
			snprintf(rate, 6, "%d", Entry.limitSpeed);
			if (Entry.policing == 1)
				strcpy(policing, "Drop");
			else if (Entry.policing == 2)
				strcpy(policing, "Continue");
		}

		nBytesSent += websWrite(wp, T("<tr>"
			"<td align=center bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"s%d\" value=1></td>\n"
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%d</td>\n"
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%d</td>\n"),
			i, phb, Entry.htbRate, wanif, Entry.totalBandwidth);
		nBytesSent += websWrite(wp, T(
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%d</td>\n"
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"),
			Entry.latency, psip, sport, pdip, dport);
		nBytesSent += websWrite(wp, T(
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
#ifdef QOS_DSCP_MATCH
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
#endif
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"</tr>\n"),
			type
#ifdef QOS_DSCP_MATCH
			, qosDscp
#endif
			, rate, policing);
	}
	return nBytesSent;
}
#endif	// of QOS_DIFFSERV
#endif	// of IP_QOS

#ifdef QOS_DSCP_MATCH
int match_dscp(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0;

	nBytesSent += websWrite(wp, T( "<td>DSCP:</td>\n"));
	nBytesSent += websWrite(wp, T( "\t<td><select name=\"s_dscp\" size=\"1\" style=\"width:150px \" onChange=\"onchange_sel1();\">\n"));
	nBytesSent += websWrite(wp, T( "\t\t<script>writeDscpList();</script>\n"));
	nBytesSent += websWrite(wp, T( "\t\t</select>\n"));
	nBytesSent += websWrite(wp, T( "\t</td>\n"));

	return nBytesSent;
}
#endif

//alex for 802.1p
#ifdef CONFIG_8021P_PRIO
int  setting_1ppriority(int eid, webs_t wp, int argc, char_t **argv)
{
   int nBytesSent=0, i,j,m=0;
   const char *pPrio;
   const char *nset1ptbl;
   unsigned char value[8];
   
#ifdef NEW_IP_QOS_SUPPORT
	nBytesSent += websWrite(wp, T("<tr>"
		"<th bgcolor=\"#808080\" colspan=1><font size=\"2\">802.1p  priority</font></th>\n"
		"<th bgcolor=\"#808080\" colspan=1><font size=\"2\">transmit priority</font></th>\n</tr>\n"
		));
#else
   nBytesSent += websWrite(wp, T("<tr>"
	"<th bgcolor=\"#808080\" colspan=1><font size=\"2\">802.1p 优先级</font></th>\n"
	"<th bgcolor=\"#808080\" colspan=1><font size=\"2\">发送优先级</font></th>\n</tr>\n"
	));
#endif
     	   
    if(mib_get(MIB_8021P_PROI, (void *)value)== 0)
    {
   	  printf("Get 8021P_PROI  error!\n");
	  return 0;
    }        

   for(i=0;i<IPQOS_NUM_PKT_PRIO;i++)
   {
       
        m= value[i];
	nset1ptbl =set1ptable[i];
       
      nBytesSent += websWrite(wp, T( "<tr>"));
      nBytesSent += websWrite(wp, T( "<td align=center width=\"50%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%d</td>\n"),i);
      nBytesSent += websWrite(wp, T("  <td align=center width=\"50%%\" bgcolor=\"#C0C0C0\"><select name=\"%s\">\n"),nset1ptbl);
      	  
      pPrio = prioLevel[IPQOS_NUM_PRIOQ-1];
#ifdef NEW_IP_QOS_SUPPORT
		if(m==IPQOS_NUM_PRIOQ-1) 
			nBytesSent += websWrite(wp, T(" \t\t<option selected value=%d>%s(lowest)</option>\n"),  IPQOS_NUM_PRIOQ-1, pPrio);
		else 
			nBytesSent += websWrite(wp, T(" \t\t<option  value=%d>%s(lowest)</option>\n"),  IPQOS_NUM_PRIOQ-1, pPrio);
#else
	if(m==IPQOS_NUM_PRIOQ-1)  
	      nBytesSent += websWrite(wp, T(" \t\t<option selected value=%d>%s(最低)</option>\n"),  IPQOS_NUM_PRIOQ-1, pPrio);
	else 
	       nBytesSent += websWrite(wp, T(" \t\t<option  value=%d>%s(最低)</option>\n"),  IPQOS_NUM_PRIOQ-1, pPrio);
#endif
	for (j=IPQOS_NUM_PRIOQ-2; j>=1; j--) {
		pPrio = prioLevel[j];
	   if(m==j)
		nBytesSent += websWrite(wp, T("  \t\t<option selected value=%d>%s</option>\n"), j, pPrio);
	   else 
	   	nBytesSent += websWrite(wp, T("  \t\t<option  value=%d>%s</option>\n"), j, pPrio);
	}
	pPrio = prioLevel[0];
#ifdef NEW_IP_QOS_SUPPORT
		if(m==0)
			nBytesSent += websWrite(wp, T("  \t\t<option selected value=0>%s(highest)</option>\n"), pPrio);
		else
			nBytesSent += websWrite(wp, T("  \t\t<option  value=0>%s(highest)</option>\n"), pPrio);
#else
	if(m==0)
	  nBytesSent += websWrite(wp, T("  \t\t<option selected value=0>%s(最高)</option>\n"), pPrio);
	else 
	   nBytesSent += websWrite(wp, T("  \t\t<option  value=0>%s(最高)</option>\n"), pPrio);
#endif
	nBytesSent += websWrite(wp, T("  '\t</select>\n"));
      nBytesSent += websWrite(wp, T( "</tr>"));
   }
 
   return nBytesSent;
}

#ifdef NEW_IP_QOS_SUPPORT
int setting_predprio(int eid, webs_t wp, int argc, char_t **argv)
{
	int nBytesSent=0, i,j,m=0;
	const char *pPrio;
	const char *nset1ptbl;
	unsigned char value[8];
	
	nBytesSent += websWrite(wp, T("<tr>"
		"<th bgcolor=\"#808080\" colspan=1><font size=\"2\">IP Precedence</font></th>\n"
		"<th bgcolor=\"#808080\" colspan=1><font size=\"2\">transmit priority</font></th>\n</tr>\n"
		));

	if(mib_get(MIB_PRED_PROI, (void *)value)== 0)
	{
		printf("Get PRED_PRIO  error!\n");
		return 0;
	}

	for(i=0;i<IPQOS_NUM_PKT_PRIO;i++)
	{
		m= value[i];
		nset1ptbl =setpredtable[i];

		nBytesSent += websWrite(wp, T( "<tr>"));
		nBytesSent += websWrite(wp, T( "<td align=center width=\"50%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%d</td>\n"),i);
		nBytesSent += websWrite(wp, T("  <td align=center width=\"50%%\" bgcolor=\"#C0C0C0\"><select name=\"%s\">\n"),nset1ptbl);

		pPrio = prioLevel[IPQOS_NUM_PRIOQ-1];
		if(m==IPQOS_NUM_PRIOQ-1) 
			nBytesSent += websWrite(wp, T(" \t\t<option selected value=%d>%s(lowest)</option>\n"),  IPQOS_NUM_PRIOQ-1, pPrio);
		else 
			nBytesSent += websWrite(wp, T(" \t\t<option  value=%d>%s(lowest)</option>\n"),  IPQOS_NUM_PRIOQ-1, pPrio);

		for (j=IPQOS_NUM_PRIOQ-2; j>=1; j--) {
			pPrio = prioLevel[j];
			if(m==j)
				nBytesSent += websWrite(wp, T("  \t\t<option selected value=%d>%s</option>\n"), j, pPrio);
			else 
				nBytesSent += websWrite(wp, T("  \t\t<option  value=%d>%s</option>\n"), j, pPrio);
		}
		pPrio = prioLevel[0];
		if(m==0)
			nBytesSent += websWrite(wp, T("  \t\t<option selected value=0>%s(highest)</option>\n"), pPrio);
		else
			nBytesSent += websWrite(wp, T("  \t\t<option  value=0>%s(highest)</option>\n"), pPrio);
		nBytesSent += websWrite(wp, T("  '\t</select>\n"));
		nBytesSent += websWrite(wp, T( "</tr>"));
	}

	return nBytesSent;
}
#endif
#endif


