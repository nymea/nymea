#!/bin/bash

if [ "$2" == "" ]; then
  echo "usage: $0 host <true|false>"
else
  (echo '{"id":1, "method":"Configuration.SetCloudEnabled", "params":{"enabled": "'$2'"}}'; sleep 10) | nc $1 2222
fi
