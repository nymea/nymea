#!/bin/bash

if [ -z $2 ]; then
  echo "usage: $0 host deviceClassId"
  exit 1
fi

cat <<EOD | nc $1 2222
{"id":0, "method":"JSONRPC.Hello"}
{"id":1, "method":"Devices.GetEventTypes", "params":{"deviceClassId":"$2"}}
EOD
