#!/bin/bash

if [ -z $2 ]; then
  echo "usage $0 host deviceclassid [paramname paramvalue]"
elif [ -z $3 ]; then
  (echo '{"id":1, "method":"Devices.GetDiscoveredDevices", "params":{"deviceClassId":"'$2'"}}'; sleep 6) | nc $1 1234
elif [ -z $4 ]; then
  echo "usage $0 host deviceclassid [paramname paramvalue]"
else
  (echo '{"id":1, "method":"Devices.GetDiscoveredDevices", "params":{"deviceClassId":"'$2'", "discoveryParams": {"'$3'":"'$4'"}}}'; sleep 6) | nc $1 1234
fi
