#!/bin/bash 
# MindBx startup script

command=$1 

if [ $command == "start" ]; then
    atlants-DATA_interface/data-daemon/data_interface atlants-DATA_interface/data-daemon/config/data_config.xml &
	atlants-DATA_preprocessing/preprocess-daemon/data_preprocessing atlants-DATA_preprocessing/preprocess-daemon/config/preprocess_config.xml &
	atlants-braintone_app/braintone_app atlants-braintone_app/config/braintone_app_config.xml &
else
    killall braintone_app
    killall data_interface
    killall data_preprocessing
fi
