#!/bin/bash

if [ -z $5 ]; then
  echo "usage: $0 host deviceId appId tagId value"
else
  (echo '{"id":1, "method":"Tags.AddTag", "params": { "tag": {"deviceId": "'$2'", "appId": "'$3'", "tagId": "'$4'", "value":"'$5'"}}}'; sleep 1) | nc $1 2222
fi
