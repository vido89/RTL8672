<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta HTTP-EQUIV='Pragma' CONTENT='no-cache'>
<meta http-equiv="Content-Type" content="text/html">
<title>ADSL Safe Mode</title>
<% language=javascript %>
<SCRIPT>
function validateKey(str)
{
	for (var i=0; i<str.length; i++) {
		if ((str.charAt(i) >= '0' && str.charAt(i) <= '9') || (str.charAt(i) >= 'A' && str.charAt(i) <= 'F') || (str.charAt(i) >= 'a' && str.charAt(i) <= 'f'))
			continue;
		return 0;
	}
	return 1;
}

function saveChanges()
{               
	if (validateKey(document.adsl_safe.ctrlin.value) == 0) {
		alert('Invalid Field Try CtrlIn value. It should be in hex number (0-9 or a-f or A-F)');
		document.adsl_safe.ctrlin.focus();
		return false;
	}
	return true;
}
</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">ADSL Safe Mode</font></h2>

<form action=/goform/formAdslSafeMode method=POST name=adsl_safe>
<table border=0 width=500 cellspacing=4 cellpadding=0>
	<tr><font size=2>
	  Adsl Field Try Safe Mode Settings.
	</tr>
	<tr><hr size=1 noshade align=top></tr>
</table>

<table border=0 width=500 cellspacing=4 cellpadding=0>
<tr>
	<th align=left width=50%><font size=2>Field Try Safe Mode :</th>
	<td><font size=2><input type="checkbox" name="safemode" value="ON">Enabled</td>
</tr>
<tr>
	<th align=left width=50%><font size=2>Field Try Test PSD Times :</th>
	<td><font size=2><input type="text" name="psdtime" size=10 maxlength=8></td>
</tr>
<tr>
	<th align=left width=50%><font size=2>Field Try CtrlIn :</th>
	<td><font size=2>0x<input type="text" name="ctrlin" size=10 maxlength=8> (Hex) </td>
</tr>
<tr>
	<th align=left width=50%><font size=2>Safe Mode Note :</th>
	<td><font size=2><% getInfo("safemodenote"); %></td>
</tr>
</table>
  <br>
	<input type=submit value="Apply Changes" name="save" onClick="return saveChanges()">
	<input type=hidden value="/adv/adslsafemode.asp" name="submit-url">
<script>
	<% initPage("adslsafemode"); %>
</script>
</form>
</blockquote>
</body>

</html>
