#-------------------------------------------------
#
# Project created by QtCreator 2013-08-11T20:58:11
#
#-------------------------------------------------

QT       += core network

TARGET = libhive
TEMPLATE = lib
CONFIG += static

DEFINES += LIBHIVE_LIBRARY

SOURCES += libhive.cpp \
    server.cpp \
    devicemanager.cpp \
    logwriter.cpp \
    jsonparser.cpp \
    jsonserializer.cpp

HEADERS += libhive.h\
        libhive_global.h \
    server.h \
    devicemanager.h \
    logwriter.h \
    jsonparser.h \
    jsonserializer.h

#unix:!symbian {
#    maemo5 {
#        target.path = /opt/usr/lib
#    } else {
#        target.path = /usr/lib
#    }
#    INSTALLS += target
#}
