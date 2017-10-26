#!/bin/bash

if [ "$3" == "" ]; then
  echo "usage: $0 host userId idToken"
else
  (echo '{"id":1, "method":"JSONRPC.SetupRemoteAccess", "params":{"userId": "'$2'", "idToken":"'$3'"}}'; sleep 1) | nc $1 2222
fi
