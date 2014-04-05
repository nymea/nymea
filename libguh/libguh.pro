TARGET = guh
TEMPLATE = lib

CONFIG += c++11

target.path = /usr/lib
INSTALLS += target

SOURCES += plugin/device.cpp \
           plugin/deviceclass.cpp \
           plugin/devicemanager.cpp \
           plugin/deviceplugin.cpp \
           hardware/radio433.cpp \
           hardware/gpio.cpp \
           types/action.cpp \
           types/actiontype.cpp \
           types/state.cpp \
           types/statetype.cpp \
           types/eventtype.cpp \
           types/event.cpp \
            types/vendor.cpp

HEADERS += plugin/device.h \
           plugin/deviceclass.h \
           plugin/devicemanager.h \
           plugin/deviceplugin.h \
           hardware/radio433.h \
           hardware/gpio.h \
           types/action.h \
           types/actiontype.h \
           types/state.h \
           types/statetype.h \
           types/eventtype.h \
           types/event.h \
            types/vendor.h

