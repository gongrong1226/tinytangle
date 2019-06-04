#!/bin/bash

#to get broadcast ip address
Bcast_addr=$(ifconfig | grep 'Bcast' | cut -d':' -f 3)
Bcast_addr=${Bcast_addr%% *}
echo "The broadcast ip address: $Bcast_addr"

#to get the permission of serial port
usb_path=/dev/ttyUSB0
if [ ! -f "$usb_path" ]; then
	echo "sudo chmod 666 $usb_path"
	sudo chmod 666 $usb_path
else
	echo "There isn't any serial port/USB"
fi

#run
cd build/bin/
./tinytangle $Bcast_addr
