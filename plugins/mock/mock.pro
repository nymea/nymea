include(../plugins.pri)

QT += network

TARGET = $$qtLibraryTarget(nymea_integrationpluginmock)

OTHER_FILES += integrationpluginmock.json

SOURCES += \
    integrationpluginmock.cpp \
    httpdaemon.cpp

HEADERS += \
    integrationpluginmock.h \
    httpdaemon.h
