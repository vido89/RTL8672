<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Remote Configuration </title>
<script>
function checkEnable()
{
	if ( document.rconf.uid.value.length == 0 ) {
		alert('User Name cannot be empty.');
		document.rconf.uid.focus();
		return false;
	}
	if ( document.rconf.pwd.value.length == 0 ) {
		alert('Password cannot be empty.');
		document.rconf.pwd.focus();
		return false;
	}
	
	return true;
}
</script>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">Remote Configuration</font></h2>

<form action=/goform/formRconfig method=POST name="rconf">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><font size=2>
    This page provides temporary WAN side remote access to web GUI.
    The default access port is 51003.
  </tr>
  <tr><hr size=1 noshade align=top></tr>
</table>

<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><b>Current Status: </b>
    <script>
      if (<% write(getIndex("rconf-status")); %> == 0)
        document.write("Disabled");
      else
        document.write("Remote access temporary opened on port ", <% write(getIndex("rconf-port")); %>);
    </script>
  </tr>
  <% state = getIndex("rconf-status"); if (state != 0) write("<!--"); %>
  <tr>
      <td width=30%><font size=2><b>
       	<input type="checkbox" name="writable" value="ON">&nbsp;&nbsp;Allow Update</b>
      </td>
      <td width=30%><b>
       	<input type="checkbox" name="portFlag" value="ON">&nbsp;&nbsp;Use Default Port</b>
      </td>
  </tr>
  <tr>
      <td width=30%><font size=2><b>User Name:</b></td>
      <td width=70%>
      	<input type=text name="uid" value="tech" size=20 maxlength=30>
      </td>
  </tr>
  <tr>
      <td width=30%><font size=2><b>Password:</b></td>
      <td width=70%>
      	<input type=text name="pwd" size=20 maxlength=30>
      </td>
  </tr>
  <% if (state != 0) write("-->"); %>
</table>
  <br>
  <% if (state != 0) write("<!--"); %>
      <input type=submit value="Enable" name="yes" onClick="return checkEnable()">
      <input type=reset value="Undo" name="reset">
  <% if (state != 0) write("-->"); %>
  <% if (state == 0) write("<!--"); %>
      <input type=submit value="Disable" name="no">
      <input type=submit value="Refresh" name="refresh">
  <% if (state == 0) write("-->"); %>
      <input type=hidden value="/rconfig.asp" name="submit-url">
</form>
</blockquote>
</body>

</html>
