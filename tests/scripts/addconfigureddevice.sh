#!/bin/bash

# Creates a Mumbi remote
if [ -z $1 ]; then
  echo "usage $0 host device [deviceClassId descriptorId]. In case of a discovered device, deviceClassId and descriptorId are mandatory."
elif [ $1 == "list" ]; then
  echo "elroremote elroswitch intertechnoremote  wifidetector mock1 mock2 openweathermap"
elif [ -z $2 ]; then
  echo "usage $0 host device [deviceClassId descriptorId]. In case of a discovered device, deviceClassId and descriptorId are mandatory."
else
  if [ $2 == "elroremote" ]; then
    # Adds an ELRO remote control on channel 00000
    (echo '{"id":1, "method":"Devices.AddConfiguredDevice", "params":{"deviceClassId": "{d85c1ef4-197c-4053-8e40-707aa671d302}","deviceParams":{"channel1":"false", "channel2":"false", "channel3":"false", "channel4": "false", "channel5":"false" }}}'; sleep 1) | nc $1 2222
  elif [ $2 == "elroswitch" ]; then
    # Adds a ELRO power switch on channel 00000 and group D
    (echo '{"id":1, "method":"Devices.AddConfiguredDevice", "params":{"deviceClassId": "{308ae6e6-38b3-4b3a-a513-3199da2764f8}","deviceParams":{"channel1":"false","channel2":"false", "channel3":"false", "channel4": "false","channel5":"false","A":"true","B":"false","C":"false","D":"false","E":"false" }}}'; sleep 1) | nc $1 2222
  elif [ $2 == "intertechnoremote" ]; then
    # Adds an intertechno remote control
    (echo '{"id":1, "method":"Devices.AddConfiguredDevice", "params":{"deviceClassId": "{ab73ad2f-6594-45a3-9063-8f72d365c5e5}","deviceParams":{"familyCode":"J"}}}'; sleep 1) | nc $1 2222
#  elif [ $2 == "meisteranker" ]; then
#    # Adds an intertechno remote control
#    (echo '{"id":1, "method":"Devices.AddConfiguredDevice", "params":{"deviceClassId": "{e37e9f34-95b9-4a22-ae4f-e8b874eec871}","deviceParams":{"id":"1"}}}'; sleep 1) | nc $1 2222
  elif [ $2 == "wifidetector" ]; then
    # Adds a WiFi detector
    (echo '{"id":1, "method":"Devices.AddConfiguredDevice", "params":{"deviceClassId": "{bd216356-f1ec-4324-9785-6982d2174e17}","deviceParams":{"mac":"90:cf:15:1b:ce:bb"}}}'; sleep 1) | nc $1 2222
  elif [ $2 == "mock1" ]; then
    # Adds a Mock device
    (echo '{"id":1, "method":"Devices.AddConfiguredDevice", "params":{"deviceClassId": "{753f0d32-0468-4d08-82ed-1964aab03298}","deviceParams":{"httpport":"8081"}}}'; sleep 1) | nc $1 2222
  elif [ $2 == "mock2" ]; then
    # Adds a Mock device
    (echo '{"id":1, "method":"Devices.AddConfiguredDevice", "params":{"deviceClassId": "{753f0d32-0468-4d08-82ed-1964aab03298}","deviceParams":{"httpport":"8082"}}}'; sleep 1) | nc $1 2222
  elif [ $2 == "openweathermap" ]; then
      (echo '{"id":1, "method":"Devices.AddConfiguredDevice", "params":{"deviceClassId": "{985195aa-17ad-4530-88a4-cdd753d747d7}","deviceDescriptorId":"'$4'"}}'; sleep 1) | nc $1 2222
  elif [ $2 == "wol" ]; then
      (echo '{"id":1, "method":"Devices.AddConfiguredDevice", "params":{"deviceClassId": "{3c8f2447-dcd0-4882-8c09-99e579e4d24c}","deviceParams":{"mac":"'$3'", "name":"Wohnzimmer-PC"}}}'; sleep 1) | nc $1 2222
  elif [ $2 == "lirc" ]; then
    (echo '{"id":1, "method":"Devices.AddConfiguredDevice", "params":{"deviceClassId": "{5c2bc4cd-ba6c-4052-b6cd-1db83323ea22}","deviceParams":{"remoteName":"'$3'"}}}'; sleep 1) | nc $1 2222
  elif [ $2 == "discovered" ]; then
    if [ -z $4]; then
      echo "usage $0 host device [deviceClassId descriptorId]. In case of a discovered device, deviceClassId and descriptorId are mandatory."
    else
      (echo '{"id":1, "method":"Devices.AddConfiguredDevice", "params":{"deviceClassId": "'$3'", "deviceDescriptorId": "'$4'"}}'; sleep 1) | nc $1 2222
    fi
  else
    echo "unknown type $2. Possible values are: elroremote, elroswitch, intertechnoremote, wifidetector, mock1, mock2, openweathermap, lirc, discovered. (In case of discovered, a deviceDescriptorId is required)"
  fi
fi
