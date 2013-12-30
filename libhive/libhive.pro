TARGET = hive
TEMPLATE = lib

target.path = /usr/lib
INSTALLS += target

SOURCES += device.cpp \
           deviceclass.cpp \
           devicemanager.cpp \
           deviceplugin.cpp \
           radio433.cpp \
           gpio.cpp \
           trigger.cpp \
           triggertype.cpp

HEADERS += device.h \
           deviceclass.h \
           devicemanager.h \
           deviceplugin.h \
           radio433.h \
           gpio.h \
           trigger.h \
           triggertype.h

