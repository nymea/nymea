#!/bin/bash

if [ -z $1 ]; then
  echo "usage: $0 host"
else
  (echo '{"id":1, "method": "JSONRPC.Version"}'; sleep 1) | nc $1 2223
fi
