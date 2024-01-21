QT += dbus testlib network sql websockets
CONFIG += testcase
CONFIG += link_pkgconfig

greaterThan(QT_MAJOR_VERSION, 5) {
    qtHaveModule(serialbus) {
        message("Building with QtSerialBus support.")
        QT *= serialbus
        DEFINES += WITH_QTSERIALBUS
    } else {
        message("QtSerialBus package not found. Building without QtSerialBus support.")
    }

    # Separate module in Qt6
    QT *= concurrent
} else {
    packagesExist(Qt5SerialBus) {
        message("Building with QtSerialBus support.")
        PKGCONFIG += Qt5SerialBus
        DEFINES += WITH_QTSERIALBUS
    } else {
        message("Qt5SerialBus package not found. Building without QtSerialBus support.")
    }
}

CONFIG(python) {
    message("Building tests with Python plugin support")
    DEFINES += WITH_PYTHON
}

PKGCONFIG += nymea-zigbee nymea-networkmanager nymea-mqtt

INCLUDEPATH += $$top_srcdir/libnymea \
               $$top_srcdir/libnymea-core \
               $$top_srcdir/tests/testlib/ \
               $$top_builddir

LIBS += -L$$top_builddir/libnymea/ -lnymea \
        -L$$top_builddir/libnymea-core/ -lnymea-core \
        -L$$top_builddir/tests/testlib/ -lnymea-tests \
        -L$$top_builddir/plugins/mock/ \
        -lssl -lcrypto -lnymea-remoteproxyclient

target.path = $$[QT_INSTALL_PREFIX]/share/tests/nymea/
INSTALLS += target

test.commands = LD_LIBRARY_PATH=../../../libnymea:../../../libnymea-core/:../../testlib/ make check TESTRUNNER=\"dbus-test-runner --bus-type=both --task\"
QMAKE_EXTRA_TARGETS += test
