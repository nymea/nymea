include(../plugins.pri)

QT+= network

TARGET = $$qtLibraryTarget(nymea_devicepluginmock)

OTHER_FILES += devicepluginmock.json

SOURCES += \
    devicepluginmock.cpp \
    httpdaemon.cpp

HEADERS += \
    devicepluginmock.h \
    httpdaemon.h
