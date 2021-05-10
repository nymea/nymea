include(../../../nymea.pri)
include(../autotests.pri)

QT += xml
PKGCONFIG += nymea-mqtt

TARGET = mqttbroker
SOURCES += testmqttbroker.cpp

