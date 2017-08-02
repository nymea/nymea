include(../../guh.pri)
include(../../server/server.pri)

TARGET = guh-core
TEMPLATE = lib

QT += sql

DEFINES += TESTING_ENABLED

INCLUDEPATH += $$top_srcdir/server/ \
               $$top_srcdir/server/jsonrpc \
               $$top_srcdir/libguh \
               $$top_srcdir/tests/auto/

LIBS += -L$$top_builddir/libguh/ -lguh -lssl -lcrypto

target.path = /usr/lib/$$system('dpkg-architecture -q DEB_HOST_MULTIARCH')
INSTALLS += target
