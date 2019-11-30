#!/bin/bash

if [ -z $1 ]; then
  echo "usage $0 host [vendorId]"
  exit 1;
fi

if [ -z $2 ]; then
cat << EOD | nc $1 2222
{"id":0, "method": "JSONRPC.Hello"}
{"id":1, "method": "Devices.GetSupportedDevices"}
EOD
else
cat << EOD | nc $1 2222
{"id":0, "method": "JSONRPC.Hello"}
{"id":1, "method": "Devices.GetSupportedDevices", "params":{"vendorId":"$2"}}
EOD
fi



