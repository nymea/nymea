TARGET = guhtests
QT += testlib network
CONFIG += testcase c++11
DEFINES += TESTING_ENABLED

INCLUDEPATH += $$top_srcdir/server/ $$top_srcdir/server/jsonrpc $$top_srcdir/libguh $$top_srcdir/tests/auto/
LIBS += -L$$top_builddir/libguh/ -lguh
QMAKE_LFLAGS += -Wl,--rpath=$$top_builddir/libguh

include($$top_srcdir/server/server.pri)

SOURCES += testjsonrpc.cpp \
    mocktcpserver.cpp

HEADERS += mocktcpserver.h

LIBS += -L$$top_builddir/plugins/deviceplugins/mock/ -lguh_devicepluginmock
