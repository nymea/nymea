include(../plugins.pri)

QT+= network

TARGET = $$qtLibraryTarget(nymea_devicepluginmock)

SOURCES += \
    devicepluginmock.cpp \
    httpdaemon.cpp

HEADERS += \
    devicepluginmock.h \
    httpdaemon.h
