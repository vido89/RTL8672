<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Parental-Control</title>
<script type="text/javascript" src="share.js">
</script>
<script>
function skip () { this.blur(); }
function addClick()
{
  if (document.formParentCtrlAdd.usrname.value=="" )
  {
  	alert("Input Username!");
	document.formParentCtrlAdd.usrname.focus();
	return false;
  }
    if (document.formParentCtrlAdd.mac.value=="" )
  {
  	alert("Input MAC address!");
	document.formParentCtrlAdd.mac.focus();
	return false;
  }
  var str = document.formParentCtrlAdd.mac.value;
  if ( str.length < 12) {
	alert("Input MAC address is not complete. It should be 12 digits in hex.");
	document.formParentCtrlAdd.mac.focus();
	return false;
  }

  for (var i=0; i<str.length; i++) {
    if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
			(str.charAt(i) >= 'a' && str.charAt(i) <= 'f') ||
			(str.charAt(i) >= 'A' && str.charAt(i) <= 'F') )
			continue;

	alert("Invalid MAC address. It should be in hex number (0-9 or a-f).");
	document.formParentCtrlAdd.mac.focus();
	return false;
  }
  
  if ((document.formParentCtrlAdd.starthr.value=="" )
  	|| (document.formParentCtrlAdd.startmin.value=="" )
  	||(document.formParentCtrlAdd.endhr.value=="" )
  	||(document.formParentCtrlAdd.endmin.value=="" )){
  	alert('Invalid value of Time value. Please input corrent time format, 00:00 ~ 23:59');
  	document.formParentCtrlAdd.starthr.focus();
	return false;
  }
	num1 = parseInt(document.formParentCtrlAdd.starthr.value,10);
	num2 = parseInt(document.formParentCtrlAdd.startmin.value,10);
	num3 = parseInt(document.formParentCtrlAdd.endhr.value,10);
	num4 = parseInt(document.formParentCtrlAdd.endmin.value,10);
  if ((num1 > 23)||(num3 > 23) )   {
  	alert('Invalid value of Time value. Please input corrent time format,00:00 ~ 23:59');
  	document.formParentCtrlAdd.starthr.focus();
	return false;
  }

  
  if ( num1 > num3 )   {
  	alert('End Time should be larger');
  	document.formParentCtrlAdd.starthr.focus();
	return false;
  }
   if (( num1 == num3 ) && (num2 >= num4))   {
  	alert('End Time should be larger');
  	document.formParentCtrlAdd.starthr.focus();
	return false;
  } 
  
  if ((num2 > 59)||(num4 > 59) )   {
  	alert('Invalid value of Time value. Please input corrent time format, ex: 12:50');
  	document.formParentCtrlAdd.starthr.focus();
	return false;
  }
  
  return true;
}

function disableDelButton()
{
  if (verifyBrowser() != "ns") {
	disableButton(document.formParentCtrlDel.deleteSelFilterMac);
	disableButton(document.formParentCtrlDel.deleteAllFilterMac);
  }
}

</script>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">Parental Control</font></h2>

<table border=0 width="600" cellspacing=4 cellpadding=0>
<tr><td><font size=2>
 Entries in this table are used to restrict access to Internet  from your local PCs/devices 
 by mac address </br>and time interval.  Use of such filters can be helpful for parents to control
 children's usage of Internet.
</font></td></tr>

<tr><td><hr size=1 noshade align=top></td></tr>


<form action=/goform/admin/formParentCtrl method=POST name="formParentCtrlAdd">
<tr><td>
     <p><font size=2>        
        <b>User Name:&nbsp;&nbsp; </b> <input type="text" name="usrname" size="32" maxlength="31">&nbsp;&nbsp;
     </p>
     
     <p><font size=2>        
        <b>MAC Address:&nbsp;&nbsp; </b> <input type="text" name="mac" size="15" maxlength="12">&nbsp;&nbsp;
     </p>
     
    
  <table border='0' cellpadding="0" cellspacing="0" >
               <tr>
                  <td width='120'></td>
                  <td align='center' width='60' ><font size="2">Monday</font></td>
                  <td align='center' width='60' ><font size="2">Tuesday</font></td>
                  <td align='center' width='60' ><font size="2">Wednesday</font></td>
                  <td align='center' width='60' ><font size="2">Thursday</font></td>
                  <td align='center' width='60' ><font size="2">Friday</font></td>
                  <td align='center' width='60' ><font size="2">Saturday</font></td>
                  <td align='center' width='60' ><font size="2">Sunday</font></td>
               </tr>
               <tr>
                  <td><font size=2><b>Controlled Days </b></font></td>
                  <td align='center'><input type='checkbox' name='mond'></td>
                  <td align='center'><input type='checkbox' name='tued'></td>
                  <td align='center'><input type='checkbox' name='wend'></td>
                  <td align='center'><input type='checkbox' name='thrd'></td>
                  <td align='center'><input type='checkbox' name='frid'></td>
                  <td align='center'><input type='checkbox' name='satd'></td>
                  <td align='center'><input type='checkbox' name='sund'></td>
               </tr>
    </table>
 <p><font size=2>        
        <b>Start Blocking time:&nbsp;&nbsp; </b> <input type="text" name="starthr" size="2" maxlength="2"><b>:</b>
         <input type="text" name="startmin" size="2" maxlength="2">
 </p>    
 <p><font size=2>        
        <b>End Blocking time:&nbsp;&nbsp; </b> <input type="text" name="endhr" size="2" maxlength="2"><b>:</b>
         <input type="text" name="endmin" size="2" maxlength="2">
 </p>
    
 <p><input type="submit" value="Apply Changes" name="addfilterMac" onClick="return addClick()">&nbsp;&nbsp;
        <input type="reset" value="Reset" name="reset">
        <input type="hidden" value="/parental-ctrl.asp" name="submit-url">
     </p>    
  </td></tr>
</form>
</table>

<br>
<form action=/goform/admin/formParentCtrl method=POST name="formParentCtrlDel">
  <table border="0" width=550>
  <tr><font size=2><b>Current Parent Control Table:</b></font></tr>
  <% parentControlList(); %>
  </table>
  <br>
  <input type="submit" value="Delete Selected" name="deleteSelFilterMac" onClick="return deleteClick()">&nbsp;&nbsp;
  <input type="submit" value="Delete All" name="deleteAllFilterMac" onClick="return deleteAllClick()">&nbsp;&nbsp;&nbsp;
  <input type="hidden" value="/parental-ctrl.asp" name="submit-url">
 <script>
 	<% checkWrite("parentCtrlNum"); %>
   	<% entryNum = getIndex("parentCtrlNum");
   	  if ( entryNum == 0 ) {
      	  	write( "disableDelButton();" );
       	  } %>
 </script>
</form>

</blockquote>
</body>
</html>
