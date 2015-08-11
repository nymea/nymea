#!/bin/bash

if [ -z $1 ]; then
  echo "usage: $0 host deviceId subject body"
else
  (echo '{"id":1, "method":"Actions.ExecuteAction","params":{"actionTypeId": "{054613b0-3666-4dad-9252-e0ebca187edc}", "deviceId":"{'$2'}","params":[{"subject":"'$3'"}, {"body":"'$4'"}]}}'; sleep 1) | nc $1 2222
fi
