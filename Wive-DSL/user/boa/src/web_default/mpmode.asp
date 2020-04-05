
<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Multi-port Administration</title>
<% language=javascript %>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">Multi-port Admin. Status</font></h2>

<form action=/goform/formMpMode method=POST name=mpmode>
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><font size=2>
  This multi-port administration page is used to select the multiport-aware
  function. Note that these functions are mutual-exclusive.
  </tr>
  <tr><hr size=1 noshade align=top></tr>

  <!--
  <tr>
      <td width="30%"><font size=2><b>Admin Status:&nbsp;&nbsp;</b>
      <select size=1 name=mpMode>
           <option selected value=0>None</option>
           <option value=1>Port Mapping</option>
           <option value=2>Vlan Enable</option>
           <option value=3>IP QoS</option>
           <option value=4>IGMP Snooping</option>
      </select>
      </td>
  </tr>
  -->
    <tr>
      <td width="30%"><font size=2><b>Port Mapping:</b></td>
      <td width="70%"><font size=2>
      <input type="radio" name=pmap value=0>Disabled&nbsp;&nbsp;
      <input type="radio" name=pmap value=1>Enabled</td>
    </tr>
    <tr>
      <td width="30%"><font size=2><b>IP QoS:</b></td>
      <td width="70%"><font size=2>
      <input type="radio" name=qos value=0>Disabled&nbsp;&nbsp;
      <input type="radio" name=qos value=1>Enabled</td>
    </tr>
</table>
  <br>
      <input type=submit value="Apply Changes" name=save>&nbsp;&nbsp;
      <input type=hidden value="/mpmode.asp" name=submit-url>
 <script>
 	<% initPage("mpmode"); %>
 </script>
 </form>
</blockquote>
</body>

</html>
