#!/bin/bash

if [ -z $2 ]; then
  echo "usage: $0 host ruleId"
else
  (echo '{"id":1, "method":"Rules.RemoveRule", "params":{"ruleId":"'$2'"}}'; sleep 1) | nc $1 1234
fi
