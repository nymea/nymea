#!/bin/bash

if [ -z $2 ]; then
  echo "usage $0 host pluginid"
else
  (echo '{"id":1, "method":"Devices.GetPluginConfiguration", "params":{"pluginId":"'$2'"}}'; sleep 1) | nc $1 1234
fi
