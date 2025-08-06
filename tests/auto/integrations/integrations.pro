include(../../../nymea.pri)
include(../autotests.pri)

CONFIG(python) {
    message("Building tests with Python plugin support")
    DEFINES += WITH_PYTHON
}

TARGET = nymeatestintegrations
SOURCES += testintegrations.cpp
