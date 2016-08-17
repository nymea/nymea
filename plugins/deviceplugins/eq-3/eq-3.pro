TRANSLATIONS = translations/en_US.ts \
               translations/de_DE.ts

# Note: include after the TRANSLATIONS definition
include(../../plugins.pri)

TARGET = $$qtLibraryTarget(guh_deviceplugineq3)

SOURCES += \
    deviceplugineq-3.cpp    \
    maxcubediscovery.cpp    \
    maxcube.cpp             \
    maxdevice.cpp           \
    room.cpp \
    wallthermostat.cpp \
    radiatorthermostat.cpp

HEADERS += \
    deviceplugineq-3.h      \
    maxcubediscovery.h      \
    maxcube.h               \
    maxdevice.h             \
    room.h \
    wallthermostat.h \
    radiatorthermostat.h

