include(../../plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginphilipshue)

QT += network

SOURCES += \
    devicepluginphilipshue.cpp \
    discovery.cpp \
    huebridgeconnection.cpp \
    light.cpp

HEADERS += \
    devicepluginphilipshue.h \
    discovery.h \
    huebridgeconnection.h \
    light.h \
    lightinterface.h



