#!/bin/bash

if [ -z $4 ]; then
  echo "usage: $0 host idToken authToken cognitoId"
else
  (echo '{"id":1, "method": "JSONRPC.SetupRemoteAccess", "params": { "idToken": "'$2'", "authToken": "'$3'", "cognitoId": "'$4'"}}'; sleep 1) | nc $1 2222
fi
