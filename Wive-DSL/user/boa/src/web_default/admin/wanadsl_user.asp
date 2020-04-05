<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>ADSL Connection Mode</title>
<script type="text/javascript" src="share.js">
</script>
<script>

var initConnectMode;
var pppConnectStatus=0;

function isNetscape(v) {
	return isBrowser("Netscape", v);
}

function isMicrosoft(v) {
	return isBrowser("Microsoft", v);
}

function pppTypeSelection()
{
	if ( document.adsl.pppConnectType.selectedIndex == 2) {
	/*
		if (pppConnectStatus==0) {
			enableButtonIB(document.adsl.pppConnect);
			disableButtonIB(document.adsl.pppDisconnect);
		}
		else {
	       		disableButtonIB(document.adsl.pppConnect);
			enableButtonIB(document.adsl.pppDisconnect);
		}
	*/
		document.adsl.pppIdleTime.value = "";
		disableTextField(document.adsl.pppIdleTime);
	}
	else {
		disableButtonIB(document.adsl.pppConnect);
		disableButtonIB(document.adsl.pppDisconnect);
		if (document.adsl.pppConnectType.selectedIndex == 1) {
			document.adsl.pppIdleTime.value = 600;
			enableTextField(document.adsl.pppIdleTime);
		}
		else {
			document.adsl.pppIdleTime.value = "";
			disableTextField(document.adsl.pppIdleTime);
		}
	}
}

function pppConnectClick(connect)
{
	if (( document.adsl.adslConnectionMode.selectedIndex == 2 ) ||
		( document.adsl.adslConnectionMode.selectedIndex == 3 )) {
		
		if ( document.adsl.pppConnectType.selectedIndex == 2 && pppConnectStatus==connect) {
			if (document.adsl.pppUserName.value=="") {
				alert('PPP user name cannot be empty!');
				document.adsl.pppUserName.focus();
				return false;
			}
			
			if (document.adsl.pppPassword.value=="") {
				alert('PPP password cannot be empty!');
				document.adsl.pppPassword.focus();
				return false;
			}
		}
		
		return true;
	}
	
	return false;
}

function getDigit(str, num)
{
	i=1;
	if ( num != 1 ) {
		while (i!=num && str.length!=0) {
			if ( str.charAt(0) == '.' ) {
				i++;
			}
			str = str.substring(1);
		}
		if ( i!=num )
			return -1;
	}
	for (i=0; i<str.length; i++) {
		if ( str.charAt(i) == '.' ) {
			str = str.substring(0, i);
			break;
		}
	}
	if ( str.length == 0)
		return -1;
	d = parseInt(str, 10);
	return d;
}

function addClick()
{
	<% checkWrite("checkVC");%>
	return vcCheck();
}

function vcCheck()
{
	digit = getDigit(document.adsl.vpi.value, 1);
	if ( validateKey(document.adsl.vpi.value) == 0 ||
		(digit > 255 || digit < 0) ) {
		alert("Invalid VPI value! You should set a value between 0-255.");
		document.adsl.vpi.focus();
		return false;
	}

	digit = getDigit(document.adsl.vci.value, 1);
	if ( validateKey(document.adsl.vci.value) == 0 ||
		(digit > 65535 || digit < 0) ) {
		alert("Invalid VCI value! You should set a value between 0-65535.");
		document.adsl.vci.focus();
		return false;
	}
	
	if (( document.adsl.adslConnectionMode.selectedIndex == 2 ) ||
		( document.adsl.adslConnectionMode.selectedIndex == 3 )) {
		if (document.adsl.pppUserName.value=="") {
			alert('PPP user name cannot be empty!');
			document.adsl.pppUserName.focus();
			return false;
		}
		
		if (document.adsl.pppPassword.value=="") {
			alert('PPP password cannot be empty!');
			document.adsl.pppPassword.focus();
			return false;
		}
	}
	
	if (( document.adsl.adslConnectionMode.selectedIndex == 1 ) ||
		( document.adsl.adslConnectionMode.selectedIndex == 4 )) {
		if (document.adsl.ipMode[0].checked) {
			if (document.adsl.ip.value=="") {
				alert("IP address cannot be empty!");
				document.adsl.ip.focus();
				return false;
			}
			
			if ( validateKey( document.adsl.ip.value ) == 0 ) {
				alert("Invalid IP address value.");
				document.adsl.ip.focus();
				return false;
			}
			if ( !checkDigitRange(document.adsl.ip.value,1,0,255) ) {
				alert('Invalid local IP address range in 1st digit. It should be 0-255.');
				document.adsl.ip.focus();
				return false;
			}
			if ( !checkDigitRange(document.adsl.ip.value,2,0,255) ) {
				alert('Invalid local IP address range in 2nd digit. It should be 0-255.');
				document.adsl.ip.focus();
				return false;
			}
			if ( !checkDigitRange(document.adsl.ip.value,3,0,255) ) {
				alert('Invalid local IP address range in 3rd digit. It should be 0-255.');
				document.adsl.ip.focus();
				return false;
			}
			if ( !checkDigitRange(document.adsl.ip.value,4,1,254) ) {
				alert('Invalid local IP address range in 4th digit. It should be 1-254.');
				document.adsl.ip.focus();
				return false;
			}
			
			if (document.adsl.remoteIp.value=="") {
				alert("Remote IP address cannot be empty!");
				document.adsl.remoteIp.focus();
				return false;
			}
			if ( validateKey( document.adsl.remoteIp.value ) == 0 ) {
				alert("Invalid remote IP address value.");
				document.adsl.remoteIp.focus();
				return false;
			}
			if ( !checkDigitRange(document.adsl.remoteIp.value,1,0,255) ) {
				alert('Invalid remote IP address range in 1st digit. It should be 0-255.');
				document.adsl.remoteIp.focus();
				return false;
			}
			if ( !checkDigitRange(document.adsl.remoteIp.value,2,0,255) ) {
				alert('Invalid remote IP address range in 2nd digit. It should be 0-255.');
				document.adsl.remoteIp.focus();
				return false;
			}
			if ( !checkDigitRange(document.adsl.remoteIp.value,3,0,255) ) {
				alert('Invalid remote IP address range in 3rd digit. It should be 0-255.');
				document.adsl.remoteIp.focus();
				return false;
			}
			if ( !checkDigitRange(document.adsl.remoteIp.value,4,1,254) ) {
				alert('Invalid remote IP address range in 4th digit. It should be 1-254.');
				document.adsl.remoteIp.focus();
				return false;
			}
		}
	}
	
	return true;
}

function setPPPConnected()
{
	pppConnectStatus = 1;
}

function resetClicked()
{
	if (( initConnectMode == 2 ) ||
		( initConnectMode == 3 )) {
		pppSettingsEnable();
		ipSettingsDisable();
	}
	else if ( initConnectMode == 0 ) {
		// bridge mode
		pppSettingsDisable();
		ipSettingsDisable();
	}
	else {
		pppSettingsDisable();
		ipSettingsEnable();
	}
}

function disableFixedIpInput()
{
	disableTextField(document.adsl.ip);
	disableTextField(document.adsl.remoteIp);
}

function enableFixedIpInput()
{
	enableTextField(document.adsl.ip);
	enableTextField(document.adsl.remoteIp);
}

function ipTypeSelection()
{
	if ( document.adsl.ipMode[0].checked ) {
		enableFixedIpInput();
	} else {
		disableFixedIpInput()
	}
}

function pppSettingsEnable()
{
	enableTextField(document.adsl.pppUserName);
	enableTextField(document.adsl.pppPassword);
	enableTextField(document.adsl.pppConnectType);
	pppTypeSelection();
}

function pppSettingsDisable()
{
	disableTextField(document.adsl.pppUserName);
	disableTextField(document.adsl.pppPassword);
	disableTextField(document.adsl.pppIdleTime);
	disableTextField(document.adsl.pppConnectType);
	
	disableButtonIB(document.adsl.pppConnect);
	disableButtonIB(document.adsl.pppDisconnect);
}

function ipSettingsEnable()
{
	document.adsl.ipMode[0].disabled = false;
	document.adsl.ipMode[1].disabled = false;
	ipTypeSelection();
}

function ipSettingsDisable()
{
	document.adsl.ipMode[0].disabled = true;
	document.adsl.ipMode[1].disabled = true;
	disableFixedIpInput();
}

function adslConnectionModeSelection()
{
	document.adsl.naptEnabled.disabled = false;
	if (( document.adsl.adslConnectionMode.selectedIndex == 1 ) ||
		( document.adsl.adslConnectionMode.selectedIndex == 2 ))
		// MER, PPPoE
		document.adsl.naptEnabled.checked = true;
	else
		document.adsl.naptEnabled.checked = false;

	if (( document.adsl.adslConnectionMode.selectedIndex == 2 ) ||
		( document.adsl.adslConnectionMode.selectedIndex == 3 )) {
		
		pppSettingsEnable();
		ipSettingsDisable();
	}
	else if ( document.adsl.adslConnectionMode.selectedIndex == 0 ) {
		// bridge mode
		document.adsl.naptEnabled.disabled = true;
		pppSettingsDisable();
		ipSettingsDisable();
	}
	else {
		pppSettingsDisable();
		ipSettingsEnable();
	}
}

function clearAll()
{
	document.adsl.vpi.value = 0;
	document.adsl.vci.value = "";
	document.adsl.adslEncap.value = 1;
	document.adsl.naptEnabled.checked = false;
	document.adsl.adslConnectionMode.value = 0;
	
	document.adsl.pppUserName.value = "";
	document.adsl.pppPassword.value = "";
	document.adsl.pppConnectType.value = 0;
	document.adsl.pppIdleTime.value = "";
	
	document.adsl.ipMode.value = 0;
	document.adsl.ip.value = "";
	document.adsl.remoteIp.value = "";
}

function postVC(vpi,vci,encap,napt,mode,username,passwd,pppType,idletime,ipunnum,ipmode,ipaddr,remoteip,netmask,droute,status,enable)
{
	clearAll();
	document.adsl.vpi.value = vpi;
	document.adsl.vci.value = vci;
	if (encap == "LLC")
		document.adsl.adslEncap[0].checked = true;
	else
		document.adsl.adslEncap[1].checked = true;
	
	if (mode == "br1483")
		document.adsl.adslConnectionMode.value = 0;
	else if (mode == "mer1483")
		document.adsl.adslConnectionMode.value = 1;
	else if (mode == "PPPoE")
		document.adsl.adslConnectionMode.value = 2;
	else if (mode == "PPPoA")
		document.adsl.adslConnectionMode.value = 3;
	else if (mode == "rt1483")
		document.adsl.adslConnectionMode.value = 4;
	
	adslConnectionModeSelection();

	if (napt == "On")
		document.adsl.naptEnabled.checked = true;
	else
		document.adsl.naptEnabled.checked = false;
	
	if (enable == 0)
		document.adsl.chEnable[1].checked = true;
	else
		document.adsl.chEnable[0].checked = true;
	
	if (mode == "PPPoE" || mode == "PPPoA")
	{
		document.adsl.pppUserName.value = username;
		document.adsl.pppPassword.value = passwd;
		
		if (pppType == "conti")
			document.adsl.pppConnectType.value = 0;
		else if (pppType == "demand")
			document.adsl.pppConnectType.value = 1;
		else
		{
			document.adsl.pppConnectType.value = 2;
			if (status == 0) // disabled
			{
				disableButtonIB(document.adsl.pppConnect);
				disableButtonIB(document.adsl.pppDisconnect);
			}
			if (status == 1) // not exists
			{
				enableButtonIB(document.adsl.pppConnect);
				disableButtonIB(document.adsl.pppDisconnect);
			}
			else	// down or up
			{
				enableButtonIB(document.adsl.pppDisconnect);
				disableButtonIB(document.adsl.pppConnect);
			}
		}
		
		pppTypeSelection();
		
		if (pppType == "demand")
			document.adsl.pppIdleTime.value = idletime;
	}
	else if (mode == "mer1483" || mode == "rt1483")
	{
		document.adsl.ipMode[ipmode].checked = true;
		ipTypeSelection();
		if (ipmode == 0)
		{
			document.adsl.ip.value=ipaddr;
			document.adsl.remoteIp.value=remoteip;
			document.adsl.netmask.value=netmask;
		}

		if (mode == "rt1483")
		{
			if (ipunnum == 1)
				document.adsl.ipUnnum.value="ON";
			else
				document.adsl.ipUnnum.value="";
		}
	}

		
	if (droute == 1)
		document.adsl.droute.value = "1";
	else
		document.adsl.droute.value = "0";	
}


</script>

</head>
<BODY>
<blockquote>
<h2><font color="#0000FF">Channel Configuration</font></h2>
<form action=/goform/admin/formWanAdsl method=POST name="adsl">
<table border="0" cellspacing="4" width="500">
 <tr><font size=2>
    This page is used to configure the parameters for the channel operation modes of your ADSL
    Modem/Router.
  </tr>
  <tr><hr size=1 noshade align=top></tr>
  <tr>
</table>
<table border=0 width="500" cellspacing=4 cellpadding=0>

	<tr><td>
		<font size=2><b>VPI: </b><input type="text"
			name="vpi" size="4" maxlength="3" value=0>&nbsp;&nbsp;
		<b>VCI: </b><input type="text"
			name="vci" size="6" maxlength="5">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		<b>Encapsulation: </b>
			<input type="radio" value="1" name="adslEncap" checked>LLC&nbsp;&nbsp;
			<input type="radio" value="0" name="adslEncap">VC-Mux
	</td></tr>
	<tr><td>
		<font size=2><b>Channel Mode:</b>
		<select size="1" name="adslConnectionMode" onChange="adslConnectionModeSelection()">
			  <option selected value="0">1483 Bridged</option>
			  <option value="1">1483 MER</option>
			  <option value="2">PPPoE</option>
			  <option value="3">PPPoA</option>
			  <option value="4">1483 Routed</option>
		</select>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		<b>Enable NAPT: </b><input type="checkbox" name="naptEnabled"
			size="2" maxlength="2" value="ON">
		<b><input type="radio" value=1 name="chEnable" checked>Enable&nbsp;&nbsp;
		   <input type="radio" value=0 name="chEnable">Disable</b>
	</td></tr>
</table>

<table border=0 width="500" cellspacing=4 cellpadding=0>
<tr><td colspan=5><hr size=2 align=top></td></tr>
<tr><th align="left"><font size=2><b>PPP Settings:</b></th>
	<td><font size=2><b>User Name:</b></td>
	<td><font size=2><input type="text" name="pppUserName" size="10" maxlength="30"></td>
	<td><font size=2><b>Password:</b></td>
	<td><font size=2><input type="password" name="pppPassword" size="10" maxlength="30"></td>
</tr>
<tr><th></th>
	<td><font size=2><b>Type:</b></td>
	<td><font size=2><select size="1" name="pppConnectType" onChange="pppTypeSelection()">
		<option selected value="0">Continuous</option>
		<option value="1">Connect on Demand</option>
		<option value="2">Manual</option>
		</select>
	</td>
	<td><font size=2><b>Idle Time (min):</b></td>
	<td><font size=2><input type="text" name="pppIdleTime" size="10" maxlength="3"></td>
	<% checkWrite("pppoeStatus"); %>
</tr>
<tr><td colspan=5><hr size=2 align=top></td></tr>
<tr><th align="left"><font size=2><b>WAN IP Settings:</b></th>

	<td><font size=2><b>Type:</b></td>
	<td>
	<input type="radio" value="0" name="ipMode" checked onClick="ipTypeSelection()">Fixed IP
	</td>
	<td>
	<input type="radio" value="1" name="ipMode" onClick="ipTypeSelection()">DHCP
	</td>
</tr>
<tr><th></th>
	<td><font size=2><b>Local IP Address:</b></td>
	<td><font size=2><input type="text" name="ip" size="10" maxlength="15"></td>
	<td><font size=2><b>Remote IP Address:</b></td>
	<td><font size=2><input type="text" name="remoteIp" size="10" maxlength="15"></td>
</tr>
</table>


<input type="hidden" name="ipUnnum">
<input type="hidden" name="netmask">
<input type="hidden" name="droute">


<BR>
<input type="hidden" value="/admin/wanadsl_user.asp" name="submit-url">
<p><input type="submit" value="Connect" name="pppConnect" onClick="return pppConnectClick(0)">
<input type="submit" value="Disconnect" name="pppDisconnect" onClick="return pppConnectClick(1)">
<!--
<input type="submit" value="Add" name="add" onClick="return addClick()">
-->
<input type="submit" value="Modify" name="modify" onClick="return vcCheck()">
<!--
<input type="submit" value="Delete" name="delvc">
<input type="reset" value="Undo" name="reset" onClick="resetClicked()">
-->
<input type="submit" value="Refresh" name="refresh">


<BR>
<BR>

<table border="0" width=700>
	<tr><font size=2><b>Current ATM VC Table:</b></font></tr>
	<% atmVcList2(); %>
</table>

<script>
	initConnectMode = document.adsl.adslConnectionMode.selectedIndex;
	
	adslConnectionModeSelection();
	<% checkWrite("devType");
	   checkWrite("vcCount"); %>
</script>
</form>

<form action=/goform/admin/formWanAdsl method=POST name="actionForm">
<input type="hidden" value="/admin/wanadsl_user.asp" name="submit-url">
<input type="hidden" name="action">
<input type="hidden" name="idx">
</form>

</blockquote>
</body>
</html>
