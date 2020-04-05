
<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Landing Page Configuration </title>
<% language=javascript %>

<script>
function saveClick()
{		
	alert("Please commit and reboot this system for take effect the Landing Page!");
}

</script>

</head>

<body>
<blockquote>
<h2><font color="#0000FF">Landing Page Configuration</font></h2>

<form action=/goform/formLanding method=POST name="landing">
<table border=0 width="500" cellspacing=0 cellpadding=0> 
  <tr><font size=2>
    This page is used to configure the time interval of Landing Page .
  </tr>
  <tr><hr size=1 noshade align=top></tr>

  <tr>
      <td width="30%"><font size=2><b> Time Interval:</b></td>
      <td width="70%"><input type="text" name="interval" size="15" maxlength="15" value=<% getInfo("landing-page-time"); %>>(seconds)</td>
  </tr>   
  
</table>
      <br>
      <input type="submit" value="Apply Changes" name="save" onClick="return saveClick()">&nbsp;&nbsp;
      <input type="hidden" value="/landing.asp" name="submit-url">

</form>
</blockquote>
</body>

</html>