include(../nymea.pri)

TARGET = nymead
TEMPLATE = app

INCLUDEPATH += ../libnymea ../libnymea-core $$top_builddir

target.path = /usr/bin
INSTALLS += target

QT += sql xml websockets network

# Note: bluetooth is not available on all platforms
qtHaveModule(bluetooth) {
    message("Building with bluetooth support")
    QT += bluetooth
    DEFINES += WITH_BLUETOOTH
} else {
    message("Building without bluetooth support.")
}

# Note: dbus is not available on all platforms
qtHaveModule(dbus) {
    message("Building with dbus support")
    QT += dbus
    DEFINES += WITH_DBUS
} else {
    message("Building without dbus support.")
}

LIBS += -L$$top_builddir/libnymea/ -lnymea \
        -L$$top_builddir/libnymea-core -lnymea-core \
        -lnymea-remoteproxyclient

CONFIG += link_pkgconfig
PKGCONFIG += nymea-zigbee

# Server files
include(qtservice/qtservice.pri)

SOURCES += main.cpp \
    nymeaservice.cpp \
    nymeaapplication.cpp

HEADERS += \
    nymeaservice.h \
    nymeaapplication.h

