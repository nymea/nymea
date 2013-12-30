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

target.path = /usr/bin
INSTALLS += target

TEMPLATE = app


LIBS += -L$$OUT_PWD/../../libhive/ -llibhive
LIBS += -L$$RPI_SYSROOT/usr/local/lib -lwiringPi

INCLUDEPATH += /home/timon/opt/rasp-pi-rootfs/usr/local/include
INCLUDEPATH += $$PWD/../../libhive

DEPENDPATH += $$PWD/../../libhive

SOURCES += main.cpp \
    hivecore.cpp \
    radio/radioreciver.cpp \
    radio/radiosender.cpp

HEADERS += \
    hivecore.h \
    radio/radioreciver.h \
    radio/radiosender.h

