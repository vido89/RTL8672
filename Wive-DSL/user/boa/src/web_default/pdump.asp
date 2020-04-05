<html>
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Packet dump</title>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">Packet dump</font></h2>

<form action=/goform/formCapture method=POST name="ping">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><font size=2>
	This page is used to start or stop a Wireshark packet capture.<br>
    You need to return to this page to Stop it.<br>
	<a href ="http://www.tcpdump.org/tcpdump_man.html" target=_blank">Click here for the documentation of the additional arguments.</a>
  </tr>
  <tr><hr size=1 noshade align=top></tr>

  <tr>
      <td width="30%"><font size=2><b>Additional arguments:</b></td>
      <td width="70%"><input type="text" name="tcpdumpArgs" value="-s 1500" size="50" maxlength="50"></td>
      <input type="hidden" value="yes" name="dostart">
  </tr>

</table>

  <br>
      <input type="submit" value="Start" name="start">
      <input type="hidden" value="/pdump.asp" name="submit-url">
 </form>
<p>

<form action=/goform/formCapture method=POST name="ping">
      <input type="submit" value="Stop" name="stop">
      <input type="hidden" value="/pdump.asp" name="submit-url">
      <input type="hidden" value="no" name="dostart">
 </form>


</blockquote>
</body>

</html>
