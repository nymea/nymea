#!/bin/bash

(echo '{"id":1, "method":"Devices.GetConfiguredDevices"}'; sleep 1) | nc 10.10.10.114 1234
