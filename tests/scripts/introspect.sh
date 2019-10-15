#!/bin/bash

if [ -z $1 ]; then
  echo "usage: $0 host"
else

cat <<EOD | nc $1 2222
{"id":1, "method": "JSONRPC.Hello"}
{"id":2, "method": "JSONRPC.Introspect"}
EOD

fi
