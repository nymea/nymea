include(../../plugins.pri)

TARGET = $$qtLibraryTarget(guh_devicepluginlgsmarttv)

QT+= network xml

SOURCES += \
    devicepluginlgsmarttv.cpp \
    tvdiscovery.cpp \
    tvdevice.cpp \
    tveventhandler.cpp

HEADERS += \
    devicepluginlgsmarttv.h \
    tvdiscovery.h \
    tvdevice.h \
    tveventhandler.h


