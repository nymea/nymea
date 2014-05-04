TARGET = events

include(../../../guh.pri)
include(../autotests.pri)

DEFINES += TESTS_SOURCE_DIR=\\\"$$top_srcdir/tests/auto/\\\"

SOURCES += testevents.cpp
