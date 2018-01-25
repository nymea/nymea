include(../plugins.pri)

QT+= network

TARGET = $$qtLibraryTarget(guh_devicepluginmock)

SOURCES += \
    devicepluginmock.cpp \
    httpdaemon.cpp

HEADERS += \
    devicepluginmock.h \
    httpdaemon.h
