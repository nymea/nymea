include(../../../nymea.pri)
include(../autotests.pri)

QT += xml
PKGCONFIG += nymea-mqtt

TARGET = nymeatestmqttbroker
SOURCES += testmqttbroker.cpp

