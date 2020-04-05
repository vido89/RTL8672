<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>DNS Configuration </title>
<script type="text/javascript" src="share.js">
</script>
<% language=javascript %>
<SCRIPT>
function autoDNSclicked()
{
	document.dns.dns1.disabled = true;
	document.dns.dns2.disabled = true;
	document.dns.dns3.disabled = true;
}

function manualDNSclicked()
{
	document.dns.dns1.disabled = false;
	document.dns.dns2.disabled = false;
	document.dns.dns3.disabled = false;
}

function saveClick()
{
	if (document.dns.dnsMode[0].checked) {
		return true;
	}
	
/*	if (document.dns.dns1.value=="") {
		alert("Enter DNS value !");
		document.dns.dns1.focus();
		return false;
	}
	if ( validateKey( document.dns.dns1.value ) == 0 ) {
		alert("Invalid DNS 1 value.");
		document.dns.dns1.focus();
		return false;
	}

	if( IsLoopBackIP( document.dns.dns1.value)==1 ) {
		alert("Invalid DNS 1 value.");
		document.dns.dns1.focus();
		return false;
	}
		
	if ( !checkDigitRange(document.dns.dns1.value,1,0,254) ) {
		alert('Invalid DNS 1 range in 1st digit. It should be 0-254.');
		document.dns.dns1.focus();
		return false;
	}
	if ( !checkDigitRange(document.dns.dns1.value,2,0,255) ) {
		alert('Invalid DNS 1 range in 2nd digit. It should be 0-255.');
		document.dns.dns1.focus();
		return false;
	}
	if ( !checkDigitRange(document.dns.dns1.value,3,0,255) ) {
		alert('Invalid DNS 1 range in 3rd digit. It should be 0-255.');
		document.dns.dns1.focus();
		return false;
	}
	if ( !checkDigitRange(document.dns.dns1.value,4,0,255) ) {
		alert('Invalid DNS 1 range in 4th digit. It should be 0-255.');
		document.dns.dns1.focus();
		return false;
	}
	if (!checkIP(document.dns.dns1))*/
	if (!checkHostIP(document.dns.dns1, 1))
		return false;
	
//	if (document.dns.dns2.value=="") {
	if (document.dns.dns2.value=="") {
		if (document.dns.dns3.value=="")	// Jenny,  buglist B059, dns2 couldn't be empty if dns3 is not empty
			return true;
		else {
			alert("Enter DNS 2 value !");
			document.dns.dns2.focus();
			return false;
		}
	}
	if (!checkHostIP(document.dns.dns2, 0))
		return false;
	/*if ( validateKey( document.dns.dns2.value ) == 0 ) {
		alert("Invalid DNS 2 value.");
		document.dns.dns2.focus();
		return false;
	}

	if( IsLoopBackIP( document.dns.dns2.value)==1 ) {
		alert("Invalid DNS 2 value.");
		document.dns.dns2.focus();
		return false;
	}
	if( IsInvalidIP( document.dns.dns2.value)==1 ) {
		alert("Invalid DNS 2 value.");
		document.dns.dns2.focus();
		return false;
	}
	
	if ( !checkDigitRange(document.dns.dns2.value,1,0,254) ) {
		alert('Invalid DNS 2 range in 1st digit. It should be 0-254.');
		document.dns.dns2.focus();
		return false;
	}
	if ( !checkDigitRange(document.dns.dns2.value,2,0,255) ) {
		alert('Invalid DNS 2 range in 2nd digit. It should be 0-255.');
		document.dns.dns2.focus();
		return false;
	}
	if ( !checkDigitRange(document.dns.dns2.value,3,0,255) ) {
		alert('Invalid DNS 2 range in 3rd digit. It should be 0-255.');
		document.dns.dns2.focus();
		return false;
	}
	if ( !checkDigitRange(document.dns.dns2.value,4,1,254) ) {
		alert('Invalid DNS 2 range in 4th digit. It should be 1-254.');
		document.dns.dns2.focus();
		return false;
	}*/
	
	if (document.dns.dns3.value=="") {
		return true;
	}
	if (!checkHostIP(document.dns.dns3, 0))
		return false;
	/*if ( validateKey( document.dns.dns3.value ) == 0 ) {
		alert("Invalid DNS 3 value.");
		document.dns.dns3.focus();
		return false;
	}
	if( IsLoopBackIP( document.dns.dns3.value)==1 ) {
		alert("Invalid DNS 3 value.");
		document.dns.dns3.focus();
		return false;
	}
	if( IsInvalidIP( document.dns.dns3.value)==1 ) {
		alert("Invalid DNS 3 value.");
		document.dns.dns3.focus();
		return false;
	}
	
	if ( !checkDigitRange(document.dns.dns3.value,1,0,254) ) {
		alert('Invalid DNS 3 range in 1st digit. It should be 0-254.');
		document.dns.dns3.focus();
		return false;
	}
	if ( !checkDigitRange(document.dns.dns3.value,2,0,255) ) {
		alert('Invalid DNS 3 range in 2nd digit. It should be 0-255.');
		document.dns.dns3.focus();
		return false;
	}
	if ( !checkDigitRange(document.dns.dns3.value,3,0,255) ) {
		alert('Invalid DNS 3 range in 3rd digit. It should be 0-255.');
		document.dns.dns3.focus();
		return false;
	}
	if ( !checkDigitRange(document.dns.dns3.value,4,0,255) ) {
		alert('Invalid DNS 3 range in 4th digit. It should be 0-255.');
		document.dns.dns3.focus();
		return false;
	}*/
	
	return true;
}

function resetClick()
{
	if (initAutoDns)
		autoDNSclicked();
	else
		manualDNSclicked();
}

</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">DNS Configuration</font></h2>

<form action=/goform/formDNS method=POST name="dns">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><font size=2>
    This page is used to configure the DNS server IP addresses for DNS Relay.
  </tr>
  <tr><hr size=1 noshade align=top></tr>
    <tr>
      <td width="100%" colspan="3"><font size=2>
        <b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type="radio" value="dnsAuto" name="dnsMode" <%checkWrite("dns0"); %> onClick="autoDNSclicked()">Attain DNS Automatically
        </b>
      </td>
    </tr>
    <tr>
      <td width="100%" colspan="3"><font size=2>
        <b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type="radio" value="dnsManual" name="dnsMode" <%checkWrite("dns1"); %> onClick="manualDNSclicked()">Set DNS Manually
        </b>
      </td>
    </tr>
    <tr>
       <td width="40%"><font size=2><b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;DNS 1:</b></td>
       <td width="60%"><font size=2><input type="text" name="dns1" size="18" maxlength="15" value=<% getInfo("wan-dns1"); %>></td>
    </tr>
    <tr>
       <td width="40%"><font size=2><b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;DNS 2:</b></td>
       <td width="60%"><font size=2><input type="text" name="dns2" size="18" maxlength="15" value=<% getInfo("wan-dns2"); %>></td>
    </tr>
    <tr>
       <td width="40%"><font size=2><b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;DNS 3:</b></td>
       <td width="60%"><font size=2><input type="text" name="dns3" size="18" maxlength="15" value=<% getInfo("wan-dns3"); %>></td>
    </tr>
</table>
  <input type="hidden" value="/dns.asp" name="submit-url">
  <input type="submit" value="Apply Changes" name="save" onClick="return saveClick()">
  <input type="reset" value="Reset Selected" name="reset" onClick="return resetClick()">
<script>
	initAutoDns = document.dns.dnsMode[0].checked;
	if (document.dns.dnsMode[0].checked)
		autoDNSclicked();
</script>
</form>
</blockquote>
</body>

</html>
