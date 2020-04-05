<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Connection Limit</title>
<script type="text/javascript" src="share.js">
</script>
<script>
function skip () { this.blur(); }

function addClick()
{

  if (document.formConnLimitAdd.connLimitcap[0].checked)
  	return false;
 
 if (document.formConnLimitAdd.ip.value=="") {
	alert("IP address cannot be empty! It should be filled with 4 digit numbers as xxx.xxx.xxx.xxx.");
	document.formConnLimitAdd.ip.focus();
	return false;
  }
   	
   	num1 = parseInt(document.formConnLimitAdd.tcpconnlimit.value,10);
   	num4 = parseInt(document.formConnLimitAdd.udpconnlimit.value,10);
   	num2 = parseInt(document.formConnLimitAdd.connnum.value,10);
   	num3 = parseInt(document.formConnLimitAdd.protocol.value,10);
   	if ((num1!=0)&&( num3 == 0) && ( num2 > num1)){
   		alert("Max Limitation Ports should be lower than Global TCP Connection Limit");
   		document.formConnLimitAdd.connnum.focus();
   		return false;
   	}
   	else if ((num4 != 0)&&( num3 == 1)&&( num2 > num4)){
   		alert("Max Limitation Ports should be lower than Global UDP Connection Limit");
   		document.formConnLimitAdd.connnum.focus();
   		return false;
   	}


  if ( !checkDigitRange(document.formConnLimitAdd.ip.value,1,0,255) ) {
      	alert('Invalid IP address range in 1st digit. It should be 0-255.');
	document.formConnLimitAdd.ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formConnLimitAdd.ip.value,2,0,255) ) {
      	alert('Invalid IP address range in 2nd digit. It should be 0-255.');
	document.formConnLimitAdd.ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formConnLimitAdd.ip.value,3,0,255) ) {
      	alert('Invalid IP address range in 3rd digit. It should be 0-255.');
	document.formConnLimitAdd.ip.focus();
	return false;
  }
  if ( !checkDigitRange(document.formConnLimitAdd.ip.value,4,1,254) ) {
      	alert('Invalid IP address range in 4th digit. It should be 1-254.');
	document.formConnLimitAdd.ip.focus();
	return false;
  }   		
  return true;
}

function disableDelButton()
{

  if (verifyBrowser() != "ns") {
	disableButton(document.formConnLimitDel.deleteSelconnLimit);
	disableButton(document.formConnLimitDel.deleteAllconnLimit);
  }

}

function updateState()
{
	
//  if (document.formConnLimitAdd.enabled.checked) {
  if (document.formConnLimitAdd.connLimitcap[1].checked) {
 	enableTextField(document.formConnLimitAdd.ip);
	enableTextField(document.formConnLimitAdd.protocol);
	enableTextField(document.formConnLimitAdd.connnum);	
	//enableTextField(document.formConnLimitAdd.cnlm_enable);
  }
  else {
 	disableTextField(document.formConnLimitAdd.ip);
	disableTextField(document.formConnLimitAdd.protocol);
	disableTextField(document.formConnLimitAdd.connnum);
	//disableTextField(document.formConnLimitAdd.cnlm_enable);
  }

}



</script>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">Connection Limit</font></h2>
<form action=/goform/formConnlimit method=POST name="formConnLimitAdd">
<table border=0 width="500" cellspacing=0 cellpadding=0>
<tr><td><font size=2>
 Entries in this table allow you to limit the number of TCP/UDP ports used by internal users.
</font></td></tr>

<tr><td><hr size=1 noshade align=top></td></tr>




<tr><td><font size=2><b>Connection Limit:</b>
	<input type="radio" value="0" name="connLimitcap" <% checkWrite("connLimit-cap0"); %> onClick="updateState()">Disable&nbsp;&nbsp;
	<input type="radio" value="1" name="connLimitcap" <% checkWrite("connLimit-cap1"); %> onClick="updateState()">Enable&nbsp;&nbsp;
	
</font></td></tr>
<tr><td><font size=2><b>Global TCP Connection Limit:</b>
	<input type="text" name="tcpconnlimit" size="4" maxlength="4" value=<% getInfo("connLimit-tcp"); %>> &nbsp;(0 for no limit) &nbsp; </td>
</font></td></tr>
<tr><td><font size=2><b>Global UDP Connection Limit:</b>
	<input type="text" name="udpconnlimit" size="4" maxlength="4" value=<% getInfo("connLimit-udp"); %> > &nbsp;(0 for no limit) &nbsp; </td>
</font></tr>
<tr><td><input type="submit" value="Apply Changes" name="apply">&nbsp;&nbsp;
	<input type="hidden" value="/connlimit.asp" name="submit-url"></td></tr>


</table>
<table border=0 width="500" cellspacing=0 cellpadding=0>
	<tr><hr size=1 noshade align=top></tr>
	<tr>
		<td><font size=2>
			<b>Protocol:</b>
				<select name="protocol">
					<option select value=0>TCP</option>
					<option value=1>UDP</option>
				</select>&nbsp;			
		<!--	<input type="checkbox" name="cnlm_enable" value="1" CHECKED><b>Enable</b>  -->
		</td>
	</tr>
	<tr>
		<td><font size=2><b>Local IP Address:&nbsp;</b>
				<input type="text" name="ip" size="10" maxlength="15">&nbsp;&nbsp;&nbsp;
			<font size=2><b>Max Limitation Ports:</b>
				<input type="text" name="connnum" size="3" maxlength="5">
		</td>
	</tr>
	<tr>
		<td>
			<input type="submit" value="Add" name="addconnlimit" onClick="return addClick()">
			<input type="hidden" value="/fw-portfw.asp" name="submit-url">
		</td>
	</tr>
<script> updateState(); </script>
</form>
</table>


<br>
<form action=/goform/formConnlimit method=POST name="formConnLimitDel">
<table border=0 width=500>
  <tr><hr size=1 noshade align=top></tr>
  <tr><font size=2><b>Current Connection Limit Table:</b></font></tr>
 <!-- 
  <tr><td align=center width="15%" bgcolor="#808080"><font size="2"><b>Select</b></font></td>
<td align=center width="30%" bgcolor="#808080"><font size="2"><b>Local IP Address</b></font></td>
<td align=center width="20%" bgcolor="#808080"><font size="2"><b>Protocol</b></font></td>
<td align=center width="25%" bgcolor="#808080"><font size="2"><b>Max ports</b></font></td>
<td align=center bgcolor="#808080"><font size="2"><b>Enable</b></font></td>
-->
<% connlmitList(); %>
</table>

 <br><input type="submit" value="Delete Selected" name="deleteSelconnLimit" onClick="return deleteClick()">&nbsp;&nbsp;
     <input type="submit" value="Delete All" name="deleteAllconnLimit" onClick="return deleteAllClick()">&nbsp;&nbsp;&nbsp;
     <input type="hidden" value="/connlimit.asp" name="submit-url">
 <script>
   	<% checkWrite("connLimitNum"); %>
 </script>
     
</form>

</td></tr></table>

</blockquote>
</body>
</html>
