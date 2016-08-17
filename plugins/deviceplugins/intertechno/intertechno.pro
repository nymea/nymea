TRANSLATIONS = translations/en_US.ts \
               translations/de_DE.ts

# Note: include after the TRANSLATIONS definition
include (../../plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginintertechno)

SOURCES += \
    devicepluginintertechno.cpp

HEADERS += \
    devicepluginintertechno.h


