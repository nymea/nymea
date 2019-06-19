TEMPLATE = lib
TARGET = nymea-testlib

include(../../nymea.pri)

QT += testlib network sql

INCLUDEPATH += $$top_srcdir/libnymea \
               $$top_srcdir/libnymea-core

LIBS += -L$$top_builddir/libnymea/ -lnymea \
        -L$$top_builddir/libnymea-core/ -lnymea-core \
        -lssl -lcrypto -lnymea-remoteproxyclient

HEADERS += nymeatestbase.h
SOURCES += nymeatestbase.cpp

target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target
