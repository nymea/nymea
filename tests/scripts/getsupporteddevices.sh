#!/bin/bash

if [ -z $1 ]; then
  echo "usage $0 host"
elif [ -z $2 ]; then
  (echo '{"id":1, "method":"Devices.GetSupportedDevices"}'; sleep 1) | nc $1 2222
else
  (echo '{"id":1, "method":"Devices.GetSupportedDevices", "params":{"vendorId":"'$2'"}}'; sleep 1) | nc $1 2222
fi
