#!/bin/sh

export LD_LIBRARY_PATH=$SNAPP_APP_PATH/usr/lib:$LD_LIBRARY_PATH
$SNAPP_APP_PATH/usr/bin/guhd -n
