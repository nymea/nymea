#!/bin/bash

(echo '{"id":1, "method":"Devices.GetSupportedDevices"}'; sleep 1) | nc localhost 1234
