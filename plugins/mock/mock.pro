TRANSLATIONS = translations/en_US.ts \
               translations/de_DE.ts

# Note: include after the TRANSLATIONS definition
include(../plugins.pri)

QT+= network

TARGET = $$qtLibraryTarget(guh_devicepluginmock)

SOURCES += \
    devicepluginmock.cpp \
    httpdaemon.cpp

HEADERS += \
    devicepluginmock.h \
    httpdaemon.h
