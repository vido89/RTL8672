<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Access Control for Network Services</title>
<script type="text/javascript" src="share.js">
</script>

<script>
function addClick()
{
   dTelnet = getDigit(document.acc.w_telnet_port.value, 1);
   dFtp = getDigit(document.acc.w_ftp_port.value, 1);
   dWeb = getDigit(document.acc.w_web_port.value, 1);
   if (dTelnet == dFtp || dTelnet == dWeb) {
	alert("Duplicated port number!");
	document.acc.w_telnet_port.focus();
	return false;
   }
   if (dFtp == dWeb) {
	alert("Duplicated port number!");
	document.acc.w_ftp_port.focus();
	return false;
   }
  
   if (document.acc.w_telnet.checked) {
      if (document.acc.w_telnet_port.value=="") {
         alert("Port range cannot be empty! You should set a value between 1-65535.");
	  document.acc.w_telnet_port.focus();
   	  return false;
      }
      if ( validateKey( document.acc.w_telnet_port.value ) == 0 ) {
	  alert("Invalid port number! It should be the decimal number (0-9).");
	  document.acc.w_telnet_port.focus();
	  return false;
      }
      //d1 = getDigit(document.acc.w_telnet_port.value, 1);
      //if (d1 > 65535 || d1 < 1) {
      if (dTelnet > 65535 || dTelnet < 1) {
	  alert("Invalid port number! You should set a value between 1-65535.");
	  document.acc.w_telnet_port.focus();
	  return false;
      }  
   }

   if (document.acc.w_ftp.checked) {
      if (document.acc.w_ftp_port.value=="") {
         alert("Port range cannot be empty! You should set a value between 1-65535.");
	  document.acc.w_ftp_port.focus();
   	  return false;
      }
      if ( validateKey( document.acc.w_ftp_port.value ) == 0 ) {
	  alert("Invalid port number! It should be the decimal number (0-9).");
	  document.acc.w_ftp_port.focus();
	  return false;
      }
      //d1 = getDigit(document.acc.w_ftp_port.value, 1);
      //if (d1 > 65535 || d1 < 1) {
      if (dFtp > 65535 || dFtp < 1) {
	  alert("Invalid port number! You should set a value between 1-65535.");
	  document.acc.w_ftp_port.focus();
	  return false;
      }  
   }

   if (document.acc.w_web.checked) {
      if (document.acc.w_web_port.value=="") {
         alert("Port range cannot be empty! You should set a value between 1-65535.");
	  document.acc.w_web_port.focus();
   	  return false;
      }
      if ( validateKey( document.acc.w_web_port.value ) == 0 ) {
	  alert("Invalid port number! It should be the decimal number (0-9).");
	  document.acc.w_web_port.focus();
	  return false;
      }
      //d1 = getDigit(document.acc.w_web_port.value, 1);
      //if (d1 > 65535 || d1 < 1) {
      if (dWeb > 65535 || dWeb < 1) {
	  alert("Invalid port number! You should set a value between 1-65535.");
	  document.acc.w_web_port.focus();
	  return false;
      }  
   }

   return true;

}

</script>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">Remote Access</font></h2>

<table border=0 width=700 cellspacing=4 cellpadding=0>
<tr><td colspan=4><font size=2>
 This page is used to enable/disable management services for the
 LAN and WAN.
</font></td></tr>

<tr><td><hr size=1 noshade align=top></td></tr>
</table>
<form action=/goform/formSAC method=POST name=acc>
<table border=0 cellpadding=3 cellspacing=0>
<tr>
	<td width=150 align=left><font size=2><b>Service Name</b></td>
	<td width=80 align=center><font size=2><b>LAN</b></td>
	<td width=80 align=center><font size=2><b>WAN</b></td>
	<td width=80 align=center><font size=2><b>WAN Port</b></td>
</tr>
<% rmtaccItem(); %>
</table>
<br>
<tr>
	<td><input type="submit" value="Apply Changes" name="set" onClick="return addClick()"></td>
	<td><input type="hidden" value="/rmtacc.asp" name="submit-url"></td>
</tr>
<script>
	<% accPost(); %>
</script>
</form>
</blockquote>
</body>
</html>
