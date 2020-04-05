
<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Finish Maintenance</title>

<SCRIPT>
function confirmfinsih()
{
   if ( !confirm('do you confirm the maintenance is over?') ) {
	return false;
  }
  else
	return true;
}
</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">Finish Maintenance</font></h2>

<form action=/goform/formFinishMaintenance method=POST name="cmfinish">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><font size=2>
    This page is used to inform ITMS that maintenance is finished,
    and then ITMS may change this gateway's password.
  </tr>
  <tr><hr size=1 noshade align=top></tr>

</table>
  <br>
      <input type="submit" value="Finish Maintenance" name="finish" onclick="return confirmfinsih()">&nbsp;&nbsp;
      <input type="hidden" value="/finishmaintenance.asp" name="submit-url">
 </form>
</blockquote>
</body>

</html>
