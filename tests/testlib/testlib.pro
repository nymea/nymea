TEMPLATE = lib
TARGET = nymea-testlib

include(../../nymea.pri)

QT += testlib dbus network sql websockets

CONFIG += link_pkgconfig
PKGCONFIG += nymea-zigbee

# Qt serial bus module is officially available since Qt 5.8
# but not all platforms host the qt serialbus package.
# Let's check if the package exists, not the qt version
packagesExist(Qt5SerialBus) {
    Qt += serialbus
    DEFINES += WITH_QTSERIALBUS
}

INCLUDEPATH += $$top_srcdir/libnymea \
               $$top_srcdir/libnymea-core

LIBS += -L$$top_builddir/libnymea/ -lnymea \
        -L$$top_builddir/libnymea-core/ -lnymea-core \
        -lssl -lcrypto -lnymea-remoteproxyclient

HEADERS += nymeatestbase.h
SOURCES += nymeatestbase.cpp

target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target
