#!/bin/bash

if test -z $3; then
  echo "usage: $1 host triggerId actionId"
else
  (echo '{"id":1, "method":"Rules.AddRule", "params":{"triggerId": "'$2'", "actionId": "'$3'" }}'; sleep 1) | nc $1 1234
fi
