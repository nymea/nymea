include(../../plugins.pri)

TARGET = $$qtLibraryTarget(guh_deviceplugingooglemail)

QT+= network

SOURCES += \
    deviceplugingooglemail.cpp \
    smtpclient.cpp

HEADERS += \
    deviceplugingooglemail.h \
    smtpclient.h


