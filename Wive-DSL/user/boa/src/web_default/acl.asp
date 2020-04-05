
<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>ACL Configuration </title>
<script type="text/javascript" src="share.js">
</script>
<% language=javascript %>
<SCRIPT>
function postACL( enable, interface, aclIP )
{
	document.acl.enable.checked = enable;		
	document.acl.interface.value = interface;	
	document.acl.aclIP.value = aclIP;	
}

function addClick()
{
/*	if (document.acl.aclIP.value=="") {
		alert("Enter Source Network ID !");
		document.acl.aclIP.focus();
		return false;
	}

	if (document.acl.aclMask.value=="") {
		alert("Enter Subnet Mask !");
		document.acl.aclMask.focus();
		return false;
	}
	
	if ( validateKey( document.acl.aclIP.value ) == 0 ) {
		alert("Invalid Source value.");
		document.acl.aclIP.focus();
		return false;
	}
	if( IsLoopBackIP( document.acl.aclIP.value)==1 ) {
		alert("Invalid IP address value.");
		document.acl.aclIP.focus();
		return false;
  	}
	if ( !checkDigitRange(document.acl.aclIP.value,1,0,254) ) {
		alert('Invalid Source range in 1st digit. It should be 0-254.');
		document.acl.aclIP.focus();
		return false;
	}
	if ( !checkDigitRange(document.acl.aclIP.value,2,0,255) ) {
		alert('Invalid Source range in 2nd digit. It should be 0-255.');
		document.acl.aclIP.focus();
		return false;
	}
	if ( !checkDigitRange(document.acl.aclIP.value,3,0,255) ) {
		alert('Invalid Source range in 3rd digit. It should be 0-255.');
		document.acl.aclIP.focus();
		return false;
	}
	if ( !checkDigitRange(document.acl.aclIP.value,4,0,254) ) {
		alert('Invalid Source range in 4th digit. It should be 0-254.');
		document.acl.aclIP.focus();
		return false;
	}	

	if ( validateKey( document.acl.aclMask.value ) == 0 ) {
		alert("Invalid Source value.");
		document.acl.aclMask.focus();
		return false;
	}
	if( IsLoopBackIP( document.acl.aclMask.value)==1 ) {
		alert("Invalid IP address value.");
		document.acl.aclMask.focus();
		return false;
  	}
	if ( !checkDigitRange(document.acl.aclMask.value,1,0,255) ) {
		alert('Invalid Source range in 1st digit. It should be 0-255.');
		document.acl.aclMask.focus();
		return false;
	}
	if ( !checkDigitRange(document.acl.aclMask.value,2,0,255) ) {
		alert('Invalid Source range in 2nd digit. It should be 0-255.');
		document.acl.aclMask.focus();
		return false;
	}
	if ( !checkDigitRange(document.acl.aclMask.value,3,0,255) ) {
		alert('Invalid Source range in 3rd digit. It should be 0-255.');
		document.acl.aclMask.focus();
		return false;
	}
	if ( !checkDigitRange(document.acl.aclMask.value,4,0,255) ) {
		alert('Invalid Source range in 4th digit. It should be 0-255.');
		document.acl.aclMask.focus();
		return false;
	}	
*/	
	if (!checkNetIP(document.acl.aclIP, 1))
		return false;
	if (!checkNetmask(document.acl.aclMask, 1))
		return false;
	return true;
}


	
function disableDelButton()
{
  if (verifyBrowser() != "ns") {
	disableButton(document.acl.delIP);
	disableButton(document.acl.delAllIP);
  }
}
</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">ACL Configuration</font></h2>

<form action=/goform/admin/formACL method=POST name="acl">
<table border=0 width="500" cellspacing=0 cellpadding=0>
  <tr><font size=2>
    This page is used to configure the IP Address for Access Control List. If ACL is enabled, just these IP address that in the ACL Table
    can access CPE. Here you can add/delete IP Address.
  </tr>
</table>
<table border=0 width="500" cellspacing=0 cellpadding=0>
<input type="hidden" name="lan_ip" value=<% getInfo("lan-ip"); %>>
<input type="hidden" name="lan_mask" value=<% getInfo("lan-subnet"); %>>
  <tr><hr size=1 noshade align=top></tr>
  
  <tr>
      <td><font size=2><b>ACL Capability:</b></td>
      <td><font size=2>
      	<input type="radio" value="0" name="aclcap" <% checkWrite("acl-cap0"); %>>Disable&nbsp;&nbsp;
     	<input type="radio" value="1" name="aclcap" <% checkWrite("acl-cap1"); %>>Enable
      </td>
      <td><input type="submit" value="Apply Changes" name="apply">&nbsp;&nbsp; </td>
  </tr> 
</table>
 
 
<table border=0 width="500" cellspacing=0 cellpadding=0> 
  <tr><hr size=1 noshade align=top></tr>
  <br>
  <tr>
      <td width="30%"><font size=2><b>Enable:</b></td>
      <td width="70%"><input type="checkbox" name="enable" value="1" checked></td>
  </tr>
  
  <tr>
      <td width="30%"><font size=2><b>Interface:</b></td>
      <td width="70%">
      <select size="1" name="interface">
      <option value="0">LAN</option>
      <option value="1">WAN</option>
      </select>
      </td>
  </tr>

  <tr>
      <td width="30%"><font size=2><b>IP Address:</b></td>
      <td width="70%"><input type="text" name="aclIP" size="15" maxlength="15"></td>
  </tr>  
  
  <tr>
      <td width="30%"><font size=2><b>Subnet Mask:</b></td>
      <td width="50%"><input type="text" name="aclMask" size="15" maxlength="15"></td>
      <td width="20%"><input type="submit" value="Add" name="addIP" onClick="return addClick()">&nbsp;&nbsp;</td>
  </tr>  
  
</table>
  
  <!--input type="submit" value="Update" name="updateACL" onClick="return addClick()">&nbsp;&nbsp;
  </tr-->
  
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><hr size=1 noshade align=top></tr>
  <tr><font size=2><b>ACL Table:</b></font></tr>
  <% showACLTable(); %>
</table>
  <br>
      <input type="submit" value="Delete Selected" name="delIP" onClick="return deleteClick()">&nbsp;&nbsp;
      <input type="submit" value="Delete All" name="delAllIP" onClick="return deleteAllClick()">&nbsp;&nbsp;&nbsp;
      <input type="hidden" value="/admin/acl.asp" name="submit-url">
 <script>
 	<% checkWrite("aclNum"); %>
  </script>
</form>
</blockquote>
</body>

</html>
