<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>ADSL Channel SNR Table</title>
<SCRIPT LANGUAGE="JavaScript1.2" SRC="graph.js"></SCRIPT>
</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">ADSL Channel SNR Table</font></h2>

<table border=0 width="800" cellspacing=0 cellpadding=0>
  <tr><font size=2>
 This table shows the channel SNR
  </tr>
  <tr><hr size=1 noshade align=top></tr>
</table>


<form action=/goform/formAdslDrvSnrTbl method=POST name="formAdslDrvSnrTbl">


<table width=400 border=0">
<SCRIPT LANGUAGE="JavaScript1.2">
<% adslDrvSnrTblGraph(); %>
</SCRIPT>
</table>

<table border=0 width="800" cellspacing=0 cellpadding=0>
  <tr><hr size=1 noshade align=top></tr>
<% adslDrvSnrTblList(); %>
</table>

  <input type="button" value=" Close " name="close" onClick="javascript: window.close();"></p>
</form>
</blockquote>
</body>

</html>
