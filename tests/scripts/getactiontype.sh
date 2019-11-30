#!/bin/bash

if [ -z $2 ]; then
  echo "usage: $0 host actionTypeId"
  exit 1;
fi

cat << EOD | nc $1 2222
{"id":0, "method":"JSONRPC.Hello"}
{"id":1, "method":"Actions.GetActionType", "params":{"actionTypeId":"$2"}}
EOD
