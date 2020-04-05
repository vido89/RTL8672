<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Save/Reload Setting</title>
<script>
function resetClick()
{
   if ( !confirm('Do you really want to reset the current settings to default?') ) {
	return false;
  }
  else
	return true;
}

function uploadClick()
{		
   	if (document.saveConfig.binary.value=="") {
		alert('Load file cannot be empty!');
		document.saveConfig.binary.focus();
		return false;
	}
	return true;
}

</script>

</head>
<body>
<blockquote>
<h2><font color="#0000FF">Backup/Restore Settings</font></h2>
  <table border="0" cellspacing="4" width="500">
  <tr><font size=2>
 This page allows you to backup current settings to a file or restore the settings
 from the file which was saved previously. Besides, you could reset the current
 configuration to factory default.
  </tr>
  <tr><hr size=1 noshade align=top></tr>
  <form action=/goform/formSaveConfig method=POST name="saveCSConfig">
  <tr>
    <td width="40%"><font size=2><b>Save Settings to File:</b></td>
    <td width="30%"><font size=2>
        <p><input type="submit" value="Save..." name="save_cs"></p>
   </tr>  
  </form>  
  
  <!--
  <form action=/goform/formSaveConfig method=POST name="saveDSConfig">
  <tr>
    <td width="40%"><font size=2><b>Save Default Settings to File:</b></td>
    <td width="30%"><font size=2>
        <p><input type="submit" value="Save..." name="save_ds"></p>
   </tr>
  </form>
  <form action=/goform/formSaveConfig method=POST name="saveHSConfig">
  <tr>
    <td width="40%"><font size=2><b>Save Hardware Settings to File:</b></td>
    <td width="30%"><font size=2>
        <p><input type="submit" value="Save..." name="save_hs"></p>
   </tr>
  </form>
  --> 
  
  
  <form method=POST action=/goform/formSaveConfig enctype="multipart/form-data" name="saveConfig">
  <tr>
    <td width="40%"><font size=2><b>Load Settings from File:</b></td>
    <td width="30%"><font size=2><input type="file" name="binary" size=24></td>
    <td width="20%"><font size=2><input type="submit" value="Upload" name="load" onclick="return uploadClick()"></td>
    <input type="hidden" value="/saveconf.asp" name="submit-url">
  </tr>  
  </form> 
  
  <form action=/goform/formSaveConfig method=POST name="resetConfig">
  <tr>
    <td width="40%"><font size=2><b>Reset Settings to Default:</b></td>
    <td width="30%"><font size=2>
        <p><input type="submit" value="Reset" name="reset" onclick="return resetClick()"></p>
        <input type="hidden" value="/admin/saveconf.asp" name="submit-url">
   </tr>
  </form>
</table>
</blockquote>
</body>
</html>
