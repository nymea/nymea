#!/bin/bash

if [ -z $3 ]; then
  echo "usage: $0 host deviceId stateTypeId"
  exit 1;
fi

cat <<EOD | nc $1 2222
{"id":0, "method":"JSONRPC.Hello"}
{"id":1, "method":"Devices.GetStateValue", "params":{"deviceId":"$2", "stateTypeId":"$3"}}
EOD
