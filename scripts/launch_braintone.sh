#!/bin/sh /etc/rc.common
# MindBx startup script

START=99
#STOP=100

start() {

	while ! ls /sys/class/bluetooth/hci0 1> /dev/null 2>&1; do
		echo "waiting for hci0..."
		sleep 1
	done

	sleep 10

	echo "hci0 is present"
	
	hciconfig hci0 sspmode 1

	/intelli/data/data_interface /intelli/data/config/data_config.xml &
	sleep 2
	/intelli/data/data_preprocessing /intelli/data/config/preprocess_config.xml &
	sleep 2
	/intelli/app/braintone_app /intelli/app/config/braintone_app_config.xml &
}

stop() {
        killall data_interface
        killall data_preprocessing
        killall braintone_app
}

boot() {
	
	
	while ! ls /sys/class/bluetooth/hci0 1> /dev/null 2>&1; do
		echo "waiting for hci0..."
		sleep 1
	done

	sleep 10

	echo "hci0 is present"
	
	
	hciconfig hci0 sspmode 1

	/intelli/data/data_interface /intelli/data/config/data_config.xml &
	sleep 2
	/intelli/data/data_preprocessing /intelli/data/config/preprocess_config.xml &
	sleep 2
	/intelli/app/braintone_app /intelli/app/config/braintone_app_config.xml &

}
