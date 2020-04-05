<html>
<! Copyright (c) sfstudio http://wive-ng.sf.net 2009. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<META HTTP-EQUIV="Cache-Control" CONTENT="no-cache"><META HTTP-EQUIV="PRAGMA" CONTENT="NO-CACHE">
<title>SHAPER tunnel setup </title>
<script type="text/javascript" src="share.js">
</script>
<% language=javascript %>
<SCRIPT>
function resetClick()
{
	documentreset;
}

function textstate()
{
}

function saveChanges()
{
	textstate();
	return true;
}

function updateState()
{
	textstate();
        <% initPage("formSHAPERSetup"); %>
	return true;
}
</SCRIPT>
<% language=javascript %>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">Simple shaper config</font></h2>

<form action=/goform/formSHAPERSetup method=POST name="formSHAPERSetup">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><font size=2>
    This page is used to configure shaper mode your Router.
  </tr>
    <br>
    <font size=2><input type="checkbox" name="shaper_enable"
    onClick=updateState()>&nbsp;&nbsp;<b>Shaper enable</b></font>
    <br>
    <font size=2><input type="checkbox" name="shaper_in_enable"
    onClick=updateState()>&nbsp;&nbsp;<b>Shape incoming connection</b></font>
    <br>
    <font size=2><input type="checkbox" name="shaper_ip2p_enable"
    onClick=updateState()>&nbsp;&nbsp;<b>IP2P enable</b></font>
    <br>
    <font size=2><input type="checkbox" name="shaper_l7_enable"
    onClick=updateState()>&nbsp;&nbsp;<b>L7 enable</b></font>
    <br>
  <tr>
      <td width="40%"><font size=2><b>Maximum upload rate:</b></td>
      <td width="60%"><input type="text" name="shaper_up" size="25" maxlength="60" 
      value=<% getInfo("shaper_up"); %>>
	<font size=2><b>Kbit/s</b></font></td>
  </tr>
  <tr>
      <td width="40%"><font size=2><b>Limit upload rate:</b></td>
      <td width="60%"><input type="text" name="shaper_limit_up" size="25" maxlength="60" 
      value=<% getInfo("shaper_limit_up"); %>>
	<font size=2><b>Kbit/s</b></font></td>
  </tr>
  <tr>
      <td width="40%"><font size=2><b>Maximum download rate:</b></td>
      <td width="60%"><input type="text" name="shaper_down" size="25" maxlength="60" 
      value=<% getInfo("shaper_down"); %>>
	<font size=2><b>Kbit/s</b></font></td>
  </tr>
  <tr>
      <td width="40%"><font size=2><b>Limit download rate:</b></td>
      <td width="60%"><input type="text" name="shaper_limit_down" size="25" maxlength="60"
      value=<% getInfo("shaper_limit_down"); %>>
	<font size=2><b>Kbit/s</b></font></td>
  </tr>
  <tr>
      <td width="40%"><font size=2><b>High prio ports:</b></td>
      <td width="60%"><input type="text" name="shaper_high_prio_ports" size="25" maxlength="60"
      value=<% getInfo("shaper_high_prio_ports"); %>></td>
  </tr>
  <tr>
      <td width="40%"><font size=2><b>Low prio ports:</b></td>
      <td width="60%"><input type="text" name="shaper_low_prio_ports" size="25" maxlength="60"
      value=<% getInfo("shaper_low_prio_ports"); %>></td>
  </tr>
    <tr></tr><tr></tr>
  </table>
  <br>
      <input type="submit" value="Apply" name="save" onClick="return saveChanges()">&nbsp;&nbsp;
      <input type="hidden" value="/shaper-set.asp" name="submit-url">
      <input type="reset" value="Reset" name="reset" onClick="resetClick()">&nbsp;&nbsp;
    <br><br>
    <font size=2>
    Enables ifb+htb+esfq shaper. For tune please edit /rwfs/init.d/shaper over ssh/telnet.
    Default esfq+htb for connections over vpn or pppoe.<br>
    Default all traffic 1-1024 udp/tcp ports and icmp is high priority, all others is low. 
    <br><br>
    <b>Maximum upload rate</b> - Establish the ~85% upload tariff speed.
    <br>
    <b>Limit upload rate</b> - Establish the ~50%-70% of upload tariff speed.
    <br>
    <b>Maximum downoad rate</b> - Establish ~85% the download tariff speed.
    <br>
    <b>Limit download rate</b> - Establish the ~50%-70% of download tariff speed.
    <br>
    <b>High prio ports</b> - Add ports for high prio connections (prio1).
    <br>
    <b>Low prio ports</b> - Add ports for low prio connections (prio2).
    <br>
    <b>IP2P enable</b> - use ip2p module for set low prio torrents traffic (prio3).
    <br>
    <b>L7 enable</b> - use l7 module for set low prio torrents traffic and prio voip.
    <br>
     </font>
<script>
    if ("<% getInfo("shaper_enable"); %>" == "on") 
    {
    document.formSHAPERSetup.shaper_enable.checked = true;
    }
    if ("<% getInfo("shaper_in_enable"); %>" == "on") 
    {
    document.formSHAPERSetup.shaper_in_enable.checked = true;
    }
    if ("<% getInfo("shaper_ip2p_enable"); %>" == "on") 
    {
    document.formSHAPERSetup.shaper_ip2p_enable.checked = true;
    }
    if ("<% getInfo("shaper_l7_enable"); %>" == "on") 
    {
    document.formSHAPERSetup.shaper_l7_enable.checked = true;
    }
    //type of tunnel select
    textstate();
    updateState();
</script>
 </form>
</blockquote>
</body>
</html>
