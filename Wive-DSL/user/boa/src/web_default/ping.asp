<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Ping Test </title>
<script type="text/javascript" src="share.js">
</script>
<% language=javascript %>
<SCRIPT>
function goClick()
{
	if (!checkHostIP(document.ping.pingAddr, 1))
		return false;
/*	if (document.ping.pingAddr.value=="") {
		alert("Enter host address !");
		document.ping.pingAddr.focus();
		return false;
	}
	
	return true;*/
}
</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">Ping Diagnostic</font></h2>

<form action=/goform/formPing method=POST name="ping">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><font size=2>
    This page is used to send ICMP ECHO_REQUEST packets to network host.
 The diagnostic result will then be displayed.
  </tr>
  <tr><hr size=1 noshade align=top></tr>

  <tr>
      <td width="30%"><font size=2><b>Host Address :</b></td>
      <td width="70%"><input type="text" name="pingAddr" size="15" maxlength="30"></td>
  </tr>

</table>
  <br>
      <input type="submit" value=" Go ! " name="go" onClick="return goClick()">
      <input type="hidden" value="/ping.asp" name="submit-url">
 </form>
</blockquote>
</body>

</html>
