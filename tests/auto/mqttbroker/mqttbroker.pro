include(../../../nymea.pri)
include(../autotests.pri)

greaterThan(QT_MAJOR_VERSION, 5) {
    QT += core5compat
} else {
    QT += xml
}

PKGCONFIG += nymea-mqtt

TARGET = nymeatestmqttbroker
SOURCES += testmqttbroker.cpp

