<html>
<! Copyright (c) Realtek Semiconductor Corp., 2008. All Rights Reserved. ->
<head>
<title>IP QoS Traffic Shaping</title>
<META http-equiv=pragma content=no-cache>
<META http-equiv=cache-control content="no-cache, must-revalidate">
<META http-equiv=content-type content="text/html; charset=gbk">
<META http-equiv=content-script-type content=text/javascript>
<!--ϵͳ����css-->
<STYLE type=text/css>
@import url(style/default.css);
.STYLE1 {color: #FF0000}
</STYLE>
<script type="text/javascript" src="share.js">
</script>
<script>

var protos = new Array("-", "ICMP", "TCP", "UDP", "TCP/UDP");
var traffictlRules = new Array();
var totalBandwidth;
var totalBandWidthEn;

function it_nr(id, inf, proto, sport, dport, srcip, dstip, uprate) {
  this.id = id;
  this.inf = inf;
  this.proto = proto;
  this.sport = sport;
  this.dport = dport;
  this.srcip = srcip;
  this.dstip = dstip;
  this.uprate = uprate;
}

<% initTraffictlPage(); %>

function on_chkdel(index) {
	if(index<0 || index>=traffictlRules.length)
		return;
	traffictlRules[index].select = !traffictlRules[index].select;
}

/********************************************************************
**          on document load
********************************************************************/
function on_init_page(){
	
	with(document.forms[0]) {
		if (totalBandWidthEn==0)
			totalbandwidth.value = "";
		else
			totalbandwidth.value = totalBandwidth;
		
		if(traffictl_tbl.rows){
			while(traffictl_tbl.rows.length > 1) 
				traffictl_tbl.deleteRow(1);
		}

		for(var i = 0; i < traffictlRules.length; i++)
		{
			var row = traffictl_tbl.insertRow(i + 1);
		
			row.nowrap = true;
			row.vAlign = "center";
			row.align = "left";

			var cell = row.insertCell(0);
			cell.style.color = "#404040";
			cell.align = "center";
			cell.innerHTML = traffictlRules[i].id;
			cell = row.insertCell(1);
			cell.style["color"] = "#404040";
			cell.align = "center";
			cell.innerHTML = traffictlRules[i].inf;
			cell = row.insertCell(2);
			cell.style["color"] = "#404040";
			cell.align = "center";
			cell.innerHTML = protos[traffictlRules[i].proto];
			cell = row.insertCell(3);
			cell.style["color"] = "#404040";
			cell.align = "center";
			if (traffictlRules[i].sport == "0")
				cell.innerHTML = "-";
			else
				cell.innerHTML = traffictlRules[i].sport;
			cell = row.insertCell(4);
			cell.style["color"] = "#404040";
			cell.align = "center";
			if (traffictlRules[i].dport == "0")
				cell.innerHTML = "-";
			else
				cell.innerHTML = traffictlRules[i].dport;
			cell = row.insertCell(5);
			cell.style["color"] = "#404040";
			cell.align = "center";
			if ((traffictlRules[i].srcip == "0.0.0.0/32") || (traffictlRules[i].srcip == "0.0.0.0"))
				cell.innerHTML = "-";
			else
				cell.innerHTML = traffictlRules[i].srcip;
			cell = row.insertCell(6);
			cell.style["color"] = "#404040";
			cell.align = "center";
			if ((traffictlRules[i].dstip == "0.0.0.0/32") || (traffictlRules[i].dstip == "0.0.0.0"))
				cell.innerHTML = "-";
			else
				cell.innerHTML = traffictlRules[i].dstip;
			cell = row.insertCell(7);
			cell.style["color"] = "#404040";
			cell.align = "center";
			cell.innerHTML = traffictlRules[i].uprate;
			cell = row.insertCell(8);
			cell.style["color"] = "#404040";
			cell.align = "center";
			cell.innerHTML = "<input type=\"checkbox\" onClick=\"on_chkdel(" + i + ");\">";
		}
	}
}


//apply total bandwidth limit
function on_apply_bandwidth(){
	
	with(document.forms[0]) {
		var sbmtstr = "applybandwidth&bandwidth=";
		var bandwidth = -1;
		if (totalbandwidth.value != "") {
			bandwidth = parseInt(totalbandwidth.value);
			if(bandwidth<0 || bandwidth >Number.MAX_VALUE)
				return;
			sbmtstr += bandwidth;
		} else {
			sbmtstr += 0;
		}
		lst.value = sbmtstr;
		submit();
	}
}

//submit traffic shaping rules
function on_submit(){
	var sbmtstr = "applysetting#id=";
	var firstFound = true;
	for(var i=0; i<traffictlRules.length; i++)
	{
		if(traffictlRules[i].select)
		{
			if(!firstFound)
				sbmtstr += "|";
			else
				firstFound = false;
			sbmtstr += traffictlRules[i].id;
		}
	}
	document.forms[0].lst.value = sbmtstr;
	document.forms[0].submit();
}

function onSelProt()
{
	with(document.forms[0]) {
		if (protolist.selectedIndex >= 2)
		{
			sport.disabled = false;
			dport.disabled = false;
		} else {
			sport.disabled = true;
			dport.disabled = true;
		}
	}
}

function on_Add()
{
	if (document.getElementById){  // DOM3 = IE5, NS6
		document.getElementById('tcrule').style.display = 'block';
	} else {
		if (document.layers == false) {// IE4
			document.all.tcrule.style.display = 'block';
		}
	}
	onSelProt();
}

function on_apply() {
	var sbmtstr = "addsetting#";
	
	with(document.forms[0]) {
		if ((srcip.value=="") && (dstip.value=="") && (sport.value=="") && (dport.value=="") && 
			(protolist.value==0))
		{
			alert("please assign at least one condition!");
			return;
		}
		
		if (inflist.value == 0)
		{
			inflist.focus();
			alert("wan interface not assigned!");
			return;
		}

		if(srcip.value != "" && checkIP(srcip) == false)
		{
			srcip.focus();
			return;
		}
		
		if(dstip.value != "" && checkIP(dstip) == false)
		{
			dstip.focus();
			return;
		}
		
		if(srcnetmask.value != "" && checkMask(srcnetmask) == false)
		{
			srcnetmask.focus();
			return;
		}
		
		if(dstnetmask.value != "" && checkMask(dstnetmask) == false)
		{
			dstnetmask.focus();
			return;
		}
		if(sport.value <0 || sport.value > 65536)
		{
			sport.focus();
			alert("source port "+ sport.value + " invalid!");
			return;
		}
		if (sport.value > 0 && sport.value < 65535)
		{
			if (protolist.value < 2) {
				sport.focus();
				alert("please assign TCP/UDP!");
				return;
			}
		}
		if(dport.value <0 || dport.value > 65536)
		{
			dport.focus();
			alert("dest port "+ dport.value + " invalid��");
			return;
		}
		if (dport.value > 0 && dport.value<65535)
		{
			if (protolist.value < 2) {
				dport.focus();
				alert("please assign TCP/UDP!");
				return;
			}
		}
		if(uprate.value<0)
		{
			uprate.focus();
			alert("uplink rate "+ uprate.value + " invalid��");
			return;
		}
		sbmtstr += "inf="+inflist.value+"&proto="+protolist.value+"&srcip="+srcip.value+"&srcnetmask="+srcnetmask.value+
			"&dstip="+dstip.value+"&dstnetmask="+dstnetmask.value+"&sport="+sport.value+"&dport="+dport.value+"&uprate="+uprate.value;
		lst.value = sbmtstr;
		submit();
	}
}

</script>
</head>

<body topmargin="0" leftmargin="0" marginwidth="0" marginheight="0" alink="#000000" link="#000000" vlink="#000000" onLoad="on_init_page();">
<blockquote>
<DIV align="left" style="padding-left:20px; padding-top:5px;">
<h2><font color="#0000FF">IP QoS Traffic Shaping</font></h2>
<form action="/goform/formQosShape" method="post" name="qosShape">

<table border=0 width=600 cellspacing=4 cellpadding=0>
<tr><td><font size=2>Entries in this table are used for traffic control.</font></td></tr>
<tr><td colspan=4><hr size=1 noshade align=top></td></tr>
</table>

<p>Total Bandwidth Limit:<input type="text" id="totalbandwidth" size="6" maxlength="6" value="1024">Kb</p>
<table class="flat" id="traffictl_tbl" border="1" cellpadding="0" cellspacing="1">
<tr class="hdb" align="center" nowrap>
	<td bgcolor="#808080">ID</td>
	<td bgcolor="#808080">WAN Inf</td>
	<td bgcolor="#808080">Protocol</td>
	<td bgcolor="#808080">Src Port</td>
	<td bgcolor="#808080">Dst Port</td>
	<td bgcolor="#808080">Src IP</td>
	<td bgcolor="#808080">Dst IP</td>
	<td bgcolor="#808080">rate(kb/s)</td>
	<td bgcolor="#808080">Delete</td>
</tr>
</table>

<table>
<tr align="left">
<td>
	<input type="button" class="button" onClick="on_Add()" value="Add">
</td>
<td>
	<input type="button" class="button" onClick="on_submit();" value="Save/Apply">
</td>
<td>
	<input type="button" class="button" onClick="on_apply_bandwidth();" value="applyTotalBandwidthLimit">
</td>
</tr>
</table>

<div id="tcrule" style="display:none">
<p></p>
<table cellSpacing="1" cellPadding="0" border="0">		
<tr>
	<td>Interface��</td>
	<td>
	  <select id="inflist">
	    <option value=0> </option>
	    <%  if_wan_list("rt");%>
	  </select>
	</td>
</tr>
<tr>
	<td>Protocol��</td>
	<td>
		<select name="protolist" onClick="return onSelProt()">
			<option value="0">NONE</option>
			<option value="1">ICMP</option>
			<option value="2">TCP </option>
			<option value="3">UDP </option>
			<option value="4">TCP/UDP</option>
		</select>
	</td>
</tr>
<tr>
	<td>Src IP��</td>
	<td><input type="text" name="srcip" size="15" maxlength="15" style="width:150px"></td>
	<td>Src Mask��</td>
	<td><input type="text" name="srcnetmask" size="15" maxlength="15" style="width:150px"></td>
</tr>
<tr>
	<td>Dst IP��</td>
	<td><input type="text" name="dstip" size="15" maxlength="15" style="width:150px"></td>
	<td>Dst Mask��</td>
	<td><input type="text" name="dstnetmask" size="15" maxlength="15" style="width:150px"></td>
</tr>
<tr>
	<td>Src Port��</td>
	<td><input type="text" name="sport" size="6" style="width:80px"></td>
	<td>Dst Port��</td>
	<td><input type="text" name="dport" size="6" style="width:80px"></td>
</tr>
<tr>
	<td>uplink rate��</td>
	<td><input type="text" name="uprate" size="6" style="width:80px">kb/s</td>
</tr>
</table>
<br>
<input type="button" name="apply" value="save" onClick="on_apply();" style="width:80px">
</div>
<input type="hidden" id="lst" name="lst" value="">
<input type="hidden" name="submit-url" value="/ipqos_shape.asp">

</form>
</DIV>
</blockquote>
</body>
</html>

