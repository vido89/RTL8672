<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>OAM Loopback Test </title>
<script type="text/javascript" src="share.js">
</script>
<% language=javascript %>
<SCRIPT>
function isHexDecimal(num)
{
	var string="1234567890ABCDEF";
	if (string.indexOf(num.toUpperCase()) != -1)
	{
		return true;
	}
	
	return false;
}

function isValidID(val)
{
	for(var i=0; i < val.length; i++)
	{       
		if  ((!isHexDecimal(val.charAt(i))))
		{
			return false;
		}
	}
	
	return true;
}

function goClick()
{
	retval = isValidID(document.oamlb.oam_llid.value);
	if((document.oamlb.oam_llid.value=="")|| (retval==false))
	{ 
		alert("Invalid Loopback Location ID!")
		document.oamlb.oam_llid.focus()
		return false
	}
       
	return true;
}
</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">OAM Fault Management - Connectivity Verification</font></h2>

<form action=/goform/formOAMLB method=POST name="oamlb">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><font size=2>
    Connectivity verification is supported by the use of the OAM loopback capability
    for both VP and VC connections. This page is used to perform the VCC loopback
    function to check the connectivity of the VCC.
  </tr>
  <tr><hr size=1 noshade align=top></tr>
</table>

<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><td>
      <font size=2><b>Select PVC:</b>
        <% oamSelectList(); %>
      </td>
  </tr>
  
  <tr><td>
      <font size=2><b>Flow Type:</b>
      	<input type="radio" value="0" name="oam_flow" checked>F5 Segment&nbsp;&nbsp;&nbsp;&nbsp;
     	<input type="radio" value="1" name="oam_flow" >F5 End-to-End
      </td>
  </tr>

  <tr><td>
      <font size=2><b>Loopback Location ID: </b>
      <input type="text" name="oam_llid" value="FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF" size=40 maxlength=32 onFocus="this.select()">
      </td>
  </tr>
</table>
  <br>
      <input type="submit" value=" Go ! " name="go" onClick="return goClick()">
      <input type="hidden" value="/oamloopback.asp" name="submit-url">
</form>
</blockquote>
</body>

</html>
