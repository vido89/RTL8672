<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<META HTTP-EQUIV=Refresh CONTENT="10; URL=adsl-statis.asp">
<meta http-equiv="Content-Type" content="text/html">
<title>ADSL Statistics</title>
</head>

<body>
<blockquote>
<!--h2><font color="#0000FF">Statistics -- ADSL</font></h2-->
<h2><font color="#0000FF">Statistics -- ADSL Line</font></h2>

<form action=/goform/formStatAdsl method=POST name=sts_adsl>
<table border=0 width=500 cellspacing=4 cellpadding=0>
	<!--tr><font size=2>
	  Adsl line statistics.
	</tr-->
	<tr><hr size=1 noshade align=top></tr>
</table>

<table border=0 width=500 cellspacing=4 cellpadding=0>
	<tr>
		<th align=left bgcolor=#c0c0c0><font size=2>Mode</th>
		<td bgcolor=#f0f0f0><% getInfo("adsl-drv-mode"); %></td>
	</tr>
	<tr>
		<th align=left bgcolor=#c0c0c0><font size=2>Latency</th>
		<td bgcolor=#f0f0f0><% getInfo("adsl-drv-latency"); %></td>
	</tr>	
	<tr>
		<th align=left bgcolor=#c0c0c0><font size=2>Trellis Coding</th>
		<td bgcolor=#f0f0f0><% getInfo("adsl-drv-trellis"); %></td>
	</tr>
	<tr>
		<th align=left bgcolor=#c0c0c0><font size=2>Status</th>
		<td bgcolor=#f0f0f0><% getInfo("adsl-drv-state"); %></td>
	</tr>
	<tr>
		<th align=left bgcolor=#c0c0c0><font size=2>Power Level</th>
		<td bgcolor=#f0f0f0><% getInfo("adsl-drv-pwlevel"); %></td>
	</tr>
	<tr>
		<th align=left bgcolor=#c0c0c0><font size=2>Uptime</th>
		<td bgcolor=#f0f0f0><% DSLuptime(); %></td>
	</tr>
</table>
<br>
<table border=0 width=500 cellspacing=4 cellpadding=0>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0>
		<th>Downstream<th>Upstream</th>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>SNR Margin (dB)</th>
		<td><% getInfo("adsl-drv-snr-ds"); %></td><td><% getInfo("adsl-drv-snr-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Attenuation (dB)</th>
		<td><% getInfo("adsl-drv-lpatt-ds"); %></td><td><% getInfo("adsl-drv-lpatt-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Output Power (dBm)</th>
		<td><% getInfo("adsl-drv-power-ds"); %></td><td><% getInfo("adsl-drv-power-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Attainable Rate (Kbps)</th>
		<td><% getInfo("adsl-drv-attrate-ds"); %></td><td><% getInfo("adsl-drv-attrate-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Rate (Kbps)</th>
		<td><% getInfo("adsl-drv-rate-ds"); %></td><td><% getInfo("adsl-drv-rate-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>K (number of bytes in DMT frame)</th>
		<td><% getInfo("adsl-drv-pms-k-ds"); %></td><td><% getInfo("adsl-drv-pms-k-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>R (number of check bytes in RS code word)</th>
		<td><% getInfo("adsl-drv-pms-r-ds"); %></td><td><% getInfo("adsl-drv-pms-r-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>S (RS code word size in DMT frame)</th>
		<td><% getInfo("adsl-drv-pms-s-ds"); %></td><td><% getInfo("adsl-drv-pms-s-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>D (interleaver depth)</th>
		<td><% getInfo("adsl-drv-pms-d-ds"); %></td><td><% getInfo("adsl-drv-pms-d-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Delay (msec)</th>
		<td><% getInfo("adsl-drv-pms-delay-ds"); %></td><td><% getInfo("adsl-drv-pms-delay-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>FEC</th>
		<td><% getInfo("adsl-drv-fec-ds"); %></td><td><% getInfo("adsl-drv-fec-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>CRC</th>
		<td><% getInfo("adsl-drv-crc-ds"); %></td><td><% getInfo("adsl-drv-crc-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Total ES</th>
		<td><% getInfo("adsl-drv-es-ds"); %></td><td><% getInfo("adsl-drv-es-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Total SES</th>
		<td><% getInfo("adsl-drv-ses-ds"); %></td><td><% getInfo("adsl-drv-ses-us"); %></td>
	</tr>
	<tr bgcolor=#f0f0f0>
		<th align=left bgcolor=#c0c0c0><font size=2>Total UAS</th>
		<td><% getInfo("adsl-drv-uas-ds"); %></td><td><% getInfo("adsl-drv-uas-us"); %></td>
	</tr>
</table>
<p>
</form>
</blockquote>
</body>

</html>
