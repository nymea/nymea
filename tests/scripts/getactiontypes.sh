#!/bin/bash

if [ -z $2 ]; then
  echo "usage: $0 host deviceClassId"
  exit 1
fi


cat <<EOD | nc $1 2222
{"id":2, "method":"JSONRPC.Hello"}
{"id":1, "method":"Devices.GetActionTypes", "params":{"deviceClassId":"$2"}}
EOD
