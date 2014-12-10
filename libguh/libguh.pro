include(../guh.pri)

TARGET = guh
TEMPLATE = lib

QT += network

target.path = /usr/lib
INSTALLS += target

SOURCES += plugin/device.cpp \
           plugin/deviceclass.cpp \
           plugin/deviceplugin.cpp \
           plugin/devicedescriptor.cpp \
           devicemanager.cpp \
           hardware/gpio.cpp \
           hardware/gpiomonitor.cpp \
           hardware/radio433/radio433.cpp \
           hardware/radio433/radio433transmitter.cpp \
           hardware/radio433/radio433receiver.cpp \
           hardware/radio433/radio433brennenstuhlgateway.cpp \
           hardware/upnpdiscovery/upnpdiscovery.cpp \
           hardware/upnpdiscovery/upnpdevice.cpp \
           hardware/upnpdiscovery/upnpdevicedescriptor.cpp
           types/action.cpp \
           types/actiontype.cpp \
           types/state.cpp \
           types/statetype.cpp \
           types/eventtype.cpp \
           types/event.cpp \
           types/eventdescriptor.cpp \
           types/vendor.cpp \
           types/paramtype.cpp \
           types/param.cpp \
           types/paramdescriptor.cpp \
           types/statedescriptor.cpp \


HEADERS += plugin/device.h \
           plugin/deviceclass.h \
           plugin/deviceplugin.h \
           plugin/devicedescriptor.h \
           devicemanager.h \
           hardware/gpio.h \
           hardware/gpiomonitor.h \
           hardware/radio433/radio433.h \
           hardware/radio433/radio433transmitter.h \
           hardware/radio433/radio433receiver.h \
           hardware/radio433/radio433brennenstuhlgateway.h \
           hardware/upnpdiscovery/upnpdiscovery.h \
           hardware/upnpdiscovery/upnpdevice.h \
           hardware/upnpdiscovery/upnpdevicedescriptor.h
           types/action.h \
           types/actiontype.h \
           types/state.h \
           types/statetype.h \
           types/eventtype.h \
           types/event.h \
           types/eventdescriptor.h \
           types/vendor.h \
           types/typeutils.h \
           types/paramtype.h \
           types/param.h \
           types/paramdescriptor.h \
           types/statedescriptor.h \
           typeutils.h \
