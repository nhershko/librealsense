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
result="$(lsusb | grep  8086:0ad3)"
	if [ -z "$result" ]
	then
	      echo "waiting for d4xx device"
	else
	      echo "found d4xx device"
		/$server_path/rs-server &
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

echo "start listennning"


while true;
do
check_if_connected
done
