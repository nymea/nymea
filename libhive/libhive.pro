TARGET = hive
TEMPLATE = lib

target.path = /usr/lib
INSTALLS += target

SOURCES += device.cpp \
           deviceclass.cpp \
           devicemanager.cpp \
           deviceplugin.cpp \
           radio433.cpp \
           gpio.cpp

HEADERS += device.h \
           deviceclass.h \
           devicemanager.h \
           radio433.h \
           gpio.h

