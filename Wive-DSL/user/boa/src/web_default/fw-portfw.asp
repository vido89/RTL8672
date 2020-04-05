<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Port Forwarding</title>
<script type="text/javascript" src="share.js">
</script>
<script>
function skip () { this.blur(); }

/*function postFW( localIP, localport_from, localport_to, protocol, comment, enable, remoteIP, remotePort, interface, select )
{
	if ( document.formPortFwAdd.enabled.checked==true )
	{
		document.formPortFwAdd.ip.value=localIP;
		document.formPortFwAdd.toPort.value=localport_to;
		document.formPortFwAdd.protocol.value=protocol;
		document.formPortFwAdd.comment.value=comment;
		if( enable==0 )
			document.formPortFwAdd.fw_enable.checked=false;
		else
			document.formPortFwAdd.fw_enable.checked=true;
			
		document.formPortFwAdd.remoteIP.value=remoteIP;
		document.formPortFwAdd.remotePort.value=remotePort;
		document.formPortFwAdd.interface.value=interface;
		document.formPortFwAdd.select_id.value=select;

	}
}*/

function addClick()
{
//  if (!document.formPortFwAdd.enabled.checked)
  if (document.formPortFwAdd.portFwcap[0].checked)
  	return true;
	
  if (document.formPortFwAdd.ip.value=="" && document.formPortFwAdd.fromPort.value=="" &&
	document.formPortFwAdd.toPort.value=="" && document.formPortFwAdd.comment.value=="" ) {
	alert("Local settings cannot be empty!");
	return false;
//	return true;
  }

  /*if (document.formPortFwAdd.ip.value=="") {
	alert("IP address cannot be empty! It should be filled with 4 digit numbers as xxx.xxx.xxx.xxx.");
	document.formPortFwAdd.ip.focus();
	return false;
  }
  if ( validateKey( document.formPortFwAdd.ip.value ) == 0 ) {
	alert("Invalid IP address value. It should be the decimal number (0-9).");
	document.formPortFwAdd.ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formPortFwAdd.ip.value,1,0,255) ) {
      	alert('Invalid IP address range in 1st digit. It should be 0-255.');
	document.formPortFwAdd.ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formPortFwAdd.ip.value,2,0,255) ) {
      	alert('Invalid IP address range in 2nd digit. It should be 0-255.');
	document.formPortFwAdd.ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formPortFwAdd.ip.value,3,0,255) ) {
      	alert('Invalid IP address range in 3rd digit. It should be 0-255.');
	document.formPortFwAdd.ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formPortFwAdd.ip.value,4,1,254) ) {
      	alert('Invalid IP address range in 4th digit. It should be 1-254.');
	document.formPortFwAdd.ip.focus();
	return false;
  }*/
  if (!checkHostIP(document.formPortFwAdd.ip, 1))
	return false;
  if ( document.formPortFwAdd.remoteIP.value!="" ) {
  	if (!checkHostIP(document.formPortFwAdd.remoteIP, 0))
		return false;
  }

  if (document.formPortFwAdd.fromPort.value!="") {
  	if ( validateKey( document.formPortFwAdd.fromPort.value ) == 0 ) {
		alert("Invalid port number! It should be the decimal number (0-9).");
		document.formPortFwAdd.fromPort.focus();
		return false;
  	}
	d2 = getDigit(document.formPortFwAdd.fromPort.value, 1);
 	if (d2 > 65535 || d2 < 1) {
		alert("Invalid port number! You should set a value between 1-65535.");
		document.formPortFwAdd.fromPort.focus();
		return false;
  	}
   }
  if (document.formPortFwAdd.remoteFromPort.value!="") {
  	if ( validateKey( document.formPortFwAdd.remoteFromPort.value ) == 0 ) {
		alert("Invalid port number! It should be the decimal number (0-9).");
		document.formPortFwAdd.remoteFromPort.focus();
		return false;
  	}
	d2 = getDigit(document.formPortFwAdd.remoteFromPort.value, 1);
 	if (d2 > 65535 || d2 < 1) {
		alert("Invalid port number! You should set a value between 1-65535.");
		document.formPortFwAdd.remoteFromPort.focus();
		return false;
  	}
   }

  if (document.formPortFwAdd.toPort.value!="") {
  	if ( validateKey( document.formPortFwAdd.toPort.value ) == 0 ) {
		alert("Invalid port number! It should be the decimal number (0-9).");
		document.formPortFwAdd.toPort.focus();
		return false;
  	}
	d2 = getDigit(document.formPortFwAdd.toPort.value, 1);
 	if (d2 > 65535 || d2 < 1) {
		alert("Invalid port number! You should set a value between 1-65535.");
		document.formPortFwAdd.toPort.focus();
		return false;
  	}
   }
  if (document.formPortFwAdd.remoteToPort.value!="") {
  	if ( validateKey( document.formPortFwAdd.remoteToPort.value ) == 0 ) {
		alert("Invalid port number! It should be the decimal number (0-9).");
		document.formPortFwAdd.remoteToPort.focus();
		return false;
  	}
	d2 = getDigit(document.formPortFwAdd.remoteToPort.value, 1);
 	if (d2 > 65535 || d2 < 1) {
		alert("Invalid port number! You should set a value between 1-65535.");
		document.formPortFwAdd.remoteToPort.focus();
		return false;
  	}
   }
   return true;
}

function disableDelButton()
{
  if (verifyBrowser() != "ns") {
	disableButton(document.formPortFwDel.deleteSelPortFw);
	disableButton(document.formPortFwDel.deleteAllPortFw);	
  }
}

function updateState()
{
//  if (document.formPortFwAdd.enabled.checked) {
  if (document.formPortFwAdd.portFwcap[1].checked) {
 	enableTextField(document.formPortFwAdd.ip);
	enableTextField(document.formPortFwAdd.protocol);
	enableTextField(document.formPortFwAdd.fromPort);
	enableTextField(document.formPortFwAdd.toPort);
	enableTextField(document.formPortFwAdd.comment);
	enableTextField(document.formPortFwAdd.remoteIP);
//	enableTextField(document.formPortFwAdd.remotePort);
	enableTextField(document.formPortFwAdd.remoteFromPort);
	enableTextField(document.formPortFwAdd.remoteToPort);
	enableTextField(document.formPortFwAdd.interface);
	enableTextField(document.formPortFwAdd.fw_enable);
	enableButton(document.formPortFwAdd.advance);
  }
  else {
 	disableTextField(document.formPortFwAdd.ip);
	disableTextField(document.formPortFwAdd.protocol);
	disableTextField(document.formPortFwAdd.fromPort);
	disableTextField(document.formPortFwAdd.toPort);
	disableTextField(document.formPortFwAdd.comment);
	disableTextField(document.formPortFwAdd.remoteIP);
//	disableTextField(document.formPortFwAdd.remotePort);
	disableTextField(document.formPortFwAdd.remoteFromPort);
	disableTextField(document.formPortFwAdd.remoteToPort);
	disableTextField(document.formPortFwAdd.interface);
	disableTextField(document.formPortFwAdd.fw_enable);
	disableButton(document.formPortFwAdd.advance);

  }
}


function portFWClick(url)
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

	window.open( url, 'portFWTbl', settings );
}


<% portFwTR069("FunctionPostFW"); %>

</script>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">Port Forwarding</font></h2>

<table border=0 width="500" cellspacing=0 cellpadding=0>
<tr><td><font size=2>
 Entries in this table allow you to automatically redirect common network services
 to a specific machine behind the NAT firewall.  These settings are only necessary
 if you wish to host some sort of server like a web server or mail server on the
 private local network behind your Gateway's NAT firewall.
</font></td></tr>

<tr><td><hr size=1 noshade align=top></td></tr>

<form action=/goform/formPortFw method=POST name="formPortFwAdd">

<!--tr><td><font size=2><b>
   	<input type="checkbox" name="enabled" value="ON" <% checkWrite("portFwEn"); %>
   	 ONCLICK=updateState()>&nbsp;&nbsp;Enable Port Forwarding</b><br>
    </td>
</tr-->
<tr><td><font size=2><b>Port Forwarding:</b>
	<input type="radio" value="0" name="portFwcap" <% checkWrite("portFw-cap0"); %> onClick="updateState()">Disable&nbsp;&nbsp;
	<input type="radio" value="1" name="portFwcap" <% checkWrite("portFw-cap1"); %> onClick="updateState()">Enable&nbsp;&nbsp;
	<input type="submit" value="Apply Changes" name="apply">&nbsp;&nbsp;
</font></td></tr>
</table>
<table border=0 width="500" cellspacing=0 cellpadding=0>
	<tr><hr size=1 noshade align=top></tr>
	<tr>
		<td><font size=2>
			<b>Protocol:</b>
				<select name="protocol">
					<option select value=4>Both</option>
					<option value=1>TCP</option>
					<option value=2>UDP</option>
				</select>&nbsp;
			<b>Comment:</b>
				<!--input type="text" name="comment" size="6" maxlength="20">&nbsp;&nbsp;-->
				<input type="text" name="comment" size="6" maxlength="19">&nbsp;&nbsp;
			<input type="checkbox" name="fw_enable" value="1" CHECKED><b>Enable</b>
		</td>
	</tr>
	<tr>
		<td><font size=2><b>Local IP Address:&nbsp;</b>
				<input type="text" name="ip" size="10" maxlength="15">&nbsp;&nbsp;&nbsp;
			<font size=2><b>Local Port:</b>
				<input type="text" name="fromPort" size="3" maxlength="5"><b>-</b>
				<input type="text" name="toPort" size="3" maxlength="5">
		</td>
		<td>
	</tr>
	<tr>
		<td><font size=2><b>Remote IP Address:</b>
				<input type="text" name="remoteIP" size="10" maxlength="15">&nbsp;&nbsp;
			<!--font size=2><b>Public Port:</b> <input type="text" name="remotePort" size="3" maxlength="5"-->
			<font size=2><b>Public Port:</b> <input type="text" name="remoteFromPort" size="3" maxlength="5"><b>-</b>
										   <input type="text" name="remoteToPort" size="3" maxlength="5">
		</td>
	</tr>
	<tr>
		<td><font size=2><b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Interface:</b> 
				<select name="interface">
					<%  if_wan_list("rt-any");%>
				</select>    
				<input type="hidden" value="" name="select_id">
		</td>
		<td>
			<input type="submit" value="Add" name="addPortFw" onClick="return addClick()">
			<!--input type="reset" value="Reset" name="reset"-->
			<input type="hidden" value="/fw-portfw.asp" name="submit-url">			
		</td>
	</tr>	
	<% showPFWAdvForm(); %>
	
<script> updateState(); </script>
</form>
</table>

<form action=/goform/formPortFw method=POST name="formPortFwDel">
<table border=0 width=500>
  <tr><hr size=1 noshade align=top></tr>
  <tr><font size=2><b>Current Port Forwarding Table:</b></font></tr>
  <% portFwList(); %>
</table>

 <br><input type="submit" value="Delete Selected" name="deleteSelPortFw" onClick="return deleteClick()">&nbsp;&nbsp;
     <input type="submit" value="Delete All" name="deleteAllPortFw" onClick="return deleteAllClick()">&nbsp;&nbsp;&nbsp;

 <script>
   	<% checkWrite("portFwNum"); %>
 </script>
     <input type="hidden" value="/fw-portfw.asp" name="submit-url">
</form>

</td></tr></table>

</blockquote>
</body>
</html>
