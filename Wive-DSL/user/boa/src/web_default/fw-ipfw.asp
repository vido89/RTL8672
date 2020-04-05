<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>IP Forwarding</title>
<script type="text/javascript" src="share.js">
</script>
<script>
function addClick()
{
  if (!document.formIPFwAdd.enabled.checked)
  	return true;
	
  if (document.formIPFwAdd.l_ip.value=="" && document.formIPFwAdd.r_ip.value=="" )
	return true;

  if (document.formIPFwAdd.l_ip.value=="") {
	alert("Empty Local IP address!");
	document.formIPFwAdd.l_ip.focus();
	return false;
  }
  if ( validateKey( document.formIPFwAdd.l_ip.value ) == 0 ) {
	alert("Invalid Local IP address value.");
	document.formIPFwAdd.l_ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formIPFwAdd.l_ip.value,1,0,255) ) {
      	alert('Invalid Local IP address range in 1st digit.');
	document.formIPFwAdd.l_ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formIPFwAdd.l_ip.value,2,0,255) ) {
      	alert('Invalid Local IP address range in 2nd digit.');
	document.formIPFwAdd.l_ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formIPFwAdd.l_ip.value,3,0,255) ) {
      	alert('Invalid Local IP address range in 3rd digit.');
	document.formIPFwAdd.l_ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formIPFwAdd.l_ip.value,4,1,254) ) {
      	alert('Invalid Local IP address range in 4th digit.');
	document.formIPFwAdd.l_ip.focus();
	return false;
  }

  if (document.formIPFwAdd.r_ip.value=="") {
	alert("Empty External IP address!");
	document.formIPFwAdd.r_ip.focus();
	return false;
  }
  if ( validateKey( document.formIPFwAdd.r_ip.value ) == 0 ) {
	alert("Invalid External IP address value.");
	document.formIPFwAdd.r_ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formIPFwAdd.r_ip.value,1,0,255) ) {
      	alert('Invalid External IP address range in 1st digit.');
	document.formIPFwAdd.r_ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formIPFwAdd.r_ip.value,2,0,255) ) {
      	alert('Invalid External IP address range in 2nd digit.');
	document.formIPFwAdd.r_ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formIPFwAdd.r_ip.value,3,0,255) ) {
      	alert('Invalid External IP address range in 3rd digit.');
	document.formIPFwAdd.r_ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formIPFwAdd.r_ip.value,4,1,254) ) {
      	alert('Invalid External IP address range in 4th digit.');
	document.formIPFwAdd.r_ip.focus();
	return false;
  }

   return true;
}

function disableDelButton()
{
  if (verifyBrowser() != "ns") {
	disableButton(document.formIPFwDel.delSelEntry);
	disableButton(document.formIPFwDel.delAllEntry);
  }
}

function updateState()
{
  if (document.formIPFwAdd.enabled.checked) {
 	enableTextField(document.formIPFwAdd.l_ip);
	enableTextField(document.formIPFwAdd.r_ip);
  }
  else {
 	disableTextField(document.formIPFwAdd.l_ip);
	disableTextField(document.formIPFwAdd.r_ip);
  }
}

</script>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">NAT IP Forwarding</font></h2>

<table border=0 width="550" cellspacing=4 cellpadding=0>
<tr><td><font size=2>
 Entries in this table allow you to automatically redirect traffic
 to a specific machine behind the NAT firewall.  These settings are only necessary
 if you wish to host some sort of server like a web server or mail server on the
 private local network behind your Gateway's NAT firewall.
</font></td></tr>

<tr><td><hr size=1 noshade align=top></td></tr>

<form action=/goform/formIPFw method=POST name="formIPFwAdd">

<tr><td><font size=2><b>
   	<input type="checkbox" name="enabled" value="ON" <% checkWrite("ipFwEn"); %>
   	 ONCLICK=updateState()>&nbsp;&nbsp;Enable NAT IP Forwarding</b><br>
    </td>
</tr>

<tr><td>
    <font size=2><b>Local IP Address:</b> <input type="text" name="l_ip" size="10" maxlength="15"></td>
</tr>
<tr><td>
    <font size=2><b>External IP Address:</b> <input type="text" name="r_ip" size="10" maxlength="15"></td>
</tr>
<tr><td>
  <input type="submit" value="Apply Changes" name="addEntry" onClick="return addClick()">&nbsp;&nbsp;
  <input type="hidden" value="/fw-ipfw.asp" name="submit-url">
</td></tr>
  <script> updateState(); </script>
</form>
</table>


<br>
<form action=/goform/formIPFw method=POST name="formIPFwDel">
<table border=0 width=500>
  <tr><font size=2><b>Current NAT IP Forwarding Table:</b></font></tr>
  <% ipFwList(); %>
</table>

 <br><input type="submit" value="Delete Selected" name="delSelEntry" onClick="return deleteClick()">&nbsp;&nbsp;
     <input type="submit" value="Delete All" name="delAllEntry" onClick="return deleteAllClick()">&nbsp;&nbsp;&nbsp;
     <input type="reset" value="Reset" name="reset">
 <script>
   	<% checkWrite("ipFwNum"); %>
 </script>
     <input type="hidden" value="/fw-ipfw.asp" name="submit-url">
</form>

</td></tr></table>

</blockquote>
</body>
</html>

