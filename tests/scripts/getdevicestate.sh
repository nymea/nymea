#!/bin/bash

if [ -z $3 ]; then
  echo "usage: $0 host deviceId stateTypeId"
else
  (echo '{"id":1, "method":"Devices.GetDeviceState", "params":{"deviceId":"'$2'", "stateTypeId":"'$3'"}}'; sleep 1) | nc $1 1234
fi
