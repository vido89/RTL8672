<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta HTTP-EQUIV='Pragma' CONTENT='no-cache'>
<meta http-equiv="Content-Type" content="text/html">
<title>WDS Settings</title>
<script type="text/javascript" src="share.js"> </script>
<script>
function addClick()
{
//  var str = document.formWlWdsAdd.mac.value;

//  if (!document.formWlWdsAdd.wlanWdsEnabled.checked)
//	return true;

	if (!checkMac(document.formWlWdsAdd.mac, 1))
		return false;
	if (checkString(document.formWlWdsAdd.comment.value) == 0) {
		alert('Invalid comment.');
		document.formWlWdsAdd.comment.focus();
		return false;
	}
	return true;
  /*if ( str.length == 0)
  	return true;

  if ( str.length < 12) {
	alert("Input MAC address is not complete. It should be 12 digits in hex.");
	document.formWlWdsAdd.mac.focus();
	return false;
  }

  for (var i=0; i<str.length; i++) {
    if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
			(str.charAt(i) >= 'a' && str.charAt(i) <= 'f') ||
			(str.charAt(i) >= 'A' && str.charAt(i) <= 'F') )
			continue;

	alert("Invalid MAC address. It should be in hex number (0-9 or a-f).");
	document.formWlWdsAdd.mac.focus();
	return false;
  }
  return true;*/
}


function disableDelButton()
{
	disableButton(document.formWlWdsDel.deleteSelWdsMac);
	disableButton(document.formWlWdsDel.deleteAllWdsMac);
}

function enableWds()
{
  enableTextField(document.formWlWdsAdd.mac);
  enableTextField(document.formWlWdsAdd.comment);
  //enableButton(document.formWlWdsAdd.showWep);
  //enableButton(document.formWlWdsAdd.showAp);
}

function disableWds()
{
  disableTextField(document.formWlWdsAdd.mac);
  disableTextField(document.formWlWdsAdd.comment);
  
  	//disableButton(document.formWlWdsAdd.showWep);
  	//disableButton(document.formWlWdsAdd.showAp);
}

function updateState()
{
  if (document.formWlWdsAdd.wlanDisabled.value == "ON") {
      disableTextField(document.formWlWdsAdd.wlanWdsEnabled);
	  disableButton(document.formWlWdsAdd.reset);
	  disableButton(document.formWlWdsAdd.addWdsMac);
	  disableButton(document.formWlWdsAdd.wdsSet);
	  disableWds();
	  disableDelButton();
  } else {
    //wlanDisabled = <% getMibValue("wlanDisabled"); %> ;
    //wlanMode = <% getMibValue("wlanMode"); %>;
    //if(wlanDisabled || wlanMode == 0 || wlanMode ==1){
    if(document.formWlWdsAdd.wlanWdsEnabled.disabled == true){
	  disableButton(document.formWlWdsAdd.reset);
	  disableButton(document.formWlWdsAdd.addWdsMac);
	  disableButton(document.formWlWdsAdd.wdsSet);
	  disableWds();
	  disableDelButton();
    }else{
      if (document.formWlWdsAdd.wlanWdsEnabled.checked) {
	  enableButton(document.formWlWdsAdd.reset);
	  enableButton(document.formWlWdsAdd.addWdsMac);
 	    enableWds();
      	}
      else {
	  disableButton(document.formWlWdsAdd.reset);
	  disableButton(document.formWlWdsAdd.addWdsMac);
  	    disableWds();
      }
    }
  }
}

function showApClick(url)
{
  if (document.formWlWdsAdd.wlanWdsEnabled.checked)
	openWindow(url, 'showWdsStatistics',620,440 );
}

function showWepClick(url)
{
  if (document.formWlWdsAdd.wlanWdsEnabled.checked)
	openWindow(url, 'showWdsWep',620,340 );
}

</script>
</head>
<body>
<blockquote>
<h2><font color="#0000FF">WDS Settings </font></h2>
<form action=/goform/admin/formWlWds method=POST name="formWlWdsAdd">
<table border=0 width="500" cellspacing=4 cellpadding=0>
<tr><font size=2>
 Wireless Distribution System uses wireless media to communicate with other
 APs, like the Ethernet does. To do this, you must set these APs in the same
 channel and set MAC address of other APs which you want to communicate with
 in the table and then enable the WDS.
</font></tr>
</table>

<table border=0 width="500" cellspacing=4 cellpadding=0>
<tr><hr size=1 noshade align=top></tr>
<input type=hidden name="wlanDisabled" value=<% wlanStatus %>>

<!--tr><p><font size=2><b>
   <input type="checkbox" name="wlanWdsEnabled" value="ON" <% checkWrite("wlanWdsEnabled"); %>  onclick="updateState()">&nbsp;&nbsp;Enable WDS</b></p><tr-->
<tr>
   <td><font size=2><b>
   <input type="checkbox" name="wlanWdsEnabled" value="ON" <% checkWrite("wlanWdsEnabled"); %>  onclick="updateState()">&nbsp;&nbsp;Enable WDS</b></td>
   <td><input type="submit" value="Apply Changes" name="wdsSet">&nbsp;&nbsp;</td>
</tr>
</table>

<table border=0 width="500" cellspacing=4 cellpadding=0>
<tr><hr size=1 noshade align=top><br></tr>
<tr><p><font size=2><b>Add WDS AP:&nbsp;&nbsp;&nbsp;&nbsp</b>
   <font size=2><b>MAC Address </b> <input type="text" name="mac" size="15" maxlength="12">&nbsp;&nbsp;
   	<b><font size=2>Comment </b> <input type="text" name="comment" size="16" maxlength="19"></font>
</p>

<p><input type="submit" value="Add" name="addWdsMac" onClick="return addClick()">&nbsp;&nbsp;
   <input type="reset" value="Reset" name="reset">&nbsp;&nbsp;
   <input type="hidden" value="/admin/wlwds.asp" name="submit-url">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;

   <!--<font size=2><input type="button" value="Set Security" name="showWep" onClick="showWepClick('/wlwdsenp.asp')">&nbsp;&nbsp
   
   <font size=2><input type="button" value="Show Statistics" name="showAp" onClick="showApClick('/wlwdstbl.asp')">&nbsp;&nbsp
   -->
</p></tr>
</table>
</form>

<form action=/goform/admin/formWlWds method=POST name="formWlWdsDel">
  <table border=0 width=440>
  <tr><font size=2><b>Current WDS AP List:</b></font></tr>
  <% wlWdsList(); %>
  </table>
  <br>
  <input type="submit" value="Delete Selected" name="deleteSelWdsMac" onClick="return deleteClick()">&nbsp;&nbsp;
  <input type="submit" value="Delete All" name="deleteAllWdsMac" onClick="return deleteAllClick()">&nbsp;&nbsp;&nbsp;
  <input type="hidden" value="/admin/wlwds.asp" name="submit-url">
 <script>
 	<% initPage("wlwds"); %>
	updateState();
 	<% checkWrite("wlanWDSNum"); %>
 </script>
</form>

</blockquote>
</body>
</html>
