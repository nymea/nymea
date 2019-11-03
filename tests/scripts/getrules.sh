#!/bin/bash

if [ -z $1 ]; then
  echo "usage: $0 host"
  exit 1
fi

cat << EOD | nc $1 2222
{"id":0, "method":"JSONRPC.Hello"}
{"id":1, "method":"Rules.GetRules"}
EOD
