TARGET = versioning

include(../../../guh.pri)
include(../autotests.pri)

INCLUDEPATH += $$top_srcdir/server/ $$top_srcdir/server/jsonrpc $$top_srcdir/libguh $$top_srcdir/tests/auto/

DEFINES += TESTING_ENABLED
DEFINES += TESTS_SOURCE_DIR=\\\"$$top_srcdir/tests/auto/\\\"

SOURCES += testversioning.cpp
