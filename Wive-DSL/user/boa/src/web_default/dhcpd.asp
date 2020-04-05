<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>DHCP Server Setup </title>
<script type="text/javascript" src="share.js">
</script>
<% language=javascript %>
<SCRIPT>
var pool_ipprefix;
var initialDhcp;
function skip () { this.blur(); }
function openWindow(url, windowName) {
	var wide=600;
	var high=400;
	if (document.all)
		var xMax = screen.width, yMax = screen.height;
	else if (document.layers)
		var xMax = window.outerWidth, yMax = window.outerHeight;
	else
	   var xMax = 640, yMax=480;
	var xOffset = (xMax - wide)/2;
	var yOffset = (yMax - high)/3;

	var settings = 'width='+wide+',height='+high+',screenX='+xOffset+',screenY='+yOffset+',top='+yOffset+',left='+xOffset+', resizable=yes, toolbar=no,location=no,directories=no,status=no,menubar=no,scrollbars=yes';

	window.open( url, windowName, settings );
}

function showdns()
{
	if (document.dhcpd.dhcpdns[0].checked == true) {
		if (document.getElementById)  // DOM3 = IE5, NS6
			document.getElementById('dnsset').style.display = 'none';
		else {
			if (document.layers == false) // IE4
				document.all.dnsset.style.display = 'none';
		}
	} else {
		if (document.getElementById)  // DOM3 = IE5, NS6
			document.getElementById('dnsset').style.display = 'block';
		else {
			if (document.layers == false) // IE4
				document.all.dnsset.style.display = 'block';
		}
	}
}

function showDhcpSvr(ip)
{
	var html;
	
	if (document.dhcpd.dhcpdenable[0].checked == true)
		document.getElementById('displayDhcpSvr').innerHTML=
			'<input type="submit" value="Apply Changes" name="save">&nbsp;&nbsp;';
	else if (document.dhcpd.dhcpdenable[1].checked == true)
		document.getElementById('displayDhcpSvr').innerHTML=
			'<table border=0 width="500" cellspacing=4 cellpadding=0>'+
			'<tr><b><font color="#000000">DHCP Relay Configuration</font></b></tr>'+
			'<tr><font color="#000000" size=2>'+
			    'This page is used to configure the DHCP server ip addresses for DHCP Relay.'+
			'</tr>'+
			'<tr><hr size=1 noshade align=top></tr>'+
			'</table>'+
			'<table border=0 width="500" cellspacing=4 cellpadding=0>'+
			   '<tr>'+
			        '<td width="40%"><font size=2><b>DHCP Server Address:</b></td>'+
			        '<td width="60%"><font size=2><input type="text" name="dhcps" size="18" maxlength="15" value=<% getInfo("wan-dhcps"); %>></td>'+
			   '</tr>'+
			'</table>'+
			'<input type="submit" value="Apply Changes" name="save" onClick="return saveClick()">&nbsp;&nbsp;';
		
	else if (document.dhcpd.dhcpdenable[2].checked == true) {
		html=
			'<table border=0 width="500" cellspacing=4 cellpadding=0>'+
			'<tr><b><font color="#000000">DHCP Server</font></b></tr>'+
			'<tr><font color="#000000" size=2>'+
			    'Enable the DHCP Server if you are using this device as a DHCP server. This page lists the '+
			    'IP address pools available to hosts on your LAN. The device distributes numbers in the '+
			    'pool to hosts on your network as they request Internet access.'+
			'</tr>'+
			'<tr><hr size=1 noshade align=top></tr>'+
			'<tr><font color="#000000" size=2><b>LAN IP Address: </b><% getInfo("dhcplan-ip"); %>&nbsp;&nbsp;'+
			    '<b>Subnet Mask: </b><% getInfo("dhcplan-subnet"); %>'+
			'</tr>'+
		 	'</table>'+
			'<table border=0 width="500" cellspacing=4 cellpadding=0>'+
			'<tr>'+
			'<td width="30%"><font size=2><b>IP Pool Range:</b></td>';
	
		if (pool_ipprefix)
			html+=
				'<td width="70%"><font size=2><b>'+pool_ipprefix+'<input type="text" name="dhcpRangeStart" size=3 maxlength=3 value="<% getInfo("lan-dhcpRangeStart"); %>">'+
				'<font face="Arial" size="5">- </font><font size=2>'+pool_ipprefix+'<input type="text" name="dhcpRangeEnd" size=3 maxlength=3 value="<% getInfo("lan-dhcpRangeEnd"); %>">&nbsp;';
		else
			html+=
				'<td width="70%"><input type="text" name="dhcpRangeStart" size=15 maxlength=15 value="<% getInfo("lan-dhcpRangeStart"); %>">'+
				'<font face="Arial" size="5">- <input type="text" name="dhcpRangeEnd" size=15 maxlength=15 value="<% getInfo("lan-dhcpRangeEnd"); %>">&nbsp;';
		html+=
			'<input type="button" value="Show Client" name="dhcpClientTbl" onClick="dhcpTblClick(\'/dhcptbl.asp\')" >'+
			'</td>'+
			'</tr>';
		
		if (!pool_ipprefix)
		{
			html +='<tr>'+
				'<td width="30%"><font size=2><b>Subnet Mask:</b></td>'+
				'<td width="70%"><font size=2>'+
				'<input type="text" name="dhcpSubnetMask" size=15 maxlength=15 value="<% getInfo("lan-dhcpSubnetMask"); %>">&nbsp;'+
				'</td>'+
				'</tr>';
		}
		
		html += '<tr>'+
			'<td width="30%"><font size=2><b>Max Lease Time:</b></td>'+
			'<td width="70%"><font size=2>'+
			'<input type="text" name="ltime" size=10 maxlength=9 value="<% getInfo("lan-dhcpLTime"); %>"><b> seconds</b><b> (-1 indicates an infinite lease)</b>'+
			'</td>'+
			'</tr>'+
			'<tr>'+
			'<td width="30%"><font size=2><b>Domain Name:</b></td>'+
			'<td width="70%">'+
			'<input type="text" name="dname" size=32 maxlength=29 value="<% getInfo("lan-dhcpDName"); %>">'+
			'</td>'+
			'</tr>'+
			'<tr>'+
			'<td width="30%"><font size=2><b>Gateway Address:</b></td>'+
			'<td width="70%"><input type="text" name="ip" size="15" maxlength="15" value=<% getInfo("lan-dhcp-gateway"); %>></td>'+
			'</tr>'+
			'</table>';
		if (en_dnsopt == 0)
			html += '<div ID=optID style="display:none">';
		else
			html += '<div ID=optID style="display:block">';
		
		html +=
			'<table border=0 width="500" cellspacing=4 cellpadding=0><tr>'+
			'<td width="30%"><font size=2><b>DNS option:</b></td>'+
			'<td width=70%><input type=radio name=dhcpdns value=0 onClick=showdns()>Use DNS Relay&nbsp;&nbsp;'+
			'<input type=radio name=dhcpdns value=1 onClick=showdns()>Set Manually&nbsp;&nbsp;</td>'+
			'</tr></table></div>'+
			'<div ID=dnsset style="display:none">'+
			'<table border=0 width="500" cellspacing=4 cellpadding=0>'+
			'<tr><td width=30%><b>DNS1:</b></td><td width=70%><input type=text name=dns1 value=<% getInfo("dhcps-dns1"); %>></td></tr>'+
			'<tr><td width=30%><b>DNS2:</b></td><td width=70%><input type=text name=dns2 value=<% getInfo("dhcps-dns2"); %>></td></tr>'+
			'<tr><td width=30%><b>DNS3:</b></td><td width=70%><input type=text name=dns3 value=<% getInfo("dhcps-dns3"); %>></td></tr></table></div>'+
			'<input type="submit" value="Apply Changes" name="save" onClick="return saveChanges()">&nbsp;&nbsp;'+
			'<input type="button" value="MAC-Base Assignment" name="macIpTbl" onClick="macIpClick(\'/macIptbl.asp\')">';
		
		document.getElementById('displayDhcpSvr').innerHTML=html;
		if (en_dnsopt) {
			document.dhcpd.dhcpdns[dnsopt].checked = true;
			showdns();
		}
	}
}

function checkInputIP(ip)
{
	var i, ip_d;
	for (i=1; i<5; i++) {
		ip_d = getDigit(ip, i);
	}
	if ((ip_d >= parseInt(document.dhcpd.dhcpRangeStart.value, 10)) && (ip_d <= parseInt(document.dhcpd.dhcpRangeEnd.value, 10))) {
		return false;
	}
	return true;
}

function saveClick()
{
	if (!checkHostIP(document.dhcpd.dhcps, 1)) {
  		return false;
  	}
	return true;
}

function checkSubnet(ip, mask, client)
{
  ip_d = getDigit(ip, 4);
  mask_d = getDigit(mask, 4);
  if ( (ip_d & mask_d) != (client & mask_d ) )
	return false;

  return true;
}

function checkDigitRange_leaseTime(str, min)
{
  d = parseInt(str, 10);
  if ( d < min || d == 0)
      	return false;
  return true;
}

function validateKey_leasetime(str)
{
   for (var i=0; i<str.length; i++) {
    if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
    		(str.charAt(i) == '-' ) )
			continue;
	return 0;
  }
  return 1;
}

function saveChanges()
{
  	if ( includeSpace(document.dhcpd.dname.value)) {
		alert('Invalid domain name.');
		document.dhcpd.dname.focus();
		return false;
 	}
	if (checkString(document.dhcpd.dname.value) == 0) {
		alert('Invalid domain name.');
		document.dhcpd.dname.focus();
		return false;
	}
	if (pool_ipprefix) {
	if (document.dhcpd.dhcpRangeStart.value=="") {
		alert("Please input DHCP IP pool range.");
		document.dhcpd.dhcpRangeStart.value = document.dhcpd.dhcpRangeStart.defaultValue;
		document.dhcpd.dhcpRangeStart.focus();
		return false;
	}
	if ( validateKey( document.dhcpd.dhcpRangeStart.value ) == 0 ) {
		alert("Invalid DHCP client start range. It should be 1-254.");	//cathy, for  bug B018
		document.dhcpd.dhcpRangeStart.value = document.dhcpd.dhcpRangeStart.defaultValue;
		document.dhcpd.dhcpRangeStart.focus();
		return false;
	}
	if ( !checkDigitRange(document.dhcpd.dhcpRangeStart.value,1,1,254) ) {
	  	   	alert('Invalid DHCP client start range. It should be 1-254.');
		document.dhcpd.dhcpRangeStart.value = document.dhcpd.dhcpRangeStart.defaultValue;
		document.dhcpd.dhcpRangeStart.focus();
		return false;
	}
	if ( !checkSubnet(document.dhcpd.lan_ip.value,document.dhcpd.lan_mask.value,document.dhcpd.dhcpRangeStart.value)) {
		alert('Invalid DHCP client start address!\nIt should be located in the same subnet of current IP address.');
		document.dhcpd.dhcpRangeStart.value = document.dhcpd.dhcpRangeStart.defaultValue;
		document.dhcpd.dhcpRangeStart.focus();
		return false;
	}
	if (document.dhcpd.dhcpRangeEnd.value=="") {
	  	alert("Please input DHCP IP pool range.");
		document.dhcpd.dhcpRangeEnd.value = document.dhcpd.dhcpRangeEnd.defaultValue;
		document.dhcpd.dhcpRangeEnd.focus();
		return false;
	}
	if ( validateKey( document.dhcpd.dhcpRangeEnd.value ) == 0 ) {
		alert("Invalid DHCP client end address range. It should be 1-254.");		//cathy, for  bug B018
		document.dhcpd.dhcpRangeEnd.value = document.dhcpd.dhcpRangeEnd.defaultValue;
		document.dhcpd.dhcpRangeEnd.focus();
		return false;
	}
	if ( !checkDigitRange(document.dhcpd.dhcpRangeEnd.value,1,1,254) ) {
	  	alert('Invalid DHCP client end range. It should be 1-254.');
		document.dhcpd.dhcpRangeEnd.value = document.dhcpd.dhcpRangeEnd.defaultValue;
		document.dhcpd.dhcpRangeEnd.focus();
		return false;
	}
	if ( !checkSubnet(document.dhcpd.lan_ip.value,document.dhcpd.lan_mask.value,document.dhcpd.dhcpRangeEnd.value)) {
		alert('Invalid DHCP client end address!\nIt should be located in the same subnet of current IP address.');
		document.dhcpd.dhcpRangeEnd.value = document.dhcpd.dhcpRangeEnd.defaultValue;
		document.dhcpd.dhcpRangeEnd.focus();
		return false;
	}
	if ( parseInt(document.dhcpd.dhcpRangeStart.value, 10) >= parseInt(document.dhcpd.dhcpRangeEnd.value, 10) ) {
		alert('Invalid DHCP client address range!\nEnding address should be greater than starting address.');
		document.dhcpd.dhcpRangeStart.focus();
		return false;
	}
	}
	else {
		if (!checkHostIP(document.dhcpd.dhcpRangeStart, 1)) {
			document.dhcpd.dhcpRangeStart.value = document.dhcpd.dhcpRangeStart.defaultValue;
			document.dhcpd.dhcpRangeStart.focus();
			return false;
		}
		if (!checkHostIP(document.dhcpd.dhcpRangeEnd, 1)) {
			document.dhcpd.dhcpRangeEnd.value = document.dhcpd.dhcpRangeEnd.defaultValue;
			document.dhcpd.dhcpRangeEnd.focus();
			return false;
		}
	}
	if (!checkInputIP(document.dhcpd.lan_ip.value)) {
		alert('Invalid IP pool range! LAN IP must be excluded from DHCP IP pool.');
		document.dhcpd.dhcpRangeStart.focus();
		return false;
	}

	if ( document.dhcpd.ltime.value=="") {
		alert("Please input dhcp lease time.");
		document.dhcpd.ltime.focus();
		return false;
	}
	if ( validateKey_leasetime( document.dhcpd.ltime.value ) == 0 ) {
		alert("Invalid DHCP Server lease time number.");
		document.dhcpd.ltime.value = document.dhcpd.ltime.defaultValue;
		document.dhcpd.ltime.focus();
		return false;
	}
	if ( !checkDigitRange_leaseTime(document.dhcpd.ltime.value, -1) ) {
	  	alert('Invalid DHCP Server lease time.');
		document.dhcpd.ltime.value = document.dhcpd.ltime.defaultValue;
		document.dhcpd.ltime.focus();
		return false;
	}
	if (!checkHostIP(document.dhcpd.ip, 1))
		return false;
	/*if (document.dhcpd.ip.value=="") {
		alert("Gateway address cannot be empty! It should be filled with 4 digit numbers as xxx.xxx.xxx.xxx.");
		document.dhcpd.ip.value = document.dhcpd.ip.defaultValue;
		document.dhcpd.ip.focus();
		return false;
  	}
  	if ( validateKey( document.dhcpd.ip.value ) == 0 ) {
		alert("Invalid Gateway address value. It should be the decimal number (0-9).");
		document.dhcpd.ip.value = document.dhcpd.ip.defaultValue;
		document.dhcpd.ip.focus();
		return false;
  	}

  	if( IsLoopBackIP( document.dhcpd.ip.value)==1 ) {
			alert("Invalid Gateway address value.");
			document.dhcpd.ip.focus();
			return false;
  	}

  	if ( !checkDigitRange(document.dhcpd.ip.value,1,0,254) ) {
  	    	alert('Invalid Gateway address range in 1st digit. It should be 0-254.');
		document.dhcpd.ip.value = document.dhcpd.ip.defaultValue;
		document.dhcpd.ip.focus();
		return false;
  	}
  	if ( !checkDigitRange(document.dhcpd.ip.value,2,0,255) ) {
  	    	alert('Invalid Gateway address range in 2nd digit. It should be 0-255.');
		document.dhcpd.ip.value = document.dhcpd.ip.defaultValue;
		document.dhcpd.ip.focus();
		return false;
  	}
  	if ( !checkDigitRange(document.dhcpd.ip.value,3,0,255) ) {
  	    	alert('Invalid Gateway address range in 3rd digit. It should be 0-255.');
		document.dhcpd.ip.value = document.dhcpd.ip.defaultValue;
		document.dhcpd.ip.focus();
		return false;
  	}
  	if ( !checkDigitRange(document.dhcpd.ip.value,4,1,254) ) {
  	    	alert('Invalid Gateway address range in 4th digit. It should be 1-254.');
		document.dhcpd.ip.value = document.dhcpd.ip.defaultValue;
		document.dhcpd.ip.focus();
		return false;
  	}*/
  	if (en_dnsopt && document.dhcpd.dhcpdns[1].checked) {
		if (document.dhcpd.dns1.value=="") {
			alert("Enter DNS value !");
			document.dhcpd.dhcpdns.value = document.dhcpd.dhcpdns.defaultValue;
			document.dhcpd.dns1.value = document.dhcpd.dns1.defaultValue;
			document.dhcpd.dns1.focus();
			return false;
		}
		if (!checkHostIP(document.dhcpd.dns1, 1)) {
			document.dhcpd.dns1.value = document.dhcpd.dns1.defaultValue;
			document.dhcpd.dns1.focus();
			return false;
		}

		if (document.dhcpd.dns2.value!="") {
			if (!checkHostIP(document.dhcpd.dns2, 0)) {
				document.dhcpd.dns2.value = document.dhcpd.dns2.defaultValue;
				document.dhcpd.dns2.focus();
				return false;
			}
			if (document.dhcpd.dns3.value!="") {
				if (!checkHostIP(document.dhcpd.dns3, 0)) {
					document.dhcpd.dns3.value = document.dhcpd.dns3.defaultValue;
					document.dhcpd.dns3.focus();
					return false;
				}
			}
		}
	}

	return true;
}


function dhcpTblClick(url) {
	openWindow(url, 'DHCPTbl' );
}

function ShowIP2(ipVal) {
	document.write(getDigit(ipVal,1));
	document.write('.');
	document.write(getDigit(ipVal,2));
	document.write('.');
	document.write(getDigit(ipVal,3));
	document.write('.');
}

function ShowIP(ipVal) {

	var str;

	str = getDigit(ipVal, 1) + '.';
	str += getDigit(ipVal, 2) + '.';
	str += getDigit(ipVal, 3) + '.';

	return str;
}

function macIpClick(url)
{
	var wide=600;
	var high=400;
	if (document.all)
		var xMax = screen.width, yMax = screen.height;
	else if (document.layers)
		var xMax = window.outerWidth, yMax = window.outerHeight;
	else
	   var xMax = 640, yMax=480;
	var xOffset = (xMax - wide)/2;
	var yOffset = (yMax - high)/3;

	var settings = 'width='+wide+',height='+high+',screenX='+xOffset+',screenY='+yOffset+',top='+yOffset+',left='+xOffset+', resizable=yes, toolbar=no,location=no,directories=no,status=no,menubar=no,scrollbars=yes';

	window.open( url, 'MACIPTbl', settings );
}

function enabledhcpd()
{
	document.dhcpd.dhcpdenable[2].checked = true;
	//ip = ShowIP(document.dhcpd.lan_ip.value);
	showDhcpSvr(pool_ipprefix);
}

function disabledhcpd()
{
	document.dhcpd.dhcpdenable[0].checked = true;
	showDhcpSvr();
}

function enabledhcprelay()
{
	document.dhcpd.dhcpdenable[1].checked = true;
	showDhcpSvr();
}

</SCRIPT>
</head>

<body>
<blockquote>

<h2><font color="#0000FF">DHCP Settings</font></h2>

<form action=/goform/formDhcpServer method=POST name="dhcpd">

<input type="hidden" name="lan_ip" value=<% getInfo("dhcplan-ip"); %>>
<input type="hidden" name="lan_mask" value=<% getInfo("dhcplan-subnet"); %>>
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><font color="#000000" size=2>
    This page be used to configure DHCP Server and DHCP Relay.
  </tr>
  <tr><hr size=1 noshade align=top></tr>

  <!-- tr><font color="#000000" size=2><b>LAN IP Address: </b>
  	<script>document.write(document.dhcpd.lan_ip.value);</script>&nbsp;&nbsp;
  <b>Subnet Mask: </b>
  	<script>document.write(document.dhcpd.lan_mask.value);</script>
  </tr -->
</table>

<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr>
  <td><font size=2><b>DHCP Mode: </b>
  <!-- input type="radio" name=dhcpdenable value=0 onClick="disabledhcpd()">None&nbsp;&nbsp;
  <input type="radio"name=dhcpdenable value=1 onClick="enabledhcprelay()">DHCP Relay&nbsp;&nbsp;
  <input type="radio"name=dhcpdenable value=2 onClick="enabledhcpd()">DHCP
  Server&nbsp;&nbsp; -->
  <% checkWrite("dhcpMode"); %>

  </td>
  </tr>
</table>

<table border="0" width="500" cellpadding="0" cellspacing="0">
<hr size=2 noshade align=top>
  <tr><td ID="displayDhcpSvr"></td></tr>
</table>

   <br>
      <input type="hidden" value="/dhcpd.asp" name="submit-url">

<script>
	<% initPage("dhcp-mode"); %>
	//ip = ShowIP(document.dhcpd.lan_ip.value);
	showDhcpSvr(pool_ipprefix);
</script>


 </form>
</blockquote>
</body>

</html>
