<html>
<! Copyright (c) Realtek Semiconductor Corp., 2003. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>VLan configuration </title>
<% language=javascript %>
</head>

<body>
<blockquote>
<h2><font color="#0000FF">VLan Configuration</font></h2>

<form action=/goform/formVlanCfg method=POST name="vlan">
<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr><font size=2>
    Here you can set the port-based VLan.
  </tr>
  <tr><hr size=1 noshade align=top></tr>
</table>

<table border=0 width="500" cellspacing=4 cellpadding=0>
  <tr>
      <th align=left><font size=2><b>VLan A:</b></th>
      <td>
       	<input type="checkbox" name="pa0" value="1">&nbsp;eth0_sw0 &nbsp
       	<input type="checkbox" name="pa1" value="1">&nbsp;eth0_sw1 &nbsp
       	<input type="checkbox" name="pa2" value="1">&nbsp;eth0_sw2 &nbsp
       	<input type="checkbox" name="pa3" value="1">&nbsp;eth0_sw3 &nbsp
      </td>
  </tr>
  <tr>
      <th></th>
      <td>
      	VLan Tag:&nbsp;&nbsp;
        <input type="text" name="tag_a" size=5 maxlength=3>
      </td>
  </tr>
  
  <tr>
      <th align=left><font size=2><b>VLan B:</b></th>
      <td>
       	<input type="checkbox" name="pb0" value="1">&nbsp;eth0_sw0 &nbsp
       	<input type="checkbox" name="pb1" value="1">&nbsp;eth0_sw1 &nbsp
       	<input type="checkbox" name="pb2" value="1">&nbsp;eth0_sw2 &nbsp
       	<input type="checkbox" name="pb3" value="1">&nbsp;eth0_sw3 &nbsp
      </td>
  </tr>
  <tr>
      <th></th>
      <td>
      	VLan Tag:&nbsp;&nbsp;
        <input type="text" name="tag_b" size=5 maxlength=3>
      </td>
  </tr>

  <tr>
      <th align=left><font size=2><b>VLan C:</b></th>
      <td>
       	<input type="checkbox" name="pc0" value="1">&nbsp;eth0_sw0 &nbsp
       	<input type="checkbox" name="pc1" value="1">&nbsp;eth0_sw1 &nbsp
       	<input type="checkbox" name="pc2" value="1">&nbsp;eth0_sw2 &nbsp
       	<input type="checkbox" name="pc3" value="1">&nbsp;eth0_sw3 &nbsp
      </td>
  </tr>
  <tr>
      <th></th>
      <td>
      	VLan Tag:&nbsp;&nbsp;
        <input type="text" name="tag_c" size=5 maxlength=3>
      </td>
  </tr>

  <tr>
      <th align=left><font size=2><b>VLan D:</b></th>
      <td>
       	<input type="checkbox" name="pd0" value="1">&nbsp;eth0_sw0 &nbsp
       	<input type="checkbox" name="pd1" value="1">&nbsp;eth0_sw1 &nbsp
       	<input type="checkbox" name="pd2" value="1">&nbsp;eth0_sw2 &nbsp
       	<input type="checkbox" name="pd3" value="1">&nbsp;eth0_sw3 &nbsp
      </td>
  </tr>
  <tr>
      <th></th>
      <td>
      	VLan Tag:&nbsp;&nbsp;
        <input type="text" name="tag_d" size=5 maxlength=3>
      </td>
  </tr>

  <tr><td colspan=2><hr size=2 align=top></td></tr>
  <tr>
      <th align=left><font size=2><b>eth0_sw0:</b></th>
      <td>PVID:&nbsp;&nbsp;
      	<select name="pidx0">
      	<option value=0>VLAN A</option>
      	<option value=1>VLAN B</option>
      	<option value=2>VLAN C</option>
      	<option value=3>VLAN D</option>
      	</select>&nbsp;&nbsp;
      	Egress Tag:&nbsp;&nbsp;
      	<select name="act0">
      	<option value=3>Don't care</option>
      	<option value=2>Add</option>
      	<option value=1>Remove</option>
      	<option value=0>Remove and add</option>
      	</select>
      </td>
  </tr>
  
  <tr>
      <th align=left><font size=2><b>eth0_sw1:</b></th>
      <td>PVID:&nbsp;&nbsp;
      	<select name="pidx1">
      	<option value=0>VLAN A</option>
      	<option value=1>VLAN B</option>
      	<option value=2>VLAN C</option>
      	<option value=3>VLAN D</option>
      	</select>&nbsp;&nbsp;
      	Egress Tag:&nbsp;&nbsp;
      	<select name="act1">
      	<option value=3>Don't care</option>
      	<option value=2>Add</option>
      	<option value=1>Remove</option>
      	<option value=0>Remove and add</option>
      	</select>
      </td>
  </tr>

  <tr>
      <th align=left><font size=2><b>eth0_sw2:</b></th>
      <td>PVID:&nbsp;&nbsp;
      	<select name="pidx2">
      	<option value=0>VLAN A</option>
      	<option value=1>VLAN B</option>
      	<option value=2>VLAN C</option>
      	<option value=3>VLAN D</option>
      	</select>&nbsp;&nbsp;
      	Egress Tag:&nbsp;&nbsp;
      	<select name="act2">
      	<option value=3>Don't care</option>
      	<option value=2>Add</option>
      	<option value=1>Remove</option>
      	<option value=0>Remove and add</option>
      	</select>
      </td>
  </tr>

  <tr>
      <th align=left><font size=2><b>eth0_sw3:</b></th>
      <td>PVID:&nbsp;&nbsp;
      	<select name="pidx3">
      	<option value=0>VLAN A</option>
      	<option value=1>VLAN B</option>
      	<option value=2>VLAN C</option>
      	<option value=3>VLAN D</option>
      	</select>&nbsp;&nbsp;
      	Egress Tag:&nbsp;&nbsp;
      	<select name="act3">
      	<option value=3>Don't care</option>
      	<option value=2>Add</option>
      	<option value=1>Remove</option>
      	<option value=0>Remove and add</option>
      	</select>
      </td>
  </tr>
</table>
  <br>
      <input type=submit value="Apply Changes" name="save">
      <input type=hidden value="/vlancfg.asp" name="submit-url">
  <script>
  	<% initPage("vlan"); %>
	<%vlanPost();%>
  </script>

</form>
</blockquote>
</body>

</html>
