include(../../../nymea.pri)
include(../autotests.pri)

TARGET = scripts
SOURCES += testscripts.cpp \
    testhelper.cpp

QT += qml

HEADERS += \
    testhelper.h
