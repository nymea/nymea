#-------------------------------------------------
#
# Project created by QtCreator 2013-08-15T13:14:35
#
#-------------------------------------------------

QT       += core network

QT       -= gui

TARGET = hive_pi

CONFIG   += console
CONFIG   -= app_bundle

target.path = /usr/bin
INSTALLS += target


TEMPLATE = app


SOURCES += main.cpp \
    hivecore.cpp

unix:!macx: LIBS += -L$$OUT_PWD/../../libhive/ -llibhive

INCLUDEPATH += $$PWD/../../libhive
DEPENDPATH += $$PWD/../../libhive

HEADERS += \
    hivecore.h
