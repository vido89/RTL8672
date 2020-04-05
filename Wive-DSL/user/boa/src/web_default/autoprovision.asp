<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Auto-Provisioning Configuration </title>
<script type="text/javascript" src="share.js">
</script>
<% language=javascript %>
<SCRIPT>
function resetClick()
{
   document.autop.reset;
}

function saveChanges()
{
  if (document.autop.ip.value=="") {
	alert("IP address cannot be empty! It should be filled with 4 digit numbers as xxx.xxx.xxx.xxx.");
	document.autop.ip.value = document.autop.ip.defaultValue;
	document.autop.ip.focus();
	return false;
  }
  if ( validateKey( document.autop.ip.value ) == 0 ) {
	alert("Invalid IP address value. It should be the decimal number (0-9).");
	document.autop.ip.value = document.autop.ip.defaultValue;
	document.autop.ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.autop.ip.value,1,0,255) ) {
      	alert('Invalid IP address range in 1st digit. It should be 0-255.');
	document.autop.ip.value = document.autop.ip.defaultValue;
	document.autop.ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.autop.ip.value,2,0,255) ) {
      	alert('Invalid IP address range in 2nd digit. It should be 0-255.');
	document.autop.ip.value = document.autop.ip.defaultValue;
	document.autop.ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.autop.ip.value,3,0,255) ) {
      	alert('Invalid IP address range in 3rd digit. It should be 0-255.');
	document.autop.ip.value = document.autop.ip.defaultValue;
	document.autop.ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.autop.ip.value,4,1,254) ) {
      	alert('Invalid IP address range in 4th digit. It should be 1-254.');
	document.autop.ip.value = document.autop.ip.defaultValue;
	document.autop.ip.focus();
	return false;
  }
  return true;
}

</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">Auto-Provisioning Configuration</font></h2>

<form action=/goform/formAutoProvision method=POST name="autop">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><font size=2>
    This page is used to configure the auto-provisioning remote HTTP server.
    Here you may change the setting for IP addresss.
  </tr>
  <tr><hr size=1 noshade align=top></tr>

  <tr>
      <td width="40%"><font size=2><b>HTTP Server IP Address:</b></td>
      <td width="60%"><input type="text" name="ip" size="15" maxlength="15" value=<% getInfo("http-ip"); %>></td>
  </tr>

  </table>
  <br>
      <input type="submit" value="Apply Changes" name="save" onClick="return saveChanges()">&nbsp;&nbsp;
      <input type="reset" value="Undo" name="reset" onClick="resetClick()">
      <input type="hidden" value="/autoprovision.asp" name="submit-url">
 </form>
</blockquote>
</body>

</html>
