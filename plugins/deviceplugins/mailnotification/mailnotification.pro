TRANSLATIONS = translations/en_US.ts \
               translations/de_DE.ts

# Note: include after the TRANSLATIONS definition
include(../../plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginmailnotification)

QT+= network

SOURCES += \
    devicepluginmailnotification.cpp \
    smtpclient.cpp

HEADERS += \
    devicepluginmailnotification.h \
    smtpclient.h


