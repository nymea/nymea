#!/bin/bash

if [ -z $4 ]; then
  echo "usage: $0 host actionTypeId deviceId power"
else
  (echo '{"id":1, "method":"Actions.ExecuteAction","params":{"actionTypeId": "{'$2'}", "deviceId":"{'$3'}","params":{"power":"'$4'"}}}'; sleep 1) | nc $1 1234
fi
