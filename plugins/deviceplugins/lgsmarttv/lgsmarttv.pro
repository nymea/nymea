include(../../plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginlgsmarttv)

QT+= network

SOURCES += \
    devicepluginlgsmarttv.cpp \
    tvdiscovery.cpp \
    tvdevice.cpp

HEADERS += \
    devicepluginlgsmarttv.h \
    tvdiscovery.h \
    tvdevice.h


