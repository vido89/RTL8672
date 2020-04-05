<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Bridge Forwarding Database Table</title>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">Bridge Forwarding Database Table</font></h2>

<table border=0 width="480" cellspacing=0 cellpadding=0>
  <tr><font size=2>
 This table shows a list of learned MAC addresses for this bridge.
  </tr>
  <tr><hr size=1 noshade align=top></tr>
</table>


<form action=/goform/formRefleshFdbTbl method=POST name="formFdbTbl">
<table border='1' width="80%">
<tr bgcolor=#7f7f7f> <td width="10%"><font size=2><b>Port No</b></td>
<td width="20%"><font size=2><b>MAC Address</b></td>
<td width="10%"><font size=2><b>Is Local?</b></td>
<td width="10%"><font size=2><b>Ageing Timer</b></td></tr>
<% bridgeFdbList(); %>
</table>

<input type="hidden" value="/fdbtbl.asp" name="submit-url">
  <p><input type="submit" value="Refresh" name="refresh">&nbsp;&nbsp;
  <input type="button" value=" Close " name="close" onClick="javascript: window.close();"></p>
</form>
</blockquote>
</body>

</html>
