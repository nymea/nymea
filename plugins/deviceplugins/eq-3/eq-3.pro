include(../../plugins.pri)

TARGET = $$qtLibraryTarget(guh_deviceplugineq3)

QT += network

SOURCES += \
    deviceplugineq-3.cpp    \
    maxcubediscovery.cpp    \
    maxcube.cpp             \
    maxdevice.cpp           \
    room.cpp \
    livemessage.cpp

HEADERS += \
    deviceplugineq-3.h      \
    maxcubediscovery.h      \
    maxcube.h               \
    maxdevice.h             \
    room.h \
    livemessage.h

