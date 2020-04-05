<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>ALG On-Off</title>
<script>
	function AlgTypeStatus()
	{
		<%checkWrite("AlgTypeStatus");%>
		return true;
	}
</script>
</head>

<body >
<blockquote>
<h2><font color="#0000FF">ALG On-Off</font></h2>

<table border=0 width=500 cellspacing=4 cellpadding=0>
<tr><td colspan=4><font size=2>
	This page is used to enable/disable ALG services.
	<br>
 </td></tr>
</table>
<table  width=500>
<tr><td colspan=4><hr size=1 noshade align=top></td></tr>
<form action=/goform/formALGOnOff method=POST name=algof>
<table>
<tr>
<td><font size=2>ALG Type:</font></td>
<td colspan="2">	
</td>
</tr>
<%checkWrite("GetAlgType")%>	
<tr>
	<td ><input type=submit value="Apply Changes" name=apply></td>
  <td> <input type="hidden" value="/algonoff.asp" name="submit-url"></td>
  <td></td>
</tr>
</table>
</form>
<script>
AlgTypeStatus();
</script>
</table>
</blockquote>
</body>
</html>
