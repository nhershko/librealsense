#!/bin/bash

server_path=$1

if [ "$#" -ne 1 ] || ! [ -d "$1" ]; then
  echo "Usage: $0 SERVER_APPLICATION_PATH" >&2
  exit 1
fi

check_if_connected()
{
while true;
do
result="$(lsusb | grep  8086)"
	if [ -z "$result" ]
	then
	      echo "waiting for d4xx device"
	else
	      echo "found d4xx device"
		#/$server_path/rs-server &
		start_server_app
		break
	fi
done
check_if_disconnected
}

check_if_disconnected() {
while true;
do
echo "check status"
result="$(lsusb | grep  8086:0ad3)"
if [ -z "$result" ]
then
	echo "Device has disconnected"
	pkill rs-server
	break
fi
done
}

start_server_app()
{
tries=0; 
status=1;
server_status=""
echo "starting server"
while true; 
do
server_status="$(ps -A | grep rs-server)"
	let "tries++"
	/$server_path/rs-server &
	echo "try number: " $tries
	sleep 5
	if [ -z "$server_status" ]
	then
	break
	fi
done
echo "server started!"
}

echo "start listennning"


while true;
do
check_if_connected
done
