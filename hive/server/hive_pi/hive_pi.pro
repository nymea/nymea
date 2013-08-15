#-------------------------------------------------
#
# Project created by QtCreator 2013-08-15T13:14:35
#
#-------------------------------------------------

QT       += core network
QT       -= gui

TARGET = hive

CONFIG   += console
CONFIG   -= app_bundle

target.path = /root/bin
INSTALLS += target


TEMPLATE = app


SOURCES += main.cpp \
    hivecore.cpp

HEADERS += \
    hivecore.h

LIBS += -L$$OUT_PWD/../../libhive/ -llibhive

INCLUDEPATH += $$PWD/../../libhive
DEPENDPATH += $$PWD/../../libhive
