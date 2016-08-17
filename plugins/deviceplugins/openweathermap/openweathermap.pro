TRANSLATIONS = translations/en_US.ts \
               translations/de_DE.ts

# Note: include after the TRANSLATIONS definition
include(../../plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginopenweathermap)

QT+= network

SOURCES += \
    devicepluginopenweathermap.cpp \

HEADERS += \
    devicepluginopenweathermap.h \


