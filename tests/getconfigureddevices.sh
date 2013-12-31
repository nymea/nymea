#!/bin/bash

(echo '{"id":1, "method":"Devices.GetConfiguredDevices"}'; sleep 1) | nc localhost 1234
