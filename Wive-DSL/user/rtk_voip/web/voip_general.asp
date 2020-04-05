<html>
<head>
<meta http-equiv="Content-Type" content="text/html">
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Cache-Control" CONTENT="no-cache">
<title>SIP</title>
<script language="javascript" src=voip_script.js></script>
<style> TABLE {width:375} </style>
<script language="javascript">
<!--
function spd_dial_remove_sel()
{
	var flag=false;

	for (var i=0; i<10; i++)
	{
		if (document.sipform.spd_sel[i].checked)
		{
			flag=true;
			break;
		}
	}

	if (!flag)
	{
		alert('You have to select first.');
		return false;
	}

	if (!confirm('Do you really want to remove the selected items?')) 
	{
		return false;
	} 
		

	for (var i=0; i<10; i++)
	{
		if (document.sipform.spd_sel[i].checked)
		{
			document.sipform.spd_sel[i].checked = false;
			document.sipform.spd_sel[i].disabled = true;
			document.all.spd_name[i].value = "";
			document.all.spd_url[i].value = "";
		}
	}
	return true;
}

function spd_dial_remove_all()
{
	if (!confirm('Do you really want to remove all items in the phone book?')) 
	{
		return false;  
	}  

	for (var i=0; i<10; i++)
	{
		document.sipform.spd_sel[i].checked = false;
		document.sipform.spd_sel[i].disabled = true;
		document.all.spd_name[i].value = "";
		document.all.spd_url[i].value = "";
	}
	return true;
}

function spd_dial_edit()
{
	for (var i=0; i<10; i++)
	{
		document.sipform.spd_sel[i].disabled = document.all.spd_url[i].value == "";
		if (document.sipform.spd_sel[i].disabled)
			document.all.spd_name[i].value = "";
	}
}

function dtmfMode_change()
{
	document.sipform.payloadType.disabled = (document.sipform.dtmfMode.selectedIndex != 0);
	document.sipform.sipInfo_duration.disabled = (document.sipform.dtmfMode.selectedIndex != 1);
}

function enable_hotline()
{
	document.sipform.hotline_number.disabled = !document.sipform.hotline_enable.checked;
}

function enable_dnd()
{
	document.sipform.dnd_from_hour.disabled = !document.sipform.dnd_mode[1].checked;
	document.sipform.dnd_from_min.disabled = !document.sipform.dnd_mode[1].checked;
	document.sipform.dnd_to_hour.disabled = !document.sipform.dnd_mode[1].checked;
	document.sipform.dnd_to_min.disabled = !document.sipform.dnd_mode[1].checked;
}
//-->
</script>
</head>
<body bgcolor="#ffffff" text="#000000">

<form method="post" action="/goform/voip_general_set" name=sipform>
<input type=hidden name=voipPort value="<%voip_general_get("voip_port");%>">

<%voip_general_get("proxy");%>

<p>
<b>NAT Traversal</b>
<table cellSpacing=1 cellPadding=2 border=0>
<%voip_general_get("stun");%>
</table>

<p>
<b>SIP Advanced</b>
<table cellSpacing=1 cellPadding=2 border=0>
	<tr>
		<td bgColor=#aaddff>SIP Port</td>
    	<td bgColor=#ddeeff>
		<input type=text name=sipPort size=10 maxlength=5 value="<%voip_general_get("sipPort"); %>">
		<%voip_general_get("sipPorts");%>
		</td>
	</tr>
	<tr>
		<td bgColor=#aaddff>Media Port</td>
    	<td bgColor=#ddeeff>
		<input type=text name=rtpPort size=10 maxlength=5 value="<%voip_general_get("rtpPort"); %>">
		<%voip_general_get("rtpPorts");%>
		</td>
	</tr>
  	<tr>
    	<td bgColor=#aaddff>DMTF Relay</td>
    	<td bgColor=#ddeeff>
			<select name=dtmfMode onchange="dtmfMode_change()">
				"<%voip_general_get("dtmfMode");%>"
			</select>
		</td>
	</tr>
	<tr>
		<td bgColor=#aaddff>RFC2833 Payload Type</td>
		<td bgColor=#ddeeff>
			<input type=text name=payloadType size=12 maxlength=3 value=<%voip_general_get("payloadType");%>>
		</td>
	</tr>
	<tr>
		<td bgColor=#aaddff>SIP INFO Duration (ms)</td>
		<td bgColor=#ddeeff>
			<input type=text name=sipInfo_duration size=12 maxlength=4 value=<%voip_general_get("sipInfo_duration");%>>
		</td>
	</tr>
	<tr>
		<td bgColor=#aaddff>Call Waiting</td>
		<td bgColor=#ddeeff><input type=checkbox name=call_waiting onclick="enable_callwaiting();" <%voip_general_get("call_waiting");%>>Enable</td>
	</tr>
	
	<tr>
		<td bgColor=#aaddff>Call Waiting Caller ID</td>
		<td bgColor=#ddeeff><input type=checkbox name=call_waiting_cid <%voip_general_get("call_waiting_cid");%>>Enable</td>
	</tr>
</table>

<p>
<b>Forward Mode</b>
<table cellSpacing=1 cellPadding=2 border=0>
	<tr>
	<td bgColor=#aaddff width=155>Immediate Forward to</td>
	<td bgColor=#ddeeff width=170>
	<%voip_general_get("CFAll");%>
	</td>
	</tr>	
    <tr>
    <td bgColor=#aaddff>Immediate Number</td>
    <td bgColor=#ddeeff>
<%voip_general_get("CFAll_No");%>
	</td>
	</tr>
	<tr>
	<td bgColor=#aaddff>Busy Forward to</td>
	<td bgColor=#ddeeff>
	<%voip_general_get("CFBusy");%>
	</td>
	</tr>	
    <tr>
    <td bgColor=#aaddff>Busy Number</td>
    <td bgColor=#ddeeff>
<%voip_general_get("CFBusy_No");%>
	</td>
	</tr>
	<tr>
	<td bgColor=#aaddff>No Answer Forward to</td>
	<td bgColor=#ddeeff>
	<%voip_general_get("CFNoAns");%>
	</td>
	</tr>	
    <tr>
    <td bgColor=#aaddff>No Answer Number</td>
    <td bgColor=#ddeeff>
<%voip_general_get("CFNoAns_No");%>
	</td>
	</tr>
    <td bgColor=#aaddff>No Answer Time (sec)</td>
    <td bgColor=#ddeeff>
<%voip_general_get("CFNoAns_Time");%>
	</td>
	</tr>      
</table>


<%voip_general_get("speed_dial_display_title");%>
<table cellSpacing=1 cellPadding=2 border=0 <%voip_general_get("speed_dial_display");%>>
	<tr align=center>
		<td bgcolor=#aaddff>Position</td>
		<td bgcolor=#aaddff>Name</td>
		<td bgcolor=#aaddff>Phone Number</td>
		<td bgcolor=#aaddff>Select</td>
	</tr>
	<%voip_general_get("speed_dial");%>
	<tr align=center>
		<td colspan=4 bgcolor=#ddeeff>
		<input type=button value="Remove Selected" name="RemoveSelected" onClick="spd_dial_remove_sel()">
		<input type=button value="Remove All" name="RemoveAll" onClick="spd_dial_remove_all()">
		</td>
	</tr>
</table>

<%voip_general_get("display_dialplan_title");%>
<table cellSpacing=1 cellPadding=1 border=0 <%voip_general_get("display_dialplan");%>>
	<tr>
    	<td bgColor=#aaddff width=155>Replace prefix code</td>
    	<td bgColor=#ddeeff width=170>
    	<%voip_general_get("ReplaceRuleOption");%>
    	</td>		
	</tr>
	<tr>
     	<td bgColor=#aaddff width=155>Relace rule</td>
   		<td bgColor=#ddeeff width=170>
    	<input type="text" name="ReplaceRuleSource" size=12 maxlength=79 value="<%voip_general_get("ReplaceRuleSource");%>"> ->
    	<input type="text" name="ReplaceRuleTarget" size=3 maxlength=9 value="<%voip_general_get("ReplaceRuleTarget");%>"></td>
	</tr>
  	<tr>
    	<td bgColor=#aaddff width=155>Dial Plan</td>
    	<td bgColor=#ddeeff width=170>
		<input type="text" name="dialplan" size=20 maxlength=79 value="<%voip_general_get("dialplan");%>"></td>
	</tr>
	<tr>
     	<td bgColor=#aaddff width=155>Auto Prefix</td>
    	<td bgColor=#ddeeff width=170>
    	<input type="text" name="AutoPrefix" size=5 maxlength=4 value="<%voip_general_get("AutoPrefix");%>">
	</tr>
	<tr>
     	<td bgColor=#aaddff width=155>Prefix Unset Plan</td>
    	<td bgColor=#ddeeff width=170>
    	<input type="text" name="PrefixUnsetPlan" size=20 maxlength=79 value="<%voip_general_get("PrefixUnsetPlan");%>">
	</tr>
</table>

<p>
<b>Codec</b>
<%voip_general_get("codec_var");%>
<table cellSpacing=1 cellPadding=2 border=0>
<%voip_general_get("codec");%>
</table>

<p>
<table cellSpacing=1 cellPadding=1 border=0 <%voip_general_get("display_voice_qos");%>>
<b>QoS</b>
  	<tr>
    	<td bgColor=#aaddff>Voice QoS</td>
		<td bgColor=#ddeeff>
		<select name=voice_qos>
			 "<%voip_general_get("voice_qos");%>"
		</select>
		</td>
		<td bgColor=#ddeeff></td>
	</tr>
</table>

<!-- ++T.38 config add by Jack Chan++ -->
<p>
<!-- style:display:none(hidden) style:display:table(show) -->
<table  cellSpacing=1 cellPadding=1 border=0 <%voip_general_get("T38_BUILD");%>>
<b>T.38(FAX)</b>
	<tr>
		<td bgColor=#aaddff>T.38</td>
		<td bgColor=#ddeeff><input type=checkbox name=useT38 size=20 <%voip_general_get("useT38");%>>Enable</td>
	</tr>
	<tr>
		<td bgColor=#aaddff>T.38 Port</td>
		<td bgColor=#ddeeff><input type=text name=T38_PORT size=20 maxlength=39 value="<%voip_general_get("T38_PORT");%>"></td>
		<%voip_general_get("t38Ports");%>
	</tr>
</table>
<!-- --end-- -->

<%voip_general_get("hotline_option_display_title");%>
<table cellSpacing=1 cellPadding=2 border=0 width=450 <%voip_general_get("hotline_option_display");%>>
<tr>
   	<td bgColor=#aaddff width=150>Use Hot Line</td>
	<td bgColor=#ddeeff>
		<input type=checkbox name=hotline_enable onClick="enable_hotline()" <%voip_general_get("hotline_enable");%>>Enable
	</td>
</tr>
<tr>
   	<td bgColor=#aaddff width=150>Hot Line Number</td>
	<td bgColor=#ddeeff>
		<input type=text name=hotline_number size=20 maxlength=39 value="<%voip_general_get("hotline_number");%>">
	</td>
</tr>
</table>
<script language=javascript>enable_hotline()</script>

<p>
<b>DND (Don't Disturb)</b>
<table cellSpacing=1 cellPadding=2 border=0 width=450>
<tr>
   	<td bgColor=#aaddff width=150>DND Mode</td>
	<td bgColor=#ddeeff>
		<input type=radio name=dnd_mode value=2 onClick="enable_dnd()" <%voip_general_get("dnd_always");%>>Always
		<input type=radio name=dnd_mode value=1 onClick="enable_dnd()" <%voip_general_get("dnd_enable");%>>Enable
		<input type=radio name=dnd_mode value=0 onClick="enable_dnd()" <%voip_general_get("dnd_disable");%>>Disable
	</td>
</tr>
<tr>
   	<td bgColor=#aaddff width=150>From</td>
	<td bgColor=#ddeeff>
		<input type=text name=dnd_from_hour size=3 maxlength=2 value="<%voip_general_get("dnd_from_hour");%>">:
		<input type=text name=dnd_from_min size=3 maxlength=2 value="<%voip_general_get("dnd_from_min");%>">
		(hh:mm)
	</td>
</tr>
<tr>
   	<td bgColor=#aaddff width=150>To</td>
	<td bgColor=#ddeeff>
		<input type=text name=dnd_to_hour size=3 maxlength=2 value="<%voip_general_get("dnd_to_hour");%>">:
		<input type=text name=dnd_to_min size=3 maxlength=2 value="<%voip_general_get("dnd_to_min");%>">
		(hh:mm)
	</td>
</tr>
</table>
<script language=javascript>enable_dnd()</script>

<p>
<b>DSP</b>
<table cellSpacing=1 cellPadding=2 border=0>
<!--
  	<tr>
    	<td bgColor=#aaddff width=155 rowspan=2>FXS Volume</td>
		<td bgColor=#ddeeff width=170>Handset Gain</td>
		<td bgColor=#ddeeff>
			<select name=slic_txVolumne>
				  "<%voip_general_get("slic_txVolumne");%>"
			</select>
		</td>
	</tr>
	<tr>
		<td bgColor=#ddeeff width=170>Handset Volume</td>
		<td bgColor=#ddeeff>
			<select name=slic_rxVolumne>
				 "<%voip_general_get("slic_rxVolumne");%>"
			</select>
		</td>
	</tr>
-->

  	<tr>
    	<td bgColor=#aaddff width=155 rowspan=3>Jitter Buffer Control</td>
		<td bgColor=#ddeeff width=170>
		Min delay (ms): 
		</td>
		<td bgColor=#ddeeff>
		<select name=jitterDelay>
			<%voip_general_get("jitterDelay");%>
		</select>
		</td>
	</tr>
	<tr>
		<td bgcolor=#ddeeff width=170>
		Max delay (ms):
		</td>
		<td bgColor=#ddeeff>
		<select name=maxDelay>
	        <%voip_general_get("maxDelay");%>
	    </select>
		</td>
	</tr>
	<tr>
		<td bgcolor=#ddeeff width=170>
		Optimization factor:
		</td>
		<td bgColor=#ddeeff>
		<select name=jitterFactor>
			<%voip_general_get("jitterFactor");%>
		</select>
		</td>
	</tr>

<!--
  	<tr>
    	<td bgColor=#aaddff>LEC Tail Length (ms)</td>
		<td bgColor=#ddeeff>
		<select name=echoTail>
			 "<%voip_general_get("echoTail");%>"
		</select>
		</td>
		<td bgColor=#ddeeff></td>
	</tr>
-->

  	<tr>
    	<td bgColor=#aaddff width=155>Vad</td>
		<td bgColor=#ddeeff width=170>
			<input type=checkbox name=useVad size=20 <%voip_general_get("useVad");%>>Enable
		</td>
		<td bgColor=#ddeeff></td>
	</tr>

  	<tr>
    	<td bgColor=#aaddff width=155 rowspan=4>Speaker AGC</td>
		<td bgColor=#ddeeff>
			<input type=checkbox name=CFuseSpeaker size=20 onClick="enableCFSpkAGC(this.checked)" <%voip_general_get("CFuseSpeaker");%>>Enable
		</td>
		<td bgColor=#ddeeff></td>
	</tr>

  	<tr>
    	
		<td bgColor=#ddeeff width=170>require level:</td>
		<td bgColor=#ddeeff>
			<select name=CF_spk_AGC_level>
				  "<%voip_general_get("CF_spk_AGC_level");%>"
			</select>
		</td>
	</tr>


  	<tr>
    	
		<td bgColor=#ddeeff width=170>Max gain up: dB</td>
		<td bgColor=#ddeeff>
			<select name=CF_spk_AGC_up_limit>
				  "<%voip_general_get("CF_spk_AGC_up_limit");%>"
			</select>
		</td>
	</tr>
	<tr>
		<td bgColor=#ddeeff width=170>Max gain down: dB</td>
		<td bgColor=#ddeeff>
			<select name=CF_spk_AGC_down_limit>
				 "<%voip_general_get("CF_spk_AGC_down_limit");%>"
			</select>
		</td>
	</tr>

  	<tr>
    	<td bgColor=#aaddff width=155 rowspan=4>MIC AGC</td>
		<td bgColor=#ddeeff>
			<input type=checkbox name=CFuseMIC size=20 onClick="enableCFMicAGC(this.checked)" <%voip_general_get("CFuseMIC");%>>Enable
		</td>
		<td bgColor=#ddeeff></td>
	</tr>

  	<tr>
    	
		<td bgColor=#ddeeff width=170>require level:</td>
		<td bgColor=#ddeeff>
			<select name=CF_mic_AGC_level>
				  "<%voip_general_get("CF_mic_AGC_level");%>"
			</select>
		</td>
	</tr>

  	<tr>
    	
		<td bgColor=#ddeeff width=170>Max gain up: dB</td>
		<td bgColor=#ddeeff>
			<select name=CF_mic_AGC_up_limit>
				  "<%voip_general_get("CF_mic_AGC_up_limit");%>"
			</select>
		</td>
	</tr>
	<tr>
		<td bgColor=#ddeeff width=170>Max gain down: dB</td>
		<td bgColor=#ddeeff>
			<select name=CF_mic_AGC_down_limit>
				 "<%voip_general_get("CF_mic_AGC_down_limit");%>"
			</select>
		</td>
	</tr>

  	<tr>
    	<td bgColor=#aaddff>Caller ID Mode</td>
	<td bgColor=#ddeeff><%voip_general_get("caller_id");%></td>
		<td bgColor=#ddeeff></td>
	</tr>
	
	<tr>
		<td bgColor=#aaddff>FSK Date & Time Sync</td>
	<td bgColor=#ddeeff><%voip_general_get("FSKdatesync");%></td>
			<td bgColor=#ddeeff></td>
	</tr>
	
	<tr>
		<td bgColor=#aaddff>Reverse Polarity before Caller ID</td>
	<td bgColor=#ddeeff><%voip_general_get("revPolarity");%></td>
			<td bgColor=#ddeeff></td>
	</tr>

	<tr>
		<td bgColor=#aaddff>Short Ring before Caller ID</td>
	<td bgColor=#ddeeff><%voip_general_get("sRing");%></td>
			<td bgColor=#ddeeff></td>
	</tr>

	<tr>
		<td bgColor=#aaddff>Dual Tone before Caller ID</td>
	<td bgColor=#ddeeff><%voip_general_get("dualTone");%></td>
			<td bgColor=#ddeeff></td>
	</tr>

	<tr>
		<td bgColor=#aaddff>Caller ID Prior First Ring</td>
	<td bgColor=#ddeeff><%voip_general_get("PriorRing");%></td>
			<td bgColor=#ddeeff></td>
	</tr>

	<tr>
    		<td bgColor=#aaddff>Caller ID DTMF Start Digit</td>
	<td bgColor=#ddeeff><%voip_general_get("cid_dtmfMode_S");%></td>
		<td bgColor=#ddeeff></td>
	</tr>

	<tr>
    		<td bgColor=#aaddff>Caller ID DTMF End Digit</td>
	<td bgColor=#ddeeff><%voip_general_get("cid_dtmfMode_E");%></td>
		<td bgColor=#ddeeff></td>
	</tr>

	<tr>
		<td bgColor=#aaddff>Caller ID Soft FSK Gen</td>
	<td bgColor=#ddeeff><%voip_general_get("SoftFskGen");%></td>
	<td bgColor=#ddeeff>Hardware caller id only support si3215/3210 slic</td>
	</tr>

	<tr>
	   	<td bgColor=#aaddff width=155>Flash Time Setting (ms) [ Space:10, Min:30, Max:2000 ]</td>
		<td bgColor=#ddeeff width=170>
		<%voip_general_get("flash_hook_time");%> 
		</td>
		<td bgColor=#ddeeff></td>
	</tr>
	
	
	<tr>
		<td bgColor=#aaddff width=155>Speaker Voice Gain (dB) [ -32~31 ],Mute:-32</td>
		<td bgColor=#ddeeff width=170>
			<input type=text name=spk_voice_gain size=4 maxlength=5 value="<%voip_general_get("spk_voice_gain");%>">
		</td>
		<td bgColor=#ddeeff></td>
	</tr>

	<tr>
		<td bgColor=#aaddff width=155>Mic Voice Gain (dB) [ -32~31 ],Mute:-32</td>
		<td bgColor=#ddeeff width=170>
			<input type=text name=mic_voice_gain size=4 maxlength=5 value="<%voip_general_get("mic_voice_gain");%>">
		</td>
		<td bgColor=#ddeeff></td>
	</tr>

	
	<tr>
    	<td colspan=3 align=center>
    		<input type="button" value="Apply" onclick="changeStartEnd();">    		
    		&nbsp;&nbsp;&nbsp;&nbsp;    	
    		<input type="reset" value="Reset">	
    	</td>
	</tr>
</table>
<script language=javascript>init();</script>
<p>
</form>
</table>
</body>
</html>
