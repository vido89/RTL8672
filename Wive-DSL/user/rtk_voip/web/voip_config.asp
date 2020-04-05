<html>
<head>
<meta http-equiv="Content-Type" content="text/html">
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Cache-Control" CONTENT="no-cache">
<title>Config</title>
<script language="javascript">
function resetClick()
{
	if ( !confirm('Do you really want to reset the VoIP settings to default?') ) 
		return false;
	else
		return true;
}

function verifyBrowser() 
{
	var ms = navigator.appVersion.indexOf("MSIE");
	ie4 = (ms>0) && (parseInt(navigator.appVersion.substring(ms+5, ms+6)) >= 4);
	var ns = navigator.appName.indexOf("Netscape");
	ns= (ns>=0) && (parseInt(navigator.appVersion.substring(0,1))>=4);
	if (ie4)
	{
		return "ie4";
	}
	else
	{
		if(ns)
			return "ns";
		else
			return false;
	}
}

function saveClick(url)
{
	if (verifyBrowser() == "ie4") 
	{
		window.location.href = url;
		return false;
	}
	else
		return true;
}
</script>
</head>
<body bgcolor="#ffffff" text="#000000">
<b>Auto Config</b>
<table cellSpacing=1 cellPadding=2 border=0 width=375>
<form method="get" action="/goform/voip_config_set" name=config_form>
<tr>
   	<td bgColor=#aaddff width=150>Mode</td>
	<td bgColor=#ddeeff>
		<input type=radio name=mode value=1 <%voip_config_get("mode_http");%>> HTTP
		<input type=radio name=mode value=0 <%voip_config_get("mode_disable");%>> Disable
	</td>
</tr>	
<tr>
   	<td bgColor=#aaddff>HTTP Server Address</td>
	<td bgColor=#ddeeff>
		<input type=text name=http_addr size=20 maxlength=39 value="<%voip_config_get("http_addr");%>">
	</td>
</tr>	
<tr>
   	<td bgColor=#aaddff>HTTP Server Port</td>
	<td bgColor=#ddeeff>
		<input type=text name=http_port size=10 maxlength=5 value="<%voip_config_get("http_port");%>">
	</td>
</tr>
<tr>
   	<td bgColor=#aaddff>File Path</td>
	<td bgColor=#ddeeff>
		<input type=text name=file_path size=10 maxlength=60 value="<%voip_config_get("file_path");%>">
	</td>
</tr>
<tr>
   	<td bgColor=#aaddff>Expire Time</td>
	<td bgColor=#ddeeff>
		<input type=text name=expire size=10 maxlength=5 value="<%voip_config_get("expire");%>"> days
	</td>
</tr>
<tr>
	<td colspan=2 align=center>
		<input type="submit" value="Apply Changes">
   		&nbsp;&nbsp;&nbsp;&nbsp;    	
		<input type="reset" value="Reset">
	</td>
</tr>
</form>
</table>

<p>
<b>Save/Reload Setting</b>
<table cellSpacing=1 cellPadding=2 border=0 width=375>
<form action=/goform/voip_config_save method=POST name=save_form>
<tr>
   	<td bgColor=#aaddff width=150>Save Settings to File</td>
	<td bgColor=#ddeeff>
		<input type="submit" value="Save..." name="save" onclick="return saveClick('/config_voip.dat')">
	</td>
</tr>
</form>
<form method="post" action=/goform/voip_config_load enctype="multipart/form-data" name=load_form>
<tr>
   	<td bgColor=#aaddff width=150>Load Settings from File</td>
	<td bgColor=#ddeeff>
		<input type="file" name="binary" size=24>
	    <input type="submit" value="Upload" name="load">
	</td>
</tr>
</form>
<form method="get" action="/goform/voip_config_set" name=config_form>
<tr>
   	<td bgColor=#aaddff width=150>Reset Settings to Default</td>
	<td bgColor=#ddeeff>
	    <input type="submit" value="Reset" name="reset" onclick="return resetClick()">
	</td>
</tr>
</form>
</table>

</body>
</html>
