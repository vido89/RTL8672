<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Domain Blocking Configuration </title>
<script type="text/javascript" src="share.js">
</script>
<% language=javascript %>
<SCRIPT>

function addClick()
{
	if (document.domainblk.blkDomain.value=="") {
		alert("Please enter the Blocked Domain !");
		document.domainblk.blkDomain.focus();
		return false;
	}
	
	if ( document.domainblk.blkDomain.value.length == 0 ) {
		if ( !confirm('Domain string is empty.\nPlease enter the blocked Domain') ) {
			document.domainblk.blkDomain.focus();
			return false;
  		}
		else
			return true;
  	}
  	
  	if ( includeSpace(document.domainblk.blkDomain.value)) {
		alert('Cannot accept space character in Blocked Domain. Please try it again.');
		document.domainblk.blkDomain.focus();
		return false;
 	}
	if (checkString(document.domainblk.blkDomain.value) == 0) {
		alert('Invalid Blocked Domain !');
		document.domainblk.blkDomain.focus();
		return false;
	}
  	
	return true;
}


	
function disableDelButton()
{
  if (verifyBrowser() != "ns") {
	disableButton(document.domainblk.delDomain);
	disableButton(document.domainblk.delAllDomain);
  }
}
</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">Domain Blocking Configuration</font></h2>

<form action=/goform/formDOMAINBLK method=POST name="domainblk">
<table border=0 width="500" cellspacing=0 cellpadding=0>
  <tr><font size=2>
    This page is used to configure the Blocked domain. Here you can add/delete the blocked domain.
  </tr>
</table>
<table border=0 width="500" cellspacing=0 cellpadding=0>
  <tr><hr size=1 noshade align=top></tr>
  
  <tr>
	<td><font size=2><b>Domain Blocking:</b></td>
	<td><font size=2>
		<input type="radio" value="0" name="domainblkcap" <% checkWrite("domainblk-cap0"); %>>Disable&nbsp;&nbsp;
		<input type="radio" value="1" name="domainblkcap" <% checkWrite("domainblk-cap1"); %>>Enable
	</td>
	<td><input type="submit" value="Apply Changes" name="apply">&nbsp;&nbsp;</td>
  </tr>  
</table>
  
<tr><hr size=1 noshade align=top></tr>
<tr>
	<td><font size=2><b>Domain:</b></td>
	<td><input type="text" name="blkDomain" size="15" maxlength="50">&nbsp;&nbsp;</td>
	<td><input type="submit" value="Add" name="addDomain" onClick="return addClick()">&nbsp;&nbsp;</td>
</tr>  
  
<br>
<br>
<table border=0 width="300" cellspacing=4 cellpadding=0>
  <tr><font size=2><b>Domain Block Table:</b></font></tr>
  <% showDOMAINBLKTable(); %>
</table>
<br>
	<input type="submit" value="Delete Selected" name="delDomain" onClick="return deleteClick()">&nbsp;&nbsp;
	<input type="submit" value="Delete All" name="delAllDomain" onClick="return deleteAllClick()">&nbsp;&nbsp;&nbsp;
	<input type="hidden" value="/domainblk.asp" name="submit-url">
 <script>
 	<% checkWrite("domainNum"); %>
  </script>
</form>
</blockquote>
</body>

</html>
