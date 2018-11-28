include(../../../nymea.pri)
include(../autotests.pri)

QT += xml

LIBS += -lnymea-mqtt
TARGET = mqttbroker
SOURCES += testmqttbroker.cpp
