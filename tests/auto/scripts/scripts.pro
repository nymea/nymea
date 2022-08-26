include(../../../nymea.pri)
include(../autotests.pri)

TARGET = nymeatestscripts
SOURCES += testscripts.cpp \
    testhelper.cpp

QT += qml

HEADERS += \
    testhelper.h
