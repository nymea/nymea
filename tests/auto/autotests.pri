QT += testlib network sql
CONFIG += testcase

INCLUDEPATH += $$top_srcdir/libnymea \
               $$top_srcdir/libnymea-core \
               $$top_srcdir/tests/testlib/

LIBS += -L$$top_builddir/libnymea/ -lnymea \
        -L$$top_builddir/libnymea-core/ -lnymea-core \
        -L$$top_builddir/tests/testlib/ -lnymea-testlib \
        -L$$top_builddir/plugins/mock/ \
        -lssl -lcrypto -lnymea-remoteproxyclient

target.path = /usr/tests
INSTALLS += target

test.commands = LD_LIBRARY_PATH=../../../libnymea:../../../libnymea-core/:../../testlib/ make check TESTRUNNER=\"dbus-test-runner --bus-type=system --task\"
QMAKE_EXTRA_TARGETS += test
