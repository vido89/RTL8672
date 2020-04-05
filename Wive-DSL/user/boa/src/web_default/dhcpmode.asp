
<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>DHCP Mode Configuration</title>
<% language=javascript %>
<SCRIPT>
</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">DHCP Mode Configuration</font></h2>

<form action=/goform/formDhcpMode method=POST name="dhcpmode">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><font size=2>
    Use this page to set and configure the Dynamic Host Protocol mode 
    for your device. With DHCP, IP addresses for your LAN are administered 
    and distributed as needed by this device or an ISP device.
  </tr>
  <tr><hr size=1 noshade align=top></tr>

  <tr>
      <td width="30%"><font size=2><b>DHCP Mode:</b>
      <select size="1" name="dhcpMode">
      <% checkWrite("dhcpMode"); %>
      </select>
      </td>
  </tr>
</table>
  <br>
      <input type="submit" value="Commit Changes" name="save"">&nbsp;&nbsp;
      <input type="hidden" value="/dhcpmode.asp" name="submit-url">
 </form>
</blockquote>
</body>

</html>


