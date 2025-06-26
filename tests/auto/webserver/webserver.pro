include(../../../nymea.pri)
include(../autotests.pri)

greaterThan(QT_MAJOR_VERSION, 5) {
    QT *= core5compat
} else {
    QT *= xml
}


TARGET = nymeatestwebserver
SOURCES += testwebserver.cpp
