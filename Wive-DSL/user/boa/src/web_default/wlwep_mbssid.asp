<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>WEP Key Setup</title>

<SCRIPT>

function validateKey(idx, str, len)
{
  if (document.formWep.defaultTxKeyId.selectedIndex==idx && str.length==0) {
	alert('The encryption key you selected as the \'Tx Default Key\' cannot be blank.');
	return 0;
  }
  if (str.length ==0)
  	return 1;

  if ( str.length != len) {
  	idx++;
	alert('Invalid length of Key ' + idx + ' value. It should be ' + len + ' characters.');
	return 0;
  }

  if ( str == "*****" ||
       str == "**********" ||
       str == "*************" ||
       str == "**************************" )
       return 1;

  if (document.formWep.format.selectedIndex==0)
       return 1;

  for (var i=0; i<str.length; i++) {
    if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
			(str.charAt(i) >= 'a' && str.charAt(i) <= 'f') ||
			(str.charAt(i) >= 'A' && str.charAt(i) <= 'F') )
			continue;

	alert("Invalid key value. It should be in hex number (0-9 or a-f).");
	return 0;
  }

  return 1;
}



function updateFormat()
{
  if (document.formWep.length.selectedIndex == 0) {
	document.formWep.format.options[0].text = 'ASCII (5 characters)';
	document.formWep.format.options[1].text = 'Hex (10 characters)';
  }
  else {
	document.formWep.format.options[0].text = 'ASCII (13 characters)';
	document.formWep.format.options[1].text = 'Hex (26 characters)';
  }

/*
  <% type = getIndex("keyType");
     write("document.formWep.format.options[" + type + "].selected = \'true\';");
  %>
*/

  setDefaultKeyValue();
}

function setDefaultKeyValue()
{
  if (document.formWep.length.selectedIndex == 0) {
	if ( document.formWep.format.selectedIndex == 0) {
		document.formWep.key1.maxLength = 5;
		document.formWep.key2.maxLength = 5;
		document.formWep.key3.maxLength = 5;
		document.formWep.key4.maxLength = 5;
		document.formWep.key1.value = "*****";
		document.formWep.key2.value = "*****";
		document.formWep.key3.value = "*****";
		document.formWep.key4.value = "*****";
	}
	else {
		document.formWep.key1.maxLength = 10;
		document.formWep.key2.maxLength = 10;
		document.formWep.key3.maxLength = 10;
		document.formWep.key4.maxLength = 10;
		document.formWep.key1.value = "**********";
		document.formWep.key2.value = "**********";
		document.formWep.key3.value = "**********";
		document.formWep.key4.value = "**********";
	}
  }
  else {
  	if ( document.formWep.format.selectedIndex == 0) {
		document.formWep.key1.maxLength = 13;
		document.formWep.key2.maxLength = 13;
		document.formWep.key3.maxLength = 13;
		document.formWep.key4.maxLength = 13;
		document.formWep.key1.value = "*************";
		document.formWep.key2.value = "*************";
		document.formWep.key3.value = "*************";
		document.formWep.key4.value = "*************";
	}
	else {
		document.formWep.key1.maxLength = 26;
		document.formWep.key2.maxLength = 26;
		document.formWep.key3.maxLength = 26;
		document.formWep.key4.maxLength = 26;
		document.formWep.key1.value = "**************************";
		document.formWep.key2.value = "**************************";
		document.formWep.key3.value = "**************************";
		document.formWep.key4.value = "**************************";
	}
  }
}

function saveChanges()
{
  var keyLen;
  if (document.formWep.length.selectedIndex == 0) {
  	if ( document.formWep.format.selectedIndex == 0)
		keyLen = 5;
	else
		keyLen = 10;
  }
  else {
  	if ( document.formWep.format.selectedIndex == 0)
		keyLen = 13;
	else
		keyLen = 26;
  }

  if (validateKey(0,document.formWep.key1.value, keyLen)==0) {
	document.formWep.key1.focus();
	return false;
  }
  if (validateKey(1,document.formWep.key2.value, keyLen)==0) {
	document.formWep.key2.focus();
	return false;
  }
  if (validateKey(2,document.formWep.key3.value, keyLen)==0) {
	document.formWep.key3.focus();
	return false;
  }
  if (validateKey(3,document.formWep.key4.value, keyLen)==0) {
	document.formWep.key4.focus();
	return false;
  }

  return true;
}



function lengthClick()
{
  updateFormat();
}

function postWEP(wep, wepKeyType, wepDefaultKey) 
{
	document.formWep.length.value = wep;
	document.formWep.format.value = wepKeyType;
	document.formWep.defaultTxKeyId.value = wepDefaultKey;
	
	updateFormat();
}

</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">Wireless WEP Key Setup</font></h2>

<form action=/goform/admin/formWep method=POST name="formWep">
<table border=0 width="500" cellspacing=4>
  <tr><font size=2>
    This page allows you setup the WEP key value. You could choose use 64-bit or
    128-bit as the encryption key, and select ASCII or Hex as the format of input value.
   </tr>
  <tr><hr size=1 noshade align=top></tr>
      <input type="hidden" name="wepEnabled" value="ON" checked>

  <tr>
      <td width="35%"><font size="2"><b>SSID TYPE:</b></font></td>
	<td width="65%"><font size="2">
	  <input type="radio" name="wepSSID" value="root"  <% postSSIDWEP("root"); %>>Root&nbsp;
	  <input type="radio" name="wepSSID" value="vap0"  <% postSSIDWEP("vap0"); %>>VAP0&nbsp;
	  <input type="radio" name="wepSSID" value="vap1"  <% postSSIDWEP("vap1"); %>>VAP1&nbsp;
	  <input type="radio" name="wepSSID" value="vap2"  <% postSSIDWEP("vap2"); %>>VAP2&nbsp;
	  <input type="radio" name="wepSSID" value="vap3"  <% postSSIDWEP("vap3"); %>>VAP3
	</font></td>
  </tr>
          
  <tr>
      <td width="15%"><font size=2><b>Key Length:</b></td>
      <td width="40%"><font size=2><select size="1" name="length" ONCHANGE=lengthClick()>
      			<option value=1>64-bit</option>
			<option value=2>128-bit</option>
      </select></td>
  </tr>

  <tr>
      <td width="15%"><font size=2><b>Key Format:</b></td>
      <td width="40%"><font size=2><select size="1" name="format" ONCHANGE=setDefaultKeyValue()>
     	<option value=1>ASCII</option>
	<option value=2>Hex</option>
       </select></td>
  </tr>

  <tr>
      <td width="15%"><font size=2><b>Default Tx Key:</b></td>
      <td width="40%"><select size="1" name="defaultTxKeyId">
      <option value="1">Key 1</option>
      <option value="2">Key 2</option>
      <option value="3">Key 3</option>
      <option value="4">Key 4</option>
      </select></td>
  </tr>
  <tr>
     <td width="15%"><font size=2><b>Encryption Key 1:</b></td>
     <td width="40%"><font size=2>
     	<input type="text" name="key1" size="26" maxlength="26">
     </td>
  </tr>
  <tr>
     <td width="15%"><font size=2><b>Encryption Key 2:</b></td>
     <td width="40%"><font size=2>
     	<input type="text" name="key2" size="26" maxlength="26">
     </td>
  </tr>
  <tr>
     <td width="15%"><font size=2><b>Encryption Key 3:</b></td>
     <td width="40%"><font size=2>
     	<input type="text" name="key3" size="26" maxlength="26">
     </td>
  </tr>
  <tr>
     <td width="15%"><font size=2><b>Encryption Key 4:</b></td>
     <td width="40%"><font size=2>
     	<input type="text" name="key4" size="26" maxlength="26">
     </td>
  </tr>

  <tr>
     <td colspan=2 width="100%"><br>
     <input type="hidden" value="/admin/wlwep_mbssid.asp" name="submit-url">
     <input type="submit" value="Apply Changes" name="save" onClick="return saveChanges()">&nbsp;&nbsp;
     <input type="button" value="Close" name="close" OnClick=window.close()>&nbsp;&nbsp;
     <input type="reset" value="Reset" name="reset">
     </td>
  </tr>

<script>
   <!--
	<% initPage("wlwep"); %>
   updateFormat();
   -->
</script>


</table>
</form>

</blockquote>
</body>
</html>
