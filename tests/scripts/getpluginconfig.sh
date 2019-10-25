#!/bin/bash

if [ -z $2 ]; then
  echo "usage $0 host pluginid"
  exit 1
fi

cat << EOD | nc $1 2222
{"id":0, "method": "JSONRPC.Hello"}
{"id":1, "method":"Devices.GetPluginConfiguration", "params":{"pluginId":"$2"}}
EOD

