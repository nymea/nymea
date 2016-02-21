include(../../plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginlircd)

QT += network

SOURCES += \
    devicepluginlircd.cpp \
    lircdclient.cpp \

HEADERS += \
    devicepluginlircd.h \
    lircdclient.h \



