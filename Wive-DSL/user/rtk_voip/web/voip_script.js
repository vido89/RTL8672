function isArray(obj) {
	return (typeof(obj.length) == "undefined") ? false : true;
}

function isNumeric(num,min,max)
{  
	return !isNaN(num) && num >= min && num <= max;
}

function ERROR(obj, msg)
{
	alert(msg);
	obj.select();
	obj.focus();
}

function updatePrecedence()
{
	var len;
	var selection;

	len = sipform.codec_num.value;
	selection = 0;
	for (i=0; i<len; i++)
	{
		for (j=0; j<len; j++)
		{
			if (sipform.precedence[i * len + j].checked == true)
			{
				sipform.preced_id[i].value = j;
				selection++;
				break;
			}
		}
	}

	return selection;
}

function checkPrecedence(obj, row, col)
{
	var x, y;
	var i, len, idx;

	x = -1;
	y = -1;
	len = sipform.codec_num.value;
	if (obj.checked == true)
	{
		for (i=0; i<len; i++)
		{
			idx = row * len + i;
			if (obj == sipform.precedence[idx])
				continue;
			if (sipform.precedence[idx].checked == true)
			{
				x = i;
				sipform.precedence[idx].checked = false;
				break;
			}
		}

		for (i=0; i<len; i++)
		{
			idx = i * len + col;
			if (obj == sipform.precedence[idx])
				continue;
			if (sipform.precedence[idx].checked == true)
			{
				y = i;
				sipform.precedence[idx].checked = false;
				break;
			}
		}

		if ((x != -1) && (y != -1))
			sipform.precedence[x + y * len].checked = true;
	}
	else
	{
		obj.checked = true;
	}
}

function enableCFSpkAGC()
{

	if (sipform.CFuseSpeaker.checked == false)
	{
		sipform.CF_spk_AGC_level.disabled=true;
		sipform.CF_spk_AGC_up_limit.disabled=true;
		sipform.CF_spk_AGC_down_limit.disabled=true;
	}
	else
	{
		sipform.CF_spk_AGC_level.disabled=false;
		sipform.CF_spk_AGC_up_limit.disabled=false;
		sipform.CF_spk_AGC_down_limit.disabled=false;
		
		sipform.CFuseMIC.checked = false;
		enableCFMicAGC();
	}

}

function enableCFMicAGC()
{
	if (sipform.CFuseMIC.checked == false)
	{
		sipform.CF_mic_AGC_level.disabled=true;
		sipform.CF_mic_AGC_up_limit.disabled=true;
		sipform.CF_mic_AGC_down_limit.disabled=true;
	}
	else
	{
		sipform.CF_mic_AGC_level.disabled=false;
		sipform.CF_mic_AGC_up_limit.disabled=false;
		sipform.CF_mic_AGC_down_limit.disabled=false;
		
		sipform.CFuseSpeaker.checked = false;
		enableCFSpkAGC();
	}
}

function check_nortel_proxy()
{
	var use_nortel_proxy;

	if (isArray(sipform.proxyNortel))
		use_nortel_proxy = sipform.proxyNortel[sipform.default_proxy.value].checked;
	else
		use_nortel_proxy = sipform.proxyNortel.checked;

	sipform.stunEnable.disabled = use_nortel_proxy;
	sipform.stunAddr.disabled = use_nortel_proxy;
	sipform.stunPort.disabled = use_nortel_proxy;
}

function default_proxy_change()
{
	if (isArray(sipform.proxyNortel))
	{
		for (i=0; i<sipform.proxyNortel.length; i++)
			sipform.proxyNortel[i].disabled = (i != sipform.default_proxy.value);
	}

	check_nortel_proxy();
}

function init()
{
	check_nortel_proxy();
	enableCFSpkAGC();
	enableCFMicAGC();
}

function chkPayloadType()
{
	var v0 			= sipform.payloadType.value;
	var v1 			= parseInt(v0);

	if (sipform.dtmfMode.selectedIndex == 0)
	{
		if (isNaN(v1) || v1 < 96 || v1 > 120)
		{
			ERROR(sipform.payloadType, 'Payload Type is out of range [96-120]');
			return false;
		}
		else if (v1 == 97 || v1 == 98)
		{
			alert('Payload Type ' + v1 + ' has reserved for iLBC');
			return false;
		}
	}
	return true;
}

function checkDialPlanRegex( szRegex )
{
	var re = new RegExp ("^[0-9xX\*\#\+]*$", "g");

	return re.test(szRegex);
}

function checkDialPlanDigits( szDigits )
{
	var re = new RegExp ("^[0-9\*\#]*$", "g");

	return re.test(szDigits);
}

function changeStartEnd() 
{
	var proxy_enables, proxy_addrs, proxy_ports, reg_expires;
	var obproxy_enables, obproxy_addrs, obproxy_ports;
	var stun_enables, stun_addrs, stun_ports;
	var sip_ports, rtp_ports, t38_ports;
	var voipport = parseInt(sipform.voipPort.value);
	var sipport = parseInt(sipform.sipPort.value);
	var rtpport = parseInt(sipform.rtpPort.value);
	var t38port = parseInt(sipform.T38_PORT.value);
	var reg_pstn	= /^[*#0-9]*$/;

	// proxies
	if (isArray(sipform.proxyEnable))
	{
		proxy_enables = sipform.proxyEnable;
		proxy_addrs = sipform.proxyAddr;
		proxy_ports = sipform.proxyPort;
		reg_expires = sipform.regExpiry;
		obproxy_enables = sipform.obEnable;
		obproxy_addrs = sipform.obProxyAddr;
		obproxy_ports = sipform.obProxyPort;
		reg_expires = sipform.regExpiry;
	}
	else
	{
		proxy_enables = new Array();
		proxy_enables[0] = sipform.proxyEnable;
		proxy_addrs = new Array();
		proxy_addrs[0] = sipform.proxyAddr;
		proxy_ports = new Array();
		proxy_ports[0] = sipform.proxyPort;
		reg_expires = new Array();
		reg_expires[0] = sipform.regExpiry;
		obproxy_enables = new Array();
		obproxy_enables[0] = sipform.obEnable;
		obproxy_addrs = new Array();
		obproxy_addrs[0] = sipform.obProxyAddr;
		obproxy_ports = new Array();
		obproxy_ports[0] = sipform.obProxyPort;
	}

	for (var i=0; i<proxy_enables.length; i++)
	{
		var regexpiry   = parseInt(reg_expires[i].value);

		if (proxy_enables[i].checked == true)
		{
			if (proxy_addrs[i].value == "")
			{
				ERROR(proxy_addrs[i], 'Porxy Addr cannot be empty');
				return false;			
			}
			if (!isNumeric(proxy_ports[i].value, 0, 65535))
			{
				ERROR(proxy_ports[i], 'Porxy Port is out of range [0-65535]');
				return false;
			}
		}
		if (!isNumeric(reg_expires[i].value, 10, 86400))
		{
			ERROR(reg_expires[i], 'Register Expire out of range [10-86400]');
			return false;
		}
		if (obproxy_enables[i].checked == true)
		{
			if (obproxy_addrs[i].value == "")
			{
				ERROR(obproxy_addrs[i], 'Outbound Porxy Addr cannot be empty');
				return false;
			}
			if (!isNumeric(obproxy_ports[i].value, 0, 65535))
			{
				ERROR(obproxy_ports[i], 'Outbound Porxy Port is out of range [0-65535]');
				return false;
			}
		}
	}

	if (sipform.stunEnable.checked == true)
	{
		if (sipform.stunAddr.value == "")
		{
			ERROR(sipform.stunAddr, 'Stun Server Addr cannot be empty');
			return false;
		}
		if (!isNumeric(sipform.stunPort.value, 0, 65535))
		{
			ERROR(sipform.stunPort, 'Stun Server Port is out of range [0-65535]');
			return false;
		}
	}

	if (!isNumeric(sipform.sipPort.value, 5000, 10000))
	{
		ERROR(sipform.sipPort, 'SIP Port out of range [5000-10000]');
		return false;
	}

	if (!isNumeric(sipform.rtpPort.value, 5000, 10000))
	{
		ERROR(sipform.rtpPort, 'Media Port out of range [5000-10000]');
		return false;
	}

	if (!isNumeric(sipform.T38_PORT.value, 5000, 10000))
	{
		ERROR(sipform.T38_PORT, 'T38 Port out of range [5000-10000]');
		return false;
	}

	if (isArray(document.all.sipPorts))
	{
		sip_ports = document.all.sipPorts;
		rtp_ports = document.all.rtpPorts;
		t38_ports = document.all.t38Ports;
	}
	else
	{
		sip_ports = new Array();
		sip_ports[0] = document.all.sipPorts;
		rtp_ports = new Array();
		rtp_ports[0] = document.all.rtpPorts;
		t38_ports = new Array();
		t38_ports[0] = document.all.t38Ports;
	}

	for (var i=0; i<sip_ports.length; i++)
	{
		var port, min, max;

		if (i == voipport) // the same page
		{
			// check sip
			max = sipport;
			min = parseInt(max) - 3;
			if (rtpport >= min && rtpport <= max)
			{
				ERROR(sipform.rtpPort, "The RTP port value couldn't be the in the range1 " + min + " ~ " + max);
				return false;
			}
			min = parseInt(max) - 1;
			if (t38port && t38port >= min && t38port <= max)
			{
				ERROR(sipform.T38_PORT, "The T38 port value couldn't be the in the range2 " + min + " ~ " + max);
				return false;
			}
			// check rtp
			max = parseInt(rtpport) + 3;
			min = parseInt(rtpport) - 1;
			if (t38port && t38port >= min && t38port <= max)
			{
				ERROR(sipform.T38_PORT, "The T38 port value couldn't be the in the range3 " + min + " ~ " + max);
				return false;
			}
		}
		else 
		{
			// check sip
			port = sip_ports[i].value;
			if (sipport == port)
			{
				ERROR(sipform.sipPort, "The SIP port value couldn't be the same.");
				return false;
			}
			max = port;
			min = parseInt(max) - 3;
			if (rtpport >= min && rtpport <= max)
			{
				ERROR(sipform.rtpPort, "The RTP port value couldn't be the in the range4 " + min + " ~ " + max);
				return false;
			}
			min = parseInt(max) - 1;
			if (t38port && t38port >= min && t38port <= max)
			{
				ERROR(sipform.T38_PORT, "The T38 port value couldn't be the in the range5 " + min + " ~ " + max);
				return false;
			}
			// check rtp
			port = rtp_ports[i].value;
			max = parseInt(port) + 3;
			min = port;
			if (sipport >= min && sipport <= max)
			{
				ERROR(sipform.sipPort, "The SIP port value couldn't be the in the range6 " + min + " ~ " + max);
				return false;
			}
			min = parseInt(port) - 3;
			if (rtpport >= min && rtpport <= max)
			{
				ERROR(sipform.rtpPort, "The RTP port value couldn't be the in the range7 " + min + " ~ " + max);
				return false;
			}
			min = parseInt(port) - 1;
			if (t38port && t38port >= min && t38port <= max)
			{
				ERROR(sipform.T38_PORT, "The T38 port value couldn't be the in the range8 " + min + " ~ " + max);
				return false;
			}
			// check t38
			port = t38_ports[i].value;
			max = parseInt(port) + 1;
			min = port;
			if (sipport >= min && sipport <= max)
			{
				ERROR(sipform.sipPort, "The SIP port value couldn't be the in the range9 " + min + " ~ " + max);
				return false;
			}
			min = parseInt(port) - 3;
			if (rtpport >= min && rtpport <= max)
			{
				ERROR(sipform.rtpPort, "The RTP port value couldn't be the in the range10 " + min + " ~ " + max);
				return false;
			}
			min = parseInt(port) - 1;
			if (t38port && t38port >= min && t38port <= max)
			{
				ERROR(sipform.T38_PORT, "The T38 port value couldn't be the in the range11 " + min + " ~ " + max);
				return false;
			}
		}
	}

	if (sipform.CFAll[2].checked &&
		!reg_pstn.test(sipform.CFAll_No.value))
	{
		alert("PSTN must be chars of '0' ~ '9', '*', '#'");
		sipform.CFAll_No.select();
		sipform.CFAll_No.focus();
		return false;
	}

	if (updatePrecedence() == 0)
	{
		alert('You must select one codec at least');
		return false;
	}

	if (sipform.dtmfMode.selectedIndex == 0)
	{
		if (chkPayloadType() == false)
			return false;
	}	

	if (checkDialPlanRegex(sipform.ReplaceRuleSource.value) == false) {
		alert('Replace rule can contains [0-9 x * # +] only.');
		return false;
	}
	
	if (checkDialPlanRegex(sipform.dialplan.value) == false) {
		alert('Dial plan can contains [0-9 x * # +] only.');
		return false;
	}	

	if (checkDialPlanRegex(sipform.PrefixUnsetPlan.value) == false) {
		alert('Prefix unset plan can contains [0-9 x * # +] only.');
		return false;
	}	

	if (checkDialPlanDigits(sipform.ReplaceRuleTarget.value) == false) {
		alert('To be replaced digits can contains [0-9 * #] only.');
		return false;
	}	

	if (checkDialPlanDigits(sipform.AutoPrefix.value) == false) {
		alert('Auto prefix can contains [0-9 * #] only.');
		return false;
	}	

	if (!isNumeric(sipform.dnd_from_hour.value, 0, 23))
	{
		alert('The hour should be between 0~23.');
		sipform.dnd_from_hour.select();
		sipform.dnd_from_hour.focus();
		return false;
	}

	if (!isNumeric(sipform.dnd_from_min.value, 0, 59))
	{
		alert('The minute should be between 0~59.');
		sipform.dnd_from_min.select();
		sipform.dnd_from_min.focus();
		return false;
	}

	if (!isNumeric(sipform.dnd_to_hour.value, 0, 23))
	{
		alert('The hour should be between 0~23.');
		sipform.dnd_to_hour.select();
		sipform.dnd_to_hour.focus();
		return false;
	}

	if (!isNumeric(sipform.dnd_to_min.value, 0, 59))
	{
		alert('The minute should be between 0~59.');
		sipform.dnd_to_min.select();
		sipform.dnd_to_min.focus();
		return false;
	}

	sipform.submit();
}

function changeCountry() 
{ 
	if (document.all.tone_country.value == "13") 
	{ 
		document.all.tonetable.style.display="inline"; 
	} 
	else 
	{ 
		document.all.tonetable.style.display="none"; 
	} 
}

function check_other()
{
	var reg=/\*[0-9]{1}/;

	if (!reg.test(document.other_form.funckey_transfer.value+";"))
	{
		alert('Function key must be 2 chars of the combination of * + 0~9');
		document.other_form.funckey_transfer.select();
		document.other_form.funckey_transfer.focus();
		return false;
	}

	if (!reg.test(document.other_form.funckey_pstn.value+";"))
	{
		alert('Function key must be 2 chars of the combination of * + 0~9');
		document.other_form.funckey_pstn.select();
		document.other_form.funckey_pstn.focus();
		return false;
	}	

	if (document.other_form.auto_dial.value != "0" && 
		!isNumeric(document.other_form.auto_dial.value, 3, 9))
	{
		alert('The auto dial time should be between 3~9.');
		document.other_form.auto_dial.select();
		document.other_form.auto_dial.focus();
		return false;
	}

	if (document.other_form.auto_dial.value == "0" &&
		document.other_form.auto_dial_always.checked)
	{
		alert('The auto dial time should be set if disable dial-out by Hash key.');
		document.other_form.auto_dial.select();
		document.other_form.auto_dial.focus();
		return false;
	}

	if (document.other_form.off_hook_alarm.value != "0" && 
		!isNumeric(document.other_form.off_hook_alarm.value, 10, 60))
	{
		alert('The off-hook alarm time should be between 10~60.');
		document.other_form.off_hook_alarm.select();
		document.other_form.off_hook_alarm.focus();
		return false;
	}


	// VLAN for Voice
        if (!isNumeric(document.other_form.wanVlanIdVoice.value, 1, 4090))
	{
                alert('The VLAN ID should be between 1~4090.');
		document.other_form.wanVlanIdVoice.select();
		document.other_form.wanVlanIdVoice.focus();
		return false;	
	}

	if (!isNumeric(document.other_form.wanVlanPriorityVoice.value, 0, 7))
	{
		alert('The User Priority should be between 0~7.');
		document.other_form.wanVlanPriorityVoice.select();
		document.other_form.wanVlanPriorityVoice.focus();
		return false;	
	}

	if (!isNumeric(document.other_form.wanVlanCfiVoice.value, 0, 1))
	{
		alert('The CFI should be 0 or 1. (0 for Ethernet)');
		document.other_form.wanVlanCfiVoice.select();
		document.other_form.wanVlanCfiVoice.focus();
		return false;	
	}

	// VLAN for Data
        if (!isNumeric(document.other_form.wanVlanIdData.value, 1, 4090))
	{
                alert('The VLAN ID should be between 1~4090.');
		document.other_form.wanVlanIdData.select();
		document.other_form.wanVlanIdData.focus();
		return false;	
	}

	if (!isNumeric(document.other_form.wanVlanPriorityData.value, 0, 7))
	{
		alert('The User Priority should be between 0~7.');
		document.other_form.wanVlanPriorityData.select();
		document.other_form.wanVlanPriorityData.focus();
		return false;	
	}

	if (!isNumeric(document.other_form.wanVlanCfiData.value, 0, 1))
	{
		alert('The CFI should be 0 or 1. (0 for Ethernet)');
		document.other_form.wanVlanCfiData.select();
		document.other_form.wanVlanCfiData.focus();
		return false;	
	}

	// VLAN for Video
        if (!isNumeric(document.other_form.wanVlanIdVideo.value, 1, 4090))
	{
                alert('The VLAN ID should be between 1~4090.');
		document.other_form.wanVlanIdVideo.select();
		document.other_form.wanVlanIdVideo.focus();
		return false;	
	}

	if (!isNumeric(document.other_form.wanVlanPriorityVideo.value, 0, 7))
	{
		alert('The User Priority should be between 0~7.');
		document.other_form.wanVlanPriorityVideo.select();
		document.other_form.wanVlanPriorityVideo.focus();
		return false;	
	}

	if (!isNumeric(document.other_form.wanVlanCfiVideo.value, 0, 1))
	{
		alert('The CFI should be 0 or 1. (0 for Ethernet)');
		document.other_form.wanVlanCfiVideo.select();
		document.other_form.wanVlanCfiVideo.focus();
		return false;	
	}

	return true;
}

function enable_callwaiting()
{
	document.sipform.call_waiting_cid.disabled = 
		!document.sipform.call_waiting.checked;
}

function enable_cid_det_mode()
{
	document.other_form.caller_id_det.disabled = 
		!document.other_form.caller_id_auto_det[0].checked;
}

function enableWanVlan()
{
	var checked = document.other_form.wanVlanEnable.checked;
	document.other_form.wanVlanIdVoice.disabled=!checked;
	document.other_form.wanVlanPriorityVoice.disabled=!checked;
	document.other_form.wanVlanCfiVoice.disabled=!checked;
	document.other_form.wanVlanIdData.disabled=!checked;
	document.other_form.wanVlanPriorityData.disabled=!checked;
	document.other_form.wanVlanCfiData.disabled=!checked;
	document.other_form.wanVlanIdVideo.disabled=!checked;
	document.other_form.wanVlanPriorityVideo.disabled=!checked;
	document.other_form.wanVlanCfiVideo.disabled=!checked;
}

