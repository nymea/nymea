TARGET = versioning

include(../../../guh.pri)
include(../autotests.pri)

DEFINES += TESTING_ENABLED
DEFINES += TESTS_SOURCE_DIR=\\\"$$top_srcdir/tests/auto/\\\"

SOURCES += testversioning.cpp
