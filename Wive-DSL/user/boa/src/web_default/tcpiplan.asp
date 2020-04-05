<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>LAN Interface Setup </title>
<script type="text/javascript" src="share.js">
</script>
<% language=javascript %>
<SCRIPT>
function resetClick()
{
	document.tcpip.reset;
}

function saveChanges()
{
	if (!checkHostIP(document.tcpip.ip, 1))
		return false;
	if (!checkNetmask(document.tcpip.mask, 1))
		return false;
	<% checkIP2 %>
	return true;
}

function disableRadioGroup (radioArrOrButton)
{
  if (radioArrOrButton.type && radioArrOrButton.type == "radio") {
 	var radioButton = radioArrOrButton;
 	var radioArray = radioButton.form[radioButton.name];
  }
  else
 	var radioArray = radioArrOrButton;
 	radioArray.disabled = true;
 	for (var b = 0; b < radioArray.length; b++) {
 	if (radioArray[b].checked) {
 		radioArray.checkedElement = radioArray[b];
 		break;
	}
  }
  for (var b = 0; b < radioArray.length; b++) {
 	radioArray[b].disabled = true;
 	radioArray[b].checkedElement = radioArray.checkedElement;
  }
}

function updateState()
{
  if (document.tcpip.wlanDisabled.value == "ON") {

    disableRadioGroup(document.tcpip.BlockEth2Wir);

  }
}

<% lanScript %>
</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">LAN Interface Setup</font></h2>

<form action=/goform/formTcpipLanSetup method=POST name="tcpip">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><font size=2>
    This page is used to configure the LAN interface of your ADSL Router.
    Here you may change the setting for IP addresses, subnet mask, etc..
  </tr>
  <tr><hr size=1 noshade align=top></tr>
	<input type=hidden name="wlanDisabled" value=<% wlanStatus %>>
  <tr>
      <td width="30%"><font size=2><b>Interface Name:</b></td>
      <td width="70%"><b>br0</b></td>
  </tr>

  <tr>
      <td width="30%"><font size=2><b>IP Address:</b></td>
      <td width="70%"><input type="text" name="ip" size="15" maxlength="15" value=<% getInfo("lan-ip"); %>></td>
  </tr>

  <tr>
      <td width="30%"><font size=2><b>Subnet Mask:</b></td>
      <td width="70%"><input type="text" name="mask" size="15" maxlength="15" value="<% getInfo("lan-subnet"); %>"></td>
  </tr>
  
    <tr></tr><tr></tr>
  </table>
    <% lan_setting(); %>
  <br>
      <input type="submit" value="Apply Changes" name="save" onClick="return saveChanges()">&nbsp;&nbsp;
      <!--input type="reset" value="Undo" name="reset" onClick="resetClick()"-->
      <input type="hidden" value="/tcpiplan.asp" name="submit-url">
<script>
	<% initPage("lan"); %>
	updateState();
</script>
 </form>
</blockquote>
</body>

</html>
