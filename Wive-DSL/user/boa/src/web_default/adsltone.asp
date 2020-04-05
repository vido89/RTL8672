<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>ADSL Tone Configuration Table</title>
<% language=javascript %>
<SCRIPT>

function maskAllClick()
{
   if ( !confirm('Do you really want to MASK all Tones?') ) {
	return false;
  }
  else
	return true;
}

function unmaskAllClick()
{
   if ( !confirm('Do you really want to UNMASK all Tones?') ) {
	return false;
  }
  else
	return true;
}


</SCRIPT>

</head>

<body>
<blockquote>
<h2><font color="#0000FF">ADSL Tone Configuration Table</font></h2>

<table border=0 width="480" cellspacing=0 cellpadding=0>
  <tr><font size=2>
 This page let user to mark the designate tones to be masked.
  </tr>
  <tr><hr size=1 noshade align=top></tr>
</table>


<form action=/goform/formSetAdslTone method=POST name="formToneTbl">

<table border=0 width=400 cellspacing=4 cellpadding=0>
<% adslToneConfDiagList(); %>
</table>

<input type="submit" value="Apply Changes" name="apply">&nbsp;&nbsp;
<input type="submit" value="Mask All" name="maskAll" onClick="return maskAllClick()">&nbsp;&nbsp;&nbsp;
<input type="submit" value="UnMask All" name="unmaskAll" onClick="return unmaskAllClick()">&nbsp;&nbsp;&nbsp;
<input type="hidden" value="/adsltone.asp" name="submit-url"> 
<input type="button" value=" Close Page" name="close" onClick="javascript: window.close();">

</form>
</blockquote>
</body>

</html>

