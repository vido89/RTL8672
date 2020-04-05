<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>DMZ Host</title>
<script type="text/javascript" src="share.js">
</script>
<script>
function skip () { this.blur(); }
function saveClick()
{
//  if (!document.formDMZ.enabled.checked)
  if (document.formDMZ.dmzcap[0].checked)
 	return true;      

/*  if ( validateKey( document.formDMZ.ip.value ) == 0 ) {
	alert("Invalid IP address value. It should be the decimal number (0-9).");
	document.formDMZ.ip.focus();
	return false;
  }
  if( IsLoopBackIP( document.formDMZ.ip.value)==1 ) {
	alert("Invalid IP address value.");
	document.formDMZ.ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formDMZ.ip.value,1,0,254) ) {
      	alert('Invalid IP address range in 1st digit. It should be 0-254.');
	document.formDMZ.ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formDMZ.ip.value,2,0,255) ) {
      	alert('Invalid IP address range in 2nd digit. It should be 0-255.');
	document.formDMZ.ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formDMZ.ip.value,3,0,255) ) {
      	alert('Invalid IP address range in 3rd digit. It should be 0-255.');
	document.formDMZ.ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formDMZ.ip.value,4,1,254) ) {
      	alert('Invalid IP address range in 4th digit. It should be 1-254.');
	document.formDMZ.ip.focus();
	return false;
  }*/
  if (!checkHostIP(document.formDMZ.ip, 1))
	return false;
  return true;
}

function updateState()
{
//  if (document.formDMZ.enabled.checked) {
  if (document.formDMZ.dmzcap[1].checked) {
 	enableTextField(document.formDMZ.ip);
  }
  else {
 	disableTextField(document.formDMZ.ip);
  }
}


</script>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">DMZ</font></h2>

<table border=0 width="500" cellspacing=0 cellpadding=0>
<tr><font size=2>

A Demilitarized Zone is used to provide Internet services without sacrificing
unauthorized access to its local private network. Typically, the DMZ host contains
devices accessible to Internet traffic, such as Web (HTTP ) servers, FTP servers,
SMTP (e-mail) servers and DNS servers.

</font></tr>

<tr><hr size=1 noshade align=top></tr>

<form action=/goform/formDMZ method=POST name="formDMZ">
<!--tr><td><font size=2><b>
   	<input type="checkbox" name="enabled" value="ON" <% checkWrite("dmzEn"); %>
   	ONCLICK=updateState()>&nbsp;&nbsp;Enable DMZ</b>
    </td>
</tr-->
<tr><td><font size=2><b>DMZ Host:</b></td>
      <td><font size=2>
	<input type="radio" value="0" name="dmzcap" <% checkWrite("dmz-cap0"); %> onClick="updateState()">Disable&nbsp;&nbsp;
	<input type="radio" value="1" name="dmzcap" <% checkWrite("dmz-cap1"); %> onClick="updateState()">Enable&nbsp;&nbsp;
      </td>
</font></td></tr>
<tr>
	<td><font size=2><b>DMZ Host IP Address: </b></td>
	<td><input type="text" name="ip" size="15" maxlength="15" value=<% getInfo("dmzHost"); %> ></font></td>
	<td><input type="submit" value="Apply Changes" name="save" onClick="return saveClick()">&nbsp;&nbsp;</td>
</tr>
<tr><td>
   <br>
        <!--input type="reset" value="Reset" name="reset"-->
        <input type="hidden" value="/fw-dmz.asp" name="submit-url">
</td></tr>
     <script> updateState(); </script>
</form>
</table>
</blockquote>
</body>
</html>
