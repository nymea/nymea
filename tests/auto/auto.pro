include(../../guh.pri)

TARGET = guhtests
QT += testlib network
CONFIG += testcase c++11
DEFINES += TESTING_ENABLED
DEFINES += TESTS_SOURCE_DIR=\\\"$$top_srcdir/tests/auto/\\\"

INCLUDEPATH += $$top_srcdir/server/ $$top_srcdir/server/jsonrpc $$top_srcdir/libguh $$top_srcdir/tests/auto/
LIBS += -L$$top_builddir/libguh/ -lguh -L$$top_builddir/plugins/deviceplugins/mock/ -lguh_devicepluginmock
QMAKE_LFLAGS += -Wl,--rpath=$$top_builddir/libguh

include($$top_srcdir/server/server.pri)

testcases="TestJSONRPC TestVersioning TestDevices"

SOURCES += guhtestbase.cpp \
    mocktcpserver.cpp \
    testjsonrpc.cpp \
    testversioning.cpp \
    testdevices.cpp

HEADERS += mocktcpserver.h \
    guhtestbase.h \
    testdefines.h \
    testversioning.h \
    testjsonrpc.h \
    testdevices.h

message("testcases: $${testcases}")
#DEFINES += TESTCASES=\\\"$${testcases}\\\"
#DEFINES += TESTCASES=\\\"TestJSONRPC\\\"

system(./generatedefines.sh $${testcases})
