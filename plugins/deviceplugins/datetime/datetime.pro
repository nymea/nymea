TRANSLATIONS = translations/en_US.ts \
               translations/de_DE.ts

# Note: include after the TRANSLATIONS definition
include (../../plugins.pri)

TARGET = $$qtLibraryTarget(guh_deviceplugindatetime)

SOURCES += \
    deviceplugindatetime.cpp \
    alarm.cpp \
    countdown.cpp

HEADERS += \
    deviceplugindatetime.h \
    alarm.h \
    countdown.h

