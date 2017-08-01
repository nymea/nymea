#!/bin/bash

if [ -z $4 ]; then
  echo "usage: $0 host username password devicename"
else
  (echo '{"id":1, "method": "JSONRPC.Authenticate", "params": { "username": "'$2'", "password": "'$3'", "deviceName": "'$4'"}}'; sleep 1) | nc $1 2222
fi
