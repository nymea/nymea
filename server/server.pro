include(../nymea.pri)

TARGET = nymead
TEMPLATE = app

INCLUDEPATH += ../libnymea ../libnymea-core $$top_builddir

target.path = /usr/bin
INSTALLS += target

QT += sql xml websockets bluetooth dbus network

LIBS += -L$$top_builddir/libnymea/ -lnymea \
        -L$$top_builddir/libnymea-core -lnymea-core \
        -lnymea-remoteproxyclient

CONFIG += link_pkgconfig
PKGCONFIG += nymea-zigbee

# Server files
include(qtservice/qtservice.pri)

SOURCES += main.cpp \
    nymeaservice.cpp \
    nymeaapplication.cpp

HEADERS += \
    nymeaservice.h \
    nymeaapplication.h

