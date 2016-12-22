TRANSLATIONS = translations/en_US.ts \
               translations/de_DE.ts

# Note: include after the TRANSLATIONS definition
include(../../plugins.pri)

TARGET = $$qtLibraryTarget(guh_deviceplugingpio)

SOURCES += \
    deviceplugingpio.cpp \
    gpiodescriptor.cpp

HEADERS += \
    deviceplugingpio.h \
    gpiodescriptor.h


