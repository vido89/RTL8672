<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>menu</title>
</head>

<body>
<table border="0" width="100%" font color="#ffffff" bgcolor="#dadfdb">
<tr><td><a href="/admin/status.asp" target="view">Status</a></td></tr>

<!--
<tr><td><a href="/adv/adsl-drv.asp" target="view">ADSL Driver</a></td></tr>
-->

<tr><td><a href="tcpiplan.asp" target="view">LAN Interface</a></td></tr>

<% wlanMenu("0"); %>

<tr><td><b>WAN Interface</b></td></tr>
<tr><td>&nbsp;&nbsp;<a href="wanadsl.asp" target="view">Channel Config</a></td></tr>
<tr><td>&nbsp;&nbsp;<a href="wanatm.asp" target="view">ATM Settings</a></td></tr>
<tr><td>&nbsp;&nbsp;<a href="/adv/adsl-set.asp" target="view">ADSL Settings</a></td></tr>

<% srvMenu("0"); %>

<tr><td><b>Advance</b></td></tr>
<tr><td>&nbsp;&nbsp;<a href="arptable.asp" target="view">ARP Table</a></td></tr>
<tr><td>&nbsp;&nbsp;<a href="bridging.asp" target="view">Bridging</a></td></tr>
<tr><td>&nbsp;&nbsp;<a href="routing.asp" target="view">Routing</a></td></tr>
<tr><td>&nbsp;&nbsp;<a href="snmp.asp" target="view">SNMP</a></td></tr>

<% vportMenu("0"); %>

<tr><td>&nbsp;&nbsp;<a href="rmtacc.asp" target="view">Remote Access</a></td></tr>
<tr><td>&nbsp;&nbsp;<a href="others.asp" target="view">Others</a></td></tr>

<% diagMenu("0"); %>

<% adminMenu("0"); %>

<tr><td><b>Statistics</b></td></tr>
<tr><td>&nbsp;&nbsp;<a href="stats.asp" target="view">Interfaces</a></td></tr>
<tr><td>&nbsp;&nbsp;<a href="/adv/adsl-statis.asp" target="view">ADSL</a></td></tr>
</table>

</body>
</html>
