<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>WDS AP Table</title>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">WDS AP Table <% if (getIndex("wlan_num") > 1) write("-wlan"+(getIndex("wlan_idx")+1)); %></font></h2>


<table border=0 width="500" cellspacing=0 cellpadding=0>
  <tr><font size=2>
 This table shows the MAC address, transmission, reception packet counters and state
 information for each configured WDS AP.
  </tr>
  <tr><hr size=1 noshade align=top></tr>
</table>
<form action=/goform/formWirelessTbl method=POST name="formWirelessTbl">
<table border='1' width="500">
<tr bgcolor=#7f7f7f><td width="30%"><font size=2><b>MAC Address</b></td>
<td width="15%"><font size=2><b>Tx Packets</b></td>
<td width="15%"><font size=2><b>Tx Errors</b></td>
<td width="15%"><font size=2><b>Rx Packets</b></td>
<td width="25%"><font size=2><b>Tx Rate (Mbps)</b></td></tr>
<% wdsList(); %>
</table>

<input type="hidden" value="/wlwdstbl.asp" name="submit-url">
  <p><input type="submit" value="Refresh" name="refresh">&nbsp;&nbsp;
  <input type="button" value=" Close " name="close" onClick="javascript: window.close();"></p>
</form>
</blockquote>
</body>

</html>
