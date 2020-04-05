<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>ADSL Settings </title>
<% language=javascript %>
<SCRIPT>

function dhcpTblClick(url) {
	openWindow(url, 'DHCPTbl' );
}

function adsltoneClick(url)
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

	window.open( url, 'ADSLTONETbl', settings );
}

function saveChanges()
{               
	if (document.set_adsl.glite.checked == false
	   && document.set_adsl.anxb.checked == false
	   && document.set_adsl.gdmt.checked == false
	   && document.set_adsl.t1413.checked == false
	   && document.set_adsl.adsl2.checked == false
	   && document.set_adsl.adsl2p.checked == false) {
		alert("ADSL modulation cannot be empty.");
		return false;
	}
	return true;
}

</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">ADSL Settings</font></h2>

<form action=/goform/formSetAdsl method=POST name=set_adsl>
<table border=0 width=500 cellspacing=4 cellpadding=0>
	<tr><font size=2>
	  Adsl Settings.
	</tr>
	<tr><hr size=1 noshade align=top></tr>
</table>

<table border=0 width=500 cellspacing=4 cellpadding=0>
<tr>
	<th align=left width=30%><font size=2>ADSL modulation:</th>
	<td width=70%></td>
</tr>
<tr>
	<th></th>
        <td><font size=2><input type=checkbox name=anxb value=1>AnnexB</td>
</tr>
<tr>
	<th></th>
	<td><font size=2><input type=checkbox name=glite value=1>G.Lite</td>
</tr>
<tr>
	<th></th>
	<td><font size=2><input type=checkbox name=gdmt value=1>G.Dmt</td>
</tr>
<tr>
	<th></th>
	<td><font size=2><input type=checkbox name=t1413 value=1>T1.413</td>
</tr>
<tr>
	<th></th>
	<td><font size=2><input type=checkbox name=adsl2 value=1>ADSL2</td>
</tr>
<tr>
	<th></th>
	<td><font size=2><input type=checkbox name=adsl2p value=1>ADSL2+</td>
</tr>
<tr>
	<th align=left width=30%><font size=2>AnnexL Option:</th>
	<td width=70%><font size=2>(Note: Only ADSL 2 supports AnnexL)</td>
</tr>
<tr>
	<th></th>
	<td><font size=2><input type=checkbox name=anxl value=1>Enabled</td>
</tr>
<tr>
	<th align=left width=30%><font size=2>AnnexM Option:</th>
	<td width=70%><font size=2>(Note: Only ADSL 2/2+ support AnnexM)</td>
</tr>
<tr>
	<th></th>
	<td><font size=2><input type=checkbox name=anxm value=1>Enabled</td>
</tr>
<tr>
	<th align=left><font size=2>ADSL Capability:</th>
	<td></td>
</tr>
<tr>
	<th></th>
	<td><font size=2><input type=checkbox name=bswap value=1>Bitswap Enable</td>
</tr>
<tr>
	<th></th>
	<td><font size=2><input type=checkbox name=sra value=1>SRA Enable</td>
</tr>

<tr>
	<th align=left><font size=2>ADSL Tone:</th>
	<td></td>
</tr>
<tr>
	<th></th>
	<td><font size=2><input type="button" value="Tone Mask" name="adsltoneTbl" onClick="adsltoneClick('/adsltone.asp')"></td>
</tr>


</table>
  <br>
	<input type=submit value="Apply Changes" name="save" onClick="return saveChanges()">
	<input type=hidden value="/adv/adsl-set.asp" name="submit-url">
<script>
	<% initPage("setdsl"); %>
</script>
</form>
</blockquote>
</body>

</html>
