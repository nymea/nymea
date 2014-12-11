include(../../plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginwemo)

QT+= network

SOURCES += \
    devicepluginwemo.cpp \
    wemoswitch.cpp


HEADERS += \
    devicepluginwemo.h \
    wemoswitch.h



