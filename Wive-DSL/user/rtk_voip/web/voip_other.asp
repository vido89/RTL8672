<html>
<head>
<meta http-equiv="Content-Type" content="text/html">
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Cache-Control" CONTENT="no-cache">
<title>SIP</title>
<script language="javascript" src=voip_script.js></script>
</head>
<body bgcolor="#ffffff" text="#000000">
<form method="get" action="/goform/voip_other_set" name=other_form>

<b>Function Key</b>
<table cellSpacing=1 cellPadding=2 border=0 width=450>
<tr bgColor=#888888>
	<td colspan=2>
	<font color=#ffffff>
	Must be * + 0~9
	</font>
	</td>
</tr>	
<tr <%voip_other_get("display_funckey_pstn");%>>
   	<td bgColor=#aaddff width=150>Switch to PSTN</td>
	<td bgColor=#ddeeff>
		<input type=text name=funckey_pstn size=5 maxlength=2 value="<%voip_other_get("funckey_pstn");%>">
		( default: *0 )
	</td>
</tr>
<tr>
   	<td bgColor=#aaddff>Call Transfer</td>
	<td bgColor=#ddeeff>
		<input type=text name=funckey_transfer size=5 maxlength=2 value="<%voip_other_get("funckey_transfer");%>">
		( default: *1 )
	</td>
</tr>	
</table>

<%voip_other_get("auto_dial_display_title");%>
<table cellSpacing=1 cellPadding=2 border=0 width=450 <%voip_other_get("auto_dial_display");%>>
<tr>
   	<td bgColor=#aaddff width=150>Auto Dial Time</td>
	<td bgColor=#ddeeff>
		<input type=text name=auto_dial size=3 maxlength=1 value="<%voip_other_get("auto_dial");%>">
		( 3~9 sec, 0 is disable )
	</td>
</tr>
<tr>
   	<td bgColor=#aaddff width=150>Dial-out by Hash Key</td>
	<td bgColor=#ddeeff>
		<input type=checkbox name=auto_dial_always <%voip_other_get("auto_dial_always");%>>Disabled
	</td>
</tr>
</table>

<%voip_other_get("off_hook_alarm_display_title");%>
<table cellSpacing=1 cellPadding=2 border=0 width=450 <%voip_other_get("off_hook_alarm_display");%>>
<tr>
   	<td bgColor=#aaddff width=150>Off-Hook Alarm Time</td>
	<td bgColor=#ddeeff>
		<input type=text name=off_hook_alarm size=3 maxlength=2 value="<%voip_other_get("off_hook_alarm");%>">
		( 10~60 sec, 0 is disable )
	</td>
</tr>
</table>

<p <%voip_other_get("display_cid_det");%>>
<b>Caller ID Detection</b>
<table cellSpacing=1 cellPadding=2 border=0 width=450>
<tr>
<td bgColor=#aaddff width=150>Auto Detection</td>
<td bgColor=#aaddff> 
<%voip_other_get("caller_id_auto_det");%>
</td>
<input type=hidden name=caller_id_test value=hello>
</tr>	
<tr>
	<td bgColor=#aaddff width=150>Caller ID Detection Mode</td>
	<td bgColor=#ddeeff>
	<%voip_other_get("caller_id_det");%>
	</td>
</tr>
</table>
</p>

<p>
<b>QoS</b>
<table cellSpacing=1 cellPadding=2 border=0 width=450>
    	<tr>
    	<td bgColor=#aaddff>SIP DSCP</td>
		<td bgColor=#ddeeff>
		<select name=sipDscp>
			 "<%voip_other_get("sipDscp");%>"
		</select>
		</td>
		<td bgColor=#ddeeff></td>
	</tr>
	<tr>
    	<td bgColor=#aaddff>RTP DSCP</td>
		<td bgColor=#ddeeff>
		<select name=rtpDscp>
			 "<%voip_other_get("rtpDscp");%>"
		</select>
		</td>
		<td bgColor=#ddeeff></td>
	</tr>
</table>

<p>
<p <%voip_other_get("display_wanVlan");%>>
<b>VLAN Tags for Voice, Data and Video.</b>
<p>
Note: The Video port is near the WAN port. (i.e. port 3)
<table cellSpacing=1 cellPadding=1 border=0 width=450>

<tr>
	<td bgColor=#aaddff width=155>Use VLAN Tags</td>
	<td bgColor=#ddeeff width=170>
		<input type=checkbox name=wanVlanEnable onClick="enableWanVlan();" <%voip_other_get("wanVlanEnable");%>>Enable
	</td>
</tr>

<tr>
   	<td bgColor=#aaddff>Voice: VLAN ID</td>
	<td bgColor=#ddeeff>
		<input type=text name=wanVlanIdVoice size=5 maxlength=4 value="<%voip_other_get("wanVlanIdVoice");%>">
                ( 1~4090 )
	</td>
</tr>
	
<tr>
   	<td bgColor=#aaddff>Voice: User Priority</td>
	<td bgColor=#ddeeff>
		<input type=text name=wanVlanPriorityVoice size=5 maxlength=1 value="<%voip_other_get("wanVlanPriorityVoice");%>">
		( 0~7 )
	</td>
</tr>
	
<tr>
   	<td bgColor=#aaddff>Voice: CFI</td>
	<td bgColor=#ddeeff>
		<input type=text name=wanVlanCfiVoice size=5 maxlength=1 value="<%voip_other_get("wanVlanCfiVoice");%>">
		( 0~1 )
	</td>
</tr>	

<tr>
   	<td bgColor=#aaddff>Data: VLAN ID</td>
	<td bgColor=#ddeeff>
		<input type=text name=wanVlanIdData size=5 maxlength=4 value="<%voip_other_get("wanVlanIdData");%>">
                ( 1~4090 )
	</td>
</tr>
	
<tr>
   	<td bgColor=#aaddff>Data: User Priority</td>
	<td bgColor=#ddeeff>
		<input type=text name=wanVlanPriorityData size=5 maxlength=1 value="<%voip_other_get("wanVlanPriorityData");%>">
		( 0~7 )
	</td>
</tr>
	
<tr>
   	<td bgColor=#aaddff>Data: CFI</td>
	<td bgColor=#ddeeff>
		<input type=text name=wanVlanCfiData size=5 maxlength=1 value="<%voip_other_get("wanVlanCfiData");%>">
		( 0~1 )
	</td>
</tr>	

<tr>
   	<td bgColor=#aaddff>Video: VLAN ID</td>
	<td bgColor=#ddeeff>
		<input type=text name=wanVlanIdVideo size=5 maxlength=4 value="<%voip_other_get("wanVlanIdVideo");%>">
                ( 1~4090 )
	</td>
</tr>
	
<tr>
   	<td bgColor=#aaddff>Video: User Priority</td>
	<td bgColor=#ddeeff>
		<input type=text name=wanVlanPriorityVideo size=5 maxlength=1 value="<%voip_other_get("wanVlanPriorityVideo");%>">
		( 0~7 )
	</td>
</tr>
	
<tr>
   	<td bgColor=#aaddff>Video: CFI</td>
	<td bgColor=#ddeeff>
		<input type=text name=wanVlanCfiVideo size=5 maxlength=1 value="<%voip_other_get("wanVlanCfiVideo");%>">
		( 0~1 )
	</td>
</tr>	


<script language=javascript>enableWanVlan();</script>
</table>
</p>

<table cellSpacing=1 cellPadding=2 border=0 width=375>
<tr>
   	<td colspan=3 align=center>
   		<input type="submit" value="Apply" onclick="return check_other()">
   		&nbsp;&nbsp;&nbsp;&nbsp;    	
   		<input type="reset" value="Reset">	
   	</td>
</tr>
</table>


</form>
</body>
</html>
