<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Firmware Update</title>
<script>
function upgrdClick(url)
{
	var wide=600;
	var high=400;
	if (document.all)
		var xMax = screen.width, yMax = screen.height;
	else if (document.layers)
		var xMax = window.outerWidth, yMax = window.outerHeight;
	else
	   var xMax = 640, yMax=480;
	var xOffset = (xMax - wide)/2;
	var yOffset = (yMax - high)/3;

	var settings = 'width='+wide+',height='+high+',screenX='+xOffset+',screenY='+yOffset+',top='+yOffset+',left='+xOffset+', resizable=yes, toolbar=no,location=no,directories=no,status=no,menubar=no,scrollbars=yes';

	document.upgrd.act.value=1;
	document.upgrd.submit();
	window.open( url, 'upgrade', settings );
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
</table>
<br>
	<input type="button" value="Start Upgrade ..." onClick=upgrdClick('/index2.html')>&nbsp;&nbsp;
	<input type="hidden" value="/admin/upload2.asp" name="submit-url">
	<input type="hidden" name="act">
 </form>
 </blockquote>
</body>
</html>
