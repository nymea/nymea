TRANSLATIONS =  translations/en_US.ts \
                translations/de_DE.ts

include(../../plugins.pri)

QT += serialport

TARGET = $$qtLibraryTarget(guh_devicepluginusbwde)

SOURCES += \
    devicepluginusbwde.cpp

HEADERS += \
    devicepluginusbwde.h


