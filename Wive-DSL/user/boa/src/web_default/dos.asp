<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta content="text/html; charset=UTF-8" http-equiv="Content-Type" />
<title>DoS Configuration</title>
<% language=javascript %>
<script type="text/javascript" src="share.js"> </script>
<script>
var dosEnabled=0 ;
function  setDosEnabled() {

}

function dosEnabledClick(){
	if(document.DosCfg.dosEnabled.checked){
		enableTextField(document.DosCfg.sysfloodSYN);
		enableTextField(document.DosCfg.sysfloodFIN);
		enableTextField(document.DosCfg.sysfloodUDP);
		enableTextField(document.DosCfg.sysfloodICMP);
		enableTextField(document.DosCfg.sysfloodSYNcount);
		enableTextField(document.DosCfg.sysfloodFINcount);
		enableTextField(document.DosCfg.sysfloodUDPcount);
		enableTextField(document.DosCfg.sysfloodICMPcount);
		enableTextField(document.DosCfg.ipfloodSYN);
		enableTextField(document.DosCfg.ipfloodFIN);
		enableTextField(document.DosCfg.ipfloodUDP);
		enableTextField(document.DosCfg.ipfloodICMP);
		enableTextField(document.DosCfg.ipfloodSYNcount);
		enableTextField(document.DosCfg.ipfloodFINcount);
		enableTextField(document.DosCfg.ipfloodUDPcount);
		enableTextField(document.DosCfg.ipfloodICMPcount);
		enableTextField(document.DosCfg.TCPUDPPortScan);
		enableTextField(document.DosCfg.portscanSensi);
		enableTextField(document.DosCfg.ICMPSmurfEnabled);
		enableTextField(document.DosCfg.IPLandEnabled);
		enableTextField(document.DosCfg.IPSpoofEnabled);
		enableTextField(document.DosCfg.IPTearDropEnabled);
		enableTextField(document.DosCfg.PingOfDeathEnabled);
		enableTextField(document.DosCfg.TCPScanEnabled);
		enableTextField(document.DosCfg.TCPSynWithDataEnabled);
		enableTextField(document.DosCfg.UDPBombEnabled);
		enableTextField(document.DosCfg.UDPEchoChargenEnabled);
		enableTextField(document.DosCfg.sourceIPblock);
		enableTextField(document.DosCfg.IPblockTime);
	}
	else{
		document.DosCfg.sysfloodSYN.checked=0;
		document.DosCfg.sysfloodFIN.checked=0;
		document.DosCfg.sysfloodUDP.checked=0;
		document.DosCfg.sysfloodICMP.checked=0;
		document.DosCfg.ipfloodSYN.checked=0;
		document.DosCfg.ipfloodFIN.checked=0;
		document.DosCfg.ipfloodUDP.checked=0;
		document.DosCfg.ipfloodICMP.checked=0;
		document.DosCfg.TCPUDPPortScan.checked=0;
		document.DosCfg.ICMPSmurfEnabled.checked=0;
		document.DosCfg.IPLandEnabled.checked=0;
		document.DosCfg.IPSpoofEnabled.checked=0;
		document.DosCfg.IPTearDropEnabled.checked=0;
		document.DosCfg.PingOfDeathEnabled.checked=0;
		document.DosCfg.TCPScanEnabled.checked=0;
		document.DosCfg.TCPSynWithDataEnabled.checked=0;
		document.DosCfg.UDPBombEnabled.checked=0;
		document.DosCfg.UDPEchoChargenEnabled.checked=0;
		document.DosCfg.sourceIPblock.checked=0;
		disableTextField(document.DosCfg.sysfloodSYN);
		disableTextField(document.DosCfg.sysfloodFIN);
		disableTextField(document.DosCfg.sysfloodUDP);
		disableTextField(document.DosCfg.sysfloodICMP);
		disableTextField(document.DosCfg.sysfloodSYNcount);
		disableTextField(document.DosCfg.sysfloodFINcount);
		disableTextField(document.DosCfg.sysfloodUDPcount);
		disableTextField(document.DosCfg.sysfloodICMPcount);
		disableTextField(document.DosCfg.ipfloodSYN);
		disableTextField(document.DosCfg.ipfloodFIN);
		disableTextField(document.DosCfg.ipfloodUDP);
		disableTextField(document.DosCfg.ipfloodICMP);
		disableTextField(document.DosCfg.ipfloodSYNcount);
		disableTextField(document.DosCfg.ipfloodFINcount);
		disableTextField(document.DosCfg.ipfloodUDPcount);
		disableTextField(document.DosCfg.ipfloodICMPcount);
		disableTextField(document.DosCfg.TCPUDPPortScan);
		disableTextField(document.DosCfg.portscanSensi);
		disableTextField(document.DosCfg.ICMPSmurfEnabled);
		disableTextField(document.DosCfg.IPLandEnabled);
		disableTextField(document.DosCfg.IPSpoofEnabled);
		disableTextField(document.DosCfg.IPTearDropEnabled);
		disableTextField(document.DosCfg.PingOfDeathEnabled);
		disableTextField(document.DosCfg.TCPScanEnabled);
		disableTextField(document.DosCfg.TCPSynWithDataEnabled);
		disableTextField(document.DosCfg.UDPBombEnabled);
		disableTextField(document.DosCfg.UDPEchoChargenEnabled);
		disableTextField(document.DosCfg.sourceIPblock);
		disableTextField(document.DosCfg.IPblockTime);
	}
}

function SelectAll(){
	if(document.DosCfg.dosEnabled.checked){
		document.DosCfg.sysfloodSYN.checked=1;
		document.DosCfg.sysfloodFIN.checked=1;
		document.DosCfg.sysfloodUDP.checked=1;
		document.DosCfg.sysfloodICMP.checked=1;
		document.DosCfg.ipfloodSYN.checked=1;
		document.DosCfg.ipfloodFIN.checked=1;
		document.DosCfg.ipfloodUDP.checked=1;
		document.DosCfg.ipfloodICMP.checked=1;
		document.DosCfg.TCPUDPPortScan.checked=1;
		document.DosCfg.ICMPSmurfEnabled.checked=1;
		document.DosCfg.IPLandEnabled.checked=1;
		document.DosCfg.IPSpoofEnabled.checked=1;
		document.DosCfg.IPTearDropEnabled.checked=1;
		document.DosCfg.PingOfDeathEnabled.checked=1;
		document.DosCfg.TCPScanEnabled.checked=1;
		document.DosCfg.TCPSynWithDataEnabled.checked=1;
		document.DosCfg.UDPBombEnabled.checked=1;
		document.DosCfg.UDPEchoChargenEnabled.checked=1;
	}
}

function ClearAll(){
	if(document.DosCfg.dosEnabled.checked){
		document.DosCfg.sysfloodSYN.checked=0;
		document.DosCfg.sysfloodFIN.checked=0;
		document.DosCfg.sysfloodUDP.checked=0;
		document.DosCfg.sysfloodICMP.checked=0;
		document.DosCfg.ipfloodSYN.checked=0;
		document.DosCfg.ipfloodFIN.checked=0;
		document.DosCfg.ipfloodUDP.checked=0;
		document.DosCfg.ipfloodICMP.checked=0;
		document.DosCfg.TCPUDPPortScan.checked=0;
		document.DosCfg.ICMPSmurfEnabled.checked=0;
		document.DosCfg.IPLandEnabled.checked=0;
		document.DosCfg.IPSpoofEnabled.checked=0;
		document.DosCfg.IPTearDropEnabled.checked=0;
		document.DosCfg.PingOfDeathEnabled.checked=0;
		document.DosCfg.TCPScanEnabled.checked=0;
		document.DosCfg.TCPSynWithDataEnabled.checked=0;
		document.DosCfg.UDPBombEnabled.checked=0;
		document.DosCfg.UDPEchoChargenEnabled.checked=0;
	}
}


</script>
</head>

<blockquote>
<h2><font color="#0000FF">DoS Setting</font></h2>

<body>
<table border=0 width="600" cellspacing=0 cellpadding=0>
  <tr><td> <font size=2>
  	DoS("denial-of-service") attack which is launched by hacker aims to prevent legal user from taking normal services. In this page you can configure to prevent some kinds of DOS attack.</font><br>
 </td></tr> 	
 <checkWrite("ModifyTip");>
   <tr><hr size=1 noshade align=top></tr>

<form action=/goform/admin/formDosCfg method=POST name="DosCfg">

  <tr><td width="200" ><font size=2><b><input type="checkbox" name="dosEnabled" value="ON" onclick="dosEnabledClick()">&nbsp;&nbsp;Enable DoS Block</b></font></td></tr>
  <br>

  <table border="0" width=600>
  <br>
  <tr><td width="60%">&nbsp;&nbsp;&nbsp;&nbsp;<input type="checkbox" name="sysfloodSYN" value="ON" > <font size=2><b> Whole System Flood: SYN </b></font><br></td>
  <td width="40%"><input type="text" name="sysfloodSYNcount" size="6" maxlength="4" value=<%getInfo("syssynFlood"); %> > <font size=2><b> packets/second</b></font><br></td></tr>
  <tr><td width="60%">&nbsp;&nbsp;&nbsp;&nbsp;<input type="checkbox" name="sysfloodFIN"  value="ON" > <font size=2><b> Whole System Flood: FIN </b></font><br></td>
  <td width="40%"><input type="text" name="sysfloodFINcount" size="6" maxlength="4" value=<%getInfo("sysfinFlood"); %> > <font size=2><b> packets/second</b></font><br></td></tr>
  <tr><td width="60%">&nbsp;&nbsp;&nbsp;&nbsp;<input type="checkbox" name="sysfloodUDP"  value="ON" > <font size=2><b> Whole System Flood: UDP </b></font><br></td>
  <td width="40%"><input type="text" name="sysfloodUDPcount" size="6" maxlength="4" value=<%getInfo("sysudpFlood"); %> > <font size=2><b> packets/second</b></font><br></td></tr>
  <tr><td width="60%">&nbsp;&nbsp;&nbsp;&nbsp;<input type="checkbox" name="sysfloodICMP"  value="ON" > <font size=2><b> Whole System Flood: ICMP </b></font><br></td>
  <td width="40%"><input type="text" name="sysfloodICMPcount" size="6" maxlength="4" value=<%getInfo("sysicmpFlood"); %> > <font size=2><b> packets/second</b></font><br></td></tr>

  <tr><td width="60%">&nbsp;&nbsp;&nbsp;&nbsp;<input type="checkbox" name="ipfloodSYN"  value="ON" > <font size=2><b> Per-Source IP Flood: SYN </b></font><br></td>
  <td width="40%"><input type="text" name="ipfloodSYNcount" size="6" maxlength="4" value=<%getInfo("pipsynFlood"); %> > <font size=2><b> packets/second</b></font><br></td></tr>
  <tr><td width="60%">&nbsp;&nbsp;&nbsp;&nbsp;<input type="checkbox" name="ipfloodFIN"  value="ON" > <font size=2><b> Per-Source IP Flood: FIN </b></font><br></td>
  <td width="40%"><input type="text" name="ipfloodFINcount" size="6" maxlength="4" value=<%getInfo("pipfinFlood"); %> > <font size=2><b> packets/second</b></font><br></td></tr>
  <tr><td width="60%">&nbsp;&nbsp;&nbsp;&nbsp;<input type="checkbox" name="ipfloodUDP"  value="ON" > <font size=2><b> Per-Source IP Flood: UDP </b></font><br></td>
  <td width="40%"><input type="text" name="ipfloodUDPcount" size="6" maxlength="4" value=<%getInfo("pipudpFlood"); %> > <font size=2><b> packets/second</b></font><br></td></tr>
  <tr><td width="60%">&nbsp;&nbsp;&nbsp;&nbsp;<input type="checkbox" name="ipfloodICMP"  value="ON" > <font size=2><b> Per-Source IP Flood: ICMP </b></font><br></td>
  <td width="40%"><input type="text" name="ipfloodICMPcount" size="6" maxlength="4" value=<%getInfo("pipicmpFlood");%> > <font size=2><b> packets/second</b></font><br></td></tr>

  <tr><td width="60%">&nbsp;&nbsp;&nbsp;&nbsp;<input type="checkbox" name="TCPUDPPortScan"  value="ON" > <font size=2><b> TCP/UDP PortScan </b></font></td>
	<td width="40%">
		<select name="portscanSensi">
			<option value=0 > Low </option>
			<option value=1 > High </option>
		</select>
	<font size=2><b> Sensitivity </b></font></td>
  </tr>

  <tr><td width="60%">&nbsp;&nbsp;&nbsp;&nbsp;<input type="checkbox" name="ICMPSmurfEnabled"  value="ON" > <font size=2><b> ICMP Smurf </b></font></td></tr>

  <tr><td width="60%">&nbsp;&nbsp;&nbsp;&nbsp;<input type="checkbox" name="IPLandEnabled" value="ON" > <font size=2><b> IP Land </b></font></td></tr>

  <tr><td width="60%">&nbsp;&nbsp;&nbsp;&nbsp;<input type="checkbox" name="IPSpoofEnabled" value="ON" > <font size=2><b> IP Spoof </b></font></td></tr>

  <tr><td width="60%">&nbsp;&nbsp;&nbsp;&nbsp;<input type="checkbox" name="IPTearDropEnabled" value="ON" > <font size=2><b> IP TearDrop </b></font></td></tr>

  <tr><td width="60%">&nbsp;&nbsp;&nbsp;&nbsp;<input type="checkbox" name="PingOfDeathEnabled"  value="ON" > <font size=2><b> PingOfDeath </b></font></td></tr>

  <tr><td width="60%">&nbsp;&nbsp;&nbsp;&nbsp;<input type="checkbox" name="TCPScanEnabled"  value="ON" > <font size=2><b> TCP Scan </b></font></td></tr>

  <tr><td width="60%">&nbsp;&nbsp;&nbsp;&nbsp;<input type="checkbox" name="TCPSynWithDataEnabled"   value="ON" > <font size=2><b> TCP SynWithData </b></font></td></tr>

  <tr><td width="60%">&nbsp;&nbsp;&nbsp;&nbsp;<input type="checkbox" name="UDPBombEnabled"  value="ON" > <font size=2><b> UDP Bomb </b></font></td></tr>

  <tr><td width="60%">&nbsp;&nbsp;&nbsp;&nbsp;<input type="checkbox" name="UDPEchoChargenEnabled"   value="ON" > <font size=2><b> UDP EchoChargen </b></font></td></tr>

    </table>
    <br>
    <input type="button" value=" Select All " name="selectAll" onClick="SelectAll()">&nbsp;&nbsp;
    <input type="button" value=" Clear " name="clearAll" onClick="ClearAll()">&nbsp;&nbsp;
    <br>
    <br>
  <table border="0" width=600>

  <tr><td width="60%">&nbsp;&nbsp;&nbsp;&nbsp;<input type="checkbox" name="sourceIPblock"  value="ON" > <font size=2><b> Enable Source IP Blocking </b></font></td>

   <td width="40%"><input type="text" name="IPblockTime" size="4" maxlength="3" value=<%getInfo("blockTime"); %> > <font size=2><b> Block Interval(second)</b></font><br></td></tr>

  </table>
  <br>
  <input type="submit" value="Apply Changes" name="enableDos" >&nbsp;&nbsp;

  <input type="hidden" value="/dos.asp" name="submit-url">
<script>
	<% initPage("dos"); %>
</script>
<script>
	dosEnabledClick();
 </script>
</form>
</table>
</body>

</html>
