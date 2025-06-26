#!/bin/bash

if [ -z $1 ]; then
  echo "usage $0 host"
  exit 1
fi

cat << EOD | nc $1 2223
{"id":0, "method": "JSONRPC.Hello"}
{"id":1, "method": "Integrations.GetPlugins"}
EOD
