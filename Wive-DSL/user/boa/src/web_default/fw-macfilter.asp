<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>MAC Filtering</title>
<script type="text/javascript" src="share.js">
</script>
<script>
function skip () { this.blur(); }
function addClick()
{
//  if (document.formFilterAdd.srcmac.value=="" )
//	return true;
  if (document.formFilterAdd.srcmac.value=="" && document.formFilterAdd.dstmac.value=="") {
	alert("Input MAC address.");
	return false;
  }

	if (document.formFilterAdd.srcmac.value != "") {
		if (!checkMac(document.formFilterAdd.srcmac, 0))
			return false;
	}
	if (document.formFilterAdd.dstmac.value != "") {
		if (!checkMac(document.formFilterAdd.dstmac, 0))
			return false;
	}
	return true;
/*  var str = document.formFilterAdd.srcmac.value;
  if ( str.length < 12) {
	alert("Input MAC address is not complete. It should be 12 digits in hex.");
	document.formFilterAdd.srcmac.focus();
	return false;
  }

  for (var i=0; i<str.length; i++) {
    if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
			(str.charAt(i) >= 'a' && str.charAt(i) <= 'f') ||
			(str.charAt(i) >= 'A' && str.charAt(i) <= 'F') )
			continue;

	alert("Invalid MAC address. It should be in hex number (0-9 or a-f).");
	document.formFilterAdd.srcmac.focus();
	return false;
  }
  return true;*/
}

function disableDelButton()
{
  if (verifyBrowser() != "ns") {
	disableButton(document.formFilterDel.deleteSelFilterMac);
	disableButton(document.formFilterDel.deleteAllFilterMac);
  }
}

</script>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">MAC Filtering</font></h2>

<table border=0 width="500" cellspacing=0 cellpadding=0>
<tr><td><font size=2>
 Entries in this table are used to restrict certain types of data packets from your local
 network to Internet through the Gateway.  Use of such filters can be helpful in securing
 or restricting your local network.
</font></td></tr>

<tr><td><hr size=1 noshade align=top></td></tr>

<form action=/goform/admin/formFilter method=POST name="formFilterDefault">
<tr><td><font size=2><b>Outgoing Default Action</b>&nbsp;&nbsp;
   	<input type="radio" name="outAct" value=0 <% checkWrite("macf_out_act0"); %>>Deny&nbsp;&nbsp;
   	<input type="radio" name="outAct" value=1 <% checkWrite("macf_out_act1"); %>>Allow&nbsp;&nbsp;
</font></td><tr>
<tr><td><font size=2><b>Incoming Default Action</b>&nbsp;&nbsp;
   	<input type="radio" name="inAct" value=0 <% checkWrite("macf_in_act0"); %>>Deny&nbsp;&nbsp;
   	<input type="radio" name="inAct" value=1 <% checkWrite("macf_in_act1"); %>>Allow&nbsp;&nbsp;
	<input type="submit" value="Apply Changes" name="setMacDft">&nbsp;&nbsp;
	<input type="hidden" value="/admin/fw-macfilter.asp" name="submit-url">
</font></td></tr>
</form>
</table>
<table border=0 width="500" cellspacing=0 cellpadding=0>
<form action=/goform/admin/formFilter method=POST name="formFilterAdd">

<tr><hr size=1 noshade align=top></tr>
<br>
<tr>
	<td><font size=2>
	<b>Direction: </b>
	<select name=dir>
		<option select value=0>Outgoing</option>
		<option value=1>Incoming</option>
	</select>&nbsp;&nbsp;
	<b>	Rule Action</b>
	<input type="radio" name="filterMode" value="Deny" checked>&nbsp;&nbsp;Deny
	<input type="radio" name="filterMode" value="Allow">&nbsp;&nbsp;Allow</b><br>
	</font></td>
</tr>

<tr>
	<td><font size=2>        
	<b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Source MAC Address: </b> <input type="text" name="srcmac" size="15" maxlength="12">&nbsp;&nbsp;
	</font></td>
</tr>
<tr>
	<td><font size=2>        
        <b>Destination MAC Address: </b> <input type="text" name="dstmac" size="15" maxlength="12">&nbsp;&nbsp;
	</font></td>
	<td><input type="submit" value="Add" name="addFilterMac" onClick="return addClick()">&nbsp;&nbsp;
	<input type="hidden" value="/admin/fw-macfilter.asp" name="submit-url">
	</td>
</tr>
</form>
</table>

<form action=/goform/admin/formFilter method=POST name="formFilterDel">
  <table border="0" width=500>
  <tr><hr size=1 noshade align=top></tr>
  <tr><font size=2><b>Current Filter Table:</b></font></tr>
  <% macFilterList(); %>
  </table>
  <br>
  <input type="submit" value="Delete Selected" name="deleteSelFilterMac" onClick="return deleteClick()">&nbsp;&nbsp;
  <input type="submit" value="Delete All" name="deleteAllFilterMac" onClick="return deleteAllClick()">&nbsp;&nbsp;&nbsp;
  <input type="hidden" value="/admin/fw-macfilter.asp" name="submit-url">
 <script>
 	<% checkWrite("macFilterNum"); %>
   	<% entryNum = getIndex("macFilterNum");
   	  if ( entryNum == 0 ) {
      	  	write( "disableDelButton();" );
       	  } %>
 </script>
</form>

</blockquote>
</body>
</html>
