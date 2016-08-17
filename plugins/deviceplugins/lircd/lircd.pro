TRANSLATIONS = translations/en_US.ts \
               translations/de_DE.ts

# Note: include after the TRANSLATIONS definition
include(../../plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginlircd)

QT += network

SOURCES += \
    devicepluginlircd.cpp \
    lircdclient.cpp \

HEADERS += \
    devicepluginlircd.h \
    lircdclient.h \



