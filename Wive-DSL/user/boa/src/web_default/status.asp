<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<META HTTP-EQUIV=Refresh CONTENT="10; URL=status.asp">
<meta http-equiv="Content-Type" content="text/html">
<title>ADSL Router Status</title>
<script type="text/javascript" src="share.js">
</script>
<script>
var getObj = null;
function modifyClick(url)
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

	window.open( url, 'Status_Modify', settings );
}

function disButton(id)
{
       getObj = document.getElementById(id);

       window.setTimeout("getObj.disabled=true", 100);
	return false;
}

</script>
</head>
<body>
<blockquote>

<h2><b><font color="#0000FF">ADSL Router Status</font></b></h2>

<table border=0 width="400" cellspacing=0 cellpadding=0>
<tr><td><font size=2>
 This page shows the current status and some basic settings of the device.
</font></td></tr>

<tr><td><hr size=1 noshade align=top><br></td></tr>
</table>


<table width=400 border=0">
  <tr>
    <td width=100% colspan="2" bgcolor="#008000"><font color="#FFFFFF" size=2><b>System</b></font></td>
  </tr>
  <tr bgcolor="#EEEEEE">
    <td width=40%><font size=2><b>Product Name</b></td>
    <td width=60%><font size=2><% getInfo("name"); %></td>
  </tr>
  <tr bgcolor="#DDDDDD">
    <td width=40%><font size=2><b>Uptime</b></td>
    <td width=60%><font size=2><% getInfo("uptime"); %></td>
  </tr>
<!--
  <tr bgcolor="#EEEEEE">
    <td width=40%><font size=2><b>Date/Time</b></td>
    <td width=60%><font size=2><% getInfo("date"); %></td>
  </tr>
-->
  <tr bgcolor="#EEEEEE">
    <td width=40%><font size=2><b>Firmware Version</b></td>
    <td width=60%><font size=2><% getInfo("fwVersion"); %></td>
  </tr>
  <tr bgcolor="#DDDDDD">
    <td width=40%><font size=2><b>DSP Version</b></td>
    <td width=60%><font size=2><% getInfo("adsl-drv-version"); %></td>
  </tr>
   <tr bgcolor="#EEEEEE">
    <td width=40%><font size=2><b>Name Servers</b></td>
    <td width=60%><font size=2><% getNameServer(); %></td>
  </tr>
  <tr bgcolor="#DDDDDD">
    <td width=40%><font size=2><b>Default Gateway</b></td>
    <td width=60%><font size=2><% getDefaultGW(); %></td>
  </tr>
  <tr>
    <td width=100% colspan="2" bgcolor="#008000"><font color="#FFFFFF" size=2><b>DSL</b></font></td>
  </tr>
  <tr bgcolor="#EEEEEE">
    <td width=40%><font size=2><b>Operational Status</b></td>
    <td width=60%><font size=2><% getInfo("dslstate"); %></td>
  </tr>
  <tr bgcolor="#DDDDDD">
    <td width=40%><font size=2><b>Upstream Speed</b></td>
    <td width=60%><font size=2><% getInfo("adsl-drv-rate-us"); %>&nbsp;kbps
       &nbsp;</td>
  </tr>
  <tr bgcolor="#EEEEEE">
    <td width=40%><font size=2><b>Downstream Speed</b></td>
    <td width=60%><font size=2><% getInfo("adsl-drv-rate-ds"); %>&nbsp;kbps
       &nbsp;</td>
  </tr>
  <tr>
    <td width=100% colspan="2" bgcolor="#008000"><font color="#FFFFFF" size=2><b>LAN Configuration</b></font></td>
  </tr>
  <tr bgcolor="#EEEEEE">
    <td width=40%><font size=2><b>IP Address</b></td>
    <td width=60%><font size=2><% getInfo("lan-ip"); %></td>
  </tr>
  <tr bgcolor="#DDDDDD">
    <td width=40%><font size=2><b>Subnet Mask</b></td>
    <td width=60%><font size=2><% getInfo("lan-subnet"); %></td>
  </tr>
  <tr bgcolor="#EEEEEE">
    <td width=40%><font size=2><b>DHCP Server</b></td>
    <td width=60%><font size=2>
      <%  checkWrite("lan-dhcp-st");
      %></td>
  </tr>
  <tr bgcolor="#DDDDDD">
    <td width=40%><font size=2><b>MAC Address</b></td>
    <td width=60%><font size=2><% getInfo("elan-Mac"); %></td>
  </tr>
</table>
<br>
<form action=/goform/admin/formStatus method=POST name="status">
<table width=600 border=0">
 <tr>
    <td width=100% colspan=7 bgcolor="#008000"><font color="#FFFFFF" size=2><b>WAN Configuration</b></font></td>
  </tr>
  <% wanConfList(); %>
</table>
  <input type="hidden" value="/admin/status.asp" name="submit-url">
  <input type="submit" value="Refresh" name="refresh">&nbsp;&nbsp;
  <!--
  <input type="button" value="Modify" name="modify" onClick="modifyClick('/admin/date.asp')">
  --> 
</form>

</blockquote>

</body>

</html>
