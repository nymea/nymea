TRANSLATIONS = translations/en_US.ts \
               translations/de_DE.ts

# Note: include after the TRANSLATIONS definition
include(../../plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginnetatmo)

SOURCES += \
    devicepluginnetatmo.cpp \
    netatmobasestation.cpp \
    netatmooutdoormodule.cpp

HEADERS += \
    devicepluginnetatmo.h \
    netatmobasestation.h \
    netatmooutdoormodule.h


