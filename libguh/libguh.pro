TARGET = guh
TEMPLATE = lib

CONFIG += c++11

target.path = /usr/lib
INSTALLS += target

SOURCES += device.cpp \
           deviceclass.cpp \
           devicemanager.cpp \
           deviceplugin.cpp \
           radio433.cpp \
           gpio.cpp \
           action.cpp \
           actiontype.cpp \
           state.cpp \
           statetype.cpp \
           eventtype.cpp \
           event.cpp

HEADERS += device.h \
           deviceclass.h \
           devicemanager.h \
           deviceplugin.h \
           radio433.h \
           gpio.h \
           action.h \
           actiontype.h \
           state.h \
           statetype.h \
           eventtype.h \
           event.h

