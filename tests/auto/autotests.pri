QT += dbus testlib network sql websockets
CONFIG += testcase
CONFIG += link_pkgconfig

packagesExist(Qt5SerialBus) {
    PKGCONFIG += Qt5SerialBus
    DEFINES += WITH_QTSERIALBUS
}

PKGCONFIG += nymea-zigbee nymea-networkmanager nymea-mqtt

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
