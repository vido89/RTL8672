<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>System Command</title>
<script type="text/javascript" src="share.js">
</script>
<script language="javascript">
var addr = '<% getInfo("syslog-server-ip"); %>';
var port = '<% getInfo("syslog-server-port"); %>';
function getLogPort() {
	var portNum = parseInt(port);
	if (isNaN(portNum) || portNum == 0)
		portNum = 514; // default system log server port is 514

	return portNum;
}

function hideInfo(hide) {
	var status = 'visible';

	if (hide == 1) {
		status = 'hidden';
		document.forms[0].logAddr.value = '';
		document.forms[0].logPort.value = '';
		changeBlockState('srvInfo', true);
	} else {
		changeBlockState('srvInfo', false);
		document.forms[0].logAddr.value = addr;
		document.forms[0].logPort.value = getLogPort();
	}
}

function hidesysInfo(hide) {
	var status = false;

	if (hide == 1) {
		status = true;
	}
	changeBlockState('sysgroup', status);
}

function changelogstatus() {
	with (document.forms[0]) {
		if (logcap[1].checked) {
			hidesysInfo(0);
			if (logMode.selectedIndex == 0) {
				hideInfo(1);
			} else {
				hideInfo(0);
			}
		} else {
			hidesysInfo(1);
			hideInfo(1);
		}
	}
}

function cbClick(obj) {
	var idx = obj.selectedIndex;
	var val = obj.options[idx].value;
	if (val == 1)
		hideInfo(1);
	else
		hideInfo(0);
}

function check_enable()
{
	if (document.formSysLog.logcap[0].checked) {
		//disableTextField(document.formSysLog.msg);
		disableButton(document.formSysLog.refresh);		
	}
	else {
		//enableTextField(document.formSysLog.msg);
		enableButton(document.formSysLog.refresh);
	}
}               

/*function scrollElementToEnd (element) {
   if (typeof element.scrollTop != 'undefined' &&
       typeof element.scrollHeight != 'undefined') {
     element.scrollTop = element.scrollHeight;
   }
}*/

function saveClick()
{
	<% RemoteSyslog("check-ip"); %>
//	if (document.forms[0].logAddr.disabled == false && !checkIP(document.formSysLog.logAddr))
//		return false;
//	alert("Please commit and reboot this system for take effect the System log!");
	return true;
}

</script>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">System Log</font></h2>
<form action=/goform/formSysLog method=POST name=formSysLog>
<table border="0" cellspacing="4" width="500">
<tr><hr size=1 noshade align=top></tr>
<tr>
	<td width="25%"><font size=2><b>System Log&nbsp;:</b></td>
	<td width="30%"><font size=2>
		<input type="radio" value="0" name="logcap" onClick='changelogstatus()' <% checkWrite("log-cap0"); %>>Disable&nbsp;&nbsp;
		<input type="radio" value="1" name="logcap" onClick='changelogstatus()' <% checkWrite("log-cap1"); %>>Enable
	</td>
</tr>    
<% ShowPPPSyslog("syslogppp"); %>		
<TBODY id='sysgroup'>
<tr>
	<td><font size=2><b>Log Level&nbsp;:</b></td>
	<td><select name='levelLog' size="1">
		<% checkWrite("syslog-log"); %>
	</select></td>
</tr>
<tr>
	<td><font size=2><b>Display Level&nbsp;:</b></td>
	<td ><select name='levelDisplay' size="1">
		<% checkWrite("syslog-display"); %>
	</select></td>
</tr>
<% RemoteSyslog("syslog-mode"); %>
<!--tr>
	<td><font size=2><b>Mode&nbsp;:</b></td>
	<td><select name='logMode' size="1" onChange='cbClick(this)'>
		<% checkWrite("syslog-mode"); %>
	</select></td>
</tr-->
<tbody id='srvInfo'>
<% RemoteSyslog("server-info"); %>
<!--tr>
	<td><font size=2><b>Server IP Address&nbsp;:</b></td>
	<td><input type='text' name='logAddr' maxlength="15"></td>
</tr>
<tr>
	<td><font size=2><b>Server UDP Port&nbsp;:</b></td>
	<td><input type='text' name='logPort' maxlength="15"></td>
</tr-->
</tbody>
</TBODY>
<tr>
	<td width="45%">	<input type="submit" value="Apply Changes" name="apply" onClick="return saveClick()"></td>
</tr>
   
<tr>
	<td width="25%"><font size=2><b>Save Log to File:</b></td>
	<td width="30%"><font size=2><input type="submit" value="Save..." name="save_log"></td>
</tr>
<tr>
	<td width="25%"><font size=2><b>Clear Log:</b></td>
	<td width="30%"><font size=2><input type="submit" value="Reset" name="clear_log"></td>
</tr>
</table>
<table border="0" cellspacing="4" width="500">
<tr><hr size=1 noshade align=top></tr>
<tr>
	<td width="25%"><font size=2><b>System Log</b></td>
	<td width="30%"><font size=2><input type="button" value="Refresh" name="refresh" onClick="javascript: window.location.reload()"></td>
</tr>
<tr>
	<td>
	<div style="overflow: auto; height: 500px; width: 500px; PADDING-LEFT: 10px; PADDING-TOP: 10px; PADDING-RIGHT: 10px; PADDING-BOTTOM: 10px">
	<table border="0" width="100%"><% sysLogList(); %></table>
	</td>
</tr>
</table>
<!--textarea rows="15" name="msg" cols="80" wrap="virtual"><% sysLogList();%></textarea-->


<input type="hidden" value="/syslog.asp" name="submit-url">
<script>
	check_enable();
	//scrollElementToEnd(this.formSysLog.msg);
</script>
</form>
<script>
	<% initPage("syslog"); %>
	<% initPage("pppSyslog"); %>
</script>
</blockquote>
</body>
</html>


