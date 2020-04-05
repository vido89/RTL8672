<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Other advanced configuration </title>
<script type="text/javascript" src="share.js">
</script>
<% language=javascript %>
<SCRIPT>
function saveChanges()
{
	if ( checkDigit(document.others.ltime.value) == 0) {
		alert("Invalid lease time!");	
		document.others.ltime.focus();
		return false;
	}
	return true;
}

function ipptSelected()
{
	document.others.ltime.value = <% getInfo("ippt-lease"); %>;
	
	if (document.others.ippt.value == 255) {
		document.others.ltime.disabled = true;
		document.others.lan_acc.disabled = true;
		/* open it, if need singlePC
		// check dependency
		if (document.others.singlePC.checked && document.others.IPtype[1].checked) {
			document.others.singlePC.checked=false;
			document.others.IPtype[0].disabled = true;
		}
		document.others.IPtype[1].disabled = true;
		*/
	}
	else {
		document.others.ltime.disabled = false;
		document.others.lan_acc.disabled = false;
		/* open it, if need singlePC
		// check dependency
		if (document.others.singlePC.checked)
			document.others.IPtype[1].disabled = false;
		*/
	}
}

function singlePCSelected()
{
	if (document.others.singlePC.checked) {
		document.others.IPtype[0].disabled = false;
		// check dependency
		if (document.others.ippt.value==255) {
			document.others.IPtype[1].disabled = true;
			document.others.IPtype[0].checked = true;
		}
		else
			document.others.IPtype[1].disabled = false;
	}
	else {
		document.others.IPtype[0].disabled = true;
		document.others.IPtype[1].disabled = true;
	}
}

</SCRIPT>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">Other Advanced Configuration</font></h2>

<form action=/goform/formOthers method=POST name="others">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><font size=2>
    Here you can set some other advanced settings.
  </tr>
  <tr><hr size=1 noshade align=top></tr>
</table>

<table border=0 width="500" cellspacing=4 cellpadding=0>
<!--
  <tr>
      <th align=left><font size=2><b>PVC Auto-search:</b></th>
      <td>
      	<input type=radio value=0 name="autosearch" <% if (getIndex("vc-auto")==0) write("checked");%>>Disable&nbsp;&nbsp;&nbsp;&nbsp;
     	<input type=radio value=1 name="autosearch" <% if (getIndex("vc-auto")==1) write("checked");%>>Enable
      </td>
  </tr>
-->

  <tr>
      <th align=left><font size=2><b>IP PassThrough:</b></th>
      <td>
      	<select name="ippt" onChange=ipptSelected()>
      	  <option value=255>None</option>
          <%  if_wan_list("p2p");  %>
      	</select>&nbsp;&nbsp;&nbsp;&nbsp;
      	Lease Time:&nbsp;&nbsp;
        <input type="text" name="ltime" size=10 maxlength=9 value="<% getInfo("ippt-lease"); %>"> seconds
      </td>
  </tr>
  <tr>
      <th></th>
      <td>
       	<input type="checkbox" name="lan_acc" value="ON">&nbsp;&nbsp;Allow LAN access
      </td>
  </tr>
<!--
  <tr>
      <th align=left><font size=2><b>Single PC Mode:</b></th>
      <td>
       	<input type="checkbox" name="singlePC" value="ON" onClick=singlePCSelected()
       	<%if (getIndex("spc-enable")==1) write("checked");%>>&nbsp;&nbsp;Enable
      </td>
  </tr>
  <tr>
      <th></th>
      <td>
      	<input type=radio value=0 name="IPtype" <% if (getIndex("spc-iptype")==0) write("checked");
      		if (getIndex("spc-enable")==0) write(" disabled"); %>>Use Private IP&nbsp;&nbsp;&nbsp;&nbsp;
     	<input type=radio value=1 name="IPtype" <% if (getIndex("spc-iptype")==1) write("checked");
     		if (getIndex("spc-enable")==0) write(" disabled"); %>>Use IP Passthrough
      </td>
  </tr>
-->
</table>
  <br>
      <input type=submit value="Apply Changes" name="save" onClick="return saveChanges()">
<!--
      <input type=reset value="Undo" name="reset">
-->
      <input type=hidden value="/others.asp" name="submit-url">
  <script>
	document.others.ippt.value = <% getInfo("ippt-itf"); %>;
	<% initPage("others"); %>
  </script>
</form>
</blockquote>
</body>

</html>
