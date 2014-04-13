#!/bin/bash

if [ -z $3 ]; then
  echo "usage: $0 host actionTypeId deviceId [paramname paramvalue]"
elif [ -z $4 ]; then
  (echo '{"id":1, "method":"Actions.ExecuteAction","params":{"actionTypeId": "{'$2'}", "deviceId":"{'$3'}"}'; sleep 1) | nc $1 1234
elif [ -z $5 ]; then
  echo "usage: $0 host actionTypeId deviceId [paramname paramvalue]"
else
  (echo '{"id":1, "method":"Actions.ExecuteAction","params":{"actionTypeId": "{'$2'}", "deviceId":"{'$3'}","params":{"'$4'":"'$5'"}}}'; sleep 1) | nc $1 1234
fi
