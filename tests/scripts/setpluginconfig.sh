#!/bin/bash

if [ -z $4 ]; then
  echo "usage $0 host pluginid param1name param1value [param2name param2value]"
elif [ -z $5 ]; then
  (echo '{"id":1, "method":"Devices.SetPluginConfiguration", "params":{"pluginId":"'$2'", "configuration":[{"'$3'":"'$4'"}]}}'; sleep 1) | nc $1 2222
elif [ -z $6 ]; then
  echo "usage $0 host pluginid param1name param1value [param2name param2value]"
else
  (echo '{"id":1, "method":"Devices.SetPluginConfiguration", "params":{"pluginId":"'$2'", "configuration":{[{"'$3'":"'$4'"}, {"'$5'":"'$6'"}]}}}'; sleep 1) | nc $1 2222
fi
