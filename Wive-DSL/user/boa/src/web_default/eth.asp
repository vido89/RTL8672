<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Link Speed/Duplex Mode </title>
<% language=javascript %>
<SCRIPT>
function postit(port, pvc)
{
	switch(port) {
	case 0:
		if (pvc != 255)
			document.eth2pvc.vc0.value = pvc;
		else
			document.eth2pvc.vc0.selectedIndex = 0;
		break;
	case 1:
		if (pvc != 255)
			document.eth2pvc.vc1.value = pvc;
		else
			document.eth2pvc.vc1.selectedIndex = 0;
		break;
	case 2:
		if (pvc != 255)
			document.eth2pvc.vc2.value = pvc;
		else
			document.eth2pvc.vc2.selectedIndex = 0;
		break;
	case 3:
		if (pvc != 255)
			document.eth2pvc.vc3.value = pvc;
		else
			document.eth2pvc.vc3.selectedIndex = 0;
	default: break;
	}
}

function linkOption()
{
	document.write('<option value=0>10 Half Mode</option>\n');
	document.write('<option value=1>10 Full Mode</option>\n');
	document.write('<option value=2>100 Half Mode</option>\n');
	document.write('<option value=3>100 Full Mode</option>\n');
	document.write('<option value=4>Auto Mode</option>\n');
}
	
</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">Ethernet Link Speed/Duplex Mode</font></h2>

<form action=/goform/formLinkMode method=POST name=link>
<table border=0 width=500 cellspacing=4 cellpadding=0>
  <tr><font size=2>
    Set the Ethernet link speed/duplex mode.
  </tr>
  <tr><hr size=1 noshade align=top></tr>
  <tr>
      <td width="30%"><font size=2><b>eth0:</b></td>
      <td width="35%">
      	<select name=p0>
      		<script>
      			linkOption();
      		</script>
      	</select>
      </td>
  </tr>
</table>
  <br>
      <input type="submit" value="Apply Changes" name="save">&nbsp;&nbsp;
      <input type="hidden" value="/eth.asp" name="submit-url">
<script>
	<% initPage("linkMode"); %>
</script>
</form>
</blockquote>
</body>

</html>
