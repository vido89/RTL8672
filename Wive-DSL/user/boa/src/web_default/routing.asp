<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Routing Configuration </title>
<script type="text/javascript" src="share.js">
</script>
<% language=javascript %>
<SCRIPT>

function postGW( enable, destNet, subMask, nextHop, metric, interface, select )
{
	document.route.enable.checked = enable;
	document.route.destNet.value=destNet;
	document.route.subMask.value=subMask;
	document.route.nextHop.value=nextHop;
	document.route.metric.value=metric;
	document.route.interface.value=interface;	
	document.route.select_id.value=select;	
}

function checkDest(ip, mask)
{
	var i, dip, dmask, nip;

	for (i=1; i<=4; i++) {
		dip = getDigit(ip.value, i);
		dmask = getDigit(mask.value,  i);
		nip = dip & dmask;
		if (nip != dip)
			return true;
	}
	return false;
}

function addClick()
{
	/*if (document.route.destNet.value=="") {
		alert("Enter Destination Network ID !");
		document.route.destNet.focus();
		return false;
	}
	
	if ( validateKey( document.route.destNet.value ) == 0 ) {
		alert("Invalid Destination value.");
		document.route.destNet.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.destNet.value,1,0,255) ) {
		alert('Invalid Destination range in 1st digit. It should be 0-255.');
		document.route.destNet.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.destNet.value,2,0,255) ) {
		alert('Invalid Destination range in 2nd digit. It should be 0-255.');
		document.route.destNet.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.destNet.value,3,0,255) ) {
		alert('Invalid Destination range in 3rd digit. It should be 0-255.');
		document.route.destNet.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.destNet.value,4,0,254) ) {
		alert('Invalid Destination range in 4th digit. It should be 0-254.');
		document.route.destNet.focus();
		return false;
	}
	
	if (document.route.subMask.value=="") {
		alert("Enter Subnet Mask !");
		document.route.subMask.focus();
		return false;
	}
	
	if ( validateKey( document.route.subMask.value ) == 0 ) {
		alert("Invalid Subnet Mask value.");
		document.route.subMask.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.subMask.value,1,0,255) ) {
		alert('Invalid Subnet Mask range in 1st digit. It should be 0-255.');
		document.route.subMask.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.subMask.value,2,0,255) ) {
		alert('Invalid Subnet Mask range in 2nd digit. It should be 0-255.');
		document.route.subMask.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.subMask.value,3,0,255) ) {
		alert('Invalid Subnet Mask range in 3rd digit. It should be 0-255.');
		document.route.subMask.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.subMask.value,4,0,255) ) {
		alert('Invalid Subnet Mask range in 4th digit. It should be 0-255.');
		document.route.subMask.focus();
		return false;
	}
	if (document.route.interface.value==255) {
	if (document.route.nextHop.value=="" ) {
		alert("Enter Next Hop IP or select a GW interface!");
		document.route.nextHop.focus();
		return false;
	}
	
	if ( validateKey( document.route.nextHop.value ) == 0 ) {
		alert("Invalid Next Hop value.");
		document.route.nextHop.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.nextHop.value,1,0,255) ) {
		alert('Invalid Next Hop range in 1st digit. It should be 0-255.');
		document.route.nextHop.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.nextHop.value,2,0,255) ) {
		alert('Invalid Next Hop range in 2nd digit. It should be 0-255.');
		document.route.nextHop.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.nextHop.value,3,0,255) ) {
		alert('Invalid Next Hop range in 3rd digit. It should be 0-255.');
		document.route.nextHop.focus();
		return false;
	}
	if ( !checkDigitRange(document.route.nextHop.value,4,1,254) ) {
		alert('Invalid Next Hop range in 4th digit. It should be 1-254.');
		document.route.nextHop.focus();
		return false;
	}*/
	if (checkDest(document.route.destNet, document.route.subMask) == true) {
		alert('The specified mask parameter is invalid. (Destination & Mask) != Destination.');
		document.route.subMask.focus();
		return false;
	}
	if (!checkHostIP(document.route.destNet, 1))
		return false;
	if (!checkNetmask(document.route.subMask, 1))
		return false;
	if (document.route.interface.value==255) {
		if (document.route.nextHop.value=="" ) {
			alert("Enter Next Hop IP or select a GW interface!");
			document.route.nextHop.focus();
			return false;
		}

		if (!checkHostIP(document.route.nextHop, 0))
			return false;
	}
	if ( !checkDigitRange(document.route.metric.value,1,0,16) ) {
		alert('Invalid Metric range. It should be 0~16.');
		document.route.metric.focus();
		return false;
	}
	
	return true;
}

function routeClick(url)
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

	window.open( url, 'RouteTbl', settings );
}
	
</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">Routing Configuration</font></h2>

<form action=/goform/formRouting method=POST name="route">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><font size=2>
    This page is used to configure the routing information. Here you can add/delete
 IP routes.
  </tr>
</table>
		<!--
		<% ShowDefaultGateway("autort"); %>
		<% ShowRouteInf("rt"); %>
		-->
<table border=0 width="600" cellspacing=4 cellpadding=0>
  <tr><hr size=1 noshade align=top></tr>
  <tr>
      <td width="30%"><font size=2><b>Enable:</b></td>
      <td width="70%"><input type="checkbox" name="enable" value="1" checked></td>
  </tr>
  <tr>
      <td width="30%"><font size=2><b>Destination:</b></td>
      <td width="70%"><input type="text" name="destNet" size="15" maxlength="15"></td>
  </tr>
  <tr>
      <td width="30%"><font size=2><b>Subnet Mask:</b></td>
      <td width="70%"><input type="text" name="subMask" size="15" maxlength="15"></td>
  </tr>
  <tr>
      <td width="30%"><font size=2><b>Next Hop:</b></td>
      <td width="70%"><input type="text" name="nextHop" size="15" maxlength="15"></td>
  </tr>
  <tr>
      <td width="30%"><font size=2><b>Metric:</b></td>
      <td width="70%"><input type="text" name="metric" size="5" maxlength="5"></td>
  </tr>
  <tr>
      <td width="30%"><font size=2><b>Interface:</b></td>
      <td width="70%"><select name="interface">
          <%  if_wan_list("rt-any");%>
      	</select></td>
  </tr>
  <input type="hidden" value="" name="select_id">
</table>
  <input type="submit" value="Add Route" name="addRoute" onClick="return addClick()">&nbsp;&nbsp;
  <input type="submit" value="Update" name="updateRoute" onClick="return addClick()">&nbsp;&nbsp;
  <input type="submit" value="Delete Selected" name="delRoute" onClick="return deleteClick()">&nbsp;&nbsp;
  <input type="button" value="Show Routes" name="showRoute" onClick="routeClick('/routetbl.asp')">
  </tr>
  <tr><hr size=1 noshade align=top></tr>
<table border=0 width="600" cellspacing=4 cellpadding=0>
  <tr><font size=2><b>Static Route Table:</b></font></tr>
  <% showStaticRoute(); %>
</table>
  <br>
      <input type="hidden" value="/routing.asp" name="submit-url">
		<!--
		<% GetDefaultGateway(); %>
		-->
</form>
</blockquote>
</body>

</html>
