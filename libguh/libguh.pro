include(../guh.pri)

TARGET = guh
TEMPLATE = lib

CONFIG += c++11

target.path = /usr/lib
INSTALLS += target

SOURCES += plugin/device.cpp \
           plugin/deviceclass.cpp \
           plugin/deviceplugin.cpp \
           plugin/devicedescriptor.cpp \
           devicemanager.cpp \
           hardware/gpio.cpp \
           hardware/radio433.cpp \
           hardware/radio433transmitter.cpp \
           hardware/radio433receiver.cpp \
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
           hardware/radio433.h \
           hardware/radio433transmitter.h \
           hardware/radio433receiver.h \
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

