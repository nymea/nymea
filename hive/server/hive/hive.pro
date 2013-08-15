#-------------------------------------------------
#
# Project created by QtCreator 2013-08-15T13:31:23
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = hive
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    hivecore.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../libhive/release/ -llibhive
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../libhive/debug/ -llibhive
else:unix: LIBS += -L$$OUT_PWD/../../libhive/ -llibhive

INCLUDEPATH += $$PWD/../../libhive
DEPENDPATH += $$PWD/../../libhive

HEADERS += \
    hivecore.h
