#!/bin/bash

if [ -z $2 ]; then
  echo "usage: $0 host ruleId"
else
  (echo '{"id":1, "method":"Rules.GetRuleDetails", "params": {"ruleId": "'$2'"}}'; sleep 1) | nc $1 2222
fi
