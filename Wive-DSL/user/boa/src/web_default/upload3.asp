<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Firmware Update</title>
<script>
function sendClicked()
{
	if (!confirm('Do you really want to upgrade the firmware?'))
		return false;
	else {
		document.upgrd.act.value=2;
		return true;
	}
}

</script>

</head>
<BODY>
<blockquote>
<h2><font color="#0000FF">Upgrade Firmware</font></h2>
<form action=/goform/admin/formUpload method=POST name="upgrd">
<table border="0" cellspacing="4" width="500">
 <tr><font size=2>
 This page allows you upgrade the ADSL Router firmware to new version. Please note,
 do not power off the device during the upload because it may crash the system.
 </tr>
  <tr><hr size=1 noshade align=top></tr>
</table>
    <p>
	<input type="submit" value="Start Upgrade ..." name="send" onclick="return sendClicked()">&nbsp;&nbsp;
	<input type="hidden" name="act">
    </p>
 </form>
 </blockquote>
</body>
</html>
