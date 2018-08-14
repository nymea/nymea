include(../nymea.pri)

TARGET = nymead
TEMPLATE = app

INCLUDEPATH += ../libnymea ../libnymea-core

target.path = /usr/bin
INSTALLS += target

QT *= sql xml websockets bluetooth dbus network

LIBS += -L$$top_builddir/libnymea/ -lnymea \
        -L$$top_builddir/libnymea-core -lnymea-core \
        -lssl -lcrypto -laws-iot-sdk-cpp -lnymea-remoteproxyclient

# Server files
include(qtservice/qtservice.pri)

SOURCES += main.cpp \
    nymeaservice.cpp \
    nymeaapplication.cpp

HEADERS += \
    nymeaservice.h \
    nymeaapplication.h

