TEMPLATE = lib
TARGET = nymea-tests

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

# install header file with relative subdirectory
for(header, HEADERS) {
    path = $$[QT_INSTALL_PREFIX]/include/nymea-tests/$${dirname(header)}
    eval(headers_$${path}.files += $${header})
    eval(headers_$${path}.path = $${path})
    eval(INSTALLS *= headers_$${path})
}

# Create pkgconfig file
CONFIG += create_pc create_prl no_install_prl
QMAKE_PKGCONFIG_NAME = libnymea-tests
QMAKE_PKGCONFIG_DESCRIPTION = nymea-tests development library
QMAKE_PKGCONFIG_PREFIX = $$[QT_INSTALL_PREFIX]
QMAKE_PKGCONFIG_INCDIR = $$[QT_INSTALL_PREFIX]/include/nymea-tests/
QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_VERSION = $$NYMEA_VERSION_STRING
QMAKE_PKGCONFIG_FILE = nymea-tests
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
