#!/bin/bash

if test -z $5; then
  echo "usage: $0 host sourceDevice eventTypeId targetDeviceId actionTypeId"
else
  (echo '{"id":1, "method":"Rules.AddRule", "params":{"event": {"eventTypeId": "$3", "deviceId":"'$2'"}, "actions": [ { "deviceId":"'$4'", "actionTypeId":"'$5'", "params":{"power":"true"}}]}}'; sleep 1) | nc $1 1234
#  (echo '{"id":1, "method":"Rules.AddRule", "params":{"event": {"eventTypeId": "'$3'", "deviceId":"'$2'"}, "actions": [ { "deviceId":"'$4'", "actionTypeId":"'$5'", "params":{"power":"true"}}]}}'; sleep 1) | nc $1 1234
#  (echo '{"id":1, "method":"Rules.AddRule", "params":{"event": {"eventTypeId": "'$2'", "deviceId":"'$3'", "params":{"power":"false"}}, "actions": [ { "deviceId":"'$4'", "name":"rule 1", "params":{"power":"false"}},{ "deviceId":"'$5'", "name":"rule 1", "params":{"power":"true"}}]}}'; sleep 1) | nc $1 1234
fi
