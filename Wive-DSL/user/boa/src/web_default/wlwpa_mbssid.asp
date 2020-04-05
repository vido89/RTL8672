<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Wireless Security Setup</title>
<script type="text/javascript" src="share.js">
</script>
<script>
var defPskLen, defPskFormat;

function disableRadioGroup (radioArrOrButton)
{
  if (radioArrOrButton.type && radioArrOrButton.type == "radio") {
 	var radioButton = radioArrOrButton;
 	var radioArray = radioButton.form[radioButton.name];
  }
  else
 	var radioArray = radioArrOrButton;
 	radioArray.disabled = true;
 	for (var b = 0; b < radioArray.length; b++) {
 	if (radioArray[b].checked) {
 		radioArray.checkedElement = radioArray[b];
 		break;
	}
  }
  for (var b = 0; b < radioArray.length; b++) {
 	radioArray[b].disabled = true;
 	radioArray[b].checkedElement = radioArray.checkedElement;
  }
}

function enableRadioGroup (radioArrOrButton)
{
  if (radioArrOrButton.type && radioArrOrButton.type == "radio") {
 	var radioButton = radioArrOrButton;
 	var radioArray = radioButton.form[radioButton.name];
  }
  else
 	var radioArray = radioArrOrButton;

  radioArray.disabled = false;
  radioArray.checkedElement = null;
  for (var b = 0; b < radioArray.length; b++) {
 	radioArray[b].disabled = false;
 	radioArray[b].checkedElement = null;
  }
}

function skip () { this.blur(); }
function preserve () { this.checked = this.storeChecked; }
function disableCheckBox (checkBox) {
  if (!checkBox.disabled) {
    checkBox.disabled = true;
    if (!document.all && !document.getElementById) {
      checkBox.storeChecked = checkBox.checked;
      checkBox.oldOnClick = checkBox.onclick;
      checkBox.onclick = preserve;
    }
  }
}

function enableCheckBox (checkBox)
{
  if (checkBox.disabled) {
    checkBox.disabled = false;
    if (!document.all && !document.getElementById)
      checkBox.onclick = checkBox.oldOnClick;
  }
}

function check_wepbutton_state()
{
  if (document.formEncrypt.method.selectedIndex==1)
	enableButton(document.formEncrypt.wepKey);
  else
 	disableButton(document.formEncrypt.wepKey);
}

function check_radius_state()
{
  form = document.formEncrypt ;
  use1x = form.use1x;
  wpaAuth = form.wpaAuth;
  if ( (form.method.selectedIndex<2 && use1x.checked) ) {
	enableTextField(form.radiusPort);
  	enableTextField(form.radiusIP);
  	enableTextField(form.radiusPass);
  }
  else if ( form.method.selectedIndex<2 ){ 
  	disableTextField(form.radiusPort);
  	disableTextField(form.radiusIP);
  	disableTextField(form.radiusPass);
  }
  
  if ( (form.method.selectedIndex>=2 && wpaAuth[0].checked) ) {
	enableTextField(form.radiusPort);
  	enableTextField(form.radiusIP);
  	enableTextField(form.radiusPass);
  	
  	disableTextField(document.formEncrypt.pskFormat);
  	disableTextField(document.formEncrypt.pskValue);
  }
  else if ( (form.method.selectedIndex>=2 && wpaAuth[1].checked) ){
  	disableTextField(form.radiusPort);
  	disableTextField(form.radiusIP);
  	disableTextField(form.radiusPass);
  	
  	enableTextField(document.formEncrypt.pskFormat);
  	enableTextField(document.formEncrypt.pskValue);
  }
}


function check_nonWpaSupp_state()
{
  check_radius_state();
  check_wepbutton_state();
  check_wepKeyLen_state();
}

function check_wepKeyLen_state()
{
  form = document.formEncrypt ;
  use1x = form.use1x;
  if ((form.method.selectedIndex==1 && use1x.checked) )
  	enableRadioGroup(form.wepKeyLen);
  else
  	disableRadioGroup(form.wepKeyLen);
}

function disable_wpa()
{
  disableTextField(document.formEncrypt.pskFormat);
  disableTextField(document.formEncrypt.pskValue);

  //if(document.formEncrypt.elements.use1x.disabled != true)
  	enableCheckBox(document.formEncrypt.elements.use1x);
  disableRadioGroup(document.formEncrypt.elements.wpaAuth);
  check_nonWpaSupp_state();
}

function enable_wpa()
{
  enableTextField(document.formEncrypt.pskFormat);
  enableTextField(document.formEncrypt.pskValue);  
  
  disableCheckBox(document.formEncrypt.elements.use1x);
 // if(document.formEncrypt.elements.use1x.disabled != true)
	enableRadioGroup(document.formEncrypt.elements.wpaAuth);  	
  check_nonWpaSupp_state();
}

function checkState()
{
  if (document.formEncrypt.wlanDisabled.value == "ON") {
    disableTextField(document.formEncrypt.method);
    disableButton(document.formEncrypt.wepKey);
    disableTextField(document.formEncrypt.use1x);
    disableRadioGroup(document.formEncrypt.wepKeyLen);
    disableRadioGroup(document.formEncrypt.elements.wpaAuth);
    disableTextField(document.formEncrypt.pskFormat);
    disableTextField(document.formEncrypt.pskValue);
    disableTextField(document.formEncrypt.radiusPort);
    disableTextField(document.formEncrypt.radiusIP);
    disableTextField(document.formEncrypt.radiusPass);
    disableButton(document.formEncrypt.save);
  } else {
    if (document.formEncrypt.method.selectedIndex>=2)
  	  enable_wpa();
    else
  	  disable_wpa();
 };
}

function openWindow(url, windowName)
{
	var wide=620;
	var high=420;
	if (document.all)
		var xMax = screen.width, yMax = screen.height;
	else if (document.layers)
		var xMax = window.outerWidth, yMax = window.outerHeight;
	else
	   var xMax = 640, yMax=500;
	var xOffset = (xMax - wide)/2;
	var yOffset = (yMax - high)/3;

	var settings = 'width='+wide+',height='+high+',screenX='+xOffset+',screenY='+yOffset+',top='+yOffset+',left='+xOffset+', resizable=yes, toolbar=no,location=no,directories=no,status=no,menubar=no,scrollbars=yes';

	window.open( url, windowName, settings );
}


function wepKeyClick(url)
{
  if (document.formEncrypt.method.selectedIndex==1) {
	openWindow(url, 'SetWEPKey' );
  }
}

function saveChanges()
{
  if (document.formEncrypt.method.selectedIndex>=2) {
	var str = document.formEncrypt.pskValue.value;
	if (document.formEncrypt.pskFormat.selectedIndex==1) {
		if (str.length != 64) {
			alert('Pre-Shared Key value should be 64 characters.');
			document.formEncrypt.pskValue.focus();
			return false;
		}
		takedef = 0;
		if (defPskFormat == 1 && defPskLen == 64) {
			for (var i=0; i<64; i++) {
    				if ( str.charAt(i) != '*')
					break;
			}
			if (i == 64 )
				takedef = 1;
  		}
		if (takedef == 0) {
			for (var i=0; i<str.length; i++) {
    				if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
					(str.charAt(i) >= 'a' && str.charAt(i) <= 'f') ||
					(str.charAt(i) >= 'A' && str.charAt(i) <= 'F') )
					continue;
				alert("Invalid Pre-Shared Key value. It should be in hex number (0-9 or a-f).");
				document.formEncrypt.pskValue.focus();
				return false;
  			}
		}
	}
	else {
		if ( (document.formEncrypt.method.selectedIndex>=2 && wpaAuth[1].checked) ) {
		if (str.length < 8) {
			alert('Pre-Shared Key value should be set at least 8 characters.');
			document.formEncrypt.pskValue.focus();
			return false;
		}
		if (str.length > 64) {
			alert('Pre-Shared Key value should be less than 64 characters.');
			document.formEncrypt.pskValue.focus();
			return false;
		}
		}
	}
  }

   return true;
}


function postSecurity(encrypt, enable1X, wep, wpaAuth, wpaPSKFormat, wpaPSK, rsPort, rsIpAddr, rsPassword) 
{	
	document.formEncrypt.method.value = encrypt;
	document.formEncrypt.pskFormat.value = wpaPSKFormat;
	document.formEncrypt.pskValue.value = wpaPSK;				
	document.formEncrypt.radiusIP.value = rsIpAddr;
	document.formEncrypt.radiusPort.value = rsPort;
	document.formEncrypt.radiusPass.value = rsPassword;
		
	if ( wep != 0 )
		document.formEncrypt.wepKeyLen[wep-1].checked = true;
	
	if (enable1X==1)
		document.formEncrypt.use1x.checked = true;		
	document.formEncrypt.wpaAuth[wpaAuth-1].checked = true;	
	
	checkState();
        defPskLen = document.formEncrypt.pskValue.value.length;
	defPskFormat = document.formEncrypt.pskFormat.selectedIndex;	
}

</script>

</head>

<body>
<blockquote>
<h2><font color="#0000FF">Wireless Security Setup</font></h2>

<form action=/goform/admin/formWlEncrypt method=POST name="formEncrypt">
<table border=0 width="540" cellspacing=4 cellpadding=0>
    <tr><font size=2>
    This page allows you setup the wireless security. Turn on WEP or WPA by using
    Encryption Keys could prevent any unauthorized access to your wireless network.
    </tr>
    <tr><hr size=1 noshade align=top></tr>
	<input type=hidden name="wlanDisabled" value=<% wlanStatus %>>        
    
    <tr>
      <td width="35%"><font size="2"><b>SSID TYPE:</b></font></td>
	<td width="65%"><font size="2">
	  <input type="radio" name="wpaSSID" value="root"  <% postSSID("root"); %>>Root&nbsp;
	  <input type="radio" name="wpaSSID" value="vap0"  <% postSSID("vap0"); %>>VAP0&nbsp;
	  <input type="radio" name="wpaSSID" value="vap1"  <% postSSID("vap1"); %>>VAP1&nbsp;
	  <input type="radio" name="wpaSSID" value="vap2"  <% postSSID("vap2"); %>>VAP2&nbsp;
	  <input type="radio" name="wpaSSID" value="vap3"  <% postSSID("vap3"); %>>VAP3
	</font></td>
    </tr>    
    
    <tr>
      <td width="35%"><font size="2"><b>Encryption:&nbsp;</b>
        <select size="1" name="method" onChange="checkState()">
	  	<% checkWrite("wpaEncrypt"); %> 
<!--          <option value=0>None</option>
          <option value=1>WEP</option>
          <option value=2>WPA (TKIP)</option>
          <option value=3>WPA (AES)</option>
          <option value=4>WPA2(AES)</option>
          <option value=5>WPA2(TKIP)</option>
          <option value=6>WPA2 Mixed</option>
-->
        </select></font></td>
      <td width="65%"><font size=2><input  type="button" value="Set WEP Key" name="wepKey" onClick="wepKeyClick('/admin/wlwep_mbssid.asp')">
    </tr>
    <tr>
      <td width="35%" onClick="checkState()"><font size="2"><b><input type="checkbox" name="use1x" value="ON"
         >Use 802.1x Authentication</b></font></td>
      <td width="65%"><font size="2">
	  <input type="radio" name="wepKeyLen" value="wep64" >WEP 64bits&nbsp;
	  <input type="radio" name="wepKeyLen" value="wep128">WEP 128bits
      </font></td>
    </tr>
    <tr>
      <td width="35%"><font size="2"><b>WPA Authentication Mode:</b></font></td>
	<td width="65%"><font size="2">
	  <input type="radio" name="wpaAuth" value="eap"  onClick="checkState()">Enterprise (RADIUS)&nbsp;
	  <input type="radio" name="wpaAuth" value="psk"  onClick="checkState()">Personal (Pre-Shared Key)
	</font></td>
    </tr>
    <tr>
      <td width="35%"><font size="2"><b>Pre-Shared Key Format:</b></font> </td>
      <td width="65%"><font size="2"><select size="1" name="pskFormat">
          <option value=0 >Passphrase</option>
          <option value=1 >Hex (64 characters)</option>
        </select></font></td>
    </tr>
    <tr>
      <td width="35%"><font size="2"><b>Pre-Shared Key:</b></font> </td>
      <!--
      <td width="65%"><font size="2"><input type="text" name="pskValue" size="32" maxlength="64" value=<% getInfo("pskValue");%>></font></td>
      -->
      <td width="65%"><font size="2"><input type="text" name="pskValue" size="32" maxlength="64" ></font></td>
    </tr>
    <tr></tr><tr></tr><tr></tr>
    <tr>
      <td width="35%"><font size="2"><b>Authentication RADIUS Server:</b></font></td>
      <td width="65%"><font size="2">
        <!--
        Port <input type="text" name="radiusPort" size="4" value=<% getInfo("rsPort"); %>>&nbsp;&nbsp;        
	IP address <input type="text" name="radiusIP" size="10" value=<% getInfo("rsIp"); %>>&nbsp;&nbsp;	
        Password <input type="password" name="radiusPass" size="8" maxlength="64" value=<% getInfo("rsPassword"); %>></font></td>
        -->
        Port <input type="text" name="radiusPort" size="4" >&nbsp;&nbsp;        
	IP address <input type="text" name="radiusIP" size="10" >&nbsp;&nbsp;	
        Password <input type="password" name="radiusPass" size="8" maxlength="64" ></font></td>
        
    </tr>
    <tr><td colspan="2" width="100%" height="55"><font size=2><em>Note: When encryption WEP is selected, you must set WEP key value.</em></font></td>
    </tr>
    <tr>
      <input type=hidden value="/admin/wlwpa_mbssid.asp" name="submit-url">
      <td width="100%" colspan="2"><input type=submit value="Apply Changes" name=save onClick="return saveChanges()">&nbsp;
      </td>
    </tr>
    <script>
        
	<% initPage("wlwpa_mbssid"); %>	
    	checkState();
        defPskLen = document.formEncrypt.pskValue.value.length;
	defPskFormat = document.formEncrypt.pskFormat.selectedIndex;
	
    </script>
  </table>
</form>
</blockquote>
</body>

</html>
