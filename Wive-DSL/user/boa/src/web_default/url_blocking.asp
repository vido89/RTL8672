<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>URL Filtering Configuration </title>
<script type="text/javascript" src="share.js">
</script>
<% language=javascript %>
<SCRIPT>

function addClick()
{	
	return true;
}

function addFQDNClick()
{	
	if (document.url.urlFQDN.value=="") {
		alert("Please enter the Blocked FQDN !");
		document.url.urlFQDN.focus();
		return false;
	}
	if (document.url.urlFQDN.value.length == 0 ) {
		if (!confirm('FQDN is empty.\nPlease enter the blocked FQDN') ) {
			document.url.urlFQDN.focus();
			return false;
  		}
		else
			return true;
  	}
  	if (includeSpace(document.url.urlFQDN.value)) {
		alert('Cannot accept space character in Blocked FQDN. Please try it again.');
		document.url.urlFQDN.focus();
		return false;
 	}
	if (checkString(document.url.urlFQDN.value) == 0) {
		alert('Invalid Blocked FQDN !');
		document.url.urlFQDN.focus();
		return false;
	}
	return true;
}

function addKeywordClick()
{	
	if (document.url.Keywd.value=="") {
		alert("Please enter the Blocked keyword !");
		document.url.Keywd.focus();
		return false;
	}
	if (document.url.Keywd.value.length == 0 ) {
		if (!confirm('Keyword is empty.\nPlease enter the blocked keyword') ) {
			document.url.Keywd.focus();
			return false;
  		}
		else
			return true;
  	}
  	if (includeSpace(document.url.Keywd.value)) {
		alert('Cannot accept space character in Blocked keyword. Please try it again.');
		document.url.Keywd.focus();
		return false;
 	}
	if (checkString(document.url.Keywd.value) == 0) {
		alert('Invalid Blocked keyword !');
		document.url.Keywd.focus();
		return false;
	}
	return true;
}
	
function disableDelFQDNButton()
{
  if (verifyBrowser() != "ns") {
	disableButton(document.url.delFQDN);
	disableButton(document.url.delFAllQDN);
  }
}
	
function disableDelKeywdButton()
{
  if (verifyBrowser() != "ns") {
	disableButton(document.url.delKeywd);
	disableButton(document.url.delAllKeywd);
  }
}
</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">URL Blocking Configuration</font></h2>

<form action=/goform/formURL method=POST name="url">
<table border=0 width="500" cellspacing=0 cellpadding=0>
  <tr><font size=2>
    This page is used to configure the Blocked FQDN(Such as tw.yahoo.com) and filtered keyword. Here you can add/delete FQDN and filtered keyword.
  </tr>
</table>
<table border=0 width="500" cellspacing=0 cellpadding=0>
<tr><hr size=1 noshade align=top></tr>
  
<tr>
	<td><font size=2><b>URL Blocking:</b></td>
	<td><font size=2>
		<input type="radio" value="0" name="urlcap" <% checkWrite("url-cap0"); %>>Disable&nbsp;&nbsp;
		<input type="radio" value="1" name="urlcap" <% checkWrite("url-cap1"); %>>Enable&nbsp;&nbsp;
	</td>
	<td><input type="submit" value="Apply Changes" name="apply">&nbsp;&nbsp;<td>
</tr>       
</table>
<tr><hr size=1 noshade align=top></tr>
<tr>
	<td><font size=2><b>FQDN:</b></td>
	<td><input type="text" name="urlFQDN" size="15" maxlength="125">&nbsp;&nbsp;</td>
	<td><input type="submit" value="Add" name="addFQDN" onClick="return addFQDNClick()">&nbsp;&nbsp;</td>
</tr> 
<br>
<br>
<table border=0 width="300" cellspacing=4 cellpadding=0>
<tr><font size=2><b>URL Blocking Table:</b></font></tr>
  <% showURLTable(); %>
</table>
<br>
<input type="submit" value="Delete Selected" name="delFQDN" onClick="return deleteClick()">&nbsp;&nbsp;
<input type="submit" value="Delete All" name="delFAllQDN" onClick="return deleteAllClick()">&nbsp;&nbsp;&nbsp;
 <script>
 	<% checkWrite("FQDNNum"); %>
  </script>
<br>
<br>
<tr><hr size=1 noshade align=top></tr>  
<tr>
	<td><font size=2><b>Keyword:</b></td>
	<td><input type="text" name="Keywd" size="15" maxlength="18">&nbsp;&nbsp;</td>
	<td><input type="submit" value="Add" name="addKeywd" onClick="return addKeywordClick()">&nbsp;&nbsp;</td>
</tr>
<br>
<br>
<table border=0 width="300" cellspacing=4 cellpadding=0>
<tr><font size=2><b>Keyword Filtering Table:</b></font></tr>
  <% showKeywdTable(); %>
</table>
<br>
<input type="submit" value="Delete Selected" name="delKeywd" onClick="return deleteClick()">&nbsp;&nbsp;
<input type="submit" value="Delete All" name="delAllKeywd" onClick="return deleteAllClick()">&nbsp;&nbsp;&nbsp;
<input type="hidden" value="/url_blocking.asp" name="submit-url">
 <script>
 	<% checkWrite("keywdNum"); %>
  </script>
</form>
</blockquote>
</body>

</html>
