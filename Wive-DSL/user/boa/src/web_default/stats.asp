<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Statisitcs</title>
<script>
function resetClick() {
	with ( document.forms[0] ) {
		reset.value = 1;
		submit();
	}
}
</script>
</head>
<body>
<blockquote>
<!--h2><font color="#0000FF">Statistics</font></h2-->
<h2><font color="#0000FF">Statistics -- Interfaces</font></h2>

<table border=0 width="500" cellpadding=0>
  <tr><font size=2>
 This page shows the packet statistics for transmission and reception regarding to network
 interface.
  </tr>
  <tr><hr size=1 noshade align=top></tr>
</table>
<form action=/goform/formStats method=POST name="formStats">
<table border="1" width="500">
	<% pktStatsList(); %>
</table>
  <br>
<!--	<% memDump(); %> -->
  <br><br>
  <input type="hidden" value="/stats.asp" name="submit-url">
  <input type="submit" value="Refresh" name="refresh">
  <input type="hidden" value="0" name="reset">
  <input type="button" onClick="resetClick()" value="Reset Statistics">
</form>
</blockquote>
</body>

</html>
