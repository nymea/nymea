#!/bin/bash

if test -z $5; then
  echo "usage: $0 host sourceDevice eventTypeId targetDeviceId actionTypeId [paramname paramvalue]"
elif test -z $6; then
  (echo '{"id":1, "method":"Rules.AddRule", "params":{"event": {"eventTypeId": "$3", "deviceId":"'$2'"}, "actions": [ { "deviceId":"'$4'", "actionTypeId":"'$5'"}]}}'; sleep 1) | nc $1 1234
elif test -z $7; then
  echo "usage: $0 host sourceDevice eventTypeId targetDeviceId actionTypeId [paramname paramvalue]"
else
  (echo '{"id":1, "method":"Rules.AddRule", "params":{"event": {"eventTypeId": "'$3'", "deviceId":"'$2'"}, "actions": [ { "deviceId":"'$4'", "actionTypeId":"'$5'", "params":{"'$6'":"'$7'"}}]}}'; sleep 1) | nc $1 1234
#  (echo '{"id":1, "method":"Rules.AddRule", "params":{"event": {"eventTypeId": "'$2'", "deviceId":"'$3'", "params":{"power":"false"}}, "actions": [ { "deviceId":"'$4'", "name":"rule 1", "params":{"power":"false"}},{ "deviceId":"'$5'", "name":"rule 1", "params":{"power":"true"}}]}}'; sleep 1) | nc $1 1234
fi
