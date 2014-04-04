#!/bin/bash

if [ -z $2 ]; then
  echo "usage: $0 host deviceId"
else
  (echo '{"id":1, "method":"Devices.RemoveConfiguredDevice", "params":{"deviceId":"'$2'"}}'; sleep 1) | nc $1 1234
fi
