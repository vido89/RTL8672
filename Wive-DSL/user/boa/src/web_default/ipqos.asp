<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>IP QoS</title>
<script type="text/javascript" src="share.js">
</script>
<script>
function adminClick()
{
	var i, num;
	num = document.qos.elements.length;
	if (document.qos.qosen[0].checked) {
		for (i=2; i<num; i++) {
			document.qos[i].disabled = true;
		}
		document.qos.admin.disabled = false;
		document.qos[num-1].disabled = false;
	}
	else {
		for (i=2; i<num; i++) {
			document.qos[i].disabled = false;
		}
		if (document.qos.prot.value != 1 && document.qos.prot.value != 2) {
			document.qos.sport.disabled = true;
			document.qos.dport.disabled = true;
		}
		if (document.qos.dscpenable[0].checked) {
			document.qos.dscp.disabled = true;
			document.qos.ipprio.disabled = false;
			document.qos.tos.disabled = false;
		}
		else {
			document.qos.dscp.disabled = false;
			document.qos.ipprio.disabled = true;
			document.qos.tos.disabled = true;
		}
	}
}               
                
function addClick()
{               
	if (document.qos.sip.value == "" && document.qos.smask.value == "" && document.qos.sport.value == ""
		&& document.qos.dip.value == "" && document.qos.dmask.value == "" && document.qos.dport.value == ""
		&& document.qos.prot.value == 0 && document.qos.phyport.value == 0) {
		alert("Traffic Classification Rules can't be empty");
		document.qos.sip.focus();
		return false;
	}

	//var i;  
	        
	if ( document.qos.sip.value!="" ) {
		if (!checkHostIP(document.qos.sip, 0))
			return false;
		if ( document.qos.smask.value != "" ) {
			if (!checkNetmask(document.qos.smask, 0))
				return false;
		}
		/*if ( validateKey( document.qos.sip.value ) == 0 ) {
			alert("Invalid source IP address.");
			document.qos.sip.focus();
			return false;
		}
		for (i=1; i<=4; i++) {
			if ( !checkDigitRange(document.qos.sip.value,i,0,255) ) {
				alert('Invalid source IP address.');
				document.qos.sip.focus();
				return false;
			}
		}
		
		if ( document.qos.smask.value!="" ) {
			if ( validateKey( document.qos.smask.value ) == 0 ) {
				alert("Invalid source IP mask.");
				document.qos.smask.focus();
				return false;
			}
			for (i=1; i<=4; i++) {
				if ( !checkDigitRange(document.qos.smask.value,i,0,255) ) {
					alert('Invalid source IP mask.');
					document.qos.smask.focus();
					return false;
				}
			}
		}*/
	}
	
	if ( document.qos.dip.value!="" ) {
		if (!checkHostIP(document.qos.dip, 0))
			return false;
		if ( document.qos.dmask.value != "" ) {
			if (!checkNetmask(document.qos.dmask, 0))
				return false;
		}
		/*if ( validateKey( document.qos.dip.value ) == 0 ) {
			alert("Invalid destination IP address.");
			document.qos.dip.focus();
			return false;
		}
		for (i=1; i<=4; i++) {
			if ( !checkDigitRange(document.qos.dip.value,i,0,255) ) {
				alert('Invalid destinationIP address.');
				document.qos.dip.focus();
				return false;
			}
		}
		
		if ( document.qos.dmask.value!="" ) {
			if ( validateKey( document.qos.dmask.value ) == 0 ) {
				alert("Invalid destination IP mask.");
				document.qos.dmask.focus();
				return false;
			}
			for (i=1; i<=4; i++) {
				if ( !checkDigitRange(document.qos.dmask.value,i,0,255) ) {
					alert('Invalid destination IP mask.');
					document.qos.dmask.focus();
					return false;
				}
			}
		}*/
	}
	
	if ( document.qos.sport.value!="" ) {
		if ( validateKey( document.qos.sport.value ) == 0 ) {
			alert("Invalid source port!");
			document.qos.sport.focus();
			return false;
		}
		
		d1 = getDigit(document.qos.sport.value, 1);
		if (d1 > 65535 || d1 < 1) {
			alert("Invalid source port number!");
			document.qos.sport.focus();
			return false;
		}
	}
	
	if ( document.qos.dport.value!="" ) {
		if ( validateKey( document.qos.dport.value ) == 0 ) {
			alert("Invalid destination port!");
			document.qos.dport.focus();
			return false;
		}
		
		d1 = getDigit(document.qos.dport.value, 1);
		if (d1 > 65535 || d1 < 1) {
			alert("Invalid destination port number!");
			document.qos.dport.focus();
			return false;
		}
	}
	
	return true;
}

function dscpClick()
{
	if (document.qos.dscpenable[0].checked) {
		document.qos.dscp.disabled = true;
		document.qos.ipprio.disabled = false;
		document.qos.tos.disabled = false;
	}
	else {
		document.qos.dscp.disabled = false;
		document.qos.ipprio.disabled = true;
		document.qos.tos.disabled = true;
	}
}               
</script>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">IP QoS</font></h2>

<table border=0 width=600 cellspacing=4 cellpadding=0>
<tr><td colspan=4><font size=2>
 Entries in this table are used to assign the precedence for each incoming packet
 based on physical LAN port, TCP/UDP port number, and source/destination
 IP address/subnet masks.
</font></td></tr>

<tr><td colspan=4><hr size=1 noshade align=top></td></tr>

<form action=/goform/formIPQoS method=POST name=qos>

<tr>
	<td><font size=2><b>IP QoS:</b>
		<input type="radio" name=qosen value=0 onClick="return adminClick()">Disabled&nbsp;&nbsp;
		<input type="radio" name=qosen value=1 onClick="return adminClick()">Enabled
	</td>
		<% dft_qos(); %>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
	<td><input type=submit value="Apply Changes" name=admin></td>
</tr>
</table>
<table border=0 width=600 cellspacing=4 cellpadding=0>
<tr><td colspan=4><hr size=1 noshade align=top></td></tr>
<tr>
<th colspan=4><u>Specify Traffic Classification Rules</u></th>
</tr>
<tr>
	<td colspan=4>
		<align=left><font size=2><b>Source IP:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; </b>
		<align=left><input type=text name=sip size=20 maxlength=15>
		<align=left><font size=2><b>Netmask: </b>
		<align=left><input type=text name=smask size=20 maxlength=15>
		<align=left><font size=2><b>Port: </b>
		<align=left><input type=text name=sport size=6 maxlength=5>
	</td>
</tr>
<tr>
	<td colspan=4><font size=2><b>Destination IP: </b>
		<align=left><input type=text name=dip size=20 maxlength=15>
		<align=left><font size=2><b>Netmask: </b>
		<align=left><input type=text name=dmask size=20 maxlength=15>
		<align=left><font size=2><b>Port: </b>
		<align=left><input type=text name=dport size=6 maxlength=5>
	</td>
</tr>
<tr>
	<td colspan=4><font size=2><b>Protocol:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; </b>
	<align=left><select name=prot onClick="return adminClick()">
		<option value=0></option>
		<option value=1>TCP</option>
		<option value=2>UDP</option>
		<option value=3>ICMP</option>
	</select>
	<align=left><b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Physical Port:</b>
	<align=left><font size=2><select name=phyport>
		<option value=0></option>
		<% if_lan_list("all"); %>
	</select>
	</td>
</tr>
	<% pr_egress(); %>
<tr><th colspan=4>&nbsp;</th></tr>
<tr>
<th colspan=4><u>Assign Priority and/or IP Precedence and/or Type of Service and/or DSCP</u></th>
</tr>
<tr>
	<td align=left><font size=2><b>Outbound Priority:</b></td>
	<td align=left>
	<% pq_egress(); %>
	</td>
	<td align=left><font size=2><b>802.1p:</b>
	<select name=m1p>
		<option value=0></option>
		<option value=1>0</option>
		<option value=2>1</option>
		<option value=3>2</option>
		<option value=4>3</option>
		<option value=5>4</option>
		<option value=6>5</option>
		<option value=7>6</option>
		<option value=8>7</option>
	</select>
	</td>
</tr>
<% mark_dscp(); %>
<!--tr>
	<td align=left><font size=2><b>IP Precedence:</b></td>
	<td align=left><select name=ipprio>
		<option value=0></option>
		<option value=1>0</option>
		<option value=2>1</option>
		<option value=3>2</option>
		<option value=4>3</option>
		<option value=5>4</option>
		<option value=6>5</option>
		<option value=7>6</option>
		<option value=8>7</option>
	</select>
	</td>
	<td align=left><font size=2><b>IP Type of Service:</b></td>
	<td align=left><select name=tos>
		<option value=255></option>
		<option value=0>Normal Service</option>
		<option value=2>Minimize Cost</option>
		<option value=4>Maximize Reliability</option>
		<option value=8>Maximize Throughput</option>
		<option value=16>Minimize Delay</option>
	</select>
	</td>
</tr-->
<tr>
	<td><input type="submit" value="Add" name=addqos onClick="return addClick()"></td>
	<td><input type="hidden" value="/ipqos.asp" name="submit-url"></td>
</tr>
</form>
</table>

<br>
<form action=/goform/formIPQoS method=POST name=qostbl>
  <table border="0" width=600>
  <tr><font size=2><b>IP QoS Rules:</b></font></tr>
  <% ipQosList(); %>
  </table>
  <br>
  <input type="submit" value="Delete Selected" name=delSel onClick="return deleteClick()">&nbsp;&nbsp;
  <input type="submit" value="Delete All" name=delAll onClick="return deleteAllClick()">&nbsp;&nbsp;&nbsp;
  <input type="hidden" value="/ipqos.asp" name="submit-url">
</form>
<script>
	<% initPage("ipqos"); %>
	adminClick();
</script>
</blockquote>
</body>
</html>
