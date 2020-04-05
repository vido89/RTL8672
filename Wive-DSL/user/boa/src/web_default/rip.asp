<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>RIP Configuration </title>
<script type="text/javascript" src="share.js">
</script>
<% language=javascript %>
<SCRIPT>
var ifnum;

function selected()
{
	document.rip.ripDel.disabled = false;
}

function resetClicked()
{
	document.rip.ripDel.disabled = true;
}

function disableDelButton()
{
  if (verifyBrowser() != "ns") {
	disableButton(document.rip.ripDel);
	disableButton(document.rip.ripDelAll);
  }
}
</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">RIP Configuration</font></h2>

<form action=/goform/formRip method=POST name="rip">
<table border=0 width="500" cellspacing=0 cellpadding=0>
  <tr><font size=2>
    Enable the RIP if you are using this device as a RIP-enabled router to
    communicate with others using the Routing Information Protocol.
    This page is used to select the interfaces on your device is that use
    RIP, and the version of the protocol used.
  </tr>
  <tr><hr size=1 noshade align=top></tr>
  <tr>
      <td><font size=2><b>RIP:</b></td>
      <td><font size=2>
      	<input type="radio" value="0" name="rip_on" <% checkWrite("rip-on-0"); %> >Disable&nbsp;&nbsp;
     	<input type="radio" value="1" name="rip_on" <% checkWrite("rip-on-1"); %> >Enable&nbsp;&nbsp;
      </td>
      <td><input type="submit" value="Apply Changes" name="ripSet">&nbsp;&nbsp;</td>
  </tr>
<!--  
  <tr>
      <td width="25%"><b>Version:</b></td>
      <td width="35%">
      	<select name="rip_ver">
      	<% checkWrite("rip-ver"); %>
      	</select>
      </td>
  </tr>
-->
</table>

  <!--
  <input type="reset" value="Reset" name="ripReset" onClick="resetClicked()">&nbsp;&nbsp;  
  -->
  
<table border=0 width="500" cellspacing=0 cellpadding=0> 
  <tr><hr size=1 noshade align=top></tr>
  <br>

  <tr>
      <td width="30%"><font size=2><b>Interface:</b></td>
      <td width="35%">
      	<select name="rip_if">
      	<option value="255">br0</option>
      	<%  if_wan_list("rt");  %>
      	</select>
      	<!--
        <input type="submit" value="Add" name="ripAdd">
        -->
      </td>
  </tr>
  
  <tr>
      <td width="30%"><font size=2><b>Receive Mode:</b></td>
      <td width="70%">
      <select size="1" name="receive_mode">
      <option value="0">None</option>
      <option value="1">RIP1</option>
      <option value="2">RIP2</option>
      <option value="3">Both</option>
      </select>
      </td>
  </tr>
  
  <tr>
      <td width="30%"><font size=2><b>Send Mode:</b></td>
      <td width="50%">
      <select size="1" name="send_mode">
      <option value="0">None</option>
      <option value="1">RIP1</option>
      <option value="2">RIP2</option>
      <option value="4">RIP1COMPAT</option>
      </select>
      </td>
      <td width="20%"><input type="submit" value="Add" name="ripAdd"></td>
  </tr>  
</table>   

<!--
<table border=0 width="500" cellspacing=0 cellpadding=0>
  <tr><hr size=1 noshade align=top></tr>

  <tr>
      <td width="30%"><b>Interface:</b></td>
      <td width="35%">
      	<select name="rip_if">
      	<option value="255">br0</option>
      	<%  if_wan_list("rt");  %>
      	</select>
        <input type="submit" value="Add" name="ripAdd">
      </td>
  </tr>
  
  <tr>
  	<td>
        <input type="submit" value="Delete Selected" name="ripDel">
        </td>
  </tr>
  <tr></tr><tr></tr>
  <tr><font size=2><td><b>RIP Interfaces:</b></td></font></tr>
  <% showRipIf(); %>
</table>
-->

<!--
<script>
	<% initPage("rip"); %>
</script>
-->

  
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><hr size=1 noshade align=top></tr>
  <tr><font size=2><b>RIP Config Table:</b></font></tr>
  <% showRipIf(); %>
</table>

<br>
   <input type="submit" value="Delete Selected" name="ripDel" onClick="return deleteClick()">&nbsp;&nbsp;  
   <input type="submit" value="Delete All" name="ripDelAll" onClick="return deleteAllClick()">&nbsp;&nbsp;&nbsp;
   <input type="hidden" value="/rip.asp" name="submit-url">
 <script>
 	<% checkWrite("ripNum"); %>
  </script>
</form>
</blockquote>
</body>

</html>
