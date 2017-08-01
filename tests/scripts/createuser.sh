#!/bin/bash

if [ -z $3 ]; then
  echo "usage: $0 host username password"
else
  (echo '{"id":1, "method": "JSONRPC.CreateUser", "params": { "username": "'$2'", "password": "'$3'"}}'; sleep 1) | nc $1 2222
fi
