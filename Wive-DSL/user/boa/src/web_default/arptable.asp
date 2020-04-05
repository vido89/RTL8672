<html>
<! Copyright (c) Realtek Semiconductor Corp., 2006. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>ARP Table</title>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">ARP Table</font></h2>

<table border=0 width="480" cellspacing=0 cellpadding=0>
  <tr><font size=2>
 This table shows a list of learned MAC addresses.
  </tr>
  <tr><hr size=1 noshade align=top></tr>
</table>


<form action=/goform/formRefleshFdbTbl method=POST name="formFdbTbl">
<table border='1' width="500" >
<tr bgcolor=#7f7f7f> <td width="20%"><font size=2><b>IP Address</b></td>
<td width="20%"><font size=2><b>MAC Address</b></td>
<% ARPTableList(); %>
</table>

<input type="hidden" value="/arptable.asp" name="submit-url">
  <p><input type="submit" value="Refresh" name="refresh">&nbsp;&nbsp;
</form>
</blockquote>
</body>

</html>
