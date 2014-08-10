include(../../plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginwemo)

QT+= network

SOURCES += \
    devicepluginwemo.cpp \
    wemodiscovery.cpp \
    wemoswitch.cpp


HEADERS += \
    devicepluginwemo.h \
    wemodiscovery.h \
    wemoswitch.h



