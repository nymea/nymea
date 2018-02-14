include(../guh.pri)

TARGET = guhd
TEMPLATE = app

INCLUDEPATH += ../libguh-core ../libguh-core/jsonrpc ../libguh

target.path = /usr/bin
INSTALLS += target

QT *= sql xml websockets bluetooth dbus network

LIBS += -L$$top_builddir/libguh/ -lguh -L$$top_builddir/libguh-core -lguh-core -lssl -lcrypto -laws-iot-sdk-cpp

# Server files
include(qtservice/qtservice.pri)

SOURCES += main.cpp \
    guhservice.cpp \
    guhapplication.cpp

HEADERS += \
    guhservice.h \
    guhapplication.h

