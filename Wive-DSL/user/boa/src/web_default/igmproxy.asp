<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>IGMP Proxy Configuration </title>
<% language=javascript %>
<SCRIPT>
function proxySelection()
{
	if (document.igmp.proxy[0].checked) {
		document.igmp.proxy_if.disabled = true;
	}
	else {
		document.igmp.proxy_if.disabled = false;
	}
}
</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">IGMP Proxy Configuration</font></h2>

<form action=/goform/formIgmproxy method=POST name="igmp">
<table border=0 width="500" cellspacing=0 cellpadding=0>
  <tr><font size=2>
    IGMP proxy enables the system to issue IGMP host messages on behalf of
    hosts that the system discovered through standard IGMP interfaces.
    The system acts as a proxy for its hosts when you enable it by doing
    the follows:
    <br>. Enable IGMP proxy on WAN interface (upstream), which connects to a router running IGMP.
    <br>. Enable IGMP on LAN interface (downstream), which connects to its hosts.
  </tr>
  <tr><hr size=1 noshade align=top></tr>

  <tr>
      <td width><font size=2><b>IGMP Proxy:</b></td>
      <td width><font size=2>
      	<input type="radio" value="0" name="proxy" <%checkWrite("igmpProxy0"); %> onClick="proxySelection()">Disable&nbsp;&nbsp;
     	<input type="radio" value="1" name="proxy" <%checkWrite("igmpProxy1"); %> onClick="proxySelection()">Enable
      </td>
  </tr>
  <tr>
      <td><font size=2><b>Proxy Interface:</b></td>
      <td>
      	<select name="proxy_if" <%checkWrite("igmpProxy0d"); %>>
          <%  if_wan_list("rt");
          %>
      	</select>
      </td>
      <td><input type="submit" value="Apply Changes" name="save">&nbsp;&nbsp;</td>
  </tr>
</table>
      <input type="hidden" value="/igmproxy.asp" name="submit-url">
<script>
	ifIdx = <% getInfo("igmp-proxy-itf"); %>;
	if (ifIdx != 255)
		document.igmp.proxy_if.value = ifIdx;
	else
		document.igmp.proxy_if.selectedIndex = 0;
	
</script>
</form>
</blockquote>
</body>

</html>
