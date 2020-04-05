<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>ADSL Driver</title>

<SCRIPT>

function channelSNR(url) {
    window.open(url, '','');
}
</SCRIPT>

</head>
<body>
<blockquote>

<h2><b><font color="#0000FF">ADSL Driver Interface</font></b></h2>
<form action=/goform/formAdslDrv method=POST name="adsl-drv">

<table border=0 width="600" cellspacing=4 cellpadding=0>
  <tr><font size=2>
  This page shows the ADSL Driver Interface.
  </tr>
<tr><hr size=1 noshade align=top><br></tr>

      <font size=3>

      <br>
      <p><b>Modem Status: </b>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<% getInfo("adsl-drv-modemstatus"); %>
      <hr size=1 noshade align=top>

      <br>
      <p><b>Link Status: </b>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;upstreamRate(<% getInfo("adsl-drv-uprate"); %> kb/sec),
         &nbsp;downstreamRate(<% getInfo("adsl-drv-downrate"); %> kb/sec),&nbsp;dataMode(<% getInfo("adsl-drv-latency"); %>),
         &nbsp;
      <hr size=1 noshade align=top>

      <br>
      <p><b>Modem Line: </b>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type="submit" value="connect" name="adsl-drv-modem-enable" >&nbsp;&nbsp;
                      <input type="submit" value="disconnect" name="adsl-drv-modem-disable" >
      <hr size=1 noshade align=top>

      <br>
      <p><b>ADSL Mode: </b>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<font size=2><select size="1" name="adsl-mode">
         <% choice = checkWrite("adsl-line-mode"); %>            
        </select></font>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type="submit" value="Set" name="adsl-drv-mode" >
      <hr size=1 noshade align=top>

      <br>
      <p><b>Driver Version: </b>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<% getInfo("adsl-drv-version"); %>
      <hr size=1 noshade align=top>

      <br>
      <p><b>Driver Build: </b>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<% getInfo("adsl-drv-build"); %>
      <hr size=1 noshade align=top>

      <br>
      <p><b>Force modem to re-train: </b>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type="submit" value="retrain" name="adsl-drv-retrain" >
      <hr size=1 noshade align=top>

      <br>
      <p><b>Re-handshaking count: </b>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<% getInfo("adsl-drv-rehskcount"); %>
      <hr size=1 noshade align=top>

      <br>
      <p><b>Average SNR: </b>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<% getInfo("adsl-drv-avgsnr"); %>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type="button" value="Show Channel SNR" name="adsl-drv-channel-snr" onClick="channelSNR('/adv/adsl-drv-snr-tbl.asp')">

      <hr size=1 noshade align=top>
                                                                                                                                                
      <br>
      <p><b>SNR Margin: </b>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<% getInfo("adsl-drv-snrmargin"); %>
      <hr size=1 noshade align=top>

      <br>
      <p><b>Performance Management Data: </b>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<% getInfo("adsl-drv-pmdata"); %>
      <hr size=1 noshade align=top>

      <br>
      <p><b>Near End ID: </b>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<% getInfo("adsl-drv-neid"); %>
      <hr size=1 noshade align=top>

      <br>
      <p><b>Far End ID: </b>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<% getInfo("adsl-drv-feid"); %>
      <hr size=1 noshade align=top>

      <br>
      <p><b>Near End Line Data: </b>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<% getInfo("adsl-drv-neld"); %>
      <hr size=1 noshade align=top>

      <br>
      <p><b>Far End Line Data: </b>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<% getInfo("adsl-drv-feld"); %>
      <hr size=1 noshade align=top>

      <br>
      <p><b>Loss Data: </b>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<% getInfo("adsl-drv-lossdata"); %>
      <hr size=1 noshade align=top>

      <br>
      <p><b>15 min Loss Data: </b>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<% getInfo("adsl-drv-15min-lossdata"); %>
      <hr size=1 noshade align=top>

      <br>
      <p><b>1 day Loss Data: </b>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<% getInfo("adsl-drv-1day-lossdata"); %>
      <hr size=1 noshade align=top>

      <br>
      <p><b>Loop Attenuation: </b>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<% getInfo("adsl-drv-loopatt"); %>
      <hr size=1 noshade align=top>

      <br>
      <p><b>Bit Swap: </b>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<% getInfo("adsl-drv-bitswap"); %>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type="submit" value="Enable" name="adsl-drv-bitswap-enable" >&nbsp;&nbsp;
                      <input type="submit" value="Disable" name="adsl-drv-bitswap-disable" >
      <hr size=1 noshade align=top>

      <br>
      <p><b>Pilot Relocation: </b>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<% getInfo("adsl-drv-pilotrelocat"); %>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type="submit" value="Enable" name="adsl-drv-pilotrelocat-enable" >&nbsp;&nbsp;
                      <input type="submit" value="Disable" name="adsl-drv-pilotrelocat-disable" >
      <hr size=1 noshade align=top>

      <br>
      <p><b>Trellis: </b>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<% getInfo("adsl-drv-trellis"); %>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type="submit" value="Enable" name="adsl-drv-trellis-enable" >&nbsp;&nbsp;
                      <input type="submit" value="Disable" name="adsl-drv-trellis-disable" >
      <hr size=1 noshade align=top>

      <br>
      <p><b>Configuration: </b>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<% getInfo("adsl-drv-config"); %>
      <hr size=1 noshade align=top>

      <br>
      <p><b>ADSL Time: </b>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<% getInfo("adsl-drv-adsltime"); %>
      <hr size=1 noshade align=top>

      <br>
      <p><b>ADSL Log: </b>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type="submit" value="Enable" name="adsl-drv-log-enable" >&nbsp;&nbsp;
                      <input type="submit" value="Disable" name="adsl-drv-log-disable" >
      <hr size=1 noshade align=top>

      <br>
      <p><b>Channel Bitload: </b>
      <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type="button" value="Show Channel Bitload" name="adsl-drv-channel-bitload" onClick="channelSNR('/adv/adsl-drv-bitload-tbl.asp')">
      <hr size=1 noshade align=top>

      <input type="hidden" value="/adv/adsl-drv.asp" name="submit-url">
</table>
</form>

<br>


</blockquote>

</body>

</html>
