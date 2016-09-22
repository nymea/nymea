TRANSLATIONS = translations/en_US.ts \
               translations/de_DE.ts

# Note: include after the TRANSLATIONS definition
include(../../plugins.pri)

TARGET = $$qtLibraryTarget(guh_deviceplugindollhouse)

SOURCES += \
    deviceplugindollhouse.cpp \
    dollhouselight.cpp

HEADERS += \
    deviceplugindollhouse.h \
    dollhouselight.h
