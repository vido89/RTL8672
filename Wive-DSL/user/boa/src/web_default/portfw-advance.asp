<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Port Forwarding Advance Setting </title>
<script type="text/javascript" src="share.js">
</script>
<% language=javascript %>
<SCRIPT>

function PFWRemove() {
   with ( document.portfwAdvance ) {
      var arrSelected = new Array();
      var count = 0;
      for ( i = 0; i < lstApply.options.length; i++ ) {
         if ( lstApply.options[i].selected == true ) {
            arrSelected[count] = lstApply.options[i].value;
         }
         count++;
      }
      var x = 0;
      for (i = 0; i < lstApply.options.length; i++) {
         for (x = 0; x < arrSelected.length; x++) {
            if (lstApply.options[i].value == arrSelected[x]) {
               //varOpt = new Option(lstApply.options[i].text, lstApply.options[i].value);
               //lstAvail.options[lstAvail.length] = varOpt;
               
               // delete the option
               lstApply.options[i] = null;
            }
         }
      }
   }
}

function PFWAdd() {
   with ( document.portfwAdvance ) {
      var arrSelected = new Array();
      var count = 0;
      var y = 0;
      for ( i = 0; i < lstAvail.options.length; i++ ) {                
             if ( lstAvail.options[i].selected == true && lstApply.options.length == 0 ) {
                  arrSelected[count] = lstAvail.options[i].value;
                  count++;
            }                
      }
      
      var x = 0;
      for (i = 0; i < lstAvail.options.length; i++) {
         for (x = 0; x < arrSelected.length; x++) {
            if (lstAvail.options[i].value == arrSelected[x]) {
               varOpt = new Option(lstAvail.options[i].text, lstAvail.options[i].value);
               lstApply.options[lstApply.length] = varOpt;
               
               // delete the option
               //lstAvail.options[i] = null;
            }
         }
      }
   }
}

function PFWApply() {
   //document.portfwAdvance.ruleApply.value="";    
   
   if (document.portfwAdvance.ip.value=="") {
	alert("Local IP Address cannot be empty!");
	document.portfwAdvance.ip.focus();
	return false;
   }   
   
   if (!checkHostIP(document.portfwAdvance.ip, 1))
	return false;
	
   if (document.portfwAdvance.lstApply.options.length == 0) {
	alert("Rule cannot be empty! Please choose one gategory and select one available rule then add into applied rule.");	
	return false;
   }
   
   with ( document.portfwAdvance ) {
      for (i = 0; i < lstApply.options.length; i++)
         ruleApply.value+=lstApply.options[i].value + ',';
      //for (i = 0; i < lstAvail.options.length; i++)
      //   itfsAvail.value+=lstAvail.options[i].value + ',';
   }
   return true;
}

function postPFW(apply, applyval, avail, availval) {
   var interfaces;
   with ( document.portfwAdvance ) {
      interfaces = apply.split(',');
      itfvals = applyval.split(',');
      
      // clear a select box
      lstApply.options.length = 0;
      for ( i = 0; i < interfaces.length; i++ ) {
         if (interfaces[i] != '') {
            // create a new option
            lstApply.options[i] = new Option(interfaces[i], itfvals[i]);
         }
      }
      
      interfaces = avail.split(',');
      itfvals = availval.split(',');
      
       // clear a select box
      lstAvail.options.length = 0;
      for ( i = 0; i < interfaces.length; i++ ) {
         if (interfaces[i] != '') {
             // create a new option
            lstAvail.options[i] = new Option(interfaces[i], itfvals[i]);
         }
      }
   }
}

</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">Port FW Advance Configuration</font></h2>

<form action=/goform/formPFWAdvance method=POST name="portfwAdvance">
   <table border=0 width="500" cellspacing=4 cellpadding=0>
      <hr size=2 noshade align=top>
      <tr>
          <td><font size=2><b>Category: </b>
              <input type="radio" name=gategory value=0 onClick="postPFW('', '', 'PPTP,L2TP', '0,1')">VPN&nbsp;&nbsp;  
          </td>
      </tr>   
   </table>  
   
   <table border=0 width="500" cellspacing=4 cellpadding=0>
      <hr size=2 noshade align=top>
      <tr>
       <td><font size=2><b>Interface:</b> 
	   <select name="interface">
	   	<%  if_wan_list("rt-any");%>		
	   </select>    
	   <input type="hidden" value="" name="select_id">
       </td>
      </tr>   

      <br>
      <tr>
       <td><font size=2><b>Local IP Address:</b>
	   <input type="text" name="ip" size="15" maxlength="15">			
       </td>		
      </tr>
   </table>   
     
   <table border=0 width="500" cellspacing=4 cellpadding=0>
      <hr size=2 noshade align=top>
      <tr>      
         <td width="150"><font size=2><b>Available Rules</b></td>
         <td width="100"></td>
         <td width="150"><font size=2><b>Applied Rules</b></td>
      </tr>      
      
      <tr>
         <td>
             <select multiple name="lstAvail" size="8" style="width: 100"></select>
         </td>
         <td>
            <table border="0" cellpadding="0" cellspacing="5">
               <tr><td>
                  <input type="button" name="rmbtn" value="->" onClick="PFWAdd()" style="width: 30; height: 30">
               </td></tr>
               <tr><td>
                  <input type="button" name="adbtn" value="<-" onClick="PFWRemove()" style="width: 30; height: 30">
               </td></tr>
            </table>
         </td>
         <td>
             <select multiple name="lstApply" size="8" style="width: 100"></select>	
         </td>
    </table>
    <br> 
    
    <tr>
      &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
      &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
      &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;      
      <input type=submit value="ADD" name="save" onClick="return PFWApply()">&nbsp;&nbsp;
    </tr>      
      
   <table border=0 width="500" cellspacing=4 cellpadding=0>
    <tr><hr size=1 noshade align=top></tr>
    <tr><font size=2><b>Port FW Advance Table:</b></font></tr>
    <% showPFWAdvTable(); %>
   </table>
   
   <br>
   <input type="hidden" name=ruleApply>
   <!--<input type="hidden" name=itfsAvail>-->
   <input type="hidden" value="/portfw-advance.asp" name="submit-url">
   <input type="submit" value="Delete Selected" name="delRule">&nbsp;&nbsp;  
   <input type="submit" value="Delete All" name="delAllRule" onClick="return deleteAllClick()">&nbsp;&nbsp;&nbsp;
         
<script>
	document.portfwAdvance.gategory[0].checked = false;	
</script>
</form>
</blockquote>
</body>

</html>