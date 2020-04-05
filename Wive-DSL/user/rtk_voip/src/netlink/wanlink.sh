#!/bin/sh
#
# script file for wan link
# Usage: wanlink.sh {connect | disconnect}
#

TOOL=flash
GETMIB="$TOOL get"
SET_IP=fixedip.sh
START_DHCP_CLIENT=dhcpc.sh
START_PPPOE=pppoe.sh
START_FIREWALL=firewall.sh
START_PPTP=pptp.sh

if [ $1 = '' ]; then
	echo "Usage: wanlink.sh {connect | disconnect}"
	exit
elif [ $1 != 'connect' ] && [ $1 != 'disconnect' ]; then
	echo "Usage: wanlink.sh {connect | disconnect}"
	exit
fi

eval `$GETMIB OP_MODE`
eval `$GETMIB WAN_DHCP`
WAN_INTERFACE=eth1
if [ $OP_MODE = '0' ]; then
if [ $1 = 'connect' ]; then
	echo WAN Port Connect
	if [ "$WAN_DHCP" = '1' ]; then
		$START_DHCP_CLIENT $WAN_INTERFACE wait&
	elif [ "$WAN_DHCP" = '3' ]; then
		echo 'start PPPoE daemon'
		# rock: support dual access in WAN
		eval `$GETMIB PPPOE_WAN_PHY_IP_MODE`
		if [ $PPPOE_WAN_PHY_IP_MODE = 0 ]; then
			eval `$GETMIB DHCP_MTU_SIZE`
			ifconfig $WAN_INTERFACE mtu $DHCP_MTU_SIZE
			$START_DHCP_CLIENT $WAN_INTERFACE wait &
		else
			eval `$GETMIB PPPOE_WAN_PHY_IP`
				eval `$GETMIB PPPOE_WAN_PHY_NETMASK`
	    		ifconfig $WAN_INTERFACE addr $PPPOE_WAN_PHY_IP netmask $PPPOE_WAN_PHY_NETMASK
		fi
		$START_PPPOE all $WAN_INTERFACE
	elif [ "$WAN_DHCP" = '4' ]; then
		echo 'start PPTP daemon'
		# rock: support dual access in WAN
		eval `$GETMIB PPTP_IP_MODE`
		if [ $PPTP_IP_MODE = 0 ]; then
			$START_DHCP_CLIENT $WAN_INTERFACE wait &
		fi
		$START_PPTP $WAN_INTERFACE &
	fi
else
	echo WAN Port Disconnect
	if [ "$WAN_DHCP" = '1' ]; then
		DHCP_PID=`cat /etc/udhcpc/udhcpc-eth1.pid`
		kill -9 $DHCP_PID
		ifconfig $WAN_INTERFACE 0.0.0.0
	elif [ "$WAN_DHCP" = '3' ]; then
		eval `$GETMIB PPPOE_WAN_PHY_IP_MODE`
		if [ $PPPOE_WAN_PHY_IP_MODE = 0 ]; then
			DHCP_PID=`cat /etc/udhcpc/udhcpc-eth1.pid`
			kill -9 $DHCP_PID
		fi
		killall -9 $START_PPPOE
		ifconfig $WAN_INTERFACE 0.0.0.0
	elif [ "$WAN_DHCP" = '4' ]; then
		eval `$GETMIB PPTP_IP_MODE`
		if [ $PPTP_IP_MODE = 0 ]; then
			DHCP_PID=`cat /etc/udhcpc/udhcpc-eth1.pid`
			kill -9 $DHCP_PID
		fi
		killall -9 $START_PPTP
		killall -9 pppd
		ifconfig $WAN_INTERFACE 0.0.0.0
	fi
fi
fi
