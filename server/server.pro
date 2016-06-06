include(../guh.pri)

TARGET = guhd
TEMPLATE = app

INCLUDEPATH += ../libguh jsonrpc

target.path = /usr/bin
INSTALLS += target

QT += sql xml websockets

LIBS += -L$$top_builddir/libguh/ -lguh

include(server.pri)
include(qtservice/qtservice.pri)

SOURCES += main.cpp \
    guhservice.cpp \
    guhapplication.cpp

HEADERS += \
    guhservice.h \
    guhapplication.h
