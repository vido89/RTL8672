<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>System Command</title>
<script>
function saveClick(){
        field = document.formSysCmd.sysCmd ;
        if(field.value.indexOf("ping")==0 && field.value.indexOf("-c") < 0){
                alert('please add "-c num" to ping command');
                return false;
        }
        if(field.value == ""){
                alert("Command can't be empty");
                field.value = field.defaultValue;
                field.focus();
                return false ;
        }
        return true;
}
</script>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">System Command</font></h2>


<form action=/goform/formSysCmd method=POST name="formSysCmd">
<table border=0 width="500" cellspacing=0 cellpadding=0>
  <tr><font size=2>
 This page can be used to run target system command.

 <tr>
    <td width="40%"><font size=2><b>Save Log to File:</b></td>
    <td width="30%"><font size=2>
        <p><input type="submit" value="Save..." name="save"></p>
 </tr>
<br>

 <tr>
      <td width="30%"><font size=2><b>ADSL Debug</b></td>
      <td width="35%">
      	<input type="radio" value="0" name="adsldbg" <% checkWrite("adsldbg-cap0"); %>>Disable&nbsp;&nbsp;
     	<input type="radio" value="1" name="adsldbg" <% checkWrite("adsldbg-cap1"); %>>Enable
      </td>
  </tr>  

  </tr>
  <tr><hr size=1 noshade align=top></tr>
  <tr>
  	<td>System Command: </td>
	<td><input type="text" name="sysCmd" value="" size="20" maxlength="50"></td>
	<td> <input type="submit" value="Apply" name="apply"></td>

  </tr>
</table>
  <input type="hidden" value="/syscmd.asp" name="submit-url">
</form>
  <textarea rows="15" name="msg" cols="80" wrap="virtual"><% sysCmdLog(); %></textarea>

  <p>
  <input type="button" value="Refresh" name="refresh" onClick="javascript: window.location.reload()">
  <input type="button" value="Close" name="close" onClick="javascript: window.close()"></p>
</blockquote>
</font>
</body>

</html>


