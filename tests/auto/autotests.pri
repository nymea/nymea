QT += dbus testlib network sql websockets
CONFIG += testcase
CONFIG += link_pkgconfig

# Qt serial bus module is officially available since Qt 5.8
# but not all platforms host the qt serialbus package.
# Let's check if the package exists, not the qt version
packagesExist(Qt5SerialBus) {
    message("Building with QtSerialBus support.")
    # Qt += serialbus
    PKGCONFIG += Qt5SerialBus
    DEFINES += WITH_QTSERIALBUS
} else {
    message("Qt5SerialBus package not found. Building without QtSerialBus support.")
}

PKGCONFIG += nymea-zigbee

INCLUDEPATH += $$top_srcdir/libnymea \
               $$top_srcdir/libnymea-core \
               $$top_srcdir/tests/testlib/ \
               $$top_builddir

LIBS += -L$$top_builddir/libnymea/ -lnymea \
        -L$$top_builddir/libnymea-core/ -lnymea-core \
        -L$$top_builddir/tests/testlib/ -lnymea-testlib \
        -L$$top_builddir/plugins/mock/ \
        -lssl -lcrypto -lnymea-remoteproxyclient

target.path = /usr/tests
INSTALLS += target

test.commands = LD_LIBRARY_PATH=../../../libnymea:../../../libnymea-core/:../../testlib/ make check TESTRUNNER=\"dbus-test-runner --bus-type=both --task\"
QMAKE_EXTRA_TARGETS += test
