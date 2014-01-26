#!/bin/bash

if [ -z $1 ]; then
  echo "usage $0 host"
else
  (echo '{"id":1, "method":"Devices.GetSupportedDevices"}'; sleep 1) | nc $1 1234
fi
