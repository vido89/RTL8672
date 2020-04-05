<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Diagnostic Tests</title>
</head>
<script type="text/javascript" src="share.js">
</script>
<script>
var initInf;

function itfSelected()
{
	initInf = document.diagtest.wan_if.value;
}
</script>

<body>
<blockquote>
<h2><font color="#0000FF">Diagnostic Test</font></h2>

<form action=/goform/formDiagTest method=POST name=diagtest>
<table border=0 width=500 cellspacing=4 cellpadding=0>
	<tr><font size=2>
	  The DSL Router is capable of testing your DSL connection. The individual tests are listed below.
	  If a test displays a fail status, click "Run Diagnostic Test" button again to make sure the fail status
	  is consistent.
	</tr>
	<tr><hr size=1 noshade align=top></tr>
</table>

<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr>
    <td><font size=2>Select the Internet Connection: 
		<select name="wan_if"  onChange="itfSelected()">
		<%  if_wan_list("all"); %>
		</select>
    </td>
    <td><input type=submit value="Run Diagnostic Test" name="start"></td>
  </tr>
</table>
<p>
<table width=400 border=0>
	<% lanTest(); %>
</table>
<p>
<table width=400 border=0>
	<% adslTest(); %>
</table>
<p>
<table width=400 border=0>
	<% internetTest(); %>
</table>
  <br>
<input type=hidden value="/diag-test.asp" name="submit-url">
</form>
<script>
	<% initPage("diagTest"); %>
</script>
</blockquote>
</body>

</html>
