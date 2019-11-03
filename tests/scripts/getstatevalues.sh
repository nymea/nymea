#!/bin/bash

if [ -z $2 ]; then
  echo "usage: $0 host deviceId"
  exit 1;
fi

cat <<EOD | nc $1 2222 | jq
{"id":0, "method":"JSONRPC.Hello"}
{"id":1, "method":"Devices.GetStateValues", "params":{"deviceId":"$2"}}
EOD
