#!/bin/bash

(echo '{"id":1, "method":"Devices.GetSupportedDevices"}'; sleep 1) | nc 10.10.10.114 1234
