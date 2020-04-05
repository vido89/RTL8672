<html>
<! Copyright (c) Realtek Semiconductor Corp., 2008. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<title>802.1p setting</title>
<script type="text/javascript" src="share.js">
</script>

</head>
<BODY>
<blockquote>
<h2><font color="#0000FF">802.1p  priority config</font></h2>

<table border=0 width="500" cellspacing=4 cellpadding=0>
<tr><td><font size=2>
This page is used to config 802.1p priority.
</font></td></tr>
<tr><td><hr size=1 noshade align=top></td></tr>
</table>
<form action=/goform/formIPQoS method=POST name=qos_set1p>
<table border="0" width=500>
<tr><font size=2>802.1p Rule List:</font></tr>
<% setting1p(); %>
</table>	
<input type="hidden" value="/qos_8021p.asp" name="submit-url">
<td><input type="submit" value="modify" name=set1p ></td>
<input type="button" value="close" name="close" onClick="javascript: window.close();"></p>
</form>
</blockquote>
</body>
</html>



