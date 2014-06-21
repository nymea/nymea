#!/bin/bash

if [ -z $3 ]; then
  echo "usage: $0 host justadd|discovery deviceClassId [deviceDescriptorId]"
elif [ $2 == "justadd" ]; then
  (echo '{"id":1, "method":"Devices.PairDevice", "params":{"deviceClassId":"'$3'"}}'; sleep 1) | nc $1 1234
elif [ $2 == "discovery" ]; then
  (echo '{"id":1, "method":"Devices.PairDevice", "params":{"deviceClassId":"'$3'", "deviceDescriptorId":"'$4'"}}'; sleep 1) | nc $1 1234
else
  echo "usage: $0 host justadd|discovery deviceClassId [deviceDescriptorId]"
fi
