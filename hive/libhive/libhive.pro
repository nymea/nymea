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

LIBS += -lqjson

SOURCES += libhive.cpp \
    server.cpp \
    devicemanager.cpp \
    logwriter.cpp \
    client.cpp \
    jsonhandler.cpp \
    jsonplugin/jsonplugin.cpp \
    jsonplugin/devicejsonplugin.cpp

HEADERS += libhive.h\
        libhive_global.h \
    server.h \
    devicemanager.h \
    logwriter.h \
    client.h \
    jsonhandler.h \
    jsonplugin/jsonplugin.h \
    jsonplugin/devicejsonplugin.h

#unix:!symbian {
#    maemo5 {
#        target.path = /opt/usr/lib
#    } else {
#        target.path = /usr/lib
#    }
#    INSTALLS += target
#}
