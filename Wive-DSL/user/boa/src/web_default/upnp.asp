<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>UPnP Configuration </title>
<% language=javascript %>
<SCRIPT>
function proxySelection()
{
	if (document.upnp.daemon[0].checked) {
		document.upnp.ext_if.disabled = true;
	}
	else {
		document.upnp.ext_if.disabled = false;
	}
}
</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">UPnP Configuration</font></h2>

<form action=/goform/formUpnp method=POST name="upnp">
<table border=0 width="500" cellspacing=0 cellpadding=0>
  <!--tr><font size=2>
    This page is used to configure UPnP. The system acts as a daemon when you enable it by doing
    the follows:
    <br>. Enable UPnP.
    <br>. Select WAN interface (uptream) that will use UPnP.
  </tr-->
  <tr><font size=2>
    This page is used to configure UPnP. The system acts as a daemon when you enable it and
    select WAN interface (upstream) that will use UPnP.
  </tr>
  <tr><hr size=1 noshade align=top></tr>

  <tr>
      <td><font size=2><b>UPnP:</b></td>
      <td><font size=2>
      	<input type="radio" value="0" name="daemon" <%checkWrite("upnp0"); %> onClick="proxySelection()">Disable&nbsp;&nbsp;
     	<input type="radio" value="1" name="daemon" <%checkWrite("upnp1"); %> onClick="proxySelection()">Enable
      </td>
  </tr>
  <tr>
      <td><font size=2><b>WAN Interface:</b></td>
      <td>
      	<select name="ext_if" <%checkWrite("upnp0d"); %>>
          <%  if_wan_list("rt");
          %>
      	</select>
      </td>
      <td><input type="submit" value="Apply Changes" name="save">&nbsp;&nbsp;</td>
  </tr>
</table>
      <input type="hidden" value="/upnp.asp" name="submit-url">
<script>
	initUpnpDisable = document.upnp.daemon[0].checked;
	ifIdx = <% getInfo("upnp-ext-itf"); %>;
	if (ifIdx != 255)
		document.upnp.ext_if.value = ifIdx;
	else
		document.upnp.ext_if.selectedIndex = 0;		

	proxySelection();
</script>
</form>
</blockquote>
</body>

</html>
