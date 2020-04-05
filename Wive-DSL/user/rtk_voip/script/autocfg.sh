#!/bin/sh
# script file for auto configuration

GET="flash get"
VOIP_GET="flash voip get"

CFG_MODE=`$VOIP_GET VOIP.AUTO_CFG_MODE | cut -d= -f2`
if [ "$CFG_MODE" = "0" ]; then
	# disable auto config
	exit 0
fi

FILE_PATH=`$VOIP_GET VOIP.AUTO_CFG_FILE_PATH | cut -d= -f2`
CFG_EXPIRE=`$VOIP_GET VOIP.AUTO_CFG_EXPIRE | cut -d= -f2`

if [ "$CFG_MODE" = "1" ]; then
	# HTTP Mode
	FILENAME=`$GET HW_NIC0_ADDR | cut -d= -f2`".dat"
	HTTP_ADDR=`$VOIP_GET VOIP.AUTO_CFG_HTTP_ADDR | cut -d= -f2`
	HTTP_PORT=`$VOIP_GET VOIP.AUTO_CFG_HTTP_PORT | cut -d= -f2`
	HTTP_URL="http://$HTTP_ADDR:$HTTP_PORT/$FILE_PATH/$FILENAME"
	TMPCFG="/tmp/voip_flash.dat"
	echo "Start Auto Config daemon"
	# start auto config
	while [ true ]; do
		# wget
		wget $HTTP_URL -O $TMPCFG
		if [ $? != 0 ]; then
			echo "=> auto config error: wget $HTTP_URL failed"
			sleep 10
			continue
		fi
		# import
		flash voip -in $TMPCFG
		case $? in
			0)  # import ok
				SLEEP_TIME=`expr $CFG_EXPIRE \* 3600 \* 24`
				RESTART=1
				;;
			1)	# the same version
				SLEEP_TIME=`expr $CFG_EXPIRE \* 3600 \* 24`
				;;
			*)	# import failed:
				echo "=> auto config error: import $TMPCFG error"
				SLEEP_TIME=300
				;;
		esac

		if [ "$RESTART" = "1" ]; then
			# import ok, restart webs & solar
			killall webs
			webs &
			solar &
		fi

		echo "=> auto config info: completed"

		CFG_EXPIRE=`$VOIP_GET VOIP.AUTO_CFG_EXPIRE | cut -d= -f2`
		if [ "$CFG_EXPIRE" = "0" ]; then
			# run once only
			break
		fi

		sleep $SLEEP_TIME
	done
fi

echo "=> auto config info: exit"
