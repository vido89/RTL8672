<html><head>

<!-- Copyright (c) sfstudio http://wive-ng.sf.net 2009. All Rights Reserved. -->

<meta http-equiv="Content-Type" content="text/html; charset=windows-1251">
<meta http-equiv="Cache-Control" content="no-cache"><meta http-equiv="PRAGMA" content="NO-CACHE">
<title>PPTP tunnel setup </title>
<script type="text/javascript" src="share.js">
</script>
<style type="text/css">
	td          { font-size: x-small; vertical-align:middle; }
	div.hint    { margin-top: 20px; }
	p.val       { margin-left: 16; }
	select      { width: 192px; }
	select.half { width: 94px; }
	input.mid   { width: 192px; }
	input.half  { width: 94px; }
	acronym     { border-bottom: 0px }
</style>


<script language="javascript">
function showHint(key)
{
	var row = document.getElementById("pptp_hint_row");
	var form = document.forms["formPPTPSetup"];
	var text = '<div class="hint"><font color="#0000ff"><b>HINT:</b></font>&nbsp;';
	
	if (key=='pptp_enabled')
	{
		text += 'Enable Virtual Private Network support.';
		text += '</div>';
		row.innerHTML = text;
	}
	else if (form.pptp_enabled.checked)
	{
		if (key=='pptp_user')
			text += 'Specify user name given by your network provider.';
		else if (key=='pptp_password')
			text += 'Specify password given by your network provider.';

		else if (key=='pptp_pppoe_iface')
			text += 'Select available interface for PPPoE.';
		else if (key=='pptp_server')
			text += 'Specify server\'s domain name, IP address, Access Concentrator or Access Point name.';
		else if (key=='pptp_range')
			text += 'Specify range of IP addresses given to clients in <b>L2TP server</b> mode.';
		else if (key=='pptp_mtu')
			text += 'Specify Maximum Transfer Unit/Maximum Recieve Unit size in octets.';
		else if (key=='pptp_mppe')
			text += 'Enable automatic Microsoft Point-to-Point Encryption (MPPE) mode for VPN.';
		else if (key=='pptp_dgw')
		{
			text += 'Manage default gateway replacing in routing table.</p><p class="val">';
			if (form.pptp_dgw_select.value == '0')
				text += '<b>Off</b> means that no default gateway will be written to routing table.';
			else if (form.pptp_dgw_select.value == '1')
				text += '<b>Enabled</b> means that default gateway will be replaced by gateway given by network provider.';
			else if (form.pptp_dgw_select.value == '2')
				text += '<b>Multiple</b> means that default gateway will be added to existing gateway but with metric 10.';
			text += '</p>';
		}
		else if (key=='pptp_peerdns')
			text += 'Allow to get DNS adress from VPN server and write to /etv/ppp/resolv.conf.';
		else if (key=='pptp_debug')
			text += 'Allow debug mode for VPN connections.';
		else if (key=='pptp_nat')
			text += 'Add Network Address Translation to new VPN connection.';
		else if (key=='pptp_type')
		{
			text += 'Specify PPTP mode.<p class="val">';
		
			if (form.pptp_type.value == "0")
				text += '<b>PPPoE</b> (see RFC #2516) means encapsulating Point-to-Point Protocol (PPP) frames ' +
					'inside Ethernet frames. It is used mainly with DSL services where individual ' +
					'users connect to the DSL modem over Ethernet and in plain Metro Ethernet networks.';
			else if (form.pptp_type.value == "1")
				text += '<b>PPTP</b> (see RFC #2637) means a method for implementing virtual private networks. ' +
					'PPTP uses a control channel over TCP and a GRE tunnel operating to encapsulate ' +
					'PPP packets.';
			else if ((form.pptp_type.value == "2") || (form.pptp_type.value == "3"))
			{
				text += '<b>L2TP</b> (see RFC #2661) means a tunneling protocol used to support virtual private networks. ' +
					'It does not provide any encryption or confidentiality by itself; it relies on an ' +
					'encryption protocol that it passes within the tunnel to provide privacy.</p><p class="val">';
				if (form.pptp_type.value == "2")
					text += '<b>PPTP client</b> means a connection to remote L2TP server.';
				else
				text += '<b>PPTP server</b> means a connection from remote machines to L2TP server on this router.';
			}
			else if (form.pptp_type.value == "4")
				text += '<b>Modem GPRS</b> means connection to Internet via GPRS/EDGE in GSM mobile networks.';
			else if (form.pptp_type.value == "5")
				text += '<b>Modem CDMA</b> means connection to Internet via UMTS in CDMA mobile networks.';
			text += '</p>';
		}

		text += '</div>';
		row.innerHTML = text;
	}
}

function hideHint(ctl)
{
	var row = document.getElementById("pptp_hint_row");
	row.innerHTML = '';
}

function vpnSwitchClick(form)
{
	if (form.pptp_enabled.checked)
	{
		enableTextField(form.pptp_server);
		enableTextField(form.pptp_range);
		enableTextField(form.pptp_user);
		enableTextField(form.pptp_pass);
		enableTextField(form.pptp_mtu);
		enableCheckBox(form.pptp_mppe);
		enableCheckBox(form.pptp_peerdns);
		enableCheckBox(form.pptp_debug);
		enableCheckBox(form.pptp_nat);
		form.pptp_dgw_select.disabled = false;
		form.pptp_mtu_type.disabled = false;
		form.pptp_pppoe_iface.disabled = false;
		form.pptp_type.disabled = false;
	}
	else
	{
		disableTextField(form.pptp_server);
		disableTextField(form.pptp_range);
		disableTextField(form.pptp_user);
		disableTextField(form.pptp_pass);
		disableTextField(form.pptp_mtu);
		disableCheckBox(form.pptp_mppe);
		disableCheckBox(form.pptp_peerdns);
		disableCheckBox(form.pptp_debug);
		disableCheckBox(form.pptp_nat);
		form.pptp_dgw_select.disabled = true;
		form.pptp_mtu_type.disabled = true;
		form.pptp_pppoe_iface.disabled = true;
		form.pptp_type.disabled = true;
	}
	
	dgwSwitchClick(form);
}

function dgwSwitchClick(form)
{
	form.pptp_dgw.value  = (form.pptp_dgw_select.value == '0') ? 'off' : 'on';
	form.pptp_mdgw.value = (form.pptp_dgw_select.value == '2') ? 'on' : 'off';
}

function mtuChange(form)
{
	var pptp_mtu_select = document.getElementById("pptp_mtu_select");
	var pptp_mtu_field  = document.getElementById("pptp_mtu_field");
	
	if (form.pptp_mtu_type.value == '1')
	{
		pptp_mtu_field.style.display = '';
		pptp_mtu_select.setAttribute("class", "half");
		pptp_mtu_field.setAttribute("class", "half");
	}
	else
	{
		pptp_mtu_select.setAttribute("class", "mid");
		pptp_mtu_field.style.display = 'none';
		form.pptp_mtu.value = form.pptp_mtu_type.value;
	}
}

function bodyOnLoad(form)
{
	if (form.pptp_dgw.value == 'on')
		form.pptp_dgw_select.value = (form.pptp_mdgw.value == 'on') ? '2' : '1';
	else
		form.pptp_dgw_select.value = '0';
	
	/* Check if option was set */
	var pptp_mtu_select = document.getElementById('pptp_mtu_select');
	for (var i=0; i < pptp_mtu_select.options.length; i++)
		if (form.pptp_mtu_type.options[i].value == form.pptp_mtu.value)
		{
			form.pptp_mtu_type.value = form.pptp_mtu_select.options[i].value;
			break;
		}

	vpnSwitchClick(form);
	selectType(form);
	mtuChange(form);
}

function selectType(form)
{
	var pppoe_row = document.getElementById("pptp_type_pppoe");
	var l2tp_row  = document.getElementById("pptp_l2tp_range");

	pppoe_row.style.display = 'none';
	l2tp_row.style.display  = 'none';

	if (form.pptp_type.value == '0')
		pppoe_row.style.display = '';
	else if (form.pptp_type.value == '3')
		l2tp_row.style.display = '';
}

function resetClick(form)
{
	form.reset();
	bodyOnLoad(form);
	return true;
}

</script>
</head><body onload="bodyOnLoad(document.formPPTPSetup)">
<blockquote>
<h2><font color="#0000ff">Virtual Private Network and USB modem setup</font></h2>
<font size="2">
	This page is used to configure the <acronym title="Virtual Private Network">VPN</acronym>
	tunnel and GSM/CDMA modems on your Router.
</font>

<form action="/goform/formPPTPSetup" method="POST" name="formPPTPSetup">
<table width="500" border="0" cellpadding="0" cellspacing="4">
	<tbody>
	<tr>
		<td colspan="2"><hr></td>
	</tr>

	<tr onMouseOver="showHint('pptp_enabled')" onMouseOut="hideHint('pptp_enabled')">
		<td colspan="2">
			<input name="pptp_enabled" onclick="vpnSwitchClick(this.form)" type="checkbox" <% condIfEquals("pptp_enabled", "on", "checked=\"checked\"") %> >
			<b>Enable <acronym title="Virtual Private Network">VPN</acronym></b>
		</td>
	</tr>
	<tr onMouseOver="showHint('pptp_type')" onMouseOut="hideHint('pptp_type')" >
		<td width="50%">
			<b><acronym title="Point-to-Point Protocol">PPP</acronym> Mode:</b>
		</td>
		<td width="50%">
			<select disabled="disabled" name="pptp_type" onChange="selectType(this.form);" >
				<option value="0" <% condIfEquals("pptp_type", "0", "selected=\"selected\"") %>>PPPoE client</option>
				<option value="1" <% condIfEquals("pptp_type", "1", "selected=\"selected\"") %>>PPTP  client</option>
				<option value="2" <% condIfEquals("pptp_type", "2", "selected=\"selected\"") %>>L2TP  client</option>
				<option value="3" <% condIfEquals("pptp_type", "3", "selected=\"selected\"") %>>L2TP  server</option>
				<option value="4" <% condIfEquals("pptp_type", "4", "selected=\"selected\"") %>>Modem GPRS</option>
				<option value="5" <% condIfEquals("pptp_type", "5", "selected=\"selected\"") %>>Modem CDMA</option>
			</select>
		</td>
	</tr>
	<tr id="pptp_type_pppoe" style="display: none;" onMouseOver="showHint('pptp_pppoe_iface')" onMouseOut="hideHint('pptp_pppoe_iface')">
		<td width="30%"><b>PPPoE interface:</b></td>
		<td width="70%">
			<select disabled="disabled" name="pptp_pppoe_iface" >
				<% pptpIfaceList(); %>
			</select>
		</td>
	</tr>
	<tr onMouseOver="showHint('pptp_server')" onMouseOut="hideHint('pptp_server')">
		<td width="50%"><b>Host, <acronym title="Internet Protocol">IP</acronym>, <acronym title="Access Concentrator">AC</acronym> or <acronym title="Access Point Name">APN</acronym> name:</b></td>
		<td width="50%"><input name="pptp_server" class="mid" size="25" maxlength="60" value="<% getInfo("pptp_server"); %>" disabled="disabled" type="text"></td>
	</tr>
	<tr id="pptp_l2tp_range" onMouseOver="showHint('pptp_range')" onMouseOut="hideHint('pptp_range')" style="display: none;" >
		<td width="50%"><b><acronym title="Virtual Private Network">VPN</acronym> range <acronym title="Internet Protocol">IP</acronym> adresses:</b></td>
		<td width="50%"><input name="pptp_range" class="mid" size="25" maxlength="60" value="<% getInfo("pptp_range"); %>" disabled="disabled" type="text"></td>
	</tr>
	<tr onMouseOver="showHint('pptp_user')" onMouseOut="hideHint('pptp_user')" >
		<td width="50%"><b>User name:</b></td>
		<td width="50%"><input name="pptp_user" class="mid" size="25" maxlength="60" value="<% getInfo("pptp_user"); %>" disabled="disabled" type="text"></td>
	</tr>
	<tr onMouseOver="showHint('pptp_password')" onMouseOut="hideHint('pptp_password')" >
		<td width="50%"><b>Password:</b></td>
		<td width="50%"><input name="pptp_pass" class="mid" size="25" maxlength="60" value="<% getInfo("pptp_pass"); %>" disabled="disabled" type="password"></td>
	</tr>
	<tr onMouseOver="showHint('pptp_mtu')" onMouseOut="hideHint('pptp_mtu')" >
		<td width="50%"><b><acronym title="Maximum Transfer Unit">MTU</acronym>/<acronym title="Maximum Recieve Unit">MRU:</acronym></b></td>
		<td width="50%">
			<input id="pptp_mtu_field" name="pptp_mtu" maxlength="4" disabled="disabled" type="text" class="half" style="display:none; " value="<% getInfo("pptp_mtu") %>" >
			<select id="pptp_mtu_select" disabled="disabled" name="pptp_mtu_type" onChange="mtuChange(this.form);" class="mid" >
				<option value="AUTO">AUTO</option>
				<option value="1" selected="selected">Custom</option>
				<option value="1500">1500</option>
				<option value="1492">1492</option>
				<option value="1440">1440</option>
				<option value="1400">1400</option>
				<option value="1300">1300</option>
				<option value="1200">1200</option>
				<option value="1100">1100</option>
				<option value="1000">1000</option>
			</select>
		</td>
	</tr>
	<tr onMouseOver="showHint('pptp_dgw')" onMouseOut="hideHint('pptp_dgw')" >
		<td width="50%" >
			<b>Default gateway:</b>
		</td>
		<td width="50%">
			<select disabled="disabled" name="pptp_dgw_select" class="mid" onChange="dgwSwitchClick(this.form);" >
				<option value="0">Off</option>
				<option value="1">Enabled</option>
				<option value="2">Multiple</option>
			</select>
		</td>
	</tr>
</tbody></table>

<table width="500" border="0" cellpadding="0" cellspacing="4">
	<tbody><tr>
		<td width="50%" onMouseOver="showHint('pptp_mppe')" onMouseOut="hideHint('pptp_mppe')" >
			<input disabled="disabled" name="pptp_mppe" checked="checked" type="checkbox" <% condIfEquals("pptp_mppe", "on", "checked=\"checked\"") %>>
			<b>Allow <acronym title="Microsoft Point-to-Point Encryption">MPPE</acronym></b>
		</td>
		<td width="50%" onMouseOver="showHint('pptp_peerdns')" onMouseOut="hideHint('pptp_peerdns')" >
			<input disabled="disabled" name="pptp_peerdns" type="checkbox" <% condIfEquals("pptp_peerdns", "on", "checked=\"checked\"") %>>
			<b>Peer <acronym title="Domain Name Server">DNS</acronym></b>
		</td>
	</tr>
	<tr>
		<td width="50%" onMouseOver="showHint('pptp_debug')" onMouseOut="hideHint('pptp_debug')" >
			<input disabled="disabled" name="pptp_debug" type="checkbox" <% condIfEquals("pptp_debug", "on", "checked=\"checked\"") %>>
			<b>Allow debug</b>
		</td>
		<td width="50%" onMouseOver="showHint('pptp_nat')" onMouseOut="hideHint('pptp_nat')" >
			<input disabled="disabled" name="pptp_nat" type="checkbox" <% condIfEquals("pptp_nat", "on", "checked=\"checked\"") %>>
			<b>Enable <acronym title="Network Address Translation">NAT</acronym></b>
	</tr>
	<tr></tr>
	<tr>
		<td colspan="2">
			<input type="hidden" name="pptp_dgw" value="<% getInfo("pptp_dgw") %>">
			<input type="hidden" name="pptp_mdgw" value="<% getInfo("pptp_mdgw") %>">
			<input value="/pptp-set.asp" name="submit-url" type="hidden">
			<input style="none" value="Apply and connect" name="save" type="submit">&nbsp;&nbsp;
			<input style="none" value="Reset" name="reset_button" onclick="resetClick(this.form);" type="button">
		</td>
	</tr>
	<tr height="32px"  >
		<td colspan="2" id="pptp_hint_row"</td>
	</tr>
</tbody></table>
</form>

</blockquote>
</body></html>
