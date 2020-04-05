<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Address Mapping Rule Configuration</title>
<script type="text/javascript" src="share.js">
</script>
<SCRIPT>


function saveClick()
{
	var ls, le, gs, ge;
	var ls1, le1, gs1, ge1;
	var ls2, le2, gs2, ge2;
	var ls3, le3, gs3, ge3;
	
	ls = getDigit(document.addressMap.lsip.value,4);
  	le = getDigit(document.addressMap.leip.value,4);
  	gs = getDigit(document.addressMap.gsip.value,4);
  	ge = getDigit(document.addressMap.geip.value,4);
  	
  	ls1 = getDigit(document.addressMap.lsip.value,1);
  	le1 = getDigit(document.addressMap.leip.value,1);
  	gs1 = getDigit(document.addressMap.gsip.value,1);
  	ge1 = getDigit(document.addressMap.geip.value,1);
  	
  	ls2 = getDigit(document.addressMap.lsip.value,2);
  	le2 = getDigit(document.addressMap.leip.value,2);
  	gs2 = getDigit(document.addressMap.gsip.value,2);
  	ge2 = getDigit(document.addressMap.geip.value,2);
  	
  	ls3 = getDigit(document.addressMap.lsip.value,3);
  	le3 = getDigit(document.addressMap.leip.value,3);
  	gs3 = getDigit(document.addressMap.gsip.value,3);
  	ge3 = getDigit(document.addressMap.geip.value,3);
  		
	if ( document.addressMap.addressMapType.selectedIndex == 0 ) {
		//alert('You select none.');			
  	} else if ( document.addressMap.addressMapType.selectedIndex == 1 ) {
		//alert('You select One-to-One.');  		
  		if ( !checkIP(document.addressMap.lsip)) {  			
  			return false;
  		}  		
  		
  		if ( !checkIP(document.addressMap.gsip)) {  		
  			return false;
  		}  		
  		  		
  	} else if ( document.addressMap.addressMapType.selectedIndex == 2 ) {
		//alert('You select Many-to-One.');  		
  		if ( !checkIP(document.addressMap.lsip)) {  		
  			return false;
  		}  		
  				
  		if ( !checkIP(document.addressMap.leip)) {  		
  			return false;
  		}  		
  		  		
  		if ( !checkIP(document.addressMap.gsip)) {  		
  			return false;
  		}  		
  		
  		if ( ls1 != le1 || ls2 != le2 || ls3 != le3 ) {
  			alert('Local Start IP domain is different from Local End IP');
  			document.addressMap.lsip.focus();
  			return false;
  		}  		 		
  		
  		if ( le <= ls ) {
  			alert('Invalid Local IP range.');
  			document.addressMap.lsip.focus();
  			return false;
  		} 		
  		
  		 		
  	} else if ( document.addressMap.addressMapType.selectedIndex == 3 ) {
		//alert('You select Many-to-Many.');		
		if ( !checkIP(document.addressMap.lsip)) {  		
  			return false;
  		}  		
  		
  		if ( !checkIP(document.addressMap.leip)) {  		
  			return false;
  		}  		
  		
  		if ( !checkIP(document.addressMap.gsip)) {  		
  			return false;
  		}  		
  		
  		if ( !checkIP(document.addressMap.geip)) {  		
  			return false;
  		}  		
  		
  		if ( ls1 != le1 || ls2 != le2 || ls3 != le3 ) {
  			alert('Local Start IP domain is different from Local End IP');
  			document.addressMap.lsip.focus();
  			return false;
  		}
  		
  		if ( gs1 != ge1 || gs2 != ge2 || gs3 != ge3 ) {
  			alert('Global Start IP domain is different from Global End IP');
  			document.addressMap.gsip.focus();
  			return false;
  		}  		
  		
  		if ( le <= ls ) {
  			alert('Invalid Local IP range.');
  			document.addressMap.lsip.focus();
  			return false;
  		}
  		
  		if ( ge <= gs ) {
  			alert('Invalid Global IP range.');
  			document.addressMap.gsip.focus();
  			return false;
  		}
  						 		
  	} else if ( document.addressMap.addressMapType.selectedIndex == 4 ) {
		//alert('You select Many One-to-Many.');		
		if ( !checkIP(document.addressMap.lsip)) {  		
  			return false;
  		}  		
  		
  		if ( !checkIP(document.addressMap.gsip)) {  		
  			return false;
  		}    		
  		
  		if ( !checkIP(document.addressMap.geip)) {  		
  			return false;
  		}  		
  		
  		if ( gs1 != ge1 || gs2 != ge2 || gs3 != ge3 ) {
  			alert('Global Start IP domain is different from Global End IP');
  			document.addressMap.gsip.focus();
  			return false;
  		}  		
  		
  		if ( ge <= gs ) {
  			alert('Invalid Global IP range.');
  			document.addressMap.gsip.focus();
  			return false;
  		}   		 		
  								 		
  	}  
  	//alert("Please commit and reboot this system for take effect the address mapping rule!");
}

function checkState()
{              		
	if ( document.addressMap.addressMapType.selectedIndex == 0 ) {
		//alert('You select none.');		
		disableTextField(document.addressMap.lsip);
  		disableTextField(document.addressMap.leip);
  		disableTextField(document.addressMap.gsip);
  		disableTextField(document.addressMap.geip);
  		//document.addressMap.lsip.value = "N/A";		
  	} else if ( document.addressMap.addressMapType.selectedIndex == 1 ) {
		//alert('You select One-to-One.');				
  		enableTextField(document.addressMap.lsip);
  		disableTextField(document.addressMap.leip);
  		enableTextField(document.addressMap.gsip);
  		disableTextField(document.addressMap.geip);  		
  	} else if ( document.addressMap.addressMapType.selectedIndex == 2 ) {
		//alert('You select Many-to-One.');				
  		enableTextField(document.addressMap.lsip);
  		enableTextField(document.addressMap.leip);
  		enableTextField(document.addressMap.gsip);
  		disableTextField(document.addressMap.geip);   		
  	} else if ( document.addressMap.addressMapType.selectedIndex == 3 ) {
		//alert('You select Many-to-Many.');
		enableTextField(document.addressMap.lsip);
  		enableTextField(document.addressMap.leip);
  		enableTextField(document.addressMap.gsip);
  		enableTextField(document.addressMap.geip); 				 		
  	} else if ( document.addressMap.addressMapType.selectedIndex == 4 ) {
		//alert('You select Many One-to-Many.');
		enableTextField(document.addressMap.lsip);
  		disableTextField(document.addressMap.leip);
  		enableTextField(document.addressMap.gsip);
  		enableTextField(document.addressMap.geip); 				 		
  	}
  
}


</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">Address Mapping Rule Configuration</font></h2>

<form action=/goform/formAddressMap method=POST name="addressMap">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><font size=2>
    Use this page to set and configure the Address Mapping Rule 
    for your device.
  </tr>
  <tr><hr size=1 noshade align=top></tr>

  <tr>
      <td width="30%"><font size=2><b>Type:</b>
      <select size="1" name="addressMapType" onChange="checkState()">
      <% checkWrite("addressMapType"); %>
      </select>
      </td>
  </tr>
  
  <tr>
      <td width="30%"><font size=2><b>Local Start IP:</b></td>
      <td width="70%"><input type="text" name="lsip" size="15" maxlength="15" value=<% getInfo("local-s-ip"); %>></td>
  </tr>
  
  <tr>
      <td width="30%"><font size=2><b>Local End IP:</b></td>
      <td width="70%"><input type="text" name="leip" size="15" maxlength="15" value=<% getInfo("local-e-ip"); %>></td>
  </tr>
  
  <tr>
      <td width="30%"><font size=2><b>Global Start IP:</b></td>
      <td width="70%"><input type="text" name="gsip" size="15" maxlength="15" value=<% getInfo("global-s-ip"); %>></td>
  </tr>
  
  <tr>
      <td width="30%"><font size=2><b>Global End IP:</b></td>
      <td width="70%"><input type="text" name="geip" size="15" maxlength="15" value=<% getInfo("global-e-ip"); %>></td>
  </tr>
  
</table>
  <br>
      <input type="submit" value="Commit Changes" name="save" onClick="return saveClick()">&nbsp;&nbsp;
      <input type="hidden" value="/address_mapping.asp" name="submit-url">
         
  <script>
	<% initPage("addressMap"); %>
  </script>  
  
 </form>
</blockquote>
</body>

</html>
