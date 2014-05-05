#!/bin/bash

if [ -z $2 ]; then
  echo "usage: $0 host actionTypeId"
else
  (echo '{"id":1, "method":"Actions.GetActionType", "params":{"actionTypeId":"'$2'"}}'; sleep 1) | nc $1 1234
fi
