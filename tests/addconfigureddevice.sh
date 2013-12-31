#!/bin/bash

# Creates a Mumbi remote
(echo '{"id":1, "method":"Devices.AddConfiguredDevice", "params":{"deviceClass": "{d85c1ef4-197c-4053-8e40-707aa671d302}", "deviceParams":{"channel1":"false", "channel2":"false", "channel3":"false", "channel4": "false", "channel5":"false" }}}'; sleep 1) | nc 10.10.10.114 1234
