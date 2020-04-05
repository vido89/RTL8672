<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Password configuration</title>
<script type="text/javascript" src="share.js">
</script>
<% language=javascript %>
<SCRIPT>

function saveChanges()
{
   if ( document.password.username.value.length == 0 ) {
	if ( !confirm('User account is empty.\nDo you want to disable the password protection?') ) {
		document.password.username.focus();
		return false;
  	}
	else
		return true;
  }

   if ( document.password.newpass.value != document.password.confpass.value) {
	alert('Password is not matched. Please type the same password between \'new\' and \'confirmed\' box.');
	document.password.newpass.focus();
	return false;
  }

  if ( document.password.username.value.length > 0 &&
  		document.password.newpass.value.length == 0 ) {
	alert('Password cannot be empty. Please try it again.');
	document.password.newpass.focus();
	return false;
  }

  if ( includeSpace(document.password.username.value)) {
	alert('Cannot accept space character in user name. Please try it again.');
	document.password.username.focus();
	return false;
  }
  if (checkString(document.password.username.value) == 0) {
	alert('Invalid user name !');
	document.password.username.focus();
	return false;
  }

  if ( includeSpace(document.password.newpass.value)) {
	alert('Cannot accept space character in password. Please try it again.');
	document.password.newpass.focus();
	return false;
  }
  if (checkString(document.password.newpass.value) == 0) {
	alert('Invalid password !');
	document.password.newpass.focus();
	return false;
  }

  return true;
}

</SCRIPT>
</head>

<BODY>
<blockquote>
<h2><font color="#0000FF">Password Setup</font></h2>

<form action=/goform/admin/formUserPasswordSetup method=POST name="password">
 <table border="0" cellspacing="4" width="500">
  <tr><font size=2>
 This page is used to set the account to access the web server of ADSL Router.
 Empty user name and password will disable the protection.
  </tr>
  <tr><hr size=1 noshade align=top></tr>

<!--
    <tr>
      <td width="20%"><font size=2><b>User Name:</b>
      <select size="1" name="userMode">
      <% checkWrite("userMode"); %>
      </select>
      </td>
    
      <td width="50%"><font size=2><input type="text" name="username" size="20" maxlength="30"></td>
   
    </tr>
-->
    
    <tr>
      <td width="20%"><font size=2><b>Login User:</b></td>
      <td width="50%"><font size=2><% getInfo("login-user"); %></td>
    </tr>
    <tr>
      <td width="20%"><font size=2><b>Old Password:</b></td>
      <td width="50%"><font size=2><input type="password" name="oldpass" size="20" maxlength="30"></td>
    </tr>
    <tr>
      <td width="20%"><font size=2><b>New Password:</b></td>
      <td width="50%"><font size=2><input type="password" name="newpass" size="20" maxlength="30"></td>
    </tr>
    <tr>
      <td width="20%"><font size=2><b>Confirmed Password:</b></td>
      <td width="50%"><font size=2><input type="password" name="confpass" size="20" maxlength="30"></td>
    </tr>
  </table>
   <input type="hidden" value="/admin/user-password.asp" name="submit-url">
  <p><input type="submit" value="Apply Changes" name="save" onClick="return saveChanges()">&nbsp;&nbsp;
  <input type="reset" value="  Reset  " name="reset"></p>
</form>
<blockquote>
</body>
</html>


