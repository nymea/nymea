#!/bin/bash

if [ -z $1 ]; then
  echo "usage: $0 host deviceId subject body"
else
  (echo '{"id":1, "method":"Actions.ExecuteAction","params":{"actionTypeId": "{fa54f834-34d0-4aaf-b0ab-a165191d39d3}", "deviceId":"{'$2'}","params":{"subject":"'$3'", "body":"'$4'"}}}'; sleep 1) | nc $1 1234
fi
