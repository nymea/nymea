TRANSLATIONS = translations/en_US.ts \
               translations/de_DE.ts

# Note: include after the TRANSLATIONS definition
include(../../plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginws2812)

SOURCES += \
    devicepluginws2812.cpp

HEADERS += \
    devicepluginws2812.h
