#!/bin/bash

if [ -z $1 ]; then
  echo "usage: $0 host"
else
  (echo '{"id":1, "token": "'$2'", "method":"Devices.GetConfiguredDevices"}'; sleep 1) | nc $1 2222
fi
