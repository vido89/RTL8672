<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Active Wireless Client Table</title>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">Active Wireless Client Table</font></h2>


<table border=0 width="500" cellspacing=0 cellpadding=0>
  <tr><font size=2>
 This table shows the MAC address, transmission, reception packet counters and encrypted
 status for each associated wireless client.
  </tr>
  <tr><hr size=1 noshade align=top></tr>
</table>
<form action=/goform/admin/formWirelessTbl method=POST name="formWirelessTbl">
<table border='1' width="500">
<tr bgcolor=#7f7f7f><td width="25%"><font size=2><b>MAC Address</b></td>
<td width="15%"><font size=2><b>Tx Packet</b></td>
<td width="15%"><font size=2><b>Rx Packet</b></td>
<td width="15%"><font size=2><b>Tx Rate (Mbps)</b></td>
<td width="15%"><font size=2><b>Power Saving</b></td>
<td width="15%"><font size=2><b>Expired Time (s)</b></td></tr>
<% wirelessClientList(); %>
</table>

<input type="hidden" value="/admin/wlstatbl.asp" name="submit-url">
  <p><input type="submit" value="Refresh" name="refresh">&nbsp;&nbsp;
  <input type="button" value=" Close " name="close" onClick="javascript: window.close();"></p>
</form>
</blockquote>
</body>

</html>
