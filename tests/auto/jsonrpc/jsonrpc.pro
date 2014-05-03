include(../../../guh.pri)
include(../autotests.pri)

TARGET = jsonrpc
DEFINES += TESTING_ENABLED
DEFINES += TESTS_SOURCE_DIR=\\\"$$top_srcdir/tests/auto/\\\"

SOURCES += testjsonrpc.cpp
