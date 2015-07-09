#!/bin/sh

export QT_PLUGIN_PATH=$SNAPP_APP_PATH/usr/lib/qt5/plugins
export LD_LIBRARY_PATH=$SNAPP_APP_PATH/usr/lib:$LD_LIBRARY_PATH
$SNAPP_APP_PATH/usr/bin/guhd -n -d LogEngine
