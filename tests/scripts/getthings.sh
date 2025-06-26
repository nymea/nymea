#!/bin/bash

if [ -z $1 ]; then
  echo "usage: $0 host <token>"
  exit 1
fi

if [ -z $2 ]; then
cat <<EOD | nc $1 2223
{"id":0, "method":"JSONRPC.Hello"}
{"id":1, "method":"Integrations.GetThings"}
EOD
exit 0
fi

cat <<EOD | nc $1 2223
{"id":0, "method":"JSONRPC.Hello"}
{"id":1, "token": "$2", "method":"Integrations.GetThings"}
EOD


