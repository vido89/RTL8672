
<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Logout</title>

<SCRIPT>
function confirmadminlogout()
{
   if ( !confirm('do you confirm to logout?') ) {
	return false;
  }
  else
	return true;
}
</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">Logout</font></h2>

<form action=/goform/formAdminLogout method=POST name="cmadminlogout">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><font size=2>
    This page is used to logout from adsl gateway.
  </tr>
  <tr><hr size=1 noshade align=top></tr>

</table>
  <br>
      <input type="submit" value="Logout" name="adminlogout" onclick="return confirmadminlogout()">&nbsp;&nbsp;
      <input type="hidden" value="/admin/adminlogout.asp" name="submit-url">
 </form>
</blockquote>
</body>

</html>
