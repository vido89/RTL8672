<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>WLAN Basic Settings</title>
<style>
.on {display:on}
.off {display:none}
</style>
<script type="text/javascript" src="share.js">
</script>
<SCRIPT>
var regDomain, defaultChan, lastBand=0;

function skip () { this.blur(); }
function openWindow(url, windowName) {
	var wide=620;
	var high=420;
	if (document.all)
		var xMax = screen.width, yMax = screen.height;
	else if (document.layers)
		var xMax = window.outerWidth, yMax = window.outerHeight;
	else
	   var xMax = 640, yMax=500;
	var xOffset = (xMax - wide)/2;
	var yOffset = (yMax - high)/3;

	var settings = 'width='+wide+',height='+high+',screenX='+xOffset+',screenY='+yOffset+',top='+yOffset+',left='+xOffset+', resizable=yes, toolbar=no,location=no,directories=no,status=no,menubar=no,scrollbars=yes';

	window.open( url, windowName, settings );
}


function showMacClick(url)
{
//  if (!document.wlanSetup.wlanDisabled.checked)
  if (document.wlanSetup.wlanDisabled.value != "ON")
	openWindow(url, 'showWirelessClient' );
}

function saveChanges()
{
  if (document.wlanSetup.ssid.value=="") {
	alert('SSID cannot be empty!');
	document.wlanSetup.ssid.value = document.wlanSetup.ssid.defaultValue;
	document.wlanSetup.ssid.focus();
	return false;
   }
	if (includeSpace(document.wlanSetup.ssid.value)) {
		alert('Invalid SSID.');
		document.wlanSetup.ssid.focus();
		return false;
	}
	if (checkString(document.wlanSetup.ssid.value) == 0) {
		alert('Invalid SSID.');
		document.wlanSetup.ssid.focus();
		return false;
	}

   if (document.wlanSetup.wlanDisabled.value != "ON") {
   	band = 0;
	if (document.wlanSetup.band.selectedIndex == 0)
		band = 1;
	else if (document.wlanSetup.band.selectedIndex == 1)
		band = 2;
	else if (document.wlanSetup.band.selectedIndex == 2)
		band = 3;
	else if (document.wlanSetup.band.value == 7)
		band = 8;
	else if (document.wlanSetup.band.value == 9)
		band = 10;
	else if (document.wlanSetup.band.value == 10)
		band = 11;
	else
		band = 4;

	basicRate=0;
	operRate=0;
	if (band & 1) {
		basicRate|=0xf;
		operRate|=0xf;
	}
	if ( (band & 2) || (band & 4) ) {
		operRate|=0xff0;
		if (!(band & 1))
			basicRate|=0xff0;
	}
	if (band & 8) {
		if (!(band & 3))
			operRate|=0xfff;	
		if (band & 1)
			basicRate=0xf;
		else if (band & 2)			
			basicRate=0x1f0;
		else
			basicRate=0xf;
	}
	operRate|=basicRate;
	
	document.wlanSetup.basicrates.value = basicRate;
	document.wlanSetup.operrates.value = operRate;
   }

   return true;
}

function disableWLAN()
{
  disableTextField(document.wlanSetup.band);
  disableTextField(document.wlanSetup.mode);
  disableTextField(document.wlanSetup.ssid);
  disableTextField(document.wlanSetup.chanwid);
  disableTextField(document.wlanSetup.ctlband);
  disableTextField(document.wlanSetup.chan);
  disableTextField(document.wlanSetup.txpower);
  disableButton(document.wlanSetup.showMac);
}

function enableWLAN()
{
  enableTextField(document.wlanSetup.band);
  enableTextField(document.wlanSetup.mode);
  enableTextField(document.wlanSetup.ssid);
  enableTextField(document.wlanSetup.chanwid);
  enableTextField(document.wlanSetup.ctlband);
  enableTextField(document.wlanSetup.chan);
  enableTextField(document.wlanSetup.txpower);
  enableButton(document.wlanSetup.showMac);
  enableButton(document.wlanSetup.save);
}

function updateIputState()
{
  if (document.wlanSetup.wlanDisabled.checked == true) {
  	document.wlanSetup.wlanDisabled.value="ON";
 	disableWLAN();
  } else {
  	document.wlanSetup.wlanDisabled.value="OFF";
  	enableWLAN();
  }
  if (document.wlanSetup.chanwid.selectedIndex == 0)
  	disableCheckBox(document.wlanSetup.elements.ctlband);
  else
  	enableCheckBox(document.wlanSetup.elements.ctlband);
  if (document.wlanSetup.band.selectedIndex == 0||
  	document.wlanSetup.band.selectedIndex == 1||
  	document.wlanSetup.band.selectedIndex == 2){
  	if (document.getElementById)
		document.getElementById('optionfor11n').style.display = 'none';
	else if(document.layers == false)
		document.all.optionfor11n.style.display = 'none';
  }
  else if (document.wlanSetup.band.selectedIndex == 3||
  	document.wlanSetup.band.selectedIndex == 4||
  	document.wlanSetup.band.selectedIndex == 5){
  	if (document.getElementById)
		document.getElementById('optionfor11n').style.display = 'block';
	else if(document.layers == false)
		document.all.optionfor11n.style.display = 'block';
  }
}

function showChannel5G()
{
	document.wlanSetup.chan.length=0;

	for (idx=0, chan=36; chan<=64; idx++, chan+=4) {
		document.wlanSetup.chan.options[idx] = new Option(chan, chan, false, false);
		if (chan == defaultChan)
			document.wlanSetup.chan.selectedIndex = idx;
	}
	document.wlanSetup.chan.length = idx;
}


function showChannel2G()
{
	start = 0;
	end = 0;
	if (regDomain==1 || regDomain==2) {
		start = 1;
		end = 11;
	}
	if (regDomain==3) {
		start = 1;
		end = 13;
	}
	if (regDomain==4) {
		start = 10;
		end = 11;
	}
	if (regDomain==5) {
		start = 10;
		end = 13;
	}
	if (regDomain==6) {
		start = 1;
		end = 14;
	}
	
	document.wlanSetup.chan.length=0;
	idx=0;
	document.wlanSetup.chan.options[idx] = new Option("Auto", 0, false, false);
	if (0 == defaultChan) {
		document.wlanSetup.chan.selectedIndex = 0;
	}
	idx++;	
	
	for (chan=start; chan<=end; chan++, idx++) {
		document.wlanSetup.chan.options[idx] = new Option(chan, chan, false, false);
		if (chan == defaultChan)
			document.wlanSetup.chan.selectedIndex = idx;
	}
	document.wlanSetup.chan.length = idx;
}

function updateChan()
{
/*
  if (document.wlanSetup.band.selectedIndex == 3)
     currentBand = 2;
  else
*/
     currentBand = 1;

  if (lastBand != currentBand) {
  	lastBand = currentBand;
  	if (currentBand == 2)
		showChannel5G();
  	else
		showChannel2G();
  }
  if (document.wlanSetup.chanwid.selectedIndex == 0)
  	disableCheckBox(document.wlanSetup.elements.ctlband);
  else
  	enableCheckBox(document.wlanSetup.elements.ctlband);
  if (document.wlanSetup.band.selectedIndex == 0||
  	document.wlanSetup.band.selectedIndex == 1||
  	document.wlanSetup.band.selectedIndex == 2){
  	if (document.getElementById)
		document.getElementById('optionfor11n').style.display = 'none';
	else if(document.layers == false)
		document.all.optionfor11n.style.display = 'none';
  }
  else if (document.wlanSetup.band.selectedIndex == 3||
  	document.wlanSetup.band.selectedIndex == 4||
  	document.wlanSetup.band.selectedIndex == 5){
  	if (document.getElementById)
		document.getElementById('optionfor11n').style.display = 'block';
	else if(document.layers == false)
		document.all.optionfor11n.style.display = 'block';
  }
}

function updateChan1()
{
   if (document.wlanSetup.chanwid.selectedIndex == 0)
  	disableCheckBox(document.wlanSetup.elements.ctlband);
   else
  	enableCheckBox(document.wlanSetup.elements.ctlband);
}

</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">Wireless Basic Settings</font></h2>

<form action=/goform/admin/formWlanSetup method=POST name="wlanSetup">
<table border=0 width="500" cellspacing=4>
  <tr><font size=2>
 This page is used to configure the parameters for wireless LAN clients which
 may connect to your Access Point. Here you may change wireless encryption settings
 as well as wireless network parameters.
  </tr>
  <tr><hr size=1 noshade align=top></tr>
  <tr>
      <td width="100%" colspan=2><font size=2><b>
   	<input type="checkbox" name="wlanDisabled" value=<% wlanStatus %> 
   	 ONCLICK=updateIputState()>&nbsp;&nbsp;Disable Wireless LAN Interface</b>
     </td>
  </tr>

  <tr>
      <td width="26%"><font size=2><b>Band:</b></td>
      <td width="74%"><font size=2><select size=1 name=band onChange="updateChan()">
     	  	<option value=0>2.4 GHz (B)</option>
     	  	<option value=1>2.4 GHz (G)</option>
     	  	<option value=2>2.4 GHz (B+G)</option>
      		<% checkWrite("wlband"); %>      		
	 </select>
      </td>
  </tr>

  <tr>
      <td width="26%"><font size=2><b>Mode:</b></td>
      <td width="74%"><font size=2><select size="1" name="mode">
      		<% checkWrite("wlmode"); %>      		
      		</select>
      </td>
  </tr>
  
  </table>

  <table border="0" width=500>
  <tr>
      <td width="26%"><font size=2><b>SSID:</b></td>
      <td width="74%"><font size=2><input type=text name=ssid size="25" maxlength="32" value="<% getInfo("ssid"); %>">
      </td>
  </tr>
  </table>
  <div  id="optionfor11n" style="display:none">
  <table border="0" width=500>
  <tr>
      <td width="26%"><font size=2><b>Channel Width:</b></td>
      <td width="74%"><font size=2><select size="1" name="chanwid" onChange="updateChan1()">
            	<% checkWrite("wlchanwid"); %>      		
      		</select>
      </td>
  </tr>
  <tr>
      <td width="26%"><font size=2><b>Conntrol Sideband:</b></td>
      <td width="74%"><font size=2><select size="1" name="ctlband">
      	      	<% checkWrite("wlctlband"); %>      		
      		</select>
      </td>
  </tr> 
  </table>
  </div>
  <table border="0" width=500>
  <tr>
      <td width="26%"><font size=2><b>Channel Number:</b></td>
      <td width="74%"><font size=2><select size="1" name="chan"> </select></td>
    <SCRIPT>
    	<% checkWrite("wl_chno"); %>
	updateChan();
    </SCRIPT>
  </tr>
  <tr>
      <td width="26%"><font size=2><b>Radio Power (mW):</b></td>
      <td width="74%"><font size=2><select size="1" name="txpower">
      		<% checkWrite("txpower"); %>      		
      		</select>
      </td>
  </tr>
  <tr>
      <td width="26%"><font size=2><b>Associated Clients:</b></td>
      <td width="74%"><font size=2><input type="button" value="Show Active Clients" name="showMac" onClick="showMacClick('/admin/wlstatbl.asp')">
      </td>
  </tr>
  </table>
  <br>
  <table border=0 width=500 cellspacing=0 cellpadding=0>
  <tr>
     <input type="hidden" value="/admin/wlbasic.asp" name="submit-url">
     <input type="submit" value="Apply Changes" name="save" onClick="return saveChanges()">&nbsp;&nbsp;
     
     <input type="hidden" name="basicrates" value=0>
     <input type="hidden" name="operrates" value=0>
  </tr>
  <script>
	<% initPage("wlbasic"); %>
	updateIputState();
  </script>
</table>
</form>

</blockquote>
</body>

</html>
