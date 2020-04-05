<html>
<! Copyright (c) Realtek Semiconductor Corp., 2007. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Wi-Fi Protected Setup</title>
<script type="text/javascript" src="share.js"> </script>
<style>
.on {display:on}
.off {display:none}
</style>
<script>
var isClient;
var isConfig;
var encrypt=<% getInfo("encrypt");%>;						
var enable1x=<% getInfo("enable1X");%>;
var wpa_auth=<% getInfo("wpaAuth");%>;
var mode=<% getInfo("wlanMode");%>;
var is_adhoc=<% getInfo("networkType");%>;
var warn_msg1='WPS was disabled automatically because wireless mode setting could not be supported. ' +
				'You need go to Wireless/Basic page to modify settings to enable WPS.';
var warn_msg2='WPS was disabled automatically because Radius Authentication could not be supported. ' +
				'You need go to Wireless/Security page to modify settings to enable WPS.';
var warn_msg3="PIN number was generated. You have to click \'Apply Changes\' button to make change effectively.";
var disable_all=0;

function triggerPBCClicked()
{
  	return true;
}

function triggerPINClicked()
{
	return(saveChangesWPS(document.formWsc));	
}

function compute_pin_checksum(val)
{
	var accum = 0;	
	var code = parseInt(val)*10;

	accum += 3 * (parseInt(code / 10000000) % 10); 
	accum += 1 * (parseInt(code / 1000000) % 10); 
	accum += 3 * (parseInt(code / 100000) % 10); 
	accum += 1 * (parseInt(code / 10000) % 10);
	accum += 3 * (parseInt(code / 1000) % 10);
	accum += 1 * (parseInt(code / 100) % 10);
	accum += 3 * (parseInt(code / 10) % 10); 
	accum += 1 * (parseInt(code / 1) % 10);	
	var digit = (parseInt(accum) % 10);
	return ((10 - digit) % 10);
}

function genPinClicked()
{
	var num_str="1";
	var rand_no;
	var num;

	while (num_str.length != 7) {
		rand_no = Math.random()*1000000000;	
		num = parseInt(rand_no, 10);
		num = num%10000000;
		num_str = num.toString();
	}
	
	num = num*10 + compute_pin_checksum(num);
	num = parseInt(num, 10);	
	document.formWsc.elements["localPin"].value = num; 
	alert(warn_msg3);
}

function validate_pin_code(code)
{
	var accum=0;

	accum += 3 * (parseInt(code / 10000000) % 10); 
	accum += 1 * (parseInt(code / 1000000) % 10); 
	accum += 3 * (parseInt(code / 100000) % 10); 
	accum += 1 * (parseInt(code / 10000) % 10);
	accum += 3 * (parseInt(code / 1000) % 10);
	accum += 1 * (parseInt(code / 100) % 10);
	accum += 3 * (parseInt(code / 10) % 10); 
	accum += 1 * (parseInt(code / 1) % 10);
	return (0 == (accum % 10));	
}

function check_pin_code(str)
{
	var i;
	var code_len;
		
	code_len = str.length;
	if (code_len != 8 && code_len != 4)
		return 1;

	for (i=0; i<code_len; i++) {
		if ((str.charAt(i) < '0') || (str.charAt(i) > '9'))
			return 2;
	}

	if (code_len == 8) {
		var code = parseInt(str, 10);
		if (!validate_pin_code(code))
			return 3;
		else
			return 0;
	}
	else
		return 0;
}

function setPinClicked(form)
{
	var ret;

	ret = check_pin_code(form.elements["peerPin"].value);
	if (ret == 1) {
		alert('Invalid Enrollee PIN length! The device PIN is usually four or eight digits long.');
		form.peerPin.focus();		
		return false;
	}
	else if (ret == 2) {
		alert('Invalid Enrollee PIN! Enrollee PIN must be numeric digits.');
		form.peerPin.focus();		
		return false;
	}
	else if (ret == 3) {
		if ( !confirm('Checksum failed! Use PIN anyway? ') ) {
			form.peerPin.focus();
			return false;
  		}
	}	
	return true;
}


function checkWPSstate(form)
{
	if (form.elements["wlanDisabled"].value == "ON") {
    		disableCheckBox(form.elements["disableWPS"]);
  	}
	if (disable_all) {
		disableCheckBox(form.elements["disableWPS"]);
		disableButton(form.elements["save"]);
		disableButton(form.elements["reset"]);  
	}
	if (disable_all || form.elements["disableWPS"].checked) {	 	
	 	disableTextField(form.elements["localPin"]);	 	
	 	disableTextField(form.elements["peerPin"]);
	 	disableButton(form.elements["setPIN"]);
		disableButton(form.elements["triggerPIN"]);	 	
		disableButton(form.elements["triggerPBC"]);
		disableButton(form.elements["genPIN"]);
  	}
	else {
		enableTextField(form.elements["localPin"]);
		enableTextField(form.elements["peerPin"]);
	 	enableButton(form.elements["setPIN"]);
		enableButton(form.elements["genPIN"]);
		enableButton(form.elements["triggerPIN"]);		
		enableButton(form.elements["triggerPBC"]);
	}
	disableRadioGroup(form.elements["config"]);	 
	return true;
}

function saveChangesWPS(form)
{
	ret = check_pin_code(form.elements["localPin"].value);
	if (ret == 1) {
		alert('Invalid PIN length! The device PIN is usually four or eight digits long.');
		form.localPin.focus();
		return false;
	}
	else if (ret == 2) {
		alert('Invalid PIN! The device PIN must be numeric digits.');
		form.localPin.focus();		
		return false;
	}
	else if (ret == 3) {
		alert('Invalid PIN! Checksum error.');
		form.localPin.focus();		
		return false;
	}  	
   	return true;
}

</script>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">Wi-Fi Protected Setup</font></h2>

<form action=/goform/formWsc method=POST name="formWsc">
<table border=0 width="500" cellspacing=4 cellpadding=0>
<tr><font size=2>
 This page allows you to change the setting for WPS (Wi-Fi Protected Setup). Using 
 this feature could let your wireless client automically syncronize its setting and 
 connect to the Access Point in a minute without any hassle.
</font></tr>
	<input type=hidden name="wlanDisabled" value=<% wlanStatus %>>
<script>
    <% checkWrite("wlanMode");%>  
	if (mode == 0 || mode == 3)
	    disable_all = check_wps_enc(encrypt, enable1x, wpa_auth);
	else
		disable_all = check_wps_wlanmode(mode, is_adhoc);
</script>
<tr>
  <td width="100%" colspan=3><font size=2><b>
   	<input type="checkbox" name="disableWPS" value="ON" <% checkWrite("wscDisable");%> ONCLICK="checkWPSstate(document.formWsc)">&nbsp;&nbsp;Disable WPS
  </td>
</tr>
<tr><hr size=1 noshade align=top></tr>
<script>
  if (isClient) {
	document.write("</table>\n");
	document.write("<span id = \"hide_div\" class = \"off\">\n");
	document.write("<table border=\"0\" width=500>\n");
  }
</script>
<tr>
  <td width="40%"><font size="2"><b>WPS Status:</b></font></td>  
  <td width="60%"><font size="2">
	  <input type="radio" name="config" value="on" <% checkWrite("wscConfig-1"); %>>Configured&nbsp;&nbsp;
	  <input type="radio" name="config" value="off" <% checkWrite("wscConfig-0"); %>>UnConfigured
	</font></td>  
</tr>
<script>
  if (isClient) {
	document.write("</table>\n");
	document.write("</span>\n");
	document.write("<table border=\"0\" width=500>\n");
  }
</script>
<tr>
  <td width="40%"><font size="2"><b>Self-PIN Number:</b></font></td>
  <td width="60%"><font size="2"><input type="text" name="localPin" size="12" maxlength="10" value=<% getInfo("wscLoocalPin");%>>
  	&nbsp;&nbsp;<input type="button" value="Regenerate PIN" name="genPIN" onClick="return genPinClicked()"></td>
</tr>

<script>
  if (!isClient) {
	document.write("</table>\n");
	document.write("<span id = \"hide_div\" class = \"off\">\n");
	document.write("<table border=\"0\" width=500>\n");
  }
</script>
<tr>
  <td width="40%"><font size="2"><b>PIN Configuration:</b></font></td> 
  <td width="60%"><font size="2">
  	<input type="submit" value="Start PIN" name="triggerPIN" onClick="return triggerPINClicked()"></td>
	</font></td>  
</tr>
<script>
  if (!isClient) {
	document.write("</table>\n");
	document.write("</span>\n");
	document.write("<table border=\"0\" width=500>\n");
  }
</script>

<tr>
  <td width="40%"><font size="2"><b>Push Button Configuration:</b></font></td> 
  <td width="60%"><font size="2">
  	<input type="submit" value="Start PBC" name="triggerPBC" onClick="return triggerPBCClicked()"></td>
	</font></td>  
</tr>

<input type="hidden" value="/wlwps.asp" name="submit-url">
<tr>
   <td width="100%" colspan="2"  height=40><input type="submit" value="Apply Changes" name="save" onClick="return saveChangesWPS(document.formWsc)">&nbsp;&nbsp;
		<input type="reset" value="Reset" name="reset"></td>
</tr>

 <script>
 	if (disable_all) {
		 document.write("<tr><td colspan=\"2\" height=\"55\"><font size=2><em>");
	   	if (disable_all == 1)     
   			document.write(warn_msg1);
	   	else
	   		document.write(warn_msg2);
		document.write("</td></tr>"); 	   	
 	}
</script>   	

</table>

<script>
    <% checkWrite("wscConfig-A");%>     		
	if (isClient || !isConfig)
		document.write("<span id = \"hide_div\" class = \"off\">\n");
</script>
<table border='0' width="500">
<tr><td><font size=2><b>Current Key Info:</b></td></tr>
<table border='1' width="500">
<tr bgcolor=#7f7f7f>
   <td width="30%"><font size=2><b>Authentication</b></td>
   <td width="20%"><font size=2><b>Encryption</b></td>
   <td width="50%"><font size=2><b>Key</b></td>
</tr>

<tr>
   <td width="30%"><font size=2>
     <% checkWrite("wps_auth");%></td>
   <td width="20%"><font size=2>
     <% checkWrite("wps_enc");%></td>
   <td width="50%"><font size=2>
     <%getInfo("wps_key");%></td>
</tr>
</table><br></table>

<script>
  if (isClient || !isConfig) {
	document.write("</span>\n");
	document.write("<table border=\"0\" width=500>\n");
  }      
  if (isClient)
	document.write("<span id = \"hide_div\" class = \"off\">\n");
</script>
<table border=0 width="500" cellspacing=4 cellpadding=0>
<hr width=100%>
<tr>
  <td width="40%"><font size="2"><b>Client PIN Number:</b></font></td>
  <td width="60%"><font size="2"><input type="text" name="peerPin" size="12" maxlength="10" value="">
  	&nbsp;&nbsp;<input type="submit" value="Start PIN" name="setPIN" onClick="return setPinClicked(document.formWsc)"></td>
</tr>
</table>
<script>
  if (isClient)
	document.write("</span>\n");
   checkWPSstate(document.formWsc);
</script>
  
</form>
</blockquote>
</body>

</html>
