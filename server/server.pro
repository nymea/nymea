include(../nymea.pri)

TARGET = guhd
TEMPLATE = app

INCLUDEPATH += ../libnymea ../libguh-core

target.path = /usr/bin
INSTALLS += target

QT *= sql xml websockets bluetooth dbus network

LIBS += -L$$top_builddir/libnymea/ -lnymea -L$$top_builddir/libguh-core -lguh-core -lssl -lcrypto -laws-iot-sdk-cpp

# Server files
include(qtservice/qtservice.pri)

SOURCES += main.cpp \
    guhservice.cpp \
    guhapplication.cpp

HEADERS += \
    guhservice.h \
    guhapplication.h

