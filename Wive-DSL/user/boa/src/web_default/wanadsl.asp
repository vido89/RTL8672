<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>ADSL Connection Mode</title>
<script type="text/javascript" src="share.js">
</script>
<script language="javascript">

var initConnectMode;
var pppConnectStatus=0;

var dgwstatus;
var gtwy;
var interfaceInfo = '';
var gtwyIfc ='';
var gwInterface=0;

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
//		disableButtonIB(document.adsl.pppConnect);
//		disableButtonIB(document.adsl.pppDisconnect);
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


/*
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
*/

function checkDefaultGW() {
	with (document.forms[0]) {
		if (droute[0].checked == false && droute[1].checked == false && gwStr[0].checked == false && gwStr[1].checked == false) {
			alert('A default gateway has to be selected.');
			return false;
		}
		if (droute[1].checked == true) {
			if (gwStr[0].checked == true) {
				if (isValidIpAddress(dstGtwy.value, "Default Gateway IP Address") == false)
					return false;
			}
		}
	}
}

function addClick()
{
	<% checkWrite("checkVC");%>
	return vcCheck();
}

// Jenny, check decimal
function validateInt(str)
{
	for (var i=0; i<str.length; i++) {
		if (str.charAt(i) == '.' )
			return 0;
	}
	return 1;
}

function vcCheck()
{
	var i;

	return checkDefaultGW();

	if (validateInt(document.adsl.vpi.value) == 0) {	// Jenny,  buglist B056, check if input VPI is a decimal
		alert("Invalid VPI value! VPI shouldn't be a decimal.");
		document.adsl.vpi.value = document.adsl.vpi.defaultValue;
		document.adsl.vpi.focus();
		return false;
	}
	digitVPI = getDigit(document.adsl.vpi.value, 1);
	if ( validateKey(document.adsl.vpi.value) == 0 ||
		(digitVPI > 255 || digitVPI < 0) ) {
		alert("Invalid VPI value! You should set a value between 0-255.");
		document.adsl.vpi.focus();
		return false;
	}

	if (validateInt(document.adsl.vci.value) == 0) {	// Jenny,  buglist B056, check if input VCI is a decimal
		alert("Invalid VCI value! VCI shouldn't be a decimal.");
		document.adsl.vci.value = document.adsl.vci.defaultValue;
		document.adsl.vci.focus();
		return false;
	}
	digitVCI = getDigit(document.adsl.vci.value, 1);
	if ( validateKey(document.adsl.vci.value) == 0 ||
		(digitVCI > 65535 || digitVCI < 0) ) {
		alert("Invalid VCI value! You should set a value between 0-65535.");
		document.adsl.vci.focus();
		return false;
	}

	if ( (digitVPI == 0 && digitVCI == 0) ) {
		alert("Invalid VPI/VCI value!");
		document.adsl.vpi.focus();
		return false;
	}

	if (( document.adsl.adslConnectionMode.selectedIndex == 2 ) ||
		( document.adsl.adslConnectionMode.selectedIndex == 3 )) {
		if (document.adsl.pppUserName.value=="") {
			alert('PPP user name cannot be empty!');
			document.adsl.pppUserName.focus();
			return false;
		}
		if (includeSpace(document.adsl.pppUserName.value)) {
			alert('Cannot accept space character in PPP user name.');
			document.adsl.pppUserName.focus();
			return false;
		}
		if (checkString(document.adsl.pppUserName.value) == 0) {
			alert('Invalid PPP user name.');
			document.adsl.pppUserName.focus();
			return false;
		}

		if (document.adsl.pppPassword.value=="") {
			alert('PPP password cannot be empty!');
			document.adsl.pppPassword.focus();
			return false;
		}
		if (includeSpace(document.adsl.pppPassword.value)) {
			alert('Cannot accept space character in PPP password.');
			document.adsl.pppPassword.focus();
			return false;
		}
		if (checkString(document.adsl.pppPassword.value) == 0) {
			alert('Invalid PPP password.');
			document.adsl.pppPassword.focus();
			return false;
		}
		if (document.adsl.pppConnectType.selectedIndex == 1) {
			if (document.adsl.pppIdleTime.value <= 0) {
				alert('Invalid PPP idle time.');
				document.adsl.pppIdleTime.focus();
				return false;
			}
		}
	}
	
	if (( document.adsl.adslConnectionMode.selectedIndex == 1 ) ||
		( document.adsl.adslConnectionMode.selectedIndex == 4 )) {
		if (document.adsl.ipMode[0].checked)
			if ( document.adsl.ipUnnum.disabled || ( !document.adsl.ipUnnum.disabled && !document.adsl.ipUnnum.checked )) {
				if (!checkHostIP(document.adsl.ip, 1))
					return false;
				if (document.adsl.remoteIp.visiblity == "hidden") {
					if (!checkHostIP(document.adsl.remoteIp, 1))
					return false;
				}
				if (document.adsl.adslConnectionMode.selectedIndex == 1 && !checkNetmask(document.adsl.netmask, 1))
					return false;
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
	document.adsl.adslEncap[0].disabled = false;
	document.adsl.adslEncap[1].disabled = false;
	if (( initConnectMode == 2 ) ||
		( initConnectMode == 3 )) {
		pppSettingsEnable();
		ipSettingsDisable();
	}
	else if ( initConnectMode == 0 ) {
		// bridge mode
		document.adsl.naptEnabled.disabled = true;
		pppSettingsDisable();
		ipSettingsDisable();
	}
	else {
		document.adsl.naptEnabled.disabled = false;
		pppSettingsDisable();
		ipSettingsEnable();
	}
	if (initConnectMode == 4)
		// Route1483
		document.adsl.ipUnnum.disabled = false;
	else
		document.adsl.ipUnnum.disabled = true;
}

function disableFixedIpInput()
{
	disableTextField(document.adsl.ip);
	disableTextField(document.adsl.remoteIp);
	disableTextField(document.adsl.netmask);
}

function enableFixedIpInput()
{
	enableTextField(document.adsl.ip);
	enableTextField(document.adsl.remoteIp);
	if (document.adsl.adslConnectionMode.value == 4)
		disableTextField(document.adsl.netmask);
	else
		enableTextField(document.adsl.netmask);
}

function ipTypeSelection()
{
	if ( document.adsl.ipMode[0].checked ) {
		enableFixedIpInput();
	} else {
		disableFixedIpInput();
	}
}

function pppSettingsEnable()
{
	enableTextField(document.adsl.pppUserName);
	enableTextField(document.adsl.pppPassword);
	enableTextField(document.adsl.pppConnectType);
	document.adsl.droute[0].disabled = false;
	document.adsl.droute[1].disabled = false;
	document.adsl.gwStr[0].disabled = false;
	document.adsl.gwStr[1].disabled = false;
	enableTextField(document.adsl.dstGtwy);
	document.adsl.wanIf.disabled = false;
	pppTypeSelection();
	autoDGWclicked();
}

function pppSettingsDisable()
{
	disableTextField(document.adsl.pppUserName);
	disableTextField(document.adsl.pppPassword);
	disableTextField(document.adsl.pppIdleTime);
	disableTextField(document.adsl.pppConnectType);
	
//	disableButtonIB(document.adsl.pppConnect);
//	disableButtonIB(document.adsl.pppDisconnect);
	document.adsl.droute[0].disabled = true;
	document.adsl.droute[1].disabled = true;
	document.adsl.gwStr[0].disabled = true;
	document.adsl.gwStr[1].disabled = true;
	disableTextField(document.adsl.dstGtwy);
	document.adsl.wanIf.disabled = true;
}

function ipSettingsEnable()
{
	if ( document.adsl.adslConnectionMode.selectedIndex == 4 ) {
		document.adsl.ipMode[0].checked = true;
		if (document.adsl.naptEnabled.checked)
			document.adsl.ipUnnum.disabled = true;
		else
			document.adsl.ipUnnum.disabled = false;
		document.adsl.ipMode[0].disabled = true;
		document.adsl.ipMode[1].disabled = true;
	}
	else {
		document.adsl.ipMode[0].disabled = false;
		document.adsl.ipMode[1].disabled = false;
	}
	document.adsl.droute[0].disabled = false;
	document.adsl.droute[1].disabled = false;
	document.adsl.gwStr[0].disabled = false;
	document.adsl.gwStr[1].disabled = false;
	enableTextField(document.adsl.dstGtwy);
	document.adsl.wanIf.disabled = false;
	ipTypeSelection();
	autoDGWclicked();
}

function ipSettingsDisable()
{
	document.adsl.ipMode[0].disabled = true;
	document.adsl.ipMode[1].disabled = true;
	document.adsl.droute[0].disabled = true;
	document.adsl.droute[1].disabled = true;
	document.adsl.gwStr[0].disabled = true;
	document.adsl.gwStr[1].disabled = true;
	disableTextField(document.adsl.dstGtwy);
	document.adsl.wanIf.disabled = true;
	disableFixedIpInput();
}

function ipModeSelection()
{
	if (document.adsl.ipUnnum.checked) {
		pppSettingsDisable();
		ipSettingsDisable();
		document.adsl.droute[0].disabled = false;
		document.adsl.droute[1].disabled = false;
		document.adsl.gwStr[0].disabled = false;
		document.adsl.gwStr[1].disabled = false;
		enableTextField(document.adsl.dstGtwy);
		document.adsl.wanIf.disabled = false;
	}
	else
		ipSettingsEnable();
}

function adslConnectionModeSelection()
{
	document.adsl.naptEnabled.disabled = false;
	document.adsl.ipUnnum.disabled = true;
	document.adsl.adslEncap[0].disabled = false;
	document.adsl.adslEncap[1].disabled = false;
/*	if (( document.adsl.adslConnectionMode.selectedIndex == 1 ) ||
		( document.adsl.adslConnectionMode.selectedIndex == 2 ))
		// MER, PPPoE
		document.adsl.naptEnabled.checked = true;
	else
		document.adsl.naptEnabled.checked = false;*/
	<% checkWrite("naptEnable"); %>

	if (( document.adsl.adslConnectionMode.selectedIndex == 2 ) ||
		( document.adsl.adslConnectionMode.selectedIndex == 3 )) {
		
		ipSettingsDisable();
		pppSettingsEnable();
	}
	else if ( document.adsl.adslConnectionMode.selectedIndex == 0 ) {
		// bridge mode
		document.adsl.naptEnabled.disabled = true;
		pppSettingsDisable();
		ipSettingsDisable();
	}
	else if ( document.adsl.adslConnectionMode.selectedIndex == 4 ) {
		// Route1483
		document.adsl.ipMode[0].checked = true;
		document.adsl.ipUnnum.disabled = false;
		pppSettingsDisable();
		ipSettingsEnable();
		document.adsl.ipMode[0].disabled = true;
		document.adsl.ipMode[1].disabled = true;
		disableTextField(document.adsl.netmask);
	}
	else if ( document.adsl.adslConnectionMode.selectedIndex == 5 ) {
		// Route1577
		document.adsl.ipMode[0].checked = true;
		pppSettingsDisable();
		ipSettingsEnable();
		document.adsl.ipMode[0].disabled = true;
		document.adsl.ipMode[1].disabled = true;
		document.adsl.adslEncap[0].disabled = true;
		document.adsl.adslEncap[1].disabled = true;
		document.adsl.adslEncap[0].checked = true;
		document.adsl.adslEncap.value = 1;
	}
	else {
		pppSettingsDisable();
		ipSettingsEnable();
	}
}

function naptClicked()
{
	if (document.adsl.adslConnectionMode.selectedIndex == 4) {
		// Route1483
		if (document.adsl.naptEnabled.checked == true) {
			document.adsl.ipUnnum.checked = false;
			document.adsl.ipUnnum.disabled = true;
		}
		else
			document.adsl.ipUnnum.disabled = false;
		ipModeSelection();
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
	
	document.adsl.ipUnnum.checked = false;
	document.adsl.ipMode.value = 0;
	document.adsl.ip.value = "";
	document.adsl.remoteIp.value = "";
	document.adsl.netmask.value = "";
	document.adsl.adslEncap[0].disabled = false;
	document.adsl.adslEncap[1].disabled = false;
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
	else if (mode == "rt1577")
		document.adsl.adslConnectionMode.value = 5;
	
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
			
/*
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
*/
		
		}
		
		pppTypeSelection();
		
		if (pppType == "demand")
			document.adsl.pppIdleTime.value = idletime;
	}
	else if (mode == "mer1483" || mode == "rt1483" || mode == "rt1577")
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
				document.adsl.ipUnnum.checked = true;
			else
				document.adsl.ipUnnum.checked = false;
			ipModeSelection();
			disableTextField(document.adsl.netmask);
		}
	}
	
<%checkWrite("pppoeProxyEnable");%>
	
	<%checkWrite("dgw");%>

}


function updatepvcState()
{
  if (document.adsl.autopvc.checked == true) {
  	document.adsl.autopvc.value="ON";
	document.adsl.enablepvc.value = 1;  	
	enableTextField(document.adsl.autopvcvci);
	enableTextField(document.adsl.autopvcvpi);
	enableButton(document.adsl.autopvcadd);		
  } else {
  	document.adsl.autopvc.value="OFF";
	document.adsl.enablepvc.value = 0;  	
	disableTextField(document.adsl.autopvcvci);
	disableTextField(document.adsl.autopvcvpi);
	disableButton(document.adsl.autopvcadd);	
  }
}

function updatepvcState2()
{
  if (document.adsl.autopvc.checked == true) {
  	document.adsl.autopvc.value="ON";
	document.adsl.enablepvc.value = 1;  	
	//enableTextField(document.adsl.autopvcvci);
	//enableTextField(document.adsl.autopvcvpi);
	//enableButton(document.adsl.autopvcadd);		
  } else {
  	document.adsl.autopvc.value="OFF";
	document.adsl.enablepvc.value = 0;  	
	//disableTextField(document.adsl.autopvcvci);
	//disableTextField(document.adsl.autopvcvpi);
	//disableButton(document.adsl.autopvcadd);	
  }
}

function autopvcCheckClick()
{
	var dVPI,dVCI;
	if (document.adsl.autopvc.checked == true) {

		document.adsl.enablepvc.value = 1;  	

		dVPI = getDigit(document.adsl.autopvcvpi.value, 1);
		if ( validateKey(document.adsl.autopvcvpi.value) == 0 ||
			(dVPI > 255 || dVPI < 0) ) {
			alert("Invalid VPI value! You should set a value between 0-255.");
			document.adsl.autopvcvpi.focus();
			return false;
		}

		dVCI = getDigit(document.adsl.autopvcvci.value, 1);
		if ( validateKey(document.adsl.autopvcvci.value) == 0 ||
			(dVCI > 65535 || dVCI < 0) ) {
			alert("Invalid VCI value! You should set a value between 0-65535.");
			document.adsl.autopvcvci.focus();
			return false;
		}
	
		if ( (dVPI == 0 && dVCI == 0) ) {
			alert("Invalid VPI/VCI value!");
			document.adsl.autopvcvpi.focus();
			return false;
		}

		document.adsl.addVPI.value = dVPI;
		document.adsl.addVCI.value = dVCI;

	}else {
		alert(" You should enable Auto-PVC search first.");	
		return false;
	}
}

function hideGWInfo(hide) {
	var status = false;

	if (hide == 1)
		status = true;

	changeBlockState('gwInfo', status);

	if (hide == 0) {
		with (document.forms[0]) {
			if (dgwstatus == 255) {
				if (isValidIpAddress(gtwy) == true) {
					gwStr[0].checked = true;
					gwStr[1].checked = false;
					dstGtwy.value=gtwy;
					wanIf.disabled=true
				} else {
					gwStr[0].checked = false;
					gwStr[1].checked = true;
					dstGtwy.value = '';
				}
			}
			else if (dgwstatus != 239) {
					gwStr[1].checked = true;
					gwStr[0].checked = false;
					wanIf.disabled=false;
					wanIf.value=dgwstatus;
					dstGtwy.disabled=true;
			} else {
					gwStr[1].checked = false;
					gwStr[0].checked = true;
					wanIf.disabled=true;
					dstGtwy.disabled=false;
			}
		}
	}
}

function autoDGWclicked() {
	if (document.adsl.droute[0].checked == true) {
		hideGWInfo(1);
	} else {
		hideGWInfo(0);
	}
}

function gwStrClick() {
	with (document.forms[0]) {
		if (gwStr[1].checked == true) {
			dstGtwy.disabled = true;
			wanIf.disabled = false;
		}
		else {
			dstGtwy.disabled = false;
			wanIf.disabled = true;
		}
      	}
}
</script>

</head>
<BODY>
<blockquote>
<h2><font color="#0000FF">WAN Configuration</font></h2>
<form action=/goform/formWanAdsl method=POST name="adsl">
<table border="0" cellspacing="4" width="700">
 <tr><font size=2>
    This page is used to configure the parameters for the channel operation modes of your ADSL
    Modem/Router.
  </tr>
  <tr><hr size=1 noshade align=top></tr>
</table>
<table border=0 width="800" cellspacing=4 cellpadding=0>
	<tr>
		<td>
			<font size=2><b>VPI: </b>
			<input type="text" name="vpi" size="4" maxlength="3" value=0>&nbsp;&nbsp;
			<b>VCI: </b>
			<input type="text" name="vci" size="6" maxlength="5"></font></td>
		<td><font size=2><b>Encapsulation: </b>
			<input type="radio" value="1" name="adslEncap" checked>LLC&nbsp;&nbsp;
			<input type="radio" value="0" name="adslEncap">VC-Mux</font></td>
		<td>
		<% ShowChannelMode("adslcmode"); %>
		<% ShowApplicationMode(); %>		
		</td>
	</tr>
	<tr>
		<td>
		<% ShowNAPTSetting(); %>
		</td>
		<td>
			<font size=2><b>Admin Status:</b>	
			<input type="radio" value=1 name="chEnable" checked>Enable&nbsp;&nbsp;
			<input type="radio" value=0 name="chEnable">Disable</td>
	</tr>
</table>
<% ShowPPPIPSettings("pppoeStatus") %>
<% ShowDefaultGateway("p2p"); %>



<BR>
<input type="hidden" value="/wanadsl.asp" name="submit-url">
<p><!--input type="submit" value="Connect" name="pppConnect" onClick="return pppConnectClick(0)">
<input type="submit" value="Disconnect" name="pppDisconnect" onClick="return pppConnectClick(1)"-->
<input type="submit" value="Add" name="add" onClick="return addClick()">
<input type="submit" value="Modify" name="modify" onClick="return vcCheck()">
<!--input type="reset" value="Undo" name="reset" onClick="resetClicked()">
<input type="submit" value="Refresh" name="refresh"-->
<BR>
<BR>

<table border="0" width=700>
	<tr><font size=2><b>Current ATM VC Table:</b></font></tr>
	<% atmVcList2(); %>

</table>
<br>
<input type="submit" value="Delete Selected" name="delvc" onClick="return deleteClick()">

<% ShowAutoPVC(); %>

<script>
	initConnectMode = document.adsl.adslConnectionMode.selectedIndex;
	
	<% initPage("dgw"); %>
	<% GetDefaultGateway(); %>
	autoDGWclicked();
	adslConnectionModeSelection();
	<% checkWrite("devType");
	   checkWrite("vcCount"); %>
</script>
</form>

<form action=/goform/formWanAdsl method=POST name="actionForm">
<input type="hidden" value="/wanadsl.asp" name="submit-url">
<input type="hidden" name="action">
<input type="hidden" name="idx">

</form>

</blockquote>
</body>
</html>
