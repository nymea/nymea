#!/bin/bash

# Creates a Mumbi remote
if [ -z $1 ]; then
  echo "usage $0 host"
else
  (echo '{"id":1, "method":"Devices.AddConfiguredDevice", "params":{"deviceClass": "{d85c1ef4-197c-4053-8e40-707aa671d302}","deviceParams":{"channel1":"false", "channel2":"false", "channel3":"false", "channel4": "false", "channel5":"false" }}}'; sleep 1) | nc $1 1234
#  (echo '{"id":1, "method":"Devices.AddConfiguredDevice", "params":{"deviceClass": "{308ae6e6-38b3-4b3a-a513-3199da2764f8}","deviceParams":{"channel1":"false","channel2":"false", "channel3":"false", "channel4": "false","channel5":"false","A":"false","B":"true","C":"false","D":"false","E":"false" }}}'; sleep 1) | nc $1 1234
fi
