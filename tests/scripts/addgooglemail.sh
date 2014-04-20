#!/bin/bash

if [ -z $1 ]; then
  echo "usage: $0 host user password sendTo"
else
  (echo '{"id":1, "method":"Devices.AddConfiguredDevice", "params":{"deviceClassId": "{38ed6ffc-f43b-48f8-aea2-8d63cdcad87e}","deviceParams":{"user":"'$2'", "password":"'$3'", "sendTo":"'$4'"}}}'; sleep 1) | nc $1 1234
fi
