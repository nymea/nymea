TARGET = hivetests
QT += testlib network
CONFIG += testcase c++11
DEFINES += TESTING_ENABLED

INCLUDEPATH += $$top_srcdir/server/ $$top_srcdir/server/jsonrpc $$top_srcdir/libhive $$top_srcdir/tests/auto/
LIBS += -L$$top_builddir/libhive/ -lhive
QMAKE_LFLAGS += -Wl,--rpath=$$top_builddir/libhive

include($$top_srcdir/server/server.pri)

SOURCES += testjsonrpc.cpp \
    mocktcpserver.cpp

HEADERS += mocktcpserver.h

LIBS += -L../mocks/mockdeviceplugin/ -lhive_devicepluginmockdevice
