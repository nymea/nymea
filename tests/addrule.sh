#!/bin/bash

if test -z $3; then
  echo "usage: $1 host triggerTypeId sourceDeviceId targetDeviceId"
else
  (echo '{"id":1, "method":"Rules.AddRule", "params":{"trigger": {"triggerTypeId": "'$2'", "deviceId":"'$3'", "params":{"power":"true"}}, "actions": [ { "deviceId":"'$4'", "name":"rule 1", "params":{"power":"true"}}, { "deviceId":"'$5'", "name":"rule 1", "params":{"power":"true"}}]}}'; sleep 1) | nc $1 1234
  (echo '{"id":1, "method":"Rules.AddRule", "params":{"trigger": {"triggerTypeId": "'$2'", "deviceId":"'$3'", "params":{"power":"false"}}, "actions": [ { "deviceId":"'$4'", "name":"rule 1", "params":{"power":"false"}},{ "deviceId":"'$5'", "name":"rule 1", "params":{"power":"true"}}]}}'; sleep 1) | nc $1 1234
fi
