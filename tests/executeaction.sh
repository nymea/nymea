#!/bin/bash

if [ -z $1 ]; then
  echo "usage: $0 host deviceId power"
else
  (echo '{"id":1, "method":"Actions.ExecuteAction","params":{"action":{"deviceId":"{'$2'}","params":{"power":"'$3'"}}}}'; sleep 1) | nc $1 1234
fi
