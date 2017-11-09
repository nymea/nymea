#!/bin/bash

if [ -z $2 ]; then
  echo "usage: $0 host devicename"
else
  (echo '{"id":1, "method": "JSONRPC.RequestPushButtonAuth", "params": { "deviceName": "'$2'"}}'; sleep 1) | nc $1 2222
fi
