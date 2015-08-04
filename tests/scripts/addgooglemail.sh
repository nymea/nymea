#!/bin/bash

if [ -z $2 ]; then
  echo "usage: $0 host user password sendTo"
else
  (echo '{"id":1, "method":"Devices.AddConfiguredDevice", "params":{"deviceClassId": "{3869884a-1592-4b8f-84a7-994be18ff555}","deviceParams":{"user":"'$2'", "password":"'$3'", "recipient":"'$4'"}}}'; sleep 1) | nc $1 2222
fi
