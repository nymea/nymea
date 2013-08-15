#-------------------------------------------------
#
# Project created by QtCreator 2013-08-11T20:58:11
#
#-------------------------------------------------

QT       += network declarative

TARGET = libhive
TEMPLATE = lib

DEFINES += LIBHIVE_LIBRARY

SOURCES += libhive.cpp \
    server.cpp

HEADERS += libhive.h\
        libhive_global.h \
    server.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
