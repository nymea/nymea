#!/bin/sh

export QT_PLUGIN_PATH=/apps/guh.sideload/current/usr/lib/qt5/plugins
export LD_LIBRARY_PATH=/apps/guh.sideload/current/usr/lib:$LD_LIBRARY_PATH
/apps/guh.sideload/current/usr/bin/guhd -n
