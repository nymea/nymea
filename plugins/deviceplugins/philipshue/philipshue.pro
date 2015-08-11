include(../../plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginphilipshue)

QT += network

SOURCES += \
    devicepluginphilipshue.cpp \
    #huebridgeconnection.cpp \
    #light.cpp \
    huebridge.cpp \
    huelight.cpp

HEADERS += \
    devicepluginphilipshue.h \
    #huebridgeconnection.h \
    #light.h \
    #lightinterface.h \
    huebridge.h \
    huelight.h



