<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>SNMP Protocol Configuration</title>
<script type="text/javascript" src="share.js">
</script>
<% language=javascript %>
<SCRIPT>
function saveChanges()
{
  //if (!checkIP(document.snmpTable.snmpTrapIpAddr))	// Jenny, sync check IP function
  if (!checkHostIP(document.snmpTable.snmpTrapIpAddr, 1))
	return false;
  /*
  if (document.snmpTable.snmpTrapIpAddr.value=="") {
	alert("Trap IP address cannot be empty! It should be filled with 4 digit numbers as xxx.xxx.xxx.xxx.");
	document.snmpTable.snmpTrapIpAddr.value = document.snmpTable.snmpTrapIpAddr.defaultValue;
	document.snmpTable.snmpTrapIpAddr.focus();
	return false;
  }
  if ( validateKey( document.snmpTable.snmpTrapIpAddr.value ) == 0 ) {
	alert("Invalid IP address value. It should be the decimal number (0-9).");
	document.snmpTable.snmpTrapIpAddr.value = document.snmpTable.snmpTrapIpAddr.defaultValue;
	document.snmpTable.snmpTrapIpAddr.focus();
	return false;
  }
  if( IsLoopBackIP( document.snmpTable.snmpTrapIpAddr.value)==1 ) {
		alert("Invalid Trap IP address value.");
		document.snmpTable.snmpTrapIpAddr.focus();
		return false;
  }
  if ( !checkDigitRange(document.snmpTable.snmpTrapIpAddr.value,1,0,254) ) {
      	alert('Invalid Trap IP address range in 1st digit. It should be 0-254.');
	document.snmpTable.snmpTrapIpAddr.value = document.snmpTable.snmpTrapIpAddr.defaultValue;
	document.snmpTable.snmpTrapIpAddr.focus();
	return false;
  }
  if ( !checkDigitRange(document.snmpTable.snmpTrapIpAddr.value,2,0,255) ) {
      	alert('Invalid Trap IP address range in 2nd digit. It should be 0-255.');
	document.snmpTable.snmpTrapIpAddr.value = document.snmpTable.snmpTrapIpAddr.defaultValue;
	document.snmpTable.snmpTrapIpAddr.focus();
	return false;
  }
  if ( !checkDigitRange(document.snmpTable.snmpTrapIpAddr.value,3,0,255) ) {
      	alert('Invalid Trap IP address range in 3rd digit. It should be 0-255.');
	document.snmpTable.snmpTrapIpAddr.value = document.snmpTable.snmpTrapIpAddr.defaultValue;
	document.snmpTable.snmpTrapIpAddr.focus();
	return false;
  }
  if ( !checkDigitRange(document.snmpTable.snmpTrapIpAddr.value,4,1,254) ) {
      	alert('Invalid Trap IP address range in 4th digit. It should be 1-254.');
	document.snmpTable.snmpTrapIpAddr.value = document.snmpTable.snmpTrapIpAddr.defaultValue;
	document.snmpTable.snmpTrapIpAddr.focus();
	return false;
  }
  */
  if ( validateKey( document.snmpTable.snmpSysObjectID.value ) == 0 ) {
	alert("Invalid Object ID value. It should be the decimal number (0-9) or '.'" );
	document.snmpTable.snmpSysObjectID.value = document.snmpTable.snmpSysObjectID.defaultValue;
	document.snmpTable.snmpSysObjectID.focus();
	return false;
  }
  
  // count the numbers of '.' on OID
  var i=0;
  var str=document.snmpTable.snmpSysObjectID.value;
  while (str.length!=0) {
	if ( str.charAt(0) == '.' ) {
		i++;
	}	
	str = str.substring(1);
  }
  
  //document.write(i);
  if ( i!=6 ) {
  	alert("Invalid Object ID value. It should be fill with OID string as 1.3.6.1.4.1.X");
  	document.snmpTable.snmpSysObjectID.value = document.snmpTable.snmpSysObjectID.defaultValue;
	document.snmpTable.snmpSysObjectID.focus();
  	return false;
  }  
  
  // Check if the OID's prefix is "1.3.6.1.4.1"
  var str2 = document.snmpTable.snmpSysObjectID.value.substring(0, 11);
  //document.write(str2);	
  if( str2!="1.3.6.1.4.1" ) {
  	alert("Invalid Object ID value. It should be fill with prefix OID string as 1.3.6.1.4.1");
  	document.snmpTable.snmpSysObjectID.value = document.snmpTable.snmpSysObjectID.defaultValue;
	document.snmpTable.snmpSysObjectID.focus();
	return false;
  }
  
  
  if (checkString(document.snmpTable.snmpSysDescr.value) == 0) {
	alert('Invalid System Description.');
	document.snmpTable.snmpSysDescr.focus();
	return false;
  }
  if (checkString(document.snmpTable.snmpSysContact.value) == 0) {
	alert('Invalid System Contact.');
	document.snmpTable.snmpSysContact.focus();
	return false;
  }
  if (checkString(document.snmpTable.snmpSysName.value) == 0) {
	alert('Invalid System Name.');
	document.snmpTable.snmpSysName.focus();
	return false;
  }
  if (checkString(document.snmpTable.snmpSysLocation.value) == 0) {
	alert('Invalid System Location.');
	document.snmpTable.snmpSysLocation.focus();
	return false;
  }
  if (checkString(document.snmpTable.snmpCommunityRO.value) == 0) {
	alert('Invalid Community name (read-only).');
	document.snmpTable.snmpCommunityRO.focus();
	return false;
  }
  if (checkString(document.snmpTable.snmpCommunityRW.value) == 0) {
	alert('Invalid Community name (write-only).');
	document.snmpTable.snmpCommunityRW.focus();
	return false;
  }
  
 return true;
}

</SCRIPT>

</head>

<body>
<blockquote>
<h2><font color="#0000FF">SNMP Protocol Configuration</font></h2>

<form action=/goform/formSnmpConfig method=POST name="snmpTable">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><font size=2>
    This page is used to configure the SNMP protocol. Here you may change
    the setting for system description, trap ip address, community name, etc..
  </tr>
  <tr><hr size=1 noshade align=top></tr>

  <tr>
      <td><font size=2><b>SNMP:</b></td>
      <td><font size=2>
      	<input type="radio" value="0" name="snmp_enable" <% checkWrite("snmpd-on"); %> >Disable&nbsp;&nbsp;
     	<input type="radio" value="1" name="snmp_enable" <% checkWrite("snmpd-off"); %> >Enable&nbsp;&nbsp;
      </td>     
  </tr>
  
  <tr>
      <td width="40%"><font size=2><b>System Description</b></td>
      <td width="60%"><input type="text" name="snmpSysDescr" size="50" maxlength="64" value="<% getInfo("snmpSysDescr"); %>"></td>
  </tr>

  <tr>
      <td width="40%"><font size=2><b>System Contact</b></td>
      <td width="60%"><input type="text" name="snmpSysContact" size="50" maxlength="64" value="<% getInfo("snmpSysContact"); %>"></td>
  </tr>

  <tr>
      <td width="40%"><font size=2><b>System Name</b></td>
      <td width="60%"><input type="text" name="snmpSysName" size="50" maxlength="64" value="<% getInfo("snmpSysName"); %>"></td>
  </tr>

  <tr>
      <td width="40%"><font size=2><b>System Location</b></td>
      <td width="60%"><input type="text" name="snmpSysLocation" size="50" maxlength="64" value="<% getInfo("snmpSysLocation"); %>"></td>
  </tr>

  <tr>
      <td width="40%"><font size=2><b>System Object ID</b></td>
      <td width="60%"><input type="text" name="snmpSysObjectID" size="50" maxlength="64" value="<% getInfo("snmpSysObjectID"); %>"></td>
  </tr>

  <tr>
      <td width="40%"><font size=2><b>Trap IP Address</b></td>
      <td width="60%"><input type="text" name="snmpTrapIpAddr" size="15" maxlength="15" value=<% getInfo("snmpTrapIpAddr"); %>></td>
  </tr>


  <tr>
      <td width="40%"><font size=2><b>Community name (read-only)</b></td>
      <td width="60%"><input type="text" name="snmpCommunityRO" size="50" maxlength="64" value="<% getInfo("snmpCommunityRO"); %>"></td>
  </tr>


  <tr>
      <td width="40%"><font size=2><b>Community name (write-only)</b></td>
      <td width="60%"><input type="text" name="snmpCommunityRW" size="50" maxlength="64" value="<% getInfo("snmpCommunityRW"); %>"></td>
  </tr>

  </table>
  <br>
      <input type="submit" value="Apply Changes" name="save" onClick="return saveChanges()">&nbsp;&nbsp;
      <input type="reset" value="Reset" name="reset">
      <input type="hidden" value="/snmp.asp" name="submit-url">
 </form>
</blockquote>
</body>

</html>
