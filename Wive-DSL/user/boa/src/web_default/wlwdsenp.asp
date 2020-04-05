<html>
<! Copyright (c) Realtek Semiconductor Corp., 2004. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>WDS Security Setup</title>
<script type="text/javascript" src="common.js"> </script>
<SCRIPT>
var defPskLen=new Array()
var defPskFormat=new Array();
var wlan_idx= <% write(getIndex("wlan_idx")); %> ;

function setWepKeyLen(form)
{
  if (form.elements["encrypt"+wlan_idx].selectedIndex == 1) {
	if ( form.elements["format"+wlan_idx].selectedIndex == 0) {
		form.elements["wepKey"+wlan_idx].maxLength = 5;
		form.elements["wepKey"+wlan_idx].value = "*****";
	}
	else {
		form.elements["wepKey"+wlan_idx].maxLength = 10;
		form.elements["wepKey"+wlan_idx].value = "**********";
	}
  }
  else {
	if ( form.elements["format"+wlan_idx].selectedIndex == 0) {
		form.elements["wepKey"+wlan_idx].maxLength = 13;
		form.elements["wepKey"+wlan_idx].value = "*************";
	}
	else {
		form.elements["wepKey"+wlan_idx].maxLength = 26;
		form.elements["wepKey"+wlan_idx].value = "**************************";
	}
  }
}

function disableWEP(form)
{
  disableTextField(form.elements["format"+wlan_idx]);
  disableTextField(form.elements["wepKey"+wlan_idx]);
}

function disableWPA(form)
{
  disableTextField(form.elements["pskFormat"+wlan_idx]);
  disableTextField(form.elements["pskValue"+wlan_idx]);
}


function enableWEP(form)
{
  enableTextField(form.elements["format"+wlan_idx]);
  enableTextField(form.elements["wepKey"+wlan_idx]);
}

function enableWPA(form)
{
  enableTextField(form.elements["pskFormat"+wlan_idx]);
  enableTextField(form.elements["pskValue"+wlan_idx]);
}

function updateEncryptState(form)
{
  if (form.elements["encrypt"+wlan_idx].selectedIndex == 0) {
  	disableWEP(form);
	disableWPA(form);
  }
  if (form.elements["encrypt"+wlan_idx].selectedIndex == 1 || form.elements["encrypt"+wlan_idx].selectedIndex == 2) {
	setWepKeyLen(document.formWdsEncrypt);
 	enableWEP(form);
	disableWPA(form);
  }
  if (form.elements["encrypt"+wlan_idx].selectedIndex == 3 || form.elements["encrypt"+wlan_idx].selectedIndex == 4) {
 	disableWEP(form);
	enableWPA(form);
  }
}

function saveChangesWep(form)
{
  var keyLen;
  if (form.elements["encrypt"+wlan_idx].selectedIndex == 1) {
  	if ( form.elements["format"+wlan_idx].selectedIndex == 0)
		keyLen = 5;
	else
		keyLen = 10;
  }
  else {
  	if ( form.elements["format"+wlan_idx].selectedIndex == 0)
		keyLen = 13;
	else
		keyLen = 26;
  }
  if (validateKey_wep(form, -1,form.elements["wepKey"+wlan_idx].value, keyLen, wlan_idx)==0) {
	form.elements["wepKey"+wlan_idx].focus();
	return false;
  }
  return true;
}

function saveChanges(form)
{
  if (form.elements["encrypt"+wlan_idx].selectedIndex == 0)
  	return true;
  else if (form.elements["encrypt"+wlan_idx].selectedIndex == 1 || form.elements["encrypt"+wlan_idx].selectedIndex == 2)
 	return saveChangesWep(form, wlan_idx);
  else
  	return check_wpa_psk(form,wlan_idx );
}

</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">WDS Security Setup <% if (getIndex("wlan_num")>1) write("-wlan"+(getIndex("wlan_idx")+1));
%></font></h2>

<form action=/goform/formWdsEncrypt method=POST name="formWdsEncrypt">
<table border=0 width="500" cellspacing=4>
  <tr><font size=2>
    This page allows you setup the wireless security for WDS. When enabled, you must
    make sure each WDS device has adopted the same encryption algorithm and Key.
  </tr>
  <tr><hr size=1 noshade align=top></tr>
  <tr>
      <td width="30%"><font size=2><b>Encryption:</b></font></td>
      <td width="70%"><font size=2><b>
      	<select size="1" name="encrypt<% write(getIndex("wlan_idx")); %>" onChange="updateEncryptState(document.formWdsEncrypt)">
          <option <% choice = getIndex("wdsEncrypt"); if (choice == 0) write("selected"); %> value="0">None</option>
          <option <% choice = getIndex("wdsEncrypt"); if (choice == 1) write("selected"); %> value="1">WEP 64bits</option>
          <option <% choice = getIndex("wdsEncrypt"); if (choice == 2) write("selected"); %> value="2">WEP 128bits</option>
          <option <% choice = getIndex("wdsEncrypt"); if (choice == 3) write("selected"); %> value="3">WPA (TKIP)</option>
          <option <% choice = getIndex("wdsEncrypt"); if (choice == 4) write("selected"); %> value="4">WPA2 (AES)</option>
      </b></font></td>
  </tr>
  <tr>
      <td width="30%"><font size=2><b>WEP Key Format:</b></font></td>
      <td width="70%"><font size=2><select size="1" name="format<% write(getIndex("wlan_idx")); %>" ONCHANGE=setWepKeyLen(document.formWdsEncrypt)>
     	<option <% if (getIndex("wdsWepFormat")== 0) write("selected"); %> value=0>ASCII (5 characters)</option>
	<option <% if (getIndex("wdsWepFormat")) write("selected"); %> value=1>Hex (10 characters)</option>
       </select></font></td>
  </tr>
  <tr>
      <td width="30%"><font size=2><b>WEP Key:</b></font></td>
      <td width="70%"><font size=2>
     	<input type="text" name="wepKey<% write(getIndex("wlan_idx")); %>" size="26" maxlength="26">
      </font></td>
  </tr>
  <tr>
      <td width="30%"><font size="2"><b>Pre-Shared Key Format:</b></font></td>
      <td width="70%"><font size="2"><select size="1" name="pskFormat<% write(getIndex("wlan_idx")); %>">
          <option value="0" <% if (getIndex("wdsPskFormat")==0) write("selected");%>>Passphrase</option>
          <option value="1" <% if (getIndex("wdsPskFormat")) write("selected");%>>Hex (64 characters)</option>
      </select></font></td>
  </tr>
  <tr>
      <td width="30%"><font size="2"><b>Pre-Shared Key:</b></font></td>
      <td width="70%"><font size="2"><input type="text" name="pskValue<% write(getIndex("wlan_idx")); %>" size="40" maxlength="64" value=<% getInfo("wdsPskValue");%>>
      </font></td>
  </tr>
  <script>
     	form = document.formWdsEncrypt ;
     updateEncryptState(document.formWdsEncrypt);
	defPskLen[wlan_idx] = form.elements["pskValue"+wlan_idx].value.length;
	defPskFormat[wlan_idx] = form.elements["pskFormat"+wlan_idx].selectedIndex;

  </script>
  <tr>
     <td colspan=2 width="100%"><br>
     <input type="hidden" value="/wlwdsenp.asp" name="submit-url">
     <input type="submit" value="Apply Changes" name="save" onClick="return saveChanges(document.formWdsEncrypt)">&nbsp;&nbsp;
     <input type="button" value="Close" name="close" OnClick=window.close()>&nbsp;&nbsp;
     <input type="reset" value="Reset" name="reset">
     </td>
  </tr>
</table>
</form>

</blockquote>
</body>
</html>
