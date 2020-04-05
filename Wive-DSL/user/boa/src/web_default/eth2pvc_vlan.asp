<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Ethernet to ATM PVC Mapping </title>
<% language=javascript %>
<SCRIPT>
function adminClick()
{
	with (document.eth2pvc) {
		if (pmap[0].checked) {
			rmbtn.disabled = true;
			adbtn.disabled = true;
			lstGrp.disabled = true;
			lstAvail.disabled = true;
			vid.disabled = true;
			dhcps.disabled = true;
			for (i=0; i<4; i++)
				select[i].disabled = true;
		}
		else {
			rmbtn.disabled = false;
			adbtn.disabled = false;
			lstGrp.disabled = false;
			lstAvail.disabled = false;
			vid.disabled = false;
			dhcps.disabled = false;
			for (i=0; i<4; i++)
				select[i].disabled = false;
		}
	}
}

function btnRemove() {
   with ( document.eth2pvc ) {
      var arrSelected = new Array();
      var count = 0;
      for ( i = 0; i < lstGrp.options.length; i++ ) {
         if ( lstGrp.options[i].selected == true ) {
            arrSelected[count] = lstGrp.options[i].value;
         }
         count++;
      }
      var x = 0;
      var y;
      var found;
      for (i = 0; i < lstGrp.options.length; i++) {
         for (x = 0; x < arrSelected.length; x++) {
            if (lstGrp.options[i].value == arrSelected[x]) {
               // kaotest
               found = 0;
               for (y = 0; y < lstAvail.options.length; y++) {
                 if (lstAvail.options[y].value == arrSelected[x]) {
                   found = 1;
                   break;
                 }
               }
               if (!found) {
                 varOpt = new Option(lstGrp.options[i].text, lstGrp.options[i].value);
                 lstAvail.options[lstAvail.length] = varOpt;
               }
               lstGrp.options[i] = null;
            }
         }
      }
   }
}

function btnAdd() {
   with ( document.eth2pvc ) {
      var arrSelected = new Array();
      var count = 0;
      for ( i = 0; i < lstAvail.options.length; i++ ) {
         if ( lstAvail.options[i].selected == true ) {
            arrSelected[count] = lstAvail.options[i].value;
         }
         count++;
      }
      var x = 0;
      for (i = 0; i < lstAvail.options.length; i++) {
         for (x = 0; x < arrSelected.length; x++) {
            if (lstAvail.options[i].value == arrSelected[x]) {
               varOpt = new Option(lstAvail.options[i].text, lstAvail.options[i].value);
               lstGrp.options[lstGrp.length] = varOpt;
               // kaotest
               if ((lstAvail.options[i].value & 0xff00) != 0x0200)
               lstAvail.options[i] = null;
            }
         }
      }
   }
}

function btnApply() {
   with ( document.eth2pvc ) {
      for (i = 0; i < lstGrp.options.length; i++)
         itfsGroup.value+=lstGrp.options[i].value + ',';
      for (i = 0; i < lstAvail.options.length; i++)
         itfsAvail.value+=lstAvail.options[i].value + ',';
   }
}

function postit(groupitf, groupval, availitf, availval, gvid, gdhcps) {
   var interfaces;
   with ( document.eth2pvc ) {
      interfaces = groupitf.split(',');
      itfvals = groupval.split(',');
      lstGrp.options.length = 0;
      for ( i = 0; i < interfaces.length; i++ ) {
         if (interfaces[i] != '') {
            lstGrp.options[i] = new Option(interfaces[i], itfvals[i]);
         }
      }
      
      interfaces = availitf.split(',');
      itfvals = availval.split(',');
      lstAvail.options.length = 0;
      for ( i = 0; i < interfaces.length; i++ ) {
         if (interfaces[i] != '') {
            lstAvail.options[i] = new Option(interfaces[i], itfvals[i]);
         }
      }
      vid.value = gvid;
      if (gdhcps==0)
      	dhcps.checked = false;
      else
      	dhcps.checked = true;
   }
}

</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">Port Mapping Configuration</font></h2>

<form action=/goform/formVlanGroup method=POST name="eth2pvc">
<table border=0 width="700" cellspacing=4 cellpadding=0>
  <tr><font size=2>
    To manipulate a mapping group:<br>
    <b>1.</b> Select a group from the table.<br>
    <b>2.</b> Select interfaces from the available/grouped interface list and
    	add it to the grouped/available interface list using the arrow buttons to
    	manipulate the required mapping of the ports.<br>
    <b>3.</b> Click "Apply Changes" button to save the changes.<br><br>
    <b>Note that the selected interfaces will be removed from their existing groups and added to the new group.</b>
  </tr>
  <tr><hr size=1 noshade align=top></tr>
</table>
<table>
      <input type="radio" name=pmap value=0 onClick="return adminClick()">Disabled&nbsp;&nbsp;
      <input type="radio" name=pmap value=1 onClick="return adminClick()">Enabled</td>
</table><br>
<table border="0" cellpadding="0" cellspacing="0">
   <tr>
      <td width="150"><font size=2><b>Grouped Interfaces</b></td>
      <td width="100"></td>
      <td width="150"><font size=2><b>Available Interfaces</b></td>
   </tr>
   <!--
   <tr><td colspan="3">&nbsp;</td></tr>
   -->
   <tr>
      <td>
          <select multiple name="lstGrp" size="8" style="width: 100"></select>
      </td>
      <td>
         <table border="0" cellpadding="0" cellspacing="5">
            <tr><td>
               <input type="button" name="rmbtn" value="->" onClick="btnRemove()" style="width: 30; height: 30">
            </td></tr>
            <tr><td>
               <input type="button" name="adbtn" value="<-" onClick="btnAdd()" style="width: 30; height: 30">
            </td></tr>
         </table>
      </td>
      <td>
          <select multiple name="lstAvail" size="8" style="width: 100">
          </select>	
      </td>
   </tr>
</table>
	<tr>
		<td><b>VID</b></td>
		<td><input type=text name=vid size=4 maxlength=4></td>
		&nbsp;&nbsp;
		<td><input type=checkbox name=dhcps value=1><b>DHCP</b></td>
	</tr>
<br><br>
<!--table border='1' cellpadding='4' cellspacing='0'-->
<table border='0'>
	<% itfGrpList(); %>
</table>
  <br>
      <input type="hidden" name=itfsGroup>
      <input type="hidden" name=itfsAvail>
      <input type=submit value="Apply Changes" name="save" onClick=btnApply()>&nbsp;&nbsp;
      <input type="hidden" value="/eth2pvc_vlan.asp" name="submit-url">
<script>
	<% initPage("vlanGroup"); %>
	adminClick();
</script>
</form>
</blockquote>
</body>

</html>
