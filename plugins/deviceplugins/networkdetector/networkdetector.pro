TRANSLATIONS = translations/en_US.ts \
               translations/de_DE.ts

# Note: include after the TRANSLATIONS definition
include(../../plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginnetworkdetector)

SOURCES += \
    devicepluginnetworkdetector.cpp \
    host.cpp

HEADERS += \
    devicepluginnetworkdetector.h \
    host.h


