
<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>MBSSID Setup </title>
<script type="text/javascript" src="share.js">
</script>
<% language=javascript %>

</head>

<body>
<blockquote>
   <h2><font color="#0000FF">Wireless Multiple BSSID Setup</font></h2>   
   
<form method=POST action="/goform/formWlanMBSSID" name=WlanMBSSID>
   <BR>
   <table cellSpacing=1 cellPadding=2 border=1>
      <!--
      <tr><td bgColor=bbccff>Wireless Card </td></tr>
      -->
   </table>
   
   <table cellSpacing=1 cellPadding=2 border=0>
    <tr>
     <tr>
         <td bgColor=#aaddff>Vap0</td>
         <td bgColor=#aaddff><input type=checkbox name=En_vap0 value=1 <% checkSSID("vap0"); %> >Enable</td>         
     </tr>
     
     <tr>
         <td bgColor=#ddeeff>SSID</td>
         <td bgColor=#ddeeff><input type=text name=ssid_v0 size=16 maxlength=16 value=<% SSIDStr("vap0"); %>></td>
     </tr>
     
     <tr>
         <td bgColor=#ddeeff>Authentication Type:</td>
         <td bgColor=#ddeeff>
   	  <input type="radio" name="authType0" value="open" >Open System&nbsp;&nbsp;
   	  <input type="radio" name="authType0" value="shared" >Shared Key&nbsp;&nbsp;
   	  <input type="radio" name="authType0" value="both" >Auto</td>
     </tr>     
    </table>
    
    <table cellSpacing=1 cellPadding=2 border=0>
     <tr>
         <td bgColor=#aaddff>Vap1</td>
         <td bgColor=#aaddff><input type=checkbox name=En_vap1 value=1 <% checkSSID("vap1"); %> >Enable</td>         
     </tr>
     
     <tr>
         <td bgColor=#ddeeff>SSID</td>
         <td bgColor=#ddeeff><input type=text name=ssid_v1 size=16 maxlength=16 value=<% SSIDStr("vap1"); %>></td>
     </tr>
     
     <tr>
         <td bgColor=#ddeeff>Authentication Type:</td>
         <td bgColor=#ddeeff>
   	  <input type="radio" name="authType1" value="open" >Open System&nbsp;&nbsp;
   	  <input type="radio" name="authType1" value="shared" >Shared Key&nbsp;&nbsp;
   	  <input type="radio" name="authType1" value="both" >Auto</td>
     </tr>     
    </table>
    
    <table cellSpacing=1 cellPadding=2 border=0>
     <tr>
          <td bgColor=#aaddff>Vap2</td>
          <td bgColor=#aaddff><input type=checkbox name=En_vap2 value=1 <% checkSSID("vap2"); %> >Enable</td>          
     </tr>
     
     <tr>
          <td bgColor=#ddeeff>SSID</td>
          <td bgColor=#ddeeff><input type=text name=ssid_v2 size=16 maxlength=16 value=<% SSIDStr("vap2"); %>></td>
     </tr>
     
     <tr>
         <td bgColor=#ddeeff>Authentication Type:</td>
         <td bgColor=#ddeeff>
   	  <input type="radio" name="authType2" value="open" >Open System&nbsp;&nbsp;
   	  <input type="radio" name="authType2" value="shared" >Shared Key&nbsp;&nbsp;
   	  <input type="radio" name="authType2" value="both" >Auto</td>
     </tr> 
    </table>
    
    <table cellSpacing=1 cellPadding=2 border=0>
     <tr>
          <td bgColor=#aaddff>Vap3</td>
          <td bgColor=#aaddff><input type=checkbox name=En_vap3 value=1 <% checkSSID("vap3"); %> >Enable</td>          
     </tr>
     
     <tr>
          <td bgColor=#ddeeff>SSID</td>
          <td bgColor=#ddeeff><input type=text name=ssid_v3 size=16 maxlength=16 value=<% SSIDStr("vap3"); %>></td>
     </tr>
     
     <tr>
         <td bgColor=#ddeeff>Authentication Type:</td>
         <td bgColor=#ddeeff>
   	  <input type="radio" name="authType3" value="open" >Open System&nbsp;&nbsp;
   	  <input type="radio" name="authType3" value="shared" >Shared Key&nbsp;&nbsp;
   	  <input type="radio" name="authType3" value="both" >Auto</td>
     </tr>
    </table>
    
    <table cellSpacing=1 cellPadding=2 border=0>
     <tr>
          <td colspan=2 align=center>
               <input type="hidden" value="/admin/wlmbssid.asp" name="submit-url">
              <input type=submit value=Apply>
              <input type=reset value=Reset>
          </td>
     </tr>
     
   </table>
   
   <script>
	<% initPage("wlmbssid"); %>	
  </script>
  
</form>

</blockquote>
</body>

</html>