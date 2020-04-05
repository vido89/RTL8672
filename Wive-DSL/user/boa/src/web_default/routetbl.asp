<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>IP Route Table</title>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">IP Route Table</font></h2>

<table border=0 width="480" cellspacing=0 cellpadding=0>
  <tr><font size=2>
 This table shows a list of destination routes commonly accessed by your network.
  </tr>
  <tr><hr size=1 noshade align=top></tr>
</table>


<form action=/goform/formRefleshRouteTbl method=POST name="formRouteTbl">
<table border='1' width="80%">
<% routeList(); %>
</table>

<input type="hidden" value="/routetbl.asp" name="submit-url">
  <p><input type="submit" value="Refresh" name="refresh">&nbsp;&nbsp;
  <input type="button" value=" Close " name="close" onClick="javascript: window.close();"></p>
</form>
</blockquote>
</body>

</html>
