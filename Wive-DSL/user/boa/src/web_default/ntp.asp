<html>
<! Copyright (c) Realtek Semiconductor Corp., 2004. All Rights Reserved. ->
<head>
<meta http-equiv="Content-Type" content="text/html">
<title>Time Zone Setting</title>
<script type="text/javascript" src="share.js"> </script>
<script>

var ntp_zone_index=4;

function ntp_entry(name, value) { 
	this.name = name ;
	this.value = value ;
} 

var ntp_zone_array=new Array(65);
ntp_zone_array[0]=new ntp_entry("(GMT-12:00)Eniwetok, Kwajalein","+12 1");
ntp_zone_array[1]=new ntp_entry("(GMT-11:00)Midway Island, Samoa","+11 1");
ntp_zone_array[2]=new ntp_entry("(GMT-10:00)Hawaii", "+10 1");
ntp_zone_array[3]=new ntp_entry("(GMT-09:00)Alaska", "+9 1");
ntp_zone_array[4]=new ntp_entry("(GMT-08:00)Pacific Time (US & Canada); Tijuana", "+8 1");
ntp_zone_array[5]=new ntp_entry("(GMT-07:00)Arizona", "+7 1");
ntp_zone_array[6]=new ntp_entry("(GMT-07:00)Mountain Time (US & Canada)", "+7 2");
ntp_zone_array[7]=new ntp_entry("(GMT-06:00)Central Time (US & Canada)", "+6 1");
ntp_zone_array[8]=new ntp_entry("(GMT-06:00)Mexico City, Tegucigalpa", "+6 2");
ntp_zone_array[9]=new ntp_entry("(GMT-06:00)Saskatchewan", "+6 3");
ntp_zone_array[10]=new ntp_entry("(GMT-05:00)Bogota, Lima, Quito", "+5 1");
ntp_zone_array[11]=new ntp_entry("(GMT-05:00)Eastern Time (US & Canada)", "+5 2");
ntp_zone_array[12]=new ntp_entry("(GMT-05:00)Indiana (East)", "+5 3");
ntp_zone_array[13]=new ntp_entry("(GMT-04:00)Atlantic Time (Canada)", "+4 1");
ntp_zone_array[14]=new ntp_entry("(GMT-04:00)Caracas, La Paz", "+4 2");
ntp_zone_array[15]=new ntp_entry("(GMT-04:00)Santiago", "+4 3");
ntp_zone_array[16]=new ntp_entry("(GMT-03:30)Newfoundland", "+3 1");
ntp_zone_array[17]=new ntp_entry("(GMT-03:00)Brasilia", "+3 2");
ntp_zone_array[18]=new ntp_entry("(GMT-03:00)Buenos Aires, Georgetown", "+3 3");
ntp_zone_array[19]=new ntp_entry("(GMT-02:00)Mid-Atlantic", "+2 1");
ntp_zone_array[20]=new ntp_entry("(GMT-01:00)Azores, Cape Verde Is.", "+1 1");
ntp_zone_array[21]=new ntp_entry("(GMT)Casablanca, Monrovia", "+0 1");
ntp_zone_array[22]=new ntp_entry("(GMT)Greenwich Mean Time: Dublin, Edinburgh, Lisbon, London", "+0 2");
ntp_zone_array[23]=new ntp_entry("(GMT+01:00)Amsterdam, Berlin, Bern, Rome, Stockholm, Vienna", "-1 1");
ntp_zone_array[24]=new ntp_entry("(GMT+01:00)Belgrade, Bratislava, Budapest, Ljubljana, Prague", "-1 2");
ntp_zone_array[25]=new ntp_entry("(GMT+01:00)Barcelona, Madrid", "-1 3");
ntp_zone_array[26]=new ntp_entry("(GMT+01:00)Brussels, Copenhagen, Madrid, Paris, Vilnius", "-1 4");
ntp_zone_array[27]=new ntp_entry("(GMT+01:00)Paris", "-1 5");
ntp_zone_array[28]=new ntp_entry("(GMT+01:00)Sarajevo, Skopje, Sofija, Warsaw, Zagreb", "-1 6");
ntp_zone_array[29]=new ntp_entry("(GMT+02:00)Athens, Istanbul, Minsk", "-2 1");
ntp_zone_array[30]=new ntp_entry("(GMT+02:00)Bucharest", "-2 2");
ntp_zone_array[31]=new ntp_entry("(GMT+02:00)Cairo", "-2 3");
ntp_zone_array[32]=new ntp_entry("(GMT+02:00)Harare, Pretoria", "-2 4");
ntp_zone_array[33]=new ntp_entry("(GMT+02:00)Helsinki, Riga, Tallinn", "-2 5");
ntp_zone_array[34]=new ntp_entry("(GMT+02:00)Jerusalem", "-2 6");
ntp_zone_array[35]=new ntp_entry("(GMT+03:00)Baghdad, Kuwait, Riyadh", "-3 1");
ntp_zone_array[36]=new ntp_entry("(GMT+03:00)Moscow, St. Petersburg, Volgograd", "-3 2");
ntp_zone_array[37]=new ntp_entry("(GMT+03:00)Mairobi", "-3 3");
ntp_zone_array[38]=new ntp_entry("(GMT+03:30)Tehran", "-3 4");
ntp_zone_array[39]=new ntp_entry("(GMT+04:00)Abu Dhabi, Muscat", "-4 1");
ntp_zone_array[40]=new ntp_entry("(GMT+04:00)Baku, Tbilisi", "-4 2");
ntp_zone_array[41]=new ntp_entry("(GMT+04:30)Kabul", "-4 3");
ntp_zone_array[42]=new ntp_entry("(GMT+05:00)Ekaterinburg", "-5 1");
ntp_zone_array[43]=new ntp_entry("(GMT+05:00)Islamabad, Karachi, Tashkent", "-5 2");
ntp_zone_array[44]=new ntp_entry("(GMT+05:30)Bombay, Calcutta, Madras, New Delhi", "-5 3");
ntp_zone_array[45]=new ntp_entry("(GMT+06:00)Omsk, Astana, Almaty, Dhaka", "-6 1");
ntp_zone_array[46]=new ntp_entry("(GMT+06:00)Colombo", "-6 2");
ntp_zone_array[47]=new ntp_entry("(GMT+07:00)Bangkok, Hanoi, Jakarta", "-7 1");
ntp_zone_array[48]=new ntp_entry("(GMT+08:00)Beijing, Chongqing, Hong Kong, Urumqi", "-8 1");
ntp_zone_array[49]=new ntp_entry("(GMT+08:00)Perth", "-8 2");
ntp_zone_array[50]=new ntp_entry("(GMT+08:00)Singapore", "-8 3");
ntp_zone_array[51]=new ntp_entry("(GMT+08:00)Taipei", "-8 4");
ntp_zone_array[52]=new ntp_entry("(GMT+09:00)Osaka, Sapporo, Tokyo", "-9 1");
ntp_zone_array[53]=new ntp_entry("(GMT+09:00)Seoul", "-9 2");
ntp_zone_array[54]=new ntp_entry("(GMT+09:00)Yakutsk", "-9 3");
ntp_zone_array[55]=new ntp_entry("(GMT+09:30)Adelaide", "-9 4");
ntp_zone_array[56]=new ntp_entry("(GMT+09:30)Darwin", "-9 5");
ntp_zone_array[57]=new ntp_entry("(GMT+10:00)Brisbane", "-10 1");
ntp_zone_array[58]=new ntp_entry("(GMT+10:00)Canberra, Melbourne, Sydney", "-10 2");
ntp_zone_array[59]=new ntp_entry("(GMT+10:00)Guam, Port Moresby", "-10 3");
ntp_zone_array[60]=new ntp_entry("(GMT+10:00)Hobart", "-10 4");
ntp_zone_array[61]=new ntp_entry("(GMT+10:00)Vladivostok", "-10 5");
ntp_zone_array[62]=new ntp_entry("(GMT+11:00)Magadan, Solomon Is., New Caledonia", "-11 1");
ntp_zone_array[63]=new ntp_entry("(GMT+12:00)Auckland, Wllington", "-12 1");
ntp_zone_array[64]=new ntp_entry("(GMT+12:00)Fiji, Kamchatka, Marshall Is.", "-12 2");

function setTimeZone(field, value){
    field.selectedIndex = 4 ;
    for(i=0 ;i < field.options.length ; i++){
    	if(field.options[i].value == value){
		field.options[i].selected = true;
		break;
	}
    }
}

function setNtpServer(field, ntpServer){
    field.selectedIndex = 0 ;
    for(i=0 ;i < field.options.length ; i++){
    	if(field.options[i].value == ntpServer){
		field.options[i].selected = true;
		break;
	}
    }
}
function updateState_ntp(form)
{
	if(form.enabled.checked){
		enableTextField(form.timeZone);
		enableTextField(form.ntpServerIp1);
		if(form.ntpServerIp2 != null)
			enableTextField(form.ntpServerIp2);
	}
	else{
		disableTextField(form.timeZone);
		disableTextField(form.ntpServerIp1);
		if(form.ntpServerIp2 != null)
			disableTextField(form.ntpServerIp2);
	}
}

function saveChanges_ntp(form)
{
	if(form.ntpServerIp2.value != ""){
//		if ( checkIpAddr(form.ntpServerIp2, 'Invalid IP address') == false )
		if (!checkHostIP(form.ntpServerIp2, 0)) {
			alert('Invalid IP address');
			return false;
		}
		    return false;
	}
	else
		form.ntpServerIp2.value = "0.0.0.0" ;
	return true;
}

function checkEmpty(field){
	if(field.value.length == 0){
		alert(field.name + " field can't be empty\n");
		field.value = field.defaultValue;
		field.focus();
		return false;
	}
	else
		return true;
}
function checkNumber(field){
    str =field.value ;
    for (var i=0; i<str.length; i++) {
    	if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9'))
                        continue;
	field.value = field.defaultValue;
        alert("Invalid " +field.name + " Number. It should be in number (0-9).");
        return false;
    }	
	return true;
}
function checkMonth(str) {
  d = parseInt(str, 10);
  if (d < 0 || d > 12)
      	return false;
  return true;
}
function checkDay(str, month) {
  d = parseInt(str, 10);
  m = parseInt (month, 10);
  if (m == 1 || m == 3 || m == 5 || m == 7 || m == 8 || m == 10 || m == 12) {
  	if (d < 0 || d > 31)
      		return false;
  }
  else if (m == 4 || m == 6 || m == 9 || m == 11) {
  	if (d < 0 || d > 31)
      		return false;
  }
  else if (m == 2) {
  	if (d < 0 || d > 29)
      		return false;
  }
  else
  	return false;
  return true;
}
function checkHour(str) {
  d = parseInt(str, 10);
  if (d < 0 || d >= 24)
      	return false;
  return true;
}
function checkTime(str) {
  d = parseInt(str, 10);
  if (d < 0 || d >= 60)
      	return false;
  return true;
}
function saveChanges(form){
	if((checkEmpty(form.year)& checkEmpty(form.month) & checkEmpty(form.hour)
	 & checkEmpty(form.day) &checkEmpty(form.minute) & checkEmpty(form.second))== false)
	 	return false;

	if((checkNumber(form.year)& checkNumber(form.month) & checkNumber(form.hour)
	 & checkNumber(form.day) &checkNumber(form.minute) & checkNumber(form.second))== false)
	 	return false;
	if(form.month.value == '0'){
		form.month.value = form.month.defaultValue;
        	alert("Invalid month Number. It should be in  number (1-9).");
		return false;
	}
	if (!checkMonth(form.month.value)) {
		alert("Invalid month setting!");	
		form.month.focus();
		return false;
	}
	if (!checkDay(form.day.value, form.month.value)) {
		alert("Invalid day setting!");	
		form.day.focus();
		return false;
	}
	if (!checkHour(form.hour.value)) {
		alert("Invalid hour setting!");	
		form.hour.focus();
		return false;
	}
	if (!checkTime(form.minute.value) || !checkTime(form.second.value)) {
		alert("Invalid time setting!");	
		return false;
	}
	if (form.enabled.checked && form.ntpServerId[1].checked) {
		
//		if (!checkIP(form.ntpServerIp2))	// Jenny,  buglist B066, sync check IP function
		if (form.ntpServerIp2.value == "") {
			alert("Cannot be empty !");
			form.ntpServerIp2.value = form.ntpServerIp2.defaultValue;
			form.ntpServerIp2.focus();
			return false;
		}
/*
		   	if ( validateKey( form.ntpServerIp2.value ) == 0 ) {
			alert("Invalid Manual IP address.");
			form.ntpServerIp2.focus();
			return false;
		}
		if( IsLoopBackIP( form.ntpServerIp2.value)==1 ) {
			alert("Invalid Manual IP address.");
			form.ntpServerIp2.focus();
			return false;
  		}
		for (i=1; i<=4; i++) {
			if ( i == 1 ) {
				if ( !checkDigitRange(form.ntpServerIp2.value,i,0,254) ) {
					alert('Invalid 1st Manual IP address');
					form.ntpServerIp2.focus();
					return false;
				}
			}else {
				if ( !checkDigitRange(document.form.ntpServerIp2.value,i,0,255) ) {
					alert('Invalid 2nd/3rd/4th Manual IP address');
					form.ntpServerIp2.focus();
					return false;
				}
			}
		}
*/
	}	
	return true;
}
function updateState(form)
{
	if(form.enabled.checked){
		enableTextField(form.ntpServerIp1);
		if(form.ntpServerIp2 != null)
			enableTextField(form.ntpServerIp2);
	}
	else{
		disableTextField(form.ntpServerIp1);
		if(form.ntpServerIp2 != null)
			disableTextField(form.ntpServerIp2);
	}
}
</script>
</head>
<body>
<blockquote>
<h2><font color="#0000FF">Time Zone Setting</font></h2>
<table border=0 width="500" cellspacing=0 cellpadding=0>
  <tr><font size=2>
  You can maintain the system time by synchronizing with a public time server over the Internet.
  </tr>
  <tr><hr size=1 noshade align=top></tr>
</table>
<form action=/goform/formNtp method=POST name="time">
<table border="0" width=520>
	<tr>
	<td width ="25%">
	<font size=2> <b> Current Time : </b> </font>
	</td>
	<td width ="75%">
                <font size =2> <b>
                Yr <input type="text" name="year" value="<% getInfo("year"); %>" size="4" maxlength="4">
                Mon <input type="text" name="month" value="<% getInfo("month"); %>" size="2" maxlength="2">
                Day <input type="text" name="day" value="<% getInfo("day"); %>" size="2" maxlength="2">
                Hr <input type="text" name="hour" value="<% getInfo("hour"); %>" size="2" maxlength="2">
                Mn <input type="text" name="minute" value="<% getInfo("minute"); %>" size="2" maxlength="2">
                Sec <input type="text" name="second" value="<% getInfo("second"); %>" size="2" maxlength="2">
                </b> </font>
        </td>
	</tr>
	
	<tr><td width="25%"><font size=2> <b>Time Zone Select : </b> </font></td>
	    <td width="75%">
            <select name="timeZone">
            	<script language="javascript">
            	var i;
            	for(i=0;i<ntp_zone_array.length;i++){
			if (i == ntp_zone_index)
				document.write('<option value="',ntp_zone_array[i].value,'" selected>',ntp_zone_array[i].name,'</option>');
			else
				document.write('<option value="',ntp_zone_array[i].value,'">',ntp_zone_array[i].name,'</option>');
            	}
		</script>
            </select>
	    </td>
	</tr>	
	<tr ><td height=10> </td> </tr>

	<tr><td colspan="2"><font size=2><b>
		<input type="checkbox" name="enabled" 
		value="ON" 
		ONCLICK=updateState(document.time)>&nbsp;&nbsp;Enable SNTP client update </b><br>
	    </td>
	</tr>
	<tr>
	<td width ="25%">
	<font size=2> <b> SNTP server : </b> </font>
	</td>
	<td width ="75%">
		<input type="radio" value="0" name="ntpServerId"></input>
		<select name="ntpServerIp1">
			<option value="94.26.135.117">94.26.135.117    - Russian Federation</option>
			<option value="188.128.19.66">188.128.19.66    - Russian Federation</option>
			<option value="192.5.41.41">192.5.41.41    - North America</option>
			<option value="192.5.41.209">192.5.41.209   - North America</option>
			<option value="130.149.17.8">130.149.17.8   - Europe</option>
			<option value="203.117.180.36"> 203.117.180.36 - Asia Pacific</option>
			</select>
		</td>
	</tr>
	<td width ="25%"> <font size=2><b> </b></font>
	</td>
	<td width ="75%">
	<input type="radio" value="1" name="ntpServerId"></input>
	<!--
	<input type="text" name="ntpServerIp2" size="15" maxlength="15" value=<% getInfo("ntpServerIp2"); %>> <font size=2> (Manual IP Setting) </font>
	!-->
	<!--ping_zhang:20081217 START:patch from telefonica branch to support WT-107-->
	<input type="text" name="ntpServerIp2" size="15" maxlength="30" value=<% getInfo("ntpServerHost2"); %>> <font size=2> (IP/NAME Setting) </font>
		</td>
	</tr>
</table>
  <input type="hidden" value="/ntp.asp" name="submit-url">
  <p><input type="submit" value="Apply Change" name="save" onClick="return saveChanges(document.time)">
&nbsp;&nbsp;
  <input type="button" value="Refresh" name="refresh" onClick="javascript: window.location.reload()">
</form>
<script>
		<% initPage("ntp"); %>
		setTimeZone(document.time.timeZone, "<% getInfo("ntpTimeZone"); %>");
		setNtpServer(document.time.ntpServerIp1, "<% getInfo("ntpServerIp1"); %>");	
		updateState(document.time);
</script>
</blockquote>
</font>
</body>

</html>
