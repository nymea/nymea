#!/bin/bash

if test -z $3; then
  echo "usage: $1 host triggerId actionId"
else
  (echo '{"id":1, "method":"Rules.AddRule", "params":{"triggerTypeId": "'$2'", "action":{ "deviceId":"'$3'", "name":"rule 1", "actionParams":{"power":"true"}}}}'; sleep 1) | nc $1 1234
  (echo '{"id":2, "method":"Rules.AddRule", "params":{"triggerTypeId": "'$2'", "action":{ "deviceId":"'$3'", "name":"rule 2", "actionParams":{"power":"false"}}}}'; sleep 1) | nc $1 1234
fi
