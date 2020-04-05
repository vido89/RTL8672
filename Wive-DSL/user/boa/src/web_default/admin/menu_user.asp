<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>menu</title>
</head>

<body>
<table border="0" width="100%" font color="#ffffff">
<tr><td><a href="status.asp" target="view">Status</a></td></tr>

<!--
<tr><td><a href="/adv/adsl-drv.asp" target="view">ADSL Driver</a></td></tr>
-->

<!--
<tr><td><a href="tcpiplan.asp" target="view">LAN Interface</a></td></tr>
-->

<% wlanMenu("0"); %>

<tr><td><b>WAN Interface</b></td></tr>
<tr><td>&nbsp;&nbsp;<a href="wanadsl_user.asp" target="view">Channel Config</a></td></tr>


<!--
<tr><td><b>WAN Interface</b></td></tr>
<tr><td>&nbsp;&nbsp;<a href="wanadsl.asp" target="view">Channel Config</a></td></tr>
<tr><td>&nbsp;&nbsp;<a href="wanatm.asp" target="view">ATM Settings</a></td></tr>
<tr><td>&nbsp;&nbsp;<a href="/adv/adsl-set.asp" target="view">ADSL Settings</a></td></tr>
-->


<tr><td><b>Firewall</b></td></tr>
<tr><td>&nbsp;&nbsp;<a href="fw-macfilter.asp" target="view">MAC Filtering</a></td></tr>


<!--
<tr><td><b>Services</b></td></tr>
<tr><td>&nbsp;&nbsp;<a href="dhcpmode.asp" target="view">DHCP Mode</a></td></tr>
<tr><td>&nbsp;&nbsp;<a href="dhcpd.asp" target="view">DHCP Server</a></td></tr>
<tr><td>&nbsp;&nbsp;<a href="dhcrelay.asp" target="view">DHCP Relay</a></td></tr>
<tr><td>&nbsp;&nbsp;<a href="dns.asp" target="view">DNS</a></td></tr>
-->



<!--
<tr><td>&nbsp;&nbsp;<b>Firewall</b></td></tr>
<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<a href="fw-ipportfilter.asp" target="view">IP/Port Filtering</a></td></tr>
<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<a href="fw-macfilter.asp" target="view">MAC Filtering</a></td></tr>
<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<a href="fw-portfw.asp" target="view">Port Forwarding</a></td></tr>
<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<a href="fw-dmz.asp" target="view">DMZ</a></td></tr>

<tr><td>&nbsp;&nbsp;<a href="igmproxy.asp" target="view">IGMP Proxy</a></td></tr>
<tr><td>&nbsp;&nbsp;<a href="rip.asp" target="view">RIP</a></td></tr>
-->



<!--
<tr><td><b>Advance</b></td></tr>
<tr><td>&nbsp;&nbsp;<a href="bridging.asp" target="view">Bridging</a></td></tr>
<tr><td>&nbsp;&nbsp;<a href="routing.asp" target="view">Routing</a></td></tr>
<tr><td>&nbsp;&nbsp;<a href="snmp.asp" target="view">SNMP</a></td></tr>

<tr><td>&nbsp;&nbsp;<b>Multi-port</b></td></tr>
<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<a href="mpmode.asp" target="view">Admin Status</a></td></tr>
<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<a href="eth2pvc.asp" target="view">Port Mapping</a></td></tr>
<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<a href="vlancfg.asp.asp" target="view">VLan Setting</a></td></tr>
<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<a href="ipqos.asp" target="view">IP QoS</a></td></tr>
<tr><td>&nbsp;&nbsp;&nbsp;&nbsp;<a href="lnkmode.asp" target="view">Link Mode</a></td></tr>

<tr><td>&nbsp;&nbsp;<a href="rmtacc.asp" target="view">Remote Access</a></td></tr>
-->

<!--
<tr><td>&nbsp;&nbsp;<a href="others.asp" target="view">Others</a></td></tr>
-->




<!--
<tr><td><b>Diagnostic</b></td></tr>
<tr><td>&nbsp;&nbsp;<a href="ping.asp" target="view">Ping</a></td></tr>
<tr><td>&nbsp;&nbsp;<a href="oamloopback.asp" target="view">ATM Loopback</a></td></tr>
<tr><td>&nbsp;&nbsp;<a href="/adv/adsl-diag.asp" target="view">ADSL</a></td></tr>
-->



<tr><td><b>Admin</b></td></tr>
<tr><td>&nbsp;&nbsp;<a href="reboot.asp", target="view">Commit/Reboot</a></td></tr>
<!--
<tr><td>&nbsp;&nbsp;<a href="saveconf.asp" target="view">Backup/Restore</a></td></tr>
-->
<tr><td>&nbsp;&nbsp;<a href="user-password.asp" target="view">Password</a></td></tr>
<!--
<tr><td>&nbsp;&nbsp;<a href="upload.asp" target="view">Upgrade Firmware</a></td></tr>
<tr><td>&nbsp;&nbsp;<a href="rconfig.asp" target="view">Remote Config</a></td></tr>
-->

<tr><td>&nbsp;&nbsp;<a href="acl.asp" target="view">ACL Config</a></td></tr>

<!--xl_yue add   -->
<% userAddAdminMenu("0"); %>




<!--
<tr><td><b>Statistics</b></td></tr>
<tr><td>&nbsp;&nbsp;<a href="stats.asp" target="view">Interfaces</a></td></tr>
<tr><td>&nbsp;&nbsp;<a href="/adv/adsl-statis.asp" target="view">ADSL</a></td></tr>
-->



</table>

</body>
</html>
