<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>ADSL Diagnostics </title>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">Diagnostics -- ADSL</font></h2>

<form action=/goform/formDiagAdsl method=POST name=diag_adsl>
<table border=0 width=500 cellspacing=4 cellpadding=0>
	<tr><font size=2>
	  Adsl Tone Diagnostics. Only ADSL2/2+ support this function.
	</tr>
	<tr><hr size=1 noshade align=top></tr>
</table>

<input type=submit value="  Start  " name=start disabled="disabled">
<input type=hidden value="/adv/adsl-diag.asp" name="submit-url">
<table border=0 cellspacing=4 cellpadding=0>
<% adslToneDiagTbl(); %>
</table>
<p>
<table border=0 width=500 cellspacing=4 cellpadding=0>
<% adslToneDiagList(); %>
</table>
  <br>
<script>
	<% initPage("diagdsl"); %>
</script>
</form>
</blockquote>
</body>

</html>
