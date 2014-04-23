include(../../plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginwakeonlan)

QT += network

SOURCES += \
    devicepluginwakeonlan.cpp

HEADERS += \
    devicepluginwakeonlan.h


