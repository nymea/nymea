#!/bin/bash

if [ -z $2 ]; then
  echo "usage: $0 host ruleId"
  exit 1
fi


cat << EOD | nc $1 2222 | jq
{"id":0, "method":"JSONRPC.Hello"}
{"id":1, "method":"Rules.GetRuleDetails", "params": {"ruleId": "$2"}}
EOD
