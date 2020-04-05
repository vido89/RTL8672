
<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>DHCP Relay Configuration </title>
<script type="text/javascript" src="share.js">
</script>
<% language=javascript %>
<SCRIPT>
function saveClick()
{	
	
	if (document.dhcrelay.dhcps.value=="") {
		alert("Enter DHCPS value !");
		document.dhcrelay.dhcps.focus();
		return false;
	}
	if ( validateKey( document.dhcrelay.dhcps.value ) == 0 ) {
		alert("Invalid DHCPS value.");
		document.dhcrelay.dhcps.focus();
		return false;
	}

	if( IsLoopBackIP( document.dhcrelay.dhcps.value)==1 ) {
		alert("Invalid DHCPS value.");
		document.dhcrelay.dhcps.focus();
		return false;
	}
	
	if ( !checkDigitRange(document.dhcrelay.dhcps.value,1,0,254) ) {
		alert('Invalid DHCPS range in 1st digit. It should be 0-254.');
		document.dhcrelay.dhcps.focus();
		return false;
	}
	if ( !checkDigitRange(document.dhcrelay.dhcps.value,2,0,255) ) {
		alert('Invalid DHCPS range in 2nd digit. It should be 0-255.');
		document.dhcrelay.dhcps.focus();
		return false;
	}
	if ( !checkDigitRange(document.dhcrelay.dhcps.value,3,0,255) ) {
		alert('Invalid DHCPS range in 3rd digit. It should be 0-255.');
		document.dhcrelay.dhcps.focus();
		return false;
	}
	if ( !checkDigitRange(document.dhcrelay.dhcps.value,4,1,254) ) {
		alert('Invalid DHCPS range in 4th digit. It should be 1-254.');
		document.dhcrelay.dhcps.focus();
		return false;
	}	
	
	return true;
}



</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">DHCP Relay Configuration</font></h2>

<form action=/goform/formDhcrelay method=POST name="dhcrelay">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><font size=2>
    This page is used to configure the DHCP server ip addresses for DHCP Relay.
  </tr>
  <tr><hr size=1 noshade align=top></tr>    
    <tr>
       <td width="40%"><font size=2><b>DHCP Server Address:</b></td>
       <td width="60%"><font size=2><input type="text" name="dhcps" size="18" maxlength="15" value=<% getInfo("wan-dhcps"); %>></td>
    </tr>    
</table>
  <input type="hidden" value="/dhcrelay.asp" name="submit-url">
  <input type="submit" value="Apply Changes" name="save" onClick="return saveClick()">  

 <script>
	<% initPage("dhcprelay"); %>
</script>

</form>
</blockquote>
</body>

</html>
